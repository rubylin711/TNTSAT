/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_unf_edid.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mt_mpi_edid.h"
typedef struct tagEDID_VIDEO_FORMAT_S
{
    MT_BOOL                      bProgressive;
    mt_u16                      u32Hact;
    mt_u16                      u32Vact;
    mt_u16                      u32VerFreq;
    MT_UNF_ENC_FMT_E enFormat;
} EDID_VIDEO_FORMAT_S;

EDID_VIDEO_FORMAT_S g_EDIDVideoFmt[] =
{
    {MT_TRUE ,1920,1080,60,MT_UNF_ENC_FMT_1080P_60},
    {MT_TRUE ,1920,1080,50,MT_UNF_ENC_FMT_1080P_50},
    {MT_TRUE ,1920,1080,30,MT_UNF_ENC_FMT_1080P_30},
    {MT_TRUE ,1920,1080,25,MT_UNF_ENC_FMT_1080P_25},
    {MT_TRUE ,1920,1080,24,MT_UNF_ENC_FMT_1080P_24},

    {MT_FALSE ,1920,1080,60,MT_UNF_ENC_FMT_1080i_60},
    {MT_FALSE ,1920,1080,50,MT_UNF_ENC_FMT_1080i_50},

    {MT_TRUE ,1280,720,60,MT_UNF_ENC_FMT_720P_60},
    {MT_TRUE ,1280,720,50,MT_UNF_ENC_FMT_720P_50},

    {MT_TRUE ,720,576,60,MT_UNF_ENC_FMT_576P_50},
    {MT_TRUE ,720,480,50,MT_UNF_ENC_FMT_480P_60},

    {MT_FALSE,720,576,50,MT_UNF_ENC_FMT_PAL},
    {MT_FALSE ,720,480,60,MT_UNF_ENC_FMT_NTSC},
};

MT_UNF_ENC_FMT_E EDID_GetVideoFormat(EDID_VIDEO_TIMING_S *pstVideoTiming,MT_BOOL *pbNativeFormat);

MT_UNF_ENC_FMT_E EDID_GetVideoFormat(EDID_VIDEO_TIMING_S *pstVideoTiming,MT_BOOL *pbNativeFormat)
{
    mt_u32 i;
    if (!pstVideoTiming)
        return MT_UNF_ENC_FMT_BUTT;

    *pbNativeFormat = MT_FALSE;
    for ( i = 0; i < sizeof(g_EDIDVideoFmt)/sizeof(EDID_VIDEO_FORMAT_S); i++ )
    {
        if ((pstVideoTiming->bProgressive == g_EDIDVideoFmt[i].bProgressive)
            &&(pstVideoTiming->u32Hact == g_EDIDVideoFmt[i].u32Hact)
            &&(pstVideoTiming->u32Vact == g_EDIDVideoFmt[i].u32Vact)
            &&(pstVideoTiming->u32VerFreq == g_EDIDVideoFmt[i].u32VerFreq)
            )
        {

            if (MT_UNF_EDID_TIMING_ATTR_PREFERRED_TIMING == pstVideoTiming->enTimingAttr)
                *pbNativeFormat = MT_TRUE;
            else
                *pbNativeFormat = MT_FALSE;

            return g_EDIDVideoFmt[i].enFormat;
        }
    }
     return MT_UNF_ENC_FMT_BUTT;
}

mt_s32 MT_UNF_EDID_EdidParse(mt_u8 *pu8Edid, mt_u32 u32EdidLength,MT_UNF_EDID_INFO_S *pstEdidInfo)
{
    mt_u32 i;
    mt_s32 s32Ret;

    struct list_head* pos = MT_NULL;
    EDID_INFO_S  stEdidMpiInfo;
    EDID_SIMPLE_TIMING_S *pstSimpleTimingList;
    EDID_DETAIL_TIMING_S *pstDetailTimingList;
    EDID_VIDEO_TIMING_S *pstVideTimingList;
    MT_UNF_EDID_SIMPLE_TIMING_S   *pstSimpleTiming = MT_NULL ;	/* Est. stTimings and mfg rsvd stTimings*/
    MT_UNF_EDID_DETAIL_TIMING_S   *pstDetailTiming = MT_NULL;	    /* Detailing stTimings  */

    memset(&stEdidMpiInfo,0x0,sizeof(EDID_INFO_S));
    memset(pstEdidInfo,0x0,sizeof(MT_UNF_EDID_INFO_S));

    s32Ret = MT_MPI_EDID_EdidParse(pu8Edid, u32EdidLength,&stEdidMpiInfo);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_EDID("EDID parse err \n");
        return MT_FAILURE;
    }

    /*add Simple Timing*/
    pstEdidInfo->u32SimpleTimingNum = stEdidMpiInfo.u32SimpleTimingNum;
    if (MT_NULL != stEdidMpiInfo.pstSimpleTimingList)
     {
        pstSimpleTiming = malloc(sizeof(MT_UNF_EDID_SIMPLE_TIMING_S)*stEdidMpiInfo.u32SimpleTimingNum);
        if (NULL == pstSimpleTiming)
        {
            MT_ERR_EDID("maloc err \n");
            s32Ret = MT_FAILURE;
            goto ERR_EXIT1;
        }

        pos = &stEdidMpiInfo.pstSimpleTimingList->head;
        i =0;
        do
        {
            pstSimpleTimingList = list_entry(pos, EDID_SIMPLE_TIMING_S, head);
            pstSimpleTiming[i] = pstSimpleTimingList->stSimpleTiming;

            pos = pos->next;
            i++;

            if(i >= stEdidMpiInfo.u32SimpleTimingNum )
                break;
        }
        while (pos != &(stEdidMpiInfo.pstSimpleTimingList->head));
   }
/*add Detail Timing*/
    pstEdidInfo->u32DetailTimingNum = stEdidMpiInfo.u32DetailTimingNum;

    if (MT_NULL != stEdidMpiInfo.pstDetailTimingList)
    {
        pstDetailTiming = malloc(sizeof(MT_UNF_EDID_DETAIL_TIMING_S)*stEdidMpiInfo.u32DetailTimingNum);
        if (NULL == pstDetailTiming)
        {

            MT_ERR_EDID("maloc err \n");
            s32Ret = MT_FAILURE;
            goto ERR_EXIT1;
        }
        pos = &stEdidMpiInfo.pstDetailTimingList->head;
         i =0;
        do
        {
            pstDetailTimingList = list_entry(pos, EDID_DETAIL_TIMING_S, head);
            pstDetailTiming[i] = pstDetailTimingList->stDetailTiming;

            if (MT_UNF_EDID_TIMING_ATTR_PREFERRED_TIMING == pstDetailTiming->enTimingAttr)
                pstEdidInfo->stEDIDBaseInfo.stPerferTiming  = pstDetailTiming[i].stTiming ;

            pos = pos->next;
            i++;

            if(i >= stEdidMpiInfo.u32DetailTimingNum )
                break;
        }
        while (pos != &(stEdidMpiInfo.pstDetailTimingList->head));
   }
    /*add base info*/
    pstEdidInfo->enVideoPort = stEdidMpiInfo.enVideoPort;
    pstEdidInfo->stEDIDBaseInfo.u8Version = stEdidMpiInfo.stVersion.u8Version;
    pstEdidInfo->stEDIDBaseInfo.u8Revision = stEdidMpiInfo.stVersion.u8Revision;
    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u32ProductCode = stEdidMpiInfo.stVendor.u16ProductCode;
    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u32SerialNumber = stEdidMpiInfo.stVendor.u32SerialNum;
    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u32Week = stEdidMpiInfo.stVendor.u32MfWeek;
    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u32Year = stEdidMpiInfo.stVendor.u32MfYear;

    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u8MfrsName[0] = (mt_u8) stEdidMpiInfo.stVendor.cMfName[0];
    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u8MfrsName[1] = (mt_u8) stEdidMpiInfo.stVendor.cMfName[1];
    pstEdidInfo->stEDIDBaseInfo.stMfrsInfo.u8MfrsName[2] = (mt_u8) stEdidMpiInfo.stVendor.cMfName[2];


    if (stEdidMpiInfo.pstExitenInfo)
    {
        if ( stEdidMpiInfo.pstExitenInfo->pstVideoFormat )
        {
            pstEdidInfo->stEDIDBaseInfo.bSupportHdmi = MT_TRUE;
            pstEdidInfo->stEDIDBaseInfo.bSupportDVIDual = stEdidMpiInfo.pstExitenInfo->pstVideoFormat->bDVI_Dual;
            pstEdidInfo->stEDIDBaseInfo.bSupportsAI = stEdidMpiInfo.pstExitenInfo->pstVideoFormat->bSupportsAI;

            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColorY444= stEdidMpiInfo.pstExitenInfo->pstVideoFormat->bDC_Y444;
            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColor30Bit = stEdidMpiInfo.pstExitenInfo->pstVideoFormat->bDC_30bit;
            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColor36Bit= stEdidMpiInfo.pstExitenInfo->pstVideoFormat->bDC_36bit;
            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColor48Bit= stEdidMpiInfo.pstExitenInfo->pstVideoFormat->bDC_48bit;
        }
        else
        {
            pstEdidInfo->stEDIDBaseInfo.bSupportHdmi = MT_FALSE;
            pstEdidInfo->stEDIDBaseInfo.bSupportDVIDual = MT_FALSE;
            pstEdidInfo->stEDIDBaseInfo.bSupportsAI = MT_FALSE;

            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColorY444 = MT_FALSE;
            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColor30Bit = MT_FALSE;
            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColor36Bit = MT_FALSE;
            pstEdidInfo->stEDIDBaseInfo.stDeepColor.bDeepColor48Bit = MT_FALSE;
        }

        MT_UNF_ENC_FMT_E enFormat = MT_UNF_ENC_FMT_BUTT;
        MT_BOOL bNativeFormat = MT_FALSE;

        if (stEdidMpiInfo.pstExitenInfo->pstVideoTiming)
        {
            pos = &stEdidMpiInfo.pstExitenInfo->pstVideoTiming->head;
            do
            {
                pstVideTimingList = list_entry(pos, EDID_VIDEO_TIMING_S, head);
                enFormat = EDID_GetVideoFormat(pstVideTimingList,&bNativeFormat);
                if (MT_UNF_ENC_FMT_BUTT != enFormat)
                {
                    pstEdidInfo->stEDIDBaseInfo.bSupportFormat[enFormat] = MT_TRUE;

                    if (MT_TRUE == bNativeFormat)
                        pstEdidInfo->stEDIDBaseInfo.enNativeFormat = enFormat;
                }
                    pos = pos->next;
            }
            while (pos != &(stEdidMpiInfo.pstExitenInfo->pstVideoTiming->head));
        }
    }

    pstEdidInfo->pstSimpleTiming = pstSimpleTiming ;
    pstEdidInfo->pstDetailTiming = pstDetailTiming ;

    MT_MPI_EDID_EdidRelease(&stEdidMpiInfo);
    return s32Ret;

ERR_EXIT1:
    if (pstSimpleTiming)
        free(pstSimpleTiming);
    MT_MPI_EDID_EdidRelease(&stEdidMpiInfo);
    return s32Ret;
}

mt_void MT_UNF_EDID_EdidRelease(MT_UNF_EDID_INFO_S *pEdidInfo)
{
    if ( !pEdidInfo )
        return;

    if ( pEdidInfo->pstSimpleTiming )
        free(pEdidInfo->pstSimpleTiming);

    if (pEdidInfo->pstDetailTiming)
        free(pEdidInfo->pstDetailTiming);
}

