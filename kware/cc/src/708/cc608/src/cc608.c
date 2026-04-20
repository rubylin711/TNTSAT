/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


/*******************************************************************************
*                       Include files
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#include "mt_cc608_def.h"

#include "cc608.h"
#include "cc608_obj.h"
#include "cc608_data.h"
#include "cc608_dec.h"
#include "cc608_xds.h"


#define CHECKDATA(data,vlue) if (data >= vlue) { \
    MT_ERR_CC(#data"is invalid\n"); \
    return MT_FAILURE;\
}

#define CHECKCLRRANGE(rgb) if( (rgb!= 0x00000000)&&(rgb!= 0x0000005f)&&(rgb!= 0x000000af)&&(rgb!= 0x000000ff) ){ \
    MT_ERR_CC("this color is invalid\n"); \
    return MT_FAILURE;\
}

#define CHECKCLR(color) do{ \
     CHECKCLRRANGE(((color&0x00ff0000)>>16)); \
     CHECKCLRRANGE(((color&0x0000ff00)>>8)); \
     CHECKCLRRANGE(((color&0x000000ff))); \
}while(0)

/*******************************************************************************
*                       Export Function Definition
*******************************************************************************/

MT_S32 CC608_Init(MT_VOID)
{
    return MT_SUCCESS;
}
MT_S32 CC608_DeInit(MT_VOID)
{
    return MT_SUCCESS;
}
/*******************************************************************************
* Function: MT_S32 CC608_Create(int moduleid)
* Description: CC module init routine
*******************************************************************************/

MT_S32 CC608_Create(MT_U8 moduleid)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);

    MT_INFO_CC("CC608_Open module %d\n", moduleid);

    if (pstObject == NULL)
    {
        MT_ERR_CC("CC608_Open handle error\n");
        return MT_FAILURE;
    }

    CC608_DATA_Create( moduleid);

    pstObject->s32VChipTimerID = CC608_XDS_Create(moduleid);
    pstObject->s32TimeoutEraseTimerID = CC608_TimeoutErase_TimerInit(moduleid);

    return MT_SUCCESS;
}

/*******************************************************************************
* Function: MT_S32 CC608_Destroy(int moduleid)
* Description: CC module shutdown routine
*******************************************************************************/
MT_S32 CC608_Destroy(MT_U8 moduleid)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    TRI_QUEUE *pstCCDataQueue = NULL;

    if (pstObject == NULL)
    {
        MT_ERR_CC("CC608_Destroy handle error\n");
        return MT_FAILURE;
    }

    EnterCriticalSection( &pstObject->stCriticalSection );
    CC608_DATA_Reset(moduleid);
    pstCCDataQueue = &(pstObject->stCCDataQueue);
    CCQueue_Flush(pstCCDataQueue);
    LeaveCriticalSection( &pstObject->stCriticalSection );
    CC608_OBJ_Destroy(moduleid);

    (MT_VOID)CC608_XDS_Destroy(moduleid);
    
    return MT_SUCCESS;
}

/*******************************************************************************
* Function: MT_S32 CC608_Config(MT_U8 moduleid,MT_S32 type,void *pConfig)
* Description: CC module configure routine
*******************************************************************************/
MT_S32 CC608_Config(MT_U8 moduleid,MT_UNF_CC_608_CONFIGPARAM_S *pstCC608Config)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    MT_UNF_CC_RECT_S stScreenRect;

    if (pstObject == NULL || pstCC608Config == NULL)
    {
        MT_ERR_CC("CC608_Config handle error\n");
        return MT_FAILURE;
    }

    CHECKDATA(pstCC608Config->enCC608DataType, MT_UNF_CC_608_DATATYPE_BUTT);
    CHECKCLR(pstCC608Config->u32CC608TextColor);
    CHECKDATA(pstCC608Config->enCC608TextOpac,  MT_UNF_CC_OPACITY_BUTT);
    CHECKCLR(pstCC608Config->u32CC608BgColor);
    CHECKDATA(pstCC608Config->enCC608BgOpac,  MT_UNF_CC_OPACITY_BUTT);
    CHECKDATA(pstCC608Config->enCC608FontStyle,  MT_UNF_CC_FONTSTYLE_BUTT);
    CHECKDATA(pstCC608Config->enCC608DispFormat,  MT_UNF_CC_DF_BUTT);

    if (pstCC608Config->enCC608TextOpac != MT_UNF_CC_OPACITY_TRANSPARENT
        && pstCC608Config->enCC608BgOpac != MT_UNF_CC_OPACITY_TRANSPARENT)
    {
        if ((pstCC608Config->u32CC608TextColor != MT_UNF_CC_COLOR_DEFAULT && pstCC608Config->u32CC608TextColor == pstCC608Config->u32CC608BgColor)
            || (pstCC608Config->u32CC608TextColor == MT_UNF_CC_COLOR_DEFAULT && pstCC608Config->u32CC608BgColor == MT_UNF_CC_COLOR_WHITE)
            || (pstCC608Config->u32CC608TextColor == MT_UNF_CC_COLOR_BLACK && pstCC608Config->u32CC608BgColor == MT_UNF_CC_COLOR_DEFAULT))
        {
            MT_ERR_CC("CC608_Config color(%d %d) error\n",pstCC608Config->u32CC608TextColor,pstCC608Config->u32CC608BgColor);
            return MT_FAILURE;
        }
    }

    // enter the critical section
    EnterCriticalSection( &pstObject->stCriticalSection );
    if(pstObject->enCCNum != pstCC608Config->enCC608DataType)
    {
        /* config CCNUM */
        (MT_VOID)CC608_DATA_ClearSTA(moduleid);
        pstObject->enCCNum = pstCC608Config->enCC608DataType;
        if(pstObject->enCCNum <= MT_UNF_CC_608_DATATYPE_CC4 )
        {
            pstObject->enCurMode = CC608_MODE_CAPTION;
        }
        else if( pstObject->enCCNum >= MT_UNF_CC_608_DATATYPE_TEXT1 && pstObject->enCCNum <= MT_UNF_CC_608_DATATYPE_TEXT4 )
        {
            pstObject->enCurMode = CC608_MODE_TEXT;
        }
        else
        {
            pstObject->enCurMode = CC608_MODE_NOCAPTION;
        }
    }
    pstObject->u8IsModeModified = 1;
    pstObject->enVbiMode = CC608_MODE_NOCAPTION;

    MT_INFO_CC("CC608_Config CCNUM to %d=======",pstObject->enCCNum);
    MT_INFO_CC("current mode = %d\n", pstObject->enCurMode);


    /* config Screen */
    switch (pstCC608Config->enCC608DispFormat)
    {
        case MT_UNF_CC_DF_720X480:
            stScreenRect.width = 720;
            stScreenRect.height = 480;
            break;
        case MT_UNF_CC_DF_720X576:
            stScreenRect.width = 720;
            stScreenRect.height = 576;
            break;
        case MT_UNF_CC_DF_960X540:
            stScreenRect.width = 960;
            stScreenRect.height = 540;
            break;
        case MT_UNF_CC_DF_1280X720:
            stScreenRect.width = 1280;
            stScreenRect.height = 720;
            break;
        case MT_UNF_CC_DF_1920X1080:
            stScreenRect.width = 1920;
            stScreenRect.height = 1080;
            break;
        default:
            stScreenRect.width = 1280;
            stScreenRect.height = 720;
            break;
    }

    pstObject->stScreen.stPhyScreen.width  = stScreenRect.width;
    pstObject->stScreen.stPhyScreen.height = stScreenRect.height;
    pstObject->stScreen.stStaScreen.x      = stScreenRect.width/10;
    pstObject->stScreen.stStaScreen.y      = stScreenRect.height/10;
    pstObject->stScreen.stStaScreen.width  = stScreenRect.width*8/10;
    pstObject->stScreen.stStaScreen.height = stScreenRect.height*8/10;
    pstObject->stScreen.u8RowHeight        = pstObject->stScreen.stStaScreen.height / CC608ROWS;
    pstObject->stScreen.u8CharWidth        = pstObject->stScreen.stStaScreen.width / CC608COLUMNS;
    (MT_VOID)CCDISP_Screen_SetSize(pstObject->stScreen.stPhyScreen);
    (MT_VOID)CCDISP_Screen_SetClipArea(pstObject->stScreen.stStaScreen);
    (MT_VOID)CCDISP_Window_SetSize(pstObject->enWindowId, pstObject->stScreen.stStaScreen);


    /* config fg color */
    pstObject->stUserPenAttr.u32FgColor = pstCC608Config->u32CC608TextColor;
    MT_INFO_CC("pstObject->stUserPenAttr.u32FgColor = 0x%x\n",pstCC608Config->u32CC608TextColor);

    /* config fb opacity */
    pstObject->stUserPenAttr.enFgOpacity = pstCC608Config->enCC608TextOpac;
    MT_INFO_CC("pstObject->stUserPenAttr.enFgOpacity = %s\n",DBG_OPA_OUT[pstCC608Config->enCC608TextOpac]);

    /* config bg color */
    pstObject->stUserPenAttr.u32BgColor = pstCC608Config->u32CC608BgColor;
    MT_INFO_CC("pstObject->stUserPenAttr.u32BgColor = 0x%x\n",pstCC608Config->u32CC608BgColor);

    /* config bg opacity */
    pstObject->stUserPenAttr.enBgOpacity = pstCC608Config->enCC608BgOpac;
    MT_INFO_CC("pstObject->stUserPenAttr.enBgOpacity = %s\n",DBG_OPA_OUT[pstCC608Config->enCC608BgOpac]);

    /* config font style */
    pstObject->stUserPenAttr.bIsItalic = MT_FALSE;
    pstObject->stUserPenAttr.bIsUnderline = MT_FALSE;
    if((pstCC608Config->enCC608FontStyle == MT_UNF_CC_FONTSTYLE_ITALIC)
        || (pstCC608Config->enCC608FontStyle == MT_UNF_CC_FONTSTYLE_ITALIC_UNDERLINE))
    {
        pstObject->stUserPenAttr.bIsItalic = MT_TRUE;
    }
    if((pstCC608Config->enCC608FontStyle == MT_UNF_CC_FONTSTYLE_UNDERLINE)
        || (pstCC608Config->enCC608FontStyle == MT_UNF_CC_FONTSTYLE_ITALIC_UNDERLINE))
    {
        pstObject->stUserPenAttr.bIsUnderline = MT_TRUE;
    }
    pstObject->bLeadingTailingSpace = pstCC608Config->bLeadingTailingSpace;

    LeaveCriticalSection( &pstObject->stCriticalSection );

    return MT_SUCCESS;
}

MT_S32 CC608_GetConfig(MT_U8 moduleid,MT_UNF_CC_608_CONFIGPARAM_S *pstCC608ConfigParam)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    if (NULL == pstObject || NULL == pstCC608ConfigParam)
    {
        MT_ERR_CC("CC608_GetConfig param error,can not operate null pointer\n");
        return MT_FAILURE;
    }

    pstCC608ConfigParam->bLeadingTailingSpace = pstObject->bLeadingTailingSpace;
    pstCC608ConfigParam->u32CC608BgColor = pstObject->stUserPenAttr.u32BgColor;
    pstCC608ConfigParam->enCC608BgOpac = pstObject->stUserPenAttr.enBgOpacity;
    pstCC608ConfigParam->enCC608DataType = pstObject->enCCNum;
    pstCC608ConfigParam->u32CC608TextColor = pstObject->stUserPenAttr.u32FgColor;
    pstCC608ConfigParam->enCC608TextOpac = pstObject->stUserPenAttr.enFgOpacity;
    pstCC608ConfigParam->enCC608FontStyle = MT_UNF_CC_FONTSTYLE_NORMAL;

    if(pstObject->stUserPenAttr.bIsItalic)
    {
        pstCC608ConfigParam->enCC608FontStyle = MT_UNF_CC_FONTSTYLE_ITALIC;
        if(pstObject->stUserPenAttr.bIsUnderline)
        {
            pstCC608ConfigParam->enCC608FontStyle = MT_UNF_CC_FONTSTYLE_ITALIC_UNDERLINE;
        }
    }
    else if(pstObject->stUserPenAttr.bIsUnderline)
    {
        pstCC608ConfigParam->enCC608FontStyle = MT_UNF_CC_FONTSTYLE_UNDERLINE;
    }

    if(pstObject->stScreen.stPhyScreen.width == 1280 && pstObject->stScreen.stPhyScreen.height == 720)
    {
        pstCC608ConfigParam->enCC608DispFormat = MT_UNF_CC_DF_1280X720;
    }
    else if(pstObject->stScreen.stPhyScreen.width == 1920 && pstObject->stScreen.stPhyScreen.height == 1080)
    {
        pstCC608ConfigParam->enCC608DispFormat = MT_UNF_CC_DF_1920X1080;
    }
    else if(pstObject->stScreen.stPhyScreen.width == 720 && pstObject->stScreen.stPhyScreen.height == 480)
    {
        pstCC608ConfigParam->enCC608DispFormat = MT_UNF_CC_DF_720X480;
    }
    else if(pstObject->stScreen.stPhyScreen.width == 720 && pstObject->stScreen.stPhyScreen.height == 576)
    {
        pstCC608ConfigParam->enCC608DispFormat = MT_UNF_CC_DF_720X576;
    }
    else if(pstObject->stScreen.stPhyScreen.width == 960 && pstObject->stScreen.stPhyScreen.height == 540)
    {
        pstCC608ConfigParam->enCC608DispFormat = MT_UNF_CC_DF_960X540;
    }
    else
    {
        pstCC608ConfigParam->enCC608DispFormat = MT_UNF_CC_DF_BUTT;
    }

    return MT_SUCCESS;
}
/*******************************************************************************
* Function: MT_U8 CC608_IsStart(MT_U8 moduleid)
* Description: CC module start routine
*******************************************************************************/
MT_U8 CC608_IsStart(MT_U8 moduleid)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    MT_U8 u8Value;

    if (pstObject == NULL )
    {
        MT_ERR_CC("CC_Close handle error\n");
        return 0;
    }

    EnterCriticalSection( &pstObject->stCriticalSection );
    u8Value = pstObject->u8IsDecStart;
    LeaveCriticalSection( &pstObject->stCriticalSection );
    return u8Value;
}

/*******************************************************************************
* Function: MT_S32 CC_Start(int moduleid)
* Description: CC module start routine
*******************************************************************************/
MT_S32 CC608_Start(MT_U8 moduleid)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    TRI_QUEUE *pstCCDataQueue = NULL;

    if ( pstObject == NULL )
    {
        MT_ERR_CC("CC_Close handle error\n");
        return MT_FAILURE;
    }

    MT_INFO_CC("Start CC608 Decoder!\n");

    if(pstObject->u8IsDecStart)
    {
        return MT_SUCCESS;
    }

    EnterCriticalSection( &pstObject->stCriticalSection );

    pstCCDataQueue = &(pstObject->stCCDataQueue);
    CCQueue_Flush(pstCCDataQueue);

    pstObject->u8IsCaptureText = MT_FALSE;
    pstObject->u8IsDecStart = 1;
    CC608_DATA_Reset( moduleid );

    LeaveCriticalSection( &pstObject->stCriticalSection );

    return MT_SUCCESS;
}


/*******************************************************************************
* Function: MT_S32 CC608_Stop(int moduleid)
* Description: CC module stop routine
*******************************************************************************/
MT_S32 CC608_Stop(MT_U8 moduleid)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    TRI_QUEUE *pstCCDataQueue = NULL;

    if (pstObject == NULL)
    {
        MT_ERR_CC("CC_Close handle error\n");
        return MT_FAILURE;
    }

    MT_INFO_CC("Stop CC608 Decoder!\n");
    if(pstObject->u8IsDecStart == 0)
    {
        return MT_SUCCESS;
    }

    EnterCriticalSection( &pstObject->stCriticalSection );

    pstCCDataQueue = &(pstObject->stCCDataQueue);
    CCQueue_Flush(pstCCDataQueue);
    (MT_VOID)CC608_DATA_ClearSTA(moduleid);
    CC608_DATA_SetDefaultRowColumn(moduleid,CC608_MODE_NOCAPTION,CC608_DISP_PAINTON);
    CC608_TimeoutErase_TimerStop(moduleid);
    pstObject->u8IsDecStart = 0;

    LeaveCriticalSection( &pstObject->stCriticalSection );

    return MT_SUCCESS;
}

/***************************************************************************
* Function: MT_S32 CC608_Reset(int moduleid)                          *
* Description: CC module Reset routine                                     *
*    clear screen. flush vbi data queue.
****************************************************************************/
MT_S32 CC608_Reset(MT_U8 moduleid)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    TRI_QUEUE *pstCCDataQueue = NULL;
    if (pstObject == NULL)
    {
        MT_ERR_CC("CC608_Reset:module error!\n");
        return MT_FAILURE;
    }

    if(pstObject->u8IsDecStart == 0)
    {
        return MT_SUCCESS;
    }

    MT_INFO_CC("CC608_Reset!\n");

    EnterCriticalSection( &pstObject->stCriticalSection );

    pstCCDataQueue = &(pstObject->stCCDataQueue);
    CCQueue_Flush(pstCCDataQueue);
    CC608_DATA_Reset(moduleid);
    CC608_XDS_Reset(moduleid);

    LeaveCriticalSection( &pstObject->stCriticalSection );
    return MT_SUCCESS;
}

/*******************************************************************************
* Function: MT_S32 CC608_VBIParse(MT_U8 moduleid, MT_U8 *pucCCData, MT_U8 ucCCCount)

* Description: CC module vbi parse routine
*******************************************************************************/
MT_S32 CC608_VBIParse(MT_U8 moduleid, MT_U8 *pu8CCData, MT_U8 u8CCCount)
{
    CC608_OBJECT_S *pstObject = CC608_OBJ_Get(moduleid);
    CC608_ELEMENT_S stCCElement;
    TRI_QUEUE *pstCCDataQueue = NULL;
    MT_U8 i;
    MT_U8 u8FieldNum;
    MT_U16 u16CCData;

    if (pstObject == NULL || pu8CCData==NULL)
    {
        MT_ERR_CC("CC608_VBIParse handle error\n");
        return MT_FAILURE;
    }

    for( i = 0; i+2 < u8CCCount; i+=3 )
    {
        MT_U32 n=0;
        u8FieldNum = pu8CCData[i];
        u16CCData = (pu8CCData[i+1]&0x7f)|((pu8CCData[i+2]&0x7f)<<8) ;

        /* return value 2 means XDS data, then we don't need to inserted it in the queue */
        if(CC608_DEC_CheckXDS(moduleid, &u16CCData, u8FieldNum) == 2) /*XDS data*/
        {
            continue;
        }

        pstCCDataQueue = &(pstObject->stCCDataQueue);
        n = CCQueue_GetHeadPos(pstCCDataQueue);
        stCCElement.u16BufferSize = CC608DATA_BUFFER_SIZE;
        stCCElement.u32DataSize = CC608DATA_BUFFER_SIZE;
        stCCElement.pu8Buffer = (MT_U8 *)pstObject->au16CCData[n];
        memcpy(stCCElement.pu8Buffer,&pu8CCData[i],CC608DATA_BUFFER_SIZE);

        if(!CCQueue_Insert(pstCCDataQueue,&stCCElement))
        {
            MT_WARN_CC("CC608_VBIParse():queue full\n");
            CCQueue_Flush(pstCCDataQueue);
            return MT_SUCCESS;
        }
    }
    return MT_SUCCESS;
}

MT_S32 CC608_Decode(MT_U8 module_id,MT_U16 *pu16CCData,  MT_U8 u8FieldNum)
{
    return CC608_DEC_Decode(module_id, pu16CCData, u8FieldNum);
}

MT_S32 CC608_ConvertCC608Char2Unicode(MT_U8 *pu8CCChars, MT_U32 u32Len, MT_U16 *pu16UniChars)
{
    return CC608_DEC_Convert2Unicode(pu8CCChars,u32Len,pu16UniChars);
}

void CC608_XDSReset(MT_U8 moduleid)
{
    CC608_XDS_Reset(moduleid);
}

/*******************************************************************************
*                      End of file
*******************************************************************************/
