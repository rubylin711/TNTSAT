/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <signal.h>
#include "mt_type.h"
#include "mt_debug.h"
#ifdef CONFIG_MT_CHIP_SYMPHONY1
#include "mt_unf_cipher.h"
#else
#include "mt_unf_cipher_v2.h"
#endif

#include "mt_drv_struct.h"

#include "mt_module_debug.h"
#include "mt_pvr_rec_ctrl.h"
#include "mt_pvr_play_ctrl.h"
#include "mt_pvr_intf.h"
#include "mt_pvr_index.h"
#include "mt_mpi_demux.h"
#include "mt_drv_pvr.h"
#include "mt_mpi_mem.h"
#include "mt_module_debug.h"
#include "mt_pvr_addon_api.h"
#include "mt_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

//#define TEST_DUAL_REC          //for sample_pvr.c
#define  PVR_TIMESHIFT_TIME_LIMIT   2*1000
#define PVR_WRITE_TS_MAX_LEN (512 * 1024)

//static MT_S32 g_anotherFds=-1;
//static MT_U32 g_anotherwlen=0;


//#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#define PVR_DEALIDX_IN_AV
//#endif

#define CRPTOBFSIZE PVR_IOBLOCK//(512*188)
#define CRPTOIDXNUM (2048)
MT_S32 g_s32PvrFd = -1;      /*PVR file description */
char api_pathname_pvr[] = "/dev/" UMAP_DEVNAME_PVR;

/* init flag of record module                                               */
STATIC PVR_REC_COMM_S g_stRecInit;

/* all information of record channel                                        */
STATIC PVR_REC_CHN_S g_stPvrRecChns[PVR_REC_MAX_CHN_NUM];

static PVR_THREAD_ATTR g_indexreadthreadattr={0};
static PVR_THREAD_ATTR g_indexwritethreadattr={0};
static PVR_THREAD_ATTR g_tswritethreadattr={0};

#ifdef VMX_ADVCA_PVR
mt_mmz_buf_s  encrypt_rec_buff[4] = {0};
#endif

pthread_mutex_t g_pvrcrypto=PTHREAD_MUTEX_INITIALIZER;
#define PVR_GET_HEADER_OFFSET() ((ulong) (&((PVR_IDX_HEADER_INFO_S *)0)->stCycInfo))
#define PVR_GET_RECPTR_BY_CHNID(chnId) (&g_stPvrRecChns[chnId - PVR_REC_START_NUM])
#define PVR_REC_IS_REWIND(pstRecAttr) ((pstRecAttr)->bRewind)
#define PVR_REC_IS_FIXSIZE(pstRecAttr) ((((pstRecAttr)->u64MaxFileSize > 0) || ((pstRecAttr)->u64MaxTimeInMs > 0)) && !((pstRecAttr)->bRewind))
#ifdef PVR_PROC_SUPPORT
static MT_PROC_ENTRY_S g_stPvrRecProcEntry;
#endif
static MT_S32 PVRRecStopDemux(PVR_REC_CHN_S *pRecChn);


//extern mt_s32 mt_proc_add_dir(const mt_char * pszName);

MT_S32 pvr_create_thread_attr(pthread_attr_t *p_thread_attr, MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize);


//extern MT_S32 MT_UNF_ADVCA_SetR2RSessionKey(MT_UNF_ADVCA_KEYLADDER_LEV_E enStage, MT_U8 *pu8Key);

static MT_U32  least_common_multiple(MT_U32 a,MT_U32 b)
{
    MT_U32 ret = 0;
    int i =1;

    while((a*i)%b)
    {
        i++;
    }
    ret = a*i;
    PVR_REC_D("---a=%d b=%d i=%d ret=%d\n",a,b,i,ret);
    return ret;
}

static MT_S32 idx_pushque(PVR_INDEX_HANDLE idxhandle,MT_UNF_DMX_REC_INDEX_S *now)
{
    MT_U32 next=((idxhandle->idxstart+1)&(idxhandle->idxmax-1));
    if(next != idxhandle->idxend){
        //printf("+++push %x\n",idxhandle->idxstart);
        idxhandle->idxcache[idxhandle->idxstart]=*now;
        idxhandle->idxstart=next;
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}
static MT_UNF_DMX_REC_INDEX_S *idx_getque(PVR_INDEX_HANDLE idxhandle)
{
    if(idxhandle->idxstart == idxhandle->idxend){
        return NULL;
    }
    //printf("++get %x\n",idxhandle->idxend);
    return &idxhandle->idxcache[idxhandle->idxend];
}
static void idx_popque(PVR_INDEX_HANDLE idxhandle)
{
    idxhandle->idxend=((idxhandle->idxend+1)&(idxhandle->idxmax-1));
}
/* unused warning
static MT_U32 idx_numque(PVR_INDEX_HANDLE idxhandle)
{
    if(idxhandle->idxstart >= idxhandle->idxend){
        return (idxhandle->idxstart-idxhandle->idxend);
    }else{
        return idxhandle->idxend-(idxhandle->idxstart-idxhandle->idxend);
    }
}

static MT_U64 pvr_get_systime(void)
{
    #if 0
    struct  timeval  start;
    MT_U64 systime;
    gettimeofday(&start,NULL);
    systime = 1000*start.tv_sec+ start.tv_usec/1000;
    return systime;
    #else
    MT_U32 time_now;
    MT_PVR_SysGetTimeStampMs(&time_now);
    return (MT_U64)time_now;
    #endif
}
*/

STATIC INLINE void PVRRecPostEvent(MT_U32 u32ChnID, MT_UNF_PVR_EVENT_E enEventType, MT_S32 s32EnvetValue)
{
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            if(p_addon->rec_inter.on_event)
            {
                p_addon->rec_inter.on_event(p_addon->handlesrec[recChannelId],enEventType,s32EnvetValue,0);
            }
        }
    }
    PVR_Intf_DoEventCallback(u32ChnID,enEventType,s32EnvetValue);
}

STATIC INLINE MT_S32 PVRRecDevInit(MT_VOID)
{
    int fd;

    if (g_s32PvrFd == -1)
    {
        fd = open (api_pathname_pvr, O_RDWR | O_CLOEXEC, 0);

        if (fd < 0)
        {
            MT_FATAL_PVR("Cannot open '%s'\n", api_pathname_pvr);
            return MT_FAILURE;
        }

        g_s32PvrFd = fd;
    }

    return MT_SUCCESS;
}

STATIC INLINE PVR_REC_CHN_S * PVRRecFindFreeChn(MT_VOID)
{
    PVR_REC_CHN_S * pChnAttr = NULL;

#if 0 /* not support multi-process */
    MT_U32 i;

    /* find a free play channel */
    for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        if (g_stPvrRecChns[i].enState == MT_UNF_PVR_REC_STATE_INVALID)
        {
            pChnAttr = &g_stPvrRecChns[i];
            pChnAttr->enState = MT_UNF_PVR_REC_STATE_INIT;
            break;
        }
    }

#else /* support multi-process by kernel manage resources */
    MT_U32 ChanId;
    if (MT_SUCCESS != ioctl(g_s32PvrFd, CMD_PVR_CREATE_REC_CHN, (ulong)&ChanId))
    {
        MT_FATAL_PVR("pvr rec creat channel error\n");
        return MT_NULL;
    }

    MT_ASSERT(g_stPvrRecChns[ChanId].enState == MT_UNF_PVR_REC_STATE_INVALID);
    pChnAttr = &g_stPvrRecChns[ChanId];
    PVR_LOCK_REC(pChnAttr);
    pChnAttr->enState = MT_UNF_PVR_REC_STATE_INIT;
    PVR_UNLOCK_REC(pChnAttr);
#endif


    return pChnAttr;
}

/*
STATIC INLINE MT_VOID PVRRecCheckExistFile(MT_CHAR* pTsFileName)
{
    if (PVR_CHECK_FILE_EXIST(pTsFileName))
    {
        MT_CHAR szIdxFileName[PVR_MAX_FILENAME_LEN + 4] = {0};
        snprintf(szIdxFileName, sizeof(szIdxFileName),"%s.idx", pTsFileName);
        truncate(pTsFileName, 0);
        truncate(szIdxFileName, 0);
    }

    return;
}
*/
STATIC INLINE MT_S32 PVRRecCheckUserCfg(const MT_UNF_PVR_REC_ATTR_S *pUserCfg)
{
    MT_U32 i;
    MT_U32 align_size=0;

    CHECK_REC_DEMUX_ID((MT_S32)(pUserCfg->u32DemuxID));

    /* we should determine which demux_id is free                           */

    if ((MT_UNF_PVR_STREAM_TYPE_TS != pUserCfg->enStreamType)
        && (MT_UNF_PVR_STREAM_TYPE_ALL_TS != pUserCfg->enStreamType))
    {
        MT_ERR_PVR("enStreamType error, not support this stream type:(%d)\n", pUserCfg->enStreamType);
        return MT_ERR_PVR_INVALID_PARA;
    }

    /* all ts record, just only used for analysing, not supported cipher and rewind */
    if (MT_UNF_PVR_STREAM_TYPE_ALL_TS == pUserCfg->enStreamType)
    {
        if (pUserCfg->bRewind || pUserCfg->stEncryptCfg.bDoCipher)
        {
            MT_ERR_PVR("All Ts record can't support rewind or ciphter\n");
            return MT_ERR_PVR_INVALID_PARA;
        }
    }

    if (pUserCfg->enIndexType >= MT_UNF_PVR_REC_INDEX_TYPE_BUTT)
    {
        MT_ERR_PVR("pUserCfg->enIndexType(%d) >= MT_UNF_PVR_REC_INDEX_TYPE_BUTT\n", pUserCfg->enIndexType);
        return MT_ERR_PVR_INVALID_PARA;
    }

    if (MT_UNF_PVR_REC_INDEX_TYPE_VIDEO == pUserCfg->enIndexType)
    {
        if (pUserCfg->enIndexVidType >= MT_UNF_VCODEC_TYPE_BUTT)
        {
            MT_ERR_PVR("pUserCfg->enIndexVidType(%d) >= MT_UNF_VCODEC_TYPE_BUTT\n", pUserCfg->enIndexVidType);
            return MT_ERR_PVR_INVALID_PARA;
        }
    }

    if(pUserCfg->u32DIO){
        //find DIO request IOBLOCK and TS packet least common multiple
        align_size = least_common_multiple(PVR_IOBLOCK,188);
        if(align_size == 0){
            align_size = 0x2f0000;
        }
    }else{
        align_size = PVR_FIFO_WRITE_BLOCK_SIZE;
    }

    PVR_REC_D("u32DavBufSize=%d align_size=%d\n",pUserCfg->u32DavBufSize,align_size);
    if ((pUserCfg->u32DavBufSize % align_size)
        || (!((pUserCfg->u32DavBufSize >= PVR_REC_MIN_DAV_BUF)
              && (pUserCfg->u32DavBufSize <= PVR_REC_MAX_DAV_BUF))))
    {
        MT_ERR_PVR("invalid dav buf size:%u ,if enable DIO, should align to %d(both align with %d and 188)\n", pUserCfg->u32DavBufSize,align_size,PVR_IOBLOCK);
        return MT_ERR_PVR_INVALID_PARA;
    }

    if ((pUserCfg->u32ScdBufSize % 188)
        || (!((pUserCfg->u32ScdBufSize >= PVR_REC_MIN_SC_BUF)
              && (pUserCfg->u32ScdBufSize <= PVR_REC_MAX_SC_BUF))))
    {
        MT_ERR_PVR("PVRRecCheckUserCfg  >>> invalid scd buf size :    %u\n", pUserCfg->u32ScdBufSize);
        return MT_ERR_PVR_INVALID_PARA;
    }

    PVR_CHECK_CIPHER_CFG(&pUserCfg->stEncryptCfg);

    /*  if record file name ok */
    if (((strlen(pUserCfg->szFileName)) >= PVR_MAX_FILENAME_LEN)
        || (strlen(pUserCfg->szFileName) != pUserCfg->u32FileNameLen))
    {
        MT_ERR_PVR("Invalid file name, file name len=%d!\n", pUserCfg->u32FileNameLen);
        return MT_ERR_PVR_FILE_INVALID_FNAME;
    }

    if (pUserCfg->u32UsrDataInfoSize > PVR_MAX_USERDATA_LEN)
    {
        MT_ERR_PVR("u32UsrDataInfoSize(%u) too larger\n", pUserCfg->u32UsrDataInfoSize);
        return MT_ERR_PVR_REC_INVALID_UDSIZE;
    }

    /* check for cycle record. for cycle record, the length should more than PVR_MIN_CYC_SIZE, and it MUST not be zero */
    if (PVR_REC_IS_REWIND(pUserCfg))
    {
        if (((pUserCfg->u64MaxFileSize > 0)&&(pUserCfg->u64MaxFileSize < PVR_MIN_CYC_SIZE))
            ||((pUserCfg->u64MaxTimeInMs > 0) && (pUserCfg->u64MaxTimeInMs < PVR_MIN_CYC_TIMEMS)))
        {
            MT_ERR_PVR("record file rewind, but file size:%llu(time:%llu) less than %llu(%llu).\n",
                   pUserCfg->u64MaxFileSize, pUserCfg->u64MaxTimeInMs, PVR_MIN_CYC_SIZE, PVR_MIN_CYC_TIMEMS);
            return MT_ERR_PVR_REC_INVALID_FSIZE;
        }
    }
    else
    {
        /* the length too less and not equal zero. zero means no limited */
        if (((pUserCfg->u64MaxFileSize > 0)&&(pUserCfg->u64MaxFileSize < PVR_MIN_CYC_SIZE))
            ||((pUserCfg->u64MaxTimeInMs > 0)&&(pUserCfg->u64MaxTimeInMs < PVR_MIN_CYC_TIMEMS)))
        {
            MT_ERR_PVR("record file not rewind, but file size:%llu(time:%llu) less than %llu(%llu) and not 0.\n",
                   pUserCfg->u64MaxFileSize, pUserCfg->u64MaxTimeInMs, PVR_MIN_CYC_SIZE, PVR_MIN_CYC_TIMEMS);
            return MT_ERR_PVR_REC_INVALID_FSIZE;
        }
    }

    for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        /* check whether the demux id used or not */
        if (MT_UNF_PVR_REC_STATE_INVALID != g_stPvrRecChns[i].enState)
        {
            if (g_stPvrRecChns[i].stUserCfg.u32DemuxID == pUserCfg->u32DemuxID)
            {
                MT_ERR_PVR("demux %d already has been used to record.\n", pUserCfg->u32DemuxID);
                return MT_ERR_PVR_ALREADY;
            }

            /* recording for the same file name or not*/
            if (0 == strncmp(g_stPvrRecChns[i].stUserCfg.szFileName, pUserCfg->szFileName,sizeof(pUserCfg->szFileName)))
            {
                MT_ERR_PVR("file %s was exist to be recording.\n", pUserCfg->szFileName);
                return MT_ERR_PVR_FILE_EXIST;
            }
        }
    }

    /* check if stream file exist!                                          */
    if(0==pUserCfg->bEnable_CTrec){
        MT_PVR_RemoveFile(pUserCfg->szFileName);
    }

    //PVRRecCheckExistFile((MT_U8*)pUserCfg->szFileName);

    return MT_SUCCESS;
}

/* on recording, in sequence write some data into ts file */
//STATIC MT_S32 PVRRecWriteStreamDirect(PVR_REC_CHN_S *pRecChn, MT_U8 *pBuf, MT_U32 len, MT_U64 u64OffsetInFile)
STATIC MT_S32 PVRRecWriteStreamDirect(PVR_REC_CHN_S *pRecChn, MT_UNF_DMX_REC_DATA_S *stDmxTsData, MT_U32 len, MT_U64 u64OffsetInFile, MT_U64 u64GlobalOffset)
{
    ssize_t sizeWrite, sizeWriten = 0;
    MT_UNF_PVR_DATA_ATTR_S stDataAttr={0};
    //MT_U32            u32StartFrm;
    //MT_U32            u32EndFrm;
    PVR_INDEX_ENTRY_S stStartFrame;
    PVR_INDEX_ENTRY_S stEndFrame;
 //   MT_U64            u64LenAdp = 0;
//    MT_U64            u64OffsetAdp = 0;
    MT_S32            ret = 0;
    
    memset(&stStartFrame, 0, sizeof(PVR_INDEX_ENTRY_S));
    memset(&stEndFrame, 0, sizeof(PVR_INDEX_ENTRY_S));



    MT_INFO_PVR("bSupportAdvCa=%d, writeCallBack = %x,pRecChn->stUserCfg.enStreamType=%x, stDmxTsData.u32Len=%x len=0x%x\n",
                          pRecChn->stUserCfg.bSupportAdvCa,
    		pRecChn->writeCallBack,pRecChn->stUserCfg.enStreamType,stDmxTsData->u32Len, len);
//   static MT_U32 total_size_writen = 0;
//    static MT_U32 total_time_cost = 0;
//    struct timeval start_time, end_time;
//    gettimeofday(&start_time,NULL);

    if((NULL != pRecChn->writeCallBack) && (MT_UNF_PVR_STREAM_TYPE_ALL_TS  != pRecChn->stUserCfg.enStreamType))
    {
#ifdef CONFIG_MT_LXC_SUPPORT
        int fd;
        off_t in_phy;
#endif
        mt_u32 time_now,time_last,offset=0,celllen=len;
//        MT_UNF_DMX_REC_DATA_S wrpart;
//        MT_S32 ret=MT_SUCCESS;
        MT_U8 *cryptobuff=NULL;
        mt_u32 phyaddr=0;

        stDataAttr.u32ChnID = pRecChn->u32ChnID;
        stDataAttr.u64GlobalOffset = u64GlobalOffset;
        stDataAttr.u32DmxID = pRecChn->stUserCfg.u32DemuxID;
        memcpy(&stDataAttr.usercfg,&pRecChn->stUserCfg.stEncryptCfg,sizeof(MT_UNF_PVR_CIPHER_S));
        if(pRecChn->stUserCfg.stEncryptCfg.bDoCipher)
        {
            #ifdef CONFIG_MT_LXC_SUPPORT
            fd=PVR_GetTsFileFd(pRecChn->dataFile);
            if(0<=fd){
                cryptobuff=mt_pvr_ipc_getbufferby_tid(fd,MAX_CRYPTOSZ_W,&in_phy);
                phyaddr=(mt_u32)in_phy;
            }
            #else
            cryptobuff=pRecChn->hCipher_buffer;
            phyaddr=pRecChn->hsec_mmz_rec.phyaddr;
            #endif
            if(NULL==cryptobuff){
                return MT_FAILURE;
            }
            //printf("++w:%x\n",stDmxTsData.u32Len);
            MT_PVR_SysGetTimeStampMs(&time_last);
        }
        else
        {
            cryptobuff = stDmxTsData->pDataAddr;
            phyaddr = stDmxTsData->u32DataPhyAddr;
        }
            while(len){
                if(pRecChn->stUserCfg.stEncryptCfg.bDoCipher)
                {
                    if(MAX_CRYPTOSZ_W > len)
                        celllen=len;
                    else
                        celllen=MAX_CRYPTOSZ_W;
                }
                ret = pRecChn->writeCallBack(&stDataAttr,
                                               cryptobuff,
                                               phyaddr,
                                               stDmxTsData->pDataAddr,
                                               stDmxTsData->u32DataPhyAddr,
                                               offset,
                                               &celllen);
                
                if(ret == MT_ERR_PVR_REC_CB_FAIL_DROP_DATA){
                    MT_ERR_PVR("pvr.write.callback.fail request drop data,offset=%llu size=%d\n",
                        u64OffsetInFile + (MT_U64)offset+(MT_U64)sizeWriten,celllen);
                }else{
                    sizeWriten=0;
                    do{
                        sizeWrite = PVR_PWRITE64(pRecChn,cryptobuff+sizeWriten,
                                                 celllen - (MT_U32)sizeWriten,
                                                 pRecChn->dataFile,
                                                 u64OffsetInFile + (MT_U64)offset+(MT_U64)sizeWriten);
                        if ((-1) == sizeWrite){
                           if (NULL != &errno){
                               if (EINTR == errno){
                                   MT_WARN_PVR("EINTR can't write ts. try:%u, addr:%p, fd:%d\n", len, cryptobuff, pRecChn->dataFile);
                                   continue;
                               }else if (ENOSPC == errno){
                                   MT_ERR_PVR("pvr.write.full.err\n");
                                   return MT_ERR_PVR_FILE_DISC_FULL;
                               }else{
                                   MT_ERR_PVR("can't write ts. try:%u, addr:%p, fd:%d\n", len, cryptobuff, pRecChn->dataFile);
                                   return MT_ERR_PVR_FILE_CANT_WRITE;
                               }
                           }
                        }
                        sizeWriten += sizeWrite;
                    }while((MT_U32)sizeWriten < celllen);
                }
                offset+=celllen;
                len -=celllen;
                stDataAttr.u64GlobalOffset+=celllen;
                if(pRecChn->stUserCfg.stEncryptCfg.bDoCipher)
                    MT_PVR_SysGetTimeStampMs(&time_now);
                if((time_now > time_last) && 30<=(time_now-time_last)){
                    MT_USLEEP(1000*4);
                    time_last=time_now;
                }
            }
    }
    else
    {
	    do
    	{

            #ifdef TEST_DUAL_REC  //add for vmx
              if((MT_UNF_PVR_STREAM_TYPE_ALL_TS != pRecChn->stUserCfg.enStreamType)){
                   if(-1==g_anotherFds){
                       char tmpname[128];
                       strcpy(tmpname,"/tmp/medias/sda1/");
                       strcat(tmpname,"another.ts");
                       g_anotherFds = PVR_OPEN(tmpname, PVR_FOPEN_MODE_DATA_WRITE);//O_CREAT | O_RDWR | O_LARGEFILE
                       MT_INFO_PVR("++open_00:%s=%x\n",tmpname,g_anotherFds);
                       g_anotherwlen=0;
                   }
                   if(-1!=g_anotherFds){
                       ssize_t writed=pvr_write(g_anotherFds,&((const char *)stDmxTsData->pDataAddr)[sizeWriten],len - (MT_U32)sizeWriten);
                       if(writed > 0){
                          g_anotherwlen+=writed;
                       }
                   }
               }
             #endif

    	         sizeWrite = PVR_PWRITE64(pRecChn, &((const char *)stDmxTsData->pDataAddr)[sizeWriten],
    	                                 len - (MT_U32)sizeWriten,
    	                                 pRecChn->dataFile,
    	                                 u64OffsetInFile + (MT_U64)sizeWriten);
    	        if ((-1) == sizeWrite)
    	        {
    	            //lint -e774
    	            if (NULL != &errno)
    	            {
    	                if (EINTR == errno)
    	                {
    	                    MT_WARN_PVR("EINTR can't write ts. try:%u, addr:%p, fd:%d\n", len, stDmxTsData->pDataAddr, pRecChn->dataFile);
    	                    continue;
    	                }
    	                else if (ENOSPC == errno)
    	                {
    	                    return MT_ERR_PVR_FILE_DISC_FULL;
    	                }
    	                else
    	                {
    	                    MT_ERR_PVR("can't write ts. try:%u, addr:%p, fd:%d\n", len, stDmxTsData->pDataAddr, pRecChn->dataFile);
    	                    return MT_ERR_PVR_FILE_CANT_WRITE;
    	                }
    	            }
    	            //lint +e774
    	        }

    	        sizeWriten += sizeWrite;
    	    } while ((MT_U32)sizeWriten < len);
	    //*realWriteLen = len;
    }
    /*
    gettimeofday(&end_time,NULL);
    int time_interval = (end_time.tv_sec - start_time.tv_sec)*(1000*1000) +(end_time.tv_usec - start_time.tv_usec);
    
    total_size_writen += sizeWriten;
    total_time_cost += time_interval;
    if( total_time_cost >= 1000*1000*10)
    {
        MT_ERR_PVR("write %d bytes, cost %d us %d Kbytes/s \n",total_size_writen,time_interval,( (total_size_writen/1024)/(total_time_cost/1000/1000)));
        total_size_writen = 0;
        total_time_cost = 0;
    }
    */
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : PVRRecCycWriteStream
 Description     : write stream to file
 Input           : pBuf      **
                   len       **
                   dataFile  **
                   chnID     **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/29
    Author       : q46153
    Modification : Created function

*****************************************************************************/
//STATIC MT_S32 PVRRecCycWriteStream(MT_U8 *pBuf, MT_U32 len, PVR_REC_CHN_S *pRecChn)
STATIC MT_S32 PVRRecCycWriteStream(MT_UNF_DMX_REC_DATA_S *stDmxTsData, PVR_REC_CHN_S *pRecChn)
{
    MT_U32 len = 0;
    MT_U32 len1 = 0;
    MT_U32 len2 = 0;
    MT_U64 before_pos = pRecChn->u64CurFileSize;
    MT_S32 ret;
    MT_U64 before_globalPos = before_pos;
    MT_U64 u64MaxSize = 0;

    //MT_ERR_PVR("==%d,%llu\n", pRecChn->stUserCfg.bRewind, pRecChn->stUserCfg.u64MaxFileSize);

    len = stDmxTsData->u32Len;
    len1 = len;

    /* record fixed file length,  reach to the length, stop record */
    if (MT_UNF_PVR_STREAM_TYPE_ALL_TS == pRecChn->stUserCfg.enStreamType)
    {
        u64MaxSize =  pRecChn->stUserCfg.u64MaxFileSize;
    }
    else
    {
        PVR_LOCK_REC(pRecChn);
        if(pRecChn->IndexHandle)
        {
            if(MT_TRUE==pRecChn->IndexHandle->bTimeRewindTsPauseFlg){             //in vir-pause
                u64MaxSize = (pRecChn->IndexHandle->u64TimeTsPausePos - pRecChn->IndexHandle->u64TimeRewindMaxSize);
            }else{
                u64MaxSize = pRecChn->IndexHandle->stCycMgr.u64MaxCycSize;
            }
        }
        PVR_UNLOCK_REC(pRecChn);
    }
    len1 = len;
    if (PVR_REC_IS_FIXSIZE(&pRecChn->stUserCfg))
    {
        if (u64MaxSize > 0)
        {
            if ((pRecChn->u64CurFileSize + len) > u64MaxSize)
            {
               MT_ERR_PVR("cur size will over fix size, cur size:%llu, fix size:%llu\n",
                         pRecChn->u64CurFileSize, u64MaxSize);
               return MT_ERR_PVR_FILE_TILL_END;
            }
        }
    }
    else if (PVR_REC_IS_REWIND(&pRecChn->stUserCfg))  /* case rewind record */
    {
        PVR_LOCK_REC(pRecChn);
        if(pRecChn->IndexHandle)
        {
            if (PVR_INDEX_REWIND_BY_TIME == pRecChn->IndexHandle->stCycMgr.enRewindType)
            {
                if (u64MaxSize > 0)
                {
                  PVR_EVENT_DD
                	before_pos = pRecChn->u64CurFileSize - pRecChn->IndexHandle->u64TimeRewindMaxSize;
                  if(MT_TRUE==pRecChn->IndexHandle->bTimeRewindTsPauseFlg){//vir-puase.
                      before_pos = pRecChn->u64CurFileSize - pRecChn->IndexHandle->u64TimeTsPausePos;
                  }
    				PVR_EVENT_DD
    				if (MT_TRUE == pRecChn->IndexHandle->bTimeRewinded)
    				{
    	                pRecChn->bEventFlg = MT_TRUE;
    	                pRecChn->s32OverFixTimes++;
    					pRecChn->IndexHandle->bTimeRewinded = MT_FALSE;
    				}
                }
            }
            else
            {
                before_pos = pRecChn->u64CurFileSize % u64MaxSize;
    
                if ((before_pos + (MT_U64)len) >= u64MaxSize) /* stride the rewind */
                {
                    pRecChn->IndexHandle->stCycMgr.enRewindType = PVR_INDEX_REWIND_BY_SIZE;
                    pRecChn->s32OverFixTimes++;
                    len1 = (MT_U32)(u64MaxSize - before_pos);
                    len2 = len - len1;
                    pRecChn->bEventFlg = MT_TRUE;
                }
            }
        }
        PVR_UNLOCK_REC(pRecChn);
    }

    if (stDmxTsData->pDataAddr && len1 > 0)
    {
        //MT_INFO_PVR("Write before_pos=%llu, len=%d.\n", before_pos ,len1);
       // ret = PVRRecWriteStreamDirect(pRecChn, pBuf, len1, before_pos);
        ret = PVRRecWriteStreamDirect(pRecChn, stDmxTsData, len1, before_pos, pRecChn->u64CurFileSize);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("write pvr file error\n");
            return ret;
        }
	    vmx_pvr_rec_printf("%s====%d, %d\n",__FUNCTION__,__LINE__, pRecChn->stUserCfg.bSupportAdvCa);
#if 0
	 if((MT_UNF_PVR_REC_VMX==pRecChn->stUserCfg.bSupportAdvCa)||(MT_UNF_PVR_REC_COMMON_CRAMBLE==pRecChn->stUserCfg.bSupportAdvCa))
	 {
		pRecChn->u64CurFileSize += pRecChn->real_write_data_len;
        	pRecChn->u32Flashlen += pRecChn->real_write_data_len;
	 }
	 else
#endif
	 {
        	pRecChn->u64CurFileSize += len1;
        	pRecChn->u32Flashlen += len1;
	 }
	if (pRecChn->u32Flashlen >= 1024 * 1024)
        {
            //PVR_FSYNC64(pRecChn->dataFile);
            pRecChn->u32Flashlen = 0;
        }

        //MT_ERR_PVR("S=%llu, Max=%llu.\n", pRecChn->u64CurFileSize ,pRecChn->stUserCfg.u64MaxFileSize);
        //printf("r + %u = %llx.\n", len1, pRecChn->u64CurFileSize);
    }

    if (len2 > 0)
    {
        /* for two direction, after writing the first direction, it just write to the end of file*/
        if(u64MaxSize > 0)
        {
            MT_ASSERT(0LLU == (pRecChn->u64CurFileSize % u64MaxSize));
        }

        //ret = PVRRecWriteStreamDirect(pRecChn, pBuf + len1, len2, 0);
        stDmxTsData->pDataAddr += len1;
        stDmxTsData->u32DataPhyAddr += len1;
        ret = PVRRecWriteStreamDirect(pRecChn, stDmxTsData, len2, 0, pRecChn->u64CurFileSize);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("write pvr file error\n");
            return ret;
        }
	    vmx_pvr_rec_printf("%s====%d, %d\n",__FUNCTION__,__LINE__, pRecChn->stUserCfg.bSupportAdvCa);
#if 0
        if((MT_UNF_PVR_REC_VMX==pRecChn->stUserCfg.bSupportAdvCa)||(MT_UNF_PVR_REC_COMMON_CRAMBLE==pRecChn->stUserCfg.bSupportAdvCa))
	 {
		pRecChn->u64CurFileSize += pRecChn->real_write_data_len;
        	pRecChn->u32Flashlen += pRecChn->real_write_data_len;
	 }
	 else
#endif
        {
        	pRecChn->u64CurFileSize += len2;
        	pRecChn->u32Flashlen += len2;
    	 }

	 pRecChn->hardware_rec_buffer_rp += len1;

	 if (pRecChn->u32Flashlen >= 1024 * 1024)
        {
            //PVR_FSYNC64(pRecChn->dataFile);
            pRecChn->u32Flashlen = 0;
        }
        if(pRecChn->stUserCfg.u32DIO){
            if(!pRecChn->stUserCfg.stEncryptCfg.bDoCipher)
            {
            	PVR_Set_Rewinded(pRecChn->dataFile);
            }
        }
        //MT_INFO_PVR("r2 + %u = %llx.\n", len1, pRecChn->u64CurFileSize);
    }
    else
    {
        MT_ASSERT(len1 == len);
    }

    if (MT_UNF_PVR_STREAM_TYPE_ALL_TS != pRecChn->stUserCfg.enStreamType)
    {
        PVR_LOCK_REC(pRecChn);
        if(pRecChn->IndexHandle)
        {
            pRecChn->IndexHandle->u64FileSizeGlobal = pRecChn->u64CurFileSize;
            if (before_globalPos < pRecChn->IndexHandle->stCurPlayFrame.u64GlobalOffset)
            {
                if (pRecChn->u64CurFileSize > pRecChn->IndexHandle->stCurPlayFrame.u64GlobalOffset)
                {
                    MT_ERR_PVR("cur size will over readPos, %llu-->%llu, ReadPos:%llu\n",
                               before_globalPos, pRecChn->u64CurFileSize,
                               pRecChn->IndexHandle->stCurPlayFrame.u64GlobalOffset);
                }
            }
        }
        PVR_UNLOCK_REC(pRecChn);
    }

    return MT_SUCCESS;
}

STATIC INLINE MT_VOID PVRRecCheckError(const PVR_REC_CHN_S  *pChnAttr, MT_S32 ret)
{
    if (MT_SUCCESS == ret)
    {
        return;
    }

    if (MT_ERR_DMX_NOAVAILABLE_DATA == ret)
    {
        return;
    }

    if (MT_ERR_PVR_FILE_DISC_FULL == ret)
    {
        PVRRecPostEvent(pChnAttr->u32ChnID, MT_UNF_PVR_EVENT_REC_DISKFULL, 0);
    }
    else if (MT_ERR_PVR_FILE_TILL_END == ret)
    {
        PVRRecPostEvent(pChnAttr->u32ChnID, MT_UNF_PVR_EVENT_REC_OVER_FIX, 0);
    }
    else
    {
        PVRRecPostEvent(pChnAttr->u32ChnID, MT_UNF_PVR_EVENT_REC_ERROR, ret);
    }

    return;
}

STATIC INLINE MT_VOID PVRRecCheckRecPosition(PVR_REC_CHN_S  *pChnAttr)
{
    MT_S32 times;

    if (PVR_REC_IS_REWIND(&pChnAttr->stUserCfg))
    {
        if (pChnAttr->s32OverFixTimes > 0)
        {
            times = pChnAttr->s32OverFixTimes;
            pChnAttr->s32OverFixTimes = 0;
            PVRRecPostEvent(pChnAttr->u32ChnID, MT_UNF_PVR_EVENT_REC_OVER_FIX, times);
        }
    }
}
/*
STATIC MT_VOID PVRRecPrintIndex(MT_UNF_DMX_REC_INDEX_S *s, void *func, MT_U32 line)
{
    printf("\n");
    printf("\n%s,%d:ftype=%u\n",func,line,(MT_U32)s->enFrameType);
    printf("%s,%d:pts=%u\n",func,line,(MT_U32)s->u32PtsMs);
    printf("%s,%d:goffset=%llu\n",func,line,s->u64GlobalOffset);
    printf("%s,%d:fsize=%u\n",func,line,(MT_U32)s->u32FrameSize);
    printf("%s,%d:dtime=%u\n",func,line,(MT_U32)s->u32DataTimeMs);
    printf("%s,%d:sc0=%u\n",func,line,(MT_U32)s->u32HdrData[0]);
    printf("%s,%d:sc1=%u\n",func,line,(MT_U32)s->u32HdrData[1]);
    printf("%s,%d:owner=%u\n",func,line,(MT_U32)s->u32BufOwner);
    printf("\n");
}
*/
#ifdef PVR_DEALIDX_IN_AV
#if 0
STATIC MT_S32 PVRRecWriteIndex(FILE *fpDmxIdx, PVR_REC_CHN_S *pRecChn,MT_S32 isfirst)
{
    MT_S32 ret = MT_SUCCESS;

    MT_UNF_DMX_REC_INDEX_S stDmxIndexInfo={0};
    if(1==isfirst){
        stDmxIndexInfo.u32DataTimeMs=1;
    }else{
        stDmxIndexInfo.u32DataTimeMs=0;
    }
    if(pRecChn->IndexHandle->idx_chn_num_avap >= 4){
        MT_USLEEP(20000);
        return ret;
    }
    /* loop to record index                                                    */
    ret = MT_UNF_DMX_AcquireRecIndex(pRecChn->DemuxRecHandle, &stDmxIndexInfo, PVR_REC_DMX_GET_SC_TIME_OUT);
    if (MT_SUCCESS == ret){
        MT_USLEEP(12000);   //release cpu!. h264 multislice
        REWRITE:
        //printf("++w0.type=%x,%x,%x,%x,%llx\n",stDmxIndexInfo.u32PtsMs,stDmxIndexInfo.u32HdrData[0],stDmxIndexInfo.u32HdrData[1],stDmxIndexInfo.u32FrameSize,stDmxIndexInfo.u64GlobalOffset);
        if(!((0==stDmxIndexInfo.u32HdrData[0])&&(0==stDmxIndexInfo.u32HdrData[1]))){

            stDmxIndexInfo.enFrameType |= (pRecChn->IndexHandle->idx_chn_num_avap<<24);
            if(MT_SUCCESS != ioctl(g_s32PvrFd, CMD_PVR_REC_WRITE_DATA, (ulong)(&stDmxIndexInfo))){
                //printf("\n%s_%d\n",__func__,__LINE__);
                if(MT_UNF_PVR_REC_STATE_STOP != pRecChn->enState){
                    MT_USLEEP(10000);
                    goto REWRITE;
                }
            }
        }
    }else{
        MT_USLEEP(20000);   //release cpu!. h264 multislice
    }
    return ret;
}
#endif

static mt_s32 find_overflow(DMX_RecInfo_Overflow_S *recinf,mt_u64 offset,mt_u64 *s,mt_u64 *e,mt_u32 *c)
{
    mt_u32 loop=DMX_OVERFLOW_MAX,wp=recinf->wp;

    if(0==recinf->overflow_allcnt){
        return -1;
    }

    while(loop){
      loop--;
      if(0==wp){
        wp=DMX_OVERFLOW_MAX-1;
      }else{
        wp--;
      }
      if(0==recinf->overflow_times[wp]){
         return -1;
      }
      if(offset>recinf->overflow_s[wp]){
        *s=recinf->overflow_s[wp];
        *e=recinf->overflow_e[wp];
        *c=recinf->overflow_times[wp];
        //printf("++f.%x,%llx,%llx\n",*c,*s,*e);
        return 0;
      }
    }
    return -1;
}
/*****************************************************************************
 Prototype       : PVRRecSaveIndex
 Description     : save index to fs
 Input           : pRecChn  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/11
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC MT_S32 PVRRecSaveIndex(FILE *fpDmxIdx, PVR_REC_CHN_S *pRecChn)
{
    MT_S32 ret = MT_SUCCESS,flag_rec_info_vld=0;
    MT_UNF_DMX_RECBUF_STATUS_S tmp_recinfo;
    MT_U32 u32DirectFlag = 0;
    MT_UNF_DMX_REC_INDEX_ARRY_S stDmxIndexInfo_arry;
    MT_UNF_DMX_REC_INDEX_S *stDmxIndexInfo;
    MT_UNF_VCODEC_TYPE_E VDecType=pRecChn->stUserCfg.enIndexVidType;
    MT_U32 i=0;
    MT_U32 nums = (sizeof(MT_UNF_DMX_REC_INDEX_ARRY_S) /sizeof(MT_UNF_DMX_REC_INDEX_S));
    MT_U64 overflow_rp_s=0,overflow_rp_e=0;
    MT_U32 overflow_rp_c=0;
    mt_u32 tmp_raw = 0;
    MT_U64 diff = 0,adjust=0,filesize=0;

    /* loop to record index                                                    */
    ret = MT_SUCCESS;//MT_UNF_DMX_AcquireRecIndex(pRecChn->DemuxRecHandle, &stDmxIndexInfo, PVR_REC_DMX_GET_SC_TIME_OUT);

    if (MT_SUCCESS == ret)
    {
        REREAD:
        if(pRecChn->IndexHandle->idx_chn_num_avap < 4){
            stDmxIndexInfo_arry.index[0].u32PrivatePara=pRecChn->IndexHandle->idx_chn_num_avap;
            //printf("+++rec.app will get %x\n",pRecChn->IndexHandle->idx_chn_num_avap);
        }else{
            MT_USLEEP(20000);//release cpu!
            return ret;
        }
        if(MT_SUCCESS != ioctl(g_s32PvrFd, CMD_PVR_REC_READ_DATA_EX, (ulong)(&stDmxIndexInfo_arry))){
            //printf("\n%s_%d\n",__func__,__LINE__);
            MT_USLEEP(10000);//release cpu!
            if((0==pRecChn->share_rec.flg_share) && (MT_UNF_PVR_REC_STATE_STOP == pRecChn->enState)){ //stop and no share
                return ret;
            }else{
                goto REREAD;
            }
        }
        PVR_LOCK_REC(pRecChn);
        flag_rec_info_vld=0;

		memset(&tmp_recinfo, 0x00, sizeof(tmp_recinfo));
        if(MT_SUCCESS==MT_UNF_DMX_GetRecBufferStatus(pRecChn->DemuxRecHandle,&tmp_recinfo)){
            //printf("+++rp=%llx.wp=%llx\n",tmp_recinfo.u64RpGlobe,tmp_recinfo.u64BufDataLen);
            flag_rec_info_vld=1;
        } 
        i=0;
        while(i<nums){
            stDmxIndexInfo=&stDmxIndexInfo_arry.index[i];
            i++;
            //printf("+++idxfromav:%x,%llx,%x,%x\n",stDmxIndexInfo->enFrameType,stDmxIndexInfo->u64GlobalOffset,
            //                                      stDmxIndexInfo->u32FrameSize,stDmxIndexInfo->u32PtsMs,stDmxIndexInfo->u32PrivatePara);
            if(0xffffffff==stDmxIndexInfo->enFrameType){
                break;
            }
            #if 1
            if(MT_UNF_FRAME_TYPE_UNKNOWN==stDmxIndexInfo->enFrameType){
                //pRecChn->IndexHandle->stCurRecFrame.u32DisplayTimeMs = stDmxIndexInfo->u32DataTimeMs;
                //pRecChn->IndexHandle->stCurRecFrame.u32DisplayTimeMs = stDmxIndexInfo->u32DataTimeMs - pRecChn->First_RawTime + pRecChn->IndexHandle->Last_DisplayTimeMs+40;
                continue;
            }
            #endif
            if((MT_UNF_VCODEC_TYPE_H264==VDecType)||
               (MT_UNF_VCODEC_TYPE_HEVC==VDecType)){   //this for 264/265
                if(MT_UNF_FRAME_TYPE_I!=stDmxIndexInfo->enFrameType){    //ignore P-frame
                    stDmxIndexInfo->enFrameType=MT_UNF_FRAME_TYPE_B;
                }
            }
            if((MT_UNF_VCODEC_TYPE_HEVC==VDecType)&&(1==stDmxIndexInfo->u32PrivatePara)){
                stDmxIndexInfo->enFrameType=MT_UNF_FRAME_TYPE_I;
                //printf("++++force I\n");
            }
            stDmxIndexInfo->u32HdrData[0]=(stDmxIndexInfo->u64GlobalOffset-(stDmxIndexInfo->u64GlobalOffset/188*188));
            stDmxIndexInfo->u64GlobalOffset+=pRecChn->re_rec_firstsize;
            if(0xffffffff==pRecChn->First_RawTime){  //record first time
                pRecChn->First_RawTime=stDmxIndexInfo->u32DataTimeMs;
                pRecChn->IndexHandle->stCycMgr.u64AllRecStartInMs=(pRecChn->IndexHandle->Last_DisplayTimeMs+40);
            }
            {   //add time
                tmp_raw=stDmxIndexInfo->u32DataTimeMs;
                stDmxIndexInfo->u32DataTimeMs=(tmp_raw-pRecChn->First_RawTime+pRecChn->IndexHandle->Last_DisplayTimeMs+40);
                //printf("++idx.dsptime=%d,%d,%d\n",stDmxIndexInfo.u32DataTimeMs,pRecChn->First_DisplayTime,
                //                               pRecChn->IndexHandle->Last_DisplayTimeMs);
                if(pRecChn->stUserCfg.bNoInterval_InRec){ //fix Bug 126839
                    if(stDmxIndexInfo->u32DataTimeMs >= (pRecChn->Last_IdxTime+1000)){  //more than 1s,we think no signal!
                        pRecChn->First_RawTime=tmp_raw;
                        pRecChn->IndexHandle->Last_DisplayTimeMs=pRecChn->Last_IdxTime;
                        stDmxIndexInfo->u32DataTimeMs=(tmp_raw-pRecChn->First_RawTime+pRecChn->IndexHandle->Last_DisplayTimeMs+40);
                        printf("+++PVR[%x]reset time=%d,%d\n",pRecChn->u32ChnID,pRecChn->Last_IdxTime,stDmxIndexInfo->u32DataTimeMs);
                    }
                    pRecChn->Last_IdxTime=stDmxIndexInfo->u32DataTimeMs;
                }
            }

            pRecChn->IndexHandle->u64GlobalOffset = stDmxIndexInfo->u64GlobalOffset;
            //printf("\n%s_%d,%lld,%lld,%d\n",__func__,__LINE__, pRecChn->IndexHandle->u64GlobalOffset,pRecChn->u64CurFileSize,pRecChn->IndexHandle->bTimeRewindFlg);
            {//get cache.
                MT_UNF_DMX_REC_INDEX_S *tmp;
                for(;;){
                    tmp=idx_getque(pRecChn->IndexHandle);
                    if(NULL==tmp){
                        //printf("idx null\n");
                        break;
                    }
                    //printf("++w2.type=%x,%x,%x,%llx\n",tmp->enFrameType,tmp->u32HdrData[0],tmp->u32HdrData[1],tmp->u64GlobalOffset);
                    if(MT_UNF_PVR_REC_STATE_STOP != pRecChn->enState){
                        if(flag_rec_info_vld && (0==tmp->u32PrivatePara)){            //adjust overflow.
                        	overflow_rp_s = 0;
							overflow_rp_e = 0;
                            overflow_rp_c=0;

                            find_overflow(&tmp_recinfo.overflowinfo,tmp->u64GlobalOffset,&overflow_rp_s,&overflow_rp_e,&overflow_rp_c);
                            if(overflow_rp_c){
                                if((tmp->u64GlobalOffset>=overflow_rp_s) && (tmp->u64GlobalOffset<overflow_rp_e)){
                                    idx_popque(pRecChn->IndexHandle);
                                    continue;
                                }
                                tmp->u64GlobalOffset-=((MT_U64)overflow_rp_c*tmp_recinfo.u32BufSize);//u32 * u32 may over u32
                                tmp->u32PrivatePara=1;
                            }
                        }

                        filesize = pRecChn->u64CurFileSize;//the u64CurFileSize will ceaseless increase,backup to local before use
                        if(((tmp->u64GlobalOffset+tmp->u32FrameSize) >= filesize) && (MT_FALSE == pRecChn->IndexHandle->bTimeRewindFlg)){
                            diff = (tmp->u64GlobalOffset+tmp->u32FrameSize) - filesize;
                            if(diff >= 0x20000000){// index offset and fileszie diff over 512M
                                adjust = (MT_U64)tmp_recinfo.overflowinfo.overflow_allcnt*tmp_recinfo.u32BufSize;
                                MT_ERR_PVR("----offset abnormal %llx,%x,%llx,%llx,%llx, %d,%d,%d,%d,%d\n",
                                    tmp->u64GlobalOffset,tmp->u32FrameSize,filesize,diff,adjust,
                                    flag_rec_info_vld,overflow_rp_c,tmp->u32PrivatePara,tmp_recinfo.overflowinfo.overflow_allcnt,tmp_recinfo.u32BufSize);
                                
                                if(flag_rec_info_vld == 1 && overflow_rp_c==0 && tmp_recinfo.overflowinfo.overflow_allcnt && (tmp->u64GlobalOffset > adjust)){
                                    //overflow not find, overflow data may overwrite
                                    tmp->u64GlobalOffset -=adjust;
                                    if((tmp->u64GlobalOffset+tmp->u32FrameSize) >= filesize){
                                        MT_ERR_PVR("idx low2 %llx,%llx\n",tmp->u64GlobalOffset,filesize);
                                        break;
                                    }
                                }else{
                                    MT_ERR_PVR("idx low3 %llx,%llx\n",tmp->u64GlobalOffset,filesize);
                                    break;
                                    //idx_popque(pRecChn->IndexHandle);
                                    //continue;
                                }
                            }else{
                                //printf("idx low %llx,%llx,%x,%llx,%d\n",tmp->u64GlobalOffset,filesize,tmp->u32FrameSize,diff,overflow_rp_c);
                                break;
                            }
                        }
                        if((pRecChn->share_rec.flg_share) && (pRecChn->share_rec.Shared_recchptr)){ //check slave saved
                            PVR_REC_CHN_S *tmprec_s=(PVR_REC_CHN_S *)pRecChn->share_rec.Shared_recchptr;
                            PVR_LOCK_REC(tmprec_s);
                            if( (MT_UNF_PVR_REC_STATE_STOP != tmprec_s->enState) && (MT_UNF_PVR_REC_STATE_INVALID != tmprec_s->enState)){
                              if(((tmp->u64GlobalOffset+tmp->u32FrameSize) >= (tmprec_s->share_rec.offset_inrecbuf+tmprec_s->u64CurFileSize)) && 
                                  (MT_FALSE == tmprec_s->IndexHandle->bTimeRewindFlg)){
                                  //printf("idx low %llx,%llx\n",tmp->u64GlobalOffset,pRecChn->u64CurFileSize);
                                  PVR_UNLOCK_REC(tmprec_s);
                                  break;
                              }
                            }
                            PVR_UNLOCK_REC(tmprec_s);
                        }
                    }else if((pRecChn->share_rec.flg_share) && (pRecChn->share_rec.Shared_recchptr)){
                        PVR_REC_CHN_S *tmprec_s=(PVR_REC_CHN_S *)pRecChn->share_rec.Shared_recchptr;
                        PVR_LOCK_REC(tmprec_s);
                        if(MT_UNF_PVR_REC_STATE_STOP == tmprec_s->enState || MT_UNF_PVR_REC_STATE_INVALID == tmprec_s->enState){
                            PVR_UNLOCK_REC(tmprec_s);
                            break;
                        }
                        if(((tmp->u64GlobalOffset+tmp->u32FrameSize) >= (tmprec_s->share_rec.offset_inrecbuf+tmprec_s->u64CurFileSize)) &&
                            (MT_FALSE == tmprec_s->IndexHandle->bTimeRewindFlg)){
                            //printf("idx low %llx,%llx\n",tmp->u64GlobalOffset,pRecChn->u64CurFileSize);
                            PVR_UNLOCK_REC(tmprec_s);
                            break;
                        }
                        PVR_UNLOCK_REC(tmprec_s);
                    }

                    idx_popque(pRecChn->IndexHandle);
                    //pRecChn->IndexHandle->u32DmxClkTimeMs = tmp->u32DataTimeMs;
                    //pRecChn->IndexHandle->stCycMgr.u64AllRecNowInMs = pvr_get_systime();

                    MT_U32 last_s=pRecChn->IndexHandle->stCycMgr.u32StartFrame;
                    if(flag_rec_info_vld && (0==tmp->u32PrivatePara)){            //adjust overflow.
                        overflow_rp_c=0;
						overflow_rp_s = 0;
						overflow_rp_e = 0;
                        find_overflow(&tmp_recinfo.overflowinfo,tmp->u64GlobalOffset,&overflow_rp_s,&overflow_rp_e,&overflow_rp_c);
                        if(overflow_rp_c){
                            if((tmp->u64GlobalOffset>=overflow_rp_s) && (tmp->u64GlobalOffset<overflow_rp_e)){
                                continue;
                            }
                            tmp->u64GlobalOffset-=((MT_U64)overflow_rp_c*tmp_recinfo.u32BufSize);
                            //printf("minus %x\n",overflow_rp_c);
                        }
                    }
                    if( (MT_UNF_PVR_REC_STATE_STOP != pRecChn->enState) && (MT_UNF_PVR_REC_STATE_INVALID != pRecChn->enState)){
                        PVR_Index_SaveFramePosition(pRecChn->u32ChnID, pRecChn->IndexHandle->u32RecPicParser, tmp, u32DirectFlag);
                    }
                    if(pRecChn->share_rec.flg_share){
                        PVR_REC_CHN_S *tmprec_s=(PVR_REC_CHN_S *)pRecChn->share_rec.Shared_recchptr;
                        if(tmprec_s && (MT_UNF_PVR_REC_STATE_STOP != tmprec_s->enState) && \
                             (tmprec_s->IndexHandle && tmprec_s->IndexHandle->recsize64))
                        { //after stream be record!
                            if(tmp->u64GlobalOffset>=tmprec_s->share_rec.offset_inrecbuf){
                                tmp->u64GlobalOffset-=tmprec_s->share_rec.offset_inrecbuf;
                              if(0xffffffff==tmprec_s->First_RawTime){  //record first time
                                 tmprec_s->First_RawTime=tmp_raw;
                                 tmprec_s->IndexHandle->stCycMgr.u64AllRecStartInMs=tmprec_s->IndexHandle->Last_DisplayTimeMs+40;
                                 tmprec_s->share_rec.time_offset_ms = tmp->u32DataTimeMs;
                                 //printf("\n##PVRRecSaveIndex Last_DisplayTimeMs=%d, First_RawTime=%d, time_offset_ms=%d\n",
                                   //tmprec_s->IndexHandle->Last_DisplayTimeMs, tmprec_s->First_RawTime,tmprec_s->share_rec.time_offset_ms);
                               }
                                tmp->u32DataTimeMs -= tmprec_s->share_rec.time_offset_ms;
                                PVR_Index_SaveFramePosition(tmprec_s->u32ChnID, tmprec_s->IndexHandle->u32RecPicParser, tmp, u32DirectFlag);
                            }
                        }
                    }
                    PVR_CYC_MGR_S *pCycMgr=&pRecChn->IndexHandle->stCycMgr;
                    if(pCycMgr->u32LastFrame>0){
                        //MT_U32 s = pCycMgr->u32StartFrame;
                        MT_U32 e = pCycMgr->u32EndFrame;
                        MT_U32 l = pCycMgr->u32LastFrame;
                        MT_U32 r = pRecChn->IndexHandle->u32ReadFrame;
                        if(e>0){
                            e--;
                        }
                        if(l>0){
                            l--;
                        }
                        //printf("+++++:s.e.l.r=%x,%x,%x,%x\n",s,e,l,r);
                        if(pRecChn->IndexHandle->stCycMgr.bIsRewind &&
                            (MT_TRUE==PVR_Play_IsFilePlayingSlowPauseBack(pRecChn->stUserCfg.szFileName))){  //pRecChn->IndexHandle->stCycMgr.bIsRewind
                            if((last_s!=pCycMgr->u32StartFrame) && (MT_TRUE==Pvr_Check_ReadInRange(last_s,pCycMgr->u32StartFrame,r))){ /* rec reach to play */
                                PVR_REC_D("++rech[%d].rec.reach play last_s=%d s=%d r=%d\n",pRecChn->u32ChnID,last_s,pCycMgr->u32StartFrame,r);
                                PVR_Index_SeekToStart(pRecChn->IndexHandle); /* force play to move forward */
                                PVR_REC_D("++rech[%d].rec.reach play seek read to start r=%d\n",pRecChn->u32ChnID,pRecChn->IndexHandle->u32ReadFrame);
                                pRecChn->IndexHandle->u32RecReachPlay = 1;
                                pRecChn->IndexHandle->cause_resume=2;
                                PVRRecPostEvent(pRecChn->u32ChnID, MT_UNF_PVR_EVENT_REC_REACH_PLAY, 0);
                            }
                        }
                    }
                }
            }
            stDmxIndexInfo->u32PrivatePara=0;
            ret=idx_pushque(pRecChn->IndexHandle,stDmxIndexInfo);
            //printf("++3s.t=%x.%x\n",pRecChn->IndexHandle->idxstart,pRecChn->IndexHandle->idxend);
            if(MT_SUCCESS != ret){
                //printf("+++cache full\n");
            }
        }
        PVR_UNLOCK_REC(pRecChn);
        return MT_SUCCESS;
    }
    else
    {
//        printf("MT_UNF_DMX_AcquireRecIndex ret: [%d]\n",ret);
        if ((MT_ERR_DMX_NOAVAILABLE_DATA != ret) && (MT_ERR_DMX_TIMEOUT != ret))
        {
            MT_ERR_PVR("Acquire index data failed:%#x\n", ret);
        }

        MT_USLEEP(10000);
    }
    return ret;
}

STATIC MT_VOID * PVRRecGetAudioThread(MT_VOID *args)
{
    MT_S32 ret = MT_SUCCESS;
    MT_S32 isfirst=1;
    PVR_REC_CHN_S *pRecChn = (PVR_REC_CHN_S *)args;
    char threadname[128]={0};
//    FILE *fpDmxSc = NULL;
    MT_UNF_DMX_REC_INDEX_S stDmxIndexInfo={0};
    MT_U32 timelast=0,timenow=0;
    MT_U64 sizelast=0;
    MT_U64 align_size=0;
    MT_U32 tsalign=(188*4);   //ts align to 16 for crypto
    MT_INFO_PVR("++++++++++++++++++++++++++++++++play write\n");

    strcpy(threadname,__FUNCTION__);
    mt_set_pthread_name((char *)threadname);

    while (MT_UNF_PVR_REC_STATE_STOP != pRecChn->enState)
    {
        if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO==pRecChn->IndexHandle->enIndexType){
            align_size=pRecChn->IndexHandle->recsize64/tsalign*tsalign;
            if(0==align_size){
              MT_USLEEP(20000);
              continue;
            }
            if(align_size && isfirst){
              isfirst=0;
              MT_PVR_SysGetTimeStampMs(&timelast);
              sizelast=align_size;
              if(0xffffffff==pRecChn->First_RawTime){  //record first time
                    pRecChn->First_RawTime = timelast;
                    pRecChn->IndexHandle->stCycMgr.u64AllRecStartInMs = (pRecChn->IndexHandle->Last_DisplayTimeMs+40);// for rewind use
                    printf("%s audio firest record done.\n",__FUNCTION__);                  
              }
              
              MT_USLEEP(20000);
              continue;
            }
            MT_PVR_SysGetTimeStampMs(&timenow);
            if(timenow > (timelast+200)){   //200ms a frame
              if((align_size > sizelast) && ((align_size - sizelast)>tsalign)){
                stDmxIndexInfo.u32PtsMs=timelast;
                stDmxIndexInfo.CodecType=timelast;
                stDmxIndexInfo.enFrameType=MT_UNF_FRAME_TYPE_I;
                stDmxIndexInfo.u32FrameSize=(mt_u32)(align_size-sizelast);
                stDmxIndexInfo.u64GlobalOffset=sizelast;
                #if 1
                stDmxIndexInfo.u32DataTimeMs = (timelast-pRecChn->First_RawTime+pRecChn->IndexHandle->Last_DisplayTimeMs+40);
                #else
                stDmxIndexInfo.u32DataTimeMs=timelast;
                #endif
                sizelast=align_size;
                timelast=timenow;
                //pRecChn->IndexHandle->u32DmxClkTimeMs = stDmxIndexInfo.u32PtsMs;
                //pRecChn->IndexHandle->stCycMgr.u64AllRecNowInMs = pvr_get_systime();
                PVR_Index_SaveFramePosition(pRecChn->u32ChnID, pRecChn->IndexHandle->u32RecPicParser, &stDmxIndexInfo, 0);
                //printf("+++index pts=%d,off=%llx,size=%x distime=%d\n",stDmxIndexInfo.u32PtsMs,stDmxIndexInfo.u64GlobalOffset,stDmxIndexInfo.u32FrameSize,stDmxIndexInfo.u32DataTimeMs);
              }
            }
            MT_USLEEP(20000);
        }
    }
    #ifdef CONFIG_MT_LXC_SUPPORT
    mt_pvr_ipc_clear_tid();
    #endif
    MT_INFO_PVR("<==PVRRecSaveIdxRoutine ret=%#x.\n", ret);
    return NULL;
}

STATIC MT_VOID * PVRRecGetIdxFromAvThread(MT_VOID *args)
{
    MT_S32 ret = MT_SUCCESS;

    PVR_REC_CHN_S *pRecChn = (PVR_REC_CHN_S *)args;
    char threadname[128]={0};
    FILE *fpDmxSc = NULL;

    strcpy(threadname,__FUNCTION__);
    mt_set_pthread_name((char *)threadname);
    while(MT_TRUE)
    {
        if((0==pRecChn->share_rec.flg_share) && (MT_UNF_PVR_REC_STATE_STOP == pRecChn->enState)){ //no share.stop
            break;
        }
        ret = PVRRecSaveIndex(fpDmxSc, pRecChn);
        if (!((MT_SUCCESS == ret) || (MT_ERR_DMX_NOAVAILABLE_DATA == ret) || (MT_ERR_DMX_TIMEOUT == ret)))
        {
            ;//MT_ERR_PVR("PVRRecSaveIndex error\n");
        }
    }
    #ifdef CONFIG_MT_LXC_SUPPORT
    mt_pvr_ipc_clear_tid();
    #endif

    MT_INFO_PVR("<==PVRRecSaveIdxRoutine ret=%#x.\n", ret);
    return NULL;
}
#endif
/*****************************************************************************
 Prototype       : PVRRecSaveStream
 Description     : save stream to fs
 Input           : pRecChn  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/11
    Author       : q46153
    Modification : Created function

*****************************************************************************/
#define TEST_DELAY_TIME_US 20
STATIC MT_VOID* PVRRecSaveStreamRoutine(MT_VOID *args)
{
    MT_S32 ret = MT_SUCCESS;
    MT_U32 u32OverflowTimes = 0;
    MT_UNF_DMX_REC_DATA_S stDmxTsData = {0};
//    MT_UNF_DMX_RECBUF_STATUS_S stRecBufStatus = {0};
    char threadname[128]={0};
    MT_UNF_DMX_REC_DATA_S crypto2;
    #ifdef TEST_DUAL_REC
    MT_UNF_DMX_REC_DATA_S crypto3;
    #endif
    PVR_REC_CHN_S *pRecChn = (PVR_REC_CHN_S *)args;
    MT_U32 gotdatalen=0;
    PVR_REC_CHN_S *tmprec_m=((PVR_REC_CHN_S *)args);
    PVR_REC_CHN_S *tmprec_s=NULL;
    MT_U32 fswrite_cell=(188*4);
    MT_U32 new_io_block=0;
    mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
    MT_UNF_DMX_RECBUF_STATUS_S recinfo={0};

    if(pRecChn == NULL)
    {
        return NULL;
    }

    if(pRecChn->stUserCfg.stEncryptCfg.bDoCipher
        && p_addon && p_addon->ts_cipher_size)
    {
        new_io_block = p_addon->ts_cipher_size;
    }

    strcpy(threadname,__FUNCTION__);
    mt_set_pthread_name((char *)threadname);

    if(MT_UNF_PVR_STREAM_TYPE_ALL_TS != pRecChn->stUserCfg.enStreamType){
        if(NULL == pRecChn->IndexHandle){
            return NULL;
        }
    }

    while (1)
    {
        {//check stopped
            tmprec_s=(PVR_REC_CHN_S *)tmprec_m->share_rec.Shared_recchptr;
            if(MT_UNF_PVR_REC_STATE_STOP == tmprec_m->enState){         //main stopped
                if(1==tmprec_m->share_rec.flg_share){
                    if(MT_UNF_PVR_REC_STATE_STOP == tmprec_s->enState){ //all stopped
                        break;
                    }else{
                        pRecChn=(PVR_REC_CHN_S *)tmprec_m->share_rec.Shared_recchptr;
                    }
                }else{
                    break;
                }
            }
            if(tmprec_s && (MT_UNF_PVR_REC_STATE_STOP == tmprec_s->enState)){
                tmprec_s->share_rec.can_stop=1;
                pRecChn=tmprec_m;
                continue;
            }
        }
        
        /* In the first time rewind by time case, wait for index global offset large than file size */
        if ((MT_UNF_PVR_STREAM_TYPE_ALL_TS != pRecChn->stUserCfg.enStreamType) &&
            (NULL != pRecChn->IndexHandle) &&
            (MT_TRUE == pRecChn->IndexHandle->bTimeRewindFlg) &&
            (PVR_INDEX_REWIND_BY_TIME == pRecChn->IndexHandle->stCycMgr.enRewindType))
        {
            if(MT_FALSE == pRecChn->IndexHandle->bTimeRewindTsPauseFlg)
            {  //record cut pos, go ahead!
                pRecChn->IndexHandle->u64TimeTsPausePos=pRecChn->u64CurFileSize;
                pRecChn->IndexHandle->bTimeRewindTsPauseFlg = MT_TRUE;
            }
        }

	 if(MT_UNF_PVR_REC_STATE_STOP  == pRecChn->timeshiftEventHandle.state || MT_UNF_PVR_REC_STATE_STOPPING == pRecChn->timeshiftEventHandle.state)
     {
		//MT_USLEEP(20000);
		MT_USLEEP(TEST_DELAY_TIME_US);
		//PVR_EVENT_D("%s====%d\n",__FUNCTION__,__LINE__);
		if(MT_UNF_PVR_REC_STATE_STOPPING != pRecChn->timeshiftEventHandle.state)
		{
		       pRecChn->timeshiftEventHandle.state  = MT_UNF_PVR_REC_STATE_STOPPING;
		       if(pRecChn->enState != MT_UNF_PVR_REC_STATE_STOPPING)
       		{
                         //PVR_EVENT_D("%s===ret=0x%x\n",__FUNCTION__,ret);
                         pRecChn->enState = MT_UNF_PVR_REC_STATE_STOPPING;
                         ret = PVRRecStopDemux(pRecChn);
                         if(ret != MT_SUCCESS)
                         {
                                  MT_ERR_PVR("MT_PVR_StopCache_Event_Rec >>> PVRRecStopDemux error !!\n");
                         }
                         //PVR_EVENT_D("%s===ret=0x%x\n",__FUNCTION__,ret);
                    }
              }
            	continue;
        }
        if((1==pRecChn->share_rec.flg_masterorslave)||
           ((0==pRecChn->share_rec.flg_masterorslave)&&(MT_UNF_PVR_REC_STATE_STOP==tmprec_m->enState))){  //master is default
            ret = MT_UNF_DMX_AcquireRecData(pRecChn->DemuxRecHandle, &stDmxTsData, 100);
        }else{
            ret = MT_SUCCESS;
        }

        if (MT_SUCCESS != ret) {
    	    if (MT_ERR_DMX_TIMEOUT == ret) {
                MT_USLEEP(20000);
    	    	continue;
    	    }

    	    if (MT_ERR_DMX_NOAVAILABLE_DATA == ret) {
                MT_USLEEP(20000);
        		continue;
    	    }
    	    MT_ERR_PVR("MT_UNF_DMX_AcquireRecData failed 0x%x\n", ret);
    	    break;
    	  }
      if(stDmxTsData.u32Len <= 0)
      {
          MT_USLEEP(20000);
          continue;
      }
      else
      {
          if(new_io_block && stDmxTsData.u32Len < new_io_block)
          {
              MT_USLEEP(50000);
              continue;
          }
          else if(pRecChn->stUserCfg.u32DIO && stDmxTsData.u32Len < (PVR_IOBLOCK))
          {
              MT_USLEEP(10000);
              continue;
          }
          else if(stDmxTsData.u32Len < fswrite_cell)
          {
              MT_USLEEP(20000);
             continue;
          }
      }

        #if 1
        memset(&recinfo, 0x00, sizeof(recinfo));
        if(MT_SUCCESS==MT_UNF_DMX_GetRecBufferStatus(pRecChn->DemuxRecHandle,&recinfo)){
            //PVR_REC_P("rec[%d] overflow info[%d,%d]\n",pRecChn->u32ChnID ,u32OverflowTimes,recinfo.overflowinfo.overflow_allcnt);
            if(recinfo.overflowinfo.overflow_allcnt != u32OverflowTimes)
            {
                PVR_REC_P("rec[%d] overflow[%d,%d] send disk slow event!\n",pRecChn->u32ChnID ,u32OverflowTimes,recinfo.overflowinfo.overflow_allcnt);
                u32OverflowTimes = recinfo.overflowinfo.overflow_allcnt;
                PVRRecPostEvent(pRecChn->u32ChnID, MT_UNF_PVR_EVENT_REC_DISK_SLOW, 0);
            }

        } 
      
        #else
        if (stDmxTsData.u32Len*10 > stDmxTsData.u32Size * 9){
            u32OverflowTimes++;
            if (u32OverflowTimes > 3){
                u32OverflowTimes = 0;
                //printf("+++used=%x,size=%x\n",stDmxTsData.u32Len,stDmxTsData.u32Size);
                PVRRecPostEvent(pRecChn->u32ChnID, MT_UNF_PVR_EVENT_REC_DISK_SLOW, 0);
            }
        }else{
            u32OverflowTimes = 0;
        }
        #endif
        PVR_EVENT_DD
        if (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState)
        {
            if ((MT_SUCCESS == ret) && (stDmxTsData.u32Len > 0))
            {
                //printf("++pause release0\n");
                (MT_VOID)MT_UNF_DMX_ReleaseRecData(pRecChn->DemuxRecHandle, &stDmxTsData);
            }
            MT_USLEEP(1000 * 10);
            continue;
        }
        gotdatalen=stDmxTsData.u32Len;
        MT_INFO_PVR("+++used=%x,size=%x\n",stDmxTsData.u32Len,stDmxTsData.u32Size);
        if(new_io_block) {
            stDmxTsData.u32Len -= (stDmxTsData.u32Len % new_io_block);
        }
        else if(pRecChn->stUserCfg.u32DIO){
//		    if(pRecChn->stUserCfg.stEncryptCfg.bDoCipher)
                stDmxTsData.u32Len=((stDmxTsData.u32Len>>PVR_IOBLOCKBITS)<<PVR_IOBLOCKBITS);
				if (stDmxTsData.u32Len > PVR_WRITE_TS_MAX_LEN)
				{
					stDmxTsData.u32Len = PVR_WRITE_TS_MAX_LEN;
				}
        }
        else{//16Byte align. for crypto
            stDmxTsData.u32Len=((stDmxTsData.u32Len/fswrite_cell)*fswrite_cell);
        }
        if ((MT_SUCCESS == ret) && (stDmxTsData.u32Len > 0))
        {
            memcpy(&crypto2,&stDmxTsData,sizeof(MT_UNF_DMX_REC_DATA_S));
            if(crypto2.u32Len)    //if crypto. write in MT_PVR_CipherRecCrypto.so don't rewrite
            {
                int i=0;

                if(pRecChn->timeshiftEventHandle.event_rec)
                {
					PVR_LOCK_REC(pRecChn);
                    for(i=0; i<pRecChn->timeshiftEventHandle.event_total_count; i++)
                    {
                        if((PVR_EVENT_START == pRecChn->timeshiftEventHandle.event_rec[i].eventState)
                            && (pRecChn->timeshiftEventHandle.event_rec[i].u32EventStartTime > pRecChn->u32RecStartTimeMs))
                        {
                            MT_BOOL  preEventIsSaved = MT_FALSE;
                            MT_U8 *   preEventName = NULL;
                            pRecChn->real_file_start_frame = 0;

                            if((pRecChn->timeshiftEventHandle.event_rec[i].eventType == TIMESHIFT_EVENT_STOP_START) && (i >= 0))
                            {
                                preEventIsSaved = pRecChn->timeshiftEventHandle.event_rec[i].bSaved;
                                preEventName= pRecChn->timeshiftEventHandle.event_rec[i].u8EventName;
                            }
                            PVR_RecNewEvent(pRecChn->dataFile,
                                                    (MT_CHAR*)pRecChn->timeshiftEventHandle.event_rec[i].u8EventName,
                                                    pRecChn->timeshiftEventHandle.event_rec[i].eventType,
                                                    pRecChn->timeshiftEventHandle.event_rec[i].u32EventStartTime,
                                                    preEventName,
                                                    preEventIsSaved);
                            MT_INFO_PVR("pRecChn->timeshiftEventHandle.event_rec.eventState =%d, pRecChn->timeshiftEventHandle.event_rec.u32EventStartTime =%d, pRecChn->u32RecStartTimeMs=%d\n",
                                                   pRecChn->timeshiftEventHandle.event_rec[i].eventState, pRecChn->timeshiftEventHandle.event_rec[i].u32EventStartTime, pRecChn->u32RecStartTimeMs);
                            if(pRecChn->timeshiftEventHandle.event_rec[i].eventType == TIMESHIFT_EVENT_STOP_START)
                            {
                                if(i>0)  pRecChn->timeshiftEventHandle.event_rec[i-1].eventState = PVR_EVENT_STOP_SAVE;
                            }
                            pRecChn->timeshiftEventHandle.event_rec[i].eventState=PVR_EVENT_RECING;

                        }
                        else  if(PVR_EVENT_STOP == pRecChn->timeshiftEventHandle.event_rec[i].eventState)
                        {
                             PVR_RecNewEvent(pRecChn->dataFile,
                                            NULL,
                                            pRecChn->timeshiftEventHandle.event_rec[i].eventType,
                                            pRecChn->timeshiftEventHandle.event_rec[i].u32EventStartTime,
                                            pRecChn->timeshiftEventHandle.event_rec[i].u8EventName,
                                            pRecChn->timeshiftEventHandle.event_rec[i].bSaved);
                             pRecChn->timeshiftEventHandle.event_rec[i].eventState = PVR_EVENT_STOP_SAVE;
                        }
                    }
					PVR_UNLOCK_REC(pRecChn);
                }
                ret = PVRRecCycWriteStream(&crypto2, pRecChn);
            }
            
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("size:%u, addr:%p, ret:%#x datafile %x chanid %d state %d\n", stDmxTsData.u32Len, stDmxTsData.pDataAddr, ret,pRecChn->dataFile,pRecChn->u32ChnID,pRecChn->enState);

                //perror("PVR Write stream failed: ");
                (MT_VOID)MT_UNF_DMX_ReleaseRecData(pRecChn->DemuxRecHandle, &stDmxTsData);
                if (MT_ERR_PVR_FILE_TILL_END == ret || MT_ERR_PVR_FILE_CANT_WRITE == ret || MT_ERR_PVR_FILE_DISC_FULL == ret )
                {
                    pRecChn->enState = MT_UNF_PVR_REC_STATE_STOP;
                }
                PVRRecCheckError(pRecChn, ret); //fixbug123971
                if(pRecChn->share_rec.flg_share && pRecChn->share_rec.flg_masterorslave) //master, need app to stop it. slave continue recording
                {
                    //tmprec_s=(PVR_REC_CHN_S *)pRecChn->share_rec.Shared_recchptr;
                    continue;
                }
                break;
            }
            if(NULL != pRecChn->IndexHandle){
                pRecChn->IndexHandle->recsize64 += stDmxTsData.u32Len;
            }

            {//check share!
                if(pRecChn->share_rec.flg_share && pRecChn->share_rec.flg_masterorslave){   //if share&&master . do salve
                    tmprec_s=(PVR_REC_CHN_S *)tmprec_m->share_rec.Shared_recchptr;
                    if(tmprec_s && (MT_UNF_PVR_REC_STATE_STOP != tmprec_s->enState) && (MT_UNF_PVR_REC_STATE_INVALID!= tmprec_s->enState)){       //deal slave
                        pRecChn=tmprec_s;
                        if(0==tmprec_s->share_rec.offset_inrecbuf){
                            tmprec_s->share_rec.offset_inrecbuf=(tmprec_m->IndexHandle->recsize64-stDmxTsData.u32Len);
                            //printf("+++rec offset=%llx,%lld\n",tmprec_s->share_rec.offset_inrecbuf,tmprec_s->share_rec.offset_inrecbuf);
                        }
                        continue;
                    }
                    if(tmprec_s && (MT_UNF_PVR_REC_STATE_STOP == tmprec_s->enState)){       //set flag
                        //printf("++normal set\n");
                        tmprec_s->share_rec.can_stop=1;
                    }
                }
                pRecChn=tmprec_m;
            }
            (MT_VOID)MT_UNF_DMX_ReleaseRecData(pRecChn->DemuxRecHandle, &stDmxTsData);
            if(pRecChn->stUserCfg.u32DIO){
                if(stDmxTsData.u32Size/2 > gotdatalen){//(0x100000 > gotdatalen){
                    MT_USLEEP(10000);     //release 10ms
                }
            }else{
                MT_USLEEP(50000);         //release 50ms
            }
        }
        else if ((MT_ERR_DMX_NOAVAILABLE_DATA == ret) || (MT_ERR_DMX_TIMEOUT == ret))
        {
            MT_USLEEP(10000);
            continue;
        }
        else
        {
            MT_ERR_PVR("receive rec stream error:%#x\n", ret);
            break;
        }

        PVRRecCheckRecPosition(pRecChn);
    } /* end while */

    if (!((MT_UNF_PVR_REC_STATE_STOP == pRecChn->enState) && (MT_ERR_DMX_TIMEOUT == ret)))
    {
        PVRRecCheckError(pRecChn, ret);
    }

    if (MT_UNF_PVR_REC_STATE_STOP != pRecChn->enState)
    {
        MT_INFO_PVR("-----PVRRecSaveStreamRoutine exiting with error:%#x...\n", ret);
    }

    MT_INFO_PVR("<==PVRRecSaveStreamRoutine,FileLen:0x%llx.\n", pRecChn->u64CurFileSize);
    #ifdef CONFIG_MT_LXC_SUPPORT
    mt_pvr_ipc_clear_tid();
    #endif
    return NULL;
}
/*
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
static void dumpdata(mt_u8 *head,mt_u32 lens)
{
    mt_u32 i;
    MT_INFO_PVR("data:\n");
    for(i=0;i<lens;i++){
        MT_INFO_PVR("%02x ",head[i]);
        if(7==(i&7)){
            MT_INFO_PVR("\n");
        }
    }
    MT_INFO_PVR("\n");
}

MT_S32 MT_PVR_CipherRecCreate(PVR_REC_CHN_S *pChnAttr,MT_CIPHER_CRYPTO_CH_E chid,mt_u32 enordec)
{//enordec,0:en 1:dec
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_cipher;
    MT_CIPHER_CTRL_S info;
    mt_u32 slot_id = 0;
    int i;
    if(2==pChnAttr->chiptype){
#ifndef CONFIG_MT_CHIP_SYMPHONY1
        slot_id = MT_CIPHER_KEYSLOT_INVALID;
        mt_unf_cipher_keyslot_request(&slot_id);

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));

        info.operation = enordec;
        info.algorithm = pChnAttr->stUserCfg.stEncryptCfg.enType;
        info.work_mode = MT_CIPHER_WORK_MODE_ECB;

        mt_unf_cipher_keyslot_set(slot_id, &info, pChnAttr->stUserCfg.stEncryptCfg.au8Key, NULL);    //clear-text key, no IV
        //printf("+p:%x,%x,%x\n",enordec,pChnAttr->stUserCfg.stEncryptCfg.enType,chid);
        //printf("key ");
        //dumpdata(pChnAttr->stUserCfg.stEncryptCfg.au8Key,16);

        ret = mt_unf_cipher_crypto_create(chid, &p_cipher);

        ret |= mt_unf_cipher_crypto_config(p_cipher, &info, slot_id);
        //printf("ret=%x,%x\n",ret,p_cipher);

        if(MT_SUCCESS==ret){
            pChnAttr->hCipher=p_cipher;
            pChnAttr->hCipher_slot=slot_id;
        }else{
            pChnAttr->hCipher=MT_INVALID_HANDLE;
            pChnAttr->hCipher_slot=MT_CIPHER_KEYSLOT_INVALID;
        }
#endif
    }
    return ret;
}

MT_S32 MT_PVR_CipherRecDestroy(PVR_REC_CHN_S *pChnAttr)
{
    if(2==pChnAttr->chiptype){
#ifndef CONFIG_MT_CHIP_SYMPHONY1
        if (pChnAttr->hCipher != MT_INVALID_HANDLE) {
            mt_unf_cipher_crypto_destroy(pChnAttr->hCipher);
            pChnAttr->hCipher = MT_INVALID_HANDLE;
        }
        if (pChnAttr->hCipher_slot != MT_CIPHER_KEYSLOT_INVALID) {
            mt_unf_cipher_keyslot_release(pChnAttr->hCipher_slot);
            pChnAttr->hCipher_slot = MT_CIPHER_KEYSLOT_INVALID;
        }
#endif
    }
    return MT_SUCCESS;
}
MT_S32 MT_PVR_CipherRecCrypto_sample(PVR_REC_CHN_S *pChnAttr,MT_U8 *in ,MT_U8 *out,MT_U32 len)
{//merge last-remain to aligned. crypto two times
    MT_S32 ret=0;
    ret|=MT_PVR_CipherRecCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_1,MT_CIPHER_OPERATION_ENCRYPT);
    ret|=mt_unf_cipher_crypto_process(pChnAttr->hCipher,in,out,len);
    //printf("+++outcphonyer=%x,%x\n",pChnAttr->hCipher,ret);
    ret|=MT_PVR_CipherRecDestroy(pChnAttr);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_CipherRecCrypto(PVR_REC_CHN_S *pChnAttr,MT_UNF_DMX_REC_DATA_S *in,MT_UNF_DMX_REC_DATA_S *ot1,MT_UNF_DMX_REC_DATA_S *ot2)
{
    #if 0

    //merge last-remain to aligned. crypto two times. 3parts:ot1\last remain. ot2\round16. \this remain.
    //ot2: be cut to 512*188. avid to src be change!!!!!!!!!! if that,index will change!!!!!!!!!!!!!!

    MT_U32 alginlen=16,cutfromthis=0;
    MT_U32 cryptooffi=0,cryptosize=0;
    MT_UNF_DMX_REC_DATA_S wrpart;
    MT_S32 ret=MT_SUCCESS,RET=MT_SUCCESS;
    if((NULL==in) || (NULL==ot1) || (NULL==ot2)){
        return MT_FAILURE;
    }
    ot1->u32Len=0;
    ot2->u32Len=0;
    if(2==pChnAttr->chiptype){
        if(MT_CIPHER_ALG_AES > pChnAttr->stUserCfg.stEncryptCfg.enType){
            alginlen=8;
        }else{
            alginlen=16;
        }
        cutfromthis=0;
        //printf("++1.remain=%x,len=%x\n",pChnAttr->hCipher_remainlen,in->u32Len);
        if(pChnAttr->hCipher_remainlen){
            cutfromthis=alginlen-pChnAttr->hCipher_remainlen;
            memcpy(pChnAttr->hCipher_remain+pChnAttr->hCipher_remainlen,in->pDataAddr,cutfromthis);
            pthread_mutex_lock(&g_pvrcrypto);
            ret=MT_PVR_CipherRecCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_1,MT_CIPHER_OPERATION_ENCRYPT);
            RET|=ret;
            ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,pChnAttr->hCipher_remain,pChnAttr->hCipher_output,alginlen);
            RET|=ret;
            MT_PVR_CipherRecDestroy(pChnAttr);
            pthread_mutex_unlock(&g_pvrcrypto);

            ot1->pDataAddr=pChnAttr->hCipher_output;
            ot1->u32Len=alginlen;
            //printf("++2.ot1.len=%x\n",ot1->u32Len);
            #if 0//write out of func
            #else//write in func
            ret = PVRRecCycWriteStream(*ot1, pChnAttr);
            RET|=ret;
            #endif
        }

        memcpy(ot2,in,sizeof(MT_UNF_DMX_REC_DATA_S));
        ot2->pDataAddr+=cutfromthis;
        ot2->u32Len=((in->u32Len-cutfromthis)/alginlen*alginlen);
        pChnAttr->hCipher_remainlen=(in->u32Len-cutfromthis-ot2->u32Len);
        //printf("++3.ot2.len=%x,remain=%x\n",ot2->u32Len,in->u32Len-cutfromthis-ot2->u32Len);
        if(pChnAttr->hCipher_remainlen){
            memcpy(pChnAttr->hCipher_remain,ot2->pDataAddr+ot2->u32Len,pChnAttr->hCipher_remainlen);
        }
        #if 0//write out of func
        if(ot2->u32Len){
            ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,ot2->pDataAddr,ot2->pDataAddr,ot2->u32Len);
            RET|=ret;
        }
        #else//write in func
        while(cryptooffi<ot2->u32Len){
            if((cryptooffi+CRPTOBFSIZE)>ot2->u32Len){
                cryptosize=ot2->u32Len-cryptooffi;
            }else{
                cryptosize=CRPTOBFSIZE;
            }
            pthread_mutex_lock(&g_pvrcrypto);
            ret=MT_PVR_CipherRecCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_1,MT_CIPHER_OPERATION_ENCRYPT);
            ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,ot2->pDataAddr+cryptooffi,pChnAttr->hCipher_buffer,cryptosize);
            RET|=ret;
            MT_PVR_CipherRecDestroy(pChnAttr);
            pthread_mutex_unlock(&g_pvrcrypto);
            cryptooffi+=cryptosize;
            wrpart.pDataAddr=pChnAttr->hCipher_buffer;
            wrpart.u32Len=cryptosize;
            ret = PVRRecCycWriteStream(wrpart, pChnAttr);
            RET|=ret;
            //printf("++crypt.sz=%x,%x\n",cryptosize,cryptosize&0xf);
        }
        ot2->u32Len=0;
        #endif
    }
    return RET;
    #else
    int fd;
    MT_U8 *cryptobuff=NULL;
    mt_u32 offset=0,celllen=0;
    MT_UNF_DMX_REC_DATA_S wrpart;
    MT_S32 ret=MT_SUCCESS,RET=MT_SUCCESS;
    mt_u32 time_last=0,time_now=0;
    if((NULL==in) || (NULL==ot1) || (NULL==ot2)){
        return MT_FAILURE;
    }
    #ifdef CONFIG_MT_LXC_SUPPORT
    fd=PVR_GetTsFileFd(pChnAttr->dataFile);
    if(0<=fd){
        cryptobuff=mt_pvr_ipc_getbufferby_tid(fd,MAX_CRYPTOSZ_W);
    }
    #else
    cryptobuff=pChnAttr->hCipher_buffer;
    #endif
    if(NULL==cryptobuff){
        return MT_FAILURE;
    }
    memcpy(ot2,in,sizeof(MT_UNF_DMX_REC_DATA_S));
    if(2==pChnAttr->chiptype){
        //printf("+++++++++++++++++++++++++++crypto start.w=%x\n",ot2->u32Len);
         MT_PVR_SysGetTimeStampMs(&time_last);
         while(ot2->u32Len){
            if(MAX_CRYPTOSZ_W<=ot2->u32Len){
                celllen=MAX_CRYPTOSZ_W;
            }else{
                celllen=ot2->u32Len;
            }

            pthread_mutex_lock(&g_pvrcrypto);
            ret=MT_PVR_CipherRecCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_1,MT_CIPHER_OPERATION_ENCRYPT);
            RET|=ret;
            ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,ot2->pDataAddr+offset,cryptobuff,celllen);
            RET|=ret;
            MT_PVR_CipherRecDestroy(pChnAttr);
            pthread_mutex_unlock(&g_pvrcrypto);

            offset+=celllen;
            wrpart.pDataAddr=cryptobuff;
            wrpart.u32Len=celllen;
            //printf("+++write.w2=%x\n",celllen);
            ret = PVRRecCycWriteStream(wrpart, pChnAttr);
            //printf("+++write.w3=%x\n",celllen);
            RET|=ret;
            ot2->u32Len-=celllen;
            MT_PVR_SysGetTimeStampMs(&time_now);
            if((time_now > time_last) && 30<=(time_now-time_last)){
                MT_USLEEP(1000*4);
                time_last=time_now;
                //printf("++sleep\n");
            }
        }
        //printf("+++++++++++++++++++++++++++crypto over.w\n");
    }
    return RET;
    #endif
}
#endif
*/
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
STATIC INLINE MT_S32 PVRRecPrepareCipher(PVR_REC_CHN_S *pChnAttr)
{
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_PVR_CIPHER_S *pCipherCfg = &(pChnAttr->stUserCfg.stEncryptCfg);
    if (!pCipherCfg->bDoCipher){
        return MT_SUCCESS;
    }
    pChnAttr->hCipher_buffer=NULL;
    #ifndef CONFIG_MT_LXC_SUPPORT
    if(pChnAttr->stUserCfg.stEncryptCfg.bDoCipher){
        pChnAttr->hsec_mmz_rec.bufsize=MAX_CRYPTOSZ_W;
        ret = mt_mmz_malloc(&pChnAttr->hsec_mmz_rec);
        if(ret){
            pChnAttr->hCipher_buffer=NULL;
            return MT_FAILURE;
        }
        pChnAttr->hCipher_buffer=pChnAttr->hsec_mmz_rec.user_viraddr;
    }
    #endif
    return MT_SUCCESS;
    //return MT_PVR_CipherRecCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,0);
}

STATIC INLINE MT_S32 PVRRecReleaseCipher(PVR_REC_CHN_S  *pChnAttr)
{
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_PVR_CIPHER_S *pCipherCfg = &(pChnAttr->stUserCfg.stEncryptCfg);
    #ifndef CONFIG_MT_LXC_SUPPORT
    if(pChnAttr->hCipher_buffer){
        mt_mmz_free(&pChnAttr->hsec_mmz_rec);
        pChnAttr->hCipher_buffer=NULL;
    }
    #endif
    return MT_SUCCESS;
}
#else //sym4
STATIC INLINE MT_S32 PVRRecPrepareCipher(PVR_REC_CHN_S *pChnAttr)
{
//    MT_S32 ret = MT_SUCCESS;
    MT_UNF_PVR_CIPHER_S *pCipherCfg = &(pChnAttr->stUserCfg.stEncryptCfg);
    if (!pCipherCfg->bDoCipher){
        return MT_SUCCESS;
    }
    pChnAttr->hCipher_buffer=NULL;
    #ifndef CONFIG_MT_LXC_SUPPORT
    if((MT_UNF_PVR_REC_VMX != pChnAttr->stUserCfg.bSupportAdvCa) && (MT_UNF_PVR_REC_COMMON_CRAMBLE != pChnAttr->stUserCfg.bSupportAdvCa)){
        mt_u32 phyaddr = 0;
        phyaddr = (mt_u32)mt_mmz_new(MAX_CRYPTOSZ_W, 4096, NULL,"pvr");
        if(!phyaddr)
            return MT_FAILURE;

        pChnAttr->hsec_mmz_rec.phyaddr=phyaddr;
        pChnAttr->hCipher_buffer=(MT_U8 *)mt_mmz_map((mt_u32)phyaddr, 0);
    }
    #endif
    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRRecReleaseCipher(PVR_REC_CHN_S  *pChnAttr)
{
//    MT_S32 ret = MT_SUCCESS;
//    MT_UNF_PVR_CIPHER_S *pCipherCfg = &(pChnAttr->stUserCfg.stEncryptCfg);
    #ifndef CONFIG_MT_LXC_SUPPORT
    if(pChnAttr->hCipher_buffer){
        if((MT_UNF_PVR_REC_VMX != pChnAttr->stUserCfg.bSupportAdvCa) && (MT_UNF_PVR_REC_COMMON_CRAMBLE != pChnAttr->stUserCfg.bSupportAdvCa)){
            //mt_mmz_unmap((void*)pChnAttr->hsec_mmz_rec.user_viraddr);
            mt_mmz_unmap((void*)pChnAttr->hCipher_buffer);//user_viraddr have not config,fixed unmap failed
            mt_mmz_delete(pChnAttr->hsec_mmz_rec.phyaddr);
        }
        pChnAttr->hCipher_buffer=NULL;
    }
    #endif
    return MT_SUCCESS;
}
#endif
PVR_REC_CHN_S* PVRRecGetChnAttrByName(const MT_CHAR *pFileName)
{
    MT_U32 i = 0;

    if(NULL == pFileName)
    {
        MT_ERR_PVR("File name point is NULL.\n");
        return NULL;
    }

    for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        if  (!strncmp(g_stPvrRecChns[i].stUserCfg.szFileName, pFileName, strlen(pFileName)) )
        {
        	if ((g_stPvrRecChns[i].enState >= MT_UNF_PVR_REC_STATE_RUNNING) ||
        		(g_stPvrRecChns[i].enState <= MT_UNF_PVR_REC_STATE_STOP))
        	{
            	return &g_stPvrRecChns[i];
            }
            else
            {
            	break;
            }
        }
    }

    return NULL;
}

static MT_S32 PVRRecStartDemux(PVR_REC_CHN_S *pRecChn, MT_UNF_PVR_STREAM_TYPE_E enRecType, const MT_UNF_PVR_REC_ATTR_S *pUserCfg, MT_BOOL bIsClearStream)
{
    MT_S32 ret = 0;
    //mt_handle ChanHandle[8];
    mt_u32 ChanCount = 0;
    MT_UNF_DMX_REC_ATTR_S stRecAttr;
    mt_u32 i;

    memset(&stRecAttr, 0 , sizeof(MT_UNF_DMX_REC_ATTR_S));
    stRecAttr.u32DmxId        = pUserCfg->u32DemuxID;
    stRecAttr.u32RecBufSize   = pUserCfg->u32DavBufSize;
    stRecAttr.enVCodecType    = pUserCfg->enIndexVidType;
    stRecAttr.u32IndexSrcPid  = pUserCfg->u32IndexPid;

#if 0
    if (MT_UNF_PVR_STREAM_TYPE_ALL_TS == enRecType)
    {
        stRecAttr.enRecType   = MT_UNF_DMX_REC_TYPE_ALL_PID;
    }
    else
    {
        stRecAttr.enRecType   = MT_UNF_DMX_REC_TYPE_SELECT_PID;
    }

    if (MT_TRUE == bIsClearStream)
    {
        stRecAttr.bDescramed  = MT_TRUE;
    }
    else
    {
        stRecAttr.bDescramed  = MT_FALSE;
    }
#else
    if (MT_UNF_PVR_STREAM_TYPE_ALL_TS == enRecType)
    {
        stRecAttr.enRecType   = MT_UNF_DMX_REC_TYPE_ALL_PID;
        
        PVR_REC_D("-----pUserCfg enTsRecMode=%d\n",pUserCfg->enTsRecMode);
        if(pUserCfg->enTsRecMode == MT_UNF_PVR_REC_FULL_TS_WITH_NULL_PACKET)
        {
            stRecAttr.type_mode = DMX_FULL_TS_WITH_NULL_PACKET; // default with NULL packet
        }
        else if(pUserCfg->enTsRecMode == MT_UNF_PVR_REC_FULL_TS_WITHOUT_NULL_PACKET)
        {
            stRecAttr.type_mode = DMX_FULL_TS_WITHOUT_NULL_PACKET;
        }
        else
        {
            MT_ERR_PVR("enTsRecMode=%d invalid, use defalut rec mode\n",pUserCfg->enTsRecMode);
        }
    }
    else
    {
        stRecAttr.enRecType   = MT_UNF_DMX_REC_TYPE_SELECT_PID;
        stRecAttr.type_mode = DMX_PARTIAL_TS_PACKET;
    }

    if (MT_TRUE == bIsClearStream)
    {
        stRecAttr.bDescramed  = MT_TRUE;
        if(MT_UNF_PVR_STREAM_TYPE_ALL_TS != enRecType)
        stRecAttr.type_mode = DMX_PARTIAL_TS_PACKET_DESCRAMBLE;
    }
    else
    {
        stRecAttr.bDescramed  = MT_FALSE;
    }
#endif

    switch (pUserCfg->enIndexType)
    {
        case MT_UNF_PVR_REC_INDEX_TYPE_VIDEO :
            stRecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_VIDEO;
            break;

        case MT_UNF_PVR_REC_INDEX_TYPE_AUDIO :
            stRecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_AUDIO;
            break;

        case MT_UNF_PVR_REC_INDEX_TYPE_NONE :
        default :
            stRecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
    }
    stRecAttr.AlignToPageBlock=MT_FALSE;	/* always map ddr with vm_insert_page, no need keep align with page block */
    MT_INFO_PVR("~~~~~~~~~~~~dmxId:[%d],vType:[%d],recType:[%d],bDescramed:[%d]\n",stRecAttr.u32DmxId,(mt_u32)stRecAttr.enVCodecType,(mt_u32)stRecAttr.enRecType,stRecAttr.bDescramed);
    MT_INFO_PVR("~~~~~~~~~~~~bufSize:[%d],srcIdxPid:[%d],recMod:[%d],IdxType[%d]\n",stRecAttr.u32RecBufSize,stRecAttr.u32IndexSrcPid,(mt_u32)stRecAttr.type_mode,(mt_u32)stRecAttr.enIndexType);
    //MT_INFO_PVR("~~~~~~~~~~~~FilterEn:[%d],sc_filt:[%llx]\n",(int)stRecAttr.filt_en,*((mt_u64*)stRecAttr.start_code_filter));
    ret = MT_UNF_DMX_CreateRecChn(&stRecAttr, &(pRecChn->DemuxRecHandle));
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Create demux rec channel fail, ret=0x%x\n", ret);
        return ret;
    }
    PVR_REC_D("pvr create dmx rec channel=0x%lx evnet start\n",pRecChn->DemuxRecHandle);
    #if 0
    PVRRecPostEvent(pRecChn->u32ChnID, MT_UNF_PVR_EVENT_REC_DMX_CREATE, pRecChn->DemuxRecHandle);
    #else
    //fixed event may not in time, call cb directly
    PVRIntfEventDirectCall(pRecChn->u32ChnID,MT_UNF_PVR_EVENT_REC_DMX_CREATE,(MT_S32)pRecChn->DemuxRecHandle);
    #endif
    PVR_REC_D("pvr create dmx rec channel evnet finish\n");
    
    ret = MT_UNF_DMX_StartRecChn(pRecChn->DemuxRecHandle);
    if (MT_SUCCESS != ret)
    {//fixbug110170
        MT_ERR_PVR("Start demux rec channel failed, ret=0x%x\n", ret);
        (void)MT_UNF_DMX_DestroyRecChn(pRecChn->DemuxRecHandle);
        return ret;
    }
    {//create & start index
        MT_UNF_PVR_REC_ATTR_S *pstRecAttr=(MT_UNF_PVR_REC_ATTR_S *)&pRecChn->stUserCfg;
        if(MT_UNF_PVR_STREAM_TYPE_ALL_TS != pstRecAttr->enStreamType){
            #ifdef PVR_DEALIDX_IN_AV
            PVRINDEXCFG_S idxcfg={0};
            MT_U32 rsize=0;
            if(pstRecAttr->u32IdxBufSize % sizeof(MT_UNF_DMX_REC_INDEX_S)){
                rsize = pstRecAttr->u32IdxBufSize + sizeof(MT_UNF_DMX_REC_INDEX_S) - (pstRecAttr->u32IdxBufSize % sizeof(MT_UNF_DMX_REC_INDEX_S));
                MT_INFO_PVR("pstRecAttr->u32IdxBufSize=%x, sizeof(MT_UNF_DMX_REC_INDEX_S)=%x, rsize=%d\n",pstRecAttr->u32IdxBufSize,sizeof(MT_UNF_DMX_REC_INDEX_S),rsize);
            }else{
                rsize = pstRecAttr->u32IdxBufSize;
                MT_INFO_PVR("rsize=%x\n",rsize);
            }
            MT_INFO_PVR("\n:ss=%d,size=%d\n",sizeof(MT_UNF_DMX_REC_INDEX_S),rsize);
            ret=ioctl(g_s32PvrFd, CMD_PVR_REC_CREATE_IDX_SHM, (ulong)&rsize);
            if(ret < 0)
            {
                MT_FATAL_PVR("+++create idx ret=%x!\n",ret);
                return ret;
            }
            pRecChn->IndexHandle->idx_chn_num_avap = ret;
            idxcfg.idxid=pRecChn->IndexHandle->idx_chn_num_avap;
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /* For SYMPHONY 6, the index channel of the hardware was allocated dynamically
             * in the demux driver, and it's not the same as the demux id. */
            idxcfg.dmxid = pRecChn->DemuxRecHandle&0xFF; 
#else
            idxcfg.dmxid = pstRecAttr->u32DemuxID;
#endif
            idxcfg.vdectype=pstRecAttr->enIndexVidType;
            if(MT_SUCCESS != (ret = ioctl(g_s32PvrFd, CMD_PVR_REC_CONFIG_IDX_SHM,(ulong)&idxcfg)))
            {
                MT_FATAL_PVR("error config idx shm!\n");
                return ret;
            }
            #endif
        }
    }

#if 1
    for(i=0;i<PVR_REC_MAX_PID;i++){
        if(pRecChn->rec_pid[i].status == PVR_REC_PID_PRE_ADD){
            if(pRecChn->rec_pid[i].DmxRecId != stRecAttr.u32DmxId){
                MT_ERR_PVR("set error dmx id:attr id=%d, rec_pid[%d]=%d\n",stRecAttr.u32DmxId, i, pRecChn->rec_pid[i].DmxRecId);
                continue;
            }
            MT_INFO_PVR("\npid[%d]=%d\n",i,pRecChn->rec_pid[i].pid);
            ret = MT_UNF_DMX_AddRecPid(pRecChn->DemuxRecHandle, pRecChn->rec_pid[i].pid, &pRecChn->rec_pidhandle[ChanCount]);
            if (MT_SUCCESS != ret)
            {
            	MT_ERR_PVR("MT_UNF_DMX_AddRecPid failed 0x%x\n",ret);
                return ret;
            }
            pRecChn->rec_pid[i].status = PVR_REC_PID_ADDED;
            ++ChanCount;
        }
    }
    pRecChn->rec_handles=(MT_S32)ChanCount;
#endif

    return ret;
}

static MT_S32 PVRRecStopDemux(PVR_REC_CHN_S *pRecChn)
{
    MT_S32 i,ret;
#ifdef PVR_DEALIDX_IN_AV
    { //destory index
        MT_UNF_PVR_REC_ATTR_S *pstRecAttr=(MT_UNF_PVR_REC_ATTR_S *)&pRecChn->stUserCfg;
        if(MT_UNF_PVR_STREAM_TYPE_ALL_TS != pstRecAttr->enStreamType){
            MT_U32 tmp=pRecChn->IndexHandle->idx_chn_num_avap;
            if(tmp < 4){
                ioctl(g_s32PvrFd, CMD_PVR_REC_DESTROY_IDX_SHM, (ulong)&tmp);
            }
        }
    }
#endif

    ret = MT_UNF_DMX_StopRecChn(pRecChn->DemuxRecHandle);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Stop demux rec channel failed, ret=0x%x\n", ret);
        return ret;
    }
#if 1
    for (i = 0; i < pRecChn->rec_handles; i++)
    {
        ret = MT_UNF_DMX_DelRecPid(pRecChn->DemuxRecHandle, pRecChn->rec_pidhandle[i]);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("=================MT_UNF_DMX_DelRecPid error=================\n");
        }
        pRecChn->rec_pid[i].status = PVR_REC_PID_INIT;
    }
#endif
    ret = MT_UNF_DMX_DestroyRecChn(pRecChn->DemuxRecHandle);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Destroy demux rec channel failed, ret=0x%x\n", ret);
        return ret;
    }

    return ret;
}

#ifdef PVR_PROC_SUPPORT
static MT_S32 PVRRecShowProc(MT_PROC_SHOW_BUFFER_S * pstBuf, MT_VOID *pPrivData)
{
    MT_U32 i=0;
    MT_U32 u32VidType=0;
    PVR_REC_CHN_S *pChnAttr = g_stPvrRecChns;
    MT_S8 pStreamType[][32] = {"MPEG2", "MPEG4 DIVX4 DIVX5", "AVS", "H263", "H264",
                             "REAL8", "REAL9", "VC-1", "VP6", "VP6F", "VP6A", "MJPEG",
                             "SORENSON SPARK", "DIVX3", "RAW", "JPEG", "VP8", "MSMPEG4V1",
                             "MSMPEG4V2", "MSVIDEO1", "WMV1", "WMV2", "RV10", "RV20",
                             "SVQ1", "SVQ3", "H261", "VP3", "VP5", "CINEPAK", "INDEO2",
                             "INDEO3", "INDEO4", "INDEO5", "MJPEGB", "MVC", "HEVC", "DV", "INVALID"};

    if((NULL==pstBuf)||((NULL==pstBuf->pu8Buf))){
        MT_INFO_PVR("show proc.pvr.rec err\n");
        return MT_SUCCESS;
    }else{
        MT_INFO_PVR("proc.pvr.rec size=%x\n",pstBuf->size);
    }
    pstBuf->pu8Buf[0]=0;
    snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\n---------Montage PVR Recording channel Info---------\n");

    for(i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        if ((pChnAttr[i].enState != MT_UNF_PVR_REC_STATE_INVALID) &&
            //(pChnAttr[i].enState != MT_UNF_PVR_REC_STATE_STOPPING) &&
            (pChnAttr[i].enState != MT_UNF_PVR_REC_STATE_STOP) &&
            (pChnAttr[i].enState != MT_UNF_PVR_REC_STATE_BUTT))
        {
            if(NULL!=pChnAttr[i].IndexHandle){
                u32VidType = (MT_U32)PVR_Index_GetVtype(pChnAttr[i].IndexHandle)-100;
                u32VidType = (u32VidType > MT_UNF_VCODEC_TYPE_BUTT) ? MT_UNF_VCODEC_TYPE_BUTT : u32VidType;
            }
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "chan %d infomation:\n", i);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRec filename    :%s\n", pChnAttr[i].stUserCfg.szFileName);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tStream type     :%s\n", pStreamType[u32VidType]);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tDemuxID         :%d\n", pChnAttr[i].stUserCfg.u32DemuxID);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tState(1:init,running,pause,stopping,stop)    :%d\n", pChnAttr[i].enState);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tUsrSetRewind      :%d\n", pChnAttr[i].stUserCfg.bRewind);

            if(NULL!=pChnAttr[i].IndexHandle){
                if (pChnAttr[i].stUserCfg.bRewind)
                {
                    if(PVR_INDEX_REWIND_BY_BOTH == pChnAttr[i].IndexHandle->stCycMgr.enRewindType){
                        snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewind Type     :%s\n", "BOTH");
                    }else if(PVR_INDEX_REWIND_BY_TIME == pChnAttr[i].IndexHandle->stCycMgr.enRewindType)
                    {
                        snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewind Type     :%s\n", "TIME");
                        snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewind time     :%lld\n", pChnAttr[i].IndexHandle->stCycMgr.u64MaxCycTimeInMs);
                    }
                    else
                    {
                        snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewind Type     :%s\n", "SIZE");
                        snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewind size     :%#llx\n", pChnAttr[i].IndexHandle->stCycMgr.u64MaxCycSize);
                    }

                    snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewind times    :%d\n", pChnAttr[i].IndexHandle->stCycMgr.s32CycTimes);
                }
            }
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tMax size        :%#llx\n", pChnAttr[i].stUserCfg.u64MaxFileSize);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tMax time        :%#llx\n", pChnAttr[i].stUserCfg.u64MaxTimeInMs);
            //snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf),pstBuf->u32Size-strlen(pstBuf->pu8Buf), "\tUserData size   :%d\n", pChnAttr[i].stUserCfg.u32UsrDataInfoSize);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tClearStream     :%d\n", pChnAttr[i].stUserCfg.bIsClearStream);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tIndexType       :%d\n", pChnAttr[i].stUserCfg.enIndexType);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tIndexPid        :%#x/%d\n", pChnAttr[i].stUserCfg.u32IndexPid, pChnAttr[i].stUserCfg.u32IndexPid);
            if(NULL!=pChnAttr[i].IndexHandle){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tGlobal offset   :%#llx\n", pChnAttr[i].IndexHandle->stCurRecFrame.u64GlobalOffset);
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tFile offset     :%#llx\n", pChnAttr[i].IndexHandle->stCurRecFrame.u64Offset);
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tIndex Write     :%d\n", pChnAttr[i].IndexHandle->u32WriteFrame);
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tCurrentTime(ms) :%#llx\n", (pChnAttr[i].IndexHandle->stCycMgr.u64AllRecNowInMs-pChnAttr[i].IndexHandle->stCycMgr.u64AllRecStartInMs));
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tRewindFlg(ms)   :%#x\n", pChnAttr[i].IndexHandle->bTimeRewindFlg);
            }
            if(NULL!=pChnAttr[i].IndexHandle){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tmagic      :%x,%x,%x,%x,%x\n",
                  pChnAttr[i].u32magic1,pChnAttr[i].u32magic2,pChnAttr[i].IndexHandle->u32magic1,pChnAttr[i].IndexHandle->u32magic2,pChnAttr[i].IndexHandle->u32magic3);
                if(pChnAttr[i].IndexHandle->u32magic1!=0xaa55aa55){
                    snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tidxlock1 line   :%d\n", pChnAttr[i].IndexHandle->line_magc1);
                }
                if(pChnAttr[i].IndexHandle->u32magic2!=0xaa55aa55){
                    snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tidxlock2 line   :%d\n", pChnAttr[i].IndexHandle->line_magc2);
                }
                if(pChnAttr[i].IndexHandle->u32magic3!=0xaa55aa55){
                    snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tidxlock3 line   :%d\n", pChnAttr[i].IndexHandle->line_magc3);
                }
            }else{
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tmagic      :%x,%x\n", pChnAttr[i].u32magic1,pChnAttr[i].u32magic2);
            }
            if(pChnAttr[i].u32magic1!=0xaa55aa55){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\treclock1 line   :%d\n", pChnAttr[i].line_magc1);
            }
            if(pChnAttr[i].u32magic2!=0xaa55aa55){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\treclock2 line   :%d\n", pChnAttr[i].line_magc2);
            }
        }
    }//300359

    return MT_SUCCESS;
}
#endif



/* return TRUE just only start record*/
MT_BOOL PVR_Rec_IsFileSaving(const MT_CHAR *pFileName)
{
    MT_U32 i;

    if (NULL == pFileName)
    {
        MT_PRINT("\nInput pointer parameter is NULL!\n");
        return MT_FALSE;
    }

    for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        //if (g_stPvrRecChns[i].bSavingData)
        {
            if (!strncmp((const char *)g_stPvrRecChns[i].stUserCfg.szFileName, (const char *)pFileName,strlen(pFileName)))
            {
            	if ((g_stPvrRecChns[i].enState == MT_UNF_PVR_REC_STATE_RUNNING) ||
            		(g_stPvrRecChns[i].enState == MT_UNF_PVR_REC_STATE_PAUSE))
            	{
            		return MT_TRUE;
            	}
                else
                {
                	break;
                }
            }
        }
    }

    return MT_FALSE;
}

/*****************************************************************************
 Prototype       : PVR_Rec_IsChnRecording
 Description     : to check if record channel is recording
 Input           : u32ChnID  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/30
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_BOOL PVR_Rec_IsChnRecording(MT_U32 u32ChnID)
{
    PVR_REC_CHN_S  *pRecChn = MT_NULL;

    if ((u32ChnID < PVR_REC_START_NUM) || (u32ChnID >= PVR_REC_MAX_CHN_NUM + PVR_REC_START_NUM))
    {
        return MT_FALSE;
    }

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);

    if (MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}

MT_BOOL PVR_Rec_IsRecording(void)
{
    return g_stRecInit.bInit;
}


/*****************************************************************************
 Prototype       : PVR_Rec_MarkPausePos
 Description     : mark a flag for timeshift, and save the current record position
                        if start timeshift, playing from this position
 Input           : u32ChnID
 Output          : None
 Return Value    :
  History
  1.Date         : 2010/06/02
    Author       : j40671
    Modification : Created function

*****************************************************************************/
MT_S32 PVR_Rec_MarkPausePos(MT_U32 u32ChnID)
{
    PVR_REC_CHN_S  *pRecChn = MT_NULL;

    CHECK_REC_CHNID(u32ChnID);
    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);

    return PVR_Index_MarkPausePos(pRecChn->IndexHandle);
}
MT_S32 MT_PVR_SetIndexThreadattr(MT_S32 type,MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize)
{
    if(0==type){        //index read
        g_indexreadthreadattr.schedpolicy=schedpolicy;
        g_indexreadthreadattr.priority=priority;
        g_indexreadthreadattr.stacksize=stacksize;
        g_indexreadthreadattr.flag_valid=1;
    }else if(1==type){  //index write
        g_indexwritethreadattr.schedpolicy=schedpolicy;
        g_indexwritethreadattr.priority=priority;
        g_indexwritethreadattr.stacksize=stacksize;
        g_indexwritethreadattr.flag_valid=1;
    }else if(2==type){  //ts write
        g_tswritethreadattr.schedpolicy=schedpolicy;
        g_tswritethreadattr.priority=priority;
        g_tswritethreadattr.stacksize=stacksize;
        g_tswritethreadattr.flag_valid=1;
    }
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecInit
 Description     : init record module
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecInit(MT_VOID)
{
    MT_U32 i,j;
    MT_S32 ret;
#ifdef PVR_PROC_SUPPORT
    //MT_U32 u32CurPid = getpid();
    static MT_CHAR pProcDirName[32] = {0};
#endif

    if (MT_TRUE == g_stRecInit.bInit)
    {
        MT_WARN_PVR("Record Module has been Initialized!\n");
        return MT_SUCCESS;
    }
    else
    {
        /* initialize all the index */
        PVR_Index_Init();

        ret = PVRRecDevInit();
        if (MT_SUCCESS != ret)
        {
            return ret;
        }

        ret = PVRIntfInitEvent();
        if (MT_SUCCESS != ret)
        {
            return ret;
        }

        /* set all record channel as INVALID status                            */
        for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
        {
            if (-1 == pthread_mutex_init(&(g_stPvrRecChns[i].stMutex_valid), NULL))
            {
                for(j = 0; j < PVR_REC_MAX_CHN_NUM; j++)
                {
                    (void)pthread_mutex_destroy(&(g_stPvrRecChns[j].stMutex_valid));
                }
                PVRIntfDeInitEvent();

                /* TODO: destroy mutex **/
                MT_ERR_PVR("init mutex lock for PVR rec chn%d failed \n", i);
                return MT_FAILURE;
            }
            if (-1 == pthread_mutex_init(&(g_stPvrRecChns[i].stMutex), NULL))
            {
                for(j = 0; j < PVR_REC_MAX_CHN_NUM; j++)
                {
                    (void)pthread_mutex_destroy(&(g_stPvrRecChns[j].stMutex_valid));
                }
                for(j = 0; j < PVR_REC_MAX_CHN_NUM; j++)
                {
                    (void)pthread_mutex_destroy(&(g_stPvrRecChns[j].stMutex));
                }

                PVRIntfDeInitEvent();

                /* TODO: destroy mutex **/
                MT_ERR_PVR("init mutex lock for PVR rec chn%d failed \n", i);
                return MT_FAILURE;
            }

            PVR_LOCK_REC(((PVR_REC_CHN_S*)(&g_stPvrRecChns[i])));//PVR_LOCK(&(g_stPvrRecChns[i].stMutex));
            g_stPvrRecChns[i].enState  = MT_UNF_PVR_REC_STATE_INVALID;
            g_stPvrRecChns[i].u32ChnID = i + PVR_REC_START_NUM;
            g_stPvrRecChns[i].hCipher = 0;
            g_stPvrRecChns[i].writeCallBack = NULL;
            PVR_UNLOCK_REC(((PVR_REC_CHN_S*)(&g_stPvrRecChns[i])));//PVR_UNLOCK(&(g_stPvrRecChns[i].stMutex));
        }

#ifdef PVR_PROC_SUPPORT
        memset(pProcDirName, 0, sizeof(pProcDirName));
        strcpy(pProcDirName,"msp");
        if (!PVR_Play_IsPlaying())
        {
            MT_INFO_PVR("+++register rec\n");
            ret = mt_module_register(MT_ID_PVR, PVR_USR_PROC_DIR);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("MT_MODULE_Register(\"%s\") return %d\n", PVR_USR_PROC_DIR, ret);
            }

            /* Add proc dir */
            //snprintf(pProcDirName, sizeof(pProcDirName), "%s_%d", PVR_USR_PROC_DIR, u32CurPid);
            ret = mt_proc_add_dir(pProcDirName);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("MT_PROC_AddDir(\"%s\") return %d\n", PVR_USR_PROC_DIR, ret);
            }
        }
        MT_INFO_PVR("+++dirname=%s\n",pProcDirName);
        /* Will be added at /proc/hisi/${DIRNAME} directory */
        g_stPvrRecProcEntry.pszDirectory = pProcDirName;
        g_stPvrRecProcEntry.pszEntryName = PVR_USR_PROC_REC_ENTRY_NAME;
        g_stPvrRecProcEntry.pfnShowProc = PVRRecShowProc;
        g_stPvrRecProcEntry.pfnCmdProc = NULL;
        g_stPvrRecProcEntry.pPrivData = g_stPvrRecChns;
        ret = mt_proc_add_entry(MT_ID_PVR, &g_stPvrRecProcEntry);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("MT_PROC_AddEntry(\"%s\") return %d\n", PVR_USR_PROC_REC_ENTRY_NAME, ret);
        }
#endif

        g_stRecInit.bInit = MT_TRUE;

        return MT_SUCCESS;
    }
}

/*****************************************************************************
 Prototype       : MT_PVR_RecDeInit
 Description     : deinit record module
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecDeInit(MT_VOID)
{
    MT_U32 i;

    if (MT_FALSE == g_stRecInit.bInit)
    {
        MT_WARN_PVR("Record Module is not Initialized!\n");
        return MT_SUCCESS;
    }
    else
    {
        /* set all record channel as INVALID status                            */
        for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
        {
            if (g_stPvrRecChns[i].enState != MT_UNF_PVR_REC_STATE_INVALID)
            {
                MT_ERR_PVR("rec chn%d is in use, can NOT deInit REC!\n", i);
                return MT_ERR_PVR_BUSY;
            }

            (MT_VOID)pthread_mutex_destroy(&(g_stPvrRecChns[i].stMutex));
        }

#ifdef PVR_PROC_SUPPORT
        mt_proc_remove_entry(MT_ID_PVR, &g_stPvrRecProcEntry);
        if (!PVR_Play_IsPlaying())
        {
            //mt_proc_remove_dir(g_stPvrRecProcEntry.pszDirectory);
            mt_module_unregister(MT_ID_PVR);
        }
#endif

        PVRIntfDeInitEvent();
        g_stRecInit.bInit = MT_FALSE;
        return MT_SUCCESS;
    }
}

/*****************************************************************************
 Prototype       : MT_PVR_RecCreateChn
 Description     : apply a new reocrd channel
 Input           : pstRecAttr  **the attr user config
 Output          : pu32ChnID   **the chn id we get
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/

static RET_CODE advCa_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
										u8 *pu8DestVirAddr,  ulong ulongDestPhyAddr,
										u8 *pu8SrcDataVirAddr, ulong ulongSrcDataPhyAddr,
										u32 u32Offset,
										u32 *u32DataSize)
{
    mt_pvr_addon_rec_param_input_t  p_param_in={0};
    mt_pvr_addon_rec_param_output_t p_param_out={0};
    mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
    p_param_in.p_src_data=(u8*)(pu8SrcDataVirAddr+u32Offset);
    p_param_in.p_dest_data=(u8*)pu8DestVirAddr;
    p_param_in.p_src_data_phy=ulongSrcDataPhyAddr+u32Offset;
    p_param_in.p_dest_data_phy=ulongDestPhyAddr;
    p_param_in.len= *u32DataSize;
    p_param_in.pos = pstDataAttr->u64FileEndPos;
	p_param_in.global_offset = pstDataAttr->u64GlobalOffset;
#if 0
    if(0)
    {
        char tmpname[128];
        strcpy(tmpname,"/tmp/medias/sda1/rec.ts");
        int fd = PVR_OPEN(tmpname, PVR_FOPEN_MODE_DATA_WRITE);
        if(fd)
        {
            int filesize = PVR_SEEK(fd , 0 , SEEK_END);
            ssize_t writed=PVR_WRITE((const void *)p_param_in.p_src_data,p_param_in.len,fd,filesize);
            PVR_CLOSE(fd);
        }
    }
#endif
    if(p_addon && (p_addon->rec_inter.data_process)){
        mt_handle_t rec_handle_nagra=p_addon->handlesrec[pstDataAttr->u32ChnID - PVR_REC_START_NUM];
        mt_s32 rec_handle_app=p_addon->handlesrec_pvr[pstDataAttr->u32ChnID - PVR_REC_START_NUM];
        if(0<rec_handle_nagra){
            if(MPVR_ADDON_STATUS_OK == p_addon->rec_inter.data_process(rec_handle_nagra,
				(u32)rec_handle_app, pstDataAttr->u32ChnID, &p_param_in, &p_param_out))
            {
                *u32DataSize = p_param_in.len;
                return MT_SUCCESS;
            }
        }
    }
    return MT_FAILURE;

}
static RET_CODE normal_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
										u8 *pu8DestVirAddr,  ulong ulongDestPhyAddr,
										u8 *pu8SrcDataVirAddr, ulong ulongSrcDataPhyAddr,
										u32 u32Offset,
										u32 *u32DataSize)
{//no need to realize me
	//PVR_PRINTF("+normal.w\n");
	return MT_SUCCESS;
}
MT_S32 MT_PVR_RecCreateChn(MT_U32 *pu32ChnID, const MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
    int fmode;
    MT_S32 ret;
    MT_U64 fileSizeReal;
    MT_UNF_PVR_REC_ATTR_S stRecAttrLocal={0,};
    PVR_REC_CHN_S *pChnAttr = NULL;
    MT_U32 u32RecIdInteral;

    PVR_CHECK_POINTER(pu32ChnID);
    PVR_CHECK_POINTER(pstRecAttr);
    CHECK_REC_INIT(&g_stRecInit);


    memcpy(&stRecAttrLocal, pstRecAttr, sizeof(MT_UNF_PVR_REC_ATTR_S));
    ret = PVRRecCheckUserCfg(&stRecAttrLocal);
    if (MT_SUCCESS != ret && MT_ERR_PVR_ALREADY != ret && MT_ERR_PVR_FILE_EXIST != ret)
    {
        fprintf(stderr,"PVRRecCheckUserCfg error\n");
        return ret;
    }
    fileSizeReal = (stRecAttrLocal.u64MaxFileSize / PVR_FIFO_WRITE_BLOCK_SIZE) * PVR_FIFO_WRITE_BLOCK_SIZE;
    stRecAttrLocal.u64MaxFileSize = fileSizeReal;

    pChnAttr = PVRRecFindFreeChn();
    if (NULL == pChnAttr)
    {
        MT_ERR_PVR("Not enough channel to be used!\n");
        return MT_ERR_PVR_NO_CHN_LEFT;
    }

    PVR_LOCK_REC(pChnAttr);
    u32RecIdInteral = pChnAttr->u32ChnID - PVR_REC_START_NUM;

    if(stRecAttrLocal.u32DIO){
        fmode=PVR_FOPEN_MODE_DATA_WRITE_DIO;
    }else{
        fmode=PVR_FOPEN_MODE_DATA_WRITE;
    }
    /* create an data file and open it                                         */
    pChnAttr->re_rec_firstsize=0;
    if(stRecAttrLocal.bEnable_CTrec){   //enable continue rec
        //printf("+++111 %s\n",stRecAttrLocal.szFileName);
        if(PVR_CHECK_FILE_EXIST(stRecAttrLocal.szFileName)){//already exist
            //printf("+++222\n");
            pChnAttr->re_rec_firstsize=PVR_FILE_GetFileSize64(stRecAttrLocal.szFileName);
            fmode=(fmode|O_APPEND);
        }
    }
    //g_timeshift_event_max_count = PVR_REC_MAX_EVENT;
    if(stRecAttrLocal.bSupportTimeShiftEvent == MT_TRUE)
    {
	if((stRecAttrLocal.bRewind == MT_TRUE)
		&& (PVR_MIN_LOOP_REC_TIME_FOR_EVENT <= stRecAttrLocal.u32TimeShiftEventLoopTimeInMs))
	{
		stRecAttrLocal.u64MaxTimeInMs  = stRecAttrLocal.u32TimeShiftEventLoopTimeInMs;
	}
	 if(strstr(stRecAttrLocal.szFileName, ".ts") == NULL)
        {
            strcat(stRecAttrLocal.szFileName, ".ts");
        }
    }

    pChnAttr->dataFile = PVR_OPEN64(stRecAttrLocal.szFileName, fmode,
                                                           stRecAttrLocal.bSupportTimeShiftEvent,
                                                           stRecAttrLocal.u32TimeShiftEventDataUnitSize,
                                                           stRecAttrLocal.u32TimeShiftEventLoopTimeInMs,
                                                           0);
    if (PVR_FILE_INVALID_FILE == pChnAttr->dataFile)
    {
        MT_ERR_PVR("create stream file error!\n");
        PVR_REMOVE_FILE64(stRecAttrLocal.szFileName);
        pChnAttr->enState = MT_UNF_PVR_REC_STATE_INVALID;
        ioctl(g_s32PvrFd, CMD_PVR_DESTROY_REC_CHN, (ulong)&(u32RecIdInteral));
        PVR_UNLOCK_REC(pChnAttr);
        return MT_ERR_PVR_FILE_CANT_OPEN;
    }

    PVR_SET_MAXFILE_SIZE(pChnAttr->dataFile, fileSizeReal);

    {
        mt_sys_version_s stVersion;
        ret=mt_sys_get_version(&stVersion);
        if(MT_SUCCESS==ret){
            if(MT_CHIP_SYMPHONY2_A0  <= stVersion.enChipVersion){
                pChnAttr->chiptype=2;
            }else{
                pChnAttr->chiptype=1;
            }
        }else{
            pChnAttr->chiptype=2;   //default
        }
    }
    pChnAttr->u32magic1 = 0xaa55aa55;
    pChnAttr->u32magic2 = 0xaa55aa55;
    pChnAttr->line_magc1 = 0;
    pChnAttr->line_magc2 = 0;

#ifndef CONFIG_MT_CHIP_SYMPHONY1
    pChnAttr->hCipher=MT_INVALID_HANDLE;
#endif

    /* save chn user-config attr */
    memcpy(&pChnAttr->stUserCfg, &stRecAttrLocal, sizeof(MT_UNF_PVR_REC_ATTR_S));
    pChnAttr->stUserCfg.u64MaxFileSize = fileSizeReal;
    if(pChnAttr->re_rec_firstsize){
        pChnAttr->u64CurFileSize=pChnAttr->re_rec_firstsize;
        //printf("++sz1=%llx\n",pChnAttr->u64CurFileSize);
    }else{
        pChnAttr->u64CurFileSize = 0;
        //printf("++sz2=%llx\n",pChnAttr->u64CurFileSize);
    }
    pChnAttr->u32Flashlen = 0;
    if (MT_UNF_PVR_STREAM_TYPE_ALL_TS != pstRecAttr->enStreamType)
    {
        /* get a new index handle                                                  */
        pChnAttr->IndexHandle = PVR_Index_CreatRec(pChnAttr->u32ChnID, &stRecAttrLocal);
        if (MT_NULL_PTR == pChnAttr->IndexHandle)
        {
            PVR_CLOSE64(pChnAttr->dataFile, pChnAttr->stUserCfg.bSupportTimeShiftEvent);
            PVR_REMOVE_FILE64(stRecAttrLocal.szFileName);
            pChnAttr->enState = MT_UNF_PVR_REC_STATE_INVALID;
            ioctl(g_s32PvrFd, CMD_PVR_DESTROY_REC_CHN, (ulong)&(u32RecIdInteral));
            PVR_UNLOCK_REC(pChnAttr);
            return MT_ERR_PVR_INDEX_CANT_MKIDX;
        }
        pChnAttr->IndexHandle->recsize64=0;
        //if(stRecAttrLocal.bSupportTimeShiftEvent)  //support tf event
        if(1)
        {
            pChnAttr->timeshiftEventHandle.caType = MT_UNF_PVR_REC_VMX;
            pChnAttr->timeshiftEventHandle.state = MT_UNF_PVR_REC_STATE_INIT;
            pChnAttr->timeshiftEventHandle.event_total_count = PVR_REC_MAX_EVENT;
            pChnAttr->timeshiftEventHandle.event_rec = (TimeShift_REC_EVENT_S*)malloc(PVR_REC_MAX_EVENT*sizeof(TimeShift_REC_EVENT_S));
            memset(pChnAttr->timeshiftEventHandle.event_rec, 0, (PVR_REC_MAX_EVENT*sizeof(TimeShift_REC_EVENT_S)));
            if(stRecAttrLocal.bSupportTimeShiftEvent)   //save timeshift as event[0]
            {
                    char *tmp = strrchr((char*)pstRecAttr->szFileName, '/') + 1;
                    (void)MT_PVR_SysGetTimeStampMs(&(pChnAttr->timeshiftEventHandle.event_rec[0].u32EventStartTime));

                    pChnAttr->timeshiftEventHandle.event_rec[0].eventType = TIMESHIFT_EVENT_START_NORMAL;
                    pChnAttr->timeshiftEventHandle.event_rec[0].bSaved = MT_FALSE;
                    if(tmp)   memcpy(pChnAttr->timeshiftEventHandle.event_rec[0].u8EventName, tmp, strlen((const char*)tmp));
                    pChnAttr->timeshiftEventHandle.event_rec[0].eventState = PVR_EVENT_RECING;
            }
	     if(stRecAttrLocal.u32TimeShiftEventLoopTimeInMs == 0)
   	     {
   		    pChnAttr->timeshiftEventHandle.event_total_count= PVR_REC_LOOPTIME_IS_0_MAX_EVENT;
   	     }
	     else
	     {
		  pChnAttr->timeshiftEventHandle.event_total_count = PVR_REC_MAX_EVENT;
	     }
        }
        else
        {
            pChnAttr->timeshiftEventHandle.event_rec = NULL;
        }

        ret = PVRRecPrepareCipher(pChnAttr);
        pChnAttr->IndexHandle->idxmax=CRPTOIDXNUM;
        pChnAttr->IndexHandle->idxstart=0;
        pChnAttr->IndexHandle->idxend=0;
        pChnAttr->IndexHandle->idxcache=NULL;
        pChnAttr->IndexHandle->idxcache=(MT_UNF_DMX_REC_INDEX_S*)malloc(CRPTOIDXNUM*sizeof(MT_UNF_DMX_REC_INDEX_S));
        if ((ret != MT_SUCCESS)||(NULL==pChnAttr->IndexHandle->idxcache))
        {
            MT_ERR_PVR("Pvr recorde prepare cipher error!\n");
            if(pChnAttr->IndexHandle->idxcache){
                free(pChnAttr->IndexHandle->idxcache);
            }

            (MT_VOID)PVR_Index_Destroy(pChnAttr->IndexHandle, PVR_INDEX_REC);

            PVR_CLOSE64(pChnAttr->dataFile, pChnAttr->stUserCfg.bSupportTimeShiftEvent);
            PVR_REMOVE_FILE64(stRecAttrLocal.szFileName);
            pChnAttr->enState = MT_UNF_PVR_REC_STATE_INVALID;
            ioctl(g_s32PvrFd, CMD_PVR_DESTROY_REC_CHN, (ulong)&(u32RecIdInteral));
            PVR_UNLOCK_REC(pChnAttr);
            return ret;
        }
        pChnAttr->IndexHandle->flag_lastI=0;
    }
    else
    {
        pChnAttr->IndexHandle = MT_NULL;
    }

    MT_INFO_PVR("file size adjust to :%lld.\n", fileSizeReal);

    /* here we get record channel successfully                              */
    *pu32ChnID = pChnAttr->u32ChnID;
    pChnAttr->real_file_start_frame = 0;
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pChnAttr->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID)
        {
            mt_pvr_addon_rec_attr_t rec_attr={0};
            rec_attr.pvr_encrypt_flag=stRecAttrLocal.stEncryptCfg.bDoCipher;
            strcpy((char*)rec_attr.file_name,stRecAttrLocal.szFileName);
			rec_attr.pvr_type = stRecAttrLocal.bPvr_timeshift?MPVR_TYPE_TIMESHITF_FILE:MPVR_TYPE_PVR;
            if(p_addon->rec_inter.create)
            {
                if(p_addon->rec_inter.create(&p_addon->handlesrec[recChannelId],&rec_attr) == MPVR_ADDON_STATUS_OK)
                {
                    p_addon->handlesrec_pvr[recChannelId] = stRecAttrLocal.u32DemuxID;
                }
                else
                {
                    MT_ERR_PVR("ERROR call addon create\n");
                    if(pChnAttr->IndexHandle->idxcache){
                        free(pChnAttr->IndexHandle->idxcache);
                    }
                    (MT_VOID)PVR_Index_Destroy(pChnAttr->IndexHandle, PVR_INDEX_REC);
                    PVR_CLOSE64(pChnAttr->dataFile, pChnAttr->stUserCfg.bSupportTimeShiftEvent);
                    PVR_REMOVE_FILE64(stRecAttrLocal.szFileName);
                    pChnAttr->enState = MT_UNF_PVR_REC_STATE_INVALID;
                    ioctl(g_s32PvrFd, CMD_PVR_DESTROY_REC_CHN, (ulong)&(u32RecIdInteral));
                    PVR_UNLOCK_REC(pChnAttr);
                    return MT_ERR_PVR_ADDON_ERROR;
                }
            }
        }
        if(pChnAttr->stUserCfg.stEncryptCfg.bDoCipher)
            pChnAttr->writeCallBack = advCa_WriteCallback;
        else
            pChnAttr->writeCallBack = normal_WriteCallback;
    }
    else
        pChnAttr->writeCallBack = normal_WriteCallback;
    //pChnAttr->stUserCfg.bSupportAdvCa = stRecAttrLocal.bSupportAdvCa;
#ifdef VMX_ADVCA_PVR
   vmx_pvr_rec_printf("stRecAttrLocal.bSupportAdvCa = %d pstRecAttr->u32ScdBufSize=%d\n",stRecAttrLocal.bSupportAdvCa,pstRecAttr->u32ScdBufSize);
   if((MT_UNF_PVR_REC_VMX==stRecAttrLocal.bSupportAdvCa)||(MT_UNF_PVR_REC_COMMON_CRAMBLE==stRecAttrLocal.bSupportAdvCa))
   {
   	int recChannelId = pChnAttr->u32ChnID - PVR_REC_START_NUM;//pstRecAttr->u32DemuxID;
   	pChnAttr->stUserCfg.bSupportAdvCa = stRecAttrLocal.bSupportAdvCa;
   	vmx_pvr_rec_printf("CHANNEL_ID >>> MT_PVR_RecCreateChn  pChnAttr->u32ChnID=%x\n",pChnAttr->u32ChnID - PVR_REC_START_NUM);
  	if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID)
  	{
        memset(&encrypt_rec_buff[recChannelId], 0, sizeof(mt_mmz_buf_s));
        if(encrypt_rec_buff[recChannelId].bufsize == 0)
        {
            encrypt_rec_buff[recChannelId].bufsize = pstRecAttr->u32ScdBufSize;
            #if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
              ret = mt_mmz_malloc(&encrypt_rec_buff[recChannelId]);
              if(MT_SUCCESS != ret)
              {
                  encrypt_rec_buff[recChannelId].bufsize=0;
                  MT_ERR_PVR("%s %d mt_mmz_malloc error\n",__FUNCTION__,__LINE__);
                  PVR_UNLOCK_REC(pChnAttr);
                  return ret;
              }
            #else
              {
                  mt_u32 phyaddr = 0;
                  mt_u32 page_block_order=0,pages_per_block=0,page_block_byte=0;
                  ret = mt_mem_get_pageinfo(&page_block_order,&pages_per_block);
                  if(0==ret){
                      page_block_byte=((1 << page_block_order) * PAGE_SIZE);
                      //printf("+++v.pageinfo=%x,%x,%x\n",page_block_order,pages_per_block,((1 << page_block_order) * PAGE_SIZE));
                      phyaddr = (mt_u32)mt_mmz_new(((pstRecAttr->u32ScdBufSize+(page_block_byte-1))&(~(page_block_byte-1))), page_block_byte,NULL,"pvr"); //alignd to page_block
                      if(0==phyaddr){
                          encrypt_rec_buff[recChannelId].bufsize=0;
                          MT_ERR_PVR("%s %d mt_mmz_new error\n",__FUNCTION__,__LINE__);
                          PVR_UNLOCK_REC(pChnAttr);
                          return MT_ERR_PVR_NO_MEM;
                      }
                      encrypt_rec_buff[recChannelId].user_viraddr=(MT_U8 *)mt_mmz_map((mt_u32)phyaddr, 0);
                      encrypt_rec_buff[recChannelId].phyaddr=phyaddr;
                  }else{
                      encrypt_rec_buff[recChannelId].bufsize=0;
                      MT_ERR_PVR("%s %d mt_mem_get_pageinfo error\n",__FUNCTION__,__LINE__);
                      PVR_UNLOCK_REC(pChnAttr);
                      return MT_FAILURE;
                  }
              }
            #endif
            memset(encrypt_rec_buff[recChannelId].user_viraddr, 0, encrypt_rec_buff[recChannelId].bufsize);
            vmx_pvr_rec_printf("encrypt_rec_buff.user_viraddr=0x%x, encrypt_rec_buff.bufsize=0x%x, encrypt_rec_buff.phyaddr,=0x%x\n",
            encrypt_rec_buff[recChannelId].user_viraddr, encrypt_rec_buff[recChannelId].bufsize, encrypt_rec_buff[recChannelId].phyaddr);
        }
    	}
	else
	{
		MT_ERR_PVR("MT_PVR_RecCreateChn  recChannel_id Error !!\n");
        PVR_UNLOCK_REC(pChnAttr);
		return MT_ERR_PVR_REC_INVALID_RECID;
	}
   }
    pChnAttr->encrypt_rec_buff_rp = 0;
    pChnAttr->encrypt_rec_buff_wp = 0;
    pChnAttr->hardware_rec_buffer_rp = 0;
    pChnAttr->real_write_data_len = 0;
#endif
#ifdef NGR_ADVCA_PVR
    if(MT_UNF_PVR_REC_NAGRA==stRecAttrLocal.bSupportAdvCa){
        pChnAttr->stUserCfg.bSupportAdvCa = MT_UNF_PVR_REC_NAGRA;
    }
#endif
    pChnAttr->First_RawTime=0xffffffff;
    pChnAttr->Last_IdxTime=(0xffffffff-1000);

    memset(&pChnAttr->share_rec,0x00,sizeof(PVE_COPY_SHARE_S));
    pChnAttr->share_rec.flg_masterorslave=1;    //default master
    MT_INFO_PVR("record create ok id=%d\n",pChnAttr->u32ChnID);
    PVR_UNLOCK_REC(pChnAttr);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecCloneChn
 Description     : clone a reocrd channel
 Input           : pstRecAttr  **the attr user config
 Output          : pu32ChnID   **the chn id we get
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecCopyChn(MT_U32 *pu32ChnID, MT_U32 u32_SRC_ChnID,const MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
    MT_U32 u32ChnID;
    PVR_REC_CHN_S *pRecChn_dst = NULL;
    MT_S32 ret=MT_PVR_RecCreateChn(pu32ChnID,pstRecAttr);
    if(0!=ret){
        return ret;
    }

    u32ChnID=(*pu32ChnID);
    pRecChn_dst = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    {//create link.
        PVR_REC_CHN_S *pChnAttr_src = NULL;
        CHECK_REC_CHNID(u32_SRC_ChnID);
        pChnAttr_src = PVR_GET_RECPTR_BY_CHNID(u32_SRC_ChnID);
        PVR_LOCK_REC(pChnAttr_src);//PVR_LOCK(&(pChnAttr_src->stMutex));
        CHECK_REC_CHN_INIT_UNLOCK(pChnAttr_src);
        pChnAttr_src->share_rec.flg_share=1;              //share cnt
        pChnAttr_src->share_rec.Shared_recchid=pRecChn_dst->u32ChnID;  //src->shareid=new->id
        pChnAttr_src->share_rec.Shared_recchptr=pRecChn_dst;
        pChnAttr_src->share_rec.time_offset_ms = 0;

        pRecChn_dst->share_rec.flg_masterorslave=0;       //slave
        pRecChn_dst->share_rec.flg_share=1;               //share cnt
        pRecChn_dst->share_rec.Shared_recchid=pChnAttr_src->u32ChnID;  //new->shareid=src->id
        pRecChn_dst->share_rec.Shared_recchptr=pChnAttr_src;
        pRecChn_dst->share_rec.time_offset_ms = 0;
        pRecChn_dst->RecordIndexThread=pChnAttr_src->RecordIndexThread;
        pRecChn_dst->RecordWIdxThread=pChnAttr_src->RecordWIdxThread;
        pRecChn_dst->RecordStreamThread=pChnAttr_src->RecordStreamThread;
        pRecChn_dst->DemuxRecHandle=pChnAttr_src->DemuxRecHandle;
        if((MT_NULL != pChnAttr_src->IndexHandle) || (MT_NULL != pRecChn_dst->IndexHandle)){
            pRecChn_dst->IndexHandle->enIndexType=pChnAttr_src->IndexHandle->enIndexType;
        }
        PVR_UNLOCK_REC(pChnAttr_src);//PVR_UNLOCK(&(pChnAttr_src->stMutex));
        //printf("++mst:%x,%x,%x\n",u32_SRC_ChnID,pChnAttr_src->share_rec.flg_share,pChnAttr_src->share_rec.flg_masterorslave);
        //printf("++slv:%x,%x,%x\n",u32ChnID,     pRecChn_dst->share_rec.flg_share,pRecChn_dst->share_rec.flg_masterorslave);
    }
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pRecChn_dst->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        PVR_LOCK_REC(pRecChn_dst);
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            if(p_addon->rec_inter.on_start &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->rec_inter.on_start(p_addon->handlesrec[recChannelId])))
            {
                MT_ERR_PVR("start addon error:%d\n",ret);
                PVR_UNLOCK_REC(pRecChn_dst);
                return MT_ERR_PVR_ADDON_ERROR;
            }
        }
        PVR_UNLOCK_REC(pRecChn_dst);
    }
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecDestroyChn
 Description     : free record channel
 Input           : u32ChnID  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecDestroyChn(MT_U32 u32ChnID)
{
    PVR_REC_CHN_S *pRecChn = NULL;
    MT_U32 u32RecIdInteral;
    MT_S32 ret = 0;

    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);
    PVR_LOCK_REC_VALID(pRecChn);

    /* to affirm record channel stopped                                        */
    if ((MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState)
        || (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState)
        //|| (MT_UNF_PVR_REC_STATE_STOPPING == pRecChn->enState)
    )
    {
        PVR_UNLOCK_REC_VALID(pRecChn);
        PVR_UNLOCK_REC(pRecChn);

        MT_ERR_PVR(" can't destroy rec chn%d : chn still runing\n", u32ChnID);
        return MT_ERR_PVR_BUSY;
    }
//    MT_ERR_PVR("id %x, handle id = %d share_rec.flg_masterorslave =%d pRecChn->share_rec.flg_share =%d\n",u32ChnID,pRecChn->u32ChnID,pRecChn->share_rec.flg_masterorslave,pRecChn->share_rec.flg_share);
    
    if(1==pRecChn->share_rec.flg_masterorslave && pRecChn->share_rec.flg_share){ //not to stop share-master
        PVR_UNLOCK_REC_VALID(pRecChn);
        PVR_UNLOCK_REC(pRecChn);
//        MT_ERR_PVR("++destroy .out 0\n");
        return MT_SUCCESS;
    }
    /* we don't care about whether it is timeshifting!                           */
    /* close index handle                                                    */
    if (MT_NULL != pRecChn->IndexHandle)
    {
        if(pRecChn->IndexHandle->idxcache){
            free(pRecChn->IndexHandle->idxcache);
            pRecChn->IndexHandle->idxcache=NULL;
        }
        (MT_VOID)PVR_Index_Destroy(pRecChn->IndexHandle, PVR_INDEX_REC);

        pRecChn->IndexHandle = NULL;
    }
    (MT_VOID)PVRRecReleaseCipher(pRecChn);
    (MT_VOID)PVR_FSYNC64(pRecChn->dataFile);

    /* set channel state to invalid                                     */
    pRecChn->enState = MT_UNF_PVR_REC_STATE_INVALID;
    /* close data file                                                         */
    (MT_VOID)PVR_CLOSE64(pRecChn->dataFile, pRecChn->stUserCfg.bSupportTimeShiftEvent);
    #ifdef TEST_DUAL_REC
    if(MT_UNF_PVR_STREAM_TYPE_ALL_TS != pRecChn->stUserCfg.enStreamType){
        if(-1!=g_anotherFds){
            PVR_FSYNC(g_anotherFds);
            PVR_CLOSE(g_anotherFds);
            g_anotherFds=-1;
        }
    }
    #endif
    u32RecIdInteral = u32ChnID - PVR_REC_START_NUM;

     if(1)//pRecChn->stUserCfg.bSupportTimeShiftEvent)  //support tf event
     {
	   pRecChn->timeshiftEventHandle.caType = 0;
	   pRecChn->timeshiftEventHandle.state = MT_UNF_PVR_REC_STATE_INVALID;
	   pRecChn->timeshiftEventHandle.event_total_count = 0;
          if(pRecChn->timeshiftEventHandle.event_rec)
          {
	     memset(&pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event, 0, sizeof(TimeShift_REC_EVENT_S));
            free(pRecChn->timeshiftEventHandle.event_rec);
            pRecChn->timeshiftEventHandle.event_rec= NULL;
          }
          pRecChn->stUserCfg.bSupportTimeShiftEvent = MT_FALSE;
     }
    if (MT_SUCCESS != ioctl(g_s32PvrFd, CMD_PVR_DESTROY_REC_CHN, (ulong)&u32RecIdInteral))
    {
        MT_FATAL_PVR("pvr rec destroy channel error.\n");
        PVR_UNLOCK_REC_VALID(pRecChn);
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_REC_FAIL_DESTROY;
    }
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pRecChn->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            if(p_addon->rec_inter.on_destroy &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->rec_inter.on_destroy(p_addon->handlesrec[recChannelId])))
            {
                MT_ERR_PVR("on_destroy addon error:%d\n",ret);
            }
        }
    }
    #ifdef CONFIG_MT_LXC_SUPPORT
    mt_pvr_ipc_clear_tid();
    #endif
    //printf("++destory.info0=%x,%x\n",pRecChn->share_rec.flg_share,pRecChn->share_rec.flg_masterorslave);
//    MT_ERR_PVR("id %x, handle id = %d share_rec.flg_masterorslave =%d pRecChn->share_rec.flg_share =%d\n",u32ChnID,pRecChn->u32ChnID,pRecChn->share_rec.flg_masterorslave,pRecChn->share_rec.flg_share);
    if((1==pRecChn->share_rec.flg_share) && (0==pRecChn->share_rec.flg_masterorslave)){ //share and slave
        PVR_REC_CHN_S *tmprec_m=(PVR_REC_CHN_S *)pRecChn->share_rec.Shared_recchptr;
        //printf("++destory.info1=%x\n",tmprec_m->enState);
        if(MT_UNF_PVR_REC_STATE_STOP==tmprec_m->enState){ //if master stopped. do it
            //printf("+++++reclose0\n");
            PVR_UNLOCK_REC_VALID(pRecChn);
            PVR_UNLOCK_REC(pRecChn);
            tmprec_m->share_rec.flg_share = 0;
            tmprec_m->share_rec.Shared_recchid = 0;
            MT_PVR_RecStopChn(tmprec_m->u32ChnID);
            MT_PVR_RecDestroyChn(tmprec_m->u32ChnID);
            PVR_LOCK_REC(pRecChn);
            PVR_LOCK_REC_VALID(pRecChn);
            pRecChn->share_rec.Shared_recchptr = NULL;
//            MT_ERR_PVR("+++++reclose1\n");
        }
        pRecChn->share_rec.flg_share=0;
        pRecChn->share_rec.Shared_recchid = 0;
        tmprec_m->share_rec.flg_share=0;
        tmprec_m->share_rec.Shared_recchid = 0;
    }
//    MT_ERR_PVR("id %x, handle id = %d share_rec.flg_masterorslave =%d pRecChn->share_rec.flg_share =%d\n",u32ChnID,pRecChn->u32ChnID,pRecChn->share_rec.flg_masterorslave,pRecChn->share_rec.flg_share);  
    PVR_UNLOCK_REC_VALID(pRecChn);
    PVR_UNLOCK_REC(pRecChn);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecSetChn
 Description     : set record channel attributes
 Input           : u32ChnID  **
                   pRecAttr  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecSetChn(MT_U32 u32ChnID, const MT_UNF_PVR_REC_ATTR_S * pstRecAttr)
{
    PVR_REC_CHN_S *pRecChn = NULL;

    CHECK_REC_CHNID(u32ChnID);
    PVR_CHECK_POINTER(pstRecAttr);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    CHECK_REC_CHN_INIT(pRecChn->enState);

    /* currently, we can't set record channel dynamically. */

    return MT_ERR_PVR_NOT_SUPPORT;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecGetChn
 Description     : get record channel attributes
 Input           : u32ChnID  **
                   pRecAttr  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecGetChn(MT_U32 u32ChnID, MT_UNF_PVR_REC_ATTR_S *pstRecAttr)
{
    PVR_REC_CHN_S *pRecChn = NULL;

    CHECK_REC_CHNID(u32ChnID);

    PVR_CHECK_POINTER(pstRecAttr);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    CHECK_REC_CHN_INIT(pRecChn->enState);

    memcpy(pstRecAttr, &(pRecChn->stUserCfg), sizeof(MT_UNF_PVR_REC_ATTR_S));

    return MT_SUCCESS;
}
MT_S32 pvr_create_thread_attr(pthread_attr_t *p_thread_attr, MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize)
{
    struct sched_param sparam;

    if((NULL==p_thread_attr)||(schedpolicy != SCHED_FIFO &&  schedpolicy != SCHED_RR && schedpolicy != SCHED_OTHER)){
        MT_ERR_PVR("++set %p.%u\n",p_thread_attr,(int)schedpolicy);
        return -1;
    }
#ifdef CONFIG_MTOS_TASK_SCHED_FIFO
    schedpolicy = SCHED_FIFO;
#elif defined(CONFIG_MTOS_TASK_SCHED_OTHER)
    schedpolicy = SCHED_OTHER;
#else
    /*default SCHED_RR for Lotus*/
    schedpolicy = SCHED_RR;
#endif

    pthread_attr_init(p_thread_attr);
    if (priority < sched_get_priority_min(schedpolicy) ||priority > sched_get_priority_max(schedpolicy) ){
        priority = sched_get_priority_max(schedpolicy) /2;
    }

    pthread_attr_setschedpolicy(p_thread_attr, schedpolicy);
    pthread_attr_setscope(p_thread_attr, PTHREAD_SCOPE_SYSTEM);

    pthread_attr_getschedparam(p_thread_attr, &sparam);
    sparam.sched_priority = priority;
    pthread_attr_setschedparam(p_thread_attr, &sparam);
    pthread_attr_setinheritsched(p_thread_attr, PTHREAD_EXPLICIT_SCHED);

    if(0!=stacksize){
        if (stacksize < 16 * 1024) {
            stacksize = 16 * 1024;
        }
        if (stacksize > 1 * 1024 * 1024) {
            stacksize = 1 * 1024 * 1024;
        }
        pthread_attr_setstacksize(p_thread_attr, (size_t)stacksize);
    }
    pthread_attr_setdetachstate(p_thread_attr, PTHREAD_CREATE_JOINABLE);
    //pthread_attr_setdetachstate(p_thread_attr, PTHREAD_CREATE_DETACHED);
    return 0;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecStartChn
 Description     : start record channel
 Input           : u32ChnID, the record channel ID
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecStartChn(MT_U32 u32ChnID)
{
    MT_S32 ret;
    PVR_REC_CHN_S *pRecChn = NULL;
    MT_UNF_PVR_REC_ATTR_S *pUserCfg;
    pthread_attr_t thread_attr;
    pthread_attr_t *p_thread_attr=NULL;

    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    if ((MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState)
        || (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState))
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_ALREADY;
    }
    else
    {
        pRecChn->enState = MT_UNF_PVR_REC_STATE_RUNNING;
    }

    pRecChn->u32Flashlen = 0;
    pRecChn->s32OverFixTimes = 0;
    pRecChn->bEventFlg = MT_FALSE;
    (void)MT_PVR_SysGetTimeStampMs(&pRecChn->u32RecStartTimeMs);

    pUserCfg = &(pRecChn->stUserCfg);

    if((MT_NULL != pRecChn->IndexHandle)&&(0==pRecChn->re_rec_firstsize)){    //sync indexheader spend time to make get bad stream.
        PVR_Index_ResetRecAttr(pRecChn->IndexHandle);
        /* failure to write user data, but still, continue to record. just only print the error info */
        if (PVR_Index_PrepareHeaderInfo(pRecChn->IndexHandle, pRecChn->stUserCfg.u32UsrDataInfoSize, pRecChn->stUserCfg.enIndexVidType))
        {
            MT_ERR_PVR("PVR_Index_PrepareHeaderInfo fail\n");
        }
    }

    if(pRecChn->share_rec.flg_share && (0==pRecChn->share_rec.flg_masterorslave)){ //share slave
        PVR_REC_CHN_S *tmprec_m=(PVR_REC_CHN_S *)pRecChn->share_rec.Shared_recchptr;
        pRecChn->DemuxRecHandle=tmprec_m->DemuxRecHandle;
        pRecChn->RecordIndexThread=tmprec_m->RecordIndexThread;
        pRecChn->RecordWIdxThread=tmprec_m->RecordWIdxThread;
        pRecChn->RecordStreamThread=tmprec_m->RecordStreamThread;
        //printf("++++slave return 0 ! dmxhd=%x\n",pRecChn->DemuxRecHandle);
        PVR_UNLOCK_REC(pRecChn);
        return MT_SUCCESS;
    }

    //PVRRecCheckExistFile(pUserCfg->szFileName);
    /* create record thread to receive index from the channel                 */
    ret = PVRRecStartDemux(pRecChn, pUserCfg->enStreamType, pUserCfg, pUserCfg->bIsClearStream);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("~~~~~~~~start demux record channel failure!\n");
        pRecChn->enState = MT_UNF_PVR_REC_STATE_INIT;
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_REC_FAIL_START_DEMUX;
    }
    else
    {
        MT_INFO_PVR("~~~~~~~~~~start demux OK, indexTYpe:%d!\n", pUserCfg->enIndexType);
    }
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pRecChn->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            if(p_addon->rec_inter.on_start &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->rec_inter.on_start(p_addon->handlesrec[recChannelId])))
            {
                MT_ERR_PVR("start addon error:%d\n",ret);
                (MT_VOID)PVRRecStopDemux(pRecChn);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_ADDON_ERROR;
            }
        }
    }
    if (MT_NULL != pRecChn->IndexHandle)
    {
        #ifdef PVR_DEALIDX_IN_AV
        if(MT_UNF_PVR_REC_INDEX_TYPE_VIDEO==pRecChn->IndexHandle->enIndexType){   //only video-index ,need it
            // create record thread to receive index from the channel
            p_thread_attr=NULL;
            if(g_indexreadthreadattr.flag_valid){
                //printf("pvr r index0 thread:%d,%d,%d\n",g_indexreadthreadattr.schedpolicy,g_indexreadthreadattr.priority,g_indexreadthreadattr.stacksize);
                if(0==pvr_create_thread_attr(&thread_attr,g_indexreadthreadattr.schedpolicy,g_indexreadthreadattr.priority,g_indexreadthreadattr.stacksize)){
                    p_thread_attr=&thread_attr;
                    //printf("pvr r index1 thread:%d,%d,%d\n",g_indexreadthreadattr.schedpolicy,g_indexreadthreadattr.priority,g_indexreadthreadattr.stacksize);
                }
            }
            ret = pthread_create(&pRecChn->RecordIndexThread, p_thread_attr, PVRRecGetIdxFromAvThread, pRecChn);
            if (ret != MT_SUCCESS)
            {
                pRecChn->enState = MT_UNF_PVR_REC_STATE_STOP;
                MT_ERR_PVR("create record INDEX thread failure!\n");
                (MT_VOID)PVRRecStopDemux(pRecChn);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_CREAT_THREAD_ERR;
            }
        }else if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO==pRecChn->IndexHandle->enIndexType){ //only audio-index ,need it
            // create record thread to write index to the shm
            p_thread_attr=NULL;
            if(g_indexwritethreadattr.flag_valid){
                //printf("pvr w index0 thread:%d,%d,%d\n",g_indexwritethreadattr.schedpolicy,g_indexwritethreadattr.priority,g_indexwritethreadattr.stacksize);
                if(0==pvr_create_thread_attr(&thread_attr,g_indexwritethreadattr.schedpolicy,g_indexwritethreadattr.priority,g_indexwritethreadattr.stacksize)){
                    p_thread_attr=&thread_attr;
                    //printf("pvr w index1 thread:%d,%d,%d\n",g_indexwritethreadattr.schedpolicy,g_indexwritethreadattr.priority,g_indexwritethreadattr.stacksize);
                }
            }
            ret = pthread_create(&pRecChn->RecordWIdxThread, p_thread_attr, PVRRecGetAudioThread, pRecChn);
            if (ret != MT_SUCCESS)
            {
                pRecChn->enState = MT_UNF_PVR_REC_STATE_STOP;
                MT_ERR_PVR("create record INDEX thread failure!\n");
                (MT_VOID)PVRRecStopDemux(pRecChn);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_CREAT_THREAD_ERR;
            }
        }
        #else
        p_thread_attr=NULL;
        if(g_indexwritethreadattr.flag_valid){
            //printf("pvr w index0 thread:%d,%d,%d\n",g_indexwritethreadattr.schedpolicy,g_indexwritethreadattr.priority,g_indexwritethreadattr.stacksize);
            if(0==pvr_create_thread_attr(&thread_attr,g_indexwritethreadattr.schedpolicy,g_indexwritethreadattr.priority,g_indexwritethreadattr.stacksize)){
                p_thread_attr=&thread_attr;
                //printf("pvr w index1 thread:%d,%d,%d\n",g_indexwritethreadattr.schedpolicy,g_indexwritethreadattr.priority,g_indexwritethreadattr.stacksize);
            }
        }
        ret = pthread_create(&pRecChn->RecordWIdxThread, p_thread_attr, PVRWriteIndexRoutine, pRecChn);
        if (ret != MT_SUCCESS)
        {
            pRecChn->enState = MT_UNF_PVR_REC_STATE_STOP;
            MT_ERR_PVR("create record INDEX thread failure!\n");
            (MT_VOID)PVRRecStopDemux(pRecChn);
            PVR_UNLOCK_REC(pRecChn);
            return MT_ERR_PVR_CREAT_THREAD_ERR;
        }
        #endif
    }
#if 1
    /* create record thread to receive stream from the channel                 */
    p_thread_attr=NULL;
    if(g_tswritethreadattr.flag_valid){
        //printf("pvr save ts0 thread:%d,%d,%d\n",g_tswritethreadattr.schedpolicy,g_tswritethreadattr.priority,g_tswritethreadattr.stacksize);
        if(0==pvr_create_thread_attr(&thread_attr,g_tswritethreadattr.schedpolicy,g_tswritethreadattr.priority,g_tswritethreadattr.stacksize)){
            p_thread_attr=&thread_attr;
            //printf("pvr save ts1 thread:%d,%d,%d\n",g_tswritethreadattr.schedpolicy,g_tswritethreadattr.priority,g_tswritethreadattr.stacksize);
        }
    }
    ret = pthread_create(&pRecChn->RecordStreamThread, p_thread_attr, PVRRecSaveStreamRoutine, pRecChn);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("create record STREAM thread failure!\n");
        (MT_VOID)PVRRecStopDemux(pRecChn);
        pRecChn->enState = MT_UNF_PVR_REC_STATE_INIT;
        PVR_UNLOCK_REC(pRecChn);
        MT_ERR_PVR("%s--------------------%d\n",__FILE__,__LINE__);
        return MT_ERR_PVR_CREAT_THREAD_ERR;
    }
    MT_INFO_PVR("channel %d start ok.\n", u32ChnID);
#endif
    PVR_UNLOCK_REC(pRecChn);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecStopChn
 Description     : stop the pointed record channel
 Input           : u32ChnId, channle id
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecStopChn(MT_U32 u32ChnID)
{
    MT_S32 ret;

    //MT_UNF_PVR_FILE_ATTR_S  fileAttr;

    PVR_REC_CHN_S  *pRecChn;
//    int wait_counter=0;
    int i=0;

    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    /* to confirm the record channel running                           */
    if ((MT_UNF_PVR_REC_STATE_RUNNING != pRecChn->enState)
        && (MT_UNF_PVR_REC_STATE_PAUSE != pRecChn->enState))
    {
        MT_WARN_PVR("Channel has already stopped!\n");
        //PVR_UNLOCK_REC(pRecChn);
        //return MT_ERR_PVR_ALREADY;
    }

    /* state: stoping -> stop. make sure the index thread exit first   */
    //pRecChn->enState = MT_UNF_PVR_REC_STATE_STOPPING;

    //(MT_VOID)MT_PthreadJoin(pRecChn->RecordIndexThread, NULL);
    //MT_ASSERT(MT_UNF_PVR_REC_STATE_STOP == pRecChn->enState);

#if 0
    ret = PVR_Index_RecGetFileAttr(pRecChn->IndexHandle, &fileAttr);
    if (MT_SUCCESS == ret)
    {
        indexedSize = fileAttr.u64ValidSizeInByte;
    }
    else
    {
        indexedSize = pRecChn->u64CurFileSize;
    }

    MT_ERR_PVR("file size:%llu, index size:%llu\n", pRecChn->u64CurFileSize,
               fileAttr.u64ValidSizeInByte);

    while ((pRecChn->u64CurFileSize < indexedSize)
           && (waitTimes < 30)
           && pRecChn->bSavingData)    /*If returned already by error,go ahead*/ /*CNcomment:如果已经出错退出，可以不用继续等待 */
    {
        MT_USLEEP(1000 * 40);
        waitTimes++;
        MT_ERR_PVR("wait%u, file size:%llu, index size:%llu\n", waitTimes,
                   pRecChn->u64CurFileSize,
                   fileAttr.u64ValidSizeInByte);
    }
#endif
    pRecChn->enState = MT_UNF_PVR_REC_STATE_STOP;

    //MT_ERR_PVR("id %x, handle id = %d share_rec.flg_masterorslave =%d pRecChn->share_rec.flg_share =%d\n",u32ChnID,pRecChn->u32ChnID,pRecChn->share_rec.flg_masterorslave,pRecChn->share_rec.flg_share);
    if((0==pRecChn->share_rec.flg_masterorslave) ||
       (1==pRecChn->share_rec.flg_masterorslave && pRecChn->share_rec.flg_share)){ //not to stop share-master
        PVR_UNLOCK_REC(pRecChn);
        //printf("++stop .rt 0\n");
        if(0==pRecChn->share_rec.flg_masterorslave){//wait thread feeling stop
            int wait_counter=0;
            while(wait_counter<8){//0.5*8=4s
                if(1==pRecChn->share_rec.can_stop){
                    //MT_ERR_PVR("++normal out\n");
                    break;
                }
                wait_counter++;
                MT_USLEEP(500*1000);
            }
        }

        //PVR_ALAWYS_PRINT("%s %d stop channel=%d rec_handles=%d flg_masterorslave=%d\n",__FUNCTION__,__LINE__,
        //            pRecChn->u32ChnID,pRecChn->rec_handles,pRecChn->share_rec.flg_masterorslave);
        //reset rec pid  array status
        for (i = 0; i < PVR_REC_MAX_PID; i++){
            pRecChn->rec_pid[i].status = PVR_REC_PID_INIT;
            //printf("%s %d reset channel=%d rec pid[%d]=0x%x\n",__FUNCTION__,__LINE__,pRecChn->u32ChnID,i,pRecChn->rec_pid[i].pid);//debug print
        }	
        
        return MT_SUCCESS;
    }
    PVR_UNLOCK_REC(pRecChn);

    (MT_VOID)pthread_join(pRecChn->RecordStreamThread, NULL);

    if (MT_NULL != pRecChn->IndexHandle)
    {
        #ifdef PVR_DEALIDX_IN_AV
        if(MT_UNF_PVR_REC_INDEX_TYPE_VIDEO==pRecChn->IndexHandle->enIndexType){
            (MT_VOID)pthread_join(pRecChn->RecordIndexThread, NULL);
        }else if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO==pRecChn->IndexHandle->enIndexType){
            (MT_VOID)pthread_join(pRecChn->RecordWIdxThread, NULL);
        }
        #else
        (MT_VOID)pthread_join(pRecChn->RecordWIdxThread, NULL);
        #endif
        //PVR_Index_RecUpdateStartFrame(pRecChn->IndexHandle, pRecChn->u64CurFileSize);

        PVR_Index_RecUpdateEndFrame(pRecChn->IndexHandle);

        PVR_Index_FlushIdxWriteCache(pRecChn->IndexHandle);
    }
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pRecChn->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            if(p_addon->rec_inter.on_stop &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->rec_inter.on_stop(p_addon->handlesrec[recChannelId])))
            {
                MT_ERR_PVR("stop addon error:%d\n",ret);
                return MT_ERR_PVR_ADDON_ERROR;
            }
        }
    }
    PVR_LOCK_REC(pRecChn);
//    MT_ERR_PVR("try to demux stop \n");
    ret = PVRRecStopDemux(pRecChn);
    if (MT_SUCCESS != ret)
    {
        PVR_UNLOCK_REC(pRecChn);
        MT_ERR_PVR("demux stop error:%#x\n", ret);
        return MT_ERR_PVR_REC_FAIL_STOP_DEMUX;
    }
    else
    {
        MT_ERR_PVR("stop demux%d ok\n", pRecChn->stUserCfg.u32DemuxID);
    }
    PVR_UNLOCK_REC(pRecChn);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_RecPauseChn(MT_U32 u32ChnID)
{
    PVR_REC_CHN_S  *pRecChn;

    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pRecChn->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            MPVR_ADDON_STATUS ret;
            if(p_addon->rec_inter.on_pause &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->rec_inter.on_pause(p_addon->handlesrec[recChannelId])))
            {
                MT_ERR_PVR("pause addon error:%d\n",ret);
                return MT_ERR_PVR_ADDON_ERROR;
            }
        }
    }
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    /* to confirm the record channel running  */
    if ((MT_UNF_PVR_REC_STATE_RUNNING != pRecChn->enState)
        && (MT_UNF_PVR_REC_STATE_PAUSE != pRecChn->enState))
    {
        PVR_UNLOCK_REC(pRecChn);
        MT_ERR_PVR("Channel not started!\n");
        return MT_ERR_PVR_REC_INVALID_STATE;
    }

    pRecChn->enState = MT_UNF_PVR_REC_STATE_PAUSE;
    PVR_UNLOCK_REC(pRecChn);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_RecResumeChn(MT_U32 u32ChnID)
{
    PVR_REC_CHN_S  *pRecChn;

    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = pRecChn->u32ChnID - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID && p_addon->handlesrec[recChannelId])
        {
            MPVR_ADDON_STATUS ret;
            if(p_addon->rec_inter.on_resume && MPVR_ADDON_STATUS_OK != (ret = p_addon->rec_inter.on_resume(p_addon->handlesrec[recChannelId])))
            {
                MT_ERR_PVR("pause addon error:%d\n",ret);
                return MT_ERR_PVR_ADDON_ERROR;
            }
        }
    }

    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    /* to confirm the record channel running  */
    if ((MT_UNF_PVR_REC_STATE_RUNNING != pRecChn->enState)
        && (MT_UNF_PVR_REC_STATE_PAUSE != pRecChn->enState))
    {
        PVR_UNLOCK_REC(pRecChn);
        MT_ERR_PVR("Channel not started!\n");
        return MT_ERR_PVR_REC_INVALID_STATE;
    }

    pRecChn->enState = MT_UNF_PVR_REC_STATE_RUNNING;
    PVR_UNLOCK_REC(pRecChn);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_RecGetStatus
 Description     : get record status and recorded file size
 Input           : u32ChnID    **
                   pRecStatus  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/10
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_RecGetStatus(MT_U32 u32ChnID, MT_UNF_PVR_REC_STATUS_S *pstRecStatus)
{
    MT_S32 ret;
    PVR_REC_CHN_S   *pRecChn;
    MT_UNF_PVR_FILE_ATTR_S fileAttr;
    MT_UNF_DMX_RECBUF_STATUS_S stStatus;
    //MT_U32 u32DeltaTimeMs = 0;
    //static MT_U32 u32EndTimeInMs = 0;
    //MT_UNF_DMX_REC_INDEX_S last_index;
    PVR_INDEX_ENTRY_S pvr_index_entry;

    CHECK_REC_CHNID(u32ChnID);
    PVR_CHECK_POINTER(pstRecStatus);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_LOCK_REC_VALID(pRecChn);
    if (MT_UNF_PVR_REC_STATE_INVALID ==  pRecChn->enState){
        PVR_UNLOCK_REC_VALID(pRecChn);
        return MT_ERR_PVR_CHN_NOT_INIT;
    }

    if ((MT_UNF_PVR_REC_STATE_INIT == pRecChn->enState)
        || (MT_UNF_PVR_REC_STATE_INVALID == pRecChn->enState)
        //|| (MT_UNF_PVR_REC_STATE_STOPPING == pRecChn->enState)
        /*|| (MT_UNF_PVR_REC_STATE_STOP == pRecChn->enState)  */
        || (MT_UNF_PVR_REC_STATE_BUTT == pRecChn->enState)) /* not running, just return state */
    {
        memset(pstRecStatus, 0, sizeof(MT_UNF_PVR_REC_STATUS_S));
        pstRecStatus->enState = pRecChn->enState;
        PVR_UNLOCK_REC_VALID(pRecChn);
        return MT_SUCCESS;
    }

    /* get record state                                                        */
    pstRecStatus->enState = pRecChn->enState;

    if ((NULL != pRecChn->IndexHandle)
      && pRecChn->IndexHandle->bIsRec
      && (0xffffffff == pRecChn->First_RawTime)
      && (MT_UNF_PVR_REC_INDEX_TYPE_VIDEO == pRecChn->IndexHandle->enIndexType)){
      PVR_UNLOCK_REC_VALID(pRecChn);
      //printf("\n##MT_PVR_RecGetStatus[%x][%ld] fail\n", pRecChn->u32ChnID, __LINE__);
      return MT_FAILURE;
    }

    ret = MT_UNF_DMX_GetRecBufferStatus(pRecChn->DemuxRecHandle, &stStatus);
    if (MT_SUCCESS == ret)
    {
        pstRecStatus->stRecBufStatus.u32BufSize  = stStatus.u32BufSize;
        pstRecStatus->stRecBufStatus.u32UsedSize = stStatus.u32UsedSize;
    }
    else
    {
        pstRecStatus->stRecBufStatus.u32BufSize  = 0;
        pstRecStatus->stRecBufStatus.u32UsedSize = 0;
    }

    if (MT_NULL == pRecChn->IndexHandle)
    {//this is for rec all
        MT_U32 u32CurTimeMs = 0;
        (void)MT_PVR_SysGetTimeStampMs(&u32CurTimeMs);
        pstRecStatus->u32CurTimeInMs   = (u32CurTimeMs-pRecChn->u32RecStartTimeMs); //fixbug 109777
        pstRecStatus->u32CurWriteFrame = 0;
        pstRecStatus->u64CurWritePos   = pRecChn->u64CurFileSize;
        pstRecStatus->u64CurWPos_Glb   = pRecChn->u64CurFileSize;
        pstRecStatus->u32StartTimeInMs = 0;
        pstRecStatus->u32EndTimeInMs = 0;
        PVR_UNLOCK_REC_VALID(pRecChn);
        return MT_SUCCESS;
    }

    /* get recorded file size                                                  */

    //pstRecStatus->u64CurWritePos = pRecChn->u64CurFileSize;

    //PVR_Index_FlushIdxWriteCache(pRecChn->IndexHandle);

    ret = PVR_Index_PlayGetFileAttrByFileName(pRecChn->stUserCfg.szFileName, pRecChn->IndexHandle, &fileAttr);
    if (MT_SUCCESS == ret)
    {

        pstRecStatus->u32CurTimeInMs = pRecChn->IndexHandle->stCurRecFrame.u32DisplayTimeMs;
        pstRecStatus->u32EndTimeInMs = fileAttr.u32EndTimeInMs;

        if(pRecChn->IndexHandle->bIsRec && (MT_UNF_PVR_REC_INDEX_TYPE_VIDEO == pRecChn->IndexHandle->enIndexType)){
            //last_index.u32PrivatePara =  pRecChn->IndexHandle->idx_chn_num_avap;
            //if(MT_SUCCESS == MT_PVR_Get_Last_index(&last_index))
            if(MT_SUCCESS ==PVR_Index_GetLastIdxWriteCacheIndexEntry(pRecChn->IndexHandle,&pvr_index_entry))
            {
              pstRecStatus->u32EndTimeInMs = pvr_index_entry.u32DisplayTimeMs;// - pRecChn->First_RawTime+pRecChn->IndexHandle->Last_DisplayTimeMs);
              pstRecStatus->u32CurTimeInMs = pstRecStatus->u32EndTimeInMs;
              //printf("\n##Get_Last_index[%x] u32StartTimeInMs[%ld],u32EndTimeInMs[%ld]", pRecChn->u32ChnID, fileAttr.u32StartTimeInMs,pstRecStatus->u32EndTimeInMs);
            }
			/*
            else
            {
              MT_ERR_PVR("MT_PVR_RecGetStatus[%x][%ld] fail\n", pRecChn->u32ChnID, __LINE__);
            }
			*/
        }
        if (pstRecStatus->u32EndTimeInMs == 0)
        {
          ret = -1;
        }
        else
        {
          pstRecStatus->u32CurWriteFrame = fileAttr.u32FrameNum;
          pstRecStatus->u64CurWritePos   = fileAttr.u64ValidSizeInByte;
          pstRecStatus->u32StartTimeInMs = fileAttr.u32StartTimeInMs;
          pstRecStatus->u32AppendRecMs   = pstRecStatus->u32CurTimeInMs;
          pstRecStatus->u64CurWPos_Glb   = fileAttr.u64CurWPos_Glb;
        }
        //printf("++rec pos=%llx,%llx\n",pstRecStatus->u64CurWritePos,pstRecStatus->u64CurWPos_Glb);
    }
    PVR_UNLOCK_REC_VALID(pRecChn);

    return ret;
}

MT_S32 MT_PVR_RecRegisterWriteCallBack(MT_U32 u32ChnID, ExtraCallBack writeCallBack)
{
    PVR_REC_CHN_S   *pRecChn;

    CHECK_REC_CHNID(u32ChnID);
    PVR_CHECK_POINTER(writeCallBack);
    vmx_pvr_rec_printf("%s================%d\n",__FUNCTION__,__LINE__);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);
    /*modify for DTS2014032900795 */
    if(MT_UNF_PVR_STREAM_TYPE_ALL_TS != pRecChn->stUserCfg.enStreamType)
    {
	 vmx_pvr_rec_printf("%s================%d\n",__FUNCTION__,__LINE__);
	 pRecChn->writeCallBack = writeCallBack;
        PVR_UNLOCK_REC(pRecChn);
        return MT_SUCCESS;
    }
    else
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_NOT_SUPPORT;
    }
}


MT_S32 MT_PVR_RecUnRegisterWriteCallBack(MT_U32 u32ChnID)
{
    PVR_REC_CHN_S   *pRecChn;

    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);
    pRecChn->writeCallBack = NULL;
    PVR_UNLOCK_REC(pRecChn);

    return MT_SUCCESS;
}

/*
 * suggesting, the user should set/get the user data by TS file name. as extend, also used by *.idx
 */
MT_S32 MT_PVR_SetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32UsrDataLen)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_S32 s32Fd;

    PVR_CHECK_POINTER(pFileName);
    PVR_CHECK_POINTER(pInfo);

    if (0 == u32UsrDataLen)
    {
        return MT_SUCCESS;
    }
    MT_INFO_PVR("++save file info:%s,%x\n",pFileName,u32UsrDataLen);
    //PVR_Index_GetIdxFileName(strIdxFileName, (MT_CHAR*)pFileName);

    /*if (MT_FALSE == PVR_CHECK_FILE_EXIST(strIdxFileName))
    {
        MT_ERR_PVR("file:%s not exist.\n", strIdxFileName);
        return MT_ERR_PVR_FILE_CANT_OPEN;
    }*/

    s32Fd = PVR_OPEN(pFileName, PVR_FOPEN_MODE_INDEX_BOTH);
    if (s32Fd < 0)
    {
        MT_ERR_PVR("open file:%s fail:0x%x\n", pFileName, s32Fd);
        return MT_ERR_PVR_FILE_CANT_OPEN;
    }

    s32Ret = PVR_Index_SetUsrDataInfo(s32Fd, pInfo, u32UsrDataLen);
    if (s32Ret > 0)
    {
        s32Ret = MT_SUCCESS;
    }
    else
    {
        MT_ERR_PVR("PVR_Index_SetUsrDataInfo fail\n");
    }

    PVR_CLOSE(s32Fd);
    return s32Ret;
}

MT_S32 MT_PVR_GetUsrDataInfoByFileName(const MT_CHAR *pFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* pUsrDataLen)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_S32 s32Fd;

    PVR_CHECK_POINTER(pFileName);
    PVR_CHECK_POINTER(pInfo);
    PVR_CHECK_POINTER(pUsrDataLen);

    if (0 == u32BufLen)
    {
        return MT_SUCCESS;
    }

    s32Fd = PVR_OPEN(pFileName, PVR_FOPEN_MODE_INDEX_READ);
    if (s32Fd < 0)
    {
        MT_ERR_PVR("open file:%s fail:0x%x\n", pFileName, s32Fd);
        return MT_ERR_PVR_FILE_CANT_OPEN;
    }

    s32Ret = PVR_Index_GetUsrDataInfo(s32Fd, pInfo, u32BufLen);
    if (s32Ret > 0)
    {
        *pUsrDataLen = (MT_U32)s32Ret;
        s32Ret = MT_SUCCESS;
    }
    else
    {
        *pUsrDataLen = 0;
        MT_ERR_PVR("PVR_Index_GetUsrDataInfo fail\n");
    }

    PVR_CLOSE(s32Fd);
    return s32Ret;
}

MT_S32 MT_PVR_SetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32CADataLen,  MT_U8  u8IsAPPEND)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_S32 s32Fd;

    PVR_CHECK_POINTER(pIdxFileName);
    PVR_CHECK_POINTER(pInfo);

    if ((0 == u32CADataLen )||(PVR_MAX_CADATA_LEN < u32CADataLen))
    {
        MT_ERR_PVR("u32CADataLen (%d) invalid!\n",u32CADataLen);
        return MT_FAILURE;
    }

   //PVR_Index_GetIdxFileName(strIdxFileName, (MT_CHAR*)pFileName);
   vmx_pvr_rec_printf("%s=======%d\n",__FUNCTION__,__LINE__);
   if(u8IsAPPEND == 0)   s32Fd = PVR_OPEN(pIdxFileName, PVR_FOPEN_MODE_CADATA_TRUNC);
   else   s32Fd = PVR_OPEN(pIdxFileName, PVR_FOPEN_MODE_CADATA_BOTH);

    if (s32Fd < 0)
    {
        MT_ERR_PVR("open file:%s fail:0x%x\n", pIdxFileName, s32Fd);
        return MT_ERR_PVR_FILE_CANT_OPEN;
    }
    s32Ret = PVR_Index_SetCADataInfo(s32Fd, pInfo, u32CADataLen);
    if (s32Ret > 0)
    {
	 s32Ret = MT_SUCCESS;
    }
    else
    {
        MT_ERR_PVR("PVR_Index_SetCADataInfo fail\n");
    }
    vmx_pvr_rec_printf("%s=======%d\n",__FUNCTION__,__LINE__);
    PVR_CLOSE(s32Fd);
    return s32Ret;
}

MT_S32 MT_PVR_GetCAData(const MT_CHAR *pIdxFileName, MT_U8 *pInfo, MT_U32 u32BufLen, MT_U32* u32CADataLen)
{
    MT_S32 s32Ret = MT_SUCCESS;
    MT_S32 s32Fd;
    PVR_CHECK_POINTER(pIdxFileName);
    PVR_CHECK_POINTER(pInfo);
    PVR_CHECK_POINTER(u32CADataLen);
    if (!u32BufLen)
    {
        return MT_SUCCESS;
    }

    //PVR_Index_GetIdxFileName(strIdxFileName, (MT_CHAR*)pFileName);
    s32Fd = PVR_OPEN(pIdxFileName, PVR_FOPEN_MODE_INDEX_READ);
    if (s32Fd < 0)
    {
         MT_ERR_PVR("open file:%s fail:0x%x\n", pIdxFileName, s32Fd);
         return MT_ERR_PVR_FILE_CANT_OPEN;
    }

     s32Ret = PVR_Index_GetCADataInfo(s32Fd, pInfo, u32BufLen);
     if (s32Ret > 0)
     {
        *u32CADataLen = (MT_U32)s32Ret;
         s32Ret = MT_SUCCESS;
     }
     else
     {
         *u32CADataLen = 0;
         MT_ERR_PVR("PVR_Index_GetCADataInfo fail\n");
     }

     PVR_CLOSE(s32Fd);
     return s32Ret;
}
/*
 virtual a index file
 */
MT_S32 MT_PVR_CreateIdxFile2(const MT_CHAR* pstTsFileName, MT_CHAR* pstIdxFileName, MT_UNF_PVR_GEN_IDX_ATTR_S* pAttr)
{
#define FRAME_SIZE (PVR_TS_LEN * 1024)
    MT_U32 i;
    MT_U32 idxNum;
    MT_U64 tsSize, offset = 0;
    PVR_INDEX_ENTRY_S entry;
    MT_U32 dispSeq = 0;
    MT_U32 clk = 0;

    PVR_FILE fdIdxFile = 0;

    PVR_CHECK_POINTER(pstTsFileName);
    PVR_CHECK_POINTER(pstIdxFileName);
    if (pAttr)
    {
        ;
    }

    tsSize = PVR_FILE_GetFileSize(pstTsFileName);
    if (tsSize < PVR_TS_LEN)
    {
        MT_ERR_PVR("the ts file '%s' size is too small:%llu.\n", pstTsFileName, tsSize);
        return MT_ERR_PVR_INVALID_PARA;
    }

    /* open file, note the right */
    fdIdxFile = open(pstIdxFileName, O_CREAT | O_WRONLY | O_APPEND, 0777);
    if (-1 == fdIdxFile)
    {
        MT_ERR_PVR("can not open file '%s' for write.\n", pstIdxFileName);
        perror("can not create file:");
        return MT_ERR_PVR_FILE_CANT_OPEN;
    }

    idxNum = (MT_U32)tsSize / FRAME_SIZE;

    entry.u161stFrameOfTT = 0;
    entry.u16FrameTypeAndGop = 1;
    entry.u16IndexType  = MT_UNF_PVR_REC_INDEX_TYPE_VIDEO;
    entry.u16UpFlowFlag = 0;
    entry.u32FrameSize = FRAME_SIZE;
    entry.s32CycTimes = 0;
    entry.u64GlobalOffset = PVR_INDEX_INVALID_SEQHEAD_OFFSET;

    for (i = 0; i < idxNum; i++)
    {
        entry.u32DisplayTimeMs = dispSeq++;
        entry.u32PtsMs  = clk;
        entry.u64Offset = offset;

        clk    += 500;
        offset += FRAME_SIZE;

        (void)write(fdIdxFile, (char *)&entry, sizeof(entry));
    }

    if (offset < tsSize)
    {
        entry.u32DisplayTimeMs = dispSeq++;
        entry.u32PtsMs  = clk;
        entry.u64Offset = offset;
        entry.u32FrameSize =(MT_U32)(tsSize - offset);

        (void)write(fdIdxFile, (char *)&entry, sizeof(entry));
    }

    close(fdIdxFile);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_Add_Pid(mt_u32 chnId, mt_u32 u32DmxId, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType)
{
    //MT_S32 ret;
    PVR_REC_CHN_S *pRecChn = NULL;
    int i;

    CHECK_REC_CHNID(chnId);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(chnId);
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = chnId - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID &&
        (p_addon->handlesrec_pvr[recChannelId] == u32DmxId))
        {
            if(p_addon->rec_inter.on_change_pid && MPVR_ADDON_STATUS_OK != p_addon->rec_inter.on_change_pid(p_addon->handlesrec[recChannelId],pid,chnType))
            {
                MT_ERR_PVR("ERROR call addon on_change_pid\n");
                return MT_FAILURE;
            }
        }
    }
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);
	  MT_INFO_PVR("\nchnid=%d\n",chnId);
    for(i=0;i<PVR_REC_MAX_PID;i++){
        if(pRecChn->rec_pid[i].status == PVR_REC_PID_INIT){
            pRecChn->rec_pid[i].DmxRecId = u32DmxId;
            pRecChn->rec_pid[i].pid = (MT_U32)pid;
            pRecChn->rec_pid[i].ChnType = chnType;
            pRecChn->rec_pid[i].status = PVR_REC_PID_PRE_ADD;
            MT_INFO_PVR("\npid[%d]=%d\n",i,pRecChn->rec_pid[i].pid);
            break;
        }
    }
    MT_INFO_PVR("channel %d start ok.\n", chnId);

    PVR_UNLOCK_REC(pRecChn);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_Get_Pids(mt_u32 chnId, int *pid, int *num)
{
   PVR_REC_CHN_S *pRecChn = NULL;
   int i,j=0;

   if((NULL==pid)||(NULL==num)){
      return MT_ERR_PARAM;
   }

   CHECK_REC_CHNID(chnId);

   pRecChn = PVR_GET_RECPTR_BY_CHNID(chnId);

   PVR_LOCK_REC(pRecChn);
   CHECK_REC_CHN_INIT_UNLOCK(pRecChn);
   MT_INFO_PVR("\nchnid=%d\n",chnId);
   for(i=0;i<PVR_REC_MAX_PID;i++){
       if(PVR_REC_PID_ADDED == pRecChn->rec_pid[i].status){
           pid[j]=pRecChn->rec_pid[i].pid;
           j++;
           MT_INFO_PVR("\npid[%d]=%d\n",i,pRecChn->rec_pid[i].pid);
       }
   }
   *num=j;
   PVR_UNLOCK_REC(pRecChn);
   return MT_SUCCESS;
}

MT_S32 MT_PVR_AddDelPid_Ex(mt_u32 chnId, mt_u32 u32DmxId, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType,MT_BOOL flag_adddel,MT_BOOL flag_index)
{
   PVR_REC_CHN_S *pRecChn = NULL;
   int i,ret;

   CHECK_REC_CHNID(chnId);

   pRecChn = PVR_GET_RECPTR_BY_CHNID(chnId);

   PVR_LOCK_REC(pRecChn);
   CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

   if((PVR_REC_MAX_PID==pRecChn->rec_handles) && (MT_TRUE==flag_adddel)){ //add ,full ,return
       PVR_UNLOCK_REC(pRecChn);
       return MT_SUCCESS;
   }
   if((0==pRecChn->rec_handles) && (MT_FALSE==flag_adddel)){ //del ,empty ,return
       PVR_UNLOCK_REC(pRecChn);
       return MT_SUCCESS;
   }
   if(MT_TRUE==flag_adddel){ //add
       for (i=0; i<pRecChn->rec_handles; i++){
           if(pRecChn->rec_pid[i].pid == pid){       //add same,return
               PVR_UNLOCK_REC(pRecChn);
               return MT_SUCCESS;
           }
       }
       pRecChn->rec_pid[pRecChn->rec_handles].DmxRecId = u32DmxId;
       pRecChn->rec_pid[pRecChn->rec_handles].pid = (MT_U32)pid;
       pRecChn->rec_pid[pRecChn->rec_handles].ChnType = chnType;
       pRecChn->rec_pid[pRecChn->rec_handles].status = PVR_REC_PID_PRE_ADD;

       ret = MT_UNF_DMX_AddRecPid(pRecChn->DemuxRecHandle, pid, &pRecChn->rec_pidhandle[pRecChn->rec_handles]);
       if(MT_SUCCESS==ret){
           pRecChn->rec_pid[pRecChn->rec_handles].status = PVR_REC_PID_ADDED;
           pRecChn->rec_handles++;
       }else{
           PVR_UNLOCK_REC(pRecChn);
           return MT_FAILURE;
       }
   }else{  //del pid
       for (i=0; i<pRecChn->rec_handles; i++){
           if(pRecChn->rec_pid[i].pid == pid){       //add same,return
               break;
           }
       }
       if(i!=pRecChn->rec_handles){
           ret = MT_UNF_DMX_DelRecPid(pRecChn->DemuxRecHandle, pRecChn->rec_pidhandle[i]);
           if(MT_SUCCESS!=ret){
               PVR_UNLOCK_REC(pRecChn);
               return MT_FAILURE;
           }
           for(;i<(pRecChn->rec_handles-1); i++){
               pRecChn->rec_pid[i].DmxRecId = pRecChn->rec_pid[i+1].DmxRecId;
               pRecChn->rec_pid[i].pid = pRecChn->rec_pid[i+1].pid;
               pRecChn->rec_pid[i].ChnType = pRecChn->rec_pid[i+1].ChnType;
               pRecChn->rec_pid[i].status = pRecChn->rec_pid[i+1].status;
           }
           pRecChn->rec_handles--;
       }
   }
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_u32 recChannelId = chnId - PVR_REC_START_NUM;
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(recChannelId >= PVR_REC_MIN_DMXID  &&  recChannelId < PVR_REC_MAX_DMXID &&
        (p_addon->handlesrec_pvr[recChannelId] == u32DmxId))
        {
            if(p_addon->rec_inter.on_change_pid_ex && MPVR_ADDON_STATUS_OK != 
            p_addon->rec_inter.on_change_pid_ex(p_addon->handlesrec[recChannelId],pid,chnType,flag_adddel))
            {
                MT_ERR_PVR("ERROR call addon on_change_pid_ex\n");
                return MT_FAILURE;
            }
        }
    }
   PVR_UNLOCK_REC(pRecChn);
   return MT_SUCCESS;
}

MT_U32  PVR_UpdateSingleEventLoopStartFrame(MT_U32 u32ChnID, MT_U32  *startFrame,   MT_U32 *tsFileNodeId)
{
    PVR_REC_CHN_S *pRecChn = NULL;

    CHECK_REC_CHNID(u32ChnID);
    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(startFrame == NULL  || tsFileNodeId == NULL)
    {
        return MT_FAILURE;
    }
    if(pRecChn)
    {
        *startFrame = pRecChn->real_file_start_frame;
        *tsFileNodeId = pRecChn->timeShiftEventTsFileFirstNode;
    }
    else
    {
        *startFrame = 0;
        *tsFileNodeId = 0;
    }
    return MT_SUCCESS;
}

#ifdef SUPPORT_TIMESHIFT_EVENT
static MT_S32 MT_PVR_Delete_Event_TSFile(MT_U8 *fNameEvent,  MT_U32 startNode)
{
    MT_CHAR  nodeFile[PVR_MAX_FILENAME_LEN];
    int i=0;

    (MT_VOID)remove(fNameEvent);    //delete ts file
    for(i=startNode; i<PVR_REC_MAX_NODE;i++)
    {
        memset(nodeFile, 0 , PVR_MAX_FILENAME_LEN);
        snprintf(nodeFile, PVR_MAX_FILENAME_LEN,"%s.%04d", fNameEvent, i);
        PVR_EVENT_D("remove >>>  1nodeFile=%s\n",nodeFile);
        if(PVR_CHECK_FILE_EXIST(nodeFile))
        {
            (MT_VOID)remove(nodeFile);
            PVR_EVENT_D("remove <<<  1 nodeFile=%s\n",nodeFile);
        }
        else
        {
            break;
        }
    }
    return 0;
}
#endif

static MT_S32 MT_PVR_Update_Index_Data(PVR_REC_CHN_S *pRecChn, MT_U32 eventId,
                        MT_U8 *pu8Data,
                        MT_U32 u32Bytes2Write)
{
        int saveSz=0;
        int dstfd = -1;
        //int flag_dosave=0;
        MT_CHAR dstfilename[512]={0};
        PVR_CYC_HEADER_INFO_S  destCycInfo={0,};
        PVR_INDEX_ENTRY_S index_unit = {0,};
        MT_CHAR *filepath=PVR_Get_EventPath(pRecChn->dataFile);
        MT_U32 u32IndexSize = sizeof(PVR_INDEX_ENTRY_S);
        MT_U32 u32HeadInfoSize = (sizeof(PVR_IDX_HEADER_INFO_S) + PVR_MAX_CADATA_LEN + pRecChn->stUserCfg.u32UsrDataInfoSize + sizeof(PVR_INDEX_ENTRY_S))
                                                     / u32IndexSize * u32IndexSize;
        //PVR_EVENT_D("%s==%d, eventId = %d, endFrame=%d \n", __FUNCTION__,__LINE__, eventId,endFrame);
        if(NULL==filepath)
        {
            return MT_ERR_PVR_REC_INVALID_STATE;
        }

        if(strstr((char*)pRecChn->timeshiftEventHandle.event_rec[eventId].u8EventName, ".ts") == NULL) 
            snprintf(dstfilename,sizeof(dstfilename)-1,"%s%s.ts.idx",(char*)filepath, (char*)pRecChn->timeshiftEventHandle.event_rec[eventId].u8EventName);
        else
            snprintf(dstfilename,sizeof(dstfilename)-1,"%s%s.idx",(char*)filepath, (char*)pRecChn->timeshiftEventHandle.event_rec[eventId].u8EventName);

        if(0==pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx){
            pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_BOTH);
            pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx++;
        }
        dstfd=(pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx-1);

        pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame ++;
        destCycInfo.u32StartFrame = pRecChn->real_file_start_frame;
        destCycInfo.u32EndFrame = (MT_U32)pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame;
        destCycInfo.u32LastFrame = (MT_U32)pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame;
        destCycInfo.u32IsRewind = 0;
        destCycInfo.u32TimeShiftEventLoopFirstNode = pRecChn->timeShiftEventTsFileFirstNode;
        saveSz = PVR_WRITE((void *)&destCycInfo, (size_t)sizeof(PVR_CYC_HEADER_INFO_S),
                                 dstfd, (off_t)PVR_GET_HEADER_OFFSET());
         if(saveSz != sizeof(PVR_CYC_HEADER_INFO_S))
         {
                //PVR_CLOSE(dstfd);
                return MT_ERR_PVR_INDEX_DATA_ERR;
         }
         if(u32Bytes2Write != sizeof(PVR_INDEX_ENTRY_S))
         {
               MT_ERR_PVR("%s ====================  %d\n",__FUNCTION__,__LINE__);
         }
         memcpy(&index_unit, pu8Data, sizeof(PVR_INDEX_ENTRY_S));
         index_unit.u64GlobalOffset -= pRecChn->timeshiftEventHandle.event_rec[eventId].u64StartFrame;
         index_unit.u64Offset -=  pRecChn->timeshiftEventHandle.event_rec[eventId].u64StartFrame;
         saveSz = (mt_u32)PVR_WRITE(&index_unit, (size_t)u32Bytes2Write, dstfd,  (u32HeadInfoSize + (MT_U32)(pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame*sizeof(PVR_INDEX_ENTRY_S))));
         if(saveSz != u32Bytes2Write)
         {
                //PVR_CLOSE(dstfd);
                return MT_ERR_PVR_INDEX_DATA_ERR;
         }
         //PVR_EVENT_D("MT_PVR_Update_Index[%d] \n",eventId);
         PVR_EVENT_D("Index[%d] %llx, %x,  %llx, %x, %x, %x, %x, %x\n",
                   eventId,index_unit.u64Offset, u32Bytes2Write, pRecChn->timeshiftEventHandle.event_rec[eventId].u64StartFrame,
                   u32HeadInfoSize, destCycInfo.u32StartFrame,destCycInfo.u32EndFrame,sizeof(PVR_INDEX_ENTRY_S),(off_t)PVR_GET_HEADER_OFFSET());
        //PVR_FSYNC(dstfd);
        //PVR_CLOSE(dstfd);
        if((index_unit.u32DisplayTimeMs-pRecChn->timeshiftEventHandle.event_rec[eventId].lst_dsptime)>3000){
            PVR_EVENT_D("+++do save. %d,%d,%d\n",index_unit.u32DisplayTimeMs,pRecChn->timeshiftEventHandle.event_rec[eventId].lst_dsptime,destCycInfo.u32EndFrame);
            PVR_FSYNC(dstfd);
            PVR_CLOSE(dstfd);
            pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx=0;
            pRecChn->timeshiftEventHandle.event_rec[eventId].lst_dsptime=index_unit.u32DisplayTimeMs;
        }

        return MT_SUCCESS;
}

 MT_S32 MT_PVR_Update_Index_Event_Rec(MT_U32 u32ChnID,
                        MT_U8 *pu8Data,
                        MT_U32 u32Bytes2Write)
 {
    PVR_REC_CHN_S *pRecChn = NULL;
    int i=0;
    int j=0;
    MT_U32 eventId = 0xffffffff;

    CHECK_REC_CHNID(u32ChnID);
    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(NULL == pRecChn)
    {
        return MT_ERR_PVR_INVALID_PARA;
    }
    if (MT_UNF_PVR_REC_STATE_INVALID == pRecChn->enState){//CHECK_REC_CHN_INIT_UNLOCK(pRecChn);
        return MT_ERR_PVR_CHN_NOT_INIT;
    }
    if(MT_FALSE==pRecChn->stUserCfg.bSupportTimeShiftEvent){
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }
    if(pRecChn->timeshiftEventHandle.event_rec== NULL)
    {
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }
    if (!((MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState) ||
      (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState)))
    {
        MT_ERR_PVR("stop event.not running\n");
        MT_USLEEP(20000);
        return MT_ERR_PVR_ALREADY;
    }
    if (MT_UNF_PVR_STREAM_TYPE_TS != pRecChn->stUserCfg.enStreamType){
        MT_ERR_PVR("stop event.not ts\n");
        MT_USLEEP(20000);
        return MT_ERR_PVR_ALREADY;
    }
    if(NULL==pRecChn->IndexHandle){
        MT_ERR_PVR("stop event.no index\n");
        MT_USLEEP(20000);
        return MT_ERR_PVR_NUL_PTR;
    }
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    j = 0;
    for(i=0; i<pRecChn->timeshiftEventHandle.event_total_count; i++)
    {
         if(pRecChn->timeshiftEventHandle.event_rec[i].eventState ==  PVR_EVENT_RECING_SAVE)
         {
            eventId = i;
            if(pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved ==  MT_TRUE)
            {
                MT_PVR_Update_Index_Data(pRecChn, eventId, pu8Data, u32Bytes2Write);
            }
            j ++;
            if(j >= 2)   break;
         }
    }

     if((eventId > pRecChn->timeshiftEventHandle.event_total_count)
     || (( eventId == pRecChn->timeshiftEventHandle.event_total_count ) && ( pRecChn->timeshiftEventHandle.event_total_count != PVR_REC_LOOPTIME_IS_0_MAX_EVENT)))
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_SUCCESS;
    }
    //PVR_EVENT_D("%s==%d, eventId = %d, bSave=%d \n", __FUNCTION__,__LINE__,
     //                                    eventId,pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved);

    if(pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved ==  MT_FALSE)
    {
	 pRecChn->timeshiftEventHandle.event_rec[eventId].u64StartFrame = 0;
	 pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame = 0;
        PVR_UNLOCK_REC(pRecChn);
        PVR_EVENT_D("%s==%d, eventId = %d, bSave=%d \n", __FUNCTION__,__LINE__, eventId,pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved);
        MT_PVR_Save_Event_Rec(u32ChnID, (MT_CHAR*)pRecChn->timeshiftEventHandle.event_rec[eventId].u8EventName,  MT_TRUE);

        return MT_SUCCESS;
    }

    PVR_UNLOCK_REC(pRecChn);
    return MT_SUCCESS;
 }

MT_S32 MT_PVR_Start_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName, MT_U32    u32StartTimeMs,  PVR_TIMESHIFT_EVENT_TYPE  type)
{
#ifdef SUPPORT_TIMESHIFT_EVENT
    PVR_REC_CHN_S *pRecChn = NULL;
    MT_U32 waittime=1000*1000;
    MT_U32 eventId = 0xffffffff;
    MT_U32 u32CurrentTime=0;
    MT_U32 i = 0;
    PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);

    if(NULL==pEeventName){
	 MT_ERR_PVR("pEeventName is null !!\n");
        return MT_ERR_PVR_INVALID_PARA;
    }
    if(0==pEeventName[0]){
	 MT_ERR_PVR("pEeventName is null !!\n");
        return MT_ERR_PVR_INVALID_PARA;
    }
    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(NULL == pRecChn)
    {
        MT_ERR_PVR("MT_ERR_PVR_INVALID_PARA !! u32ChnID = 0x%x\n",u32ChnID);
        return MT_ERR_PVR_INVALID_PARA;
    }
    if(pRecChn->timeshiftEventHandle.event_rec == NULL)
    {
        MT_ERR_PVR("pRecChn->timeshiftEventHandle.event_rec is null !!\n");
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }

    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    if (!((MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState)||
          (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState)))
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_ALREADY;
    }

    if (MT_UNF_PVR_STREAM_TYPE_TS != pRecChn->stUserCfg.enStreamType){
        PVR_UNLOCK_REC(pRecChn);
	 MT_ERR_PVR("pRecChn->stUserCfg.enStreamType !=  MT_UNF_PVR_STREAM_TYPE_TS !\n");		
        return MT_ERR_PVR_ALREADY;
    }

    if (MT_FALSE  == pRecChn->stUserCfg.bSupportTimeShiftEvent){
        PVR_UNLOCK_REC(pRecChn);
	 MT_ERR_PVR("pRecChn->stUserCfg.bSupportTimeShiftEvent is False !!\n");	
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }
	
    if(NULL==pRecChn->IndexHandle){
        PVR_UNLOCK_REC(pRecChn);
	 MT_ERR_PVR("NULL==pRecChn->IndexHandle !!\n");		
        return MT_ERR_PVR_NUL_PTR;
    }
    //pRecChn->timeshiftEventHandle.event_rec[0] : save timeshift as event[0]
    for(i=1; i<pRecChn->timeshiftEventHandle.event_total_count; i++)
    {

        if(pRecChn->timeshiftEventHandle.event_rec[i].eventState == PVR_EVENT_IDLE)
        {
            eventId = i;
            break;
        }
        else
        {
            if( !strcmp(pRecChn->timeshiftEventHandle.event_rec[i].u8EventName, pEeventName))
            {
                MT_ERR_PVR("event is occupy !!\n");
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_BUSY;
            }
        }
    }

    if (i >= pRecChn->timeshiftEventHandle.event_total_count)
    {
    	 TimeShift_REC_EVENT_S *tmp =  NULL;
        MT_U32 original_event_count = pRecChn->timeshiftEventHandle.event_total_count;

        if(pRecChn->timeshiftEventHandle.event_total_count == PVR_REC_LOOPTIME_IS_0_MAX_EVENT)
        {
                MT_ERR_PVR("eventId(%d)  >= event_total_count(2)\n",eventId);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_TIMESHIFT_EVENT_BUSY;
        }
        pRecChn->timeshiftEventHandle.event_total_count += PVR_REC_MAX_EVENT;
        tmp =  (TimeShift_REC_EVENT_S*)malloc(pRecChn->timeshiftEventHandle.event_total_count*sizeof(TimeShift_REC_EVENT_S));
        memset(tmp, 0, (pRecChn->timeshiftEventHandle.event_total_count*sizeof(TimeShift_REC_EVENT_S)));
		memcpy(tmp, pRecChn->timeshiftEventHandle.event_rec, original_event_count*sizeof(TimeShift_REC_EVENT_S));
		if (pRecChn->timeshiftEventHandle.event_rec)
		{
	        free(pRecChn->timeshiftEventHandle.event_rec);
			pRecChn->timeshiftEventHandle.event_rec = MT_NULL;
		}
		pRecChn->timeshiftEventHandle.event_rec = tmp;

		eventId = original_event_count;
    }

    strcpy(pRecChn->timeshiftEventHandle.event_rec[eventId].u8EventName,pEeventName);

    if(!strcmp(pEeventName, pRecChn->stUserCfg.szFileName))
    {
    	 pRecChn->timeshiftEventHandle.event_rec[i].eventState=PVR_EVENT_RECING;
    }
    else
    {
        pRecChn->timeshiftEventHandle.event_rec[eventId].eventState=PVR_EVENT_START;
    }
    pRecChn->timeshiftEventHandle.event_rec[eventId].u32EventStartTime = u32StartTimeMs;

    pRecChn->timeshiftEventHandle.event_rec[eventId].eventType = type;
    pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved = MT_FALSE;
    PVR_EVENT_D("%s >>>> %d      eventId=%d, pEeventName=%s, timeshift_name=%s \n",
		__FUNCTION__,__LINE__, eventId,pEeventName,pRecChn->stUserCfg.szFileName);

    PVR_UNLOCK_REC(pRecChn);
#endif
    return MT_SUCCESS;
}

/*
func:stop a event rec, which be linked to u32ChnID
*/
MT_S32 MT_PVR_Stop_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName)
{
#ifdef SUPPORT_TIMESHIFT_EVENT
    PVR_REC_CHN_S *pRecChn = NULL;
    MT_U32 waittime=1000*1000;
    MT_U32 u32CurrentTime=0;
    MT_U32 eventId = 0xffffffff;
    int i=0;
    CHECK_REC_CHNID(u32ChnID);

    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);

    if(NULL == pRecChn)
    {
        MT_ERR_PVR("pEeventName is null !!\n");
        return MT_ERR_PVR_INVALID_PARA;
    }
    if(NULL == pRecChn->timeshiftEventHandle.event_rec)
    {
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }

    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    if (MT_FALSE  == pRecChn->stUserCfg.bSupportTimeShiftEvent){
        PVR_UNLOCK_REC(pRecChn);
	 MT_ERR_PVR("pRecChn->stUserCfg.bSupportTimeShiftEvent is False !!\n");
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }

    (void)MT_PVR_SysGetTimeStampMs(&(u32CurrentTime));
    if(u32CurrentTime - pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u32EventStartTime < PVR_TIMESHIFT_TIME_LIMIT)
    {
        MT_ERR_PVR("pvr timeshift event operate is too frequent\n");
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_TIMESHIFT_EVENT_BUSY;
    }
    pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u32EventStartTime = u32CurrentTime;


    if (!((MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState)||
          (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState)))
    {
        PVR_UNLOCK_REC(pRecChn);
        MT_ERR_PVR("stop event.not running\n");
        return MT_ERR_PVR_ALREADY;
    }

    if (MT_UNF_PVR_STREAM_TYPE_TS != pRecChn->stUserCfg.enStreamType){
        PVR_UNLOCK_REC(pRecChn);
        MT_ERR_PVR("stop event.not ts\n");
        return MT_ERR_PVR_ALREADY;
    }

    if(NULL==pRecChn->IndexHandle){
        MT_ERR_PVR("stop event.no index\n");
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_NUL_PTR;
    }

    PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
    for(i=0; i<pRecChn->timeshiftEventHandle.event_total_count; i++)
    {
         PVR_EVENT_D("%s >>>> %s      eventId=%d, %d\n",pRecChn->timeshiftEventHandle.event_rec[i].u8EventName,pEeventName,i, pRecChn->timeshiftEventHandle.event_rec[i].eventState);
         if(( !strcmp(pRecChn->timeshiftEventHandle.event_rec[i].u8EventName, pEeventName))
                && ((PVR_EVENT_RECING == pRecChn->timeshiftEventHandle.event_rec[i].eventState) || (PVR_EVENT_RECING_SAVE == pRecChn->timeshiftEventHandle.event_rec[i].eventState)))
         {
            eventId = i;
            if(i == 0)
            {
                 if(1)  //save ixd_event0
                {
                     int srcfd,dstfd,filesize;
                     size_t cpsize=0;
                     off_t offset=0;
                     MT_CHAR srcfilename[256]={0};
                     MT_CHAR dstfilename[256]={0};
                     MT_U8 tmpmem[64]={0};
                     MT_CHAR *filepath=PVR_Get_EventPath(pRecChn->dataFile);
                     if(NULL==filepath)
                     {
                         MT_ERR_PVR("attr filepath is NULL\n");
                         PVR_UNLOCK_REC(pRecChn);
                         return MT_ERR_PVR_REC_INVALID_STATE;
                     }
                     //sprintf(srcfilename,"%s.attr",pRecChn->stUserCfg.szFileName);
                     if(strstr(pEeventName, ".ts") == NULL)
                     {
                        sprintf(srcfilename,"%s%s.ts.idx",filepath,pEeventName);
                        sprintf(dstfilename,"%s%s.ts.idx_event0",filepath,pEeventName);
                     }
                     else
                     {
                        sprintf(srcfilename,"%s%s.idx",filepath,pEeventName);
                        sprintf(dstfilename,"%s%s.idx_event0",filepath,pEeventName);
                     }
                     PVR_EVENT_D("...cut0:%s,%s\n",srcfilename,dstfilename);
                     srcfd = PVR_OPEN(srcfilename, PVR_FOPEN_MODE_DATA_READ);
                     dstfd = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_WRITE);
             				filesize = PVR_SEEK(srcfd , 0 , SEEK_END);
             				PVR_SEEK(srcfd , 0 , SEEK_SET);
                     while(filesize>0){
                         if(filesize>=64){
                             cpsize=64;
                         }else{
                             cpsize=(size_t)filesize;
                         }
                         PVR_READ(tmpmem,cpsize,srcfd,offset);
                         PVR_WRITE(tmpmem,cpsize,dstfd,offset);
                         filesize-=64;
                         offset+=64;
                     }
                     PVR_FSYNC(dstfd);
                     PVR_CLOSE(srcfd);
                     PVR_CLOSE(dstfd);
                     PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
                 }

            }

            break;
          }
    }

     if((eventId > pRecChn->timeshiftEventHandle.event_total_count)
     || (( eventId == pRecChn->timeshiftEventHandle.event_total_count ) && ( pRecChn->timeshiftEventHandle.event_total_count != PVR_REC_LOOPTIME_IS_0_MAX_EVENT)))
    {
        MT_ERR_PVR("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_TIMESHIFT_EVENT_BUSY;
    }
    PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
    PVR_EVENT_D("%s >>>> %d      eventId=%d\n",pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u8EventName,
                                                pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState,eventId);

    if( !strcmp(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u8EventName, pEeventName))
    {
         if(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState == PVR_EVENT_APPEND_SAVE)
        {
            //ecChn->timeshiftEventHandle.event_rec[eventId].bSaved =  MT_FALSE;
            pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_STOP;
            pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState = PVR_EVENT_IDLE;
            PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
            PVR_UNLOCK_REC(pRecChn);
            return MT_SUCCESS;
        }

    }

    pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_STOP;
    PVR_EVENT_D("%s >>>> %d      event_rec[%d].bSaved=%d, eventState=%d\n",__FUNCTION__,__LINE__,
                eventId, pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved,pRecChn->timeshiftEventHandle.event_rec[eventId].eventState);
    /*
    if((pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved == MT_TRUE)
        && ((pRecChn->timeshiftEventHandle.event_rec[eventId].eventState  == PVR_EVENT_STOP)
        || (pRecChn->timeshiftEventHandle.event_rec[eventId].eventState  == PVR_EVENT_STOP_SAVE)))
    {
        pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_IDLE;
        memset(&pRecChn->timeshiftEventHandle.event_rec[eventId], 0, sizeof(PVR_EVENT_S));
    }
    */
    memcpy(&pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event,  &(pRecChn->timeshiftEventHandle.event_rec[eventId]), sizeof(TimeShift_REC_EVENT_S));
    if(0!=pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx){
        PVR_FSYNC((pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx-1));
        PVR_CLOSE((pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx-1));
        pRecChn->timeshiftEventHandle.event_rec[eventId].fd_idx=0;
    }

    PVR_UNLOCK_REC(pRecChn);
#endif
    return MT_SUCCESS;
}


/*
func:change event-timeshift to pvr
*/
MT_S32 MT_PVR_Save_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pEeventName,  MT_BOOL  internalCall)
{
#ifdef SUPPORT_TIMESHIFT_EVENT
    MT_S32 ret=0;
    MT_U64 s=0, m=0, e=0;
    PVR_REC_CHN_S *pRecChn = NULL;
    PVR_IDX_HEADER_INFO_S *stIdxHeaderInfo=NULL;
    MT_U32 u32HeadInfoSize = 0, u32IndexSize=0;
    MT_U32 eventId = 0xffffffff;

    MT_U32  u32CurrentTime = 0;
    int i=0;

    if(NULL==pEeventName){
	 MT_ERR_PVR("pEeventName is null !!\n");	
        return MT_ERR_PVR_INVALID_PARA;
    }

    if(0==pEeventName[0]){
	 MT_ERR_PVR("pEeventName is null !!\n");
        return MT_ERR_PVR_INVALID_PARA;
    }
    CHECK_REC_CHNID(u32ChnID);
    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(NULL == pRecChn)
    {
        return MT_ERR_PVR_INVALID_PARA;
    }
	
    if (MT_FALSE  == pRecChn->stUserCfg.bSupportTimeShiftEvent){
        PVR_UNLOCK_REC(pRecChn);
	    MT_ERR_PVR("pRecChn->stUserCfg.bSupportTimeShiftEvent is False !!\n");
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }
	
    if(pRecChn->timeshiftEventHandle.event_rec == NULL)
    {
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }

    PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    (void)MT_PVR_SysGetTimeStampMs(&(u32CurrentTime));
    if((u32CurrentTime - pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u32EventStartTime < PVR_TIMESHIFT_TIME_LIMIT) && (internalCall == MT_FALSE))
    {
        MT_ERR_PVR("pvr timeshift event operate is too frequent\n");
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_TIMESHIFT_EVENT_BUSY;
    }
    pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u32EventStartTime = u32CurrentTime;

    for(i=0; i<pRecChn->timeshiftEventHandle.event_total_count; i++)
    {
         PVR_EVENT_D("%s >>>> %s      eventId=%d, %d\n",pRecChn->timeshiftEventHandle.event_rec[i].u8EventName,pEeventName,i, pRecChn->timeshiftEventHandle.event_rec[i].eventState);
         if( !strcmp(pRecChn->timeshiftEventHandle.event_rec[i].u8EventName, pEeventName))
         {
            eventId = i;
            if(i == 0)
            {
                  if((PVR_EVENT_STOP_SAVE == pRecChn->timeshiftEventHandle.event_rec[0].eventState)
                     || (PVR_EVENT_RECING == pRecChn->timeshiftEventHandle.event_rec[eventId].eventState))
                  {
                        PVR_Rec_SaveTimeShiftEvent0Data(pRecChn->dataFile, pRecChn->timeshiftEventHandle.event_rec[0].eventState);
                        pRecChn->timeshiftEventHandle.event_rec[0].bSaved = MT_TRUE;
                        //1.create attr file
                        {
                            int srcfd,dstfd,filesize;
                            size_t cpsize=0;
                            off_t offset=0;
                            MT_CHAR srcfilename[256]={0};
                            MT_CHAR dstfilename[256]={0};
                            MT_U8 tmpmem[64]={0};
                            MT_CHAR *filepath=PVR_Get_EventPath(pRecChn->dataFile);
                            if(NULL==filepath)
                            {
                                MT_ERR_PVR("attr filepath is NULL\n");
                                if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
                                PVR_UNLOCK_REC(pRecChn);
                                return MT_ERR_PVR_REC_INVALID_STATE;
                            }
                            //sprintf(srcfilename,"%s.attr",pRecChn->stUserCfg.szFileName);
                            sprintf(srcfilename,"%s%s.attr",filepath,pEeventName);
                            if(strstr(pEeventName, ".ts") == NULL)   sprintf(dstfilename,"%s%s.ts.attr",filepath,pEeventName);
                            else   sprintf(dstfilename,"%s%s.attr",filepath,pEeventName);
                            PVR_EVENT_D("...cut0:%s,%s\n",srcfilename,dstfilename);
                            srcfd = PVR_OPEN(srcfilename, PVR_FOPEN_MODE_DATA_READ);
                            dstfd = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_WRITE);
                    				filesize = PVR_SEEK(srcfd , 0 , SEEK_END);
                    				PVR_SEEK(srcfd , 0 , SEEK_SET);
                            while(filesize>0){
                                if(filesize>=64){
                                    cpsize=64;
                                }else{
                                    cpsize=(size_t)filesize;
                                }
                                PVR_READ(tmpmem,cpsize,srcfd,offset);
                                PVR_WRITE(tmpmem,cpsize,dstfd,offset);
                                filesize-=64;
                                offset+=64;
                            }
                            PVR_FSYNC(dstfd);
                            PVR_CLOSE(srcfd);
                            PVR_CLOSE(dstfd);
                            PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
                        }
                        PVR_UNLOCK_REC(pRecChn);

                        return 0;
                  }
                  else
                  {
                        MT_ERR_PVR("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
                        PVR_UNLOCK_REC(pRecChn);
                        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
                  }
            }
            break;
          }
    }
    if((eventId > pRecChn->timeshiftEventHandle.event_total_count)
     || (( eventId == pRecChn->timeshiftEventHandle.event_total_count ) && ( pRecChn->timeshiftEventHandle.event_total_count != PVR_REC_LOOPTIME_IS_0_MAX_EVENT)))
    {
        MT_ERR_PVR("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_TIMESHIFT_EVENT_BUSY;
    }
#if 1   //save---->stop
    if(PVR_EVENT_RECING == pRecChn->timeshiftEventHandle.event_rec[eventId].eventState)
    {
        pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_RECING_SAVE;
        PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
        PVR_UNLOCK_REC(pRecChn);
        return MT_SUCCESS;
    }
    if(1)
    {
        MT_U32  count=0;
        while(PVR_EVENT_START == pRecChn->timeshiftEventHandle.event_rec[eventId].eventState)
        {//fix bug128982
            MT_USLEEP(10000);
            count ++;
            if(count >= 1000)  break;
        }
    }
    PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
    if(pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved == MT_TRUE)
    {
        PVR_EVENT_D("%s ==== %s      pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState=%d\n",__FUNCTION__,
                                        pEeventName,pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState);

        if( !strcmp(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u8EventName, pEeventName))
        {
             if(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState != PVR_EVENT_APPEND_SAVE)
            {
                pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved =  MT_FALSE;
                pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_RECING_SAVE;
                pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState = PVR_EVENT_APPEND_SAVE;
                PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
                PVR_UNLOCK_REC(pRecChn);
                return MT_SUCCESS;
            }
            else
            {
                MT_ERR_PVR("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_INVALID_PARA;
            }
        }
    }
#endif
    //stop---->save
    if((PVR_EVENT_STOP_SAVE != pRecChn->timeshiftEventHandle.event_rec[eventId].eventState)
        && (PVR_EVENT_RECING_SAVE != pRecChn->timeshiftEventHandle.event_rec[eventId].eventState))
    {
        MT_ERR_PVR("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_INVALID_PARA;
    }
    PVR_EVENT_D("%s >>>> %d      bSave=%d, %s  %s,   pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState=%d\n",__FUNCTION__,__LINE__,
                    pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved,
                    pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u8EventName,
                    pEeventName,
                    pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState);
#if 0
    if(pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved == MT_TRUE)
    {
        if( !strcmp(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u8EventName, pEeventName))
        {
            event_continue_rec = MT_TRUE;
        }
    }
#else
PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
    if(pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved == MT_TRUE)
    {
        PVR_EVENT_D("%s ==== %s      pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState=%d\n",__FUNCTION__,
                                        pEeventName,pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState);

        if( !strcmp(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.u8EventName, pEeventName))
        {
             if(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState != PVR_EVENT_APPEND_SAVE)
            {
                pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved =  MT_FALSE;
                pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_RECING_SAVE;
                pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState = PVR_EVENT_APPEND_SAVE;
                PVR_EVENT_D("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
                PVR_UNLOCK_REC(pRecChn);
                return MT_SUCCESS;
            }
            else
            {
                MT_ERR_PVR("%s >>>> %d      eventId=%d\n",__FUNCTION__,__LINE__,eventId);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_INVALID_PARA;
            }
        }
    }
#endif
    u32IndexSize = sizeof(PVR_INDEX_ENTRY_S);
    u32HeadInfoSize = (sizeof(PVR_IDX_HEADER_INFO_S) + PVR_MAX_CADATA_LEN + pRecChn->stUserCfg.u32UsrDataInfoSize + sizeof(PVR_INDEX_ENTRY_S))
                      / u32IndexSize * u32IndexSize;
    stIdxHeaderInfo=(PVR_IDX_HEADER_INFO_S *)malloc(u32HeadInfoSize);
    if(NULL==stIdxHeaderInfo)
    {
        PVR_UNLOCK_REC(pRecChn);
        return -1;
    }
	memset(stIdxHeaderInfo, 0x00, u32HeadInfoSize);

    ret=PVR_Rec_GetEventIndexDataEntry(pRecChn->dataFile,
                                pEeventName, stIdxHeaderInfo, &s, &e,
                                pRecChn->timeshiftEventHandle.event_rec[eventId].eventState);
    if(0!=ret)
    {
        if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
        MT_ERR_PVR("change event.find no event\n");
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_REC_INVALID_STATE;
    }
    PVR_EVENT_D("s = %llx  e = %llx\n",s,e);
    if(MT_FALSE == pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved)
    {
        //1.create attr file
        {
            int srcfd,dstfd,filesize;
            size_t cpsize=0;
            off_t offset=0;
            MT_CHAR srcfilename[256]={0};
            MT_CHAR dstfilename[256]={0};
            MT_U8 tmpmem[64]={0};
            MT_CHAR *filepath=PVR_Get_EventPath(pRecChn->dataFile);
            if(NULL==filepath)
            {
                MT_ERR_PVR("attr filepath is NULL\n");
                if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_REC_INVALID_STATE;
            }
            sprintf(srcfilename,"%s.attr",pRecChn->stUserCfg.szFileName);
            if(strstr(pEeventName, ".ts") == NULL)   sprintf(dstfilename,"%s%s.ts.attr",filepath,pEeventName);
            else  sprintf(dstfilename,"%s%s.attr",filepath,pEeventName);
            PVR_EVENT_D("...cut0:%s,%s\n",srcfilename,dstfilename);
            srcfd = PVR_OPEN(srcfilename, PVR_FOPEN_MODE_DATA_READ);
            dstfd = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_WRITE);
    				filesize = PVR_SEEK(srcfd , 0 , SEEK_END);
    				PVR_SEEK(srcfd , 0 , SEEK_SET);
            while(filesize>0){
                if(filesize>=64){
                    cpsize=64;
                }else{
                    cpsize=(size_t)filesize;
                }
                PVR_READ(tmpmem,cpsize,srcfd,offset);
                PVR_WRITE(tmpmem,cpsize,dstfd,offset);
                filesize-=64;
                offset+=64;
            }
            PVR_FSYNC(dstfd);
            PVR_CLOSE(srcfd);
            PVR_CLOSE(dstfd);
            PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
        }
        //2.create ca file

        if(pRecChn->stUserCfg.stEncryptCfg.bDoCipher == MT_TRUE)        {
            int srcfd,dstfd,filesize;
            size_t cpsize=0;
            off_t offset=0;
            MT_CHAR srcfilename[256]={0};
            MT_CHAR dstfilename[256]={0};
            MT_U8 tmpmem[64]={0};
            MT_U8 *tmp=NULL;

            MT_CHAR *filepath=PVR_Get_EventPath(pRecChn->dataFile);
            if(NULL==filepath){
                MT_ERR_PVR("ca filepath is NULL\n");
                if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
                PVR_UNLOCK_REC(pRecChn);
                return MT_ERR_PVR_REC_INVALID_STATE;
            }
            sprintf(srcfilename,"%s.ca",pRecChn->stUserCfg.szFileName);
            if(strstr(pEeventName, ".ts") == NULL)   sprintf(dstfilename,"%s%s.ts.ca",filepath,pEeventName);
            else  sprintf(dstfilename,"%s%s.ca",filepath,pEeventName);
            PVR_EVENT_D("...cut0:%s,%s\n",srcfilename,dstfilename);
            srcfd = PVR_OPEN(srcfilename, PVR_FOPEN_MODE_DATA_READ);
            dstfd = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_WRITE);
    	     filesize = PVR_SEEK(srcfd , 0 , SEEK_END);
    	     PVR_SEEK(srcfd , 0 , SEEK_SET);
            while(filesize>0){
                if(filesize>=64){
                    cpsize=64;
                }else{
                    cpsize=(size_t)filesize;
                }
                PVR_READ(tmpmem,cpsize,srcfd,offset);
                PVR_WRITE(tmpmem,cpsize,dstfd,offset);
                filesize-=64;
                offset+=64;
            }
            PVR_FSYNC(dstfd);
            PVR_CLOSE(srcfd);
            PVR_CLOSE(dstfd);
            PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
        }
    }

    //3.create index file
    {
        int srcfd,dstfd;
        MT_CHAR szIndexName[256]={0};
        MT_CHAR dstfilename[256]={0};
        MT_CHAR *filepath=PVR_Get_EventPath(pRecChn->dataFile);
        PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
        if(NULL==filepath){
            MT_ERR_PVR("index filepath is NULL\n");
            if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
            PVR_UNLOCK_REC(pRecChn);
            return MT_ERR_PVR_REC_INVALID_STATE;
        }
        sprintf(szIndexName,"%s.idx",pRecChn->stUserCfg.szFileName);
        if(strstr(pEeventName, ".ts") == NULL)   sprintf(dstfilename,"%s%s.ts.idx",filepath,pEeventName);
        else   sprintf(dstfilename,"%s%s.idx",filepath,pEeventName);
        PVR_EVENT_D("...cut0:%s,%s,  filepath=%s %s\n",szIndexName,dstfilename,filepath,pRecChn->stUserCfg.szFileName);
        srcfd = PVR_OPEN(szIndexName, PVR_FOPEN_MODE_INDEX_READ);
        dstfd = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_WRITE);
        pRecChn->timeshiftEventHandle.event_rec[eventId].u64StartFrame = s;
        pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame = PVR_Index_CutEventPart(srcfd,dstfd, stIdxHeaderInfo, u32HeadInfoSize, s, e);
        PVR_FSYNC(dstfd);
        PVR_CLOSE(srcfd);
        PVR_CLOSE(dstfd);
        pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved = MT_TRUE;
        if(0 > pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame)
        {
            PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
            if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
            PVR_UNLOCK_REC(pRecChn);
            return MT_ERR_PVR_REC_INVALID_STATE;
        }
        /*
        * if ts file is delete while event loop, create ts file
        */
        if(1)
        {
            MT_CHAR dstfilename[256]={0};
            memset(dstfilename, 0, 256);
            if(strstr(pEeventName, ".ts") == NULL)   sprintf(dstfilename,"%s%s.ts",filepath,pEeventName);
            else   sprintf(dstfilename,"%s%s",filepath,pEeventName);
            PVR_EVENT_D("%s===========ts file: %s\n",__FUNCTION__,dstfilename);
            if(!PVR_CHECK_FILE_EXIST(dstfilename))
            {
                int dstfd;
                MT_U8 *tmp = "tmpForTimeshiftEvent";
                dstfd = PVR_OPEN(dstfilename, PVR_FOPEN_MODE_INDEX_WRITE);
                PVR_WRITE(tmp, strlen(tmp), dstfd, 0);
                PVR_CLOSE(dstfd);
                PVR_EVENT_D("%s============%d\n",__FUNCTION__,__LINE__);
            }
        }
    }

    PVR_EVENT_D("%s============%d, u64EndFrame=0x%llx\n",__FUNCTION__,__LINE__,pRecChn->timeshiftEventHandle.event_rec[eventId].u64EndFrame);
    if(stIdxHeaderInfo)   free(stIdxHeaderInfo);
    //free event_unit
    PVR_EVENT_D("%s >>>> %d      event_rec[%d].bSaved=%d, eventState=%d\n",__FUNCTION__,__LINE__,
                eventId, pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved,pRecChn->timeshiftEventHandle.event_rec[eventId].eventState);
    /*
    if((pRecChn->timeshiftEventHandle.event_rec[eventId].bSaved == MT_TRUE) && (pRecChn->timeshiftEventHandle.event_rec[eventId].eventState  == PVR_EVENT_STOP_SAVE))
    {
        pRecChn->timeshiftEventHandle.event_rec[eventId].eventState = PVR_EVENT_IDLE;
        memset(&pRecChn->timeshiftEventHandle.event_rec[eventId], 0, sizeof(PVR_EVENT_S));
    }
    */

    PVR_UNLOCK_REC(pRecChn);
#endif
    return MT_SUCCESS;
}


/*
func:delet a event rec, which be linked to u32ChnID

1、can`t delete ts, while recording
2、can`t delete ts file only, without idx file
3、((u32ChnID < PVR_REC_START_NUM) || (u32ChnID >= (PVR_REC_MAX_CHN_NUM + PVR_REC_START_NUM)))  express no
*/
MT_S32 MT_PVR_Delete_Event_Rec(MT_U32 u32ChnID, MT_CHAR *pTimeShiftPath, MT_CHAR *pEeventName)
{
#ifdef SUPPORT_TIMESHIFT_EVENT
    MT_U32 waittime=1000*1000;
    MT_CHAR  fNameEvent[PVR_MAX_FILENAME_LEN];
    //MT_CHAR *filepath= NULL;

    if(pEeventName == NULL)
    {
        MT_ERR_PVR("pEeventName is null !!\n");
        return MT_ERR_PVR_INVALID_PARA;
    }
    memset(fNameEvent, 0, PVR_MAX_FILENAME_LEN);
    if(pTimeShiftPath)
    {
        strcpy(fNameEvent, pTimeShiftPath);
        strcat(fNameEvent, pEeventName);
    }
    else
    {
        strcpy(fNameEvent, pEeventName);
    }

    if((strstr(fNameEvent, "*.*") != NULL) && pTimeShiftPath)
    {
        (MT_VOID)remove(pTimeShiftPath);
        PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>> delete all  fNameEvent=%s\n",fNameEvent);
        return MT_SUCCESS;
    }

    if(strstr(fNameEvent, ".ts") == NULL)
    {
        strcat(fNameEvent, ".ts");
    }
    PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>>  tmp_cell->cellname=%s\n",fNameEvent);

    if(pEeventName && PVR_CHECK_FILE_EXIST(fNameEvent))
    {
        MT_CHAR  tmp[PVR_MAX_FILENAME_LEN];

        memset(tmp, 0, PVR_MAX_FILENAME_LEN);
        sprintf(tmp, "%s%s", fNameEvent,".attr");
        if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);

        memset(tmp, 0, PVR_MAX_FILENAME_LEN);
        sprintf(tmp, "%s%s", fNameEvent, ".ca");
        if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);

        memset(tmp, 0, PVR_MAX_FILENAME_LEN);
        sprintf(tmp, "%s%s", fNameEvent, ".idx");
        PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>> %d\n",__LINE__);
        if(PVR_CHECK_FILE_EXIST(tmp))  //idx exist
        {
            int indexFd = PVR_OPEN(tmp, PVR_FOPEN_MODE_INDEX_READ);
            PVR_IDX_HEADER_INFO_S pHeadInfo = {0,};
            if (MT_SUCCESS != PVRIndexGetHeaderInfo(indexFd, &pHeadInfo))
            {
                return MT_ERR_PVR_INDEX_DATA_ERR;
            }
            PVR_CLOSE(indexFd);
            //remove share ts
	     if ((u32ChnID < PVR_REC_START_NUM) || (u32ChnID >= (PVR_REC_MAX_CHN_NUM + PVR_REC_START_NUM)))    //no running
            {
                MT_CHAR  tmp1[PVR_MAX_FILENAME_LEN];
                MT_CHAR  tmp2[PVR_MAX_FILENAME_LEN];
                memset(tmp1, 0, PVR_MAX_FILENAME_LEN);
                memset(tmp2, 0, PVR_MAX_FILENAME_LEN);
                if((pHeadInfo.u8ShareEventCount > 0)
                    && ((strstr(pHeadInfo.shareEvent[0].shareFileName, ".ts") != NULL) ||(strstr(pHeadInfo.shareEvent[0].shareFileName, ".dat") != NULL)))
                {
                    int i=0;
                    PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>>pHeadInfo.u8ShareEventCount = %d\n",pHeadInfo.u8ShareEventCount);
                    for(i=0; i<pHeadInfo.u8ShareEventCount; i++)
                    {
                          memset(tmp1, 0, PVR_MAX_FILENAME_LEN);
                          if(pTimeShiftPath)   strcpy(tmp1, pTimeShiftPath);
                          if(strstr(pHeadInfo.shareEvent[i].shareFileName, ".ts") != NULL)
                          {
                              strcat(tmp1, pHeadInfo.shareEvent[i].shareFileName);
                              strcpy(tmp2, tmp1);
                              strcat(tmp2, ".idx");
                              if(PVR_CHECK_FILE_EXIST(tmp1))
                              {
                                  PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>> %d\n",__LINE__);
                                  if(!PVR_CHECK_FILE_EXIST(tmp2))
                                  {                  //delete other ts file
                                        PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>> %d\n",__LINE__);
                                        if(pHeadInfo.stCycInfo.u32TimeShiftEventLoopFirstNode > 0)    MT_PVR_Delete_Event_TSFile(tmp1, pHeadInfo.stCycInfo.u32TimeShiftEventLoopFirstNode);
                                        else MT_PVR_Delete_Event_TSFile(tmp1, 1);
                                  }
                                  else   //modify index, delete shareFlag
                                  {
                                        indexFd = PVR_OPEN(tmp2, PVR_FOPEN_MODE_INDEX_READ);
                                        if (MT_SUCCESS != PVRIndexGetHeaderInfo(indexFd, &pHeadInfo))
                                       {
                                           return MT_ERR_PVR_INDEX_DATA_ERR;
                                       }
                                       PVR_CLOSE(indexFd);
                                       pHeadInfo.shareFlagTsFile = 0;
                                       PVR_EVENT_D("modify tmp2=%s  shareFlagTsFile=0\n",tmp2);
                                       PVRIndexSaveHeaderInfo(tmp2, &pHeadInfo);
                                  }
                              }
                              PVR_EVENT_D("remove <<<  ttmp=%s\n",tmp1);
                          }
                    }
                }
                else
                {
                      PVR_EVENT_D("MT_PVR_Delete_Event_Rec >>> %d\n",__LINE__);
                }
                if(pHeadInfo.shareFlagTsFile == 0x0)
                {
                    int i=0;

                    if(pHeadInfo.stCycInfo.u32TimeShiftEventLoopFirstNode > 0)
                        MT_PVR_Delete_Event_TSFile(fNameEvent, pHeadInfo.stCycInfo.u32TimeShiftEventLoopFirstNode);
                    else
                       MT_PVR_Delete_Event_TSFile(fNameEvent, 1);

                    PVR_EVENT_D("remove <<<  0  tmp_cell->cellname=%s\n",fNameEvent);
                }
            }
            (MT_VOID)remove(tmp);  //delete idx file
	     PVR_EVENT_D("remove <<<  tmp=%s\n",tmp);
        }
        else
        {
            //if(pRecChn->timeshiftEventHandle.g_current_timeshift_rec_event.eventState == PVR_EVENT_IDLE)
            /*
     	     if ((u32ChnID < PVR_REC_START_NUM) || (u32ChnID >= (PVR_REC_MAX_CHN_NUM + PVR_REC_START_NUM)))   //no running
            {
                 PVR_EVENT_D("MT_PVR_Delete_Event_Rec 1>>>fNameEvent= %s\n",fNameEvent);
	          if(PVR_CHECK_FILE_EXIST(fNameEvent))  //idx exist
	          {
			 int i=0;
                      MT_CHAR  nodeFile[PVR_MAX_FILENAME_LEN];

                      (MT_VOID)remove(fNameEvent);    //delete ts file
                      PVR_EVENT_D("MT_PVR_Delete_Event_Rec 1>>>fNameEvent= %s\n",fNameEvent);
                      for(i=1; i<PVR_REC_MAX_NODE;i++)
                      {
                           memset(nodeFile, 0 , PVR_MAX_FILENAME_LEN);
                           snprintf(nodeFile, PVR_MAX_FILENAME_LEN,"%s.%04d", fNameEvent, i);
                           if(PVR_CHECK_FILE_EXIST(nodeFile))
   			      {
   			   	   (MT_VOID)remove(nodeFile);
   				   PVR_EVENT_D("remove <<< 1 nodeFile=%s\n",nodeFile);
                           }
   			      else
   			      {
   				   break;
   			      }
                     }
                     MT_INFO_PVR("remove <<< 1 tmp_cell->cellname=%s\n",fNameEvent);
	          }
		   return MT_SUCCESS;
            }
            */
            PVR_EVENT_D("%s :: %d >>>>event name =  %s\n",__FUNCTION__,__LINE__,pEeventName);

            return MT_ERR_PVR_FILE_INVALID_FNAME;
        }
    }
    PVR_EVENT_D("%s :: %d >>>>event name =  %s\n",__FUNCTION__,__LINE__,pEeventName);
#endif
    return MT_SUCCESS;
}

/*
func:delet a event rec, which be linked to u32ChnID
*/
MT_S32 MT_PVR_StopCache_Event_Rec(MT_U32 u32ChnID)
{
#ifdef SUPPORT_TIMESHIFT_EVENT
    PVR_REC_CHN_S *pRecChn = NULL;
    CHECK_REC_CHNID(u32ChnID);
    PVR_EVENT_D("%s :: %d >>>> u32ChnID =  0x%x\n",__FUNCTION__,__LINE__,u32ChnID);
    pRecChn = PVR_GET_RECPTR_BY_CHNID(u32ChnID);
    if(NULL == pRecChn)
    {
        return MT_ERR_PVR_INVALID_PARA;
    }
    if(pRecChn->timeshiftEventHandle.event_rec == NULL)
    {
        return MT_ERR_PVR_TIMESHIFT_EVENT_NOT_SURPPORT;
    }
    PVR_LOCK_REC(pRecChn);
    CHECK_REC_CHN_INIT_UNLOCK(pRecChn);

    if (!((MT_UNF_PVR_REC_STATE_RUNNING == pRecChn->enState)||
          (MT_UNF_PVR_REC_STATE_PAUSE == pRecChn->enState)))
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_ALREADY;
    }

    if (MT_UNF_PVR_STREAM_TYPE_TS != pRecChn->stUserCfg.enStreamType)
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_ALREADY;
    }

    if(NULL==pRecChn->IndexHandle)
    {
        PVR_UNLOCK_REC(pRecChn);
        return MT_ERR_PVR_NUL_PTR;
    }
    pRecChn->timeshiftEventHandle.state = MT_UNF_PVR_REC_STATE_STOP;
    /*
    ret = PVR_Rec_FreeAdvCaEncryptMem(pRecChn, u32ChnID);
    if(ret != MT_SUCCESS)
    {
           MT_ERR_PVR("MT_PVR_StopCache_Event_Rec >>> PVR_Rec_FreeAdvCaEncryptMem error !!\n");
    }
    pRecChn->enState = MT_UNF_PVR_REC_STATE_STOP;
    ret = PVRRecStopDemux(pRecChn);
    if(ret != MT_SUCCESS)
    {
           MT_ERR_PVR("MT_PVR_StopCache_Event_Rec >>> PVRRecStopDemux error !!\n");
    }
    */
    PVR_UNLOCK_REC(pRecChn);
#endif
    return   MT_SUCCESS;
}

MT_S32 MT_PVR_Get_Last_index(MT_UNF_DMX_REC_INDEX_S *last_index)
{
    MT_S32 ret = ioctl(g_s32PvrFd, CMD_PVR_REC_READ_LAST_INDEX, (ulong)(last_index));
    MT_USLEEP(11000);//release cpu!
    return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

