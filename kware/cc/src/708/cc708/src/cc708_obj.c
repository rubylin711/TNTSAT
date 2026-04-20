/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*******************************************************************************
*                       Include files
*******************************************************************************/

#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>

#include <string.h>
#include <semaphore.h>
#include <sys/ioctl.h>

#include "cc708.h"
#include "mt_cc708_def.h"
#include "cc708_obj.h"
#include "ccdisp_api.h"
#include "cc_debug.h"

static CC708_OBJECT_S CCObj[DTVCC_MAX_MODULE];

/*******************************************************************************
* Function: MT_S32  CC708_OBJ_Create(MT_U8 id)
* Description: initalize the CC object
*******************************************************************************/
MT_S32  CC708_OBJ_Create(MT_U8 u8ModuleID)
{
    if (u8ModuleID >= DTVCC_MAX_MODULE)
    {
        return MT_FAILURE;
    }

    CCObj[u8ModuleID].u8ModuleID = u8ModuleID;
    CCObj[u8ModuleID].u8IsStart = 0;
    CCObj[u8ModuleID].u8IsDelay = 0;
    CCObj[u8ModuleID].enModuleState = CC_READY;
    InitializeCriticalSection( &CCObj[u8ModuleID].mCriticalSection );

    /*User set options*/
    EnterCriticalSection(&CCObj[u8ModuleID].mCriticalSection);
    CCObj[u8ModuleID].u32ServiceType         = MT_UNF_CC_708_SERVICE1;
    LeaveCriticalSection(&CCObj[u8ModuleID].mCriticalSection);
    CCObj[u8ModuleID].u32UserWinOpac         = MT_UNF_CC_OPACITY_DEFAULT;
    CCObj[u8ModuleID].u32UserWinColor        = MT_UNF_CC_COLOR_DEFAULT;
    CCObj[u8ModuleID].u32UserTextBgColor     = MT_UNF_CC_COLOR_DEFAULT;
    CCObj[u8ModuleID].u32UserTextBGOpacity   = MT_UNF_CC_OPACITY_DEFAULT;
    CCObj[u8ModuleID].u32UserTextFGOpacity   = MT_UNF_CC_OPACITY_DEFAULT;
    CCObj[u8ModuleID].u32UserTextFGColor     = MT_UNF_CC_COLOR_DEFAULT;
    CCObj[u8ModuleID].u32UserFontName        = MT_UNF_CC_FN_DEFAULT;
    CCObj[u8ModuleID].u32UserFontSize        = MT_UNF_CC_FONTSIZE_DEFAULT;
    CCObj[u8ModuleID].u32UserFontStyle       = MT_UNF_CC_FONTSTYLE_DEFAULT;

    memset(&CCObj[u8ModuleID].stCMDBuffer, 0, sizeof(DTVCC_CMDBuffer_S));
    memset(&CCObj[u8ModuleID].stPacket, 0, sizeof(DTVCC_Packet_S));
    memset(&CCObj[u8ModuleID].stServiceBlock, 0, sizeof(DTVCC_ServiceBlock_S));
    CCObj[u8ModuleID].s32LastSeqenceNo       = -1;
    CCObj[u8ModuleID].bServiceBlockEnded     = MT_TRUE;

    CCObj[u8ModuleID].u8ScrollTimes = 6;


    CCQueue_Init(&(CCObj[u8ModuleID].stUserdataQueue),USERDATA_QUEUE_SIZE,sizeof(DTVCC_Element_S));
    CCQueue_Init(&(CCObj[u8ModuleID].stServiceQueue),SERVICE_QUEUE_SIZE,sizeof(DTVCC_ServiceElement_S));

    CCObj[u8ModuleID].stScreen.PhysicalRect.x          = 0;
    CCObj[u8ModuleID].stScreen.PhysicalRect.y          = 0;
    CCObj[u8ModuleID].stScreen.PhysicalRect.width      = 1280;
    CCObj[u8ModuleID].stScreen.PhysicalRect.height     = 720;
    CCObj[u8ModuleID].stScreen.CaptionRect.x           = CCObj[u8ModuleID].stScreen.PhysicalRect.width/10;
    CCObj[u8ModuleID].stScreen.CaptionRect.y           = CCObj[u8ModuleID].stScreen.PhysicalRect.height/10;
    CCObj[u8ModuleID].stScreen.CaptionRect.width       = CCObj[u8ModuleID].stScreen.PhysicalRect.width * 8 / 10;
    CCObj[u8ModuleID].stScreen.CaptionRect.height      = CCObj[u8ModuleID].stScreen.PhysicalRect.height* 8 / 10;
    CCObj[u8ModuleID].stScreen.u16MaxHorizonalCells    = MAX_CC_ABSOLUTE_ANCHOR_HORZ_16B9;
    CCObj[u8ModuleID].stScreen.u16MaxColumnNum         = MAX_CC_COLUMNS_4B3;
    (MT_VOID)CCDISP_Screen_SetSize(CCObj[u8ModuleID].stScreen.PhysicalRect);

    return MT_SUCCESS;
}

/*******************************************************************************
* Function: MT_BOOL DTVCCObjDestroy(MT_U8 id)
* Description: destroy the CC object
*******************************************************************************/
MT_BOOL CC708_OBJ_Destroy(MT_U8 id)
{
    if(id >= DTVCC_MAX_MODULE )
    {
        return MT_FALSE;
    }
    CCQueue_Destroy(&CCObj[id].stServiceQueue);
    CCQueue_Destroy(&CCObj[id].stUserdataQueue);
    memset(&CCObj[id], 0, sizeof(CC708_OBJECT_S));
    CCObj[id].enModuleState = CC_INIT;
    return MT_TRUE;
}

/*******************************************************************************
* Function: DTVCCOBJ * DTVCCGetObjHandle(MT_U8 id)
* Description: Get the CC Object handle
*******************************************************************************/
CC708_OBJECT_S * CC708_OBJ_GetHandle(MT_U8 ModuleId)
{
    CC708_OBJECT_S * pObject = NULL;
    if( ModuleId >= DTVCC_MAX_MODULE)
    {
        MT_ERR_CC("Invalide module id:%d!\n",ModuleId);
        return NULL;
    }

    pObject = &CCObj[ModuleId];
    return pObject;
}
/*******************************************************************************
*                       Static Function Definition
*******************************************************************************/
