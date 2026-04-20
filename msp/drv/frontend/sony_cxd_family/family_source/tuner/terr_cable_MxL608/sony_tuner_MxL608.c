/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************/
/* Montage-LZ Group                                                          */
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                      */
/* Copyright (c) 2021 Montage-LZ Group Limited. All Rights Reserved.         */
/*****************************************************************************/

/*------------------------------------------------------------------------------
  Copyright 2016 Sony Semiconductor Solutions Corporation

  Last Updated    : 2016/08/01
  Modification ID : b30d76210d343216ea52b88e9b450c8fd5c0359f
------------------------------------------------------------------------------*/
#include "sony_tuner_MxL608.h"

/*------------------------------------------------------------------------------
 Driver Version
------------------------------------------------------------------------------*/
const char* sony_tuner_MxL608_version =  MxLWare608DrvVersion;

/*------------------------------------------------------------------------------
 Static Function Prototypes
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_MxL608_Initialize (sony_tuner_t * pTuner);

static sony_result_t sony_tuner_MxL608_Tune (sony_tuner_t * pTuner,
                                             uint32_t frequency,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth);

static sony_result_t sony_tuner_MxL608_Sleep (sony_tuner_t * pTuner);

static sony_result_t sony_tuner_MxL608_Shutdown (sony_tuner_t * pTuner);

static sony_result_t sony_tuner_MxL608_ReadRFLevel (sony_tuner_t * pTuner, int32_t * pRFLevel);

/*------------------------------------------------------------------------------
 Functions
------------------------------------------------------------------------------*/
sony_result_t sony_tuner_MxL608_Create (sony_tuner_t * pTuner,
                                        MXL608_XTAL_FREQ_E xtalFreq,
                                        uint8_t i2cAddress,
                                        sony_i2c_t * pI2c,
                                        uint32_t configFlags,
                                        sony_MxL608_t * pAscot3Tuner)
{
    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_MxL608_Create");

    if ((!pI2c) || (!pAscot3Tuner) || (!pTuner)) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Create the underlying Ascot3 reference driver. */
    //result = sony_ascot3_Create (pAscot3Tuner, xtalFreq, i2cAddress, pI2c, configFlags);
    //if (result != SONY_RESULT_OK) {
    //    SONY_TRACE_RETURN (result);
    //}
    

    /* Create local copy of instance data. */
    pTuner->Initialize = sony_tuner_MxL608_Initialize;
    pTuner->TerrCableTune = sony_tuner_MxL608_Tune;
	pTuner->SatTune = NULL;
    pTuner->Sleep = sony_tuner_MxL608_Sleep;
    pTuner->Shutdown = sony_tuner_MxL608_Shutdown;
    pTuner->ReadRFLevel = sony_tuner_MxL608_ReadRFLevel;
    pTuner->CalcRFLevelFromAGC = NULL;
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->i2cAddress = i2cAddress;
    pTuner->pI2c = pI2c;
    pTuner->flags = configFlags;
    pTuner->user = pAscot3Tuner;

    SONY_TRACE_RETURN (result);
}

sony_result_t sony_tuner_MxL608_ReadRssi (sony_tuner_t * pTuner, int32_t * pRssi)
{
    sony_result_t result = SONY_RESULT_OK;
	int32_t level = 0;
	
    SONY_TRACE_ENTER ("sony_tuner_MxL608_ReadRssi");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

	sony_tuner_MxL608_ReadRFLevel(pTuner, &level);
	*pRssi = level + 107;

	if (*pRssi > 100)	*pRssi = 100;
	if (*pRssi < 0)		*pRssi = 0;

    //result = sony_ascot3_ReadRssi (((sony_ascot3_t *) pTuner->user), pRssi);

    SONY_TRACE_RETURN (result);
}

/*------------------------------------------------------------------------------
 Static Functions
------------------------------------------------------------------------------*/
static sony_result_t sony_tuner_MxL608_Initialize (sony_tuner_t * pTuner)
{
	MXL_STATUS status;
	UINT8 devId;
	MXL_BOOL singleSupply_3_3V;
	MXL608_XTAL_SET_CFG_T xtalCfg;
	MXL608_IF_OUT_CFG_T ifOutCfg;
	MXL608_AGC_CFG_T agcCfg;
	MXL608_TUNER_MODE_CFG_T tunerModeCfg;
	MXL608_VER_INFO_T tunerVer;

    sony_result_t result = SONY_RESULT_OK;
    SONY_TRACE_ENTER ("sony_tuner_MxL608_Initialize");

    if (!pTuner || !pTuner->user)
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

    /* Call into underlying driver. */

	/* If OEM data is not required, customer should treat devId as
	 I2C slave Address */
	//devId = 0xC0;//MXL608_I2C_ADDR;
	devId = pTuner->i2cAddress;

	//MXL608_Address = handle->m_device_ctt2.tuner_cfg.tuner_dev_addr;
	//Step 1 : Soft Reset MxL603
	status = MxLWare608_API_CfgDevSoftReset(devId);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
	}

	status = MxLWare608_API_ReqDevVersionInfo(devId, &tunerVer);

	//Step 2 : Overwrite Default
	singleSupply_3_3V = MXL_ENABLE;//MXL_DISABLE;
	status = MxLWare608_API_CfgDevOverwriteDefaults(devId, singleSupply_3_3V);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
	}

	//Step 3 : XTAL Setting
	xtalCfg.xtalFreqSel = MXL608_XTAL_16MHz;//MXL608_XTAL_24MHz;
	xtalCfg.xtalCap = 20;//16;//20;//12;//16 for 24 mhz,20 for 16mhz
	xtalCfg.clkOutEnable = MXL_DISABLE;
	xtalCfg.clkOutDiv = MXL_DISABLE;
	xtalCfg.clkOutExt = MXL_DISABLE;
	xtalCfg.singleSupply_3_3V = MXL_ENABLE;
	xtalCfg.XtalSharingMode = MXL_DISABLE;
	status = MxLWare608_API_CfgDevXtal(devId, xtalCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		// printf("Error! MxLWare608_API_CfgDevXtal\n");
	}

	//Step 4 : IF Out setting
	switch (pTuner->system)
	{
		case SONY_DTV_SYSTEM_DVBC:
		case SONY_DTV_SYSTEM_DVBC2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_DVBT:
		case SONY_DTV_SYSTEM_DVBT2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_ISDBC:
		case SONY_DTV_SYSTEM_ISDBT:
			tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_J83B:
			tunerModeCfg.signalMode = MXL608_DIG_J83B;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		default:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
			break;
	}

	ifOutCfg.ifOutFreq = MXL608_IF_5MHz;

	ifOutCfg.ifInversion = MXL_ENABLE;//MXL_DISABLE;
	ifOutCfg.gainLevel = 11;
	ifOutCfg.manualFreqSet = MXL_DISABLE;
	ifOutCfg.manualIFOutFreqInKHz = 5000;//0;
	status = MxLWare608_API_CfgTunerIFOutParam(devId, ifOutCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		//printf("Error! MxLWare608_API_CfgTunerIFOutParam\n");
	}

	//Step 5 : AGC Setting
	agcCfg.agcType = MXL608_AGC_EXTERNAL;
	agcCfg.setPoint = 66;
	agcCfg.agcPolarityInverstion = MXL_DISABLE;// MXL_ENABLE
	status = MxLWare608_API_CfgTunerAGC(devId, agcCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		//printf("Error! MxLWare608_API_CfgTunerAGC\n");
	}

	//Step 6 : Application Mode setting
#if 0	// Same as step 4
	switch (pTuner->system)
	{
		case SONY_DTV_SYSTEM_DVBC:
		case SONY_DTV_SYSTEM_DVBC2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_DVBT:
		case SONY_DTV_SYSTEM_DVBT2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_ISDBC:
		case SONY_DTV_SYSTEM_ISDBT:
			tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_J83B:
			tunerModeCfg.signalMode = MXL608_DIG_J83B;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		default:
			return MtFeErr_Fail;
			break;
	}
#endif

	tunerModeCfg.xtalFreqSel = MXL608_XTAL_16MHz;//MXL608_XTAL_24MHz;
	tunerModeCfg.ifOutGainLevel = 11;
	status = MxLWare608_API_CfgTunerMode(devId, tunerModeCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		//printf("Error! MxLWare608_API_CfgTunerMode\n");
	}

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_MxL608_Tune (sony_tuner_t * pTuner,
                                             uint32_t frequencyKHz,
                                             sony_dtv_system_t system,
                                             sony_dtv_bandwidth_t bandwidth)
{
    sony_result_t result = SONY_RESULT_OK;

	MXL_STATUS status;
	MXL608_TUNER_MODE_CFG_T tunerModeCfg;
	MXL608_CHAN_TUNE_CFG_T chanTuneCfg;
	MXL608_IF_OUT_CFG_T ifOutCfg;
	MXL608_AGC_CFG_T agcCfg;

	UINT8 devId;

    SONY_TRACE_ENTER ("sony_tuner_ascot3_Tune");

    if (!pTuner || !pTuner->user)
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

	devId = pTuner->i2cAddress;

	switch (system)
	{
		case SONY_DTV_SYSTEM_DVBC:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_6MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3700;//5000;//4570;// 4100;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_7MHz;
					//tunerModeCfg.ifOutFreqinKHz = 4900;//5000;//4570;// 4100;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_8MHz;
					//tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
					break;
			}
			break;

		case SONY_DTV_SYSTEM_DVBC2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_6MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3700;//5000;//4570;// 4100;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_8MHz;
					//tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
					break;
			}
			break;

		case SONY_DTV_SYSTEM_DVBT:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_5_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_5MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3600;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3600;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_7MHz;
					//tunerModeCfg.ifOutFreqinKHz = 4200;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
					//tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
					break;
			}
			break;

		case SONY_DTV_SYSTEM_DVBT2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_1_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3500;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_5_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_5MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3600;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
					//tunerModeCfg.ifOutFreqinKHz = 3600;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_7MHz;
					//tunerModeCfg.ifOutFreqinKHz = 4200;//150;//4570;// 4100;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
					//tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
					break;
			}
			break;

		case SONY_DTV_SYSTEM_ISDBC:
			tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_6MHz;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_7MHz;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_8MHz;
					break;
			}
			break;

		case SONY_DTV_SYSTEM_ISDBT:
			tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_7MHz;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
					break;
			}
			break;

		case SONY_DTV_SYSTEM_J83B:
			tunerModeCfg.signalMode = MXL608_DIG_J83B;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_6MHz;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_7MHz;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_CABLE_BW_8MHz;
					break;
			}
			break;

		default:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;

			switch (bandwidth)
			{
				case SONY_DTV_BW_6_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_6MHz;
					break;

				case SONY_DTV_BW_7_MHZ:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_7MHz;
					break;

				case SONY_DTV_BW_8_MHZ:
				default:
					chanTuneCfg.bandWidth = MXL608_TERR_BW_8MHz;
					break;
			}
			break;
	}

	//Step 4 : IF Out setting
#if 0
	switch (pTuner->system)
	{
		case SONY_DTV_SYSTEM_DVBC:
		case SONY_DTV_SYSTEM_DVBC2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_DVBT:
		case SONY_DTV_SYSTEM_DVBT2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_ISDBC:
		case SONY_DTV_SYSTEM_ISDBT:
			tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_J83B:
			tunerModeCfg.signalMode = MXL608_DIG_J83B;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		default:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
			break;
	}
#endif

	ifOutCfg.ifOutFreq = MXL608_IF_5MHz;
	ifOutCfg.ifInversion = MXL_DISABLE;
	ifOutCfg.gainLevel = 11;
	ifOutCfg.manualFreqSet = MXL_DISABLE;
	ifOutCfg.manualIFOutFreqInKHz = 0;
	status = MxLWare608_API_CfgTunerIFOutParam(devId, ifOutCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		//printf("Error! MxLWare608_API_CfgTunerIFOutParam\n");
	}

	//Step 5 : AGC Setting
	agcCfg.agcType = MXL608_AGC_EXTERNAL;
	agcCfg.setPoint = 66;
	agcCfg.agcPolarityInverstion = MXL_DISABLE;
	status = MxLWare608_API_CfgTunerAGC(devId, agcCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		//printf("Error! MxLWare608_API_CfgTunerAGC\n");
	}

	//Step 6 : Application Mode setting
#if 0	// Same as step 4
	switch (pTuner->system)
	{
		case SONY_DTV_SYSTEM_DVBC:
		case SONY_DTV_SYSTEM_DVBC2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_C;
			tunerModeCfg.ifOutFreqinKHz = 5000;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_DVBT:
		case SONY_DTV_SYSTEM_DVBT2:
			tunerModeCfg.signalMode = MXL608_DIG_DVB_T_DTMB;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_ISDBC:
		case SONY_DTV_SYSTEM_ISDBT:
			tunerModeCfg.signalMode = MXL608_DIG_ISDBT_ATSC;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		case SONY_DTV_SYSTEM_J83B:
			tunerModeCfg.signalMode = MXL608_DIG_J83B;
			tunerModeCfg.ifOutFreqinKHz = 5000;//150;//4570;// 4100;
			break;

		default:
			return MtFeErr_Fail;
			break;
	}
#endif

	tunerModeCfg.xtalFreqSel = MXL608_XTAL_16MHz;//MXL608_XTAL_24MHz;
	tunerModeCfg.ifOutGainLevel = 11;
	status = MxLWare608_API_CfgTunerMode(devId, tunerModeCfg);
	if (status != MXL_SUCCESS)
	{
		return SONY_RESULT_ERROR_HW_STATE;
		//printf("Error! MxLWare608_API_CfgTunerMode\n");
	}

	//Step 7 : Channel frequency & bandwidth setting

	chanTuneCfg.freqInHz = frequencyKHz * 1000;//666000000;


	chanTuneCfg.xtalFreqSel = MXL608_XTAL_16MHz;//MXL608_XTAL_24MHz;
	chanTuneCfg.startTune = MXL_START_TUNE;
	chanTuneCfg.signalMode = tunerModeCfg.signalMode;
	status = MxLWare608_API_CfgTunerChanTune(devId, chanTuneCfg);

    /* Assign current values. */
    pTuner->system = system;
    //pTuner->frequencyKHz = ((sony_ascot3_t *) pTuner->user)->frequencykHz;
    pTuner->frequencyKHz = frequencyKHz;
    pTuner->bandwidth = bandwidth;


	if (status != MXL_SUCCESS)
	{
		return  SONY_RESULT_ERROR_HW_STATE;
		// printf("Error! MxLWare608_API_CfgTunerChanTune\n");
	}
	else
	{
		// Wait 15 ms
		MxLWare608_OEM_Sleep(15);	// 15

		return SONY_RESULT_OK;
	}

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_MxL608_Sleep (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
	MXL_STATUS status;
	UINT8 devId;// = pTuner->i2cAddress;

    SONY_TRACE_ENTER ("sony_tuner_ascot3_Sleep");

    if (!pTuner || !pTuner->user)
	{
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

	devId = pTuner->i2cAddress;

    /* Call into underlying driver. */

	status = MxLWare608_API_CfgDevPowerMode(devId, MXL608_PWR_MODE_STANDBY, MXL_DISABLE, 0);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_MxL608_Shutdown (sony_tuner_t * pTuner)
{
    sony_result_t result = SONY_RESULT_OK;
	MXL_STATUS status;
	uint8_t devId;// = pTuner->i2cAddress;

    SONY_TRACE_ENTER ("sony_tuner_MxL608_Shutdown");

    if (!pTuner || !pTuner->user) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

	devId = pTuner->i2cAddress;

    /* Call into underlying driver. */
	status = MxLWare608_API_CfgDevPowerMode(devId, MXL608_PWR_MODE_STANDBY, MXL_DISABLE, 0);

    /* Device is in "Power Save" state. */
    pTuner->system = SONY_DTV_SYSTEM_UNKNOWN;
    pTuner->frequencyKHz = 0;
    pTuner->bandwidth = SONY_DTV_BW_UNKNOWN;

    SONY_TRACE_RETURN (result);
}

static sony_result_t sony_tuner_MxL608_ReadRFLevel (sony_tuner_t * pTuner, int32_t * pRFLevel)
{
    sony_result_t result = SONY_RESULT_OK;
	uint8_t devId;// = pTuner->i2cAddress;
	int16_t PwrPtr = 0;
	MXL_BOOL rfLockPtr = MXL_UNLOCKED;
	MXL_BOOL refLockPtr = MXL_UNLOCKED;

    SONY_TRACE_ENTER ("sony_tuner_MxL608_ReadRFLevel");

    if (!pTuner || !pTuner->user || !pRFLevel) {
        SONY_TRACE_RETURN (SONY_RESULT_ERROR_ARG);
    }

	devId = pTuner->i2cAddress;

	//mt_fe_i2c_repeat_enable_dm6k(handle);

	MxLWare608_API_ReqTunerLockStatus(devId, &rfLockPtr, &refLockPtr);
	
	if((rfLockPtr != MXL_LOCKED) && (refLockPtr != MXL_LOCKED))
	{
		//mt_fe_i2c_repeat_disable_dm6k(handle);
		return SONY_RESULT_ERROR_UNLOCK;
	}

	MxLWare608_API_ReqTunerRxPower(devId, &PwrPtr);

	*pRFLevel = PwrPtr / 100;

	//printk("%s[%d] ---- end! rfLock[%d], refLock[%d], Level = %d\n", __FUNCTION__, __LINE__, rfLockPtr, refLockPtr, *pRFLevel);

	//mt_fe_i2c_repeat_disable_dm6k(handle);

    SONY_TRACE_RETURN (result);
}

