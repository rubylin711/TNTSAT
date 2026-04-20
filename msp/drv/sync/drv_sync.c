/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/sched/clock.h>
#include <linux/printk.h>
#include <linux/hrtimer.h>
#include <linux/sizes.h>
#include <linux/interrupt.h>
#include <linux/sched/clock.h>
#include <linux/math64.h>
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module_debug.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_ao.h"
#include "mt_drv_sync.h"
#include "drv_sync_priv.h"
#include "drv_sync.h"
#include "drv_sync_ioctl.h"
#include "drv_sync_reg.h"
#include "drv_disp_ext.h"
#include "mt_drv_disp.h"
#include "mt_drv_module.h"
#include "mt_unf_sound.h"
#include "mt_audio_codec.h"
#include "vo_fw.h"
#include "drv_disp_Symphony6_reg.h"

#if defined(CONFIG_MT_CHIP_ARIA)
#include <mach/aria_io.h>
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#include "mt_mach/irq.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

extern SYNC_GLOBAL_STATE_S    g_SyncGlobalState;
avsync_priv_t                *g_p_avsync_private = NULL;
avsync_recorder_queue         g_vid_frame_info_queue;
MT_BOOL                          g_vid_frame_info_cap_break = 0;

mt_void SYNC_ApplyConfig(SYNC_S *pSync);
mt_s32 avsync_stc_pause(void);
mt_s32 avsync_stc_resume(void);
mt_s32 avsync_set_sync_mode(avsync_priv_t *priv, avsync_sync_mode_e sync_mode);
mt_s32 avsync_set_user_sync_mode(avsync_priv_t *priv, avsync_sync_mode_e sync_mode);
mt_void avsync_get_current_psync(SYNC_S *pSync);
void avsync_pcr_set_new_pcr(avsync_priv_t *priv, mt_u32 pcr, mt_u64 cpu_tick);
MT_BOOL avsync_pcr_reliable_check(avsync_priv_t *priv, mt_u32 pcr);
mt_s32 avsync_pcr_switch_to_audio_master(avsync_priv_t *priv);
mt_s32 avsync_audio_adjust_apts_for_ddp_verfication(avsync_priv_t *priv, u32 *pts);
mt_s32 avsync_audio_adjust_apts_for_ddp_verfication_no_hdmi(avsync_priv_t *priv, u32 *pts);

mt_s32 avsync_resume_sync_mode(avsync_priv_t *priv);
mt_s32 avsync_module_default_cfg(avsync_priv_t *priv);
mt_s32 avsync_int_register(void);
mt_s32 avsync_int_unregister(void);
mt_s32 avsync_is_ddp_verfication(void *private);
mt_s32 avsync_video_sync_status_reset(avsync_priv_t *priv);
mt_s32 avsync_audio_sync_status_reset(avsync_priv_t *priv);
static mt_s32 avsync_video_frame_info_capture_write(mt_u32 frame_pts, mt_u32 frame_id);
mt_s32 avsync_is_ddp_atmos(void *private, MT_HA_AUDIO_STREAM_INFO_S *info);
mt_s32 avsync_get_ddp_prog(avsync_priv_t *priv);
extern mt_s32 dmx_get_audio_es_buf_id(mt_u32 *buf_id);
extern mt_s32 DMXOsiChnEsDescBufGetAddrSize(mt_u32 Id, ulong *EsAddr, ulong *EsSize, ulong *DescAddr, ulong *DescSize);
extern mt_void DMXOsiChnEsDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead);
extern mt_void DMXOsiChnEsDescDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead);
extern mt_s32 AO_Track_GetAudioStreamInfo(MT_HA_AUDIO_STREAM_INFO_S *pInfo);
mt_s32 is_liveplay(void);
extern u32 avsync_get_ao_data_time(void);

mt_spts SYNC_GetSysTime(mt_void)
{
    mt_u64   SysTime;

    SysTime = sched_clock();

#if 1
#if MT_PTS_USE_64_US
    do_div(SysTime, 1000);
#else
    do_div(SysTime, 1000000);
#endif
#endif

    return (mt_upts)SysTime;
}

mt_upts SYNC_GetSysTimeCost(mt_upts LastSysTime)
{
    mt_upts   CrtSysTime;
    mt_upts   Delta;

    CrtSysTime = SYNC_GetSysTime();

    if (CrtSysTime > LastSysTime)
    {
        Delta = CrtSysTime - LastSysTime;
    }
    else
    {
        Delta = (SYS_TIME_MAX - LastSysTime) + 1 + CrtSysTime;
    }

    return Delta;
}


mt_spts SYNC_GetLocalTime(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    mt_upts        CrtLocaltime = -1;
    mt_upts        CostSysTime;

    /*In TPLAY Mode, we need video channel localtime, ignore others channel*/
    if ((SYNC_STATUS_TPLAY == pSync->CrtStatus) && (SYNC_CHAN_VID != enChn))
    {
        return -1;
    }

    if (SYNC_CHAN_AUD == enChn)
    {
        if (!pSync->AudLocalTimeFlag)
        {
            return -1;
        }

        CostSysTime = SYNC_GetSysTimeCost(pSync->AudLastSysTime);
        CrtLocaltime = pSync->AudLastLocalTime + CostSysTime;
    }
    else if (SYNC_CHAN_VID == enChn)
    {
        if (!pSync->VidLocalTimeFlag)
        {
            return -1;
        }

        CostSysTime = SYNC_GetSysTimeCost(pSync->VidLastSysTime);

        CrtLocaltime = pSync->VidLastLocalTime + CostSysTime;
    }
    else if (SYNC_CHAN_PCR == enChn)
    {
        if(((!pSync->PcrSyncInfo.PcrFirstCome)
            && (SYNC_SCR_ADJUST_BUTT == pSync->PcrSyncInfo.enPcrAdjust)))
        {
            return -1;
        }

        if (!pSync->PcrSyncInfo.PcrLocalTimeFlag)
        {
            return -1;
        }

        CostSysTime = SYNC_GetSysTimeCost(pSync->PcrSyncInfo.PcrLastSysTime);

        /*adjust the diff between pcr and sys time*//*CNCommont: 校正PCR与本地时钟不匹配引起的误差*/
        if (pSync->PcrSyncInfo.PcrGradient <= 101 && pSync->PcrSyncInfo.PcrGradient >= 99)
        {
#if MT_PTS_USE_64_US
	CostSysTime= div64_ul(CostSysTime * 100, pSync->PcrSyncInfo.PcrGradient);
#else
          CostSysTime = CostSysTime * 100 / pSync->PcrSyncInfo.PcrGradient;
#endif
        }

        CrtLocaltime = pSync->PcrSyncInfo.PcrLastLocalTime + CostSysTime;
    }

#ifdef MT_AVPLAY_SCR_SUPPORT
    else if (SYNC_CHAN_SCR == enChn)
    {
        if( !pSync->ScrInitFlag)
        {
            return -1;
        }

        CostSysTime = SYNC_GetSysTimeCost(pSync->ScrLastSysTime);

        CrtLocaltime = pSync->ScrLastLocalTime + CostSysTime;
    }
#endif

    if (CrtLocaltime > PCR_TIME_MAX)
    {
        CrtLocaltime -= PCR_TIME_MAX;
    }

    return CrtLocaltime;
}

mt_void SYNC_SetLocalTime(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_upts LocalTime)
{
    if (SYNC_CHAN_AUD == enChn)
    {
        pSync->AudLastSysTime = SYNC_GetSysTime();
        pSync->AudLastLocalTime = LocalTime;
        pSync->AudLocalTimeFlag = MT_TRUE;
    }
    else if (SYNC_CHAN_VID == enChn)
    {
        pSync->VidLastSysTime = SYNC_GetSysTime();
        pSync->VidLastLocalTime = LocalTime;
        pSync->VidLocalTimeFlag = MT_TRUE;
    }
    else if (SYNC_CHAN_PCR == enChn)
    {
        pSync->PcrSyncInfo.PcrLastSysTime = SYNC_GetSysTime();
        pSync->PcrSyncInfo.PcrLastLocalTime = LocalTime;
        pSync->PcrSyncInfo.PcrLocalTimeFlag = MT_TRUE;
    }
#ifdef MT_AVPLAY_SCR_SUPPORT
    else if (SYNC_CHAN_SCR == enChn)
    {
        pSync->ScrLastSysTime = SYNC_GetSysTime();
        pSync->ScrLastLocalTime = LocalTime;
    }
#endif

    return;
}

MT_BOOL SYNC_VerifyHandle(mt_handle hSync)
{
    if ((hSync == -1) || ((hSync & 0xff) >= SYNC_MAX_NUM))
    {
        MT_ERR_SYNC("%s: hSync == -1 or hSync&0xff >= 16 !\n", __FUNCTION__);
        return MT_FALSE;
    }

    if (MT_NULL == g_SyncGlobalState.SyncInfo[hSync & 0xff].pSync)
    {
        MT_ERR_SYNC("this is invalid handle.\n");
        return MT_FALSE;
    }

    return MT_TRUE;
}

mt_s32 SYNC_StartSync(mt_u32 SyncId)
{
    SYNC_S  *pSync;

    pSync = g_SyncGlobalState.SyncInfo[SyncId].pSync;

    pSync->PreSyncStartSysTime = SYNC_GetSysTime();

    pSync->PcrSyncInfo.PcrSyncStartSysTime =  pSync->PreSyncStartSysTime ;

    /* if timeout of presync is zero or sync adjust is disabled, disable presync*/
    if ((!pSync->SyncAttr.u32PreSyncTimeoutMs) ||
        (MT_UNF_SYNC_REF_NONE == pSync->SyncAttr.enSyncRef))
    {
        pSync->PreSyncEndSysTime = pSync->PreSyncStartSysTime;
        pSync->PreSyncFinish = MT_TRUE;
        pSync->BufFundFinish = MT_TRUE;
    }

    /* no sync,then no buffund and audio resync*/
    if (MT_UNF_SYNC_REF_NONE == pSync->SyncAttr.enSyncRef)
    {
        pSync->AudReSync = MT_FALSE;
        pSync->AudReBufFund = MT_FALSE;
    }

	SYNC_ApplyConfig(pSync);
    return MT_SUCCESS;
}

mt_s32 SYNC_PauseSync(mt_u32 SyncId)
{
    SYNC_S  *pSync;

    pSync = g_SyncGlobalState.SyncInfo[SyncId].pSync;

    pSync->PcrSyncInfo.PcrPauseLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_PCR);
    pSync->AudPauseLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_AUD);
    pSync->VidPauseLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_VID);
#ifdef MT_AVPLAY_SCR_SUPPORT
    pSync->ScrPauseLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_SCR);
#endif

    avsync_stc_pause();
    return MT_SUCCESS;
}

mt_s32 SYNC_ResumeSync(mt_u32 SyncId)
{
    SYNC_S  *pSync;

    pSync = g_SyncGlobalState.SyncInfo[SyncId].pSync;

    /* if resume after reset, the local time won't be reset */
    if (pSync->PcrSyncInfo.PcrPauseLocalTime != -1)
    {
        SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR, pSync->PcrSyncInfo.PcrPauseLocalTime);
    }

    if (pSync->AudPauseLocalTime != -1)
    {
        SYNC_SetLocalTime(pSync, SYNC_CHAN_AUD, pSync->AudPauseLocalTime);
    }

    if (pSync->VidPauseLocalTime != -1)
    {
        SYNC_SetLocalTime(pSync, SYNC_CHAN_VID, pSync->VidPauseLocalTime);
    }
#ifdef MT_AVPLAY_SCR_SUPPORT
    if (pSync->ScrPauseLocalTime != -1)
    {
        SYNC_SetLocalTime(pSync, SYNC_CHAN_SCR, pSync->ScrPauseLocalTime);
    }
#endif

    avsync_stc_resume();
    return MT_SUCCESS;
}

mt_s32 SYNC_GetTime(mt_u32 SyncId, mt_upts *pLocalTime, mt_upts *pPlayTime)
{
    SYNC_S      *pSync;
    mt_upts      AudLocalTime;
    mt_upts      VidLocalTime;
    mt_upts      PcrLocalTime;

    pSync = g_SyncGlobalState.SyncInfo[SyncId].pSync;

    if (SYNC_STATUS_PAUSE == pSync->CrtStatus)
    {
        AudLocalTime = pSync->AudPauseLocalTime;
        VidLocalTime = pSync->VidPauseLocalTime;
        PcrLocalTime = pSync->PcrSyncInfo.PcrPauseLocalTime;
    }
    else
    {
        AudLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_AUD);
        VidLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_VID);
        PcrLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_PCR);
    }

    if (AudLocalTime <= PCR_TIME_MAX)
    {
        *pLocalTime = AudLocalTime;
        *pPlayTime = abs((mt_s32)(AudLocalTime - pSync->AudFirstValidPts));
    }
    else if (VidLocalTime <= PCR_TIME_MAX)
    {
        *pLocalTime = VidLocalTime;
        *pPlayTime = abs((mt_s32)(VidLocalTime - pSync->VidFirstValidPts));
    }
    else if(PcrLocalTime <= PCR_TIME_MAX)
    {
        *pLocalTime = PcrLocalTime;
        *pPlayTime = abs((mt_s32)(PcrLocalTime - pSync->PcrSyncInfo.PcrFirst));
    }
    else
    {
        *pLocalTime = -1;
        *pPlayTime = 0;
    }

    return MT_SUCCESS;
}

mt_void SYNC_CalcDiffTime(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    mt_upts             CurSysTime;
    mt_upts             PcrLocalTime;
    mt_upts             AudLocalTime;
    mt_upts             VidLocalTime;
    mt_spts             AudPcrDiff;
    mt_spts             VidPcrDiff;
    mt_spts             VidAudDiff;
    mt_spts             DefaultDiff;

    mt_upts             ScrLocalTime;
    mt_spts             AudScrDiff;
    mt_spts             VidScrDiff;

    CurSysTime = SYNC_GetSysTime();

    PcrLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_PCR);
    AudLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_AUD);
    VidLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_VID);
    ScrLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_SCR);

#if MT_PTS_USE_64_US
    DefaultDiff = (pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime + pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime) >>1 ;

    AudPcrDiff = AudLocalTime - PcrLocalTime + pSync->SyncAttr.s32AudPtsAdjust * 1000;
    VidPcrDiff = VidLocalTime - PcrLocalTime + pSync->SyncAttr.s32VidPtsAdjust * 1000;
    VidAudDiff = VidLocalTime - AudLocalTime + (pSync->SyncAttr.s32VidPtsAdjust  - pSync->SyncAttr.s32AudPtsAdjust) * 1000;

    AudScrDiff = AudLocalTime - ScrLocalTime + pSync->SyncAttr.s32AudPtsAdjust * 1000;
    VidScrDiff = VidLocalTime - ScrLocalTime + pSync->SyncAttr.s32VidPtsAdjust * 1000;
#else
    DefaultDiff = (pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime + pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
    >> 1;

    AudPcrDiff = AudLocalTime - PcrLocalTime + pSync->SyncAttr.s32AudPtsAdjust;
    VidPcrDiff = VidLocalTime - PcrLocalTime + pSync->SyncAttr.s32VidPtsAdjust;
    VidAudDiff = VidLocalTime - AudLocalTime + pSync->SyncAttr.s32VidPtsAdjust - pSync->SyncAttr.s32AudPtsAdjust;

    AudScrDiff = AudLocalTime - ScrLocalTime + pSync->SyncAttr.s32AudPtsAdjust;
    VidScrDiff = VidLocalTime - ScrLocalTime + pSync->SyncAttr.s32VidPtsAdjust;
#endif

    if(enChn == SYNC_CHAN_VID && pSync->VidInfo.Pts != pSync->VidLastPts)
    {
       //MT_INFO_SYNC("[%x] [%x][%x] [%x][%x] [%x][%x] [%x][%x] [%x][%x]\n", pSync->SyncAttr.enSyncRef, PcrLocalTime, ScrLocalTime, AudLocalTime, VidLocalTime, pSync->AudInfo.Pts, pSync->VidInfo.Pts, pSync->SyncAttr.s32AudPtsAdjust, pSync->SyncAttr.s32VidPtsAdjust, pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime,  pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime);
       //MT_INFO_SYNC("[%x] [%x][%x] [%x][%x] [%x][%x] [%x][%x][%x] [%x][%x]\n", pSync->SyncAttr.enSyncRef, PcrLocalTime, ScrLocalTime, AudLocalTime, VidLocalTime, pSync->AudInfo.Pts, pSync->VidInfo.Pts, pSync->VidOpt.SyncProc, pSync->VidOpt.Repeat, pSync->VidOpt.Discard, pSync->VidOpt.VdecDiscardTime, pSync->AudOpt.SyncProc, pSync->AudOpt.SpeedAdjust);

       //MT_INFO_SYNC("[%x] [%x][%x] [%x][%x] [%x][%x] [%x][%x] [%x][%x] [%x]\n", pSync->SyncAttr.enSyncRef, PcrLocalTime, ScrLocalTime, AudLocalTime, VidLocalTime, pSync->AudInfo.Pts, pSync->VidInfo.Pts, pSync->CrtBufStatus.AudBufPercent, pSync->CrtBufStatus.VidBufPercent, pSync->CrtBufStatus.AudBufState, pSync->CrtBufStatus.VidBufState, pSync->CrtBufStatus.bOverflowDiscFrm);
    }

    if(MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
    {
        if (SYNC_AUD_ADJUST_SCR != pSync->PcrSyncInfo.enPcrAdjust)
        {
            if ((-1 == PcrLocalTime) ||  (-1 == AudLocalTime) ||  (-1 == VidLocalTime))
            {
                AudPcrDiff = 0;
                VidPcrDiff = 0;
                VidAudDiff = 0;
            }
        }
        else
        {
            if ((-1 == AudLocalTime) || (-1 == VidLocalTime))
            {
                VidAudDiff = DefaultDiff;
            }
        }
    }
    else if(MT_UNF_SYNC_REF_AUDIO== pSync->SyncAttr.enSyncRef)
    {
        if ((-1 == AudLocalTime) || (-1 == VidLocalTime))
        {
            VidAudDiff = DefaultDiff;
        }
    }
    else if(MT_UNF_SYNC_REF_SCR== pSync->SyncAttr.enSyncRef)
    {
        if (-1 == ScrLocalTime)
        {
            VidScrDiff = DefaultDiff;
            AudScrDiff = DefaultDiff;
        }
    }

    if(-1 != ScrLocalTime)
    {
        SYNC_SetLocalTime(pSync, SYNC_CHAN_SCR, ScrLocalTime);
    }

    /* if AudScrDiff and VidScrDiff are both too large, we reinit scr */
    if ((abs(AudScrDiff) > SCR_DISCARD_THRESHOLD) && (abs(VidScrDiff) > SCR_DISCARD_THRESHOLD)
       )
    {
        pSync->ScrInitFlag = MT_FALSE;
    }

    MT_INFO_VSYNC(enChn, "SysTime %d Aud LastSysTime %d Aud LocatTime %d Aud LstPts %d, Aud LstBufTime %3d Vid LocatTime %d Vid LstPts %d VidAudDiff %d\n",
                      CurSysTime,pSync->AudLastSysTime,AudLocalTime, pSync->AudLastPts, pSync->AudLastBufTime,VidLocalTime, pSync->VidLastPts, VidAudDiff);

    MT_INFO_ASYNC(enChn, "SysTime %d Aud LastSysTime %d Aud LocatTime %d Aud LstPts %d, Aud LstBufTime %3d Vid LocatTime %d Vid LstPts %d VidAudDiff %d\n",
    CurSysTime,pSync->AudLastSysTime,AudLocalTime, pSync->AudLastPts, pSync->AudLastBufTime,VidLocalTime, pSync->VidLastPts, VidAudDiff);

    if(MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
    {
        MT_INFO_VSYNC(enChn, ">>>>PcrLocalTime %d  AudPcrDiff %d VidPcrDiff %d VidAudDiff %d\n",
                          PcrLocalTime, AudPcrDiff, VidPcrDiff, VidAudDiff);
    }

#ifdef MT_AVPLAY_SCR_SUPPORT
    if(MT_UNF_SYNC_REF_SCR == pSync->SyncAttr.enSyncRef)
    {
        MT_INFO_VSYNC(enChn, "ScrLocalTime %d  AudScrDiff %d VidScrDiff %d VidAudDiff %d\n",
                          ScrLocalTime, AudScrDiff, VidScrDiff, VidAudDiff);
    }
#endif

    pSync->PcrSyncInfo.LastAudPcrDiff = pSync->PcrSyncInfo.AudPcrDiff;
    pSync->PcrSyncInfo.LastVidPcrDiff = pSync->PcrSyncInfo.VidPcrDiff;
    pSync->PcrSyncInfo.AudPcrDiff = AudPcrDiff;
    pSync->PcrSyncInfo.VidPcrDiff = VidPcrDiff;

    pSync->LastVidAudDiff = pSync->VidAudDiff;
    pSync->VidAudDiff = VidAudDiff;

    pSync->AudScrDiff = AudScrDiff;
    pSync->VidScrDiff = VidScrDiff;

    return;
}

mt_void SYNC_PreSyncTargetInit(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    if ((SYNC_CHAN_VID == enChn) && (!pSync->VidPreSyncTargetInit))
    {
        if (-1 == pSync->VidInfo.Pts)
        {
            MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync VidFrame SrcPts = -1\n");
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

            return;
        }
        else
        {
            pSync->VidPreSyncTargetTime = pSync->VidInfo.Pts;
            pSync->VidPreSyncTargetInit = MT_TRUE;
        }
    }

    if ((SYNC_CHAN_AUD == enChn) && (!pSync->AudPreSyncTargetInit))
    {
        if (-1 == pSync->AudInfo.Pts)
        {
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync AudFrame SrcPts = -1\n");
            pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

            return;
        }
        else
        {
            pSync->AudPreSyncTargetTime = pSync->AudInfo.Pts;
            pSync->AudPreSyncTargetInit = MT_TRUE;
        }
    }

    if (pSync->VidPreSyncTargetInit && pSync->AudPreSyncTargetInit)
    {
        if (pSync->VidPreSyncTargetTime > pSync->AudPreSyncTargetTime)
        {
            if (pSync->UseExtPreSyncTaget
            && pSync->ExtPreSyncTagetTime > pSync->VidPreSyncTargetTime)
            {
                pSync->PreSyncTarget = SYNC_CHAN_EXT;
                pSync->PreSyncTargetTime = pSync->ExtPreSyncTagetTime;
            }
            else
            {
                pSync->PreSyncTarget = SYNC_CHAN_VID;
                pSync->PreSyncTargetTime = pSync->VidPreSyncTargetTime;
            }
        }
        else
        {
            if (pSync->UseExtPreSyncTaget
              &&(pSync->ExtPreSyncTagetTime > pSync->AudPreSyncTargetTime)
               )
            {
                pSync->PreSyncTarget = SYNC_CHAN_EXT;
                pSync->PreSyncTargetTime = pSync->ExtPreSyncTagetTime;
            }
            else
            {
                pSync->PreSyncTarget = SYNC_CHAN_AUD;
                pSync->PreSyncTargetTime = pSync->AudPreSyncTargetTime;
            }
        }

        pSync->UseExtPreSyncTaget = MT_FALSE;

        pSync->PreSyncTargetInit = MT_TRUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync Target Init %d Vid pts %d Aud pts %d\n", pSync->PreSyncTarget, pSync->VidPreSyncTargetTime, pSync->AudPreSyncTargetTime);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync Target Init %d Vid pts %d Aud pts %d\n", pSync->PreSyncTarget, pSync->VidPreSyncTargetTime, pSync->AudPreSyncTargetTime);
    }
    else
    {
        if (SYNC_CHAN_VID == enChn)
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
            //MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync Target UnInit AudPreSyncTargetInit %d VidPreSyncTargetInit %d\n", pSync->AudPreSyncTargetInit, pSync->VidPreSyncTargetInit);
        }
        else if(SYNC_CHAN_AUD == enChn)
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
            //MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync Target UnInit AudPreSyncTargetInit %d VidPreSyncTargetInit %d\n", pSync->AudPreSyncTargetInit, pSync->VidPreSyncTargetInit);
        }

        return;
    }

    return;
}

MT_BOOL SYNC_CheckPcrTimeout(SYNC_S *pSync)
{
    mt_u32      PcrCostSysTime;
    PcrCostSysTime = SYNC_GetSysTimeCost(pSync->PcrSyncInfo.PcrSyncStartSysTime);
    return (PcrCostSysTime >= PCR_TIMEOUTMS) ? MT_TRUE : MT_FALSE;
}


MT_BOOL SYNC_CheckAudTimeout(SYNC_S *pSync)
{
    mt_u32      AudCostSysTime;
    AudCostSysTime = SYNC_GetSysTimeCost(pSync->PcrSyncInfo.PcrSyncStartSysTime);
    return (AudCostSysTime >= AUD_TIMEOUTMS) ? MT_TRUE : MT_FALSE;
}



mt_void SYNC_PreSync(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    mt_upts      CostSysTime;
    mt_spts      VidAudDiff;
    mt_spts      AudPcrDiff;
    mt_spts      VidPcrDiff;
    mt_upts      PcrLocalTime;
    mt_spts      VidTargetDiff, AudTargetDiff;

    CostSysTime = SYNC_GetSysTimeCost(pSync->PreSyncStartSysTime);

    /* do not do presync if video or audio is disable */
    if ( (!pSync->AudEnable) || (!pSync->VidEnable) )
    {
        pSync->PreSyncEndSysTime = SYNC_GetSysTime();
        pSync->PreSyncFinish = MT_TRUE;
        pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        mt_drv_stat_event(STAT_EVENT_PRESYNC,0);

        return;
    }

    /* presync timeout*/
    if (CostSysTime >= pSync->SyncAttr.u32PreSyncTimeoutMs)
    {
        pSync->PreSyncEndSysTime = SYNC_GetSysTime();
        pSync->PreSyncFinish = MT_TRUE;
        pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync TimeOut %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync TimeOut %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
        mt_drv_stat_event(STAT_EVENT_PRESYNC,0);

        return;
    }

    /* video buffer or audio buffer will be blocked*/
    if ((SYNC_BUF_STATE_HIGH == pSync->CrtBufStatus.VidBufState)
        || (SYNC_BUF_STATE_HIGH == pSync->CrtBufStatus.AudBufState)
        )
    {
        pSync->PreSyncEndSysTime = SYNC_GetSysTime();
        pSync->PreSyncFinish = MT_TRUE;
        pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync BufBlock Aud %d Vid %d\n", pSync->CrtBufStatus.AudBufState, pSync->CrtBufStatus.VidBufState);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync BufBlock Aud %d Vid %d\n", pSync->CrtBufStatus.AudBufState, pSync->CrtBufStatus.VidBufState);
        mt_drv_stat_event(STAT_EVENT_PRESYNC,1);

        return;
    }

    /* prepare presync target*/
    if (!pSync->PreSyncTargetInit)
    {
        SYNC_PreSyncTargetInit(pSync, enChn);

        /* presync target is not ready*/
        if (!pSync->PreSyncTargetInit)
        {
            return;
        }
    }

	if(SYNC_CHAN_EXT == pSync->PreSyncTarget)
    {
        if ((SYNC_CHAN_VID == enChn) || (SYNC_CHAN_AUD == enChn))
        {
            VidTargetDiff = pSync->VidInfo.Pts - pSync->VidInfo.DelayTime - pSync->PreSyncTargetTime;
            AudTargetDiff = pSync->AudInfo.Pts - pSync->PreSyncTargetTime;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidTargetDiff %d, AudTargetDiff %d\n", VidTargetDiff, AudTargetDiff);
            MT_INFO_VSYNC(SYNC_CHAN_AUD, "VidTargetDiff %d, AudTargetDiff %d\n", VidTargetDiff, AudTargetDiff);

            if(VidTargetDiff > pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime
            && AudTargetDiff > pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
            {
                pSync->PreSyncEndSysTime = SYNC_GetSysTime();
                pSync->PreSyncFinish = MT_TRUE;
                pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
                pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
                mt_drv_stat_event(STAT_EVENT_PRESYNC,0);
                return;
            }
            else
            {
                if(VidTargetDiff < pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                }

                if(AudTargetDiff < pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
                {
                    pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;
                }
                else
                {
                    pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
                }
            }
        }
    }
    else if (enChn == pSync->PreSyncTarget)
    {
        if (SYNC_CHAN_VID == enChn)
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
            VidAudDiff = pSync->VidPreSyncTargetTime - pSync->AudInfo.Pts;
            pSync->VidAudDiff = VidAudDiff;
        }
        else
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
            VidAudDiff = pSync->VidInfo.Pts - pSync->AudPreSyncTargetTime;
            pSync->VidAudDiff = VidAudDiff;
        }
    }
    else
    {
        /* audio wait for video*/
        if (SYNC_CHAN_VID == enChn)
        {
            VidAudDiff = pSync->VidInfo.Pts - pSync->AudPreSyncTargetTime;
            pSync->VidAudDiff = VidAudDiff;

            /* the difference between video and audio is too large*/
            if (VidAudDiff < (-VID_LAG_DISCARD_THRESHOLD))
            {
                pSync->PreSyncEndSysTime = SYNC_GetSysTime();
                pSync->PreSyncFinish = MT_TRUE;
                pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
                MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync Giveup VidAudDiff %d > VID_LAG_DISCARD_THRESHOLD %d\n", VidAudDiff,VID_LAG_DISCARD_THRESHOLD);
                mt_drv_stat_event(STAT_EVENT_PRESYNC,0);
            }
            else
            {
                mt_s32 MaxWinDelay;

                if (pSync->VidInfo.DispRate == 0)
                {
                    MaxWinDelay = 40;
                }
                else
                {
                    MaxWinDelay = 2 * 1000 * 100 / pSync->VidInfo.DispRate;
                }

                if (VidAudDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime + MaxWinDelay)
                {
                    pSync->PreSyncEndSysTime = SYNC_GetSysTime();
                    pSync->PreSyncFinish = MT_TRUE;

                    if (MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
                    {
                        if (SYNC_PCR_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
                        {
                            PcrLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_PCR);

                            AudPcrDiff = pSync->AudPreSyncTargetTime - PcrLocalTime;
                            VidPcrDiff = pSync->VidInfo.Pts - PcrLocalTime;

                             //adjust pcr to this one which is more behind
                             pSync->PcrSyncInfo.PcrDelta += (VidAudDiff > 0) ? AudPcrDiff : VidPcrDiff;

                             MT_INFO_SYNC("adjust pcr, PcrDelta %d\n", pSync->PcrSyncInfo.PcrDelta);
                         }
                         else
                         {
                             pSync->PcrSyncInfo.enPcrAdjust = SYNC_AUD_ADJUST_SCR;    //pcr timeout or pcr doesn't come
                         }
                    }

                    pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
                    MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync Ok VidAudDiff %d\n", VidAudDiff);
                    MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync Ok VidAudDiff %d\n", VidAudDiff);
                    mt_drv_stat_event(STAT_EVENT_PRESYNC,0);
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
                }
            }
        }
        /* video wait for audio*/
        else
        {
            VidAudDiff = pSync->VidPreSyncTargetTime - pSync->AudInfo.Pts;
            pSync->VidAudDiff = VidAudDiff;

            /* the difference between video and audio is too large*/
            if (VidAudDiff > VID_LEAD_DISCARD_THRESHOLD)
            {
                pSync->PreSyncEndSysTime = SYNC_GetSysTime();
                pSync->PreSyncFinish = MT_TRUE;
                pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
                MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync VidAudDiff %d > u32PreSyncTimeoutMs %d\n", VidAudDiff, pSync->SyncAttr.u32PreSyncTimeoutMs);
                MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync VidAudDiff %d > u32PreSyncTimeoutMs %d\n", VidAudDiff, pSync->SyncAttr.u32PreSyncTimeoutMs);
                mt_drv_stat_event(STAT_EVENT_PRESYNC,0);
            }
            else
            {
                if (VidAudDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
                {
                    pSync->PreSyncEndSysTime = SYNC_GetSysTime();
                    pSync->PreSyncFinish = MT_TRUE;

                    if (MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
                    {
                        if (SYNC_PCR_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
                        {
                            PcrLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_PCR);

                            AudPcrDiff = pSync->AudPreSyncTargetTime - PcrLocalTime;
                            VidPcrDiff = pSync->VidInfo.Pts - PcrLocalTime;

                             //adjust pcr to this one which is more behind
                             pSync->PcrSyncInfo.PcrDelta += (VidAudDiff > 0) ? AudPcrDiff : VidPcrDiff;

                             MT_INFO_SYNC("adjust pcr, PcrDelta %d\n", pSync->PcrSyncInfo.PcrDelta);
                         }
                         else
                         {
                             pSync->PcrSyncInfo.enPcrAdjust = SYNC_AUD_ADJUST_SCR;    //pcr timeout or pcr doesn't come
                         }
                    }

                    pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
                    MT_INFO_ASYNC(SYNC_CHAN_AUD, "PreSync Ok VidAudDiff %d\n", VidAudDiff);
                    MT_INFO_VSYNC(SYNC_CHAN_VID, "PreSync Ok VidAudDiff %d\n", VidAudDiff);
                    mt_drv_stat_event(STAT_EVENT_PRESYNC,0);
                }
                else
                {
                    pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;
                }
            }
        }
    }

    return;
}

mt_void SYNC_BufFund(SYNC_S *pSync)
{
    mt_u32        CostSysTime;

    CostSysTime = SYNC_GetSysTimeCost(pSync->PreSyncEndSysTime);

    /* cumulation timeout */
    if (CostSysTime > BUF_FUND_TIMEOUT)
    {
        pSync->BufFundEndSysTime = SYNC_GetSysTime();
        pSync->BufFundFinish = MT_TRUE;
        pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        mt_drv_stat_event(STAT_EVENT_BUFREADY,2);
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "BufFund TimeOut %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "BufFund TimeOut %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);

        return;
    }

    /* video or audio buffer will be blocked*/
    if ((SYNC_BUF_STATE_HIGH == pSync->CrtBufStatus.VidBufState)
        || (SYNC_BUF_STATE_HIGH == pSync->CrtBufStatus.AudBufState)
        )
    {
        pSync->BufFundEndSysTime = SYNC_GetSysTime();
        pSync->BufFundFinish = MT_TRUE;
        pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        mt_drv_stat_event(STAT_EVENT_BUFREADY,3);

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "BufFund BufBlock Aud %d Vid %d\n", pSync->CrtBufStatus.AudBufState, pSync->CrtBufStatus.VidBufState);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "BufFund BufBlock Aud %d Vid %d\n", pSync->CrtBufStatus.AudBufState, pSync->CrtBufStatus.VidBufState);

        return;
    }

    if (pSync->VidEnable && pSync->AudEnable)
    {
        if (pSync->AudInfo.BufTime + pSync->AudInfo.FrameTime >= AO_TRACK_AIP_START_LATENCYMS)
        {
            pSync->BufFundEndSysTime = SYNC_GetSysTime();
            pSync->BufFundFinish = MT_TRUE;
            pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
            pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

            mt_drv_stat_event(STAT_EVENT_BUFREADY,1);
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "BufFund Ok %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
            MT_INFO_VSYNC(SYNC_CHAN_VID, "BufFund Ok %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

            if (MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
            {
                 if ((MT_FALSE == pSync->VidFirstCome) && (MT_FALSE== SYNC_CheckAudTimeout(pSync)))
                 {
                     pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
                 }
            }

            MT_INFO_ASYNC(SYNC_CHAN_AUD, "BufFund %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
            MT_INFO_VSYNC(SYNC_CHAN_VID, "BufFund %d AudBufTime %d AudFrameNum %d VidDelayTime %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum, pSync->VidInfo.DelayTime);
        }
    }
    else if (pSync->VidEnable && (!pSync->AudEnable))
    {

        pSync->BufFundEndSysTime = SYNC_GetSysTime();
        pSync->BufFundFinish = MT_TRUE;
        pSync->VidOpt.SyncProc = SYNC_PROC_CONTINUE;
        mt_drv_stat_event(STAT_EVENT_BUFREADY,1);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "BufFund Ok\n");
    }
    else if ((!pSync->VidEnable) && pSync->AudEnable)
    {
        if (pSync->AudInfo.BufTime + pSync->AudInfo.FrameTime >= AO_TRACK_AIP_START_LATENCYMS)
        {
            pSync->BufFundEndSysTime = SYNC_GetSysTime();
            pSync->BufFundFinish = MT_TRUE;
            pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
            mt_drv_stat_event(STAT_EVENT_BUFREADY,1);
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "BufFund Ok\n");
        }
        else
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
        }
    }

    return;
}

mt_void SYNC_AudReSync(SYNC_S *pSync)
{
    mt_upts             VidLocalTime;
    mt_spts             VidAudDiff;
    mt_upts               CostSysTime;

    CostSysTime = SYNC_GetSysTimeCost(pSync->PreSyncStartSysTime);

    /* resync timeout */
    if (CostSysTime >= AUD_RESYNC_TIMEOUT)
    {
        pSync->PreSyncEndSysTime = SYNC_GetSysTime();
        pSync->AudReSync = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync TimeOut %d BufTime %d FrameNum %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum);
        return;
    }

    /* buffer blocked */
    if (SYNC_BUF_STATE_HIGH == pSync->CrtBufStatus.AudBufState)
    {
        pSync->PreSyncEndSysTime = SYNC_GetSysTime();
        pSync->AudReSync = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync BufBlock\n");
        return;
    }

    /* discard the frame if pts is -1 */
    if (-1 == pSync->AudInfo.Pts)
    {
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync AudFrame Pts = -1\n");
        pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

        return;
    }

    VidLocalTime = SYNC_GetLocalTime(pSync, SYNC_CHAN_VID);

    VidAudDiff = VidLocalTime - pSync->AudInfo.Pts + AO_TRACK_AIP_START_LATENCYMS;

    /* The difference is too large */
    if (abs(VidAudDiff) > AUD_RESYNC_ADJUST_THRESHOLD)
    {
        pSync->AudReSync = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync VidAudDiff %d > AUD_RESYNC_ADJUST_THRESHOLD %d\n", VidAudDiff, AUD_RESYNC_ADJUST_THRESHOLD);
        return;
    }

    /*adjust into sync start range*/
    if ((VidAudDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
      &&(VidAudDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
      &&(abs(VidAudDiff) <= pSync->AudInfo.FrameTime)
       )
    {
        pSync->PreSyncEndSysTime = SYNC_GetSysTime();
        pSync->AudReSync = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync Ok VidAudDiff %d\n", VidAudDiff);

        return;
    }

    if (VidAudDiff > 0)
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync DISCARD VidAudDiff %d\n", VidAudDiff);
    }
    else
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReSync REPEAT VidAudDiff %d\n", VidAudDiff);
    }

    return;
}

mt_void SYNC_AudReBufFund(SYNC_S *pSync)
{
    mt_u32        CostSysTime;

    CostSysTime = SYNC_GetSysTimeCost(pSync->PreSyncEndSysTime);

    /* timeout*/
    if (CostSysTime > BUF_FUND_TIMEOUT)
    {
        pSync->AudReBufFund = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudReBufFund TimeOut %d BufTime %d FrameNum %d\n", CostSysTime, pSync->AudInfo.BufTime, pSync->AudInfo.FrameNum);

        return;
    }

    /* buffer blocked */
    if (SYNC_BUF_STATE_HIGH == pSync->CrtBufStatus.AudBufState)
    {
        pSync->AudReBufFund = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, ("AudReBufFund BufBlock\n"));

        return;
    }

    if (pSync->AudInfo.BufTime + pSync->AudInfo.FrameTime >= AO_TRACK_AIP_START_LATENCYMS)
    {
        pSync->AudReBufFund = MT_FALSE;
        pSync->AudOpt.SyncProc = SYNC_PROC_CONTINUE;
        MT_INFO_VSYNC(SYNC_CHAN_AUD, "AudReBufFund Ok\n");
    }
    else
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
    }

    return;
}

mt_void SYNC_PcrSyncDiscardAud(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_NORMAL;

    pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

    pSync->AudDiscardCnt++;

    MT_INFO_ASYNC(enChn, ">>>>AudPcrDiff %d , Vid Lead Aud %d, discard\n", pSync->PcrSyncInfo.AudPcrDiff, VidAudDiff);

    return;
}

mt_void SYNC_PcrSyncRepeatAud(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

    if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.AudBufState)
    {
        #if 0
        if (SYNC_BUF_STATE_EMPTY == pSync->CrtBufStatus.AudBufState)
        {
             pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_MUTE_REPEAT;
        }
        else
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
        }
        #else
        pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
        #endif

        pSync->AudRepeatCnt++;
        MT_INFO_ASYNC(enChn, ">>>>AudPcrDiff %d , VidAudDiff: %d, Aud Repeat\n", pSync->PcrSyncInfo.AudPcrDiff, VidAudDiff);
    }

    return;
}

mt_void SYNC_PcrSyncDiscardVid(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    /* abs(VidPcrDiff) and abs(AudPcrDiff) must be less than PCR_LAG_STOP_THRESHOLD = 100
        so abs(VidAudDiff) largest 2*PCR_LAG_STOP_THRESHOLD = 200*/
    if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay)
    {
        /*discard one frame in every VID_SMOOTH_DISCARD_INTERVAL frame*/
        if (!(pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL))
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
            pSync->VidDiscardCnt++;
            MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lag Aud %d Smooth, Discard\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

            MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lag Aud %d Smooth, Play\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
        }

        if (SYNC_CHAN_VID == enChn)
        {
            pSync->VidDisPlayCnt++;
        }
    }
    else
    {
        /* discard time before vdec*/
        pSync->VidOpt.VdecDiscardTime = abs(VidAudDiff); //do what ? nouse
        pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
        pSync->VidDiscardCnt++;
        MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lag Aud %d, Discard\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
    }

    return;
}

mt_void SYNC_PcrSyncRepeatVid(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    /* there is space in video buffer */
    if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.VidBufState)
    {
        /* abs(VidPcrDiff) and abs(AudPcrDiff) must be less than PCR_LAG_STOP_THRESHOLD = 100
         so abs(VidAudDiff) largest 2*PCR_LAG_STOP_THRESHOLD = 200
        */
        {
            if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay || pSync->VidFirstPts == -1)
            {
                /*repeat one frame every VID_SMOOTH_REPEAT_INTERVAL frame*/
                if (!(pSync->VidRepPlayCnt % VID_SMOOTH_REPEAT_INTERVAL))
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                    pSync->VidRepeatCnt++;
                    MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lead Aud %d Smooth, Repeat\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                    MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lead Aud %d Smooth, Play\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
                }

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidRepPlayCnt++;
                }
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                pSync->VidRepeatCnt++;
                MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lead Aud %d, Repeat\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
            }
        }
    }
    /* the video buffer reach high waterline*/
    else
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

        MT_INFO_VSYNC(enChn, ">>>>VidPcrDiff %d, Vid Lead Pcr %d BufBlock, Play\n", pSync->PcrSyncInfo.VidPcrDiff, VidAudDiff);
    }

    return;
}

/* refer to pcr and audio is ahead of pcr*/
mt_void SYNC_PcrSyncAudLeadAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 AudPcrDiff)
{
    pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

    if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.AudBufState)
    {
        #if 0
        if (SYNC_BUF_STATE_EMPTY == pSync->CrtBufStatus.AudBufState)
        {
             pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_MUTE_REPEAT;
        }
        else
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
        }
        #else
            pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
        #endif
        pSync->AudRepeatCnt++;
        MT_INFO_ASYNC(enChn, ">>>>Pcr Lag Aud, AudPcrDiff %d, Aud Repeat\n", AudPcrDiff);
    }

    return;
}

/* refer to pcr and audio is behind pcr*/
mt_void SYNC_PcrSyncAudLagAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 AudPcrDiff)
{
    pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_NORMAL;

    pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

    pSync->AudDiscardCnt++;

    MT_INFO_ASYNC(enChn, ">>>>Pcr Lead Aud %d, discard\n", AudPcrDiff);

    return;
}

/* refer to pcr and video is ahead of pcr*/
mt_void SYNC_PcrSyncVidLeadAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidPcrDiff)
{
    /* there is space in video buffer */
    if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.VidBufState)
    {
        if (VidPcrDiff > pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
        {
            if (pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay || pSync->VidFirstPts == -1)
            {
                /* repeat one frame every VID_SMOOTH_REPEAT_INTERVAL frame*/
                if (!(pSync->VidRepPlayCnt % VID_SMOOTH_REPEAT_INTERVAL))
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                    pSync->VidRepeatCnt++;
                    MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d Smooth, Repeat\n", VidPcrDiff);
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                    MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d Smooth, Play\n", VidPcrDiff);
                }

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidRepPlayCnt++;
                }
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                pSync->VidRepeatCnt++;
                MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d, Repeat\n", VidPcrDiff);
            }
        }
        else
        {
            if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay || pSync->VidFirstPts == -1)
            {
                /*repeat one frame every VID_SMOOTH_REPEAT_INTERVAL frame*/
                if (!(pSync->VidRepPlayCnt % VID_SMOOTH_REPEAT_INTERVAL))
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                    pSync->VidRepeatCnt++;
                    MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d Smooth, Repeat\n", VidPcrDiff);
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                    MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d Smooth, Play\n", VidPcrDiff);
                }

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidRepPlayCnt++;
                }
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;
                pSync->VidRepeatCnt++;
                MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d, Repeat\n", VidPcrDiff);
            }
        }
    }
    /* the video buffer reach high waterline*/
    else
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

        MT_INFO_VSYNC(enChn, ">>>>Pcr Lag Vid %d BufBlock, Play\n", VidPcrDiff);
    }

    return;
}

/* refer to pcr and video is behind pcr*/
mt_void SYNC_PcrSyncVidLagAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidPcrDiff)
{
    if (VidPcrDiff < pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime)
    {
        if (pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay)
        {
            /*discard one frame in every VID_SMOOTH_DISCARD_INTERVAL frame*/
            if (!(pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL))
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
                pSync->VidDiscardCnt++;
                MT_INFO_VSYNC(enChn, ">>>>Pcr Lead Vid %d Smooth, Discard\n", VidPcrDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(enChn, ">>>>Pcr Lead Vid %d Smooth, Play\n", VidPcrDiff);
            }

            if (SYNC_CHAN_VID == enChn)
            {
                pSync->VidDisPlayCnt++;
            }
        }
        else
        {
            /* discard time before vdec*/
            pSync->VidOpt.VdecDiscardTime = abs(VidPcrDiff); //do what ? nouse
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
            pSync->VidDiscardCnt++;
            MT_INFO_VSYNC(enChn, ">>>>Pcr Lead Vid %d, Discard\n", VidPcrDiff);
        }
    }
    else
    {
        if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay)
        {
            /*discard one frame in every VID_SMOOTH_DISCARD_INTERVAL frame*/
            if (!(pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL))
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
                pSync->VidDiscardCnt++;
                MT_INFO_VSYNC(enChn, ">>>>Pcr Lead Vid %d Smooth, Discard\n", VidPcrDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(enChn, ">>>>Pcr Lead Vid %d Smooth, Play\n", VidPcrDiff);
            }

            if (SYNC_CHAN_VID == enChn)
            {
                pSync->VidDisPlayCnt++;
            }
        }
        else
        {
            /* discard time before vdec*/
            pSync->VidOpt.VdecDiscardTime = abs(VidPcrDiff); //do what ? nouse
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
            pSync->VidDiscardCnt++;
            MT_INFO_VSYNC(enChn, ">>>>Pcr Lead Vid %d, Discard\n", VidPcrDiff);
        }
    }

    return;
}

/*adjust referring to pcr */
mt_void SYNC_PcrSyncAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 AudPcrDiff, mt_s32 VidPcrDiff)
{
    //adjust ok
    if ((pSync->VidAudDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
      &&(pSync->VidAudDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
      &&((AudPcrDiff <= PCR_LAG_ADJUST_THRESHOLD) && (AudPcrDiff >= (-PCR_LEAD_ADJUST_THRESHOLD)))
      &&((VidPcrDiff <= PCR_LAG_ADJUST_THRESHOLD) && (VidPcrDiff >= (-PCR_LEAD_ADJUST_THRESHOLD)))
        )
    {
        if ((pSync->bUseStopRegion == MT_TRUE && abs(pSync->VidAudDiff) <= pSync->VidInfo.FrameTime)
            || (pSync->bUseStopRegion == MT_FALSE)
            )
        {
            pSync->VidSyndAdjust = MT_FALSE;
            pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
            pSync->PcrSyncInfo.PcrVidSyncOK = MT_TRUE;
            pSync->PcrSyncInfo.PcrAudSyncOK = MT_TRUE;

            MT_INFO_SYNC(">>>>Pcr Sync OK. VidAudDiff %d, AudPcrDiff %d, VidPcrDiff %d, Play\n",
                            pSync->VidAudDiff, AudPcrDiff, VidPcrDiff);

            return;
        }
    }

    if(SYNC_CHAN_AUD == enChn)
    {
        /* already in the stop region, */
        if (abs(AudPcrDiff) < PCR_LAG_STOP_THRESHOLD)
        {
            /*vid already in stop region, goon to adjust aud to sync vidaud*/
            if(MT_TRUE == pSync->PcrSyncInfo.PcrVidSyncOK)
            {
                /* video is ahead of audio, discard aud*/
                if (pSync->VidAudDiff > 0)
                {
                    SYNC_PcrSyncDiscardAud(pSync, enChn, pSync->VidAudDiff);
                }
                /* video is behind audio, repeat aud*/
                else
                {
                    SYNC_PcrSyncRepeatAud(pSync, enChn, pSync->VidAudDiff);
                }
            }
            else
            {
                pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
                pSync->PcrSyncInfo.PcrAudSyncOK = MT_TRUE;
            }

            return;
        }
        else
        {
            pSync->PcrSyncInfo.PcrAudSyncOK = MT_FALSE;
        }

        /* audio is ahead of pcr*/
        if (pSync->PcrSyncInfo.AudPcrDiff > 0)
        {
             SYNC_PcrSyncAudLeadAdjust(pSync, enChn, pSync->PcrSyncInfo.AudPcrDiff);
        }
        /* audio is behind pcr*/
        else
        {
             SYNC_PcrSyncAudLagAdjust(pSync, enChn, pSync->PcrSyncInfo.AudPcrDiff);
        }
    }
    else if (SYNC_CHAN_VID == enChn)
    {
        /* already in the stop region, */
        if (abs(VidPcrDiff) < PCR_LAG_STOP_THRESHOLD)
        {
            /*aud already in stop region, goon to adjust vid to sync vidaud*/
            if (MT_TRUE == pSync->PcrSyncInfo.PcrAudSyncOK)
            {
                /* video is ahead of audio, repeat vid*/
                if (pSync->VidAudDiff > 0)
                {
                    SYNC_PcrSyncRepeatVid(pSync, enChn, pSync->VidAudDiff);
                }
                /* video is behind audio, discard vid*/
                else
                {
                    SYNC_PcrSyncDiscardVid(pSync, enChn, pSync->VidAudDiff);
                }
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
                pSync->PcrSyncInfo.PcrVidSyncOK = MT_TRUE; //aud and vid both possible to be MT_TRUE at the same time
            }

            return;
        }
        else
        {
            pSync->PcrSyncInfo.PcrVidSyncOK = MT_FALSE;
        }

        /* video is ahead of pcr*/
        if (pSync->PcrSyncInfo.VidPcrDiff > 0)
        {
            SYNC_PcrSyncVidLeadAdjust(pSync, enChn, pSync->PcrSyncInfo.VidPcrDiff);
        }
        /* video is behind pcr*/
        else
        {
            SYNC_PcrSyncVidLagAdjust(pSync, enChn, pSync->PcrSyncInfo.VidPcrDiff);
        }
    }
}

/* refer to audio and video is ahead of audio*/
mt_void SYNC_AudSyncVidLeadAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    if (VidAudDiff > pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
    {
        /* there is enough audio data in ao buffer */
        if ((pSync->AudInfo.BufTime >= (AO_PCM_DF_UNSTALL_THD_FRAMENUM * pSync->AudInfo.FrameTime))
          &&(pSync->AudInfo.FrameNum >= 10)
           )
        {
            // TODO: x57522  can we discard audio data when playing
            pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

            MT_INFO_ASYNC(enChn, "Vid Lead Aud %d, Discard\n", VidAudDiff);
        }
    }
    else
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_NORMAL;

        MT_INFO_ASYNC(enChn, "Vid Lead Aud %d, Play\n", VidAudDiff);
    }

    /* there is space in video buffer */
    // TODO: x57522 change VidBlockFlag to VidFullFlag
    if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.VidBufState)
    {
        if (VidAudDiff > pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
        {
            if (pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay || pSync->VidFirstPts == MT_INVALID_PTS_U64)
            {
                /* repeat one frame every VID_SMOOTH_REPEAT_INTERVAL frame*/
                if (!(pSync->VidRepPlayCnt % VID_SMOOTH_REPEAT_INTERVAL))
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                    if (SYNC_CHAN_VID == enChn)
                    {
                        pSync->VidRepeatCnt++;
                    }

                    MT_INFO_VSYNC(enChn, "Vid Novel Lead Aud %d Smooth, Repeat\n", VidAudDiff);
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                    MT_INFO_VSYNC(enChn, "Vid Novel Lead Aud %d Smooth, Play\n", VidAudDiff);
                }

                if (SYNC_CHAN_VID == enChn)
                {
                    // TODO:x57522 usage of VidPlayCnt VidDiscardCnt VidRepeatCnt
                    pSync->VidRepPlayCnt++;
                }
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidRepeatCnt++;
                }

                MT_INFO_VSYNC(enChn, "Vid Novel Lead Aud %d, Repeat\n", VidAudDiff);
            }
        }
        else
        {
            if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay || pSync->VidFirstPts == MT_INVALID_PTS_U64)
            {
                /*repeat one frame every VID_SMOOTH_REPEAT_INTERVAL frame*/
                if (!(pSync->VidRepPlayCnt % VID_SMOOTH_REPEAT_INTERVAL))
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                    if (SYNC_CHAN_VID == enChn)
                    {
                        pSync->VidRepeatCnt++;
                    }

                    MT_INFO_VSYNC(enChn, "Vid Lead Aud %d Smooth, Repeat\n", VidAudDiff);
                }
                else
                {
                    pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                    MT_INFO_VSYNC(enChn, "Vid Lead Aud %d Smooth, Play\n", VidAudDiff);
                }

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidRepPlayCnt++;
                }
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidRepeatCnt++;
                }

                MT_INFO_VSYNC(enChn, "Vid Lead Aud %d, Repeat\n", VidAudDiff);
            }
        }
    }
    /* the video buffer reach high waterline*/
    else
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

        MT_INFO_VSYNC(enChn, "Vid Lead Aud %d BufBlock, Play\n", VidAudDiff);
    }

    return;
}

/* refer to audio and video is behind audio*/
mt_void SYNC_AudSyncVidLagAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

    if(SYNC_BUF_STATE_EMPTY == pSync->CrtBufStatus.VidBufState)
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

        if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.AudBufState)
        {
            pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_DOWN;
            MT_INFO_ASYNC(enChn, "Vid Buf Low, VidAudDiff: %d, Aud Speed Down\n", VidAudDiff);
#if 0
            if(pSync->VidAudDiff < -500)
            {
                pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_MUTE_REPEAT;
                MT_INFO_ASYNC(enChn, "Vid Buf Low, VidAudDiff: %d, Aud Mute Repeat\n", VidAudDiff);
            }
            else
            {
                pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_DOWN;
                MT_INFO_ASYNC(enChn, "Vid Buf Low, VidAudDiff: %d, Aud Speed Down\n", VidAudDiff);
            }
#endif
        }

        MT_INFO_VSYNC(enChn, "Vid Buf Low, VidAudDiff: %d, Vid Play\n", VidAudDiff);

        return;
    }
    else if (SYNC_BUF_STATE_LOW == pSync->CrtBufStatus.VidBufState)
    {
        if (SYNC_BUF_STATE_HIGH != pSync->CrtBufStatus.AudBufState)
        {
            pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_DOWN;
            MT_INFO_ASYNC(enChn, "Vid Buf Low, VidAudDiff: %d, Aud Speed Down\n", VidAudDiff);
        }
    }

    if (VidAudDiff < pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime)
    {
        if (pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay)
        {
            /*discard one frame in every VID_SMOOTH_DISCARD_INTERVAL frame*/
            if (!(pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL))
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidDiscardCnt++;
                }

                MT_INFO_VSYNC(enChn, "Vid Novel Lag Aud %d Smooth, Discard\n", VidAudDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(enChn, "Vid Novel Lag Aud %d Smooth, Play\n", VidAudDiff);
            }

            if (SYNC_CHAN_VID == enChn)
            {
                pSync->VidDisPlayCnt++;
            }
        }
        else
        {
            /* discard time before vdec*/
            pSync->VidOpt.VdecDiscardTime = abs(VidAudDiff);
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

            if (SYNC_CHAN_VID == enChn)
            {
                pSync->VidDiscardCnt++;
            }

            MT_INFO_VSYNC(enChn, "Vid Novel Lag Aud %d, Discard\n", VidAudDiff);
        }
    }
    else
    {
        if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay)
        {
            /*discard one frame in every VID_SMOOTH_DISCARD_INTERVAL frame*/
            if (!(pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL))
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

                if (SYNC_CHAN_VID == enChn)
                {
                    pSync->VidDiscardCnt++;
                }

                MT_INFO_VSYNC(enChn, "Vid Lag Aud %d Smooth, Discard\n", VidAudDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(enChn, "Vid Lag Aud %d Smooth, Play\n", VidAudDiff);
            }

            if (SYNC_CHAN_VID == enChn)
            {
                pSync->VidDisPlayCnt++;
            }
        }
        else
        {
            /* discard time before vdec*/
            pSync->VidOpt.VdecDiscardTime = abs(VidAudDiff);
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

            if (SYNC_CHAN_VID == enChn)
            {
                pSync->VidDiscardCnt++;
            }

            MT_INFO_VSYNC(enChn, "Vid Lag Aud %d, Discard\n", VidAudDiff);
        }
    }

    return;
}

/*adjust referring to audio */
mt_void SYNC_AudSyncAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff)
{
    if ((VidAudDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
      &&(VidAudDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
       )
    {
        if ((pSync->bUseStopRegion == MT_TRUE && abs(VidAudDiff) <= pSync->VidInfo.FrameTime)
            || (pSync->bUseStopRegion == MT_FALSE)
            )
        {
            pSync->VidSyndAdjust = MT_FALSE;
            pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

            pSync->PcrSyncInfo.PcrAudSyncOK = MT_TRUE;
            pSync->PcrSyncInfo.PcrVidSyncOK = MT_TRUE;

            MT_INFO_ASYNC(enChn, "Vid AdjustSync Aud, VidAudDiff %d, Play\n", VidAudDiff);
            MT_INFO_VSYNC(enChn, "Vid AdjustSync Aud, VidAudDiff %d, Play\n", VidAudDiff);

            return;
        }
    }

    /* video is ahead of audio and the difference is too large */
    if (VidAudDiff > VID_LEAD_DISCARD_THRESHOLD)
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

        MT_INFO_ASYNC(enChn, "VidAudDiff %d > VID_LEAD_DISCARD_THRESHOLD, Play\n", VidAudDiff);
        MT_INFO_VSYNC(enChn, "VidAudDiff %d > VID_LEAD_DISCARD_THRESHOLD, Play\n", VidAudDiff);

        return;
    }

    /* video is behind audio and the difference is too large */
    if (VidAudDiff < (-VID_LAG_DISCARD_THRESHOLD))
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

        MT_INFO_ASYNC(enChn, "VidAudDiff %d < VID_LAG_DISCARD_THRESHOLD, Play\n", VidAudDiff);
        MT_INFO_VSYNC(enChn, "VidAudDiff %d < VID_LAG_DISCARD_THRESHOLD, Play\n", VidAudDiff);

        return;
    }

    /* video is ahead of audio*/
    if (VidAudDiff > 0)
    {
        SYNC_AudSyncVidLeadAdjust(pSync, enChn, VidAudDiff);
    }
    /* video is behind audio*/
    else
    {
        SYNC_AudSyncVidLagAdjust(pSync, enChn, VidAudDiff);
    }

    return;
}

mt_void SYNC_ScrSyncAudAdjust(SYNC_S *pSync, mt_s32 VidAudDiff, mt_s32 AudScrDiff, mt_s32 VidScrDiff)
{

#ifdef MT_AVPLAY_SCR_SUPPORT

    /* aud is behind scr and AudScrDiff is in novel region */
    if(AudScrDiff >= pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime
        && AudScrDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
    {
        #if 1
        pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudScrDiff: %d, DISCARD\n", AudScrDiff);
        #else
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_UP;

        MT_INFO_SYNC("AudScrDiff: %d, Play, SPEED UP\n", AudScrDiff);
        #endif
    }
    /* aud is behind scr and AudScrDiff is out of novel region*/
    else if(AudScrDiff < pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime)
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_DISCARD;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudScrDiff: %d, DISCARD\n", AudScrDiff);
    }
    /* aud is ahead scr and AudScrDiff is in novel region */
    else if(AudScrDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime
        && AudScrDiff <= pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
    {
        #if 1
        pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudScrDiff: %d, REPEAT\n", AudScrDiff);
        #else
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_DOWN;

        MT_INFO_SYNC("AudScrDiff: %d, Play, SPEED DOWN\n", AudScrDiff);
        #endif
    }
    /* aud is ahead scr and AudScrDiff is out of novel region*/
    else if(AudScrDiff > pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudScrDiff: %d, REPEAT\n", AudScrDiff);
    }
#endif
    return;
}

mt_void SYNC_ScrSyncVidAdjust(SYNC_S *pSync, mt_s32 VidAudDiff, mt_s32 AudScrDiff, mt_s32 VidScrDiff)
{
#ifdef MT_AVPLAY_SCR_SUPPORT
    /* VidScrDiff is already in start region */
    if(VidScrDiff > pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime
        && VidScrDiff < pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
    {
        /* VidAudDiff is already in start region */
        if (VidAudDiff < pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime
            && VidAudDiff > pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, Play\n", VidScrDiff);
        }
        /* VidAudDiff is beyond start region, we adjust vid */
        /* we only need adjust one or two frames, so we ignore smooth play */
        else
        {
            if(AudScrDiff > 0 && VidScrDiff < 0)
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

                pSync->VidDiscardCnt++;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, DISCARD\n", VidScrDiff);
            }

            if(AudScrDiff < 0 && VidScrDiff > 0)
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                pSync->VidRepeatCnt++;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, REPEAT\n", VidScrDiff);
            }
        }
    }
    /* vid is behind scr and VidScrDiff is in novel region */
    else if(VidScrDiff >= pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime
        && VidScrDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
    {
        if(pSync->SyncAttr.stSyncStartRegion.bSmoothPlay)
        {
            if(0 == pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL)
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

                pSync->VidDiscardCnt++;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH DISCARD\n", VidScrDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH Play\n", VidScrDiff);
            }

            pSync->VidDisPlayCnt++;
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, DISCARD\n", VidScrDiff);
        }
    }
    /* vid is behind scr and VidScrDiff is out of novel region*/
    else if(VidScrDiff < pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime)
    {
        if(pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay)
        {
            if(0 == pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL)
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

                pSync->VidDiscardCnt++;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH DISCARD\n", VidScrDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH Play\n", VidScrDiff);
            }

            pSync->VidDisPlayCnt++;
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

            pSync->VidDiscardCnt++;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, DISCARD\n", VidScrDiff);
        }
    }
    /* vid is ahead scr and VidScrDiff is in novel region */
    else if(VidScrDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime
        && VidScrDiff <= pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
    {
        if(pSync->SyncAttr.stSyncStartRegion.bSmoothPlay)
        {
            if(0 == pSync->VidRepPlayCnt % VID_SMOOTH_REPEAT_INTERVAL)
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                pSync->VidRepeatCnt++;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH REPEAT\n", VidScrDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH Play\n", VidScrDiff);
            }

            pSync->VidRepPlayCnt++;
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

            pSync->VidRepeatCnt++;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, REPEAT\n", VidScrDiff);
        }
    }
    /* vid is ahead scr and VidScrDiff is out of novel region*/
    else if(VidScrDiff > pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
    {
       if(pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay)
        {
            if(0 == pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL)
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

                pSync->VidRepeatCnt++;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH REPEAT\n", VidScrDiff);
            }
            else
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, SMOOTH PLAY\n", VidScrDiff);
            }

            pSync->VidRepPlayCnt++;
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_REPEAT;

            pSync->VidRepeatCnt++;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidScrDiff: %d, REPEAT\n", VidScrDiff);
        }
    }

#endif

    return;
}

mt_void SYNC_ScrSyncAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn, mt_s32 VidAudDiff, mt_s32 AudScrDiff, mt_s32 VidScrDiff)
{

#ifdef MT_AVPLAY_SCR_SUPPORT

    /* AudScrDiff or VidScrDiff is too large, give up */
    if ((abs(AudScrDiff) > SCR_DISCARD_THRESHOLD)
      ||(abs(VidScrDiff) > SCR_DISCARD_THRESHOLD)
       )
    {
        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudScrDiff %d VidScrDiff %d > SCR_DISCARD_THRESHOLD, Play\n", AudScrDiff, VidScrDiff);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "AudScrDiff %d VidScrDiff %d > SCR_DISCARD_THRESHOLD, Play\n", AudScrDiff, VidScrDiff);

        return;
    }

    if(SYNC_CHAN_AUD == enChn)
    {
        /* already in start region */
        if(AudScrDiff > pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime
            && AudScrDiff < pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

            MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudScrDiff: %d, Play\n", AudScrDiff);
        }
        else
        {
            SYNC_ScrSyncAudAdjust(pSync, pSync->VidAudDiff, pSync->AudScrDiff, pSync->VidScrDiff);
        }
    }
    else if (SYNC_CHAN_VID == enChn)
    {
        SYNC_ScrSyncVidAdjust(pSync, pSync->VidAudDiff, pSync->AudScrDiff, pSync->VidScrDiff);
    }
#endif
    return;
}

mt_void SYNC_SyncAdjust(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    mt_s32      AudPcrDiff = 0;
    mt_s32      VidPcrDiff = 0;

    AudPcrDiff = pSync->PcrSyncInfo.AudPcrDiff;
    VidPcrDiff = pSync->PcrSyncInfo.VidPcrDiff;

    /* refer to scr or refer to audio but audio first frame does not come */
    if((MT_UNF_SYNC_REF_SCR == pSync->SyncAttr.enSyncRef)
        || ((MT_UNF_SYNC_REF_AUDIO == pSync->SyncAttr.enSyncRef) && (!pSync->AudFirstCome))
        )
    {
        SYNC_ScrSyncAdjust(pSync, enChn, pSync->VidAudDiff, pSync->AudScrDiff, pSync->VidScrDiff);
    }
    /* refer to aud or refer to pcr but pcr does not come */
    else if ((MT_UNF_SYNC_REF_AUDIO == pSync->SyncAttr.enSyncRef)
      ||(((MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef) && (!pSync->PcrSyncInfo.PcrFirstCome))
      || (SYNC_AUD_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust))   //pcr does not come and pcr timeout
       )
    {
        if (!pSync->VidSyndAdjust)
        {
            /* already in the start region*/
            if ((pSync->VidAudDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
              &&(pSync->VidAudDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
               )
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
                pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_ASYNC(enChn, "Vid Sync Aud, VidAudDiff %d, Play\n", pSync->VidAudDiff);
                MT_INFO_VSYNC(enChn, "Vid Sync Aud, VidAudDiff %d, Play\n", pSync->VidAudDiff);

                return;
            }
            else
            {
                pSync->VidSyndAdjust = MT_TRUE;
            }
        }

        if (pSync->VidSyndAdjust)
        {
            SYNC_AudSyncAdjust(pSync, enChn, pSync->VidAudDiff);
        }

        return;
    }
    /*refer to pcr and received pcr*/
    else if(MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
    {
        /*1. VidAudDiff is too large*/
        if (abs(pSync->VidAudDiff) >= VID_LEAD_DISCARD_THRESHOLD)
        {
            /*1.0 initial value*/
            pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

           /*1.1 believe vidpcr, only adjust aud to match pcr*/
           if ((SYNC_CHAN_AUD == enChn) && (abs(AudPcrDiff) < PCR_DISCARD_THRESHOLD))
           {
               if (!pSync->VidSyndAdjust)
               {
                   /* already in the start region*/
                   if (abs(AudPcrDiff) <= PCR_LAG_ADJUST_THRESHOLD)
                   {
                       pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

                       MT_INFO_ASYNC(enChn, "@@@@Aud Sync Pcr, AudPcrDiff %d, Play\n", AudPcrDiff);

                       return;
                   }
                   else
                   {
                       pSync->VidSyndAdjust = MT_TRUE;
                   }
               }

               if (pSync->VidSyndAdjust)
               {
                   if (abs(AudPcrDiff) <= PCR_LEAD_STOP_THRESHOLD)
                   {
                       pSync->VidSyndAdjust = MT_FALSE;
                       pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

                       MT_INFO_ASYNC(enChn, "@@@@Aud AdjustSync Pcr, AudPcrDiff %d, Play\n", AudPcrDiff);

                       return;
                   }

                   MT_INFO_ASYNC(enChn, "@@@@Aud Need To Sync Pcr, AudPcrDiff %d\n", AudPcrDiff);

                   /* audio is ahead of pcr*/
                   if (AudPcrDiff > 0)
                   {
                        SYNC_PcrSyncAudLeadAdjust(pSync, enChn, AudPcrDiff);
                   }
                   /* audio is behind pcr*/
                   else
                   {
                        SYNC_PcrSyncAudLagAdjust(pSync, enChn, AudPcrDiff);
                   }
               }

               return;
           }

           /*1.2 believe audpcr, only adjust vid to match pcr*/
           if ((SYNC_CHAN_VID == enChn) && (abs(VidPcrDiff) < PCR_DISCARD_THRESHOLD))
           {
               if (!pSync->VidSyndAdjust)
               {
                   /* already in the start region*/
                   if (abs(VidPcrDiff) <= PCR_LAG_ADJUST_THRESHOLD)
                   {
                       pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                       MT_INFO_VSYNC(enChn, "@@@@Vid Sync Pcr, VidPcrDiff %d, Play\n", VidPcrDiff);

                       return;
                   }
                   else
                   {
                       pSync->VidSyndAdjust = MT_TRUE;
                   }
               }

               if (pSync->VidSyndAdjust)
               {
                   if (abs(VidPcrDiff) <= PCR_LEAD_STOP_THRESHOLD)
                   {
                       pSync->VidSyndAdjust = MT_FALSE;
                       pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

                       MT_INFO_VSYNC(enChn, "@@@@Vid AdjustSync Pcr, VidPcrDiff %d, Play\n", VidPcrDiff);

                       return;
                   }

                   MT_INFO_VSYNC(enChn, "@@@@Vid Need To Sync Pcr, VidPcrDiff %d\n", VidPcrDiff);

                   /* video is ahead of pcr*/
                   if (VidPcrDiff > 0)
                   {
                        SYNC_PcrSyncVidLeadAdjust(pSync, enChn, VidPcrDiff);
                   }
                   /* video is behind pcr*/
                   else
                   {
                        SYNC_PcrSyncVidLagAdjust(pSync, enChn, VidPcrDiff);
                   }
               }

               return;
           }

           MT_INFO_ASYNC(enChn, ">>>>VidAudDiff %d is too large, Play\n", pSync->VidAudDiff);
           MT_INFO_VSYNC(enChn, ">>>>VidAudDiff %d is too large, Play\n", pSync->VidAudDiff);

           return;
        }


        if (!pSync->VidSyndAdjust)
        {
            /* 2. VidAudDiff is in sync start region, vidpcr and audpcr is in sync start region, don't need to adjust  */
            if ((pSync->VidAudDiff <= pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
              &&(pSync->VidAudDiff >= pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
              &&((AudPcrDiff <= PCR_LAG_ADJUST_THRESHOLD) && (AudPcrDiff >= (-PCR_LEAD_ADJUST_THRESHOLD)))
              &&((VidPcrDiff <= PCR_LAG_ADJUST_THRESHOLD) && (VidPcrDiff >= (-PCR_LEAD_ADJUST_THRESHOLD)))
               )
            {
                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
                pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

                MT_INFO_ASYNC(enChn, ">>>>Vid Sync Aud, VidAudDiff %d, Play\n", pSync->VidAudDiff);
                MT_INFO_VSYNC(enChn, ">>>>Vid Sync Aud, VidAudDiff %d, Play\n", pSync->VidAudDiff);

                return;
            }
            else
            {
                pSync->PcrSyncInfo.PcrAudSyncOK = MT_FALSE;
                pSync->PcrSyncInfo.PcrVidSyncOK = MT_FALSE;
                pSync->VidSyndAdjust = MT_TRUE;
            }
        }

        if (pSync->VidSyndAdjust)
        {
            /*3. adjust pcr to this one which is more behind */
            if (((AudPcrDiff >= PCR_DISCARD_THRESHOLD) && (VidPcrDiff >= PCR_DISCARD_THRESHOLD))
                || ((AudPcrDiff <= (-PCR_DISCARD_THRESHOLD)) && (VidPcrDiff <= (-PCR_DISCARD_THRESHOLD)))
                )
            {
                if (MT_TRUE == pSync->PcrSyncInfo.PcrAdjustDeltaOK)
                {
                    /*3.1 adjust pcr to this one which is more behind */
                    pSync->PcrSyncInfo.PcrDelta += (pSync->VidAudDiff > 0) ? AudPcrDiff : VidPcrDiff;

                    pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_FALSE;

                    MT_INFO_SYNC(">>>>adjust pcr, PcrDelta %d, AudPcrDiff %d, VidPcrDiff %d\n",
                                pSync->PcrSyncInfo.PcrDelta,pSync->PcrSyncInfo.AudPcrDiff, pSync->PcrSyncInfo.VidPcrDiff);
                }

                pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
                pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

                return;
            }
            else
            {
                /*4. adjust aud and vid to match pcr*/
                SYNC_PcrSyncAdjust(pSync, enChn, AudPcrDiff, VidPcrDiff);
            }
        }
    }

    return;
}

SYNC_REGION_STAT_E SYNC_CheckRegion(SYNC_S *pSync, mt_s32 Diff)
{
    if ((Diff < pSync->SyncAttr.stSyncStartRegion.s32VidPlusTime)
        && (Diff > pSync->SyncAttr.stSyncStartRegion.s32VidNegativeTime)
        )
    {
        return SYNC_REGION_STAT_IN_START;
    }
    else if ((Diff < pSync->SyncAttr.stSyncNovelRegion.s32VidPlusTime)
            && (Diff > pSync->SyncAttr.stSyncNovelRegion.s32VidNegativeTime)
            )
    {
        return SYNC_REGION_STAT_IN_NOVEL;
    }
    else if ((Diff < VID_LEAD_DISCARD_THRESHOLD)
            && (Diff > -VID_LAG_DISCARD_THRESHOLD)
            )
    {
        return SYNC_REGION_STAT_IN_DISCARD;
    }
    else
    {
        return SYNC_REGION_STAT_OUT_DISCARD;
    }

}


mt_void SYNC_UpLoadEvent(SYNC_S *pSync, SYNC_AVSYNC_EVENT_E event)
{
    if (NULL == pSync)
    {
         MT_ERR_SYNC("%s, %d, pSync is NULL\n", __func__, __LINE__);
         return;
    }

    if (event >= SYNC_AVSYNC_EVENT_BUTT)
    {
        MT_ERR_SYNC("%s, %d, avsync event error\n", __func__, __LINE__);
        return;
    }

    switch (event)
    {
        case SYNC_AVSYNC_EVENT_APTS_JUMP:
        {
            pSync->SyncEvent.bAudPtsJump = MT_TRUE;
            pSync->SyncEvent.AudPtsJumpParam.enPtsChan = MT_UNF_SYNC_PTS_CHAN_AUD;
            pSync->SyncEvent.AudPtsJumpParam.u32CurPts = pSync->AudInfo.Pts;
            pSync->SyncEvent.AudPtsJumpParam.u32CurSrcPts = pSync->AudInfo.SrcPts;
            pSync->SyncEvent.AudPtsJumpParam.u32FirstPts = pSync->AudFirstPts;
            pSync->SyncEvent.AudPtsJumpParam.u32FirstValidPts = pSync->AudFirstValidPts;
            pSync->SyncEvent.AudPtsJumpParam.u32LastPts = pSync->AudLastPts;
            pSync->SyncEvent.AudPtsJumpParam.u32LastSrcPts = pSync->AudLastSrcPts;
        }
        break;

        case SYNC_AVSYNC_EVENT_VPTS_JUMP:
        {
            pSync->SyncEvent.bVidPtsJump = MT_TRUE;
            pSync->SyncEvent.VidPtsJumpParam.enPtsChan = MT_UNF_SYNC_PTS_CHAN_VID;
            pSync->SyncEvent.VidPtsJumpParam.u32CurPts = pSync->VidInfo.Pts;
            pSync->SyncEvent.VidPtsJumpParam.u32CurSrcPts = pSync->VidInfo.SrcPts;
            pSync->SyncEvent.VidPtsJumpParam.u32FirstPts = pSync->VidFirstPts;
            pSync->SyncEvent.VidPtsJumpParam.u32FirstValidPts = pSync->VidFirstValidPts;
            pSync->SyncEvent.VidPtsJumpParam.u32LastPts = pSync->VidLastPts;
            pSync->SyncEvent.VidPtsJumpParam.u32LastSrcPts = pSync->VidLastSrcPts;
        }
        break;

        case SYNC_AVSYNC_EVENT_STA_CHANGE:
        {
            pSync->SyncEvent.bStatChange = MT_TRUE;
            pSync->SyncEvent.StatParam.s32VidAudDiff = pSync->VidAudDiff;
            pSync->SyncEvent.StatParam.s32VidPcrDiff = pSync->PcrSyncInfo.VidPcrDiff;
            pSync->SyncEvent.StatParam.s32AudPcrDiff = pSync->PcrSyncInfo.AudPcrDiff;
            pSync->SyncEvent.StatParam.u32VidLocalTime = pSync->VidLastLocalTime;
            pSync->SyncEvent.StatParam.u32AudLocalTime = pSync->AudLastLocalTime;
            pSync->SyncEvent.StatParam.u32PcrLocalTime = pSync->PcrSyncInfo.PcrLastLocalTime;
        }
        break;

        case SYNC_AVSYNC_EVENT_EOS:
        {
            pSync->SyncEvent.bEos_back = MT_TRUE;
        }
        break;

        case SYNC_AVSYNC_EVENT_AUD_ROLLBACK:
        {
            pSync->SyncEvent.AudPtsJumpParam.bLoopback = MT_TRUE;
        }
        break;

        case SYNC_AVSYNC_EVENT_VID_ROLLBACK:
        {
            pSync->SyncEvent.VidPtsJumpParam.bLoopback = MT_TRUE;
        }
        break;

        default:
        {
            MT_INFO_SYNC("%s, %d, avsync event error\n", __func__, __LINE__);
        }
        break;
    }
}

mt_void SYNC_CheckEvent(SYNC_S *pSync, SYNC_CHAN_E enChn)
{
    mt_u32                  ErrDelta;
    SYNC_REGION_STAT_E      LastStat = SYNC_REGION_STAT_BUTT;
    SYNC_REGION_STAT_E      CurStat = SYNC_REGION_STAT_BUTT;
    mt_u32                  CurSysTime;

    if (SYNC_CHAN_VID == enChn)
    {
        ErrDelta = pSync->VidInfo.FrameTime * SYNC_PTS_JUMP_FRM_NUM;

        if (MT_INVALID_PTS_U64 == pSync->VidLastPts)
        {
            return;
        }

        if (abs(pSync->VidInfo.Pts - pSync->VidLastPts) > ErrDelta)
        {
            MT_INFO_VSYNC(SYNC_CHAN_VID, "VidLastPts %d, VidPts %d, u32FirstValidPts %d, ErrDelta %d\n",
                pSync->VidInfo.Pts, pSync->VidLastPts, pSync->VidFirstValidPts, ErrDelta);

            pSync->SyncEvent.bVidPtsJump = MT_TRUE;
            pSync->SyncEvent.VidPtsJumpParam.enPtsChan = MT_UNF_SYNC_PTS_CHAN_VID;
            pSync->SyncEvent.VidPtsJumpParam.u32CurPts = pSync->VidInfo.Pts;
            pSync->SyncEvent.VidPtsJumpParam.u32CurSrcPts = pSync->VidInfo.SrcPts;
            pSync->SyncEvent.VidPtsJumpParam.u32FirstPts = pSync->VidFirstPts;
            pSync->SyncEvent.VidPtsJumpParam.u32FirstValidPts = pSync->VidFirstValidPts;
            pSync->SyncEvent.VidPtsJumpParam.u32LastPts = pSync->VidLastPts;
            pSync->SyncEvent.VidPtsJumpParam.u32LastSrcPts = pSync->VidLastSrcPts;

            /* pts is one of VidFirstDecPts/VidSecondDecPts/VidFirstValidPts */
            if ((pSync->VidInfo.SrcPts == pSync->VidFirstDecPts)
                    || (pSync->VidInfo.SrcPts == pSync->VidSecondDecPts)
                    || (pSync->VidInfo.SrcPts == pSync->VidFirstValidPts)
               )
            {
                MT_INFO_VSYNC(SYNC_CHAN_VID,
                    "Vid pts LoopBack, SrcPts %u, VidFirstDecPts %u, VidSecondDecPts %u, VidFirstValidPts %u!\n",
                    pSync->VidInfo.SrcPts, pSync->VidFirstDecPts, pSync->VidSecondDecPts, pSync->VidFirstValidPts);

                pSync->SyncEvent.VidPtsJumpParam.bLoopback = MT_TRUE;
                pSync->VidPtsLoopBack = MT_TRUE;
            }
            else
            {
                pSync->SyncEvent.VidPtsJumpParam.bLoopback = MT_FALSE;
            }
        }
    }
    else if (SYNC_CHAN_AUD == enChn)
    {
        ErrDelta = pSync->AudInfo.FrameTime * SYNC_PTS_JUMP_FRM_NUM;

        if (MT_INVALID_PTS_U64 == pSync->AudLastPts)
        {
            return;
        }

        if (abs(pSync->AudInfo.Pts - pSync->AudLastPts) > ErrDelta)
        {

            MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudLastPts %d, AuddPts %d, u32FirstValidPts %d, ErrDelta %d\n",
                pSync->AudLastPts, pSync->AudInfo.Pts, pSync->AudFirstValidPts, ErrDelta);

            pSync->SyncEvent.bAudPtsJump = MT_TRUE;
            pSync->SyncEvent.AudPtsJumpParam.enPtsChan = MT_UNF_SYNC_PTS_CHAN_AUD;
            pSync->SyncEvent.AudPtsJumpParam.u32CurPts = pSync->AudInfo.Pts;
            pSync->SyncEvent.AudPtsJumpParam.u32CurSrcPts = pSync->AudInfo.SrcPts;
            pSync->SyncEvent.AudPtsJumpParam.u32FirstPts = pSync->AudFirstPts;
            pSync->SyncEvent.AudPtsJumpParam.u32FirstValidPts = pSync->AudFirstValidPts;
            pSync->SyncEvent.AudPtsJumpParam.u32LastPts = pSync->AudLastPts;
            pSync->SyncEvent.AudPtsJumpParam.u32LastSrcPts = pSync->AudLastSrcPts;

            if (pSync->AudInfo.SrcPts == pSync->AudFirstValidPts)
            {
                MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud pts LoopBack!\n");

                pSync->SyncEvent.AudPtsJumpParam.bLoopback = MT_TRUE;
                pSync->AudPtsLoopBack = MT_TRUE;
            }
            else
            {
                pSync->SyncEvent.AudPtsJumpParam.bLoopback = MT_FALSE;
            }
        }
    }
    /* pcr jump check, to add */
    else
    {
    }

    if (MT_UNF_SYNC_REF_AUDIO == pSync->SyncAttr.enSyncRef)
    {
        LastStat = SYNC_CheckRegion(pSync, pSync->LastVidAudDiff);
        CurStat = SYNC_CheckRegion(pSync, pSync->VidAudDiff);
    }
    else if (MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef)
    {
        LastStat = SYNC_CheckRegion(pSync, pSync->PcrSyncInfo.LastAudPcrDiff);
        LastStat |= SYNC_CheckRegion(pSync, pSync->PcrSyncInfo.LastVidPcrDiff);
        CurStat = SYNC_CheckRegion(pSync, pSync->PcrSyncInfo.AudPcrDiff);
        CurStat |= SYNC_CheckRegion(pSync, pSync->PcrSyncInfo.VidPcrDiff);
    }

    /* if sync status change, record this event */
    if (LastStat != CurStat)
    {
        MT_INFO_SYNC("Sync Status change: LastStat %d, CurStat %d!\n",
            LastStat, CurStat);

        pSync->SyncEvent.bStatChange = MT_TRUE;
        pSync->SyncEvent.StatParam.s32VidAudDiff = pSync->VidAudDiff;
        pSync->SyncEvent.StatParam.s32VidPcrDiff = pSync->PcrSyncInfo.VidPcrDiff;
        pSync->SyncEvent.StatParam.s32AudPcrDiff = pSync->PcrSyncInfo.AudPcrDiff;
        pSync->SyncEvent.StatParam.u32VidLocalTime = pSync->VidLastLocalTime;
        pSync->SyncEvent.StatParam.u32AudLocalTime = pSync->AudLastLocalTime;
        pSync->SyncEvent.StatParam.u32PcrLocalTime = pSync->PcrSyncInfo.PcrLastLocalTime;
    }

    /*do not sync when vid or aud loopback*/
    if (!pSync->LoopBackFlag && (pSync->AudPtsLoopBack || pSync->VidPtsLoopBack))
    {
        pSync->LoopBackFlag = MT_TRUE;
        pSync->LoopBackTime = SYNC_GetSysTime();

        MT_INFO_SYNC("Change SyncRef=NONE, AudPtsLoopBack %d, VidPtsLoopBack %d\n",
            pSync->AudPtsLoopBack, pSync->VidPtsLoopBack);
    }

    if (pSync->LoopBackFlag)
    {
        CurSysTime = SYNC_GetSysTime();

        /*recover sync when timeout or vid and aud both loopback*/
        if ((CurSysTime - pSync->LoopBackTime > PTS_LOOPBACK_TIMEOUT)
            || (pSync->AudPtsLoopBack && pSync->VidPtsLoopBack)
            )
        {
            pSync->LoopBackFlag = MT_FALSE;
            pSync->AudPtsLoopBack = MT_FALSE;
            pSync->VidPtsLoopBack = MT_FALSE;

            MT_INFO_SYNC("Change SyncRef=UserSetRed, AudPtsLoopBack %d, VidPtsLoopBack %d\n",
                pSync->AudPtsLoopBack, pSync->VidPtsLoopBack);
        }
    }

    return;
}

mt_void SYNC_CheckTBMatchAdjust(SYNC_S *pSync)
{
    SYNC_REGION_STAT_E      enSyncRegion;

    enSyncRegion = SYNC_CheckRegion(pSync, pSync->VidAudDiff);

    if (enSyncRegion == SYNC_REGION_STAT_IN_START)
    {
        if (pSync->VidSyndAdjust)
        {
            pSync->VidOpt.enTBAdjust = MT_DRV_VIDEO_TB_PLAY;
            return;
        }

        if (pSync->VidAudDiff >= 0)
        {
            pSync->VidOpt.enTBAdjust = MT_DRV_VIDEO_TB_REPEAT;
        }
        else
        {
            pSync->VidOpt.enTBAdjust = MT_DRV_VIDEO_TB_DISCARD;
        }
        return ;
    }
    else
    {
        pSync->VidOpt.enTBAdjust = MT_DRV_VIDEO_TB_PLAY;
        return;
    }
}

mt_void SYNC_VidProc(mt_handle hSync, SYNC_VID_INFO_S *pVidInfo, SYNC_VID_OPT_S *pVidOpt)
{
    SYNC_S             *pSync;
    mt_upts             PtsDelta;
    mt_u32             SysTime;

    if (MT_FALSE == SYNC_VerifyHandle(hSync))
    {
       // return;
    }

    //pSync = g_SyncGlobalState.SyncInfo[hSync&0xff].pSync;
    pSync = g_SyncGlobalState.SyncInfo[0].pSync;

    pVidOpt->SyncProc = SYNC_PROC_PLAY;
    pVidOpt->Repeat = 1;
    pVidOpt->Discard = 0;
    pVidOpt->VdecDiscardTime = 0;

    pSync->VidOpt = *pVidOpt;

    /* record the video frame information */
    pSync->VidInfo = *pVidInfo;

    SysTime = SYNC_GetSysTime();

    MT_INFO_SYNC("V: [%x] [%llx] [%x][%llx][%llx]\n",pSync->VidFirstCome,  pVidInfo->Pts, pSync->VidPtsSeriesCnt,  pSync->VidFirstPts, pSync->VidLastPts);

    MT_INFO_VSYNC(SYNC_CHAN_VID, "VidInfo SrcPts %-8d,Pts %-8d,FrameTime %d, DelayTime %d,SysTime %d\n",
                                 pVidInfo->SrcPts, pVidInfo->Pts, pVidInfo->FrameTime,pVidInfo->DelayTime,SysTime);

    /*can't do(no effect) smooth adjust when do sync without FRC(frame rate convertion)*/
    if (!pSync->bUseStopRegion)
    {
        pSync->SyncAttr.stSyncStartRegion.bSmoothPlay = MT_FALSE;
        pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay = MT_FALSE;
    }

    /*pcr timeout, we used aud adjust scr*/
    if (SYNC_SCR_ADJUST_BUTT == pSync->PcrSyncInfo.enPcrAdjust)
    {
        if (MT_TRUE == SYNC_CheckPcrTimeout(pSync))
        {
            pSync->PcrSyncInfo.enPcrAdjust = SYNC_AUD_ADJUST_SCR;

            MT_INFO_SYNC("Pcr is timeout adjust pcr by audpts\n");
        }
    }

    if (0 == pVidInfo->DispTime)
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;
        *pVidOpt = pSync->VidOpt;
        MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid Frc Discard--------\n");
        MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");
        return;
    }

    /* quick output the first frame*/
    if ((pSync->SyncAttr.bQuickOutput)&&(!pSync->VidFirstPlay))
    {
        pSync->VidFirstPlay = MT_TRUE;
        pSync->VidFirstPlayTime = SYNC_GetSysTime();

        pSync->VidFirstCome = MT_TRUE;
        pSync->VidFirstSysTime = pSync->VidFirstPlayTime;
        pSync->VidFirstPts = pVidInfo->Pts;

        mt_drv_stat_event(STAT_EVENT_FRAMESYNCOK, 0);

        pSync->VidOpt.SyncProc = SYNC_PROC_QUICKOUTPUT;
        *pVidOpt = pSync->VidOpt;

        MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid QuickOutput Proc %d--------\n", pVidOpt->SyncProc);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");

        MT_INFO_VSYNC(SYNC_CHAN_VID, "First VidFrame SysTime %d, Pts %d, SrcPts %d\n", pSync->VidFirstSysTime, pSync->VidFirstPts, pVidInfo->SrcPts);
        return;
    }

    /* record the information and the time of the first video frame used to sync*/
    if (!pSync->VidFirstCome)
    {
        pSync->VidFirstCome = MT_TRUE;
        pSync->VidFirstSysTime = SYNC_GetSysTime();
        pSync->VidFirstPts = pVidInfo->Pts;

        MT_INFO_VSYNC(SYNC_CHAN_VID, "First VidFrame SysTime %d, Pts %d, SrcPts %d\n", pSync->VidFirstSysTime, pSync->VidFirstPts, pVidInfo->SrcPts);
    }

    if (!pSync->VidFirstValidCome && (-1 != pVidInfo->SrcPts))
    {
        pSync->VidFirstValidCome = MT_TRUE;
        pSync->VidFirstValidPts = pVidInfo->SrcPts;

        MT_INFO_VSYNC(SYNC_CHAN_VID, "VidFirstValidPts come: %u\n", pSync->VidFirstValidPts);
    }

    if (SYNC_STATUS_TPLAY == pSync->CrtStatus)
	{
		SYNC_SetLocalTime(pSync, SYNC_CHAN_VID, (pVidInfo->Pts - pSync->VidInfo.DelayTime));
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
        pSync->VidLastPts = pVidInfo->Pts;
    	pSync->VidLastSrcPts = pVidInfo->SrcPts;

        *pVidOpt = pSync->VidOpt;
        MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid TPLAY--------\n");
        MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");
        return;
	}

    /* presync*/
    if (!pSync->PreSyncFinish)
    {
        SYNC_PreSync(pSync, SYNC_CHAN_VID);

        if (pSync->VidOpt.SyncProc != SYNC_PROC_CONTINUE)
        {
            pSync->VidLastPts = pVidInfo->Pts;
            pSync->VidLastSrcPts = pVidInfo->SrcPts;
            *pVidOpt = pSync->VidOpt;

            if (pSync->PreSyncTargetInit)
            {
                MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid PreSync VidAudDiff %d Proc %d--------\n", pSync->VidAudDiff, pVidOpt->SyncProc);
            }

            MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");
            return;
        }
    }

    /* first set scr */
    if(pSync->PreSyncFinish && !pSync->ScrInitFlag)
    {
        if(-1 != pVidInfo->Pts)
        {
            SYNC_SetLocalTime(pSync, SYNC_CHAN_SCR, (pVidInfo->Pts - pSync->VidInfo.DelayTime));
            pSync->ScrInitFlag = MT_TRUE;
            pSync->ScrFirstLocalTime = pVidInfo->Pts - pSync->VidInfo.DelayTime;
            pSync->ScrFirstSysTime = SYNC_GetSysTime();
        }
    }

    /* presync finished. cumulate video and audio data to prevent underflow*/
    if (!pSync->BufFundFinish)
    {
        SYNC_BufFund(pSync);

        if (pSync->VidOpt.SyncProc != SYNC_PROC_CONTINUE)
        {
            *pVidOpt = pSync->VidOpt;
            MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid BufFund Proc %d--------\n", pVidOpt->SyncProc);
            MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");
            return;
        }

        if (pSync->AudFirstCome)
        {
            SYNC_SetLocalTime(pSync, SYNC_CHAN_AUD, (pSync->AudInfo.Pts - pSync->AudInfo.BufTime));
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud BufFund First SetLocalTime %d\n", (pSync->AudInfo.Pts - pSync->AudInfo.BufTime));

            if (SYNC_AUD_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
            {
               SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR,  (pSync->AudInfo.Pts - pSync->AudInfo.BufTime));

               MT_INFO_SYNC(">>>>Pcr SetLocalTime %d  by Audpts\n",  (pSync->AudInfo.Pts - pSync->AudInfo.BufTime));
            }
        }

        SYNC_SetLocalTime(pSync, SYNC_CHAN_VID, (pVidInfo->Pts - pSync->VidInfo.DelayTime));

        MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid BufFund First SetLocalTime %d\n", (pVidInfo->Pts - pSync->VidInfo.DelayTime));
    }

    /* Pts is invalid*/
    if (-1 == pVidInfo->Pts)
    {
        MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid Pts == -1 invalid\n");
        pSync->VidPtsSeriesCnt = 0;
    }
    /* Pts go back */
    else if (pVidInfo->Pts < pSync->VidLastPts)
    {
        MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid Pts <= Vid LstPts %d\n", pSync->VidLastPts);
        pSync->VidPtsSeriesCnt = 0;
    }
    /* Pts jump too much */
    else
    {
        PtsDelta = pVidInfo->Pts - pSync->VidLastPts;

        if (PtsDelta > VID_PTS_GAP)
        {
            MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid PtsDelta %d > VID_PTS_GAP %d\n", PtsDelta, VID_PTS_GAP);
            pSync->VidPtsSeriesCnt = 0;
        }
    }

    pSync->VidPtsSeriesCnt++;

    /* if video localtime has not been initialized or there are three successive video pts,update video localtime*/
    if ((pSync->VidPtsSeriesCnt >= PTS_SERIES_COUNT)
        ||((MT_FALSE == pSync->VidLocalTimeFlag) && (-1 != pVidInfo->Pts)))
    {
        SYNC_SetLocalTime(pSync, SYNC_CHAN_VID, (pVidInfo->Pts - pSync->VidInfo.DelayTime));

        MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid SetLocalTime %d\n", (pVidInfo->Pts - pSync->VidInfo.DelayTime));
    }

    SYNC_CalcDiffTime(pSync, SYNC_CHAN_VID);

    SYNC_CheckEvent(pSync, SYNC_CHAN_VID);

    SYNC_CheckTBMatchAdjust(pSync);

    pSync->VidLastPts = pVidInfo->Pts;
    pSync->VidLastSrcPts = pVidInfo->SrcPts;

    if (pSync->CrtBufStatus.bOverflowDiscFrm)
    {
        /*discard one frame in every VID_SMOOTH_DISCARD_INTERVAL frame*/
        if (!(pSync->VidDisPlayCnt % VID_SMOOTH_DISCARD_INTERVAL))
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_DISCARD;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid Buf overflow, Discard\n");
        }
        else
        {
            pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

            MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid Buf overflow, Play\n");
        }

        pSync->VidDisPlayCnt++;

        *pVidOpt = pSync->VidOpt;
        MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid Sync Proc %d--------\n", pVidOpt->SyncProc);
        MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");
        return;
    }

    if (MT_UNF_SYNC_REF_NONE == pSync->SyncAttr.enSyncRef)
    {
        if (!pSync->VidFirstPlay)
        {
            pSync->VidFirstPlay = MT_TRUE;
            pSync->VidFirstPlayTime = SYNC_GetSysTime();
            mt_drv_stat_event(STAT_EVENT_FRAMESYNCOK, 0);
        }

        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
        *pVidOpt = pSync->VidOpt;
        MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid NoneSync Play--------\n");
        MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");
        return;
    }

    /* do sync when  LoopBackFlag is false */
    if (MT_FALSE == pSync->LoopBackFlag)
    {
        SYNC_SyncAdjust(pSync, SYNC_CHAN_VID);
    }

    /* get information , need to change VidOpt.SyncProc to SYNC_PROC_PLAY , then adjust pcr*/
    if ((SYNC_PROC_DISCARD == pSync->VidOpt.SyncProc)
        && (SYNC_BUF_STATE_EMPTY == pSync->CrtBufStatus.VidBufState)
        )
    {
        pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;

        if (MT_TRUE == pSync->PcrSyncInfo.PcrAdjustDeltaOK)
        {
            /*adjust pcr to this one which is more behind*/
            if (abs(pSync->VidAudDiff) < VID_LEAD_DISCARD_THRESHOLD)
            {
                pSync->PcrSyncInfo.PcrDelta += (pSync->VidAudDiff > 0) ? pSync->PcrSyncInfo.AudPcrDiff : pSync->PcrSyncInfo.VidPcrDiff;
                pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_FALSE;

                MT_INFO_SYNC(">>>>Vid buf low, change PcrDelta %d \n", pSync->PcrSyncInfo.PcrDelta);
            }
        }
    }

    *pVidOpt = pSync->VidOpt;

    if (SYNC_PROC_PLAY == pVidOpt->SyncProc)
    {
        if (!pSync->VidFirstPlay)
        {
            pSync->VidFirstPlay = MT_TRUE;
            pSync->VidFirstPlayTime = SYNC_GetSysTime();
            mt_drv_stat_event(STAT_EVENT_FRAMESYNCOK, 0);
        }
    }

    MT_INFO_VSYNC(SYNC_CHAN_VID, "--------Vid Sync Proc %d ,TBA:%d--------\n", pVidOpt->SyncProc, pVidOpt->enTBAdjust);
    MT_INFO_VSYNC(SYNC_CHAN_VID, "\n");

    return;
}


mt_void SYNC_AudProc(mt_handle hSync, SYNC_AUD_INFO_S *pAudInfo, SYNC_AUD_OPT_S *pAudOpt)
{
    SYNC_S              *pSync;
    mt_upts              PtsDelta;
    mt_u32              SysTime;

    if (MT_FALSE == SYNC_VerifyHandle(hSync))
    {
       // return;
    }

    //pSync = g_SyncGlobalState.SyncInfo[hSync&0xff].pSync;
    pSync = g_SyncGlobalState.SyncInfo[0].pSync;
    pAudOpt->SyncProc = SYNC_PROC_PLAY;
    pAudOpt->SpeedAdjust = SYNC_AUD_SPEED_ADJUST_NORMAL;

    pSync->AudOpt = *pAudOpt;

    /* record audio frame information */
    pSync->AudInfo = *pAudInfo;

    SysTime = SYNC_GetSysTime();

    MT_INFO_SYNC("A: [%x] [%llx] [%x] [%llx][%llx]\n",pSync->AudFirstCome,  pAudInfo->Pts, pAudInfo->FrameNum,  pSync->AudFirstPts, pSync->AudLastPts);
    //MT_INFO_ASYNC(SYNC_CHAN_AUD, "AudInfo SrcPts %-8d, Pts %-8d, FrameTime %d, BufTime %-4d, FrameNum %d, SysTime %d\n", pAudInfo->SrcPts, pAudInfo->Pts, pAudInfo->FrameTime, pAudInfo->BufTime, pAudInfo->FrameNum, SysTime);

    /*can't do(no effect) smooth adjust when do sync without FRC(frame rate convertion)*/
    if (!pSync->bUseStopRegion)
    {
        pSync->SyncAttr.stSyncStartRegion.bSmoothPlay = MT_FALSE;
        pSync->SyncAttr.stSyncNovelRegion.bSmoothPlay = MT_FALSE;
    }

    if (SYNC_SCR_ADJUST_BUTT == pSync->PcrSyncInfo.enPcrAdjust)
    {
        if (MT_TRUE == SYNC_CheckPcrTimeout(pSync))
        {
            pSync->PcrSyncInfo.enPcrAdjust = SYNC_AUD_ADJUST_SCR;

            MT_INFO_SYNC("Pcr is timeout adjust pcr by audpts\n");
        }
    }

    if (!pSync->AudFirstCome)
    {
        pSync->AudFirstCome = MT_TRUE;
        pSync->AudFirstSysTime = SYNC_GetSysTime();
        pSync->AudFirstPts = pAudInfo->Pts;
        pSync->AudLastPts = pAudInfo->Pts;
        pSync->AudLastBufTime = pAudInfo->BufTime;
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "First AudFrame SysTime %d, Pts %d, SrcPts %d\n", pSync->AudFirstSysTime, pSync->AudFirstPts, pAudInfo->SrcPts);
    }

    if (!pSync->AudFirstValidCome && (-1 != pAudInfo->SrcPts))
    {
        pSync->AudFirstValidCome = MT_TRUE;
        pSync->AudFirstValidPts = pAudInfo->SrcPts;

        MT_INFO_VSYNC(SYNC_CHAN_VID, "AudFirstValidPts come: %u\n", pSync->AudFirstValidPts);
    }

    if(pSync->SyncAttr.enSyncRef == MT_UNF_SYNC_REF_AUDIO){
		return;
    }

    if (!pSync->PreSyncFinish)
    {
        /* test the value again after local_irq_save to prevent being preempted by vo interrupt */
        if (!pSync->PreSyncFinish)
        {
            SYNC_PreSync(pSync, SYNC_CHAN_AUD);
            if (pSync->AudOpt.SyncProc != SYNC_PROC_CONTINUE)
            {
                pSync->AudLastPts = pAudInfo->Pts;
                pSync->AudLastBufTime = pAudInfo->BufTime;
                *pAudOpt = pSync->AudOpt;

                if (pSync->PreSyncTargetInit)
                {
                    MT_INFO_ASYNC(SYNC_CHAN_AUD, "--------Aud PreSync VidAudDiff %d Proc %d Speed %d--------\n", pSync->VidAudDiff, pAudOpt->SyncProc, pAudOpt->SpeedAdjust);
                }

                return;
            }
        }
    }

    /* first set scr */
    if(pSync->PreSyncFinish && !pSync->ScrInitFlag)
    {
        if(-1 != pAudInfo->Pts)
        {
            SYNC_SetLocalTime(pSync, SYNC_CHAN_SCR, (pAudInfo->Pts - pAudInfo->BufTime));
            pSync->ScrInitFlag = MT_TRUE;
            pSync->ScrFirstLocalTime = pAudInfo->Pts - pAudInfo->BufTime;
            pSync->ScrFirstSysTime = SYNC_GetSysTime();
        }
    }

    if (!pSync->BufFundFinish)
    {
        /* test the value again after local_irq_save to prevent being preempted by vo interrupt */
        if (!pSync->BufFundFinish)
        {
            SYNC_BufFund(pSync);
            if (pSync->AudOpt.SyncProc != SYNC_PROC_CONTINUE)
            {
                *pAudOpt = pSync->AudOpt;
                MT_INFO_ASYNC(SYNC_CHAN_AUD, "--------Aud BufFund Proc %d Speed %d--------\n", pAudOpt->SyncProc, pAudOpt->SpeedAdjust);
                MT_INFO_ASYNC(SYNC_CHAN_AUD, "\n");
                return;
            }

            SYNC_SetLocalTime(pSync, SYNC_CHAN_AUD, (pAudInfo->Pts - pAudInfo->BufTime));

            if (SYNC_AUD_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
            {
               SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR, (pAudInfo->Pts - pAudInfo->BufTime ));

               MT_INFO_SYNC("Pcr SetLocalTime %d  by Audpts\n", (pAudInfo->Pts - pAudInfo->BufTime ));
            }

            if (pSync->VidFirstCome)
            {
                SYNC_SetLocalTime(pSync, SYNC_CHAN_VID, (pSync->VidInfo.Pts - pSync->VidInfo.DelayTime));
                MT_INFO_VSYNC(SYNC_CHAN_VID, "Vid BufFund First SetLocalTime %d\n", (pSync->VidInfo.Pts - pSync->VidInfo.DelayTime));
            }

            MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud BufFund First SetLocalTime %d\n", (pAudInfo->Pts - pAudInfo->BufTime));
        }
    }

	/*AudDDPMode do not set audio resynchronization*/
    if (pSync->AudDDPMode)
    {
        pSync->AudReSync = MT_FALSE;
    }

    /* audio resynchronization is needed when change audio track.StopAud->StartAud*/
    if (pSync->AudReSync)
    {
        SYNC_AudReSync(pSync);
        if (pSync->AudOpt.SyncProc != SYNC_PROC_CONTINUE)
        {
            pSync->AudLastPts = pAudInfo->Pts;
            pSync->AudLastBufTime = pAudInfo->BufTime;
            *pAudOpt = pSync->AudOpt;
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "--------Aud ReSync VidAudDiff %d Proc %d Speed %d--------\n", pSync->VidAudDiff, pAudOpt->SyncProc, pAudOpt->SpeedAdjust);
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "\n");
            return;
        }
    }

    /* it is needed to accumulate audio data again */
    if (pSync->AudReBufFund)
    {
        SYNC_AudReBufFund(pSync);
        if (pSync->AudOpt.SyncProc != SYNC_PROC_CONTINUE)
        {
            *pAudOpt = pSync->AudOpt;
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "--------Aud ReBufFund Proc %d Speed %d--------\n", pAudOpt->SyncProc, pAudOpt->SpeedAdjust);
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "\n");
            return;
        }

        if (SYNC_AUD_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
        {
           SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR, (pAudInfo->Pts - pAudInfo->BufTime));

           MT_INFO_SYNC("Pcr SetLocalTime %d  by Audpts\n", (pAudInfo->Pts - pAudInfo->BufTime));
        }

        SYNC_SetLocalTime(pSync, SYNC_CHAN_AUD, (pAudInfo->Pts - pAudInfo->BufTime));

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud ReBufFund First SetLocalTime %d\n", (pAudInfo->Pts - pAudInfo->BufTime));
    }

    if ((MT_UNF_SYNC_REF_PCR == pSync->SyncAttr.enSyncRef) && (!pSync->AudFirstPlay))
    {
        if ((MT_FALSE == pSync->VidFirstCome) && (MT_FALSE== SYNC_CheckAudTimeout(pSync)))
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_REPEAT;
            *pAudOpt = pSync->AudOpt;
            return;
        }
    }

    /* pts gets smaller*/
    if (-1 == pAudInfo->Pts)
    {
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud Pts == -1 invalid\n");
        pSync->AudPtsSeriesCnt = 0;
    }
    /* The same audio frame may be sent to sync twice for repeating.So pAudInfo->Pts == pSync->AudLastPts maybe*/
    else if (pAudInfo->Pts < pSync->AudLastPts)
    {
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud Pts < Aud LstPts %d\n", pSync->AudLastPts);
        pSync->AudPtsSeriesCnt = 0;
    }
    else
    {
        PtsDelta = pAudInfo->Pts - pSync->AudLastPts;

        /* PtsDelta is more than FrameTime*/
        if (PtsDelta > pAudInfo->FrameTime)
        {
            MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud PtsDelta %d > Aud FrameTime %d\n", PtsDelta, pAudInfo->FrameTime);
            pSync->AudPtsSeriesCnt = 0;
        }
    }

    pSync->AudPtsSeriesCnt++;

    /* if pts keep successive ,update localtime*/
    if ((pSync->AudPtsSeriesCnt >= PTS_SERIES_COUNT)
        || ((MT_FALSE == pSync->AudLocalTimeFlag) && (-1 != pAudInfo->Pts)))
    {
        //SYNC_SetLocalTime(pSync, SYNC_CHAN_AUD, (pAudInfo->Pts - pAudInfo->BufTime));

        if (SYNC_AUD_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
        {
           //SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR, (pAudInfo->Pts - pAudInfo->BufTime));

           MT_INFO_SYNC("Pcr SetLocalTime %d  by Audpts\n", (pAudInfo->Pts - pAudInfo->BufTime));
        }

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud SetLocalTime %d\n", (pAudInfo->Pts - pAudInfo->BufTime));
    }

    SYNC_CalcDiffTime(pSync, SYNC_CHAN_AUD);

    SYNC_CheckEvent(pSync, SYNC_CHAN_AUD);

    pSync->AudLastPts = pAudInfo->Pts;
    pSync->AudLastBufTime = pAudInfo->BufTime;

    if (MT_UNF_SYNC_REF_NONE == pSync->SyncAttr.enSyncRef)
    {
        if (!pSync->AudFirstPlay)
        {
            pSync->AudFirstPlay = MT_TRUE;
            pSync->AudFirstPlayTime = SYNC_GetSysTime();
        }

        if (SYNC_BUF_STATE_EMPTY == pSync->CrtBufStatus.AudBufState)
        {
            pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_DOWN;
        }
        else
        {
            pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_NORMAL;
        }

        pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;
        *pAudOpt = pSync->AudOpt;
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "--------Aud NoneSync Play Speed %d--------\n", pAudOpt->SpeedAdjust);
        MT_INFO_ASYNC(SYNC_CHAN_AUD, "\n");
        return;
    }

    /* do sync when  LoopBackFlag is false */
    if (MT_FALSE == pSync->LoopBackFlag)
    {
        //SYNC_SyncAdjust(pSync, SYNC_CHAN_AUD);
                //pSync->VidOpt.SyncProc = SYNC_PROC_PLAY;
                pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

    }

    /* If there isn't enough audio data,low the audio output speed*/
    if (SYNC_BUF_STATE_EMPTY == pSync->CrtBufStatus.AudBufState)
    {
        if (SYNC_PROC_DISCARD == pSync->AudOpt.SyncProc)
        {
            pSync->AudOpt.SyncProc = SYNC_PROC_PLAY;

            if (MT_TRUE == pSync->PcrSyncInfo.PcrAdjustDeltaOK)
            {
                if (abs(pSync->VidAudDiff) < VID_LEAD_DISCARD_THRESHOLD)
                {
                    /*adjust pcr to this one which is more behind*/
                    pSync->PcrSyncInfo.PcrDelta += (pSync->VidAudDiff > 0) ? pSync->PcrSyncInfo.AudPcrDiff : pSync->PcrSyncInfo.VidPcrDiff;
                    pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_FALSE;

                    MT_INFO_SYNC(">>>>Aud buf low, change PcrDelta %d \n", pSync->PcrSyncInfo.PcrDelta);
                }
            }
        }

        if(SYNC_AUD_SPEED_ADJUST_MUTE_REPEAT != pSync->AudOpt.SpeedAdjust)
        {
            pSync->AudOpt.SpeedAdjust = SYNC_AUD_SPEED_ADJUST_DOWN;
        }

        MT_INFO_ASYNC(SYNC_CHAN_AUD, "Aud Buf Low\n");
    }

    *pAudOpt = pSync->AudOpt;

    if (SYNC_PROC_PLAY == pAudOpt->SyncProc)
    {
        if (!pSync->AudFirstPlay)
        {
            pSync->AudFirstPlay = MT_TRUE;
            pSync->AudFirstPlayTime = SYNC_GetSysTime();
        }
    }

    MT_INFO_ASYNC(SYNC_CHAN_AUD, "--------Aud Sync Proc %d Speed %d--------\n", pAudOpt->SyncProc, pAudOpt->SpeedAdjust);
    MT_INFO_ASYNC(SYNC_CHAN_AUD, "\n");

    return;
}

mt_void SYNC_PcrProc(mt_handle hSync, mt_u32 PcrTime)
{
    SYNC_S    *pSync;
    mt_u32    PcrDelta;
    mt_u32    SysCostTime = 0;

    if (MT_FALSE == SYNC_VerifyHandle(hSync))
    {
       // return;
    }

    //pSync = g_SyncGlobalState.SyncInfo[hSync&0xff].pSync;
    pSync = g_SyncGlobalState.SyncInfo[0].pSync;

    /* first pcr*/
    if (!pSync->PcrSyncInfo.PcrFirstCome)
    {
        if ((MT_FALSE == SYNC_CheckPcrTimeout(pSync))
            || (MT_INVALID_TIME_U64 == pSync->PcrSyncInfo.PcrSyncStartSysTime))
        {
            //fisrt set pcr local  time
            SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR, (PcrTime + pSync->PcrSyncInfo.PcrDelta));
            MT_INFO_SYNC(">>>>Pcr First SetLocalTime %d\n", (PcrTime + pSync->PcrSyncInfo.PcrDelta));
            pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_TRUE;
            pSync->PcrSyncInfo.enPcrAdjust = SYNC_PCR_ADJUST_SCR;
        }

        //second change PcrFirstCome value to MT_TRUE
        pSync->PcrSyncInfo.PcrFirstCome = MT_TRUE;
        pSync->PcrSyncInfo.PcrFirstSysTime = SYNC_GetSysTime();
        pSync->PcrSyncInfo.PcrFirst = PcrTime;
        pSync->PcrSyncInfo.PcrLast = PcrTime;

        return;
    }

    /* pcr gets smaller*/
    if (PcrTime <= pSync->PcrSyncInfo.PcrLast)
    {
        pSync->PcrSyncInfo.PcrLast = PcrTime;
        pSync->PcrSyncInfo.PcrSeriesCnt = 0;
        return;
    }

    PcrDelta = PcrTime - pSync->PcrSyncInfo.PcrLast;

    /* pcr jump*/
    if (PcrDelta > PCR_MAX_DELTA)
    {
        pSync->PcrSyncInfo.PcrLast = PcrTime;
        pSync->PcrSyncInfo.PcrSeriesCnt = 0;
        return;
    }

    pSync->PcrSyncInfo.PcrLast = PcrTime;

    pSync->PcrSyncInfo.PcrSeriesCnt++;

    /* If pcr keep successivem,update the pcr localtime*/
    if (pSync->PcrSyncInfo.PcrSeriesCnt >= PTS_SERIES_COUNT)
    {
        SysCostTime = SYNC_GetSysTimeCost(pSync->PcrSyncInfo.PcrLastSysTime);

        if (SysCostTime != 0)
        {
            pSync->PcrSyncInfo.PcrGradient = PcrDelta * 100 / SysCostTime;
        }

        if (SYNC_PCR_ADJUST_SCR == pSync->PcrSyncInfo.enPcrAdjust)
        {
            SYNC_SetLocalTime(pSync, SYNC_CHAN_PCR, (PcrTime + pSync->PcrSyncInfo.PcrDelta));

            pSync->PcrSyncInfo.PcrAdjustDeltaOK = MT_TRUE;

            MT_INFO_SYNC(">>>>PcrTime %d, PcrDelta %d \n", PcrTime, pSync->PcrSyncInfo.PcrDelta);

            MT_INFO_SYNC(">>>>Pcr SetLocalTime %d by Pcr\n", (PcrTime + pSync->PcrSyncInfo.PcrDelta));
        }
    }

    return;
}

mt_s32 SYNC_reference_config(SYNC_S *pSync)
{
    avsync_priv_t *priv = NULL;
    mt_s32 ret = MT_SUCCESS;

    if (NULL == pSync)
    {
        MT_ERR_SYNC("pSync is null\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    switch (pSync->SyncAttr.enSyncRef)
    {
		case MT_UNF_SYNC_REF_AUDIO:
			avsync_set_sync_mode(priv, AVSYNC_REF_AUDIO_MODE);
			avsync_set_user_sync_mode(priv, AVSYNC_REF_AUDIO_MODE);
            if (pSync->SyncAttr.enStreamType == MT_UNF_AVPLAY_STREAM_TYPE_TS)
            {
                MT_INFO_SYNC("Sync Ref -> [TS]%s\n","AUDIO");
            }
            else
            {
                MT_INFO_SYNC("Sync Ref -> [ES]%s\n","AUDIO");
            }
			
            break;

        case MT_UNF_SYNC_REF_PCR:
            MT_INFO_SYNC("Sync Ref -> %s\n","PCR");
			avsync_set_sync_mode(priv, AVSYNC_REF_PCR_MODE);
			avsync_set_user_sync_mode(priv, AVSYNC_REF_PCR_MODE);
            break;

        case MT_UNF_SYNC_REF_SCR:
            MT_INFO_SYNC("Sync Ref -> %s\n","SCR");
            MT_INFO_SYNC("not support MT_UNF_SYNC_REF_SCR\n");
            break;

        case MT_UNF_SYNC_REF_NONE:
            MT_INFO_SYNC("Sync Ref -> %s, AV FREERUN\n","NONE");
			avsync_set_sync_mode(priv, AVSYNC_REF_NONE_MODE);
			avsync_set_user_sync_mode(priv, AVSYNC_REF_NONE_MODE);
            break;

        case MT_UNF_SYNC_REF_VIDEO:
            MT_INFO_SYNC("Sync Ref -> [TS]%s\n","VIDEO");
			avsync_set_sync_mode(priv, AVSYNC_REF_VIDEO_MODE);
			avsync_set_user_sync_mode(priv, AVSYNC_REF_VIDEO_MODE);
            break;

        default:
			avsync_set_sync_mode(priv, AVSYNC_REF_AUDIO_MODE);
			avsync_set_user_sync_mode(priv, AVSYNC_REF_AUDIO_MODE);
            ret = MT_ERR_SYNC_INVALID_PARA;
            MT_INFO_SYNC("Sync Ref(%d) -> %s, set to ref aud\n",pSync->SyncAttr.enSyncRef,"UNKONWN REF");
            break;
    }

    return ret;
}


/**
 * @brief Apply Firmware Configuration
 *    Applay Sync DRV's Configuration to Firmware.
 */
mt_void SYNC_ApplyConfig(SYNC_S *pSync)
{
	MT_INFO_SYNC("SmoothPlay: %d\n",pSync->SyncAttr.stSyncStartRegion.bSmoothPlay);

	if (pSync->SyncAttr.stSyncStartRegion.bSmoothPlay)
	{
		avsync_set_slow_sync_policy(1);
	}
	else
	{
		avsync_set_slow_sync_policy(0);
	}

    avsync_get_current_psync(pSync);
}

mt_u64 avsync_read_tsc(void)
{
    u64 ns = 0;

    ns = sched_clock();
    do_div(ns, 1000);
    return ns;
}

mt_u32 avsync_osal_get_tick(void)
{
    return (mt_u32)ktime_to_ms(ktime_get_boottime());
}

void avsync_private_reset(avsync_priv_t *priv)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return;
    }

    memset((mt_u8 *)&priv->async, 0, sizeof(struct async_t));
    memset((mt_u8 *)&priv->vsync, 0, sizeof(struct vsync_t));
    memset((mt_u8 *)&priv->async_info, 0, sizeof(struct avsync_async_info_t));
    memset((mt_u8 *)&priv->vsync_info, 0, sizeof(struct avsync_vsync_info_t));
    memset((mt_u8 *)&priv->pcr_info, 0, sizeof(struct pcr_info_t));

    priv->pcr_info.pcr_probe_start_tick = 0xffffffff;
    priv->init_stc                      = 0;
    priv->avsync_sync_mode              = priv->avsync_cfg_info.avsync_sync_mode;
    priv->cur_apts                      = 0xffffffff;
    priv->cur_vpts                      = 0xffffffff;
    priv->cur_stc                       = 0xffffffff;
    priv->cur_pcr                       = 0xffffffff;
    priv->fast_mode_in_adjust           = 0;
    priv->pcr_fast_offset               = 0;
    priv->pcr_bad_cnt                   = 0;
    priv->reliable_pcr                  = 0;
    priv->async.latest_parsed_apts      = 0xffffffff;
    priv->async.latest_parsed_tick      = AVSYNC_MAX_64BIT_VALUE;
    priv->async.first_parsed_tick       = AVSYNC_MAX_64BIT_VALUE;
    priv->vsync.latest_parsed_vpts      = 0xffffffff;
    priv->apts_bad_cnt                  = 0;
    priv->stc_offset                    = AVSYNC_PCR_SET_STC_DELAY;
    priv->apts_total_cnt                = 0;
    priv->apts_total_cnt_before_sw_sync_mode = 0;
    priv->pcr_info.latest_tick          = AVSYNC_MAX_64BIT_VALUE;
    priv->first_apts                    = 0xffffffff;
    priv->async.last_apts               = 0xffffffff;
    priv->first_vpts                    = 0xffffffff;
    priv->vsync.last_vpts               = 0xffffffff;
    priv->vsync.vpts                    = 0xffffffff;
	priv->audio_bypass_apts_adjust       = 0x4b0;	//bit15: 0-decrease, 1-increase; bit14-bit0:the value, default decrease 1200ms
}

enum COMPARE_RESULT avsync_compare_slope(mt_u64 prev_tick,
                                         mt_u64 tick,
                                         mt_u32 prev_pcr,
                                         mt_u32 pcr,
                                         mt_u32 *slope)
{
    mt_u32 sys_slope = AVSYNC_C0COUNTER_SLOPE;
    mt_u32 new_slope = 0;
    mt_u64 tick_dif;

    if (NULL == slope)
    {
        MT_ERR_SYNC("param err\n");
        return ALMOST_EQUAL;
    }

    if (pcr == prev_pcr)
    {
        MT_ERR_SYNC("same pcr:%#x\n", pcr);
        return MUCH_MORE_THAN;
    }

    tick_dif = tick - prev_tick;
    do_div(tick_dif, pcr - prev_pcr);
    new_slope = (mt_u32)tick_dif;
    *slope = new_slope;

    /*
     * AVSYNC_C0COUNTER_SLOPE is a fixed value, if the stream is lined, the new
     * slope must be equal to AVSYNC_C0COUNTER_SLOPE, if the dif between the
     * new slpoe and AVSYNC_C0COUNTER_SLOPE is great AVSYNC_SLOPE_THRESHOLD,
     * means the pcr is unreliable
     */
    if (((new_slope >= sys_slope) &&
         ((new_slope - sys_slope) < AVSYNC_SLOPE_THRESHOLD)) ||
        ((new_slope < sys_slope) &&
         ((sys_slope - new_slope) < AVSYNC_SLOPE_THRESHOLD)))
    {
        return ALMOST_EQUAL;
    }

    return MUCH_MORE_THAN;
}

enum COMPARE_RESULT avsync_do_compare(avsync_priv_t *priv, mt_u32 pcr, mt_u32 stc)
{
    enum COMPARE_RESULT ret      = ALMOST_EQUAL;
    mt_u8 big_than               = 0;
    mt_u32 temp                  = 0;
    mt_u32 equal_threshold       = 0;
    mt_u32 little_diff_threshold = 0;
    mt_u32 much_diff_threshold   = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return ret;
    }

    equal_threshold       = ALMOST_EQUAL_THRESHOLD;
    little_diff_threshold = LITTLE_DIFFERENT_THRESHOLD;
    much_diff_threshold   = DIFFERENT_THRESHOLD;

    if (pcr > stc)
    {
        temp = pcr - stc;
        big_than = 1;
    }
    else
    {
        temp = stc - pcr;
        big_than = 0;
    }

    if (temp <= equal_threshold)
    {
        ret = ALMOST_EQUAL;
    }
    else if ((equal_threshold < temp) && (temp <= little_diff_threshold))
    {
        ret = big_than ? LITTLE_MORE_THAN : LITTLE_LESS_THAN;
    }
    else if ((little_diff_threshold < temp) && (temp <= much_diff_threshold))
    {
        ret = big_than ? MORE_THAN : LESS_THAN;
    }
    else
    {
        ret = big_than ? MUCH_MORE_THAN : MUCH_LESS_THAN;
    }

    return ret;
}

mt_s32 avsync_stc_set(avsync_priv_t *priv, mt_u32 stc)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if (AVSYNC_REF_PCR_MODE == priv->avsync_sync_mode)
    {
        stc -= priv->stc_offset;
    }

    symphony_stc_disable();
    symhony_avsync_set_stc_cnt_ini_base_value(stc);
    symphony_stc_reload();
    symphony_stc_enable();
    SYNC_LOGT_DEBUG("new stc = %#x\n", stc);
    return MT_SUCCESS;
}

mt_s32 avsync_stc_get(mt_u32 *pstc)
{
    if (NULL == pstc)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    *pstc = (mt_u32)symphony_stc_get();
    return MT_SUCCESS;
}

mt_s32 avsync_stc_pause(void)
{
    symphony_stc_disable();
    return MT_SUCCESS;
}

mt_s32 avsync_stc_resume(void)
{
    symphony_stc_enable();
    return MT_SUCCESS;
}

mt_s32 avsync_stc_init(avsync_priv_t *priv, mt_u32 stc)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    avsync_stc_set(priv, stc);
    priv->init_stc = 1;
    SYNC_LOGT_DEBUG("init stc = %#x\n", stc);
    return MT_SUCCESS;
}

void avsync_stc_adjust_by_pcr(avsync_priv_t *priv, mt_u32 pcr)
{
    mt_u32 stc                  = 0;
    mt_u32 set_stc              = 0;
    mt_u32 diff                 = 0;
    enum COMPARE_RESULT cmp_rlt = ALMOST_EQUAL;
    mt_u32 last_pcr             = 0;
    struct pcr_info_t *ppcr_info = NULL;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return;
    }

    ppcr_info = &priv->pcr_info;

    if ((0 == pcr) || (0xffffffff == pcr))
    {
        MT_ERR_SYNC("err pcr:%#x\n", pcr);
        return;
    }

    if (!priv->init_stc)
    {
        priv->stc_reference = TM_PCR;
        avsync_stc_init(priv, pcr);
        avsync_pcr_set_new_pcr(priv, pcr, avsync_read_tsc());
        SYNC_LOGT_DEBUG("Init stc by pcr[%08x]\n", pcr);
        return;
    }

    last_pcr       = priv->last_pcr;
    priv->last_pcr = pcr;

    if ((pcr < last_pcr) && (last_pcr - pcr >= 60 * 1000 * STC_CNT_PER_MS))
    {
        SYNC_LOGP_DEBUG(" May stream roll back now1. pcr from [%#x] to [%#x]\n", last_pcr, pcr);
        priv->stream_total_time  = ppcr_info->latest_pcr - pcr;
        priv->pcr_info.bpcr_loop = 1;
    }
    else if (((pcr < last_pcr) && (last_pcr - pcr > 800 * STC_CNT_PER_MS)) &&
             (last_pcr != 0xffffffff) && ((last_pcr & 0xffff0000) == 0xffff0000))
    {
        SYNC_LOGP_DEBUG("May stream roll back now2. pcr from [%#x] to [%#x]\n", last_pcr, pcr);
        avsync_private_reset(priv);
        priv->stream_total_time  = ppcr_info->latest_pcr - pcr;
        priv->pcr_info.bpcr_loop = 1;
    }
    else if ((pcr < ppcr_info->latest_pcr) &&
             (ppcr_info->latest_pcr - pcr > 800 * STC_CNT_PER_MS))
    {
        SYNC_LOGP_DEBUG("May stream roll back now3. pcr from [%#x] to [%#x]\n",
                          ppcr_info->latest_pcr,
                          pcr);
        priv->stream_total_time = ppcr_info->latest_pcr - pcr;
        ppcr_info->bpcr_loop    = 1;
    }

    if (avsync_pcr_reliable_check(priv, pcr))
    {
        if (priv->pcr_bad_cnt > 0)
        {
            priv->pcr_bad_cnt--;
        }

        priv->apts_total_cnt_before_sw_sync_mode = 0;
    }
    else
    {
        priv->pcr_info.pcr_unreliable_total++;
        SYNC_LOGP_DEBUG("pcr is unreliable. pcr 0x%08x, %d - %d - %d\n", pcr,
            priv->pcr_info.pcr_unreliable_total, priv->pcr_bad_cnt,
            priv->apts_bad_cnt);

        /*
         * if pcr is bad too much, check the apts bad counter, if the number of
         * bad apts is less than  APTS_BAD_THRESHOLD, change the sync mode to
         * audio master, if not, means apts is bad too, do not change to aduio
         * master. But more tests are needed to determine the number of
         * APTS_BAD_THRESHOLD.
         */
        if ((++priv->pcr_bad_cnt) > PCR_BAD_THRESHOLD)
        {
            if (!priv->apts_total_cnt_before_sw_sync_mode)
            {
                priv->apts_total_cnt_before_sw_sync_mode = priv->apts_total_cnt;
            }
            else
            {
                if (priv->apts_total_cnt >= (priv->apts_total_cnt_before_sw_sync_mode + 3))
                {
                    if (priv->apts_bad_cnt < APTS_BAD_THRESHOLD)
                    {
                        avsync_pcr_switch_to_audio_master(priv);
                        priv->async.stat.switch_audio_reason = 8;
                        SYNC_LOGP_DEBUG("PCR bad, change to audio master[%d], pcr-apts bad:[%d][%d]\n",
                            priv->async.stat.switch_audio_reason, priv->pcr_bad_cnt,
                            priv->apts_bad_cnt);
                        priv->pcr_bad_cnt                    = 0;
                    }
                    else
                    {
                        SYNC_LOGP_DEBUG("PCR bad, pcr-apts bad:[%d][%d]\n", priv->pcr_bad_cnt,
                            priv->apts_bad_cnt);
                    }

                    priv->apts_total_cnt_before_sw_sync_mode = 0;
                }
            }
        }

        return;
    }

    avsync_stc_get(&stc);
    stc += priv->stc_offset;
    priv->cur_stc = stc;

    if (priv->calc_pcr_time_start == 0)
    {
        priv->calc_pcr_time_start = avsync_osal_get_tick();
    }

    if (avsync_osal_get_tick() > priv->calc_pcr_time_start + 10 * 1000)
    {
        priv->pcr_reduce_cnt      = 0;
        priv->calc_pcr_time_start = avsync_osal_get_tick();
    }

    /*
     * priv->reliable_pcr replaced priv->last_pcr, because if the priv->last_pcr is bigger than
     * pcr, sync mode may be switch to audio master, this case had been tested when playing the
     * conax certification stream
     */
    if ((pcr < priv->reliable_pcr) && (pcr + STC_CNT_PER_MS * 1000 > priv->reliable_pcr))
    {
        priv->pcr_reduce_cnt++;
        SYNC_LOGP_DEBUG("pcr reduce, pcr:%#x, reli_pcr:%#x, cnt:%d\n", pcr, priv->reliable_pcr, priv->pcr_reduce_cnt);
    }

    priv->reliable_pcr = pcr;

    if (0)//(priv->pcr_reduce_cnt >= 3)
    {
        avsync_pcr_switch_to_audio_master(priv);
        priv->async.stat.switch_audio_reason = 7;
        SYNC_LOGP_DEBUG("PCR reduce, change to audio master[%d]\n", priv->async.stat.switch_audio_reason);
        return;
    }

    cmp_rlt = avsync_do_compare(priv, pcr, stc);

    switch (cmp_rlt)
    {
        case ALMOST_EQUAL:
        {
            break;
        }

        case LITTLE_LESS_THAN:
        case LITTLE_MORE_THAN:
        {
            diff = 0;

            if ((mt_u8)cmp_rlt & 0x01) // pcr < stc
            {
                diff = (stc - pcr);
                set_stc = stc - (diff >> 1);
            }
            else
            {
                diff = (pcr - stc);
                set_stc = stc + (diff >> 1);
            }

            avsync_stc_set(priv, set_stc);
            SYNC_LOGT_DEBUG("pcr: little diff (%s%d)ms, pcr: %#x, stc:%#x, set:%#x\n",
                         (pcr > stc) ? "+" : "-",
                         (pcr > stc) ? (pcr - stc) / STC_CNT_PER_MS : (stc - pcr) / STC_CNT_PER_MS,
                         pcr, stc, set_stc);
            break;
        }

        default:
        {
            set_stc = pcr;
            avsync_stc_set(priv, set_stc);
            SYNC_LOGT_DEBUG("pcr: big diff (%s%d)ms, pcr: %#x, stc:%#x, set:%#x\n",
                         (pcr > stc) ? "+" : "-",
                         (pcr > stc) ? (pcr - stc) / STC_CNT_PER_MS : (stc - pcr) / STC_CNT_PER_MS,
                         pcr, stc, set_stc);
            break;
        }
    }

    return;
}

mt_s32 avsync_stc_adjust_by_apts(avsync_priv_t *priv, struct pts_node_t *pnode)
{
    mt_u32 stc       = 0;
    mt_u32 delta     = 0;
    mt_u32 last_apts = 0;
    mt_u32 frm_dura  = 0;
    mt_s32 ret       = MT_SUCCESS;

    if ((NULL == priv) || (NULL == pnode))
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if (0xffffffff == pnode->pts)
    {
        MT_ERR_SYNC("apts ERR!\n");
        return MT_FAILURE;
    }

    if (!priv->init_stc)
    {
        priv->stc_reference = TM_APTS;
        avsync_stc_init(priv, pnode->pts);
        SYNC_LOGT_DEBUG("stc inited by apts[%#x]\n", pnode->pts);
        return ret;
    }

    frm_dura  = pnode->pts_step;
    ret       = avsync_stc_get(&stc);
	if (priv->async_info.audio_bypass)
	{
		last_apts = priv->audio_bypass_last_apts;
	}
	else
	{
    	last_apts = priv->isr_pop.pts;
	}

    if (((pnode->pts > last_apts) && ((pnode->pts - last_apts) > (frm_dura + APTS_STC_DIFF_THRD_MAX))) ||
        ((pnode->pts < last_apts) && ((last_apts - pnode->pts) > (frm_dura + APTS_STC_DIFF_THRD_MAX))))
    {
        SYNC_LOGT_DEBUG("apts jump, adjust stc, apts[%#x](new stc), last_apts[%#x], frm[%d], dif[%d]ms\n", pnode->pts, last_apts,
            frm_dura/STC_CNT_PER_MS, abs(pnode->pts - last_apts)/STC_CNT_PER_MS);

        avsync_stc_set(priv, pnode->pts);
    }
    else
    {
        mt_u32 tmp_diff = 0;

        tmp_diff = ((frm_dura > (80 * STC_CNT_PER_MS)) ? ((frm_dura * 2) + (frm_dura / 2))
                                                        : (80 * STC_CNT_PER_MS));
        delta = (stc > pnode->pts) ? (stc - pnode->pts) : (pnode->pts - stc);

        if (delta > tmp_diff)
        {
            avsync_stc_set(priv, pnode->pts);
            SYNC_LOGT_DEBUG("adjust stc by apts, stc[%x], apts[%x](new stc), frm[%d], dif[%d]ms\n",
                stc, pnode->pts, frm_dura/STC_CNT_PER_MS, delta/STC_CNT_PER_MS);
        }
    }

    return ret;
}

mt_s32 avsync_stc_adjust_by_vpts(avsync_priv_t *priv)
{
    mt_u32 stc       = 0;
    mt_u32 delta     = 0;
    mt_u32 vpts      = 0;
    mt_u32 last_vpts = 0;
    mt_u32 frm_dura  = 0;
    mt_s32 ret       = MT_SUCCESS;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if (0xffffffff == priv->cur_vpts)
    {
        MT_ERR_SYNC("vpts ERR!\n");
        return MT_FAILURE;
    }

    if (!priv->init_stc)
    {
        priv->stc_reference = TM_VPTS;
        avsync_stc_init(priv, priv->cur_vpts);
        SYNC_LOGT_DEBUG("stc inited by vpts[%#x]\n", priv->cur_vpts);
        return ret;
    }

    frm_dura  = priv->vpts_step;
    ret       = avsync_stc_get(&stc);
    vpts      = priv->cur_vpts;
    last_vpts = priv->vsync.last_vpts;

    if (((vpts > last_vpts) && ((vpts - last_vpts) > (frm_dura + APTS_STC_DIFF_THRD_MAX))) ||
        ((vpts < last_vpts) && ((last_vpts - vpts) > (frm_dura + APTS_STC_DIFF_THRD_MAX))))
    {
        SYNC_LOGT_DEBUG("vpts jump, adjust stc, vpts[%#x], last_vpts[%#x], frm[%d], dif[%d]ms\n",
            vpts, last_vpts, frm_dura/STC_CNT_PER_MS, abs(vpts - last_vpts)/STC_CNT_PER_MS);
        avsync_stc_set(priv, vpts);
    }
    else
    {
        mt_u32 tmp_diff = 0;

        tmp_diff = ((frm_dura > (80 * STC_CNT_PER_MS)) ? ((frm_dura * 2) + (frm_dura / 2))
                                                        : (80 * STC_CNT_PER_MS));
        delta = (stc > vpts) ? (stc - vpts) : (vpts - stc);

        if (delta > tmp_diff)
        {
            avsync_stc_set(priv, vpts);
            SYNC_LOGT_DEBUG("adjust stc by vpts, stc[%x], vpts[%x], frm[%d], dif[%d]ms\n",
                stc, vpts, frm_dura/STC_CNT_PER_MS, delta/STC_CNT_PER_MS);
        }
    }

    return ret;
}

mt_s32 avsync_pcr_switch_to_audio_master(avsync_priv_t *priv)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    avsync_set_sync_mode(priv, AVSYNC_REF_AUDIO_MODE);
    return MT_SUCCESS;
}

void avsync_pcr_set_new_pcr(avsync_priv_t *priv, mt_u32 pcr, mt_u64 cpu_tick)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return;
    }
    priv->pcr_info.last_reliable_pcr = priv->pcr_info.latest_pcr = pcr;
    priv->pcr_info.last_reliable_tick = priv->pcr_info.latest_tick = cpu_tick;
}

mt_s32 avsync_pcr_check_nextpcr_segment(avsync_priv_t *priv, mt_u8 *nextpcr_err, mt_u32 pcr)
{
    mt_u32 pcr_diff = 0;

    if ((NULL == priv) || (NULL == nextpcr_err))
    {   
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if (priv->pcr_info.last_pcr >= pcr)
    {
        pcr_diff = priv->pcr_info.last_pcr - pcr;
    }
    else
    {
        pcr_diff = pcr - priv->pcr_info.last_pcr;
    }

    if (((priv->pcr_info.last_pcr & 0xfff00000) != (pcr & 0xfff00000))
        && (pcr_diff > 0xf0000))
        *nextpcr_err = 1;
    else
        *nextpcr_err = 0;

    return MT_SUCCESS;
}

MT_BOOL avsync_pcr_reliable_check(avsync_priv_t *priv, mt_u32 pcr)
{
    mt_u64 cur_tick = 0;
    struct pcr_info_t *ppcr_info = NULL;
    mt_u8 pcr_reliable = 0;
    mt_u32 new_slope = 0;
    MT_BOOL ret = MT_FALSE;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return ret;
    }

    ppcr_info = &priv->pcr_info;
    cur_tick = avsync_read_tsc();    

    /* compare pcr with last reliable pcr, check whether pcr is reliable */
    if (ALMOST_EQUAL == avsync_compare_slope(ppcr_info->last_reliable_tick,
                                             cur_tick,
                                             ppcr_info->last_reliable_pcr,
                                             pcr,
                                             &new_slope))
    {
        pcr_reliable = 1;
        ret          = MT_TRUE;
        SYNC_LOGP_DEBUG("pcr: %#x - %#x, tick:%#llx - %#llx, slope:%d\n",
            pcr, ppcr_info->last_reliable_pcr, cur_tick, ppcr_info->last_reliable_tick, new_slope);
    }
    else
    {
        /* compare pcr with last pcr, check whether pcr is reliable */
        ppcr_info->pcr_reliable   = 0;
        if ((ppcr_info->latest_pcr != ppcr_info->last_reliable_pcr) &&
            (ALMOST_EQUAL == avsync_compare_slope(ppcr_info->latest_tick,
                                                  cur_tick,
                                                  ppcr_info->latest_pcr,
                                                  pcr,
                                                  &new_slope)))
        {
            pcr_reliable = 1;
            ret          = MT_TRUE;
            SYNC_LOGP_DEBUG("pcr: %#x - %#x, tick:%#llx - %#llx, slope:%d\n",
                pcr, ppcr_info->latest_pcr, cur_tick, ppcr_info->latest_tick, new_slope);
        }
        else
        {
            ret = MT_FALSE;
            SYNC_LOGP_DEBUG("pcr: %#x - %#x, tick:%#llx - %#llx, slope:%d\n",
                pcr, ppcr_info->latest_pcr, cur_tick, ppcr_info->latest_tick, new_slope);
        }
    }

    if (pcr_reliable)
    {
        ppcr_info->pcr_reliable       = 1;
        ppcr_info->last_reliable_pcr  = pcr;
        ppcr_info->last_reliable_tick = cur_tick;
    }

    ppcr_info->latest_pcr  = pcr;
    ppcr_info->latest_tick = cur_tick;
    return ret;
}

mt_s32 avsync_pcr_check_avpts_pcr_segment(avsync_priv_t *priv,
                                        mt_u8 *pbneed_change_sync_mode,
                                        mt_u32 pcr)
{
    if ((NULL == priv) || (NULL == pbneed_change_sync_mode))
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if ((((priv->vsync.latest_parsed_vpts & 0xfff00000) != (pcr & 0xfff00000))
        || ((priv->async.latest_parsed_apts & 0xfff00000) != (pcr & 0xfff00000)))
        && ((priv->vsync.latest_parsed_vpts & 0xfffc0000) == (priv->async.latest_parsed_apts & 0xfffc0000)))
    {
        *pbneed_change_sync_mode = 1;
    }
    else
    {
        *pbneed_change_sync_mode = 0;
    }

    return MT_SUCCESS;
}

mt_s32 avsync_pcr_process_isr(avsync_priv_t *priv, mt_u32 pcr)
{
    mt_u8 bneed_change_sync_mode      = 0;
    static mt_u8 need_change_mode_cnt = 0;
    mt_u8 nextpcr_err                 = 0;
    static mt_u8 nextpcr_err_cnt      = 0;
    static mt_u8 nextpcr_succuess_cnt = 0;
    mt_u32 ts_apts                    = 0;
    mt_u32 ts_vpts                    = 0;
    mt_u32 stc                        = 0;
    mt_u32 hw_stc                     = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return !MT_SUCCESS;
    }

    if ((0 == pcr) || (0xffffffff == pcr))
    {
        MT_ERR_SYNC("err pcr:%#x\n", pcr);
        return !MT_SUCCESS;
    }

    priv->pcr_info.total_pcr_cnt++;
    priv->pcr_info.pcr_got = 1;

    /*
    1.if get the same pcr, drop it and return directly
    2.if the diff between new pcr and pre pcr is too small ( < 10ms), drop it and return
    directly
    */
    if (((pcr >= priv->last_pcr) &&
         (pcr < priv->last_pcr + AVSYNC_PCR_DIFF_TOO_SMALL * STC_CNT_PER_MS)) ||
        ((pcr < priv->last_pcr) &&
         (priv->last_pcr < pcr + AVSYNC_PCR_DIFF_TOO_SMALL * STC_CNT_PER_MS)))
    {
        SYNC_LOGP_DEBUG("same pcr or similar pcr:%#x, lst:%#x\n", pcr, priv->last_pcr);
        return !MT_SUCCESS;
    }

    avsync_stc_get(&hw_stc);
    stc = hw_stc;

    if (AVSYNC_REF_PCR_MODE != priv->avsync_sync_mode)
    {
        return !MT_SUCCESS;
    }

    avsync_pcr_check_nextpcr_segment(priv, &nextpcr_err, pcr);

    if (nextpcr_err)
    {
        nextpcr_err_cnt++;

        if (nextpcr_err_cnt > 5)
        {
            nextpcr_err_cnt = 0;
            SYNC_LOGP_DEBUG("pcr and lastpcr in different segment, chage sync mode to audio"
                              "pcr:0x%x lastpcr:0x%x\n", pcr, priv->pcr_info.last_pcr);
            avsync_pcr_switch_to_audio_master(priv);
            priv->async.stat.switch_audio_reason = 2;
            return MT_SUCCESS;
        }

        nextpcr_succuess_cnt = 0;
    }
    else
    {
        nextpcr_succuess_cnt++;

        if (nextpcr_succuess_cnt > 5)
        {
            nextpcr_err_cnt = 0;
        }
    }

    priv->pcr_info.last_pcr = pcr;
    priv->cur_pcr           = pcr;
    priv->vsync.latest_parsed_vpts = symphony_ts_vpts_get();
    ts_apts = priv->async.latest_parsed_apts;
    ts_vpts = priv->vsync.latest_parsed_vpts;

    if (ts_apts && ts_vpts && (-1 != ts_vpts) && (-1 != ts_apts))
    {
        // check 3 times, if pts and pcr in different segment, force sync mode to audio
        if (0 == priv->pcr_info.check_pts_pcr_cnt)
            need_change_mode_cnt = 0;

        if (priv->pcr_info.check_pts_pcr_cnt < 3)
        {
            avsync_pcr_check_avpts_pcr_segment(priv, &bneed_change_sync_mode, pcr);

            if (bneed_change_sync_mode)
            {
                need_change_mode_cnt++;
            }

            priv->pcr_info.check_pts_pcr_cnt++;
        }

        if (priv->pcr_info.check_pts_pcr_cnt && need_change_mode_cnt)
        {
            SYNC_LOGP_DEBUG("pts and pcr in different segment, chage sync mode to audio,"
                              "pcr-apts-vpts: %#x - %#x - %#x\n",
                              pcr,
                              priv->async.latest_parsed_apts,
                              priv->vsync.latest_parsed_vpts);

            avsync_pcr_switch_to_audio_master(priv);
            priv->async.stat.switch_audio_reason = 3;
            priv->hold_video_num_before_audio_output = 4;
            return MT_SUCCESS;
        }
    }

    avsync_stc_adjust_by_pcr(priv, pcr);

    return MT_SUCCESS;
}

MT_BOOL avsync_audio_pts_reliable_check(avsync_priv_t *priv, mt_u32 apts)
{
    mt_u64 cur_tick = 0;
    mt_u32 new_slope = 0;
    MT_BOOL ret = MT_FALSE;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return ret;
    }

    cur_tick = avsync_read_tsc();

    if (ALMOST_EQUAL == avsync_compare_slope(priv->async.latest_parsed_tick,
                                             cur_tick,
                                             priv->async.latest_parsed_apts,
                                             apts,
                                             &new_slope))
    {
        ret = MT_TRUE;
    }
    else
    {
        ret = MT_FALSE;
    }

    SYNC_LOGP_DEBUG("apts:%#x - %#x, tick:%#llx - %#llx, slope: %d\n",
        apts, priv->async.latest_parsed_apts, 
        cur_tick, priv->async.latest_parsed_tick, new_slope);

    return ret;
}

mt_s32 avsync_audio_pts_parsed_isr(avsync_priv_t *priv)
{
    mt_u32 apts = 0;
    mt_u64 tick = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    apts = (mt_u32)symphony_ts_apts_get();
    tick = avsync_read_tsc(); 

    if ((AVSYNC_MAX_32BIT_VALUE != priv->async.latest_parsed_apts)
        &&(AVSYNC_MAX_64BIT_VALUE != priv->async.latest_parsed_tick))
    {
        if (avsync_audio_pts_reliable_check(priv, apts))
        {
            if (priv->apts_bad_cnt > 0)
            {
                priv->apts_bad_cnt--;
            }
        }
        else
        {
            priv->apts_bad_cnt++;
        }
    }

    priv->apts_total_cnt++;
    priv->async.latest_parsed_apts = apts;
    priv->async.latest_parsed_tick = tick;

    if (AVSYNC_MAX_64BIT_VALUE == priv->async.first_parsed_tick)
    {
        priv->async.first_parsed_tick = tick;
    }

    if (AVSYNC_REF_PCR_MODE == priv->avsync_sync_mode)
    {
        if ((priv->async.latest_parsed_tick - priv->async.first_parsed_tick) >= 3000000)
        {
            if (!priv->pcr_info.pcr_got)
            {
                avsync_pcr_switch_to_audio_master(priv);
                priv->async.stat.switch_audio_reason = 4;
                SYNC_LOGP_DEBUG("pcr timeout, switch to audio master, tick:%#llx - %#llx\n",
                    priv->async.latest_parsed_tick, priv->async.first_parsed_tick);
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 avsync_audio_pts_fifo_init(avsync_priv_t *priv)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    memset(&priv->apts_fifo, 0, sizeof(struct pts_fifo_t));
    priv->apts_fifo.fifo_len = MAX_PTS_FIFO_NODE_COUNT;
    priv->apts_fifo.fifo_init = 1;
	spin_lock_init(&priv->apts_fifo.fifo_spin_lock);
	return MT_SUCCESS;
}

mt_s32 avsync_audio_push_pts_fifo(struct pts_fifo_t *pfifo, struct pts_node_t *pnode)
{
    struct pts_node_t *pnew = NULL;
    mt_u32 wr = 0;
    mt_u32 rd = 0;
    mt_u32 pts_cnt = 0;

    if ((NULL == pfifo) || (NULL == pnode))
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if (!pfifo->fifo_init)
    {
        MT_ERR_SYNC("apts fifo not init\n");
        return MT_FAILURE;
    }

    wr = pfifo->fifo_wr;
	rd = pfifo->fifo_rd;

    if (wr >= rd)
    {
        pts_cnt = wr - rd;
    }
    else
    {
        pts_cnt = pfifo->fifo_len + wr - rd;
	}

    if (pts_cnt >= pfifo->fifo_len)
    {
        MT_ERR_SYNC("apts fifo full:%d - %d\n", pts_cnt, pfifo->fifo_len);
        return MT_FAILURE;
	}

    pnew = &pfifo->fifo_buf[wr];
    memcpy(pnew, pnode, sizeof(struct pts_node_t));
    wr++;
    wr %= pfifo->fifo_len;
    pfifo->fifo_wr = wr;

	return MT_SUCCESS;
}

mt_s32 avsync_audio_pop_pts_fifo(struct pts_fifo_t *pfifo, struct pts_node_t *pnode)
{
    struct pts_node_t *ppop = NULL;
    mt_u32 wr = 0;
    mt_u32 rd = 0;
    mt_u32 pts_cnt = 0;

    if ((NULL == pfifo) || (NULL == pnode))
    {
        MT_ERR_SYNC("[%s %d]param err\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    if (!pfifo->fifo_init)
    {
        MT_ERR_SYNC("[%s %d]apts fifo not init\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }

    wr = pfifo->fifo_wr;
	rd = pfifo->fifo_rd;

    if (wr >= rd)
    {
        pts_cnt = wr - rd;
    }
    else
    {
        pts_cnt = pfifo->fifo_len + wr - rd;
	}

    if (pts_cnt <= 0)
    {
        MT_ERR_SYNC("[%s %d]apts fifo empty, fifo_len=%d, pts_cnt=%d, wr=%d, rd=%d\n", __FUNCTION__, __LINE__, pfifo->fifo_len, pts_cnt, wr, rd);
        return MT_FAILURE;
	}

    ppop = &pfifo->fifo_buf[rd];
    memcpy(pnode, ppop, sizeof(struct pts_node_t));
    rd++;
    rd %= pfifo->fifo_len;
    pfifo->fifo_rd = rd;

	return MT_SUCCESS;
}

mt_u32 avsync_audio_calc_pts_fifo(avsync_priv_t *priv)
{
    mt_u32 wr = 0;
    mt_u32 rd = 0;
    mt_u32 pts_cnt = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return 0;
    }

    if (!priv->apts_fifo.fifo_init)
    {
        MT_ERR_SYNC("apts fifo not init\n");
        return 0;
    }

    wr = priv->apts_fifo.fifo_wr;
    rd = priv->apts_fifo.fifo_rd;

    if (wr >= rd)
    {
        pts_cnt = wr - rd;
    }
    else
    {
        pts_cnt = priv->apts_fifo.fifo_len - wr + rd;
    }

    return pts_cnt;
}

mt_void avsync_audio_pop_pts(mt_void)
{
    avsync_priv_t *priv = NULL;
    struct pts_fifo_t *pts_fifo = NULL;
    struct pts_node_t pts_node;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return;
    }

    pts_fifo = &(priv->apts_fifo);
    avsync_audio_pop_pts_fifo(pts_fifo, &pts_node);
}
EXPORT_SYMBOL(avsync_audio_pop_pts);

mt_s32 avsync_audio_push_pts(SYNC_S  *pSync, SYNC_PUSH_APTS_S *pSyncApts)
{
    avsync_priv_t *priv = NULL;
    struct pts_fifo_t *pts_fifo = NULL;
    struct pts_node_t pts_node;
    SYNC_PUSH_APTS_S *push_apts = NULL;
    mt_u32 apts = 0;
    mt_u32 i = 0;
    mt_s32  ret = MT_SUCCESS;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    pts_fifo = &(priv->apts_fifo);

    if (NULL == pSyncApts)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    push_apts = pSyncApts;

#ifdef SYNC_LOGA_PUSH
    {
        static mt_u32 pre_pts = 0;
        static mt_u32 pre_id = 0;

        SYNC_LOGA_PUSH("push pts: %#x, id: %d, dif = %d - %d, step: %d\n", 
            push_apts->pts_u32, push_apts->id,
            (push_apts->pts_u32 - pre_pts)/45, push_apts->id - pre_id,
            push_apts->step_u32);
        pre_pts = push_apts->pts_u32;
        pre_id = push_apts->id;
    }
#endif

    memset(&pts_node, 0, sizeof(struct pts_node_t));
	if (priv->do_ddp_verf &&
        ((1536 == priv->aud_frame_sample_num) ||
         (256 == priv->aud_frame_sample_num) ||
         (1792 == priv->aud_frame_sample_num) ||
		 (HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)))	
	{
        apts = push_apts->pts_u32;
        //avsync_audio_adjust_apts_for_ddp_verfication(priv, &apts);

        for (i = 0; i < 4; i++)
        {
            pts_node.pts = apts + i*(push_apts->step_u32 >> 2);
            pts_node.pts_id = (push_apts->id << 2) + i;
            pts_node.pts_step = (push_apts->step_u32 >> 2);
            ret = avsync_audio_push_pts_fifo(pts_fifo, &pts_node);

            if (MT_SUCCESS != ret)
            {
                MT_ERR_SYNC("push apts failed, ret:%#x\n", ret);
            }
			else
			{
				priv->apts_fifo_push_cnt++;
				
				priv->apts_fifo_push_pts = pts_node.pts;
			}
        }
    }
    else
    {    	
        pts_node.pts = push_apts->pts_u32;
        pts_node.pts_id = push_apts->id;
        pts_node.pts_step = push_apts->step_u32;

        ret = avsync_audio_push_pts_fifo(pts_fifo, &pts_node);

        if (ret) 
        {
            MT_ERR_SYNC("push apts failed, ret:%#x\n", ret);
        }
		else
		{
			priv->apts_fifo_push_cnt++;
		}
    }

    return ret;
}
EXPORT_SYMBOL(avsync_audio_push_pts);

mt_s32 avsync_audio_one_frame_out_isr(avsync_priv_t *priv)
{
    struct pts_node_t pts_node;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 tk = 0;
    static mt_u32  pre_tk = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    memset(&pts_node, 0, sizeof(struct pts_node_t));
    ret = avsync_audio_pop_pts_fifo(&priv->apts_fifo, &pts_node);

    if (ret != MT_SUCCESS)
    {
        MT_ERR_SYNC("isr pop apts failed\n");
        return MT_FAILURE;
    }
	
    priv->cur_play_apts = pts_node.pts;
	
    tk = avsync_osal_get_tick();
    SYNC_LOGA_POP("pop:%#x - %d, atdif:%d - %d\n", 
        pts_node.pts, pts_node.pts_id, pts_node.pts - priv->isr_pop.pts, tk - pre_tk);
	
    pre_tk = tk;

    if (AVSYNC_REF_AUDIO_MODE == priv->avsync_sync_mode)
    {
        avsync_stc_adjust_by_apts(priv, &pts_node);
    }

    memcpy(&priv->isr_pop, &pts_node, sizeof(struct pts_node_t));
    priv->apts_fifo_pop_cnt++;
    return MT_SUCCESS;
}

static mt_s32 avsync_aduio_pts_rollback_event(avsync_priv_t *priv)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    if (priv->do_ddp_verf &&
        ((1536 == priv->aud_frame_sample_num) ||
         (256 == priv->aud_frame_sample_num) ||
         (1792 == priv->aud_frame_sample_num) ||
		 (HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)))    
    {
	//	priv->dolby_sync_done = 0;
		printk(KERN_ERR "[%s %d]aud roll back dolby_sync_done \n", __FUNCTION__, __LINE__);
		avsync_audio_init(0);
		avsync_audio_sync_status_reset(priv);
		avsync_video_sync_status_reset(priv);
		priv->vid_dolby_ply_cnt = 0;
		priv->atoms_looped = 1;
    }
	else
	{	
	    if (AVSYNC_REF_PCR_MODE != priv->avsync_sync_mode)
	    {
			if(is_liveplay())
			{
		        avsync_audio_sync_status_reset(priv);
		        avsync_video_sync_status_reset(priv);
		        avsync_set_sync_mode(priv, AVSYNC_REF_AV_MODE);
				
	        	printk(KERN_ERR "[%s %d]stream roll back, set to av ref\n", __FUNCTION__, __LINE__);
			}
	    }
	}
    return MT_SUCCESS;
}

static mt_s32 avsync_aduio_pts_stable_check(avsync_priv_t *priv)
{
    struct async_t *pasync = NULL;
    mt_u32 pts_diff = 0x00;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return MT_FAILURE;
    }

    pasync = &priv->async;

    if (0xffffffff == pasync->last_apts)
    {
        MT_INFO_SYNC("A stbld: first apts, [%#x][%d]\n", priv->cur_apts, priv->cur_apts_id);
        priv->astabled_flag = 1;
        return MT_SUCCESS;
	}

	if (0xffffffff == priv->first_apts)
	{
		if(priv->cur_apts != 0)
		{
			priv->first_apts = priv->cur_apts;
		}
	}
	
    if (priv->cur_apts > pasync->last_apts)
    {
        pts_diff = priv->cur_apts - pasync->last_apts;

        if (pts_diff <= ((priv->apts_step << 1) + 0x300))
        {
            if (priv->astabled_cnt > AVSYNC_MAX_AUD_READY_NUMBER)
            {
                if (!priv->astabled_flag)
                {
                    MT_INFO_SYNC("A stbld:[%#x][%d]\n", priv->cur_apts, priv->cur_apts_id);
                }

                priv->astabled_flag = 1;
            }
            else
            {
                priv->astabled_cnt++;
            }
        }
        else
        {            
            if (priv->astabled_flag)
            {
                MT_INFO_SYNC("A unstbld1:[%#x][%d]\n", priv->cur_apts, priv->cur_apts_id);
            }
			
			priv->astabled_cnt = 0;
            priv->astabled_flag = 0;
        }
    }
    else if ((priv->cur_apts + 0x100) < pasync->last_apts)
    {        
        if (priv->astabled_flag)
        {
            MT_INFO_SYNC("A unstbld2:[%#x][%d]\n", priv->cur_apts, priv->cur_apts_id);
        }

		priv->astabled_cnt = 0;
        priv->astabled_flag = 0;
    }
    else
    {
        //keep the pre apts status
    }

    return MT_SUCCESS;
}

static mt_s32 avsync_aduio_pts_event_check(avsync_priv_t *priv)
{
    struct async_t *pasync = NULL;

    if (NULL == priv)
    {
        return MT_FAILURE;
    }

    pasync = &priv->async;

    if (priv->p_avsync_psync)
    {
        if ((priv->cur_apts < pasync->last_apts) && (0xffffffff != pasync->last_apts) &&
            ((pasync->last_apts - priv->cur_apts) > AVSYNC_STREAM_ROLLABCK_DIFF))
        {
            priv->avsync_aud_rollback = 1;
            SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_AUD_ROLLBACK);
            avsync_aduio_pts_rollback_event(priv);
            SYNC_LOGAP("aud rollback\n");
        }

        if ((0xffffffff != pasync->last_apts) &&
            (abs(priv->cur_apts - pasync->last_apts) >= (SYNC_PTS_JUMP_FRM_NUM * priv->apts_step)))
        {
            SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_APTS_JUMP);
            SYNC_LOGAP("apts jump, pts(cur: %#x, pre: %#x, diff=%d ms), step: %d, id(cur: %d, pre: %d)\n", priv->cur_apts, 
                pasync->last_apts, abs(pasync->last_apts-priv->cur_apts)/45, priv->apts_step, priv->cur_apts_id, pasync->last_apts_id);
        }
    }

    return MT_SUCCESS;
}

static mt_s32 avsync_aduio_pts_check(avsync_priv_t *priv)
{
    struct async_t *pasync = NULL;
    static mt_u32 pre_tick = 0;
    mt_u32 tick = 0;

    if (NULL == priv)
    {
        return MT_FAILURE;
    }

    pasync = &priv->async;
    avsync_aduio_pts_event_check(priv);
    avsync_aduio_pts_stable_check(priv);
    tick = avsync_osal_get_tick();
    SYNC_LOGAP("apts: %#x, atdif: %d - %d, id: %d\n", priv->cur_apts,
        (priv->cur_apts - pasync->last_apts)/45, tick - pre_tick, priv->cur_apts_id);
    pre_tick = tick;
    pasync->last_apts = priv->cur_apts;
    pasync->last_apts_id = priv->cur_apts_id;
    return MT_SUCCESS;
}

mt_s32 avsync_audio_adjust_apts_for_ddp_verfication(avsync_priv_t *priv, u32 *pts)
{
	priv->pts_offset = 0;
	
    if ((NULL == priv))
    {
        return !MT_SUCCESS;
    }    

	priv->dolby_adjust = 0;//default don't adjust according to pts when it has synced.
	
	switch (priv->ddp_prog)
    {   
    	case AVSYNC_H264_DD_2500_PG1:		
		{
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3020*45;
			}
			else
			{
				priv->pts_offset = 3020*45;
			}
		}		
		break;

		case AVSYNC_H264_DD_2500_PG2:		
		{
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3000*45;
			}
			else
			{
				priv->pts_offset = 2980*45;
			}
		}		
		break;
		
        case AVSYNC_H264_DDP_2500_PG1:
        {      
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3025*45;
			}
			else
			{
				priv->pts_offset = 3020*45;
			}
        }
        break;
		
        case AVSYNC_H264_DDP_2500_PG2:
        {      
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 12005*45;
			}
			else
			{
				priv->pts_offset = 11985*45;
			}
        }
        break;

		case AVSYNC_MPEG2_DD_2500_PG1:
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3015*45;
			}
			else
          	{
				priv->pts_offset = 3030*45;
			}
        }
        break;
		
		case AVSYNC_MPEG2_DD_2500_PG2:	
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 2990*45;
			}
			else
          	{
				priv->pts_offset = 2980*45;
			}
        }
        break;

		case AVSYNC_MPEG2_DDP_2500_PG1:
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3020*45;
			}
			else
          	{
				priv->pts_offset = 3020*45;
			}
        }
        break;

		case AVSYNC_MPEG2_DDP_2500_PG2:
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 2980*45;
			}
			else
           	{
				priv->pts_offset = 2990*45;
			}
        }
        break;        

        case AVSYNC_H264_DD_2997_PG1:
        {
			priv->pts_offset = 3050*45;
        }
        break;

		case AVSYNC_H264_DD_2997_PG2:
        {
			priv->pts_offset = 3030*45;
        }
        break;		

		case AVSYNC_H264_DDP_2997_PG1:
        {
        	//this program must adjust, otherwise it can't pass at 2:22~3:00(the stream's time)
        	//i don't know the reason at present.
        	priv->dolby_adjust = 1;
			if ((MT_DRV_DISP_FMT_3840X2160_60 == priv->av_play_info.tvformat) ||
				(MT_DRV_DISP_FMT_4096X2160_60 == priv->av_play_info.tvformat))
        	{
        		priv->pts_offset = 3055*45;
        	}
			else
			{
				priv->pts_offset = 3040*45;
			}
        }
        break;
		
        case AVSYNC_H264_DDP_2997_PG2:
        {
        	if ((MT_DRV_DISP_FMT_3840X2160_60 == priv->av_play_info.tvformat) ||
				(MT_DRV_DISP_FMT_4096X2160_60 == priv->av_play_info.tvformat))
        	{
        		priv->pts_offset = 12035*45;
        	}
			else
			{
				priv->pts_offset = 12025*45;
			}
        }
        break;

		//----the below is used for TB44----
		case AVSYNC_MPEG2_DDP_2500_ATMOS:  
		{
				if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
					priv->pts_offset = 3200*45;
				else if(AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
					priv->pts_offset = 3000*45;
				else
					priv->pts_offset = 2950*45;
		}
		break;

		case AVSYNC_H264_DDP_2500_ATMOS: 
		{
			if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				if(1 == priv->atoms_looped)
				{
					priv->pts_offset = 3225*45;
				}
				else
				{
					priv->pts_offset = 3215*45;
				}
			}
			else if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				if(1 == priv->atoms_looped)
				{
					priv->pts_offset = 2980*45;
				}
				else
				{
					priv->pts_offset = 2980*45;
				}
			}
			else
			{
				if(1 == priv->atoms_looped)
				{
					priv->pts_offset = 2940*45;
				}
				else
				{
					priv->pts_offset = 2980*45;
				}
				
			}
		}
		break;

		case AVSYNC_H265_DDP_2500_ATMOS:
		{	
			if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 3200*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2950*45;
			}
			else
			{
				priv->pts_offset = 2980*45;
			}
		}
		break;
		
		case AVSYNC_H265_DDP_5000_ATMOS:
		{
			if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 3225*45;
			}
			else
			{
				if(1 == priv->atoms_looped)
				{
					if(AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
						priv->pts_offset = 3000*45;
					else
						priv->pts_offset = 2980*45;
				}
				else
				{
					priv->pts_offset = 2980*45;
				}
			}
		}
		break;	
		
		case AVSYNC_H264_DDP_2997_ATMOS:  
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 1990*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 1950*45;
			}
			else
			{
				priv->pts_offset = 2200*45;
			}
		}
		break;

		case AVSYNC_H265_DDP_2997_ATMOS: 
		{
			if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2960*45;
			}
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 3200*45;
			}	
			else
			{
				priv->pts_offset = 3000*45;
			}	
		}
		break;
		
		case AVSYNC_H265_DDP_5994_ATMOS:
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				if(1 == priv->atoms_looped)
					priv->pts_offset = 2990*45;
				else
					priv->pts_offset = 2990*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{				
				if(1 == priv->atoms_looped)
				{
					priv->pts_offset = 2950*45;
				}
				else
				{
					priv->pts_offset = 2970*45;
				}
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 3200*45;
			}	
		}
		break;

		case AVSYNC_H264_AC4_2997_ATMOS:    
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 2980*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 1905*45;
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 1965*45;
			}	
        }
        break;

		case AVSYNC_H265_AC4_2997_ATMOS:     
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 2970*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2920*45;
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 2955*45;
			}	
        }
        break;

		case AVSYNC_H265_AC4_5994_ATMOS:      
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 2980*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2930*45;
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				if(1 == priv->atoms_looped)
				{
					priv->pts_offset = 2960*45;
				}
				else
				{
					priv->pts_offset = 2980*45;
				}
			}	
        }
        break;

		case AVSYNC_H264_AC4_2500_ATMOS: 
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 2950*45;
				
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2930*45;
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 2960*45;
			}	
        }
        break;

		case AVSYNC_H265_AC4_2500_ATMOS: 
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 2970*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2910*45;
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 2965*45;
			}	
        }
        break;

		case AVSYNC_H265_AC4_5000_ATMOS:    
		{
			if (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
			{
				priv->pts_offset = 2970*45;
			}
			else if (AVSYNC_AUD_OUT_SPDIF == priv->avsync_aud_output)
			{
				priv->pts_offset = 2930*45;
			}	
			else if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
			{
				priv->pts_offset = 2980*45;
			}	
        }
        break;
		
		case AVSYNC_H264_DDP_5000_ATMOS:				
		case AVSYNC_H264_AC4_5000_ATMOS:   
		{
			if (AVSYNC_AUD_OUT_MAT == priv->avsync_aud_output)
				priv->pts_offset = 3000*45;
			else
				priv->pts_offset = 3000*45;
        }
        break;
		//--------------------------------

        default:
        break;
    }    

	*pts += priv->pts_offset;
	
    return MT_SUCCESS;
}

mt_s32 avsync_audio_adjust_apts_for_ddp_verfication_no_hdmi(avsync_priv_t *priv, u32 *pts)
{
	priv->pts_offset = 0;
	
    if ((NULL == priv))
    {
        return !MT_SUCCESS;
    }    

	priv->dolby_adjust = 0;//default don't adjust according to pts when it has synced.
	
	switch (priv->ddp_prog)
    {   
    	case AVSYNC_H264_DD_2500_PG1:		
		{
			if ((MT_DRV_DISP_FMT_3840X2160_50 == priv->av_play_info.tvformat)
			  ||(MT_DRV_DISP_FMT_4096X2160_50 == priv->av_play_info.tvformat))
			{
				priv->pts_offset = 3020*45;
			}
			else if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3020*45;
			}
			else
			{
				priv->pts_offset = 3015*45;
			}
		}		
		break;

    	case AVSYNC_H264_DD_2500_PG2:		
		{
			if ((MT_DRV_DISP_FMT_3840X2160_50 == priv->av_play_info.tvformat)
			  ||(MT_DRV_DISP_FMT_4096X2160_50 == priv->av_play_info.tvformat))
			{
				priv->pts_offset = 2990*45;
			}
			else if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 2990*45;
			}
			else
			{
				priv->pts_offset = 2985*45;
			}
		}		
		break;
		
        case AVSYNC_H264_DDP_2500_PG1:
        {   
        	if ((MT_DRV_DISP_FMT_3840X2160_50 == priv->av_play_info.tvformat)
			  ||(MT_DRV_DISP_FMT_4096X2160_50 == priv->av_play_info.tvformat))
			{
				priv->pts_offset = 3020*45;
			}
			else if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3025*45;
			}
			else
			{
				priv->pts_offset = 3010*45;
			}
        }
        break;
		
        case AVSYNC_H264_DDP_2500_PG2:
        {  
        	if ((MT_DRV_DISP_FMT_3840X2160_50 == priv->av_play_info.tvformat)
			  ||(MT_DRV_DISP_FMT_4096X2160_50 == priv->av_play_info.tvformat))
			{
				priv->pts_offset = 11985*45;
			}
			else if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 11985*45;
			}
			else
			{
				priv->pts_offset = 11990*45;
			}
        }
        break;

		case AVSYNC_MPEG2_DD_2500_PG1:
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3020*45;
			}
			else
          	{
				priv->pts_offset = 3010*45;
			}
        }
        break;

		case AVSYNC_MPEG2_DD_2500_PG2:	
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3000*45;
			}
			else
          	{
				priv->pts_offset = 2995*45;
			}
        }
        break;

        case AVSYNC_MPEG2_DDP_2500_PG1:
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 3020*45;
			}
			else
          	{
				priv->pts_offset = 3015*45;
			}
        }
        break;

		case AVSYNC_MPEG2_DDP_2500_PG2:
        {
			if(MT_DRV_DISP_FMT_1080i_50 == priv->av_play_info.tvformat)
			{
				priv->pts_offset = 2980*45;
			}
			else
           	{
				priv->pts_offset = 2990*45;
			}
        }
        break;        

        case AVSYNC_H264_DD_2997_PG1:
        {
			priv->pts_offset = 3045*45;
        }
        break;

		case AVSYNC_H264_DD_2997_PG2:
        {
			priv->pts_offset = 3010*45;
        }
        break;		

		case AVSYNC_H264_DDP_2997_PG1:
        {
        	//this program must adjust, otherwise it can't pass at 2:22~3:00(the stream's time)
        	//i don't know the reason at present.
        	priv->dolby_adjust = 1;
			if ((MT_DRV_DISP_FMT_3840X2160_60 == priv->av_play_info.tvformat) ||
				(MT_DRV_DISP_FMT_4096X2160_60 == priv->av_play_info.tvformat))
        	{
        		priv->pts_offset = 3045*45;
        	}
			else
			{
				priv->pts_offset = 3040*45;
			}			
        }
        break;
		
        case AVSYNC_H264_DDP_2997_PG2:
        {    
        	if ((MT_DRV_DISP_FMT_3840X2160_60 == priv->av_play_info.tvformat) ||
				(MT_DRV_DISP_FMT_4096X2160_60 == priv->av_play_info.tvformat))
        	{
        		priv->pts_offset = 12020*45;
        	}
			else
			{
				priv->pts_offset = 12030*45;
			}
        }
        break;

        case AVSYNC_MPEG2_DDP_2500_ATMOS:
        case AVSYNC_H264_DDP_2500_ATMOS:
		case AVSYNC_H264_DDP_5000_ATMOS:	
        case AVSYNC_H264_DDP_2997_ATMOS:

		case AVSYNC_H265_DDP_2500_ATMOS:			
        case AVSYNC_H265_DDP_5000_ATMOS:
		case AVSYNC_H265_DDP_2997_ATMOS:	
        case AVSYNC_H265_DDP_5994_ATMOS:
			
		case AVSYNC_H264_AC4_2500_ATMOS:
		case AVSYNC_H264_AC4_5000_ATMOS:
		case AVSYNC_H264_AC4_2997_ATMOS:

		case AVSYNC_H265_AC4_2500_ATMOS:
		case AVSYNC_H265_AC4_5000_ATMOS:	
		case AVSYNC_H265_AC4_2997_ATMOS:
		case AVSYNC_H265_AC4_5994_ATMOS:       
		{
			priv->pts_offset = 3000*45;
        }
        break;

        default:
        break;
    }    

	*pts += priv->pts_offset;
	
    return MT_SUCCESS;
}

mt_s32 avsync_audio_sync_status_reset(avsync_priv_t *priv)
{
    priv->a_synced_flag = 0;
	priv->astabled_cnt = 0;
	priv->astabled_flag = 0;
	priv->a_unsynced_cnt = 0;
    priv->av_synced_flag = 0;
	priv->dolby_sync_done = 0;
	return MT_SUCCESS;
}

mt_void avsync_audio_es_level_check(avsync_priv_t *priv)
{
    mt_u32 es_id = 0;
    ulong es_addr = 0;
    ulong es_size = 0;
    ulong desc_addr = 0;
    ulong desc_size = 0;
    mt_u32 es_wr = 0;
    mt_u32 es_rd = 0;
    mt_u32 es_len = 0;
    mt_u32 desc_wr = 0;
    mt_u32 desc_rd = 0;
    mt_u32 desc_len = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return;
    }

    if (!(priv->avsync_loglevel & SYNC_AUDIO_DATA_INFO))
    {
        return;
    }

    if ((SYNC_DATA_SOURCE_TUNER != priv->data_source_mode)
        && (SYNC_DATA_SOURCE_PVR != priv->data_source_mode))
    {
        return;
    }

    if (dmx_get_audio_es_buf_id(&es_id))
    {
        return;
    }

    if (DMXOsiChnEsDescBufGetAddrSize(es_id, &es_addr, &es_size, &desc_addr, &desc_size))
    {
        return;
    }

    DMXOsiChnEsDataGetReadWrite(es_id, &es_wr, &es_rd);
    DMXOsiChnEsDescDataGetReadWrite(es_id, &desc_wr, &desc_rd);

    if (es_wr >= es_rd)
    {
        es_len = es_wr - es_rd;
    }
    else
    {
        es_len = es_size - es_rd + es_wr;
    }

    if (desc_wr >= desc_rd)
    {
        desc_len = desc_wr - desc_rd;
    }
    else
    {
        desc_len = desc_size - desc_rd + desc_wr;
    }

    SYNC_LOGA_DATA("AES: %#x/%#lx, wd:%#x - %#x, ADESC:%#x/%#lx, wd:%#x - %#x\n",
        es_len, es_size, es_wr, es_rd, desc_len, desc_size, desc_wr, desc_rd);

    return;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_play_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PLAY;

    if (!priv->a_synced_flag)
    {
        priv->a_unsynced_cnt = 0;
        SYNC_LOGA_SYNCED("A synced, apts: %#x, id: %d\n", priv->cur_apts, priv->cur_apts_id);
    }

    priv->a_synced_flag = 1;
    //SYNC_LOGA_SYNCED("AUD synced\n");

    if (!priv->av_synced_flag && priv->v_synced_flag && priv->a_synced_flag)
    {
        priv->av_synced_flag = 1;
        priv->disp_flag = 1;

        if (priv->avsync_do_aud_track)
        {
            priv->avsync_do_aud_track = 0;
			SYNC_LOGV_SYNCED("synced, aud_track, change sync mode to %d(0:none,1:audio,2:video,3:pcr,4:av)\n", priv->avsync_cfg_info.avsync_sync_mode);
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        if (priv->avsync_do_trick_seek)
        {
            priv->avsync_do_trick_seek = 0;
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        if (priv->avsync_aud_rollback)
        {
            priv->avsync_aud_rollback = 0;			
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        if (priv->av_play_flag)
        {
            priv->av_play_flag = 0;
			SYNC_LOGV_SYNCED("synced, change sync mode to %d(0:none,1:audio,2:video,3:pcr,4:av)\n", priv->avsync_cfg_info.avsync_sync_mode);
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        SYNC_LOGA_SYNCED("AVSYNC_FINISHED\n");
		if(0 == priv->normal_sync_done)
		{
			mt_drv_stat_event(STAT_EVENT_SYNCDONE,0);
			priv->normal_sync_done = 1;
		}

        if (priv->p_avsync_psync)
        {
            SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_STA_CHANGE);
        }
    }

	return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_pause_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PLAY;

    if (!priv->a_synced_flag)
    {
        if (priv->aud_pause_on)
        {
            sync_flag = AVSYNC_FRAME_PAUSE;
            SYNC_LOGA_PAUSE("AUD pause\n");
        }
        else
        {
            sync_flag = AVSYNC_FRAME_PLAY;
            SYNC_LOGA_FREE("AUD pause off, AUD free\n");
        }
    }
    else
    {
        priv->a_unsynced_cnt++;

        if (priv->a_unsynced_cnt > AVSYNC_UNSYNCED_THRESHOLD)
        {
            SYNC_LOGA_SYNCED("A unsynced, apts: %#x, id: %d, audio sync reset\n", priv->cur_apts, priv->cur_apts_id);
            avsync_audio_sync_status_reset(priv);
        }
    }

	return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_skip_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PLAY;

    if (!priv->a_synced_flag)
    {
        if (priv->aud_skip_on)
        {
            sync_flag = AVSYNC_FRAME_SKIP;
            SYNC_LOGA_SKIP("AUD skip\n");
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
            SYNC_LOGA_FREE("AUD skip off, AUD free\n");
        }
    }
    else
    {
        priv->a_unsynced_cnt++;

        if (priv->a_unsynced_cnt > AVSYNC_UNSYNCED_THRESHOLD)
        {
            SYNC_LOGA_SYNCED("A unsynced, apts: %#x, id: %d, audio sync reset\n", priv->cur_apts, priv->cur_apts_id);
            avsync_audio_sync_status_reset(priv);
        }
    }

	return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_free_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    if (!priv->a_synced_flag)
    {
        SYNC_LOGA_FREE("AUD free: AV Diff(%u) too much! > free thrd(%u)\n",
            abs(priv->cur_apts - priv->cur_vpts)/45, AVSYNC_AV_DIFF_FREE_THRESHOLD/45);
    
        if (priv->avsync_do_aud_track)
        {
            /*
            do audio sync when audio do track, and if av diff is too large,
            finishing audio track directly
            */            
            priv->avsync_do_aud_track = 0;
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }
    }
    else
    {
        priv->a_unsynced_cnt++;
        if (priv->a_unsynced_cnt > AVSYNC_UNSYNCED_THRESHOLD)
        {
            SYNC_LOGA_SYNCED("A unsynced, apts: %#x, id: %d, audio sync reset\n", priv->cur_apts, priv->cur_apts_id);
            avsync_audio_sync_status_reset(priv);
        }
    }

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_check_sync_flag(avsync_priv_t *priv, AVSYNC_FRAME_SYNCFLAG_E sync_flag)
{
    AVSYNC_FRAME_SYNCFLAG_E async_flag = sync_flag;

    switch (async_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
            async_flag = avsync_audio_play_check(priv);
		    break;
        }

        case AVSYNC_FRAME_PAUSE:
        {
            async_flag = avsync_audio_pause_check(priv);
		    break;
        }

        case AVSYNC_FRAME_SKIP:
        {
            async_flag = avsync_audio_skip_check(priv);
		    break;
        }

        case AVSYNC_FRAME_FREE:
        {
            async_flag = avsync_audio_free_check(priv);
		    break;
        }

        default:
        {
            MT_ERR_SYNC("%s, %d, error sync result:%d\n", __func__, __LINE__, sync_flag);
		    break;
        }
    }

    return async_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_ref_sync_flag(avsync_priv_t *priv, mt_u32 apts, mt_u32 vpts)
{
    mt_u32 sync_thresold = 0;
    mt_u32 av_diff = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    if (priv->a_synced_flag)
    {
        sync_thresold = 120*45;
    }
    else
    {
        sync_thresold = 60*45;
    }

    if (apts >= vpts)
    {
        av_diff = apts - vpts;

        if (av_diff <= sync_thresold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
        }
        else if ((av_diff > sync_thresold) && (av_diff <= AVSYNC_AV_DIFF_FREE_THRESHOLD))
        {
			if (priv->aud_pause_on)
	        {
	            sync_flag = AVSYNC_FRAME_PAUSE;
	        }
	        else
	        {
	            sync_flag = AVSYNC_FRAME_PLAY;
	        }
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
        }
    }
    else
    {
        av_diff = vpts - apts;

        if (av_diff <= sync_thresold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
        }
        else if ((av_diff > sync_thresold) && (av_diff <= AVSYNC_AV_DIFF_FREE_THRESHOLD))
        {
            sync_flag = AVSYNC_FRAME_SKIP;
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
        }
    }

    sync_flag = avsync_audio_check_sync_flag(priv, sync_flag);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_aud_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 apts = 0xffffffff;
    mt_u32 stc = 0xffffffff;
    mt_u32 remain_tm = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("apts < remain_time, AUD free\n");
        return sync_flag;
    }

    if (!priv->init_stc)
    {
        SYNC_LOGA_FREE("stc not init, AUD free\n");
        return sync_flag;
    }

    avsync_stc_get(&stc);
    priv->cur_stc = stc;
    sync_flag = avsync_audio_get_ref_sync_flag(priv, apts, stc);
    SYNC_LOGAF("af(aud_ref): %d - %d, apts: (%#x - %#x = %#x), stc: %#x, dif: %d ms, aid: %d\n",
        priv->av_synced_flag, sync_flag, priv->cur_apts, remain_tm, apts, stc,
        abs(apts - stc)/45, priv->cur_apts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_vid_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 apts = 0xffffffff;
    mt_u32 stc = 0xffffffff;
    mt_u32 remain_tm = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();
	
    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("apts < remain_time, AUD free\n");
        return sync_flag;
    }

    if (!priv->init_stc)
    {
        SYNC_LOGA_FREE("stc not init, AUD free\n");
        return sync_flag;
    }

    avsync_stc_get(&stc);
    priv->cur_stc = stc;
    sync_flag = avsync_audio_get_ref_sync_flag(priv, apts, stc);
    SYNC_LOGAF("af(vid_ref): %d - %d, apts: (%#x - %#x = %#x), stc: %#x, dif: %d ms, aid: %d\n",
        priv->av_synced_flag, sync_flag, priv->cur_apts, remain_tm, apts, stc,
        abs(apts - stc)/45, priv->cur_apts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_av_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 apts = 0xffffffff;
    mt_u32 vpts = 0xffffffff;
    mt_u32 remain_tm = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("priv->cur_apts(0x%x) < remain_time(0x%x), AUD free\n", priv->cur_apts, remain_tm);
        return sync_flag;
    }

    vpts = priv->cur_vpts;
    sync_flag = avsync_audio_get_ref_sync_flag(priv, apts, vpts);

	if(AVSYNC_FRAME_SKIP == sync_flag)
	{
		if(priv->cur_apts < vpts)
		{
			sync_flag = AVSYNC_FRAME_SKIP_AOUT;
		}
		else
		{
		}
	}
	
    SYNC_LOGAF("af(av_ref): %d - %d, apts: (%#x - %#x = %#x), vpts: %#x, dif: %d ms, avid: %d - %d\n",
        priv->av_synced_flag, sync_flag, priv->cur_apts, remain_tm, apts, vpts,
        abs(apts - vpts)/45, priv->cur_apts_id, priv->cur_vpts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_pcr_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 at_diff = 0;
    mt_u32 apts = 0xffffffff;
    mt_u32 stc = 0xffffffff;
    mt_u32 pause_pts = 0;
    mt_u32 skip_pts = 0;
    mt_u32 remain_tm = 0;
    mt_u32 cur_tick = 0;
    struct pcr_info_t *ppcr_info = NULL;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    ppcr_info = &priv->pcr_info;
    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("apts < remain_time, AUD free\n");
        return sync_flag;
    }

    if (!ppcr_info->pcr_got)
    {
        cur_tick = avsync_osal_get_tick();

        if (0xffffffff == ppcr_info->pcr_probe_start_tick)
        {
            ppcr_info->pcr_probe_start_tick = cur_tick;
			sync_flag = AVSYNC_FRAME_PAUSE;
            SYNC_LOGA_PAUSE("no pcr, AUD pause\n");
            return sync_flag;
        }

        if ((cur_tick - ppcr_info->pcr_probe_start_tick) <= 100)
        {
            sync_flag = AVSYNC_FRAME_PAUSE;
            SYNC_LOGA_PAUSE("no pcr, AUD pause\n");
            return sync_flag;
        }
    }

    if (!priv->init_stc)
    {
        SYNC_LOGA_FREE("stc not init, AUD free\n");
        return sync_flag;
    }

    pause_pts = apts - priv->avsync_pause_threhold;
    skip_pts  = apts + priv->avsync_skip_threhold;

    avsync_stc_get(&stc);
    priv->cur_stc = stc;

    if ((pause_pts <= stc) && (stc <= skip_pts))
    {
        sync_flag = AVSYNC_FRAME_PLAY;
    }
    else if ((stc > skip_pts) && ((stc - apts) < AVSYNC_AV_DIFF_FREE_THRESHOLD))
    {
        sync_flag = AVSYNC_FRAME_SKIP;
	}
    else if ((stc < pause_pts) && ((apts - stc) < AVSYNC_AV_DIFF_FREE_THRESHOLD))
    {
        sync_flag = AVSYNC_FRAME_PAUSE;
    }
    else
    {
        sync_flag = AVSYNC_FRAME_FREE;
    }

    at_diff = abs(apts - stc);
    sync_flag = avsync_audio_check_sync_flag(priv, sync_flag);
    SYNC_LOGAF("af(pcr_ref): %d - %d, cur_apts: %#x - rem: %#x = %#x, stc: %#x, dif: %d ms, aid: %d\n",
        priv->av_synced_flag, sync_flag, priv->cur_apts, remain_tm, apts, stc,
        at_diff/45, priv->cur_apts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_sync_flg(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    switch (priv->avsync_sync_mode)
    {
         case AVSYNC_REF_AUDIO_MODE:
         {
             sync_flag = avsync_audio_get_aud_ref_sync_flg(priv);
             break;
         }

         case AVSYNC_REF_VIDEO_MODE:
         {
             sync_flag = avsync_audio_get_vid_ref_sync_flg(priv);
             break;
         }

         case AVSYNC_REF_AV_MODE:
         {
             sync_flag = avsync_audio_get_av_ref_sync_flg(priv);
             break;
         }

         case AVSYNC_REF_PCR_MODE:
         {
             sync_flag = avsync_audio_get_pcr_ref_sync_flg(priv);
             break;
         }

         default:
         {
            MT_ERR_SYNC("%s, %d, sync ref mode error:%d\n", __func__, __LINE__, priv->avsync_sync_mode);
		 	break;
         }
    }

    return sync_flag;
}

char * get_aout_name(avsync_priv_t *priv)
{
	char * cname = NULL;
	switch (priv->avsync_aud_output)
	{
		case AVSYNC_AUD_OUT_PCM:
			cname = "PCM";
			break;
		case AVSYNC_AUD_OUT_SPDIF:
			cname = "SPDIF";
			break;
		case AVSYNC_AUD_OUT_MAT:
			cname = "MAT";
			break;
		default:
			cname = "NULL";
			break;
	}
	return cname;
}
char * get_tvsys_name(avsync_priv_t *priv)
{
	char * cname = NULL;
	switch (priv->av_play_info.tvformat)
	{
	     case MT_DRV_DISP_FMT_1080i_50:
	         cname = "1080i50";
	         break;
	     case MT_DRV_DISP_FMT_1080P_50:
	         cname = "1080p50";
	         break;

	     case MT_DRV_DISP_FMT_1080i_60:
	         cname = "1080i60";
	         break;

	     case MT_DRV_DISP_FMT_1080P_60:
	         cname = "1080p60";
	         break;

	     case MT_DRV_DISP_FMT_720P_60:
	         cname = "720p60";
	         break;

	     case MT_DRV_DISP_FMT_720P_50:
	         cname = "720p50";
	         break;

	     case MT_DRV_DISP_FMT_PAL:
	         cname = "576i50";
	         break;

	     case MT_DRV_DISP_FMT_576P_50:
	         cname = "576p50";
	         break;

	     case MT_DRV_DISP_FMT_480P_60:
	         cname = "480p60";
	         break;

	     case MT_DRV_DISP_FMT_3840X2160_60:
	         cname = "3840p60";
	         break;

	     case MT_DRV_DISP_FMT_3840X2160_50:
	         cname = "3840p50";
	         break;

	     case MT_DRV_DISP_FMT_4096X2160_50:
	         cname = "4096p50";
	         break;

	     case MT_DRV_DISP_FMT_4096X2160_60:
	         cname = "4096p60";
	         break;
	     default:
	         cname = "NONE";
	         break;
	 }
	return cname;
}


char * get_programe_name(avsync_priv_t *priv)
{
	char * cname = NULL;
	switch (priv->ddp_prog)
	{
	    case AVSYNC_DD_NONE_PG:
	        cname = "NONE_PG";
	        break;
		
	    case AVSYNC_MPEG2_DD_2500_PG1:
	        cname = "MPEG2_DD_2500_PG1";
	        break;

	    case AVSYNC_MPEG2_DD_2500_PG2:
	        cname = "MPEG2_DD_2500_PG2";
	        break;

	    case AVSYNC_MPEG2_DDP_2500_PG1:
	        cname = "MPEG2_DDP_2500_PG1";
	        break;

	    case AVSYNC_MPEG2_DDP_2500_PG2:
	        cname = "MPEG2_DDP_2500_PG2";
	        break;

	    case AVSYNC_H264_DD_2500_PG1:
	        cname = "H264_DD_2500_PG1";
	        break;

	    case AVSYNC_H264_DD_2500_PG2:
	        cname = "H264_DD_2500_PG2";
	        break;

	    case AVSYNC_H264_DDP_2500_PG1:
	        cname = "H264_DDP_2500_PG1";
	        break;

	    case AVSYNC_H264_DDP_2500_PG2:
	        cname = "H264_DDP_2500_PG2";
	        break;

	    case AVSYNC_H264_DD_2997_PG1:
	        cname = "H264_DD_2997_PG1";
	        break;

	    case AVSYNC_H264_DD_2997_PG2:
	        cname = "H264_DD_2997_PG2";
	        break;

	    case AVSYNC_H264_DDP_2997_PG1:
	        cname = "H264_DDP_2997_PG1";
	        break;

	    case AVSYNC_H264_DDP_2997_PG2:
	        cname = "H264_DDP_2997_PG2";
	        break;

	    case AVSYNC_H264_DDP_2500_ATMOS:
	        cname = "H264_DDP_2500_ATMOS";
	        break;

		case AVSYNC_H264_DDP_5000_ATMOS:
	        cname = "H264_DDP_5000_ATMOS";
	        break;
		
	    case AVSYNC_H264_DDP_2997_ATMOS:
	        cname = "H264_DDP_2997_ATMOS";
	        break;		
			 
	    case AVSYNC_MPEG2_DDP_2500_ATMOS:
	        cname = "MPEG2_DDP_2500_ATMOS";
	        break;

		case AVSYNC_H265_DDP_2500_ATMOS:
			cname = "H265_DDP_2500_ATMOS";
	        break;

		case AVSYNC_H265_DDP_5000_ATMOS:
	        cname = "H265_DDP_5000_ATMOS";
	        break;
		
		case AVSYNC_H265_DDP_2997_ATMOS:
	        cname = "H265_DDP_2997_ATMOS";
	        break;
		
	    case AVSYNC_H265_DDP_5994_ATMOS:
	        cname = "H265_DDP_5994_ATMOS";
	        break;
		 
		case AVSYNC_H264_AC4_2500_ATMOS:
			cname = "AVSYNC_H264_AC4_2500_ATMOS";
			break;

		case AVSYNC_H264_AC4_5000_ATMOS:
			cname = "AVSYNC_H264_AC4_5000_ATMOS";
			break;
		
		case AVSYNC_H264_AC4_2997_ATMOS:
			cname = "AVSYNC_H264_AC4_2997_ATMOS";
			break;

		case AVSYNC_H265_AC4_2500_ATMOS:
			cname = "AVSYNC_H265_AC4_2500_ATMOS";
			break;
		
	    case AVSYNC_H265_AC4_5000_ATMOS:
			cname = "AVSYNC_H265_AC4_5000_ATMOS";
			break;
		
		case AVSYNC_H265_AC4_2997_ATMOS:
			cname = "AVSYNC_H265_AC4_2997_ATMOS";
			break;
			
		case AVSYNC_H265_AC4_5994_ATMOS:
			 cname = "AVSYNC_H265_AC4_5994_ATMOS";
			 break;
		 
	     default:
	         cname = "NONE";
	         break;
	 }
	return cname;
}
extern void snd_reg_DCH_set_pcm_play_stop(mt_u32 data);
extern mt_u32 snd_reg_DCH_get_pcm_play_stop(void);

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_dolby_sync_flg(avsync_priv_t *priv)
{
    mt_u32 av_diff = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;
    mt_u32 vpts = 0xffffffff;
    mt_u32 apts = 0xffffffff;
    mt_u32 tm = 0;
    mt_u32 tm_dif = 0;
    mt_u32 remain_tm = 0;
    static mt_u32  pre_tk = 0;
	mt_u32  tk = avsync_osal_get_tick();
	mt_u32  sync_done = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null, AUD free\n");
        return sync_flag;
    }

    priv->aud_sync_sys_time_ms = avsync_osal_get_tick( );

	if(priv->aud_push_cnt == 1)
	{
		//printk(KERN_ERR "first_aud apts %d ms\n",  priv->cur_apts/45);
	}
    if ((AVSYNC_REF_NONE_MODE == priv->avsync_sync_mode) ||
        (AVSYNC_REF_AUDIO_MODE == priv->avsync_sync_mode))
    {
        SYNC_LOGA_FREE("sync ref mode is none or audio, AUD free\n");
        return sync_flag;               
    }

    if (0xffffffff == priv->cur_vpts)
    {
    	sync_flag = AVSYNC_FRAME_FREE; 
        SYNC_LOGA_FREE("dolby VID no pts, AUD FREE\n");
		pre_tk = tk;
        return sync_flag;
    }

    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("apts < remain_time, AUD free\n");
        return sync_flag;
    }

    tm = avsync_osal_get_tick( );
    tm_dif = tm - priv->vid_sync_sys_time_ms;
    vpts = tm_dif*45 + priv->cur_vpts;

	sync_done = priv->dolby_sync_done;

	av_diff = abs(vpts - apts);
	
	if ((1 == priv->dolby_adjust) || (0 == priv->dolby_sync_done))
	{
	    if (vpts >= apts)
	    {
	        if (av_diff <= priv->avsync_pause_threhold)
	        {
	            sync_flag = AVSYNC_FRAME_PLAY;
				priv->dolby_sync_done = 1;
	        }
	        else
	        {
	            sync_flag = AVSYNC_FRAME_SKIP;
	        }
	    }
	    else
	    {
			if (av_diff <= priv->avsync_skip_threhold)
			{
				sync_flag = AVSYNC_FRAME_PLAY;
				priv->dolby_sync_done = 1;
			}
			else
			{
				sync_flag = AVSYNC_FRAME_PAUSE;
			}			
	    }
	}
	else
	{
		if(av_diff > 1000*45)
		{
			priv->dolby_sync_done = 0;
		}
		else
		{
			sync_flag = AVSYNC_FRAME_PLAY;
		}
	}

    SYNC_LOGAF("[%s] %d, apts: %d, vpts: %d, dif: %d, rem: %d, aid: %d vid %d, tm_dif %d tk_diff %d papts %d \n",get_programe_name(priv),
			sync_flag, apts/45, vpts/45, av_diff/45, remain_tm/45,	priv->cur_apts_id, priv->cur_vpts_id, tm_dif,(tk - pre_tk), priv->cur_play_apts/45);
	
    pre_tk = tk;
	
	if (priv->apts_fifo_pop_cnt < 8)
	{
    	sync_flag = AVSYNC_FRAME_FREE;
        return sync_flag;
	}
	
    if (av_diff > (45*12*1000))
    {
        SYNC_LOGA_FREE("AUD free: AV Diff: %d > Free threhold: %d\n", av_diff/45,
            AVSYNC_AV_DIFF_FREE_THRESHOLD/45);
        sync_flag = AVSYNC_FRAME_FREE;
		if(snd_reg_DCH_get_pcm_play_stop() == 1)
		{
			snd_reg_DCH_set_pcm_play_stop(0);
		}
    }

    switch (sync_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
			if(snd_reg_DCH_get_pcm_play_stop() == 1)
			{
				snd_reg_DCH_set_pcm_play_stop(0);
			}
			SYNC_LOGA_SYNCED("AUD dolby synced\n");
            break;
        }
    
        case AVSYNC_FRAME_SKIP:
        {
            SYNC_LOGA_SKIP("AUD dolby skip\n");
            break;
        }
    
        case AVSYNC_FRAME_PAUSE:
        {
			if(snd_reg_DCH_get_pcm_play_stop() == 0)
			{
				snd_reg_DCH_set_pcm_play_stop(1);
			}

            SYNC_LOGA_PAUSE("AUD dolby pause\n");
            break;
        }

	    case AVSYNC_FRAME_FREE:
        {
            SYNC_LOGA_FREE("AUD dolby free\n");
            break;
        }

        default:
        {
            MT_ERR_SYNC("A sync flag is error:%d\n", sync_flag);
            break;
        }
    }

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_audio_get_atmos_sync_flg(avsync_priv_t *priv)
{
    mt_u32 av_diff = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;
    mt_u32 vpts = 0xffffffff;
    mt_u32 apts = 0xffffffff;
    mt_u32 tm = 0;
    mt_u32 tm_dif = 0;
    mt_u32 remain_tm = 0;
	mt_u32  sync_done = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null, AUD free\n");
        return sync_flag;
    }

    priv->aud_sync_sys_time_ms = avsync_osal_get_tick( );

    if ((AVSYNC_REF_NONE_MODE == priv->avsync_sync_mode) ||
        (AVSYNC_REF_AUDIO_MODE == priv->avsync_sync_mode))
    {
        SYNC_LOGA_FREE("sync ref mode is none or audio, AUD free\n");
        return sync_flag;               
    }

    if (0xffffffff == priv->cur_vpts)
    {
        SYNC_LOGA_FREE("VID no pts, AUD free\n");
        return sync_flag;
    }

    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("apts < remain_time, AUD free\n");
        return sync_flag;
    }

    tm = avsync_osal_get_tick( );
    tm_dif = tm - priv->vid_sync_sys_time_ms;
    vpts = tm_dif*45 + priv->cur_vpts;
	sync_done = priv->dolby_sync_done;

    if (vpts >= apts)
    {
        av_diff = vpts - apts;

        if (av_diff <= priv->avsync_pause_threhold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
			priv->dolby_sync_done = 1;
        }
        else
        {
            sync_flag = AVSYNC_FRAME_SKIP;
        }
    }
    else
    {
        av_diff = apts - vpts;

        if (av_diff <= priv->avsync_skip_threhold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
			priv->dolby_sync_done = 1;
        }
        else
        {
            sync_flag = AVSYNC_FRAME_PAUSE;
        }
    }    

    if (av_diff > AVSYNC_AV_DIFF_FREE_THRESHOLD)
    {
        SYNC_LOGA_FREE("AUD free: AV Diff: %d > Free threhold: %d\n", av_diff/45,
            AVSYNC_AV_DIFF_FREE_THRESHOLD/45);
        sync_flag = AVSYNC_FRAME_FREE;
    }
	
	if (priv->apts_fifo_pop_cnt < 8)
	{
    	sync_flag = AVSYNC_FRAME_FREE;
		SYNC_LOGAF("af(ATOMS): %d, apts: %#x, vpts: %#x, dif: %d ms, rem: %d ms, avid: %d - %d, priv->apts_fifo_pop_cnt=%d\n",
			        sync_flag, priv->cur_apts, priv->cur_vpts, av_diff/45, remain_tm/45,
			        priv->cur_apts_id, priv->cur_vpts_id, priv->apts_fifo_pop_cnt);
        return sync_flag;
	}
	
    switch (sync_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
            SYNC_LOGA_SYNCED("AUD dolby synced\n");
            break;
        }
    
        case AVSYNC_FRAME_SKIP:
        {
            SYNC_LOGA_SKIP("AUD dolby skip\n");
            break;
        }
    
        case AVSYNC_FRAME_PAUSE:
        {
            SYNC_LOGA_PAUSE("AUD dolby pause\n");
            break;
        }

	    case AVSYNC_FRAME_FREE:
        {
            SYNC_LOGA_FREE("AUD dolby free\n");
            break;
        }

        default:
        {
            MT_ERR_SYNC("A sync flag is error:%d\n", sync_flag);
            break;
        }
    }

	SYNC_LOGAF("af(ATOMS): %d, apts: %#x, vpts: %#x, dif: %d ms, rem: %d ms, avid: %d - %d\n",
        sync_flag, priv->cur_apts, priv->cur_vpts, av_diff/45, remain_tm/45,
        priv->cur_apts_id, priv->cur_vpts_id);
	
    return sync_flag;
}


mt_s32 avsync_audio_do_sync(avsync_priv_t *priv,
                                         AVSYNC_FRAME_SYNCFLAG_E *psync_flg)
{
	mt_u32 stc = 0;
	
    *psync_flg = AVSYNC_FRAME_FREE;

    if (DEV_INIT != priv->aud_dev_sta)
    {
        SYNC_LOGA_FREE("AUD not init, AUD free\n");
        return MT_FAILURE;
    }

    if (0xffffffff == priv->cur_apts)
    {
        SYNC_LOGA_FREE("AUD no pts, AUD free\n");
        return MT_FAILURE;
    }

	//if((priv->ddp_atmos) && (AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output))
	if (priv->ddp_atmos)
	{
		*psync_flg = avsync_audio_get_atmos_sync_flg(priv);
		return MT_SUCCESS;
	}

    if (priv->do_ddp_verf)
    {
        *psync_flg = avsync_audio_get_dolby_sync_flg(priv);
		return MT_SUCCESS;
    }

    if (AVSYNC_REF_NONE_MODE == priv->avsync_sync_mode)
    {
        SYNC_LOGA_FREE("AV free mode, AUD free\n");
		priv->a_synced_flag = 1;
        return MT_SUCCESS;
    }

    if (AVSYNC_REF_PCR_MODE == priv->avsync_sync_mode)
    {
        goto SYNC_CB;
    }

    if (DEV_INIT != priv->vid_dev_sta)
    {
        SYNC_LOGA_FREE("VID not init, AUD free\n");
        return MT_SUCCESS;
    }	
	
    if ((!priv->astabled_flag) || (!priv->vstabled_flag))
    {
        if (!priv->avsync_do_aud_track)
        {
			SYNC_LOGA_FREE("%s %s pts unstable, AUD free\n", ((priv->astabled_flag) ? "" : "AUD"),
                ((priv->vstabled_flag) ? "" : "VID"));
            return MT_SUCCESS;
        }
    }

	//When the liveplay switch audio track, the apts is less than vpts at some time for some stream.
	//In the case, the audio has to skip. If the avpts diff is large, there is not enough data to skip, 
	//so the av can't sync and the auido can't play until the stream rollback.
	//for this case, we change to av mode to sync.
	if ((priv->avsync_do_aud_track) && (priv->audio_track_set_av_mode_flag) && (is_liveplay()))
	{  		
    	avsync_stc_get(&stc);
		if ((priv->cur_apts < stc) && ((stc - priv->cur_apts) > AVSYNC_AUDIO_TRACK_DIFF))
		{
			priv->audio_track_set_av_mode_flag = 0;
			pr_err("[%s %d]stc=0x%x, apts=0x%x, diff=%d ms, set to av mode\n", __FUNCTION__, __LINE__, stc, priv->cur_apts, (stc - priv->cur_apts)/45);
			avsync_set_sync_mode(priv, AVSYNC_REF_AV_MODE);
		}
	}
	
SYNC_CB:
    *psync_flg = avsync_audio_get_sync_flg(priv);  
	return MT_SUCCESS;
}

mt_void avsync_audio_loop(struct avsync_async_info_t *pdata,
                                         AVSYNC_FRAME_SYNCFLAG_E *psync_flg)
{
    avsync_priv_t *priv = NULL;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;
    struct avsync_async_info_t *pasync_info = NULL;
    SYNC_PUSH_APTS_S push_info;
	struct pts_node_t audio_node;
	MT_HA_AUDIO_STREAM_INFO_S audio_info;
	
	memset(&audio_node, 0, sizeof(struct pts_node_t));	

    if ((NULL == pdata) || (NULL == psync_flg))
    {
        SYNC_LOGA_FREE("param err, AUD free\n");
        return;
    }

    *psync_flg = AVSYNC_FRAME_FREE;
    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null, AUD free\n");
        return;
    }

    pasync_info = &priv->async_info;
    memset(pasync_info, 0, sizeof(struct avsync_async_info_t));
    memset(&push_info, 0, sizeof(SYNC_PUSH_APTS_S));
    memcpy(pasync_info, pdata, sizeof(struct avsync_async_info_t));
    priv->cur_apts = pasync_info->apts;
    priv->cur_apts_id = pasync_info->apts_id;
    priv->apts_step = pasync_info->apts_step;
    priv->cur_pcm_time = pasync_info->pcm_temp_time;
    priv->aud_push_cnt++;

    if (pasync_info->ao_data_size <= priv->aud_underflow_size)
    {
        priv->aud_underflow_cnt++;
        SYNC_LOGA_DATA("Aundflw:[%d][%d]\n", pasync_info->ao_data_size,
            priv->aud_underflow_size);
    }

	//in sym4, it is dealed in avsync_audio_push_pts.
	//but in sym6, only AVSYNC_FRAME_PLAY or AVSYNC_FRAME_FREE call avsync_audio_push_pts.
	//so correct apts in here for dolby test.
    if (priv->do_ddp_verf &&
        ((1536 == priv->aud_frame_sample_num) ||
         (256 == priv->aud_frame_sample_num) ||
         (1792 == priv->aud_frame_sample_num) ||
		 (HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)))
    {
    	if ((priv->ddp_atmos) && (AVSYNC_AUD_OUT_BUTT == priv->avsync_aud_output))
    	{
	    	memset(&audio_info, 0, sizeof(MT_HA_AUDIO_STREAM_INFO_S));

	        if(MT_SUCCESS == AO_Track_GetAudioStreamInfo(&audio_info))
	        {
	        	if (MT_UNF_SND_HDMI_MODE_RAW == audio_info.hdmi_mode)
	            {
	               priv->avsync_aud_output = AVSYNC_AUD_OUT_SPDIF;
	            }
				else if (MT_UNF_SND_HDMI_MODE_MAT == audio_info.hdmi_mode)
				{
					 priv->avsync_aud_output = AVSYNC_AUD_OUT_MAT;
				}
	            else
	            {
	                priv->avsync_aud_output = AVSYNC_AUD_OUT_PCM;
	            }
	        }
    	}
		
    	//The parameter can't use for both hdmi and no-hdmi, and i don't find the reason.
    	//so use different parameter for hdmi and no-hdmi at present.
    	if (priv->av_play_info.is_hdmi_connected)
    	{
        	avsync_audio_adjust_apts_for_ddp_verfication(priv, &priv->cur_apts);
    	}
		else
		{
			avsync_audio_adjust_apts_for_ddp_verfication_no_hdmi(priv, &priv->cur_apts);
		}
    }
	
	if (priv->async_info.audio_bypass)
	{		
		if (priv->audio_bypass_apts_adjust)
		{
	        if (AVSYNC_APTS_ADJUST_INCREASE_FLAG & priv->audio_bypass_apts_adjust)//increase apts
			{
	            priv->cur_apts += 45 * (AVSYNC_APTS_ADJUST_MS_MAX & priv->audio_bypass_apts_adjust);
	        }
			else//decrease apts
			{
	            priv->cur_apts -= 45 * (AVSYNC_APTS_ADJUST_MS_MAX & priv->audio_bypass_apts_adjust);
	        }
	    }
	}
	
    avsync_audio_es_level_check(priv);
    avsync_aduio_pts_check(priv);
    avsync_audio_do_sync(priv, &sync_flag);
    push_info.id = priv->cur_apts_id;
    push_info.pts_u32 = priv->cur_apts;
    push_info.step_u32 = priv->apts_step;
	
    switch (sync_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
            priv->aud_play_cnt++;
            avsync_audio_push_pts(NULL, &push_info);
            break;
        }
		
		case AVSYNC_FRAME_SKIP_AOUT:
        case AVSYNC_FRAME_SKIP:
        {
            priv->aud_skip_cnt++;
            break;
        }

        case AVSYNC_FRAME_PAUSE:
        {
            priv->aud_pause_cnt++;
            break;
        }

	    case AVSYNC_FRAME_FREE:
        {
            priv->aud_free_cnt++;
            avsync_audio_push_pts(NULL, &push_info);
            break;
        }

        default:
        {
            MT_ERR_SYNC("A sync flag is error:%d\n", sync_flag);
            break;
        }
    }

    *psync_flg = sync_flag;
	
	if (priv->async_info.audio_bypass)
	{
		if ((priv->astabled_flag) && (AVSYNC_REF_AUDIO_MODE == priv->avsync_sync_mode))
	    {
			audio_node.pts = priv->cur_apts;
			audio_node.pts_step = priv->apts_step;				
			avsync_stc_adjust_by_apts(priv, &audio_node);
			priv->audio_bypass_last_apts = audio_node.pts;		          
	    }
	}
}

mt_s32 avsync_audio_init(mt_u32 atype)
{
    avsync_priv_t *priv = NULL;

    MT_INFO_SYNC("avsync_audio_init, atype:%#x\n", atype);

	priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    avsync_audio_pts_fifo_init(priv);
    memset((mt_u8 *)&priv->async, 0, sizeof(struct async_t));
    memset((mt_u8 *)&priv->async_info, 0, sizeof(struct avsync_async_info_t));
    memset((mt_u8 *)&priv->isr_pop, 0, sizeof(struct pts_node_t));
    priv->first_apts = 0xffffffff;
    priv->cur_apts = 0xffffffff;
    priv->cur_apts_id = 0;
    priv->cur_play_apts = 0;
    priv->aud_free_cnt = 0;
    priv->aud_pause_cnt = 0;
    priv->aud_play_cnt = 0;
    priv->aud_skip_cnt = 0;
    priv->aud_push_cnt = 0;
    priv->aud_dev_sta = DEV_INIT;
    priv->aud_sync_sys_time_ms = 0;
    priv->apts_fifo_push_cnt = 0;
    priv->apts_fifo_pop_cnt = 0;
    priv->apts_step = 0;
    priv->astabled_cnt = 0;
    priv->astabled_flag = 0;
    priv->a_unsynced_cnt = 0;
    priv->async_info.apts = 0;
    priv->async.last_apts = 0xffffffff;
    priv->async.latest_parsed_apts = 0xffffffff;
    priv->a_synced_flag = 0;
    priv->av_synced_flag = 0;
    priv->apts_total_cnt = 0;
    priv->apts_total_cnt_before_sw_sync_mode = 0;
    priv->dolby_avsync = 0;
    priv->aud_underflow_cnt = 0;
	priv->atoms_looped = 0;
	priv->avsync_aud_output = AVSYNC_AUD_OUT_BUTT;
    MT_INFO_SYNC("avsync_audio_init end\n");

    return MT_SUCCESS;
}
EXPORT_SYMBOL(avsync_audio_init);


static mt_u32 R_CLKGEN_VIDEO_SW_REG1_VALUE = 0;
static mt_u32 R_CLKGEN_VIDEO_SW_REG0_VALUE = 0;
static mt_u32 R_INTP_CTRL_REG1_VALUE = 0;
static mt_u32 R_CLKGEN_VIDEO_SW_HD_REG0_VALUE = 0;
static mt_u32 R_CLKGEN_VIDEO_SW_HD_REG1_VALUE = 0;

static int   g_display_value_seted = 0;

static mt_s32 avsync_reset_display_for_dolby_stream(void)
{
    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_REG1, R_CLKGEN_VIDEO_SW_REG1_VALUE);
    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_REG0, R_CLKGEN_VIDEO_SW_REG0_VALUE);
    HAL_PUT_U32((volatile u32 *)R_INTP_CTRL_REG1, R_INTP_CTRL_REG1_VALUE);
    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG0, R_CLKGEN_VIDEO_SW_HD_REG0_VALUE);
    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG1, R_CLKGEN_VIDEO_SW_HD_REG1_VALUE);
	
	g_display_value_seted = 0;
    return MT_SUCCESS;
}

mt_s32 avsync_audio_deinit(mt_void)
{
    avsync_priv_t *priv = NULL;

    MT_INFO_SYNC("avsync_audio_deinit\n");

	priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

	if(1 == g_display_value_seted)
	{
		avsync_reset_display_for_dolby_stream();
	}

    priv->aud_dev_sta = DEV_DEINIT;
    MT_INFO_SYNC("avsync_audio_deinit end\n");
    return MT_SUCCESS;
}
EXPORT_SYMBOL(avsync_audio_deinit);

static DISP_EXPORT_FUNC_S *disp_func_ops = MT_NULL;

#define GET_DISP_FUNC()		do {	\
								if (disp_func_ops == NULL)	\
									(void)mt_drv_module_getfunction(MT_ID_DISP, (mt_void**)&disp_func_ops);	\
							} while (0)
#define CHECK_DISP_FUNC()	do {	\
								if (disp_func_ops == NULL) {	\
									MT_ERR_SYNC("disp_func_ops is null!\n");	\
									return MT_FAILURE;	\
								} 	\
							} while (0)


static mt_s32 DF_AvsyncGetVptsInfo_Wrap(mt_handle pstDispBP, DF_AVSYNC_PTS_S *pInfo)
{
	GET_DISP_FUNC();

	CHECK_DISP_FUNC();

	if (disp_func_ops->DRV_DISP_AvsyncGetVptsInfo)
	{
		return (mt_s32)disp_func_ops->DRV_DISP_AvsyncGetVptsInfo((void*)pstDispBP, (void*)pInfo);
	}
	else
	{
		MT_ERR_SYNC("DRV_DISP_AvsyncGetVptsInfo is null!\n");
		return MT_FAILURE;
	}
}

static mt_s32 DF_AvsyncSetSkipFrame_Wrap(mt_u32 skip_num)
{
	GET_DISP_FUNC();

	CHECK_DISP_FUNC();

	if (disp_func_ops->DRV_DISP_AvsyncSetSkipFrame)
	{
		return (mt_s32)disp_func_ops->DRV_DISP_AvsyncSetSkipFrame(skip_num);
	}
	else
	{
		MT_ERR_SYNC("DRV_DISP_AvsyncSetSkipFrame is null!\n");
		return MT_FAILURE;
	}
}

static mt_s32 DF_AvsyncSetRepeatFrame_Wrap(void)
{
	GET_DISP_FUNC();

	CHECK_DISP_FUNC();

	if (disp_func_ops->DRV_DISP_AvsyncSetRepeatFrame)
	{
		return (mt_s32)disp_func_ops->DRV_DISP_AvsyncSetRepeatFrame();
	}
	else
	{
		MT_ERR_SYNC("DRV_DISP_AvsyncSetRepeatFrame is null!\n");
		return MT_FAILURE;
	}
}

mt_s32 avsync_video_init(mt_u32 vtype)
{
    avsync_priv_t *priv = NULL;

    MT_INFO_SYNC("avsync_video_init, vtype:%#x\n", vtype);

	priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    memset((mt_u8 *)&priv->vsync, 0, sizeof(struct vsync_t));
    memset((mt_u8 *)&priv->vsync_info, 0, sizeof(struct avsync_vsync_info_t));
    priv->first_vpts = 0xffffffff;
    priv->cur_vpts = 0xffffffff;
    priv->cur_vpts_id = 0;
    priv->cur_vpts_valid = 0;
    priv->vid_free_cnt = 0;
    priv->vid_pause_cnt = 0;
    priv->vid_play_cnt = 0;
    priv->vid_skip_cnt = 0;
    priv->vid_push_cnt = 0;
    priv->vid_dev_sta = DEV_INIT;
    priv->vid_pause_cnt_before_aud_push = 0;
    priv->vid_sync_sys_time_ms = 0;
    priv->vid_dolby_ply_cnt = 0;
	priv->dolby_sync_done = 0;
	priv->normal_sync_done = 0;
	priv->normal_vfirst = 0;
    priv->dolby_avsync = 0;
    priv->avsync_vloop_cnt = 0;
    priv->vpts_step = 0;
    priv->vstabled_cnt = 0;
    priv->vstabled_flag = 0;
    priv->v_unsynced_cnt = 0;
    priv->vsync_info.vpts = 0;
    priv->vsync.last_vpts = 0xffffffff;
    priv->vsync.latest_parsed_vpts = 0xffffffff;
    priv->vsync.vpts = 0xffffffff;
    priv->v_synced_flag = 0;
    priv->av_synced_flag = 0;
    priv->disp_flag = 0;
    priv->slow_sync_cnt = 0;
    priv->vsync_diff = 0;
    MT_INFO_SYNC("avsync_video_init end\n");

    return MT_SUCCESS;
}
EXPORT_SYMBOL(avsync_video_init);

mt_s32 avsync_video_deinit(mt_void)
{
    avsync_priv_t *priv = NULL;
    MT_INFO_SYNC("avsync_video_deinit\n");

	priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    priv->vid_dev_sta = DEV_DEINIT;
    MT_INFO_SYNC("avsync_video_deinit end\n");

    return MT_SUCCESS;
}

static mt_s32 avsync_video_pts_stable_check(avsync_priv_t *priv)
{
    struct vsync_t *pvsync = NULL;
    mt_u32 pts_diff = 0x00;
    mt_u32 pts_diff_max = 0x00;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    pvsync = &priv->vsync;

    if (0xffffffff == pvsync->last_vpts)
    {
        MT_INFO_SYNC("V stbld: first vpts, [%#x][%d]\n", priv->cur_vpts, priv->cur_vpts_id);
        priv->vstabled_flag = 1;
        return MT_SUCCESS;
	}

	
	if (0xffffffff == priv->first_vpts)
	{
		if(priv->cur_vpts != 0)
		{
			priv->first_vpts = priv->cur_vpts;
		}
	}

    if (priv->cur_vpts_id < pvsync->last_vpts_id)
    {
        if (priv->vstabled_flag)
        {
            MT_INFO_SYNC("V unstbld1:[%#x][%d]\n", priv->cur_vpts, priv->cur_vpts_id);
        }

        priv->vstabled_cnt = 0;
        priv->vstabled_flag = 0;
        return MT_SUCCESS;
    }

    if (priv->cur_vpts > pvsync->last_vpts)
    {
        pts_diff = priv->cur_vpts - pvsync->last_vpts;
		//for 60fps stream on 1080p24, step is 16,  for bug 26160,  
		pts_diff_max = (abs(priv->cur_vpts_id - pvsync->last_vpts_id) + 1 ) * priv->vpts_step;
        if (pts_diff <= pts_diff_max)
        {
            if (priv->vstabled_cnt > AVSYNC_MAX_VID_READY_NUMBER)
            {
                if (!priv->vstabled_flag)
                {
                    MT_INFO_SYNC("V stbld:[%#x][%d]\n", priv->cur_vpts, priv->cur_vpts_id);
                }

                priv->vstabled_flag = 1;
            }
            else
            {
                priv->vstabled_cnt++;
            }
        }
        else
        {
            if (priv->vstabled_flag)
            {
                MT_INFO_SYNC("V unstbld2:[%#x][%d]\n", priv->cur_vpts, priv->cur_vpts_id);
            }

            priv->vstabled_cnt = 0;
            priv->vstabled_flag = 0;
        }
    }
    else if ((priv->cur_vpts + 0x100) < pvsync->last_vpts)
    {
        if (priv->vstabled_flag)
        {
            MT_INFO_SYNC("V unstbld3:[%#x][%d]\n", priv->cur_vpts, priv->cur_vpts_id);
        }

        priv->vstabled_cnt = 0;
        priv->vstabled_flag = 0;
    }
    else
    {
        //keep the pre vpts status
    }

    return MT_SUCCESS;
}

static mt_s32 avsync_video_pts_event_check(avsync_priv_t *priv)
{
    struct vsync_t *pvsync = NULL;
	mt_u32  pts_diff = 0;
    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    pvsync = &priv->vsync;

    if (priv->p_avsync_psync != NULL)
    {
        if (priv->cur_vpts != pvsync->last_vpts)
        {
            if (priv->end_of_stream_flag)
            {
                SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_EOS);
                SYNC_LOGVP("end_of_stream_flag, vpts: %#x, id: %d \n", priv->cur_vpts, priv->cur_vpts_id);
            }
        }

        if ((priv->cur_vpts < pvsync->last_vpts) && (0xffffffff != pvsync->last_vpts) &&
            ((pvsync->last_vpts - priv->cur_vpts) > AVSYNC_STREAM_ROLLABCK_DIFF))
        {
            SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_VID_ROLLBACK);
         //   printk(KERN_ERR "vid rollback\n");
        }

		//for 60fps stream on 1080p24, step is 16,  for bug 26160,  
		pts_diff = (abs(priv->cur_vpts_id - pvsync->last_vpts_id) + 1 ) * priv->vpts_step;		
        if ((0xffffffff != pvsync->last_vpts) &&
            (abs(priv->cur_vpts - pvsync->last_vpts) >= pts_diff))
        {
            SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_VPTS_JUMP);
            SYNC_LOGVP("vpts jump, pts(cur: %#x, pre: %#x, diff: %d ms), step: %d, id(cur: %d, pre: %d)\n", priv->cur_vpts, 
                pvsync->last_vpts, abs(pvsync->last_vpts-priv->cur_vpts)/45, priv->vpts_step, priv->cur_vpts_id, pvsync->last_vpts_id);
        }
    }

    return MT_SUCCESS;	
}

static mt_s32 avsync_video_pts_repeat_check(avsync_priv_t *priv)
{
    struct vsync_t *pvsync = NULL;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    pvsync = &priv->vsync;

    if (priv->cur_vpts == 0xffffffff)
    {
        return MT_FAILURE;
    }

    if ((priv->cur_vpts == pvsync->last_vpts) &&
        (priv->cur_vpts_id == pvsync->last_vpts_id))
    {
        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

static mt_s32 avsync_video_pts_check(avsync_priv_t *priv)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    avsync_video_pts_stable_check(priv);
    avsync_video_pts_event_check(priv);
    priv->vid_push_cnt++;

    return MT_SUCCESS;
}

mt_s32 avsync_video_pts_correct(avsync_priv_t *priv, DF_AVSYNC_PTS_S *pinfo)
{
    struct  avsync_vsync_info_t *pvsync_info = NULL;
    mt_u32  cnt = 0;
    mt_u32 pts = 0;
    mt_u32 id = 0;

    if ((NULL == priv) || (NULL == pinfo))
    {
        MT_ERR_SYNC("%s, %d, param null\n", __func__, __LINE__);
        return MT_FAILURE;
    }

    pvsync_info = &priv->vsync_info;
    pts = pinfo->cur_vpts;
    id = pinfo->cur_vpts_id;

    if (pts == pvsync_info->vpts)
    {
        if (id > pvsync_info->vpts_id)
        {
            cnt = id - pvsync_info->vpts_id;
        }

        priv->cur_vpts += pvsync_info->vpts_step * cnt;
        priv->cur_vpts_id = id;
    }
    else
    {
        priv->cur_vpts = pts;
        priv->cur_vpts_id = id;
    }

    return MT_SUCCESS;
}

extern mt_s32 update_lcevc_displayed_pts(mt_u32 frame_pts, mt_u32 frame_id);

mt_s32 avsync_set_display_for_dolby_stream(avsync_priv_t *priv);
mt_s32 avsync_video_pts_get(avsync_priv_t *priv, mt_u64 handle)
{
    struct  avsync_vsync_info_t *pvsync_info = NULL;
    struct vsync_t *pvsync = NULL;
    DF_AVSYNC_PTS_S vpts_info;
    mt_u64  vpts_u64 = 0;
    mt_u32 framerate = 0;
    static mt_u32 pre_tick = 0;
    mt_u32 tick = 0;
    mt_s32 ret = MT_SUCCESS;

    if (NULL == priv)
    {
        MT_ERR_SYNC("%s, %d, priv null\n", __func__, __LINE__);
        return MT_FAILURE;
    }

    memset(&vpts_info, 0, sizeof(DF_AVSYNC_PTS_S));
    ret = DF_AvsyncGetVptsInfo_Wrap(handle,  &vpts_info);

    if (MT_SUCCESS != ret)
    {
//        SYNC_LOGVP("%s, %d, ret:%#x\n", __func__, __LINE__, ret);
        return MT_FAILURE;
    }

	if(priv->normal_vfirst == 0)
	{
		mt_drv_stat_event(STAT_EVENT_SYNCVIDEO, 0);
		priv->normal_vfirst = 1;
	}
	
    if ((vpts_info.cur_vpts == 0xffffffffUL) ||
        (vpts_info.cur_vpts_u64 == 0xffffffffffffffffULL))
    {
        SYNC_LOGVP("%s, %d, err: invalid pts\n", __func__, __LINE__);
            vpts_info.cur_vpts = 0;
            vpts_info.cur_vpts_u64 = 0;
    }

    if (vpts_info.cur_vpts_u64 != 0)
    {
        if (0 == vpts_info.cur_vpts)
        {
            vpts_u64 = vpts_info.cur_vpts_u64;
            do_div(vpts_u64, 1000);
            vpts_info.cur_vpts = (mt_u32)(vpts_u64 * 45);
        }
    }

	(void)update_lcevc_displayed_pts(vpts_info.cur_vpts, vpts_info.cur_vpts_id);
	
    priv->avsync_vloop_cnt++;
    avsync_video_frame_info_capture_write(vpts_info.cur_vpts, vpts_info.cur_vpts_id);

	if ((priv->avsync_vloop_cnt % 2) == 0)//same as avsync_video_repeat_check_printf in sym4
    {
        return MT_FAILURE;
    }

#if 0
	//bug 27815
	if((1 == vpts_info.dolby_AVSNC_TB44) && (g_p_avsync_private->data_source_mode == SYNC_DATA_SOURCE_FILE) &&
		(priv->av_play_info.a_type == HA_AUDIO_ID_DOLBY_PLUS))
	{
		if((AVSYNC_REF_VIDEO_MODE != priv->avsync_sync_mode) && (vpts_info.cur_vpts_id == 0))
		{
            avsync_set_display_for_dolby_stream(priv);
			avsync_set_sync_mode(priv, AVSYNC_REF_VIDEO_MODE);
			avsync_set_user_sync_mode(priv, AVSYNC_REF_VIDEO_MODE);
		}
	}
#endif
    pvsync_info = &priv->vsync_info;
    pvsync = &priv->vsync;
    tick = avsync_osal_get_tick();
	SYNC_LOGVP("vpts: %#x, vtdif: %d - %d, id: %d, fr:%d\n", vpts_info.cur_vpts,
		 (vpts_info.cur_vpts - pvsync->last_vpts)/45, tick - pre_tick, vpts_info.cur_vpts_id, vpts_info.framerate);

    pre_tick = tick;

    if (0 == vpts_info.framerate)
    {
        framerate = 2500;
        MT_ERR_SYNC("%s, %d, err: framerate is 0\n", __func__, __LINE__);
    }
	else if (2900 == vpts_info.framerate)
    {
        framerate = 3000;
    }
    else
    {
        framerate = vpts_info.framerate;
    }

    pvsync_info->vpts_step = 4500000 / framerate;

    if (0x5dc == pvsync_info->vpts_step)
    {
        pvsync_info->vpts_step = 0x5de;
    }
	
    avsync_video_pts_correct(priv, &vpts_info);
  //  memset(pvsync_info, 0, sizeof(struct  avsync_vsync_info_t));
    pvsync_info->vpts = vpts_info.cur_vpts;
    pvsync_info->vpts_id = vpts_info.cur_vpts_id;
    pvsync_info->trick_state = vpts_info.trick_state;
    pvsync_info->trick_speed = vpts_info.trick_speed;
    pvsync_info->dolby_avsync = vpts_info.dolby_AVSNC_TB44;
    pvsync_info->screen_mode = vpts_info.screen_mode;
    pvsync_info->end_of_stream_flag = vpts_info.end_of_stream_flag;

    /*
     * dolby_avsync flag just update once
     */
    if (!priv->dolby_avsync)
    {
        if (pvsync_info->dolby_avsync)
        {
            priv->dolby_avsync = pvsync_info->dolby_avsync;
        }
    }

    priv->vpts_step = pvsync_info->vpts_step;
    priv->end_of_stream_flag = pvsync_info->end_of_stream_flag;

    return MT_SUCCESS;
}

mt_s32 avsync_video_sync_status_reset(avsync_priv_t *priv)
{
    priv->v_synced_flag = 0;
    priv->vstabled_cnt = 0;
    priv->vstabled_flag = 0;
    priv->v_unsynced_cnt = 0;
    priv->av_synced_flag = 0;
    return MT_SUCCESS;
}

struct dmx_desc_data_s
{
    u32 u32Info;
    u32 u32Addr;
    u32 u32Dts;
    u32 u32Pts;
};

mt_void avsync_video_es_level_check(avsync_priv_t *priv)
{
    ulong es_addr = 0;
    ulong es_size = 0;
    ulong desc_addr = 0;
    ulong desc_size = 0;
    mt_u32 es_wr = 0;
    mt_u32 es_rd = 0;
    mt_u32 es_len = 0;
    mt_u32 desc_wr = 0;
    mt_u32 desc_rd = 0;
    mt_u32 desc_len = 0;
    struct dmx_desc_data_s *pdesc = NULL;
    mt_u32 origPTS = 0xffffffff;

    if (NULL == priv)
    {
        MT_ERR_SYNC("param err\n");
        return;
    }

    if (!(priv->avsync_loglevel & SYNC_VIDEO_DATA_INFO))
    {
        return;
    }

    if ((SYNC_DATA_SOURCE_TUNER != priv->data_source_mode)
        && (SYNC_DATA_SOURCE_PVR != priv->data_source_mode))
    {
        return;
    }

    if (DMXOsiChnEsDescBufGetAddrSize(0, &es_addr, &es_size, &desc_addr, &desc_size))
    {
        return;
    }

    DMXOsiChnEsDataGetReadWrite(0, &es_wr, &es_rd);
    DMXOsiChnEsDescDataGetReadWrite(0, &desc_wr, &desc_rd);

    if (es_wr >= es_rd)
    {
        es_len = es_wr - es_rd;
    }
    else
    {
        es_len = es_size - es_rd + es_wr;
    }

    if (desc_wr >= desc_rd)
    {
        desc_len = desc_wr - desc_rd;
    }
    else
    {
        desc_len = desc_size - desc_rd + desc_wr;
    }

    /*
     * get the latest vpts
     */
    if (desc_wr >= desc_rd)
    {

        pdesc = (struct dmx_desc_data_s *)(desc_addr + desc_wr - sizeof(*pdesc));

        for (; pdesc >= (struct dmx_desc_data_s *)(desc_addr + desc_rd); pdesc--)
        {
            if (pdesc->u32Info & 0x1)
            {
                origPTS = (pdesc->u32Pts >> 1) & 0x3FFFFFFF;        /* bit[29:0] */
                origPTS |= ((pdesc->u32Info >> 24) & 0x03) << 30;   /* bit[31:30] */
                break;
            }
        }
    }
    else
    {
        pdesc = (struct dmx_desc_data_s *)(desc_addr + desc_wr - sizeof(*pdesc));

        for (; pdesc >= (struct dmx_desc_data_s *)(desc_addr); pdesc--)
        {
            if (pdesc->u32Info & 0x1)
            {
                origPTS = (pdesc->u32Pts >> 1) & 0x3FFFFFFF;        /* bit[29:0] */
                origPTS |= ((pdesc->u32Info >> 24) & 0x03) << 30;   /* bit[31:30] */
                break;
            }
        }

        if (origPTS == 0xffffffff)
        {
            pdesc = (struct dmx_desc_data_s *)((desc_addr + desc_size - sizeof(*pdesc)));

            for (; pdesc >= (struct dmx_desc_data_s *)(desc_addr + desc_rd); pdesc--)
            {
                if (pdesc->u32Info & 0x1)
                {
                    origPTS = (pdesc->u32Pts >> 1) & 0x3FFFFFFF;        /* bit[29:0] */
                    origPTS |= ((pdesc->u32Info >> 24) & 0x03) << 30;   /* bit[31:30] */
                    break;
                }
            }
        }
    }

    SYNC_LOGV_DATA("VES: %#x/%#lx, wd:%#x - %#x, VDESC:%#x/%#lx, wd:%#x - %#x, vpts:%#x\n",
        es_len, es_size, es_wr, es_rd, desc_len, desc_size, desc_wr, desc_rd, origPTS);

    return;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_play_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PLAY;

    if (!priv->v_synced_flag)
    {
        priv->v_unsynced_cnt = 0;
        SYNC_LOGV_SYNCED("V synced, vpts: %#x, id: %d\n", priv->cur_vpts, priv->cur_vpts_id);
    }

    priv->v_synced_flag = 1;
    //SYNC_LOGV_SYNCED("VID synced\n");

    if (!priv->av_synced_flag && priv->v_synced_flag && priv->a_synced_flag)
    {
        priv->av_synced_flag = 1;
        priv->disp_flag = 1;
        priv->slow_sync_cnt = 0;

        if (priv->avsync_do_aud_track)
        {
            priv->avsync_do_aud_track = 0;
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        if (priv->avsync_do_trick_seek)
        {
            priv->avsync_do_trick_seek = 0;
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        if (priv->avsync_aud_rollback)
        {
            priv->avsync_aud_rollback = 0;
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        if (priv->av_play_flag)
        {
            priv->av_play_flag = 0;
			SYNC_LOGV_SYNCED("synced, change sync mode to %d(0:none,1:audio,2:video,3:pcr,4:av)\n", priv->avsync_cfg_info.avsync_sync_mode);
            avsync_set_sync_mode(priv, priv->avsync_cfg_info.avsync_sync_mode);
        }

        SYNC_LOGV_SYNCED("AVSYNC_FINISHED\n");
		if(0 == priv->normal_sync_done)
		{
			mt_drv_stat_event(STAT_EVENT_SYNCDONE,0);
			priv->normal_sync_done = 1;
		}
		
        if (priv->p_avsync_psync)
        {
            SYNC_UpLoadEvent(priv->p_avsync_psync, SYNC_AVSYNC_EVENT_STA_CHANGE);
        }
    }

	return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_pause_slow_sync_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PAUSE;
    struct  avsync_vsync_info_t *pvsync_info = NULL;
    mt_u32 mod = 0;

    pvsync_info = &priv->vsync_info;

    switch (pvsync_info->screen_mode)
    {
        case AVSYNC_UNBLANK_FAST:
        case AVSYNC_UNBLANK_STABLE:
        {
            /*
             * Unconditional slow sync
             */
            mod = priv->slow_sync_cnt %
                (AVSYNC_SLOW_SYNC_PAUSE_FRM_CNT + AVSYNC_SLOW_SYNC_PLAY_FRM_CNT);

            if (mod < AVSYNC_SLOW_SYNC_PAUSE_FRM_CNT)
            {
                sync_flag = AVSYNC_FRAME_PAUSE;
                SYNC_LOGV_PAUSE("VID: SLOW Sync: Pause\n");
            }
            else
            {
                priv->disp_flag = 1;
                sync_flag = AVSYNC_FRAME_PLAY;
                SYNC_LOGV_PAUSE("VID: SLOW Sync: Play\n");
            }

            priv->slow_sync_cnt++;
        }
        break;

        case AVSYNC_UNBLANK_SYNC:
        case AVSYNC_UNBLANK_USER:
        {
            /*
             * Conditional slow sync
             */
            if (priv->vsync_diff <= AVSYNC_SLOW_SYNC_PTS_DIFF)
            {
                mod = priv->slow_sync_cnt % (AVSYNC_SLOW_SYNC_PAUSE_FRM_CNT + AVSYNC_SLOW_SYNC_PLAY_FRM_CNT);

                if (mod < AVSYNC_SLOW_SYNC_PAUSE_FRM_CNT)
                {
                    sync_flag = AVSYNC_FRAME_PAUSE;
                    SYNC_LOGV_PAUSE("VID: SLOW Sync: Pause\n");
                }
                else
                {
                    priv->disp_flag = 1;
                    sync_flag = AVSYNC_FRAME_PLAY;
                    SYNC_LOGV_PAUSE("VID: SLOW Sync: Play\n");
                }

                priv->slow_sync_cnt++;
            }
            else
            {
                sync_flag = AVSYNC_FRAME_PAUSE;
                SYNC_LOGV_PAUSE("VID pause\n");
            }
        }
        break;

        default:
            MT_ERR_SYNC("Unblank err:%d\n", pvsync_info->screen_mode);
        break;
    }

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_pause_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PLAY;

    if (!priv->v_synced_flag)
    {
        if (priv->vid_pause_on)
        {
        	//if the sync mode is AVSYNC_REF_AV_MODE, don't do slow sync
            if ((priv->slow_sync_en) && (priv->avsync_sync_mode != AVSYNC_REF_AV_MODE))
            {
                sync_flag = avsync_video_pause_slow_sync_check(priv);
            }
            else
            {
                sync_flag = AVSYNC_FRAME_PAUSE;
                SYNC_LOGV_PAUSE("VID pause\n");
            }
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
            SYNC_LOGV_FREE("VID pause off, VID free\n");
        }
    }
    else
    {
        priv->v_unsynced_cnt++;
        SYNC_LOGV_FREE("VID pause to free\n");

        if (priv->v_unsynced_cnt > AVSYNC_UNSYNCED_THRESHOLD)
        {
            SYNC_LOGV_SYNCED("V unsynced, vpts: %#x, id: %d\n", priv->cur_vpts, priv->cur_vpts_id);
            avsync_video_sync_status_reset(priv);
        }
    }

	return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_skip_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_PLAY;

    if (!priv->v_synced_flag)
    {
        if (priv->vid_skip_on)
        {
            sync_flag = AVSYNC_FRAME_SKIP;
            SYNC_LOGV_SKIP("VID skip\n");
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
            SYNC_LOGV_FREE("VID skip off, VID free\n");
        }
    }
    else
    {
        priv->v_unsynced_cnt++;
        SYNC_LOGV_FREE("VID skip to free\n");

        if (priv->v_unsynced_cnt > AVSYNC_UNSYNCED_THRESHOLD)
        {
            SYNC_LOGV_SYNCED("V unsynced, vpts: %#x, id: %d\n", priv->cur_vpts, priv->cur_vpts_id);
            avsync_video_sync_status_reset(priv);
        }
    }

	return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_free_check(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    if (!priv->v_synced_flag)
    {
        SYNC_LOGV_FREE("VID free: AV Diff(%u) too much! > free thrd(%u)\n",
            abs(priv->cur_apts - priv->cur_vpts)/45, AVSYNC_AV_DIFF_FREE_THRESHOLD/45);
    }
    else
    {
        priv->v_unsynced_cnt++;
        SYNC_LOGV_FREE("VID free to free\n");

        if (priv->v_unsynced_cnt > AVSYNC_UNSYNCED_THRESHOLD)
        {
            SYNC_LOGV_SYNCED("V unsynced, vpts: %#x, id: %d\n", priv->cur_vpts, priv->cur_vpts_id);
            avsync_video_sync_status_reset(priv);
        }
    }

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_check_sync_flag(avsync_priv_t *priv, AVSYNC_FRAME_SYNCFLAG_E sync_flag)
{
    AVSYNC_FRAME_SYNCFLAG_E async_flag = sync_flag;

    switch (async_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
            async_flag = avsync_video_play_check(priv);
		    break;
        }

        case AVSYNC_FRAME_PAUSE:
        {
            async_flag = avsync_video_pause_check(priv);
		    break;
        }

        case AVSYNC_FRAME_SKIP:
        {
            async_flag = avsync_video_skip_check(priv);
		    break;
        }

        case AVSYNC_FRAME_FREE:
        {
            async_flag = avsync_video_free_check(priv);
		    break;
        }

        default:
        {
            MT_ERR_SYNC("%s, %d, error sync result:%d\n", __func__, __LINE__, sync_flag);
		    break;
        }
    }

    return async_flag;
}


AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_ref_sync_flag(avsync_priv_t *priv, mt_u32 apts, mt_u32 vpts)
{
    mt_u32 sync_thresold = 0;
    mt_u32 av_diff = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    if (priv->v_synced_flag)
    {
        sync_thresold = 120*45;
    }
    else
    {
        sync_thresold = 60*45;
    }

    if (vpts >= apts)
    {
        av_diff = vpts - apts;

        if (av_diff <= sync_thresold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
        }
        else if ((av_diff > sync_thresold) && (av_diff <= AVSYNC_AV_DIFF_FREE_THRESHOLD))
        {
            sync_flag = AVSYNC_FRAME_PAUSE;
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
        }
    }
    else
    {
        av_diff = apts - vpts;

        if (av_diff <= sync_thresold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
        }
        else if ((av_diff > sync_thresold) && (av_diff <= AVSYNC_AV_DIFF_FREE_THRESHOLD))
        {
            sync_flag = AVSYNC_FRAME_SKIP;
        }
        else
        {
            sync_flag = AVSYNC_FRAME_FREE;
        }
    }

    priv->vsync_diff = abs(vpts - apts);
    sync_flag = avsync_video_check_sync_flag(priv, sync_flag);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_aud_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 vpts = 0xffffffff;
    mt_u32 stc = 0xffffffff;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    if (!priv->init_stc)
    {
        /*
         need more consideration, if the video output earlier than audio,
         video should be paused, if not, the sync time will increase
		*/
        SYNC_LOGV_FREE("stc not init, VID free\n");
        return sync_flag;
    }

    vpts = priv->cur_vpts;
    avsync_stc_get(&stc);
    priv->cur_stc = stc;
    sync_flag = avsync_video_get_ref_sync_flag(priv, stc, vpts);
    SYNC_LOGVF("vf(aud_ref): %d - %d, vpts: %#x, stc: %#x, dif: %d ms, vid: %d\n",
        priv->av_synced_flag, sync_flag, vpts, stc, abs(vpts - stc)/45, priv->cur_vpts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_vid_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 vpts = 0xffffffff;
    mt_u32 stc = 0xffffffff;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    if (!priv->init_stc)
    {
        /*
         need more consideration, if the video output earlier than audio,
         video should be paused, if not, the sync time will increase
		*/
        SYNC_LOGV_FREE("stc not init, VID free\n");
        return sync_flag;
    }

    vpts = priv->cur_vpts;
    avsync_stc_get(&stc);
    priv->cur_stc = stc;
    sync_flag = avsync_video_get_ref_sync_flag(priv, stc, vpts);
    SYNC_LOGVF("vf(vid_ref): %d - %d, vpts: %#x, stc: %#x, dif: %d ms, vid: %d\n",
        priv->av_synced_flag, sync_flag, vpts, stc, abs(vpts - stc)/45, priv->cur_vpts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_av_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 apts = 0xffffffff;
    mt_u32 vpts = 0xffffffff;
    mt_u32 remain_tm = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    //remain_tm = priv->async_info.ao_data_time;
    remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGV_FREE("priv->cur_apts(0x%x) < remain_time(0x%x), AUD free\n", priv->cur_apts, remain_tm);
        return sync_flag;
    }

    vpts = priv->cur_vpts;
	if(priv->cur_apts_id > 2)
	{
		sync_flag = avsync_video_get_ref_sync_flag(priv, apts, vpts);
	}
    SYNC_LOGVF("vf(av_ref): %d - %d, apts: (%#x - %#x = %#x), vpts: %#x, dif: %d ms, avid: %d - %d\n",
        priv->av_synced_flag, sync_flag, priv->cur_apts, remain_tm, apts, vpts, abs(apts - vpts)/45, 
        priv->cur_apts_id, priv->cur_vpts_id);

    return sync_flag;
}

AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_pcr_ref_sync_flg(avsync_priv_t *priv)
{
    mt_u32 vpts = 0xffffffff;
    mt_u32 stc = 0xffffffff;
    mt_u32 pause_pts = 0;
    mt_u32 skip_pts = 0;
    mt_u32 cur_tick = 0;
    struct pcr_info_t *ppcr_info = NULL;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    ppcr_info = &priv->pcr_info;
    vpts = priv->cur_vpts;

    if (!ppcr_info->pcr_got)
    {
        cur_tick = avsync_osal_get_tick();

        if (0xffffffff == ppcr_info->pcr_probe_start_tick)
        {
            ppcr_info->pcr_probe_start_tick = cur_tick;
			sync_flag = AVSYNC_FRAME_PAUSE;
            SYNC_LOGV_PAUSE("no pcr, VID pause\n");
            return sync_flag;
        }

        if (cur_tick - ppcr_info->pcr_probe_start_tick <= 100)
        {
            sync_flag = AVSYNC_FRAME_PAUSE;
            SYNC_LOGV_PAUSE("no pcr, VID pause\n");
            return sync_flag;
        }
    }

    if (!priv->init_stc)
    {
        SYNC_LOGV_FREE("stc not init, VID free\n");
        return sync_flag;
    }

    pause_pts = vpts - priv->avsync_pause_threhold;
    skip_pts  = vpts + priv->avsync_skip_threhold;

    avsync_stc_get(&stc);
    priv->cur_stc = stc;

    if ((pause_pts <= stc) && (stc <= skip_pts))
    {
        sync_flag = AVSYNC_FRAME_PLAY;
    }
    else if ((stc > skip_pts) && ((stc - vpts) < AVSYNC_AV_DIFF_FREE_THRESHOLD))
    {
        sync_flag = AVSYNC_FRAME_SKIP;
	}
    else if ((stc < pause_pts) && ((vpts - stc) < AVSYNC_AV_DIFF_FREE_THRESHOLD))
    {
        sync_flag = AVSYNC_FRAME_PAUSE;
    }
    else
    {
        sync_flag = AVSYNC_FRAME_FREE;
    }

    priv->vsync_diff = abs(vpts - stc);
    sync_flag = avsync_video_check_sync_flag(priv, sync_flag);
    SYNC_LOGVF("vf(pcr_ref): %d - %d, vpts: %#x, stc: %#x, dif: %d ms, vid: %d\n",
        priv->av_synced_flag, sync_flag, vpts, stc, abs(vpts - stc)/45, priv->cur_vpts_id);

    return sync_flag;
}


AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_sync_flg(avsync_priv_t *priv)
{
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;

    switch (priv->avsync_sync_mode)
    {
         case AVSYNC_REF_AUDIO_MODE:
         {
             sync_flag = avsync_video_get_aud_ref_sync_flg(priv);
             break;
         }

         case AVSYNC_REF_VIDEO_MODE:
         {
             sync_flag = avsync_video_get_vid_ref_sync_flg(priv);
             break;
         }

         case AVSYNC_REF_AV_MODE:
         {
             sync_flag = avsync_video_get_av_ref_sync_flg(priv);
             break;
         }

         case AVSYNC_REF_PCR_MODE:
         {
             sync_flag = avsync_video_get_pcr_ref_sync_flg(priv);
             break;
         }

         default:
         {
            MT_ERR_SYNC("%s, %d, sync ref mode error:%d\n", __func__, __LINE__, priv->avsync_sync_mode);
		 	break;
         }
    }

    return sync_flag;
}

void avsync_video_get_atmos_sync_flg(avsync_priv_t *priv)
{
    if ((0 == priv->avsync_cfg_info.avsync_pause_threhold) &&
        (0 == priv->avsync_cfg_info.avsync_skip_threhold))
    {	

#if 0	
		if(AVSYNC_AUD_OUT_PCM == priv->avsync_aud_output)
		{
			 switch (priv->ddp_prog)
			 {
			 	case AVSYNC_H264_DDP_2997_ATMOS:
				{
					if(1 == priv->atoms_looped)
					{
						priv->avsync_pause_threhold = 50*45;
						priv->avsync_skip_threhold = 50*45;
					}
					else
					{
						priv->avsync_pause_threhold = 40*45;
						priv->avsync_skip_threhold = 40*45;
					}
					break;
				}				

				case AVSYNC_H265_DDP_2997_ATMOS:
					{
						if(1 == priv->atoms_looped)
						{
							priv->avsync_pause_threhold = 50*45;
							priv->avsync_skip_threhold = 50*45;
						}
						else
						{
							priv->avsync_pause_threhold = 40*45;
							priv->avsync_skip_threhold = 40*45;
						}
					}
					break;
				
				case AVSYNC_H265_DDP_5994_ATMOS:
					{
						if(1 == priv->atoms_looped)
						{
							priv->avsync_pause_threhold = 35*45;
							priv->avsync_skip_threhold = 35*45;
						}
						else
						{
							priv->avsync_pause_threhold = 60*45;
							priv->avsync_skip_threhold = 60*45;
						}
					}
					break;
				
				case AVSYNC_MPEG2_DDP_2500_ATMOS:
				case AVSYNC_H264_DDP_2500_ATMOS:
				case AVSYNC_H264_DDP_5000_ATMOS:	
					{
						if(1 == priv->atoms_looped)
						{
							priv->avsync_pause_threhold = 50*45;
							priv->avsync_skip_threhold = 50*45;
						}
						else
						{
							priv->avsync_pause_threhold = 80*45;
							priv->avsync_skip_threhold = 80*45;
						}
						
					}
					break;

				case AVSYNC_H265_DDP_2500_ATMOS:
				case AVSYNC_H265_DDP_5000_ATMOS:
					{
						if(1 == priv->atoms_looped)
						{
							if(MT_DRV_DISP_FMT_1080P_50  == priv->av_play_info.tvformat)
							{
								priv->avsync_pause_threhold = 20*45;
								priv->avsync_skip_threhold = 20*45;
							}
							else
							{
								priv->avsync_pause_threhold = 25*45;
								priv->avsync_skip_threhold = 25*45;
							}
						}
						else
						{
							priv->avsync_pause_threhold = 60*45;
							priv->avsync_skip_threhold = 60*45;
						}
					}
					break;

				case AVSYNC_H264_AC4_2500_ATMOS:
				case AVSYNC_H264_AC4_5000_ATMOS:
				case AVSYNC_H265_AC4_2500_ATMOS:
				case AVSYNC_H265_AC4_5000_ATMOS:	
					{
						if(1 == priv->atoms_looped)
						{
							priv->avsync_pause_threhold = 100*45;
							priv->avsync_skip_threhold = 100*45;
						}
						else
						{
							priv->avsync_pause_threhold = 120*45;
							priv->avsync_skip_threhold = 120*45;
						}
					}
					break;	

				case AVSYNC_H264_AC4_2997_ATMOS:
				case AVSYNC_H265_AC4_2997_ATMOS:
				case AVSYNC_H265_AC4_5994_ATMOS:
					{
						if(1 == priv->atoms_looped)
						{
							priv->avsync_pause_threhold = 120*45;
							priv->avsync_skip_threhold = 120*45;
						}
						else
						{
							priv->avsync_pause_threhold = 100*45;
							priv->avsync_skip_threhold = 100*45;
						}
					}
					break;
				
				default:
					{
						priv->avsync_pause_threhold = 50*45;
						priv->avsync_skip_threhold = 50*45;
					}
					break;
			 }
		}
		else
		{
			priv->avsync_pause_threhold = 80*45;
			priv->avsync_skip_threhold = 80*45;
		}	
#else
		priv->avsync_pause_threhold = 40*45;
		priv->avsync_skip_threhold = 40*45;
#endif
    }
    else
    {
   		priv->avsync_pause_threhold = priv->avsync_cfg_info.avsync_pause_threhold;
        priv->avsync_skip_threhold = priv->avsync_cfg_info.avsync_skip_threhold;
    }
}	
AVSYNC_FRAME_SYNCFLAG_E avsync_video_get_dolby_sync_flg(avsync_priv_t *priv)
{
#ifdef CONFIG_MT_AIAO
    MT_HA_AUDIO_STREAM_INFO_S audio_info;
#endif
    mt_u32 av_diff = 0;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;
    mt_u32 vpts = 0xffffffff;
    mt_u32 apts = 0xffffffff;
    mt_u32 tm = 0;
    mt_u32 tm_dif = 0;
    mt_u32 remain_tm = 0;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null, VID free\n");
        return sync_flag;
    }

    if (priv->dolby_avsync)
    {
        priv->avsync_sync_mode = AVSYNC_REF_VIDEO_MODE;

        if (AVSYNC_DD_NONE_PG == priv->ddp_prog)
        {
            avsync_get_ddp_prog(priv);
        }

        if (!priv->ddp_atmos)
        {
            if ((0 == priv->avsync_cfg_info.avsync_pause_threhold) &&
                (0 == priv->avsync_cfg_info.avsync_skip_threhold))
            {
                switch (priv->ddp_prog)
                {                					
                	case AVSYNC_H264_DD_2500_PG1:
                    case AVSYNC_H264_DDP_2500_PG1:
                        priv->avsync_pause_threhold = 10*45;
                        priv->avsync_skip_threhold = 10*45;
                    break;

					case AVSYNC_H264_DDP_2500_PG2:
                    case AVSYNC_H264_DD_2500_PG2:
                        priv->avsync_pause_threhold = 20*45;
                        priv->avsync_skip_threhold = 10*45;
                    break;				
                    
					case AVSYNC_MPEG2_DD_2500_PG1:
					case AVSYNC_MPEG2_DD_2500_PG2:
                    case AVSYNC_MPEG2_DDP_2500_PG1:
					case AVSYNC_MPEG2_DDP_2500_PG2:
					case AVSYNC_H264_DD_2997_PG1:
					case AVSYNC_H264_DD_2997_PG2:
                    case AVSYNC_H264_DDP_2997_PG1:
					case AVSYNC_H264_DDP_2997_PG2:
                        priv->avsync_pause_threhold = 20*45;
                        priv->avsync_skip_threhold = 10*45;
                    break;

                    default:
                        priv->avsync_pause_threhold = 35*45;
                        priv->avsync_skip_threhold = 20*45;
                    break;
                }
            }
            else
            {
                priv->avsync_pause_threhold = priv->avsync_cfg_info.avsync_pause_threhold;
                priv->avsync_skip_threhold = priv->avsync_cfg_info.avsync_skip_threhold;
            }

#ifdef CONFIG_MT_AIAO
            memset(&audio_info, 0, sizeof(MT_HA_AUDIO_STREAM_INFO_S));

            if(MT_SUCCESS == AO_Track_GetAudioStreamInfo(&audio_info))
            {
                if (MT_SUCCESS == avsync_is_ddp_atmos(priv, &audio_info))
                {                    										
					avsync_video_get_atmos_sync_flg(priv);//if the stream is atmos, only call this func at first time
                }
            }
#endif
        }
        else
        {
        	avsync_video_get_atmos_sync_flg(priv);
        }
    }
    else
    {
        priv->avsync_sync_mode = AVSYNC_REF_AUDIO_MODE;
    }

    priv->vid_sync_sys_time_ms = avsync_osal_get_tick(); 
	priv->vid_dolby_ply_cnt++;

#if 1
	if(priv->vid_dolby_ply_cnt == 1)
	{
		printk(KERN_ERR "first_video vpts %d ms\n",  priv->cur_vpts/45);
		priv->dolby_sync_done = 0;
	}
	
	if(priv->vid_dolby_ply_cnt < 10)
	{	
		sync_flag = AVSYNC_FRAME_PAUSE;
		SYNC_LOGV_PAUSE("VID dolby pause, %d priv->vid_dolby_ply_cnt %d \n", __LINE__, priv->vid_dolby_ply_cnt);		
        return sync_flag;               
	}	
#endif
	
    if ((AVSYNC_REF_NONE_MODE == priv->avsync_sync_mode) ||
        (AVSYNC_REF_VIDEO_MODE == priv->avsync_sync_mode))
    {
        SYNC_LOGV_FREE("sync ref mode is none or video, VID free\n");
        return sync_flag;               
    }

    if (0xffffffff == priv->cur_apts)
    {
        SYNC_LOGV_FREE("AUD no pts, VID free\n");
        return sync_flag;
    }

    tm = avsync_osal_get_tick( );
    tm_dif = tm - priv->aud_sync_sys_time_ms;
    //remain_tm = priv->async_info.ao_data_time;
	remain_tm = avsync_get_ao_data_time();

    if (priv->cur_apts >= remain_tm)
    {
        apts = priv->cur_apts - remain_tm;
    }
    else
    {
        SYNC_LOGA_FREE("apts < remain_time, AUD free\n");
        return sync_flag;
    }

    apts += tm_dif*45;
    vpts = priv->cur_vpts;

    if (apts >= vpts)
    {
        av_diff = apts - vpts;

        if (av_diff <= priv->avsync_skip_threhold)
        {
            sync_flag = AVSYNC_FRAME_PLAY;
        }
        else
        {
            sync_flag = AVSYNC_FRAME_SKIP;
        }
    }
    else
    {
        av_diff = vpts - apts;

        if (av_diff <= priv->avsync_pause_threhold)
        {
            sync_flag = AVSYNC_FRAME_PLAY; 
        }
        else
        {
            sync_flag = AVSYNC_FRAME_PAUSE;
        }
    }

    SYNC_LOGVF("vf: %d, apts: %#x, vpts: %#x, dif: %d, rem: %d, avid: %d - %d\n",
        sync_flag, priv->cur_apts, priv->cur_vpts, av_diff/45, remain_tm/45,
        priv->cur_apts_id, priv->cur_vpts_id);

    if (av_diff > AVSYNC_AV_DIFF_FREE_THRESHOLD)
    {
        SYNC_LOGV_FREE("VID free: AV Diff: %d > Free threhold: %d\n", av_diff/45,
            AVSYNC_AV_DIFF_FREE_THRESHOLD/45);
        sync_flag = AVSYNC_FRAME_FREE;
    }

    switch (sync_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
            SYNC_LOGV_SYNCED("VID dolby synced\n");
            break;
        }
    
        case AVSYNC_FRAME_SKIP:
        {
            SYNC_LOGV_SYNCED("VID dolby skip\n");
            break;
        }
    
        case AVSYNC_FRAME_PAUSE:
        {
            SYNC_LOGV_PAUSE("VID dolby pause, %d\n", __LINE__);
            break;
        }

	    case AVSYNC_FRAME_FREE:
        {
            SYNC_LOGV_FREE("VID dolby free\n");
            break;
        }

        default:
        {
            MT_ALWAYS_PRINT("V sync flag is error:%d\n", sync_flag);
            break;
        }
    }

    return sync_flag;
}

mt_s32 avsync_video_do_sync(avsync_priv_t *priv, AVSYNC_FRAME_SYNCFLAG_E *psync_flg)
{
    struct avsync_vsync_info_t *pvsync_info = NULL;

    if ((NULL == priv) || (NULL == psync_flg))
    {
        return MT_FAILURE;
    }

    *psync_flg = AVSYNC_FRAME_FREE;
    pvsync_info = &priv->vsync_info;

    if (DEV_INIT != priv->vid_dev_sta)
    {
        SYNC_LOGV_FREE("VID not init, VID free\n");
        return MT_SUCCESS;
    }

    if (0xffffffff == priv->cur_vpts)
    {
        SYNC_LOGV_FREE("VID no pts, VID free\n");
        return MT_FAILURE;
    }

    if (priv->do_ddp_verf)
    {
        *psync_flg = avsync_video_get_dolby_sync_flg(priv);
        return MT_SUCCESS;
    }

    if (AVSYNC_REF_NONE_MODE == priv->avsync_sync_mode)
    {
        SYNC_LOGV_FREE("AV free mode, VID free\n");
		priv->v_synced_flag = 1;
        return MT_SUCCESS;
    }

    if (AVSYNC_REF_PCR_MODE == priv->avsync_sync_mode)
    {
        goto SYNC_CB;
    }

    if (DEV_INIT != priv->aud_dev_sta)
    {
        SYNC_LOGV_FREE("AUD not init, VID free\n");
        return MT_FAILURE;
    }

    if ((!priv->vstabled_flag) || (!priv->astabled_flag))
    {
		SYNC_LOGV_FREE("%s %s pts unstable, VID free\n", ((priv->astabled_flag) ? "" : "AUD"),
            ((priv->vstabled_flag) ? "" : "VID"));
        return MT_SUCCESS;
    }

SYNC_CB:
    *psync_flg = avsync_video_get_sync_flg(priv);
	return MT_SUCCESS;
}

mt_s32 avsync_video_play_func(void)
{
    return MT_SUCCESS;
}

mt_s32 avsync_video_pause_func(void)
{
    return DF_AvsyncSetRepeatFrame_Wrap();
}

mt_s32 avsync_video_skip_func(void)
{
    return DF_AvsyncSetSkipFrame_Wrap(1);
}

mt_s32 avsync_video_free_func(void)
{
    return MT_SUCCESS;
}

mt_void avsync_video_loop(mt_void *pdata)
{
    avsync_priv_t *priv = NULL;
    AVSYNC_FRAME_SYNCFLAG_E sync_flag = AVSYNC_FRAME_FREE;
	mt_handle disp_hanle = (mt_handle)pdata;
    mt_s32 ret = MT_SUCCESS;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null, VID free\n");
        return;
    }


    ret = avsync_video_pts_get(priv, disp_hanle);
    if (MT_SUCCESS != ret)
    {
      //  SYNC_LOGV_FREE("get vpts failed, VID free\n");
        return;
    }
	
    avsync_video_pts_check(priv);
    avsync_video_es_level_check(priv);
    avsync_video_do_sync(priv, &sync_flag);

    switch (sync_flag)
    {
        case AVSYNC_FRAME_PLAY:
        {
            if (MT_SUCCESS != avsync_video_pts_repeat_check(priv))
                priv->vid_play_cnt++;
            avsync_video_play_func();
            break;
        }

        case AVSYNC_FRAME_SKIP:
        {
            priv->vid_skip_cnt++;
            avsync_video_skip_func();
            break;
        }

        case AVSYNC_FRAME_PAUSE:
        {
            priv->vid_pause_cnt++;
            avsync_video_pause_func();
            break;
        }

	    case AVSYNC_FRAME_FREE:
        {
            if (MT_SUCCESS != avsync_video_pts_repeat_check(priv))
                priv->vid_free_cnt++;
            avsync_video_free_func();
            break;
        }

        default:
        {
            if (MT_SUCCESS != avsync_video_pts_repeat_check(priv))
                priv->vid_free_cnt++;
            avsync_video_free_func();
            MT_ALWAYS_PRINT("V sync flag is error:%d\n", sync_flag);
            break;
        }
    }

    if ((priv->vstabled_flag) && (AVSYNC_REF_VIDEO_MODE == priv->avsync_sync_mode))
    {
        avsync_stc_adjust_by_vpts(priv);
    }

    priv->vsync.last_vpts = priv->cur_vpts;
    priv->vsync.last_vpts_id = priv->cur_vpts_id;
}
EXPORT_SYMBOL(avsync_video_loop);

mt_s32 avsync_video_frame_info_capture_create(unsigned int num)
{
    avsync_recorder_queue *queue = &g_vid_frame_info_queue;

    if (0 == num)
    {
        return MT_FAILURE;
    }

    queue->ridx = 0;
    queue->widx = 0;
    queue->total = num;
    queue->pdata = kmalloc(num * sizeof(SYNC_VOUT_FRAME_INFO), GFP_KERNEL);

    if (NULL == queue->pdata)
    {
        MT_INFO_SYNC("failed to alloc %u bytes memory for frame info capture\n",
            num * sizeof(SYNC_VOUT_FRAME_INFO));
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}
EXPORT_SYMBOL(avsync_video_frame_info_capture_create);

static mt_s32 avsync_video_frame_info_capture_write(mt_u32 frame_pts, mt_u32 frame_id)
{
    avsync_recorder_queue *queue = &g_vid_frame_info_queue;
    static mt_dvb_pts32 last_pts = 0;
    static mt_u32 last_frame_id = 0;
    static mt_u32 last_frame_crc = 0;
    static mt_u32 same_cnt = 0;
    SYNC_VOUT_FRAME_INFO vout_info;
    mt_u32 frm_crc = 0;
    unsigned int ofs = 0;
    mt_s32 ret = 0;


#ifdef CONFIG_MT_CHIP_SYMPHONY6
    frm_crc = drv_reg_4k_disp_get_video_hd_out_top_crc();
#else
    goto the_end;
#endif

    /* discard the same frame */    
    if (frame_id == last_frame_id)
    {
        if (last_frame_crc == frm_crc)
        {
            same_cnt++;
        }
        
        if (same_cnt < 20)
        {
            goto the_end;
        }
    }

//    printk(KERN_ERR  "drv_crc %u-%u, %08x-%d\n", frame_id, last_frame_id, frm_crc, frame_pts/45);
    same_cnt = 0;
    if (NULL == queue->pdata)
    {
        goto the_end;
    }

    memset(&vout_info, 0, sizeof(vout_info));

    if (same_cnt < 20)
    {       
        /* save crc and its related frame information */
        vout_info.idx       = frame_id;
        vout_info.vpts      = frame_pts;
        vout_info.out_crc   = frm_crc;
    }
    else
    {
        vout_info.idx       = 0xffffffff;
        vout_info.vpts      = 0xffffffff;
        vout_info.out_crc   = 0xffffffff;
    }

    /* check if there is enougth space to write */
    ofs = queue->widx + 1;
    if (ofs >= queue->total)
        ofs = 0;

    if (ofs != queue->ridx)
    {
        unsigned int wofs = queue->widx * sizeof(vout_info);
        memcpy(queue->pdata + wofs, &vout_info, sizeof(vout_info));        
        queue->widx = ofs;
    }
    else
    {
        MT_INFO_SYNC("Error: video frame info capture overrun\n");
        ret = -1;
    }

    if(last_frame_id + 1 != frame_id)
    {
        MT_INFO_SYNC("idx discont %u -> %u\n", last_frame_id, vout_info.idx);
    }

    same_cnt = 0;

the_end:
    /* update last frame index and vpts */
    last_frame_id   = frame_id;
    last_pts        = frame_pts;
    last_frame_crc  = frm_crc;

    /* Any lock should not be applied in this func due to it will be called in ISR */
    if(g_vid_frame_info_cap_break)
    {
        queue->pdata = NULL;
        g_vid_frame_info_cap_break = 0;
    }

    return ret;
}

mt_s32 avsync_video_frame_info_capture_read(SYNC_VOUT_FRAME_INFO *frame_info)
{
    avsync_recorder_queue *queue = &g_vid_frame_info_queue;
    SYNC_VOUT_FRAME_INFO *info;
    unsigned int ofs = 0;

    if (NULL == frame_info)
    {
        MT_INFO_SYNC("invalid param for frame info capture\n");
        return -EINVAL;
    }
    
    if (queue->ridx == queue->widx)
    {
        MT_INFO_SYNC("no video frame info in capture for pop\n");
        return MT_FAILURE;
    }

    if (NULL == queue->pdata)
    {
        MT_INFO_SYNC("invalid param for frame info capture\n");
        return -EINVAL;
    }
    
    if (queue->ridx >= queue->total)
    {
        MT_INFO_SYNC("invalid ridx %u, range: 0 ~ %u\n", queue->ridx, queue->total);
        return MT_FAILURE;
    }

    ofs = queue->ridx * sizeof(SYNC_VOUT_FRAME_INFO);
    info = (SYNC_VOUT_FRAME_INFO *)(queue->pdata + ofs);
    memcpy(frame_info, info, sizeof(SYNC_VOUT_FRAME_INFO));
    MT_INFO_SYNC("kvout: %u -> %u, %x, %p\n", frame_info->idx, frame_info->vpts, frame_info->out_crc, frame_info);
    queue->ridx++;

    if (queue->ridx >= queue->total)
        queue->ridx = 0;

    return 0;
}
EXPORT_SYMBOL(avsync_video_frame_info_capture_read);

mt_s32 avsync_video_frame_info_capture_destroy(void)
{
    avsync_recorder_queue *queue = &g_vid_frame_info_queue;
    unsigned char *buf = queue->pdata;
    int timeout_cnt = 40;

    if (buf != NULL)
    {        
        /* notify the write func to break the capture */
        g_vid_frame_info_cap_break = 1;

        while (g_vid_frame_info_cap_break && (timeout_cnt > 0))
        {
            timeout_cnt--;
            msleep(2);
        }

        kfree(buf);

        if ((timeout_cnt <= 0) && g_vid_frame_info_cap_break)
        {
            MT_INFO_SYNC("FIXME: %s timeout was unexpect\n", __func__);
        }
    }

    memset(queue, 0, sizeof(avsync_recorder_queue));

    return MT_SUCCESS;
}
EXPORT_SYMBOL(avsync_video_frame_info_capture_destroy);

mt_s32 avsync_is_ddp_avsync_verfication(void)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    if (priv->do_ddp_verf)
    {
        return  MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}
EXPORT_SYMBOL(avsync_is_ddp_avsync_verfication);

mt_s32 avsync_video_get_play_cnt(void)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return 0;
    }

    if (avsync_is_finished())
    {
        return (priv->vid_play_cnt + priv->vid_free_cnt);
    }
    else
    {
		return 0;
    }
}

void avsync_set_aud_frame_sample_num(mt_u32 num)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return;
    }

    priv->aud_frame_sample_num = num;
}

/* the configuration of frame 29.97
 * 0xbf5d005c[31:8] must be 0xfdf4b9, do not change 0xbf5d005c[7:0]
 * 0xbf5d0058[7:4] the value shoule be 0x2
 * 0xbf5d000c bit8 must be 1, not care the bit16 and bit0
 * 0xbf5d00b0[31:16] must be 0xfdf4
 * 0xbf5d00b4[31:16] must be 0xfdf4
 */
mt_s32 avsync_set_display_for_dolby_stream(avsync_priv_t *priv)
{
    mt_u32 val = 0;
    ulong reg = 0;

	if((MT_DRV_DISP_FMT_1080P_60 == priv->av_play_info.tvformat) ||
		(MT_DRV_DISP_FMT_3840X2160_60 == priv->av_play_info.tvformat) ||
		(MT_DRV_DISP_FMT_4096X2160_60 == priv->av_play_info.tvformat))
	{
	    reg = R_CLKGEN_VIDEO_SW_REG1;	//0xbf5d005c
		val = HAL_GET_U32((volatile u32 *)reg);

		R_CLKGEN_VIDEO_SW_REG1_VALUE = val;

		if ((MT_DRV_DISP_FMT_3840X2160_60 == priv->av_play_info.tvformat)||
			(MT_DRV_DISP_FMT_4096X2160_60 == priv->av_play_info.tvformat))
		{
			MT_INFO_SYNC("[%s %d]4K g_avsync_private.av_play_info.tvformat=%d\n", __FUNCTION__, __LINE__, priv->av_play_info.tvformat);
			val &= 0xf1;//clear bit31-8 and bit3-1
		    val |= 0xfdf4b906;//bit31-8 set to 0xfdf4b9, bit3-1 set to 011
		}
		else
		{
			MT_INFO_SYNC("[%s %d]tvformat is MT_DRV_DISP_FMT_1080P_60\n", __FUNCTION__, __LINE__);
		    val &= 0xff;		//clear bit31-8
		    val |= 0xfdf4b900;	//bit31-8 set to 0xfdf4b9
		}
		MT_INFO_SYNC("[%s %d]set reg(0xbf5d005c)=0x%08x\n", __FUNCTION__, __LINE__, val);
	    HAL_PUT_U32((volatile u32 *)reg, val);	//0xbf5d005c

	    reg = R_CLKGEN_VIDEO_SW_REG0;	//0xbf5d0058	    
	    val = HAL_GET_U32((volatile u32 *)reg);
		R_CLKGEN_VIDEO_SW_REG0_VALUE = val;
		val &= 0xffffff0f;	//clear bit7~bit4	
		//val |= 0xF0;
		val |= 0x20;
		MT_INFO_SYNC("[%s %d]set reg(0xbf5d0058)=0x%08x\n", __FUNCTION__, __LINE__, val);
	    HAL_PUT_U32((volatile u32 *)reg, val);	//0xbf5d0058	    
	    
		R_CLKGEN_VIDEO_SW_HD_REG0_VALUE = HAL_GET_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG0);	//0xbf5d00b0
		R_CLKGEN_VIDEO_SW_HD_REG1_VALUE = HAL_GET_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG1);	//0xbf5d00b4
		MT_INFO_SYNC("[%s %d]set reg(0xbf5d00b0)=0xfdf48000\n", __FUNCTION__, __LINE__);
	    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG0, 0xfdf48000);	//0xbf5d00b0

		MT_INFO_SYNC("[%s %d]set reg(0xbf5d00b4)=0xfdf48000\n", __FUNCTION__, __LINE__);
	    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG1, 0xfdf48000);	//0xbf5d00b4		

		//must set R_INTP_CTRL_REG1 at last
		val = HAL_GET_U32((volatile u32 *)R_INTP_CTRL_REG1);	//0xbf5d000c
		R_INTP_CTRL_REG1_VALUE = val;
	    val = val | (1<<16);  //needn't
	    val = val | (1<<8);  //bit8 must be 1
	    val = val & (~(1<<0));  //needn't
		MT_INFO_SYNC("[%s %d]set reg(0xbf5d000c)=0x%08x\n", __FUNCTION__, __LINE__, val);
	    HAL_PUT_U32((volatile u32 *)R_INTP_CTRL_REG1, val);	//0xbf5d000c
	    
		MT_INFO_SYNC("[%s %d]default reg(0xbf5d0058)=0x%08x\n", __FUNCTION__, __LINE__, R_CLKGEN_VIDEO_SW_REG0_VALUE);
		MT_INFO_SYNC("[%s %d]default reg(0xbf5d005c)=0x%08x\n", __FUNCTION__, __LINE__, R_CLKGEN_VIDEO_SW_REG1_VALUE);		
		MT_INFO_SYNC("[%s %d]default reg(0xbf5d00b0)=0x%08x\n", __FUNCTION__, __LINE__, R_CLKGEN_VIDEO_SW_HD_REG0_VALUE);
		MT_INFO_SYNC("[%s %d]default reg(0xbf5d00b4)=0x%08x\n", __FUNCTION__, __LINE__, R_CLKGEN_VIDEO_SW_HD_REG1_VALUE);
		MT_INFO_SYNC("[%s %d]default reg(0xbf5d000c)=0x%08x\n", __FUNCTION__, __LINE__, R_INTP_CTRL_REG1_VALUE);
		
		MT_INFO_SYNC("[%s %d]reg(0xbf5d0058)=0x%x\n", __FUNCTION__, __LINE__, HAL_GET_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_REG0));
		MT_INFO_SYNC("[%s %d]reg(0xbf5d005c)=0x%x\n", __FUNCTION__, __LINE__, HAL_GET_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_REG1));		
		MT_INFO_SYNC("[%s %d]reg(0xbf5d00b0)=0x%x\n", __FUNCTION__, __LINE__, HAL_GET_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG0));
		MT_INFO_SYNC("[%s %d]reg(0xbf5d00b4)=0x%x\n", __FUNCTION__, __LINE__, HAL_GET_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG1));
		MT_INFO_SYNC("[%s %d]reg(0xbf5d000c)=0x%x\n", __FUNCTION__, __LINE__, HAL_GET_U32((volatile u32 *)R_INTP_CTRL_REG1));
		g_display_value_seted = 1;

	}
    return MT_SUCCESS;
}

mt_s32 avsync_module_init(mt_void)
{
    avsync_priv_t *priv = NULL;

    MT_INFO_SYNC("avsync_module_init in\n");

    priv = kmalloc(sizeof(avsync_priv_t), GFP_KERNEL);

    if (!priv)
    {
        MT_ERR_SYNC("malloc avsync priv failed\n");
        return -ENOMEM;
	}

    g_p_avsync_private = priv;
    memset(priv, 0, sizeof(avsync_priv_t));
	avsync_private_reset(priv);
    avsync_module_default_cfg(priv);

    symphony_avsync_set_stc_cnt_load_mode(8);
    symphony_pcr_sel_cfg(0);
    symphony_audio_sel_cfg(1);
    symphony_video_sel_cfg(0);
    avsync_int_register();

    MT_INFO_SYNC("avsync_module_init end\n");
    return MT_SUCCESS;
}

mt_s32 avsync_module_resume(mt_void)
{
	if(g_p_avsync_private != NULL)
	{
	    memset(g_p_avsync_private, 0, sizeof(avsync_priv_t));
		avsync_private_reset(g_p_avsync_private);
	    avsync_module_default_cfg(g_p_avsync_private);
	}
    symphony_avsync_set_stc_cnt_load_mode(8);
    symphony_pcr_sel_cfg(0);
    symphony_audio_sel_cfg(1);
    symphony_video_sel_cfg(0);
    avsync_int_register();
    return MT_SUCCESS;
}

mt_s32 avsync_module_deinit(mt_void)
{
    MT_INFO_SYNC("avsync_module_deinit in\n");

    avsync_int_unregister();
    kfree(g_p_avsync_private);
    g_p_avsync_private = NULL;

    MT_INFO_SYNC("avsync_module_deinit end\n");
    return MT_SUCCESS;
}

mt_s32 avsync_resume_sync_mode(avsync_priv_t *priv)
{
    if (!priv)
    {
        return MT_FAILURE;
    }

    switch (priv->avsync_cfg_info.avsync_sync_mode)
    {
        case AVSYNC_REF_AUDIO_MODE:
        {
            priv->aud_pause_on = 0;
            priv->aud_skip_on = 0;
            priv->vid_pause_on = 1;
            priv->vid_skip_on = 1;
            priv->avsync_sync_mode = AVSYNC_REF_AUDIO_MODE;
            MT_INFO_SYNC("back sync mode to aud\n");
        }
        break;

        case AVSYNC_REF_VIDEO_MODE:
        {
            priv->aud_pause_on = 1;
            priv->aud_skip_on = 1;
            priv->vid_pause_on = 0;
            priv->vid_skip_on = 0;
            priv->avsync_sync_mode = AVSYNC_REF_VIDEO_MODE;
            MT_INFO_SYNC("back sync mode to vid\n");
        }
        break;

        default:
            MT_ERR_SYNC("user cfg sync mode err:%d\n",
                priv->avsync_cfg_info.avsync_sync_mode);
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 avsync_set_sync_mode(avsync_priv_t *priv, avsync_sync_mode_e sync_mode)
{
    if (!priv)
    {
        return MT_FAILURE;
    }

    switch (sync_mode)
    {
        case AVSYNC_REF_NONE_MODE:
        {
            priv->aud_pause_on = 0;
            priv->aud_skip_on = 0;
            priv->vid_pause_on = 0;
            priv->vid_skip_on = 0;
            priv->avsync_sync_mode = AVSYNC_REF_NONE_MODE;
            MT_INFO_SYNC("set sync mode to none\n");
        }
        break;

        case AVSYNC_REF_AUDIO_MODE:
        {
            priv->init_stc = 0;
            priv->aud_pause_on = 0;
            priv->aud_skip_on = 0;
            priv->vid_pause_on = 1;
            priv->vid_skip_on = 1;
            priv->avsync_sync_mode = AVSYNC_REF_AUDIO_MODE;
            MT_INFO_SYNC("set sync mode to aud\n");
        }
        break;

        case AVSYNC_REF_VIDEO_MODE:
        {
            priv->init_stc = 0;
            priv->aud_pause_on = 1;
            priv->aud_skip_on = 1;
            priv->vid_pause_on = 0;
            priv->vid_skip_on = 0;
            priv->avsync_sync_mode = AVSYNC_REF_VIDEO_MODE;
            MT_INFO_SYNC("set sync mode to vid\n");
        }
        break;

        case AVSYNC_REF_PCR_MODE:
        {
            priv->aud_pause_on = 1;
            priv->aud_skip_on = 1;
            priv->vid_pause_on = 1;
            priv->vid_skip_on = 1;
            priv->avsync_sync_mode = AVSYNC_REF_PCR_MODE;
            MT_INFO_SYNC("set sync mode to pcr\n");
        }
        break;

        /*
         * a temp sync mode for steam rollback, set by drv not user
		 */
        case AVSYNC_REF_AV_MODE:
        {
            if ((AVSYNC_REF_PCR_MODE == priv->avsync_sync_mode)
                || (AVSYNC_REF_NONE_MODE == priv->avsync_sync_mode))
            {
                MT_ERR_SYNC("cur sync mode err:%d\n", priv->avsync_sync_mode);
                return MT_FAILURE;
            }

            priv->aud_pause_on = 1;
            priv->aud_skip_on = 1;
            priv->vid_pause_on = 1;
            priv->vid_skip_on = 1;
			/*
             * do not update priv->avsync_cfg_info.avsync_sync_mode
			 */
            priv->avsync_sync_mode = AVSYNC_REF_AV_MODE;
            MT_INFO_SYNC("set sync mode to av\n");
        }
        break;

        default:
            MT_ERR_SYNC("cur sync mode err:%d\n", sync_mode);
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 avsync_set_user_sync_mode(avsync_priv_t *priv, avsync_sync_mode_e sync_mode)
{
    if (!priv)
    {
        return MT_FAILURE;
    }

    switch (sync_mode)
    {
        case AVSYNC_REF_NONE_MODE:
        {
            priv->avsync_cfg_info.avsync_sync_mode = AVSYNC_REF_NONE_MODE;
            MT_INFO_SYNC("set user sync mode to none\n");
        }
        break;

        case AVSYNC_REF_AUDIO_MODE:
        {
            priv->avsync_cfg_info.avsync_sync_mode = AVSYNC_REF_AUDIO_MODE;
            MT_INFO_SYNC("set user sync mode to aud\n");
        }
        break;

        case AVSYNC_REF_VIDEO_MODE:
        {
            priv->avsync_cfg_info.avsync_sync_mode = AVSYNC_REF_VIDEO_MODE;
            MT_INFO_SYNC("set user sync mode to vid\n");
        }
        break;

        case AVSYNC_REF_PCR_MODE:
        {
            priv->avsync_cfg_info.avsync_sync_mode = AVSYNC_REF_PCR_MODE;
            MT_INFO_SYNC("set user sync mode to pcr\n");
        }
        break;

        default:
            MT_ERR_SYNC("cur user sync mode err:%d\n", sync_mode);
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

mt_s32 avsync_set_slow_sync_policy(MT_BOOL on_off)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    if (!priv)
    {
        return MT_FAILURE;
    }

    if (MT_TRUE == on_off)
    {
        priv->slow_sync_en = 1;
    }
    else
    {
        priv->slow_sync_en = 0;
    }

	return MT_SUCCESS;
}

MT_BOOL avsync_is_finished(mt_void)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FALSE;
    }

    if (priv->disp_flag)
    {
        return MT_TRUE;
    }

    if (priv->vid_play_cnt > 0)
    {
        return MT_TRUE;
    }

    if (priv->vid_push_cnt > AVSYNC_DEFAULT_TIME_OUT_DISPLAY)
    {
        return MT_TRUE;
    }

    return MT_FALSE;
}
EXPORT_SYMBOL(avsync_is_finished);

mt_void avsync_get_current_psync(SYNC_S *pSync)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return;
    }

    priv->p_avsync_psync = pSync;
}

irqreturn_t avsync_handle_isr(mt_s32 irq, mt_void *dev_id)
{
    avsync_priv_t *priv = NULL;
    mt_u32 int_sta = 0;
    mt_u32 pcr = 0;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return IRQ_NONE;
    }

    int_sta = symphony_avsync_get_int_sta();
    symphony_avsync_clr_int_sta(int_sta);

    if (int_sta & (1 << BIT_PCR_INT_STA))
    {
        pcr = symphony_pcr_get();
        avsync_pcr_process_isr(priv, pcr);
    }

    if (int_sta & (1 << BIT_APTS_INT_STA))
    {
        avsync_audio_pts_parsed_isr(priv);
    }

    if (int_sta & (1 << BIT_AUD_ONE_FRM_OUT_INT_STA))
    {
        avsync_audio_one_frame_out_isr(priv);
    }

    return IRQ_HANDLED;
}

mt_s32 avsync_int_register(void)
{
    mt_s32 ret = MT_SUCCESS;

    symphony_avsync_irq_enable(BIT_PCR_INT_EN);
    symphony_avsync_irq_enable(BIT_APTS_INT_EN);
    symphony_avsync_irq_enable(BIT_AUD_ONE_FRM_OUT_INT_EN);
    ret = request_irq(IRQ_AVSYNC_ID, (irq_handler_t)avsync_handle_isr, IRQF_TRIGGER_HIGH, "mt_avsync_irq", MT_NULL);

    if (0 != ret)
    {
        MT_ERR_SYNC("request_irq IRQ_AVSYNC_ID error:%d\n", ret);
    }

    return ret;
}

mt_s32 avsync_int_unregister(void)
{
    mt_s32 ret = MT_SUCCESS;

    symphony_avsync_irq_disable(BIT_PCR_INT_EN);
    symphony_avsync_irq_disable(BIT_APTS_INT_EN);
    symphony_avsync_irq_disable(BIT_AUD_ONE_FRM_OUT_INT_EN);
    free_irq(IRQ_AVSYNC_ID, MT_NULL);

    return ret;
}

mt_u64 avsync_pts_to_us(mt_u32 pts)
{
    mt_u64 us = 0;

	if(pts == 0xffffffff)
		return us;
	
    us = (mt_u64)pts;
    us *= 1000;
    do_div(us, 45);

    return us;
}

mt_s32 avsync_get_time_info(mt_u32 SyncId, SYNC_GET_TIME_INFO_S *pSyncTimeInfo)
{
    avsync_priv_t *priv = NULL;
    mt_u64 apts = 0;
    mt_u64 vpts = 0;
    mt_u64 stc  = 0;
    mt_u64 pcr  = 0;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    if (NULL == pSyncTimeInfo)
    {
         MT_ERR_SYNC("para null\n");
         return MT_FAILURE;
    }

    apts = avsync_pts_to_us(priv->cur_apts);
    vpts = avsync_pts_to_us(priv->cur_vpts);
	stc = avsync_pts_to_us(priv->cur_stc);
    pcr = avsync_pts_to_us(priv->cur_pcr);

    pSyncTimeInfo->AudLastPts = apts;
    pSyncTimeInfo->VidLastPts = vpts;
    pSyncTimeInfo->VidAudDiff = vpts - apts;
    pSyncTimeInfo->AudFirstPts = avsync_pts_to_us(priv->first_apts);
    pSyncTimeInfo->VidFirstPts = avsync_pts_to_us(priv->first_vpts);
    pSyncTimeInfo->LocalTime = 0; // stc;
    pSyncTimeInfo->PlayTime = 0; //pcr;

    return MT_SUCCESS;
}

mt_void avsync_dump_state_proc(struct seq_file *p)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return;
    }

    switch (priv->data_source_mode)
    {
        case SYNC_DATA_SOURCE_TUNER:
            PROC_PRINT(p, "\t                      Inject Mode: NIM\n");
        break;

        case SYNC_DATA_SOURCE_FILE:
            PROC_PRINT(p, "\t                      Inject Mode: FILE\n");
        break;

        case SYNC_DATA_SOURCE_PVR:
            PROC_PRINT(p, "\t                      Inject Mode: PVR\n");
        break;

        case SYNC_DATA_SOURCE_NET:
            PROC_PRINT(p, "\t                      Inject Mode: IPTV\n");
        break;

        default:
            PROC_PRINT(p, "\t                      Inject Mode: UNKNOWN\n");
        break;
    }

    PROC_PRINT(p, "\t                      Aud Skip On: %u\n", priv->aud_skip_on);
    PROC_PRINT(p, "\t                     Aud Pause On: %u\n", priv->aud_pause_on);
    PROC_PRINT(p, "\t                      Vid Skip On: %u\n", priv->vid_skip_on);
    PROC_PRINT(p, "\t                     Vid Pause On: %u\n", priv->vid_pause_on);
    PROC_PRINT(p, "\t                         Stc Init: %u\n", priv->init_stc);

    switch (priv->avsync_sync_mode)
    {
        case AVSYNC_REF_NONE_MODE:
        {
             PROC_PRINT(p, "\t                    Sync Ref Mode: AVSYNC_REF_NONE_MODE\n");
        }
        break;

        case AVSYNC_REF_AUDIO_MODE:
        {
             PROC_PRINT(p, "\t                    Sync Ref Mode: AVSYNC_REF_AUDIO_MODE\n");
        }
        break;

        case AVSYNC_REF_VIDEO_MODE:
        {
             PROC_PRINT(p, "\t                    Sync Ref Mode: AVSYNC_REF_VIDEO_MODE\n");
        }
        break;

        case AVSYNC_REF_PCR_MODE:
        {
             PROC_PRINT(p, "\t                    Sync Ref Mode: AVSYNC_REF_PCR_MODE\n");
        }
        break;

        case AVSYNC_REF_AV_MODE:
        {
             PROC_PRINT(p, "\t                    Sync Ref Mode: AVSYNC_REF_AV_MODE\n");
        }
        break;

        default:
        {
            PROC_PRINT(p, "\t                    Sync Ref Mode: AVSYNC UNKNOWN REF MODE\n");
        }
        break;
    }

    switch (priv->avsync_cfg_info.avsync_sync_mode)
    {
        case AVSYNC_REF_NONE_MODE:
        {
            PROC_PRINT(p, "\t           User Cfg Sync Ref Mode: AVSYNC_REF_NONE_MODE\n");
        }
        break;

        case AVSYNC_REF_AUDIO_MODE:
        {
            PROC_PRINT(p, "\t           User Cfg Sync Ref Mode: AVSYNC_REF_AUDIO_MODE\n");
        }
        break;

        case AVSYNC_REF_VIDEO_MODE:
        {
            PROC_PRINT(p, "\t           User Cfg Sync Ref Mode: AVSYNC_REF_VIDEO_MODE\n");
        }
        break;

        case AVSYNC_REF_PCR_MODE:
        {
            PROC_PRINT(p, "\t           User Cfg Sync Ref Mode: AVSYNC_REF_PCR_MODE\n");
        }
        break;

        default:
        {
            PROC_PRINT(p, "\t           User Cfg Sync Ref Mode: AVSYNC UNKNOWN REF MODE\n");
        }
        break;
    }

    PROC_PRINT(p, "\t                 Audio Fifo Count: %u\n", avsync_audio_calc_pts_fifo(priv));
    PROC_PRINT(p, "\t                 Audio Sync Count: %u\n", priv->aud_push_cnt);
    PROC_PRINT(p, "\t            Audio Push Fifo Count: %u\n", priv->apts_fifo_push_cnt);
    PROC_PRINT(p, "\t                  Audio Pop Count: %u\n", priv->apts_fifo_pop_cnt);
    PROC_PRINT(p, "\t                 Audio Play Count: %u\n", priv->aud_play_cnt);
    PROC_PRINT(p, "\t                 Audio Skip Count: %u\n", priv->aud_skip_cnt);
    PROC_PRINT(p, "\t                Audio Pause Count: %u\n", priv->aud_pause_cnt);
    PROC_PRINT(p, "\t                 Audio Free Count: %u\n", priv->aud_free_cnt);
    PROC_PRINT(p, "\t                  Video Slow Sync: %u\n", priv->slow_sync_en);
    PROC_PRINT(p, "\t                 Video Sync Count: %u\n", priv->vid_push_cnt);
    PROC_PRINT(p, "\t                 Video Play Count: %u\n", priv->vid_play_cnt);
    PROC_PRINT(p, "\t                 Video Skip Count: %u\n", priv->vid_skip_cnt);
    PROC_PRINT(p, "\t                Video Pause Count: %u\n", priv->vid_pause_cnt);
    PROC_PRINT(p, "\t                 Video Free Count: %u\n", priv->vid_free_cnt);
    PROC_PRINT(p, "\t                       Apts Stble: %u\n", priv->astabled_flag);
    PROC_PRINT(p, "\t                       Vpts Stble: %u\n", priv->vstabled_flag);

	if (priv->async_info.audio_bypass)
	{
		PROC_PRINT(p, "\t                     audio bypass: %u\n", priv->async_info.audio_bypass);
		PROC_PRINT(p, "\t                Apts adjust param: %#x\n", priv->audio_bypass_apts_adjust);
		PROC_PRINT(p, "\t                             Apts: %#x\n", priv->cur_apts);
	    PROC_PRINT(p, "\t                             Vpts: %#x\n", priv->cur_vpts);
	    PROC_PRINT(p, "\t                              Stc: %#x\n", priv->cur_stc);
	    PROC_PRINT(p, "\t                       AVpts Diff: %u ms\n", abs(priv->cur_vpts - priv->cur_stc)/45);
	}
	else
	{
	    PROC_PRINT(p, "\t                             Apts: %#x\n", priv->isr_pop.pts);
	    PROC_PRINT(p, "\t                             Vpts: %#x\n", priv->cur_vpts);
	    PROC_PRINT(p, "\t                              Stc: %#x\n", priv->cur_stc);
	    PROC_PRINT(p, "\t                       AVpts Diff: %u ms\n", abs(priv->cur_vpts - priv->isr_pop.pts)/45);
	}
	
    PROC_PRINT(p, "\t                          Afrm Id: %u\n", priv->cur_apts_id);
    PROC_PRINT(p, "\t                          Vfrm Id: %u\n", priv->cur_vpts_id);
    PROC_PRINT(p, "\t                       Async Flag: %u\n", priv->a_synced_flag);
    PROC_PRINT(p, "\t                       Vsync Flag: %u\n", priv->v_synced_flag);
    PROC_PRINT(p, "\t                      AVSync Flag: %u\n", priv->av_synced_flag);
    PROC_PRINT(p, "\t              Audio Underflow Cnt: %u\n", priv->aud_underflow_cnt);
    PROC_PRINT(p, "\t                       Audio Step: %u\n", priv->apts_step/45);
    PROC_PRINT(p, "\t                       Video Step: %u\n", priv->vpts_step/45);
    PROC_PRINT(p, "\t                        Audio Pid: %u\n", priv->av_play_info.a_pid);
    PROC_PRINT(p, "\t                        Video Pid: %u\n", priv->av_play_info.v_pid);

    switch (priv->av_play_info.a_type)
    {
        case HA_AUDIO_ID_DOLBY_TRUEHD:
            PROC_PRINT(p, "\t                          AUDType: AC-3\n");
            break;
        case HA_AUDIO_ID_DOLBY_PLUS:
            PROC_PRINT(p, "\t                          AUDType: EAC-3\n");
            break;
        case HA_AUDIO_ID_PCM:
            PROC_PRINT(p, "\t                          AUDType: PCM\n");
            break;
        case HA_AUDIO_ID_MP2:
            PROC_PRINT(p, "\t                          AUDType: MP2\n");
            break;
        case HA_AUDIO_ID_MP3:
            PROC_PRINT(p, "\t                          AUDType: MP3\n");
            break;
        case HA_AUDIO_ID_AAC:
            PROC_PRINT(p, "\t                          AUDType: AAC\n");
            break;
        case HA_AUDIO_ID_VORBIS:
            PROC_PRINT(p, "\t                          AUDType: VORBIS\n");
            break;
        case HA_AUDIO_ID_OGG:
            PROC_PRINT(p, "\t                          AUDType: OGG\n");
            break;
        case HA_AUDIO_ID_APE:
            PROC_PRINT(p, "\t                          AUDType: APE\n");
            break;
        default:
            break;
    }

    switch (priv->av_play_info.v_type)
    {
        case MT_UNF_VCODEC_TYPE_H261:
            PROC_PRINT(p, "\t                          VIDType: H261\n");
            break;
        case MT_UNF_VCODEC_TYPE_H264:
            PROC_PRINT(p, "\t                          VIDType: H264\n");
            break;
        case MT_UNF_VCODEC_TYPE_MPEG2:
            PROC_PRINT(p, "\t                          VIDType: MPEG2\n");
            break;
        case MT_UNF_VCODEC_TYPE_HEVC:
            PROC_PRINT(p, "\t                          VIDType: HEVC\n");
            break;
        case MT_UNF_VCODEC_TYPE_MPEG4:
            PROC_PRINT(p, "\t                          VIDType: MP4\n");
            break;
        case MT_UNF_VCODEC_TYPE_AVS:
            PROC_PRINT(p, "\t                          VIDType: AVS\n");
            break;
        case MT_UNF_VCODEC_TYPE_AVS2:
            PROC_PRINT(p, "\t                          VIDType: AVS2\n");
            break;
        case MT_UNF_VCODEC_TYPE_VC1:
            PROC_PRINT(p, "\t                          VIDType: VC1\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP3:
            PROC_PRINT(p, "\t                          VIDType: VP3\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP5:
            PROC_PRINT(p, "\t                          VIDType: VP5\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP8:
            PROC_PRINT(p, "\t                          VIDType: VP8\n");
            break;
        case MT_UNF_VCODEC_TYPE_VP9:
            PROC_PRINT(p, "\t                          VIDType: VP9\n");
            break;
        default:
            break;
    }

    switch (priv->av_play_info.tvformat)
    {
        case MT_DRV_DISP_FMT_1080i_50:
            PROC_PRINT(p, "\t                         TVformat: 1080i50\n");
            break;
        case MT_DRV_DISP_FMT_1080P_50:
            PROC_PRINT(p, "\t                         TVformat: 1080p50\n");
            break;
        case MT_DRV_DISP_FMT_1080i_60:
            PROC_PRINT(p, "\t                         TVformat: 1080i60\n");
            break;
        case MT_DRV_DISP_FMT_1080i_59_94:
            PROC_PRINT(p, "\t                         TVformat: MT_DRV_DISP_FMT_1080i_59_94\n");
            break;
        case MT_DRV_DISP_FMT_1080P_60:
            PROC_PRINT(p, "\t                         TVformat: 1080p60\n");
            break;
        case MT_DRV_DISP_FMT_1080P_59_94:
            PROC_PRINT(p, "\t                         TVformat: MT_DRV_DISP_FMT_1080P_59_94\n");
            break;
        case MT_DRV_DISP_FMT_720P_60:
            PROC_PRINT(p, "\t                         TVformat: 720p60\n");
            break;
        case MT_DRV_DISP_FMT_720P_59_94:
            PROC_PRINT(p, "\t                         TVformat: MT_DRV_DISP_FMT_720P_59_94\n");
            break;
        case MT_DRV_DISP_FMT_720P_50:
            PROC_PRINT(p, "\t                         TVformat: 720p50\n");
            break;
        case MT_DRV_DISP_FMT_PAL:
            PROC_PRINT(p, "\t                         TVformat: 576i50\n");
            break;
        case MT_DRV_DISP_FMT_576P_50:
            PROC_PRINT(p, "\t                         TVformat: 576p50\n");
            break;
        case MT_DRV_DISP_FMT_480P_60:
            PROC_PRINT(p, "\t                         TVformat: 480p60\n");
            break;
		case MT_DRV_DISP_FMT_3840X2160_60:
            PROC_PRINT(p, "\t Vformat: 3840X2160_60\n");
			break;
        case MT_DRV_DISP_FMT_3840X2160_59_94:
            PROC_PRINT(p, "\t Vformat: MT_DRV_DISP_FMT_3840X2160_59_94\n");
			break;
		case MT_DRV_DISP_FMT_3840X2160_50:
            PROC_PRINT(p, "\t TVformat: MT_DRV_DISP_FMT_3840X2160_50\n");
			break;
		case MT_DRV_DISP_FMT_4096X2160_50:
			PROC_PRINT(p, "\t TVformat: MT_DRV_DISP_FMT_4096X2160_50\n");
			break;
		case MT_DRV_DISP_FMT_4096X2160_60:
			PROC_PRINT(p, "\t TVformat: MT_DRV_DISP_FMT_4096X2160_60\n");
			break;
        case MT_DRV_DISP_FMT_4096X2160_59_94:
			PROC_PRINT(p, "\t TVformat: MT_DRV_DISP_FMT_4096X2160_60\n");
			break;

        default:
            PROC_PRINT(p, "\t                         TVformat: %u\n", priv->av_play_info.tvformat);
            break;
    }

    if (priv->do_ddp_verf)
    {
        PROC_PRINT(p, "\t                        Skip Thrd: %u\n", priv->avsync_skip_threhold/45);
        PROC_PRINT(p, "\t                       Pause Thrd: %u\n", priv->avsync_pause_threhold/45);

        switch (priv->ddp_prog)
        {
            case AVSYNC_DD_NONE_PG:
                PROC_PRINT(p, "\t                       Dolby Prog: NONE_PG\n");
                break;

            case AVSYNC_MPEG2_DD_2500_PG1:
                PROC_PRINT(p, "\t                       Dolby Prog: MPEG2_DD_2500_PG1\n");
                break;

            case AVSYNC_MPEG2_DD_2500_PG2:
                PROC_PRINT(p, "\t                       Dolby Prog: MPEG2_DD_2500_PG2\n");
                break;

            case AVSYNC_MPEG2_DDP_2500_PG1:
                PROC_PRINT(p, "\t                       Dolby Prog: MPEG2_DDP_2500_PG1\n");
                break;

            case AVSYNC_MPEG2_DDP_2500_PG2:
                PROC_PRINT(p, "\t                       Dolby Prog: MPEG2_DDP_2500_PG2\n");
                break;

            case AVSYNC_H264_DD_2500_PG1:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DD_2500_PG1\n");
                break;

            case AVSYNC_H264_DD_2500_PG2:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DD_2500_PG2\n");
                break;

            case AVSYNC_H264_DDP_2500_PG1:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_2500_PG1\n");
                break;

            case AVSYNC_H264_DDP_2500_PG2:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_2500_PG2\n");
                break;

            case AVSYNC_H264_DD_2997_PG1:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DD_2997_PG1\n");
                break;

            case AVSYNC_H264_DD_2997_PG2:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DD_2997_PG2\n");
                break;

            case AVSYNC_H264_DDP_2997_PG1:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_2997_PG1\n");
                break;

            case AVSYNC_H264_DDP_2997_PG2:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_2997_PG2\n");
                break;

            case AVSYNC_H264_DDP_2500_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_2500_ATMOS\n");
                break;

			case AVSYNC_H264_DDP_5000_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_5000_ATMOS\n");
                break;	

            case AVSYNC_H264_DDP_2997_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H264_DDP_2997_ATMOS\n");
                break;								

            case AVSYNC_MPEG2_DDP_2500_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: MPEG2_DDP_2500_ATMOS\n");
                break;

			case AVSYNC_H265_DDP_2500_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H265_DDP_2500_ATMOS\n");
                break;
			
            case AVSYNC_H265_DDP_5000_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H265_DDP_5000_ATMOS\n");
                break;

			case AVSYNC_H265_DDP_2997_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H265_DDP_2997_ATMOS\n");
                break;
			
            case AVSYNC_H265_DDP_5994_ATMOS:
                PROC_PRINT(p, "\t                       Dolby Prog: H265_DDP_5994_ATMOS\n");
                break;
				
			case AVSYNC_H264_AC4_2500_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H264_AC4_2500_ATMOS\n");
				break;

			case AVSYNC_H264_AC4_5000_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H264_AC4_5000_ATMOS\n");
				break;
			
			case AVSYNC_H264_AC4_2997_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H264_AC4_2997_ATMOS\n");
				break;

			case AVSYNC_H265_AC4_2500_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H265_AC4_2500_ATMOS\n");
				break;

			case AVSYNC_H265_AC4_5000_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H265_AC4_5000_ATMOS\n");
				break;
			
			case AVSYNC_H265_AC4_2997_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H265_AC4_2997_ATMOS\n");
				break;				
			
			case AVSYNC_H265_AC4_5994_ATMOS:
				PROC_PRINT(p, "\t                       Dolby Prog: AVSYNC_H265_AC4_5994_ATMOS\n");
				break;

            default:
                PROC_PRINT(p, "\t                       Dolby Prog: %u\n", priv->ddp_prog);
                break;
        }

        if (priv->ddp_atmos)
        {
            switch (priv->avsync_aud_output)
            {
                case AVSYNC_AUD_OUT_PCM:
                    PROC_PRINT(p, "\t                          Aud Out: PCM\n");
                    break;
                case AVSYNC_AUD_OUT_SPDIF:
                    PROC_PRINT(p, "\t                          Aud Out: SPDIF\n");
                    break;
				case AVSYNC_AUD_OUT_MAT:
                    PROC_PRINT(p, "\t                          Aud Out: MAT \n");
                    break;
                default:
                    PROC_PRINT(p, "\t                          Aud Out: UNKONWN\n");
                    break;
            }
        }
    }

    switch (priv->vsync_info.screen_mode)
    {
        case AVSYNC_UNBLANK_FAST:
            PROC_PRINT(p, "\t                 Screen Open Mode: FAST\n");
            break;
        case AVSYNC_UNBLANK_STABLE:
            PROC_PRINT(p, "\t                 Screen Open Mode: STABLE\n");
            break;
        case AVSYNC_UNBLANK_SYNC:
            PROC_PRINT(p, "\t                 Screen Open Mode: SYNC\n");
            break;
        case AVSYNC_UNBLANK_USER:
            PROC_PRINT(p, "\t                 Screen Open Mode: USER\n");
            break;
        default:
            PROC_PRINT(p, "\t                 Screen Open Mode: %d\n", priv->vsync_info.screen_mode);
            break;
    }
    PROC_PRINT(p, "\t                       pts_offset: %d \n", priv->pts_offset/45);
    PROC_PRINT(p, "\t                            SysHZ: %d - %#lx\n", HZ, mt_get_avsync_base());
    PROC_PRINT(p, "\t                         LogLevel: %#010x\n", priv->avsync_loglevel);
}

mt_s32 avsync_apts_adjust(mt_u32 adjust)
{
	avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }
	
    //range: 0 to 1000; unit:ms; bit15: 0-decrease, 1-decrease;  
    priv->audio_bypass_apts_adjust = adjust;
    
    MT_INFO_SYNC("[%s %d]Apts adjust value(%u)!\n", __FUNCTION__, __LINE__, priv->audio_bypass_apts_adjust);

	return MT_SUCCESS;
}

mt_s32 avsync_audio_do_track(mt_u32 *ptrack)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    if (!ptrack)//ptrack don't use at present, reserved for development
    {
        MT_INFO_SYNC("input para null\n");
        return MT_FAILURE;
    }

	//if switch audio when the av don't synced because of stream rollback
	//the sync mode use av mode according to rollback.
	//otherwise the sync mode use video mode according to audio track.
	if (priv->avsync_aud_rollback)
	{
		priv->avsync_do_aud_track = 0;
		MT_INFO_SYNC("[%s %d]priv->avsync_aud_rollback=%d\n", __FUNCTION__, __LINE__, priv->avsync_aud_rollback);
	}
	else
	{
		priv->avsync_do_aud_track = 1;
		priv->audio_track_set_av_mode_flag = 1;
		MT_INFO_SYNC("[%s %d]priv->avsync_do_aud_track = 1\n", __FUNCTION__, __LINE__);
    	avsync_set_sync_mode(priv, AVSYNC_REF_VIDEO_MODE);
	}
    MT_INFO_SYNC("audio track\n");

    return MT_SUCCESS;
}

mt_s32 avsync_get_avsync_info(SYNC_GET_AVSYNC_INFO_S *pavsyncinfo)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    if (!pavsyncinfo)
    {
        MT_ERR_SYNC("para null\n");
        return MT_FAILURE;
    }

    pavsyncinfo->cur_apts = priv->cur_apts;
    pavsyncinfo->cur_vpts = priv->cur_vpts;

    MT_INFO_SYNC("vpts: %#x, apts: %#x\n", pavsyncinfo->cur_vpts, pavsyncinfo->cur_apts);

    return MT_SUCCESS;
}

mt_s32 avsync_is_special_stream(avsync_priv_t *priv)
{
    if (NULL == priv) 
    {
        return MT_FAILURE;
    }

    /*
    * next judgement is just for issue20738 special test steam
    * 337 and 336 is used for issues#32308(4096x2160-4K-H264-25FPS-120Mbps-AAC.ts)
    */
    if ((((2049 == priv->av_play_info.a_pid) ||
           (2050 == priv->av_play_info.a_pid) ||
           (2051 == priv->av_play_info.a_pid) ||
           (2052 == priv->av_play_info.a_pid) ||
           (2053 == priv->av_play_info.a_pid) ||
           (2054 == priv->av_play_info.a_pid)) &&
          (1536 == priv->av_play_info.v_pid) &&
          (HA_AUDIO_ID_AAC == priv->av_play_info.a_type) &&
          (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type))
      ||((337 == priv->av_play_info.a_pid) &&
         (336 == priv->av_play_info.v_pid) &&
         (HA_AUDIO_ID_AAC == priv->av_play_info.a_type) &&
         (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)))
    {
        MT_INFO_SYNC("apid: %d, vpid: %d, atype: %#x, vtype: %#x\n",
            priv->av_play_info.a_pid,
            priv->av_play_info.v_pid, 
            priv->av_play_info.a_type,
            priv->av_play_info.v_type);

        return MT_SUCCESS;
    }

    return MT_FAILURE;
}

mt_s32 avsync_module_default_cfg(avsync_priv_t *priv)
{
    if (NULL == priv)
    {
        MT_ERR_SYNC("para null\n");
        return MT_FAILURE;
    }

    priv->init_stc = 0;
    priv->do_ddp_verf = 0;
    priv->ddp_atmos = 0;
    priv->avsync_do_aud_track = 0;
    priv->avsync_do_trick_seek = 0;
    priv->avsync_pause_threhold = 45*45;
    priv->avsync_skip_threhold = 45*45;
    //priv->avsync_aud_output = AVSYNC_AUD_OUT_PCM;
	priv->avsync_aud_output = AVSYNC_AUD_OUT_BUTT;
    priv->ddp_prog = AVSYNC_DD_NONE_PG;
    priv->avsync_cfg_info.avsync_pause_threhold = 0;
    priv->avsync_cfg_info.avsync_skip_threhold = 0;
    priv->data_source_mode = SYNC_DATA_SOURCE_TUNER;
    priv->aud_underflow_size = AVSYNC_AUD_PCM_UNDERFLOW_SIZE;
    avsync_set_sync_mode(priv, AVSYNC_REF_AV_MODE);
    return MT_SUCCESS;
}

mt_s32 avsync_play_info_cfg(mt_u32 *pargs)
{
    avsync_priv_t *priv = NULL;
    avsync_play_info_t *play_info = NULL;
    mt_s32 ret = MT_SUCCESS;

    if (NULL == pargs) 
    {
        return !MT_SUCCESS;
    }

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

	//for bug 31028/31039, if do audio track or turn on/off AD
	//don't set to default cfg.
	if (!priv->avsync_do_aud_track)	
	{
    	avsync_module_default_cfg(priv);
	}
    play_info = (avsync_play_info_t *)pargs;

    priv->av_play_info.a_pid = play_info->a_pid;
    priv->av_play_info.v_pid = play_info->v_pid;
    priv->av_play_info.a_type = play_info->a_type;
    priv->av_play_info.v_type = play_info->v_type;
    priv->av_play_info.tvformat = play_info->tvformat;
	priv->av_play_info.is_hdmi_connected = play_info->is_hdmi_connected;
	
    MT_INFO_SYNC("apid: %d, vpid: %d, atype: %#x, vtype: %#x, TVformat: %d, is_hdmi_connected: %d\n",
        priv->av_play_info.a_pid,
        priv->av_play_info.v_pid,
        priv->av_play_info.a_type,
        priv->av_play_info.v_type,
        priv->av_play_info.tvformat,
        priv->av_play_info.is_hdmi_connected);

    priv->av_play_flag = 1;
    ret = avsync_is_ddp_verfication(priv);

    if (MT_SUCCESS == ret)
    {
        priv->do_ddp_verf = 1;
    }
    else
    {
        priv->do_ddp_verf = 0;
    }
    	return MT_SUCCESS;
}

mt_s32 is_liveplay(void)
{
	if(NULL == g_p_avsync_private)
		return 0;
	
    if(g_p_avsync_private->data_source_mode == SYNC_DATA_SOURCE_TUNER)
    {
    	return 1;
    }
	else
	{
		return 0;
	}
}
EXPORT_SYMBOL(is_liveplay);

void ao_all_skip_avsync_reset(void)
{
	avsync_audio_init(0);
	avsync_audio_sync_status_reset(g_p_avsync_private);
	avsync_set_sync_mode(g_p_avsync_private, AVSYNC_REF_AV_MODE);
}
EXPORT_SYMBOL(ao_all_skip_avsync_reset);

void avsync_reset(void)
{
	avsync_audio_init(0);
	avsync_audio_sync_status_reset(g_p_avsync_private);
	//avsync_video_sync_status_reset(g_p_avsync_private);

	if (g_p_avsync_private->avsync_do_aud_track)
	{	
		avsync_set_sync_mode(g_p_avsync_private, AVSYNC_REF_VIDEO_MODE);
		//for bug 31028/31039, if do audio track or turn on/off AD
		//don't reset video status and init stc by vpts.
		avsync_stc_adjust_by_vpts(g_p_avsync_private);		
	}
	else
	{
		//if not do audio track of change AD, reset video status
		avsync_video_sync_status_reset(g_p_avsync_private);
		avsync_set_sync_mode(g_p_avsync_private, AVSYNC_REF_AV_MODE);
	}
	printk(KERN_ERR "%s %d \n", __func__, __LINE__);
}
EXPORT_SYMBOL(avsync_reset);

mt_s32 avsync_data_source_cfg(MT_SYNC_DATA_SOURCE_E *pargs)
{
    avsync_priv_t *priv = NULL;
    mt_s32 ret = MT_SUCCESS;

    if (!pargs)
    {
        MT_ERR_SYNC("pargs is null\n");
        return MT_ERR_SYNC_NULL_PTR;
    }

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return MT_FAILURE;
    }

    priv->data_source_mode = *pargs;
    
    if(priv->data_source_mode != SYNC_DATA_SOURCE_PVR)
    {
        ret = avsync_is_special_stream(priv);
        if (MT_SUCCESS == ret)
        {
            if (priv->p_avsync_psync)
            {
                priv->p_avsync_psync->SyncAttr.enSyncRef = MT_UNF_SYNC_REF_PCR;
                SYNC_reference_config(priv->p_avsync_psync);
                MT_INFO_SYNC("special AAC stream, set sync mode to pcr\n");
            }
            else
            {
                MT_ERR_SYNC("p_avsync_psync is null\n");
            }
        }
    }
     
    return MT_SUCCESS;
}

mt_s32 avsync_is_ddp_verfication(void *private)
{
    avsync_priv_t *priv = NULL;

    if (NULL == private) 
    {
        return !MT_SUCCESS;
    }

    priv = (avsync_priv_t *)private;

    MT_INFO_SYNC("avpid = %d - %d, avtype = %#x - %#x, format = %d\n",
    priv->av_play_info.a_pid,
    priv->av_play_info.v_pid,
    priv->av_play_info.a_type,
    priv->av_play_info.v_type,
    priv->av_play_info.tvformat);

    if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
        (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
        ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
        (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) ||
        (HA_AUDIO_ID_EAC3PASSTHROUGH == priv->av_play_info.a_type) || 
		(HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type) ||
		(HA_AUDIO_ID_MP3 == priv->av_play_info.a_type)) &&
        ((MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type) ||
        (MT_UNF_VCODEC_TYPE_MPEG2 == priv->av_play_info.v_type) ||
        (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type))) 
    {
        if (HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type)
        {
            MT_INFO_SYNC("ddp stream, apid:%d, vpip:%d\n",
                AVSYNC_PROG1_APID, AVSYNC_PROG1_VPID);
        }
        else
        {
            MT_INFO_SYNC("dd stream, apid:%d, vpip:%d\n",
                AVSYNC_PROG1_APID, AVSYNC_PROG1_VPID);
        }

        return MT_SUCCESS;
    }

    if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) &&
        (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
        ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
        (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type)) &&
        ((MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type) ||
        (MT_UNF_VCODEC_TYPE_MPEG2 == priv->av_play_info.v_type)))
    {
        if (HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type)
        {
            MT_INFO_SYNC("ddp stream, apid:%d, vpip:%d\n",
                AVSYNC_PROG2_APID, AVSYNC_PROG2_VPID);
        }
        else
        {
            MT_INFO_SYNC("dd stream, apid:%d, vpip:%d\n",
                AVSYNC_PROG2_APID, AVSYNC_PROG2_VPID);
        }

        return MT_SUCCESS;
    }

    return !MT_SUCCESS;
}

mt_s32 avsync_is_ddp_atmos(void *private, MT_HA_AUDIO_STREAM_INFO_S *info)
{
    avsync_priv_t *priv = NULL;

    if (NULL == private)
    {
        return !MT_SUCCESS;
    }

    priv = (avsync_priv_t *)private;

    /*
    for dolby TB-44 verfification, avsync need the flow info to judge wether the
    stream is TB-44 verfification stream.
    */
    if ((7 == info->dolby_acmod) &&
        (16 == info->dolby_bsid) &&
        (1 == info->dolby_lfeon))
    {
        switch (priv->ddp_prog)
        {
            case AVSYNC_H264_DDP_2500_PG1:
                priv->ddp_prog = AVSYNC_H264_DDP_2500_ATMOS;
                priv->ddp_atmos = 1;

                if (MT_DRV_DISP_FMT_1080P_50 != priv->av_play_info.tvformat)
                {
                    MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p50 is needed\n",
                         priv->av_play_info.tvformat);
                }

            break;

            case AVSYNC_H264_DDP_2997_PG1:
                priv->ddp_prog = AVSYNC_H264_DDP_2997_ATMOS;
                priv->ddp_atmos = 1;

                if (MT_DRV_DISP_FMT_1080P_60 != priv->av_play_info.tvformat)
                {
                    MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p60 is needed\n",
                        priv->av_play_info.tvformat);
                }

            break;

            case AVSYNC_MPEG2_DDP_2500_PG1:
                priv->ddp_prog = AVSYNC_MPEG2_DDP_2500_ATMOS;
                priv->ddp_atmos = 1;
                //priv->avsync_aud_output = AVSYNC_AUD_OUT_PCM;

                if (MT_DRV_DISP_FMT_1080P_50 != priv->av_play_info.tvformat)
                {
                    MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p50 is needed\n",
                        priv->av_play_info.tvformat);
                }

            break;

			case AVSYNC_H265_DDP_2500_ATMOS:
				priv->ddp_atmos = 1;

                if (MT_DRV_DISP_FMT_1080P_50 != priv->av_play_info.tvformat)
                {
                    MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p50 is needed\n",
                        priv->av_play_info.tvformat);
                }
				
				break;
			
			case AVSYNC_H265_DDP_2997_ATMOS:
				priv->ddp_atmos = 1;

				if (MT_DRV_DISP_FMT_1080P_60 != priv->av_play_info.tvformat)
				{
					MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p50 is needed\n",
						priv->av_play_info.tvformat);
				}
			
				break;
							
            case AVSYNC_H265_DDP_5000_ATMOS:
                priv->ddp_atmos = 1;
                //priv->avsync_aud_output = AVSYNC_AUD_OUT_PCM;

                if (MT_DRV_DISP_FMT_1080P_50 != priv->av_play_info.tvformat)
                {
                    MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p50 is needed\n",
                        priv->av_play_info.tvformat);
                }

            break;

            case AVSYNC_H265_DDP_5994_ATMOS:
                priv->ddp_atmos = 1;
                //priv->avsync_aud_output = AVSYNC_AUD_OUT_PCM;

                if (MT_DRV_DISP_FMT_1080P_60 != priv->av_play_info.tvformat)
                {
                    MT_ALWAYS_PRINT("tvfmt: %d is wrong, 1080p60 is needed\n",
                        priv->av_play_info.tvformat);
                }

            break;

            default:
                priv->ddp_atmos = 0;
            break;
        }

        if (priv->ddp_atmos)
        {
            MT_INFO_SYNC("%s, %d, ATMOS stream, %d\n", __func__, __LINE__, priv->ddp_prog);
            return MT_SUCCESS;
        }
        else
        {
            MT_INFO_SYNC("%s, %d, not ATMOS stream, %d\n", __func__, __LINE__);
            return !MT_SUCCESS;
        }
    }
    else
    {
		if(HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)
		{		
			priv->ddp_atmos = 1;
			return MT_SUCCESS;
		}
		else
		{
	        priv->ddp_atmos = 0;
	        return !MT_SUCCESS;
		}
    }
}

mt_s32 avsync_get_ddp_prog(avsync_priv_t *priv)
{
    if (NULL == priv) 
    {
        return !MT_SUCCESS;
    }

    if (0 == priv->vpts_step)
    {
        return MT_SUCCESS;
    }

    /*
    if the steam is 29.97 frame stream, the step is between 33ms and 34ms
    */
    if ((AVSYNC_DDP2997_VID_STEP0 < priv->vpts_step) &&
        (AVSYNC_DDP2997_VID_STEP1 > priv->vpts_step))
    {
        if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
            priv->ddp_prog = AVSYNC_H264_DD_2997_PG1;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type))
        {
             avsync_set_display_for_dolby_stream(priv);
             priv->ddp_prog = AVSYNC_H264_DD_2997_PG2;
             MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
            (HA_AUDIO_ID_EAC3PASSTHROUGH == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
			if (1 == priv->async_info.dolby_type)
			{
				priv->ddp_prog = AVSYNC_H264_DD_2997_PG1;
			}
			else
			{
				priv->ddp_prog = AVSYNC_H264_DDP_2997_PG1;
			}

            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) && 
            (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type))
        {
            avsync_set_display_for_dolby_stream(priv);
 			if (1 == priv->async_info.dolby_type)
			{
				priv->ddp_prog = AVSYNC_H264_DD_2997_PG2;
			}
			else
			{
				priv->ddp_prog = AVSYNC_H264_DDP_2997_PG2;
			}
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
		else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type ) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
			priv->ddp_prog = AVSYNC_H265_DDP_2997_ATMOS;

            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type ) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
			priv->ddp_prog = AVSYNC_H264_AC4_2997_ATMOS;

            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
		else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
		        (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
		        (HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type ) &&
		        (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
			priv->ddp_prog = AVSYNC_H265_AC4_2997_ATMOS;

            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }

        return MT_SUCCESS;
    } 
    else if (AVSYNC_DDP2500_VID_STEP == priv->vpts_step)
    {
        if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H264_DD_2500_PG1 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }

            priv->ddp_prog = AVSYNC_H264_DD_2500_PG1;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type))
        {
            if (AVSYNC_H264_DD_2500_PG2 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }

            priv->ddp_prog = AVSYNC_H264_DD_2500_PG2;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        } 
        else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
            (HA_AUDIO_ID_EAC3PASSTHROUGH == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H264_DDP_2500_PG1 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }
			
			if (1 == priv->async_info.dolby_type)
			{
				priv->ddp_prog = AVSYNC_H264_DD_2500_PG1;
			}
			else
			{
				priv->ddp_prog = AVSYNC_H264_DDP_2500_PG1;
			}
			
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) && 
            (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type))
        {
            if (AVSYNC_H264_DDP_2500_PG2 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }

			if (1 == priv->async_info.dolby_type)
			{
				priv->ddp_prog = AVSYNC_H264_DD_2500_PG2;
			}
			else
			{
				priv->ddp_prog = AVSYNC_H264_DDP_2500_PG2;
			}
			
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_MPEG2 == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_MPEG2_DD_2500_PG1 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }

            priv->ddp_prog = AVSYNC_MPEG2_DD_2500_PG1;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_TRUEHD == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_MPEG2 == priv->av_play_info.v_type))
        {
            if (AVSYNC_MPEG2_DD_2500_PG2 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }

            priv->ddp_prog = AVSYNC_MPEG2_DD_2500_PG2;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        } 
        else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
            (HA_AUDIO_ID_EAC3PASSTHROUGH == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_MPEG2 == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_MPEG2_DDP_2500_PG1 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }
			
			if (1 == priv->async_info.dolby_type)
			{
				priv->ddp_prog = AVSYNC_MPEG2_DD_2500_PG1;
			}
			else
			{
				priv->ddp_prog = AVSYNC_MPEG2_DDP_2500_PG1;
			}

            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG2_APID == priv->av_play_info.a_pid) && 
            (AVSYNC_PROG2_VPID == priv->av_play_info.v_pid) &&
            (HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) &&
            (MT_UNF_VCODEC_TYPE_MPEG2 == priv->av_play_info.v_type))
        {
            if (AVSYNC_MPEG2_DDP_2500_PG2 == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }
			if (1 == priv->async_info.dolby_type)
			{
				priv->ddp_prog = AVSYNC_MPEG2_DD_2500_PG2;
			}
			else
			{
				priv->ddp_prog = AVSYNC_MPEG2_DDP_2500_PG2;
			}

            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
        else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type) ) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H264_AC4_2500_ATMOS == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }
			priv->ddp_prog = AVSYNC_H264_AC4_2500_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
		else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type) ) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H265_AC4_2500_ATMOS == priv->ddp_prog)
            {
            	MT_INFO_SYNC("[%s %d]priv->ddp_prog=%d\n", __FUNCTION__, __LINE__, priv->ddp_prog);
                return MT_SUCCESS;
            }
			priv->ddp_prog = AVSYNC_H265_AC4_2500_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
		else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H265_DDP_2500_ATMOS == priv->ddp_prog)
            {
            	MT_INFO_SYNC("[%s %d]priv->ddp_prog=%d\n", __FUNCTION__, __LINE__, priv->ddp_prog);
                return MT_SUCCESS;
            }
			priv->ddp_prog = AVSYNC_H265_DDP_2500_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }		
    }
    else if (AVSYNC_DDP5000_VID_STEP == priv->vpts_step)
    {
        if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
            (HA_AUDIO_ID_EAC3PASSTHROUGH == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H265_DDP_5000_ATMOS == priv->ddp_prog)
            {
                return MT_SUCCESS;
            }

            priv->ddp_prog = AVSYNC_H265_DDP_5000_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
		else if((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type))
		{
				if (AVSYNC_H265_AC4_5000_ATMOS == priv->ddp_prog)
				{
					return MT_SUCCESS;
				}
				priv->ddp_prog = AVSYNC_H265_AC4_5000_ATMOS;
		}
		else if((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type))
		{
				if (AVSYNC_H264_AC4_5000_ATMOS == priv->ddp_prog)
				{
					MT_INFO_SYNC("[%s %d]priv->ddp_prog=%d\n", __FUNCTION__, __LINE__, priv->ddp_prog);
					return MT_SUCCESS;
				}
				priv->ddp_prog = AVSYNC_H264_AC4_5000_ATMOS;
				MT_INFO_SYNC("[%s %d]priv->ddp_prog=%d\n", __FUNCTION__, __LINE__, priv->ddp_prog);
		}	
		else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ) &&
            (MT_UNF_VCODEC_TYPE_H264 == priv->av_play_info.v_type)) 
        {
            if (AVSYNC_H264_DDP_5000_ATMOS == priv->ddp_prog)
            {
            	MT_INFO_SYNC("[%s %d]priv->ddp_prog=%d\n", __FUNCTION__, __LINE__, priv->ddp_prog);
                return MT_SUCCESS;
            }
			priv->ddp_prog = AVSYNC_H264_DDP_5000_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
			MT_INFO_SYNC("[%s %d]priv->ddp_prog=%d\n", __FUNCTION__, __LINE__, priv->ddp_prog);
        }	
    }
    /*
    if the steam is 59.94 frame stream, the step is between 16ms and 17ms
    */
    else if ((AVSYNC_DDP5994_VID_STEP0 < priv->vpts_step) &&
        (AVSYNC_DDP5994_VID_STEP1 > priv->vpts_step))
    {
        if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_PLUS == priv->av_play_info.a_type) ||
            (HA_AUDIO_ID_EAC3PASSTHROUGH == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
            priv->ddp_prog = AVSYNC_H265_DDP_5994_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
		else if ((AVSYNC_PROG1_APID == priv->av_play_info.a_pid) &&
            (AVSYNC_PROG1_VPID == priv->av_play_info.v_pid) &&
            ((HA_AUDIO_ID_DOLBY_AC4 == priv->av_play_info.a_type)) &&
            (MT_UNF_VCODEC_TYPE_HEVC == priv->av_play_info.v_type)) 
        {
            avsync_set_display_for_dolby_stream(priv);
            priv->ddp_prog = AVSYNC_H265_AC4_5994_ATMOS;
            MT_INFO_SYNC("Dolby PROG = %d\n", priv->ddp_prog);
        }
    }
    else
    {
        /*
        other dolby stream
        */
    }

    return MT_SUCCESS;
}

void avsync_trick_seek_to_normal(void)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return;
    }

    priv->avsync_do_trick_seek = 1;
    avsync_set_sync_mode(priv, AVSYNC_REF_VIDEO_MODE);
}
EXPORT_SYMBOL(avsync_trick_seek_to_normal);

void avsync_fill_avsync_ci_test_info(MT_UNF_AVPLAY_CI_TEST_INFO_S* pstInfo)
{
    avsync_priv_t *priv = NULL;

    priv = g_p_avsync_private;

    if (NULL == priv)
    {
        MT_ERR_SYNC("priv null\n");
        return;
    }

    if (NULL != pstInfo)
    {
        if ((priv->vid_play_cnt + priv->vid_free_cnt) >= 1)
        {
            pstInfo->VFirstFrmShowed = 1;
        }
        else
        {
            pstInfo->VFirstFrmShowed = 0;
        }

        pstInfo->VDropFrmCnt = priv->vid_skip_cnt;
        pstInfo->VWrite2DisplayIdx = priv->vid_play_cnt + priv->vid_free_cnt + priv->vid_skip_cnt;
        pstInfo->AUnderrunCnt = priv->aud_underflow_cnt;
        pstInfo->SyncMode = priv->avsync_cfg_info.avsync_sync_mode;
        pstInfo->SyncFlag = priv->av_synced_flag;
        pstInfo->SyncVPts = priv->cur_vpts;
        pstInfo->SyncAPts = priv->isr_pop.pts;
        pstInfo->SyncAVptsDiff = abs(priv->cur_vpts - priv->isr_pop.pts) / 45;
        pstInfo->SyncTotalVFrmPlayCnt = priv->vid_play_cnt + priv->vid_free_cnt;
        pstInfo->SyncTotalVFrmHoldCnt = priv->vid_pause_cnt;
        pstInfo->SyncTotalVFrmDropCnt = priv->vid_skip_cnt;
	}
}
EXPORT_SYMBOL(avsync_fill_avsync_ci_test_info);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

