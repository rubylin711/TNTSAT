/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_frontend.h"
#include "mt_adp_demux.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_frontend.h"
#include "mt_cmdline.h"
#include "mt_adp_pvr.h"

/***************************** Macro Definition ******************************/

#ifdef MT_SAMPLE_DVBS_DEBUG

#define MT_DVBS_PRINT   printf
#else

#define MT_DVBS_PRINT

#endif

#define SAMPLE_DVBS_FUNCTION_ENTER()    MT_DVBS_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DVBS_FUNCTION_EXIT()     MT_DVBS_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DVBS_FATAL_PRINT(fmt...)         MT_DVBS_PRINT(" [FATAL] " fmt)
#define SAMPLE_DVBS_ERR_PRINT(fmt...)           MT_DVBS_PRINT(" [ERROR] " fmt)
#define SAMPLE_DVBS_WARN_PRINT(fmt...)          MT_DVBS_PRINT(" [WARN] "  fmt)
#define SAMPLE_DVBS_INFO_PRINT(fmt...)          MT_DVBS_PRINT(" [INFO] "  fmt)
#define SAMPLE_DVBS_DBG_PRINT(fmt...)           MT_DVBS_PRINT(" [DEBUG] " fmt)


#define SAMPLE_DVBS_PRINT  printf


#define DMX_ID_0            0
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

//#define HD2502_ABS_SAMPPLE
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_s32 tuner_id;
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar; /**<Polarization mode>*/
    mt_u32 dvbs_type; /**<dvbs type>*/
	mt_u32 low_lo; /**< Low Local Oscillator Frequency, MHz */
    mt_u32 high_lo; /**< High Local Oscillator Frequency, MHz*/
} mt_input_Dvbs_para_t;
typedef struct
{
    MT_HANDLE          hAvPlay;
    MT_HANDLE          hSoundTrack;
    MT_HANDLE          hWin;
    PMT_COMPACT_TBL *pProgTbl;
    mt_input_Dvbs_para_t  sInputParam;
} MT_DVBS_RUN_INFO;


/********************** Global Variable declaration **************************/

static MT_BOOL    g_bTaskQuit = MT_TRUE;
static MT_DVBS_RUN_INFO    g_stDvbsRunInfo = {MT_INVALID_HANDLE};
#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif

#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbsMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif
static mt_s32 MT_DvbsFrontendLnbConfig(mt_input_Dvbs_para_t *pInutParam)
{
	MT_S32                 ret = 0;
	mt_unf_fe_lnb_config_t lnb_config = { 0 };
    if(pInutParam->onoff_22k)
    {
        ret = mt_unf_fe_set_lnb_power(pInutParam->tuner_id, MT_UNF_FE_LNB_POWER_ON);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("mt_unf_fe_set_lnb_power err!\n");
            return ret;
        }
        memset(&lnb_config,0,sizeof(mt_unf_fe_lnb_config_t));
		if((pInutParam->high_lo != pInutParam->low_lo)&&(pInutParam->low_lo != 0)&&(pInutParam->high_lo != 0))
		{
			lnb_config.lnb_type = MT_UNF_FE_LNB_DUAL_FREQUENCY;
			if((pInutParam->high_lo > 7500)&&(pInutParam->low_lo > 7500))
			{
				lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_KU;
			}
			else if((pInutParam->high_lo < 7500)&&(pInutParam->low_lo < 7500))
			{
				lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
			}
			else
			{
				lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_BUTT;
				ret = MT_FAILURE;
				SAMPLE_DVBS_ERR_PRINT("Invalid LO freq!low_lo:%d,high_lo:%d\n",pInutParam->low_lo,pInutParam->high_lo);
            	return ret;
			}
        	lnb_config.low_lo = pInutParam->low_lo;
	       	lnb_config.high_lo = pInutParam->high_lo;
		}
		else
		{
			lnb_config.lnb_type = MT_UNF_FE_LNB_SINGLE_FREQUENCY;
			if(pInutParam->low_lo != 0)
			{
				if(pInutParam->low_lo > 7500)
					lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_KU;
				else
					lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
				lnb_config.low_lo = pInutParam->low_lo;
				lnb_config.high_lo = pInutParam->low_lo;
			}
			else
			{
				lnb_config.lnb_band = MT_UNF_FE_LNB_BAND_C;
				lnb_config.low_lo = 5150;
				lnb_config.high_lo = 5150;
			}
		}
		SAMPLE_DVBS_INFO_PRINT("Lnb Config:Low_lo:%d MHz,High_lo:%d MHz in %s \n",lnb_config.low_lo,lnb_config.high_lo,(lnb_config.lnb_band == MT_UNF_FE_LNB_BAND_C) ? "C BAND" : "KU BAND");
        ret = mt_unf_fe_set_lnb_config(pInutParam->tuner_id, &lnb_config);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("mt_unf_fe_set_lnb_config err!\n");
            return ret;
        }
    }
	return MT_SUCCESS;
}
static mt_s32 MT_DvbsFrontendSetParameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel)
{
    MT_S32           ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };
    mt_sys_version_s stSysChipInfo = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("mt_sys_get_version err!\n");
        return ret;
    }

    SAMPLE_DVBS_INFO_PRINT("MTCommand_Tuner_Operation:   chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id,&mtTunerAttr);
    if(SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("mt_unf_fe_get_default_attr err!\n");
        return ret;
    }


    mtTunerAttr.sig_type = sig_type;

    if (stSysChipInfo.enChipVersion < MT_CHIP_SYMPHONY2_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY2_A3)
    {
        if ((tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC3800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_M88TC6800)
          ||(tuner_dev_type == MT_UNF_TUNER_TYPE_MXL_608))
        {
            mtTunerAttr.fe_config.tun2_type = tuner_dev_type;
            mtTunerAttr.fe_config.tun2_addr = tuner_addr;
        }
        else
        {
            mtTunerAttr.tuner_type = tuner_dev_type;
            mtTunerAttr.tuner_addr = tuner_addr;
        }
    }
    else if (stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY4_A1)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else if(stSysChipInfo.enChipVersion <= MT_CHIP_SYMPHONY6_A0)
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }
#endif
    else
    {
        mtTunerAttr.tuner_type = tuner_dev_type;
        mtTunerAttr.tuner_addr = tuner_addr;
    }

    mtTunerAttr.demod_dev_type = demod_dev_type;
    mtTunerAttr.demod_addr = demod_addr;
    mtTunerAttr.demod_i2c_id = I2c_channel;
    mtTunerAttr.output_mode = out_put_mode;
    mtTunerAttr.tuner_i2c_id[0] = 0;
    mtTunerAttr.no_need_init = 0;

    SAMPLE_DVBS_INFO_PRINT("tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );
    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("mt_unf_fe_set_attr failed.ret is0x%x\n",ret);
        return ret;
    }

    SAMPLE_DVBS_INFO_PRINT("output_mode = 0x%x\n", out_put_mode);

    return MT_SUCCESS;
}
static mt_s32 MT_DvbsFrontendConnect(mt_input_Dvbs_para_t *pInutParam)
{
    MT_S32                 ret = 0;
    mt_sys_version_s       stSysChipInfo = { 0 };
    
    MT_U32                   u32Loop = 0;
    MT_U32                   u32Freq = 0;
    MT_U32                   u32SymbolRate = 0;
    mt_unf_fe_status_t       stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t stConnectPara = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));

   /** Get chip information */
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("mt_sys_get_version failed. ret = 0x%x\n",  ret);
        return ret;
    }

    SAMPLE_DVBS_INFO_PRINT("chipVersion = %#x. \n", stSysChipInfo.enChipVersion);

    if(MT_CHIP_SYMPHONY4_A1 >= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY4_A0 <= stSysChipInfo.enChipVersion)
    {
        if(pInutParam->tuner_id == 0)
        {
#ifdef MT_SYM4_DSS
            ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011, 0x58, MT_UNF_DEMOD_DEV_TYPE_M88DS6113, 0xD2, 1, 1);
#else
            ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 34, 88, 288, 24, 1, 0);
#endif

        }
        else if(pInutParam->tuner_id == 1)
        {
            ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 80, 90, 336, 210, 4, 0);
        }
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        ret = MT_DvbsFrontendLnbConfig(pInutParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbsFrontendLnbConfig err!\n");
            return ret;
        }
    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else  if(MT_CHIP_SYMPHONY6_MAX >= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY6_A0 <= stSysChipInfo.enChipVersion)
    {
        /* tuner_id, Port of the tuner
           sig_type, Type of received signal
           tuner_dev_type, tuner Device type
           tuner_addr, The address of tuner
           demod_dev_type, Type of the demod device
           demod_addr, The address of demod
           out_put_mode, Output mode
           I2c_channel, i2c Channel mode*/
        if(pInutParam->tuner_id == 0)
        {
#ifdef HD2502_ABS_SAMPPLE
			ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 34, 88, 387, 24, 4, 0);
#else
           	ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 34, 88, 288, 24, 4, 0);
#endif
        }
        else if(pInutParam->tuner_id == 1)
        {
            ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 80, 90, 336, 210, 4, 1);
        }
        else
        {
            ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 34, 88, 288, 24, 4, 0);
        }


        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        ret = MT_DvbsFrontendLnbConfig(pInutParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbsFrontendLnbConfig err!\n");
            return ret;
        }

    }
#endif
    else  if(MT_CHIP_SYMPHONY2_A0 > stSysChipInfo.enChipVersion)
    {
        /* tuner_id, Port of the tuner
           sig_type, Type of received signal
           tuner_dev_type, tuner Device type
           tuner_addr, The address of tuner
           demod_dev_type, Type of the demod device
           demod_addr, The address of demod
           out_put_mode, Output mode
           I2c_channel, i2c Channel mode*/
        ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id, 2048, 34, 88, 261, 208, 1, 0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        ret = MT_DvbsFrontendLnbConfig(pInutParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbsFrontendLnbConfig err!\n");
            return ret;
        }

    }
    else
    {
        /* tuner_id, Port of the tuner
           sig_type, Type of received signal
           tuner_dev_type, tuner Device type
           tuner_addr, The address of tuner
           demod_dev_type, Type of the demod device
           demod_addr, The address of demod
           out_put_mode, Output mode
           I2c_channel, i2c Channel mode*/
        ret = MT_DvbsFrontendSetParameter(pInutParam->tuner_id,2048,34,88,280,24,1,0);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        ret = MT_DvbsFrontendLnbConfig(pInutParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbsFrontendLnbConfig err!\n");
            return ret;
        }
    }
    if (MT_UNF_PORT_TYPE_DVBS == pInutParam->dvbs_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        SAMPLE_DVBS_INFO_PRINT("dvbs\n");
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == pInutParam->dvbs_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
        SAMPLE_DVBS_INFO_PRINT("dvbs2\n");
    }
    else if(MT_UNF_PORT_TYPE_DVBS_AUTO == pInutParam->dvbs_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
        SAMPLE_DVBS_INFO_PRINT("dvbs_auto\n");
    }

    stConnectPara.connect_param.sat.freq = pInutParam->freq * 1000;
    stConnectPara.connect_param.sat.sym_rate = pInutParam->sym_rate;
    stConnectPara.connect_param.sat.port_type = pInutParam->dvbs_type;
    stConnectPara.connect_param.sat.onoff_22k = pInutParam->onoff_22k;
    stConnectPara.connect_param.sat.polarization = pInutParam->polar;

    ret = mt_unf_fe_connect(pInutParam->tuner_id, &stConnectPara, 2000);
    /* Get the actual lock frequency and rate*/
    u32Freq = stConnectPara.connect_param.sat.freq;
    u32SymbolRate = stConnectPara.connect_param.sat.sym_rate;

    if (MT_SUCCESS == ret)
    {
        for (u32Loop = 0; u32Loop < 1000; u32Loop++)
        {
            ret = mt_unf_fe_get_status(pInutParam->tuner_id, &stTunerStatus);
            if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
            {
                SAMPLE_DVBS_INFO_PRINT("Tuner Lock freq %d symb %d polar%d Success!\n", u32Freq, u32SymbolRate, pInutParam->polar);
                return MT_SUCCESS;
            }
            else
            {
                usleep(10000);
            }
        }
    }
    else
    {
        SAMPLE_DVBS_ERR_PRINT("Tuner Lock freq %d symb %d polar%d mt_unf_fe_connect Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, pInutParam->polar, ret);
    }

    if (u32Loop == 1000)
    {
        SAMPLE_DVBS_ERR_PRINT("Tuner Lock freq %d symb %d  polar%d Fail!\n", u32Freq, u32SymbolRate, pInutParam->polar);
    }

    return MT_FAILURE;
}

static mt_s32 MT_DvbsCheckParam(mt_input_Dvbs_para_t *p_dvbs_in)
{
	mt_s32 ret = MT_SUCCESS;
	switch(p_dvbs_in->low_lo)
	{
		case 5750:
			if((p_dvbs_in->freq) > 4800 || (p_dvbs_in->freq) < 3600)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is %d MHz,freq must be more than 3600 MHz and less than 4800 MHz.\n",p_dvbs_in->low_lo);
        		ret = MT_FAILURE;
    		}
			break;
		case 9750:
			if((p_dvbs_in->freq) > 11900 || (p_dvbs_in->freq) < 10700)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is %d MHz,freq must be more than 10700 MHz and less than 11900 MHz.\n",p_dvbs_in->low_lo);
        		ret = MT_FAILURE;
    		}
			break;
		case 10600:
			if((p_dvbs_in->freq) > 12750 || (p_dvbs_in->freq) < 11550)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is %d MHz,freq must be more than 11550 MHz and less than 12750 MHz.\n",p_dvbs_in->low_lo);
        		ret = MT_FAILURE;
    		}
			break;
		case 10750:
			if((p_dvbs_in->freq) > 12900 || (p_dvbs_in->freq) < 11700)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is %d MHz,freq must be more than 11550 MHz and less than 12750 MHz.\n",p_dvbs_in->low_lo);
        		ret = MT_FAILURE;
    		}
			break;			
		case 11250:
			if((p_dvbs_in->freq) > 13400 || (p_dvbs_in->freq) < 12200)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is %d MHz,freq must be more than 12200 MHz and less than 13400 MHz.\n",p_dvbs_in->low_lo);
        		ret = MT_FAILURE;
    		}
			break;
		case 11300:
			if((p_dvbs_in->freq) > 13450 || (p_dvbs_in->freq) < 12250)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is %d MHz,freq must be more than 13450 MHz and less than 12250 MHz.\n",p_dvbs_in->low_lo);
        		ret = MT_FAILURE;
    		}
			break;
		case 5150:
		default:
    		if((p_dvbs_in->freq) > 4200 || (p_dvbs_in->freq) < 3000)
    		{
        		SAMPLE_DVBS_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        		SAMPLE_DVBS_ERR_PRINT("Local Oscillator is 5150 MHz,freq must be more than 3000 MHz and less than 4200 MHz.\n");
        		ret = MT_FAILURE;
    		}
			break;
	}
    return ret;
}


/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsDmxInit(MT_VOID)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("mt_sys_get_version failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    if(play_resource.demux_use != MT_TRUE)
    {
        /** Initializes the demux module */
        ret = MT_UNF_DMX_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
            return ret;
        }
    }



    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        if(g_stDvbsRunInfo.sInputParam.tuner_id == 0)
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_DVBS_INFO_PRINT("Connect port 0!\n");
            play_resource.sig_type = 1;
        }
        else if(g_stDvbsRunInfo.sInputParam.tuner_id == 1)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /** Bind Demux to tuner port 3 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_3);
            SAMPLE_DVBS_INFO_PRINT("Connect port 3!\n");
#else
            /** Bind Demux to tuner port 1 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            SAMPLE_DVBS_INFO_PRINT("Connect port 1!\n");
#endif
           play_resource.sig_type = 3;
        }
        else
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_DVBS_INFO_PRINT("Connect port 0!\n");
            play_resource.sig_type = 1;
        }

    }
    else
    {
        /** Bind Demux to tuner port 1 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);

        SAMPLE_DVBS_INFO_PRINT("Connect port 1!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_DvbsDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}

/*!
@brief audio and video player initialization.
@param[out] phAvplay            Handle to AV player
@param[out] hWin                The input window handler
@param[out] phSoundTrack        Handle to sound track
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsAvplayInit(mt_handle *phAvplay, mt_handle *phWin, mt_handle *phSoundTrack)
{
    MT_S32                   ret = MT_FAILURE;
    mt_handle                hAvplay = 0;
    mt_handle                hWin = 0;
    mt_handle                hSoundTrack = 0;
    MT_UNF_AVPLAY_ATTR_S     AvplayAttr = { 0 };
    MT_UNF_AUDIOTRACK_ATTR_S stTrackAttr = { 0 };

    SAMPLE_DVBS_FUNCTION_ENTER();

    if(NULL == phAvplay)
    {
        SAMPLE_DVBS_ERR_PRINT("phAvplay is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phWin)
    {
        SAMPLE_DVBS_ERR_PRINT("phWin is null.\n");
        return MT_FAILURE;

    }
    if(NULL == phSoundTrack)
    {
        SAMPLE_DVBS_ERR_PRINT("phSoundTrack is null.\n");
        return MT_FAILURE;

    }

    /** Audio decoder */
    ret = MTADP_AVPlay_RegADecLib();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MTADP_AVPlay_RegADecLib failed.\n");
        return ret;
    }

    /** AV player initialization */
    ret = MT_UNF_AVPLAY_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_AVPLAY_Init failed.\n");
        return ret;

    }

    /** Get the default parameters of AV player based on the data input stream interface type and put the parameters in Avplayattr */
    ret = MT_UNF_AVPLAY_GetDefaultConfig(&AvplayAttr, MT_UNF_AVPLAY_STREAM_TYPE_TS);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_AVPLAY_GetDefaultConfig failed.\n");
        goto ERR1;
    }

    /** Defines the playing attributes of the AV player */
    AvplayAttr.u32DemuxId = DMX_ID_0;
    AvplayAttr.stStreamAttr.u32VidBufSize = AVPLAYER_VIDEO_BUFFER_SIZE;
    AvplayAttr.stStreamAttr.u32AudBufSize = AVPLAYER_AUDIO_BUFFER_SIZE;

    /** Creates an AVPLAY */
    ret = MT_UNF_AVPLAY_Create(&AvplayAttr, &hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_AVPLAY_Create failed.\n");
        goto ERR1;
    }

    /** Open the video channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR2 ;
    }

    /** Open the audio channel of the AV player */
    ret = MT_UNF_AVPLAY_ChnOpen(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD, MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_AVPLAY_ChnOpen failed.\n");
        goto ERR3;
    }

    /** Obtains the default configured parameters of an AO Track */
    ret = MT_UNF_SND_GetDefaultTrackAttr(MT_UNF_SND_TRACK_TYPE_MASTER, &stTrackAttr);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_SND_GetDefaultTrackAttr failed.\n");
        goto ERR4;
    }

    /** Create a track based on the audio device model */
    ret = MT_UNF_SND_CreateTrack(MT_UNF_SND_0, &stTrackAttr, &hSoundTrack);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_SND_CreateTrack failed.\n");
        goto ERR4;
    }

    /** Attaches the SND module to an AV player */
    ret = MT_UNF_SND_Attach(hSoundTrack, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_SND_Attach failed.\n");
        goto ERR5;
    }

    /** Create a window */
    ret = MTADP_VO_CreatWin(MT_NULL, &hWin);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("MTADP_VO_CreatWin error\n");
        goto ERR6;
    }
    /** Bind AV player to the window */
    ret = MT_UNF_VO_AttachWindow(hWin, hAvplay);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_VO_AttachWindow failed.\n");
        goto ERR7;
    }

    /** Enable windows */
    ret = MT_UNF_VO_SetWindowEnable(hWin, MT_TRUE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("call MT_UNF_VO_SetWindowEnable failed.\n");
        goto ERR8;
    }

    *phAvplay = hAvplay;
    *phWin = hWin;
    *phSoundTrack = hSoundTrack;

    SAMPLE_DVBS_FUNCTION_EXIT();

    return MT_SUCCESS;

ERR8:
    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

ERR7:
    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

ERR6:
    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

ERR5:
    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

ERR4:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

ERR3:
    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

ERR2:
    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

ERR1:
    (MT_VOID)MT_UNF_AVPLAY_DeInit();

    return MT_FAILURE;
}



/*
 @brief Audio and video playback Deinit
 @param[in] hWin, A pointer to the Window handle passed in
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] phSoundTrack,A pointer to the SoundTrack handle passed in
 @return void

*/
static void  MT_DvbsAvplayDeInit(mt_handle hAvplay, mt_handle hWin, mt_handle hSoundTrack)
{

    (MT_VOID)MT_UNF_VO_SetWindowEnable(hWin, MT_FALSE);

    (MT_VOID)MT_UNF_VO_DetachWindow(hWin, hAvplay);

    (MT_VOID)MT_UNF_VO_DestroyWindow(hWin);

    (MT_VOID)MT_UNF_SND_Detach(hSoundTrack, hAvplay);

    (MT_VOID)MT_UNF_SND_DestroyTrack(hSoundTrack);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID);

    (MT_VOID)MT_UNF_AVPLAY_ChnClose(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_AUD);

    (MT_VOID)MT_UNF_AVPLAY_Destroy(hAvplay);

    (MT_VOID)MT_UNF_AVPLAY_DeInit();

}

/*
 @brief Audio and video Type of decoding
 @param[in] phAvplay,A pointer to the Avplay handle passed in
 @param[in] pProgInfo,Data type corresponding to an attribute ID CNcomment
 @return ::MT_SUCCESS
 @return ::MT_FAILURE
*/
static MT_S32 MT_DvbsSetAvplayPidAndCodecType(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_S32                           ret = MT_FAILURE;
    MT_U32                           u32AudType = 0;
    MT_U32                           VidPid = 0;
    MT_U32                           AudPid = 0;
    MT_U32                           PcrPid = 0;
    MT_UNF_VCODEC_ATTR_S             VdecAttr = { 0 };
    MT_UNF_ACODEC_ATTR_S             AdecAttr = { 0 };
    MT_UNF_VCODEC_TYPE_E             enVidType = { 0 };
    MT_UNF_AVPLAY_DMX_AVSYNC_ATTR_S  DmxAvsync = { 0 };
    MT_UNF_VCODEC_UNBLANK_E          unblank;


    SAMPLE_DVBS_FUNCTION_ENTER();

    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVBS_ERR_PRINT("=====hAvplay is INVALID_HANDLE ======\n");
        return ret;
    }
    if(NULL == pProgInfo)
    {
        SAMPLE_DVBS_ERR_PRINT("=====pProgInfo == NULL=====\n");
        return ret;
    }


    if(pProgInfo->VElementNum > 0)
    {
        VidPid = pProgInfo->VElementPid;
        enVidType = pProgInfo->VideoType;
    }
    else
    {
        VidPid = INVALID_TSPID;
        enVidType = MT_UNF_VCODEC_TYPE_BUTT;
    }

    if(pProgInfo->AElementNum > 0)
    {
        AudPid  = pProgInfo->AElementPid;
        u32AudType = pProgInfo->AudioType;
    }
    else
    {
        AudPid = INVALID_TSPID;
        u32AudType = 0xffffffff;
    }

    if (u32AudType == HA_AUDIO_ID_PCM)
    {
        mt_s32 i = 0;
        pcm_info_t pcm = { 0 };
        for (i = 0; i < pProgInfo->AElementNum; i++)
        {
            if (pProgInfo->Audioinfo[i].u16AudioPid == AudPid) {
                break;
            }
        }

        pcm = pProgInfo->Audioinfo[i].pcm;
        MTADP_Set_AudPcmInfo(pcm);
    }

    PcrPid = pProgInfo->PcrPid;

    SAMPLE_DVBS_INFO_PRINT("VidPid=%x, Vidtype=0x%x, AudPid=%x, AudType=0x%x \n", VidPid, enVidType, AudPid, u32AudType);

    /** Get the audio properties of the AV player */

    if(INVALID_TSPID != PcrPid)
    {
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_PCR_PID, &PcrPid);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("call MT_UNF_AVPLAY_SetAttr  MT_UNF_AVPLAY_ATTR_ID_PCR_PID failed.\n");
            return MT_FAILURE;
        }
    }

    if(VidPid != INVALID_TSPID)
    {
        MTADP_Get_VcodeUnblank(&unblank);
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }

        /* The type of code stream supported by the decoder*/
        if (MT_UNF_VCODEC_TYPE_VC1 == enVidType)
        {
            VdecAttr.unExtAttr.stVC1Attr.bAdvancedProfile = 1;
            VdecAttr.unExtAttr.stVC1Attr.u32CodecVersion = 8;
        }

        if (MT_UNF_VCODEC_TYPE_VP6 == enVidType)
        {
            VdecAttr.unExtAttr.stVP6Attr.bReversed = 0;
        }

        VdecAttr.enType = enVidType;
        VdecAttr.enUnBlank = unblank;
        VdecAttr.enMode = MT_UNF_VCODEC_MODE_NORMAL;
        VdecAttr.u32ErrCover = 100;
        VdecAttr.s32CtrlOptions = 0;
        VdecAttr.u32Priority = 3;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VDEC, &VdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT(" MT_UNF_AVPLAY_SetAttr failed.\n");
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_VID_PID,&VidPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("Set video properties or video PID property failed.\n");
            return ret;
        }
    }


    if(AudPid != INVALID_TSPID)
    {
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_ADEC, &AdecAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT(" MT_UNF_AVPLAY_GetAttr failed.\n");
            return ret;
        }
        /* PCM decoding mode*/
        ret = MTADP_AVPlay_SetAdecAttr(hAvplay,u32AudType,HD_DEC_MODE_RAWPCM,1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_AVPlay_SetAdecAttr failed:%#x\n",ret);
            return ret;
        }

        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_AUD_PID,&AudPid);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("Setting the decoding mode or audio PID property failed:%#x\n",ret);
            return ret;
        }
    }

    /* insert pts*/
    if((VidPid != INVALID_TSPID) || (AudPid != INVALID_TSPID))
    {
        /* Defines the attribute of low delay*/

        DmxAvsync.VdecType = enVidType;
        DmxAvsync.AdecType = u32AudType;
        DmxAvsync.AvsyncFlage = 1;   // 1--insert pts 0--do not insert pts
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_DMX_AVSYNC, (mt_void *)&DmxAvsync);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_UNF_AVPLAY_SetAttr failed:%#x\n", ret);
            return ret;
        }
    }

    SAMPLE_DVBS_FUNCTION_EXIT();


    return MT_SUCCESS;
}


/*!
@brief start the AV playback into the start state.
@param[in]  phAvplay            handle to AV player
@param[in]  pProgInfo           The data structure of the PMT
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsStarToPlay(mt_handle hAvplay, const PMT_COMPACT_PROG *pProgInfo)
{
    MT_U32                        ret = MT_FAILURE;
    MT_UNF_AVPLAY_MEDIA_CHAN_E    enMediaType = 0;
    MT_UNF_AVPLAY_FRMRATE_PARAM_S stFrmRateAttr = { 0 };
    MT_UNF_SYNC_ATTR_S            stSyncAttr = { 0 };

    SAMPLE_DVBS_FUNCTION_ENTER();

    if(NULL == pProgInfo)
    {
        SAMPLE_DVBS_ERR_PRINT("p_ProgInfo is NULL!\n");
        return MT_FAILURE;
    }
    if(MT_INVALID_HANDLE == hAvplay)
    {
        SAMPLE_DVBS_ERR_PRINT("hAvplay is not exist\n");
        return ret;
    }

    /** Set the PID of the AV player and set the encoder type */
    ret = MT_DvbsSetAvplayPidAndCodecType(hAvplay, pProgInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("MT_DvbsSetAvplayPidAndCodecType fail! \n");
        return ret;
    }

    if(pProgInfo->AElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_AUD;
    }
    else
    {
        SAMPLE_DVBS_INFO_PRINT("has no audio info \n");
    }

    if(pProgInfo->VElementNum != 0)
    {
        enMediaType |=  MT_UNF_AVPLAY_MEDIA_CHAN_VID;
    }
    else
    {
        SAMPLE_DVBS_INFO_PRINT("has no vide0 info \n");
    }


    if((enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_AUD) && (enMediaType & MT_UNF_AVPLAY_MEDIA_CHAN_VID))
    {
        /** Set the frame rate parameter of AV player, enable vo frame rate detect */
        stFrmRateAttr.enFrmRateType = MT_UNF_AVPLAY_FRMRATE_TYPE_PTS;
        stFrmRateAttr.stSetFrmRate.u32fpsInteger = 0;
        stFrmRateAttr.stSetFrmRate.u32fpsDecimal = 0;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_FRMRATE_PARAM, &stFrmRateAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("Set frame to VO fail.\n");
            return ret;
        }

        /** Get synchronization properties of AV player */
        ret = MT_UNF_AVPLAY_GetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("Get avplay sync attr fail!\n");
            return ret;
        }

        /** Set synchronization properties of AV player */
        stSyncAttr.enSyncRef = MT_UNF_SYNC_REF_AUDIO;
        stSyncAttr.stSyncStartRegion.s32VidPlusTime = 60;
        stSyncAttr.stSyncStartRegion.s32VidNegativeTime = -20;
        stSyncAttr.stSyncStartRegion.bSmoothPlay = MT_TRUE;
        stSyncAttr.u32PreSyncTimeoutMs = 1000;
        stSyncAttr.bQuickOutput = MT_FALSE;
        ret = MT_UNF_AVPLAY_SetAttr(hAvplay, MT_UNF_AVPLAY_ATTR_ID_SYNC, &stSyncAttr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("Set avplay sync attr fail!\n");
            return ret;
        }
    }

    /*start to play audio and video*/
    ret = MT_UNF_AVPLAY_Start(hAvplay, enMediaType, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("MT_UNF_AVPLAY_Start fail!  ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_DVBS_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*!
@brief stop AV playback into the stop state.
@param[in]  phAvplay            handle to AV player
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_DvbsStopToPlay(mt_handle hAvplay, MT_UNF_AVPLAY_STOP_MODE_E enmode)
{
    MT_UNF_AVPLAY_STOP_OPT_S option = { 0 };

    option.enMode = enmode;
    option.u32TimeoutMs = 0;

    SAMPLE_DVBS_INFO_PRINT("stop live play ...\n");

    /*stop playing audio and video*/
    return MT_UNF_AVPLAY_Stop(hAvplay, MT_UNF_AVPLAY_MEDIA_CHAN_VID | MT_UNF_AVPLAY_MEDIA_CHAN_AUD, &option);
}

static MT_VOID MT_DvbsPrintMenu(MT_U32 prog_num)
{

    SAMPLE_DVBS_PRINT("\n 1 - %d : select the program \n", prog_num);
#ifdef MT_SAMPLE_APP
    SAMPLE_DVBS_PRINT("     b : background run \n");
#endif
    SAMPLE_DVBS_PRINT("     p : pause \n");
    SAMPLE_DVBS_PRINT("     r : resume \n");
    SAMPLE_DVBS_PRINT("     z : Channel Switch Mode \n");
    SAMPLE_DVBS_PRINT("     s : signal strength \n");
    SAMPLE_DVBS_PRINT("     l : signal quality \n");
	SAMPLE_DVBS_PRINT("     i : get signal information \n");
    SAMPLE_DVBS_PRINT("     k : multistream to play \n");
    SAMPLE_DVBS_PRINT("     d : check if audio dolby mono \n");
    SAMPLE_DVBS_PRINT("     m : set unblank screen mode\n");
    SAMPLE_DVBS_PRINT("     g : get first video frame show and avsync done cost time \n");
    SAMPLE_DVBS_PRINT("     h : help \n");
    SAMPLE_DVBS_PRINT("     q : quit \n");
    SAMPLE_DVBS_PRINT("DVBS>> ");

}

static MT_VOID MT_DvbsExit(void)
{
    MT_UNF_VCODEC_UNBLANK_E unblank;

    SAMPLE_DVBS_FUNCTION_ENTER();
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    (MT_VOID)MT_DvbsStopToPlay(g_stDvbsRunInfo.hAvPlay, 1);

    (MT_VOID)MT_DvbsAvplayDeInit(g_stDvbsRunInfo.hAvPlay, g_stDvbsRunInfo.hWin, g_stDvbsRunInfo.hSoundTrack);

    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbsRunInfo.pProgTbl);

    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        (MT_VOID)MT_DvbsDmxDeInit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_DVBS_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_DVBS_INFO_PRINT("PVR is recording now \n");
        SAMPLE_DVBS_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }

    (MT_VOID)MTADP_Snd_DeInit();

    (MT_VOID)MTADP_VO_DeInit();
    if(play_resource.rec_status != MT_TRUE)
    {
        (MT_VOID)MTADP_Fe_DeInit(g_stDvbsRunInfo.sInputParam.tuner_id);
    }

    memset(&g_stDvbsRunInfo, 0xff, sizeof(g_stDvbsRunInfo));
    g_bTaskQuit = MT_TRUE;

    MTADP_Get_VcodeUnblank(&unblank);
    if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
    {
        MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
    }

    SAMPLE_DVBS_FUNCTION_EXIT();
}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_DvbsPrint_help(char *name)
{
    SAMPLE_DVBS_PRINT(" [ options ]...\n"
       "\n"
       "Options:\n"
       " ?/-h/-H        print this help\n"
       " -t <tuner>     set tuner id \n"
       " -f <freq.M>    set freq (50~900) \n"
       " -s <srate.K>   set srate default:5361\n"
       " -k <22k>       set 22k on/off:0 is off, 1 is on\n"
       " -p <polar>     0/1:0 is the horizontal polarization,1 is vertically polarized\n"
       " -d <sig_type>  0/1/2:0 is dvbs, 1 is dvbs2, 2 is dvbs_auto\n"
       " -l <Lo_Freq.M>  Low Local Oscillator Frequency\n"
       " -g <Hi_Freq.M>  High Local Oscillator Frequency\n");
    SAMPLE_DVBS_PRINT("example: %s -t 0 -f 3840 -s 27500 -k 1 -p 0 -d 2 \n", name);
	SAMPLE_DVBS_PRINT("example: %s -t 0 -f 3840 -s 27500 -k 1 -p 0 -d 2 -l 5150 -g 5150 \n", name);
	SAMPLE_DVBS_PRINT("example: %s -t 0 -f 11060 -s 27500 -k 1 -p 0 -d 2 -l 9750 -g 9750 \n", name);
    SAMPLE_DVBS_PRINT("         %s -q  <exit> \n", name);
}





static void MT_DvbsCmdTask(MT_HANDLE     hAvplay, PMT_COMPACT_TBL *pProgTbl)
{
    MT_S32                  ret = 0;
    MT_U32                  u32ProgNum = 1;
#ifdef MT_SAMPLE_APP
    play_resource.s32ProgNum = u32ProgNum;
#endif
    MT_CHAR                 inputCmd[32] = { 0 };
    PMT_COMPACT_PROG        *stCurrentProgInfo = { 0 };
    mt_u32 strength = 0;
    mt_s32 agc = 0;
    mt_u32 quality = 0;
    mt_s32 accurate_snr = 0;
    mt_u32 snr = 0;
    MT_UNF_AVPLAY_STOP_MODE_E enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    mt_unf_fe_connect_para_t connect_para;
    mt_s32 ts_id = 0;
    mt_s32 i = 0;
    mt_s32 unblank = MT_UNF_VCODEC_UNBLANK_STABLE;
    mt_s64 first_vid_frm_show_time, avsync_done_time;

    while(1)
    {
        (MT_VOID)MT_DvbsPrintMenu(pProgTbl->prog_num);
        /* get inputCmd*/
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_DVBS_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit = MT_TRUE;
            if (MT_UNF_VCODEC_UNBLANK_STABLE != unblank)
            {
                MTADP_Set_VcodeUnblank(MT_UNF_VCODEC_UNBLANK_STABLE);
            }
            break;
        }
        else if('z' == inputCmd[0])
        {
            if(enmode == MT_UNF_AVPLAY_STOP_MODE_BLACK)
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_STILL;
                SAMPLE_DVBS_INFO_PRINT("Set mode to Freeze \n");
            }
            else
            {
                enmode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
                SAMPLE_DVBS_INFO_PRINT("Set mode to black \n");
            }

        }
        else if('p' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Pause(hAvplay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT("MT_UNF_AVPLAY_Pause failed\n");
            }
        }
        else if('r' == inputCmd[0])
        {
            ret = MT_UNF_AVPLAY_Resume(hAvplay, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT("MT_UNF_AVPLAY_Resume failed\n");
            }
        }
        else if('s' == inputCmd[0])
        {
            memset(&strength, 0, sizeof(strength));
            memset(&agc, 0, sizeof(agc));
            ret = mt_unf_fe_get_signal_strength(g_stDvbsRunInfo.sInputParam.tuner_id, &strength);
            ret = mt_unf_fe_get_agc(g_stDvbsRunInfo.sInputParam.tuner_id, 0, &agc);

            SAMPLE_DVBS_PRINT("\t Signal strength = %d, agc = %d\n", strength, agc);
        }
        else if('l' == inputCmd[0])
        {
            memset(&snr, 0, sizeof(snr));
            memset(&quality, 0, sizeof(quality));
            memset(&accurate_snr, 0, sizeof(accurate_snr));
            ret = mt_unf_fe_get_signal_quality(g_stDvbsRunInfo.sInputParam.tuner_id, &quality);
            ret = mt_unf_fe_get_snr(g_stDvbsRunInfo.sInputParam.tuner_id, &snr);
            ret = mt_unf_fe_get_accurate_snr(g_stDvbsRunInfo.sInputParam.tuner_id, &accurate_snr);

            SAMPLE_DVBS_PRINT("\t Signal quality = %d, snr = %d accurate_snr:%02d.%03d\n",
                quality, snr, accurate_snr/1000, accurate_snr%1000);
        }
		else if('i' == inputCmd[0])
		{
			ret = MTADP_Fe_Get_Signal_Info(g_stDvbsRunInfo.sInputParam.tuner_id);
			if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT("MTADP_Fe_Get_Signal_Info failed\n");
            }
		}
        else  if(inputCmd[0] > '0' && inputCmd[0] <= '9')
        {

            u32ProgNum = atoi(inputCmd);

            if(u32ProgNum > 0 && u32ProgNum <= pProgTbl->prog_num)
            {
                stCurrentProgInfo = pProgTbl->proginfo + ((u32ProgNum-1)% pProgTbl->prog_num);

                /** Stop AV playback into the stop state */
                ret = MT_DvbsStopToPlay(hAvplay, enmode);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DVBS_ERR_PRINT(" MT_DvbsStopToPlay failed.\n");
                }
                SAMPLE_DVBS_INFO_PRINT("Start play ProgNum: %d \n", u32ProgNum);
                // restore ac4    attr info.
                MTADP_AUD_RestoreAc4PlayAttrInfo(hAvplay);

                /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
                ret = MT_DvbsStarToPlay(hAvplay, stCurrentProgInfo);
                if(MT_SUCCESS != ret)
                {
                    SAMPLE_DVBS_ERR_PRINT(" SwitchProg failed.\n");
                    return;
                }
                play_resource.s32ProgNum = u32ProgNum;
            }
            else
            {
                SAMPLE_DVBS_ERR_PRINT(" prog_num the biggest is %d \n\n", pProgTbl->prog_num);
            }
#ifdef MT_SAMPLE_APP
            ret = MTADP_Set_Current_Info(stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT(" MTADP_Set_Current_Info failed.\n");
            }
#endif
        }
#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_DVBS_INFO_PRINT("Dvbs play in back!\n");
            break;
        }

#endif
        else if('k' == inputCmd[0])
        {

            ret = MT_DvbsStopToPlay(hAvplay, enmode);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT(" MT_DvbsStopToPlay failed.\n");
            }
            (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbsRunInfo.pProgTbl);

            mt_unf_fe_get_s2_multi_stream_info(g_stDvbsRunInfo.sInputParam.tuner_id, &connect_para);
            SAMPLE_DVBS_INFO_PRINT("total ts number id %d \n", connect_para.connect_param.sat.DataTsNumber);
            for (i = 0; i < connect_para.connect_param.sat.DataTsNumber; i++)
            {
                SAMPLE_DVBS_INFO_PRINT("\t\tTS[%d] ---- ts_id[%02x]\n", i, connect_para.connect_param.sat.DataTsIdArray[i]);
            }
            SAMPLE_DVBS_INFO_PRINT("please input you want to play ts_id \n");
            scanf("%d", &ts_id);
            mt_unf_fe_set_s2_multi_stream_ts_id(g_stDvbsRunInfo.sInputParam.tuner_id, ts_id);

            ret = MTADP_Search_GetAllPmt(DMX_ID_0, &pProgTbl);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT(" MTADP_Search_GetAllPmt failed.\n");
            }

            stCurrentProgInfo = pProgTbl->proginfo;

            ret = MT_DvbsStarToPlay(hAvplay, stCurrentProgInfo);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_DVBS_ERR_PRINT(" MT_DvbsStarToPlay failed.\n");
                return;
            }
        }
        else if('d' == inputCmd[0])
        {
            MT_UNF_AUDIOTRACK_ATTR_S trackAttr;
            memset(&trackAttr, 0, sizeof(MT_UNF_AUDIOTRACK_ATTR_S));
            ret = MT_UNF_SND_GetTrackAttr(g_stDvbsRunInfo.hSoundTrack, &trackAttr);
            if (MT_SUCCESS == ret && MT_FALSE != trackAttr.dolby_dd_ddp && MT_TRUE == trackAttr.dolby_dualmono)
            {
                SAMPLE_DVBS_INFO_PRINT("Audio dolby info: dolby[%d] Dual-Mono [1+1].\n", trackAttr.dolby_dd_ddp);
            }
        }
        else if ('m' == inputCmd[0])
        {
            SAMPLE_DVBS_PRINT("input unblank mode(0:fast 1:stable 2:sync):");
            scanf("%d", &unblank);
            getchar();
            SAMPLE_DVBS_PRINT("unblank: %d \n", unblank);
            unblank = unblank % MT_UNF_VCODEC_UNBLANK_BUTT;
            MTADP_Set_VcodeUnblank(unblank);
        }
        else if('g' == inputCmd[0])
        {
            (MT_VOID)MTADP_ReadPlayStat(&first_vid_frm_show_time, &avsync_done_time);
            SAMPLE_DVBS_PRINT("[time] first video frame showed cost time: %lldms \n", first_vid_frm_show_time);
            SAMPLE_DVBS_PRINT("[time] avsync done cost time: %lldms \n", avsync_done_time);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DVBS_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_DvbsParase_args(int argc, char *argv[], mt_input_Dvbs_para_t *pInutParam)
{
    int opt = 0;
    /** example: ./sample_dvbs -t 0 -f 3840 -s 27500 -k 1 -p 0 -d 2  */

    SAMPLE_DVBS_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, "?hHf:t:s:k:d:p:l:g:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_DvbsPrint_help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_DvbsExit();
                }
                return MT_TASK_EXIT;
            case 't':
                pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                break;
            case 'f':
                pInutParam->freq = strtol(mt_optarg, 0, 0);
                break;

            case 's':
                pInutParam->sym_rate = strtol(mt_optarg, 0, 0);
                break;

            case 'k':
                pInutParam->onoff_22k = strtol(mt_optarg, 0, 0);
                break;

            case 'd':
                pInutParam->dvbs_type = strtol(mt_optarg, 0, 0);
                break;

            case 'p':
                pInutParam->polar = strtol(mt_optarg, 0, 0);
                break;
				
            case 'l':
                pInutParam->low_lo = strtol(mt_optarg, 0, 0);
                break;
				
            case 'g':
                pInutParam->high_lo = strtol(mt_optarg, 0, 0);
                break;
				
            default:
                (void)MT_DvbsPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_DVBS_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DvbsMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32                  ret = 0;
    PMT_COMPACT_PROG        *pstCurrentProgInfo= {0};


    SAMPLE_DVBS_FUNCTION_ENTER();

    //if(argc != 13 && g_bTaskQuit == MT_TRUE)
	if(argc < 13 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_DvbsPrint_help(argv[0]);
        return MT_SUCCESS;
    }
	g_stDvbsRunInfo.sInputParam.low_lo = 0;
	g_stDvbsRunInfo.sInputParam.high_lo = 0;
    ret = MT_DvbsParase_args(argc, argv, &g_stDvbsRunInfo.sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_DVBS_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_DVBS_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        ret = MT_DvbsCheckParam(&g_stDvbsRunInfo.sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbsCheckParam failed.\n");
            return MT_FAILURE;
        }

#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("mt_sys_init error. ret=0x%x \n", ret);
            return MT_FAILURE;
        }
        /** HDMI initialization */
        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_HDMI_Init failed, ret = %x\n", ret);
            goto ERR0;
        }

        /** Display initialization */
        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_Disp_Init failed, ret = %x\n", ret);
            goto ERR1;
        }
#endif
        /** Tuner initialization, Set the default parameters for tuner */
        ret = MTADP_Fe_Init(g_stDvbsRunInfo.sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_Fe_Init failed, ret = %x\n", ret);
            goto ERR2;
        }
		
		ret = MT_DvbsFrontendConnect(&g_stDvbsRunInfo.sInputParam);
        //ret = MTADP_Fe_Connect_Dvbs(g_stDvbsRunInfo.sInputParam.tuner_id, g_stDvbsRunInfo.sInputParam.freq, g_stDvbsRunInfo.sInputParam.sym_rate, g_stDvbsRunInfo.sInputParam.onoff_22k, g_stDvbsRunInfo.sInputParam.polar, g_stDvbsRunInfo.sInputParam.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_Fe_Connect_Dvbs error\n");
            goto ERR2;
        }

        /** VO device initialization */
        ret = MTADP_VO_Init(MT_UNF_VO_DEV_MODE_NORMAL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_VO_Init failed, ret = %x\n", ret);
            goto ERR3;
        }

        /** Audio device initialization */
        ret = MTADP_Snd_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MTADP_Snd_Init failed, ret = %x\n", ret);
            goto ERR4;
        }

        /** Demux initializes and retrieves the PMT and PAT tables in TS */
        ret = MT_DvbsDmxInit();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbcDmxInit failed, ret = %x\n", ret);
            goto ERR5;
        }

        /** The search module is initialized */
        (MT_VOID)MTADP_Search_Init();

        /** Get the PMT table */
        ret = MTADP_Search_GetAllPmt(DMX_ID_0, &g_stDvbsRunInfo.pProgTbl);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_DVBS_ERR_PRINT("MTADP_Search_GetAllPmt failed.\n");
            goto ERR7;

        }

        ret = MT_DvbsAvplayInit(&g_stDvbsRunInfo.hAvPlay, &g_stDvbsRunInfo.hWin, &g_stDvbsRunInfo.hSoundTrack);
        if(MT_SUCCESS != ret)
        {

            SAMPLE_DVBS_ERR_PRINT("MT_DvbsAvplayInit failed.\n");
            goto ERR8;
        }
#ifdef MT_SAMPLE_APP
        avplayHandle.hAvPlay = g_stDvbsRunInfo.hAvPlay;
        avplayHandle.hSoundTrack = g_stDvbsRunInfo.hSoundTrack;
        avplayHandle.hWin = g_stDvbsRunInfo.hWin;
#endif
           /* Play the first program on the program list*/
        pstCurrentProgInfo = g_stDvbsRunInfo.pProgTbl->proginfo;
        /** Start the AV playback into the start state, play according to pstCurrentProgInfo */
        ret = MT_DvbsStarToPlay(g_stDvbsRunInfo.hAvPlay, pstCurrentProgInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_DVBS_ERR_PRINT("MT_DvbsStarToPlay failed.\n");
            goto ERR9;
        }

        g_bTaskQuit = MT_FALSE;

    }


    (void)MT_DvbsCmdTask(g_stDvbsRunInfo.hAvPlay, g_stDvbsRunInfo.pProgTbl);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }
#ifdef MT_SAMPLE_APP
    memset(&avplayHandle, 0, sizeof(avplayHandle));
    usleep(1000*500);
#endif
    /** Stop AV playback and enter the stop state */
    ret = MT_DvbsStopToPlay(g_stDvbsRunInfo.hAvPlay, 1);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DVBS_ERR_PRINT("MT_DvbsStopToPlay failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
    }


ERR9:
    /** Audio and video player deinitialization */
    (MT_VOID)MT_DvbsAvplayDeInit(g_stDvbsRunInfo.hAvPlay, g_stDvbsRunInfo.hWin, g_stDvbsRunInfo.hSoundTrack);

ERR8:
    /** Release the PMT table */
    (MT_VOID)MTADP_Search_FreeAllPmt(g_stDvbsRunInfo.pProgTbl);
ERR7:
    (MT_VOID)MTADP_Search_DeInit();

    if(play_resource.rec_status != MT_TRUE)
    {
        /** Demux module deinitialization */
        (MT_VOID)MT_DvbsDmxDeInit();
        play_resource.demux_use = MT_FALSE;
    }
    else
    {
        play_resource.demux_use = MT_TRUE;
        SAMPLE_DVBS_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        SAMPLE_DVBS_INFO_PRINT("PVR is recording now \n");
        SAMPLE_DVBS_INFO_PRINT("+++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    }
ERR5:
    /** Audio output is deinitialized */
    (MT_VOID)MTADP_Snd_DeInit();

ERR4:
    /** VO device deinitialization */
    (MT_VOID)MTADP_VO_DeInit();
ERR3:
    /** Disconnect the tuner lock */
    (MT_VOID)MTADP_Fe_DeInit(g_stDvbsRunInfo.sInputParam.tuner_id);

ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR1:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR0:
    (MT_VOID)mt_sys_deinit();
#endif

    g_bTaskQuit = MT_TRUE;
    memset(&g_stDvbsRunInfo, 0xff, sizeof(g_stDvbsRunInfo));

    //reset ac4 config attr.
    MTADP_AUD_ResetAc4PlayAttrInfo();

    return ret;
}

