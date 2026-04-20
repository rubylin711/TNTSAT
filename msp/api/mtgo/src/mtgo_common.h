/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_COMMOM_H__
#define __MTGO_COMMOM_H__

/* add include here */
#include "mt_go_comm.h"
#ifdef TEST_IN_ROOTBOX
#include "mtgo_memmng.h"
#include "adp_vmem.h"
#else
#include "mtgo_memory.h"
#endif
#include "mtgo_adp_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RGB24
/***************************** Macro Definition ******************************/

#define IS_CLUT_FORMAT(p) (((p) >= MTGO_PF_CLUT8 && (p) <= MTGO_PF_CLUT4) || ((p) == MTGO_PF_ACLUT88))
#define IS_ALPHA_FORMAT(p) ((p) >= MTGO_PF_A1 &&(p) <= MTGO_PF_A8)
#define IS_YUV_FORMAT(p) (((p) >= MTGO_PF_YUV400 &&(p) <= MTGO_PF_YUV444) ||((p) >= MTGO_PF_YUV888 &&(p) <= MTGO_PF_YUV8888 ) || ((p) == MTGO_PF_YUV420TILE))
#define IS_SP_YUV_FORMAT(p) ((p) >= MTGO_PF_YUV400 &&(p) <= MTGO_PF_YUV444)
#define IS_RGB_FORMAT(p) (((p) >= MTGO_PF_4444 &&(p) <= MTGO_PF_0888) || ((p) == MTGO_PF_233) || ((p) == MTGO_PF_XYLC))
#define IS_SP_FORMAT(p) (((p) >= MTGO_PF_YUV400 &&(p) <= MTGO_PF_YUV444) || ((p) == MTGO_PF_SP_CMYK))

#define CHECK_NULLPTR(ptr)       \
    if (MT_NULL == ptr) {        \
	return MTGO_ERR_NULLPTR; \
    }

#define IS_SRCRECT_IN_DSTRECT(pSrcRect, pDstRect) ((pSrcRect->x >= pDstRect->x) && (pSrcRect->y >= pDstRect->y) && ((pSrcRect->x + pSrcRect->w) <= (pDstRect->x + pDstRect->w)) && ((pSrcRect->y + pSrcRect->h) <= (pDstRect->y + pDstRect->h)))

#define RGB32TO4444(c) \
    (((c & 0xf0000000) >> 16) | ((c & 0xf00000) >> 12) | ((c & 0xf000) >> 8) | ((c & 0xf0) >> 4))

#define RGB32TO1555(c) \
    (((c & 0x80000000) >> 16) | ((c & 0xf80000) >> 9) | ((c & 0xf800) >> 6) | ((c & 0xf8) >> 3))

#define RGB32TO565(c) \
    (((c & 0xf80000) >> 8) | ((c & 0xfc00) >> 5) | ((c & 0xf8) >> 3))

#define RGB32TO8565(c) \
    ((c & 0xff000000) >> 8 | ((c & 0xf80000) >> 8) | ((c & 0xfc00) >> 5) | ((c & 0xf8) >> 3))

#define RGB4444TO32(c) (((c & 0xf000) << 16) | \
                        ((c & 0x0f00) << 12) | \
                        ((c & 0x00f0) << 8) |  \
                        ((c & 0x000f) << 4))

#define RGB0444TO32(c) (0xff000000 | ((c & 0x0f00) << 12) | \
                        ((c & 0x00f0) << 8) |               \
                        ((c & 0x000f) << 4))

#define RGB565TO32(c) ((0xff000000 | (c & 0xf800) << 8) | \
                       ((c & 0x7e0) << 5) |               \
                       ((c & 0x1f) << 3))
#define RGB8565TO32(c) (((c & 0x00ff0000) << 8) | ((c & 0xf800) << 8) | \
                        ((c & 0x7e0) << 5) |                            \
                        ((c & 0x1f) << 3))

#define RGB555TO32(c) (0xff000000 |          \
                       ((c & 0x7c00) << 9) | \
                       ((c & 0x3e0) << 6) |  \
                       ((c & 0x1f) << 3))

#define RGB1555TO32(c) (((c & 0x8000) ? 0xff000000 : 0x0) | \
                        ((c & 0x7c00) << 9) |               \
                        ((c & 0x3e0) << 6) |                \
                        ((c & 0x1f) << 3))

/** use "MT_COLOR" get ARGB */
#define GetARGB(c, a, r, g, b)    \
    do {                          \
	(a) = ((c) >> 24) & 0xff; \
	(r) = ((c) >> 16) & 0xff; \
	(g) = ((c) >> 8) & 0xff;  \
	(b) = (c) & 0xff;         \
    } while (0)

/** use "MT_COLOR" get ARGB */
#define GetRGB(c, r, g, b)        \
    do {                          \
	(r) = ((c) >> 16) & 0xff; \
	(g) = ((c) >> 8) & 0xff;  \
	(b) = (c) & 0xff;         \
    } while (0)
#ifdef TEST_IN_ROOTBOX
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#else
#define MTGO_ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))
#endif
#define MTGO_MIN(x, y) (((x) > (y)) ? (y) : (x))
#define MTGO_MAX(x, y) (((x) > (y)) ? (x) : (y))

#define RECT2REGION(rect, region)                 \
    do {                                          \
	(region).l = (rect).x;                    \
	(region).t = (rect).y;                    \
	(region).r = (((rect).x + (rect).w) - 1); \
	(region).b = (((rect).y + (rect).h) - 1); \
    } while (0)

#define REGION2RECT(rect, region)                   \
    do {                                            \
	(rect).x = (region).l;                      \
	(rect).y = (region).t;                      \
	(rect).w = (((region).r - (region).l) + 1); \
	(rect).h = (((region).b - (region).t) + 1); \
    } while (0)

#define _MTGO_DEBUG
#ifdef _MTGO_DEBUG
#define MTGO_LOG_L(level, str, args...) MTGO_ADP_LOG(level, str, ##args)
#define MTGO_LOG(str, args...) MTGO_ADP_LOG(MTGO_LOG_LEVEL_INFO, str, ##args)
#define MTGO_SetError(errno) MTGO_ADP_SetError(errno)
#define MTGO_ASSERT(cond) MTGO_ADP_ASSERT(cond)
#define MTGO_ASSERT_EQUAL(actual, expect) MTGO_ASSERT((actual) == (expect))
#define MTGO_ASSERT_NOTEQUAL(actual, expect) MTGO_ASSERT((actual) != (expect))
#define MTGO_ERROR(Errno) printf("+++++ FILE: %s, LINE: %d, ret:%lx\n", __FILE__, __LINE__, (ulong)Errno); //MT_LogOut(MT_LOG_LEVEL_ERROR, , __FUNCTION__, __LINE__, MT_U8 *format, ...);
#define MTGO_ERROR2(str...)                                                                        //MTGO_ADP_LOG(MTGO_LOG_LEVEL_NOTICE, " ERR: %x\n", (mt_u32)errno)
#define MTGO_TRACE(str...)                                                                         //MTGO_ADP_LOG(MTGO_LOG_LEVEL_NOTICE, " ERR: %x\n", (mt_u32)errno)
#else
#define MTGO_LOG(str, args...)
#define MTGO_LOG_L(level, str, args...)
#define MTGO_SetError(errno)
#define MTGO_ASSERT(cond)
#define MTGO_ASSERT_EQUAL(actual, expect) (mt_void)(actual)
#define MTGO_ASSERT_NOTEQUAL(actual, expect) (mt_void)(actual)
#define MTGO_ERROR(Errno)
#define MTGO_ERROR2(str...)
#define MTGO_TRACE(str...)
#endif

#define ERR_CHKNEQ_RETURN(exp, value, label) \
    if ((exp) != (value))                    \
    return label

#define ERR_CHKEQ_RETURN(exp, value, label) \
    if ((exp) == (value))                   \
    return label

#define ERR_CHKNEQ_GOTO(exp, value, label) \
    if ((exp) != (value))                  \
    goto label

#define ERR_CHKEQ_GOTO(exp, value, label) \
    if ((exp) == (value))                 \
    goto label

#define UN_INIT_STATE 0
#define CLEAR_INIT_STATE 1
/*************************** Structure Definition ****************************/
typedef ulong MTGO_HANDLE;
typedef ulong DEC_HANDLE;
#ifndef TEST_IN_ROOTBOX
mt_s32 MTGO_GetRealRect(const MT_RECT *pSrcRect, const MT_RECT *pRect, MT_RECT *pRealRect);
#endif
/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/

#ifdef __cplusplus
}
#endif
#endif /* __MTGO_COMMOM_H__ */
