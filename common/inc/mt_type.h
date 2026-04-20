#ifndef __MT_TYPE_H__
#define __MT_TYPE_H__

#ifndef CONFIG_MT_MONTAGE_PLATFORM
#error "you must use -include $(INCLUDE_DIR)/autoconf-old.h in Makefile, INCLUDE_DIR is ddk/buildroot/output/host/aarch64-buildroot-linux-gnu/sysroot/usr/include/mt or ddk/buildroot/output/host/arm-buildroot-linux-gnueabihf/sysroot/usr/include/mt"
#endif

#ifndef __KERNEL__
#include <stdint.h>	/* int64_t, int32_t etc */
#include <stdbool.h>	/* bool, true, false */
#include <stddef.h>	/* NULL */
#else
#include <linux/stddef.h>	/* NULL, true, false */
#endif

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/* Description switch */
#define MT_DES(a, b)           (b)

#if MT_DES("Primary Type Defenition", 1)
/*
* Basically, redefined type should not
* exceed built-in type char numbers
*/
typedef signed char             mt_s8;
typedef unsigned char           mt_u8;
typedef char                    mt_char;
typedef unsigned char           mt_uchar;
typedef signed short            mt_s16;
typedef unsigned short          mt_u16;
typedef signed int              mt_s32;
typedef unsigned int            mt_u32;

#ifndef _M_IX86
typedef signed   long long      mt_s64;
typedef unsigned long long      mt_u64;
#else
typedef __int64                 mt_s64;
typedef unsigned __int64        mt_u64;
#endif /* _M_IX86 */

#if (defined(__cplusplus))
/*
 * C++ Standard Core Language Closed Issues, Revision 30 states:
 * If the parameter-declaration-clause is empty, the function takes no arguments.
 * The parameter list (void) is equivalent to the empty parameter list.
 *
 * Can a typedef to void be used instead of the type void in the parameter list
 * Rationale: The IS is already clear that this is not allowed.
 */
#define mt_void                 void
#else
typedef void                    mt_void;
#endif /* (defined(__cplusplus)) */

typedef char*                   mt_pchar;
typedef signed long             mt_length_t;
typedef unsigned long           mt_size_t;
typedef mt_void*                mt_handle_t;

#ifdef __KERNEL__
#include <linux/types.h>	/* phys_addr_t, u32, u16, ulong etc */
#else
# ifdef CONFIG_MT_PHYS_RANGE_BEYOND_4G
typedef mt_u64                  phys_addr_t;
# else
typedef mt_u32                  phys_addr_t;
# endif
typedef unsigned long           ulong;
#endif

/* note: arm u64 align with 8 byte (aapcs32.pdf), aarch64 u64 align with 8 byte (IHI0055B_aapcs64.pdf), x86 u64 align with 4 byte */
#if defined(CONFIG_MT_64BIT_KMODE) && defined(CONFIG_MT_32BIT_MODE)
# ifndef __KERNEL__
typedef unsigned long long      mt_kern_ulong_t;
typedef long long               mt_kern_long_t;
# else
typedef mt_u32                  mt_user_ulong_t;
typedef mt_s32                  mt_user_long_t;
# endif
#else
# ifndef __KERNEL__
typedef unsigned long           mt_kern_ulong_t;
typedef long                    mt_kern_long_t;
# else
typedef unsigned long           mt_user_ulong_t;
typedef long                    mt_user_long_t;
# endif
#endif

typedef int                     mt_bool;

#endif

#if MT_DES("Extended Type Defenition", 1)

#define MT_TRUE                 1	/* follow stdbool.h in toolchain and stddef.h in kernel */
#define MT_FALSE                0
#ifndef TRUE
#define TRUE                    MT_TRUE
#endif
#ifndef FALSE
#define FALSE                   MT_FALSE
#endif

#define SUCCESS                 0

#ifndef __KERNEL__
#define s8                      mt_s8
#define u8                      mt_u8
#define uchar                   mt_uchar
#define s16                     mt_s16
#define u16                     mt_u16
#define s32                     mt_s32
#define u32                     mt_u32
#define s64                     mt_s64
#define u64                     mt_u64
#endif

#define S8                      mt_s8
#define MT_S8                   mt_s8
#define CHAR                    mt_char
#define MT_CHAR                 mt_char
#define U8                      mt_u8
#define MT_U8                   mt_u8
#define MT_UCHAR                mt_u8
#define S16                     mt_s16
#define MT_S16                  mt_s16
#define U16                     mt_u16
#define MT_U16                  mt_u16
#define S32                     mt_s32
#define MT_S32                  mt_s32
#define U32                     mt_u32
#define MT_U32                  mt_u32
#define S64                     mt_s64
#define MT_S64                  mt_s64
#define U64                     mt_u64
#define MT_U64                  mt_u64
#define mt_float                float
#define mt_double               double
#define mt_handle               ulong
#define mt_session              ulong
#define MT_PCHAR                mt_pchar
#define MT_VOID                 mt_void
#define MT_BOOL                 mt_bool
#define MT_HANDLE               ulong
#define MT_HANDLE_T             mt_handle_t
#define MT_ErrCode              mt_u32
#define ErrorCode_t             MT_ErrCode

/*
 * DVB: PTS 33bit, in unit of 90KHz
 * Montage: DVB PTS 32bit, in unit of 45KHz
 */
#define mt_dvb_pts32            mt_u32

#endif

#define MT_NULL                 0UL
#define MT_NULL_PTR             0UL

#define MT_SUCCESS              0
#define MT_FAILURE             (-1)
#define MT_ERR_PARAM           (-3)
#define MT_ERR_NORES           (-10)
#define INVALID                (~0)

#define MT_INVALID_HANDLE      (0xffffffff)
#define MT_INVALID_PTS         (0xffffffff)
#define MT_INVALID_TIME        (0xffffffff)
#define MT_INVALID_PTS_U64      0x00
#define MT_INVALID_TIME_U64     0x00
#define MT_INVALID_TIME64       (0xffffffffffffffff)

#define RET_CODE               mt_s32
#define ERR_FAILURE           ((mt_s32) -1)
#define ERR_TIMEOUT           ((mt_s32) -2)
#define ERR_PARAM             ((mt_s32) -3)
#define ERR_STATUS            ((mt_s32) -4)
#define ERR_BUSY              ((mt_s32) -5)
#define ERR_NO_MEM            ((mt_s32) -6)
#define ERR_NO_RSRC           ((mt_s32) -7)
#define ERR_HARDWARE          ((mt_s32) -8)
#define ERR_NOFEATURE         ((mt_s32) -9)

/* For unicode fs */
#define UFS_UNICODE 1
/* for unicode char type */
typedef u16 tchar_t;
/* for unicode char strings */
#define _T(x) L ## x
/* for unicode char strings */
#define _TEXT(x) L ## x

#ifndef ABS
#define ABS(x)              (((x) < 0) ? -(x) : (x))
#endif
#ifndef __KERNEL__
#include <sys/param.h>		/* MAX, MIN, INT32_MAX, INT32_MIN, UINT32_MAX */
#include <stdint.h>
#else
#define MAX(x, y)           (((x) > (y)) ? (x) : (y))
#define MIN(a, b)           ((a) < (b) ? (a) : (b))
#define INT32_MAX           (2147483647)    /* 0x7fffffff */
#define INT32_MIN           (-2147483647-1) /* 0x80000000 */
#define UINT32_MAX          (4294967295U)   /* 0xffffffff */
#endif
#define UINT32_MIN          (0U)            /* 0x0 */

enum {
    ERROR_CODE_NO_ERROR = 0                        ,
    ERROR_CODE_ERROR_RESULT                        ,
    ERROR_CODE_ERROR_TIMEOUT                       ,
    ERROR_CODE_ERROR_PARM                          ,
    ERROR_CODE_ERROR_NOCONNECT                     , /* 设备不存在 */
    ERROR_CODE_ERROR_NOOPEN                        ,
    ERROR_CODE_ERROR_READ                          ,
    ERROR_CODE_ERROR_WRITE                         , /* 写出错 */
    ERROR_CODE_ERROR_UNDOWN                        , /* 缓冲区上溢 */
    ERROR_CODE_ERROR_OVER                          , /* 缓冲区下溢 */
    ERROR_CODE_ERROR_MAXDEV                        , /* 超出设备的最大数量 */
    ERROR_CODE_ERROR_NODEV                         , /* 设备不存在 */
    ERROR_CODE_ERROR_NOREADY                       ,
    ERROR_CODE_ERROR_NOBUFF                        ,
    ERROR_CODE_ERROR_NOTENOUGHMEM                  ,
    ERROR_CODE_ERROR_NOT_SUPPORT                   ,
    ERROR_CODE_ERROR_MEM                           ,
    ERROR_CODE_ERROR_BE_USED                       , /* 设备被占用 */
    ERROR_CODE_ERROR_NO_GOOD_BLOCK_SEARCH          ,
    ERROR_CODE_ERROR_GET_BAD_BLOCK_FUNCTION_NOTSUPP,
    ERROR_CODE_ERROR_DEV_OPEN_FAILURE              ,
    ERROR_CODE_ERROR_ALREADY_INITIALIZED
};

#ifdef MT_ADVCA_SUPPORT
#define __INIT__
#define __EXIT__
#else
#define __INIT__  __init
#define __EXIT__  __exit
#endif

/*
 * define of MT_HANDLE :
 * bit31                                                           bit0
 *   |<----   16bit --------->|<---   8bit    --->|<---  8bit   --->|
 *   |--------------------------------------------------------------|
 *   |      MT_MOD_ID_E       |  mod defined data |     chnID       |
 *   |--------------------------------------------------------------|
 *
 * mod defined data: private data define by each module(for example: sub-mod id), usually, set to 0.
 */
#define MT_HANDLE_MAKEHANDLE(mod, privatedata, chnid)  \
    (MT_HANDLE)( (((mod)& 0xffff) << 16) | ((((privatedata)& 0xff) << 8) ) | (((chnid) & 0xff)) )

#define MT_HANDLE_GET_MODID(handle)    (((handle) >> 16) & 0xffff)
#define MT_HANDLE_GET_PriDATA(handle)  (((handle) >> 8) & 0xff)
#define MT_HANDLE_GET_CHNID(handle)    (((handle)) & 0xff)

#define UNUSED(x) ((x)=(x))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MT_TYPE_H__ */
