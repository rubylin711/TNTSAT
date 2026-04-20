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
#include <linux/gpio.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <mt_unf_keyled.h>
#include <linux/mutex.h>
#include "mt_unf_gpio.h"
#include "../drv_keyled_priv.h"
#include "drv_pt6393.h"
#include "drv_sys_misc.h"
#include "mt_drv_pinctrl.h"
#include "drv_gpio_ioctl.h"

#include "mt_mach/symphony_regs.h"
//#include "mt_mach/symphony_reg_base_addr.h"
#include "mt_mach/symphony_io.h"
#include "mt_drv_clock.h"

//#define FP_CHIPTEST
#define FP_SPI_TIME_OUT				20000
#define SPI_FP_GPIO	0

#define PT6393_SPECIAL_MAX_CNT		28

static PT6393_DIS_LED pt6393_dis_content;
static led_bitmap_t pt6393_encode[] = {
	{'A', 0x0477},
	{'B', 0x047C},
	{'C', 0x0039},
	{'D', 0x110F},
	{'E', 0x0479},
	{'F', 0x0471},
	{'G', 0x043D},
	{'H', 0x0476},
	{'I', 0x1109},
	{'J', 0x000E},
	{'K', 0x0A70},
	{'L', 0x0038},
	{'M', 0x02B6},
	{'N', 0x08B6},
	{'O', 0x003F},
	{'P', 0x0473},
	{'Q', 0x083F},
	{'R', 0x0C73},
	{'S', 0x046D},
	{'T', 0x1101},
	{'U', 0x003E},
	{'V', 0x2230},
	{'W', 0x2836},
	{'X', 0x2A80},
	{'Y', 0x046E},
	{'Z', 0x2209},

	{'a', 0x0477},
	{'b', 0x047C},
	{'c', 0x0039},
	{'d', 0x110F},
	{'e', 0x0479},
	{'f', 0x0471},
	{'g', 0x043D},
	{'h', 0x0476},
	{'i', 0x1109},
	{'j', 0x000E},
	{'k', 0x0A70},
	{'l', 0x0038},
	{'m', 0x02B6},
	{'n', 0x08B6},
	{'o', 0x003F},
	{'p', 0x0473},
	{'q', 0x083F},
	{'r', 0x0C73},
	{'s', 0x046D},
	{'t', 0x1101},
	{'u', 0x003E},
	{'v', 0x2230},
	{'w', 0x2836},
	{'x', 0x2A80},
	{'y', 0x046E},
	{'z', 0x2209},

	{'0', 0x003F},
	{'1', 0x0006},
	{'2', 0x045B},
	{'3', 0x044F},
	{'4', 0x0466},
	{'5', 0x046D},
	{'6', 0x047D},
	{'7', 0x0007},
	{'8', 0x047F},
	{'9', 0x046F},
};

//the led's seg(a,b,c...) map according to hardward
//for example, if a is connected to p14(start from p1), the value is 13.
//u8 pt6393_seg_remap[14] = {13, 12, 5, 0, 1, 8, 7, 9, 10, 11, 6, 4, 3, 2};
static u8 pt6393_seg_remap[14] = {1, 2, 9, 14, 13, 6, 7, 5, 4, 3, 8, 10, 11, 12};
						   /*a, b, c, d ,  e,  f,  g, h, j,  k, m, n,   p,   r*/
//the start addr for every DIG.
static u8 pt6393_pos_addr[13] = {0x00, 0x03, 0x06, 0x09, 0x0C, 0x0F, 0x12, 0x15, 0x18, 0x1B, 0x1E, 0x21, 0x24};

//the led's pos map according to hardward
//for example, if 13G dispaly at first, the first element's value is 12,
//then find out the 13G's addr is 0x24 in pt6311b_pos_addr according to 12.
static u8 pt6393_pos_map[13] = {0,1,2,3,4,5,6,7,8,9,10,11,12};

//first element is the position to display accordong to hardware
//second element is the encode of special character, like REC,Play...
static u16 pt6393_special_encode[PT6393_SPECIAL_MAX_CNT][2] = {
	{0, 0x1000},//REC
	{0, 0x2000},//PLAY
	{0, 0x4000},//USB

	{1, 0x0001},//TSHIFT
	{2, 0x0001},//MOVIE
	{3, 0x0001},//MP3
	{4, 0x0001},//JPG
	{5, 0x0001},//ALL
	{6, 0x0001},//CYCLE
	{7, 0x0001},//SATTV
	{8, 0x0001},//RADIO
	{9, 0x0001},//STEREO
	{10, 0x0001},//AUDL
	{11, 0x0001},//AUDR

	{12, 0x4000},//Q
	{12, 0x2000},//S
	{12, 0x1000},//R1
	{12, 0x0800},//R2
	{12, 0x0400},//R3
	{12, 0x0200},//R4
	{12, 0x0100},//R5
	{12, 0x0080},//R6
	{12, 0x0040},//R7
	{12, 0x0020},//R8
	{12, 0x0008},//R9
	{12, 0x0010},//R10
};
extern keyled_info_s klinfo;
#ifdef FP_CHIPTEST
static void pt_6393_reset(void)
{
	u32 val = 0;
	printk(KERN_EMERG "###FPSPI reset. %s.%d \n",__FUNCTION__,__LINE__);

    val = HAL_GET_U32((volatile mt_u32 *)FP_SPI_CLK_EN);
    if (0 == (val&0x1)) {
        HAL_PUT_U32((volatile mt_u32 *)FP_SPI_CLK_EN, 1);
        printk(KERN_EMERG "###Open spi_fp clk. %s.%d \n",__FUNCTION__,__LINE__);
    }
    
	//2.reset
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_CH_BAUD,0xf00000ff);
	val = HAL_GET_U32((volatile mt_u32 *)FP_SPI_CH_BAUD);
	printk(KERN_EMERG "###FPSPI reset.before reg:0xbf159100 val:0x%x\n",val);

    mt_clk_reset(MT_CLK_AO_FPSPI);

	val = HAL_GET_U32((volatile mt_u32 *)FP_SPI_CH_BAUD);
	printk(KERN_EMERG "###FPSPI reset. Check reg:0xbf159100 val:0x%x ?= 0x30000008\n", val);
	if(0x30000008 == val)
	{
		printk(KERN_EMERG "###FPSPI reset OK.\n");
	}
	else
	{
		val = HAL_GET_U32((volatile mt_u32 *)FP_SPI_RESET_1);
		printk(KERN_EMERG "###FPSPI reset. 0xBF153844 val:0x%x\n",val);
		printk(KERN_EMERG "###FPSPI reset FAIL.\n");
	}
}
#endif

//remap the encode according to hardware
static void pt6393_remap_encode(led_bitmap_t *p_encode, u8 encode_len, u8 *p_remap, u8 map_len)
{
	u8 i = 0, j = 0;
	u16 temp = 0;
	if ((NULL == p_encode) || (NULL == p_remap))
	{
		return;
	}
	//printk(KERN_EMERG "[%s %d]encode_len=%d, map_len=%d\n", __FUNCTION__, __LINE__, encode_len, map_len);
	for (i=0; i<encode_len; i++)
	{
		//printk(KERN_EMERG "orignal p_encode[%d].ch=%c,bit map is 0x%x----", i, p_encode[i].ch,p_encode[i].bitmap);
		temp = 0;
		for (j=0; j<map_len; j++)
		{
			temp = temp | ((p_encode[i].bitmap & 0x1) << p_remap[j]);
			p_encode[i].bitmap >>= 1;
		}
		p_encode[i].bitmap = temp;

		//printk(KERN_EMERG "new p_encode[%d].bitmap=0x%x\n", i, p_encode[i].bitmap);
	}
}

#if SPI_FP_GPIO
static void pt6393_write(u8 *p_wbuf, u8 wlen)
{
	u8 i = 0;
	u8 j = 0;
	u8 dat = 0;
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
			udelay(pt6393_delay_us);

			PT6393_SCL_HIGH;
			dat >>= 1;
			udelay(pt6393_delay_us);
		}
	}
	PT6393_STB_HIGH;
}

#if 0
static void pt6393_read(u8 *p_rbuf, u8 rlen)
{
	u8 i = 0;
	u8 j=0;
	u8 dat = 0;
	gpio_value_e in_data = 0;
	PT6393_DATA_IN_MODE;
	PT6393_STB_LOW;
	for (i=0; i<rlen; i++)
	{
		dat = 0;
		for (j=0; j<8; j++)
		{
			PT6393_SCL_LOW;

			udelay(pt6393_delay_us);

			PT6393_SCL_HIGH;

			PT6393_DATA_IN_VAL(in_data);
			in_data <<= j;//lsb first
			dat |= in_data;
			udelay(pt6393_delay_us);
		}

		p_rbuf[i] = dat;
	}
	PT6393_STB_HIGH;

}
#endif

static void pt6393_write_read(u8 *p_wbuf, u8 wlen, u8 *p_rbuf, u8 rlen)
{
	u8 i = 0;
	u8 j=0;
	u8 dat = 0;
	gpio_value_e in_data = 0;

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
			udelay(pt6393_delay_us);

			PT6393_SCL_HIGH;
			dat >>= 1;
			udelay(pt6393_delay_us);
		}
	}

	PT6393_DATA_IN_MODE;
	udelay(pt6393_delay_us);
	for (i=0; i<rlen; i++)
	{
		dat = 0;
		for (j=0; j<8; j++)
		{
			PT6393_SCL_LOW;

			udelay(pt6393_delay_us);

			PT6393_SCL_HIGH;

			PT6393_DATA_IN_VAL(in_data);
			in_data <<= j;//lsb first
			dat |= in_data;
			udelay(pt6393_delay_us);
		}

		p_rbuf[i] = dat;
	}
	PT6393_STB_HIGH;
}

#else

static MT_BOOL spi_fp_is_trans_complete(void)
{
    u32 dtmp_sta = 0;
    dtmp_sta = HAL_GET_U32((volatile u32 *)FP_SPI_STA);
    if(!((dtmp_sta >> 16) & 0x1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static MT_BOOL spi_fp_is_txd_fifo_full(void)
{
    u32 dtmp = 0;
    dtmp = HAL_GET_U32((volatile u32 *)FP_SPI_STA);
    if((31 - ((dtmp >> 8) & 0x3f)) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static MT_BOOL spi_fp_is_rxd_fifo_empty(void)
{
    u32 dtmp = 0;
    dtmp = HAL_GET_U32((volatile u32 *)(FP_SPI_STA));
    if((dtmp & 0x3f) > 0)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}
/*
static void spi_fp_dump_reg(void)
{
	u32 val = 0;
	printk(KERN_EMERG "\n#######SPI REG INFO:\n");
	val = HAL_GET_U32((volatile u32 *)FP_SPI_CH_BAUD);
	printk(KERN_EMERG "BAUD_0xBF159100:	0x%x\n",val);
	val = HAL_GET_U32((volatile u32 *)FP_SPI_CH_MODE_CFG);
	printk(KERN_EMERG "MODE_0xBF159104:	0x%x\n",val);
	val = HAL_GET_U32((volatile u32 *)FP_SPI_STA);
	printk(KERN_EMERG "STA_0xBF159140:	0x%x\n",val);
	val = HAL_GET_U32((volatile u32 *)FP_SPI_TC);
	printk(KERN_EMERG "TC_0xBF159120:	0x%x\n",val);
	val = HAL_GET_U32((volatile u32 *)FP_SPI_CMD_FIFO);
	printk(KERN_EMERG "FIFO_0xBF159148:	0x%x\n",val);
	val = HAL_GET_U32((volatile u32 *)FP_SPI_TXD);
	printk(KERN_EMERG "TXD_0xBF159000:	0x%x\n",val);
	val = HAL_GET_U32((volatile u32 *)FP_SPI_CTRL);
	printk(KERN_EMERG "CTRL_0xBF159124:	0x%x\n",val);
}

static MT_BOOL fp_is_trans_complete(void)
{
    u32 dtmp_sta = 0;
    dtmp_sta = HAL_GET_U32((volatile u32 *)(FP_SPI_STA));
    if(!((dtmp_sta >> 16) & 0x1))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
*/
static RET_CODE spi_fp_write(u8 *p_cmd_buf, u32 cmd_len, u8 *p_data_buf, u32 data_len, u8 secure)
{
    u32 timeout = FP_SPI_TIME_OUT;
    u32 i  = 0;
    u8 r = 0;
    u32 j = 0;
    u32 dtmp = 0;
    u8 *p_buf = NULL;

    u32 cmd_dtmp = 0;

    if((p_cmd_buf == NULL) && (cmd_len != 0))
    {
        printk(KERN_EMERG "%s.%d\n",__FILE__,__LINE__);
        return ERR_PARAM;
    }

    if((p_data_buf == NULL) && (data_len != 0))
    {
        printk(KERN_EMERG "%s.%d\n",__FILE__,__LINE__);
        return ERR_PARAM;
    }

	cmd_dtmp = (cmd_len & 0x3f) << 16;
    if(cmd_len > 0)
    {
        p_buf = p_cmd_buf;
        HAL_PUT_U32((volatile u32 *)FP_SPI_TC, data_len);

        if(secure)
        {
            HAL_PUT_U32((volatile u32 *)FP_SPI_CTRL, (cmd_dtmp | 0x303));
        }
        else
        {
            HAL_PUT_U32((volatile u32 *)FP_SPI_CTRL, (cmd_dtmp | 0x203));
        }

        for(i = 0; i < cmd_len; i ++)
        {
            if(spi_fp_is_txd_fifo_full())
            {
                continue;
            }
            HAL_PUT_U32((volatile u32 *)FP_SPI_CMD_FIFO, p_buf[i]);
        }
    }

    if(data_len > 0)
    {
        i = data_len / 4;
        r = data_len % 4;

        p_buf = p_data_buf;
        if(cmd_len <= 0)
        {
            HAL_PUT_U32((volatile u32 *)FP_SPI_CTRL, data_len);

            if(secure)
            {
                HAL_PUT_U32((volatile u32 *)FP_SPI_CTRL, (cmd_dtmp | 0x303));
            }
            else
            {
                HAL_PUT_U32((volatile u32 *)FP_SPI_CTRL, (cmd_dtmp | 0x203));
            }
        }

        while(i)
        {
            if(spi_fp_is_txd_fifo_full())
            {
                continue;
            }
            dtmp = (p_buf[0] << 24) | (p_buf[1] << 16) | (p_buf[2] << 8) | p_buf[3];
            HAL_PUT_U32((volatile u32 *)FP_SPI_TXD, dtmp);
            i --;
            p_buf += 4;
        }

        while(spi_fp_is_txd_fifo_full());
        if(r > 0)
        {
            dtmp = 0;
            for(j = 1; j <= r; j++)
            {
                dtmp |= (p_buf[0] << (8 * (4 - j)));
                p_buf ++;
            }
			HAL_PUT_U32((volatile u32 *)FP_SPI_TXD, dtmp);
        }
    }
    timeout = FP_SPI_TIME_OUT;
    while(!spi_fp_is_trans_complete() && timeout)
    {
#ifndef MT_BUILD_LOADER
    	 usleep_range(50, 100);
#endif
         timeout --;
    }
    if(timeout == 0)
    {
        printk(KERN_EMERG "spi timeout  %s.%d\n",__FILE__,__LINE__);
    }
    return SUCCESS;
}


static RET_CODE spi_fp_read(u8 *p_wbuf, u8 wlen, u8 *p_rbuf, u8 rlen)
{
    u32 timeout = FP_SPI_TIME_OUT;
    u32 i = 0;
    u8 r = 0;
    u32 j = 0;
    u32 dtmp = 0;
    u8 *p_buf = 0;
    u32 cmd_dtmp = 0;

    cmd_dtmp = (wlen & 0x3f) << 16;

    if(wlen > 0)
    {
        p_buf = p_wbuf;
        HAL_PUT_U32((volatile u32 *)FP_SPI_TC, rlen);
		HAL_PUT_U32((volatile u32 *)FP_SPI_CTRL, (cmd_dtmp | 0x205));

        for(i = 0; i < wlen; i ++)
        {
			if(spi_fp_is_txd_fifo_full())
            {
                continue;
            }
            HAL_PUT_U32((volatile u32 *)FP_SPI_CMD_FIFO, p_buf[i]);
        }
    }

    if(rlen > 0)
    {
        i = rlen / 4;
        r = rlen % 4;
        p_buf = p_rbuf;

        while(i)
        {
            if(spi_fp_is_rxd_fifo_empty())
            {
                continue;
            }

            dtmp = HAL_GET_U32((volatile u32 *)(FP_SPI_RXD));

            p_buf[0] = (dtmp >> 24) & 0xff ;
            p_buf[1] = (dtmp >> 16) & 0xff;
            p_buf[2] = (dtmp >> 8) & 0xff;
            p_buf[3] = dtmp&0xff;
            i --;
            p_buf += 4;
        }

        if(r > 0)
        {
			while(spi_fp_is_rxd_fifo_empty());
            dtmp = HAL_GET_U32((volatile u32 *)(FP_SPI_RXD));
            for(j = 1; j <= r; j++)
            {
                p_buf[0] = (dtmp >> (8 * (4 - j))) & 0xff;
                p_buf ++;
            }
        }
    }
    timeout = FP_SPI_TIME_OUT;

    while(!spi_fp_is_trans_complete() && timeout)
    {
        timeout --;
    }
    if(timeout == 0)
    {
         printk(KERN_EMERG "spi timeout  %s.%d\n",__FILE__,__LINE__);
    }

    return SUCCESS;
}
#endif

static void pt6393_cmd_display_mode(u8 mode)
{
	u8 cmd = mode & 0x3F;//clear bit7/bit6
#if SPI_FP_GPIO
	pt6393_write(&cmd, 1);
#else
	spi_fp_write(&cmd, 1, NULL, 0, 0);
#endif
}

static void pt6393_cmd_date_setting( u8 addr_mode, u8 ctrl_mode)
{
	u8 cmd = 0;
	cmd = 0x40 |(addr_mode & 0x04) | (ctrl_mode & 0x03);//bit7/bit6 set to 01
#if SPI_FP_GPIO
	pt6393_write(&cmd, 1);
#else
	spi_fp_write(&cmd, 1, NULL, 0, 0);
#endif
}

static void pt6393_cmd_display_ctrl(u8 on_off, u8 bright)
{
	u8 cmd = 0;
	cmd = 0x80 | (on_off & 0x08) | (bright & 0x07);//bit7/bit6 set to 10
#if SPI_FP_GPIO
	pt6393_write(&cmd, 1);
#else
	spi_fp_write(&cmd, 1, NULL, 0, 0);
#endif
}

static void pt6393_fix_addr_write(u8 addr, u8 dat)
{
	u8 buf[2];

	buf[0] = 0xC0 | (addr & 0x3F);//bit7/bit6 set to 11
	buf[1] = dat;
#if SPI_FP_GPIO
	pt6393_write(buf, 2);
#else
	spi_fp_write(buf, 1, &buf[1], 1, 0);
#endif
}

#if SPI_FP_GPIO
static void pt6393_increase_addr_write(u8 addr, u8 *p_buf, u8 len)
{
	u8 buf[40];

	if (len>39)
	{
		len = 39;//max addr is 39 bytes.
	}

	buf[0] = 0xC0 | (addr & 0x3F);//bit7/bit6 set to 11

	memcpy(&buf[1], p_buf, len);

	pt6393_write(buf, len+1);
}
#endif

static void pt6393_init(void *dev)
{
	u8 encode_len = sizeof(pt6393_encode)/sizeof(led_bitmap_t);
	pt6393_remap_encode(pt6393_encode, encode_len, pt6393_seg_remap, sizeof(pt6393_seg_remap)/sizeof(u8));

	memset(&pt6393_dis_content,0,sizeof(PT6393_DIS_LED));

#if SPI_FP_GPIO
	//symphony_setpinmux(AO_PIN0, 0, 3, 0x1);
	//symphony_setpinmux(AO_PIN1, 0, 3, 0x1);
	//symphony_setpinmux(AO_PIN2, 0, 3, 0x1);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL0, 1);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL1, 1);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL2, 1);

	PT6393_STB_OUT_MODE;
	PT6393_STB_HIGH;
	PT6393_SCL_OUT_MODE;
	PT6393_SCL_LOW;
	PT6393_DATA_OUT_MODE;
	PT6393_DATA_LOW;
	mutex_init(&pt6393_dis_content.PT6393_lock);
	pt6393_cmd_display_mode(PT6393_DISPLAY_MODE_13DIG_15SEG);
	pt6393_cmd_display_ctrl(PT6393_DISPLAY_ON, PT6393_PULSE_WIDTH_7);

#else
	u32 val = 0;
    #ifdef FP_CHIPTEST
    pt_6393_reset();
    #endif
	//1.set pinmux
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL0, 2);//fp_spi_data
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL1, 2);//fp_spi_clk
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL2, 2);//fp_spi_csn

/*
	symphony_setpinmux(AO_PIN0, 0, 3, 0x2);
	symphony_setpinmux(AO_PIN1, 0, 3, 0x2);
	symphony_setpinmux(AO_PIN2, 0, 3, 0x2);
*/

/*
	//2.reset
	val = HAL_GET_U32((volatile mt_u32 *)FP_SPI_RESET_1);
	//val |= 0x3000;
	val = 0;
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_RESET_1,val);
*/
	//3.data cfg
	val = 0x10000;
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_DATA_CFG,val);

	//val = 0xf0000040;
	val = 0xf00000f0;//0x87->220K,0xf0->112K
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_CH_BAUD, val);
	val = 0x67f57f14;
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_CH_MODE_CFG, val);

	val = 0;//0x00100807;
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_INT_CFG, val);

	val = 0x0000005f;
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_PIN_MODE, val);
	val = 0x000100a0;
	HAL_PUT_U32((volatile mt_u32 *)FP_SPI_PIN_CTRL, val);

	mutex_init(&pt6393_dis_content.PT6393_lock);
	pt6393_cmd_display_mode(PT6393_DISPLAY_MODE_13DIG_15SEG);
	pt6393_cmd_display_ctrl(PT6393_DISPLAY_ON, PT6393_PULSE_WIDTH_7);

#endif
    if (klinfo.kl_ops.keyled_init_kadc) {
        klinfo.kl_ops.keyled_init_kadc(dev);
    }
}
static void pt6393_deinit(MT_VOID)
{
    if (klinfo.kl_ops.release_kadc) {
        klinfo.kl_ops.release_kadc();
    }
}

static void pt6393_clear_display(PT6393_DIS_LED *p_fp)
{
	u8 clear_buf[39];
	memset(clear_buf, 0, 39);
	if (NULL == p_fp)
	{
		return;
	}

	//memset(p_fp->dis_buff, 0, sizeof(p_fp->dis_buff)/sizeof(u16));
	//memset(p_fp->special_ch_buff, 0, sizeof(p_fp->special_ch_buff)/sizeof(u16));
	memset(p_fp->dis_buff, 0, sizeof(mt_u16) * FP_MAX_LED_NUM);
	memset(p_fp->special_ch_buff, 0, sizeof(mt_u16) * FP_MAX_LED_NUM);
#if SPI_FP_GPIO
	printk(KERN_EMERG "pt6393_cmd_date_setting\n");
	pt6393_cmd_date_setting(PT6393_INCREASE_ADDR, PT6393_WRITE_TO_DISPLAY_MODE);
	printk(KERN_EMERG "pt6393_increase_addr_write\n");
	pt6393_increase_addr_write(0x00, clear_buf, 39);
#else
    {
    	u8 cmd = 0;
    	printk(KERN_EMERG "pt6393_cmd_date_setting\n");
    	cmd =  0x40 |(PT6393_INCREASE_ADDR & 0x04) | (PT6393_WRITE_TO_DISPLAY_MODE & 0x03);
    	spi_fp_write(&cmd, 1, NULL, 0, 0);

    	printk(KERN_EMERG "pt6393_increase_addr_write\n");
    	cmd = 0;
    	cmd = 0xC0 | (0x00 & 0x3F);//bit7/bit6 set to 11
    	spi_fp_write(&cmd, 1, clear_buf, 39, 0);
    }
#endif
}

static void pt6393_display_hex(ulong param)
{//for debug
	u8 addr = ((param>>16)&0xff);
    printk(KERN_EMERG "show hex:%x\n",(u32)param);
    mutex_lock(&pt6393_dis_content.PT6393_lock);

    pt6393_clear_display(&pt6393_dis_content);

    pt6393_cmd_date_setting(PT6393_FIX_ADDR, PT6393_WRITE_TO_DISPLAY_MODE);
    pt6393_fix_addr_write(addr, (param&0xff));
    pt6393_fix_addr_write(addr+1, ((param>>8)&0xff));

    mutex_unlock(&pt6393_dis_content.PT6393_lock);
}

static void pt6393_display(ulong param)
{
	u8 i = 0, j = 0;
	u8 addr = 0;
	u8 index = 0;
	u8 len = 0;
	u8 encode_len = sizeof(pt6393_encode)/sizeof(led_bitmap_t);
	vfd_display_char *p_ledbuf = (void *)param;

	if (NULL == p_ledbuf)
	{
		return;
	}

	mutex_lock(&pt6393_dis_content.PT6393_lock);
	len = p_ledbuf->len;

	printk(KERN_EMERG "pt6393_display ch  is %s, len  is %d\n",p_ledbuf->ch,p_ledbuf->len);
	pt6393_clear_display(&pt6393_dis_content);
	pt6393_cmd_date_setting(PT6393_FIX_ADDR, PT6393_WRITE_TO_DISPLAY_MODE);

	if (len> 11)
	{
		len = 11;
	}

	pt6393_dis_content.dis_len = len;//store the length of display

	for (i=0; i<len; i++)//find the character's encode
	{
		for (j=0; j<encode_len; j++)
		{
			if (p_ledbuf->ch[i] == pt6393_encode[j].ch)
			{
				break;
			}
		}

		index = i+1;//the position from 1 to 11 will display normal character
		if (encode_len == j)//not find the character's encoder, so set to 0, not display
		{
			pt6393_dis_content.dis_buff[index] = 0;
		}
		else
		{
			pt6393_dis_content.dis_buff[index] = pt6393_encode[j].bitmap;
		}
	}

	pt6393_dis_content.dis_buff[index] |= pt6393_dis_content.special_ch_buff[index];//also set the sepcial character in display buf.

	for (i=1; i<len+1; i++)//display
	{
		index = pt6393_pos_map[i];		//find the index
		addr = pt6393_pos_addr[index]; //find the addr

		//printk(KERN_EMERG "%s %d index:%d addr:0x%x\n",__FUNCTION__,__LINE__,index,addr);
		pt6393_fix_addr_write(addr, (pt6393_dis_content.dis_buff[i] & 0xFF));
		pt6393_fix_addr_write(addr+1, ((pt6393_dis_content.dis_buff[i]>>8) & 0xFF));//use two addrs to display a character
	}

	mutex_unlock(&pt6393_dis_content.PT6393_lock);
	return ;
}

static void pt6393_display_special_ch(ulong param)
{
	led_special_ch_t index = 0;
	u8 pos = 0;
	u8 addr;
	u8 i;
	led_special_dis_cfg_t *spec_char = (void *)param;
	mutex_lock(&pt6393_dis_content.PT6393_lock);
	//printk(KERN_EMERG "special ch  is %d,on off  is %d\n",spec_char->ch,spec_char->on_off);
	pt6393_cmd_date_setting(PT6393_FIX_ADDR, PT6393_WRITE_TO_DISPLAY_MODE);
	if (NULL == spec_char)
	{
		return;
	}

	index = (led_special_ch_t)(spec_char->ch);
	if (PT6393_SPECIAL_MAX_CNT <= index)
	{
		return;
	}

	pos = pt6393_special_encode[index][0];
	i = pt6393_pos_map[pos];
	addr = pt6393_pos_addr[i];

	//if the pos larger than display's len, the normal character should be 0

	if ((pos > pt6393_dis_content.dis_len) && (pos < 12))
	{
		pt6393_dis_content.dis_buff[pos] = 0;
	}

	if (spec_char->on_off)
	{
		pt6393_dis_content.special_ch_buff[pos] |= pt6393_special_encode[index][1];
		pt6393_dis_content.dis_buff[pos] |= pt6393_special_encode[index][1];
	}
	else
	{
		pt6393_dis_content.special_ch_buff[pos] &= (~pt6393_special_encode[index][1]);
		pt6393_dis_content.dis_buff[pos] &= (~pt6393_special_encode[index][1]);
	}
	//printk(KERN_EMERG "pt6393_display_special_ch addr  is 0x%x, data  is 0x%x\n",addr,pt6393_dis_content.dis_buff[pos]);
	pt6393_fix_addr_write(addr, (pt6393_dis_content.dis_buff[pos] & 0xFF));
	pt6393_fix_addr_write(addr+1, ((pt6393_dis_content.dis_buff[pos]>>8) & 0xFF));
	mutex_unlock(&pt6393_dis_content.PT6393_lock);
}

static void pt6393_set_led_port(ulong param)
{
	keyled_display_lbd_s *p_lbdbuf = (void *)param;
	u8 buf[2] = {0, 0};
	if (NULL == p_lbdbuf)
	{
		return;
	}
	//printk(KERN_EMERG "led port enable is %d,pos is %d\n",p_lbdbuf->enable,p_lbdbuf->grid_pos);
	mutex_lock(&pt6393_dis_content.PT6393_lock);

	buf[0] = 0x40 | (PT6393_FIX_ADDR & 0x04) | (PT6393_WRITE_TO_LED_PORT & 0x03);//bit7/bit6 set to 01
	if (p_lbdbuf->enable)
	{
		pt6393_dis_content.led_port_value |= 1<<p_lbdbuf->grid_pos;
	}
	else
	{
		pt6393_dis_content.led_port_value &= (~(1<<p_lbdbuf->grid_pos));
	}

	buf[1] = pt6393_dis_content.led_port_value;

#if SPI_FP_GPIO
	pt6393_write(buf, 2);
#else
	spi_fp_write(buf, 1, &buf[1], 1, 0);
#endif

	mutex_unlock(&pt6393_dis_content.PT6393_lock);
}

/*********************************
* encode the key as below:
* K1  1     3      5       7......
* K2  2     4      6       8......
*    SG1  SG2 SG2  SG4......
*********************************/
static u32 pt6393_get_key(MT_VOID)
{
	u8 w_data= 0;
	u8 buf[4];
	u8 key = 0;
	u8 i = 0, j = 0;
	memset(buf, 0, 4);

	mutex_lock(&pt6393_dis_content.PT6393_lock);
	#if 1
	w_data = 0x40 | (PT6393_FIX_ADDR & 0x04) | (PT6393_READ_KEY_DATA & 0x03);//bit7/bit6 set to 01

	//OS_PRINTF("[%s %d]w_data = 0x%x\n", __FUNCTION__, __LINE__, w_data);

#if SPI_FP_GPIO
	pt6393_write_read(&w_data, 1, buf, 4);
#else
	spi_fp_read(&w_data, 1, buf, 4);
#endif

	for (i=0; i<4; i++)
	{
		for (j=0; j<8; j++)
		{
			if ((buf[i]>>j)&0x01)
			{
				key = i*8 + (j+1);
				//printk(KERN_EMERG "[%s %d]buf[%d]=0x%x,key is %d\n", __FUNCTION__, __LINE__, i, buf[i],key);
				i = 4;//use this to exit out loop
				break;
			}
		}
	}
	mutex_unlock(&pt6393_dis_content.PT6393_lock);
	#endif
	return key;

}

MT_VOID keyled_pt6393_attach(keyled_info_s *klinfo, int kadc)
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
		klinfo->type |= MT_UNF_KEYLED_TYPE_PT6393;
		printk(KERN_EMERG "%s %d\n",__FUNCTION__,__LINE__);

		klinfo->kl_ops.keyled_init = pt6393_init;
		klinfo->kl_ops.display = pt6393_display_hex;
		klinfo->kl_ops.vfd_display_ch = pt6393_display;
		klinfo->kl_ops.display_special_ch = pt6393_display_special_ch;
		klinfo->kl_ops.display_lbd = pt6393_set_led_port;
		if (0 == kadc) {
			klinfo->kl_ops.get_keyval = pt6393_get_key;
		}
		klinfo->kl_ops.release = pt6393_deinit;
		#ifdef FP_CHIPTEST
		klinfo->kl_ops.vfd_reset = pt_6393_reset;
		#endif
		klinfo->idle_keyval = 0;
		klinfo->is_attach = 1;
	}
}
