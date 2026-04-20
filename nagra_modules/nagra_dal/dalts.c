#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "ca_cak.h"
#include "mt_unf_cipher_v2.h"
#include "mt_unf_frontend.h"
#include "mt_unf_demux.h"
#include "mt_unf_misc.h"
#include "mt_adp_search.h"
#include "mt_unf_ir.h"
#include "mt_unf_avplay.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_vo.h"
#include "mt_go.h"
//#include "mtgo_surface.h"
#include "mt_unf_flash.h"
#include "mt_sec_ext.h"
#include "nocs_csd_impl.h"
#include "nocs_csd.h"
#include "nocs_sec_impl.h"
#include "ca_sec.h"

#include "mt_adp_mpi.h"
#include "mt_hdmi.h"
#include <semaphore.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define OPC_NETLINK_ID         24
#define OPC_MAX_RECV_SIZE      64 

#define TUNER_ID 1
#define DMX_ID_LIVE 0
//#define USE_MONT_PID_API
typedef struct hiCODEC_VIDEO_CMD_S
{
    mt_u32      u32CmdID;   /**<Commond ID*/ /**<CNcomment: 命令ID*/
    mt_void     *pPara;     /**<Control parameter*/ /**<CNcomment: 命令携带参数*/
}MT_CODEC_VIDEO_CMD_S;

PMT_COMPACT_TBL *g_pProgTbl = MT_NULL;
static MT_HANDLE	hAvplay;

static mt_unf_fe_connect_para_t g_fe_connect_param;
mt_s32 DALTS_StartLivePlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo);
mt_s32 DALTS_StopLivePlay(mt_handle hAvplay);
extern MT_S32 MT_DrawInit();
extern MT_S32 MT_DrawText(mt_char *inputCmd,mt_s32 x, mt_s32 y, mt_s32 w, mt_s32 h);

void mt_cert_init()
{
	int ret = 0;

	//ret = mt_sys_init();
	ret |= mt_unf_misc_init();
	ret |= mt_unf_cipher_init();
	ret |= mt_unf_misc_module_set(HAL_KT, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_DES, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_TDES, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_AES, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_SHA, 1);
	ret |= mt_unf_misc_module_set(HAL_CRYPTO_RSA, 1);
	ret |= mt_unf_misc_module_set(HAL_KL_CW, 1);
	ret |= mt_unf_misc_module_set(HAL_SECHD0, 1); // enable cert clock!
	printf("mt_cert_init ret 0x%x \n", ret);
	mt_s32 data = 0;
	mt_sys_read_register(0xbf50c000, &data);
	printf("value 0x%x \n", data);
	data |= (1 << 3); //enable cert clock!
	mt_sys_write_register(0xbf50c000, data);
	mt_sys_read_register(0xbf50c000, &data);
	printf("value == 0x%x \n", data);

}

static mt_s32 dev_init()
{
    mt_s32 ret = 0;
    mt_unf_fe_attr_t fe_attr;
    /*sys init*/
    mt_sys_init();
	mt_unf_misc_init();
#if 0
	printf("line --> %d, ret=%d\n", __LINE__, ret);
	ret = mt_unf_misc_module_set(HAL_DS_DES, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_DS_DES);
	ret = mt_unf_misc_module_set(HAL_DS_TDES, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_DS_TDES);
	ret = mt_unf_misc_module_set(HAL_DS_AES, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_DS_AES);
	ret = mt_unf_misc_module_set(HAL_DS_CSA3, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_DS_CSA3);
	ret = mt_unf_misc_module_set(HAL_DS_CSA2, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_DS_CSA2);
	ret = mt_unf_misc_module_set(HAL_DS_SECHD1, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_DS_SECHD1);
	ret = mt_unf_misc_module_set(HAL_SECHD0, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_SECHD0);
	ret = mt_unf_misc_module_set(HAL_KT, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_KT);
	ret = mt_unf_misc_module_set(HAL_RNG2, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_RNG2);
	ret = mt_unf_misc_module_set(HAL_KL_CW, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_KL_CW);
	ret = mt_unf_misc_module_set(HAL_KL_PVR, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_KL_PVR);
	ret = mt_unf_misc_module_set(HAL_KDF, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_KDF);
	ret = mt_unf_misc_module_set(HAL_CRYPTO, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_CRYPTO);
	ret = mt_unf_misc_module_set(HAL_CRYPTO_DES, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_CRYPTO_DES);
	ret = mt_unf_misc_module_set(HAL_CRYPTO_TDES, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_CRYPTO_TDES);
	ret = mt_unf_misc_module_set(HAL_CRYPTO_AES, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_CRYPTO_AES);
	ret = mt_unf_misc_module_set(HAL_CRYPTO_SHA, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_CRYPTO_SHA);
	ret = mt_unf_misc_module_set(HAL_CRYPTO_RSA, 1);
	printf("line --> %d, ret=%d mid=0x%x\n", __LINE__, ret, HAL_CRYPTO_RSA);
#endif	

	mt_unf_cipher_init();

	ret = mt_unf_fe_init();
	if (MT_SUCCESS != ret)
	{
		printf("call mt_unf_fe_init failed.\n");
		return ret;
    }

    /* open Tuner */
    ret = mt_unf_fe_open(TUNER_ID);
    if (MT_SUCCESS != ret)
	{
		printf("call mt_unf_fe_open failed.\n");
		mt_unf_fe_deinit();
		return ret;
    }

    /* get default attribute */
    ret = mt_unf_fe_get_default_attr(TUNER_ID, &fe_attr);
    if (MT_SUCCESS != ret)
	{
		printf("call mt_unf_fe_get_default_attr failed.\n");
		mt_unf_fe_close(TUNER_ID);
		mt_unf_fe_deinit();
		return ret;
    }

    fe_attr.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
    ret = mt_unf_fe_set_attr(TUNER_ID, &fe_attr);
    if (MT_SUCCESS != ret)
	{
		printf("call mt_unf_fe_set_attr failed.\n");
		return ret;
    }

    return MT_SUCCESS;
}

static mt_s32 mt_fe_connect_dvbs(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 onoff_22k, mt_u32 polar, mt_u32 port_type)
{	mt_s32 ret = MT_FAILURE;
	mt_unf_fe_status_t stTunerStatus;
	mt_u32 u32Loop = 0;
	mt_u32 u32LoopTimes = 100;
	mt_u32 u32Freq = 0;
	mt_u32 u32SymbolRate = 0;

	if(port_type == 0)			// DVB-S
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
		g_fe_connect_param.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS;
	}
	else if(port_type == 1)		// DVB-S2
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
		g_fe_connect_param.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS2;
	}
	else						// DVB-S/S2
	{
		g_fe_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
	   	g_fe_connect_param.connect_param.sat.port_type = MT_UNF_PORT_TYPE_DVBS_AUTO;
	}
	g_fe_connect_param.connect_param.sat.freq = freq * 1000;
	g_fe_connect_param.connect_param.sat.sym_rate = sym_rate;
	g_fe_connect_param.connect_param.sat.onoff_22k = onoff_22k;
	g_fe_connect_param.connect_param.sat.polarization = polar;

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
				usleep(10000);
			}
		}
	}

	return MT_FAILURE;
}

static mt_void* signal_monitor_task(mt_void* args)
{
        mt_s32 ret;
        mt_unf_fe_status_t stTunerStatus;
        printf("signal_monitor_task ----->\n");
        while(1) {
        	ret = mt_unf_fe_get_status(TUNER_ID, &stTunerStatus);

			if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
			{
			       usleep(500000);
				continue;
			}
			else
			{
			       printf("signal_monitor_task --> UNLOCK\n");
			       mt_fe_connect_dvbs(TUNER_ID,4150,27500,0, 0, 2);
				usleep(100000);
			}
       }     
}

#define YELLOW_KEY	0x58

static void programs_search(void)
{
	mt_s32 s32Ret;
	int i;
	MTADP_Search_FreeAllPmt(g_pProgTbl);
	MTADP_Search_DeInit();
	MTADP_Search_Init();	
	s32Ret = MTADP_Search_GetAllPmt(DMX_ID_LIVE,&g_pProgTbl);
	if (MT_SUCCESS != s32Ret){
		printf("call MTADP_Search_GetAllPmt failed\n");
		return s32Ret;
	}
	printf("Prog Num: %d,g_pProgTbl:0x%x\n", g_pProgTbl->prog_num,g_pProgTbl);
	
	for (i = 0; i < g_pProgTbl->prog_num; i++) {
		printf("i:%d,ProgID:%d\n",i,g_pProgTbl->proginfo[i].ProgID);
	}
	
	printf("ProgID:%d,ProgID:%d\n",g_pProgTbl->proginfo[0].ProgID,g_pProgTbl->proginfo[1].ProgID);
	printf("proginfo 0:0x%x,proginfo 1:0x%x\n",&g_pProgTbl->proginfo[0],&g_pProgTbl->proginfo[1]);
}

static PMT_COMPACT_PROG * get_program_by_service_id(mt_u32 id)
{
	int i;
	for (i = 0; i < g_pProgTbl->prog_num; i ++) {
		//printf("ProgID:%d\n",g_pProgTbl->proginfo[i].ProgID);
		if (g_pProgTbl->proginfo[i].ProgID == id) {
			return &g_pProgTbl->proginfo[i];
		}
	}
	return NULL;
}

#ifdef USE_MONT_PID_API
static mt_u16 cur_vpid = 0x1fff, cur_apid = 0x1fff;
static mt_u16 vpid[2] = {0x1fff, 0x1fff};
static mt_u16 apid[2] = {0x1fff, 0x1fff};

void NocsSecEventCallback(MT_SEC_EVENT_E event, TTransportSessionId tsid)
{
	printf("%s -> tsid=%d, event=%d\n", __FUNCTION__, tsid, event);
	TSecPidInfo pids;
	vpid[tsid] = cur_vpid;
	apid[tsid] = cur_apid;
	if (event == MT_SEC_EVENT_OPEN) {
		pids.pidList[0].pid= vpid[tsid];
		pids.pidList[0].type = MT_UNF_DMX_CHAN_TYPE_VID;
		pids.pidList[1].pid = apid[tsid];
		pids.pidList[1].type = MT_UNF_DMX_CHAN_TYPE_AUD; 
		pids.sessionType = TRANSPORT_SESSION_TYPE_DVB;
		pids.tsid = tsid;
		pids.pidNum = 2;
		mtSecSetSessionPid(tsid,&pids,0);
	} else if (event == MT_SEC_EVENT_CLOSE) {
		pids.pidList[0].pid= vpid[tsid];
		pids.pidList[0].type = MT_UNF_DMX_CHAN_TYPE_VID; 
		pids.pidList[1].pid = apid[tsid];
		pids.pidList[1].type = MT_UNF_DMX_CHAN_TYPE_AUD; 
		pids.sessionType = TRANSPORT_SESSION_TYPE_DVB;
		pids.tsid = tsid;	
		pids.pidNum = 2;
		mtSecClearSessionPid(tsid, SESSION_OP_ENCRYPT, &pids);
	}
}
#endif

static mt_void* ir_monitor_task(mt_void* args)
{
	int ret;
	mt_u64 key;
	mt_u32 Usercode = 0;
	mt_u32 Keycode = 0;
	char name[64];
	MT_UNF_KEY_STATUS_E status;
	unsigned int read_timeout = 200;
	PMT_COMPACT_PROG* pmt_prog = NULL;
	MT_RECT rc = {100, 90, 500, 30};
	char Text[1024];
#ifdef USE_MONT_PID_API
	mtSecSetEventCallback(MT_SEC_EVENT_OPEN, NocsSecEventCallback);
	mtSecSetEventCallback(MT_SEC_EVENT_CLOSE, NocsSecEventCallback);
#endif
	while (1) 
	{
		ret = MT_UNF_IR_GetValueWithProtocol(&status, &key, name, sizeof(name), read_timeout);
		if (!ret) 
		{
			printf("Received key: 0x%.08llx, %s,\tprotocol: %s.\n", key, status == MT_UNF_KEY_STATUS_DOWN ? "DOWN" : (status == MT_UNF_KEY_STATUS_UP ? "UP" : "HOLD"), name);
			MT_UNF_IR_GetUserAndKey(name, &key, &Usercode, &Keycode);
			printf("Keycode 0x%x,Usercode:0x%x\n",Keycode,Usercode);
			if (Keycode == YELLOW_KEY) {				
				MT_DrawInit();				
				memset(Text, 0, sizeof(Text));
				sprintf(Text, "Searching Programs >>>");
				MT_DrawText("Searching Programs >>>",100,100,1000,60);
				programs_search();
				//pmt_prog = get_program_by_service_id(12);				
				pmt_prog = get_program_by_service_id(4222);
				if (pmt_prog == NULL) {
					pmt_prog = get_program_by_service_id(32);
					if (pmt_prog == NULL) {
						pmt_prog = get_program_by_service_id(11);
						if (pmt_prog == NULL)
							continue;
					}
				}
				memset(Text, 0, sizeof(Text));
				sprintf(Text, "Service %d > vpid=%d / apid=%d", 
					pmt_prog->ProgID, pmt_prog->VElementPid, pmt_prog->AElementPid);
				MT_DrawText(Text,100,200,1000,60);				
				DALTS_StopLivePlay(hAvplay);
				DALTS_StartLivePlay(hAvplay, pmt_prog);
				#ifdef USE_MONT_PID_API
				cur_vpid = pmt_prog->VElementPid;
				cur_apid = pmt_prog->AElementPid;
				#endif
			}
		}

		usleep(50000);	
	}

	return (void *)0;
}

mt_s32 DALTS_AvplayInit(mt_handle hWin, mt_handle *phAvplay, mt_handle* phSoundTrack)
{
	mt_s32                  Ret;
	mt_handle               hAvplay;
	MT_UNF_AVPLAY_ATTR_S        AvplayAttr;
	MT_UNF_SYNC_ATTR_S          SyncAttr;
	MT_UNF_AUDIOTRACK_ATTR_S  stTrackAttr;

	if (phSoundTrack == NULL)
	{
		return MT_FAILURE;
	}

	Ret = MTADP_AVPlay_RegADecLib();
	if (Ret != MT_SUCCESS)
	{
		printf("call MTADP_AVPlay_RegADecLib failed.\n");
		return Ret;
	}


	Ret = MT_UNF_AVPLAY_Init();
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_Init failed.\n");
		return Ret;
	}

	Ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	AvplayAttr.u32DemuxId = DMX_ID_LIVE;
	AvplayAttr.stStreamAttr.u32VidBufSize = 2*1024*1024;
	AvplayAttr.stStreamAttr.u32AudBufSize = 192*1024;

	Ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_Create failed.\n");
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	Ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_GetAttr failed.\n");
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	SyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
	SyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
	SyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
	SyncAttr.u32PreSyncTimeoutMs = 1000;
	SyncAttr.bQuickOutput = MT_FALSE;

	Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &SyncAttr);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
		MT_UNF_AVPLAY_Destroy(hAvplay);
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	Ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_AVPLAY_ChnOpen failed.\n");
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
		MT_UNF_AVPLAY_Destroy(hAvplay);
		return Ret;
	}

	Ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_VO_AttachWindow failed.\n");
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
		MT_UNF_AVPLAY_Destroy(hAvplay);
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	Ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_VO_SetWindowEnable failed.\n");
		MT_UNF_VO_DetachWindow(hWin, hAvplay);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
		MT_UNF_AVPLAY_Destroy(hAvplay);
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	Ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
	if (Ret != MT_SUCCESS)
	{
		printf("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
		return Ret;
	}
	Ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0,&stTrackAttr, phSoundTrack);
	if (Ret != MT_SUCCESS)
	{
		MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
		MT_UNF_VO_DetachWindow(hWin, hAvplay);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
		MT_UNF_AVPLAY_Destroy(hAvplay);
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	Ret = MT_UNF_SND_Attach(*phSoundTrack, hAvplay);
	if (Ret != MT_SUCCESS)
	{
		MT_UNF_SND_DestroyTrack(*phSoundTrack);
		printf("call MT_SND_Attach failed.\n");
		MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);
		MT_UNF_VO_DetachWindow(hWin, hAvplay);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);
		MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);
		MT_UNF_AVPLAY_Destroy(hAvplay);
		MT_UNF_AVPLAY_DeInit();
		return Ret;
	}

	*phAvplay = hAvplay;
	return MT_SUCCESS;
}

mt_s32 DALTS_SetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
	MT_UNF_AVPLAY_STOP_OPT_S    Stop;

	mt_u32                  VidPid;
	mt_u32                  AudPid;

	MT_UNF_VCODEC_ATTR_S        VdecAttr;
	MT_UNF_ACODEC_ATTR_S        AdecAttr;
	mt_s32                  Ret = 0;
	MT_UNF_VCODEC_TYPE_E    enVidType;
	mt_u32                  u32AudType;

	MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
	MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);

	if(pProgInfo == NULL)  
	{
		printf("=====pProgInfo == NULL=====\n");
		return Ret;
	}
	if (pProgInfo->VElementNum > 0 )
	{
		VidPid = pProgInfo->VElementPid;
		enVidType = pProgInfo->VideoType;
	}
	else
	{
		VidPid = INVALID_TSPID;
		enVidType = MT_UNF_VCODEC_TYPE_BUTT;
	}

	if (pProgInfo->AElementNum > 0)
	{
		AudPid  = pProgInfo->AElementPid;
		u32AudType = pProgInfo->AudioType;
	}
	else
	{
		AudPid = INVALID_TSPID;
		u32AudType = 0xffffffff;
	}

    	printf("VidPid=%x, AudPid=%x\n",VidPid,AudPid); 
	if (VidPid != INVALID_TSPID)
	{
		VdecAttr.enType = enVidType;
		Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
		if (Ret != MT_SUCCESS)
		{
			printf("call MT_UNF_AVPLAY_SetAttr failed.\n");
			return Ret;
		}

		Ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
		if (Ret != MT_SUCCESS)
		{
			printf("call MTADP_AVPlay_SetVdecAttr failed.\n");
			return Ret;
		}
	}

	if (AudPid != INVALID_TSPID)
	{
		Ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
		Ret |= MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
		if (MT_SUCCESS != Ret)
		{
			printf("MTADP_AVPlay_SetAdecAttr failed:%#x\n",Ret);
			return Ret;
		}
	}
	if(1){//insert pts
		if ((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID)) {
			MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S DmxAvsync;
			DmxAvsync.VdecType = enVidType;
			DmxAvsync.AdecType = u32AudType;
			DmxAvsync.AvsyncFlage = 1; // 1--insert pts 0--do not insert pts
			MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
		}
	}

	return MT_SUCCESS;
}

mt_s32 DALTS_StartLivePlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
	mt_u32 ret = MT_SUCCESS;
	mt_u32 pid = 0;
	MT_UNF_AVPLAY_MEDIA_CHAN_E enMediaType = 0;
	MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = {0};
	MT_UNF_SYNC_ATTR_S stSyncAttr = {0};
	MT_CODEC_VIDEO_CMD_S  stVdecCmdPara = {0};
	MT_UNF_AVPLAY_TPLAY_OPT_S stTplayOpts = {0};

	DALTS_SetAvplayPidAndCodecType(hAvplay, pProgInfo);

	ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID, &pid);
	if ((MT_SUCCESS != ret) || (0x1fff == pid))
	{
		printf("has no audio stream!\n");
	}
	else
	{
		enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
	}

	ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID, &pid);
	if ((MT_SUCCESS != ret) || (0x1fff == pid))
	{
		printf("has no video stream!\n");
	}
	else
	{
		enMediaType |= MT_UNF_AVPLAY_MEDIA_CHAN_VID;
	}

	if ((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
	{
		/*enable vo frame rate detect*//*CNcomment:使能VO自动帧率检测*/
		stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
		stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
		stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
		if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr))
		{
			printf("set frame to VO fail.\n");
			return MT_FAILURE;
		}

		/*enable avplay A/V sync*//*CNcomment:使能avplay音视频同步*/
		if (MT_SUCCESS != MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
		{
			printf("get avplay sync attr fail!\n");
			return MT_FAILURE;
		}

		stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
		stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
		stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
		stSyncAttr.u32PreSyncTimeoutMs = 1000;
		stSyncAttr.bQuickOutput = MT_FALSE;

		if (MT_SUCCESS != MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr))
		{
			printf("set avplay sync attr fail!\n");
			return MT_FAILURE;
		}
	}

	/*start to play audio and video*//*CNcomment:开始音视频播放*/
	ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
	if (MT_SUCCESS != ret)
	{
		return ret;
	}

	/*set avplay trick mode to normal*//*CNcomment:设置avplay特技模式为正常*/
	if (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID)
	{
		stTplayOpts.enTplayDirect = MT_UNF_AVPLAY_TPLAY_DIRECT_FORWARD;
		stTplayOpts.u32SpeedInteger = 1;
		stTplayOpts.u32SpeedDecimal = 0;
		stVdecCmdPara.u32CmdID = MT_UNF_AVPLAY_SET_TPLAY_PARA_CMD;
		stVdecCmdPara.pPara = &stTplayOpts;
		ret = MT_UNF_AVPLAY_Invoke(hAvplay, MT_UNF_AVPLAY_INVOKE_VCODEC, (void *)&stVdecCmdPara);
		if (MT_SUCCESS != ret)
		{
			printf("Resume Avplay trick mode to normal fail.\n");
			return MT_FAILURE;
		}
	}

	return MT_SUCCESS;
}

mt_s32 DALTS_StopLivePlay(mt_handle hAvplay)
{
	MT_UNF_AVPLAY_STOP_OPT_S option;

	option.enMode = MT_UNF_AVPLAY_STOP_MODE_STILL;
	option.u32TimeoutMs = 0;

	printf("stop live play ...\n");

	/*stop playing audio and video*//*CNcomment:停止音视频设备*/
	return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}
static pthread_t signal_monitor;
static pthread_t ir_monitor;

#define DALTS_TEST_VERSION ""

int opc_fd = -1;


static void MT_OPCDeinit(int opc_sock)
{
	if(opc_sock >= 0)
		close(opc_sock);
}

static int MT_OPCInit(void)
{
	struct sockaddr_nl    local    = {0,};
	int                   opc_sock = -1;

	printf("%s: in\n", __func__);

	opc_sock = socket(PF_NETLINK, SOCK_RAW, OPC_NETLINK_ID);
	if (opc_sock < 0) {
		printf("%s: Failed to open netlink socket, errno:%s\n", __func__, strerror(errno));
		if(opc_sock >= 0)
			close(opc_sock);
		return -1;
	}

	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = 1;
	if (bind(opc_sock, (struct sockaddr *) &local, sizeof(local)) < 0) {
		printf("%s: Failed to bind netlink socket, errno:%s\n", __func__, strerror(errno));
		if(opc_sock >= 0)
			close(opc_sock);
		return -1;
	}

	return opc_sock;
}


uint64_t MT_GetOPCStatus(void)
{
	typedef struct
	{
		struct nlmsghdr hdr;
		uint64_t status;
	}MT_OPC_STATUS_S;

	RET_CODE               ret          = 0;
	char                   buf[OPC_MAX_RECV_SIZE];
	struct nlmsghdr        *nlh         = (struct nlmsghdr *)buf;
	struct sockaddr_nl     dest_addr    = {0,};
	MT_OPC_STATUS_S        opc_data     = {0,};
	int                    rxlen        = 0;
	int                    opc_sock     = opc_fd;

	nlh->nlmsg_len = NLMSG_SPACE(OPC_MAX_RECV_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;
	strcpy(NLMSG_DATA(nlh), "GetStatus");
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	sendto(opc_sock, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

	memset(&opc_data, 0, sizeof(MT_OPC_STATUS_S));
       rxlen = sizeof(struct sockaddr_nl);

	ret = recvfrom(opc_sock, &opc_data, sizeof(MT_OPC_STATUS_S), 0, (struct sockaddr*)&dest_addr, &rxlen);
	if(ret <= 0){
		printf("%s: Failed to recv opc status, errno:%s\n", __func__, strerror(errno));
	}

	printf("===================> size=%ld, get opc status: 0x%lx \n", sizeof(uint64_t), opc_data.status);
	return opc_data.status;
}


void MT_NagraOPCDeinit(void)
{
	MT_OPCDeinit(opc_fd);

}

int main(int argc, char *argv[])
{
	mt_s32	s32Ret;
	MT_HANDLE	hWin;
	MT_HANDLE	hSoundTrack;
	//TSecNuid nuid;
	//TCsd4BytesVector csd_nuid;
	//TCsdInitParameters pxInitParameters;
    ir_wavefilter_config_s wavefiler = {0};

	MTGO_LAYER_INFO_S stLayerInfo = { 0 };
	MT_RECT rc = {100, 50, 600, 30};
	mt_u32 transvalue = 60;
	
	//csdInitialize(&pxInitParameters);
	//csdGetStbCaSn(csd_nuid);
	//printf("CA SN: 0x%x 0x%x 0x%x 0x%x\n", csd_nuid[0], csd_nuid[1], csd_nuid[2], csd_nuid[3]);
	//printf("DALTS Test Version:%s\n", DALTS_TEST_VERSION);
	s32Ret = dev_init();
	if (MT_SUCCESS != s32Ret) {
		printf("failed to dev_init\n");
		return 0;
	}

	mt_cert_init();
	mt_unf_fe_set_lnb_power(TUNER_ID, MT_UNF_FE_LNB_POWER_ON);
	s32Ret = mt_fe_connect_dvbs(TUNER_ID,4150,27500,0, 0, 2);
	if (MT_SUCCESS != s32Ret) {
		printf("failed to MTADP_Tuner_Connect\n");
		//return 0;
	}
	
	s32Ret = MT_UNF_IR_Init();
	if (s32Ret) 
	{
		printf("Fail to open ir dev! s32Ret = %d\n", s32Ret);
		return s32Ret;
	}


	MT_UNF_IR_EnableRepKey(MT_TRUE);
	MT_UNF_IR_SetRepKeyTimeoutAttr(400);
	MT_UNF_IR_SetFetchMode(0);
	MT_UNF_IR_Enable (MT_TRUE, RC_PROTO_NEC_);
	wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NEC_;
	wavefiler.irda_wfilt_channel = 1;
	wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
	wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x7F800AF5;   // 16 bit usercode | 8 bit keycode | 8 bit reversed keycode
	MT_UNF_IR_SetWaveFilter(&wavefiler);


	s32Ret = MT_UNF_DMX_Init();
	if (s32Ret != MT_SUCCESS) {
		printf("call MT_UNF_DMX_Init failed.\n");
		return s32Ret;
	}

	s32Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID_LIVE, MT_UNF_DMX_PORT_TSI_0);
	if (s32Ret != MT_SUCCESS){
		printf("call MT_UNF_DMX_AttachTSPort failed.\n");
		MT_UNF_DMX_DeInit();
		return s32Ret;
	}

	s32Ret = MTADP_Snd_Init();
	if (MT_SUCCESS != s32Ret){
		printf("call MTADP_Snd_Init failed.\n");
		return s32Ret;
	}

	s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_1080i_50);
	if (MT_SUCCESS != s32Ret){
		printf("call MTADP_Disp_DeInit failed.\n");
		return s32Ret;
	}

	s32Ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
	s32Ret |= MTADP_VO_CreatWin(MT_NULL,&hWin);
	if (MT_SUCCESS != s32Ret){
		printf("call MTADP_VO_Init failed.\n");
		MTADP_VO_DeInit();
		return s32Ret;
	}

	s32Ret = mt_hdmi_init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_1080i_50);
	if (MT_SUCCESS != s32Ret){
		printf("call MTADP_HDMI_Init failed.\n");
		return s32Ret;
	}

	opc_fd = MT_OPCInit();


	s32Ret = DALTS_AvplayInit(hWin, &hAvplay, &hSoundTrack);
	if (s32Ret != MT_SUCCESS){
		printf("call VoInit failed.\n");
		return s32Ret;
	}

	MT_DrawInit();
	u8 asc[128]={0};
	memset(asc, 0, sizeof(asc));
	snprintf(asc, 128, "Press Yellow Key to Search Program");
	printf("%s\n",asc);
	MT_DrawText(asc,100,100,1000,60);

#if 1
	MT_HANDLE mt_hflash;
	unsigned char *buf = NULL;
	int len = 0;
	FILE * fp;
	int ret;

	system("mkdir -p /tmp/datafs");
	system("mkdir -p /tmp/nagra_perso_data");
	system("mount -t ubifs ubi0_0 /tmp/datafs");
	system("mkdir -p /tmp/datafs/nagra_vfs");
	system("mkdir -p /tmp/datafs/nagra_trusted_storage");

	buf = malloc(0x20000);
	mt_unf_flash_init();
	mt_unf_flash_open("/dev/mtd0", &mt_hflash);

	//read and re-write CSCD
	mt_unf_flash_read(mt_hflash, 0x80800, buf, 0x1000);
	if(buf[0] == 0x00 && buf[1] == 0x01 && buf[2] == 0x02 && buf[3] == 0x03) { //just for talts
		;
	} else {
		mt_unf_flash_read(mt_hflash, 0x20800, buf, 0x1000);
	}
	//len = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	fp = fopen ("/tmp/nagra_perso_data/csc.dat", "wb");
	ret = fwrite(buf, 0x1000, 1, fp);

	if (ret != 0) {
		printf("csc.dat is written.\n");
	}
	fclose(fp);

	//read and re-write PK
	mt_unf_flash_read(mt_hflash, 0x80000, buf, 0x1000);
	len = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
	fp = fopen ("/tmp/nagra_perso_data/0303.pk", "wb");
	ret = fwrite(buf + 4, len, 1, fp);
	if (ret != 0) {
		printf("0303.pk is written.\n");
	}
	fclose(fp);
	free(buf);

	s32Ret = pthread_create(&signal_monitor, NULL, signal_monitor_task, NULL);
	if(0 != s32Ret) {
		printf("failed to signal_monitor_task\n");
	}

	s32Ret = pthread_create(&ir_monitor, NULL, ir_monitor_task, NULL);
	if(0 != s32Ret) {
		printf("failed to signal_monitor_task\n");
	}
#endif	
	caInitialization(0,0,1);
	caSetPersoDataPath("/tmp/nagra_perso_data/");
	caSetStoragePath("/tmp/datafs/nagra_vfs/");
	caSetTrustedStoragePath("/tmp/datafs/nagra_trusted_storage");
	caStartUp();
	while(1) {
		sleep(1);
	}
	return 0;
}
