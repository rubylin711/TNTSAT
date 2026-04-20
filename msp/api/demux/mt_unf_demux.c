/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
#include <stdio.h>
#include <string.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module_debug.h"
//#include "demux_debug.h"
#include "mt_mpi_demux.h"
#include "mt_unf_demux.h"


#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
#include <pthread.h>
static pthread_mutex_t SamePidMutiChan_Lock = PTHREAD_MUTEX_INITIALIZER;

#define DMX_CHANNEL_CNT         128
#define DMX_FILTER_CNT          64
#define DMX_CACHE_SECTIONS      128

enum
{
    CHAN_INVALID=0,
    CHAN_VALID,
};
enum
{
    CHANDELED=0,
    CHANCREATE,
    CHANOPEN,
    CHANSTOP
};
typedef struct UNF_FILTERINFOR
{
    mt_u8 linkvchannel;
    mt_u8 linkvchannel1;
    mt_u8 linkvchannel2;
    mt_u8 linkvchannel3;
}MT_UNF_FILTERINFOR;

typedef struct UNF_CHANNELINFOR
{
    mt_u32 dmxid;
    MT_UNF_DMX_CHAN_ATTR_S tmp;
    mt_u8 flag_using;           //invalid or valid
    mt_u8 flag_status;          //create\open\stop.1\2\3
    mt_u8 flag_main;            //main channel
    mt_u8 flag_using3;
    mt_handle true_chanhd;      //true channel id
    mt_u32 u32Pid;              //channel pid
    MT_UNF_DMX_DATA_S datas[DMX_CACHE_SECTIONS];
    mt_u32 datanum;
}MT_UNF_CHANNELINFOR;
static MT_UNF_CHANNELINFOR g_channel_infor[DMX_CHANNEL_CNT]={0};
static MT_UNF_FILTERINFOR  g_filter_infor[DMX_FILTER_CNT]={{0xff}};

static mt_u8 mt_getavchannel(void)
{
    mt_s32 i=0;
    for(i=0;i<DMX_CHANNEL_CNT;i++){
        if(((mt_u8)CHAN_INVALID)==g_channel_infor[i].flag_using){
            return i;
        }
    }
    return 0xff;
}

static mt_s32 mt_findsametruehd(mt_handle true_chanhd)
{
    mt_s32 i=0;
    for(i=0;i<DMX_CHANNEL_CNT;i++){
        if((((mt_u8)CHAN_VALID)==g_channel_infor[i].flag_using) && 
           (true_chanhd==g_channel_infor[i].true_chanhd)){
            return MT_TRUE;
        }
    }
    return MT_FALSE;
}

static mt_u8 mt_findsamepid(mt_u32 pid)
{
    mt_s32 i=0;
    for(i=0;i<DMX_CHANNEL_CNT;i++){
        if((((mt_u8)CHAN_VALID)==g_channel_infor[i].flag_using) && 
           (pid==g_channel_infor[i].u32Pid)){
            return i;
        }
    }
    return 0xff;
}

static mt_s32 mt_checkanyopenchan(mt_handle true_chanhd)
{//to check is there any open_channel?
    mt_s32 i=0;
    for(i=0;i<DMX_CHANNEL_CNT;i++){
        if((((mt_u8)CHAN_VALID)==g_channel_infor[i].flag_using) && 
           (true_chanhd==g_channel_infor[i].true_chanhd) &&
           (CHANOPEN==g_channel_infor[i].flag_status)){
            return MT_TRUE;
        }
    }
    return MT_FALSE;
}

static mt_u8 mt_checksamehandle(mt_handle *handlebuf,mt_handle handle,mt_u32 count)
{//to check same handle
    mt_s32 i=0;
    for(i=0;i<handle;i++){
        if(handlebuf[i]==handle){
            return MT_TRUE;
        }
    }
    return MT_FALSE;
}

mt_s32 MT_UNF_DMX_ResetCheckChanBuf(mt_handle *hdbuff,mt_u32 maxcnt)
{
    if(NULL==hdbuff){
        return MT_FAILURE;
    }
    memset(hdbuff,0xff,maxcnt*sizeof(mt_handle));
    return MT_SUCCESS;
}

mt_s32 MT_UNF_DMX_CheckChanHandle(mt_handle *hdbuff,mt_u32 maxcnt,mt_handle vhd)
{
    mt_u32 i=0;
    
    if(NULL==hdbuff){
        return MT_FALSE;
    }
    if(vhd>=DMX_CHANNEL_CNT){   //not vhd
        return MT_FALSE;
    }
    for(i=0;i<maxcnt;i++){
        if(hdbuff[i] > DMX_CHANNEL_CNT){        //seek to tail.add it
            hdbuff[i]=vhd;
            return MT_FALSE;
        }
        if(g_channel_infor[vhd].true_chanhd==g_channel_infor[hdbuff[i]].true_chanhd){   //same,it dealed
            return MT_TRUE;
        }
    }
    return MT_FALSE;
}
#endif

mt_u32 MT_UNF_DMX_GetRegister(mt_u32 registerOffset)
{
   return  MT_MPI_DMX_GetRegister(registerOffset);
}

mt_void MT_UNF_DMX_SetRegister(mt_u32 registerOffset, mt_u32 value)
{
    return MT_MPI_DMX_SetRegister(registerOffset, value);
}

mt_void MT_UNF_DMX_DumpAllRegister(void)
{
    return MT_MPI_DMX_DumpAllRegister();
}


mt_s32 MT_UNF_DMX_Init(mt_void)
{
    return MT_MPI_DMX_Init();
}

mt_s32 MT_UNF_DMX_DeInit(mt_void)
{
    return MT_MPI_DMX_DeInit();
}

mt_s32 MT_UNF_DMX_GetCapability(MT_UNF_DMX_CAPABILITY_S *pstCap)
{
    return MT_MPI_DMX_GetCapability(pstCap);
}

mt_s32 MT_UNF_DMX_GetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_ATTR_S *pstAttr)
{
    return MT_MPI_DMX_GetTSPortAttr(enPortId, pstAttr);
}

mt_s32 MT_UNF_DMX_SetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, const MT_UNF_DMX_PORT_ATTR_S *pstAttr)
{
    return MT_MPI_DMX_SetTSPortAttr(enPortId, pstAttr);
}

mt_s32 MT_UNF_DMX_GetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr)
{
    return MT_MPI_DMX_GetTSOPortAttr(enPortId, pstAttr);
}

mt_s32 MT_UNF_DMX_SetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr)
{
    return MT_MPI_DMX_SetTSOPortAttr(enPortId, pstAttr);
}

mt_s32 MT_UNF_DMX_GetDmxTagAttr(mt_u32 u32DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr)
{
    return MT_MPI_DMX_GetDmxTagAttr(u32DmxId, pstAttr);;
}

mt_s32 MT_UNF_DMX_SetDmxTagAttr(mt_u32 u32DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr)
{
    return MT_MPI_DMX_SetDmxTagAttr(u32DmxId, pstAttr);
}

mt_s32 MT_UNF_DMX_AttachTSPort(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E enPortId)
{
    return MT_MPI_DMX_AttachTSPort(u32DmxId, enPortId);
}

mt_s32 MT_UNF_DMX_DetachTSPort(mt_u32 u32DmxId)
{
    return MT_MPI_DMX_DetachTSPort(u32DmxId);
}

mt_s32 MT_UNF_DMX_GetTSPortId(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E *penPortId)
{
    return MT_MPI_DMX_GetTSPortId(u32DmxId, penPortId);
}

mt_s32 MT_UNF_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_E enPortId, mt_u32 u32TsBufSize, mt_handle *phTsBuffer)
{
    return MT_MPI_DMX_CreateTSBuffer(enPortId, u32TsBufSize, phTsBuffer);
}

mt_s32 MT_UNF_DMX_DestroyTSBuffer(mt_handle hTsBuffer)
{
    return MT_MPI_DMX_DestroyTSBuffer(hTsBuffer);
}

mt_s32 MT_UNF_DMX_GetTSBuffer(mt_handle hTsBuffer, mt_u32 u32ReqLen, MT_UNF_STREAM_BUF_S *pstData, mt_u32 u32TimeOutMs)
{
    phys_addr_t u32PhyAddr;

    return MT_MPI_DMX_GetTSBuffer(hTsBuffer, u32ReqLen, pstData, &u32PhyAddr, u32TimeOutMs);
}

mt_s32 MT_UNF_DMX_GetTSBufferEx(mt_handle hTsBuffer, mt_u32 u32ReqLen,
        MT_UNF_STREAM_BUF_S *pstData, phys_addr_t *pu32PhyAddr, mt_u32 u32TimeOutMs)
{
    return MT_MPI_DMX_GetTSBuffer(hTsBuffer, u32ReqLen, pstData, pu32PhyAddr, u32TimeOutMs);
}

mt_s32 MT_UNF_DMX_PutTSBuffer(mt_handle hTsBuffer, mt_u32 u32ValidDataLen)
{
    return MT_MPI_DMX_PutTSBuffer(hTsBuffer, u32ValidDataLen, 0);
}

mt_s32 MT_UNF_DMX_PutTSBuffer_V1(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u8 ts_type, mt_u32 pid)
{
    return MT_MPI_DMX_PutTSBuffer_V1(hTsBuffer, u32ValidDataLen, 0, (dmx_ts_data_t)ts_type, pid);
}

mt_s32 MT_UNF_DMX_PutTSBufferEx(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos)
{
    return MT_MPI_DMX_PutTSBuffer(hTsBuffer, u32ValidDataLen, u32StartPos);
}

mt_s32 MT_UNF_DMX_ResetTSBuffer(mt_handle hTsBuffer)
{
    return MT_MPI_DMX_ResetTSBuffer(hTsBuffer);
}

mt_s32 MT_UNF_DMX_GetTSBufferStatus(mt_handle hTsBuffer, MT_UNF_DMX_TSBUF_STATUS_S *pStatus)
{
    return MT_MPI_DMX_GetTSBufferStatus(hTsBuffer, pStatus);
}

mt_s32 MT_UNF_DMX_GetTSBufferPortId(mt_handle hTsBuffer, MT_UNF_DMX_PORT_E *penPortId)
{
    return MT_MPI_DMX_GetTSBufferPortId(hTsBuffer, penPortId);
}

mt_s32 MT_UNF_DMX_GetTSBufferHandle(MT_UNF_DMX_PORT_E enPortId, mt_handle *phTsBuffer)
{
    return MT_MPI_DMX_GetTSBufferHandle(enPortId, phTsBuffer);
}

mt_s32 MT_UNF_DMX_GetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S *p_av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S *p_rec_care)
{
	return MT_MPI_DMX_GetTSBufferFullCare(hTsBuffer, p_av_care, p_rec_care);
}

mt_s32 MT_UNF_DMX_SetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S rec_care)
{
	return MT_MPI_DMX_SetTSBufferFullCare(hTsBuffer, av_care, rec_care);
}

mt_s32 MT_UNF_DMX_GetChannelDefaultAttr(MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    return MT_MPI_DMX_GetChannelDefaultAttr(pstChAttr);
}

mt_s32 MT_UNF_DMX_CreateChannel(mt_u32 u32DmxId, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr, mt_handle *phChannel)
{
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    mt_u8 vchanid=0xff;
#endif

    if (MT_NULL == pstChAttr)
    {
        MT_WARN_DEMUX("parameter pstChAttr is NULL\n");
        return MT_ERR_DMX_NULL_PTR;
    }

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if((MT_UNF_DMX_CHAN_TYPE_SEC==pstChAttr->enChannelType)||
       (MT_UNF_DMX_CHAN_TYPE_ECM_EMM==pstChAttr->enChannelType)){   //create a v channel
        vchanid=mt_getavchannel();
        if(0xff==vchanid){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }else{
            g_channel_infor[vchanid].flag_using=(mt_u8)CHAN_VALID;
            g_channel_infor[vchanid].flag_status=(mt_u8)CHANCREATE;
            g_channel_infor[vchanid].dmxid=u32DmxId;
            g_channel_infor[vchanid].tmp=*pstChAttr;
            g_channel_infor[vchanid].u32Pid=0xffff;
            *phChannel=(mt_s32)vchanid;
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif
	
    if (    (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY       == pstChAttr->enOutputMode)
        ||  (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY_REC   == pstChAttr->enOutputMode) )
    {
        /*
        if (    (MT_UNF_DMX_CHAN_TYPE_AUD == pstChAttr->enChannelType)
            ||  (MT_UNF_DMX_CHAN_TYPE_VID == pstChAttr->enChannelType) )
        {
            MT_ERR_DEMUX("Not support to creat av channel to play\n");
            return MT_ERR_DMX_INVALID_PARA;
        }
        */
    }
    //printf("pstChAttr->buffsize = 0x%x\n",pstChAttr->u32BufSize);
    return MT_MPI_DMX_CreateChannel(u32DmxId, pstChAttr, phChannel);
}

mt_s32 MT_UNF_DMX_DestroyChannel(mt_handle hChannel)
{
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){     //this s v-channel
        if((mt_u8)CHAN_VALID==g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
            memset(&g_channel_infor[hChannel],0x00,sizeof(MT_UNF_CHANNELINFOR));
            if(MT_TRUE==mt_findsametruehd(hChannel_true)){
                pthread_mutex_unlock(&SamePidMutiChan_Lock);
                return MT_SUCCESS;
            }
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_DestroyChannel(hChannel_true);
}

mt_s32 MT_UNF_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    mt_handle hChannel_true=hChannel;
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){     //this s v-channel
        if((mt_u8)CHAN_VALID==g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_GetChannelAttr(hChannel_true, pstChAttr);
}

mt_s32 MT_UNF_DMX_SetChannelAttr(mt_handle hChannel, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    mt_handle hChannel_true=hChannel;
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){     //this s v-channel
        if((mt_u8)CHAN_VALID==g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_SetChannelAttr(hChannel_true, pstChAttr);
}

mt_s32 MT_UNF_DMX_SetChannelPID(mt_handle hChannel, mt_u32 u32Pid)
{    
    mt_s32 ret=0;
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	mt_u8 pos=0xff;
#endif

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){                                         //this s v-channel
        if((mt_u8)CHAN_VALID!=g_channel_infor[hChannel].flag_using){        //bad para
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        pos=mt_findsamepid(u32Pid);
        if(DMX_CHANNEL_CNT>pos){                                            
            g_channel_infor[hChannel].true_chanhd=g_channel_infor[pos].true_chanhd;
            g_channel_infor[hChannel].u32Pid=u32Pid;
            g_channel_infor[hChannel].datanum=0;
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }else{
            ret=MT_MPI_DMX_CreateChannel(g_channel_infor[hChannel].dmxid,&g_channel_infor[hChannel].tmp,&g_channel_infor[hChannel].true_chanhd);
            if(MT_SUCCESS==ret){
                hChannel_true=g_channel_infor[hChannel].true_chanhd;
                g_channel_infor[hChannel].datanum=0;
            }else{
                pthread_mutex_unlock(&SamePidMutiChan_Lock);
                return MT_FAILURE;
            }
        }
    }
#endif

    ret=MT_MPI_DMX_SetChannelPID(hChannel_true, u32Pid);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    if((MT_SUCCESS==ret)&&(hChannel < DMX_CHANNEL_CNT)){                    //this is v-channel
        g_channel_infor[hChannel].u32Pid=u32Pid;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_GetChannelPID(mt_handle hChannel, mt_u32 *pu32Pid)
{
    mt_handle hChannel_true=hChannel;
    
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){     //this s v-channel
        if((mt_u8)CHAN_VALID==g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_GetChannelPID(hChannel_true, pu32Pid);
}

mt_s32 MT_UNF_DMX_OpenChannel(mt_handle hChannel)
{
    mt_s32 ret=0;
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){                         //this s v-channel
        if((mt_u8)CHAN_VALID!=g_channel_infor[hChannel].flag_using){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        if(0xffff==g_channel_infor[hChannel].u32Pid){       //don't set pid
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        hChannel_true=g_channel_infor[hChannel].true_chanhd;
        ret=mt_checkanyopenchan(hChannel_true);                //find open
        if(MT_TRUE==ret){
            g_channel_infor[hChannel].flag_status=CHANOPEN;
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }
    }
#endif

    ret=MT_MPI_DMX_OpenChannel(hChannel_true);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    if((MT_SUCCESS==ret)&&(hChannel < DMX_CHANNEL_CNT)){    //this s v-channel
        g_channel_infor[hChannel].flag_status=CHANOPEN;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_CloseChannel(mt_handle hChannel)
{
    mt_s32 ret=0;
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){                             //this s v-channel
        if((mt_u8)CHAN_VALID!=g_channel_infor[hChannel].flag_using){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        if(CHANOPEN!=g_channel_infor[hChannel].flag_status){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }
        hChannel_true=g_channel_infor[hChannel].true_chanhd;
        g_channel_infor[hChannel].flag_status=CHANSTOP;
        ret=mt_checkanyopenchan(hChannel_true);                //find open
        if(MT_TRUE==ret){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }
        g_channel_infor[hChannel].flag_status=CHANOPEN;
    }
#endif

    ret=MT_MPI_DMX_CloseChannel(hChannel_true);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    if((MT_SUCCESS==ret)&&(hChannel < DMX_CHANNEL_CNT)){        //this s v-channel
        g_channel_infor[hChannel].flag_status=CHANSTOP;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_GetChannelStatus(mt_handle hChannel, MT_UNF_DMX_CHAN_STATUS_S *pstStatus)
{
    mt_handle hChannel_true=hChannel;
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(DMX_CHANNEL_CNT>hChannel){
        if((mt_u8)CHAN_VALID!=g_channel_infor[hChannel].flag_using){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        hChannel_true=g_channel_infor[hChannel].true_chanhd;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_GetChannelStatus(hChannel_true, pstStatus);
}

mt_s32 MT_UNF_DMX_GetChannelTsCount(mt_handle hChannel, mt_u32 *pu32TsCount)
{
    return MT_MPI_DMX_GetChannelTsCount(hChannel,pu32TsCount);
}

mt_s32 MT_UNF_DMX_GetChannelHandle(mt_u32 u32DmxId, mt_u32 u32Pid, mt_handle *phChannel)
{// only for audio & video
    return MT_MPI_DMX_GetChannelHandle(u32DmxId, u32Pid, phChannel);
}

mt_s32 MT_UNF_DMX_GetChannelHandleByPidType(mt_u32 u32DmxId, mt_u32 u32Pid, MT_UNF_DMX_CHAN_TYPE_E enChannelType, mt_handle *phChannel)
{
    return MT_MPI_DMX_GetChannelHandleByPidType(u32DmxId, u32Pid, enChannelType, phChannel);
}

/*
**  for Advca get audio & video channel handle
*/
mt_s32 MT_UNF_DMX_GetAVChannelHandle(mt_u32 u32DmxId, mt_handle *videoChannel, mt_handle *audioChannel)
{
    return MT_MPI_DMX_GetAVChannelHandle(u32DmxId, videoChannel, audioChannel);
}

mt_s32 MT_UNF_DMX_GetFreeChannelCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount)
{
    return MT_MPI_DMX_GetFreeChannelCount(u32DmxId, pu32FreeCount);
}

mt_s32 MT_UNF_DMX_GetScrambledFlag(mt_handle hChannel, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag)
{
    mt_handle hChannel_true=hChannel;
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(DMX_CHANNEL_CNT>hChannel){
        if((mt_u8)CHAN_VALID!=g_channel_infor[hChannel].flag_using){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        hChannel_true=g_channel_infor[hChannel].true_chanhd;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_GetScrambledFlag(hChannel_true, penScrambleFlag);
}

mt_s32 MT_UNF_DMX_CreateFilter(mt_u32 u32DmxId, const MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr, mt_handle *phFilter)
{
    mt_s32 ret=0;
    ret=MT_MPI_DMX_CreateFilter(u32DmxId, pstFilterAttr, phFilter);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(MT_SUCCESS==ret){
        g_filter_infor[(*phFilter)&0xff].linkvchannel=0xff;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_DestroyFilter(mt_handle hFilter)
{
    mt_s32 ret=0;
    ret=MT_MPI_DMX_DestroyFilter(hFilter);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(MT_SUCCESS==ret){
        g_filter_infor[(hFilter)&0xff].linkvchannel=0xff;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_DeleteAllFilter(mt_handle hChannel)
{
    mt_s32 ret=0;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	mt_s32 i = 0;
#endif

    ret=MT_MPI_DMX_DeleteAllFilter(hChannel);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if((MT_SUCCESS==ret)&&(hChannel < DMX_CHANNEL_CNT)){
        for(i=0;i<DMX_FILTER_CNT;i++){
            if(g_filter_infor[i].linkvchannel==hChannel){
                g_filter_infor[i].linkvchannel=0xff;
            }
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_SetFilterAttr(mt_handle hFilter, const MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr)
{
    return MT_MPI_DMX_SetFilterAttr(hFilter, pstFilterAttr);
}

mt_s32 MT_UNF_DMX_GetFilterAttr(mt_handle hFilter, MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr)
{
    return MT_MPI_DMX_GetFilterAttr(hFilter, pstFilterAttr);
}

mt_s32 MT_UNF_DMX_AttachFilter(mt_handle hFilter, mt_handle hChannel)
{
    mt_s32 ret=0;
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){                         //this s v-channel
        if((mt_u8)CHAN_VALID==g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
    }
#endif
    
    ret=MT_MPI_DMX_AttachFilter(hFilter, hChannel_true);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    if((MT_SUCCESS==ret) && (hChannel < DMX_CHANNEL_CNT)){  //this s v-channel
        g_filter_infor[hFilter&0xff].linkvchannel=hChannel;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_DetachFilter(mt_handle hFilter, mt_handle hChannel)
{
    mt_s32 ret=0;
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){                         //this s v-channel
        if((mt_u8)CHAN_INVALID!=g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
    }
#endif
    
    ret=MT_MPI_DMX_DetachFilter(hFilter, hChannel_true);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    if((MT_SUCCESS==ret) && (hChannel < DMX_CHANNEL_CNT)){  //this s v-channel
        g_filter_infor[hFilter&0xff].linkvchannel=0xff;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_GetFilterChannelHandle(mt_handle hFilter, mt_handle *phChannel)
{
    mt_s32 ret=0;

    ret=MT_MPI_DMX_GetFilterChannelHandle(hFilter, phChannel);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if((MT_SUCCESS==ret) && ((hFilter&0xff)<DMX_FILTER_CNT)){                //this s v-channel
        if(g_filter_infor[hFilter&0xff].linkvchannel<DMX_CHANNEL_CNT){
            *phChannel=g_filter_infor[hFilter&0xff].linkvchannel;
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif
    
    return ret;
}

mt_s32 MT_UNF_DMX_GetFreeFilterCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount)
{
    return MT_MPI_DMX_GetFreeFilterCount(u32DmxId, pu32FreeCount);
}

mt_s32 MT_UNF_DMX_CheckDataHandle(mt_handle hChannel, mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;
	mt_handle hChannel_true=hChannel;
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){
        if((mt_u8)CHAN_VALID==g_channel_infor[hChannel].flag_using){
            hChannel_true=g_channel_infor[hChannel].true_chanhd;
        }else{
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }

		if (0 != g_channel_infor[hChannel].datanum)
		{
			pthread_mutex_unlock(&SamePidMutiChan_Lock);
			return MT_SUCCESS;
		}
    }		
	
	pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    ret=MT_MPI_DMX_CheckDataHandle(hChannel_true, u32TimeOutMs);
	
    return ret;
}

mt_s32 MT_UNF_DMX_GetDataHandle(mt_handle *phChannel, mt_u32 *pu32ChNum, mt_u32 u32TimeOutMs)
{
    mt_s32 ret=0;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	mt_s32 i = 0, j = 0, m = 0, flag_findit=0;
#endif

    MT_ALWAYS_PRINT("please use MT_UNF_DMX_CheckDataHandle instead of MT_UNF_DMX_GetDataHandle\n");   
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    mt_u32 maxchnum=*pu32ChNum;
    mt_handle tmpChannel[DMX_CHANNEL_CNT]={0xff};
    //begin. to add cache-handle
    for(i=0; i<DMX_CHANNEL_CNT; i++){               //v-chanid  .first get cached-data!
        if((m>=maxchnum)||(m>=DMX_CHANNEL_CNT)){
            break;
        }
        if((CHANOPEN==g_channel_infor[i].flag_status)&&
           (0!=g_channel_infor[i].datanum)){
            phChannel[m++]=i;
        }
    }
    //end. to add chache-handle
    if(maxchnum<=m){        //overflow
        *pu32ChNum=maxchnum;
        pthread_mutex_unlock(&SamePidMutiChan_Lock);
        return MT_SUCCESS;
    }
    if(0!=m){               //no need to wait for infor from driver
        u32TimeOutMs=0;
    }
	pthread_mutex_unlock(&SamePidMutiChan_Lock);
    #endif

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    ret=MT_MPI_DMX_GetDataHandle(tmpChannel, pu32ChNum, u32TimeOutMs);
#else
    ret=MT_MPI_DMX_GetDataHandle(phChannel, pu32ChNum, u32TimeOutMs);
#endif
	
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	pthread_mutex_lock(&SamePidMutiChan_Lock);
    if((MT_SUCCESS == ret) && (0!=*pu32ChNum)){
        for(j=0;j<*pu32ChNum;j++){
            flag_findit=0;
            //begin. to add vchannel
            for(i=0; i<DMX_CHANNEL_CNT; i++){
                if((m>=maxchnum)||(m>=DMX_CHANNEL_CNT)){
                    break;
                }
                if(g_channel_infor[i].true_chanhd==tmpChannel[j]){
                    flag_findit=1;
                    if(((mt_u8)CHANOPEN==g_channel_infor[i].flag_status)&&
                        (MT_FALSE==mt_checksamehandle(phChannel,i,m))){
                        phChannel[m++]=i;
                    }
                }
            }
            //end. to add vchannel
            if((m>=maxchnum)||(m>=DMX_CHANNEL_CNT)){
                break;
            }
            if(0==flag_findit){                         //add true handle
                phChannel[m++]=tmpChannel[j];
            }
        }
        if(maxchnum<=m){        //overflow
            *pu32ChNum=maxchnum;
        }else{
            *pu32ChNum=m;
        }
    }else if(0!=m){             //get nothing from driver,so return cache!
        *pu32ChNum=m;
        ret=MT_SUCCESS;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_SelectDataHandle(mt_handle *phWatchChannel, mt_u32 u32WatchNum,
            mt_handle *phDataChannel, mt_u32 *pu32ChNum, mt_u32 u32TimeOutMs)
{
    mt_s32 ret=0;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	mt_s32 i = 0, j = 0, m = 0, flag_findit = 0;
#endif

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
        pthread_mutex_lock(&SamePidMutiChan_Lock);
        mt_u32 maxchnum=*pu32ChNum;
        mt_handle tmpChannel[DMX_CHANNEL_CNT]={0xff};
        mt_handle BakWchChannel[DMX_CHANNEL_CNT]={0xff};

        memcpy(BakWchChannel,phWatchChannel,u32WatchNum*sizeof(mt_handle));
        //begin. to add cache-handle
        for(j=0; j<u32WatchNum;j++){
            if(BakWchChannel[j]>=DMX_CHANNEL_CNT){
                continue;
            }
            if(CHANOPEN==g_channel_infor[BakWchChannel[j]].flag_status){
                if(0!=g_channel_infor[BakWchChannel[j]].datanum){                 //v-chanid  .first get cached-data!
                    if((m<maxchnum)&&(m<DMX_CHANNEL_CNT)){
                        phDataChannel[m++]=BakWchChannel[j];
                    }
                }
            }
            BakWchChannel[j]=g_channel_infor[BakWchChannel[j]].true_chanhd;       //replace to true handle
        }
        //end. to add cache-handle
        if(m==u32WatchNum){                                 //return cached handles
            *pu32ChNum=u32WatchNum;
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }
        if(maxchnum<=m){                                    //if *pu32ChNum<u32WatchNum. this is app'abnormal
            *pu32ChNum=maxchnum;
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_SUCCESS;
        }
        if(0!=m){               //no need to wait for infor from driver
            u32TimeOutMs=0;
        }
		pthread_mutex_unlock(&SamePidMutiChan_Lock);
    #endif

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    ret=MT_MPI_DMX_SelectDataHandle(BakWchChannel, u32WatchNum, tmpChannel, pu32ChNum, u32TimeOutMs);
#else
    ret=MT_MPI_DMX_SelectDataHandle(phWatchChannel, u32WatchNum, phDataChannel, pu32ChNum, u32TimeOutMs);
#endif

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	pthread_mutex_lock(&SamePidMutiChan_Lock);
    if((MT_SUCCESS == ret) && (0!=*pu32ChNum)){
        for(j=0;j<*pu32ChNum;j++){
            flag_findit=0;
            //begin. to add vchannel
            for(i=0;i<u32WatchNum;i++){
                if(phWatchChannel[i]>=DMX_CHANNEL_CNT){      //true handle
                    continue;
                }
                if((m>=maxchnum)||(m>=DMX_CHANNEL_CNT)){    //overflow
                    break;
                }
                if(tmpChannel[j]==g_channel_infor[phWatchChannel[i]].true_chanhd){           //link to vhanlde
                    flag_findit=1;
                    if((CHANOPEN==g_channel_infor[phWatchChannel[i]].flag_status)&&
                       (MT_FALSE==mt_checksamehandle(phDataChannel,phWatchChannel[i],m))){
                        phDataChannel[m++]=phWatchChannel[i];
                    }
                }
            }
            //end. to add vchannel
            if((m>=maxchnum)||(m>=DMX_CHANNEL_CNT)){
                break;
            }
            if(0==flag_findit){                                 //add true handle
                phDataChannel[m++]=tmpChannel[j];
            }
        }
        if(maxchnum<=m){        //overflow
            *pu32ChNum=maxchnum;
        }else{
            *pu32ChNum=m;
        }
    }else if(0!=m){                     //get nothing from driver,so return cache!
        *pu32ChNum=m;
        ret=MT_SUCCESS;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_AcquireBuf(mt_handle hChannel, mt_u32 u32AcquireNum,
            mt_u32 *pu32AcquiredNum, MT_UNF_DMX_DATA_S *pstBuf, mt_u32 u32TimeOutMs)
{
    mt_s32 ret;
    mt_handle hChannel_true=hChannel;

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
	mt_s32 i = 0;
#endif

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    mt_u8 tmpchanid;
    
    if(hChannel<DMX_CHANNEL_CNT){              //v-chanid
        if((mt_u8)CHAN_VALID!=g_channel_infor[hChannel].flag_using){        //bad para
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        hChannel_true=g_channel_infor[hChannel].true_chanhd;
        if(0!=g_channel_infor[hChannel].datanum){
            if(u32AcquireNum<g_channel_infor[hChannel].datanum){         //get part
                *pu32AcquiredNum=u32AcquireNum;
            }else{
                *pu32AcquiredNum=g_channel_infor[hChannel].datanum;
            }
            memcpy(pstBuf,g_channel_infor[hChannel].datas,*pu32AcquiredNum*sizeof(MT_UNF_DMX_DATA_S));
            if(u32AcquireNum<g_channel_infor[hChannel].datanum){         //get part
                memmove(g_channel_infor[hChannel].datas,&g_channel_infor[hChannel].datas[*pu32AcquiredNum],(g_channel_infor[hChannel].datanum-*pu32AcquiredNum)*sizeof(MT_UNF_DMX_DATA_S));
                g_channel_infor[hChannel].datanum-=*pu32AcquiredNum;
            }else{
                g_channel_infor[hChannel].datanum=0;
            }
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            //printf("++++get data from cache.%d\n",*pu32AcquiredNum);
            return MT_SUCCESS;
        }
    }
#endif
    
    ret=MT_MPI_DMX_AcquireBuf(hChannel_true, u32AcquireNum, pu32AcquiredNum, pstBuf, u32TimeOutMs);

#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    if((MT_SUCCESS == ret) && (0!=*pu32AcquiredNum)){       //copy section info!
        if(hChannel < DMX_CHANNEL_CNT){
            for(i=0; i<*pu32AcquiredNum; i++){
                tmpchanid=g_filter_infor[pstBuf[i].filthandle&0xff].linkvchannel;
                if(DMX_CACHE_SECTIONS > g_channel_infor[tmpchanid].datanum){
                    g_channel_infor[tmpchanid].datas[g_channel_infor[tmpchanid].datanum++]=pstBuf[i];
                    //printf("+++in:%x,%x,%x,%x\n",(mt_u32)pstBuf[i].pu8Data,pstBuf[i].pu8Data[0],pstBuf[i].pu8Data[1],pstBuf[i].filthandle);
                }
            }
            if(0!=g_channel_infor[hChannel].datanum){
                if(u32AcquireNum<g_channel_infor[hChannel].datanum){         //get part
                    *pu32AcquiredNum=u32AcquireNum;
                }else{
                    *pu32AcquiredNum=g_channel_infor[hChannel].datanum;
                }
                memcpy(pstBuf,g_channel_infor[hChannel].datas,*pu32AcquiredNum*sizeof(MT_UNF_DMX_DATA_S));
                if(u32AcquireNum<g_channel_infor[hChannel].datanum){         //get part
                    memmove(g_channel_infor[hChannel].datas,&g_channel_infor[hChannel].datas[*pu32AcquiredNum],(g_channel_infor[hChannel].datanum-*pu32AcquiredNum)*sizeof(MT_UNF_DMX_DATA_S));
                    g_channel_infor[hChannel].datanum-=*pu32AcquiredNum;
                }else{
                    g_channel_infor[hChannel].datanum=0;
                }
                pthread_mutex_unlock(&SamePidMutiChan_Lock);
                return MT_SUCCESS;
            }else{
                *pu32AcquiredNum=0;
                pthread_mutex_unlock(&SamePidMutiChan_Lock);
                return MT_SUCCESS;
            }
        }
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return ret;
}

mt_s32 MT_UNF_DMX_ReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum, MT_UNF_DMX_DATA_S *pstBuf)
{
    mt_handle hChannel_true=hChannel;
    
#ifdef CONFIG_MT_SAMEPIDMULTICHANNEL
    pthread_mutex_lock(&SamePidMutiChan_Lock);
    if(hChannel < DMX_CHANNEL_CNT){
        if((mt_u8)CHAN_VALID != g_channel_infor[hChannel].flag_using){
            pthread_mutex_unlock(&SamePidMutiChan_Lock);
            return MT_FAILURE;
        }
        hChannel_true=g_channel_infor[hChannel].true_chanhd;
    }
    pthread_mutex_unlock(&SamePidMutiChan_Lock);
#endif

    return MT_MPI_DMX_ReleaseBuf(hChannel_true, u32ReleaseNum, pstBuf);
}

mt_s32 MT_UNF_DMX_GetTSPortPacketNum(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_PACKETNUM_S *sPortStat)
{
    return MT_MPI_DMX_GetTSPortPacketNum(enPortId, sPortStat);
}

mt_s32 MT_UNF_DMX_AcquireEs(mt_handle hChannel, MT_UNF_ES_BUF_S *pEsBuf)
{
    return MT_MPI_DMX_AcquireEs(hChannel, pEsBuf);
}

mt_s32 MT_UNF_DMX_ReleaseEs(mt_handle hChannel, MT_UNF_ES_BUF_S *pEsBuf)
{
    return MT_MPI_DMX_ReleaseEs(hChannel, pEsBuf);
}

mt_s32 MT_UNF_DMX_CreateRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn)
{
    return MT_MPI_DMX_CreateRecChn(pstRecAttr, phRecChn);
}

mt_s32 MT_UNF_DMX_CreateLinkRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn)
{
	return MT_MPI_DMX_CreateLinkRecChn(pstRecAttr, phRecChn);
}

mt_s32 MT_UNF_DMX_DestroyRecChn(mt_handle hRecChn)
{
    return MT_MPI_DMX_DestroyRecChn(hRecChn);
}

mt_s32 MT_UNF_DMX_DestroyLinkRecChn(mt_handle hRecChn)
{
    return MT_MPI_DMX_DestroyLinkRecChn(hRecChn);
}

mt_s32 MT_UNF_DMX_AddRecPid(mt_handle hRecChn, mt_u32 u32Pid, mt_handle *phChannel)
{
    return MT_MPI_DMX_AddRecPid(hRecChn, u32Pid, phChannel);
}

mt_s32 MT_UNF_DMX_DelRecPid(mt_handle hRecChn, mt_handle hChannel)
{
    return MT_MPI_DMX_DelRecPid(hRecChn, hChannel);
}

mt_s32 MT_UNF_DMX_DelAllRecPid(mt_handle hRecChn)
{
    return MT_MPI_DMX_DelAllRecPid(hRecChn);
}

mt_s32 MT_UNF_DMX_AddExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid)
{
    return MT_MPI_DMX_AddExcludeRecPid(hRecChn, u32Pid);
}

mt_s32 MT_UNF_DMX_DelExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid)
{
    return MT_MPI_DMX_DelExcludeRecPid(hRecChn, u32Pid);
}

mt_s32 MT_UNF_DMX_DelAllExcludeRecPid(mt_handle hRecChn)
{
    return MT_MPI_DMX_DelAllExcludeRecPid(hRecChn);
}

mt_s32 MT_UNF_DMX_StartRecChn(mt_handle hRecChn)
{
    return MT_MPI_DMX_StartRecChn(hRecChn);
}

mt_s32 MT_UNF_DMX_StopRecChn(mt_handle hRecChn)
{
    return MT_MPI_DMX_StopRecChn(hRecChn);
}

mt_s32 MT_UNF_DMX_AcquireRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs)
{
    return MT_MPI_DMX_AcquireRecData(hRecChn, pstRecData, u32TimeoutMs);
}

mt_s32 MT_UNF_DMX_AcquireLinkRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs)
{
	return MT_MPI_DMX_AcquireLinkRecData(hRecChn, pstRecData, u32TimeoutMs);
}

mt_s32 MT_UNF_DMX_ReleaseRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData)
{
    return MT_MPI_DMX_ReleaseRecData(hRecChn, pstRecData);
}

mt_s32 MT_UNF_DMX_ReleaseLinkRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData)
{
	return MT_MPI_DMX_ReleaseLinkRecData(hRecChn, pstRecData);
}

mt_s32 MT_UNF_DMX_AcquireRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs)
{
    return MT_MPI_DMX_AcquireRecIndex(hRecChn, pstRecIndex, u32TimeoutMs);
}

mt_s32 MT_UNF_DMX_AcquireLinkRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs)
{
	return MT_MPI_DMX_AcquireLinkRecIndex(hRecChn, pstRecIndex, u32TimeoutMs);
}

mt_s32 MT_UNF_DMX_GetRecBufferStatus(mt_handle hRecChn, MT_UNF_DMX_RECBUF_STATUS_S *pstBufStatus)
{
    return MT_MPI_DMX_GetRecBufferStatus(hRecChn, pstBufStatus);
}

mt_s32 MT_UNF_DMX_Invoke(MT_UNF_DMX_INVOKE_TYPE_E enCmd, const mt_void *pCmdPara)
{
    return MT_MPI_DMX_Invoke(enCmd, pCmdPara);
}

mt_void MT_UNF_DMX_T2MIConfig(MT_UNF_DMX_T2MI_CONFIG_S mpi_t2mi_para)
{
	MT_MPI_DMX_T2MI_Config(mpi_t2mi_para);
}

mt_void MT_UNF_DMX_T2MIEnable(mt_u32 enable)
{
	MT_MPI_DMX_T2MIEnable(enable);
}

mt_void MT_UNF_DMX_T2MISetInCh(MT_UNF_DMX_PORT_E enPortId)
{
	MT_MPI_DMX_T2MISetInCh(enPortId);
}

mt_void MT_UNF_DMX_T2MISetOutCh(MT_UNF_DMX_PORT_E enPortId)
{
	//printf("[%s %d]enPortId=0x%x\n", __FUNCTION__, __LINE__, enPortId);
	MT_MPI_DMX_T2MISetOutCh(enPortId);
}

mt_void MT_UNF_DMX_T2MISetPid(mt_u32 pid)
{
	MT_MPI_DMX_T2MISetPid(pid);
}

mt_void MT_UNF_DMX_T2MISetPlpid(mt_u32 plpid)
{
	MT_MPI_DMX_T2MISetPlpid(plpid);
}

mt_void MT_UNF_DMX_T2MISoftReset()
{
    MT_MPI_DMX_T2MISoftReset();
}

mt_void MT_UNF_DMX_SetPortId(mt_handle hChannel, MT_UNF_DMX_PORT_E enPortId)
{
    MT_MPI_DMX_SetPortId(hChannel, enPortId);
}

mt_void MT_UNF_DMX_Soft_Reset()
{
    MT_MPI_DMX_Soft_Reset();
}

mt_void MT_UNF_DMX_HardwareInit()
{
    MT_MPI_DMX_HardwareInit();
}

mt_s32 MT_UNF_DMX_FastPlayStart(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E enPortId, mt_handle *phRecChn)
{
	mt_s32 ret = MT_SUCCESS;
	MT_UNF_DMX_REC_ATTR_S rec_attr = {0};
	mt_u32 rec_id;
	mt_u8 swtsi_num;
	
	memset(&rec_attr, 0, sizeof(MT_UNF_DMX_REC_ATTR_S));
	rec_attr.u32DmxId = u32DmxId;
	rec_attr.enRecType = MT_UNF_DMX_REC_TYPE_ALL_PID;
	rec_attr.u32RecBufSize = 188*1024*10*4;//8 * 1024 * 1024;
    rec_attr.bDescramed = MT_FALSE;
	rec_attr.rec_mode = MT_UNF_DMX_REC_TIMESTAMP_NONE;
	ret = MT_MPI_DMX_CreateLinkRecChn(&rec_attr, phRecChn);
	if (ret != MT_SUCCESS)
	{
		printf("[%s %d]create link rec channel failed\n", __FUNCTION__, __LINE__);
		return ret;
	}		
	
	ret = MT_MPI_DMX_StartRecChn(*phRecChn);
	if (ret != MT_SUCCESS)
	{
		printf("[%s %d]start rec channel failed\n", __FUNCTION__, __LINE__);
		return ret;
	}

	swtsi_num = enPortId - MT_UNF_DMX_PORT_RAM_0;
	rec_id = (*phRecChn) & 0xFF;
	//printf("[%s %d]swtsi_num=%d, rec_id=%d\n", __FUNCTION__, __LINE__, swtsi_num, rec_id);
	ret = MT_MPI_DMX_FastPlayStart(swtsi_num, rec_id, 0);
	if (ret != MT_SUCCESS)
	{
		printf("[%s %d]start fast play failed\n", __FUNCTION__, __LINE__);
		return ret;
	}

	return ret;
}

mt_s32 MT_UNF_DMX_FastPlayStop(mt_handle phRecChn)
{
	mt_s32 ret = MT_SUCCESS;
	
	ret = MT_MPI_DMX_StopRecChn(phRecChn);
	if (ret != MT_SUCCESS)
	{
		printf("[%s %d]stop rec channel failed\n", __FUNCTION__, __LINE__);
		return ret;
	}
	
	ret = MT_MPI_DMX_DestroyLinkRecChn(phRecChn);
	if (ret != MT_SUCCESS)
	{
		printf("[%s %d]destroy link rec channel failed\n", __FUNCTION__, __LINE__);
		return ret;
	}
	
	ret = MT_MPI_DMX_FastPlayStop();
	if (ret != MT_SUCCESS)
	{
		printf("[%s %d]stop fast play failed\n", __FUNCTION__, __LINE__);
		return ret;
	}

	return ret;
}


mt_s32 MT_UNF_DMX_GetEsBuffPhyAddr(mt_handle hChannel, ulong *esBuffPhyAddr, mt_u32 *esBuffSize)
{
    mt_s32 ret = MT_SUCCESS;
    MT_MPI_DMX_GetEsBuffPhyAddr(hChannel,esBuffPhyAddr,esBuffSize);
    return ret;
    
}

mt_s32 MT_UNF_DMX_Get_TSI_Clk(MT_UNF_DMX_TSI_CLK_E *sel_clk)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Get_TSI_Clk(sel_clk);
	return ret;
}

mt_s32 MT_UNF_DMX_Set_TSI_Clk(MT_UNF_DMX_TSI_CLK_E sel_clk)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Set_TSI_Clk(sel_clk);
	return ret;
}

mt_s32 MT_UNF_DMX_Get_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S *port_ctrl)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Get_TSPort_Ctrl(port_id, port_ctrl);
	return ret;
}

mt_s32 MT_UNF_DMX_Set_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S port_ctrl)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Set_TSPort_Ctrl(port_id, port_ctrl);
	return ret;
}

mt_s32 MT_UNF_DMX_Get_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E *port_clk)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Get_TSPort_Clk(port_id, port_clk);
	return ret;
}

mt_s32 MT_UNF_DMX_Set_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E port_clk)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Set_TSPort_Clk(port_id, port_clk);
	return ret;
}

mt_s32 MT_UNF_DMX_Get_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E *port_src)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Get_TSPort_Src(port_id, port_src);
	return ret;
}

mt_s32 MT_UNF_DMX_Set_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E port_src)
{
	mt_s32 ret = MT_SUCCESS;
	ret = MT_MPI_DMX_Set_TSPort_Src(port_id, port_src);
	return ret;
}

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 MT_UNF_DMX_Chan_Ci_Enable(mt_handle hChannel, mt_u8 isenable)
{

 	return MT_MPI_DMX_CiplusEnable(hChannel, isenable);
}

mt_s32 MT_UNF_TSI_CI_RecChanSet(mt_handle recchan)
{
	return MT_MPI_Tsi_Ciplus_RecChanSet(recchan);
}
	
mt_s32 MT_UNF_TSI_CI_SwtsiChanSet(mt_handle swtsi_chan)
{
	return MT_MPI_Tsi_Ciplus_SwtsiChanSet(swtsi_chan);
}

	
mt_s32 MT_UNF_TSI_CI_BufCfg(mt_u8 usecache)
{
	return MT_MPI_Tsi_Ciplus_BufCfg(usecache);
}

mt_s32 MT_UNF_TSI_CI_ClkCfg(mt_u32 clkdiv)
{
	return MT_MPI_Tsi_Ciplus_ClkCfg(clkdiv);
}

mt_s32 MT_UNF_TSI_CI_TsintervalCfg(mt_u32 interval)
{
	return MT_MPI_Tsi_Ciplus_TsIntervalCfg(interval);
}

mt_s32 MT_UNF_TSI_Tsi2_SourceCfg(mt_u8 from_cam,mt_u8 serial)
{
	return MT_MPI_Tsi_Tsi2SourceCfg(from_cam,serial);
}

mt_s32 MT_UNF_TSI_CI_LlnNumstartCfg(mt_u32 lln_num_start)
{
	return MT_MPI_Tsi_LlnNumStartCfg(lln_num_start);
}

mt_s32 MT_UNF_TSI_CI_SwtsiByteorderCfg(mt_u32 islittle)
{
	return MT_MPI_Tsi_SwtsiByteorderCfg(islittle);
}

mt_s32 MT_UNF_TSI_CI_SwtsiBufFullCareCfg(mt_handle hChannel,mt_u32 full_care)
{
	return MT_MPI_Tsi_SwtsiBufFullCfg(hChannel,full_care);
}

mt_s32 MT_UNF_TSI_CI_AhbRdDelayCfg(mt_handle hChannel, mt_u32 delay)
{
	return MT_MPI_Tsi_AhbRdDelayCfg(hChannel, delay);
}

mt_s32 MT_UNF_TSI_CI_GetCiStaus(mt_u8 *pbufstatus,mt_u8 *ci_status)
{
	return MT_MPI_Tsi_Ciplus_GetStatus(pbufstatus,ci_status);
}

mt_s32 MT_UNF_TSI_CI_EnableCfg(mt_u8 enable)
{
	return MT_MPI_Tsi_Ciplus_EnableCfg(enable);
}


#endif

/*add end*/
