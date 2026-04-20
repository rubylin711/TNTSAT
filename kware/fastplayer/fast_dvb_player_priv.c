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
#include "common.h"
#include "drv_dev.h"
#include "dmx.h"
#include "vdec.h"
#include "aud_vsb.h"
#include "drv_misc.h"
#include "scart.h"
#include "rf.h"
#include "nim.h"
#include "avsync.h"
#include "lib_rect.h"
#include "display.h"
#include "hal_misc.h"

#include "ts_sequence.h"
#include "fast_dvb_player.h"
#include "fast_dvb_player_priv.h"

typedef struct tag_fdp_perf_test_record
{
  u32 start_play_tick;
  u32 start_lock_tick;
  u32 start_render_reset_tick;
  u32 start_dmx_reset_tick;
  u32 start_vdec_start_tick;
  u32 start_adec_start_tick;
  u32 start_vdec_stop_tick;
  u32 start_adec_stop_tick;
  u32 start_audio_set_tick;
  u32 start_ack_ui_tick;
}fdp_perf_test_record_t;

static fdp_perf_test_record_t g_fdp_perf_test_para = {0};

void fast_dvb_player_pref_test(fdp_perf_test_t e)
{
  u32 t = mtos_ticks_get();
  mtos_open_printk();
  switch(e)
  {
    case FDP_PERF_TEST_START_PLAY:
      memset(&g_fdp_perf_test_para, 0, sizeof(fdp_perf_test_record_t));
      g_fdp_perf_test_para.start_play_tick = t;
      FDP_PERF_PRINT("fdp->play start\n");
      break;
    case FDP_PERF_TEST_STOP_PLAY:
      FDP_PERF_PRINT("fdp->play stop\n");
      return; //skip mtos_close_printk
      
    case FDP_PERF_TEST_LOCK_S:
      g_fdp_perf_test_para.start_lock_tick = t;
      FDP_PERF_PRINT("fdp->lock before\n");
      break;
    case FDP_PERF_TEST_LOCK_E:
      FDP_PERF_PRINT("fdp->lock after[%d]\n", t - g_fdp_perf_test_para.start_lock_tick);
      break;
      
    case FDP_PERF_TEST_RENDER_RESET_S:
      g_fdp_perf_test_para.start_render_reset_tick = t;
      FDP_PERF_PRINT("fdp->render reset before\n");
      break;
    case FDP_PERF_TEST_RENDER_RESET_E:
      FDP_PERF_PRINT("fdp->render reset after[%d]\n", t - g_fdp_perf_test_para.start_render_reset_tick);
      break;

    case FDP_PERF_TEST_DMX_RESET_S:
      g_fdp_perf_test_para.start_dmx_reset_tick = t;
      FDP_PERF_PRINT("fdp->dmx reset before\n");
      break;
    case FDP_PERF_TEST_DMX_RESET_E:
      FDP_PERF_PRINT("fdp->dmx reset after[%d]\n", t - g_fdp_perf_test_para.start_dmx_reset_tick);
      break;
      
    case FDP_PERF_TEST_VDEC_START_S:
      g_fdp_perf_test_para.start_vdec_start_tick = t;
      FDP_PERF_PRINT("fdp->vdec start before\n");
      break;
    case FDP_PERF_TEST_VDEC_START_E:
      FDP_PERF_PRINT("fdp->vdec start after[%d]\n", t - g_fdp_perf_test_para.start_vdec_start_tick);
      break;
      
    case FDP_PERF_TEST_ADEC_START_S:
      g_fdp_perf_test_para.start_adec_start_tick = t;
      FDP_PERF_PRINT("fdp->adec start before\n");
      break;
    case FDP_PERF_TEST_ADEC_START_E:
      FDP_PERF_PRINT("fdp->adec start after[%d]\n", t - g_fdp_perf_test_para.start_adec_start_tick);
      break;
      
    case FDP_PERF_TEST_VDEC_STOP_S:
      g_fdp_perf_test_para.start_vdec_stop_tick = t;
      FDP_PERF_PRINT("fdp->vdec stop before\n");
      break;
    case FDP_PERF_TEST_VDEC_STOP_E:
      FDP_PERF_PRINT("fdp->vdec stop after[%d]\n", t - g_fdp_perf_test_para.start_vdec_stop_tick);
      break;
      
    case FDP_PERF_TEST_ADEC_STOP_S:
      g_fdp_perf_test_para.start_adec_stop_tick = t;
      FDP_PERF_PRINT("fdp->adec stop before\n");
      break;
    case FDP_PERF_TEST_ADEC_STOP_E:
      FDP_PERF_PRINT("fdp->adec stop after[%d]\n", t - g_fdp_perf_test_para.start_adec_stop_tick);
      break;
      
    case FDP_PERF_TEST_AUDIO_SET_S:
      g_fdp_perf_test_para.start_audio_set_tick = t;
      FDP_PERF_PRINT("fdp->audio set before\n");
      break;
    case FDP_PERF_TEST_AUDIO_SET_E:
      FDP_PERF_PRINT("fdp->audio set after[%d]\n", t - g_fdp_perf_test_para.start_audio_set_tick);
      break;
      
    case FDP_PERF_TEST_ACK_UI_S:
      g_fdp_perf_test_para.start_ack_ui_tick = t;
      FDP_PERF_PRINT("fdp->ack ui before\n");
      break;
    case FDP_PERF_TEST_ACK_UI_E:
      FDP_PERF_PRINT("fdp->ack ui after[%d]\n", t - g_fdp_perf_test_para.start_ack_ui_tick);
      break;

    default:
      break;
  }
  
  mtos_close_printk();
}

