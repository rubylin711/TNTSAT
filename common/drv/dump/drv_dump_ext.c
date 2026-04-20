/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>
#include <linux/slab.h>

#include "mt_type.h"
#include "mt_drv_dump.h"

typedef struct {
    DATA_DUMP_TYPE dump_type;
    mt_u8 target_file[64];  /* for file dump */
    mt_u8 *target_addr;     /* for memory dump */

#ifdef __KERNEL__
    struct file *fp;
    loff_t       pos;
#else
    FILE *fp;
#endif

    mt_s32  target_size;
    mt_s32  dumped_size;
} DATA_DUMP_S;

#ifdef __KERNEL__
#define DUMP_PRINT(fmt, args...) printk(KERN_WARNING fmt, ##args)
#else
#define DUMP_PRINT(fmt, args...) printf(fmt, ##args)
#endif

mt_void *mt_drv_dump_create(DATA_DUMP_TYPE type, mt_u8 *target, mt_s32 size)
{
    DATA_DUMP_S *h_dump;

    if(target == MT_NULL || (type >= E_DUMP_TYPE_MAX))
        return MT_NULL;

    if(type == E_DUMP_TYPE_FILE && strlen(target) <= 0) {
        DUMP_PRINT("invalid file path\n");
        return MT_NULL;
    }

    if(type == E_DUMP_TYPE_MEM && size <= 0) {
        DUMP_PRINT("invalid target size for dump %d\n", size);
        return MT_NULL;
    }

#ifdef __KERNEL__
    h_dump = kmalloc(sizeof(DATA_DUMP_S), GFP_KERNEL);
#else
    h_dump = malloc(sizeof(DATA_DUMP_S));
#endif
    if(h_dump == MT_NULL) {
        DUMP_PRINT("alloc memory failed to create data dump instance\n");
        return MT_NULL;
    }
    memset(h_dump, 0, sizeof(DATA_DUMP_S));

    if(type == E_DUMP_TYPE_FILE) {
        int slen = 0;

        strncpy(h_dump->target_file, target, sizeof(h_dump->target_file));
        h_dump->fp = filp_open(h_dump->target_file, O_RDWR | O_CREAT, 0644);

        slen = strlen(h_dump->target_file);
        if(IS_ERR(h_dump->fp)) {
            DUMP_PRINT("open %s for write failed, path len:%d.\n", h_dump->target_file, slen);
            kfree(h_dump);
            return MT_NULL;
        }
        DUMP_PRINT("create file %s for dump\n", h_dump->target_file);
    } else if(type == E_DUMP_TYPE_MEM) {
        h_dump->target_addr = target;
        memset(target, 0, size);
        DUMP_PRINT("memset memory from %p for dump, size %d\n", h_dump->target_addr, size);
    }

    h_dump->dump_type   = type;
    h_dump->target_size = size;

    return h_dump;
}

mt_s32 mt_drv_dump_destroy(mt_void *handle)
{
    DATA_DUMP_S *h_dump = (DATA_DUMP_S *)handle;

    if(h_dump == MT_NULL)
        return MT_FAILURE;

    if(h_dump->dump_type == E_DUMP_TYPE_FILE) {
        DUMP_PRINT("End of dump, dumped size: %d, path: %s\n", h_dump->dumped_size, h_dump->target_file);
    } else {
        DUMP_PRINT("End of dump, dumped size: %d, addr: %p\n", h_dump->dumped_size, h_dump->target_addr);
    }

#ifdef __KERNEL__
    if(h_dump->fp != MT_NULL) {
        filp_close(h_dump->fp, MT_NULL);
        h_dump->fp = MT_NULL;
    }
    kfree(h_dump);
#else
    if(h_dump->fp != MT_NULL) {
        fclose(h_dump->fp);
    }
    free(h_dump);
#endif

    return MT_SUCCESS;
}

mt_s32 mt_drv_dump_do(mt_void *handle, const mt_u8 *data, mt_s32 size)
{
    DATA_DUMP_S *h_dump = (DATA_DUMP_S *)handle;
    mt_s32 target_size = 0;
    mt_s32 ret = 0;

    if(h_dump == MT_NULL || data == MT_NULL || size < 0)
        return MT_FAILURE;

    target_size = h_dump->dumped_size + size;
    if(h_dump->target_size > 0 && target_size > h_dump->target_size) {
        return MT_SUCCESS;
    }

    if(h_dump->dump_type == E_DUMP_TYPE_FILE) {
#ifdef __KERNEL__
        if(h_dump->fp == MT_NULL) {
           return MT_FAILURE;
        }
        ret = kernel_write(h_dump->fp, data, size, &h_dump->pos);
#else
        if(h_dump->fp == MT_NULL) {
            h_dump->fp = fopen(h_dump->target_file, "wb+");
            if(h_dump->fp == MT_NULL) {
                DUMP_PRINT("open %s failed for write\n", h_dump->target_file);
                return MT_FAILURE;
            }
            DUMP_PRINT("open file %s for dump\n", h_dump->target_file);
        }

        ret = fwrite(data, 1, size, h_dump->fp);
#endif

        h_dump->dumped_size += ret;
        if(ret != size) {
            DUMP_PRINT("write error, expect write size %d, actual write size %d\n", size, ret);
        } else {
            DUMP_PRINT("dump src %p, size %d, dumped %d\n", data, ret, h_dump->dumped_size);
        }
    } else {
        memcpy(h_dump->target_addr + h_dump->dumped_size, data, size);
        h_dump->dumped_size += size;
        ret = size;
        DUMP_PRINT("dump src %p, size %d, dumped %d\n", data, ret, h_dump->dumped_size);
    }

    return ret;
}

EXPORT_SYMBOL(mt_drv_dump_create);
EXPORT_SYMBOL(mt_drv_dump_destroy);
EXPORT_SYMBOL(mt_drv_dump_do);
