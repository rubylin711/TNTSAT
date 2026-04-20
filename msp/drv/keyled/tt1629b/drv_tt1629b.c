/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <mt_unf_keyled.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include "../drv_keyled_priv.h"
#include "drv_tt1629b.h"
#include "mt_unf_gpio.h"
#include "mt_kernel_adapt.h"
#include "drv_gpio_ioctl.h"
#include "drv_sys_misc.h"


/*************************************************************************
*
*								TM1629B support
*
**************************************************************************
*				pos define
*	There are 8 grids for this TT1629B frontpanel. Every grid has 14 segs.
*	eg:
*	1.	grid2 for colon led, grid 8 for first grid, grid7 for second grid, grid6 for third grid,
*		grid5 for fourth grid.
*		a-seg3, b-seg1, c-seg2, d-seg9, e-seg4, f-seg5, g1-seg10, g2-seg11, H-seg12,
*		J-seg6
*	for auto add address mode, if light number 8, will light a-b-c-d-e-f, the data are 1111,
*	1000,1110,0000,
*	because of lsb first, the data are 0x1f at address 0x00, data 0x07 at address 0x01,
*	other 14 bytes data are 0x00.
*
*		 		      a
*				 _______
*			f	|g1   g2 |b      .D1(H)
*				 ___ ___
*			e	|		 |c      .D2(J)
*				 _______
*				       d
*
*************************************************************************/


typedef struct
{
 /*!
   ascii character to display
   */
 MT_U8 ch;
 /*!
   bitmap
   */
 MT_U16 bitmap;
}led_bitmap_t;


 typedef struct
{
     MT_U16 dis_buff[FP_MAX_LED_NUM] ;
     MT_BOOL colon_enable;
     MT_BOOL spot_enable;
}dis_led;

static dis_led display_content ={0};
static MT_U8 display_bright_level = 0;

//static void display_refresh(long unsigned int);
static void display_refresh(struct timer_list *t);
//static DEFINE_TIMER(led_display_timer, display_refresh, 0, 0);
static DEFINE_TIMER(led_display_timer, display_refresh);

static MT_U8 led_open_cfg[4]={0};


#if ENABLE_TT1629
static led_bitmap_t tm1629b_fp_bitmap[] =
{
  {'.', BIT_DP},
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
  {'a', BIT_A+BIT_B+BIT_C+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'A', BIT_A+BIT_B+BIT_C+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'b', BIT_C+BIT_D+BIT_E+BIT_F+BIT_G1+BIT_G2},
  //{'B', BIT_C+BIT_D+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'B', BIT_A+BIT_B+BIT_C+BIT_D+BIT_J+BIT_M+BIT_G2},
  {'c', BIT_A+BIT_D+BIT_E+BIT_F},
  {'C', BIT_A+BIT_D+BIT_E+BIT_F},
  {'d', BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},
  {'D', BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},
  {'e', BIT_A+BIT_B+BIT_D+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'E', BIT_A+BIT_D+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'f', BIT_A+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'F', BIT_A+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'g', BIT_A+BIT_C+BIT_D+BIT_E+BIT_F+BIT_G2},
  {'G', BIT_A+BIT_C+BIT_D+BIT_E+BIT_F+BIT_G2},
  {'h', BIT_B+BIT_C+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'H', BIT_B+BIT_C+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'i', BIT_E+BIT_F},
  {'I', BIT_E+BIT_F},
  {'j', BIT_B+BIT_C+BIT_D},
  {'J', BIT_B+BIT_C+BIT_D},
  {'k', BIT_E+BIT_F+BIT_K+BIT_L+BIT_G1+BIT_G2},
  {'K', BIT_E+BIT_F+BIT_K+BIT_L+BIT_G1+BIT_G2},
  {'m', BIT_B+BIT_C+BIT_E+BIT_F+BIT_H+BIT_K},
  {'M', BIT_B+BIT_C+BIT_E+BIT_F+BIT_H+BIT_K},
  {'l',  BIT_D+BIT_E+BIT_F},
  {'L', BIT_D+BIT_E+BIT_F},
  {'n', BIT_B+BIT_C+BIT_E+BIT_F+BIT_L+BIT_H},
  {'N', BIT_B+BIT_C+BIT_E+BIT_F+BIT_L+BIT_H},
  {'o', BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},
  {'O', BIT_A+BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},
  {'p', BIT_A+BIT_B+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'P', BIT_A+BIT_B+BIT_E+BIT_F+BIT_G1+BIT_G2},
  {'q', BIT_A+BIT_B+BIT_C+BIT_F+BIT_G1+BIT_G2},
  {'Q', BIT_A+BIT_B+BIT_C+BIT_F+BIT_G1+BIT_G2},
  {'r', BIT_A+BIT_B+BIT_E+BIT_F+BIT_L+BIT_G1+BIT_G2},
  {'R', BIT_A+BIT_B+BIT_E+BIT_F+BIT_L+BIT_G1+BIT_G2},
  {'s', BIT_A+BIT_C+BIT_D+BIT_F+BIT_G1+BIT_G2},
  {'S', BIT_A+BIT_C+BIT_D+BIT_F+BIT_G1+BIT_G2},
  {'t', BIT_A+BIT_J+BIT_M},
  {'T', BIT_A+BIT_J+BIT_M},
  {'u', BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},
  {'U', BIT_B+BIT_C+BIT_D+BIT_E+BIT_F},
  {'w', BIT_B+BIT_C+BIT_E+BIT_F+BIT_N+BIT_L},
  {'W', BIT_B+BIT_C+BIT_E+BIT_F+BIT_N+BIT_L},
  {'x', BIT_N+BIT_L+BIT_H+BIT_K},
  {'X', BIT_N+BIT_L+BIT_H+BIT_K},
  {'y', BIT_H+BIT_K+BIT_M},
  {'Y', BIT_H+BIT_K+BIT_M},
  {'z', BIT_A+BIT_K+BIT_D+BIT_N},
  {'Z', BIT_A+BIT_K+BIT_D+BIT_N},

  {':', BIT_D1_D2},  {'-', BIT_G1+BIT_G2},
  {'_', BIT_B},  {' ', 0x00}
};
#endif

#if ENABLE_TT1629B
static led_bitmap_t tm1629b_fp_bitmap[] =
{
  {'.', bit_h},
  {'0', bit_a+bit_b+bit_c+bit_d+bit_e+bit_f},  {'1', bit_b+bit_c},  {'2', bit_a+bit_b+bit_d+bit_e+bit_g1+bit_g2},
  {'3', bit_a+bit_b+bit_c+bit_d+bit_g1+bit_g2},{'4', bit_b+bit_c+bit_f+bit_g1+bit_g2},
  {'5', bit_a+bit_c+bit_d+bit_f+bit_g1+bit_g2}, {'6', bit_a+bit_c+bit_d+bit_e+bit_f+bit_g1+bit_g2},
  {'7', bit_a+bit_b+bit_c},{'8', bit_a+bit_b+bit_c+bit_d+bit_e+bit_f+bit_g1+bit_g2},
  {'9', bit_a+bit_b+bit_c+bit_d+bit_f+bit_g1+bit_g2},  {'a', bit_a+bit_b+bit_c+bit_e+bit_f+bit_g1+bit_g2},
  {'A', bit_a+bit_b+bit_c+bit_e+bit_f+bit_g1+bit_g2},{'b', bit_c+bit_d+bit_e+bit_f+bit_g1+bit_g2},
  {'B', bit_c+bit_d+bit_e+bit_f+bit_g1+bit_g2},  {'c', bit_a+bit_d+bit_e+bit_f},  {'C', bit_a+bit_d+bit_e+bit_f},
  {'d', bit_b+bit_c+bit_d+bit_e+bit_g1+bit_g2},  {'D', bit_b+bit_c+bit_d+bit_e+bit_g1+bit_g2},
  {'e', bit_a+bit_d+bit_e+bit_f+bit_g1+bit_g2},  {'E', bit_a+bit_d+bit_e+bit_f+bit_g1+bit_g2},
  {'f', bit_a+bit_e+bit_f+bit_g1+bit_g2},  {'F', bit_a+bit_e+bit_f+bit_g1+bit_g2},
  {'g', bit_a+bit_b+bit_c+bit_d+bit_f+bit_g1+bit_g2},  {'G', bit_a+bit_b+bit_c+bit_d+bit_f+bit_g1+bit_g2},
  {'h', bit_b+bit_c+bit_e+bit_f+bit_g1+bit_g2},  {'H', bit_b+bit_c+bit_e+bit_f+bit_g1+bit_g2},  {'i', bit_e+bit_f},
  {'I', bit_e+bit_f},{'j', bit_a+bit_b+bit_c+bit_d},  {'J', bit_a+bit_b+bit_c+bit_d},
  {'k', bit_b+bit_d+bit_e+bit_f+bit_g1+bit_g2},  {'K', bit_b+bit_d+bit_e+bit_f+bit_g1+bit_g2},{'l', bit_d+bit_e+bit_f},
  {'L', bit_d+bit_e+bit_f},{'n', bit_a+bit_b+bit_c+bit_e+bit_f},  {'N', bit_a+bit_b+bit_c+bit_e+bit_f},
  {'o', bit_a+bit_b+bit_c+bit_d+bit_e+bit_f},  {'O', bit_a+bit_b+bit_c+bit_d+bit_e+bit_f},
  {'p', bit_a+bit_b+bit_e+bit_f+bit_g1+bit_g2},  {'P', bit_a+bit_b+bit_e+bit_f+bit_g1+bit_g2},
  {'q', bit_a+bit_b+bit_c+bit_f+bit_g1+bit_g2},  {'Q', bit_a+bit_b+bit_c+bit_f+bit_g1+bit_g2},{'r', bit_e+bit_g1+bit_g2},
  {'R', bit_e+bit_g1+bit_g2},  {'s', bit_a+bit_c+bit_d+bit_f+bit_g1+bit_g2},  {'S', bit_a+bit_c+bit_d+bit_f+bit_g1+bit_g2},
  {'t', bit_d+bit_e+bit_f+bit_g1+bit_g2},  {'T', bit_d+bit_e+bit_f+bit_g1+bit_g2},  {'u', bit_b+bit_c+bit_d+bit_e+bit_f},
  {'U', bit_b+bit_c+bit_d+bit_e+bit_f},{'y', bit_b+bit_c+bit_d+bit_f+bit_g1+bit_g2},
  {'Y', bit_b+bit_c+bit_d+bit_f+bit_g1+bit_g2},  {'z', bit_a+bit_b+bit_d+bit_e+bit_g1+bit_g2},
  {'Z', bit_a+bit_b+bit_d+bit_e+bit_g1+bit_g2},{':', bit_h+bit_j},  {'-', bit_g1+bit_g2},
  {'_', bit_d},  {' ', 0x00}
};
#endif
#define FP_TM1629B_TABLE_SIZE sizeof(tm1629b_fp_bitmap) / sizeof(led_bitmap_t)

MT_DECLARE_MUTEX(g_Tt1629bMutex);

static inline MT_VOID SET_STB(MT_VOID)
{
	drv_gpio_set_value(TM1629B_STB, GPIO_VALUE_HIGH_LEVEL);
}

static inline MT_VOID SET_CLK(MT_VOID)
{
	drv_gpio_set_value(TM1629B_CLK, GPIO_VALUE_HIGH_LEVEL);
}

static inline MT_VOID SET_DIO(MT_VOID)
{
	drv_gpio_set_value(TM1629B_DAT, GPIO_VALUE_HIGH_LEVEL);
}

static inline MT_BOOL IS_DIO_HIGH(MT_VOID)
{
	gpio_value_e val = 0;
	drv_gpio_get_value(TM1629B_DAT, &val);
	//printk("##%s, val=[%d]##\n",__FUNCTION__,val);
	return (val == GPIO_VALUE_HIGH_LEVEL) ? MT_TRUE : MT_FALSE;
}


static inline MT_VOID CLR_STB(MT_VOID)
{
	drv_gpio_set_value(TM1629B_STB, GPIO_VALUE_LOW_LEVEL);
}

static inline MT_VOID CLR_CLK(MT_VOID)
{
	drv_gpio_set_value(TM1629B_CLK, GPIO_VALUE_LOW_LEVEL);
}

static inline MT_VOID CLR_DIO(MT_VOID)
{
	drv_gpio_set_value(TM1629B_DAT, GPIO_VALUE_LOW_LEVEL);
}

MT_VOID set_dio_to_input(MT_VOID)
{
	drv_gpio_set_dir(TM1629B_DAT, GPIO_DIR_INPUT);
}

MT_VOID set_dio_to_output(MT_VOID)
{
	drv_gpio_set_dir(TM1629B_DAT, GPIO_DIR_OUTPUT);
}

static void write_data(MT_U8 data)
{
	MT_U8 i;

	for(i=0;i<8;i++)
	{
		if((data >> i)&0x01)
		{
			SET_DIO();
		}
		else
		{
			CLR_DIO();
		}
		udelay(2);
		CLR_CLK();
		udelay(2);
		SET_CLK();
		udelay(2);
	}
	udelay(2);

}

static void write_cmd(MT_U8 cmd)
{
	SET_STB();
	CLR_STB();
	write_data(cmd);
}

MT_VOID display_auto_addr(MT_U8 wdata)
{
	MT_U8 i;

	SET_DIO();
	SET_CLK();
	SET_STB();//all initinal 1 before transfer
	write_data(AUTO_ADDRESS_MODE);//auto address mode, address auto add 1
	SET_STB();
	write_data(START_ADDRESS);
	for(i=0;i < GRID_DATA_CNT;i++)
	{
		write_data(0x00);       //flush fp firstly
	}
}

void display_fixed_addr(u16 wdata, MT_U8 addr)//MT_VOID
{
	SET_DIO();
	SET_CLK();
	SET_STB();//all initinal 1 before transfer
	write_cmd(FIXED_ADDRESS_MODE);//fixed address mode
	SET_STB();
	write_cmd(addr);
	write_data(wdata&0xff);
	//SET_DIO();
	//SET_CLK();
	SET_STB();//all initinal 1 before transfer
	write_cmd(addr+1);
	write_data(wdata>>8);
	//printk("##display_fixed_addr, data=[0x%x]##\n",wdata);
}

//static void display_refresh(long unsigned int val)
static void display_refresh(struct timer_list *t)
{
	MT_U16 addr = 0x00;
	MT_U16 data = 0x00;
	MT_U8 led_type = 0;

	if(t != &led_display_timer)
	{
		printk("\n###[%s.%d] Err: timer is unknown!\n",__FUNCTION__,__LINE__);
		return;
	}

	display_fixed_addr(display_content.dis_buff[0], FIRST_GRID_ADDR);
	display_fixed_addr(display_content.dis_buff[1], SECOND_GRID_ADDR);
	display_fixed_addr(display_content.dis_buff[2], THIRD_GRID_ADDR);
	display_fixed_addr(display_content.dis_buff[3], FOURTH_GRID_ADDR);

	SET_STB();
	for(led_type=LBD_TYPE_POWER;led_type<LBD_TYPE_SPOT+1;led_type++)
	{
		switch(led_type)
		{
			case LBD_TYPE_POWER:
				addr = RED_LED_ADDR;
				data = (led_open_cfg[led_type] == 1) ? 0xffff : 0x00;
				SET_STB();
				display_fixed_addr(data, addr);
				break;
			case LBD_TYPE_LOCK:
				addr = GREEN_LED_ADDR;
				data = (led_open_cfg[led_type] == 1) ? 0xffff : 0x00;
				SET_STB();
				display_fixed_addr(data, addr);
				break;
			default:
				break;
		}

	}
	//printk("##display_zhao.jing, data=[0x%x]##\n",jiffies + HZ);
	mod_timer(&led_display_timer, jiffies + HZ);
}

static MT_VOID setup_tm1629b(MT_VOID)
{
	MT_U8 i;
	MT_U8 gpio_pin[3] = {MT_UNF_AO_GPIO_5,MT_UNF_AO_GPIO_6,MT_UNF_AO_GPIO_7};

#if 0
	//pinmux move to uboot cmd config or dtsi
	symphony_setpinmux(AO_PIN0, 0, 12, 0x111);//AO_GPIO_7,6,5
#endif
	printk("\n##%s.%d Todo(pinmux move to uboot cmd config or dtsi).###\n",__FUNCTION__,__LINE__);
	for(i = 0;i < 3;i++)
	{
		drv_gpio_io_enable(gpio_pin[i], GPIO_MASK_ENABLE);
		drv_gpio_set_dir(gpio_pin[i], GPIO_DIR_OUTPUT);
	}

	SET_DIO();
	SET_CLK();
	SET_STB();

	write_data(AUTO_ADDRESS_MODE);//auto address mode, address auto add 1
	SET_STB();
	write_data(START_ADDRESS);
	for(i=0;i < GRID_DATA_CNT;i++)
	{
		write_data(0x00);       //flush fp firstly
	}
	write_data(0x8c);   //
	mod_timer(&led_display_timer, jiffies + HZ);
	printk("##%s, line[%d]###\n",__FUNCTION__,__LINE__);


}

static MT_U8 tt1629b_Read( MT_VOID )		//∂¡»°∞¥º¸
{
	MT_U8 i,j,k, key_data = 0;
	MT_U8 key_val[6];

	write_cmd(READ_DATA_MODE);
	set_dio_to_input();
	// udelay(1);
	SET_DIO();
	// udelay(1);

	/*
	must read 4 bytes data, byte1's bit3 high for key1,
	byte1's bit7 high for key2, byte2's bit3 high for key3, depends hw design
	*/
	for(j = 0;j < 4;j++)
	{
		for(i = 0;i < 8;i++)
		{
			key_val[j]>>=1;
			//  udelay(1);
			CLR_CLK();
			udelay(1);
			SET_CLK();
			udelay(1);
			if(IS_DIO_HIGH())
			{
				//key_val[j] |= 0x80;
				memset(key_val, 0x00, sizeof(key_val));
				key_val[j] |= 0x1<<i;
				if (key_val[1] != 0x00 && key_val[1] != 0xff)
				{
					key_val[1] += 0x01;//fake key value for diff different key,because byte1 not the same
				}

				for(k = 0;i < 4;k++)
				{
					if(key_val[k] != 0 && key_val[k] != 0xff)
					{
						key_data = key_val[k];
						// printk("##%s, key_data=[0x%x]###\n",__FUNCTION__,key_data);
					}
				}
				break;
			}
		}
	}

	SET_STB();
	// udelay(1);
	set_dio_to_output();
	// udelay(1);
	SET_DIO();
	// mdelay(1);
	//up(&g_Tt1629bMutex);
	return key_data;
}

static MT_VOID tt1629b_init(void *dev)
{
	setup_tm1629b();
    if (klinfo.kl_ops.keyled_init_kadc) {
        klinfo.kl_ops.keyled_init_kadc(dev);
    }
}
static MT_VOID tt1629b_deinit(MT_VOID)
{
    if (klinfo.kl_ops.release_kadc) {
        klinfo.kl_ops.release_kadc();
    }
}
static MT_VOID tt1629b_display_asc(ulong param)
{
	keyled_display_char_s *p_ledbuf = (void *)param;
	MT_U8 i = 0, j = 0;
	MT_U8 tmp[FP_MAX_LED_NUM] = {0};

	if (NULL == p_ledbuf)
		return;

	memset(tmp, 0x00, sizeof(tmp)); //space ascii value == 0x00
	for(i = 0; i < FP_MAX_LED_NUM; i++)
	{
		tmp[j++] = p_ledbuf->ch[i];
	}

	for (i = 0; i < FP_MAX_LED_NUM; i++)
	{
		for(j = 0; j < FP_TM1629B_TABLE_SIZE; j++)
		{
			if(tm1629b_fp_bitmap[j].ch == tmp[i])
			{
				display_content.dis_buff[i] = tm1629b_fp_bitmap[j].bitmap;
			}
		}
	}

#if ENABLE_TT1629
	if(display_content.colon_enable)
	{
		display_content.dis_buff[1] += BIT_D1_D2;
	}
	if(display_content.spot_enable)
	{
		display_content.dis_buff[2] += BIT_DP;
	}
#endif

	display_fixed_addr(display_content.dis_buff[0], FIRST_GRID_ADDR);
	display_fixed_addr(display_content.dis_buff[1], SECOND_GRID_ADDR);
	display_fixed_addr(display_content.dis_buff[2], THIRD_GRID_ADDR);
	display_fixed_addr(display_content.dis_buff[3], FOURTH_GRID_ADDR);
	SET_STB();
	if(display_bright_level)
	{
		write_cmd(LEVEL1_BRIGHT+(display_bright_level-1));
	}
	else
	{
		write_cmd(LEVEL4_BRIGHT);                   //œ‘ æøÿ÷∆√¸¡Ó
	}
	SET_STB();
}


static MT_VOID tt1629b_display_lbd(ulong param)
{
	keyled_display_lbd_s *p_lbdbuf = (void *)param;
	MT_U16 addr = 0x00;
	MT_U16 data = 0x00;
	MT_BOOL is_enable = 0;
	if (NULL == p_lbdbuf)
		return;

	is_enable = p_lbdbuf->enable;
	led_open_cfg[p_lbdbuf->type]=is_enable;
	switch(p_lbdbuf->type)
	{
		case LBD_TYPE_POWER:
			addr = RED_LED_ADDR;
			data = (is_enable == 1) ? 0xffff : 0x00;
			break;

		case LBD_TYPE_LOCK:
			addr = GREEN_LED_ADDR;
			data = (is_enable == 1) ? 0xffff : 0x00;
			break;

		#if ENABLE_TT1629
		case LBD_TYPE_COLON:
			display_content.colon_enable= is_enable;
			addr = COLON_ADDR;
			data = (is_enable == 1) ? (display_content.dis_buff[1]+BIT_D1_D2):display_content.dis_buff[1] ;
			break;

		case LBD_TYPE_SPOT:
			display_content.spot_enable= is_enable;
			addr = SPOT_ADDR;
			data = (is_enable == 1)?(display_content.dis_buff[2]+BIT_DP):display_content.dis_buff[2];
			break;
		#endif

		#if ENABLE_TT1629B
		case LBD_TYPE_COLON:
                   addr = COLON_ADDR;
                   data = (is_enable == 1) ? (bit_h+bit_j) : 0x00;
                   break;
		#endif

		default:
			printk("##%s, type error!!!##\n",__func__);
			return ;
			break;
	}

	display_fixed_addr(data, addr);
	SET_STB();

       if(display_bright_level)
       {
            write_cmd(LEVEL1_BRIGHT+(display_bright_level-1));
       }
       else
       {
            write_cmd(LEVEL4_BRIGHT);                   //œ‘ æøÿ÷∆√¸¡Ó
       }
    SET_STB();
}

static MT_U32 tt1629b_get_keyval(MT_VOID)
{
	return tt1629b_Read();
}


static MT_VOID tt1629b_set_bright_level(keyled_display_led_bright_t bright)
{
      if(bright <= LED_LEVEL8_BRIGHT)
      {
            display_bright_level = bright;
      }
      else if(bright == LED_DISPLAY_CLOSE)
      {
            SET_STB();
	      write_cmd(0x80);//close tt1629 led display
            SET_STB();
      }
      else if(bright == LED_DISPLAY_OPEN)
      {
            SET_STB();
            write_cmd(0x88); //open tt1629 led display
            SET_STB();
      }

}

MT_VOID keyled_tt1629b_attach(keyled_info_s *klinfo, int kadc)
{
	if (klinfo)
	{
		#ifdef CFG_MT_KEYLED_ADC_SUPPORT
		if (kadc) {
			keyled_keyadc_attach(klinfo);
			klinfo->kl_ops.keyled_init_kadc = klinfo->kl_ops.keyled_init;
			klinfo->kl_ops.release_kadc = klinfo->kl_ops.release;
		}
		#endif
		klinfo->type |= MT_UNF_KEYLED_TYPE_TT1629B;

		klinfo->kl_ops.keyled_init = tt1629b_init;
		klinfo->kl_ops.display_asc = tt1629b_display_asc;
		klinfo->kl_ops.display_lbd = tt1629b_display_lbd;
		if (0 == kadc) {
			klinfo->kl_ops.get_keyval = tt1629b_get_keyval;
		}
		klinfo->kl_ops.set_brightness = tt1629b_set_bright_level;
		klinfo->kl_ops.release = tt1629b_deinit;
		klinfo->idle_keyval = 0;
		klinfo->is_attach = 1;
	}
}


