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
//static bit   set_rtc_int_en;

#define MAX_BYTES_COUNT	(64)
U8 buff[MAX_BYTES_COUNT];


#define  CLK_GATE_CFG0_REG_ADDR   0x0010  
#define  CLK_GATE_CFG1_REG_ADDR   0x0014  
#define  CLK_GATE_CFG2_REG_ADDR   0x0018  
#define  CLK_GATE_CFG3_REG_ADDR   0x001c  
#define  LPM_GLB_CTRL 0x001C

#define PCB_CORE_ON


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
//U8 LED_DISPLAY_CHAR;
//U8 LED_DISPLAY_ORDER;
//U8 SECOND_LED_DIS;
U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
//U8 LED_POS = 0;
U8 STANDBY_CONFIG2 = 0;
U8 GPEN_VAL = 0;			//1-->no define NORMAL_MEM(need repair);0-->define NORMAL_MEM(no need repair)
U8 WAKE_UP_DAY = 0;
U8 LED_DISPLAY_BRIG = 0;
U8 LED_DISPLAY_STA = 0;
U8 fd650_display_config = 0;
U8 OSC_FLAG = 0;

U8 IR_INT_FLAG  = 0;
U8 WAKEUP_FPAG = 0;

U8 CEC_ENABLE = 0;

U8 ir_state = 0;

U8 core_vcode = 0;

U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;

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

#define PT6393_DISPLAY_MODE_4DIG_24SEG  0x00	
#define PT6393_DISPLAY_MODE_5DIG_23SEG  0x01
#define PT6393_DISPLAY_MODE_6DIG_22SEG 0x02
#define PT6393_DISPLAY_MODE_7DIG_21SEG 0x03
#define PT6393_DISPLAY_MODE_8DIG_20SEG 0x04
#define PT6393_DISPLAY_MODE_9DIG_19SEG 0x05
#define PT6393_DISPLAY_MODE_10DIG_18SEG 0x06
#define PT6393_DISPLAY_MODE_11DIG_17SEG 0x07
#define PT6393_DISPLAY_MODE_12DIG_16SEG 0x08
#define PT6393_DISPLAY_MODE_13DIG_15SEG 0x09

#define PT6393_WRITE_TO_DISPLAY_MODE	0x00
#define PT6393_WRITE_TO_LED_PORT 		0x01
#define PT6393_READ_KEY_DATA			0x02

#define PT6393_INCREASE_ADDR	0x00
#define PT6393_FIX_ADDR		0x04

#define PT6393_PULSE_WIDTH_0	0x00
#define PT6393_PULSE_WIDTH_1	0x01
#define PT6393_PULSE_WIDTH_2	0x02
#define PT6393_PULSE_WIDTH_3	0x03
#define PT6393_PULSE_WIDTH_4	0x04
#define PT6393_PULSE_WIDTH_5	0x05
#define PT6393_PULSE_WIDTH_6	0x06
#define PT6393_PULSE_WIDTH_7	0x07

#define PT6393_DISPLAY_ON	0x08
#define PT6393_DISPLAY_OFF	0x00

U8 PT6393_STB = 2;
U8 PT6393_SCL = 1;
U8 PT6393_DATA = 0;//if MOSI and MISO use a signle pin, config the same gpio
//U8 key_temp=0;

#define GPIO_DIR_OUTPUT       0x0
#define GPIO_DIR_INPUT        0x1

#define GPIO_LEVEL_LOW        0x0
#define GPIO_LEVEL_HIGH       0x1

void mcu_aogpio_set_mask(U8 aogpio, U8 enable)
{
	U8 val = 0;
	
	rd_port(AO_GPIO_MASK, val);

	if(enable)
	{
		val |= (1 << aogpio);
	}
	else
	{
		val &= ~(1 << aogpio);
	}

	wr_port(AO_GPIO_MASK, val);
}

void mcu_aogpio_set_dir(U8 aogpio, U8 dir)
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

void mcu_aogpio_set_val(U8 aogpio, U8 val)
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

void mcu_aogpio_get_val(U8 aogpio, U8 *p_val)
{
	U8 temp = 0;
	
	rd_port(AO_GPIO_RDATA, temp);
	temp = (temp >> aogpio) & 0x01;
	
	*p_val = temp;
}

#define PT6393_STB_HIGH			mcu_aogpio_set_val(PT6393_STB, GPIO_LEVEL_HIGH)   
#define PT6393_STB_LOW				mcu_aogpio_set_val(PT6393_STB, GPIO_LEVEL_LOW)  
#define PT6393_STB_OUT_MODE		mcu_aogpio_set_dir(PT6393_STB, GPIO_DIR_OUTPUT)

#define PT6393_SCL_HIGH			mcu_aogpio_set_val(PT6393_SCL, GPIO_LEVEL_HIGH)   
#define PT6393_SCL_LOW				mcu_aogpio_set_val(PT6393_SCL, GPIO_LEVEL_LOW)  
#define PT6393_SCL_OUT_MODE		mcu_aogpio_set_dir(PT6393_SCL, GPIO_DIR_OUTPUT)

#define PT6393_DATA_HIGH   		mcu_aogpio_set_val(PT6393_DATA, GPIO_LEVEL_HIGH)  
#define PT6393_DATA_LOW   			mcu_aogpio_set_val(PT6393_DATA, GPIO_LEVEL_LOW)
#define PT6393_DATA_OUT_MODE		mcu_aogpio_set_dir(PT6393_DATA, GPIO_DIR_OUTPUT)

#define PT6393_DATA_IN_VAL(val)		mcu_aogpio_get_val(PT6393_DATA, &val)
#define PT6393_DATA_IN_MODE		mcu_aogpio_set_dir(PT6393_DATA, GPIO_DIR_INPUT)

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
	//LED_POS 		= AP_DATA[13];
	STANDBY_CONFIG2 = AP_DATA[14];
	CEC_ENABLE = AP_DATA[15] & 0x01;
	fd650_display_config = AP_DATA[18];
        GPEN_VAL                 =  STANDBY_CONFIG & 0x1;
        LED_DISPLAY_BRIG	= (STANDBY_CONFIG>>1)&0x07;
        LED_DISPLAY_TIME 	=  (STANDBY_CONFIG>>6)&0x1;	
        //LED_DISPLAY_CHAR 	=  (STANDBY_CONFIG>>5)&0x1;	
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


void PT6393_write(U8 *p_wbuf, U8 wlen)
{
	U8 i = 0;
	U8 j = 0;
	U8 dat = 0;

	PT6393_DATA_OUT_MODE;
	PT6393_STB_LOW;
	for (i=0; i<wlen; i++)
	{
		dat = p_wbuf[i];
		for (j=0; j<8; j++)
		{
			PT6393_SCL_LOW;
			if (dat & 0x01)//lsb first
			{
				PT6393_DATA_HIGH;
			}
			else 
			{
				PT6393_DATA_LOW;
			}
			asm ("nop");
			asm ("nop");
			
			PT6393_SCL_HIGH;
			dat >>= 1;
			asm ("nop");
			asm ("nop");			
		}
	}
	PT6393_STB_HIGH;
}

void PT6393_read(U8 *p_rbuf, U8 rlen)
{
	U8 i = 0;
	U8 j=0;
	U8 dat = 0;
	U8 in_data = 0;

	PT6393_DATA_IN_MODE;
	PT6393_STB_LOW;
	for (i=0; i<rlen; i++)
	{		
		dat = 0;
		for (j=0; j<8; j++)
		{
			PT6393_SCL_LOW;
			
			asm ("nop");
			asm ("nop");
			
			PT6393_SCL_HIGH;

			PT6393_DATA_IN_VAL(in_data);
			in_data <<= j;//lsb first
			dat |= in_data;
			asm ("nop");
			asm ("nop");			
		}		

		p_rbuf[i] = dat;
	}
	PT6393_STB_HIGH;
}

void PT6393_write_read(U8 *p_wbuf, U8 wlen, U8 *p_rbuf, U8 rlen)
{
	U8 i = 0;
	U8 j=0;
	U8 dat = 0;
	U8 in_data = 0;

	PT6393_DATA_OUT_MODE;
	PT6393_STB_LOW;
	for (i=0; i<wlen; i++)
	{
		dat = p_wbuf[i];
		for (j=0; j<8; j++)
		{
			PT6393_SCL_LOW;
			if (dat & 0x01)//lsb first
			{
				PT6393_DATA_HIGH;
			}
			else 
			{
				PT6393_DATA_LOW;
			}
			asm ("nop");
			asm ("nop");
			
			PT6393_SCL_HIGH;
			dat >>= 1;
			asm ("nop");
			asm ("nop");			
		}
	}

	PT6393_DATA_IN_MODE;
	asm ("nop");
	asm ("nop");
	
	for (i=0; i<rlen; i++)
	{		
		dat = 0;
		for (j=0; j<8; j++)
		{
			PT6393_SCL_LOW;
			
			asm ("nop");
			asm ("nop");
			
			PT6393_SCL_HIGH;

			PT6393_DATA_IN_VAL(in_data);
			in_data <<= j;//lsb first
			dat |= in_data;
			asm ("nop");
			asm ("nop");			
		}

		p_rbuf[i] = dat;
	}
	PT6393_STB_HIGH;
}

void pt6393_cmd_display_mode(U8 mode)
{
	U8 cmd = mode & 0x3F;//clear bit7/bit6
	PT6393_write(&cmd, 1);
}

void pt6393_cmd_date_setting(U8 addr_mode, U8 ctrl_mode)
{
	U8 cmd = 0;
	cmd = 0x40 | (addr_mode & 0x04) | (ctrl_mode & 0x03);//bit7/bit6 set to 01
	PT6393_write(&cmd, 1);
}

void pt6393_cmd_set_addr(U8 addr)
{
	U8 cmd = 0;
	cmd = 0xC0 | (addr & 0x3F);//bit7/bit6 set to 11
	PT6393_write(&cmd, 1);
}

void pt6393_cmd_display_ctrl(U8 on_off, U8 bright)
{
	U8 cmd = 0;
	cmd = 0x80 | (on_off & 0x08) | (bright & 0x07);//bit7/bit6 set to 10
	PT6393_write(&cmd, 1);
}

void pt6393_fix_addr_write(U8 addr, U8 dat)
{
	U8 buf[2];

	buf[0] = 0xC0 | (addr & 0x3F);//bit7/bit6 set to 11
	buf[1] = dat;
	PT6393_write(buf, 2);
}

void pt6393_increase_addr_write(U8 addr, U8 *p_buf, U8 len)
{
	if (len>MAX_BYTES_COUNT-1)
	{
		len = (MAX_BYTES_COUNT-1);
	}
	
	buff[0] = 0xC0 | (addr & 0x3F);//bit7/bit6 set to 11

	PT6393_write(buff, len+1);
}

void pt6393_display_num(U8 index,U8 num)
{
	if(num > 10)
		return;
	pt6393_cmd_date_setting(PT6393_FIX_ADDR,PT6393_WRITE_TO_DISPLAY_MODE);
	switch(num){
		case 0:
			pt6393_fix_addr_write(index,0x3f);
			pt6393_fix_addr_write(index+1,0x0);
			break;
		case 1:
			pt6393_fix_addr_write(index,0x6);
			pt6393_fix_addr_write(index+1,0x0);
			break;
		case 2:
			pt6393_fix_addr_write(index,0x1B);
			pt6393_fix_addr_write(index+1,0x6);
			break;
		case 3:
			pt6393_fix_addr_write(index,0x0f);
			pt6393_fix_addr_write(index+1,0x6);
			break;
		case 4:
			pt6393_fix_addr_write(index,0xa0);
			pt6393_fix_addr_write(index+1,0x16);
			break;
		case 5:
			pt6393_fix_addr_write(index,0x2d);
			pt6393_fix_addr_write(index+1,0x06);
			break;
		case 6:
			pt6393_fix_addr_write(index,0x3d);
			pt6393_fix_addr_write(index+1,0x06);
			break;
		case 7:
			pt6393_fix_addr_write(index,0x07);
			pt6393_fix_addr_write(index+1,0x0);
			break;
		case 8:
			pt6393_fix_addr_write(index,0x3f);
			pt6393_fix_addr_write(index+1,0x6);
			break;
		case 9:
			pt6393_fix_addr_write(index,0x2f);
			pt6393_fix_addr_write(index+1,0x06);
			break;
		default:
			pt6393_fix_addr_write(index,0xff);
			pt6393_fix_addr_write(index+1,0x3f);
			break;
	}

}

void pt6393_ledpower_status(U8 light)
{
	pt6393_cmd_date_setting(PT6393_FIX_ADDR,PT6393_WRITE_TO_DISPLAY_MODE);
	if(light == 1)
	{
		pt6393_fix_addr_write(52,0xff);
		pt6393_fix_addr_write(53,0xff);
	}
	else
	{
		pt6393_fix_addr_write(52,0x0);
		pt6393_fix_addr_write(53,0x0);
	}
}

void pt6393_ledcolon_status(U8 light)
{
	pt6393_cmd_date_setting(PT6393_FIX_ADDR,PT6393_WRITE_TO_DISPLAY_MODE);
	if(light == 1)
	{
		pt6393_fix_addr_write(20,0x0);
		pt6393_fix_addr_write(21,0x06);
	}
	else
	{
		pt6393_fix_addr_write(20,0x0);
		pt6393_fix_addr_write(21,0x0);
	}
}

void pt6393_clear_display(void)
{
	U8 i = 0;
	for(i = 0;i<MAX_BYTES_COUNT;i++)
		buff[i] = 0;
	pt6393_cmd_date_setting(0,0);
	pt6393_increase_addr_write(0,buff,MAX_BYTES_COUNT-1);
}
void pt6393_init()
{
	U8 tmp = 0;
	//use gpio to simulate SPI, so set the pinmux to gpio
	wr_port(0xb400, 0x1);
	wr_port(0xb404, 0x1);
	wr_port(0xb408, 0x1);

	//enable access to gpio
	mcu_aogpio_set_mask(PT6393_STB,  1);
	mcu_aogpio_set_mask(PT6393_SCL,  1);
	mcu_aogpio_set_mask(PT6393_DATA, 1);
	
	PT6393_STB_OUT_MODE;
	PT6393_SCL_OUT_MODE;
	PT6393_SCL_LOW;
	PT6393_DATA_LOW;

	pt6393_cmd_display_mode(PT6393_DISPLAY_MODE_13DIG_15SEG);
	pt6393_cmd_display_ctrl(PT6393_DISPLAY_ON, LED_DISPLAY_BRIG);

	pt6393_clear_display();

	tmp = fd650_display_config >> 2;
	tmp &= 1;
	if(tmp)
	{
		pt6393_ledpower_status(1);
	}
	else
	{
		pt6393_ledpower_status(0);
	}
	pt6393_ledcolon_status(0);
}

/********************************* 
* encode the key as below:
* K1  1    5    9    13......
* K2  2    6    10   14......
* K3  3    7    11   15......
* K4  4    8    12   16......
*    SG1  SG2  SG3  SG4
*********************************/
U8 pt6393_get_key()
{
	U8 w_data= 0;
	U8 buf[4];//every key use 12bits, 4 keys will use 48bits, 48/8=6bytes
	U8 key = 0;
	U8 i = 0, j = 0;

	for (i=0; i<4; i++)
	{
		buf[i] = 0;
	}
	w_data = 0x40 | (PT6393_FIX_ADDR & 0x04) | (PT6393_READ_KEY_DATA & 0x03);//bit7/bit6 set to 01

	//OS_PRINTF("[%s %d]w_data = 0x%x\n", __FUNCTION__, __LINE__, w_data);	

	PT6393_write_read(&w_data, 1, buf, 4);
	
	for (i=0; i<4; i++)
	{
		for (j=0; j<8; j++)
		{
			if ((buf[i]>>j)&0x01)
			{
				key = i*8 + (j+1);

				i = 4;//use this to exit out loop
				break;
			}
		}
	}			

	return key;
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
void wake_up(void)
{
	U8 tmp = 0;
	U8 tmp1 = 0;
	U8 tmp2 = 0;
	U8 tmp3 = 0;
	U8 cur_vcode = 0;

	if(LED_DISPLAY_TIME && ((STANDBY_CONFIG2 >> 5) & 0x1))
	{
		pt6393_ledcolon_status(1);
	}

	wr_port(LPM_AOMCU_WAKEUP_FLAG, 0x80);
	msdelay(2);
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
	if(CEC_ENABLE)
	{
		cec_send_cmd_to_wake_tv();
	}
	
	rd_port(0x0004,tmp);
	tmp &= ~(0x1 << 0x0);
	wr_port(0x0004,tmp); 
	
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

void clock_advance_pt6393()
{
	
	
	if(LED_DISPLAY_TIME)
	{
		if(LED_DISPLAY_STA != (CURRENT_SECOND & 0x1))
		{
			LED_DISPLAY_STA = CURRENT_SECOND & 0x1;
			pt6393_display_num(12,CURRENT_HOUR/10);
			pt6393_display_num(16,CURRENT_HOUR%10);
			if((STANDBY_CONFIG2 >> 5) & 0x1)
			{
				pt6393_ledcolon_status(LED_DISPLAY_STA);
			}
			pt6393_display_num(24,CURRENT_MINUTE/10);
			pt6393_display_num(28,CURRENT_MINUTE%10);
		}
	}

    if ((CURRENT_SECOND >= WAKE_UP_SECOND)&&(CURRENT_HOUR == WAKE_UP_HOUR) && (CURRENT_MINUTE == WAKE_UP_MINUTE) && (WAKE_UP_DAY == PASS_DAY))
    {
        if (AUTO_WAKE_UP)
        {
           WAKEUP_FPAG = 3;
	    	wake_up();
        }
    }

}

/*this is for sym6 cec */
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

	//for 27M xtal
	wr_port(ADDR_CEC_BASE+0x12, 0x0d);
	wr_port(ADDR_CEC_BASE+0x13, 0x01);

#if 0
	//for 24M xtal
	wr_port(ADDR_CEC_BASE+0x12, 0xf0);
	wr_port(ADDR_CEC_BASE+0x13, 0x00);
#endif
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

		msdelay(5);
		cec_data_cnt++;
		if(cec_data_cnt>254)
		{
			break;
		}

	}
}
/*end for sym6 cec*/
void sys_init( void );
void main(void ) 
{
	U8 u8KeyValue;
	U8  cnt = 0 ;
	U8 ir_int = 0;

	parse_data();
	sys_init();
	if(CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	LED_DISPLAY_STA = 2;
	standby(); 
	pt6393_init();
	rd_port(IR_INT_CFG,ir_int);
	ir_int&=~0xf;//only keep ir wave filter interrupt
  	wr_port(IR_INT_CFG,ir_int);

	while(1)
	{
		cnt++;
		if(CEC_ENABLE)
		{
			sym4_cec_read();
		}

		if(IR_INT_FLAG)
		{
			WAKEUP_FPAG = 1;
			wake_up();
		}

		if(cnt == 10)  // polling status
		{
			u8KeyValue = pt6393_get_key();
			if (u8KeyValue == WAKE_UP_KEY)
			{
				WAKEUP_FPAG = 2;
				wake_up();
			}
			clock_advance_pt6393();
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
  U8 val;
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


 
