/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */

#ifndef __MT_MPI_DEMUX_H__
#define __MT_MPI_DEMUX_H__

#include "mt_type.h"

#include "mt_unf_demux.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*************************** Struct Definition *********************************/
typedef struct mtMPI_DMX_BUF_STATUS_S
{
    mt_u32 u32BufSize;  /*buffer size */
    mt_u32 u32UsedSize; /* buffer used size *//*CNcomment:缓冲区已使用大小 */
    mt_u32 u32BufRptr;  /*buffer read pointer *//*CNcomment:缓冲区读指针*/
    mt_u32 u32BufWptr;/*buffer written pointer *//*CNcomment:缓冲区写指针*/
} MT_MPI_DMX_BUF_STATUS_S;

typedef struct mtDMX_DATA_S
{
    mt_u8   *pAddr;
    mt_u32   u32PhyAddr;
    mt_u32   u32Len;
} DMX_DATA_S;

typedef enum
{
    DMX_TS_188,
    DMX_TS_192,
    DMX_TS_ES,
    DMX_TS_PES,
    DMX_TS_204,
} dmx_ts_data_t;

/*
  MT_MPI_DMX_DESCRIPTOR_DATA_S   desc_data;
  desc_data.u32Addr = (es_addr + p_audio_priv->aud_es_buf_w_pointer);
  desc_data.u32Pts  = (((high32bitpts&0x3fffffff)<<1)|0x80000000);    //low30
  desc_data.u32Info = ((high32bitpts>>30)<<24);                     //high 2 bit.fixbug110904
  desc_data.u32Dts = 0;
*/                 
typedef struct 
{
    mt_u32 u32Info;
    mt_u32 u32Addr;
    mt_u32 u32Dts;
    mt_u32 u32Pts;
} MT_MPI_DMX_DESCRIPTOR_DATA_S;

/******************************* API declaration *****************************/
mt_u32  MT_MPI_DMX_GetRegister(mt_u32 registerOffset);
mt_void MT_MPI_DMX_SetRegister(mt_u32 registerOffset, mt_u32 value);
mt_void MT_MPI_DMX_DumpAllRegister(void);   //for debug

mt_void MT_MPI_DMX_GetEsBuffAddr(mt_handle hChannel,
						ulong *esBuffAddr,  ulong *esBuffSize,  ulong *kerVirEsBuffAddr,
						ulong *kerVirDescBuffAddr, ulong *descBuffSize);

mt_void MT_MPI_DMX_GetEsBuffAddrEx(mt_handle hChannel,
						ulong *esBuffAddr,  ulong *kerVirEsBuffAddr,  ulong *esBuffSize,
						ulong *descBuffAdrr, ulong *kerVirDescBuffAddr,  ulong *descBuffSize);


mt_void MT_MPI_DMX_GetEsBuffKerVirAddr(mt_handle hChannel, ulong *esBuffKerVirAddr,  mt_u32 *esBuffSize);
mt_void MT_MPI_DMX_GetEsBuffPhyAddr(mt_handle hChannel, ulong *esBuffPhyAddr,  mt_u32 *esBuffSize);
mt_void MT_MPI_DMX_GetDescBuffAddr(mt_handle hChannel,
								phys_addr_t *phyDescBuffAddr,
								ulong *kerVirDescBuffAddr,
								ulong *usrVirDescBuffAddr,
								ulong *descBuffSize);

mt_u32 MT_MPI_DMX_GetAVsync(mt_handle hChannel);

mt_u32  MT_MPI_DMX_GetCurrentEsBufferWritePoint(mt_handle hChannel);
mt_void MT_MPI_DMX_SetCurrentEsBufferReadPoint(mt_handle hChannel, mt_u32 rp);

mt_u32 MT_MPI_DMX_GetCurrentDescBufferWritePoint(mt_handle hChannel);
mt_void MT_MPI_DMX_SetCurrentDescBufferReadPoint(mt_handle hChannel, mt_u32 rp);
mt_u32 MT_MPI_DMX_GetCurrentDescBufferReadPoint(mt_handle hChannel);

mt_u32  MT_MPI_DMX_GetRegristerValue(mt_handle hChannel, mt_u32 offset);
mt_void MT_MPI_DMX_SetRegristerValue(mt_handle hChannel, mt_u32 offset, mt_u32 value);

mt_s32 MT_MPI_DMX_Init(mt_void);
mt_s32 MT_MPI_DMX_DeInit(mt_void);
mt_s32 MT_MPI_DMX_GetCapability(MT_UNF_DMX_CAPABILITY_S *pstCap);

/* Port */
mt_s32 MT_MPI_DMX_GetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_SetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, const MT_UNF_DMX_PORT_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_GetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_SetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, const MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_GetDmxTagAttr(mt_u32 u32DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_SetDmxTagAttr(mt_u32 u32DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr);
mt_s32 MT_MPI_DMX_AttachTSPort(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E enPortId);
mt_s32 MT_MPI_DMX_DetachTSPort(mt_u32 u32DmxId);
mt_s32 MT_MPI_DMX_GetTSPortId(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E *penPortId);
mt_s32 MT_MPI_DMX_GetTSPortPacketNum(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_PACKETNUM_S *sPortStat);


/* TS Buffer */
mt_s32 MT_MPI_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_E enPortId, mt_u32 u32TsBufSize, mt_handle *phTsBuffer);
mt_s32 MT_MPI_DMX_DestroyTSBuffer(mt_handle hTsBuffer);
mt_s32 MT_MPI_DMX_GetTSBuffer(mt_handle hTsBuffer, mt_u32 u32ReqLen,
            MT_UNF_STREAM_BUF_S *pstData, phys_addr_t *pu32PhyAddr, mt_u32 u32TimeOutMs);
mt_s32 MT_MPI_DMX_PutTSBuffer(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos);
mt_s32 MT_MPI_DMX_PutTSBuffer_V1(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos, dmx_ts_data_t ts_type, mt_u32 pid);

/*
**  u32SpecifiedDataAddr == 0; inject data addr is default TSbuffAddr.
**  u32SpecifiedDataAddr != 0; inject data addr is u32SpecifiedDataAddr.
*/
mt_s32 MT_MPI_DMX_PutTSBufferEx(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos, phys_addr_t u32SpecifiedDataPhyAddr);
mt_s32 MT_MPI_DMX_PutTSBufferForEsPes(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos, mt_u32 Pid, dmx_ts_data_t DateType, mt_u32 Pts);
mt_s32 MT_MPI_DMX_ResetTSBuffer(mt_handle hTsBuffer);
mt_s32 MT_MPI_DMX_GetTSBufferStatus(mt_handle hTsBuffer, MT_UNF_DMX_TSBUF_STATUS_S *pStatus);
mt_s32 MT_MPI_DMX_GetTSBufferPortId(mt_handle hTsBuffer, MT_UNF_DMX_PORT_E *penPortId);
mt_s32 MT_MPI_DMX_GetTSBufferHandle(MT_UNF_DMX_PORT_E enPortId, mt_handle *phTsBuffer);
mt_s32 MT_MPI_DMX_GetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S *p_av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S *p_rec_care);
mt_s32 MT_MPI_DMX_SetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S rec_care);

/* Channel */
/*Buffer for video and audio channel should be attached,it's meaningless to config when apply them *//*CNcomment: 音视频通道的buffer需要绑定，在申请时配置没有意义*/
mt_s32 MT_MPI_DMX_GetPortMode(mt_u32 u32DmxId, MT_UNF_DMX_PORT_MODE_E *penPortMod);
mt_s32 MT_MPI_DMX_GetChannelDefaultAttr(MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);
mt_s32 MT_MPI_DMX_CreateChannel(mt_u32 u32DmxId, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr,
            mt_handle *phChannel);
mt_s32 MT_MPI_DMX_DestroyChannel(mt_handle hChannel);
mt_s32 MT_MPI_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);
mt_s32 MT_MPI_DMX_SetChannelAttr(mt_handle hChannel, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);
mt_s32 MT_MPI_DMX_SetChannelPID(mt_handle hChannel, mt_u32 u32Pid);
mt_s32 MT_MPI_DMX_GetChannelPID(mt_handle hChannel, mt_u32 *pu32Pid);
mt_s32 MT_MPI_DMX_OpenChannel(mt_handle hChannel);
mt_s32 MT_MPI_DMX_CloseChannel(mt_handle hChannel);
mt_s32 MT_MPI_DMX_GetChannelStatus(mt_handle hChannel, MT_UNF_DMX_CHAN_STATUS_S *pstStatus);
mt_s32 MT_MPI_DMX_GetChannelHandle(mt_u32 u32DmxId, mt_u32 u32Pid, mt_handle *phChannel);
mt_s32 MT_MPI_DMX_GetChannelHandleByPidType(mt_u32 u32DmxId , mt_u32 u32Pid, MT_UNF_DMX_CHAN_TYPE_E enChannelType, mt_handle *phChannel);
mt_s32 MT_MPI_DMX_GetAVChannelHandle(mt_u32 u32DmxId, mt_handle *videoChannel, mt_handle *audioChannel);  // for Advca
mt_s32 MT_MPI_DMX_GetFreeChannelCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount);
mt_s32 MT_MPI_DMX_GetScrambledFlag(mt_handle hChannel, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag);
mt_s32 MT_MPI_DMX_SetChannelEosFlag(mt_handle hChannel);
mt_s32 MT_MPI_DMX_GetChannelTsCount(mt_handle hChannel, mt_u32 *pu32TsCount);
mt_s32 MT_MPI_DMX_GetAVChaneId(mt_handle hChannel);


/* Filter */
mt_s32 MT_MPI_DMX_CreateFilter(mt_u32   u32DmxId, const MT_UNF_DMX_FILTER_ATTR_S  *pstFilterAttr,
            mt_handle *phFilter);
mt_s32 MT_MPI_DMX_DestroyFilter(mt_handle hFilter);
mt_s32 MT_MPI_DMX_DeleteAllFilter(mt_handle hChannel);
mt_s32 MT_MPI_DMX_SetFilterAttr(mt_handle hFilter, const MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr);
mt_s32 MT_MPI_DMX_GetFilterAttr(mt_handle hFilter, MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr );
mt_s32 MT_MPI_DMX_AttachFilter(mt_handle hFilter, mt_handle hChannel);
mt_s32 MT_MPI_DMX_DetachFilter(mt_handle hFilter, mt_handle hChannel);
mt_s32 MT_MPI_DMX_GetFilterChannelHandle(mt_handle hFilter, mt_handle *phChannel);
mt_s32 MT_MPI_DMX_GetFreeFilterCount(mt_u32 u32DmxId ,  mt_u32 * pu32FreeCount);


/* Data receive */
mt_s32  MT_MPI_DMX_CheckDataHandle(mt_handle hChannel, mt_u32 u32TimeOutMs);
mt_s32  MT_MPI_DMX_GetDataHandle(mt_handle *phChannel, mt_u32 *pu32ChNum,
            mt_u32 u32TimeOutMs);

mt_s32  MT_MPI_DMX_SelectDataHandle(mt_handle *phWatchChannel, mt_u32 u32WatchNum,
            mt_handle *phDataChannel, mt_u32 *pu32ChNum, mt_u32 u32TimeOutMs);

mt_s32  MT_MPI_DMX_AcquireBuf(mt_handle hChannel, mt_u32 u32AcquireNum,
            mt_u32 * pu32AcquiredNum, MT_UNF_DMX_DATA_S *pstBuf,
            mt_u32 u32TimeOutMs);
mt_s32  MT_MPI_DMX_ReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum,
            MT_UNF_DMX_DATA_S *pstBuf);

/* PCR */
mt_s32 MT_MPI_DMX_CreatePcrChannel(mt_u32 u32DmxId, ulong *pu32PcrChId);
mt_s32 MT_MPI_DMX_DestroyPcrChannel(mt_u32 u32PcrChId);
mt_s32 MT_MPI_DMX_PcrPidSet(mt_u32 pu32PcrChId, mt_u32 u32Pid);
mt_s32 MT_MPI_DMX_PcrPidGet(mt_u32 pu32PcrChId, mt_u32 *pu32Pid);
mt_s32 MT_MPI_DMX_PcrScrGet(mt_u32 pu32PcrChId, mt_u64 *pu64PcrMs, mt_u64 *pu64ScrMs);
mt_s32 MT_MPI_DMX_PcrSyncAttach(mt_u32 u32PcrChId, mt_u32 u32SyncHandle);
mt_s32 MT_MPI_DMX_PcrSyncDetach(mt_u32 u32PcrChId);

/*Only video and audio channel are enable to attach PES Buffer *//*CNcomment:   只有音视频通道允许绑定和解绑定PES Buffer*/
mt_s32 MT_MPI_DMX_GetPESBufferStatus(mt_handle hChannel, MT_MPI_DMX_BUF_STATUS_S *pBufStat);

/*Be used to send stream to user status decoder *//*CNcomment:   用于给用户态的音频解码送码流*/
mt_s32 MT_MPI_DMX_AcquireEs(mt_handle hChannel, MT_UNF_ES_BUF_S *pAudioEsBuf);
mt_s32 MT_MPI_DMX_ReleaseEs(mt_handle hChannel,const MT_UNF_ES_BUF_S *pAudioEsBuf);

mt_s32 MT_MPI_DMX_CreateRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn);
mt_s32 MT_MPI_DMX_DestroyRecChn(mt_handle hRecChn);

mt_s32 MT_MPI_DMX_CreateLinkRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn);
mt_s32 MT_MPI_DMX_DestroyLinkRecChn(mt_handle hRecChn);

mt_s32 MT_MPI_DMX_AddRecPid(mt_handle hRecChn, mt_u32 u32Pid, mt_handle *phChannel);
mt_s32 MT_MPI_DMX_DelRecPid(mt_handle hRecChn, mt_handle hChannel);
mt_s32 MT_MPI_DMX_DelAllRecPid(mt_handle hRecChn);

mt_s32 MT_MPI_DMX_AddExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid);
mt_s32 MT_MPI_DMX_DelExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid);
mt_s32 MT_MPI_DMX_DelAllExcludeRecPid(mt_handle hRecChn);

mt_s32 MT_MPI_DMX_StartRecChn(mt_handle hRecChn);
mt_s32 MT_MPI_DMX_StopRecChn(mt_handle hRecChn);

mt_s32 MT_MPI_DMX_AcquireRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs);
mt_s32 MT_MPI_DMX_ReleaseRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData);

mt_s32 MT_MPI_DMX_AcquireLinkRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs);
mt_s32 MT_MPI_DMX_ReleaseLinkRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData);

mt_s32 MT_MPI_DMX_AcquireRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs);
mt_s32 MT_MPI_DMX_AcquireLinkRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs);

mt_s32 MT_MPI_DMX_GetRecBufferStatus(mt_handle hRecChn, MT_UNF_DMX_RECBUF_STATUS_S *pstBufStatus);
mt_s32 MT_MPI_DMX_BufFullCareSet(mt_handle hRecChn, const MT_UNF_DMX_BUF_FULL_CARE_S *pBufFullCare);
mt_s32 MT_MPI_DMX_ChannelIndexResume(mt_handle hChannel);

mt_s32 MT_MPI_DMX_DataPushStart(mt_handle hChannel);
mt_s32 MT_MPI_DMX_DataPushStop(mt_handle hChannel);

mt_s32 MT_MPI_DMX_T2MIEnable(mt_u32 enable);
mt_s32 MT_MPI_DMX_T2MISetInCh(MT_UNF_DMX_PORT_E enPortId);
mt_s32 MT_MPI_DMX_T2MISetOutCh(MT_UNF_DMX_PORT_E enPortId);
mt_s32 MT_MPI_DMX_T2MISetPid(mt_u32 pid);
mt_s32 MT_MPI_DMX_T2MISetPlpid(mt_u32 plpid);
mt_s32 MT_MPI_DMX_T2MISoftReset(void);
mt_s32 MT_MPI_DMX_T2MI_Config(MT_UNF_DMX_T2MI_CONFIG_S mpi_t2mi_para);

mt_s32 MT_MPI_DMX_Soft_Reset(void);
mt_s32 MT_MPI_DMX_HardwareInit(void);

mt_s32 MT_MPI_DMX_SetPortId(mt_handle hChannel, MT_UNF_DMX_PORT_E enPortId);
mt_s32 MT_MPI_DMX_TrickSeekIn(mt_handle hChannel);
mt_s32 MT_MPI_DMX_TrickSeekOut(mt_handle hChannel);

mt_s32 MT_MPI_DMX_FastPlayStart(mt_u8 swtsi_num, mt_u8 rec_id, mt_u8 node_num);
mt_s32 MT_MPI_DMX_FastPlayStop(void);

mt_s32 MT_MPI_DMX_Get_TSI_Clk(MT_UNF_DMX_TSI_CLK_E *sel_clk);
mt_s32 MT_MPI_DMX_Set_TSI_Clk(MT_UNF_DMX_TSI_CLK_E sel_clk);

mt_s32 MT_MPI_DMX_Get_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S *port_ctrl);
mt_s32 MT_MPI_DMX_Set_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S port_ctrl);

mt_s32 MT_MPI_DMX_Get_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E *port_clk);
mt_s32 MT_MPI_DMX_Set_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E port_clk);

mt_s32 MT_MPI_DMX_Get_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E *port_src);
mt_s32 MT_MPI_DMX_Set_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E port_src);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 MT_MPI_DMX_CiplusEnable(mt_handle hChannel, mt_u8 isenable);
mt_s32 MT_MPI_Tsi_Ciplus_RecChanSet(mt_handle recchan);
mt_s32 MT_MPI_Tsi_Ciplus_SwtsiChanSet(mt_handle swtsichan);
mt_s32 MT_MPI_Tsi_Ciplus_BufCfg(mt_u8 usecache);
mt_s32 MT_MPI_Tsi_Ciplus_ClkCfg(mt_u32 clkdiv);
mt_s32 MT_MPI_Tsi_Ciplus_TsIntervalCfg(mt_u32 interval);
mt_s32 MT_MPI_Tsi_Tsi2SourceCfg(mt_u8 from_cam,mt_u8 serial);
mt_s32 MT_MPI_Tsi_LlnNumStartCfg(mt_u32 lln_num);
mt_s32 MT_MPI_Tsi_SwtsiByteorderCfg(mt_u32 islittle);
mt_s32 MT_MPI_Tsi_SwtsiBufFullCfg(mt_handle hChannel,mt_u32 fullcfg);
mt_s32 MT_MPI_Tsi_AhbRdDelayCfg(mt_handle hChannel,mt_u32 delay);
mt_s32 MT_MPI_Tsi_Ciplus_GetStatus(mt_u8 *bufstatus,mt_u8 *cistatus);
mt_s32 MT_MPI_Tsi_Ciplus_EnableCfg(mt_u8 enable);
#endif
#ifndef MT_PTS64
typedef unsigned long long MT_PTS64;
#endif

size_t mpi_demux_desc_get_size(void);
size_t mpi_demux_desc_read(void *buffer, size_t size, size_t count, mt_handle dmx_ch);
mt_s32 mpi_demux_desc_seek(mt_handle dmx_ch, long offset, int fromwhere);
void *mpi_demux_desc_parse(void *buffer, size_t size);
void mpi_demux_desc_destroy(void *desc);
mt_s32 mpi_demux_desc_get_orig_pts(void *desc, MT_PTS64 *pts);
mt_s32 mpi_demux_desc_get_pts64(void *desc, MT_PTS64 *pts);
mt_s32 mpi_demux_desc_get_es_start_addr(void *desc, mt_u32 *addr);
void *mpi_demux_desc_next(mt_handle dmx_ch);

/***********************recoder type***************/
/* Sequence of DMX_IDX_DATA_S 's member can not change,must match the sequence defined by hardware*/
/* CNcomment:DMX_IDX_DATA_S 中各成员的顺序不能改变，必须与硬件规定的顺序保持一致 */
typedef struct mtDMX_IDX_DATA_S
{
    mt_u32 u32Chn_Ovflag_IdxType_Flags;
    mt_u32 u32ScType_Byte12AfterSc_OffsetInTs;
    mt_u32 u32TsCntLo32;
    mt_u32 u32TsCntHi8_Byte345AfterSc;
    mt_u32 u32ScCode_Byte678AfterSc;
    mt_u32 u32SrcClk;
    mt_u32 u32BackPacetNum;/*Back package number*//* CNcomment:回退包计数*/
} DMX_IDX_DATA_S;

mt_s32 MT_MPI_DMX_Invoke(MT_UNF_DMX_INVOKE_TYPE_E enCmd, const mt_void *pCmdPara);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif  // __MT_MPI_DEMUX_H__

