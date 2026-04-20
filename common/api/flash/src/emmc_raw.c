#define _LARGEFILE64_SOURCE

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mt_flash.h"
#include "nand.h"
#include "emmc_raw.h"
#include "mt_module_debug.h"
#include "flash_api_internal.h"

emmc_flash_s g_emmcflash;

#define EMMC_RAW_AREA_START 0//EMMC_SECTOR_SIZE

mt_s32 find_part_from_devname(char *media_name, char *bootargs,
			      char *devname, mt_u64 *start, mt_u64 *size)
{
	mt_u8 partnum = 0;
	char *tmp;

	if (!(tmp = strstr(bootargs, "blkdevparts=")))
		return MT_FAILURE;
	tmp += strlen("blkdevparts=");

	if (!strstr(bootargs, media_name))
		return MT_FAILURE;

	if (!(tmp = strstr(devname, "mmcblk0p")))
		return MT_FAILURE;
	tmp += strlen("mmcblk0p");
	partnum = (mt_u8)strtol(tmp, NULL, 10);

	if (get_part_info(partnum, start, size))
		return MT_FAILURE;

	return MT_SUCCESS;
}

mt_s32 emmc_raw_init(char *bootargs)
{
    mt_u8  aucBuf[512];
    mt_s32  dev_fd;
    mt_s32  tmp_fd = -1;
    mt_s32 ret = -1;

    if (!bootargs) {
        MT_ERR_FLASH("Invalid parameter, bootargs NULL\n");
        return MT_FAILURE;
    }

    ret = cmdline_parts_init(bootargs);
    if (ret < 0)
    {
        MT_ERR_FLASH("cmdline parts init failed, ret=%d, bootargs:%s\n", ret, bootargs);
        return MT_FAILURE;
    }

#if defined (ANDROID)
    if ((dev_fd = open("/dev/block/mmcblk0", O_RDWR | O_CLOEXEC)) == -1)
#else
    if ((dev_fd = open("/dev/mmcblk0", O_RDWR | O_CLOEXEC)) == -1)
#endif
    {
        MT_ERR_FLASH("open mmcblk0 failed, errno=%d\n", errno);
        return MT_FAILURE;
    }

    g_emmcflash.erasesize = EMMC_SECTOR_SIZE;

    if((ssize_t)sizeof(aucBuf) != read(dev_fd, aucBuf, sizeof(aucBuf)))
    {
        MT_ERR_FLASH("Failed to read dev.");
        close(dev_fd);
        return MT_FAILURE;
    }

    close(dev_fd);

    /* Raw area start from 512, after MBR.. */
    g_emmcflash.raw_areastart = EMMC_RAW_AREA_START;
	tmp_fd = open("/sys/block/mmcblk0/size", O_RDONLY | O_CLOEXEC);
	if (tmp_fd < 0)
	{
		MT_ERR_FLASH("Fail to open the size of mmcblk0\n");
		return MT_FAILURE;
	}

	memset(aucBuf, 0, sizeof(aucBuf));
	if (0 > read(tmp_fd, aucBuf, sizeof(aucBuf)))
	{
        MT_ERR_FLASH("Failed to read the size of mmcblk0\n");
        close(tmp_fd);
        return MT_FAILURE;
    }
	aucBuf[sizeof(aucBuf)-1] = '\0';
	close(tmp_fd);

	g_emmcflash.raw_areasize  = (mt_u64)strtoull((char *)aucBuf, NULL, 10)
									* EMMC_SECTOR_SIZE - EMMC_RAW_AREA_START;

    return MT_SUCCESS;
}

static mt_s32 emmc_flash_probe(void)
{
    int dev;
#if defined (ANDROID)
    if ((dev = open("/dev/block/mmcblk0", O_RDWR | O_SYNC | O_CLOEXEC)) == -1)
#else
    if ((dev = open("/dev/mmcblk0", O_RDWR | O_SYNC | O_CLOEXEC)) == -1)
#endif
    {
        MT_ERR_FLASH("Failed to open device '/dev/mmcblk0'.");
        return MT_FAILURE;
    }

    return (dev);
}

emmc_cb_s *emmc_raw_open(mt_u64 addr,
                         mt_u64 len)
{
    emmc_cb_s *p_emmc_cb;
    int fd;

    fd = emmc_flash_probe();
    if( -1 == fd )
    {
        MT_ERR_FLASH("no devices available.");
        return NULL;
    }

    /* Reject open, which are not block aligned */
    if ((addr & (g_emmcflash.erasesize - 1))
            || (len & (g_emmcflash.erasesize - 1)))
    {
        MT_ERR_FLASH("Attempt to open non block aligned, "
                "eMMC blocksize: 0x%x, addr: 0x%08llx, length: 0x%08llx.",
                g_emmcflash.erasesize,
                addr,
                len);
        close(fd);
        return NULL;
    }

    if ((addr > g_emmcflash.raw_areastart + g_emmcflash.raw_areasize)
            || (len > g_emmcflash.raw_areastart + g_emmcflash.raw_areasize)
            || ((addr + len) > g_emmcflash.raw_areastart + g_emmcflash.raw_areasize))
    {
        MT_ERR_FLASH("Attempt to open outside the flash area, "
                "eMMC chipsize: 0x%08llx, addr: 0x%08llx, length: 0x%08llx\n",
                g_emmcflash.raw_areastart + g_emmcflash.raw_areasize,
                addr,
                len);
        close(fd);
        return NULL;
    }

    if ((p_emmc_cb = (emmc_cb_s *)malloc(sizeof(emmc_cb_s))) == NULL)
    {
        MT_ERR_FLASH("no many memory.");
        close(fd);
        return NULL;
    }

    p_emmc_cb->addr  = addr;
    p_emmc_cb->partsize = len;
    p_emmc_cb->erasesize = g_emmcflash.erasesize;
    p_emmc_cb->fd          = fd;
    p_emmc_cb->part_type  = EMMC_PART_TYPE_RAW;

    return p_emmc_cb;
}

emmc_cb_s *emmc_node_open(const mt_u8 *p_node)
{
    emmc_cb_s *p_emmc_cb;
    mt_s32    fd;

    if( NULL == p_node )
    {
        return NULL;
    }

    if (-1 == (fd = open((const char*)p_node, O_RDWR | O_CLOEXEC)))
    {
        MT_ERR_FLASH("no devices available.");
        return NULL;
    }

    if( NULL == (p_emmc_cb = (emmc_cb_s *)malloc(sizeof(emmc_cb_s))))
    {
        MT_ERR_FLASH("No enough space.");
        close(fd);
        return NULL;
    }

    p_emmc_cb->addr   = 0;
    p_emmc_cb->partsize  = 0;
    p_emmc_cb->erasesize = g_emmcflash.erasesize;
    p_emmc_cb->fd         = fd;
    p_emmc_cb->part_type = EMMC_PART_TYPE_LOGIC;

    return p_emmc_cb;
}

mt_s32 emmc_block_read(mt_s32 fd,
                              mt_u64 start,
                              mt_u32 u32Len,
                              void *buff)
{
    ssize_t ret;
    if( -1 == lseek64(fd, (off64_t)start, SEEK_SET))
    {
        MT_ERR_FLASH("Failed to lseek64.");
        return MT_FAILURE;
    }

    ret = read(fd, buff, u32Len);
    if (ret < 0)
        return MT_FAILURE;

    return (mt_s32)ret;
}

mt_s32 emmc_block_write(mt_s32 fd,
                               mt_u64 start,
                               mt_u32 u32Len,
                               const void *buff)
{
    ssize_t ret;
    if( -1 == lseek64(fd, (off64_t)start, SEEK_SET))
    {
        MT_ERR_FLASH("Failed to lseek64.");
        return MT_FAILURE;
    }

    ret = write(fd, buff, u32Len);
    if (ret < 0)
        return MT_FAILURE;

    return (mt_s32)ret;
}

mt_s32 emmc_raw_read(const emmc_cb_s *p_emmc_cb,
                     mt_u64    offset,    /* should be alignment with emmc block size */
                     mt_u32    len,    /* should be alignment with emmc block size */
                     mt_u8     *buf)
{
    mt_s32 ret;
    mt_u64 start;

    if( NULL == p_emmc_cb || NULL == buf)
    {
        MT_ERR_FLASH("Pointer is null.");
        return MT_FAILURE;
    }

    /* Reject read, which are not block aligned */
    if( EMMC_PART_TYPE_RAW == p_emmc_cb->part_type )
    {
        if ((offset > p_emmc_cb->partsize)
                || (len > p_emmc_cb->partsize)
                || ((offset + len) > p_emmc_cb->partsize))
        {
            MT_ERR_FLASH("Attempt to write outside the flash handle area, "
                    "eMMC part size: 0x%08llx, offset: 0x%08llx, "
                    "length: 0x%08x.",
                    p_emmc_cb->partsize,
                    offset,
                    len);

            return MT_FAILURE;
        }
    }

    start = p_emmc_cb->addr + offset;
    ret = emmc_block_read(p_emmc_cb->fd, start, len, buf);
    return ret;
}

mt_s32 emmc_raw_write(const emmc_cb_s *p_emmc_cb,
                      mt_u64    offset,    /* should be alignment with emmc block size */
                      mt_u32    len,    /* should be alignment with emmc block size */
                      const mt_u8     *buf)
{
    mt_s32 ret;
    mt_u64 start;

    if( NULL == p_emmc_cb || NULL == buf)
    {
        MT_ERR_FLASH("Pointer is null.");
        return MT_FAILURE;
    }

    if( EMMC_PART_TYPE_RAW == p_emmc_cb->part_type )
    {
        if ((offset > p_emmc_cb->partsize)
                || (len > p_emmc_cb->partsize)
                || ((offset + len) > p_emmc_cb->partsize))
        {
            MT_ERR_FLASH("Attempt to write outside the flash handle area, "
                    "eMMC part size: 0x%08llx, offset: 0x%08llx, "
                    "length: 0x%08x\n",
                    p_emmc_cb->partsize,
                    offset,
                    len);

            return MT_FAILURE;
        }
    }

    start = p_emmc_cb->addr + offset;
    ret = emmc_block_write(p_emmc_cb->fd, start, len, buf);
    return ret;
}

mt_s32 emmc_raw_close(emmc_cb_s *p_emmc_cb)
{
    if( NULL == p_emmc_cb)
    {
        MT_ERR_FLASH("Pointer is null.");
        return MT_FAILURE;
    }

    close(p_emmc_cb->fd);
    free(p_emmc_cb);
    p_emmc_cb = NULL;

    return MT_SUCCESS;
}

#if defined (MT_EMC_ERASE_SUPPORT)
mt_s32 emc_raw_erase(emmc_cb_s *p_emmc_cb,
                      mt_u64 offset,
                      mt_u64 len)
{
    mt_s32 ret;
    mt_u64 start;
    struct mmc_erase_cmd cmd;

    if(NULL == p_emmc_cb)
    {
        MT_ERR_FLASH("Pointer is null.");
        return MT_FAILURE;
    }

    if(EMMC_PART_TYPE_RAW == p_emmc_cb->part_type)
    {
        if ((offset > p_emmc_cb->partsize)
                || (len > p_emmc_cb->partsize)
                || ((offset + len) > p_emmc_cb->partsize))
        {
            MT_ERR_FLASH("Attempt to write outside the flash handle area, "
                    "eMMC part size: 0x%08llx, offset: 0x%08llx, "
                    "length: 0x%08llx\n",
                    p_emmc_cb->partsize,
                    offset,
                    len);

            return MT_FAILURE;
        }
    }

    start = p_emmc_cb->addr + offset;
    //MT_PRINT("[%s][%d][%s] offset=0x%08llx, len=0x%08llx, start=0x%08llx, fd=%d \n", __FILE__, __LINE__, __FUNCTION__, offset, len, start, p_emmc_cb->fd);
	memset(&cmd, 0, sizeof(cmd));

	cmd.from = (unsigned int)(start >> 9);
	cmd.nr   = (unsigned int)(len >> 9);
	cmd.arg = 0x01;
   // printf("from=0x%x, nr=0x%x\n", cmd.from, cmd.nr);
	ret = ioctl(p_emmc_cb->fd, MMC_ERASE_CMD, &cmd);
	if (ret)
		perror("ioctl");

    return ret;
}
#endif

