/********************************************************************************************/
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/***************************************************************************/
/* MONTAGE PROPRIETARY AND CONFIDENTIAL                                    */
/* Montage Technology (Shanghai) Co., Ltd                                  */
/* Copyright (C) 2020                                                      */
/* Montage Technology Group Limited and/or its affiliated companies        */
/* All Rights Reserved                                                     */
/***************************************************************************/
/*
* File:				mt_fe_dmd_cs8800.c
*
* Current version:	00.22
*
* Description:		Symphony4 IC Driver.
*
* Log:
*	Description		Version		Date			Author
*--------------------------------------------------------------------------
*	Create			00.01		2018.06.18		YZ.Huang
*	Modify			00.01		2018.06.26		YZ.Huang
*	Modify			00.04		2018.09.05		YZ.Huang
*	Modify			00.05		2018.09.25		YZ.Huang
*	Modify			00.06		2018.10.17		YZ.Huang
*	Modify			00.07		2018.12.11		YZ.Huang
*	Modify			00.09		2019.01.11		YZ.Huang
*	Modify			00.10		2019.01.16		YZ.Huang
*	Modify			00.11		2019.02.25		YZ.Huang
*	Modify			00.12		2019.03.13		YZ.Huang
*	Modify			00.13		2019.03.14		YZ.Huang
*	Modify			00.14		2019.07.23		YZ.Huang
*	Modify			00.15		2019.08.19		YZ.Huang
*	Modify			00.16		2019.11.08		YZ.Huang
*	Modify			00.17		2020.04.20		YZ.Huang
*	Modify			00.18		2020.09.30		YZ.Huang
*	Modify			00.19		2021.08.10		YZ.Huang
*	Modify			00.20		2022.01.07		YZ.Huang
*	Modify			00.21		2022.01.14		YZ.Huang
*	Modify			00.22		2022.02.22		YZ.Huang
***************************************************************************************************************/
#include "mt_fe_def.h"
#include "mt_fe_i2c_cs8800.h"

#if MT_FE_DMD_DVBC_SUPPORT
#include "mt_fe_dmd_cs8800_C.h"
#endif

#if MT_FE_DMD_J83B_SUPPORT
#include "mt_fe_dmd_cs8800_B.h"
#endif

#if MT_FE_DMD_DVBS_S2_SUPPORT
#include "mt_fe_dmd_cs8800_S_S2.h"
#endif
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#include <linux/printk.h>
#else //(__RTOS__)
#define CONFIG_MT_FRONTEND_DMD_CS8800_TN_TS6011    1
#define CONFIG_MT_FRONTEND_DMD_CS8800_TN_RT720     1
#define CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL603    1
#define CONFIG_MT_FRONTEND_DMD_CS8800_TN_TC6800    1
extern int mt_dbg_printf(unsigned int level, const char *p_fmt, ...);
#endif //defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)

MT_BOOL	g_bNeedReCali = TRUE;

/*	FUNCTION:
**		mt_fe_get_driver_version_cs8800
**
**	DESCRIPTION:
**		get version of this driver
**
**	IN:
**		none
**
**	OUT:
**		*pd_ver
**
**	RETURN:
*/
MT_FE_RET mt_fe_get_driver_version_cs8800(U8 *pd_ver)
{
	MT_FE_RET ret = MtFeErr_Ok;

	#if MT_FE_DMD_DVBS_S2_SUPPORT
		mt_fe_dmd_get_driver_version_cs8800_ss2(&pd_ver[0]);/*s/s2 driver software's version*/
	#else
		pd_ver[0] = 0;
	#endif

	#if MT_FE_DMD_DVBC_SUPPORT
		mt_fe_dmd_get_driver_version_cs8800_c(&pd_ver[1]);/*c driver software's version*/
	#else
		pd_ver[1] = 0;
	#endif

	#if MT_FE_DMD_J83B_SUPPORT
		mt_fe_dmd_get_driver_version_cs8800_b(&pd_ver[2]);/*b driver software's version*/
	#else
		pd_ver[2] = 0;
	#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_cs8800_config_default(MT_FE_CS8800_Device_Handle handle)
{
	handle->sar_dev_addr = 0x80;
	handle->dvbc_dev_addr = 0x38;
	handle->j83b_dev_addr = 0xb8;
	handle->dvbs_dev_addr = 0xd0;

	handle->sys_dev_xtal = MtFeXTALMode_27M;

	handle->bSysInitOk	 = FALSE;

	//g_bNeedReCali		 = TRUE;

	//printk("%s[%d] ---- handle[%08x], g_bNeedReCali = [%d]\n", __FUNCTION__, __LINE__, handle, g_bNeedReCali);

	mt_fe_dmd_config_default_c_b_cs8800(handle);

#if MT_FE_DMD_DVBS_S2_SUPPORT
	mt_fe_dmd_config_default_ss2_cs8800(handle);
#endif

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_calibration_cs8800_c_b(MT_FE_CS8800_Device_Handle handle, U8 iMaxTimes)
{
	U8 tmp1=0, tmp2=0, regv_35 = 0,regv_39 = 0;
	U8 i, reg_03=0;
	U32 reg = 0;
	U32 ulTmp1=0;
	static U32 calibration_val[2] = {0};
	U8 cal_Times=iMaxTimes;
#if 1
	if(calibration_val[0] != 0 && calibration_val[1] !=0)
	{
		cal_Times = 1;               //use saved cal reg
	}
	
	for (i = 0; i < cal_Times; i++)
	{
		//BF157000[20]=1;power down cadc  
		//wait 5ms			
		//BF157000[20]=0;power down cadc  
		if(calibration_val[0] == 0){
			handle->Get32Bits(0xBF157000, &reg);
			calibration_val[0] = reg;
			reg |= (1 << 20);
			handle->Set32Bits(0xBF157000, reg);
			_mt_delay_cs8800(5); //wait 5ms

			reg &= ~(1 << 20);
			handle->Set32Bits(0xBF157000, reg);	
		}
		else
		{
			reg =  calibration_val[0];
			reg |= (1 << 20);
			handle->Set32Bits(0xBF157000, reg);
			_mt_delay_cs8800(5); //wait 5ms

			reg &= ~(1 << 20);
			handle->Set32Bits(0xBF157000, reg);		
		}
		// CADC delay cell cal for 54M
		if(calibration_val[1] == 0){
			handle->Get32Bits(0xBF5D00D0, &ulTmp1);
			calibration_val[1] = ulTmp1;
			//BF5D00D0H[28 : 27] = _00
			//BF5D00D0H[22] = 0
			ulTmp1 &= 0xE7BFFFFF;
			handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0
			//wait 1ms
			_mt_delay_cs8800(1);

			//BF5D00D0H[22] = 1
			ulTmp1 |= 0x00400000;
			handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0
			
		}
		else
		{
			ulTmp1 = calibration_val[1];
			ulTmp1 &= 0xE7BFFFFF;
			handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0
			//wait 1ms
			_mt_delay_cs8800(1);

			//BF5D00D0H[22] = 1
			ulTmp1 |= 0x00400000;
			handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0
		}
		_mt_fe_sar_get_reg_cs8800(handle, 0x03, &reg_03);
		reg_03 |= 0x80;
		_mt_fe_sar_set_reg_cs8800(handle, 0x03, reg_03);
		_mt_delay_cs8800(1);
		reg_03 &= 0x7F;
		_mt_fe_sar_set_reg_cs8800(handle, 0x03, reg_03);
		
		_mt_fe_sar_get_reg_cs8800(handle, 0x32, &tmp1);
		tmp1 |= 0x53;//53//5f
		_mt_fe_sar_set_reg_cs8800(handle, 0x32, tmp1); //set clear lms_lock log
		
		_mt_fe_sar_get_reg_cs8800(handle, 0x33, &tmp1);
		tmp1 |= 0x40;//0x43;//42-div2//43-div4//4b-div8//40-div1
		_mt_fe_sar_set_reg_cs8800(handle, 0x33, tmp1); //enable ext control & clock div4

		_mt_fe_sar_get_reg_cs8800(handle, 0x34, &tmp1);
		tmp1 |= 0x40;//43//40
		_mt_fe_sar_set_reg_cs8800(handle, 0x34, tmp1); //enable ext control

		_mt_fe_sar_set_reg_cs8800(handle, 0x40, 0x71); //enable ext control

		_mt_fe_sar_get_reg_cs8800(handle, 0x31, &tmp1);
		tmp1 |= 0x0F;
		_mt_fe_sar_set_reg_cs8800(handle, 0x31, tmp1); //enable calibration & ext control

		_mt_delay_cs8800(2); //wait 2ms

		_mt_fe_sar_get_reg_cs8800(handle, 0x30, &tmp2);
		tmp2 |= 0x08;
		_mt_fe_sar_set_reg_cs8800(handle, 0x30, tmp2); //reset calibration

		_mt_delay_cs8800(5);//wait 5ms

		_mt_fe_sar_get_reg_cs8800(handle, 0x32, &tmp1);
		tmp1 &= 0x3F;
		_mt_fe_sar_set_reg_cs8800(handle, 0x32, tmp1);//release clear lms_lock log

		tmp2 &= 0xF0;
		_mt_fe_sar_set_reg_cs8800(handle, 0x30, tmp2);//release calibration

		_mt_delay_cs8800(10); //wait 10ms

#if 0
		_mt_fe_sar_get_reg_cs8800(handle, 0x39, &tmp3);  //judge if calibration is success
		if ((tmp3 >= 0x29) && (tmp3 <= 0x2F))  //after calibration, reg 0x39 should be 0x29 - 0x2F
			break;
#else
		_mt_fe_sar_get_reg_cs8800(handle, 0x36, &tmp2);
		tmp2 |= 0x02;
		_mt_fe_sar_set_reg_cs8800(handle, 0x36, tmp2);

		_mt_fe_sar_get_reg_cs8800(handle, 0x35, &regv_35);
                _mt_fe_sar_get_reg_cs8800(handle, 0x39, &regv_39);

		//printk("%s[%d] ---- CS8800 C & B calibration result -- 0x35 = [0x%02x],0x39 = [0x%02x], i = %d\n", __FUNCTION__, __LINE__, regv_35,regv_39,i);

		tmp2 &= 0xFD;
		_mt_fe_sar_set_reg_cs8800(handle, 0x36, tmp2);

		_mt_fe_sar_get_reg_cs8800(handle, 0x31, &tmp1);
		tmp1 &= 0x00;
		_mt_fe_sar_set_reg_cs8800(handle, 0x31, tmp1); //disable calibration

		if ((regv_35 >= 0xC7) && (regv_35 <= 0xC9) && (regv_39 >= 0x29) && (regv_39 <= 0x2F)) 
			break;
#endif
	}

	if ((regv_35 >= 0xC7) && (regv_35 <= 0xC9) && (regv_39 >= 0x29) && (regv_39 <= 0x2F))
	{
		g_bNeedReCali		 = FALSE;

		mt_fe_print(("\tCS8800 C & B calibration OK!\n"));
		printk("%s[%d] ---- CS8800 C & B calibration OK! 0x35 = [0x%02x],0x39 = [0x%02x] \n", __FUNCTION__, __LINE__,regv_35,regv_39);
	}
	else
	{
		mt_fe_print(("\tCS8800 C & B calibration failed!\n"));
	        printk("%s[%d] ---- CS8800 C & B calibration failed! 0x35 = [0x%02x],0x39 = [0x%02x]\n", __FUNCTION__, __LINE__,regv_35,regv_39);
	}


	//printk("%s[%d] ---- handle[%08x], g_bNeedReCali = [%d]\n", __FUNCTION__, __LINE__, handle, g_bNeedReCali);

#else
	_mt_fe_sar_get_reg_cs8800(handle, 0x32, &tmp1);
	tmp1 |= 0x53;//53//5f
	_mt_fe_sar_set_reg_cs8800(handle, 0x32, tmp1); //set clear lms_lock log

	_mt_fe_sar_get_reg_cs8800(handle, 0x33, &tmp1);
	tmp1 |= 0x42;//0x43;//42-div2//43-div4//4b-div8
	_mt_fe_sar_set_reg_cs8800(handle, 0x33, tmp1); //enable ext control & clock div4

	_mt_fe_sar_get_reg_cs8800(handle, 0x34, &tmp1);
	tmp1 |= 0x40;//43//40
	_mt_fe_sar_set_reg_cs8800(handle, 0x34, tmp1); //enable ext control

	_mt_fe_sar_set_reg_cs8800(handle, 0x40, 0x71); //enable ext control

	_mt_fe_sar_get_reg_cs8800(handle, 0x31, &tmp1);
	tmp1 |= 0x0F;
	_mt_fe_sar_set_reg_cs8800(handle, 0x31, tmp1); //enable calibration & ext control

	_mt_delay_cs8800(2); //wait 2ms

	for (i = 0; i < iMaxTimes; i++)
	{
		_mt_fe_sar_get_reg_cs8800(handle, 0x30, &tmp2);
		tmp2 |= 0x08;
		_mt_fe_sar_set_reg_cs8800(handle, 0x30, tmp2); //reset calibration

		_mt_delay_cs8800(5);//wait 5ms

		_mt_fe_sar_get_reg_cs8800(handle, 0x32, &tmp1);
		tmp1 &= 0x3F;
		_mt_fe_sar_set_reg_cs8800(handle, 0x32, tmp1);//release clear lms_lock log

		tmp2 &= 0xF0;
		_mt_fe_sar_set_reg_cs8800(handle, 0x30, tmp2);//release calibration

		_mt_delay_cs8800(10); //wait 10ms

#if 0
		_mt_fe_sar_get_reg_cs8800(handle, 0x39, &tmp3);  //judge if calibration is success
		if ((tmp3 >= 0x29) && (tmp3 <= 0x2F))  //after calibration, reg 0x39 should be 0x29 - 0x2F
			break;
#else
		_mt_fe_sar_get_reg_cs8800(handle, 0x36, &tmp2);
		tmp2 |= 0x02;
		_mt_fe_sar_set_reg_cs8800(handle, 0x36, tmp2);

		//_mt_fe_sar_get_reg_cs8800(handle, 0x35, &tmp1);
		_mt_fe_sar_get_reg_cs8800(handle, 0x35, &tmp3);

		tmp2 &= 0xFD;
		_mt_fe_sar_set_reg_cs8800(handle, 0x36, tmp2);

		if ((tmp3 >= 0xC7) && (tmp3 <= 0xC9))  //after calibration, reg 0x35 should be 0xC7 - 0xC9
			break;
#endif
	}

#if 0
	if ((tmp3 >= 0x29) && (tmp3 <= 0x2F))
	{
		g_bNeedReCali		 = FALSE;

		mt_fe_print(("\tCS8800 C & B calibration OK!\n"));
		printk("%s[%d] ---- CS8800 C & B calibration OK! 0x39 = [0x%02x]\n", __FUNCTION__, __LINE__, tmp3);
	}
	else
	{
		mt_fe_print(("\tCS8800 C & B calibration failed!\n"));
		printk("%s[%d] ---- CS8800 C & B calibration failed! 0x39 = [0x%02x]\n", __FUNCTION__, __LINE__, tmp3);
	}
#else
	if ((tmp3 >= 0xC7) && (tmp3 <= 0xC9))
	{
		g_bNeedReCali		 = FALSE;

		mt_fe_print(("\tCS8800 C & B calibration OK!\n"));
		printk("%s[%d] ---- CS8800 C & B calibration OK! 0x35 = [0x%02x], i = %d\n", __FUNCTION__, __LINE__, tmp3, i);
	}
	else
	{
		mt_fe_print(("\tCS8800 C & B calibration failed!\n"));
		printk("%s[%d] ---- CS8800 C & B calibration failed! 0x35 = [0x%02x], i = %d\n", __FUNCTION__, __LINE__, tmp3, i);
	}
#endif

	//printk("%s[%d] ---- handle[%08x], g_bNeedReCali = [%d]\n", __FUNCTION__, __LINE__, handle, g_bNeedReCali);

	_mt_fe_sar_get_reg_cs8800(handle, 0x31, &tmp1);
	tmp1 &= 0x00;
	_mt_fe_sar_set_reg_cs8800(handle, 0x31, tmp1); //disable calibration
#endif

	return MtFeErr_Ok;
}

MT_FE_RET _mt_fe_dmd_re_calibration_cs8800_c_b(MT_FE_CS8800_Device_Handle handle, U8 *p_buf)
{
	U8 tmp1 = 0, i = 0, tmp2 = 0;

	/*Read Registers*/
	_mt_sleep_cs8800(40); //wait 40ms

	for (i = 0x39; i <= 0x3F; i++)
	{
		_mt_fe_sar_get_reg_cs8800(handle, i, &tmp1);
	}

	_mt_fe_sar_get_reg_cs8800(handle, 0x36, &tmp2);
	tmp2 |= 0x02;
	_mt_fe_sar_set_reg_cs8800(handle, 0x36, tmp2);

	printk("\t re Calibration weight:\n");

	for (i = 0; i < 28; i++)
	{
		//_mt_fe_sar_get_reg_cs8800(handle, 0x35, &tmp1);
		_mt_fe_sar_get_reg_cs8800(handle, 0x35, &p_buf[i]);
		//printk("\t%02x\n", p_buf[i]);
		//if ((i % 14) == 13)   printk("\n");
	}

	tmp2 &= 0xFD;
	_mt_fe_sar_set_reg_cs8800(handle, 0x36, tmp2);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_calibration_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	U8 data = 0, tmp = 0;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	U32 reg1 = 0, reg2 = 0, reg3 = 0, reg4 = 0, reg = 0;
#else
	U32 reg1 = 0, reg2 = 0, reg3 = 0, reg4 = 0, reg = 0;
#endif

	U32 ulTmp1;

	if (handle == NULL)
	{
		return MtFeErr_Uninit;
	}

	mt_fe_dmd_config_clock_cs8800(handle, MtFeType_J83B);	// config J83B clock, 20231221


#if 1
	// CADC delay cell cal for 54M
	handle->Get32Bits(0xBF5D00D0, &ulTmp1);
	//BF5D00D0H[28 : 27] = _00
	//BF5D00D0H[22] = 0
	ulTmp1 &= 0xE7BFFFFF;
	handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0

	//wait 1ms
	_mt_delay_cs8800(1);

	//BF5D00D0H[22] = 1
	ulTmp1 |= 0x00400000;
	handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0
#endif

	_mt_fe_sar_get_reg_cs8800(handle, 0x04, &tmp);
	_mt_fe_sar_set_reg_cs8800(handle, 0x04, 0x05);
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#if defined(CONFIG_MT_CHIP_SYMPHONY4)|| defined(CONFIG_MT_CHIP_SYMPHONY6)
#endif
	if ((handle->chip_version == 0xA000) || (handle->chip_version == 0xA001)) // Sym4 A0/A1
	{
		handle->Get32Bits(0xBF5D00D0, &reg1);
		// bit[19] = 0
		//reg1 &= 0xFFF7F8FF;
		reg1 |= 0x00000400;
		handle->Set32Bits(0xBF5D00D0, reg1);		//+++ ANA: CADC_REG0

		//w BF5D0120 0x801F_001F (calibration clock)
		//w BF5D0124 0x1FA0_1F76 (calibration clock)
		//w BF5D0128 0x_0700 (calibration clock)
		//w BF5D0098 0xF000 081F(calibration clock)

		handle->Get32Bits(0xBF5D0120, &reg1);
		handle->Get32Bits(0xBF5D0124, &reg2);
		handle->Get32Bits(0xBF5D0128, &reg3);
		handle->Get32Bits(0xBF5D0098, &reg4);

		handle->Set32Bits(0xBF5D0120, 0x801F001F);	//+++ ANA: CLKGEN_USBACLK_REG0
		handle->Set32Bits(0xBF5D0124, 0x1FA01F76);	//+++ ANA: CLKGEN_USBACLK_REG1
		handle->Set32Bits(0xBF5D0128, 0x00000700);	//+++ ANA: CLKGEN_USBACLK_REG2
		handle->Set32Bits(0xBF5D0098, 0xF000081F);	//+++ ANA: CLKGEN_TEST_SW_REG
	}
	else if (handle->chip_version == 0xB000) // Sym6 A0
	{
		handle->Get32Bits(0xBF157004, &reg);
                if ((reg & 0x40000000) == 0x40000000)   // bit 30 = 1, USB_PLL_PI original status = power down
                {
                        handle->Set32Bits(0xBF157004, reg & 0xBFFFFFFF);        // bit 30 = 0, power on USB_PLL_PI
                }
                printk("[%s : %d]===== > A0\n",__FUNCTION__,__LINE__);
                handle->Get32Bits(0xBF5D00D0, &reg1);
                // bit[10:8] = _100
                // bit[19] = 0
                reg1 &= 0xFFF7F8FF;
                reg1 |= 0x00000400;
                handle->Set32Bits(0xBF5D00D0, reg1);

                handle->Get32Bits(0xBF5D004C, &reg1);
                handle->Get32Bits(0xBF5D0098, &reg2);
                handle->Get32Bits(0xBF5D011C, &reg3);

                handle->Set32Bits(0xBF5D004C, (reg1 & 0xFFFFFCFF) | 0x00000100);
                handle->Set32Bits(0xBF5D0098, 0xF000283F);//good
                //handle->Set32Bits(0xBF5D011C, ((reg3 & 0x0000FFFF) | 0x84000000));
                handle->Set32Bits(0xBF5D011C, ((reg3 & 0x0000FFFF) | 0x86000000));

	}
	else if (handle->chip_version == 0xB001) // Sym6 A1
	{
		handle->Get32Bits(0xBF157004, &reg);
		if ((reg & 0x40000000) == 0x40000000)	// bit 30 = 1, USB_PLL_PI original status = power down
		{
			handle->Set32Bits(0xBF157004, reg & 0xBFFFFFFF);	// bit 30 = 0, power on USB_PLL_PI
		}
		printk("[%s : %d]===== > A1\n",__FUNCTION__,__LINE__);
		handle->Get32Bits(0xBF5D00D0, &reg1);
		// bit[10:8] = _100
		// bit[19] = 0
		// bit[3] = 1     ??  
		reg1 &= 0xFFF7F8FF;
		reg1 |= 0x00000400;
		handle->Set32Bits(0xBF5D00D0, reg1);

		//add on 20240402 for sym6 A1
		handle->Get32Bits(0xBF5D013C, &reg1);
		handle->Set32Bits(0xBF5D013C, (reg1 & 0xC7FFFFFF) | 0x20000000);   //bit[29:27] = b100

		handle->Get32Bits(0xBF5D004C, &reg1);
		handle->Get32Bits(0xBF5D0098, &reg2);
		handle->Get32Bits(0xBF5D0128, &reg3);

		handle->Set32Bits(0xBF5D004C,  reg1  | 0x00000300);   //bit[9:8]=b00---0xf000083f   bit[9:8]=b11---0xF000081F
		//handle->Set32Bits(0xBF5D004C,  ((reg1 | 0x00000c00 ) & 0xfffffcff));
		handle->Set32Bits(0xBF5D0098, 0xF000081F);//good    
		//handle->Set32Bits(0xBF5D0098, 0xF000083F);//good    
		handle->Set32Bits(0xBF5D0128, ((reg3 & 0xFFFF0000) | 0x00001FA1));
		if(1)
		{
			U32 reg[3] = {0};
			handle->Get32Bits(0xBF5D004C, &reg[0]);
			handle->Get32Bits(0xBF5D0098, &reg[1]);
			handle->Get32Bits(0xBF5D0128, &reg[2]);
			printk("[%s : %d]===== > 0xBF5D004c = 0x%x, 0x%x, 0x%x\n",
				__FUNCTION__,__LINE__,reg[0],reg[1],reg[2]);
		}
	 }
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#endif	
#endif //defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)

	_mt_sleep_cs8800(10);

	_mt_fe_sar_get_reg_cs8800(handle, 0x0A, &data);
	data &= 0xFB;		// 2V
	if (handle->m_device_c_b.board_cfg.iVppSel == 1)		// 1V
	{
		data |= 0x04;
	}
	_mt_fe_sar_set_reg_cs8800(handle, 0x0A, data);  //Set 1V or 2V Vdpp


	//ADC calibration in demod register(release by Huang You Zhong)
	if (g_bNeedReCali)
		_mt_fe_dmd_calibration_cs8800_c_b(handle, 20);	//5

	if (handle->m_device_c_b.board_cfg.bReadCali)
	{
		_mt_fe_dmd_re_calibration_cs8800_c_b(handle, handle->m_device_c_b.calibration_data);
	}

	//printk("[%s : %d] ---- handle[%08x], g_bNeedReCali = [%d]\n", __FUNCTION__, __LINE__, handle, g_bNeedReCali);
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#if defined(CONFIG_MT_CHIP_SYMPHONY4)|| defined(CONFIG_MT_CHIP_SYMPHONY6)
#endif //defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
	if ((handle->chip_version == 0xA000) || (handle->chip_version == 0xA001)) // Sym4 A0/A1
	{
		//w BF5D0120 preset value (calibration clock change back)
		//w BF5D0124 preset value  (calibration clock change back)
		//w BF5D0128 preset value (calibration clock change back)
		//w BF5D0098 preset value (calibration clock change back)

		handle->Set32Bits(0xBF5D0120, reg1);	//+++ ANA: CLKGEN_USBACLK_REG0
		handle->Set32Bits(0xBF5D0124, reg2);	//+++ ANA: CLKGEN_USBACLK_REG1
		handle->Set32Bits(0xBF5D0128, reg3);	//+++ ANA: CLKGEN_USBACLK_REG2
		handle->Set32Bits(0xBF5D0098, reg4);	//+++ ANA: CLKGEN_TEST_SW_REG
	}
	else if (handle->chip_version == 0xB000) // Sym6 A0
	{
		//w BF5D0120 preset value (calibration clock change back)
		//w BF5D0124 preset value  (calibration clock change back)
		//w BF5D0128 preset value (calibration clock change back)
		//w BF5D0098 preset value (calibration clock change back)

		handle->Set32Bits(0xBF5D004C, reg1);
		handle->Set32Bits(0xBF5D0098, reg2);
		handle->Set32Bits(0xBF5D011C, reg3);

		if ((reg & 0x40000000) == 0x40000000)	// bit 30 = 1, USB_PLL_PI original status = power down
		{
			handle->Set32Bits(0xBF157004, reg | 0x40000000);	// bit 30 = 1, power down USB_PLL_PI
		}
	}
	else if (handle->chip_version == 0xB001) // Sym6 A1
	{
		//w BF5D0120 preset value (calibration clock change back)
		//w BF5D0124 preset value  (calibration clock change back)
		//w BF5D0128 preset value (calibration clock change back)
		//w BF5D0098 preset value (calibration clock change back)


		handle->Set32Bits(0xBF5D004C, reg1);
		handle->Set32Bits(0xBF5D0098, reg2);
		handle->Set32Bits(0xBF5D0128, reg3);

		if ((reg & 0x40000000) == 0x40000000)	// bit 30 = 1, USB_PLL_PI original status = power down
		{
			handle->Set32Bits(0xBF157004, reg | 0x40000000);	// bit 30 = 1, power down USB_PLL_PI
		}
	}
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
#endif
#endif //defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)

	_mt_fe_sar_set_reg_cs8800(handle, 0x04, tmp);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_config_clock_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type)
{
	U32 ulTmp = 0;

	if ((MtFeType_DVBC == handle->m_device_c_b.demod_current_type) && 
		((MtFeType_DVBS == handle->m_device_ss2.demod_current_type) || 
		 (MtFeType_DVBS2 == handle->m_device_ss2.demod_current_type) || 
		 (MtFeType_DVBS_S2 == handle->m_device_ss2.demod_current_type)))//it means DVBC and DVBS coexit at same time
	{
		if (handle->m_device_ss2.tp_cfg.iSymRateKSs <= 46000)
		{
			//printk(KERN_ERR "[%s %d]config clock for sym <= 46000\n", __FUNCTION__, __LINE__);
			handle->m_device_ss2.global_cfg.iMclkKHz = 96000;

			handle->Set32Bits(0xBF5D004C, 0x0EA1BB0C);		//+++ ANA: CLKGEN_ETHERNET_SW_REG0
			handle->Set32Bits(0xBF5D0048, 0x1C3A9009);		//+++ ANA: CLKGEN_CPUPLL_SW_REG0

#if 1 // 20210727
			handle->Get32Bits(0xBF157004, &ulTmp);
			if ((ulTmp & 0x80000000) == 0)
			{
				ulTmp |= (1 << 31);
				handle->Set32Bits(0xBF157004, ulTmp);			//+++ ANA: ANA_AO_REG1
			}
#endif

			_mt_fe_cs8800_set_reg(handle->dvbs_dev_addr, 0xa0, 0x44);
		}
		else
		{
			//printk(KERN_ERR "[%s %d]config clock for sym > 46000\n", __FUNCTION__, __LINE__);
			handle->m_device_ss2.global_cfg.iMclkKHz = 135000;

			handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);		//+++ ANA: CLKGEN_ETHERNET_SW_REG0
			handle->Set32Bits(0xBF5D0048, 0x1C3A9009);		//+++ ANA: CLKGEN_CPUPLL_SW_REG0

			handle->Get32Bits(0xBF157004, &ulTmp);
			if (ulTmp & 0x80000000)
			{
				ulTmp &= ~(1 << 31);
				handle->Set32Bits(0xBF157004, ulTmp);			//+++ ANA: ANA_AO_REG1
			}

			//_mt_fe_cs8800_set_reg(handle->dvbs_dev_addr, 0xa0, 0x44);
			_mt_fe_cs8800_set_reg(handle->dvbs_dev_addr, 0xa0, 0x60);
		}
		if((demod_type == MtFeType_DVBC))
		{
			handle->Set32Bits(0xBF5D004C, 0x0EA1BB0C);		//+++ ANA: CLKGEN_ETHERNET_SW_REG0
			handle->Set32Bits(0xBF5D0048, 0x1C3A9009);		//+++ ANA: CLKGEN_CPUPLL_SW_REG0

#if 1 // 20210727
			handle->Get32Bits(0xBF157004, &ulTmp);
			if ((ulTmp & 0x80000000) == 0)
			{
				ulTmp |= (1 << 31);
				handle->Set32Bits(0xBF157004, ulTmp);			//+++ ANA: ANA_AO_REG1
			}
#endif
		}
		return MtFeErr_Ok;
	}

	if (demod_type == MtFeType_J83B)
	{
		handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);	//+++ ANA: CLKGEN_ETHERNET_SW_REG0
		handle->Set32Bits(0xBF5D0048, 0x1C4CB009);	//+++ ANA: CLKGEN_CPUPLL_SW_REG0

		handle->Get32Bits(0xBF157004, &ulTmp);
		if (ulTmp & 0x80000000)
		{
			ulTmp &= ~(1 << 31);
			handle->Set32Bits(0xBF157004, ulTmp);	//+++ ANA: ANA_AO_REG1
		}
	}
	else if ((demod_type == MtFeType_DVBS) || 
			(demod_type == MtFeType_DVBS2) || 
			(demod_type == MtFeType_DVBS_S2))
	{
		//BF5D004CH	clkgen_ethintp_reg	_0x0EA1 bb 0c	_0x0EA1 bb1E	_0x0EA1 bb1E
		//BF5D0048H	clkgen_cpupll_reg	_0x1c 3a 90 b9	_0x1c 3a 90 b9	_0x5c 28 b0 b9

		if (handle->m_device_ss2.tp_cfg.iSymRateKSs > 53000)
		{
			handle->m_device_ss2.global_cfg.iMclkKHz = 135000;	// 270000;

			handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);	//+++ ANA: CLKGEN_ETHERNET_SW_REG0
			handle->Set32Bits(0xBF5D0048, 0x1C3A9009);	//+++ ANA: CLKGEN_CPUPLL_SW_REG0

			handle->Get32Bits(0xBF157004, &ulTmp);
			if (ulTmp & 0x80000000)
			{
				ulTmp &= ~(1 << 31);
				handle->Set32Bits(0xBF157004, ulTmp);	//+++ ANA: ANA_AO_REG1
			}

			_mt_fe_cs8800_set_reg(handle->dvbs_dev_addr, 0xa0, 0x60);
		}
		else if (handle->m_device_ss2.tp_cfg.iSymRateKSs > 46000)
		{
			handle->m_device_ss2.global_cfg.iMclkKHz = 108000;	//216000;

			handle->Set32Bits(0xBF5D004C, 0x0EA1BB1E);	//+++ ANA: CLKGEN_ETHERNET_SW_REG0
			handle->Set32Bits(0xBF5D0048, 0x5C289009);	//+++ ANA: CLKGEN_CPUPLL_SW_REG0

			handle->Get32Bits(0xBF157004, &ulTmp);		//+++ ANA: ANA_AO_REG1
			if (ulTmp & 0x80000000)
			{
				ulTmp &= ~(1 << 31);
				handle->Set32Bits(0xBF157004, ulTmp);
			}

			_mt_fe_cs8800_set_reg(handle->dvbs_dev_addr, 0xa0, 0x4c);
		}
		else
		{
			handle->m_device_ss2.global_cfg.iMclkKHz = 96000;	// 192000

			handle->Set32Bits(0xBF5D004C, 0x0EA1BB0C);		//+++ ANA: CLKGEN_ETHERNET_SW_REG0
			handle->Set32Bits(0xBF5D0048, 0x1C3A9009);		//+++ ANA: CLKGEN_CPUPLL_SW_REG0

#if 1 // 20210727
			handle->Get32Bits(0xBF157004, &ulTmp);
			if ((ulTmp & 0x80000000) == 0)
			{
				ulTmp |= (1 << 31);
				handle->Set32Bits(0xBF157004, ulTmp);			//+++ ANA: ANA_AO_REG1
			}
#endif

			_mt_fe_cs8800_set_reg(handle->dvbs_dev_addr, 0xa0, 0x44);
		}
	}
	else	// DVB-C
	{
		handle->Set32Bits(0xBF5D004C, 0x0EA1BB0C);		//+++ ANA: CLKGEN_ETHERNET_SW_REG0
		handle->Set32Bits(0xBF5D0048, 0x1C3A9009);		//+++ ANA: CLKGEN_CPUPLL_SW_REG0

#if 1 // 20210727
		handle->Get32Bits(0xBF157004, &ulTmp);
		if ((ulTmp & 0x80000000) == 0)
		{
			ulTmp |= (1 << 31);
			handle->Set32Bits(0xBF157004, ulTmp);			//+++ ANA: ANA_AO_REG1
		}
#endif
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_config_adc_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type)
{
	U32 ulTmp = 0;

	if ((MtFeType_DVBC == handle->m_device_c_b.demod_current_type) && 
		((MtFeType_DVBS == handle->m_device_ss2.demod_current_type) || 
		 (MtFeType_DVBS2 == handle->m_device_ss2.demod_current_type) || 
		 (MtFeType_DVBS_S2 == handle->m_device_ss2.demod_current_type)))//it means DVBC and DVBS coexit at same time
	{
		handle->Get32Bits(0xBF157000, &ulTmp);
		ulTmp &= (~((1 << 21) | (1 << 20)));	// bit21(S_ADC), 0:on, 1:off, bit20(C_ADC), 0:on, 1:off
		handle->Set32Bits(0xBF157000, ulTmp);

		return MtFeErr_Ok;
	}


	if ((demod_type == MtFeType_DVBS) || 
		(demod_type == MtFeType_DVBS2) || 
		(demod_type == MtFeType_DIRECTV) || 
		(demod_type == MtFeType_DVBS_S2))
	{
		handle->Get32Bits(0xBF157000, &ulTmp);
		ulTmp &= (~(1 << 21));//S_ADC on
		if (MtFeType_DVBC == handle->m_device_c_b.demod_current_type)
			ulTmp &= (~(1 << 20));	// C_ADC on
		else
			ulTmp |= (1 << 20);		// C_ADC off
		handle->Set32Bits(0xBF157000, ulTmp);
	}
	else if (demod_type == MtFeType_DVBC)	// DVB-C
	{
		handle->Get32Bits(0xBF157000, &ulTmp);
		ulTmp &= (~(1 << 20));//C_ADC on
		if ((MtFeType_DVBS == handle->m_device_ss2.demod_current_type) || 
			(MtFeType_DVBS2 == handle->m_device_ss2.demod_current_type) || 
			(MtFeType_DVBS_S2 == handle->m_device_ss2.demod_current_type))
			ulTmp &= (~(1 << 21));	// S_ADC on
		else
			ulTmp |= (1 << 21);		// S_ADC off
		handle->Set32Bits(0xBF157000, ulTmp);
	}
	else	// J83B
	{
		handle->Get32Bits(0xBF157000, &ulTmp);
		ulTmp &= (~(1 << 20));	// C_ADC on
		ulTmp |= (1 << 21);		// S_ADC off
		handle->Set32Bits(0xBF157000, ulTmp);
	}

	return MtFeErr_Ok;
}

/*	FUNCTION:
**		mt_fe_set_demod_type_cs8800
**
**	DESCRIPTION:
**		set Symphony2 demod_type
**
**	IN:
**		MT_FE_CS8800_Device_Handle handle, MT_FE_DEMOD_MODE demod_mode
**		MT_FE_TYPE demod_type
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_set_demod_type_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DEMOD_MODE demod_mode, MT_FE_TYPE demod_type)
{
	U8	reg_04 = 0, reg_06 = 0, reg_07 = 0, data = 0;

	U32 ulTmp1;

	MT_FE_RET ret = MtFeErr_Ok;


	//MT_FE_TYPE c_b_type = handle->m_device_c_b.demod_type;
	//MT_FE_TYPE ss2_type = handle->m_device_ss2.demod_type;

	//MT_BOOL bXtal27M = TRUE;

	// reset registers to improve the J83B/DVB-C stability for unknown reason(bug 133085) @ 20220114 by YZ.Huang
	mt_fe_dmd_config_clock_cs8800(handle, demod_type);
	mt_fe_dmd_config_adc_cs8800(handle, demod_type);

#if 1	// ucos or linux
/* PINMUX configuration moved to Uboot */
/*
	handle->Get32Bits(0xBF13C054, &ulTmp1);
	ulTmp1 &= ~0x07;
	ulTmp1 |= 0x05;	// 0xbf13c054[2:0] = 5
	handle->Set32Bits(0xBF13C054, ulTmp1);

	handle->Get32Bits(0xBF13C058, &ulTmp1);
	ulTmp1 &= ~0x07;
	ulTmp1 |= 0x05;	// 0xbf13c054[2:0] = 5
	handle->Set32Bits(0xBF13C058, ulTmp1);
*/
	handle->Get32Bits(0xBF138020, &ulTmp1);
	ulTmp1 |= 0x01;	// 0xbf138020[0] = 1
	handle->Set32Bits(0xBF138020, ulTmp1);

	handle->Get32Bits(0xBF138008, &ulTmp1);
	ulTmp1 &= ~0x200;	// 0xbf138008[9] = 0
	handle->Set32Bits(0xBF138008, ulTmp1);

/* PINMUX configuration moved to Uboot */
/*
	handle->Get32Bits(0xBF13C04C, &ulTmp1);
	ulTmp1 &= ~0x03;
	//ulTmp1 |= 0x02;	// 0xbf13c04c[2:0] = 2
	handle->Set32Bits(0xBF13C04C, ulTmp1);

	handle->Get32Bits(0xBF13C050, &ulTmp1);
	ulTmp1 &= ~0x03;
	//ulTmp1 |= 0x02;	// 0xbf13c050[2:0] = 2
	handle->Set32Bits(0xBF13C050, ulTmp1);
*/
#else	// GUI
	handle->Set32Bits(0xBF138008, 0x00);
	handle->Set32Bits(0xBF13C04C, 0x3002);
	handle->Set32Bits(0xBF13C050, 0x3002);
#endif

	if (demod_mode == MtFeDemodMode_DVBCTT2)
	{
#if 1	// 20200819
		//BF13C048[3:0] = 4	//C_AGC pinmux
		//BF13C048[13] = 0	//C_AGC 配置为CMOS
		if ((handle->chip_version == 0xA000) || (handle->chip_version == 0xA001)) // Sym4 A0/A1
		{
			handle->Get32Bits(0xBF13C048, &ulTmp1);
			ulTmp1 &= 0xFFFFDFF0;
			ulTmp1 |= 0x04;
			handle->Set32Bits(0xBF13C048, ulTmp1);
		}
		else if((handle->chip_version == 0xB000)||(handle->chip_version == 0xB001))//SYM6 A0/A1
		{
			//C_IF_AGC pinmux set by DTS symphony6-base.dtsi node pmx0: pinmux@bf13c000
			//fix issue 30147
		}
#else	// 20200624
		handle->Set32Bits(0xBF13C048, 0x1204);
#endif

		//BF5D0094[3] = 0	//IF PAD设置为ADC input
		handle->Get32Bits(0xBF5D0094, &ulTmp1);
		ulTmp1 &= 0xFFFFFFF7;
		handle->Set32Bits(0xBF5D0094, ulTmp1);	//+++ ANA: ANA_TOP_SW_REG1

#if 0	// 20200819
		//BF138008[0] = 1	//TS2 为DVBC Demod 输入
		handle->Get32Bits(0xBF138008, &ulTmp1);
		ulTmp1 |= 0x01;
		handle->Set32Bits(0xBF138008, ulTmp1);
#endif

#if 0
		//BF13C04C[2:0] = 2	//I2C rpt
		handle->Get32Bits(0xBF13C04C, &ulTmp1);
		ulTmp1 &= 0xFFFFFFF8;
		ulTmp1 |= 0x02;
		handle->Set32Bits(0xBF13C04C, ulTmp1);

		//BF13C050[2:0] = 2	//I2C rpt
		handle->Get32Bits(0xBF13C050, &ulTmp1);
		ulTmp1 &= 0xFFFFFFF8;
		ulTmp1 |= 0x02;
		handle->Set32Bits(0xBF13C050, ulTmp1);
#endif
	}
#if MT_FE_DMD_DVBS_S2_SUPPORT
	else if (demod_mode == MtFeDemodMode_DVBSS2)
	{
		if ((handle->chip_version == 0xA000) || (handle->chip_version == 0xA001)) // Sym4 A0/A1
		{
#if 1	// 20200819
			//BF13C038[3:0] = 4
			//BF13C038[13]=0 //S_AGC 配置为CMOS
			//BF13C038[11:10]=0 //S_AGC 驱动电流改为最小
			
			handle->Get32Bits(0xBF13C038, &ulTmp1);
			ulTmp1 &= 0xFFFFD3F0;
			ulTmp1 |= 0x04;
			handle->Set32Bits(0xBF13C038, ulTmp1);
#else	// 20200624
			//handle->Set32Bits(0xBF13C038, 0x1804);
			handle->Set32Bits(0xBF13C038, 0x1004);		//[11:10]设置为0（驱动改为最小）。
#endif

			//BF13C03C[3:0] = 4
			handle->Get32Bits(0xBF13C03C, &ulTmp1);
			ulTmp1 &= 0xFFFFFFF0;
			ulTmp1 |= 0x04;
			handle->Set32Bits(0xBF13C03C, ulTmp1);

			//BF13C040[3:0] = 4
			handle->Get32Bits(0xBF13C040, &ulTmp1);
			ulTmp1 &= 0xFFFFFFF0;
			ulTmp1 |= 0x04;
			handle->Set32Bits(0xBF13C040, ulTmp1);

			//BF13C044[3:0] = 4 //LNB  pinmux, S_AGC pinmux
			handle->Get32Bits(0xBF13C044, &ulTmp1);
			ulTmp1 &= 0xFFFFFFF0;
			ulTmp1 |= 0x04;
			handle->Set32Bits(0xBF13C044, ulTmp1);
		}
		else if((handle->chip_version == 0xB000)||(handle->chip_version == 0xB001))//SYM6 A0/A1
		{
			//S_IF_AGC LNB Diseqc pinmux set by DTS symphony6-base.dtsi node pmx0: pinmux@bf13c000
			//s_rf_agc_pmx_func
			//lnb_pmx_func
			//diseqc_pmx_func_a
			//fix issue 30147

			//handle->Get32Bits(0xBF13C074, &ulTmp1);//Diseqc_in
			//ulTmp1 &= 0xFFFF0000;
			//ulTmp1 |= 0x0400;
			//handle->Set32Bits(0xBF13C074, ulTmp1);
		}
		//BF5D0094[0] = 0	//IQ PAD设置为ADC input
		handle->Get32Bits(0xBF5D0094, &ulTmp1);
		ulTmp1 &= 0xFFFFFFFE;
		handle->Set32Bits(0xBF5D0094, ulTmp1);		//+++ ANA: ANA_TOP_SW_REG1

#if 0	// 20200819
		//BF138008[4] = 1	//TS1 为DVBS Demod 输入
		handle->Get32Bits(0xBF138008, &ulTmp1);
		ulTmp1 |= 0x10;
		handle->Set32Bits(0xBF138008, ulTmp1);
#endif

#if 0
		//BF13C04C[2:0] = 2	//I2C rpt
		handle->Get32Bits(0xBF13C04C, &ulTmp1);
		ulTmp1 &= 0xFFFFFFF8;
		ulTmp1 |= 0x02;
		handle->Set32Bits(0xBF13C04C, ulTmp1);

		//BF13C050[2:0] = 2	//I2C rpt
		handle->Get32Bits(0xBF13C050, &ulTmp1);
		ulTmp1 &= 0xFFFFFFF8;
		ulTmp1 |= 0x02;
		handle->Set32Bits(0xBF13C050, ulTmp1);
#endif
	}
#endif

	handle->Get32Bits(0xBF5D00D0, &ulTmp1);
	//BF5D00D0H[28 : 27] = _00
	//BF5D00D0H[22] = 0
	ulTmp1 &= 0xE7BFFFFF;
	handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0

	//wait 1ms
	_mt_delay_cs8800(1);

	//BF5D00D0H[22] = 1
	ulTmp1 |= 0x00400000;
	handle->Set32Bits(0xBF5D00D0, ulTmp1);		//+++ ANA: CADC_REG0

	//DVBC and DVBS can coexit at the same time
	//but (DVBS and J83B) or (DVBC or J83B) can't coexit at the same time.
	_mt_fe_sar_get_reg_cs8800(handle, 0x04, &reg_04);
	_mt_fe_sar_get_reg_cs8800(handle, 0x06, &reg_06);
	_mt_fe_sar_get_reg_cs8800(handle, 0x07, &reg_07);

	if (demod_mode == MtFeDemodMode_DVBCTT2)
	{
		_mt_fe_sar_get_reg_cs8800(handle, 0x0A, &data);
		data &= 0xFB;		// 2V

		switch (demod_type)
		{
#if MT_FE_DMD_DVBC_SUPPORT
			case MtFeType_DVBC:
				reg_04 &= (~(1<<2));//bit2(j83B mode), disable J83B
				reg_04 |= (1<<0);//bit0(DVBC mode),enable dvbc
				_mt_fe_sar_set_reg_cs8800(handle, 0x04, reg_04);
				reg_07 |= (1<<3);//bit3(dvbc i2c), enable dvbc i2c
				_mt_fe_sar_set_reg_cs8800(handle, 0x07, reg_07);
				reg_06 &= (~(1<<5));//bit5(J83B_i2c), disable j83b i2c
				_mt_fe_sar_set_reg_cs8800(handle, 0x06, reg_06);
				if (handle->m_device_c_b.board_cfg.iVppSelC == 1)		// 1V
				{
					data |= 0x04;
				}
				break;
#endif

#if MT_FE_DMD_J83B_SUPPORT
			case MtFeType_J83B:
				reg_04 &= (~((1<<0) | (1<<3)));//bit0(DVBC mode)/bit3(DVBS mode), disable DVBC/DVBS
				reg_04 |= (1<<2);//bit2(J83B mode)
				_mt_fe_sar_set_reg_cs8800(handle, 0x04, reg_04);
				reg_07 &= (~((1<<1) | (1<<3)));//bit1(s_i2c_en)/bit3(c_i2c_en), disable DVBC/DVBS i2c
				_mt_fe_sar_set_reg_cs8800(handle, 0x07, reg_07);
				reg_06 |= (1<<5);//bit5(J83B_i2c)
				_mt_fe_sar_set_reg_cs8800(handle, 0x06, reg_06);
				if (handle->m_device_c_b.board_cfg.iVppSelC == 1)		// 1V
				{
					data |= 0x04;
				}
				break;
#endif

			default:
				ret = MtFeErr_NoSupportDemod;
				break;
		}

		_mt_fe_sar_set_reg_cs8800(handle, 0x0A, data);  //Set 1V or 2V Vdpp

		handle->m_device_c_b.demod_type = demod_type;

		//if (handle->chip_version == 0x9000)		// Sym2 A0
			//handle->m_device_ss2.demod_type = MtFeType_Undef;
	}
#if MT_FE_DMD_DVBS_S2_SUPPORT
	else if (demod_mode == MtFeDemodMode_DVBSS2)
	{
		switch (demod_type)
		{
			case MtFeType_DVBS:
			case MtFeType_DVBS2:
			case MtFeType_DVBS_S2:
				reg_04 &= (~(1<<2));//bit2(j83B mode), disable J83B
				reg_04 |= (1<<3);//bit3(dvbs mode),enable dvbs
				_mt_fe_sar_set_reg_cs8800(handle, 0x04, reg_04);
				reg_07 |= (1<<1);//bit1(s_i2c_en),enable dvbs i2c
				_mt_fe_sar_set_reg_cs8800(handle, 0x07, reg_07);
				reg_06 &= (~(1<<5));//bit5(J83B_i2c), disable j83b i2c
				_mt_fe_sar_set_reg_cs8800(handle, 0x06, reg_06);
				break;

			default:
				ret = MtFeErr_NoSupportDemod;
				break;
		}


		if (ret == MtFeErr_Ok)
		{
			handle->m_device_ss2.demod_type = demod_type;

			//if (handle->chip_version == 0x9000)		// Sym2 A0
				//handle->m_device_c_b.demod_type = MtFeType_Undef;
		}
	}
#endif

	//mt_fe_dmd_config_clock_cs8800(handle, demod_type);
	//mt_fe_dmd_config_adc_cs8800(handle, demod_type);

	return ret;
}

MT_FE_RET mt_fe_system_init_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_TYPE demod_type = MtFeType_Undef;
	U8 reg_03 = 0;

	U32 ulTmp = 0, ulData = 0;

	handle->Get32Bits(0xBF140008, &ulTmp);
	handle->Set32Bits(0xBF140008, ulTmp | 0xFFFF);

	handle->Get32Bits(0xBF140004, &ulData);

	handle->Set32Bits(0xBF140008, ulTmp);

	if ((ulData & 0xFFFF) != 0)
		handle->chip_version = (U16)(ulData & 0xFFFF);

	// 0xA000:	Sym4 A0
	// 0xA001:	Sym4 A1
	// 0xB000:	Sym6 A0
	// 0xB001:	Sym6 A1

	//if (handle->bSysInitOk)	// disabled for GUI function @ 20200703
	//	return MtFeErr_Ok;

	if (!g_bNeedReCali)
		return MtFeErr_Ok;

	_mt_fe_sar_get_reg_cs8800(handle, 0x03, &reg_03);
	reg_03 |= 0x80;
	_mt_fe_sar_set_reg_cs8800(handle, 0x03, reg_03);
	_mt_delay_cs8800(1);
	reg_03 &= 0x7F;
	_mt_fe_sar_set_reg_cs8800(handle, 0x03, reg_03);

	_mt_fe_sar_set_reg_cs8800(handle, 0x04, 0x00);
	_mt_fe_sar_set_reg_cs8800(handle, 0x05, 0x00);
	_mt_fe_sar_set_reg_cs8800(handle, 0x06, 0x00);
	_mt_fe_sar_set_reg_cs8800(handle, 0x07, 0x00);
	_mt_fe_sar_set_reg_cs8800(handle, 0x08, 0x00);

	//_mt_fe_sar_set_reg_cs8800(handle, 0x09, 0x04);
	//_mt_fe_sar_set_reg_cs8800(handle, 0x00, 0x01);

	//mt_fe_dmd_config_default_c_b_cs8800(handle);

	handle->Get32Bits(0xBF157004, &ulTmp);
	if ((ulTmp & 0x80000000) == 0)
	{
		ulTmp |= (1 << 31);
		handle->Set32Bits(0xBF157004, ulTmp);			//+++ ANA: ANA_AO_REG1
	}


	//mt_fe_dmd_select_tuner_c_b_cs8800(handle, handle->m_device_c_b.tuner_cfg.tuner_type);

	mt_fe_dmd_calibration_c_b_cs8800(handle);


#if 0
	handle->Get32Bits(0xBF138008, &ulTmp);
	printk("%s[%d] -- 0xBF138008 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp);

	_mt_fe_tn_get_reg_c_b_cs8800(handle, 0x00, &reg_03);

	handle->Get32Bits(0xBF138008, &ulTmp);
	printk("%s[%d] -- 0xBF138008 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp);
#endif


#if MT_FE_DMD_DVBS_S2_SUPPORT
	demod_type = MtFeType_DVBS; //MtFeType_DVBS2;
	//mt_fe_dmd_config_default_ss2_cs8800(handle);

	//mt_fe_dmd_select_tuner_ss2_cs8800(handle, handle->m_device_ss2.tuner_cfg.tuner_type);

	_mt_fe_sar_get_reg_cs8800(handle, 0x03, &reg_03);
	reg_03 = (U8)(reg_03 | 0x0f);
	_mt_fe_sar_set_reg_cs8800(handle, 0x03, reg_03);
	reg_03 = (U8)(reg_03 & 0xf0);
	_mt_fe_sar_set_reg_cs8800(handle, 0x03, reg_03);

	//mt_fe_dmd_open_ss2_cs8800(handle, demod_type);
	//mt_fe_dmd_close_ss2_cs8800(handle);

#if 0
	if (handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
		_mt_fe_tn_get_reg_ss2_cs8800(handle, 0x00, &reg_03);

	handle->Get32Bits(0xBF138008, &ulTmp);
	//printk("%s[%d] -- 0xBF138008 = [0x%08x]\n", __FUNCTION__, __LINE__, ulTmp);
#endif
#endif

#if 0
	if (handle->m_device_c_b.tuner_cfg.tuner_type == MtFeTN_TC6800)
		_mt_fe_tn_get_reg_c_b_cs8800(handle, 0x00, &reg_03);

	if (handle->m_device_ss2.tuner_cfg.tuner_type == MtFeTn_TS6011)
		_mt_fe_tn_get_reg_ss2_cs8800(handle, 0x00, &reg_03);
#endif

	handle->bSysInitOk = TRUE;

	return MtFeErr_Ok;
}


/*	FUNCTION:
**		mt_fe_dmd_close_cs8800
**
**	DESCRIPTION:
**		Finalize Symphony2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	//mt_fe_dmd_close_c_b_cs8800(handle);		// fix 113692
	//mt_fe_dmd_close_ss2_cs8800(handle);		// fix 113692

	_mt_fe_sar_set_reg_cs8800(handle, 0x04, 0x00);
	_mt_fe_sar_set_reg_cs8800(handle, 0x07, 0x00);
	_mt_fe_sar_set_reg_cs8800(handle, 0x06, 0x00);

	handle->m_device_c_b.demod_current_type = MtFeType_Undef;
	handle->m_device_ss2.demod_current_type = MtFeType_Undef;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_dmd_config_default_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_CTT2_Device_Handle handle_c_b = &handle->m_device_c_b;

	if (handle_c_b == NULL)
	{
		return MtFeErr_Uninit;
	}

	handle_c_b->demod_type						 = MtFeType_Undef;
	handle_c_b->ts_out_mode						 = MtFeTsOutMode_Parallel;
	handle_c_b->m_iPageNo						 = 0;
	handle_c_b->input_params.demod_bandwidth	 = MtFeBandwidth_8M;

	handle_c_b->board_cfg.iVppSel				 = 1;		// 0: 2V; 1: 1V
	handle_c_b->board_cfg.iVppSelC				 = 0;
	handle_c_b->board_cfg.bReadCali				 = TRUE;//FALSE;


	handle->m_device_c_b.mcu_status			 = 0;			/* demod device i2c address for C&T */
	handle_c_b->demod_current_type			 = MtFeType_Undef;
	handle_c_b->tuner_cfg.tuner_type		 = MtFeTN_TC6800;
	handle_c_b->tuner_cfg.tuner_dev_addr	 = 0xC6;		/* tuner device i2c address */
	handle_c_b->tuner_cfg.tuner_init_ok		 = 0;			/* tuner init yes or no,0 :no 1:yes */
	handle_c_b->tuner_cfg.tuner_open		 = 0;			/* tuner open yes or no,0 :no 1:yes */
	handle_c_b->tuner_cfg.tuner_lna_type	 = 0;
	handle_c_b->tuner_cfg.tuner_sleep		 = NULL;		/* set tuner function */
	handle_c_b->tuner_cfg.tuner_wakeup		 = NULL;		/* set tuner function */

	handle->Get32Bits = _mt_fe_read32_cs8800;
	handle->Set32Bits = _mt_fe_write32_cs8800;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_select_tuner_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_CS8800_SUPPORTED_TUNER tuner_type)
{
#if 1
	MT_FE_RET ret = MtFeErr_NoSupportTuner;

	switch (tuner_type)
	{
#ifdef CONFIG_MT_FRONTEND_DMD_CS8800_TN_MXL603
		case MtFeTN_MxL603:
			handle->m_device_c_b.tuner_cfg.tuner_type = MtFeTN_MxL603;
			if (handle->m_device_c_b.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_c_b.tuner_cfg.tuner_dev_addr = 0xc0;
			}
			handle->m_device_c_b.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_c_b.tuner_cfg.tuner_open = 0;
			handle->m_device_c_b.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_c_b.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_MxL603_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_MxL603_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_MxL603_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_MxL603_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_c_b.tuner_cfg.tuner_get_gain = NULL;
			ret = MtFeErr_Ok;
			break;
#endif			
#ifdef CONFIG_MT_FRONTEND_DMD_CS8800_TN_TC6800
		case MtFeTN_TC6800:
			handle->m_device_c_b.tuner_cfg.tuner_type = MtFeTN_TC6800;
			if (handle->m_device_c_b.tuner_cfg.tuner_dev_addr == 0x00)
			{
				handle->m_device_c_b.tuner_cfg.tuner_dev_addr = 0xc6;
			}
			handle->m_device_c_b.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_c_b.tuner_cfg.tuner_open = 0;
			handle->m_device_c_b.tuner_cfg.tuner_lna_type = 0;
			handle->m_device_c_b.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_tc6800_c_b;
			handle->m_device_c_b.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_tc6800_c_b;
			handle->m_device_c_b.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_tc6800_c_b;
			handle->m_device_c_b.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_tc6800_c_b;
			handle->m_device_c_b.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_tc6800_c_b;
			handle->m_device_c_b.tuner_cfg.tuner_get_offset = NULL;
			handle->m_device_c_b.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_tc6800_c_b_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_set_application = NULL;//(MT_FE_RET(*)(void *, U8))mt_fe_tn_application_tc6800_c_b_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_set_loop_through = (MT_FE_RET(*)(void *, S16))mt_fe_tn_loop_tc6800_c_b_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_set_xtal = (MT_FE_RET(*)(void *, U32))mt_fe_tn_xtal_tc6800_c_b_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_set_clkout = (MT_FE_RET(*)(void *, U8))mt_fe_tn_clkout_tc6800_c_b_cs8800;
			handle->m_device_c_b.tuner_cfg.tuner_get_diagnose_info = (MT_FE_RET(*)(void *, U32*, U32*))mt_fe_tn_get_diagnose_info_tc6800_c_b_cs8800;
			ret = MtFeErr_Ok;
			break;
#endif

		default:
			break;
	}

	return ret;
#endif

	return MtFeErr_Ok;
}

/*	FUNCTION:
**		mt_fe_dmd_open_c_b_cs8800
**
**	DESCRIPTION:
**		initialize Symphony2 DVB-C, J83.B, DVB-T or DVB-T2
**
**	IN:
**		MT_FE_CS8800_Device_Handle handle
**		MT_FE_TYPE 	demod_type
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	mt_fe_set_demod_type_cs8800(handle, MtFeDemodMode_DVBCTT2, demod_type);

	if (handle->m_device_c_b.demod_current_type == demod_type)
	{
		ret = MtFeErr_Ok;
		return ret;
	}


	if (
#if MT_FE_DMD_DVBC_SUPPORT
		(demod_type != MtFeType_DVBC)
#endif
#if MT_FE_DMD_J83B_SUPPORT
		&& (demod_type != MtFeType_J83B)
#endif
		)
	{
		ret = MtFeErr_Fail;
		return ret;
	}

	//if ((demod_type == MtFeType_DVBC) || 
	//   (demod_type == MtFeType_J83B))
	{
		U32 ulTmp;

		handle->Get32Bits(0xBF5D0094, &ulTmp);
		ulTmp &= 0xFFFFFFF0;
		//ulTmp |= 0x00000006;
		handle->Set32Bits(0xBF5D0094, ulTmp);		//+++ ANA: ANA_TOP_SW_REG1
	}

	/*if (handle->m_device_c_b.demod_current_type != MtFeType_Undef)
	{
		if (demod_type != handle->m_device_c_b.demod_current_type)
		{
			mt_fe_dmd_close_c_b_cs8800(handle);
			ret = mt_fe_set_demod_type_cs8800(handle, MtFeDemodMode_DVBCTT2, demod_type);
		}
	}
	else
	{
		ret = mt_fe_set_demod_type_cs8800(handle, MtFeDemodMode_DVBCTT2, demod_type);
	}*/

	if (ret == MtFeErr_Ok)
	{
#if MT_FE_DMD_DVBC_SUPPORT
		if (demod_type == MtFeType_DVBC)
		{
			ret = _mt_fe_dmd_init_cs8800_c(handle);
		}
#endif
#if MT_FE_DMD_J83B_SUPPORT
		if (demod_type == MtFeType_J83B)
		{
			ret = _mt_fe_dmd_init_cs8800_b(handle);
		}
#endif

		//mt_fe_dmd_select_tuner_c_b_cs8800(handle, handle->m_device_c_b.tuner_cfg.tuner_type);

		handle->m_device_c_b.demod_current_type = handle->m_device_c_b.demod_type;
	}

#if MT_FE_DMD_J83B_SUPPORT
	if (demod_type == MtFeType_J83B)
	{  // reset J83B related registers to improve the J83B stability for unknown reason(bug 133085) @ 20220114 by YZ.Huang
		mt_fe_set_demod_type_cs8800(handle, MtFeDemodMode_DVBCTT2, demod_type);
	}
#endif

	return ret;
}


/*	FUNCTION:
**		mt_fe_dmd_close_c_b_cs8800
**
**	DESCRIPTION:
**		finalize Symphony2 DVB-C, J83.B, DVB-T or DVB-T2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
#if 0
	if ((handle->m_device_c_b.tuner_cfg.tuner_open != 0) && (handle->m_device_c_b.demod_current_type != MtFeType_Undef))
	{	// fix 113692
		if (handle->m_device_c_b.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_cs8800(handle);
			handle->m_device_c_b.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_cs8800(handle);
		}
	}
#endif

	//mt_fe_dmd_config_c_b_adc_cs8800(handle, TRUE);	// power down CT-ADC	// fix 112220

	handle->m_device_c_b.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_c_b.demod_type = MtFeType_Undef;						// fix 112220
	handle->m_device_c_b.demod_current_type = MtFeType_Undef;
    /*fix issue 28177*/
    handle->bDVBCInitOk = FALSE;
    handle->bJ83BInitOk = FALSE;
    
	if (handle->m_device_ss2.demod_current_type == MtFeType_Undef)
		handle->bSysInitOk = FALSE;

	return MtFeErr_Ok;
}

/*	FUNCTION:
**		mt_fe_dmd_connect_c_b_cs8800
**
**	DESCRIPTION:
**		connect to a special channel for dvbc or dvbt or dvbt2
**
**	IN:
**
**		MT_FE_CS8800_Device_Handle handle
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	ret = mt_fe_dmd_open_c_b_cs8800(handle, handle->m_device_c_b.demod_type);

	if (ret == MtFeErr_Ok)
	{
#if MT_FE_DMD_DVBC_SUPPORT
		if (handle->m_device_c_b.demod_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_connect_cs8800_c(handle);
#endif
#if MT_FE_DMD_J83B_SUPPORT
		if (handle->m_device_c_b.demod_type == MtFeType_J83B)
			ret = _mt_fe_dmd_connect_cs8800_b(handle);
#endif
	}

	return ret;
}

/*	FUNCTION:
**		mt_fe_dmd_get_lock_state_c_b_cs8800
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_CS8800_Device_Handle handle
**
**	OUT:
**		*p_state	-	MtFeLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (
#if MT_FE_DMD_DVBC_SUPPORT
		(handle->m_device_c_b.demod_current_type != MtFeType_DVBC)
#endif
#if MT_FE_DMD_J83B_SUPPORT
	 && (handle->m_device_c_b.demod_current_type != MtFeType_J83B)
#endif
	 )
	{
		ret = MtFeErr_Fail;
	}

	if (ret == MtFeErr_Ok)
	{
#if MT_FE_DMD_DVBC_SUPPORT
		if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_get_lock_state_cs8800_c(handle, p_state);
#endif
#if MT_FE_DMD_J83B_SUPPORT
		if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
			ret = _mt_fe_dmd_get_lock_state_cs8800_b(handle, p_state);
#endif
	}

	return ret;
}

/*	FUNCTION:
**		mt_fe_dmd_hard_reset_cs8800
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_hard_reset_cs8800(void)
{
	return MtFeErr_Ok;
}


/*	FUNCTION:
**		mt_fe_dmd_soft_reset_c_b_cs8800
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if (
#if MT_FE_DMD_DVBC_SUPPORT
		(handle->m_device_c_b.demod_current_type != MtFeType_DVBC)
#endif
#if MT_FE_DMD_J83B_SUPPORT
	 && (handle->m_device_c_b.demod_current_type != MtFeType_J83B)
#endif
	   )
	{
		ret = MtFeErr_Fail;
	}

	if (ret == MtFeErr_Ok)
	{
#if MT_FE_DMD_DVBC_SUPPORT
		if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
			ret = _mt_fe_dmd_soft_reset_cs8800_c(handle);
#endif
#if MT_FE_DMD_J83B_SUPPORT
		if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
			ret = _mt_fe_dmd_soft_reset_cs8800_b(handle);
#endif
	}

	return ret;
}


/*	FUNCTION:
**		mt_fe_dmd_set_output_mode_cs8800
**
**	DESCRIPTION:
**		select the serial interface or parallel interface.
**
**	IN:
**		mode	-	MtFeDmdTsOutputMode_Serial
**				-	MtFeDmdTsOutputMode_Parallel
**
**	OUT:
**		none.
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_set_output_mode_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_set_output_mode_cs8800_c(handle);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_set_bw_c_b_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

//	if (handle->demod_type == MtFeType_DVBC)
//		ret = _mt_fe_dmd_set_bw_cs8800_c(handle);

	return ret;
}

MT_FE_RET mt_fe_dmd_get_accurate_snr_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, S32 *p_snr)
{
	MT_FE_RET	ret = MtFeErr_Ok;
#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_accurate_snr_cs8800_c(handle, p_snr);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_accurate_snr_cs8800_b(handle, p_snr);
#endif
	return ret;

}

MT_FE_RET mt_fe_dmd_get_snr_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_snr)
{
	MT_FE_RET	ret = MtFeErr_Ok;

#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_snr_cs8800_c(handle, p_snr);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
	{
		U16 snr_b = 0;

		ret = _mt_fe_dmd_get_snr_cs8800_b(handle, &snr_b);

		*p_snr = snr_b / 10;

		//printk("\t J83B SNR = %d.%d dB\n", snr_b / 10, snr_b % 10);
	}
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_quality_cs8800_c(handle, p_percent);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_quality_cs8800_b(handle, p_percent);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_quality_nordig_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_quality_cs8800_c(handle, p_percent);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_quality_cs8800_b(handle, p_percent);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_strength_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;

#if MT_FE_DMD_DVBC_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
		ret = _mt_fe_dmd_get_strength_cs8800_c(handle, ssi_percent);
#endif
#if MT_FE_DMD_J83B_SUPPORT
	if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
		ret = _mt_fe_dmd_get_strength_cs8800_b(handle, ssi_percent);
#endif

	return ret;
}

MT_FE_RET mt_fe_dmd_get_per_c_b_cs8800(MT_FE_CS8800_Device_Handle handle, U32 *p_err_cnt, U32 *p_total_cnt)
{
	if (handle->m_device_c_b.demod_current_type == MtFeType_DVBC)
	{
		return _mt_fe_dmd_get_ber_cs8800_c(handle, p_err_cnt, p_total_cnt);
	}
	else if (handle->m_device_c_b.demod_current_type == MtFeType_J83B)
	{
		return _mt_fe_dmd_get_ber_cs8800_b(handle, p_err_cnt, p_total_cnt);
	}
	else
	{
		return MtFeErr_Ok;
	}

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_sleep_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	U8	data = 0;

	if ((handle->m_device_c_b.tuner_cfg.tuner_open != 0) && (handle->m_device_c_b.demod_current_type != MtFeType_Undef))
	{
		if (handle->m_device_c_b.tuner_cfg.tuner_sleep != NULL)
		{
			handle->m_device_c_b.tuner_cfg.tuner_sleep(handle);
		}
	}

#if MT_FE_DMD_DVBS_S2_SUPPORT
	if ((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if (handle->m_device_ss2.tuner_cfg.tuner_sleep != NULL)
		{
			handle->m_device_ss2.tuner_cfg.tuner_sleep(handle);
		}
	}
#endif

	_mt_fe_sar_get_reg_cs8800(handle, 0x03, &data);
	data = (U8)(data | 0x20);
	_mt_fe_sar_set_reg_cs8800(handle, 0x03, data);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_dmd_wake_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	U8	data = 0;

	_mt_fe_sar_get_reg_cs8800(handle, 0x03, &data);
	data = (U8)(data & 0xdf);
	_mt_fe_sar_set_reg_cs8800(handle, 0x03, data);

	if ((handle->m_device_c_b.tuner_cfg.tuner_open != 0) && (handle->m_device_c_b.demod_current_type != MtFeType_Undef))
	{
		if (handle->m_device_c_b.tuner_cfg.tuner_wakeup != NULL)
		{
			handle->m_device_c_b.tuner_cfg.tuner_wakeup(handle);
		}
	}

#if MT_FE_DMD_DVBS_S2_SUPPORT
	if ((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if (handle->m_device_ss2.tuner_cfg.tuner_wakeup != NULL)
		{
			handle->m_device_ss2.tuner_cfg.tuner_wakeup(handle);
		}
	}
#endif

	return MtFeErr_Ok;
}

/****************************************DVBS/S2 APIS**********************************************/

#if MT_FE_DMD_DVBS_S2_SUPPORT

MT_FE_RET mt_fe_dmd_config_default_ss2_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	return mt_fe_dmd_cs8800_ss2_config_default(handle);
}

MT_FE_RET mt_fe_dmd_select_tuner_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_CS8800_SUPPORTED_TUNER tuner_type)
{
#if 1
	MT_FE_RET ret = MtFeErr_NoSupportTuner;

	switch (tuner_type)
	{
#ifdef CONFIG_MT_FRONTEND_DMD_CS8800_TN_TS6011
		case MtFeTn_TS6011:
			handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_TS6011;
			if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
				handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x58;
			handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
			handle->m_device_ss2.tuner_cfg.tuner_open = 0;
			handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_ts6011_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_ts6011_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_ts6011_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_ts6011_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_ts6011_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_ts6011_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_ts6011_cs8800;
			handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
			ret = MtFeErr_Ok;
			break;
#endif

#ifdef CONFIG_MT_FRONTEND_DMD_CS8800_TN_RT720
		case MtFeTn_RT720:
			handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_RT720;
			if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
				handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x34;
			handle->m_device_ss2.tuner_cfg.tuner_init_ok    = 0;
			handle->m_device_ss2.tuner_cfg.tuner_open       = 0;
			handle->m_device_ss2.tuner_cfg.tuner_init       = (MT_FE_RET(*)(void *))mt_fe_tn_init_RT720_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_set        = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_RT720_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_strength   = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_RT720_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_sleep      = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_RT720_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_wakeup     = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_RT720_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_RT720_cs8800;
			handle->m_device_ss2.tuner_cfg.tuner_get_gain   = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_RT720_cs8800;
			handle->m_device_ss2.board_cfg.bIQInverted      = TRUE;
			handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;//TRUE;
			handle->m_device_ss2.bs_cfg.bs_symrate = BLINDSCAN_SYMRATEKSs;	// 40000;
			ret = MtFeErr_Ok;
			break;
#endif

		default:
			break;
	}

	return ret;
#endif

	return MtFeErr_Ok;
}

/*	FUNCTION:
**		mt_fe_dmd_close_ss2_cs8800
**
**	DESCRIPTION:
**		finalize Symphony2 dvbs or  dvbs2
**
**	IN:
**		none
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_close_ss2_cs8800(MT_FE_CS8800_Device_Handle handle)
{
#if 0
	if ((handle->m_device_ss2.tuner_cfg.tuner_open != 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if (handle->m_device_ss2.tuner_cfg.tuner_sleep != NULL)
		{
			mt_fe_i2c_repeat_enable_cs8800(handle);
			handle->m_device_ss2.tuner_cfg.tuner_sleep(handle);
			mt_fe_i2c_repeat_disable_cs8800(handle);
		}
	}
#endif

	handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ss2.demod_type = MtFeType_Undef;					// fix 112220
	handle->m_device_ss2.demod_current_type = MtFeType_Undef;
    /*fix issue 28177*/
    handle->bDVBSInitOk = FALSE;

	if (handle->m_device_c_b.demod_current_type == MtFeType_Undef)
		handle->bSysInitOk = FALSE;


	return MtFeErr_Ok;
}

/*	FUNCTION:
**		mt_fe_dmd_open_ss2_cs8800
**
**	DESCRIPTION:
**		initialize Symphony2 dvbs or dvbs2
**
**	IN:
**		MT_FE_CS8800_Device_Handle		handle
**		MT_FE_TYPE 	demod_type
**
**	OUT:
**		none
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_open_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_TYPE demod_type)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((demod_type != MtFeType_DVBS) && 
		(demod_type != MtFeType_DVBS2) && 
		(demod_type != MtFeType_DVBS_S2))
	{
		demod_type = MtFeType_DVBS_S2;
	}

	mt_fe_set_demod_type_cs8800(handle, MtFeDemodMode_DVBSS2, demod_type);

	if (handle->m_device_ss2.demod_current_type == demod_type)
		return ret;

#if 1
	if (((handle->m_device_ss2.demod_current_type == MtFeType_DVBS) || 
		 (handle->m_device_ss2.demod_current_type == MtFeType_DVBS2) || 
		 (handle->m_device_ss2.demod_current_type == MtFeType_DVBS_S2)) && 
		((demod_type == MtFeType_DVBS) || 
		 (demod_type == MtFeType_DVBS2) || 
		 (demod_type == MtFeType_DVBS_S2))
	   )
	{
		handle->m_device_ss2.demod_type = demod_type;
		handle->m_device_ss2.demod_current_type = handle->m_device_ss2.demod_type;

		//if (handle->chip_version == 0x9000)		// Sym2 A0
			//handle->m_device_c_b.demod_current_type = MtFeType_Undef;

		return MtFeErr_Ok;
	}
#endif

	if ((demod_type != MtFeType_DVBS) && (demod_type != MtFeType_DVBS2) && (demod_type != MtFeType_DVBS_S2))
	{
		ret = MtFeErr_Fail;
		return ret;
	}

	//ret = mt_fe_set_demod_type_cs8800(handle, MtFeDemodMode_DVBSS2, demod_type);

#if 0
	if ((handle->m_device_ss2.tuner_cfg.tuner_open == 0) && (handle->m_device_ss2.demod_current_type != MtFeType_Undef))
	{
		if (handle->m_device_ss2.tuner_cfg.tuner_wakeup != NULL)
		{
			mt_fe_i2c_repeat_enable_cs8800(handle);
			handle->m_device_ss2.tuner_cfg.tuner_wakeup(handle);
			mt_fe_i2c_repeat_disable_cs8800(handle);
		}
	}
#endif

	//if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_init_cs8800_ss2(handle);
	}

	handle->m_device_ss2.demod_current_type = handle->m_device_ss2.demod_type;

	return ret;
}

/*	FUNCTION:
**		mt_fe_dmd_connect_ss2_cs8800
**
**	DESCRIPTION:
**		connect to a special channel for dvbs or dvbs2
**
**	IN:
**
**		MT_FE_CS8800_Device_Handle handle
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_connect_ss2_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET ret = MtFeErr_Ok;

	mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);

	if (ret == MtFeErr_Ok)
	{
		U32 iFreqMHz = (handle->m_device_ss2.input_params.input_freq_kHz + 500) / 1000;
		U32 iSymRateKSs = handle->m_device_ss2.input_params.symbol_rate_KSs;
		MT_FE_TYPE dvbs_type = handle->m_device_ss2.demod_type;

		ret = mt_fe_dmd_connect_cs8800_ss2(handle, iFreqMHz, iSymRateKSs, dvbs_type);
	}

	return ret;
}

/*	FUNCTION:
**		mt_fe_dmd_get_lock_state_ss2_cs8800
**
**	DESCRIPTION:
**
**	IN:
**		MT_FE_CS8800_Device_Handle handle
**
**	OUT:
**		*p_state	-	MtFeLockState_Unlocked
**					-	MtFeLockState_Locked
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_get_lock_state_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_LOCK_STATE *p_state)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_lock_state_cs8800_ss2(handle, p_state);
	}

	return ret;
}

/*	FUNCTION:
**		mt_fe_dmd_soft_reset_ss2_cs8800
**
**	DESCRIPTION:
**
**	IN:
**
**	OUT:
**
**	RETURN:
*/
MT_FE_RET mt_fe_dmd_soft_reset_ss2_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_soft_reset_cs8800_ss2(handle);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_accurate_snr_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, S32 *p_snr)
{
	MT_FE_RET	ret = MtFeErr_Ok;
#if defined(__linux__) && defined(CONFIG_MT_FRONTEND_DMD_CS8800)
	return ret;
#else
	ret = mt_fe_dmd_get_accurate_snr_cs8800_ss2(handle, p_snr);
	return ret;
#endif
}

MT_FE_RET mt_fe_dmd_get_quality_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *p_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_sat_quality_cs8800_ss2(handle, p_percent);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_get_strength_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U8 *ssi_percent)
{
	MT_FE_RET	ret = MtFeErr_Ok;
	S8 	p_strength = 0;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
		ret = MtFeErr_Fail;

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_get_strength_cs8800_ss2(handle, &p_strength);
		*ssi_percent =(U8)(p_strength);
	}

	return 	ret;
}

MT_FE_RET mt_fe_dmd_blindscan_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, U32 begin_freq_MHz, U32 end_freq_MHz, MT_FE_BS_TP_INFO *p_bs_info)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, MtFeType_DVBS_S2);
	}

	if (ret == MtFeErr_Ok)
	{
		ret = mt_fe_dmd_blindscan_cs8800_ss2(handle, begin_freq_MHz, end_freq_MHz, p_bs_info);
	}

	return ret;
}

MT_FE_RET mt_fe_dmd_blindscan_abort_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_BOOL bs_abort)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);
	}
	mt_fe_dmd_blindscan_abort_cs8800_ss2(handle, bs_abort);

	return ret;
}

MT_FE_RET  mt_fe_dmd_set_LNB_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_BOOL is_LNB_enable, MT_FE_BOOL is_22k_enable, MT_FE_LNB_VOLTAGE voltage_type, MT_FE_BOOL is_envelop_mode)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_set_LNB_cs8800_ss2(handle, is_LNB_enable, is_22k_enable, voltage_type, is_envelop_mode);

	return ret;
}

MT_FE_RET  mt_fe_dmd_DiSEqC_send_tone_burst_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_TONE_BURST mode, MT_FE_BOOL is_envelop_mode)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_send_tone_burst_cs8800_ss2(handle, mode, is_envelop_mode);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_send_receive_msg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);
	}

	ret = mt_fe_dmd_DiSEqC_send_receive_msg_cs8800_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_send_msg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_send_msg_cs8800_ss2(handle, msg);

	return ret;
}

MT_FE_RET mt_fe_dmd_DiSEqC_receive_msg_ss2_cs8800(MT_FE_CS8800_Device_Handle handle, MT_FE_DiSEqC_MSG *msg)
{
	MT_FE_RET	ret = MtFeErr_Ok;

	if ((handle->m_device_ss2.demod_current_type != MtFeType_DVBS) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS2) && 
		(handle->m_device_ss2.demod_current_type != MtFeType_DVBS_S2))
	{
		ret = mt_fe_dmd_open_ss2_cs8800(handle, handle->m_device_ss2.demod_type);
	}

	mt_fe_dmd_DiSEqC_receive_msg_cs8800_ss2(handle, msg);

	return ret;
}

#endif

