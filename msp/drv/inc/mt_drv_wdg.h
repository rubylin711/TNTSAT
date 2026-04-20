/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _MT_DRV_WDG_H
#define _MT_DRV_WDG_H

#include "mt_debug.h"

#define MT_FATAL_WDG(fmt...) \
    MT_FATAL_PRINT(MT_ID_WDG, fmt)

#define MT_ERR_WDG(fmt...) \
    MT_ERR_PRINT(MT_ID_WDG, fmt)

#define MT_WARN_WDG(fmt...) \
    MT_WARN_PRINT(MT_ID_WDG, fmt)

#define MT_INFO_WDG(fmt...) \
    MT_INFO_PRINT(MT_ID_WDG, fmt)

#define MT_WDG_NUM  (1)


mt_s32 wdg_drv_modinit(mt_void);
mt_void wdg_drv_modexit(mt_void);

#endif  /* ifndef _MT_DRV_WDG_H */
