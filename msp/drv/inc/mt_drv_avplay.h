/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : mt_drv_avplay.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2011/11/25
  Description   :
  History       :
  1.Date        : 2011/11/25
    Author      : 
    Modification: Created file

******************************************************************************/
#ifndef __MT_DRV_AVPLAY_H__
#define __MT_DRV_AVPLAY_H__

#include "mt_type.h"
#include "mt_debug.h"

#ifdef __cplusplus
#if __cplusplus
    extern "C"{
#endif
#endif

#define MT_FATAL_AVPLAY(fmt...) \
            MT_FATAL_PRINT(MT_ID_AVPLAY, fmt)

#define MT_ERR_AVPLAY(fmt...) \
            MT_ERR_PRINT(MT_ID_AVPLAY, fmt)

#define MT_WARN_AVPLAY(fmt...) \
            MT_WARN_PRINT(MT_ID_AVPLAY, fmt)

#define MT_INFO_AVPLAY(fmt...) \
            MT_INFO_PRINT(MT_ID_AVPLAY, fmt)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
