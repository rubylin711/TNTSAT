/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*******************************************************************************
 * 
 * FILE NAME          : MxL603_TunerApi.cpp
 * 
 * AUTHOR             : Dong Liu
 * 
 * 
 * DATE CREATED       : 3/8/2013
 * 
 * DESCRIPTION        : This file contains MxL603 driver APIs
 * 
 * 
 *******************************************************************************
 *                Copyright (c) 2012, MaxLinear, Inc.
 ******************************************************************************/

#include "mt_fe_def.h"
#include "mt_fe_i2c_cs8800.h"


#include "mt_fe_tn_MxL603.h"
#include "./MxL6/MxL603_TunerCfg.h"

static U8 MXL603_Address_cs8800 = 0;
/* MxLWare Driver version for MxL603 */
const UINT8 MxLWare603DrvVersion_cs8800[5] = {2, 1, 3, 2, 0};

/* OEM Data pointer array */
void * MxL603_OEM_DataPtr[MXL603_MAX_NUM_DEVICES];

/*----------------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_WriteRegister_cs8800
--| 
--| AUTHOR        : Brenndon Lee
--| 
--| DATE CREATED  : 7/30/2009
--| 
--| DESCRIPTION   : This function does I2C write operation.
--| 
--| RETURN VALUE  : True or False
--| 
--|-------------------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_OEM_WriteRegister_cs8800(UINT8 devId, UINT8 RegAddr, UINT8 RegData)
{
  // OEM should implement I2C write protocol that complies with MxL603 I2C
  // format.

  // 8 bit Register Write Protocol:
  // +------+-+-----+-+-+----------+-+----------+-+-+
  // |MASTER|S|SADDR|W| |RegAddr   | |RegData(L)| |P|
  // +------+-+-----+-+-+----------+-+----------+-+-+
  // |SLAVE |         |A|          |A|          |A| |
  // +------+---------+-+----------+-+----------+-+-+
  // Legends: SADDR (I2c slave address), S (Start condition), A (Ack), N(NACK),
  // P(Stop condition)

	MXL_STATUS status = MXL_FALSE;


/* If OEM data is implemented, customer needs to use OEM data structure related operation
   Following code should be used as a reference.
   For more information refer to sections 2.5 & 2.6 of MxL603_mxLWare_API_UserGuide document.

  UINT8 i2cSlaveAddr;
  UINT8 i2c_bus;
  user_data_t * user_data = (user_data_t *) MxL603_OEM_DataPtr[devId];

  if (user_data)
  {
    i2cSlaveAddr = user_data->i2c_address;           // get device i2c address
    i2c_bus = user_data->i2c_bus;                   // get device i2c bus

    sem_up(user_data->sem);                         // up semaphore if needed

    // I2C Write operation
    status = USER_I2C_WRITE_FUNCTION(i2cSlaveAddr, i2c_bus, RegAddr, RegData);

    sem_down(user_data->sem);                       // down semaphore
    user_data->i2c_cnt++;                           // user statistics
  }

*/

  /* If OEM data is not required, customer should treat devId as I2C slave Address */

	U8 buf[2] = {0,0};

	buf[0] = RegAddr;
	buf[1] = RegData;

	if ((mt_fe_tn_write_cs8800(MXL603_Address_cs8800, buf, 2)) != MtFeErr_Ok)
	{
		status = MXL_FALSE;
	}
	else
		status = MXL_TRUE;

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_ReadRegister_cs8800
--| 
--| AUTHOR        : Brenndon Lee
--| 
--| DATE CREATED  : 7/30/2009
--| 
--| DESCRIPTION   : This function does I2C read operation.
--| 
--| RETURN VALUE  : True or False
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_OEM_ReadRegister_cs8800(UINT8 devId, UINT8 RegAddr, UINT8 *DataPtr)
{
  // OEM should implement I2C read protocol that complies with MxL603 I2C
  // format.

  // 8 bit Register Read Protocol:
  // +------+-+-----+-+-+----+-+----------+-+-+
  // |MASTER|S|SADDR|W| |0xFB| |RegAddr   | |P|
  // +------+-+-----+-+-+----+-+----------+-+-+
  // |SLAVE |         |A|    |A|          |A| |
  // +------+-+-----+-+-+----+-+----------+-+-+
  // +------+-+-----+-+-+-----+--+-+
  // |MASTER|S|SADDR|R| |     |MN|P|
  // +------+-+-----+-+-+-----+--+-+
  // |SLAVE |         |A|Data |  | |
  // +------+---------+-+-----+--+-+
  // Legends: SADDR(I2c slave address), S(Start condition), MA(Master Ack), MN(Master NACK),
  // P(Stop condition)

	MXL_STATUS status = MXL_TRUE;

/* If OEM data is implemented, customer needs to use OEM data structure related operation
   Following code should be used as a reference.
   For more information refer to sections 2.5 & 2.6 of MxL603_mxLWare_API_UserGuide document.

  UINT8 i2cSlaveAddr;
  UINT8 i2c_bus;
  user_data_t * user_data = (user_data_t *) MxL603_OEM_DataPtr[devId];

  if (user_data)
  {
    i2cSlaveAddr = user_data->i2c_address;          // get device i2c address
    i2c_bus = user_data->i2c_bus;                   // get device i2c bus

    sem_up(user_data->sem);                         // up semaphore if needed

    // I2C Write operation
    status = USER_I2C_READ_FUNCTION(i2cSlaveAddr, i2c_bus, RegAddr, DataPtr);

    sem_down(user_data->sem);                       // down semaphore
    user_data->i2c_cnt++;                           // user statistics
  }

*/

  /* If OEM data is not required, customer should treat devId as I2C slave Address */



	U8 tmp[2] = {0, 0};

	tmp[0] = 0xfb;
	tmp[1] = RegAddr;
	if ((mt_fe_tn_read_cs8800(MXL603_Address_cs8800, tmp, 2, DataPtr, 1)) != MtFeErr_Ok)
	{
		status = MXL_FALSE;
	}
	else
	{
		status = MXL_TRUE;
	}

	return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_OEM_Sleep_cs8800
--| 
--| AUTHOR        : Dong Liu
--| 
--| DATE CREATED  : 01/10/2010
--| 
--| DESCRIPTION   : This function complete sleep operation. WaitTime is in ms unit
--| 
--| RETURN VALUE  : None
--| 
--|-------------------------------------------------------------------------------------*/

static void MxLWare603_OEM_Sleep_cs8800(UINT16 DelayTimeInMs)
{
	// OEM should implement sleep operation
	_mt_sleep_cs8800(DelayTimeInMs);
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgDrvInit_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This API must be called prior to any other API function.
--|                 Cannot be called more than once.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgDrvInit_cs8800(UINT8 devId, void* oemDataPtr)
{
  MXL_STATUS status = MXL_SUCCESS;

  if (oemDataPtr)
  {
    if (devId < MXL603_MAX_NUM_DEVICES) MxL603_OEM_DataPtr[devId] = oemDataPtr;
    else status = MXL_INVALID_PARAMETER;
  }

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgDevSoftReset_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This API is used to reset MxL603 tuner device. After reset,
--|                 all the device regiaters and modules will be set to power-on
--|                 default state.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgDevSoftReset_cs8800(UINT8 devId)
{
  UINT8 status = MXL_SUCCESS;

//  MxL_DLL_DEBUG0("%s", __FUNCTION__);

  // Write 0xFF with 0 to reset tuner
  status = MxLWare603_OEM_WriteRegister_cs8800(devId, AIC_RESET_REG, 0x00);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgDevOverwriteDefaults_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : Register(s) that requires default values to be overwritten
--|                 during initialization
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgDevOverwriteDefaults_cs8800(UINT8 devId, 
                                                  MXL_BOOL singleSupply_3_3V)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 readData = 0;

//  MxL_DLL_DEBUG0("%s", __FUNCTION__);

  status |= MxL603_Ctrl_ProgramRegisters_cs8800(devId, MxL603_OverwriteDefaults_cs8800);

  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x00, 0x01);
  MxLWare603_OEM_ReadRegister_cs8800(devId, 0x00, &readData);

  status |= MxLWare603_OEM_ReadRegister_cs8800(devId, 0x31, &readData);

  readData &= 0x2F;
  readData |= 0xD0;
  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x31, readData);
  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x00, 0x00);


  /* If Single supply 3.3v is used */
  if (MXL_ENABLE == singleSupply_3_3V)
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, MAIN_REG_AMP, 0x04);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgDevXtal_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--|                 Dong Liu
--| 
--| DATE CREATED  : 12/10/2011
--|                 08/29/2012
--| 
--| DESCRIPTION   : This API is used to configure XTAL settings of MxL603 tuner
--|                 device. XTAL settings include frequency, capacitance &
--|                 clock out
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgDevXtal_cs8800(UINT8 devId, MXL603_XTAL_SET_CFG_T xtalCfg)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 control = 0;

//  MxL_DLL_DEBUG0("%s", __FUNCTION__);

  // XTAL freq and cap setting, Freq set is located at bit<5>, cap bit<4:0>
  // and  XTAL clock out enable <0>
  if ((xtalCfg.xtalFreqSel == MXL603_XTAL_16MHz) || (xtalCfg.xtalFreqSel == MXL603_XTAL_24MHz))
  {
    control = (UINT8)((xtalCfg.xtalFreqSel << 5) | (xtalCfg.xtalCap & 0x1F));
    control |= (xtalCfg.clkOutEnable << 7);
    status = MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_CAP_CTRL_REG, control);

    // XTAL frequency div 4 setting <1>
    control = (0x01 & (UINT8)xtalCfg.clkOutDiv);

    // XTAL sharing mode
    if (xtalCfg.XtalSharingMode == MXL_ENABLE)
    {
      control |= 0x40;
      // program Clock out div & Xtal sharing
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_ENABLE_DIV_REG, control);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_EXT_BIAS_REG, 0x80);
    }
    else
    {
      control &= 0x01;
      // program Clock out div & Xtal sharing
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_ENABLE_DIV_REG, control);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_EXT_BIAS_REG, 0x0A);
    }

    // Main regulator re-program
    if (MXL_ENABLE == xtalCfg.singleSupply_3_3V)
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, MAIN_REG_AMP, 0x14);
  }
  else
    status |= MXL_INVALID_PARAMETER;

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgDevPowerMode_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--|                 Dong Liu
--| 
--| DATE CREATED  : 12/10/2011
--|                 03/21/2013
--| 
--| DESCRIPTION   : This function configures MxL603 power mode
--|                 If Power mode is set to standby and application environment
--|                 is south africa area, then the standbyLt shall be set as 0x06
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgDevPowerMode_cs8800(UINT8 devId,
                                          MXL603_PWR_MODE_E powerMode,
                                          MXL_BOOL enableLoopthrough,
                                          UINT8 standbyLt)
{
  UINT8 status = MXL_SUCCESS;

//  MxL_DLL_DEBUG0("%s", __FUNCTION__);

  switch(powerMode)
  {
    case MXL603_PWR_MODE_SLEEP:
      break;

    case MXL603_PWR_MODE_ACTIVE:
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, TUNER_ENABLE_REG, MXL_ENABLE);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, START_TUNE_REG, MXL_ENABLE);

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x01);
      if (enableLoopthrough == MXL_ENABLE)
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_TUNE_RF1_BO_REG, 0x0E);
      else
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_TUNE_RF1_BO_REG, 0x37);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x00);
      break;

    case MXL603_PWR_MODE_STANDBY:
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, START_TUNE_REG, MXL_DISABLE);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, TUNER_ENABLE_REG, MXL_DISABLE);
      if ((standbyLt != 0) && (enableLoopthrough == MXL_ENABLE))
      {
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x01);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_TUNE_RF1_BO_REG, (UINT8)(standbyLt & 0x3F));
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x00);
      }
      break;

    default:
      status |= MXL_INVALID_PARAMETER;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgDevGPO_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This API configures GPO pin of MxL603 tuner device.
--|                 There is only 1 GPO pin available in MxL603 device.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgDevGPO_cs8800(UINT8 devId, MXL603_GPO_STATE_E gpoState)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 regData = 0;

//  MxL_DLL_DEBUG0("%s", __FUNCTION__);

  switch(gpoState)
  {
    case MXL603_GPO_AUTO_CTRL:
    case MXL603_GPO_HIGH:
    case MXL603_GPO_LOW:
      status = MxLWare603_OEM_ReadRegister_cs8800(devId, GPO_SETTING_REG, &regData);
      if (MXL603_GPO_AUTO_CTRL == gpoState)
        regData &= 0xFE;
      else
      {
        regData &= 0xFC;
        regData |= (UINT8)(0x01 | (gpoState << 1));
      }

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, GPO_SETTING_REG, regData);
      break;

    default:
      status = MXL_INVALID_PARAMETER;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_ReqDevVersionInfo_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This function is used to get MxL603 version information.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_ReqDevVersionInfo_cs8800(UINT8 devId,
                                            MXL603_VER_INFO_T* mxlDevVerInfoPtr)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 readBack = 0;
  UINT8 k = 0;

//  MxL_DLL_DEBUG0("%s", __FUNCTION__);

  if (mxlDevVerInfoPtr)
  {
    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, CHIP_ID_REQ_REG, &readBack);
    mxlDevVerInfoPtr->chipId = (readBack & 0xFF);

    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, CHIP_VERSION_REQ_REG, &readBack);
    mxlDevVerInfoPtr->chipVersion = (readBack & 0xFF);

    // MxL_DLL_DEBUG0("Chip ID = 0x%d, Version = 0x%d \n", mxlDevVerInfoPtr->chipId, 
    //                                                    mxlDevVerInfoPtr->chipVersion);

    // Get MxLWare version infromation
    for (k = 0; k < MXL603_VERSION_SIZE; k++)
      mxlDevVerInfoPtr->mxlwareVer[k] = MxLWare603DrvVersion_cs8800[k];
  }
  else
    status = MXL_INVALID_PARAMETER;

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_ReqDevGPOStatus_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This API is used to get GPO pin's status information from
--|                 MxL603 tuner device.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_ReqDevGPOStatus_cs8800(UINT8 devId, 
                                          MXL603_GPO_STATE_E* gpoStatusPtr)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 regData = 0;

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  if (gpoStatusPtr)
  {
    status = MxLWare603_OEM_ReadRegister_cs8800(devId, GPO_SETTING_REG, &regData);

    // GPO1 bit<1:0>
    if ((regData & 0x01) == 0) *gpoStatusPtr = MXL603_GPO_AUTO_CTRL;
    else *gpoStatusPtr = (MXL603_GPO_STATE_E)((regData & 0x02) >> 1);
  }
  else
    status = MXL_INVALID_PARAMETER;

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_ReqDevPllState_cs8800
--| 
--| AUTHOR        : Dong Liu
--| 
--| DATE CREATED  : 11/26/2012
--| 
--| DESCRIPTION   : This API is used to check PLL state is normal or wrong.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_ReqDevPllState_cs8800(UINT8 devId, MXL603_PLL_STATE_E* PllStatePtr)
{
  UINT8 status = MXL_TRUE;
  UINT8 condition[3] = {0};
  UINT8 regAddr[7] = {0x2B, 0x30, 0x32, 0x34, 0x2F, 0x31, 0x33};
  UINT8 k, regData[7] = {0};

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  *PllStatePtr = MXL603_PLL_STATE_NA;

  for (k = 0; k < 7; k++)
  {
    // inquire PLL circuitry status and RSSI read back registers
    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, regAddr[k], &regData[k]);
  }

  if ((MXL_STATUS)status == MXL_SUCCESS)
  {
    // Check 0x2B register
    condition[0] = (regData[0] != 0x07)? 1 : 0;

    // Check if register 0x30, 0x32, 0x34 values are all 0
    condition[1] = ((regData[1] == 0) && (regData[2] == 0) && (regData[3] == 0)) ? 1: 0;

    // Check if register 0x30, 0x32, 0x34 values are all 0
    condition[2] = ((regData[1] == regData[2]) && (regData[2] == regData[3])
                 && (regData[4] == regData[5]) && (regData[5] == regData[6])) ? 1: 0;

    if ((condition[0] == 1) || (condition[1] == 1) || (condition[2] == 1))
      *PllStatePtr = MXL603_PLL_STATE_WRONG;
    else
      *PllStatePtr = MXL603_PLL_STATE_NORMAL;
  }

 // MxL_DLL_DEBUG0("Tuner PLL state = %d (0:Normal, 1:Wrong) \n", *PllStatePtr);
  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgTunerMode_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This fucntion is used to configure MxL603 tuner's
--|                 application modes like DVB-T, DVB-C, ISDB-T etc.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgTunerMode_cs8800(UINT8 devId, 
                                       MXL603_TUNER_MODE_CFG_T tunerModeCfg)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 dfeRegData = 0;
  MXL603_REG_CTRL_INFO_T* tmpRegTable;

/* MxL_DLL_DEBUG0("%s: Signal Mode = %d, IF Freq = %d, xtal = %d, IF Gain = %d", 
                                                __FUNCTION__,
                                                tunerModeCfg.signalMode,
                                                tunerModeCfg.ifOutFreqinKHz,
                                                tunerModeCfg.xtalFreqSel,
                                                tunerModeCfg.ifOutGainLevel);
*/
  switch(tunerModeCfg.signalMode)
  {
    case MXL603_DIG_DVB_C:
    case MXL603_DIG_J83B:
      tmpRegTable = MxL603_DigitalDvbc_cs8800;
      status = MxL603_Ctrl_ProgramRegisters_cs8800(devId, tmpRegTable);

      if (tunerModeCfg.ifOutFreqinKHz < HIGH_IF_35250_KHZ)
      {
        // Low power
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_0, 0xFE);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_1, 0x10);
      }
      else
      {
        // High power
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_0, 0xD9);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_1, 0x16);
      }

      if (tunerModeCfg.xtalFreqSel == MXL603_XTAL_16MHz) dfeRegData = 0x0D;
      else if (tunerModeCfg.xtalFreqSel == MXL603_XTAL_24MHz) dfeRegData = 0x0E;
      else status |= MXL_INVALID_PARAMETER;

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_CSF_SS_SEL, dfeRegData);

      break;

    case MXL603_DIG_ISDBT_ATSC:
      tmpRegTable = MxL603_DigitalIsdbtAtsc_cs8800;
      status = MxL603_Ctrl_ProgramRegisters_cs8800(devId, tmpRegTable);

      if (tunerModeCfg.ifOutFreqinKHz < HIGH_IF_35250_KHZ)
      {
        // Low power
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_0, 0xF9);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_1, 0x18);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_PWR, 0xF1);
      }
      else
      {
        // High power
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_0, 0xD9);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_1, 0x16);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_PWR, 0xB1);
      }

      if (MXL603_XTAL_16MHz == tunerModeCfg.xtalFreqSel) dfeRegData = 0x0D;
      else if (MXL603_XTAL_24MHz == tunerModeCfg.xtalFreqSel) dfeRegData = 0x0E;
      else status |= MXL_INVALID_PARAMETER;

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_CSF_SS_SEL, dfeRegData);

      dfeRegData = 0x1C;
      switch(tunerModeCfg.ifOutGainLevel)
      {
        case 0x09: dfeRegData = 0x44; break;
        case 0x08: dfeRegData = 0x43; break;
        case 0x07: dfeRegData = 0x42; break;
        case 0x06: dfeRegData = 0x41; break;
        case 0x05: dfeRegData = 0x40; break;
        default: break;
      }
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_DACIF_GAIN, dfeRegData);

      break;

    case MXL603_DIG_DVB_T_DTMB:
      tmpRegTable = MxL603_DigitalDvbt_cs8800;
      status = MxL603_Ctrl_ProgramRegisters_cs8800(devId, tmpRegTable);
      if (tunerModeCfg.ifOutFreqinKHz < HIGH_IF_35250_KHZ)
      {
        // Low power
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_0, 0xFE);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_1, 0x18);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_PWR, 0xF1);
      }
      else
      {
        // High power
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_0, 0xD9);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_CFG_1, 0x16);
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_IF_PWR, 0xB1);
      }
      if (MXL603_XTAL_16MHz == tunerModeCfg.xtalFreqSel) dfeRegData = 0x0D;
      else if (MXL603_XTAL_24MHz == tunerModeCfg.xtalFreqSel) dfeRegData = 0x0E;
      else status |= MXL_INVALID_PARAMETER;

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_CSF_SS_SEL, dfeRegData);
      dfeRegData = 0;
      switch(tunerModeCfg.ifOutGainLevel)
      {
        case 0x09: dfeRegData = 0x44; break;
        case 0x08: dfeRegData = 0x43; break;
        case 0x07: dfeRegData = 0x42; break;
        case 0x06: dfeRegData = 0x41; break;
        case 0x05: dfeRegData = 0x40; break;
        default: break;
      }
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_DACIF_GAIN, dfeRegData);
      break;

    default:
      status = MXL_INVALID_PARAMETER;
      break;
  }

  if (status == MXL_SUCCESS)
  {
    // XTAL calibration
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_CALI_SET_REG, 0x00);
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, XTAL_CALI_SET_REG, 0x01);

    // 50 ms sleep after XTAL calibration
    MxLWare603_OEM_Sleep_cs8800(50);
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgTunerAGC_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This function is used to configure AGC settings of MxL603
--|                 tuner device.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgTunerAGC_cs8800(UINT8 devId, MXL603_AGC_CFG_T agcCfg)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 regData = 0;

/* MxL_DLL_DEBUG0("%s, AGC sel = %d, attack point set = %d, Flip = %d \n", 
                                                __FUNCTION__, 
                                                agcCfg.agcType, 
                                                agcCfg.setPoint, 
                                                agcCfg.agcPolarityInverstion);
*/
  if ((agcCfg.agcPolarityInverstion <= MXL_ENABLE) && 
      (agcCfg.agcType <= MXL603_AGC_EXTERNAL))
  {
    // AGC selecton <3:2> and mode setting <0>
    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, AGC_CONFIG_REG, &regData);
    regData &= 0xF2; // Clear bits <3:2> & <0>
    regData = (UINT8) (regData | (agcCfg.agcType << 2) | 0x01);
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, AGC_CONFIG_REG, regData);

    // AGC set point <6:0>
    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, AGC_SET_POINT_REG, &regData);
    regData &= 0x80; // Clear bit <6:0>
    regData |= agcCfg.setPoint;
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, AGC_SET_POINT_REG, regData);

    // AGC Polarity <4>
    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, AGC_FLIP_REG, &regData);
    regData &= 0xEF; // Clear bit <4>
    regData |= (agcCfg.agcPolarityInverstion << 4);
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, AGC_FLIP_REG, regData);
  }
  else
    status = MXL_INVALID_PARAMETER;

  return(MXL_STATUS) status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgTunerLoopThrough_cs8800
--| 
--| AUTHOR        : Mahendra Kondur, Dong Liu
--| 
--| DATE CREATED  : 12/10/2011, 06/18/2012
--| 
--| DESCRIPTION   : This function is used to enable or disable Loop-Through
--|                 settings of MxL603 tuner device.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgTunerLoopThrough_cs8800(UINT8 devId, MXL_BOOL loopThroughCtrl, UINT8* gainArray, UINT8* attArray)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 regData;

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  if (loopThroughCtrl <= MXL_ENABLE)
  {
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x01);

    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, DIG_ANA_GINJO_LT_REG, &regData);

    if (loopThroughCtrl == MXL_ENABLE)
      regData |= 0x10;  // Bit<4> = 1
    else
      regData &= 0xEF;  // Bit<4> = 0
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DIG_ANA_GINJO_LT_REG, regData);

    if ((loopThroughCtrl == MXL_ENABLE) && (gainArray != NULL) && (attArray != NULL))
    {
      regData = (gainArray[3]<<6 ) | (gainArray[2] <<4 ) | (gainArray[1] <<2 ) | gainArray[0] ;
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_DIGANA_LT_GAIN_REG, regData);
      regData = (attArray[3]<<6 ) | (attArray[2] <<4 ) | (attArray[1] <<2 ) | attArray[0] ;
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_DIGANA_LT_ATTN_REG, regData);
    }
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x00);
  }
  else
    status = MXL_INVALID_PARAMETER;

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgTunerSouthAfricaLT_cs8800
--| 
--| AUTHOR        : Dong Liu
--| 
--| DATE CREATED  : 03/21/2013
--| 
--| DESCRIPTION   : This function is used to set Loop-Through settings of
--|                 MxL603 tuner device for South Africa.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgTunerSouthAfricaLT_cs8800(UINT8 devId)
{
  UINT8 status = MXL_SUCCESS;

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x01);
  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_DIGANA_LT_GAIN_REG, 0x95);
  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, DFE_SEQ_DIGANA_LT_ATTN_REG, 0x00);
  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x00);

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgTunerChanTune_cs8800
--| 
--| AUTHOR        : Mahendra Kondur, Dong Liu
--| 
--| DATE CREATED  : 12/10/2011, 10/27/2012
--| 
--| DESCRIPTION   : This API configures RF channel frequency and bandwidth.
--|                 Radio Frequency unit is Hz, and Bandwidth is in MHz units.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgTunerChanTune_cs8800(UINT8 devId, 
                                           MXL603_CHAN_TUNE_CFG_T chanTuneCfg)
{
  UINT32 frequency;
  UINT32 freq = 0;
  UINT8 status = MXL_SUCCESS;
  UINT8 regData = 0;
  UINT8 agcData = 0;
  UINT8 dfeTuneData = 0;
  UINT8 dfeCdcData = 0;
  MXL603_CHAN_DEPENDENT_FREQ_TABLE_T *freqLutPtr = NULL;

  /*MxL_DLL_DEBUG0("%s, signal type = %d, Freq = %d, BW = %d, Xtal = %d \n", 
                                              __FUNCTION__, 
                                              chanTuneCfg.signalMode, 
                                              chanTuneCfg.freqInHz, 
                                              chanTuneCfg.bandWidth, 
                                              chanTuneCfg.xtalFreqSel);
*/
  // Abort Tune
  status |= MxLWare603_OEM_WriteRegister_cs8800(devId, START_TUNE_REG, 0x00);

  if (chanTuneCfg.startTune == MXL_ENABLE)
  {
    if (chanTuneCfg.signalMode <= MXL603_DIG_J83B)
    {
      // RF Frequency VCO Band Settings
      if (chanTuneCfg.freqInHz < APP_MODE_FREQ_HZ_THRESHOLD_3)
      {
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x7C, 0x1F);
        if ((chanTuneCfg.signalMode == MXL603_DIG_DVB_C) || (chanTuneCfg.signalMode == MXL603_DIG_J83B))
          regData = 0xC1;
        else
          regData = 0x81;
      }
      else
      {
        status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x7C, 0x9F);
        if ((chanTuneCfg.signalMode == MXL603_DIG_DVB_C) || (chanTuneCfg.signalMode == MXL603_DIG_J83B))
          regData = 0xD1;
        else
          regData = 0x91;
      }

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x00, 0x01);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x31, regData);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x00, 0x00);

      // Process spur table programming
      switch (chanTuneCfg.signalMode)
      {
        case MXL603_DIG_DVB_C:
        case MXL603_DIG_J83B:
          freqLutPtr = MXL603_DIG_CABLE_FREQ_LUT_cs8800;
          break;

        case MXL603_DIG_ISDBT_ATSC:
        case MXL603_DIG_DVB_T_DTMB:
          freqLutPtr = MXL603_DIG_TERR_FREQ_LUT_cs8800;
          break;
      }

      if (freqLutPtr)
        status |= Ctrl_SetRfFreqLutTblReg_cs8800(devId, chanTuneCfg.freqInHz, freqLutPtr);

      // Bandwidth <7:0>
      switch(chanTuneCfg.bandWidth)
      {
        case MXL603_CABLE_BW_6MHz:
        case MXL603_CABLE_BW_7MHz:
        case MXL603_CABLE_BW_8MHz:
        case MXL603_TERR_BW_6MHz:
        case MXL603_TERR_BW_7MHz:
        case MXL603_TERR_BW_8MHz:
            status |= MxLWare603_OEM_WriteRegister_cs8800(devId, CHAN_TUNE_BW_REG, (UINT8)chanTuneCfg.bandWidth);

            // Frequency
            frequency = chanTuneCfg.freqInHz/1000;

            /* Calculate RF Channel = DIV(64*RF(Hz), 1E6) */
            frequency *= 64;
            freq = (UINT32)((frequency + 500) / 1000); // Round operation

            // Set RF
            status |= MxLWare603_OEM_WriteRegister_cs8800(devId, CHAN_TUNE_LOW_REG, (UINT8)(freq & 0xFF));
            status |= MxLWare603_OEM_WriteRegister_cs8800(devId, CHAN_TUNE_HI_REG, (UINT8)((freq >> 8 ) & 0xFF));
            break;

        default:
          status |= MXL_INVALID_PARAMETER;
          break;
      }

      // Power up tuner module
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, TUNER_ENABLE_REG, 0x01);

      // Start Sequencer settings
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x01);
      status |= MxLWare603_OEM_ReadRegister_cs8800(devId, DIG_ANA_GINJO_LT_REG, &regData);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x00);

      status |= MxLWare603_OEM_ReadRegister_cs8800(devId, 0xB6, &agcData);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x01);
      status |= MxLWare603_OEM_ReadRegister_cs8800(devId, 0x60, &dfeTuneData);
      status |= MxLWare603_OEM_ReadRegister_cs8800(devId, 0x5F, &dfeCdcData);

      // Check if LT is enabled
      if ((regData & 0x10) == 0x10)
      {
        // dfe_agc_auto = 0 & dfe_agc_rf_bo_w = 14
        agcData &= 0xBF;
        agcData |= 0x0E;

        // dfe_seq_tune_rf1_bo = 14
        dfeTuneData &= 0xC0;
        dfeTuneData |= 0x0E;

        // dfe_seq_cdc_rf1_bo = 14
        dfeCdcData &= 0xC0;
        dfeCdcData |= 0x0E;
      }
      else
      {
        // dfe_agc_auto = 1 & dfe_agc_rf_bo_w = 0
        agcData |= 0x40;
        agcData &= 0xC0;

        // dfe_seq_tune_rf1_bo = 55
        dfeTuneData &= 0xC0;
        dfeTuneData |= 0x37;

        // dfe_seq_cdc_rf1_bo = 55
        dfeCdcData &= 0xC0;
        dfeCdcData |= 0x37;
      }

      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x60, dfeTuneData);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0x5F, dfeCdcData);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, PAGE_CHANGE_REG, 0x00);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0xB6, agcData);

      // Bit <0> 1 : start , 0 : abort calibrations
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, START_TUNE_REG, 0x01);

      // Sleep 15 ms
      MxLWare603_OEM_Sleep_cs8800(15);

      // dfe_agc_auto = 1
      agcData = (agcData | 0x40);
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, 0xB6, agcData);
    }
    else
      status = MXL_INVALID_PARAMETER;
  }

  return (MXL_STATUS)status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_CfgTunerIFOutParam_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This function is used to configure IF out settings of MxL603
--|                 tuner device.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_CfgTunerIFOutParam_cs8800(UINT8 devId, MXL603_IF_OUT_CFG_T ifOutCfg)
{
  UINT16 ifFcw;
  UINT8 status = MXL_SUCCESS;
  UINT8 readData = 0;
  UINT8 control = 0;

  //MxL_DLL_DEBUG0("%s, Manual set = %d \n", __FUNCTION__, ifOutCfg.manualFreqSet);

  // Read back register for manual IF Out
  status = MxLWare603_OEM_ReadRegister_cs8800(devId, IF_FREQ_SEL_REG, &readData);

  if (ifOutCfg.manualFreqSet == MXL_ENABLE)
  {
   // MxL_DLL_DEBUG0("%s, IF Freq = %d \n", __FUNCTION__, ifOutCfg.manualIFOutFreqInKHz);

    // IF out manual setting : bit<5>
    readData |= 0x20;
    status = MxLWare603_OEM_WriteRegister_cs8800(devId, IF_FREQ_SEL_REG, readData);

    // Manual IF freq set
    ifFcw = (UINT16)(ifOutCfg.manualIFOutFreqInKHz * 8192 / 216000);
    control = (ifFcw & 0xFF); // Get low 8 bit
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, IF_FCW_LOW_REG, control);

    control = ((ifFcw >> 8) & 0x0F); // Get high 4 bit
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, IF_FCW_HIGH_REG, control);
  }
  else if (ifOutCfg.manualFreqSet == MXL_DISABLE)
  {
    // bit<5> = 0, use IF frequency from IF frequency table
    readData &= 0xC0;

    // IF Freq <4:0>
    readData |= ifOutCfg.ifOutFreq;
    status |= MxLWare603_OEM_WriteRegister_cs8800(devId, IF_FREQ_SEL_REG, readData);
  }
  else
    status |= MXL_INVALID_PARAMETER;

  if (status == MXL_SUCCESS)
  {
    // Set spectrum invert, gain level and IF path
    // Spectrum invert indication is bit<7:6>
    if (ifOutCfg.ifInversion <= MXL_ENABLE)
    {
      control = 0;
      if (MXL_ENABLE == ifOutCfg.ifInversion) control = 0x3 << 6;

      // Gain level is bit<3:0>
      control += (ifOutCfg.gainLevel & 0x0F);
      control |= (0x20); // Enable IF out
      status |= MxLWare603_OEM_WriteRegister_cs8800(devId, IF_PATH_GAIN_REG, control);
    }
    else
      status |= MXL_INVALID_PARAMETER;
  }

  return(MXL_STATUS) status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_ReqTunerAGCLock_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This function returns AGC Lock status of MxL603 tuner.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_ReqTunerAGCLock_cs8800(UINT8 devId, MXL_BOOL* agcLockStatusPtr)
{
  MXL_STATUS status = MXL_SUCCESS;
  UINT8 regData = 0;
  MXL_BOOL lockStatus = MXL_UNLOCKED;

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  if (agcLockStatusPtr)
  {
    status = MxLWare603_OEM_ReadRegister_cs8800(devId, AGC_SAGCLOCK_STATUS_REG, &regData);
    if ((regData & 0x08) == 0x08) lockStatus = MXL_LOCKED;

    *agcLockStatusPtr =  lockStatus;

  //  MxL_DLL_DEBUG0(" Agc lock = %d", (UINT8)*agcLockStatusPtr);
  }
  else
    status = MXL_INVALID_PARAMETER;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_ReqTunerLockStatus_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--| 
--| DATE CREATED  : 12/10/2011
--| 
--| DESCRIPTION   : This function returns Tuner Lock status of MxL603 tuner.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_ReqTunerLockStatus_cs8800(UINT8 devId, MXL_BOOL* rfLockPtr,
                                                          MXL_BOOL* refLockPtr)
{
  MXL_STATUS status = MXL_SUCCESS;
  UINT8 regData = 0;
  MXL_BOOL rfLockStatus = MXL_UNLOCKED;
  MXL_BOOL refLockStatus = MXL_UNLOCKED;

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  if ((rfLockPtr) && (refLockPtr))
  {
    status = MxLWare603_OEM_ReadRegister_cs8800(devId, RF_REF_STATUS_REG, &regData);

    if ((regData & 0x02) == 0x02) rfLockStatus = MXL_LOCKED;
    if ((regData & 0x01) == 0x01) refLockStatus = MXL_LOCKED;

  //  MxL_DLL_DEBUG0(" RfSynthStatus = %d, RefSynthStatus = %d", (UINT8)rfLockStatus,
   //                                                            (UINT8)refLockStatus);

    *rfLockPtr =  rfLockStatus;
    *refLockPtr = refLockStatus;
  }
  else
    status = MXL_INVALID_PARAMETER;

  return status;
}

/*------------------------------------------------------------------------------
--| FUNCTION NAME : MxLWare603_API_ReqTunerRxPower_cs8800
--| 
--| AUTHOR        : Mahendra Kondur
--|                 Dong Liu
--| 
--| DATE CREATED  : 12/10/2011
--|                 06/18/2012
--| 
--| DESCRIPTION   : This function returns RF input power in 0.01dBm.
--| 
--| RETURN VALUE  : MXL_SUCCESS, MXL_INVALID_PARAMETER, MXL_FAILED
--| 
--|---------------------------------------------------------------------------*/

MXL_STATUS MxLWare603_API_ReqTunerRxPower_cs8800(UINT8 devId, SINT16* rxPwrPtr)
{
  UINT8 status = MXL_SUCCESS;
  UINT8 regData = 0;
  UINT16 tmpData = 0;

 // MxL_DLL_DEBUG0("%s", __FUNCTION__);

  if (rxPwrPtr)
  {
    // RF input power low <7:0>
    status = MxLWare603_OEM_ReadRegister_cs8800(devId, RFPIN_RB_LOW_REG, &regData);
    tmpData = regData;

    // RF input power high <1:0>
    status |= MxLWare603_OEM_ReadRegister_cs8800(devId, RFPIN_RB_HIGH_REG, &regData);
    tmpData |= (regData & 0x03) << 8;

    // Fractional last 2 bits
    *rxPwrPtr = (tmpData & 0x01FF) * 25;  //100 times dBm

    if (tmpData & 0x02) *rxPwrPtr += 50;;
    if (tmpData & 0x01) *rxPwrPtr += 25;
    if (tmpData & 0x0200) *rxPwrPtr -= 128*100;
  }
  else
    status = MXL_INVALID_PARAMETER;

  return (MXL_STATUS)status;
}


/*	FUNCTIN:
**		mt_fe_tn_MxL603_Init
**
**	DESCRIPTION:
**		 init mxl603 tuner
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/

MT_FE_RET mt_fe_tn_init_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MXL_STATUS status;
	UINT8 devId;
	MXL_BOOL singleSupply_3_3V;
	MXL603_XTAL_SET_CFG_T xtalCfg;
	MXL603_IF_OUT_CFG_T ifOutCfg;
	MXL603_AGC_CFG_T agcCfg;
	MXL603_TUNER_MODE_CFG_T tunerModeCfg;


	/* If OEM data is implemented, customer needs to use OEM data structure
	   related operation. Following code should be used as a reference.
	   For more information refer to sections 2.5 & 2.6 of
	   MxL603_mxLWare_API_UserGuide document.

	for (devId = 0; devId < EXAMPLE_DEV_MAX; devId++)
	{
		// assigning i2c address for  the device
		device_context[devId].i2c_address = GET_FROM_FILE_I2C_ADDRESS(devId);

		// assigning i2c bus for  the device
		device_context[devId].i2c_bus = GET_FROM_FILE_I2C_BUS(devId);

		// create semaphore if necessary
		device_context[devId].sem = CREATE_SEM();

		// sample stat counter
		device_context[devId].i2c_cnt = 0;

		status = MxLWare603_API_CfgDrvInit_cs8800(devId, (void *) &device_context[devId]);

		// if you don’t want to pass any oem data, just use NULL as a parameter:
		// status = MxLWare603_API_CfgDrvInit_cs8800(devId, NULL);
	}

	*/

	/* If OEM data is not required, customer should treat devId as
	 I2C slave Address */
	devId = 0xC0;//MXL603_I2C_ADDR;

	MXL603_Address_cs8800 = handle->m_device_c_b.tuner_cfg.tuner_dev_addr;
	//Step 1 : Soft Reset MxL603
	status = MxLWare603_API_CfgDevSoftReset_cs8800(devId);
	if (status != MXL_SUCCESS)
	{
		return MtFeErr_Fail;
	}

	//Step 2 : Overwrite Default
	singleSupply_3_3V = MXL_ENABLE;//MXL_DISABLE;
	status = MxLWare603_API_CfgDevOverwriteDefaults_cs8800(devId, singleSupply_3_3V);
	if (status != MXL_SUCCESS)
	{
		return MtFeErr_Fail;
	}

	//Step 3 : XTAL Setting
	xtalCfg.xtalFreqSel = MXL603_XTAL_24MHz;
	xtalCfg.xtalCap = 16;//20;//12;//16 for 24 mhz,20 for 16mhz
	xtalCfg.clkOutEnable = MXL_ENABLE;//MXL_DISABLE;
	xtalCfg.clkOutDiv = MXL_DISABLE;
	xtalCfg.clkOutExt = MXL_DISABLE;
	xtalCfg.singleSupply_3_3V = MXL_ENABLE;
	xtalCfg.XtalSharingMode = MXL_DISABLE;
	status = MxLWare603_API_CfgDevXtal_cs8800(devId, xtalCfg);
	if (status != MXL_SUCCESS)
	{
		return MtFeErr_Fail;
		// printf("Error! MxLWare603_API_CfgDevXtal_cs8800\n");
	}

	//Step 4 : IF Out setting
	if(handle->m_device_c_b.demod_type == MtFeType_DVBC)
	{
		ifOutCfg.ifOutFreq = MXL603_IF_7_2MHz;//MXL603_IF_5MHz;//MXL603_IF_4_1MHz;
	}
	else if(handle->m_device_c_b.demod_type == MtFeType_J83B)
	{
		ifOutCfg.ifOutFreq = MXL603_IF_7_2MHz;//MXL603_IF_5MHz;//MXL603_IF_4_1MHz;
	}
	else
	{
		ifOutCfg.ifOutFreq = MXL603_IF_5MHz;//MXL603_IF_36MHz;//15MHz;//MXL603_IF_4_1MHz;
	}

	ifOutCfg.ifInversion = MXL_DISABLE;
	ifOutCfg.gainLevel = 11;
	ifOutCfg.manualFreqSet = MXL_DISABLE;
	ifOutCfg.manualIFOutFreqInKHz = 0;
	status = MxLWare603_API_CfgTunerIFOutParam_cs8800(devId, ifOutCfg);
	if (status != MXL_SUCCESS)
	{
		return MtFeErr_Fail;
		//printf("Error! MxLWare603_API_CfgTunerIFOutParam_cs8800\n");
	}

	//Step 5 : AGC Setting
	agcCfg.agcType = MXL603_AGC_EXTERNAL;
	agcCfg.setPoint = 66;
	agcCfg.agcPolarityInverstion = MXL_DISABLE;
	status = MxLWare603_API_CfgTunerAGC_cs8800(devId, agcCfg);
	if (status != MXL_SUCCESS)
	{
		return MtFeErr_Fail;
		//printf("Error! MxLWare603_API_CfgTunerAGC_cs8800\n");
	}

	//Step 6 : Application Mode setting
	if(handle->m_device_c_b.demod_type == MtFeType_J83B)
	{
		tunerModeCfg.signalMode = MXL603_DIG_J83B;
		tunerModeCfg.ifOutFreqinKHz = 7200;//5000;//4570;// 4100;
	}
	else
	{
		tunerModeCfg.signalMode = MXL603_DIG_DVB_C;
		tunerModeCfg.ifOutFreqinKHz = 7200;//5000;//4570;// 4100;
	}

	tunerModeCfg.xtalFreqSel = MXL603_XTAL_24MHz;
	tunerModeCfg.ifOutGainLevel = 11;
	status = MxLWare603_API_CfgTunerMode_cs8800(devId, tunerModeCfg);
	if (status != MXL_SUCCESS)
	{
		return MtFeErr_Fail;
		//printf("Error! MxLWare603_API_CfgTunerMode_cs8800\n");
	}

	handle->m_device_c_b.tuner_cfg.tuner_init_ok = 1;
	handle->m_device_c_b.tuner_cfg.tuner_open = 1;

	return MtFeErr_Ok;
}

/*	FUNCTIN:
**		mt_fe_tn_set_freq_MxL_603
**
**	DESCRIPTION:
**		set the frequency of tuner
**
**	IN:
**		freq	in KHz
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_tn_set_freq_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle, U32 freq, U32 sym_rate, S16 lpf_offset)
{
	MXL_STATUS status;
	UINT8 devId = 0xc0;
	MXL603_CHAN_TUNE_CFG_T chanTuneCfg;


	if(!handle->m_device_c_b.tuner_cfg.tuner_init_ok)
	{
		mt_fe_tn_init_MxL603_cs8800(handle);
	}

	//Step 7 : Channel frequency & bandwidth setting

	chanTuneCfg.freqInHz = freq * 1000;//666000000;
	if(handle->m_device_c_b.demod_type == MtFeType_DVBC)
	{
		chanTuneCfg.signalMode = MXL603_DIG_DVB_C;
		if (handle->m_device_c_b.input_params.demod_bandwidth == MtFeBandwidth_8M)
			chanTuneCfg.bandWidth = MXL603_CABLE_BW_8MHz;
		else if (handle->m_device_c_b.input_params.demod_bandwidth == MtFeBandwidth_7M)
			chanTuneCfg.bandWidth = MXL603_CABLE_BW_7MHz;
		else if (handle->m_device_c_b.input_params.demod_bandwidth == MtFeBandwidth_6M)
			chanTuneCfg.bandWidth = MXL603_CABLE_BW_6MHz;
		else
			chanTuneCfg.bandWidth = MXL603_CABLE_BW_8MHz;
	}
	else if(handle->m_device_c_b.demod_type == MtFeType_J83B)
	{
		chanTuneCfg.signalMode = MXL603_DIG_J83B;
		chanTuneCfg.bandWidth = MXL603_CABLE_BW_6MHz;
	}
	else
	{
		chanTuneCfg.signalMode = MXL603_DIG_DVB_T_DTMB;
		if (handle->m_device_c_b.input_params.demod_bandwidth == MtFeBandwidth_8M)
			chanTuneCfg.bandWidth = MXL603_TERR_BW_8MHz;
		else if (handle->m_device_c_b.input_params.demod_bandwidth == MtFeBandwidth_7M)
			chanTuneCfg.bandWidth = MXL603_TERR_BW_7MHz;
		else if (handle->m_device_c_b.input_params.demod_bandwidth == MtFeBandwidth_6M)
			chanTuneCfg.bandWidth = MXL603_TERR_BW_6MHz;
		else
			chanTuneCfg.bandWidth = MXL603_TERR_BW_8MHz;
	}

	chanTuneCfg.xtalFreqSel = MXL603_XTAL_24MHz;
	chanTuneCfg.startTune = MXL_START_TUNE;
	status = MxLWare603_API_CfgTunerChanTune_cs8800(devId, chanTuneCfg);

	if (status != MXL_SUCCESS)
	{
		return  MtFeErr_Undef;
		// printf("Error! MxLWare603_API_CfgTunerChanTune_cs8800\n");
	}
	else
	{
		// Wait 15 ms
		MxLWare603_OEM_Sleep_cs8800(15);

		return MtFeErr_Ok;
	}
}

/*	FUNCTIN:
**		mt_fe_tn_get_strength_MxL603_cs8800
**
**	DESCRIPTION:
**		get the signal strength
**
**	IN:
**		none.
**
**	OUT:
**		*p_strength	-	signal strength in %
**
**	RETURN:
*/
MT_FE_RET mt_fe_tn_get_strength_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle, S8 *p_strength)
{
	//S8	value = 0;
	S8	strength = 0;
	UINT8 devId = 0xc0;
	SINT16 PwrPtr = 0;
	MXL_BOOL rfLockPtr = MXL_UNLOCKED;
	MXL_BOOL refLockPtr = MXL_UNLOCKED;


	*p_strength = 0;

	//mt_fe_i2c_repeat_enable_cs8800(handle);

	MxLWare603_API_ReqTunerLockStatus_cs8800(devId, &rfLockPtr, &refLockPtr);
	if((rfLockPtr != MXL_LOCKED) && (refLockPtr != MXL_LOCKED))
	{
		//mt_fe_i2c_repeat_disable_cs8800(handle);
		return MtFeErr_Fail;
	}

#if 0
	if (handle->m_device_c_b.demod_type != MtFeType_DVBC)
	{
		MxLWare603_API_ReqTunerRxPower_cs8800(devId, &PwrPtr);
		strength =(S8)(PwrPtr / 100);
		//value = (105+strength)*2;

		//if(value > 100)
		//	value = 100;
		//else if(value < 0)
		//	value = 0;
		*p_strength = strength;//+107;//value;		/* %*/
	}
	else
#endif
	{
		MxLWare603_API_ReqTunerRxPower_cs8800(devId, &PwrPtr);
		strength = (S8)(PwrPtr / 100);
		//value = (105+strength)*2;

		//if(value > 100)
		//	value = 100;
		//else if(value < 0)
		//	value = 0;
		*p_strength = strength;//value;		/* %*/
	}

	//mt_fe_i2c_repeat_disable_cs8800(handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_sleep_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MXL_STATUS status;
	UINT8 devId = 0xc0;
	//MXL603_CHAN_TUNE_CFG_T chanTuneCfg;

	devId = handle->m_device_c_b.tuner_cfg.tuner_dev_addr;

	//mt_fe_i2c_repeat_enable_cs8800(handle);
	status = MxLWare603_API_CfgDevPowerMode_cs8800(devId, MXL603_PWR_MODE_STANDBY, MXL_DISABLE, 0);
	//mt_fe_i2c_repeat_disable_cs8800(handle);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_wake_up_MxL603_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MXL_STATUS status;
	UINT8 devId = 0xc0;
	//MXL603_CHAN_TUNE_CFG_T chanTuneCfg;

	//mt_fe_i2c_repeat_enable_cs8800(handle);
	status = MxLWare603_API_CfgDevPowerMode_cs8800(devId, MXL603_PWR_MODE_ACTIVE, MXL_DISABLE, 0);
	//mt_fe_i2c_repeat_disable_cs8800(handle);

	return MtFeErr_Ok;
}


// fix 105318 start
extern MT_FE_CS8800_Device_Handle mt_fe_cs8800_get_handle(void);

void mt_fe_cs8800_tn_attach_MxL603(void)
{
	MT_FE_CS8800_Device_Handle handle = mt_fe_cs8800_get_handle();

	handle->m_device_c_b.tuner_cfg.tuner_type = MtFeTN_MxL603;

	if (handle->m_device_c_b.tuner_cfg.tuner_dev_addr == 0x00)
		handle->m_device_c_b.tuner_cfg.tuner_dev_addr = 0xc0;

	handle->m_device_c_b.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_c_b.tuner_cfg.tuner_open = 0;
	handle->m_device_c_b.tuner_cfg.tuner_lna_type = 0;

	handle->m_device_c_b.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_MxL603_cs8800;
	handle->m_device_c_b.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_MxL603_cs8800;
	handle->m_device_c_b.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_MxL603_cs8800;
	handle->m_device_c_b.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603_cs8800;
	handle->m_device_c_b.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_MxL603_cs8800;

	handle->m_device_c_b.tuner_cfg.tuner_get_offset = NULL;
	handle->m_device_c_b.tuner_cfg.tuner_get_gain = NULL;
	handle->m_device_c_b.tuner_cfg.tuner_set_application = NULL;
	handle->m_device_c_b.tuner_cfg.tuner_set_loop_through = NULL;
	handle->m_device_c_b.tuner_cfg.tuner_set_xtal = NULL;
	handle->m_device_c_b.tuner_cfg.tuner_set_clkout = NULL;
	handle->m_device_c_b.tuner_cfg.tuner_get_diagnose_info = NULL;

	return;
}
// fix 105318 end


