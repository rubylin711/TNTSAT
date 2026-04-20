/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>
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
#include <linux/mutex.h>
//#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/jiffies.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <asm/atomic.h>
#include <asm/delay.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/clk.h>
#include <linux/i2c.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_dma.h"
#include "mt_module_debug.h"
#include "drv_sys_misc.h"
#include "drv_i2c_ioctl.h"
#include "drv_gpio_ioctl.h"
#include "mt_drv_pinctrl.h"
#include "mt_drv_clock.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_mach/clock.h"
#include <linux/i2c_mt_symphony.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/irq.h"

#elif defined(CONFIG_MT_CHIP_ARIA)
#include <linux/i2c-aria.h>
#endif

static mt_device_s g_I2cRegisterData;
static int open_count = 0;

#define PROC_NAME_LEN (8)
#define MT_I2C_DEBUG	0
//#define SYMPHONY6_CHIP_TEST			1

#if 0
#define R_PINMUX_ADDR1	0xBF15B400
#define R_PINMUX_ADDR2	0xBF15B404
#define R_PINMUX_SWPIN1_ADDR	0xBF13C004
#define R_PINMUX_SWPIN2_ADDR	0xBF13C008
#endif
#define I2C_FP_ID		(2)
#define I2C_GPIO_ID		(9)
#define I2C_GPIO_DELAY	(150)
#define MAX_SENDBUFF	(128)

extern mt_s32 mt_drv_module_unregister(mt_u32 u32ModuleID);
extern mt_s32 mt_drv_module_register(mt_u32 u32ModuleID, const mt_u8* pu8ModuleName, mt_void* pFunc);

mt_s32 mt_osal_snprintf(mt_char *pszstr, mt_size_t ullen, const mt_char *pszformat, ...);

static unsigned char sendbuf[MAX_SENDBUFF];
static unsigned char recebuf[MAX_SENDBUFF];
static unsigned char databuf[MAX_SENDBUFF];
static i2c_gpio_t g_I2cGpio;
static struct mutex g_i2c_lock;


#if defined CONFIG_MT_FPGA || defined(SYMPHONY6_CHIP_TEST)

#define PROC_PARAM_MAXLEN (64)

typedef struct i2c_test
{
  mt_u32 i2c_id;
  mt_u8 irq[4];
} i2c_test_t;

static i2c_test_t g_i2c_test = {0};
#endif


static void i2c_clk_reset(mt_u8 id);



void GPIO_I2cStart(void)
{
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	drv_gpio_set_dir(g_I2cGpio.sda_gpio_no, GPIO_DIR_OUTPUT);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	drv_gpio_set_dir(g_I2cGpio.scl_gpio_no, GPIO_DIR_OUTPUT);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no, GPIO_VALUE_LOW_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no, GPIO_VALUE_LOW_LEVEL);
}

void GPIO_I2cStop(void)
{
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no, GPIO_VALUE_LOW_LEVEL);
	drv_gpio_set_dir(g_I2cGpio.sda_gpio_no, GPIO_DIR_OUTPUT);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_dir(g_I2cGpio.sda_gpio_no, GPIO_DIR_INPUT);
}

void GPIO_I2cWrByte(mt_u8 dat)
{
	mt_u8 i = 0;
	drv_gpio_set_dir(g_I2cGpio.sda_gpio_no, GPIO_DIR_OUTPUT);
	for( i = 0; i != 8; i++ )
	{
		if( dat & 0x80 )
		{
		    drv_gpio_set_value(g_I2cGpio.sda_gpio_no,GPIO_VALUE_HIGH_LEVEL);
		}
		else
		{
		    drv_gpio_set_value(g_I2cGpio.sda_gpio_no, GPIO_VALUE_LOW_LEVEL);
		}
		udelay(I2C_GPIO_DELAY);
		drv_gpio_set_value(g_I2cGpio.scl_gpio_no,GPIO_VALUE_HIGH_LEVEL);
		dat <<= 1;
		udelay(I2C_GPIO_DELAY);
		drv_gpio_set_value(g_I2cGpio.scl_gpio_no, GPIO_VALUE_LOW_LEVEL);
	}
	drv_gpio_set_dir(g_I2cGpio.sda_gpio_no, GPIO_DIR_INPUT);
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no, GPIO_VALUE_LOW_LEVEL);
}

mt_u8  GPIO_I2cRdByte(void)
{
	mt_u8 dat = 0,i = 0;
        gpio_value_e gpioval = 0;
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	drv_gpio_set_dir(g_I2cGpio.sda_gpio_no, GPIO_DIR_INPUT);
	dat = 0;
	for( i = 0; i != 8; i++ )
	{
		udelay(I2C_GPIO_DELAY);
		drv_gpio_set_value(g_I2cGpio.scl_gpio_no,GPIO_VALUE_HIGH_LEVEL);
		udelay(I2C_GPIO_DELAY);
		dat <<= 1;
              drv_gpio_get_value(g_I2cGpio.sda_gpio_no, &gpioval);
		if(gpioval)
		{
			dat++;
		}
		drv_gpio_set_value(g_I2cGpio.scl_gpio_no, GPIO_VALUE_LOW_LEVEL);
	}
	drv_gpio_set_value(g_I2cGpio.sda_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no,GPIO_VALUE_HIGH_LEVEL);
	udelay(I2C_GPIO_DELAY);
	drv_gpio_set_value(g_I2cGpio.scl_gpio_no, GPIO_VALUE_LOW_LEVEL);
	return dat;
}

int GPIO_I2cWrite(mt_u8 *buff,mt_u8 len)
{
	mt_u8 i = 0;
	GPIO_I2cStart();
	for(i = 0;i<len;i++)
	{
		GPIO_I2cWrByte(buff[i]);
	}
	GPIO_I2cStop();
	return 0;
}


int GPIO_I2cRead(mt_u8 *buff,mt_u8 len)
{
	mt_u8 i = 0;

	GPIO_I2cStart();
	GPIO_I2cWrByte(buff[0]);
	for(i = 0;i<len;i++)
	{
		buff[i] = GPIO_I2cRdByte();
	}
	GPIO_I2cStop();

	return 0;
}
#if 0
static void GPIO_fp_enable(void)
{
	unsigned int val = 0;
	val = readl(0xbf15b400);
	val &= ~0xff;
	val |= 0x11;
	writel(val,0xbf15b400);
}
#endif
extern void symphony_setpinmux(unsigned int index, unsigned int offset, unsigned int len, unsigned int value);
void i2c_fp_enable(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	//pinmux
	//symphony4 config pinmux in uboot_symphony_pinctrl.env, so commit this config
	//symphony_setpinmux(AO_PIN0, 0, 4, 0x03);
	//symphony_setpinmux(AO_PIN1, 0, 4, 0x03);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL0, 3);
	mt_pinctrl_set_function(INDEX_AO_PIN_CTRL1, 3);
#endif
}

static void i2c_clk_reset(mt_u8 id)
{
	switch (id)
		{
		case 0:
			printk(KERN_ERR "i2c0 clk reset.");
			mt_clk_reset(MT_CLK_I2C0);
			break;
		case 1:
			printk(KERN_ERR "i2c1 clk reset.");
			mt_clk_reset(MT_CLK_I2C1);
			break;
		case 2:
			printk(KERN_ERR "fp_i2c clk reset.");
			mt_clk_reset(MT_CLK_AO_FPI2C);
			break;
		default:
			break;
		}
}


int i2c_read_fp(i2c_data_t data)
{
	int ret = 0;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;

	i2c = i2c_get_adapter(data.i2c_id);
	msg->addr = 0;
	msg->flags = I2C_M_STD_RD;
	msg->buf = (__u8 *)data.p_data;
	msg->len = data.data_len;
	//mt_msg.rlen = 1;
	//mt_msg.wlen = 1;
	//msg->buf[0] = 0x4f;
	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		MT_ERR_I2C("ret=%d \n",ret);
		return -1;
	}
	//MT_INFO_I2C("%s %d data = %02x\n",__FUNCTION__, __LINE__,data.p_data[0]);

	return 0;
}



int i2c_write_fp(i2c_data_t data)
{
	int ret = 0;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;

	i2c = i2c_get_adapter(data.i2c_id);
	msg->addr = 0;
	msg->flags = I2C_M_TEN;
	msg->buf = (__u8 *)data.p_data;
	msg->len = data.data_len;

	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		MT_ERR_I2C("ret=%d \n",ret);
		return -1;
	}

	return 0;
}

EXPORT_SYMBOL(i2c_fp_enable);

int i2c_read_common(i2c_data_ex_t data_ex)
{
	int i,ret = 0;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	struct mt_i2c_dev *dev;

	if(data_ex.data_len > (SEND_MAX_BUFF-1))
	{
		return -1;
	}
    #if MT_I2C_DEBUG
	printk(KERN_EMERG "%s,%d.in id=%x,devaddr=%x reg=%x reglen=%x,dlen=%x\n",__FUNCTION__,__LINE__,
        data_ex.i2c_id,data_ex.dev_addr,data_ex.reg_addr,data_ex.reg_count,data_ex.data_len);
    #endif
	i2c = i2c_get_adapter(data_ex.i2c_id);
	dev = i2c_get_adapdata(i2c);
	msg->addr = data_ex.dev_addr;
	msg->flags = data_ex.flags;         //I2C_M_SEQ_RD|I2C_M_SALVE_TYPE;
	msg->buf = recebuf;
	msg->len = 1;
	mt_msg.rlen = data_ex.data_len;
	mt_msg.wlen = data_ex.reg_count;
	mt_msg.slave_type = data_ex.types;  //I2C_SLAVE_DEV_SOC_EXTER;

    for(i=0; i<data_ex.reg_count; i++){
        msg->buf[i] = ((data_ex.reg_addr>>((data_ex.reg_count-i-1)*8))&0xff);       
        #if MT_I2C_DEBUG
        printk("i2c.r.reg[%d]:0x%02x\n",i,msg->buf[i]);
        #endif
    }
	ret = i2c_transfer(i2c, msg, 1);
    #if MT_I2C_DEBUG
	printk(KERN_EMERG "%s,%d. ret=%x,devaddr=%x reg=%x \n",__FUNCTION__,__LINE__,ret,msg->addr, msg->len);
    #endif
	if (ret < 0) {
		MT_INFO_I2C("ret=%d devaddr=%x reg=%x \n",ret, data_ex.dev_addr,data_ex.reg_addr);
		return -1;
	}
    #if MT_I2C_DEBUG
	for(ret = 0; ret<data_ex.data_len; ret++) {
        printk("i2c.r.dat[%d]:0x%02x\n",ret,msg->buf[ret]);
	}
    #endif
	return 0;
}

int i2c_write_common(i2c_data_ex_t dataex)
{
	int ret = 0,i = 0;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;

	if(dataex.data_len > (SEND_MAX_BUFF-1))
	{
		return -1;
	}
    #if MT_I2C_DEBUG
	printk(KERN_EMERG "%s,%d.in id=%x,devaddr=%x reg=%x reglen=%x,dlen=%x\n",__FUNCTION__,__LINE__,
        dataex.i2c_id,dataex.dev_addr,dataex.reg_addr,dataex.reg_count,dataex.data_len);
    #endif
	i2c = i2c_get_adapter(dataex.i2c_id);
	msg->addr = dataex.dev_addr;
	msg->flags = dataex.flags;          //I2C_M_TEN | I2C_M_SALVE_TYPE;
	msg->buf = sendbuf;
	msg->len = dataex.reg_count+dataex.data_len;

    mt_msg.rlen = 0;
    mt_msg.wlen = msg->len;
    mt_msg.slave_type = dataex.types;   //I2C_SLAVE_DEV_SOC_EXTER;
     
    for(i=0; i<dataex.reg_count; i++) {
        msg->buf[i] = ((dataex.reg_addr>>((dataex.reg_count-i-1)*8))&0xff);
        #if MT_I2C_DEBUG
        printk("i2c.w.reg[%d]:0x%02x\n",i,msg->buf[i]);
        #endif
    }

	for(i = 0;i<dataex.data_len;i++) {
		msg->buf[i+dataex.reg_count] = ((__u8 *)dataex.p_data)[i];
        #if MT_I2C_DEBUG
		printk("i2c.w.dat[%d]:0x%02x\n",i,dataex.p_data[i]);
        #endif
	}

	ret = i2c_transfer(i2c, msg, 1);
    #if MT_I2C_DEBUG
	printk(KERN_EMERG "%s,%d. ret=%x,devaddr=%x reg=%x \n",__FUNCTION__,__LINE__,ret,msg->addr, msg->len);
    #endif
	if (ret < 0) {
		return -1;
	}
	return 0;
}
EXPORT_SYMBOL(i2c_read_common);
EXPORT_SYMBOL(i2c_write_common);

static int i2c_read(i2c_data_t data)
{
	int ret = 0;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	struct mt_i2c_dev *dev;

	if(data.data_len > (SEND_MAX_BUFF-1))
	{
		return -1;
	}

	i2c = i2c_get_adapter(data.i2c_id);
	dev = i2c_get_adapdata(i2c);

	if(NULL == dev)
	{
	  MT_INFO_I2C("\n%s.%d ERROR! No i2c dev.\n",__FUNCTION__,__LINE__);
	  return -1;
	}

	msg->addr = data.dev_addr;
	msg->flags = I2C_M_STD_RD;
	msg->buf = recebuf;
	msg->len = 1;
	mt_msg.rlen = data.data_len;
	mt_msg.wlen = 1;
	//msg->buf[0] = (mt_u8)data.reg_addr;
	msg->buf[0] = data.reg_addr;
	ret = i2c_transfer(i2c, msg, 1);
#if MT_I2C_DEBUG
	MT_INFO_I2C("ret=%d devaddr=%x reg=%x \n",ret, data.dev_addr,data.reg_addr);
#endif
	if (ret < 0)
	{
		MT_INFO_I2C("ret=%d devaddr=%x reg=%x \n",ret, data.dev_addr,data.reg_addr);
		return -1;
	}
#if MT_I2C_DEBUG
	MT_INFO_I2C(" read %d bytes data:",data.data_len);
	for(ret = 0;ret<data.data_len;ret++)
	{
		MT_INFO_I2C("0x%02x ",msg->buf[ret]);
	}
	printk("\n");
#endif
	return 0;
}


static int i2c_write(i2c_data_t data)
{
	int ret = 0;
	struct mt_i2c_msg mt_msg = {0,};
	struct i2c_msg *msg = &mt_msg.msg;
	struct i2c_adapter *i2c = NULL;
	int i = 0;

	if(data.data_len > (SEND_MAX_BUFF-1))
	{
		return -1;
	}
	memset(sendbuf,0,MAX_SENDBUFF);
	i2c = i2c_get_adapter(data.i2c_id);
	msg->addr = data.dev_addr;
	msg->flags = I2C_M_TEN;
	msg->buf = sendbuf;
	msg->len = 1 + data.data_len;
	msg->buf[0] = data.reg_addr;

#if MT_I2C_DEBUG
	printk(KERN_ERR "ret=%d devaddr=%x reg=%x \n",ret, data.dev_addr, data.reg_addr);
#endif
	for(i=1; i<msg->len; i++)
	{
		msg->buf[i] = ((__u8 *)data.p_data)[i-1];
		#if MT_I2C_DEBUG
		printk(KERN_ERR "0x%02x ",msg->buf[i]);
		#endif
	}

	ret = i2c_transfer(i2c, msg, 1);
	if (ret < 0)
	{
		printk(KERN_ERR "ret=%d devaddr=%x reg=%x \n",ret, data.dev_addr, data.reg_addr);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL(i2c_read_fp);
EXPORT_SYMBOL(i2c_write_fp);

extern void mt_i2c_isrdefaulton_onoff(struct i2c_adapter *adapter, int on);

void i2c_isr_onoff(int i2_id, int on)
{ 
    struct i2c_adapter *i2cadp = i2c_get_adapter(i2_id); 
    if (i2cadp) { 
        mt_i2c_isrdefaulton_onoff(i2cadp, on);
    } else {
        printk(KERN_ERR "error i2cadp = NULL \n");
    }
}
EXPORT_SYMBOL(i2c_isr_onoff);

static long i2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	i2c_data_t data;
	i2c_rate_t rate;
	i2c_gpio_t i2cgpio;
	int ret = 0,i = 0;
	mt_u32 i2c_id = 0;
	switch(cmd)
	{
		case CMD_I2C_READ:
#if MT_I2C_DEBUG
			printk(KERN_ERR "--------------\n");
#endif
			if (copy_from_user(&data, argp, sizeof(i2c_data_t)) != 0)
			{
				return -EIO;
			}
			if(data.data_len > (SEND_MAX_BUFF-1))
			{
				return -EPERM;
			}
			mutex_lock(&g_i2c_lock);
			memset(recebuf,0,MAX_SENDBUFF);
			if((data.i2c_id == I2C_GPIO_ID) && (g_I2cGpio.b_used))
			{
				recebuf[0] = data.reg_addr;
				ret = GPIO_I2cRead(recebuf,data.data_len);
			}
			else if(I2C_FP_ID == data.i2c_id)
			{//fp i2c for unf
				i2c_data_t i2c_fp_r = {0};
				int i = 0;

				if (copy_from_user(recebuf, (const void __user *)data.p_data, 1) != 0)
				{//cmd
					return -EIO;
				}
				i2c_fp_r.i2c_id = data.i2c_id;
				i2c_fp_r.p_data = (u64)recebuf;
				i2c_fp_r.data_len = data.data_len;
				ret = i2c_read_fp(i2c_fp_r);
				for (i=0; i<data.data_len; i++) {
					recebuf[i]=recebuf[i+1];
				}
			}
			else
			{
				ret = i2c_read(data);
			}

			if (copy_to_user((void __user *)data.p_data, recebuf, data.data_len) != 0)
			{
				mutex_unlock(&g_i2c_lock);
				return -EIO;
			}
			mutex_unlock(&g_i2c_lock);
			if (copy_to_user(argp, &data, sizeof(i2c_data_t)) != 0)
			{
				return -EIO;
			}

			break;
		case CMD_I2C_WRITE:
#if MT_I2C_DEBUG
			printk(KERN_ERR "--------------\n");
#endif
			if (copy_from_user(&data, argp, sizeof(i2c_data_t)) != 0)
			{
				return -EIO;
			}
			if(data.data_len > (SEND_MAX_BUFF-1))
			{
				return -EPERM;
			}
			mutex_lock(&g_i2c_lock);
			if (copy_from_user(databuf, (const void __user *)data.p_data, data.data_len) != 0)
			{
				mutex_unlock(&g_i2c_lock);
				return -EIO;
			}
			data.p_data = (u64)databuf;
			if((data.i2c_id == I2C_GPIO_ID) && (g_I2cGpio.b_used))
			{
				memset(sendbuf,0,MAX_SENDBUFF);
				sendbuf[0] = data.reg_addr;
				for(i = 0;i<data.data_len;i++)
				{
					sendbuf[i+1]=((mt_u8 *)data.p_data)[i];
				}
				ret = GPIO_I2cWrite(sendbuf,data.data_len+1);
			}
			else if(I2C_FP_ID == data.i2c_id)
			{//fp i2c for unf
				/*
				i2c_data.i2c_id = 2;
				buff[0] = ((u8)(cmd >> 7) & 0x3E) | 0x40;
				buff[1] = (u8)cmd;
				i2c_data.p_data = buff;
				i2c_data.data_len = 2;*/
				ret = i2c_write_fp(data);
			}
			else
			{
				ret = i2c_write(data);
			}
			mutex_unlock(&g_i2c_lock);
			break;
        case CMD_I2C_READ_EX:
            {
                i2c_data_ex_t dataex;
                #if MT_I2C_DEBUG
                MT_INFO_I2C("------i2c.r.ex--------\n");
                #endif
                if (copy_from_user(&dataex, argp, sizeof(i2c_data_ex_t)) != 0) {
                    return -EIO;
                }
                if((I2C_GPIO_ID == dataex.i2c_id) || (I2C_FP_ID == dataex.i2c_id)) {
                    return -EPERM;
                }
                if (dataex.data_len > (SEND_MAX_BUFF-1)) {
                    return -EPERM;
                }
                mutex_lock(&g_i2c_lock);
                memset(recebuf,0,MAX_SENDBUFF);
                ret = i2c_read_common(dataex);
                if (copy_to_user((void __user *)dataex.p_data, recebuf, dataex.data_len) != 0) {
                    mutex_unlock(&g_i2c_lock);
                    return -EIO;
                }
                mutex_unlock(&g_i2c_lock);
                if (copy_to_user(argp, &dataex, sizeof(i2c_data_ex_t)) != 0) {
                    return -EIO;
                }
            }
            break;
        case CMD_I2C_WRITE_EX:
            {
                i2c_data_ex_t dataex;
                #if MT_I2C_DEBUG
                MT_INFO_I2C("-----i2c.w.ex---------\n");
                #endif
                if (copy_from_user(&dataex, argp, sizeof(i2c_data_ex_t)) != 0) {
                    return -EIO;
                }
                if((I2C_GPIO_ID == dataex.i2c_id) || (I2C_FP_ID == dataex.i2c_id)) {
                    return -EPERM;
                }
                if(dataex.data_len > (SEND_MAX_BUFF-1)) {
                    return -EPERM;
                }
                mutex_lock(&g_i2c_lock);
                if (copy_from_user(databuf, (const void __user *)dataex.p_data, dataex.data_len) != 0) {
                    mutex_unlock(&g_i2c_lock);
                    return -EIO;
                }
                dataex.p_data = (u64)databuf;
                ret = i2c_write_common(dataex);
                mutex_unlock(&g_i2c_lock);
            }
            break;
		case CMD_I2C_SET_RATE:
			{
				struct i2c_adapter *i2c = NULL;
				struct mt_i2c_dev *dev = NULL;
#if MT_I2C_DEBUG
				printk(KERN_ERR "--------------\n");
#endif
				if (copy_from_user(&rate, argp, sizeof(i2c_rate_t)) != 0)
				{
					return -EIO;
				}
				i2c = i2c_get_adapter(rate.i2c_id);
				dev = i2c_get_adapdata(i2c);
				dev->new_busclk_khz = (rate.i2c_rate/1000);
				i2c->algo->functionality(i2c);
				dev->new_busclk_khz = 0;
			}
			break;
		case CMD_I2C_CONFIG:
#if MT_I2C_DEBUG
			printk(KERN_ERR "--------------\n");
#endif
			if (copy_from_user(&i2cgpio, argp, sizeof(i2c_gpio_t)) != 0)
			{
				return -EIO;
			}
			if(g_I2cGpio.b_used)
			{
				ret = - EUSERS;
				break;
			}
			g_I2cGpio.b_used = 1;
			g_I2cGpio.scl_gpio_no = i2cgpio.scl_gpio_no;
			g_I2cGpio.sda_gpio_no = i2cgpio.sda_gpio_no;
			i2cgpio.i2c_id = g_I2cGpio.i2c_id;
			if (copy_to_user(argp, &i2cgpio, sizeof(i2c_gpio_t)) != 0)
			{
				return -EIO;
			}
			break;
		case CMD_I2C_DESTROY:
#if MT_I2C_DEBUG
			printk(KERN_ERR "--------------\n");
#endif
			if (copy_from_user(&i2cgpio, argp, sizeof(i2c_gpio_t)) != 0)
			{
				return -EIO;
			}

			if(g_I2cGpio.b_used)
			{
				if(i2cgpio.i2c_id == g_I2cGpio.i2c_id)
				{
					g_I2cGpio.b_used = 0;
					g_I2cGpio.scl_gpio_no = 0;
					g_I2cGpio.sda_gpio_no = 0;
				}
				else
				{
					ret = -EINVAL;
				}
			}
			else
			{
				ret = -ENXIO;
			}
			break;
		case CMD_I2C_RESET:
			{
				if (copy_from_user(&i2c_id, argp, sizeof(i2c_id)) != 0)
				{
					return -EIO;
				}

				i2c_clk_reset(i2c_id);

			}
			break;


		default:
			ret = -EINVAL;
			break;
	}
	//    MT_INFO_I2C("[%s %d] ret = %d\n", __FUNCTION__, __LINE__, ret);
	return ret;
}

static int i2c_open(struct inode *inode, struct file *file)
{
    open_count ++;
#if MT_I2C_DEBUG
    MT_INFO_I2C("open_count %d\n",open_count);
#endif

    return 0;
}

static int i2c_release(struct inode *inode, struct file *file)
{

    open_count --;
#if MT_I2C_DEBUG
    MT_INFO_I2C("[%s %d]open_count %d\n", __FUNCTION__, __LINE__, open_count);
#endif

    return 0;
}

static int i2c_proc(struct seq_file *p, mt_void *v)
{
    struct i2c_adapter *i2c = NULL;
    struct mt_i2c_dev *dev = NULL;
    //i2c_gpio_t *i2c_gpio = (i2c_gpio_t *)v;
    int idx = 0;
    PROC_PRINT(p, "------------Montage-LZ I2C Info---------------\n");
    PROC_PRINT(p, "No.\t\tRate\n");
    idx = 0;
    while(1)
    {
        i2c = i2c_get_adapter(idx);
        if(i2c == NULL)
            break;
        dev = i2c_get_adapdata(i2c);
        PROC_PRINT(p, "%d \t\t%d\n", idx, dev->busclk_khz);
        idx++;
    }
    if(g_I2cGpio.b_used)
    {
         PROC_PRINT(p, "------------Montage-LZ GPIO Simulate I2C Info---------------\n");
         PROC_PRINT(p, "ID.\tSDA_IO\tSCL_IO\n");
         PROC_PRINT(p, "%d \t%d\t%d\n", g_I2cGpio.i2c_id, g_I2cGpio.sda_gpio_no, g_I2cGpio.scl_gpio_no);
    }
    return 0;
}

#if defined CONFIG_MT_FPGA || defined(SYMPHONY6_CHIP_TEST)

static void i2c_proc_help(void)
{
	printk(KERN_ERR "\nusage:\n");
	printk(KERN_ERR "help:\n");
	printk(KERN_ERR "\t echo h > /proc/msp/i2c\n");

	printk(KERN_ERR "i2c reset cmd:\n");
	printk(KERN_ERR "\t echo R,a=i2c_id > /proc/msp/i2c\n");
	printk(KERN_ERR "\t\t example:echo R,a=0 > /proc/msp/i2c\n");

	printk(KERN_ERR "i2c dump reg value:\n");
	printk(KERN_ERR "\t echo d,a=i2c_id > /proc/msp/i2c\n");
	printk(KERN_ERR "\t\t example:echo d,a=0 > /proc/msp/i2c\n");

	printk(KERN_ERR "i2c read cmd:\n");
	printk(KERN_ERR "\t echo r,a=?[1:enable interrupt],b=?[i2c id],c=?[dev_addr],d=?[reg_addr],e=?[reg cnt],f=?[data lend] > /proc/msp/i2c\n");
	printk(KERN_ERR "\t\t example:echo r,a=0,b=0,c=198,d=288,e=1,f=2 > /proc/msp/i2c\n");

	printk(KERN_ERR "i2c write cmd:\n");
	printk(KERN_ERR "\t echo r,a=?[1:enable interrupt],b=?[i2c id],c=?[dev_addr],d=?[reg_addr],e=?[reg cnt],f=?[data lend], g=?[value,value,...] > /proc/msp/i2c\n");
	printk(KERN_ERR "\t\t example:echo w,a=0,b=0,c=198,d=288,e=1,f=2,g=27,52 > /proc/msp/i2c\n");

}


irqreturn_t test_i2c_interrupt_handler(int irq, void *dev_id)
{

  struct i2c_adapter *i2c = NULL;
  struct mt_i2c_dev *i2c_dev;
  u8 val = 0;
  u8 val_old = 0;
  int reg = 0x14;//R_I2C_CR

  i2c_test_t *i2c_test = (i2c_test_t *)dev_id;
  mt_u32 i2_cid = i2c_test->i2c_id;

  i2c = i2c_get_adapter(i2_cid);
  i2c_dev = i2c_get_adapdata(i2c);

  val_old = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  val = val_old | 0x08;//I2C_CR_IACK
  HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), val);
  //MT_INFO_I2C("\nirq:%d R_I2C_CR:0x%x -> 0x%x clear IFLAG_QUERY.\n",irq,val_old,val);
  MT_INFO_I2C("\nirq:%d R_I2C_CR:0x%x -> 0x%x clear IFLAG_QUERY.\n",irq,val_old,val);

  return IRQ_HANDLED;
}

static mt_s32 test_i2c_irq(mt_u8 i2c_id,mt_u8 enable)
{
  mt_s32 ret = 0;
  unsigned int irq = 0;
  struct i2c_adapter *i2c = NULL;
  struct mt_i2c_dev *i2c_dev;
  u8 val = 0;
  int reg = 0x10;//R_I2C_CTR

  switch (i2c_id)
  {
    case 0:
    {
      irq = IRQ_I2C0_ID;
      break;
    }
    case 1:
    {
      irq = IRQ_I2C1_ID;
      break;
    }
/*
    case 2:
    {
      irq = IRQ_I2C_FB_ID;
      break;
    }
    case 3:
    {
      irq = IRQ_I2C_HDMI_CFG_ID;
      break;
    }
*/
    default:
      MT_INFO_I2C("\ni2c id error!!!\n");
      return -1;
  }

  g_i2c_test.i2c_id = i2c_id;

  i2c = i2c_get_adapter(i2c_id);
  i2c_dev = i2c_get_adapdata(i2c);
  val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));

  if(enable)
  {
    val |= 0x40;//I2C_CTR_IEN
    HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), val);

    if(0 == g_i2c_test.irq[i2c_id])
    {
      printk(KERN_ERR "\nrequest_irq\n");
      ret = request_irq(irq, test_i2c_interrupt_handler,IRQF_TRIGGER_HIGH,"test_i2c", &g_i2c_test);
      if(0!=ret){
        printk(KERN_ERR "request_irq %d error!!!\n",irq);
        return -1;
      }
      g_i2c_test.irq[i2c_id] = 1;
    }
  }
  else
  {
    val &= 0xbf;//I2C_CTR_IEN
    HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), val);
    if(1 == g_i2c_test.irq[i2c_id])
    {
       printk(KERN_ERR "\nfree_irq\n");
      free_irq(irq, NULL);
      g_i2c_test.irq[i2c_id] = 0;
    }
  }

  return 0;
}

static mt_s32 i2c_test_param_parse(char *in,mt_u32 *out)
{
  mt_u32 len = 0;
  mt_u32 len1 = 0;
  mt_u32 len2 = 0;
  mt_u32 i = 0;
  mt_u32 count = 0;
  mt_u32 index = 0;
  mt_u32 value = 0;
  char *pstart = NULL;
  char *pend = NULL;
  char data[PROC_PARAM_MAXLEN] = {0};
  if(NULL == in)
  {
    return -1;
  }

  pstart = in;
  len = strlen(pstart);
  while(*pstart)
  {
    if('=' == *pstart++)
    {
      count++;
    }
  }

  //MT_INFO_I2C("Param Count:%d\n",count);
  pstart = in;
  pend = in;
  //MT_INFO_I2C("pstart:%s\n",pstart);
  for(i = 0; i<count; i++)
  {
    len1 = 0;
    len2 = 0;
    pstart = strchr(pstart,'=');
    pend = strchr(pstart,',');
    pstart++;
    len1 = strlen(pstart);

    if(pend)
    {
      len2 = strlen(pend);
    }

    len = len1 - len2;

    memset(data,0,PROC_PARAM_MAXLEN);
    strncpy(data,pstart,len);

    if(i != (count -1))
    {
      if(pend)
      {
        pstart = strchr(pend,'=');
      }
    }
    else
    {

    }
    //value = atoi(data);
    sscanf(data,"%u",&value);
    out[i] = value;
    //MT_INFO_I2C("i:%d value:%u\n",i,value);

  }

  if(strstr(in,"w,"))
  {
    index = count;
    pend = pstart;
    count = 0;
    while(*pend)
    {
      if(',' == *pend++)
      {
        count++;
      }
    }

    printk(KERN_ERR "pstart:%s\n",pstart);
    printk(KERN_ERR "w_cnt:%d\n",count);

    for(i = 0; i<count; i++)
    {
      len1 = 0;
      len2 = 0;
      pstart = strchr(pstart,',');
      pstart++;
      pend = strchr(pstart,',');

      len1 = strlen(pstart);

      if(pend)
      {
        len2 = strlen(pend);
      }

      len = len1 - len2;

      memset(data,0,PROC_PARAM_MAXLEN);
      strncpy(data,pstart,len);
      //value = atoi(data);
      sscanf(data,"%u",&value);
      out[index] = value;
      index++;
      //MT_INFO_I2C("w_data[%d]:%u\n",i,value);
    }

  }
  return 0;
}


static void test_i2c_dump_reg(mt_u8 i2c_id)
{
  struct i2c_adapter *i2c = NULL;
  struct mt_i2c_dev *i2c_dev;
  //u8 val = 0;
  u8 val_ctr = 0;//0x10;//R_I2C_CTR
  u8 val_cr = 0;//0x14;//R_I2C_CR
  u8 val_prer_h = 0;//0x18
  u8 val_prer_l = 0;//0x1c
  u8 val_scll_h = 0;//0x20
  u8 val_scll_l = 0;//0x24
  u8 val_hddat_h = 0;//0x28
  u8 val_hddat_l = 0;//0x2c
  u8 val_sudat_h = 0;//0x38
  u8 val_sudat_l = 0;//0x30
  int reg = 0x10;//R_I2C_CTR

  i2c = i2c_get_adapter(i2c_id);
  i2c_dev = i2c_get_adapdata(i2c);
  val_ctr = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x14;
  val_cr = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x18;
  val_prer_h = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x1c;
  val_prer_l = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x20;
  val_scll_h = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x24;
  val_scll_l = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x28;
  val_hddat_h = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x2c;
  val_hddat_l = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x30;
  val_sudat_h = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  reg = 0x34;
  val_sudat_l = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
  printk(KERN_ERR "\nI2C_CTR:0x%x\n",val_ctr);
  printk(KERN_ERR "\nI2C_CR:0x%x\n",val_cr);
  printk(KERN_ERR "\n{PRER_H,PRER_L}={0x%x,0x%x}\n",val_prer_h,val_prer_l);
  printk(KERN_ERR "\n{SCLL_H,SCLL_L}={0x%x,0x%x}\n",val_scll_h,val_scll_l);
  printk(KERN_ERR "\n{HDDAT_H,HDDAT_L}={0x%x,0x%x}\n",val_hddat_h,val_hddat_l);
  printk(KERN_ERR "\n{SUDAT_H,SUDAT_L}={0x%x,0x%x}\n",val_sudat_h,val_sudat_l);
}


static mt_s32 test_i2c_config(mt_u32 *cfg)
{
  mt_u32 i2c_id = 0;
  mt_u32 mode = 0;
  mt_u32 sysclk = 0;
  mt_u32 busclk = 0;
  mt_u32 tmp = 0;

  struct i2c_adapter *i2c = NULL;
  struct mt_i2c_dev *i2c_dev;
  u8 val = 0;
  int val_scll = 0;
  int val_hddat = 0;
  int val_sudat = 0;
  int reg = 0;

  i2c_id = cfg[0];
  mode = cfg[1];
  sysclk = cfg[2];
  busclk = cfg[3];

  i2c = i2c_get_adapter(i2c_id);
  i2c_dev = i2c_get_adapdata(i2c);
  val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));


  printk(KERN_ERR "i2c [id:%u mode:%u sysclk:%u busclk:%u KHZ]\n",i2c_id,mode,sysclk,busclk);
  if(0 == mode)
  {
	printk(KERN_ERR "\nI2C disable[STD MODE 0].\n");
	reg = 0x10;
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), 0x0);

    tmp = ((sysclk / (5000 * busclk)) - 1);
	reg = 0x1c;//R_I2C_PRER_L
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)tmp);
	reg = 0x18;//R_I2C_PRER_H
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)(tmp >> 8));

	printk(KERN_ERR "\nI2C enable.\n");
	reg = 0x10;
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), 0x80);
  }
  else if(1 == mode)
  {
    tmp = cfg[4];
    val_scll = cfg[5];
    val_hddat = cfg[6];
    val_sudat = cfg[7];
    printk(KERN_ERR "\nI2C reg config befor:\n");
    test_i2c_dump_reg(i2c_id);

	printk(KERN_ERR "\nI2C disable[STD MODE 1].\n");
	reg = 0x10;
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), 0x1);


	printk(KERN_ERR "i2c [PRER:%u SCLL:%u HDDAT:%u SUDAT:%u]\n",tmp,val_scll,val_hddat,val_sudat);
    reg = 0x1c;//R_I2C_PRER_L
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)tmp);
	reg = 0x18;//R_I2C_PRER_H
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)(tmp >> 8));

    reg = 0x24;//R_I2C_SCLL_[7:0]
    HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)val_scll);
	reg = 0x20;//R_I2C_SCLL_[15:8]
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)(val_scll >> 8));

	reg = 0x2c;//R_I2C_HDDAT_[7:0]
    HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)val_hddat);
	reg = 0x28;//R_I2C_HDDAT_[15:8]
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)(val_hddat >> 8));

	reg = 0x34;//R_I2C_SUDAT_[7:0]
    HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)val_sudat);
	reg = 0x30;//R_I2C_SUDAT_[15:8]
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)(val_sudat >> 8));

	printk(KERN_ERR "\nI2C reg config after:\n");
    test_i2c_dump_reg(i2c_id);

	printk(KERN_ERR "\nI2C enable.\n");
	reg = 0x10;
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), 0x81);
  }

  return 0;
}

static void test_i2c_reset(mt_u8 i2c_id)
{
	//mt_u32 i2c_id = 0;
	//mt_u32 mode = 0;
	mt_u32 reg = 0;
	mt_u32 val = 0;
	mt_u8 tmp_l = 0;
	mt_u8 tmp_h = 0;

	struct i2c_adapter *i2c = NULL;
	struct mt_i2c_dev *i2c_dev;

	if ((0 != i2c_id) && (1 != i2c_id) && (2 != i2c_id))
	{
		return;
	}

	printk(KERN_ERR "\nbefore reset:\n");
	test_i2c_dump_reg(i2c_id);
	printk(KERN_ERR "\n-------\n");

	printk(KERN_ERR "\nI2C reset test:\n");
	i2c = i2c_get_adapter(i2c_id);
	i2c_dev = i2c_get_adapdata(i2c);
	reg = 0x1c;//R_I2C_PRER_L
	val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
	tmp_l = ~val;
	printk(KERN_ERR "\nold PRER_L:0x%x\n",val);
	reg = 0x18;//R_I2C_PRER_H
	val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
	tmp_h = ~val;
	printk(KERN_ERR "\nold PRER_H:0x%x\n",val);

	reg = 0x1c;//R_I2C_PRER_L
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)tmp_l);
	reg = 0x1c;//R_I2C_PRER_L
	val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
	printk(KERN_ERR "\nnew PRER_L:0x%x\n",val);


	reg = 0x18;//R_I2C_PRER_H
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), (u8)tmp_h);
	val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
	printk(KERN_ERR "\nnew PRER_H:0x%x\n",val);

	printk(KERN_ERR "\ni2c clk reset:\n");
	if (0 == i2c_id)
	{
		printk(KERN_ERR "i2c0 clk reset.");
		mt_clk_reset(MT_CLK_I2C0);
	}
	else if (1 == i2c_id)
	{
		printk(KERN_ERR "i2c0 clk reset.");
		mt_clk_reset(MT_CLK_I2C1);
	}
	else if (2 == i2c_id)
	{
		printk(KERN_ERR "fp_i2c clk reset.");
		mt_clk_reset(MT_CLK_AO_FPI2C);
	}

	reg = 0x1c;//R_I2C_PRER_L
	val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
	printk(KERN_ERR "\ndefault PRER_L:0x%x\n",val);
	reg = 0x18;//R_I2C_PRER_H
	val = HAL_GET_U8((volatile u8 *)(i2c_dev->base + reg));
	printk(KERN_ERR "\ndefault PRER_H:0x%x\n",val);

	printk(KERN_ERR "\nafter reset:\n");
	test_i2c_dump_reg(i2c_id);
	printk(KERN_ERR "\n-------\n");

	printk(KERN_ERR "\ni2c enable\n");
	reg = 0x10;//R_I2C_PRER_L
	HAL_PUT_U8((volatile u8 *)(i2c_dev->base + reg), 0x80);
}


mt_s32 i2c_procwrite(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
	mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
	mt_u32 para_val[PROC_PARAM_MAXLEN] = {0};
	mt_char *param = NULL;
	//mt_u8 *p_data = MT_NULL;
	i2c_data_t data;
	int i = 0;
	int ret = 0;
	mt_u8 irq_enable = 0;
	mt_u8 index = 0;

	if(count > PROC_PARAM_MAXLEN)
	{
		printk(KERN_ERR "write data is too long!\n");
		return -EFAULT;
	}

	if(copy_from_user(ProcPara, buf, count))
	{
		MT_INFO_I2C("write data is too long!\n");
		return -EFAULT;
	}
	ProcPara[PROC_PARAM_MAXLEN-1] = 0;
	param =ProcPara;

	memset(&data,0,sizeof(i2c_data_t));
	i2c_test_param_parse(param,para_val);

	irq_enable = para_val[0];
	data.i2c_id = para_val[1];
	data.dev_addr = para_val[2];
	data.reg_addr = para_val[3];
	data.reg_count = para_val[4];
	data.data_len = para_val[5];

	if(ProcPara[0] == 'r')
	{
		if(data.data_len > (SEND_MAX_BUFF-1))
		{
			return -EPERM;
		}
		mutex_lock(&g_i2c_lock);
		test_i2c_irq(data.i2c_id,irq_enable);
		memset(recebuf,0,MAX_SENDBUFF);
		data.p_data = (u64)recebuf;

		if((data.i2c_id == I2C_GPIO_ID) && (g_I2cGpio.b_used))
		{
			recebuf[0] = data.reg_addr;
			ret = GPIO_I2cRead(recebuf,data.data_len);
		}
		else
		{
			ret = i2c_read(data);
		}

		mutex_unlock(&g_i2c_lock);
	}
	else if(ProcPara[0] == 'w')
	{
		for(i = 0;i < data.data_len;i++)
		{
				databuf[i] = para_val[6+i];
		}

		if(data.data_len > (SEND_MAX_BUFF-1))
		{
			return -EPERM;
		}
		mutex_lock(&g_i2c_lock);
		test_i2c_irq(data.i2c_id,irq_enable);
		data.p_data = (u64)databuf;

		if((data.i2c_id == I2C_GPIO_ID) && (g_I2cGpio.b_used))
		{
			memset(sendbuf,0,MAX_SENDBUFF);
			sendbuf[0] = data.reg_addr;
			for(i = 0;i<data.data_len;i++)
			{
				sendbuf[i+1]=((mt_u8 *)data.p_data)[i];
			}
			ret = GPIO_I2cWrite(sendbuf,data.data_len+1);
		}
		else
		{
			ret = i2c_write(data);
		}
		mutex_unlock(&g_i2c_lock);

		if(0 == ret)
		{
			printk(KERN_ERR "\ndata write success.\n");
		}
	}
	else if(ProcPara[0] == 's')
	{
		test_i2c_config(para_val);
	}
	else if(ProcPara[0] == 'd')
	{
		index = para_val[0];
		test_i2c_dump_reg(index);
	}
	else if(ProcPara[0] == 'h')
	{
		i2c_proc_help();
	}
	else if(ProcPara[0] == 'R')
	{
		index = para_val[0];
		test_i2c_reset(index);
	}

	return count;
}
#endif

static baseops_s i2c_drvops =
{
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = NULL,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = NULL,
};

static struct file_operations i2c_fops =
{
    .owner  =	THIS_MODULE,
    .unlocked_ioctl =	i2c_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = i2c_ioctl,
#endif
    .open  = i2c_open,
    .read = NULL,
    .write = NULL,
    .release = i2c_release,
};

mt_s32 __init i2c_drv_modinit(mt_void)
{
    mt_proc_entry_t *item = NULL;
    mt_drv_proc_t i2c_proc_ops;

    mt_drv_module_register(MT_ID_I2C, "MT_I2C",  NULL);
    mt_osal_snprintf(g_I2cRegisterData.devfs_name, sizeof(g_I2cRegisterData.devfs_name), UMAP_DEVNAME_I2C);
    g_I2cRegisterData.minor  = UMAP_MIN_MINOR_I2C;
    g_I2cRegisterData.owner  = THIS_MODULE;
    g_I2cRegisterData.drvops = &i2c_drvops;
    g_I2cRegisterData.fops = &i2c_fops;
    //printk(KERN_ERR "--------------\n");
    /* I2C device regeister */
    if (mt_drv_dev_register(&g_I2cRegisterData) < 0)
    {
        MT_ERR_I2C("register SCI failed.\n");
        return MT_FAILURE;
    }
    g_I2cGpio.b_used = 0;
    g_I2cGpio.i2c_id = I2C_GPIO_ID;
	mutex_init(&g_i2c_lock);
    //MT_INFO_I2C("[%s %d]Load mt_i2c.ko success.   \t(%s)\n", __FUNCTION__, __LINE__, VERSION_STRING);
    memset(&i2c_proc_ops, 0, sizeof(mt_drv_proc_t));
    i2c_proc_ops.fnRead = i2c_proc;

#if defined CONFIG_MT_FPGA || defined(SYMPHONY6_CHIP_TEST)
    i2c_proc_ops.fnWrite = i2c_procwrite;
#endif

    item = mt_drv_proc_add_module("i2c", &i2c_proc_ops, &g_I2cGpio);
    if(!item)
    {
        return -1;
    }
    return MT_SUCCESS;
}

mt_void __exit i2c_drv_modexit(mt_void)
{
    mt_drv_module_unregister(MT_ID_I2C);
    mt_drv_dev_unregister(&g_I2cRegisterData);
	mutex_destroy(&g_i2c_lock);

    return;
}

