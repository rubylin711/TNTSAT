/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/****************************************************************
                           RT710.c
Copyright 2014 by Rafaelmicro., Inc.
                 C H A N G E   R E C O R D
  Date        Author           Version      Description
--------   --------------    --------   ----------------------
01/29/2015	   Ryan				v3.0		add 720 setting
01/29/2015	   Ryan				v3.1		add scan mode for RT720 (RT710 not support)
02/17/2015	   Ryan				v3.2		Change the pll execution time
02/12/2015	   Ryan				v3.2a		Standby function add clock parameter
03/30/2015	   Ryan				v3.3		Add RT720 filter table (with different RT710)
04/07/2015	   Ryan				v3.3a		optimize PLL Setting
04/07/2015	   Ryan				v3.4c		Add Xtal Cap Option only for R720 chip
10/30/2015     Vincent          v3.5        modify IF Fillter bandwidth
03/21/2016     Vincent          v3.6        Modify vth vtl parameter range
											Remove floating calculation
											Standby LT off,xtal ldo off,RT720 lna off,LNA match change(s11)
11/30/2016     Vincent          v3.7        Modify RT720 vtl range
09/04/2019     Vincent          v3.8        Modify coras tune boundary

****************************************************************/
//#include <stdio.h>
//#include <string.h>
//#include <linux/printk.h>


#include "mt_fe_tn_RT710.h"

#include "mt_fe_def.h"
#include "mt_fe_i2c_cs8800.h"

#if MT_FE_DMD_DVBS_S2_SUPPORT


RT710_INFO_Type RT720_Tuner;


UINT8 RT710_ADDRESS = 0xF4;

UINT8 R0TOR3_Write_Arry[4] = {0x00, 0x00, 0x00, 0x00};

//===========Xtal Cap set (Range is 0~30pF) ==============
//Suggest Xtal_cap use 30pF when xtal CL value is 16pF , Default.
UINT8 Xtal_cap = 30; //Unit:pF

I2C_TYPE RT710_Write_Byte;
I2C_LEN_TYPE RT710_Write_Len;
I2C_LEN_TYPE RT710_Read_Len;
UINT8 R710_Initial_done_flag = FALSE;
UINT8 Chip_type_flag = TRUE; //TRUE:RT710 ; FALSE;RT720
UINT32 R710_satellite_bw;
UINT32 R710_pre_satellite_bw;

RT710_Err_Type RT710_PLL(UINT32 PLL_Freq);

#if (RT710_0DBM_SETTING == TRUE)
//0dBm ; LT:lna output ; LT:HPF off  ; LT Current High
UINT8 RT710_Reg_Arry_Init[RT710_Reg_Num] = {0x40, 0x1C, 0xA0, 0x90, 0x41, 0x50, 0xED, 0x25, 0x47, 0x58, 0x39, 0x60, 0x38, 0xE7, 0x90, 0x35};
#else
//-10~-30dBm ; LT:lna center ; LT:HPF on  ; LT Current Low
UINT8 RT710_Reg_Arry_Init[RT710_Reg_Num] = {0x40, 0x1D, 0x20, 0x10, 0x41, 0x50, 0xED, 0x25, 0x07, 0x58, 0x39, 0x64, 0x38, 0xE7, 0x90, 0x35};
#endif

UINT8 RT720_Reg_Arry_Init[RT710_Reg_Num] = {0x00, 0x1C, 0x00, 0x10, 0x41, 0x48, 0xDA, 0x4B, 0x07, 0x58, 0x28, 0x40, 0x37, 0xE7, 0x4C, 0x59};

UINT8 RT710_Reg_Arry[RT710_Reg_Num];

static UINT16 RT710_Lna_Acc_Gain[19] =
{
	  0,  26,  42,  74, 103, 129, 158, 181, 188, 200, //0~9
	220, 248, 280, 312, 341, 352, 366, 389, 409,      //10~19
};

static UINT16 RT710_Lna2_Acc_Gain[32] =
{
	  0,  27,  53,  81, 109, 134, 156, 176, 194, 202, //0~9
	211, 221, 232, 245, 258, 271, 285, 307, 326, 341, //10~19
	357, 374, 393, 410, 428, 439, 445, 470, 476, 479, //20~29
	495, 507										  //30~31
};


static int RT720_Convert(int InvertNum)
{
	int ReturnNum = 0;
	int AddNum    = 0x80;
	int BitNum    = 0x01;
	int CuntNum   = 0;

	for(CuntNum = 0; CuntNum < 8; CuntNum ++)
	{
		if(BitNum & InvertNum)
			ReturnNum += AddNum;

		AddNum /= 2;
		BitNum *= 2;
	}

	return ReturnNum;
}

static int I2C_Write_Len(I2C_LEN_TYPE *I2C_Info)
{
	U8 i = 0;
	RT710_Err_Type status = RT_Fail;
	U8 pbuf[64] = {0};

	pbuf[0] = I2C_Info->RegAddr;

	//printf("%s[%d] ---- Register value, total %d bytes:\n", __FUNCTION__, __LINE__, I2C_Info->Len);

	for(i = 0; i < I2C_Info->Len; i ++)
	{
		pbuf[i + 1] = I2C_Info->Data[i];

#if 0
		//printf("\t%02x, ", I2C_Info->Data[i]);

		if ((i % 0x08) == 0x07)
			//printf("\n");
#endif
	}

	//printf("\n");

	if (mt_fe_tn_write_cs8800(RT710_ADDRESS, pbuf, (U8)(I2C_Info->Len + 1)) != MtFeErr_Ok)
	{
		//printf("%s[%d] ---- FAILED! Addr = 0x%02x\n", __FUNCTION__, __LINE__, RT710_ADDRESS);
		status = RT_Fail;
	}
	else
	{
		status = RT_Success;
	}

	return status;
}

static int I2C_Read_Len(I2C_LEN_TYPE *I2C_Info)
{
	RT710_Err_Type status = RT_Fail;

	U8 i = 0, tmp_reg = 0;
	U8 pbuf[64] = {0};


	if(I2C_Info->RegAddr != 0)
	{
		I2C_Info->RegAddr = 0;
	}
	tmp_reg = I2C_Info->RegAddr;

	//if ((mt_fe_tn_read_cs8800(RT710_ADDRESS, &tmp_reg, 1, pbuf, I2C_Info->Len)) != MtFeErr_Ok)
	if ((mt_fe_tn_read_cs8800(RT710_ADDRESS, &tmp_reg, 0, pbuf, I2C_Info->Len)) != MtFeErr_Ok)
	{
		//mt_dbg_printf(1, "FUN[%s] LINE[%u] FAILED!\n", __FUNCTION__, __LINE__);
		status = RT_Fail;

		//printf("%s[%d] ---- Register value, total %d bytes:\n", __FUNCTION__, __LINE__, I2C_Info->Len);
		//printf("%s[%d] ---- FAILED! Addr = 0x%02x\n", __FUNCTION__, __LINE__, RT710_ADDRESS);


		for(i = 0; i < I2C_Info->Len; i++)
		{
			//I2C_Info->Data[i] = R840_Convert(pbuf[i]);
#if 0
			//printf("\t%02x, ", I2C_Info->Data[i]);

			if ((i % 0x08) == 0x07)
				//printf("\n");
#endif
		}

		//printf("\n");
	}
	else
	{
		//printk("%s[%d] ---- Register value, total %d bytes:\n", __FUNCTION__, __LINE__, I2C_Info->Len);

		for(i = 0; i < I2C_Info->Len; i++)
		{
			I2C_Info->Data[i] = RT720_Convert(pbuf[i]);

#if 0
			printk("\t%02x,\n", I2C_Info->Data[i]);

			//if ((i % 0x08) == 0x07)
			//	printf("\n");
#endif
		}

		//printk("\n");

		status = RT_Success;
	}

	return status;
}


static int I2C_Write(I2C_TYPE *I2C_Info)
{
	RT710_Err_Type status = RT_Fail;

	U8 pbuf[2] = {0};

	pbuf[0] = I2C_Info->RegAddr;
	pbuf[1] = I2C_Info->Data;

	if ((mt_fe_tn_write_cs8800(RT710_ADDRESS, pbuf, 2)) != MtFeErr_Ok)
	{
		//printf("%s[%d] ---- FAILED! Addr = 0x%02x\n", __FUNCTION__, __LINE__, RT710_ADDRESS);
		status = RT_Fail;
	}
	else
	{
		status = RT_Success;
	}

	return status;
}


static void RT710_Delay_MS(U32 ms)
{
	if (ms >= 10)
		_mt_sleep_cs8800(ms);
	else
		_mt_delay_cs8800(ms);
}

RT710_Err_Type RT710_Setting(RT710_INFO_Type RT710_Set_Info)
{
	UINT8 fine_tune = 0;
	UINT8 Coarse = 0;
	UINT16 bw_offset = 20000;
	UINT32 offset_range = 0;

	int icount = 0;
	int i = 0;

	RT710_Write_Len.RegAddr = 0x00;
	RT710_Write_Len.Len = 0x10;

	RT710_Read_Len.RegAddr = 0x00;
	RT710_Read_Len.Len = 0x04;

	if (R710_Initial_done_flag == FALSE)
	{
		R710_pre_satellite_bw = 0; //Init BW Value
		R710_satellite_bw = 0;

		I2C_Read_Len(&RT710_Read_Len);	 // read data length
		if ((RT710_Read_Len.Data[3] & 0xF0) == 0x70) //TRUE:RT710(R3[7:4]=7) ; FALSE;RT720(R3[7:4]=0)
			Chip_type_flag = TRUE;
		else
			Chip_type_flag = FALSE;

		if (Chip_type_flag == TRUE) //TRUE:RT710 ; FALSE;RT720
		{
			for (icount = 0; icount < RT710_Reg_Num; icount++)
			{
				RT710_Reg_Arry[icount] = RT710_Reg_Arry_Init[icount];
			}
		}
		else
		{
			for (icount = 0; icount < RT710_Reg_Num; icount++)
			{
				RT710_Reg_Arry[icount] = RT720_Reg_Arry_Init[icount];
			}
		}

		// LOOP_THROUGH(0=on ; 1=off)
		if (RT710_Set_Info.RT710_LoopThrough_Mode != LOOP_THROUGH)
		{
			RT710_Reg_Arry[1] |= 0x04;
		}
		else
		{
			RT710_Reg_Arry[1] &= 0xFB;
		}

		//Clock out(1=off ; 0=on)
		if (RT710_Set_Info.RT710_ClockOut_Mode != ClockOutOn)
		{
			RT710_Reg_Arry[3] |= 0x10;
		}
		else
		{
			RT710_Reg_Arry[3] &= 0xEF;
		}

		//Output Signal Mode
		if (RT710_Set_Info.RT710_OutputSignal_Mode != DifferentialOut)
		{
			RT710_Reg_Arry[11] |= 0x10;
		}
		else
		{
			RT710_Reg_Arry[11] &= 0xEF;
		}

		//AGC Type  //R13[4] Negative=0 ; Positive=1;
		if (RT710_Set_Info.RT710_AGC_Mode != AGC_Positive)
		{
			RT710_Reg_Arry[13] &= 0xEF;
		}
		else
		{
			RT710_Reg_Arry[13] |= 0x10;
		}

		//RT710_Vga_Sttenuator_Type
		if (RT710_Set_Info.RT710_AttenVga_Mode != ATTENVGAON)
		{
			RT710_Reg_Arry[11] &= 0xF7;
		}
		else
		{
			RT710_Reg_Arry[11] |= 0x08;
		}

		if (Chip_type_flag == TRUE) //TRUE:RT710 ; FALSE;RT720
		{
			RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFC) | RT710_Set_Info.R710_FineGain;
		}
		else
		{
			if (RT710_Set_Info.R710_FineGain > 1)
				RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFE) | 0x01;
			else
				RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xFE) | 0x00;

			RT710_Reg_Arry[3] = ((RT710_Reg_Arry[3] & 0xF0) | ((30 - Xtal_cap) >> 1));
		}

		for (i = 0; i < RT710_Write_Len.Len; i++)
		{
			RT710_Write_Len.Data[i] = RT710_Reg_Arry[i];
		}

		if (I2C_Write_Len(&RT710_Write_Len) != RT_Success)
			return RT_Fail;

		R710_Initial_done_flag = TRUE;
	}

	// Check Input Frequency Range
	if (Chip_type_flag == TRUE) //TRUE:RT710 ; FALSE;RT720
	{
		if ((RT710_Set_Info.RF_KHz < 600000) || (RT710_Set_Info.RF_KHz > 2400000))
			return RT_Fail;
	}
	else
	{
		if ((RT710_Set_Info.RF_KHz < 200000) || (RT710_Set_Info.RF_KHz > 2400000))
			return RT_Fail;
	}

	if (RT710_PLL(RT710_Set_Info.RF_KHz) != RT_Success)
		return RT_Fail;

	RT710_Delay_MS(10);

	if (Chip_type_flag == TRUE)
	{
		if ((RT710_Set_Info.RF_KHz >= 1600000) && (RT710_Set_Info.RF_KHz < 1950000))
		{
			RT710_Reg_Arry[2] |= 0x40;						//LNA Mode with att
			RT710_Reg_Arry[8] |= 0x80;						//Mixer Buf -3dB
			RT710_Reg_Arry[10] = (RT710_Reg_Arry_Init[10]); //AGC VTH:1.24V  ;  VTL:0.64V
			RT710_Write_Byte.RegAddr = 0x0A;
			RT710_Write_Byte.Data = RT710_Reg_Arry[10];
			if (I2C_Write(&RT710_Write_Byte) != RT_Success)
				return RT_Fail;
		}
		else
		{
			RT710_Reg_Arry[2] &= 0xBF; //LNA Mode no att
			RT710_Reg_Arry[8] &= 0x7F; //Mixer Buf off
			if (RT710_Set_Info.RF_KHz >= 1950000)
			{
				RT710_Reg_Arry[10] = ((RT710_Reg_Arry[10] & 0xF0) | 0x08); //AGC VTH:1.14V
				RT710_Reg_Arry[10] = ((RT710_Reg_Arry[10] & 0x0F) | 0x20); //AGC VTL:0.54V
				RT710_Write_Byte.RegAddr = 0x0A;
				RT710_Write_Byte.Data = RT710_Reg_Arry[10];
				if (I2C_Write(&RT710_Write_Byte) != RT_Success)
					return RT_Fail;
			}
			else
			{
				RT710_Reg_Arry[10] = (RT710_Reg_Arry_Init[10]); //AGC VTH:1.14V  ;  VTL:0.54V
				RT710_Write_Byte.RegAddr = 0x0A;
				RT710_Write_Byte.Data = RT710_Reg_Arry[10];
				if (I2C_Write(&RT710_Write_Byte) != RT_Success)
					return RT_Fail;
			}
		}
		RT710_Write_Byte.RegAddr = 0x02;
		RT710_Write_Byte.Data = RT710_Reg_Arry[2];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
		RT710_Write_Byte.RegAddr = 0x08;
		RT710_Write_Byte.Data = RT710_Reg_Arry[8];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;

		if (RT710_Set_Info.RF_KHz >= 2000000)
		{
			RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xF3) | 0x08;
		}
		else
		{
			RT710_Reg_Arry[14] = (RT710_Reg_Arry[14] & 0xF3) | 0x00;
		}
		RT710_Write_Byte.RegAddr = 0x0E;
		RT710_Write_Byte.Data = RT710_Reg_Arry[14];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
	}
	else
	{
		//Scan_Type
		if (RT710_Set_Info.RT710_Scan_Mode != AUTO_SCAN)
		{
			RT710_Reg_Arry[11] &= 0xFC;
			if (RT710_Set_Info.SymbolRate_Kbps >= 15000)
				RT710_Set_Info.SymbolRate_Kbps += 6000;
		}
		else
		{
			RT710_Reg_Arry[11] |= 0x02;
			RT710_Set_Info.SymbolRate_Kbps += 10000;
		}
		RT710_Write_Byte.RegAddr = 0x0B;
		RT710_Write_Byte.Data = RT710_Reg_Arry[11];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
	}

	switch (RT710_Set_Info.RT710_RollOff_Mode)
	{
		case ROLL_OFF_0_15:
			R710_satellite_bw = (UINT32)((RT710_Set_Info.SymbolRate_Kbps * 115) / 10);
			break;
		case ROLL_OFF_0_20:
			R710_satellite_bw = (UINT32)((RT710_Set_Info.SymbolRate_Kbps * 120) / 10);
			break;
		case ROLL_OFF_0_25:
			R710_satellite_bw = (UINT32)((RT710_Set_Info.SymbolRate_Kbps * 125) / 10);
			break;
		case ROLL_OFF_0_30:
			R710_satellite_bw = (UINT32)((RT710_Set_Info.SymbolRate_Kbps * 130) / 10);
			break;
		case ROLL_OFF_0_35:
			R710_satellite_bw = (UINT32)((RT710_Set_Info.SymbolRate_Kbps * 135) / 10);
			break;
		case ROLL_OFF_0_40:
			R710_satellite_bw = (UINT32)((RT710_Set_Info.SymbolRate_Kbps * 140) / 10);
			break;
	}

	if (R710_satellite_bw != R710_pre_satellite_bw)
	{
		if (Chip_type_flag == TRUE) //TRUE:RT710 ; FALSE;RT720
		{
			if ((R710_satellite_bw) >= 380000)
			{
				fine_tune = 1;
				Coarse = (UINT8)((R710_satellite_bw - 380000) / 17400) + 16;
				if (((R710_satellite_bw - 380000) % 17400) > 0)
					Coarse += 1;
			}
			else if (R710_satellite_bw <= 50000)
			{
				Coarse = 0;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 50000) && (R710_satellite_bw <= 73000))
			{
				Coarse = 0;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 73000) && (R710_satellite_bw <= 96000))
			{
				Coarse = 1;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 96000) && (R710_satellite_bw <= 104000))
			{
				Coarse = 1;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 104000) && (R710_satellite_bw <= 116000))
			{
				Coarse = 2;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 116000) && (R710_satellite_bw <= 126000))
			{
				Coarse = 2;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 126000) && (R710_satellite_bw <= 134000))
			{
				Coarse = 3;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 134000) && (R710_satellite_bw <= 146000))
			{
				Coarse = 3;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 146000) && (R710_satellite_bw <= 158000))
			{
				Coarse = 4;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 158000) && (R710_satellite_bw <= 170000))
			{
				Coarse = 4;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 170000) && (R710_satellite_bw <= 178000))
			{
				Coarse = 5;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 178000) && (R710_satellite_bw <= 190000))
			{
				Coarse = 5;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 190000) && (R710_satellite_bw <= 202000))
			{
				Coarse = 6;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 202000) && (R710_satellite_bw <= 212000))
			{
				Coarse = 6;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 212000) && (R710_satellite_bw <= 218000))
			{
				Coarse = 7;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 218000) && (R710_satellite_bw <= 234000))
			{
				Coarse = 7;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 234000) && (R710_satellite_bw <= 244000))
			{
				Coarse = 9;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 244000) && (R710_satellite_bw <= 246000))
			{
				Coarse = 10;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 246000) && (R710_satellite_bw <= 262000))
			{
				Coarse = 10;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 262000) && (R710_satellite_bw <= 266000))
			{
				Coarse = 11;
				fine_tune = 0;
			}
			else if ((R710_satellite_bw > 266000) && (R710_satellite_bw <= 282000))
			{
				Coarse = 11;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 282000) && (R710_satellite_bw <= 298000))
			{
				Coarse = 12;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 298000) && (R710_satellite_bw <= 318000))
			{
				Coarse = 13;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 318000) && (R710_satellite_bw <= 340000))
			{
				Coarse = 14;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 340000) && (R710_satellite_bw <= 358000))
			{
				Coarse = 15;
				fine_tune = 1;
			}
			else if ((R710_satellite_bw > 358000) && (R710_satellite_bw < 380000))
			{
				Coarse = 16;
				fine_tune = 1;
			}
		}
		else
		{
			if (RT710_Set_Info.RT710_RollOff_Mode >= 2)
				fine_tune = 1;
			else
				fine_tune = 0;

			offset_range = (UINT32)(fine_tune * bw_offset);

			/*
				Symbol Rate <= 15MHz =>  +3MHz
				Symbol Rate 15MHz ~ 20MHz  =>  +2MHz
				Symbol Rate <= 30MHz =>  +1MHz
				*/
			if (RT710_Set_Info.SymbolRate_Kbps <= 15000)
				RT710_Set_Info.SymbolRate_Kbps += 3000;
			else if (RT710_Set_Info.SymbolRate_Kbps <= 20000)
				RT710_Set_Info.SymbolRate_Kbps += 2000;
			else if (RT710_Set_Info.SymbolRate_Kbps <= 30000)
				RT710_Set_Info.SymbolRate_Kbps += 1000;
			else
				RT710_Set_Info.SymbolRate_Kbps += 0;

			if ((RT710_Set_Info.SymbolRate_Kbps * 12) <= (88000 + offset_range))
			{
				Coarse = 0;
			}
			else if ((RT710_Set_Info.SymbolRate_Kbps * 12) > (88000 + offset_range) && (RT710_Set_Info.SymbolRate_Kbps * 12) <= (368000 + offset_range))
			{
				Coarse = (UINT8)(((RT710_Set_Info.SymbolRate_Kbps * 12) - (88000 + offset_range)) / 20000);
				if (((UINT16)((RT710_Set_Info.SymbolRate_Kbps * 12) - (88000 + offset_range)) % 20000) > 0)
					Coarse += 1;
				if (Coarse >= 7)
					Coarse += 1;
			}
			else if ((RT710_Set_Info.SymbolRate_Kbps * 12) > (368000 + offset_range) && (RT710_Set_Info.SymbolRate_Kbps * 12) <= (764000 + offset_range))
			{
				Coarse = (UINT8)(((RT710_Set_Info.SymbolRate_Kbps * 12) - (368000 + offset_range)) / 20000) + 15;
				if (((UINT16)((RT710_Set_Info.SymbolRate_Kbps * 12) - (368000 + offset_range)) % 20000) > 0)
					Coarse += 1;
				if (Coarse >= 33)
					Coarse += 3;
				else if (Coarse >= 29)
					Coarse += 2;
				else if (Coarse >= 27)
					Coarse += 3;
				else if (Coarse >= 24)
					Coarse += 2;
				else if (Coarse >= 19)
					Coarse += 1;
				else
					Coarse += 0;
			}
			else
			{
				Coarse = 42;
			}
		}
		//fine tune and coras filter write
		RT710_Write_Byte.RegAddr = 0x0F;
		RT710_Read_Len.Data[15] &= 0x00;
		RT710_Reg_Arry[15] = ((RT710_Read_Len.Data[15] | (fine_tune)) | (Coarse << 2));
		RT710_Write_Byte.Data = RT710_Reg_Arry[15];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
	}

	R710_pre_satellite_bw = R710_satellite_bw;

	for (icount = 0; icount < 4; icount++)
	{
		R0TOR3_Write_Arry[icount] = RT710_Reg_Arry[icount];
	}

	return RT_Success;
}

RT710_Err_Type RT710_PLL(UINT32 Freq)
{
	UINT8 MixDiv = 2;
	//UINT8 DivBuf = 0;
	UINT8 Ni = 0;
	UINT8 Si = 0;
	UINT8 DivNum = 0;
	UINT8 Nint = 0;
	UINT32 VCO_Min = 2350000;
	UINT32 VCO_Max = VCO_Min * 2;
	UINT32 VCO_Freq = 0;
	UINT32 PLL_Ref = RT710_Xtal;
	UINT32 VCO_Fra = 0; //VCO contribution by SDM (kHz)
	UINT16 Nsdm = 2;
	UINT16 SDM = 0;
	UINT16 SDM16to9 = 0;
	UINT16 SDM8to1 = 0;
	//UINT8  Judge    = 0;
	//UINT8 VCO_fine_tune = 0;
	
	if (Freq >= 2350000)
	{
		VCO_Min = Freq;
	}
	
	while (MixDiv <= 16) // 2 -> 4 -> 8 -> 16
	{
		if (((Freq * MixDiv) >= VCO_Min) && ((Freq * MixDiv) <= VCO_Max)) // Lo_Freq->Freq
		{
			if (MixDiv == 2)
				DivNum = 1;
			else if (MixDiv == 4)
				DivNum = 0;
			else if (MixDiv == 8)
				DivNum = 2;
			else //16
				DivNum = 3;
			break;
		}
		MixDiv = MixDiv << 1;
	}

	//Divider
	RT710_Write_Byte.RegAddr = 0x04;
	RT710_Reg_Arry[4] &= 0xFE;
	RT710_Reg_Arry[4] |= (DivNum & 0x01);
	RT710_Write_Byte.Data = RT710_Reg_Arry[4];
	if (I2C_Write(&RT710_Write_Byte) != RT_Success)
		return RT_Fail;

	if (Chip_type_flag == FALSE) //TRUE:RT710 ; FALSE;RT720
	{
		RT710_Write_Byte.RegAddr = 0x08;
		RT710_Reg_Arry[8] &= 0xEF;
		RT710_Reg_Arry[8] |= ((DivNum & 0x02) >> 1) << 4;
		RT710_Write_Byte.Data = RT710_Reg_Arry[8];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
		//Depend on divider setting
		//If Div /2 or /4 : Div Current 150uA(10); other : Div Current 100uA(01) R4[7:6]
		//If Div /2 or /4 : PLL IQ Buf High(1); other : PLL IQ Buf Low(0) R12[4]
		if (DivNum <= 1) // Div/2 or /4
		{
			RT710_Reg_Arry[4] &= 0x3F;
			RT710_Reg_Arry[4] |= 0x40;
			RT710_Reg_Arry[12] |= 0x10;
		}
		else //Div /8 or /16
		{
			RT710_Reg_Arry[4] &= 0x3F;
			RT710_Reg_Arry[4] |= 0x80;
			RT710_Reg_Arry[12] &= 0xEF;
		}
		RT710_Write_Byte.RegAddr = 0x0C;
		RT710_Write_Byte.Data = RT710_Reg_Arry[12];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
		RT710_Write_Byte.RegAddr = 0x04;
		RT710_Write_Byte.Data = RT710_Reg_Arry[4];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
	}

	VCO_Freq = Freq * MixDiv; // Lo_Freq->Freq
	Nint = (UINT8)(VCO_Freq / 2 / PLL_Ref);
	VCO_Fra = (UINT16)(VCO_Freq - 2 * PLL_Ref * Nint);

	//boundary spur prevention
	if (VCO_Fra < PLL_Ref / 64) // 2*PLL_Ref/128
		VCO_Fra = 0;
	else if (VCO_Fra > PLL_Ref * 127 / 64) // 2*PLL_Ref*127/128
	{
		VCO_Fra = 0;
		Nint++;
	}
	else if ((VCO_Fra > PLL_Ref * 127 / 128) && (VCO_Fra < PLL_Ref)) //> 2*PLL_Ref*127/256,  < 2*PLL_Ref*128/256
		VCO_Fra = PLL_Ref * 127 / 128;								 // VCO_Fra = 2*PLL_Ref*127/256
	else if ((VCO_Fra > PLL_Ref) && (VCO_Fra < PLL_Ref * 129 / 128)) //> 2*PLL_Ref*128/256,  < 2*PLL_Ref*129/256
		VCO_Fra = PLL_Ref * 129 / 128;								 // VCO_Fra = 2*PLL_Ref*129/256
	//else
	//	VCO_Fra = VCO_Fra;

	//N & S
	Ni = (Nint - 13) / 4;
	Si = Nint - 4 * Ni - 13;
	RT710_Write_Byte.RegAddr = 0x05;
	RT710_Reg_Arry[5] = 0x00;
	RT710_Reg_Arry[5] |= (Ni + (Si << 6));
	RT710_Write_Byte.Data = RT710_Reg_Arry[5];
	if (I2C_Write(&RT710_Write_Byte) != RT_Success)
		return RT_Fail;

	//pw_sdm
	RT710_Write_Byte.RegAddr = 0x04;
	RT710_Reg_Arry[4] &= 0xFD;
	if (VCO_Fra == 0)
		RT710_Reg_Arry[4] |= 0x02;
	RT710_Write_Byte.Data = RT710_Reg_Arry[4];
	if (I2C_Write(&RT710_Write_Byte) != RT_Success)
		return RT_Fail;

	//SDM calculator
	while (VCO_Fra > 1)
	{
		if (VCO_Fra > (2 * PLL_Ref / Nsdm))
		{
			SDM = SDM + 32768 / (Nsdm / 2);
			VCO_Fra = VCO_Fra - 2 * PLL_Ref / Nsdm;
			if (Nsdm >= 0x8000)
				break;
		}
		Nsdm = Nsdm << 1;
	}

	SDM16to9 = SDM >> 8;
	SDM8to1 = SDM - (SDM16to9 << 8);

	RT710_Write_Byte.RegAddr = 0x07;
	RT710_Reg_Arry[7] = (UINT8)SDM16to9;
	RT710_Write_Byte.Data = RT710_Reg_Arry[7];
	if (I2C_Write(&RT710_Write_Byte) != RT_Success)
		return RT_Fail;
	RT710_Write_Byte.RegAddr = 0x06;
	RT710_Reg_Arry[6] = (UINT8)SDM8to1;
	RT710_Write_Byte.Data = RT710_Reg_Arry[6];
	if (I2C_Write(&RT710_Write_Byte) != RT_Success)
		return RT_Fail;

	return RT_Success;
}

RT710_Err_Type RT710_Standby(RT710_LoopThrough_Type RT710_LTSel, RT710_ClockOut_Type RT710_CLKSel)
{
	UINT8 RT710_Standby_Reg_Arry[RT710_Reg_Num] = {0xFF, 0x5C, 0x88, 0x30, 0x41, 0xC8, 0xED, 0x25, 0x47, 0xFC, 0x48, 0xA2, 0x08, 0x0F, 0xF3, 0x59};

	int i = 0;
	int icount = 0;

	//Clock out(1=off ; 0=on)
	if (RT710_CLKSel != ClockOutOn)
	{
		RT710_Standby_Reg_Arry[3] |= 0x10;
	}
	else
	{
		RT710_Standby_Reg_Arry[3] &= 0xEF;
	}

	if (Chip_type_flag == FALSE) //TRUE:RT710 ; FALSE;RT720
	{
		RT710_Standby_Reg_Arry[1] |= 0x02; //lna off ;can off together

		RT710_Standby_Reg_Arry[3] |= 0x20; //Xtal ldo off
		RT710_Write_Byte.RegAddr = 0x03;
		RT710_Write_Byte.Data = RT710_Standby_Reg_Arry[3];
		if (I2C_Write(&RT710_Write_Byte) != RT_Success)
			return RT_Fail;
	}

	RT710_Write_Len.RegAddr = 0x00;
	RT710_Write_Len.Len = 0x10;
	for (i = 0; i < RT710_Write_Len.Len; i++)
	{
		RT710_Write_Len.Data[i] = RT710_Standby_Reg_Arry[i];
	}

	if (I2C_Write_Len(&RT710_Write_Len) != RT_Success)
		return RT_Fail;

	R710_Initial_done_flag = FALSE;

	for (icount = 0; icount < 4; icount++)
	{
		R0TOR3_Write_Arry[icount] = RT710_Standby_Reg_Arry[icount];
	}

	return RT_Success;
}

//------------------------------------------------------------------//
//  RT710_PLL_Lock( ): Read PLL lock status (R2[7])        //
//  Return: 1: PLL lock                                                    //
//          0: PLL unlock                                                //
//------------------------------------------------------------------//
UINT8 RT710_PLL_Lock(void)
{
	UINT8 fg_lock = 0;
	I2C_LEN_TYPE Dlg_I2C_Len;

	Dlg_I2C_Len.RegAddr = 0x00;
	Dlg_I2C_Len.Len = 3;
	if (I2C_Read_Len(&Dlg_I2C_Len) != RT_Success)
	{
		I2C_Read_Len(&Dlg_I2C_Len);
	}

	if ((Dlg_I2C_Len.Data[2] & 0x80) == 0x00)
		fg_lock = 0;
	else
		fg_lock = 1;

	return fg_lock;
}

RT710_Err_Type RT710_GetRfGain(RT710_RF_Gain_Info *RT710_rf_gain)
{
	I2C_LEN_TYPE Dlg_I2C_Len;
	Dlg_I2C_Len.RegAddr = 0x00;
	Dlg_I2C_Len.Len = 0x04;
	I2C_Read_Len(&Dlg_I2C_Len);				 // read data length
	RT710_rf_gain->RF_gain = (Dlg_I2C_Len.Data[1] >> 4); //get rf gain
	RT710_rf_gain->RF_gain += ((Dlg_I2C_Len.Data[1] & 0x01) << 4);

	/*0~5: mixeramp
	  6~7: mix-buf
	  29~30:mix-buf
	  other:lna
	*/
	if (Chip_type_flag == TRUE)
	{
		if (RT710_rf_gain->RF_gain <= 2)
		{
			RT710_rf_gain->RF_gain = 0;
		}
		else if (RT710_rf_gain->RF_gain > 2 && RT710_rf_gain->RF_gain <= 9)
		{
			RT710_rf_gain->RF_gain -= 2;
		}
		else if (RT710_rf_gain->RF_gain > 9 && RT710_rf_gain->RF_gain <= 12)
		{
			RT710_rf_gain->RF_gain = 7;
		}
		else if (RT710_rf_gain->RF_gain > 12 && RT710_rf_gain->RF_gain <= 22)
		{
			RT710_rf_gain->RF_gain -= 5;
		}
		else if (RT710_rf_gain->RF_gain > 22)
		{
			RT710_rf_gain->RF_gain = 18;
		}
	}

	return RT_Success;
}

//----------------------------------------------------------------------//
//  RT710_GetRfRssi( ): Get RF RSSI                                      //
//  1st parameter: input RF Freq    (KHz)                               //
//  2rd parameter: output signal level (dBm*1000)                       //
//  3th parameter: output RF max gain indicator (0:min gain, 1:max gain, 2:active gain)	//
//-----------------------------------------------------------------------//
RT710_Err_Type RT710_GetRfRssi(UINT32 RF_Freq_Khz, INT32 *RfLevelDbm, UINT8 *fgRfGainflag)
{
	RT710_RF_Gain_Info rf_gain_info;
	UINT8 u1Gain1;
	UINT16 acc_lna_gain;
	UINT16 rf_total_gain;
	UINT16 u2FreqFactor;
	INT32 rf_rssi;
	INT32 fine_tune = 0; //for find tune

	RT710_GetRfGain(&rf_gain_info);

	u1Gain1 = rf_gain_info.RF_gain;

	//max gain indicator
	if (((u1Gain1 == 18) && (Chip_type_flag == TRUE)) || ((u1Gain1 == 31) && (Chip_type_flag == FALSE)))
		*fgRfGainflag = 1;
	else if (u1Gain1 == 0)
		*fgRfGainflag = 0;
	else
		*fgRfGainflag = 2;

	u2FreqFactor = 0;

	//Select LNA freq table
	if (Chip_type_flag == FALSE)
	{
		u2FreqFactor = 70;
	}
	else if (RF_Freq_Khz < 1200000) //<1200M
	{
		u2FreqFactor = 190;
	}
	else if ((RF_Freq_Khz >= 1200000) && (RF_Freq_Khz < 1800000)) //1200~1800M
	{
		u2FreqFactor = 170;
	}
	else // >=2000MHz
	{
		u2FreqFactor = 140;
	}

	//LNA Gain
	if (Chip_type_flag == FALSE)
		acc_lna_gain = RT710_Lna2_Acc_Gain[u1Gain1];
	else
		acc_lna_gain = RT710_Lna_Acc_Gain[u1Gain1];

	//Add Rf Buf and Mixer Gain
	rf_total_gain = acc_lna_gain;

	rf_rssi = fine_tune - (INT32)(rf_total_gain + u2FreqFactor);
	*RfLevelDbm = (rf_rssi * 100);

	return RT_Success;
}

MT_FE_RET mt_fe_tn_init_RT720_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	//RT710_Scan_Type RT710_Scan_Mode; //only support RT720

	RT720_Tuner.RF_KHz					 = handle->m_device_ss2.input_params.input_freq_kHz;
	RT720_Tuner.SymbolRate_Kbps			 = handle->m_device_ss2.input_params.symbol_rate_KSs;
	RT720_Tuner.RT710_RollOff_Mode		 = ROLL_OFF_0_25;
	RT720_Tuner.RT710_LoopThrough_Mode	 = LOOP_THROUGH;	// SIGLE_IN;
	RT720_Tuner.RT710_ClockOut_Mode		 = ClockOutOn;		// ClockOutOff;
	RT720_Tuner.RT710_OutputSignal_Mode	 = DifferentialOut;	// SingleOut;
	RT720_Tuner.RT710_AGC_Mode			 = AGC_Negative;	// AGC_Postive;
	RT720_Tuner.RT710_AttenVga_Mode		 = ATTENVGAOFF;		// ATTENVGAON;
	RT720_Tuner.R710_FineGain			 = FINEGAIN_3DB;
	RT720_Tuner.RT710_Scan_Mode			 = MANUAL_SCAN;		// AUTO_SCAN;

	//RT710_Setting(RT720_Tuner);

	RT710_ADDRESS = handle->m_device_ss2.tuner_cfg.tuner_dev_addr;

	//printf("%s[%d] ---- RT710_ADDRESS = 0x%02x, tuner_addr = 0x%02x\n", __FUNCTION__, __LINE__, RT710_ADDRESS, handle->m_device_ss2.tuner_cfg.tuner_dev_addr);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_set_freq_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	RT710_Err_Type ret = RT_Success;

	//ret = RT710_PLL(freq_KHz);

	RT720_Tuner.RF_KHz					 = freq_KHz;
	RT720_Tuner.SymbolRate_Kbps			 = sym_rate_KSs;

	RT710_ADDRESS = handle->m_device_ss2.tuner_cfg.tuner_dev_addr;

	//printk("%s[%d] ---- RT710_ADDRESS = 0x%02x, tuner_addr = 0x%02x\n", __FUNCTION__, __LINE__, RT710_ADDRESS, handle->m_device_ss2.tuner_cfg.tuner_dev_addr);

	ret = RT710_Setting(RT720_Tuner);

	return (ret == RT_Success) ? MtFeErr_Ok : MtFeErr_Fail;
}

MT_FE_RET mt_fe_tn_get_gain_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, U32 *p_gain)
{
	//U32 uiGain = 0;
	U8 iRfGainIndicator = 0;
	INT32 iRfLeveldBm = 0;
	int iGain = 0;//, iRfLeveldBuV = 0, iStrength = 0;
	U8 reg_0x3f, reg_0x40, reg_0x41;
	U32 tmp;

	_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0x3f, &reg_0x3f);
	_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0x40, &reg_0x40);
	_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0x41, &reg_0x41);

	tmp = (reg_0x3f << 5) | (reg_0x40 & 0x1F);

	iGain = tmp;

	RT710_GetRfRssi(handle->m_device_ss2.input_params.input_freq_kHz, &iRfLeveldBm, &iRfGainIndicator);

	iRfLeveldBm /= 1000;

	iRfLeveldBm -= 2;
	iRfLeveldBm -= (iGain - 1038) / 50;

	*p_gain = -iRfLeveldBm;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_strength_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, S8 *p_strength)
{
#if 0
	U32 uiGain = 0;
	U8 iRfGainIndicator = 0;
	int iRfLeveldBm = 0, iGain = 0, iRfLeveldBuV = 0, iStrength = 0;
	U8 reg_0x3f, reg_0x40, reg_0x41;
	U32 tmp;

	_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0x3f, &reg_0x3f);
	_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0x40, &reg_0x40);
	_mt_fe_dmd_get_reg_ss2_cs8800(handle, 0x41, &reg_0x41);

	tmp = (reg_0x3f << 5) | (reg_0x40 & 0x1F);

	iGain = tmp;

	RT710_GetRfRssi(handle->m_device_ss2.input_params.input_freq_kHz, &iRfLeveldBm, &iRfGainIndicator);

	iRfLeveldBm /= 1000;

	iRfLeveldBm -= 2;
	iRfLeveldBm -= (iGain - 1038) / 50;
#endif
	int iRfLeveldBm = 0, iRfLeveldBuV = 0 ;
	U32 uiGain = 0;

	mt_fe_tn_get_gain_RT720_cs8800(handle, &uiGain);

	iRfLeveldBm = (int)(-uiGain);

	iRfLeveldBuV = iRfLeveldBm + 107;

	//printf("%s[%d] ---- gain = %d, level = %d dBm\n", __FUNCTION__, __LINE__, uiGain, iRfLeveldBm);
	//printk("%s[%d] ---- gain = %d, level = %d dBm\n", __FUNCTION__, __LINE__, uiGain, iRfLeveldBm);

	if(iRfLeveldBuV >= 100)	iRfLeveldBuV = 100;
	if(iRfLeveldBuV <= 0)	iRfLeveldBuV = 0;

	*p_strength = iRfLeveldBuV;

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_tuner_freq_offset_RT720_cs8800(MT_FE_CS8800_Device_Handle handle, S32 *p_offset)
{
	*p_offset = 0;

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_tn_sleep_RT720_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	RT710_Standby(RT720_Tuner.RT710_LoopThrough_Mode, RT720_Tuner.RT710_ClockOut_Mode);

	return MtFeErr_Ok;
}


MT_FE_RET mt_fe_tn_wake_up_RT720_cs8800(MT_FE_CS8800_Device_Handle handle)
{
	S16 offset_KHz = 0;

	mt_fe_tn_set_freq_RT720_cs8800(handle, handle->m_device_ss2.input_params.input_freq_kHz, handle->m_device_ss2.input_params.symbol_rate_KSs, offset_KHz);

	return MtFeErr_Ok;
}

extern MT_FE_CS8800_Device_Handle mt_fe_cs8800_get_handle(void);

void mt_fe_cs8800_tn_attach_RT720(void)
{
	MT_FE_CS8800_Device_Handle handle = mt_fe_cs8800_get_handle();

	handle->m_device_ss2.tuner_cfg.tuner_type = MtFeTn_RT720;

	if (handle->m_device_ss2.tuner_cfg.tuner_dev_addr == 0x00)
		handle->m_device_ss2.tuner_cfg.tuner_dev_addr = 0x34;

	handle->m_device_ss2.tuner_cfg.tuner_init_ok = 0;
	handle->m_device_ss2.tuner_cfg.tuner_open = 0;
	handle->m_device_ss2.tuner_cfg.tuner_init = (MT_FE_RET(*)(void *))mt_fe_tn_init_RT720_cs8800;
	handle->m_device_ss2.tuner_cfg.tuner_set = (MT_FE_RET(*)(void *, U32, U32, S16))mt_fe_tn_set_freq_RT720_cs8800;
	handle->m_device_ss2.tuner_cfg.tuner_strength = (MT_FE_RET(*)(void *, S8 *))mt_fe_tn_get_strength_RT720_cs8800;
	handle->m_device_ss2.tuner_cfg.tuner_sleep = (MT_FE_RET(*)(void *))mt_fe_tn_sleep_RT720_cs8800;
	handle->m_device_ss2.tuner_cfg.tuner_wakeup = (MT_FE_RET(*)(void *))mt_fe_tn_wake_up_RT720_cs8800;
	handle->m_device_ss2.tuner_cfg.tuner_get_offset = (MT_FE_RET(*)(void *, S32 *))mt_fe_tn_get_tuner_freq_offset_RT720_cs8800;
	handle->m_device_ss2.tuner_cfg.tuner_get_gain = (MT_FE_RET(*)(void *, U32 *))mt_fe_tn_get_gain_RT720_cs8800;

	handle->m_device_ss2.tuner_cfg.tuner_set_application = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_loop_through = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_xtal = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_set_clkout = NULL;
	handle->m_device_ss2.tuner_cfg.tuner_get_diagnose_info = NULL;

	handle->m_device_ss2.board_cfg.bIQInverted = TRUE;
	handle->m_device_ss2.board_cfg.bAGCPolar = FALSE;//TRUE;

	handle->m_device_ss2.bs_cfg.bs_symrate = 40000;	// BLINDSCAN_SYMRATEKSs;

	return;
}

#endif

