/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _MT_DRV_GFX2D_H_
#define _MT_DRV_GFX2D_H_

#include <linux/ioctl.h>

#define GFX2D_IOC_MAGIC 'G'

typedef struct
{
    MT_GFX2D_DEV_ID_E enDevId;
    MT_GFX2D_COMPOSE_LIST_S *pstComposeList;
    MT_GFX2D_SURFACE_S *pstDst;
    MT_BOOL bSync;
    mt_u32 u32Timeout;
}GFX2D_COMPOSE_CMD_S;

typedef struct
{
    MT_GFX2D_DEV_ID_E enDevId;
    mt_u32 u32Timeout;
}GFX2D_WAITALLDONE_CMD_S;

#define GFX2D_COMPOSE _IOW(GFX2D_IOC_MAGIC, 1, GFX2D_COMPOSE_CMD_S)
#define GFX2D_WATIALLDONE _IOW(GFX2D_IOC_MAGIC, 2, GFX2D_WAITALLDONE_CMD_S)

#endif
