#ifndef __SUBTITLE_DEBUG_H__
#define __SUBTITLE_DEBUG_H__

#include "mt_debug.h"

#define MT_FATAL_SUBT(fmt...)      MT_FATAL_PRINT(MT_ID_SUBT, fmt)
#define MT_ERR_SUBT(fmt...)        MT_ERR_PRINT(MT_ID_SUBT, fmt)
#define MT_WARN_SUBT(fmt...)       MT_WARN_PRINT(MT_ID_SUBT, fmt)
#define MT_INFO_SUBT(fmt...)       MT_INFO_PRINT(MT_ID_SUBT, fmt)
#endif
