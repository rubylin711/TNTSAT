/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_DRV_DUMP_H__
#define __MT_DRV_DUMP_H__

#include "mt_type.h"

typedef enum {
    E_DUMP_TYPE_FILE,   /* dump to file */
    E_DUMP_TYPE_MEM,    /* dump to memory */
    E_DUMP_TYPE_MAX
} DATA_DUMP_TYPE;

mt_void *mt_drv_dump_create(DATA_DUMP_TYPE type, mt_u8 *target, mt_s32 size);

mt_s32 mt_drv_dump_destroy(mt_void *handle);

mt_s32 mt_drv_dump_do(mt_void *handle, const mt_u8 *data, mt_s32 size);

#endif
