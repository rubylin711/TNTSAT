/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __SAMPLE_CC_COMMON_H__
#define __SAMPLE_CC_COMMON_H__


#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#ifdef  MT_SAMPLE_CC_DEBUG 
#define MT_CC_PRINT   printf
#else
#define MT_CC_PRINT 
#endif

#define SAMPLE_CC_FUNCTION_ENTER()  	        MT_CC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_CC_FUNCTION_EXIT()		        MT_CC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)
 
#define SAMPLE_CC_FATAL_PRINT(fmt...) 		    MT_CC_PRINT(" [FATAL] " fmt)
#define SAMPLE_CC_ERR_PRINT(fmt...)		        MT_CC_PRINT(" [ERROR] " fmt)
#define SAMPLE_CC_WARN_PRINT(fmt...)		    MT_CC_PRINT(" [WARN] "  fmt)
#define SAMPLE_CC_INFO_PRINT(fmt...)		    MT_CC_PRINT(" [INFO] "  fmt)
#define SAMPLE_CC_DBG_PRINT(fmt...)			    MT_CC_PRINT(" [DEBUG] " fmt)

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __SAMPLE_MTGO_COMM_H__ */
