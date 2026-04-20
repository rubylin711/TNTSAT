/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/*****************************************************************************
*    Airoha DVB-S2 AV2018 Tuner sample code (Mulitple API Version)
*
*    History:
*      	Date         Version    Reason
*
*	  ============	=========	=================
*	1.May.25.2015	 Ver 1.0	Separate different application interfaces
*****************************************************************************/

//#include "AVhost.h"
#include "mt_fe_def_ds6113.h"
#include "mt_fe_i2c_ds6113.h"

#include "mt_fe_tn_av2018.h"


extern MT_FE_DS6113_Device_Handle ds6113_handle;


static TunerPara AVtuner;


static TUNER_ErrorCode Tuner_Set_RXout_PGA(pTunerPara pAVtuner);


/*********************************************************************************
* Tuner IIC interface writting function
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Paramter2:    UINT8      : register start address
* Paramter3:    UINT8*     : pointer of sending register data buffer
* Paramter4:    UINT8      : the length of sending register data
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. User define
**********************************************************************************/
#if 1
static TUNER_ErrorCode Tuner_I2C_Write(pTunerPara pAVtuner, UINT8 reg_start, UINT8* buff, UINT8 length)
{
	UINT8 err = 0;
	UINT8 buf[64];
	UINT8 i = 0;

	buf[0] = reg_start;

	for(i = 1; i <= length; i ++)
		buf[i] = buff[i - 1];

	ds6113_handle->tn_write(ds6113_handle, buf, (U16)(length + 1));

	return err;
}
#else
static TUNER_ErrorCode Tuner_I2C_Write (pTunerPara pAVtuner, UINT8 reg_start, UINT8* buff, UINT8 length)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT8 queue[16];
	UINT8 i;

	queue[0] = pAVtuner->I2C_ADDR; // I2C write address
	queue[1] = reg_start;         // register start address

	for (i = 0; i < length; i ++)
	{
		queue[2 + i]= buff[i];
	}

	ds6113_handle->tn_write(ds6113_handle, queue, (U16)(length + 2));


#if 0

	printf("I2C write queue");
	for (i=0;i<length+2;i++)
		printf("_%02x_", queue[i]);
	printf("\n");
#endif

	return result;
}
#endif


#if 0

/*********************************************************************************
* Tuner IIC interface reading function
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Paramter2:    UINT8      : register start address
* Paramter3:    UINT8*     : pointer of reading register data buffer
* Paramter4:    UINT8      : the length of reading register data
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. User define
**********************************************************************************/
static TUNER_ErrorCode Tuner_I2C_Read (pTunerPara pAVtuner, UINT8 reg_start, UINT8* buff, UINT8 length)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	//UINT8 queue[16];
	//UINT8 i;

	ds6113_handle->tn_read(ds6113_handle, reg_start, buff, length);

#if 0
	queue[0]=pAVtuner->I2C_ADDR; // I2C write address
	queue[1]=reg_start;			// register start address
	queue[2]=pAVtuner->I2C_ADDR | 0x01; // I2C read address

	printf("I2C read queue");
	for (i=0;i<length+3;i++)
		printf("_%02x_", queue[i]);
	printf("\n");

	for (i=0;i<length;i++)
	{
		buff[i] = queue[3+i];
	}
#endif

	return result;
}
#endif


/*********************************************************************************
* Time delay function
* Paramter1:    UINT32     : delay time. Unit is millisecond
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. User define
**********************************************************************************/
static TUNER_ErrorCode Time_DELAY_MS (UINT32 ms)
{
	TUNER_ErrorCode result = Tuner_No_Error;

	ds6113_handle->mt_sleep(ms);

	//printf("Time dalay %02d ms\n",ms);
	return result;
}

#if 0
/*******************************************************************
* Tuner parameter initialization:
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description: Initialize all the parameters in the data structure of pAVtuner
********************************************************************/
static TUNER_ErrorCode Tuner_Parameter_Initial (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;

	pAVtuner->crystal_khz   = 27000;				// (Unit: KHz)Tuner_crystal

	pAVtuner->I2C_ADDR      = ADDR0_H;			// Tuner I2C address at write mode
	pAVtuner->IQmode	    = Single;			// IQ_MODE
	pAVtuner->PGA_Gain      = PGA_1_5dB ;		// PGA_GAIN
	pAVtuner->PGA_Current   = PGA_1_5mA ;		// PGA_DRIVE_CURRENT
	pAVtuner->XO_Current    = HIGH;				// XO_DRIVE_CURRENT
	pAVtuner->XO_EN		    = XO_ON;			// XO_ENABLE Setting
	pAVtuner->RFLP_EN       = RFLP_OFF;			// RFLP_ENABLE Setting
	pAVtuner->PD_EN         = Wake_Up;			// SOFTWARE_POWERDOWN
	pAVtuner->FT            = FT_ON;				// TUNER_Fine_Tune
	pAVtuner->blind_scan    = 0;					// blind_scan

	return result;
}

/*******************************************************************************
* Tuner Power-ON registers initialization
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description: Initialize all registers of the Tuner
********************************************************************************/
static TUNER_ErrorCode Tuner_Register_Initial (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;
#if 0
	UINT8 reg[50];

	reg[0]= 0x50; //2150M
	reg[1]= 0xA1; //2150M
	reg[2]= 0x2F; //2150M
	reg[3]= 0x54; //2150M
	reg[4]= 0x1f;
	reg[5]= 0xa3;
	reg[6]= 0xfd;
	reg[7]= 0x58;
	reg[8]= (0x04)|(pAVtuner->PGA_Gain <<3)|(pAVtuner->PGA_Current);

	if( pAVtuner->crystal_khz < 25000 )
		reg[9]= (0x00)|(pAVtuner->XO_Current<<6); // ctrl_clk Divider=/1
	else
		reg[9]= (0x02)|(pAVtuner->XO_Current<<6); // ctrl_clk Divider=/2

	reg[10]= 0x88;
	reg[11]= 0xb4;
	reg[12]= (0x16)|(pAVtuner->XO_EN <<7)|(pAVtuner->RFLP_EN <<6)|(pAVtuner->PD_EN <<5);
	reg[13]= 0x40;
	reg[14]= 0x94;
	reg[15]= 0x4a;
	reg[16]= 0x66;
	reg[17]= 0x40;
	reg[18]= 0x80;
	reg[19]= 0x2b;
	reg[20]= 0x6a;
	reg[21]= 0x50;
	reg[22]= 0x91;
	reg[23]= 0x27;
	reg[24]= 0x8f;
	reg[25]= 0xcc;
	reg[26]= 0x21;
	reg[27]= 0x10;
	reg[28]= 0x80;
	reg[29]= 0xE2;
	reg[30]= 0xf5;
	reg[31]= 0x7f;
	reg[32]= 0x4a;
	reg[33]= 0x9b;
	reg[34]= 0xe0;
	reg[35]= 0xe0;
	reg[36]= 0x36;
	reg[37]= (UINT8)(pAVtuner->FT);
	reg[38]= 0xab;
	reg[39]= 0x97;
	reg[40]= 0xc5;
	reg[41]= 0xa8;

	/* Sequence 1*/
	/* Send Reg0 ->Reg11*/
	result = Tuner_I2C_Write(pAVtuner,0,reg,12);
	if(result!=Tuner_No_Error){ return result; }

	/* Sequence 2*/
	/* Send Reg13 ->Reg24*/
	result = Tuner_I2C_Write(pAVtuner,13,reg+13,12);
	if(result!=Tuner_No_Error){ return result; }

	/* Send Reg25 ->Reg35*/
	result = Tuner_I2C_Write(pAVtuner,25,reg+25,11);
	if(result!=Tuner_No_Error){ return result; }

	/* Send Reg36 ->Reg41*/
	result = Tuner_I2C_Write(pAVtuner,36,reg+36,6);
	if(result!=Tuner_No_Error){ return result; }

	/* Sequence 3*/
	/* Send reg12*/
	result = Tuner_I2C_Write(pAVtuner,12,reg+12,1);
	if(result!=Tuner_No_Error){ return result; }

	/* Time delay ms*/
	Time_DELAY_MS(100);
	/*Reinitial again*/
	{
		/* Sequence 1*/
		/* Send Reg0 ->Reg11*/
		result = Tuner_I2C_Write(pAVtuner,0,reg,12);
		if(result!=Tuner_No_Error){ return result; }

		/* Sequence 2*/
		/* Send Reg13 ->Reg24*/
		result = Tuner_I2C_Write(pAVtuner,13,reg+13,12);
		if(result!=Tuner_No_Error){ return result; }

		/* Send Reg25 ->Reg35*/
		result = Tuner_I2C_Write(pAVtuner,25,reg+25,11);
		if(result!=Tuner_No_Error){ return result; }

		/* Send Reg36 ->Reg41*/
		result = Tuner_I2C_Write(pAVtuner,36,reg+36,6);
		if(result!=Tuner_No_Error){ return result; }

		/* Sequence 3*/
		/* Send reg12*/
		result = Tuner_I2C_Write(pAVtuner,12,reg+12,1);
	}
	/* Time delay ms*/
	Time_DELAY_MS(5);
#endif

	return result;
}
#endif


/*********************************************************************************
* Channel frequency setting
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Paramter2:    UINT32 : channel frequency (unit: MHz)
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. Send reg3,2,1,0 to set tuner at the channel frequency.
*    reg0 is int<7:0>; reg1 is frac<16:9>; reg2 is frac<8:1>; reg3_D7 is frac<0>
*	 For Tuner model name =1, reg3_D2 is IQ mode selection.
* 2. If XO is not default 27MHz, need to manually select VCO Divider.
*    The VCO Divider switching point is freqency=1167.75MHz,
*    and send Reg16 before Reg0~3
**********************************************************************************/
#if 1
static TUNER_ErrorCode Tuner_Set_Channel_Frequency(pTunerPara pAVtuner, UINT32 channel_freq_khz, UINT32 bb_sym)
{
	//TUNER_ErrorCode result = Tuner_No_Error;

	UINT32 channel_freq = (channel_freq_khz + 500) / 1000;

	UINT32 	tuner_crystal = 27;//pAVtuner->crystal_khz / 1000;

	unsigned char reg[50];
	//unsigned char i;
	unsigned int fracN;
	unsigned int BW;
	unsigned int BF;
	//unsigned int success = 1;

	// Auto-scan mode flag. Default is not at auto-scan mode
	unsigned char auto_scan = 0;

	// Register initial flag. Static constant for first entry
	static unsigned char tuner_initial = 0;

	// At Power ON, tuner_initial = 0, will run sequence 1~3 at first call of "Tuner_control().
	//if (tuner_initial == 0)
	{
		//Initail registers R0~R41
		reg[0]=(char) (0x38);
		reg[1]=(char) (0x00);
		reg[2]=(char) (0x00);
		reg[3]=(char) (0x50);
		reg[4]=(char) (0x1f);
		reg[5]=(char) (0xa3);
		reg[6]=(char) (0xfd);
		reg[7]=(char) (0x58);
		reg[8]=(char) (0xc7);
		reg[9]=(char) (0x82);
		reg[10]=(char) (0x88);
		reg[11]=(char) (0xb4);

		reg[12]=(char) (0xd6); //RFLP=ON at Power on initial
		//reg[12]=(char) (0x96); //RFLP=OFF at Power on initial

		reg[13]=(char) (0x40);

		// reg[14]=(char) (0x5b); //AV2010
		reg[14]=(char) (0x94); //AV2011

		// reg[15]=(char) (0x6a); //AV2010
		reg[15]=(char) (0x4a); //AV2011

		reg[16]=(char) (0x66);
		reg[17]=(char) (0x40);
		reg[18]=(char) (0x80);
		reg[19]=(char) (0x2b);
		reg[20]=(char) (0x6a);
		reg[21]=(char) (0x50);
		reg[22]=(char) (0x91);
		reg[23]=(char) (0x27);
		reg[24]=(char) (0x8f);
		reg[25]=(char) (0xcc);
		reg[26]=(char) (0x21);
		reg[27]=(char) (0x10);
		reg[28]=(char) (0x80);
		reg[29]=(char) (0x02);
		reg[30]=(char) (0xf5);
		reg[31]=(char) (0x7f);
		reg[32]=(char) (0x4a);
		reg[33]=(char) (0x9b);
		reg[34]=(char) (0xe0);
		reg[35]=(char) (0xe0);
		reg[36]=(char) (0x36);

		reg[37]=(char) (0x00);	// Disble FT function at Power on initial

		reg[38]=(char) (0xab);
		reg[39]=(char) (0x97);
		reg[40]=(char) (0xc5);
		reg[41]=(char) (0xa8);

		// Sequence 1
		// Send Reg0 ->Reg11
		if(Tuner_I2C_Write(pAVtuner, 0, reg, 12))return 0;

		// Sequence 2
		// Send Reg13 ->Reg24
		Tuner_I2C_Write(pAVtuner, 13, reg+13, 12);
		// Send Reg25 ->Reg35
		Tuner_I2C_Write(pAVtuner, 25, reg+25, 11);
		// Send Reg36 ->Reg41
		Tuner_I2C_Write(pAVtuner, 36, reg+36, 6);

		// Sequence 3
		// Send reg12
		Tuner_I2C_Write(pAVtuner, 12, reg+12, 1);

		Time_DELAY_MS(100);
		//Reinitial again
		{
			// Sequence 1
			// Send Reg0 ->Reg11
			Tuner_I2C_Write(pAVtuner, 0, reg, 12);

			// Sequence 2
			// Send Reg13 ->Reg24
			Tuner_I2C_Write(pAVtuner, 13, reg+13, 12);
			// Send Reg25 ->Reg35
			Tuner_I2C_Write(pAVtuner, 25, reg+25, 11);
			// Send Reg36 ->Reg41
			Tuner_I2C_Write(pAVtuner, 36, reg+36, 6);

			// Sequence 3
			// Send reg12
			Tuner_I2C_Write(pAVtuner, 12, reg+12, 1);
		}

		// After power on initial
		tuner_initial = 1;

		// Time delay 4ms
		Time_DELAY_MS(4);
	}

	// Channel Frequency Calculation.
	fracN = (channel_freq + tuner_crystal / 2) / tuner_crystal;
	if(fracN > 0xff)
		fracN = 0xff;

	reg[0]=(char)(fracN & 0xff);
	fracN = (channel_freq << 17) / tuner_crystal;
	fracN = fracN & 0x1ffff;
	reg[1]=(char)((fracN >> 9) & 0xff);
	reg[2]=(char)((fracN >> 1) & 0xff);

	/******************************************************
		reg[3]_D7 is frac<0>, D6~D0 is 0x50, For differential RXIQ out
		reg[3]_D7 is frac<0>, D6~D0 is 0x54, For single RXIQ out  //EDS11660FNP
	******************************************************/
	reg[3]=(char) (((fracN<<7)&0x80) | 0x54);

	// Channel Filter Bandwidth Calculation.

	// rolloff is 35%
	BW = bb_sym*135/200;
	// add 6M when Rs<6.5M for low IF
	if(bb_sym<6500)
		BW = BW + 6000;
	// add 2M for LNB frequency shifting
	BW = BW + 2000;
	// add 8% margin since the calculated fc of BB Auto-scanning is not very accurate
	BW = BW*108/100;
	// Bandwidth can be tuned from 4M to 40M
	if( BW< 4000)
		BW = 4000;

	if( BW> 40000)
		BW = 40000;


	//BW(MHz) * 1.27 / 211KHz
	BF = (BW*127 + 21100/2) / (21100);
	reg[5] = (unsigned char)BF;


	if (bb_sym == 0 || bb_sym == 45000)
	{
		auto_scan = 1;
		reg[5] = 0xf1; //F1=BW=40M A3:BW=27MHz
	}

	if(auto_scan)
	{
		Tuner_I2C_Write(pAVtuner, 0, reg,4);

		// Time delay 4ms
		Time_DELAY_MS(4);

		// Sequence 5
		// Send Reg5
		Tuner_I2C_Write(pAVtuner, 5, reg+5, 1);

		// Fine-tune Function Control
		//Auto-scan mode. FT_block=1, FT_EN=0, FT_hold=1
		reg[37] = 0x05;
		Tuner_I2C_Write(pAVtuner, 37, reg+37, 1);
		// Time delay 4ms
		Time_DELAY_MS(4);
	}
	else
	{
		Tuner_I2C_Write(pAVtuner, 0, reg,4);

		// Time delay 4ms
		Time_DELAY_MS(4);

		// Sequence 5
		// Send Reg5
		Tuner_I2C_Write(pAVtuner, 5, reg+5, 1);

		// Fine-tune Function Control
		// Non-auto-scan mode. FT_block=1, FT_EN=1, FT_hold=0
		reg[37] = 0x06;
		Tuner_I2C_Write(pAVtuner, 37, reg+37, 1);
		// Fine-tune function is starting tracking after sending reg[37].
		// Make sure the RFAGC do not have a sharp jump.

		reg[12] = 0x96; //Disable RFLP at Lock Channel sequence after reg[37]	 //EDS11660FNP
		Tuner_I2C_Write(pAVtuner, 12, reg+12, 1);

		// Time delay 4ms
		Time_DELAY_MS(4);
	}

	Tuner_Set_RXout_PGA(pAVtuner);

	return 1;
}

#else
static TUNER_ErrorCode Tuner_Set_Channel_Frequency(pTunerPara pAVtuner, UINT32 channel_freq_khz)
{
	UINT8 reg0;
	UINT8 reg1;
	UINT8 reg2;
	UINT8 reg3;
	UINT8 reg16;
	UINT8 reg30;
	UINT32 fracN;
	UINT32 XO_khz;

	XO_khz = pAVtuner->crystal_khz;

	if((channel_freq_khz%XO_khz) > (XO_khz-501))
	{
		channel_freq_khz /= 1000;
		channel_freq_khz *= 1000;
		channel_freq_khz += 1000;
	}
	else if((channel_freq_khz%XO_khz) < 500)
	{
		channel_freq_khz /= 1000;
		channel_freq_khz *= 1000;
	}
	else
	{}

	if( XO_khz != 27000 )
	{
		if(channel_freq_khz > 1167750)
			reg16 = 0x56; // VCO Divider=/2
		else
			reg16 = 0x46; // VCO Divider=/4

		/* Send Reg16*/
		result=Tuner_I2C_Write(pAVtuner,16,&reg16,1);
		if(result!=Tuner_No_Error){ return result; }
	}

	fracN = (channel_freq_khz + XO_khz/2)/XO_khz;
	if(fracN > 0xff)
		fracN = 0xff;
	reg0=(UINT8) (fracN & 0xff);
	fracN = ((channel_freq_khz/100)<<17)/(XO_khz/100);
	fracN = fracN & 0x1ffff;
	reg1=(UINT8) ((fracN>>9)&0xff);
	reg2=(UINT8) ((fracN>>1)&0xff);

	reg3= (0x50)|(fracN<<7)|(pAVtuner->IQmode<<2);
	/* Sequence 4*/

	/* Send Reg3*/
	result = Tuner_I2C_Write(pAVtuner,3,&reg3,1);
	if(result!=Tuner_No_Error){ return result; }
	/* Send Reg2*/
	result = Tuner_I2C_Write(pAVtuner,2,&reg2,1);
	if(result!=Tuner_No_Error){ return result; }
	/* Send Reg1*/
	result = Tuner_I2C_Write(pAVtuner,1,&reg1,1);
	if(result!=Tuner_No_Error){ return result; }
	/* Send Reg0*/
	result = Tuner_I2C_Write(pAVtuner,0,&reg0,1);
	/* Time delay ms*/

	Time_DELAY_MS(100);

	/* Send Reg3*/
	result = Tuner_I2C_Write(pAVtuner,3,&reg3,1);
	if(result!=Tuner_No_Error){ return result; }
	/* Send Reg2*/
	result = Tuner_I2C_Write(pAVtuner,2,&reg2,1);
	if(result!=Tuner_No_Error){ return result; }
	/* Send Reg1*/
	result = Tuner_I2C_Write(pAVtuner,1,&reg1,1);
	if(result!=Tuner_No_Error){ return result; }
	/* Send Reg0*/
	result = Tuner_I2C_Write(pAVtuner,0,&reg0,1);
	/* Time delay ms*/

	Time_DELAY_MS(4);

	if( (channel_freq_khz%XO_khz)==0)
	{
		reg30 = 0xe5;
	}
	else
	{
		reg30 = 0xf5;
	}
	result = Tuner_I2C_Write(pAVtuner,30,&reg30,1);
	if(result!=Tuner_No_Error){ return result; }

	Time_DELAY_MS(4);

	return result;
}
#endif

#if 0
/*********************************************************************************
* ZIF low pass filter bandwidth setting
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Paramter2:    UINT32 : LPF bandwidth (unit: kHz)
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. Calculate the filter reference clk from XO. Transfer Filter BW setting to register5
* 2. Add Fine-tune function after Bandwidth setting.
**********************************************************************************/
static TUNER_ErrorCode Tuner_Set_Filter_Bandwith (pTunerPara pAVtuner, UINT32 filter_BW_khz)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT32 ctrl_clk_khz;
	UINT32 BW_CLK_khz;
	UINT32 XO_khz;
	UINT32 filter_khz;
	UINT32 BF;
	UINT8 reg5;

	XO_khz = pAVtuner->crystal_khz;
	filter_khz = filter_BW_khz;
	if( XO_khz < 25000 )
		ctrl_clk_khz = XO_khz;
	else
		ctrl_clk_khz = XO_khz/2;
	/* Filter Bandwidth reference CLK calculation */
	BW_CLK_khz = (ctrl_clk_khz +64/2)/64;
	/* BF = filter_BW (KHz) * 1.27 / BW_CLK (KHz)*/
	BF = (filter_khz*127 + 100*BW_CLK_khz/2)/(100*BW_CLK_khz);
	if(BF > 0xff)
		BF = 0xff;
	reg5 = (UINT8)BF;
	/* Sequence 5*/
	/* Send Reg5*/
	result = Tuner_I2C_Write(pAVtuner,5, &reg5, 1);
	if(result!=Tuner_No_Error){ return result; }
	/* Time delay ms*/
	Time_DELAY_MS(4);

	/* Reset FT after Filter BW setting */
	if (pAVtuner->blind_scan == 1)
	{
		/* choose one for different blind scan mechanism */
		// pAVtuner->FT = FT_OFF;
		// pAVtuner->FT = FT_Hold;
		pAVtuner->FT = FT_ON;
	}
	else
	{
		pAVtuner->FT = FT_Delay_ON;
	}
	result = Tuner_Set_Fine_Tune (pAVtuner);
	if(result!=Tuner_No_Error){ return result; }

	return result;
}

/*********************************************************************************
* Fine-tune function setting
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. Fine-tune the gain for sensitivity and linearity.
*    reg[37]_D2 is FT_block,Tp=12ms. reg[37]_D1 is FT_EN. reg[37]_D0 is FT_hold.
* 2. FT_OFF      = 0x00  : FT_block=0, FT_EN=0, FT_hold=0. The Gain is at best sensitivity gain.
* 3. FT_ON       = 0x02  : FT_block=0, FT_EN=1, FT_hold=0. Turn on to fine-tuned gain continuously between sensitivity and linearity.
* 4. FT_Hold     = 0x03  : FT_block=0, FT_EN=1, FT_hold=1. Stop fine-tuning the gain and hold the current fine-tuned gain
* 5. FT_Delay_ON = 0x06	 : FT_block=1, FT_EN=1, FT_hold=0. FT_block=1 trigger the FT hardware delay.
*    After the  delay time 12ms, Fine-tune Function is turned on. And,the chip hardware resets FT_block=1 to FT_block=0 internally.
*    Then, FT function is continuously fine-tuning the gain by the voltage level of pin RFAGC
* 6. Fint-tune function take a reference to RFAGC voltage level of Hardware Pin5.
*	 When Fine-tune is enable, make sure the RFAGC do not have a sharp jump that cause a longer AGC settling time.
***********************************************************************************/
static TUNER_ErrorCode Tuner_Set_Fine_Tune (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT32 pre_delay;
	UINT32 post_delay;
	UINT8 reg37;

	switch ( pAVtuner->FT )
	{
		case  FT_ON:
		{
			pre_delay  = 1;
			post_delay = 5;
			break;
		}
		case  FT_Hold:
		{
			pre_delay  = 10;
			post_delay = 1;
			break;
		}
		case  FT_Delay_ON:
		{
			pre_delay  = 1;
			post_delay = 15;
			break;
		}
		default:
		{ //OFF
			pre_delay  = 1;
			post_delay = 1;
			break;
		}
	}

	reg37 = pAVtuner->FT;
	/* Time delay ms*/
	Time_DELAY_MS(pre_delay);
	/* Send Fine-tune Function Control*/
	result = Tuner_I2C_Write(pAVtuner,37, &reg37, 1);
	/* Time delay ms*/
	Time_DELAY_MS(post_delay);

	return result;
}

/*********************************************************************************
* Loopthrough and power-down setting
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. reg[12]_D7 is xocore_ena. Enable/disable XO section
* 2. reg[12]_D6 is RFLP_ena.   Enable/disable Loop-through section
* 3. reg[12]_D5 is PD_soft.	   Power ON/OFF Receiver,Synthesizer,VCO section
* 4. For tuner model name =1, hardware Power-down is controled by Hardware Pin13, which turn off Receiver,Synthesizer,VCO,XO,Control section.
*	 For tuner model name =1, make sure Pin13 is at low when sending registers.
**********************************************************************************/
static TUNER_ErrorCode Tuner_Set_RFLP_PD (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT8 reg12;

	reg12 = (0x16)|( pAVtuner->XO_EN <<7)|(pAVtuner->RFLP_EN <<6)|(pAVtuner->PD_EN <<5);

	result = Tuner_I2C_Write(pAVtuner,12, &reg12, 1);
	/* Time delay ms*/
	Time_DELAY_MS(5);
	return result;
}
#endif

/*********************************************************************************
* RX output baseband programmable gain amplifier setting
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. reg[8]_D6~D3 is gc. PGA gain setting
* 2. reg[8]_D1~0  is PGAout_cs. PGA output driving current setting
**********************************************************************************/
static TUNER_ErrorCode Tuner_Set_RXout_PGA (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT8 reg8;

	reg8 = (0x04)|(pAVtuner->PGA_Gain <<3)|(pAVtuner->PGA_Current);

	result = Tuner_I2C_Write(pAVtuner,8, &reg8, 1);

	return result;
}

#if 0
/*********************************************************************************
* Channel lock status
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. reg[11]_D0 is CHLF. Read-only bit. Lock=1. Unlock=0.
**********************************************************************************/
static TUNER_ErrorCode Tuner_Get_Channel_Lock (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT8 reg11;
	UINT8 lock;

	Tuner_I2C_Read(pAVtuner, 11, &reg11, 1);
	lock = reg11 &0x01;
	if ( lock != 1)
		result = PLL_Lock_Error;
	return result;
}

/*********************************************************************************
* Filter bandwidth lock status
* Paramter1:    pTunerPara : pointer of Tuner parameter structure
* Return value: TUNER_ErrorCode : error code of definition
* Description:
* 1. reg[11]_D1 is BWLF. Read-only bit. Lock=1. Unlock=0.
**********************************************************************************/
static TUNER_ErrorCode Tuner_Get_Filter_Lock (pTunerPara pAVtuner)
{
	TUNER_ErrorCode result = Tuner_No_Error;
	UINT8 reg11;
	UINT8 lock;

	Tuner_I2C_Read(pAVtuner, 11, &reg11, 1);
	lock = (reg11>>1) &0x01;
	if ( lock != 1)
		result = Filter_Lock_Error;
	return result;
}
#endif

MT_FE_RET mt_fe_tn_init_AV2018_ds6113(MT_FE_DS6113_Device_Handle handle)
{
	ds6113_handle = handle;

	//Tuner_Parameter_Initial(&AVtuner);
	//Tuner_Register_Initial(&AVtuner);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_set_freq_AV2018_ds6113(MT_FE_DS6113_Device_Handle handle, U32 freq_KHz, U32 sym_rate_KSs, S16 lpf_offset_KHz)
{
	ds6113_handle = handle;

	Tuner_Set_Channel_Frequency(&AVtuner, freq_KHz, sym_rate_KSs);
	//Tuner_Set_Filter_Bandwith(&AVtuner, sym_rate_KSs);

	//AVtuner.RFLP_EN = RFLP_ON;
	//Tuner_Set_RFLP_PD (&AVtuner);

	return MtFeErr_Ok;
}

MT_FE_RET mt_fe_tn_get_gain_AV2018_ds6113(MT_FE_DS6113_Device_Handle handle, U32 *p_gain)
{
	U8 reg_0x3f, reg_0x40;
	U32 tmp;

	ds6113_handle = handle;

	handle->dmd_get_reg(handle, 0x3f, &reg_0x3f);
	handle->dmd_get_reg(handle, 0x40, &reg_0x40);

	tmp = reg_0x3f * 32 + reg_0x40;

	*p_gain = tmp;

	return MtFeErr_Ok;
}


#define STRENGTH_RATIO 100

MT_FE_RET mt_fe_tn_get_strength_AV2018_ds6113(MT_FE_DS6113_Device_Handle handle, U32 *p_gain, S32 *p_strength)
{
	U32 gain_val;
	S32 strength;
	MT_FE_LOCK_STATE lock_state;

	ds6113_handle = handle;

	mt_fe_tn_get_gain_AV2018_ds6113(handle, &gain_val);

	if (gain_val <= 338)		strength = 99;										  //about -30dBm    99%
	else if (gain_val > 2080)	strength = 0;
	else 						strength = 99 - (gain_val - 338) * 10 / 156;


	mt_fe_dmd_ds6113_get_pure_lock(handle, &lock_state);
	if ((lock_state == MtFeLockState_Locked) && (strength < 40))
		strength = 20 + strength / 2;


	strength = strength * STRENGTH_RATIO / 100;


	if (strength >= 100) strength = 99;
	if (strength < 0) strength = 0;

	*p_gain = gain_val;
	*p_strength = strength;

	return MtFeErr_Ok;
}


S32 mt_fe_tn_get_tuner_freq_offset_AV2018_ds6113(MT_FE_DS6113_Device_Handle handle)
{
	ds6113_handle = handle;

	return 0;
}

