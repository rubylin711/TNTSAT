/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HAL_DEMUX_H__
#define __HAL_DEMUX_H__

#include "drv_demux_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    DMX_CHAN_DATA_TYPE_SEC = 0,
    DMX_CHAN_DATA_TYPE_PES = 1,
} DMX_CHAN_DATA_TYPE_E;

typedef enum
{
    DMX_REC_TYPE_NONE       = 0,
    DMX_REC_TYPE_DESCRAM_TS = 1,
    DMX_REC_TYPE_PES        = 2,
    DMX_REC_TYPE_SCRAM_TS   = 3,
    DMX_REC_TYPE_ALL_TS     = 4,
    DMX_REC_TYPE_UNDEF
} DMX_REC_TYPE_E;

mt_void DmxHalDvbPortSetAttr(
        mt_u32                  PortId,
        MT_UNF_DMX_PORT_TYPE_E  PortType,
        mt_u32                  SyncOn,
        mt_u32                  SyncOff,
        mt_u32                  TunerInClk,
        mt_u32                  BitSelector
    );

mt_u32  DmxHalGetClk(mt_void);
mt_void DmxHalTSOPortSetAttr(mt_u32 PortId,MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr);

mt_void DmxHalDvbPortSetClkInPol(mt_u32 PortId, MT_BOOL bClkInPol);
mt_void DmxHalDvbPortSetTsCountCtrl(const mt_u32 PortId, const mt_u32 option);
mt_u32  DmxHalDvbPortGetTsPackCount(mt_u32 PortId);
mt_void DmxHalDvbPortSetErrTsCountCtrl(const mt_u32 PortId, const mt_u32 option);
mt_u32  DmxHalDvbPortGetErrTsPackCount(mt_u32 PortId);

mt_void DmxHalIPPortSetAttr(mt_u32 PortId, MT_UNF_DMX_PORT_TYPE_E PortType, mt_u32 SyncOn, mt_u32 SyncOff);
mt_void DmxHalIPPortSetSyncLen(mt_u32 PortId, mt_u32 SyncLen1, mt_u32 SyncLen2);

#ifdef DMX_RAM_PORT_AUTO_SCAN_SUPPORT
mt_void DmxHalIPPortSetAutoScanRegion(mt_u32 PortId, mt_u32 len, mt_u32 step);
#endif

mt_void DmxHalIPPortSetTsCountCtrl(const mt_u32 PortId, const MT_BOOL enable);
mt_u32  DmxHalIPPortGetTsPackCount(mt_u32 PortId);
mt_void DmxHalIPPortStartStream(const mt_u32 PortId, const MT_BOOL Enable);
mt_void DmxHalIPPortDescSet(mt_u32 PortId, mt_u32 StartAddr, mt_u32 Depth);
mt_void DmxHalIPPortDescAdd(const mt_u32 PortId, const mt_u32 DescNum);
mt_void DmxHalIPPortRateSet(mt_u32 PortId, mt_u32 Rate);

mt_void DmxHalGetChannelTSCount(mt_u32 ChanId, mt_u32 *ChanTsCount, mt_u32* OQTsCount);
mt_void DmxHalSetChannelDataType(mt_u32 ChanId, DMX_CHAN_DATA_TYPE_E DataType);
mt_void DmxHalSetChannelAFMode(mt_u32 ChanId, DMX_Ch_AFMode_E eAfMode);
mt_void DmxHalSetChannelCRCMode(mt_u32 ChanId, MT_UNF_DMX_CHAN_CRC_MODE_E CrcMode);
mt_void DmxHalSetChannelCCDiscon(mt_u32 ChanId, mt_u32 DiscardFlag);
mt_void DmxHalSetChannelPusiCtrl(mt_u32 ChanId, mt_u32 PusiCtrl);
mt_void DmxHalSetChannelCCRepeatCtrl(mt_u32 ChanId, mt_u32 CCRepeatCtrl);
mt_void DmxHalSetChannelTsPostMode(mt_u32 ChanId, mt_u32 TsPost);
mt_void DmxHalSetChannelTsPostThresh(mt_u32 ChanId, mt_u32 Threshold);
mt_void DmxHalSetChannelAttr(mt_u32 ChanId, DMX_Ch_ATTR_E echattr);
mt_void DmxHalSetChannelFltMode(mt_u32 ChanId, MT_BOOL bEnable);
mt_void DmxHalGetChannelPlayDmxid(mt_u32 ChanId, mt_u32 *dmxid);
mt_void DmxHalSetChannelPlayDmxid(mt_u32 ChanId, mt_u32 dmxid);
mt_void DmxHalSetChannelRecDmxid(mt_u32 ChanId, mt_u32 dmxid);
mt_void DmxHalSetChannelPid(mt_u32 ChanId, mt_u32 pid);
mt_void DmxHalSetChannelRecBufId(mt_u32 ChanId, mt_u32 bufid);
mt_void DmxHalSetChannelPlayBufId(mt_u32 ChanId, mt_u32 bufid);
mt_void DmxHalSetDemuxPortId(mt_u32 DmxId, DMX_PORT_MODE_E PortMode, mt_u32 PortId);
mt_void DmxHalSetDataFakeMod(MT_BOOL bFakeEn);
mt_void DmxHalSetRecType(mt_u32 DmxId, DMX_REC_TYPE_E RecType);
mt_void DmxHalFlushChannel(mt_u32 ChanId, DMX_FLUSH_TYPE_E FlushType);
MT_BOOL DmxHalIsFlushChannelDone(mt_void);

mt_void DmxHalSetTsRecBufId(mt_u32 DmxId, mt_u32 OqId);
mt_void DmxHalSetSpsRefRecCh(mt_u32 DmxId, mt_u32 ChanId);
mt_void DmxHalSetSpsPauseType(mt_u32 u32DmxId, mt_u32 type);
mt_void DmxHalSetFilter(mt_u32 FilterId, mt_u32 Depth, mt_u8 Content, MT_BOOL bReverse, mt_u8 Mask);
mt_void DmxHalAttachFilter(mt_u32 FilterId, mt_u32 ChanId);
mt_void DmxHalDetachFilter(mt_u32 FilterId, mt_u32 ChanId);

mt_void DmxHalClearOq(mt_u32 OqId, DMX_OQ_CLEAR_TYPE_E ClearType);
MT_BOOL DmxHalIsClearOqDone(mt_void);

mt_void DmxHalEnableAllPVRInt(mt_void);
mt_void DmxHalDisableAllPVRInt(mt_void);

mt_void DmxHalIPPortAutoClearBP(mt_void);
MT_BOOL DmxHalGetIPBPStatus(mt_u32 PortId);
mt_void DmxHalClrIPBPStatus(mt_u32 PortId);
mt_void DmxHalClrIPFqBPStatus(mt_u32 PortId, mt_u32 FQId);

mt_u32  DmxHalIPPortGetOutIntStatus(mt_u32 u32PortId);
mt_void DmxHalIPPortClearOutIntStatus(mt_u32 u32PortId);
mt_void DmxHalIPPortSetOutInt(const mt_u32 PortId, const MT_BOOL Enable);
mt_u32  DmxHalIPPortDescGetRead(mt_u32 PortId);
mt_void DmxHalIPPortEnableInt(mt_u32 PortId);
mt_void DmxHalIPPortDisableInt(mt_u32 PortId);


mt_u32  DmxHalOQGetAllEopIntStatus(mt_void);
mt_void DmxHalEnableAllChEopInt(mt_void);
mt_void DmxHalEnableAllChEnqueInt(mt_void);

mt_void DmxHalEnableFQOutqueInt(mt_void);
mt_void DmxHalDisableFQOutqueInt(mt_void);
mt_void DmxHalSetFlushMaxWaitTime(mt_u32 u32MaxTime);
mt_s32  DmxHalSetScdFilter(mt_u32 u32FltId, mt_u8 u8Content);
mt_s32  DmxHalSetScdRangeFilter(mt_u32 u32FltId, mt_u8 u8High, mt_u8 u8Low);
mt_void DmxHalSetScdNewRangeFilter(mt_u32 FilterId, mt_u8 High, mt_u8 Low, mt_u8 Mask, MT_BOOL Negate);

mt_void DmxHalChooseScdFilter(mt_u32 ScdId, mt_u32 FilterId);
mt_void DmxHalScdFilterClear(mt_u32 ScdId);
mt_void DmxHalChooseScdRangeFilter(mt_u32 ScdId, mt_u32 FilterId);
mt_void DmxHalScdRangeFilterClear(mt_u32 ScdId);
mt_void DmxHalChooseScdNewRangeFilter(mt_u32 ScdId, mt_u32 FilterId);
mt_void DmxHalScdNewRangeFilterClear(mt_u32 ScdId);

mt_void DmxHalEnablePesSCD(mt_u32 ScdId);
mt_void DmxHalDisablePesSCD(mt_u32 ScdId);
mt_void DmxHalEnableEsSCD(mt_u32 ScdId);
mt_void DmxHalDisableEsSCD(mt_u32 ScdId);
mt_void DmxHalEnableMp4SCD(mt_u32 ScdId);
mt_void DmxHalDisableMp4SCD(mt_u32 ScdId);
mt_void DmxHalSetSCDAttachChannel(mt_u32 ScdId, mt_u32 ChanId);
mt_void DmxHalAllocSCDBufferId(mt_u32 ScdId, mt_u32 OqId);

mt_void DmxHalSetFlushIPPort(mt_u32 PortId);

mt_void DmxHalGetChannelTSScrambleFlag(mt_u32 u32Chid, MT_BOOL  *pEnable);
mt_void DmxHalGetChannelPesScrambleFlag(mt_u32 u32Chid, MT_BOOL  *pEnable);

mt_void DmxHalSetPcrDmxId(const mt_u32 PcrId, const mt_u32 DmxId);
mt_void DmxHalSetPcrPid(const mt_u32 PcrId, const mt_u32 PcrPid);
mt_void DmxHalGetPcrValue(const mt_u32 PcrId, mt_u64 *PcrVal);
mt_void DmxHalGetScrValue(const mt_u32 PcrId, mt_u64 *ScrVal);

/*---------------------------test IP-----------------------------*/

mt_u32  DmxHalGetTotalTeiIntStatus(mt_void);
mt_u32  DmxHalGetTotalPcrIntStatus(mt_void);
mt_u32  DmxHalGetTotalDiscIntStatus(mt_void);
mt_u32  DmxHalGetTotalCrcIntStatus(mt_void);
mt_u32  DmxHalGetTotalPenLenIntStatus(mt_void);
mt_u32  DmxHalGetPcrIntStatus(mt_void);
mt_void DmxHalClrPcrIntStatus(const mt_u32 PcrId);
mt_void DmxHalSetPcrIntEnable(const mt_u32 PcrId, const MT_BOOL Enable);
mt_void DmxHalGetTeiIntInfo(mt_u32* pu32DmxId, mt_u32* pu32ChanId);
mt_void DmxHalClrTeiIntStatus(mt_void);
mt_u32  DmxHalGetDiscIntStatus(mt_u32 RegionNum);
mt_void DmxHalClearDiscIntStatus(mt_u32 ChanId);
mt_u32  DmxHalGetCrcIntStatus(mt_u32 RegionNum);
mt_void DmxHalClearCrcIntStatus(mt_u32 ChanId);
mt_u32  DmxHalGetPesLenIntStatus(mt_u32 RegionNum);
mt_void DmxHalClearPesLenIntStatus(mt_u32 ChanId);
mt_void DmxHalFlushScdBuf(mt_u32 u32ScdId);
mt_void DmxHalClrScdCnt(mt_u32 u32ScdId);
mt_void DmxHalGetRecTsCnt(mt_u32 ScdId, mt_u64 *TsCnt);
mt_void DmxHalGetCurrentSCR(mt_u32 *ScrClk);
mt_void DmxHalConfigHardware(mt_void);
mt_void DmxHalIPPortSetIntCnt(mt_u32 PortId, mt_u32 DescNum);
mt_void DmxHalGetTSOClkCfg(mt_u32 PortId,MT_BOOL *ClkReverse,mt_u32 *enClk,mt_u32 *ClkDiv);
mt_void DmxHalCfgTSOClk(mt_u32 PortId,MT_BOOL ClkReverse,mt_u32 enClk,mt_u32 ClkDiv);



mt_void DmxHalEnableOQOutDInt(mt_u32 u32OQId);
mt_void DmxHalDisableOQOutDInt(mt_u32 u32OQId);
MT_BOOL DmxHalGetOQEopIntStatus(mt_u32 u32OQId);
mt_void DmxHalClearOQEopIntStatus(mt_u32 u32OQId);
mt_void DmxHalEnableOQEopInt(mt_u32 u32OQId);
mt_void DmxHalDisableOQEopInt(mt_u32 u32OQId);

#ifdef MT_DEMUX_PROC_SUPPORT
mt_void DmxHalFQEnableAllOverflowInt(mt_void);
mt_u32  DmxHalFQGetAllOverflowIntStatus(mt_void);
mt_u32  DmxHalFQGetOverflowIntStatus(mt_u32 offset);
mt_u32  DmxHalFQGetOverflowIntType(mt_u32 offset);
mt_void DmxHalFQClearOverflowInt(mt_u32 FqId);
mt_void DmxHalFQSetOverflowInt(mt_u32 FqId, MT_BOOL Enable);
MT_BOOL DmxHalFQIsEnableOverflowInt(mt_u32 FqId);
mt_void DmxHalIPPortGetDescInfo(mt_u32 PortId, DMX_Proc_RamPort_DescInfo_S *DescInfo);
mt_void DmxHalIPPortGetBPStatus(mt_u32 PortId, DMX_Proc_RamPort_BPStatus_S *BPStatus);
mt_void DMXHalGetChannelDataFlow(mt_u32 ChannelId, ChannelDataFlow_info_t *ChannelDF);
#endif

mt_u32  DmxHalOQGetAllOverflowIntStatus(mt_void);
MT_BOOL DmxHalOQGetOverflowIntStatus(mt_u32 OQId);
mt_void DmxHalOQClearOverflowInt(mt_u32 OQId);
mt_void DmxHalOQEnableOverflowInt(mt_u32 OQId);
mt_void DmxHalOQDisableOverflowInt(mt_u32 OQId);

mt_u32  DmxHalOQGetAllOutputIntStatus(mt_void);
mt_u32  DmxHalOQGetOutputIntStatus(mt_u32 OqRegionId);
mt_void DmxHalOQEnableOutputInt(mt_u32 OQId, MT_BOOL Enable);

mt_void DxmHalSetOQRegMask(mt_u32 u32MaskValue);
mt_u32  DxmHalGetOQRegMask(mt_void);
mt_void DmxHalEnableFQOvflErrInt(mt_void);
mt_void DmxHalDisableFQOvflErrInt(mt_void);
mt_void DmxHalEnableOQOvflErrInt(mt_void);
mt_void DmxHalDisableOQOvflErrInt(mt_void);
mt_void DmxHalEnableFQCfgErrInt(mt_void);
mt_void DmxHalDisableFQCfgErrInt(mt_void);
mt_void DmxHalEnableFQDescErrInt(mt_void);
mt_void DmxHalDisableFQDescErrInt(mt_void);
mt_void DmxHalEnableAllDavInt(mt_void);
mt_void DmxHalEnableOQRecive(mt_u32 u32OQId);
mt_void DmxHalDisableOQRecive(mt_u32 u32OQId);
MT_BOOL DmxHalGetOQEnableStatus(mt_u32 u32OQId);
mt_void DmxHalFQEnableRecive(mt_u32 FQId, MT_BOOL Enable);
mt_s32  DmxHalGetInitStatus(mt_void);
mt_void DmxHalSetFQWORDx(mt_u32 u32FQId, mt_u32 u32Offset, mt_u32 u32Data);
mt_void DmxHalGetFQWORDx(mt_u32 u32FQId, mt_u32 u32Offset, mt_u32 *pu32Data);
mt_void DmxHalSetFQWritePtr(mt_u32 u32FQId, mt_u32 u32WritePtr);
mt_u32  DmxHalGetFQWritePtr(mt_u32 FQId);
mt_u32  DmxHalGetFQReadPtr(mt_u32 FQId);
mt_void DmxHalSetOQWORDx(mt_u32 u32OQId, mt_u32 u32Offset, mt_u32 u32Data);
mt_void DmxHalGetOQWORDx(mt_u32 u32OQId, mt_u32 u32Offset, mt_u32 *pu32Data);
mt_void DmxHalSetOQReadPtr(mt_u32 u32OQId, mt_u32 u32ReadPtr);

mt_void DmxHalAttachIPBPFQ(mt_u32 PortId, mt_u32 FQId);
mt_void DmxHalDetachIPBPFQ(mt_u32 PortId, mt_u32 FQId);

mt_void DmxHalSetRecTsCounter(mt_u32 u32DmxId, mt_u32 u32OqId);
mt_void DmxHalSetRecTsCntReplace(mt_u32 u32DmxId);
mt_u32  DmxHalGetOqCounter(mt_u32 OQId);
mt_void DmxHalResetOqCounter(mt_u32 u32OqId);
mt_u32  DmxHalGetChannelCounter(mt_u32 u32ChId);
mt_void DmxHalResetChannelCounter(mt_u32 u32ChId);

mt_void DmxHalFilterSetSecStuffCtrl(MT_BOOL Enable);
mt_void DmxHalSetTei(mt_u32   u32DemuxID,MT_BOOL bCheckTei);

#ifdef DMX_FILTER_DEPTH_SUPPORT
mt_void DmxHalFilterEnableDepth(mt_void);
mt_void DmxHalFilterSetDepth(mt_u32 FilterId, mt_u32 Depth);
#endif

#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
mt_void DmxHalEnableAllRecExcludePid(mt_u32 DmxID);
mt_void DmxHalDisableAllRecExcludePid(mt_u32 DmxID);
mt_void DmxHalGetAllRecExcludePid(mt_u32 RecCfgID, mt_u32* DmxID, mt_u32* PID);
mt_void DmxHalSetAllRecExcludePid(mt_u32 RecCfgID, mt_u32 DmxID, mt_u32 PID);
#endif


#ifdef DMX_REC_TIME_STAMP_SUPPORT    /*only hi3719 support this */
mt_void DmxHalConfigRecTsTimeStamp(mt_u32 DmxID, DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp);
#endif

#ifdef DMX_TAG_DEAL_SUPPORT
mt_s32 DmxHalSetTagDealCtl(mt_u32 PortId, MT_BOOL bEnable, mt_u32 u32SyncMode, mt_u32 u32TagLen);
mt_void DmxHalSetTagDealAttr(mt_u32 u32TagPortId, mt_u32 u32Low, mt_u32 u32Mid, mt_u32 u32High);
mt_void DmxHalSetDemuxTagPortId(mt_u32 DmxId, mt_u32 u32TagPortId);
#endif

#ifdef DMX_SUPPORT_RAM_CLK_AUTO_CTL
mt_void DmxHalEnableRamClkAutoCtl(mt_void);
#else
inline static mt_void DmxHalEnableRamClkAutoCtl(mt_void) { };
#endif

#ifdef __cplusplus
}
#endif

#endif

