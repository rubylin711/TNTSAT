/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2020, Montage Technology Co., Ltd.
 *
 * File Name      : drv_disp_fw_bridge.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2020/11/11
 * Description    : Display(VO) Firmware Bridge
 * History        :
 * 1.Date         : 2020/11/11
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/printk.h>

#include "mt_module_debug.h"
#include "mt_drv_module.h"

#include "vo_fw.h"

static VO_FW_EXPORT_FUNC_S *g_vofw_func = NULL;
#define GET_VOFW_FUNC()	do { } while(0)

#define CHECK_VOFW_FUNC()			do { \
    if (g_vofw_func == NULL) { \
        MT_ERR_VO("VOFW function is null!!!\n"); \
        return MT_FAILURE; \
    } \
} while(0)

#define CHECK_VOFW_FUNC_VOID()		do { \
    if (g_vofw_func == NULL) { \
        MT_ERR_VO("VOFW function is null!!!\n"); \
        return; \
    } \
} while(0)

#define CHECK_NULL_PARAM(pa)		do { \
    if (pa == NULL) { \
        MT_ERR_VO("parameter is null!\n"); \
        return MT_FAILURE; \
    } \
} while(0)


static unsigned long fw_disp_isr_counter = 0;
extern int debug_disp_isr;

//---------------------------------------------------------------------------//

mt_s32 DF_Create(mt_handle* pstDispPool, mt_u32 u32BufNumber, MT_BOOL bCloseHdDI)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pstDispPool);

    if (g_vofw_func->pfnDF_Create == NULL)
    {
        MT_ERR_VO("pfnDF_Create is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_Create(pstDispPool, u32BufNumber, bCloseHdDI);
}

mt_s32 DF_Destroy(mt_handle pstDispPool)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_Destroy == NULL)
    {
        MT_ERR_VO("pfnDF_Destory is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_Destroy(pstDispPool);
}

mt_s32 DF_TransformDec2Disp_sinfo(mt_handle pstDispPool, MT_DRV_VIDEO_FRAME_S *pFrameInfo, MT_DF_VIDEO_FRAME_S *pstVideoFrame)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pFrameInfo);
    CHECK_NULL_PARAM(pstVideoFrame);

    if (g_vofw_func->pfnDF_TransformDec2Disp_sinfo == NULL)
    {
        MT_ERR_VO("pfnDF_TransformDec2Disp_sinfo is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_TransformDec2Disp_sinfo(pstDispPool, pFrameInfo, pstVideoFrame);
}
/*
mt_s32 DF_TransformDec2Disp_sinfo1(IMAGE *pstImage, MT_DF_VIDEO_FRAME_S *pstVideoFrame)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pstImage);
    CHECK_NULL_PARAM(pstVideoFrame);

    if (g_vofw_func->pfnDF_TransformDec2Disp_sinfo1 == NULL)
    {
        MT_ERR_VO("pfnDF_TransformDec2Disp_sinfo1 is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_TransformDec2Disp_sinfo1(pstImage, pstVideoFrame);
}
*/
mt_s32 DF_TSK_enQueue(mt_handle pstDispPool, MT_DF_VIDEO_FRAME_S *pstVideoFrame, MT_BOOL *pbNeedNewData)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pstVideoFrame);
    CHECK_NULL_PARAM(pbNeedNewData);

    if (g_vofw_func->pfnDF_TSK_enQueue == NULL)
    {
        MT_ERR_VO("pfnDF_TSK_enQueue is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_TSK_enQueue(pstDispPool, pstVideoFrame, pbNeedNewData);
}

mt_void DF_ISR_Update(mt_handle pstDispPool, DF_DRV_SETTING_S *pstDrvSetting)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC_VOID();

    if (pstDrvSetting == NULL)
    {
        MT_ERR_VO("invalide parameter!\n");
        return;
    }

    if (g_vofw_func->pfnDF_ISR_Update == NULL)
    {
        MT_ERR_VO("pfnDF_ISR_Update is null!\n");
        return;
    }

    //debug
    fw_disp_isr_counter ++;
    if (debug_disp_isr)
    {
        if ((fw_disp_isr_counter % 1000) == 0)
        {
            printk(KERN_INFO "\r\n DF_ISR_Update: %lu\r\n", fw_disp_isr_counter);
        }
    }

    g_vofw_func->pfnDF_ISR_Update(pstDispPool, pstDrvSetting);

    return;
}

MT_DF_STATUS DF_GetStatus(mt_handle pstDispPool)
{
    GET_VOFW_FUNC();
    //CHECK_VOFW_FUNC();

    if (g_vofw_func == NULL || g_vofw_func->pfnDF_GetStatus == NULL)
    {
        MT_ERR_VO("pfnDF_GetStatus is null!\n");
        return DF_STATUS_BOT;
    }

    return g_vofw_func->pfnDF_GetStatus(pstDispPool);
}

mt_s32 DF_SetCmd(mt_handle pstDispPool, MT_DF_CMD eCMD, MT_VOID* pArgs)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_SetCmd == NULL)
    {
        MT_ERR_VO("pfnDF_SetCmd is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_SetCmd(pstDispPool, eCMD, pArgs);
}

mt_void DF_FlushDisp(mt_handle pstDispPool, MT_DF_FLUSH_TYPE_E eType)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC_VOID();

    if (g_vofw_func->pfnDF_FlushDisp == NULL)
    {
        MT_ERR_VO("pfnDF_FlushDisp is null!\n");
        return;
    }

    g_vofw_func->pfnDF_FlushDisp(pstDispPool, eType);

    return;
}

#if 0
mt_s32 DF_FreezeMalloc(mt_handle pstDispPool)
{
	GET_VOFW_FUNC();
	CHECK_VOFW_FUNC();

	if (g_vofw_func->pfnDF_FreezeMalloc == NULL)
	{
		MT_ERR_VO("pfnDF_FreezeMalloc is null!\n");
		return MT_FAILURE;
	}

	return g_vofw_func->pfnDF_FreezeMalloc(pstDispPool);
}

mt_void DF_FreezeRelease(mt_handle pstDispPool)
{
	GET_VOFW_FUNC();
	CHECK_VOFW_FUNC_VOID();

	if (g_vofw_func->pfnDF_FreezeRelease == NULL)
	{
		MT_ERR_VO("pfnDF_FreezeRelease is null!\n");
		return;
	}

	g_vofw_func->pfnDF_FreezeRelease(pstDispPool);

	return;
}
#else
MT_VOID DF_SetFreezeBuffer(mt_handle pstDispPool, phys_addr_t u32PhyAddr, MT_U32 u32Size)
{
	GET_VOFW_FUNC();
	CHECK_VOFW_FUNC_VOID();

	if (g_vofw_func->pfnDF_SetFreezeBuffer == NULL)
	{
		MT_ERR_VO("pfnDF_SetFreezeBuffer is null!\n");
		return;
	}

	g_vofw_func->pfnDF_SetFreezeBuffer(pstDispPool, u32PhyAddr, u32Size);

	return;
}
#endif
mt_u32 DF_TestFreezeDone(mt_handle pstDispPool)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_TestFreezeDone == NULL)
    {
        MT_ERR_VO("pfnDF_TestFreezeDone is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_TestFreezeDone(pstDispPool);
}

mt_void DF_Wait_For_FreezeCopy_Done(mt_handle pstDispPool)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC_VOID();

    if (g_vofw_func->pfnDF_Wait_For_FreezeCopy_Done == NULL)
    {
        MT_ERR_VO("pfnDF_Wait_For_FreezeCopy_Done is null!\n");
        return;
    }

    g_vofw_func->pfnDF_Wait_For_FreezeCopy_Done(pstDispPool);

    return;
}

mt_u32 DF_GetVideoCoeffTable(VIDEO_SCALE_COEFF_TABLE_E table)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_GetVideoCoeffTable == NULL)
    {
        MT_ERR_VO("pfnDF_GetVideoCoeffTable is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_GetVideoCoeffTable(table);
}

mt_s32 DF_SetSource(mt_handle pstDispPool, DISP_SOURCE_INFO_S *pstSrc)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pstSrc);

    if (g_vofw_func->pfnDF_SetSource == NULL)
    {
        MT_ERR_VO("pfnDF_SetSource is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_SetSource(pstDispPool, pstSrc);
}

///////////////////////////////////////////////////////////////////////////////

mt_s32 DF_AvsyncGetVptsInfo(mt_handle pstDispPool, DF_AVSYNC_PTS_S *pInfo)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pInfo);

    if (g_vofw_func->pfnDF_AvsyncGetVptsInfo == NULL)
    {
        MT_ERR_VO("pfnDF_AvsyncGetVptsInfo is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_AvsyncGetVptsInfo(pstDispPool, pInfo);
}

mt_s32 DF_AvsyncGetInputRate(mt_handle pstDispPool, mt_u32 *pInputRate)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pInputRate);

    if (g_vofw_func->pfnDF_AvsyncGetInputRate == NULL)
    {
        MT_ERR_VO("pfnDF_AvsyncGetInputRate is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_AvsyncGetInputRate(pstDispPool, pInputRate);
}

mt_s32 DF_AvsyncSetSkipFrame(mt_handle pstDispPool, mt_u32 skip_num)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_AvsyncSetSkipFrame == NULL)
    {
        MT_ERR_VO("pfnDF_AvsyncSetSkipFrame is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_AvsyncSetSkipFrame(pstDispPool, skip_num);
}

mt_s32 DF_AvsyncSetRepeatFrame(mt_handle pstDispPool)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_AvsyncSetRepeatFrame == NULL)
    {
        MT_ERR_VO("pfnDF_AvsyncSetRepeatFrame is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_AvsyncSetRepeatFrame(pstDispPool);
}

mt_s32 DF_AvsyncSetPause(mt_handle pstDispPool, MT_BOOL is_play)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_AvsyncSetPause == NULL)
    {
        MT_ERR_VO("pfnDF_AvsyncSetPause is null!\n");
        return MT_FAILURE;
    }

    return g_vofw_func->pfnDF_AvsyncSetPause(pstDispPool, is_play);
}

mt_s32 DF_CheckFifoEmpty(mt_handle pstDispPool, MT_BOOL *bEmpty)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_CheckFifoEmpty == NULL)
    {
        MT_ERR_VO("pfnDF_CheckFifoEmpty is null!\n");
        return MT_FAILURE;
    }

    *bEmpty = g_vofw_func->pfnDF_CheckFifoEmpty(pstDispPool);

    return MT_SUCCESS;
}

mt_s32 DF_GetCurrVideoInfo(mt_handle pstDispPool, MT_DF_VIDEO_INFO* vid_info)
{
	GET_VOFW_FUNC();
	CHECK_VOFW_FUNC();

	if (g_vofw_func->pfnDF_GetCurrVideoInfo == NULL)
	{
		MT_ERR_VO("pfnDF_GetCurrVideoInfo is null!\n");
		return MT_FAILURE;
	}

	return g_vofw_func->pfnDF_GetCurrVideoInfo(pstDispPool, vid_info);
}

mt_s32 DF_UpdateOsdCsc(tv_mode_t curr_tv_mode)
{
	GET_VOFW_FUNC();
	CHECK_VOFW_FUNC();

	if (g_vofw_func->pfnDF_UpdateOsdCsc == NULL)
	{
		MT_ERR_VO("DF_UpdateOsdCsc is null!\n");
		return MT_FAILURE;
	}

	g_vofw_func->pfnDF_UpdateOsdCsc(curr_tv_mode);

    return MT_SUCCESS;
}

mt_s32 DF_UpdateSdCsc(tv_mode_t curr_tv_mode)
{
	GET_VOFW_FUNC();
	CHECK_VOFW_FUNC();

	if (g_vofw_func->pfnDF_UpdateSdCsc == NULL)
	{
		MT_ERR_VO("DF_UpdateSdCsc is null!\n");
		return MT_FAILURE;
	}

	g_vofw_func->pfnDF_UpdateSdCsc(curr_tv_mode);

    return MT_SUCCESS;
}

mt_s32 DF_GraScaler_Update(DF_DRV_SETTING_S *pDrvSetting)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_GraScaler_Update== NULL)
    {
        MT_ERR_VO("pfnDF_GraScaler_Update is null!\n");
        return MT_FAILURE;
    }

    g_vofw_func->pfnDF_GraScaler_Update(pDrvSetting);

    return MT_SUCCESS;
}

mt_s32 DF_OsdScaler_Update(MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_OsdScaler_Update== NULL)
    {
        MT_ERR_VO("pfnDF_Osd_Scale is null!\n");
        return MT_FAILURE;
    }

    g_vofw_func->pfnDF_OsdScaler_Update(src_width, src_height, dst_width, dst_height);

    return MT_SUCCESS;
}

mt_s32 DF_StillScaler_Update(mt_handle pstDispPool,MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height, MT_BOOL bProgressive)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();
    if (g_vofw_func->pfnDF_StillScaler_Update== NULL)
    {
        MT_ERR_VO("pfnDF_StillScaler_Update is null!\n");
        return MT_FAILURE;
    }
    g_vofw_func->pfnDF_StillScaler_Update(pstDispPool, src_width, src_height, dst_width, dst_height, bProgressive); 
    
    return MT_SUCCESS;
}

mt_void DF_DumpScaler_Update(mt_handle pstDispPool, MT_U32 src_width, MT_U32 src_height, MT_U32 dst_width, MT_U32 dst_height)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC_VOID();
    if (g_vofw_func->pfnDF_DumpScaler_Update== NULL)
    {
        MT_ERR_VO("pfnDF_DumpScaler_Update is null!\n");
        return;
    }
    g_vofw_func->pfnDF_DumpScaler_Update(pstDispPool, src_width, src_height, dst_width, dst_height); 
    
    return;
}

//init coeff table in VO FW
/*
mt_s32 DF_InitCoeffTable(void)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();
    if (g_vofw_func->pfnDF_InitCoeffTable== NULL)
    {
        MT_ERR_VO("pfnDF_InitCoeffTable is null!\n");
        return MT_FAILURE;
    }
    return g_vofw_func->pfnDF_InitCoeffTable(); 
}
*/

mt_u32 DF_GetSlHdrVersion(void)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    if (g_vofw_func->pfnDF_GetSlHdrVersion == NULL)
    {
        MT_ERR_VO("pfnDF_DF_GetSlHdrVersion is null!\n");
        return 0;
    }

    return g_vofw_func->pfnDF_GetSlHdrVersion();
}


////
int PS_AddOrder(PRESCALE_ORDERINFO* pstOrderInfo)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC();

    CHECK_NULL_PARAM(pstOrderInfo);

    //TODO:
    return MT_SUCCESS;
}

void PS_Loop(mt_void)
{
    GET_VOFW_FUNC();
    CHECK_VOFW_FUNC_VOID();

    //TODO:
    return;
}

extern MT_BOOL b_disp_coeff_update; 
void VO_FW_Inited(void)
{
	(void)mt_drv_module_getfunction(MT_ID_VO_FW, (mt_void **)&g_vofw_func);
    //DF_InitCoeffTable();  //force init coeff table bug26070 //init coeff table in vo fw
    b_disp_coeff_update = 1;
} 
EXPORT_SYMBOL(VO_FW_Inited);
