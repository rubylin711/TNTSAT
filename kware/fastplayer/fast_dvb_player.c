/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include "mt_type.h"
#include "sys_define.h"
#include "mtos_misc.h"
#include "mtos_task.h"
#include "mtos_sem.h"
#include "mtos_mutex.h"
#include "mtos_printk.h"
#include "mtos_mem.h"
#include "mtos_fifo.h"
#include "mtos_msg.h"

//drv
#include "mt_unf_frontend.h"
#include "mt_unf_demux.h"
#include "mt_mpi_demux.h"
#include "mt_unf_descrambler.h"
#include "mt_unf_avplay.h"
#include "mt_unf_disp.h"
#include "mt_unf_sound.h"
#include "mt_unf_vo.h"

#include "ts_sequence.h"
#include "fast_dvb_player.h"
#include "fast_dvb_player_priv.h"


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
//#if defined (DOLBYPLUS_HACODEC_SUPPORT)
#include "HA.AUDIO.DOLBYPLUS.decode.h"
//#endif
#include "HA.AUDIO.AC3PASSTHROUGH.decode.h"
#include "HA.AUDIO.DTSM6.decode.h"

#include "HA.AUDIO.DTSPASSTHROUGH.decode.h"
#include "HA.AUDIO.FFMPEG_DECODE.decode.h"
#include "HA.AUDIO.AAC.encode.h"


//#define FDP_DUMP_FILE

//#define FDP_DEBUG
#ifdef FDP_DEBUG
#define FDP_PRINTF printf
#else
#define FDP_PRINTF DUMMY_PRINTF
#endif

#define TS_PID(data) \
  ((mt_u16)(((mt_u8)(data[2])) | \
         (((mt_u16)((mt_u8)(data[1] & 0x1F))) << 8)))

/*!
  max push payload, must < 255k, dependes sonata dmx_dma
  */
#define MAX_PUSH_PAYLOAD (1024 * 188)

#define DMX_UNIT_SIZE (512 * 188)  //94k
#define ECM_ROLLBACK_SIZE (10 * 1024 * 188)  //2M
#define FOR_CAPTURE_ECM_SIZE (DMX_UNIT_SIZE * 5)  //480K
#define FOR_PLAY_SIZE (DMX_UNIT_SIZE * 10)  //960K
#define FOR_FIRST_SIZE (DMX_UNIT_SIZE * 10 * 3)  //960K *5 , fix me should be rollback size



#define    AVPLAY_DFT_VID_SIZE       (5*1024*1024)
#define    AVPLAY_TS_DFT_AUD_SIZE    (384*1024)

typedef enum tag_fast_dvb_player_cmd
{
  FDP_CMD_DUMMY,
  FDP_CMD_SW,
  FDP_CMD_STOP,
  FDP_CMD_CAPTURE_TABLE,
  FDP_CMD_CAPTURE_DONE,
  FDP_CMD_SET_CW,
  FDP_CMD_DESTROY,
}fast_dvb_player_cmd_t;

typedef enum tag_fast_dvb_player_sm
{
  /*!
    status machine idle
    */
  FDP_SM_IDLE = 0,
  /*!
    wait lock
    */
  FDP_SM_WAIT_LOCK,
  /*!
    record TS
    */
  FDP_SM_PRE_LOAD,
  /*!
    status machine monitor
    */
  FDP_SM_MONITOR,
}fast_dvb_player_sm_t;

typedef struct tag_fast_dvb_player_priv
{
 mt_handle hRecChn;
 mt_u32 u32RecDmxId;
 MT_UNF_DMX_PORT_E enPortId;
 mt_handle h_TsBuf;
 mt_u32 u32PlayerDmxId;
 mt_handle av_handle;

  mt_u32 q_id;
  mt_u32 prio;
  mt_handle_t user_handle;
  MT_BOOL block_ir_key;
  void *mutex;

  //fifo info
  mt_u32 fifo_size;
  mt_u8 *p_fifo;
  phys_addr_t fifo_phy;
  u64 data_got_total;
  u64 hw_rec_total;

  //state
  mt_u16 rec_in;
  fast_dvb_player_sm_t sm;
  MT_BOOL is_playing;
  MT_BOOL is_show;
  MT_BOOL is_wait_capture_table;
  MT_BOOL is_wait_capture_done;
  MT_BOOL is_wait_set_cw;
  MT_BOOL is_need_break;
  MT_BOOL is_locked;
  MT_BOOL is_using_special_extern_demod;
  MT_BOOL is_avhandle_byuser;

  //call back flag
  MT_BOOL send_lock_evt;
  MT_BOOL send_pg_done_evt;

  //play para
  mt_u32 start_tick;
  MT_UNF_fdp_play_param_t play_para;
  u32 stack[4 * 1024];  //16 KBYTES
  fdp_callback cb;
  fdp_callback_set_cw cb_set_cw;
  u64 total_push_len;
  mt_u8 u8DecOpenBuf[1024];


	mt_u32 Win;
	MT_HANDLE hTrack;

//#if defined (DOLBYPLUS_HACODEC_SUPPORT)
  DOLBYPLUS_STREAM_INFO_S stDDpStreamInfo;
//#endif
}fast_dvb_player_priv_t;

/*dolby Dual Mono type control*/
mt_u32  g_u32DolbyAcmod = 0;
MT_BOOL g_bDrawChnBar = MT_TRUE;


#ifdef FDP_DUMP_FILE
#include "lib_util.h"
#include "vfs.h"
//debug
#define FDP_DEBUG_DUMP_SIZE (40 * MBYTES)
hfile_t g_debug_hfile = NULL;
mt_u32 g_debug_dmup_size = 0;

static void _dump_data_stop(void)
{
  if(g_debug_hfile != NULL)
  {
    vfs_close(g_debug_hfile);
    g_debug_hfile = NULL;
    g_debug_dmup_size = 0;
  }
}

static mt_u32 utf8_to_unicode(mt_u16 *p_dst, mt_u32 outsize, mt_u8 *p_src, mt_u32 insize)
{
  mt_u32 totalNum = 0;
  mt_u8 *p_data = p_src;
  mt_u32 resultsize = 0;
  mt_u8 *p_tmp = (mt_u8 *)p_dst;
  mt_u32 i = 0;
  mt_u8 t1 = 0, t2 = 0;
  mt_u16 t3 = 0, t4 = 0, t5 = 0;
  mt_u32 tmp_size = 0;

  if(p_dst == NULL || p_src == NULL || insize < 0)
  {
    return -1;
  }

  for(i = 0; i < insize; i++)
  {
    if (*p_data >= 0x00 && *p_data <= 0x7f)
    {
      p_data ++;
      totalNum ++;
    }
    else if ((*p_data & (0xe0))== 0xc0)
    {
      p_data += 2;
      totalNum ++;
    }
    else if ((*p_data & (0xf0))== 0xe0)
    {
      p_data += 3;
      totalNum ++;
    }
  }

  if(outsize < totalNum)
  {
    return -1;
  }

  p_data = p_src;
  while(*p_data)
  {
    if (*p_data >= 0x00 && *p_data <= 0x7f)
    {
      *p_tmp = *p_data;
      p_tmp ++;
      p_tmp ++;
      resultsize += 1;
    }
    else if ((*p_data & 0xe0)== 0xc0)
    {
      t1 = *p_data & (0x1f);
      p_data ++;
      t2 = *p_data & (0x3f);

      *p_tmp = t2 | ((t1 & (0x03)) << 6);
      p_tmp ++;
      *p_tmp = t1 >> 2;
      p_tmp ++;
      resultsize += 1;
    }
    else if ((*p_data & (0xf0))== 0xe0)
    {
      t3 = *p_data & (0x1f);
      p_data ++;
      t4 = *p_data & (0x3f);
      p_data ++;
      t5 = *p_data & (0x3f);

      *p_tmp = ((t4 & (0x03)) << 6) | t5;
      p_tmp ++;
      *p_tmp = (t3 << 4) | (t4 >> 2);
      p_tmp ++;
      resultsize += 1;
    }
    p_data ++;
    tmp_size ++;
    if (tmp_size >= insize)
    {
      break;
    }
  }

  return resultsize;

}

static void _dump_data_start(mt_u16 v_pid)
{
  mt_u16 path[64] = {0};
  mt_u8 spath[64] = {0};

  _dump_data_stop();

  sprintf(spath, "C:\\fdp_dump_%d.ts", v_pid);
  FDP_PRINTF("Create: %s\n", spath);
  utf8_to_unicode(path, 64, spath, 64);

  g_debug_dmup_size = 0;
  g_debug_hfile = vfs_open(path, VFS_NEW);
  if(g_debug_hfile != NULL)
  {
    FDP_PRINTF("Create: %s success\n", spath);
  }
  else
  {
    FDP_PRINTF("Create: %s fail\n", spath);
  }
}

static void _dump_data_write(mt_u8 *p_buffer, mt_u32 size)
{
  if(g_debug_hfile == NULL)
  {
    return;
  }
  else if(g_debug_dmup_size >= FDP_DEBUG_DUMP_SIZE)
  {
    _dump_data_stop();
    FDP_PRINTF("dump file done ~~~~~~~~~~\n");
    return;
  }

  vfs_write(p_buffer, size, 1, g_debug_hfile);
  g_debug_dmup_size += size;
}
#endif

static mt_u32 fdp_dataflow(fast_dvb_player_priv_t *p_priv, mt_u32 push_len, MT_BOOL fast);  //push_len: 0 for auto
static MT_BOOL fdp_check_nim(fast_dvb_player_priv_t *p_priv);
static RET_CODE fdp_stop_av(fast_dvb_player_priv_t *p_priv, MT_BOOL b_freeze_stop);
static RET_CODE fdp_start_av(fast_dvb_player_priv_t *p_priv);
static void fdp_lock_tuner(fast_dvb_player_priv_t *p_priv, MT_UNF_fdp_play_param_t *p_param)
{
	mt_s32 ret = 0;
	MT_UNF_fdp_tp_info_t tp_info;
	memcpy(&tp_info,&p_param->tp_info,sizeof(MT_UNF_fdp_tp_info_t));
	ret = mt_unf_fe_connect(p_param->tp_info.tuner_id, &(p_param->tp_info.conn_para), p_param->tp_info.timeout);
	memcpy(&p_param->tp_info,&tp_info,sizeof(MT_UNF_fdp_tp_info_t));
	if( ret !=MT_SUCCESS )
		printf("fdp_lock_tuner ret = %d\n",ret);
}

static u64 fdp_get_dmx_data_size(fast_dvb_player_priv_t *p_priv)
{
  u64 demx_size    = 0;
  u32 sr = 0;
  MT_UNF_DMX_RECBUF_STATUS_S stBufStatus;
  mt_s32 ret;
  memset(&stBufStatus,0,sizeof(MT_UNF_DMX_RECBUF_STATUS_S));
  mtos_critical_enter(&sr);
 ret = MT_UNF_DMX_GetRecBufferStatus(p_priv->hRecChn, &stBufStatus);
 if (MT_SUCCESS != ret) {
	 FDP_PRINTF("\n MT_UNF_DMX_GetRecBufferStatus fail \n\n\n");

 }
 demx_size = stBufStatus.u64BufDataLen;
  //FDP_PRINTF("u64BufDataLen=  %llx\n", stBufStatus.u64BufDataLen);
  mtos_critical_exit(sr);

  if(demx_size < p_priv->data_got_total)
  {
    demx_size = p_priv->data_got_total;
  }
  p_priv->hw_rec_total = demx_size;
  return demx_size;
}

static mt_u32 fdp_fill_data(fast_dvb_player_priv_t *p_priv, mt_u8 **pp_data,phys_addr_t *p_phydata, mt_u32 max_len)//max_len: 0 for auto
{
  u64 demx_size    = 0;
  u64 get_size     = 0;
  u64 left_size    = 0;
  u64 offset       = 0;
  u64 overflow_len = 0;
  u64 fill_align = 4 * 188; //align to 16

  demx_size = fdp_get_dmx_data_size(p_priv);

  get_size = demx_size - p_priv->data_got_total;

  if(get_size == 0)
  {
    //FDP_PRINTF("^");
  }

  if(get_size >= 4*188)
  {
    if(get_size > p_priv->fifo_size)
    {
      overflow_len = get_size - p_priv->fifo_size;
      FDP_PRINTF("\n\n\nrec data overflow %lld packet !!~~~~~~~~\n\n\n", overflow_len / 188);

      //find to the don't overwrite buffer
      p_priv->data_got_total += overflow_len;
      get_size -= overflow_len;
    }

    offset = p_priv->data_got_total % (u64)p_priv->fifo_size;
    *pp_data = p_priv->p_fifo+ (mt_u32)offset;
    *p_phydata = p_priv->fifo_phy+ offset;
	//FDP_PRINTF("\p_priv->fifo_phy+ offset = %x  \n\n\n", (p_priv->p_fifo+ (mt_u32)offset)[0]);
    left_size = (u64)p_priv->fifo_size - offset;

    if(get_size > left_size)
    {
      get_size = left_size;
    }

    if((max_len != 0) && (get_size > max_len))
    {
      get_size = max_len;
    }

    get_size = get_size / fill_align * fill_align;

    p_priv->data_got_total += get_size;
    return get_size;
  }
  return 0;
}

static void fdp_check_payload(fast_dvb_player_priv_t *p_priv, mt_u8 *p_data,
                        mt_u32 len, mt_u32 *p_video_len, mt_u32 *p_audio_len)
{
  mt_u8 *p_packet = p_data;
  mt_u32 audio_len = 0;
  mt_u32 video_len = 0;
  mt_u32 loopi = 0;
  mt_u16 apid = p_priv->play_para.pg_info.a_pid;
  mt_u16 vpid = p_priv->play_para.pg_info.v_pid;

  for(loopi = 0; loopi < len; loopi += 188)
  {
    if(TS_PID(p_packet) == vpid)
    {
      video_len += 188;
    }
    else if(TS_PID(p_packet) == apid)
    {
      audio_len += 188;
    }
    p_packet += 188;
  }

  *p_video_len = video_len;
  *p_audio_len = audio_len;
}

static inline mt_u32 fdp_dma_dbg_get_es_len(void)
{
    return *((volatile mt_u32 *)0xbf260134); //symphony es write point
}

static inline mt_u32 fdp_get_left_ves_len(void)
{
    return 0; //symphony left video es, todo
}


static MT_BOOL fdp_dma_idle(fast_dvb_player_priv_t *p_priv,
                            mt_u8 *p_data, mt_u32 ts_len, mt_u32 v_payload, mt_u32 a_payload)
{


  s32 ret = 0;
  MT_UNF_STREAM_BUF_S stData;

  ret = MT_UNF_AVPLAY_GetBuf(p_priv->av_handle,MT_UNF_AVPLAY_BUF_ID_ES_VID, v_payload, &stData, 0);

  if (stData.u32Size < v_payload)
  {
	  return TRUE;
  }

  ret = MT_UNF_AVPLAY_GetBuf(p_priv->av_handle,MT_UNF_AVPLAY_BUF_ID_ES_AUD, a_payload, &stData, 0);

  if (stData.u32Size < a_payload)
  {
	  return TRUE;
  }
  return FALSE;

}

static MT_BOOL fdp_render_data(fast_dvb_player_priv_t *p_priv, mt_u8 *p_data,  phys_addr_t phydata, mt_u32 len, MT_BOOL fast)
{
  mt_u32 video_payload = 0;
  mt_u32 audio_payload = 0;
  mt_u32 retry_time = 0;
  static mt_u32 fail_time = 0;
  mt_s32 ret =0;

#if 0 //temp marked
  //check A/V data
  fdp_check_payload(p_priv, p_data, len, &video_payload, &audio_payload);

  while(!fdp_dma_idle(p_priv, p_data, len, video_payload, audio_payload))
  {
    if(retry_time > 100)  //wait 1s
    {
      FDP_PRINTF(" fdp_render_data retry fail\n");
      fail_time++;

      if(fail_time >= 10) //error 10s reset dmx
      {
        FDP_PRINTF(" fdp_render_data reset DMX\n");
        fdp_stop_av(p_priv, TRUE);
       // dmx_av_reset(p_priv->p_dmx_dev);
        fdp_start_av(p_priv);
        fail_time = 0;
      }
      return FALSE;
    }
    if(fast)
    {
      return FALSE;
    }
    if(p_priv->is_need_break)
    {
      return FALSE;
    }
	#if 0
    if((chip_ic_id == IC_TANGO) && (!p_priv->send_pg_done_evt) && (retry_time > 0)) //tango patch
    {
      int i = 0;
      for(i = 0; i < 1000; i++)
      {
        if(dmx_get_dma_state(p_priv->p_dmx_dev))
        {
          break;
        }
      }
    }
    else
	#endif
    {
      mtos_task_sleep(10);
    }
    retry_time++;
  }
#endif


  MT_UNF_STREAM_BUF_S StreamBuf;
do{
  ret =MT_UNF_DMX_GetTSBuffer(p_priv->h_TsBuf, len, &StreamBuf, 1000);
  if (MT_SUCCESS == ret){
	 // ret = MT_MPI_DMX_PutTSBufferEx(p_priv->h_TsBuf,len, 0, p_phydata);
	  //FDP_PRINTF("MT_MPI_DMX_PutTSBufferEx %x  %d   ret = %d\n",p_data, len,ret  );
	 // p_priv->total_push_len += len;
	  ret = MT_MPI_DMX_PutTSBufferEx(p_priv->h_TsBuf,StreamBuf.u32Size, 0, phydata);
	  p_priv->total_push_len +=StreamBuf.u32Size;
	  len-=StreamBuf.u32Size;
  }else{
	mtos_task_sleep(10);
  }
}while(len);
  fail_time = 0;

#if 0
  #ifdef FDP_DUMP_FILE
  _dump_data_write(p_data, len);
  #endif
  //FDP_PRINTF("fdp--->%s push %d \n", __FUNCTION__, len);
  MT_ASSERT(((mt_u32)p_data % 16) == 0);
  dma_cfg.data_length = len;
  dma_cfg.mem_address = (mt_u32)p_data;
  dma_cfg.ts_clk = 600 * MBYTES;
  dmx_set_dma_config(p_priv->p_dmx_dev, &dma_cfg);
  p_priv->total_push_len += len;
  fail_time = 0;
  //FDP_PRINTF("@\n");

  /*
  while(!dmx_get_dma_state(p_priv->p_dmx_dev))
  {
  }
  */
#endif
  return TRUE;
}

static RET_CODE fdp_start_rec(fast_dvb_player_priv_t *p_priv)
{
  RET_CODE ret = SUCCESS;
  MT_UNF_DMX_RECBUF_STATUS_S stBufStatus;
 // return 0;
 if(p_priv->hRecChn != 0xFFFF)
	 return SUCCESS;

  MT_UNF_DMX_REC_ATTR_S stRecAttr;
  memset(&stRecAttr, 0 ,sizeof(MT_UNF_DMX_REC_ATTR_S));
  stRecAttr.u32DmxId		= p_priv->u32RecDmxId;
  stRecAttr.u32RecBufSize	= p_priv->fifo_size;
  stRecAttr.bDescramed = MT_FALSE;
  stRecAttr.enIndexType= MT_UNF_DMX_REC_INDEX_TYPE_NONE;
  stRecAttr.enRecType	= MT_UNF_DMX_REC_TYPE_ALL_PID;
  stRecAttr.type_mode	= DMX_FULL_TS_WITHOUT_NULL_PACKET;



  FDP_PRINTF("orig p_priv->fifo_size %d    stRecAttr.u32DmxId = %d   p_priv->enPortId = %d\n",p_priv->fifo_size,stRecAttr.u32DmxId ,p_priv->enPortId);
  ret |= MT_UNF_DMX_CreateRecChn(&stRecAttr, &p_priv->hRecChn);
  ret |= MT_UNF_DMX_StartRecChn(p_priv->hRecChn);


  MT_UNF_DMX_GetRecBufferStatus(p_priv->hRecChn, &stBufStatus);
  p_priv->p_fifo= (mt_u8 *)stBufStatus.u32BufUsrAddr;
  p_priv->fifo_phy= stBufStatus.u32BufPhyAddr;
  FDP_PRINTF("mt_u32BufPhyAddr   = %x   end = %d  stBufStatus.mt_u32BufSize %d   ret = %d\n",stBufStatus.u32BufPhyAddr ,stBufStatus.u32BufPhyAddr+ stBufStatus.u32BufSize,stBufStatus.u32BufSize,ret );
  return ret;
}

static RET_CODE fdp_stop_rec(fast_dvb_player_priv_t *p_priv, MT_BOOL b_force_stop)
{
  if(p_priv->hRecChn != 0xFFFF){
  	  if(!b_force_stop)
	  	return SUCCESS;////////////////////////////////////

	  MT_UNF_DMX_StopRecChn( p_priv->hRecChn);
	  MT_UNF_DMX_DestroyRecChn(p_priv->hRecChn);
	  p_priv->hRecChn = 0xFFFF;
  }
  return SUCCESS;
}

static void  fdp_setvolume_1(mt_u8 volume)
{
    MT_UNF_SND_GAIN_ATTR_S vs;
    int ret = 0;
#if 1//def __LINUX__
    vs.s32Gain = volume;
#else
    vs.s32Gain = volume * 100 / 31;
#endif
    vs.bLinearMode = MT_TRUE;

    ret = MT_UNF_SND_SetVolume(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, &vs);
}

static void fdp_set_audio_mode_1(MT_UNF_TRACK_MODE_E mode)
{

    MT_UNF_SND_SetTrackMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, mode);
    MT_UNF_SND_SetTrackMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_DAC0, mode);
}


static mt_s32 fdp_av_set_vdecattrr(mt_handle hAvplay,MT_UNF_VCODEC_TYPE_E enType,MT_UNF_VCODEC_MODE_E enMode)
{
    mt_s32 Ret;
    MT_UNF_VCODEC_ATTR_S        VdecAttr;

    Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (MT_SUCCESS != Ret)
    {
        FDP_PRINTF("MT_UNF_AVPLAY_GetAttr failed:%#x\n",Ret);
        return Ret;
    }

    VdecAttr.enType = enType;
    VdecAttr.enMode = enMode;
    VdecAttr.u32ErrCover = 100;
    VdecAttr.u32Priority = 3;

    Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
    if (Ret != MT_SUCCESS)
    {
        FDP_PRINTF("call MT_UNF_AVPLAY_SetAttr failed.\n");
        return Ret;
    }

    return Ret;
}

static mt_void DDPlusCallBack(DOLBYPLUS_EVENT_E Event, mt_void *pUserData)
{
    DOLBYPLUS_STREAM_INFO_S *pstInfo = (DOLBYPLUS_STREAM_INFO_S *)pUserData;

#if 0
    sample_common_printf( "DDPlusCallBack show info:\n \
                s16StreamType          = %d\n \
                s16Acmod               = %d\n \
                s32BitRate             = %d\n \
                s32SampleRateRate      = %d\n \
                Event                  = %d\n",
                pstInfo->s16StreamType, pstInfo->s16Acmod, pstInfo->s32BitRate, pstInfo->s32SampleRateRate,Event);
#endif
    g_u32DolbyAcmod = pstInfo->s16Acmod;

    if (HA_DOLBYPLUS_EVENT_SOURCE_CHANGE == Event)
    {
        g_bDrawChnBar = MT_TRUE;
        //printf("DDPlusCallBack enent !\n");
    }
    return;
}

static mt_s32 fdp_av_set_adecattr(fast_dvb_player_priv_t *p_priv,mt_handle hAvplay, mt_u32 enADecType, MT_HA_DECODEMODE_E enMode, mt_s32 isCoreOnly)
{
    MT_UNF_ACODEC_ATTR_S AdecAttr;
    WAV_FORMAT_S stWavFormat;


   MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
    AdecAttr.enType = enADecType;
    if (HA_AUDIO_ID_PCM == AdecAttr.enType)
    {
    	/* set pcm wav format here base on pcm file 48k.raw */
        stWavFormat.nChannels = 1;
        stWavFormat.nSamplesPerSec = 48000;
        stWavFormat.wBitsPerSample = 16;
        HA_PCM_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam),&stWavFormat);
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
        AMRNB_DECODE_OPENCONFIG_S *pstConfig = (AMRNB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
        HA_AMRNB_GetDecDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        pstConfig->enFormat = AMRNB_MIME;
    }
    else if (HA_AUDIO_ID_AMRWB== AdecAttr.enType)
    {
        AMRWB_DECODE_OPENCONFIG_S *pstConfig = (AMRWB_DECODE_OPENCONFIG_S *)u8DecOpenBuf;
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
            FDP_PRINTF(" MLP decoder enMode(%d) error (mlp only support hbr Pass-through only).\n", enMode);
            return -1;
        }

        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_THRU;        /* truehd just support pass-through */
        FDP_PRINTF(" TrueHD decoder(HBR Pass-through only).\n");
    }
    else if (HA_AUDIO_ID_DOLBY_TRUEHD == AdecAttr.enType)
    {
          TRUEHD_DECODE_OPENCONFIG_S *pstConfig = (TRUEHD_DECODE_OPENCONFIG_S *)p_priv->u8DecOpenBuf;
        HA_DOLBY_TRUEHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DOLBY_TRUEHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
    else if (HA_AUDIO_ID_DTSHD == AdecAttr.enType)
    {
        DTSHD_DECODE_OPENCONFIG_S *pstConfig = (DTSHD_DECODE_OPENCONFIG_S *)p_priv->u8DecOpenBuf;
        HA_DTSHD_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSHD_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
    else if (HA_AUDIO_ID_DTSM6 == AdecAttr.enType)
    {
        DTSM6_DECODE_OPENCONFIG_S *pstConfig = (DTSM6_DECODE_OPENCONFIG_S *)p_priv->u8DecOpenBuf;
        HA_DTSM6_DecGetDefalutOpenConfig(pstConfig);
        HA_DTSM6_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
    }
//#if defined (DOLBYPLUS_HACODEC_SUPPORT)
    else if (HA_AUDIO_ID_DOLBY_PLUS == AdecAttr.enType)
    {
        DOLBYPLUS_DECODE_OPENCONFIG_S *pstConfig = (DOLBYPLUS_DECODE_OPENCONFIG_S *)p_priv->u8DecOpenBuf;
        HA_DOLBYPLUS_DecGetDefalutOpenConfig(pstConfig);
        pstConfig->pfnEvtCbFunc[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = DDPlusCallBack;
        pstConfig->pAppData[HA_DOLBYPLUS_EVENT_SOURCE_CHANGE] = &p_priv->stDDpStreamInfo;
        /* Dolby DVB Broadcast default settings */
        pstConfig->enDrcMode = DOLBYPLUS_DRC_RF;
        pstConfig->enDmxMode = DOLBYPLUS_DMX_SRND;
        HA_DOLBYPLUS_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        AdecAttr.stDecodeParam.enDecMode = HD_DEC_MODE_SIMUL;
    }
//#endif
    else if(HA_AUDIO_ID_DRA == AdecAttr.enType)
    {
//       HA_DRA_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam));
         HA_DRA_DecGetOpenParam_MultichPcm(&(AdecAttr.stDecodeParam));
    }
    else if(HA_AUDIO_ID_COOK == AdecAttr.enType || HA_AUDIO_ID_AMRNB ==AdecAttr.enType
        || HA_AUDIO_ID_AMRWB == AdecAttr.enType)
    {
        HA_FFMPEG_DECODE_OPENCONFIG_S *pstConfig = (HA_FFMPEG_DECODE_OPENCONFIG_S *)p_priv->u8DecOpenBuf;
        HA_FFMPEG_DecGetDefalutOpenConfig(pstConfig);
        HA_FFMPEGC_DecGetDefalutOpenParam(&(AdecAttr.stDecodeParam), pstConfig);
        FDP_PRINTF("cook dec set ffmpeg dec param \n");
    }
	#if 1
    if (AdecAttr.enType == HA_AUDIO_ID_AC3PASSTHROUGH) {
		MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_RAW);
    } else {
		MT_UNF_SND_SetHdmiMode(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_HDMI0, MT_UNF_SND_HDMI_MODE_LPCM);
    }
	#endif

    MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

    return MT_SUCCESS;
}


static RET_CODE fast_dvb_player_av_reglib()
{
    MT_S32 Ret = MT_SUCCESS;

    Ret = MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRWB.codec.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP3.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.MP2.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AAC.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYTRUEHD.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DRA.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.TRUEHDPASSTHROUGH.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AMRNB.codec.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.WMA.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.COOK.decode.so");
#ifdef DOLBYPLUS_HACODEC_SUPPORT
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DOLBYPLUS.decode.so");
#endif
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSHD.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSM6.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.DTSPASSTHROUGH.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.AC3PASSTHROUGH.decode.so");
    Ret |= MT_UNF_AVPLAY_RegisterAcodecLib("libHA.AUDIO.PCM.decode.so");

    if (Ret != MT_SUCCESS) {
	printf("\n\n!!! some audio codec NOT found. you may NOT able to decode "
	       "some audio type.\n\n");
    }

    return MT_SUCCESS;
}

static RET_CODE fast_dvb_player_av_init(MT_UNF_AVPLAY_STREAM_TYPE_E streamType,mt_u32  u32DemuxId,fast_dvb_player_priv_t *p_priv)
{
    MT_S32 ret = 0;
    MT_S32 ret1 = 0;
    MT_UNF_SYNC_ATTR_S AvSyncAttr;

	MT_UNF_AVPLAY_ATTR_S avAttr;
	if(!p_priv->is_avhandle_byuser)
	{
		p_priv->av_handle = -1;

	    memset(&(avAttr),0,sizeof(MT_UNF_AVPLAY_ATTR_S));
	    memset(&AvSyncAttr,0,sizeof(MT_UNF_SYNC_ATTR_S));
	    ret = MT_UNF_AVPLAY_Init();
	    if (ret != MT_SUCCESS) {
		return MT_FAILURE;
	    }
	    ret = fast_dvb_player_av_reglib();
	    if (ret != MT_SUCCESS) {
		return MT_FAILURE;
	    }
	    ret = MT_UNF_AVPLAY_GetDefaultConfig(&avAttr, streamType);
		 ret |= MT_UNF_DMX_AttachTSPort(u32DemuxId, p_priv->enPortId);
		if (MT_SUCCESS != ret) {
			FDP_PRINTF("call MT_UNF_DMX_AttachTSPort failed.\n");
			return MT_SUCCESS;
		}

		 ret |= MT_UNF_DMX_CreateTSBuffer(p_priv->enPortId, 0x400000, &p_priv->h_TsBuf);

	    avAttr.stStreamAttr.enStreamType = streamType;
		avAttr.u32DemuxId =u32DemuxId;

		avAttr.stStreamAttr.u32VidBufSize = AVPLAY_DFT_VID_SIZE ;//1024;
		avAttr.stStreamAttr.u32AudBufSize=AVPLAY_TS_DFT_AUD_SIZE;// 1024;

	    ret |= MT_UNF_AVPLAY_Create(&avAttr, &p_priv->av_handle);
	    if (ret != MT_SUCCESS) {
		return ret;
	    }
		#if 1
	    ret1 = MT_UNF_AVPLAY_ChnOpen(p_priv->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	    ret |= MT_UNF_AVPLAY_ChnOpen(p_priv->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	    if (MT_SUCCESS != ret && MT_SUCCESS != ret1) {
	        printf("08.0.ZZZZZZZZZZZZZZZZZZZZZZZZZZ %d %d\n",ret,ret1);
			return ret;
	    }
	    #endif
	    ret = MT_UNF_VO_AttachWindow(p_priv->Win, p_priv->av_handle);
	    if (MT_SUCCESS != ret) {
		return ret;
	    }

	    ret = MT_UNF_VO_SetWindowEnable(p_priv->Win, MT_TRUE);
	    if (MT_SUCCESS != ret) {
		return ret;
	    }

#if 1
	    ret = MT_UNF_SND_Attach(p_priv->hTrack, p_priv->av_handle);
	    if (ret != MT_SUCCESS) {
		return ret;
	    }
#endif
	    ret = MT_UNF_AVPLAY_GetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
	    AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
	    ret |= MT_UNF_AVPLAY_SetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
	}else{
		mtos_printk("111 p_priv->enPortId = %x\n", p_priv->enPortId);
		 ret |= MT_UNF_DMX_AttachTSPort(u32DemuxId, p_priv->enPortId);
		if (MT_SUCCESS != ret) {
			FDP_PRINTF("call MT_UNF_DMX_AttachTSPort failed.\n");
			return MT_SUCCESS;
		}
		 ret |= MT_UNF_DMX_CreateTSBuffer(p_priv->enPortId, 0x400000, &p_priv->h_TsBuf);
	}
    return ret;
}

static RET_CODE fast_dvb_player_av_uninit(fast_dvb_player_priv_t *p_priv)
{
    if (p_priv->av_handle == -1)
		return 0;

	if(!p_priv->is_avhandle_byuser)
	{
	    MT_UNF_SND_Detach(p_priv->hTrack, p_priv->av_handle);
	    //MT_UNF_SND_DestroyTrack(g_fast_av_priv.hTrack);
	    MT_UNF_VO_SetWindowEnable(p_priv->Win, MT_FALSE);
	    MT_UNF_VO_DetachWindow(p_priv->Win, p_priv->av_handle);
	    MT_UNF_AVPLAY_ChnClose(p_priv->av_handle,
	                           (MT_UNF_AVPLAY_MEDIA_CHAN_E)(MT_UNF_AVPLAY_MEDIA_CHAN_VID |
	                                                        MT_UNF_AVPLAY_MEDIA_CHAN_AUD));
	    MT_UNF_AVPLAY_Destroy(p_priv->av_handle);
	    MT_UNF_AVPLAY_DeInit();
	}
    p_priv->av_handle = -1;
    return 0;
}

static RET_CODE fdp_start_av(fast_dvb_player_priv_t *p_priv)
{
	s32 ret = 0;
	MT_UNF_VCODEC_ATTR_S		VdecAttr;
	MT_UNF_ACODEC_ATTR_S		AdecAttr;
	mt_u8 times = 0;
//return 0;
    MT_UNF_AVPLAY_STOP_OPT_S StopOpt;
    StopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    StopOpt.u32TimeoutMs = 0;
    ret = MT_UNF_AVPLAY_Stop(p_priv->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &StopOpt);

  MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, TRUE);

   if(p_priv->play_para.pg_info.a_pid == 0x1fff)
   	p_priv->play_para.pg_info.a_pid = 0;

   if(p_priv->play_para.pg_info.v_pid == 0x1fff)
   	p_priv->play_para.pg_info.v_pid = 0;

   if(p_priv->play_para.pg_info.pcr_pid == 0x1fff)
   	p_priv->play_para.pg_info.pcr_pid = 0;

  #if 0
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync;
    DmxAvsync.VdecType = p_priv->play_para.pg_info.video_type;
    DmxAvsync.AdecType = p_priv->play_para.pg_info.audio_type;
    DmxAvsync.AvsyncFlage = 1;
    MT_UNF_AVPLAY_SetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
 #else
  if (p_priv->play_para.pg_info.a_pid!=0  &&  p_priv->play_para.pg_info.v_pid!=0){
	    MT_UNF_SYNC_ATTR_S AvSyncAttr;
	    MT_UNF_AVPLAY_GetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
	    //if (1 == syncFlag) {
		AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
	    //} else {
		//AvSyncAttr.enSyncRef = MT_UNF_SYNC_REF_NONE;
	   // }
	    MT_UNF_AVPLAY_SetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_SYNC, &AvSyncAttr);
  	}
  #endif



	if (p_priv->play_para.pg_info.pcr_pid)
	{
	  ret = MT_UNF_AVPLAY_SetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, (mt_u16 *)&p_priv->play_para.pg_info.pcr_pid);
	}

	if (p_priv->play_para.pg_info.a_pid!=0 ){
	  AdecAttr.enType = p_priv->play_para.pg_info.audio_type;
	  ret = fdp_av_set_adecattr(p_priv,p_priv->av_handle, AdecAttr.enType, HD_DEC_MODE_RAWPCM, 1);

	  ret = MT_UNF_AVPLAY_SetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, (mt_u16 *)&p_priv->play_para.pg_info.a_pid);
	  ret |= MT_UNF_AVPLAY_Start(p_priv->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_AUD,MT_NULL);
	  FDP_PRINTF("\n##MT_UNF_AVPLAY_Start a[%ld]", ret);
	}
 	 if (p_priv->play_para.pg_info.v_pid!=0 ){
		VdecAttr.enType = p_priv->play_para.pg_info.video_type;

		ret = fdp_av_set_vdecattrr(p_priv->av_handle, VdecAttr.enType , MT_UNF_VCODEC_MODE_NORMAL);
		ret = MT_UNF_AVPLAY_SetAttr(p_priv->av_handle, MT_UNF_AVPLAY_ATTR_ID_VID_PID, (mt_u16 *)&p_priv->play_para.pg_info.v_pid);
		ret |= MT_UNF_AVPLAY_Start(p_priv->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID,MT_NULL);
		FDP_PRINTF("\n##MT_UNF_AVPLAY_Start v[%ld]", ret);
  	}

  //  MT_UNF_AVPLAY_SetAvsyncMode(p_priv->av_handle, MT_VDEC_AVSYNC_PVR);
   fdp_setvolume_1(p_priv->play_para.pg_info.audio_volume);
   fdp_set_audio_mode_1(p_priv->play_para.pg_info.audio_track);
   p_priv->is_playing = TRUE;
   MT_UNF_SND_SetMute(MT_UNF_SND_0, MT_UNF_SND_OUTPUTPORT_ALL, FALSE);

  return SUCCESS;
}

static RET_CODE fdp_stop_av(fast_dvb_player_priv_t *p_priv, MT_BOOL b_freeze_stop)
{
  s32 ret = 0;

  if(!p_priv->is_playing)
  {
    return ret;
  }
  MT_UNF_AVPLAY_STOP_OPT_S stopOpt;
  stopOpt.u32TimeoutMs = 0;
  if(!b_freeze_stop)
  {
      stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
  }
  else
  {
      stopOpt.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;//MT_UNF_AVPLAY_STOP_MODE_FREEZE_NO_STILL;
  }
  ret = MT_UNF_AVPLAY_Stop(p_priv->av_handle, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stopOpt);

  FDP_PRINTF("\n##MT_UNF_AVPLAY_Stop [%ld]", ret);

  p_priv->is_playing = FALSE;
 return ret;
}

static void fdp_av_render_reset(fast_dvb_player_priv_t *p_priv)
{
  mt_u32 pre_push_len = MAX_PUSH_PAYLOAD;
  mt_u32 real_push_len = 0;

	pre_push_len = FOR_FIRST_SIZE * 2;
  p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_SWITCH_PG_START, 0, 0);

  FDP_PERF_TEST(FDP_PERF_TEST_RENDER_RESET_S);
  fdp_stop_av(p_priv, p_priv->play_para.stop_mode == 0);

  //set cw
  if(p_priv->play_para.pg_info.is_scrambled)
  {
    if(p_priv->cb_set_cw != NULL)
    {
      MT_BOOL ret = p_priv->cb_set_cw(
        p_priv->play_para.tp_info.conn_para.connect_param.sat.freq,
        p_priv->play_para.pg_info.s_id);
      FDP_PRINTF("set cw ret %d\n", ret);
      ret = ret;  //for build warnning
    }
  }


  real_push_len =  fdp_dataflow(p_priv, pre_push_len, FALSE);
  if (real_push_len < 2 * MBYTES)
  {
    fdp_dataflow(p_priv, pre_push_len - real_push_len, FALSE);
  }
  fdp_start_av(p_priv);
  fdp_dataflow(p_priv, FOR_PLAY_SIZE * 3, FALSE);

  FDP_PERF_TEST(FDP_PERF_TEST_RENDER_RESET_E);
}

static void fdp_av_render_switch(fast_dvb_player_priv_t *p_priv, MT_BOOL is_same_tp)
{
  ts_detail_info_t detail = {0};
  u64 dmx_data_total = 0;
  u64 rollback_data_total = 0;
  mt_u32 rollback_size = p_priv->fifo_size / 4 * 3;  //rollback to found the last I frame
  mt_u32 offset = 0;
  MT_BOOL b_hw = TRUE;
  p_priv->total_push_len = 0;
  if(!is_same_tp) //reset all
  {
  	MT_UNF_DMX_ResetTSBuffer(p_priv->h_TsBuf);
    p_priv->data_got_total = 0;
  }
  else if(!fdp_check_nim(p_priv)) //unlock don't rollback
  {
  }
  else if(b_hw)
  {
    dmx_data_total = fdp_get_dmx_data_size(p_priv);

    if(dmx_data_total > rollback_size)
    {
      p_priv->data_got_total = dmx_data_total - rollback_size;
    }
    else
    {
      p_priv->data_got_total = 0;
      rollback_size = dmx_data_total;
    }
  }
  else
  {
    dmx_data_total = fdp_get_dmx_data_size(p_priv);
    if(dmx_data_total > rollback_size)
    {
      rollback_data_total = dmx_data_total - rollback_size;
    }
    else
    {
      rollback_data_total = 0;
      rollback_size = dmx_data_total;
    }

    offset = (mt_u32)(rollback_data_total % (u64)p_priv->fifo_size);

    if((offset + rollback_size) > p_priv->fifo_size)
    {
      mt_u32 end_rollback_size = (offset + rollback_size) - p_priv->fifo_size;
      mt_u32 head_rollback_size = rollback_size - end_rollback_size;

      //last part
      ts_seq_get_vkey_frame_fastswitch((char *)p_priv->p_fifo + 0, end_rollback_size,
          p_priv->play_para.pg_info.video_type,
          p_priv->play_para.pg_info.v_pid, &detail);
      if(detail.fragment_num > 0) //found
      {
        p_priv->data_got_total = rollback_data_total
          + head_rollback_size + (u64)detail.fragment_offset[detail.fragment_num - 1];
        FDP_PRINTF("A. find IF roll %lld\n", dmx_data_total - p_priv->data_got_total);
        //fdt_test(p_priv, detail.fragment_offset[detail.fragment_num - 1],
        //  end_rollback_size - detail.fragment_offset[detail.fragment_num - 1]);
      }
      else  //head part
      {
        ts_seq_get_vkey_frame_fastswitch((char *)p_priv->p_fifo + offset, head_rollback_size,
            p_priv->play_para.pg_info.video_type,
            p_priv->play_para.pg_info.v_pid, &detail);
        if(detail.fragment_num > 0) //found
        {
          p_priv->data_got_total = rollback_data_total + (u64)detail.fragment_offset[detail.fragment_num - 1];
          FDP_PRINTF("B. find IF roll %lld\n", dmx_data_total - p_priv->data_got_total);
          //fdt_test(p_priv, offset + detail.fragment_offset[detail.fragment_num - 1],
          //  head_rollback_size - detail.fragment_offset[detail.fragment_num - 1]);
        }
        else  // no found
        {
          p_priv->data_got_total = rollback_data_total;
          FDP_PRINTF("C. no find IF roll %lld\n", dmx_data_total - p_priv->data_got_total);
        }
      }
    }
    else
    {
      ts_seq_get_vkey_frame_fastswitch((char *)p_priv->p_fifo + offset, rollback_size,
          p_priv->play_para.pg_info.video_type,
          p_priv->play_para.pg_info.v_pid, &detail);
      if(detail.fragment_num > 0) //found
      {
        p_priv->data_got_total = rollback_data_total + (u64)detail.fragment_offset[detail.fragment_num - 1];
        FDP_PRINTF("D. find IF roll %lld\n", dmx_data_total - p_priv->data_got_total);
        //fdt_test(p_priv, offset + detail.fragment_offset[detail.fragment_num - 1],
        //  rollback_size - detail.fragment_offset[detail.fragment_num - 1]);
      }
      else
      {
        p_priv->data_got_total = rollback_data_total;
        FDP_PRINTF("E. no find IF roll %lld\n", dmx_data_total - p_priv->data_got_total);
      }
    }
  }

  //fix me 188?
  if((p_priv->data_got_total % 8) != 0)
  {
    if(p_priv->data_got_total >= 188)
    {
      p_priv->data_got_total -= 188;
    }
    else
    {
      p_priv->data_got_total = 0;
    }
  }
  if((p_priv->data_got_total % 16) != 0)
  {
    if(p_priv->data_got_total >= 188 * 2)
    {
      p_priv->data_got_total -= 188 *2;
    }
    else
    {
      p_priv->data_got_total = 0;
    }
  }
}

static void fdp_destroy(fast_dvb_player_priv_t *p_priv)
{
  mt_u32 q_id = p_priv->q_id;

  fdp_stop_rec(p_priv,TRUE);
  fdp_stop_av(p_priv, FALSE);

  fast_dvb_player_av_uninit(p_priv);
  MT_UNF_DMX_DetachTSPort(p_priv->u32PlayerDmxId);
  MT_UNF_DMX_DestroyTSBuffer(p_priv->h_TsBuf);


  mtos_mutex_delete(p_priv->mutex);
 // mtos_messageq_release(p_priv->q_id);
  mtos_free(p_priv);
  mtos_messageq_ack(q_id);
  mtos_task_exit();
}

static mt_u32 fdp_dataflow(fast_dvb_player_priv_t *p_priv, mt_u32 push_len, MT_BOOL fast)  //push_len: 0 for auto
{
  mt_u8 *p_data = NULL;
  phys_addr_t phydata = 0;
  mt_u32 get_data_len = 0;
  mt_u32 push_total_data_len = 0;
  mt_u32 push_data_len = 0;
  mt_u32 t = 0;

  get_data_len = fdp_fill_data(p_priv, &p_data,&phydata, push_len);
  t = mtos_ticks_get();
  if(get_data_len > 0)
  {
    //FDP_PRINTF("~~fdp->push start [%d], tick %d, es %d\n",
    //  get_data_len, t, fdp_dma_dbg_get_es_len());
  }

  while(push_total_data_len < get_data_len)
  {
    if(p_priv->is_need_break)
    {
      break; //for sonata split data
    }

	push_data_len = get_data_len;
    fdp_render_data(p_priv, p_data + push_total_data_len,phydata + push_total_data_len, push_data_len, fast);
    push_total_data_len += push_data_len;
  }

  t = mtos_ticks_get() - t;
  //if(t > 5)
  if((get_data_len > 0) && !p_priv->send_pg_done_evt)
  {
    //FDP_PERF_DBG(("~~fdp->push done len[%d], [%d]ms, tick %d, es %d, left es %d\n",
    //  get_data_len, t * 10, mtos_ticks_get(), fdp_dma_dbg_get_es_len(), fdp_get_left_ves_len()));
  }

  return get_data_len;
}

static void print_tpinfo(MT_UNF_fdp_tp_info_t *p_tp_info)
{
	switch(p_tp_info->conn_para.sig_type)
	{
		case MT_UNF_FE_SIG_TYPE_SAT:
		case MT_UNF_FE_SIG_TYPE_SAT_2:
	      FDP_PRINTF("dvbs tc lock tuner freq %d, sym %d\n",
	        p_tp_info->conn_para.connect_param.sat.freq,
	        p_tp_info->conn_para.connect_param.sat.sym_rate);
	      break;
		  case MT_UNF_FE_SIG_TYPE_CAB:
		   FDP_PRINTF("dvbc tc lock tuner freq %d, modulation %d, symbol_rate %d\n",
			 p_tp_info->conn_para.connect_param.cab.freq,
			 p_tp_info->conn_para.connect_param.cab.mod_type,
			 p_tp_info->conn_para.connect_param.cab.sym_rate);
		   break;
		 case MT_UNF_FE_SIG_TYPE_BUTT:
			 FDP_PRINTF("%s, Please check lock mode[%d]\n",__FUNCTION__, p_tp_info->conn_para.sig_type);
			 break;
		 default:
	      FDP_PRINTF("dvbt tc lock tuner freq %d, band_width %d, plp id[%d]n",
	        p_tp_info->conn_para.connect_param.ter.freq,
	        p_tp_info->conn_para.connect_param.ter.band_width,
	        p_tp_info->conn_para.connect_param.ter.plp_id);
	      break;
  }
}

static MT_BOOL fdp_check_same_tp(fast_dvb_player_priv_t *p_priv, MT_UNF_fdp_play_param_t *p_para)
{
  MT_UNF_fdp_tp_info_t *p_old = &(p_priv->play_para.tp_info);
  MT_UNF_fdp_tp_info_t *p_new = &(p_para->tp_info);
  MT_BOOL ret = FALSE;

  print_tpinfo(p_new);

  if(p_new->conn_para.sig_type!= p_old->conn_para.sig_type)
  {
    FDP_PRINTF("conn_para.sig_type  not same,[%d, %d]\n", p_new->conn_para.sig_type,p_old->conn_para.sig_type);
    return ret;
  }
  switch(p_new->conn_para.sig_type)
  {
    case MT_UNF_FE_SIG_TYPE_SAT:
	case MT_UNF_FE_SIG_TYPE_SAT_2:
      ret = (p_new->conn_para.connect_param.sat.freq== p_old->conn_para.connect_param.sat.freq)
        && (p_old->conn_para.connect_param.sat.sym_rate==
            p_new->conn_para.connect_param.sat.sym_rate);
      break;
    case MT_UNF_FE_SIG_TYPE_CAB:
      ret = (p_old->conn_para.connect_param.cab.freq == p_new->conn_para.connect_param.cab.freq)
        && (p_old->conn_para.connect_param.cab.sym_rate==
            p_new->conn_para.connect_param.cab.sym_rate)
        && (p_old->conn_para.connect_param.cab.mod_type==
            p_new->conn_para.connect_param.cab.mod_type);
      break;
   case MT_UNF_FE_SIG_TYPE_BUTT:
 	  FDP_PRINTF("%s, Please check sig_type[%d]\n",__FUNCTION__, p_new->conn_para.sig_type);
 	  break;
    default:
      ret = (p_old->conn_para.connect_param.ter.freq == p_new->conn_para.connect_param.ter.freq )
        && (p_old->conn_para.connect_param.ter.band_width==
            p_new->conn_para.connect_param.ter.band_width)
        && (p_old->conn_para.connect_param.ter.plp_id ==
            p_new->conn_para.connect_param.ter.plp_id);
      break;
  }
  if(!ret)
  {
    print_tpinfo(p_old);
  }
  return ret;
}

static MT_BOOL fdp_check_nim(fast_dvb_player_priv_t *p_priv)
{
  static mt_u32 s_tick = 0;
  static mt_u32 wait_to_relock_cnt = 0;
  static mt_u32 unlock_count = 0;
  mt_u32 tick = mtos_ticks_get();
  mt_u32 signal_quality;
  mt_u32 signal_strength;

  if(tick - s_tick > 50) //0.5s
  {
	mt_unf_fe_status_t fe_status;

	 mt_unf_fe_get_status(p_priv->play_para.tp_info.tuner_id, &fe_status);

	 mt_unf_fe_get_signal_quality(p_priv->play_para.tp_info.tuner_id, &signal_quality);
	 mt_unf_fe_get_signal_strength(p_priv->play_para.tp_info.tuner_id,&signal_strength);
	FDP_PRINTF("tuner_id = %d sig_type = %d  fe_status.lock_status  %d  signal_quality = %d signal_strength =%d\n",p_priv->play_para.tp_info.tuner_id,p_priv->play_para.tp_info.conn_para.sig_type,\
	fe_status.lock_status ,\
	signal_quality,signal_strength);

    if(fe_status.lock_status !=MT_UNF_FE_SIGNAL_LOCKED )
    {
      wait_to_relock_cnt++;
      unlock_count++;
      //FDP_PRINTF("fdp--->%s locked %d\n", __FUNCTION__, locked);
      if(wait_to_relock_cnt > 4)
      {
        fdp_lock_tuner(p_priv, &(p_priv->play_para));
		wait_to_relock_cnt = 0; //relock reset
      }
    }
    else
    {
      wait_to_relock_cnt = 0; //reset
      unlock_count = 0; //reset
    }

    s_tick = tick;
  }

  return (unlock_count < 2);
}

static MT_BOOL fdp_check_dec_state(fast_dvb_player_priv_t *p_priv)
{
  mt_u32 t = mtos_ticks_get() - p_priv->start_tick;
  mt_u32 time_out = p_priv->play_para.pg_info.is_scrambled ? 400 : 200;  // 4s and 2s

  if(t > time_out)
  {
    FDP_PRINTF("fdp--->%s timeout\n", __FUNCTION__);
    return TRUE;
  }
  return TRUE;
}

static void fdp_thread(void *p)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)p;
  os_msg_t m = {0};
  MT_BOOL is_same_tp = FALSE;
  MT_UNF_fdp_play_param_t * p_param = NULL;
  mt_set_pthread_name(__FUNCTION__);
  while(1)
  {
    if(mtos_messageq_receive(p_priv->q_id, &m, 10))
    {
      FDP_PRINTF("fdp--->%s cmd %d, sm %d, tick %d\n", __FUNCTION__, m.content, p_priv->sm, mtos_ticks_get());
      switch(m.content)
      {
        case FDP_CMD_SW:
          FDP_PERF_TEST(FDP_PERF_TEST_START_PLAY);

          p_param = (MT_UNF_fdp_play_param_t *)m.para1;
          is_same_tp = fdp_check_same_tp(p_priv, p_param);
          memcpy(&(p_priv->play_para), p_param, sizeof(MT_UNF_fdp_play_param_t));

          p_priv->send_lock_evt = FALSE;
          p_priv->send_pg_done_evt = FALSE;

		   p_priv->u32RecDmxId = p_param->tp_info.ts_in;

          #ifdef FDP_DUMP_FILE
          _dump_data_start(p_param->pg_info.v_pid);
          #endif

          if(is_same_tp)  //same tp
          {
            fdp_av_render_switch(p_priv, TRUE);
            fdp_av_render_reset(p_priv);
            if(p_priv->is_using_special_extern_demod && p_priv->sm == FDP_SM_WAIT_LOCK)
            {
              p_priv->sm = FDP_SM_WAIT_LOCK;
            }
            else
            {
              p_priv->sm = FDP_SM_MONITOR;
            }
          }
          else
          {
            fdp_lock_tuner(p_priv, p_param);
            //if(p_priv->rec_chan_id != 0xFFFF) //for the first play
			if(p_priv->hRecChn!= 0xFFFF) //for the first play
            {
              fdp_dataflow(p_priv, FOR_PLAY_SIZE, TRUE); //flush
            }
            fdp_stop_rec(p_priv,FALSE);
            if(!p_priv->is_using_special_extern_demod)
            {
              fdp_start_rec(p_priv);
              fdp_av_render_switch(p_priv, FALSE); //reset to render CA Table
              p_priv->sm = FDP_SM_PRE_LOAD;
            }
            else
            {
              p_priv->sm = FDP_SM_WAIT_LOCK;  //extern_demod(DM6060 ) patch
            }
          }
          p_priv->start_tick = mtos_ticks_get();
          mtos_free((void *)m.para1);
          if(p_priv->block_ir_key)
          {
           // uio_device_t *p_dev = dev_find_identifier(NULL, DEV_IDT_TYPE, SYS_DEV_TYPE_UIO);
          //  uio_clear_key(p_dev);
          }

          FDP_PERF_TEST(FDP_PERF_TEST_ACK_UI_S);
          mtos_messageq_ack(p_priv->q_id);
          FDP_PERF_TEST(FDP_PERF_TEST_ACK_UI_E);
          break;
        case FDP_CMD_STOP:
          fdp_stop_rec(p_priv,TRUE);
          fdp_stop_av(p_priv, !m.para1);
          memset(&(p_priv->play_para), 0, sizeof(MT_UNF_fdp_play_param_t));
          p_priv->sm = FDP_SM_IDLE;

          FDP_PERF_TEST(FDP_PERF_TEST_STOP_PLAY);
          mtos_messageq_ack(p_priv->q_id);
          break;
        case FDP_CMD_DESTROY:
          fdp_destroy(p_priv);
          break;
        default:
          break;
      }
      continue;
    }

    //FDP_PRINTF("sm %d\n", p_priv->sm);
    //sm
    switch(p_priv->sm)
    {
      case FDP_SM_IDLE:
        break;
      case FDP_SM_WAIT_LOCK:
        {
          mt_u32 locked = 0;
          mt_u32 wait_time = mtos_ticks_get() - p_priv->start_tick;
		  mt_unf_fe_status_t fe_status;

		  mt_unf_fe_get_status(p_priv->play_para.tp_info.tuner_id, &fe_status);
         // dev_io_ctrl(p_priv->play_para.tp_info.p_nim_dev, NIM_IOCTRL_CHECK_LOCK, (mt_u32)&locked);
		 FDP_PRINTF("fe_status.lock_status   %d   %d\n", fe_status.lock_status,wait_time);
          if((fe_status.lock_status ==MT_UNF_FE_SIGNAL_LOCKED ) ||(wait_time > 200))
          {
              fdp_start_rec(p_priv);
              p_priv->sm = FDP_SM_PRE_LOAD;
          }
        }
        break;
      case FDP_SM_PRE_LOAD:
        {
          mt_u32 left_es = fdp_get_left_ves_len();
          mt_u32 rec_time = mtos_ticks_get() - p_priv->start_tick;

          //FDP_PERF_DBG(("fdp--->wait left es[%d], rec[%d], start[%d], ts[%lld]\n",
          //  left_es, rec_time, p_priv->start_tick, fdp_get_dmx_data_size(p_priv)));
          if((left_es <= 30 * KBYTES) || (rec_time > 80))
          {
            fdp_av_render_switch(p_priv, FALSE);  //reset to render AV data
            fdp_av_render_reset(p_priv);
            p_priv->sm = FDP_SM_MONITOR;
            p_priv->start_tick = mtos_ticks_get();  //flush tick for waiting video decoder
          }
          else
          {
            if(p_priv->play_para.pg_info.is_scrambled)
            {
              fdp_dataflow(p_priv, FOR_PLAY_SIZE, FALSE); //render CA Table
            }
          }
        }
        break;
      case FDP_SM_MONITOR:
        if(fdp_check_nim(p_priv))
        {
          if(!p_priv->is_locked)
          {
            p_priv->is_locked = TRUE;
            p_priv->send_lock_evt = TRUE;
            p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_LOCKED, 0, 0);
          }
          else if(!p_priv->send_lock_evt)
          {
            p_priv->send_lock_evt = TRUE;
            p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_LOCKED, 0, 0);
          }

          fdp_dataflow(p_priv, FOR_PLAY_SIZE, FALSE);

          if(!p_priv->send_pg_done_evt)
          {
            if(fdp_check_dec_state(p_priv))
            {
              p_priv->send_pg_done_evt = TRUE;
              p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_SWITCH_PG_DONE, 1, 0);
            }
          }
        }
        else
        {
          if(p_priv->is_locked)
          {
            p_priv->is_locked = FALSE;
            p_priv->send_lock_evt = TRUE;
            p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_UNLOCKED, 0, 0);
          }
          else if(!p_priv->send_lock_evt)
          {
            p_priv->send_lock_evt = TRUE;
            p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_UNLOCKED, 0, 0);
          }
          if(!p_priv->send_pg_done_evt)
          {
            p_priv->send_pg_done_evt = TRUE;
            p_priv->cb(p_priv->user_handle, MT_UNF_FDP_EVENT_SWITCH_PG_DONE, 0, 0);
          }
        }
        break;
      default:
        break;
    }
  }
}

static void fdp_dummy_callback(mt_handle_t user_handle, MT_UNF_FTP_EVENT_E e, mt_u32 para1, mt_u32 para2)
{
}

void MT_UNF_fast_dvbplayer_switch(mt_handle_t h, MT_UNF_fdp_play_param_t *p_para)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
  os_msg_t m = {0};
  MT_UNF_fdp_play_param_t *p_play_para = mtos_malloc(sizeof(MT_UNF_fdp_play_param_t));

  mtos_mutex_take(p_priv->mutex);
  mtos_mutex_give(p_priv->mutex);

  FDP_PRINTF("fdp--->%s, vpid %d, tick %d\n", __FUNCTION__, p_para->pg_info.v_pid, mtos_ticks_get());

  memcpy(p_play_para, p_para, sizeof(MT_UNF_fdp_play_param_t));

  //p_priv->is_need_break         = TRUE;

  m.content = FDP_CMD_SW;
  m.is_sync = TRUE;
  m.para1 = (ulong)p_play_para;
  mtos_messageq_send(p_priv->q_id, &m);
  p_priv->is_wait_capture_table = TRUE;
  p_priv->is_wait_capture_done  = TRUE;
  p_priv->is_wait_set_cw        = TRUE;

  p_priv->is_need_break         = FALSE;

}

void MT_UNF_fast_dvbplayer_stop(mt_handle_t h, mt_u32 stop_mode)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
  os_msg_t m = {0};

  mtos_mutex_take(p_priv->mutex);
  mtos_mutex_give(p_priv->mutex);

  FDP_PRINTF("fdp--->%s tick %d\n", __FUNCTION__, mtos_ticks_get());

  m.content = FDP_CMD_STOP;
  m.is_sync = TRUE;
  m.para1 = stop_mode;
  mtos_messageq_send(p_priv->q_id, &m);
  p_priv->is_wait_capture_table = FALSE;
  p_priv->is_wait_capture_done  = FALSE;
  p_priv->is_wait_set_cw        = FALSE;
}

void MT_UNF_fast_dvbplayer_start_capture_ecm(mt_handle_t h)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
  os_msg_t m = {0};

  mtos_mutex_take(p_priv->mutex);
  if(p_priv->is_wait_capture_table)
  {
    FDP_PRINTF("fdp--->%s tick %d\n", __FUNCTION__, mtos_ticks_get());

    m.content = FDP_CMD_CAPTURE_TABLE;
    mtos_messageq_send(p_priv->q_id, &m);
    p_priv->is_wait_capture_table = FALSE;
  }
  mtos_mutex_give(p_priv->mutex);
}

void MT_UNF_fast_dvbplayer_finish_capture_ecm(mt_handle_t h)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
  os_msg_t m = {0};
  mtos_mutex_take(p_priv->mutex);
  if(p_priv->is_wait_capture_done)
  {
    FDP_PRINTF("fdp--->%s tick %d\n", __FUNCTION__, mtos_ticks_get());

    m.content = FDP_CMD_CAPTURE_DONE;
    mtos_messageq_send(p_priv->q_id, &m);
    p_priv->is_wait_capture_done = FALSE;
  }
  mtos_mutex_give(p_priv->mutex);
}

void MT_UNF_fast_dvbplayer_set_cw_done(mt_handle_t h)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
  os_msg_t m = {0};

  mtos_mutex_take(p_priv->mutex);
  if(p_priv->is_wait_set_cw)
  {
    FDP_PRINTF("fdp--->%s tick %d, cw %d\n", __FUNCTION__, mtos_ticks_get(), p_priv->is_wait_set_cw);
    m.content = FDP_CMD_SET_CW;
    mtos_messageq_send(p_priv->q_id, &m);
    p_priv->is_wait_set_cw = FALSE;
  }
  mtos_mutex_give(p_priv->mutex);
}

void MT_UNF_fast_dvbplayer_reg_callback(mt_handle_t h, fdp_callback f_cb)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;

  p_priv->cb = f_cb;
}

void MT_UNF_fast_dvbplayer_reg_set_cw_callback(mt_handle_t h, fdp_callback_set_cw f_cb)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;

  p_priv->cb_set_cw = f_cb;
}

void MT_UNF_fast_dvbplayer_get_avhandle(mt_handle_t h,MT_UNF_fdp_av_info_t* p_av)
{
	fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
	p_av->av_handle =p_priv->av_handle;
}

mt_handle_t MT_UNF_fast_dvbplayer_create(MT_UNF_fast_dvbplayer_init_para_t *p_para)
{
  fast_dvb_player_priv_t *p_priv = NULL;
  char *p_name = "fast_dvb_player";
  mt_u32 fifo_addr_align = 16;
  mt_u32 fifo_size_align = DMX_UNIT_SIZE * 2;
 // mt_u32 align = 0;
  mt_s32 Ret = SUCCESS;

fifo_addr_align = 4 * KBYTES;
  p_priv = (fast_dvb_player_priv_t *)mtos_malloc(sizeof(fast_dvb_player_priv_t));

  if(p_priv == NULL)
  {
    //no mem
    return NULL;
  }
  memset(p_priv, 0, sizeof(fast_dvb_player_priv_t));

  p_priv->fifo_size = ((p_para->fifo_size ) / fifo_size_align) * fifo_size_align;
  FDP_PRINTF("fdp--->%s buffer addr 0x%x size %d  \n", __FUNCTION__, p_priv->p_fifo, p_priv->fifo_size);

  p_priv->Win=p_para->Win;
  p_priv->hTrack=p_para->hTrack;
  p_priv->enPortId = p_para->enPortId;
  FDP_PRINTF(" p_priv->enPortId = %x\n", p_priv->enPortId);

 if(p_para->hAvHandle)
 {
	 p_priv->av_handle=p_para->hAvHandle;
	 p_priv->is_avhandle_byuser = TRUE;
 }

  fast_dvb_player_av_init(MT_UNF_AVPLAY_STREAM_TYPE_TS,p_para->u32PlayDmxId,p_priv);

  p_priv->u32PlayerDmxId =p_para->u32PlayDmxId;//g_fast_av_priv.avAttr.mt_u32DemuxId;
  p_priv->u32RecDmxId =0;//p_para->u32RecDmxId;

  p_priv->q_id = mtos_messageq_create(32, (mt_u8 *)p_name);
  p_priv->prio = p_para->prio;
  p_priv->user_handle = p_para->user_handle;
  p_priv->block_ir_key = p_para->block_ir_key;
  p_priv->is_using_special_extern_demod = p_para->special_demod;
  p_priv->mutex = mtos_mutex_create(1);
  if(!mtos_task_create((mt_u8 *)p_name, fdp_thread, (void *)p_priv,
     p_priv->prio, p_priv->stack, sizeof(p_priv->stack)))
  {
    mtos_mutex_delete(p_priv->mutex); //clear resource
    mtos_messageq_release(p_priv->q_id);
    mtos_free(p_priv);
    FDP_PRINTF("fdp--->%s create task[%d] fail\n", __FUNCTION__, p_priv->prio);
    return NULL;
  }
  p_priv->hRecChn = 0xFFFF;

  p_priv->cb = fdp_dummy_callback;


  return p_priv;
}

void MT_UNF_fast_dvbplayer_destroy(mt_handle_t h)
{
  fast_dvb_player_priv_t *p_priv = (fast_dvb_player_priv_t *)h;
  os_msg_t m = {0};
  mt_u32 q_id = p_priv->q_id;

  FDP_PRINTF("fdp--->%s\n", __FUNCTION__);

  m.content = FDP_CMD_DESTROY;
  m.is_sync = TRUE;
  mtos_messageq_send(p_priv->q_id, &m);
  mtos_messageq_release(q_id);

}

mt_u8* MT_UNF_fast_dvbplayer_get_ver(void)
{
	return "fastplay_v1.0";
}

