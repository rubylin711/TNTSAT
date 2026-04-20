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
                                    EXPORT_RDATA_EN = 1;                   \
                                    data=EXPORT_RDATA;						\
                                } while(0)


//static bit  set_rtc_int_en;

U8	LED_MAP[8] = {0, 1, 2, 3, 4, 5, 6, 7};	//default
// default value 0-f, ':', '-', 'b', 'o', 't'
//U8	LED_CHAR[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e, 0x79, 0x71, 0x80, 0x40, 0x5c, 0x78, 0x37, 0x0};
//0~9,b,f,Dp,-(0x40),o(0x5C),t(0x78),N(0x37)
U8	LED_CHAR[] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x7c, 0x71, 0x80, 0x40, 0x5c, 0x78, 0x37, 0x0};

#define LED_CHAR_SIZE (sizeof(LED_CHAR)/sizeof(U8))

#define  CLK_GATE_CFG0_REG_ADDR   0x0010
#define  CLK_GATE_CFG1_REG_ADDR   0x0014
#define  CLK_GATE_CFG2_REG_ADDR   0x0018
#define  CLK_GATE_CFG3_REG_ADDR   0x001c
#define  LPM_GLB_CTRL 0x001C


U16 ctick = 0;
U8 CURRENT_HOUR,CURRENT_MINUTE;
U8 WAKE_UP_HOUR,WAKE_UP_MINUTE;
U8 WAKE_UP_KEY,AUTO_WAKE_UP;
U8 STANDBY_CONFIG;
U8 LED_DISPLAY_TIME;
U8 LED_DISPLAY_CHAR;
U8 SECOND_LED_DIS;
U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
U8 LED_POS = 0;
U8 STANDBY_CONFIG2 = 0;
U8 GPEN_VAL = 0;			//1-->no define NORMAL_MEM(need repair);0-->define NORMAL_MEM(no need repair)
U8 WAKE_UP_DAY = 0;
U8 LED_DISPLAY_BRIG = 0;

U8 IR_INT_FLAG  = 0;
U8 WAKEUP_FPAG = 0;
U8 CEC_ENABLE = 0;
U8 OSC_FLAG = 0;

U8 wake_up_lock = 0;
U8 fp_reverse = 0;
U8 fd650_display_config = 0;
U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;

U8 aotimer_irq_cnt = 0;

U8 ir_state = 0;
U8 core_vcode = 0;

#define WAKEUP_TURNON_LED	7//use a value to differ from 0 and 1
#define WAKEUP_TURNOFF_LED	8
#define AO_GPIO_STATE 0x502C

#define PCB_CORE_ON

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

void msdelay(U16 ms)
{
	ctick = 0;
	while(((ctick * 24) /10) < ms)
	{
		;
	}
}

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


#define CPU0_TI_INIT0		0x00010080
#define CPU0_TI_CW			0x000100a0

/*!
  define I2C delay function in us
  */
//#define I2C_DELAY_US mtos_task_delay_us

#define R_I2C_PRER_H		0xbf158018
#define R_I2C_PRER_L		0xbf15801c
#define R_I2C_TXR			0xbf158008
#define R_I2C_RXR			0xbf15800c
#define R_I2C_SR			0xbf158004
#define R_I2C_CTR			0xbf158010
#define R_I2C_CR			0xbf158014

//sym4 use these register
/*!
 * RTC enable register
 */
#define RTC_EN				0xC400
/*!
 * RTC param register
 */
#define RTC_PARAMETER_H		0xC404
/*!
 * RTC param register
 */
#define RTC_PARAMETER_L		0xC408
/*!
 * RTC status register
 */
#define RTC_STA				0xC40C



#define WAKEUP_EN			0x0054

#define LPM_POW_CTRL		0x0004

#define IR_INT_RAWSTA		0x1014
#define IR_INT_CFG			0x1010

#define AO_TIMER_RESET		0x385C


#define AO_TIMER0_CW		0xC088//[0]:aotimer0 open(1)/close(0);[1]:loop(0)/once(1)
#define AO_TIMER0_INIT		0xC080//1s:0xF4240-1=0xF423F
#define AO_TIMER0_MODE		0xC090

#define AO_TIMER1_CW		0xC188//[0]:aotimer1 open(1)/close(0);[1]:loop(0)/once(1);bit[16]:64bit timer(1)
#define AO_TIMER1_INIT		0xC180//1s:0xF4240-1=0xF423F


// command of read key input
#define FD650_GET_KEY		0x0700 // read key input and return the key value

#define FD650_DIG0			0x1400 // display data on seg 0(need 8-bits-data)
#define FD650_DIG1			0x1500 // display data on seg 1(need 8-bits-data)
#define FD650_DIG2			0x1600 // display data on seg 2(need 8-bits-data)
#define FD650_DIG3			0x1700 // display data on seg 3(need 8-bits-data)


//set the command of system parameter
#define FD650_BIT_ENABLE	0x01 // open/close bit
#define FD650_BIT_SLEEP		0x04 // sleep control bit
#define FD650_BIT_7SEG		0x08 // 7 segment control bit
#define FD650_BIT_INTENS1	0x10 // 1 level brightness
#define FD650_BIT_INTENS2	0x20 // 2 level brightness
#define FD650_BIT_INTENS3	0x30 // 3 level brightness
#define FD650_BIT_INTENS4	0x40 // 4 level brightness
#define FD650_BIT_INTENS5	0x50 // 5 level brightness
#define FD650_BIT_INTENS6	0x60 // 6 level brightness
#define FD650_BIT_INTENS7	0x70 // 7 level brightness
#define FD650_BIT_INTENS8	0x00 // 8 level brightness

#define FD650_SYSOFF		0x0400 // disable display and key input
#define FD650_SYSON			(FD650_SYSOFF | FD650_BIT_ENABLE) // enable display and key input
#define FD650_8SEG_ON		(FD650_SYSON | 0x00) // enable 8seg mode
#define FD650_SYSON_4		(FD650_SYSON | FD650_BIT_INTENS4) // enable display, key input, 4 level brightness
#define FD650_SYSON_8		(FD650_SYSON | FD650_BIT_INTENS8) // enable display, key input, 8 level brightness

static S8 i2c_check_opdone_status(U8 i2c_id, U8 cmd)
{
	U8 data = 0;

	rd_port(R_I2C_SR,data);
	x_zero_delay();
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

static S8 mcu_i2c_stop(U8 i2c_id)
{
	volatile U32 retry_cnt = 0;
	S8 ret = 0;
	U8 temp = 0;

	wr_port(R_I2C_CR, I2C_CR_STP);
	x_zero_delay();
	rd_port(R_I2C_CR,temp);
	x_zero_delay();

	while(1) // default value
	{
		rd_port(R_I2C_CR,temp);
		x_zero_delay();
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
	ret = i2c_check_opdone_status(i2c_id, I2C_CR_STP);

	return ret;
}

static U8 mcu_i2c_wbyte(U8 i2c_id, U8 ucWdata, U8 bSetStart)
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
	x_zero_delay();
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
	x_zero_delay();

	rd_port(R_I2C_CR,temp);
	x_zero_delay();
	/* Wait write byte done. */
	while(1) // default value
	{
		rd_port(R_I2C_CR,temp);
		x_zero_delay();
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
	ret = i2c_check_opdone_status(i2c_id, I2C_CR_WR);

	return ret;
}

static S8 mcu_i2c_rbyte(U8 i2c_id, U8 *p_value, U8 nack)
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
	x_zero_delay();
	/* Wait read byte done. */
	while(1) // default value
	{
		rd_port(R_I2C_CR,temp);
		x_zero_delay();
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
	ret = i2c_check_opdone_status(i2c_id, I2C_CR_RD);

#if I2C_DEBUG
	wr_port(0x24, 0x11);
	x_zero_delay();
	wr_port(0x0054, 0x1);
	x_zero_delay();
	while(1);
#endif

	if(0 == ret)
	{
		/* get read data */
		rd_port(R_I2C_RXR,temp);
		x_zero_delay();
#if I2C_DEBUG
		if(temp !=0)
		{
		wr_port(0x24, temp);
		x_zero_delay();
		wr_port(0x0054, 0x1);
		x_zero_delay();
		while(1);
		}
#endif
		*p_value = temp;
	}
	return ret;
}

static S8 mcu_i2c_concerto_raw_read_byte(U8  *p_data, U8 b_nack)
{
	return mcu_i2c_rbyte(2,p_data, b_nack);
}

static S8 mcu_i2c_concerto_raw_write_byte(U8 data, U8 b_start)
{
	return mcu_i2c_wbyte(2,data, b_start);
}

static S8 mcu_i2c_concerto_raw_stop(void)
{
	return mcu_i2c_stop(2);
}

U8 mcu_i2c_raw_write_byte(U8 data, U8 b_start)
{
	U8 ret = ERR_NOFEATURE;
	mcu_i2c_concerto_raw_write_byte(data, b_start);
	return ret;
}

U8 mcu_i2c_raw_read_byte(U8 *p_data, U8 b_nack)
{
	U8 ret = ERR_NOFEATURE;
	ret = mcu_i2c_concerto_raw_read_byte(p_data, b_nack);
	return ret;
}

U8 mcu_i2c_raw_stop(void)
{
	U8 ret = ERR_NOFEATURE;
	mcu_i2c_concerto_raw_stop();
	return ret;
}

static U8 mcu_fd650_read() //read the key value
{
	U8 keycode = 0;
	mcu_i2c_raw_write_byte(((FD650_GET_KEY/128) & 0x3E) | 0x01 | 0x40, 1);
	mcu_i2c_raw_read_byte(&keycode, 1);
	mcu_i2c_raw_stop();

	if((keycode & 0x40) == 0)
	{
		keycode = 0;
	}

	return keycode;
}

static void mcu_fd650_write(U16 cmd) //write command
{
	mcu_i2c_raw_write_byte(((U8)(cmd/128) & 0x3E) | 0x40, 1);
	mcu_i2c_raw_write_byte((U8)cmd, 0);
	mcu_i2c_raw_stop();
}
#endif


static void mcu_fd650_write_t(U8 addr, U8 data) //write command
{
	mcu_i2c_raw_write_byte(addr, 1);
	mcu_i2c_raw_write_byte(data, 0);
	mcu_i2c_raw_stop();
}


void char_init ()
{
	U8 u8temp, i, j;

	if ((LED_MAP[0] == LED_MAP[1]))
	{
		return;
	}

	for (i=0; i<LED_CHAR_SIZE; i++)
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

void led_disp(U8 data,U8 num)
{
	U8 addr , pos;
	U8 bitmap = 0;
	U8 led_data =  0;

	if (data < LED_CHAR_SIZE)
	{
		bitmap =  LED_CHAR[data];
	}
	else
	{
		bitmap = 0;
	}

	if (fp_reverse)
	{
		led_data = ((bitmap&0xC0)|((bitmap&0x07)<<3)|((bitmap>>3)&0x07));
	}
	else
	{
		led_data =  bitmap;
	}

	if(STANDBY_CONFIG2 & (0x1 << 1))  //config led pos
	{
		pos = (LED_POS >> (num *2)) & 0x3;  //true pos
	}
	else
	{
		if (fp_reverse)
		   pos = 3 -num;
		else
		   pos = num ;
	}

	addr = 0x68;

	if((STANDBY_CONFIG2 & (1 << 5)) && (((STANDBY_CONFIG2 >> 6) & 0x3) == pos)) //config colon
	{
		mcu_fd650_write_t( (addr + 2 * pos), led_data | SECOND_LED_DIS);
		return;
	}

	if ((fd650_display_config & (1 << 2)) && (((fd650_display_config >> 3) & 0x3) == pos)) //config power led or other Dp led
	{
		mcu_fd650_write_t((addr + 2 * pos), led_data | LED_CHAR[12]);//power led on
		return;
	}

	if((STANDBY_CONFIG2 & (1 << 2)) && (((STANDBY_CONFIG2 >> 3) & 0x3) == pos)) //config lock or power
	{
		//mcu_fd650_write_t((addr + 2 * pos), led_data | 0x80);//lock led on
		//when wakeup, control the led according to wake_up_lock, it is reconfigured in wake_up_fd650
		if (WAKEUP_TURNON_LED == wake_up_lock)
		{
			mcu_fd650_write_t((addr + 2 * pos), led_data | LED_CHAR[12]);
		}
		else
		{
			mcu_fd650_write_t((addr + 2 * pos), led_data);
		}
		return;
	}

	mcu_fd650_write_t((addr + 2 * pos), led_data);
}

void led_disp_char(U8 data,U8 num)
{
	U8 addr , pos;

	U8 led_data = 0;

	if (data < LED_CHAR_SIZE)
	{
		led_data = LED_CHAR[data];
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

   if ((fd650_display_config & (1 << 2)) && (((fd650_display_config >> 3) & 0x3) == pos)) //config power led or other Dp led
   {
	   mcu_fd650_write_t((addr + 2 * pos), led_data | LED_CHAR[12]);//power led on
	   return;
   }

   if((STANDBY_CONFIG2 & (1 << 2)) && (((STANDBY_CONFIG2 >> 3) & 0x3) == pos)) //config lock or power
   {
	   //when wakeup, control the led according to wake_up_lock, it is reconfigured in wake_up_fd650
	   if (WAKEUP_TURNON_LED == wake_up_lock)
	   {
		   mcu_fd650_write_t((addr + 2 * pos), led_data | LED_CHAR[12]);
	   }
	   else
	   {
		   mcu_fd650_write_t((addr + 2 * pos), led_data);
	   }
	   return;
   }

   mcu_fd650_write_t((addr + 2 * pos), led_data);
}

void rtc_config_1s(void)
{
	U8 tmp = 0;
	U8 rtc_state = 0;

	rd_port(RTC_STA,rtc_state);

	/* disable it first*/
	wr_port(RTC_EN, 0x0);

	wr_port(RTC_PARAMETER_H,    0);
	wr_port(RTC_PARAMETER_H+1,  0);
	wr_port(RTC_PARAMETER_H+2,  0);
	wr_port(RTC_PARAMETER_H+3,  0);

	//0x000B4D85,1M*(20M/Freq_xtal)
	wr_port(RTC_PARAMETER_L,    0x85);
	wr_port(RTC_PARAMETER_L+1,  0x4d);
	wr_port(RTC_PARAMETER_L+2,  0x0b);
	wr_port(RTC_PARAMETER_L+3,  0x00);

	/* enable it */
	wr_port(RTC_EN, 0x1);
}


void parse_data ()
{
	U8 i;
	U8 AP_DATA[20] = {0};

	for (i=0; i<16; i++)
	{
		rd_port(LPM_MESSAGE2AO_0 + i, AP_DATA[i]);
	}

	rd_port(LPM_MESSAGE2AO_4, AP_DATA[16]);
	rd_port(LPM_MESSAGE2AO_4+1,AP_DATA[17]);
	rd_port(LPM_MESSAGE2AO_4+2,AP_DATA[18]);

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
	OSC_FLAG 		= (AP_DATA[16]>>3)&0x01;
	STANDBY_CONFIG2 = AP_DATA[14];
	CEC_ENABLE = AP_DATA[15] & 0x01;
	fd650_display_config = AP_DATA[18];
	wake_up_lock = (AP_DATA[18] >> 0x0)&0x1;
	fp_reverse =  (AP_DATA[18] >> 1)&0x1;
	GPEN_VAL		 	= STANDBY_CONFIG & 0x1;
	LED_DISPLAY_BRIG	= (STANDBY_CONFIG>>1)&0x07;
	LED_DISPLAY_CHAR 	= (STANDBY_CONFIG>>5)&0x1;
	LED_DISPLAY_TIME 	= (STANDBY_CONFIG>>6)&0x1;
	AUTO_WAKE_UP       	= (STANDBY_CONFIG>>7)&0x1;

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
	U8 temp = 0;
	U8 cur_vcode = 0;

	//power down analog blocks
    wr_port(0x7000,0xff);
    wr_port(0x7001,0xff);
    wr_port(0x7002,0xff);
    wr_port(0x7003,0xfb);
    //wr_port(0x7003,0xff);
    msdelay(1);

#ifdef PCB_CORE_ON
	wr_port(LPM_GLB_CTRL,0x5);
	wr_port(LPM_GLB_CTRL,0x7);
	wr_port(LPM_GLB_CTRL,0x6); //gpen
#endif
	msdelay(1);

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

#ifdef PCB_CORE_ON
	if(GPEN_VAL == 0)	//define NORMAL_MEM normal board
	{
		rd_port(0xb418, temp);
		temp &= ~7;
		temp |= 1;
		wr_port(0xb418, temp); //pinmux ao_gpio6
		rd_port(0x500c, temp);
		temp |= 1<<6;
		wr_port(0x500c, temp); //mark
		rd_port(0x5000, temp);
		temp |= 1<<6;
		wr_port(0x5000, temp); //pull up
		wr_port(0x70c1, 0x1); //pd cpu avs
		rd_port(0x5004, temp);
		temp &= ~(1<<6);
		wr_port(0x5004, temp); //enable out

		rd_port(0xb41c, temp);
		temp &= ~7;
		temp |= 1;
		wr_port(0xb41c, temp); //pinmux ao_gpio7
		rd_port(0x500c, temp);
		temp |= 1<<7;
		wr_port(0x500c, temp); //mark
		rd_port(0x5000, temp);
		temp |= 1<<7;
		wr_port(0x5000, temp); //pull up
		wr_port(0x70c5, 0x1); //pd sys avs
		rd_port(0x5004, temp);
		temp &= ~(1<<7);
		wr_port(0x5004, temp); //enable out
	}
	else
	{
		rd_port(LPM_SLEEP_MESSAGE+1, core_vcode);
		//core avs
		rd_port(0x2201, cur_vcode);
		rd_port(0x220c, temp);
		temp &= ~(0x3<<4);
		wr_port(0x220c, temp); //core avs use fix mode
		wr_port(0x2214, cur_vcode); //set core avs code
		rd_port(0x2200, temp);
		temp |= 1;
		wr_port(0x2200, temp); //enable core avs
		rd_port(0x70c4, temp);
		temp |= 1;
		wr_port(0x70c4, temp); //core avs dac bias circuit power on
		while(cur_vcode > core_vcode)
		{
			cur_vcode -= 1;
			wr_port(0x2214, cur_vcode); //set core avs code
			msdelay(1);
		}

		//cpu avs
		rd_port(0x2101, cur_vcode);
		rd_port(0x210c, temp);
		temp &= ~(0x3<<4);
		wr_port(0x210c, temp); //cpu avs use fix mode
		wr_port(0x2114, cur_vcode); //set cpu avs code
		rd_port(0x2100, temp);
		temp |= 1;
		wr_port(0x2100, temp); //enable cpu avs
		rd_port(0x70c0, temp);
		temp |= 1;
		wr_port(0x70c0, temp); //cpu avs dac bias circuit power on
		while(cur_vcode > core_vcode)
		{
			cur_vcode -= 1;
			wr_port(0x2114, cur_vcode); //set cpu avs code
			msdelay(1);
		}
	}
#else
	rd_port(0x0004,temp);
	temp &= ~(0x1 << 0x0);
	wr_port(0x0004,temp);
#endif
	//set sys off, mcpu enter sleep standby mode
	//wr_port(0x0004, 0x0);
}

void cec_send_cmd_to_wake_tv();
void wake_up_fd650()
{
	U8 tmp = 0;
	U8 tmp1 = 0;
	U8 tmp2 = 0;
	U8 tmp3 = 0;

	U8 loop = 10;

	U8 dis_config = 0;
	U8 cur_vcode = 0;

	tmp |= WAKEUP_FPAG&0x7;
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


	//reconfig wake_up_lock when wake up, if wake_up_lock is 1, set WAKEUP_TURNON_LED
	if (wake_up_lock)
	{
		wake_up_lock = WAKEUP_TURNON_LED;
	}
	else
	{
		wake_up_lock = WAKEUP_TURNOFF_LED;
	}

	dis_config = (fd650_display_config >> 5) & 0x03;
	if (0 == dis_config)
	{
		led_disp(CURRENT_HOUR/10,0);
		led_disp(CURRENT_HOUR%10,1);
		led_disp(CURRENT_MINUTE/10,2);
		led_disp(CURRENT_MINUTE%10,3);
	}
	else if (1 == dis_config)//turn off display
	{
		mcu_fd650_write(FD650_SYSOFF);
	}
	else if (2 == dis_config) //display 'ON'
	{
		led_disp_char(LED_CHAR_SIZE - 1,  0);//doesn't display
		led_disp_char(0,  1);//use 0 instead of 'O'
		led_disp_char(16, 2);
		led_disp_char(LED_CHAR_SIZE - 1,  3);
	}
	else if (3 == dis_config) //display 'boot'
	{
		led_disp_char(10, 0);
		led_disp_char(14, 1);
		led_disp_char(14, 2);
		led_disp_char(15, 3);
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

	wr_port(LPM_GLB_CTRL, 0x7); //power up PCB cpu vdd

	if(GPEN_VAL == 1)	//no define NORMAL_MEM
	{
		//core avs
		rd_port(0x2201, cur_vcode);
		rd_port(0x220c, tmp);
		tmp &= ~(0x3<<4);
		wr_port(0x220c, tmp); //core avs use fix mode
		wr_port(0x2214, cur_vcode); //set core avs code
		rd_port(0x2200, tmp);
		tmp |= 1;
		wr_port(0x2200, tmp); //enable core avs
		rd_port(0x70c4, tmp);
		tmp |= 1;
		wr_port(0x70c4, tmp); //core avs dac bias circuit power on
		while(cur_vcode < 128)
		{
			cur_vcode += 5;
			wr_port(0x2214, cur_vcode); //set core avs code
			msdelay(1);
		}

		//cpu avs
		rd_port(0x2101, cur_vcode);
		rd_port(0x210c, tmp);
		tmp &= ~(0x3<<4);
		wr_port(0x210c, tmp); //cpu avs use fix mode
		wr_port(0x2114, cur_vcode); //set cpu avs code
		rd_port(0x2100, tmp);
		tmp |= 1;
		wr_port(0x2100, tmp); //enable cpu avs
		rd_port(0x70c0, tmp);
		tmp |= 1;
		wr_port(0x70c0, tmp); //cpu avs dac bias circuit power on
		while(cur_vcode < 128)
		{
			cur_vcode += 5;
			wr_port(0x2114, cur_vcode); //set cpu avs code
			msdelay(1);
		}
	}
	msdelay(1);

	TMR0 =0;
	T0IF = 0; //clear interupt
	INTCON = 0x80;

	wr_port(0x0054, 0x1); //wake up
}


void clock_advance_fd650 ()
{
	if(LED_DISPLAY_TIME)
	{
		led_disp(CURRENT_HOUR/10,	0);
		led_disp(CURRENT_HOUR%10,	1);
		led_disp(CURRENT_MINUTE/10,	2);
		led_disp(CURRENT_MINUTE%10,	3);
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
            wake_up_fd650();
        }
    }

}

/*this is for sym4 cec */
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
			    WAKEUP_FPAG = 4;
				wake_up_fd650();
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
#if 1
		msdelay(5);
		cec_data_cnt++;
		if(cec_data_cnt>254)
		{
			break;
		}
#endif
	}
}
/*end for sym4 cec*/

static void ao_timer_init(U8 timer_id)
{
	//reset ao timer
	wr_port(AO_TIMER_RESET, 0x0);
	wr_port(AO_TIMER_RESET, 0x1);

	if(0 == timer_id)
	{
		//sym6 A0 aotimer bug
		//wr_port(AO_TIMER0_MODE, 0x1);

		//1S:0xF423F
		//AO_TIMER1_CW[16] should be 0
		wr_port(AO_TIMER0_INIT, 0x40);
		wr_port(AO_TIMER0_INIT+1, 0x42);
		wr_port(AO_TIMER0_INIT+2, 0x0f);
		wr_port(AO_TIMER0_INIT+3, 0x0);
		//start aotimer
		wr_port(AO_TIMER0_CW, 0x1);
	}
	else if(1 == timer_id)
	{
		wr_port(AO_TIMER1_INIT, 0x3f);
		wr_port(AO_TIMER1_INIT+1, 0x42);
		wr_port(AO_TIMER1_INIT+2, 0x0f);
		wr_port(AO_TIMER1_CW, 0x1);
	}
}

static void ao_timer_isr_clean(U8 timer_id)
{
	U8 val = 0;
	if(0 == timer_id)
	{
		rd_port(AO_TIMER0_CW, val);
		val |= 0x8;
		//start aotimer
		wr_port(AO_TIMER0_CW, val);
	}
	else if(1 == timer_id)
	{
		rd_port(AO_TIMER1_CW, val);
		val |= 0x8;
		//start aotimer
		wr_port(AO_TIMER1_CW, val);
	}
}


void sys_init( void );
void main(void )
{
	U8 u8KeyValue;
	U8  cnt = 0 ;
	U16 cmd = 0;

	//set fp i2c pinmux
	wr_port(0xb400, 0x03);
	wr_port(0xb404, 0x03);
	parse_data();
	char_init();
	sys_init();

	if(OSC_FLAG)
	{
		rtc_config_1s();
	}

	if(CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	standby();

	ao_timer_init(0);
	SECOND_LED_DIS =  LED_CHAR[12];

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
		led_disp_char(0,  0); //display "OFF"
		led_disp_char(11, 1);
		led_disp_char(11, 2);
		led_disp_char(LED_CHAR_SIZE-1, 3);
	}

	while(1)
	{
		cnt++;
		SECOND_LED_DIS = (CURRENT_SECOND & 0x1) ? LED_CHAR[12] : 0x0;

		if(CEC_ENABLE)
		{
			sym4_cec_read();
		}
		if(IR_INT_FLAG)
		{
			WAKEUP_FPAG = 1;
			wake_up_fd650();
		}

		if(cnt == 10)  // polling status
		{
			u8KeyValue = mcu_fd650_read();

			if (u8KeyValue == WAKE_UP_KEY)
			{
				WAKEUP_FPAG = 2;
				wake_up_fd650();
			}

			clock_advance_fd650();
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
	ctick = 0;

	//**********************************
	// Reset the chip core
	//**********************************
	//wr_port((MCU_REG_BASE + RESET_TO_CORE),CORE_RESET_ENABLE); //unused
	//**********************************
	// Disable Interrupt
	//**********************************
	GIE =  0;   // Global Interrupt
	INTE    = 0b00000000; // disable INTE_B7~INTE_B0
	INTE_B8 = 0;
	INTE_B9 = 0;
	// set OPTION_reg for TMR0
	T0IE = 0;

	//**********************************
	// Watchdog Timer (WDT)
	//**********************************
	// wdt prescale 1 : 2048
	WDTPS3  = 1;
	WDTPS2  = 0;
	WDTPS1  = 1;
	WDTPS0  = 0;

	// wdt enable
	WDTE    = 0;

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
	INTE = 0b00001110; // enable INTE_B3~1

	INTCON = 0xa0;                                                //add for test
	INTE_B0 = 1;//irda
	if(OSC_FLAG)
	{
		INTE_B1 = 1;//rtc
	}
	INTE_B7 = 1;//aotimer0

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
	tmp_export_address_l = EXPORT_ADDRESS_L ;
	tmp_export_address_h = EXPORT_ADDRESS_H ;
	U8 val = 0;
	if (INTF_B0 & INTE_B0 )
	{
		rd_port(IR_INT_RAWSTA,ir_state);
		if(ir_state & 0xf0)
		{
			IR_INT_FLAG = 1;
		}
		INTF_B0 = 0;
	}


	if (OSC_FLAG)
	{
		if (INTF_B1 & INTE_B1 )
		{
			INTF_B1 = 0;

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

			rtc_config_1s();
		}
	}

	if (INTF_B7 & INTE_B7)
	{
		ao_timer_isr_clean(0);
		INTF_B7 = 0;
#if 0
		rd_port(AO_TIMER0_CW, val);
		val |= 0x8;
		//start aotimer
		wr_port(AO_TIMER0_CW, val);

		if (aotimer_irq_cnt < 200)
		{
			aotimer_irq_cnt++;
		}
#endif

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

	}


	if(T0IF)
	{
		ctick++;

		if(ctick == 0xffff)
		{
			ctick = 0;
		}

		TMR0 =0;
		T0IF = 0; //clear interupt
	}

	EXPORT_ADDRESS_L = tmp_export_address_l;
	EXPORT_ADDRESS_H = tmp_export_address_h;
}

