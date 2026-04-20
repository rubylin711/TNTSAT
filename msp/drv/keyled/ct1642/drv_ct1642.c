/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/timer.h>
#include <linux/irqreturn.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>

#include "mt_mach/irq.h"

#include "mt_unf_keyled.h"
#include "mt_error_mpi.h"
#include "mt_drv_keyled.h"
#include "drv_keyled_ioctl.h"
#include "../drv_keyled_priv.h"
#include "drv_ct1642.h"
#include "mt_drv_pinctrl.h"
#include "drv_gpio_ioctl.h"
#include "mt_module_debug.h"

#define CT1642_LED_BITMAP_CFG	(1)
static keyled_keyfifo_s kvfifo_ct1642;

struct mt_ledkb_dev g_dev_ledkb;

extern keyled_info_s klinfo;

static volatile MT_U32 attached = 0;

#if CT1642_LED_BITMAP_CFG
static led_bitmap_t ct1642_bitmap[] =
{
	{'.',0x20}, {'0',0xd7}, {'1',0x14}, {'2',0xcd},
	{'3',0x5d}, {'4',0x1e}, {'5',0x5b}, {'6',0xdb},
	{'7',0x15}, {'8',0xdf}, {'9',0x5f}, {'a',0x9f},
	{'A',0x9f}, {'b',0xda}, {'B',0xda}, {'c',0xc3},
	{'C',0xc3}, {'d',0xdc}, {'D',0xdc}, {'e',0xcb},
	{'E',0xcb}, {'f',0x8b}, {'F',0x8b}, {'g',0x5f},
	{'G',0xd3}, {'h',0x9e}, {'H',0x9e}, {'i',0x10},
	{'I',0x82}, {'j',0x54}, {'J',0x54}, {'l',0xc2},
	{'L',0xc2}, {'n',0x98}, {'N',0x97}, {'o',0xd8},
	{'O',0xd7}, {'p',0x8f}, {'P',0x8f}, {'q',0x1f},
	{'Q',0x1f}, {'r',0x88}, {'R',0x9f}, {'s',0x5b},
	{'S',0x5b}, {'t',0xca}, {'T',0x83}, {'u',0xd6},
	{'U',0xd6}, {'y',0x5e}, {'Y',0x5e}, {'z',0xcd},
	{'Z',0xcd}, {':',0x20}, {'-',0x08}, {'_',0x40},
	{' ',0x00}, {'U',0xd6}, {'v',0xd6}, {'V',0xd6},
	{'x',0x9e}, {'X',0x9e}, {'y',0x5e}, {'Y',0x5e},
	{'z',0xcd}, {'Z',0xcd}, {':',0x00}, {'-',0x08},
	{'_',0x40},
};
#else
static led_bitmap_t ct1642_bitmap[] =
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
#endif

#define FP_CT1642_TABLE_SIZE sizeof(ct1642_bitmap) / sizeof(led_bitmap_t)

//#define  WRITE_REG(Addr, Value) ((*(volatile mt_u32 *)(Addr)) = (Value))
//#define  READ_REG(Addr) (*(volatile mt_u32 *)(Addr))


static MT_VOID keyled_keyfifo_ct1642_init(MT_VOID)
{
    memset(kvfifo_ct1642.kv_ringbuf, 0, sizeof(kvfifo_ct1642.kv_ringbuf));
    kvfifo_ct1642.count = 0;
    kvfifo_ct1642.rp = 0;
    kvfifo_ct1642.wp = 0;
    spin_lock_init(&kvfifo_ct1642.lock);
}

static MT_S32 keyled_keyfifo_ct1642_put(MT_U32 keyval, MT_U32 pressStatus)
{
    spin_lock(&kvfifo_ct1642.lock);
    if(kvfifo_ct1642.count < KEYLED_KEYVAL_RINGBUF_SIZE)
    {
        kvfifo_ct1642.kv_ringbuf[kvfifo_ct1642.wp].keyVal = keyval;
        kvfifo_ct1642.kv_ringbuf[kvfifo_ct1642.wp].pressStatus = pressStatus;
        kvfifo_ct1642.wp ++;
        kvfifo_ct1642.wp = kvfifo_ct1642.wp % KEYLED_KEYVAL_RINGBUF_SIZE;
        kvfifo_ct1642.count ++;
        spin_unlock(&kvfifo_ct1642.lock);
        return 0;
    }
    spin_unlock(&kvfifo_ct1642.lock);
    return -1;
}

static MT_S32 keyled_keyfifo_ct1642_get(MT_U32 *keyval, MT_U32 *pressStatus)
{
	if((NULL == keyval) || (NULL== pressStatus))
	{
		return -1;
	}

	spin_lock(&kvfifo_ct1642.lock);
	if(kvfifo_ct1642.count > 0 && kvfifo_ct1642.count <= KEYLED_KEYVAL_RINGBUF_SIZE)
	{
		*keyval = kvfifo_ct1642.kv_ringbuf[kvfifo_ct1642.rp].keyVal;
		*pressStatus = kvfifo_ct1642.kv_ringbuf[kvfifo_ct1642.rp].pressStatus;
		kvfifo_ct1642.rp ++;
		kvfifo_ct1642.rp = kvfifo_ct1642.rp % KEYLED_KEYVAL_RINGBUF_SIZE;
		kvfifo_ct1642.count --;
		spin_unlock(&kvfifo_ct1642.lock);
		return 0;
	}
	spin_unlock(&kvfifo_ct1642.lock);
	return -1;
}


static void SetHostFreq(u32 clk)
{
    mt_u32 div = 0;
    mt_u32 ledkb_time_set = 0;
    mt_u32 need_num_clk = 0;
    mt_u32 one_led_display_clk = 0;
    mt_u32 reg_value = 0;

    //printk("SetClk:%d\n",clk);
    if(clk == 0) return;
    div = 12000000 / clk - 1;
    if(div > 255)
    {
       MT_ERR_KEYLED("Error Clk too low!!!(div>256) %d\n",div);
       return;
    }
    ledkb_time_set = (div << 24)|0xf;

    //WRITE_REG(LEDKB_TIMESEL,ledkb_time_set);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_TIMESEL), ledkb_time_set);

    //printk("LEDKB_TIMESEL =0x%08x\n", HAL_GET_U32((volatile mt_u32 *)LEDKB_TIMESEL));
    need_num_clk = 20 * clk / 1000;
    one_led_display_clk = (need_num_clk - 32*8)/4;
    if(one_led_display_clk < 20)
    {
      MT_ERR_KEYLED("Error Clk too low %d!!!\n",one_led_display_clk);
      return;
    }
    if(one_led_display_clk > 65535)
    {
      MT_ERR_KEYLED("Error Clk too high %d!!!\n",one_led_display_clk);
      return;
    }
    reg_value = (one_led_display_clk << 16)+32;

    //WRITE_REG(LEDKB_TIMESET, reg_value);
    HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_TIMESET), reg_value);

    //printk("LEDKB_TIMESET=%08x\n", HAL_GET_U32((volatile mt_u32 *)LEDKB_TIMESET));
}


static MT_U32 get_key_mask_value(MT_U32 bit)
{
  MT_U32 key_value[8]={0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
  MT_U32 scankey_mask = 0;
  scankey_mask =(key_value[bit] << 10) | 0xf;
  return scankey_mask;
}


static irqreturn_t ct1642_interrupt_handler(int irq, void *param)
{
    mt_u32 key_press[2] ={0};
    mt_u32 key_int = 0;
    mt_u32 key_release[2] = {0};
    mt_u8  key_int_status = 0;
    mt_u16 cur_key_val;

    mt_u32 frontkey_value = 0;
    key_press[0] = 0;
    key_press[1] = 0;
    key_release[0] = 0;
    key_release[1] = 0;

	//key_int = HAL_GET_U32((volatile mt_u32 *)LEDKB_KEY_PRESS0);
	key_int = HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_KEY_PRESS0));
    key_int_status = (key_int >> 24) & 0x0f;

    if(HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_INTMASK_KEYFILT)) & 0x100000)
    {
		key_press[0] = HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_KEY_PRESS0));
		key_release[0] = HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_KEY_RELEASE0));
		HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_KEY_PRESS0), key_press[0]);
		HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_KEY_RELEASE0), key_release[0]);
    }

    if(HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_INTMASK_KEYFILT)) & 0x200000)
    {
		key_press[1] = HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_KEY_PRESS1));
		key_release[1] = HAL_GET_U32((volatile mt_u32 *)(g_dev_ledkb.base + LEDKB_KEY_RELEASE1));
		HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_KEY_PRESS1), key_press[1]);
		HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_KEY_RELEASE1), key_release[1]);
    }

	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_KEY_PRESS0), key_int);

    key_press[0] = key_press[0]&0x3ffff;
    key_release[0] = key_release[0]&0x3ffff;
    key_press[1] = key_press[1]&0x3ffff;
    key_release[1] = key_release[1]&0x3ffff;

    if(key_int_status&0x05)//Any key been press
    {
      if(key_press[0])
      {
        frontkey_value = (key_press[0] >> 4);
        cur_key_val = frontkey_value | (UIO_FRONTPANEL << 8);
	 keyled_keyfifo_ct1642_put(cur_key_val, MT_UNF_KEY_STATUS_DOWN);
      }
      if(key_press[1])
      {
        frontkey_value = (key_press[1] >> 4);
        cur_key_val = frontkey_value | (UIO_FRONTPANEL << 8);
	 keyled_keyfifo_ct1642_put(cur_key_val, MT_UNF_KEY_STATUS_DOWN);
      }
    }

    if(key_int_status&0x0a)//Any key been release
    {
      if(key_release[0])
      {
          frontkey_value = (key_release[0] >> 4);
          cur_key_val = frontkey_value | (UIO_FRONTPANEL << 8);
	   keyled_keyfifo_ct1642_put(cur_key_val, MT_UNF_KEY_STATUS_UP);
      }
      if(key_release[1])
      {
          frontkey_value = (key_release[1] >> 4);
          cur_key_val = frontkey_value | (UIO_FRONTPANEL << 8);
    	   keyled_keyfifo_ct1642_put(cur_key_val, MT_UNF_KEY_STATUS_UP);
      }
    }

    return IRQ_HANDLED;
}


static MT_VOID ct1642_init(void *dev)
{
	mt_u32 seg_data[4] ={0};
	//mt_u32 val = 0;
	mt_s32 ret = 0;
	unsigned long ir_flag = IRQF_TRIGGER_RISING;
	struct mt_ledkb_dev *pledkb = NULL;

	memset(&g_dev_ledkb,0,sizeof(struct mt_ledkb_dev));
	if(dev)
	{
		pledkb = &((struct mt_keyled_device *)dev)->ledkb_dev;
	}

	if(pledkb)
	{
		g_dev_ledkb.base = pledkb->base;
		g_dev_ledkb.clk_khz = pledkb->clk_khz;
	}
	else
	{
		g_dev_ledkb.base = (void *)LEDKB_HOST_REG_BASE;
		g_dev_ledkb.clk_khz = 200;
	}

	//pinmux
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL0, 0);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL1, 0);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL2, 0);

	drv_gpio_io_enable(AO_GPIO_2,GPIO_MASK_ENABLE);
	drv_gpio_set_dir(AO_GPIO_2,GPIO_DIR_INPUT);

	//printk("ledkeyscan_init(ct1642)...\n");
	seg_data[0] = 0xff;
	seg_data[1] = 0xff;
	seg_data[2] = 0xff;
	seg_data[3] = 0xff;

	//WRITE_REG(LEDKB_CLK_PARA,0xb00000);
	//WRITE_REG(LEDKB_DAT_DELAY_NUM,1);
	//WRITE_REG((unsigned long *)LEDKB_INTMASK_KEYFILT,0x00100ff0);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_CLK_PARA), 0xb00000);

	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_DELAY_NUM), 1);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_INTMASK_KEYFILT), 0x00100ff0);
	//SetHostFreq(200*FREQ_KHZ);
	SetHostFreq(g_dev_ledkb.clk_khz*FREQ_KHZ);

	//WRITE_REG(LEDKB_DAT_PARA(0),(seg_data[0] << 10) | 0xe);
	//WRITE_REG(LEDKB_DAT_PARA(1),(seg_data[1] << 10) | 0xd);
	//WRITE_REG(LEDKB_DAT_PARA(2),(seg_data[2] << 10) | 0xb);
	//WRITE_REG(LEDKB_DAT_PARA(3),(seg_data[3] << 10) | 0x7);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(0)), (seg_data[0] << 10) | 0xe);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(1)), (seg_data[1] << 10) | 0xd);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(2)), (seg_data[2] << 10) | 0xb);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(3)), (seg_data[3] << 10) | 0x7);

/*
	WRITE_REG(LEDKB_DAT_PARA(4),get_key_mask_value(0));
	WRITE_REG(LEDKB_DAT_PARA(5),get_key_mask_value(1));
	WRITE_REG(LEDKB_DAT_PARA(6),get_key_mask_value(2));
	WRITE_REG(LEDKB_DAT_PARA(7),get_key_mask_value(3));
	WRITE_REG(LEDKB_DAT_PARA(8),get_key_mask_value(4));
	WRITE_REG(LEDKB_DAT_PARA(9),get_key_mask_value(5));
	WRITE_REG(LEDKB_DAT_PARA(10),get_key_mask_value(6));
	WRITE_REG(LEDKB_DAT_PARA(11),get_key_mask_value(7));
*/
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(4)), get_key_mask_value(0));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(5)), get_key_mask_value(1));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(6)), get_key_mask_value(2));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(7)), get_key_mask_value(3));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(8)), get_key_mask_value(4));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(9)), get_key_mask_value(5));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(10)), get_key_mask_value(6));
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(11)), get_key_mask_value(7));

	if(attached == 0)
	{
	    keyled_keyfifo_ct1642_init();

		ir_flag = IRQF_TRIGGER_HIGH;
		ret = request_irq(IRQ_LEDKB_ID,(irq_handler_t)ct1642_interrupt_handler,
							ir_flag,"KEYLED_SYMPHONY", NULL);
		if (ret != MT_SUCCESS)
		{
			MT_ERR_KEYLED("register CT1642 INT failed 0x%x.\n", ret);
		}
		attached = 1;
	}

	//WRITE_REG(LEDKB_CTRL,0x12020b09); /*4 led 8 key*/
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_CTRL), 0x12020b09); /*4 led 8 key*/
    if (NULL != klinfo.kl_ops.keyled_init_kadc) {
        klinfo.kl_ops.keyled_init_kadc(dev);
    }
}


static void ct1642_display(ulong param)
{
    MT_U32 ledval = (MT_U32)param;
/*
    WRITE_REG((mt_u32)LEDKB_DAT_PARA(0),(((ledval>>24)&0xff)<<10)|0xe);
    WRITE_REG((mt_u32)LEDKB_DAT_PARA(1),(((ledval>>16)&0xff)<<10)|0xd);
    WRITE_REG((mt_u32)LEDKB_DAT_PARA(2),(((ledval>>8)&0xff)<<10)|0xb);
    WRITE_REG((mt_u32)LEDKB_DAT_PARA(3),((ledval&0xff)<<10)|0x7);
*/
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(0)), (((ledval>>24)&0xff)<<10)|0xe);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(1)), (((ledval>>16)&0xff)<<10)|0xd);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(2)), (((ledval>>8)&0xff)<<10)|0xb);
	HAL_PUT_U32((volatile mt_u32*)(g_dev_ledkb.base + LEDKB_DAT_PARA(3)), ((ledval&0xff)<<10)|0x7);
}

static void ct1642_display_asc(ulong param)
{
	keyled_display_char_s *p_ledbuf = (keyled_display_char_s *)param;
	MT_U8 i = 0, j = 0;
	ulong ledval = 0;

	if(NULL == p_ledbuf)
	{
		return;
	}

	for (i=0; i<FP_MAX_LED_NUM; i++)
	{
		for(j = 0; j < FP_CT1642_TABLE_SIZE; j++)
		{
			if(ct1642_bitmap[j].ch == p_ledbuf->ch[i])
			{
				ledval |= (ct1642_bitmap[j].bitmap) << ((3-i)*8);
				break;
			}
		}
	}
	ct1642_display(ledval);
}


static MT_U32 g_keyval = 0;
static MT_U32 ct1642_get_keyval(MT_VOID)
{
	keyled_keyval_s keyval = {0, 0};

	(void)keyled_keyfifo_ct1642_get(&keyval.keyVal, &keyval.pressStatus);

	if(g_keyval == 0)
	{
		g_keyval = keyval.keyVal;
	}

	if(keyval.pressStatus == MT_UNF_KEY_STATUS_DOWN)
	{
		return g_keyval;
	}
	else
	{
		g_keyval = 0;
		return keyval.keyVal;
	}

}

static MT_VOID ct1642_release(MT_VOID)
{
	MT_INFO_KEYLED("keyled_ct1642_realese\n");
	if (attached) {
		attached = 0;
		free_irq(IRQ_LEDKB_ID, NULL);
		if (NULL != klinfo.kl_ops.release_kadc) {
			klinfo.kl_ops.release_kadc();
		}
	}
}

MT_VOID keyled_ct1642_attach(keyled_info_s *klinfo, int kadc)
{
	if(klinfo)
	{
		#ifdef CFG_MT_KEYLED_ADC_SUPPORT
		if (kadc) {
			keyled_keyadc_attach(klinfo);
			klinfo->kl_ops.keyled_init_kadc = klinfo->kl_ops.keyled_init;
			klinfo->kl_ops.release_kadc = klinfo->kl_ops.release;
		}
		#endif
		klinfo->type |= MT_UNF_KEYLED_TYPE_CT1642;
		klinfo->kl_ops.keyled_init = ct1642_init;
		klinfo->kl_ops.display = ct1642_display;
		klinfo->kl_ops.display_asc = ct1642_display_asc;
		if (0 == kadc) {
			klinfo->kl_ops.get_keyval = ct1642_get_keyval;
		}
		klinfo->kl_ops.release = ct1642_release;
		klinfo->idle_keyval = 0;
		klinfo->is_attach = 1;
	}
}

