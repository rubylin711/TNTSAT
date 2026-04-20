/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
#include "string.h"
#include "adp_layer.h"
#include "mt_go_gdev.h"
#include "mtgo_adp_sys.h"
#include "mtgo_gdev.h"
#include "mt_common.h"

#include "mtfb.h"
#include "mt_drv_pdm.h"
#include "optm_mtfb.h"
//#include "mtfb_debug.h"
#include "mt_tde_api.h"
#include "adp_gfx.h"
#include "mt_drv_disp.h" 
#include "mt_module_debug.h"
//#include "drv_display.h"
#include "adp_mtfb.h"
#include "mpi_mmz.h"


#define IS_SD_ADP_LAYER(LayerID) ((LayerID >= MTGO_LAYER_SD_0)&&(LayerID <= MTGO_LAYER_SD_1))
#define IS_HD_ADP_LAYER(LayerID) (LayerID <= MTGO_LAYER_HD_3)
#define IS_AD_ADP_LAYER(LayerID) ((LayerID >= MTGO_LAYER_AD_0)&&(LayerID <= MTGO_LAYER_AD_1))


#define SD_INDEX 0
#define HD_INDEX 1
#define AD_INDEX 2
#define LAYER_TYPE_NUM 3

#ifndef MTGO_CODE_CUT
#ifdef CONFIG_MT_FPGA_GPE
#define MTGO_LAYER_PFCOUNT 12
#else
#define MTGO_LAYER_PFCOUNT 9
#endif
#define MTGO_LAYER_COUNT 10
#else
#define MTGO_LAYER_COUNT 4
#endif

typedef struct
{
    mt_u32       u32LayerID;
    mt_u32       u32LayerAddr;   
    MTFB_RECT     stInRect;  
    MTFB_RECT     stOutRect;   
    mt_u32       u32Stride;  
    mt_u32       hwStride;    //  used for osd hw , pixel number, 16 byte aligned
}MTFB_LAYER_REFRESH_S;

typedef struct _MTGO_DISPLAY_INFO_S
{
    phys_addr_t u32ScreenAddr;	
    ulong u32VirScreenAddr;	
    //MTFB_POINT_S stScreenPos;
    MTFB_RECT stInRect;
    MTFB_RECT stOutRect;
    MTFB_ALPHA_S stAlpha;
#ifndef MTGO_CODE_CUT
    MTFB_COLORKEYEX_S stColorKey;
#endif
#ifdef CONFIG_MT_FPGA_GPE
    mt_u32 screen_pitch;
#endif
}MTGO_DISPLAY_INFO_S;

static MTGO_DISPLAY_INFO_S s_DisplayInfo[MTGO_LAYER_MAXCOUNT];

//use the mtfb ioctrol to instead of the opem funciton
#define _NO_USE_OPTM_DRV_
#define _USE_MTFB_IOC_
//#define _USE_MTFB_CMP_

#ifndef _NO_USE_OPTM_DRV_
static OPTM_GFX_OPS_S   s_optm_gfx_ops;
#endif


#ifdef _USE_MTFB_IOC_
#include "adp_mtfb.h"
#endif



/** 图层私有信息结构 */
typedef struct _MTGO_LAYER_PRIVATE_S
{
    mt_s32            fd;            /**< 文件操作符 */
    mt_char*          pszLayerName;  /**< 图层路径名称 */
    MTGO_LAYER_CAP_S  struLayerCap;  /**< 图层能力集 */
} MTGO_LAYER_PRIVATE_S;

#ifndef MTGO_CODE_CUT
/** 各像素格式对应的位域信息数组 */
#ifdef CONFIG_MT_FPGA_GPE
const MTGO_BITFIELD_S s_BitField[12] =
#else
const MTGO_BITFIELD_S s_BitField[9] =
#endif
{
    {
        .red    ={     8, 4, 0}, /**< RGB4444 */
        .green  ={     4, 4, 0},
        .blue   ={     0, 4, 0},
        .transp ={    12, 4, 0},
    },
    {
        .red    ={     8, 4, 0}, /**< RGB0444 */
        .green  ={     4, 4, 0},
        .blue   ={     0, 4, 0},
        .transp ={     0, 0, 0},
    },
    {
        .red    ={    10, 5, 0}, /**< RGB1555 */
        .green  ={     5, 5, 0},
        .blue   ={     0, 5, 0},
        .transp ={    15, 1, 0},
    },
    {
        .red    ={    10, 5, 0}, /**< RGB0555 */
        .green  ={     5, 5, 0},
        .blue   ={     0, 5, 0},
        .transp ={     0, 0, 0},
    },
    {
        .red    ={    11, 5, 0}, /**< RGB565 */
        .green  ={     5, 6, 0},
        .blue   ={     0, 5, 0},
        .transp ={     0, 0, 0},
    },
    {
        .red    ={    16, 8, 0}, /**< RGB8888 */
        .green  ={     8, 8, 0},
        .blue   ={     0, 8, 0},
        .transp ={    24, 8, 0},
    },
    {
        .red    ={    16, 8, 0}, /**< RGB0888 */
        .green  ={     8, 8, 0},
        .blue   ={     0, 8, 0},
        .transp ={     0, 0, 0},
    },
    {
        .red    ={     0, 8, 0}, /**< CLUT8 */
        .green  ={     0, 8, 0},
        .blue   ={     0, 8, 0},
        .transp ={     0, 0, 0},
    },
    {
        .red    = {    11, 5, 0}, /**< RGB8565 */
        .green  = {    5, 6, 0},
        .blue   = {    0, 5, 0},
        .transp = {   16, 8, 0},
    }
#ifdef CONFIG_MT_FPGA_GPE	
	,
    {
        .red    = {    6, 2, 0}, /**< RGB233 */
        .green  = {    3, 3, 0},
        .blue   = {    0, 3, 0},
        .transp = {   0, 0, 0},
    },  
       
    /*ACLUT88*/
    {
        .red    = {8, 8, 0},
        .green  = {8, 8, 0},
        .blue   = {8, 8, 0},
        .transp = {0, 8, 0},
    }
#endif	
};
#endif

static MTGO_LAYER_INFO_S s_MtGoLayerDefInfo[LAYER_TYPE_NUM] = 
{{720 , 576, 720 , 576, 720 , 576, (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_DOUBLE, MTGO_LAYER_DEFLICKER_AUTO, MTGO_PF_1555, MTGO_LAYER_SD_0},
 //{1920, 1080, 1920, 1080, 1920, 1080, (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_TRIPLE, MTGO_LAYER_DEFLICKER_AUTO, MTGO_PF_1555, MTGO_LAYER_HD_0},
 {1280, 720, 1280, 720, 1280, 720, (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_DOUBLE, MTGO_LAYER_DEFLICKER_AUTO, MTGO_PF_8888, MTGO_LAYER_HD_0}, 
 {1280, 80, 1280, 80, 1280, 80, (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_DOUBLE, MTGO_LAYER_DEFLICKER_AUTO, MTGO_PF_1555, MTGO_LAYER_AD_0}};
#if 0
static MTGO_LAYER_INFO_S s_MtGoLayerDefInfo[1] = 
{
 //{1920, 1080, 1920, 1080, 1920, 1080, (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_TRIPLE, MTGO_LAYER_DEFLICKER_AUTO, MTGO_PF_1555, MTGO_LAYER_HD_0},
 {1280, 720, 1280, 720, 1280, 720, (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_DOUBLE, MTGO_LAYER_DEFLICKER_AUTO, MTGO_PF_8888, MTGO_LAYER_HD_0}, 
 };
#endif
//#endif
#ifndef MTGO_CODE_CUT
/** 图层信息数组，数组下标为硬件层的ID */
static MTGO_LAYER_PRIVATE_S s_LayerInfo[MTGO_LAYER_COUNT];

/** 图形叠加层支持的像素格式 */
#ifdef CONFIG_MT_FPGA_GPE
static MTGO_PF_E s_LayerFmt[MTGO_LAYER_PFCOUNT] = {/*MTGO_PF_CLUT8,*/ MTGO_PF_4444, MTGO_PF_0444, MTGO_PF_1555,
                                  MTGO_PF_0555,  MTGO_PF_565,  MTGO_PF_0888, MTGO_PF_8888,MTGO_PF_CLUT8,  MTGO_PF_8565, MTGO_PF_233, MTGO_PF_ACLUT88};
#else
static MTGO_PF_E s_LayerFmt[MTGO_LAYER_PFCOUNT] = {/*MTGO_PF_CLUT8,*/ MTGO_PF_4444, MTGO_PF_0444, MTGO_PF_1555,
                                  MTGO_PF_0555,  MTGO_PF_565,  MTGO_PF_0888, MTGO_PF_8888, MTGO_PF_8565};
#endif
static MTGO_PF_E s_PfMem[MTGO_LAYER_PFCOUNT*MTGO_LAYER_COUNT];
#endif


/******************************************************************************
    internal function
******************************************************************************/
#ifdef MT_BUILD_IN_BOOT

static mt_u32 WinParamAlignUp(mt_u32 x, mt_u32 a) 
{
    if (!a)
    {
        return x;
    }
    else
    {
        return ( (( x + (a-1) ) / a ) * a);
    }
}


/*this func is for both graphics and video  virtual screen deal, it's a common function.*/
static mt_s32 Win_ReviseOutRect(const mt_rect_s *tmp_virtscreen, 
                         const MT_DRV_DISP_OFFSET_S *stOffsetInfo,
                         const mt_rect_s *stFmtResolution,
                         const mt_rect_s *stPixelFmtResolution,
                         mt_rect_s *stToBeRevisedRect, 
                         mt_rect_s *stRevisedRect)
{    
    mt_u32 width_ratio = 0,  height_ratio = 0;
    mt_u32 zmeDestWidth = 0, zmeDestHeight = 0;
    MT_DRV_DISP_OFFSET_S tmp_offsetInfo;

    tmp_offsetInfo = *stOffsetInfo;


    if ( (stFmtResolution->s32Width * 2) == stPixelFmtResolution->s32Width)
    {
        tmp_offsetInfo.u32Left  *= 2;
        tmp_offsetInfo.u32Right *= 2;        
    }

    zmeDestWidth = (mt_u32)(stPixelFmtResolution->s32Width) - tmp_offsetInfo.u32Left - tmp_offsetInfo.u32Right;
    zmeDestHeight = (mt_u32)(stPixelFmtResolution->s32Height) - tmp_offsetInfo.u32Top - tmp_offsetInfo.u32Bottom;

    
    /*pay attention ,we must care about that  u32 overflow.....*/
    width_ratio  = zmeDestWidth  * 100 /((mt_u32)(tmp_virtscreen->s32Width));
    height_ratio = zmeDestHeight * 100 /(mt_u32)((tmp_virtscreen->s32Height));    

    if (tmp_virtscreen->s32Width != stToBeRevisedRect->s32Width)
    {
        stRevisedRect->s32Width = (mt_s32)(((mt_u32)(stToBeRevisedRect->s32Width) * width_ratio) / 100);         
    } else {
        stRevisedRect->s32Width = (mt_s32)zmeDestWidth;
    }
    
    if (tmp_virtscreen->s32Height != stToBeRevisedRect->s32Height)
    {
        stRevisedRect->s32Height = (mt_s32)(((mt_u32)(stToBeRevisedRect->s32Height) * height_ratio) / 100);      
    } else {
        stRevisedRect->s32Height = (mt_s32)zmeDestHeight;
    }
    
        
    stRevisedRect->s32X = (mt_s32)(((mt_u32)(stToBeRevisedRect->s32X) * width_ratio) /100 + tmp_offsetInfo.u32Left);
    stRevisedRect->s32Y= (mt_s32)(((mt_u32)(stToBeRevisedRect->s32Y) * height_ratio) /100 + tmp_offsetInfo.u32Top);
    
    stRevisedRect->s32X = (mt_s32)WinParamAlignUp((mt_u32)(stRevisedRect->s32X), 2);
    stRevisedRect->s32Y = (mt_s32)WinParamAlignUp((mt_u32)(stRevisedRect->s32Y), 2);
    stRevisedRect->s32Width  = (mt_s32)WinParamAlignUp((mt_u32)(stRevisedRect->s32Width), 2);
    stRevisedRect->s32Height = (mt_s32)WinParamAlignUp((mt_u32)(stRevisedRect->s32Height), 2);



    return MT_SUCCESS;
}
//#endif
#endif

#ifndef _NO_USE_OPTM_DRV_
static OPTM_GFX_GP_E OPTM_GetGfxGpId(MTFB_LAYER_ID_E enLayerId)
{
   return OPTM_GFX_GP_0;
/*   
	if(MTFB_LAYER_HD_3 >= enLayerId)
{
		return OPTM_GFX_GP_0;
	}
	else if (enLayerId >= MTFB_LAYER_SD_0
			&& MTFB_LAYER_SD_1 >= enLayerId)
	{
		return OPTM_GFX_GP_1;
	}

	return OPTM_GFX_GP_BUTT;
*/	
}
#endif


mt_void ADP_Layer_ConvertFieldInfo(MTGO_PF_E enType, MTGO_BITFIELD_S *pstFieldInfo)
{
#ifndef MTGO_CODE_CUT
    switch (enType)
    {
    case MTGO_PF_4444:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[0], sizeof(MTGO_BITFIELD_S));
        break;
    case MTGO_PF_0444:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[1], sizeof(MTGO_BITFIELD_S));
        break;
    case MTGO_PF_1555:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[2], sizeof(MTGO_BITFIELD_S));
        break;
    case MTGO_PF_0555:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[3], sizeof(MTGO_BITFIELD_S));
        break;
    case MTGO_PF_565:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[4], sizeof(MTGO_BITFIELD_S));
        break;
    case MTGO_PF_8888:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[5], sizeof(MTGO_BITFIELD_S));
        break;
    case MTGO_PF_0888:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[6], sizeof(MTGO_BITFIELD_S));
        break;
#ifdef CONFIG_MT_FPGA_GPE		
    case MTGO_PF_CLUT8:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[7], sizeof(MTGO_BITFIELD_S));
        break;
#endif		
    case MTGO_PF_8565:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[8], sizeof(MTGO_BITFIELD_S));
        break;
#ifdef CONFIG_MT_FPGA_GPE     
     case MTGO_PF_233:
         MTGO_MemCopy(pstFieldInfo, &s_BitField[9], sizeof(MTGO_BITFIELD_S));
         break;
     case MTGO_PF_ACLUT88:
        MTGO_MemCopy(pstFieldInfo, &s_BitField[10],sizeof(MTGO_BITFIELD_S));
        break;
#endif	 
    default:
        break;
    }
#endif
    return;
}
#if 0 
static mt_s32 ADP_Layer_RefreshLayer(MTFB_LAYER_REFRESH_S *pstLayerAttr)
{
#ifndef _NO_USE_OPTM_DRV_
    OPTM_GFX_GP_E enGpId = OPTM_GetGfxGpId(pstLayerAttr->u32LayerID);
    if (enGpId >= OPTM_GFX_GP_BUTT)
        return MT_FAILURE;
    
    s_optm_gfx_ops.OPTM_GfxSetLayerAddr(pstLayerAttr->u32LayerID, pstLayerAttr->u32LayerAddr);
   // Debug("Set Layer Addr: 0x%x\n",pstLayerAttr->u32LayerAddr);
 
    s_optm_gfx_ops.OPTM_GfxSetLayerStride(pstLayerAttr->u32LayerID, pstLayerAttr->u32Stride);
    //Debug("Set Layer Stride: 0x%x\n",pstLayerAttr->u32Stride);

    s_optm_gfx_ops.OPTM_GfxSetLayerRect(pstLayerAttr->u32LayerID, &(pstLayerAttr->stInRect));

    s_optm_gfx_ops.OPTM_GfxSetGpRect(enGpId, &(pstLayerAttr->stInRect));


    //OPTM_GfxSetDispFMTSize(enGpId, (mt_rect_s *)(&(pstLayerAttr->stOutRect)));
    s_optm_gfx_ops.OPTM_GfxSetEnable(pstLayerAttr->u32LayerID, MT_TRUE);
/*

    if (OPTM_Get_GfxWorkMode() == MTFB_GFX_MODE_HD_WBC)
    {
        OPTM_GFX_GP_E   enGfxGpId = OPTM_GFX_GP_1;
        OPTM_Wbc2Isr(&enGfxGpId, NULL);
        s_optm_gfx_ops.OPTM_GfxUpLayerReg(MTGO_LAYER_SD_0);
    }
*/    
    s_optm_gfx_ops.OPTM_GfxUpLayerReg(pstLayerAttr->u32LayerID);
#endif
  
#ifdef _USE_MTFB_IOC_
  MTFB_GfxSetLayerAddr(pstLayerAttr->u32LayerID, pstLayerAttr->u32LayerAddr);
  //MTFB_GfxSetLayerStride(pstLayerAttr->u32LayerID, pstLayerAttr->u32Stride);
  MTFB_GfxSetLayerStride(pstLayerAttr->u32LayerID, pstLayerAttr->hwStride);   // in aria, this osd stride is pixel nubmer, 16 byte aligned
  MTFB_GfxSetLayerRect(pstLayerAttr->u32LayerID, &(pstLayerAttr->stInRect));
  MTFB_GfxSetEnable(pstLayerAttr->u32LayerID, MT_TRUE);
#endif

    return MT_SUCCESS;
}
#endif
#ifndef MTGO_CODE_CUT
static mt_s32 ADP_Layer_Init(mt_void** pInitData)
{
    mt_u32 i, j;
    MTGO_PF_E* pPixelFmt;

    /** 分配支持的像素格式数据所占的内存 */
    pPixelFmt = (MTGO_PF_E*)s_PfMem;

    /** 初始化图层私有结构数组 */
    for (i = 0; i < MTGO_LAYER_COUNT; i++)
    {
        //s_LayerInfo[i].pszLayerName = s_LayerName[i];
        s_LayerInfo[i].struLayerCap.bPremultiply = MT_TRUE;        
        s_LayerInfo[i].struLayerCap.bNonPremultiply = MT_FALSE;
        
        /** 初始化各图层支持的像素格式 */
        s_LayerInfo[i].struLayerCap.u8FmtCount = MTGO_LAYER_PFCOUNT;
        for (j = 0; j < MTGO_LAYER_PFCOUNT; j++)
        {
            *(pPixelFmt + MTGO_LAYER_PFCOUNT * i + j) = s_LayerFmt[j];
        }

        s_LayerInfo[i].struLayerCap.enPixelFmt = pPixelFmt + MTGO_LAYER_PFCOUNT * i;
        if (IS_SD_ADP_LAYER(i))
        {
            s_LayerInfo[i].struLayerCap.MaxWidth = MTGO_SD_MAXWIDTH;
            s_LayerInfo[i].struLayerCap.MaxHeight= MTGO_SD_MAXHEIGHT;            
            s_LayerInfo[i].struLayerCap.MinWidth = MTGO_SD_MINWIDTH;
            s_LayerInfo[i].struLayerCap.MinHeight= MTGO_SD_MINHEIGHT;       
            s_LayerInfo[i].struLayerCap.bVoScale = MT_TRUE; //TO be modify in pilot
            
        }
        else if (IS_HD_ADP_LAYER(i) /*|| IS_AD_LAYER(i)*/)
        {
            s_LayerInfo[i].struLayerCap.MaxWidth = MTGO_HD_MAXWIDTH;
            s_LayerInfo[i].struLayerCap.MaxHeight= MTGO_HD_MAXHEIGHT;            
            s_LayerInfo[i].struLayerCap.MinWidth = MTGO_HD_MINWIDTH;
            s_LayerInfo[i].struLayerCap.MinHeight= MTGO_HD_MINHEIGHT;              
            s_LayerInfo[i].struLayerCap.bVoScale = MT_TRUE;
        }
    }


    return MT_SUCCESS;
}
#endif


mt_s32 MTGO_ADP_InitDisplay(mt_void)
{
#ifndef MTGO_CODE_CUT
    mt_s32 s32Ret;
    mt_void* pInitData;
    s32Ret = ADP_Layer_Init(&pInitData);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
#endif    

#ifndef _NO_USE_OPTM_DRV_
   OPTM_GFX_GetOps(&s_optm_gfx_ops);
   s_optm_gfx_ops.OPTM_GfxInit();
	OPTM_GfxInit();
#endif

#ifdef _USE_MTFB_IOC_
  MTFB_GfxInit();
#endif

    return MT_SUCCESS;
}

static mt_void MTGO_ADP_DeinitDisplay(mt_void)
{
    /** 释放初始化时分配的内存 */
    return ;
}

mt_void MTGO_ADP_CapabilityInquire(mt_u32 LayerID,  MTGO_LAYER_CAP_S** pstruCap)
{
#ifndef MTGO_CODE_CUT
    /** 参数检查 */
    MTGO_ASSERT(MT_NULL_PTR != pstruCap);

    /** 获取图层能力集 */
    *pstruCap = &(s_LayerInfo[LayerID].struLayerCap);
#endif    
    return ;
}

mt_s32 MTGO_ADP_GetDefaultParam(mt_u32  LayerID, MTGO_LAYER_INFO_S *pLayerInfo)
{
//#ifndef MTGO_CODE_CUT
    if (IS_SD_ADP_LAYER(LayerID))
    {   
        MTGO_MemCopy(pLayerInfo, &s_MtGoLayerDefInfo[SD_INDEX], sizeof(MTGO_LAYER_INFO_S));
    }
    else if (IS_HD_ADP_LAYER(LayerID))
    {  
        MTGO_MemCopy(pLayerInfo, &s_MtGoLayerDefInfo[HD_INDEX], sizeof(MTGO_LAYER_INFO_S));
    }
    else if (IS_AD_ADP_LAYER(LayerID))
    {   
        MTGO_MemCopy(pLayerInfo, &s_MtGoLayerDefInfo[AD_INDEX], sizeof(MTGO_LAYER_INFO_S));        
    }
    else
    {
        MTGO_ERROR(MTGO_ERR_INVPARAM);
        return MTGO_ERR_INVPARAM;
    }
//#else
//    MTGO_MemCopy(pLayerInfo, &s_MtGoLayerDefInfo[0], sizeof(MTGO_LAYER_INFO_S));
//#endif
    pLayerInfo->LayerID = (MTGO_LAYER_E)LayerID;
    return MT_SUCCESS;
}

static mt_void ADP_Layer_MtgoFmtToMtfbFmt(MTGO_PF_E SrcFmt, MTFB_COLOR_FMT_E *DstFmt)
{
    switch (SrcFmt)
    {
        case MTGO_PF_4444:
            *DstFmt = MTFB_FMT_ARGB4444;
            break;
        case MTGO_PF_0444:
            *DstFmt = MTFB_FMT_KRGB444;
            break;            
        case MTGO_PF_1555:
            *DstFmt = MTFB_FMT_ARGB1555;
            break;
        case MTGO_PF_0555:
            *DstFmt = MTFB_FMT_KRGB555;
            break; 
        case MTGO_PF_565:
            *DstFmt = MTFB_FMT_RGB565;
            break;
        case MTGO_PF_8565:
            *DstFmt = MTFB_FMT_ARGB8565;
            break;            
        case MTGO_PF_8888:
            *DstFmt = MTFB_FMT_ARGB8888;
            break;
        case MTGO_PF_0888:
            *DstFmt = MTFB_FMT_RGB888;
            break;     
        case MTGO_PF_UYVY:
            *DstFmt = MTFB_FMT_PUYVY;
            break;     
        case MTGO_PF_YUV8888:
            *DstFmt = MTFB_FMT_AYUV8888;
            break;     
        case MTGO_PF_CLUT8:
            *DstFmt = MTFB_FMT_8BPP_ARGB;
            break;   
        case MTGO_PF_YUV420:
            *DstFmt = MTFB_FMT_SP_YUV420;
            break;  
#ifdef CONFIG_MT_FPGA_GPE        
         case MTGO_PF_ACLUT88:
            *DstFmt = MTFB_FMT_ACLUT88;
             break;
         case MTGO_PF_233:
            *DstFmt = MTFB_FMT_RGB233;
            break;
#endif			
        default:
            *DstFmt = MTFB_FMT_ARGB8888;
            break;
    }

}

static mt_s32 ADP_Layer_AllocateDisplayBuffer(MTGO_LAYER_INFO_S *pLayerInfo, MTGO_DISPLAY_INFO_S *pDisplayInfo)        __attribute__((noinline));

static mt_s32 ADP_Layer_AllocateDisplayBuffer(MTGO_LAYER_INFO_S *pLayerInfo, MTGO_DISPLAY_INFO_S *pDisplayInfo)
{
//    mt_s32 s32Ret;
    mt_u32 u32BPP;
    mt_u32 u32BufferSize;

    mt_mmz_buf_s  psMBuf;   /// TODO:: 
//    mt_char BufName[16] = "disp_buf";

    Surface_CalculateBpp0(pLayerInfo->PixelFormat, &u32BPP);

    //u32BufferSize = u32BPP *pLayerInfo->DisplayWidth * pLayerInfo->DisplayHeight;
    u32BufferSize = (u32BPP *(mt_u32)(pLayerInfo->DisplayWidth) + 0xf)&0xfffffff0;
	u32BufferSize *= (mt_u32)(pLayerInfo->DisplayHeight);

  
    //memset((mt_void*)stDisplayBuf.u32StartPhyAddr,0x0, u32BufferSize);
    //Debug("Display Buffer Address: 0x%x\n",stDisplayBuf.u32StartPhyAddr);


//aria
#if 0
    psMBuf.bufsize = u32BufferSize;
    strncpy(psMBuf.bufname, BufName, sizeof(mt_char)*16);
    s32Ret = mt_mpi_mmz_malloc(&psMBuf);
    if(s32Ret != MT_SUCCESS)
    {
        MT_ASSERT(0);
        MTGO_ERROR(s32Ret);
        return MT_FAILURE;
    }        

     // memset the psMBuf 
   {
       Debug("memset: [%x][%x][%x][%x]\n", psMBuf.user_viraddr, psMBuf.phyaddr, psMBuf.kernel_viraddr, psMBuf.bufsize);
//       memset(psMBuf.user_viraddr, 0xff, psMBuf.bufsize);
	   memset(psMBuf.user_viraddr, 0, psMBuf.bufsize);
    }
#else //symphony_linux
	{
//		mt_void *ptr;
		psMBuf.bufsize = u32BufferSize;		
		psMBuf.user_viraddr = (mt_u8 *)MTGO_MMZ_Malloc(u32BufferSize, &psMBuf.phyaddr);
	}	
#endif
    pDisplayInfo->u32ScreenAddr = psMBuf.phyaddr;
    pDisplayInfo->u32VirScreenAddr = (ulong)(psMBuf.user_viraddr);

    MT_INFO_MTGO("ADP_Layer_AllocateDisplayBuffer: [0x%x][0x%x] [%d][%d]\n", pDisplayInfo->u32ScreenAddr, psMBuf.bufsize, pLayerInfo->DisplayWidth, pLayerInfo->DisplayHeight);
    
    return MT_SUCCESS;   
}

static mt_s32 ADP_Layer_DisplayInfoInit(MTGO_LAYER_INFO_S *LayerInfo, MTGO_DISPLAY_INFO_S * DisplayInfo) __attribute__((noinline));

static mt_s32 ADP_Layer_DisplayInfoInit(MTGO_LAYER_INFO_S *LayerInfo, MTGO_DISPLAY_INFO_S * DisplayInfo)
{
    mt_s32 s32Ret;

    memset(DisplayInfo, 0, sizeof(MTGO_DISPLAY_INFO_S));    
#ifndef MTGO_CODE_CUT
    MTGO_BITFIELD_S pstFieldInfo;
    ADP_Layer_ConvertFieldInfo(LayerInfo->PixelFormat, &pstFieldInfo);
  
    DisplayInfo->stColorKey.u8BlueMask = (mt_u8)(0Xff >> pstFieldInfo.blue.length);
    DisplayInfo->stColorKey.u8RedMask = (mt_u8)(0Xff >> pstFieldInfo.red.length);
    DisplayInfo->stColorKey.u8GreenMask= (mt_u8)(0Xff >> pstFieldInfo.green.length);
#endif    
    DisplayInfo->stAlpha.bAlphaEnable = MT_TRUE;
    DisplayInfo->stAlpha.bAlphaChannel = MT_FALSE;
    DisplayInfo->stAlpha.u8Alpha0 = 0x00;
    DisplayInfo->stAlpha.u8Alpha1 = 0xff;
    DisplayInfo->stAlpha.u8GlobalAlpha = 0xff;

#if 0    
	if (LayerInfo->LayerID <= MTGO_LAYER_HD_3)
	{
		DISP_GetDisplayInfo(MT_DRV_DISPLAY_1, &pstInfo);  
	}
	else
	{
		DISP_GetDisplayInfo(MT_DRV_DISPLAY_0, &pstInfo);
	}    

    DisplayInfo->stInRect.x = pstInfo.stVirtaulScreen.s32X;
    DisplayInfo->stInRect.y = pstInfo.stVirtaulScreen.s32Y;
    DisplayInfo->stInRect.w = pstInfo.stVirtaulScreen.s32Width;
    DisplayInfo->stInRect.h = pstInfo.stVirtaulScreen.s32Height;
#endif

    DisplayInfo->stInRect.x = 0;
    DisplayInfo->stInRect.y = 0;
//    DisplayInfo->stInRect.w = 1280;//1920;
//    DisplayInfo->stInRect.h = 720;//1080;

    DisplayInfo->stInRect.w = LayerInfo->DisplayWidth;
    DisplayInfo->stInRect.h = LayerInfo->DisplayHeight;

//    LayerInfo->DisplayWidth = DisplayInfo->stInRect.w;
//    LayerInfo->DisplayHeight = DisplayInfo->stInRect.h;
//    LayerInfo->CanvasWidth = DisplayInfo->stInRect.w;
//    LayerInfo->CanvasHeight = DisplayInfo->stInRect.h;
    //memcpy(&DisplayInfo->stOutRect,&DisplayInfo->stInRect,sizeof(DisplayInfo->stInRect));
    MT_INFO_MTGO("DisplayInfo->stInRect[%d,%d,%d,%d]\n",DisplayInfo->stInRect.x,DisplayInfo->stInRect.y,DisplayInfo->stInRect.w,DisplayInfo->stInRect.h);
#if 0
  if(0) // because the DISP_GetDisplayInfo has bug, this function will crash
    {
    Win_ReviseOutRect(&pstInfo.stVirtaulScreen, &pstInfo.stOffsetInfo, 
                      &pstInfo.stFmtResolution, &pstInfo.stPixelFmtResolution,
                      &pstInfo.stVirtaulScreen, (mt_rect_s*)&DisplayInfo->stOutRect);
    }
  else
#endif    
    {
        DisplayInfo->stOutRect.x = DisplayInfo->stInRect.x;
        DisplayInfo->stOutRect.y = DisplayInfo->stInRect.y;
        DisplayInfo->stOutRect.w = DisplayInfo->stInRect.w;
        DisplayInfo->stOutRect.h = DisplayInfo->stInRect.h;
    }

    MT_INFO_MTGO("DisplayInfo->stOutRect[%d,%d,%d,%d]\n",DisplayInfo->stOutRect.x,DisplayInfo->stOutRect.y,DisplayInfo->stOutRect.w,DisplayInfo->stOutRect.h);


/*

	if (LayerInfo->LayerID == MTFB_SD_LOGO_ID)
	{
		LayerInfo->DisplayWidth = DisplayInfo->stOutRect.w;
		LayerInfo->DisplayHeight= DisplayInfo->stOutRect.h;
	}
*/

    s32Ret = ADP_Layer_AllocateDisplayBuffer(LayerInfo, DisplayInfo);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    } 
   //printf("ADP_Layer_AllocateDisplayBuffer %d us\n",(get_timer(0)-start)*10);
    return MT_SUCCESS;
}

#define OPTM_TC_WIDTH 720
static mt_s32 MTGO_ADP_CreateLayer(mt_u32  LayerID, MTGO_LAYER_INFO_S *pLayerInfo)
{
    mt_s32 s32Ret = 0;
    MTFB_COLOR_FMT_E LayerFmt;
    MTFB_ALPHA_S pstAlpha = {0};  
    MTGO_DISPLAY_INFO_S *DisplayInfo;
#ifndef _NO_USE_OPTM_DRV_     
	OPTM_GFX_GP_E enGpId;
#endif
  mt_u32 u32BPP;
  mt_u32 u32Stride;

	//MTFB_RECT stDefRect;
    /** 参数检查 */
    MTGO_ASSERT(MT_NULL_PTR != pLayerInfo);

    DisplayInfo = s_DisplayInfo + LayerID;

#ifndef _NO_USE_OPTM_DRV_      
    s32Ret = s_optm_gfx_ops.OPTM_GfxOpenLayer(LayerID, pLayerInfo->EnableOsdc);
#endif

#ifdef _USE_MTFB_IOC_   
   MTFB_GfxOpenLayer(LayerID, pLayerInfo->EnableOsdc);
#endif

	if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }  

    s32Ret = ADP_Layer_DisplayInfoInit(pLayerInfo, DisplayInfo);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
#ifndef _NO_USE_OPTM_DRV_ 

	if (LayerID <= MTFB_LAYER_HD_3)
	{
		enGpId = OPTM_GFX_GP_0;
	}
	else
	{
		enGpId = OPTM_GFX_GP_1;
	}
#endif  
	/*set layer in rect*/

#ifndef _NO_USE_OPTM_DRV_  
	s_optm_gfx_ops.OPTM_GfxSetLayerRect(LayerID, &DisplayInfo->stInRect);
	s_optm_gfx_ops.OPTM_GfxSetGpRect(enGpId, &DisplayInfo->stInRect);

    OPTM_GfxSetDispFMTSize(enGpId, (mt_rect_s *)(&DisplayInfo->stOutRect));

    s32Ret = OPTM_GpInitFromDisp(enGpId);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
#endif


	/**if work in wbc mode, open slv layer, then init slv gp*/

#ifndef _NO_USE_OPTM_DRV_
	if (MTFB_GFX_MODE_HD_WBC == OPTM_Get_GfxWorkMode())
	{
		MT_DISP_DISPLAY_INFO_S stInfo;
		
		DISP_GetDisplayInfo(MT_DRV_DISPLAY_0, &stInfo);
		if (stInfo.stFmtResolution.s32Width > OPTM_TC_WIDTH)
		{
			s_optm_gfx_ops.OPTM_GFX_SetTCFlag(MT_TRUE);
		}
		
		OPTM_GfxOpenSlvLayer(MTFB_LAYER_SD_0);
		OPTM_GpInitFromDisp(OPTM_GFX_GP_1);
	}
#endif

    pstAlpha.bAlphaEnable  = MT_TRUE;
    pstAlpha.bAlphaChannel = MT_FALSE;
    pstAlpha.u8Alpha0      = 0;
    pstAlpha.u8Alpha1      = 255;
    pstAlpha.u8GlobalAlpha = 255;   

#ifndef _NO_USE_OPTM_DRV_
    s_optm_gfx_ops.OPTM_GfxSetLayerAlpha(LayerID,&pstAlpha); 
    if (OPTM_Get_GfxWorkMode() == MTFB_GFX_MODE_HD_WBC)
    {
        OPTM_GFX_GP_E   enGfxGpId = OPTM_GFX_GP_1;
        OPTM_Wbc2Isr(&enGfxGpId, NULL);
        s_optm_gfx_ops.OPTM_GfxUpLayerReg(MTGO_LAYER_SD_0);
    }   
#endif

    ADP_Layer_MtgoFmtToMtfbFmt(pLayerInfo->PixelFormat, &LayerFmt);

#ifndef _NO_USE_OPTM_DRV_
    s_optm_gfx_ops.OPTM_GfxSetLayerDataFmt(LayerID, LayerFmt);
#endif

#ifdef _USE_MTFB_IOC_
    MTFB_GfxSetLayerAlpha(LayerID,&pstAlpha); 

    MTFB_GfxSetLayerDataFmt(LayerID, LayerFmt);
    printf("%s stInRect: %d %d %d %d \n", __FUNCTION__, 
        DisplayInfo->stInRect.x, DisplayInfo->stInRect.y, DisplayInfo->stInRect.w, DisplayInfo->stInRect.h);

    MTFB_GfxSetLayerRect(LayerID, &DisplayInfo->stInRect);
    //MTFB_GfxSetGpRect(enGpId, &DisplayInfo->stInRect);

    Surface_CalculateBpp0(pLayerInfo->PixelFormat, &u32BPP);
      if(u32BPP == 0)
        {
            MTGO_ERROR(s32Ret);
            return s32Ret;
        }
      u32Stride = ((u32BPP * pLayerInfo->DisplayWidth +0xf)&0xfffffff0)/ u32BPP;
      MTFB_GfxSetLayerStride(LayerID, u32Stride);  

      MTFB_GfxSetLayerAddr(LayerID, DisplayInfo->u32ScreenAddr);  
#ifdef CONFIG_MT_FPGA_GPE
      DisplayInfo->screen_pitch = u32Stride;
#endif	  
#endif

    return MT_SUCCESS;
}

static mt_void MTGO_ADP_DestroyLayer(mt_u32 LayerID)
{
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    mt_mmz_buf_s         stScreenBuf;

    DisplayInfo = s_DisplayInfo + LayerID;

    stScreenBuf.phyaddr = DisplayInfo->u32ScreenAddr;
    stScreenBuf.user_viraddr = (mt_u8 *)(DisplayInfo->u32VirScreenAddr);

    if(stScreenBuf.phyaddr != 0)
    {
        mt_mpi_mmz_free(&stScreenBuf);
    }

    /** 关闭图层设备 */
#ifndef _NO_USE_OPTM_DRV_    
    s_optm_gfx_ops.OPTM_GfxCloseLayer(LayerID);
#endif

#ifdef _USE_MTFB_IOC_
    MTFB_GfxCloseLayer(LayerID);
#endif

    return ;   
}


static mt_s32 MTGO_ADP_GetCanvasSurface(mt_u32 LayerID, const MTGO_LAYER_INFO_S *pLayerInfo, mt_handle *pSurface)
{
    mt_s32 s32Ret;
    MTGO_SURINFO_S       SurInfo;
    mt_mmz_buf_s         stSurfaceBuf;
    mt_u32 u32BPP;
    mt_handle hSurface;

     mt_char BufName[16] = "canvas_buf";
     mt_u32 buf_size = 0;
     
    Surface_CalculateBpp0(pLayerInfo->PixelFormat, &u32BPP);
    SurInfo.PixelFormat = pLayerInfo->PixelFormat;
    SurInfo.Width = pLayerInfo->CanvasWidth;
    SurInfo.Pitch[0] = (mt_u32)SurInfo.Width * u32BPP;
    SurInfo.Height = pLayerInfo->CanvasHeight;

    /*
    s32Ret = HI_MEM_Alloc(&stSurfaceBuf.u32StartPhyAddr, SurInfo.Pitch[0] * SurInfo.Height);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return MT_FAILURE;
    }
    */

    MT_INFO_MTGO("GetCanvasSurface: [%x][%d][%d][0x%x]\n", SurInfo.PixelFormat, SurInfo.Width, SurInfo.Height, SurInfo.Pitch[0]);

    ///*

    buf_size = stSurfaceBuf.bufsize = SurInfo.Pitch[0] * (mt_u32)(SurInfo.Height);
    //psMBuf.bufsize = 0x1 * 1024 * 1024; // 1M
    strncpy(stSurfaceBuf.bufname, BufName, sizeof(mt_char)*16);
    s32Ret = mt_mpi_mmz_malloc(&stSurfaceBuf);
    if(s32Ret != MT_SUCCESS)
    {
        MT_ASSERT(0);
        MTGO_ERROR(s32Ret);
        return MT_FAILURE;
    }        
    
   
   // stSurfaceBuf.u32StartPhyAddr = mt_mem_malloc(0, SurInfo.Pitch[0] * SurInfo.Height);
    

    //stSurfaceBuf.user_viraddr = stSurfaceBuf.user_viraddr;
    SurInfo.pPhyAddr[0] = stSurfaceBuf.phyaddr;
    SurInfo.pVirAddr[0]  =(mt_char*)stSurfaceBuf.user_viraddr;
    memset((mt_void*)SurInfo.pVirAddr[0], 0, buf_size);
    
    //memset((mt_void*)SurInfo.pPhyAddr[0],0,SurInfo.Pitch[0] * SurInfo.Height);
    //printf("MTGO_ADP_GetCanvasSurface: 0x%x, 0x%x\n",stSurfaceBuf.user_viraddr, stSurfaceBuf.phyaddr, buf_size);

    
  //*/
    s32Ret = MT_GO_CreateSurfaceFromMem(&SurInfo , &hSurface);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

    *pSurface = hSurface;
    return MT_SUCCESS;  
}

static mt_s32 MTGO_ADP_FreeCanvasSurface(mt_handle Surface)
{
    mt_mmz_buf_s         stSurfaceBuf;
	MT_PIXELDATA pPixelData = {0};
	
	(mt_void)Surface_LockSurface((MTGO_HANDLE)Surface, pPixelData);
	stSurfaceBuf.phyaddr = pPixelData[0].pPhyData;
	stSurfaceBuf.user_viraddr = pPixelData[0].pData;
	(mt_void)Surface_UnlockSurface((MTGO_HANDLE)Surface); 
	
	MT_GO_FreeSurface(Surface);
	mt_mpi_mmz_free(&stSurfaceBuf);
		
    return MT_SUCCESS;  
}

static mt_s32 MTGO_ADP_SetPos( mt_u32 LayerID, mt_u32 u32XStart, mt_u32 u32YStart)
{
#if 0
    MTFB_RECT InputRect;
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    HI_DISP_DISPLAY_INFO_S pstInfo;
    
    DisplayInfo = s_DisplayInfo + LayerID;  

    DISP_GetDisplayInfo(HI_DRV_DISPLAY_1, &pstInfo);

    /** 设置屏幕起始位置 */
    DisplayInfo->stScreenPos.s32XPos = u32XStart;
    DisplayInfo->stScreenPos.s32YPos = u32YStart;
    DisplayInfo->stInRect.x = DisplayInfo->stOutRect.w * u32XStart / pstInfo.stOrgRect.s32Width;
    DisplayInfo->stInRect.y = DisplayInfo->stOutRect.h * u32YStart / pstInfo.stOrgRect.s32Height;
    memcpy(&InputRect, &DisplayInfo->stInRect, sizeof(InputRect));
    OPTM_GfxSetLayerRect(LayerID, &InputRect);  
    OPTM_GfxUpLayerReg(LayerID);
#endif    
    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_GetPos(mt_u32 LayerID, mt_u32* pXStart, mt_u32* pYStart)
{
#if 0
    MTGO_DISPLAY_INFO_S *DisplayInfo;

    MTGO_ASSERT((MT_NULL_PTR != pXStart) && (MT_NULL_PTR != pYStart));
    
    DisplayInfo = s_DisplayInfo + LayerID;  

    /** 获取屏幕起始位置 */
    *pXStart = (mt_u32)DisplayInfo->stScreenPos.s32XPos;
    *pYStart = (mt_u32)DisplayInfo->stScreenPos.s32YPos;
#endif    
    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_SetDisplaySize(mt_u32 LayerID, mt_u32 u32Width, mt_u32 u32Height)
{
#if 0
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    
    DisplayInfo = s_DisplayInfo + LayerID;
    DisplayInfo->stOutRect.w = u32Width;
    DisplayInfo->stOutRect.h = u32Height;
  //  ADP_Layer_DefScreenSize(u32Width, u32Height, &DisplayInfo->stOutRect);
    OPTM_GfxSetGpRect(OPTM_GetGfxGpId(LayerID), &DisplayInfo->stOutRect);
    if (OPTM_Get_GfxWorkMode() == MTFB_GFX_MODE_HD_WBC)
    {
        OPTM_GFX_GP_E   enGfxGpId = OPTM_GFX_GP_1;
        OPTM_Wbc2Isr(&enGfxGpId, NULL);
        OPTM_GfxUpLayerReg(MTGO_LAYER_SD_0);
    }
    OPTM_GfxUpLayerReg(LayerID);   
#endif
    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_GetDisplaySize(mt_u32 LayerID, mt_u32 *pWidth, mt_u32 *pHeight)
{
#if 0
    MTGO_DISPLAY_INFO_S *DisplayInfo;

    DisplayInfo = s_DisplayInfo + LayerID;

    MTGO_ASSERT(pWidth != NULL);
    MTGO_ASSERT(pHeight != NULL);

    *pWidth = DisplayInfo->stOutRect.w;
    *pHeight = DisplayInfo->stOutRect.h;
#endif
     return  MT_SUCCESS;
}

static mt_s32 MTGO_ADP_SetScreenSize(mt_u32 LayerID, mt_u32 u32Width, mt_u32 u32Height)
{
#if 0
    MTGO_DISPLAY_INFO_S *DisplayInfo;
 //   MTFB_RECT OutRect;
//    MTFB_RECT InputRect;

    DisplayInfo = s_DisplayInfo + LayerID;

    DisplayInfo->stOutRect.w = u32Width;
    DisplayInfo->stOutRect.h = u32Height;

  // memcpy(&InputRect, &DisplayInfo->stInRect, sizeof(InputRect));
   // memcpy(&OutRect, &DisplayInfo->stOutRect, sizeof(OutRect));
  //  ADP_Layer_DefScreenSize(u32Width, u32Height, &DisplayInfo->stOutRect);
   // s_stDrvOps.MTFB_DRV_SetLayerInRect(pLayerRec->LayerID, &InputRect);
    OPTM_GfxSetGpRect(OPTM_GetGfxGpId(LayerID), &DisplayInfo->stOutRect);
    if (OPTM_Get_GfxWorkMode() == MTFB_GFX_MODE_HD_WBC)
    {
        OPTM_GFX_GP_E   enGfxGpId = OPTM_GFX_GP_1;
        OPTM_Wbc2Isr(&enGfxGpId, NULL);
        OPTM_GfxUpLayerReg(MTGO_LAYER_SD_0);
    }
   
    OPTM_GfxUpLayerReg(LayerID);   
#endif
    return MT_SUCCESS;

}

static mt_s32 MTGO_ADP_GetScreenSize(mt_u32 LayerID, mt_u32 *pWidth, mt_u32 *pHeight)
{
#if 0
    //mt_s32 s32Ret;
    MTGO_DISPLAY_INFO_S *DisplayInfo;

    DisplayInfo = s_DisplayInfo + LayerID;
    MTGO_ASSERT(pWidth != NULL);
    MTGO_ASSERT(pHeight != NULL);

    *pWidth = DisplayInfo->stOutRect.w;
    *pHeight = DisplayInfo->stOutRect.h;
#endif    
    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_SetLayerAlpha(mt_u32 LayerID, const MTGO_LAYER_ALPHA_S *pAlphaInfo)
{
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    
    DisplayInfo = s_DisplayInfo + LayerID;
    
    /** 设置全局ALPHA */
    DisplayInfo->stAlpha.bAlphaEnable = pAlphaInfo->bAlphaEnable;
    DisplayInfo->stAlpha.bAlphaChannel = pAlphaInfo->bAlphaChannel;
    DisplayInfo->stAlpha.u8GlobalAlpha = pAlphaInfo->GlobalAlpha;
    DisplayInfo->stAlpha.u8Alpha0 = pAlphaInfo->Alpha0;
    DisplayInfo->stAlpha.u8Alpha1 = pAlphaInfo->Alpha1; 
    DisplayInfo->stAlpha.bRegionAlphaEnable = pAlphaInfo->bRegionAlphaEnable;
    DisplayInfo->stAlpha.u8RegionAlpha = pAlphaInfo->u8RegionAlpha;

#ifndef _NO_USE_OPTM_DRV_
    s_optm_gfx_ops.OPTM_GfxSetLayerAlpha(LayerID, &DisplayInfo->stAlpha);
    if (OPTM_Get_GfxWorkMode() == MTFB_GFX_MODE_HD_WBC)
    {
        OPTM_GFX_GP_E   enGfxGpId = OPTM_GFX_GP_1;
        OPTM_Wbc2Isr(&enGfxGpId, NULL);
        s_optm_gfx_ops.OPTM_GfxUpLayerReg(MTGO_LAYER_SD_0);
    }
    s_optm_gfx_ops.OPTM_GfxUpLayerReg(LayerID); 
#endif   

#ifdef _USE_MTFB_IOC_
    MTFB_GfxSetLayerAlpha(LayerID, &DisplayInfo->stAlpha);
#endif

   return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_GetLayerAlpha(mt_u32 LayerID, MTGO_LAYER_ALPHA_S *pAlphaInfo)
{
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    
    MTGO_ASSERT(pAlphaInfo != NULL);

    DisplayInfo = s_DisplayInfo + LayerID;

    pAlphaInfo->bAlphaEnable = DisplayInfo->stAlpha.bAlphaEnable;
    pAlphaInfo->bAlphaChannel = DisplayInfo->stAlpha.bAlphaChannel;
    pAlphaInfo->GlobalAlpha = DisplayInfo->stAlpha.u8GlobalAlpha;
    pAlphaInfo->Alpha0 = DisplayInfo->stAlpha.u8Alpha0;
    pAlphaInfo->Alpha1 = DisplayInfo->stAlpha.u8Alpha1;    
    pAlphaInfo->bRegionAlphaEnable = DisplayInfo->stAlpha.bRegionAlphaEnable;
    pAlphaInfo->u8RegionAlpha = DisplayInfo->stAlpha.u8RegionAlpha;
    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_SetLayerColorkey(mt_u32 LayerID, MT_BOOL bEnable, mt_u32 Key)
{
#ifndef MTGO_CODE_CUT
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    
    DisplayInfo = s_DisplayInfo + LayerID;

    /** 设置COLORKEY */
    DisplayInfo->stColorKey.u32Key = Key;
    DisplayInfo->stColorKey.bKeyEnable = bEnable;

#ifndef _NO_USE_OPTM_DRV_
    s_optm_gfx_ops.OPTM_GfxSetLayKeyMask(LayerID, &DisplayInfo->stColorKey);
    s_optm_gfx_ops.OPTM_GfxUpLayerReg(LayerID); 
#endif

#ifdef _USE_MTFB_IOC_
    MTFB_GfxSetLayKeyMask(LayerID, &DisplayInfo->stColorKey);
    //MTFB_GfxUpLayerReg(LayerID); 
#endif    

#endif

    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_GetLayerColorKey(mt_u32 LayerID, MT_BOOL *pbEnable, mt_u32 *pKey)
{
#ifndef MTGO_CODE_CUT
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    
    DisplayInfo = s_DisplayInfo + LayerID;

    *pKey = DisplayInfo->stColorKey.u32Key;
    *pbEnable = DisplayInfo->stColorKey.bKeyEnable;
#endif    
    return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_ShowLayer(mt_u32 LayerID, MT_BOOL bVisbile)
{
    /** 显示或隐藏图层 */
#ifndef _NO_USE_OPTM_DRV_
    s_optm_gfx_ops.OPTM_GfxSetEnable(LayerID, bVisbile);  
    if (OPTM_Get_GfxWorkMode() == MTFB_GFX_MODE_HD_WBC)
    {
        OPTM_GFX_GP_E   enGfxGpId = OPTM_GFX_GP_1;
        OPTM_Wbc2Isr(&enGfxGpId, NULL);
        s_optm_gfx_ops.OPTM_GfxUpLayerReg(MTGO_LAYER_SD_0);
    }  
#endif

#ifdef _USE_MTFB_IOC_
    MTFB_GfxSetEnable(LayerID, bVisbile);  
#endif 

    return MT_SUCCESS;    
}

static mt_s32 MTGO_ADP_SetLayerSurface(mt_handle Layer, const MTGO_SURFACE_S *pSurface, MT_RECT *pRect)
{    
    mt_s32 s32Ret;
    TDE_HANDLE       s32Handle;
    TDE2_SURFACE_S   stSrcSur = {0};
    TDE2_SURFACE_S   stDstSur = {0};
    TDE2_RECT_S      stSrcRect = {0};
    TDE2_RECT_S      stDstRect = {0};
    TDE2_OPT_S       stOpt = {0};
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_SURFACE_S *LayerSurface;
#ifdef CFG_MTGO_PROC_SUPPORT
    MTFB_PROC_SURFACE_S stProcInfo;
#endif

    LayerSurface = (MTGO_SURFACE_S*)pSurface;
    
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    DisplayInfo = s_DisplayInfo + pLayerRec->LayerID;

    if (pRect->x + pRect->w > pLayerRec->LayerInfo.CanvasWidth)
    {
        pRect->w = pLayerRec->LayerInfo.CanvasWidth - pRect->x;
    }
    if (pRect->y + pRect->h > pLayerRec->LayerInfo.CanvasHeight)
    {
        pRect->h = pLayerRec->LayerInfo.CanvasHeight - pRect->y;
    }
       
    stSrcRect.s32Xpos = pRect->x;
    stSrcRect.s32Ypos = pRect->y;
    stSrcRect.u32Width = (mt_u32)(pRect->w);
    stSrcRect.u32Height = (mt_u32)(pRect->h);
    
    stSrcSur.u32PhyAddr = LayerSurface->Data[0].pPhyData;
    stSrcSur.u32Stride = ((mt_u32)(pLayerRec->LayerInfo.CanvasWidth) * LayerSurface->Data[0].Bpp + 0xf) &0xfffffff0;
    stSrcSur.u32Width = (mt_u32)(pLayerRec->LayerInfo.CanvasWidth);
    stSrcSur.u32Height = (mt_u32)(pLayerRec->LayerInfo.CanvasHeight);

    stSrcSur.bAlphaMax255 = MT_TRUE;
    stSrcSur.u8Alpha0 = 0;
    stSrcSur.u8Alpha1 = 0xff;

    s32Ret = ADP_ConvertFormat(LayerSurface->PixelFormat, &stSrcSur.enColorFmt);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
    
    if (pLayerRec->LayerInfo.CanvasWidth != pLayerRec->LayerInfo.DisplayWidth ||
        pLayerRec->LayerInfo.CanvasHeight!= pLayerRec->LayerInfo.DisplayHeight)
    {
	    stSrcRect.s32Xpos = 0;
	    stSrcRect.s32Ypos = 0;
	    stSrcRect.u32Width = (mt_u32)(pLayerRec->LayerInfo.CanvasWidth);
	    stSrcRect.u32Height = (mt_u32)(pLayerRec->LayerInfo.CanvasHeight); 
        stDstRect.s32Xpos = 0;
        stDstRect.s32Ypos = 0;
        stDstRect.u32Width = (mt_u32)(pLayerRec->LayerInfo.DisplayWidth);
        stDstRect.u32Height = (mt_u32)(pLayerRec->LayerInfo.DisplayHeight);

        stOpt.bResize = MT_TRUE;
    }
    else 
    {
        stDstRect = stSrcRect;
    }
    
    memcpy((mt_void*)&stDstSur, (mt_void*)&stSrcSur, sizeof(TDE2_SURFACE_S));
    
    stDstSur.u32PhyAddr = DisplayInfo->u32ScreenAddr;
    stDstSur.u32Stride = ((mt_u32)(pLayerRec->LayerInfo.DisplayWidth) * LayerSurface->Data[0].Bpp +0xf)&0xfffffff0;
    stDstSur.u32Width = (mt_u32)(pLayerRec->LayerInfo.DisplayWidth);
    stDstSur.u32Height = (mt_u32)(pLayerRec->LayerInfo.DisplayHeight);

    stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
    stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_RGB;   

#ifndef _USE_MTFB_CMP_
   // stOpt.bResize = MT_TRUE;
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle < 0)
    {
        MTGO_ERROR(s32Handle);
        return s32Handle;
    }

#if 0
    Debug("stSrcRect.x=%d,stSrcRect.y=%d,stSrcRect.w=%d,stSrcRect.h=%d\n",stSrcRect.s32Xpos,stSrcRect.s32Ypos,stSrcRect.u32Width,stSrcRect.u32Height);
    Debug("stDstRect.x=%d,stDstRect.y=%d,stDstRect.w=%d,stDstRect.h=%d\n",stDstRect.s32Xpos,stDstRect.s32Ypos,stDstRect.u32Width,stDstRect.u32Height);
    Debug("stSrcSur.enColorFmt:%d,stSrcSur.u32Stride:%d,stSrcSur.u32Width:%d,stSrcSur.u32Height:%d\n",stSrcSur.enColorFmt,stSrcSur.u32Stride,stSrcSur.u32Width,stSrcSur.u32Height);
    Debug("stDstSur.enColorFmt:%d,stDstSur.u32Stride:%d,stDstSur.u32Width:%d,stDstSur.u32Height:%d\n",stDstSur.enColorFmt,stDstSur.u32Stride,stDstSur.u32Width,stDstSur.u32Height);
#endif
    s32Ret = MT_TDE2_Bitblit(s32Handle, &stDstSur, &stDstRect, &stSrcSur, &stSrcRect, &stDstSur,  &stDstRect, &stOpt);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
		//for bug 104373
		MT_TDE2_EndJob(s32Handle, MT_FALSE, MT_TRUE, 100);
        return s32Ret;
    }  
    
    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, MT_TRUE, 100);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    } 
#else
    MTFB_GfxCmpDecmpProcess(pLayerRec->LayerID, stSrcSur.u32PhyAddr, stSrcSur.u32Width, stSrcSur.u32Height, pLayerRec->LayerInfo.CanvasWidth, stDstSur.u32PhyAddr);
#endif

#if 0
   pstLayerAttr.u32LayerID = pLayerRec->LayerID;
   pstLayerAttr.u32LayerAddr = DisplayInfo->u32ScreenAddr;
 
   pstLayerAttr.u32Stride = (LayerSurface->Data[0].Bpp * (mt_u32)(pLayerRec->LayerInfo.DisplayWidth) +0xf)&0xfffffff0;

   // calc the hw osd stride, pixel nubmer, 16 byte aligned
   MT_ASSERT(LayerSurface->Data[0].Bpp >= 1);
   pstLayerAttr.hwStride = pstLayerAttr.u32Stride / (LayerSurface->Data[0].Bpp);
   
   memcpy(&pstLayerAttr.stInRect, &DisplayInfo->stInRect, sizeof(pstLayerAttr.stInRect));
   memcpy(&pstLayerAttr.stOutRect, &DisplayInfo->stOutRect, sizeof(pstLayerAttr.stOutRect));
   //update register
   ADP_Layer_RefreshLayer(&pstLayerAttr);
#else
  MTFB_GfxSetEnable(pLayerRec->LayerID, MT_TRUE);
#endif

#ifdef CFG_MTGO_PROC_SUPPORT
  stProcInfo.addr = stSrcSur.u32PhyAddr;
  stProcInfo.stride = stSrcSur.u32Stride;
  stProcInfo.width = stSrcSur.u32Width;
  stProcInfo.height = stSrcSur.u32Height;
  ADP_Layer_MtgoFmtToMtfbFmt(LayerSurface->PixelFormat,(MTFB_COLOR_FMT_E *) &stProcInfo.fmt);
  MTFB_GfxProcSurfaceInfo(pLayerRec->LayerID, &stProcInfo);
#endif
   return MT_SUCCESS;
}


static mt_s32 MTGO_ADP_SetLayerSurface_Zoom(mt_handle Layer, const MTGO_SURFACE_S *pSurface, MT_RECT *pRect, MT_RECT *pZoomRect)
{    
    mt_s32 s32Ret;
    TDE_HANDLE       s32Handle;
    TDE2_SURFACE_S   stSrcSur = {0};
    TDE2_SURFACE_S   stDstSur = {0};
    TDE2_RECT_S      stSrcRect = {0};
    TDE2_RECT_S      stDstRect = {0};
    TDE2_RECT_S      stFullRect = {0};
    static TDE2_RECT_S   stLastDstRect = {0};
    TDE2_OPT_S       stOpt = {0};
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_SURFACE_S *LayerSurface;
#ifdef CFG_MTGO_PROC_SUPPORT
    MTFB_PROC_SURFACE_S stProcInfo;
#endif

    LayerSurface = (MTGO_SURFACE_S*)pSurface;
    
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    DisplayInfo = s_DisplayInfo + pLayerRec->LayerID;

    if (pRect->x + pRect->w > pLayerRec->LayerInfo.CanvasWidth)
    {
        pRect->w = pLayerRec->LayerInfo.CanvasWidth - pRect->x;
    }
    if (pRect->y + pRect->h > pLayerRec->LayerInfo.CanvasHeight)
    {
        pRect->h = pLayerRec->LayerInfo.CanvasHeight - pRect->y;
    }

    if(NULL != pZoomRect)
    {
        if (pZoomRect->x + pZoomRect->w > pLayerRec->LayerInfo.DisplayWidth)
        {
            pZoomRect->w = pLayerRec->LayerInfo.DisplayWidth - pZoomRect->x;
        }
        if (pZoomRect->y + pZoomRect->h > pLayerRec->LayerInfo.DisplayHeight)
        {
            pZoomRect->h = pLayerRec->LayerInfo.DisplayHeight - pZoomRect->y;
        }
    }
    
    stSrcRect.s32Xpos = pRect->x;
    stSrcRect.s32Ypos = pRect->y;
    stSrcRect.u32Width = (mt_u32)(pRect->w);
    stSrcRect.u32Height = (mt_u32)(pRect->h);
    
    stSrcSur.u32PhyAddr = LayerSurface->Data[0].pPhyData;
    stSrcSur.u32Stride = ((mt_u32)(pLayerRec->LayerInfo.CanvasWidth) * LayerSurface->Data[0].Bpp + 0xf) &0xfffffff0;
    stSrcSur.u32Width = (mt_u32)(pLayerRec->LayerInfo.CanvasWidth);
    stSrcSur.u32Height = (mt_u32)(pLayerRec->LayerInfo.CanvasHeight);

    stSrcSur.bAlphaMax255 = MT_TRUE;
    stSrcSur.u8Alpha0 = 0;
    stSrcSur.u8Alpha1 = 0xff;

    s32Ret = ADP_ConvertFormat(LayerSurface->PixelFormat, &stSrcSur.enColorFmt);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
    

    stSrcRect.s32Xpos = 0;
    stSrcRect.s32Ypos = 0;
    stSrcRect.u32Width = (mt_u32)(pLayerRec->LayerInfo.CanvasWidth);
    stSrcRect.u32Height = (mt_u32)(pLayerRec->LayerInfo.CanvasHeight);

    if(NULL != pZoomRect)
    {
        stDstRect.s32Xpos = pZoomRect->x;
        stDstRect.s32Ypos = pZoomRect->y;
        stDstRect.u32Width = (mt_u32)(pZoomRect->w);
        stDstRect.u32Height = (mt_u32)(pZoomRect->h);
    }
    else
    {
        stDstRect.s32Xpos = 0;
        stDstRect.s32Ypos = 0;
        stDstRect.u32Width = (mt_u32)(pLayerRec->LayerInfo.DisplayWidth);
        stDstRect.u32Height = (mt_u32)(pLayerRec->LayerInfo.DisplayHeight);
    }

    stOpt.bResize = MT_TRUE;
    
    memcpy((mt_void*)&stDstSur, (mt_void*)&stSrcSur, sizeof(TDE2_SURFACE_S));
    
    stDstSur.u32PhyAddr = DisplayInfo->u32ScreenAddr;
    stDstSur.u32Stride = ((mt_u32)(pLayerRec->LayerInfo.DisplayWidth) * LayerSurface->Data[0].Bpp +0xf)&0xfffffff0;
    stDstSur.u32Width = (mt_u32)(pLayerRec->LayerInfo.DisplayWidth);
    stDstSur.u32Height = (mt_u32)(pLayerRec->LayerInfo.DisplayHeight);

    stOpt.enFilterMode = TDE2_FILTER_MODE_COLOR;
    stOpt.enDeflickerMode = TDE2_DEFLICKER_MODE_RGB;   

#ifndef _USE_MTFB_CMP_
   // stOpt.bResize = MT_TRUE;
    s32Handle = MT_TDE2_BeginJob();
    if (s32Handle < 0)
    {
        MTGO_ERROR(s32Handle);
        return s32Handle;
    }

#if 0
    Debug("stSrcRect.x=%d,stSrcRect.y=%d,stSrcRect.w=%d,stSrcRect.h=%d\n",stSrcRect.s32Xpos,stSrcRect.s32Ypos,stSrcRect.u32Width,stSrcRect.u32Height);
    Debug("stDstRect.x=%d,stDstRect.y=%d,stDstRect.w=%d,stDstRect.h=%d\n",stDstRect.s32Xpos,stDstRect.s32Ypos,stDstRect.u32Width,stDstRect.u32Height);
    Debug("stSrcSur.enColorFmt:%d,stSrcSur.u32Stride:%d,stSrcSur.u32Width:%d,stSrcSur.u32Height:%d\n",stSrcSur.enColorFmt,stSrcSur.u32Stride,stSrcSur.u32Width,stSrcSur.u32Height);
    Debug("stDstSur.enColorFmt:%d,stDstSur.u32Stride:%d,stDstSur.u32Width:%d,stDstSur.u32Height:%d\n",stDstSur.enColorFmt,stDstSur.u32Stride,stDstSur.u32Width,stDstSur.u32Height);
#endif

    if((stLastDstRect.s32Xpos != stDstRect.s32Xpos)
        || (stLastDstRect.s32Ypos != stDstRect.s32Ypos)
        || (stLastDstRect.u32Width != stDstRect.u32Width)
        || (stLastDstRect.u32Height != stDstRect.u32Height))
    {
        stFullRect.u32Width = pLayerRec->LayerInfo.DisplayWidth;
        stFullRect.u32Height = pLayerRec->LayerInfo.DisplayHeight;    
        MT_TDE2_QuickFill(s32Handle, &stDstSur, &stFullRect, 0x00000000);

        stLastDstRect = stDstRect;
    }
        
    s32Ret = MT_TDE2_Bitblit(s32Handle, &stDstSur, &stDstRect, &stSrcSur, &stSrcRect, &stDstSur,  &stDstRect, &stOpt);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
		//for bug 104373
		MT_TDE2_EndJob(s32Handle, MT_FALSE, MT_TRUE, 100);
        return s32Ret;
    }  
    
    s32Ret = MT_TDE2_EndJob(s32Handle, MT_FALSE, MT_TRUE, 100);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    } 
#else
    MTFB_GfxCmpDecmpProcess(pLayerRec->LayerID, stSrcSur.u32PhyAddr, stSrcSur.u32Width, stSrcSur.u32Height, pLayerRec->LayerInfo.CanvasWidth, stDstSur.u32PhyAddr);
#endif

#if 0
   pstLayerAttr.u32LayerID = pLayerRec->LayerID;
   pstLayerAttr.u32LayerAddr = DisplayInfo->u32ScreenAddr;
 
   pstLayerAttr.u32Stride = (LayerSurface->Data[0].Bpp * (mt_u32)(pLayerRec->LayerInfo.DisplayWidth) +0xf)&0xfffffff0;

   // calc the hw osd stride, pixel nubmer, 16 byte aligned
   MT_ASSERT(LayerSurface->Data[0].Bpp >= 1);
   pstLayerAttr.hwStride = pstLayerAttr.u32Stride / (LayerSurface->Data[0].Bpp);
   
   memcpy(&pstLayerAttr.stInRect, &DisplayInfo->stInRect, sizeof(pstLayerAttr.stInRect));
   memcpy(&pstLayerAttr.stOutRect, &DisplayInfo->stOutRect, sizeof(pstLayerAttr.stOutRect));
   //update register
   ADP_Layer_RefreshLayer(&pstLayerAttr);
#else
  MTFB_GfxSetEnable(pLayerRec->LayerID, MT_TRUE);
#endif

#ifdef CFG_MTGO_PROC_SUPPORT
  stProcInfo.addr = stSrcSur.u32PhyAddr;
  stProcInfo.stride = stSrcSur.u32Stride;
  stProcInfo.width = stSrcSur.u32Width;
  stProcInfo.height = stSrcSur.u32Height;
  ADP_Layer_MtgoFmtToMtfbFmt(LayerSurface->PixelFormat,(MTFB_COLOR_FMT_E *) &stProcInfo.fmt);
  MTFB_GfxProcSurfaceInfo(pLayerRec->LayerID, &stProcInfo);
#endif
   return MT_SUCCESS;
}

static mt_s32 MTGO_ADP_SetLayerPalette(mt_u32 LayerID, mt_u32 *pPalette, mt_u32 u32Start, mt_u32 u32Len)
{
    mt_u32 i = 0, u32Color = 0;

    for (i = 0; i < u32Len; i++)	
    {
        u32Color = *pPalette++;
#ifdef _USE_MTFB_IOC_
        MTFB_GfxSetColorReg(LayerID, u32Start++, u32Color, MT_TRUE);
    }
#endif

   return MT_SUCCESS;
}
static mt_s32 MTGO_ADP_WaitSync(mt_void)
{
  return MTFB_GfxWaitSync();
}

#ifdef CONFIG_MT_FPGA_GPE
void MTGO_ADP_Get_LayerInfo(mt_u32 LayerID, phys_addr_t  *phyaddr,ulong *usr_viraddr,mt_u32 *pitch)
{
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    DisplayInfo = s_DisplayInfo + LayerID;
    if(phyaddr)
    {
        *phyaddr = DisplayInfo->u32ScreenAddr;
    }
    if(usr_viraddr)
    {
        *usr_viraddr = DisplayInfo->u32VirScreenAddr;
    }
#ifdef CONFIG_MT_FPGA_GPE     
    if(pitch)
    { 
        *pitch = DisplayInfo->screen_pitch;
    }
#endif    
}

#endif

mt_s32 MTGO_ADP_CreateVideoDevice(MTGO_LAYER_ADP_S *thiz)
{
    /** 参数检查 */
    if (MT_NULL_PTR == thiz)
    {
        MTGO_ADP_SetError(MT_FAILURE);
        return MT_FAILURE;
    }
    /** 为图层操作函数集结构赋值 */
    thiz->InitDisplay   =    MTGO_ADP_InitDisplay;
    thiz->DeinitDisplay =    MTGO_ADP_DeinitDisplay;
    thiz->GetDefautParam =   MTGO_ADP_GetDefaultParam;
    thiz->CreateLayer  =     MTGO_ADP_CreateLayer;
    thiz->DestroyLayer =     MTGO_ADP_DestroyLayer;
    thiz->SetLayerSurface =  MTGO_ADP_SetLayerSurface;
    thiz->SetLayerSurface_Zoom =  MTGO_ADP_SetLayerSurface_Zoom;
    thiz->GetCanvasSurface = MTGO_ADP_GetCanvasSurface;
	thiz->FreeCanvasSurface = MTGO_ADP_FreeCanvasSurface;
   // thiz->SetLayerZorder =   MTGO_ADP_SetLayerZorder;
   // thiz->GetLayerZorder =   MTGO_ADP_GetLayerZorder;
    thiz->ShowLayer  =       MTGO_ADP_ShowLayer;  
   // thiz->GetLayerShowState= MTGO_ADP_GetLayerStatus;
    thiz->SetLayerAlpha =    MTGO_ADP_SetLayerAlpha;
    thiz->GetLayerAlpha =    MTGO_ADP_GetLayerAlpha;
#ifndef MTGO_CODE_CUT
    thiz->SetLayerColorKey = MTGO_ADP_SetLayerColorkey;
    thiz->GetLayerColorKey = MTGO_ADP_GetLayerColorKey;
    thiz->SetPos =           MTGO_ADP_SetPos;
    thiz->GetPos =           MTGO_ADP_GetPos;
	thiz->SetScreenSize =    MTGO_ADP_SetScreenSize;
	thiz->GetScreenSize =    MTGO_ADP_GetScreenSize;
	thiz->SetDisplaySize =   MTGO_ADP_SetDisplaySize;
	thiz->GetDisplaySize =   MTGO_ADP_GetDisplaySize;	
    thiz->CapabilityInquire= MTGO_ADP_CapabilityInquire;
    thiz->SetLayerPalette =  MTGO_ADP_SetLayerPalette;	
    thiz->LayerCount =       MTGO_LAYER_COUNT;
    thiz->WaitSync = MTGO_ADP_WaitSync;
#endif
    return MT_SUCCESS;
}

