/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mt_unf_avplay.h"
#include "mt_debug.h"
#include "mt_go.h"
#include "sample_teletext_out.h"
#include "sample_teletext_msg.h"

#ifdef  MT_SAMPLE_TTX_DEBUG

#define MT_TTX_PRINT   printf
#else

#define MT_TTX_PRINT

#endif

#define SAMPLE_TTX_FUNCTION_ENTER() MT_TTX_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_TTX_FUNCTION_EXIT()      MT_TTX_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_TTX_FATAL_PRINT(fmt...)          MT_TTX_PRINT(" [FATAL] " fmt)
#define SAMPLE_TTX_ERR_PRINT(fmt...)            MT_TTX_PRINT(" [ERROR] " fmt)
#define SAMPLE_TTX_WARN_PRINT(fmt...)           MT_TTX_PRINT(" [WARN] "  fmt)
#define SAMPLE_TTX_INFO_PRINT(fmt...)           MT_TTX_PRINT(" [INFO] "  fmt)
#define SAMPLE_TTX_DBG_PRINT(fmt...)            MT_TTX_PRINT(" [DEBUG] " fmt)





static MT_HANDLE g_hLayer = 0;
static MT_HANDLE g_hLayerSurface = 0;
static MT_HANDLE g_hMemSurface = 0;
static MT_HANDLE g_hCharSurface = 0;



static mt_void   TTX_SendGUIMsg(MT_HANDLE  * phDispalyHandle);
static mt_s32    TTX_Create_CharBuffer(MT_UNF_TTX_CHAR_BUFFER_PARAM_S * pstBufferParam);
static mt_s32    TTX_Destroy_CharBuffer(mt_void);
static mt_s32   TTX_DrawOsd(MT_UNF_TTX_PAGEAREA_S* pstParam);
static mt_s32   TTX_ShowOsd(MT_UNF_TTX_REFRESHLAYER_S* pstParam);



/*
@brief Callback  function, register to ttx module. draw char ,refresh osd ,fill rectangle  ,get pts,  get localtime
@param[in] hTTX, ttx data handle
@param[in] enCB, The set of callback cmd
@param[in] pvCBParam, Teletext Buffer info
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_s32 TTX_SampleCallBack(MT_HANDLE hTTX, MT_UNF_TTX_CB_E enCB, mt_void *pvCBParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_UNF_TTX_FILLRECT_S *pstFillrectparam = { 0 };
    MT_UNF_TTX_REFRESHLAYER_S *pstRefreshparam = { 0 };
    MT_HANDLE hDispalyHandle = 0;
    MT_RECT rect = { 0 };
    MT_PALETTE Palette = { 0 };

    if ((MT_NULL == pvCBParam) || (enCB > MT_UNF_TTX_CB_BUTT))
    {
        return MT_FAILURE;
    }

    switch (enCB)
    {
        case MT_UNF_TTX_CB_APP_FILLRECT:
        {
            pstFillrectparam = (MT_UNF_TTX_FILLRECT_S *)pvCBParam;

            rect.x = pstFillrectparam->pstPageArea->u32Column;
            rect.y = pstFillrectparam->pstPageArea->u32Row;
            rect.w = pstFillrectparam->pstPageArea->u32ColumnCount;
            rect.h = pstFillrectparam->pstPageArea->u32RowCount;

            s32Ret = MT_GO_FillRect(g_hLayerSurface, &rect, pstFillrectparam->u32Color, MTGO_COMPOPT_NONE);

        }
        break;

        case MT_UNF_TTX_CB_APP_SETPALETTE:
        {
            memcpy(Palette, (unsigned char *)pvCBParam, 256 * 4);
            s32Ret = MT_GO_SetLayerPalette(g_hLayer, Palette, 0, 256);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_TTX_ERR_PRINT("MT_GO_SetLayerPalette is failed\n");
            }
        }
        break;

        case MT_UNF_TTX_CB_APP_DRAWOSD:
        {
            (void)TTX_DrawOsd((MT_UNF_TTX_PAGEAREA_S *)pvCBParam);
        }
        break;

        case MT_UNF_TTX_CB_APP_REFRESH:
        {
            pstRefreshparam = (MT_UNF_TTX_REFRESHLAYER_S *)pvCBParam;
            TTX_ShowOsd(pstRefreshparam);
            s32Ret = MT_SUCCESS;
        }
        break;

        case MT_UNF_TTX_CB_TTX_TO_APP_MSG:
        {
            hDispalyHandle = *((MT_HANDLE  *)pvCBParam);
            (void)TTX_SendGUIMsg(&hDispalyHandle);

        }
        break;
        case MT_UNF_TTX_CB_CREATE_CHAR_BUFF:
        {
            s32Ret = TTX_Create_CharBuffer((MT_UNF_TTX_CHAR_BUFFER_PARAM_S *)pvCBParam);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_TTX_ERR_PRINT("TTX_Create_CharBuffer is failed\n");
            }
        }
        break;

        case MT_UNF_TTX_CB_DESTROY_CHARBUFF:
        {
            s32Ret = TTX_Destroy_CharBuffer();
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_TTX_ERR_PRINT("TTX_Destroy_CharBuffer is failed\n");
            }
        }
        break;

    default:
        break;
    }

    return s32Ret;
}


/*
@brief Init mtgo , used to show teletext
@param void
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_s32  Mtgo_Teletext_Init()
{
    mt_s32 s32Ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;

    /**initial resource*/
    s32Ret = MT_GO_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_Init is failed\n");
        goto ERR1;
    }

    s32Ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_GetLayerDefaultParam is failed\n");
        goto ERR1;
    }

    stLayerInfo.PixelFormat   = MTGO_PF_CLUT8;
    stLayerInfo.CanvasWidth = 720;
    stLayerInfo.CanvasHeight = 550;
    stLayerInfo.DisplayWidth = 720;
    stLayerInfo.DisplayHeight = 550;

    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
    /**create the graphic layer and get the handler */
    s32Ret = MT_GO_CreateLayer(&stLayerInfo, &g_hLayer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_CreateLayer failed !\n");
        goto ERR1;
    }

    s32Ret = MT_GO_GetLayerSurface(g_hLayer, &g_hLayerSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_GetLayerSurface failed !\n");
        goto ERR2;
    }

    s32Ret = MT_GO_CreateSurface(720, 480, MTGO_PF_CLUT8, &g_hMemSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_CreateSurface failed !\n");
        goto ERR2;
    }


    return s32Ret;


ERR2:
    s32Ret = MT_GO_DestroyLayer(g_hLayer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_DestroyLayer failed !\n");
    }
ERR1:
    s32Ret = MT_GO_Deinit();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_DestroyLayer failed !\n");
    }

    return s32Ret;
}


/*
@brief Deinit mtgo
@param void
@retuen MT_SUCCESS
@return MT_FALSE
*/
mt_void   Mtgo_Teletext_DeInit()
{
    mt_s32 s32Ret = MT_SUCCESS;

    if (g_hLayer)
    {
        s32Ret = MT_GO_DestroyLayer(g_hLayer);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_TTX_ERR_PRINT("MT_GO_DestroyLayer failed !\n");
        }
    }

    s32Ret = MT_GO_FreeSurface(g_hMemSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_FreeSurface failed !\n");
    }

    s32Ret = MT_GO_Deinit();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("call MT_GO_Deinit failed !\n");
    }

    return;
}



/*
@brief Refresh  osd  layer
@param[in] pstParam, Teletext Buffer info
@retuen MT_SUCCESS
@return MT_FALSE
*/
static mt_s32 TTX_DrawOsd(MT_UNF_TTX_PAGEAREA_S *pstParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_RECT stDesRect = { 0 };
    MT_RECT stSrcRect = { 0 };
    MT_PIXELDATA stPixData = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };

    memset(stPixData, 0, sizeof(stPixData));
    memset(&stBltOpt, 0, sizeof(stBltOpt));
    stBltOpt.EnableScale = MT_TRUE;

    if (MT_NULL == pstParam)
    {
        SAMPLE_TTX_ERR_PRINT("In RefreshLayer, Invalid param!\n");

        return MT_FAILURE;
    }

    stDesRect.x = pstParam->u32Column;
    stDesRect.y = pstParam->u32Row;
    stDesRect.w = pstParam->u32ColumnCount;
    stDesRect.h = pstParam->u32RowCount;


    stSrcRect.w = stDesRect.w;
    stSrcRect.h = stDesRect.h;

    s32Ret = MT_GO_Blit(g_hCharSurface, &stSrcRect, g_hLayerSurface, &stDesRect, &stBltOpt);
    if (MT_SUCCESS != s32Ret)
    {
        return s32Ret;
    }

    s32Ret = MT_GO_LockSurface(g_hLayerSurface, stPixData, MT_TRUE);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_LockSurface failed !\n");
        return s32Ret;
    }
    s32Ret = MT_GO_RefreshLayer(g_hLayer, &stDesRect);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_RefreshLayer failed !\n");
        return s32Ret;
    }
    s32Ret = MT_GO_UnlockSurface(g_hLayerSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_UnlockSurface failed !\n");
        return s32Ret;
    }
    return MT_SUCCESS;
}

static mt_s32   TTX_ShowOsd(MT_UNF_TTX_REFRESHLAYER_S* pstParam)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MT_RECT stDesRect = {0};

        if(pstParam != NULL)
        {
            stDesRect.x = pstParam->pstPageArea->u32Column;
            stDesRect.y = pstParam->pstPageArea->u32Row;
            stDesRect.w = pstParam->pstPageArea->u32ColumnCount;
            stDesRect.h = pstParam->pstPageArea->u32RowCount;
        s32Ret = MT_GO_RefreshLayer(g_hLayer, &stDesRect);
        }
        else
            s32Ret = MT_GO_RefreshLayer(g_hLayer, NULL);

    return s32Ret;
}


/*
@brief Send interface message
@param[in] phDispalyHandle, display handle
@retuen MT_SUCCESS
@return MT_FALSE
*/
static mt_void TTX_SendGUIMsg(MT_HANDLE  *phDispalyHandle)
{
    MT_HANDLE hDispalyHandle = 0;
    MESSAGE_S stMsg = { 0 };

    if (phDispalyHandle == MT_NULL)
    {
        SAMPLE_TTX_ERR_PRINT("phDispalyHandle failed !\n");
        return;
    }

    hDispalyHandle = (*phDispalyHandle);

    stMsg.pu8MsgData   = (mt_u8 *) &hDispalyHandle;
    stMsg.u16MsgLength = sizeof(hDispalyHandle);
    if (MT_SUCCESS != MsgQueue_En(&stMsg))
    {
        SAMPLE_TTX_ERR_PRINT("call MsgQueue_En failed !\n");
        return;
    }
}


/*
@brief Create a buffer for teletext cache information
@param[in] pstBufferParam, Teletext Buffer info
@retuen MT_SUCCESS
@return MT_FALSE
*/
static mt_s32 TTX_Create_CharBuffer(MT_UNF_TTX_CHAR_BUFFER_PARAM_S* pstBufferParam)
{

    mt_s32 s32Ret = 0;
    mt_u8 *pu8CharAddr = MT_NULL;
    MT_PIXELDATA stPixData = { 0 };

    if (MT_NULL == pstBufferParam)
    {
        SAMPLE_TTX_ERR_PRINT("pstBufferParam failed !\n");
        return MT_FAILURE;
    }

    s32Ret = MT_GO_CreateSurface(pstBufferParam->width, pstBufferParam->height, MTGO_PF_CLUT8, &g_hCharSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_CreateSurface failed ! w = %d, h = %d\n", pstBufferParam->width, pstBufferParam->height);
        goto BEBACK;
    }

    s32Ret = MT_GO_LockSurface(g_hCharSurface, stPixData, MT_TRUE);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_TTX_ERR_PRINT("MT_GO_LockSurface failed !\n");
        (void)MT_GO_FreeSurface(g_hCharSurface);
        goto BEBACK;
    }

    pu8CharAddr = (mt_u8 *)stPixData[0].pData;
    s32Ret = MT_GO_UnlockSurface(g_hCharSurface);
    if (MT_SUCCESS != s32Ret)
    {
        (void)MT_GO_FreeSurface(g_hCharSurface);
        SAMPLE_TTX_ERR_PRINT("MT_GO_UnlockSurface failed !\n");
        goto BEBACK;
    }
    pstBufferParam->pBuf = pu8CharAddr;
    pstBufferParam->pitch = stPixData[0].Pitch;

    return MT_SUCCESS;

BEBACK:
    SAMPLE_TTX_ERR_PRINT("TTX_Create_Buffer failed !\n");
    return MT_FAILURE;
}


/*
@brief Destroy a buffer for teletext cache information
@param void
@retuen MT_SUCCESS
@return MT_FALSE
*/
static mt_s32 TTX_Destroy_CharBuffer()
{
    mt_s32 s32Ret = 0;
    if (0 != g_hCharSurface)
    {
        s32Ret = MT_GO_FreeSurface(g_hCharSurface);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_TTX_ERR_PRINT("TTX_Destroy_CharBuffer failed !\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

