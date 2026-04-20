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

#define FD650_TEST 1
#define I2C_DEBUG 0

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

#define  CLK_GATE_CFG0_REG_ADDR   0x0010
#define  CLK_GATE_CFG1_REG_ADDR   0x0014
#define  CLK_GATE_CFG2_REG_ADDR   0x0018
#define  CLK_GATE_CFG3_REG_ADDR   0x001c
#define  LPM_GLB_CTRL 0x001C

#define IR_INT_RAWSTA	0x1014
#define IR_INT_CFG		0x1010

// default value 0-f, ':', '-', 'b', 'o', 't'
U8	LED_CHAR[21] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x80, 0x40, 0x7c, 0x5c, 0x78};
//static bit   set_rtc_int_en;


U16 tick = 0;
U16 ctick = 0;
//U32 curr_tick = 0;
U8 CURRENT_HOUR,CURRENT_MINUTE;
U8 WAKE_UP_HOUR,WAKE_UP_MINUTE;
U8 AUTO_WAKE_UP;
U8 STANDBY_CONFIG;
U8 STANDBY_CONFIG2 = 0;

U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
//U8 GPEN_VAL = 0;
U8 WAKE_UP_DAY = 0;

U8 IR_INT_FLAG  = 0;
U8 WAKEUP_FPAG = 0;
U8 WAKE_UP_KEY = 0;
U8 ADC_KEY_INT_FLAG = 0;
U8 ADC_KEY_TYPE = 0;//uio_kadc_type_t

U8 CEC_ENABLE = 0;


U8 LED_DISPLAY_TIME;
U8 LED_DISPLAY_CHAR;
U8 SECOND_LED_DIS;
U8 LED_POS = 0;
U8 LED_DISPLAY_BRIG = 0;


U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;

U8 update_sec = 0;
U8 POWER_LED = 0;

U8 ir_state = 0;

void aomcu_delay(U8 times);
void msdelay(U16 ms);
void cec_send_cmd_to_wake_tv(void);


#if FD650_TEST
/*!
  I2C register bit defination
  */

/*!
  I2C controller enable
  */
#define I2C_CTR_EN          0x80
/*!
  I2C interrupts enable
  */
#define I2C_CTR_IEN        0x40

/*!
  I2C cmd: Start Transmit
  */
#define I2C_CR_STA                0x80
/*!
  I2C cmd: Stop Transmit
  */
#define I2C_CR_STP                0x40
/*!
  I2C writing command
  */
#define I2C_CR_WR                 0x20
/*!
  I2C reading command
  */
#define I2C_CR_RD                 0x10
/*!
  I2C cmd: Intterrupt acknowledge
  */
#define I2C_CR_IACK               0x08
/*!
  I2C cmd: Not acknowledge to slave
  */
#define I2C_CR_NACK               0x04

/*!
  I2C status: not recieve ACK from slave
  */
#define I2C_SR_RXNACK              0x80
/*!
  I2C status: I2c bus arbitrage invalidation flag
  */
#define I2C_SR_AL                 0x20
/*!
  I2C status: Read or Write transmit running flag
  */
#define I2C_SR_TIP                0x10
/*!
  I2C status: Interrupts Request flag
  */
#define I2C_SR_IFLAG              0x08
/*!
  define I2C delay time in us under polling mode
  */
#define I2C_TIME_OUT   20000

/*!
  define I2C default clock frequency in KHz
  */
#define I2C_DEF_CLK_KHZ 200
/*!
  Return code type. Please reference return code macro for the values.
  */
//#define U8 S8

/*!
  Success return
  */
//#define 0 ((S8)0)
/*!
  Fail for common reason
  */
#define ERR_FAILURE ((S8)-1)
/*!
  Fail for waiting timeout
  */
#define ERR_TIMEOUT ((S8)-2)
/*!
  Fail for function param invalid
  */
#define ERR_PARAM ((S8)-3)
/*!
  Fail for module status invalid
  */
#define ERR_STATUS ((S8)-4)
/*!
  Fail for module busy
  */
#define ERR_BUSY ((S8)-5)
/*!
  Fail for no enough memory
  */
#define ERR_NO_MEM ((S8)-6)
/*!
  Fail for no enough resource
  */
#define ERR_NO_RSRC ((S8)-7)
/*!
  Fail for hardware error
  */
#define ERR_HARDWARE ((S8)-8)
/*!
  Fail for feature not support
  */
#define ERR_NOFEATURE ((S8)-9)


#define CPU0_TI_INIT0 0x00010080
#define CPU0_TI_CW 0x000100a0

/*!
  define I2C delay function in us
  */
//#define I2C_DELAY_US mtos_task_delay_us

#define R_I2C_PRER_H	0xbf158018
#define R_I2C_PRER_L	0xbf15801c
#define R_I2C_TXR		0xbf158008
#define R_I2C_RXR		0xbf15800c
#define R_I2C_SR		0xbf158004
#define R_I2C_CTR		0xbf158010
#define R_I2C_CR		0xbf158014



#define WAKEUP_EN   	0x0054

#define LPM_POW_CTRL	0x0004

#define IR_INT_RAWSTA          0x1014
#define IR_INT_CFG                 0x1010


// command of read key input
#define FD650_GET_KEY 0x0700 // read key input and return the key value

#define FD650_DIG0 0x1400 // display data on seg 0(need 8-bits-data)
#define FD650_DIG1 0x1500 // display data on seg 1(need 8-bits-data)
#define FD650_DIG2 0x1600 // display data on seg 2(need 8-bits-data)
#define FD650_DIG3 0x1700 // display data on seg 3(need 8-bits-data)


//set the command of system parameter
#define FD650_BIT_ENABLE  0x01 // open/close bit
#define FD650_BIT_SLEEP   0x04 // sleep control bit
#define FD650_BIT_7SEG    0x08 // 7 segment control bit
#define FD650_BIT_INTENS1 0x10 // 1 level brightness
#define FD650_BIT_INTENS2 0x20 // 2 level brightness
#define FD650_BIT_INTENS3 0x30 // 3 level brightness
#define FD650_BIT_INTENS4 0x40 // 4 level brightness
#define FD650_BIT_INTENS5 0x50 // 5 level brightness
#define FD650_BIT_INTENS6 0x60 // 6 level brightness
#define FD650_BIT_INTENS7 0x70 // 7 level brightness
#define FD650_BIT_INTENS8 0x00 // 8 level brightness

#define FD650_SYSOFF 0x0400 // disable display and key input
#define FD650_SYSON  (FD650_SYSOFF | FD650_BIT_ENABLE) // enable display and key input
#define FD650_8SEG_ON (FD650_SYSON | 0x00) // enable 8seg mode
#define FD650_SYSON_4 (FD650_SYSON | FD650_BIT_INTENS4) // enable display, key input, 4 level brightness
#define FD650_SYSON_8 (FD650_SYSON | FD650_BIT_INTENS8) // enable display, key input, 8 level brightness

static S8 i2c_check_opdone_status(U8 cmd)
{
	U8 data = 0;

	rd_port(R_I2C_SR,data);
	aomcu_delay(1);
	if(I2C_SR_AL == (data & I2C_SR_AL))
	{
		return ERR_HARDWARE;
	}

	if(cmd == I2C_CR_WR)
	{
		/* check if receive ACK from slave */
		if(I2C_SR_RXNACK == (data & I2C_SR_RXNACK)) /* bit 9 */
		{
			return ERR_FAILURE;
		}
	}

	return 0;
}

static S8 mcu_i2c_stop(void)
{
	volatile U32 retry_cnt = 0;
	S8 ret = 0;
	U8 temp = 0;

	wr_port(R_I2C_CR, I2C_CR_STP);
	aomcu_delay(1);
	rd_port(R_I2C_CR,temp);
	aomcu_delay(1);

	while(1) // default value
	{
		rd_port(R_I2C_CR,temp);
		aomcu_delay(1);
		if(temp == 0x04)
		{
			break;
		}

		if(retry_cnt == I2C_TIME_OUT)
		{
			/* time out */
			return ERR_HARDWARE;
		}
		retry_cnt++;
	}

	/* check result status */
	ret = i2c_check_opdone_status(I2C_CR_STP);

	return ret;
}

static S8 mcu_i2c_wbyte(U8 ucWdata, U8 bSetStart)
{
	U8 data = 0;
	volatile U32 retry_cnt = 0;
	S8 ret = 0;
	U8 temp = 0;
#if 0
	//mtos_printk("i2c_wbyte id=%d, data=0x%x, start=%d\n",
	i2c_id, ucWdata, bSetStart);
#endif
	wr_port(R_I2C_TXR, ucWdata);
	aomcu_delay(1);
	if(1 == bSetStart)
	{
		data = I2C_CR_WR | I2C_CR_STA; // Write|Start command.
	}
	else
	{
		data = I2C_CR_WR;  // Write command.
	}

	/* start write */
	wr_port(R_I2C_CR, data);
	aomcu_delay(1);

	rd_port(R_I2C_CR,temp);
	aomcu_delay(1);
	/* Wait write byte done. */
	while(1) // default value
	{
		rd_port(R_I2C_CR,temp);
		aomcu_delay(1);
		if(temp == 0x04)
		{
			break;
		}

		if(retry_cnt == I2C_TIME_OUT)
		{
			/* time out */
			return ERR_HARDWARE;
		}
		retry_cnt++;
	}

	/* check result status */
	ret = i2c_check_opdone_status(I2C_CR_WR);

	return ret;
}
#if 0
static S8 mcu_i2c_rbyte(U8 *p_value, U8 nack)
{
	U8 data = 0;
	volatile U32 retry_cnt = 0;
	S8 ret = 0;
	U8 temp =0;
	////mtos_printk("i2c_rbyte id=%d\n", i2c_id);
	if(1 == nack)
	{
		data = I2C_CR_RD | I2C_CR_NACK;  // Read data without ACK.
	}
	else
	{
		data = I2C_CR_RD;  // Read data with ACK.
	}

	/* start read */
	wr_port(R_I2C_CR, data);
	aomcu_delay(1);
	/* Wait read byte done. */
	while(1) // default value
	{
		rd_port(R_I2C_CR,temp);
		aomcu_delay(1);
		if(temp == 0x04)
		{
			break;
		}

		if(retry_cnt == I2C_TIME_OUT)
		{
			/* time out */
			return ERR_HARDWARE;
		}
		retry_cnt++;
	}

	/* check result status */
	ret = i2c_check_opdone_status(I2C_CR_RD);

#if I2C_DEBUG
	wr_port(0x24, 0x11);
	aomcu_delay(1);
	wr_port(0x0054, 0x1);
	aomcu_delay(1);
	while(1);
#endif

	if(0 == ret)
	{
		/* get read data */
		rd_port(R_I2C_RXR,temp);
		aomcu_delay(1);
#if I2C_DEBUG
		if(temp !=0)
		{
		wr_port(0x24, temp);
		aomcu_delay(1);
		wr_port(0x0054, 0x1);
		aomcu_delay(1);
		while(1);
		}
#endif
		*p_value = temp;
	}
	return ret;
}

static U8 mcu_fd650_read() //read the key value
{
	U8 keycode = 0;
	mcu_i2c_wbyte(((FD650_GET_KEY/128) & 0x3E) | 0x01 | 0x40, 1);
	mcu_i2c_rbyte(&keycode, 1);
	mcu_i2c_stop();

	if((keycode & 0x40) == 0)
	{
		keycode = 0;
	}

	return keycode;
}
#endif
static void mcu_fd650_write(U16 cmd) //write command
{
	mcu_i2c_wbyte(((U8)(cmd/128) & 0x3E) | 0x40, 1);
	mcu_i2c_wbyte((U8)cmd, 0);
	mcu_i2c_stop();
}

static void mcu_fd650_write_t(U8 addr, U8 data) //write command
{
	mcu_i2c_wbyte(addr, 1);
	mcu_i2c_wbyte(data, 0);
	mcu_i2c_stop();
}

void led_disp(U8 data,U8 num)
{
	U8 addr , pos;
	U8 tmp = 0;
	U8 led_data =  LED_CHAR[(data & 0x0F)];

	if(STANDBY_CONFIG2 & (0x1 << 1))  //config led pos
	{
		pos = (LED_POS >> (num *2)) & 0x3;  //true pos
	}
	else
	{
		pos = num;
	}

	addr = 0x68;

	if((STANDBY_CONFIG2 & (1 << 5)) && (((STANDBY_CONFIG2 >> 6) & 0x3) == pos)) //config colon
	{
		if(0 == (CURRENT_SECOND & 1 ))
			tmp = led_data | SECOND_LED_DIS;
		else
			tmp = (led_data | SECOND_LED_DIS) & 0x7f;
		mcu_fd650_write_t( (addr + 2 * pos), tmp);
		return;
	}
	if((STANDBY_CONFIG2 & (1 << 2)) && (((STANDBY_CONFIG2 >> 3) & 0x3) == pos)) //config lock or power
	{
		mcu_fd650_write_t((addr + 2 * pos), led_data |0x80);
		return;
	}

	if((POWER_LED & (1 << 2)) && (((POWER_LED >> 3) & 0x3) == pos)) //config power
	{
		mcu_fd650_write_t((addr + 2 * pos), led_data |0x80);
		return;
	}

	if(0 == STANDBY_CONFIG2 & (1 << 5) && 1 == pos) //default colon is com1
	{
		mcu_fd650_write_t((addr + 2 * pos), led_data |SECOND_LED_DIS);
		return;
	}

	mcu_fd650_write_t((addr + 2 * pos), led_data);
}

void led_disp_char(U8 data,U8 num)
{
	U8 addr , pos;

	U8 led_data = 0;

	if(data != 0x7f)
	{
		led_data = LED_CHAR[(data & 0x0F)];
	}
	else
	{
		led_data = 0;
	}

	if(STANDBY_CONFIG2 & (0x1 << 1))  //config led pos
	{
		pos = (LED_POS >> (num *2)) & 0x3;  //true pos
	}
	else
	{
		pos = num;
	}

	addr = 0x68;

	if((STANDBY_CONFIG2 & (1 << 2)) && (((STANDBY_CONFIG2 >> 3) & 0x3) == pos)) //config lock or power
	{
		mcu_fd650_write_t((addr + 2 * pos), led_data |0x80);
		return;
	}

	mcu_fd650_write_t((addr + 2 * pos), led_data);
}

#endif



/*************************/

void aomcu_delay(U8 times)
{
	int i =0;
	int j = 0;
	for(j=0;j<times;j++)
	{
		for(i=0;i<times;i++)
		asm ("nop");
	}
}

void msdelay(U16 ms)
{
	ctick = 0;
	while(((ctick * 24) /10) < ms);
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
	//LED_POS                  = AP_DATA[10];
	// STANDBY_CONFIG2  = AP_DATA[11];
	ADC_KEY_TYPE	= (AP_DATA[16]>>1)&0x03;//valid value 0~3

	//GPEN_VAL		= STANDBY_CONFIG & 0x1;
	LED_DISPLAY_TIME 	=  (STANDBY_CONFIG>>6)&0x1;
	LED_DISPLAY_CHAR 	=  (STANDBY_CONFIG>>5)&0x1;
	AUTO_WAKE_UP	= (STANDBY_CONFIG>>7)&0x1;
	unix_time_flag = (AP_DATA[16]>>5)&0x01;

	POWER_LED = AP_DATA[18];
#if 0//have no led, this is useless
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
	//U8 temp = 0;

 	//power down analog blocks
    wr_port(0x7000,0xff);
    wr_port(0x7001,0xff);
    wr_port(0x7002,0xff);
    wr_port(0x7003,0xfb);
    msdelay(1);

#if 0
	//enable GPIO to control global power,LPM_GLB_CTRL(0xbf15001c)
	rd_port(LPM_GLB_CTRL,temp);
	temp |= (0x1 << 1);

	if(GPEN_VAL)
	{
		temp |= (0x1 << 0);
	}
	else
	{
		temp &= ~(0x1 << 0);
	}
	wr_port(LPM_GLB_CTRL,temp);
#endif

	//power down analog plls bit28

	wr_port(0x7004,0xef);
	wr_port(0x7005,0x00);
	wr_port(0x7006,0x44);
	wr_port(0x7007,0xff);

	msdelay(50);
	wr_port(0x0004, 0x0);//LPM_POW_CTRL(0xbf150004)
}


void wake_up()
{
	U8 tmp = 0;
	U8 tmp1 = 0;
	U8 tmp2 = 0;
	U8 tmp3 = 0;

	tmp | = WAKEUP_FPAG&0x7;
	tmp | = 0x80;
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
#endif

	msdelay(10);//60ms

	TMR0 =0;
	T0IF = 0; //clear interupt
	INTCON = 0x80;

	wr_port(0x0054, 0x1); //wake up,LPM_WAKEUP_EN(0xbf150054)
}

void clock_advance()
{
	if((update_sec != CURRENT_SECOND) && LED_DISPLAY_TIME)
	{
		led_disp(CURRENT_HOUR/10,	0);
		led_disp(CURRENT_HOUR%10,	1);
		led_disp(CURRENT_MINUTE/10,	2);
		led_disp(CURRENT_MINUTE%10,	3);
		update_sec = CURRENT_SECOND;
	}

    if ((CURRENT_SECOND >= WAKE_UP_SECOND)&&(CURRENT_HOUR == WAKE_UP_HOUR) && (CURRENT_MINUTE == WAKE_UP_MINUTE) && (WAKE_UP_DAY == PASS_DAY))
    {
        if (AUTO_WAKE_UP)
        {
            //here static hour and min changed, so refresh
            led_disp(CURRENT_HOUR/10, 	0);
            led_disp(CURRENT_HOUR%10, 	1);
            led_disp(CURRENT_MINUTE/10, 2);
            led_disp(CURRENT_MINUTE%10, 3);
            WAKEUP_FPAG = 3;
            wake_up();
        }
    }

}



//for key adc
#define KEY_ADC_BASE_ADDR	0x6000
#define KEY_ADC_CFG     	(KEY_ADC_BASE_ADDR)
#define KEY_ADC_VALUE   	(KEY_ADC_BASE_ADDR + 4)
#define KEY_ADC_INT 		(KEY_ADC_BASE_ADDR + 8)
#define KEY_ADC_RANGE 		(KEY_ADC_BASE_ADDR + 0x0C)
#define KADC_PLUUING_CNT 	(10)
#define KADC_AOGPIO			2

void mcu_key_adc_setup()
{
	U8 val = 0;

	rd_port(0xb408, val);
	val &= ~0x7;
	val |= 0x01;
	wr_port(0xb408, val);//set pinmux to kadc

	rd_port(AOGPIO0_SET_WRITEEN, val);
	val |= (1<<KADC_AOGPIO);
	wr_port(AOGPIO0_SET_WRITEEN, val);//aogpio2 set to write enable

	rd_port(AOGPIO0_SET_DIRECTION, val);
	val |= (1<<KADC_AOGPIO);
	wr_port(AOGPIO0_SET_DIRECTION, val);//set to input


	rd_port(0x0058,val);//AO_CTRL0
	val |= (0x1 << 1);//bit1 set 1, the 6th interrupt select KADC
	wr_port(0x0058, val);

	val = (1 << 4);
	wr_port(KEY_ADC_CFG, val);
	val = 0;
	wr_port(KEY_ADC_CFG+1, val);
	wr_port(KEY_ADC_CFG+2, val);
	wr_port(KEY_ADC_CFG+3, val);

#if 0

	//the IC will use KEY_ADC_RANGE to generate interrupt
	//if the value is 2, only one interrupt will be generated if keep press a key
	//the value was got by test.
	if (7 == WAKE_UP_KEY)
	{
		wr_port(KEY_ADC_RANGE, 1);
	}
	else
	{
		wr_port(KEY_ADC_RANGE, 2);
	}
#endif

	//enable interrupt
	//the IC will check the kadc's value according to analogy frequence,
	//if the value between twice check larger than KEY_ADC_RANGE
	//it will generate intertupt
	rd_port(KEY_ADC_INT, val);
	val |= (1 << 4);
	wr_port(KEY_ADC_INT, val);

	//clear interrupt
	if (val & 0x01)
	{
		val |= (1 << 0);
		wr_port(KEY_ADC_INT, val);
	}
}

U8 mcu_kadc_read()
{
    U8 val = 0;
    U8 i = 0;
    U8 valid_cnt = 0;
    U8 vals[KADC_PLUUING_CNT] = {0};

    //rd_port(KEY_ADC_VALUE, val); //read key value from Reg 0xbf156004
    for (i = 0; i < KADC_PLUUING_CNT; i++)
	{
        rd_port(KEY_ADC_VALUE, vals[i]);
        if (i > 0)
        {
            if (vals[i] != vals[i-1])
            {
                val = vals[i];
            }
            else
            {
            	valid_cnt++;
            }
        }

        if (i == 0)
        {
        	val = vals[i];
        }

        if (valid_cnt >= 2)
		{
			break;
        }
    }

    return val;
}


void aomcu_kadc_wakeup(U8 key_value)
{
	U8 wake_flg = 0;
	switch(ADC_KEY_TYPE)
	{
		case 0://KADC_KEY_TYPE_3
			switch(WAKE_UP_KEY)
			{
				case 1:
					if (key_value <= 5)
					{
						wake_flg = 1;
					}
					break;

				case 2:
					if (key_value >= 6 && key_value <= 16)
					{
						wake_flg = 1;
					}
					break;

				case 3:
					if (key_value >= 17 && key_value <= 27)
					{
						wake_flg = 1;
					}
					break;

				default:
					break;
			}
			break;

		case 1://KADC_KEY_TYPE_5
			switch(WAKE_UP_KEY)
			{
				case 1:
					if (key_value <= 3)
					{
						wake_flg = 1;
					}
					break;

				case 2:
					if (key_value >= 4 && key_value <= 10)
					{
						wake_flg = 1;
					}
					break;

				case 3:
					if (key_value >= 11 && key_value <= 17)
					{
						wake_flg = 1;
					}
					break;

				case 4:
					if (key_value >= 18 && key_value <= 24)
					{
						wake_flg = 1;
					}
					break;

				case 5:
					if (key_value > 25 && key_value <= 31)
					{
						wake_flg = 1;
					}
					break;

				default:
					break;
			}
			break;

		case 2://KADC_KEY_TYPE_7
			switch(WAKE_UP_KEY)
			{
				case 1:
					if (key_value <= 2)
					{
						wake_flg = 1;
					}
					break;

				case 2:
					if (key_value > 2 && key_value <= 7)
					{
						wake_flg = 1;
					}
					break;

				case 3:
					if (key_value > 7 && key_value <= 12)
					{
						wake_flg = 1;
					}
					break;

				case 4:
					if (key_value > 12 && key_value <= 17)
					{
						wake_flg = 1;
					}
					break;

				case 5:
					if (key_value > 17 && key_value <= 22)
					{
						wake_flg = 1;
					}
					break;

				case 6:
					if (key_value > 22 && key_value <= 27)
					{
						wake_flg = 1;
					}
					break;

				case 7:
					if (key_value > 27 && key_value <= 32)
					{
						wake_flg = 1;
					}
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}
	if (wake_flg)
	{
		WAKEUP_FPAG = 5;
		wake_up();
	}
}

#define AOMCU_KADC_REPEAT_TICKS (125)//3 //about 300 ms
void aomcu_kadc_interrupt(void)
{
    U8 key_val = 0;
#if 0	
    static U32 start_ticks = 0;

    if (curr_tick - start_ticks < AOMCU_KADC_REPEAT_TICKS)
    {
        return ;
    }

    start_ticks = curr_tick;
#endif
    key_val = mcu_kadc_read();

    aomcu_kadc_wakeup(key_val);
}

/*this is for sym4 cec */
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
	U8 send_buf[5],int_data;
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
	U8 cnt = 0 ;
	U8 val = 0;
	U8 disp_char0 = 0x0;
	U8 disp_charf = 0xf;
	U8 char_ff = 0x7f;
	U16 cmd = 0;

	//i2c pinmux
	wr_port(0xb400, 0x03);
	wr_port(0xb404, 0x03);

	parse_data();
	sys_init();

	if(CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	standby();

	if (3 != ADC_KEY_TYPE)
	{
		mcu_key_adc_setup();
	}
	// aomcu_delay(200);

	if(1)
	{
		SECOND_LED_DIS =  LED_CHAR[16];

		if(LED_DISPLAY_BRIG == 0)
		{
			cmd = FD650_SYSON_4 | FD650_8SEG_ON ;
		}
		else
		{
			cmd = FD650_8SEG_ON | ((LED_DISPLAY_BRIG << 4) & 0xf0);
		}

		mcu_fd650_write(cmd);

		if(LED_DISPLAY_TIME)
		{
			led_disp(CURRENT_HOUR/10,   0);
			led_disp(CURRENT_HOUR%10,   1);
			led_disp(CURRENT_MINUTE/10, 2);
			led_disp(CURRENT_MINUTE%10, 3);
		}
		else if(LED_DISPLAY_CHAR)
		{
			led_disp_char(disp_char0, 0); //display "OFF"
			led_disp_char(disp_charf, 1);
			led_disp_char(disp_charf, 2);
			led_disp_char(char_ff   , 3);
		}
	}

	msdelay(50);
	ADC_KEY_INT_FLAG = 0;

	while(1)
	{
		cnt++;
		if(CEC_ENABLE)
		{
			sym4_cec_read();
		}
		if (IR_INT_FLAG)
		{
			WAKEUP_FPAG = 1;
			wake_up();
		}

		if (ADC_KEY_INT_FLAG)
		{
			aomcu_kadc_interrupt();
			rd_port(KEY_ADC_INT, val);
			val |= 1;
			wr_port(KEY_ADC_INT, val);
			ADC_KEY_INT_FLAG = 0;
		}

		if (cnt == 10)  // polling status
		{
			if(CEC_ENABLE)
			{
				sym4_cec_read();
			}

			clock_advance();
			cnt = 0 ;
		}
		else
		{
			asm("NOP");
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

	// enable Interrupt
	//**********************************
	INTE = 0b01001110; // enable INTE_B3~1

	INTCON = 0xa0;                                                //add for test
	INTE_B0 = 1;//irda

	if (3 != ADC_KEY_TYPE)
	{
		INTE_B6 = 1;//enable kadc
	}
	else
	{
		INTE_B6 = 0;//disable kadc
	}

	GIE = 1;    // enable Global Interrupt
}

/*
Internal clock: 0x84MHz?
Clock cycle : 1/0x84 = 1.19us

首先1.19us时钟周期4分频后变成4.76us指令周期；

然后预分频器 2 分频后变成 9.52us周期 供给定时器；

定时器每隔9.52us加一 ，加到256次  256X9.52us=2437us溢出中断 ；

*/
void interrupt ISR(void)
{
	//U8 U8data;//, U8temp4;
	U8 tmp_export_address_l,tmp_export_address_h;
	U8 tmp_export_rd;
	U8 val;

	tmp_export_rd = EXPORT_RDATA;
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

	//add for kadc interrupt
	if (INTF_B6 & INTE_B6 )
	{
		ADC_KEY_INT_FLAG = 1;
		INTF_B6 = 0;
	}

	if(T0IF)
	{
		tick++;
		ctick++;
		//curr_tick++;

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
				if(unix_time_sec == 0xff)
				{
					unix_time_sec = 0x0;
					if(unix_time_sec1 == 0xff)
					{
						unix_time_sec1 = 0x0;
						if(unix_time_sec2 == 0xff)
						{
							unix_time_sec2 = 0x0;
							unix_time_sec3++;
						}
						else
						{
							unix_time_sec2++;
						}
					}
					else
					{
						unix_time_sec1++;
					}
				}
				else
				{
					unix_time_sec++;
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

