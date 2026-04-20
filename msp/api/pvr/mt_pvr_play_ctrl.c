/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <malloc.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>

#include "mt_type.h"
#ifdef CONFIG_MT_CHIP_SYMPHONY1
#include "mt_unf_cipher.h"
#else
#include "mt_unf_cipher_v2.h"
#endif

#include "mt_mpi_pvr.h"
#include "mt_mpi_avplay.h"
#include "mt_mpi_disp.h"

#include "pvr_debug.h"
#include "mt_pvr_play_ctrl.h"
#include "mt_pvr_index.h"
#include "mt_pvr_rec_ctrl.h"
#include "mt_pvr_intf.h"
#include "mt_pvr_priv.h"
#include "mt_pvr_smooth_ctrl.h"
#include "mt_mpi_demux.h"
#include "mt_drv_pvr.h"
#include "mt_common.h"
#include "mt_pvr_addon_api.h"

#include "HA.AUDIO.G711.codec.h"
#include "HA.AUDIO.MP3.decode.h"
#include "HA.AUDIO.MP2.decode.h"
#include "HA.AUDIO.AAC.decode.h"
#include "HA.AUDIO.DRA.decode.h"
#include "HA.AUDIO.PCM.decode.h"
#include "HA.AUDIO.WMA9STD.decode.h"
#include "HA.AUDIO.AMRNB.codec.h"
#include "HA.AUDIO.AMRWB.codec.h"
#include "HA.AUDIO.TRUEHDPASSTHROUGH.decode.h"
#include "HA.AUDIO.DOLBYTRUEHD.decode.h"
#include "HA.AUDIO.DTSHD.decode.h"
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
#endif
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"

#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.AAC.encode.h"

#include "mt_common.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* End of #ifdef __cplusplus */

//#define PVR_DEBUG_PLAY_TIME_STEP_1
//#define PVR_DEBUG_ON_RUNTIME


#define PVR_CACHE_FRAME_ERROR           0x55aa55aa
#define TIMESHIFT_INVALID_CHN           0XFF
#define PVR_PLAY_MAX_SEND_BUF_SIZE      (256*1024)

#define PVR_DMX_TS_BUFFER_GAP           0x100   /*DMX_TS_BUFFER_GAP*/

#define PUSH_VBLOCK_SZ   (512*188*4)
#define PUSH_ABLOCK_SZ   (32*188)

#define MAX_CACHE_FRAME_SIZE 3

#define PVR_MAX_PTS_LEN 3

#define CHECKOVER_BY_DISPLAY

#ifndef MT_FREE
#define MT_FREE mt_free
#endif
#ifndef MT_MALLOC
#define MT_MALLOC mt_malloc
#endif

#define MT_PVR_FIND_IFRAME_DISTANCE     400//150 //fixed Bug #29470,some steam I frame gap may large than 150

#define MT_PVR_FIND_PTS_TOLERANCE 200 //start from 200 ms
#define MT_PVR_FIND_PTS_STEP    400
#define MT_PVR_FIND_PTS_COUNT   3

MT_U32 g_pvrplay_loglevel = 0;


extern MT_S32   g_s32PvrFd;      /*PVR module file description */
extern char api_pathname_pvr[];

#ifdef PVR_DEBUG_ON_RUNTIME
MT_U32 debug_get_displaytime_by_cache = 0;
#endif

/* initial flag for play module                                             */
STATIC PVR_PLAY_COMM_S g_stPlayInit;

static MT_U32  g_pvr_last_vid_frame_decoded = MT_FALSE;
static MT_U32  g_pvr_last_vid_frame_showed = MT_FALSE;
static MT_U32  g_pvr_last_i_frame = 0;

/* all information of play channel                                          */
STATIC PVR_PLAY_CHN_S g_stPvrPlayChns[PVR_PLAY_MAX_CHN_NUM];
#ifdef PVR_PROC_SUPPORT
static MT_PROC_ENTRY_S g_stPvrPlayProcEntry;
#endif
static FILE *g_pvrfpSend = NULL; /* handle of file */
static MT_BOOL g_bPlayTimerInitFlag = MT_FALSE;
static timer_t g_stPlayTimer;
static MT_BOOL g_bPvrTrickDump = MT_FALSE;
//extern MT_S32 MT_MPI_VO_GetWindowDelay(MT_HANDLE hWindow, MT_DRV_WIN_PLAY_INFO_S *pDelay);
extern MT_S32 pvr_create_thread_attr(pthread_attr_t *p_thread_attr, MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize);
//extern mt_s32 mt_proc_add_dir(const mt_char * pszName);
//extern MT_S32 PVR_Index_GetPreXFrame(PVR_INDEX_HANDLE handle, PVR_INDEX_ENTRY_S *pFrame);
#ifdef VMX_ADVCA_PVR
mt_mmz_buf_s  decrypt_rec_buff[2];
#endif //VMX_ADVCA_PVR

MT_S32 MT_PVR_PlayDispTPlay(PVR_PLAY_CHN_S *pChnAttr, MT_UNF_PVR_PLAY_SPEED_E speed,const MT_UNF_VCODEC_FRMRATE_S *pstTplayOpt);
MT_S32 MT_PVR_PlayGetPolicy(PVR_PLAY_CHN_S *pChnAttr, MT_UNF_PVR_PLAY_SPEED_E speed_x1024, PVR_PLAY_POLICY_S *pPvrPlayOpt);
MT_S32 MT_PVR_PlayCheckInfo(MT_U32 u32Chn);
MT_S32 MT_PVR_PlaySetPid(MT_U32 u32Chn, MT_BOOL aorv,MT_U32 type, MT_U32 pid);
mt_s32 MT_PVR_SetAdecAttr(mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly);
MT_S32 PVRPlaySendAframe_dealframe(PVR_PLAY_CHN_S  *pChnAttr,  PVR_INDEX_ENTRY_S *ipframe);
PVR_PLAY_CHN_S* PVRPlayGetChnAttrByName(const MT_CHAR *pFileName);
MT_S32 PVRPlaySendAframe(PVR_PLAY_CHN_S  *pChnAttr,  PVR_INDEX_ENTRY_S *pframe);
MT_S32 PVRPlaySetProc(MT_PROC_SHOW_BUFFER_S * pstBuf, MT_U32 u32Argc, MT_U8 *pu8Argv[], MT_VOID *pPrivData);

static PVR_THREAD_ATTR g_playthreadattr={0};
extern pthread_mutex_t g_pvrcrypto;
#define PVR_GET_STATE_BY_SPEED(state, speed) \
do {\
    switch (speed)\
    {\
        case MT_UNF_PVR_PLAY_SPEED_NORMAL           :\
            state = MT_UNF_PVR_PLAY_STATE_PLAY;\
            break;\
        case MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD  :\
        case MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD  :\
        case MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD  :\
        case MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD :\
        case MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD :\
        case MT_UNF_PVR_PLAY_SPEED_64X_FAST_FORWARD :\
            state = MT_UNF_PVR_PLAY_STATE_FF;\
            break;\
        case MT_UNF_PVR_PLAY_SPEED_1X_FAST_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_2X_FAST_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_4X_FAST_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_16X_FAST_BACKWARD:\
        case MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD:\
        case MT_UNF_PVR_PLAY_SPEED_64X_FAST_BACKWARD:\
            state = MT_UNF_PVR_PLAY_STATE_FB;\
            break;\
        case MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD  :\
        case MT_UNF_PVR_PLAY_SPEED_4X_SLOW_FORWARD:\
            state = MT_UNF_PVR_PLAY_STATE_SF;\
            break;\
        case MT_UNF_PVR_PLAY_SPEED_2X_SLOW_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_4X_SLOW_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_8X_SLOW_BACKWARD :\
        case MT_UNF_PVR_PLAY_SPEED_16X_SLOW_BACKWARD:\
        case MT_UNF_PVR_PLAY_SPEED_32X_SLOW_BACKWARD:\
        case MT_UNF_PVR_PLAY_SPEED_64X_SLOW_BACKWARD:\
        case MT_UNF_PVR_PLAY_SPEED_43_SLOW_BACKWARD :\
            state = MT_UNF_PVR_PLAY_STATE_FB;\
            break;\
        default:\
            state = MT_UNF_PVR_PLAY_STATE_INVALID;\
            break;\
    }\
}while(0)

/*play direction whether it is forward or not: current or before pause the state is play, fast forward, slow forward or step forward */
#define PVR_IS_PLAY_FORWARD(StateNow, StateLast)\
         (  (MT_UNF_PVR_PLAY_STATE_PLAY == StateNow) \
         || (MT_UNF_PVR_PLAY_STATE_FF == StateNow) \
         || (MT_UNF_PVR_PLAY_STATE_SF == StateNow) \
         || (MT_UNF_PVR_PLAY_STATE_STEPF == StateNow)\
         || ((MT_UNF_PVR_PLAY_STATE_PAUSE == StateNow) \
             && ( (MT_UNF_PVR_PLAY_STATE_PLAY == StateLast) \
               || (MT_UNF_PVR_PLAY_STATE_FF == StateLast) \
               || (MT_UNF_PVR_PLAY_STATE_SF == StateLast) \
               || (MT_UNF_PVR_PLAY_STATE_STEPF == StateLast) \
          )))

/*play direction whether it is backward or not: current or before puase the state is fast rewind or step rewind */
#define PVR_IS_PLAY_BACKWARD(StateNow, StateLast)\
         (  (MT_UNF_PVR_PLAY_STATE_FB == StateNow) \
         || (MT_UNF_PVR_PLAY_STATE_STEPB == StateNow)\
         || ((MT_UNF_PVR_PLAY_STATE_PAUSE == StateNow) \
             && ( (MT_UNF_PVR_PLAY_STATE_FB == StateLast) \
               || (MT_UNF_PVR_PLAY_STATE_STEPB == StateLast) \
          )))

#define PVR_IS_PLAY_INVALID(enState)\
        ((enState == MT_UNF_PVR_PLAY_STATE_INVALID)\
        ||(enState == MT_UNF_PVR_PLAY_STATE_STOP)\
        ||(enState == MT_UNF_PVR_PLAY_STATE_BUTT)\
        )


static int pvr_record_current_frame(PVR_PLAY_CHN_S *pChnAttr)
{
    PVR_INDEX_ENTRY_S stReadFrame={0};
    int ret=0;
    ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stReadFrame, pChnAttr->IndexHandle->u32ReadFrame);
    if(0==ret){
        pChnAttr->u32CurPlayTimeMs = stReadFrame.u32DisplayTimeMs;
        pChnAttr->current_playing_frame = pChnAttr->IndexHandle->u32ReadFrame;
        pChnAttr->u64CurPlayPosition = stReadFrame.u64GlobalOffset;
    }
	return ret;
}


//#define BACK_WARD_FRAME_CNT 10
static int pvr_modify_current_frame(PVR_PLAY_CHN_S *pChnAttr)
{
	if(pChnAttr->IndexHandle->u32ReadFrame > MAX_CACHE_FRAME_SIZE)
	{
	    pChnAttr->IndexHandle->u32ReadFrame = pChnAttr->IndexHandle->u32ReadFrame - MAX_CACHE_FRAME_SIZE;
	}
	else
	{
		pChnAttr->IndexHandle->u32ReadFrame = 0;
	}
    pvr_record_current_frame(pChnAttr);
    return 0;
    //MT_ERR_PVR("pChnAttr->u32CurPlayTimeMs = %d current_playing_frame %x,u64CurPlayPosition=0x%llx\n",pChnAttr->u32CurPlayTimeMs,pChnAttr->current_playing_frame,pChnAttr->u64CurPlayPosition);
}
/*
u32 get_debug_info(void)
{
    return need_debug_info;
}

u32 get_debug_info_2(void)
{
    return need_debug_info_2;
}
*/

#ifdef PVR_DEBUG_PLAY_TIME_STEP_1
static MT_S32 dump_frame_cache(PVR_PLAY_CHN_S *play,MT_U32 len)
{
    PVR_PlAY_FRAMES *tmpcache=play->framecache;
    MT_U32 pos =0;
    MT_U32 cnt =0;
    MT_U32 total = len;
    MT_U32 local_findnum=0;

    if(len > play->framesz || 0 == len)
        total = play->framesz;


    pos=((play->framewp+play->framesz-1)%play->framesz);
    while(1){
        local_findnum++;
        if(cnt >= total || local_findnum >play->framesz)
        {
            break;
        }
        if(0==tmpcache[pos].size){  //head
            if(0==pos){
                pos=play->framesz-1;
            }else{
                pos--;
            }
            continue;
        }
        MT_ERR_PVR("pos = %d frame id %d, dtps %d, tpts %d\n",pos,tmpcache[pos].frameid,tmpcache[pos].dpts,tmpcache[pos].tpts);
        if(0==pos){
            pos=play->framesz-1;
        }else{
            pos--;
        }
        cnt ++;

    }
}
#endif

static MT_U32 frame_gettimelatesttime(PVR_PLAY_CHN_S *play, MT_U32 *dts_buf, MT_U32 len)
{
	MT_U32 i = 0;
	MT_U32 frame_len = len;
	MT_U32 max_frame_len = MAX_CACHE_FRAME_SIZE;
	MT_U32 pos = 0;
	PVR_PlAY_FRAMES *tmpcache = play->framecache;

	if ((!dts_buf) || (len > PVR_IDX_CACHED_FRMNUM) || (0 == len))
	{
		MT_ERR_PVR("[%s %d], failed, len = %d, max = %d\n",
			__func__, __LINE__,
			len, PVR_IDX_CACHED_FRMNUM);

		return MT_FAILURE;
	}
        
	if (frame_len > max_frame_len)
	{
		frame_len = max_frame_len;
	}

	for (i = 0; i < frame_len; i++)
	{
		pos = ((play->framewp + play->framesz - (1 + i)) % play->framesz);
		dts_buf[i] = tmpcache[pos].dpts;
	}

	return MT_SUCCESS;
}

static void frame_pushque(PVR_PLAY_CHN_S *play, MT_U32 id,MT_U32 size,MT_U32 pts,MT_U32 dpts,MT_U64 offset)
{
    PVR_PlAY_FRAMES *tmp=play->framecache;
    MT_U32 nowpos=play->framewp;
    tmp[nowpos].frameid=id;
    tmp[nowpos].tpts=pts;
    tmp[nowpos].dpts=dpts;
    tmp[nowpos].size=size;
    tmp[nowpos].offset=offset;
    PVRPLAY_LOG_PUSH_QUEUE("speed %d; push:%d,nowpos=%d pts=0x%x,dpts=%d size =0x%x,0ff =0x%llx\n",play->enSpeed,id,nowpos,pts,dpts,size,offset);
    play->framewp=((nowpos+1)%play->framesz);
}

static MT_S32 frame_gettimebysize(PVR_PLAY_CHN_S *play,MT_U32 size,MT_U32 *framid,MT_U32 *pts,MT_U32 flag_order,MT_U32 *findnum)
{//flag_oder=0(get directly).=1,Positive and =2 negative
    MT_U32 pos=((play->framewp+play->framesz-1)%play->framesz);
    MT_U32 totalsz=0;
    MT_U32 orderpos=pos;
    MT_U32 local_findnum=0;
    MT_S32 retval=0;
    PVR_PlAY_FRAMES *tmpcache=play->framecache;

    while(totalsz < size){
        if(0==tmpcache[pos].size){  //head
            break;
        }
        totalsz+=tmpcache[pos].size;
        if(0==pos){
            pos=play->framesz-1;
        }else{
            pos--;
        }
        if(1==flag_order){
            if(tmpcache[pos].dpts < tmpcache[orderpos].dpts){
                orderpos=pos;
            }
        }else if(2==flag_order){
            if(tmpcache[pos].dpts > tmpcache[orderpos].dpts){
                orderpos=pos;
            }
        }else{
            orderpos=pos;
        }
        local_findnum++;
        if(play->framesz==local_findnum){
            retval=-1;
            break;
        }
    }
    if(framid){
        *framid=tmpcache[orderpos].frameid;
    }
    if(pts){
        *pts=tmpcache[orderpos].dpts;
    }
    if(findnum){
        *findnum=local_findnum;
    }
//    MT_ERR_PVR("frame id %x, dtps %d, tpts %d flg=%d,find pos %d= count:%d\n",tmpcache[orderpos].frameid,tmpcache[orderpos].dpts,tmpcache[orderpos].tpts,flag_order,orderpos,local_findnum);
    if(-1==retval){
        return MT_FAILURE;
    }else{
        return MT_SUCCESS;
    }
}


static MT_S32 frame_gettimebytpts(PVR_PLAY_CHN_S *play,MT_U32 tpts,MT_U32 *framid,MT_U32 *pts,MT_U32 flag_order,MT_U32 *findnum,MT_U64 *offst)
{//flag_oder=0(get directly).=1,Positive and =2 negative
    MT_U32 pos=((play->framewp+play->framesz-1)%play->framesz);
    MT_U32 orderpos=pos;
    MT_U32 local_findnum=0;
    MT_S32 retval=0;
    MT_BOOL find = MT_FALSE;
    PVR_PlAY_FRAMES *tmpcache=play->framecache;
    int i=0;
    MT_U32 find_tolerance = 0;
    
    PVRPLAY_LOG_PLAY_TIME("find0 tpts=0x%x pos=%d framesz=%d\n",tpts,pos,play->framesz);
    while(1){
        if(0==tmpcache[pos].size){  //head
            PVRPLAY_LOG_PLAY_TIME("size is 0,pos=%d ,frameid=%d tpts=0x%x dpts=%d offset=%llu\n",pos,tmpcache[pos].frameid,tmpcache[pos].tpts,tmpcache[pos].dpts,tmpcache[pos].offset);
            break;
        }

        if(tpts==tmpcache[pos].tpts){
            /* patch for stream rewind,same PTS in cache case, data have not send over,should not match final 5 frame*/
            if(local_findnum <5 && play->data_send_over == MT_FALSE){
                PVRPLAY_LOG_PLAY_TIME("find1 tpts=0x%x local_findnum=%d pos=%d abnormal----------\n",tpts,local_findnum,pos);
            }else{
                orderpos=pos; 
                find = MT_TRUE;
                PVRPLAY_LOG_PLAY_TIME("find1 tpts=0x%x pos=%d dpts=%d\n",tpts,pos,tmpcache[orderpos].dpts);
                break;        
            }
        }
        

        if(0==pos){
            pos=play->framesz-1;
        }else{
            pos--;
        }
        local_findnum++;
        if(play->framesz==local_findnum){
            retval=-1;
            break;
        }
    }
    PVRPLAY_LOG_PLAY_TIME("retval=%d local_findnum=%d\n",retval,local_findnum);

    if(-1==retval || find == MT_FALSE )
    {
        PVRPLAY_LOG_PLAY_TIME("+++ retval=%d find=%d\n",retval,find);

        for(i=0;i<MT_PVR_FIND_PTS_COUNT;i++){
            find_tolerance = MT_PVR_FIND_PTS_TOLERANCE+ i*MT_PVR_FIND_PTS_STEP;//find on tolerance 200, 600, 10000
            find = MT_FALSE;
            pos=((play->framewp+play->framesz-1)%play->framesz);
            retval = 0;
            local_findnum = 0;
            //find_abnormal = MT_FALSE;
            orderpos=pos;
            while(1){
                if(0==tmpcache[pos].size){  //head
                    PVRPLAY_LOG_PLAY_TIME("tmpcache[%d].size is 0, break!!!!\n",pos);
                    break;
                }
                #if 1
                //ff/fb check pts more accurately between tolerance
                if((tpts >= (tmpcache[pos].tpts - find_tolerance)) && (tpts <= (tmpcache[pos].tpts + find_tolerance))){


                    if(local_findnum <5 && play->data_send_over == MT_FALSE){
                        PVRPLAY_LOG_PLAY_TIME("find%d local_findnum=%d pos=%d abnormal----------\n",(2+i),local_findnum,pos);

                    }else{
                        orderpos=pos;
                        PVRPLAY_LOG_PLAY_TIME("find%d tpts=0x%x cache tpts=0x%x pos=%d dpts=%d tolerance=%d\n",
                            (2+i),tpts,tmpcache[pos].tpts,pos,tmpcache[orderpos].dpts,find_tolerance);
                        find = MT_TRUE;
                        break;
                    }

                }
                #else
                if(1==flag_order){//ff
                    if(tpts>=tmpcache[pos].tpts){
                        orderpos=pos;
                        break;
                    }
                }else if(2==flag_order){//fb
                    if(tpts<=tmpcache[pos].tpts){
                        orderpos=pos;
                        break;
                    }
                }
                #endif
                if(0==pos){
                    pos=play->framesz-1;
                }else{
                    pos--;
                }
                local_findnum++;
                if(play->framesz==local_findnum){
                    retval=-1;
                    break;
                }
            }
            if(find == MT_TRUE){
                break;
            }
            
        }

    }

    if(framid){
        *framid=tmpcache[orderpos].frameid;
    }
    if(pts){
        *pts=tmpcache[orderpos].dpts;
    }
    if(offst){
        *offst=tmpcache[orderpos].offset;
    }
    if(findnum){
        *findnum=local_findnum;
    }
	// the flowing comment code for debug
	#ifdef PVR_DEBUG_ON_RUNTIME
    if(debug_get_displaytime_by_cache)
	{
        MT_ERR_PVR("+++ retval=%d: tmpcache[%d], frame:%d,dpts =%d tpts = %ld VS tpts=%ld,count = %d, u32CurPlayTimeMs = %x\n",
                retval,orderpos,tmpcache[orderpos].frameid,tmpcache[orderpos].dpts,tmpcache[pos].tpts,tpts,local_findnum,play->u32CurPlayTimeMs);
	}
    #endif

    if(-1==retval){
        #ifdef PVR_DEBUG_PLAY_TIME_STEP_1
        dump_frame_cache(play,150);
        #endif
        MT_ERR_PVR("+++ retval=%d error\n");
        return MT_FAILURE;
    }else{
        return MT_SUCCESS;
    }
}
static MT_S32 Frame_GetDispTimeByCache(PVR_PLAY_CHN_S *play,MT_U32 *framid,MT_U32 *pts,MT_U32 *findnum,MT_U64 *offst)
{
    MT_UNF_AVPLAY_STATUS_INFO_S playinfo;
    MT_U32 flag_direct=1;
    if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == play->IndexHandle->enIndexType){
      if(MT_SUCCESS==MT_UNF_AVPLAY_GetAudioStatusInfo(play->hAvplay, &playinfo)){
        if(MT_SUCCESS==frame_gettimebysize(play,playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize,framid,pts,flag_direct,findnum)){
            //printf("+++pvr.status=%x,%x,%x\n",playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize,stCurPlayFrame,pStatus->u32CurPlayTimeInMs);
            return MT_SUCCESS;
        }else{
            return MT_FAILURE;
        }
      }else{
          return MT_FAILURE;
      }
    }else{
      if(MT_SUCCESS==MT_UNF_AVPLAY_GetSyncStatusInfo(play->hAvplay, &playinfo)){
        if(play->enSpeed < 0)
		{
            flag_direct = 2;
        }
		else
		{
			if (MT_UNF_PVR_PLAY_STATE_PAUSE == play->enState)
		  	{
				if ((MT_UNF_PVR_PLAY_STATE_FB == play->enLastState)
					|| (MT_UNF_PVR_PLAY_STATE_STEPB == play->enLastState))
				{
					flag_direct = 2;
				}
		  	}
		}
        if(0==(MT_U32)(playinfo.stSyncStatus.u64LastVidPts/1000)){
            PVRPLAY_LOG_DEBUG_FRAME("cc playinfo.stSyncStatus.u64LastVidPts %lld\n",playinfo.stSyncStatus.u64LastVidPts);
            return MT_FAILURE;
        }
#ifdef PVR_DEBUG_ON_RUNTIME
        if(debug_get_displaytime_by_cache)
            MT_ERR_PVR("playinfo.stSyncStatus.u64LastVidPts %lld\n",playinfo.stSyncStatus.u64LastVidPts/1000);
#endif

        if(MT_SUCCESS==frame_gettimebytpts(play,(MT_U32)(playinfo.stSyncStatus.u64LastVidPts/1000),framid,pts,flag_direct,findnum,offst)){
            return MT_SUCCESS;
        }else{
            #ifdef PVR_DEBUG_PLAY_TIME_STEP_1
                MT_ERR_PVR("++getinfo1=%x,%x\n",(MT_U32)(playinfo.stSyncStatus.u64LastVidPts/1000),*pts);
            #endif
            return MT_FAILURE;
        }
      }else{
//            MT_ERR_PVR("ee\n");
            return MT_FAILURE;
      }
  }
}

static void pvrplay_clearvframe(PVR_PLAY_CHN_S *play)
{
    if(play){
        play->vframe_start=0xffffffffffffffffULL;
        play->vframe_size=0;
        memset(play->framecache,0x00,sizeof(PVR_PlAY_FRAMES)*play->framesz);
        play->play_status_disptime=0;
        play->vframe_reseted=1; //reset vir-frame
        PVRPLAY_LOG_PLAY_TIME("-----------vframe_reseted framewp=%d\n",play->framewp);
    }
}

static void PVRPlayCloseDumpFile(void)
{
    if (NULL != g_pvrfpSend){
        fclose(g_pvrfpSend);
        g_pvrfpSend = NULL;
        PVR_PLAY_P("pvr play dump ts file close!\n");
    }
}

static void PVRPlayCalcTime(union sigval unSig)
{
    #if 0
    MT_U32 i = 0;
    MT_S32 s32SeedRatio = 0;
    PVR_PLAY_CHN_S *pChnAttr;
    union sigval tmp=unSig;   //fix warning
    unSig=tmp;

    for(i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        pChnAttr = &g_stPvrPlayChns[i];

        if ((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState) ||
            (MT_UNF_PVR_PLAY_STATE_STEPF == pChnAttr->enState) ||
            (pChnAttr->bEndOfFile) ||
            (pChnAttr->bTsBufReset == MT_TRUE) ||
            (MT_TRUE==pChnAttr->bQuickUpdateStatus))
        {
            MT_USLEEP(1000);
            continue;
        }

        if (!(MT_UNF_PVR_PLAY_STATE_STOP == pChnAttr->enState
          || MT_UNF_PVR_PLAY_STATE_INVALID == pChnAttr->enState))
        {
            /* normal or fast mode */
            if (abs(pChnAttr->enSpeed) >= MT_UNF_PVR_PLAY_SPEED_NORMAL)
            {
                s32SeedRatio = pChnAttr->enSpeed/MT_UNF_PVR_PLAY_SPEED_NORMAL;
                pChnAttr->u32CurPlayTimeMs += (MT_U32)(s32SeedRatio*(PVR_TIME_CTRL_TIMEBASE_NS))/1000000;
            }
            else/* slow mode */
            {
                s32SeedRatio = (MT_S32)MT_UNF_PVR_PLAY_SPEED_NORMAL/pChnAttr->enSpeed;
                pChnAttr->u32CurPlayTimeMs += (MT_U32)((PVR_TIME_CTRL_TIMEBASE_NS)/s32SeedRatio)/1000000;
            }

            if ((pChnAttr->u32CurPlayTimeMs > pChnAttr->IndexHandle->stCurPlayFrame.u32DisplayTimeMs) &&
                (pChnAttr->enSpeed > 0))
            {
                pChnAttr->u32CurPlayTimeMs = pChnAttr->IndexHandle->stCurPlayFrame.u32DisplayTimeMs;
            }
        }

        if((MT_S32)pChnAttr->u32CurPlayTimeMs < 0)
        {
            pChnAttr->u32CurPlayTimeMs = 0;
        }
    }
    #else
    union sigval tmp=unSig;   //fix warning
    unSig=tmp;
    #endif
}

#ifdef PVR_PROC_SUPPORT
static MT_S32 PVRPlayShowProc(MT_PROC_SHOW_BUFFER_S * pstBuf, MT_VOID *pPrivData)
{
    MT_U32 i=0;
    MT_U32 u32VidType=0;
    PVR_PLAY_CHN_S *pChnAttr = g_stPvrPlayChns;
    MT_S8 pStreamType[][32] = {"MPEG2", "MPEG4 DIVX4 DIVX5", "AVS", "H263", "H264",
                             "REAL8", "REAL9", "VC-1", "VP6", "VP6F", "VP6A", "MJPEG",
                             "SORENSON SPARK", "DIVX3", "RAW", "JPEG", "VP8", "MSMPEG4V1",
                             "MSMPEG4V2", "MSVIDEO1", "WMV1", "WMV2", "RV10", "RV20",
                             "SVQ1", "SVQ3", "H261", "VP3", "VP5", "CINEPAK", "INDEO2",
                             "INDEO3", "INDEO4", "INDEO5", "MJPEGB", "MVC", "HEVC", "DV", "INVALID"};
    MT_S8 pPlayStats[][16] = { "INVALID", "INIT", "PLAY", "PAUSE", "FF", "FB", "SF", "STEPF",
                             "STEPB", "STOP", "BUTT"};
    if((NULL==pstBuf)||((NULL==pstBuf->pu8Buf))){
        MT_INFO_PVR("show proc.pvr.ply err\n");
        return MT_SUCCESS;
    }else{
        MT_INFO_PVR("proc.pvr.ply size=%llx\n",pstBuf->size);
    }
    pstBuf->pu8Buf[0]=0;
    snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\n---------Montage PVR Playing channel Info---------\n");

    for(i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        if ((pChnAttr[i].enState != MT_UNF_PVR_PLAY_STATE_INVALID) &&
            (pChnAttr[i].enState != MT_UNF_PVR_PLAY_STATE_STOP) &&
            (pChnAttr[i].enState != MT_UNF_PVR_PLAY_STATE_BUTT))
        {
            u32VidType = (MT_U32)(PVR_Index_GetVtype(pChnAttr[i].IndexHandle)-100);
            u32VidType = (u32VidType > MT_UNF_VCODEC_TYPE_BUTT) ? MT_UNF_VCODEC_TYPE_BUTT : u32VidType;

            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"chan %d infomation\n", i);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tPlay filename     \t:%s\n", pChnAttr[i].stUserCfg.szFileName);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tStram type        \t:%s\n", pStreamType[u32VidType]);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tDemuxID           \t:%d\n", pChnAttr[i].u32chnID);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tTsBuffer handle   \t:%#lx\n", pChnAttr[i].hTsBuffer);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tAvplay handle     \t:%#lx\n", pChnAttr[i].hAvplay);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tCipher handle     \t:%#lx\n", pChnAttr[i].hCipher);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tPlay State        \t:%s\n", pPlayStats[pChnAttr[i].enState]);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tPlay Speed        \t:%d\n", pChnAttr[i].enSpeed);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tStream Read Pos   \t:%#llx\n", pChnAttr[i].u64CurReadPos);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIndex read       \t:%d\n",  pChnAttr[i].IndexHandle->u32ReadFrame);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIndex Start       \t:%d\n", pChnAttr[i].IndexHandle->stCycMgr.u32StartFrame);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIndex End         \t:%d\n", pChnAttr[i].IndexHandle->stCycMgr.u32EndFrame);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIndex Last        \t:%d\n", pChnAttr[i].IndexHandle->stCycMgr.u32LastFrame);
            //snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tIDR flag          \t:%d\n", pChnAttr[i].stVdecCtrlInfo.u32IDRFlag);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tB frame ref flag  \t:%d\n", pChnAttr[i].stVdecCtrlInfo.u32BFrmRefFlag);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tContinuous flag   \t:%d\n", pChnAttr[i].stVdecCtrlInfo.u32ContinuousFlag);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tDispOptimize flag \t:%d\n", pChnAttr[i].stVdecCtrlInfo.u32DispOptimizeFlag);
            /*
            snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tStart Frm&GOP num \t:%d %d\n", (pChnAttr[i].u32GopNumOfStart)>>16, (pChnAttr[i].u32GopNumOfStart)&0xffff);
            snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal GOP num     \t:%d\n", pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopTotalNum);
            snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tMax GOP size      \t:%d\n", pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32MaxGopSize);
            snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tAverage GOP size  \t:%d\n",
                           (pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32FrameTotalNum)/(pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopTotalNum));
            snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tIndex GOP distr   \t:%d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[0],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[1],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[2],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[3],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[4],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[5],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[6],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[7],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[8],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[9],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[10],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[11],
                           pChnAttr[i].IndexHandle->stRecIdxInfo.stIdxInfo.u32GopSizeInfo[12]);*/
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tFrameRate    \t:%d\n", pChnAttr[i].u32FrameRate);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIFrame num    \t:%d\n", pChnAttr[i].u32ICnt);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIpFrame num    \t:%d\n", pChnAttr[i].u32IPCnt);
            snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tIndex Read Now    \t:%d\n", pChnAttr[i].IndexHandle->u32ReadFrame);
            if(pChnAttr[i].IndexHandle){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tmagic    \t:%x,%x,%x,%x,%x\n",
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
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf), pstBuf->size-strlen((char*)pstBuf->pu8Buf),"\tmagic    \t:%x,%x\n", pChnAttr[i].u32magic1,pChnAttr[i].u32magic2);
            }
            if(pChnAttr[i].u32magic1!=0xaa55aa55){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tplylock1 line   :%d\n", pChnAttr[i].line_magc1);
            }
            if(pChnAttr[i].u32magic2!=0xaa55aa55){
                snprintf((char*)pstBuf->pu8Buf+strlen((char*)pstBuf->pu8Buf),pstBuf->size-strlen((char*)pstBuf->pu8Buf), "\tplylock2 line   :%d\n", pChnAttr[i].line_magc2);
            }
/*
            if ((MT_UNF_PVR_PLAY_STATE_FF == pChnAttr[i].enState) ||
                (MT_UNF_PVR_PLAY_STATE_FB == pChnAttr[i].enState))
            {
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\n");
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\t------ Trick play para ------\n");

                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tChip ID                   :%d\n", pChnAttr[i].stPlayProcInfo.u32ChipId);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tChip Ver                  :%x\n", pChnAttr[i].stPlayProcInfo.u32ChipVer);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tWidth                     :%d\n", pChnAttr[i].stPlayProcInfo.u32Width);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tHeigth                    :%d\n", pChnAttr[i].stPlayProcInfo.u32Heigth);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tFrame buffer              :%d\n", pChnAttr[i].u32FrmNum);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tDecodec ablity            :%d\n", pChnAttr[i].stPlayProcInfo.u32DecAblity);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tOri frame rate            :%d\n", pChnAttr[i].stPlayProcInfo.u32OrigFrmRate);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tField flag                :%d\n", pChnAttr[i].stPlayProcInfo.u32FieldFlg);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tSetup frame rate int      :%d\n", pChnAttr[i].stPlayProcInfo.u32SetFrmRateInt);
                snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tSetup frame rate dec      :%d\n", pChnAttr[i].stPlayProcInfo.u32SetFrmRateDec);
                if (MT_UNF_PVR_PLAY_STATE_FB == pChnAttr[i].enState)
                {
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\t-------- FB control --------\n");
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tOptimize flag             :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32OptimizeFlg);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tDisplay distance          :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32DispDistance);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tSupported max gop size    :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32SupportMaxGopSize);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tFirst frame num           :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32FirstFrm);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal frame num           :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32TotalFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal GOP num             :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32TotalGopNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal P frame num         :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32TotalPFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal B frame num         :%d\n", pChnAttr[i].stPlayProcInfo.stFBCtrlParameter.u32TotalBFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\t-----------------------------\n");
                }
                else
                {
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\t-------- FF control --------\n");
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTime control next frame   :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TimeCtrlFindFrm);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTime control cur frame    :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TimeCtrlCurFrm);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tFirst frame num           :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32FirstFrm);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTry frame num             :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TryFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal frame num           :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TotalFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal I frame num         :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TotalIFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal P frame num         :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TotalPFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tTotal B frame num         :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32TotalBFrmNum);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tNext I frame              :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32NextIFrm);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\tNext time start frame     :%d\n", pChnAttr[i].stPlayProcInfo.stFFCtrlParameter.u32NextTimeStartFrm);
                    snprintf(pstBuf->pu8Buf+strlen(pstBuf->pu8Buf), pstBuf->u32Size-strlen(pstBuf->pu8Buf),"\t-----------------------------\n");
                }
            }
*/
        }
    }

    return MT_SUCCESS;
}

static void PVRPlayProcHelp(void)
{
    printf("\nUsage: echo [command] [args] > /proc/msp/pvr_play\n");
    printf("Command:\n");
    printf("  dump_ts_start: dump the pvr play ts data to file before send to demux\n");
    printf("  dump_ts_stop: stop pvr play dump ts\n");
    printf("  dump_trick_ts_open: open dump the pvr trick play ts data every trick operate\n");
    printf("  dump_trick_ts_close: close pvr trick play dump ts\n");
    printf("  loglevel: set pvr play debug log\n");

    printf("Examples:\n");
    printf("  1. dump pvr play ts data:\n");
    printf("     dump file without input file name,use default 'play file name'+'_dump.ts':\n");
    printf("     echo dump_ts_start > /proc/msp/pvr_play\n");
    printf("     dump file with input file name:\n");
    printf("     echo dump_ts_start /media/sda1/dump.ts > /proc/msp/pvr_play\n");
    printf("     echo dump_ts_stop > /proc/msp/pvr_play\n");
    printf("  2. dump pvr trick play ts data:\n");
    printf("     echo dump_trick_ts_open > /proc/msp/pvr_play\n");
    printf("     echo dump_trick_ts_close > /proc/msp/pvr_play\n");
    printf("  3. set pvr play debug log :\n");
    printf("     echo loglevel 0xx > /proc/msp/pvr_play\n");
    printf("     loglevel define:\n");
    printf("     PVRPLAY_DEBUG_PLAY_OVER_INFO    (0x1)\n");
    printf("     PVRPLAY_DEBUG_FRAME_INFO        (0x2)\n");
    printf("     PVRPLAY_DEBUG_PUSH_QUEUE_INFO   (0x4)\n");
    printf("     PVRPLAY_DEBUG_PLAY_TIME_INFO    (0x8)\n");
    printf("     PVRPLAY_DEBUG_SEND_FRAME_INFO   (0x10)\n");
    printf("     PVRPLAY_DEBUG_TRICK_POLICY_INFO (0x100)\n");
    printf("     example: echo loglevel 0x11 > /proc/msp/pvr_play, play over and send frame debug info will be printed out\n");

    
}


MT_S32 PVRPlaySetProc(MT_PROC_SHOW_BUFFER_S * pstBuf, MT_U32 u32Argc, MT_U8 *pu8Argv[], MT_VOID *pPrivData)//(mt_proc_show_buffer_s * pstBuf, mt_u32 u32Argc, mt_u8 *pu8Argv[], mt_void *pPrivData)//
{
    MT_U32 u32ChnNum = 0;
    //MT_U32 u32PrintFlg = 0;
    PVR_PLAY_CHN_S *pChnAttr = g_stPvrPlayChns;
    int i = 0;
    MT_CHAR dump_name[2*PVR_MAX_FILENAME_LEN]={0};
    //MT_CHAR file[2*PVR_MAX_FILENAME_LEN]={0};
    MT_CHAR *p_cmd = NULL;
    MT_CHAR *p_cmd_help = "help";
    MT_CHAR *p_cmd_dump_ts_start = "dump_ts_start";
    MT_CHAR *p_cmd_dump_ts_stop = "dump_ts_stop";

    MT_CHAR *p_cmd_dump_trick_ts_open = "dump_trick_ts_open";
    MT_CHAR *p_cmd_dump_trick_ts_close = "dump_trick_ts_close";
    
    MT_CHAR *p_cmd_loglevel = "loglevel";
    MT_U32 loglevel = 0;

    PVR_PLAY_D("u32Argc=%d\n",u32Argc);
    if(u32Argc == 0){
        MT_ERR_PVR("echo pvr_play argc  is 0 incorrect.\n");
        return MT_FAILURE;
    }
    
    for(i=0;i<u32Argc;i++){
        PVR_PLAY_D("pu8Argv[%d]=%s\n",i,(MT_CHAR *)pu8Argv[i]);
    }

    #if 0
    if (2 != u32Argc)
    {
        MT_ERR_PVR("echo pvr_play argc is incorrect.\n");
        return MT_FAILURE;
    }

    u32ChnNum = strtoul((MT_CHAR *)pu8Argv[0], MT_NULL, 10);

    if (u32ChnNum >= PVR_PLAY_MAX_CHN_NUM)
    {
        MT_ERR_PVR("invalid channel number %d\n.", u32ChnNum);
        return MT_FAILURE;
    }
    #endif
    p_cmd = (MT_CHAR *)pu8Argv[0];
    if(strstr(p_cmd,p_cmd_help)){
        PVRPlayProcHelp();
    }else if(strstr(p_cmd,p_cmd_dump_ts_start)){
        
        if(PVR_IS_PLAY_INVALID(pChnAttr[u32ChnNum].enState)){
            MT_ERR_PVR("pvr play have not start,channel status is invalid.\n");
            return MT_FAILURE;
        }
        memset(dump_name,0,sizeof(dump_name));
        if(u32Argc >= 2){
            PVR_PLAY_D("----input dump file name=%s\n",(MT_CHAR *)pu8Argv[1]);
            snprintf(dump_name, sizeof(dump_name), "%s", (MT_CHAR *)pu8Argv[1]);
        }else{
            snprintf(dump_name, sizeof(dump_name), "%s_dump.ts", (MT_CHAR *)&pChnAttr->stUserCfg.szFileName[0]);
        }
        
        
        PVR_PLAY_D("----dump_ts_start dump file=%s\n",dump_name);
        if (NULL == g_pvrfpSend){
            g_pvrfpSend = fopen(dump_name, "wb");
            if(g_pvrfpSend){
                PVR_PLAY_P("pvr play dump ts start %s ok!\n",dump_name);
                return MT_SUCCESS;
            }else{
                MT_ERR_PVR("pvr play open dump file %s failed!\n",dump_name);
                return MT_FAILURE;
            }     
        }else{
            MT_ERR_PVR("pvr play g_pvrfpSend have opened!!!!\n");
            return MT_FAILURE;
        }
    }else if(strstr(p_cmd,p_cmd_dump_ts_stop)){

        PVR_PLAY_D("----dump_ts_stop\n");
        if(PVR_IS_PLAY_INVALID(pChnAttr[u32ChnNum].enState)){
            MT_ERR_PVR("pvr play have not start,channel status is invalid.\n");
            return MT_FAILURE;
        }
    
        if (NULL != g_pvrfpSend){
            PVRPlayCloseDumpFile();
            PVR_PLAY_P("pvr play dump ts stop ok!\n");
            return MT_SUCCESS;
        }else{
            MT_ERR_PVR("pvr play  g_pvrfpSend have closed!!!!\n");
            return MT_FAILURE;
        }
    }else if(strstr(p_cmd,p_cmd_dump_trick_ts_open)){
        g_bPvrTrickDump = MT_TRUE;
        PVR_PLAY_P("pvr trick play dump ts open ok!\n");
    }else if(strstr(p_cmd,p_cmd_dump_trick_ts_close)){
        g_bPvrTrickDump = MT_FALSE;
        PVRPlayCloseDumpFile();
        PVR_PLAY_P("pvr trick play dump ts close ok!\n");
    }else if(strstr(p_cmd,p_cmd_loglevel)){
        loglevel = strtol((MT_CHAR *)pu8Argv[1], NULL, 0);
        PVR_PLAY_P("set pvr play loglevel=0x%x\n",loglevel);
        g_pvrplay_loglevel = loglevel; 
    }else{
        PVRPlayProcHelp();
    }
    

#if 0 //not ready
    if ((MT_UNF_PVR_PLAY_STATE_FF == pChnAttr[u32ChnNum].enState) ||
        (MT_UNF_PVR_PLAY_STATE_FB == pChnAttr[u32ChnNum].enState))
    {
        u32PrintFlg = strtoul((MT_CHAR *)pu8Argv[1], MT_NULL, 10);

        if(1 == u32PrintFlg)
        {
            g_stPvrPlayChns[u32ChnNum].stPlayProcInfo.u32PrintFlg = 1;
        }
        else if (0 == u32PrintFlg)
        {
            g_stPvrPlayChns[u32ChnNum].stPlayProcInfo.u32PrintFlg = 0;
        }
    }
#endif
    return MT_SUCCESS;
}

#endif
/*
static MT_S32 frame_pushwp(PVR_PLAY_CHN_S *idxhandle,  PVR_INDEX_ENTRY_S *now)
{
    MT_U32 next=((idxhandle->idxwp+1)&(idxhandle->idxmax-1));
    if(next != idxhandle->idxrp){
        idxhandle->idxcache[idxhandle->idxwp]=*now;
        idxhandle->idxwp=next;
        //printf("+1.%d,%d\n",idxhandle->idxrp,idxhandle->idxwp);
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}
static void frame_popwp(PVR_PLAY_CHN_S *idxhandle)
{
    if(idxhandle->idxwp != idxhandle->idxrp){
        idxhandle->idxwp=((idxhandle->idxwp+idxhandle->idxmax-1)&(idxhandle->idxmax-1));
        //printf("+2.%d,%d\n",idxhandle->idxrp,idxhandle->idxwp);
    }
}
static PVR_INDEX_ENTRY_S * frame_getwp(PVR_PLAY_CHN_S *idxhandle)
{
    MT_U32 last;
    if(idxhandle->idxwp == idxhandle->idxrp){
        return NULL;
    }
    last=((idxhandle->idxwp+idxhandle->idxmax-1)&(idxhandle->idxmax-1));
    //printf("+3.%d,%d\n",idxhandle->idxrp,idxhandle->idxwp);
    return &idxhandle->idxcache[last];
}
static PVR_INDEX_ENTRY_S *frame_getrp(PVR_PLAY_CHN_S *idxhandle)
{
    PVR_INDEX_ENTRY_S *tmp;
    if(idxhandle->idxwp == idxhandle->idxrp){
        return NULL;
    }
    tmp=&idxhandle->idxcache[idxhandle->idxrp];
    idxhandle->idxrp=((idxhandle->idxrp+1)&(idxhandle->idxmax-1));
    //printf("+4.%d,%d\n",idxhandle->idxrp,idxhandle->idxwp);
    return tmp;
}
static MT_U32 frame_numque(PVR_PLAY_CHN_S *idxhandle)
{
    if(idxhandle->idxwp >= idxhandle->idxrp){
        return (idxhandle->idxwp-idxhandle->idxrp);
    }else{
        return idxhandle->idxmax-(idxhandle->idxrp-idxhandle->idxwp);
    }
}
*/
static void frame_empty(PVR_PLAY_CHN_S *idxhandle)
{
    //idxhandle->idxrp=0;
    idxhandle->idxwp=0;
}
static void frame_checkbuffer(PVR_PLAY_CHN_S *pChnAttr)
{
    MT_UNF_AVPLAY_STATUS_INFO_S playinfo;
    MT_U32 errnum=0;

    for(;;){
        if (MT_SUCCESS == MT_UNF_AVPLAY_GetVideoStatusInfo(pChnAttr->hAvplay, &playinfo)){
            /*
            static int xx=0;
            if((xx%200)==0){
                printf("+++v=%x,%x.a=%x,%x\n",playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize,
                                              playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize,
                                              playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize,
                                              playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufSize);
            }
            xx++;
            */
            if((playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize*5) > (playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize*4)){
                MT_USLEEP(5000);
            }else{
                break;
            }
            errnum=0;
        }else{
            errnum++;
            MT_USLEEP(5000);
        }
        if(errnum>10){
            break;
        }
    }
}
STATIC INLINE MT_S32 PVRPlayDevInit(MT_VOID)
{
    int fd;

    if (g_s32PvrFd == -1)
    {
        fd = open (api_pathname_pvr, O_RDWR | O_CLOEXEC, 0);

        if(fd < 0)
        {
            MT_FATAL_PVR("Cannot open '%s'\n", api_pathname_pvr);
            return MT_ERR_PVR_FILE_CANT_OPEN;
        }
        g_s32PvrFd = fd;

    }

    return MT_SUCCESS;
}
/* unused.warning
STATIC INLINE MT_BOOL PVRPlayIsVoEmpty(PVR_PLAY_CHN_S  *pChnAttr)
{
    MT_S32 ret = MT_FAILURE;
    MT_HANDLE hWindow;
    MT_DRV_WIN_PLAY_INFO_S stWinDelay = {0};

    ret = MT_MPI_AVPLAY_GetWindowHandle(pChnAttr->hAvplay, &hWindow);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_MPI_AVPLAY_GetWindowHandle fail:0x%x\n", ret);
        return MT_TRUE;

    }

    ret = MT_MPI_VO_GetWindowDelay(hWindow, &stWinDelay);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_MPI_VO_GetWindowDelay fail:0x%x\n", ret);
        return MT_TRUE;
    }

    MT_INFO_PVR("WinDelay=%d\n", stWinDelay.u32DelayTime);

    if (stWinDelay.u32DelayTime <= 40) // less-equal than 40 is OK, less-equal than 1 frame
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}

STATIC INLINE MT_BOOL PVRPlayIsAoEmpty(PVR_PLAY_CHN_S  *pChnAttr)
{
    MT_S32 ret = MT_FAILURE;
    MT_HANDLE hSnd;
    MT_BOOL bEmpty = MT_FALSE;

    ret = MT_MPI_AVPLAY_GetSndHandle(pChnAttr->hAvplay, &hSnd);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_MPI_AVPLAY_GetSndHandle fail:0x%x\n", ret);
        return MT_TRUE;

    }

    ret = MT_MPI_AO_Track_IsBufEmpty(hSnd, &bEmpty);

    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_MPI_AO_Track_IsBufEmpty fail:0x%x\n", ret);
        return MT_TRUE;
    }

    return bEmpty;
}

STATIC INLINE MT_BOOL PVRPlayIsEOS(PVR_PLAY_CHN_S  *pChnAttr)
{
    MT_BOOL Eof = MT_TRUE;
    MT_S32 ret = MT_FAILURE;
    MT_UNF_AVPLAY_BUFID_E bufID;
    MT_UNF_AVPLAY_STATUS_INFO_S info;
    MT_UNF_DMX_TSBUF_STATUS_S  stTsBufStat;
    MT_U32 u32BufLowSize = 0;
    MT_U32 u32CurPts = PVR_INDEX_INVALID_PTSMS;

    // the audio index should set audio esbuffer, video index set video esbuffer
    if (PVR_INDEX_IS_TYPE_AUDIO(pChnAttr->IndexHandle))
    {
        bufID = MT_UNF_AVPLAY_BUF_ID_ES_AUD;
        u32BufLowSize = 1024;

        // audio haven't completely end the play, until to play over
        if (!PVRPlayIsAoEmpty(pChnAttr))
        {
            Eof = MT_FALSE;
            return Eof;
        }
        else
        {
            return MT_TRUE;
        }
    }
    else // video
    {
        bufID = MT_UNF_AVPLAY_BUF_ID_ES_VID;
        u32BufLowSize = 8*1024;

        // video haven't completely end the play, until to play over
        if (!PVRPlayIsVoEmpty(pChnAttr))
        {
            Eof = MT_FALSE;
            return Eof;
        }
    }

    ret = MT_UNF_DMX_GetTSBufferStatus(pChnAttr->hTsBuffer, &stTsBufStat);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_UNF_DMX_GetTSBufferStatus fail:0x%x\n", ret);
        return MT_TRUE;
    }

    ret = MT_UNF_AVPLAY_GetStatusInfo(pChnAttr->hAvplay, &info);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_UNF_AVPLAY_GetStatusInfo fail:0x%x\n", ret);
        return MT_TRUE;
    }

    if (PVR_INDEX_IS_TYPE_AUDIO(pChnAttr->IndexHandle))
    {
//        u32CurPts = info.stSyncStatus.u32LastAudPts;
        u32CurPts = info.stSyncStatus.u64LastAudPts;
    }
    else // video
    {
        u32CurPts = info.stSyncStatus.u64LastVidPts;
    }

    // pts invariable, whether size is invariable or not or es buffer size less than some byte
    if ((stTsBufStat.u32UsedSize < (PVR_TS_LEN + PVR_DMX_TS_BUFFER_GAP))
        && (pChnAttr->u32LastPtsMs == u32CurPts)
        && ((info.stBufStatus[bufID].u32UsedSize == pChnAttr->u32LastEsBufSize)
          || (info.stBufStatus[bufID].u32UsedSize < u32BufLowSize)))
    {
        Eof = MT_TRUE;
        MT_INFO_PVR("BUF EMPTY NOW. ES in buf:%u,lst:%u, PTS:%d,lst:%d\n",
                info.stBufStatus[bufID].u32UsedSize,pChnAttr->u32LastEsBufSize, u32CurPts, pChnAttr->u32LastPtsMs);
    }
    else
    {
        Eof = MT_FALSE;

        MT_INFO_PVR("ES in buf:%u, PTS:%d lst:%d\n", info.stBufStatus[bufID].u32UsedSize, u32CurPts, pChnAttr->u32LastPtsMs);
        pChnAttr->u32LastEsBufSize = info.stBufStatus[bufID].u32UsedSize;
        pChnAttr->u32LastPtsMs = u32CurPts;
    }

    return Eof;
}*/

static mt_s32 pvr_check_av_eof(PVR_PLAY_CHN_S  *pChnAttr,mt_handle hAvplay, MT_BOOL *pbIsEmpty)
{
    mt_s32 ret;
    MT_UNF_AVPLAY_STATUS_INFO_S status;
    //MT_BOOL bAeos = MT_FALSE;
    //MT_BOOL bVeos = MT_FALSE;
    static mt_u32 aused=0,vused=0,apts=0,vpts=0,samecounter=0;
    mt_u32 max_samecounter = 10;

    *pbIsEmpty=MT_FALSE;
    if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == pChnAttr->IndexHandle->enIndexType){
        max_samecounter = 15;
        if(MT_SUCCESS == MT_UNF_AVPLAY_GetAudioStatusInfo(pChnAttr->hAvplay, &status)){
        if((aused==status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize)&&
           (apts==(mt_u32)(status.stSyncStatus.u64LastAudPts/1000))){
            samecounter++;
            if(samecounter > max_samecounter){
                samecounter=0;
                PVRPLAY_LOG_PLAY_OVER("+++more than %d.true eof\n",max_samecounter);
                *pbIsEmpty=MT_TRUE;
            }
        }else{
            samecounter=0;
        }
        aused=status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize;
        apts=(mt_u32)(status.stSyncStatus.u64LastAudPts/1000);
        MT_USLEEP(100 * 1000);
        return MT_SUCCESS;
      }else{
        return MT_FAILURE;
      }
        return MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_GetVideoStatusInfo(hAvplay, &status);
    if (ret == MT_SUCCESS)
    {//2s same rp wp. it's eof!
        PVRPLAY_LOG_PLAY_OVER("----a %x,%x,%x,%x,%llx,%llx!\n",
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].bEndOfStream,
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize,
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufRptr,
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32BufWptr,
        status.stSyncStatus.u64LastVidPts,
        status.stSyncStatus.u64LastAudPts
        );
        PVRPLAY_LOG_PLAY_OVER("----v %x,%x,%x,%x,%x,%x,%x!samecounter=%d\n",
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream,
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize,
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufRptr,
        status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufWptr,
        status.enRunStatus,
        status.u32VidFrameCount,
        status.u32AuddFrameCount,
        samecounter
        );
        if((aused==status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize)&&
           (vused==status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize)&&
           (apts==(mt_u32)(status.stSyncStatus.u64LastAudPts/1000))&&
           (vpts==(mt_u32)(status.stSyncStatus.u64LastVidPts/1000))
          ){
            samecounter++;
            if(samecounter > max_samecounter){
                samecounter=0;
                PVRPLAY_LOG_PLAY_OVER("+++more than %d.true eof\n",max_samecounter);
                *pbIsEmpty=MT_TRUE;
            }
        }else{
            samecounter=0;
        }
        aused=status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize;
        vused=status.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize;
        apts=(mt_u32)(status.stSyncStatus.u64LastAudPts/1000);
        vpts=(mt_u32)(status.stSyncStatus.u64LastVidPts/1000);
        MT_USLEEP(100 * 1000);
        return MT_SUCCESS;
    }else{
        return MT_FAILURE;
    }
}
STATIC INLINE void PVRPlayPostEvent(MT_U32 u32ChnID, MT_UNF_PVR_EVENT_E enEventType, MT_S32 s32EnvetValue)
{
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply && p_addon->play_inter.on_event)
        {
            p_addon->play_inter.on_event(p_addon->handlesply,enEventType,s32EnvetValue,0);
        }
    }
    PVR_Intf_DoEventCallback(u32ChnID,enEventType,s32EnvetValue);
}

#if 0
STATIC INLINE MT_S32 PVRPlayWaitForEndOfFile(PVR_PLAY_CHN_S  *pChnAttr,  MT_U32 timeOutMs)
{
    MT_BOOL Eof = MT_TRUE;
    MT_U32 findnum = 0;
    static MT_U32 lst_findnum = 0,same_counter=0;
    /* On step mode, just need wait next command, that is it. regardless of whether it is end or not in main thread*/
    if (MT_UNF_PVR_PLAY_STATE_STEPF == pChnAttr->enState)
    {
        MT_USLEEP(200 * 1000);
        return MT_FALSE;
    }
    #if 0
    do {
        (MT_VOID)MT_MPI_AVPLAY_IsBuffEmpty(pChnAttr->hAvplay, &Eof);
        if (Eof)
        {
            break;
        }
        else
        {
            /* look up interval 200ms */
            MT_USLEEP(200 * 1000);
            u32time += 200;
            continue;
        }
    } while(u32time < timeOutMs);
    #else
    if(MT_FAILURE==Frame_GetDispTimeByCache(pChnAttr,NULL,NULL,&findnum,NULL)){
        if(MT_FAILURE==pvr_check_av_eof(pChnAttr,pChnAttr->hAvplay, &Eof)){
            MT_USLEEP(200 * 1000);
        }
    }else{
        if(lst_findnum==findnum){
            same_counter++;
        }else{
            lst_findnum=findnum;
            same_counter=0;
        }
        if(same_counter >= 10){  //display cache frame!
            Eof=MT_TRUE;
        }else{
            Eof=MT_FALSE;
        }
        MT_USLEEP(200 * 1000);
    }
    #endif
    MT_INFO_PVR("Eof=%d\n", Eof);

    return Eof;
}
#endif

STATIC INLINE PVR_PLAY_CHN_S * PVRPlayFindFreeChn(MT_VOID)
{
    PVR_PLAY_CHN_S * pChnAttr = NULL;

    /* find a free play channel */
#if 0 /*not support multi-thread */
    MT_U32 i;
    for (i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        if (g_stPvrPlayChns[i].enState == MT_UNF_PVR_PLAY_STATE_INVALID)
        {
            pChnAttr = &g_stPvrPlayChns[i];
            pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INIT;
            break;
        }
    }
#else /* manage the resources by kernel driver */
    MT_U32 ChanId;
    if (MT_SUCCESS != ioctl(g_s32PvrFd, CMD_PVR_CREATE_PLAY_CHN, (ulong)&ChanId))
    {
        MT_FATAL_PVR("pvr play creat channel error\n");
        return MT_NULL;
    }

    MT_ASSERT(g_stPvrPlayChns[ChanId].enState == MT_UNF_PVR_PLAY_STATE_INVALID);
    pChnAttr = &g_stPvrPlayChns[ChanId];

    PVR_LOCK_PLAY(pChnAttr);
    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INIT;
    pChnAttr->enLastState = MT_UNF_PVR_PLAY_STATE_INIT;
    PVR_UNLOCK_PLAY(pChnAttr);
#endif

    return pChnAttr;
}

/* unused.warning
STATIC INLINE MT_S32 PVRPlayCheckUserCfg(const MT_UNF_PVR_PLAY_ATTR_S *pUserCfg, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer)
{
    MT_S32 ret;
    MT_UNF_AVPLAY_ATTR_S         AVPlayAttr;
    MT_UNF_AVPLAY_STATUS_INFO_S  StatusInfo;
    MT_UNF_DMX_TSBUF_STATUS_S    TsBufStatus;
    MT_CHAR szIndexName[PVR_MAX_FILENAME_LEN + 4];
    MT_U32 i;

    if (MT_UNF_PVR_STREAM_TYPE_TS != pUserCfg->enStreamType )
    {
        MT_ERR_PVR("invalid play enStreamType:%d\n", pUserCfg->enStreamType );
        return MT_ERR_PVR_INVALID_PARA;
    }

    PVR_CHECK_CIPHER_CFG(&pUserCfg->stDecryptCfg);

    //  if play file name ok
    if (!((pUserCfg->u32FileNameLen > 0)
        && (strlen(pUserCfg->szFileName) == pUserCfg->u32FileNameLen)))
    {
        MT_ERR_PVR("Invalid file name!\n");
        return MT_ERR_PVR_FILE_INVALID_FNAME;
    }

    //check if stream exist!
    if (!PVR_CHECK_FILE_EXIST64(pUserCfg->szFileName))
    {
        MT_ERR_PVR("Stream file %s doesn't exist!\n", pUserCfg->szFileName);
        return MT_ERR_PVR_FILE_NOT_EXIST;
    }
    //snprintf(szIndexName, PVR_MAX_FILENAME_LEN,"%s.%s", pUserCfg->szFileName, "idx");
    PVR_Index_GetIdxFileName(szIndexName,  pUserCfg->szFileName);
    if (!PVR_CHECK_FILE_EXIST(szIndexName))
    {
        MT_ERR_PVR("can NOT find index file for '%s'!\n", pUserCfg->szFileName);
        return MT_ERR_PVR_FILE_NOT_EXIST;
    }

    for (i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        // check whether demux id is used or not
        if (MT_UNF_PVR_PLAY_STATE_INVALID != g_stPvrPlayChns[i].enState)
        {
                  // check whether the same file is playing or not
            if (0 == strncmp(g_stPvrPlayChns[i].stUserCfg.szFileName, pUserCfg->szFileName,sizeof(pUserCfg->szFileName)))
            {
                MT_ERR_PVR("file %s was exist to be playing.\n", pUserCfg->szFileName);
                return MT_ERR_PVR_FILE_EXIST;
            }

            if (g_stPvrPlayChns[i].hAvplay == hAvplay)
            {
                MT_ERR_PVR("avplay 0x%x already has been used to play.\n", hAvplay);
                return MT_ERR_PVR_ALREADY;
            }
            if (g_stPvrPlayChns[i].hTsBuffer == hTsBuffer)
            {
                MT_ERR_PVR("Ts buffer 0x%x already has been used to play.\n", hTsBuffer);
                return MT_ERR_PVR_ALREADY;
            }
        }

    }

    ret = MT_UNF_AVPLAY_GetStatusInfo(hAvplay, &StatusInfo);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("check hAvplay for PVR failed:%#x\n", ret);
        return MT_FAILURE;
    }
    if (StatusInfo.enRunStatus != MT_UNF_AVPLAY_STATUS_STOP)
    {
        MT_WARN_PVR("the hAvplay is not stopped\n");

        //lint -e655
        ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
        if (ret != MT_SUCCESS)
        {
            MT_ERR_PVR("can NOT stop hAvplay for pvr replay\n");
            return ret;
        }
        //lint +e655
    }

    ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_STREAM_MODE, &AVPlayAttr);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("check hAvplay attr failed\n");
        return MT_FAILURE;
    }
    if (AVPlayAttr.stStreamAttr.enStreamType != MT_UNF_AVPLAY_STREAM_TYPE_TS)
    {
        MT_ERR_PVR("hAvplay's enStreamType is NOT TS.\n");
        return MT_ERR_PVR_INVALID_PARA;
    }

    ret = MT_UNF_DMX_GetTSBufferStatus(hTsBuffer, &TsBufStatus);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("check hTsBuffer failed.\n");
        return MT_ERR_PVR_INVALID_PARA;
    }

    return MT_SUCCESS;
}*/

mt_s32 MT_PVR_VIDEO_Reset(mt_handle hAvplay, const MT_UNF_AVPLAY_RESET_OPT_S *pstResetOpt);
mt_s32 MT_MPI_AVPLAY_set_trickmode(mt_handle hAvplay, u32 mode);

mt_s32 MT_PVR_VIDEO_Reset(mt_handle hAvplay, const MT_UNF_AVPLAY_RESET_OPT_S *pstResetOpt)
{

	MT_UNF_AVPLAY_STOP_OPT_S pstStopOpt;
	pstStopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
	pstStopOpt.u32TimeoutMs = 0;
	(void)MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, &pstStopOpt);
	(void)MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
	return 0;
}
MT_S32 MT_Pvr_Play_Read_File(MT_U8 *pu8Addr, MT_U64 offset, MT_U32 size, PVR_PLAY_CHN_S *pChnAttr)
{
    MT_U64 u64Offset1 = 0;
    MT_U64 u64Offset2 = 0;
    MT_U32 u32Size1 = 0;
    MT_U32 u32Size2 = 0;
    MT_U8 *pTempBuf = MT_NULL;
//    struct timeval start_time, end_time;
 //   gettimeofday(&start_time,NULL);

    u64Offset1 = offset;
    //fixed Bug #25636,may read 0 when offset+size over file maxcycsize
    if(pChnAttr->IndexHandle->stCycMgr.u64MaxCycSize > 0)
    {
        if (PVR_INDEX_IS_REWIND(pChnAttr->IndexHandle))
        {
            u64Offset1 = offset % PVR_INDEX_MAX_FILE_SIZE(pChnAttr->IndexHandle);
            if (u64Offset1 + size > PVR_INDEX_MAX_FILE_SIZE(pChnAttr->IndexHandle))
            {
                u32Size1 = PVR_INDEX_MAX_FILE_SIZE(pChnAttr->IndexHandle) - u64Offset1;
                u32Size2 = size - u32Size1;
                u64Offset2 = (MT_U64)0;
                PVR_PLAY_D("---------u64MaxCycSize=%lld offset=%lld u64Offset1=%lld size=%d u32Size1=%d u32Size2=%d\n",
                    pChnAttr->IndexHandle->stCycMgr.u64MaxCycSize,
                    offset,u64Offset1,size,u32Size1,u32Size2);
            }else if(u64Offset1 !=  offset){
                PVR_PLAY_D("---------rewind read u64MaxCycSize=%lld size=%d offset=%lld to u64Offset1=%lld\n",
                    pChnAttr->IndexHandle->stCycMgr.u64MaxCycSize,size,offset,u64Offset1);
            }
        }
    }
    
    if (0 == u32Size1)
    {
        PVR_PLAY_READ_FILE(pu8Addr, u64Offset1, size, pChnAttr);
    }
    else
    {
        //read size1 first
        PVR_PLAY_READ_FILE(pu8Addr,u64Offset1,u32Size1,pChnAttr);
        //read size2
        pTempBuf = (MT_U8 *)pu8Addr + u32Size1;
        PVR_PLAY_READ_FILE(pTempBuf,u64Offset2,u32Size2,pChnAttr);
    }
    
//   gettimeofday(&end_time,NULL);
//   int time_interval = (end_time.tv_sec - start_time.tv_sec)*(1000*1000) +(end_time.tv_usec - start_time.tv_usec);
//    if(time_interval > 500*1000)
//    {
//        MT_ERR_PVR("Read.%llx,%x, cost %d us\n",offset,size,time_interval);
//    }

    return MT_SUCCESS;
}
/*
#ifdef CONFIG_MT_PVR_CIPHER_SUPPORT
static void dumpdata(char *s,mt_u8 *head,mt_u32 lens)
{
    mt_u32 i;
    MT_INFO_PVR("%s [%x]:\n",s,lens);
    for(i=0;i<lens;i++){
        MT_INFO_PVR("%02x ",head[i]);
        if(7==(i&7)){
            MT_INFO_PVR("\n");
        }
    }
    MT_INFO_PVR("\n");
}

MT_S32 MT_PVR_CipherPlyCreate(PVR_PLAY_CHN_S *pChnAttr,MT_CIPHER_CRYPTO_CH_E chid,mt_u32 enordec)
{//enordec,0:en 1:dec
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_cipher;
    MT_CIPHER_CTRL_S info;
    mt_u32 slot_id;
    int i;
    if(2==pChnAttr->chiptype){
#ifndef CONFIG_MT_CHIP_SYMPHONY1
        slot_id = MT_CIPHER_KEYSLOT_INVALID;
        mt_unf_cipher_keyslot_request(&slot_id);

        memset(&info, 0, sizeof(MT_CIPHER_CTRL_S));

        info.operation = enordec;
        info.algorithm = pChnAttr->stUserCfg.stDecryptCfg.enType;
        info.work_mode = MT_CIPHER_WORK_MODE_ECB;

        mt_unf_cipher_keyslot_set(slot_id, &info, pChnAttr->stUserCfg.stDecryptCfg.au8Key, NULL);    //clear-text key, no IV
        //printf("+p:%x,%x,%x\n",enordec,pChnAttr->stUserCfg.stDecryptCfg.enType,chid);
        //dumpdata("key:",pChnAttr->stUserCfg.stDecryptCfg.au8Key,16);

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

MT_S32 MT_PVR_CipherPlyDestroy(PVR_PLAY_CHN_S *pChnAttr)
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

MT_S32 MT_PVR_CipherPlyCrypto_sample(PVR_PLAY_CHN_S *pChnAttr,MT_U8 *in ,MT_U8 *out,MT_U32 len)
{//merge last-remain to aligned. crypto two times

    int ret=0;
    ret|=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
    ret|=mt_unf_cipher_crypto_process(pChnAttr->hCipher,in,out,len);
    //printf("+++outcphonyer=%x,%x\n",pChnAttr->hCipher,ret);
    ret|=MT_PVR_CipherPlyDestroy(pChnAttr);
    return MT_SUCCESS;
}
//#define TEST_PVR_MEM_CRYPTO
MT_S32 MT_PVR_CipherPlyCrypto(PVR_PLAY_CHN_S *pChnAttr,MT_UNF_STREAM_BUF_S *out,MT_U64 u64ReadOffset,MT_U32 u32BytesSend,MT_U8 *mem)
{
    #if 1

    //u64ReadOffset_aligned+x=u64ReadOffset;
    //u64ReadEnd_aligned=u64ReadOffset+u32BytesSend+x;
    //u32BytesSend_aligned=u64ReadEnd_aligned-u64ReadOffset_aligned;

    MT_U32 alginlen=16,round_hd,round_tl,modex=0;
    int fd;
    MT_U64 u64ReadOffset_aligned=0;
    MT_U32 u32BytesSend_aligned=0;

    MT_U64 u64ReadEnd_aligned=0;
    MT_U64 u64ReadEnd=0;

    mt_u32 flag_head=0,flag_tail=0;
    mt_u32 offset=0,celllen=0,dest_offset=0;

    MT_U8 *cryptobuff=NULL;
    MT_S32 ret=MT_SUCCESS,RET=MT_SUCCESS;
    if(NULL==out){
        return MT_FAILURE;
    }
    out->u32Size=0;
        #ifdef CONFIG_MT_LXC_SUPPORT
        fd=PVR_GetTsFileFd(pChnAttr->s32DataFile);
        if(0<=fd){
            cryptobuff=mt_pvr_ipc_getbufferby_tid(fd,MAX_CRYPTOSZ_R);
        }
        #else
        cryptobuff=pChnAttr->hCipher_buffer;
        #endif
        if(NULL==cryptobuff){
            return MT_FAILURE;
        }
        if(MT_CIPHER_ALG_AES > pChnAttr->stUserCfg.stDecryptCfg.enType){
            alginlen=8;
        }else{
            alginlen=16;
        }
        u64ReadOffset_aligned=(u64ReadOffset-(u64ReadOffset&(alginlen-1)));
        u64ReadEnd=(u64ReadOffset+u32BytesSend);
        modex=(u64ReadEnd&(alginlen-1));
        if(modex){
            u64ReadEnd_aligned=u64ReadEnd+(alginlen-modex);
        }else{
            u64ReadEnd_aligned=u64ReadEnd;
        }
        u32BytesSend_aligned=u64ReadEnd_aligned-u64ReadOffset_aligned;
        round_hd=(u64ReadOffset%alginlen);                      //1
        round_tl=((u64ReadOffset+u32BytesSend)%alginlen);       //1
        //printf("+++++p=%llx,s=%x,e=%llx;\n",u64ReadOffset,u32BytesSend,u64ReadEnd);
        //printf("+++++p=%llx,s=%x,e=%llx,%x;\n",u64ReadOffset_aligned,u32BytesSend_aligned,u64ReadEnd_aligned,modex);
        //printf("++++++++++++++++++++++++crypto start.r=%x,%x\n",cryptobuff,out->pu8Data);

        while(u32BytesSend_aligned){
            if(MAX_CRYPTOSZ_R<=u32BytesSend_aligned){
                celllen=MAX_CRYPTOSZ_R;
            }else{
                celllen=u32BytesSend_aligned;
            }
            //printf("+++read0=%x\n",celllen);
            MT_Pvr_Play_Read_File(cryptobuff, u64ReadOffset_aligned+offset, celllen, pChnAttr);
            //printf("+++read1=%x\n",celllen);
            { //crypto sharebuffer to dmxbuffer
                mt_u32 remainlen=celllen,subcopy=0,discard=0,src_offset=0;
                flag_head=0;
                flag_tail=0;
                if((u64ReadOffset_aligned+offset)<u64ReadOffset){                         //head
                    //printf("+++cypto.r0=%x\n",alginlen);
                    pthread_mutex_lock(&g_pvrcrypto);
                    ret=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
                    ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,cryptobuff,pChnAttr->hCipher_head,alginlen);
                    ret=MT_PVR_CipherPlyDestroy(pChnAttr);
                    pthread_mutex_unlock(&g_pvrcrypto);

                    dest_offset=(alginlen-round_hd);
                    src_offset=alginlen;
                    remainlen-=alginlen;
                    flag_head=1;
                }
                if(((u64ReadOffset_aligned+offset+celllen)==u64ReadEnd_aligned) &&
                    (u64ReadEnd_aligned!=u64ReadEnd)){  //tail
                    //printf("+++cypto.r2=%x\n",alginlen);
                    pthread_mutex_lock(&g_pvrcrypto);
                    ret=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
                    ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,cryptobuff+celllen-alginlen,pChnAttr->hCipher_tail,alginlen);
                    ret=MT_PVR_CipherPlyDestroy(pChnAttr);
                    pthread_mutex_unlock(&g_pvrcrypto);
                    //printf("+++cypto.r3=%x\n",alginlen);

                    remainlen-=alginlen;
                    flag_tail=1;
                }
                if(remainlen){                                                            //middle
                    //printf("+++cypto.r4=%x\n",remainlen,cryptobuff+src_offset,out->pu8Data+dest_offset,src_offset,dest_offset);
                    pthread_mutex_lock(&g_pvrcrypto);
                    ret=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
                    ret= mt_unf_cipher_crypto_process(pChnAttr->hCipher,cryptobuff+src_offset,out->pu8Data+dest_offset,remainlen);
                    ret=MT_PVR_CipherPlyDestroy(pChnAttr);
                    pthread_mutex_unlock(&g_pvrcrypto);
                    //printf("+++cypto.r5=%x\n",remainlen);
                    dest_offset+=remainlen;
                }
                if(flag_head){   //from 1 cp len=15 to head
                    //printf("++cphead.r=%x\n",(alginlen-round_hd));
                    memcpy(out->pu8Data,pChnAttr->hCipher_head+round_hd,alginlen-round_hd);
                }
                if(flag_tail){   //from 0 cp len=1 to tail
                    //printf("++cptail.r=%x\n",round_tl);
                    memcpy(out->pu8Data+u32BytesSend-round_tl,pChnAttr->hCipher_tail,round_tl);
                }
            }
            offset+=celllen;
            u32BytesSend_aligned-=celllen;
        }
        out->u32Size=u32BytesSend;
        //printf("+++++++++++++++++++++++++++crypto over.r\n");
    #else
    MT_U32 alginlen=16,dechead,declen,round_hd,round_tl;
    MT_S32 ret=MT_SUCCESS,RET=MT_SUCCESS;
    if(NULL==out){
        return MT_FAILURE;
    }
    out->u32Size=0;
    if(2==pChnAttr->chiptype){
        if(MT_CIPHER_ALG_AES > pChnAttr->stUserCfg.stDecryptCfg.enType){
            alginlen=8;
        }else{
            alginlen=16;
        }
        dechead=0;
        declen=u32BytesSend;
        round_hd=(u64ReadOffset%alginlen);                      //1
        round_tl=((u64ReadOffset+u32BytesSend)%alginlen);       //1
        //printf("++off=%llx,len=%x,rd.head=%x,rd.tl=%x\n",u64ReadOffset,u32BytesSend,round_hd,round_tl);
        if(round_hd){
            dechead+=(alginlen-round_hd);                       //+15.dump to next block
            declen-=(alginlen-round_hd);                        //-15
            #ifdef TEST_PVR_MEM_CRYPTO
            if(mem){
                memcpy(pChnAttr->hCipher_head,mem+(MT_U32)(u64ReadOffset-round_hd),alginlen);
            }
            #else
            MT_Pvr_Play_Read_File(pChnAttr->hCipher_head, u64ReadOffset-round_hd, alginlen, pChnAttr);
            #endif
            pthread_mutex_lock(&g_pvrcrypto);
            ret=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
            RET|=ret;
            ret=mt_unf_cipher_crypto_process(pChnAttr->hCipher,pChnAttr->hCipher_head,pChnAttr->hCipher_head,alginlen);
            RET|=ret;
            ret=MT_PVR_CipherPlyDestroy(pChnAttr);
            RET|=ret;
            pthread_mutex_unlock(&g_pvrcrypto);
            //dumpdata("dec.head:",pChnAttr->hCipher_head,alginlen);
        }
        if(round_tl){                                           //1
            declen-=round_tl;                                   //-1
            #ifdef TEST_PVR_MEM_CRYPTO
            if(mem){
                memcpy(pChnAttr->hCipher_tail,mem+(MT_U32)(u64ReadOffset+u32BytesSend-round_tl),alginlen);
            }
            #else
            MT_Pvr_Play_Read_File(pChnAttr->hCipher_tail, u64ReadOffset+u32BytesSend-round_tl, alginlen, pChnAttr);
            #endif
            pthread_mutex_lock(&g_pvrcrypto);
            ret=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
            RET|=ret;
            ret=mt_unf_cipher_crypto_process(pChnAttr->hCipher,pChnAttr->hCipher_tail,pChnAttr->hCipher_tail,alginlen);
            RET|=ret;
            ret=MT_PVR_CipherPlyDestroy(pChnAttr);
            RET|=ret;
            pthread_mutex_unlock(&g_pvrcrypto);
            //dumpdata("dec.tail:",pChnAttr->hCipher_tail,alginlen);
        }
        if(declen){
            #ifdef TEST_PVR_MEM_CRYPTO
            if(mem){
                memcpy(out->pu8Data,mem+(MT_U32)u64ReadOffset,u32BytesSend);
            }
            #else
            MT_Pvr_Play_Read_File(out->pu8Data, u64ReadOffset, u32BytesSend, pChnAttr);
            #endif
            //dumpdata("src.midl:",out->pu8Data+dechead,declen);
            pthread_mutex_lock(&g_pvrcrypto);
            ret=MT_PVR_CipherPlyCreate(pChnAttr,MT_CIPHER_CRYPTO_CH_0,MT_CIPHER_OPERATION_DECRYPT);
            RET|=ret;
            ret=mt_unf_cipher_crypto_process(pChnAttr->hCipher,out->pu8Data+dechead,out->pu8Data+dechead,declen);
            RET|=ret;
            RET|=ret;
            ret=MT_PVR_CipherPlyDestroy(pChnAttr);
            RET|=ret;
            pthread_mutex_unlock(&g_pvrcrypto);
            //dumpdata("dec.midl:",out->pu8Data+dechead,declen);
        }
        if(round_hd){   //from 1 cp len=15 to head
            memcpy(out->pu8Data,pChnAttr->hCipher_head+round_hd,alginlen-round_hd);
            //dumpdata("cpy.head:",out->pu8Data,alginlen-round_hd);
        }
        if(round_tl){   //from 0 cp len=1 to tail
            memcpy(out->pu8Data+u32BytesSend-round_tl,pChnAttr->hCipher_tail,round_tl);
            //dumpdata("cpy.tail:",out->pu8Data+u32BytesSend-round_tl,round_tl);
        }
        //printf("f=%llx,len=%x,round_hd=%x,midd=%x,round_tl=%x\n",u64ReadOffset,u32BytesSend,round_hd,declen,round_tl);
        //dumpdata("out:",out->pu8Data,u32BytesSend);
        out->u32Size=u32BytesSend;
    }
    #endif
    return RET;
}
#endif
*/
STATIC INLINE MT_S32 PVRPlayPrepareCipher(PVR_PLAY_CHN_S  *pChnAttr)
{
    MT_UNF_PVR_CIPHER_S *pCipherCfg;
#ifndef CONFIG_MT_LXC_SUPPORT
    mt_s32 ret=0;
#endif
    pCipherCfg = &(pChnAttr->stUserCfg.stDecryptCfg);
    if (!pCipherCfg->bDoCipher){
        return MT_SUCCESS;
    }
    #ifndef CONFIG_MT_LXC_SUPPORT
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon && p_addon->ts_cipher_size)
        {
            pChnAttr->hsec_mmz_ply.bufsize = p_addon->ts_cipher_size;
        }
        else
        {
            pChnAttr->hsec_mmz_ply.bufsize=MAX_CRYPTOSZ_R;
        }
        ret = mt_mmz_malloc(&pChnAttr->hsec_mmz_ply);
        if(ret){
            pChnAttr->hCipher_buffer=NULL;
            return MT_FAILURE;
        }
        pChnAttr->hCipher_buffer=pChnAttr->hsec_mmz_ply.user_viraddr;
    #endif
    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRPlayReleaseCipher(PVR_PLAY_CHN_S  *pChnAttr)
{
    #ifndef CONFIG_MT_LXC_SUPPORT
    if(pChnAttr->hCipher_buffer){
        mt_mmz_free(&pChnAttr->hsec_mmz_ply);
        //else{
        //    free(pChnAttr->hCipher_buffer);
        //}
        pChnAttr->hCipher_buffer=NULL;
    }
    #endif
    return MT_SUCCESS;
}

/*
dataEnd: position of valid data end in last TS pkg(Byte)
*                               dataEnd
*                                 |
original:                         V
* -----------------------------------------------------------
*| TS head | pending | valid data |  invalid data     |
*------------------------------------------------------------

0 == PVR_TS_MOVE_TO_END:
* -----------------------------------------------------------
*| TS head | pending | valid data |  0xff 0xff  ...  |
*------------------------------------------------------------

1 == PVR_TS_MOVE_TO_END:
* -----------------------------------------------------------
*| TS head | pending |   0xff 0xff  ... | valid data |
*------------------------------------------------------------
*/

/*****************************************************************************
 Prototype       : PVRPlaySendData
 Description     : by TS packet align mode, send pointed size data to demux, and the data must be cotinuious and valid
 Input           : pChnAttr     **the attribute of channel
                   offSet       ** the data offset from start in ts
                   bytesToSend  ** the data size
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/23
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
/*
PVR_CIPHER_PKG_LEN aligned, read file from here        PVR_CIPHER_PKG_LEN aligned, read file to here
|                                                                             |
|     188 aligned        offSet(picture header,StartCode)    188 aligned                                        |
|              |             |                                  |             |
V              V             V                                  V             V
-------------------------------------------------------------------------------
| xxx          | TS head |xx |       valid data    |    xxx     |0x47...      |
-------------------------------------------------------------------------------
|<-cipherHead->|<-headToAdd->|<---bytesToSend----->|<-endToAdd->|<-cipherEnd->|
|<---------------------------alignSize--------------------------------------->|
*/
 #if 1

STATIC INLINE MT_S32 PVRPlaySendToTsBuffer(PVR_PLAY_CHN_S *pChnAttr, MT_U64 u64ReadOffset,MT_U64 u64GlobeOffset,MT_U32 u32BytesSend, MT_U64 clroffSet)
{
    MT_S32   ret,timeslice=40000,timeconter=0;
    MT_U32   u32BytesRealSend = u32BytesSend;
    MT_U32   u32StartPos = 0;
    MT_U64   PhyAddr;
    MT_UNF_STREAM_BUF_S demuxBuf={0};
    MT_UNF_PVR_DATA_ATTR_S stDataAttr={0};

    PVR_INDEX_ENTRY_S stStartFrame;
    PVR_INDEX_ENTRY_S stEndFrame;

    memset(&stStartFrame, 0, sizeof(PVR_INDEX_ENTRY_S));
    memset(&stEndFrame, 0, sizeof(PVR_INDEX_ENTRY_S));

    /*find out ts buffer size u32ReadOnce*/
    ret = MT_UNF_DMX_GetTSBufferEx(pChnAttr->hTsBuffer, u32BytesSend, &demuxBuf, (phys_addr_t *)&PhyAddr, 0);
    PRV_PRINTF("PVR_PLAY_READ_FILE >>> ret=%d, u32BytesSend=%d, demuxBuf.pu8Data=0x%x\n",ret, u32BytesSend, demuxBuf.pu8Data);
    while (MT_SUCCESS != ret)
    {
        if((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
            return MT_SUCCESS;
        }
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_USLEEP(timeslice);
        PVR_LOCK_PLAY(pChnAttr);
        if((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
            return MT_SUCCESS;
        }
        ret = MT_UNF_DMX_GetTSBufferEx(pChnAttr->hTsBuffer, u32BytesSend, &demuxBuf, (phys_addr_t *)&PhyAddr, 0);
        timeconter++;
        if(timeconter>50){  //50*40=2s
            MT_ERR_PVR("++out2 %x,%x u32BytesSend=%d\n",timeconter,ret,u32BytesSend);
            return MT_FAILURE;
        }
    }
    //if(0!=timeconter){
        //printf("counter=%x\n",timeconter);
    //}

    /* read ts to buffer */
    //lint -e774
    vmx_pvr_play_printf("PVR_PLAY_READ_FILE >>> demuxBuf.pu8Data ====x%x, u64ReadOffset=0x%llx, u32BytesSend=%x, fd=%d  realcallback=%x, advca=%d\n",
                                            demuxBuf.pu8Data,u64ReadOffset, u32BytesSend, pChnAttr->s32DataFile,
                                            pChnAttr->readCallBack,pChnAttr->stUserCfg.bSupportAdvCa);

	if(u32BytesSend > 0)
	{
#ifdef VMX_ADVCA_PVR
		int rec_id = 0;
#endif
		//struct timeval start, end;
		//gettimeofday( &start, NULL );
		if(pChnAttr->readCallBack)
		{
            if(pChnAttr->stUserCfg.stDecryptCfg.bDoCipher)
            {
                #define ALIGNED_SIZE  16  //max(aes.16 des.8)
                #define NULLPACKTSZ   188
                mt_u32 cellsz=0;
                mt_u32 offset=0;
                mt_u32 morestart=0,moresize=0;
                mt_u32 alignedsize=u32BytesSend;
                mt_u32 aligneddst=0;
                mt_u64 u64alignedofft=u64ReadOffset,diffalign=0;
                MT_U8 *cryptobuff=NULL;
                mt_u32 phyaddr=0;
#ifdef CONFIG_MT_LXC_SUPPORT
                int fd;
                off_t in_phy;
#endif

                mt_u32 max_crypto_size = MAX_CRYPTOSZ_R;
                mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
                if(p_addon && p_addon->ts_cipher_size)
                {
                    max_crypto_size = p_addon->ts_cipher_size;
                }
                else
                {
                    morestart=(u64alignedofft&(ALIGNED_SIZE-1));
                    if(morestart)
                    {
                        aligneddst = morestart & 0x7;//MT_UNF_DMX_PutTSBufferEx need u32StartPos 8 bit alignment
                        u32StartPos = morestart + aligneddst;
                        alignedsize += morestart;
                    }
                    moresize =(alignedsize & (ALIGNED_SIZE-1));
                    if(moresize)
                    {
                        alignedsize +=(ALIGNED_SIZE - moresize);
                    }
                    u64alignedofft -= morestart;
                    //printf("++r:0x%llx,0x%llx,%d,%d,%d,%d,%d\n",u64alignedofft,u64ReadOffset,alignedsize,u32BytesRealSend,moresize,aligneddst,u32StartPos);
                }

                #ifdef CONFIG_MT_LXC_SUPPORT
                fd=PVR_GetTsFileFd(pChnAttr->s32DataFile);
                if(0<=fd){
                    cryptobuff=mt_pvr_ipc_getbufferby_tid(fd,max_crypto_size,&in_phy);
                    phyaddr=(mt_u32)in_phy;
                }
                #else
                cryptobuff=pChnAttr->hCipher_buffer;
                phyaddr=pChnAttr->hsec_mmz_ply.phyaddr;
                #endif
                if(NULL==cryptobuff){
                    return MT_FAILURE;
                }
                //printf("++r:%llx,%llx,%x,%x,%x,%x\n",u64alignedofft,u64ReadOffset,alignedsize,moresize,aligneddst,u32StartPos);
                    stDataAttr.u32ChnID = pChnAttr->u32chnID;
                if(MT_UNF_PVR_REC_DEFALT == pChnAttr->stUserCfg.bSupportAdvCa){
                    memcpy(&stDataAttr.usercfg,&pChnAttr->stUserCfg.stDecryptCfg,sizeof(MT_UNF_PVR_CIPHER_S));
                }
                stDataAttr.u32ChnID = pChnAttr->u32chnID;
                memcpy(&stDataAttr.usercfg,&pChnAttr->stUserCfg.stDecryptCfg,sizeof(MT_UNF_PVR_CIPHER_S));
                do{

                    if(max_crypto_size<=alignedsize){
                        cellsz=max_crypto_size;
                    }else{
                        cellsz=alignedsize;
                    }
                    stDataAttr.u64GlobalOffset=((u64GlobeOffset-diffalign)+offset);
                    stDataAttr.u32FirstPush=(pChnAttr->first_push|(pChnAttr->pause2resume_forirdeto<<8));
                    //printf("+++read goffset=%llx\n",stDataAttr.u64GlobalOffset);
                    ret = MT_Pvr_Play_Read_File(cryptobuff, u64alignedofft+offset, cellsz, pChnAttr);
                    if(ret != MT_SUCCESS)
                    {
                        break;
                    }
#ifdef CONFIG_MT_LXC_SUPPORT
                    mt_mem_flush((void*)cryptobuff,(size_t)cellsz);
#endif
                    ret = pChnAttr->readCallBack(&stDataAttr,
                                      						demuxBuf.pu8Data+aligneddst+offset,
                                      						PhyAddr+aligneddst+offset,
                                      						cryptobuff,
                                      						phyaddr,
                                      						0,
                                      						&cellsz);
                    pChnAttr->first_push=0;
                    pChnAttr->pause2resume_forirdeto=0;
                    offset+=cellsz;
                    alignedsize-=cellsz;
                }while(alignedsize);
                if (ret != MT_SUCCESS)
                {
                    MT_ERR_PVR("read call back error:%x\n", ret);
                      return MT_ERR_PVR_PLAY_INVALID_PACKETBUFFER;
                }
            }
            else
            {
                ret = MT_Pvr_Play_Read_File(demuxBuf.pu8Data, u64ReadOffset+u32StartPos, u32BytesSend, pChnAttr);
                if(ret != MT_SUCCESS)
                {
                    MT_ERR_PVR("read ts data error:%x\n", ret);
                    return MT_ERR_PVR_PLAY_INVALID_PACKETBUFFER;
                }
            }
        }
		ret = MT_UNF_DMX_PutTSBufferEx(pChnAttr->hTsBuffer, u32BytesRealSend, u32StartPos);
		if (ret != MT_SUCCESS)
		{
			MT_ERR_PVR("MT_UNF_DMX_PutTSBufferEx failed:%#x!\n", ret);
			return MT_ERR_PVR_PLAY_INVALID_STATE;
		}
		else
		{
			vmx_pvr_play_printf("MT_UNF_DMX_PutTSBufferEx  Success !\n");
			if (g_pvrfpSend)
			{
// the flowing comment code for debug
//                if(MT_UNF_PVR_PLAY_SPEED_32X_FAST_BACKWARD <= pChnAttr->enSpeed && MT_UNF_PVR_PLAY_SPEED_8X_FAST_BACKWARD >= pChnAttr->enSpeed)
//                {
				    fwrite(demuxBuf.pu8Data + u32StartPos, 1, u32BytesRealSend, g_pvrfpSend);
//                }
//debug code end
			}
		}
		//gettimeofday( &end, NULL );
		//printf("Push >>> %d : %d\n", ((end.tv_sec * 1000) + (end.tv_usec  / 1000)) - ((start.tv_sec * 1000) + (start.tv_usec  / 1000)),u32BytesRealSend);
    }
    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRPlaySendData(PVR_PLAY_CHN_S *pChnAttr, MT_U64 offSet, MT_U64 offglobe, MT_U32 bytesToSend, MT_U64 clroffSet)
{
    MT_U32 thispushsz=0;
    MT_U32 pushszedsz=0;
    MT_S32 retv=0;
    if(0){
        mt_s32 xret=0;
        MT_UNF_AVPLAY_STATUS_INFO_S playinfo;
        xret=MT_UNF_AVPLAY_GetVideoStatusInfo(pChnAttr->hAvplay, &playinfo);
        if(MT_SUCCESS==xret){
            MT_U32 remain=(playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize-playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize);
            if(remain<=bytesToSend){
                MT_ERR_PVR("+++push offset=%llx,size=%x.u=%x,a=%x\n",offSet,bytesToSend,
                                                    playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize,
                                                    playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize
                                                    );
            }
        }
    }

    do{
        thispushsz=bytesToSend;
        if(PUSH_VBLOCK_SZ < bytesToSend){
            thispushsz=PUSH_VBLOCK_SZ;
        }
        retv=PVRPlaySendToTsBuffer(pChnAttr,offSet+pushszedsz,offglobe+pushszedsz, thispushsz,clroffSet);
        if(MT_SUCCESS != retv){//try again.
            MT_USLEEP(20*1000);
            retv=PVRPlaySendToTsBuffer(pChnAttr,offSet+pushszedsz,offglobe+pushszedsz,thispushsz,clroffSet);
            if(MT_ERR_PVR_FILE_CANT_READ==retv){
                pChnAttr->play_errcode=retv;
            }
        }
        if(MT_SUCCESS != retv){
            MT_ERR_PVR("+++++push err\n");
            return retv;
        }
        pushszedsz+=thispushsz;
        bytesToSend-=thispushsz;
    }while(bytesToSend);
    pChnAttr->play_errcode=0;

    return MT_SUCCESS;
}

#endif
/* unused.warning
STATIC MT_S32 PVRPlaySendPrivatePacketToTsBuffer(PVR_PLAY_CHN_S *pChnAttr, MT_U32 u32DisPlayTime, MT_U32 u32Pid)
{
    MT_S32 ret;
    MT_U32 u32Bytes2Send = PVR_TS_LEN;
    MT_U32 u32PhyAddr;
    MT_UNF_STREAM_BUF_S DataBuf;
    MT_U8 u8PesPacket[41] = {0x00, 0x00, 0x01, 0xee, 0x00, 0x00, 0x80, 0x00, 0x00,
                             0x00, 0x00, 0x01, 0x1E, 0x70, 0x76, 0x72, 0x63, 0x75, 0x72, 0x74, 0x6d,
                             0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    ret = MT_UNF_DMX_GetTSBufferEx(pChnAttr->hTsBuffer, u32Bytes2Send, &DataBuf, &u32PhyAddr, 0);
    if ((MT_ERR_DMX_NOAVAILABLE_BUF == ret) || (MT_ERR_DMX_TIMEOUT == ret))
    {
        if (MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)
        {
            MT_WARN_PVR("Send private packet, get tsbuffer fail! ret=%#x", ret);
            return MT_ERR_PVR_PLAY_INVALID_PACKETBUFFER;
        }
        else
        {
            while (MT_SUCCESS != ret)
            {
                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                {
                    return MT_FAILURE;
                }

                if (MT_ERR_DMX_NOAVAILABLE_BUF != ret)
                {
                    MT_ERR_PVR("MT_UNF_DMX_GetTSBufferEx failed:%#x!\n", ret);
                    return MT_FAILURE;
                }
                else
                {
                    MT_USLEEP(40000);
                    if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                    {
                        return MT_FAILURE;
                    }

                    ret = MT_UNF_DMX_GetTSBufferEx(pChnAttr->hTsBuffer, u32Bytes2Send, &DataBuf, &u32PhyAddr, 0);
                }
            }
        }
    }


    memset(DataBuf.pu8Data, 0xff, DataBuf.u32Size);
    DataBuf.pu8Data[0] = 0x47;
    DataBuf.pu8Data[1] = (MT_U8)(((u32Pid & 0x1f00) >> 8) | 0x40);
    DataBuf.pu8Data[2] = (MT_U8)(u32Pid & 0xff);
    DataBuf.pu8Data[3] = 0x10;
    memcpy((void *)((MT_U32)u8PesPacket + 21), &u32DisPlayTime, sizeof(u32DisPlayTime));
    memcpy((void *)((MT_U32)u8PesPacket + 25), &(pChnAttr->stFrmTag), sizeof(PVR_FRAME_TAG_S));
    memcpy((void *)((MT_U32)DataBuf.pu8Data+4), u8PesPacket, sizeof(u8PesPacket));

    ret = MT_UNF_DMX_PutTSBufferEx(pChnAttr->hTsBuffer, u32Bytes2Send, 0);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("MT_UNF_DMX_PutTSBufferEx failed:%#x!\n", ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}*/


/*****************************************************************************
 Prototype       : PVRPlaySendAframe
 Description     : send one frame data to demux
 Input           : pChnAttr     **the attribute of play channel
                   pframe       ** frame index info
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 PVRPlaySendAframe(PVR_PLAY_CHN_S  *pChnAttr,  PVR_INDEX_ENTRY_S *pframe)
{
    MT_S32 ret = MT_SUCCESS;
    MT_U64 push_offset=0,push_offset_glb=0;
    MT_U32 size1;
    MT_U64 Clear_posi = 0xffffffffffffffffULL;   //for 2 frames share 1 ts packet
    PVR_INDEX_ENTRY_S tmpframe;
    PVR_INDEX_ENTRY_S *p_frame=NULL;
    MT_ASSERT_RET(NULL != pframe);
    MT_ASSERT_RET(NULL != pChnAttr);

    tmpframe=*pframe;

    if (0 == pframe->u32FrameSize){
        return MT_SUCCESS;
    }

    if(MT_UNF_VCODEC_TYPE_MPEG2==pChnAttr->IndexHandle->VdecType){      //bug115667, 2 frames share 1 tspacket
        MT_U32 aligned_size=0;
        MT_U64 aligned_start=tmpframe.u64Offset-tmpframe.u8StcOffsinTs;   //aligned to 188*n
        MT_U64 aligned_start_glb=tmpframe.u64GlobalOffset-tmpframe.u8StcOffsinTs; //aligned to 188*n

        PVR_EVENT_D("PVRPlaySendAframe>>> push.1 %llx,%x,%x, %llx, %llx\n",tmpframe.u64Offset,tmpframe.u32FrameSize,
                        tmpframe.u8StcOffsinTs, pframe->u64Offset, pframe->u64GlobalOffset);
        tmpframe.u32FrameSize+=((MT_U32)tmpframe.u8StcOffsinTs);
        aligned_size=tmpframe.u32FrameSize/188*188;

        if((MT_UNF_PVR_PLAY_STATE_PLAY  == pChnAttr->enState) ||
           (MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState && MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD == pChnAttr->enSpeed)||
           (MT_UNF_PVR_PLAY_STATE_SF  == pChnAttr->enState))
        {
            tmpframe.u32FrameSize=aligned_size;
        }
        else
        {            //push by frame
            if(tmpframe.u32FrameSize != aligned_size){
                tmpframe.u32FrameSize=aligned_size+188;
                Clear_posi=(pframe->u64Offset+pframe->u32FrameSize);
            }else{
                tmpframe.u32FrameSize=aligned_size;
            }
        }
        tmpframe.u64Offset=aligned_start;
        tmpframe.u64GlobalOffset=aligned_start_glb;
        MT_INFO_PVR("+++push.2 %llx,%x\n",tmpframe.u64Offset,tmpframe.u32FrameSize);
        p_frame=&tmpframe;
    }
    else
    {
        p_frame=pframe;
    }

    size1 = p_frame->u32FrameSize;

    if (p_frame->u32FrameSize > PVR_PLAY_MAX_FRAME_SIZE){
        MT_WARN_PVR("Frame size too large, drop it(Size:%u, offset=%llu).\n",p_frame->u32FrameSize, p_frame->u64Offset);
        return MT_SUCCESS;
    }

    if (PVR_INDEX_is_Iframe(p_frame))
	{
		g_pvr_last_i_frame = pChnAttr->IndexHandle->u32ReadFrame;
	}
    
    PVRPLAY_LOG_SEND_FRAME("Send frame=%d type=%ld size=%d pts=%d(0x%x) disp=%d offset=%lld global offset=%lld\n",
        (pChnAttr->IndexHandle->u32ReadFrame - 1),PVR_INDEX_get_frameType(p_frame),p_frame->u32FrameSize,p_frame->u32PtsMs,p_frame->u32PtsMs,p_frame->u32DisplayTimeMs,p_frame->u64Offset,p_frame->u64GlobalOffset);

    frame_pushque(pChnAttr,pChnAttr->IndexHandle->u32ReadFrame,p_frame->u32FrameSize,p_frame->u32PtsMs,p_frame->u32DisplayTimeMs,p_frame->u64Offset);
    push_offset = p_frame->u64Offset;
    push_offset_glb = p_frame->u64GlobalOffset;

    if(pChnAttr->IndexHandle->stCycMgr.u64MaxCycSize > 0){
        if (PVR_INDEX_IS_REWIND(pChnAttr->IndexHandle) &&
            (PVR_INDEX_REWIND_BY_SIZE == PVR_INDEX_TYPE_REWIND(pChnAttr->IndexHandle))){
            push_offset = p_frame->u64Offset % PVR_INDEX_MAX_FILE_SIZE(pChnAttr->IndexHandle);
        }
    }
    MT_INFO_PVR(" pChnAttr->enState = %d, pChnAttr->enSpeed=%d\n",pChnAttr->enState,pChnAttr->enSpeed);
    if((MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState && MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD == pChnAttr->enSpeed)
        || (MT_UNF_PVR_PLAY_STATE_SF == pChnAttr->enState))       //not play status.push it directly
    {
        MT_U32 pushblocksz=0;
        MT_U32 now_pushsz=0;
        pChnAttr->vframe_reseted=0;
        if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == pChnAttr->IndexHandle->enIndexType){
            pushblocksz=PUSH_ABLOCK_SZ;
        }else{
            pushblocksz=PUSH_VBLOCK_SZ;
        }
        //PVR_PLAY_D("+++push vframe_start=%lld vframe_size=%d push_offset=%lld u64MaxCycSize=%lld\n",pChnAttr->vframe_start,pChnAttr->vframe_size,push_offset,pChnAttr->IndexHandle->stCycMgr.u64MaxCycSize);
        if((pChnAttr->vframe_start+pChnAttr->vframe_size) != push_offset){
            if(pChnAttr->vframe_size){                                                                      //discontinous,must push
                ret = PVRPlaySendData(pChnAttr, pChnAttr->vframe_start, pChnAttr->vframe_start_glb,(MT_U32)pChnAttr->vframe_size,0xffffffffffffffffULL);
                //PVR_PLAY_D("+++push.3: vframe_start=%lld vframe_size=%d push_offset=%lld\n",pChnAttr->vframe_start,pChnAttr->vframe_size,push_offset);
            }
            if(0==pChnAttr->vframe_reseted){  //maybe reseted by unf
                pChnAttr->vframe_start=push_offset;
                pChnAttr->vframe_start_glb=push_offset_glb;
                pChnAttr->vframe_size = size1;
                //PVR_PLAY_D("+++push.4: vframe_start=%lld vframe_size=%d\n",pChnAttr->vframe_start,pChnAttr->vframe_size);
            }
        }else{
            pChnAttr->vframe_size += size1;
        }
        if(ret)
        {
            MT_ERR_PVR("+++pvr.send.1 ret=%x\n",ret);
        }

        if(pChnAttr->vframe_size >= pushblocksz){                                                       //more than block,must push
            now_pushsz=(pChnAttr->vframe_size/pushblocksz)*pushblocksz;
            ret |= PVRPlaySendData(pChnAttr, pChnAttr->vframe_start, pChnAttr->vframe_start_glb,now_pushsz,0xffffffffffffffffULL);
            //PVR_PLAY_D("+++push.5: vframe_start=%lld now_pushsz=%d vframe_size=%d\n",pChnAttr->vframe_start,now_pushsz,pChnAttr->vframe_size);
            if(0==pChnAttr->vframe_reseted){  //maybe reseted by unf
                pChnAttr->vframe_start+= now_pushsz;
                pChnAttr->vframe_start_glb+=now_pushsz;
                pChnAttr->vframe_size -= now_pushsz;
                //PVR_PLAY_D("+++push.6: vframe_start=%lld vframe_size=%d\n",pChnAttr->vframe_start,pChnAttr->vframe_size);
            }
            pChnAttr->play_real_pushed=1;
        }

        if(ret){
            MT_ERR_PVR("+++pvr.send.2 ret=%x\n",ret);
        }
    }
    else
    {
        MT_U64 Clear_posi_1 = 0xffffffffffffffffULL;
        MT_U64 fsub1_endpos = (push_offset+size1);

        if(0xffffffffffffffffULL != Clear_posi)
        {
            if(Clear_posi < fsub1_endpos){
                Clear_posi_1=Clear_posi;
            }
        }
        ret = PVRPlaySendData(pChnAttr, push_offset, push_offset_glb,(MT_U32)p_frame->u32FrameSize,Clear_posi_1);
        pChnAttr->play_real_pushed=1;
    }

    return ret;
}

MT_S32 PVRPlaySendAframe_dealframe(PVR_PLAY_CHN_S  *pChnAttr,  PVR_INDEX_ENTRY_S *ipframe)
{
    MT_U32 i=0;
    MT_U32 frametime=(((1000/pChnAttr->u32FrameRate_fromav)>>1)+1);

    PVR_INDEX_ENTRY_S *sndframe=NULL;
    //PVR_INDEX_ENTRY_S *tmpframe=NULL;
    static PVR_INDEX_ENTRY_S lastIslice={0};
    PVR_INDEX_ENTRY_S tmpuse={0};

    if((PVR_PLAY_SEND_DATA_IP == pChnAttr->enSendDataMode) &&
       (pChnAttr->u32SkipPCnt_EveryI||pChnAttr->u32SkipPMore_EveryxI)){ //to discard pframe
        if(PVR_INDEX_is_Iframe(ipframe)){
            pChnAttr->idxwp=0;
            pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
        }
        if(pChnAttr->idxwp < pChnAttr->u32SkipPCnt_EveryI){
            pChnAttr->idxwp++;
            sndframe=ipframe;
            //printf("++send0 %x\n",pChnAttr->idxwp);
        }else{
            if(pChnAttr->u32PushAddPoint>100){
                pChnAttr->u32PushAddPoint-=100;
                pChnAttr->idxwp++;
                sndframe=ipframe;
                //printf("++send1 %x,%x\n",pChnAttr->idxwp,pChnAttr->u32PushAddPoint);
            }
        }
    }else if(PVR_PLAY_SEND_DATA_I == pChnAttr->enSendDataMode){     //to repeat iframe
        if(((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&(pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1)))){   //field mode
            if((FRAME_BOT_FIELD==pChnAttr->fieldtype) && (lastIslice.u32FrameSize!=0)){
                //printf("++0.repeat %d\n",pChnAttr->u32SkipPCnt_EveryI);
                for(i=0;  i<= pChnAttr->u32SkipPCnt_EveryI; i++){
                    memcpy(&tmpuse,&lastIslice,sizeof(PVR_INDEX_ENTRY_S));
                    frame_checkbuffer(pChnAttr);
                    PVRPlaySendAframe(pChnAttr,&tmpuse);
                    MT_USLEEP(15000);
                    memcpy(&tmpuse,ipframe,sizeof(PVR_INDEX_ENTRY_S));
                    if((tmpuse.u32PtsMs>lastIslice.u32PtsMs) &&
                       (tmpuse.u32PtsMs<(frametime+lastIslice.u32PtsMs))){
                        frame_checkbuffer(pChnAttr);
                        PVRPlaySendAframe(pChnAttr,&tmpuse);
                        //printf("+++pts=%d\n",tmpuse.u32PtsMs-lastIslice.u32PtsMs);
                        MT_USLEEP(15000);
                    }
                }

                pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                if(pChnAttr->u32PushAddPoint>100){
                    pChnAttr->u32PushAddPoint-=100;
                    //printf("++0.repeat.more %d\n");
                    memcpy(&tmpuse,&lastIslice,sizeof(PVR_INDEX_ENTRY_S));
                    frame_checkbuffer(pChnAttr);
                    PVRPlaySendAframe(pChnAttr,&tmpuse);
                    MT_USLEEP(15000);
                    memcpy(&tmpuse,ipframe,sizeof(PVR_INDEX_ENTRY_S));
                    if((tmpuse.u32PtsMs>lastIslice.u32PtsMs) &&
                       (tmpuse.u32PtsMs<(frametime+lastIslice.u32PtsMs))){
                        frame_checkbuffer(pChnAttr);
                        PVRPlaySendAframe(pChnAttr,&tmpuse);
                        //printf("+++pts=%d\n",tmpuse.u32PtsMs-lastIslice.u32PtsMs);
                        MT_USLEEP(15000);
                    }
                }
            }
            if(FRAME_TOP_FIELD==pChnAttr->fieldtype){
                memcpy(&lastIslice,ipframe,sizeof(PVR_INDEX_ENTRY_S));
                //printf("++3pts.i=%x,%x\n",ipframe->u32PtsMs,ipframe->u32FrameSize);
            }
        }else{
            //printf("++1.repeat %d\n",pChnAttr->u32SkipPCnt_EveryI);
            for(i=0;  i <= pChnAttr->u32SkipPCnt_EveryI; i++){
                memcpy(&tmpuse,ipframe,sizeof(PVR_INDEX_ENTRY_S));
                PVRPlaySendAframe(pChnAttr,&tmpuse);
                MT_USLEEP(15000);
            }
            pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
            if(pChnAttr->u32PushAddPoint>100){
                pChnAttr->u32PushAddPoint-=100;
                //printf("++1.repeat.more\n");
                memcpy(&tmpuse,ipframe,sizeof(PVR_INDEX_ENTRY_S));
                PVRPlaySendAframe(pChnAttr,&tmpuse);
            }
        }
        return MT_SUCCESS;
    }else{
        sndframe=ipframe;
    }

    if(NULL!=sndframe){
        //printf("+++normal send\n");
        return PVRPlaySendAframe(pChnAttr,sndframe);
    }else{
        return PVR_CACHE_FRAME_ERROR;
    }
}

/*****************************************************************************
 Prototype       : PVRPlayCheckError
 Description     : Check return value, if not success, trigger callback event
 Input           : pChnAttr  **
                   ret     **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/5/26
    Author       : q46153
    Modification : Created function

*****************************************************************************/
STATIC INLINE MT_VOID PVRPlayCheckError(const PVR_PLAY_CHN_S  *pChnAttr,  MT_S32 ret)
{
    MT_INFO_PVR("00====callback occured, ret=0x%x\n", ret);
    if (MT_SUCCESS  == ret)
    {
        return;
    }

    /* after pause, stop play, notice error play event, AI7D02621 */
    if (MT_ERR_DMX_NOAVAILABLE_BUF == ret
        || MT_ERR_DMX_NOAVAILABLE_DATA == ret)
    {
        return;
    }

    MT_INFO_PVR("0====callback occured, ret=0x%x\n", ret);
    switch (ret)
    {
        case MT_ERR_PVR_FILE_TILL_END:
            PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",
                pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->IndexHandle->stCycMgr.u32StartFrame,
                pChnAttr->IndexHandle->stCycMgr.u32EndFrame,pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
            if (PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName))
            {
                pChnAttr->IndexHandle->cause_resume=4;
                PVRPlayPostEvent(pChnAttr->u32chnID, MT_UNF_PVR_EVENT_PLAY_REACH_REC, 0);
                PVR_PLAY_D("+++++++++cause_resume set to 4(PLAY REACH REC)\n");
            }
            else
            {
                pChnAttr->IndexHandle->cause_resume=3;
                PVRPlayPostEvent(pChnAttr->u32chnID, MT_UNF_PVR_EVENT_PLAY_EOF, 0);
		        MT_INFO_PVR("2====callback occured, ret=0x%x\n", ret);
                PVR_PLAY_D("+++++++++cause_resume set to 3(EOF)\n");
            }
            break;
        case MT_ERR_PVR_FILE_TILL_START:
            //if (!PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName))
            {
                pChnAttr->IndexHandle->cause_resume=1;
                PVRPlayPostEvent(pChnAttr->u32chnID, MT_UNF_PVR_EVENT_PLAY_SOF, 0);
                PVR_PLAY_D("+++++++++cause_resume set to 1(SOF)\n");
            }
          MT_INFO_PVR("3====callback occured, ret=0x%x\n", ret);
            break;
        default:
            PVRPlayPostEvent(pChnAttr->u32chnID, MT_UNF_PVR_EVENT_PLAY_ERROR, ret);
    }

    return;
}

/* play catch up to the record, how long wait to retry */
/*
STATIC INLINE MT_U32 PVRPlayCalcWaitTimeForPlayEnd(PVR_PLAY_CHN_S  *pChnAttr)
{
#define PVR_MIN_HDD_SIZE  10000
#define PVR_MID_HDD_SIZE  400000
#define PVR_DFT_WAIT_TIME 40000

    MT_U32 u32BufSizeHdd = 0;
    MT_U32 u32WaitTime = 0;
    MT_S32 ret = 0;
    MT_UNF_DMX_TSBUF_STATUS_S  tsBufStatus;

    ret = MT_UNF_DMX_GetTSBufferStatus(pChnAttr->hTsBuffer, &tsBufStatus);
    if (MT_SUCCESS != ret)
    {
        return PVR_DFT_WAIT_TIME;
    }
    u32BufSizeHdd = tsBufStatus.u32UsedSize;
    if (u32BufSizeHdd < PVR_MIN_HDD_SIZE)
        u32WaitTime = (800000 - u32BufSizeHdd);
    else if (u32BufSizeHdd >= PVR_MIN_HDD_SIZE && u32BufSizeHdd < PVR_MID_HDD_SIZE)
        u32WaitTime = (800000 - u32BufSizeHdd) / 100;
    else
        u32WaitTime = 1000;

    return u32WaitTime;
}

*/

/* seek the read pointer of index to the output one frame, so that, seek or play state can get the correct reference position*/
STATIC INLINE MT_S32 PVRPlaySeekToCurFrame(PVR_INDEX_HANDLE handle, PVR_PLAY_CHN_S  *pChnAttr,
            MT_UNF_PVR_PLAY_STATE_E enCurState, MT_UNF_PVR_PLAY_STATE_E enNextState)
{
    MT_S32 ret;
    MT_U32 u32SeekToPTS;
    MT_U32 u32SeekToTime;
    MT_U32 IsForword;
    MT_U32 IsNextForword;
    MT_UNF_AVPLAY_STATUS_INFO_S stInfo;
    MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S stAvplayPrivInfo;

    memset(&stAvplayPrivInfo, 0, sizeof(MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S));
    memset(&stInfo, 0, sizeof(MT_UNF_AVPLAY_STATUS_INFO_S));
    if (MT_UNF_PVR_PLAY_STATE_INVALID == pChnAttr->enState)
    {
        MT_ERR_PVR("Can not seek to current play frame when state is invalid!\n");
        return MT_FAILURE;
    }

    if (MT_UNF_PVR_PLAY_STATE_STEPF == pChnAttr->enState)
    {
        PVRPlaySyncTrickPlayTime(pChnAttr);
    }

    /* if current play to the start or end of file, seek it to start or end directly. no longer find it by current frame. */
    if (pChnAttr->bEndOfFile)
    {
        if (pChnAttr->bTillStartOfFile)
        {
            PVR_INDEX_ENTRY_S stStartEntry = {0};
            PVR_PLAY_D("----till start of file, seek to start\n");
            ret =  PVR_Index_SeekToStart(handle);
            if ( MT_SUCCESS != ret )
            {
                MT_ERR_PVR("seek to start entry error\n");
                return ret;
            }
            /*For DTS2014052701682, Sync modify from V1R1*/
            ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stStartEntry, pChnAttr->IndexHandle->stCycMgr.u32StartFrame);
            if (MT_SUCCESS == ret)
            {
                pChnAttr->u32CurPlayTimeMs = stStartEntry.u32DisplayTimeMs;
            }
            else
            {
                MT_ERR_PVR("PVR_Index_GetFrameByNum  Get Start Enctr error, ret = 0x%x\n",ret);
            }
            return MT_SUCCESS;
        }
        else
        {
            PVR_INDEX_ENTRY_S stEndEntry = {0};
            ret =  PVR_Index_SeekToEnd(handle);
            if ( MT_SUCCESS != ret )
            {
                MT_ERR_PVR("seek to end entry error\n");
                return ret;
            }
            ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stEndEntry, pChnAttr->IndexHandle->stCycMgr.u32EndFrame);

            if (MT_SUCCESS == ret)
            {
                pChnAttr->u32CurPlayTimeMs = stEndEntry.u32DisplayTimeMs;
            }
            else
            {
                if (MT_ERR_PVR_FILE_TILL_END == ret)
                {
                    ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stEndEntry, pChnAttr->IndexHandle->stCycMgr.u32EndFrame - 1);
                    if (MT_SUCCESS != ret)
                    {
                        MT_ERR_PVR("get the %d entry fail.\n", pChnAttr->IndexHandle->stCycMgr.u32EndFrame - 1);
                        return MT_FAILURE;
                    }
                }
                else
                {
                    MT_ERR_PVR("PVR_Index_GetFrameByNum  Get End Enctr error, ret = 0x%x\n",ret);
                }
            }
            return MT_SUCCESS;
        }
    }

    if (PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName) &&
        (MT_UNF_PVR_PLAY_STATE_INIT == pChnAttr->enLastState))
    {
        if ((MT_UNF_PVR_PLAY_STATE_FB == enCurState)
            || (MT_UNF_PVR_PLAY_STATE_STEPB == enCurState))
        {
            IsForword = 1;
        }
        else
        {
            IsForword = 0;
        }

        if ((MT_UNF_PVR_PLAY_STATE_FB == enNextState)
            || (MT_UNF_PVR_PLAY_STATE_STEPB == enNextState))
        {
            IsNextForword = 1;
        }
        else
        {
            IsNextForword = 0;
        }

        ret = MT_UNF_AVPLAY_GetStatusInfo(pChnAttr->hAvplay, &stInfo);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("MT_UNF_AVPLAY_GetStatusInfo failed!\n");
            return MT_FAILURE;
        }

        if (PVR_INDEX_IS_TYPE_AUDIO(handle))
        {
             u32SeekToPTS = (MT_U32)stInfo.stSyncStatus.u64LocalTime;
        }
        else
        {
             u32SeekToPTS = (MT_U32)stInfo.stSyncStatus.u64LastVidPts;
        }
        /* evade case */
        if ((PVR_INDEX_INVALID_PTSMS == u32SeekToPTS)||((stInfo.stSyncStatus.u64LastVidPts-stInfo.stSyncStatus.u64FirstVidPts)<500))
        {
            MT_WARN_PVR("current pts invalid(-1), do not seek to it!\n");
            return MT_SUCCESS;
        }


        if (MT_TRUE == pChnAttr->bTimeShiftStartFlg)
        {
            u32SeekToPTS = pChnAttr->IndexHandle->stCurPlayFrame.u32PtsMs;
        }

        ret = PVR_Index_SeekToPTS(handle, u32SeekToPTS, IsForword, IsNextForword);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVR_Index_SeekToPTS not found the PTS failed!\n");
            return MT_FAILURE;
        }
    }
    else
    {
        if ( pChnAttr->bRecordedVideoExist == MT_TRUE)
        {
#if 0
            ret = MT_UNF_AVPLAY_Invoke(pChnAttr->hAvplay, MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO, (void *)&stAvplayPrivInfo);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("Get Avplay private info fail.\n");
                u32SeekToTime = pChnAttr->u32CurPlayTimeMs;
            }
            else
            {
                u32SeekToTime = stAvplayPrivInfo.u32LastPlayTime;
            }
#else
            u32SeekToTime = pChnAttr->u32CurPlayTimeMs;
#endif
        }
        else
        {
            /*In pure audio case, or can not get last disptime,
              using channel timer value to seek, and update stAvplayPrivInfo.u32LastPlayTime*/
            u32SeekToTime = pChnAttr->u32CurPlayTimeMs;
            stAvplayPrivInfo.u32LastPlayTime = u32SeekToTime;
        }
        if (MT_TRUE == pChnAttr->bTimeShiftStartFlg)
        {
            mt_u32 absv=0;
            u32SeekToTime = pChnAttr->u32CurPlayTimeMs;
            if(pChnAttr->u32CurPlayTimeMs >= stAvplayPrivInfo.u32LastPlayTime){
                absv=pChnAttr->u32CurPlayTimeMs - stAvplayPrivInfo.u32LastPlayTime;
            }else{
                absv=stAvplayPrivInfo.u32LastPlayTime - pChnAttr->u32CurPlayTimeMs;
            }
            if (1000 >= absv)//abs(pChnAttr->u32CurPlayTimeMs - stAvplayPrivInfo.u32LastPlayTime))
            {
                pChnAttr->bTimeShiftStartFlg= MT_FALSE;
                u32SeekToTime = stAvplayPrivInfo.u32LastPlayTime;
            }
        }
        /*The following statement cause DTS2014072302180.
               The reason is stAvplayPrivInfo may be error,  so it should not assign to pChnAttr->u32CurPlayTimeMs*/
/*
        else
        {
            pChnAttr->u32CurPlayTimeMs = stAvplayPrivInfo.u32LastPlayTime;
        }
*/
        ret = PVR_Index_SeekToTime(handle, u32SeekToTime);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("PVR_Index_SeekToTime not found the time failed!\n");
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

/* reset buffer and player, seek ts position to the current play frame, if that frame invalid, mean to reset it already, the decoder no longer reset it */
STATIC INLINE MT_S32 PVRPlayResetToCurFrame(PVR_INDEX_HANDLE handle, PVR_PLAY_CHN_S  *pChnAttr, MT_UNF_PVR_PLAY_STATE_E enNextState)
{
    MT_S32 ret = MT_SUCCESS;
    MT_UNF_PVR_PLAY_STATE_E enPreState;

    return MT_SUCCESS;
    /* failed to get current frame, no longer reset player and ts buffer, imply has reset it already*/
    if (MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
    {
        //enPreState = pChnAttr->enLastState;
        ret = MT_SUCCESS;
    }
    else
    {
        enPreState = pChnAttr->enState;
        ret = PVRPlaySeekToCurFrame(pChnAttr->IndexHandle, pChnAttr, enPreState, enNextState);
    }

    if (MT_SUCCESS == ret)
    {
        MT_INFO_PVR("to reset buffer and player.\n");
        PVR_Index_ChangePlayMode(pChnAttr->IndexHandle);


        pChnAttr->bTsBufReset = MT_TRUE;
        pChnAttr->bNotAvailableTsBuff = MT_FALSE;
        //pChnAttr->bSendRewindFail = MT_FALSE;
        //pChnAttr->u32FailToSendTimes = 0;

        ret = MT_UNF_DMX_ResetTSBuffer(pChnAttr->hTsBuffer);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("ts buffer reset failed!\n");
            return MT_FAILURE;
        }

        ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("AVPLAY reset failed!\n");
            return MT_FAILURE;
        }
    }


    UNUSED(handle);
    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRPlayAvplaySyncCtrl(PVR_PLAY_CHN_S  *pChnAttr, MT_BOOL bEnableSync)
{
    MT_UNF_SYNC_ATTR_S          stSyncAttr;
    MT_UNF_AVPLAY_STOP_OPT_S    stStopOpt;
    MT_U32 u32VidPid = 0x1fff;
    MT_U32 u32AudPid = 0x1fff;
    MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaChn = (MT_UNF_AVPLAY_MEDIA_CHAN_E)0;

    stStopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
    stStopOpt.u32TimeoutMs = 0;

    //lint -e655
    if (MT_SUCCESS == MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &u32VidPid))
    {
        if (0x1fff != u32VidPid)
        {
            enMediaChn |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        }
    }
    if (MT_SUCCESS == MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &u32AudPid))
    {
        if (0x1fff != u32AudPid)
        {
            enMediaChn |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        }
    }
    //lint +e655

    if ((MT_UNF_AVPLAY_MEDIA_CHAN_E)0 == enMediaChn)
    {
        MT_ERR_PVR("No Vpid and Apid!\n");
        return MT_FAILURE;
    }

    if (MT_SUCCESS != MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
    {
        MT_ERR_PVR("get sync attr failed!!!!!!!!!!!!!!!!!!\n");
        return MT_FAILURE;
    }

    if((MT_FALSE == bEnableSync)&&(MT_UNF_SYNC_REF_NONE == stSyncAttr.enSyncRef))
    {
        pChnAttr->enLastSyncState = stSyncAttr.enSyncRef;
        return MT_SUCCESS;
    }

    if (MT_SUCCESS != MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, enMediaChn, &stStopOpt))
        return MT_FAILURE;

    if(MT_FALSE == bEnableSync)
    {
        pChnAttr->enLastSyncState = stSyncAttr.enSyncRef;
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
    }
    else
    {
        if(MT_UNF_AVPLAY_SYNC_REF_BUTT != pChnAttr->enLastSyncState)
        {
            PVR_PLAY_P("--------set enSyncRef=%d to enLastSyncState=%d\n",stSyncAttr.enSyncRef,pChnAttr->enLastSyncState);
            stSyncAttr.enSyncRef = pChnAttr->enLastSyncState;
        }
    }
    PVR_PLAY_P("--------call sync enSyncRef=%d\n",stSyncAttr.enSyncRef);
    if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
    {
        (void)MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, enMediaChn, MT_NULL);
        return MT_FAILURE;
    }

    if (MT_SUCCESS != MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, enMediaChn, MT_NULL))
        return MT_FAILURE;

    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRPlayCheckIfTsOverByRec(PVR_PLAY_CHN_S  *pChnAttr)
{
    MT_S32 ret;
    MT_BOOL seek2start = MT_TRUE;

    if((MT_UNF_PVR_PLAY_SPEED_NORMAL>=pChnAttr->enSpeed)&&(0<pChnAttr->enSpeed) &&
       (MT_UNF_PVR_PLAY_STATE_PAUSE!=pChnAttr->enState)){ //if ff, fb, pause,no need to deal it
        if (PVR_Index_QureyClearRecReachPlay(pChnAttr->IndexHandle))
        {
            PVR_PLAY_D("+++play.recrechpaly0.%x,%x r=%d s=%d\n",pChnAttr->enState,pChnAttr->enSpeed,
                pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->IndexHandle->stCycMgr.u32StartFrame);
            pvrplay_clearvframe(pChnAttr);
            if(pChnAttr->IndexHandle->u32ReadFrame > pChnAttr->IndexHandle->stCycMgr.u32StartFrame &&
                pChnAttr->IndexHandle->u32ReadFrame -pChnAttr->IndexHandle->stCycMgr.u32StartFrame >=  PVR_TPLAY_MIN_DISTANCE){
                seek2start = MT_FALSE;//read - start enough,may have seek to start before,not need to seek to start again
                PVR_PLAY_D("not need to seek to start r-s=%d\n",pChnAttr->IndexHandle->u32ReadFrame -pChnAttr->IndexHandle->stCycMgr.u32StartFrame);
            }

            if(seek2start){
                PVR_Index_SeekToStart(pChnAttr->IndexHandle);
            }
            pChnAttr->bTsBufReset = MT_TRUE;
            ret = MT_UNF_DMX_ResetTSBuffer(pChnAttr->hTsBuffer);
            if (MT_SUCCESS != ret){
                MT_ERR_PVR("ts buffer reset failed!\n");
                return ret;
            }
            ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, NULL);
            if (MT_SUCCESS != ret){
                MT_ERR_PVR("AVPLAY reset failed!\n");
                return ret;
            }
        }
    }

    return MT_SUCCESS;
}

STATIC INLINE MT_S32 PVRPlayIsChnTplay(MT_U32 u32Chn)
{
    PVR_PLAY_CHN_S  *pChnAttr;

    PVR_PLAY_CHECK_INIT(&g_stPlayInit);

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    PVR_PLAY_CHECK_CHN_INIT(pChnAttr->enState);

    if ( pChnAttr->enState > MT_UNF_PVR_PLAY_STATE_PAUSE
        && pChnAttr->enState < MT_UNF_PVR_PLAY_STATE_STOP)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_PVR_PLAY_INVALID_STATE;
    }
}

/*
check if the frame to play is saved to ts file
*/
STATIC INLINE MT_BOOL PVRPlayIsTsSaved(PVR_PLAY_CHN_S *pChnAttr, PVR_INDEX_ENTRY_S *pFrameToPlay)
{
    if (pChnAttr->IndexHandle->bIsRec)
    {
        if (pChnAttr->IndexHandle->u64FileSizeGlobal >= (MT_U64)pFrameToPlay->u64GlobalOffset)
        {
            return MT_TRUE;
        }
        else
        {
            MT_ERR_PVR("Play Over Rec when timeshift: R/W/Real: %lld, %llu, %llu.\n",pFrameToPlay->u64GlobalOffset,pChnAttr->IndexHandle->u64GlobalOffset, pChnAttr->IndexHandle->u64FileSizeGlobal );
            return MT_FALSE;
        }
    }
    else
    {
        return MT_TRUE;
    }
}
/* unused.warning
STATIC INLINE MT_S32 PVRPlayGetCurPlayPts(PVR_PLAY_CHN_S  *pChnAttr, MT_U32 *pu32CurPlayPts)
{
    MT_UNF_AVPLAY_STATUS_INFO_S stInfo;
    PVR_INDEX_ENTRY_S  frame_tmp ={0};
    MT_U32 u32LoopTime = 0;
    MT_S32 ret = 0;
    do
    {
        ret = MT_UNF_AVPLAY_GetStatusInfo(pChnAttr->hAvplay, &stInfo);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("MT_UNF_AVPLAY_GetStatusInfo failed!\n");

            return MT_FAILURE;
        }

        if (PVR_INDEX_IS_TYPE_AUDIO(pChnAttr->IndexHandle))
            *pu32CurPlayPts = stInfo.stSyncStatus.u64LocalTime;
        else
            *pu32CurPlayPts = stInfo.stSyncStatus.u64LastVidPts;

        u32LoopTime ++;
        if (PVR_INDEX_INVALID_PTSMS == *pu32CurPlayPts)
        {
            if (u32LoopTime > 10)
            {
                MT_WARN_PVR("Get invalid current pts %d from MT_UNF_AVPLAY_GetStatusInfo\n", *pu32CurPlayPts);
                break;
            }
            MT_USLEEP(100*1000);
        }

    } while(PVR_INDEX_INVALID_PTSMS == *pu32CurPlayPts);

    if (PVR_INDEX_INVALID_PTSMS == *pu32CurPlayPts)
    {
        ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &frame_tmp, pChnAttr->IndexHandle->u32ReadFrame);
        if(MT_SUCCESS != ret)
        {
            if(MT_ERR_PVR_FILE_TILL_END == ret)
            {
                if(MT_SUCCESS != PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &frame_tmp, --pChnAttr->IndexHandle->u32ReadFrame))
                {
                    MT_INFO_PVR("get the %d entry fail.\n", pChnAttr->IndexHandle->u32ReadFrame);
                    return MT_FAILURE;
                }
            }
        }
        *pu32CurPlayPts = frame_tmp.u32PtsMs;
    }

    return MT_SUCCESS;
}*/

void PVRPlaySyncTrickPlayTime(PVR_PLAY_CHN_S *pChnAttr)
{
    MT_U32 u32CurFrmTimeMs = 0;

    if ( pChnAttr->bRecordedVideoExist == MT_FALSE)
    {
        MT_WARN_PVR("Pure Audio stream not support Trick Play\n");
        return;
    }
#if 0
    ret = MT_UNF_AVPLAY_Invoke(pChnAttr->hAvplay, MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO, &stAvplayPrivInfo);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Get Avplay private info fail.\n");
        return;
    }
#endif
/* Tscancode: duplicate branches for 'if' and 'else'. */
//    if (MT_TRUE == pChnAttr->bTimeShiftStartFlg)
    {
        u32CurFrmTimeMs = pChnAttr->u32CurPlayTimeMs;
    }
//    else
//    {
//        u32CurFrmTimeMs = stAvplayPrivInfo.u32LastPlayTime;
//      u32CurFrmTimeMs = pChnAttr->u32CurPlayTimeMs;
//    }

    if(u32CurFrmTimeMs > pChnAttr->u32CurPlayTimeMs)
    {
        if ((u32CurFrmTimeMs - pChnAttr->u32CurPlayTimeMs)>1000)
        {
            pChnAttr->u32CurPlayTimeMs = u32CurFrmTimeMs;
        }
    }
    else if(u32CurFrmTimeMs < pChnAttr->u32CurPlayTimeMs)
    {
        if ((pChnAttr->u32CurPlayTimeMs - u32CurFrmTimeMs)>1000)
        {
            pChnAttr->u32CurPlayTimeMs = u32CurFrmTimeMs;
        }
    }
}

static INLINE void PVRPlayAnalysisStream(PVR_PLAY_CHN_S *pChnAttr)
{
    /*MT_U32 u32VideoType = 0;
    MT_U32 u32PreReadCnt = 0x400;
    MT_U32 u32StartFrm=0, u32EndFrm=0, u32LastFrm=0, u32TotalFrm=0;
    MT_U32 i = 0;
    MT_U32 u32NalHeader;
    */
    MT_U32 u32IPBCnt=0;
    PVR_INDEX_ENTRY_S stEntry;

    //MT_Index_GetTotalCntIP(pChnAttr->IndexHandle, &pChnAttr->u32IPCnt);
    //MT_Index_GetTotalCntI(pChnAttr->IndexHandle, &pChnAttr->u32ICnt);
    MT_Index_GetTotalCntI_IP(pChnAttr->IndexHandle, &pChnAttr->u32ICnt,&pChnAttr->u32IPCnt,&u32IPBCnt,50000);
    PVR_Index_GetFrameRate(pChnAttr->IndexHandle, &pChnAttr->u32FrameRate);

    memset(&stEntry,0 ,sizeof(PVR_INDEX_ENTRY_S));

    return;
    /*
    u32VideoType = (MT_U32)PVR_Index_GetVtype(pChnAttr->IndexHandle);
    u32VideoType -= 100;

    if(MT_UNF_VCODEC_TYPE_H264 != u32VideoType)
    {
        return;
    }

    u32StartFrm = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
    u32EndFrm = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
    u32LastFrm = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;

    if(u32StartFrm > u32EndFrm)
    {
        u32TotalFrm = u32EndFrm + u32LastFrm - u32StartFrm;
    }
    else
    {
        u32TotalFrm = u32LastFrm - u32StartFrm;
    }

    if (u32TotalFrm < u32PreReadCnt)
    {
        u32PreReadCnt = u32TotalFrm;
    }

    pChnAttr->u32GopNumOfStart = (u32PreReadCnt << 16);
    for(i = u32StartFrm; i < (u32StartFrm + u32PreReadCnt); i++)
    {
        u32NalHeader = 0;

        if (MT_SUCCESS != PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stEntry, i))
        {
            MT_ERR_PVR("Get IndexEntry fail, Can't Analysis anymore.\n");
            return;
        }

        if (PVR_INDEX_is_Pframe(&stEntry))
        {
            continue;
        }

        if (sizeof(u32NalHeader) != PVR_PREAD64((MT_U8 *)&u32NalHeader, sizeof(u32NalHeader), pChnAttr->s32DataFile, stEntry.u64Offset))
        {
            MT_ERR_PVR("Get Stream ES head fail, Can't Analysis anymore.\n");
            return;
        }

        u32NalHeader = (u32NalHeader >> 24);

        if (PVR_INDEX_is_Iframe(&stEntry))
        {
            if(1 != pChnAttr->stVdecCtrlInfo.u32IDRFlag)
            {
                if (5 == (u32NalHeader & 0x1f))
                {
                    pChnAttr->stVdecCtrlInfo.u32IDRFlag = 1;
                    MT_WARN_PVR("The playing stream's I frame has IDR\n");
                }
            }

            pChnAttr->u32GopNumOfStart++;
        }

        if ((PVR_INDEX_is_Bframe(&stEntry)) && (1 != pChnAttr->stVdecCtrlInfo.u32BFrmRefFlag))
        {
            if (0 != ((u32NalHeader >> 5) & 3))
            {
                pChnAttr->stVdecCtrlInfo.u32BFrmRefFlag = 1;
                MT_WARN_PVR("The playing stream's B frame can be reference\n");
            }
        }

        //if((0 != pChnAttr->stVdecCtrlInfo.u32IDRFlag) && (0 != pChnAttr->stVdecCtrlInfo.u32BFrmRefFlag))
        //{
        //    break;
        //}

        if((u32StartFrm > u32EndFrm) && ((u32StartFrm + u32PreReadCnt) > u32LastFrm) && (i == (u32LastFrm - 1)))
        {
            u32PreReadCnt = u32PreReadCnt - (u32LastFrm - u32StartFrm);
            i = 0;
            u32StartFrm = 0;
        }
    }*/
}

STATIC INLINE MT_BOOL PVRPlayCheckFBTillStartInRewind(PVR_PLAY_CHN_S  *pChnAttr,MT_U32 *disptime,MT_S32 SNotMove)
{
    PVR_INDEX_ENTRY_S stStartFrame = {0};
    MT_U32 u32CurFrmTimeMs=0,u32EndTimeGap=2000;
    MT_S32 s32Ret = 0;

    if(SNotMove){
        u32EndTimeGap = 3000;
    }
    //printf("+++%s,%d\n",__FUNCTION__,__LINE__);
    if(MT_SUCCESS ==Frame_GetDispTimeByCache(pChnAttr,NULL,&u32CurFrmTimeMs,NULL,NULL)){
        if(disptime){
            *disptime=u32CurFrmTimeMs;
        }
        s32Ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stStartFrame, pChnAttr->IndexHandle->stCycMgr.u32StartFrame);
        if (MT_SUCCESS != s32Ret){
            //printf("+++%s,%d\n",__FUNCTION__,__LINE__);
            return MT_FALSE;
        }
        if (u32CurFrmTimeMs <= (stStartFrame.u32DisplayTimeMs+u32EndTimeGap))
        {
            //printf("+++%s,%d,%d,%d,%d\n",__FUNCTION__,__LINE__,u32CurFrmTimeMs , stStartFrame.u32DisplayTimeMs,u32EndTimeGap);
            return MT_TRUE;
        }else{
            //printf("+++%s,%d,%d,%d,%d\n",__FUNCTION__,__LINE__,u32CurFrmTimeMs , stStartFrame.u32DisplayTimeMs,u32EndTimeGap);
            return MT_FALSE;
        }
    }else{
        //printf("+++%s,%d\n",__FUNCTION__,__LINE__);
        return MT_FALSE;
    }
}
STATIC INLINE MT_BOOL PVRPlayTime_CacBySpeed(PVR_PLAY_CHN_S *pChnAttr,MT_U32 timeslice)
{//timeslice=x00ms
    MT_U32 cntpersecond=(1000000/timeslice);
    if(MT_UNF_PVR_PLAY_SPEED_NORMAL<=pChnAttr->enSpeed){        //normal.speedup
        return (cntpersecond*2);
    }else if(MT_UNF_PVR_PLAY_SPEED_NORMAL<pChnAttr->enSpeed){   //ff
        return (cntpersecond*4);
    }else if(0<pChnAttr->enSpeed){                              //sf
        MT_U32 tmp=(cntpersecond*(MT_UNF_PVR_PLAY_SPEED_NORMAL/pChnAttr->enSpeed));
        return (tmp*3/2);
    }else if(MT_UNF_PVR_PLAY_SPEED_1X_FAST_BACKWARD>=pChnAttr->enSpeed){  //fb
        return (cntpersecond*2);
    }else if((0>pChnAttr->enSpeed)&&
             (MT_UNF_PVR_PLAY_SPEED_1X_FAST_BACKWARD<pChnAttr->enSpeed)){ //sb
        MT_U32 tmp=(cntpersecond*(MT_UNF_PVR_PLAY_SPEED_NORMAL/abs(pChnAttr->enSpeed)));
        return (tmp*3/2);
    }else {
        return (cntpersecond*2);
    }
}

MT_U32 PVRPlayFFEndTimeGap(PVR_PLAY_CHN_S *pChnAttr)
{
    MT_U32 u32EndTimeGap = 2000;
    switch(pChnAttr->enSpeed)
    {
        case MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD:
            u32EndTimeGap = 3000;
            break;
        case MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD:
            u32EndTimeGap = 4000;
            break;
        case MT_UNF_PVR_PLAY_SPEED_8X_FAST_FORWARD:
            u32EndTimeGap = 6000;
            break;
        case MT_UNF_PVR_PLAY_SPEED_16X_FAST_FORWARD:
            u32EndTimeGap = 6000;
            break;
        case MT_UNF_PVR_PLAY_SPEED_32X_FAST_FORWARD:
            u32EndTimeGap = 8000;
            break;
        case MT_UNF_PVR_PLAY_SPEED_64X_FAST_FORWARD:
            u32EndTimeGap = 10000;
            break;
        default:
            break;
    }
    return u32EndTimeGap;
}


STATIC INLINE MT_BOOL PVRPlayCheckFFTillEnd(PVR_PLAY_CHN_S *pChnAttr,MT_U32 *disptime,MT_S32 moveend)
{
    MT_U32 pts,diff,u32EndTimeGap = 2000;
    if(MT_SUCCESS ==Frame_GetDispTimeByCache(pChnAttr,NULL,&pts,NULL,NULL)){
        if(moveend){
            u32EndTimeGap = PVRPlayFFEndTimeGap(pChnAttr);   
        }
        if(pts>pChnAttr->IndexHandle->Latest_DisplayTimeMs){
            diff=pts-pChnAttr->IndexHandle->Latest_DisplayTimeMs;
        }else{
            diff=pChnAttr->IndexHandle->Latest_DisplayTimeMs-pts;
        }
        if(diff<=u32EndTimeGap){
            PVRPLAY_LOG_PLAY_OVER("++++end0:diff=%d,pts=%d,latest=%d,gap=%d\n",diff,pts,pChnAttr->IndexHandle->Latest_DisplayTimeMs,u32EndTimeGap);
            return MT_TRUE;
        }
        if(disptime){
            *disptime=pts;
        }
        //PVRPLAY_LOG_PLAY_OVER("++++end1:diff=%d,pts=%d,latest=%d, gap=%d\n",diff,pts,pChnAttr->IndexHandle->Latest_DisplayTimeMs,u32EndTimeGap);
        return MT_FALSE;
    }else{
        PVRPLAY_LOG_PLAY_OVER("++++end2 get disp time failed\n");
        //return MT_TRUE; //should not return ture if get disp time failed,other wise may play over immediately
        return MT_FALSE; 
    }
}

/*
STATIC INLINE void PVRPlayProcessLastPlayingFrames(PVR_PLAY_CHN_S *pChnAttr)
{
    if ( pChnAttr->enSpeed == MT_UNF_PVR_PLAY_SPEED_NORMAL  )
    {
        (MT_VOID)MT_UNF_AVPLAY_FlushStream(pChnAttr->hAvplay, MT_NULL);
    }
    return;
}
*/
PVR_PLAY_CHN_S* PVRPlayGetChnAttrByName(const MT_CHAR *pFileName)
{
    MT_U32 i = 0;

    if(NULL == pFileName)
    {
        MT_ERR_PVR("File name point is NULL.\n");
        return NULL;
    }

    for (i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        if  (!strncmp(g_stPvrPlayChns[i].stUserCfg.szFileName, pFileName, strlen(pFileName)) )
        {
            return (PVR_PLAY_CHN_S*)(&g_stPvrPlayChns[i]);
        }
    }

    return (PVR_PLAY_CHN_S*)NULL;
}


MT_S32 PVRPlaySmoothFBward(PVR_PLAY_CHN_S *pChnAttr,MT_HANDLE hWindow,MT_PVR_SEND_RESULT_S *pstSendFrame, MT_PVR_FETCH_RESULT_S *pPvrFetchRes){
  return 0;
}

/*****************************************************************************
 Prototype       : PVRPlayMainRoute
 Description     : the main control thread of player
 Input           : args  ** the attribute of play channel
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/

static MT_S32 get_next_field_slice(PVR_PLAY_CHN_S *pChnAttr,PVR_INDEX_ENTRY_S *tslice,PVR_INDEX_ENTRY_S *bslice,MT_U32 framerate)
{
    MT_S32 ret;
    framerate=(((1000/framerate)>>1)+1);    //+1 for protect
    for(;;){
        ret = PVR_Index_GetNextFrame(pChnAttr->IndexHandle, bslice);
        if(MT_SUCCESS!=ret){                //seek to end,out of range
            return ret;
        }
        if(PVR_INDEX_is_Iframe(bslice)){    //seek to next i slice
            PVR_Index_GetPreXFrame(pChnAttr->IndexHandle, bslice);
            return ret;
        }
        if((bslice->u32PtsMs >= tslice->u32PtsMs)&&
           ((bslice->u32PtsMs <= (tslice->u32PtsMs+framerate)))){//bug119313
            return MT_SUCCESS;
        }
    }
}
/*
static u32 heart_break_count = 0;
#define  HEART_BREAK_COUNT_MAX 50000
*/
STATIC void* PVRPlayMainRoute(void *args)
{
    //MT_U32              sendcounter=0;
    MT_S32              ret = MT_SUCCESS;
    mt_s32              ret0=0;
    MT_S32              ret_sent = MT_SUCCESS;
    PVR_INDEX_ENTRY_S   frame ={0};
    PVR_INDEX_ENTRY_S   frame_nouse ={0};
    PVR_PLAY_CHN_S      *pChnAttr = (PVR_PLAY_CHN_S*)args;
    MT_BOOL             bCallBack = MT_FALSE;
    MT_BOOL             bLastFrameSent = MT_TRUE;
    MT_HANDLE hWindow;
    MT_PVR_SEND_RESULT_S *pstSendFrame = (MT_PVR_SEND_RESULT_S *)NULL;
    MT_PVR_FETCH_RESULT_S *pPvrFetchRes = (MT_PVR_FETCH_RESULT_S *)NULL;
    //MT_CODEC_VIDEO_CMD_S  stVdecCmdPara = {0};
    MT_UNF_AVPLAY_TPLAY_OPT_S stTplayOpts;    
    MT_UNF_AVPLAY_STATUS_INFO_S playinfo_a={0};
    MT_UNF_AVPLAY_STATUS_INFO_S playinfo_v={0};
    MT_U32 first_run=MT_TRUE;
    mt_s32 ret_a, ret_v;
    MT_U32 pre_readframe=0;

    memset(&stTplayOpts, 0, sizeof(MT_UNF_AVPLAY_TPLAY_OPT_S));

    if ((!pChnAttr) || (!pChnAttr->IndexHandle))
    {
        return MT_NULL;
    }
    mt_set_pthread_name(__FUNCTION__);
#if 0
    if (NULL == g_pvrfpSend)
    {
        MT_CHAR saveName[256];
        i = 0;
        do{
            memset(saveName,sizeof(saveName),0);
            snprintf(saveName, 255, "%s_send_%d.ts", pChnAttr->stUserCfg.szFileName,i);
            //MT_ERR_PVR(saveName);
            if(access(saveName,F_OK) == 0)
            {
                ++i;
                if(i<200)
                    continue;
                else
                    break;
            }
            g_pvrfpSend = fopen(saveName, "wb");
            if(NULL == g_pvrfpSend)
                continue;
            break;
        }while(1);

        //g_pvrfpSend = fopen("/tmp/medias/sda1/play.ts", "wb");
    }
    if (NULL == g_pvrfpSend)
    {
        g_pvrfpSend = fopen("/tmp/medias/sdb1/play.ts", "wb");
    }
#endif

    ret = MT_MPI_AVPLAY_GetWindowHandle(pChnAttr->hAvplay, &hWindow);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY get window handle failed!\n");
        return MT_NULL;
    }
    while (!pChnAttr->bPlayMainThreadStop)
    {
        /* read file to the end, so wait until new comman incomming */
        if (pChnAttr->bEndOfFile && !pChnAttr->bQuickUpdateStatus)
        {
            MT_USLEEP(10000);
            continue;
        }
		//these code try to optimize FB function.
//        if(pChnAttr->bTillStartOfFile && (MT_UNF_PVR_PLAY_STATE_FB == pChnAttr->enState || MT_UNF_PVR_PLAY_STATE_STEPB== pChnAttr->enState))
//        {
//            MT_USLEEP(10000);
//            continue;
//        }

        if(!((MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState) ||
             (MT_UNF_PVR_PLAY_STATE_FB == pChnAttr->enState)))
        {
            if((MT_PVR_FETCH_RESULT_S *)NULL != pPvrFetchRes)
            {
                MT_FREE(MT_ID_PVR, pPvrFetchRes);
                pPvrFetchRes = (MT_PVR_FETCH_RESULT_S *)NULL;
            }

            if((MT_PVR_SEND_RESULT_S *)NULL != pstSendFrame)
            {
                MT_FREE(MT_ID_PVR, pstSendFrame);
                pstSendFrame = (MT_PVR_SEND_RESULT_S *)NULL;
            }
        }

        PVR_LOCK_PLAY(pChnAttr);

        if((MT_UNF_PVR_PLAY_STATE_PLAY<=pChnAttr->enState)&&
           (MT_UNF_PVR_PLAY_STATE_STOP>pChnAttr->enState)){//check es buffer! if more than 3/5,pause push.
            MT_UNF_AVPLAY_STATUS_INFO_S playinfo;

            if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO != pChnAttr->IndexHandle->enIndexType){
              if(first_run && pChnAttr->IndexHandle && pChnAttr->IndexHandle->bIsRec){//check frame num.Bug 111521
                #if 1
                  MT_U32 u32StartFrmNum = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
                  MT_U32 u32EndFrmNum = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
                  MT_U32 u32LastFrmNum = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
                  MT_U32 difftime=0;
                  PVR_INDEX_ENTRY_S start_idx,tail_idx;
                  PVR_EVENT_D("u32StartFrmNum = %d  u32EndFrmNum=%d   %d\n",u32StartFrmNum,u32EndFrmNum,u32LastFrmNum);
                  if(0==u32LastFrmNum){
                      PVR_UNLOCK_PLAY(pChnAttr);
                      MT_INFO_PVR("+++no frame\n");
                      MT_USLEEP(200*1000);   //sleep 500ms
                      continue;
                  }
                  if(0==u32EndFrmNum){
                      u32EndFrmNum=u32LastFrmNum-1;
                  }else{
                      u32EndFrmNum=u32EndFrmNum-1;
                  }
                  PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &start_idx, u32StartFrmNum);
                  PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &tail_idx, u32EndFrmNum);
                  difftime=tail_idx.u32DisplayTimeMs-start_idx.u32DisplayTimeMs;
                  MT_INFO_PVR("+++start=%d,%d,tail=%d,%d,diff=%d\n",u32StartFrmNum,start_idx.u32PtsMs,
                                                               u32EndFrmNum,tail_idx.u32PtsMs,difftime);
                  if(difftime < 5000){    //wait 5s
                      PVR_UNLOCK_PLAY(pChnAttr);
                      MT_USLEEP(200*1000);   //sleep 200ms
                      continue;
                  }else{
                      first_run=MT_FALSE;
                      MT_INFO_PVR("+++run really\n");
                  }
                #endif
              }
            }

            if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == pChnAttr->IndexHandle->enIndexType){
                if(MT_SUCCESS==MT_UNF_AVPLAY_GetAudioStatusInfo(pChnAttr->hAvplay, &playinfo)){
                if(playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize>(50*1024)){//125662
                    PVR_UNLOCK_PLAY(pChnAttr);
                    //printf("+++will full\n");
                    MT_USLEEP(4000);
                    continue;
                }
              }
            }else{
                if(MT_UNF_PVR_PLAY_STATE_PLAY==pChnAttr->enState){
                    MT_U32 pushtime=0;
                    //ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &frame_tmp,pChnAttr->IndexHandle->u32ReadFrame);
                    frame_gettimelatesttime(pChnAttr, &pushtime, 1);
                    if(0!=pushtime){ //If Iframe.check it
                        //printf("++diff=%d\n",(pushtime-pChnAttr->u32CurPlayTimeMs));
                        if((pushtime > pChnAttr->u32CurPlayTimeMs)&&
                           (pushtime-pChnAttr->u32CurPlayTimeMs)>3000){//sync make time not change. so and judge size
                            ret_a=MT_UNF_AVPLAY_GetAudioStatusInfo(pChnAttr->hAvplay, &playinfo_a);
                            //printf("++1 %x,%x\n",ret_a,ret_v);
                            if(MT_SUCCESS==ret_a){
                                //aes more than 64k,wait
                                //printf("++2 %x\n",playinfo_a.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize);
                                if(playinfo_a.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize > (64*1024)){
                                    //PVR_PLAY_P("++2 %x\n",playinfo_a.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize);
                                    PVR_UNLOCK_PLAY(pChnAttr);
                                    MT_USLEEP(8000);
                                    continue;
                                }
                            }
                            ret_v=MT_UNF_AVPLAY_GetVideoStatusInfo(pChnAttr->hAvplay, &playinfo_v);
                            if(MT_SUCCESS==ret_v){  //ves more than 4/5*total,wait
                                if((playinfo_v.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize*5)>(playinfo_v.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize*4)){
                                    PVR_UNLOCK_PLAY(pChnAttr);
                                    MT_USLEEP(8000);
                                    //printf("++o3\n");
                                    //PVR_PLAY_P("++o3 %x\n",playinfo_v.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize);
                                    //pvr_push_trace(1,'\)');
                                    continue;
                                }
                            }
                        }
                    }
                }else if(MT_SUCCESS==MT_UNF_AVPLAY_GetVideoStatusInfo(pChnAttr->hAvplay, &playinfo)){
                    if((playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32UsedSize*5)>(playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].u32BufSize*4)){
                        PVR_UNLOCK_PLAY(pChnAttr);
                        //printf("++o4\n");
                        MT_USLEEP(5000);
                        continue;
                    }
                }
            }
        }
        ret = PVRPlayCheckIfTsOverByRec(pChnAttr);
        if (MT_SUCCESS != ret){
            PVR_UNLOCK_PLAY(pChnAttr);
            continue;
        }

        pChnAttr->bTsBufReset = MT_FALSE;
        pChnAttr->bTillStartOfFile = MT_FALSE;
        pChnAttr->data_send_over = MT_FALSE;
        if(pChnAttr->play_errcode){  //read data err
            PVRPlayCheckError(pChnAttr,pChnAttr->play_errcode);
            pChnAttr->play_errcode=0;
        }

        switch (pChnAttr->enState)
        {
            case MT_UNF_PVR_PLAY_STATE_PLAY:
            case MT_UNF_PVR_PLAY_STATE_SF:
            case MT_UNF_PVR_PLAY_STATE_STEPF:
            {
                /* noexistent index file, read fixed size per-time*/
                if (pChnAttr->bPlayingTsNoIdx || !pChnAttr->stUserCfg.bIsClearStream)
                {
                    if (pChnAttr->u64CurReadPos >= pChnAttr->u64TsFileSize)
                    {
                        ret = MT_ERR_PVR_FILE_TILL_END;
                    }
                    else
                    {
                        frame.u16UpFlowFlag = 0;
                        frame.u64Offset = pChnAttr->u64CurReadPos;
                        frame.u32FrameSize = PVR_FIFO_WRITE_BLOCK_SIZE;
                    }
                }
                else
                {
                    /* normal play, not need to read data from the first I frame, send all the data is ok */
                    if ((MT_FALSE == pChnAttr->bNotAvailableTsBuff) || (MT_TRUE == pChnAttr->bQuickUpdateStatus))
                    {
                        if(MT_UNF_PVR_PLAY_STATE_STEPF==pChnAttr->enState){//avoid overflow
                            MT_USLEEP(1000*1000);
                            ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);
                        }else{
                            //PVR_PLAY_D("+++read:%d\n",pChnAttr->IndexHandle->u32ReadFrame);
                            ret = PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &frame);
                        }
                        if (MT_SUCCESS == ret)
                        {
                            bLastFrameSent = MT_FALSE;
                        }else if(MT_ERR_PVR_FILE_CANT_READ == ret){
                            pChnAttr->play_errcode=ret;
                        }
                    }

                    if (!PVRPlayIsTsSaved(pChnAttr, &frame))
                    {
                        ret = MT_ERR_PVR_FILE_TILL_END;
                    }
                }
                pChnAttr->bQuickUpdateStatus = MT_FALSE;
                break;
            }

            case MT_UNF_PVR_PLAY_STATE_FF:
            {
                pChnAttr->bQuickUpdateStatus = MT_FALSE;

                if ((MT_TRUE == pChnAttr->set_eof)
					&& (MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD != pChnAttr->enSpeed))
				{
					ret = MT_ERR_PVR_FILE_TILL_END;
					break;
				}

               /* noexistent index file, read fixed size per-time*/
                if (pChnAttr->bPlayingTsNoIdx)
                {
                    frame.u16UpFlowFlag = 0;
                    frame.u64Offset = pChnAttr->u64CurReadPos;
                    frame.u32FrameSize = PVR_FIFO_WRITE_BLOCK_SIZE;
                }
                else
                {
                    if ((MT_FALSE == pChnAttr->bNotAvailableTsBuff) || (MT_TRUE == pChnAttr->bQuickUpdateStatus))
                    {
                        if(pChnAttr->enSendDataMode == PVR_PLAY_SEND_DATA_ALL){
                            ret = PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &frame);
                      }else if(pChnAttr->enSendDataMode == PVR_PLAY_SEND_DATA_IP){
                            ret = PVR_Index_GetNextIPFrame(pChnAttr->IndexHandle, &frame);
                      }else if(pChnAttr->enSendDataMode == PVR_PLAY_SEND_DATA_I){
                           if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                               (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){
                                if(FRAME_TOP_FIELD==pChnAttr->fieldtype){
                                    ret = get_next_field_slice(pChnAttr,&frame_nouse,&frame,pChnAttr->u32FrameRate_fromav);
                                    //printf("+++pts.findp=%x,%x,%x,%d\n",ret,frame_nouse.u32PtsMs,frame.u32PtsMs,frame.u32PtsMs-frame_nouse.u32PtsMs);
                                    pChnAttr->fieldtype=FRAME_BOT_FIELD;
                                }else{
                                    ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);
                                    //printf("+++pts.findi=%x\n",frame.u32PtsMs);
                                    pChnAttr->fieldtype=FRAME_TOP_FIELD;
                                    frame_nouse=frame;
                                }
                            }else{
                                ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);
                            }
                        }else if(pChnAttr->enSendDataMode == PVR_PLAY_SEND_DATA_SKIP_I){
                            MT_U32 i,j,skipnum;
                            pre_readframe = pChnAttr->IndexHandle->u32ReadFrame; //backup the read frame
                            if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                               (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){
                                if(FRAME_TOP_FIELD==pChnAttr->fieldtype){
                                    ret = get_next_field_slice(pChnAttr,&frame_nouse,&frame,pChnAttr->u32FrameRate_fromav);
                                    //printf("+++pts.find=%x,%x,%d\n",frame_nouse.u32PtsMs,frame.u32PtsMs,frame.u32PtsMs-frame_nouse.u32PtsMs);
                                    pChnAttr->fieldtype=FRAME_BOT_FIELD;
                                }else{  //skip x*n+y
                                    //printf("++f.skip0=%d\n",pChnAttr->u32SkipPCnt_EveryI);
                                    pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                    for(i = 0; i < pChnAttr->u32SkipPCnt_EveryI; i++){
                                        ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);

                                        skipnum=pChnAttr->u32PushAddPoint/100;
                                        //printf("++skip %d %d ap = %d more %d\n",__LINE__,skipnum,pChnAttr->u32PushAddPoint,pChnAttr->u32SkipPMore_EveryxI);
                                        pChnAttr->u32PushAddPoint=pChnAttr->u32PushAddPoint%100;
                                        //printf("++f.skip.more %d\n",skipnum);
                                        for(j=0; j<skipnum; j++){
                                            //printf("++f.skip.sub %d\n",skipnum);
                                            ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);
                                            //pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                        }
                                    }
                                    frame_nouse=frame;
                                    pChnAttr->fieldtype=FRAME_TOP_FIELD;
                                }
                            }else{      //skip x*n+y
                                //printf("++f.skip1=%d\n",pChnAttr->u32SkipPCnt_EveryI);
                                pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                for(i = 0; i < pChnAttr->u32SkipPCnt_EveryI; i++){
                                    ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);

                                    skipnum=pChnAttr->u32PushAddPoint/100;
                                    //printf("++skip %d %d ap = %d more %d\n",__LINE__,skipnum,pChnAttr->u32PushAddPoint,pChnAttr->u32SkipPMore_EveryxI);
                                    pChnAttr->u32PushAddPoint=pChnAttr->u32PushAddPoint%100;
                                    for(j=0; j<skipnum; j++){
                                        //printf("++f.skip.sub %d\n",skipnum);
                                        ret = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &frame);
                                        //pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                    }
                                }                                
                            }

                            if(ret == MT_ERR_PVR_FILE_TILL_END)
                            {
                                //PVR_PLAY_P("ret=0x%x, cur and next is same: S:%d, E:%d, L:%d, R:%d\n",ret, pChnAttr->IndexHandle->stCycMgr.u32StartFrame, pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
                                //                pChnAttr->IndexHandle->stCycMgr.u32LastFrame, pChnAttr->IndexHandle->u32ReadFrame);
                                //maybe rec end frame not enough for skip,wait next time,revert the read frame
                                pChnAttr->IndexHandle->u32ReadFrame = pre_readframe;    
                            }
                        }
                        if (MT_SUCCESS == ret)
                        {
                            bLastFrameSent = MT_FALSE;
                        }
                    }

                    if (!PVRPlayIsTsSaved(pChnAttr, &frame))
                    {
                        ret = MT_ERR_PVR_FILE_TILL_END;
                    }
                }
                break;
            }

            case MT_UNF_PVR_PLAY_STATE_FB:
            {
                pChnAttr->bQuickUpdateStatus = MT_FALSE;

                /* noexistent index file, read fixed size per-time*/
                if (pChnAttr->bPlayingTsNoIdx)
                {
                    frame.u16UpFlowFlag = 0;
                    frame.u64Offset = pChnAttr->u64CurReadPos;
                    frame.u32FrameSize = PVR_FIFO_WRITE_BLOCK_SIZE;
                }
                else
                {
 #if 1
                    if ((MT_FALSE == pChnAttr->bNotAvailableTsBuff) || (MT_TRUE == pChnAttr->bQuickUpdateStatus))
                    {
                        if(pChnAttr->enSendDataMode == PVR_PLAY_SEND_DATA_I){
                            if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                               (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){
                                if(FRAME_TOP_FIELD==pChnAttr->fieldtype){
                                    PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &frame_nouse);
                                    ret = get_next_field_slice(pChnAttr,&frame_nouse,&frame,pChnAttr->u32FrameRate_fromav);
                                    pChnAttr->fieldtype=FRAME_BOT_FIELD;
                                    PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame_nouse);
                                }else{
                                    ret = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame);
                                    pChnAttr->fieldtype=FRAME_TOP_FIELD;
                                }
                            }else{
                                ret = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame);
                            }
                        }
                        else if(pChnAttr->enSendDataMode == PVR_PLAY_SEND_DATA_SKIP_I)
                        {
                            MT_U32 i,j,skipnum;
                            if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                               (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){
                                if(FRAME_TOP_FIELD==pChnAttr->fieldtype){
                                    PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &frame_nouse);
                                    ret = get_next_field_slice(pChnAttr,&frame_nouse,&frame,pChnAttr->u32FrameRate_fromav);
                                    pChnAttr->fieldtype=FRAME_BOT_FIELD;
                                    PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame_nouse);
                                }else{
                                    //printf("++r.skip0=%d\n",pChnAttr->u32SkipPCnt_EveryI);
                                    pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                    for(i = 0; i < pChnAttr->u32SkipPCnt_EveryI; i++){
                                        ret = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame);
                                        //Iframe be jump. consider I itself

                                        skipnum=pChnAttr->u32PushAddPoint/100;
                                        pChnAttr->u32PushAddPoint=pChnAttr->u32PushAddPoint%100;
                                        //printf("++r.skip.more %d\n",skipnum);
                                        for(j=0; j<skipnum; j++){
                                            ret = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame);
                                            //pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                        }
                                    }
                                    pChnAttr->fieldtype=FRAME_TOP_FIELD;
                                }
                            }
                            else
                            {
                                //printf("++r.skip1=%d\n",pChnAttr->u32SkipPCnt_EveryI);
                                pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                for(i = 0; i < pChnAttr->u32SkipPCnt_EveryI; i++){
                                    ret = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame);
                                    //Iframe be jump. consider I itself

                                    skipnum=pChnAttr->u32PushAddPoint/100;
                                    pChnAttr->u32PushAddPoint=pChnAttr->u32PushAddPoint%100;
                                    //printf("++r.skip.more %d\n",skipnum);
                                    for(j=0; j<skipnum; j++){
                                        ret = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &frame);
                                        //pChnAttr->u32PushAddPoint+=pChnAttr->u32SkipPMore_EveryxI;
                                    }
                                }
                            }
                        }
                        if (MT_SUCCESS == ret)
                        {
                            bLastFrameSent = MT_FALSE;
                        }
                    }
#endif
                }
                break;
            }

            case MT_UNF_PVR_PLAY_STATE_PAUSE:
            {
                pChnAttr->bQuickUpdateStatus = MT_FALSE;
                PVR_UNLOCK_PLAY(pChnAttr);
                (MT_VOID)MT_USLEEP(10000);
                continue;
            }

            case MT_UNF_PVR_PLAY_STATE_STEPB:
            {
                pChnAttr->bQuickUpdateStatus = MT_FALSE;
                pChnAttr->bEndOfFile = MT_FALSE;
                MT_ERR_PVR("not support status.\n");
                PVR_UNLOCK_PLAY(pChnAttr);
                (MT_VOID)MT_USLEEP(10000);
                continue;
            }

            default:
            {
                MT_INFO_PVR("Stop or invalid status: State=%d\n", pChnAttr->enState);
                PVR_UNLOCK_PLAY(pChnAttr);
                MT_USLEEP(10000);
                continue;
            }
        } /* end switch */


        if (MT_SUCCESS == ret)  /* read index OK  */
        {
            pChnAttr->bEndOfFile = MT_FALSE;
            pChnAttr->u64CurReadPos = frame.u64Offset + (MT_U64)frame.u32FrameSize;

        }
        else /* read index error */
        {
            /* on normally playing, catch up the record, wait a momnet, and then continue, the wait time up to content size of TS buffer */
            if (PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName)
                && ((MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)
                || (MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState)
                || (MT_UNF_PVR_PLAY_STATE_SF == pChnAttr->enState))
                && (MT_ERR_PVR_FILE_TILL_END == ret))
            {
               ;
            }
            else
            {
                MT_ERR_PVR("read idx err:%#x, Name %s, Save %d, State %d\n",
					ret,
					pChnAttr->stUserCfg.szFileName,
					PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName),
					pChnAttr->enState);
            }
            
            if(ret == MT_ERR_PVR_FILE_TILL_END || ret == MT_ERR_PVR_FILE_TILL_START){
                pChnAttr->data_send_over = MT_TRUE;
            }
        }

        //PVR_UNLOCK_PLAY(pChnAttr);

        if (pChnAttr->bPlayMainThreadStop)
        {
            PVR_UNLOCK_PLAY(pChnAttr);
            break;
        }
        {
            if(MT_SUCCESS == ret)
            {
                //PVR_LOCK_PLAY(pChnAttr);
                if (MT_FALSE == pChnAttr->bTsBufReset)
                {
                    if (!((MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState)
						&& (MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD != pChnAttr->enSpeed)
						&& (MT_TRUE == pChnAttr->set_eof)))
                    {
                        ret_sent = PVRPlaySendAframe_dealframe(pChnAttr, &frame);
                        if ((MT_SUCCESS != ret_sent)&&(PVR_CACHE_FRAME_ERROR != ret_sent))
                        {
                            if ((MT_ERR_PVR_PLAY_INVALID_TSBUFFER == ret_sent) ||
                               (MT_ERR_PVR_PLAY_INVALID_PACKETBUFFER == ret_sent))
                            {
                                 pChnAttr->bNotAvailableTsBuff = MT_TRUE;
                                PVR_UNLOCK_PLAY(pChnAttr);
                                MT_USLEEP(4000);
                                continue;
                            }
                            else
                            {
                                MT_ERR_PVR("======== send a frame err:%x ==========\n", ret_sent);
                            }
                        }

                        pChnAttr->bNotAvailableTsBuff = MT_FALSE;
                        pChnAttr->IndexHandle->u32PlayFrame = pChnAttr->IndexHandle->u32ReadFrame;
                        bLastFrameSent = MT_TRUE;
                    }
                }
                //PVR_UNLOCK_PLAY(pChnAttr);

                /* send all the frame case, send one frame wait a moment, so that make a sleep for other schedule */
                if (MT_SUCCESS == ret_sent){    //release cpu to other thread
                    static MT_U32 counter=0;

                    if((counter&63) == 63){ //every 1s,run onece
                        if((0xffffffff==pChnAttr->u32FrameRate_fromav) && (MT_UNF_PVR_PLAY_STATE_PLAY==pChnAttr->enState)){
                            MT_UNF_AVPLAY_STREAM_INFO_S streaminfo;
                            ret0=MT_UNF_AVPLAY_GetStreamInfo(pChnAttr->hAvplay,&streaminfo);
                            if((MT_SUCCESS==ret0)&&(0!=streaminfo.stVidStreamInfo.u32fpsInteger)){
                                pChnAttr->u32FrameRate_fromav=streaminfo.stVidStreamInfo.u32fpsInteger;
                                MT_INFO_PVR("+++get.framerate=%d,%d,%d\n",counter,streaminfo.stVidStreamInfo.u32fpsInteger,
                                  streaminfo.stVidStreamInfo.bProgressive);
                            }
                        }
                    }
                    counter++;
                    PVR_UNLOCK_PLAY(pChnAttr); //sleep after lock
                    if(1==pChnAttr->play_real_pushed){
                        if((MT_UNF_PVR_REC_VMX == pChnAttr->stUserCfg.bSupportAdvCa)||
                           (MT_UNF_PVR_REC_COMMON_CRAMBLE == pChnAttr->stUserCfg.bSupportAdvCa)){
                            MT_USLEEP(4000);            //sleep 4ms
                        }else{
                            MT_USLEEP(8000);  //sleep 8ms
                        }
                        pChnAttr->play_real_pushed=0;
                    }
                }else{
                    PVR_UNLOCK_PLAY(pChnAttr); //sleep after lock
                    if(PVR_CACHE_FRAME_ERROR == ret_sent){
                        ret_sent=MT_SUCCESS;
                    }else{
                        MT_USLEEP(15000);
                    }
                }
            }
            else
            {
                int flag_catch_tail=0;
                if(MT_FALSE==pChnAttr->IndexHandle->bIsRec){//check playback idx,this for event playing
                    MT_U8 headermem[64]={0};
                    PVR_IDX_HEADER_INFO_S *fhead=(PVR_IDX_HEADER_INFO_S*)headermem;
                    MT_S32 tret=PVRIndexGetHeaderInfoByName((MT_U8*)pChnAttr->IndexHandle->szIdxFileName,fhead,64);
                    //PVR_EVENT_D("++reload idx head=%s\n",pChnAttr->IndexHandle->szIdxFileName);
                    if(MT_SUCCESS==tret){
                        PVR_CYC_MGR_S *cycle_working=&pChnAttr->IndexHandle->stCycMgr;
                        if(0 > pChnAttr->enSpeed){          //if FB, exit
                            flag_catch_tail=1;
                        }else{
                            if(fhead->stCycInfo.u32EndFrame != cycle_working->u32EndFrame){ //updating idx
                                if(MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD<pChnAttr->enSpeed){
                                    PVR_INDEX_ENTRY_S tmpf;
                                    MT_U32 u32ReadFrame=pChnAttr->IndexHandle->u32ReadFrame;
                                    ret0 = PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &tmpf);
                                    pChnAttr->IndexHandle->u32ReadFrame=u32ReadFrame;
                                    if(MT_SUCCESS!=ret0){
                                        flag_catch_tail=1;
                                        PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &tmpf);
                                        printf("++no I frame ,so exit,ret0=%x\n",ret0);
                                    }
                                }

                                if(0==flag_catch_tail){
                                    cycle_working->u32StartFrame = fhead->stCycInfo.u32StartFrame;
                                    cycle_working->u32EndFrame = fhead->stCycInfo.u32EndFrame;
                                    cycle_working->u32LastFrame = fhead->stCycInfo.u32LastFrame;
                                    cycle_working->u64MaxCycSize = fhead->u64ValidSize;
                                    pChnAttr->IndexHandle->UpdatingEvent_CheckTimes=31;
                                    ret=MT_SUCCESS;
                                }
                            }
                        }
                    }
                    if(MT_SUCCESS!=ret){//no updating idx
                        if((pChnAttr->IndexHandle->UpdatingEvent_CheckTimes)&&(0==flag_catch_tail)){
                            pChnAttr->IndexHandle->UpdatingEvent_CheckTimes--;
                            ret=MT_SUCCESS;
                        }
                        //PVR_EVENT_D("+++read header error\n");
                    }
                }else{
                    ;//PVR_EVENT_D("+++timeshift end\n");
                }
                PVR_UNLOCK_PLAY(pChnAttr);
                if((pChnAttr->IndexHandle->UpdatingEvent_CheckTimes)&&(0==flag_catch_tail)){ //need to recheck
                    PVRPLAY_LOG_PLAY_OVER("+++updating check=%x\n",pChnAttr->IndexHandle->UpdatingEvent_CheckTimes);
                    ret=MT_SUCCESS;
                    MT_USLEEP(100000);
                }
            }
        }

        if (MT_SUCCESS != ret)
        {
            //printf("++=ret0=%x\n",ret);
            if (MT_ERR_PVR_FILE_TILL_START == ret){
                pChnAttr->bTillStartOfFile = MT_TRUE;
            }else{
                pChnAttr->bTillStartOfFile = MT_FALSE;
            }

            if(MT_TRUE==pChnAttr->IndexHandle->bIsRec){//(PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName)){//recording.
                if (MT_ERR_PVR_FILE_TILL_END == ret) /* NO EOF when play speed < 1x */
                {
                    bCallBack = MT_FALSE;
                    ret = MT_SUCCESS;

                    if(MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState)
                    {//check reach end by time
                        MT_U32 playtime=0;
                        /* 2X */
                        static MT_U32 static_playtime=0;
                        //static MT_U32 same_playtime=0;
                        static MT_U32 time_play=0;
                        static MT_U32 time_check=0;
                        /* 2X */

                        /* not 2X */
                        static MT_U32 static_playtime_not_2x = 0;
                        static MT_U32 time_play_not_2x = 0;
                        static MT_U32 time_check_not_2x = 0;
                        static MT_U32 got_last_vid_frame_decoded = MT_FALSE;
                        MT_U32 playtime_to_delay = 3000; //3000ms
                        /* not 2X */


                        MT_U32 latestdts_push[PVR_MAX_PTS_LEN];
                        MT_U32 latestdts_index = 0;
                        MT_U32 latestdts_len = sizeof(latestdts_push) / sizeof(latestdts_push[0]);
                        MT_U32 ret_dts = MT_FAILURE;
                        //PVRPLAY_LOG_PLAY_OVER("+++play.1.1\n");
                        if (MT_TRUE == PVRPlayCheckFFTillEnd(pChnAttr,&playtime,1)){
                            PVRPLAY_LOG_PLAY_OVER("+++play.1.2\n");
                            bCallBack = MT_TRUE;
                            ret = MT_ERR_PVR_FILE_TILL_END;
                        }
                        else if(MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD == pChnAttr->enSpeed)
                        {
                            memset(&latestdts_push, 0x00, sizeof(latestdts_push));
                            ret_dts = frame_gettimelatesttime(pChnAttr, latestdts_push, latestdts_len);
                            if (MT_SUCCESS == ret_dts)
                            {
                                for (latestdts_index = 0; latestdts_index < latestdts_len; latestdts_index++)
                                {
                                    if (playtime == latestdts_push[latestdts_index])
                                    {
                                        break;
                                    }
                                }

                                if (latestdts_index != latestdts_len)
                                {
                                    bCallBack = MT_TRUE;
                                    ret = MT_ERR_PVR_FILE_TILL_END;
                                }
                            }

                            if (MT_TRUE != bCallBack)
                            {
                                if(static_playtime == playtime)
                                {
                                    MT_PVR_SysGetTimeStampMs(&time_check);
                                }
                                else
                                {
                                    MT_PVR_SysGetTimeStampMs(&time_play);
                                    static_playtime = playtime;
                                }

                                if((time_check > time_play) && ((time_check - time_play) > 500))    //0.5s
                                {
                                    bCallBack = MT_TRUE;
                                    ret = MT_ERR_PVR_FILE_TILL_END;
                                }
                            }
                        }
                        else
                        {
                            //PVRPLAY_LOG_PLAY_OVER("+++play.1.3 set_eof=%d\n",pChnAttr->set_eof);
                            //fixed for CSTM_Bug #24980
                            MT_U32 diff = 0,time_gap = 0;
                            //time_gap= PVRPlayFFEndTimeGap(pChnAttr)+10000;
                            //iframe may too few or iframe interval too large
                            if(pChnAttr->u32FrameRate_trick == 1 && pChnAttr->IndexHandle->iframe_interal_max >= 5000){
                                time_gap = (abs(pChnAttr->enSpeed) / MT_UNF_PVR_PLAY_SPEED_NORMAL)*1000;
                                if(pChnAttr->IndexHandle->iframe_interal_max > time_gap*2){
                                    time_gap = pChnAttr->IndexHandle->iframe_interal_max;
                                }else{
                                    time_gap = time_gap*2;
                                }
                            }else{
                                time_gap = (abs(pChnAttr->enSpeed) / MT_UNF_PVR_PLAY_SPEED_NORMAL)*1000 + 2000; 
                            }
                            
                            if(playtime>pChnAttr->IndexHandle->Latest_DisplayTimeMs){
                                diff=playtime-pChnAttr->IndexHandle->Latest_DisplayTimeMs;
                            }else{
                                diff=pChnAttr->IndexHandle->Latest_DisplayTimeMs-playtime;
                            }

                            #if 0
                            if(MT_FALSE == pChnAttr->set_eof){
                                PVR_PLAY_D("====diff=%d time_gap=%d playtime=%d Latest_DisplayTimeMs=%d u32FrameRate_trick=%d iframe_interval=%d\n",diff,time_gap,
                                    playtime,pChnAttr->IndexHandle->Latest_DisplayTimeMs,pChnAttr->u32FrameRate_trick,pChnAttr->IndexHandle->iframe_interal_max);
                            }
                            #endif
                            if (MT_FALSE == pChnAttr->set_eof && diff <= time_gap)
                            {
                                PVRPLAY_LOG_PLAY_OVER("----diff=%d time_gap=%d playtime=%d Latest_DisplayTimeMs=%d u32FrameRate_trick=%d iframe_interval=%d\n",diff,time_gap,
                                playtime,pChnAttr->IndexHandle->Latest_DisplayTimeMs,pChnAttr->u32FrameRate_trick,pChnAttr->IndexHandle->iframe_interal_max);
                                //repeat last frame
                                PVR_LOCK_PLAY(pChnAttr);
                                MT_U32 loopcnt = 3;
                                MT_U32 flag_withB = 0;
                                PVR_INDEX_ENTRY_S Iframe={0};
                                PVR_INDEX_ENTRY_S Bframe={0};
                                MT_U32 ret_repeat = MT_FAILURE;
                                MT_U32 u32ReadFrameBak = pChnAttr->IndexHandle->u32ReadFrame;


                                pChnAttr->IndexHandle->u32ReadFrame = g_pvr_last_i_frame;

                                ret_repeat = PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &Iframe);
                                if (MT_SUCCESS != ret_repeat)
                                {
                                    MT_ERR_PVR("PVR_Index_GetPreIFrame failed 0x%x\n", ret_repeat);
                                }

                                ret_repeat = PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &frame_nouse);
                                if (MT_SUCCESS != ret_repeat)
                                {
                                    MT_ERR_PVR("PVR_Index_GetNextFrame failed 0x%x\n", ret_repeat);
                                }
                                if (0 == get_next_field_slice(pChnAttr,&Iframe,&Bframe,pChnAttr->u32FrameRate_fromav))
                                {
                                    flag_withB=1;
                                }
                                PVRPLAY_LOG_PLAY_OVER("+++repush start++++, flag_withB %d\n", flag_withB);
                                while (loopcnt--){
                                    PVRPlaySendAframe(pChnAttr,&Iframe);
                                    if(flag_withB)
                                    {
                                        PVRPlaySendAframe(pChnAttr,&Bframe);
                                    }
                                    if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
                                        bCallBack = MT_FALSE;
                                        break;
                                    }
                                }

                                pChnAttr->IndexHandle->u32ReadFrame = u32ReadFrameBak;
                                PVRPLAY_LOG_PLAY_OVER("+++repush end++++\n");
                                PVR_UNLOCK_PLAY(pChnAttr);


                                pChnAttr->set_eof = MT_TRUE;
                                MT_UNF_AVPLAY_FlushStream(pChnAttr->hAvplay, MT_NULL);
                                //g_pvr_last_vid_frame_showed = MT_FALSE;
                                PVRPLAY_LOG_PLAY_OVER("+++call flush stream++++ g_pvr_last_vid_frame_showed=%d\n",g_pvr_last_vid_frame_showed);
                                
                            }

                            if (MT_TRUE == g_pvr_last_vid_frame_showed /*&& MT_TRUE == g_pvr_last_vid_frame_decoded*/)
                            {
                                PVRPLAY_LOG_PLAY_OVER("+++last video frame showed, set callback true+++\n");
                                bCallBack = MT_TRUE;
                                ret = MT_ERR_PVR_FILE_TILL_END;
                                g_pvr_last_vid_frame_showed = MT_FALSE;
                            }
                            /* timeout mechanism for displaying the last frame failure */
                            if (MT_TRUE != bCallBack)
                            {
                                if ((MT_TRUE == g_pvr_last_vid_frame_decoded)
                                    && (MT_FALSE == got_last_vid_frame_decoded))
                                {
                                    // backup first dts and time after getting MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED
                                    static_playtime_not_2x = playtime;
                                    MT_PVR_SysGetTimeStampMs(&time_play_not_2x);

                                    got_last_vid_frame_decoded = MT_TRUE;
                                }

                                if (MT_TRUE == got_last_vid_frame_decoded)
                                {
                                    MT_PVR_SysGetTimeStampMs(&time_check_not_2x);

                                    memset(&latestdts_push, 0x00, sizeof(latestdts_push));
                                    ret_dts = frame_gettimelatesttime(pChnAttr, latestdts_push, 1);
                                    if (MT_SUCCESS == ret_dts)
                                    {
                                        if((time_check_not_2x > time_play_not_2x)
                                            && ((time_check_not_2x - time_play_not_2x)
                                             > (((latestdts_push[0] - static_playtime_not_2x) / (abs(pChnAttr->enSpeed) / MT_UNF_PVR_PLAY_SPEED_NORMAL)) + playtime_to_delay)))	// dts gap 3s
                                        {
                                            bCallBack = MT_TRUE;
                                            ret = MT_ERR_PVR_FILE_TILL_END;
                                            PVR_ALAWYS_PRINT("[%s %d], got, to send EOF\n", __func__, __LINE__);
                                        }
                                    }
                                }
                            }
                        }

                        if (MT_TRUE == bCallBack)
                        {
                            //same_playtime = 0;
                            time_play = 0;
                            time_check = 0;
                            static_playtime = 0;

                            time_play_not_2x = 0;
                            time_check_not_2x = 0;
                            static_playtime_not_2x = 0;

                            got_last_vid_frame_decoded = MT_FALSE;
                        }
                        if(MT_ERR_PVR_FILE_TILL_END==ret)
                        {
                            pChnAttr->bEndOfFile = MT_TRUE;
                            PVRPLAY_LOG_PLAY_OVER("+++set bEndOfFile true+++\n");
                        }
                    }else{
                        //should sleep some time here for release CPU
                        //PVR_PLAY_P("=======================\n");
                        MT_USLEEP(8000);
                    }
                    //printf("+++play.1.4\n");
                }else if (MT_ERR_PVR_FILE_TILL_START == ret){
                    //printf("+++play.4\n");
                    if(0==pChnAttr->IndexHandle->stCycMgr.s32CycTimes){
                        PVRPLAY_LOG_PLAY_OVER("+++play.4.0\n");
                        goto LABEL_PLAYBACK;
                    }
                    MT_U32 s,l,cur,err_loop=0,same_cnt=0,cur_time=0,pre_time=0xffffffff,same_time_cnt=0;
                    MT_U32 distance=0,distance_old=0xffffffff;
                    MT_U32 distance_gap = 50;//default value
                   
                    while(MT_TRUE){
                        s=pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
                        l=pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
                        if(0==l){ //abnoraml
                            bCallBack = MT_TRUE;
                            break;
                        }
                        l--;

                        //printf("+++play.4.1\n");
                        if(MT_SUCCESS ==Frame_GetDispTimeByCache(pChnAttr,&cur,&cur_time,NULL,NULL)){

                            //add for check cur play time move not not
                            if(pre_time == 0xffffffff){
                                pre_time = cur_time;
                            }else if(cur_time == pre_time){
                                same_time_cnt ++;
                                //PVRPLAY_LOG_PLAY_OVER("+++ same_time_cnt=%d(%d,%d)\n",same_time_cnt,pre_time,cur_time);
                            }else if(cur_time != pre_time){
                                same_time_cnt = 0;
                                pre_time = cur_time;
                            }
                            err_loop=0;
                            distance=Pvr_Calc_distance(s,l,cur);
                            //PVRPLAY_LOG_PLAY_OVER("+++play.4.2 s=%d cur=%d distance=%d pre_time=%d cur_time=%d\n",s,cur,distance,pre_time,cur_time);
                            
                            if(distance<distance_gap){
                                PVRPLAY_LOG_PLAY_OVER("+++play.4.3 distance=%d distance_gap=%d\n",distance,distance_gap);
                                bCallBack = MT_TRUE;
                                break;
                            }
                            if((distance_old<distance)&&((distance-distance_old)>(l/2))){   //cross
                                PVRPLAY_LOG_PLAY_OVER("+++play.4.4.0 same_cnt=%d\n",same_cnt);
                                bCallBack = MT_TRUE;
                                break;
                            }else if(distance_old==distance){
                                same_cnt++;
                                PVRPLAY_LOG_PLAY_OVER("+++play.4.4.1 same_cnt=%d\n",same_cnt);
                            }else{
                                same_cnt=0;
                                distance_old=distance;
                            }

                            if(same_time_cnt >= 40){// 2s
                                PVRPLAY_LOG_PLAY_OVER("+++play.4.4.4 same_time_cnt=%d\n",same_time_cnt);
                                bCallBack = MT_TRUE;
                                break;
                            }
                        }else{
                            err_loop++;
                            PVRPLAY_LOG_PLAY_OVER("+++play.4.4.2 err_loop=%d\n",err_loop);
                        }
                        MT_USLEEP(50000);//MT_USLEEP(200000);
                        if((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
                            PVRPLAY_LOG_PLAY_OVER("+++play.4.5 err_loop=%d,same_cnt=%d\n",err_loop,same_cnt);
                            bCallBack = MT_FALSE;
                            break;
                        }
                        if(err_loop>60){//3s
                            PVRPLAY_LOG_PLAY_OVER("+++play.4.6 err_loop=%d\n",err_loop);
                            bCallBack = MT_TRUE;
                            break;
                        }
                        if(same_cnt>60){//3s
                            PVRPLAY_LOG_PLAY_OVER("+++play.4.7 same_cnt=%d\n",same_cnt);
                            goto LABEL_PLAYBACK;
                        }
                    }
                    pChnAttr->bEndOfFile = MT_TRUE;
                    PVRPLAY_LOG_PLAY_OVER("+++play.4.8\n");
                }else{
                    PVRPLAY_LOG_PLAY_OVER("+++play.4.9\n");
                    bCallBack = MT_TRUE;
                }
            }else{//play back
                LABEL_PLAYBACK:
                if(MT_ERR_PVR_FILE_TILL_START == ret){
                    PVR_LOCK_PLAY(pChnAttr);
                    {//repush head
                        MT_U32 loopcnt=5,flag_withB=0;
                        PVR_INDEX_ENTRY_S Iframe={0},Bframe={0};
                        #ifdef CHECKOVER_BY_DISPLAY
                        loopcnt=3;
                        #endif
                        pChnAttr->IndexHandle->u32ReadFrame=pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
                        PVR_Index_GetNextIFrame(pChnAttr->IndexHandle, &Iframe);
                        if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                               (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){
                            get_next_field_slice(pChnAttr,&Iframe,&Bframe,pChnAttr->u32FrameRate_fromav);
                            flag_withB=1;
                        }

                        PVRPLAY_LOG_PLAY_OVER("+++play.8.0.1 loopcnt= %d pChnAttr->u32FrameRate =%d pChnAttr->u32FrameRate_fromav =%d\n",loopcnt,pChnAttr->u32FrameRate,pChnAttr->u32FrameRate_fromav);
                        PVRPLAY_LOG_PLAY_OVER("Iframe offset =%lld size = %d pts = %d dpts = %d\n",Iframe.u64GlobalOffset,Iframe.u32FrameSize,Iframe.u32PtsMs,Iframe.u32DisplayTimeMs);
                        if(flag_withB)
                        {
                            PVRPLAY_LOG_PLAY_OVER("BFrem offset =%lld size = %d pts = %d dpts = %d\n",Bframe.u64GlobalOffset,Bframe.u32FrameSize,Bframe.u32PtsMs,Bframe.u32DisplayTimeMs);
                        }

                        if (MT_FALSE == pChnAttr->set_eof)
                        {
                            while(loopcnt--){
                                PVRPlaySendAframe(pChnAttr,&Iframe);
                                if(flag_withB){
                                    PVRPlaySendAframe(pChnAttr,&Bframe);
                                }
                                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){    
                                    PVRPLAY_LOG_PLAY_OVER("+++play.8.0.1 pChnAttr->bTillStartOfFile =%d pChnAttr->bQuickUpdateStatus= %d %d\n",pChnAttr->bTillStartOfFile,pChnAttr->bQuickUpdateStatus,loopcnt);                                  
                                    bCallBack = MT_FALSE;
                                    break;
                                }                             
                                PVRPLAY_LOG_PLAY_OVER("+++play.8.0 %d %d\n",__LINE__,loopcnt);                              
                            }
                            
                            //fixed #29481
                            if(!pChnAttr->bQuickUpdateStatus)
                            {
                                MT_UNF_AVPLAY_FlushStream(pChnAttr->hAvplay, MT_NULL);
                                PVR_PLAY_P("----------pvr set eos!-----------\n");
                                pChnAttr->set_eof = MT_TRUE;
                            }
                            else
                            {
                                PVR_PLAY_P("---------bQuickUpdateStatus is TRUE should not set eos!------------\n");
                            }
                            
                        }
                        PVR_UNLOCK_PLAY(pChnAttr);

                        //wait last frame decoded,add for CSTM_Bug #20795 case2 if u32VidBufSize set to 6M
                        loopcnt = 0;
                        while(MT_TRUE){
                            if(g_pvr_last_vid_frame_decoded){
                                PVRPLAY_LOG_PLAY_OVER("+++play.7.1.0 counter =%d %d\n",loopcnt,__LINE__);
                                break;
                            }else{
                                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                                    break;

                                loopcnt++;
                                if(loopcnt>3000)
                                {
                                    PVRPLAY_LOG_PLAY_OVER("+++play.7.1.1 counter =%d %d\n",loopcnt,__LINE__);
                                    break;
                                }
                                MT_USLEEP(20*1000);
                            }
                        }

                        //wait last frame show 
                        loopcnt = 0;
                        while(MT_TRUE){
                            if(g_pvr_last_vid_frame_showed){
                                PVRPLAY_LOG_PLAY_OVER("+++play.7.0.0 counter =%d %d\n",loopcnt,__LINE__);
                                break;
                            }else{
                                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                                    break;

                                loopcnt++;
                                if(loopcnt>3000) {
                                    PVRPLAY_LOG_PLAY_OVER("+++play.7.0.1 counter =%d %d\n",loopcnt,__LINE__);
                                    break;
                                }
                                MT_USLEEP(20*1000);
                            }
                        }
                    }
                    PVRPLAY_LOG_PLAY_OVER("+++play.8.0 %d %x\n",__LINE__,ret);
                }else if(MT_ERR_PVR_FILE_TILL_END == ret){
                  if((PVR_PLAY_SEND_DATA_SKIP_I==pChnAttr->enSendDataMode)||
                     (PVR_PLAY_SEND_DATA_I==pChnAttr->enSendDataMode)){//repush end
                        MT_U32 loopcnt = 0,flag_withB=0;
                        if (!((MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState) && (MT_TRUE == pChnAttr->set_eof)))
                     	{
                            PVR_LOCK_PLAY(pChnAttr);
                            loopcnt=5;
                            flag_withB=0;
                            PVR_INDEX_ENTRY_S Iframe={0};
                            PVR_INDEX_ENTRY_S Bframe={0};
                            //printf("+++get frame=%x\n",pChnAttr->IndexHandle->u32ReadFrame);
                            PVR_Index_GetPreIFrame(pChnAttr->IndexHandle, &Iframe);
                            //printf("+++repush type=%d\n",PVR_INDEX_is_Iframe(&Iframe));
                            if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                                   (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1)))
                            {
                                PVR_Index_GetNextFrame(pChnAttr->IndexHandle, &frame_nouse);
                                get_next_field_slice(pChnAttr,&Iframe,&Bframe,pChnAttr->u32FrameRate_fromav);
                                flag_withB=1;
                            }
                            //printf("+++repush start++++\n");  //print
                            while(loopcnt--)
                            {
                                PVRPlaySendAframe(pChnAttr,&Iframe);
                                if(flag_withB)
                                {
                                    PVRPlaySendAframe(pChnAttr,&Bframe);
                                }
                                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                                {
                                    //printf("+++play.x.2\n");
                                    bCallBack = MT_FALSE;
                                    break;
                                }
                            }
                            MT_UNF_AVPLAY_FlushStream(pChnAttr->hAvplay, MT_NULL);
                            pChnAttr->set_eof = MT_TRUE;
                            PVRPLAY_LOG_PLAY_OVER("+++repush end++++\n");
                            PVR_UNLOCK_PLAY(pChnAttr);
                        }

                        //wait last frame decoded
                        loopcnt = 0;
                        while(MT_TRUE){
                            if(g_pvr_last_vid_frame_decoded){
                                PVRPLAY_LOG_PLAY_OVER("+++play.7.1.0 counter =%d %d\n",loopcnt,__LINE__);
                                break;
                            }else{
                                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                                    break;

                                loopcnt++;
                                if(loopcnt>3000)
                                {
                                    PVRPLAY_LOG_PLAY_OVER("+++play.7.1.1 counter =%d %d\n",loopcnt,__LINE__);
                                    break;
                                }
                                MT_USLEEP(20*1000);
                            }
                        }

                        //wait last frame show 
                        loopcnt = 0;
                        while(MT_TRUE){
                            if(g_pvr_last_vid_frame_showed){
                                PVRPLAY_LOG_PLAY_OVER("+++play.7.0.0 counter =%d %d\n",loopcnt,__LINE__);
                                break;
                            }else{
                                if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop))
                                    break;

                                loopcnt++;
                                if(loopcnt>3000) {
                                    PVRPLAY_LOG_PLAY_OVER("+++play.7.0.1 counter =%d %d\n",loopcnt,__LINE__);
                                    break;
                                }
                                MT_USLEEP(20*1000);
                            }
                        }
                        
                    }
                }

                PVRPLAY_LOG_PLAY_OVER("+++push over\n");
                //if(MT_FALSE==pChnAttr->IndexHandle->bIsRec)
                {//check es empty
                    MT_BOOL Eof=MT_FALSE;
                    mt_u32  counter=0;
                    while(MT_TRUE){
                        ret0=pvr_check_av_eof(pChnAttr,pChnAttr->hAvplay, &Eof);
                        if(MT_SUCCESS==ret0){
                            counter=0;
                            if(Eof){
                                PVRPLAY_LOG_PLAY_OVER("+++play.7.0.0\n");
                                break;
                            }
                        }else{
                            counter++;
                            if(counter>100){
                                PVRPLAY_LOG_PLAY_OVER("+++play.7.0.1 counter =%d %d\n",counter,__LINE__);
                                break;
                            }
                            MT_USLEEP(10000);
                        }
                        if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
                            PVRPLAY_LOG_PLAY_OVER("+++play.8.2.0\n");
                            bCallBack = MT_FALSE;
                            break;
                        }
                    }
                }
                #ifdef CHECKOVER_BY_DISPLAY
                if(MT_UNF_PVR_REC_INDEX_TYPE_VIDEO==pChnAttr->IndexHandle->enIndexType)
                {
                    PVRPLAY_LOG_PLAY_OVER("+++play start wait video bEndOfStream...\n");
                    mt_u32 r=0,counter=3000; //30s ,to check display over
                    MT_UNF_AVPLAY_STATUS_INFO_S pstStatusInfo;
                    while(counter--){
                        PVR_LOCK_PLAY(pChnAttr);// lock and check update status first for #20795
                        if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
                            PVRPLAY_LOG_PLAY_OVER("+++play.8.2.1\n");
                            bCallBack = MT_FALSE;
                            PVR_UNLOCK_PLAY(pChnAttr);
                            break;
                        }
                        PVR_UNLOCK_PLAY(pChnAttr);
                        r = MT_UNF_AVPLAY_GetVideoStatusInfo(pChnAttr->hAvplay, &pstStatusInfo);
                        if((0==r) && pstStatusInfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_VID].bEndOfStream){
                            PVRPLAY_LOG_PLAY_OVER("+++display: %d bEndOfStream=1\n",(3000-counter));
                            bCallBack = MT_TRUE;
                            break;
                        }

                        MT_USLEEP(10000);
                    }
                }
                else
                #endif
                {

                    if(MT_ERR_PVR_FILE_TILL_END == ret){
                        MT_S32 retthis=0;
                        MT_U32 playtime=0,timeslice=200000,waitcnt=0;
                        static MT_U32 static_playtime_plyback=0;
                        static MT_U32 same_playtime_plyback=0;
                        waitcnt=PVRPlayTime_CacBySpeed(pChnAttr,timeslice);
                        //printf("+++play.7 %x\n",waitcnt);
                        while(MT_TRUE){
                            retthis=PVRPlayCheckFFTillEnd(pChnAttr,&playtime,0);
                            if(MT_TRUE==retthis){
                                MT_USLEEP(500000);  //let disp show
                                //printf("+++play.7.1\n");
                                bCallBack = MT_TRUE;
                                break;
                            }
                            if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
                                //printf("+++play.7.2\n");
                                bCallBack = MT_FALSE;
                                break;
                            }
                            MT_USLEEP(timeslice);
                            if(static_playtime_plyback==playtime){
                                same_playtime_plyback++;
                            }else{
                                same_playtime_plyback=0;
                                static_playtime_plyback=playtime;
                            }
                            //printf("++play.7.3 %x\n",same_playtime_plyback);
                            if(same_playtime_plyback>waitcnt){ //same.frame_rate>=1
                                //printf("+++play.7.4 %x,%x\n",same_playtime_plyback,waitcnt);
                                bCallBack = MT_TRUE;
                                break;
                            }
                        }
                        pChnAttr->bEndOfFile = MT_TRUE;
                    }else if(MT_ERR_PVR_FILE_TILL_START == ret){
                        MT_S32 retthis=0;
                        MT_U32 playtime=0,timeslice=200000,waitcnt=0;
                        static MT_U32 static_playtime_plyback=0;
                        static MT_U32 same_playtime_plyback=0;
                        waitcnt=PVRPlayTime_CacBySpeed(pChnAttr,timeslice);
                        //printf("+++play.8.0 %d %d\n",waitcnt,__LINE__);
                        while(MT_TRUE){
                            retthis=PVRPlayCheckFBTillStartInRewind(pChnAttr,&playtime,0);
                            if(MT_TRUE==retthis){
                                MT_USLEEP(500000);  //let disp show
                                //printf("+++play.8.1 %d\n",__LINE__);
                                bCallBack = MT_TRUE;
                                break;
                            }
                            if ((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)){
                                //printf("+++play.8.2\n");
                                bCallBack = MT_FALSE;
                                break;
                            }
                            MT_USLEEP(timeslice);
                            if(static_playtime_plyback==playtime){
                                //printf("+++play.8.2.0 %x, %d\n",playtime,__LINE__);
                                same_playtime_plyback++;
                            }else{
                                same_playtime_plyback=0;
                                static_playtime_plyback=playtime;
                            }
                            //printf("++play.8.3 %x\n",same_playtime_plyback);
                            if(same_playtime_plyback>waitcnt){ //same.frame_rate>=1
                                //printf("+++play.8.4 %x,%x %d\n",same_playtime_plyback,waitcnt,__LINE__);
                                bCallBack = MT_TRUE;
                                break;
                            }
                        }
                        pChnAttr->bEndOfFile = MT_TRUE;
                    }
                }
            }

            /* callback by error */
            //printf("+++will cb %x,%x,%x\n",bCallBack,pChnAttr->bQuickUpdateStatus,pChnAttr->bPlayMainThreadStop);
            if ((bCallBack) && !((pChnAttr->bQuickUpdateStatus) || (pChnAttr->bPlayMainThreadStop)))
            {
//                MT_ERR_PVR("+++cb %x\n",ret);
                PVRPlayCheckError(pChnAttr, ret);
            }
        }

        ret = MT_SUCCESS;
    } /* end while */

    MT_INFO_PVR("<<----------PVRPlayMainRoute exit---------------\n");
    if (NULL != g_pvrfpSend)
    {
        fclose(g_pvrfpSend);

        g_pvrfpSend = NULL;
        PVR_PLAY_P("pvr close dump file.\n");
    }
    if ((MT_PVR_SEND_RESULT_S *)NULL != pstSendFrame)
    {
        MT_FREE(MT_ID_PVR, pstSendFrame);
        pstSendFrame = (MT_PVR_SEND_RESULT_S *)NULL;
    }
    if ((MT_PVR_FETCH_RESULT_S *)NULL != pPvrFetchRes)
    {
        MT_FREE(MT_ID_PVR, pPvrFetchRes);
        pPvrFetchRes = (MT_PVR_FETCH_RESULT_S *)NULL;
    }
    UNUSED(bLastFrameSent);

    #ifdef CONFIG_MT_LXC_SUPPORT
    mt_pvr_ipc_clear_tid();
    #endif
    return NULL;
}

/*the following case, the record can catch up to the live stream */
MT_BOOL PVR_Play_IsFilePlayingSlowPauseBack(const MT_CHAR *pFileName)
{
    MT_U32 i;
    MT_UNF_PVR_PLAY_STATE_E enPlayStatus;

    if(NULL == pFileName)
    {
        MT_ERR_PVR("\nInput pointer parameter is NULL!\n");
        return MT_FALSE;
    }

    for (i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        if  ( !strncmp(g_stPvrPlayChns[i].stUserCfg.szFileName, pFileName,strlen(pFileName)) )
        {
            break;
        }
    }
    if ( i ==  PVR_PLAY_MAX_CHN_NUM)
    {
        //MT_INFO_PVR("No PVRPlayChan Exist\n");
        return MT_FALSE;
    }

    enPlayStatus   = g_stPvrPlayChns[i].enState;

    /*two situation may cause rec catch play:
    1. paly pause or slow
    2. paly FB while rec rewind . add this situation for DTS2014052701682,
    for this  situation,we suggest customer check playtime and start entry disptime
    (inorder to wait the stream in demux/vdec/vfwm/vo buffer playing end),
    if playtime meet the start entry disptime ,resume the play channel*/
    /*For DTS2014052701682, Sync modify from V1R1*/
    if ( (enPlayStatus == MT_UNF_PVR_PLAY_STATE_PAUSE)
        || (enPlayStatus == MT_UNF_PVR_PLAY_STATE_SF)
        || (enPlayStatus == MT_UNF_PVR_PLAY_STATE_STEPB)
        || (enPlayStatus == MT_UNF_PVR_PLAY_STATE_STEPF) )
    {
        return MT_TRUE;
    }
    else if ( enPlayStatus == MT_UNF_PVR_PLAY_STATE_FB )
    {
        /*if alreadly rewind*/
        if (1)//( g_stPvrPlayChns[i].IndexHandle->stCycMgr.u32StartFrame >= g_stPvrPlayChns[i].IndexHandle->stCycMgr.u32EndFrame )
        {
            return MT_TRUE;
        }
    }

    return MT_FALSE;
}

MT_BOOL PVR_Play_IsPlaying(void)
{
    return g_stPlayInit.bInit;
}


/*****************************************************************************
 Prototype       : MT_PVR_PlayInit
 Description     : play module initializde
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/14
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_SetPlayThreadattr(MT_S32 type,MT_S32 schedpolicy, MT_S32 priority,MT_S32 stacksize)
{
    g_playthreadattr.schedpolicy=schedpolicy;
    g_playthreadattr.priority=priority;
    g_playthreadattr.stacksize=stacksize;
    g_playthreadattr.flag_valid=1;
    return MT_SUCCESS;
}
static void mt_pvr_init_playattr(PVR_PLAY_CHN_S *pChnAttr,mt_u32 chid)
{
    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INVALID;
    pChnAttr->enLastState = MT_UNF_PVR_PLAY_STATE_INVALID;
    pChnAttr->bPlayMainThreadStop = MT_TRUE;
    pChnAttr->u32chnID = chid;
    pChnAttr->s32DataFile = PVR_FILE_INVALID_FILE;
    pChnAttr->u64CurReadPos = 0;
    pChnAttr->IndexHandle = NULL;
    pChnAttr->hCipher = 0;
    pChnAttr->PlayStreamThread = 0;
    pChnAttr->bCAStreamHeadSent = MT_FALSE;
    pChnAttr->u64LastSeqHeadOffset = PVR_INDEX_INVALID_SEQHEAD_OFFSET;
    pChnAttr->readCallBack = NULL;
    memset(&pChnAttr->stUserCfg, 0, sizeof(MT_UNF_PVR_PLAY_ATTR_S));
    memset(&pChnAttr->stCipherBuf, 0, sizeof(PVR_PHY_BUF_S));
    memset(&pChnAttr->stSmoothPara, 0, sizeof(PVR_SMOOTH_PARA_S));
    pChnAttr->stSmoothPara.enBackwardLastSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
    pChnAttr->stSmoothPara.enSmoothLastSpeed = MT_UNF_PVR_PLAY_SPEED_BUTT;
    pChnAttr->bTimeShiftStartFlg = MT_FALSE;
    pChnAttr->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
    pChnAttr->enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
}
MT_S32 MT_PVR_PlayInit(MT_VOID)
{
    MT_U32 i=0,j=0;
    MT_S32 ret;
    PVR_PLAY_CHN_S *pChnAttr;
#ifdef PVR_PROC_SUPPORT
    //MT_U32 u32CurPid = getpid();
    static MT_CHAR pProcDirName[32] = {0};
#endif

    if (MT_TRUE == g_stPlayInit.bInit)
    {
        MT_WARN_PVR("Play Module has been Initialized!\n");
        return MT_SUCCESS;
    }
    else
    {
        /*initialize whole of  index */
        PVR_Index_Init();

        ret = PVRPlayDevInit();
        if (MT_SUCCESS != ret)
        {
            return ret;
        }

        ret = PVRIntfInitEvent();
        if (MT_SUCCESS != ret)
        {
            close(g_s32PvrFd);
            return ret;
        }

        /* set all play channel as INVALID status                            */
        for (i = 0 ; i < PVR_PLAY_MAX_CHN_NUM; i++)
        {
            pChnAttr = &g_stPvrPlayChns[i];
            if(0 != pthread_mutex_init(&(pChnAttr->stMutex_valid), NULL))
            {
                close(g_s32PvrFd);
                for(j = 0; j < PVR_PLAY_MAX_CHN_NUM; j++)
                {
                    (void)pthread_mutex_destroy(&(g_stPvrPlayChns[j].stMutex_valid));
                }
                PVRIntfDeInitEvent();
                MT_ERR_PVR("init mutex lock for PVR play chn%d failed \n", i);
                return MT_ERR_PVR_CREAT_MUTEX_ERR;
            }
            if(0 != pthread_mutex_init(&(pChnAttr->stMutex), NULL))
            {
                close(g_s32PvrFd);
                for(j = 0; j < PVR_PLAY_MAX_CHN_NUM; j++)
                {
                    (void)pthread_mutex_destroy(&(g_stPvrPlayChns[j].stMutex));
                }
                for(j = 0; j < PVR_PLAY_MAX_CHN_NUM; j++)
                {
                    (void)pthread_mutex_destroy(&(g_stPvrPlayChns[j].stMutex_valid));
                }
                PVRIntfDeInitEvent();
                MT_ERR_PVR("init mutex lock for PVR play chn%d failed \n", i);
                return MT_ERR_PVR_CREAT_MUTEX_ERR;
            }

            PVR_LOCK_PLAY(pChnAttr);
            mt_pvr_init_playattr(pChnAttr,i);
            PVR_UNLOCK_PLAY(pChnAttr);
        }

#ifdef PVR_PROC_SUPPORT
        memset(pProcDirName, 0, sizeof(pProcDirName));
        strcpy(pProcDirName,"msp");
        if (!PVR_Rec_IsRecording())
        {
            ret = mt_module_register(MT_ID_PVR, PVR_USR_PROC_DIR);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("MT_MODULE_Register(\"%s\") return %d\n", PVR_USR_PROC_DIR, ret);
            }

            /* Add proc dir */
            ret = mt_proc_add_dir(pProcDirName);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("MT_PROC_AddDir(\"%s\") return %d\n", pProcDirName, ret);
            }
        }
        g_stPvrPlayProcEntry.pszDirectory = pProcDirName;
        g_stPvrPlayProcEntry.pszEntryName = PVR_USR_PROC_PLAY_ENTRY_NAME;
        g_stPvrPlayProcEntry.pfnShowProc = PVRPlayShowProc;
        g_stPvrPlayProcEntry.pfnCmdProc = PVRPlaySetProc;
        g_stPvrPlayProcEntry.pPrivData = g_stPvrPlayChns;
        ret = mt_proc_add_entry(MT_ID_PVR, &g_stPvrPlayProcEntry);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("MT_PROC_AddEntry(\"%s\") return %d\n", PVR_USR_PROC_PLAY_ENTRY_NAME, ret);
        }
#endif

        g_stPlayInit.bInit = MT_TRUE;
#ifdef VMX_ADVCA_PVR
    memset(&decrypt_rec_buff[0], 0, sizeof(mt_mmz_buf_s));
    memset(&decrypt_rec_buff[1], 0, sizeof(mt_mmz_buf_s));
#endif

        return MT_SUCCESS;
    }
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayDeInit
 Description     : play module de-initialize
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/14
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayDeInit(MT_VOID)
{
    MT_U32 i;

    if ( MT_FALSE == g_stPlayInit.bInit )
    {
        MT_WARN_PVR("Play Module is not Initialized!\n");
        return MT_SUCCESS;
    }
    else
    {
        /* set all play channel as INVALID status                            */
        for (i = 0 ; i < PVR_PLAY_MAX_CHN_NUM; i++)
        {
            if (g_stPvrPlayChns[i].enState != MT_UNF_PVR_PLAY_STATE_INVALID)
            {
                MT_ERR_PVR("play chn%d is in use, can NOT deInit PLAY!\n", i);
                return MT_ERR_PVR_BUSY;
            }

            (MT_VOID)pthread_mutex_destroy(&(g_stPvrPlayChns[i].stMutex));
            (MT_VOID)pthread_mutex_destroy(&(g_stPvrPlayChns[i].stMutex_valid));
        }

#ifdef PVR_PROC_SUPPORT
        mt_proc_remove_entry(MT_ID_PVR, &g_stPvrPlayProcEntry);
        if (!PVR_Rec_IsRecording())
        {
            //mt_proc_remove_dir(g_stPvrPlayProcEntry.pszDirectory);
            mt_module_unregister(MT_ID_PVR);
        }
#endif

        g_stPlayInit.bInit = MT_FALSE;
        PVRIntfDeInitEvent();
        return MT_SUCCESS;
    }
}
static RET_CODE advCa_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
										u8 *pu8DestVirAddr,  ulong ulongDestPhyAddr,
										u8 *pu8SrcDataVirAddr, ulong ulongSrcDataPhyAddr,
										MT_U32 u32Offset,MT_U32 *u32DataSize)
{
    mt_pvr_addon_play_input_param_t  p_param_in={0};
    mt_pvr_addon_play_output_param_t p_param_out={0};
    mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
    p_param_in.p_src_data = (u8*)(pu8SrcDataVirAddr+u32Offset);
    p_param_in.p_dest_data = (u8*)pu8DestVirAddr;
    p_param_in.p_src_data_phy = ulongSrcDataPhyAddr+u32Offset;
    p_param_in.p_dest_data_phy = ulongDestPhyAddr;
    p_param_in.len=*u32DataSize;
	p_param_in.global_offset = pstDataAttr->u64GlobalOffset;
	p_param_in.first_push = pstDataAttr->u32FirstPush;
    if(p_addon && (p_addon->play_inter.data_process)){
        mt_handle_t ply_handle_nagra=p_addon->handlesply;
        if(0<ply_handle_nagra){
            if(MPVR_ADDON_STATUS_OK == p_addon->play_inter.data_process(ply_handle_nagra, pstDataAttr->u32DmxID,
				&p_param_in,&p_param_out))
            {
                *u32DataSize = p_param_in.len;
#if 0
                if(0)
                {
                    char tmpname[128];
                    strcpy(tmpname,"/tmp/medias/sda1/play.ts");
                    int fd = PVR_OPEN(tmpname, PVR_FOPEN_MODE_DATA_WRITE);
                    if(fd)
                    {
                        int filesize = PVR_SEEK(fd , 0 , SEEK_END);
                        ssize_t writed=PVR_WRITE((const void *)p_param_in.p_dest_data,p_param_in.len,fd,filesize);
                        PVR_CLOSE(fd);
                    }
                }
#endif
                return MT_SUCCESS;
            }
        }
    }
    return MT_FAILURE;
}
static RET_CODE normal_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr,
										u8 *pu8DestVirAddr,  ulong ulongDestPhyAddr,
										u8 *pu8SrcDataVirAddr, ulong ulongSrcDataPhyAddr,
										u32 u32Offset,
										u32 *u32DataSize)
{//no need to realize me
	//PVR_PRINTF("+normal.r\n");
	return 0;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayCreateChn
 Description     : create one play channel
 Input           : pAttr  **the attribute of channel
 Output          : pchn   **play channel number
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayCreateChn(MT_U32 *pChn, const MT_UNF_PVR_PLAY_ATTR_S *pAttr, MT_HANDLE hAvplay, MT_HANDLE hTsBuffer)
{
    MT_S32 ret;
    PVR_PLAY_CHN_S *pChnAttr =NULL;
    MT_U32 tsFileFirstNode=0;

    PVR_CHECK_POINTER(pAttr);
    PVR_CHECK_POINTER(pChn);

    PVR_PLAY_CHECK_INIT(&g_stPlayInit);

#ifdef JQW
    ret = PVRPlayCheckUserCfg(pAttr, hAvplay, hTsBuffer);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }
#endif
    pChnAttr = PVRPlayFindFreeChn();
    if (NULL == pChnAttr)
    {
        MT_ERR_PVR("Not enough channel to be used!\n");
        return MT_ERR_PVR_NO_CHN_LEFT;
    }

    PVR_LOCK_PLAY(pChnAttr);

    pChnAttr->u32magic1 = 0xaa55aa55;
    pChnAttr->u32magic2 = 0xaa55aa55;
    pChnAttr->line_magc1 = 0;
    pChnAttr->line_magc2 = 0;
    pChnAttr->s32DataFile = PVR_FILE_INVALID_FILE;
    pChnAttr->IndexHandle = NULL;
    pChnAttr->hCipher = 0;
    pChnAttr->PlayStreamThread = 0;
    pChnAttr->bCAStreamHeadSent = MT_FALSE;
    pChnAttr->u64LastSeqHeadOffset = PVR_INDEX_INVALID_SEQHEAD_OFFSET;
    pChnAttr->bPlayMainThreadStop = MT_TRUE;
    pChnAttr->u64CurReadPos = 0;
    //pChnAttr->bAdecStoped = MT_FALSE;
    pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
    pChnAttr->bPlayingTsNoIdx = MT_FALSE;
    pChnAttr->bTsBufReset = MT_TRUE;
    pChnAttr->enLastSyncState = MT_UNF_AVPLAY_SYNC_REF_BUTT;
    pChnAttr->play_errcode=0;
    pChnAttr->first_push=1;
    pChnAttr->pause2resume_forirdeto=0;
    memcpy(&pChnAttr->stUserCfg, pAttr, sizeof(MT_UNF_PVR_PLAY_ATTR_S));
    pChnAttr->stUserCfg.stDecryptCfg.bDoCipher = pAttr->stDecryptCfg.bDoCipher;
    //pChnAttr->stUserCfg.bIsClearStream = MT_FALSE;

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

    pChnAttr->start_timeoffset = pAttr->u32StartTimeOffset;
    PVR_PLAY_P("start_timeoffset=%d\n",pChnAttr->start_timeoffset);

#ifndef CONFIG_MT_CHIP_SYMPHONY1
    pChnAttr->hCipher=MT_INVALID_HANDLE;
    pChnAttr->hCipher_slot=MT_CIPHER_KEYSLOT_INVALID;
#endif

    /* initialize cipher module */
    ret = PVRPlayPrepareCipher(pChnAttr);
    if (ret != MT_SUCCESS)
    {
        goto ErrorExit;
    }

    /*  check whether current to open file is recording or not
        if recording, return the recording file index handle, regard it as timeshift play channel to manage
        or alone play channel, which not support record the playing file.
    */
   PVR_EVENT_D("%s >>>> pAttr->bSupportTimeShiftEvent =%d\n",__FUNCTION__,pAttr->bSupportTimeShiftEvent);

    pChnAttr->IndexHandle = PVR_Index_CreatPlay(pChnAttr->u32chnID,
                                                                                pAttr,
                                                                                &pChnAttr->bPlayingTsNoIdx,
                                                                                &tsFileFirstNode,
                                                                                pAttr->bSupportTimeShiftEvent,
                                                                                pAttr->bIsTimeShiftEventFile);
    if (NULL == pChnAttr->IndexHandle)
    {
        MT_ERR_PVR("index init failed.\n");
        ret = MT_ERR_PVR_FILE_CANT_READ;
        goto ErrorExit;
    }

    /* open ts file */
    pChnAttr->s32DataFile = PVR_OPEN64(pAttr->szFileName,PVR_FOPEN_MODE_DATA_READ,
                                                                 pAttr->bSupportTimeShiftEvent,
                                                                 pAttr->u32TimeShiftEventDataUnitSize,
                                                                 0,
                                                                 tsFileFirstNode);
    if (PVR_FILE_INVALID_FILE == pChnAttr->s32DataFile)
    {
        ret = MT_ERR_PVR_FILE_CANT_OPEN;
        goto ErrorExit;
    }

    pChnAttr->u64TsFileSize = PVR_FILE_GetFileSize64(pAttr->szFileName);

    pChnAttr->hAvplay = hAvplay;
    pChnAttr->hTsBuffer = hTsBuffer;
    //pChnAttr->idxmax=PVR_PLAY_CACHE_NUM;
    pChnAttr->idxwp=0;
    pChnAttr->framewp=0;
    pChnAttr->framesz=PVR_IDX_CACHED_FRMNUM;
    pChnAttr->framecache=malloc(pChnAttr->framesz*sizeof(PVR_PlAY_FRAMES));
    if(NULL==pChnAttr->framecache){
        MT_ERR_PVR("create index = NULL\n");
        ret = MT_ERR_PVR_NO_MEM;
        goto ErrorExit;
    }
	memset(pChnAttr->framecache, 0x00, (pChnAttr->framesz*sizeof(PVR_PlAY_FRAMES)));

    pvrplay_clearvframe(pChnAttr);

    //pChnAttr->idxrp=0;
    //posix_memalign(((void **)&(pChnAttr->diocache256k)), (size_t)PVR_IOBLOCK, (size_t)PVR_CACHESIZE);
    //MT_INFO_PVR("+++page256k_m=%x\n",pChnAttr->diocache256k);
    //pChnAttr->fstartdpos=0;
    //pChnAttr->fendpos=0;
    *pChn = pChnAttr->u32chnID;
#ifdef PVR_PROC_SUPPORT
    memset(&(pChnAttr->stPlayProcInfo), 0, sizeof(PVR_PLAY_PROC_S));
#endif
  //pChnAttr->stUserCfg.bSupportAdvCa = MT_UNF_PVR_REC_FREE;
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        mt_pvr_addon_play_attr_t ply_attr={0};
        ply_attr.pvr_encrypt_flag=pChnAttr->stUserCfg.stDecryptCfg.bDoCipher;
        strcpy((char*)ply_attr.file_name,pAttr->szFileName);
        if(p_addon->play_inter.create)
        {
            if(p_addon->play_inter.create(&p_addon->handlesply,&ply_attr) != MPVR_ADDON_STATUS_OK)
            {
                MT_ERR_PVR("ERROR call addon create\n");
                goto ErrorExit;
            }
            else
                p_addon->handlesply_pvr = pChnAttr->u32chnID;
        }
    }
    if(pChnAttr->stUserCfg.stDecryptCfg.bDoCipher)
        pChnAttr->readCallBack = advCa_ReadCallback;
    else
        pChnAttr->readCallBack = normal_ReadCallback;
#ifdef VMX_ADVCA_PVR
    PVR_EVENT_D("%s==========pAttr->bSupportAdvCa=%d, u32ReadFrame=%d\n",__FUNCTION__,pAttr->bSupportAdvCa, pChnAttr->IndexHandle->u32ReadFrame);
    if((MT_UNF_PVR_REC_VMX==pAttr->bSupportAdvCa) || (MT_UNF_PVR_REC_COMMON_CRAMBLE==pAttr->bSupportAdvCa))
    {
         int rec_id = 0;
         pChnAttr->stUserCfg.bSupportAdvCa = pAttr->bSupportAdvCa;//MT_UNF_PVR_REC_VMX;

         if(decrypt_rec_buff[rec_id].bufsize == 0)
         {
                decrypt_rec_buff[rec_id].bufsize = PUSH_VBLOCK_SZ + 188*1024*2;
                ret = mt_mmz_malloc(&decrypt_rec_buff[rec_id]);
                if(MT_SUCCESS != ret)
                {
                    MT_ERR_PVR("%s %d mt_mmz_malloc error\n",__FUNCTION__,__LINE__);
                    PVR_UNLOCK_PLAY(pChnAttr);
                    return ret;
                }
                memset(decrypt_rec_buff[rec_id].user_viraddr, 0, decrypt_rec_buff[rec_id].bufsize);
                vmx_pvr_play_printf("decrypt_rec_buff.user_viraddr=0x%x, decrypt_rec_buff.bufsize=0x%x, decrypt_rec_buff.phyaddr,=0x%x\n",
                        decrypt_rec_buff[rec_id].user_viraddr, decrypt_rec_buff[rec_id].bufsize, decrypt_rec_buff[rec_id].phyaddr);
         }
     }
#endif
#ifdef NGR_ADVCA_PVR
     if(MT_UNF_PVR_REC_NAGRA == pAttr->bSupportAdvCa){
        pChnAttr->stUserCfg.bSupportAdvCa = MT_UNF_PVR_REC_NAGRA;
     }
#endif
#ifdef IRD_ADVCA_PVR
     if(MT_UNF_PVR_REC_IRDETO == pAttr->bSupportAdvCa){
        pChnAttr->stUserCfg.bSupportAdvCa = MT_UNF_PVR_REC_IRDETO;
     }
#endif
#ifdef DEF_ADVCA_PVR
      if(MT_UNF_PVR_REC_DEFALT == pAttr->bSupportAdvCa){
         pChnAttr->stUserCfg.bSupportAdvCa = MT_UNF_PVR_REC_DEFALT;
      }
#endif
    PVR_UNLOCK_PLAY(pChnAttr);

    MT_INFO_PVR("\n--------PVR.PLAY.Ver\n\n");

    return MT_SUCCESS;

ErrorExit:
    if (PVR_FILE_INVALID_FILE != pChnAttr->s32DataFile)
    {
        (MT_VOID)PVR_CLOSE64(pChnAttr->s32DataFile, pChnAttr->stUserCfg.bSupportTimeShiftEvent);
        pChnAttr->s32DataFile = PVR_FILE_INVALID_FILE;
    }

    if (pChnAttr->IndexHandle)
    {
       (MT_VOID)PVR_Index_Destroy(pChnAttr->IndexHandle, PVR_INDEX_PLAY);
       pChnAttr->IndexHandle = NULL;
    }

    (MT_VOID)PVRPlayReleaseCipher(pChnAttr);

    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INVALID;
    ioctl(g_s32PvrFd, CMD_PVR_DESTROY_PLAY_CHN, (ulong)&(pChnAttr->u32chnID));
    PVR_UNLOCK_PLAY(pChnAttr);
//    MT_ERR_PVR(" play create ok id=%d data file %x\n",pChnAttr->u32chnID,pChnAttr->s32DataFile);
    return ret;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayDestroyChn
 Description     : destroy one play channel
 Input           : u32Chn  **channel number
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayDestroyChn(MT_U32 u32Chn)
{
    PVR_PLAY_CHN_S  *pChnAttr;

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);
    PVR_LOCK_PLAY_VALID(pChnAttr);

    /* check channel state */
    if (!(MT_UNF_PVR_PLAY_STATE_STOP == pChnAttr->enState
        || MT_UNF_PVR_PLAY_STATE_INIT == pChnAttr->enState))
    {
        MT_ERR_PVR("You should stop channel first!\n");
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_PLAY_INVALID_STATE;
    }

    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INVALID;
    if (MT_SUCCESS != ioctl(g_s32PvrFd, CMD_PVR_DESTROY_PLAY_CHN, (ulong)&u32Chn))
    {
        MT_FATAL_PVR("pvr play destroy channel error.\n");
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_PLAY_FAIL_DESTROY;
    }
    PVR_EVENT_D("%s >>>> pChnAttr->stUserCfg.bSupportTimeShiftEvent =%d\n",__FUNCTION__,pChnAttr->stUserCfg.bSupportTimeShiftEvent);
    /* close stream file                                                    */
    (MT_VOID)PVR_CLOSE64(pChnAttr->s32DataFile,  pChnAttr->stUserCfg.bSupportTimeShiftEvent);

    pChnAttr->IndexHandle->u32ReadFrame = 0; //zyy 2009.05.22 AI7D05498
    (MT_VOID)PVR_Index_Destroy(pChnAttr->IndexHandle, PVR_INDEX_PLAY);
    pChnAttr->IndexHandle = NULL;

    (MT_VOID)PVRPlayReleaseCipher(pChnAttr);

    pChnAttr->u64LastSeqHeadOffset = PVR_INDEX_INVALID_SEQHEAD_OFFSET;
    pChnAttr->PlayStreamThread = 0;
    if(pChnAttr->framecache){
        free(pChnAttr->framecache);
        pChnAttr->framecache=NULL;
    }
    //MT_INFO_PVR("+++page256k_f=%x\n",pChnAttr->diocache256k);
    //if(pChnAttr->diocache256k){
    //    free(pChnAttr->diocache256k);
    //    pChnAttr->diocache256k=NULL;
    //}
#ifdef PVR_PROC_SUPPORT
    memset(&(pChnAttr->stPlayProcInfo), 0, sizeof(PVR_PLAY_PROC_S));
#endif
#ifdef VMX_ADVCA_PVR
    vmx_pvr_play_printf("%s==========pAttr->bSupportAdvCa=%d\n",__FUNCTION__,pChnAttr->stUserCfg.bSupportAdvCa);
    if((pChnAttr->stUserCfg.bSupportAdvCa == MT_UNF_PVR_REC_VMX)||
       (pChnAttr->stUserCfg.bSupportAdvCa == MT_UNF_PVR_REC_COMMON_CRAMBLE))
    {
        int rec_id=0;
        int ret=0;
        if(decrypt_rec_buff[rec_id].bufsize != 0)
        {
            ret = mt_mmz_free(&decrypt_rec_buff[rec_id]);
            if(MT_SUCCESS != ret)
            {
                MT_ERR_PVR("%s %d mt_mmz_malloc error\n",__FUNCTION__,__LINE__);
                PVR_UNLOCK_PLAY_VALID(pChnAttr);
                PVR_UNLOCK_PLAY(pChnAttr);
                return MT_ERR_PVR_FREEMEM_ERR;
            }
           memset(&decrypt_rec_buff[rec_id], 0, sizeof(mt_mmz_buf_s));
        }
    }
#endif  //VMX_ADVCA_PVR
    #ifdef CONFIG_MT_LXC_SUPPORT
    mt_pvr_ipc_clear_tid();
    #endif
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->play_inter.on_destroy &&p_addon->handlesply)
        {
            if(p_addon->play_inter.on_destroy(p_addon->handlesply) != MPVR_ADDON_STATUS_OK)
                MT_ERR_PVR("ERROR call addon create\n");
        }
    }
    mt_pvr_init_playattr(pChnAttr,pChnAttr->u32chnID);
    PVR_UNLOCK_PLAY_VALID(pChnAttr);
    PVR_UNLOCK_PLAY(pChnAttr);

    return MT_SUCCESS;
}


/*****************************************************************************
 Prototype       : MT_PVR_PlaySetChn
 Description     : set play channle attributes
 Input           : chn    **
                   pAttr  **
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
MT_S32  MT_PVR_PlaySetChn(MT_U32 u32ChnID, const MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr)
{
    PVR_PLAY_CHN_S  *pChnAttr;

    PVR_PLAY_CHECK_CHN(u32ChnID);
    pChnAttr = &g_stPvrPlayChns[u32ChnID];
    PVR_PLAY_CHECK_CHN_INIT(pChnAttr->enState);

    PVR_CHECK_POINTER(pstPlayAttr);

    /* TODO: we set several attributes which can be set dynamically. */

    return MT_ERR_PVR_NOT_SUPPORT;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayGetChn
 Description     : get play channel attribute
 Input           : chn    **
                   pAttr  **
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
MT_S32 MT_PVR_PlayGetChn(MT_U32 u32ChnID, MT_UNF_PVR_PLAY_ATTR_S *pstPlayAttr)
{
    PVR_PLAY_CHN_S  *pChnAttr;

    PVR_PLAY_CHECK_CHN(u32ChnID);
    pChnAttr = &g_stPvrPlayChns[u32ChnID];
    PVR_PLAY_CHECK_CHN_INIT(pChnAttr->enState);

    PVR_CHECK_POINTER(pstPlayAttr);

    memcpy(pstPlayAttr, &pChnAttr->stUserCfg, sizeof(MT_UNF_PVR_PLAY_ATTR_S));

    return MT_SUCCESS;
}


//AVPlay Callback
static mt_s32 avplayer_callback(mt_handle hAvplay, MT_UNF_AVPLAY_EVENT_E enEvent, ulong u32Para)
{
	if (hAvplay == MT_INVALID_HANDLE)
	{
        MT_ERR_PVR("An invalid avplay handle\n");
        return MT_FAILURE;
    }

	switch (enEvent)
	{
		case MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED:
			g_pvr_last_vid_frame_decoded = MT_TRUE;

			PVR_ALAWYS_PRINT("[%s %d], got MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED\n",
				__func__, __LINE__);

			break;

		case MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED:

			g_pvr_last_vid_frame_showed = MT_TRUE;

			PVR_ALAWYS_PRINT("[%s %d], got MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED %d\n",
				__func__, __LINE__, enEvent);
			break;

		default:
			PVR_ALAWYS_PRINT("[%s %d]: avplay %lx - avent(%d) not support!\n",
                __FUNCTION__, __LINE__, hAvplay, enEvent);
			break;
	}

	return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayStartChn
 Description     : start play channel
 Input           : u32ChnID **channel number
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayStartChn(MT_U32 u32ChnID) /* pause when end of file */
{
    MT_S32                      ret;
    PVR_PLAY_CHN_S              *pChnAttr;
    MT_U32                      pid;
    MT_U32                      u32DispOptimizeFlag;
    MT_BOOL                     bAudNGFlg = MT_FALSE;
    MT_HANDLE                   hWindow;
    MT_CODEC_VIDEO_CMD_S    stVdecCmd;
    MT_UNF_AVPLAY_VDEC_INFO_S stVdecInfo = {0};
      MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S stAvplayPrivInfo = {0};
    struct sigevent stSigEvt;
    struct itimerspec stTimeSpec;
    pthread_attr_t thread_attr;
    pthread_attr_t *p_thread_attr=NULL;

    PVR_PLAY_CHECK_CHN(u32ChnID);
    pChnAttr = &g_stPvrPlayChns[u32ChnID];
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply && p_addon->play_inter.on_start && MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_start(p_addon->handlesply)))
        {
            MT_ERR_PVR("start addon error:%d\n",ret);
            return ret;
        }
    }
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    if(NULL==pChnAttr->IndexHandle){//tscan
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }
    if (!(MT_UNF_PVR_PLAY_STATE_STOP == pChnAttr->enState
          || MT_UNF_PVR_PLAY_STATE_INIT == pChnAttr->enState))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("Can't start play channel at current state!\n");
        return MT_ERR_PVR_PLAY_INVALID_STATE;
    }

    ret = MT_MPI_AVPLAY_GetWindowHandle(pChnAttr->hAvplay, &hWindow);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY get window handle failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    pChnAttr->u32FrameRate_fromav=0xffffffff;

    ret = MT_UNF_AVPLAY_SetDecodeMode(pChnAttr->hAvplay, MT_UNF_VCODEC_MODE_NORMAL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("set vdec normal mode error!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    ret = MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        bAudNGFlg = MT_TRUE;
        MT_ERR_PVR("has not audio stream! ret=%#x pid=%d\n", ret, pid);
    }
    else
    {
        ret = MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
        if (MT_SUCCESS != ret)
        {
            bAudNGFlg = MT_TRUE;
            MT_ERR_PVR("Can't start audio, error:%#x!\n", ret);
        }
        else
        {
            MT_INFO_PVR("MT_UNF_AVPLAY_start audio ok!\n");
        }
    }

    MT_MPI_AVPLAY_set_trickmode(pChnAttr->hAvplay, 0);//fixed #32681

    ret = MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        pChnAttr->bRecordedVideoExist = MT_FALSE;
        MT_ERR_PVR("has not video stream! ret=%#x pid=%d\n", ret, pid);
        if(MT_TRUE == bAudNGFlg)
        {
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
    }
    else
    {
        pChnAttr->bRecordedVideoExist = MT_TRUE;
        ret = MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("Can't start video, error:%#x!\n", ret);
            if(MT_TRUE == bAudNGFlg)
            {
                PVR_UNLOCK_PLAY(pChnAttr);
                return ret;
            }
        }
        else
        {
            MT_INFO_PVR("MT_UNF_AVPLAY_start video ok!\n");
        }
    }

    ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY resume failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    pChnAttr->set_eof = MT_FALSE;
    pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
    pChnAttr->bPlayMainThreadStop = MT_FALSE;
    pChnAttr->bQuickUpdateStatus = MT_TRUE;
    pChnAttr->u64LastSeqHeadOffset = PVR_INDEX_INVALID_SEQHEAD_OFFSET;
    pChnAttr->bEndOfFile = MT_FALSE;
    pChnAttr->bNotAvailableTsBuff = MT_FALSE;
    pChnAttr->u32CurPlayTimeMs=0;
    pChnAttr->bEofSeek2Start = MT_FALSE;

    g_pvr_last_vid_frame_decoded = MT_FALSE;
	g_pvr_last_vid_frame_showed = MT_FALSE;
	g_pvr_last_i_frame = 0;

    memset(&pChnAttr->stLastStatus, 0, sizeof(MT_UNF_PVR_PLAY_STATUS_S));
    memset(&pChnAttr->stVdecCtrlInfo, 0, sizeof(MT_UNF_AVPLAY_CONTROL_INFO_S));

    PVR_Index_ResetPlayAttr(pChnAttr->IndexHandle);

    /*if(MT_SUCCESS!=PVRPlayFindCorretEndIndex(pChnAttr))
    {
        MT_ERR_PVR("find endframe failed!\n");
    }*/

    if ((pChnAttr->IndexHandle->u64PauseOffset != 0) &&
       (pChnAttr->IndexHandle->u64PauseOffset != PVR_INDEX_PAUSE_INVALID_OFFSET))
    {
        #if 0
        MT_S32 s32Ret = 0;
        if (MT_SUCCESS != PVR_Index_SeekToPauseOrStart(pChnAttr->IndexHandle))
        {
            MT_ERR_PVR("seek to pause frame failed!\n");
        }

        s32Ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &(pChnAttr->IndexHandle->stCurPlayFrame), pChnAttr->IndexHandle->u32ReadFrame);
        if((MT_ERR_PVR_FILE_TILL_END == s32Ret) || (pChnAttr->IndexHandle->stCurPlayFrame.u64GlobalOffset < pChnAttr->IndexHandle->u64FileSizeGlobal))
        {
            s32Ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &(pChnAttr->IndexHandle->stCurPlayFrame), --pChnAttr->IndexHandle->u32ReadFrame);
            if (MT_SUCCESS != s32Ret)
            {
                MT_ERR_PVR("get the %d entry fail.\n", pChnAttr->IndexHandle->u32ReadFrame);
            }
        }
        pChnAttr->bTimeShiftStartFlg = MT_TRUE;
        pChnAttr->u32CurPlayTimeMs = pChnAttr->IndexHandle->stCurPlayFrame.u32DisplayTimeMs;
        #else
        //clear pause infor, and seek to start-pos not to pause-pos. fix bug109758
        pChnAttr->IndexHandle->u64PauseOffset = PVR_INDEX_PAUSE_INVALID_OFFSET;
        pChnAttr->IndexHandle->u32PauseFrame = 0;
        pChnAttr->bTimeShiftStartFlg = MT_TRUE;
        if (MT_SUCCESS != PVR_Index_SeekToStart(pChnAttr->IndexHandle))
        {
            MT_ERR_PVR("seek to start frame failed!\n");
        }
        #endif
    }
    else
    {

        MT_INFO_PVR("start_timeoffset=%u\n",pChnAttr->start_timeoffset);
        if(pChnAttr->start_timeoffset)
        {
            PVR_INDEX_ENTRY_S   frame ={0};
            if (MT_SUCCESS != PVR_Index_SeekToTime(pChnAttr->IndexHandle,pChnAttr->start_timeoffset))
            {
                MT_ERR_PVR("seek to time=%u failed!\n",pChnAttr->start_timeoffset);
            }
            //MT_ERR_PVR("&&&& pChnAttr->IndexHandle->u32ReadFrame = %d enState = %d enLastState=%d\n",pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->enState,pChnAttr->enLastState);
			if( (pChnAttr->IndexHandle->u32ReadFrame+25) >= pChnAttr->IndexHandle->stCycMgr.u32EndFrame)
            {
                PVR_Index_GetPreIFrame(pChnAttr->IndexHandle,&frame);
                PVR_Index_GetPreIFrame(pChnAttr->IndexHandle,&frame);
                PVR_Index_GetPreIFrame(pChnAttr->IndexHandle,&frame);
                //MT_ERR_PVR("&&&& pChnAttr->IndexHandle->u32ReadFrame = %d enState = %d enLastState=%d\n",pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->enState,pChnAttr->enLastState);
            }
        }
        else
        {
            if (MT_SUCCESS != PVR_Index_SeekToStart(pChnAttr->IndexHandle))
            {
                MT_ERR_PVR("seek to start frame failed!\n");
            }
        }
    }
    pChnAttr->current_playing_frame=pChnAttr->IndexHandle->u32ReadFrame;    //record first playing frame
    {//get current display;
        PVR_INDEX_ENTRY_S tmp;
        if(MT_SUCCESS == PVR_Index_GetFrameByNum(pChnAttr->IndexHandle,&tmp,pChnAttr->current_playing_frame)){
            pChnAttr->u32CurPlayTimeMs=tmp.u32DisplayTimeMs;
            pChnAttr->u64CurPlayPosition=tmp.u64GlobalOffset;
        }else{
            pChnAttr->u32CurPlayTimeMs=0;
            pChnAttr->u64CurPlayPosition=0ULL;
        }
        //printf("++set playing time0=%d\n",pChnAttr->u32CurPlayTimeMs);
    }

    PVR_Index_GetRecIdxInfo(pChnAttr->IndexHandle);

    if (PVR_REC_INDEX_MAGIC_WORD != pChnAttr->IndexHandle->stRecIdxInfo.u32MagicWord)
    {
        PVR_Index_GetIdxInfo(pChnAttr->IndexHandle);
    }

    if ( pChnAttr->bRecordedVideoExist == MT_TRUE)
    {
        stVdecCmd.u32CmdID = MT_UNF_AVPLAY_GET_VDEC_INFO_CMD;
        stVdecCmd.pPara = &stVdecInfo;
        if (MT_SUCCESS != MT_UNF_AVPLAY_Invoke(pChnAttr->hAvplay, MT_UNF_AVPLAY_INVOKE_VCODEC, &stVdecCmd))
        {
            MT_ERR_PVR("MT_UNF_AVPLAY_Invoke get vdec info fail\n");
            pChnAttr->u32FrmNum = PVR_DEFAULT_FRAME_BUFF_NUM;
        }
        else
        {
            pChnAttr->u32FrmNum = stVdecInfo.u32DispFrmBufNum;

            ret = MT_UNF_AVPLAY_Invoke(pChnAttr->hAvplay, MT_UNF_AVPLAY_INVOKE_GET_PRIV_PLAYINFO, &stAvplayPrivInfo);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("MT_UNF_AVPLAY_Invoke get private info fail\n");
            }
            u32DispOptimizeFlag = stAvplayPrivInfo.u32DispOptimizeFlag;

            if (1 == u32DispOptimizeFlag)
            {
                if ((pChnAttr->IndexHandle->stRecIdxInfo.stIdxInfo.u32MaxGopSize + PVR_VO_FRMBUFF_NUM_OF_ENABLE_DEI) <= stVdecInfo.u32DispFrmBufNum)
                {
                    pChnAttr->u32VoFrmNum = PVR_VO_FRMBUFF_NUM_OF_ENABLE_DEI;
                    pChnAttr->stVdecCtrlInfo.u32DispOptimizeFlag = PVR_ENABLE_DISP_OPTIMIZE;
                }
                else
                {
                    MT_ERR_PVR("Can not enable display optimize, max gop size=%d but frame buffer=%d.\n",
                        pChnAttr->IndexHandle->stRecIdxInfo.stIdxInfo.u32MaxGopSize,
                        stVdecInfo.u32DispFrmBufNum);
                    //lint -e655
                    MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD | MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
                    //lint +e655
                    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INIT ;
                    pChnAttr->enLastState = MT_UNF_PVR_PLAY_STATE_INIT ;
                    PVR_UNLOCK_PLAY(pChnAttr);
                    return MT_FAILURE;
                }
            }
            else
            {
                pChnAttr->u32VoFrmNum = PVR_VO_FRMBUFF_NUM_OF_DISABLE_DEI;
                pChnAttr->stVdecCtrlInfo.u32DispOptimizeFlag = PVR_DISABLE_DISP_OPTIMIZE;
            }
        }
    }
    pChnAttr->enFBTimeCtrlLastSpeed = MT_UNF_PVR_PLAY_SPEED_BUTT;

    PVRPlayAnalysisStream(pChnAttr);
    p_thread_attr=NULL;
    if(g_playthreadattr.flag_valid){
        //printf("pvr r ts0 thread:%d,%d,%d\n",g_playthreadattr.schedpolicy,g_playthreadattr.priority,g_playthreadattr.stacksize);
        if(0==pvr_create_thread_attr(&thread_attr,g_playthreadattr.schedpolicy,g_playthreadattr.priority,g_playthreadattr.stacksize)){
            p_thread_attr=&thread_attr;
            //printf("pvr r ts1 thread:%d,%d,%d\n",g_playthreadattr.schedpolicy,g_playthreadattr.priority,g_playthreadattr.stacksize);
        }
    }
    if (pthread_create(&pChnAttr->PlayStreamThread, p_thread_attr, PVRPlayMainRoute, pChnAttr))
    {
        MT_ERR_PVR("create play thread failed!\n");
        //lint -e655
        (void)MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD | MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
        //lint +e655
        /* and also reset play state to init                                      */
        pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_INIT ;
        pChnAttr->enLastState = MT_UNF_PVR_PLAY_STATE_INIT ;
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    /* init timer only once */
    if (MT_FALSE == g_bPlayTimerInitFlag)
    {
        memset (&stSigEvt, 0, sizeof (struct sigevent));
        stSigEvt.sigev_value.sival_ptr = &g_stPlayTimer;
        stSigEvt.sigev_notify = SIGEV_THREAD;
        stSigEvt.sigev_notify_function = PVRPlayCalcTime;

        if(MT_SUCCESS != timer_create(CLOCK_REALTIME, &stSigEvt, &g_stPlayTimer))
        {
            MT_ERR_PVR("Create play timer failed!\n");
        }

        stTimeSpec.it_interval.tv_sec = 0;
        stTimeSpec.it_interval.tv_nsec = PVR_TIME_CTRL_TIMEBASE_NS;
        stTimeSpec.it_value.tv_sec = 0;
        stTimeSpec.it_value.tv_nsec = PVR_TIME_CTRL_TIMEBASE_NS;

        if(MT_SUCCESS != timer_settime(g_stPlayTimer, 0,/*TIMER_ABSTIME,*/ &stTimeSpec, NULL))
        {
            MT_ERR_PVR("Start play timer failed!\n");
        }

        g_bPlayTimerInitFlag = MT_TRUE;
    }
    if(pChnAttr->IndexHandle){
        MT_UNF_VCODEC_ATTR_S        VdecAttr;
        if(MT_UNF_PVR_REC_INDEX_TYPE_VIDEO==pChnAttr->IndexHandle->enIndexType){
            if(MT_SUCCESS==MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr)){
                pChnAttr->IndexHandle->VdecType=VdecAttr.enType;
                //printf("+++get vdec type=%x\n",VdecAttr.enType);
            }
        }
    }

  	MT_UNF_AVPLAY_RegisterEvent(pChnAttr->hAvplay,
		MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED,	avplayer_callback);

	MT_UNF_AVPLAY_RegisterEvent(pChnAttr->hAvplay,
		MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED,	avplayer_callback);

	pChnAttr->enLastState = pChnAttr->enState;
    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PLAY;

    PVR_UNLOCK_PLAY(pChnAttr);

    return MT_SUCCESS;
}

static mt_u8 u8unf_DecOpenBuf[1024];

mt_s32 MT_PVR_SetAdecAttr(mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly)
{
    MT_UNF_ACODEC_ATTR_S AdecAttr;
    WAV_FORMAT_S stWavFormat;

    MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    AdecAttr.enType = enADecType;

    if (HA_AUDIO_ID_PCM == AdecAttr.enType)
    {
        /* set pcm wav format here base on pcm file 48k.raw */
        stWavFormat.nChannels = 2;
        stWavFormat.nSamplesPerSec = 48000;
        stWavFormat.wBitsPerSample = 16;
        stWavFormat.cbExtWord[0] = 0;
        stWavFormat.cbExtWord[1] = 1;

        HA_PCM_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam),&stWavFormat);
        MT_PRINT("please make sure the attributes of PCM stream is tme same as defined in function of \"MTADP_AVPlay_SetAdecAttr\"? \n");
        MT_PRINT("(nChannels = 1, wBitsPerSample = 16, nSamplesPerSec = 48000, isBigEndian = MT_FALSE) \n");
    }
#if 0
    else if (HA_AUDIO_ID_G711 == AdecAttr.enType)
    {
    HA_G711_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#endif
    else if (HA_AUDIO_ID_MP2 == AdecAttr.enType)
    {
        HA_MP2_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_AAC == AdecAttr.enType)
    {
        HA_AAC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
    else if (HA_AUDIO_ID_MP3 == AdecAttr.enType)
    {
        HA_MP3_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
    }
#if 0
    else if (HA_AUDIO_ID_AMRNB== AdecAttr.enType)
    {
    AMRNB_DECODE_OPENCONFIG_S *pstConfig = (AMRNB_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
    HA_AMRNB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    pstConfig->enFormat = AMRNB_MIME;
    }
    else if (HA_AUDIO_ID_AMRWB== AdecAttr.enType)
    {
    AMRWB_DECODE_OPENCONFIG_S *pstConfig = (AMRWB_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
    HA_AMRWB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    pstConfig->enFormat = AMRWB_FORMAT_MIME;
    }
#endif
    else if (HA_AUDIO_ID_AC3PASSTHROUGH== AdecAttr.enType)
    {
        HA_AC3PASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if(HA_AUDIO_ID_DTSPASSTHROUGH ==  AdecAttr.enType)
    {
        HA_DTSPASSTHROUGH_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;
    }
    else if (HA_AUDIO_ID_TRUEHD == AdecAttr.enType)
    {
        HA_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        if (HD_DEC_MODE_THRU != enMode)
        {
            MT_PRINT(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return -1;
        }

        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;        /* truehd just support pass-through */
        MT_PRINT(" TrueHD decoder(HBR Pass-through only).\n");
    }
    else if (HA_AUDIO_ID_DOLBY_TRUEHD == AdecAttr.enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DOLBY_CONVERT == AdecAttr.enType)
    {
        TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
        HA_DOLBY_CONVERT_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_CONVERT_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DTSHD == AdecAttr.enType)
    {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
    else if (HA_AUDIO_ID_DTSM6 == AdecAttr.enType)
    {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if (HA_AUDIO_ID_DOLBY_PLUS == AdecAttr.enType)
    {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &g_stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        //AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
#endif
    else if(HA_AUDIO_ID_DRA == AdecAttr.enType)
    {
        //       HA_DRA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
        HA_DRA_DecGetOpenParam_MultichPcm(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_COOK == AdecAttr.enType || HA_AUDIO_ID_AMRNB ==AdecAttr.enType
    || HA_AUDIO_ID_AMRWB == AdecAttr.enType)
    {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)u8unf_DecOpenBuf;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);
        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        MT_PRINT("cook dec set ffmpeg dec param \n");
    }


    MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayStopChn
 Description     : stop play channel
 Input           : u32Chn  **channel number
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlaySetPid(MT_U32 u32Chn, MT_BOOL aorv,MT_U32 type, MT_U32 pid)
{
    MT_S32 ret;
    MT_U32 XPid=pid;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_UNF_AVPLAY_MEDIA_CHAN_E Chen=0;
    MT_UNF_AVPLAY_STOP_OPT_S stopOpt;
    MT_UNF_VCODEC_ATTR_S        VdecAttr;
    MT_UNF_ACODEC_ATTR_S        AdecAttr;

    pChnAttr = &g_stPvrPlayChns[u32Chn];
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);
    if ((MT_UNF_PVR_PLAY_STATE_STOP == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_INIT == pChnAttr->enState))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("Play channel is stopped already!\n");
        return MT_ERR_PVR_ALREADY;
    }

    MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    if(MT_TRUE==aorv){
        Chen=MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
        ret = MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, Chen, NULL);
    }else{
        stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
        stopOpt.u32TimeoutMs = 0;
        Chen=MT_UNF_AVPLAY_MEDIA_CHAN_VID;
        ret = MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, Chen, &stopOpt);
    }
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_UNF_AVPLAY_Stop failed:%#x, force PVR stop!\n", ret);
        PVR_UNLOCK_PLAY(pChnAttr);
        return ret;
    }
    if (MT_TRUE==aorv)
    {
        ret = MT_PVR_SetAdecAttr(pChnAttr->hAvplay, type, HD_DEC_MODE_RAWPCM, 1);
        ret |= MT_UNF_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &XPid);
        if (MT_SUCCESS != ret) {
            MT_ERR_PVR("MTADP_AVPlay_SetAdecAttr failed:%#x\n", ret);
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
        ret = MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
        if (ret != MT_SUCCESS) {
            MT_ERR_PVR("call MT_UNF_AVPLAY_Start failed.\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
    }else{
        VdecAttr.enType = type;
        ret = MT_UNF_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if (ret != MT_SUCCESS){
            MT_ERR_PVR("call MT_UNF_AVPLAY_SetAttr failed.\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
        ret = MT_UNF_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&XPid);
        if (ret != MT_SUCCESS){
            MT_ERR_PVR("call MTADP_AVPlay_SetVdecAttr failed.\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
        ret = MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
        if (ret != MT_SUCCESS) {
            MT_ERR_PVR("call MT_UNF_AVPLAY_Start failed.\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
    }
    PVR_UNLOCK_PLAY(pChnAttr);

    return MT_SUCCESS;
}

MT_S32 MT_PVR_ChangeAudioPid(MT_U32 u32Chn, MT_BOOL aorv, MT_U32 type, MT_U32 pid)
{
    MT_S32 ret;
    MT_U32 XPid=pid;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_UNF_VCODEC_ATTR_S        VdecAttr;
    MT_UNF_ACODEC_ATTR_S        AdecAttr;

    pChnAttr = &g_stPvrPlayChns[u32Chn];
    if(MT_TRUE == aorv && NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply)
        {
            if(p_addon->play_inter.on_change_audio &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_change_audio(p_addon->handlesply,pid,type)))
            {
                MT_ERR_PVR("pause addon error:%d\n",ret);
                return ret;
            }
        }
    }

    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);
    if ((MT_UNF_PVR_PLAY_STATE_STOP == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_INIT == pChnAttr->enState))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("Play channel is stopped already!\n");
        return MT_ERR_PVR_ALREADY;
    }

    MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    if (MT_TRUE == aorv)
    {
		ret = MT_UNF_AVPLAY_StopAudDec(pChnAttr->hAvplay);
		if (MT_SUCCESS != ret)
		{
			MT_ERR_PVR("MT_UNF_AVPLAY_StopAudDec failed:%#x!\n", ret);
			PVR_UNLOCK_PLAY(pChnAttr);
			return ret;
		}

        ret = MT_PVR_SetAdecAttr(pChnAttr->hAvplay, type, HD_DEC_MODE_RAWPCM, 1);
		if (MT_SUCCESS != ret)
		{
			MT_ERR_PVR("MT_PVR_SetAdecAttr failed:%#x!\n", ret);
			PVR_UNLOCK_PLAY(pChnAttr);
			return ret;
		}

	 	ret = MT_UNF_AVPLAY_StartAudDec(pChnAttr->hAvplay);
        if (ret != MT_SUCCESS) {
            MT_ERR_PVR("call MT_UNF_AVPLAY_StartAudDec failed.\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &XPid);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("call MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            PVR_UNLOCK_PLAY(pChnAttr);
            return ret;
        }
    }


    PVR_UNLOCK_PLAY(pChnAttr);

    return MT_SUCCESS;
}

MT_S32 MT_PVR_PlayStopChn(MT_U32 u32Chn, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt)
{
    MT_S32 ret;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_HANDLE       hWindow;
    MT_U32 i = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr;
    MT_CODEC_VIDEO_CMD_S  stVdecCmdPara = {0};
    MT_UNF_AVPLAY_TPLAY_OPT_S stTplayOpts;
    MT_UNF_AVPLAY_STATUS_INFO_S stAvplayStatus;

    PVR_PLAY_CHECK_CHN(u32Chn);

    memset(&stFrmRateAttr, 0, sizeof(MT_UNF_AVPLAY_FRMRATE_PARAM_S));
    memset(&stTplayOpts, 0, sizeof(MT_UNF_AVPLAY_TPLAY_OPT_S));
    memset(&stAvplayStatus, 0, sizeof(MT_UNF_AVPLAY_STATUS_INFO_S));
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply)
        {
            if(p_addon->play_inter.on_stop && MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_stop(p_addon->handlesply)))
            {
                MT_ERR_PVR("stop addon error:%d\n",ret);
                return ret;
            }
        }
    }
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    if ((MT_UNF_PVR_PLAY_STATE_STOP == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_INIT == pChnAttr->enState))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("Play channel is stopped already!\n");
        return MT_ERR_PVR_ALREADY;
    }

    MT_INFO_PVR("wait Play thread.\n");
    pChnAttr->bPlayMainThreadStop = MT_TRUE;

    PVR_UNLOCK_PLAY(pChnAttr);

    (MT_VOID)pthread_join(pChnAttr->PlayStreamThread, NULL);
    MT_INFO_PVR("wait Play thread OK.\n");

    PVR_LOCK_PLAY(pChnAttr);

    if ( pChnAttr->bRecordedVideoExist == MT_TRUE)
    {
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        ret = MT_UNF_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("set frame to VO fail.\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }


        if (MT_SUCCESS == MT_UNF_AVPLAY_GetStatusInfo(pChnAttr->hAvplay, &stAvplayStatus))
        {
            if (MT_UNF_AVPLAY_STATUS_STOP != stAvplayStatus.enRunStatus)
            {
                stTplayOpts.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
                stTplayOpts.u32SpeedInteger = 1;
                stTplayOpts.u32SpeedDecimal = 0;
                stVdecCmdPara.u32CmdID = MT_UNF_AVPLAY_SET_TPLAY_PARA_CMD;
                stVdecCmdPara.pPara = &stTplayOpts;
                ret = MT_UNF_AVPLAY_Invoke(pChnAttr->hAvplay, MT_UNF_AVPLAY_INVOKE_VCODEC, (void *)&stVdecCmdPara);

                if (MT_SUCCESS != ret)
                {
                    MT_ERR_PVR("Resume Avplay trick mode to normal fail.\n");
                    PVR_UNLOCK_PLAY(pChnAttr);
                    return MT_FAILURE;
                }
            }
        }
    }

    ret = MT_MPI_AVPLAY_SetWindowRepeat(pChnAttr->hAvplay, 1);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY set window repeat failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    ret = MT_MPI_AVPLAY_GetWindowHandle(pChnAttr->hAvplay, &hWindow);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY get window handle failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }
#if 0
    /* set window to step mode */
    ret = MT_MPI_VO_SetWindowStepMode(hWindow, MT_FALSE);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Set window step mode failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }
#endif
    ret = MT_UNF_AVPLAY_SetDecodeMode(pChnAttr->hAvplay, MT_UNF_VCODEC_MODE_NORMAL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("set vdec normal mode error!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }
    {//reset to normal
        PVR_PLAY_POLICY_S stPvrPlayOpt;
        MT_PVR_PlayGetPolicy(pChnAttr, MT_UNF_PVR_PLAY_SPEED_NORMAL, &stPvrPlayOpt);
        MT_UNF_AVPLAY_DecFrmType(pChnAttr->hAvplay,MT_UNF_DEC_FRM_ALL);
        MT_PVR_PlayDispTPlay(pChnAttr, MT_UNF_PVR_PLAY_SPEED_NORMAL,&(stPvrPlayOpt.stDispFrc));
    }
    //lint -e655
    ret = MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD | MT_UNF_AVPLAY_MEDIA_CHAN_VID, pstStopOpt);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("MT_UNF_AVPLAY_Stop failed:%#x, force PVR stop!\n", ret);
    }
    //lint +e655

    ret = MT_UNF_DMX_ResetTSBuffer(pChnAttr->hTsBuffer);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("ts buffer reset failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    pChnAttr->enLastState = MT_UNF_PVR_PLAY_STATE_STOP;
    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_STOP;
    pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_BUTT;
    pChnAttr->set_eof = MT_FALSE;
    pChnAttr->bPlayMainThreadStop = MT_FALSE;
    pChnAttr->bQuickUpdateStatus = MT_FALSE;
    pChnAttr->u64LastSeqHeadOffset = PVR_INDEX_INVALID_SEQHEAD_OFFSET;
    pChnAttr->bEndOfFile = MT_FALSE;
    pChnAttr->bRecordedVideoExist = MT_FALSE;
	pChnAttr->u32CurPlayTimeMs = 0;
    memset(&pChnAttr->stLastStatus, 0, sizeof(MT_UNF_PVR_PLAY_STATUS_S));
    g_pvr_last_vid_frame_decoded = MT_FALSE;
	g_pvr_last_vid_frame_showed = MT_FALSE;
	g_pvr_last_i_frame = 0;

    PVR_Index_ResetPlayAttr(pChnAttr->IndexHandle);

    for(i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        if ((MT_UNF_PVR_PLAY_STATE_STOP != g_stPvrPlayChns[i].enState) &&
            (MT_UNF_PVR_PLAY_STATE_INVALID != g_stPvrPlayChns[i].enState) &&
            (MT_UNF_PVR_PLAY_STATE_BUTT != g_stPvrPlayChns[i].enState))
        {
            break;
        }
    }
    if (i == PVR_PLAY_MAX_CHN_NUM)
    {
        if(MT_SUCCESS != timer_delete(g_stPlayTimer))
        {
            MT_ERR_PVR("Delete play timer failed!\n");
        }

        g_bPlayTimerInitFlag = MT_FALSE;
    }
    MT_UNF_AVPLAY_UnRegisterEvent(pChnAttr->hAvplay, MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_DECODED);
	MT_UNF_AVPLAY_UnRegisterEvent(pChnAttr->hAvplay, MT_UNF_AVPLAY_EVENT_LAST_VID_FRAME_SHOWED);

    PVR_UNLOCK_PLAY(pChnAttr);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayStartTimeShift
 Description     : start TimeShift
 Input           : pu32PlayChnID   **
                   u32DemuxID   **
                   u32RecChnID  **
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
MT_S32 MT_PVR_PlayStartTimeShift(MT_U32 *pu32PlayChnID,
                                                                    MT_U32 u32RecChnID,
                                                                    MT_HANDLE hAvplay,
                                                                    MT_HANDLE hTsBuffer)
{
    MT_S32 ret;
    MT_U32 u32PlayChnID = 0;
    MT_UNF_PVR_REC_ATTR_S RecAttr;
    MT_UNF_PVR_PLAY_ATTR_S PlayAttr={0};

//    MT_ERR_PVR(" in  id=%d \n",u32PlayChnID);
    PVR_CHECK_POINTER(pu32PlayChnID);

    /* get record channel attribute                                            */
    ret = MT_PVR_RecGetChn(u32RecChnID, &RecAttr);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    /* configure play channel with record channel attributes                   */
    PlayAttr.enStreamType = RecAttr.enStreamType;
    PlayAttr.u32FileNameLen = RecAttr.u32FileNameLen;
    PlayAttr.bIsClearStream = RecAttr.bIsClearStream;
    memset(PlayAttr.szFileName, 0, sizeof(PlayAttr.szFileName));
    strncpy(PlayAttr.szFileName, RecAttr.szFileName, strlen(RecAttr.szFileName));
    PlayAttr.stDecryptCfg.bDoCipher = RecAttr.stEncryptCfg.bDoCipher;
    PlayAttr.stDecryptCfg.enType = RecAttr.stEncryptCfg.enType;
    PlayAttr.stDecryptCfg.u32KeyLen = RecAttr.stEncryptCfg.u32KeyLen;
    PlayAttr.bSupportAdvCa = RecAttr.bSupportAdvCa;  //add
    PlayAttr.u32TimeShiftEventDataUnitSize = RecAttr.u32TimeShiftEventDataUnitSize;
    PlayAttr.bSupportTimeShiftEvent = RecAttr.bSupportTimeShiftEvent;
    memcpy(PlayAttr.stDecryptCfg.au8Key, RecAttr.stEncryptCfg.au8Key, PVR_MAX_CIPHER_KEY_LEN);
    /* apply a new play channel for timeshift playing                          */
    ret = MT_PVR_PlayCreateChn(&u32PlayChnID, &PlayAttr, hAvplay, hTsBuffer);
    if ( MT_SUCCESS != ret)
    {
        return ret;
    }

    /* start timeshift playing                                                 */
    ret = MT_PVR_PlayStartChn(u32PlayChnID);
    if ( MT_SUCCESS != ret)
    {
        (MT_VOID)MT_PVR_PlayDestroyChn(u32PlayChnID);
        return ret;
    }
//    MT_ERR_PVR("  id=%d \n",u32PlayChnID);


    *pu32PlayChnID = u32PlayChnID;
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayStopTimeShift
 Description     : stop TimeShift
 Input           : u32PlayChnID  **
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
MT_S32 MT_PVR_PlayStopTimeShift(MT_U32 u32PlayChnID, const MT_UNF_AVPLAY_STOP_OPT_S *pstStopOpt)
{
    MT_S32 ret;

    PVR_PLAY_CHN_S  *pChnAttr;
    PVR_PLAY_CHECK_CHN(u32PlayChnID);
    pChnAttr = &g_stPvrPlayChns[u32PlayChnID];
    PVR_PLAY_CHECK_CHN_INIT(pChnAttr->enState);

    /* stop timeshift play channel                                             */
    ret = MT_PVR_PlayStopChn(u32PlayChnID, pstStopOpt);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("stop play chn failed:%#x!\n", ret);
        return ret;
    }

    ret = MT_PVR_PlayDestroyChn(u32PlayChnID);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("destroy play chn failed:%#x!\n", ret);
        return ret;
    }

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayPauseChn
 Description     : pause play channel
 Input           : u32Chn  ** channel number
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayPauseChn(MT_U32 u32Chn)
{
    MT_S32 ret;
    PVR_PLAY_CHN_S  *pChnAttr;
    /* when u32Chn is record channel, mean to pause the live stream */
    if (PVR_Rec_IsChnRecording(u32Chn))
    {
        return PVR_Rec_MarkPausePos(u32Chn);
    }
    PVR_ALAWYS_PRINT("%s in\n",__FUNCTION__);
    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply)
        {
            if(p_addon->play_inter.on_pause && MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_pause(p_addon->handlesply)))
            {
                MT_ERR_PVR("pause addon error:%d\n",ret);
                return ret;
            }
        }
    }
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    if (MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }

    if (!((MT_SUCCESS == PVRPlayIsChnTplay(u32Chn))
          || (MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)))
    {
        MT_ERR_PVR("state:%d NOT support Pause!\n", pChnAttr->enState);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_PLAY_INVALID_STATE;
    }

    //(void)PVRPlaySeekToCurFrame(pChnAttr->IndexHandle, pChnAttr, pChnAttr->enState, MT_UNF_PVR_PLAY_STATE_PLAY);

    ret = MT_UNF_AVPLAY_Pause(pChnAttr->hAvplay, NULL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY_Pause ERR:%#x!\n", ret);
        PVR_UNLOCK_PLAY(pChnAttr);
        return ret;
    }

    pChnAttr->bQuickUpdateStatus = MT_TRUE;
    pChnAttr->enLastState = pChnAttr->enState;
    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PAUSE;
    pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;

    MT_WARN_PVR("pause OK!\n");
    PVR_UNLOCK_PLAY(pChnAttr);
    return MT_SUCCESS;

}

/*
fixed Bug #27343,should find the nearby I frame base on u32ReadFrame,play start fromI frame,otherwise may take long time to play out video,
avsync can't get current vpts,,demux tickseek in can't pick out the avsync point and push audio data,than audio es have full,pvr stop push ts data
*/
static MT_S32 MT_PVR_PlayReadFrameToIFrame(PVR_PLAY_CHN_S  *pChnAttr,MT_S32 direct)
{
    PVR_INDEX_ENTRY_S frame={0};
    MT_S32 ret = 0;
    MT_U32 r=0,s=0,e=0,l=0;

    r = pChnAttr->IndexHandle->u32ReadFrame;
    s = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
    e = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
    l = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
    PVRPLAY_LOG_DEBUG_FRAME("----r=%d s=%d e=%d l=%d direct=%d\n",r,s,e,l,direct);
    ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle,&frame,r);
    if(ret == MT_SUCCESS){
        if(PVR_INDEX_is_Iframe(&frame)){
            PVRPLAY_LOG_DEBUG_FRAME("----u32ReadFrame=%d is I Frame\n",r);
            if(r>1){
                pChnAttr->IndexHandle->u32ReadFrame = r -1;//read again for I frame        
            }               
            return MT_SUCCESS;
        }
    }
    
    ret = PVR_Index_GetNearbyIFrameByNum(pChnAttr->IndexHandle,&frame,r,direct);
    if(ret != -1){
        PVRPLAY_LOG_DEBUG_FRAME("----get nearby iframe ret=%d MT_PVR_FIND_IFRAME_DISTANCE=%d-----\n",ret,MT_PVR_FIND_IFRAME_DISTANCE);
        if(ret < MT_PVR_FIND_IFRAME_DISTANCE){
            if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){
                if(direct>0){
                    if(r > e && r + ret > l){
                        pChnAttr->IndexHandle->u32ReadFrame = r + ret - l;
                        PVRPLAY_LOG_DEBUG_FRAME("----rewind case u32ReadFrame=%d-----\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }else if((r > e && r + ret < l) ||  (r < e && r+ ret < e)){
                        pChnAttr->IndexHandle->u32ReadFrame += ret;
                        PVRPLAY_LOG_DEBUG_FRAME("----u32ReadFrame=%d-----\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }
                }else{
                    if(r < e && r < ret+1){
                        pChnAttr->IndexHandle->u32ReadFrame = l - (ret+1-r); //rewind case
                        PVRPLAY_LOG_DEBUG_FRAME("----rewind case u32ReadFrame=%d-----\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }else if((r < e && r > ret+1) ||(r > s && r > ret && r-ret+1 > s)){
                        pChnAttr->IndexHandle->u32ReadFrame -= (ret+1);
                        PVRPLAY_LOG_DEBUG_FRAME("----u32ReadFrame=%d-----\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }
                }
            }else{
                if(direct > 0){
                    if(r+ ret < e){
                        pChnAttr->IndexHandle->u32ReadFrame += ret;
                    }
                }else{
                    if(r > ret+1 ){
                        pChnAttr->IndexHandle->u32ReadFrame -= (ret+1);
                    }
                }
            }
            PVRPLAY_LOG_DEBUG_FRAME("----after find I Frame u32ReadFrame=%d-----\n",pChnAttr->IndexHandle->u32ReadFrame);
        }else{
            MT_ERR_PVR("---find Iframe num=%d >= %d\n",ret,MT_PVR_FIND_IFRAME_DISTANCE);
        }
            
    }else{
        MT_ERR_PVR("---get nearby Iframe failed!\n");
    } 
    return MT_SUCCESS;
}

static MT_S32 MT_PVR_PlayResumeChnProc(MT_U32 u32Chn)
{
    MT_S32 ret;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_HANDLE hWindow;
    MT_U32  pid;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr;
    MT_UNF_AVPLAY_TPLAY_OPT_S stTplayOpts;
    PVR_PLAY_POLICY_S stPvrPlayOpt = {0};
    MT_UNF_PVR_PLAY_SPEED_E lstspeed=MT_UNF_PVR_PLAY_SPEED_NORMAL;
    MT_S32 direct = 0;
    PVR_PLAY_CHECK_CHN(u32Chn);

    memset(&stFrmRateAttr, 0, sizeof(MT_UNF_AVPLAY_FRMRATE_PARAM_S));
    memset(&stTplayOpts, 0, sizeof(MT_UNF_AVPLAY_TPLAY_OPT_S));

    pChnAttr = &g_stPvrPlayChns[u32Chn];
    
    PVR_ALAWYS_PRINT("%s in enState=%d bEndOfFile=%d\n",__FUNCTION__,pChnAttr->enState,pChnAttr->bEndOfFile);
    PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
        pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
        pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply)
        {
            if(p_addon->play_inter.on_resume && MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_resume(p_addon->handlesply)))
            {
                MT_ERR_PVR("resume addon error:%d\n",ret);
                return ret;
            }
        }
    }
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);
    if (MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }

    //for debug use,close trick dump file when resume play     
    if(g_bPvrTrickDump){
        PVRPlayCloseDumpFile();
    }
    
    MT_INFO_PVR("++++++resume.in pChnAttr->bEndOfFile = %d\n",pChnAttr->bEndOfFile);
//    if(pChnAttr->bTillStartOfFile)
//        pChnAttr->bQuickUpdateStatus = TRUE;

    /* 
        among forward play mode, just notice the event when reach to the start, not switch status  
        bEndOfFile is TRUE, timeshift playtorec/sof(s32CycTimes > 0)
    */
    if (pChnAttr->bEndOfFile)
    {//need to recover av-status!
        MT_INFO_PVR("++++++resume.in.eof enSendDataMode %d\n",pChnAttr->enSendDataMode);
        if((MT_UNF_PVR_PLAY_STATE_PAUSE<=pChnAttr->enState) && (MT_UNF_PVR_PLAY_STATE_STEPB>=pChnAttr->enState)){
            pvrplay_clearvframe(pChnAttr);
            ret = MT_UNF_DMX_ResetTSBuffer(pChnAttr->hTsBuffer);
            if(ret != MT_SUCCESS){
                MT_ERR_PVR("AVPLAY_Reset failed:%#x\n", ret);
            }
            ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, MT_NULL);    //fixbug110127
            if(ret != MT_SUCCESS){
                MT_ERR_PVR("AVPLAY_Reset failed:%#x\n", ret);
            }
            ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
            if (ret != MT_SUCCESS)
            {
                MT_ERR_PVR("AVPLAY_Resume failed:%#x\n", ret);
                PVR_UNLOCK_PLAY(pChnAttr);
                return MT_FAILURE;
            }
            ret = MT_UNF_AVPLAY_SetDecodeMode(pChnAttr->hAvplay, MT_UNF_VCODEC_MODE_NORMAL);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("set vdec normal mode error!\n");
                PVR_UNLOCK_PLAY(pChnAttr);
                return MT_FAILURE;
            }
            MT_PVR_PlayGetPolicy(pChnAttr, MT_UNF_PVR_PLAY_SPEED_NORMAL, &stPvrPlayOpt);
            MT_UNF_AVPLAY_DecFrmType(pChnAttr->hAvplay,MT_UNF_DEC_FRM_ALL);
            ret = MT_PVR_PlayDispTPlay(pChnAttr, MT_UNF_PVR_PLAY_SPEED_NORMAL,&(stPvrPlayOpt.stDispFrc));
            if (ret != MT_SUCCESS)
            {
                MT_ERR_PVR("AVPLAY_Resume failed:%#x\n", ret);
                PVR_UNLOCK_PLAY(pChnAttr);
                return MT_FAILURE;
            }

            //fixbug109940
            pChnAttr->bQuickUpdateStatus = MT_TRUE;
            pChnAttr->enLastState = pChnAttr->enState;
            lstspeed=pChnAttr->enSpeed;
            pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;
            pChnAttr->set_eof = MT_FALSE;

            pChnAttr->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
            pChnAttr->enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
            pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PLAY;

            g_pvr_last_vid_frame_decoded = MT_FALSE;
            g_pvr_last_vid_frame_showed = MT_FALSE;
            g_pvr_last_i_frame = 0;

            ret = MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
            if ((MT_SUCCESS == ret) && (0x1fff != pid)){
                MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
            }
            PVR_PLAY_P("---call PVRPlayAvplaySyncCtrl enable av sync\n");
            if (MT_SUCCESS != PVRPlayAvplaySyncCtrl(pChnAttr, MT_TRUE))
            {//bug115924
                PVR_UNLOCK_PLAY(pChnAttr);
                MT_ERR_PVR("Resume Avplay sync fail.\n");
                return MT_FAILURE;
            }
            ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, MT_NULL);
            if(ret != MT_SUCCESS){
                MT_ERR_PVR("AVPLAY_Reset failed:%#x\n", ret);
            }
            if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){//adjust r!
                MT_U32 s = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
                MT_U32 e = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
                MT_U32 l = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
                if(l>0){
                    MT_U32 distance=0;
                    if(e>0){
                        e--;
                    }
                    if(l>0){
                        l--;
                    }
                    if((pChnAttr->IndexHandle->cause_resume==1)||(pChnAttr->IndexHandle->cause_resume==2)){ //to sof
                        pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                        //printf("+++resumexy:%x\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }else if((pChnAttr->IndexHandle->cause_resume==3)||(pChnAttr->IndexHandle->cause_resume==4)){
                        pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_backward(s,e,l,e,50);
                        //printf("+++resumexz:%x\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }else{
                        //if r not in range.if speed>normal,seek to end,if speed<normal,seek to start
                        Pvr_Adjust_ReadInGoodRange(pChnAttr->IndexHandle,(lstspeed>MT_UNF_PVR_PLAY_SPEED_NORMAL)?0:1);
                        distance=Pvr_Calc_distance(s,l,pChnAttr->IndexHandle->u32ReadFrame);
                        if(distance<PVR_TPLAY_MIN_DISTANCE){
                            pChnAttr->IndexHandle->u32ReadFrame = Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                            //printf("++++resumexx %x\n",pChnAttr->IndexHandle->u32ReadFrame);
                        }
                    }
                }
            }

            PVRPLAY_LOG_DEBUG_FRAME("cause_resume=%d lstspeed=%d\n",pChnAttr->IndexHandle->cause_resume,lstspeed);
            if(pChnAttr->IndexHandle->cause_resume == 4 ||pChnAttr->IndexHandle->cause_resume == 1){
                if(pChnAttr->IndexHandle->cause_resume == 4){
                    direct = -1; //find I frame revert
                }else{
                    direct = 1;
                }
                MT_PVR_PlayReadFrameToIFrame(pChnAttr,direct);            
            }
            
            pChnAttr->IndexHandle->cause_resume=0;
            if(MT_ERR_PVR_FILE_TILL_END == pvr_record_current_frame(pChnAttr))
            {
                pvr_modify_current_frame(pChnAttr);
            }
            PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
            pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
            pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
            PVR_UNLOCK_PLAY(pChnAttr);
            
            return MT_SUCCESS;
        }
    }

    ret = MT_MPI_AVPLAY_GetWindowHandle(pChnAttr->hAvplay, &hWindow);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY get window handle failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }
    {
      MT_INFO_PVR("++++++resume.in.will do seek\n");
      if((MT_UNF_PVR_PLAY_STATE_PAUSE != pChnAttr->enState)||
         (MT_UNF_PVR_PLAY_STATE_PLAY!=pChnAttr->enLastState)){//trick to noraml
            MT_U32 stCurPlayFrame;
            MT_U32 stCurDispPts;
            MT_U32 s = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
            MT_U32 e = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
            MT_U32 l = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
            MT_U32 r = pChnAttr->IndexHandle->u32ReadFrame;
            if(0==l){
                PVR_UNLOCK_PLAY(pChnAttr);
                return MT_FAILURE;
            }
            if(e>0){
                e--;
            }
            if(l>0){
                l--;
            }
#ifdef PVR_DEBUG_ON_RUNTIME
            debug_get_displaytime_by_cache = 1;
#endif
            if(MT_SUCCESS==Frame_GetDispTimeByCache(pChnAttr, &stCurPlayFrame,&stCurDispPts,NULL,NULL)){
                PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
                    pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
                    pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
                PVRPLAY_LOG_DEBUG_FRAME("stCurPlayFrame :%d stCurDispPts = %d\n",stCurPlayFrame,stCurDispPts);
                if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){//maybe s.r -> r.s, such as  2xBF
                    MT_U32 distance1=Pvr_Calc_distance(s,l,r);                //before adjust.s--->r
                    MT_U32 distance2=Pvr_Calc_distance(s,l,stCurPlayFrame);   //after  adjust.s--->r
                    MT_U32 diff=0;
                    if(distance2>distance1){
                        diff=distance2-distance1;
                    }else{
                        diff=distance1-distance2;
                    }

                    if(diff>(l/2)){  //it must be cross
                        if(distance1<distance2){  //it's at s
                            pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                            //printf("+++resume43:%x\n",pChnAttr->IndexHandle->u32ReadFrame,r);
                        }else{                    //it's at e
                            pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_backward(s,e,l,e,50);
                            //printf("+++resume44:%x\n",pChnAttr->IndexHandle->u32ReadFrame,r);
                        }
                    }else{
                        if((pChnAttr->IndexHandle->cause_resume==1)||(pChnAttr->IndexHandle->cause_resume==2)){ //to sof
                            pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                            //printf("+++resume45:%x\n",pChnAttr->IndexHandle->u32ReadFrame,r);
                        }else if((pChnAttr->IndexHandle->cause_resume==3)||(pChnAttr->IndexHandle->cause_resume==4)){
                            pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_backward(s,e,l,e,50);
                            //printf("+++resume46:%x\n",pChnAttr->IndexHandle->u32ReadFrame,r);
                        }else{
                            pChnAttr->IndexHandle->u32ReadFrame=stCurPlayFrame;
                            //printf("+++resume47:%x\n",pChnAttr->IndexHandle->u32ReadFrame);
                        }
                    }
                    pChnAttr->IndexHandle->cause_resume=0;
                }else{
                    PVR_PLAY_D("-----u32StartFrame=%d stCurPlayFrame=%d cause_resume=%d\n",s,stCurPlayFrame,pChnAttr->IndexHandle->cause_resume);
                    if(pChnAttr->IndexHandle->cause_resume == 1){
                        pChnAttr->IndexHandle->u32ReadFrame=s;/*resume play from startframe when sof,fix sof resume play may miss I frame*/
                    }else{
                        pChnAttr->IndexHandle->u32ReadFrame=stCurPlayFrame;
                    }
                    PVR_PLAY_D("------u32ReadFrame=%d\n",pChnAttr->IndexHandle->u32ReadFrame);
                    
//                    MT_ERR_PVR("+++resume48:%ld\n",pChnAttr->IndexHandle->u32ReadFrame);
                }
            }else{
                PVR_PLAY_D("Frame_GetDispTimeByCache failed!\n");
                Pvr_Adjust_ReadInGoodRange(pChnAttr->IndexHandle,(lstspeed>MT_UNF_PVR_PLAY_SPEED_NORMAL)?0:1);
                //MT_ERR_PVR("+++av.nr get error\n");
            }

            /*
            fixed #30802,playback ff over, app may seek to head and trick normal speed, pvr may haved sended i frame
            before resume operate, if send frame is not i,may can't resume,if send next i frame, play time may not from 0    
            */
            if(pChnAttr->bEofSeek2Start == MT_TRUE){
                pChnAttr->bEofSeek2Start = MT_FALSE;
                pChnAttr->IndexHandle->u32ReadFrame= s;
                PVR_PLAY_P("bEofSeek2Start set u32ReadFrame to s=%d\n",s);
            }
            
            PVR_PLAY_P("------u32ReadFrame=%d\n",pChnAttr->IndexHandle->u32ReadFrame);
#ifdef PVR_DEBUG_ON_RUNTIME
            debug_get_displaytime_by_cache = 0;
#endif
            MT_INFO_PVR("+++resume.playing pts:%x\n",stCurDispPts);
            pvrplay_clearvframe(pChnAttr);
            ret = MT_UNF_DMX_ResetTSBuffer(pChnAttr->hTsBuffer);
            if(ret != MT_SUCCESS){
                MT_ERR_PVR("AVPLAY_Reset failed:%#x\n", ret);
            }
            ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, MT_NULL);        //to clear cached pictures
            if(ret != MT_SUCCESS){
                MT_ERR_PVR("AVPLAY_Reset failed:%#x\n", ret);
            }
        }
    }
    ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
      if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Resume avplay failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    if((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)&&
       (MT_UNF_PVR_PLAY_STATE_PLAY==pChnAttr->enLastState))
    {//play->pause->play
        MT_INFO_PVR("pause.resume ok\n");
        pChnAttr->bQuickUpdateStatus = MT_TRUE;
        pChnAttr->enLastState = pChnAttr->enState;
        lstspeed=pChnAttr->enSpeed;
        pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;

        pChnAttr->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
        pChnAttr->enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
        pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PLAY;
        PVR_PLAY_D("--- enState set to MT_UNF_PVR_PLAY_STATE_PLAY enLastState=%d\n",pChnAttr->enLastState);

        if (PVR_Index_QureyClearRecReachPlay(pChnAttr->IndexHandle)){
            pvrplay_clearvframe(pChnAttr);
            PVR_Index_SeekToStart(pChnAttr->IndexHandle);
            ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, NULL);
            if(ret){
                MT_ERR_PVR("Resume reset failure!\n");
            }
            //printf("++resume.pause to resume jump\n");
        }
        pChnAttr->pause2resume_forirdeto=1;
        PVR_UNLOCK_PLAY(pChnAttr);
        PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
            pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
            pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
        return MT_SUCCESS;
    }
    //ret = PVRPlaySetWinRate(hWindow, (MT_UNF_PVR_PLAY_SPEED_E)(MT_UNF_PVR_PLAY_SPEED_NORMAL/4));
    //MT_INFO_PVR("resume play PVRPlaySetWinRate result is %d\n", ret);
    MT_INFO_PVR("++++++resume.in.will do more\n");
    ret = MT_UNF_AVPLAY_SetDecodeMode(pChnAttr->hAvplay, MT_UNF_VCODEC_MODE_NORMAL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("set vdec normal mode error!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PLAY;
    PVR_PLAY_D("--- enState set to MT_UNF_PVR_PLAY_STATE_PLAY\n");
    /*on n->p-n situation, it's will appear repeat play issue, normal play also need to reset buffer*/
    /*except of pause state, all the other state switch to play need to reset it to current playing frame
    else if ((MT_UNF_PVR_PLAY_STATE_PAUSE != pChnAttr->enState)
      || ((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_PLAY != pChnAttr->enLastState)))*/

      //only reset to current frame when play direct change or the previous send data mode is not equal to PVR_PLAY_SEND_DATA_ALL
      if((pChnAttr->enTplayDirect == MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD) || (pChnAttr->enSendDataMode != PVR_PLAY_SEND_DATA_ALL))
    {
        MT_INFO_PVR("to reset buffer and player.\n");
        ret = PVRPlayResetToCurFrame(pChnAttr->IndexHandle, pChnAttr, MT_UNF_PVR_PLAY_STATE_PLAY);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("reset to current frame failed!\n");
        }
        pChnAttr->enFBTimeCtrlLastSpeed = MT_UNF_PVR_PLAY_SPEED_BUTT;
    }
    MT_INFO_PVR("++++++resume.in.will resume audio\n");
    ret = MT_UNF_AVPLAY_GetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
    if ((MT_SUCCESS != ret) || (0x1fff == pid))
    {
        MT_WARN_PVR("has not audio stream!\n");
    }
    else
    {
        ret = MT_UNF_AVPLAY_Start(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
        if (MT_SUCCESS != ret)
        {
            PVR_UNLOCK_PLAY(pChnAttr);
            MT_ERR_PVR("Can't start audio, error:%#x!\n", ret);
            return ret;
        }
        else
        {
            MT_INFO_PVR("MT_UNF_AVPLAY_start audio ok!\n");
        }
    }

    ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
    if (ret != MT_SUCCESS)
    {
        MT_ERR_PVR("AVPLAY_Resume failed:%#x\n", ret);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    MT_PVR_PlayGetPolicy(pChnAttr, MT_UNF_PVR_PLAY_SPEED_NORMAL, &stPvrPlayOpt);
    MT_UNF_AVPLAY_DecFrmType(pChnAttr->hAvplay,MT_UNF_DEC_FRM_ALL);
    ret = MT_PVR_PlayDispTPlay(pChnAttr, MT_UNF_PVR_PLAY_SPEED_NORMAL,&(stPvrPlayOpt.stDispFrc));

    pChnAttr->bQuickUpdateStatus = MT_TRUE;
    pChnAttr->enLastState = pChnAttr->enState;
    lstspeed=pChnAttr->enSpeed;
    pChnAttr->enSpeed = MT_UNF_PVR_PLAY_SPEED_NORMAL;

    pChnAttr->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
    pChnAttr->enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
    pChnAttr->stTplayCtlInfo.u32RefFrmPtsMs = 0;
    PVR_PLAY_P("---call PVRPlayAvplaySyncCtrl enable av sync\n");
    if (MT_SUCCESS != PVRPlayAvplaySyncCtrl(pChnAttr, MT_TRUE))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("Resume Avplay sync fail.\n");
        return MT_FAILURE;
    }
    MT_WARN_PVR("resume OK!\n");
    pvrplay_clearvframe(pChnAttr);
    MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, MT_NULL);

    //cause_resume is 0 mean ff/fb to resume when not sof/eof
    if(pChnAttr->IndexHandle->cause_resume == 0){
        MT_PVR_PlayReadFrameToIFrame(pChnAttr,1);
    }
    //cause_resume is 3, playback eof and may seek to head resume play
    else if( pChnAttr->IndexHandle->cause_resume == 3){
        MT_PVR_PlayReadFrameToIFrame(pChnAttr,-1);
    }

    pChnAttr->IndexHandle->cause_resume=0;
    
    if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){  //check agin.
        MT_U32 s = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
        MT_U32 e = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
        MT_U32 l = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
        MT_U32 distance;
        if(l>0){
            if(e>0){
                e--;
            }
            if(l>0){
                l--;
            }
            distance=Pvr_Calc_distance(s,l,pChnAttr->IndexHandle->u32ReadFrame);
            if(distance<PVR_TPLAY_MIN_DISTANCE){
                pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
//                MT_ERR_PVR("+++resume.48:%x\n",pChnAttr->IndexHandle->u32ReadFrame);
            }
        }
    }
    if(MT_ERR_PVR_FILE_TILL_END == pvr_record_current_frame(pChnAttr))
    {
        pvr_modify_current_frame(pChnAttr);
    }
    
    PVR_UNLOCK_PLAY(pChnAttr);
    PVR_ALAWYS_PRINT("%s out cause_resume=%d\n",__FUNCTION__,pChnAttr->IndexHandle->cause_resume);
    PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
        pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
        pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
    return MT_SUCCESS;

}

/*****************************************************************************
 Prototype       : MT_PVR_PlayResumeChn
 Description     : resume the play channel
 Input           : u32Chn  **channel number
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayResumeChn(MT_U32 u32Chn)
{
    MT_S32 ret = 0,res = 0;
    PVR_PLAY_CHN_S  *pChnAttr=NULL;
    MT_UNF_PVR_PLAY_STATE_E cur_state; 

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];

    /*
    it's must be called before MT_PVR_PlayResumeChn, if not, audio es data
    will be updated to adec during the brief period of time between them,
    that not allowed.
    */
    /* add judge the cause_resume is not SOF/rec to start/EOF/play reach rec for Bug #23853 */
    //if (0 == pChnAttr->IndexHandle->cause_resume) //not need now, handle on PVRPlayMainRoute
    cur_state = pChnAttr->enState;
    if (cur_state != MT_UNF_PVR_PLAY_STATE_PAUSE)
    {
        PVR_PLAY_D("----------call trick seek in\n");   
        res = MT_UNF_AVPLAY_TrickSeekIn(pChnAttr->hAvplay);

        if (MT_SUCCESS != res)
        {
            MT_ERR_PVR("stat trick or seek error:%#x\n", res);
            return res;
        }
    }
    PVR_PLAY_D("----------call resume chn proc\n");
    ret = MT_PVR_PlayResumeChnProc(u32Chn);

    //if (0 == pChnAttr->IndexHandle->cause_resume)
    if (cur_state != MT_UNF_PVR_PLAY_STATE_PAUSE)
    {
        PVR_PLAY_D("----------call trick seek out\n");   
        res = MT_UNF_AVPLAY_TrickSeekOut(pChnAttr->hAvplay);

        if (MT_SUCCESS != res)
        {
            MT_ERR_PVR("end trick or seek error:%#x\n", res);
            return res;
        }
    }
    
    return ret;

}

MT_S32 MT_PVR_GetAvHandle(MT_U32 u32Chn, MT_U32  *avhandle)
{
    PVR_PLAY_CHN_S  *pChnAttr=NULL;
    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];

    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    *avhandle=pChnAttr->hAvplay;

    PVR_UNLOCK_PLAY(pChnAttr);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_TPlayPauseResume(MT_U32 u32Chn, MT_BOOL bPause)
{
    MT_S32 ret = MT_SUCCESS;
    PVR_PLAY_CHN_S  *pChnAttr;

    PVR_ALAWYS_PRINT("%s in\n",__FUNCTION__);

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];

    mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();

    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    if(bPause)
    {
        if(p_addon && p_addon->handlesply && p_addon->play_inter.on_pause
            && MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_pause(p_addon->handlesply)))
        {
            MT_ERR_PVR("pause addon error:%d\n",ret);
            goto RET;
        }


        ret = MT_UNF_AVPLAY_Pause(pChnAttr->hAvplay, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("AVPLAY_Pause ERR:%#x!\n", ret);
            goto RET;
        }

        pChnAttr->bQuickUpdateStatus = MT_TRUE;
        pChnAttr->enLastState = pChnAttr->enState;
        pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PAUSE;

    }
    else
    {
        if(p_addon && p_addon->handlesply && p_addon->play_inter.on_resume
            && MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_resume(p_addon->handlesply)))
        {
            MT_ERR_PVR("resume addon error:%d\n",ret);
            goto RET;
        }

        ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("Resume avplay failed!\n");
            goto RET;
        }

        pChnAttr->enState = pChnAttr->enLastState;

    }

RET:
    PVR_UNLOCK_PLAY(pChnAttr);
    return ret;

}


/*****************************************************************************
 Prototype       : MT_PVR_PlayTrickMode
 Description     : set the play mode of play channel
 Input           : u32Chn         **channel number
                   pTrickMode  **play mode, trick mode
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayTrickMode(MT_U32 u32Chn, const MT_UNF_PVR_PLAY_MODE_S *pTrickMode)
{
    MT_S32 ret;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_UNF_PVR_PLAY_STATE_E stateToSet;
    MT_UNF_AVPLAY_TPLAY_OPT_S stTPlayOpt;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr;
    PVR_PLAY_POLICY_S stPvrPlayOpt = {0};
    MT_UNF_DEC_FRM_TYPE_E decfrmtype;

    PVR_CHECK_POINTER(pTrickMode);
    PVR_PLAY_CHECK_CHN(u32Chn);
    MT_ERR_PVR("&&&& %s in %d\n",__FUNCTION__,pTrickMode->enSpeed);
    memset(&stTPlayOpt, 0, sizeof(MT_UNF_AVPLAY_TPLAY_OPT_S));
    memset(&stFrmRateAttr, 0, sizeof(MT_UNF_AVPLAY_FRMRATE_PARAM_S));
    pChnAttr = &g_stPvrPlayChns[u32Chn];

    //fixed set the same ff/fb speed, video will freeze 
    if(pChnAttr && pChnAttr->enSpeed == pTrickMode->enSpeed &&
        MT_UNF_PVR_PLAY_SPEED_NORMAL != pChnAttr->enSpeed) //fixed Bug #28379 video can't resume play, if the state is STEPF, app may call trickmode set the speed is normal 
    {
        MT_ERR_PVR("set speed=%d is same as current speed=%d,not need set,return success.\n",pChnAttr->enSpeed,pTrickMode->enSpeed);
        return MT_SUCCESS;
    }
    
    /*set the mode is rate one, that should be equal to resume */
    if (MT_UNF_PVR_PLAY_SPEED_NORMAL == pTrickMode->enSpeed) /* switch to the normal mode */
    {

        PVR_PLAY_D("----------call resume chn\n");
        ret = MT_PVR_PlayResumeChn(u32Chn);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("pvr resume to normal error:%d\n", ret);
            return ret;
        }
		MT_MPI_AVPLAY_set_trickmode(pChnAttr->hAvplay, 0);
        return ret;
    }

	MT_MPI_AVPLAY_set_trickmode(pChnAttr->hAvplay, 1);
//    MT_ERR_PVR("&&&& first pChnAttr->IndexHandle->u32ReadFrame = %d enState = %d enLastState=%d\n",pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->enState,pChnAttr->enLastState);
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply)
        {
            if(p_addon->play_inter.on_trickplay &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_trickplay(p_addon->handlesply,pTrickMode->enSpeed)))
            {
                MT_ERR_PVR("trickplay addon error:%d\n",ret);
                return ret;
            }
        }
    }

    if( (pChnAttr->enState >= MT_UNF_PVR_PLAY_STATE_PLAY && pChnAttr->enState<= MT_UNF_PVR_PLAY_STATE_STEPB) )
    {//if can't get framerate from av.wait 2s.beacuse app run tp quickly
        int i=0,count=20;
        for(i=0; i<count; i++){
            if(0xffffffff != pChnAttr->u32FrameRate_fromav){
                break;
            }
            if (MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
            {
                pChnAttr->u32FrameRate_fromav=25;//set default-value
                break;
            }
            MT_USLEEP(100000);//100ms
        }
        if(i==count){
            pChnAttr->u32FrameRate_fromav=25;//set default-value
        }
    }else
    {
        pChnAttr->u32FrameRate_fromav=25;//set default-value
    }
    PVR_GET_STATE_BY_SPEED(stateToSet, pTrickMode->enSpeed);
    if (MT_UNF_PVR_PLAY_STATE_INVALID == stateToSet)
    {
        MT_ERR_PVR("NOT support this trick mode.\n");
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    /* audio index file not support Tplay*/
    if (PVR_INDEX_IS_TYPE_AUDIO(pChnAttr->IndexHandle))
    {
        MT_ERR_PVR("audio indexed stream NOT support trick mode.\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }
    PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
        pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
        pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
    {
        MT_U32 stCurPlayFrame;
        MT_U32 stCurDispPts;
        MT_U32 s = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
        MT_U32 e = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
        MT_U32 l = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
        if(0==l){
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_ERR_PVR_NOT_SUPPORT;
        }
        if(e>0){
            e--;
        }
        if(l>0){
            l--;
        }
        //if(PVR_INDEX_INVALID_PTSMS !=pChnAttr->stTplayCtlInfo.u32RefFrmPtsMs) //fixed Bug #25357
        {
            if(MT_SUCCESS==Frame_GetDispTimeByCache(pChnAttr, &stCurPlayFrame,&stCurDispPts,NULL,NULL)){
                if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){//maybe s.r -> r.s, such as  2xBF
                    MT_U32 r = pChnAttr->IndexHandle->u32ReadFrame;
                    MT_U32 distance1=Pvr_Calc_distance(s,l,r);                //before adjust.s--->r, read frame
                    MT_U32 distance2=Pvr_Calc_distance(s,l,stCurPlayFrame);   //after  adjust.s--->r, play frame
                    
                    PVRPLAY_LOG_DEBUG_FRAME("stCurPlayFrame=%d stCurDispPts=%d distance1=%d distance2=%d\n",stCurPlayFrame,stCurDispPts,distance1,distance2);
                    
                    MT_U32 diff=0;
                    if(distance2>distance1){
                        diff=distance2-distance1;
                    }else{
                        diff=distance1-distance2;
                    }
                    // this judge is not well,some case will run in this case,such as Seek to End, 2xFB->4XFB: 0.p.e.s.r.l
                    if(diff>(l/2)){  //it must be cross
                        if(distance1<distance2 && pChnAttr->enSpeed > 0){  //it's at s, such as normal speed s.p.r -> p.s.r 
                            pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                            MT_ERR_PVR("+++resume.T53: cross adjust u32ReadFrame=%d r=%d\n",pChnAttr->IndexHandle->u32ReadFrame,r);
                        }
                        
                        else{                    //it's at e
                            #if 0 //disable these code,work not well,such as fb may run in this case, s.r.p -> r.s.p
                            pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_backward(s,e,l,e,50);
                            MT_ERR_PVR("+++resume.T54: cross adjust u32ReadFrame=%d r=%d\n",pChnAttr->IndexHandle->u32ReadFrame,r);
                            #else
                            pChnAttr->IndexHandle->u32ReadFrame=stCurPlayFrame;
                            PVRPLAY_LOG_DEBUG_FRAME("+++resume.T54:%d\n",pChnAttr->IndexHandle->u32ReadFrame);
                            #endif
                        }
                        
                    }else{
                        pChnAttr->IndexHandle->u32ReadFrame=stCurPlayFrame;
                        PVRPLAY_LOG_DEBUG_FRAME("+++resume.T55:%d\n",pChnAttr->IndexHandle->u32ReadFrame);
                    }
                }else{
                    pChnAttr->IndexHandle->u32ReadFrame=stCurPlayFrame;
                    PVRPLAY_LOG_DEBUG_FRAME("+++resume.T56:%d\n",pChnAttr->IndexHandle->u32ReadFrame);
                }
            }else{
                if((pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes)&&(MT_UNF_PVR_PLAY_STATE_PLAY==pChnAttr->enState)){
                    pChnAttr->IndexHandle->u32ReadFrame=pChnAttr->current_playing_frame;
                    //pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                    PVRPLAY_LOG_DEBUG_FRAME("+++resume.T57:%d current_playing_frame=%d\n",pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->current_playing_frame);
                }
                MT_ERR_PVR("+++av.nr get error u32ReadFrame = %d\n",pChnAttr->IndexHandle->u32ReadFrame);
            }
        }
        MT_INFO_PVR("+++resume.playing pts:%x\n",stCurDispPts);
        pvrplay_clearvframe(pChnAttr);
        MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, MT_NULL);        //to clear cached pictures
        PVR_PLAY_D("-----------------trick play clear vframe and avplay reset finish!\n");
    }
    PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
        pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
        pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
    if (MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enLastState)
    {
        pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_PAUSE;
    }

    /* after playing, any play state can be switched to Tplay */
    if ((pChnAttr->enState < MT_UNF_PVR_PLAY_STATE_PLAY)
        || (pChnAttr->enState > MT_UNF_PVR_PLAY_STATE_STEPB))
    {
        MT_ERR_PVR("State now:%d, NOT support TPlay!\n", pChnAttr->enState);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_PLAY_INVALID_STATE;
    }

    if (MT_TRUE == pChnAttr->bPlayingTsNoIdx)
    {
        MT_ERR_PVR("No index file, NOT support trick mode.\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    /* scramble stream not support Tplay */
    if (!pChnAttr->stUserCfg.bIsClearStream)
    {
        MT_ERR_PVR("scrambed stream NOT support trick mode !\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    if ((pChnAttr->enState == stateToSet) && (pChnAttr->enSpeed == pTrickMode->enSpeed))
    {
        MT_WARN_PVR("Set the same speed: %d\n", pTrickMode->enSpeed);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }

    if ((pChnAttr->enState == (MT_UNF_PVR_PLAY_STATE_E)MT_UNF_PVR_PLAY_STATE_PLAY) &&
        ((stateToSet == MT_UNF_PVR_PLAY_STATE_FB) ||
         (stateToSet == MT_UNF_PVR_PLAY_STATE_FF) ||
         (stateToSet == MT_UNF_PVR_PLAY_STATE_SF)))
    {
        PVR_PLAY_P("---call PVRPlayAvplaySyncCtrl close av sync\n");
        if (MT_SUCCESS != PVRPlayAvplaySyncCtrl(pChnAttr, MT_FALSE))
        {
            MT_ERR_PVR("Close avplay sync fail!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
    }

    /* between forward play and  backword play, just notice the event when reach to the start, not switch status  */
    if (((pChnAttr->bEndOfFile) && (pChnAttr->bTillStartOfFile == MT_TRUE))
        /*&& (PVR_IS_PLAY_BACKWARD(pChnAttr->enState, pChnAttr->enLastState))  */
        && ((MT_UNF_PVR_PLAY_STATE_FB == stateToSet) || (MT_UNF_PVR_PLAY_STATE_STEPB == stateToSet)))
    {
        MT_INFO_PVR("need not start main rout, state=%d, laststate=%d!\n", stateToSet, pChnAttr->enState);
        pChnAttr->bQuickUpdateStatus = MT_FALSE;
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }
    else if (((pChnAttr->bEndOfFile) && (pChnAttr->bTillStartOfFile == MT_FALSE))
        /*&& (PVR_IS_PLAY_FORWARD(pChnAttr->enState, pChnAttr->enLastState))  */
        && ((MT_UNF_PVR_PLAY_STATE_FF == stateToSet)
            || (MT_UNF_PVR_PLAY_STATE_SF == stateToSet)
            || (MT_UNF_PVR_PLAY_STATE_PLAY == stateToSet)
            || (MT_UNF_PVR_PLAY_STATE_STEPF == stateToSet)))
    {
        MT_INFO_PVR("need not start main rout, state=%d, laststate=%d!\n", stateToSet, pChnAttr->enState);
        pChnAttr->bQuickUpdateStatus = MT_FALSE;
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }

    MT_PVR_PlayGetPolicy(pChnAttr, pTrickMode->enSpeed, &stPvrPlayOpt);
//    MT_ERR_PVR("&&&& %s in %d\n",__FUNCTION__,pTrickMode->enSpeed);

    /* before and after pause, state invariable, not need to reset */
    if ((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
        && (pChnAttr->enLastState == stateToSet))
    {
        MT_WARN_PVR("State not change before and after pause:state=%d!\n", pChnAttr->enState);
    }
    /* among PLAY/SF/STEPF state, not need to reset */
    else if (((MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_SF == stateToSet))
      || ((MT_UNF_PVR_PLAY_STATE_STEPF == pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_SF == stateToSet)))
    {
        MT_WARN_PVR("Change state from %d to %d!\n", pChnAttr->enState, stateToSet);
    }
    /* before and after pause,among PLAY/SF/STEPF state,not need to reset  */
    else if (((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enLastState)
        && (MT_UNF_PVR_PLAY_STATE_SF == stateToSet))
      || ((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_STEPF == pChnAttr->enLastState)
        && (MT_UNF_PVR_PLAY_STATE_SF == stateToSet)))
    {
        MT_WARN_PVR("Change state from %d to %d, after pause!\n", pChnAttr->enLastState, stateToSet);
    }
    /* other state need to reset the current playing frame */
    else
    {
        MT_INFO_PVR("\r\n ###############direct:%d,%d, mode:%d", pChnAttr->enTplayDirect, stPvrPlayOpt.enTplayDirect, pChnAttr->enSendDataMode);
        //only reset to current frame when play direct change or the previous send data mode is not equal to PVR_PLAY_SEND_DATA_ALL
        if((pChnAttr->enTplayDirect != stPvrPlayOpt.enTplayDirect) || (pChnAttr->enSendDataMode != PVR_PLAY_SEND_DATA_ALL))
        {
        MT_INFO_PVR("\r\n #################reset to current frame");
            ret = PVRPlayResetToCurFrame(pChnAttr->IndexHandle, pChnAttr, stateToSet);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("reset to current frame failed!\n");
            }
        }
    }

    if(MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
    {
        ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("Resume avplay failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
    }
#if 0  //for tts 
    /* video trick mode, stop the audio */
    ret = MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY stop audio failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }
#endif


    /* forward or backward trick mode need to set I frame mode */
    if ((MT_UNF_PVR_PLAY_STATE_FF == stateToSet)
        || (MT_UNF_PVR_PLAY_STATE_FB == stateToSet)
        || (MT_UNF_PVR_PLAY_STATE_SF == stateToSet))
    {
        if(PVR_PLAY_SEND_DATA_ALL==stPvrPlayOpt.enSendDataMode){
            decfrmtype=MT_UNF_DEC_FRM_ALL;
        }else if(PVR_PLAY_SEND_DATA_IP==stPvrPlayOpt.enSendDataMode){
            decfrmtype=MT_UNF_DEC_FRM_IP;
        }else if((PVR_PLAY_SEND_DATA_I==stPvrPlayOpt.enSendDataMode)||
                 (PVR_PLAY_SEND_DATA_SKIP_I==stPvrPlayOpt.enSendDataMode)){
            decfrmtype=MT_UNF_DEC_FRM_I;
            if(PVR_PLAY_SEND_DATA_SKIP_I==stPvrPlayOpt.enSendDataMode){
                PVR_PLAY_P("u32FrameRate=%d u32FrameRate_fromav=%d field_encode=%d stateToSet=%d\n",
                    pChnAttr->u32FrameRate,pChnAttr->u32FrameRate_fromav,pChnAttr->IndexHandle->field_encode,stateToSet);

                if(stateToSet != MT_UNF_PVR_PLAY_STATE_FB){//fixed #31469
                    if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
                               (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){       
                        //IP filed encode need decode IP frame
                        if(pChnAttr->IndexHandle->field_encode == FIELD_ENCODE_IP && pChnAttr->u32IPCnt != pChnAttr->u32ICnt){//fixed #32764
                            decfrmtype=MT_UNF_DEC_FRM_IP;
                            PVR_PLAY_P("IP filed encode,decfrmtype set to IP\n");
                        }                       
                    }
                }
                
            }
        }else{
            decfrmtype=MT_UNF_DEC_FRM_ALL;
        }
        PVR_PLAY_P("%s %d enSpeed=%d stDispFrc=%d.%d decfrmtype=%d\n",__func__,__LINE__,pTrickMode->enSpeed,stPvrPlayOpt.stDispFrc.u32fpsInteger,stPvrPlayOpt.stDispFrc.u32fpsDecimal,decfrmtype);
        MT_UNF_AVPLAY_DecFrmType(pChnAttr->hAvplay,decfrmtype);
            ret = MT_PVR_PlayDispTPlay(pChnAttr, pTrickMode->enSpeed, &(stPvrPlayOpt.stDispFrc));
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("AVPLAY Tplay failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
    }
    else
    {
        MT_ERR_PVR("NOT support this trick mode.\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    pChnAttr->bQuickUpdateStatus = MT_TRUE;
    pChnAttr->stTplayCtlInfo.u32RefFrmPtsMs = 0;
    pChnAttr->enLastState = pChnAttr->enState;
    pChnAttr->enState = stateToSet;
    pChnAttr->enSpeed = pTrickMode->enSpeed;
    pChnAttr->enSendDataMode = stPvrPlayOpt.enSendDataMode;
    pChnAttr->u32SkipPCnt_EveryI = stPvrPlayOpt.u32SkipPCnt_EveryI;
    pChnAttr->u32SkipPMore_EveryxI = stPvrPlayOpt.u32SkipPMore_EveryxI;
    pChnAttr->enTplayDirect = stPvrPlayOpt.enTplayDirect;
    pChnAttr->u32FrameRate_trick=stPvrPlayOpt.stDispFrc.u32fpsInteger;

    pChnAttr->set_eof = MT_FALSE;
    g_pvr_last_vid_frame_decoded = MT_FALSE;
    g_pvr_last_vid_frame_showed = MT_FALSE;
    g_pvr_last_i_frame = 0;


    if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){  //check agin.
//        MT_ERR_PVR("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);
        MT_U32 s = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
        MT_U32 e = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
        MT_U32 l = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;
        MT_U32 distance;
        if(0<l){
            if(e>0){
                e--;
            }
            if(l>0){
                l--;
            }
            distance=Pvr_Calc_distance(s,l,pChnAttr->IndexHandle->u32ReadFrame);
            if(distance<PVR_TPLAY_MIN_DISTANCE){
                pChnAttr->IndexHandle->u32ReadFrame=Pvr_Move_Readpos_forward(s,e,l,s,PVR_TPLAY_MIN_DISTANCE);
                MT_ERR_PVR("+++resume.T58:%d\n",pChnAttr->IndexHandle->u32ReadFrame);
            }
        }
    }
    pvr_record_current_frame(pChnAttr);
    pChnAttr->IndexHandle->cause_resume=0;

    if(g_bPvrTrickDump){
        PVRPlayCloseDumpFile();

        MT_CHAR dump_name[2*PVR_MAX_FILENAME_LEN]={0};
        snprintf(dump_name, sizeof(dump_name), "%s_dump_%d.ts", (MT_CHAR *)&pChnAttr->stUserCfg.szFileName[0],pChnAttr->enSpeed);
        g_pvrfpSend = fopen(dump_name, "wb");
        if(g_pvrfpSend){
            PVR_PLAY_P("pvr play open dump trick ts %s ok!\n",dump_name);
        }else{
            MT_ERR_PVR("pvr play open dump file %s failed!\n",dump_name);
        }
    }
    
    PVR_UNLOCK_PLAY(pChnAttr);

    PVRPLAY_LOG_DEBUG_FRAME("pChnAttr->IndexHandle R: %d, S:%d E:%d L:%d state = %d\n",pChnAttr->IndexHandle->u32ReadFrame,
        pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
        pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->enState);

    return ret;
}


/*****************************************************************************
 Prototype       : MT_PVR_PlaySeek
 Description     : seek to play
 Input           : u32Chn        **channel number
                   pPosition  **the position to play
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlaySeek(MT_U32 u32Chn, const MT_UNF_PVR_PLAY_POSITION_S *pPosition)
{
    MT_S32 ret = MT_SUCCESS;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_U32 u32CurPlayTime = 0;
    MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S stAvplayPrivInfo;

    PVR_CHECK_POINTER(pPosition);
    PVR_PLAY_CHECK_CHN(u32Chn);
    PVR_ALAWYS_PRINT("%s in %x,%x,%llx\n",__FUNCTION__,pPosition->enPositionType,pPosition->s32Whence,pPosition->s64Offset);
    memset(&stAvplayPrivInfo, 0 ,sizeof(MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S));

    pChnAttr = &g_stPvrPlayChns[u32Chn];
    if(NULL != mt_pvr_get_ca_addon())
    {
        mt_pvr_addon_t *p_addon = mt_pvr_get_ca_addon();
        if(p_addon->handlesply)
        {
            if(p_addon->play_inter.on_seek &&
            MPVR_ADDON_STATUS_OK != (ret = p_addon->play_inter.on_seek(p_addon->handlesply,pPosition)))
            {
                MT_ERR_PVR("seek addon error:%d\n",ret);
                return ret;
            }
        }
    }
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);
    PVR_ALAWYS_PRINT("%s %d\n",__FUNCTION__,__LINE__);
    /* for scramble stream, not support seek, presently */
    if (!pChnAttr->stUserCfg.bIsClearStream)
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        /* TODO: about the scramble stream */
        MT_ERR_PVR("Not support scram ts to seek!\n");
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    if (MT_UNF_PVR_PLAY_POS_TYPE_SIZE == pPosition->enPositionType)
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("Not support seek by size!\n");
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    if (MT_TRUE == pChnAttr->bPlayingTsNoIdx)
    {
        MT_ERR_PVR("No index file, NOT support seek.\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    if ((pPosition->s32Whence == SEEK_SET) && (pPosition->s64Offset < 0))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("seek from start, offset error: %lld!\n", pPosition->s64Offset);
        return MT_ERR_PVR_INVALID_PARA;
    }

    if ((pPosition->s32Whence == SEEK_END) && (pPosition->s64Offset > 0))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_ERR_PVR("seek from end, offset error: %lld!\n", pPosition->s64Offset);
        return MT_ERR_PVR_INVALID_PARA;
    }

    if ((pPosition->s32Whence == SEEK_CUR) && (pPosition->s64Offset == 0))
    {
        PVR_UNLOCK_PLAY(pChnAttr);
        MT_WARN_PVR("seek from current, offset is %lld!\n", pPosition->s64Offset);
        return MT_SUCCESS;
    }

    MT_INFO_PVR("Seek: type:%s, whence:%s, offset:%lld. \n",
        MT_UNF_PVR_PLAY_POS_TYPE_TIME == pPosition->enPositionType ? "TIME" : "Frame",
        WHENCE_STRING(pPosition->s32Whence),
        pPosition->s64Offset);

//    MT_ERR_PVR("pPosition->enPositionType = %d. pChnAttr->bRecordedVideoExist = %d \n",pPosition->enPositionType,pChnAttr->bRecordedVideoExist);
    if (MT_UNF_PVR_PLAY_POS_TYPE_TIME == pPosition->enPositionType)
    {
        if ( pChnAttr->bRecordedVideoExist == MT_TRUE)
        {
         //get now avpts!
            MT_U32 u32Flag_getbadframeid=0;
            MT_U32 u32CurFrm;
            MT_U32 u32CurPlayTimeInMs;
            PVR_INDEX_ENTRY_S stCurPlayFrame;

            PVR_PLAY_D("s=%d e=%d l=%d u32ReadFrame=%d\n",pChnAttr->IndexHandle->stCycMgr.u32StartFrame,
                pChnAttr->IndexHandle->stCycMgr.u32EndFrame,pChnAttr->IndexHandle->stCycMgr.u32LastFrame,pChnAttr->IndexHandle->u32ReadFrame);
            
            if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == pChnAttr->IndexHandle->enIndexType){
                MT_UNF_AVPLAY_STATUS_INFO_S playinfo;
                if(MT_SUCCESS==MT_UNF_AVPLAY_GetAudioStatusInfo(pChnAttr->hAvplay, &playinfo)){
                    if(MT_SUCCESS==frame_gettimebysize(pChnAttr,playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize,&u32CurFrm,&u32CurPlayTimeInMs,1,NULL)){
                        ;
                    }else{
                        u32CurPlayTime = pChnAttr->u32CurPlayTimeMs;
                        u32Flag_getbadframeid=1;
                    }
                }else{
                    u32CurPlayTime = pChnAttr->u32CurPlayTimeMs;
                    u32Flag_getbadframeid=1;
                }
            }else{
                if(MT_SUCCESS !=Frame_GetDispTimeByCache(pChnAttr,&u32CurFrm,&u32CurPlayTimeInMs,NULL,NULL)){
                    u32CurPlayTime = pChnAttr->u32CurPlayTimeMs;
                    u32Flag_getbadframeid=1;
                    //MT_ERR_PVR("pChnAttr->u32CurPlayTimeMs = %d.\n",pChnAttr->u32CurPlayTimeMs);
                    PVR_PLAY_D("get disp time by cache failed,u32CurPlayTime=%d\n",pChnAttr->u32CurPlayTimeMs);
                }
            }
            if(0==u32Flag_getbadframeid){
                ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stCurPlayFrame, u32CurFrm);
                if(MT_SUCCESS == ret){
                    u32CurPlayTime = stCurPlayFrame.u32DisplayTimeMs;
                    PVR_PLAY_D("u32CurFrm=%d type=%ld u32CurPlayTime=%d\n",u32CurFrm,PVR_INDEX_get_frameType(&stCurPlayFrame),u32CurPlayTime);
                    MT_INFO_PVR("+++get %d,%d\n",u32CurFrm,u32CurPlayTime);
                }else{
                    u32CurPlayTime = pChnAttr->u32CurPlayTimeMs;
                }
            }

        }
        else
        {
            u32CurPlayTime = pChnAttr->u32CurPlayTimeMs;
        }

    }
    if ((MT_UNF_PVR_PLAY_STATE_PLAY == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_FF == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_FB == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_SF == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_STEPF == pChnAttr->enState)
        || (MT_UNF_PVR_PLAY_STATE_STEPB == pChnAttr->enState))
        /*&& (0 != u32CurPlayTime)*/ /*)*/ /*this condition may cause that it will not enter RESET operation*/
    {
        MT_INFO_PVR("to reset buffer and player.\n");
        pChnAttr->stTplayCtlInfo.u32RefFrmPtsMs = PVR_INDEX_INVALID_PTSMS;

        ret = PVRPlayResetToCurFrame(pChnAttr->IndexHandle, pChnAttr, pChnAttr->enState);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("reset to current frame failed!\n");
        }
        MT_INFO_PVR("to reset buffer and player. over\n");
    }
    PVR_ALAWYS_PRINT("\r\n pPosition->enPositionType :%d", pPosition->enPositionType );
    switch ( pPosition->enPositionType )
    {
        case MT_UNF_PVR_PLAY_POS_TYPE_SIZE:
            /*
            ret = PVR_Index_SeekByByte(pChnAttr->IndexHandle,
                        pPosition->s64Offset, pPosition->s32Whence);
            */
            ret = MT_ERR_PVR_NOT_SUPPORT;
            break;
        case MT_UNF_PVR_PLAY_POS_TYPE_FRAME:
            /*
            if (pPosition->s64Offset > 0x7fffffff)
            {
                ret = MT_ERR_PVR_INVALID_PARA;
            }
            else
            {
                ret = PVR_Index_SeekByFrame2I(pChnAttr->IndexHandle,
                        (MT_S32)pPosition->s64Offset, pPosition->s32Whence);
            }
            */
            ret = MT_ERR_PVR_NOT_SUPPORT;
            break;
        case MT_UNF_PVR_PLAY_POS_TYPE_TIME:
            pChnAttr->bEofSeek2Start = MT_FALSE;    
            if (MT_TRUE == pChnAttr->bTimeShiftStartFlg)
            {
//                MT_ERR_PVR("\r\n +++seek by timeshift \n" );
                ret = PVR_Index_SeekByTime(pChnAttr->IndexHandle, pPosition->s64Offset, pPosition->s32Whence, pChnAttr->u32CurPlayTimeMs);
            }
            else
            {
//                MT_ERR_PVR("\r\n +++seek by pvr \n" );
                ret = PVR_Index_SeekByTime(pChnAttr->IndexHandle, pPosition->s64Offset, pPosition->s32Whence, u32CurPlayTime);
            }
//            PVR_ALAWYS_PRINT("%s %d ret = %x\n",__FUNCTION__,__LINE__,ret);
            PVR_PLAY_D("after seek by time=%d u32ReadFrame=%d ret=0x%x\n",u32CurPlayTime,pChnAttr->IndexHandle->u32ReadFrame,ret);
            if(ret == MT_SUCCESS)
            {
                PVR_INDEX_ENTRY_S stReadFrame;

                memset(&stReadFrame, 0 ,sizeof(PVR_INDEX_ENTRY_S));

                ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stReadFrame, pChnAttr->IndexHandle->u32ReadFrame);
//                PVR_ALAWYS_PRINT("%s %d pChnAttr->IndexHandle->u32ReadFrame = %d stReadFrame.u32DisplayTimeMs = %d, pChnAttr->u32CurPlayTimeMs =%d\n",__FUNCTION__,__LINE__,pChnAttr->IndexHandle->u32ReadFrame,stReadFrame.u32DisplayTimeMs,pChnAttr->u32CurPlayTimeMs);
                
                if (MT_SUCCESS != ret)
                {
                    if (MT_ERR_PVR_FILE_TILL_END == ret)
                    {
                        ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stReadFrame, pChnAttr->IndexHandle->u32ReadFrame-1);
                        if (MT_SUCCESS != ret)
                        {
                            PVR_UNLOCK_PLAY(pChnAttr);
                            MT_ERR_PVR("Can't get EndFrame:%d\n", pChnAttr->IndexHandle->u32ReadFrame);
                            return ret;
                        }
                    }
                    else
                    {
                        PVR_UNLOCK_PLAY(pChnAttr);
                        MT_ERR_PVR("Can't get EndFrame:%d\n", pChnAttr->IndexHandle->u32ReadFrame);
                        return ret;
                    }
                }
                pChnAttr->u32CurPlayTimeMs = stReadFrame.u32DisplayTimeMs;
                pChnAttr->current_playing_frame = pChnAttr->IndexHandle->u32ReadFrame;
                pChnAttr->u64CurPlayPosition = stReadFrame.u64GlobalOffset;
                
                PVR_ALAWYS_PRINT("++set playing time2=%d u32ReadFrame=%d type=%ld bIsRec=%d cause_resume=%d\n",
                  pChnAttr->u32CurPlayTimeMs,pChnAttr->IndexHandle->u32ReadFrame,PVR_INDEX_get_frameType(&stReadFrame),
                  pChnAttr->IndexHandle->bIsRec,pChnAttr->IndexHandle->cause_resume);

                //fixed #30802
                if(pChnAttr->IndexHandle->bIsRec == MT_FALSE && pChnAttr->IndexHandle->cause_resume ==3)
                {
                    if(pPosition->s32Whence == SEEK_SET && pPosition->s64Offset == 0)
                    {
                        pChnAttr->bEofSeek2Start = MT_TRUE;//the flag for playback over seek to start use
                        PVR_PLAY_P("bEofSeek2Start set to TRUE!\n");
                    }
                    
                }
                
            }
            break;
        default:
            ret = MT_ERR_PVR_INVALID_PARA;
    }

    pChnAttr->bQuickUpdateStatus = MT_TRUE;
    pChnAttr->set_eof = MT_FALSE;
//    PVR_ALAWYS_PRINT("%s %d\n",__FUNCTION__,__LINE__);
    if(1){//reset to clear av buffer!
        /*
        it's must be called before MT_PVR_VIDEO_Reset, if not, audio es data
        will be updated to adec during the brief period of time between them,
        that not allowed.
        */
        ret = MT_UNF_AVPLAY_TrickSeekIn(pChnAttr->hAvplay);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("start trick seek error:%d\n", ret);
            return ret;
        }

        pvrplay_clearvframe(pChnAttr);
        //ret = MT_PVR_VIDEO_Reset(pChnAttr->hAvplay, NULL);
        ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, NULL); //fixed #32744
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("AVPLAY reset failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }

        ret = MT_UNF_AVPLAY_TrickSeekOut(pChnAttr->hAvplay);

        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("end trick seek error:%d\n", ret);
            return ret;
        }
    }
    PVR_ALAWYS_PRINT("SEEK OK!\n");
    PVR_UNLOCK_PLAY(pChnAttr);

    return ret;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayStep
 Description     : play by step frame
 Input           : u32Chn        **channel number
                   direction  ** direction:forward or backward. presently, just only support backward.
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayStep(MT_U32 u32Chn, MT_S32 direction)
{
    MT_S32 ret = MT_SUCCESS;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_HANDLE hWindow;
    MT_CODEC_VIDEO_CMD_S  stVdecCmdPara = {0};
    MT_UNF_AVPLAY_TPLAY_OPT_S stTPlayOpt;
    MT_BOOL bIsEmpty= MT_FALSE;

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    PVR_ALAWYS_PRINT("%s in %x\n",__FUNCTION__,direction);
    memset(&stTPlayOpt, 0, sizeof(MT_UNF_AVPLAY_TPLAY_OPT_S));

    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);
    /* audio type index file, which not support step forward play */
    if (PVR_INDEX_IS_TYPE_AUDIO(pChnAttr->IndexHandle))
    {
        MT_ERR_PVR("audio stream NOT support step play!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    MT_INFO_PVR("PVR step once, channel=%d!\n", u32Chn);

    if (direction < 0)
    {
        MT_ERR_PVR("PVR Play: NOT support step back!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_ERR_PVR_NOT_SUPPORT;
    }

    if (direction == 0)
    {
        MT_WARN_PVR("PVR Play: step no direction!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }

    if ((pChnAttr->bEndOfFile)
        && (PVR_IS_PLAY_FORWARD(pChnAttr->enState, pChnAttr->enLastState))
        && (direction > 0))
    {
        MT_INFO_PVR("till end, need not start main rout, state=%d, laststate=%d!\n",  pChnAttr->enState, pChnAttr->enLastState);
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_SUCCESS;
    }

    /* record rewrite the file, so reset it to the start */
    if (PVR_Index_QureyClearRecReachPlay(pChnAttr->IndexHandle))
    {
        PVR_Index_SeekToStart(pChnAttr->IndexHandle);
        pChnAttr->bTsBufReset = MT_TRUE;
        ret = MT_UNF_DMX_ResetTSBuffer(pChnAttr->hTsBuffer);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("ts buffer reset failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
        pvrplay_clearvframe(pChnAttr);
        ret = MT_UNF_AVPLAY_Reset(pChnAttr->hAvplay, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("AVPLAY reset failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
    }
    else if (((MT_UNF_PVR_PLAY_STATE_PLAY != pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_PAUSE != pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_STEPF != pChnAttr->enState))
     || ((MT_UNF_PVR_PLAY_STATE_PAUSE == pChnAttr->enState)
        && (MT_UNF_PVR_PLAY_STATE_PLAY != pChnAttr->enLastState)
        && (MT_UNF_PVR_PLAY_STATE_STEPF != pChnAttr->enLastState)))
    {
        MT_INFO_PVR("to reset buffer and player.\n");
        ret = PVRPlayResetToCurFrame(pChnAttr->IndexHandle, pChnAttr, MT_UNF_PVR_PLAY_STATE_STEPF);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("reset to current frame failed!\n");
        }
    }

    ret = MT_MPI_AVPLAY_GetWindowHandle(pChnAttr->hAvplay, &hWindow);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("AVPLAY get window handle failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    /* the first incoming step play mode, set the player */
    if (MT_UNF_PVR_PLAY_STATE_STEPF != pChnAttr->enState)
    {
        ret = MT_UNF_AVPLAY_SetDecodeMode(pChnAttr->hAvplay, MT_UNF_VCODEC_MODE_NORMAL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("set vdec normal mode error!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }

        /*video trick mode, stop the audio */
        ret = MT_UNF_AVPLAY_Stop(pChnAttr->hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("AVPLAY stop audio failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
#if 0
        /* set window to step mode */
        ret = MT_MPI_VO_SetWindowStepMode(hWindow, MT_TRUE);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_PVR("Set window step mode failed!\n");
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }
#endif
        /* on step mode, resume the player to normal mode */
        ret = MT_UNF_AVPLAY_Resume(pChnAttr->hAvplay, NULL);
        if (ret != MT_SUCCESS)
        {
            MT_ERR_PVR("AVPLAY_Resume failed:%#x\n", ret);
            PVR_UNLOCK_PLAY(pChnAttr);
            return MT_FAILURE;
        }

        pChnAttr->bQuickUpdateStatus = MT_TRUE;
        pChnAttr->enLastState = pChnAttr->enState;
        pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_STEPF;

        //set play speed to normal
        stTPlayOpt.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
        stTPlayOpt.u32SpeedInteger = MT_UNF_PVR_PLAY_SPEED_NORMAL;
        stTPlayOpt.u32SpeedDecimal = 0;

        if ( pChnAttr->bRecordedVideoExist == MT_TRUE)
        {
            stVdecCmdPara.u32CmdID = MT_UNF_AVPLAY_SET_TPLAY_PARA_CMD;
            stVdecCmdPara.pPara = &stTPlayOpt;
            ret = MT_UNF_AVPLAY_Invoke(pChnAttr->hAvplay, MT_UNF_AVPLAY_INVOKE_VCODEC, (void *)&stVdecCmdPara);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_PVR("AVPLAY Tplay failed!\n");
                PVR_UNLOCK_PLAY(pChnAttr);
                return MT_FAILURE;
            }
        }
    }

    /* on step mode forward one frame */
    /*MT_MPI_VO_SetWindowStepPlay(hWindow);  */
    ret = MT_UNF_AVPLAY_Step(pChnAttr->hAvplay, NULL);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_PVR("Step window failed!\n");
        PVR_UNLOCK_PLAY(pChnAttr);
        return MT_FAILURE;
    }

    //if (PVRPlayIsEOS(pChnAttr))
    (MT_VOID)MT_MPI_AVPLAY_IsBuffEmpty(pChnAttr->hAvplay, &bIsEmpty);
    if(bIsEmpty)
    {
        if (pChnAttr->IndexHandle->stCycMgr.u32StartFrame >= pChnAttr->IndexHandle->stCycMgr.u32EndFrame)
        {
            if ((pChnAttr->IndexHandle->u32ReadFrame >= pChnAttr->IndexHandle->stCycMgr.u32EndFrame) &&
                (pChnAttr->IndexHandle->u32ReadFrame < pChnAttr->IndexHandle->stCycMgr.u32StartFrame))
            {
                MT_INFO_PVR("till end, need not start main rout, state=%d, laststate=%d!\n",  pChnAttr->enState, pChnAttr->enLastState);
                PVRPlayPostEvent(pChnAttr->u32chnID, MT_UNF_PVR_EVENT_PLAY_EOF, 0);
            }
        }
        else
        {
            if (pChnAttr->IndexHandle->u32ReadFrame >= pChnAttr->IndexHandle->stCycMgr.u32EndFrame)
            {
                MT_INFO_PVR("till end, need not start main rout, state=%d, laststate=%d!\n",  pChnAttr->enState, pChnAttr->enLastState);
                PVRPlayPostEvent(pChnAttr->u32chnID, MT_UNF_PVR_EVENT_PLAY_EOF, 0);
            }
        }
    }
    pChnAttr->enState = MT_UNF_PVR_PLAY_STATE_STEPF;
    PVR_UNLOCK_PLAY(pChnAttr);

    return MT_SUCCESS;
}

MT_S32 MT_PVR_PlayRegisterReadCallBack(MT_U32 u32Chn, ExtraCallBack readCallBack)
{
    PVR_PLAY_CHN_S              *pChnAttr;
    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    pChnAttr->readCallBack = readCallBack;
    PVR_UNLOCK_PLAY(pChnAttr);
    return MT_SUCCESS;
}

MT_S32 MT_PVR_PlayUnRegisterReadCallBack(MT_U32 u32Chn)
{
    PVR_PLAY_CHN_S              *pChnAttr;

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    PVR_LOCK_PLAY(pChnAttr);
    PVR_PLAY_CHECK_CHN_INIT_UNLOCK(pChnAttr);

    pChnAttr->readCallBack = NULL;

    PVR_UNLOCK_PLAY(pChnAttr);
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype       : MT_PVR_PlayGetStatus
 Description     : get the status of play channel
 Input           : u32Chn      **channel number
 Output          : pStatus  **the status of channel
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/22
    Author       : quyaxin 46153
    Modification : Created function

*****************************************************************************/
MT_S32 MT_PVR_PlayGetStatus(MT_U32 u32Chn, MT_UNF_PVR_PLAY_STATUS_S *pStatus)
{
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_S32 ret = 0;
    MT_UNF_PVR_PLAY_STATE_E enCurState;
    //PVR_INDEX_ENTRY_S    stCurPlayFrame;   /* the current displaying frame info  */
    MT_U32 u32CurFrm;
    MT_UNF_PVR_FILE_ATTR_S FileStatus = { 0 };
    MT_U32 cur_time=0;

    MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S stAvplayPrivInfo;

    PVR_PLAY_CHECK_CHN(u32Chn);

    memset(&stAvplayPrivInfo, 0, sizeof(MT_UNF_AVPLAY_PRIVATE_STATUS_INFO_S));
    pChnAttr = &g_stPvrPlayChns[u32Chn];

    //MT_ERR_PVR("pChnAttr->enState = %d speed = %d\n",pChnAttr->enState,pChnAttr->enSpeed);

    PVR_LOCK_PLAY_VALID(pChnAttr);
    if (MT_UNF_PVR_PLAY_STATE_INVALID ==  pChnAttr->enState ){
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        return MT_ERR_PVR_CHN_NOT_INIT;
    }

    enCurState = pChnAttr->enState;
    if ((enCurState < MT_UNF_PVR_PLAY_STATE_PLAY)
        || (enCurState > MT_UNF_PVR_PLAY_STATE_STEPB))
    {
        pStatus->enState = pChnAttr->enState;
        pStatus->enSpeed = pChnAttr->enSpeed;
        pStatus->u32CurPlayTimeInMs = 0;
        pStatus->u64CurPlayPos=pChnAttr->u64CurPlayPosition;
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        return MT_SUCCESS;
    }
    #if 0 /* fixed get current play time is not right when ff to end of file */
    if (pChnAttr->bEndOfFile) /*reach to the start or end of the file, return the previous status */
    {
        memcpy(pStatus, &pChnAttr->stLastStatus, sizeof(MT_UNF_PVR_PLAY_STATUS_S));
        pStatus->enState = pChnAttr->enState;
        pStatus->enSpeed = pChnAttr->enSpeed;
        pStatus->u32CurPlayTimeInMs = pChnAttr->u32CurPlayTimeMs;
        pStatus->u64CurPlayPos=pChnAttr->u64CurPlayPosition;
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        return MT_SUCCESS;
    }
    #endif
    pStatus->enState = pChnAttr->enState;
    pStatus->enSpeed = pChnAttr->enSpeed;

    if(NULL==pChnAttr->IndexHandle){
        //printf("+++err1\n");
//        MT_ERR_PVR("%s======%d\n",__FUNCTION__,__LINE__);
        pStatus->u32CurPlayTimeInMs=0;
        pStatus->u64CurPlayPos=0;
        pStatus->u32CurPlayFrame=0;
        memcpy(&pChnAttr->stLastStatus, pStatus, sizeof(MT_UNF_PVR_PLAY_STATUS_S));
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        return MT_SUCCESS;
    }
    { //get now avpts!
        if(MT_UNF_PVR_REC_INDEX_TYPE_AUDIO == pChnAttr->IndexHandle->enIndexType){
            MT_UNF_AVPLAY_STATUS_INFO_S playinfo;
            if(MT_SUCCESS==MT_UNF_AVPLAY_GetAudioStatusInfo(pChnAttr->hAvplay, &playinfo)){
                if(MT_SUCCESS==frame_gettimebysize(pChnAttr,playinfo.stBufStatus[MT_UNF_AVPLAY_BUF_ID_ES_AUD].u32UsedSize,&u32CurFrm,&pStatus->u32CurPlayTimeInMs,1,NULL)){
                    ;
                }else{
                    u32CurFrm=0;
                    pStatus->u32CurPlayTimeInMs=0;
                    pStatus->u64CurPlayPos=0;
                    pStatus->u32CurPlayFrame=0;
                }
            }else{
                u32CurFrm=0;
                pStatus->u32CurPlayTimeInMs=0;
                pStatus->u64CurPlayPos=0;
                pStatus->u32CurPlayFrame=0;
            }
        }else{
            if(MT_SUCCESS !=Frame_GetDispTimeByCache(pChnAttr,&u32CurFrm,&pStatus->u32CurPlayTimeInMs,NULL,&pStatus->u64CurPlayPos)){
                u32CurFrm=0;
                //if(pStatus->enSpeed != MT_UNF_PVR_PLAY_SPEED_NORMAL) //fixed Bug #26980
                {
                    /*after pause or trick play, some time get stSyncStatus.u64LastVidPts may failed*/
                    PVRPLAY_LOG_DEBUG_FRAME("enSpeed=%d play get current play time failed,return pre time=%d\n",pStatus->enSpeed,pChnAttr->u32CurPlayTimeMs);
                    pStatus->u32CurPlayTimeInMs = pChnAttr->u32CurPlayTimeMs;
                }
                #if 0
                else{
                    pStatus->u32CurPlayTimeInMs=0;
                    PVR_PLAY_D("enSpeed=%d play get current play time failed,set to 0\n",pStatus->enSpeed);
                }
                #endif
                pStatus->u64CurPlayPos=0;
                pStatus->u32CurPlayFrame=0;
            }
        }
    }
    //MT_ERR_PVR("video dpts =%d index: last %d pChnAttr->u32CurPlayTimeMs=%d pChnAttr->play_status_disptime =%d\n",
    //    pStatus->u32CurPlayTimeInMs,pChnAttr->IndexHandle->Latest_DisplayTimeMs,pChnAttr->u32CurPlayTimeMs,pChnAttr->play_status_disptime);

    //MT_ERR_PVR("u32CurFrm %d stCycMgr start =%d end = %d current_playing_frame = %d\n", u32CurFrm,pChnAttr->IndexHandle->stCycMgr.u32StartFrame,pChnAttr->IndexHandle->stCycMgr.u32EndFrame,pChnAttr->current_playing_frame);
    if(pStatus->u32CurPlayTimeInMs>pChnAttr->IndexHandle->Latest_DisplayTimeMs){    //check time in range
        //MT_ERR_PVR("+++out of time.range %x,%x\n",pStatus->u32CurPlayTimeInMs,pChnAttr->IndexHandle->Latest_DisplayTimeMs);
        pStatus->u32CurPlayTimeInMs=0;
    }
    if(!Pvr_Check_ReadInRange(pChnAttr->IndexHandle->stCycMgr.u32StartFrame, pChnAttr->IndexHandle->stCycMgr.u32EndFrame, u32CurFrm))
    {    //check frame in range
        #if 1
        MT_ERR_PVR("+++out of frame.range %d,%d,%d,%d,%d,%d\n",pChnAttr->IndexHandle->stCycMgr.u32StartFrame,
                                                      pChnAttr->IndexHandle->stCycMgr.u32EndFrame,
                                                      pChnAttr->IndexHandle->stCycMgr.u32LastFrame,u32CurFrm,
                                                      pStatus->u32CurPlayTimeInMs,pChnAttr->u32CurPlayTimeMs);
        #endif
        if ((pStatus->u32CurPlayTimeInMs > pChnAttr->u32CurPlayTimeMs) && ((pStatus->u32CurPlayTimeInMs - pChnAttr->u32CurPlayTimeMs) < 1000)){

        }
        else{
          pStatus->u32CurPlayTimeInMs=0;
          MT_ERR_PVR("%s======%d error\n",__FUNCTION__,__LINE__);
        }
    }

    /* 
        fixed Bug #25586,after rewind rec, the start time will update all the time,and the start time may large than curren play time,
        if the app use curren play time - start time will < 0, display error(ply=4294966)
    */
    if(pChnAttr->IndexHandle->bIsRec && pChnAttr->IndexHandle->stCycMgr.s32CycTimes){
        ret = PVR_Index_PlayGetFileAttrByFileName(pChnAttr->stUserCfg.szFileName, pChnAttr->IndexHandle, &FileStatus); 
        if(MT_SUCCESS == ret){
            if(pStatus->u32CurPlayTimeInMs < FileStatus.u32StartTimeInMs){
                PVR_PLAY_D("--- u32CurPlayTimeInMs=%d FileStatus.u32StartTimeInMs=%d\n",pStatus->u32CurPlayTimeInMs,FileStatus.u32StartTimeInMs);
                if(pStatus->u32CurPlayTimeInMs !=0)
                {
                    cur_time = FileStatus.u32StartTimeInMs+ 100; //adjudt to star time more litte
                    PVR_PLAY_D("pvr current play time < start time,adjust from %d to %d\n",pStatus->u32CurPlayTimeInMs,cur_time);
                    pStatus->u32CurPlayTimeInMs = cur_time;
                }
                
            }
        }
    }
   
    {   //check direction of time
        if(0==pChnAttr->play_status_disptime){
            pChnAttr->play_status_disptime=pStatus->u32CurPlayTimeInMs;
        }else{
            if(pChnAttr->enSpeed>0){  //forward
                if(pStatus->u32CurPlayTimeInMs<pChnAttr->play_status_disptime){
//                    MT_ERR_PVR("+++use last display time0=%x,%x,%x\n",pChnAttr->enSpeed,pStatus->u32CurPlayTimeInMs,pChnAttr->play_status_disptime);
                    pStatus->u32CurPlayTimeInMs=pChnAttr->play_status_disptime;
                }else{
                    pChnAttr->play_status_disptime=pStatus->u32CurPlayTimeInMs;
                }
            }else{                    //backward
                if(pStatus->u32CurPlayTimeInMs>pChnAttr->play_status_disptime){
//                    MT_ERR_PVR("+++use last display time1=%x,%x,%x\n",pChnAttr->enSpeed,pStatus->u32CurPlayTimeInMs,pChnAttr->play_status_disptime);
                    pStatus->u32CurPlayTimeInMs=pChnAttr->play_status_disptime;
                }else{
                    pChnAttr->play_status_disptime=pStatus->u32CurPlayTimeInMs;
                }
            }
        }
    }
    if(0!=pStatus->u32CurPlayTimeInMs){ //record now playing frame
        pChnAttr->current_playing_frame=u32CurFrm;
        pChnAttr->u32CurPlayTimeMs=pStatus->u32CurPlayTimeInMs;
        pChnAttr->u64CurPlayPosition=pStatus->u64CurPlayPos;
        //printf("+++set time x=%d\n",pChnAttr->u32CurPlayTimeMs);
    }else{
        pStatus->u32CurPlayTimeInMs=pChnAttr->u32CurPlayTimeMs;   //return last valid time
        pStatus->u64CurPlayPos=pChnAttr->u64CurPlayPosition;
        //printf("+++get 0 time=%d\n",pChnAttr->u32CurPlayTimeMs);
    }
    pStatus->u32CurPlayFrame=pChnAttr->current_playing_frame;     //return last valid frame
    //if(MT_UNF_PVR_PLAY_SPEED_NORMAL != pChnAttr->enSpeed)
    //    MT_ERR_PVR("+++ps:%d,%d\n",pChnAttr->u32CurPlayTimeMs,pStatus->u32CurPlayFrame);

    /*
    ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &stCurPlayFrame, u32CurFrm);
    if (MT_SUCCESS == ret)
    {
        pChnAttr->u32CurPlayTimeMs=stCurPlayFrame.u32DisplayTimeMs;
        pStatus->u32CurPlayFrame = u32CurFrm;
        pStatus->u64CurPlayPos =  stCurPlayFrame.u64Offset;
    }*/

    memcpy(&pChnAttr->stLastStatus, pStatus, sizeof(MT_UNF_PVR_PLAY_STATUS_S));

    PVR_UNLOCK_PLAY_VALID(pChnAttr);

    return MT_SUCCESS;
}

MT_S32 MT_PVR_PlayGetFileAttr(MT_U32 u32Chn, MT_UNF_PVR_FILE_ATTR_S *pAttr)
{
    MT_S32 ret;
    PVR_PLAY_CHN_S  *pChnAttr;

    PVR_CHECK_POINTER(pAttr);
    PVR_PLAY_CHECK_INIT(&g_stPlayInit);

    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
/*
    if(PVR_Rec_IsFileSaving(pChnAttr->stUserCfg.szFileName))
    {
        if(pChnAttr->IndexHandle != NULL)
        {
            PVR_Index_FlushIdxWriteCache(pChnAttr->IndexHandle);
        }
        else
        {
            MT_ERR_PVR("Index Handle is NULL.\n");
            return MT_FAILURE;
        }
    }
*/
    PVR_LOCK_PLAY_VALID(pChnAttr);
    if (MT_UNF_PVR_PLAY_STATE_INVALID ==  pChnAttr->enState){
        PVR_UNLOCK_PLAY_VALID(pChnAttr);
        return MT_ERR_PVR_CHN_NOT_INIT;
    }

    ret = PVR_Index_PlayGetFileAttrByFileName(pChnAttr->stUserCfg.szFileName, pChnAttr->IndexHandle, pAttr);
    PVR_UNLOCK_PLAY_VALID(pChnAttr);

    return ret;
}

MT_S32 MT_PVR_PlayCheckInfo(MT_U32 u32Chn)
{
    MT_S32 ret=MT_SUCCESS;
    PVR_PLAY_CHN_S  *pChnAttr;
    MT_U32 u32StartFrm;
    MT_U32 u32EndFrm;
    MT_U32 u32LastFrm;
    MT_U32 u32TotalFrm;
    PVR_PLAY_CHECK_CHN(u32Chn);
    pChnAttr = &g_stPvrPlayChns[u32Chn];
    u32StartFrm = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
    u32EndFrm = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
    u32LastFrm = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;

    if(u32StartFrm > u32EndFrm)
    {
        u32TotalFrm = u32EndFrm + u32LastFrm - u32StartFrm;
    }
    else
    {
        u32TotalFrm = u32LastFrm - u32StartFrm;
    }


    MT_INFO_PVR("\r\n total s:%d,e:%d,l:%d,total:%d", u32StartFrm,u32EndFrm,u32LastFrm,u32TotalFrm);
#if 0
    for(num = 0; num < u32TotalFrm; num++)
    {
        ret = PVR_Index_GetFrameByNum(pChnAttr->IndexHandle, &entry, num);
        printf("\r\n u64Offset:0x%08x, u16IndexType:%d, u64GlobalOffset:0x%08x, u32PtsMs:0x%08x, u32FrameSize:0x%08x, \
        s32CycTimes:%d, u161stFrameOfTT:%d, u16FrameTypeAndGop:%d, u16UpFlowFlag:%d, u32DisplayTimeMs:%d",
        entry.u64Offset, entry.u16IndexType, entry.u64GlobalOffset, entry.u32PtsMs, entry.u32FrameSize,
        entry.s32CycTimes, entry.u161stFrameOfTT, entry.u16FrameTypeAndGop, entry.u16UpFlowFlag, entry.u32DisplayTimeMs);
    }
#endif

    //MT_Index_GetTotalCntIP(pChnAttr->IndexHandle, &u32IPCnt);
    //MT_Index_GetTotalCntI(pChnAttr->IndexHandle, &u32ICnt);

    //ret = PVR_Index_GetFrameRate(pChnAttr->IndexHandle, &framerate);
    //printf("\r\n MT_PVR_PlayCheckInfo framerate:%d, IPCnt:%d, ICnt:%d", pChnAttr->u32FrameRate, u32IPCnt, u32ICnt);
    return ret;
}
#if 1
//get policy of trick play!
MT_S32 MT_PVR_PlayGetPolicy(PVR_PLAY_CHN_S *pChnAttr, MT_UNF_PVR_PLAY_SPEED_E speed_x1024, PVR_PLAY_POLICY_S *pPvrPlayOpt)
{
    MT_U32 framerate;
    //MT_U32 u32StartFrm =0;
    //MT_U32 u32EndFrm = 0;
    //MT_U32 u32LastFrm = 0;
    MT_U32 u32TotalFrm = 0;
    MT_U32 u32IPCnt = 0;
    MT_U32 u32ICnt = 0;
    MT_U32 u32MaxCnt=0;
    MT_U32 width=0;
    MT_U32 height=0;
    MT_U32 skipICnt = 0;
    MT_U32 temp_framerate=0;
    MT_U32 totaltime=0;
    MT_U32 u32IPBCnt=0;
    float float_framerate=0.0;
    MT_U32 configRate = 5;
    MT_BOOL bFiledEncode = 0;
    
    if((MT_UNF_PVR_PLAY_SPEED_NORMAL == speed_x1024)||
    (MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD == speed_x1024)){          //normal play.reset to normal play
        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
        pPvrPlayOpt->stDispFrc.u32fpsInteger = 0;       //zero
        pPvrPlayOpt->stDispFrc.u32fpsDecimal = 10;      //non-zero
        return MT_SUCCESS;
    }

    pPvrPlayOpt->enTplayDirect = (speed_x1024 >= 0) ? MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD : MT_UNF_AVPLAY_TPLAY_DIRECT_BACKWARD;
    pPvrPlayOpt->stDispFrc.u32fpsDecimal = 0;
    //u32StartFrm = pChnAttr->IndexHandle->stCycMgr.u32StartFrame;
    //u32EndFrm = pChnAttr->IndexHandle->stCycMgr.u32EndFrame;
    //u32LastFrm = pChnAttr->IndexHandle->stCycMgr.u32LastFrame;

    //MT_Index_GetTotalCntIP(pChnAttr->IndexHandle, &pChnAttr->u32IPCnt);
    //MT_Index_GetTotalCntI(pChnAttr->IndexHandle, &pChnAttr->u32ICnt);
    MT_Index_GetTotalCntI_IP(pChnAttr->IndexHandle, &pChnAttr->u32ICnt,&pChnAttr->u32IPCnt,&u32IPBCnt,50000);
    PVR_Index_GetFrameRate(pChnAttr->IndexHandle, &pChnAttr->u32FrameRate);
    if(0==pChnAttr->u32ICnt){ //abnormal
        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
        pPvrPlayOpt->stDispFrc.u32fpsInteger = 0;       //zero
        pPvrPlayOpt->stDispFrc.u32fpsDecimal = 10;      //non-zero
        return MT_SUCCESS;
    }
    u32IPCnt = pChnAttr->u32IPCnt;
    u32ICnt = pChnAttr->u32ICnt;
    framerate = pChnAttr->u32FrameRate;

     u32TotalFrm=u32IPBCnt;
    /*if(u32StartFrm > u32EndFrm)
    {
        u32TotalFrm = u32EndFrm + u32LastFrm - u32StartFrm;
    }
    else
    {
        u32TotalFrm = u32LastFrm - u32StartFrm;
    }*/

    if((0xffffffff!=pChnAttr->u32FrameRate_fromav)&&
        (pChnAttr->u32FrameRate > ((pChnAttr->u32FrameRate_fromav*3)>>1))){
        bFiledEncode = MT_TRUE;
    }

    PVRPLAY_LOG_TRICK_POLICY("----------------------MT_PVR_PlayGetPolicy----------------------\n");
    PVRPLAY_LOG_TRICK_POLICY("+++ploicy:%d.%d bFiledEncode=%d\n",pChnAttr->u32FrameRate_fromav,pChnAttr->u32FrameRate,bFiledEncode);
    temp_framerate=((0xffffffff!=pChnAttr->u32FrameRate_fromav)?pChnAttr->u32FrameRate_fromav:pChnAttr->u32FrameRate);

    if((MT_UNF_PVR_PLAY_SPEED_NORMAL > speed_x1024)&&
       (0 < speed_x1024))
    {   //fast slowly
        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_ALL;
        pPvrPlayOpt->stDispFrc.u32fpsInteger = temp_framerate * (mt_u32)speed_x1024 /MT_UNF_PVR_PLAY_SPEED_NORMAL;
        pPvrPlayOpt->stDispFrc.u32fpsDecimal = 0;
        pPvrPlayOpt->u32SkipPCnt_EveryI = 0;
        pPvrPlayOpt->u32SkipPMore_EveryxI = 0;
        PVRPLAY_LOG_TRICK_POLICY("+++sf=%d,speed=%d\n",pPvrPlayOpt->stDispFrc.u32fpsInteger,(mt_u32)speed_x1024/MT_UNF_PVR_PLAY_SPEED_NORMAL);
    }
    else
    {
        totaltime=u32TotalFrm/pChnAttr->u32FrameRate;
        PVRPLAY_LOG_TRICK_POLICY("+++totaltime=%d\n",totaltime);
        totaltime=(totaltime*((mt_u32)MT_UNF_PVR_PLAY_SPEED_NORMAL)) / (mt_u32)abs(speed_x1024);
        PVRPLAY_LOG_TRICK_POLICY("+++tptime=%d\n",totaltime);
        if(0==totaltime){
            totaltime=1;
        }
        PVRPLAY_LOG_TRICK_POLICY("frame:total=%d,u32IPCnt=%d,u32ICnt=%d\n",u32TotalFrm,u32IPCnt,u32ICnt);
        if(pPvrPlayOpt->enTplayDirect == MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD)
        {
            if((u32IPCnt != u32ICnt) && (u32IPCnt > 25*totaltime) && (u32ICnt < 25*totaltime)){ //skip ip to 25
                float_framerate=(float)(25*totaltime)/(float)u32ICnt;
                pPvrPlayOpt->stDispFrc.u32fpsInteger=25;
                pPvrPlayOpt->stDispFrc.u32fpsDecimal=0;
                pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                            //every I(need x IP frame)
                pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);            //every 100 I ,need more x pframe
                pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_IP;
                frame_empty(pChnAttr);
                PVRPLAY_LOG_TRICK_POLICY("+++++sf.skip ip to %d.%d,%d,%d,%d,%d\n",25,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,u32IPCnt,totaltime);
            }else{
                pPvrPlayOpt->stDispFrc.u32fpsInteger=25;
                pPvrPlayOpt->stDispFrc.u32fpsDecimal=0;
                if(u32ICnt < 25*totaltime){
                    if(u32IPCnt == u32ICnt){  //265
                        if(u32ICnt < totaltime){    //add to 1
                            float_framerate=(float)(totaltime-u32ICnt)/(float)u32ICnt;
                            pPvrPlayOpt->stDispFrc.u32fpsInteger = 1;
                            pPvrPlayOpt->stDispFrc.u32fpsDecimal = 0;
                            pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_I;
                            pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                      //every I, repeat x times
                            pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);      //every 100 I ,repeat more x pframe
                            PVRPLAY_LOG_TRICK_POLICY("+++++sf.add0 to %d.%d,%d,%d,%d\n",1,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                        }else{                      //skip to x=(icnt/time)
                            MT_U32 tmpfr=u32ICnt/totaltime;
                            float_framerate=(float)u32ICnt/((float)tmpfr*totaltime);
                            pPvrPlayOpt->stDispFrc.u32fpsInteger=tmpfr;
                            pPvrPlayOpt->stDispFrc.u32fpsDecimal=0;
                            pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                      //every x I,push one
                            pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);      //every 100 I ,discard more x pframe
                            pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_SKIP_I;
                            PVRPLAY_LOG_TRICK_POLICY("+++++sf.skip0 to %d.%d,%d,%d,%d\n",pPvrPlayOpt->stDispFrc.u32fpsInteger,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                        }
                    }else{                          //add to 25
                        float_framerate=(float)(25*totaltime-u32ICnt)/(float)u32ICnt;
                        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_I;
                        pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                          //every I, repeat x times
                        pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);          //every 100 I ,repeat more x pframe
                        PVRPLAY_LOG_TRICK_POLICY("+++++sf.add1 to %d.%d,%d,%d,%d\n",25,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                    }
                }else{                              //skip to 25
                    float_framerate=(float)(u32ICnt)/(float)(configRate*totaltime);
                    pPvrPlayOpt->stDispFrc.u32fpsInteger=configRate;
                    pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_SKIP_I;
                    pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                              //every x I,push one
                    pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);              //every 100 I ,discard more x pframe
                    PVRPLAY_LOG_TRICK_POLICY("+++++sf.skip1 to %d,%d,%d,%d,%d\n",configRate,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                }
                pChnAttr->fieldtype=FRAME_BOT_FIELD;
            }
        }
        else // backward.
        {
            pPvrPlayOpt->stDispFrc.u32fpsInteger=25;
            pPvrPlayOpt->stDispFrc.u32fpsDecimal=0;
            if(u32ICnt < 25*totaltime){
                if(u32IPCnt == u32ICnt){  //265
                    if(u32ICnt < totaltime){          //add to 1
                        float_framerate=(float)(totaltime-u32ICnt)/(float)u32ICnt;
                        pPvrPlayOpt->stDispFrc.u32fpsInteger = 1;
                        pPvrPlayOpt->stDispFrc.u32fpsDecimal = 0;
                        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_I;
                        pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                    //every I, repeat x times
                        pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);    //every 100 I ,repeat more x pframe
                        PVRPLAY_LOG_TRICK_POLICY("+++++back.add0 to %d.%d,%d,%d,%d\n",1,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                    }else{                            //skip to x=(icnt/time)
                        MT_U32 tmpfr=u32ICnt/totaltime;
                        float_framerate=(float)u32ICnt/((float)tmpfr*totaltime);
                        pPvrPlayOpt->stDispFrc.u32fpsInteger=(MT_U32)tmpfr;
                        pPvrPlayOpt->stDispFrc.u32fpsDecimal=0;
                        pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                    //every x I,push one
                        pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);    //every 100 I ,discard more x pframe
                        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_SKIP_I;
                        PVRPLAY_LOG_TRICK_POLICY("+++++back.skip0 to %d.%d,%d,%d,%d\n",pPvrPlayOpt->stDispFrc.u32fpsInteger,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                    }
                }else{
                    if(u32ICnt>=totaltime){   //skip frame
                        MT_U32 tmpfr=u32ICnt/totaltime;
                        float_framerate=(float)u32ICnt/((float)tmpfr*totaltime);;
                        pPvrPlayOpt->stDispFrc.u32fpsInteger=(MT_U32)tmpfr;
                        pPvrPlayOpt->stDispFrc.u32fpsDecimal=0;
                        pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                    //every x I,push one
                        pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);    //every 100 I ,discard more x pframe
                        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_SKIP_I;
                        PVRPLAY_LOG_TRICK_POLICY("+++++back.skip1 to %d.%d,%d,%d,%d\n",pPvrPlayOpt->stDispFrc.u32fpsInteger,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                    }else{                    //round to 25. maybe flicker!
                        float_framerate=(float)(25*totaltime-u32ICnt)/(float)u32ICnt;
                        pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_I;
                        pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                     //every I, repeat x times
                        pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);     //every 100 I ,repeat more x pframe
                        PVRPLAY_LOG_TRICK_POLICY("+++++back.add1 to %d.%d,%d,%d,%d\n",25,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
                    }
                }
            }else{                                    //skip to 25
                float_framerate=(float)u32ICnt/(float)(configRate*totaltime);
                pPvrPlayOpt->stDispFrc.u32fpsInteger=configRate;
                pPvrPlayOpt->enSendDataMode = PVR_PLAY_SEND_DATA_SKIP_I;
                pPvrPlayOpt->u32SkipPCnt_EveryI = (MT_U32)float_framerate;                            //every x I,push one
                pPvrPlayOpt->u32SkipPMore_EveryxI = (((MT_U32)(float_framerate*100))%100);            //every 100 I ,discard more x pframe
                PVRPLAY_LOG_TRICK_POLICY("+++++sf.skip1 to %d,%d,%d,%d,%d\n",configRate,pPvrPlayOpt->u32SkipPCnt_EveryI,pPvrPlayOpt->u32SkipPMore_EveryxI,u32ICnt,totaltime);
            }
            pChnAttr->fieldtype=FRAME_BOT_FIELD;
        }
        pChnAttr->u32PushAddPoint=0;
        pChnAttr->u32PushCount_Print=0;
    }

    PVR_ALAWYS_PRINT("\r\n ^^^^MT_PVR_PlayGetPolicy \r\nusrspeed:%d, \r\nsend_data:%d, \r\nfrcint:%d, \r\nskipi:%d, \
    \r\ntotalcnt:%d, \r\nipcnt:%d, \r\nicnt:%d, \r\nframerate:%d, \r\nwidth:%d, \r\nheight:%d, \
    \r\nmaxcnt:%d, \r\nifinterval:%d\n",
    speed_x1024, pPvrPlayOpt->enSendDataMode,
    pPvrPlayOpt->stDispFrc.u32fpsInteger, skipICnt, u32TotalFrm, u32IPCnt,
    u32ICnt,framerate, width, height,  u32MaxCnt,pChnAttr->IndexHandle->iframe_interal_max);

    return MT_SUCCESS;
}


#endif
MT_S32 MT_PVR_PlayDispTPlay(PVR_PLAY_CHN_S *pChnAttr, MT_UNF_PVR_PLAY_SPEED_E speed,const MT_UNF_VCODEC_FRMRATE_S *pstTplayOpt)
{
    MT_S32 ret;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S framerate = {0};
    DISP_TRICK_MODE_E trickmode=TM_NORMAL;
    MT_UNF_DEC_TRICK_PARAM_S stTrickParam;

    if(MT_UNF_PVR_PLAY_SPEED_NORMAL==speed){  //back to default
        framerate.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM;
        framerate.stSetFrmRate.u32fpsInteger = 0;
        framerate.stSetFrmRate.u32fpsDecimal = 0;
    }else{
        framerate.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_USER;
        framerate.stSetFrmRate.u32fpsInteger = pstTplayOpt->u32fpsInteger;
        framerate.stSetFrmRate.u32fpsDecimal = pstTplayOpt->u32fpsDecimal;
    }
    if((0 < speed) && (MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD >= speed)){//Bug 118113
        stTrickParam.is_incomplete_stream = 0;
    }else{
        stTrickParam.is_incomplete_stream = 1;
    }

    if(MT_UNF_PVR_PLAY_SPEED_NORMAL==speed){
        stTrickParam.trick_mode = MT_UNF_DEC_TM_NORMAL;
    }else if(MT_UNF_PVR_PLAY_SPEED_NORMAL < speed){                  //ff > 1
        stTrickParam.trick_mode = MT_UNF_DEC_TM_FFWD;
    }else if(MT_UNF_PVR_PLAY_SPEED_1X_FAST_BACKWARD > speed){       //fb < -1
        stTrickParam.trick_mode = MT_UNF_DEC_TM_FREV;
    }else if(0<speed && MT_UNF_PVR_PLAY_SPEED_NORMAL > speed){      //0< sf < 1
        stTrickParam.trick_mode = MT_UNF_DEC_TM_SFWD;
    }else if(0>speed && MT_UNF_PVR_PLAY_SPEED_NORMAL > abs(speed)){ //0> sb > -1
        stTrickParam.trick_mode = MT_UNF_DEC_TM_SREV;
    }else{
        stTrickParam.trick_mode = MT_UNF_DEC_TM_NORMAL;
    }
    if(stTrickParam.trick_mode != MT_UNF_DEC_TM_SFWD)//not set framrate when sf
    {
        ret = MT_MPI_AVPLAY_SetAttr(pChnAttr->hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &framerate);
        if(MT_SUCCESS==ret){//set tp cfg
            ret = MT_UNF_AVPLAY_SetTrickCfg(pChnAttr->hAvplay, &stTrickParam);
        }
    }
    if(MT_UNF_PVR_PLAY_SPEED_2X_FAST_FORWARD==speed){
        trickmode = TM_FFWD_X2;
    }else if(MT_UNF_PVR_PLAY_SPEED_4X_FAST_FORWARD<=speed && MT_UNF_PVR_PLAY_SPEED_64X_FAST_FORWARD>=speed){
        trickmode = TM_FFWD_X2_MORE;
    }else if((MT_UNF_PVR_PLAY_SPEED_64X_FAST_BACKWARD<=speed && MT_UNF_PVR_PLAY_SPEED_1X_FAST_BACKWARD>=speed)||
        (MT_UNF_PVR_PLAY_SPEED_2X_SLOW_BACKWARD<=speed && MT_UNF_PVR_PLAY_SPEED_64X_SLOW_BACKWARD>=speed)){
        trickmode = TM_FREV;
    }
    else if(0<speed && MT_UNF_PVR_PLAY_SPEED_NORMAL > speed){
        if(MT_UNF_PVR_PLAY_SPEED_2X_SLOW_FORWARD == speed)
            trickmode = TM_SFWD_X2;
        else if(MT_UNF_PVR_PLAY_SPEED_4X_SLOW_FORWARD == speed)
            trickmode = TM_SFWD_X2_MORE;
    }else{
        trickmode = TM_NORMAL;
    }
    (void)MT_MPI_DISP_SetTrickMode(trickmode);

    MT_INFO_PVR("++tp.speed:%x\n", speed);
    MT_INFO_PVR("++tp.setframe:%d,%d\n", framerate.stSetFrmRate.u32fpsInteger,framerate.stSetFrmRate.u32fpsDecimal);
    MT_INFO_PVR("++tp.setmode:%d,%d,%d\n", stTrickParam.is_incomplete_stream,stTrickParam.trick_mode,trickmode);
    return 0 ;
}

MT_S32 MT_PVR_GetFileAttrByFileName(const MT_CHAR *pFileName, MT_UNF_PVR_FILE_ATTR_S *pAttr)
{
    MT_S32 ret;
    PVR_REC_CHN_S*  pstRecChnAttr = (PVR_REC_CHN_S *)NULL;
    PVR_PLAY_CHN_S*  pstPlayChnAttr = (PVR_PLAY_CHN_S *)NULL;

    if(PVR_Rec_IsFileSaving(pFileName))
    {
        pstRecChnAttr = PVRRecGetChnAttrByName(pFileName);

        if (pstRecChnAttr != NULL)
        {
             //PVR_Index_FlushIdxWriteCache(pstRecChnAttr->IndexHandle);
             ret = PVR_Index_PlayGetFileAttrByFileName(pFileName, pstRecChnAttr->IndexHandle, pAttr);
        }
        else
        {
            ret = PVR_Index_PlayGetFileAttrByFileName(pFileName, MT_NULL, pAttr);
        }
    }
    else
    {
        pstPlayChnAttr = (PVR_PLAY_CHN_S *)PVRPlayGetChnAttrByName(pFileName);

        if ((PVR_PLAY_CHN_S *)NULL != pstPlayChnAttr)
        {
            ret = PVR_Index_PlayGetFileAttrByFileName(pFileName, pstPlayChnAttr->IndexHandle, pAttr);
        }
        else
        {
            ret = PVR_Index_PlayGetFileAttrByFileName(pFileName, MT_NULL, pAttr);
        }
    }

    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    return MT_SUCCESS;
}


MT_S32 MT_PVR_GetTSBufferStatus(MT_U32 u32ChnID, MT_UNF_DMX_TSBUF_STATUS_S *Status)
{
    PVR_PLAY_CHN_S              *pChnAttr;
    PVR_PLAY_CHECK_CHN(u32ChnID);
    pChnAttr = &g_stPvrPlayChns[u32ChnID];
    return MT_UNF_DMX_GetTSBufferStatus(pChnAttr->hTsBuffer,Status);
}


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

