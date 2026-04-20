#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "mt_flash.h"
#include "nand.h"
#include "spi_raw.h"
#include "mt_module_debug.h"

static struct nand_raw_ctrl * spiraw_ctrl = NULL;

/*****************************************************************************/

int spi_raw_init(void)
{
    int ix;
    int dev;
    int readonly;
    int max_partition;
    //struct stat status;
    struct mtd_partition *ptn;
    struct mtd_info_user mtdinfo;

    // char buf[sizeof(DEV_MTDBASE) + 5] = DEV_MTDBASE;
    // char *ptr = buf + sizeof(DEV_MTDBASE) - 1;
    char buf[PATH_MAX];

    if (spiraw_ctrl)
        return 0;

    if (0 == (max_partition = get_max_partition()))
    {
#ifdef SPI_RAW_DBG
        MT_ERR_FLASH("Can't find mtd device at /dev/mtdx.\n");
#endif
        return -1;
    }

    if (++max_partition >= MAX_MTD_PARTITION)
    {
        MT_INFO_FLASH("partition maybe more than %d, please increase MAX_MTD_PARTITION.\n", MAX_MTD_PARTITION);
    }

    if(0 != (flash_partition_info_init()))
    {
        DBG_OUT("Initial partition information failure.\n");
        return -1;
    }
    spiraw_ctrl = (struct nand_raw_ctrl *)malloc(sizeof(struct nand_raw_ctrl)
        + (mt_u32)max_partition * sizeof(struct mtd_partition));

    if (!spiraw_ctrl)
    {
        MT_ERR_FLASH("Not enough memory.\n");
        return -ENOMEM;
    }

    spiraw_ctrl->num_partition = 0;
    spiraw_ctrl->size = 0;
    ptn = spiraw_ctrl->partition;

    for (ix = 0; ix < max_partition; ix++)
    {
        readonly = 0;
        ptn->fd = INVALID_FD;
        (mt_void)snprintf(buf, PATH_MAX, DEV_MTDBASE"%d", ix);
        if ((dev = open(buf, O_RDWR | O_CLOEXEC)) == -1)
        {
            if ((dev = open(buf, O_RDONLY | O_CLOEXEC)) == -1)
            {
                //MT_PRINT("Can't open \"%s\"\n", buf);
                ptn->perm =  ACCESS_NONE;
                continue;
            }
            ptn->perm = ACCESS_RD;
            readonly = 1;
        } else {
            ptn->perm = ACCESS_RDWR;
        }

        if (ioctl(dev, MEMGETINFO, &mtdinfo))
        {
            MT_ERR_FLASH("Can't get \"%s\" information.\n", buf);
            close(dev);
            continue;
        }

        if (mtdinfo.type != MTD_SPIFLASH)
        {
            close(dev);
            continue;
        }

        if (mtdinfo.type == MTD_NANDFLASH)
        {
            close(dev);
            break;
        }
        mt_flash_partinfo_s *p_partinfo = NULL;
        char devname[32];
        memset(devname, 0, sizeof(devname));
        (mt_void)snprintf(devname, sizeof(devname), "mtd%d", ix);
        p_partinfo = get_flash_partition_info(MT_FLASH_TYPE_SPI_0, devname);
        if(NULL == p_partinfo)
        {
            DBG_OUT("Can't get \"%s\" partition information.\n", devname);
            close(dev);
            continue;
        }

        strncpy(ptn->mtddev, buf, sizeof(ptn->mtddev));
        ptn->mtddev[sizeof(ptn->mtddev) - 1] = '\0';
        ptn->fd = dev;
        ptn->readonly = readonly;
  #if 0
        ptn->start = spiraw_ctrl->size;
        spiraw_ctrl->size += mtdinfo.size;
        ptn->end   = spiraw_ctrl->size - 1;
  #else
        ptn->start = p_partinfo->start_addr;
        ptn->end   = p_partinfo->start_addr + mtdinfo.size -1;
  #endif
        spiraw_ctrl->num_partition++;

        ptn++;

        if (spiraw_ctrl->num_partition == 1)
        {
            spiraw_ctrl->pagesize  = mtdinfo.writesize;
            spiraw_ctrl->blocksize = mtdinfo.erasesize;
            spiraw_ctrl->pagemask  = (mtdinfo.writesize - 1);
            spiraw_ctrl->blockmask = (mtdinfo.erasesize - 1);
            spiraw_ctrl->oobsize   = mtdinfo.oobsize;

            spiraw_ctrl->pageshift  = (mt_u32)offshift(mtdinfo.writesize);
            spiraw_ctrl->blockshift = (mt_u32)offshift(mtdinfo.erasesize);
        }
    }

    if (!spiraw_ctrl->num_partition)
    {
        free(spiraw_ctrl);
        spiraw_ctrl = NULL;
        return 0;
    }
    spiraw_ctrl->size = get_flash_total_size(MT_FLASH_TYPE_SPI_0);

    return 0;
}

void spi_raw_get_info
(
    unsigned long long *totalsize,
    unsigned long *pagesize,
    unsigned long *blocksize,
    unsigned long *oobsize,
    unsigned long *blockshift)
{
    *totalsize  = spiraw_ctrl->size;
    *pagesize   = spiraw_ctrl->pagesize;
    *blocksize  = spiraw_ctrl->blocksize;
    *oobsize    = spiraw_ctrl->oobsize;
    *blockshift = spiraw_ctrl->blockshift;
}

/*****************************************************************************/
/*
 * warning:
 *    1. if open SPI/NOR FLASH, return 0
      2. if dev_name cannot match spiraw_ctrl, return error_valid;
 */
unsigned long long spi_raw_get_start_addr(const char *dev_name, unsigned long blocksize, int *value_valid)
{
    struct mtd_partition *ptn;
    int max_partition;
    int ix;

    ptn = spiraw_ctrl->partition;

    if (0 == (max_partition = get_max_partition()))
    {
        //MT_PRINT("Can't find mtd device at /dev/mtdx.\n");
        return 0;
    }

    if (spiraw_ctrl->blocksize != blocksize)
    {
        *value_valid = 1;
        return 0;
    }

    for (ix = 0; ix <= max_partition; ix++)
    {
        if (!strncmp(ptn->mtddev, dev_name,
            strlen(ptn->mtddev) > strlen(dev_name) ?
                strlen(ptn->mtddev) : strlen(dev_name)))
        {
            break;
        }
        ptn++;
    }

    if (max_partition < ix)
    {
        *value_valid = 0;
        return 0;
    }
    else
    {
        *value_valid = 1;
        /*lint -e661*/
        return ptn->start;
        /*lint +e661*/
    }
}
/*****************************************************************************/
/*
 * warning:
 *    1. start_addr should be alignment with pagesize
 */

int spi_raw_read
(
    int fd,
    unsigned long long *start_addr, /* this address maybe change when meet bad block */
    unsigned char      *buffer,
    unsigned long       len,    /* if MT_FLASH_RW_FLAG_WITH_OOB, include oob*/
    unsigned long long  openaddr,
    unsigned long long  limit_len,
    int                 read_oob,
    int                 skip_badblock)
{
    int ix;
    int totalread = 0;
    int num_read  = 0;
    struct mtd_partition *ptn;
    unsigned long long offset = *start_addr;

    openaddr = openaddr;
    skip_badblock = skip_badblock;
    fd = fd;
    limit_len = limit_len;
    read_oob=read_oob;

    if (!spiraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= spiraw_ctrl->size || !len)
    {
        return -1;
    }

    for (ix = 0; ix < spiraw_ctrl->num_partition && len; ix++)
    {
        ptn = &spiraw_ctrl->partition[ix];
        /*lint -save -e655 */
        if ((ptn->start <= offset) && (offset < ptn->end) && len && (ptn->perm & ACCESS_RD) && (ptn->fd != INVALID_FD))
        {
            if (offset + len > ptn->end)
            {
                num_read = (int)((ptn->end - offset) + 1);
            }
            else
            {
                num_read = (int)len;
            }
            DBG_OUT(">ptn->fd=%d, len=%#x, *start_addr=%#llx, offset=%#llx\n", ptn->fd, num_read, *start_addr, offset);

            if (lseek(ptn->fd, (long)(offset - ptn->start), SEEK_SET) != -1
                && read(ptn->fd, buffer, (size_t)num_read) != (ssize_t)num_read)
            {
                MT_ERR_FLASH("read \"%s\" fail. %s\n", ptn->mtddev, strerror(errno));
                return MT_FAILURE;
            }

            buffer    += num_read;
            len    -= (unsigned long)num_read;
            totalread += num_read;

            offset += (unsigned long long)num_read;
        }
    }
    *start_addr = offset;

    return totalread;
}
/*****************************************************************************/
/*
 * warning:
 *    1. offset and len should be alignment with blocksize
 */
long long spi_raw_erase
(
    int fd,
    unsigned long long start_addr,
    unsigned long long len,
    unsigned long long openaddr,
    unsigned long long limit_len)
{
    int ix;
    long long totalerase = 0;
    unsigned long long offset = start_addr;
    struct mtd_partition *ptn;

    openaddr = openaddr;
    fd = fd;
    limit_len = limit_len;


    if (!spiraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= spiraw_ctrl->size || !len)
    {
        return -1;
    }

    if (((unsigned long)offset & spiraw_ctrl->blockmask)
        || ((unsigned long)len & spiraw_ctrl->blockmask))
    {
        MT_ERR_FLASH("offset or len should be alignment with blocksize(0x%X)\n",
            (unsigned int)spiraw_ctrl->blocksize);
        return -1;
    }

    if (offset + len > spiraw_ctrl->size)
        len = spiraw_ctrl->size - offset;

    for (ix = 0; ix < spiraw_ctrl->num_partition && len; ix++)
    {
        ptn = &spiraw_ctrl->partition[ix];

        if (ptn->readonly)
        {
            MT_ERR_FLASH("erase a read only partition \"%s\".\n", ptn->mtddev);
            return -1;
        }

        if ((ptn->start <= offset) && (offset < ptn->end) && len && (ptn->perm & ACCESS_WR) && ptn->fd != INVALID_FD)
        {
            struct erase_info_user64 eraseinfo;

            eraseinfo.start  = (unsigned long long)(offset - ptn->start);
            if (offset + len > ptn->end)
            {
                eraseinfo.len = (unsigned long long)((ptn->end - offset) + 1);
            }
            else
            {
                eraseinfo.len = (unsigned long long)len;
            }

            DBG_OUT(">ptn->fd=%d, len=%#llx, *start_addr=%#llx, offset=%#llx\n", ptn->fd, eraseinfo.len, start_addr, offset);

            /* don't deal with */
            if (ioctl(ptn->fd, MEMERASE64, &eraseinfo))
            {
                MT_INFO_FLASH("Erase 0x%llx failed!\n", offset);
            }

            len -= eraseinfo.len;
            offset += eraseinfo.len;
            totalerase += (long long)eraseinfo.len;
        }
    }
    return totalerase;
}


/*****************************************************************************/
/*
 * warning:
 *    1. start_addr should be alignment with pagesize
 */
int spi_raw_write
(
    int fd,
    unsigned long long *start_addr,
    unsigned char *buffer,
    unsigned long len,
    unsigned long long openaddr,
    unsigned long long limit_len,
    int write_oob)
{
    int ix;
    int totalwrite = 0;
    int num_write = 0;
    struct mtd_partition *ptn;
    unsigned long long offset = *start_addr;

    openaddr = openaddr;
    write_oob = write_oob;
    fd = fd;
    limit_len = limit_len;

    if (!spiraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= spiraw_ctrl->size || !len)
    {
        return -1;
    }

    for (ix = 0; ix < spiraw_ctrl->num_partition && len; ix++)
    {
        ptn = &spiraw_ctrl->partition[ix];
        if (ptn->readonly)
        {
            MT_ERR_FLASH("Write a read only partition \"%s\".\n", ptn->mtddev);
            return -1;
        }

        if ((ptn->start <= offset) && (offset < ptn->end) && len && (ptn->perm & ACCESS_WR) && ptn->fd != INVALID_FD)
        {
            if (offset + len > ptn->end)
            {
                num_write = (int)((ptn->end - offset) + 1);
            }
            else
            {
                num_write = (int)len;
            }
            DBG_OUT(">ptn->fd=%d, len=%#x, *start_addr=%#llx, offset=%#llx\n", ptn->fd, num_write, *start_addr, offset);

            if (lseek(ptn->fd, (off_t)(offset - ptn->start), SEEK_SET) != -1
                && write(ptn->fd, buffer, (size_t)num_write) != (ssize_t)num_write)
            {
                MT_ERR_FLASH("write \"%s\" fail. %s\n", ptn->mtddev, strerror(errno));
                return MT_FAILURE;
            }

            buffer     += num_write;
            len     -= (mt_u32)num_write;
            totalwrite += num_write;

            offset += (unsigned long long)num_write;
        }
    }

    *start_addr = offset;

    return totalwrite;
}


/*****************************************************************************/

int spi_raw_dump_partition(void)
{
    int ix;
    struct mtd_partition *ptn;

    if (!spiraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    MT_PRINT("-------------------------\n");
    MT_PRINT("mtd device   start len mode\n");
    for (ix = 0; ix < spiraw_ctrl->num_partition; ix++)
    {
        ptn = &spiraw_ctrl->partition[ix];
        if ((ptn->perm & ACCESS_RD) == ACCESS_RD) {
            MT_PRINT("%-12s ", ptn->mtddev);
            MT_PRINT("%5s ", int_to_size(ptn->start));
            MT_PRINT("%6s ", int_to_size(ptn->end + 1 - ptn->start));
            MT_PRINT("%2s ", ptn->readonly ? "r" : "rw");
            MT_PRINT("\n");
        }
    }

    return 0;
}
/*****************************************************************************/

int spi_raw_destroy(void)
{
    int ix;

    if (!spiraw_ctrl)
        return 0;

    for (ix = 0; ix < spiraw_ctrl->num_partition; ix++)
    {
        if( INVALID_FD != spiraw_ctrl->partition[ix].fd)
        {
            close(spiraw_ctrl->partition[ix].fd);
        }
    }

    if( NULL != spiraw_ctrl )
    {
        free(spiraw_ctrl);
    }
    spiraw_ctrl = NULL;
    //DBG_OUT("\n");

    return 0;
}

