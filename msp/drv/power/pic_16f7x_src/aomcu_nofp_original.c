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

#define  CLK_GATE_CFG0_REG_ADDR   0x0010
#define  CLK_GATE_CFG1_REG_ADDR   0x0014
#define  CLK_GATE_CFG2_REG_ADDR   0x0018
#define  CLK_GATE_CFG3_REG_ADDR   0x001c
#define  LPM_GLB_CTRL 0x001C

#define IR_INT_RAWSTA	0x1014
#define IR_INT_CFG		0x1010


U16 tick = 0;
U16 ctick = 0;
U32 curr_tick = 0;
U8 CURRENT_HOUR,CURRENT_MINUTE;
U8 WAKE_UP_HOUR,WAKE_UP_MINUTE;
U8 AUTO_WAKE_UP;
U8 STANDBY_CONFIG;
U8 CURRENT_SECOND = 0;
U8 WAKE_UP_SECOND = 0;
U8 PASS_DAY = 0;
U8 GPEN_VAL = 0;			//1-->no define NORMAL_MEM(need repair);0-->define NORMAL_MEM(no need repair)
U8 WAKE_UP_DAY = 0;

U8 IR_INT_FLAG  = 0;
U8 WAKEUP_FPAG = 0;
U8 WAKE_UP_KEY = 0;
U8 ADC_KEY_INT_FLAG = 0;
U8 ADC_KEY_TYPE = 0;//uio_kadc_type_t
U8 OSC_FLAG = 0;
U8 STANDBY_PARAM = 0;//0 means symphony2 a0;1 means symphony2 a1

U8 CEC_ENABLE = 0;

U8 unix_time_sec = 0;
U8 unix_time_sec1 = 0;
U8 unix_time_sec2 = 0;
U8 unix_time_sec3 = 0;
bit unix_time_flag = 0;

U8 ir_state = 0;

U8 core_vcode = 0;
/*!
 * RTC enable register
 */
#define RTC_EN            0xC400
/*!
 * RTC param register
 */
#define RTC_PARAMETER_H   0xC404
/*!
 * RTC param register
 */
#define RTC_PARAMETER_L   0xC408
/*!
 * RTC status register
 */
#define RTC_STA           0xC40C


#define PCB_CORE_ON

void wake_up();


void rtc_config_1s(void)
{
	U8 tmp = 0;
	U8 rtc_state = 0;

	rd_port(RTC_STA,rtc_state);

	/* disable it first*/
	wr_port(RTC_EN, 0x0);

#if 0
	/*set rtc to 20250000*1 = 1second */
	wr_port(RTC_PARAMETER_H,    0);
	wr_port(RTC_PARAMETER_H+1,  0);
	wr_port(RTC_PARAMETER_H+2,  0);
	wr_port(RTC_PARAMETER_H+3,  0);

      //0x134FD90
	wr_port(RTC_PARAMETER_L,    0x90);
	wr_port(RTC_PARAMETER_L+1,  0xfd);
	wr_port(RTC_PARAMETER_L+2,  0x34);
	wr_port(RTC_PARAMETER_L+3,  0x01);
#else
	//0x000f4240,1M clk
	wr_port(RTC_PARAMETER_H,    0);
	wr_port(RTC_PARAMETER_H+1,  0);
	wr_port(RTC_PARAMETER_H+2,  0);
	wr_port(RTC_PARAMETER_H+3,  0);

	//0x000f4240,1M clk
	wr_port(RTC_PARAMETER_L,    0x40);
	wr_port(RTC_PARAMETER_L+1,  0x42);
	wr_port(RTC_PARAMETER_L+2,  0x0f);
	wr_port(RTC_PARAMETER_L+3,  0x00);
#endif

	/* enable it */
	wr_port(RTC_EN, 0x1);
}

/*************************/

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

void msdelay(U16 ms)
{
	ctick = 0;
	while(((ctick * 24) /10) < ms);
}

void mcu_aogpio_get_val(U8 aogpio, U8 *p_val)
{
	U8 temp = 0;

	rd_port(AO_GPIO_RDATA, temp);
	temp = (temp >> aogpio) & 0x01;

	*p_val = temp;
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
	//LED_POS                  = AP_DATA[10];
	// STANDBY_CONFIG2  = AP_DATA[11];
	STANDBY_PARAM	= AP_DATA[16] & 0x1;
	ADC_KEY_TYPE	= (AP_DATA[16]>>1)&0x03;//valid value 0~3

	GPEN_VAL		= STANDBY_CONFIG & 0x1;
	//LED_DISPLAY_TIME 	=  (STANDBY_CONFIG>>6)&0x1;
	//LED_DISPLAY_CHAR 	=  (STANDBY_CONFIG>>5)&0x1;
	AUTO_WAKE_UP	= (STANDBY_CONFIG>>7)&0x1;
	OSC_FLAG 		= (AP_DATA[16]>>3)&0x01;

    CEC_ENABLE = AP_DATA[15] & 0x01;
	unix_time_flag = (AP_DATA[16]>>5)&0x01;

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
#if 0
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
#endif
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
void wake_up()
{
	U8 tmp = 0;
	U8 tmp1 = 0;
	U8 tmp2 = 0;
	U8 tmp3 = 0;
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

void clock_advance()
{
    if ((CURRENT_SECOND >= WAKE_UP_SECOND)&&(CURRENT_HOUR >= WAKE_UP_HOUR) && (CURRENT_MINUTE >= WAKE_UP_MINUTE) && (PASS_DAY >= WAKE_UP_DAY))
    {
		if (AUTO_WAKE_UP)
		{
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
/*
	rd_port(AO_GPIO_WDATA, val);
	val &= ~(1<<KADC_AOGPIO);
	wr_port(AO_GPIO_WDATA, val);//set to output low level
*/
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

#define AOMCU_KADC_REPEAT_TICKS (30)//3 //about 300 ms
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
	U8 val = 0;
	U8 cnt = 0;
	parse_data();
	sys_init();

	if(OSC_FLAG)
	{
		rtc_config_1s();
	}

	if (CEC_ENABLE)
	{
		sym4_cec_setup();
	}
	standby();

	if (3 != ADC_KEY_TYPE)
	{
		mcu_key_adc_setup();
	}

	// aomcu_delay(200);

	ADC_KEY_INT_FLAG = 0;
	rd_port(LPM_MESSAGE2AO_0 + 6, CURRENT_SECOND);
	rd_port(LPM_MESSAGE2AO_0 + 5, CURRENT_MINUTE);
	rd_port(LPM_MESSAGE2AO_0 + 4, CURRENT_HOUR);
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

	while(1)
	{
		if (CEC_ENABLE)
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
			clock_advance();
			cnt = 0;
		}
		else
		{
			asm("NOP");
		}
		cnt++;
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
	if(OSC_FLAG)
	{
		INTE_B1 = 1;//rtc
	}

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

	if(T0IF)
	{
		tick++;
		ctick++;
		curr_tick++;

		if(ctick == 0xffff)
			ctick = 0;

		if (!OSC_FLAG)
		{
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

					tick += 701;
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
				tick -= 415;//tick = 0;
			}
		}

		TMR0 =0;
		T0IF = 0; //clear interupt
	}

	EXPORT_ADDRESS_L = tmp_export_address_l;
	EXPORT_ADDRESS_H = tmp_export_address_h;
	EXPORT_RDATA = tmp_export_rd;
}

