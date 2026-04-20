
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "../common/mt_adp_mpi.h"
#include "../common/mt_adp_demux.h"


#define DEMUX_ID 0

#define PAT_PID 0
#define PAT_TABLEID 0

#define INVALID_PID 0x1FFF
//#define USE_ORIGINAL_SCD

static PMT_COMPACT_TBL *ProgTbl_Pt = MT_NULL;

typedef struct
{
    mt_handle RecHandle;
    mt_char FileName[256];
    MT_BOOL ThreadRunFlag;
} TsFileInfo;

#ifdef USE_ORIGINAL_SCD
typedef struct mtDMX_DATA_S
{
    mt_u8 *pDataAddr;
    mt_u32 u32PhyAddr;
    mt_u32 u32Len;
} DMX_DATA_S;
extern mt_s32 MT_MPI_DMX_AcquireRecScdBuf(mt_u32 u32DmxId, DMX_DATA_S *pstBuf, mt_u32 u32TimeoutMs);
extern mt_s32 MT_MPI_DMX_ReleaseRecScdBuf(mt_u32 u32DmxId, const DMX_DATA_S *pstBuf);
#endif

static mt_void *SaveRecDataThread(mt_void *arg)
{
    mt_s32 ret;
    TsFileInfo *Ts = (TsFileInfo *)arg;
    FILE *RecFile = MT_NULL;
    mt_u32 len = 0;
    printf("[%s] open file %s\n", __FUNCTION__, Ts->FileName);
    RecFile = fopen(Ts->FileName, "wb");
    if (!RecFile) {
        printf("fopen error");
        Ts->ThreadRunFlag = MT_FALSE;
        return MT_NULL;
    }

    printf("[%s] oTs->ThreadRunFlag %d\n", __FUNCTION__, Ts->ThreadRunFlag);
    while (Ts->ThreadRunFlag) {
#ifndef USE_ORIGINAL_SCD
        MT_UNF_DMX_REC_DATA_S RecData;
        ret = MT_UNF_DMX_AcquireRecData(Ts->RecHandle, &RecData, 100);
#else
        DMX_DATA_S RecData;
        ret = MT_MPI_DMX_AcquireRecScdBuf(DEMUX_ID, &RecData, 1000);
#endif
        if (MT_SUCCESS != ret) {
            if (MT_ERR_DMX_TIMEOUT == ret) {
                continue;
            }
            if (MT_ERR_DMX_NOAVAILABLE_DATA == ret) {
                continue;
            }
            printf("[%s] MT_UNF_DMX_AcquireRecData failed 0x%x\n", __FUNCTION__, ret);
            break;
        }
        len = RecData.u32Len;
        ret = (mt_s32)fwrite(RecData.pDataAddr, 1, RecData.u32Len, RecFile);
        if (ret != (mt_s32)len) {
            printf("ret=%x\n", ret);
            printf("[SaveRecDataThread] fwrite error");
            break;
        }
#ifndef USE_ORIGINAL_SCD
        ret = MT_UNF_DMX_ReleaseRecData(Ts->RecHandle, &RecData);
#else
        ret = MT_MPI_DMX_ReleaseRecScdBuf(DEMUX_ID, &RecData);
#endif
        if (MT_SUCCESS != ret) {
            printf("[%s] MT_UNF_DMX_ReleaseRecData failed 0x%x\n", __FUNCTION__, ret);
            break;
        }
    }
    fclose(RecFile);
    Ts->ThreadRunFlag = MT_FALSE;
    return MT_NULL;
}

static mt_void *SaveIndexDataThread(mt_void *arg)
{
    mt_s32 ret;
    TsFileInfo *Ts = (TsFileInfo *)arg;
    FILE *IndexFile = MT_NULL;
    mt_u32 IndexCount = 0;
    MT_UNF_DMX_REC_INDEX_S RecIndex[64];

    IndexFile = fopen(Ts->FileName, "w");
    if (!IndexFile) {
        printf("fopen error");
        Ts->ThreadRunFlag = MT_FALSE;
        return MT_NULL;
    }

    printf("[%s] open file %s\n", __FUNCTION__, Ts->FileName);
    while (Ts->ThreadRunFlag) {
        ret = MT_UNF_DMX_AcquireRecIndex(Ts->RecHandle, &RecIndex[IndexCount], 100);
        if (MT_SUCCESS != ret) {
            if ((MT_ERR_DMX_NOAVAILABLE_DATA == ret) || (MT_ERR_DMX_TIMEOUT == ret)) {
                continue;
            }
            printf("[%s] MT_UNF_DMX_AcquireRecIndex failed 0x%x\n", __FUNCTION__, ret);
            break;
        }

        if (++IndexCount >= (sizeof(RecIndex) / sizeof(RecIndex[0]))) {
            IndexCount = 0;
            if (sizeof(RecIndex) != fwrite(&RecIndex, 1, sizeof(RecIndex), IndexFile)) {
                printf("[SaveIndexDataThread] fwrite error");
                break;
            }
            fflush(IndexFile);
        }
    }
    fclose(IndexFile);
    Ts->ThreadRunFlag = MT_FALSE;
    return MT_NULL;
}

static mt_s32 DmxStartRecord(mt_char *Path, PMT_COMPACT_PROG *ProgInfo,mt_u32 flag_recpid)
{
    mt_s32 ret;
    MT_UNF_DMX_REC_ATTR_S RecAttr;
    mt_handle RecHandle;
    mt_handle ChanHandle[8];
    mt_u32 ChanCount = 0;
    MT_BOOL RecordStatus = MT_FALSE;
    pthread_t RecThreadId=(pthread_t)0xffffffff;
    pthread_t IndexThreadId=(pthread_t)0xffffffff;
    TsFileInfo TsRecInfo;
    TsFileInfo TsIndexInfo;
    mt_char FileName[256]={0},TmpName[256]={0};
    mt_u32 i;
    mt_char InputCmd[256] = { 0 };

    memset(&RecAttr, 0 , sizeof(MT_UNF_DMX_REC_ATTR_S));
    RecAttr.u32DmxId = DEMUX_ID;
    RecAttr.u32RecBufSize = 4 * 1024 * 1024;
    if(1==flag_recpid){
        RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_SELECT_PID;
        RecAttr.bDescramed = MT_TRUE;
        if (ProgInfo->VElementPid < INVALID_PID) {
        	RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_VIDEO;
        	RecAttr.u32IndexSrcPid = ProgInfo->VElementPid;
        	RecAttr.enVCodecType = MT_UNF_VCODEC_TYPE_MPEG2;
        	RecAttr.type_mode = DMX_PARTIAL_TS_PACKET;
        } else if (ProgInfo->AElementPid < INVALID_PID) {
        	RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_AUDIO;
        	RecAttr.u32IndexSrcPid = ProgInfo->AElementPid;
        	RecAttr.type_mode = DMX_PARTIAL_TS_PACKET;
        } else {
        	RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
        }
    }else{
        RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_ALL_PID;
        RecAttr.bDescramed = MT_TRUE;
        RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
        RecAttr.type_mode = DMX_FULL_TS_WITHOUT_NULL_PACKET;
    }

    ret = MT_UNF_DMX_CreateRecChn(&RecAttr, &RecHandle);
    if (MT_SUCCESS != ret) {
    	printf("[%s - %u] MT_UNF_DMX_CreateRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    	return ret;
    }

    if(1==flag_recpid){
        ret = MT_UNF_DMX_AddRecPid(RecHandle, PAT_PID, &ChanHandle[ChanCount]);
        if (MT_SUCCESS != ret){
    		printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
            goto exit;
        }
        ++ChanCount;

        if(ProgInfo->PmtPid < INVALID_PID){
            ret = MT_UNF_DMX_AddRecPid(RecHandle, ProgInfo->PmtPid, &ChanHandle[ChanCount]);
            if (MT_SUCCESS != ret){
        		printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
                goto exit;
            }
            ++ChanCount;
        }

        sprintf(FileName, "%s/rec", Path);
        if (ProgInfo->VElementPid < INVALID_PID) {
        	ret = MT_UNF_DMX_AddRecPid(RecHandle, ProgInfo->VElementPid, &ChanHandle[ChanCount]);
        	if (MT_SUCCESS != ret) {
        	    printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
        	    goto exit;
        	}
            sprintf(TmpName,"_v%d",ProgInfo->VElementPid);
        	strcat(FileName, TmpName);
        	++ChanCount;
        }

        if (ProgInfo->AElementPid < INVALID_PID){
            ret = MT_UNF_DMX_AddRecPid(RecHandle, ProgInfo->AElementPid, &ChanHandle[ChanCount]);
            if (MT_SUCCESS != ret){
        		printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
                goto exit;
            }
            sprintf(TmpName,"_a%d",ProgInfo->AElementPid);
        	strcat(FileName, TmpName);
            ++ChanCount;
        }

        if (ProgInfo->PcrPid < INVALID_PID){
            ret = MT_UNF_DMX_AddRecPid(RecHandle, ProgInfo->PcrPid, &ChanHandle[ChanCount]);
            if (MT_SUCCESS != ret){
        		printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
                goto exit;
            }
            ++ChanCount;
        }
    }else{
        strcat(FileName, "rec_allpid");
    }
    ret = MT_UNF_DMX_StartRecChn(RecHandle);
    if (MT_SUCCESS != ret) {
    	printf("[%s - %u] MT_UNF_DMX_StartRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    	goto exit;
    }

    RecordStatus = MT_TRUE;

    TsRecInfo.RecHandle = RecHandle;
    TsRecInfo.ThreadRunFlag = MT_TRUE;
    sprintf(TsRecInfo.FileName, "%s.ts", FileName);
    printf("recordtspath=%s\n",TsRecInfo.FileName);
    ret = pthread_create(&RecThreadId, MT_NULL, SaveRecDataThread, (mt_void *)&TsRecInfo);
    if (0 != ret) {
    	printf("[DmxStartRecord] pthread_create record error");
    	goto exit;
    }
    
    if(1==flag_recpid){
        TsIndexInfo.RecHandle = RecHandle;
        TsIndexInfo.ThreadRunFlag = MT_TRUE;
        sprintf(TsIndexInfo.FileName, "%s.ts.idx", FileName);
        printf("recordidpath=%s\n",TsIndexInfo.FileName);
        ret = pthread_create(&IndexThreadId, MT_NULL, SaveIndexDataThread, (mt_void*)&TsIndexInfo);
        if (0 != ret){
            printf("[DmxStartRecord] pthread_create index error");
            goto exit;
        }
    }
    
    while (1) {
    	printf("please input the q to quit!\n");
    	SAMPLE_GET_INPUTCMD(InputCmd);
    	if ('q' == InputCmd[0]) {
    	    break;
    	}
    }

exit:
    if ((pthread_t)0xffffffff != RecThreadId) {
    	TsRecInfo.ThreadRunFlag = MT_FALSE;
    	pthread_join(RecThreadId, MT_NULL);
    }

    if ((pthread_t)0xffffffff != IndexThreadId) {
    	TsIndexInfo.ThreadRunFlag = MT_FALSE;
    	pthread_join(IndexThreadId, MT_NULL);
    }
    printf("======================== stop rec =================================1\n");
    if (RecordStatus) {
	ret = MT_UNF_DMX_StopRecChn(RecHandle);
	if (MT_SUCCESS != ret) {
	    printf("[%s - %u] MT_UNF_DMX_StopRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
	}
    }
    if(1==flag_recpid){
        printf("======================== stop rec =================================ChanCount=%d\n", ChanCount);
        for (i = 0; i < ChanCount; i++) {
        	ret = MT_UNF_DMX_DelRecPid(RecHandle, ChanHandle[i]);
        	if (MT_SUCCESS != ret) {
        	    printf("[%s - %u] MT_UNF_DMX_DelRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
        	}
        }
    }
    ret = MT_UNF_DMX_DestroyRecChn(RecHandle);
    if (MT_SUCCESS != ret) {
    	printf("[%s - %u] MT_UNF_DMX_DestroyRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    }

    //goto StrRecord;
    return ret;
}

mt_s32 main(mt_s32 argc, mt_char *argv[])
{//./sample_demux_record 477 6875 64 /media/casetest
    mt_s32 ret;
    mt_char *FilePath = MT_NULL;
    mt_u32 i,ProgNum = 0;
    mt_char InputCmd[256] = { 0 };
    mt_u32 freq, symbol_rate, qam_mode;
    mt_sys_version_s stSysChipInfo;
    char *fgetret=NULL;

    mt_sys_init();
    MTADP_MCE_Exit();
    ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != ret){
        printf("call MT_UNF_DMX_Init failed.\n");
        return -1;
    }

    freq = (mt_u32)strtol(argv[1], NULL, 0);
    symbol_rate = (mt_u32)strtol(argv[2], NULL, 0);
    qam_mode = (mt_u32)strtol(argv[3], NULL, 0);

    mtadp_fe_init();
    if ((16<=qam_mode) && (256>=qam_mode)){     //dvbc
        mtadp_fe_connect(0, freq, symbol_rate, qam_mode);
    }else if(0<qam_mode){                       //dvbt.t2
        mtadp_fe_connect_dvbtauto(0,freq,qam_mode);
    }else{
        mtadp_fe_connect_dvbs(0,freq, symbol_rate,0, 0, 0);
    }

    FilePath = argv[4];

    ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != ret) {
    	printf("MT_UNF_DMX_Init failed 0x%x\n", ret);
    	goto TUNER_DEINIT;
    }

#ifdef CONFIG_MT_CHIP_ARIA
    ret |= MT_UNF_DMX_AttachTSPort(DEMUX_ID, MT_UNF_DMX_PORT_TSI_2);
#else
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);
    if (0<qam_mode){ //dvbc
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.C
            ret |= MT_UNF_DMX_AttachTSPort(DEMUX_ID, MT_UNF_DMX_PORT_TSI_1);
        }else{                                                      //sym1.C
            ret |= MT_UNF_DMX_AttachTSPort(DEMUX_ID, MT_UNF_DMX_PORT_TSI_0);
        }
    }else{
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.S
            ret |= MT_UNF_DMX_AttachTSPort(DEMUX_ID, MT_UNF_DMX_PORT_TSI_0);
        }else{                                                      //sym1.S
            ret |= MT_UNF_DMX_AttachTSPort(DEMUX_ID, MT_UNF_DMX_PORT_TSI_1);
        }
    }
#endif  
    if (MT_SUCCESS != ret){
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        return -1;
    }

    MTADP_Search_Init();
    ret = MTADP_Search_GetAllPmt(DEMUX_ID, &ProgTbl_Pt);
    if (MT_SUCCESS != ret){
        printf("call MTADP_Search_GetAllPmt failed\n");
        goto PSISI_FREE;
    }else{
        printf("All prog_num = %x \n",  ProgTbl_Pt->prog_num);
        for(i=0; i<ProgTbl_Pt->prog_num; i++){
            printf("VElementNum = %x,  AElementNum=%x\n", ProgTbl_Pt->proginfo[i].VElementNum, ProgTbl_Pt->proginfo[i].AElementNum);
            printf("vpid = %x , videoType = %x \n",
            ProgTbl_Pt->proginfo[i].VElementPid, ProgTbl_Pt->proginfo[i].VideoType);
            printf("apid = %x , audioType = %x \n",
            ProgTbl_Pt->proginfo[i].AElementPid, ProgTbl_Pt->proginfo[i].AudioType);
        }
    }
    if(0==ProgTbl_Pt->prog_num){
        printf("there is no video or audio\n");
        goto PSISI_FREE;
    }
    printf("\nPlease input the number of program to record[1-num],0 for All:");
    fgetret=fgets((char *)(InputCmd), (sizeof(InputCmd) - 1), stdin);
    fgetret=fgetret;
    if(0==atoi(InputCmd)){                                              //record all ts
        DmxStartRecord(FilePath, ProgTbl_Pt->proginfo + ProgNum,0);
    }else{                                                              //record one programme
        ProgNum = ((mt_u32)atoi(InputCmd) - (mt_u32)1) % (mt_u32)ProgTbl_Pt->prog_num;
        DmxStartRecord(FilePath, ProgTbl_Pt->proginfo + ProgNum,1);
    }

PSISI_FREE:
    MTADP_Search_FreeAllPmt(ProgTbl_Pt);
    MTADP_Search_DeInit();
    MT_UNF_DMX_DetachTSPort(DEMUX_ID);
    MT_UNF_DMX_DeInit();
TUNER_DEINIT:
    mtadp_fe_deinit();
    mt_sys_deinit();
    return ret;
}

