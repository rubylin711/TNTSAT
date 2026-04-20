/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <string.h>
 
#include "mt_go_gdev.h"
#include "mtgo_gdev.h"
#include "adp_gfx.h"
#include "adp_layer.h"
#include "mtgo_surface.h"
#include "mt_tde_api.h"


static MTGO_LAYER_ADP_S s_struGdevDevice;

/** 图层操作函数集全局指针 */
static MTGO_LAYER_ADP_S* s_pGdevDevice = MT_NULL_PTR;

#define CHECK_GDEVINIT() \
do \
{  \
    if (UN_INIT_STATE == s_InitLayerCount) \
    { \
        MTGO_ERROR(MTGO_ERR_NOTINIT); \
        return MTGO_ERR_NOTINIT; \
    } \
} \
while(0)

#define CHECK_GDEV_NULLPTR(ptr) \
do\
{\
    if (NULL == ptr)\
    {\
        MTGO_ERROR(MTGO_ERR_NULLPTR);\
        return MTGO_ERR_NULLPTR;\
    }\
}\
while (0)

//static HIFB_DRV_OPS_S s_stDrvOps;
static mt_s32 s_InitLayerCount = 0;
//static mt_s32 s_InitDisplayCount = 0;
/** 图层数据数组 */
static MTGO_LAYER_REC_S s_LayerRec[MTGO_LAYER_MAXCOUNT]; 

#ifndef MTGO_LAYER_MAXCOUNT
static mt_s32 MTGO_CheckCanvas(const MTGO_LAYER_INFO_S *pLayerInfo,const MTGO_LAYER_CAP_S* pstruLayerCap)
{
    /** 如果非0，并且不在范围内无效*/
    if (pLayerInfo->CanvasWidth * pLayerInfo->CanvasHeight == 0)
        return MT_SUCCESS;
        
    if ((pLayerInfo->CanvasWidth < pstruLayerCap->MinWidth) || (pLayerInfo->CanvasHeight < pstruLayerCap->MinHeight) || \
        (pLayerInfo->CanvasWidth > pstruLayerCap->MaxWidth) || (pLayerInfo->CanvasHeight > pstruLayerCap->MaxHeight))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }
    return MT_SUCCESS;
}

static mt_s32 MTGO_CheckDisplay(const MTGO_LAYER_INFO_S *pLayerInfo, const MTGO_LAYER_CAP_S* pstruLayerCap)
{
    if (pLayerInfo->LayerFlushType == MTGO_LAYER_BUFFER_SINGLE)
    {
        return MT_SUCCESS;
    }
    
    if ((pLayerInfo->DisplayWidth < pstruLayerCap->MinWidth) || (pLayerInfo->DisplayHeight < pstruLayerCap->MinHeight) ||\
    (pLayerInfo->DisplayWidth > pstruLayerCap->MaxWidth) || (pLayerInfo->DisplayHeight > pstruLayerCap->MaxHeight))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }
    return MT_SUCCESS;
}


static mt_s32 MTGO_CheckScreen(MTGO_LAYER_INFO_S *pLayerInfo, const MTGO_LAYER_CAP_S* pstruLayerCap)
{
    if ((pLayerInfo->ScreenWidth < pstruLayerCap->MinWidth) || (pLayerInfo->ScreenHeight < pstruLayerCap->MinHeight) ||\
    (pLayerInfo->ScreenWidth > pstruLayerCap->MaxWidth) || (pLayerInfo->ScreenHeight > pstruLayerCap->MaxHeight))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }

    /** 如果不支持缩放，必须保证screen size is same with display size
     for x5v200 vo only support vertical scale, so we should check screen width and display width*/
    if (pstruLayerCap->bVoScale !=  MT_TRUE)            
    {
        if (pLayerInfo->LayerFlushType != MTGO_LAYER_BUFFER_SINGLE)
        {
            /** 多buffer模式下与display buffer相同*/
            pLayerInfo->ScreenHeight = pLayerInfo->DisplayHeight;
            pLayerInfo->ScreenWidth  = pLayerInfo->DisplayWidth;
        }
        //MTGO_TRACE("vo unsupport scale, the display size will be the sampe with display size\n");
    }
    return MT_SUCCESS;
}

static mt_s32 MTGO_CheckFlushType(MTGO_LAYER_INFO_S *pLayerInfo)
{
    if ((pLayerInfo->LayerFlushType & MTGO_LAYER_BUFFER_OVER ) == MTGO_LAYER_BUFFER_OVER)
    {
        pLayerInfo->LayerFlushType = (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_OVER;	
    }
    else if ((pLayerInfo->LayerFlushType & MTGO_LAYER_BUFFER_TRIPLE ) == MTGO_LAYER_BUFFER_TRIPLE)
    {
        pLayerInfo->LayerFlushType = (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_TRIPLE;
    }
    else if((pLayerInfo->LayerFlushType & MTGO_LAYER_BUFFER_DOUBLE) == MTGO_LAYER_BUFFER_DOUBLE)
    {
        pLayerInfo->LayerFlushType = (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_DOUBLE;
    }
    else if((pLayerInfo->LayerFlushType & MTGO_LAYER_BUFFER_SINGLE) == MTGO_LAYER_BUFFER_SINGLE)
    {
        pLayerInfo->LayerFlushType = (MTGO_LAYER_FLUSHTYPE_E)MTGO_LAYER_BUFFER_SINGLE;
    }
    else
    {
        MTGO_ERROR(MTGO_ERR_INVFLUSHTYPE);
        return MTGO_ERR_INVFLUSHTYPE;
    }
    
    return  MT_SUCCESS;
}

static MT_BOOL LAYER_CheckFmt(mt_u32 LayerID, MTGO_PF_E enFmt)
{
    mt_s32 i;
    MTGO_LAYER_CAP_S* pstruLayerCap = MT_NULL_PTR;

    s_pGdevDevice->CapabilityInquire(LayerID, &pstruLayerCap);
    for (i = 0; i < pstruLayerCap->u8FmtCount; i++)
    {
        if (enFmt == *(pstruLayerCap->enPixelFmt + i))
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

static mt_s32 MTGO_CheckParam(MTGO_LAYER_INFO_S *pLayerInfo)
{
    MTGO_LAYER_CAP_S* pstruLayerCap = MT_NULL_PTR;
    mt_s32 s32Ret;
    
    if (pLayerInfo->AntiLevel >= MTGO_LAYER_DEFLICKER_BUTT)
    {
        MTGO_ERROR(MTGO_ERR_INVANILEVEL);
        return MTGO_ERR_INVANILEVEL;
    }

    if (pLayerInfo->PixelFormat >= MTGO_PF_BUTT)
    {
        MTGO_ERROR(MTGO_ERR_INVPIXELFMT);
        return MTGO_ERR_INVPIXELFMT;
    }

    if (pLayerInfo->LayerID >= MTGO_LAYER_BUTT)
    {
        MTGO_ERROR(MTGO_ERR_INVLAYERID);
        return MTGO_ERR_INVLAYERID;
    }
    
    if (MT_TRUE != LAYER_CheckFmt(pLayerInfo->LayerID, pLayerInfo->PixelFormat))
    {
        MTGO_ERROR(MTGO_ERR_INVPIXELFMT);
        return MTGO_ERR_INVPIXELFMT;
    }

    s_pGdevDevice->CapabilityInquire(pLayerInfo->LayerID, &pstruLayerCap);

    s32Ret = MTGO_CheckFlushType(pLayerInfo);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

    /** 非单buffer模式下，display buffer could not be 0*/
    s32Ret = MTGO_CheckCanvas(pLayerInfo, pstruLayerCap);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    
    s32Ret = MTGO_CheckDisplay(pLayerInfo, pstruLayerCap);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

    s32Ret = MTGO_CheckScreen(pLayerInfo, pstruLayerCap);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

    return MT_SUCCESS;
}

#endif


static mt_s32 MTGO_GetLayerSize(mt_handle pLayer, mt_s32* pWidth, mt_s32* pHeight)
{
    /** canvas */
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerInstance = (MTGO_LAYER_REC_S*)pLayer;

    MTGO_ASSERT (pWidth != NULL);
    MTGO_ASSERT (pHeight != NULL);
    if (pLayerInstance->hLayerSurface)
    {
        s32Ret = Surface_GetSurfaceSize(pLayerInstance->hLayerSurface, pWidth, pHeight);
        if (MT_SUCCESS != s32Ret)
        {
            MTGO_ERROR(s32Ret);
            return MTGO_ERR_INVLAYERSURFACE;
        }
        return MT_SUCCESS;
    }

    return MT_FAILURE;
}


mt_s32 MTGO_InitDisplay(mt_void)
{
    mt_s32 s32Ret;

    /** re initial  just remember the times*/
    if (UN_INIT_STATE != s_InitLayerCount)
    {
        /** */
        s_InitLayerCount++;
        return MT_SUCCESS;
    }
      
     /** 初始化图层适配层 */
    s32Ret = MTGO_ADP_CreateVideoDevice(&s_struGdevDevice);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    
    s_pGdevDevice = &s_struGdevDevice;   
    s32Ret = s_pGdevDevice->InitDisplay();
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(MTGO_ERR_INITFAILED);
        return s32Ret;
    }
    
    s_InitLayerCount++;
    return MT_SUCCESS;    
}

mt_s32 MTGO_DinitDisplay(mt_void)
{
    //mt_s32 s32Ret;
    
    /** 去初始化图层适配层 */
    s_pGdevDevice->DeinitDisplay();

    /** re initial  just remember the times*/
    if (UN_INIT_STATE != s_InitLayerCount)
    {
        /** */
        s_InitLayerCount--;
        return MT_SUCCESS;
    }

    return MT_SUCCESS;
}

mt_s32 MT_GO_GetLayerDefaultParam (MTGO_LAYER_E LayerID, MTGO_LAYER_INFO_S *pLayerInfo)
{
    mt_s32 s32Ret;    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pLayerInfo);

    if (LayerID >= MTGO_LAYER_BUTT)
    {
        MTGO_ERROR(MTGO_ERR_INVLAYERID);
        return MTGO_ERR_INVLAYERID;
    }
    s32Ret = s_pGdevDevice->GetDefautParam(LayerID, pLayerInfo);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    
    return MT_SUCCESS;
    
}

mt_s32 MT_GO_DestroyLayer(mt_handle Layer)
{
    MTGO_LAYER_REC_S* pLayerRec;

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    CHECK_NULLPTR(pLayerRec);
    pLayerRec->bCreated = MT_FALSE;

	if(pLayerRec->hLayerSurface != 0)
	{
		s_pGdevDevice->FreeCanvasSurface(pLayerRec->hLayerSurface);
		pLayerRec->hLayerSurface = 0;
	}
    /** destroy adp layer*/
    s_pGdevDevice->DestroyLayer(pLayerRec->LayerID);  


    //MTGO_DestroyLayerSuface();
    //MTGO_FreeDisplayBuffer();

   return MT_SUCCESS;
}

mt_s32 MT_GO_CreateLayer (const MTGO_LAYER_INFO_S *pLayerInfo, mt_handle* pLayer)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    mt_u32 LayerId;
    MTGO_LAYER_INFO_S stLayerInfo = {0};

    CHECK_GDEVINIT();

    /** check the param*/
    CHECK_NULLPTR(pLayerInfo);
    CHECK_NULLPTR(pLayer);

    MTGO_MemCopy(&stLayerInfo, pLayerInfo, sizeof(MTGO_LAYER_INFO_S));
    
#ifndef MTGO_LAYER_MAXCOUNT  
    s32Ret = MTGO_CheckParam(&stLayerInfo);
    if (s32Ret != MT_SUCCESS)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    } 
#endif   

    LayerId = pLayerInfo->LayerID;
    pLayerRec = s_LayerRec + LayerId;

    /** if create twice failed*/
    if (pLayerRec->bCreated == MT_TRUE)
    {
        *pLayer = pLayerRec->hLayer;
        return MT_SUCCESS;
    }
    MTGO_MemSet(pLayerRec, 0, sizeof(MTGO_LAYER_REC_S) );

    pLayerRec->LayerID =  LayerId;
    MTGO_MemCopy (&(pLayerRec->LayerInfo), &stLayerInfo, sizeof(MTGO_LAYER_INFO_S ));
    
    /** create the layer*/
    s32Ret = s_pGdevDevice->CreateLayer(LayerId, &pLayerRec->LayerInfo);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        goto err;
    }  
    
    *pLayer = (mt_handle)pLayerRec;
    pLayerRec->bCreated = MT_TRUE;
    pLayerRec->hLayer = *pLayer;
    return MT_SUCCESS;
    
 err:   
    s_pGdevDevice->DestroyLayer(LayerId);
    return MT_FAILURE;
}

mt_s32 MT_GO_GetLayerSurface(mt_handle Layer, mt_handle *pSurface)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;

    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pSurface);

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

	if(pLayerRec->hLayerSurface == 0)
	{
	    s32Ret = s_pGdevDevice->GetCanvasSurface(pLayerRec->LayerID, &pLayerRec->LayerInfo, pSurface);
	    if (MT_SUCCESS != s32Ret)
	    {
	        MTGO_ERROR(s32Ret);
	        return s32Ret;
	    }     

	    pLayerRec->hLayerSurface = *pSurface;
	}
	else
		*pSurface = pLayerRec->hLayerSurface;
    return MT_SUCCESS;
}

mt_s32 MT_GO_RefreshLayer(mt_handle Layer, const MT_RECT* pRect)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MT_RECT ActRect = {0};
    MT_RECT FullRect = {0};
   
    CHECK_GDEVINIT();

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s32Ret = MTGO_GetLayerSize(Layer, &FullRect.w, &FullRect.h);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);        
        return s32Ret;
    }
    
    if (pRect == NULL)
    {
        MTGO_MemCopy(&ActRect, &FullRect, sizeof(MT_RECT));
    }
    else
    {
        s32Ret = MTGO_GetRealRect(&FullRect, pRect, &ActRect);
        if (MT_SUCCESS != s32Ret)
        {
            MTGO_ERROR(s32Ret);
             return s32Ret;
        }
    }   

    /**  attention : the rect is no clipped*/
    s32Ret = s_pGdevDevice->SetLayerSurface(Layer, (MTGO_SURFACE_S*)pLayerRec->hLayerSurface, &ActRect);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
  
    return MT_SUCCESS;
}

mt_s32 MT_GO_RefreshLayerSync(mt_handle Layer, const MT_RECT* pRect)
{
    mt_s32 s32Ret = 0;
    TDE_HANDLE cmd_fifo_handle = MT_TDE2_CmdFifo_CreateBeginEx(MT_TRUE);        
    if (cmd_fifo_handle == 0)
    {
        printf("MT_TDE2_CmdFifo_CreateBeginEx Error cmd_fifo_handle\n");
        return MT_FAILURE;
    }

    s32Ret = MT_GO_RefreshLayer(Layer, pRect);    

    s32Ret |= MT_TDE2_CmdFifo_CreateEnd(cmd_fifo_handle);
    s32Ret |= MT_TDE2_CmdFifo_Run(cmd_fifo_handle);
    s32Ret |= MT_TDE2_WaitAllDone();
    s32Ret |= MT_TDE2_CmdFifo_Destroy(cmd_fifo_handle); 

    if (MT_SUCCESS != s32Ret)
       {
            MTGO_ERROR(s32Ret);
            return MT_FAILURE;
       }    
    return  MT_SUCCESS;
  
}

mt_s32 MT_GO_RefreshLayer_Zoom(mt_handle Layer, const MT_RECT* pZoomRect)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MT_RECT ActRect = {0};
    MT_RECT FullRect = {0};
   
    CHECK_GDEVINIT();

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s32Ret = MTGO_GetLayerSize(Layer, &FullRect.w, &FullRect.h);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);        
        return s32Ret;
    }
    
    /**  attention : the rect is no clipped*/
    s32Ret = s_pGdevDevice->SetLayerSurface_Zoom(Layer, (MTGO_SURFACE_S*)pLayerRec->hLayerSurface, &FullRect, pZoomRect);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
  
    return MT_SUCCESS;
}

mt_s32 MT_GO_RefreshLayerSync_Zoom(mt_handle Layer, const MT_RECT* pZoomRect)
{
    mt_s32 s32Ret = 0;
    TDE_HANDLE cmd_fifo_handle = MT_TDE2_CmdFifo_CreateBeginEx(MT_TRUE);        
    if (cmd_fifo_handle == 0)
    {
        printf("MT_TDE2_CmdFifo_CreateBeginEx Error cmd_fifo_handle\n");
              return MT_FAILURE;
    }
    printf("MT_TDE2_CmdFifo_CreateBeginEx\n");

    s32Ret = MT_GO_RefreshLayer_Zoom(Layer, pZoomRect);    

    s32Ret |= MT_TDE2_CmdFifo_CreateEnd(cmd_fifo_handle);
    s32Ret |= MT_TDE2_CmdFifo_Run(cmd_fifo_handle);
    s32Ret |= MT_TDE2_WaitAllDone();
    s32Ret |= MT_TDE2_CmdFifo_Destroy(cmd_fifo_handle); 

    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return MT_FAILURE;
    }    
    return  MT_SUCCESS;
  
}

mt_s32 MT_GO_SetLayerPos(mt_handle Layer, mt_u32 u32StartX, mt_u32 u32StartY)
{
#ifndef MTGO_LAYER_MAXCOUNT
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    
    CHECK_GDEVINIT();

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
        /** 不可以为负数*/
    if((((mt_s32)u32StartX) < 0) || (((mt_s32)u32StartY) < 0))
    {   
        return MTGO_ERR_INVLAYERPOS;
    }

    /** 设置屏幕位置 */
    s32Ret = s_pGdevDevice->SetPos(pLayerRec->LayerID, u32StartX, u32StartY);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 MT_GO_GetLayerPos(mt_handle Layer, mt_u32 *pStartX, mt_u32 *pStartY)
{
#ifndef MTGO_LAYER_MAXCOUNT
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;

    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pStartX);
    CHECK_GDEV_NULLPTR(pStartY);

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    /** 获取屏幕位置 */
    s32Ret = s_pGdevDevice->GetPos(pLayerRec->LayerID, pStartX, pStartY);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 MT_GO_SetDisplaySize(mt_handle Layer, mt_u32 u32DWidth, mt_u32 u32DHeight)
{
#ifndef MTGO_LAYER_MAXCOUNT
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_LAYER_CAP_S * pLayerCap;
    CHECK_GDEVINIT();

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    if ((u32DWidth == 0) || (u32DHeight == 0))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }
    
    /** 如果不支持后端缩放就直接返回失败*/
    s_pGdevDevice->CapabilityInquire(pLayerRec->LayerID, &pLayerCap);
    if ((u32DWidth > (mt_u32)pLayerCap->MaxDisplayWidth) || (u32DHeight > (mt_u32)pLayerCap->MaxDisplayHeight))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }

    if ((u32DWidth  >(mt_u32)pLayerCap->MaxWidth) || (u32DWidth < (mt_u32)pLayerCap->MinWidth)||
        (u32DHeight > (mt_u32)pLayerCap->MaxHeight) || (u32DHeight <(mt_u32)pLayerCap->MinHeight))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }
    
    s32Ret = s_pGdevDevice->SetDisplaySize(pLayerRec->LayerID, u32DWidth, u32DHeight);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

    pLayerRec->LayerInfo.DisplayWidth = u32DWidth;
    pLayerRec->LayerInfo.DisplayHeight = u32DHeight;    
#endif
    return MT_SUCCESS;
    
}

mt_s32 MT_GO_GetDisplaySize(mt_handle Layer, mt_u32 *pDWidth, mt_u32 *pDHeight)
{
#ifndef MTGO_LAYER_MAXCOUNT
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;

    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pDWidth);
    CHECK_GDEV_NULLPTR(pDHeight);    
    
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s32Ret = s_pGdevDevice->GetDisplaySize(pLayerRec->LayerID, pDWidth, pDHeight);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
#endif    
    return MT_SUCCESS;
}

mt_s32 MT_GO_SetScreenSize(mt_handle Layer, mt_u32 u32SWidth, mt_u32 u32SHeight)
{
#ifndef MTGO_LAYER_MAXCOUNT
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_LAYER_CAP_S* pstruLayerCap = MT_NULL_PTR;

    CHECK_GDEVINIT();
    
    if ((u32SWidth == 0)||(u32SHeight == 0))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s_pGdevDevice->CapabilityInquire(pLayerRec->LayerID, &pstruLayerCap);
    if ((u32SWidth  >(mt_u32) pstruLayerCap->MaxWidth) || (u32SWidth < (mt_u32)pstruLayerCap->MinWidth)||
        (u32SHeight > (mt_u32)pstruLayerCap->MaxHeight) || (u32SHeight < (mt_u32)pstruLayerCap->MinHeight))
    {
        MTGO_ERROR(MTGO_ERR_INVSIZE);
        return MTGO_ERR_INVSIZE;
    }

    pLayerRec->LayerInfo.ScreenWidth = u32SWidth;
    pLayerRec->LayerInfo.ScreenHeight = u32SHeight;
    
    s32Ret = s_pGdevDevice->SetScreenSize(pLayerRec->LayerID, u32SWidth, u32SHeight);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 MT_GO_GetScreenSize(mt_handle Layer, mt_u32 *pSWidth, mt_u32 *pSHeight)
{
#ifndef MTGO_LAYER_MAXCOUNT
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pSWidth);
    CHECK_GDEV_NULLPTR(pSHeight);  

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s32Ret = s_pGdevDevice->GetScreenSize(pLayerRec->LayerID, pSWidth, pSHeight);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
 #endif
    return MT_SUCCESS;
}

mt_s32 MT_GO_SetLayerAlpha(mt_handle Layer, mt_u8 Alpha)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_LAYER_ALPHA_S LayerALpha;
    
    CHECK_GDEVINIT();

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
   
   	MTGO_MemSet(&LayerALpha, 0, sizeof(MTGO_LAYER_ALPHA_S));
	s32Ret = s_pGdevDevice->GetLayerAlpha(pLayerRec->LayerID, &LayerALpha);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

	LayerALpha.bAlphaEnable = MT_TRUE;
	LayerALpha.bAlphaChannel = MT_TRUE;
	LayerALpha.GlobalAlpha  =  Alpha;

    /** 在直接设置硬件ALPHA */    
    s32Ret = s_pGdevDevice->SetLayerAlpha(pLayerRec->LayerID, &LayerALpha);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
 
    return MT_SUCCESS;

}

mt_s32 MT_GO_GetLayerAlpha(mt_handle Layer, mt_u8* pAlpha)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_LAYER_ALPHA_S LayerALpha;    

    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pAlpha);

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    s32Ret = s_pGdevDevice->GetLayerAlpha(pLayerRec->LayerID, &LayerALpha);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    
    *pAlpha = LayerALpha.GlobalAlpha;
    return MT_SUCCESS;
}

mt_s32 MT_GO_SetRegionAlpha(mt_handle Layer, MT_BOOL Enable, mt_u8 Alpha)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_LAYER_ALPHA_S LayerALpha;
    
    CHECK_GDEVINIT();

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
   
   	MTGO_MemSet(&LayerALpha, 0, sizeof(MTGO_LAYER_ALPHA_S));
	s32Ret = s_pGdevDevice->GetLayerAlpha(pLayerRec->LayerID, &LayerALpha);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

	LayerALpha.bRegionAlphaEnable = Enable;
	LayerALpha.u8RegionAlpha = Alpha;

    /** 在直接设置硬件ALPHA */    
    s32Ret = s_pGdevDevice->SetLayerAlpha(pLayerRec->LayerID, &LayerALpha);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }    
 
    return MT_SUCCESS;

}

mt_s32 MT_GO_GetRegionAlpha(mt_handle Layer, mt_u8* pAlpha)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_LAYER_ALPHA_S LayerALpha;    

    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pAlpha);

    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    s32Ret = s_pGdevDevice->GetLayerAlpha(pLayerRec->LayerID, &LayerALpha);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    
    *pAlpha = LayerALpha.u8RegionAlpha;
    return MT_SUCCESS;
}

mt_s32 MT_GO_SetLayerColorkey(mt_handle Layer, const MTGO_LAYER_KEY_S *pKey)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pKey);
        
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s32Ret = s_pGdevDevice->SetLayerColorKey(pLayerRec->LayerID, pKey->bEnableCK, pKey->ColorKey);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    return MT_SUCCESS;
}

mt_s32 MT_GO_GetLayerColorkey(mt_handle Layer, MTGO_LAYER_KEY_S *pKey)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pKey);
        
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    
    s32Ret = s_pGdevDevice->GetLayerColorKey(pLayerRec->LayerID, &(pKey->bEnableCK), &(pKey->ColorKey));
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    return MT_SUCCESS;
}

#if 0
mt_s32 MT_GO_SetLayerBGColor(mt_handle Layer, MT_COLOR Color)
{
    return MT_SUCCESS;
}


mt_s32 MT_GO_SetLayerAlphaEx(mt_handle Layer,  MTGO_LAYER_ALPHA_S *pAlphaInfo)
{
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    GRC_ALPHA_S pAlpha;
    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pAlphaInfo);
    
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    DisplayInfo = s_DisplayInfo + pLayerRec->LayerID;

    memcpy(&DisplayInfo->stAlpha, pAlphaInfo, sizeof(MTGO_LAYER_ALPHA_S));    

    pAlpha.bAlphaChannel = DisplayInfo->stAlpha.bAlphaChannel;
    pAlpha.bAlphaEnable = DisplayInfo->stAlpha.bAlphaEnable;
    pAlpha.u8Alpha0 = DisplayInfo->stAlpha.Alpha0;
    pAlpha.u8Alpha1 = DisplayInfo->stAlpha.Alpha1;
    pAlpha.u8GlobalAlpha = DisplayInfo->stAlpha.GlobalAlpha; 
    //update Alpha register
    Grc_DRV_SetLayerAlpha(pLayerRec->LayerID, &pAlpha);   
    Grc_DRV_UpdataLayerReg(pLayerRec->LayerID);    
    return MT_SUCCESS;	
}


mt_s32 MT_GO_GetLayerAlphaEx(mt_handle Layer,  MTGO_LAYER_ALPHA_S *pAlphaInfo)
{
    MTGO_LAYER_REC_S* pLayerRec;
    MTGO_DISPLAY_INFO_S *DisplayInfo;
    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pAlphaInfo);
    
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;
    DisplayInfo = s_DisplayInfo + pLayerRec->LayerID;

    memcpy(pAlphaInfo, &DisplayInfo->stAlpha, sizeof(MTGO_LAYER_ALPHA_S));
        
    return MT_SUCCESS;	
} 
#endif


mt_s32 MT_GO_ShowLayer(mt_handle Layer, MT_BOOL bVisbile)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;

    CHECK_GDEVINIT();
    
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    /** 显示或隐藏图层 */
    s32Ret = s_pGdevDevice->ShowLayer(pLayerRec->LayerID, bVisbile);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }

    return MT_SUCCESS;	
} 

mt_s32 MT_GO_SetLayerPalette(mt_handle Layer, mt_u32 *pPalette, mt_u32 u32Start, mt_u32 u32Len)
{
    mt_s32 s32Ret;
    MTGO_LAYER_REC_S* pLayerRec;
    
    CHECK_GDEVINIT();
    CHECK_GDEV_NULLPTR(pPalette);
        
    pLayerRec = (MTGO_LAYER_REC_S*)Layer;

    s32Ret = s_pGdevDevice->SetLayerPalette(pLayerRec->LayerID, pPalette, u32Start, u32Len);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_ERROR(s32Ret);
        return s32Ret;
    }
    return MT_SUCCESS;

}
