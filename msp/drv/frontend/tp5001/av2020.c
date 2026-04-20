/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#include "av2020.h"
#include "TP5001.h"

#ifdef _USE_TP5001_CHIP_
static TP_UINT16 tuner_crystal = 27; //unit is MHz
TP_UINT8 AV2020_DEV_ADDR = 0x63;
// ---------------------------------------------------------------------------------------------------------
unsigned int Tuner_control (unsigned int channel_freq, unsigned int bb_sym)
{
	unsigned char reg[50];
    unsigned char ret;   
	unsigned int fracN;
	unsigned int BW;
	unsigned int BF;
	
// Auto-scan mode flag. Default is not at auto-scan mode
unsigned char auto_scan = 0; 

// Register initial flag. Static constant for first entry
//static unsigned char tuner_initial = 0;	



    // Channel Frequency Calculation.
	fracN = (channel_freq + tuner_crystal/2)/tuner_crystal;
	if(fracN > 0xff)
  	   fracN = 0xff;
    reg[1]=(char) (fracN & 0xff);
  	fracN = (channel_freq<<17)/tuner_crystal;
  	fracN = fracN & 0x1ffff;
  	reg[2]=(char) ((fracN>>9)&0xff);
  	reg[3]=(char) ((fracN>>1)&0xff);
  	// reg[3]_D7 is frac<0>, D6~D0 is 0x50
  	reg[4]=(char) (((fracN<<7)&0x80) | 0x50);

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
    reg[6] = (unsigned char)BF;

    // Auto-scan mode setting is depended on the algorithm of Base-Band.
    // Here shows a example.
    // When bb_sym is 0 or 45000, means auto-scan channel.
    // Base-band set a fixed BW=27MHz at auto-scan mode.
    if (bb_sym == 0 || bb_sym == 45000)
    {
		auto_scan = 1;
		reg[6] = 0xA3; //BW=27MHz
	}

	if(auto_scan)
	{
  	    // Sequence 4
  	    // Send Reg0 ->Reg4
		reg[0] = 0;
		reg[5] = 0x1f;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 6); 
		if(ret != TP_SUCCESS)
			return ret;


  	    // Time delay 4ms
  	   TP_Delay(4);

  	    // Sequence 5
  	    // Send Reg5
	    reg[0] = 5;
		reg[1] = reg[6] ;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);  
		if(ret != TP_SUCCESS)
			return ret;


  	    // Fine-tune Function Control
  	    //Auto-scan mode. FT_block=1, FT_EN=0, FT_hold=1
	    reg[0] = 37;
		reg[1] = 0x05;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);  
		if(ret != TP_SUCCESS)
			return ret;

		// Time delay 4ms
		TP_Delay(4);
	}
	else
	{
		// Sequence 4
  	    // Send Reg0 ->Reg4

		reg[0] = 0;
		reg[5] = 0x1f;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 6);   //??
		if(ret != TP_SUCCESS)
			return ret;
		// Time delay 4ms
		TP_Delay(4);

		// Sequence 5
		// Send Reg5
		reg[0] = 5;
		reg[1] = reg[6] ;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);  
		if(ret != TP_SUCCESS)
			return ret;
	

  	    // Fine-tune Function Control
  	    // Non-auto-scan mode. FT_block=1, FT_EN=1, FT_hold=0
		//reg[37] = 0x06;
		reg[0] = 37;
		reg[1] = 0x06;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);  
		if(ret != TP_SUCCESS)
			return ret;

		// Fine-tune function is starting tracking after sending reg[37].
		// Make sure the RFAGC do not have a sharp jump.
		
		reg[12] = 0x96; //Disable RFLP at Lock Channel sequence after reg[37]		
		// reg[12] = 0xd6; //Enable RFLP at Lock Channel sequence after reg[37]  
		 reg[0] = 12;
		reg[1] = 0x96;
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);  
		if(ret != TP_SUCCESS)
			return ret;


		// Time delay 4ms
		TP_Delay(4);
	}

    return TP_SUCCESS;
}

TP_UINT8 av2020_init()
{
	unsigned char reg[50];
    unsigned char ret;

   //Initail registers R0~R41			
		// Sequence 1
		// Send Reg0 ->Reg11
		reg[0] = 0x00;
		reg[1]=(unsigned  char) (0x50);
		reg[2]=(unsigned  char) (0xa1);
		reg[3]=(unsigned char) (0x2f);
		reg[4]=(unsigned char) (0x50);
		reg[5]=(unsigned char) (0x1f);
		reg[6]=(unsigned char) (0xa3);
		reg[7]=(unsigned char) (0xfd);
		reg[8]=(unsigned char) (0x58);
		reg[9]=(unsigned char) (0x46); //0x0e
		reg[10]=(unsigned char) (0x82);
		reg[11]=(unsigned char) (0x88);
		reg[12]=(unsigned char) (0xb4);
		reg[13]=(unsigned char) (0xd6);	 //RFLP=ON at Power on initial

		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 13);   
		if(ret != TP_SUCCESS)
			return ret;
		
		// Sequence 2
		// Send Reg13 ->Reg24	
		reg[0] = 13;
		reg[1]=(unsigned char) (0x40);
		reg[2]=(unsigned char) (0x5b);
		reg[3]=(unsigned char) (0x6a);
		reg[4]=(unsigned char) (0x66);
		reg[5]=(unsigned char) (0x40);
		reg[6]=(unsigned char) (0x80);
		reg[7]=(unsigned char) (0x2b);
		reg[8]=(unsigned char) (0x6a);
		reg[9]=(unsigned char) (0x50);
		reg[10]=(unsigned char) (0x91);
		reg[11]=(unsigned char) (0x27);
		reg[12]=(unsigned char) (0x8f);

		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 13);   
		if(ret != TP_SUCCESS)
			return ret;

  	    // Send Reg25 ->Reg35	
		reg[0] = 25;
		reg[1]=(unsigned char) (0xcc);
		reg[2]=(unsigned char) (0x21);
		reg[3]=(unsigned char) (0x10);
		reg[4]=(unsigned char) (0x80);
		reg[5]=(unsigned char) (0x02);
		reg[6]=(unsigned char) (0xf5);
		reg[7]=(unsigned char) (0x7f);
		reg[8]=(unsigned char) (0x4a);
		reg[9]=(unsigned char) (0x9b);
		reg[10]=(unsigned char) (0xe0);
		reg[11]=(unsigned char) (0xe0);

		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 12);   
		if(ret != TP_SUCCESS)
			return ret;

		// Send Reg36 ->Reg41	
		reg[0] = 36;
		reg[1]=(unsigned char) (0x36);
		reg[2]=(unsigned char) (0x00);	// Disble FT function at Power on initial 
		reg[3]=(unsigned char) (0xab);
		reg[4]=(unsigned char) (0x97);
		reg[5]=(unsigned char) (0xc5);
		reg[6]=(unsigned char) (0xa8);

		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 7);   
		if(ret != TP_SUCCESS)
			return ret;
		
		TP_Delay(100);		

	
        // Sequence 3
		// Send reg12
		reg[0] = 12;
		reg[1]=(unsigned char) (0xd6);	 //RFLP=ON at Power on initial
		ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);   
		if(ret != TP_SUCCESS)
			return ret;
	
		
		TP_Delay(100);
		//Reinitial again
		{
		// Sequence 1
			// Send Reg0 ->Reg11

			reg[0] = 0x00;
			reg[1]=(unsigned char) (0x50);
			reg[2]=(unsigned char) (0xa1);
			reg[3]=(unsigned char) (0x2f);
			reg[4]=(unsigned char) (0x50);
			reg[5]=(unsigned char) (0x1f);
			reg[6]=(unsigned char) (0xa3);
			reg[7]=(unsigned char) (0xfd);
			reg[8]=(unsigned char) (0x58);
			reg[9]=(unsigned char) (0x46);//0e
			reg[10]=(unsigned char) (0x82);
			reg[11]=(unsigned char) (0x88);
			reg[12]=(unsigned char) (0xb4);
			reg[13]=(unsigned char) (0xd6);	 //RFLP=ON at Power on initial

			ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 13);   
			if(ret != TP_SUCCESS)
				return ret;
			
			// Sequence 2
			// Send Reg13 ->Reg24	
			reg[0] = 13;
			reg[1]=(unsigned char) (0x40);
			reg[2]=(unsigned char) (0x5b);
			reg[3]=(unsigned char) (0x6a);
			reg[4]=(unsigned char) (0x66);
			reg[5]=(unsigned char) (0x40);
			reg[6]=(unsigned char) (0x80);
			reg[7]=(unsigned char) (0x2b);
			reg[8]=(unsigned char) (0x6a);
			reg[9]=(unsigned char) (0x50);
			reg[10]=(unsigned char) (0x91);
			reg[11]=(unsigned char) (0x27);
			reg[12]=(unsigned char) (0x8f);

			ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 13);   
			if(ret != TP_SUCCESS)
				return ret;

  			// Send Reg25 ->Reg35	
			reg[0] = 25;
			reg[1]=(unsigned char) (0xcc);
			reg[2]=(unsigned char) (0x21);
			reg[3]=(unsigned char) (0x10);
			reg[4]=(unsigned char) (0x80);
			reg[5]=(unsigned char) (0x02);
			reg[6]=(unsigned char) (0xf5);
			reg[7]=(unsigned char) (0x7f);
			reg[8]=(unsigned char) (0x4a);
			reg[9]=(unsigned char) (0x9b);
			reg[10]=(unsigned char) (0xe0);
			reg[11]=(unsigned char) (0xe0);

			ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 12);   
			if(ret != TP_SUCCESS)
				return ret;

			// Send Reg36 ->Reg41	
			reg[0] = 36;
			reg[1]=(unsigned char) (0x36);
			reg[2]=(unsigned char) (0x00);	// Disble FT function at Power on initial 
			reg[3]=(unsigned char) (0xab);
			reg[4]=(unsigned char) (0x97);
			reg[5]=(unsigned char) (0xc5);
			reg[6]=(unsigned char) (0xa8);

			ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 7);   
			if(ret != TP_SUCCESS)
				return ret;

		
			// Sequence 3
			// Send reg12
			reg[0] = 12;
			reg[1]=(unsigned char) (0xd6);	 //RFLP=ON at Power on initial
			ret = TP_iic_tuner_write(AV2020_DEV_ADDR, reg, 2);   
			if(ret != TP_SUCCESS)
				return ret;
		
		 }	
	
 		// Time delay 4ms
		TP_Delay(4);

		return TP_SUCCESS;
}
#endif // _USE_TP5001_CHIP_
