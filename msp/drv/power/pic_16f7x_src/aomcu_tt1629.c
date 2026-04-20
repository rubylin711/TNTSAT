/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include    "pic16f7x.h"
#include    "ledkb_regs.h"
#include    "irda_regs.h"
#include    "mcu_regs.h"
#include    "datatype.h"



#define VERSION            100

#define wr_port(port_addr,data) do { \
                                    EXPORT_ADDRESS_L = port_addr;    		\
                                    EXPORT_ADDRESS_H = (port_addr>>0x08);	\
                                    EXPORT_WDATA = data;					\
                                } while(0)

#define rd_port(port_addr,data) do { \
                                    EXPORT_ADDRESS_L = port_addr;   		\
                                    EXPORT_ADDRESS_H = (port_addr>>0x08);	\
                                    GIE = 0;                                                 \
                                    EXPORT_RDATA_EN = 1;                            \
                                    data=EXPORT_RDATA;				       \
                                    GIE = 1;                                                 \
                                } while(0)


#define  LPM_GLB_CTRL 0x001C


U16 tick = 0;
U32 ctick = 0;
U8 CURRENT_HOUR,CURRENT_MINUTE;
U8 WAKE_UP_HOUR,WAKE_UP_MINUTE;
U8 WAKE_UP_KEY,AUTO_WAKE_UP;
U8 STANDBY_CONFIG;
U8 LED_DISPLAY_TIME;
U8 LED_DISPLAY_CHAR;
U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
U8 LED_POS = 0;
U8 STANDBY_CONFIG2 = 0;
U8 GPEN_VAL = 0;
U8 WAKE_UP_DAY = 0;
U8 LED_DISPLAY_BRIG = 0;

U8 IR_INT_FLAG  = 0;
U8 WAKEUP_FPAG = 0;
U8 STANDBY_PARAM = 0;//0 means symphony chip;1 means syphony2/symphony3
U8 bright_idx = 0;
U8 CEC_ENABLE = 0;

U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;

void aomcu_delay(int times)
{
	int i =0;
	int j = 0;
	for(j=0;j<times;j++)
	{
		for(i=0;i<times;i++)
		asm ("nop");
	}
}

 void x_zero_delay(void)
 {
	aomcu_delay(1);
 }

void msdelay(U32 ms)
{
		ctick = 0;
		while(((ctick * 24) /10) < ms)
                ;
}


/*!
 * RTC enable register
 */
#define RTC_EN            0x2000
/*!
 * RTC param register
 */
#define RTC_PARAMETER_H   0x2004
/*!
 * RTC param register
 */
#define RTC_PARAMETER_L   0x2008
/*!
 * RTC status register
 */
#define RTC_STA           0x200C


#define WAKEUP_EN   	0x0054

#define LPM_POW_CTRL	0x0004

#define IR_INT_RAWSTA          0x1014
#define IR_INT_CFG                 0x1010

#define LEVEL1_BRIGHT           (0x88) // 1 level brightness
#define LEVEL2_BRIGHT           (0x89) // 2 level brightness
#define LEVEL3_BRIGHT           (0x8A) // 3 level brightness
#define LEVEL4_BRIGHT           (0x8B) // 4 level brightness
#define LEVEL5_BRIGHT           (0x8C) // 5 level brightness
#define LEVEL6_BRIGHT           (0x8D) // 6 level brightness
#define LEVEL7_BRIGHT           (0x8E) // 7 level brightness
#define LEVEL8_BRIGHT           (0x8F) // 8 level brightness

#define AUTO_ADDRESS_MODE   		(0x40)
#define READ_DATA_MODE      		(0x42)
#define FIXED_ADDRESS_MODE  		(0x44)

#define START_ADDRESS       		(0xc0)

#define GRID1_FIXED_ADDR    		(START_ADDRESS)
#define GRID2_FIXED_ADDR    		(GRID1_FIXED_ADDR+0x02)
#define GRID3_FIXED_ADDR    		(GRID2_FIXED_ADDR+0x02)
#define GRID4_FIXED_ADDR    		(GRID3_FIXED_ADDR+0x02)
#define GRID5_FIXED_ADDR    		(GRID4_FIXED_ADDR+0x02)
#define GRID6_FIXED_ADDR    		(GRID5_FIXED_ADDR+0x02)
#define GRID7_FIXED_ADDR    		(GRID6_FIXED_ADDR+0x02)
#define GRID8_FIXED_ADDR    		(GRID7_FIXED_ADDR+0x02)

//depends on customer HW design
#define FIRST_GRID_ADDR          	 (GRID6_FIXED_ADDR)
#define SECOND_GRID_ADDR          (GRID7_FIXED_ADDR)
#define THIRD_GRID_ADDR          	 (GRID5_FIXED_ADDR)
#define FOURTH_GRID_ADDR          (GRID4_FIXED_ADDR)
#define COLON_ADDR          		 (GRID7_FIXED_ADDR)
#define SPOT_ADDR 				 (GRID5_FIXED_ADDR)
#define GREEN_LED_ADDR          	 (GRID3_FIXED_ADDR)
#define RED_LED_ADDR          		 (GRID2_FIXED_ADDR)

#define SEG1  						(0x01)  		//BIT0
#define SEG2  						(0x02)  		//BIT1
#define SEG3  						(0x04)  		//BIT2
#define SEG4  						(0x08)  		//BIT3
#define SEG5  						(0x10)  		//BIT4
#define SEG6  						(0x20)  		//BIT5
#define SEG7  						(0x40)  		//BIT6
#define SEG8  						(0x80)  		//BIT7
#define SEG9  						(0x100) 		//BIT8
#define SEG10  						(0x200)    	//BIT9
#define SEG11  						(0x400)    	//BIT10
#define SEG12  						(0x800)    	//BIT11
#define SEG13  						(0x1000)    	//BIT12
#define SEG14  						(0x2000)    	//BIT13
#define SEG15  						(0x4000)    	//BIT14
#define SEG16  						(0x8000)    	//BIT15


#define BIT_A	   				   	(SEG16)  
#define BIT_B 						(SEG9)  
#define BIT_C						(SEG10)      
#define BIT_D						(SEG7)
#define BIT_E 						(SEG1)
#define BIT_F 						(SEG11)
#define BIT_G1 						(SEG15)
#define BIT_G2						(SEG6)
#define BIT_H 						(SEG12)
#define BIT_J 						(SEG13)  
#define BIT_K						(SEG14)
#define BIT_L						(SEG5)
#define BIT_M						(SEG4)
#define BIT_N						(SEG2)
#define BIT_DP						(SEG8)
#define BIT_D1_D2					(SEG3)

#define GRID_CNT					(8)
#define GRID_DATA_CNT				(2*GRID_CNT)
#define FP_MAX_LED_NUM  			(4)

#define TM1629B_STB          		5//(AO_GPIO_5)
#define TM1629B_CLK          		6//(AO_GPIO_6)
#define TM1629B_DAT          		7//(AO_GPIO_7)



/*!
 LEB bitmap
 */
typedef struct
{
 /*!
   ascii character to display
   */
 U8 ch;
 /*!
   bitmap
   */
 U16 bitmap;
}aomcu_led_bitmap_t;

#define FP_TM1629B_TABLE_SIZE 11

static aomcu_led_bitmap_t mcu_tm1629b_bitmap[FP_TM1629B_TABLE_SIZE] =
{
  {'0', BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},  
  {'1', BIT_B+BIT_C}, 
  {'2', BIT_A+BIT_B+BIT_E+BIT_D+BIT_G1+BIT_G2},
  {'3', BIT_A+BIT_B+BIT_C+BIT_D+BIT_G1+BIT_G2},
  {'4', BIT_B+BIT_C+BIT_F+BIT_G1+BIT_G2},
  {'5', BIT_A+BIT_C+BIT_D+BIT_F+BIT_G1+BIT_G2}, 
  {'6', BIT_A+BIT_C+BIT_D+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'7', BIT_A+BIT_B+BIT_C},
  {'8', BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'9', BIT_A+BIT_B+BIT_C+BIT_D+BIT_F+BIT_G1+BIT_G2}, 
};

#define FP_MAX_LED_NUM  4

U16 dis_buff[FP_MAX_LED_NUM];

U8 brigth_level[8] = {LEVEL1_BRIGHT,LEVEL2_BRIGHT,LEVEL3_BRIGHT,LEVEL4_BRIGHT,
  LEVEL5_BRIGHT,LEVEL6_BRIGHT,LEVEL7_BRIGHT,LEVEL8_BRIGHT};

void aomcu_aogpio_set_value(U8 aogpio, U8 val)
{
	U8 temp = 0;
	
	rd_port(AO_GPIO_WDATA, temp);

	if(val)
	{
		temp |= (1 << aogpio);
	}
	else
	{
		temp &= ~(1 << aogpio);
	}

	wr_port(AO_GPIO_WDATA, temp);
}

U8 aomcu_gpio_get_value(U8 aogpio)
{
	U8 temp = 0;
	
	rd_port(AO_GPIO_RDATA, temp);
	temp = (temp >> aogpio) & 0x01;
	
	return temp;
}

void aomcu_aogpio_set_dir(U8 aogpio, U8 dir)
{
	U8 val = 0;
	
	rd_port(AO_GPIO_WR_EN, val);

	if(dir)
	{
		val |= (1 << aogpio);
	}
	else
	{
		val &= ~(1 << aogpio);
	}

	wr_port(AO_GPIO_WR_EN, val);
}


void SET_STB(void)
{
	aomcu_aogpio_set_value(TM1629B_STB, 1);
}

static void SET_CLK(void)
{
	aomcu_aogpio_set_value(TM1629B_CLK, 1);
}

static void SET_DIO(void)
{
	aomcu_aogpio_set_value(TM1629B_DAT, 1);
}

static U8 IS_DIO_HIGH(void)
{
	U8 val = 0;
	val = aomcu_gpio_get_value(TM1629B_DAT);
	
	return (val == 1) ? 1 : 0;
}


static void CLR_STB(void)
{
	aomcu_aogpio_set_value(TM1629B_STB, 0);
}

static void CLR_CLK(void)
{
	aomcu_aogpio_set_value(TM1629B_CLK, 0);
}

static void CLR_DIO(void)
{
	aomcu_aogpio_set_value(TM1629B_DAT, 0);
}

void set_dio_to_input(void)
{
	aomcu_aogpio_set_dir(TM1629B_DAT, 1);
}

void set_dio_to_output(void)
{
	aomcu_aogpio_set_dir(TM1629B_DAT, 0);
}

static void aomcu_write_data(U8 data)
{
   U8 i;
   for(i=0;i<8;i++)
   {
	if((data >> i)&0x01){
     	 SET_DIO();

 	   }
	   else{
		 CLR_DIO();
 	   }
	   asm ("nop");
	   CLR_CLK();
	   asm ("nop");
	   SET_CLK();
	   asm ("nop");
  }
	asm ("nop");
}

static void write_cmd(U8 cmd)
{
	SET_STB();
	CLR_STB();
	aomcu_write_data(cmd);
}

void display_fixed_addr(U16 wdata, U8 addr)
{
	SET_DIO();
	SET_CLK();
	SET_STB();//all initinal 1 before transfer
	write_cmd(FIXED_ADDRESS_MODE);//fixed address mode
	SET_STB();
	write_cmd(addr);
	aomcu_write_data(wdata&0xff);
	//SET_DIO();
	//SET_CLK();
	SET_STB();//all initinal 1 before transfer
	write_cmd(addr+1);
	aomcu_write_data(wdata>>8);
}

static U16 get_tm1629b_bitmap(U8 data)
{
	return mcu_tm1629b_bitmap[data].bitmap;
}
void wake_up_tt1629b();

static U8 mcu_tt1629b_read() //read the key value
{
  U8 i,j, key_data = 0;
  U8 key_val[6] = {0};
  
  write_cmd(READ_DATA_MODE);
  set_dio_to_input();
  //SET_DIO();
  
  for(j = 0;j < 4;j++){
    for(i = 0;i < 8;i++){
      key_val[j]>>=1;
	//   asm ("nop");
      CLR_CLK();
  	//   asm ("nop");
  	//   asm ("nop");
      SET_CLK();
	 //  asm ("nop");
	 //  asm ("nop");
      if(IS_DIO_HIGH())
      {
      	//key_val[j] |= 0x80;
      	//wake_up_tt1629b();
      	key_val[j] = 0;
      	key_val[j] |= 0x1<<i;
		if (key_val[1] != 0x00 && key_val[1] != 0xff)
		{
		    key_val[1] += 0x01;//fake key value for diff different key,because byte1 not the same
		}

		for(i = 0;i < 4;i++)
		{
		    if(key_val[i] != 0 && key_val[i] != 0xff)
		    {
		      key_data = key_val[i];
		    }
		}
		break;
      }
    }
  }

  SET_STB();
  //asm ("nop");
 // asm ("nop");
  set_dio_to_output();
  SET_DIO();

  return key_data;
}


void parse_data ()
{
	U8 i;
	U8	AP_DATA[18] = {0};
	
	for (i=0; i<16; i++)
	{
		rd_port(LPM_MESSAGE2AO_0 + i, AP_DATA[i]);
	}

	rd_port(LPM_MESSAGE2AO_4, AP_DATA[16]);
	rd_port(LPM_MESSAGE2AO_4+1, AP_DATA[17]);
	
	CURRENT_HOUR	= AP_DATA[4];		
	CURRENT_MINUTE	= AP_DATA[5];	
	CURRENT_SECOND	= AP_DATA[6];		
	WAKE_UP_HOUR	= AP_DATA[7];		
	WAKE_UP_MINUTE	= AP_DATA[8];
	WAKE_UP_SECOND	= AP_DATA[9];
	WAKE_UP_DAY 	= AP_DATA[10];
	WAKE_UP_KEY 	= AP_DATA[11];	
	STANDBY_CONFIG	= AP_DATA[12];
	LED_POS 		= AP_DATA[13];
	STANDBY_CONFIG2 = AP_DATA[14];
	CEC_ENABLE = AP_DATA[15] & 0x01;
	GPEN_VAL			= STANDBY_CONFIG & 0x1;
	LED_DISPLAY_BRIG	= (STANDBY_CONFIG>>1)&0x07;
	LED_DISPLAY_CHAR	= (STANDBY_CONFIG>>5)&0x1;
	LED_DISPLAY_TIME	= (STANDBY_CONFIG>>6)&0x1;
	AUTO_WAKE_UP		= (STANDBY_CONFIG>>7)&0x1;

	CEC_ENABLE	 = AP_DATA[15] & 0x01;
	
	unix_time_flag = (AP_DATA[16] >> 0x5 ) & 0x1;
#if 0	
	for (i=0; i<4; i++)
	{
		LED_MAP[i<<1] = AP_DATA[i] & 0xf;
		LED_MAP[(i<<1)+1] = (AP_DATA[i] >> 4) & 0xf;
	}
#endif
	if(unix_time_flag)//for unix time
	{
		rd_port(LPM_MESSAGE2AO_5 + 0, unix_time_sec);
		rd_port(LPM_MESSAGE2AO_5 + 1, unix_time_sec1);
		rd_port(LPM_MESSAGE2AO_5 + 2, unix_time_sec2);
		rd_port(LPM_MESSAGE2AO_5 + 3, unix_time_sec3);
	}
	
	if (CURRENT_HOUR >= 24)
	{
		CURRENT_HOUR = 0;
	}
	
	if (CURRENT_MINUTE >= 60)
	{
		CURRENT_MINUTE = 0;
	}

	if (CURRENT_SECOND >= 60)
	{
		CURRENT_SECOND = 0;
	}
	
	if (WAKE_UP_HOUR >= 24)
	{
		WAKE_UP_HOUR = 0;
	}
	
	if (WAKE_UP_MINUTE >= 60)
	{
		WAKE_UP_MINUTE = 0;
	}
	
	if (WAKE_UP_SECOND >= 60)
	{
		WAKE_UP_SECOND = 0;
	}
	
	WAKE_UP_SECOND += CURRENT_SECOND;
	if (WAKE_UP_SECOND >= 60)
	{
		WAKE_UP_SECOND -= 60;
		WAKE_UP_MINUTE++;
	}
	
	WAKE_UP_MINUTE += CURRENT_MINUTE;
	if (WAKE_UP_MINUTE >= 60)
	{
		WAKE_UP_MINUTE -= 60;
		WAKE_UP_HOUR ++;
	}
	
	WAKE_UP_HOUR += CURRENT_HOUR;
	if (WAKE_UP_HOUR >= 24)
	{
		WAKE_UP_HOUR -= 24;
		WAKE_UP_DAY++;
	}
}




void standby()
{
	U8 temp = 0;

	/*
	*	set analog down
	*/
		wr_port(0x7000,0xff);
		wr_port(0x7001,0xff);
		wr_port(0x7002,0xff);
		wr_port(0x7003,0xff);


		rd_port(0x7006,temp);
		temp  &=  0xf0;
		temp |=  0x04;
		wr_port(0x7006,temp);
		wr_port(0x7007,0xff);


       /*!
  	*  For testing GPEN lpm case
   	*
   	* enable GPIO to control global power
   	*/
	rd_port(LPM_GLB_CTRL,temp);
	temp |= (0x1 << 1);

	if(GPEN_VAL)
		temp |= (0x1 << 0);
	else
		temp &= ~(0x1 << 0);

	wr_port(LPM_GLB_CTRL,temp);


	if(STANDBY_PARAM == 1)
	{
		//set sys off, mcpu enter sleep standby mode
		wr_port(0x0004, 0x0);
	}
}

void cec_send_cmd_to_wake_tv();
void wake_up_tt1629b()
{
	U8 tmp = 0;
	U8 tmp1 = 0;
	U8 tmp2 = 0;
	U8 tmp3 = 0;

	tmp |= WAKEUP_FPAG&0x7;
	wr_port(LPM_AOMCU_WAKEUP_FLAG, tmp);
	
	tmp = 0;
	if(unix_time_flag)
	{
		wr_port(LPM_AOMCU_MESSAGE,unix_time_sec);
		wr_port(LPM_AOMCU_MESSAGE +1 ,unix_time_sec1);
		wr_port(LPM_AOMCU_MESSAGE +2 , unix_time_sec2 );
		wr_port(LPM_AOMCU_MESSAGE +3 ,unix_time_sec3 );
	}
	else
	{
		tmp |= (CURRENT_HOUR & 0xf) << 4;
		tmp1 = (CURRENT_HOUR >> 4) & 0x3;
		tmp1 |= (CURRENT_MINUTE & 0x3f) << 2;
		tmp2 = CURRENT_SECOND & 0x3f;
		tmp3 = PASS_DAY;
		
		wr_port(LPM_AOMCU_MESSAGE, tmp );
		wr_port(LPM_AOMCU_MESSAGE +1 ,tmp1 );
		wr_port(LPM_AOMCU_MESSAGE +2 , tmp2);
		wr_port(LPM_AOMCU_MESSAGE +3 , tmp3);
	}

	cec_send_cmd_to_wake_tv();
	
	rd_port(0x0004,tmp);
	tmp &= ~(0x1 << 0x0);
	wr_port(0x0004,tmp); 


     /*!
       * out of low power
       * For testing GPEN lpm case
       *
       *core 1.05V power up by standby pin output high.
       */
	rd_port(LPM_GLB_CTRL,tmp);
	tmp |= (0x1 << 0);
	wr_port(LPM_GLB_CTRL,tmp);

       msdelay(60);//60ms

	TMR0 =0;
	T0IF = 0; //clear interupt
	INTCON = 0x80;

	wr_port(0x0054, 0x1); //wake up

}


void clock_advance_tt1629b ()
{
    static U8 show_colon = 1;
    U16 colon_map = BIT_D1_D2;

    if(LED_DISPLAY_TIME)
    {
      display_fixed_addr(get_tm1629b_bitmap(CURRENT_HOUR/10), FIRST_GRID_ADDR);
      
      if(show_colon)
      {
      display_fixed_addr((get_tm1629b_bitmap(CURRENT_HOUR%10)+BIT_D1_D2), SECOND_GRID_ADDR);
      //  display_fixed_addr(BIT_D1_D2, COLON_ADDR);
        show_colon = 0;
      }
      else
      {
       display_fixed_addr(get_tm1629b_bitmap(CURRENT_HOUR%10), SECOND_GRID_ADDR);
     //   display_fixed_addr(0x00, COLON_ADDR);
        show_colon = 1;
      }
      display_fixed_addr(get_tm1629b_bitmap(CURRENT_MINUTE/10), THIRD_GRID_ADDR);
      display_fixed_addr(get_tm1629b_bitmap(CURRENT_MINUTE%10), FOURTH_GRID_ADDR);
      SET_STB();
      write_cmd(bright_idx);
      SET_STB();
    }
	
    if ((CURRENT_HOUR == WAKE_UP_HOUR) && (CURRENT_MINUTE == WAKE_UP_MINUTE) && (WAKE_UP_DAY == PASS_DAY))
    {
        if (AUTO_WAKE_UP)
        {
          display_fixed_addr(get_tm1629b_bitmap(CURRENT_HOUR/10), FIRST_GRID_ADDR);
          display_fixed_addr((get_tm1629b_bitmap(CURRENT_HOUR%10)+BIT_D1_D2), SECOND_GRID_ADDR);
        //  display_fixed_addr(BIT_D1_D2, COLON_ADDR);
          display_fixed_addr(get_tm1629b_bitmap(CURRENT_MINUTE/10), THIRD_GRID_ADDR);
          display_fixed_addr(get_tm1629b_bitmap(CURRENT_MINUTE%10), FOURTH_GRID_ADDR);
		  SET_STB();
          write_cmd(bright_idx);
	      SET_STB();
          WAKEUP_FPAG = 3;
	        wake_up_tt1629b();
        }
    }
}

/*this is for sym4 cec */
//for cec
#define ADDR_CEC_BASE 0xd000
void sym4_cec_setup()
{
/*
pinmux config
data = hal_get_u32((volatile unsigned int *)(0xbf15b420));

data = data&(~(0x1 << 0x13));
hal_put_u32((volatile unsigned int *)(0xbf15b420),data);

hal_put_u8((volatile unsigned int *)(0xbf15b420),0x3);
data = hal_get_u32((volatile unsigned int *)(0xbf15b420));
*/
     U16 addr = ADDR_CEC_BASE+0x10;
     U8 temp_data = 0;
     
     wr_port(0x0008,0xf);
     //wr_port(0x0058,0xf);
     wr_port(addr,0x3f);
     //temp_data|0x00008100
     rd_port(addr+ 1, temp_data);
     temp_data = temp_data|0x81;//bit7:0->reset, 1:release reset
     wr_port(addr+ 1,temp_data);
     
     wr_port(addr+ 2,0x0d);
     wr_port(addr+ 3,0x01);

}
U16 g_cec_read_cnt = 0;
#define CEC_WAKEUP_DATA 0x85
#define CEC_WAKEUP_DATA2 0x87

void sym4_cec_read()
{
	U8 temp_data = 0;
	U8 rx_data = 0;
	
	rd_port(ADDR_CEC_BASE+0x1E, temp_data);
	
	if (temp_data&0x01)//REG0x1E--bit0, 1:rx message ready
	{
		//start compare cec rx data
		temp_data = 0;
		rd_port(ADDR_CEC_BASE+0x14, temp_data);
		g_cec_read_cnt = 0;
		while (!(temp_data&(0x01<<5)))//REG0x14--bit5:rx FIFO empty indicator, 1:empty
		{
			//msdelay(1);
			g_cec_read_cnt++;

			rd_port(ADDR_CEC_BASE+0x18, rx_data);//CEC FIFO, read rx data						
			
			if (rx_data == CEC_WAKEUP_DATA || rx_data == CEC_WAKEUP_DATA2 || rx_data == 0x86 || rx_data == 0x80 || rx_data == 0x82)
			{
			    WAKEUP_FPAG= 4;
				wake_up_tt1629b();
			}
			
			if(g_cec_read_cnt > 500)
			{
				g_cec_read_cnt = 0;
				break;
			}
			
			//read rx fifo cnt bit[4:0]
			rd_port(ADDR_CEC_BASE+0x14, temp_data);
		}	
	}
}

#define cec_logical_addr 3
#define g_cec_pa 0x10000

void cec_send_cmd_to_wake_tv()
{
	U16 ctrl_addr = ADDR_CEC_BASE+0x11;
	U16 data_addr = ADDR_CEC_BASE+0x18;
	U8 send_buf[5],i,int_data;
	U16 cec_data_cnt = 0;

	//wr_port(ADDR_CEC_BASE+0x10, 0x3f);
	//wr_port(ADDR_CEC_BASE+0x12, 0xf2);
	//wr_port(ADDR_CEC_BASE+0x13,  0);

	//send_buf[0] = (cec_logical_addr<<4)|0x0f;
	send_buf[0] = (cec_logical_addr<<3)|0x0f;
	send_buf[1] = 0x84;
	send_buf[2] = (g_cec_pa&0xff00)>>8;
	send_buf[3] = g_cec_pa&0xff;
	send_buf[4] = 0x03;

	wr_port(ctrl_addr, 0x80);//release cec reset

	wr_port(data_addr, 0x30);//header 2 bytes
	wr_port(data_addr, 0x4);//

	wr_port(ctrl_addr, 0x82);//set cec tx start

	while(1)
	{
		rd_port(ADDR_CEC_BASE+0x1e, int_data);//wait for tx done interrupt
		if((int_data>>1) & 0x1)
		{
			break;
		}

		//msdelay(10);
		cec_data_cnt++;
		if(cec_data_cnt>254)
		{
			break;
		}
	}
}
/*end for sym4 cec*/

void sys_init( void );
void main(void )
{
	U8 U8KeyValue = 0;
	U8 i;
	U8 ir_state = 0;
	U16 disp_char0 = BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F;
	U16 disp_charf = BIT_A+BIT_E+BIT_F+BIT_G1+BIT_G2;
	U8 ir_int = 0;
	U16 cmd = 0;
	U32 tem_sec = 0;
	parse_data();	
	sys_init();

	if(CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	standby();

	rd_port(IR_INT_CFG,ir_int);
	
			
	if(LED_DISPLAY_BRIG == 0)
	{
		bright_idx = brigth_level[3] ;
	}
	else
	{
    	bright_idx = brigth_level[LED_DISPLAY_BRIG];
	}

	for(i=0;i < 16;i++)
	{
		aomcu_write_data(0x00);       //flush fp firstly
	}
	SET_STB();
	aomcu_write_data(bright_idx);
	SET_STB();

	if(LED_DISPLAY_TIME)
	{
		display_fixed_addr(get_tm1629b_bitmap(CURRENT_HOUR/10), FIRST_GRID_ADDR);
		display_fixed_addr(get_tm1629b_bitmap(CURRENT_HOUR%10)+BIT_D1_D2, SECOND_GRID_ADDR);
		display_fixed_addr(get_tm1629b_bitmap(CURRENT_MINUTE/10), THIRD_GRID_ADDR);
		display_fixed_addr(get_tm1629b_bitmap(CURRENT_MINUTE%10), FOURTH_GRID_ADDR);
		//display_fixed_addr(BIT_D1_D2, COLON_ADDR);
	}
	else if(LED_DISPLAY_CHAR)
	{
          display_fixed_addr(disp_char0, FIRST_GRID_ADDR);//display "OFF"
          display_fixed_addr(disp_charf, SECOND_GRID_ADDR);
          display_fixed_addr(disp_charf, THIRD_GRID_ADDR);
          display_fixed_addr(0x00, FOURTH_GRID_ADDR);
         // display_fixed_addr(0x00, COLON_ADDR);
	}
	SET_STB();
  	write_cmd(bright_idx);
	SET_STB();
	
	tem_sec = ctick;
	while(1)
	{
	    if(CEC_ENABLE)
		{
			sym4_cec_read();
		}
		
		if(IR_INT_FLAG)
		{
			rd_port(IR_INT_RAWSTA,ir_state);
			if((ir_state & 0xf0) &  (ir_int & 0xf0))
			{
				WAKEUP_FPAG = 1;
				wake_up_tt1629b();
			}
		}
		
		if(ctick-tem_sec > 415)  // one sec, polling status
		{
			U8KeyValue = mcu_tt1629b_read();
			
			if (U8KeyValue == WAKE_UP_KEY )
			{
				WAKEUP_FPAG = 2;
				wake_up_tt1629b();
			}
			clock_advance_tt1629b();
			tem_sec = ctick;
		}
		else
		{
			asm("NOP") ;
		}

	}

}



//---------------------------------------------------------------------------------
// Function name: sys_init()
// Description  : Initialize mcu and ledkb register.
// Parameters   : None.
// Return       : None.
// Notes        : None.
//---------------------------------------------------------------------------------

void sys_init(void)
{
    //U8 U8Temp ;
    //U8Temp = 0;
    tick = 0;
    ctick = 0;

    //**********************************
    // Reset the chip core
    //**********************************
//wr_port((MCU_REG_BASE + RESET_TO_CORE),CORE_RESET_ENABLE); //unused
    //**********************************
    // Disable Interrupt
    //**********************************
    GIE =  0;   // Global Interrupt
    INTE    = 0b00000000 ; // disable INTE_B7~INTE_B0
    INTE_B8 = 0 ;
    INTE_B9 = 0 ;
    // set OPTION_reg for TMR0
    T0IE = 0;

    //**********************************
    // Watchdog Timer (WDT)
    //**********************************
    // wdt prescale 1 : 2048
    WDTPS3  = 1 ;
    WDTPS2  = 0 ;
    WDTPS1  = 1 ;
    WDTPS0  = 0 ;
    // wdt enable
    WDTE    = 0 ;

    //**********************************
    // TMR0 Register
    //**********************************
    // claer tmr0 and prescale counter
    TMR0 =0;
    // Prescaler 1:4096
    PS3  = 1;
    PS2  = 0;
    PS1  = 1;
    PS0  = 1;
    // TMR0 Enable, tmr0 start
    T0CS = 1; // = 1 ,off ; =0 , on


    OPTION=0x07;          // Prescaler :256                           //add for test


    //**********************************
    // enable Interrupt
    //**********************************
    INTE    = 0b00000110 ; // enable INTE_B2~1
    GIE =  1;   // enable Global Interrupt



       // enable Interrupt
    //**********************************
    INTE = 0b00001110; // enable INTE_B3~1


    INTCON = 0xa0;                                                //add for test
    INTE_B0 = 1;//irda

    GIE = 1;    // enable Global Interrupt
}

/*
Internal clock: 0x84MHz?
Clock cycle : 1/0x84 = 1.19us

首先1.19us时钟周期4分频后变成4.76us指令周期；

然后预分频器 2 分频后变成 9.52us周期 供给定时器；

定时器每隔9.52us加一 ，加到256次  256X9.52us=2437us溢出中断 ；

*/

void interrupt ISR(void) {
	//U8 U8data;//, U8temp4;
  U8 tmp_export_address_l,tmp_export_address_h;
  U8 tmp_export_rd;

  tmp_export_rd = EXPORT_RDATA;
  tmp_export_address_l = EXPORT_ADDRESS_L ;
  tmp_export_address_h = EXPORT_ADDRESS_H ;

 if (INTF_B0 & INTE_B0 )
 {
	IR_INT_FLAG = 1;
	INTF_B0 = 0;
 }

if(T0IF)
{
	tick++;
	ctick++;

	if(ctick == 0xffffffff)
		ctick = 0;

	if(tick >= 415)
	{
		CURRENT_SECOND++;
		if (CURRENT_SECOND == 60)
		{
			CURRENT_SECOND = 0;
			CURRENT_MINUTE++;

		}
		if (CURRENT_MINUTE == 60)
		{
			CURRENT_MINUTE = 0;
			CURRENT_HOUR++;
		}
		if (CURRENT_HOUR == 24){
		         PASS_DAY++;
			CURRENT_HOUR = 0;
		   }
		
		if (unix_time_flag)
		{
			unix_time_sec++;
			if(unix_time_sec ==0xff)
			{
				unix_time_sec = 0x0;
				unix_time_sec1++;
			}
			
			if(unix_time_sec1 == 0xff)
			{
				unix_time_sec1 = 0x0;
				unix_time_sec2++;
			}
			
			if(unix_time_sec2 ==0xff)
			{
				unix_time_sec3++;
				unix_time_sec2 = 0x0;
			}
		}

		//set_rtc_int_en = 1;
		tick = 0;
	}

	TMR0 =0;
	T0IF = 0; //clear interupt
}


  EXPORT_ADDRESS_L = tmp_export_address_l;
  EXPORT_ADDRESS_H = tmp_export_address_h;
  EXPORT_RDATA = tmp_export_rd;
}



