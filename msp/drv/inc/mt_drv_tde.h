/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_TDE_IOCTL_H__
#define __MT_TDE_IOCTL_H__

#include <linux/ioctl.h>
#include "mt_tde_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/* Use 't' as magic number */
#define TDE_IOC_MAGIC 't'

/** blit info */
typedef struct mtTDE_BITBLIT_CMD_S
{
    TDE_HANDLE     s32Handle;        /**< TDE handle */
    TDE2_SURFACE_S stBackGround;     /**< background surface */
    TDE2_RECT_S    stBackGroundRect; /**< background surface operating rect */
    TDE2_SURFACE_S stForeGround;     /**< foreground surface*/
    TDE2_RECT_S    stForeGroundRect; /**< foreground surface operating rect */
    TDE2_SURFACE_S stDst;            /**< target surface*/
    TDE2_RECT_S    stDstRect;        /**< target surface operating rect */
    TDE2_OPT_S     stOpt;            /**< operating option*/
    mt_u32         u32NullIndicator; /**< flag of mask , 1: valid ,0: invalid */
} TDE_BITBLIT_CMD_S;

/** color fill info */
typedef struct mtTDE_SOLIDDRAW_CMD_S
{
    TDE_HANDLE       s32Handle;        /**< TDE handle */
    TDE2_SURFACE_S   stForeGround;     /**< background surface */
    TDE2_RECT_S      stForeGroundRect; /**< background surface operating rect */
    TDE2_SURFACE_S   stDst;            /**< foreground surface */
    TDE2_RECT_S      stDstRect;        /**< foreground surface operating rect */
    TDE2_FILLCOLOR_S stFillColor;      /**< fill color */
    TDE2_OPT_S       stOpt;            /**< operating option */
    mt_u32           u32NullIndicator; /**< flag of mask , 1: valid ,0: invalid */
} TDE_SOLIDDRAW_CMD_S;

/** quick copy info */
typedef struct mtTDE_QUICKCOPY_CMD_S
{
    TDE_HANDLE     s32Handle;   /**< TDE handle */
    TDE2_SURFACE_S stSrc;       /**< src surface */
    TDE2_RECT_S    stSrcRect;   /**< src surface rect */
    TDE2_SURFACE_S stDst;       /**< target surface */
    TDE2_RECT_S    stDstRect;   /**< target rect */
} TDE_QUICKCOPY_CMD_S;

/** quick fill info */
typedef struct mtTDE_QUICKFILL_CMD_S
{
    TDE_HANDLE     s32Handle;   /**< TDE handle */
    TDE2_SURFACE_S stDst;       /**< target surface */
    TDE2_RECT_S    stDstRect;   /**< target surface rect */
    mt_u32         u32FillData; /**< fill color*/
} TDE_QUICKFILL_CMD_S;

/** quick defilicker info */
typedef struct mtTDE_QUICKDEFLICKER_CMD_S
{
    TDE_HANDLE     s32Handle;   /**< TDE handle */
    TDE2_SURFACE_S stSrc;       /**< src surface*/
    TDE2_RECT_S    stSrcRect;   /**< src rect*/
    TDE2_SURFACE_S stDst;       /**< target surface */
    TDE2_RECT_S    stDstRect;   /**< target rect */
} TDE_QUICKDEFLICKER_CMD_S;

/** quick scale info */
typedef struct mtTDE_QUICKRESIZE_CMD_S
{
    TDE_HANDLE     s32Handle; /**< TDE handle */
    TDE2_SURFACE_S stSrc;     /**< src surface*/
    TDE2_RECT_S    stSrcRect; /**< src rect*/
    TDE2_SURFACE_S stDst;     /**< target surface */
    TDE2_RECT_S    stDstRect; /**< target rect */
} TDE_QUICKRESIZE_CMD_S;

/** semi-planar YUV to RGB info */
typedef struct mtTDE_MBBITBLT_CMD_S
{
    TDE_HANDLE     s32Handle;  /**< TDE handle */
    TDE2_MB_S      stMB;       /**< src surface*/    
    TDE2_RECT_S    stMbRect;   /**< src rect*/       
    TDE2_SURFACE_S stDst;      /**< target surface */
    TDE2_RECT_S    stDstRect;  /**< target rect */   
    TDE2_MBOPT_S   stMbOpt;    /**< operating option */
} TDE_MBBITBLT_CMD_S;

/** commond info */
typedef struct mtTDE_ENDJOB_CMD_S
{
    TDE_HANDLE s32Handle;  /**< TDE handle */
    MT_BOOL    bSync;      /**< weather sync */
    MT_BOOL    bBlock;     /**< weather block */
    mt_u32     u32TimeOut; /**< time out(ms) */
} TDE_ENDJOB_CMD_S;

/** mask blit rop blend info */
typedef struct mtTDE_BITMAP_MASKROP_CMD_S
{
    TDE_HANDLE     s32Handle;         /**< TDE handle */
    TDE2_SURFACE_S stBackGround;      /**< bk surface */
    TDE2_RECT_S    stBackGroundRect;  /**< bk rect */
    TDE2_SURFACE_S stForeGround;      /**< fore surface*/ 
    TDE2_RECT_S    stForeGroundRect;  /**< fore rect*/    
    TDE2_SURFACE_S stMask;            /**< mask surface  */
    TDE2_RECT_S    stMaskRect;        /**< mask surface rect */
    TDE2_SURFACE_S stDst;             /**< target surface */ 
    TDE2_RECT_S    stDstRect;         /**< target rect */    
    TDE2_ROP_CODE_E enRopCode_Color;  /**< RGB rop type */
    TDE2_ROP_CODE_E enRopCode_Alpha;  /**< alpha rop type */
} TDE_BITMAP_MASKROP_CMD_S;

/** mask blit alpha blend info */
typedef struct mtTDE_BITMAP_MASKBLEND_CMD_S
{
    TDE_HANDLE     s32Handle;         /**< TDE handle */
    TDE2_SURFACE_S stBackGround;      /**< bk surface */      
    TDE2_RECT_S    stBackGroundRect;  /**< bk rect */         
    TDE2_SURFACE_S stForeGround;      /**< fore surface*/      
    TDE2_RECT_S    stForeGroundRect;  /**< fore rect*/         
    TDE2_SURFACE_S stMask;            /**< mask surface  */        
    TDE2_RECT_S    stMaskRect;        /**< mask surface rect */    
    TDE2_SURFACE_S stDst;             /**< target surface */       
    TDE2_RECT_S    stDstRect;         /**< target rect */          
    mt_u8          u8Alpha;           /**< global alpha */
    mt_u8          Reserved0;
    mt_u8          Reserved1;
    mt_u8          Reserved2;    
    TDE2_ALUCMD_E enBlendMode;        /**< blend opt */
}TDE_BITMAP_MASKBLEND_CMD_S;



/** mask blit alpha blend info */
typedef struct mtTDE_IMAGE_MULTIPLY_CMD_S
{
    TDE_HANDLE     s32Handle;         /**< TDE handle */
    TDE2_SURFACE_S stBackGround;      /**< bk surface */      
    TDE2_RECT_S    stBackGroundRect;  /**< bk rect */         
    TDE2_SURFACE_S stForeGround;      /**< fore surface*/      
    TDE2_RECT_S    stForeGroundRect;  /**< fore rect*/         
    TDE2_SURFACE_S stPattern;            /**< Pattern surface  */        
    TDE2_RECT_S    stPatternRect;        /**< masPatternk surface rect */    
    TDE2_SURFACE_S stDst;             /**< target surface */       
    TDE2_RECT_S    stDstRect;         /**< target rect */          

    TDE2_MULTIPLY_OPT_S  opt;
}TDE_IMAGE_MULTIPLY_CMD_S;



/** pattern fill  */
typedef struct mtTDE_PATTERN_FILL_CMD_S
{
    TDE_HANDLE s32Handle;          /**< TDE handle */
    TDE2_SURFACE_S stBackGround;   /**< bk surface */
    TDE2_RECT_S stBackGroundRect;  /**< bk rect */
    TDE2_SURFACE_S stForeGround;   /**< fore surface */
    TDE2_RECT_S stForeGroundRect;  /**< fore rect */
    TDE2_SURFACE_S stDst;          /**< target surface */
    TDE2_RECT_S stDstRect;         /**< target rect */
    TDE2_PATTERN_FILL_OPT_S stOpt; /**< option */
    mt_u32 u32NullIndicator;       /**< flag of mask , 1: valid ,0: invalid */
}TDE_PATTERN_FILL_CMD_S;

#define TDE_BEGIN_JOB _IOW(TDE_IOC_MAGIC, 1, TDE_HANDLE)
#define TDE_BIT_BLIT _IOW(TDE_IOC_MAGIC, 2, TDE_BITBLIT_CMD_S)
#define TDE_SOLID_DRAW _IOW(TDE_IOC_MAGIC, 3, TDE_SOLIDDRAW_CMD_S)
#define TDE_QUICK_COPY _IOW(TDE_IOC_MAGIC, 4, TDE_QUICKCOPY_CMD_S)
#define TDE_QUICK_RESIZE _IOW(TDE_IOC_MAGIC, 5, TDE_QUICKRESIZE_CMD_S)
#define TDE_QUICK_FILL _IOW(TDE_IOC_MAGIC, 6, TDE_QUICKFILL_CMD_S)
#define TDE_QUICK_DEFLICKER _IOW(TDE_IOC_MAGIC, 7, TDE_QUICKDEFLICKER_CMD_S)
#define TDE_MB_BITBLT _IOW(TDE_IOC_MAGIC, 8, TDE_MBBITBLT_CMD_S)
#define TDE_END_JOB _IOW(TDE_IOC_MAGIC, 9, TDE_ENDJOB_CMD_S)
#define TDE_WAITFORDONE _IOW(TDE_IOC_MAGIC, 10, TDE_HANDLE)
#define TDE_CANCEL_JOB _IOW(TDE_IOC_MAGIC, 11, TDE_HANDLE)
#define TDE_BITMAP_MASKROP _IOW(TDE_IOC_MAGIC, 12, TDE_BITMAP_MASKROP_CMD_S)
#define TDE_BITMAP_MASKBLEND _IOW(TDE_IOC_MAGIC, 13, TDE_BITMAP_MASKBLEND_CMD_S)
#define TDE_WAITALLDONE _IO(TDE_IOC_MAGIC, 14)
#define TDE_RESET _IO(TDE_IOC_MAGIC, 15)
#define TDE_TRIGGER_SEL _IOW(TDE_IOC_MAGIC, 16, TDE_TRIGGER_E)
#define TDE_SET_DEFLICKERLEVEL _IOW(TDE_IOC_MAGIC, 17, TDE_DEFLICKER_LEVEL_E)
#define TDE_GET_DEFLICKERLEVEL _IOR(TDE_IOC_MAGIC, 18, TDE_DEFLICKER_LEVEL_E)
#define TDE_SET_ALPHATHRESHOLD_VALUE _IOW(TDE_IOC_MAGIC, 19, mt_u8)
#define TDE_GET_ALPHATHRESHOLD_VALUE _IOR(TDE_IOC_MAGIC, 20, mt_u8)
#define TDE_SET_ALPHATHRESHOLD_STATE _IOW(TDE_IOC_MAGIC, 21, MT_BOOL)
#define TDE_GET_ALPHATHRESHOLD_STATE _IOW(TDE_IOC_MAGIC, 22, MT_BOOL)
#define TDE_PATTERN_FILL _IOW(TDE_IOC_MAGIC, 23, TDE_PATTERN_FILL_CMD_S)
#define TDE_ENABLE_REGIONDEFLICKER _IOW(TDE_IOC_MAGIC, 24, MT_BOOL)
#define TDE_IMAGE_MULTIPLY  _IOW(TDE_IOC_MAGIC, 25, TDE_IMAGE_MULTIPLY_CMD_S)
#ifndef CONFIG_TDE_PROC_DISABLE
#define TDE_SET_PROC _IOWR(TDE_IOC_MAGIC, 26, MT_TDE_PROC_INFO_S*)
#endif
#define TDE_WAIT_FINISH _IO(TDE_IOC_MAGIC, 27)
#define TDE_CF_WAIT_FINISH _IO(TDE_IOC_MAGIC, 28)
#define TDE_CF_WAIT_SYNC_FINISH _IO(TDE_IOC_MAGIC, 29)

//#pragma pack()
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __MT_TDE_IOCTL_H__ */

