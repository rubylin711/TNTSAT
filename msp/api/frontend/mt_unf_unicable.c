/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "mt_module_debug.h"
#include "mt_type.h"
#include "mt_common.h"
#include "mt_mpi_mem.h"
#include "mt_unf_frontend.h"
#include "drv_frontend_ioctl.h"
#include "mt_drv_struct.h"

//#define UNICABLE_UB_OCCUPIED_CHECK

/** BEGIN: Added by Howie, 2011/1/5  修改原因: 增加全频点扫描UB功能 */

/** Step size for full band scan. */
#define SCR_D_SCAN_FREQ_STEP (4)

#define SCR_D_SCAN_FREQ_STEP_MIN (2)

#define SCR_USER_BAND_NUMBERS (8)

#define SCR_D_MAX_USER_BAND (8)

/** Start frequency for full band scan. */
#define SCR_D_SCAN_FREQ_START (950)

/** End frequency for full band scan. */
#define SCR_D_SCAN_FREQ_END (2150)

/** Scan points for full band scan. */
#define SCR_D_SCAN_POINTS (((SCR_D_SCAN_FREQ_END) - (SCR_D_SCAN_FREQ_START)) / (SCR_D_SCAN_FREQ_STEP))

#define THRESHOLD_TOLERANT_AGC_DISTANCE 70

#define MIN_DISTANCE_BETWEEN_UB_MHZ 80

/** Max number of points maybe UBs after scan full band. */
#define SCR_D_MAX_POINTS_MAYBE_USER_BAND (160)

#define HON_D_FREQ_OFFSET_NO (20) //unic MHz

#define FRAMING_BYTE (0)
#define ADDRESS_BYTE (1)
#define COMMAND_BYTE (2)
#define DATA_BYTE_0 (3)
#define DATA_BYTE_1 (4)
#define DATA_BYTE_2 (5)

#define FORMAT_DISEQC_CMD_VALUE(a, F, A, C, aD, L) \
    {                                              \
        int i;                                     \
        a[FRAMING_BYTE] = F;                       \
        a[ADDRESS_BYTE] = A;                       \
        a[COMMAND_BYTE] = C;                       \
        for (i = 0; i < L; i++)                    \
        {                                          \
            a[DATA_BYTE_0 + i] = ((mt_u8 *)aD)[i]; \
        }                                          \
    }

/*#define ABS(val) (((val) < 0) ? (0 - (val)) : (val)) Clean Warning redefined*/

typedef enum
{
    TONE_SEARCH_UB_NOT_ASSIGNED,
    TONE_SEARCH_UB_ASSIGNED,
    TONE_SEARCH_UB_NO_TONE,
    TONE_SEARCH_UB_ACCEPT_ERROR
} TONE_SEARCH_UB_ACCEPT_E;

typedef enum
{
    UB_NOT_ASSIGNED,
    UB_ASSIGNED,
    UB_CHECK_BUTT
} UB_STATUS_E;

typedef struct
{
    mt_s32 s32Freq;
    mt_s32 s32Agc;
} SCR_TONE_S;

typedef struct
{
    mt_u32 u32UBIndex;
    mt_u32 u32CurrCentralFreq; //unit MHz
    mt_unf_fe_scr_ub_t u32UBInfo[SCR_USER_BAND_NUMBERS];
} UNICABLE_INFO_S;

//static SCR_TONE_S s_sScanToneList[SCR_D_MAX_POINTS_MAYBE_USER_BAND];

static UNICABLE_INFO_S s_SysUnicable;

static mt_u32 tsResultAgc = 0xFFFFFFFF;
static mt_u32 tsAvgToneCount = 0;
static mt_u32 tsAvgToneAgc = 0;

//static int32_t ScrPresetValue[SCR_D_MAX_USER_BAND] =
static int ScrPresetValue[SCR_D_MAX_USER_BAND] =
    {
        0x042,
        0x084,
        0x0c6,
        0x108,
        0x14a,
        0x18c,
        0x1ce,
        0x210};

//static int32_t ScrLastValue[SCR_D_MAX_USER_BAND] =
static int ScrLastValue[SCR_D_MAX_USER_BAND] =
    {
        0x042,
        0x084,
        0x0c6,
        0x108,
        0x14a,
        0x18c,
        0x1ce,
        0x210};

mt_s32 Scr_SetFrequency(mt_u32 tuner_id, mt_u32 Frequency, mt_u32 LNBIndex, mt_u32 SCRBPF);
mt_u32 Scr_RandValue(mt_u32 ucUbNo, mt_u32 ulIsFirstTime);
mt_u32 Scr_RandTime(mt_u32 ulMaxDur);
TONE_SEARCH_UB_ACCEPT_E Scr_CheckToneOnFreq(mt_u32 tuner_id, mt_u32 ulCenterFreq);
TONE_SEARCH_UB_ACCEPT_E Scr_CheckToneOnUB(mt_u32 tuner_id, mt_u32 SCRBPF);
mt_u32 Scr_GetToneAgc(mt_u32 tuner_id, mt_u32 u32CenterFreq);
mt_s32 Scr_CheckOffValid(mt_u32 tuner_id, mt_u32 u32Index, mt_u32 u32CentralFreq);
mt_u8 Scr_GetAverageAGC(mt_u8 *pcAGC, mt_s32 iLength);
mt_s32 Scr_DeleteSameTone(mt_u8 *pu8NbTones, mt_u32 *uiToneList, mt_u32 *uiPowerList);
mt_s32 Scr_SortUbByAgc(SCR_TONE_S *psUBList, mt_s32 s32Count);
mt_s32 Scr_SortUbByFreq(SCR_TONE_S *psUBList, mt_s32 s32Count);
void *Scr_BlindScanTone(void *tuner_id);

//extern mt_s32 unicable_diseqc_sendrecv_message(mt_u32 tuner_id,
//const mt_unf_fe_diseqc_sendmsg_t * pstSendMsg,
//mt_unf_fe_diseqc_recvmsg_t * pstRecvMsg);
extern mt_s32 unicable_diseqc_sendrecv_message(mt_u32 tuner_id,
                                               const mt_unf_fe_diseqc_sendmsg_t *p_sendmsg,
                                               mt_unf_fe_diseqc_recvmsg_t *p_recvmsg);
extern mt_unf_fe_switch_toneburst_t fe_diseqc_get_toneburst_status(mt_u32 tuner_id);

/**
uint8_t ucScrIndex: range[0-3]
*/
static mt_u32 Scr_GetCenterFreq(mt_u32 ucScrIndex)
{
    return (mt_u32)s_SysUnicable.u32UBInfo[s_SysUnicable.u32UBIndex].center_freq;
}

static int _search_tone(mt_u32 tuner_id, mt_u32 CenterFreq, mt_u32 *pu32ToneTrue, mt_u32 *peakAgc)
{
#if 1
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 u32PeakAgc;
    mt_u32 u32ToneTrue = 0;

    //MT_INFO_FRONTEND("\n center_freq is %d. \n", CenterFreq);

    if ((CenterFreq < SCR_D_SCAN_FREQ_START) || (CenterFreq > SCR_D_SCAN_FREQ_END))
    {
        MT_ERR_FRONTEND("start_freq_hz out of range\n");
        return MT_FAILURE;
    }

    s32Ret = mt_unf_fe_get_agc(tuner_id, (mt_s32)CenterFreq, (mt_s32 *)&u32PeakAgc);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("HI_UNF_TUNER_GetAgc fail.\n");
        return MT_FAILURE;
    }

    tsResultAgc = u32PeakAgc;
    *peakAgc = u32PeakAgc;

    if (u32ToneTrue)
    {
        if (tsAvgToneCount == 0)
        {
            tsAvgToneAgc = tsResultAgc;
            tsAvgToneCount++;
        }
        else
        {
            tsAvgToneAgc = (mt_u32)(((mt_u64)(tsAvgToneAgc)*tsAvgToneCount + tsResultAgc) / (tsAvgToneCount + 1));
            tsAvgToneCount++;
            //MT_INFO_FRONTEND("Get Average agc value %d\n", tsAvgToneAgc);
        }

        *pu32ToneTrue = 1;
    }
    else
    {
        *pu32ToneTrue = 0;
    }

    /*MT_USLEEP(2000);*/
#endif
    return MT_SUCCESS;
}

/* ----------------------------------------------------------------------------
Name: scr_tone_enable()
Description:

Parameters:
Return Value:
---------------------------------------------------------------------------- */
static mt_s32 Scr_EnableTone(mt_u32 tuner_id)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u8 u8ChanByte[2] = {0x0};

    /* E0(Framing)  10/11(address)   5A/5B(command)  data1(Sub_func = 0)   data2 = 00*/
    //unsigned char command_array [5] = {0xE0, 0x00, 0x5B, 0x00, 0x00};

    MT_INFO_FRONTEND("enter Scr_EnableTone\n");
    MT_INFO_FRONTEND("@@@@@@@@@@@@Send the tone enable cmd!");

    stSendMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, 0xE0, 0x10, 0x5B, u8ChanByte, 2);
    s32Ret = unicable_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("Send scr msg fail.\n");
        return s32Ret;
    }

    return s32Ret;
#else
    return 0;
#endif
}

/* ----------------------------------------------------------------------------
Name: Scr_OffTones()
Description:
Parameters:
Return Value:
---------------------------------------------------------------------------- */
static mt_s32 Scr_OffTones(mt_u32 tuner_id, mt_u32 SCRBPF)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u8 u8ChanByte[2] = {0};

    //unsigned char command_array[] = {0xE0, 0x00, 0x5A, 0x00, 0x00};

    if (SCRBPF >= 8)
    {
        return MT_FAILURE;
    }
    MT_INFO_FRONTEND("@@@@@@@@@@@@Send the scrdrv off cmd!");
    MT_INFO_FRONTEND("enter Scr_OffTones, tuner_id is 0x%x, UB is %d\n", tuner_id, SCRBPF + 1);

    stSendMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    u8ChanByte[0] |= (mt_u8)(SCRBPF << 5);

    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, 0xE0, 0x10, 0x5A, u8ChanByte, 2);
    s32Ret = unicable_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("Send scr msg fail.\n");
        return s32Ret;
    }

    /* 修改原因: 等待MDU设备将信号关闭 */
    MT_USLEEP(2000);
#endif
    return MT_SUCCESS;
}

/* ----------------------------------------------------------------------------
Name: scr_scrdrv_SetFrequency()
Description:
Parameters:
Return Value:
---------------------------------------------------------------------------- */
mt_s32 Scr_SetFrequency(mt_u32 tuner_id, mt_u32 Frequency, mt_u32 LNBIndex, mt_u32 SCRBPF)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_s32 s32Ret = MT_SUCCESS;
    mt_u32 tuning_word, temp;
    mt_u8 u8ChanByte[2] = {0};

    if (SCRBPF >= 12)
    {
        return MT_FAILURE;
    }

    MT_INFO_FRONTEND("@@@@@@@@@@@@Send the scrdrv setfreq cmd!");
    MT_INFO_FRONTEND("enter SetFrequency Scr set Freq %d, LNBIndex %d, SCRBPF %d\n", Frequency, LNBIndex, SCRBPF);

    stSendMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    SCRBPF = SCRBPF % 4;

    tuning_word = (Frequency / 4) - 350; /* Formula according to data sheet */

    u8ChanByte[1] = tuning_word & 0xFF;
    temp = tuning_word & 0x300;
    temp = temp >> 8;
    u8ChanByte[0] |= (mt_u8)temp;

    temp = SCRBPF << 5;
    u8ChanByte[0] |= (mt_u8)temp;

    temp = LNBIndex << 2;
    u8ChanByte[0] |= (mt_u8)temp;

    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, 0xE0, 0x10, 0x5A, u8ChanByte, 2);
    s32Ret = unicable_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("Send scr msg fail.\n");
        return s32Ret;
    }

    return (s32Ret);
#else
    return 0;
#endif
}

/* Clean Warning [-Wunused-function]
static mt_s32 Scr_UBxDeAllocation(mt_u32 tuner_id, mt_u8 SCRBPF)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_s32 s32Ret = MT_SUCCESS;
    //mt_u8 command_array [5] = {0xE0, 0x00, 0x5B, 0x05, 0x00};
    mt_u8 u8ChanByte[2] = {0x5, 0x0};

    if (SCRBPF >= SCR_USER_BAND_NUMBERS)
    {
        return MT_FAILURE;
    }

    MT_INFO_FRONTEND("@@@@@@@@@@@@Send the scrdrv deallocation cmd!");
    MT_INFO_FRONTEND("enter Scr_UBxDeAllocation,UB is %d\n", SCRBPF);

    stSendMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    u8ChanByte[0] |= (mt_u8)(SCRBPF << 5);

    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, 0xE0, 0x10, 0x5B, u8ChanByte, 2);
    s32Ret = unicable_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("Send scr msg fail.\n");
        return s32Ret;
    }

    return s32Ret;
#else
    return 0;
#endif
}*/
    
/*Clean Warning [-Wunused-function]
static mt_s32 Scr_UBxPresent(mt_u32 tuner_id, mt_u32 SCRBPF)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_s32 s32Ret = MT_SUCCESS;
    //E0(Framing)  10/11(address)   5B(command)  data1(Sub_func = 0x06)   data2 =00
    mt_u8 u8ChanByte[2] = {0x6, 0x0};

    if (SCRBPF >= SCR_USER_BAND_NUMBERS)
    {
        return MT_FAILURE;
    }
    MT_INFO_FRONTEND("@@@@@@@@@@@@Send the scrdrv ub present cmd!");
    MT_INFO_FRONTEND("enter Scr_UBxPresent,UB is %d\n", SCRBPF);

    stSendMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    u8ChanByte[0] |= (mt_u8)(SCRBPF << 5);

    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, 0xE0, 0x10, 0x5B, u8ChanByte, 2);
    s32Ret = unicable_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("Send scr msg fail.\n");
        return s32Ret;
    }

    return s32Ret;
#else
    return 0;
#endif
}*/

mt_u32 Scr_RandValue(mt_u32 ucUbNo, mt_u32 ulIsFirstTime)
{
#if 1
    mt_u32 ulValue = 0;
    mt_u32 ulPvalue = 0;
    mt_u8 ucX1 = 0, ucX3 = 0, ucX10 = 0;
    /** BEGIN: Modified by x00221621, 2014/1/2 问题单号:JIRA DS360T-68  */
    if (ucUbNo >= SCR_USER_BAND_NUMBERS)
    {
        return 0;
    }
    /** END:   Modified by x00221621, 2014/1/2 问题单号:JIRA DS360T-68  */
    if (ulIsFirstTime)
    {
        ulValue = (mt_u32)ScrPresetValue[ucUbNo];
    }
    else
    {
        ulValue = (mt_u32)ScrLastValue[ucUbNo];
    }
    ucX10 = ulValue & 0x001;
    ucX3 = (ulValue & 0x080) >> 7;
    ucX1 = (ucX3 ^ ucX10) & 0x01;
    ulPvalue = (mt_u32)((ulValue >> 1) | (mt_u32)(ucX1 << 9));
    ScrLastValue[ucUbNo] = (int)ulPvalue;

    /** Howie add 200ms to avoid a verry small value. */
    return (ulPvalue + 200);
#else
    return 0;
#endif
}

mt_u32 Scr_RandTime(mt_u32 ulMaxDur)
{
#if 1
    int fd = -1;
    mt_u32 ulRand = 0;
    mt_u32 ulRandTime = 0;

    fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    if (fd == -1)
    {
        MT_INFO_FRONTEND("============/dev/urandom===========\n");
        fd = open("/dev/urandom", O_RDONLY | O_NONBLOCK);
    }
    if (fd >= 0)
    {
        read(fd, &ulRand, sizeof(mt_u32)); /* generate a random integer from /dev/random */
        close(fd);
    }

    ulRandTime = ulRand % ulMaxDur;

    if (ulRandTime < 200)
    {
        ulRandTime += 200;
    }
    return ulRandTime;
#else
    return 0;
#endif
}

TONE_SEARCH_UB_ACCEPT_E Scr_CheckToneOnFreq(mt_u32 tuner_id, mt_u32 ulCenterFreq)
{
#if 1
    mt_u32 power_yes = 1;
    mt_u32 u32ToneTrueYes = 0;
    mt_u32 power_no = 1;
    mt_u32 u32ToneTrueNo = 0;
    mt_u32 center_freq = ulCenterFreq;
    mt_u32 ulTryTimes = 0;
    TONE_SEARCH_UB_ACCEPT_E enAccept = TONE_SEARCH_UB_NO_TONE;

    MT_INFO_FRONTEND("Start tone signal scan. Freq = %d!\n", ulCenterFreq);

    if ((center_freq < SCR_D_SCAN_FREQ_START) || (center_freq > SCR_D_SCAN_FREQ_END))
    {
        MT_ERR_FRONTEND("center_freq out of range!\n");
        return TONE_SEARCH_UB_NO_TONE;
    }

    do
    {
        ulTryTimes++;
        /** 获取位置YES的TONE信号状态 */
        (void)_search_tone(tuner_id, center_freq, &u32ToneTrueYes, &power_yes);

        /** 获取位置NO的TONE信号状态 */
        (void)_search_tone(tuner_id, center_freq + HON_D_FREQ_OFFSET_NO, &u32ToneTrueNo, &power_no);

        MT_INFO_FRONTEND("[%d]Search tone yes %d   %d", center_freq, center_freq, power_yes);
        MT_INFO_FRONTEND("[%d]Search tone no  %d   %d", center_freq, center_freq + HON_D_FREQ_OFFSET_NO, power_no);

        if (u32ToneTrueYes)
        {
            MT_INFO_FRONTEND("[YES] tone detected. tone_freq = %d!\n", center_freq);
            enAccept = TONE_SEARCH_UB_NOT_ASSIGNED;
            break;
        }
        else if (u32ToneTrueNo)
        {
            MT_INFO_FRONTEND("[NO] tone detected. tone_freq = %d!\n", center_freq + HON_D_FREQ_OFFSET_NO);
            enAccept = TONE_SEARCH_UB_ASSIGNED;
            break;
        }
        else
        {
            MT_ERR_FRONTEND("[%d]There is no valid tone detected. \n", ulTryTimes);
        }

    } while (ulTryTimes < 3);
    MT_INFO_FRONTEND("Stop tone signal scan. center_freq = %d!\n", center_freq);
    return enAccept;
#else
    return 0;
#endif
}

TONE_SEARCH_UB_ACCEPT_E Scr_CheckToneOnUB(mt_u32 tuner_id, mt_u32 SCRBPF)
{
#if 1
    mt_u32 center_freq = 0;
    mt_u32 ucIndex = 0;

    MT_INFO_FRONTEND("Start tone signal scan. UB = %d!\n", SCRBPF + 1);

    ucIndex = SCRBPF;

    center_freq = Scr_GetCenterFreq(ucIndex);

    return Scr_CheckToneOnFreq(tuner_id, center_freq);
#else
    return 0;
#endif
}

mt_u32 Scr_GetToneAgc(mt_u32 tuner_id, mt_u32 u32CenterFreq)
{
#if 1
    mt_u32 u32RatioYes = 1;
    mt_u32 u32ToneTrue = 0;
    mt_u32 u32IfAgcYes = 0xffffffff;

    MT_INFO_FRONTEND("Start tone signal scan. ulCenterFreq = %d!\n", u32CenterFreq);

    if ((u32CenterFreq < SCR_D_SCAN_FREQ_START) || (u32CenterFreq > SCR_D_SCAN_FREQ_END))
    {
        MT_ERR_FRONTEND("center_freq out of range!\n");
        return u32RatioYes;
    }

    /** 获取位置YES的TONE信号状态 */
    _search_tone(tuner_id, u32CenterFreq, &u32ToneTrue, &u32IfAgcYes);

    if (u32ToneTrue)
    {
        MT_INFO_FRONTEND("[YES] tone detected. tone_freq = %d!\n", u32CenterFreq);
    }
    else
    {
        MT_INFO_FRONTEND("There is no valid tone detected. \n");
    }

    //MT_INFO_FRONTEND("(%s,%d): Stop tone signal scan. center_freq = %d!\n", __FUNCTION__, __LINE__, u32CenterFreq);
    return u32IfAgcYes;
#else
    return 0;
#endif
}

mt_s32 Scr_CheckOffValid(mt_u32 tuner_id, mt_u32 u32Index, mt_u32 u32CentralFreq)
{
#if 1
    mt_u32 u32TaskDelayMs = 0;
    mt_s32 s32Agc = 0;
    mt_u32 i = 0;
    mt_u8 u32FreqIndex = 0;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u32 u32Fcenter = 0;

    if (SCR_USER_BAND_NUMBERS <= u32Index)
    {
        MT_ERR_FRONTEND("cIndex out of range!\n");
        return MT_FAILURE;
    }

    MT_INFO_FRONTEND("Check off valid index[%d]  centralfreq[%d]", u32Index, u32CentralFreq);

    u32FreqIndex = (mt_u8)u32Index;
    u32Fcenter = u32CentralFreq;

    if (0 == u32Fcenter)
    {
        u32Fcenter = Scr_GetCenterFreq(u32FreqIndex);
    }

    for (i = 0; i < 3; i++)
    {
        /*ulTaskDelayMs = Scr_RandValue(ucIndex,ulIsFirstTime);
        MT_USLEEP(ulTaskDelayMs);*/
        u32TaskDelayMs = Scr_RandTime(1000);
        MT_INFO_FRONTEND("==============ulTaskDelayMs = %d\n", u32TaskDelayMs);
        MT_USLEEP(u32TaskDelayMs + 1000);

        Scr_OffTones(tuner_id, u32Index);

        Scr_GetToneAgc(tuner_id, u32Fcenter);

        s32Agc = (mt_s32)tsResultAgc;
        MT_INFO_FRONTEND("+++tsResultAgc:%d+++tsAvgToneAgc:%d\n", tsResultAgc, tsAvgToneAgc);

        if (s32Agc > (mt_s32)tsAvgToneAgc + 300)
        {
            MT_INFO_FRONTEND("UB:%d off valid.\n", u32Index + 1);
            s32Ret = MT_SUCCESS;
            break;
        }
    }

    return s32Ret;
#else
    return 0;
#endif
}

/** 修改原因: 解决MDU自动安装冲突问题 */
/*Clean Warning [-Wunused-function]
static TONE_SEARCH_UB_ACCEPT_E Scr_MDUAccept(mt_u32 tuner_id, mt_u32 SCRBPF, mt_u16 SCRCenterFrequency)
{
#if 1
    mt_unf_fe_diseqc_sendmsg_t stSendMsg;
    mt_s32 s32Ret = MT_FAILURE;
    mt_u8 u8ChanByte[2] = {0x4, 0x0};
    /// HON_Status Error1       = NEXUS_TIMEOUT; 

    //mt_u8 command_array [5] = {0xE0, 0x00, 0x5B, 0x04, 0x00};

    TONE_SEARCH_UB_ACCEPT_E enAccept = TONE_SEARCH_UB_NO_TONE;

    MT_INFO_FRONTEND("entered SCRBPF %d, Freq %d\n", SCRBPF + 1, SCRCenterFrequency);

    if (SCRBPF >= SCR_USER_BAND_NUMBERS || SCRCenterFrequency <= 950 || SCRCenterFrequency >= 2150)
    {
        MT_ERR_FRONTEND("Accept error\n");
        return TONE_SEARCH_UB_ACCEPT_ERROR;
    }

    stSendMsg.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
    stSendMsg.tone_burst = fe_diseqc_get_toneburst_status(tuner_id);
    stSendMsg.len = 5;
    stSendMsg.repeat_times = 0;

    // E0(Framing)  10/11(address)   5B(command)  data1(Sub_func = 0x04)   data2 =00 
    u8ChanByte[0] |= (mt_u8)(SCRBPF << 5);

    MT_INFO_FRONTEND("@@@@@@@@@@@@Send the scrdrv ub accept cmd!");
    MT_INFO_FRONTEND("Accept the ub %d !", SCRBPF);

    FORMAT_DISEQC_CMD_VALUE(stSendMsg.data, 0xE0, 0x0, 0x5B, u8ChanByte, 2);
    s32Ret = unicable_diseqc_sendrecv_message(tuner_id, &stSendMsg, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MT_ERR_FRONTEND("Send scr msg fail.\n");
        return s32Ret;
    }

    MT_USLEEP(1000);

    // 修改原因: 只要状态不是空闲，都需要放弃该UB
    enAccept = Scr_CheckToneOnFreq(tuner_id, SCRCenterFrequency);

    return enAccept;
#else
    return 0;
#endif
}*/

mt_u8 Scr_GetAverageAGC(mt_u8 *pcAGC, mt_s32 iLength)
{
#if 1
    mt_s32 iLoop = 0;
    mt_s32 iIndex = 0;
    mt_u8 cAgcTemp = 0;
    mt_s32 iAgcSum = 0;
    mt_s32 iAgcAverage = 0;

    if (0 >= iLength)
    {
        return 0;
    }
    else if (1 == iLength)
    {
        return *pcAGC;
    }

    /** Sort the AGC value at first. */
    for (iLoop = 0; iLoop < iLength - 1; iLoop++)
    {
        for (iIndex = iLoop + 1; iIndex < iLength; iIndex++)
        {
            if (*(pcAGC + iLoop) < *(pcAGC + iIndex))
            {
                cAgcTemp = *(pcAGC + iLoop);
                *(pcAGC + iLoop) = *(pcAGC + iIndex);
                *(pcAGC + iIndex) = cAgcTemp;
            }
        }
    }

    /** Calculate the average value of AGC. */
    if (2 == iLength)
    {
        iAgcAverage = (*pcAGC + *(pcAGC + 1)) / 2;
    }
    else
    {
        for (iIndex = 1; iIndex < iLength - 1; iIndex++)
        {
            iAgcSum += *(pcAGC + iIndex);
        }

        iAgcAverage = iAgcSum / (iLength - 2);
    }

    return (mt_u8)iAgcAverage;
#else
    return 0;
#endif
}

mt_s32 Scr_DeleteSameTone(mt_u8 *pu8NbTones, mt_u32 *uiToneList, mt_u32 *uiPowerList)
{
#if 1
    mt_u32 LastTone = 0;
    mt_u32 ToneSum = 0;
    mt_u32 PowerSum = 0;
    //mt_u32 u32ToneTmp = 0;
    //mt_u32 u32PowerTmp = 0;
    mt_u32 u32Count = 1;
    mt_u32 UBList[SCR_D_MAX_POINTS_MAYBE_USER_BAND];
    mt_u32 PowerList[SCR_D_MAX_POINTS_MAYBE_USER_BAND];
    mt_u8 i, j = 0;
    mt_u8 TonesNum = 0;

    TonesNum = *pu8NbTones;
    LastTone = *uiToneList;
    ToneSum += LastTone;
    PowerSum += *uiPowerList;
    for (i = 1; i < TonesNum; i++)
    {
        if (ABS((mt_s32) * (uiToneList + i) - (mt_s32)LastTone) <= 10)
        {
            ToneSum += *(uiToneList + i);
            PowerSum += *(uiPowerList + i);
            u32Count++;
            LastTone = *(uiToneList + i);
        }
        else
        {
            UBList[j] = ToneSum / u32Count;
            PowerList[j] = PowerSum / u32Count;
            ToneSum = 0;
            PowerSum = 0;
            u32Count = 1;
            LastTone = *(uiToneList + i);
            ToneSum += LastTone;
            PowerSum += *(uiPowerList + i);
            j++;
        }
    }

    UBList[j] = ToneSum / u32Count;
    PowerList[j] = PowerSum / u32Count;
    j++;

    for (i = 0; i < j; i++)
    {
        *(uiToneList + i) = UBList[i];
        *(uiPowerList + i) = PowerList[i];
    }

    *pu8NbTones = j;
#endif
    return 0;
}

mt_s32 Scr_SortUbByAgc(SCR_TONE_S *psUBList, mt_s32 s32Count)
{
#if 1
    mt_s32 s32Index = 0;
    SCR_TONE_S sUbTemp;
    SCR_TONE_S sUb1;
    SCR_TONE_S sUb2;
    mt_s32 s32Loop = 0;

    memset((void *)&sUbTemp, 0x00, sizeof(sUbTemp));
    memset((void *)&sUb1, 0x00, sizeof(sUb1));
    memset((void *)&sUb2, 0x00, sizeof(sUb2));

    if (1 >= s32Count)
    {
        return MT_FAILURE;
    }

    /** Sort the AGC value at first. */
    for (s32Loop = 0; s32Loop < s32Count - 1; s32Loop++)
    {
        for (s32Index = s32Loop + 1; s32Index < s32Count; s32Index++)
        {
            sUb1 = *(psUBList + s32Loop);
            sUb2 = *(psUBList + s32Index);
            if (sUb1.s32Agc > sUb2.s32Agc)
            {
                sUbTemp = *(psUBList + s32Loop);
                *(psUBList + s32Loop) = *(psUBList + s32Index);
                *(psUBList + s32Index) = sUbTemp;
            }
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 Scr_SortUbByFreq(SCR_TONE_S *psUBList, mt_s32 s32Count)
{
#if 1
    mt_s32 s32Loop = 0;
    mt_s32 s32Index = 0;
    SCR_TONE_S sUbTemp;
    SCR_TONE_S sUb1;
    SCR_TONE_S sUb2;

    memset((void *)&sUbTemp, 0x00, sizeof(sUbTemp));
    memset((void *)&sUb1, 0x00, sizeof(sUb1));
    memset((void *)&sUb2, 0x00, sizeof(sUb2));

    if (1 >= s32Count)
    {
        return MT_FAILURE;
    }

    /** Sort the freq value at first. */
    for (s32Loop = 0; s32Loop < s32Count - 1; s32Loop++)
    {
        for (s32Index = s32Loop + 1; s32Index < s32Count; s32Index++)
        {
            sUb1 = *(psUBList + s32Loop);
            sUb2 = *(psUBList + s32Index);
            if (sUb1.s32Freq > sUb2.s32Freq)
            {
                sUbTemp = *(psUBList + s32Loop);
                *(psUBList + s32Loop) = *(psUBList + s32Index);
                *(psUBList + s32Index) = sUbTemp;
            }
        }
    }
#endif
    return MT_SUCCESS;
}

void *Scr_BlindScanTone(void *tuner_id)
{
#if 1
    mt_s32 s32Index = 0, s32Index2 = 0;
    mt_s32 s32MinAgcIndex = 0;
    //mt_s32 s32Ret = MT_FAILURE;
    mt_s32 s32Count = 0, s32ToneFreucncy = 0;
    mt_s32 s32Threshold = 0;
    mt_s32 s32TolerantAgcDistance = 0;
    mt_s32 s32ToneFreq[SCR_USER_BAND_NUMBERS] = {0};
    mt_u32 u32UBCount = 0, u32RealUBNumber = 0;
    mt_u32 u32TunerPort;
    SCR_TONE_S sSCRArray[1200];
    SCR_TONE_S sMinAgc[SCR_USER_BAND_NUMBERS], sMaxAgc;
    //SCR_TONE_S sThresholdStart, sThresholdStop;
    mt_double dThresholdStart = 0, dThresholdStop = 0, dToneFreq = 0;

    mt_set_pthread_name(__FUNCTION__);
    u32TunerPort = *((mt_u32 *)tuner_id);
    Scr_EnableTone(u32TunerPort);
    MT_USLEEP(1000);

    memset(sSCRArray, 0x77, sizeof(sSCRArray));

    for (s32Index = SCR_D_SCAN_FREQ_START; s32Index < SCR_D_SCAN_FREQ_END; s32Index++)
    {
        sSCRArray[s32Count].s32Freq = s32Index;
        (void)mt_unf_fe_get_agc(u32TunerPort, s32Index, &(sSCRArray[s32Count].s32Agc));
        s32Count++;
    }

    Scr_SortUbByAgc(sSCRArray, s32Count);

    sMinAgc[0] = sSCRArray[0];
    u32UBCount = 1;
    for (s32Index = 1; s32Index < s32Count; s32Index++)
    {
        for (s32Index2 = 0; s32Index2 < (mt_s32)u32UBCount; s32Index2++)
        {
            if (ABS(sSCRArray[s32Index].s32Freq - sMinAgc[s32Index2].s32Freq) > MIN_DISTANCE_BETWEEN_UB_MHZ)
                continue;
            else
                break;
        }

        if (s32Index2 == (mt_s32)u32UBCount)
        {
            sMinAgc[u32UBCount] = sSCRArray[s32Index];
            u32UBCount++;
            if (u32UBCount >= SCR_USER_BAND_NUMBERS)
            {
                break;
            }
        }
    }

    Scr_SortUbByFreq(sSCRArray, s32Count);

    Scr_SortUbByAgc(sMinAgc, SCR_USER_BAND_NUMBERS);

    //tuner lowpass filter bandwidth = 30MHz
    sMaxAgc = sSCRArray[sMinAgc[3].s32Freq - 15 - SCR_D_SCAN_FREQ_START];

    //agc threshold equal one half of max agc subing min agc
    s32Threshold = sMinAgc[3].s32Agc + ABS(sMaxAgc.s32Agc - sMinAgc[3].s32Agc) / 4;
    //s32Threshold = 1385;

    //error distance near threshold
    s32TolerantAgcDistance = ABS(s32Threshold - sMinAgc[3].s32Agc) / 2;
    if (s32TolerantAgcDistance > THRESHOLD_TOLERANT_AGC_DISTANCE)
    {
        //THRESHOLD_TOLERANT_AGC_DISTANCE is an experiential value
        s32TolerantAgcDistance = THRESHOLD_TOLERANT_AGC_DISTANCE;
    }

    MT_INFO_FRONTEND("++++sMinAgc.u32Freq:%d+++sMinAgc.u32Agc:%d+++sMaxAgc.u32Freq:%d+++sMaxAgc.u32Agc:%d\n", sMinAgc[3].s32Freq, sMinAgc[3].s32Agc, sMaxAgc.s32Freq, sMaxAgc.s32Agc);
    MT_INFO_FRONTEND("++++++++u32Index:%d++++++u32Threshold:%d+++s32TolerantAgcDistance:%d\n", s32Index, s32Threshold, s32TolerantAgcDistance);

    for (s32Index = 0; s32Index < SCR_USER_BAND_NUMBERS; s32Index++)
    {
        MT_INFO_FRONTEND("++++TONE %d freq is:%d\n", s32Index, sMinAgc[s32Index].s32Freq);
    }

    for (u32UBCount = 0; u32UBCount < SCR_USER_BAND_NUMBERS; u32UBCount++)
    {
        s32MinAgcIndex = sMinAgc[u32UBCount].s32Freq - SCR_D_SCAN_FREQ_START;
        MT_INFO_FRONTEND("++++sMinAgc.u32Freq:%d+++sMinAgc.u32Agc:%d\n", sMinAgc[u32UBCount].s32Freq, sMinAgc[u32UBCount].s32Agc);
        MT_INFO_FRONTEND("freq start:%dMHz ", sSCRArray[s32MinAgcIndex - 50].s32Freq);
        for (s32Index = s32MinAgcIndex - 50; s32Index < s32MinAgcIndex + 50; s32Index++)
        {
            MT_INFO_FRONTEND("%d ", sSCRArray[s32Index].s32Agc);
        }

        MT_INFO_FRONTEND("\n");
    }

    for (u32UBCount = 0; u32UBCount < SCR_USER_BAND_NUMBERS; u32UBCount++)
    {
        if (sMinAgc[u32UBCount].s32Agc <= s32Threshold)
        {
            s32ToneFreq[u32UBCount] = sMinAgc[u32UBCount].s32Freq;
            u32RealUBNumber++;
        }
        else
            break;
    }

    Scr_SortUbByFreq(sMinAgc, (mt_s32)u32RealUBNumber);

    /*if(sSCRArray[s32MinAgcIndex + 30].s32Agc - sMinAgc[0].s32Agc < THRESHOLD_TOLERANT_AGC_DISTANCE)
	{
		MT_INFO_FRONTEND("there is no tone.\n");
		return MT_FAILURE;
	}*/

    for (u32UBCount = 0; u32UBCount < u32RealUBNumber; u32UBCount++)
    {
        s32MinAgcIndex = sMinAgc[u32UBCount].s32Freq - SCR_D_SCAN_FREQ_START;
        dThresholdStart = 0;
        dThresholdStop = 0;
        dToneFreq = 0;

        for (s32Index = s32MinAgcIndex; s32Index > s32MinAgcIndex - 30; s32Index--)
        {
            if (sSCRArray[s32Index].s32Agc < s32Threshold)
            {
                continue;
            }

            //连续两个点都在阈值线以下
            if (sSCRArray[s32Index - 1].s32Agc >= s32Threshold)
            {
                break;
            }
        }

        dThresholdStart = (mt_double)sSCRArray[s32Index].s32Freq + ((mt_double)sSCRArray[s32Index + 1].s32Freq - (mt_double)sSCRArray[s32Index].s32Freq) / 2.0;

        for (s32Index = s32MinAgcIndex; s32Index < s32MinAgcIndex + 30; s32Index++)
        {
            if (sSCRArray[s32Index].s32Agc < s32Threshold)
            {
                continue;
            }

            //连续两个点都在阈值线以下
            if (sSCRArray[s32Index + 1].s32Agc >= s32Threshold)
            {
                break;
            }
        }

        dThresholdStop = (mt_double)sSCRArray[s32Index - 1].s32Freq + ((mt_double)sSCRArray[s32Index].s32Freq - (mt_double)sSCRArray[s32Index - 1].s32Freq) / 2.0;
        dToneFreq = dThresholdStart + (dThresholdStop - dThresholdStart) / 2.0;
        s32ToneFreucncy = (mt_s32)dToneFreq;
        MT_INFO_FRONTEND("<<<<<<<<<====dToneFreq:%f======s32ToneFreucncy:%d\n", dToneFreq, s32ToneFreucncy);

        if (s32ToneFreucncy % 2)
        {
            for (s32Index2 = 1; s32Index2 < 30; s32Index2++)
            {
                if (sSCRArray[s32MinAgcIndex + s32Index2].s32Agc < sSCRArray[s32MinAgcIndex - s32Index2].s32Agc)
                {
                    if (sSCRArray[s32MinAgcIndex + s32Index2 + 1].s32Agc <= sSCRArray[s32MinAgcIndex - s32Index2 - 1].s32Agc)
                    {
                        s32ToneFreucncy += 1;
                        break;
                    }
                }
                else if (sSCRArray[s32MinAgcIndex + s32Index2].s32Agc > sSCRArray[s32MinAgcIndex - s32Index2].s32Agc)
                {
                    if (sSCRArray[s32MinAgcIndex + s32Index2 + 1].s32Agc >= sSCRArray[s32MinAgcIndex - s32Index2 - 1].s32Agc)
                    {
                        s32ToneFreucncy -= 1;
                        break;
                    }
                }
            }
        }

        s32ToneFreq[u32UBCount] = s32ToneFreucncy;
    }

    for (u32UBCount = 0; u32UBCount < u32RealUBNumber; u32UBCount++)
    {
        MT_INFO_FRONTEND("++++++++u32ToneFreq:%d\n", s32ToneFreq[u32UBCount]);
    }

    for (u32UBCount = 0; u32UBCount < u32RealUBNumber; u32UBCount++)
    {
        s_SysUnicable.u32UBInfo[u32UBCount].scr_no = u32UBCount;
        s_SysUnicable.u32UBInfo[u32UBCount].center_freq = s32ToneFreq[u32UBCount];
    }

    s_SysUnicable.u32UBIndex = 0;
#endif
    return NULL;
}

mt_s32 mt_unf_fe_scan_and_install_unicable(mt_u32 tuner_id)
{
#if 1
    //TONE_SEARCH_UB_ACCEPT_E enToneState = TONE_SEARCH_UB_NO_TONE;
    //mt_s32 s32ToneList[SCR_USER_BAND_NUMBERS] = {0};
    //mt_s32 s32PwTonelist[SCR_USER_BAND_NUMBERS] = {0};
    mt_s32 s32Result;
    //mt_u32 i = 0;
    static mt_u32 u32TunerPort;
    pthread_t *pScanUB = MT_NULL;

    tsAvgToneCount = 0;
    tsAvgToneAgc = 0;

    /*在空闲UB上产生tone信号*/
    Scr_EnableTone(tuner_id);

    MT_USLEEP(2000);

    if (MT_NULL != pScanUB)
    {
        (mt_void) pthread_join(*pScanUB, MT_NULL);
        mt_free(MT_ID_FRONTEND, pScanUB);
        pScanUB = MT_NULL;
    }

    pScanUB = (pthread_t *)mt_malloc(MT_ID_FRONTEND, sizeof(pthread_t));
    if (MT_NULL == pScanUB)
    {
        MT_ERR_FRONTEND("No memory.\n");
        return MT_FAILURE;
    }

    u32TunerPort = tuner_id;
    /*全频段扫描疑似tone信号，并保存*/
    s32Result = pthread_create(pScanUB, 0, Scr_BlindScanTone, &u32TunerPort);
    if (MT_SUCCESS != s32Result)
    {
        MT_ERR_FRONTEND("Create pthread fail.\n");
        if (pScanUB)
        {
            mt_free(MT_ID_FRONTEND, pScanUB);
            pScanUB = MT_NULL;
        }

        return MT_FAILURE;
    }

#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_get_unicable_info(mt_u32 tuner_id, mt_unf_fe_scr_ub_t *p_ubinfo)
{
#if 1
    if (MT_NULL == p_ubinfo)
    {
        MT_ERR_FRONTEND("data is null ptr.\n");
        return MT_FAILURE;
    }

    memcpy(p_ubinfo, s_SysUnicable.u32UBInfo, SCR_USER_BAND_NUMBERS * sizeof(mt_unf_fe_scr_ub_t));
#endif
    return MT_SUCCESS;
}

mt_s32 mt_unf_fe_set_current_unicable(mt_u32 tuner_id)
{
    mt_s32 s32Ret = MT_FAILURE;
#if 1

    s_SysUnicable.u32UBIndex = 0;
    s_SysUnicable.u32CurrCentralFreq = (mt_u32)s_SysUnicable.u32UBInfo[s_SysUnicable.u32UBIndex].center_freq;
#endif

    return s32Ret;
}

/* ----------------------------- End of file (mdu.c) ------------------ */
