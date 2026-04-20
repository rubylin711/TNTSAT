/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MTFB_SCROLLTEXT_H__
#define __MTFB_SCROLLTEXT_H__


/*********************************add include here******************************/
#include "mt_type.h"
#include <linux/fb.h>


/*****************************************************************************/


#ifdef __cplusplus
#if __cplusplus
   extern "C"
{
#endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/** \addtogroup      H_3_5 */
/** @{ */  /** <!—[ECS error code]*/

#if 1
#define SCROLLTEXT_CACHE_NUM  2   /** 双buffer  **/
#define SCROLLTEXT_NUM         2  /** 最大只支持上下两个字幕  **/


/*************************** Structure Definition ****************************/

typedef enum
{
    MTFB_SCROLLTEXT_HD0_HANDLE0 = 0x00,
    MTFB_SCROLLTEXT_HD0_HANDLE1 = 0x01,
    MTFB_SCROLLTEXT_HD1_HANDLE0 = 0x10,
    MTFB_SCROLLTEXT_HD1_HANDLE1 = 0x11,
    MTFB_SCROLLTEXT_HD2_HANDLE0 = 0x20,
    MTFB_SCROLLTEXT_HD2_HANDLE1 = 0x21,
    MTFB_SCROLLTEXT_SD0_HANDLE0 = 0x40,
    MTFB_SCROLLTEXT_SD0_HANDLE1 = 0x41,
    MTFB_SCROLLTEXT_BUTT_HANDLE
}MTFB_SCROLLTEXT_HANDLE;

/** @} */  /*! <!-- Macro Definition End */


/*************************** Structure Definition ****************************/
/** \addtogroup      H_2_3_2*/
/** @{*/  /** <!—[HiFB_SCROLLTEXT]*/

/**Obtains the colorkey of a graphics layer.*/



/**MTFB_SCROLLTEXT_CACHE*/
typedef struct 
{
    //MT_BOOL bAvailable;           /*the cachebuffer for scrolltext is available or not*/
    MT_BOOL bInusing;             
    phys_addr_t u32PhyAddr;
    mt_u8   *pVirAddr;
}MTFB_SCROLLTEXT_CACHE;


/**MTFB_SCROLLTEXT_S*/
typedef struct 
{
    MT_BOOL bAvailable;           /*the scrolltext is available or not*/    
	MT_BOOL bPause;               /*0:resume; 1:pause;*/
    MT_BOOL bDeflicker;
	MT_BOOL bBliting;			  /*0:          ; 1:tde bliting cache buffer*/
    mt_u32  u32cachebufnum;       /*the number of cache buffer for scrolltext*/
    mt_u32  u32Stride;
    mt_s32  s32TdeBlitHandle;     /* blit handle */
	MTFB_SCROLLTEXT_HANDLE enHandle;
    volatile mt_u32 u32IdleFlag;  /*whether it is an idle cache buffer*/
    wait_queue_head_t wbEvent;    /*wait for a idle cache buffer*/
    MTFB_RECT  stRect;            /*region of the scrolltext showing on the screen*/
	MTFB_COLOR_FMT_E  ePixelFmt;  /*the color fmt of the scrolltext content*/
    MTFB_SCROLLTEXT_CACHE stCachebuf[SCROLLTEXT_CACHE_NUM];
}MTFB_SCROLLTEXT_S;


/**MTFB_SCROLLTEXT_INFO_S*/
typedef struct 
{
    MT_BOOL bAvailable;            /** the scrolltext layer is available or not **/
    mt_u32  u32textnum;     
	mt_u32  u32ScrollTextId;      /**the ID allocated for the scrolltext that will be created */
    MTFB_SCROLLTEXT_S stScrollText[SCROLLTEXT_NUM];
}MTFB_SCROLLTEXT_INFO_S;



/** @}*/  /** <!-- ==== Structure Definition End ====*/



/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/

/** \addtogroup      H_1_3_2*/
/** @{*/  /** <!-- -HiFB_SCROLLTEXT=*/

/**-----Standard Functions--------*/
/**
\brief allocate scrolltext handle when create scrolltext.
\attention \n

\param[in] fd ID of an FB device
\param[out]
\retval MTFB_SCROLLTEXT_HANDLE
\par example
\code
\endcode
*/
mt_u32 mtfb_alloscrolltext_handle(mt_u32 u32LayerId);


/**-----Standard Functions--------*/
/**
\brief parse the handle of scrolltext.
\attention \n

\param[in] handle of scrolltext
\param[out] pU32LayerId, pScrollTextId
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_u32 mtfb_parse_scrolltexthandle(mt_u32 u32Handle, mt_u32 *pU32LayerId, mt_u32 *pScrollTextId);


/**-----Standard Functions--------*/
/**
\brief check the parament of scrolltext before create scrolltext.
\attention \n

\param[in] u32LayerId, attributes of scrolltext
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_check_scrolltext_para(mt_u32 u32LayerId, MTFB_SCROLLTEXT_ATTR_S *stAttr);



/**-----Standard Functions--------*/
/**
\brief release the cache buffer of scrolltext.
\attention \n

\param[in] struct MTFB_SCROLLTEXT_S *
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_freescrolltext_cachebuf(MTFB_SCROLLTEXT_S *pstScrollText);

/**-----Standard Functions--------*/
/**
\brief allocate the cache buffer for the scrolltext.
\attention \n

\param[in] struct MTFB_SCROLLTEXT_ATTR_S *
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_allocscrolltext_buf(mt_u32 u32LayerId, MTFB_SCROLLTEXT_ATTR_S *stAttr);


/**-----Standard Functions--------*/
/**
\brief create scrolltext.
\attention \n

\param[in] struct MTFB_SCROLLTEXT_CREATE_S *
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_create_scrolltext(mt_u32 u32LayerId, MTFB_SCROLLTEXT_CREATE_S *stScrollText);


/**-----Standard Functions--------*/
/**
\brief fill the usr data to the cache buffer.
\attention \n

\param[in] struct MTFB_SCROLLTEXT_DATA_S *
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_fill_scrolltext(MTFB_SCROLLTEXT_DATA_S *stScrollTextData);


/**-----Standard Functions--------*/
/**
\brief destroy the scrolltext and release the resource.
\attention \n

\param[in] 
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_destroy_scrolltext(mt_u32 u32LayerID, mt_u32 u32ScrollTextID);


/**-----Standard Functions--------*/
/**
\brief blit the cache buffer to the display buffer.
\attention \n

\param[in] 
\param[out] 
\retval 0:SUCCESS
\par example
\code
\endcode
*/
mt_s32 mtfb_scrolltext_blit(mt_u32 u32LayerId);

/** @}*/  /** <!-- ==== API Declaration End ====*/
#endif



#ifdef __cplusplus

#if __cplusplus

}
#endif
#endif /* __cplusplus */

#endif /* __MTFB_SCROLLTEXT_H__ */


