/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_UNF_FAST_DVBPLAYER_H_
#define __MT_UNF_FAST_DVBPLAYER_H_

/*!
  fast player event define
  */
typedef enum
{
  /*!
    Start Tag
    */
  MT_UNF_FDP_EVENT_START,
  /*!
    finish switch
    */
  MT_UNF_FDP_EVENT_SWITCH_PG_START,
  /*!
    switch diseqc
    */
  MT_UNF_FDP_EVENT_SWITCH_DISEQC,
  /*!
    finish switch
    */
  MT_UNF_FDP_EVENT_SWITCH_PG_DONE,
  /*!
    locked
    */
  MT_UNF_FDP_EVENT_LOCKED,
  /*!
    unlocked
    */
  MT_UNF_FDP_EVENT_UNLOCKED,
  /*!
    End Tag
    */
  MT_UNF_FDP_EVENT_END
}MT_UNF_FTP_EVENT_E;

/*!
  fast player callback function define
  */
typedef void (*fdp_callback)(mt_handle_t user_handle, MT_UNF_FTP_EVENT_E e, mt_u32 para1, mt_u32 para2);

/*!
  fast player callback function define
  */
typedef MT_BOOL (*fdp_callback_set_cw)(mt_u32 freq, mt_u16 s_id);


/*!
  tp info
  */
typedef struct
{
	/*!
	  tunner id 
	  */	
	mt_u32 tuner_id;
	/*!
	  tunner id 
	  */	
	mt_u32 ts_in;
	/*!
	  connect parameter 
	  */	
	mt_unf_fe_connect_para_t conn_para;
	/*!
	  connect timeout 
	  */	
	mt_u32 timeout;  
}MT_UNF_fdp_tp_info_t;



/*!
  av info
  */
typedef struct
{
  /*!
    av handle
    */	
  mt_handle av_handle;
}MT_UNF_fdp_av_info_t;


/*!
  program info
  */
typedef struct
{
  /*!
    Service ID(DVB concept, program number)
    */
  mt_u32 s_id;
  /*!
    Video pid
    */
  mt_u32 v_pid;
  /*!
    Audio pid
    */
  mt_u32 a_pid;
  /*!
    PCR pid
    */
  mt_u32 pcr_pid;
  /*!
    PMT pid
    */
  mt_u32 pmt_pid;
  #if 1
  /*!
    Audio track
    */
  MT_UNF_TRACK_MODE_E audio_track;
  /*!
    Audio volumn;
    */
  mt_u16 audio_volume;
  #endif
  /*!
    Audio type
    */
  HA_CODEC_ID_E audio_type;
  /*!
    scrambled?
    */
  MT_BOOL is_scrambled;
  /*!
    Video type
    */
  MT_UNF_VCODEC_TYPE_E video_type;
}MT_UNF_fdp_program_info_t;

/*!
  Parameter for fast dvb player
  */
typedef struct
{
  /*!
    tp info
    */
  MT_UNF_fdp_tp_info_t tp_info;
  
  /*!
    program info
    */
  MT_UNF_fdp_program_info_t pg_info;

  /*!
    start mode
    */
  //vdec_start_mode_t start_mode;

  /*!
    stop mode FREEZE = 0, BLACK = 1
    */
  mt_u32 stop_mode;
}MT_UNF_fdp_play_param_t;

/*!
  fast player init parameter define
  */
typedef struct mt_tag_fast_dvb_player_init_para
{
  /*!
    player fifo size [2M ~ 20M]
    */
  mt_u32 fifo_size;
  /*!
    player task priority
    */
  mt_u32 prio;
  /*!
    player user handle
    */
  mt_handle_t user_handle;
  /*!
    continue_ir_key: FALSE (default), TRUE clear ir after switch channel
    */
  MT_BOOL block_ir_key;
  /*!
   rec dmx id
    */  
 // mt_u32 u32RecDmxId;
  /*!
   Play dmx id
    */  
  mt_u32 u32PlayDmxId;
  /*!
   Play dmx port
    */  
  MT_UNF_DMX_PORT_E enPortId;
  /*!
  Win
    */  
  mt_u32 Win;
  /*!
  SND Track
    */  
  MT_HANDLE hTrack;
  /*!
  AV Handle
    */  
  MT_HANDLE hAvHandle;
  
  /*!
    special_demod: FALSE (default), TRUE using special extern demod
    */
  MT_BOOL special_demod;
}MT_UNF_fast_dvbplayer_init_para_t;

/*!
  play/switch program.

  \param[in] h fast player handle
  \param[in] p_para play parameter
  */
void MT_UNF_fast_dvbplayer_switch(mt_handle_t h, MT_UNF_fdp_play_param_t *p_para);

/*!
  stop play program.

  \param[in] h fast player handle
  \param[in] stop mode FREEZE = 0, BLACK = 1
  */
void MT_UNF_fast_dvbplayer_stop(mt_handle_t h, mt_u32 stop_mode);

/*!
  Call this method when CA module ready to capture ecm

  \param[in] h fast player handle
  */
void MT_UNF_fast_dvbplayer_start_capture_ecm(mt_handle_t h);

/*!
  Call this method when CA module captured ecm

  \param[in] h fast player handle
  */
void MT_UNF_fast_dvbplayer_finish_capture_ecm(mt_handle_t h);

/*!
  Call this method when parse done cw and set it to demux

  \param[in] h fast player handle
  */
void MT_UNF_fast_dvbplayer_set_cw_done(mt_handle_t h);

/*!
  registe callback function

  \param[in] h fast player handle
  \param[in] f_cb callback function
  */
void MT_UNF_fast_dvbplayer_reg_callback(mt_handle_t h, fdp_callback f_cb);

/*!
  registe callback function

  \param[in] h fast player handle
  \param[in] f_cb callback function
  */
void MT_UNF_fast_dvbplayer_reg_set_cw_callback(mt_handle_t h, fdp_callback_set_cw f_cb);
/*!
   fast player get av handle.

  \param[in] h fast player handle
  */
void MT_UNF_fast_dvbplayer_get_avhandle(mt_handle_t h,MT_UNF_fdp_av_info_t* p_av);


/*!
  create fast player.

  \param[in] p_para parameter
  \return Return player instance handle
  */
mt_handle_t MT_UNF_fast_dvbplayer_create(MT_UNF_fast_dvbplayer_init_para_t *p_para);

/*!
  destroy fast player module.

  \param[in] h fast player handle
  */
void MT_UNF_fast_dvbplayer_destroy(mt_handle_t h);

/*!
  show fast player version.
  \return Return player version
  */
mt_u8* MT_UNF_fast_dvbplayer_get_ver(void);


#endif // End for __MT_UNF_FAST_DVBPLAYER_H_

