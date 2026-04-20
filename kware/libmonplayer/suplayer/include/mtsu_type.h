/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MTSU_TYPE_H__
#define __MTSU_TYPE_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/*----------------------------------------------*
 * 操作系统定义，默认是Linux
 *----------------------------------------------*/

#if !defined(MT_OS_LINUX)
#define MT_OS_LINUX         1
#endif
#if !defined(MT_OS_WIN32)
#define MT_OS_WIN32         2
#endif
#if !defined(MT_OS_TYPE)
#define MT_OS_TYPE          MT_OS_LINUX
#endif
#ifndef __FUNCTION__
//#define __FUNCTION__        ""
#endif

/*----------------------------------------------*
 * 数据类型定义，应用层和内核代码均使用         *
 *----------------------------------------------*/
#if 0
typedef unsigned char       MT_U8;
typedef unsigned char       MT_UCHAR;
typedef unsigned short      MT_U16;
typedef unsigned int        MT_U32;
#if MT_OS_TYPE == MT_OS_LINUX
typedef unsigned long long  MT_U64;
#elif MT_OS_TYPE == MT_OS_WIN32
typedef unsigned __int64    MT_U64;
#endif

typedef char                MT_S8;
typedef short               MT_S16;
typedef int                 MT_S32;
typedef long                MT_LONG;
typedef unsigned long       MT_UL;
#endif
#if MT_OS_TYPE == MT_OS_LINUX
typedef  long long          MT_S64;
#elif MT_OS_TYPE == MT_OS_WIN32
typedef  __int64            MT_S64;
#endif
#if 0
typedef char                MT_CHAR;
typedef char*               MT_PCHAR;

typedef float               MT_FLOAT;
typedef double              MT_DOUBLE;

typedef void                MT_VOID;


typedef MT_U64              MT_PTS_TIME;
typedef unsigned long       MT_SIZE_T;
typedef unsigned long       MT_LENGTH_T;

typedef int                 STATUS;

typedef enum 
{
    MT_FALSE    = 0,
    MT_TRUE     = 1
} MT_BOOL;

#ifndef __MT_HANDLE__
#define __MT_HANDLE__
typedef MT_U32 MT_HANDLE;
#endif
typedef MT_U32 MT_HANDLE;
#ifndef NULL
#define NULL                (0U)
#endif

#define MT_NULL             (0U)
#define MT_NULL_PTR         (0U)

#define MT_SUCCESS          (0)
#define MT_FAILURE          (-1)
#endif

#define MT_LITTLE_ENDIAN    (1234)
#define MT_BIG_ENDIAN       (4321)
#define mtsu_malloc malloc
#define mtsu_free free
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __MTSU_TYPE_H__ */

