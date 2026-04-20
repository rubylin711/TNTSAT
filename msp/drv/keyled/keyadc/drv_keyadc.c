/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/sched.h>
#include <linux/wait.h>
//#include <../../arch/arm/mach-aria/aria_reg_base_addr.h>
#include <linux/timer.h>
#include <linux/irqreturn.h>
#include <linux/semaphore.h>
#include <linux/interrupt.h>
#include <linux/gpio/driver.h>

#include "mt_unf_keyled.h"
#include "mt_error_mpi.h"
#include "mt_drv_keyled.h"
#include "drv_keyled_ioctl.h"
#include "../drv_keyled_priv.h"
#include "mt_module_debug.h"
//#include "mt_unf_gpio.h"
#include "mt_drv_pinctrl.h"
#include "drv_gpio_ioctl.h"


#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

//#define SUPPORT_KADC_INT
#define KADC_ERROR(...)				printk(KERN_ERR __VA_ARGS__)

#define KADC_PLUUING_CNT (10)
#define KADC_REPEAT_TICKS (30)

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define KEY_ADC_BASE_ADDR (SYMPHONY_IO_VA(0xbf156000))
#else
#define KEY_ADC_BASE_ADDR 0xbf156000
#endif

//#define KEY_ADC_BASE_ADDR 0xbf156000
#define KEY_ADC_CFG     (KEY_ADC_BASE_ADDR)
#define KEY_ADC_VALUE   (KEY_ADC_BASE_ADDR + 0x04)
#define KEY_ADC_INT 	(KEY_ADC_BASE_ADDR + 0x08)
#define KEY_ADC_RANGE 	(KEY_ADC_BASE_ADDR + 0x0C)
#define KEY_ADC_SAMPLE	(KEY_ADC_BASE_ADDR + 0x10)

#define IR_IRQ_KEYADC   (IRQ_KADC_ID)

static volatile int flag_kadc_init = 0;
static mt_u8 kadc_version = 0;
extern keyled_info_s klinfo;

#ifdef SUPPORT_KADC_INT
static keyled_keyfifo_s kvfifo_keyadc;
static MT_VOID keyled_keyfifo_keyadc_init(MT_VOID)
{
	memset(kvfifo_keyadc.kv_ringbuf, 0, sizeof(kvfifo_keyadc.kv_ringbuf));
	kvfifo_keyadc.count = 0;
	kvfifo_keyadc.rp = 0;
	kvfifo_keyadc.wp = 0;
	spin_lock_init(&kvfifo_keyadc.lock);
}

static MT_S32 keyled_keyfifo_keyadc_put(MT_U32 keyval, MT_U32 pressStatus)
{
    spin_lock(&kvfifo_keyadc.lock);
    if(kvfifo_keyadc.count < KEYLED_KEYVAL_RINGBUF_SIZE)
    {
        kvfifo_keyadc.kv_ringbuf[kvfifo_keyadc.wp].keyVal = keyval;
        kvfifo_keyadc.kv_ringbuf[kvfifo_keyadc.wp].pressStatus = pressStatus;
        kvfifo_keyadc.wp ++;
        kvfifo_keyadc.wp = kvfifo_keyadc.wp % KEYLED_KEYVAL_RINGBUF_SIZE;
        kvfifo_keyadc.count ++;
        spin_unlock(&kvfifo_keyadc.lock);
        return 0;
    }
    spin_unlock(&kvfifo_keyadc.lock);
    return -1;
}

static MT_S32 keyled_keyfifo_keyadc_get(MT_U32 *keyval, MT_U32 *pressStatus)
{
	if((NULL == keyval) || (NULL== pressStatus))
		return -1;

	spin_lock(&kvfifo_keyadc.lock);
	if(kvfifo_keyadc.count > 0 && kvfifo_keyadc.count <= KEYLED_KEYVAL_RINGBUF_SIZE)
	{
		*keyval = kvfifo_keyadc.kv_ringbuf[kvfifo_keyadc.rp].keyVal;
		*pressStatus = kvfifo_keyadc.kv_ringbuf[kvfifo_keyadc.rp].pressStatus;
		kvfifo_keyadc.rp ++;
		kvfifo_keyadc.rp = kvfifo_keyadc.rp % KEYLED_KEYVAL_RINGBUF_SIZE;
		kvfifo_keyadc.count --;
		spin_unlock(&kvfifo_keyadc.lock);
		return 0;
	}
	spin_unlock(&kvfifo_keyadc.lock);
	return -1;
}
#endif

static MT_VOID keyadc_display(ulong param)
{
	//MT_U32 ledval = (MT_U32)param;
	return;
}

static mt_u8 keyadc_key_idx_new(u32 key_value)
{
	mt_u8 key_idx = 0;
	switch(klinfo.kadc_type)
	{
	    case KADC_KEY_TYPE_3:
	        {
	            if (key_value <= 5)
	                key_idx = 1;
	            else if (key_value > 5 && key_value <= 16)
	                key_idx = 2;
	            else if (key_value > 16 && key_value <= 27)
	                key_idx = 3;
	            else//key_value > 27
	                key_idx = 0;
	        }
	        break;

	    case KADC_KEY_TYPE_5:
	        {
	            if (key_value <= 3)
	                key_idx = 1;
	            else if (key_value > 3 && key_value <= 10)
	                key_idx = 2;
	            else if (key_value > 10 && key_value <= 17)
	                key_idx = 3;
	            else if (key_value > 17 && key_value <= 24)
	                key_idx = 4;
	            else if (key_value > 24 && key_value <= 31)
	                key_idx = 5;
	            else//key_value > 31
	                key_idx = 0;
	        }
	        break;

	    case KADC_KEY_TYPE_7:
	        {
	            if (key_value <= 2)
	                key_idx = 1;
	            else if (key_value > 2 && key_value <= 7)
	                key_idx = 2;
	            else if (key_value > 7 && key_value <= 12)
	                key_idx = 3;
	            else if (key_value > 12 && key_value <= 17)
	                key_idx = 4;
	            else if (key_value > 17 && key_value <= 22)
	                key_idx = 5;
	            else if (key_value > 22 && key_value <= 27)
	                key_idx = 6;
	            else if (key_value > 27 && key_value <= 32)
	                key_idx = 7;
	            else//key_value > 32
	                key_idx = 0;
	        }
	        break;

	    default:
	        key_idx = 0;
	        break;
	}

    return key_idx;

}

//kadc index 1/2/3/4/5/6/7
static mt_u8 keyadc_key_idx(u32 key_value)
{
	mt_u8 key_idx = 0;

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
	//KADC_ERROR("##%s, keyadc_type=%d, key_value=%d###\n", __FUNCTION__, klinfo.kadc_type, key_value);
	switch(klinfo.kadc_type)
	{
		//Attention:at present(20201201), only have the data of KADC_KEY_TYPE_7
		//if the KADC_KEY_TYPE_3/KADC_KEY_TYPE_5 have new data, should modify this
	    case KADC_KEY_TYPE_3:
        {
            if (key_value > 52)
            {
             	key_idx = 1;
            }
            else if (key_value > 44 && key_value <= 52)
            {
            	key_idx = 2;
            }
            else if (key_value > 36 && key_value <= 44)
            {
            	key_idx = 3;
            }
            else
            {
            	key_idx = 0;
            }
			break;
        }

	    case KADC_KEY_TYPE_5:
        {
            if (key_value > 52)
            {
             	key_idx = 1;
            }
            else if (key_value > 44 && key_value <= 52)
            {
            	key_idx = 2;
            }
            else if (key_value > 36 && key_value <= 44)
            {
            	key_idx = 3;
            }
            else if (key_value > 29 && key_value <= 36)
            {
            	key_idx = 4;
            }
            else if (key_value > 21 && key_value <= 29)
            {
            	key_idx = 5;
            }
            else
            {
            	key_idx = 0;
            }

			break;
        }

	    case KADC_KEY_TYPE_7:
        {
            if (key_value > 52)
            {
             	key_idx = 1;
            }
            else if (key_value > 44 && key_value <= 52)
            {
            	key_idx = 2;
            }
            else if (key_value > 36 && key_value <= 44)
            {
            	key_idx = 3;
            }
            else if (key_value > 29 && key_value <= 36)
            {
            	key_idx = 4;
            }
            else if (key_value > 21 && key_value <= 29)
            {
            	key_idx = 5;
            }
            else if (key_value > 13 && key_value <= 21)
            {
            	key_idx = 6;
            }
            else if (key_value > 4 && key_value <= 13)
            {
            	key_idx = 7;
            }
            else
            {
            	key_idx = 0;
            }

			break;
        }

	    default:
	        key_idx = 0;
	        break;
	}
#else
	//KADC_ERROR("##%s, keyadtype=[%d]###\n", __FUNCTION__,klinfo.kadc_type);
	switch(klinfo.kadc_type)
	{
	    case KADC_KEY_TYPE_3:
        {
            if (key_value <= 10)
                key_idx = 1;
            else if (key_value > 10 && key_value <= 28)
                key_idx = 2;
            else if (key_value > 28 && key_value <= 50)
                key_idx = 3;
            else//key_value > 50
                key_idx = 0;

			break;
        }

	    case KADC_KEY_TYPE_5:
        {
            if (key_value <= 4)
                key_idx = 1;
            else if (key_value > 4 && key_value <= 15)
                key_idx = 2;
            else if (key_value > 15 && key_value <= 26)
                key_idx = 3;
            else if (key_value > 26 && key_value <= 38)
                key_idx = 4;
            else if (key_value > 38 && key_value <= 55)
                key_idx = 5;
            else//key_value > 55
                key_idx = 0;

			break;
        }

	    case KADC_KEY_TYPE_7:
        {
            if (key_value <= 4)
                key_idx = 1;
            else if (key_value > 4 && key_value <= 11)
                key_idx = 2;
            else if (key_value > 11 && key_value <= 20)
                key_idx = 3;
            else if (key_value > 20 && key_value <= 30)
                key_idx = 4;
            else if (key_value > 30 && key_value <= 39)
                key_idx = 5;
            else if (key_value > 39 && key_value <= 49)
                key_idx = 6;
            else if (key_value > 49 && key_value <= 59)
                key_idx = 7;
            else//key_value > 60
                key_idx = 0;

			break;
        }

	    default:
	        key_idx = 0;
	        break;
	}
#endif

    //KADC_ERROR("##%s,key_idx=[%d]##  key_value:%d\n",__FUNCTION__,key_idx, key_value);
    return key_idx;

}

static MT_U32 keyadc_get_keyval(MT_VOID)
{
	mt_u32 keyvalue = 0;
	mt_u32 key_idx = 0;
	mt_u8 vals[KADC_PLUUING_CNT] = {0}, i = 0, valid_cnt = 0;

    #ifdef SUPPORT_KADC_INT
    {
        MT_U32 pressStatus;
        keyled_keyfifo_keyadc_get(&key_idx, &pressStatus);
        return key_idx;
    }
    #else
	for (i = 0; i < KADC_PLUUING_CNT; i++)
	{
		vals[i] = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_VALUE);
		if (i > 0)
		{
			if (vals[i] != vals[i-1])
			{
				keyvalue = vals[i];
			}
			else
			{
				valid_cnt++;
			}
		}

		if (i == 0)
		{
			keyvalue = vals[i];//the first read value.
		}
		//KADC_ERROR("##%s, KEY_ADC_VALUE value[%d]=[%d],valid_cnt=[%d]##\n", __FUNCTION__, i, vals[i], valid_cnt);

		if (valid_cnt >= 2)//if the value is indentical while continuous read 3 times, we think it is valid.
			break;
	}

	if(kadc_version)
	{
		key_idx = keyadc_key_idx_new(keyvalue);
	}
	else
	{
		key_idx = keyadc_key_idx(keyvalue);
	}

#if 0
	if(key_idx)
	{
		KADC_ERROR("##%s, key_idx=[%d] keyvalue=%d##\n",__FUNCTION__,key_idx,keyvalue);
	}
#endif

	return key_idx;
    #endif
}

#ifdef SUPPORT_KADC_INT
static irqreturn_t keyadc_interrupt_handler_new(int irq, mt_void *dev_id)
{
	mt_u32 regvalue = 0;
	mt_u32 keyvalue = 0;
	static u32 start_ticks = 0;
	u32 val = 0;
	u8 key_idx = 0;
	u8 i = 0, valid_cnt = 0;
	u8 vals[KADC_PLUUING_CNT] = {0};
	//printk("Keyadc_Isr.\n");

#if 1
	if(time_after(start_ticks + msecs_to_jiffies(100), jiffies))
	{
#if 0
		KADC_ERROR("##startticks=[%ld], msecs_to_jiffies(300)=[%ld],jiffies=[%ld]##\n",
					start_ticks, msecs_to_jiffies(300),jiffies);
		KADC_ERROR(":%s, %d, ingore nearly interrupt\n", __FUNCTION__, __LINE__);
#endif
		regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_INT);

		//int status
		if(regvalue & 0x1)
		{
			//clear int status
			regvalue |= (0x1 << 0);
			HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);
		}
		return IRQ_HANDLED;
	}
#endif

	start_ticks = jiffies;

	for (i = 0; i < KADC_PLUUING_CNT; i++)
	{
		vals[i] = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_VALUE);
		if (i > 0)
		{
			if (vals[i] != vals[i-1])
				keyvalue = vals[i];
			else
				valid_cnt++;
		}

		if (i == 0)
		{
			keyvalue = vals[i];
		}
		//KADC_ERROR("##%s, KEY_ADC_VALUE value[%d]=[%d],valid_cnt=[%d]##\n",__FUNCTION__,i, vals[i],valid_cnt);

		if (valid_cnt >= 4)
		{
			break;
		}
	}

	key_idx = keyadc_key_idx_new(keyvalue);

	if(key_idx > 0)
	{
		//do something
		keyled_keyfifo_keyadc_put(key_idx, MT_UNF_KEY_STATUS_DOWN);
	}

	//clear int status
	regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_INT);
	//int status
	if(regvalue & 0x1)
	{
		//clear int status
		regvalue |= (0x1 << 0);
		HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);
	}

	return IRQ_HANDLED;
}
static irqreturn_t keyadc_interrupt_handler(int irq, mt_void *dev_id)
{
	mt_u32 regvalue = 0;
	mt_u32 keyvalue = 0;
	static u32 start_ticks = 0;
	u32 val = 0;
	u8 key_idx = 0;
	u8 i = 0, valid_cnt = 0;
	u8 vals[KADC_PLUUING_CNT] = {0};

	//printk("Keyadc_Isr.\n");

#if 1
	if(time_after(start_ticks + msecs_to_jiffies(300), jiffies))
	{
#if 0
		KADC_ERROR("##startticks=[%ld], msecs_to_jiffies(300)=[%ld],jiffies=[%ld]##\n",
			start_ticks, msecs_to_jiffies(300),jiffies);
		KADC_ERROR(":%s, %d, ingore nearly interrupt\n", __FUNCTION__, __LINE__);
#endif
		regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_INT);

		//int status
		if(regvalue & 0x1)
		{
			//clear int status
			regvalue |= (0x1 << 0);
			HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);
		}

		return IRQ_HANDLED;
	}
#endif

	start_ticks = jiffies;

	for (i = 0; i < KADC_PLUUING_CNT; i++)
	{
		vals[i] = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_VALUE);
		if (i > 0)
		{
			if (vals[i] != vals[i-1])
				keyvalue = vals[i];
			else
				valid_cnt++;
		}

		if (i == 0)
		{
			keyvalue = vals[i];
		}

		//KADC_ERROR("##%s, KEY_ADC_VALUE value[%d]=[%d],valid_cnt=[%d]##\n",__FUNCTION__,i, vals[i],valid_cnt);
		if (valid_cnt >= 4)
		{
			break;
		}
	}

	key_idx = keyadc_key_idx(keyvalue);

	if(key_idx > 0)
	{
		//do something
		keyled_keyfifo_keyadc_put(key_idx, MT_UNF_KEY_STATUS_DOWN);
	}

	//clear int status
	regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_INT);
	//int status
	if(regvalue & 0x1)
	{
		//clear int status
		regvalue |= (0x1 << 0);
		HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);
	}

	return IRQ_HANDLED;
}
#endif


static MT_VOID keyadc_init(void *dev)
{
	mt_u32 regvalue = 0;

	//pinmux
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL2, 1);
	drv_gpio_io_enable(AO_GPIO_2, GPIO_MASK_ENABLE);
	drv_gpio_set_dir(AO_GPIO_2, GPIO_DIR_INPUT);

	kadc_version = 1;

	//Kadc reset ...
	//default value: 0x2d00
	//bit[8-14]     :sample frequency, 5bit0  corresponds to highest sampling frequency.

	regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_CFG);
	regvalue &= ~(1<<1); //clear pwr
	//regvalue |= (1 << 8);
	//WRITE_REG((volatile mt_u32 *)KEY_ADC_CFG, regvalue);
	regvalue &= ~(1 << 8);// clear reset
	HAL_PUT_U32((volatile mt_u32*)KEY_ADC_CFG, regvalue);


	//enable kadc
	regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_CFG);
	regvalue |= (1 << 4);
	HAL_PUT_U32((volatile mt_u32*)KEY_ADC_CFG, regvalue);
	//disable kadc INT, default use timer to get kadc value.
	regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_INT);
	regvalue &= ~(1 << 4);
	HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);
	//clear interrupt
	regvalue |= (1 << 0);
	HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);

#ifdef SUPPORT_KADC_INT
	int ret = 0;
	//enable INT
	regvalue = HAL_GET_U32((volatile mt_u32 *)KEY_ADC_INT);
	regvalue |= (1 << 4);
	HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);

	//clear interrupt status
	regvalue |= (1 << 0);
	HAL_PUT_U32((volatile mt_u32*)KEY_ADC_INT, regvalue);

    if (0 == flag_kadc_init) {
        keyled_keyfifo_keyadc_init();

		if(kadc_version)
		    ret = request_irq(IR_IRQ_KEYADC, (irq_handler_t)keyadc_interrupt_handler_new,  IRQF_TRIGGER_RISING, "mt_keyadc_irq", MT_NULL);
		else
		    ret = request_irq(IR_IRQ_KEYADC, (irq_handler_t)keyadc_interrupt_handler,  IRQF_TRIGGER_RISING, "mt_keyadc_irq", MT_NULL);
    }
	if (ret != MT_SUCCESS) {
	    printk("register keyadc INT failed  %d.\n", ret);
	    return MT_FAILURE;
	}
#endif
    flag_kadc_init = 1;
}

static MT_VOID keyadc_deinit(MT_VOID)
{
    if (flag_kadc_init) {
        #ifdef SUPPORT_KADC_INT
        free_irq(IR_IRQ_KEYADC, NULL);
        #endif
        flag_kadc_init = 0;
    }
}

MT_U8 kadc_get_type(MT_VOID)
{
	return klinfo.kadc_type;
}

MT_VOID keyled_keyadc_attach(keyled_info_s *klinfo)
{
	if (klinfo)
	{
		klinfo->type |= MT_UNF_KEYLED_TYPE_KEYADC;
		klinfo->kl_ops.keyled_init = keyadc_init;
		klinfo->kl_ops.display = keyadc_display;
		klinfo->kl_ops.get_keyval = keyadc_get_keyval;
		klinfo->kl_ops.release= keyadc_deinit;
		klinfo->idle_keyval = 0;
		klinfo->is_attach = 1;
	}
}

