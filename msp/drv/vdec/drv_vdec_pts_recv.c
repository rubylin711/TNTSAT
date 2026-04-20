/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************* Include Files *******************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include "drv_vdec_pts_recv.h"
#include "mt_drv_vdec.h"
#include "drv_vdec_private.h"
#include "drv_vdec_alg.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

#define PTS_REC_MAX_NUM         (MT_VDEC_MAX_INSTANCE_NEW)
#ifdef JQW
#define PTS_REC_MAX_FRAMERATE   (60)           /* fps*/
#else
#define PTS_REC_MAX_FRAMERATE   (120)           /* fps*/
#endif
#define PTS_MAX_VALUE           (0x5B05B05)    /* ms */
#define PTS_MAX_PASSTIME        (120 * 1000)   /* ms */
#define FRMTIME_SERIES_CNT      (2)
#define FRMTIME_MAX_JUMP        (1000)

#define MT_FATAL_PTSREC         MT_FATAL_VDEC
#define MT_ERR_PTSREC           MT_ERR_VDEC
#define MT_WARN_PTSREC          MT_WARN_VDEC
#define MT_INFO_PTSREC          MT_INFO_VDEC

/*************************** Structure Definition ****************************/

typedef enum tagPTSREC_CHAN_STATUS_E
{
    PTSREC_CHAN_INITED = 0,
    PTSREC_CHAN_STOP  = 1,
    PTSREC_CHAN_START = 2,
    PTSREC_CHAN_BUTT
} PTSREC_CHAN_STATUS_E;

typedef struct tagPTSREC_CHAN_INFO_S
{
    PTSREC_CHAN_STATUS_E enStatus;          /* Status */
    mt_u32               u32LastSrcPts;     /* Last valid src pts */
    mt_u32               u32LastSrcFrmIdx;  /* The index of last frame with a valid src pts */
    mt_u32               u32FrameNum;       /* Total frame number */
    mt_u32               u32FrameTimeCnt;   /* Frame interval counter */
    mt_u32               u32LastCalcFrmTime;/* Last calculated frame time(us) */
    mt_u32               u32CalcFrameTime;  /* Calculated frame time(us) */
    mt_u32               u32SetFrameTime;   /* Setted frame time(us) */
    mt_u32               u32GesFrameTime;   /* Guessed frame time(us) */
    mt_s32               s32InterPtsDelta; /*interleaved source, VPSS module swtich field to frame, need to adjust pts*/
    MT_UNF_AVPLAY_FRMRATE_TYPE_E enFrmRateType; /* Frame rate type */
    OPTM_ALG_FRD_S       stPtsInfo;
} PTSREC_CHAN_INFO_S;

/***************************** Global Definition *****************************/
/***************************** Static Definition *****************************/

static PTSREC_CHAN_INFO_S s_stPtsRecChan[MT_VDEC_MAX_INSTANCE_NEW];
static mt_void PTSREC_FrameRateDetect(mt_handle hHandle, IMAGE *pstImage);

/*********************************** Code ************************************/

static mt_void PTSREC_InitParam(mt_handle hHandle)
{
    s_stPtsRecChan[hHandle].u32LastSrcPts = MT_INVALID_PTS;
    s_stPtsRecChan[hHandle].u32LastSrcFrmIdx = -1;
    s_stPtsRecChan[hHandle].u32FrameNum = 0;
    s_stPtsRecChan[hHandle].u32FrameTimeCnt = 0;
    s_stPtsRecChan[hHandle].u32LastCalcFrmTime = MT_INVALID_TIME;
    s_stPtsRecChan[hHandle].u32CalcFrameTime = MT_INVALID_TIME;
    s_stPtsRecChan[hHandle].u32GesFrameTime = MT_INVALID_TIME;
    s_stPtsRecChan[hHandle].s32InterPtsDelta = 0;
    s_stPtsRecChan[hHandle].stPtsInfo.u320_Pts = 0xffffffff;
    s_stPtsRecChan[hHandle].stPtsInfo.u32120_Pts = 0xffffffff;
    s_stPtsRecChan[hHandle].stPtsInfo.u32QueCnt = 0;
    s_stPtsRecChan[hHandle].stPtsInfo.u32QueStable = 0;
}

mt_s32 PTSREC_Init(mt_void)
{
    mt_s32 i;

    for (i = 0; i < PTS_REC_MAX_NUM; i++)
    {
        s_stPtsRecChan[i].enStatus = PTSREC_CHAN_INITED;
        s_stPtsRecChan[i].u32SetFrameTime = MT_INVALID_TIME;
        PTSREC_InitParam(i);
        OPTM_ALG_FrdInfo_Reset(&s_stPtsRecChan[i].stPtsInfo, 0);
        s_stPtsRecChan[i].stPtsInfo.u320_Pts = 0xffffffff;
        s_stPtsRecChan[i].stPtsInfo.u32120_Pts = 0xffffffff;
        s_stPtsRecChan[i].stPtsInfo.u32QueCnt = 0;
        s_stPtsRecChan[i].stPtsInfo.u32QueStable = 0;
        s_stPtsRecChan[i].enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
    }

    return MT_SUCCESS;
}

mt_s32 PTSREC_DeInit(mt_void)
{
    mt_s32 i;

    for (i = 0; i < PTS_REC_MAX_NUM; i++)
    {
        if (s_stPtsRecChan[i].enStatus > PTSREC_CHAN_INITED)
        {
            PTSREC_Free(i);
            MT_WARN_PTSREC("Free %d in deinit!\n", i);
            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

mt_s32 PTSREC_Alloc(mt_handle hHandle)
{
    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].enStatus > PTSREC_CHAN_INITED)
    {
        MT_ERR_PTSREC("Already alloc:%d\n", hHandle);
        return MT_FAILURE;
    }

    s_stPtsRecChan[hHandle].enStatus = PTSREC_CHAN_STOP;
    s_stPtsRecChan[hHandle].u32SetFrameTime = MT_INVALID_TIME;
    PTSREC_InitParam(hHandle);
    OPTM_ALG_FrdInfo_Reset(&s_stPtsRecChan[hHandle].stPtsInfo, 0);
    return MT_SUCCESS;
}

mt_s32 PTSREC_Free(mt_handle hHandle)
{
    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    s_stPtsRecChan[hHandle].enStatus = PTSREC_CHAN_INITED;
    return MT_SUCCESS;
}

mt_s32 PTSREC_Start(mt_handle hHandle)
{
    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].enStatus < PTSREC_CHAN_STOP)
    {
        MT_ERR_PTSREC("Alloc first:%d\n", hHandle);
        return MT_FAILURE;
    }

    s_stPtsRecChan[hHandle].enStatus = PTSREC_CHAN_START;
    PTSREC_InitParam(hHandle);
    return MT_SUCCESS;
}

mt_s32 PTSREC_Stop(mt_handle hHandle)
{
    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].enStatus < PTSREC_CHAN_STOP)
    {
        MT_ERR_PTSREC("Alloc first:%d\n", hHandle);
        return MT_FAILURE;
    }

    s_stPtsRecChan[hHandle].enStatus = PTSREC_CHAN_STOP;
    return MT_SUCCESS;
}

mt_s32 PTSREC_Reset(mt_handle hHandle)
{
    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].enStatus < PTSREC_CHAN_STOP)
    {
        MT_ERR_PTSREC("Alloc first:%d\n", hHandle);
        return MT_FAILURE;
    }

	/* 20171226: Start state still could reset */
/*    if (s_stPtsRecChan[hHandle].enStatus > PTSREC_CHAN_STOP)
    {
        MT_ERR_PTSREC("Stop first:%d\n", hHandle);
        return MT_FAILURE;
    }*/

    PTSREC_InitParam(hHandle);
    OPTM_ALG_FrdInfo_Reset(&s_stPtsRecChan[hHandle].stPtsInfo, 0);
    return MT_SUCCESS;
}

mt_s32 PTSREC_SetFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_u32 u32DivNum;

    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].enStatus < PTSREC_CHAN_STOP)
    {
        MT_ERR_PTSREC("Alloc first:%d\n", hHandle);
        return MT_FAILURE;
    }

    s_stPtsRecChan[hHandle].enFrmRateType = pstFrmRate->enFrmRateType;

    if ((MT_UNF_AVPLAY_FRMRATE_TYPE_USER == s_stPtsRecChan[hHandle].enFrmRateType)
        || (MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS == s_stPtsRecChan[hHandle].enFrmRateType))
    {
        // 1
        u32DivNum = pstFrmRate->stSetFrmRate.u32fpsInteger* 1000 + pstFrmRate->stSetFrmRate.u32fpsDecimal;

#if 0
        if((u32DivNum > PTS_REC_MAX_FRAMERATE * 1000) || (0 == u32DivNum))
        {
            MT_ERR_PTSREC("invalid frame rate, Integer(%d), Decimal(%d) \n", pstFrmRate->stSetFrmRate.u32fpsInteger, pstFrmRate->stSetFrmRate.u32fpsDecimal);
            return MT_FAILURE;
        }
#endif
        /*correct to us*/
        /*CNcomment: 保留小数点后三位 */
        s_stPtsRecChan[hHandle].u32SetFrameTime = 1000000000 / u32DivNum;
        MT_INFO_PTSREC("set chan(%d) frm diff(%d)\n", hHandle,  s_stPtsRecChan[hHandle].u32SetFrameTime);
    }
    else
    {
        s_stPtsRecChan[hHandle].u32SetFrameTime = -1;
    }

    return MT_SUCCESS;
}

mt_s32 PTSREC_GetFrmRate(mt_handle hHandle, MT_UNF_AVPLAY_FRMRATE_PARAM_S *pstFrmRate)
{
    mt_u32 u32Val;

    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].enStatus < PTSREC_CHAN_STOP)
    {
        MT_ERR_PTSREC("Alloc first:%d\n", hHandle);
        return MT_FAILURE;
    }

    if (s_stPtsRecChan[hHandle].u32SetFrameTime != MT_INVALID_TIME)
    {
        u32Val = 1000000000 / s_stPtsRecChan[hHandle].u32SetFrameTime;
        pstFrmRate->stSetFrmRate.u32fpsInteger = u32Val / 1000;
        pstFrmRate->stSetFrmRate.u32fpsDecimal = u32Val % 1000;
    }
    else
    {
        MT_ERR_PTSREC("Didn't set\n");
        pstFrmRate->stSetFrmRate.u32fpsInteger = 0;
        pstFrmRate->stSetFrmRate.u32fpsDecimal = 0;
    }

    return MT_SUCCESS;
}

mt_u32 PTSREC_GetInterPtsDelta(mt_handle hHandle)
{
    return s_stPtsRecChan[hHandle].s32InterPtsDelta;
}
/*
 * For MT_UNF_AVPLAY_FRMRATE_TYPE_PTS:      Recover PTS and detect frame rate;
 * For MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM:   Only Recover PTS, use frame rate from stream;
 * For MT_UNF_AVPLAY_FRMRATE_TYPE_USER:     Only Recover PTS, use user set frame rate;
 * For MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS: Recover PTS and detect frame rate;
 */
mt_u32 PTSREC_CalcStamp(mt_handle hHandle, MT_UNF_VCODEC_TYPE_E enVdecType, IMAGE *pstImage)
{
    mt_u32 u32NumPass = 0;
    mt_u32 u32PtsPass = 0;
    mt_u32 u32FrameTime = MT_INVALID_TIME;
    mt_u32 u32SrcPts, u32Pts;
    PTSREC_CHAN_INFO_S *pstPtsInfo = NULL;

    if (hHandle >= PTS_REC_MAX_NUM)
    {
        MT_ERR_PTSREC("Bad handle:%d\n", hHandle);
        return MT_FAILURE;
    }

    pstPtsInfo = &s_stPtsRecChan[hHandle];

    if (pstPtsInfo->enStatus < PTSREC_CHAN_START)
    {
        MT_ERR_PTSREC("Start first:%d\n", hHandle);
        return MT_FAILURE;
    }

    /* Temprary usage */
    pstPtsInfo->u32FrameNum++;
    pstImage->frame_idx = pstPtsInfo->u32FrameNum;

    /* TOADD */
#if 0
    /* If there were some frames discard before this frame,calculate again */
    if (pstImage->frame_num == 0)
    {
        pstPtsInfo->u32LastSrcPts = -1;
    }
#endif


    /*calculate how many frames between two frames with valid src pts, ingore the loopback situation*/
    /*CNcomment: 计算两个有效PTS帧之间的帧数 无符号减法不考虑环回的情况 0 - 255 = 1*/
    if (MT_INVALID_PTS != pstPtsInfo->u32LastSrcPts)
    {
        u32NumPass = pstImage->frame_idx - pstPtsInfo->u32LastSrcFrmIdx;
    }

    u32SrcPts = (mt_u32)pstImage->SrcPts;
    u32Pts = (mt_u32)pstImage->PTS;

    /* u32SrcPts is valid */
    if (MT_INVALID_PTS != u32SrcPts)
    {
        /* calculate StatPtsDelta based on pts delta and frame numbers between two valid src pts */
        /* CNcomment: 根据两个有效PTS间的PTS差值和帧数计算StatPtsDelta */
        if (MT_INVALID_PTS != pstPtsInfo->u32LastSrcPts)
        {
            if (u32SrcPts >= pstPtsInfo->u32LastSrcPts)
            {
                u32PtsPass = u32SrcPts - pstPtsInfo->u32LastSrcPts;
            }
            /* pts loopback */
            /* TODO:how to distinguish pts back and loopback */
            else if (u32SrcPts < pstPtsInfo->u32LastSrcPts)
            {
                MT_INFO_PTSREC("Pts rewind\n");
                u32PtsPass = PTS_MAX_VALUE - pstPtsInfo->u32LastSrcPts + u32SrcPts;
            }

            /*calculate the frame time*/
            if (u32NumPass)
            {
                if (u32PtsPass <= PTS_MAX_PASSTIME)
                {
                    /*only successive twice u32FrameTime is the same, then update u32CalcFrameTime*/
                    /**CNcomment: 只有连续2次的FrameTime计算相同才会更新CalcFrameTime*/
                    if (MT_INVALID_TIME != pstPtsInfo->u32CalcFrameTime)
                    {
                        if (abs(u32PtsPass * 1000 / u32NumPass - pstPtsInfo->u32LastCalcFrmTime) > FRMTIME_MAX_JUMP)
                        {
                            pstPtsInfo->u32FrameTimeCnt = 0;
                        }

                        pstPtsInfo->u32FrameTimeCnt++;
                        if (pstPtsInfo->u32FrameTimeCnt >= FRMTIME_SERIES_CNT)
                        {
                            pstPtsInfo->u32CalcFrameTime = u32PtsPass * 1000 / u32NumPass;
                            pstPtsInfo->u32FrameTimeCnt = FRMTIME_SERIES_CNT;
                        }
                    }
                    else
                    {
                        pstPtsInfo->u32CalcFrameTime = u32PtsPass * 1000 / u32NumPass;
                        pstPtsInfo->u32FrameTimeCnt++;
                    }

                    pstPtsInfo->u32LastCalcFrmTime = u32PtsPass * 1000 / u32NumPass;

                    s_stPtsRecChan[hHandle].s32InterPtsDelta = pstPtsInfo->u32CalcFrameTime /2000;
                }
                else
                {
                    /*keep the u32CalcFrameTime while pts is abnormal*/
                    MT_WARN_PTSREC("pts %d exception\n", u32SrcPts);
                    if (u32SrcPts >= pstPtsInfo->u32LastSrcPts) /* pts jump*/
                    {
                        s_stPtsRecChan[hHandle].s32InterPtsDelta = 20;
                    }
                    else
                    {
                        s_stPtsRecChan[hHandle].s32InterPtsDelta = -20; /* pts back and loopback  */
                    }
                }
            }
            else
            {
                MT_WARN_PTSREC("Frame index %d repeat\n", pstImage->frame_idx);
                s_stPtsRecChan[hHandle].s32InterPtsDelta = 20;
            }
        }
        else
        {
            MT_INFO_PTSREC("Got first valid source pts:%d\n", u32SrcPts);
            s_stPtsRecChan[hHandle].s32InterPtsDelta = 20;
        }

        pstPtsInfo->u32LastSrcPts = u32SrcPts;
        pstPtsInfo->u32LastSrcFrmIdx = pstImage->frame_idx;
        u32Pts = u32SrcPts;
    }
    /* u32SrcPts is invalid */
    else
    {
        if (MT_INVALID_PTS != pstPtsInfo->u32LastSrcPts)
        {
#if 0
            if (((MT_UNF_VCODEC_TYPE_REAL8 == enVdecType)
                 || (MT_UNF_VCODEC_TYPE_REAL9 == enVdecType))
                && (-1 != pstPtsInfo->u32SetFrameTime))
            {
                u32Pts = pstPtsInfo->u32LastSrcPts + pstPtsInfo->u32SetFrameTime / 1000;
                pstPtsInfo->u32LastSrcPts = u32Pts;
                pstImage->PTS = (mt_u64)u32Pts;
                return u32Pts;
            }
#endif

            if (MT_UNF_AVPLAY_FRMRATE_TYPE_USER == pstPtsInfo->enFrmRateType)
            {
                /* Choose u32FrameTime from: 1.SetFrameTime 2.CalcFrameTime 3.GuessFrameTime*/
                if (MT_INVALID_TIME != pstPtsInfo->u32SetFrameTime)
                {
                    u32FrameTime = pstPtsInfo->u32SetFrameTime;
                }
                else if (MT_INVALID_TIME != pstPtsInfo->u32CalcFrameTime)
                {
                    u32FrameTime = pstPtsInfo->u32CalcFrameTime;
                }
                else
                {
                    if (MT_INVALID_TIME == pstPtsInfo->u32GesFrameTime)
                    {
                        if (pstImage->image_width <= 720 && pstImage->image_height <= 480)
                        {
                            pstPtsInfo->u32GesFrameTime = 33333;  /* MAYBE NTSC */
                        }
/* make tsscan happy */
#if 0
                        else if (pstImage->image_width <= 720 && pstImage->image_height <= 576)
                        {
                            pstPtsInfo->u32GesFrameTime = 40000;  /* MAYBE PAL */
                        }
#endif
                        else
                        {
                            pstPtsInfo->u32GesFrameTime = 40000;
                        }
                    }

                    u32FrameTime = pstPtsInfo->u32GesFrameTime;
                }
            }
            else /* MT_UNF_AVPLAY_FRMRATE_TYPE_USER_PTS ... */
            {
                /* Choose u32FrameTime from: 1.u32CalcFrameTime 2.u32SetFrameTime 3.GuessFrameTime*/
                if (MT_INVALID_TIME != pstPtsInfo->u32CalcFrameTime)
                {
                    u32FrameTime = pstPtsInfo->u32CalcFrameTime;
                }
                else if (MT_INVALID_TIME != pstPtsInfo->u32SetFrameTime)
                {
                    u32FrameTime = pstPtsInfo->u32SetFrameTime;
                }
                else
                {
                    if (MT_INVALID_TIME == pstPtsInfo->u32GesFrameTime)
                    {
                        if ((pstImage->image_width <= 720) && (pstImage->image_height <= 480))
                        {
                            pstPtsInfo->u32GesFrameTime = 33333;  /* MAYBE NTSC */
                        }
/* make tsscan happy */
#if 0
                        else if ((pstImage->image_width <= 720) && (pstImage->image_height <= 576))
                        {
                            pstPtsInfo->u32GesFrameTime = 40000;  /* MAYBE PAL */
                        }
                        else
#endif
                        {
                            pstPtsInfo->u32GesFrameTime = 40000;
                        }
                    }

                    u32FrameTime = pstPtsInfo->u32GesFrameTime;
                }
            }

            u32Pts = pstPtsInfo->u32LastSrcPts + (u32FrameTime * u32NumPass) / 1000;
		    s_stPtsRecChan[hHandle].s32InterPtsDelta = u32FrameTime/2000;
        }
        else //if (MT_INVALID_PTS != pstPtsInfo->u32LastSrcPts)
        {
            u32Pts = MT_INVALID_PTS;
            s_stPtsRecChan[hHandle].s32InterPtsDelta = 0;
        }
    }
    #if 0
    if (0 == (pstImage->format & 0x300))/*PROGRESSIVE*/
    {
		s_stPtsRecChan[hHandle].s32InterPtsDelta = 0;
    }
	#endif

    pstImage->PTS = (mt_u64)u32Pts;
    PTSREC_FrameRateDetect(hHandle, pstImage);
    return u32Pts;
}

static mt_void PTSREC_FrameRateDetect(mt_handle hHandle, IMAGE *pstImage)
{
    mt_u32 u32FrameRate;

    /*
     * For RWZB test, VO will use default frame rate adapted to output norm.
     * If frame type was set to STREAM type, need not detect, use stream frame rate.
     * If frame type was set to USER type, need not detect, use the user set value.
     */
    if (!((pstImage->optm_inf.Rwzb > 0) ||
          (MT_UNF_AVPLAY_FRMRATE_TYPE_STREAM == s_stPtsRecChan[hHandle].enFrmRateType) ||
          (MT_UNF_AVPLAY_FRMRATE_TYPE_USER == s_stPtsRecChan[hHandle].enFrmRateType)))
    {
        u32FrameRate = OPTM_ALG_FrameRateDetect(
                                &(s_stPtsRecChan[hHandle].stPtsInfo),
                                (mt_u32)pstImage->PTS);
        pstImage->frame_rate = u32FrameRate * 1024 / 10;
    }
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
