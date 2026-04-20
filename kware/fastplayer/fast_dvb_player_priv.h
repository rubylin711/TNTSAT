/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __FAST_DVB_PLAYER_PRIV_H_
#define __FAST_DVB_PLAYER_PRIV_H_

//#define PERF_TEST

#ifdef PERF_TEST
#define FDP_PERF_TEST fast_dvb_player_pref_test
#define FDP_PERF_PRINT OS_PRINTF

#define FDP_PERF_DBG(x) mtos_open_printk(); OS_PRINTF x; mtos_close_printk()
#else
#define FDP_PERF_TEST(x)
#define FDP_PERF_PRINT DUMMY_PRINTF

#define FDP_PERF_DBG(x)  OS_PRINTF x
#endif

/*!
  fast player performance test
  */
typedef enum
{
  /*!
    Start Tag
    */
  FDP_PERF_TEST_START,
  /*!
    Start play
    */
  FDP_PERF_TEST_START_PLAY,
  /*!
    Stop play
    */
  FDP_PERF_TEST_STOP_PLAY,
  /*!
    Lock Nim
    */
  FDP_PERF_TEST_LOCK_S,
  /*!
    Lock Nim
    */
  FDP_PERF_TEST_LOCK_E,
  /*!
    Render reset
    */
  FDP_PERF_TEST_RENDER_RESET_S,
  /*!
    Render reset
    */
  FDP_PERF_TEST_RENDER_RESET_E,
  /*!
    Demux reset
    */
  FDP_PERF_TEST_DMX_RESET_S,
  /*!
    Demux reset
    */
  FDP_PERF_TEST_DMX_RESET_E,
  /*!
    video decorder start
    */
  FDP_PERF_TEST_VDEC_START_S,
  /*!
    video decorder start
    */
  FDP_PERF_TEST_VDEC_START_E,
  /*!
    audio decorder start
    */
  FDP_PERF_TEST_ADEC_START_S,
  /*!
    audio decorder start
    */
  FDP_PERF_TEST_ADEC_START_E,
  /*!
    video decorder stop
    */
  FDP_PERF_TEST_VDEC_STOP_S,
  /*!
    video decorder stop
    */
  FDP_PERF_TEST_VDEC_STOP_E,
  /*!
    audio decorder stop
    */
  FDP_PERF_TEST_ADEC_STOP_S,
  /*!
    audio decorder stop
    */
  FDP_PERF_TEST_ADEC_STOP_E,
  /*!
    audio mode set
    */
  FDP_PERF_TEST_AUDIO_SET_S,
  /*!
    audio mode set
    */
  FDP_PERF_TEST_AUDIO_SET_E,
  /*!
    ack ui
    */
  FDP_PERF_TEST_ACK_UI_S,
  /*!
    ack ui
    */
  FDP_PERF_TEST_ACK_UI_E,

  /*!
    End Tag
    */
  FDP_PERF_TEST_END
}fdp_perf_test_t;


void fast_dvb_player_pref_test(fdp_perf_test_t e);

#endif // End for __FAST_DVB_PLAYER_PRIV_H_

