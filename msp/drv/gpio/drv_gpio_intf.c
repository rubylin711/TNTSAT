/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/string.h>

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include <linux/gpio/driver.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

#include "mt_drv_pinctrl.h"
#include "drv_gpio_ioctl.h"


#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_module.h"
#include "drv_gpio_ioctl.h"
#include "mt_osal.h"
#if defined(CONFIG_MT_CHIP_ARIA)
#include "../../arch/arm/mach-aria/aria_reg_base_addr.h"
#endif

//#define DEBUG
#undef DEBUG

#ifdef DEBUG
#define GPIO_DEBUG					printk
#else
#define GPIO_DEBUG(...)				do{}while(0)
#endif
#define GPIO_ERROR(...)				printk(KERN_ERR __VA_ARGS__)

#define CHECK_NULL_RETVOID(para)	do { \
										if (para == NULL) { \
											GPIO_ERROR("[E]%s: param is null!\n", __FUNCTION__); \
											return; \
										} \
									} while(0)

#define  WRITE_REG(Addr, Value) 		HAL_PUT_U32((volatile u32*)((ulong)Addr), (u32)(Value))
#define  READ_REG(Addr) 				HAL_GET_U32((volatile u32*)((ulong)Addr))


#if defined(CONFIG_MT_CHIP_ARIA)
#define R_GPIO_BASE_ADDR              	(ARIA_IO_VA(0xFFA80000))
#else
#define R_GPIO_BASE_ADDR              	((ulong)SYMPHONY_IO_VA((ulong)0xBF0A0000))
#define R_GPIO_AO_REG_BASE            	((ulong)SYMPHONY_IO_VA((ulong)0xBF155000))
#define R_GPIO_IN_BASE_ADDR				((ulong)SYMPHONY_IO_VA((ulong)0xBF0A0040))
#endif

#if defined(CONFIG_MT_CHIP_ARIA)
#define NUM_GPIO 95
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#define NUM_GPIO 72
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
#define NUM_GPIO 114
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
#define NUM_GPIO 144
#else
#define NUM_GPIO 63
#endif

#define GPIO_OFFSET_WDATA				0x0
#define GPIO_OFFSET_WR_N				0x4
#define GPIO_OFFSET_RDATA				0x8
#define GPIO_OFFSET_MASK_N				0xc
#define GPIO_IN_OFFSET_RDATA			0x0
//Group: 32 bit * 4
#define GPIO_OFFSET_SUB_GROUP			0x10

#define GPIO_REG_BITS					32

//flags
#define FLAG_GPIO_DIR_IN_ONLY			0x01

#define IS_GPIO_IN(num)		(((num) >= GPIO_IN_0) && ((num) < GPIO_IN_MAX))

/* GPIO group manager */
struct mt_gpio_group_t
{
	ulong reg_base;	//gpio register base address

	mt_u32 num_start;	//start gpio number
	mt_u32 num_end;		//end gpio number

	mt_u32 io_flags;	//IO direction flags
};

#if defined(CONFIG_MT_CHIP_ARIA)
//aria
static struct mt_gpio_group_t gpio_groups[] =
{
	/* GPIO */
	{R_GPIO_BASE_ADDR, GPIO_0, GPIO_95, 0}
};

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
//symphony1&2
static struct mt_gpio_group_t gpio_groups[] =
{
	/* GPIO */
	{R_GPIO_BASE_ADDR, GPIO_0, GPIO_63, 0},
	/* AO_GPIO */
	{R_GPIO_AO_REG_BASE, AO_GPIO_0, AO_GPIO_8, 0}
};

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
//symphony4
static struct mt_gpio_group_t gpio_groups[] =
{
	/* GPIO */
	{R_GPIO_BASE_ADDR, GPIO_0, GPIO_99, 0},
	/* AO_GPIO */
	{R_GPIO_AO_REG_BASE, AO_GPIO_0, AO_GPIO_8, 0},
	/* GPIO_IN */
	{R_GPIO_IN_BASE_ADDR, GPIO_IN_0, GPIO_IN_5, FLAG_GPIO_DIR_IN_ONLY}
};
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
static struct mt_gpio_group_t gpio_groups[] =
{
	/* GPIO */
	{R_GPIO_BASE_ADDR, GPIO_0, GPIO_123, 0},
	/* AO_GPIO */
	{R_GPIO_AO_REG_BASE, AO_GPIO_0, AO_GPIO_13, 0},
	/* GPIO_IN */
	{R_GPIO_IN_BASE_ADDR, GPIO_IN_0, GPIO_IN_5, FLAG_GPIO_DIR_IN_ONLY}
};

#else
#error "invalid chip, pls config select one correct chip!"
#endif

static mt_device_s gpio_RegisterData;


#define PROC_PARAM_MAXLEN (64)

//---------------------------------------------------------------------------//
/* get gpio group struct by gpio number */
static struct mt_gpio_group_t *gpio_get_group(mt_u32 gpio)
{
	int i;
	int n = ARRAY_SIZE(gpio_groups);

	for (i=0; i<n; i++)
	{
		//printk(KERN_ALERT "%s: gpio %u -> group %d num_start:%d num_end:%d\n", __FUNCTION__, gpio, i,gpio_groups[i].num_start,gpio_groups[i].num_end);
		if (gpio >= gpio_groups[i].num_start
			&& gpio <= gpio_groups[i].num_end)
		{
			GPIO_DEBUG("%s: gpio %u -> group %d\n", __FUNCTION__, gpio, i);
			return &gpio_groups[i];
		}
	}

	GPIO_ERROR("[E]%s: not found gpio %u!\n", __FUNCTION__, gpio);
	return NULL;
}

/* get gpio's register base address */
static ulong group_get_reg_base(struct mt_gpio_group_t *group, mt_u32 gpio)
{
	if (group)
	{
		return (group->reg_base +
			((gpio - group->num_start) / GPIO_REG_BITS)
			* GPIO_OFFSET_SUB_GROUP);
	}
	else
	{
		return 0;
	}
}

/* get gpio's register bit shift */
static mt_u32 group_get_reg_shift(struct mt_gpio_group_t *group, mt_u32 gpio)
{
	if (group)
	{
		return ((gpio - group->num_start) % GPIO_REG_BITS);
	}
	else
	{
		return 0;
	}
}

/* get gpio's io flags */
static mt_u32 group_get_io_flags(struct mt_gpio_group_t *group)
{
	if (group)
	{
		return group->io_flags;
	}
	else
	{
		return 0;
	}
}

static void gpio_set_bit(ulong reg, mt_u32 bit)
{
	mt_u32 val = 0;
	val = READ_REG(reg);
	val |= (0x01 << bit);
	WRITE_REG(reg, val);

	GPIO_DEBUG("%s: reg[0x%lx] bit[%u]\n",__FUNCTION__,reg,bit);
}

static mt_u32 gpio_get_bit(ulong reg, mt_u32 bit)
{
	mt_u32 val = 0;

	val = READ_REG(reg);

	val = (val >> bit) & 0x01;

	GPIO_DEBUG("%s: reg[0x%lx] bit[%u]=%u\n",__FUNCTION__,reg,bit,val);

	return val;
}

static void gpio_clear_bit(ulong reg, mt_u32 bit)
{
	mt_u32 val = 0;
	val = READ_REG(reg);
	val &= ~(0x01 << bit);
	WRITE_REG(reg, val);

	GPIO_DEBUG("%s: reg[0x%lx] bit[%u]\n",__FUNCTION__,reg,bit);
}

/* GPIOx_WDATA */
static mt_u32 gpio_get_wdata(ulong reg_base, mt_u32 shift)
{
	return gpio_get_bit(reg_base + GPIO_OFFSET_WDATA, shift);
}

/* GPIOx_WDATA */
static void gpio_set_wdata(ulong reg_base, mt_u32 shift, mt_u32 data)
{
	if (data)
	{
		gpio_set_bit(reg_base + GPIO_OFFSET_WDATA, shift);
	}
	else
	{
		gpio_clear_bit(reg_base + GPIO_OFFSET_WDATA, shift);
	}
}

/* GPIOx_WR_N */
static mt_u32 gpio_get_wr_en(ulong reg_base, mt_u32 shift)
{
	return gpio_get_bit(reg_base + GPIO_OFFSET_WR_N, shift);
}

/* GPIOx_WR_N */
static void gpio_set_wr_en(ulong reg_base, mt_u32 shift, mt_u32 input)
{
	if (input)
	{
		/* input */
		gpio_set_bit(reg_base + GPIO_OFFSET_WR_N, shift);
	}
	else
	{
		/* output */
		gpio_clear_bit(reg_base + GPIO_OFFSET_WR_N, shift);
	}
}

/* GPIOx_RDATA */
static mt_u32 gpio_get_rdata(ulong reg_base, mt_u32 shift)
{
	return gpio_get_bit(reg_base + GPIO_OFFSET_RDATA, shift);
}

/* GPIO_IN_RDATA */
static mt_u32 gpio_in_get_rdata(ulong reg_base, mt_u32 shift)
{
	return gpio_get_bit(reg_base + GPIO_IN_OFFSET_RDATA, shift);
}

/* GPIOx_MASK_N */
static mt_u32 gpio_get_mask_en(ulong reg_base, mt_u32 shift)
{
	return gpio_get_bit(reg_base + GPIO_OFFSET_MASK_N, shift);
}

/* GPIOx_MASK_N */
static void gpio_set_mask_en(ulong reg_base, mt_u32 shift, mt_u32 en)
{
	if (en)
	{
		/* unmask */
		gpio_set_bit(reg_base + GPIO_OFFSET_MASK_N, shift);
	}
	else
	{
		/* masked */
		gpio_clear_bit(reg_base + GPIO_OFFSET_MASK_N, shift);
	}
}

/* mask/unmask gpio */
static void gpio_io_enable(u8 gpio, gpio_mask_e enable)
{
	struct mt_gpio_group_t *group;
	ulong reg_base;
	mt_u32 io_flags;
	mt_u32 shift;

	GPIO_DEBUG("%s: gpio %u, enable %d\n",__FUNCTION__,gpio,enable);

	group = gpio_get_group(gpio);
	CHECK_NULL_RETVOID(group);

	io_flags = group_get_io_flags(group);
	if ((io_flags & FLAG_GPIO_DIR_IN_ONLY) != 0)
	{
		GPIO_ERROR("[E]gpio_io_enable: gpio(%u) IN only!\n", gpio);
		return;
	}

	reg_base = group_get_reg_base(group, gpio);
	shift = group_get_reg_shift(group, gpio);
	gpio_set_mask_en(reg_base, shift, enable);
}

/* set gpio direction */
static void gpio_set_dir(u8 gpio, gpio_dir_e dir)
{
	struct mt_gpio_group_t *group;
	ulong reg_base;
	mt_u32 io_flags;
	mt_u32 shift;
	mt_u32 mask_n;

	GPIO_DEBUG("%s: gpio %u, dir %d\n",__FUNCTION__,gpio,dir);

	group = gpio_get_group(gpio);
	CHECK_NULL_RETVOID(group);

	io_flags = group_get_io_flags(group);
	if ((io_flags & FLAG_GPIO_DIR_IN_ONLY) != 0)
	{
		GPIO_ERROR("[E]gpio_set_dir: gpio(%u) IN only!\n", gpio);
		return;
	}

	reg_base = group_get_reg_base(group, gpio);
	shift = group_get_reg_shift(group, gpio);

	//Debug
	mask_n = gpio_get_mask_en(reg_base, shift);
	if (mask_n == 0)
	{
		GPIO_ERROR("[E]gpio_set_dir: io masked! pls call gpio_io_enable first!\n");
	}

	gpio_set_wr_en(reg_base, shift, dir);
}

/* get gpio direction */
static void gpio_get_dir(u8 gpio, gpio_dir_e *dir)
{
	struct mt_gpio_group_t *group;
	ulong reg_base;
	mt_u32 io_flags;
	mt_u32 shift;

	if (NULL == dir)
	{
		return;
	}

	group = gpio_get_group(gpio);
	CHECK_NULL_RETVOID(group);

	io_flags = group_get_io_flags(group);
	if ((io_flags & FLAG_GPIO_DIR_IN_ONLY) != 0)
	{
		*dir = GPIO_DIR_INPUT;
		return;
	}

	reg_base = group_get_reg_base(group, gpio);
	shift = group_get_reg_shift(group, gpio);

	*dir = (gpio_dir_e)gpio_get_wr_en(reg_base, shift);

	GPIO_DEBUG("%s: gpio %u, dir %d\n",__FUNCTION__,gpio,*dir);
}

/* set gpio output High/Low value */
static void m_gpio_set_value(u8 gpio, gpio_value_e val)
{
	struct mt_gpio_group_t *group;
	ulong reg_base;
	mt_u32 io_flags;
	mt_u32 shift;
	mt_u32 input;

	GPIO_DEBUG("%s: gpio %u, val %d\n",__FUNCTION__,gpio,val);
	group = gpio_get_group(gpio);
	CHECK_NULL_RETVOID(group);

	io_flags = group_get_io_flags(group);
	if ((io_flags & FLAG_GPIO_DIR_IN_ONLY) != 0)
	{
		GPIO_ERROR("[E]m_gpio_set_value: gpio(%u) IN only!\n", gpio);
		return;
	}

	reg_base = group_get_reg_base(group, gpio);
	shift = group_get_reg_shift(group, gpio);

	//Debug
	input = gpio_get_wr_en(reg_base, shift);
	if (input)
	{
		GPIO_ERROR("[E]m_gpio_set_value: input gpio %u, should not set output value!\n", gpio);
	}

	gpio_set_wdata(reg_base, shift, val);
}

/* get gpio input High/Low value */
static void m_gpio_get_value(u8 gpio, gpio_value_e *val)
{
	struct mt_gpio_group_t *group;
	ulong reg_base;
	mt_u32 shift;
	mt_u32 input;

	if (NULL == val)
	{
		return;
	}

	group = gpio_get_group(gpio);
	CHECK_NULL_RETVOID(group);

	reg_base = group_get_reg_base(group, gpio);
	shift = group_get_reg_shift(group, gpio);

	if (IS_GPIO_IN(gpio))
	{
		*val = (gpio_value_e)gpio_in_get_rdata(reg_base, shift);
	}
	else
	{
		//Debug
		input = gpio_get_wr_en(reg_base, shift);
		if (input == 0)
		{
			GPIO_ERROR("[E]m_gpio_set_value: output gpio %u, should not get input value!\n", gpio);
		}

		*val = (gpio_value_e)gpio_get_rdata(reg_base, shift);
	}

	GPIO_DEBUG("%s: gpio %u, val %d\n",__FUNCTION__,gpio,*val);
}

/* get gpio output High/Low value */
static void m_gpio_get_output_value(u8 gpio, gpio_value_e *val)
{
	struct mt_gpio_group_t *group;
	ulong reg_base;
	mt_u32 shift;
	mt_u32 io_flags;
	mt_u32 input;

	if (NULL == val)
	{
		return;
	}
	group = gpio_get_group(gpio);
	CHECK_NULL_RETVOID(group);

	io_flags = group_get_io_flags(group);
	if ((io_flags & FLAG_GPIO_DIR_IN_ONLY) != 0)
	{
		GPIO_ERROR("[E]m_gpio_get_output_value: gpio(%u) IN only!\n", gpio);
		return;
	}

	reg_base = group_get_reg_base(group, gpio);
	shift = group_get_reg_shift(group, gpio);

	//Debug
	input = gpio_get_wr_en(reg_base, shift);
	if (input)
	{
		GPIO_ERROR("[E]m_gpio_get_output_value: input gpio %u, should not get output value!\n", gpio);
	}

	*val = (gpio_value_e)gpio_get_wdata(reg_base, shift);

	GPIO_DEBUG("%s: gpio %u, val %d\n",__FUNCTION__,gpio,*val);
}

static void gpio_dump_state(struct seq_file *p)
{
	u8 gpio;
	gpio_dir_e dir;
	gpio_value_e val;

	PROC_PRINT(p, "====== DUMP GPIO =====\n");
	PROC_PRINT(p, "GPIO\tDIR\tVAL\n");

	for (gpio=GPIO_0; gpio<GPIO_UNKNOWN; gpio++)
	{
#if defined(CONFIG_MT_CHIP_ARIA)
		if (gpio > GPIO_95)
			continue;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
		if (gpio <= GPIO_63
			|| (gpio >= AO_GPIO_0 && gpio < AO_GPIO_MAX))
		{
		}
		else
		{
			continue;
		}
#endif

		if (gpio != AO_GPIO_MAX && gpio != GPIO_IN_MAX)
		{
			gpio_get_dir(gpio, &dir);

			if (dir == GPIO_DIR_INPUT)
			{
				m_gpio_get_value(gpio, &val);
			}
			else
			{
				m_gpio_get_output_value(gpio, &val);
			}

			PROC_PRINT(p, "%u:\t%d\t%d\n", gpio, dir, val);
		}
	}
}

static int gpio_open (struct inode *inode, struct file *file)
{
	return 0;
}

static long gpio_ioctl(struct file *file,
						unsigned int cmd, unsigned long arg)
{
	//struct inode *inode;
	//int minor;
	int retval = 0;
	struct gpio_dir_param dir;
	struct gpio_value_param val;
	struct gpio_mask_param mask;

	//inode=file->f_path.dentry->d_inode;
	//minor = iminor(inode);
	//printk("gpio_ioctl cmd == %d   dir.gpio == %d\n",cmd, dir.gpio);

	switch(cmd)
	{
		case GPIO_DRV_IOC_IO_ENABLE:
		{
			if (copy_from_user((void *)&mask, (void *)arg, sizeof(struct gpio_mask_param)) != 0)
			{
				return -EIO;
			}
			GPIO_DEBUG("gpio_ioctl cmd GPIO_DRV_IOC_IO_ENABLE mask.gpio == %d   mask.val == %d\n",mask.gpio, mask.val);
			if(mask.gpio >= GPIO_UNKNOWN)
				return -EIO;
			if(mask.val == GPIO_MASK_UNENABLE || mask.val == GPIO_MASK_ENABLE)
			{
				gpio_io_enable(mask.gpio, mask.val);
			}
			else
			{
				return -EIO;
			}
			break;
		}
		case GPIO_DRV_IOC_SET_DIR:
		{
			if (copy_from_user((void *)&dir, (void *)arg, sizeof(struct gpio_dir_param)) != 0)
			{
				return -EIO;
			}
			GPIO_DEBUG("gpio_ioctl cmd GPIO_DRV_IOC_SET_DIR dir.dir == %d   dir.gpio == %d\n",dir.dir, dir.gpio);
			if(dir.gpio >= GPIO_UNKNOWN)
				return -EIO;
			if(dir.dir == GPIO_DIR_INPUT || dir.dir == GPIO_DIR_OUTPUT)
			{
				gpio_set_dir(dir.gpio, dir.dir);
			}
			else
			{
				return -EIO;
			}
			break;
		}
		case GPIO_DRV_IOC_GET_DIR:
		{
			if (copy_from_user((void *)&dir, (void *)arg, sizeof(struct gpio_dir_param)) != 0)
			{
				return -EIO;
			}

			if(dir.gpio >= GPIO_UNKNOWN)
				return -EIO;

			gpio_get_dir(dir.gpio, &dir.dir);
			if (copy_to_user((void *)arg, (void *)&dir, sizeof(struct gpio_dir_param)) != 0)
			{
				return -EIO;
			}
			break;
		}
		case GPIO_DRV_IOC_SET_VALUE:
		{
			if (copy_from_user((void *)&val, (void *)arg, sizeof(struct gpio_value_param)) != 0)
			{
				return -EIO;
			}
			GPIO_DEBUG("gpio_ioctl cmd GPIO_DRV_IOC_SET_VALUE val == %d   gpio == %d\n",val.val, val.gpio);
			if(val.gpio >= GPIO_UNKNOWN)
				return -EIO;

			m_gpio_set_value(val.gpio, val.val);
			break;
		}
		case GPIO_DRV_IOC_GET_VALUE:
		{
			if (copy_from_user((void *)&val, (void *)arg, sizeof(struct gpio_value_param)) != 0)
			{
				return -EIO;
			}

			if(val.gpio >= GPIO_UNKNOWN)
				return -EIO;

			m_gpio_get_value(val.gpio, &val.val);
			if (copy_to_user((void *)arg, (void *)&val, sizeof(struct gpio_value_param)) != 0)
			{
				return -EIO;
			}
			break;
		}
		default:
			break;
    }

	if(retval < 0)
		return -EIO;
	return 0;
}

static mt_s32 gpio_resume(basedev_s *pdev)
{

	return 0;
}

static mt_s32 gpio_suspend(basedev_s *pdev, pm_message_t state)
{
	return 0;
}

static int gpio_close (struct inode *inode, struct file *file)
{
    return 0;
}

const struct file_operations GPIO_FOPS = {
  .open                 = gpio_open,
  .unlocked_ioctl       = gpio_ioctl,
  .release              = gpio_close,
};

static baseops_s gpio_baseOps =
{
        .probe = NULL,
        .remove = NULL,
        .shutdown = NULL,
        .prepare = NULL,
        .complete = NULL,
        .suspend = gpio_suspend,
        .suspend_late = NULL,
        .resume_early = NULL,
        .resume = gpio_resume
};

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
static int mt_chip_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	int ret = 0;
	gpio_value_e val = 0;

	m_gpio_get_value(offset,&val);
	if(val ==  GPIO_VALUE_LOW_LEVEL)
	{
		ret = 0;
	}
	else if(val == GPIO_VALUE_HIGH_LEVEL)
	{
		ret = 1;
	}
	return ret;
}

static void mt_chip_gpio_set(struct gpio_chip *chip, unsigned offset, int val)
{
	gpio_value_e val_t ;
	if(val == 1)
	{
		val_t = GPIO_VALUE_HIGH_LEVEL;
	}
	else if(val == 0)
	{
		val_t = GPIO_VALUE_LOW_LEVEL;
	}
	m_gpio_set_value(offset, val_t);
}

static int mt_chip_direction_input(struct gpio_chip *c, unsigned offset)
{
     gpio_io_enable(offset, GPIO_MASK_ENABLE);
     gpio_set_dir(offset, GPIO_DIR_INPUT);
     return 0;
}

static int mt_chip_direction_output(struct gpio_chip *c, unsigned offset, int val)
{
	gpio_io_enable(offset, GPIO_MASK_ENABLE);
	gpio_set_dir(offset, GPIO_DIR_OUTPUT);
	return 0;
}

static struct gpio_chip mt_gpio_chip = {
	.owner = THIS_MODULE,
	.label = UMAP_DEVNAME_GPIO,

	.base = 0,
	.ngpio = NUM_GPIO,
	.get = mt_chip_gpio_get,
	.set = mt_chip_gpio_set,

	.direction_input = mt_chip_direction_input,
	.direction_output = mt_chip_direction_output,
};

#endif

static mt_s32 gpio_Proc(struct seq_file *p, mt_void *v)
{
	//p += PROC_PRINT(p, "+++++++ this is GPIO ++++++++ \n");
	gpio_dump_state(p);

	return 0;
}

mt_s32 gpio_procwrite(struct file * file, const char __user * buf, size_t count, loff_t *ppos)
{
	mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
	//mt_u32 para_val[PROC_PARAM_MAXLEN] = {0};
	mt_char *param = NULL;

	if(count > PROC_PARAM_MAXLEN)
	{
		printk(KERN_ERR "write data is too long!\n");
		return -EFAULT;
	}

	memset(ProcPara,0,PROC_PARAM_MAXLEN);
	if(copy_from_user(ProcPara, buf, count))
	{
		printk(KERN_ALERT "write data is too long!\n");
		return -EFAULT;
	}
	ProcPara[PROC_PARAM_MAXLEN-1] = 0;
	param =ProcPara;

	return count;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
extern void (*p_drv_gpio_io_enable)(u8 gpio, int enable);
extern void (*p_drv_gpio_set_dir)(u8 gpio, int dir);
extern void (*p_drv_gpio_get_dir)(u8 gpio, int *dir);
extern void (*p_drv_gpio_set_value)(u8 gpio, int val);
extern void (*p_drv_gpio_get_value)(u8 gpio, int *val);

static void gpio_setup_func_ptr(void)
{
	p_drv_gpio_io_enable 	= (void (*)(u8, int))drv_gpio_io_enable;
	p_drv_gpio_set_dir 		= (void (*)(u8, int))drv_gpio_set_dir;
	p_drv_gpio_get_dir 		= (void (*)(u8, int*))drv_gpio_get_dir;
	p_drv_gpio_set_value 	= (void (*)(u8, int))drv_gpio_set_value;
	p_drv_gpio_get_value 	= (void (*)(u8, int*))drv_gpio_get_value;
}
#endif

int __init gpio_modinit(void)
{
    mt_proc_entry_t *pProcItem;
    #if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    int err = -EIO;
    #endif
	mt_drv_proc_t gpioOpt =
	{
		.fnRead = gpio_Proc,
		.fnWrite = gpio_procwrite,
	};

    (mt_void)mt_drv_module_register(MT_ID_GPIO, "MT_GPIO", MT_NULL);

	mt_osal_snprintf(gpio_RegisterData.devfs_name, sizeof(gpio_RegisterData.devfs_name), UMAP_DEVNAME_GPIO);
    gpio_RegisterData.minor	= UMAP_MIN_MINOR_GPIO;
    gpio_RegisterData.owner	= THIS_MODULE;
    gpio_RegisterData.fops	= (struct file_operations *)&GPIO_FOPS;
    gpio_RegisterData.drvops = &gpio_baseOps;

    if (mt_drv_dev_register(&gpio_RegisterData) < 0)
    {
        MT_PRINT("register gpio failed.\n");
        return MT_FAILURE;
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    err = gpiochip_add(&mt_gpio_chip);
    if(err)
    {
        mt_drv_dev_unregister(&gpio_RegisterData);
        return MT_FAILURE;
    }
#endif
    pProcItem = mt_drv_proc_add_module(MT_MOD_GPIO, &gpioOpt, MT_NULL);
    if (!pProcItem)
    {
        MT_PRINT("add gpio proc failed.\n");
        mt_drv_dev_unregister(&gpio_RegisterData);
        return MT_FAILURE;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	gpio_setup_func_ptr();
#endif

    return MT_SUCCESS;
}

void __exit gpio_cleanup(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    gpiochip_remove(&mt_gpio_chip);
#endif
    mt_drv_proc_rm_module(MT_MOD_GPIO);
    mt_drv_dev_unregister(&gpio_RegisterData);
    mt_drv_module_unregister(MT_ID_GPIO);
}

void drv_gpio_io_enable(u8 gpio, gpio_mask_e enable)
{
    gpio_io_enable(gpio, enable);
}
EXPORT_SYMBOL(drv_gpio_io_enable);

void drv_gpio_set_dir(u8 gpio, gpio_dir_e dir)
{
    gpio_set_dir(gpio, dir);
}
EXPORT_SYMBOL(drv_gpio_set_dir);

void drv_gpio_get_dir(u8 gpio, gpio_dir_e *dir)
{
	if (dir)
	{
		gpio_get_dir(gpio,dir);
	}
}
EXPORT_SYMBOL(drv_gpio_get_dir);

void drv_gpio_set_value(u8 gpio, gpio_value_e val)
{
    m_gpio_set_value(gpio, val);
}
EXPORT_SYMBOL(drv_gpio_set_value);

void drv_gpio_get_value(u8 gpio, gpio_value_e *val)
{
	if (val)
	{
		m_gpio_get_value(gpio,val);
	}
}
EXPORT_SYMBOL(drv_gpio_get_value);

void drv_gpio_standby(void)
{
    gpio_list_e index=0;

	for(index=GPIO_2; index<=GPIO_123; index++)
	{
		mt_pinctrl_set_function((PINMUX_INDEX_E)index, 1);   //set gpio mode
		drv_gpio_io_enable(index, TRUE);
		drv_gpio_set_dir(index, 1);		 //set input
	}
	return;
}
EXPORT_SYMBOL(drv_gpio_standby);

