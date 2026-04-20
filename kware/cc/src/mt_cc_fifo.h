/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __CC_FIFO_HEAD_FILE__
#define __CC_FIFO_HEAD_FILE__

#include "string.h"
#include <sys/time.h>
#include "mt_type.h"
#include <stdlib.h>
#include <stdio.h>



#define CC_FIFO_TOTAL_LENGTH    (4*1024)
typedef struct
{
    //mt_u32 cc_enable;
    mt_u32 fifo_read_p;
    mt_u32 fifo_write_p;
    //mt_u32 fifo_write_p_pre;
    mt_u8 fifo_buff[CC_FIFO_TOTAL_LENGTH];
} cc_fifo_inserter_struct;

typedef struct
{
    MT_BOOL  used;
    mt_u8 cc_type[2];
    mt_u8 cc_data1[2];
    mt_u8 cc_data2[2];
} cc_data_struct;

mt_s32 cc_fifo_init(void);
mt_s32 cc_fifo_deinit(void);

mt_s32 cc_fifo_put(mt_u8 *data, mt_u32 length);
mt_s32 cc_fifo_get(mt_u8 *data, mt_u32* length);

mt_s32 cc_fifo_clear(void);

#endif //__CC_FIFO_HEAD_FILE__
