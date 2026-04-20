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
#include "mt_unf_frontend.h"

#define MT_FE_SIGNAL_TYPE (2)

#ifdef CONFIG_SUPPORT_CA_RELEASE
#define MT_FE_ERR(format, arg...)
#define MT_FE_INFO(format, arg...)
#define MT_FE_WARN(format, arg...)
#else
#define MT_FE_ERR(format, arg...) printf("[ERR]: %s,%d: " format, __FUNCTION__, __LINE__, ##arg)
#define MT_FE_INFO(format, arg...) printf(format, ##arg)
#define MT_FE_WARN(format, arg...) printf("[WARNING]: %s,%d: " format, __FUNCTION__, __LINE__, ##arg)
#endif

#if (MT_FE_SIGNAL_TYPE == 1)
#define GET_CONNECT_PARA(fe_connect_param)                                \
{                                                                         \
	fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_CAB;                   \
	fe_connect_param.connect_param.cab.b_reverse = 0;                     \
	fe_connect_param.connect_param.cab.freq = 474000;                     \
	fe_connect_param.connect_param.cab.sym_rate = 6875000;                \
	fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64; \
	fe_connect_param.connect_param.cab.band_width = 8;                    \
}
#elif (MT_FE_SIGNAL_TYPE == 2)
#define GET_CONNECT_PARA(fe_connect_param)                                  \
{                                                                           \
	fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;                     \
	fe_connect_param.connect_param.sat.freq = 3840000;                      \
	fe_connect_param.connect_param.sat.sym_rate = 27500000;                 \
	fe_connect_param.connect_param.sat.en_polar = MT_UNF_FE_POLARIZATION_H; \
}
#elif (MT_FE_SIGNAL_TYPE == 4)
#define GET_CONNECT_PARA(fe_connect_param)                \
{                                                         \
	fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T; \
	fe_connect_param.connect_para.ter.b_reverse = 0;      \
	fe_connect_param.connect_para.ter.freq = 682000;      \
	fe_connect_param.connect_para.ter.band_width = 8000;  \
}
#endif

mt_unf_demod_type_t g_demod_type = MT_UNF_DEMOD_TYPE_DS3103;

typedef mt_void (*test_func_proc)(char *args);

typedef struct _test_func_s
{
	mt_char *name;
	test_func_proc proc;
} test_func_s;

typedef struct _cmd_help_t
{
	mt_char name[30];
	mt_char help_info[3000];
} cmd_help_t;

typedef struct _t2_multi_group
{
	mt_u32 grp_id;
	mt_u32 com_id;
	mt_u32 combination;
} t2_multi_group;

cmd_help_t g_cmdhelp[] = 
{
	{"help", "help: show help menu, only input 'help' will print command list,\n"
			 "\t 'help cmd' will print the help infomation of the cmd.\n"},
	{"getsignalinfo", "getsignalinfo: get detailed infomation of current locked signal.[FOR satellite and terrestrial signal].\n"},
#if (MT_FE_SIGNAL_TYPE == 2)
	{"setlnbpower", "setlnbpower 0: set LNB power.[FOR DVB-S/S2]\n"
					"\t 0 Power off,  1 Power auto(13V/18V),  2 Enhanced(14V/19V).\n"},
	{"setlnb", "setlnb 1 5150 5750 0: set LNB band and Low/High Local Oscillator.[FOR DVB-S/S2]\n"
			   "\t Param1: LNB type:0 single LO, 1 dual LO.\n"
			   "\t Param2: LNB low LO: MHz, e.g.5150.\n"
			   "\t Param3: LNB low LO: MHz, e.g.5750.\n"
			   "\t Param4: LNB band:0 C, 1 Ku.\n"},
	{"blindscan", "blindscan 0 [0] [0] [950000] [2150000]: Blind scan. [FOR DVB-S/S2]\n"
				  "\t Param1: blind scan type: 0 Auto, 1 Manual.\n"
				  "\t Param2: LNB Polarization: 0 H, 1 V. Only for manual type.\n"
				  "\t Param3: LNB 22K:0 Off, 1 On. Only for manual type.\n"
				  "\t Param4: Start frequency. Only for manual type.\n"
				  "\t Param5: Stop frequency. Only for manual type.\n"},
	{"bsstop", "bsstop: Stop blind scan. [FOR DVB-S/S2]\n"},
#ifdef CONFIG_MT_DISEQC_SUPPORT
	{"switch", "switch 0 0 [0] [0]: Switch test. [FOR DVB-S/S2]\n"
			   "\t Param1: Switch type:0 0/12V, 1 Tone burst, 2 22K, 3 DiSEqC 1.0, 4 DiSEqC 1.1, 5 Reset, 6 Standby, 7 WakeUp.\n"
			   "\t Param2: Switch port:0 None, 1-16.\n"
			   "\t Param3: LNB Polarization: 0 H, 1 V.\n"
			   "\t Param4: LNB 22K:0 Off, 1 On.\n"},
	{"motor", "motor 0 [0]: DiSEqC motor test. [FOR DVB-S/S2]\n"
			  "\t Param1: Control type:0 StorePos, 1 GotoPos, 2 SetLimit, 3 Move, 4 Stop, 5 USALS, 6 Recalculate, 7 GotoAng.\n"
			  "\t StorePos: Param2:position(0-255).\n"
			  "\t GotoPos: Param2:position(0-255).\n"
			  "\t SetLimit: Param2:limit type(0 off, 1 east, 2 west).\n"
			  "\t Move: Param2:move direction(0 east 1 west ), Param3:move type(0 slow, 1 fast, 2 continus).\n"
			  "\t USALS: Param2:local longitude(0-3600), Param3:local latitude(0-1800), Param4:satellite longitude(0-3600).\n"
			  "\t Recalculate: Param2/Param3/Param4: parameter1/2/3 for recalculate.\n"
			  "\t GotoAng: Param2:angle.\n"},
	{"unic", "unic 0:unicable test.[FOR DVB-S/S2]\n"
			 "\t Param1:0 unicable power off,1 unicable power on,2 unicable config,3 unicable lofreq.\n"},
#endif
#endif
	{"select", "select 0: select inside tuner or outside\n"},
	{"play", "play 513 660 [vcodec]: set video pid 513 and audio pid 660\n"
			 "\t vcodec=0 mpeg2,vcodec=1 mpeg4,vcodec=4 h264\n"},
	{"getmsc", "getmsc: get mosaic num ,arg type: int time,float berlimit\n"},
	{"getber", "getber: get ber\n"},
	{"settype", "settype 0: set tuner type. \n"
				"\t 0  cd1616,  1  tdae3,  2  mt2081,  3  tdcc,   4  tmx7070x,\n"
				"\t 5  tda18250, 6  tda18250b 7  mxl203, 8  r820c\n"},
	{"setchnl", "setchnl 3840 27500000 0 (freqency/symbolrate/ploar),\n"
				"\t freqency unit MHz for cable and satellite, KHz for terrestrial. If test DVB S/S2, you should input downlink frequency here,\n"
				"\t symbolrate unit baud for satellite,\n"
				"\t polar 0:Horizontal 1:Vertical 2:Left-hand circular 3:Right-hand circular.\n"},
	{"setqam", "setqam  64: set qam type , 64 means 64qam etc.\n"},
	{"getoffset", "getoffset : getfreq getsyb. \n"},
	{"start", "start  500: lock 500 times --->time_file\n"},
	{"setmode", "setmode j83b/j83ac:change mode to j83b or j83ac\n"},
	{"exit", "exit    : exit the sample\n"}
};

#define MAX_CMD_BUFFER_LEN 256
#define UDP_STR_SPLIT ' '
#define TEST_ARGS_SPLIT " "
#define MT_RESULT_SUCCESS "SUCCESS"
#define MT_RESULT_FAIL "FAIL"
#define DEFAULT_PORT 1234

#ifdef FPGA
static mt_s32 g_fe_port = 0;
#else
static mt_s32 g_fe_port = 0;      /* use dvbc */
static mt_s32 g_fe_dvbs_port = 0; /* use dvbc */
#endif

/*static mt_s32 g_fe_port = 1;*/
/*static mt_u32 s_u32TunerFreq = 403000;*/
/*static mt_u32 s_u32ErrorNum;*/
/*static HI_FLOAT s_fMskBer = 0.0014;*/
/*static mt_u8 s_au8MskTmp[64];*/
static mt_u32 g_current_qam_type;

/*save results, then send client*/
char g_test_result[MAX_CMD_BUFFER_LEN];

static mt_unf_fe_connect_para_t g_fe_connect_param;
#if 0
static HI_UNF_ENC_FMT_E s_enDefaultFmt = HI_UNF_ENC_FMT_1080i_50;

HI_HANDLE phWin;
HI_HANDLE hAvplay;

HI_HANDLE hTrack;
#endif
FILE *fp = NULL;
static mt_s32 s_s32LoopNum = 0;

static mt_s32 s_s32FailTime = 0;
static mt_s32 s_s32Out1Time = 0;
static mt_s32 s_s32OutFailTime = 0;
static mt_s32 s_s32Time = 0;
static MT_BOOL g_fe_bRun = MT_TRUE;

#define FE_MAX_CMDLINE_LEN 1280
#define FE_MAX_ARGS_COUNT 10
#define FE_MAX_ARGS_LEN 128
static mt_char g_fe_cmd_line[FE_MAX_CMDLINE_LEN];
static mt_s32 g_fe_argc;
static mt_char g_fe_argv[FE_MAX_ARGS_COUNT][FE_MAX_ARGS_LEN];
//static MT_BOOL g_bRun = MT_TRUE;
//static mt_u32 g_avsync_mode = 0;

mt_s32 printime_init()
{
	FILE *time_file = NULL;

	time_file = fopen("time_file", "wt");
	if (NULL == time_file)
	{
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

	if (NULL == time_file)
	{
		MT_FE_INFO("open time_file,line = %d\n", __LINE__);
	}

	if (isfail == 1)
	{
		/*fprintf(time_file, "FAIL: locknum = %d,locktime = %d   isfail = %d\n", locknum, locktime,isfail);*/
		fprintf(time_file, "%d       isfail = %d\n", locktime, isfail);
		s_s32Time = s_s32Time + locktime;
		s_s32FailTime++;
	}
	else if (isfail == 2)
	{
		s_s32Out1Time++;
		fprintf(time_file, "%d      isfail = %d\n", locktime, isfail);
		s_s32Time = s_s32Time + locktime;
	}
	else if (isfail == 3)
	{
		s_s32OutFailTime++;
		if (locktime > 1000)
		{
			s_s32Out1Time++;
		}
		fprintf(time_file, "%d      isfail = %d\n", locktime, isfail);
		s_s32Time = s_s32Time + locktime;
	}
	else
	{
		/*fprintf(time_file, "OK: locknum = %d,locktime = %d,isfail = %d\n", locknum, locktime,isfail);*/
		fprintf(time_file, "%d   isfail = %d\n", locktime, isfail);
		s_s32Time = s_s32Time + locktime;
	}

	if (locknum + 1 >= s_s32LoopNum)
	{
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
	return;
}

//#define BOARD_TYPE_hi3716mdmo3fvera

mt_s32 dev_init()
{
	mt_s32 ret = 0;
	mt_unf_fe_attr_t fe_attr;
	//MT_UNF_DMX_PORT_ATTR_S PortAttr;

#ifdef CHIP
	set_pin_mux(fe_attr);
#endif

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
	ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
	ret |= HIADP_VO_CreatWin(MT_NULL, &phWin);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call MTADP_VO_Init failed.\n");
		MTADP_VO_DeInit();
		return ret;
	}
#endif

#if 1
	ret = mt_unf_fe_init();
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_init failed.\n");
		return ret;
	}

	/* open Tuner */
	ret = mt_unf_fe_open(g_fe_dvbs_port);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_open failed.\n");
		mt_unf_fe_deinit();
		return ret;
	}

	/* get default attribute */
	ret = mt_unf_fe_get_default_attr(g_fe_dvbs_port, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_get_default_attr failed.\n");
		mt_unf_fe_close(g_fe_dvbs_port);
		mt_unf_fe_deinit();
		return ret;
	}

#if 0
	//fe_attr.fe_config.pin_config.port = 0;
	fe_attr.fe_config.pin_config.lock_indicate = 0;
	fe_attr.fe_config.pin_config.vsel_when_13v = 0;
	fe_attr.fe_config.pin_config.vsel_when_lnb_off = 0;
	fe_attr.fe_config.pin_config.diseqc_out_when_lnb_off = 0;
	fe_attr.fe_config.pin_config.lnb_enable = 0;
	fe_attr.fe_config.pin_config.lnb_prot_level = 0;
	fe_attr.fe_config.pin_config.lnb_enable_by_mcu = 0;
	fe_attr.fe_config.pin_config.lnb_prot_by_mcu = 0;
	fe_attr.fe_config.pin_config.lnb_enable_pin = 0;
	fe_attr.fe_config.pin_config.lnb_prot_pin = 0;
	fe_attr.fe_config.pin_config.lock_pin_by_mcu = 0;
	fe_attr.fe_config.pin_config.lock_pin = 0;
	fe_attr.fe_config.pin_config.lnb_vol_pin_by_mcu = 0;
	fe_attr.fe_config.pin_config.lnb_vol_pin = 0;
	fe_attr.fe_config.pin_config.demod_reset_pin = 0;
	fe_attr.fe_config.pin_config.lnb_vol_pin_mode = 0;
	//fe_attr.fe_config.pin_config.reset_gpio_no = 0;
	//fe_attr.fe_config.pin_config.use_unicable = 0;
#endif

	/* need to check */
	//MTADP_FE_GET_CONFIG(g_fe_dvbs_port, fe_attr);
	fe_attr.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
	ret = mt_unf_fe_set_attr(g_fe_dvbs_port, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
		return ret;
	}

#ifdef GET_SAT_TUNER_CONFIG
	{
		mt_unf_fe_sat_attr_t sat_fe_attr;
		GET_SAT_TUNER_CONFIG(g_fe_dvbs_port, sat_fe_attr);
		ret = mt_unf_fe_set_sat_attr(g_fe_dvbs_port, &sat_fe_attr);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
			return ret;
		}
	}
#endif

#ifdef GET_SAT_TUNER1_CONFIG
	{
		mt_unf_fe_sat_attr_t sat_fe_attr;
		GET_SAT_TUNER1_CONFIG(g_fe_port, sat_fe_attr);
		ret = mt_unf_fe_set_sat_attr(g_fe_port, &sat_fe_attr);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
			return ret;
		}
	}
#endif

#ifdef GET_SAT_TUNER2_CONFIG
	{
		mt_unf_fe_sat_attr_t sat_fe_attr;
		GET_SAT_TUNER2_CONFIG(g_fe_port, sat_fe_attr);
		ret = mt_unf_fe_set_sat_attr(g_fe_port, &sat_fe_attr);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
			return ret;
		}
	}
#endif

#ifdef GET_SAT_TUNER3_CONFIG
	{
		mt_unf_fe_sat_attr_t sat_fe_attr;
		GET_SAT_TUNER2_CONFIG(g_fe_port, sat_fe_attr);
		ret = mt_unf_fe_set_sat_attr(g_fe_port, &sat_fe_attr);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
			return ret;
		}
	}
#endif

#ifdef GET_TER_TUNER_CONFIG
	{
		mt_unf_fe_ter_attr_t ter_fe_attr;
		GET_TER_TUNER_CONFIG(g_fe_port, ter_fe_attr);
		ret = mt_unf_fe_set_ter_attr(g_fe_port, &ter_fe_attr);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("call mt_unf_fe_set_ter_attr failed.\n");
			return ret;
		}
	}
#endif

#if (MT_TUNER_TYPE == 22) //si2147
	mt_unf_fe_tsout_set_t fe_ts_out;
	fe_ts_out.ts_output[0] = MT_UNF_FE_OUTPUT_TSDAT7;
	fe_ts_out.ts_output[1] = MT_UNF_FE_OUTPUT_TSDAT6;
	fe_ts_out.ts_output[2] = MT_UNF_FE_OUTPUT_TSDAT5;
	fe_ts_out.ts_output[3] = MT_UNF_FE_OUTPUT_TSDAT4;
	fe_ts_out.ts_output[4] = MT_UNF_FE_OUTPUT_TSDAT2;
	fe_ts_out.ts_output[5] = MT_UNF_FE_OUTPUT_TSDAT3;
	fe_ts_out.ts_output[6] = MT_UNF_FE_OUTPUT_TSDAT0;
	fe_ts_out.ts_output[7] = MT_UNF_FE_OUTPUT_TSDAT1;
	fe_ts_out.ts_output[8] = MT_UNF_FE_OUTPUT_TSSYNC;
	fe_ts_out.ts_output[9] = MT_UNF_FE_OUTPUT_TSVLD;
	fe_ts_out.ts_output[10] = MT_UNF_FE_OUTPUT_TSERR;

	ret = mt_unf_fe_set_ts_out(g_fe_port, &fe_ts_out);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_set_ts_out failed.\n");
		return ret;
	}
#endif

#if 0
	mt_unf_fe_extra_cmd_t fe_extra_cmd;
	memset(fe_extra_cmd, 0, sizeof(mt_unf_fe_extra_cmd_t));
	fe_extra_cmd->data[0] = 0;
	fe_extra_cmd->data[1] = 0;
	mt_unf_fe_set_onff22k_attr(g_fe_port, &fe_extra_cmd)
	memset(fe_extra_cmd, 0, sizeof(mt_unf_fe_extra_cmd_t));
	fe_extra_cmd->data[0] = 0;
	mt_unf_fe_set_lnb_onff_attr(g_fe_port, &fe_extra_cmd);

	/* connect Tuner*/
	GET_CONNECT_PARA(g_fe_connect_param);
#endif

#if 0
	/* If satellite signal, maybe need config lnb power, switch, motor */
	if (MT_UNF_FE_SIG_TYPE_SAT == g_fe_connect_param.sig_type)
	{
		mt_unf_fe_lnb_power_t en_power = MT_UNF_FE_LNB_POWER_ON;
		mt_unf_fe_lnb_config_t lnb_config;

		/* Set LNB power on/off/enhanced */
		ret = mt_unf_fe_set_lnb_power(g_fe_port, en_power);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("call mt_unf_fe_set_lnb_power failed.\n");
		}

		/* Before connect or blindscan, you need config LNB */
		lnb_config.lnb_type = MT_UNF_FE_LNB_SINGLE_FREQUENCY;//MT_UNF_FE_LNB_DUAL_FREQUENCY;
		lnb_config.low_lo = 5150;
		lnb_config.high_lo = 0;//5750;
		lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
		ret = mt_unf_fe_set_lnb_config(g_fe_port, &lnb_config);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("Set LNB config failed.\n");
		}
	}
#endif

	/*
	ret = mt_unf_fe_connect(g_fe_port, &g_fe_connect_param, 500);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_connect failed.\n");
	}

	MT_FE_INFO("mt_unf_fe_connect OK.\n");
	*/

#endif

#if 0
	ret = MT_UNF_DMX_Init();
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call MT_UNF_DMX_Init failed.\n");
		mt_unf_fe_close(g_fe_port);
		mt_unf_fe_deinit();
		return ret;
	}

	MT_UNF_DMX_GetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);

	/* For parallel TS */
	PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID;
	//PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_SERIAL2BIT;
	PortAttr.u32SerialBitSelector = 0;

#if defined(BOARD_TYPE_hi3716mdmo3dvera)
	/* For serial TS */
	if (MT_UNF_FE_SIG_TYPE_CAB != g_fe_connect_param.sig_type)
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

	ret = mt_unf_fe_close(g_fe_dvbs_port);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call mt_unf_fe_close failed.\n");
		return ret;
	}

	ret = mt_unf_fe_deinit();
	if (MT_SUCCESS != ret)
	{
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
#if 0
	mt_s32 ret = MT_FAILURE;
	//mt_unf_fe_signal_info_t stInfo;
	mt_unf_fe_status_t fe_status;
	mt_u32 i_loop = 0;
	mt_u32 i_loop_times = 100;
	mt_u32 freq = 0;
	mt_u32 sym_rate = 0;
	//mt_u32 modulation = 0;
	mt_u32 band_width = 0;
	mt_u8 plpnum = 0;
	mt_unf_fe_t2_plp_type_t plptype = MT_UNF_FE_T2_PLP_TYPE_BUTT;
	mt_unf_fe_ter_channel_attr_t tp_array[200];
	mt_unf_fe_ter_plp_attr_t plp_attr[16], plp_buf;
	mt_u8 i = 0, j, k, plpid = 0, grp_id = 0, grp_num = 0, cnt = 0, comm_exist = 0;
	t2_multi_group t2_plp_array[10];

#if 0
	MT_UNF_DMX_PORT_ATTR_S PortAttr;
#endif

	ret = mt_unf_fe_connect(g_fe_port, &g_fe_connect_param, 0);

	switch (g_fe_connect_param.sig_type)
	{
		case MT_UNF_FE_SIG_TYPE_CAB:
			freq = g_fe_connect_param.connect_param.cab.freq;
			sym_rate = g_fe_connect_param.connect_param.cab.sym_rate;
			break;

		case MT_UNF_FE_SIG_TYPE_SAT:
			freq = g_fe_connect_param.connect_param.sat.freq;
			sym_rate = g_fe_connect_param.connect_param.sat.sym_rate;
			if (sym_rate < 5000000)
				i_loop_times = 1000;
			else if (sym_rate < 10000000)
				i_loop_times = 120;
			else
				i_loop_times = 100;
			break;

		case MT_UNF_FE_SIG_TYPE_DVB_T:
		case MT_UNF_FE_SIG_TYPE_DVB_T2:
		case MT_UNF_FE_SIG_TYPE_ISDB_T:
		case MT_UNF_FE_SIG_TYPE_ATSC_T:
		case MT_UNF_FE_SIG_TYPE_DTMB:
			freq = g_fe_connect_param.connect_param.ter.freq;
			band_width = g_fe_connect_param.connect_param.ter.band_width / 1000;
#if defined(DVBT_IT9133) || defined(ISDBT_IT9170)
			i_loop_times = 1;
#endif
			break;

		default:
			return MT_SUCCESS;
	}

	if (MT_SUCCESS == ret)
	{
		for (i_loop = 0; i_loop < i_loop_times; i_loop++)
		{
			ret = mt_unf_fe_get_status(g_fe_port, &fe_status);
			if (MT_UNF_FE_SIGNAL_LOCKED == fe_status.lock_status)
			{
#if 0
				/* For serial TS */
				if (MT_UNF_FE_SIG_TYPE_CAB != g_fe_connect_param.sig_type)
				{
					ret = HI_UNF_DMX_GetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);
					PortAttr.u32SerialBitSelector = 0;
					PortAttr.enPortType = HI_UNF_DMX_PORT_TYPE_SERIAL;

					ret |= HI_UNF_DMX_SetTSPortAttr(DEFAULT_DVB_PORT, &PortAttr);
				}
#endif
				switch (g_fe_connect_param.sig_type)
				{
					case MT_UNF_FE_SIG_TYPE_CAB:
						printf("Tuner   Lock freq %d symb %d  qam%d Success!\n",
							   freq, sym_rate, g_current_qam_type);

						/*automatically play the first program after locked successfully*/
						printf("SUCCESS end\n");
						strcpy(g_test_result, MT_RESULT_SUCCESS);
						return MT_SUCCESS;

					case MT_UNF_FE_SIG_TYPE_SAT:
						printf("Tuner   Lock freq %d symb %d Success!\n",
							   freq, sym_rate);

						/*automatically play the first program after locked successfully*/
						//printf("SUCCESS end\n");
						strcpy(g_test_result, MT_RESULT_SUCCESS);
						return MT_SUCCESS;

					case MT_UNF_FE_SIG_TYPE_DVB_T:
					case MT_UNF_FE_SIG_TYPE_DVB_T2:
					case MT_UNF_FE_SIG_TYPE_ISDB_T:
					case MT_UNF_FE_SIG_TYPE_ATSC_T:
					case MT_UNF_FE_SIG_TYPE_DTMB:
						printf("Tuner   Lock freq %d bandwidth %d Success!\n",
							   freq, band_width);

						/*automatically play the first program after locked successfully*/
						strcpy(g_test_result, MT_RESULT_SUCCESS);

						if (MT_UNF_FE_SIG_TYPE_DVB_T2 == g_fe_connect_param.sig_type)
						{
							ret = mt_unf_fe_get_plpnum(g_fe_port, &plpnum);
							if (MT_SUCCESS != ret)
							{
								MT_FE_ERR("HI_UNF_TUNER_GetPLPNum error.\n");
								return MT_FAILURE;
							}

							for (i = 0; i < plpnum; i++)
							{
								mt_unf_fe_set_plp_mode(g_fe_port, 1);
								ret = mt_unf_fe_set_plp_id(g_fe_port, i);
								if (MT_SUCCESS != ret)
								{
									MT_FE_ERR("mt_unf_fe_set_pip_id error.\n");
								}

								ret = mt_unf_fe_get_current_plp_type(g_fe_port, &plptype);
								if (MT_SUCCESS != ret)
								{
									MT_FE_ERR("mt_unf_fe_get_current_plp_type error.\n");
								}
								ret = mt_unf_fe_get_plpid(g_fe_port, &plpid);
								if (MT_SUCCESS != ret)
								{
									MT_FE_ERR("mt_unf_fe_get_plpid error.\n");
								}
								ret = mt_unf_fe_get_plp_grpid(g_fe_port, &grp_id);
								if (MT_SUCCESS != ret)
								{
									MT_FE_ERR("mt_unf_fe_get_plp_grpid error.\n");
								}

								plp_attr[i].plp_index = i;
								plp_attr[i].plpid = plpid;
								plp_attr[i].u8PlpGrpId = grp_id;
								plp_attr[i].enPlpType = plptype;
							}

							//reinit plp id to 0
							mt_unf_fe_set_plp_mode(g_fe_port, 0);
							ret = mt_unf_fe_set_plp_id(g_fe_port, 0);
							if (MT_SUCCESS != ret)
							{
								MT_FE_ERR("mt_unf_fe_set_plp_id error.\n");
							}

							for (i = 0; i < plpnum - 1; i++)
							{
								for (j = 0; j < plpnum - 1 - i; j++)
								{
									if (plp_attr[j].u8PlpGrpId > plp_attr[j + 1].u8PlpGrpId)
									{
										memcpy(&plp_buf, &plp_attr[j], sizeof(mt_unf_fe_ter_plp_attr_t));
										memcpy(&plp_attr[j], &plp_attr[j + 1], sizeof(mt_unf_fe_ter_plp_attr_t));
										memcpy(&plp_attr[j + 1], &plp_buf, sizeof(mt_unf_fe_ter_plp_attr_t));
									}
								}
							}

							memset(t2_plp_array, 0, sizeof(t2_plp_array));
							t2_plp_array[0].grp_id = plp_attr[0].u8PlpGrpId;
							for (i = 0, grp_num = 0; i < plpnum; i++)
							{
								if (t2_plp_array[grp_num].grp_id != plp_attr[i].u8PlpGrpId)
								{
									grp_num++;
									t2_plp_array[grp_num].grp_id = plp_attr[i].u8PlpGrpId;
									cnt = 0;
									comm_exist = 0;
								}

								if (MT_UNF_FE_T2_PLP_TYPE_COM == plp_attr[i].enPlpType)
								{
									t2_plp_array[grp_num].com_id = plp_attr[i].plpid;
									comm_exist = 1;
								}

								cnt++;
								if ((cnt >= 2) && comm_exist)
								{
									t2_plp_array[grp_num].combination = 1;
								}
							}

							grp_num = grp_num + 1;

							for (i = 0, k = 0; i < plpnum; i++)
							{
								for (j = 0; j < grp_num; j++)
								{
									if (plp_attr[i].u8PlpGrpId == t2_plp_array[j].grp_id)
										break;
								}

								if (MT_UNF_FE_T2_PLP_TYPE_COM == plp_attr[i].enPlpType)
								{
									continue;
								}

								tp_array[k].plp_index = plp_attr[i].plp_index;
								tp_array[k].plpid = plp_attr[i].plpid;
								tp_array[k].com_id = t2_plp_array[j].com_id;
								tp_array[k].combination = t2_plp_array[j].combination;
								k++;
							}

							printf("plp index       plp id      common plp id       combination\n");
							for (i = 0; i < k; i++)
							{
								printf("%d          %d          %d          %d\n", tp_array[i].plp_index, tp_array[i].plpid, tp_array[i].com_id, tp_array[i].combination);
							}
						}

						return MT_SUCCESS;

					default:
						return MT_SUCCESS;
				}
			}
			else
			{
				MT_USLEEP(10000);
			}
		}
	}
	else
	{
		/* signal unclok */
		switch (g_fe_connect_param.sig_type)
		{
			case MT_UNF_FE_SIG_TYPE_CAB:
				printf("fe lock freq %d symb %d  qam%d Fail!, ret = 0x%x\n",
					   freq, sym_rate, g_current_qam_type, ret);
				break;

			case MT_UNF_FE_SIG_TYPE_SAT:
				printf("fe lock freq %d symb %d  Fail!, ret = 0x%x\n",
					   freq, sym_rate, ret);
				break;

			case MT_UNF_FE_SIG_TYPE_DVB_T:
			case MT_UNF_FE_SIG_TYPE_DVB_T2:
			case MT_UNF_FE_SIG_TYPE_ISDB_T:
			case MT_UNF_FE_SIG_TYPE_ATSC_T:
			case MT_UNF_FE_SIG_TYPE_DTMB:
				printf("fe lock freq %d bandwidth %d Fail!, ret = 0x%x\n",
					   freq, band_width, ret);
				break;

			default:
				break;
		}
	}

	if (i_loop == i_loop_times)
	{
		switch (g_fe_connect_param.sig_type)
		{
			case MT_UNF_FE_SIG_TYPE_CAB:
				printf("fe lock freq %d symb %d  qam%d Fail!\n",
					   freq, sym_rate, g_current_qam_type);
				break;

			case MT_UNF_FE_SIG_TYPE_SAT:
				printf("fe lock freq %d symb %d  Fail!\n",
					   freq, sym_rate);
				break;

			case MT_UNF_FE_SIG_TYPE_DVB_T:
			case MT_UNF_FE_SIG_TYPE_DVB_T2:
			case MT_UNF_FE_SIG_TYPE_ISDB_T:
			case MT_UNF_FE_SIG_TYPE_ATSC_T:
			case MT_UNF_FE_SIG_TYPE_DTMB:
				/*printf("Tuner Lock freq %d bandwidth %d Fail!\n",
					   freq, band_width);*/
				break;

			default:
				break;
		}
	}

	//printf("FAIL end\n");
	strcpy(g_test_result, MT_RESULT_FAIL);
#endif

	return MT_FAILURE;
}

mt_void mt_fe_start(char *locktime)
{
	mt_s32 ret = MT_FAILURE;
	mt_unf_fe_status_t fe_status;
	mt_unf_fe_connect_para_t stTmpConnectPara;
	mt_u32 i_loop = 0;
	mt_u32 u32StatTime = 0;
	mt_u32 u32EndTime = 0;
	mt_u32 u32TempTime = 0;
	mt_u32 u32IndexLockTime;

	if (MT_NULL_PTR == locktime)
	{
		MT_FE_ERR("please input loctime count\n");
		return;
	}
	mt_u32 total_locktime = atoi(locktime);

	memcpy(&stTmpConnectPara, &g_fe_connect_param, sizeof(mt_unf_fe_connect_para_t));

	s_s32LoopNum = total_locktime;

	s_s32FailTime = 0;
	s_s32Out1Time = 0;
	s_s32OutFailTime = 0;
	s_s32Time = 0;

	ret = printime_init();
	if (MT_FAILURE == ret)
	{
		MT_FE_ERR("printime_init failure\n");
		return;
	}

	for (u32IndexLockTime = 0; u32IndexLockTime < total_locktime; u32IndexLockTime++)
	{
		u32StatTime = getcurtime();
		ret = mt_unf_fe_connect(g_fe_port, &g_fe_connect_param, 0);
		if (MT_SUCCESS == ret)
		{
			for (i_loop = 0; i_loop < 300; i_loop++)
			{
				ret = mt_unf_fe_get_status(g_fe_port, &fe_status);
				if (MT_UNF_FE_SIGNAL_LOCKED == fe_status.lock_status)
				{
					u32EndTime = getcurtime();
					u32TempTime = u32EndTime - u32StatTime;
					if (u32TempTime > 1000)
					{
						MT_FE_INFO("===111===IndexLockTime=%d,locktime=%d\n", u32IndexLockTime, u32TempTime);
						printtime_file(u32IndexLockTime, u32TempTime, 2);
					}
					else
					{
						MT_FE_INFO("===000===IndexLockTime=%d,locktime=%d\n", u32IndexLockTime, u32TempTime);
						printtime_file(u32IndexLockTime, u32TempTime, 0);
					}

					MT_FE_INFO("SUCCESS end\n");
					strcpy(g_test_result, MT_RESULT_SUCCESS);
					break;
				}
				else
				{
					MT_USLEEP(10000);
				}
			}
		}
		else
		{
			MT_FE_WARN("Tuner Lock freq %d symb %d  qam%d Fail!, ret = 0x%x\n",
					   g_fe_connect_param.connect_param.cab.freq,
					   g_fe_connect_param.connect_param.cab.sym_rate, g_current_qam_type, ret);
		}

		if (i_loop == 300)
		{
			MT_FE_WARN("Tuner Lock freq %d symb %d  qam%d time out , ret = 0x%x\n",
					   g_fe_connect_param.connect_param.cab.freq,
					   g_fe_connect_param.connect_param.cab.sym_rate, g_current_qam_type, ret);

			u32EndTime = getcurtime();
			u32TempTime = u32EndTime - u32StatTime;
			printtime_file(u32IndexLockTime, u32TempTime, 3);
		}

		if ((u32IndexLockTime % 2) == 0)
		{
			stTmpConnectPara.connect_param.cab.freq = g_fe_connect_param.connect_param.cab.freq + 8 * 1000;
			stTmpConnectPara.connect_param.cab.mod_type = g_fe_connect_param.connect_param.cab.mod_type;
			stTmpConnectPara.connect_param.cab.sym_rate = g_fe_connect_param.connect_param.cab.sym_rate;

			mt_unf_fe_connect(g_fe_port, &stTmpConnectPara, 500);

			MT_USLEEP(200000);
		}
	}
}

/* set signal type and call mt_fe_connect */
mt_void mt_fe_set_sig_type(char *sigtype)
{
	if (sigtype == MT_NULL_PTR)
	{
		return;
	}

	switch (atoi(sigtype))
	{
		case 0:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_CAB;
			g_fe_connect_param.connect_param.cab.sym_rate = 6875000;
			break;

		case 1:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
			g_fe_connect_param.connect_param.ter.band_width = 8000;
			break;

		case 2:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
			g_fe_connect_param.connect_param.ter.band_width = 8000;
			break;

		case 3:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_ISDB_T;
			break;

		case 4:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_ATSC_T;
			break;

		case 5:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DTMB;
			break;

		case 6:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
			break;

		default:
			g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_CAB;
			break;
	}

	mt_fe_connect();
}

/* set channel freqency/symbolrate/polar and call mt_fe_connect */
mt_void mt_fe_channel_set_dvbs(char *channel)
{
	mt_u32 freq, u32Symb, u32Polar = 0;
	mt_s32 ret;
	//MT_UNF_AVPLAY_STOP_OPT_S	stStop;
	//MT_UNF_VCODEC_ATTR_S		stVdecAttr;
	//PMT_COMPACT_TBL			*ProgTbl = MT_NULL;

	//stStop.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
	//stStop.u32TimeoutMs = 0;

	if (channel == MT_NULL_PTR)
	{
		return;
	}

	sscanf(channel, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
		   (mt_u32 *)&freq, (mt_u32 *)&u32Symb, (mt_u32 *)&u32Polar);

	if (MT_UNF_FE_POLARIZATION_BUTT <= u32Polar)
	{
		return;
	}

	g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
	g_fe_connect_param.connect_param.sat.freq = freq * 1000;
	g_fe_connect_param.connect_param.sat.sym_rate = u32Symb;
	g_fe_connect_param.connect_param.sat.polarization = u32Polar;

	ret = mt_fe_connect();

	if (ret != MT_SUCCESS)
	{
		printf("FAIL end\n");
		return;
	}

#if 0
	return = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stStop);
	if (return != MT_SUCCESS )
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_Stop failed.\n");
		return ;
	}

	MTADP_Search_Init();
	return = MTADP_Search_GetAllPmt(0,&ProgTbl);
	if (MT_SUCCESS != return)
	{
		MT_FE_ERR("call MTADP_Search_GetAllPmt failed\n");
		printf("FAIL end\n");
	}
	else
	{
		printf("SUCCESS end\n");
	}

	return = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
	if (return != MT_SUCCESS)
	{
		MT_FE_WARN("call MT_UNF_AVPLAY_Start_AUD failed.\n");
		/*return ;*/
	}

	return = MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
	if (return != MT_SUCCESS)
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_Start_VID failed.\n");
		return ;
	}
#endif
}

static mt_u32 parse_QAM(mt_u16 value)
{
	mt_u32 n_qam = MT_UNF_MOD_TYPE_QAM_32;

	switch (value)
	{
		case 16:
			n_qam = MT_UNF_MOD_TYPE_QAM_16;
			break;

		case 32:
			n_qam = MT_UNF_MOD_TYPE_QAM_32;
			break;

		case 64:
			n_qam = MT_UNF_MOD_TYPE_QAM_64;
			break;

		case 128:
			n_qam = MT_UNF_MOD_TYPE_QAM_128;
			break;

		case 256:
			n_qam = MT_UNF_MOD_TYPE_QAM_256;
			break;

		default:
			break;
	}

	return n_qam;
}

/* set channel freqency/symbolrate/polar and call mt_fe_connect */
mt_void mt_fe_channel_set_dvbc(char *channel)
{
	mt_u32 freq = 0, symb = 0, modulation = 0, band_width = 0;
	mt_s32 ret;
	//MT_UNF_AVPLAY_Stop_OPT_S	stStop;
	//MT_UNF_VCODEC_ATTR_S		stVdecAttr;
	//PMT_COMPACT_TBL			*ProgTbl = MT_NULL;

	//stStop.enMode = MT_UNF_AVPLAY_Stop_MODE_STILL;
	//stStop.u32TimeoutMs = 0;

	if (channel == MT_NULL_PTR)
	{
		return;
	}

	sscanf(channel, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
		   (mt_u32 *)&freq, (mt_u32 *)&symb, (mt_u32 *)&modulation, (mt_u32 *)&band_width);

	g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_CAB;
	g_fe_connect_param.connect_param.cab.freq = freq * 1000;
	g_fe_connect_param.connect_param.cab.sym_rate = symb;
	g_fe_connect_param.connect_param.cab.mod_type = parse_QAM(modulation);
	;
	g_fe_connect_param.connect_param.cab.band_width = band_width;

	ret = mt_fe_connect();

	if (ret != MT_SUCCESS)
	{
		printf("FAIL end\n");
		return;
	}
}

mt_void mt_fe_channel_set_dvbt(char *channel)
{
	mt_u32 freq = 0, band_width = 0, is_dvbt = 0;
	mt_s32 ret;
	//MT_UNF_AVPLAY_Stop_OPT_S	stStop;
	//MT_UNF_VCODEC_ATTR_S		stVdecAttr;
	//PMT_COMPACT_TBL   *ProgTbl = MT_NULL;
	//stStop.enMode = MT_UNF_AVPLAY_Stop_MODE_STILL;
	//stStop.u32TimeoutMs = 0;

	if (channel == MT_NULL_PTR)
	{
		return;
	}

	sscanf(channel, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
		   (mt_u32 *)&freq, (mt_u32 *)&band_width, (mt_u32 *)&is_dvbt);

	g_fe_connect_param.connect_param.ter.freq = freq;
	g_fe_connect_param.connect_param.ter.band_width = band_width;
	g_fe_connect_param.connect_param.ter.channel_mode = is_dvbt;
	g_fe_connect_param.connect_param.ter.dvbt_prio = (is_dvbt + 1); /*need to check*/

	ret = mt_fe_connect();

	//try dvb-t signal, if failed, then try dvb-t2 signal, finally, resume to dvb-t signal.
	if ((ret == MT_FAILURE) && (MT_UNF_FE_SIG_TYPE_DVB_T == g_fe_connect_param.sig_type))
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
		ret = mt_fe_connect(); //lock dvb-t2 signal
		if (ret == MT_FAILURE)
			printf("FAIL end\n");

		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
	}

	if (ret != MT_SUCCESS)
	{
		printf("FAIL end\n");
		return;
	}
}

/* set type of modulation and call mt_fe_connect */
mt_void mt_fe_set_qam(char *qam)
{
	mt_s32 qam_type;

	if (qam == MT_NULL_PTR)
	{
		return;
	}

	qam_type = atoi(qam);
	g_current_qam_type = qam_type;

	switch (qam_type)
	{
		case 64:
		{
			if (MT_UNF_DEMOD_TYPE_J83B == g_demod_type)
			{
				g_fe_connect_param.connect_param.cab.sym_rate = 5057000;
			}
			else
			{
				g_fe_connect_param.connect_param.cab.sym_rate = 6875000;
			}
			g_fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64;
			break;
		}

		case 256:
		{
			if (MT_UNF_DEMOD_TYPE_J83B == g_demod_type)
			{
				g_fe_connect_param.connect_param.cab.sym_rate = 5361000;
			}
			else
			{
				g_fe_connect_param.connect_param.cab.sym_rate = 6875000;
			}
			g_fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_256;
			break;
		}

		case 16:
			g_fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_16;
			break;

		case 32:
			g_fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_32;
			break;

		case 128:
			g_fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_128;
			break;

		default:
			g_current_qam_type = 64;
			g_fe_connect_param.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64;
			break;
	}

	//mt_fe_connect_dvbc();
}

mt_void mt_fe_select_port(char *port)
{
	mt_s32 ret = 0;
	mt_s32 tuner_id;
	mt_unf_fe_attr_t fe_attr;

	if (port == MT_NULL_PTR)
	{
		return;
	}

	tuner_id = atoi(port);
	if ((tuner_id < 0) || (tuner_id > 1))
	{
		MT_FE_ERR("Input Port err %d\n", tuner_id);
		return;
	}

	g_fe_port = tuner_id;

	ret = mt_unf_fe_get_default_attr(g_fe_port, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("HI_UNF_TUNER_GetAttr error \n");
		return;
	}

	fe_attr.demod_i2c_id = 3;
	fe_attr.demod_dev_type = MT_UNF_DEMOD_TYPE_DS3103;
	MT_FE_INFO("fe_attr.sig_type = %d\n", fe_attr.sig_type);

	ret = mt_unf_fe_set_attr(g_fe_port, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("mt_unf_fe_set_attr error\n");
		return;
	}

/* for test */
#if 0
	ret = mt_unf_dmx_attach__ts_port(0, port);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("call HI_UNF_DMX_AttachTSPort failed.\n");
		HI_UNF_DMX_DeInit();
		mt_unf_fe_close(0);
		mt_unf_fe_deinit();
		return;
	}
#endif

	mt_fe_connect();
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

	ret = mt_unf_fe_get_attr(g_fe_port, &fe_attr);
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

	/* for test */
	fe_attr.tuner_type = MT_UNF_TUNER_TYPE_M88TS2022;

	(mt_void) mt_unf_fe_set_attr(g_fe_port, &fe_attr);

	mt_fe_connect();
}

mt_void mt_fe_get_signal_info(char *para)
{
	mt_s32 ret = 0;
	mt_unf_fe_signal_info_t stInfo;
	mt_u32 u32SNR;
	mt_u32 u32SignalStrength;
	mt_u32 u32SignalQuality;

	ret = mt_unf_fe_get_snr(g_fe_port, &u32SNR);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("mt_unf_fe_get_snr failed\n");
	}
	else
	{
		MT_FE_INFO("SNR:\t\t\t%d\n", u32SNR);
	}

	ret = mt_unf_fe_get_signal_strength(g_fe_port, &u32SignalStrength);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("mt_unf_fe_get_signal_strength failed\n");
	}
	else
	{
		MT_FE_INFO("Signal Strength:\t%ddBuv\n", u32SignalStrength);
	}

	ret = mt_unf_fe_get_signal_quality(g_fe_port, &u32SignalQuality);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("HI_UNF_TUNER_GetSignalQuality failed\n");
	}
	else
	{
		MT_FE_INFO("Signal Quality:\t\t%d%%\n", u32SignalQuality);
	}

	if ((MT_UNF_FE_SIG_TYPE_SAT <= g_fe_connect_param.sig_type) ||
		(MT_UNF_FE_SIG_TYPE_DTMB >= g_fe_connect_param.sig_type))
	{
		ret = mt_unf_fe_get_signal_info(g_fe_port, &stInfo);
		if (ret != MT_SUCCESS)
		{
			MT_FE_ERR("call HI_UNF_TUNER_GetSignalInfo failed.\n");
			return;
		}

		switch (stInfo.sig_type)
		{
			case MT_UNF_FE_SIG_TYPE_CAB:
				MT_FE_INFO("Signal type:\t\tCable\n");
				break;

			case MT_UNF_FE_SIG_TYPE_DVB_T:
				MT_FE_INFO("Signal type:\t\tDVB-T\n");
				break;

			case MT_UNF_FE_SIG_TYPE_DVB_T2:
				MT_FE_INFO("Signal type:\t\tDVB-T2\n");
				break;

			case MT_UNF_FE_SIG_TYPE_ISDB_T:
			case MT_UNF_FE_SIG_TYPE_ATSC_T:
			case MT_UNF_FE_SIG_TYPE_DTMB:
				MT_FE_INFO("Signal type:\t\tTerrestrial\n");
				break;

			case MT_UNF_FE_SIG_TYPE_SAT:
				MT_FE_INFO("Signal type:\t\tSatellite\n");
				break;

			case MT_UNF_FE_SIG_TYPE_BUTT:
			default:
				MT_FE_INFO("Signal type:\t\tUnknown\n");
				break;
		}

		if (MT_UNF_FE_SIG_TYPE_SAT == stInfo.sig_type)
		{
			switch (stInfo.sig_info.sat.mode_type)
			{
				case MT_UNF_MOD_TYPE_QAM_16:
				case MT_UNF_MOD_TYPE_QAM_32:
				case MT_UNF_MOD_TYPE_QAM_64:
				case MT_UNF_MOD_TYPE_QAM_128:
				case MT_UNF_MOD_TYPE_QAM_256:
				case MT_UNF_MOD_TYPE_QAM_512:
					MT_FE_INFO("Modulation type: \tQAM\n");
					break;

				case MT_UNF_MOD_TYPE_BPSK:
					MT_FE_INFO("Modulation type: \tBPSK\n");
					break;

				case MT_UNF_MOD_TYPE_QPSK:
					MT_FE_INFO("Modulation type: \tQPSK\n");
					break;

				case MT_UNF_MOD_TYPE_8PSK:
					MT_FE_INFO("Modulation type: \t8PSK\n");
					break;

				case MT_UNF_MOD_TYPE_16APSK:
					MT_FE_INFO("Modulation type: \t16APSK\n");
					break;

				case MT_UNF_MOD_TYPE_32APSK:
					MT_FE_INFO("Modulation type: \t32APSK\n");
					break;

				case MT_UNF_MOD_TYPE_DEFAULT:
				case MT_UNF_MOD_TYPE_AUTO:
				case MT_UNF_MOD_TYPE_BUTT:
				default:
					MT_FE_INFO("Modulation type: \tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.sat.sat_type)
			{
				case MT_UNF_FE_DVBS:
					MT_FE_INFO("FEC type:\t\tDVBS\n");
					break;

				case MT_UNF_FE_DVBS2:
					MT_FE_INFO("FEC type:\t\tDVBS2\n");
					break;

				case MT_UNF_FE_DIRECTV:
					MT_FE_INFO("FEC type:\t\tDIRECTV\n");
					break;

				case MT_UNF_FE_BUTT:
				default:
					MT_FE_INFO("FEC type:\t\tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.sat.fec_rate)
			{
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

			ret = mt_unf_fe_get_signal_quality(g_fe_port, &u32SignalQuality);
			if (MT_SUCCESS != ret)
			{
				MT_FE_ERR("HI_UNF_TUNER_GetSignalQuality failed\n");
			}
			MT_FE_INFO("Signal Quality:\t\t%d%%\n", u32SignalQuality);
		}
		else if (MT_UNF_FE_SIG_TYPE_DVB_T <= stInfo.sig_type && MT_UNF_FE_SIG_TYPE_DTMB >= stInfo.sig_type)
		{
			switch (stInfo.sig_info.ter.enFECRate)
			{
				case MT_UNF_FE_FEC_1_2:
					printf("FEC rate:\t\t1/2\n");
					break;

				case MT_UNF_FE_FEC_2_3:
					printf("FEC rate:\t\t2/3\n");
					break;

				case MT_UNF_FE_FEC_3_4:
					printf("FEC rate:\t\t3/4\n");
					break;

				case MT_UNF_FE_FEC_4_5:
					MT_FE_INFO("FEC rate:\t\t4/5\n");
					break;

				case MT_UNF_FE_FEC_5_6:
					printf("FEC rate:\t\t5/6\n");
					break;

				case MT_UNF_FE_FEC_6_7:
					printf("FEC rate:\t\t6/7\n");
					break;

				case MT_UNF_FE_FEC_7_8:
					printf("FEC rate:\t\t7/8\n");
					break;

				case MT_UNF_FE_FEC_8_9:
					printf("FEC rate:\t\t8/9\n");
					break;

				case MT_UNF_FE_FEC_9_10:
					printf("FEC rate:\t\t8/9\n");
					break;

				case MT_UNF_FE_FEC_1_4:
					printf("FEC rate:\t\t1/4\n");
					break;

				case MT_UNF_FE_FEC_1_3:
					printf("FEC rate:\t\t1/3\n");
					break;

				case MT_UNF_FE_FEC_2_5:
					printf("FEC rate:\t\t2/5\n");
					break;

				case MT_UNF_FE_FEC_3_5:
					printf("FEC rate:\t\t3/5\n");
					break;

				case MT_UNF_FE_FECRATE_BUTT:
				case MT_UNF_FE_FEC_AUTO:
				default:
					printf("FEC rate:\t\tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.ter.enGuardIntv)
			{
				case MT_UNF_FE_GUARD_INTV_1_32:
					printf("GI:\t\t\t1/32\n");
					break;

				case MT_UNF_FE_GUARD_INTV_1_16:
					printf("GI:\t\t\t1/16\n");
					break;

				case MT_UNF_FE_GUARD_INTV_1_8:
					printf("GI:\t\t1/8\n");
					break;

				case MT_UNF_FE_GUARD_INTV_1_4:
					printf("GI:\t\t\t1/4\n");
					break;

				case MT_UNF_FE_GUARD_INTV_1_128:
					printf("GI:\t\t\t1/128\n");
					break;

				case MT_UNF_FE_GUARD_INTV_19_128:
					printf("GI:\t\t\t19/128\n");
					break;

				case MT_UNF_FE_GUARD_INTV_19_256:
					printf("GI:\t\t\t19/256\n");
					break;

				default:
					printf("GI:\t\t\tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.ter.enModType)
			{
				case MT_UNF_MOD_TYPE_QPSK:
					printf("ModType:\t\tQPSK\n");
					break;

				case MT_UNF_MOD_TYPE_QAM_16:
					printf("ModType:\t\tQAM_16\n");
					break;

				case MT_UNF_MOD_TYPE_QAM_32:
					printf("ModType:\t\tQAM_32\n");
					break;

				case MT_UNF_MOD_TYPE_QAM_64:
					printf("ModType:\t\tQAM_64\n");
					break;

				case MT_UNF_MOD_TYPE_QAM_128:
					printf("ModType:\t\tQAM_128\n");
					break;

				case MT_UNF_MOD_TYPE_QAM_256:
					printf("ModType:\t\tQAM_256\n");
					break;

				case MT_UNF_MOD_TYPE_QAM_512:
					printf("ModType:\t\tQAM_512\n");
					break;

				case MT_UNF_MOD_TYPE_BPSK:
					printf("ModType:\t\tBPSK\n");
					break;

				case MT_UNF_MOD_TYPE_DQPSK:
					printf("ModType:\t\tDQPSK\n");
					break;

				case MT_UNF_MOD_TYPE_8PSK:
					printf("ModType:\t\t8PSK\n");
					break;

				case MT_UNF_MOD_TYPE_16APSK:
					printf("ModType:\t\t16APSK\n");
					break;

				case MT_UNF_MOD_TYPE_32APSK:
					printf("ModType:\t\t32APSK\n");
					break;

				default:
					printf("ModType:\t\tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.ter.enFFTMode)
			{
				case MT_UNF_FE_FFT_1K:
					printf("FFTMode:\t\t1K\n");
					break;

				case MT_UNF_FE_FFT_2K:
					printf("FFTMode:\t\t2K\n");
					break;

				case MT_UNF_FE_FFT_4K:
					printf("FFTMode:\t\t4K\n");
					break;

				case MT_UNF_FE_FFT_8K:
					printf("FFTMode:\t\t8K\n");
					break;

				case MT_UNF_FE_FFT_16K:
					printf("FFTMode:\t\t16K\n");
					break;

				case MT_UNF_FE_FFT_32K:
					printf("FFTMode:\t\t32K\n");
					break;

				case MT_UNF_FE_FFT_64K:
					printf("FFTMode:\t\t64K\n");
					break;

				default:
					printf("FFTMode:\t\tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.ter.enHierMod)
			{
				case MT_UNF_FE_HIERARCHY_NO:
					printf("HierMod:\t\tNONE\n");
					break;

				case MT_UNF_FE_HIERARCHY_ALHPA1:
					printf("HierMod:\t\tALHPA1\n");
					break;

				case MT_UNF_FE_HIERARCHY_ALHPA2:
					printf("HierMod:\t\tALHPA2\n");
					break;

				case MT_UNF_FE_HIERARCHY_ALHPA4:
					printf("HierMod:\t\tALHPA4\n");
					break;

				default:
					printf("HierMod:\t\tUnknown\n");
					break;
			}

			switch (stInfo.sig_info.ter.enTsPriority)
			{
				case MT_UNF_FE_TS_PRIORITY_NONE:
					printf("TsPriority:\t\tNONE\n");
					break;

				case MT_UNF_FE_TS_PRIORITY_HP:
					printf("TsPriority:\t\tHP\n");
					break;

				case MT_UNF_FE_TS_PRIORITY_LP:
					printf("TsPriority:\t\tLP\n");
					break;

				default:
					printf("TsPriority:\t\tUnknown\n");
					break;
			}
		}
	}
}

mt_void mt_fe_set_lnb_power(char *power)
{
	if (power == MT_NULL_PTR)
	{
		return;
	}

	if (MT_SUCCESS != mt_unf_fe_set_lnb_power(g_fe_port, atoi(power)))
	{
		MT_FE_ERR("Set LNB power fail:%d\n", atoi(power));
	}
}

mt_void mt_fe_set_antena_power(char *power)
{
	mt_unf_fe_ter_antenna_power_t en_power;

	if (power == MT_NULL_PTR)
	{
		return;
	}

	sscanf(power, "%d", (mt_u32 *)&en_power);

	mt_unf_fe_set_antenna_power(g_fe_port, en_power);
}

/* Configurate LNB parameter */
//mt_void mt_fe_set_lnb(char *freq)
mt_void mt_fe_set_lnb(mt_u32 low_lo, mt_u32 high_lo, mt_u8 unicable_port_no, mt_u8 unicable_scr_no, mt_u32 unicable_if_freq_mhz)
{
	mt_unf_fe_lnb_type_t enType;
	mt_u32 u32LowLOFreq = 0;
	mt_u32 u32HighLOFreq = 0;
	mt_unf_fe_lnb_band_t enBand;
	mt_unf_fe_lnb_config_t lnb_config;
	mt_u32 u32SCRNO = 0;
	mt_u32 u32IFCenterFreq_MHz = 0;
	mt_unf_fe_sat_position_t enSatPosn = MT_UNF_FE_SATPOSN_A;

#if 0
	if (freq == MT_NULL_PTR)
	{
		return;
	}

	sscanf(freq, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
		   (mt_u32 *)&enType, &u32LowLOFreq, &u32HighLOFreq, (mt_u32 *)&enBand, &u32SCRNO, &u32IFCenterFreq_MHz, (mt_u32 *)&enSatPosn);
	MT_FE_INFO("mt_fe_set_lnb Type %d, Low LO:%dMHz, High LO: %dMHz, Band %d \n",
			   enType, u32LowLOFreq, u32HighLOFreq, enBand);
#endif

	g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;

	if (g_fe_connect_param.sig_type == MT_UNF_FE_SIG_TYPE_SAT)
	{
		lnb_config.lnb_type = MT_UNF_FE_LNB_UNICABLE; //enType;
		lnb_config.low_lo = low_lo;                   //u32LowLOFreq;
		lnb_config.high_lo = high_lo;                 //u32HighLOFreq;
		lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_KU;  //enBand;

		//lnb_config.unicable_bank = 0;
		lnb_config.unicable_bank = 0;
		lnb_config.unicable_scr_no = unicable_scr_no;           //u32SCRNO;
		lnb_config.unicable_if_freq_mhz = unicable_if_freq_mhz; //u32IFCenterFreq_MHz;
		lnb_config.unicable_port_no = unicable_port_no;         //enSatPosn;
	}
	else
	{
		MT_FE_WARN("Your Signal Type unsupport lnb config.\n");
	}

	if (MT_SUCCESS != mt_unf_fe_set_lnb_config(g_fe_dvbs_port, &lnb_config))
	{
		MT_FE_ERR("call mt_unf_fe_set_lnb_config failed.\n");
	}
}

/* set plp id */
mt_void mt_fe_set_plp_id(char *plpid)
{
/*for test*/
#if 0
	mt_u32 u32PLPID = 0, u32ComPlpId = 0, u32Combination = 0;
	mt_s32 return;
	MT_UNF_AVPLAY_Stop_OPT_S stStop;
	PMT_COMPACT_TBL *ProgTbl = MT_NULL;
	MT_UNF_VCODEC_ATTR_S stVdecAttr;

	if (plpid == MT_NULL_PTR)
	{
		return;
	}

	sscanf(plpid, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d", &u32PLPID, &u32ComPlpId, &u32Combination);

	MT_UNF_TUNER_SetPLPMode(g_fe_port, 0);
	if (MT_SUCCESS != MT_UNF_TUNER_SetPLPID(g_fe_port, (mt_u8)u32PLPID))
	{
		printf("set plpid %d failed.\n", u32PLPID);
	}
	else
	{
		printf("set plpid %d succeed.\n", u32PLPID);
	}

	if (MT_SUCCESS != MT_UNF_TUNER_SetCommonPLPID(g_fe_port, (mt_u8)u32ComPlpId))
	{
		printf("set common plp id %d failed.\n", u32ComPlpId);
	}
	else
	{
		printf("set common plp id %d succeed.\n", u32ComPlpId);
	}

	if (MT_SUCCESS != MT_UNF_TUNER_SetCommonPLPCombination(g_fe_port, (mt_u8)u32Combination))
	{
		printf("set common plp combination %d failed.\n", u32Combination);
	}
	else
	{
		printf("set common plp combination %d succeed.\n", u32Combination);
	}

	stStop.enMode = MT_UNF_AVPLAY_Stop_MODE_STILL;
	stStop.u32TimeoutMs = 0;

	return = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stStop);
	if (return != MT_SUCCESS)
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_Stop failed.\n");
		return;
	}

	MTADP_Search_Init();
	return = MTADP_Search_GetAllPmt(0, &ProgTbl);
	if (MT_SUCCESS != return)
	{
		MT_FE_ERR("call MTADP_Search_GetAllPmt failed\n");
		//return;
	}
	else
	{
		MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &stVdecAttr);
		stVdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
		stVdecAttr.enType = ProgTbl->proginfo->VideoType;
		stVdecAttr.u32ErrCover = 80;
		stVdecAttr.u32Priority = MT_UNF_VCODEC_MAX_PRIORITY;
		stVdecAttr.bOrderOutput = MT_FALSE;
		return = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &stVdecAttr);
		if (MT_SUCCESS != return)
		{
			MT_FE_ERR("call MT_UNF_AVPLAY_SetAttr_VDEC failed.\n");
			MT_UNF_AVPLAY_Destroy(hAvplay);
			MT_UNF_AVPLAY_DeInit();
			dev_deinit();
			return;
		}
	}
#endif
}

/* If your diseqc device need config polarization and 22K, you need registe the callback */
mt_void mt_fe_diseqc_set(mt_u32 tuner_id, mt_unf_fe_polar_t polar,
						 mt_unf_fe_lnb_22k_t enLNB22K)
{
}

static struct timeval s_stStart, s_stStop;
static mt_s32 s_s32TPNum = 0;
static mt_unf_fe_sat_tpinfo_t s_astTP[400];

mt_void mt_fe_blindscan_notify(mt_u32 tuner_id, mt_unf_fe_blindscan_evt_t enEVT, mt_unf_fe_blindscan_notify_t *punNotify)
{
	mt_s32 i = 0;
	mt_u32 u32TimeUse;

	switch (enEVT)
	{
		case MT_UNF_FE_BLINDSCAN_EVT_STATUS:
			if (MT_UNF_FE_BLINDSCAN_STATUS_FAIL == *(punNotify->status))
			{
				MT_FE_ERR("Scan fail.\n");
			}
			else if ((MT_UNF_FE_BLINDSCAN_STATUS_FINISH == *(punNotify->status)) || (MT_UNF_FE_BLINDSCAN_STATUS_QUIT == *(punNotify->status)))
			{
				gettimeofday(&s_stStop, NULL);
				u32TimeUse = 1000000 * (s_stStop.tv_sec - s_stStart.tv_sec) + s_stStop.tv_usec - s_stStart.tv_usec;
				u32TimeUse /= 1000000;
				MT_FE_INFO("100");
				putchar('%');
				MT_FE_INFO(" done!\n");
				MT_FE_INFO("Scan over, find %d TP, use %ds.\n", s_s32TPNum, u32TimeUse);
				for (i = 0; i < s_s32TPNum; i++)
				{
					//MT_FE_INFO("%03d %d %d %d %d\n", i, s_astTP[i].freq, s_astTP[i].sym_rate, s_astTP[i].polar, \
								//s_astTP[i].cbs_reliablity/*, s_astTP[i].agc_h8*/);
					MT_FE_INFO("%03d %d %d %d %d\n", i, s_astTP[i].freq, s_astTP[i].symbol_rate, s_astTP[i].polar, s_astTP[i].code_rate);
				}

				memset(s_astTP, 0, sizeof(mt_unf_fe_sat_tpinfo_t) * 400);
				s_s32TPNum = 0;
			}
			break;

		case MT_UNF_FE_BLINDSCAN_EVT_PROGRESS:
#if 0	
			MT_FE_INFO("%d",*(punNotify->progress_percent));
			putchar('%');
			MT_FE_INFO(" done!\n");
#else
			MT_FE_ERR("%d\n", *(punNotify->progress_percent));
#endif
			break;

		case MT_UNF_FE_BLINDSCAN_EVT_NEWRESULT:
			if (s_s32TPNum < sizeof(s_astTP) / sizeof(mt_unf_fe_sat_tpinfo_t))
			{
				s_astTP[s_s32TPNum] = *(punNotify->result);
				s_s32TPNum++;
			}
			else
			{
				MT_FE_WARN("Too many channels!\n");
			}
			break;

		default:
			break;
	}
}

/* Blind scan */
//mt_void mt_fe_blindscan(char *pPara)
mt_void mt_fe_blindscan(mt_u8 bs_mode, mt_u32 u32StartFreq, mt_u32 u32StopFreq, mt_u8 polar, mt_u8 onoff_22k)
{
	mt_s32 ret = 0;
	//mt_s32 s32BlindScanType;
	//mt_u32 u32StartFreq;
	//mt_u32 u32StopFreq;
	//mt_unf_fe_polar_t polar;
	//mt_unf_fe_lnb_22k_t enLNB22K;
	mt_unf_fe_blindscan_para_t stBlindScanPara;

#if 0
	if (pPara == MT_NULL_PTR)
	{
		return;
	}

	sscanf(pPara, "%d", (mt_s32 *)&s32BlindScanType);
	if (1 == s32BlindScanType)
	{
		sscanf(pPara, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
			   (mt_s32 *)&s32BlindScanType, &u32StartFreq, &u32StopFreq, (mt_s32 *)&polar, (mt_s32 *)&enLNB22K);
		MT_FE_INFO("mt_fe_blindscan Type:%d, Polar %d, 22K %d,  %dkHz - %dkHz\n",
				   s32BlindScanType, polar, enLNB22K, u32StartFreq, u32StopFreq);
	}
#endif

	//s_s32TPNum = 0;
	gettimeofday(&s_stStart, NULL);

	//s32BlindScanType = 1;
	//polar = 0;
	//enLNB22K = 0;
	//u32StartFreq = 950*1000;
	//u32StopFreq = 2150*1000;

	g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;

	if (g_fe_connect_param.sig_type == MT_UNF_FE_SIG_TYPE_SAT)
	{
		if (0 == bs_mode)
		{
			/* Auto */
			stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_AUTO;
			/* If your diseqc device need config polarization and 22K, you need register the callback */
			stBlindScanPara.scan_para.sat.diseqc_set = mt_fe_diseqc_set;
			stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))mt_fe_blindscan_notify;
		}
		else
		{
			stBlindScanPara.mode = MT_UNF_FE_BLINDSCAN_MODE_MANUAL;
			stBlindScanPara.scan_para.sat.polar = polar;
			stBlindScanPara.scan_para.sat.lnb_22k = onoff_22k; //enLNB22K;
			stBlindScanPara.scan_para.sat.start_freq = u32StartFreq * 1000;
			stBlindScanPara.scan_para.sat.stop_freq = u32StopFreq * 1000;
			stBlindScanPara.scan_para.sat.diseqc_set = MT_NULL;
			stBlindScanPara.scan_para.sat.scan_notify = (mt_void(*)(mt_u32, mt_unf_fe_blindscan_evt_t, void *))mt_fe_blindscan_notify;
		}
	}

	ret = mt_unf_fe_blindscan_start(g_fe_dvbs_port, &stBlindScanPara);
	if (ret != MT_SUCCESS)
	{
		MT_FE_ERR("call mt_unf_fe_blindscan_start failed.\n");
		return;
	}
}

/* Stop blind scan */
mt_void mt_fe_blindscan_stop(char *arg)
{
	mt_unf_fe_blindscan_stop(g_fe_port);
}

#ifdef CONFIG_MT_DISEQC_SUPPORT
/* Switch test, for DVB-S/S2 */
mt_void mt_fe_switch_diseqc(char *pSwitch)
{
	mt_s32 s32SwitchType;
	mt_s32 s32Port;
	mt_unf_fe_polar_t polar;
	mt_unf_fe_lnb_22k_t enLNB22K;
	mt_s32 ret = MT_SUCCESS;
	mt_unf_fe_diseqc_switch4port_t st4Port;
	mt_unf_fe_diseqc_switch16port_t st16Port;

	if (pSwitch == MT_NULL_PTR)
	{
		return;
	}

	sscanf(pSwitch, "%d", (mt_s32 *)&s32SwitchType);
	if (s32SwitchType == 3)
	{
		sscanf(pSwitch, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
			   (mt_s32 *)&s32SwitchType, (mt_s32 *)&s32Port, (mt_s32 *)&polar, (mt_s32 *)&enLNB22K);
		MT_FE_INFO("mt_fe_switch_diseqc Type %d, Port:%d, Polar: %d, LNB22K %d \n",
				   s32SwitchType, s32Port, polar, enLNB22K);
	}
	else if (s32SwitchType < 5)
	{
		ret = sscanf(pSwitch, "%d" TEST_ARGS_SPLIT "%d", (mt_s32 *)&s32SwitchType, (mt_s32 *)&s32Port);
		MT_FE_INFO("mt_fe_switch_diseqc Type %d, Port:%d \n", s32SwitchType, s32Port);
	}
	else if (s32SwitchType > 7)
	{
		MT_FE_WARN("Error SwitchType!\n");
		return;
	}

	switch (s32SwitchType)
	{
		case 0:
			ret = mt_unf_fe_switch_0_12v(g_fe_dvbs_port, (mt_unf_fe_switch_0_12v_t)s32Port);
			break;

		case 1:
			ret = mt_unf_fe_switch_toneburst(g_fe_dvbs_port, (mt_unf_fe_switch_toneburst_t)s32Port);
			break;

		case 2:
			ret = mt_unf_fe_switch_22k(g_fe_dvbs_port, (mt_unf_fe_switch_22k_t)s32Port);
			break;

		case 3:
			st4Port.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			st4Port.port = (mt_unf_fe_diseqc_switch_port_t)s32Port;
			st4Port.polar = polar;
			st4Port.lnb_22k = enLNB22K;
			ret = mt_unf_fe_diseqc_switch4port(g_fe_dvbs_port, &st4Port);
			break;

		case 4:
			st16Port.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			st16Port.port = (mt_unf_fe_diseqc_switch_port_t)s32Port;
			ret = mt_unf_fe_diseqc_switch16port(g_fe_dvbs_port, &st16Port);
			break;

		case 5:
			ret = mt_unf_fe_diseqc_reset(g_fe_dvbs_port, MT_UNF_FE_DISEQC_LEVEL_1_X);
			break;

		case 6:
			ret = mt_unf_fe_diseqc_standby(g_fe_dvbs_port, MT_UNF_FE_DISEQC_LEVEL_1_X);
			break;

		case 7:
			ret = mt_unf_fe_diseqc_wakeup(g_fe_dvbs_port, MT_UNF_FE_DISEQC_LEVEL_1_X);
			break;
	}

	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("Switch control failed.\n");
	}
}

/* Motor control test, for DVB-S/S2 */
mt_void mt_fe_motor(char *pPara)
{
	mt_s32 s32CtrlType;
	mt_u32 u32Para1;
	mt_u32 u32Para2;
	mt_u32 u32Para3;
	mt_unf_fe_diseqc_position_t stPos;
	mt_unf_fe_diseqc_limit_t stLimit;
	mt_unf_fe_diseqc_move_t stMove;
	mt_unf_fe_diseqc_usals_para_t stUSALS;
	mt_unf_fe_diseqc_usals_angular_t stAngular;
	mt_unf_fe_diseqc_recalculate_t stRecal;
	mt_s32 ret = MT_SUCCESS;

	if (pPara == MT_NULL_PTR)
	{
		return;
	}

	sscanf(pPara, "%d", (mt_s32 *)&s32CtrlType);
	if (s32CtrlType <= 2)
	{
		sscanf(pPara, "%d" TEST_ARGS_SPLIT "%d", (mt_s32 *)&s32CtrlType, (mt_s32 *)&u32Para1);
		MT_FE_INFO("mt_fe_motor Ctrl %d, Para1:%d\n", s32CtrlType, u32Para1);
	}
	else if (s32CtrlType == 3)
	{
		sscanf(pPara, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
			   (mt_s32 *)&s32CtrlType, &u32Para1, &u32Para2);
		MT_FE_INFO("mt_fe_motor Ctrl %d, Para1:%d, Para2:%d\n", s32CtrlType, u32Para1, u32Para2);
	}
	else if ((s32CtrlType == 5) || (s32CtrlType == 6))
	{
		sscanf(pPara, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d",
			   (mt_s32 *)&s32CtrlType, &u32Para1, &u32Para2, &u32Para3);
		MT_FE_INFO("mt_fe_motor Ctrl %d, Para1:%d, Para2:%d, Para3:%d\n", s32CtrlType, u32Para1, u32Para2, u32Para3);
	}
	else if (s32CtrlType == 7)
	{
		sscanf(pPara, "%d" TEST_ARGS_SPLIT "%d",
			   (mt_s32 *)&s32CtrlType, &u32Para1);
		MT_FE_INFO("mt_fe_motor Ctrl %d, Para1:%d\n", s32CtrlType, u32Para1);
	}
	else if (s32CtrlType == 4)
	{
		ret = sscanf(pPara, "%d", (mt_s32 *)&s32CtrlType);
		MT_FE_INFO("mt_fe_motor Ctrl %d\n", s32CtrlType);
	}
	else
	{
		MT_FE_WARN("Error Ctrl type!\n");
		return;
	}

	switch (s32CtrlType)
	{
		case 0:
			stPos.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stPos.pos = u32Para1;
			ret = mt_unf_fe_diseqc_storepos(g_fe_dvbs_port, &stPos);
			break;

		case 1:
			stPos.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stPos.pos = u32Para1;
			ret = mt_unf_fe_diseqc_goto_pos(g_fe_dvbs_port, &stPos);
			break;

		case 2:
			stLimit.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stLimit.limit = (mt_unf_fe_diseqc_dir_limit_t)u32Para1;
			ret = mt_unf_fe_diseqc_set_limit(g_fe_dvbs_port, &stLimit);
			break;

		case 3:
			stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stMove.dir = (mt_unf_fe_diseqc_move_dir_t)u32Para1;
			stMove.type = (mt_unf_fe_diseqc_move_type_t)u32Para2;
			ret = mt_unf_fe_diseqc_move(g_fe_dvbs_port, &stMove);
			break;

		case 4:
			ret = mt_unf_fe_diseqc_stop(g_fe_dvbs_port, MT_UNF_FE_DISEQC_LEVEL_1_X);
			break;

		case 5:
			stUSALS.local_longitude = (mt_u16)u32Para1;
			stUSALS.local_latitude = (mt_u16)u32Para2;
			stUSALS.sat_longitude = (mt_u16)u32Para3;
			ret = mt_unf_fe_diseqc_calc_angular(g_fe_dvbs_port, &stUSALS);
			MT_FE_INFO("Angular: %02x, %02x\n", (mt_u8)(stUSALS.angular >> 8), (mt_u8)stUSALS.angular);
			stAngular.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stAngular.angular = stUSALS.angular;
			ret |= mt_unf_fe_diseqc_goto_angular(g_fe_dvbs_port, &stAngular);
			break;

		case 6:
			stRecal.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stRecal.para1 = (mt_u8)u32Para1;
			stRecal.para2 = (mt_u8)u32Para2;
			stRecal.para3 = (mt_u8)u32Para3;
			ret = mt_unf_fe_diseqc_recalculate(g_fe_dvbs_port, &stRecal);
			break;

		case 7:
			stAngular.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
			stAngular.angular = u32Para1;
			ret = mt_unf_fe_diseqc_goto_angular(g_fe_dvbs_port, &stAngular);
			break;
	}

	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("Motor control failed.\n");
	}
}

mt_void mt_fe_unicable(char *pPara)
{
	mt_s32 s32UnicType;
	mt_u32 u32SCRNO, u32Para;
	mt_u32 u32Agc = 0;

	if (pPara == MT_NULL_PTR)
	{
		return;
	}

	sscanf(pPara, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d", (mt_s32 *)&s32UnicType, (mt_u32 *)&u32SCRNO, (mt_u32 *)&u32Para);

	switch (s32UnicType)
	{
		case 0:
			mt_unf_fe_scan_and_install_unicable(g_fe_dvbs_port);
			break;

		case 1:
			mt_unf_fe_unicable_power_off(g_fe_dvbs_port, (mt_u8)u32SCRNO);
			break;

		case 2:
			mt_unf_fe_unicable_scrx_on(g_fe_dvbs_port);
			break;

		case 3:
			mt_unf_fe_unicable_config(g_fe_dvbs_port, (mt_u8)u32SCRNO, (mt_u8)u32Para);
			break;

		case 4:
			mt_unf_fe_unicable_lofreq(g_fe_dvbs_port, (mt_u8)u32SCRNO, (mt_u8)u32Para);
			break;

		case 5:
			//mt_unf_fe_get_agc(g_fe_dvbs_port, u32SCRNO, &u32Agc);
			//printf("+++++Freq:%d++++++Agc:%d\n", u32SCRNO, u32Agc);
			break;
	}
}
#endif /* CONFIG_MT_DISEQC_SUPPORT */

/* Standby test */
mt_void mt_fe_standby(char *pPara)
{
	mt_u32 u32Para;
	mt_s32 ret = MT_SUCCESS;

	if (pPara == MT_NULL_PTR)
	{
		return;
	}

	sscanf(pPara, "%d", (mt_s32 *)&u32Para);
	MT_FE_INFO("mt_fe_standby %d\n", u32Para);

	if (0 == u32Para)
	{
		ret = mt_unf_fe_wakeup(g_fe_port);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("Tuner wake up failed.\n");
		}
	}
	else
	{
		ret = mt_unf_fe_standby(g_fe_port);
		if (MT_SUCCESS != ret)
		{
			MT_FE_ERR("Tuner standby failed.\n");
		}
	}
}

/*play program*/
mt_void mt_fe_play(char *avpid)
{
#if 0
	mt_s32 ret;
	mt_u32 u32Apid = 0;
	mt_u32 u32Vpid = 0;
	mt_u32 u32Vcodec = 0;
	MT_UNF_AVPLAY_Stop_OPT_S stStop;
	MT_UNF_VCODEC_ATTR_S stVdecAttr;

	stStop.enMode = MT_UNF_AVPLAY_Stop_MODE_STILL;
	stStop.u32TimeoutMs = 0;

	if (avpid == MT_NULL_PTR)
	{
		return;
	}

	ret = sscanf(avpid, "%d" TEST_ARGS_SPLIT "%d" TEST_ARGS_SPLIT "%d", &u32Vpid, &u32Apid, &u32Vcodec);

	ret = MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &stStop);
	if (ret != MT_SUCCESS)
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_Stop failed.\n");
		return;
	}

	MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &stVdecAttr);
	stVdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
	stVdecAttr.enType = u32Vcodec;
	stVdecAttr.u32ErrCover = 80;
	stVdecAttr.u32Priority = MT_UNF_VCODEC_MAX_PRIORITY;
	stVdecAttr.bOrderOutput = MT_FALSE;
	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &stVdecAttr);

	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &u32Apid);
	if (ret != MT_SUCCESS)
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_SetAttr failed.\n");
		return;
	}

	ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &u32Vpid);
	if (ret != MT_SUCCESS)
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_SetAttr failed.\n");
		return;
	}

	MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, NULL);
	if (ret != MT_SUCCESS)
	{
		MT_FE_WARN("call MT_UNF_AVPLAY_Start_AUD failed.\n");
		/*return ;*/
	}

	MT_UNF_AVPLAY_Start(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, NULL);
	if (ret != MT_SUCCESS)
	{
		MT_FE_ERR("call MT_UNF_AVPLAY_Start_VID failed.\n");
		return;
	}

	MT_FE_INFO("Play u32Vpid %d u32Apid %d\n", u32Vpid, u32Apid);

	MT_FE_INFO("SUCCESS end\n");
	strcpy(g_test_result, MT_RESULT_SUCCESS);
#endif
}

mt_void mt_fe_get_offset()
{
	mt_u32 u32Symb;
	mt_u32 freq;
	mt_s32 s32FreqOffset;
	/*mt_u32 au32BER[3] = {0};
	mt_u32 u32SNR = 0;
	mt_u32 u32SignalStrength = 0;*/
	mt_s32 ret = MT_FAILURE;

	ret = mt_unf_fe_get_real_freq_symb(g_fe_port, &freq, &u32Symb, &s32FreqOffset);
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("HI_UNF_TUNER_GetOffset failed\n");
		return;
	}
	MT_FE_INFO("freq = %d, actul_symb = %d\n", freq, u32Symb);

	/*ret = mt_unf_fe_get_ber(g_fe_port , au32BER);
	if ( MT_SUCCESS != ret )
	{
		printf("mt_unf_fe_get_ber failed\n");
		return;
	}
	printf("BER :%d.%de-%d\n", au32BER[0], au32BER[1], au32BER[2]);

	ret = mt_unf_fe_get_snr(g_fe_port, &u32SNR);
	if ( MT_SUCCESS != ret )
	{
		printf("mt_unf_fe_get_snr failed\n");
		return;
	}
	printf("SNR :%d\n", u32SNR);

	ret = mt_unf_fe_get_signal_strength(g_fe_port, &u32SignalStrength);
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

	ret = mt_unf_fe_get_attr(g_fe_port, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		printf("HI_UNF_TUNER_GetAttr fail\n");
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
		g_demod_type = MT_UNF_DEMOD_TYPE_J83B;
	}
	else
	{
		printf("j83ac\n");
		fe_attr.demod_dev_type = MT_UNF_DEMOD_TYPE_DS3103;
		g_demod_type = MT_UNF_DEMOD_TYPE_DS3103;
	}

	ret = mt_unf_fe_set_attr(g_fe_port, &fe_attr);
	if (MT_SUCCESS != ret)
	{
		printf("mt_unf_fe_set_attr fail\n");
		return;
	}

	return;
}

/*typedef struct hiAGC_TEST_S
{
	mt_u32	  u32Port;
	mt_u32	  u32Agc1;
	mt_u32	  u32Agc2;
	MT_BOOL	 bLockFlag;
	MT_BOOL	 bAgcLockFlag;
	mt_u8	   u8BagcCtrl12;
	mt_u32	  u32Count;
} AGC_TEST_S;

extern mt_s32 MT_UNF_TUNER_TEST_SINGLE_AGC(mt_u32 u32tunerId, AGC_TEST_S *pstAgcTest);

mt_void mt_tuner_single_agc()
{
	mt_s32 ret = MT_FAILURE;
	AGC_TEST_S stAgcTest = { 0 };


	ret = MT_UNF_TUNER_TEST_SINGLE_AGC(g_fe_port, &stAgcTest);
	if ( MT_SUCCESS != ret )
	{
		printf("mt_tuner_single_agc failed\n");
		return;
	}

	printf("lock = %d, agc_lock = %d, agc2 = %d,	  BAGC_CTRL_12 = %d, end\n",
		stAgcTest.bLockFlag, stAgcTest.bAgcLockFlag, stAgcTest.u32Agc2, stAgcTest.u8BagcCtrl12);

	return;
}*/

/*help function*/
mt_void mt_show_help(char *pCmd)
{
	mt_u32 i_loop = 0;
	mt_u32 u32CmdNum = sizeof(g_cmdhelp) / sizeof(cmd_help_t);

	if (pCmd == MT_NULL_PTR)
	{
		MT_FE_INFO("command list:\n");
		for (i_loop = 0; i_loop < u32CmdNum; i_loop++)
		{
			MT_FE_INFO("%s:%s\n", g_cmdhelp[i_loop].name, g_cmdhelp[i_loop].help_info);
		}
		return;
	}

	for (i_loop = 0; i_loop < u32CmdNum; i_loop++)
	{
		if (0 == strncmp(pCmd, g_cmdhelp[i_loop].name, strlen(g_cmdhelp[i_loop].name)))
		{
			MT_FE_INFO("%s", g_cmdhelp[i_loop].help_info);
		}
	}
}

/* set of received command */
test_func_s g_testfunc[] =
{
	{"ant", mt_fe_set_antena_power},
	{"setdvbs", mt_fe_channel_set_dvbs},
	{"setdvbc", mt_fe_channel_set_dvbc},
	{"setdvbt", mt_fe_channel_set_dvbt},
	{"setsigtype", mt_fe_set_sig_type},
	{"setqam", mt_fe_set_qam},
	{"getsignalinfo", mt_fe_get_signal_info},
	{"setlnbpower", mt_fe_set_lnb_power},
	//{"setlnb", mt_fe_set_lnb },
	{"setplpid", mt_fe_set_plp_id},
	//{"scan", mt_fe_ter_scan },
	//{"blindscan", mt_fe_blindscan },
	{"bsstop", mt_fe_blindscan_stop},
#ifdef CONFIG_MT_DISEQC_SUPPORT
	{"switch", mt_fe_switch_diseqc},
	{"motor", mt_fe_motor},
	{"unic", mt_fe_unicable},
#endif /* CONFIG_MT_DISEQC_SUPPORT */
	{"play", mt_fe_play},
	//   { "getmsc", mt_fe_get_msc },
	//{"getber", mt_fe_get_msc_ber},
	{"select", mt_fe_select_port},
	{"settype", mt_fe_set_type},
	{"getoffset", mt_fe_get_offset},
	{"start", mt_fe_start},
	{"setmode", mt_fe_change_ac_to_b},
	/*{"singleagc", mt_tuner_single_agc},*/
	{"standby", mt_fe_standby},
	{"help", mt_show_help}
};

mt_void fe_show_usage(mt_void)
{
	printf("\n\n==========command list=============\n"
		   "help:show this command list\n"
		   "tunerlock freq sym_rate polar onoff_22k port_type\n"
		   "uclock freq sym_rate polar onoff_22k port_type low_lo high_lo scr_no if_freq_mhz\n"
		   "blindscan mode start_freq stop_freq polar onoff_22k\n"
		   "blindscan mode start_freq stop_freq polar onoff_22k low_lo high_lo scr_no if_freq_mhz\n"
		   "quit\n"
		   "=============================\n\n\n");
}

/*search the handle function corresponding with the command in the character string */
test_func_proc getFunbyName(char *name)
{
	mt_u32 i_loop;
	mt_u32 u32FunNum = sizeof(g_testfunc) / sizeof(test_func_s);

	if (MT_NULL_PTR == name)
	{
		return MT_NULL_PTR;
	}

	for (i_loop = 0; i_loop < u32FunNum; i_loop++)
	{
		if (0 == strncmp(name, g_testfunc[i_loop].name, strlen(g_testfunc[i_loop].name)))
		{
			return g_testfunc[i_loop].proc;
			MT_FE_WARN("here line:%d\n", __LINE__);
		}
	}

	return MT_NULL_PTR;
}

/*deals with the universal command */
mt_s32 procfun(char *funargs)
{
	char *argstr;
	test_func_proc func;

	argstr = strchr(funargs, UDP_STR_SPLIT);
	if (MT_NULL_PTR != argstr)
	{
		*argstr = 0;
		argstr += 1;
	}

	func = getFunbyName(funargs);
	if (MT_NULL_PTR != func)
	{
		func(argstr);
		return MT_SUCCESS;
	}
	else
	{
		strcpy(g_test_result, MT_RESULT_FAIL " Can't find the function\n");
		MT_FE_ERR("Can't find the function  %s %s\n\n", funargs, argstr);
		return MT_FAILURE;
	}
}

int TestTCPOpen()
{
	mt_s32 s32SockFd = -1;
	struct sockaddr_in stAddr;
	s32SockFd = socket(AF_INET, SOCK_STREAM, 0);
	if (s32SockFd < 0)
	{
		MT_FE_ERR("Socket error...\n");
		return -1;
	}

	memset(&stAddr, 0, sizeof(stAddr));
	stAddr.sin_family = AF_INET;
	stAddr.sin_addr.s_addr = INADDR_ANY;
	stAddr.sin_port = htons(DEFAULT_PORT);
	if (bind(s32SockFd, (struct sockaddr *)&stAddr, sizeof(struct sockaddr)) < 0)
	{
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
	mt_char recvbuf[MAX_CMD_BUFFER_LEN];
	mt_s32 s32Socketd = TestTCPOpen();
	mt_s32 ret;

	if (s32Socketd < 0)
	{
		return (void *)0;
	}

	s32Size = sizeof(stCliAddr);
	if ((s32NewFd = accept(s32Socketd, (struct sockaddr *)&stCliAddr, (socklen_t *)&s32Size)) < 0)
	{
		MT_FE_ERR("accept err\n");
		close(s32Socketd);
		return (void *)1;
	}
	MT_FE_INFO("accept socket %d\n", s32NewFd);

	while (1)
	{
		s32RcvLength = read(s32NewFd, recvbuf, 1500);
		if (s32RcvLength > 0)
		{
			recvbuf[s32RcvLength] = 0;
			MT_FE_INFO("receive cmd: %s\n", recvbuf);
			ret = procfun(recvbuf);

			g_test_result[strlen(g_test_result)] = '\n';

			ret = send(s32NewFd, g_test_result, strlen(g_test_result), 0);
			if (ret < 0)
			{
				MT_FE_WARN("send err:%s\n", strerror(errno));
			}
			MT_FE_INFO("%s result:%s\n", recvbuf, g_test_result);

			memset(g_test_result, 0, sizeof(g_test_result));
			/*fflush(newfd);*/
		}
	}
}

mt_s32 mt_fe_connect_dvbs(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 onoff_22k, mt_u32 polar, mt_u32 port_type)
{
	mt_s32 ret = MT_FAILURE;
	mt_unf_fe_status_t stTunerStatus;
	mt_u32 u32Loop = 0;
	mt_u32 u32LoopTimes = 100;
	mt_u32 u32Freq = 0;
	mt_u32 u32SymbolRate = 0;

#if 1
	if (port_type == 0) // DVB-S
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
		g_fe_connect_param.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS;
	}
	else if (port_type == 1) // DVB-S2
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
		g_fe_connect_param.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS2;
	}
	else // DVB-S/S2
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
		g_fe_connect_param.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
	}
	g_fe_connect_param.connect_param.sat.freq = freq * 1000;
	g_fe_connect_param.connect_param.sat.sym_rate = sym_rate;
	g_fe_connect_param.connect_param.sat.onoff_22k = onoff_22k;
	g_fe_connect_param.connect_param.sat.polarization = polar;
#else
	g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
	g_fe_connect_param.connect_param.sat.freq = freq * 1000;
	g_fe_connect_param.connect_param.sat.sym_rate = sym_rate;
	g_fe_connect_param.connect_param.sat.port_type = port_type;
	g_fe_connect_param.connect_param.sat.onoff_22k = onoff_22k;
	g_fe_connect_param.connect_param.sat.polarization = polar;
#endif

	printf("%s() %d: tuner_id = %d, freq = %d, sym_rate = %d, 22k = %d, polar = %d, port_type = %d\n", __FUNCTION__, __LINE__, tuner_id, freq, sym_rate, onoff_22k, polar, port_type);

	ret = mt_unf_fe_connect(tuner_id, &g_fe_connect_param, 0);
	u32Freq = g_fe_connect_param.connect_param.sat.freq;
	u32SymbolRate = g_fe_connect_param.connect_param.sat.sym_rate;

	if (MT_SUCCESS == ret)
	{
		u32LoopTimes = g_fe_connect_param.channel_set_info.lock_time / 10;
		for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
		{
			ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
			if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
			{
				printf("Tuner lock freq %d symb %d polar %d success!\n", u32Freq, u32SymbolRate, polar);
				/*automatically play the first program after locked successfully*/
				printf("SUCCESS end\n");
				//strcpy(s_acTestResult, MT_RESULT_SUCCESS);
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
		printf("%s() %d: Tuner lock freq %d symb %d polar %d fail!, ret = 0x%x\n", __FUNCTION__, __LINE__, u32Freq, u32SymbolRate, polar, ret);
	}

	if (u32Loop == u32LoopTimes)
	{
		printf("%s() %d: Tuner lock freq %d symb %d  polar %d timeout!\n", __FUNCTION__, __LINE__, u32Freq, u32SymbolRate, polar);
	}

	printf("FAIL end\n");
	//strcpy(s_acTestResult, MT_RESULT_FAIL);

	return MT_FAILURE;
}

mt_s32 mt_fe_connect_dvbs_unicable(mt_u32 tuner_id,
								   mt_u32 freq,
								   mt_u32 sym_rate,
								   mt_u32 onoff_22k,
								   mt_u32 polar,
								   mt_u32 port_type,
								   mt_u32 unicable_port_no,
								   mt_u32 user_band,
								   mt_u32 ub_freq_mhz)
{
	mt_s32 ret = MT_FAILURE;
	mt_unf_fe_status_t stTunerStatus;
	mt_u32 u32Loop = 0;
	mt_u32 u32LoopTimes = 100;
	mt_u32 u32Freq = 0;
	mt_u32 u32SymbolRate = 0;

	g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
	g_fe_connect_param.connect_param.sat.freq = freq * 1000;
	g_fe_connect_param.connect_param.sat.sym_rate = sym_rate;
	g_fe_connect_param.connect_param.sat.port_type = port_type;
	g_fe_connect_param.connect_param.sat.onoff_22k = onoff_22k;
	g_fe_connect_param.connect_param.sat.polarization = polar;

	g_fe_connect_param.connect_param.sat.uc_param.use_uc = 1;

	if (MT_UNF_FE_SATPOSN_A == unicable_port_no)
	{
		if (MT_UNF_FE_POLARIZATION_V == polar)
		{
			//g_sat_para[tuner_id].lnb_config.unicable_bank = 1;
			g_fe_connect_param.connect_param.sat.uc_param.bank = onoff_22k ? 1 : 0;
		}
		else
		{
			//g_sat_para[tuner_id].lnb_config.unicable_bank = 3;
			g_fe_connect_param.connect_param.sat.uc_param.bank = onoff_22k ? 3 : 2;
		}
	}
	else
	{
		if (MT_UNF_FE_POLARIZATION_V == polar)
		{
			//g_sat_para[tuner_id].lnb_config.unicable_bank = 5;
			g_fe_connect_param.connect_param.sat.uc_param.bank = onoff_22k ? 5 : 4;
		}
		else
		{
			//g_sat_para[tuner_id].lnb_config.unicable_bank = 7;
			g_fe_connect_param.connect_param.sat.uc_param.bank = onoff_22k ? 7 : 6;
		}
	}

	//g_fe_connect_param.connect_param.sat.uc_param.bank = g_sat_para[tuner_id].lnb_config.unicable_bank;
	g_fe_connect_param.connect_param.sat.uc_param.ub_freq_mhz = ub_freq_mhz;
	g_fe_connect_param.connect_param.sat.uc_param.user_band = user_band;

	ret = mt_unf_fe_connect(tuner_id, &g_fe_connect_param, 0);
	u32Freq = g_fe_connect_param.connect_param.sat.freq;
	u32SymbolRate = g_fe_connect_param.connect_param.sat.sym_rate;

	if (MT_SUCCESS == ret)
	{
		u32LoopTimes = g_fe_connect_param.channel_set_info.lock_time / 10;
		for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
		{
			ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
			if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
			{
				printf("Tuner Lock freq %d symb %d polar%d Success!\n", u32Freq, u32SymbolRate, polar);
				/*automatically play the first program after locked successfully*/
				printf("SUCCESS end\n");
				//strcpy(s_acTestResult, MT_RESULT_SUCCESS);
				return MT_SUCCESS;
			}
			else
			{
				MT_USLEEP(10000);
#if 0
				if (u32Loop % 20 == 0)
					printf("sample line:%d lock_status = %d.\n", __LINE__, stTunerStatus.lock_status);
#endif
			}
		}
	}
	else
	{
		printf("Tuner Lock freq %d symb %d polar%d Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, polar, ret);
	}

	if (u32Loop == u32LoopTimes)
	{
		printf("Tuner Lock freq %d symb %d  polar%d Fail!\n", u32Freq, u32SymbolRate, polar);
	}

	printf("FAIL end\n");
	//strcpy(s_acTestResult, MT_RESULT_FAIL);
	return MT_FAILURE;
}

mt_s32 fe_parse_cmdline(mt_char *pCmdLine, mt_s32 *pArgc, mt_char Argv[FE_MAX_ARGS_COUNT][FE_MAX_ARGS_LEN])
{
	mt_char *ptr = pCmdLine;
	int i;

	while ((*ptr == ' ') && (*ptr++ != '\0'))
	{
		;
	}

	for (i = strlen(ptr); i > 0; i--)
	{
		if ((*(ptr + i - 1) == 0x0a) || (*(ptr + i - 1) == ' '))
		{
			*(ptr + i - 1) = '\0';
		}
		else
		{
			break;
		}
	}

	for (i = 0; i < FE_MAX_ARGS_COUNT; i++)
	{
		int j = 0;
		while ((*ptr == ' ') && (*(++ptr) != '\0'))
		{
			;
		}

		while ((*ptr != ' ') && (*ptr != '\0') && (j < FE_MAX_ARGS_LEN))
		{
			Argv[i][j++] = *ptr++;
		}

		Argv[i][j] = '\0';
		if ('\0' == *ptr)
		{
			i++;
			break;
		}
	}
	*pArgc = i;

	return MT_SUCCESS;
}

mt_s32 fe_run_cmdline(mt_s32 argc, mt_char argv[FE_MAX_ARGS_COUNT][FE_MAX_ARGS_LEN])
{
	mt_s32 ret;

	printf("argv[0] = %s, argc = %d\n", argv[0], argc);

	if (!strcmp(argv[0], "help"))
	{
		fe_show_usage();
	}
	else if (!strcmp(argv[0], "tunerlock"))
	{
		mt_u32 freq, symbol_rate, onoff_22k, polar, port_type;
		///*
		if (argc < 6)
		{
			printf("invalid arguments\n");
			return MT_FAILURE;
		}

		//*/
		freq = strtol(argv[1], NULL, 0);
		symbol_rate = strtol(argv[2], NULL, 0);
		polar = strtol(argv[3], NULL, 0);
		onoff_22k = strtol(argv[4], NULL, 0);
		port_type = strtol(argv[5], NULL, 0);

		printf("%s() %d: tunerlock -- freq = %d, symbol_rate = %d, polar = %d, 22k = %d, port_type = %d\n", __FUNCTION__, __LINE__, freq, symbol_rate, polar, onoff_22k, port_type);

		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);
		ret = mt_fe_connect_dvbs(g_fe_dvbs_port, freq, symbol_rate, onoff_22k, polar, port_type);
		if (MT_SUCCESS != ret)
		{
			printf("tuner lock failed\n");
			return MT_FAILURE;
		}
		/*
		else
		{
			printf("tuner lock success\n");
		}
		*/
#if 0
		mt_u32 s_snr = 0, s_ber = 0, s_strength = 0, s_quality = 0;
		mt_u32 k = 0;
		for(k = 0; k < 10; k++)
		{
			mt_unf_fe_get_snr(g_fe_dvbs_port, &s_snr);
			mt_unf_fe_get_ber(g_fe_dvbs_port, &s_ber);
			mt_unf_fe_get_signal_strength(g_fe_dvbs_port, &s_strength);
			mt_unf_fe_get_signal_quality(g_fe_dvbs_port, &s_quality);
			printf("s_snr[%d], s_ber[%d], s_strength[%d], s_quality[%d]\n", s_snr, s_ber, s_strength, s_quality);
			sleep(10);
		}
#endif
	}
	else if (!strcmp(argv[0], "uclock"))
	{
		mt_u32 freq, symbol_rate, polar, onoff_22k, port_type, low_lo, high_lo, port_no, scr_no, if_freq_mhz;
		if ((argc < 10) || (argc > 10))
		{
			printf("invalid arguments\n");
			return MT_FAILURE;
		}

		freq = strtol(argv[1], NULL, 0);
		symbol_rate = strtol(argv[2], NULL, 0);
		polar = strtol(argv[3], NULL, 0);
		onoff_22k = strtol(argv[4], NULL, 0);
		port_type = strtol(argv[5], NULL, 0);

		low_lo = strtol(argv[6], NULL, 0);
		high_lo = strtol(argv[7], NULL, 0);

		port_no = 0; //strtol(argv[8], NULL, 0);
		scr_no = strtol(argv[8], NULL, 0);
		if_freq_mhz = strtol(argv[9], NULL, 0);

		mt_fe_set_lnb(low_lo, high_lo, port_no, scr_no, if_freq_mhz);
		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);

		ret = mt_fe_connect_dvbs_unicable(g_fe_dvbs_port, freq, symbol_rate, onoff_22k, polar, port_type, port_no, scr_no, if_freq_mhz);
#if 0
		mt_u32 s_snr = 0, s_ber = 0, s_strength = 0, s_quality = 0;
		mt_u32 k = 0;
		for(k = 0; k < 10; k++)
		{
			mt_unf_fe_get_snr(g_fe_dvbs_port, &s_snr);
			//printf("sample line[%d]\n", __LINE__);
			mt_unf_fe_get_ber(g_fe_dvbs_port, &s_ber);
			//printf("sample line[%d]\n", __LINE__);
			mt_unf_fe_get_signal_strength(g_fe_dvbs_port, &s_strength);
			//printf("sample line[%d]\n", __LINE__);
			mt_unf_fe_get_signal_quality(g_fe_dvbs_port, &s_quality);
			printf("s_snr[%d], s_ber[%d], s_strength[%d], s_quality[%d]\n", s_snr, s_ber, s_strength, s_quality);
			sleep(10);
		}
#endif
	}
	else if (!strcmp(argv[0], "blindscan"))
	{
		if (argc < 5)
		{
			printf("invalid arguments\n");
			return MT_FAILURE;
		}

		mt_u32 bs_mode, start_freq, stop_freq, polar, onoff_22k;
		bs_mode = strtol(argv[1], NULL, 0);
		start_freq = strtol(argv[2], NULL, 0);
		stop_freq = strtol(argv[3], NULL, 0);
		polar = strtol(argv[4], NULL, 0);
		onoff_22k = strtol(argv[5], NULL, 0);
		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);
		mt_fe_blindscan(bs_mode, start_freq, stop_freq, polar, onoff_22k);
	}
	else if (!strcmp(argv[0], "ucblindscan"))
	{
		if ((argc < 10) || (argc > 10))
		{
			printf("invalid arguments\n");
			return MT_FAILURE;
		}

		mt_u32 bs_mode, start_freq, stop_freq, polar, onoff_22k, low_lo, high_lo, port_no, scr_no, if_freq_mhz;
		bs_mode = strtol(argv[1], NULL, 0);
		start_freq = strtol(argv[2], NULL, 0);
		stop_freq = strtol(argv[3], NULL, 0);
		polar = strtol(argv[4], NULL, 0);
		onoff_22k = strtol(argv[5], NULL, 0);

		low_lo = strtol(argv[6], NULL, 0);
		high_lo = strtol(argv[7], NULL, 0);

		port_no = 0; //strtol(argv[8], NULL, 0);
		scr_no = strtol(argv[8], NULL, 0);
		if_freq_mhz = strtol(argv[9], NULL, 0);

		mt_fe_set_lnb(low_lo, high_lo, port_no, scr_no, if_freq_mhz);
		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);
		mt_fe_blindscan(bs_mode, start_freq, stop_freq, polar, onoff_22k);
	}
#ifdef CONFIG_MT_DISEQC_SUPPORT
	else if (!strcmp(argv[0], "diseqc"))
	{
		mt_unf_fe_diseqc_switch4port_t st4Port;
		mt_u32 port, freq, symbol_rate, polar, onoff_22k, port_type;
		port = strtol(argv[1], NULL, 0);
		freq = strtol(argv[2], NULL, 0);
		symbol_rate = strtol(argv[3], NULL, 0);
		polar = strtol(argv[4], NULL, 0);
		onoff_22k = strtol(argv[5], NULL, 0);
		port_type = strtol(argv[6], NULL, 0);

		st4Port.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		st4Port.port = (mt_unf_fe_diseqc_switch_port_t)port;
		st4Port.polar = polar;
		st4Port.lnb_22k = onoff_22k;
		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);
		ret = mt_unf_fe_diseqc_switch4port(g_fe_dvbs_port, &st4Port);
		if (MT_SUCCESS != ret)
		{
			printf("switch port failed\n");
			return MT_FAILURE;
		}

		ret = mt_fe_connect_dvbs(g_fe_dvbs_port, freq, symbol_rate, onoff_22k, polar, port_type);
		if (MT_SUCCESS != ret)
		{
			printf("tuner lock failed\n");
			return MT_FAILURE;
		}

		mt_u32 i = 0;
		mt_s32 agc = 0;
		for (i = 0; i < 10; i++)
		{
			ret = mt_unf_fe_get_agc(g_fe_dvbs_port, 0, &agc);
			if (MT_SUCCESS != ret)
			{
				printf("tuner get agc failed\n");
				return MT_FAILURE;
			}
			printf("agc[%d]\n", agc);
			sleep(10);
		}
	}
	else if (!strcmp(argv[0], "diseqc2"))
	{
		mt_unf_fe_diseqc_switch16port_t st16Port;
		mt_u32 port, freq, symbol_rate, polar, onoff_22k, port_type;
		port = strtol(argv[1], NULL, 0);
		freq = strtol(argv[2], NULL, 0);
		symbol_rate = strtol(argv[3], NULL, 0);
		polar = strtol(argv[4], NULL, 0);
		onoff_22k = strtol(argv[5], NULL, 0);
		port_type = strtol(argv[6], NULL, 0);

		st16Port.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		st16Port.port = (mt_unf_fe_diseqc_switch_port_t)port;

		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);

		ret = mt_unf_fe_diseqc_switch16port(g_fe_dvbs_port, &st16Port);
		if (MT_SUCCESS != ret)
		{
			printf("switch port failed\n");
			return MT_FAILURE;
		}

		ret = mt_fe_connect_dvbs(g_fe_dvbs_port, freq, symbol_rate, onoff_22k, polar, port_type);
		if (MT_SUCCESS != ret)
		{
			printf("tuner lock failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "storepos"))
	{
		mt_unf_fe_diseqc_position_t stPos;
		mt_u32 u32Para1;
		u32Para1 = strtol(argv[1], NULL, 0);
		stPos.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stPos.pos = u32Para1;
		ret = mt_unf_fe_diseqc_storepos(g_fe_dvbs_port, &stPos);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "gotopos"))
	{
		mt_u32 u32Para1;
		mt_unf_fe_diseqc_position_t stPos;
		u32Para1 = strtol(argv[1], NULL, 0);

		stPos.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stPos.pos = u32Para1;
		ret = mt_unf_fe_diseqc_goto_pos(g_fe_dvbs_port, &stPos);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "limit"))
	{
		mt_u32 u32Para1;
		mt_unf_fe_diseqc_limit_t stLimit;
		u32Para1 = strtol(argv[1], NULL, 0);

		stLimit.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stLimit.limit = (mt_unf_fe_diseqc_dir_limit_t)u32Para1;
		ret = mt_unf_fe_diseqc_set_limit(g_fe_dvbs_port, &stLimit);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "move"))
	{
		mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);
		//mt_unf_fe_set_lnb_power(g_fe_dvbs_port, MT_UNF_FE_LNB_POWER_ON);

		mt_u32 u32Para1;
		mt_u32 u32Para2;
		mt_unf_fe_diseqc_move_t stMove;

		u32Para1 = strtol(argv[1], NULL, 0);
		u32Para2 = strtol(argv[2], NULL, 0);

		stMove.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stMove.dir = (mt_unf_fe_diseqc_move_dir_t)u32Para1;
		stMove.type = (mt_unf_fe_diseqc_move_type_t)u32Para2;
		ret = mt_unf_fe_diseqc_move(g_fe_dvbs_port, &stMove);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "stop"))
	{
		ret = mt_unf_fe_diseqc_stop(g_fe_dvbs_port, MT_UNF_FE_DISEQC_LEVEL_1_X);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "goto_angular"))
	{
		mt_u32 u32Para1;
		mt_u32 u32Para2;
		mt_u32 u32Para3;
		mt_unf_fe_diseqc_usals_para_t stUSALS;
		mt_unf_fe_diseqc_usals_angular_t stAngular;

		u32Para1 = strtol(argv[1], NULL, 0);
		u32Para2 = strtol(argv[2], NULL, 0);
		u32Para3 = strtol(argv[3], NULL, 0);

		stUSALS.local_longitude = (mt_u16)u32Para1;
		stUSALS.local_latitude = (mt_u16)u32Para2;
		stUSALS.sat_longitude = (mt_u16)u32Para3;
		ret = mt_unf_fe_diseqc_calc_angular(g_fe_dvbs_port, &stUSALS);
		MT_FE_INFO("Angular: %02x, %02x\n", (mt_u8)(stUSALS.angular >> 8), (mt_u8)stUSALS.angular);
		stAngular.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stAngular.angular = stUSALS.angular;
		ret |= mt_unf_fe_diseqc_goto_angular(g_fe_dvbs_port, &stAngular);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "recalculate"))
	{
		mt_u32 u32Para1;
		mt_u32 u32Para2;
		mt_u32 u32Para3;
		mt_unf_fe_diseqc_recalculate_t stRecal;

		u32Para1 = strtol(argv[1], NULL, 0);
		u32Para2 = strtol(argv[2], NULL, 0);
		u32Para3 = strtol(argv[3], NULL, 0);

		stRecal.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stRecal.para1 = (mt_u8)u32Para1;
		stRecal.para2 = (mt_u8)u32Para2;
		stRecal.para3 = (mt_u8)u32Para3;
		ret = mt_unf_fe_diseqc_recalculate(g_fe_dvbs_port, &stRecal);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
	else if (!strcmp(argv[0], "goto_angular"))
	{
		mt_u32 u32Para1;
		mt_unf_fe_diseqc_usals_angular_t stAngular;
		u32Para1 = strtol(argv[1], NULL, 0);

		stAngular.level = MT_UNF_FE_DISEQC_LEVEL_1_X;
		stAngular.angular = u32Para1;
		ret = mt_unf_fe_diseqc_goto_angular(g_fe_dvbs_port, &stAngular);
		if (MT_SUCCESS != ret)
		{
			printf("diseqc storepos failed\n");
			return MT_FAILURE;
		}
	}
#endif
	else if (!strcmp(argv[0], "quit"))
	{
		g_fe_bRun = MT_FALSE;
	}
	else
	{
		printf("invalid command\n");
		fe_show_usage();
	}

	return MT_SUCCESS;
}

mt_s32 main(mt_s32 argc, char *argv[])
{
#if 0
	mt_s32 ret = MT_FAILURE;
	mt_u8 first_param = 0;
	mt_u32 freq = 0;
	mt_u32 symbol_rate = 0;
	mt_u32 polar = 0;
	mt_u32 onoff_22k = 0;
	mt_u32 band_width = 8;
	mt_u32 port_type = 0;

	mt_u8 scr_no = 0;
	mt_u8 port_no = 0;

	/* init device */
	ret = dev_init();
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, ret);
		return ret;
	}

	first_param = strtol(argv[1], NULL, 0);

	if (first_param == 1)
	{
		//mt_u32 start_freq = 0, stop_freq = 0;
		//start_freq = strtol(argv[2], NULL, 0);
		//stop_freq = strtol(argv[2], NULL, 0);
		//polar = strtol(argv[3], NULL, 0);
		//onoff_22k = strtol(argv[4], NULL, 0);
		//port_type = strtol(argv[5], NULL, 0);
		//ret = mt_fe_connect_dvbs(g_fe_dvbs_port, freq, symbol_rate, onoff_22k, polar, port_type);

		polar = strtol(argv[2], NULL, 0);
		onoff_22k = strtol(argv[3], NULL, 0);
		mt_fe_blindscan(polar, onoff_22k);
	}
	else if (first_param == 2)
	{
		scr_no = strtol(argv[2], NULL, 0);
		port_no = strtol(argv[3], NULL, 0);
		mt_fe_set_lnb(scr_no, port_no);

		polar = strtol(argv[4], NULL, 0);
		onoff_22k = strtol(argv[5], NULL, 0);
		mt_fe_blindscan(polar, onoff_22k);
	}
	else
	{
		freq = strtol(argv[1], NULL, 0);
		symbol_rate = strtol(argv[2], NULL, 0);
		polar = strtol(argv[3], NULL, 0);
		onoff_22k = strtol(argv[4], NULL, 0);
		port_type = strtol(argv[5], NULL, 0);

		scr_no = strtol(argv[6], NULL, 0);
		port_no = strtol(argv[7], NULL, 0);
		mt_fe_set_lnb(scr_no, port_no);

		ret = mt_fe_connect_dvbs(g_fe_dvbs_port, freq, symbol_rate, onoff_22k, polar, port_type);
	}
#endif

#if 1
	mt_s32 ret = MT_FAILURE;
	mt_u8 first_param = 0;
	mt_u32 freq = 0;
	mt_u32 symbol_rate = 0;
	mt_u32 polar = 0;
	mt_u32 onoff_22k = 0;
	mt_u32 band_width = 8;
	mt_u32 port_type = 0;

	mt_u8 scr_no = 0;
	mt_u8 port_no = 0;

	/* init device */
	ret = dev_init();
	if (MT_SUCCESS != ret)
	{
		MT_FE_ERR("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, ret);
		return ret;
	}

#if 0
	while(1)
	{
		mt_char InputCmd[32];
		printf("please input the q to quit!\n");
		fgets(InputCmd, 30, stdin);
		if ('q' == InputCmd[0])
		{
			printf("prepare to quit!\n");
			g_fe_bRun = MT_FALSE;
			break;
		}
	}

	fe_show_usage();
#endif

	while (g_fe_bRun)
	{
		mt_char *pCmdLine, chRet;
		printf("\n>");
		pCmdLine = fgets(g_fe_cmd_line, FE_MAX_CMDLINE_LEN, stdin);
		if (MT_NULL == pCmdLine)
		{
			fe_show_usage();
			continue;
		}

		printf("pCmdLine = %s\n", pCmdLine);
		chRet = fe_parse_cmdline(pCmdLine, &g_fe_argc, g_fe_argv);
		if (MT_SUCCESS != chRet || 0 == g_fe_argc)
		{
			fe_show_usage();
			continue;
		}

		(mt_void) fe_run_cmdline(g_fe_argc, g_fe_argv);
	}
#endif

	dev_deinit();

	return ret;
}
