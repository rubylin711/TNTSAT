/********************************************************************************************/
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
#include "RDA5815.h"
#include "TP5001.h"
#ifdef _USE_TP5001_CHIP_

#define RDA5815WriteReg(addr, value){ \
	reg_data[0] = addr;\
	reg_data[1] = value;\
	ret = TP_iic_tuner_write(RDA5815_DEV_ADDR, reg_data, 2);\
	if(ret != TP_SUCCESS)\
		return ret;\
}

TP_UINT8 rda_5815_init()
{
	TP_UINT8 ret;
	TP_UINT8 reg_data[16];
// reset tuner
	TP_Delay(1);
	 // Chip register soft reset 	
	RDA5815WriteReg(0x04,0x04);
	RDA5815WriteReg(0x04,0x05); 

	// Initial configuration start

	//pll setting 
	RDA5815WriteReg(0x1a,0x13);   //add by rda 2011.5.28
	RDA5815WriteReg(0x41,0x53);  //add by rda 2011.12.27
	RDA5815WriteReg(0x38,0x93); //modify by rda 2012.04.17
	RDA5815WriteReg(0x39,0x15); 
	RDA5815WriteReg(0x3A,0x00);
	RDA5815WriteReg(0x3B,0x00);
	RDA5815WriteReg(0x3C,0x0c);  //add by rda 2011.8.11
	RDA5815WriteReg(0x0c,0xE2);
	RDA5815WriteReg(0x2e,0x6F);
	RDA5815WriteReg(0x72,0x07);
	RDA5815WriteReg(0x73,0x20);//modify by rda 2011.10.28
	RDA5815WriteReg(0x74,0x72);//modify by rda 2011.10.28
	RDA5815WriteReg(0x5b,0x20);
	RDA5815WriteReg(0x2f,0x57);
	RDA5815WriteReg(0x0d,0x70);
	RDA5815WriteReg(0x16,0x03);
	RDA5815WriteReg(0x18,0x4B);
	RDA5815WriteReg(0x30,0xFF);
	RDA5815WriteReg(0x5c,0xFF);
	RDA5815WriteReg(0x6c,0xFF);
	RDA5815WriteReg(0x6e,0xFF);
	RDA5815WriteReg(0x65,0x00);//modify by rda 2011.12.27
	RDA5815WriteReg(0x70,0x3F);
	RDA5815WriteReg(0x71,0x3F);
	RDA5815WriteReg(0x75,0x06);
	RDA5815WriteReg(0x76,0x40);
	RDA5815WriteReg(0x77,0x89);
	RDA5815WriteReg(0x53,0xA8);
	RDA5815WriteReg(0x46,0x21);
	RDA5815WriteReg(0x47,0x84);
	RDA5815WriteReg(0x48,0x10);
	RDA5815WriteReg(0x49,0x08);
	RDA5815WriteReg(0x60,0x80);
	RDA5815WriteReg(0x61,0x80);
	RDA5815WriteReg(0x6A,0x08);
	RDA5815WriteReg(0x6B,0x63);
	RDA5815WriteReg(0x69,0xF8);
	RDA5815WriteReg(0x57,0x64);//modify by rda 2011.12.27
	RDA5815WriteReg(0x05,0x88);
	RDA5815WriteReg(0x06,0xF8);//modify by rda 2012.8.10
	RDA5815WriteReg(0x15,0xAE);//modify by rda 2011.10.28
       RDA5815WriteReg(0x4a,0x68);//modify by rda 2012.7.18
       RDA5815WriteReg(0x4b,0x78);//modify by rda 2012.7.18
  

	   
	//agc setting

  RDA5815WriteReg(0x4f,0x40);
  RDA5815WriteReg(0x5b,0x20);
  
  // for blocker
  RDA5815WriteReg(0x16,0x10);//stage setting
  RDA5815WriteReg(0x18,0x20);            
  RDA5815WriteReg(0x30,0x30);            
  RDA5815WriteReg(0x5c,0x30);            
  RDA5815WriteReg(0x6c,0x30);            
  RDA5815WriteReg(0x6e,0x70);            
  RDA5815WriteReg(0x1b,0xB2);            
  RDA5815WriteReg(0x1d,0xB2);            
  RDA5815WriteReg(0x1f,0xB2);            
  RDA5815WriteReg(0x21,0xB2);            
  RDA5815WriteReg(0x23,0xB6);            
  RDA5815WriteReg(0x25,0xB6);            
  RDA5815WriteReg(0x27,0xBA);            
  RDA5815WriteReg(0x29,0xBF);            
  RDA5815WriteReg(0xb3,0xFF);            
  RDA5815WriteReg(0xb5,0xFF);            
                                         
  RDA5815WriteReg(0x17,0xF0);            
  RDA5815WriteReg(0x19,0xF0);            
  RDA5815WriteReg(0x31,0xF0);            
  RDA5815WriteReg(0x5d,0xF1);            
  RDA5815WriteReg(0x6d,0xF2);            
  RDA5815WriteReg(0x6f,0xF2);            
  RDA5815WriteReg(0x1c,0x31);            
  RDA5815WriteReg(0x1e,0x72);            
  RDA5815WriteReg(0x20,0x96);            
  RDA5815WriteReg(0x22,0xBA);            
  RDA5815WriteReg(0x24,0xBA);            
  RDA5815WriteReg(0x26,0xBE);            
  RDA5815WriteReg(0x28,0xCE);            
  RDA5815WriteReg(0x2a,0xDE);            
  RDA5815WriteReg(0xb4,0x0F);            
  RDA5815WriteReg(0xb6,0x0F);            
                                         
  RDA5815WriteReg(0xb7,0x10);	//start    
  RDA5815WriteReg(0xb9,0x10);	           
  RDA5815WriteReg(0xbb,0x00);	           
  RDA5815WriteReg(0xbd,0x00);	           
  RDA5815WriteReg(0xbf,0x00);	           
  RDA5815WriteReg(0xc1,0x10);	           
  RDA5815WriteReg(0xc3,0x10);	           
  RDA5815WriteReg(0xc5,0x10);	           
  RDA5815WriteReg(0xa3,0x19);	           
  RDA5815WriteReg(0xa5,0x2E);	           
  RDA5815WriteReg(0xa7,0x37);	           
  RDA5815WriteReg(0xa9,0x47);	           
  RDA5815WriteReg(0xab,0x47);            
  RDA5815WriteReg(0xad,0x3F);            
  RDA5815WriteReg(0xaf,0x00);            
  RDA5815WriteReg(0xb1,0x95); //modify by rda 2012.1.12              
                                         
                                         
  RDA5815WriteReg(0xb8,0x47); //end      
  RDA5815WriteReg(0xba,0x3F);            
  RDA5815WriteReg(0xbc,0x37);            
  RDA5815WriteReg(0xbe,0x3F);            
  RDA5815WriteReg(0xc0,0x3F);            
  RDA5815WriteReg(0xc2,0x3F);            
  RDA5815WriteReg(0xc4,0x3F);            
  RDA5815WriteReg(0xc6,0x3F);            
  RDA5815WriteReg(0xa4,0x47);            
  RDA5815WriteReg(0xa6,0x57);            
  RDA5815WriteReg(0xa8,0x5F);            
  RDA5815WriteReg(0xaa,0x70);            
  RDA5815WriteReg(0xac,0x70);            
  RDA5815WriteReg(0xae,0x68);            
  RDA5815WriteReg(0xb0,0x95); //modify by rda 2012.1.12              
  RDA5815WriteReg(0xb2,0x95); //modify by rda 2012.1.12              
                                         
                                         
  RDA5815WriteReg(0x81,0x77); //rise     
  RDA5815WriteReg(0x82,0x68);            
  RDA5815WriteReg(0x83,0x70);            
  RDA5815WriteReg(0x84,0x68);            
  RDA5815WriteReg(0x85,0x68);            
  RDA5815WriteReg(0x86,0x68);            
  RDA5815WriteReg(0x87,0x70);            
  RDA5815WriteReg(0x88,0x47);            
  RDA5815WriteReg(0x89,0x68);            
  RDA5815WriteReg(0x8a,0x8E);            
  RDA5815WriteReg(0x8b,0x8E);            
  RDA5815WriteReg(0x8c,0x8E);            
  RDA5815WriteReg(0x8d,0x9C);            
  RDA5815WriteReg(0x8e,0xe0); //modify by rda 2012.1.12           
  RDA5815WriteReg(0x8f,0x95); //modify by rda 2012.1.12               
                                         
  RDA5815WriteReg(0x90,0x00); //fall     
  RDA5815WriteReg(0x91,0x00);            
  RDA5815WriteReg(0x92,0x00);            
  RDA5815WriteReg(0x93,0x00);            
  RDA5815WriteReg(0x94,0x00);            
  RDA5815WriteReg(0x95,0x00);            
  RDA5815WriteReg(0x96,0x00);            
  RDA5815WriteReg(0x97,0x00);            
  RDA5815WriteReg(0x98,0x00);            
  RDA5815WriteReg(0x99,0x00);            
  RDA5815WriteReg(0x9a,0x10);            
  RDA5815WriteReg(0x9b,0x24);            
  RDA5815WriteReg(0x9c,0x10);            
  RDA5815WriteReg(0x9d,0x00);            
  RDA5815WriteReg(0x9e,0x00);            
  

	TP_Delay(10);

	return TP_SUCCESS;
                     
// Initial configuration end
}

	
	/*************************************************************************/
//	Function to Set the RDA5815                       
//	fPLL:   Frequency        			unit: MHz  from 250 to 2300 
//	fSym:   SymbolRate       			unit: KS/s from 1000 to 45000 
/************************************************************************/
                         
//INT32 RDA5815Set(UINT32 fPLL, UINT32 fSym )
TP_UINT8 rda_5815_set_frequency(TP_UINT32 frequency, TP_UINT32 Symbol_Rate_Value)
{			                   
	TP_UINT8 ret;
	TP_UINT8 reg_data[16];

	TP_UINT8 buffer; 
 	TP_UINT32 temp_value = 0;
	TP_UINT32 bw;/*,temp_value1 = 0,temp_value2=0 ;*/
	TP_UINT8 Filter_bw_control_bit;	
	
  
		
	RDA5815WriteReg(0x04,0xc1); //add by rda 2011.8.9,RXON = 0 , change normal working state to idle state
    RDA5815WriteReg(0x2b,0x95);//clk_interface_27m=0  add by rda 2012.1.12     
	//set frequency start
	temp_value = (TP_UINT32)frequency* 77672;//((2<<21) / RDA5815_XTALFREQ);
  
	buffer = ((TP_UINT8)((temp_value>>24)&0xff));
	RDA5815WriteReg(0x07,buffer);
	buffer = ((TP_UINT8)((temp_value>>16)&0xff));	
	RDA5815WriteReg(0x08,buffer);	
   	buffer = ((TP_UINT8)((temp_value>>8)&0xff));
	RDA5815WriteReg(0x09,buffer);	
   	buffer = ((TP_UINT8)( temp_value&0xff));
	RDA5815WriteReg(0x0a,buffer);
	//set frequency end
	
	// set Filter bandwidth start
	bw=Symbol_Rate_Value;
	
	if(bw<4000)
	bw= 4000;    // KHz
	else if(bw>45000)
	bw = 40000;   // KHz    //modify by rda 2012.1.12   
	
	Filter_bw_control_bit = (TP_UINT8)((bw*135/200+4000)/1000);
	
	Filter_bw_control_bit&=0x3f;
	RDA5815WriteReg(0x0b,Filter_bw_control_bit);
	// set Filter bandwidth end
	
	RDA5815WriteReg(0x04,0xc3); //add by rda 2011.8.9,RXON = 0 ,rxon=1,normal working
       RDA5815WriteReg(0x2b,0x97);//clk_interface_27m=1  add by rda 2012.1.12  
	
  TP_Delay(5);
	return TP_SUCCESS;

}
#endif //_USE_TP5001_CHIP_
