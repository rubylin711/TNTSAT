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
#include "../drv_keyled_priv.h"
#include "drv_fd650.h"

#include "drv_i2c_ioctl.h"

static led_bitmap_t fd650_fp_bitmap[] =
{
	{'.', 0x80},
	{'0', 0x3F},  {'1', 0x06},  {'2', 0x5B},  {'3', 0x4F},
	{'4', 0x66},  {'5', 0x6D},  {'6', 0x7D},  {'7', 0x07},
	{'8', 0x7F},  {'9', 0x6F},  {'a', 0x77},  {'A', 0x77},
	{'b', 0x7C},  {'B', 0x7C},  {'c', 0x39},  {'C', 0x39},
	{'d', 0x5E},  {'D', 0x5E},  {'e', 0x79},  {'E', 0x79},
	{'f', 0x71},  {'F', 0x71},  {'g', 0x6F},  {'G', 0x3D},
	{'h', 0x76},  {'H', 0x76},  {'i', 0x04},  {'I', 0x30},
	{'j', 0x0E},  {'J', 0x0E},  {'l', 0x38},  {'L', 0x38},
	{'n', 0x54},  {'N', 0x37},  {'o', 0x5C},  {'O', 0x3F},
	{'p', 0x73},  {'P', 0x73},  {'q', 0x67},  {'Q', 0x67},
	{'r', 0x50},  {'R', 0x77},  {'s', 0x6D},  {'S', 0x6D},
	{'t', 0x78},  {'T', 0x31},  {'u', 0x3E},  {'U', 0x3E},
	{'y', 0x6E},  {'Y', 0x6E},  {'z', 0x5B},  {'Z', 0x5B},
	{':', 0x80},  {'-', 0x40},  {'_', 0x08},  {' ', 0x00},
};
#define FP_FD650_TABLE_SIZE sizeof(fd650_fp_bitmap) / sizeof(led_bitmap_t)

typedef struct
{
     MT_U8 dis_buff[FP_MAX_LED_NUM];
     MT_U8 lbd_enable;//bit3-bit0 indicate buf[3]-buf[0], 1:should display lbd, 0:not display
     MT_U8 lbd_seg_pos[FP_MAX_LED_NUM];//which seg to display lbd
}FD650_DIS_LED;

static FD650_DIS_LED fd650_dis_content;
static MT_U8 fd650_pos[4] = {0, 1, 2, 3};//default led pos, set it in fd650_set_led_pos
static MT_U8 fd650_map[8] = {0, 1, 2, 3, 4, 5, 6, 7};//default seg map, set it in fd650_set_led_map
static MT_U16 fd650_cmd[4] = {FD650_DIG0, FD650_DIG1, FD650_DIG2, FD650_DIG3};//the grid cmd

static MT_U16 fd650_display_bright_level = 0;
extern keyled_info_s klinfo;

static void fd650_char_init(void)
{
	MT_U8 u8temp, i, j;
	for (i=0; i<FP_FD650_TABLE_SIZE; i++)
	{
		u8temp = 0;
		for (j=0; j<8; j++)
		{
			u8temp = u8temp | ((fd650_fp_bitmap[i].ch & 0x1) << fd650_map[j]);
			fd650_fp_bitmap[i].ch = fd650_fp_bitmap[i].ch >> 1;
		}
		fd650_fp_bitmap[i].ch = u8temp;
	}
}

static void FD650_Write(u16 cmd)
{
	i2c_data_t i2c_data = {0};
	u8 buff[4] = {0};

	i2c_data.i2c_id = 2;
	buff[0] = ((u8)(cmd >> 7) & 0x3E) | 0x40;
	buff[1] = (u8)cmd;
	i2c_data.p_data = (mt_u64)buff;
    i2c_data.data_len = 2;
	i2c_write_fp(i2c_data);
}

static u8 FD650_Read( void )
{
	u8 keycode = 0;
	int ret = -1;
	i2c_data_t i2c_data = {0};
	u8 buff[4] = {0};

	i2c_data.i2c_id = 2;
	buff[0] = ((FD650_GET_KEY >> 7) & 0x3E) | 0x01 | 0x40;
	i2c_data.p_data = (mt_u64)buff;
    i2c_data.data_len = 1;
	ret = i2c_read_fp(i2c_data);

	keycode = buff[1];

	if(((keycode & 0x00000040) == 0) || (ret != 0))
	{
	    keycode = 0;
	}

	return keycode;
}

static void fd650_init(void *dev)
{
    i2c_fp_enable();
    FD650_Write(FD650_SYSON_4 | FD650_8SEG_ON);// set a default Brightness
    if (klinfo.kl_ops.keyled_init_kadc) {
        klinfo.kl_ops.keyled_init_kadc(dev);
    }
}
static void fd650_deinit(void)
{
    if (klinfo.kl_ops.release_kadc) {
        klinfo.kl_ops.release_kadc();
    }
}
static MT_VOID fd650_set_bright_level(keyled_display_led_bright_t bright)
{
	switch(bright)
	{
		case LED_LEVEL1_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_1;
			break;

		case LED_LEVEL2_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_2;
			break;

		case LED_LEVEL3_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_3;
			break;

		case LED_LEVEL4_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_4;
			break;

		case LED_LEVEL5_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_5;
			break;

		case LED_LEVEL6_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_6;
			break;

		case LED_LEVEL7_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_7;
			break;

		case LED_LEVEL8_BRIGHT:
			fd650_display_bright_level = FD650_SYSON_8;
			break;

		case LED_DISPLAY_CLOSE:
			FD650_Write(FD650_SYSOFF);//close led display
			break;

		case LED_DISPLAY_OPEN:
			FD650_Write(FD650_SYSON); //open led display
			break;
		default:
			break;
	}

	// --->bug 133761: set Brightness immediately
	//case LED_DISPLAY_CLOSE,  can't use FD650_Write set brightness any value;
	if(LED_DISPLAY_CLOSE == bright)
	{
		return;
	}

	if(((bright>= LED_LEVEL1_BRIGHT) && (bright <= LED_LEVEL8_BRIGHT)) || ( LED_DISPLAY_OPEN == bright))
	{
		FD650_Write(fd650_display_bright_level | FD650_8SEG_ON);
	}
	else
	{
		//set a default Brightness
		FD650_Write(FD650_SYSON_4 | FD650_8SEG_ON);
	}
}

static void fd650_display(ulong param)
{
	u32 ledval = (u32)param;
	u8 i = 0;
	u8 pos;
#if 0
	if(fd650_display_bright_level)
	{
		FD650_Write(fd650_display_bright_level | FD650_8SEG_ON);
	}
	else
	{
		FD650_Write(FD650_SYSON_4 | FD650_8SEG_ON);
	}
#endif
	for (i=0; i<FP_MAX_LED_NUM; i++)
	{
		fd650_dis_content.dis_buff[i] =  ((ledval >> (8*(3-i))) & 0xff);
		if (fd650_dis_content.lbd_enable & (1<<i))
		{
			fd650_dis_content.dis_buff[i] |= fd650_dis_content.lbd_seg_pos[i];
		}

		pos = fd650_pos[i];
		FD650_Write(fd650_cmd[pos] | fd650_dis_content.dis_buff[i]);
	}
}

static void fd650_display_lbd(ulong param)
{
	keyled_display_lbd_s *p_lbdbuf = (void *)param;
	MT_U8 grid_pos = 0;
#if 0
	if(fd650_display_bright_level)
	{
		FD650_Write(fd650_display_bright_level | FD650_8SEG_ON);
	}
	else
	{
		FD650_Write(FD650_SYSON_4 | FD650_8SEG_ON);
	}
#endif

	if(NULL == p_lbdbuf)
	{
		return;
	}

	grid_pos = p_lbdbuf->grid_pos;
	fd650_dis_content.lbd_seg_pos[grid_pos] = (1<<fd650_map[p_lbdbuf->seg_pos]);
	if (p_lbdbuf->enable)
	{
		fd650_dis_content.lbd_enable |= (1<<grid_pos);
		fd650_dis_content.dis_buff[grid_pos] |= fd650_dis_content.lbd_seg_pos[grid_pos];
	}
	else
	{
		fd650_dis_content.lbd_enable &= (~(1<<grid_pos));
		fd650_dis_content.dis_buff[grid_pos] &= ~(fd650_dis_content.lbd_seg_pos[grid_pos]);
	}

	printk(KERN_ERR "[%s %d]grid_pos=%d, seg_pos=0x%02x, buf=0x%02x\n", __FUNCTION__, __LINE__,
					grid_pos, p_lbdbuf->seg_pos, fd650_dis_content.dis_buff[grid_pos]);

	FD650_Write(fd650_cmd[grid_pos] | fd650_dis_content.dis_buff[grid_pos]);
}

static void fd650_display_asc(ulong param)
{
	keyled_display_char_s *p_ledbuf = (void *)param;
	MT_U8 i = 0, j = 0;
	MT_U8 pos;

	if(NULL == p_ledbuf)
	{
		return;
	}

	for (i=0; i<FP_MAX_LED_NUM; i++)
	{
		for(j = 0; j < FP_FD650_TABLE_SIZE; j++)
		{
			if(fd650_fp_bitmap[j].ch == p_ledbuf->ch[i])
			{
				fd650_dis_content.dis_buff[i] = fd650_fp_bitmap[j].bitmap;
				break;
			}
		}

		if (FP_FD650_TABLE_SIZE == j)
		{
			fd650_dis_content.dis_buff[i] = 0x00;
		}
	}
#if 0
	if(fd650_display_bright_level)
	{
		FD650_Write(fd650_display_bright_level | FD650_8SEG_ON);
	}
	else
	{
		FD650_Write(FD650_SYSON_4 | FD650_8SEG_ON);
	}
#endif

	for (i=0; i<FP_MAX_LED_NUM; i++)
	{
		if (fd650_dis_content.lbd_enable & (1<<i))//set lbd pos
		{
			fd650_dis_content.dis_buff[i] |= fd650_dis_content.lbd_seg_pos[i];
		}

		pos = fd650_pos[i];
		FD650_Write(fd650_cmd[pos] | fd650_dis_content.dis_buff[i]);
	}
}

static void fd650_set_led_pos(keyled_param_s param)
{
	MT_U8 i;

	if (param.length > 4)
	{
		printk(KERN_ERR "[%s %d]param error!\n", __FUNCTION__, __LINE__);
		return;
	}

	for (i=0; i<4; i++)
	{
		fd650_pos[i] = param.buf[i];
		printk(KERN_ERR "fd650_pos[%d]=%d\n", i, fd650_pos[i]);
	}
}

static void fd650_set_led_map(keyled_param_s param)
{
	MT_U8 i;

	if (param.length > 8)
	{
		printk(KERN_ERR "[%s %d]param error!\n", __FUNCTION__, __LINE__);
		return;
	}

	for (i=0; i<8; i++)
	{
		fd650_map[i] = param.buf[i];
		printk(KERN_ERR "fd650_map[%d]=%d\n", i, fd650_map[i]);
	}

	fd650_char_init();
}

static u32 fd650_get_keyval(void)
{
	return FD650_Read();
}

MT_VOID keyled_fd650_attach(keyled_info_s *klinfo, int kadc)
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
		klinfo->type |= MT_UNF_KEYLED_TYPE_FD650;

		klinfo->kl_ops.keyled_init = fd650_init;
		klinfo->kl_ops.display = fd650_display;
		klinfo->kl_ops.display_lbd = fd650_display_lbd;
		if (0 == kadc) {
			klinfo->kl_ops.get_keyval = fd650_get_keyval;
		}
		klinfo->kl_ops.set_brightness = fd650_set_bright_level;
		klinfo->kl_ops.display_asc = fd650_display_asc;
		klinfo->kl_ops.set_led_map = fd650_set_led_map;
		klinfo->kl_ops.set_led_pos = fd650_set_led_pos;
		klinfo->kl_ops.release = fd650_deinit;
		klinfo->idle_keyval = 0;
		klinfo->is_attach = 1;
	}
}


