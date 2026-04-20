/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>
#include <linux/string.h>

#include "mt_type.h"

#include "demux_debug.h"
#include "drv_demux_scd.h"


#define PVR_INDEX_INVALID_PTSMS             ((mt_u32)(-1))

/* pvr index's SCD descriptor                                               */
/* format between firmware index and demux scd */
typedef struct
{
    mt_u8  u8IndexType;                           /* type of index(pts,sc,pause,ts) */
    mt_u8  u8StartCode;                           /* type of start code */
    mt_u16 u16OffsetInTs;                         /* start code offset in a TS package */

    mt_u32 u32OffsetInDavBuf;                     /* start code offset in DAV buffer */

    mt_u8  au8ByteAfterSC[8];                      /* 8Byte next to SC */


    mt_u64 u64TSCnt;                              /* count of TS package */

    mt_u32 u32PtsMs;
    mt_u16 u16OverFlow;
    mt_u16 u16Reserv;
}PVR_INDEX_SCD_S;



static mt_void PVR_SCDDmxIdxToPvrIdx(const DMX_IDX_DATA_S *pDmxIndexData,
                PVR_INDEX_SCD_S *pPvrIndexData)
{
    PVR_INDEX_SCD_S indexData;

    indexData.u16OverFlow = !((pDmxIndexData->u32Chn_Ovflag_IdxType_Flags >> 28) & 0x1);
    if (indexData.u16OverFlow)
    {
        //MT_INFO_DEMUX("indexData.u16OverFlow == 1\n");
    }

    indexData.au8ByteAfterSC[0]
        = (pDmxIndexData->u32ScType_Byte12AfterSc_OffsetInTs >> 16) & 0xffU;
    indexData.au8ByteAfterSC[1]
        = (pDmxIndexData->u32ScType_Byte12AfterSc_OffsetInTs >> 8) & 0xffU;
    indexData.au8ByteAfterSC[2]
        = (pDmxIndexData->u32TsCntHi8_Byte345AfterSc >> 16) & 0xffU;
    indexData.au8ByteAfterSC[3]
        = (pDmxIndexData->u32TsCntHi8_Byte345AfterSc >> 8) & 0xffU;
    indexData.au8ByteAfterSC[4]
        = (pDmxIndexData->u32TsCntHi8_Byte345AfterSc) & 0xffU;
    indexData.au8ByteAfterSC[5]
        = (pDmxIndexData->u32ScCode_Byte678AfterSc >> 16) & 0xffU;
    indexData.au8ByteAfterSC[6]
        = (pDmxIndexData->u32ScCode_Byte678AfterSc >> 8) & 0xffU;
    indexData.au8ByteAfterSC[7]
        = (pDmxIndexData->u32ScCode_Byte678AfterSc) & 0xffU;

    indexData.u16OffsetInTs = pDmxIndexData->u32ScType_Byte12AfterSc_OffsetInTs & 0x00ff;
    indexData.u64TSCnt = pDmxIndexData->u32TsCntHi8_Byte345AfterSc & 0xff000000;
    indexData.u64TSCnt = indexData.u64TSCnt << 8;
    indexData.u64TSCnt |= pDmxIndexData->u32TsCntLo32;
    indexData.u32OffsetInDavBuf = pDmxIndexData->u32BackPacetNum & 0x0001fffff;

    indexData.u8IndexType = (pDmxIndexData->u32Chn_Ovflag_IdxType_Flags >> 24) & 0xf;
    indexData.u8StartCode = (pDmxIndexData->u32ScType_Byte12AfterSc_OffsetInTs >> 24) & 0xff;
    //indexData.u32OffsetInDavBuf += indexData.u16OffsetInTs;
    indexData.u32PtsMs = PVR_INDEX_INVALID_PTSMS;
    indexData.u16Reserv = 0;

    if (DMX_INDEX_SC_TYPE_PTS == indexData.u8IndexType)
    {
        if (0 == (pDmxIndexData->u32TsCntHi8_Byte345AfterSc & 0x2))
        {
            indexData.u32PtsMs = PVR_INDEX_INVALID_PTSMS;
        }
        else
        {
            mt_u64 Pts      = 0;
            mt_u32 Pts33    = pDmxIndexData->u32TsCntHi8_Byte345AfterSc & 0x1;
            mt_u32 Pts32    = pDmxIndexData->u32ScCode_Byte678AfterSc;

            if (Pts33)
            {
                Pts = 0x100000000ULL;
            }

            Pts += (mt_u64)Pts32;

            Pts = Pts >> 1;

            indexData.u32PtsMs = (mt_u32)Pts;

            indexData.u32PtsMs /= 45;
        }

        //indexData.u8IndexType = DMX_INDEX_SC_TYPE_PIC;
        //indexData.u8StartCode = PVR_INDEX_SC_PIC;

        //indexData.au8ByteAfterSC[1] = s_refCounter++;
        //indexData.au8ByteAfterSC[1] = indexData.au8ByteAfterSC[1] << 6;
    }

    memcpy(pPvrIndexData, &indexData, sizeof(PVR_INDEX_SCD_S));
    return;
}


static mt_u64 PVR_SCDIndexCalcGlobalOffset(MT_BOOL bUseTimeStamp,const PVR_INDEX_SCD_S *pScData)
{
    mt_u64 offset; /* frame header offset (tota value) */

    if ( bUseTimeStamp)
    {
        offset = (pScData->u64TSCnt - pScData->u32OffsetInDavBuf - 1) * 192ULL + pScData->u16OffsetInTs;
    }
    else
    {
        offset = (pScData->u64TSCnt - pScData->u32OffsetInDavBuf - 1) * 188ULL + pScData->u16OffsetInTs;
    }
    return offset;
}


/*****************************************************************************
 Prototype       : DmxScdToVideoIndex
 Description     : transform SCD data to INDEX structure
 Input           : handle   **
                   pScData  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2010/06/17
    Author       : j40671
    Modification : Created function

*****************************************************************************/
mt_s32 DmxScdToVideoIndex(MT_BOOL bUseTimeStamp,const DMX_IDX_DATA_S *ScData, FINDEX_SCD_S *pstFidx)
{
    PVR_INDEX_SCD_S IndexData;
    mt_u64          CurrGlobalOffset = 0;

    PVR_SCDDmxIdxToPvrIdx(ScData, &IndexData);

    /* just only deal with SC of frame and pts */
    if (   (DMX_INDEX_SC_TYPE_PTS != IndexData.u8IndexType)
        && (DMX_INDEX_SC_TYPE_PIC != IndexData.u8IndexType)
        && (DMX_INDEX_SC_TYPE_PIC_SHORT != IndexData.u8IndexType) )
    {
        return MT_FAILURE;
    }

    /* need to calculate global offset, that is, the size from start record to current, included rewind */
    if (DMX_INDEX_SC_TYPE_PTS != IndexData.u8IndexType)
    {
        CurrGlobalOffset = PVR_SCDIndexCalcGlobalOffset(bUseTimeStamp,&IndexData);
    }
    else
    {
        if (PVR_INDEX_INVALID_PTSMS == IndexData.u32PtsMs)
        {
            return MT_FAILURE;
        }
    }

    pstFidx->s64GlobalOffset    = (mt_s64)CurrGlobalOffset;
    pstFidx->u32PtsMs           = IndexData.u32PtsMs;
    pstFidx->u8IndexType        = IndexData.u8IndexType;
    pstFidx->u8StartCode        = IndexData.u8StartCode;

    memcpy(pstFidx->au8DataAfterSC, IndexData.au8ByteAfterSC, 8);

    return MT_SUCCESS;
}

mt_s32 DmxScdToAudioIndex(MT_UNF_DMX_REC_INDEX_S *CurrFrame, const DMX_IDX_DATA_S *ScData)
{
    mt_u32  IndexType   = (ScData->u32Chn_Ovflag_IdxType_Flags >> 24) & 0xF;
    mt_u32  PtsValid    = (ScData->u32TsCntHi8_Byte345AfterSc >> 1) & 0x1;
    // mt_u64  LastOffset  = CurrFrame->u64GlobalOffset;
    mt_u64  Pts         = 0;
    mt_u32  Pts33       = ScData->u32TsCntHi8_Byte345AfterSc & 0x1;
    mt_u32  Pts32       = ScData->u32ScCode_Byte678AfterSc;
    mt_u32  PtsValue;
    mt_u64  TsCount;
    mt_u32  OffsetInTs;
    mt_u32  BackTsCount;

    if ((DMX_INDEX_SC_TYPE_PTS != IndexType) || (0 == PtsValid))
    {
        return MT_FAILURE;
    }

    if (Pts33)
    {
        Pts = 0x100000000ULL;
    }

    Pts += (mt_u64)Pts32;

    Pts = Pts >> 1;

    PtsValue = (mt_u32)Pts;

    PtsValue /= 45;

    OffsetInTs  = ScData->u32ScType_Byte12AfterSc_OffsetInTs & 0xff;
    TsCount     = (mt_u64)(ScData->u32TsCntHi8_Byte345AfterSc & 0xff000000) << 8;
    TsCount     = TsCount | ScData->u32TsCntLo32;
    BackTsCount = ScData->u32BackPacetNum & 0x0001fffff;

    CurrFrame->enFrameType      = MT_UNF_FRAME_TYPE_I;
    CurrFrame->u32PtsMs         = PtsValue;
    CurrFrame->u64GlobalOffset  = (TsCount - BackTsCount - 1) * 188 + OffsetInTs;
    //CurrFrame->u32FrameSize     = (mt_u32)(CurrFrame->u64GlobalOffset - LastOffset);
    CurrFrame->u32DataTimeMs    = ScData->u32SrcClk / 90;

    return MT_SUCCESS;
}

mt_void DmxRecUpdateFrameInfo(mt_u32 *Param, FRAME_POS_S *IndexInfo)
{
    MT_UNF_DMX_REC_INDEX_S *FrameInfo = (MT_UNF_DMX_REC_INDEX_S*)Param;

    switch (IndexInfo->eFrameType)
    {
        case FIDX_FRAME_TYPE_I :
            FrameInfo->enFrameType = MT_UNF_FRAME_TYPE_I;

            break;

        case FIDX_FRAME_TYPE_P :
            FrameInfo->enFrameType = MT_UNF_FRAME_TYPE_P;

            break;

        case FIDX_FRAME_TYPE_B :
            FrameInfo->enFrameType = MT_UNF_FRAME_TYPE_B;

            break;

        default :
            return;
    }

    FrameInfo->u32PtsMs          = IndexInfo->u32PTS;
    FrameInfo->u64GlobalOffset   = (mt_u64)IndexInfo->s64GlobalOffset;
    FrameInfo->u32FrameSize      = (mt_u32)IndexInfo->s32FrameSize;
}

