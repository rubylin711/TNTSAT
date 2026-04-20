
#ifndef __DEMUX_DEBUG_H__
#define __DEMUX_DEBUG_H__

#ifndef USE_LOG_OUT
#define USE_LOG_OUT 1
#endif

#include "mt_debug.h"
#ifdef USE_LOG_OUT
#define MT_FATAL_DEMUX(fmt...)      MT_FATAL_PRINT  (MT_ID_DEMUX, fmt)
#define MT_ERR_DEMUX(fmt...)          MT_ERR_PRINT    (MT_ID_DEMUX, fmt)
#define MT_WARN_DEMUX(fmt...)      MT_ERR_PRINT    (MT_ID_DEMUX, fmt)// MT_WARN_PRINT   (MT_ID_DEMUX, fmt)
#define MT_INFO_DEMUX(fmt...)       MT_ERR_PRINT    (MT_ID_DEMUX, fmt)//MT_INFO_PRINT   (MT_ID_DEMUX, fmt)
#define MT_DBG_DEMUX(fmt...)        MT_ERR_PRINT    (MT_ID_DEMUX, fmt)//MT_DBG_PRINT   (MT_ID_DEMUX, fmt)
#else
#define MT_FATAL_DEMUX      printf
#define MT_ERR_DEMUX        printf
#define MT_WARN_DEMUX       printf
#define MT_INFO_DEMUX       printf
#define MT_DBG_DEMUX        printf
#endif
#endif

