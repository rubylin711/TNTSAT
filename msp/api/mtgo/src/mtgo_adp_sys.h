/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_POSIX_H__
#define __MTGO_POSIX_H__

/* add include here */
//#include "exports.h"
#include "string.h"


#ifdef __cplusplus
extern "C" {
#endif

/***************************** Macro Definition ******************************/

#define MTGO_MemSet memset
#define MTGO_MemCopy memcpy
#define MTGO_MemCmp memcmp
#define MTGO_Strncmp strncmp

//#define MTGO_ADP_ASSERT(cond) if (!(cond)) { printf("assert failed\n");}
#define MTGO_ADP_ASSERT(cond) 

#define MTGO_ADP_LOG(level, str, args...)

//#define MTGO_ADP_SetError(errno) printf( " ERR: %x\n", (mt_u32)errno)
#define MTGO_ADP_SetError(errno) 

#if 0
#define BM_TRACE(fmt, args... )\
 do { \
            printf("%s(): Line %d : "fmt, __FUNCTION__,  __LINE__ , ##args);\
    } while (0)
#else
#define BM_TRACE(fmt, args... )
#endif
/*************************** Structure Definition ****************************/


/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/


#ifdef __cplusplus
}
#endif
#endif /* __MTGO_POSIX_H__ */


