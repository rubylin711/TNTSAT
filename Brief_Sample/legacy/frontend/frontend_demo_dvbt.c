/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "mt_common.h"
#include "mt_unf_frontend.h"

#define MT_FE_ERR(format, arg...)  printf("[ERR]: %s,%d: " format, __FUNCTION__, __LINE__, ##arg)
#define MT_FE_INFO(format, arg...) printf(format, ##arg)
#define MT_FE_WARN(format, arg...) printf("[WARNING]: %s,%d: " format, __FUNCTION__, __LINE__, ##arg)

#if 1
#define CONFIG_FE_ATTR_DVBT(pAttr)	\
			pAttr->sig_type 		= MT_UNF_FE_SIG_TYPE_DVB_T; 		\
			pAttr->tuner_type 		= MT_UNF_TUNER_TYPE_M88TC6800; 		\
			pAttr->tuner_addr 		= 0xc6; 							\
			pAttr->demod_dev_type 	= MT_UNF_DEMOD_DEV_TYPE_M88CT8K; 	\
			pAttr->demod_addr 		= 0x18; 							\
			pAttr->output_mode 		= MT_UNF_FE_OUTPUT_MODE_PARALLEL; 	\
			pAttr->demod_i2c_id 	= 0; 								\
			pAttr->tuner_i2c_id[0] 	= 0

#define CONFIG_FE_ATTR_DVBT2(pAttr)	\
			pAttr->sig_type 		= MT_UNF_FE_SIG_TYPE_DVB_T2; 		\
			pAttr->tuner_type 		= MT_UNF_TUNER_TYPE_M88TC6800; 		\
			pAttr->tuner_addr 		= 0xc6; 							\
			pAttr->demod_dev_type 	= MT_UNF_DEMOD_DEV_TYPE_M88CT8K; 	\
			pAttr->demod_addr 		= 0x18; 							\
			pAttr->output_mode 		= MT_UNF_FE_OUTPUT_MODE_PARALLEL; 	\
			pAttr->demod_i2c_id 	= 0; 								\
			pAttr->tuner_i2c_id[0] 	= 0
#else
#define CONFIG_FE_ATTR_DVBT(pAttr)	\
			pAttr->sig_type 		= MT_UNF_FE_SIG_TYPE_DVB_T; 		\
			pAttr->tuner_type 		= MT_UNF_TUNER_TYPE_MXL603; 		\
			pAttr->tuner_addr 		= 0xc0; 							\
			pAttr->demod_dev_type 	= MT_UNF_DEMOD_DEV_TYPE_M88DM6K; 	\
			pAttr->demod_addr 		= 0x1c; 							\
			pAttr->output_mode 		= MT_UNF_FE_OUTPUT_MODE_PARALLEL; 	\
			pAttr->demod_i2c_id 	= 0; 								\
			pAttr->tuner_i2c_id[0] 	= 0

#define CONFIG_FE_ATTR_DVBT2(pAttr)	\
			pAttr->sig_type 		= MT_UNF_FE_SIG_TYPE_DVB_T2; 		\
			pAttr->tuner_type 		= MT_UNF_TUNER_TYPE_MXL603; 		\
			pAttr->tuner_addr 		= 0xc0; 							\
			pAttr->demod_dev_type 	= MT_UNF_DEMOD_DEV_TYPE_M88DM6K; 	\
			pAttr->demod_addr 		= 0x1c; 							\
			pAttr->output_mode 		= MT_UNF_FE_OUTPUT_MODE_PARALLEL; 	\
			pAttr->demod_i2c_id 	= 0; 								\
			pAttr->tuner_i2c_id[0] 	= 0
#endif

//----------------------------------------------------------------------------//
static mt_s32 dev_init_dvbt(mt_u32 tuner_id, MT_BOOL is_t2)
{
    mt_s32 ret = 0;
    mt_unf_fe_attr_t fe_attr;

    /*sys init*/
    mt_sys_init();

    ret = mt_unf_fe_init();
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_init failed.\n");
		return ret;
    }

    /* open Tuner*/
    ret = mt_unf_fe_open(tuner_id);
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_open failed.\n");
		mt_unf_fe_deinit();
		return ret;
    }

    /* get default attribute */
    ret = mt_unf_fe_get_default_attr(tuner_id, &fe_attr);
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_get_default_attr failed.\n");
		mt_unf_fe_close(tuner_id);
		mt_unf_fe_deinit();
		return ret;
    }

	if (is_t2)
	{
		CONFIG_FE_ATTR_DVBT2((&fe_attr));
	}
	else
	{
		CONFIG_FE_ATTR_DVBT((&fe_attr));
	}

    ret = mt_unf_fe_set_attr(tuner_id, &fe_attr);
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_set_attr failed.\n");
		return ret;
    }

    return MT_SUCCESS;
}

static mt_s32 dev_deinit(mt_u32 tuner_id)
{
    mt_s32 ret = 0;

    ret = mt_unf_fe_close(tuner_id);
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_close failed.\n");
		return ret;
    }

    ret = mt_unf_fe_deinit();
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("call mt_unf_fe_deinit failed.\n");
		return ret;
    }

    mt_sys_deinit();

    return MT_SUCCESS;
}

static mt_s32 mt_fe_connect_test_dvbt(mt_u32 tuner_id, mt_u32 freq, mt_u32 qam_type, mt_u32 band_width, mt_u8 plp_id, MT_BOOL is_t2)
{
    mt_s32 ret = MT_FAILURE;
    mt_unf_fe_status_t stTunerStatus;
    mt_u32 u32Loop = 0;
    mt_u32 u32LoopTimes = 100;
	mt_unf_fe_connect_para_t s_stConnectPara;

    s_stConnectPara.connect_param.ter.freq = freq * 1000;
    s_stConnectPara.connect_param.ter.band_width = band_width * 1000;
    s_stConnectPara.connect_param.ter.mode_type = qam_type;

	if (is_t2)
	{
		s_stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
		//TODO...
		s_stConnectPara.connect_param.ter.port_type = MT_UNF_PORT_TYPE_DVBT2;	//MT_UNF_PORT_TYPE_DVBT_AUTO
		s_stConnectPara.connect_param.ter.plp_id = plp_id;
		//TODO...
		s_stConnectPara.connect_param.ter.channel_mode = MT_UNF_FE_TER_MODE_BASE;	//MT_UNF_FE_TER_MODE_LITE
	}
	else
	{
		s_stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
		s_stConnectPara.connect_param.ter.port_type = MT_UNF_PORT_TYPE_DVBT;
		s_stConnectPara.connect_param.ter.plp_id = plp_id;
		//TODO...
		s_stConnectPara.connect_param.ter.dvbt_prio = MT_UNF_FE_TS_PRIORITY_NONE;	//MT_UNF_FE_TS_PRIORITY_LP,MT_UNF_FE_TS_PRIORITY_HP
	}

    ret = mt_unf_fe_connect(tuner_id, &s_stConnectPara, 2000);
    //ret = mt_unf_fe_connect(tuner_id, &s_stConnectPara, 0);

	MT_FE_ERR("ret = %d, %s\n", ret, (ret == MT_SUCCESS) ? "success" : "failed");

    if (MT_SUCCESS == ret)
	{
		for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop ++)
		{
		    ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);

		    if ((ret == MT_SUCCESS) && MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
			{
				MT_FE_INFO("Tuner %u lock freq %u -- Success!\n", tuner_id, freq);
				MT_FE_INFO("SUCCESS end\n");
				return MT_SUCCESS;
		    }
			else
			{
				MT_USLEEP(60000);
		    }
		}
    }
	else
	{
		MT_FE_ERR("Tuner %u lock freq %u -- Failed!, ret = 0x%x\n", tuner_id, freq, ret);
    }

    MT_FE_ERR("FAIL end\n");

    return MT_FAILURE;
}

/*
 * help: frontend_demo_dvbt -T|-T2 tuner_id freq(MHz) qam band_width [plp_id]
 *           tuner_id: 0, 1
 *           qam: 16, 32, 64, 128, 256, 512
 *           band_width: 6, 7, 8
 *           plp_id: 0-255
 */
mt_s32 main(mt_s32 argc, char *argv[])
{
	const char *help = "help: frontend_demo_dvbt -T|-T2 tuner_id freq(MHz) qam band_width [plp_id]\n"
						"          tuner_id: 0, 1\n"
						"		   qam: 16, 32, 64, 128, 256, 512\n"
						"		   band_width: 6, 7, 8\n"
						"		   plp_id: 0-255";
    mt_s32 ret = MT_FAILURE;

	mt_u32 tuner_id = 0;
    mt_u32 freq = 0;
    mt_u32 qam_type = 0;
    mt_u32 qam_mode = 0;
    mt_u32 band_width = 8;
	mt_u8  plp_id = 0;
	MT_BOOL is_t2 = MT_FALSE;
	int n = 1;

	if (argc < 6)
	{
		MT_FE_WARN("%s\n",help);
		return ret;
	}

	if (strcmp(argv[n], "-T") == 0)
	{
		is_t2 = MT_FALSE;
	}
	else if (strcmp(argv[n], "-T2") == 0)
	{
		if (argc < 7)
		{
			MT_FE_WARN("%s\n",help);
			return ret;
		}
		is_t2 = MT_TRUE;
	}
	else
	{
		MT_FE_WARN("%s\n",help);
		return ret;
	}

	tuner_id = strtol(argv[++n], NULL, 0);
	if (tuner_id != 0 && tuner_id != 1)
	{
		MT_FE_WARN("%s\n",help);
		return ret;
	}

    /* init device */
    ret = dev_init_dvbt(tuner_id, is_t2);
    if (MT_SUCCESS != ret) {
		MT_FE_ERR("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, ret);
		return ret;
    }

    freq = strtol(argv[++n], NULL, 0);
    qam_mode = strtol(argv[++n], NULL, 0);
    band_width = strtol(argv[++n], NULL, 0);
	if (argc >= 7)
	    plp_id = strtol(argv[++n], NULL, 0);
	if (band_width != 6 && band_width != 7 && band_width != 8)
	{
		MT_FE_WARN("%s\n",help);
		goto DEV_DEINIT;
	}

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
	else
	{
		MT_FE_WARN("%s\n",help);
		goto DEV_DEINIT;
	}

    ret = mt_fe_connect_test_dvbt(tuner_id, freq, qam_type, band_width, plp_id, is_t2);

DEV_DEINIT:
    dev_deinit(tuner_id);

    return ret;
}

