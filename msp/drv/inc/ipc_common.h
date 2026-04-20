/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

/*!
  The width of full screen for video layer (in pixels).
  */
#define DISP_FULLSCR_WIDTH            720
/*!
  The height of full screen for video layer in PAL standard (in pixels).
  */
#define DISP_FULLSCR_HEIGHT_PAL       576

/*!
  The max number of avsync cmd in AVSYNC_CMD_VSB_t
  */
#define VDEC_AVSYNC_MAX_CMD_NUMB  (10)
/*!
  The supported vdec fw cmd
  */
typedef enum
{
  /*!
    VDEC FW START
    NOTE start from 0x100 cause other process on av cpu will share cmd_id
    */
  VDEC_FW_CMD_START = 0x1,
  /*!
    STOP
    */
  VDEC_FW_CMD_STOP,
  /*!
    PAUSE
    */
  VDEC_FW_CMD_PAUSE,
  /*!
    RESUME
    */
  VDEC_FW_CMD_RESUME,
  /*!
    FREEZE
    */
  VDEC_FW_CMD_FREEZE,
  /*!
    FREEZE
    */
  VDEC_FW_CMD_SWITCH_CH,
  /*!
    TRICK
    */
  VDEC_FW_CMD_TRICK,
  /*!
    DEC FRAME
    */
  VDEC_FW_CMD_DECFRAME,
  //add by HY for autotest 2012-3-28 begin
  /*!
    GET INFO
    */
  VDEC_FW_CMD_GET_INFO,
  //add by HY for autotest 2012-3-28 end
  /*!
    SET INFO
    */
  VDEC_FW_CMD_SET_INFO,
  /*!
    CLEAN BUFFER
    */
  VDEC_FW_CMD_CLEAN_BUFFER,
    /*!
    SET AVSYNC
    */
  VDEC_FW_CMD_SET_AVSYNC,
    /*!
   SET FILE PLAYBACK FLAG
  */
  VDEC_FW_CMD_SET_AVSYNC_MODE,
  /*!
   SET FILE PLAYBACK FLAG
  */
  VDEC_FW_CMD_SET_FILE_PLAYBACK_MODE,
  /*!
   SET FILE PLAYBACK FRAME RATE
  */
  VDEC_FW_CMD_SET_FRAMERATE,
  /*!
   SET VIDEO BUF POLICY
  */
  VDEC_FW_CMD_SET_BUF_POLICY,
  /*!
   SET VIDEO INFO BUF ADDR
  */
  VDEC_FW_CMD_SET_INFO_ADDR,
  /*!
   SET VIDEO DETEC DISPLAY ORDER
  */
  VDEC_FW_CMD_DETEC_DISPLAY_ORDER,
  /*!
   RUN FUNCTION
  */
  VDEC_FW_CMD_RUN_FUNCTION,
  /*!
   AVS DETEC PTS REPEAT FUNCTION
  */
  VDEC_FW_CMD_AVS_DETEC_PTS_REPEAT,
  /*!
   FORCE INPUT FRAME RATE FUNCTION
  */
  VDEC_FW_CMD_FORCE_INPUT_FRAME_RATE,
  /*!
  avsync cmd start
  */
  VDEC_FW_DO_AVSYNC_CMD_BASE,
  /*!
   avsync cmd ends
  */
  VDEC_FW_DO_AVSYNC_CMD_END = (VDEC_FW_DO_AVSYNC_CMD_BASE + VDEC_AVSYNC_MAX_CMD_NUMB),
  /*!
  added by jqw for send OTP info to av cpu
  */
  VDEC_FW_CMD_SEND_OTP_INFO,
  /*!
  added by jqw set shared memory addr
  this memory is used by ap and av cpu
  */
  VDEC_SET_SHARE_MEM,
  /*!
  added by feliu set av pts diff when some
  stream which av pts has constant difference
  */
  VDEC_SET_SYNC_DIFF,
  /*!
  add for setting freeze memory addr
  this memory is used by ap and av cpu
  */
  VDEC_SET_FREEZE_MEM,
   /*!
  Enter spical mode that display ASAP such as WASU cloud menu
  */
  VDEC_SET_DISPLAY_ASAP,
  /*!
   SET VIDEO FORCE BOT FIRST
  */
  VDEC_FW_CMD_FORCE_BOT_FIRST,

  /*!
  VDEC_FW_ERROR_DEAL
  */
  VDEC_FW_ERROR_DEAL,

  /*!
   avsync cfg
  */
  VDEC_FW_DO_AVSYNC_CFG,
  /*!
   error deal cfg
  */
  VDEC_FW_ERROR_DEAL_CFG,
  /*!
   Freeze support
   */
  VDEC_FREEZE_SUPPORT,
  /*!
   I frame only
  */
  VDEC_I_FRAME_ONLY,
/*!
   AV USB LOG
  */
  VDEC_SET_SHARE_MEM_USBLOG,
  /*!
   SET VIDEO MEM CPY ADDR
  */
  VDEC_FW_CMD_SET_MEM_CPY_INFO_ADDR,
  /*!
  this memory is used by ap and av cpu
  */
  VDEC_MEM_CPY,
  /*!
  set memcpy size
  */	
  VDEC_SET_MEM_CPY_SIZE,
  /*!
  VDEC FW INIT
  */	
  VDEC_FW_CMD_INIT,
  /*!
  VDEC FW CONTROL CMD
  */	
  VDEC_FW_CMD_CONTROL,
  /*!
  VDEC FW EXIT
  */	
  VDEC_FW_CMD_EXIT,
  
}vdec_fw_cmd_t;

/*!
  The supported aud fw cmd
  */
typedef enum
{
  /*!
    AUD FW ATTACH
    */
  AUD_FW_CMD_ATTACH,

/*!
    AUD_FW_CMD_SECURE_INFO
    */
  AUD_FW_CMD_SECURE_INFO,

  /*!
    AUD FW QUERY BUFFER
    */
  AUD_FW_CMD_QUERY_BUFFER,
  /*!
    AUD FW QUERY BUFFER CALLBACK
    */
  AUD_FW_CMD_QUERY_BUFFER_CB,
  /*!
    AUD FW INIT
    */
  AUD_FW_CMD_INIT,
  /*!
    AUD FW START
    */
  AUD_FW_CMD_START,
  /*!
    STOP
    */
  AUD_FW_CMD_STOP,
  /*!
    PAUSE
    */
  AUD_FW_CMD_PAUSE,
  /*!
    RESUME
    */
  AUD_FW_CMD_RESUME,
  /*!
    MUTE
    */
  AUD_FW_CMD_MUTE,
  /*!
    DEBUG IRQ
    */
  AUD_FW_CMD_DEBUG_IRQ,
  /*!
    SET PCM PARAM
    */
  AUD_FW_CMD_SET_PCM_PARAM,
  /*!
    DEINIT
    */
  AUD_FW_CMD_DEINIT,
   /*!
    DEINIT CALLBACK
    */
  AUD_FW_CMD_DEINIT_CB,
 /*!
    set output format
    */
  AUD_FW_CMD_SET_OUTPUT_FORMAT,
  /*!
    set output format
    */
  AUD_FW_CMD_SET_AUD_INFO,
  /*!
    chimera cmd
    */
  AUD_FW_CMD_CHIMERA,
  /*!
    output fmt setting cmd
    */
  AUD_FW_CMD_OUTPUT,
  /*!
    AUD FW DETACH
    */
  AUD_FW_CMD_DETACH,
  /*!
    AUD FW SLEEP
    */
  AUD_FW_CMD_SLEEP,
  /*!
    AUD FW AWAKE,
    */
  AUD_FW_CMD_AWAKE,
  /*!
    AUD FW itaf args
    */
  AUD_FW_CMD_ITAF_ARGS,
  /*!
    AUD FW itaf get udc hdl
    */
  AUD_FW_CMD_ITAF_GET_UDC_HDL,
  /*!
    AUD FW itaf invalid hdl
    */
  AUD_FW_CMD_ITAF_INVALID_HDL,
  /*!
    AUD FW itaf set udc ret hdl
    */
  AUD_FW_CMD_ITAF_SET_UDC_RET_HDL,
  /*!
    AUD FW itaf get output files
    */
  AUD_FW_CMD_ITAF_GET_OUTPUT_FILES,
  /*!
    AUD FW itaf open file
    */
  AUD_FW_CMD_ITAF_OPEN_FILE,
  /*!
    AUD FW itaf write file
    */
  AUD_FW_CMD_ITAF_WRITE_FILE,
  /*!
    AUD FW itaf close file
    */
  AUD_FW_CMD_ITAF_CLOSE_FILE,
 /*!
    AUD FW itaf args
    */
  AUD_FW_CMD_PLAY_ARGS,
  /*!
    avsync mirc adjust
    */
  AUD_CFG_MICRO_AVSYNC,

   /*!
    AUD FW dolby decoding
    */
  AUD_FW_DOLBY_DIGITA_ON,
  /*!
    AUD FW dolby plus decoding
    */
  AUD_FW_DOLBY_PLUS_ON,
  /*!
    AUD FW dolby dual mono stream on
    */
  AUD_FW_DOLBY_DUAL_ON,
 /*!
    AUD FW dolby dual mono stream off
    */
  AUD_FW_DOLBY_DUAL_OFF,
 /*!
   AUD FW dolby off
    */
  AUD_FW_DOLBY_STREAM_OFF,
 /*!
   AUD descrip
    */
  AUD_DESCRIP_PARAM_SET,
  /*!
   AUD descrip
    */
  AUD_DESCRIP_VOLUME_SET,
  /*!
   AUD descrip
    */
  AUD_DESCRIP_VOLFLAG_SET,
    /*!
    AUD_FW_SPDIF_MODE config
    */
  AUD_FW_SPDIF_MODE_CFG,
  /*!
    AUD_FW_DEBUG_BUFFER cmd
    */
  AUD_FW_DEBUG_BUFFER,
    /*!
    AUD_FW_CMD_AVC config
    */
  AUD_FW_CMD_AVC_CFG,

  /*!
    AUD_FW_SYSDATE_SET 
    */
  AUD_FW_SYSDATE_SET,
  /*!
    AUD_FW_SYSDATE_GET 
    */
  AUD_FW_SYSDATE_GET,
}aud_fw_cmd_t;

/*!
//defines for SRC&DST_ID On AP&AV CPU
  */
#define CPU_BASE_ID_AP 0x10000000

/*!
//defines for SRC&DST_ID On AP&AV CPU
  */
#define CPU_BASE_ID_AV 0x20000000

/*!
//AP cpu add task for av
  */
#define IPC_MSG_ADD_AV_TASK  ((0x1 << 16) + \
                                   SYS_DEV_TYPE_AP_AV_FAKE_DEV)

/*!
//AV cpu add task for av
  */
#define IPC_MSG_REMOVE_AV_TASK  ((0x2 << 16) + \
                                   SYS_DEV_TYPE_AP_AV_FAKE_DEV)

/*!
//AV cpu suspend task for av
  */
#define IPC_MSG_SUSPEND_AV_TASK  ((0x3 << 16) + \
                                   SYS_DEV_TYPE_AP_AV_FAKE_DEV)
/*!
//AV cpu suspend task for av
  */
#define IPC_MSG_DEBUG_AV_TASK  ((0x3 << 16) + \
                                   SYS_DEV_TYPE_AP_AV_FAKE_DEV)


/*!
//example for SRC&DST_ID
//src_id of vdec dev on ap cpu is
  */
#define AP_SYS_DEV_VDEC (CPU_BASE_ID_AP + SYS_DEV_TYPE_VDEC_VSB)
/*!
//example for SRC&DST_ID
//dst_id of vdec dev on av cpu is
  */
#define AV_SYS_DEV_VDEC (CPU_BASE_ID_AV +  SYS_DEV_TYPE_VDEC_VSB)

/*!
//example for SRC&DST_ID
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_START ((VDEC_FW_CMD_START << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_STOP ((VDEC_FW_CMD_STOP << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_PAUSE ((VDEC_FW_CMD_PAUSE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_RESUME ((VDEC_FW_CMD_RESUME << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_FREEZE ((VDEC_FW_CMD_FREEZE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_SWITCH_CH ((VDEC_FW_CMD_SWITCH_CH << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu isVDEC_FW_ERROR_DEAL
  */
#define IPC_MSG_VDEC_FW_CMD_ERROR_DEAL ((VDEC_FW_ERROR_DEAL << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_FAST_SWITCH ((VDEC_FW_CMD_FAST_SWITCH << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_TRICK ((VDEC_FW_CMD_TRICK << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
//add by HY for autotest 2012-3-28 begin
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_GET_INFO ((VDEC_FW_CMD_GET_INFO << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
//add by HY for autotest 2012-3-28 end
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_SET_INFO ((VDEC_FW_CMD_SET_INFO << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_DECFRAME ((VDEC_FW_CMD_DECFRAME << 16) \
                                        + SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_CLEAN_BUFFER ((VDEC_FW_CMD_CLEAN_BUFFER << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_SET_AVSYNC ((VDEC_FW_CMD_SET_AVSYNC << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_SET_AVSYNC_MODE  ((VDEC_FW_CMD_SET_AVSYNC_MODE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_SET_PLAYBACK_MODE  ((VDEC_FW_CMD_SET_FILE_PLAYBACK_MODE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_SET_PLAYBACK_FRAMERATE  ((VDEC_FW_CMD_SET_FRAMERATE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//set vdec buf policy
  */
#define IPC_MSG_VDEC_FW_CMD_SET_BUF_POLICY  ((VDEC_FW_CMD_SET_BUF_POLICY << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//set vdec info buf addr
  */
#define IPC_MSG_VDEC_FW_CMD_SET_INFO_ADDR  ((VDEC_FW_CMD_SET_INFO_ADDR << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)


/*!
//set vdec mem cpy info buf addr
  */
#define IPC_MSG_VDEC_FW_CMD_SET_MEM_CPY_INFO_ADDR  ((VDEC_FW_CMD_SET_MEM_CPY_INFO_ADDR << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//Detec display order
  */
#define IPC_MSG_VDEC_DETEC_DISPLAY_ORDER  ((VDEC_FW_CMD_DETEC_DISPLAY_ORDER << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//AVS detec pts repeat
  */
#define IPC_MSG_VDEC_AVS_DETEC_PTS_REPEAT  ((VDEC_FW_CMD_AVS_DETEC_PTS_REPEAT << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//Force input frame rate
  */
#define IPC_MSG_VDEC_FORCE_INPUT_FRAME_RATE  ((VDEC_FW_CMD_FORCE_INPUT_FRAME_RATE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
    AVSYNC_PAUSE_SYNC_CMD
  */
#define IPC_MSG_AVSYNC_PAUSE_SYNC_CMD   (((VDEC_FW_DO_AVSYNC_CMD_BASE + AVSYNC_PAUSE_SYNC_CMD) << 16) + SYS_DEV_TYPE_VDEC_VSB)

/*!
    AVSYNC_NO_PAUSE_SYNC_CMD
  */
#define IPC_MSG_AVSYNC_NO_PAUSE_SYNC_CMD  (((VDEC_FW_DO_AVSYNC_CMD_BASE + AVSYNC_NO_PAUSE_SYNC_CMD) << 16) + SYS_DEV_TYPE_VDEC_VSB)

/*!
    AVSYNC_NO_ADJUST_CMD
  */
#define IPC_MSG_AVSYNC_NO_ADJUST_CMD          (((VDEC_FW_DO_AVSYNC_CMD_BASE + AVSYNC_NO_ADJUST_CMD) << 16) + SYS_DEV_TYPE_VDEC_VSB)

/*!
    AVSYNC_RESUME_CMD
  */
#define IPC_MSG_AVSYNC_RESUME_CMD               (((VDEC_FW_DO_AVSYNC_CMD_BASE + AVSYNC_RESUME_CMD) << 16) + SYS_DEV_TYPE_VDEC_VSB)
/*!
//video run function
  */
#define IPC_MSG_VDEC_RUN_FUNCTION  ((VDEC_FW_CMD_RUN_FUNCTION << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//send otp info to fw
  */
#define IPC_MSG_VDEC_FW_CMD_SEND_OTP_INFO ((VDEC_FW_CMD_SEND_OTP_INFO << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//send otp info to fw
  */
#define IPC_MSG_VDEC_SET_SHARE_MEM ((VDEC_SET_SHARE_MEM << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//send otp info to fw
  */
#define IPC_MSG_VDEC_MEM_CPY ((VDEC_MEM_CPY << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//set mem cpy size
  */
#define IPC_MSG_VDEC_SET_MEM_CPY_SIZE ((VDEC_SET_MEM_CPY_SIZE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//send otp info to fw
  */
#define IPC_MSG_VDEC_SET_SYNC_DIFF ((VDEC_SET_SYNC_DIFF << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//send otp info to fw
  */
#define IPC_MSG_VDEC_SET_FREEZE_MEM ((VDEC_SET_FREEZE_MEM << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//send otp info to fw
  */
#define IPC_MSG_VDEC_DISPLAY_ASAP ((VDEC_SET_DISPLAY_ASAP << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//force bot first
  */
#define IPC_MSG_VDEC_FORCE_BOT_FIRST  ((VDEC_FW_CMD_FORCE_BOT_FIRST << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
    AVSYNC CFG
  */
#define IPC_MSG_VDEC_FW_DO_AVSYNC_CFG    ((VDEC_FW_DO_AVSYNC_CFG << 16) + SYS_DEV_TYPE_VDEC_VSB)

/*!
//error deal config
  */
#define IPC_MSG_VDEC_FW_CMD_ERROR_DEAL_CFG ((VDEC_FW_ERROR_DEAL_CFG << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//freeze support
  */
#define IPC_MSG_VDEC_FREEZE_SUPPORT ((VDEC_FREEZE_SUPPORT << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//I frame only
  */
#define IPC_MSG_VDEC_I_FRAME_ONLY ((VDEC_I_FRAME_ONLY << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_INIT ((VDEC_FW_CMD_INIT << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_CONTROL ((VDEC_FW_CMD_CONTROL << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_EXIT ((VDEC_FW_CMD_EXIT << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_MSG_DEPTH 8

/*!
//example for SRC&DST_ID
//src_id of aud dev on ap cpu is
  */
#define AP_SYS_DEV_AUD (CPU_BASE_ID_AP + SYS_DEV_TYPE_AUDIO)
/*!
//example for SRC&DST_ID
//src_id of aud dev on ap cpu is
  */
#define AV_SYS_DEV_AUD (CPU_BASE_ID_AV + SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_ATTACH        ((AUD_FW_CMD_ATTACH << 16) + \
                                              SYS_DEV_TYPE_AUDIO)
/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_QUERY_BUFFER  ((AUD_FW_CMD_QUERY_BUFFER << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_SECURE_INFO  ((AUD_FW_CMD_SECURE_INFO << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_QUERY_BUFFER_CB ((AUD_FW_CMD_QUERY_BUFFER_CB << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_INIT  ((AUD_FW_CMD_INIT << 16) + \
                                   SYS_DEV_TYPE_AUDIO)
/*!
//config spdif mode
  */
#define IPC_MSG_AUD_FW_CMD_SPDMODE ((AUD_FW_SPDIF_MODE_CFG << 16) + \
                                   SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_START ((AUD_FW_CMD_START << 16) + \
                                    SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_STOP ((AUD_FW_CMD_STOP << 16) + \
                                          SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_PAUSE ((AUD_FW_CMD_PAUSE << 16) + \
                                        SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_RESUME ((AUD_FW_CMD_RESUME << 16) + \
                                      SYS_DEV_TYPE_AUDIO)


/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_MUTE ((AUD_FW_CMD_MUTE << 16) + \
                                      SYS_DEV_TYPE_AUDIO)


/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_DEBUG_IRQ ((AUD_FW_CMD_DEBUG_IRQ << 16) + \
                                      SYS_DEV_TYPE_AUDIO)
/*!
//aud description
  */
#define IPC_MSG_AUD_FW_CMD_SET_DESCRIP_PARAM ((AUD_DESCRIP_PARAM_SET << 16) + \
                                      SYS_DEV_TYPE_AUDIO)
/*!
//aud description
  */
#define IPC_MSG_AUD_FW_CMD_SET_DESCRIP_VOLFLAG ((AUD_DESCRIP_VOLFLAG_SET << 16) + \
                                      SYS_DEV_TYPE_AUDIO)
/*!
//for aud description
  */
#define IPC_MSG_AUD_FW_CMD_SET_DESCRIP_VOLUME ((AUD_DESCRIP_VOLUME_SET << 16) + \
                                      SYS_DEV_TYPE_AUDIO)
/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_SET_PCM_PARAM ((AUD_FW_CMD_SET_PCM_PARAM << 16) + \
                                      SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_DEINIT ((AUD_FW_CMD_DEINIT << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_DEINIT_CB ((AUD_FW_CMD_DEINIT_CB << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_SET_AUD_INFO ((AUD_FW_CMD_SET_AUD_INFO << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_DETACH        ((AUD_FW_CMD_DETACH << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
//dst_id of aud cpu sleep
  */
#define IPC_MSG_AUD_FW_CMD_SLEEP        ((AUD_FW_CMD_SLEEP << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
//dst_id of aud cpu awake
  */
#define IPC_MSG_AUD_FW_CMD_AWAKE        ((AUD_FW_CMD_AWAKE << 16) + \
                                          SYS_DEV_TYPE_AUDIO)

/*!
//dst_id to debug buffer of audio
  */
#define IPC_MSG_AUD_FW_DEBUG_BUFFER        ((AUD_FW_DEBUG_BUFFER << 16) + \
                                          SYS_DEV_TYPE_AUDIO)

/*!
  chimera cmd
  */
#define IPC_MSG_AUD_FW_CMD_CHIMERA       ((AUD_FW_CMD_CHIMERA << 16) + \
                                          SYS_DEV_TYPE_AUDIO)

/*!
  output fmt setting cmd
  */
#define IPC_MSG_AUD_FW_CMD_OUTPUT       ((AUD_FW_CMD_OUTPUT << 16) + \
                                          SYS_DEV_TYPE_AUDIO)




/*!
//dst_id of aud dev on av cpu is
  */
#define IPC_MSG_AUD_FW_CMD_SET_PCM_PARAM ((AUD_FW_CMD_SET_PCM_PARAM << 16) + \
                                      SYS_DEV_TYPE_AUDIO)
/*!
  play args
  */
#define IPC_MSG_AUD_FW_CMD_PLAY_ARGS   ((AUD_FW_CMD_PLAY_ARGS << 16) + \
                                          SYS_DEV_TYPE_AUDIO)

/*!
  MICROAVSYNC
  */
#define IPC_MSG_AUD_FW_MICROAVSYNC     ((AUD_CFG_MICRO_AVSYNC << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf args
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_ARGS       ((AUD_FW_CMD_ITAF_ARGS << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf get udc hld
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_GET_UDC_HDL       ((AUD_FW_CMD_ITAF_GET_UDC_HDL << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf invalide udc hld
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_INVALID_HDL       ((AUD_FW_CMD_ITAF_INVALID_HDL << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf set udc ret hdl
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_SET_UDC_RET_HDL       ((AUD_FW_CMD_ITAF_SET_UDC_RET_HDL << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf get udc output files
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_GET_OUTPUTFILES       ((AUD_FW_CMD_ITAF_GET_OUTPUT_FILES << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf open file
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_OPEN_FILE       ((AUD_FW_CMD_ITAF_OPEN_FILE << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf write file
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_WRITE_FILE       ((AUD_FW_CMD_ITAF_WRITE_FILE << 16) + \
                                          SYS_DEV_TYPE_AUDIO)
/*!
  itaf close file
  */
#define IPC_MSG_AUD_FW_CMD_ITAF_CLOSE_FILE       ((AUD_FW_CMD_ITAF_CLOSE_FILE << 16) + \
                                          SYS_DEV_TYPE_AUDIO)

/*!
  dolby msg
  */
#define IPC_MSG_DOLBY_DIGITA_ON  ((AUD_FW_DOLBY_DIGITA_ON << 16) + \
                                  SYS_DEV_TYPE_AUDIO)
/*!
  dolby msg
  */
#define IPC_MSG_DOLBY_PLUS_ON    ((AUD_FW_DOLBY_PLUS_ON << 16) + \
                                 SYS_DEV_TYPE_AUDIO)
/*!
  dolby msg
  */
#define IPC_MSG_DOLBY_DUAL_ON   ((AUD_FW_DOLBY_DUAL_ON << 16) + \
                                  SYS_DEV_TYPE_AUDIO)
/*!
  dolby msg
  */
#define IPC_MSG_DOLBY_DUAL_OFF   ((AUD_FW_DOLBY_DUAL_OFF << 16) + \
                                 SYS_DEV_TYPE_AUDIO)
/*!
  dolby msg
  */
#define IPC_MSG_DOLBY_STREAM_OFF  ((AUD_FW_DOLBY_STREAM_OFF << 16) + \
                                  SYS_DEV_TYPE_AUDIO)

/*!
//avc config msg
  */
#define IPC_MSG_AUD_FW_CMD_AVC_CFG ((AUD_FW_CMD_AVC_CFG << 16) + \
                                    SYS_DEV_TYPE_AUDIO)

/*!
// for avcpu sysdate sync on apcpu
  */
#define IPC_MSG_AUD_FW_SYSDATE_SET ((AUD_FW_SYSDATE_SET << 16) + \
                                            SYS_DEV_TYPE_AUDIO)
                                            
/*!
// for avcpu sysdate sync on apcpu
  */
#define IPC_MSG_AUD_FW_SYSDATE_GET ((AUD_FW_SYSDATE_GET << 16) + \
                                            SYS_DEV_TYPE_AUDIO)

/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_AUD_FW_MSG_DEPTH 8

/*!
  This structure defines the pos
  */
typedef struct
{
  /*!
    X pos
    */
  u32 x;
  /*!
    Y pos
    */
  u32 y;
} pos_t;

/*!
  This structure defines the rect size
  */
typedef struct
{
  /*!
    width
    */
  u32 w;
  /*!
    height
    */
  u32 h;
} rect_size_t;

/*!
  This structure defines the rect for driver usage
  */
typedef struct
{
  /*!
    X pos
    */
  u32 x;
  /*!
    Y pos
    */
  u32 y;
  /*!
    width
    */
  u32 w;
  /*!
    height
    */
  u32 h;
} rect_vsb_t;


/*!
  This structure defines the colorspace
  */
typedef enum
{
  /*!
    rgb
    */
  COLOR_RGB,
  /*!
    yuv
    */
  COLOR_YUV,
  /*!
    jazz: cvbs+s-video
    */
  COLOR_SVID_CVBS,
  /*!
    jazz: cvbs+YUV
    */
  COLOR_CVBS_YUV,
  /*!
    jazz: cvbs+RGB
    */
  COLOR_CVBS_RGB,
  /*!
    jazz: 2cvbs+s-video
    */
  COLOR_2CVBS_SVID,
} colorspace_t;

/*!
  This structure defines the pixfmt
  */
typedef enum
{
  /*!
    1-bit RGB Palette index  argb palette  = 0
    */
  PIX_FMT_RGBPALETTE1,
  /*!
    2-bit RGB Palette index  argb palette  = 1
    */
  PIX_FMT_RGBPALETTE2,
  /*!
    4-bit RGB Palette index  argb palette  = 2
    */
  PIX_FMT_RGBPALETTE4,
  /*!
    8-bit RGB Palette index  argb palette  = 3
    */
  PIX_FMT_RGBPALETTE8,
  /*!
    1-bit YUV Palette index  ayuv palette  = 4
    */
  PIX_FMT_YUVPALETTE1,
  /*!
    2-bit YUV Palette index  ayuv palette  = 5
    */
  PIX_FMT_YUVPALETTE2,
  /*!
    4-bit YUV Palette index  ayuv palette  = 6
    */
  PIX_FMT_YUVPALETTE4,
  /*!
    8-bit YUV Palette index  ayuv palette  = 7
    */
  PIX_FMT_YUVPALETTE8,
  /*!
    A1 and 1-bit RGB Palette index  argb palette  = 8
    */
  PIX_FMT_ARGBPALETTE11,
  /*!
    A2 and 2-bit RGB Palette index  argb palette  = 9
    */
  PIX_FMT_ARGBPALETTE22,
  /*!
    A4 and 4-bit RGB Palette index  argb palette  = 10
    */
  PIX_FMT_ARGBPALETTE44,
  /*!
    A8 and 8-bit RGB Palette index  argb palette  = 11
    */
  PIX_FMT_ARGBPALETTE88,
  /*!
    A1 and 1-bit YUV Palette index  ayuv palette  = 12
    */
  PIX_FMT_AYUVPALETTE11,
  /*!
    A2 and 2-bit YUV Palette index  ayuv palette  = 13
    */
  PIX_FMT_AYUVPALETTE22,
  /*!
    A4 and 4-bit YUV Palette index  ayuv palette  = 14
    */
  PIX_FMT_AYUVPALETTE44,
  /*!
    A8 and 8-bit YUV Palette index  ayuv palette  = 15
    */
  PIX_FMT_AYUVPALETTE88,
  /*!
    8-bit, no per-pixel alpha        = 16
    */
  PIX_FMT_RGB233,
  /*!
    16-bit, no per-pixel alpha       = 17
    */
  PIX_FMT_RGB565,
  /*!
    16-bit                           = 18
    */
  PIX_FMT_ARGB1555,
  /*!
    16-bit                           = 19
    */
  PIX_FMT_RGBA5551,
  /*!
    16-bit                           = 20
    */
  PIX_FMT_ARGB4444,
  /*!
    16-bit                           = 21
    */
  PIX_FMT_RGBA4444,
  /*!
    32-bit                           = 22
    */
  PIX_FMT_ARGB8888,
  /*!
    32-bit                           = 23
    */
  PIX_FMT_RGBA8888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 24
    */
  PIX_FMT_Y0CBY1CR8888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 25
    */
  PIX_FMT_Y0CRY1CB8888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 26
    */
  PIX_FMT_Y1CBY0CR8888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 27
    */
  PIX_FMT_Y1CRY0CB8888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 28
    */
  PIX_FMT_CBY0CRY18888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 29
    */
  PIX_FMT_CBY1CRY08888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 30
    */
  PIX_FMT_CRY1CBY08888,
  /*!
    32-bit for 2 pixels, YCbCr422, 8-bit values    = 31
    */
  PIX_FMT_CRY0CBY18888,
  /*!
    32-bit for 1 pixel,  YCbCr444, 10-bit values   = 32
    */
  PIX_FMT_X2C10Y10CB10,
  /*!
    YCbCr444, 8-bit values   ayuv8888              = 33
    */
  PIX_FMT_AYCBCR8888,
  /*!
    YCbCr444, 8-bit values   vuya8888              = 34
    */
  PIX_FMT_CRCBYA8888,
  /*!
    YCbCr444, 8-bit values   yuva8888              = 35
    */
  PIX_FMT_YCBCRA8888,
  /*!
    YCbCr444, 8-bit values                         = 36
    */
  PIX_FMT_ACRCBY8888,
  /*!
    YCbCr444, 24-bit values                        = 37
    */
  PIX_FMT_YCBCR444,
  /*!
    YCbCr422, 16-bit values                        = 38
    */
  PIX_FMT_YCBCR422,
  /*!
    YCbCr420, 12-bit values                        = 39
    */
  PIX_FMT_YCBCR420,
  /*!
    2-bit RGB Palette index  bgra palette  = 40
    */
  PIX_FMT_RGBPALETTE2_PALETTE_BGRA,
  /*!
    4-bit RGB Palette index  bgra palette  = 41
    */
  PIX_FMT_RGBPALETTE4_PALETTE_BGRA,
  /*!
    8-bit RGB Palette index  bgra palette  = 42
    */
  PIX_FMT_RGBPALETTE8_PALETTE_BGRA,
  /*!
    2-bit YUV Palette index  vuya palette  = 43
    */
  PIX_FMT_YUVPALETTE2_PALETTE_VUYA,
  /*!
    4-bit YUV Palette index  vuya palette  = 44
    */
  PIX_FMT_YUVPALETTE4_PALETTE_VUYA,
  /*!
    8-bit YUV Palette index  vuya palette  = 45
    */
  PIX_FMT_YUVPALETTE8_PALETTE_VUYA,
  /*!
    A4 and 4-bit RGB Palette index   bgra palette   = 46
    */
  PIX_FMT_ARGBPALETTE44_PALETTE_BGRA,
  /*!
    A8 and 8-bit RGB Palette index  bgra palette  = 47
    */
  PIX_FMT_ARGBPALETTE88_PALETTE_BGRA,
  /*!
    8-bit RGB and A8 Paletteindex   argb palette   = 48
    */
  PIX_FMT_RGBAPALETTE88,
  /*!
     8-bit RGB and A8 Palette index  bgra palette  = 49
    */
  PIX_FMT_RGBAPALETTE88_PALETTE_BGRA,
  /*!
    A4 and 4-bit YUV Palette index  vuya palette  = 50
    */
  PIX_FMT_AYUVPALETTE44_PALETTE_VUYA,
  /*!
    A8 and 8-bit YUV Palette index  vuya palette  = 51
    */
  PIX_FMT_AYUVPALETTE88_PALETTE_VUYA,
  /*!
    8-bit YUV and A8 Palette index  ayuv palette  = 52
    */
  PIX_FMT_YUVAPALETTE88,
  /*!
     8-bit YUV and A8 Palette index  vuya palette  = 53
    */
  PIX_FMT_YUVAPALETTE88_PALETTE_VUYA,
  /*!
    16-bit, no per-pixel alpha       = 54
    */
  PIX_FMT_RGB565_SMALL_ENDIAN,
  /*!
    16-bit                           = 55
    */
  PIX_FMT_ARGB1555_SMALL_ENDIAN,
  /*!
    16-bit                           = 56
    */
  PIX_FMT_ARGB4444_SMALL_ENDIAN,
  /*!
    32-bit                           = 57
    */
  PIX_FMT_ARGB8888_SMALL_ENDIAN,
  /*!
    32-bit                           = 58
    */
  PIX_FMT_RGBA8888_SMALL_ENDIAN,
  /*!
    16-bit                           = 59
    */
  PIX_FMT_RGBA5551_SMALL_ENDIAN,
  /*!
    16-bit                           = 60
    */
  PIX_FMT_RGBA4444_SMALL_ENDIAN,
  /*!
    2-bit RGB Palette index  rgba palette  = 61
    */
  PIX_FMT_RGBPALETTE2_PALETTE_RGBA,
  /*!
    2-bit RGB Palette index  abgr palette  = 62
    */
  PIX_FMT_RGBPALETTE2_PALETTE_ABGR,
  /*!
    4-bit RGB Palette index  rgba palette  = 63
    */
  PIX_FMT_RGBPALETTE4_PALETTE_RGBA,
  /*!
    4-bit RGB Palette index  abgr palette  = 64
    */
  PIX_FMT_RGBPALETTE4_PALETTE_ABGR,
  /*!
    8-bit RGB Palette index  rgba palette  = 65
    */
  PIX_FMT_RGBPALETTE8_PALETTE_RGBA,
  /*!
    8-bit RGB Palette index  abgr palette  = 66
    */
  PIX_FMT_RGBPALETTE8_PALETTE_ABGR,
  /*!
    2-bit YUV Palette index  yuva palette  = 67
    */
  PIX_FMT_YUVPALETTE2_PALETTE_YUVA,
  /*!
    2-bit YUV Palette index  avuy palette  = 68
    */
  PIX_FMT_YUVPALETTE2_PALETTE_AVUY,
  /*!
    4-bit YUV Palette index  yuva palette  = 69
    */
  PIX_FMT_YUVPALETTE4_PALETTE_YUVA,
  /*!
    4-bit YUV Palette index  avuy palette  = 70
    */
  PIX_FMT_YUVPALETTE4_PALETTE_AVUY,
  /*!
    8-bit YUV Palette index  yuva palette  = 71
    */
  PIX_FMT_YUVPALETTE8_PALETTE_YUVA,
  /*!
    8-bit YUV Palette index  avuy palette  = 72
    */
  PIX_FMT_YUVPALETTE8_PALETTE_AVUY,
  /*!
    A4 and 4-bit RGB Palette index   rgba palette   = 73
    */
  PIX_FMT_ARGBPALETTE44_PALETTE_RGBA,
  /*!
    A4 and 4-bit RGB Palette index   abgr palette   = 74
    */
  PIX_FMT_ARGBPALETTE44_PALETTE_ABGR,
  /*!
    A8 and 8-bit RGB Palette index   rgba palette   = 75
    */
  PIX_FMT_ARGBPALETTE88_PALETTE_RGBA,
  /*!
    A8 and 8-bit RGB Palette index   abgr palette   = 76
    */
  PIX_FMT_ARGBPALETTE88_PALETTE_ABGR,
  /*!
    8-bit RGB and A8 Paletteindex   rgba palette   = 77
    */
  PIX_FMT_RGBAPALETTE88_PALETTE_RGBA,
  /*!
     8-bit RGB and A8 Palette index  abgr palette  = 78
    */
  PIX_FMT_RGBAPALETTE88_PALETTE_ABGR,
  /*!
    A4 and 4-bit YUV Palette index  yuva palette  = 79
    */
  PIX_FMT_AYUVPALETTE44_PALETTE_YUVA,
  /*!
    A4 and 4-bit YUV Palette index  avuy palette  = 80
    */
  PIX_FMT_AYUVPALETTE44_PALETTE_AVUY,
  /*!
    A8 and 8-bit YUV Palette index  yuva palette  = 81
    */
  PIX_FMT_AYUVPALETTE88_PALETTE_YUVA,
  /*!
    A8 and 8-bit YUV Palette index  avuy palette  = 82
    */
  PIX_FMT_AYUVPALETTE88_PALETTE_AVUY,
  /*!
    8-bit YUV and A8 Palette index  yuva palette  = 83
    */
  PIX_FMT_YUVAPALETTE88_PALETTE_YUVA,
  /*!
     8-bit YUV and A8 Palette index  avuy palette  = 84
    */
  PIX_FMT_YUVAPALETTE88_PALETTE_AVUY,
  /*!
      = 85
    */
  PIX_FMT_GRAY_8,
  /*!
    used by concerto = 86
  */
  PIX_FMT_RGBPALETTE1_PALETTE_BGRA,
  /*!
    used by concerto = 87
  */
  PIX_FMT_XY,
  /*!
    used by concerto = 88
  */
  PIX_FMT_XYL,
  /*!
    used by concerto = 89
  */
  PIX_FMT_XYC,
  /*!
    used by concerto = 90
  */
  PIX_FMT_XYLC,
  /*!
    used by concerto = 91
  */
  PIX_FMT_XY_SMALL,
  /*!
    used by concerto = 92
  */
  PIX_FMT_XYL_SMALL,
  /*!
    used by concerto = 93
  */
  PIX_FMT_XYC_SMALL,
  /*!
    used by concerto = 94
  */
  PIX_FMT_XYLC_SMALL,
  /*!
    used by concerto = 95
  */
  PIX_FMT_TILE,
  /*!
    rgb888 24bits = 96
  */
  PIX_FMT_RGB888,
  /*!
    bgr8888 24bits = 97
  */
  PIX_FMT_BGR888,
  /*!
    gray16 16bits = 98
  */
  PIX_FMT_GRAY_16,
  /*!
    semi-planner yuv = 99
  */
  PIX_FMT_SP_YUV444,
  /*!
    semi-planner yuv = 100
  */
  PIX_FMT_SP_YUV422,
  /*!
    semi-planner yuv = 101
  */
  PIX_FMT_SP_YUV420,
  /*!
    semi-planner yuv = 102
  */
  PIX_FMT_SP_YUV422_1x2,
  /*!
    semi-planner yuv = 103
  */
  PIX_FMT_SP_YUV422_2x1,
  /*!
    semi-planner yuv = 104
  */
  PIX_FMT_SP_YUV444_UVSWAP,
  /*!
    semi-planner yuv = 105
  */
  PIX_FMT_SP_YUV422_UVSWAP,
  /*!
    semi-planner yuv = 106
  */
  PIX_FMT_SP_YUV420_UVSWAP,
  /*!
    semi-planner yuv = 107
  */
  PIX_FMT_SP_YUV422_1x2_UVSWAP,
  /*!
    semi-planner yuv = 108
  */
  PIX_FMT_SP_YUV422_2x1_UVSWAP,
  /*!
    cmyk = 109
  */
  PIX_FMT_CMYK,
  /*!
    kymc = 110
  */
  PIX_FMT_KYMC,
  /*!
    semi-planner cmyk = 111
  */
  PIX_FMT_SP_CMYK,
  /*!
    semi-planner cmyk = 112
  */
  PIX_FMT_SP_CMYK_SWAP,
  /*!
  gray 1 bit per pixel = 113
  */
  PIX_FMT_GRAY_1,
  /*!
  gray 2 bits per pixel = 114
  */
  PIX_FMT_GRAY_2,
  /*!
  gray 4 bits per pixel = 115
  */
  PIX_FMT_GRAY_4,
  /*!
  gray with alpha, 16bits per pixel,  = 116
  */
  PIX_FMT_AGRAY_8_8,  
  /*!
  gray with alpha, 32bits per pixel,  = 117
  */
  PIX_FMT_AGRAY_16_16,  
  /*!
    bgr16_16_16 48bits per pixel = 118
  */
  PIX_FMT_BGR48,  
  /*!
    abgr16_16_16_16 64bits per pixel= 119
  */
  PIX_FMT_ABGR64,    
  /*!
    max
    */
  PIX_FMT_MAX
} pix_fmt_t;

/*!
  The luma and chroma sampling formats
  */
typedef enum
{
  /*!
    4:2:0
    */
  YUV_420,
  /*!
    4:2:2
    */
  YUV_422,
  /*!
    4:4:4
    */
  YUV_444
}yuv_sample_t;

/*!
  The aspect ratio
  */
typedef enum
{
  /*!
     4:3
     */
  AR_43,
  /*!
     16:9
     */
  AR_169,
  /*!
    Square or 1:1
    */
  AR_SQUARE
}aspect_ratio_t;

/*!
  GPIO pin information, 1 Byte
  */
typedef struct
{
  /*!
    Polarity of GPIO, GPIO_LEVEL_LOW or GPIO_LEVEL_HIGH active(light)
    */
  u16 polar:1;
  /*!
    GPIO direction: GPIO_DIR_OUTPUT or GPIO_DIR_INPUT
    */
  u16 io   :1;
  /*!
    GPIO index, upto 64 GPIO, 63 means invalid
    */
  u16 pos  :6;
  /*!
    Definition of GPIO level low
    */
  u16 polar_0 :4;
  /*!
    Definition of GPIO level high
    */
  u16 polar_1 :4;
}gpio_pin_t;

/*!
  This structure defines the profile id of mpeg2 video
  */

typedef enum
{
  /*!
    unkonwn or uninit
    */
  MPEG2_UNKOWN_PROFILE,
  /*!
    simple profile
    */
  MPEG2_SIMPLE_PROFILE,
  /*!
    main profile
    */
  MPEG2_MAIN_PROFILE,
  /*!
    snr scalable profile
    */
  MPEG2_SNR_SCALABLE_PROFILE,
    /*!
    spatially scalable profile
    */
  MPEG2_SPATIAL_SCALABLE_PROFILE,
    /*!
    high profile
    */
  MPEG2_HIGH_PROFILE,
  /*!
    max
    */
  MPEG2_MAX_PROFILE,
}MEPG2_PROFILE_format_t;

/*!
  This structure defines the level id of mpeg2 video
  */

typedef enum
{
  /*!
    unkonwn or uninit
    */
  MPEG2_UNKOWN_LEVEL,
  /*!
    low level
    */
  MPEG2_LOW_LEVEL,
  /*!
    main level
    */
  MPEG2_MAIN_LEVEL,
  /*!
    high 1440 level
    */
  MPEG2_HIGH1440_LEVEL,
    /*!
    high level
    */
  MPEG2_HIGH_LEVEL,
  /*!
    max
    */
  MPEG2_MAX_LEVEL,
}MEPG2_LEVEL_format_t;

/*!
  This structure defines the profile id of mpeg4 video
  */

typedef enum
{
  /*!
    unkonwn or uninit
    */
  MPEG4_UNKOWN_PROFILE,
  /*!
    simple profile
    */
  MPEG4_SIMPLE_PROFILE,
  /*!
    advanced simple profile
    */
  MPEG4_ADVANCED_SIMPLE_PROFILE,
  /*!
    main profile
    */
  MPEG4_MAIN_PROFILE,
  /*!
    max
    */
  MPEG4_MAX_PROFILE,
}MEPG4_PROFILE_format_t;

/*!
  This structure defines the level id of mpeg4 video
  */

typedef enum
{
  /*!
    unkonwn or uninit
    */
  MPEG4_UNKOWN_LEVEL,
  /*!
    level 0
    */
  MPEG4_0_LEVEL,
  /*!
    level 1
    */
  MPEG4_1_LEVEL,
  /*!
    level 2
    */
  MPEG4_2_LEVEL,
  /*!
    level 3
    */
  MPEG4_3_LEVEL,
  /*!
    level 4
    */
  MPEG4_4_LEVEL,
  /*!
    level 5
    */
  MPEG4_5_LEVEL,
  /*!
    Simple Profile level 0
    */
  MPEG4_SIMPLE_0_LEVEL,
  /*!
    max
    */
  MPEG4_MAX_LEVEL,
}MEPG4_LEVEL_format_t;
/*!
  This structure defines the profile id of h264/mpeg4-avc video
  */

typedef enum
{
  /*!
    unkonwn or uninit
    */
  MPEG4AVC_UNKOWN_PROFILE,
  /*!
    baseline profile
    */
  MPEG4AVC_BASELINE_PROFILE,
  /*!
    main profile
    */
  MPEG4AVC_MAIN_PROFILE,
  /*!
    extended profile
    */
  MPEG4AVC_EXTENDED_PROFILE,
    /*!
    high profile
    */
  MPEG4AVC_HIGH_PROFILE,
    /*!
    high10 profile
    */
  MPEG4AVC_HIGH10_PROFILE,
    /*!
    high422profile
    */
  MPEG4AVC_HIGH422_PROFILE,
    /*!
    high444 profile
    */
  MPEG4AVC_HIGH444_PROFILE,
  /*!
    max
    */
  MPEG4AVC_MAX_PROFILE,
}MEPG4AVC_PROFILE_format_t;

/*!
  This structure defines the level id of h264/mpeg4-avc video
  */
typedef enum
{
  /*!
    unkonwn or uninit
    */
  MPEG4AVC_UNKOWN_LEVEL,
  /*!
    level 1
    */
  MPEG4AVC_LEVEL_1,
  /*!
    level 1b
    */
  MPEG4AVC_LEVEL_1b,
  /*!
    level 1.1
    */
  MPEG4AVC_LEVEL_11,
    /*!
    level 1.2
    */
  MPEG4AVC_LEVEL_12,
    /*!
    level 1.3
    */
  MPEG4AVC_LEVEL_13,
    /*!
    level 2
    */
  MPEG4AVC_LEVEL_2,
    /*!
    level 2.1
    */
  MPEG4AVC_LEVEL_21,
    /*!
    level 2.2
    */
  MPEG4AVC_LEVEL_22,
    /*!
    level 3
    */
  MPEG4AVC_LEVEL_3,
    /*!
    level 3.1
    */
  MPEG4AVC_LEVEL_31,
    /*!
    level 3.2
    */
  MPEG4AVC_LEVEL_32,
    /*!
    level 4
    */
  MPEG4AVC_LEVEL_4,
    /*!
    level 4.1
    */
  MPEG4AVC_LEVEL_41,
    /*!
    level 4.2
    */
  MPEG4AVC_LEVEL_42,
    /*!
    level 5
    */
  MPEG4AVC_LEVEL_5,
    /*!
    level 5.1
    */
  MPEG4AVC_LEVEL_51,
  /*!
    max
    */
  MPEG4AVC_MAX_LEVEL,
}MEPG4AVC_LEVEL_format_t;

/*!
  This structure defines the profile id of avs video
  */
typedef enum
{
  AVS_UNKNOWN_PROFILE,
  /*!
    baseline profile
    */
  AVS_BASELINE_PROFILE,
  /*!
    broadcasting profile
    */
  AVS_BROADCASTING_PROFILE,
  /*!
    profile max
    */
  AVS_MAX_PROFILE,
}AVS_PROFILE_format_t;

/*!
  This structure defines the level id of avs
  */
typedef enum
{
  AVS_UNKNOWN_LEVEL,
  /*!
    level 2.0.0
    */
  AVS_2_0_0_LEVEL,
  /*!
    level 2.1.0.8.15
    */
  AVS_2_1_0_08_15_LEVEL,
  /*!
    level 2.1.0.8.30
    */
  AVS_2_1_0_08_30_LEVEL,
  /*!
    level 4.0.0
    */
  AVS_4_0_0_LEVEL,
  /*!
    level 4.2.0
    */
  AVS_4_2_0_LEVEL,
  /*!
    level 4.0.2
    */
  AVS_4_0_2_LEVEL,
  /*!
    level 6.0.0
    */
  AVS_6_0_0_LEVEL,
  /*!
    level 6.0.1
    */
  AVS_6_0_1_LEVEL,
  /*!
    level 6.2.0
    */
  AVS_6_2_0_LEVEL,
  /*!
    level 6.0.3
    */
  AVS_6_0_3_LEVEL,
  /*!
    level 6.0.5
    */
  AVS_6_0_5_LEVEL,
  /*!
    level max
    */
  AVS_MAX_LEVEL
}AVS_LEVEL_format_t;


//add by HY for autotest 2012-3-28 begin
/*!
  Autotest info
  */
typedef struct
{
  /*!
    Key frame.
    */
  int is_key_frame[20];
  /*!
    luma address
    */
  unsigned int luma_addr[20];
  /*!
    chroma address
    */
  unsigned int chroma_addr[20];
  /*!
    length
    */
  unsigned int height;
  /*!
    width
    */
  unsigned int width;
  /*!
    frames statistic, have display frames
    */
  u32 frames_stat;
  /*!
    frames statistic, have decoder frames
    */
  u32 decoder_frame_count;
  /*!
    cur frames pts, 0 is invalidt
    */
  u32 pts;
  /*!
  video decoding is ready for display
  */
  MT_BOOL is_stable;
  /*!
    open or close di flage
    */
  MT_BOOL open_di;
  /*!
    av sync status
    */
  MT_BOOL is_av_sync;
  /*!
    no enough buffer for video decoder
  */
  MT_BOOL no_enough_buffer;
  /*!
    First I frame output
  */
  MT_BOOL I_frame_stable;
  /*!
    Image is field pic.
  */
  u32 field_pic_flag;
  /*!
    If image is sd image, the flage is true.
  */
  MT_BOOL sd_image_flag;
  /*!
    If image is interlaced, the flage is true.
  */
  MT_BOOL interlaced_flag;
  /*!
    video format.
  */
  u32 video_format;
  /*!
    video decoder err data flag
  */
  u32 vdec_err_data_flag;
  /*!
    display fifo number.
  */
  u32  fifo_cnt;
  /*!
    display fifo remain number.
  */
  u32  remain_frm_cnt;
  /*!
    video profile id.
  */
  /*!
    video decoder unsupported.
  */
  MT_BOOL  vdec_unsupported_flag;
  /*!
    video profile id.
  */
  union{
  unsigned int profile_id;
  MEPG2_PROFILE_format_t mpeg2_profile_id;
  MEPG4_PROFILE_format_t mpeg4_profile_id;
  MEPG4AVC_PROFILE_format_t mpeg4avc_profile_id;
  AVS_PROFILE_format_t avs_profile_id;
  };
  /*!
    video level id.
  */
  union{
  unsigned int level_id;
  MEPG2_LEVEL_format_t mpeg2_level_id;
  MEPG4_LEVEL_format_t mpeg4_level_id;
  MEPG4AVC_LEVEL_format_t mpeg4avc_level_id;
  AVS_LEVEL_format_t avs_level_id;
  };
}autotest_info;

/*!
  This structure defines memory copy info.
  */
typedef struct
{
  /*!
    Dst addr.
    */
  u32 dst_addr;
  /*!
    luma address
    */
  u32 src_addr;
  /*!
    Cpy buffer size.
    */
  u32 size;
  /*!
    Return val.
    */
  u32 ret;
}mem_cpy_info;

typedef enum
{
	CONCERTO_CHIP_A0 = 0x6000,
	CONCERTO_CHIP_A1 = 0x6010,
	CONCERTO_CHIP_A2 = 0x6030,
	CONCERTO_CHIP_A3 = 0x6020,
}CONCERTO_CHIPID_E;

int concerto_get_chipid(void); // by gu zhimin

/*!
    for debug
  */
#define DBG_PRT(...)
/*!
    for debug
  */
//#define DBG_PRT  AV_PRINTF

#ifdef __cplusplus
}
#endif

#endif //__COMMON_H__

