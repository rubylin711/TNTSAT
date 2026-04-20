/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 File Name     : mt_drv_demux.h
 Version       : Initial Draft
 Author        : Montage multimedia software group
 Created       : 2013/04/10
 Description   :
******************************************************************************/

#ifndef __MT_DRV_DEMUX_H__
#define __MT_DRV_DEMUX_H__

#include "mt_type.h"
#include "mt_unf_demux.h"
#include "mt_mpi_demux.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DMX_FLTHANDLE(FilterId)     ((FilterId) | 0x00000200 | (MT_ID_DEMUX << 16))

#define CFG_MT_DEMUX_POOLBUF_SIZE 0x80000

#define DMX_1_PACK_LEN_PTS              64

#define DMX_1_PACK_LEN_NOPTS            56

typedef enum
{
    DMX_PORT_MODE_TUNER = 0,
    DMX_PORT_MODE_RAM   = 1,
    DMX_PORT_MODE_BUTT
} DMX_PORT_MODE_E;

typedef struct
{
    phys_addr_t  u32BufPhyAddr;            /*Physical address of a buffer.*/
    ulong  u32BufKerVirAddr;         /*Kernel virtual address of a buffer.*/
    ulong  u32BufUsrVirAddr;         /*User virtual address of a buffer.*/
    ulong  u32BufSize;               /*Buffer size, in the unit of byte.*/
} DMX_MMZ_BUF_S;

typedef struct
{
    phys_addr_t  BufPhyAddr;
    ulong  BufKerAddr;
    ulong  BufLen;
} DMX_DATA_BUF_S;

typedef struct
{
    ulong  u32BufVirAddr;
    phys_addr_t  u32BufPhyAddr;
    ulong  u32BufLen;
    mt_u32  u32PtsMs;		// 32bit PTS
    mt_u64  u64PtsMs;		// 64bit PTS in unit of microsecond
    mt_u32  u32Index;
    mt_u32  u32DispTime;//add for pvr
    mt_u32  u32DispEnableFlag;
    mt_u32  u32DispFrameDistance;
    mt_u32  u32DistanceBeforeFirstFrame;
    mt_u32  u32GopNum;
    mt_u32  pts_valid;
} DMX_Stream_S;

typedef struct mtDMX_UserMsg_S
{
    mt_u32                  filterid;
    mt_u32                  u32MsgLen;
    phys_addr_t             u32BufStartAddr;
    MT_UNF_DMX_DATA_TYPE_E  enDataType;  /**< the data packet type */
} DMX_UserMsg_S;

typedef struct mtDMX_BUF_STATUS_S
{
    mt_u32 u32BufSize;  /*buffer size */
    mt_u32 u32UsedSize; /* buffer used size */
    mt_u32 u32BufRptr;  /*buffer read pointer */
    mt_u32 u32BufWptr;/*buffer written pointer */
} MT_DRV_DMX_BUF_STATUS_S;

typedef struct
{
    mt_handle               RecHandle;
    MT_UNF_DMX_REC_ATTR_S   RecAttr;
	mt_u8					link_node_num;
    phys_addr_t             RecBufPhyAddr[8];
    mt_u32                  RecBufSize[8];
    phys_addr_t             RecIdxBufPhyAddr;
    mt_u32                  RecIdxBufSize;
} DMX_LinkRec_CreateChan_S;

mt_u32 MT_DRV_DMX_ReadRegister(mt_u32 dmxRegisterOffset);
mt_void MT_DRV_DMX_WriteRegister(mt_u32 dmxRegisterOffset, mt_u32 value);

mt_s32  MT_DRV_DMX_Init(mt_void);
mt_void MT_DRV_DMX_DeInit(mt_void);

mt_s32  MT_DRV_DMX_Open(mt_void);
mt_s32  MT_DRV_DMX_Close(ulong file);

mt_s32  MT_DRV_DMX_GetPoolBufAddr(DMX_MMZ_BUF_S *PoolBuf);
mt_s32  MT_DRV_DMX_GetCapability(MT_UNF_DMX_CAPABILITY_S *Cap);
mt_s32 MT_DRV_DMX_SetPusi(MT_BOOL bCheckPusi);
mt_s32 MT_DRV_DMX_SetTei(MT_UNF_DMX_TEI_SET_S *pstTei);
mt_s32 MT_DRV_DMX_TSIAttachTSO(MT_UNF_DMX_TSI_ATTACH_TSO_S *pstTSIAttachTSO);
MT_BOOL MT_DRV_DMX_IsTSIAttachTSO(mt_u32 PortId, MT_UNF_DMX_TSO_PORT_E* TSO);

mt_s32  MT_DRV_DMX_TSOPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr);
mt_s32  MT_DRV_DMX_TSOPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr);

mt_s32  MT_DRV_DMX_TunerPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr);
mt_s32  MT_DRV_DMX_TunerPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr);

mt_s32  MT_DRV_DMX_RamPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr);
mt_s32  MT_DRV_DMX_RamPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr);

mt_s32  MT_DRV_DMX_GetTagAttr(const mt_u32 DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_s32  MT_DRV_DMX_SetTagAttr(const mt_u32 DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_s32  MT_DRV_DMX_GetTagPortId(mt_u32 DmxId, mt_u32 *TagPortId);


mt_s32  MT_DRV_DMX_AttachTunerPort(mt_u32 DmxId, mt_u32 PortId);
mt_s32  MT_DRV_DMX_AttachRamPort(mt_u32 DmxId, mt_u32 PortId);
mt_s32  MT_DRV_DMX_DetachPort(mt_u32 DmxId);

mt_s32  MT_DRV_DMX_GetPortId(mt_u32 DmxId, DMX_PORT_MODE_E *PortMode, mt_u32 *PortId);

mt_s32  MT_DRV_DMX_TunerPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt, mt_u32 *ErrTsPackCnt);
mt_s32  MT_DRV_DMX_RamPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt);

mt_s32  MT_DRV_DMX_CreateTSBuffer(const mt_u32 PortId, const mt_u32 Size, DMX_MMZ_BUF_S *TsBuf, const ulong file);
mt_s32  MT_DRV_DMX_DestroyTSBuffer(const mt_u32 PortId);
mt_s32  MT_DRV_DMX_GetTSBuffer(const mt_u32 PortId, const mt_u32 ReqLen, DMX_DATA_BUF_S *Data, const mt_u32 TimeoutMs);
mt_s32  MT_DRV_DMX_PutTSBuffer(const mt_u32 PortId, const mt_u32 DataLen, const mt_u32 StartPos, const mt_u16 pid, const dmx_ts_data_t date_type, const mt_u32  pts, mt_u32 SpecifiedDataAddr);
mt_s32  MT_DRV_DMX_ResetTSBuffer(const mt_u32 PortId);
mt_s32  MT_DRV_DMX_GetTSBufferStatus(const mt_u32 PortId, MT_UNF_DMX_TSBUF_STATUS_S *Status);

mt_s32  MT_DRV_DMX_CreateChannel(mt_u32 u32DmxId, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr,
                           mt_handle *phChannel, DMX_MMZ_BUF_S *pstChBuf, DMX_MMZ_BUF_S *pstChDescBuf, ulong file);
mt_s32  MT_DRV_DMX_DestroyChannel(mt_handle hChannel);
mt_s32  MT_DRV_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);
mt_s32  MT_DRV_DMX_SetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);
mt_s32  MT_DRV_DMX_SetChannelPID(mt_handle hChannel, mt_u32 u32Pid);
mt_s32  MT_DRV_DMX_GetChannelPID(mt_handle hChannel, mt_u32 *pu32Pid);
mt_s32  MT_DRV_DMX_OpenChannel(mt_handle hChannel);
mt_s32  MT_DRV_DMX_CloseChannel(mt_handle hChannel);
mt_s32  MT_DRV_DMX_GetChannelId(mt_handle hChannel, mt_u32 *channelId);
mt_s32  MT_DRV_DMX_GetChannelStatus(mt_handle hChannel, MT_UNF_DMX_CHAN_STATUS_S *pstStatus);
//mt_s32  MT_DRV_DMX_GetChannelHandle(mt_u32 u32DmxId, mt_u32 u32Pid, mt_handle *phChannel);
mt_s32 MT_DRV_DMX_GetChannelHandle(mt_u32 DmxId, mt_u32 Pid, MT_UNF_DMX_CHAN_TYPE_E  ChanType, mt_handle *ChanHandle);

mt_s32 MT_DRV_DMX_GetAVChannelHandle(mt_u32 DmxId, mt_handle *vidHandle, mt_handle *auddHandle);  //add for advca
mt_s32  MT_DRV_DMX_GetFreeChannelCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount);
mt_s32  MT_DRV_DMX_GetScrambledFlag(mt_handle hChannel, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag);
mt_s32  MT_DRV_DMX_GetChannelTsCount(mt_handle hChannel, mt_u32* pu32TsCnt);
mt_s32  MT_DRV_DMX_SetChannelEosFlag(mt_handle hChannel);
mt_s32  MT_DRV_DMX_SetChannelCCRepeat(mt_handle hChannel, MT_UNF_DMX_CHAN_CC_REPEAT_SET_S *pstChCCReaptSet);


mt_s32  MT_DRV_DMX_CreateFilter(mt_u32 DmxId, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr, mt_handle *Filter, ulong file);
mt_s32  MT_DRV_DMX_DestroyFilter(mt_handle Filter);
mt_s32  MT_DRV_DMX_DestroyAllFilter(mt_handle Channel);
mt_s32  MT_DRV_DMX_SetFilterAttr(mt_handle Filter, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr);
mt_s32  MT_DRV_DMX_GetFilterAttr(mt_handle Filter, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr);
mt_s32  MT_DRV_DMX_AttachFilter(mt_handle Filter, mt_handle Channel);
mt_s32  MT_DRV_DMX_DetachFilter(mt_handle Filter, mt_handle Channel);
mt_s32  MT_DRV_DMX_GetFilterChannelHandle(mt_handle Filter, mt_handle *Channel);
mt_s32  MT_DRV_DMX_GetFreeFilterCount(mt_u32 DmxId, mt_u32 *FreeCount);

mt_s32 MT_DRV_DMX_CheckDataHandle(mt_handle hChannel, mt_u32 u32TimeOutMs);
mt_s32  MT_DRV_DMX_GetDataHandle(mt_u32 *pu32Flag, mt_u32 u32TimeOutMs);
mt_s32  MT_DRV_DMX_SelectDataHandle(mt_handle *pu32WatchChannel, mt_u32 u32WatchNum, mt_u32 *pu32Flag, mt_u32 u32TimeOutMs);
mt_s32  MT_DRV_DMX_AcquireBuf(mt_handle hChannel, mt_u32 u32AcquireNum,
                           mt_u32 *pu32AcquiredNum, DMX_UserMsg_S *pstBuf,
                           mt_u32 u32TimeOutMs);
mt_s32  MT_DRV_DMX_ReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum, DMX_UserMsg_S *pstBuf);
mt_s32  MT_DRV_DMX_PeekBuf(mt_handle hChannel, mt_u32 u32PeekLen,DMX_UserMsg_S *pstBuf);

mt_s32  MT_DRV_DMX_CreatePcrChannel(const mt_u32 DmxId, mt_u32 *PcrHandle, const ulong file);
mt_s32  MT_DRV_DMX_DestroyPcrChannel(const mt_u32 PcrHandle);
mt_s32  MT_DRV_DMX_PcrPidSet(const mt_u32 PcrHandle, const mt_u32 PcrPid);
mt_s32  MT_DRV_DMX_PcrPidGet(const mt_u32 PcrHandle, mt_u32 *PcrPid);
mt_s32  MT_DRV_DMX_PcrScrGet(const mt_u32 PcrHandle, mt_u64 *PcrValue, mt_u64 *ScrValue);
mt_s32  MT_DRV_DMX_PcrSyncAttach(const mt_u32 PcrHandle, const mt_u32 SyncHandle);
mt_s32  MT_DRV_DMX_PcrSyncDetach(const mt_u32 PcrHandle);

mt_s32  MT_DRV_DMX_AcquireEs(mt_handle hChannel, DMX_Stream_S *pEsBuf);
mt_s32  MT_DRV_DMX_ReleaseEs(mt_handle hChannel, DMX_Stream_S *pEsBuf);
mt_s32  MT_DRV_DMX_GetPESBufferStatus(mt_handle ChanHandle, MT_MPI_DMX_BUF_STATUS_S *BufStatus);

mt_s32  MT_DRV_DMX_CreateRecChn(
        MT_UNF_DMX_REC_ATTR_S  *RecAttr,
        mt_handle              *RecHandle,
        phys_addr_t            *RecBufPhyAddr,
        mt_u32                 *RecBufSize,
        phys_addr_t            *RecIdxBufPhyAddr,
        mt_u32                 *RecIdxBufSize,
        ulong                  file
    );
mt_s32  MT_DRV_DMX_DestroyRecChn(mt_handle RecHandle);
mt_s32  MT_DRV_DMX_AddRecPid(mt_handle RecHandle, mt_u32 Pid, mt_handle *ChanHandle, ulong file);
mt_s32  MT_DRV_DMX_DelRecPid(mt_handle RecHandle, mt_handle ChanHandle);
mt_s32  MT_DRV_DMX_DelAllRecPid(mt_handle RecHandle);
mt_s32  MT_DRV_DMX_GetRecTsCnt(mt_handle RecHandle,mt_u32* TSCnt);
mt_s32  MT_DRV_DMX_AddExcludeRecPid(mt_handle RecHandle, mt_u32 Pid);
mt_s32  MT_DRV_DMX_DelExcludeRecPid(mt_handle RecHandle, mt_u32 Pid);
mt_s32  MT_DRV_DMX_DelAllExcludeRecPid(mt_handle RecHandle);
mt_s32  MT_DRV_DMX_StartRecChn(mt_handle RecHandle);
mt_s32  MT_DRV_DMX_StopRecChn(mt_handle RecHandle);
mt_s32  MT_DRV_DMX_AcquireRecData(mt_handle RecHandle, MT_UNF_DMX_REC_DATA_S *RecData, mt_u32 Timeout);
mt_s32  MT_DRV_DMX_ReleaseRecData(mt_handle RecHandle, const MT_UNF_DMX_REC_DATA_S *RecData);
mt_s32  MT_DRV_DMX_AcquireRecIndex(mt_handle RecHandle, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout);
mt_s32  MT_DRV_DMX_GetRecBufferStatus(mt_handle RecHandle, MT_UNF_DMX_RECBUF_STATUS_S *BufStatus);
mt_s32 MT_DRV_DMX_SetChannelBufFullCare(mt_handle hChannel, MT_UNF_DMX_BUF_FULL_CARE_S *pstChBufFull);
mt_s32 MT_DRV_DMX_CreateLinkRecChn(DMX_LinkRec_CreateChan_S *link_rec_param,	ulong		file);
mt_s32 MT_DRV_DMX_DestroyLinkRecChn(mt_handle RecHandle);
mt_s32 MT_DRV_DMX_AcquireLinkRecData(mt_handle RecHandle, MT_UNF_DMX_REC_DATA_S *RecData, mt_u32 Timeout, mt_u8 *p_node_index);
mt_s32 MT_DRV_DMX_ReleaseLinkRecData(mt_handle RecHandle, const MT_UNF_DMX_REC_DATA_S *RecData);
mt_s32 MT_DRV_DMX_AcquireLinkRecIndex(mt_handle RecHandle, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout);

mt_s32 MT_DRV_DMX_SetPortId(mt_handle hChannel, DMX_PORT_MODE_E PortMode, mt_u32 PortId);

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 MT_DRV_DMX_Channel_Ci_Enable(mt_handle hChannel, mt_u32 enable);
mt_s32 MT_DRV_TSI_CI_Rec_Channel_Cfg(mt_handle hChannel);
mt_s32 MT_DRV_TSI_CI_Swtsi_Channel_Cfg(mt_handle hChannel);
mt_s32 MT_DRV_TSI_CI_Buf_Cfg(mt_u8 usecache);
mt_s32 MT_DRV_TSI_CI_Cicamclk_Cfg(mt_u32 clkdiv);
mt_s32 MT_DRV_TSI_CI_TsInterval_Cfg(mt_u32 tsinterval);
mt_s32 MT_DRV_TSI_Tsi2_Source_Cfg(mt_u8 from_cam,mt_u8 serial);
mt_s32 MT_DRV_TSI_CI_LlnStartNum_Cfg(mt_u32 lln_start_num);
mt_s32 MT_DRV_TSI_CI_SwtsiByteorder_Cfg(mt_u32 byteorder);
mt_s32 MT_DRV_DMX_Channel_Swtsi_Full_Cfg(mt_handle hChannel, mt_u32 full);
mt_s32 MT_DRV_TSI_CI_AhbRdDelay_Cfg(mt_u32 rddelay);
mt_s32 MT_DRV_TSI_CI_GetStatus(mt_u8 *bufstatus,mt_u8 *cistaus);
mt_s32 MT_DRV_TSI_CI_Enable_Cfg(mt_u8 enable);
#endif

/*add end*/

#ifdef __cplusplus
}
#endif

#endif  // __MT_DRV_DEMUX_H__

