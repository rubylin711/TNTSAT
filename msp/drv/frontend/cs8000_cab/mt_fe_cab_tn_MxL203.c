/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
//#include <stdio.h>
#include "mt_fe_cab_tn_MxL203.h"

#define EXAMPLE_DEV_MAX 2
#define MXL203_I2C_ADDR 0x60


extern MxL_ERR_MSG MxL_Check_RF_Input_Power(MxL203RF_TunerConfigS* myTuner, UINT32* RF_Input_Level);

MxL203RF_TunerConfigS myTuner;

MT_FE_RET mt_fe_tn_init_MxL203(void *dev_handle)
{
	MxL_ERR_MSG Status = MxL_OK;

	//Set Tuner's I2C Address
	myTuner.I2C_Addr = MxL_I2C_ADDR_96;

	//Set Tuner Mode to Cable mode
	myTuner.Mode = MxL_MODE_CAB_STD;

	//Set Tuner's XTAL freq and capacitance
	myTuner.Xtal_Freq = MxL_XTAL_24_MHZ;
	myTuner.Xtal_Cap = MxL_XTAL_CAP_12_PF;

	//Set Tuner's IF Freq
	myTuner.IF_Freq = MxL_IF_6_MHZ;
	myTuner.IF_Spectrum = MxL_NORMAL_IF;

	//Set Tuner's Clock out setting
	myTuner.ClkOut_Setting = MxL_CLKOUT_ENABLE;
	myTuner.ClkOut_Amp = MxL_CLKOUT_AMP_10;

	//Init Tuner
	if((Status = MxL_Tuner_Init(&myTuner)))
	{
		//Init Tuner fail
		return MtFeErr_Fail;
	}

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_tn_set_freq_MxL203(void *dev_handle, U32 freq_KHz, U16 symbol_rate_KSs)
{
	MxL_ERR_MSG Status = MxL_OK;
	MT_BOOL RFSynthLock, REFSynthLock;

	//Tune Tuner
	if((Status = MxL_Tuner_RFTune(&myTuner, (freq_KHz + 500) / 1000, MxL_BW_8MHz)))
	{
		return MtFeErr_Fail;
	}

	//Check Lock Status and RF Input Level
	MxL_RFSynth_Lock_Status(&myTuner, &RFSynthLock);
	MxL_REFSynth_Lock_Status(&myTuner, &REFSynthLock);
	MxL_Enable_LT(&myTuner, 1); //Enable LT

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_tn_get_signal_strength_MxL203(void *dev_handle, U32 *p_gain, U32 *p_strength)
{
	U32 fLevel = 0;

	MxL_Check_RF_Input_Power(&myTuner, &fLevel);

	*p_gain = 0;
	*p_strength = fLevel;

	return MtFeErr_Ok;
}

S32 mt_fe_tn_self_check_MxL203(MT_FE_CS8000_CAB_Device_Handle dev_handle)
{
	U8 tmp = 0;
	U32 ret = 0;

	ret = MxL_I2C_Read((UINT8)(dev_handle->tuner_settings.tuner_dev_addr >> 1), 0x18, &tmp);
	if(ret != 0)
	{
		mt_fe_print(("\tMxL203: I2C failed! Chip addr = 0x%02X\n", dev_handle->tuner_settings.tuner_dev_addr));
		return -1;
	}

	mt_fe_print(("\tMxL203: chip_id[0x18] = 0x%02X, %s\n", tmp, (tmp == 0x02) ? "OK" : "Fail"));

	return (tmp == 0x02) ? 1 : 0;
}


