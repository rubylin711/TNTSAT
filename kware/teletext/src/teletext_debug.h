/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __TELETEXT_DEBUG_H__
#define __TELETEXT_DEBUG_H__

#include "mt_debug.h"

#define MT_FATAL_TTX(fmt...)      MT_FATAL_PRINT(MT_ID_TTX, fmt)
#define MT_ERR_TTX(fmt...)        MT_ERR_PRINT(MT_ID_TTX, fmt)
#define MT_WARN_TTX(fmt...)       MT_WARN_PRINT(MT_ID_TTX, fmt)
#define MT_INFO_TTX(fmt...)       MT_INFO_PRINT(MT_ID_TTX, fmt)
#endif
