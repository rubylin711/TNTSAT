/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MTFB_COMM_H__
#define __MTFB_COMM_H__


/*********************************add include here******************************/

#include "mtfb.h"
#include "mt_debug.h"
//#include "mt_gfx_comm_k.h"


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */



/***************************** Macro Definition ******************************/
/* define debug level */
#if 0
#define MTFB_FATAL(fmt...)              MT_GFX_COMM_LOG_FATAL(MTGFX_FB_ID,fmt)
#define MTFB_ERROR(fmt...)              MT_GFX_COMM_LOG_ERROR(MTGFX_FB_ID,fmt)
#define MTFB_WARNING(fmt...)            MT_GFX_COMM_LOG_WARNING(MTGFX_FB_ID,fmt)
#define MTFB_INFO(fmt...)               MT_GFX_COMM_LOG_INFO(MTGFX_FB_ID,fmt)
#else
#define MTFB_FATAL(fmt...)
#define MTFB_ERROR(fmt...)
#define MTFB_WARNING(fmt...) 
#define MTFB_INFO(fmt...) 
#endif

#define MTFB_FILE_PATH_MAX_LEN         256
#define MTFB_FILE_NAME_MAX_LEN         32

#define CFG_MTFB_COMPRESSION_SUPPORT 
#ifdef CFG_MTFB_COMPRESSION_SUPPORT
#define CFG_MTFB_COMPRESSION_SUPPORT_OSD0
#define CFG_MTFB_COMPRESSION_SUPPORT_OSD1
//#define CFG_MTFB_COMPRESSION_SUPPORT_SUB
#endif

/* unit rect */
#define MTFB_UNITE_RECT(stDstRect, stSrcRect) do\
{\
    MTFB_RECT stRect;\
    stRect.x = (stDstRect.x < stSrcRect.x)? stDstRect.x : stSrcRect.x;\
    stRect.y = (stDstRect.y < stSrcRect.y)? stDstRect.y : stSrcRect.y;\
    stRect.w = ((stDstRect.x + stDstRect.w) > (stSrcRect.x + stSrcRect.w))? \
        (stDstRect.x + stDstRect.w - stRect.x) : (stSrcRect.x + stSrcRect.w - stRect.x);\
    stRect.h = ((stDstRect.y + stDstRect.h) > (stSrcRect.y + stSrcRect.h))? \
        (stDstRect.y + stDstRect.h - stRect.y) : (stSrcRect.y + stSrcRect.h - stRect.y);\
    memcpy(&stDstRect, &stRect, sizeof(MTFB_RECT));\
}while(0)


#define MTFB_MIN(m, n) (m) > (n) ? (n) : (m)


/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/

mt_void mtfb_version(mt_void);

mt_void *mtfb_buf_map(phys_addr_t u32PhyAddr);

mt_s32 mtfb_buf_ummap(mt_void *pViraddr);

mt_void mtfb_buf_freemem(phys_addr_t u32Phyaddr);

phys_addr_t mtfb_buf_allocmem(mt_char *pName, mt_u32 u32LayerSize);


MT_BOOL MTFB_IsIntersectRect(const MTFB_RECT* pRect1, const MTFB_RECT* pRect2);

mt_void mtfb_addrect(MTFB_RECT *pRectHead, mt_u32 TotalNum, mt_u32 *pValidNum, MTFB_RECT *pRect);

MT_BOOL mtfb_iscontain(MTFB_RECT *pstParentRect, MTFB_RECT *pstChildRect);

MT_BOOL mtfb_isoverlay(MTFB_RECT *pstSrcRect, MTFB_RECT *pstDstRect);

mt_u32 mtfb_getbppbyfmt(MTFB_COLOR_FMT_E enColorFmt);

mt_u32 calc_pixel_stride_by_pitch_fmt(mt_u32 pitch, MTFB_COLOR_FMT_E enColorFmt, mt_u32 line);

mt_s32 mtfb_bitfieldcmp(struct fb_bitfield x, struct fb_bitfield y);

#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_COMM_H__ */
