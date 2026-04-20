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
                                    GIE = 0;                   				\
                                    asm("clrf 21");                   		\
                                    asm("clrf 21");                   		\
                                    GIE = 1;                   				\
                                    data=EXPORT_RDATA;						\
                                } while(0)


#define  CLK_GATE_CFG0_REG_ADDR   0x0010
#define  CLK_GATE_CFG1_REG_ADDR   0x0014
#define  CLK_GATE_CFG2_REG_ADDR   0x0018
#define  CLK_GATE_CFG3_REG_ADDR   0x001c
#define  LPM_GLB_CTRL 0x001C


#define IR_INT_RAWSTA          0x1014
#define IR_INT_CFG                 0x1010
//fix bug 99011 start
//#define AOGPIO_CEC  (6)

#define RTC_EN            0x2000//0xBF152000
/*!
 * RTC param register
 */
#define RTC_PARAMETER_H   0x2004//0xBF152004
/*!
 * RTC param register
 */
#define RTC_PARAMETER_L   0x2008//0xBF152008
/*!
 * RTC status register
 */
#define RTC_STA           0x200c//0xBF15200C
//fix bug 99011 end

U16 tick = 0;
U16 ctick = 0;
U8 CURRENT_HOUR,CURRENT_MINUTE;
U8 WAKE_UP_HOUR,WAKE_UP_MINUTE;
U8 AUTO_WAKE_UP;
U8 STANDBY_CONFIG;
U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
U8 GPEN_VAL = 0;
U8 WAKE_UP_DAY = 0;
U8 WAKE_UP_KEY = 0;
U8 XTAL_CFG = 0;//fix bug 99011
//fix bug 100018
U8 CEC_AOGPIO = 0;
U8 CEC_ENABLE = 0;

U8 IR_INT_FLAG  = 0;
U8 AOGPIO_LEVEL;
U8 WAKEUP_FPAG = 0;

U8 aogpio_shutdown_int = 0;
U8 cur_int_type = 0;
//U8 step_stat = 0;
U8 cec_wake = 0;

U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;
void aomcu_delay(int times)
{
	int i = 0;
	int j = 0;
	for(j=0; j<times; j++)
	{
		for(i=0; i<times; i++)
			asm ("nop");
	}
}

void msdelay(U8 ms)
{
	ctick = 0;
	while(((ctick * 24) /10) < ms)
		;
}

U8 mcu_aogpio_read(U8 aogpio)
{
	U8 temp = 0;
	rd_port(AO_GPIO_RDATA, temp);

	temp = (temp >> aogpio) & 0x01;
	return temp;
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
    CURRENT_SECOND 	= AP_DATA[6];
	WAKE_UP_HOUR	= AP_DATA[7];
	WAKE_UP_MINUTE	= AP_DATA[8];
	WAKE_UP_SECOND 	= AP_DATA[9];
    WAKE_UP_DAY 	= AP_DATA[10];

    WAKE_UP_KEY 	= AP_DATA[11] & 0x07; //aogpio0~aogpio7
    STANDBY_CONFIG 	= AP_DATA[12];
    //LED_POS          = AP_DATA[10];
	//STANDBY_CONFIG2  = AP_DATA[11];
	//fix bug 100018
	CEC_ENABLE   = AP_DATA[15] & 0x01;

    GPEN_VAL = STANDBY_CONFIG&0x1;
    //LED_DISPLAY_TIME 	=  (STANDBY_CONFIG>>6)&0x1;
    //LED_DISPLAY_CHAR 	=  (STANDBY_CONFIG>>5)&0x1;
    AUTO_WAKE_UP = (STANDBY_CONFIG>>7)&0x1;

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

	AOGPIO_LEVEL = mcu_aogpio_read(WAKE_UP_KEY);
}

//fix bug 99011 start
void rtc_config(U16 rtc_us, U16 enable)
{
#if 1
	U16 count_ahbclks = 0;
	U16 freq = 0;
	//U8 rtc_sta_val = 0;
	U8 tmp = 0;

	/*
	 *		BF140020[30]
	 *		XTAL_CFG	30	R crystal config
	 *		0：27MHz
	 *		1：24MHz
	 */
	if(XTAL_CFG)//rd_port(XTAL_CFG) & (1 << 30))
	{
		freq = 24;//24000000;
	}
	else
	{
		freq = 27;//27000000;
	}

	//count_ahbclks = (freq/1000000)* rtc_us;
	//count_ahbclks = (27000000/1000000)* rtc_us;
	count_ahbclks = freq * rtc_us;

	/*let's read to clear it*/
	//rd_port(RTC_STA, rtc_sta_val);
	//rtc_sta_val +=1;

	/* disable it first*/
	rd_port(RTC_EN, tmp);
	tmp &= ~0x1;
	wr_port(RTC_EN, tmp);

	/* we do not know how they use RTC, so we configure params anyway */
#if 0
	wr_port(RTC_PARAMETER_H, (count_ahbclks >> 8) & 0xff);//(count_ahbclks >> 32) & 0x3ffff;
	wr_port(RTC_PARAMETER_L, count_ahbclks & 0xff);//count_ahbclks  & 0xffffffff;
#endif

	wr_port(RTC_PARAMETER_H,     0);//(count_ahbclks >> 32) & 0x3ffff;
	wr_port(RTC_PARAMETER_H+1,   0);
	wr_port(RTC_PARAMETER_H+2,   0);
	wr_port(RTC_PARAMETER_H+3,   0);

	wr_port(RTC_PARAMETER_L,     count_ahbclks & 0xff);//count_ahbclks  & 0xffffffff;
	wr_port(RTC_PARAMETER_L+1,   (count_ahbclks >> 8) & 0xff);
	//wr_port(RTC_PARAMETER_L+2, (count_ahbclks >> 16) & 0xff);
	//wr_port(RTC_PARAMETER_L+3, (count_ahbclks>>24) & 0xff);
	wr_port(RTC_PARAMETER_L+2,   0);
	wr_port(RTC_PARAMETER_L+3,   0);


	/* disable it? return without hesitancy */
	if (0 == enable)
		return ;

	/* enable it */
	rd_port(RTC_EN, tmp);
	tmp |= 0x1;
	wr_port(RTC_EN, tmp);
#endif
}

U8 rtc_state(void)
{
	U8 tmp = 0;
	rd_port(RTC_STA, tmp);

#if 1
	if(tmp & 0x1)
        return 1;
    else
        return 0;
#endif
}
//fix bug 99011 end


void standby()
{
	//U8 temp = 0;

 //   power down analog blocks
    wr_port(0x7000,0xff);
    wr_port(0x7001,0xff);
    wr_port(0x7002,0xff);
    wr_port(0x7003,0xff);
    msdelay(1);
#if 0
	//power down PCB cpu vdd through register LPM_GLB_CTRL
    wr_port(0x001c,0x2);
#endif
    msdelay(1);
	//power down analog plls
    wr_port(0x7004,0xff);
    wr_port(0x7005,0x00);
    wr_port(0x7006,0x44);
    wr_port(0x7007,0xef);
    msdelay(1);
	//power down analog plls bit28
    wr_port(0x7004,0xff);
    wr_port(0x7005,0x00);
    wr_port(0x7006,0x44);
    wr_port(0x7007,0xff);

	//set sys off, mcpu enter sleep standby mode
	wr_port(0x0004, 0x0);

}

void cec_send_cmd_to_wake_tv();
void wake_up()
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

#if 0
	/*!
	* out of low power
	* For testing GPEN lpm case
	*
	*core 1.05V power up by standby pin output high.
	*/
	rd_port(LPM_GLB_CTRL,tmp);
	tmp |= (0x1 << 0);
	wr_port(LPM_GLB_CTRL,tmp);
#endif
	msdelay(60);//60ms

	TMR0 =0;
	T0IF = 0; //clear interupt
	INTCON = 0x80;

	wr_port(0x0054, 0x1); //wake up
}


void clock_advance()
{
    if ((CURRENT_SECOND >= WAKE_UP_SECOND)&&(CURRENT_HOUR == WAKE_UP_HOUR) && (CURRENT_MINUTE == WAKE_UP_MINUTE) && (WAKE_UP_DAY == PASS_DAY))
    {
        if (AUTO_WAKE_UP)
        {
           WAKEUP_FPAG = 3;
	       wake_up();
        }
    }
}

/*this is for sym4 cec */
//for cec
#define ADDR_CEC_BASE 0xd000
void sym4_cec_setup()
{
	U8 temp_data = 0;

	//set pinmux to ao_cec
	rd_port(0xb420, temp_data);
	temp_data &= 0xF8;
	temp_data |= 0x03;
	wr_port(0xb420, temp_data);

	wr_port(0x0008, 0xf);
	//wr_port(0x0058, 0xf);
	wr_port(ADDR_CEC_BASE+0x10, 0x3f);

	//temp_data|0x00008100
	rd_port(ADDR_CEC_BASE+0x11, temp_data);
	temp_data = temp_data|0x81;//bit7:0->reset, 1:release reset
	wr_port(ADDR_CEC_BASE+0x11, temp_data);

	wr_port(ADDR_CEC_BASE+0x12, 0x0d);
	wr_port(ADDR_CEC_BASE+0x13, 0x01);
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
				wake_up();
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
	U8 cnt = 0;
	U8 ir_state = 0;
	U8 ir_int = 0;
	U8 val = 0;

	parse_data();
	sys_init();

	if(CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	standby();

	rd_port(IR_INT_CFG,ir_int);

	while(1)
	{
		cnt++;
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
				wake_up();
			}
		}

		if (aogpio_shutdown_int)
		{
			WAKEUP_FPAG = 5;
			wake_up();
		}

		//fix bug 99011 start
		if (cec_wake)
		{
			cec_wake = 0;
			//wr_port(LPM_AOMCU_MESSAGE, 0x55);
			WAKEUP_FPAG = 4;/*through it, ap cpu can know it was woke up by what*/
			wake_up();
		}
        //fix bug 99011 end

		if(cnt == 10)  // polling status
		{
#if 0
			val = mcu_aogpio_read(WAKE_UP_KEY);
			if(val != AOGPIO_LEVEL)
			{
				WAKEUP_FPAG = 2;
				wake_up();
			}
#endif
			clock_advance();
			cnt = 0 ;
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
    //U8 u8Temp ;
    //u8Temp = 0;
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
	//fix bug 99011 start
    INTE_B1 = 1;//rtc
    INTE_B3 = 1;//aogpio_init0
    INTE_B4 = 1;//aogpio_init1
	//fix bug 99011 end
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
	U8 tmp_export_address_l,tmp_export_address_h, tmp_export_rd;
	U8 tmp = 0;//fix bug 99011

	tmp_export_rd = EXPORT_RDATA;
	tmp_export_address_l = EXPORT_ADDRESS_L ;
	tmp_export_address_h = EXPORT_ADDRESS_H ;

	if (INTF_B0 & INTE_B0 ) //irda
	{
		IR_INT_FLAG = 1;
		INTF_B0 = 0;
	}

	//fix bug 99011 start
	if (INTF_B1 & INTE_B1 ) //rtc
	{
		INTF_B1 = 0;
	}

	if (INTF_B3 & INTE_B3 ) //aogpio_init0
	{
		aogpio_shutdown_int = 1;
		INTF_B3 = 0;
	}

	if (INTF_B4 & INTE_B4 ) //aogpio_init1
	{
		aogpio_shutdown_int = 1;
		INTF_B4 = 0;
	}
	//fix bug 99011 end


	if(T0IF)
	{
		tick++;
		ctick++;

		if(ctick == 0xffff)
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

			if (CURRENT_HOUR == 24)
			{
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



