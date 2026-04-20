#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

#include "mt_adp_frontend.h"
#include "mt_adp_boardcfg.h"
#include "mt_unf_frontend.h"
#include "mt_unf_demux.h"
#include "mt_adp_debug.h"


#ifdef MTADP_FRONT_DEBUG

#define MTADP_FRONT_PRINT   printf

#else

#define MTADP_FRONT_PRINT

#endif


#define MTADP_FRONT_FUNCTION_ENTER()  MTADP_FRONT_PRINT("[MTADP_FRONT][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_FRONT_FUNCTION_EXIT()   MTADP_FRONT_PRINT("[MTADP_FRONT][%s]: Exit ==<< \n", __FUNCTION__)

#define MTADP_FRONT_FATAL_PRINT(fmt...)       MTADP_FRONT_PRINT(" [MTADP_FRONT][FATAL] " fmt)
#define MTADP_FRONT_ERR_PRINT(fmt...)         MTADP_FRONT_PRINT(" [MTADP_FRONT][ERROR] " fmt)
#define MTADP_FRONT_WARN_PRINT(fmt...)        MTADP_FRONT_PRINT(" [MTADP_FRONT][WARN] "  fmt)
#define MTADP_FRONT_INFO_PRINT(fmt...)        MTADP_FRONT_PRINT(" [MTADP_FRONT][INFO] "  fmt)
#define MTADP_FRONT_MT_DBG_PRINT(fmt...)      MTADP_FRONT_PRINT(" [MTADP_FRONT][DEBUG] " fmt)

#define MTADP_FRONT_SIGNAL_PRINT(fmt...)      printf(fmt)

/*!
@brief Data type conversion
@param[in]  qamtype             QAM modulation method
@return::mod_type               The result of the data type conversion
@*/
static mt_unf_modulation_type_t MT_Tuner_Get_Modtype(MT_U32 qamtype)
{
  mt_unf_modulation_type_t mod_type;
  switch(qamtype)
  {
    case 16:
      mod_type = MT_UNF_MOD_TYPE_QAM_16;
      break;
    case 32:
      mod_type = MT_UNF_MOD_TYPE_QAM_32;
      break;
    case 64:
      mod_type = MT_UNF_MOD_TYPE_QAM_64;
      break;
    case 128:
      mod_type = MT_UNF_MOD_TYPE_QAM_128;
      break;
    case 256:
      mod_type = MT_UNF_MOD_TYPE_QAM_256;
      break;
    case 512:
      mod_type = MT_UNF_MOD_TYPE_QAM_512;
      break;
    default:
      mod_type = MT_UNF_MOD_TYPE_QAM_64;
      break;
  }
  return mod_type;
}


/*!
@brief Tuner locked, set the frequency locking parameters
@param[in]  tuner_id            Tuner ID
@param[in]  freq                Tuner frequency
@param[in]  sym_rate            Tuner symbol rate
@param[in]  qam_type            QAM modulation mode
@param[in]  bReverse            Spectrum reverse mode
@param[in]  timeout             Timeout
@param[in]  signal_type         Signal type
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Fail.
@return::ret                    The return value of the error.
@*/
static mt_s32 MT_Tuner_Lock_Dvbc(MT_U32 tuner_id, MT_U32 freq, MT_U32 sym_rate, MT_U32 qam_type, MT_BOOL bReverse, MT_U32 timeout)
{
    MT_S32                   ret = MT_FAILURE;
    mt_unf_fe_status_t       stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t connectPara = { 0 };

    MTADP_FRONT_FUNCTION_ENTER();

    /** Connection parameters */
    connectPara.sig_type = MT_UNF_FE_SIG_TYPE_CAB;
    connectPara.connect_param.cab.b_reverse = bReverse;
    connectPara.connect_param.cab.freq = freq * 1000;
    connectPara.connect_param.cab.sym_rate = sym_rate * 1000;
    connectPara.connect_param.cab.mod_type = MT_Tuner_Get_Modtype(qam_type);
    connectPara.connect_param.cab.band_width = 8;

    MTADP_FRONT_INFO_PRINT("freq = %d, symbol rate = %d, qam_type = %d \n",freq, sym_rate, qam_type);

    /** Start connect, param[in] [1]Tuner ID [2]Tuner frequency lock parameters [3]Timeout */
    ret = mt_unf_fe_connect(tuner_id, &connectPara, timeout);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_unf_fe_connect failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("Frequency lock successful!. ret = 0x%x\n",  ret);

    /** Get status, param[in/out] [1/in]Tuner ID [2/out]Frequency locking status and parameters of the tuner */
    ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_unf_fe_get_status failed. ret = 0x%x\n",  ret);
        return ret;
    }

    switch(stTunerStatus.lock_status)
    {
        case MT_UNF_FE_SIGNAL_LOCKED:
            MTADP_FRONT_INFO_PRINT("lock: f[%d], s[%d]\n", freq, sym_rate);
            break;
        case MT_UNF_FE_SIGNAL_DROPPED:
            MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_DROPPED\nunlock: f[%d], s[%d]\n", freq, sym_rate);
            break;
        case MT_UNF_FE_SIGNAL_BUTT:
            MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_BUTT\nunlock: f[%d], s[%d]\n", freq, sym_rate);
            break;
        default:
            MTADP_FRONT_INFO_PRINT("unlock: f[%d], s[%d]\n", freq, sym_rate);
            break;
    }

    MTADP_FRONT_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*
@brief connect dvbs
@param[in] tuner_id, tuner port number
@param[in] freq, frequency
@param[in] sym_rate, Rate of transmission
@param[in] onoff_22k, 22k
@param[in] polar, Mode of polarization
@param[in] port_type, Type of signal
@param[in] u32LoopTimes, Time out
@return MT_SUCCESS
@return MT_FAILURE
*/
static MT_S32 MT_Tuner_Lock_Dvbs(MT_U32 tuner_id, MT_U32 freq, MT_U32 sym_rate, MT_U32 onoff_22k, MT_U32 polar, MT_U32 port_type,MT_U32 u32LoopTimes)
{
    MT_S32                   ret = 0;
    MT_U32                   u32Loop = 0;
    MT_U32                   u32Freq = 0;
    MT_U32                   u32SymbolRate = 0;
    mt_unf_fe_status_t       stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t stConnectPara = { 0 };

    MTADP_FRONT_FUNCTION_ENTER();

    if (MT_UNF_PORT_TYPE_DVBS == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT;
        MTADP_FRONT_INFO_PRINT("dvbs\n");
    }
    else if (MT_UNF_PORT_TYPE_DVBS2 == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_SAT_2;
        MTADP_FRONT_INFO_PRINT("dvbs2\n");
    }
    else if(MT_UNF_PORT_TYPE_DVBS_AUTO == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVBS_AUTO;
        MTADP_FRONT_INFO_PRINT("dvbs_auto\n");
    }

    stConnectPara.connect_param.sat.freq = freq * 1000;
    stConnectPara.connect_param.sat.sym_rate = sym_rate;
    stConnectPara.connect_param.sat.port_type = port_type;
    stConnectPara.connect_param.sat.onoff_22k = onoff_22k;
    stConnectPara.connect_param.sat.polarization = polar;

    ret = mt_unf_fe_connect(tuner_id, &stConnectPara, 2000);
    /* Get the actual lock frequency and rate*/
    u32Freq = stConnectPara.connect_param.sat.freq;
    u32SymbolRate = stConnectPara.connect_param.sat.sym_rate;

    if (MT_SUCCESS == ret)
    {
        for (u32Loop = 0; u32Loop < u32LoopTimes; u32Loop++)
        {
            ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
            if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
            {
                MTADP_FRONT_INFO_PRINT("Tuner Lock freq %d symb %d polar%d Success!\n", u32Freq, u32SymbolRate, polar);
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
        MTADP_FRONT_ERR_PRINT("Tuner Lock freq %d symb %d polar%d mt_unf_fe_connect Fail!, ret = 0x%x\n", u32Freq, u32SymbolRate, polar, ret);
    }

    if (u32Loop == u32LoopTimes)
    {
        MTADP_FRONT_ERR_PRINT("Tuner Lock freq %d symb %d  polar%d Fail!\n", u32Freq, u32SymbolRate, polar);
    }

    MTADP_FRONT_FUNCTION_EXIT();

    return MT_FAILURE;
}


/*****************************************************************************
*brief DVBT/DVBT2 frequency locking
*param[in] tuner_id, frequency locking id
*param[in] sig_type, signal type
*param[in] freq, The frequency of the input required
*param[in] band_width   The required input signal bandwidth
*param[in] plp_id   pid
*param[in] port_type  Signal type
*param[in] timeout   timeout
*****************************************************************************/
static mt_s32 MT_Tuner_Lock_Dvbt(MT_U32 tuner_id, MT_U32 freq, MT_U32 band_width,
                                                  MT_BOOL plp_id, MT_U32 port_type, MT_U32 timeout)
{
    MT_S32 ret = 0;
    mt_unf_fe_status_t stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t stConnectPara = { 0 };
    mt_u8 plp_num = 0;
    mt_u8 i = 0;

    MTADP_FRONT_FUNCTION_ENTER();
    if(MT_UNF_PORT_TYPE_DVBT == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T;
        MTADP_FRONT_INFO_PRINT("Signal Type set to DVB_T \n");
    }
    else if(MT_UNF_PORT_TYPE_DVBT2 == port_type)
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVB_T2;
        MTADP_FRONT_INFO_PRINT("Signal Type set to DVB_T2 \n");
    }
    else
    {
        stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_DVBT_AUTO;
        MTADP_FRONT_INFO_PRINT("Signal Type set to DVB_T_AUTO \n");
    }

    stConnectPara.connect_param.ter.freq = freq * 1000;
    stConnectPara.connect_param.ter.band_width = band_width * 1000;
    stConnectPara.connect_param.ter.plp_id = plp_id;
    stConnectPara.connect_param.ter.port_type = port_type;

    //connect
    ret = mt_unf_fe_connect(tuner_id, &stConnectPara, timeout);
    if(MT_SUCCESS != ret)
    {
      MTADP_FRONT_ERR_PRINT(" mt_unf_fe_connect fail \n");
      return ret;
    }

    //get status
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    for(int j = 0;j < 10;j++)
    {
        ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
        if(MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
        {
            MTADP_FRONT_INFO_PRINT("lock: f[%d], bw[%d],port_type[%d]\n", freq, band_width,stConnectPara.connect_param.ter.port_type);
            mt_unf_fe_get_plpnum(tuner_id, &plp_num);
            for(i = 0; i < plp_num; i++)
            {
                MTADP_FRONT_INFO_PRINT("dvbt&t2 plp_id=%d\n",i);
            }
			
			MTADP_FRONT_FUNCTION_EXIT();

            return MT_SUCCESS;
        }
        else
        {
            usleep(10000);
        }
    }
    MTADP_FRONT_INFO_PRINT("unlock: f[%d], bw[%d]\n", freq, band_width);
    ret = MT_FAILURE;
#else
    ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
    switch(stTunerStatus.lock_status)
    {
      case MT_UNF_FE_SIGNAL_LOCKED:
        MTADP_FRONT_INFO_PRINT("lock: f[%d], bw[%d]\n", freq, band_width);
        mt_unf_fe_get_plpnum(tuner_id, &plp_num);
        for (i = 0; i < plp_num; i++)
        {
          MTADP_FRONT_INFO_PRINT("dvbt&t2 plp_id=%d\n",i);
        }
        break;
      case MT_UNF_FE_SIGNAL_DROPPED:
        MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_DROPPED\nunlock: f[%d], bw[%d]\n", freq, band_width);
        break;
      case MT_UNF_FE_SIGNAL_BUTT:
        MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_BUTT\nunlock: f[%d], bw[%d]\n", freq, band_width);
        break;
      default:
        MTADP_FRONT_INFO_PRINT("unlock: f[%d], bw[%d]\n", freq, band_width);
        break;
    }
#endif
    MTADP_FRONT_FUNCTION_EXIT();

    return ret;
}

static mt_s32 MT_Tuner_Lock_Dvbt_Certainty_Signal(mtadp_dvbt_info_t *p_dvbt_info)
{
    MT_S32 ret = 0;
    mt_unf_fe_status_t stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t stConnectPara = { 0 };
    mt_u8 plp_num = 0;
    mt_u8 i = 0;
	mt_u8 port_type = 0;

    MTADP_FRONT_FUNCTION_ENTER();
    if( p_dvbt_info->signal_type == MT_UNF_FE_SIG_TYPE_DVB_T)
    {
		port_type =MT_UNF_PORT_TYPE_DVBT;
        MTADP_FRONT_INFO_PRINT("Signal Type set to DVB_T \n");
    }
    else if( p_dvbt_info->signal_type == MT_UNF_FE_SIG_TYPE_DVB_T2)
    {
        port_type = MT_UNF_PORT_TYPE_DVBT2;
		stConnectPara.connect_param.ter.channel_mode = p_dvbt_info->channel_mode;
        MTADP_FRONT_INFO_PRINT("Signal Type set to DVB_T2 channel_mode:%d p_dvbt_info->channel_mode:%d\n",stConnectPara.connect_param.ter.channel_mode,p_dvbt_info->channel_mode);
    }
    else
    {
        port_type = MT_UNF_PORT_TYPE_DVBT_AUTO;
        MTADP_FRONT_INFO_PRINT("Signal Type set to DVB_T_AUTO \n");
    }
	stConnectPara.sig_type = p_dvbt_info->signal_type;
    stConnectPara.connect_param.ter.freq = p_dvbt_info->freq * 1000;
    stConnectPara.connect_param.ter.band_width = p_dvbt_info->band_width * 1000;
    stConnectPara.connect_param.ter.plp_id = p_dvbt_info->plp_id;
    stConnectPara.connect_param.ter.port_type = port_type;

	
    //connect
    ret = mt_unf_fe_connect(p_dvbt_info->tunerId, &stConnectPara, p_dvbt_info->lock_timeout);
    if(MT_SUCCESS != ret)
    {
      MTADP_FRONT_ERR_PRINT(" mt_unf_fe_connect fail \n");
      return ret;
    }

    //get status
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    for(int j = 0;j < 10;j++)
    {
        ret = mt_unf_fe_get_status(p_dvbt_info->tunerId, &stTunerStatus);
        if(MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
        {
            MTADP_FRONT_INFO_PRINT("lock: f[%d], bw[%d],port_type[%d],signal_type[%d]\n", p_dvbt_info->freq, p_dvbt_info->band_width,
																	stTunerStatus.param.channel_info.port_type,stConnectPara.sig_type);
            mt_unf_fe_get_plpnum(p_dvbt_info->tunerId, &plp_num);
            for(i = 0; i < plp_num; i++)
            {
                MTADP_FRONT_INFO_PRINT("dvbt&t2 plp_id=%d\n",i);
            }
			/*update info*/
			if(stTunerStatus.param.channel_info.port_type == MT_UNF_FE_DVBT2)
        	{
				p_dvbt_info->plp_num = stConnectPara.connect_param.ter.data_plp_number;
				p_dvbt_info->plp_id = stConnectPara.connect_param.ter.data_plpid_array[0];
				p_dvbt_info->channel_mode = stConnectPara.connect_param.ter.channel_mode;
				MTADP_FRONT_INFO_PRINT("p_dvbt_info plp_num=%d,plp_id=%d,channel_mode=%d\n",p_dvbt_info->plp_num,p_dvbt_info->plp_id,p_dvbt_info->channel_mode);
			}
			MTADP_FRONT_FUNCTION_EXIT();
			

            return MT_SUCCESS;
        }
        else
        {
            usleep(10000);
        }
    }
    MTADP_FRONT_INFO_PRINT("unlock: f[%d], bw[%d]\n", p_dvbt_info->freq, p_dvbt_info->band_width);
    ret = MT_FAILURE;
#else
    ret = mt_unf_fe_get_status(p_dvbt_info->tunerId, &stTunerStatus);
    switch(stTunerStatus.lock_status)
    {
      case MT_UNF_FE_SIGNAL_LOCKED:
        MTADP_FRONT_INFO_PRINT("lock: f[%d], bw[%d]\n", p_dvbt_info->freq, p_dvbt_info->band_width);
        mt_unf_fe_get_plpnum(p_dvbt_info->tunerId, &plp_num);
        for (i = 0; i < plp_num; i++)
        {
          MTADP_FRONT_INFO_PRINT("dvbt&t2 plp_id=%d\n",i);
        }
        break;
      case MT_UNF_FE_SIGNAL_DROPPED:
        MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_DROPPED\nunlock: f[%d], bw[%d]\n", p_dvbt_info->freq, p_dvbt_info->band_width);
        break;
      case MT_UNF_FE_SIGNAL_BUTT:
        MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_BUTT\nunlock: f[%d], bw[%d]\n", p_dvbt_info->freq, p_dvbt_info->band_width);
        break;
      default:
        MTADP_FRONT_INFO_PRINT("unlock: f[%d], bw[%d]\n", p_dvbt_info->freq, p_dvbt_info->band_width);
        break;
    }
#endif
    MTADP_FRONT_FUNCTION_EXIT();

    return ret;
}

/*!
@brief Tuner locked, set the frequency locking parameters.
@param[in]  tuner_id            Tuner ID
@param[in]  freq                Tuner frequency
@param[in]  sym_rate            Tuner symbol rate
@param[in]  qam_type            QAM modulation mode
@param[in]  bReverse            Spectrum reverse mode
@param[in]  timeout             Timeout
@return::MT_SUCCESS             Success.
@return::MT_FAILURE             Fail.
@return::ret                    The return value of the error.
@*/
 static mt_s32 MT_Tuner_Lock_J83b(MT_U32 tuner_id, MT_U32 freq, MT_U32 sym_rate, MT_U32 qam_type, MT_BOOL bReverse,MT_U32 timeout)
{
    MT_S32                   ret = MT_FAILURE;
    mt_unf_fe_status_t       stTunerStatus = { 0 };
    mt_unf_fe_connect_para_t s_stConnectPara = { 0 };

    MTADP_FRONT_FUNCTION_ENTER();

    /** J83B Connection parameters */
    s_stConnectPara.sig_type = MT_UNF_FE_SIG_TYPE_J83B;
    s_stConnectPara.connect_param.cab.b_reverse = bReverse;
    s_stConnectPara.connect_param.cab.freq = freq * 1000;
    s_stConnectPara.connect_param.cab.sym_rate = sym_rate * 1000;
    s_stConnectPara.connect_param.cab.mod_type = MT_Tuner_Get_Modtype(qam_type);
    s_stConnectPara.connect_param.cab.band_width = 8;

    /** Start connect, param[in] [1]Tuner ID [2]Tuner frequency lock parameters [3]Timeout */
    ret = mt_unf_fe_connect(tuner_id, &s_stConnectPara, timeout);
    MTADP_FRONT_INFO_PRINT("mt_unf_fe_connect ret:%d\n", ret);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT(" mt_unf_fe_connect fail \n");
        return ret;
    }

    /** Get status, param[in/out] [1/in]Tuner ID [2/out]Frequency locking status and parameters of the tuner */
    ret = mt_unf_fe_get_status(tuner_id, &stTunerStatus);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT(" mt_unf_fe_get_status fail \n");
        return ret;
    }

    switch(stTunerStatus.lock_status)
    {
        case MT_UNF_FE_SIGNAL_LOCKED:
            MTADP_FRONT_INFO_PRINT("lock: f[%d], s[%d]\n", freq, sym_rate);
            break;
        case MT_UNF_FE_SIGNAL_DROPPED:
            MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_DROPPED\nunlock: f[%d], s[%d]\n", freq, sym_rate);
            break;
        case MT_UNF_FE_SIGNAL_BUTT:
            MTADP_FRONT_INFO_PRINT("MT_UNF_FE_SIGNAL_BUTT\nunlock: f[%d], s[%d]\n", freq, sym_rate);
            break;
        default:
            MTADP_FRONT_ERR_PRINT("unlock: f[%d], s[%d]\n", freq, sym_rate);
            break;
    }

    MTADP_FRONT_FUNCTION_EXIT();

    return MT_SUCCESS;
}


/*
@brief set tuner parameters
@param[in] tuner_id,Port of the tuner
@param[in] sig_type,Type of received signal
@param[in] tuner_dev_type,tuner Device type
@param[in] tuner_addr, The address of tuner
@param[in] demod_dev_type, Type of the demod device
@param[in] demod_addr, The address of demod
@param[in] out_put_mode, Output mode
@param[in] I2c_channel, i2c Channel mode
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_Tuner_Set_Parameter(MT_U32 tuner_id, MT_U32 sig_type, MT_U32 tuner_dev_type, MT_U32 tuner_addr,
             MT_U32 demod_dev_type, MT_U32 demod_addr, MT_U32 out_put_mode, MT_U32 I2c_channel)
{
    MT_S32           ret = 0;
    mt_unf_fe_attr_t mtTunerAttr = { 0 };
    mt_sys_version_s stSysChipInfo = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_sys_get_version err!\n");
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("MTCommand_Tuner_Operation:   chipVersion = 0x%x\n", stSysChipInfo.enChipVersion);

    ret = mt_unf_fe_get_default_attr(tuner_id,&mtTunerAttr);
    if(SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_unf_fe_get_default_attr err!\n");
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

    MTADP_FRONT_INFO_PRINT("tuner_type[%d], tuner_addr[0x%x], demod_type[%d], demod_addr[0x%x], demod_id[%d], output_mode[%d] \n",
                            mtTunerAttr.tuner_type, mtTunerAttr.tuner_addr,
                            mtTunerAttr.demod_dev_type, mtTunerAttr.demod_addr,
                            mtTunerAttr.demod_i2c_id, mtTunerAttr.output_mode );
    ret = mt_unf_fe_set_attr(tuner_id, &mtTunerAttr);
    if (MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_attr failed.ret is0x%x\n",ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("output_mode = 0x%x\n", out_put_mode);

    return MT_SUCCESS;
}
static void MT_Tuner_Format_Signal_Type_String(mt_unf_fe_sig_type_t sig_type, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (sig_type)
  {
    case MT_UNF_FE_SIG_TYPE_CAB:
      strcpy(dest_str, "DVB-C");
      break;

    case MT_UNF_FE_SIG_TYPE_SAT:
      strcpy(dest_str, "DVB-S");
      break;

    case MT_UNF_FE_SIG_TYPE_SAT_2:
      strcpy(dest_str, "DVB-S2");
      break;

    case MT_UNF_FE_SIG_TYPE_DVB_T:
      strcpy(dest_str, "DVB-T");
      break;

    case MT_UNF_FE_SIG_TYPE_DVB_T2:
      strcpy(dest_str, "DVB-T2");
      break;

    case MT_UNF_FE_SIG_TYPE_ISDB_T:
      strcpy(dest_str, "ISDB-T");
      break;

    case MT_UNF_FE_SIG_TYPE_ATSC_T:
      strcpy(dest_str, "ATSC-T");
      break;

    case MT_UNF_FE_SIG_TYPE_DTMB:
      strcpy(dest_str, "DTMB");
      break;

    case MT_UNF_FE_SIG_TYPE_J83B:
      strcpy(dest_str, "J.83B");
      break;

    case MT_UNF_FE_SIG_TYPE_CTTB:
      strcpy(dest_str, "CTTB");
      break;

    case MT_UNF_FE_SIG_TYPE_DVBT_AUTO:
      strcpy(dest_str, "DVB-T/T2 Auto");
      break;

    case MT_UNF_FE_SIG_TYPE_DVBS_AUTO:
      strcpy(dest_str, "DVB-S/S2 Auto");
      break;

    default:
      strcpy(dest_str, "Unknown signal type");
      break;
  }

  return;
}
static void MT_Tuner_Format_Modulation_Mode_String(mt_unf_modulation_type_t mod_type, char *dest_str)
{
  char sig_type_str[20];

  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (mod_type)
  {
    case MT_UNF_MOD_TYPE_QAM_4:
      strcpy(dest_str, "4QAM");
      break;

    case MT_UNF_MOD_TYPE_QAM_4_NR:
      strcpy(dest_str, "4QAM NR");
      break;

    case MT_UNF_MOD_TYPE_QAM_16:
      strcpy(dest_str, "16QAM");
      break;

    case MT_UNF_MOD_TYPE_QAM_32:
      strcpy(dest_str, "32QAM");
      break;

    case MT_UNF_MOD_TYPE_QAM_64:
      strcpy(dest_str, "64QAM");
      break;

    case MT_UNF_MOD_TYPE_QAM_128:
      strcpy(dest_str, "128QAM");
      break;

    case MT_UNF_MOD_TYPE_QAM_256:
      strcpy(dest_str, "256QAM");
      break;

    case MT_UNF_MOD_TYPE_QAM_512:
      strcpy(dest_str, "512QAM");
      break;

    case MT_UNF_MOD_TYPE_BPSK:
      strcpy(dest_str, "BPSK");
      break;

    case MT_UNF_MOD_TYPE_QPSK:
      strcpy(dest_str, "QPSK");
      break;

    case MT_UNF_MOD_TYPE_DQPSK:
      strcpy(dest_str, "DQPSK");
      break;

    case MT_UNF_MOD_TYPE_8PSK:
      strcpy(dest_str, "8PSK");
      break;

    case MT_UNF_MOD_TYPE_16APSK:
      strcpy(dest_str, "16APSK");
      break;

    case MT_UNF_MOD_TYPE_32APSK:
      strcpy(dest_str, "32APSK");
      break;

    case MT_UNF_MOD_TYPE_64APSK:
      strcpy(dest_str, "64APSK");
      break;

    case MT_UNF_MOD_TYPE_128APSK:
      strcpy(dest_str, "128APSK");
      break;

    case MT_UNF_MOD_TYPE_256APSK:
      strcpy(dest_str, "256APSK");
      break;

    case MT_UNF_MOD_TYPE_8APSK_L:
      strcpy(dest_str, "8APSK_L");
      break;

    case MT_UNF_MOD_TYPE_16APSK_L:
      strcpy(dest_str, "16APSK_L");
      break;

    case MT_UNF_MOD_TYPE_32APSK_L:
      strcpy(dest_str, "32APSK_L");
      break;

    case MT_UNF_MOD_TYPE_64APSK_L:
      strcpy(dest_str, "64APSK_L");
      break;

    case MT_UNF_MOD_TYPE_128APSK_L:
      strcpy(dest_str, "128APSK_L");
      break;

    case MT_UNF_MOD_TYPE_256APSK_L:
      strcpy(dest_str, "256APSK_L");
      break;

    case MT_UNF_MOD_TYPE_8VSB:
      strcpy(dest_str, "8VSB");
      break;

    case MT_UNF_MOD_TYPE_16VSB:
      strcpy(dest_str, "16VSB");
      break;

    case MT_UNF_MOD_TYPE_AUTO:
      strcpy(dest_str, "Mod Auto");
      break;

    default:
      strcpy(dest_str, "Unknown Mod");
      break;
  }

  return;
}
static void MT_Tuner_Format_Code_Rate_String(mt_unf_fe_fecrate_t code_rate, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (code_rate)
  {
    case MT_UNF_FE_FEC_1_2:
      strcpy(dest_str, "1/2");
      break;

    case MT_UNF_FE_FEC_2_3:
      strcpy(dest_str, "2/3");
      break;

    case MT_UNF_FE_FEC_3_4:
      strcpy(dest_str, "3/4");
      break;

    case MT_UNF_FE_FEC_4_5:
      strcpy(dest_str, "4/5");
      break;

    case MT_UNF_FE_FEC_5_6:
      strcpy(dest_str, "5/6");
      break;

    case MT_UNF_FE_FEC_6_7:
      strcpy(dest_str, "6/7");
      break;

    case MT_UNF_FE_FEC_7_8:
      strcpy(dest_str, "7/8");
      break;

    case MT_UNF_FE_FEC_8_9:
      strcpy(dest_str, "8/9");
      break;

    case MT_UNF_FE_FEC_9_10:
      strcpy(dest_str, "9/10");
      break;

    case MT_UNF_FE_FEC_1_4:
      strcpy(dest_str, "1/4");
      break;

    case MT_UNF_FE_FEC_1_3:
      strcpy(dest_str, "1/3");
      break;

    case MT_UNF_FE_FEC_2_5:
      strcpy(dest_str, "2/5");
      break;

    case MT_UNF_FE_FEC_3_5:
      strcpy(dest_str, "3/5");
      break;

    case MT_UNF_FE_FEC_5_9:
      strcpy(dest_str, "5/9");
      break;

    case MT_UNF_FE_FEC_7_9:
      strcpy(dest_str, "7/9");
      break;

    case MT_UNF_FE_FEC_4_15:
      strcpy(dest_str, "4/15");
      break;

    case MT_UNF_FE_FEC_7_15:
      strcpy(dest_str, "7/15");
      break;

    case MT_UNF_FE_FEC_8_15:
      strcpy(dest_str, "8/15");
      break;

    case MT_UNF_FE_FEC_11_15:
      strcpy(dest_str, "11/15");
      break;

    case MT_UNF_FE_FEC_13_18:
      strcpy(dest_str, "13/18");
      break;

    case MT_UNF_FE_FEC_9_20:
      strcpy(dest_str, "9/20");
      break;

    case MT_UNF_FE_FEC_11_20:
      strcpy(dest_str, "11/20");
      break;

    case MT_UNF_FE_FEC_23_36:
      strcpy(dest_str, "23/36");
      break;

    case MT_UNF_FE_FEC_25_36:
      strcpy(dest_str, "25/36");
      break;

    case MT_UNF_FE_FEC_11_45:
      strcpy(dest_str, "11/45");
      break;

    case MT_UNF_FE_FEC_13_45:
      strcpy(dest_str, "13/45");
      break;

    case MT_UNF_FE_FEC_14_45:
      strcpy(dest_str, "14/45");
      break;

    case MT_UNF_FE_FEC_26_45:
      strcpy(dest_str, "26/45");
      break;

    case MT_UNF_FE_FEC_28_45:
      strcpy(dest_str, "28/45");
      break;

    case MT_UNF_FE_FEC_29_45:
      strcpy(dest_str, "29/45");
      break;

    case MT_UNF_FE_FEC_31_45:
      strcpy(dest_str, "31/45");
      break;

    case MT_UNF_FE_FEC_32_45:
      strcpy(dest_str, "32/45");
      break;

    case MT_UNF_FE_FEC_77_90:
      strcpy(dest_str, "77/90");
      break;

    case MT_UNF_FE_FEC_RESERVED:
    case MT_UNF_FE_FEC_UNDEF:
    default:
      strcpy(dest_str, "Unknown");
      break;
  }

  return;
}
static void MT_Tuner_Format_FEC_Type_String(mt_unf_fe_fec_type_t fec_type, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (fec_type)
  {
    case MT_UNF_FE_DVBS:
      strcpy(dest_str, "DVB-S");
      break;

    case MT_UNF_FE_DVBS2:
      strcpy(dest_str, "DVB-S2");
      break;

    case MT_UNF_FE_DIRECTV:
      strcpy(dest_str, "DIRECTV");
      break;

    case MT_UNF_FE_DVBC:
      strcpy(dest_str, "DVB-C");
      break;

    case MT_UNF_FE_J83B:
      strcpy(dest_str, "J.83B");
      break;

    case MT_UNF_FE_DVBT:
      strcpy(dest_str, "DVB-T");
      break;

    case MT_UNF_FE_DVBT2:
      strcpy(dest_str, "DVB-T2");
      break;

    case MT_UNF_FE_DTMB:
      strcpy(dest_str, "DTMB");
      break;

    case MT_UNF_FE_CTTB:
      strcpy(dest_str, "CTTB");
      break;

    default:
      strcpy(dest_str, "Unknown fec type");
      break;
  }

  return;
}
static void MT_Tuner_Format_Roll_Off_String(mt_unf_fe_roll_off_t roll_off, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }
  
  switch (roll_off)
  {
    case MT_UNF_FE_ROLL_OFF_0P35:
      strcpy(dest_str, "0.35");
      break;

    case MT_UNF_FE_ROLL_OFF_0P25:
      strcpy(dest_str, "0.25");
      break;

    case MT_UNF_FE_ROLL_OFF_0P20:
      strcpy(dest_str, "0.20");
      break;

    case MT_UNF_FE_ROLL_OFF_0P15:
      strcpy(dest_str, "0.15");
      break;

    case MT_UNF_FE_ROLL_OFF_0P10:
      strcpy(dest_str, "0.10");
      break;

    case MT_UNF_FE_ROLL_OFF_0P05:
      strcpy(dest_str, "0.05");
      break;

    default:
      strcpy(dest_str, "Unknown roll off");
      break;
  }

  return;
}
static void MT_Tuner_Format_FFT_Mode_String(mt_unf_fe_fft_t enFFTMode, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (enFFTMode)
  {
    case MT_UNF_FE_FFT_1K:
      strcpy(dest_str, "1K");
      break;

    case MT_UNF_FE_FFT_2K:
      strcpy(dest_str, "2K");
      break;

    case MT_UNF_FE_FFT_4K:
      strcpy(dest_str, "4K");
      break;

    case MT_UNF_FE_FFT_8K:
      strcpy(dest_str, "8K");
      break;

    case MT_UNF_FE_FFT_16K:
      strcpy(dest_str, "16K");
      break;

    case MT_UNF_FE_FFT_32K:
      strcpy(dest_str, "32K");
      break;

    case MT_UNF_FE_FFT_64K:
      strcpy(dest_str, "64K");
      break;

    case MT_UNF_FE_FFT_8E:
      strcpy(dest_str, "8E");
      break;

    case MT_UNF_FE_FFT_16E:
      strcpy(dest_str, "16E");
      break;

    case MT_UNF_FE_FFT_32E:
      strcpy(dest_str, "32E");
      break;

    default:
      strcpy(dest_str, "Unknown");
      break;
  }

  return;
}
static void MT_Tuner_Format_Guard_Interval_String(mt_unf_fe_guard_intv_t enGuardIntv, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (enGuardIntv)
  {
    case MT_UNF_FE_GUARD_INTV_1_128:
      strcpy(dest_str, "1/128");
      break;

    case MT_UNF_FE_GUARD_INTV_1_32:
      strcpy(dest_str, "1/32");
      break;

    case MT_UNF_FE_GUARD_INTV_1_16:
      strcpy(dest_str, "1/16");
      break;

    case MT_UNF_FE_GUARD_INTV_1_8:
      strcpy(dest_str, "1/8");
      break;

    case MT_UNF_FE_GUARD_INTV_1_4:
      strcpy(dest_str, "1/4");
      break;

    case MT_UNF_FE_GUARD_INTV_19_128:
      strcpy(dest_str, "19/128");
      break;

    case MT_UNF_FE_GUARD_INTV_19_256:
      strcpy(dest_str, "19/256");
      break;

    default:
      strcpy(dest_str, "Unknown");
      break;
  }

  return;
}
static void MT_Tuner_Format_TER_PP_String(mt_unf_fe_ter_pilot_pattern_t enPilotPattern, char *dest_str)
{
  if (dest_str == NULL)
  {
    MTADP_FRONT_SIGNAL_PRINT("\tDestination string error: NULL pointer\n");

    return;
  }

  switch (enPilotPattern)
  {
    case MT_UNF_FE_T2_PILOT_PATTERN_PP1:
      strcpy(dest_str, "PP1");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP2:
      strcpy(dest_str, "PP2");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP3:
      strcpy(dest_str, "PP3");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP4:
      strcpy(dest_str, "PP4");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP5:
      strcpy(dest_str, "PP5");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP6:
      strcpy(dest_str, "PP6");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP7:
      strcpy(dest_str, "PP7");
      break;

    case MT_UNF_FE_T2_PILOT_PATTERN_PP8:
      strcpy(dest_str, "PP8");
      break;

    default:
      strcpy(dest_str, "Undef PP");
      break;
  }

  return;
}
static void MT_Tuner_Print_Signal_Type(mt_unf_fe_sig_type_t sig_type)
{
  char sig_type_str[20];

  MT_Tuner_Format_Signal_Type_String(sig_type, sig_type_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Signal Type:" ,sig_type_str);

  return;
}
static void MT_Tuner_Print_FEC_Type(mt_unf_fe_fec_type_t fec_type)
{
  char fec_type_str[20];

  MT_Tuner_Format_FEC_Type_String(fec_type, fec_type_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n", "FEC Type:",fec_type_str);

  return;
}
static void MT_Tuner_Print_Modulation_Mode(mt_unf_modulation_type_t mod_mode)
{
  char modulation_mode_str[20];

  MT_Tuner_Format_Modulation_Mode_String(mod_mode, modulation_mode_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Modulation:", modulation_mode_str);

  return;
}
static void MT_Tuner_Print_Code_Rate(mt_unf_fe_fecrate_t enFECRate)
{
  char fec_str[20];

  MT_Tuner_Format_Code_Rate_String(enFECRate, fec_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Code Rate:",fec_str);

  return;
}
static void MT_Tuner_Print_Roll_Off(mt_unf_fe_roll_off_t enRollOff)
{
  char rolloff_str[20];

  MT_Tuner_Format_Roll_Off_String(enRollOff, rolloff_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Roll Off:",rolloff_str);

  return;
}
static void MT_Tuner_Print_FFT_Mode(mt_unf_fe_fft_t enFFTMode)
{
  char fft_mode_str[20];

  MT_Tuner_Format_FFT_Mode_String(enFFTMode, fft_mode_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","FFT Mode:" ,fft_mode_str);

  return;
}
static void MT_Tuner_Print_Guard_Interval(mt_unf_fe_guard_intv_t enGuardIntv)
{
  char guard_interval_str[20];

  MT_Tuner_Format_Guard_Interval_String(enGuardIntv, guard_interval_str);

  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Guard Interval:" ,guard_interval_str);

  return;
}
/*
@brief Initialize and open the tuner
@param[in] tuner_id, tuner port number
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MTADP_Fe_Init(mt_u32 tuner_id)
{
    MT_S32 ret = 0;

    ret = mt_unf_fe_init();
    if (MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT(" mt_unf_fe_open failed.ret = 0x%x\n",ret);
        return ret;
    }

    /* open Tuner*/
    MTADP_FRONT_INFO_PRINT("tuner id is 0x%x\n",tuner_id);
    ret = mt_unf_fe_open(tuner_id);
    if (MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT(" mt_unf_fe_open failed.ret = 0x%x\n",ret);
        mt_unf_fe_deinit();
        return ret;
    }
    return ret;
}

/*
@brief Uninitialize and close the tuner
@param[in] tuner_id, tuner port number
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MTADP_Fe_DeInit(mt_u32 tuner_id)
{
    MT_S32 ret;

    ret = mt_unf_fe_close(tuner_id);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("fe close failure\n");
        return ret;
    }

    ret = mt_unf_fe_deinit();
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("fe deinit failure\n");
        return ret;
    }
    return SUCCESS;
}

/*!
@brief Tuner Connect
@param[in]  tuner_id            Tuner ID
@param[in]  freq                Tuner frequency
@param[in]  sym_rate            Tuner symbol rate
@param[in]  third_param         QAM modulation mode
@return::0
@*/
mt_s32 MTADP_Fe_Connect_Dvbc(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 qam)
{
    mt_s32 ret = MT_FAILURE;
    mt_sys_version_s  stSysChipInfo = { 0 };

    /** Get chip information */
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_sys_get_version failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("chipVersion = %#x. \n", stSysChipInfo.enChipVersion);

    if(MT_CHIP_SYMPHONY2_A0 > stSysChipInfo.enChipVersion)
    {

        /** Set the Tuner parameter */
        ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_CAB, MT_UNF_TUNER_TYPE_M88TC3800, 0xc2, MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB, 0x38, 1, 0);

    }
    else if(MT_CHIP_SYMPHONY4_A1 >= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY4_A0 <= stSysChipInfo.enChipVersion)
    {

        /** Set the Tuner parameter */
        ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_CAB, MT_UNF_TUNER_TYPE_M88TC6800, 0xc6, MT_UNF_DEMOD_DEV_TYPE_M88CS8800, 0x38, 4, 0);

    }
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    else if(MT_CHIP_SYMPHONY6_MAX >= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY6_A0 <= stSysChipInfo.enChipVersion)
    {

        /** Set the Tuner parameter */
        ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_CAB, MT_UNF_TUNER_TYPE_M88TC6800, 0xc6, MT_UNF_DEMOD_DEV_TYPE_M88CS8800, 0x38, 1, 0);

    }
#endif
    else
    {

        /** Set the Tuner parameter */
        ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_CAB, MT_UNF_TUNER_TYPE_M88TC6800, 0xc6, MT_UNF_DEMOD_DEV_TYPE_M88CT8K, 0x18, 1, 0);

    }

    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("MT_Tuner_Opera_Set_Parameter failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("Tuner setting parameters is complete!. ret = 0x%x\n",  ret);


    return MT_Tuner_Lock_Dvbc(tuner_id, freq, sym_rate, qam, 0, 1000);
}


/*
@brief connect dvbs
@param[in] tuner_id, tuner port number
@param[in] freq, frequency
@param[in] sym_rate, Rate of transmission
@param[in] onoff_22k, 22k
@param[in] polar, Mode of polarization
@param[in] port_type, Type of signal
@return MT_SUCCESS
@return MT_FAILURE
*/
mt_s32 MTADP_Fe_Connect_Dvbs(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 onoff_22k, mt_u32 polar, mt_u32 port_type)
{
    MT_S32                 ret = 0;
    mt_sys_version_s       stSysChipInfo = { 0 };
    mt_unf_fe_lnb_config_t lnb_config = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));

   /** Get chip information */
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_sys_get_version failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("chipVersion = %#x. \n", stSysChipInfo.enChipVersion);

    if(MT_CHIP_SYMPHONY4_A1 >= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY4_A0 <= stSysChipInfo.enChipVersion)
    {
        /* tuner_id, Port of the tuner
           sig_type, Type of received signal
           tuner_dev_type, tuner Device type
           tuner_addr, The address of tuner
           demod_dev_type, Type of the demod device
           demod_addr, The address of demod
           out_put_mode, Output mode
           I2c_channel, i2c Channel mode*/
        if(tuner_id == 0)
        {
#ifdef MT_SYM4_DSS
            ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_DVBS_AUTO, MT_UNF_TUNER_TYPE_M88TS6011, 0x58, MT_UNF_DEMOD_DEV_TYPE_M88DS6113, 0xD2, 1, 1);
#else
            ret = MT_Tuner_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 1, 0);
#endif

        }
        else if(tuner_id == 1)
        {
            ret = MT_Tuner_Set_Parameter(tuner_id, 2048, 80, 90, 336, 210, 4, 0);
        }
        if(MT_SUCCESS != ret)
        {
            MTADP_FRONT_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        if(onoff_22k)
        {
            ret = mt_unf_fe_set_lnb_power(tuner_id, MT_UNF_FE_LNB_POWER_ON);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_power err!\n");
                return ret;
            }
            memset(&lnb_config,0,sizeof(mt_unf_fe_lnb_config_t));
            /* Low Local Oscillator Frequency, MHz */
            lnb_config.low_lo = 5150;
            /* High Local Oscillator Frequency, MHz*/
            lnb_config.high_lo = 5150;

            ret = mt_unf_fe_set_lnb_config(tuner_id, &lnb_config);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_config err!\n");
                return ret;
            }
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
        if(tuner_id == 0)
        {
           ret = MT_Tuner_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 4, 0);
        }
        else if(tuner_id == 1)
        {
            ret = MT_Tuner_Set_Parameter(tuner_id, 2048, 80, 90, 336, 210, 4, 1);
        }
        else
        {
            ret = MT_Tuner_Set_Parameter(tuner_id, 2048, 34, 88, 288, 24, 4, 0);
        }


        if(MT_SUCCESS != ret)
        {
            MTADP_FRONT_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        if(onoff_22k)
        {
            ret = mt_unf_fe_set_lnb_power(tuner_id, MT_UNF_FE_LNB_POWER_ON);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_power err!\n");
                return ret;
            }
            memset(&lnb_config,0,sizeof(mt_unf_fe_lnb_config_t));
            /* Low Local Oscillator Frequency, MHz */
            lnb_config.low_lo  = 5150;
            /* High Local Oscillator Frequency, MHz*/
            lnb_config.high_lo = 5150;

            ret = mt_unf_fe_set_lnb_config(tuner_id, &lnb_config);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_config err!\n");
                return ret;
            }
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
        ret = MT_Tuner_Set_Parameter(tuner_id, 2048, 34, 88, 261, 208, 1, 0);
        if(MT_SUCCESS != ret)
        {
            MTADP_FRONT_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        if(onoff_22k)
        {
            ret = mt_unf_fe_set_lnb_power(tuner_id, MT_UNF_FE_LNB_POWER_ON);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_power err!\n");
                return ret;
            }
            memset(&lnb_config,0,sizeof(mt_unf_fe_lnb_config_t));
            /* Low Local Oscillator Frequency, MHz */
            lnb_config.low_lo  = 5150;
            /* High Local Oscillator Frequency, MHz*/
            lnb_config.high_lo = 5150;

            ret = mt_unf_fe_set_lnb_config(tuner_id, &lnb_config);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_config err!\n");
                return ret;
            }
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
        ret = MT_Tuner_Set_Parameter(tuner_id,2048,34,88,280,24,1,0);
        if(MT_SUCCESS != ret)
        {
            MTADP_FRONT_ERR_PRINT("MT_Tuner_Set_Parameter err!\n");
        }
        if(onoff_22k)
        {
            ret = mt_unf_fe_set_lnb_power(tuner_id, MT_UNF_FE_LNB_POWER_ON);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_power err!\n");
                return ret;
            }
            memset(&lnb_config,0,sizeof(mt_unf_fe_lnb_config_t));
            /* Low Local Oscillator Frequency, MHz */
            lnb_config.low_lo = 5150;
            /* High Local Oscillator Frequency, MHz*/
            lnb_config.high_lo = 5150;

            ret = mt_unf_fe_set_lnb_config(tuner_id, &lnb_config);
            if(MT_SUCCESS != ret)
            {
                MTADP_FRONT_ERR_PRINT("mt_unf_fe_set_lnb_config err!\n");
                return ret;
            }
        }
    }

    return MT_Tuner_Lock_Dvbs(tuner_id, freq, sym_rate,onoff_22k, polar, port_type, 1000);
}



/*****************************************************************************
mt_tuner -o .setpara 0 1024 6 198 280 24 1 0
mt_tuner -o .tlock 0 1024 858 8 0 2 2500
*****************************************************************************/
/*****************************************************************************
*brief DVBT/DVBT2 frequency locking
*param[in] tuner_id, frequency locking id
*param[in] freq, The frequency of the input required
*param[in] band_width   The required input signal bandwidth
*return ::0  success.
*****************************************************************************/
mt_s32 MTADP_Fe_Connect_Dvbt(mt_u32 tuner_id, mt_u32 freq, mt_u32 band_width)
{

    mt_s32 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };
    mt_u32 timeout = 0;

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));

    /** Get chip information */
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_sys_get_version failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("chipVersion = %#x.\n", stSysChipInfo.enChipVersion);
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    MT_Tuner_Set_Parameter(tuner_id, 1024, 6, 198, 384, 200, 4, 1);
    //MT_Tuner_Set_Parameter(tuner_id, 1024, 6, 198, 400, 200, 4, 1);/*Sony cxd family chipset include cxd2856 cxd2878*/
    timeout = 0;
#else
    MT_Tuner_Set_Parameter(tuner_id, 1024, 6, 198, 280, 24, 1, 0);
    timeout = 2500;
#endif

    /* 0::pid */
    /* 2::Signal type (automatic) */
    /* 2500::timeout */

    return MT_Tuner_Lock_Dvbt(tuner_id, freq, band_width, 0, MT_UNF_PORT_TYPE_DVBT_AUTO, timeout);;
}


/*****************************************************************************
mt_tuner -o .setpara 0 1024 6 198 280 24 1 0
mt_tuner -o .tlock 0 1024 858 8 0 2 2500
*****************************************************************************/
/*****************************************************************************
*brief DVBT/DVBT2 frequency locking
*param[in] mtadp_dvbt_info_t *p_dvbt_info

*return ::0  success.
*****************************************************************************/
mt_s32 MTADP_Fe_Connect_Dvbt_Certainty_Signal(mtadp_dvbt_info_t *p_dvbt_info)
{

    mt_s32 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

	if(!p_dvbt_info)
	{
        MTADP_FRONT_ERR_PRINT("p_dvbt_info is NULL failed. \n");
        return MT_FAILURE;
	}
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));

    /** Get chip information */
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_sys_get_version failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("chipVersion = %#x.\n", stSysChipInfo.enChipVersion);
	if(p_dvbt_info->noneed_setpara_flag == 0)
	{
#ifdef CONFIG_MT_CHIP_SYMPHONY6
    	MT_Tuner_Set_Parameter(p_dvbt_info->tunerId, 1024, 6, 198, 384, 200, 4, 1);
		//MT_Tuner_Set_Parameter(p_dvbt_info->tunerId, 1024, 6, 198, 400, 200, 4, 1);/*Sony cxd family chipset include cxd2856 cxd2878*/
#else
    	MT_Tuner_Set_Parameter(p_dvbt_info->tunerId, 1024, 6, 198, 280, 24, 1, 0);
#endif
	}

    /* 0::pid */
    /* 2::Signal type (automatic) */
    /* 2500::timeout */

    return MT_Tuner_Lock_Dvbt_Certainty_Signal(p_dvbt_info);
}
/*!
@brief Tuner Connect. CNcomment:
@param[in]  tuner_id            Tuner ID
@param[in]  freq                Tuner frequency
@param[in]  sym_rate            Tuner symbol rate
@param[in]  tuner_qam           QAM modulation mode
@return::0
@*/
mt_s32 MTADP_Fe_Connect_J83b(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 qam)
{
    mt_s32 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));

    /** Get chip information */
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        MTADP_FRONT_ERR_PRINT("mt_sys_get_version failed. ret = 0x%x\n",  ret);
        return ret;
    }

    MTADP_FRONT_INFO_PRINT("chipVersion = %#x. ret = 0x%x\n", stSysChipInfo.enChipVersion,  ret);

    if(MT_CHIP_SYMPHONY2_A0 > stSysChipInfo.enChipVersion)
    {

        ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_J83B, MT_UNF_TUNER_TYPE_M88TC6800, 0xc2, MT_UNF_DEMOD_DEV_TYPE_M88CS8000_CAB, 0x38, 1, 0);
        if(MT_SUCCESS != ret)
        {
            MTADP_FRONT_ERR_PRINT("MT_Tuner_Opera_Set_Parameter fail \n");
            return ret;
        }

    }
    else if(MT_CHIP_SYMPHONY4_A1 >= stSysChipInfo.enChipVersion && MT_CHIP_SYMPHONY4_A0 <= stSysChipInfo.enChipVersion)
    {

        ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_J83B, MT_UNF_TUNER_TYPE_M88TC6800, 0xc6, 288, 0x38, 4, 0);  //sym4
        if(MT_SUCCESS != ret)
        {
            MTADP_FRONT_ERR_PRINT("MT_Tuner_Opera_Set_Parameter fail \n");
            return ret;
        }


    }
    else
    {

         ret = MT_Tuner_Set_Parameter(tuner_id, MT_UNF_FE_SIG_TYPE_J83B, MT_UNF_TUNER_TYPE_M88TC6800, 0xc6, MT_UNF_DEMOD_DEV_TYPE_M88CS8800, 0x38, 1, 0);     //sym4
         if(MT_SUCCESS != ret)
         {
             MTADP_FRONT_ERR_PRINT("MT_Tuner_Opera_Set_Parameter fail \n");
             return ret;
         }

    }


    return MT_Tuner_Lock_J83b(tuner_id, freq, sym_rate, qam, 0, 1000);
}

/*!
@brief Tuner Connect. CNcomment:
@param[in]  tuner_id            Tuner ID
@return::0
@*/
mt_s32 MTADP_Fe_Get_Signal_Info(mt_u32 tuner_id)
{
	mt_unf_fe_status_t       stTunerStatus = {0};
	mt_unf_fe_signal_info_t  stTunerSignalInfo = {0};
	mt_u32 mtGetBer[3], mtPreBer[3];
    mt_u32 strength = 0;
    mt_s32 agc = 0;
    mt_u32 quality = 0;
    mt_s32 accurate_snr = 0;
    mt_u32 snr = 0;
	MT_S32 ret = 0;

	ret |= mt_unf_fe_get_status(tuner_id, &stTunerStatus);
	ret |= mt_unf_fe_get_signal_strength(tuner_id, &strength);
    ret |= mt_unf_fe_get_agc(tuner_id, 0, &agc);
    MTADP_FRONT_SIGNAL_PRINT("*===============================================================\n");
	if (MT_UNF_FE_SIGNAL_LOCKED == stTunerStatus.lock_status)
	{
		ret |= mt_unf_fe_get_signal_quality(tuner_id, &quality);
		ret |= mt_unf_fe_get_snr(tuner_id, &snr);
        ret |= mt_unf_fe_get_accurate_snr(tuner_id, &accurate_snr);
		ret |= mt_unf_fe_get_ber(tuner_id, mtGetBer);
    	ret |= mt_unf_fe_get_pre_ber(tuner_id, mtPreBer);
		ret |= mt_unf_fe_get_signal_info(tuner_id, &stTunerSignalInfo);
		if(ret != MT_SUCCESS)
		{
			MTADP_FRONT_SIGNAL_PRINT("MTADP_Fe_Get_Signal_Info Failed,ret:0x%x\n",ret);
			return ret;
		}
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Signal Status:",(stTunerStatus.lock_status == MT_UNF_FE_SIGNAL_LOCKED) ? "SIGNAL_LOCKED" : "SIGNAL_DROPPED");
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %d%\n","Signal Strength:",strength);
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %02d.%02d\n","Signal Agc:",agc/100,agc%100);
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %d%\n","Signal Quality:",quality);
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %d\n","Signal SNR:",snr);
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %02d.%03d\n","Accurate SNR:",accurate_snr/1000, accurate_snr%1000);
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %ld.%03dE-%d\n","Post BER:", mtGetBer[0], mtGetBer[1], mtGetBer[2]);
		MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %ld.%03dE-%d\n","Pre  BER:", mtPreBer[0], mtPreBer[1], mtPreBer[2]);
		MTADP_FRONT_SIGNAL_PRINT("*===============================================================\n");
		MT_Tuner_Print_Signal_Type(stTunerSignalInfo.sig_type);
		switch (stTunerSignalInfo.sig_type)
		{
          case MT_UNF_FE_SIG_TYPE_CAB:
          case MT_UNF_FE_SIG_TYPE_J83B:
		  	MT_Tuner_Print_FEC_Type(stTunerSignalInfo.sig_info.cab.cab_type);
			MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %6d KHz\n","Frequency:" ,stTunerSignalInfo.sig_info.cab.freq);
            MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %4d KSs\n","Symbol Rate:" ,stTunerSignalInfo.sig_info.cab.symbol_rate);
            MT_Tuner_Print_Modulation_Mode(stTunerSignalInfo.sig_info.cab.mode_type);
		  	break;
		  case MT_UNF_FE_SIG_TYPE_SAT:
          case MT_UNF_FE_SIG_TYPE_SAT_2:
          case MT_UNF_FE_SIG_TYPE_DVBS_AUTO:
		  	MT_Tuner_Print_FEC_Type(stTunerSignalInfo.sig_info.sat.sat_type);
            MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %7d KHz\n","Frequency:" ,stTunerSignalInfo.sig_info.sat.freq);
            MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %5d KSs\n","Symbol Rate:", stTunerSignalInfo.sig_info.sat.symbol_rate);

            MT_Tuner_Print_Modulation_Mode(stTunerSignalInfo.sig_info.sat.mode_type);

            MT_Tuner_Print_Code_Rate(stTunerSignalInfo.sig_info.sat.fec_rate);
			
			MT_Tuner_Print_Roll_Off(stTunerSignalInfo.sig_info.sat.roll_off);

            if (stTunerSignalInfo.sig_type == MT_UNF_FE_SIG_TYPE_SAT_2)
            {
              MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Pilot Mode:" ,(stTunerSignalInfo.sig_info.sat.pilot_mode == 1) ? "On" : "Off");
            }
			
		  	break;
          case MT_UNF_FE_SIG_TYPE_DVB_T:
          case MT_UNF_FE_SIG_TYPE_DVB_T2:
          case MT_UNF_FE_SIG_TYPE_DVBT_AUTO:
            MT_Tuner_Print_FEC_Type(stTunerSignalInfo.sig_info.ter.ter_type);

            MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %6d KHz\n","Frequency:" ,stTunerSignalInfo.sig_info.ter.freq);
            MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %4d\n","Bandwidth:" ,stTunerSignalInfo.sig_info.ter.band_width);

            MT_Tuner_Print_Modulation_Mode(stTunerSignalInfo.sig_info.ter.enModType);

            MT_Tuner_Print_FFT_Mode(stTunerSignalInfo.sig_info.ter.enFFTMode);

            MT_Tuner_Print_Guard_Interval(stTunerSignalInfo.sig_info.ter.enGuardIntv);

            MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t 0x%04x\n","Cell ID:" ,stTunerSignalInfo.sig_info.ter.cell_id);

            if (stTunerSignalInfo.sig_type == MT_UNF_FE_SIG_TYPE_DVB_T2)
            {
              char t2_pp_str[20];

              MT_Tuner_Print_Code_Rate(stTunerSignalInfo.sig_info.ter.enFECRate);

              MT_Tuner_Format_TER_PP_String(stTunerSignalInfo.sig_info.ter.enPilotPattern, t2_pp_str);

              MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","Pilot Pattern:",t2_pp_str);

              switch (stTunerSignalInfo.sig_info.ter.enPLPType)
              {
                case MT_UNF_FE_T2_PLP_TYPE_COM:
                  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","PLP Type:","Common");
                  break;

                case MT_UNF_FE_T2_PLP_TYPE_DAT1:
                  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","PLP Type:","Data 1");
                  break;

                case MT_UNF_FE_T2_PLP_TYPE_DAT2:
                  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","PLP Type:","Data 2");
                  break;

                default:
                  MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","PLP Type:","Unknown");
                  break;
              }
            }
            else if (stTunerSignalInfo.sig_type == MT_UNF_FE_SIG_TYPE_DVB_T)
            {
              char t_hp_fec_str[20], t_lp_fec_str[20];

              MT_Tuner_Format_Code_Rate_String(stTunerSignalInfo.sig_info.ter.enFECRate, t_hp_fec_str);

              MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","HP Code Rate:",t_hp_fec_str);

              MT_Tuner_Format_Code_Rate_String(stTunerSignalInfo.sig_info.ter.enLowPriFECRate, t_lp_fec_str);

              MTADP_FRONT_SIGNAL_PRINT("*\t %-20s\t %s\n","LP Code Rate:",t_lp_fec_str);
            }
            else //if (signal_info.sig_type == MT_UNF_FE_SIG_TYPE_DVBT_AUTO)
            {
              MT_Tuner_Print_Code_Rate(stTunerSignalInfo.sig_info.ter.enFECRate);
            }  	
		  	break;
		  default:
		  	break;
		}
		
	}
	else
	{
		MTADP_FRONT_SIGNAL_PRINT("*\t Signal Status:  \t %s \n",(stTunerStatus.lock_status == MT_UNF_FE_SIGNAL_LOCKED) ? "SIGNAL_LOCKED" : "SIGNAL_DROPPED");
		MTADP_FRONT_SIGNAL_PRINT("*\t Signal Strength:\t %d(%) \n",strength);
		MTADP_FRONT_SIGNAL_PRINT("*\t Signal Agc:     \t %02d.%02d \n",agc/100,agc%100);
	}
	MTADP_FRONT_SIGNAL_PRINT("*===============================================================\n");
	return ret;
	
}
