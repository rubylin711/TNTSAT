/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __PVR_DEBUG_H__
#define __PVR_DEBUG_H__

#include "mt_debug.h"
#ifdef JQW
#ifndef MT_FATAL_PVR
#define MT_FATAL_PVR(fmt...)        MT_FATAL_PRINT  (MT_ID_PVR, fmt)
#endif
#ifndef MT_ERR_PVR
#define MT_ERR_PVR(fmt...)          MT_ERR_PRINT    (MT_ID_PVR, fmt)
#endif
#ifndef MT_WARN_PVR
#define MT_WARN_PVR(fmt...)         MT_WARN_PRINT   (MT_ID_PVR, fmt)
#endif
#ifndef MT_INFO_PVR
#define MT_INFO_PVR(fmt...)         MT_INFO_PRINT   (MT_ID_PVR, fmt)
#endif
#else
#ifndef MT_FATAL_PVR
#define MT_FATAL_PVR(fmt...)        printf  (fmt)
#endif
#ifndef MT_ERR_PVR
#define MT_ERR_PVR(fmt...)          printf    (fmt)
#endif
#ifndef MT_WARN_PVR
#define MT_WARN_PVR(fmt...)         printf   (fmt)
#endif
#ifndef MT_INFO_PVR
#define MT_INFO_PVR(fmt...)         //printf   (fmt)
#endif
#ifndef PRV_PRINTF
#define PRV_PRINTF(fmt...)         //printf   (fmt)
#endif
#endif
#endif

