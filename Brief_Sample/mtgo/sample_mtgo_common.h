/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __SAMPLE_MTGO_COMMON_H__
#define __SAMPLE_MTGO_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#include "mt_type.h"

#ifdef MT_SAMPLE_DEBUG 

#define MTGO_PRINT   printf
#else

#define MTGO_PRINT 

#endif

#define SAMPLE_MTGO_FUNCTION_ENTER()	        MTGO_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_MTGO_FUNCTION_EXIT()		        MTGO_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_MTGO_FATAL_PRINT(fmt...) 		MTGO_PRINT("[FATAL]" fmt)
#define SAMPLE_MTGO_ERR_PRINT(fmt...)		    MTGO_PRINT("[ERROR]" fmt)
#define SAMPLE_MTGO_WARN_PRINT(fmt...)		    MTGO_PRINT("[WARN]"  fmt)
#define SAMPLE_MTGO_INFO_PRINT(fmt...)		    MTGO_PRINT("[INFO]"  fmt)
#define SAMPLE_MTGO_DBG_PRINT(fmt...)			MTGO_PRINT("[DEBUG]" fmt)


/*!
@brief Display initialization.
@param[in] tv_sys        tv format
@return::MT_SUCCESS
@return::MT_FALSE
@*/
mt_s32 Sample_MTGO_Display_Init(mt_u32 tv_sys);


/*!
@brief Display deinitializes.
@return::MT_SUCCESS
@return::MT_FALSE
@*/
mt_s32 Sample_MTGO_Display_DeInit(void);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __SAMPLE_MTGO_COMM_H__ */
