/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include "config.h"
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/times.h>
#include "mt_type.h"
#include "mtos_task.h"
#include "mtos_mem.h"
#include "mt_common.h"
#include "mtsu_type.h"
#include "mtsu_svr_mssplayer.h"
#include "suplayer_internal.h"
#include "SmoothStreamingAdapter.h"
#include "fifo_kw.h"
#include "drv_adp.h"
#include "ts_sequence.h"
#include "file_playback_sequence.h"

/***************************************************************************************
*
*
*
*
*
*
*
*
*                                public  api  for suplayer
*
*
*
*
*
*
*
*
*****************************************************************************************/
static mtUNF_SUPLAYER_STATUS_S mss_suplayer_status[MT_SRV_SUPLAYER_INSTANCE_MAX]={{0}};
static pthread_mutex_t   g_mss_suplayMutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_MSS_SUPLAY_LOCK()        (void)pthread_mutex_lock(&g_mss_suplayMutex);
#define MT_MSS_SUPLAY_UNLOCK()      (void)pthread_mutex_unlock(&g_mss_suplayMutex);

//#define MSS_SUPLAYER_TRACE_ENABLE

#if defined(MSS_SUPLAYER_TRACE_ENABLE)
#define MSS_SUPLAYER_MSG(format, args...) printf(format, ##args)
#else
#define MSS_SUPLAYER_MSG(format, args... )   {}
#endif

#define MSS_SUPLAYER_ERR_MSG(format, args...) printf(format, ##args)

#define   SUB_FIFO_LEN             (128*1024)


typedef struct mtUNF_MPLAYER_PRIVATE
{
	MT_U32	load_success;
	MT_HANDLE hdl;
	void *pSspkHandle;

	void * p_sub_fifo_handle;
	unsigned int sub_fifo_mutex;
	unsigned char subt_buf[2048];
}mtUNF_MSS_PLAYER_PRIVATE_S;


static SSAdapter_Callback_t callback;
static void* pmssplayer = NULL;
static int mss_player_state = 0;
static MT_BOOL is_loadmedia_succuss = FALSE;
static MT_BOOL is_pause_to_play = FALSE;


void mss_sub_fifo_init()
{
		mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		if (mssplayer_pri){
			if (mssplayer_pri->p_sub_fifo_handle) {
			  MSS_SUPLAYER_MSG("\n[%s]%d - p_sub_fifo_handle = 0x%x\n", __func__, __LINE__, mssplayer_pri->p_sub_fifo_handle);

				void * p_start_pos = NULL;
				p_start_pos = (void *)(((fifo_type_t *)(mssplayer_pri->p_sub_fifo_handle))->start_pos);
				if(p_start_pos)
				{
				mtos_free(p_start_pos);
				((fifo_type_t *)(mssplayer_pri->p_sub_fifo_handle))->start_pos = 0;			
				}		
				deinit_fifo_kw(mssplayer_pri->p_sub_fifo_handle);
				mssplayer_pri->p_sub_fifo_handle = NULL;
			}

			void * p_tmp = NULL;
			p_tmp = mtos_malloc(SUB_FIFO_LEN);
			if (p_tmp) {
				memset(p_tmp, 0, SUB_FIFO_LEN);
			}
			mssplayer_pri->p_sub_fifo_handle = init_fifo_kw(p_tmp, SUB_FIFO_LEN);
			MSS_SUPLAYER_MSG("\n[%s]%d - p_sub_fifo_handle = 0x%x\n", __func__, __LINE__, mssplayer_pri->p_sub_fifo_handle);
		}
		else
		{
			MSS_SUPLAYER_ERR_MSG("\n[%s] - mssplayer_pri = 0x%x\n", __func__, mssplayer_pri);
		}
		
}

void mss_sub_fifo_deinit()
{
    //deinit subtitle 
		mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		if (mssplayer_pri){
			if (mssplayer_pri->p_sub_fifo_handle) {
				void * p_start_pos = NULL;
				p_start_pos = (void *)(((fifo_type_t *)(mssplayer_pri->p_sub_fifo_handle))->start_pos);
				if(p_start_pos)
				{
					mtos_free(p_start_pos);
					((fifo_type_t *)(mssplayer_pri->p_sub_fifo_handle))->start_pos = 0;			
				}
				deinit_fifo_kw(mssplayer_pri->p_sub_fifo_handle);
				mssplayer_pri->p_sub_fifo_handle = NULL;
			}
		}
}

void mss_sub_fifo_clear()
{
		mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		if (mssplayer_pri){
			if (mssplayer_pri->p_sub_fifo_handle) {
				clear_sub_fifo_kw(mssplayer_pri->p_sub_fifo_handle);
			}
		}
}
void mss_write_sub_fifo(char * p_data, long size)
{
		if (!p_data)
		{
      MSS_SUPLAYER_ERR_MSG("\n[%s] - p_data = 0x%x\n", __func__, p_data);
			return;
		}
		
		mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		if (mssplayer_pri){
			if (mssplayer_pri->p_sub_fifo_handle) {
      	MSS_SUPLAYER_MSG("\n[%s] - write_sub_fifo_kw p_data = 0x%x\n", __func__, p_data);
				write_sub_fifo_kw(mssplayer_pri->p_sub_fifo_handle, p_data, size);
			}
		}
		else
		{
			MSS_SUPLAYER_ERR_MSG("\n[%s] [%d]\n", __func__, __LINE__);
		}
}

int mss_read_sub_fifo(char * p_data)
{
    int size = 0;
		if (!p_data)
		{
      MSS_SUPLAYER_ERR_MSG("\n[%s] - p_data = 0x%x\n", __func__, p_data);
			return;
		}
		
		mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		if (mssplayer_pri){
			if (mssplayer_pri->p_sub_fifo_handle) {
				size = read_sub_fifo_kw(mssplayer_pri->p_sub_fifo_handle, p_data);
      	MSS_SUPLAYER_MSG("\n[%s] - read_sub_fifo_kw size = 0x%x\n", __func__, size);
			}
			else
			{
				MSS_SUPLAYER_ERR_MSG("\n[%s] - p_sub_fifo_handle= 0x%x\n", __func__, mssplayer_pri->p_sub_fifo_handle);
			}
		}
		MSS_SUPLAYER_MSG("\n[%s] - ret size = 0x%x\n", __func__, size);
		return size;
}

static SS_ADAPTER_ERROR_STATUS Event_Callback(SS_ADAPTER_EVENT_TYPE type, int32_t param)
{
    unsigned long  cur_hour = 0;
    unsigned long  cur_min  = 0;
    unsigned long  cur_sec = 0;
    MT_SVR_PLAYER_EVENT_S event_s;
    MT_SVR_PLAYER_STATE_E data;
    MT_U32 (*pfunc)(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_S *pstruEvent);
    ss_adapter_event_info_t *pEvent_info = (ss_adapter_event_info_t *)param;
    if(type == SS_ADAPTER_EVENT_TYPE_STATUS)
    {
        ss_adapter_status_info_t *pStatus_info = (ss_adapter_status_info_t *)pEvent_info->m_Data;
        MSS_SUPLAYER_MSG("\n[%s] - m_currentState = %d\n", __func__, (int)pStatus_info->m_currentState);
        switch (pStatus_info->m_currentState) {
            case eSS_ADAPTER_EVENT_TunerState_Closed:
                if(pmssplayer){
                    mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
                    mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
                    mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
                    if (mssplayer_pri){
                        data = MT_SVR_PLAYER_STATE_STOP;
                        event_s.eEvent = MT_SVR_PLAYER_EVENT_STATE_CHANGED;
                        event_s.pu8Data = (MT_U8 *)(&data);
                        event_s.u32Len = sizeof(MT_SVR_PLAYER_STATE_E);
                        pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
                        (*pfunc)(NULL, &event_s);
                    }
                }
                MSS_SUPLAYER_MSG("\n &&&&@@@@@!!!1%s %d eSS_ADAPTER_EVENT_TunerState_Closed\n", __func__, __LINE__);
                mss_player_state = 2;
                break;
            case eSS_ADAPTER_EVENT_TunerState_Playing:
                MSS_SUPLAYER_MSG("\neSS_ADAPTER_EVENT_TunerState_Playing\n");
                cur_hour = (pStatus_info->m_currentTime / 1000) / 3600;
                cur_min = ((pStatus_info->m_currentTime / 1000) - (cur_hour * 3600)) / 60;
                cur_sec = (pStatus_info->m_currentTime / 1000) - (cur_hour * 3600) - cur_min * 60;
                if(pmssplayer){
                    if(is_pause_to_play == TRUE){
                      if(vdec_get_seek_frame_setup_mode() == FALSE){
                            vdec_resume(NULL);
                            aud_resume_vsb(NULL);
                        }
                        is_pause_to_play = FALSE;
                    }
                    if(pStatus_info->m_update == eSS_ADAPTER_EVENT_STATUS_Heartbeat){
                        mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
                        mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
                        mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
                        if (mssplayer_pri){
                            MT_SVR_PLAYER_PROGRESS_S data;
                            data.s64BufferSize = 0;
                            data.s64Duration = (pStatus_info->m_endTime - pStatus_info->m_startTime)/1000;
                            data.u32Progress = pStatus_info->m_currentTime;
                            event_s.eEvent = MT_SVR_PLAYER_EVENT_PROGRESS;
                            event_s.pu8Data = (MT_U8 *)(&data);
                            event_s.u32Len = sizeof(MT_SVR_PLAYER_PROGRESS_S);
                            pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
                            (*pfunc)(NULL, &event_s);
                            MSS_SUPLAYER_MSG("\n &&&&@@@@@!!!1%s %d eSS_ADAPTER_EVENT_TunerState_Playing current time:%lld,total time:%lld\n", __func__, __LINE__, pStatus_info->m_currentTime/1000,(pStatus_info->m_endTime - pStatus_info->m_startTime)/1000);
                        }
                    }
                }
                break;
            case eSS_ADAPTER_EVENT_TunerState_MediaEnded:
                 if(pmssplayer){
                     mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
                     mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
                     mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
                     if (mssplayer_pri){
                         data = MT_SVR_PLAYER_STATE_STOP;
                         event_s.eEvent = MT_SVR_PLAYER_EVENT_EOF;
                         event_s.pu8Data = (MT_U8 *)(&data);
                         event_s.u32Len = sizeof(MT_SVR_PLAYER_STATE_E);
                         pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
                        (*pfunc)(NULL, &event_s);
					 }
				 }
				 MSS_SUPLAYER_MSG("\n &&&&@@@@@!!!1%s %d eSS_ADAPTER_EVENT_TunerState_MediaEnded\n", __func__, __LINE__);
				 mss_player_state = 2;
				 break;
			 case eSS_ADAPTER_EVENT_TunerState_Paused:
				 if(pmssplayer){
                    vdec_pause(NULL);
				    aud_pause_vsb(NULL);
                    is_pause_to_play = TRUE;
				 }
                 MSS_SUPLAYER_MSG("\n &&&&@@@@@!!!1%s %d eSS_ADAPTER_EVENT_TunerState_Paused\n", __func__, __LINE__);
				 break;
            default :
               break;
        }
    }
    else if (type == SS_ADAPTER_EVENT_TYPE_ERROR)
    {
			ss_adapter_error_info_t *pError_info = (ss_adapter_error_info_t *)pEvent_info->m_Data;
			MSS_SUPLAYER_ERR_MSG("\n%s %d m_errorCode:%d\n", __func__, __LINE__,pError_info->m_errorCode);
			switch (pError_info->m_errorCode) {
				case SS_ADAPTER_EVENT_ERROR_None:
					//no error
					break;
				case SS_ADAPTER_EVENT_ERROR_DrmInitFailed:
				case SS_ADAPTER_EVENT_ERROR_ManifestParseFailed:
				case SS_ADAPTER_EVENT_ERROR_ManifestVersionUnsupported:
				case SS_ADAPTER_EVENT_ERROR_ManifestInvalid:
				case SS_ADAPTER_EVENT_ERROR_ManifestHttpInvalidResult:
					if(pmssplayer){
					mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
					mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
					mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
					if (mssplayer_pri){
						MT_SVR_PLAYER_ERROR_E error = MT_SVR_PLAYER_ERROR_PLAY_FAIL;
						if(pError_info->m_isLive == 1){//live media
							event_s.eEvent = MT_SVR_PLAYER_EVENT_STREAM_NOT_AVAIABLE;							
							printf("\n%s %d live media error\n", __func__, __LINE__);
						}else{
							event_s.eEvent = MT_SVR_PLAYER_EVENT_ERROR;
						}
						event_s.pu8Data = (MT_U8 *)(&error);
						event_s.u32Len = sizeof(MT_SVR_PLAYER_ERROR_E);
						pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
						(*pfunc)(NULL, &event_s);
						}
					}
					//mss_player_state = 2;
					break;
				case SS_ADAPTER_EVENT_ERROR_SocketReadError:
				case SS_ADAPTER_EVENT_ERROR_SocketOpenFailed:
				case SS_ADAPTER_EVENT_ERROR_SocketConnectFailed:
				case SS_ADAPTER_EVENT_ERROR_SocketSendFailed:
				case SS_ADAPTER_EVENT_ERROR_SocketRecvFailed:
					if(pmssplayer){
					MT_SVR_PLAYER_ERROR_E error = MT_SVR_PLAYER_ERROR_NON;
					mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
					mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
					mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
					if (mssplayer_pri){
					if(is_loadmedia_succuss == TRUE){
						error = MT_SVR_PLAYER_ERROR_TIMEOUT;//during play socket error occur
						if(pError_info->m_isLive == 1){//live media
							event_s.eEvent = MT_SVR_PLAYER_EVENT_STREAM_NOT_AVAIABLE;							
							printf("\n%s %d live media error\n", __func__, __LINE__);
						}else{
							event_s.eEvent = MT_SVR_PLAYER_EVENT_ERROR;
						}
					}else{
						error = MT_SVR_PLAYER_ERROR_PLAY_FAIL;//during loadmedia socket error occur						
						event_s.eEvent = MT_SVR_PLAYER_EVENT_ERROR;
					}
					event_s.pu8Data = (MT_U8 *)(&error);
					event_s.u32Len = sizeof(MT_SVR_PLAYER_ERROR_E);
					pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
					(*pfunc)(NULL, &event_s);
					}
					MSS_SUPLAYER_ERR_MSG("\n%s %d network error occur error:%d\n", __func__, __LINE__, error);
					}
					//mss_player_state = 2;
					break;
				default :
					if(pmssplayer){
						mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
						mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
						mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
						if (mssplayer_pri){
						MT_SVR_PLAYER_ERROR_E error = MT_SVR_PLAYER_ERROR_UNKNOW;
						event_s.eEvent = MT_SVR_PLAYER_EVENT_ERROR;
						event_s.pu8Data = (MT_U8 *)(&error);
						event_s.u32Len = sizeof(MT_SVR_PLAYER_ERROR_E);
						pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
						(*pfunc)(NULL, &event_s);
						}
					}
					//mss_player_state = 2;
					break;
			}
    }
    else if (type == SS_ADAPTER_EVENT_TYPE_MANIFEST_READY)
    {
        //load media successfully
        if(pmssplayer){
            mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
            mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
            mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
            if (mssplayer_pri){
                data = MT_SVR_PLAYER_STATE_PLAY;
                event_s.eEvent = MT_SVR_PLAYER_EVENT_UPDATE_FILE_INFO;
                event_s.pu8Data = (MT_U8 *)(&data);
                event_s.u32Len = sizeof(MT_SVR_PLAYER_STATE_E);
                pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
                (*pfunc)(NULL, &event_s);
                is_loadmedia_succuss = TRUE;
            }
        }
    }
		else if (type == SS_ADAPTER_EVENT_TYPE_SELECTED_STREAM_CHANGED)
		{
			MSS_SUPLAYER_MSG("\n[%s] - SS_ADAPTER_EVENT_TYPE_SELECTED_STREAM_CHANGED\n", __func__);
		}
		else if (type == SS_ADAPTER_EVENT_TYPE_FRAGEMENT_DATA_RECEIVED)
		{
			MSS_SUPLAYER_MSG("\n[%s] - SS_ADAPTER_EVENT_TYPE_FRAGEMENT_DATA_RECEIVED len=%d\n", __func__, pEvent_info->m_Len);

			if(pmssplayer){
            mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)pmssplayer;
            mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
            mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
            if (mssplayer_pri){
								//write to subtitle fifo
								MSS_SUPLAYER_MSG("\n[%s] - write to subt fifo len=%d.\n", __func__, pEvent_info->m_Len);
								mss_write_sub_fifo(pEvent_info->m_Data, pEvent_info->m_Len);

                event_s.eEvent = MT_SVR_PLAYER_EVENT_NEW_SUBTITLE_RECEIVED;
                event_s.pu8Data = NULL;
                event_s.u32Len = 0;
                pfunc = ((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb;
                (*pfunc)(NULL, &event_s);
            }
			}
		}

    return SS_ADAPTER_ERROR_STATUS_OK;
}


MT_VOID* MT_SVR_PLAYER_MSS_Init(mt_void* args)
{
	mtUNF_SUPLAYER_IN_ARG_S * in_args = NULL;
	mtUNF_SUPLAYER_STATUS_S * ret = NULL;
	MT_U32 i=0,j=0;

	MT_MSS_SUPLAY_LOCK();
	
	in_args = (mtUNF_SUPLAYER_IN_ARG_S *)args;

	if(in_args->ptype == MT_SUPLAYER_MSS){
		
		MSS_SUPLAYER_MSG("[%s]\n  MT_SUPLAYER_MSS !!!\n",__func__);


		for(i=0; i<MT_SRV_SUPLAYER_INSTANCE_MAX; i++){
			
			if(mss_suplayer_status[i].ptype== MT_SUPLAYER_MSS){
				goto INIT_FAIL0;
			}else if(mss_suplayer_status[i].ptype == MT_SUPLAYER_UNKNOWN){
				j = i;
				if(i == MT_SRV_SUPLAYER_INSTANCE_MAX - 1){
				    break;
				}
			}
			
		}
		
		if(i == MT_SRV_SUPLAYER_INSTANCE_MAX){
			goto INIT_FAIL0;
		}

		
		ret = &mss_suplayer_status[j];
		pmssplayer =  ret;
		ret->instance = j+1;
		ret->ptype = MT_SUPLAYER_MSS;
		ret->status = MT_SVR_PLAYER_STATE_INIT;
		ret->pri = NULL;
		MSS_SUPLAYER_MSG("\n[%s]suplayer type=%d,%d\n",__func__,mss_suplayer_status[j].ptype,ret->ptype);

	}else{
		MSS_SUPLAYER_ERR_MSG("\n[%s] Unknown Player type!!!\n",__func__);
	}

	MT_MSS_SUPLAY_UNLOCK();
	
	MSS_SUPLAYER_MSG("\n%s %d \n", __FUNCTION__, __LINE__);
	
	return (MT_VOID *)ret;

INIT_FAIL0:
	MT_MSS_SUPLAY_UNLOCK();
	return NULL;
}




MT_S32 MT_SVR_PLAYER_MSS_Create(const MT_SVR_PLAYER_PARAM_S *pstruParam, MT_HANDLE *phPlayer)
{
	mtUNF_SUPLAYER_STATUS_S *iargs = (mtUNF_SUPLAYER_STATUS_S *)pstruParam->suplayer_status;
	MT_S32	ret = SUCCESS;
	MSS_SUPLAYER_MSG("\n%s %d ptype[%d]\n", __FUNCTION__, __LINE__, iargs->ptype);
	if(iargs->ptype == MT_SUPLAYER_MSS){

		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		if((MT_HANDLE *)(*phPlayer) == NULL){
			*phPlayer = (MT_HANDLE )mtsu_malloc(sizeof(MT_PLAYBACK_INTERNAL_T));
			if((MT_HANDLE *)(*phPlayer) == NULL){
				goto FAIL0;
			}
			mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S * )mtsu_malloc(sizeof(mtUNF_MSS_PLAYER_PRIVATE_S));
			if(mssplayer_pri == NULL){
				goto FAIL1;
			}
			MSS_SUPLAYER_MSG("\n%s %d \n", __FUNCTION__, __LINE__);
			memset(mssplayer_pri,0,sizeof(mtUNF_MSS_PLAYER_PRIVATE_S));
			mssplayer_pri->hdl = *phPlayer;
			mssplayer_pri->load_success = 0;
			mssplayer_pri->pSspkHandle = NULL;
			mss_suplayer_status[iargs->instance - 1].pri = (MT_VOID*)(mssplayer_pri);
			((MT_PLAYBACK_INTERNAL_T*)(*phPlayer))->suplayer_status = (MT_VOID*)(&mss_suplayer_status[iargs->instance - 1]);
		}else{
			
			goto FAIL0;
			
		}

		/////init for player handle 
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Init(&mssplayer_pri->pSspkHandle);
		if(sspk_status != SS_ADAPTER_ERROR_STATUS_OK) {
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Init failed! err=%d\n", sspk_status);
			goto FAIL1;
		}
		callback.event_callback = (SSAdapter_Event_Callback_t)Event_Callback;
		sspk_status = SSAdapter_Register_Callback(mssplayer_pri->pSspkHandle, &callback);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Register_Callback failed! err=%d\n", sspk_status);
			goto FAIL1;
		}
		//init subtitle fifo
		MSS_SUPLAYER_MSG("SSAdapter_Register_Callback mss_sub_fifo_init\n");
		mss_sub_fifo_init();
	}
	else{
		
	}
	MSS_SUPLAYER_MSG("\n%s %d ret=%d\n", __FUNCTION__, __LINE__, ret);
	return ret;

FAIL1:
	if(phPlayer){
		mtsu_free(phPlayer);
		phPlayer = NULL;
	}

FAIL0:
	ret = ERR_FAILURE;

	return ret;
}

MT_S32 MT_SVR_PLAYER_MSS_SetMedia(MT_HANDLE hPlayer, MT_U32 eType, MT_SVR_PLAYER_MSS_MEDIA_S *pstruMedia)
{
	MT_S32 ret = SUCCESS;
	ss_adapter_media_info_t mediaInfo;
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);
	if(phdl->ptype == MT_SUPLAYER_MSS)
	{
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_PREPARING;
		mediaInfo.url = pstruMedia->aszUrl;
		mediaInfo.DrmCustomData = pstruMedia->aszCustomData;
		mediaInfo.licenseUrl = pstruMedia->aszLicenseUrl;
		mediaInfo.DrmCertPath = pstruMedia->aszCertPath;

		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Set_Media(mssplayer_pri->pSspkHandle, &mediaInfo);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Set_Media failed! err=%d\n", sspk_status);
			return - 1;
		}
	}
	else
	{

	}

	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);
	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Play(MT_HANDLE hPlayer, MT_PCHAR url)
{
	MT_S32 ret = SUCCESS;
	
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);
	if(phdl->ptype == MT_SUPLAYER_MSS)
	{
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_PLAY;
        vdec_set_seek_frame_setup_mode(FALSE);
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Open(mssplayer_pri->pSspkHandle, url);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Open failed! err=%d\n", sspk_status);
			return - 1;
		}
		mss_player_state = 1;
		is_loadmedia_succuss = FALSE;
	}
	else
	{
		
	}
	
	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return ret;

}

MT_S32 MT_SVR_PLAYER_MSS_Stop(MT_HANDLE hPlayer)
{
	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_U32	tout_cnt = 0;
	MT_U32	tout_cnt_max = 1000;
	MT_S32 ret = MT_SUCCESS;
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_STOP;
       vdec_set_seek_frame_setup_mode(FALSE);
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Close(mssplayer_pri->pSspkHandle);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Close failed! err=%d\n", sspk_status);
			return - 1;
		}
		while(mss_player_state != 2 && tout_cnt < tout_cnt_max){
			mtos_task_sleep(3);//sleep 3ms
			tout_cnt++;
			MSS_SUPLAYER_MSG("\n%s_%d:stop mssplayer, tout_cnt=%d\n",__func__,__LINE__,tout_cnt);
		}
		/*during loadmeida exit by user need return ok*/
		if(mss_player_state != 2 && is_loadmedia_succuss == TRUE){
			MSS_SUPLAYER_MSG("\n%s_%d:stop mssplayer fail, fp_state=%d\n",__func__,__LINE__,mss_player_state);
			mss_player_state = 2;
		    return -1;
		}
	}else{
		
	}

	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return ret;

}


MT_S32 MT_SVR_PLAYER_MSS_Pause(MT_HANDLE hPlayer)
{
	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS)
	{
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
       vdec_set_seek_frame_setup_mode(FALSE);
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Pause(mssplayer_pri->pSspkHandle);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Pause failed! err=%d\n", sspk_status);
			return - 1;
		}
	}
	else
	{
	
	}
	
	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
	
}




MT_S32 MT_SVR_PLAYER_MSS_Resume(MT_HANDLE hPlayer)
{
	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
       vdec_set_seek_frame_setup_mode(FALSE);
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Resume(mssplayer_pri->pSspkHandle);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Resume failed! err=%d\n", sspk_status);
			return - 1;
		}
	}else{

	
	}
	
	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	
	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_TPlay(MT_HANDLE hPlayer, MT_S32 s32Speed)
{
	MT_S32 ret = SUCCESS;

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);
	if(phdl->ptype == MT_SUPLAYER_MSS)
	{
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_PLAY;
       vdec_set_seek_frame_setup_mode(FALSE);
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_FastSpeed(mssplayer_pri->pSspkHandle, (const float)s32Speed);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_FastSpeed failed! err=%d\n", sspk_status);
			return - 1;
		}
	}
	else
	{

	}

	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return ret;

}


MT_S32 MT_SVR_PLAYER_MSS_Seek(MT_HANDLE hPlayer, mt_s32 s32TimeStampSec)
{
	MSS_SUPLAYER_MSG("\n%s %d ===start start=%d=\n", __FUNCTION__, __LINE__,s32TimeStampSec);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
       vdec_set_seek_frame_setup_mode(FALSE);
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Seek(mssplayer_pri->pSspkHandle, s32TimeStampSec);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Seek failed! err=%d\n", sspk_status);
			return - 1;
		}
		//clear subtitle fifo after seek
		mss_sub_fifo_clear();
	}
	else
	{

	
	}

	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Frame_Seek(MT_HANDLE hPlayer, mt_s32 s32TimeStampSec)
{
	printf("\n%s %d ===start start=%d=\n", __FUNCTION__, __LINE__,s32TimeStampSec);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
       vdec_set_seek_frame_setup_mode(TRUE);
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Seek(mssplayer_pri->pSspkHandle, s32TimeStampSec);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Cmd_Seek failed! err=%d\n", sspk_status);
			return - 1;
		}
		//clear subtitle fifo after seek
		mss_sub_fifo_clear();
       
	}
	else
	{

	
	}

	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}




MT_S32 MT_SVR_PLAYER_MSS_RegCallback(MT_HANDLE hPlayer, MT_SVR_PLAYER_EVENT_FN pfnCallback)
{
	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);


	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);

	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		((MT_PLAYBACK_INTERNAL_T *)mssplayer_pri->hdl)->cb  = pfnCallback;
	}else{

	}


    MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Set_Aud_Track(MT_HANDLE hPlayer, int track_id)
{
	MSS_SUPLAYER_MSG("\n%s_%d:set audio_id=%d\n",__FUNCTION__,__LINE__,track_id);
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Change_Audio_Track(mssplayer_pri->pSspkHandle, track_id);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("%s() SSAdapter_Cmd_Change_Audio_Track failed! err=%d\n", __FUNCTION__, sspk_status);
			return - 1;
		}
	}else{
	}
	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Set_Subtitle(MT_HANDLE hPlayer, int sub_id)
{
	MSS_SUPLAYER_MSG("\n%s_%d:set sub_id=%d\n",__FUNCTION__,__LINE__,sub_id);
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Cmd_Set_Subt_Id(mssplayer_pri->pSspkHandle, sub_id);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("%s() SSAdapter_Cmd_Set_Subt_Id failed! err=%d\n", __FUNCTION__, sspk_status);
			return - 1;
		}
	}else{
	}	
	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Get_Media_Info(MT_HANDLE hPlayer, void * pResult)
{
	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);
	ss_adapter_film_info_t film_info;
	ss_adapter_track_lang_t *pAudio_track_info;
	memset(&film_info, 0, sizeof(ss_adapter_film_info_t));
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		//film info
		sspk_status = SSAdapter_Get_Film_Info(mssplayer_pri->pSspkHandle, &film_info);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Get_Film_Info failed! err=%d\n", sspk_status);
			return - 1;
		}
		/*Make sure FILE_INFO_S is started with FILM_INFO_T*/
		if(sizeof(MT_SVR_PLAYER_FILM_S) == sizeof(ss_adapter_film_info_t))
			((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->film = *((MT_SVR_PLAYER_FILM_S *)&film_info);
		else
			MSS_SUPLAYER_ERR_MSG("\n%s_%d:Error film info structure different!!!\n",__func__,__LINE__);
		//audio track info
		if (film_info.audio_track_num > 0 && film_info.audio_track_num <= SS_ADAPTER_MAX_A_STREAMS)
		{
			pAudio_track_info = mtos_malloc(sizeof(ss_adapter_track_lang_t) * film_info.audio_track_num);
			memset(pAudio_track_info, 0, sizeof(ss_adapter_track_lang_t) * film_info.audio_track_num);

			SSAdapter_Get_Audio_Track_Lang(mssplayer_pri->pSspkHandle, pAudio_track_info);
			if(sizeof(MT_SVR_PLAYER_TRACK_LANG_S) == sizeof(ss_adapter_track_lang_t))
			{
				((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->aud_lang_cnt = film_info.audio_track_num;
				((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->aud_lang = (MT_SVR_PLAYER_TRACK_LANG_S*)pAudio_track_info;
				for (int audioIdx = 0; audioIdx < film_info.audio_track_num; ++audioIdx)
				{
						MSS_SUPLAYER_MSG("MT_SVR_PLAYER_MSS_Get_Media_Info() audio track %d, lang=%s, title=%s\n",
												pAudio_track_info[audioIdx].track_id,
												pAudio_track_info[audioIdx].lang,
												pAudio_track_info[audioIdx].title);
				}
			}
			else
			{
				MSS_SUPLAYER_ERR_MSG("\n%s_%d:Error audio track structure different!!!\n",__func__,__LINE__);
			}
		}
    //subtitle info
		ss_adapter_subt_t *subt = mtos_malloc(sizeof(ss_adapter_subt_t));
		sspk_status = SSAdapter_Get_Subt_Info(mssplayer_pri->pSspkHandle, (void *)subt);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_ERR_MSG("SSAdapter_Get_Subt_Info failed! err=%d\n", sspk_status);
			return - 1;
		}
		if(sizeof(MT_SVR_PLAYER_SUBT_S) == sizeof(ss_adapter_subt_t))
			((MT_SVR_PLAYER_FILE_INFO_S *)pResult)->subt = (MT_SVR_PLAYER_SUBT_S *)subt;
		else
			MSS_SUPLAYER_ERR_MSG("\n%s_%d:Error subt info structure different!!!\n",__func__,__LINE__);

	}
	else
	{

	}
	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Get_Subt_Data(MT_HANDLE hPlayer, void * subt, int *pts, MT_U32 *size)
{
	int size_sub = 0;
	int size_read = 0;
	unsigned char p_data[2048] = {0};
	void ** retSubt = (void **)subt;
	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);

	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;

    *retSubt =  mssplayer_pri->subt_buf;
    memset(mssplayer_pri->subt_buf,0,2048);
    size_read = mss_read_sub_fifo((char *)p_data);
		*size = 0;

    if (size_read == 0) {
				MSS_SUPLAYER_ERR_MSG("\n%s %d \n", __FUNCTION__, __LINE__);
        return 0;
    } else {
        size_sub = (p_data[0] << 24) + (p_data[1] << 16) + (p_data[2] << 8) + p_data[3];
        *pts = ((p_data[4] << 24) + (p_data[5] << 16) + (p_data[6] << 8) + p_data[7]);

        if (size_sub > 2048) {
            size_sub = 2048;
        }

        memcpy(*retSubt, (p_data + 8), size_sub);
				*size = size_sub;
    }
	}
	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_Destroy(MT_HANDLE hPlayer)
{

	MSS_SUPLAYER_MSG("\n%s %d ===start start==\n", __FUNCTION__, __LINE__);
    MT_S32 i = 0;
	MT_U32	tout_cnt = 0;
	MT_U32	tout_cnt_max = 500;
	MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
	if(p_MonPlayer == NULL){
		MSS_SUPLAYER_MSG("\n%s [ERROR] =p_MonPlayer == NULL!!!!\n", __FUNCTION__);
		return MT_SUCCESS;
	}
	
	mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl == NULL){
		
		MSS_SUPLAYER_MSG("\n%s [ERROR] =phdl == NULL!!!!\n", __FUNCTION__);
		return MT_SUCCESS;
	
	}
	if(phdl->ptype == MT_SUPLAYER_MSS){
		while(mss_player_state != 2 && tout_cnt < tout_cnt_max){
			mtos_task_sleep(3);//sleep 3ms
			tout_cnt++;
		}
		if(mss_player_state != 2){
			MSS_SUPLAYER_MSG("\n%s_%d:Destroy mssplayer fail, fp_state=%d\n",__func__,__LINE__,mss_player_state);
			mss_player_state = 2;
		//	return MT_FAILURE; //no ret err even in play state.
		}
		mtUNF_MSS_PLAYER_PRIVATE_S *mssplayer_pri;
		mssplayer_pri = (mtUNF_MSS_PLAYER_PRIVATE_S *)phdl->pri;
		phdl->status = MT_SVR_PLAYER_STATE_STOP;
		SS_ADAPTER_ERROR_STATUS sspk_status = SS_ADAPTER_ERROR_STATUS_OK;
		sspk_status = SSAdapter_Exit(mssplayer_pri->pSspkHandle);
		if (sspk_status != SS_ADAPTER_ERROR_STATUS_OK)
		{
			MSS_SUPLAYER_MSG("SSAdapter_Exit failed! err=%d\n", sspk_status);
			return - 1;
		}
		mtsu_free((mtUNF_MSS_PLAYER_PRIVATE_S *)mssplayer_pri);
		mssplayer_pri = NULL;
		mtsu_free((MT_PLAYBACK_INTERNAL_T *)p_MonPlayer);
		p_MonPlayer = NULL;		
	}else{
	
	}

    for(i=0; i<MT_SRV_SUPLAYER_INSTANCE_MAX; i++){ 

		mss_suplayer_status[i].ptype = MT_SUPLAYER_UNKNOWN;
    }

	//deinit subtitle fifo
	mss_sub_fifo_deinit();

	MSS_SUPLAYER_MSG("\n%s %d ===end end==\n", __FUNCTION__, __LINE__);

	return MT_SUCCESS;
}


MT_S32 MT_SVR_PLAYER_MSS_Deinit(MT_HANDLE hPlayer)
{

	MSS_SUPLAYER_MSG("\n%s %d \n", __FUNCTION__, __LINE__);
	MT_MSS_SUPLAY_LOCK();
	MT_MSS_SUPLAY_UNLOCK();
	return 0;
}

extern  void file_seq_avplay_handle_set(MT_HANDLE handle_avplay, int handle_track);
MT_S32 MT_SVR_PLAYER_MSS_SetAvplayHdl(MT_HANDLE hPlayer, MT_HANDLE hAvplay, MT_HANDLE hTrack)
{
  MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
  mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		file_seq_avplay_handle_set(hAvplay, hTrack);
	}else{
	}
	return MT_SUCCESS;
}

MT_S32 MT_SVR_PLAYER_MSS_SetVoHdl(MT_HANDLE hPlayer,int vHandle)
{
  MT_PLAYBACK_INTERNAL_T *p_MonPlayer = (MT_PLAYBACK_INTERNAL_T *)hPlayer;
  mtUNF_SUPLAYER_STATUS_S *phdl = (mtUNF_SUPLAYER_STATUS_S *)(p_MonPlayer->suplayer_status);
	if(phdl->ptype == MT_SUPLAYER_MSS){
		file_seq_vo_handle_set(vHandle);
	}else{
	}
	return MT_SUCCESS;
}
