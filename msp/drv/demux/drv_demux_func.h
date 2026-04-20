/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_DEMUX_FUNC_H__
#define __DRV_DEMUX_FUNC_H__

#include "mt_drv_dev.h"
#include "drv_demux_define.h"
#include "mt_unf_demux.h"
#include "mt_mpi_demux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define TSI_TRPP_PES_CHNUM                        16
//#define TSI_TRPP_REC_CHNUM                        4
//#define TSI_TRPP_INDEX_CHNUM                      8
#define TSI_DS_CHNUM                              16
#define TSI_PTI_SLOT_CHNUM                        128
#define TSI_SF_FILT_CHNUM                         128
#define TSI_SF_FUNIT_NUM                          256 

#define TRPP_REC_INDEX_BUFFER_SIZE                4096*100

#define DMX_SYMPHONY_STREAM_ID_VIDEO              0xa0
#define DMX_SYMPHONY_STREAM_ID_MASK_VIDEO         0x5f
#define DMX_SYMPHONY_STREAM_ID_AUDIO              0x80
#define DMX_SYMPHONY_STREAM_ID_MASK_AUDIO         0x7f
#define DMX_SYMPHONY_STREAM_ID_VBI                0xbd
#define DMX_SYMPHONY_STREAM_ID_MASK_VBI           0x0
#define DMX_SYMPHONY_STREAM_ID_MASK_HW_PES        (0xff)  //hw pes should filter all pes packet
#define DMX_PROC_CH_MAX                           32

#undef LOG_TAG
#define LOG_TAG         "DMX"
#define DMX_AUDIO_PTS_INFO      (0x00000008)

extern mt_u32 g_dmx_loglevel;

#define DMX_LOGAP(fmt, ...)         do{if (g_dmx_loglevel & DMX_AUDIO_PTS_INFO){printk("["LOG_TAG"]" fmt, ## __VA_ARGS__);}}while(0)


/*!
  live process type
  */
typedef enum dmx_slot_process_type_e
{
  /*!
    TS
  */
  DRV_DMX_SLOT_PROCESS_TS = 0x01,
  /*!
    PES/ES
  */
  DRV_DMX_SLOT_PROCESS_PESES = 0x02,
  /*!
    section
  */
  DRV_DMX_SLOT_PROCESS_SECTION = 0x04,
  /*!
    record
  */
  DRV_DMX_SLOT_PROCESS_RECORD = 0x08,
}dmx_slot_process_type_t;

/*!
  Type of Program
  */
typedef enum
{
    /*!
      Video data
      */
    DMX_VIDEO_TYPE,
    /*!
      Audio data
      */
    DMX_AUDIO_TYPE,
    /*!
      PCR data
      */
    DMX_PCR_TYPE,
    /*!
      VBI data
      */
    DMX_VBI_TYPE
} dmx_media_t;

typedef struct {
    mt_u32 ch_index; /* channel index */
    mt_u32 selected; /* 0: the data of this channel should be discard, 1: the data of this channel should be send to decoder */
} demux_channel;

typedef struct {
    mt_u32 main_chan_num;
    mt_s32 main_select_index;
    mt_u32 main_chan_id[DMX_PROC_CH_MAX];   // Main channel
    mt_u32 desc_chan_id;                    // AD channel
    mt_s32 main_pre_index;                  //pre channel
    mt_u32 cur_play_vpts;
	
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	mt_handle halInBuf;
	mt_handle halPktQue;

	mt_handle halFrameQue[2];
#endif

    struct semaphore aud_track_sem;
    mt_u32 trick_seek_to_play;
    struct semaphore trick_seek_sem;
} demux_aud_ch_list;

typedef struct {
    struct clk *tsiclk;
    demux_aud_ch_list aud_proc_list;
    atomic_t atmOpenCnt;                 /* Open times */
} demux_priv_data;

mt_s32  DMX_OsiInit(mt_u32 PoolBufSize, mt_u32 BlockSize);
mt_s32  DMX_OsiDeInit(mt_void);
mt_s32  DMX_OsiGetPoolBufAddr(mt_u32 *VirAddr, phys_addr_t *PhyAddr, mt_u32 *BufSize);
mt_void DMX_OsiSetNoPusiEn(MT_BOOL bNoPusiEn);
mt_void DMX_OsiSetTei(mt_u32 u32DmxId,MT_BOOL bTei);
mt_void DMX_OsiTSIAttashTSO(mt_u32 TunerPortID,MT_UNF_DMX_TSO_PORT_E TSO);
MT_BOOL DMX_OsiIsTSIAttachTSO(mt_u32 PortId, MT_UNF_DMX_TSO_PORT_E* TSO);

/* Port */
mt_s32 DMX_OsiTSOPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr);
mt_s32 DMX_OsiTSOPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr);

mt_s32  DMX_OsiTunerPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr);
mt_s32  DMX_OsiTunerPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr);
mt_s32  DMX_OsiRamPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr);
mt_s32  DMX_OsiRamPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr);

mt_s32  DMX_OsiTunerPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt, mt_u32 *ErrTsPackCnt);
mt_s32  DMX_OsiRamPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt);

mt_s32  DMX_OsiTsBufferCreate(const mt_u32 PortId, const mt_u32 Size, DMX_MMZ_BUF_S *TsBuf);
mt_s32  DMX_OsiTsBufferDestroy(const mt_u32 PortId);
mt_s32  DMX_OsiTsBufferGet(const mt_u32 PortId, const mt_u32 ReqLen, DMX_DATA_BUF_S *Buf, mt_u32 Timeout);
mt_s32  DMX_OsiTsBufferPut(const mt_u32 PortId, const mt_u32 DataLen, const mt_u32 StartPos, const mt_u16 pid, const dmx_ts_data_t date_type, const mt_u32  pts, mt_u32 data_addr);
mt_s32  DMX_OsiTsBufferReset(const mt_u32 PortId);
mt_s32  DMX_OsiTsBufferGetStatus(const mt_u32 PortId, MT_UNF_DMX_TSBUF_STATUS_S *Status);
mt_s32  DMX_OsiGetTsBufferFullCare(mt_u32 port_id, MT_UNF_DMX_AV_CHN_FULL_CARE_S *p_av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S *p_rec_care);
mt_s32  DMX_OsiSetTsBufferFullCare(mt_u32 port_id, MT_UNF_DMX_AV_CHN_FULL_CARE_S av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S rec_care);

/*SubDev*/
mt_s32  DMX_OsiAttachPort(const mt_u32 DmxId, const DMX_PORT_MODE_E PortMode, const mt_u32 PortId);
mt_s32  DMX_OsiDetachPort(const mt_u32 DmxId);
mt_s32  DMX_OsiGetPortId(const mt_u32 DmxId, DMX_PORT_MODE_E *PortMode, mt_u32 *PortId);

/* Filter */
mt_s32  DMX_OsiNewFilter(const mt_u32 DmxId, mt_u32 *FilterId);
mt_s32  DMX_OsiDeleteFilter(const mt_u32 FilterId);
mt_s32  DMX_OsiSetFilterAttr(const mt_u32 FilterId, const MT_UNF_DMX_FILTER_ATTR_S *FilterAttr);
mt_s32  DMX_OsiGetFilterAttr(const mt_u32 FilterId, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr);
mt_s32  DMX_OsiAttachFilter(const mt_u32 FilterId, const mt_u32 ChanId);
mt_s32  DMX_OsiDetachFilter(const mt_u32 FilterId, const mt_u32 ChanId);
mt_s32  DMX_OsiGetFilterChannel(const mt_u32 FilterId, mt_u32 *ChanId);
mt_s32  DMX_OsiGetFreeFilterNum(const mt_u32 DmxId, mt_u32 *FreeCount);

// Channel
mt_s32  DMX_OsiCreateChannel(mt_u32 DmxId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr, DMX_MMZ_BUF_S *ChanBuf, DMX_MMZ_BUF_S *ChanDescBuf, mt_u32 *ChanId);
mt_s32  DMX_OsiDestroyChannel(mt_u32 ChanId);
mt_s32  DMX_OsiOpenChannel(mt_u32 ChanId);
mt_s32  DMX_OsiCloseChannel(mt_u32 ChanId);
mt_s32  DMX_OsiGetChannelAttr(mt_u32 ChanId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr);
mt_s32  DMX_OsiSetChannelAttr(mt_u32 ChanId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr);
mt_s32  DMX_OsiGetChannelPid(mt_u32 ChanId, mt_u32 *Pid);
mt_s32  DMX_OsiSetChannelPid(mt_u32 ChanId, mt_u32 Pid);
mt_s32  DMX_OsiGetChannelStatus(mt_u32 ChanId, MT_UNF_DMX_CHAN_STATUS_E *ChanStatus);
mt_s32  DMX_OsiGetFreeChannelNum(mt_u32 u32DmxId, mt_u32 *pu32FreeCount);
mt_s32  DMX_OsiGetChannelScrambleFlag(mt_u32 u32ChannelId, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag);
mt_s32  DMX_OsiSetChannelEosFlag(mt_u32 ChanId);
mt_s32  DMX_OsiGetChnDataFlag(mt_u32 u32ChanId);
mt_s32  DMX_OsiCheckDataFlag(mt_u32 u32ChId, mt_u32 u32TimeOutMs);
#ifdef DMX_USE_ECM
mt_s32  DMX_OsiGetChannelSwFlag(mt_u32 u32ChannelId, mt_u32* pu32SwFlag);
mt_s32  DMX_OsiGetChannelSwBufAddr(mt_u32 u32ChannelId, mmz_buffer_s* pstSwBuf);
#endif
mt_s32  DMX_OsiResetChannel(mt_u32 u32ChId, DMX_FLUSH_TYPE_E eFlushType);
mt_s32  DMX_OsiGetChannelTsCnt(mt_u32 u32ChannelId, mt_u32* pu32TsCnt);
mt_s32  DMX_OsiSetChannelCCRepeat(mt_u32 ChanId, MT_UNF_DMX_CHAN_CC_REPEAT_SET_S * pstChCCReaptSet);


/* get channel ID by pid, return MT_ERR_DMX_NOMATCH_CHN when failed to find it */
//mt_s32  DMX_OsiGetChannelId(mt_u32 u32DmxId, mt_u32 u32Pid, mt_u32 *pu32ChannelId);
mt_s32  DMX_OsiGetChannelId(mt_u32 DmxId, mt_u32 Pid, MT_UNF_DMX_CHAN_TYPE_E  ChanType, mt_u32 *ChanId);

mt_s32  DMX_OsiGetAVChannelId(mt_u32 u32DmxId, mt_u32 *vChannelId, mt_u32 *aChannelId);
mt_s32  DMX_OsiSelectDataFlag(mt_u32 *pu32WatchCh, mt_u32 u32WatchNum, mt_u32 *pu32Flag, mt_u32 u32TimeOutMs);

mt_s32  DMX_OsiReadDataRequest(mt_u32 u32ChId, mt_u32 u32AcqNum,
                               mt_u32 *pu32AcqedNum, DMX_UserMsg_S* psMsgList, mt_u32 u32TimeOutMs);
mt_s32  DMX_OsiReleaseReadData(mt_u32 u32ChId, mt_u32 u32RelNum, DMX_UserMsg_S* psMsgList);

mt_s32  DMX_OsiReadEsRequest(mt_u32 ChanId, DMX_Stream_S *EsData);
mt_s32  DMX_OsiReadEsRequestByPtsForAudTrack(mt_u32 ChanId, DMX_Stream_S *EsData, mt_u32 pts);
mt_s32  DMX_OsiReadEsRequestByPtsForTrickSeek(mt_u32 ChanId, DMX_Stream_S *EsData, mt_u32 pts);
mt_s32  DMX_OsiReleaseReadEs(mt_u32 ChanId, DMX_Stream_S *EsData);

mt_s32  DMX_OsiGetChanBufStatus(mt_u32 ChanId, MT_MPI_DMX_BUF_STATUS_S *BufStatus);

/* get whether all the channel(96 channenels) channel data exist or not, returned by three u32 flag
    per-bit present one channel, just only return common channel, except the audio-video channel
 */
mt_s32  DMX_OsiGetAllDataFlag(mt_u32 *pu32Flag, mt_u32 u32TimeOutMs);

/* PCR */
mt_s32  DMX_OsiPcrChannelCreate(const mt_u32 DmxId, mt_u32 *PcrId);
mt_s32  DMX_OsiPcrChannelDestroy(const mt_u32 PcrId);
mt_s32  DMX_OsiPcrChannelSetPid(const mt_u32 PcrId, const mt_u32 PcrPid);
mt_s32  DMX_OsiPcrChannelGetPid(const mt_u32 PcrId, mt_u32 *PcrPid);
mt_s32  DMX_OsiPcrChannelGetClock(const mt_u32 PcrId, mt_u64 *PcrValue, mt_u64 *ScrValue);
mt_s32  DMX_OsiPcrChannelAttachSync(const mt_u32 PcrId, const mt_u32 SyncHadle);
mt_s32  DMX_OsiPcrChannelDetachSync(const mt_u32 PcrId);

mt_s32  DMX_DRV_REC_CreateChannel(MT_UNF_DMX_REC_ATTR_S *RecAttr, DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp,mt_u32 *RecId, phys_addr_t *BufPhyAddr, mt_u32 *BufSize, phys_addr_t *IdxBufPhyAddr, mt_u32 *IdxBufSize);
mt_s32  DMX_DRV_REC_DestroyChannel(mt_u32 RecId);
mt_s32  DMX_DRV_REC_AddRecPid(mt_u32 RecId, mt_u32 Pid, mt_u32 *ChanId);
mt_s32  DMX_DRV_REC_DelRecPid(mt_u32 RecId, mt_u32 ChanId);
mt_s32  DMX_DRV_REC_DelAllRecPid(mt_u32 RecId);
mt_s32  DMX_DRV_REC_AddExcludeRecPid(mt_u32 RecId, mt_u32 Pid);
mt_s32  DMX_DRV_REC_DelExcludeRecPid(mt_u32 RecId, mt_u32 Pid);
mt_s32  DMX_DRV_REC_DelAllExcludeRecPid(mt_u32 RecId);
mt_s32  DMX_DRV_REC_GetTsCnt(mt_u32 RecId,mt_u32* TSCnt);
mt_s32  DMX_DRV_REC_StartRecChn(mt_u32 RecId);
mt_s32  DMX_DRV_REC_StopRecChn(mt_u32 RecId);
mt_s32  DMX_DRV_REC_AcquireRecData(mt_u32 RecId, phys_addr_t *PhyAddr, ulong *KerAddr, mt_u32 *Len, mt_u32 Timeout);
mt_s32  DMX_DRV_REC_ReleaseRecData(mt_u32 RecId, phys_addr_t PhyAddr, mt_u32 Len);
mt_s32  DMX_DRV_REC_AcquireRecIndex(mt_u32 RecId, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout);
mt_s32  DMX_DRV_REC_GetRecBufferStatus(mt_u32 RecId, MT_UNF_DMX_RECBUF_STATUS_S *BufStatus);

mt_s32 DMX_OsiPeekDataRequest(mt_u32 u32ChId, mt_u32 u32PeekLen, DMX_UserMsg_S* psMsgList);
mt_s32 DMX_OsiSetChannelBufFullCare(mt_u32 ChanId, MT_UNF_DMX_BUF_FULL_CARE_S *ChanBufFull);
mt_s32 DMX_OsiClearEsBuf(mt_u8 index, mt_u32 data);
mt_s32 DMX_OsiClearDscrptBuf(mt_u8 index, mt_u32 data);
mt_s32 DMX_OsiClearRecBuf(mt_u8 index, mt_u32 data);
mt_s32 DMX_OsiClearIndexBuf(mt_u8 index, mt_u32 data);
mt_s32 DMX_OsiParsePes(mt_u32 ChanId);

mt_s32  DMX_OsiDeviceInit(mt_u32 PoolBufSize, mt_u32 BlockSize);
mt_void DMX_OsiDeviceDeInit(mt_void);
mt_u32  DMX_OsiGetChType(mt_u32 u32ChId);

mt_s32  DMX_OsiSuspend(basedev_s *himd, pm_message_t state);
mt_s32  DMX_OsiResume(basedev_s *himd);

#ifdef DMX_TAG_DEAL_SUPPORT
mt_s32  DMX_OsiGetTagAttr(const mt_u32 DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_s32  DMX_OsiSetTagAttr(const mt_u32 DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_s32 DMX_OsiGetTagPortId(const mt_u32 DmxId, mt_u32 *TagPortId);
mt_s32 DMX_OsiGetTagPortAttr(const mt_u32 TagPortId, MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_void DMX_OsiResumeTag(mt_void);
#endif

//#define DMX_DBG_FPGA_TEST       1
mt_s32 DMX_OsiReadRegister(mt_u32 dmxRegisterOffset);
mt_void DMX_OsiWriteRegister(mt_u32 dmxRegisterOffset, mt_u32 value);

mt_s32 DMX_GetChanRef(mt_u32 ChannelId);
mt_void DMX_OsiT2miSoftReset(mt_void);
mt_s32 DMX_GetChanAVId(mt_u32 ChannelId);

mt_s32 DMX_OsiSoftReset(mt_void);
mt_void DMX_OsiDeviceHardInit(mt_void);

mt_s32 DMX_OsiSetPortId(mt_u32 ChanId, const DMX_PORT_MODE_E PortMode, const mt_u32 PortId);

mt_s32 DMX_DRV_LinkREC_CreateChannel(MT_UNF_DMX_REC_ATTR_S *RecAttr, DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp, mt_u32 *RecId, DMX_LinkRec_CreateChan_S *link_rec_param);
mt_s32 DMX_DRV_LinkREC_DestroyChannel(mt_u32 RecId);
mt_s32 DMX_DRV_LinkREC_AcquireRecData(mt_u32 RecId, phys_addr_t *PhyAddr, ulong *KerAddr, mt_u32 *Len, mt_u32 Timeout, mt_u8 *p_node_index);
mt_s32 DMX_DRV_LinkREC_ReleaseRecData(mt_u32 RecId, phys_addr_t PhyAddr, mt_u32 Len);
mt_s32 DMX_DRV_LinkREC_AcquireRecIndex(mt_u32 RecId, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout);
mt_s32 DMX_DRV_FastPlay_Start(mt_u8 swtsi_num, mt_u8 rec_id, mt_u8 node_num);
mt_s32 DMX_DRV_FastPlay_Stop(mt_void);

mt_s32 DMX_OsiSetTsiClk(MT_UNF_DMX_TSI_CLK_E value);
mt_s32 DMX_OsiGetTsiClk(MT_UNF_DMX_TSI_CLK_E *value);

mt_s32 DMX_OsiSetPortCtrl(mt_u32 port, mt_u32 value);
mt_s32 DMX_OsiGetPortCtrl(mt_u32 port, mt_u32 *value);

mt_s32 DMX_OsiSetPortClk(mt_u32 port, MT_UNF_DMX_PORT_CLK_SEL_E value);
mt_s32 DMX_OsiGetPortClk(mt_u32 port, MT_UNF_DMX_PORT_CLK_SEL_E *value);

mt_s32 DMX_OsiSetPortSrc(mt_u32 port, MT_UNF_DMX_PORT_SRC_SEL_E value);
mt_s32 DMX_OsiGetPortSrc(mt_u32 port, MT_UNF_DMX_PORT_SRC_SEL_E *value);

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 DMX_OsiChannelCiEnable(mt_u32 ChanId, mt_u32 enable);
mt_s32 DMX_OsiCiRecChanCfg(mt_u32 rec_chanid);
mt_s32 DMX_OsiCiSwtsiChanCfg(mt_u32 swtsi_chanid);
mt_s32 DMX_OsiCiBufCfg(mt_u8 usecache);
mt_s32 DMX_OsiCiCamClkCfg(mt_u32 clkdiv);
mt_s32 DMX_OsiCiTsIntervalCfg(mt_u32 tsinterval);
mt_s32 DMX_OsiTsi2SourceCfg(mt_u8 from_cam,mt_u8 serial);
mt_s32 DMX_OsiCiLlnNumStartCfg(mt_u32 lln_num_start);
mt_s32 DMX_OsiSwtsiByteorderCfg(mt_u32 byteorder);
mt_s32 DMX_OsiSwtsiFullcareCfg(mt_u32 ChanId, mt_u32 full);
mt_s32 DMX_OsiCiAhbDelayCfg(mt_u32 ahb_rd_delay);
mt_s32 DMX_OsiGetCiStatus(mt_u8 *bufstatus,mt_u8 *cistatus);
mt_s32 DMX_OsiCiEnableCfg(mt_u8 enable);


#endif

/*add end*/

#ifdef MT_DEMUX_PROC_SUPPORT

typedef struct
{
    mt_u32 ReleaseErrCount;
    mt_u32 ReleaseErrLastChan;
    mt_u32 ReleaseErrLastOQId;
    /*PES error*/
    mt_u32 PesErrCount;
    MT_U64 PesErrLastPCR;

    /*poolbuf err, may be caused by multi-process */
    mt_u32 ResetPoolbufCount;
} DMX_Proc_Err_Info_S;


typedef struct
{
    mt_s32              MaxRef;
    mt_s32              Ref;
    DMX_Proc_Err_Info_S ErrInfo;
    /*some other infor can be added ,such as ,clk ,version and so on*/
} DMX_Proc_Global_Info_S;


typedef struct
{
    phys_addr_t  PhyAddr;
    mt_u32  BufSize;
    mt_u32  UsedSize;
    mt_u32  Read;
    mt_u32  Write;
    mt_u32  GetCount;
    mt_u32  GetValidCount;
    mt_u32  PutCount;
} DMX_Proc_RamPort_BufInfo_S;

typedef struct
{
    mt_u32  DescDepth;
    mt_u32  DescRead;
    mt_u32  DescWrite;
    mt_u32  BlockSize;
    mt_u32  Overflow;
    mt_u32  DataRead;
    mt_u32  DataWrite;
    mt_u32  SoftDataRead;
    mt_u32  SoftDescRead;
    mt_u32  DescSize;
    mt_u32  DescOverflow;
} DMX_Proc_ChanBuf_S;

typedef struct
{
    MT_UNF_DMX_REC_TYPE_E   RecType;
    MT_BOOL                 Descramed;
    mt_u32                  BlockCnt;
    mt_u32                  BlockSize;
    mt_u32                  BufRead;
    mt_u32                  BufWrite;
    mt_u32                  RecStatus;
    mt_u32                  Overflow;
    mt_u32                  IndexOverflow;
    mt_u32                  BufSize;
    mt_u32                  IndexSize;
    mt_u32                  IndexRead;
    mt_u32                  IndexWrite;
} DMX_Proc_Rec_BufInfo_S;

typedef struct
{
    MT_UNF_DMX_REC_INDEX_TYPE_E IndexType;
    mt_u32                      IndexPid;
    mt_u32                      BlockCnt;
    mt_u32                      BlockSize;
    mt_u32                      BufRead;
    mt_u32                      BufWrite;
    mt_u32                  Overflow;
} DMX_Proc_RecScd_BufInfo_S;


#define SLOT_REG_COUNT                 (128)
#define CHANNEL_REG_COUNT              (16)
#define REC_CHANNEL_REG_COUNT          (4)

typedef struct
{
    mt_u32  cw_op;
    mt_u32  tscfg[CHANNEL_REG_COUNT];
    mt_u32  core[CHANNEL_REG_COUNT];
    mt_u32  ive[CHANNEL_REG_COUNT];
    mt_u32  mode[CHANNEL_REG_COUNT];
    mt_u32  pktmode[CHANNEL_REG_COUNT];
}dmx_ds_reg_s;

typedef struct
{
    mt_u32  staddr[SLOT_REG_COUNT];
    mt_u32  size[SLOT_REG_COUNT];
	mt_u32  disc_wptr[SLOT_REG_COUNT];
    mt_u32  int_cfg[SLOT_REG_COUNT];
    mt_u32  filter_config[SLOT_REG_COUNT];
    mt_u32  filter_data[SLOT_REG_COUNT];
    mt_u32  filter_mask[SLOT_REG_COUNT];
    mt_u32  filter_mode[SLOT_REG_COUNT];
}dmx_sf_reg_s;

typedef struct
{
    mt_u32  trpp_global[10];
    mt_u32  ch_property[CHANNEL_REG_COUNT];
    mt_u32  ch_parse_set[CHANNEL_REG_COUNT];
    mt_u32  ch_start_code1[CHANNEL_REG_COUNT];
    mt_u32  ch_frm_start_code_m1[CHANNEL_REG_COUNT];
    mt_u32  ch_start_code2[CHANNEL_REG_COUNT];
    mt_u32  ch_frm_start_code_m2[CHANNEL_REG_COUNT];
    mt_u32  ch_dscrpt_start_addr[CHANNEL_REG_COUNT];
    mt_u32  ch_data_start_addr[CHANNEL_REG_COUNT];
    mt_u32  ch_dscrpt_end_addr[CHANNEL_REG_COUNT];
    mt_u32  ch_data_end_addr[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info1[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info2[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info3[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info4[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info5[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info6[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info7[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info8[CHANNEL_REG_COUNT];
    mt_u32  ch1_ini_info9[CHANNEL_REG_COUNT];
    mt_u32  ch_rec_set[REC_CHANNEL_REG_COUNT];
    mt_u32  rec_start_addr[REC_CHANNEL_REG_COUNT];
    mt_u32  rec_end_addr[REC_CHANNEL_REG_COUNT];
}dmx_trpp_reg_s;
typedef struct
{
	mt_u32 sample_int_mask;
    mt_u32 sample_int_edge;
	mt_u32 sample_int_clr;

	mt_u32 swtsi_int_mask;
    mt_u32 swtsi_int_edge;
	mt_u32 swtsi_int_clr;

	mt_u32 ds_int_mask;
    mt_u32 ds_int_edge;
	mt_u32 ds_int_clr;

	mt_u32 trpp_int_mask[CHANNEL_REG_COUNT];
    mt_u32 trpp_int_edge[CHANNEL_REG_COUNT];
	mt_u32 trpp_int_clr[CHANNEL_REG_COUNT];
	
    mt_u32 pvr_int_mask;
    mt_u32 pvr_int_edge;
	mt_u32 pvr_int_clr;

	mt_u32 gglb_int_mask;
    mt_u32 gglb_int_edge;
	mt_u32 gglb_int_clr;
}dmx_global_reg_s;

typedef struct
{
    u32  tsi_in[4];
    u32  tsi_clksel_reg;
    u32  slotn_cfg0[SLOT_REG_COUNT];
    u32  slotn_cfg1[SLOT_REG_COUNT];
    dmx_ds_reg_s  		ds;
    dmx_sf_reg_s  		sf;
    dmx_trpp_reg_s  	trpp;
	dmx_global_reg_s	global; 
}dmx_all_reg_s;


mt_s32  DMX_OsiRamPortGetBufInfo(mt_u32 PortId, DMX_Proc_RamPort_BufInfo_S *BufInfo);
mt_s32 DMX_OsiRamPortGetDescInfo(mt_u32 PortId, DMX_Proc_RamPort_DescInfo_S *DescInfo);
mt_s32 DMX_OsiRamPortGetBPStatus(mt_u32 PortId, DMX_Proc_RamPort_BPStatus_S *BPStatus);
mt_void DMX_OsiGetFQInfo(mt_u32 FQId, FQ_HeaderInfor_t *FQInfo);
mt_void DMX_OsiGetOQInfo(mt_u32 OQId, OQ_HeaderInfor_t *OQInfo);

mt_s32  DMX_OsiGetChanBufProc(mt_u32 ChanId, DMX_Proc_ChanBuf_S *BufInfo);
mt_s32 DMX_OsiGetFQBufStatus(mt_u32 FQId, MT_MPI_DMX_BUF_STATUS_S *pBufStat);
mt_void DMX_OsiGetChannelDataFlow(mt_u32 ChannelId, ChannelDataFlow_info_t *ChannelDF);

mt_s32  DMX_OsiGetDmxRecProc(mt_u32 RecId, DMX_Proc_Rec_BufInfo_S *RecBufInfo);
mt_s32  DMX_OsiGetDmxRecScdProc(mt_u32 RecId, DMX_Proc_RecScd_BufInfo_S *ScdBufInfo);
DMX_ChanEsBuff_S*  DMX_OsiGetChannelEsBufProc(mt_u32 ChanId);
DMX_ChanInfo_S*     DMX_OsiGetChannelProc(mt_u32 ChanId);
DMX_FilterInfo_S*   DMX_OsiGetFilterProc(mt_u32 FilterId);
DMX_PCR_Info_S*     DMX_OsiGetPcrChannelProc(const mt_u32 PcrId);
DMX_RamPort_Info_S *DMX_OsiGetSwTsiProc(const mt_u32 SwtsiId);
DMX_Sub_DevInfo_S *DMX_OsiGetTsiPortProc(const mt_u32 DmxId);

mt_s32  DMX_OsiSaveDmxTs_Start(mt_u32 u32DmxId, mt_u32 u32RecDmxId);
mt_s32  DMX_OsiSaveDmxTs_Stop(mt_u32 u32RecDmx);

#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __DRV_DEMUX_FUNC_H__

