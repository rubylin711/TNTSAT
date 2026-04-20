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


//#define RTC_TEST  0
//#define AOGPIO0_TEST 0
//#define AOGPIO1_TEST 0
//#define GPTIME_TEST 1
//#define IR_TEST 1
#define CT1642_TEST 1
//#define FD650_TEST 0
//#define SPIFP_TEST 0

//#define I2C_DEBUG 0
#define CT1642_DEBUG 1

#define VERSION            100

#define wr_port(port_addr,data) do { \
                                    EXPORT_ADDRESS_L = port_addr;    		\
                                    EXPORT_ADDRESS_H = (port_addr>>0x08);	\
                                    EXPORT_WDATA = data;					\
                                } while(0)

#define rd_port(port_addr,data) do { \
                                    EXPORT_ADDRESS_L = port_addr;   		\
                                    EXPORT_ADDRESS_H = (port_addr>>0x08);	\
                                    EXPORT_RDATA_EN = 1;                   \
                                    data=EXPORT_RDATA;						\
                                } while(0)


//static bit  set_rtc_int_en;

U8	LED_MAP[8] = {0, 1, 2, 3, 4, 5, 6, 7};	//default
// default value 0-f, ':', '-', 'b', 'o', 't'
//U8	LED_CHAR[21] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x80, 0x40, 0x7c, 0x5c, 0x78};
U8	LED_CHAR[21] = {0xd7, 0x14, 0xcd, 0x5d, 0x1e, 0x5b, 0xdb, 0x15, 0xdf, 0x5f, 0x9f, 0xda, 0xc3, 0xdc, 0xcb, 0x8b, 0x00, 0x08, 0xda, 0xd8, 0xca};

//static bit   set_rtc_int_en;

#define  CLK_GATE_CFG0_REG_ADDR   0x0010
#define  CLK_GATE_CFG1_REG_ADDR   0x0014
#define  CLK_GATE_CFG2_REG_ADDR   0x0018
#define  CLK_GATE_CFG3_REG_ADDR   0x001c
#define  LPM_GLB_CTRL 0x001C


//U8 TEST_MODE;
U16 tick = 0;
U16 ctick = 0;
U8 CURRENT_HOUR,CURRENT_MINUTE;
U8 WAKE_UP_HOUR,WAKE_UP_MINUTE;
//U8 WAKE_UP_IRDA,WAKE_UP_IRDA1;
U8 WAKE_UP_KEY, AUTO_WAKE_UP;
//U8 IRDA_REPEAT,POWER_DOWN;
U8 STANDBY_CONFIG;
U8 LED_DISPLAY_TIME;
U8 LED_DISPLAY_CHAR;
//U8 LED_DISPLAY_ORDER;
U8 SECOND_LED_DIS;
U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
U8 LED_POS = 0;
U8 STANDBY_CONFIG2 = 0;
U8 GPEN_VAL = 0;
U8 WAKE_UP_DAY = 0;

U8 IR_INT_FLAG  = 0;
U8 WAKEUP_FPAG = 0;
U8 OSC_FLAG = 0;
U8 CEC_ENABLE = 0;
U8 KEY_INT_FLAG  = 0;
U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;

U8 ir_state = 0;

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

#define IR_INT_RAWSTA          0x1014
#define IR_INT_CFG                 0x1010


#define LEDKB_HOST_REG_BASE 0x4000
#define LEDKB_CTRL      (LEDKB_HOST_REG_BASE)
#define LEDKB_TIMESET   (LEDKB_HOST_REG_BASE + 4)
#define LEDKB_TIMESEL   (LEDKB_HOST_REG_BASE + 8)
#define LEDKB_INTMASK_KEYFILT   (LEDKB_HOST_REG_BASE + 0x0C)
#define LEDKB_CLK_PARA   (LEDKB_HOST_REG_BASE + 0x10)
#define LEDKB_DEN_PARA   (LEDKB_HOST_REG_BASE + 0x14)
#define LEDKB_DAT_PARA(i)   (LEDKB_HOST_REG_BASE + 0x18 + (i) * 4)
#define LEDKB_DAT_DELAY_NUM (LEDKB_HOST_REG_BASE + 0x60)
#define LEDKB_KEY_VALUE0   (LEDKB_HOST_REG_BASE + 0x70)
#define LEDKB_KEY_VALUE1   (LEDKB_HOST_REG_BASE + 0x74)
#define LEDKB_KEY_PRESS0   (LEDKB_HOST_REG_BASE + 0x78)
#define LEDKB_KEY_PRESS1   (LEDKB_HOST_REG_BASE + 0x7C)
#define LEDKB_KEY_RELEASE0   (LEDKB_HOST_REG_BASE + 0x80)
#define LEDKB_KEY_RELEASE1   (LEDKB_HOST_REG_BASE + 0x84)

#define AO_GPIO_STATE 0x502C
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


void msdelay(U8 ms)
{
		ctick = 0;
		while(((ctick * 24) /10) < ms)
                ;
}

void char_init ()
{
	U8 u8temp, i, j;
	for (i=0; i<21; i++)
	{
		u8temp = 0;
		for (j=0; j<8; j++)
		{
			u8temp = u8temp | ((LED_CHAR[i] & 0x1) << LED_MAP[j]);
			LED_CHAR[i] = LED_CHAR[i] >> 1;
		}
		LED_CHAR[i] = u8temp;
	}
}

void parse_data ()
{
	U8 i;
	U8	AP_DATA[20] = {0};

	for (i=0; i<16; i++)
	{
		rd_port(LPM_MESSAGE2AO_0 + i, AP_DATA[i]);
	}

	rd_port(LPM_MESSAGE2AO_4, AP_DATA[16]);
    rd_port(LPM_MESSAGE2AO_4+1, AP_DATA[17]);
	rd_port(LPM_MESSAGE2AO_4+2, AP_DATA[18]);
	rd_port(LPM_MESSAGE2AO_4+3, AP_DATA[19]);

	CURRENT_HOUR	= AP_DATA[4];
	CURRENT_MINUTE	= AP_DATA[5];
	CURRENT_SECOND 	= AP_DATA[6];
	WAKE_UP_HOUR	= AP_DATA[7];
	WAKE_UP_MINUTE	= AP_DATA[8];
	WAKE_UP_SECOND 	= AP_DATA[9];
    WAKE_UP_DAY 	= AP_DATA[10];
	WAKE_UP_KEY 	= AP_DATA[11];
	STANDBY_CONFIG 	= AP_DATA[12];
	LED_POS 		= AP_DATA[13];
	STANDBY_CONFIG2 = AP_DATA[14];
	CEC_ENABLE = AP_DATA[15] & 0x01;
	GPEN_VAL                 =  STANDBY_CONFIG & 0x1;
	LED_DISPLAY_TIME 	=  (STANDBY_CONFIG>>6)&0x1;
	LED_DISPLAY_CHAR 	=  (STANDBY_CONFIG>>5)&0x1;
	AUTO_WAKE_UP       =  (STANDBY_CONFIG>>7)&0x1;

	CEC_ENABLE   = AP_DATA[15] & 0x01;

	OSC_FLAG 		= (AP_DATA[16]>>3)&0x01;
	unix_time_flag = (AP_DATA[16] >> 0x5 ) & 0x1;
	for (i=0; i<4; i++)
	{
		LED_MAP[i<<1] = AP_DATA[i] & 0xf;
		LED_MAP[(i<<1)+1] = (AP_DATA[i] >> 4) & 0xf;
	}

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
	//U8 temp = 0;

 //   power down analog blocks
    wr_port(0x7000,0xff);
    wr_port(0x7001,0xff);
    wr_port(0x7002,0xff);
    wr_port(0x7003,0xfb);
    msdelay(2);
#if 0
	//power down PCB cpu vdd through register LPM_GLB_CTRL
    wr_port(0x001c,0x2);
    msdelay(1);
#endif
	//power down analog plls
	if(OSC_FLAG)
	{
	    wr_port(0x7004,0xef);
	    wr_port(0x7005,0x00);
	    wr_port(0x7006,0x54);
	    wr_port(0x7007,0xff);
	}
	else
	{
		wr_port(0x7004,0xef);
		wr_port(0x7005,0x00);
		wr_port(0x7006,0x44);
		wr_port(0x7007,0xff);
	}

	/*
	* set sys off, mcpu enter sleep standby mode
	* otherwise can't enter standby mode
	*/
    wr_port(0x0004, 0x0);
}

void ct1642_display_clear()
{
	U8 i = 0;

#if 1
	for(i = 0; i <  (0x5c-0x18+ 1); i++)
		wr_port(0x4018 + i,0x00); //clear
	msdelay(100);//100ms
	wr_port(0x4000,0x00);
#endif
}
void cec_send_cmd_to_wake_tv();
void wake_up_ct1642()
{
	U8 tmp = 0;
	U8 tmp1 = 0;
	U8 tmp2 = 0;
	U8 tmp3 = 0;

	ct1642_display_clear();

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

		wr_port(LPM_AOMCU_MESSAGE, tmp);
		wr_port(LPM_AOMCU_MESSAGE +1 ,tmp1);
		wr_port(LPM_AOMCU_MESSAGE +2 , tmp2);
		wr_port(LPM_AOMCU_MESSAGE +3 , tmp3);
	}

	if (CEC_ENABLE)
	{
		if(4 != WAKEUP_FPAG)
		{
			cec_send_cmd_to_wake_tv();
		}
	}

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

	msdelay(60);//60ms
#endif
	TMR0 =0;
	T0IF = 0; //clear interupt
	INTCON = 0x80;

	wr_port(0x0054, 0x1); //wake up

}


void led_disp(U8 data,U8 num)
{
	//U8 addr ;
	U8 pos;
	U8 led_data =  LED_CHAR[(data & 0x0F)];

	if(STANDBY_CONFIG2 & (0x1 << 1))  //config led pos
	{
		pos = (LED_POS >> (num *2)) & 0x3;  //true pos
	}
	else
		pos = num;

	if((STANDBY_CONFIG2 & (1 << 5)) && (((STANDBY_CONFIG2 >> 6) & 0x3) == pos)) //config colon
	{
		wr_port((0x4000 + 0x18 + pos * 4),     (~(1 << pos)) & 0x0f);
		wr_port((0x4000 + 0x18 + pos * 4 + 1), ((led_data |SECOND_LED_DIS ) << 2 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 2), ((led_data |SECOND_LED_DIS ) >> 6 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 3), 0x00);
		return;
	}
	if((STANDBY_CONFIG2 & (1 << 2)) && (((STANDBY_CONFIG2 >> 3) & 0x3) == pos)) //config lock or power
	{
		wr_port((0x4000 + 0x18 + pos * 4),     (~(1 << pos)) & 0x0f);
		wr_port((0x4000 + 0x18 + pos * 4 + 1), ((led_data | 0x80 ) << 2 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 2), ((led_data |0x80 ) >> 6 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 3), 0x00);
		return;
	}

	if(0 == STANDBY_CONFIG2 & (1 << 5) && 1 == pos) //default colon is com1
	{
		wr_port((0x4000 + 0x18 + pos * 4),     (~(1 << pos)) & 0x0f);
		wr_port((0x4000 + 0x18 + pos * 4 + 1), ((led_data | SECOND_LED_DIS ) << 2 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 2), ((led_data | SECOND_LED_DIS ) >> 6 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 3), 0x00);
		return;
	}

	wr_port((0x4000 + 0x18 + pos * 4),      (~(1 << pos)) & 0x0f);
	wr_port((0x4000 + 0x18 + pos * 4 + 1), (led_data << 2 ) & 0xff);
	wr_port((0x4000 + 0x18 + pos * 4 + 2), (led_data >> 6 ) & 0xff);
	wr_port((0x4000 + 0x18 + pos * 4 + 3), 0x00);

}


void led_disp_char(U8 data,U8 num)
{
	//U8 addr ;
	U8 pos;
	U8 led_data = 0;

	if(data != 0xff)
		led_data = LED_CHAR[(data & 0x0F)];
	else
		led_data = 0;

	if(STANDBY_CONFIG2 & (0x1 << 1))  //config led pos
	{
		pos = (LED_POS >> (num *2)) & 0x3;  //true pos
	}
	else
		pos = num;

	if((STANDBY_CONFIG2 & (1 << 2)) && (((STANDBY_CONFIG2 >> 3) & 0x3) == pos)) //config lock or power
	{
		wr_port((0x4000 + 0x18 + pos * 4),     (~(1 << pos)) & 0x0f);
		wr_port((0x4000 + 0x18 + pos * 4 + 1), ((led_data | 0x80 ) << 2 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 2), ((led_data |0x80 ) >> 6 ) & 0xff);
		wr_port((0x4000 + 0x18 + pos * 4 + 3), 0x00);
		return;
	}

	wr_port((0x4000 + 0x18 + pos * 4),      (~(1 << pos)) & 0x0f);
	wr_port((0x4000 + 0x18 + pos * 4 + 1), (led_data << 2 ) & 0xff);
	wr_port((0x4000 + 0x18 + pos * 4 + 2), (led_data >> 6 ) & 0xff);
	wr_port((0x4000 + 0x18 + pos * 4 + 3), 0x00);
}


void clock_advance_ct1642()
{
	if(LED_DISPLAY_TIME)
	{
		led_disp(CURRENT_HOUR/10,0);
		led_disp(CURRENT_HOUR%10,1);
		led_disp(CURRENT_MINUTE/10,2);
		led_disp(CURRENT_MINUTE%10,3);
	}

	if ((CURRENT_SECOND >= WAKE_UP_SECOND)&&(CURRENT_HOUR == WAKE_UP_HOUR) && (CURRENT_MINUTE == WAKE_UP_MINUTE) && (WAKE_UP_DAY == PASS_DAY))
	{
		if (AUTO_WAKE_UP)
		{
			WAKEUP_FPAG = 3;
			wake_up_ct1642();
		}
	}
}


void ct1642_init()
{
	U8 val = 0;

	wr_port(0x4004,0x20);
	wr_port(0x4005,0x0);
	wr_port(0x4006,0xa8);
	wr_port(0x4007,0x3);

	wr_port(0x4008,0xf);
	wr_port(0x4009,0x0);
	wr_port(0x400a,0x0);
	wr_port(0x400b,0x3d);

	wr_port(0x400c,0xf0);
	wr_port(0x400d,0x0f);
	wr_port(0x400e,0x10);
	wr_port(0x400f,0x0);

	wr_port(0x4010,0x0);
	wr_port(0x4011,0x0);
	wr_port(0x4012,0xb0);
	wr_port(0x4013,0x0);

	wr_port(0x4014,0x0);
	wr_port(0x4015,0x0);
	wr_port(0x4016,0x0);
	wr_port(0x4017,0x0);

	wr_port(0x4018,0x0e);
	wr_port(0x4019,0x0);
	wr_port(0x401a,0x0);
	wr_port(0x401b,0x0);

	wr_port(0x401c,0x0d);
	wr_port(0x401d,0x0);
	wr_port(0x401e,0x0);
	wr_port(0x401f,0x0);

	wr_port(0x4020,0xb);
	wr_port(0x4021,0x0);
	wr_port(0x4022,0x0);
	wr_port(0x4023,0x0);

	wr_port(0x4024,0x07);
	wr_port(0x4025,0x0);
	wr_port(0x4026,0x0);
	wr_port(0x4027,0x0);

	val = 0x1 << 2;//standby
	wr_port(0x4028,0x0f);
	wr_port(0x4029,val);
	wr_port(0x402a,0x0);
	wr_port(0x402b,0x0);
#if 0
	val = 0x2 << 2;
	wr_port(0x402c,0x0f);
	wr_port(0x402d,val);
	wr_port(0x402e,0x0);
	wr_port(0x402f,0x0);

	val = 0x4 << 2;//0x4 CH-
	wr_port(0x4030,0x0f);
	wr_port(0x4031,val);
	wr_port(0x4032,0);
	wr_port(0x4033,0);

	val = 0x8 << 2;//0x8 CH+
	wr_port(0x4034,0x0f);
	wr_port(0x4035,val);
	wr_port(0x4036,0);
	wr_port(0x4037,0);

	val = 0x10 << 2;//0x10 VOL-
	wr_port(0x4038,0x0f);
	wr_port(0x4039,val);
	wr_port(0x403a,0);
	wr_port(0x403b,0);

	val = 0x20 << 2;//0x20 VOL+
	wr_port(0x403c,0x0f);
	wr_port(0x403d,val);
	wr_port(0x403e,0);
	wr_port(0x403f,0);

	val = 0x1;//0x40 0k
	wr_port(0x4040,0x0f);
	wr_port(0x4041,0);
	wr_port(0x4042,val);
	wr_port(0x4043,0);

	val = 0x2;//0x80 menu
	wr_port(0x4044,0x0f);
	wr_port(0x4045,0);
	wr_port(0x4046,val);
	wr_port(0x4047,0);
#endif

	wr_port(0x4060,0x01);
	wr_port(0x4061,0x00);
	wr_port(0x4062,0x0);
	wr_port(0x4063,0x0);

	wr_port(0x4001,0x04);//config 1 key
	//wr_port(0x4001,0x0b);//config 8 key
	wr_port(0x4002,0x02);
	wr_port(0x4003,0x12);
	wr_port(0x4000,0x09);

}
/*this is for sym4 cec */
//for cec
#define ADDR_CEC_BASE 0xd000
void sym4_cec_setup()
{
	U8 temp_data = 0;

	//set pinmux to ao_cec
	//rd_port(0xb420, temp_data);//sym4
	rd_port(0xb428, temp_data);
	temp_data &= 0xF8;
	temp_data |= 0x02;
	wr_port(0xb428, temp_data);

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
				wake_up_ct1642();
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

// read the key press value
static U8 ct1642_read_keypress(void)
{
	U8 keycode = 0;
	U8 keycode0 = 0;
	U8 keycode1 = 0;
	U8 key_en = 0;

	U8  key_int_status = 0;
	//U8 key_val[4] = {0};
	U8 data1 = 0;
	U8 data2 = 0;
	U8 data3 = 0;
	U8 data4 = 0;

	rd_port(LEDKB_KEY_PRESS0, data1);
	rd_port(LEDKB_KEY_PRESS0 + 1, data2);
	rd_port(LEDKB_KEY_PRESS0 + 2, data3);
	rd_port(LEDKB_KEY_PRESS0 + 3, data4);

	key_int_status = data4 & 0x0f;

	rd_port(LEDKB_INTMASK_KEYFILT + 2, key_en);
	if (key_en & 0x10)
	{
		// clear KEY_INTSTA
		wr_port(LEDKB_KEY_PRESS0,  data1);
		wr_port(LEDKB_KEY_PRESS0 + 1, data2);
		wr_port(LEDKB_KEY_PRESS0 + 2, data3);
		wr_port(LEDKB_KEY_PRESS0 + 3, data4);

		keycode0 = ((data2 & 0xf) << 4) | (data1 >> 4);
	}
	if (key_en & 0x20)
	{
		rd_port(LEDKB_KEY_PRESS1,  data1);
		rd_port(LEDKB_KEY_PRESS1 + 1, data2);
		rd_port(LEDKB_KEY_PRESS1 + 2, data3);
		rd_port(LEDKB_KEY_PRESS1 + 3, data4);

		wr_port(LEDKB_KEY_PRESS1,  data1);
		wr_port(LEDKB_KEY_PRESS1 + 1, data2);
		wr_port(LEDKB_KEY_PRESS1 + 2, data3);
		wr_port(LEDKB_KEY_PRESS1 + 3, data4);

		keycode1 = ((data2 & 0xf) << 4) | (data1 >> 4);
	}

	//Any key been press
	if (key_int_status & 0x05)
	{
		keycode = keycode0 | keycode1;
	}

	return keycode;
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

		msdelay(5);
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
	U8 u8KeyValue;
	U8 cnt = 0 ;
	U8 disp_char0 = 0x0;
	U8 disp_charf = 0xf;
	U8 char_ff = 0xff;

	//pinmux ledkb
	wr_port(0xb400, 0x0);
	wr_port(0xb404, 0x0);
	wr_port(0xb408, 0x0);

	parse_data();
	char_init();
	sys_init();
	if(CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	standby();
	ct1642_init();

	SECOND_LED_DIS =  LED_CHAR[16];

	if(LED_DISPLAY_TIME)
	{
		led_disp(CURRENT_HOUR/10,0);
		led_disp(CURRENT_HOUR%10,1);
		led_disp(CURRENT_MINUTE/10,2);
		led_disp(CURRENT_MINUTE%10,3);
	}
	else if(LED_DISPLAY_CHAR)
	{
		led_disp_char(disp_char0  ,0); //display "OFF"
		led_disp_char(disp_charf  ,1);
		led_disp_char(disp_charf  ,2);
		led_disp_char(char_ff     ,3);
	}

	KEY_INT_FLAG = 0;
	while(1)
	{
		cnt++;
		if(CEC_ENABLE)
		{
			sym4_cec_read();
		}
		SECOND_LED_DIS = (CURRENT_SECOND & 0x1) ? LED_CHAR[16] : 0x0;


		if(IR_INT_FLAG)
		{
			WAKEUP_FPAG = 1;
			wake_up_ct1642();
		}

		if (KEY_INT_FLAG)
		{
			KEY_INT_FLAG = 0;
			u8KeyValue = ct1642_read_keypress();
			if (u8KeyValue == WAKE_UP_KEY )
			{
				WAKEUP_FPAG = 2;
				wake_up_ct1642();
			}
		}


		if(cnt == 10)  // polling status
		{
			clock_advance_ct1642();
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
	//wr_port((MCU_REG_BASE + RESET_TO_CORE),CORE_RESET_ENABLE);
    //**********************************
    // Disable Interrupt
    //**********************************
    GIE =  0;   // Global Interrupt
    INTE    = 0b00000000 ; // disable INTE_B7~INTE_B0
    INTE_B8 = 0 ;
    INTE_B9 = 0 ;
	INTE_B2 = 0;
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
    INTF_B2 = 0;
	INTE_B2 = 1;//enable key
	INTF_B2 = 0;

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
	tmp_export_address_l = EXPORT_ADDRESS_L ;
	tmp_export_address_h = EXPORT_ADDRESS_H ;

	if (INTF_B0 & INTE_B0 )
	{
		rd_port(IR_INT_RAWSTA,ir_state);
		if(ir_state & 0xf0)
		{
			IR_INT_FLAG = 1;
		}
		INTF_B0 = 0;
	}

	if (INTF_B2 & INTE_B2 )
	{
		KEY_INT_FLAG = 1;
		INTF_B2 = 0;
	}


	if(T0IF)
	{
		tick++;
		ctick++;

		if(ctick == 0xffff)
		{
			ctick = 0;
		}

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
}



