/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_module_debug.h"
#include "mt_module.h"
#include "mt_mpi_mem.h"
#include "mt_drv_struct.h"
#include "mt_error_mpi.h"

#include "mt_mpi_ai.h"
#include "drv_ai_ioctl.h"

#include "mt_drv_ao.h"
#include "mt_mpi_ao.h"
#include "mt_drv_aenc.h"
#include "mt_mpi_aenc.h"

#define AI_MAX_DST (4)
#define AI_SLEEP_TIME_MS (5)
#define AI_CHECK_DELAY_SLEEP_TIME_MS (1)
#define AO_TRACK_DELAY_FOR_AI_TIME 20
#define U32_MAX_VALUE 0xffffffff

typedef struct hiAI_MPISTATE_S
{
    mt_handle hAi;
    MT_BOOL bDataFlag; //is data valid or not

    mt_handle hSlaTrack; //ai->ao by aip buff
    MT_BOOL bNeedStart;  //start track

    mt_u32 u32DstNum;
    mt_handle hDst[AI_MAX_DST];

    MT_BOOL AiThreadRun;
    pthread_t AiDataThdInst;         /* run handle of ai thread */
    pthread_t AiTrackThdInst;        /* run handle of start track */
    pthread_mutex_t *pAiThreadMutex; /*mutex for data safety use*/
} AI_MPISTATE_S;

typedef struct hiAI_MPIRESOURCE_S
{
    AI_MPISTATE_S *pstAI_S[AI_MAX_TOTAL_NUM];
} AI_MPIRESOURCE_S;

static mt_s32 g_s32AIFd = -1;
static const mt_char g_acAIDevName[] = "/dev/" UMAP_DEVNAME_AI;
static AI_MPIRESOURCE_S g_AiRes;

void AI_ThreadMutex_Lock(pthread_mutex_t *ss)
{
    pthread_mutex_lock(ss);
}

void Ai_ThreadMutex_UnLock(pthread_mutex_t *ss)
{
    pthread_mutex_unlock(ss);
}

static mt_s32 AI_ChnAcquireFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_s32 s32Ret;
    AI_Frame_Param_S stAiGetFrame;
    AI_Buf_Param_S stAiBufInfo;

    stAiBufInfo.hAi = hAI;
    stAiGetFrame.hAi = hAI;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GETBUFINFO, &stAiBufInfo);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("\n GET AI BUF_INFO s32Ret=0x%x Failed \n", s32Ret);
	return s32Ret;
    }

    s32Ret = ioctl(g_s32AIFd, CMD_AI_ACQUIREFRAME, &stAiGetFrame);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("AI GetFrame Failed 0x%x \n", s32Ret);
	return s32Ret;
    }

    memcpy(pstFrame, &stAiGetFrame.stAiFrame, sizeof(MT_UNF_AO_FRAMEINFO_S));

    pstFrame->ps32PcmBuffer = (mt_s32 *)(stAiBufInfo.stAiBuf.u32UserVirBaseAddr);

    //printf("chn acquire frame ok addr 0x%x \n", pstFrame->ps32PcmBuffer);

    return MT_SUCCESS;
}

static mt_s32 AI_ChnReleaseFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_s32 s32Ret;
    AI_Frame_Param_S stAiRleFrame;

    stAiRleFrame.hAi = hAI;

    memcpy(&stAiRleFrame.stAiFrame, pstFrame, sizeof(MT_UNF_AO_FRAMEINFO_S));

    s32Ret = ioctl(g_s32AIFd, CMD_AI_RELEASEFRAME, &stAiRleFrame);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("AI ReleaseFrame Failed 0x%x \n", s32Ret);
    }

    return s32Ret;
}

mt_void *AI_TrackThread(mt_void *Arg)
{
    mt_s32 s32Ret;
    AI_MPISTATE_S *pstAiState = (AI_MPISTATE_S *)Arg;
    MT_BOOL bEnable;
    mt_u32 DelayMs = 0;
    mt_s32 s32Flag = 0;
    mt_u32 u32StartSysTime = 0;
    mt_u32 u32EndSysTime = 0;
    mt_u32 u32DelayTime = 0;
    MT_UNF_AI_DELAY_S stDelayCompensation;
    memset(&stDelayCompensation, 0, sizeof(MT_UNF_AI_DELAY_S));
    mt_set_pthread_name(__FUNCTION__);
    while ((pstAiState->AiThreadRun)) {
	AI_ThreadMutex_Lock(pstAiState->pAiThreadMutex);
	if (MT_INVALID_HANDLE != pstAiState->hSlaTrack) {
	    s32Ret = MT_MPI_AI_GetEnable(pstAiState->hAi, &bEnable);
	    MT_INFO_AI("calling MT_MPI_AI_GetEnable \n");
	    if ((MT_SUCCESS == s32Ret) && (MT_TRUE == bEnable)) {
		if (MT_TRUE == pstAiState->bNeedStart) // SetEnable/Attach called
		{
		    // cooperdel
		    //if(0 == s32Flag)
		    //MT_SYS_GetTimeStampMs(&u32StartSysTime);

		    s32Flag = 1;

		    // cooperdel
		    //MT_SYS_GetTimeStampMs(&u32EndSysTime);
		    if (u32EndSysTime >= u32StartSysTime) {
			u32DelayTime = u32EndSysTime - u32StartSysTime;
		    } else {
			u32DelayTime = (U32_MAX_VALUE - u32StartSysTime) + (u32EndSysTime + 1);
		    }
		    s32Ret = MT_MPI_AO_Track_GetDelayMs(pstAiState->hSlaTrack, &DelayMs);
		    s32Ret |= MT_MPI_AI_GetDelay(pstAiState->hAi, &stDelayCompensation);

		    MT_INFO_AI("calling MT_MPI_AI_GetDelay \n");

		    if (((MT_SUCCESS == s32Ret) && (DelayMs + stDelayCompensation.u32DelayMs >= AO_TRACK_DELAY_FOR_AI_TIME)) || ((DelayMs + stDelayCompensation.u32DelayMs + u32DelayTime) >= AO_TRACK_DELAY_FOR_AI_TIME)) {

			s32Ret = MT_MPI_AO_Track_Start(pstAiState->hSlaTrack);
			if (s32Ret != MT_SUCCESS) {
			    MT_ERR_AO("call MT_MPI_AO_Track_Start failed.\n");
			}
			MT_INFO_AO("MT_MPI_AO_Track_Start make bNeedStart MT_FALSE\n");
			pstAiState->bNeedStart = MT_FALSE;
		    }
		} else
		    s32Flag = 0;
	    } else
		s32Flag = 0;
	} else
	    s32Flag = 0;

	Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
	MT_USLEEP(AI_CHECK_DELAY_SLEEP_TIME_MS * 1000);
    }

    return MT_NULL;
}

mt_void *AI_DataThread(mt_void *Arg)
{
    mt_s32 s32Ret;
    mt_u32 i;
    AI_MPISTATE_S *pstAiState = (AI_MPISTATE_S *)Arg;
    MT_UNF_AO_FRAMEINFO_S stAiFrame;
    MT_UNF_AO_FRAMEINFO_S *pstAiFrame = &stAiFrame;
   mt_set_pthread_name(__FUNCTION__);
    while ((pstAiState->AiThreadRun)) {
	AI_ThreadMutex_Lock(pstAiState->pAiThreadMutex);
	if ((MT_FALSE == pstAiState->bDataFlag) && (pstAiState->u32DstNum)) {
	    s32Ret = AI_ChnAcquireFrame(pstAiState->hAi, pstAiFrame);
	    MT_INFO_AI("calling AI_ChnAcquireFrame\n");
	    if (MT_SUCCESS == s32Ret) {
		pstAiState->bDataFlag = MT_TRUE;
	    } else {
		MT_WARN_AI("call MT_MPI_AI_AcquireFrame failed!\n");
		Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
		MT_USLEEP(AI_SLEEP_TIME_MS * 1000);
		continue;
	    }
	}

	if ((pstAiState->u32DstNum) && (MT_TRUE == pstAiState->bDataFlag)) {
	    if (MT_ID_AO == (pstAiState->hDst[0]) >> 16) {
		s32Ret = MT_MPI_AO_Track_SendData(pstAiState->hDst[0], pstAiFrame);
		MT_INFO_AI("calling MT_MPI_AO_Track_SendData\n");

	    } else { // cooperdel
		//s32Ret = MT_MPI_AENC_SendBuffer(pstAiState->hDst[0], pstAiFrame);
		MT_INFO_AI("calling MT_MPI_AENC_SendBuffer\n");
	    }

	    if (MT_SUCCESS == s32Ret) {
		pstAiState->bDataFlag = MT_FALSE;
		if (1 < pstAiState->u32DstNum) {
		    for (i = 1; i < pstAiState->u32DstNum; i++) {
			if (MT_ID_AO == (pstAiState->hDst[i]) >> 16) {
			    MT_MPI_AO_Track_SendData(pstAiState->hDst[i], pstAiFrame);
			    MT_INFO_AI("calling MT_MPI_AO_Track_SendData\n");

			} else {
			    // cooperdel
			    //(mt_void)MT_MPI_AENC_SendBuffer(pstAiState->hDst[i], pstAiFrame);
			    MT_INFO_AI("calling MT_MPI_AENC_SendBuffer\n");
			}
		    }
		}

		s32Ret = AI_ChnReleaseFrame(pstAiState->hAi, pstAiFrame);
		MT_INFO_AI("calling AI_ChnReleaseFrame \n");

	    } else {
		pstAiState->bDataFlag = MT_TRUE;
	    }
	}
	Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
	MT_USLEEP(AI_SLEEP_TIME_MS * 1000);
    }

    return MT_NULL;
}

mt_s32 AI_Attach(mt_handle hAI, mt_handle hDst)
{
    mt_s32 s32Ret;
    mt_u32 i;
    AI_MPISTATE_S *pstAiState;

    CHECK_AI_ID(hAI);
    if (MT_INVALID_HANDLE == hDst) {
	MT_ERR_AI("para hDst is invalid.\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    if ((MT_ID_AO != (hDst >> 16)) && (MT_ID_AENC != (hDst >> 16))) {
	MT_ERR_AI("para hDst is invalid, just support attach sound_track or aenc.\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if ((MT_NULL != g_AiRes.pstAI_S[i]) && (hAI == g_AiRes.pstAI_S[i]->hAi))
	    break;
    }

    if (AI_MAX_TOTAL_NUM == i) {
	MT_ERR_AI("AI chn not open\n");
	return MT_FAILURE;
    }
    pstAiState = g_AiRes.pstAI_S[i];

    if ((MT_ID_AO == hDst >> 16) && (AO_MAX_REAL_TRACK_NUM > (hDst & 0xff))) {
	if (MT_INVALID_HANDLE != pstAiState->hSlaTrack) {
	    if (hDst == pstAiState->hSlaTrack) {
		return MT_SUCCESS;
	    } else {
		MT_ERR_AI("Ai can not attach more than one slave track!\n");
		return MT_FAILURE;
	    }
	}
	s32Ret = MT_MPI_AO_Track_AttachAi(hAI, hDst);
	if (MT_SUCCESS == s32Ret) {
	    pstAiState->hSlaTrack = hDst;
	    MT_INFO_AI("pstAiState->bNeedStart=%d .\n", pstAiState->bNeedStart);
	    pstAiState->bNeedStart = MT_TRUE;
	}

	return s32Ret;
    }

    AI_ThreadMutex_Lock(pstAiState->pAiThreadMutex);

    for (i = 0; i < AI_MAX_DST; i++) {
	if (pstAiState->hDst[i] == hDst) {
	    Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);

	    return MT_SUCCESS;
	}
    }

    for (i = 0; i < AI_MAX_DST; i++) {
	if (MT_INVALID_HANDLE == pstAiState->hDst[i]) {
	    break;
	}
    }

    if (AI_MAX_DST == i) {
	MT_ERR_AI("AI has attached max dst.\n");
	Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
	return MT_FAILURE;
    }

    pstAiState->hDst[i] = hDst;
    pstAiState->u32DstNum++;

    Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
    return MT_SUCCESS;
}

mt_s32 AI_Detach(mt_handle hAI, mt_handle hDst)
{
    mt_u32 i;
    mt_s32 s32Ret;
    AI_MPISTATE_S *pstAiState;

    CHECK_AI_ID(hAI);

    if (MT_INVALID_HANDLE == hDst) {
	MT_ERR_AI("para hDst is invalid.\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    if ((MT_ID_AO != (hDst >> 16)) && (MT_ID_AENC != (hDst >> 16))) {
	MT_ERR_AI("para hDst is invalid, just support attach sound_track or aenc.\n");
	return MT_ERR_AI_INVALID_PARA;
    }

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if ((MT_NULL != g_AiRes.pstAI_S[i]) && (hAI == g_AiRes.pstAI_S[i]->hAi))
	    break;
    }

    if (AI_MAX_TOTAL_NUM == i) {
	MT_ERR_AI("AI chn not open\n");
	return MT_FAILURE;
    }

    pstAiState = g_AiRes.pstAI_S[i];

    if ((MT_ID_AO == hDst >> 16) && (AO_MAX_REAL_TRACK_NUM > (hDst & 0xff))) {
	if (hDst == pstAiState->hSlaTrack) {
	    s32Ret = MT_MPI_AO_Track_DetachAi(hAI, hDst);
	    if (MT_SUCCESS == s32Ret) {
		pstAiState->hSlaTrack = MT_INVALID_HANDLE;
	    }
	    return s32Ret;
	} else {
	    MT_ERR_AI("This track is not attach Ai,can not detach!\n");
	    return MT_FAILURE;
	}
    }

    AI_ThreadMutex_Lock(pstAiState->pAiThreadMutex);

    for (i = 0; i < pstAiState->u32DstNum; i++) {
	if (pstAiState->hDst[i] == hDst) {
	    break;
	}
    }

    if (i == pstAiState->u32DstNum) {
	MT_ERR_AI("this track is not attached, can not detach.\n");
	Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
	return MT_FAILURE;
    }

    pstAiState->hDst[i] = pstAiState->hDst[pstAiState->u32DstNum - 1];
    pstAiState->hDst[pstAiState->u32DstNum - 1] = MT_INVALID_HANDLE;
    pstAiState->u32DstNum--;

    Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AI_Init(mt_void)
{
    mt_u32 i;
    MT_INFO_AI("\n mpi layer MT_MPI_AI_Init \n");
    if (g_s32AIFd < 0) {
	g_s32AIFd = open(g_acAIDevName, O_RDWR, 0);
	if (g_s32AIFd < 0) {
	    MT_FATAL_AI("OpenAIDevice err\n");
	    g_s32AIFd = -1;
	    return MT_ERR_AI_NOT_INIT;
	}
	for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	    g_AiRes.pstAI_S[i] = NULL;
	}
    }
    return MT_SUCCESS;
}

mt_s32 MT_MPI_AI_DeInit(mt_void)
{
    if (g_s32AIFd > 0) {
	close(g_s32AIFd);
	g_s32AIFd = -1;
    }

    return MT_SUCCESS;
}

mt_s32 MT_MPI_AI_GetDefaultAttr(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    AI_GetDfAttr_Param_S stAiGetDfAttr;

    CHECK_AI_NULL_PTR(pstAttr);
    stAiGetDfAttr.enAiPort = enAiPort;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GEtDEFAULTATTR, &stAiGetDfAttr);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("AI GetDfAttr Failed 0x%x \n", s32Ret);
    }

    memcpy(pstAttr, &stAiGetDfAttr.stAttr, sizeof(MT_UNF_AI_ATTR_S));

    return s32Ret;
}

mt_s32 MT_MPI_AI_SetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr)
{
    mt_s32 s32Ret;
    AI_Attr_Param_S stAiSetAttr;
    AI_MPISTATE_S *pstAiState;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pstAttr);

    stAiSetAttr.hAi = hAI;

    memcpy(&stAiSetAttr.stAttr, pstAttr, sizeof(MT_UNF_AI_ATTR_S));

    s32Ret = ioctl(g_s32AIFd, CMD_AI_SETATTR, &stAiSetAttr);

    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("AI SetAttr Failed 0x%x \n", s32Ret);
	return s32Ret;
    }

    pstAiState = g_AiRes.pstAI_S[hAI & AI_CHNID_MASK];
    if (MT_NULL != pstAiState && MT_INVALID_HANDLE != pstAiState->hSlaTrack) {
	mt_handle hTrack = pstAiState->hSlaTrack;
	//detach track for new ai attr
	MT_INFO_AI("AI_Detach Track\n");
	s32Ret = AI_Detach(hAI, pstAiState->hSlaTrack);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_AI("AI_Detach Failed 0x%x \n", s32Ret);
	}

	MT_INFO_AI("MT_MPI_AO_Track_Stop called\n");
	s32Ret = MT_MPI_AO_Track_Stop(hTrack);
	if (s32Ret != MT_SUCCESS) {
	    MT_ERR_AO("call MT_MPI_AO_Track_Stop failed.\n");
	}

	MT_INFO_AI("AI_Detach Track\n");
	s32Ret = AI_Attach(hAI, hTrack);
	if (MT_SUCCESS != s32Ret) {
	    MT_ERR_AI("AI_Attach Failed 0x%x \n", s32Ret);
	}
    }

    return s32Ret;
}

mt_s32 MT_MPI_AI_GetAttr(mt_handle hAI, MT_UNF_AI_ATTR_S *pstAttr)
{

    mt_s32 s32Ret;
    AI_Attr_Param_S stAiGetAttr;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pstAttr);

    stAiGetAttr.hAi = hAI;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GETATTR, &stAiGetAttr);

    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("AI GetAttr Failed 0x%x \n", s32Ret);
    }

    memcpy(pstAttr, &stAiGetAttr.stAttr, sizeof(MT_UNF_AI_ATTR_S));

    return s32Ret;
}

mt_s32 MT_MPI_AI_Create(MT_UNF_AI_E enAiPort, MT_UNF_AI_ATTR_S *pstAttr, mt_handle *phandle)
{
    mt_s32 s32Ret;
    mt_u32 i;
    AI_MPISTATE_S *pstAiState;

    AI_Create_Param_S stAiParam;
    AI_Buf_Param_S stAiBufInfo;

    CHECK_AI_NULL_PTR(pstAttr);
    CHECK_AI_NULL_PTR(phandle);

    stAiParam.enAiPort = enAiPort;
    stAiParam.bAlsaUse = MT_FALSE;
    stAiParam.pAlsaPara = NULL;

    memcpy(&stAiParam.stAttr, pstAttr, sizeof(MT_UNF_AI_ATTR_S));

    s32Ret = ioctl(g_s32AIFd, CMD_AI_CREATE, &stAiParam);

    if (MT_SUCCESS == s32Ret) {
	*phandle = stAiParam.hAi;
    } else {
	MT_ERR_AI("CMD_AI_CREATE Failed 0x%x\n", s32Ret);
	return s32Ret;
    }

    stAiBufInfo.hAi = stAiParam.hAi;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GETBUFINFO, &stAiBufInfo);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("GET AI BUF_INFO s32Ret=0x%x Failed \n", s32Ret);
	goto ERR_CREAT;
    }

    stAiBufInfo.stAiBuf.u32UserVirBaseAddr = (ulong)mt_mem_map(stAiBufInfo.stAiBuf.u32PhyBaseAddr, stAiBufInfo.stAiBuf.u32Size);

    s32Ret = ioctl(g_s32AIFd, CMD_AI_SETBUFINFO, &stAiBufInfo);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("SET AI BUF_INFO Failed 0x%x\n", s32Ret);
	goto ERR_MMAP;
    }

    MT_ERR_AI("minnan debug ai buf virtual 0x%x\n", stAiBufInfo.stAiBuf.u32UserVirBaseAddr);

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if (NULL == g_AiRes.pstAI_S[i])
	    break;
    }

    if (AI_MAX_TOTAL_NUM == i) {
	MT_ERR_AI("too many Ai chn\n");
	goto ERR_MMAP;
    }

    g_AiRes.pstAI_S[i] = (AI_MPISTATE_S *)mt_malloc(MT_ID_AI, sizeof(AI_MPISTATE_S));
    if (MT_NULL == g_AiRes.pstAI_S[i]) {
	goto ERR_MMAP;
    }

    pstAiState = g_AiRes.pstAI_S[i];

    //  cooper
    pstAiState->pAiThreadMutex = (pthread_mutex_t *)mt_malloc(MT_ID_AI, sizeof(pthread_mutex_t));
    if (MT_NULL == pstAiState->pAiThreadMutex) {
	goto ERR_KFREE_STATE;
    }
    (mt_void) pthread_mutex_init(pstAiState->pAiThreadMutex, NULL);

    pstAiState->hAi = stAiParam.hAi;
    pstAiState->bDataFlag = MT_FALSE;
    pstAiState->u32DstNum = 0;
    pstAiState->AiThreadRun = MT_TRUE;
    pstAiState->hSlaTrack = MT_INVALID_HANDLE;

    for (i = 0; i < AI_MAX_DST; i++) {
	pstAiState->hDst[i] = MT_INVALID_HANDLE;
    }

    s32Ret = pthread_create(&pstAiState->AiDataThdInst, MT_NULL, AI_DataThread, (mt_void *)pstAiState);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("Ai pthread_create failed!\n");
	goto ERR_KFREE_MUTEX;
    }

    s32Ret = pthread_create(&pstAiState->AiTrackThdInst, MT_NULL, AI_TrackThread, (mt_void *)pstAiState);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("Ai pthread_create failed!\n");
	goto ERR_DESTROY_DATATHREAD;
    }

    return s32Ret;

ERR_DESTROY_DATATHREAD:
    pstAiState->AiThreadRun = MT_FALSE;
    //AI_ThreadMutex_Lock(pstAiState->pAiThreadMutex);
    (mt_void) pthread_join(pstAiState->AiDataThdInst, MT_NULL);
//Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);
ERR_KFREE_MUTEX:
    (mt_void) pthread_mutex_destroy(pstAiState->pAiThreadMutex);
    mt_free(MT_ID_AI, (mt_void *)(pstAiState->pAiThreadMutex));
ERR_KFREE_STATE:
    mt_free(MT_ID_AI, (mt_void *)(pstAiState));
ERR_MMAP:
    mt_mem_unmap((mt_void *)stAiBufInfo.stAiBuf.u32PhyBaseAddr);
ERR_CREAT:
    ioctl(g_s32AIFd, CMD_AI_DESTROY, &stAiParam);
    return s32Ret;
}

mt_s32 MT_MPI_AI_Destroy(mt_handle hAI)
{
    mt_s32 s32Ret;
    mt_u32 i;
    AI_Buf_Param_S stAiBufInfo;
    AI_MPISTATE_S *pstAiState;

    CHECK_AI_ID(hAI);

    stAiBufInfo.hAi = hAI;

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if ((MT_NULL != g_AiRes.pstAI_S[i]) && (hAI == g_AiRes.pstAI_S[i]->hAi))
	    break;
    }

    if (AI_MAX_TOTAL_NUM == i) {
	MT_ERR_AI("This AI chn is not open!\n");
	return MT_FAILURE;
    }

    pstAiState = g_AiRes.pstAI_S[i];

    //AI_ThreadMutex_Lock(pstAiState->pAiThreadMutex);

    pstAiState->AiThreadRun = MT_FALSE;
    (mt_void) pthread_join(pstAiState->AiTrackThdInst, MT_NULL);
    (mt_void) pthread_join(pstAiState->AiDataThdInst, MT_NULL);

    //Ai_ThreadMutex_UnLock(pstAiState->pAiThreadMutex);

    (mt_void) pthread_mutex_destroy(pstAiState->pAiThreadMutex);

    //mt_free(MT_ID_AI, (mt_void*)(pstAiState->pAiThreadMutex));
    //mt_free(MT_ID_AI, (mt_void*)(pstAiState));
    g_AiRes.pstAI_S[i] = MT_NULL;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GETBUFINFO, &stAiBufInfo);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("\n GET AI BUF_INFO s32Ret=0x%x Failed \n", s32Ret);
	return s32Ret;
    } else {
	//MT_MEM_Unmap((mt_void *)stAiBufInfo.stAiBuf.u32UserVirBaseAddr);
    }

    return ioctl(g_s32AIFd, CMD_AI_DESTROY, &hAI);
}

mt_s32 MT_MPI_AI_SetEnable(mt_handle hAI, MT_BOOL bEnable)
{
    mt_s32 s32Ret;
    AI_MPISTATE_S *pstAiState;
    AI_Enable_Param_S stAiEnable;

    CHECK_AI_ID(hAI);

    stAiEnable.hAi = hAI;
    stAiEnable.bAiEnable = bEnable;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_SETENABLE, &stAiEnable);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("ENABLE AI Failed 0x%x \n", s32Ret);
    } else {
	if (MT_TRUE == bEnable) {
	    MT_INFO_AI("pstAiState->bNeedStart = MT_TRUE\n");
	    pstAiState = g_AiRes.pstAI_S[hAI & AI_CHNID_MASK];
	    pstAiState->bNeedStart = MT_TRUE;
	}
    }
    return s32Ret;
}

mt_s32 MT_MPI_AI_GetEnable(mt_handle hAI, MT_BOOL *pbEnable)
{
    mt_s32 s32Ret;
    AI_Enable_Param_S stAiEnable;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pbEnable);

    stAiEnable.hAi = hAI;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GETENABLE, &stAiEnable);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("Get AI Enable Failed 0x%x \n", s32Ret);
    } else {
	*pbEnable = stAiEnable.bAiEnable;
    }

    return s32Ret;
}

mt_s32 MT_MPI_AI_AcquireFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_u32 i;
    AI_MPISTATE_S *pstAiState;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pstFrame);

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if ((MT_NULL != g_AiRes.pstAI_S[i]) && (hAI == g_AiRes.pstAI_S[i]->hAi))
	    break;
    }

    if (AI_MAX_TOTAL_NUM != i) {
	pstAiState = g_AiRes.pstAI_S[i];

	if ((MT_INVALID_HANDLE != pstAiState->hDst[0]) || (MT_INVALID_HANDLE != pstAiState->hSlaTrack)) {
	    MT_ERR_AI("Aenc or Track attach this Ai chn,can not acquire frame!\n");
	    return MT_FAILURE;
	}
    }

    return AI_ChnAcquireFrame(hAI, pstFrame);
}

mt_s32 MT_MPI_AI_ReleaseFrame(mt_handle hAI, MT_UNF_AO_FRAMEINFO_S *pstFrame)
{
    mt_u32 i;
    AI_MPISTATE_S *pstAiState;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pstFrame);

    for (i = 0; i < AI_MAX_TOTAL_NUM; i++) {
	if ((MT_NULL != g_AiRes.pstAI_S[i]) && (hAI == g_AiRes.pstAI_S[i]->hAi))
	    break;
    }

    if (AI_MAX_TOTAL_NUM != i) {
	pstAiState = g_AiRes.pstAI_S[i];

	if ((MT_INVALID_HANDLE != pstAiState->hDst[0]) || (MT_INVALID_HANDLE != pstAiState->hSlaTrack)) {
	    MT_WARN_AI("Aenc or Track attach this Ai chn,can not release frame!\n");
	    return MT_FAILURE;
	}
    }

    return AI_ChnReleaseFrame(hAI, pstFrame);
}

mt_s32 MT_MPI_AI_Attach(mt_handle hAI, mt_handle hDst)
{
    return AI_Attach(hAI, hDst);
}

mt_s32 MT_MPI_AI_Detach(mt_handle hAI, mt_handle hDst)
{
    return AI_Detach(hAI, hDst);
}

mt_s32 MT_MPI_AI_SetDelay(mt_handle hAI, const MT_UNF_AI_DELAY_S *pstDelay)
{
    mt_s32 s32Ret;
    AI_DelayComps_Param_S stDelayCompsParam;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pstDelay);

    stDelayCompsParam.hAi = hAI;
    memcpy(&stDelayCompsParam.stDelayComps, pstDelay, sizeof(MT_UNF_AI_DELAY_S));

    s32Ret = ioctl(g_s32AIFd, CMD_AI_SETDELAYCOMPS, &stDelayCompsParam);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("Set AI Delay Compensation Failed 0x%x \n", s32Ret);
    }

    return s32Ret;
}

mt_s32 MT_MPI_AI_GetDelay(mt_handle hAI, MT_UNF_AI_DELAY_S *pstDelay)
{
    mt_s32 s32Ret;
    AI_DelayComps_Param_S stDelayCompsParam;

    CHECK_AI_ID(hAI);
    CHECK_AI_NULL_PTR(pstDelay);

    stDelayCompsParam.hAi = hAI;

    s32Ret = ioctl(g_s32AIFd, CMD_AI_GETDELAYCOMPS, &stDelayCompsParam);
    if (MT_SUCCESS != s32Ret) {
	MT_ERR_AI("Get AI Delay Compensation Failed 0x%x \n", s32Ret);
    } else {
	memcpy(pstDelay, &stDelayCompsParam.stDelayComps, sizeof(MT_UNF_AI_DELAY_S));
    }

    return s32Ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */
