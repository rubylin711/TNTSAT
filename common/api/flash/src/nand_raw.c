#define _LARGEFILE64_SOURCE
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
#include "nand_raw.h"
#include "mt_module_debug.h"

#define NAND_PAGE_BUF_SIZE       16384
#define NAND_OOB_BUF_SIZE        1200

static struct nand_raw_ctrl * nandraw_ctrl = NULL;

static int check_skip_badblock(struct mtd_partition *ptn, int *blockindex, int blocksize);
/*****************************************************************************/

int nand_raw_init(void)
{
    int ix;
    int dev;
    int readonly;
    int max_partition;
    //struct stat status;
    struct mtd_partition *ptn;
    struct mtd_info_user mtdinfo;
    mt_flash_partinfo_s *p_partinfo = NULL;
    char devname[32];

    char buf[PATH_MAX];

    if (nandraw_ctrl)
        return 0;

    if (0 > (max_partition = get_max_partition()))
    {

#ifdef NAND_RAW_DBG
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
        MT_ERR_FLASH("Initial partition information failure.\n");
        return -1;
    }
    nandraw_ctrl = (struct nand_raw_ctrl *)malloc(sizeof(struct nand_raw_ctrl)
        + (mt_u32)max_partition * sizeof(struct mtd_partition));

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Not enough memory.\n");
        return -1;
    }

    nandraw_ctrl->num_partition = 0;
    nandraw_ctrl->size = 0;
    ptn = nandraw_ctrl->partition;

    for (ix = 0; ix < max_partition; ix++)
    {
        readonly = 0;
        ptn->fd = INVALID_FD;
        (mt_void)snprintf(buf, PATH_MAX, DEV_MTDBASE"%d", ix);
        if ((dev = open(buf, O_RDWR | O_CLOEXEC)) == -1)
        {
            if ((dev = open(buf, O_RDONLY | O_CLOEXEC)) == -1)
            {
                //MT_PRINT("%s->%d Can't open \"%s\", set to access none!\n", __func__,__LINE__,buf);
                ptn->perm = ACCESS_NONE;
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

        if (mtdinfo.type != MTD_NANDFLASH)
        {
            close(dev);
            continue;
        }

        memset(devname, 0, sizeof(devname));
        (mt_void)snprintf(devname, sizeof(devname), "mtd%d", ix);
        p_partinfo = get_flash_partition_info(MT_FLASH_TYPE_NAND_0, devname);
        if(NULL == p_partinfo)
        {
            DBG_OUT("Can't get \"%s\" partition information.\n", buf);
            close(dev);
            continue;
        }
        strncpy(ptn->mtddev, buf, sizeof(ptn->mtddev));
        ptn->mtddev[sizeof(ptn->mtddev) - 1] = '\0';
        ptn->fd = dev;
        ptn->readonly = readonly;
  #if 0
        ptn->start = nandraw_ctrl->size;
        nandraw_ctrl->size += mtdinfo.size;
        ptn->end   = nandraw_ctrl->size - 1;
  #else
        ptn->start = p_partinfo->start_addr;
        ptn->end   = p_partinfo->start_addr + mtdinfo.size -1;
  #endif
        nandraw_ctrl->num_partition++;

        ptn++;

        if (nandraw_ctrl->num_partition == 1)
        {
            nandraw_ctrl->pagesize  = mtdinfo.writesize;
            nandraw_ctrl->blocksize = mtdinfo.erasesize;
            nandraw_ctrl->pagemask  = (mtdinfo.writesize - 1);
            nandraw_ctrl->blockmask = (mtdinfo.erasesize - 1);
            nandraw_ctrl->oobsize   = mtdinfo.oobsize;
            nandraw_ctrl->oobused    = MTNFC610_OOBSIZE_FOR_YAFFS;

            nandraw_ctrl->pageshift  = (mt_u32)offshift(mtdinfo.writesize);
            nandraw_ctrl->blockshift = (mt_u32)offshift(mtdinfo.erasesize);
        }
    }

    if (!nandraw_ctrl->num_partition)
    {
//        MT_PRINT("Can't find nand type mtd device at /dev/mtdx\n");
        free(nandraw_ctrl);
        nandraw_ctrl = NULL;
        return 0;
    }
    nandraw_ctrl->size = get_flash_total_size(MT_FLASH_TYPE_NAND_0);

    return 0;
}

void nand_raw_get_info
(
    unsigned long long *totalsize,
    unsigned long *pagesize,
    unsigned long *blocksize,
    unsigned long *oobsize,
    unsigned long *blockshift)
{
    if (!nandraw_ctrl) {
        MT_ERR_FLASH("Nandraw Control is not initialized!\n");
        return;
    }
    *totalsize  = nandraw_ctrl->size;
    *pagesize   = nandraw_ctrl->pagesize;
    *blocksize  = nandraw_ctrl->blocksize;
    *oobsize    = nandraw_ctrl->oobused;
    *blockshift = nandraw_ctrl->blockshift;
}

/*****************************************************************************/
/*
 * warning:
 *    1. if open SPI/NOR FLASH, return 0
      2. if dev_name cannot match nandraw_ctrl, return error_valid;
 */
unsigned long long nand_raw_get_start_addr(const char *dev_name, unsigned long blocksize, int *value_valid)
{
    struct mtd_partition *ptn;
    int max_partition;
    int ix;

    blocksize = blocksize;

    ptn = nandraw_ctrl->partition;

    if (0 > (max_partition = get_max_partition()))
    {
        MT_ERR_FLASH("Can't find mtd device at /dev/mtdx.\n");
        return 0;
    }

    /*as the partition start with 0, if we have x partition,the max_partition will be x-1 ,
    so in following code,we use  " ix <= max_partition" instead of " ix < max_partition"
    which be used in old SDK version. as well as , the later "if (max_partition < ix)" ,we use
     "<" instead of "=" which be used in old SDK version*/
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

int nand_raw_read
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
    int rel;
    int totalread = 0;
    int num_read  = 0;
    struct mtd_partition *ptn;
    unsigned long long offset = *start_addr;

    fd = fd;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= nandraw_ctrl->size || !len)
    {
        return -1;
    }

    if ((unsigned long)offset & nandraw_ctrl->pagemask)
    {
        MT_ERR_FLASH("start_addr should be align with pagesize(0x%lX)\n",
            nandraw_ctrl->pagesize);
        return -1;
    }

    for (ix = 0; ix < nandraw_ctrl->num_partition && len; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];
        /*lint -save -e655 */
        while ((ptn->start <= offset) && (offset < ptn->end) && len && (ptn->perm & ACCESS_RD) && ptn->fd != INVALID_FD)
        {
            //DBG_OUT("dev: \"%s\", from: 0x%llX, len: 0x%lX\n",
                //ptn->mtddev, (offset - ptn->start), len);
            if (skip_badblock)
            {
                int blockindex = (int)((offset - ptn->start) / nandraw_ctrl->blocksize);

                rel = check_skip_badblock(ptn, &blockindex, (int)nandraw_ctrl->blocksize);
                if (rel < 0)
                    return rel;

                if (rel > 0)
                {
                    /* move to start address of the block */
                    offset += (unsigned long)((mt_u32)rel << nandraw_ctrl->blockshift);
                    continue;
                }

                /* rel == 0, no bad block */
            }

            DBG_OUT(">ptn->fd=%d, len=%#lx, *start_addr=%#llx, offset=%#llx\n", ptn->fd, len, *start_addr, offset);

            if (offset - openaddr >= limit_len)
            {
                MT_ERR_FLASH("bad block cause read end(beyond limit_len =%#llx)!\n", limit_len);
                return totalread;
            }

            /* read all pages in one block */
            do
            {
                /* read one page one by one */
                num_read = (int)(len >= nandraw_ctrl->pagesize ? nandraw_ctrl->pagesize : len);
                if (lseek64(ptn->fd, (off64_t)(offset - ptn->start), SEEK_SET) != -1
                    && read(ptn->fd, buffer, (size_t)num_read) != num_read)
                {
                    MT_ERR_FLASH("read \"%s\" fail. %s\n",
                        ptn->mtddev, strerror(errno));
                    return -1;
                }

                buffer    += num_read;
                len    -= (unsigned long)num_read;
                totalread += num_read;

                if (read_oob)
                {
                    struct mtd_oob_buf oob;

                    if (len >= nandraw_ctrl->oobused)
                    {
                        oob.start  = (unsigned long)(offset - ptn->start);
                        oob.len = nandraw_ctrl->oobused;
                        oob.ptr    = (unsigned char *)buffer;

                        if (ioctl(ptn->fd, MEMREADOOB, &oob))
                        {
                            MT_ERR_FLASH("read oob \"%s\" fail. %s\n",
                                ptn->mtddev, strerror(errno));
                            return -1;
                        }

                        buffer    += nandraw_ctrl->oobused;
                        len    -= nandraw_ctrl->oobused;
                        totalread += (int)nandraw_ctrl->oobused;
                    }
                    else
                    {
                        /* read end when len less than oobsize. */
                        len = 0;
                    }
                }
                offset += (unsigned long long)num_read;

            } while (len && (offset & nandraw_ctrl->blockmask));
        }
    }
    *start_addr = offset;

    return totalread;
}
/*****************************************************************************/
/*
 * warning:
 *    1. offset should be alignment with blocksize
 *    2. if there is a bad block, len should subtract.
 */
//int nand_raw_erase(unsigned long long offset, unsigned long long len, unsigned long long limit_len)
long long nand_raw_erase
(
    int fd,
    unsigned long long start_addr,
    unsigned long long len,
    unsigned long long openaddr,
    unsigned long long limit_len)
{
    int ix;
    int rel;
    long long totalerase = 0;
    unsigned long long offset = start_addr;
    int blockindex;
    struct mtd_partition *ptn;

    fd = fd;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= nandraw_ctrl->size || !len)
    {
        return -1;
    }

    if (((unsigned long)offset & nandraw_ctrl->blockmask)
        || ((unsigned long)len & nandraw_ctrl->blockmask))
    {
        MT_ERR_FLASH("offset or len should be alignment with blocksize(0x%X)\n",
            (unsigned int)nandraw_ctrl->blocksize);
        return -1;
    }

    if (offset + len > nandraw_ctrl->size)
        len = nandraw_ctrl->size - offset;

    for (ix = 0; ix < nandraw_ctrl->num_partition && len; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];

        if (ptn->readonly)
        {
            MT_ERR_FLASH("erase a read only partition \"%s\".\n", ptn->mtddev);
            return -1;
        }

        while ((ptn->start <= offset) && (offset < ptn->end) && len && (ptn->perm & ACCESS_WR) && ptn->fd != INVALID_FD)
        {
            //DBG_OUT("dev: \"%s\", from: 0x%llX, len: 0x%llX\n",
                //ptn->mtddev, (offset - ptn->start), len);
            if (offset - openaddr >= limit_len)
            {
                MT_ERR_FLASH("1bad block cause erase end(beyond limit_len =%#llx)!\n", limit_len);
                return totalerase;
            }

            blockindex = (int)((offset - ptn->start) >> nandraw_ctrl->blockshift);

            rel = check_skip_badblock(ptn, &blockindex, (int)nandraw_ctrl->blocksize);

            /* no bad block */
            if (!rel)
            {
                struct erase_info_user64 eraseinfo;

                eraseinfo.start  = (unsigned long long)(offset - ptn->start);
                eraseinfo.len = (unsigned long long)(nandraw_ctrl->blocksize);

                DBG_OUT(">ptn->fd=%d, len=%#llx, *start_addr=%#llx, offset=%#llx\n", ptn->fd, len, start_addr, offset);

                /* the block will be marked bad when erase error, so don't deal with */
                if (ioctl(ptn->fd, MEMERASE64, &eraseinfo))
                {
                    MT_ERR_FLASH("Erase 0x%llx failed!\n", offset);
                    if ((rel = nand_mark_badblock(offset, (unsigned long long)nandraw_ctrl->blocksize)) != 0)
                    {
                        MT_ERR_FLASH("\nMTD block_markbad at 0x%08llx failed: %d, aborting\n",
                                offset, rel);
                        return MT_FAILURE;
                    }
                }
                rel = 1;
                len -=   nandraw_ctrl->blocksize;
                totalerase +=  (long long)nandraw_ctrl->blocksize;
            }

            if (rel < 0)
                return MT_FAILURE;

            /* rel > 0 */
            offset += (unsigned long)((mt_u32)rel << nandraw_ctrl->blockshift);
        }
    }
    return totalerase;
}

/*****************************************************************************/
/*
 * warning:
 *    1. offset should be alignment with blocksize
 */
int nand_raw_force_erase(unsigned long long offset)
{
    int ix;
    int rel;
    struct mtd_partition *ptn;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= nandraw_ctrl->size)
    {
        return 0;
    }

    if ((unsigned long)offset & nandraw_ctrl->blockmask)
    {
        MT_ERR_FLASH("offset should be alignment with blocksize(0x%X)\n",
            (unsigned int)nandraw_ctrl->blocksize);
        return -1;
    }

    for (ix = 0; ix < nandraw_ctrl->num_partition; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];

        if (ptn->readonly)
        {
            MT_ERR_FLASH("erase a read only partition \"%s\".\n", ptn->mtddev);
            return -1;
        }

        if ((ptn->start <= offset) && (offset < ptn->end) && (ptn->perm & ACCESS_WR) && ptn->fd != INVALID_FD)
        {
            DBG_OUT("dev: \"%s\", from: 0x%llX\n",
                ptn->mtddev, (offset - ptn->start));

            offset =offset - ptn->start;

            rel = ioctl(ptn->fd, MEMFORCEERASEBLOCK, &offset);
            if (rel)
            {
                MT_ERR_FLASH("Force Erase 0x%llx failed!\n", offset);
                return -1;
            }
        }
    }
    return 0;
}
/*****************************************************************************/
/*
 * warning:
 *    1. start_addr should be alignment with pagesize
 */
int nand_raw_write
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
    int rel;
    int totalwrite = 0;
    int num_write = 0;
    int blockindex;
    unsigned char *databuf = NULL;
    unsigned char *oobbuf  = NULL;
    int ret = -1;

    /* for ioctl cmd args */
    struct mtd_write_req write_req_buf;
    unsigned long long start;
    unsigned long long datalen;
    unsigned long long ooblen;
    unsigned char *data_ptr;
    unsigned char *oob_ptr;

    struct mtd_partition *ptn;
    unsigned long long offset = *start_addr;

    fd = fd;

    databuf = malloc(NAND_PAGE_BUF_SIZE);
    if (NULL == databuf)
    {
        ret = -1;
        goto fail;
    }
    memset(databuf, 0, NAND_PAGE_BUF_SIZE);

    oobbuf = malloc(NAND_OOB_BUF_SIZE);
    if (NULL == oobbuf)
    {
        ret = -1;
        goto fail;
    }
    memset(oobbuf, 0, NAND_OOB_BUF_SIZE);

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        ret = -1;
        goto fail;
    }

    if (offset >= nandraw_ctrl->size || !len)
    {
        ret = -1;
        goto fail;
    }

    if ((unsigned long)offset & nandraw_ctrl->pagemask)
    {
        MT_ERR_FLASH("start_addr should be alignment with pagesize(0x%lX)\n",
            nandraw_ctrl->pagesize);
        ret = -1;
        goto fail;

    }

    /* if write_oob is used, align with (oobsize + pagesize) */
    if (write_oob && (len % (nandraw_ctrl->pagesize + nandraw_ctrl->oobused)))
    {
        MT_ERR_FLASH("len should be alignment with pagesize + oobsize when write oob.\n");
        ret = -1;
        goto fail;

    }

    if (!write_oob)
    {
        if (NAND_OOB_BUF_SIZE < nandraw_ctrl->oobused)
        {
            MT_ERR_FLASH("BUG program need enough oobbuf.\n");
            ret = -1;
            goto fail;
        }
        memset(oobbuf, 0xFF, NAND_OOB_BUF_SIZE);
    }

    if (NAND_PAGE_BUF_SIZE < nandraw_ctrl->pagesize)
    {
        MT_ERR_FLASH("BUG program need enough databuf.\n");
        ret = -1;
        goto fail;
    }

    for (ix = 0; ix < nandraw_ctrl->num_partition && len; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];
        if (ptn->readonly)
        {
            MT_ERR_FLASH("Write a read only partition \"%s\".\n", ptn->mtddev);
            ret = -1;
            goto fail;
        }

        while ((ptn->start <= offset) && (offset < ptn->end) && len && (ptn->perm & ACCESS_WR) && ptn->fd != INVALID_FD)
        {
            /* must skip bad block when write */
            blockindex = (int)((offset - ptn->start) >> nandraw_ctrl->blockshift);

            rel = check_skip_badblock(ptn, &blockindex, (int)nandraw_ctrl->blocksize);
            if (rel < 0) {
                ret = rel;
                goto fail;
            }

            if (rel > 0)
            {
                /* move to start address of the block */
                offset += (unsigned long)((mt_u32)rel << nandraw_ctrl->blockshift);
                continue;
            }

            /* rel == 0, no bad block */

            DBG_OUT(">ptn->fd=%d, len=%#lx, *start_addr=%#llx, offset=%#llx\n", ptn->fd, len, *start_addr, offset);

            if (offset - openaddr >= limit_len)
            {
                MT_ERR_FLASH("bad block cause write end(beyond limit_len =%#llx)!\n", limit_len);
                ret = MT_FLASH_END_DUETO_BADBLOCK;
                goto fail;
            }

            /* write all pages in one block */
            do
            {
                num_write = (int)(len >= nandraw_ctrl->pagesize ? nandraw_ctrl->pagesize : len);

                start = offset - ptn->start;
                if ((mt_u32)num_write < nandraw_ctrl->pagesize)
                {
                    /* less than one pagesize */
                    memset(databuf, 0xFF, NAND_PAGE_BUF_SIZE);
                    memcpy(databuf, buffer, (size_t)num_write);
                    data_ptr = databuf;
                }
                else
                    data_ptr = buffer;

                datalen = nandraw_ctrl->pagesize;

                offset     += (unsigned long long)num_write;
                len     -= (mt_u32)num_write;
                buffer     += num_write;
                totalwrite += num_write;

                if (write_oob) /* if write_oob */
                {
                    if (len < nandraw_ctrl->oobused)
                    {
                        MT_ERR_FLASH("buf not align error!\n");
                        ret = -1;
                        goto fail;

                    }
                    oob_ptr = buffer;

                    buffer     += nandraw_ctrl->oobused;
                    len     -= nandraw_ctrl->oobused;
                    totalwrite += (int)nandraw_ctrl->oobused;
                }
                else
                    oob_ptr = oobbuf;

                ooblen = nandraw_ctrl->oobused;

                /* avoid mark bad block unexpected. */
                if ((*(oob_ptr + 1) << 8) + *oob_ptr != 0xFFFF)
                {
                    MT_ERR_FLASH("Please check input data, it maybe mark bad block. value:0x%04X\n",
                        (*(oob_ptr + 1) << 8) + *oob_ptr);
                    ret = -1;
                    goto fail;
                }

                /* should reerase and write if write error when upgrade. */
                memset(&write_req_buf, 0xff, sizeof(write_req_buf));
                write_req_buf.start = start;
                write_req_buf.usr_data = (unsigned long long)((size_t)((void *)data_ptr));
                write_req_buf.len = datalen;
                write_req_buf.usr_oob = (unsigned long long)((size_t)((void *)oob_ptr));
                write_req_buf.ooblen = ooblen;
                write_req_buf.mode = MTD_OPS_PLACE_OOB;

                if (ioctl(ptn->fd, MEMWRITE, &write_req_buf) != 0)
                {
                        MT_ERR_FLASH("ioctl(%s MEMWRITE) fail. %s\n",
                                 ptn->mtddev, strerror(errno));
                        /* union return value to MT_FAILURE.*/
                        ret = MT_FAILURE;
                        goto fail;
                }
            } while (len && (offset & nandraw_ctrl->blockmask));
        }
    }

    *start_addr = offset;
    ret = totalwrite;

done:
    if (databuf)
        free(databuf);
    if (oobbuf)
        free(oobbuf);
    return ret;
fail:
    goto done;
}
/*****************************************************************************/
/*
 * warning:
 *    1. offset and len should be alignment with blocksize
 */
int nand_mark_badblock(unsigned long long offset, unsigned long long len)
{
    int ix;
    unsigned long long blockoffset;
    struct mtd_partition *ptn;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= nandraw_ctrl->size || !len)
    {
        return 0;
    }

    if (((unsigned long)offset & nandraw_ctrl->blockmask)
        || ((unsigned long)len & nandraw_ctrl->blockmask))
    {
        MT_ERR_FLASH("offset or len should be alignment with blocksize(0x%X)\n",
            (unsigned int)nandraw_ctrl->blocksize);
        return -1;
    }

    if (offset + len > nandraw_ctrl->size)
        len = nandraw_ctrl->size - offset;

    for (ix = 0; ix < nandraw_ctrl->num_partition && len; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];

        while ((ptn->start <= offset) && (offset < ptn->end) && len && ptn->fd != INVALID_FD)
        {
            if (ptn->readonly)
            {
                MT_ERR_FLASH("mark bad block error, a read only partition \"%s\".\n", ptn->mtddev);
                return -1;
            }

            DBG_OUT("dev: \"%s\", from: 0x%llX, len: 0x%llX\n",
                ptn->mtddev, (offset - ptn->start), len);

            blockoffset = offset - ptn->start;

            if (ioctl(ptn->fd, MEMSETBADBLOCK, &blockoffset))
            {
                MT_ERR_FLASH("Mark bad block 0x%llX failed!\n", offset);
            }

            offset += (unsigned long)(1UL << nandraw_ctrl->blockshift);
            len -= (unsigned long)(1UL << nandraw_ctrl->blockshift);
        }
    }
    return 0;
}
/*****************************************************************************/
/*
 * warning:
 *    1. offset and len should be alignment with blocksize
 */
int nand_show_badblock(unsigned long long offset, unsigned long long len)
{
    int ix;
    int badblock;
    unsigned long long blockoffset;
    struct mtd_partition *ptn;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (offset >= nandraw_ctrl->size || !len)
    {
        return 0;
    }

    if (((unsigned long)offset & nandraw_ctrl->blockmask)
        || ((unsigned long)len & nandraw_ctrl->blockmask))
    {
        MT_ERR_FLASH("offset or len should be alignment with blocksize(0x%X)\n",
            (unsigned int)nandraw_ctrl->blocksize);
        return -1;
    }

    if (offset + len > nandraw_ctrl->size)
        len = nandraw_ctrl->size - offset;

    for (ix = 0; ix < nandraw_ctrl->num_partition && len; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];

        while ((ptn->start <= offset) && (offset < ptn->end) && len && ptn->fd != INVALID_FD)
        {
            DBG_OUT("dev: \"%s\", from: 0x%llX, len: 0x%llX\n",
                ptn->mtddev, (offset - ptn->start), len);

            blockoffset = offset - ptn->start;

            if ((badblock = ioctl(ptn->fd, MEMGETBADBLOCK, &blockoffset)) < 0)
            {
                MT_ERR_FLASH("Get nand badblock fail. %s\n", strerror(errno));
                return -1;
            }
            if (badblock == 1)
            {
                MT_ERR_FLASH("Bad block at address: 0x%lX of \"%s\", absolute address: 0x%llX\n",
                    (unsigned long)blockoffset, ptn->mtddev, (unsigned long long)offset);
            }

            offset += (unsigned long)(1UL << nandraw_ctrl->blockshift);
            len -= (unsigned long)(1UL << nandraw_ctrl->blockshift);
        }
    }
    return 0;
}
/*****************************************************************************/

int nand_raw_info(struct mtd_info_user *mtdinfo)
{
    int rel;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    if (nandraw_ctrl->partition[0].fd == INVALID_FD)
        return -1;
    rel = ioctl(nandraw_ctrl->partition[0].fd, MEMGETINFO, mtdinfo);

    if (rel)
    {
        MT_ERR_FLASH("ioctl \"%s\" fail. %s\n",
                nandraw_ctrl->partition[0].mtddev, strerror(errno));
        return rel;
    }

    if (nandraw_ctrl->size > 0xFFFFFFFF)
    {
        MT_INFO_FLASH("nand flash size out of range of an unsigned long.\n");
    }
    mtdinfo->size = (unsigned long)nandraw_ctrl->size;

    return 0;
}
/*****************************************************************************/

int nand_raw_dump_partition(void)
{
    int ix;
    struct mtd_partition *ptn;

    if (!nandraw_ctrl)
    {
        MT_ERR_FLASH("Please initialize before use this function.\n");
        return -1;
    }

    printf("-------------------------\n");
    printf("mtd device   start len mode\n");
    for (ix = 0; ix < nandraw_ctrl->num_partition; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];

        printf("%-12s ", ptn->mtddev);
        printf("%5s ", int_to_size(ptn->start));
        printf("%6s ", int_to_size(ptn->end + 1 - ptn->start));
        printf("%2s ", ptn->readonly ? "r" : "rw");
        printf("\n");
    }

    return 0;
}
/*****************************************************************************/

int nand_raw_destroy(void)
{
    int ix;

    if (!nandraw_ctrl)
        return 0;

    for (ix = 0; ix < nandraw_ctrl->num_partition; ix++)
    {
        if( INVALID_FD != nandraw_ctrl->partition[ix].fd)
        {
            close(nandraw_ctrl->partition[ix].fd);
        }
    }

    if( NULL != nandraw_ctrl)
    {
        free(nandraw_ctrl);
    }
    nandraw_ctrl = NULL;
    //DBG_OUT("\n");

    return 0;
}

/*****************************************************************************/
/*
 *  > 0 skip bad block num.
 *  = 0 no bad block.
 *  < 0 error.
 */
static int check_skip_badblock(struct mtd_partition *ptn, int *blockindex,
    int blocksize)
{
    int rel = 0;
    int badblock;
    unsigned long long size = (ptn->end - ptn->start) + 1;
    loff_t offset = ((loff_t)(*blockindex) * (loff_t)blocksize);

    if (ptn->fd == INVALID_FD)
        return -1;
    do
    {
        if ((badblock = ioctl(ptn->fd, MEMGETBADBLOCK, &offset)) < 0)
        {
            MT_ERR_FLASH("Get nand badblock fail. %s\n", strerror(errno));
            return -1;
        }
        if (badblock == 1)
        {
            MT_INFO_FLASH("Skip bad block at address: 0x%llX of \"%s\", absolute address: 0x%llX\n",
                offset, ptn->mtddev, ((unsigned long long)offset + ptn->start));
            (*blockindex)++;
            rel++;
        }
        offset = (loff_t)(*blockindex) * (loff_t)blocksize;
    } while (badblock == 1 && offset < (loff_t)size);

    return rel;
}

//MT_PRINT("\t> ptn->start=%#llx,end=%#llx,endaddr=%#llx,offset_addr=%#llx, offset=%#x, ptn->fd=%d\n",
    //ptn->start, ptn->end, endaddr, offset_addr, (unsigned int)offset, ptn->fd);
int nand_raw_get_physical_index(unsigned long long start_addr, int *blockindex, int blocksize)
{
    struct mtd_partition *ptn;
    int badblock;
    int ix, i = 0;

    loff_t offset = (unsigned long)0;
    unsigned long long offset_addr = start_addr;
    int logcial_index = *blockindex;
    int physical_index = 0;

    for (ix = 0; i < logcial_index && ix < nandraw_ctrl->num_partition; ix++)
    {
        ptn = &nandraw_ctrl->partition[ix];

        if (ptn->end + 1 <= start_addr)
        {
            continue;
        }

        if (ptn->fd == INVALID_FD)
            return -1;
        while ((i < logcial_index) && (offset_addr < ptn->end))
        {
            offset = (loff_t)(offset_addr - ptn->start);

            if ((badblock = ioctl(ptn->fd, MEMGETBADBLOCK, &offset)) < 0)
            {
                MT_ERR_FLASH("Get nand badblock fail. %s\n", strerror(errno));
                return -1;
            }
            if (0 == badblock)
            {
                //MT_PRINT("> Skip bad block at address: 0x%lX of \"%s\", absolute address: 0x%llX\n",
                //(unsigned long)offset, ptn->mtddev, (unsigned long long)(offset + ptn->start));
                i++;
            }
            physical_index++;
            offset_addr += (unsigned long long)blocksize;
        }
    }

    DBG_OUT("logcial_addr=%d, physical_addr=%d\n", logcial_index, physical_index);
    //(unsigned long long)physical_index << nandraw_ctrl->blockshift, physical_index);
    *blockindex = physical_index;

    return 0;
}

/*****************************************************************************/

