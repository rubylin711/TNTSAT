/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#if 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_ecs.h"
#include "mt_unf_demux.h"
#include "HA.AUDIO.MP3.decode.h"
#include "mt_unf_ecs.h"
#include "mt_adp.h"
#include "mt_adp_audio.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"

#ifdef CONFIG_SUPPORT_CA_RELEASE
#define MT_FE_ERR(format, arg...)
#define MT_FE_INFO(format, arg...)
#define MT_FE_WARN(format, arg...)
#else
#define MT_FE_ERR(format, arg...) printf("[ERR]: %s,%d: " format, __FUNCTION__, __LINE__, ##arg)
#define MT_FE_INFO(format, arg...) printf(format, ##arg)
#define MT_FE_WARN(format, arg...) printf("[WARNING]: %s,%d: " format, __FUNCTION__, __LINE__, ##arg)
#endif

#define MT_FE_SIGNAL_TYPE (1)

#if (MT_FE_SIGNAL_TYPE == 1)
#define GET_CONNECT_PARA(stConnectPara)                                    \
    {                                                                      \
	stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_CAB;                   \
	stConnectPara.connect_param.cab.b_reverse = 0;                     \
	stConnectPara.connect_param.cab.freq = 474000;                     \
	stConnectPara.connect_param.cab.sym_rate = 6875000;                \
	stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64; \
	stConnectPara.connect_param.cab.band_width = 8;                    \
    }
#elif(MT_FE_SIGNAL_TYPE == 2)
#define GET_CONNECT_PARA(stConnectPara)                                          \
    {                                                                            \
	stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT;                         \
	stConnectPara.connect_param.sat.freq = 3840000;                          \
	stConnectPara.connect_param.sat.sym_rate = 27500000;                     \
	stConnectPara.connect_param.sat.polarization = MT_UNF_FE_POLARIZATION_H; \
    }
#elif(MT_FE_SIGNAL_TYPE == 4)
#define GET_CONNECT_PARA(stConnectPara)                    \
    {                                                      \
	stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T; \
	stConnectPara.connect_param.ter.b_reverse = 0;     \
	stConnectPara.connect_param.ter.freq = 682000;     \
	stConnectPara.connect_param.ter.band_width = 8000; \
    }
#endif

mt_unf_demod_type_t S_DEMOD_TYPE = MT_UNF_DEMOD_DEV_TYPE_M88DC2800;
typedef mt_void (*Test_Func_Proc)(char *args);
typedef struct stTEST_FUNC_S
{
    mt_char *name;
    Test_Func_Proc proc;
} TEST_FUNC_S;

typedef struct stCMD_HELP_S
{
    mt_char name[30];
    mt_char help_info[3000];
} CMD_HELP_S;

CMD_HELP_S g_cmdhelp[] =
    {
        { "help", "help: show help menu, only input 'help' will print command list,\n"
	          "\t 'help cmd' will print the help infomation of the cmd.\n" },
        { "getsignalinfo", "getsignalinfo: get detailed infomation of current locked signal.[FOR satellite and terrestrial signal].\n" },
        { "play", "play 513 660: set video pid 513 and audio pid 660\n" },
        { "getmsc", "getmsc: get mosaic num ,arg type: int time,float berlimit\n" },
        { "getber", "getber: get ber\n" },
        { "settype", "settype 0: set tuner type. \n"
	             "\t 0  cd1616,  1  tdae3,  2  mt2081,  3  tdcc,   4  tmx7070x,\n"
	             "\t 5  tda18250, 6  tda18250b 7  mxl203, 8  r820c\n" },
        { "setfreq", "setfreq 403: set freqency(MHz for cable).\n" },
        { "setsymb", "setsymb 6875000: set symbrate, unit baud\n" },
        { "setqam", "setqam  64: set qam type , 64 means 64qam etc.\n" },
        { "getoffset", "getoffset : getfreq getsyb. \n" },
        { "start", "start  500: lock 500 times --->time_file\n" },
        { "setmode", "setmode j83b/j83ac:change mode to j83b or j83ac\n" },
        { "exit", "exit    : exit the sample\n" }
    };

#define MAX_CMD_BUFFER_LEN 256
#define UDP_STR_SPLIT ' '
#define TEST_ARGS_SPLIT " "
#define MT_RESULT_SUCCESS "SUCCESS"
#define MT_RESULT_FAIL "FAIL"
#define DEFAULT_PORT 1234

static mt_s32 g_tuner_id = 0;
/*static mt_s32 g_tuner_id = 1;*/
/*static mt_u32 s_u32TunerFreq = 403000;*/
/*static mt_u32 s_u32ErrorNum;*/
/*static mt_float s_fMskBer = 0.0014;*/
/*static MT_U8 s_au8MskTmp[64];*/
static mt_u32 s_u32CurrentQamType;
static mt_u32 s_u32J83B;

/*save results, then send client*/
char s_acTestResult[MAX_CMD_BUFFER_LEN];

static mt_unf_fe_connect_para_t s_stConnectPara;

mt_u32 ProgNum = 0;

FILE *fp = NULL;
static mt_s32 s_s32LoopNum = 0;

static mt_s32 s_s32FailTime = 0;
static mt_s32 s_s32Out1Time = 0;
static mt_s32 s_s32OutFailTime = 0;
static mt_s32 s_s32Time = 0;

mt_s32 printime_init()
{

    FILE *time_file = NULL;

    time_file = fopen("time_file", "wt");
    if (NULL == time_file) {
	MT_FE_ERR("open time_file ,line = %d\n", __LINE__);
	return MT_FAILURE;
    }

    fclose(time_file);

    return MT_SUCCESS;
}
void printtime_file(int locknum, int locktime, int isfail)
{
    FILE *time_file = NULL;

#ifdef CONFIG_SUPPORT_CA_RELEASE
    time_file = fopen("/tmp/time_file", "at");
#else
    time_file = fopen("time_file", "at");
#endif
    if (NULL == time_file) {
	MT_FE_INFO("open time_file,line = %d\n", __LINE__);
    }
    if (isfail == 1) {
	/*fprintf(time_file, "FAIL: locknum = %d,locktime = %d   isfail = %d\n", locknum, locktime,isfail);*/
	fprintf(time_file, "%d       isfail = %d\n", locktime, isfail);
	s_s32Time = s_s32Time + locktime;
	s_s32FailTime++;
    } else if (isfail == 2) {
	s_s32Out1Time++;
	fprintf(time_file, "%d      isfail = %d\n", locktime, isfail);
	s_s32Time = s_s32Time + locktime;
    } else if (isfail == 3) {
	s_s32OutFailTime++;
	if (locktime > 1000) {
	    s_s32Out1Time++;
	}
	fprintf(time_file, "%d      isfail = %d\n", locktime, isfail);
	s_s32Time = s_s32Time + locktime;
    } else {
	/*fprintf(time_file, "OK: locknum = %d,locktime = %d,isfail = %d\n", locknum, locktime,isfail);*/
	fprintf(time_file, "%d   isfail = %d\n", locktime, isfail);
	s_s32Time = s_s32Time + locktime;
    }

    if (locknum + 1 >= s_s32LoopNum) {

	/*printf("locknum =%d,s_s32LoopNum=%d\n",locknum,s_s32LoopNum);*/
	fprintf(time_file, "locknum =%d,s_s32LoopNum=%d\n", locknum + 1, s_s32LoopNum);
	fprintf(time_file, "AVERVAGE TIME : %d\n", s_s32Time / (s_s32LoopNum));
	fprintf(time_file, "FAIL NUM: %d\n", s_s32FailTime);
	fprintf(time_file, " >1s NUM: %d\n", s_s32Out1Time);
	fprintf(time_file, " out fail  NUM: %d\n", s_s32OutFailTime);
    }
    fclose(time_file);
}

mt_u32 getcurtime()
{
    struct timeval tv;
    gettimeofday(&tv, MT_NULL);
    return (((mt_u32)tv.tv_sec) * 1000 + ((mt_u32)tv.tv_usec) / 1000);
}

mt_void set_pin_mux(mt_unf_fe_attr_t fe_attr)
{
}

//#define BOARD_TYPE_hi3716mdmo3fvera

mt_s32 dev_init()
{
    mt_s32 ret = 0;
    mt_unf_fe_attr_t fe_attr;
    //  HI_UNF_TUNER_SAT_ATTR_S   stSatTunerAttr;
    //MT_UNF_DMX_PORT_ATTR_S PortAttr;

    set_pin_mux(fe_attr);

    /*sys init*/
    mt_sys_init();

#if 0
    /*sound init*/
    ret = MTADP_Snd_Init();
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MTADP_Snd_Init failed.\n");
        return ret;
    }

    /*display init*/
    ret = MTADP_Disp_Init(s_enDefaultFmt);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MTADP_Disp_DeInit failed.\n");
        return ret;
    }

    /*vo init*/
    ret = MTADP_VO_Init(HI_UNF_VO_DEV_MODE_NORMAL);
    ret |= MTADP_VO_CreatWin(MT_NULL,&phWin);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MTADP_VO_Init failed.\n");
        MTADP_VO_DeInit();
        return ret;
    }
#endif

#if 1
    ret = mt_unf_fe_init();
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call mt_unf_fe_init failed.\n");
	return ret;
    }

    /* open Tuner*/
    ret = mt_unf_fe_open(g_tuner_id);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call mt_unf_fe_open failed.\n");
	mt_unf_fe_deinit();
	return ret;
    }

    /* get default attribute */
    ret = mt_unf_fe_get_default_attr(g_tuner_id, &fe_attr);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call mt_unf_fe_get_default_attr failed.\n");
	mt_unf_fe_close(g_tuner_id);
	mt_unf_fe_deinit();
	return ret;
    }

    //MTADP_FE_GET_CONFIG(g_tuner_id, fe_attr);
    ret = mt_unf_fe_set_attr(g_tuner_id, &fe_attr);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
	return ret;
    }

#if 0

#ifdef GET_TUNER0_TSOUT_CONFIG
    {
	if (g_tuner_id == 0) {
	    mt_unf_fe_ts_out_t stTSOut0;
	    GET_TUNER0_TSOUT_CONFIG(stTSOut0);
	    ret = mt_unf_fe_set_ts_out(g_tuner_id, &stTSOut0);
	    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_set_ts_out failed.\n");
		mt_unf_fe_close(g_tuner_id);
		mt_unf_fe_deinit();
		return ret;
	    }
	}
    }
#endif
#ifdef GET_TUNER1_TSOUT_CONFIG
    {
	if (g_tuner_id == 1) {
	    mt_unf_fe_ts_out_t stTSOut1;
	    GET_TUNER1_TSOUT_CONFIG(stTSOut1);
	    ret = mt_unf_fe_set_ts_out(g_tuner_id, &stTSOut1);
	    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_set_ts_out failed.\n");
		mt_unf_fe_close(g_tuner_id);
		mt_unf_fe_deinit();
		return ret;
	    }
	}
    }
#endif
#ifdef GET_TUNER2_TSOUT_CONFIG
    {
	if (g_tuner_id == 2) {
	    mt_unf_fe_ts_out_t stTSOut2;
	    GET_TUNER2_TSOUT_CONFIG(stTSOut2);
	    ret = mt_unf_fe_set_ts_out(g_tuner_id, &stTSOut2);
	    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_set_ts_out failed.\n");
		mt_unf_fe_close(g_tuner_id);
		mt_unf_fe_deinit();
		return ret;
	    }
	}
    }
#endif
#ifdef GET_TUNER3_TSOUT_CONFIG
    {
	if (g_tuner_id == 3) {
	    mt_unf_fe_ts_out_t stTSOut3;
	    GET_TUNER3_TSOUT_CONFIG(stTSOut3);
	    ret = mt_unf_fe_set_ts_out(g_tuner_id, &stTSOut3);
	    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_set_ts_out failed.\n");
		mt_unf_fe_close(g_tuner_id);
		mt_unf_fe_deinit();
		return ret;
	    }
	}
    }
#endif

    /* connect Tuner*/
    GET_CONNECT_PARA(s_stConnectPara);

    ret = mt_unf_fe_connect(g_tuner_id, &s_stConnectPara, 500);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call mt_unf_fe_connect failed.\n");
    }
    MT_FE_INFO("mt_unf_fe_connect OK.\n");
#endif

#endif

#if 0
    ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DMX_Init failed.\n");
        mt_unf_fe_close(g_tuner_id);
        mt_unf_fe_deinit();
        return ret;
    }

    MT_UNF_DMX_GetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);

    /* For parallel TS */
    PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID;
    PortAttr.u32SerialBitSelector = 0;

#if 0
    /* For serial TS */
    if (MT_UNF_FE_SIG_TYPE_CAB != s_stConnectPara.sig_type)
    {
        /* For serial TS */
        PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_SERIAL;
        PortAttr.u32SerialBitSelector = 0;
    }
#endif

    MT_UNF_DMX_SetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);

    ret = MT_UNF_DMX_AttachTSPort(0, DEFAULT_DVB_PORT);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        mt_unf_fe_close(0);
        mt_unf_fe_deinit();
        return ret;
    }
#endif

    return MT_SUCCESS;
}

mt_s32 dev_deinit()
{
    mt_s32 ret = 0;

#if 0
    ret = MT_UNF_DMX_DetachTSPort(0);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DMX_DetachTSPort failed.\n");
        return ret;
    }

    ret = MT_UNF_DMX_DeInit();
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DMX_AttachTSPort failed.\n");
        return ret;
    }
#endif

    ret = mt_unf_fe_close(g_tuner_id);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call mt_unf_fe_close failed.\n");
	return ret;
    }

    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("call HI_UNF_TUNER_Destroy failed.\n");
	return ret;
    }

#if 0
    ret = MT_UNF_VO_DestroyWindow(phWin);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_VO_DestroyWindow failed.\n");
        return ret;
    }

#if 0
    ret = MT_UNF_VO_Close(MT_UNF_DISPLAY1);
     if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_VO_Close failed.\n");
        return ret;
    }
#endif
    ret = MT_UNF_VO_DeInit();
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_VO_DeInit failed.\n");
        return ret;
    }

    ret = MT_UNF_DISP_Close(MT_UNF_DISPLAY0);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DISP_Close failed.\n");
        return ret;
    }

    ret = MT_UNF_DISP_Close(MT_UNF_DISPLAY1);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DISP_Close failed.\n");
        return ret;
    }

    ret = MT_UNF_DISP_Detach(MT_UNF_DISPLAY0, MT_UNF_DISPLAY1);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DISP_Detach failed.\n");
        return ret;
    }

    ret = MT_UNF_DISP_DeInit();
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DISP_DeInit failed.\n");
        return ret;
    }

    /*aaa hdmiDeInit();*/
    ret = MT_UNF_SND_Close(MT_UNF_SND_0);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_SND_Close failed.\n");
        return ret;
    }

    ret = MT_UNF_SND_DeInit();
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_SND_DeInit failed.\n");
        return ret;
    }

#endif

    mt_sys_deinit();

    return MT_SUCCESS;
}

/*lock freq function*/
mt_s32 mt_fe_connect()
{
	mt_s32 ret = MT_FAILURE;
	//mt_unf_fe_signal_info_t stInfo;
	mt_unf_fe_status_t stTunerStatus;
	mt_u32 u32Loop = 0;
	mt_u32 u32LoopTimes = 100;
	mt_u32 u32Freq = 0;
	mt_u32 u32SymbolRate = 0;
	//mt_char InputCmd[32];

#if 0 //defined (BOARD_TYPE_hi3716mdmo3dvera)
	HI_UNF_DMX_PORT_ATTR_S PortAttr;
#endif

	s_stConnectPara.connect_param.cab.freq = 578000;
	s_stConnectPara.connect_param.cab.sym_rate = 6875000;
	s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64;
	s_stConnectPara.connect_param.cab.band_width = 8;

	ret = mt_unf_fe_connect(g_tuner_id, &s_stConnectPara, 0);
	u32Freq = s_stConnectPara.connect_param.cab.freq;
	u32SymbolRate = s_stConnectPara.connect_param.cab.sym_rate;

	if (MT_SUCCESS == ret)
	{
		for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
		{
			ret = mt_unf_fe_get_status(g_tuner_id, &stTunerStatus);
			if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
			{
#if 0 //defined (BOARD_TYPE_hi3716mdmo3dvera)
				/* For serial TS */
				if (MT_UNF_FE_SIG_TYPE_CAB != s_stConnectPara.sig_type)
				{
					ret = MT_UNF_DMX_GetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);
					PortAttr.u32SerialBitSelector = 0;
					PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_SERIAL;

					ret |= MT_UNF_DMX_SetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);
				}
#endif
				printf("Tuner Lock freq %d symb %d qam%d Success!\n", u32Freq, u32SymbolRate, s_u32CurrentQamType);

				/*automatically play the first program after locked successfully*/
				printf("SUCCESS end\n");

				strcpy(s_acTestResult, MT_RESULT_SUCCESS);

				return MT_SUCCESS;
			}
			else
			{
				MT_USLEEP(10000);
				if (u32Loop % 20 == 0)
					printf("sample line: %d lock_status = %d.\n", __LINE__, stTunerStatus.lock_status);
			}
		}
	}
	else
	{
		printf("Tuner lock freq %d symb %d %dQAM fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, s_u32CurrentQamType, ret);
	}

	if (u32Loop == u32LoopTimes)
	{
		printf("Tuner lock freq %d symb %d  %dQAM Fail!\n", u32Freq, u32SymbolRate, s_u32CurrentQamType);
	}

	printf("FAIL end\n");
	strcpy(s_acTestResult, MT_RESULT_FAIL);

	return MT_FAILURE;
}

mt_s32 mt_fe_connect_test(mt_u32 freq, mt_u32 sym_rate, mt_u32 qam_type, mt_u32 band_width)
{
	mt_s32 ret = MT_FAILURE;
	mt_unf_fe_status_t stTunerStatus;
	mt_u32 u32Loop = 0;
	mt_u32 u32LoopTimes = 100;
	mt_u32 u32Freq = 0;
	mt_u32 u32SymbolRate = 0;

	s_stConnectPara.sig_type = s_u32J83B ? MT_UNF_FE_SIG_TYPE_J83B : MT_UNF_FE_SIG_TYPE_CAB;
	s_stConnectPara.connect_param.cab.freq = freq * 1000;
	s_stConnectPara.connect_param.cab.sym_rate = sym_rate * 1000;
	s_stConnectPara.connect_param.cab.mod_type = qam_type;
	s_stConnectPara.connect_param.cab.band_width = band_width;

	ret = mt_unf_fe_connect(g_tuner_id, &s_stConnectPara, 0);
	u32Freq = s_stConnectPara.connect_param.cab.freq;
	u32SymbolRate = s_stConnectPara.connect_param.cab.sym_rate;

	if (MT_SUCCESS == ret)
	{
		for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop ++)
		{
			ret = mt_unf_fe_get_status(g_tuner_id, &stTunerStatus);
			if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
			{
				printf("Tuner lock freq %d symb %d %dQAM Success!\n", u32Freq, u32SymbolRate, s_u32CurrentQamType);
				/*automatically play the first program after locked successfully*/
				printf("SUCCESS end\n");
				strcpy(s_acTestResult, MT_RESULT_SUCCESS);
#if 0
				mt_u32 s_snr = 0, s_ber = 0, s_strength = 0, s_quality = 0;
				mt_u32 k = 0;
				for(k = 0; k < 10; k++)
				{
					mt_unf_fe_get_snr(g_tuner_id, &s_snr);
					//printf("sample line[%d]\n", __LINE__);
					mt_unf_fe_get_ber(g_tuner_id, &s_ber);
					//printf("sample line[%d]\n", __LINE__);
					mt_unf_fe_get_signal_strength(g_tuner_id, &s_strength);
					//printf("sample line[%d]\n", __LINE__);
					mt_unf_fe_get_signal_quality(g_tuner_id, &s_quality);
					printf("s_snr[%d], s_ber[%d], s_strength[%d], s_quality[%d]\n", s_snr, s_ber, s_strength, s_quality);
					sleep(10);
				}
#endif
				return MT_SUCCESS;
			}
			else
			{
				MT_USLEEP(10000);
#if 0
				if (u32Loop % 20 ==0)
					printf("sample line:%d lock_status = %d.\n", __LINE__, stTunerStatus.lock_status);
#endif
			}
		}
	}
	else
	{
		printf("Tuner lock freq %d symb %d %dQAM Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, s_u32CurrentQamType, ret);
	}

	if (u32Loop == u32LoopTimes)
	{
		printf("Tuner lock freq %d symb %d %dQAM Fail!\n", u32Freq, u32SymbolRate, s_u32CurrentQamType);
	}

	printf("FAIL end\n");
	strcpy(s_acTestResult, MT_RESULT_FAIL);

	return MT_FAILURE;
}

mt_void mt_fe_start(char *locktime)
{
    mt_s32 ret = MT_FAILURE;
    mt_unf_fe_status_t stTunerStatus;
    mt_unf_fe_connect_para_t stTmpConnectPara;
    mt_u32 u32Loop = 0;
    mt_u32 u32StatTime = 0;
    mt_u32 u32EndTime = 0;
    mt_u32 u32TempTime = 0;
    mt_u32 u32IndexLockTime;

    if (MT_NULL_PTR == locktime) {
	MT_FE_ERR("please input loctime count\n");
	return;
    }
    mt_u32 total_check_time = atoi(locktime);

    memcpy(&stTmpConnectPara, &s_stConnectPara, sizeof(mt_unf_fe_connect_para_t));

    s_s32LoopNum = total_check_time;

    s_s32FailTime = 0;
    s_s32Out1Time = 0;
    s_s32OutFailTime = 0;
    s_s32Time = 0;

    ret = printime_init();
    if (MT_FAILURE == ret) {
	MT_FE_ERR("printime_init failure\n");
	return;
    }

    for (u32IndexLockTime = 0; u32IndexLockTime < total_check_time; u32IndexLockTime++) {
	u32StatTime = getcurtime();
	ret = mt_unf_fe_connect(g_tuner_id, &s_stConnectPara, 0);
	if (MT_SUCCESS == ret) {
	    for (u32Loop = 0; u32Loop < 300; u32Loop++) {
		ret = mt_unf_fe_get_status(g_tuner_id, &stTunerStatus);
		if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status) {
		    u32EndTime = getcurtime();
		    u32TempTime = u32EndTime - u32StatTime;
		    if (u32TempTime > 1000) {
			MT_FE_INFO("===111===IndexLockTime=%d,locktime=%d\n", u32IndexLockTime, u32TempTime);
			printtime_file(u32IndexLockTime, u32TempTime, 2);
		    } else {
			MT_FE_INFO("===000===IndexLockTime=%d,locktime=%d\n", u32IndexLockTime, u32TempTime);
			printtime_file(u32IndexLockTime, u32TempTime, 0);
		    }

		    MT_FE_INFO("SUCCESS end\n");
		    strcpy(s_acTestResult, MT_RESULT_SUCCESS);
		    break;
		} else {
		    MT_USLEEP(10000);
		}
	    }
	} else {
	    MT_FE_WARN("Tuner lock freq %d symb %d %dQAM Fail!, ret = 0x%x\n",
	               s_stConnectPara.connect_param.cab.freq,
	               s_stConnectPara.connect_param.cab.sym_rate, s_u32CurrentQamType, ret);
	}

	if (u32Loop == 300) {
	    MT_FE_WARN("Tuner lock freq %d symb %d %dQAM time out , ret = 0x%x\n",
	               s_stConnectPara.connect_param.cab.freq,
	               s_stConnectPara.connect_param.cab.sym_rate, s_u32CurrentQamType, ret);

	    u32EndTime = getcurtime();
	    u32TempTime = u32EndTime - u32StatTime;
	    printtime_file(u32IndexLockTime, u32TempTime, 3);
	}

	if ((u32IndexLockTime % 2) == 0) {

	    stTmpConnectPara.connect_param.cab.freq = s_stConnectPara.connect_param.cab.freq + 8 * 1000;
	    stTmpConnectPara.connect_param.cab.mod_type = s_stConnectPara.connect_param.cab.mod_type;
	    stTmpConnectPara.connect_param.cab.sym_rate = s_stConnectPara.connect_param.cab.sym_rate;

	    mt_unf_fe_connect(g_tuner_id, &stTmpConnectPara, 500);

	    MT_USLEEP(200000);
	}
    }
}

/* set frequency and call mt_fe_connect */
mt_void mt_fe_set_freq(char *freq)
{
	if (freq == MT_NULL_PTR)
	{
		return;
	}

	s_stConnectPara.connect_param.cab.freq = atoi(freq) * 1000;
	//mt_fe_connect();
}

mt_void mt_fe_set_band_width(char *band_width)
{
	if (band_width == MT_NULL_PTR || s_stConnectPara.sig_type == MT_UNF_FE_SIG_TYPE_SAT)
	{
		return;
	}

	s_stConnectPara.connect_param.cab.band_width = atoi(band_width);
	//mt_fe_connect();
}

/* set symbol rate and call mt_fe_connect */
mt_void mt_fe_set_symb(char *symb)
{
	if (symb == MT_NULL_PTR)
	{
		return;
	}

	if (s_stConnectPara.sig_type == MT_UNF_FE_SIG_TYPE_CAB)
	{
		s_stConnectPara.connect_param.cab.sym_rate = atoi(symb);
	}
	else if (s_stConnectPara.sig_type == MT_UNF_FE_SIG_TYPE_SAT)
	{
		s_stConnectPara.connect_param.sat.sym_rate = atoi(symb);
	}
	else
	{
		printf("Error signal type!\n");
	}

	//mt_fe_connect();
}

/* set  type of modulation and call mt_fe_connect */
mt_void mt_fe_set_qam(char *qam)
{
	mt_s32 qam_val;

	if (qam == MT_NULL_PTR)
	{
		return;
	}

	qam_val = atoi(qam);
	s_u32CurrentQamType = qam_val;
	switch (qam_val)
	{
		case 64:
		{
			if (MT_UNF_DEMOD_TYPE_J83B == S_DEMOD_TYPE)
			{
				s_stConnectPara.connect_param.cab.sym_rate = 5057000;
			}
			else
			{
				s_stConnectPara.connect_param.cab.sym_rate = 6875000;
			}

			s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64;
			break;
		}

		case 256:
		{
			if (MT_UNF_DEMOD_TYPE_J83B == S_DEMOD_TYPE)
			{
				s_stConnectPara.connect_param.cab.sym_rate = 5361000;
			}
			else
			{
				s_stConnectPara.connect_param.cab.sym_rate = 6875000;
			}

			s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_256;
			break;
		}

		case 16:
			s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_16;
			break;

		case 32:
			s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_32;
			break;

		case 128:
			s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_128;
			break;

		default:
			s_u32CurrentQamType = 64;
			s_stConnectPara.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64;
			break;
	}

	//mt_fe_connect();
}

mt_void mt_fe_get_msc(char *arg)
{
#if 0
    mt_s32 ret;
    mt_s32 i;
    mt_s32 s32Time;
    //HI_UNF_AVPLAY_STATUS_INFO_S stStatusInfoStart;
    //HI_UNF_AVPLAY_STATUS_INFO_S stStatusInfoEnd;

    /*mt_u32 u32ErrNumStart = 0;
    mt_u32 u32TotalNumStart = 0;
    mt_u32 u32ErrNumEnd = 0;
    mt_u32 u32TotalNumEnd = 0;*/
    mt_u32 u32ErrNum;
    mt_s32 s32TotalErrNum = 0;

    if(arg == 0)
    {
        s32Time = 1;
    }
    else
    {
        sscanf(arg,"%d ",&s32Time);
    }

    /*msc_appear_times = 0;*/
    for(i = 0; i < s32Time; i++)
    {

        ret = HI_UNF_AVPLAY_GetStatusInfo( hAvplay, &stStatusInfoStart);
	//ret =  HI_MPI_VDEC_GetChanStatusInfo(0x260000, &stVdecStatInfoStart );

        sleep(1);

	 ret |= HI_UNF_AVPLAY_GetStatusInfo( hAvplay, &stStatusInfoEnd);
      //  ret |= HI_MPI_VDEC_GetChanStatusInfo(0x260000, &stVdecStatInfoEnd );

		/*u32ErrNum = u32ErrNumEnd - u32ErrNumStart;*/
		//u32ErrNum = stVdecStatInfoEnd.u32TotalErrFrmNum - stVdecStatInfoStart.u32TotalErrFrmNum;
		u32ErrNum = stStatusInfoEnd.u32VidErrorFrameCount - stStatusInfoStart.u32VidErrorFrameCount;
        if(MT_SUCCESS == ret)
        {
            if( u32ErrNum > 0 )
            {
                MT_FE_INFO("---SUCCESS%d end\n", u32ErrNum );
                s32TotalErrNum = s32TotalErrNum + u32ErrNum;
                sprintf(s_acTestResult,MT_RESULT_SUCCESS"%d", u32ErrNum );
                return;
            }
            /*else if( u32TotalNumStart == u32TotalNumEnd )*/
           // else if( stVdecStatInfoStart.u32TotalDecFrmNum == stVdecStatInfoEnd.u32TotalDecFrmNum )
            else if( stStatusInfoStart.u32VidFrameCount == stStatusInfoEnd.u32VidFrameCount )
            {
                MT_FE_ERR("SUCCESS%d end\n",25);
                sprintf(s_acTestResult,MT_RESULT_SUCCESS"%d",25);
                return;
            }
        }
        else
        {
            MT_FE_ERR("HI_VID_GetErrorFrameNum Faild %x!\n",ret);
            MT_FE_ERR("FAIL end\n");
            strcpy(s_acTestResult,MT_RESULT_FAIL);
            return;
        }
    }

    MT_FE_INFO("SUCCESS%d end\n",0);
    sprintf(s_acTestResult,MT_RESULT_SUCCESS"%d",0);
#endif
    return;
}

#if 0
/* not be used temporarily */
mt_void mt_fe_get_msc_ber(char * arg)
{
    mt_s32 ret;
    mt_u32 au32Ber[3];
    mt_u32 au32MskTmp[3];
    HI_DOUBLE dRealBer;
    struct timeval stBtv;
    struct timeval stEtv;
    mt_s32 s32UseTime;
    mt_s32 s32SetTime = 0;
    mt_s32 s32MskNum = 0;
    mt_float fMskBer = 0.0;
    mt_float fAvgBer = 0.0;
    mt_s32 i = 0;

    if(arg == 0)
    {
        s32SetTime = 2;
    }
    else
    {
        sscanf(arg,"%d %f",&s32SetTime,&fMskBer);
    }

    if(fMskBer < 0.0000001)
    {
        fMskBer = s_fMskBer;
    }

    gettimeofday(&stBtv,0);
    while(1)
    {
        ret = mt_unf_fe_get_ber(g_tuner_id, au32Ber);
        if(MT_SUCCESS == ret)
        {
            sprintf((char *)au32MskTmp, "%d.%de-%d", au32Ber[0], au32Ber[1], au32Ber[2]);
            dRealBer = strtod((char *)au32MskTmp, NULL);
#if 0
            u32ber = (ber[0]<<16)|(ber[1]<<8)|ber[2];
            realber = u32ber /8388608.0;
#endif
            /*printf("ber :%f\n", realber);*/
            i++;
            fAvgBer = (fAvgBer * (i - 1) + dRealBer) / i;

            /*printf("HI_TUNER_GetBER ber:%10.6e    avg:%10.6e\n",realber,avgber);*/
            gettimeofday(&stEtv,0);
            s32UseTime = (stEtv.tv_sec - stBtv.tv_sec) * 1000 + (stEtv.tv_usec - stBtv.tv_usec) / 1000;
            if(fAvgBer > fMskBer)
            {
                s32MskNum = (int)(fAvgBer / fMskBer);
                if(s32SetTime > 10)
                {
                    MT_FE_INFO("SUCCESS%d end\n",s32MskNum);
                    break;
                }
            }

            if(s32UseTime >= s32SetTime * 1000)
            {
                MT_FE_INFO("SUCCESS%d end\n",s32MskNum);
                break;
            }
        }
        else
        {
            gettimeofday(&stEtv,0);
            s32UseTime = (stEtv.tv_sec - stBtv.tv_sec) * 1000 + (stEtv.tv_usec - stBtv.tv_usec) /1000;
            if(s32UseTime >= 2000)
            {
                MT_FE_WARN("NOTLOCK end\n");
                break;
            }

            MT_USLEEP(250000);
        }

        /*MT_USLEEP(100000);*/
    }

    return;
}
#endif

mt_void mt_fe_select_port(char *port)
{
#if 0
    mt_s32 ret = 0;
    mt_s32 s32TunerPort;
    mt_unf_fe_attr_t fe_attr;

    if (port == MT_NULL_PTR)
    {
        return;
    }

    s32TunerPort = atoi(port);
    if ((s32TunerPort < 0) || (s32TunerPort > 1))
    {
        MT_FE_ERR("Input Port err %d\n", s32TunerPort);
        return;
    }

    g_tuner_id = s32TunerPort;

    ret = mt_unf_fe_get_default_attr(g_tuner_id, &fe_attr);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("mt_unf_fe_get_attr error \n");
        return;
    }
    fe_attr.enI2cChannel = 3;
    fe_attr.enDemodDevType = HI_UNF_DEMOD_DEV_TYPE_3130I;
    MT_FE_INFO("fe_attr.sig_type = %d\n", fe_attr.sig_type);
    ret = mt_unf_fe_set_attr(g_tuner_id, &fe_attr);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("mt_unf_fe_set_attr error\n");
        return;
    }

    ret = MT_UNF_DMX_AttachTSPort(0, s32TunerPort);
    if (MT_SUCCESS != ret)
    {
        MT_FE_ERR("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        mt_unf_fe_close(0);
        mt_unf_fe_deinit();
        return;
    }
    mt_fe_connect();
#endif
}

mt_void mt_fe_set_type(char *type)
{
	mt_unf_fe_attr_t fe_attr;
	mt_s32 s32TunerType;
	mt_s32 ret = MT_SUCCESS;

	if (type == MT_NULL_PTR)
	{
		return;
	}

	ret = mt_unf_fe_get_attr(g_tuner_id, &fe_attr);
	if (MT_SUCCESS != ret)
		return;

	s32TunerType = atoi(type);

	switch (s32TunerType)
	{
		case 0:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_CD1616;
			break;

		case 1:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_ALPS_TDAE;
			break;

		case 2:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_MT2081;
			break;

		case 3:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_TDCC;
			break;

		case 4:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_TMX7070X;
			break;

		case 5:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_TDA18250;
			break;

		case 6:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_TDA18250B;
			break;

		case 7:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_MXL203;
			break;

		case 8:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_R820C;
			break;

		default:
			fe_attr.tuner_type = MT_UNF_TUNER_TYPE_CD1616;
			break;
	}

	(mt_void) mt_unf_fe_set_attr(g_tuner_id, &fe_attr);
	mt_fe_connect();
}

mt_void mt_fe_get_signal_info(char *para)
{
    mt_s32 ret = 0;
    mt_unf_fe_signal_info_t stInfo;
    mt_u32 u32SNR;
    mt_u32 u32SignalStrength;
    mt_u32 u32SignalQuality;

    ret = mt_unf_fe_get_snr(g_tuner_id, &u32SNR);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("mt_unf_fe_get_snr failed\n");
    }
    MT_FE_ERR("SNR:\t\t\t%d\n", u32SNR);

    ret = mt_unf_fe_get_signal_strength(g_tuner_id, &u32SignalStrength);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("mt_unf_fe_get_signal_strength failed\n");
    }
    MT_FE_INFO("Signal Strength:\t%ddBuv\n", u32SignalStrength);

    if ((MT_UNF_FE_SIG_TYPE_SAT <= s_stConnectPara.sig_type) ||
        (MT_UNF_FE_SIG_TYPE_DTMB >= s_stConnectPara.sig_type)) {
	ret = mt_unf_fe_get_signal_info(g_tuner_id, &stInfo);
	if (ret != MT_SUCCESS) {
	    MT_FE_ERR("call mt_unf_fe_get_signal_info failed.\n");
	    return;
	}
	MT_FE_INFO("Signal type:\t\tCable\n");

	if (MT_UNF_FE_SIG_TYPE_SAT == stInfo.sig_type) {
	    switch (stInfo.sig_info.sat.mode_type) {
	    case MT_UNF_MOD_TYPE_QAM_16:
	    case MT_UNF_MOD_TYPE_QAM_32:
	    case MT_UNF_MOD_TYPE_QAM_64:
	    case MT_UNF_MOD_TYPE_QAM_128:
	    case MT_UNF_MOD_TYPE_QAM_256:
	    case MT_UNF_MOD_TYPE_QAM_512:
		MT_FE_INFO("Modulation type: \tQAM\n");
		break;
	    case MT_UNF_MOD_TYPE_DEFAULT:
	    default:
		MT_FE_INFO("Modulation type: \tUnknown\n");
		break;
	    }

	    switch (stInfo.sig_info.sat.fec_rate) {
	    case MT_UNF_FE_FEC_1_2:
		MT_FE_INFO("FEC rate:\t\t1/2\n");
		break;
	    case MT_UNF_FE_FEC_2_3:
		MT_FE_INFO("FEC rate:\t\t2/3\n");
		break;
	    case MT_UNF_FE_FEC_3_4:
		MT_FE_INFO("FEC rate:\t\t3/4\n");
		break;
	    case MT_UNF_FE_FEC_4_5:
		MT_FE_INFO("FEC rate:\t\t4/5\n");
		break;
	    case MT_UNF_FE_FEC_5_6:
		MT_FE_INFO("FEC rate:\t\t5/6\n");
		break;
	    case MT_UNF_FE_FEC_6_7:
		MT_FE_INFO("FEC rate:\t\t6/7\n");
		break;
	    case MT_UNF_FE_FEC_7_8:
		MT_FE_INFO("FEC rate:\t\t7/8\n");
		break;
	    case MT_UNF_FE_FEC_8_9:
		MT_FE_INFO("FEC rate:\t\t8/9\n");
		break;
	    case MT_UNF_FE_FEC_9_10:
		MT_FE_INFO("FEC rate:\t\t9/10\n");
		break;
	    case MT_UNF_FE_FEC_1_4:
		MT_FE_INFO("FEC rate:\t\t1/4\n");
		break;
	    case MT_UNF_FE_FEC_2_5:
		MT_FE_INFO("FEC rate:\t\t2/5\n");
		break;
	    case MT_UNF_FE_FEC_3_5:
		MT_FE_INFO("FEC rate:\t\t3/5\n");
		break;
	    case MT_UNF_FE_FECRATE_BUTT:
	    case MT_UNF_FE_FEC_AUTO:
	    default:
		MT_FE_INFO("FEC rate:\t\tUnknown\n");
		break;
	    }

	    ret = mt_unf_fe_get_signal_quality(g_tuner_id, &u32SignalQuality);
	    if (MT_SUCCESS != ret) {
		MT_FE_ERR("mt_unf_fe_get_signal_quality failed\n");
	    }
	    MT_FE_INFO("Signal Quality:\t\t%d%%\n", u32SignalQuality);
	} else if (MT_UNF_FE_SIG_TYPE_DVB_T <= stInfo.sig_type && MT_UNF_FE_SIG_TYPE_DTMB >= stInfo.sig_type) {
#if 0
            switch (stInfo.sig_info.ter.enFECRate)
            {
            case MT_UNF_TUNER_FE_FEC_1_2:
                printf("FEC rate:\t\t1/2\n");
                break;
            case MT_UNF_TUNER_FE_FEC_2_3:
                printf("FEC rate:\t\t2/3\n");
                break;
            case MT_UNF_TUNER_FE_FEC_3_4:
                printf("FEC rate:\t\t3/4\n");
                break;
            case MT_UNF_TUNER_FE_FEC_5_6:
                printf("FEC rate:\t\t5/6\n");
                break;
            case MT_UNF_TUNER_FE_FEC_7_8:
                printf("FEC rate:\t\t7/8\n");
                break;
            case MT_UNF_TUNER_FE_FECRATE_BUTT:
            case MT_UNF_TUNER_FE_FEC_AUTO:
            default:
                printf("FEC rate:\t\tUnknown\n");
                break;
            }

            switch (stInfo.unSignalInfo.stTer.enGuardIntv)
            {
            case MT_UNF_TUNER_FE_GUARD_INTV_1_32:
                printf("GI:\t\t1/32\n");
                break;
            case MT_UNF_TUNER_FE_GUARD_INTV_1_16:
                printf("GI:\t\t1/16\n");
                break;
            case MT_UNF_TUNER_FE_GUARD_INTV_1_8:
                printf("GI:\t\t1/8\n");
                break;
            case MT_UNF_TUNER_FE_GUARD_INTV_1_4:
                printf("GI:\t\t1/4\n");
                break;
            default:
                printf("GI:\t\tUnknown\n");
                break;
            }

            switch (stInfo.unSignalInfo.stTer.enModType)
            {
            case MT_UNF_MOD_TYPE_QAM_16:
                printf("ModType:\t\tQAM_16\n");
                break;
            case MT_UNF_MOD_TYPE_QAM_64:
                printf("ModType:\t\tQAM_64\n");
                break;
            default:
                printf("ModType:\t\tUnknown\n");
                break;
            }

            switch (stInfo.unSignalInfo.stTer.enFFTMode)
            {
            case MT_UNF_TUNER_FE_FFT_2K:
                printf("FFTMode:\t\t2K\n");
                break;
            case MT_UNF_TUNER_FE_FFT_4K:
                printf("FFTMode:\t\t4K\n");
                break;
            case MT_UNF_TUNER_FE_FFT_8K:
                printf("FFTMode:\t\t8K\n");
                break;
            default:
                printf("FFTMode:\t\tUnknown\n");
                break;
            }
#endif
	}
    }
}

/* If your diseqc device need config polarization and 22K, you need registe the callback */
mt_void mt_fe_diseqc_set(mt_u32 u32TunerId, mt_unf_fe_polar_t enPolar, mt_unf_fe_lnb_22k_t enLNB22K)
{
}

/* Standby test */
mt_void mt_fe_standby(char *pPara)
{
    mt_u32 u32Para;
    mt_s32 ret = MT_SUCCESS;

    if (pPara == MT_NULL_PTR) {
	return;
    }

    sscanf(pPara, "%d", (mt_s32 *)&u32Para);
    MT_FE_INFO("mt_fe_standby %d\n", u32Para);

    if (0 == u32Para) {
	ret = mt_unf_fe_wakeup(g_tuner_id);
	if (MT_SUCCESS != ret) {
	    MT_FE_ERR("Tuner wake up failed.\n");
	}
    } else {
	ret = mt_unf_fe_standby(g_tuner_id);
	if (MT_SUCCESS != ret) {
	    MT_FE_ERR("Tuner standby failed.\n");
	}
    }
}

mt_void mt_fe_get_offset()
{
    mt_u32 u32Symb;
    mt_u32 u32Freq;
    mt_s32 s32FreqOffset;
    /*mt_u32 au32BER[3] = {0};
    mt_u32 u32SNR = 0;
    mt_u32 u32SignalStrength = 0;*/
    mt_s32 ret = MT_FAILURE;

    ret = mt_unf_fe_get_real_freq_symb(g_tuner_id, &u32Freq, &u32Symb, &s32FreqOffset);
    if (MT_SUCCESS != ret) {
	MT_FE_ERR("HI_UNF_TUNER_GetOffset failed\n");
	return;
    }
    MT_FE_INFO("freq = %d, actul_symb = %d\n", u32Freq, u32Symb);

    /*ret = mt_unf_fe_get_ber(g_tuner_id , au32BER);
    if ( MT_SUCCESS != ret )
    {
        printf("mt_unf_fe_get_ber failed\n");
        return;
    }
    printf("BER :%d.%de-%d\n", au32BER[0], au32BER[1], au32BER[2]);

    ret = mt_unf_fe_get_snr(g_tuner_id, &u32SNR);
    if ( MT_SUCCESS != ret )
    {
        printf("mt_unf_fe_get_snr failed\n");
        return;
    }
    printf("SNR :%d\n", u32SNR);

    ret = mt_unf_fe_get_signal_strength(g_tuner_id, &u32SignalStrength);
    if ( MT_SUCCESS != ret )
    {
        printf("mt_unf_fe_get_signal_strength failed\n");
        return;
    }
    printf("SignalStrength :%d\n", u32SignalStrength);*/

    return;
}

mt_void mt_fe_change_ac_to_b(char *type)
{
	//mt_s32 s32Type = 0;
	char mode[16] = "";
	mt_s32 ret = MT_FAILURE;
	mt_unf_fe_attr_t fe_attr;

	if (type == MT_NULL_PTR)
	{
		printf("pointer null\n");
		return;
	}

	sscanf(type, "%10s ", mode);
	printf("sizeof(mode) = %d, strlen(mode) = %d, mode = %s\n", sizeof(mode), strlen(mode), mode);

	ret = mt_unf_fe_get_attr(g_tuner_id, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		printf("mt_unf_fe_get_attr fail\n");
		return;
	}

	//printf("type = %s \n", type);
	//printf("sizeof(type) = %d, strlen(type) = %d, %d\n", sizeof(type), strlen(type), strcmp(type, "j83b"));
	//if(0 == strcmp(type, "j83b"))
	//if(0 == strncmp(type, "j83b", (strlen(type) - 1)))
	if (0 == strcmp(mode, "j83b"))
	{
		printf("j83b\n");
		fe_attr.demod_dev_type = MT_UNF_DEMOD_TYPE_J83B;
		S_DEMOD_TYPE = MT_UNF_DEMOD_TYPE_J83B;
	}
	else
	{
		printf("j83ac\n");
		fe_attr.demod_dev_type = MT_UNF_DEMOD_TYPE_3130I;
		S_DEMOD_TYPE = MT_UNF_DEMOD_TYPE_3130I;
	}

	ret = mt_unf_fe_set_attr(g_tuner_id, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		printf("mt_unf_fe_set_attr fail\n");
		return;
	}

	return;
}

/*typedef struct
{
    mt_u32      u32Port;
    mt_u32      u32Agc1;
    mt_u32      u32Agc2;
    MT_BOOL     bLockFlag;
    MT_BOOL     bAgcLockFlag;
    MT_U8       u8BagcCtrl12;
    mt_u32      u32Count;
}AGC_TEST_S;

extern mt_s32 MT_UNF_TUNER_TEST_SINGLE_AGC(mt_u32   u32tunerId , AGC_TEST_S  *pstAgcTest);

mt_void mt_tuner_single_agc()
{
    mt_s32 ret = MT_FAILURE;
    AGC_TEST_S stAgcTest = { 0 };


    ret = MT_UNF_TUNER_TEST_SINGLE_AGC(g_tuner_id, &stAgcTest);
    if ( MT_SUCCESS != ret )
    {
        printf("mt_tuner_single_agc failed\n");
        return;
    }

    printf("lock = %d, agc_lock = %d, agc2 = %d,      BAGC_CTRL_12 = %d, end\n",
        stAgcTest.bLockFlag, stAgcTest.bAgcLockFlag , stAgcTest.u32Agc2, stAgcTest.u8BagcCtrl12 );

    return;
}*/

/*help function*/
mt_void mt_showhelp(char *pCmd)
{
    mt_u32 u32Loop = 0;
    mt_u32 u32CmdNum = sizeof(g_cmdhelp) / sizeof(CMD_HELP_S);

    if (pCmd == MT_NULL_PTR) {
	MT_FE_INFO("command list:\n");
	for (u32Loop = 0; u32Loop < u32CmdNum; u32Loop++) {
	    MT_FE_INFO("%s:%s\n", g_cmdhelp[u32Loop].name, g_cmdhelp[u32Loop].help_info);
	}
	return;
    }

    for (u32Loop = 0; u32Loop < u32CmdNum; u32Loop++) {
	if (0 == strncmp(pCmd, g_cmdhelp[u32Loop].name, strlen(g_cmdhelp[u32Loop].name))) {
	    MT_FE_INFO("%s", g_cmdhelp[u32Loop].help_info);
	}
    }
}

/* set of received command */
TEST_FUNC_S g_testfunc[] =
    {
        { "setfreq", mt_fe_set_freq },
        { "setbw", mt_fe_set_band_width },
        { "setsymb", mt_fe_set_symb },
        { "setqam", mt_fe_set_qam },
        { "getsignalinfo", mt_fe_get_signal_info },
        //        { "play", mt_fe_play },
        { "getmsc", mt_fe_get_msc },
        //{"getber",        mt_fe_get_msc_ber},
        { "select", mt_fe_select_port },
        { "settype", mt_fe_set_type },
        { "getoffset", mt_fe_get_offset },
        { "start", mt_fe_start },
        { "setmode", mt_fe_change_ac_to_b },
        { "standby", mt_fe_standby },
        { "help", mt_showhelp }
    };

/*search the handle function corresponding with the command in the character string */
Test_Func_Proc getFunbyName(char *name)
{
    mt_u32 u32Loop;
    mt_u32 u32FunNum = sizeof(g_testfunc) / sizeof(TEST_FUNC_S);

    if (MT_NULL_PTR == name) {
	return MT_NULL_PTR;
    }

    for (u32Loop = 0; u32Loop < u32FunNum; u32Loop++) {
	if (0 == strncmp(name, g_testfunc[u32Loop].name, strlen(g_testfunc[u32Loop].name))) {
	    return g_testfunc[u32Loop].proc;
	    MT_FE_WARN("here line:%d\n", __LINE__);
	}
    }

    return MT_NULL_PTR;
}

/*deals with the universal command */
mt_s32 procfun(char *funargs)
{
    char *argstr;
    Test_Func_Proc func;

    argstr = strchr(funargs, UDP_STR_SPLIT);
    if (MT_NULL_PTR != argstr) {
	*argstr = 0;
	argstr += 1;
    }

    func = getFunbyName(funargs);
    if (MT_NULL_PTR != func) {
	func(argstr);
	return MT_SUCCESS;
    } else {
	strcpy(s_acTestResult, MT_RESULT_FAIL " Can't find the function\n");
	MT_FE_ERR("Can't find the function  %s %s\n\n", funargs, argstr);
	return MT_FAILURE;
    }
}

int TestTCPOpen()
{
    mt_s32 s32SockFd = -1;
    struct sockaddr_in stAddr;
    s32SockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (s32SockFd < 0) {
	MT_FE_ERR("Socket error...\n");
	return -1;
    }

    memset(&stAddr, 0, sizeof(stAddr));
    stAddr.sin_family = AF_INET;
    stAddr.sin_addr.s_addr = INADDR_ANY;
    stAddr.sin_port = htons(DEFAULT_PORT);
    if (bind(s32SockFd, (struct sockaddr *)&stAddr, sizeof(struct sockaddr)) < 0) {
	close(s32SockFd);
	MT_FE_ERR("Bind error...\n");
	return -1;
    }

    listen(s32SockFd, 5);

    return s32SockFd;
}

void *tcprcv(void *arg)
{
    struct sockaddr_in stCliAddr;
    mt_s32 s32NewFd;
    mt_s32 s32Size;
    mt_s32 s32RcvLength;
    mt_char acRecvBuf[MAX_CMD_BUFFER_LEN];
    mt_s32 s32Socketd = TestTCPOpen();
    mt_s32 ret;
    if (s32Socketd < 0) {
	return (void *)0;
    }
    s32Size = sizeof(stCliAddr);
    if ((s32NewFd = accept(s32Socketd, (struct sockaddr *)&stCliAddr, (socklen_t *)&s32Size)) < 0) {
	MT_FE_ERR("accept err\n");
	close(s32Socketd);
	return (void *)1;
    }
    MT_FE_INFO("accept socket %d\n", s32NewFd);

    while (1) {
	/*
     * error: call to '__read_dest_size_error' declared with attribute
     * error: read called with size bigger than destination
     */
	//s32RcvLength = read(s32NewFd, acRecvBuf, 1500);
	s32RcvLength = read(s32NewFd, acRecvBuf, MAX_CMD_BUFFER_LEN);
	if (s32RcvLength > 0) {
	    acRecvBuf[s32RcvLength] = 0;
	    MT_FE_INFO("receive cmd: %s\n", acRecvBuf);
	    ret = procfun(acRecvBuf);

	    s_acTestResult[strlen(s_acTestResult)] = '\n';

	    ret = send(s32NewFd, s_acTestResult, strlen(s_acTestResult), 0);
	    if (ret < 0) {
		MT_FE_WARN("send err:%s\n", strerror(errno));
	    }
	    MT_FE_INFO("%s result:%s\n", acRecvBuf, s_acTestResult);

	    memset(s_acTestResult, 0, sizeof(s_acTestResult));
	    /*fflush(newfd);*/
	}
    }
}

mt_s32 main(mt_s32 argc, char *argv[])
{
    mt_s32 ret = MT_FAILURE;
    mt_char acRecvBuf[MAX_CMD_BUFFER_LEN];
    mt_s32 s32Threadd;

    mt_u32 freq = 0;
    mt_u32 symbol_rate = 0;
    mt_u32 qam_type = 0;
    mt_u32 qam_mode = 0;
    mt_u32 band_width = 8;
	mt_u32 be_j83b = 0;

    /* init device */
    ret = dev_init();
    if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, ret);
		return ret;
    }

    freq = strtol(argv[1], NULL, 0);
    symbol_rate = strtol(argv[2], NULL, 0);
    qam_mode = strtol(argv[3], NULL, 0);
    band_width = strtol(argv[4], NULL, 0);
	be_j83b = strtol(argv[5], NULL, 0);

    if (band_width == 0)
		band_width = 8;

    if (qam_mode == 16)
		qam_type = MT_UNF_MOD_TYPE_QAM_16;
    else if (qam_mode == 32)
		qam_type = MT_UNF_MOD_TYPE_QAM_32;
    else if (qam_mode == 64)
		qam_type = MT_UNF_MOD_TYPE_QAM_64;
    else if (qam_mode == 128)
		qam_type = MT_UNF_MOD_TYPE_QAM_128;
    else if (qam_mode == 256)
		qam_type = MT_UNF_MOD_TYPE_QAM_256;
    else if (qam_mode == 512)
		qam_type = MT_UNF_MOD_TYPE_QAM_512;

    s_u32CurrentQamType = qam_mode;

	s_u32J83B = be_j83b;

    ret = mt_fe_connect_test(freq, symbol_rate, qam_type, band_width);

/*there are in Ret = MTADP_Snd_Init()*/
/*AudioLineOutMuteCntrDisable();
	    AudioSPDIFOutSharedEnable();*/
#if 0
    /* init avplay */
    ret = avplay_Init();
    if(MT_SUCCESS != ret)
    {
        MT_FE_ERR("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, ret);
        return ret;
    }
#endif

#if 0
    pthread_create((pthread_t *)&s32Threadd, 0, tcprcv, 0);

    /*print help*/
    mt_showhelp(MT_NULL);

    /* recieve command */
    while (1)
	{
		SAMPLE_GET_INPUTCMD(acRecvBuf);
		if (strlen(acRecvBuf) < 3)
		{
			MT_USLEEP(10000);
			continue;
		}

		if (strncmp(acRecvBuf, "exit", 4) == 0)
		{
			break;
		}

		procfun(acRecvBuf);
    }

    /* deinit device */
    //avplay_deinit();
#endif
    dev_deinit();

    return ret;
}

#endif
