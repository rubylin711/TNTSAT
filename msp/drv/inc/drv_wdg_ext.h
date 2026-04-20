/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_WDG_EXT_H__
#define __DRV_WDG_EXT_H__

#include "mt_type.h"
#include "mt_drv_dev.h"

mt_s32 wdg_drv_modinit(mt_void);
mt_void wdg_drv_modexit(mt_void);

typedef mt_s32 (*fn_wdg_suspend)(basedev_s* , pm_message_t );
typedef mt_s32 (*fn_wdg_resume)(basedev_s* );

typedef struct
{
    fn_wdg_suspend				   pfnWdgSuspend;
    fn_wdg_resume				   pfnWdgResume;
} WDG_EXT_FUNC_S;

#endif  /* ifndef _DRV_WDG_EXT_H */
