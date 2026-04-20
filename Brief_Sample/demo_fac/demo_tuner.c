/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mt_type.h"
#include "mt_unf_frontend.h"
#include "demo.h"
#include "ui_manager.h"

static MT_BOOL g_bStatusTaskRunning = MT_FALSE;

static pthread_t g_statusThread = {0};

mt_s32 demo_dvbs_connect(void)
{
	mt_s32 ret;
	mt_unf_fe_attr_t tuner_attr = {0};
	mt_unf_fe_connect_para_t dvbs_connect_param;
	mt_unf_fe_status_t status = {0};

	printf("%s(line: %d), entry\n", __FUNCTION__, __LINE__);
	//open
	ret = mt_unf_fe_open(1);
	if (ret != MT_SUCCESS)
	{
		printf("mt_unf_fe_open(1) failed! return %d.\r\n", ret);
		return ret;
	}

	tuner_attr.tuner_type = MT_UNF_TUNER_TYPE_M88TS6011;
	tuner_attr.tuner_addr = 0x58;	//88
	tuner_attr.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
    tuner_attr.demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800;
    tuner_attr.demod_addr = 0x38;
    tuner_attr.demod_i2c_id = 0;
    tuner_attr.output_mode = MT_UNF_FE_OUTPUT_MODE_SERIAL;
    tuner_attr.tuner_i2c_id[0] = 0;
    tuner_attr.no_need_init = 0;

	ret = mt_unf_fe_set_attr(1, &tuner_attr);
    if (ret != MT_SUCCESS)
	{
        printf("Call mt_unf_fe_set_attr failed, ret = 0x%x.", ret);
        return ret;
    }

	dvbs_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
    dvbs_connect_param.connect_param.sat.freq = 3840 * 1000;
    dvbs_connect_param.connect_param.sat.sym_rate = 27500;
    dvbs_connect_param.connect_param.sat.port_type = 2;
    dvbs_connect_param.connect_param.sat.onoff_22k = 0;
    dvbs_connect_param.connect_param.sat.polarization = 0;

	ret = mt_unf_fe_connect(1, &dvbs_connect_param, 0);
    if (ret != MT_SUCCESS)
	{
        printf("Call mt_unf_fe_connect failed, ret = 0x%x.", ret);
    }

	return ret;
}


mt_s32 demo_dvbc_connect(void)
{
	mt_s32 ret;

	printf("%s(line: %d), entry\n", __FUNCTION__, __LINE__);
	//open
	ret = mt_unf_fe_open(0);
	if (ret != MT_SUCCESS)
	{
		printf("mt_unf_fe_open(0) failed! return %d.\r\n", ret);
		return ret;
	}

	//set tuner attr	CS8800/TC6800
	mt_unf_fe_attr_t attr = {0};

	ret = mt_unf_fe_get_default_attr(0, &attr);
	if (ret != MT_SUCCESS)
	{
		printf("mt_unf_fe_get_default_attr failed! return %d.\n", ret);
		return ret;
	}

	attr.sig_type = 1;
	attr.tuner_type = MT_UNF_TUNER_TYPE_M88TC6800;
	attr.tuner_addr = 0xc6;	//198
	attr.demod_dev_type = MT_UNF_DEMOD_DEV_TYPE_M88CS8800; //288
	attr.demod_addr = 0x38;
	attr.demod_i2c_id = 0;
	attr.output_mode = MT_UNF_FE_OUTPUT_MODE_SERIAL; //4
	attr.tuner_i2c_id[0] = 0;
    attr.no_need_init = 0;

	ret = mt_unf_fe_set_attr(0, &attr);
	if (ret != MT_SUCCESS)
	{
		printf("mt_unf_fe_set_attr failed! return %d.\n", ret);
		return ret;
	}

	//set tuner param	538/6875/64/8
	mt_unf_fe_connect_para_t tune_para = {0};
	mt_u32 timeout = 1000;
	
	tune_para.sig_type = MT_UNF_FE_SIG_TYPE_CAB;	//1
	tune_para.connect_param.cab.freq = 530000;
	tune_para.connect_param.cab.sym_rate = 6875000;
	tune_para.connect_param.cab.mod_type = MT_UNF_MOD_TYPE_QAM_64;	//0x102
	tune_para.connect_param.cab.band_width = 8;
	tune_para.connect_param.cab.b_reverse = 0;

	//start tune
	printf("mt_unf_fe_connect(%d, %d, %x) \r\n", tune_para.connect_param.cab.freq , tune_para.connect_param.cab.sym_rate, tune_para.connect_param.cab.mod_type);
	ret = mt_unf_fe_connect(0, &tune_para, timeout);
	if (ret != MT_SUCCESS)
	{
		printf("mt_unf_fe_connect(0) failed! return %d.\r\n", ret);
		return ret;
	}

	return ret;
}

static void* demo_tune_thread(void* arg)
{
	mt_s32 ret;
	mt_unf_fe_status_t status = {0};
	mt_unf_fe_connect_para_t dvbs_connect_param;

	ret = demo_dvbc_connect();
	if (ret != MT_SUCCESS)
	{
		printf("%s(line: %d), dvbc connect failed.\n", __FUNCTION__, __LINE__);
	}

	ret = demo_dvbs_connect();
	if (ret != MT_SUCCESS)
	{
		printf("%s(line: %d), dvbs connect failed.\n", __FUNCTION__, __LINE__);
	}

	while(g_bStatusTaskRunning == MT_TRUE)
	{
		ret = mt_unf_fe_get_status(0, &status);
		if (ret == MT_SUCCESS && status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
		{
			mt_unf_fe_get_signal_quality(0, &status.param.channel_info.perf.snr);
			mt_unf_fe_get_signal_strength(0, &status.param.channel_info.perf.agc);
//			printf("Tuner[0]: Locked! agc %d, snr %d.\n", status.param.channel_info.perf.agc, status.param.channel_info.perf.snr);
			
		}
		else
		{
//			printf("Tuner[0]: Unlocked! agc %d, snr %d.\n", status.param.channel_info.perf.agc, status.param.channel_info.perf.snr);
		}
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        manage_update_sub_event(ROOT_ID_DVBC, 3, (mt_u32)&status);
		manage_update_sub_event(ROOT_ID_DVBC, 4, (mt_u32)&status);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        manage_update_sub_event(ROOT_ID_DVBC, 3, (ulong)&status);
		manage_update_sub_event(ROOT_ID_DVBC, 4, (ulong)&status);
#endif
		manage_update_ui(UPDATE_MODULE, ROOT_ID_DVBC);
		usleep(500000);
		
		ret = mt_unf_fe_get_status(1, &status);
		if (ret == MT_SUCCESS && status.lock_status == MT_UNF_FE_SIGNAL_LOCKED)
		{
			mt_unf_fe_get_signal_quality(1, &status.param.channel_info.perf.snr);
			mt_unf_fe_get_signal_strength(1, &status.param.channel_info.perf.agc);
//			printf("Tuner[1]: Locked! agc %d, snr %d.\n", status.param.channel_info.perf.agc, status.param.channel_info.perf.snr);
		}
		else
		{
//			printf("Tuner[1]: Unlocked! agc %d, snr %d.\n", status.param.channel_info.perf.agc, status.param.channel_info.perf.snr);
/*
			dvbs_connect_param.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
		    dvbs_connect_param.connect_param.sat.freq = 4150 * 1000;
		    dvbs_connect_param.connect_param.sat.sym_rate = 27500;
		    dvbs_connect_param.connect_param.sat.port_type = 2;
		    dvbs_connect_param.connect_param.sat.onoff_22k = 0;
		    dvbs_connect_param.connect_param.sat.polarization = 0;

			mt_unf_fe_connect(1, &dvbs_connect_param, 0);
*/
		}
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        manage_update_sub_event(ROOT_ID_DVBS, 8, (mt_u32)&status);
		manage_update_sub_event(ROOT_ID_DVBS, 9, (mt_u32)&status);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        manage_update_sub_event(ROOT_ID_DVBS, 8, (ulong)&status);
		manage_update_sub_event(ROOT_ID_DVBS, 9, (ulong)&status);
#endif
		manage_update_ui(UPDATE_MODULE, ROOT_ID_DVBS);
		usleep(500000);		
		
	}
}


mt_s32 demo_tuner_start(void)
{
	
	mt_s32 ret;
	pthread_t demo_tune;

	printf("%s(line: %d), entry\n", __FUNCTION__, __LINE__);
	//init
	ret = mt_unf_fe_init();
	if (ret != MT_SUCCESS)
	{
		printf("mt_unf_fe_init failed! return %d.\r\n", ret);
		return ret;
	}
	g_bStatusTaskRunning = MT_TRUE;
	ret = pthread_create(&g_statusThread, NULL, demo_tune_thread, NULL);
	printf("%s(line: %d), return\n", __FUNCTION__, __LINE__);
	return ret;

}

mt_s32 demo_tuner_stop(void)
{
	mt_s32 ret;
	g_bStatusTaskRunning = MT_FALSE;
	
	ret = pthread_join(g_statusThread, NULL);
    if(MT_SUCCESS != ret)
    {
        printf("pthread_join failed.\n");
    }
	printf("%s(line: %d), return\n", __FUNCTION__, __LINE__);
}

