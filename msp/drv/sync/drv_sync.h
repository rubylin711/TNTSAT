/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __SYNC_DRV_H__
#define __SYNC_DRV_H__

#include "mt_drv_sync.h"
#include "drv_sync_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

MT_BOOL SYNC_VerifyHandle(mt_handle hSync);
mt_s32 SYNC_StartSync(mt_u32 hSync);
mt_void SYNC_VidProc(mt_handle hSync, SYNC_VID_INFO_S *pVidInfo, SYNC_VID_OPT_S *pVidOpt);
mt_void SYNC_AudProc(mt_handle hSync, SYNC_AUD_INFO_S *pAudInfo, SYNC_AUD_OPT_S *pAudOpt);
mt_void SYNC_PcrProc(mt_handle hSync, mt_u32 PcrTime);
mt_s32 SYNC_PauseSync(mt_u32 SyncId);
mt_s32 SYNC_ResumeSync(mt_u32 SyncId);
mt_s32 SYNC_GetTime(mt_u32 SyncId, mt_upts *pLocalTime, mt_upts *pPlayTime);
mt_spts SYNC_GetLocalTime(SYNC_S *pSync, SYNC_CHAN_E enChn);
mt_s32 SYNC_reference_config(SYNC_S *pSync);
mt_void SYNC_UpLoadEvent(SYNC_S *pSync, SYNC_AVSYNC_EVENT_E event);
mt_s32 SYNC_PlayInfoCfg(mt_u32 *pargs);
mt_void avsync_audio_loop(struct avsync_async_info_t *pdata, AVSYNC_FRAME_SYNCFLAG_E *psync_flg);
mt_void avsync_video_loop(mt_void *pdata);
mt_s32 avsync_audio_init(mt_u32 atype);
mt_s32 avsync_audio_deinit(mt_void);
mt_s32 avsync_video_init(mt_u32 vtype);
mt_s32 avsync_video_deinit(mt_void);
mt_s32 avsync_module_init(mt_void);
mt_s32 avsync_module_deinit(mt_void);
mt_s32 avsync_play_info_cfg(mt_u32 *pargs);
mt_s32 avsync_data_source_cfg(MT_SYNC_DATA_SOURCE_E *pargs);
mt_s32 avsync_apts_adjust(mt_u32 adjust);
mt_s32 avsync_audio_do_track(mt_u32 *ptrack);
mt_void avsync_dump_state_proc(struct seq_file *p);
mt_s32 avsync_get_avsync_info(SYNC_GET_AVSYNC_INFO_S *pavsyncinfo);
void avsync_fill_avsync_ci_test_info(MT_UNF_AVPLAY_CI_TEST_INFO_S* pstInfo);
void avsync_trick_seek_to_normal(void);
mt_s32 avsync_audio_push_pts(SYNC_S  *pSync, SYNC_PUSH_APTS_S *pSyncApts);
mt_s32 avsync_set_slow_sync_policy(MT_BOOL on_off);
mt_s32 avsync_get_time_info(mt_u32 SyncId, SYNC_GET_TIME_INFO_S *pSyncTimeInfo);
mt_s32 avsync_video_get_play_cnt(void);
mt_s32 avsync_video_frame_info_capture_create(unsigned int num);
mt_s32 avsync_video_frame_info_capture_destroy(void);
mt_s32 avsync_video_frame_info_capture_read(SYNC_VOUT_FRAME_INFO *frame_info);
MT_BOOL avsync_is_finished(mt_void);
mt_s32 avsync_is_ddp_avsync_verfication(void);
void avsync_set_aud_frame_sample_num(mt_u32 num);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif


