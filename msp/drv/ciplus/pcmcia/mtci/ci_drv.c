/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/irq.h>
#include <asm/irq.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <mach/irqs.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>
#include <asm/uaccess.h>
#include <linux/compat.h>
#include <linux/spinlock.h>
#include <linux/gpio/consumer.h>
#include <linux/clk.h>
#include <linux/reset.h>


#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"

#include "drv_ci_ioctl.h"
#include "mt_drv_pinctrl.h"
#include "drv_gpio_ioctl.h"

#include "ci_drv.h"

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#define REG_CI_TSPIDCTRL    	0x40
#define REG_CI_TSPIDRAM0    	0x300
#define REG_CI_TSCTRL       		0x800
#define REG_CI_TSOCLKCTRL   	0x804
#endif

/******************************************
montage clk register define
*******************************************/
#define TSI_CLKEN_REG			0xbf50b000
#define TSI_CLKSEL_REG	   	0xbf50b004
#define CI_CLKEN_REG		   	0xbf50b100
#define CI_CLKSEL_REG	   		0xbf50b104
#define CI_SRSTN_REG	   		0xbf50b10c

#define BIT_TS3_CLKSEL	   		7
#define BIT_CICLK_EN				0
#define BIT_CI_CLLSEL				0
#define BIT_CITSIN_CLKSEL   	1

/******************************************
tsi register define
*******************************************/
#define TS_SRC_SEL		   			0xbf138008
#define TS0_SAMPLE_CTRL	   0xbf200000
#define TS3_SAMPLE_CTRL	   0xbf200030
/*
#define TS2_SAMPLE_CTRL    0xbf200020
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define TSI_CI_SET_CFG		   0xbf290000
#define TSI_CI_LLN_NUM_START   0xbf290008
#define TSI_CI_STATUS		   0xbf29000c
#define TSI_CI_SWTSI_CH_SET    0xbf290010
#define TSI_CI_CICAM_SET	   0xbf290014
#endif
*/
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
#define BIT_CICAM_SEL	12
#define BIT_TS0_EN		31
#define BIT_TS3_EN		31
#define BIT_SERIAL_EN	30
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
#define BIT_CICAM_SEL	20
#define BIT_TS2_SEL 		12
#endif


#define MTCI_NAME "mt_ci"
#define MAX_OPEN	1
#define MAX_CI_DEV 	1

#define ts1_src_sel(a)				reg_set_bits(TS_SRC_SEL,4,1,a)
#define ciclk_ciahb_reset(a)		reg_set_bits(CI_SRSTN_REG,0,1,a)
/* calculate the virtual address of reg by the vitual iobase address*/
#define ci_reg_vitual_addr(iobase,reg_offset) 		(reg_offset+iobase)

static inline u32 mtci_reg_read_u32(struct mt_ci_dev *dev, u32 reg_offset)
{
	return readl(((void*)(dev->iobase))+reg_offset);
}

static inline void mtci_reg_write_u32(struct mt_ci_dev *dev, u32 reg_offset, u32 value)
{
	writel(value,(volatile void*)(dev->iobase) + reg_offset);
}

static inline void mtci_reg_write_u8(struct mt_ci_dev *dev, u32 reg, u8 value)
{
	writeb(value,((volatile u8 *)(dev->iobase)) + reg);
}

static inline u8 mtci_reg_read_u8(struct mt_ci_dev *dev, u32 reg)
{
	return readb((volatile u8 *)(dev->iobase) + reg);
}

static int reg_set_bits(u32 reg, u8 sbit, u8 size, u32 val)
{
    u32 tmp = 0, vbit = 0, cur_val = 0;
	unsigned long reg_va = 0;

	reg_va = SYMPHONY_IO_VA(reg);
    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = HAL_GET_U32((volatile u32 *)reg_va);
    cur_val = (tmp >> sbit) & vbit; //get old value from register
    if(val == cur_val)
        return 0;
    tmp &= ~(vbit << sbit);
    tmp |= (val << sbit);
    HAL_PUT_U32((volatile u32 *)reg_va, tmp);

    return 0;
}

/*
static int reg_get_bits(unsigned int reg, unsigned int sbit, unsigned int size, unsigned int *val)
{
    unsigned int tmp = 0, vbit = 0;
	unsigned long reg_va = 0;
	reg_va = SYMPHONY_IO_VA(reg);
    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = HAL_GET_U32((volatile u32*)reg_va);
    *val = (tmp >> sbit) & vbit; //get old value from register
    return 0;
}
*/


static void cicam_pwr_ctrl(u8 pwron)
{
	if(pwron != 0){
		drv_gpio_set_value(INDEX_SW_PIN_CTRL122,GPIO_VALUE_HIGH_LEVEL);
	} else {
		drv_gpio_set_value(INDEX_SW_PIN_CTRL122,GPIO_VALUE_LOW_LEVEL);
	}
}

static void mt_ci_relate_to_permission(void)
{
	unsigned int val;
	unsigned long reg;

	reg = SYMPHONY_IO_VA(0xbf121000);
	val = HAL_GET_U32((volatile u32*)reg);
	val = 0xffffffff;
	HAL_PUT_U32((volatile u32*)reg,val);

	reg = SYMPHONY_IO_VA(0xbf121020);
	val = HAL_GET_U32((volatile u32*)reg);
	val = 0xffffffff;
	HAL_PUT_U32((volatile u32*)reg,val);
}

static void mt_ci_gpio_power(void)
{
	//power pin
	drv_gpio_io_enable(INDEX_SW_PIN_CTRL122,GPIO_MASK_ENABLE);
	drv_gpio_set_dir(INDEX_SW_PIN_CTRL122,GPIO_DIR_OUTPUT);
}

static void mt_ci_switch_iomode(struct mt_ci_dev *dev)
{
	u32 val = 0;
	ci_reg_struct *ci_reg = dev->iobase;

	val = readl(&ci_reg->reg_ci_ctrl);
	val |= 1<<2;
	writel(val,&ci_reg->reg_ci_ctrl);

	writel(0x0dc30c31,&ci_reg->reg_ci_fsmcnt1);
}
static void mt_ci_switch_memmode(struct mt_ci_dev *dev)
{
	u32 val = 0;
	ci_reg_struct *ci_reg = dev->iobase;

	val = readl(&ci_reg->reg_ci_ctrl);
	val &=  ~(1<<2);
	writel(val,&ci_reg->reg_ci_ctrl);

	writel(0x37426131,&ci_reg->reg_ci_fsmcnt1);
}

static void mt_cam_write_register(struct mt_ci_dev *dev, u32 addr, u8 data)
{
	//printk(KERN_INFO "write cam reg u8 address: 0x%lx\n",((volatile u8 *)(dev->cam_reg_base)) + addr);
	writeb(data,((volatile u8 *)(dev->cam_reg_base)) + addr);
}

static u8 mt_cam_read_register(struct mt_ci_dev *dev, u32 addr)
{
	//printk(KERN_INFO "read cam reg u8 address: 0x%lx\n",(volatile u8 *)(dev->cam_reg_base) + addr);
	return readb((volatile u8 *)(dev->cam_reg_base) + addr);
}

/*static void mt_ci_intsts_clear(struct mt_ci_dev *dev,u32 mask)
{
	reg_write_u32(dev,REG_CI_INTSTS,mask);
}
*/

static void mt_ci_intctrl_set(struct mt_ci_dev *cidev,u32 mask)
{
	ci_reg_struct *ci_reg = (ci_reg_struct *)(cidev->iobase);
	writel(mask,&ci_reg->reg_ci_intctrl);
}

static void mt_ci_tsctl_enable(struct mt_ci_dev *dev,u8 isenable)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	u32 dtmp = reg_read_u32(dev,REG_CI_TSCTRL);
	if(isenable)
		dtmp |= (1<<31);
	else
		dtmp &= ~(1<<31);
	reg_write_u32(dev,REG_CI_TSCTRL,dtmp);
#endif
}

static void mt_ci_tsctl_serial(struct mt_ci_dev *dev,u8 isserial)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	u32 dtmp = reg_read_u32(dev,REG_CI_TSCTRL);
	if(isserial)
		dtmp |= (1<<30);
	else
		dtmp &= ~(1<<30);
	reg_write_u32(dev,REG_CI_TSCTRL,dtmp);
#endif
}

static void mt_ci_pidfilter_enable(struct mt_ci_dev *dev,u8 isenable)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	u32 dtmp = reg_read_u32(dev,REG_CI_TSPIDCTRL);
	if(isenable)
		dtmp |= (1<<31);
	else
		dtmp &= ~(1<<31);
	reg_write_u32(dev,REG_CI_TSPIDCTRL,dtmp);
#endif
}

static void mt_ci_cfg_unsetce(struct mt_ci_dev *dev)
{
	ci_reg_struct *ci_reg = dev->iobase;
	u32 dtmp = readl(&ci_reg->reg_ci_fsmcnt2);
	dtmp &= ~0x0f;
	dtmp |= 0x0d;
	writel(dtmp,&ci_reg->reg_ci_fsmcnt2);
}

static void mt_ci_pidram_set(struct mt_ci_dev *dev,u32 index,u32 val)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	reg_write_u32(dev,REG_CI_TSPIDRAM0+index*4,val)
#endif
}

static u32 mt_ci_pidram_get(struct mt_ci_dev *dev,u32 index)
{
	u32 dtmp = 0;

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	dtmp = reg_read_u32(dev,REG_CI_TSPIDRAM0 + index*4);
#endif

	return dtmp;
}


/*
* input arg:dev ¡¢the reg after ioramp
*/
static void mt_ci_cam_detect(struct mt_ci_dev *cidev)
{
	u32 int_status = 0;
	ci_reg_struct *ci_reg;
	static u32 cam_dectect = 0;

	ci_reg = cidev->iobase;
	int_status = readl(&ci_reg->reg_ci_intsts);
	printk(KERN_INFO "value:0x%x\n",int_status);
	if(int_status & CI_INTST_CD1INSTD) {
		cam_dectect |= CI_INTST_CD1INSTD;
	}
	if(int_status & CI_INTST_CD2INSTD) {
		cam_dectect |= CI_INTST_CD2INSTD;
	}
	if( int_status & CI_INTST_CD1RMVD) {
		cam_dectect |= CI_INTST_CD1RMVD;
	}
	if( int_status & CI_INTST_CD2RMVD) {
		cam_dectect |= CI_INTST_CD2RMVD;
	}

	if (((cam_dectect & CI_CAMOUT) == CI_CAMOUT) && ((cam_dectect & CI_CAMIN) == CI_CAMIN)) {
		if (cidev->cam_status == CAM_OUT) {
			cidev->cam_status= CAM_IN;
		} else {
			cidev->cam_status = CAM_OUT;
		}
		cam_dectect = 0;
	}	else if ((cam_dectect & CI_CAMOUT) == CI_CAMOUT) {
		cam_dectect = 0;
		cidev->cam_status = CAM_OUT;
	}	else if ((cam_dectect & CI_CAMIN) == CI_CAMIN) {
		cidev->cam_status = CAM_IN;
		cam_dectect = 0;
	}

	writel(CI_INTST_MASK,&ci_reg->reg_ci_intsts);
}

static void mt_ci_reg_dump(struct mt_ci_dev *cidev)
{
	ci_reg_struct *ci_reg;

	ci_reg = (ci_reg_struct *)(cidev->iobase);
	printk(KERN_INFO "REG_CI_CTRL:0x%x\n",readl(&ci_reg->reg_ci_ctrl));
	printk(KERN_INFO "REG_CI_INTSTS:0x%x\n",readl(&ci_reg->reg_ci_intsts));
	printk(KERN_INFO "REG_CI_INTCTRL:0x%x\n",readl(&ci_reg->reg_ci_intctrl));
	printk(KERN_INFO "REG_CI_FSMCNT1:0x%x\n",readl(&ci_reg->reg_ci_fsmcnt1));
	printk(KERN_INFO "REG_CI_FSMCNT2:0x%x",readl(&ci_reg->reg_ci_fsmcnt2));
	printk(KERN_INFO "REG_CI_CDCFG:0x%x",readl(&ci_reg->reg_ci_cdcfg));
	printk(KERN_INFO "REG_CI_TS1CTRL:0x%x",readl(&ci_reg->reg_ci_ts1ctrl));


#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	printk(KERN_INFO "REG_CI_TSPIDCTRL:0x%x:0x%x",REG_CI_TSPIDCTRL,readl(dev,REG_CI_TSPIDCTRL));
//	for(int i = 0; i < 64;i++) {
//		printk(KERN_INFO "REG_CI_TSPIDRAM0:0x%x:0x%x",REG_CI_TSPIDRAM0+i*4,reg_read_u32(dev,REG_CI_TSPIDRAM0+i*4));
//	}
	printk(KERN_INFO "REG_CI_TSCTRL:0x%x:0x%x",REG_CI_TSCTRL,readl(dev,REG_CI_TSCTRL));
	printk(KERN_INFO "REG_CI_TSOCLKCTRL:0x%x:0x%x",REG_CI_TSOCLKCTRL,readl(dev,REG_CI_TSOCLKCTRL));
#endif
	/*
	printk(KERN_INFO "CI_CLKEN_REG: 0x%x:0x%x\n",CI_CLKEN_REG,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(CI_CLKEN_REG)));
	printk(KERN_INFO "CI_CLKSEL_REG: 0x%x:0x%x\n",CI_CLKSEL_REG,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(CI_CLKSEL_REG)));
	printk(KERN_INFO "CI_SRSTN_REG: 0x%x:0x%x\n",CI_SRSTN_REG,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(CI_SRSTN_REG)));

	printk(KERN_INFO "CI_SET_CFG: 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf290000)));
	printk(KERN_INFO "CI_LLN_NUM_START: 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf290008)));
	printk(KERN_INFO "CI_STATUS: 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf29000C)));
	printk(KERN_INFO "CI_SWTSI_CH_SET: 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf290010)));
	printk(KERN_INFO "CI_CICAM_SET: 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf290014)));
	printk(KERN_INFO "CI_DEBUG: 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf290018)));

	printk(KERN_INFO "tsi sample reg:\n");
	printk(KERN_INFO "TS_SRC_SEL(0xbf138008): 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf138008)));
	printk(KERN_INFO "TS0_SAMPLE_CTRL(0xbf200000):0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200000)));
	printk(KERN_INFO "TS0_SAMPLE_STA(0xbf200004):0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200004)));
	printk(KERN_INFO "TS1_SAMPLE_CTRL(0xbf200010): 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200010)));
	printk(KERN_INFO "TS1_SAMPLE_STA(0xbf200014): 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200014)));
	printk(KERN_INFO "TS2_SAMPLE_CTRL(0xbf200020): 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200020)));
	printk(KERN_INFO "TS2_SAMPLE_STA(0xbf200024): 0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf200024)));
	printk(KERN_INFO "\n");


	printk(KERN_INFO "swtsi reg:\n");
	printk(KERN_INFO "SWTSI_CI_EN(0xbf210018):0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf210018)));
	for(i = 0; i < 4;i++) {
		printk(KERN_INFO "SWTSI_CHAN_%d_CTRL(0x%x):0x%x\n",i,0xbf210024+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf210024+i*0x100)));
		printk(KERN_INFO "SWTSI_CHAN_%d_STA(0x%x):0x%x\n",i,0xbf210028+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf210028+i*0x100)));
		printk(KERN_INFO "SWTSI_CHAN_%d_DBUF_RD(0x%x):0x%x\n",i,0xbf210098+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf210098+i*0x100)));
		printk(KERN_INFO "SWTSI_CHAN_%d_DBUF_SUBLEN(0x%x):0x%x\n",i,0xbf21009C+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf21009C+i*0x100)));
		printk(KERN_INFO "SWTSI_CHAN_%d_DBUF_WR(0x%x):0x%x\n",i,0xbf2100A0+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf2100A0+i*0x100)));
		printk(KERN_INFO "SWTSI_CHAN_%d_CFG5(0x%x):0x%x\n",i,0xbf210054+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf210054+i*0x100)));
	}
	printk(KERN_INFO "\n");

	printk(KERN_INFO "demux reg:\n");
	printk(KERN_INFO "demux_debug_ctrl(0xbf230608):0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf230608)));
	printk(KERN_INFO "demux_state(0xbf230610):0x%x\n",HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf230610)));
	for(i = 0; i < 48; i++) {
		printk(KERN_INFO "slot_%d_cfg0(0x%x):0x%x\n",i,0xbf230000+i*8,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf230000+i*8)));
		printk(KERN_INFO "slot_%d_cfg1(0x%x):0x%x\n",i,0xbf230004+i*8,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf230004+i*8)));
	}
	printk(KERN_INFO "\n");

	printk(KERN_INFO "rec chan reg:\n");
	for(i = 0; i < 4; i++) {
		printk(KERN_INFO "chan_%d,REC_SET(0x%x):0x%x\n",i,0xbf261100+i*0x100,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100)));
		printk(KERN_INFO "chan_%d,START_ADDR(0x%x):0x%x\n",i,0xbf261100+i*0x100+0x04,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100+0x04)));
		printk(KERN_INFO "chan_%d,END_ADDR(0x%x):0x%x\n",i,0xbf261100+i*0x100+0x08,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100+0x08)));
		printk(KERN_INFO "chan_%d,RD_ADDR(0x%x):0x%x\n",i,0xbf261100+i*0x100+0x0c,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100+0x0c)));
		printk(KERN_INFO "chan_%d,WR_ADDR(0x%x):0x%x\n",i,0xbf261100+i*0x100+0x10,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100+0x10)));
		printk(KERN_INFO "chan_%d,TS_SN(0x%x):0x%x\n",i,0xbf261100+i*0x100+0x18,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100+0x18)));
		printk(KERN_INFO "chan_%d,DATA_BYTE_NUM(0x%x):0x%x\n",i,0xbf261100+i*0x100+0x7c,HAL_GET_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf261100+i*100+0x7c)));

	}
	*/
	printk(KERN_INFO "\n");
}


void ci_wq_func(struct work_struct *work)
{
	struct mt_ci_dev *dev;
	dev = container_of(work,struct mt_ci_dev,ci_wq);
	printk(KERN_INFO "CAM :%s\n",dev->cam_status?"INSERT":"OUT");
}


static irqreturn_t mt_ci_isr(int irq, void *dev_id)
{
	static int old_status = 0;


	struct mt_ci_dev *cidev = (struct mt_ci_dev*)dev_id;
	spin_lock(&cidev->spinlock);
	mt_ci_cam_detect(cidev);
	spin_unlock(&cidev->spinlock);
	if(old_status != cidev->cam_status) {
		schedule_work(&cidev->ci_wq);
		old_status = cidev->cam_status;
	}
	return IRQ_HANDLED;
}

static int  mt_ci_irq_enable(struct mt_ci_dev *dev,int enable)
{
	ci_reg_struct *ci_reg = (ci_reg_struct *)(dev->iobase);
	if (enable) {
		writel(CI_INTST_MASK,&ci_reg->reg_ci_intsts);
		mt_ci_intctrl_set(dev,~(CI_INTCTRL_CAMDEC|CI_INTCTRL_CAMIRQ));
		enable_irq(dev->irq);
	} else {
		disable_irq(dev->irq);
		writel(CI_INTST_MASK,&ci_reg->reg_ci_intsts);
		mt_ci_intctrl_set(dev,~(CI_INTCTRL_CAMDEC|CI_INTCTRL_CAMIRQ));
	}
	return 0;
}


/*
static void mt_ci_slot_reset(struct mt_ci_dev *dev,int reset)
{
	printk(KERN_INFO "mt_ci_slot_reset-----%s\n",reset?"START":"STOP");
	if (reset) {
		reg_write_u32(dev,REG_CI_CTRL,0x02);
	} else {
		reg_write_u32(dev,REG_CI_CTRL,0x01);
	}
}
*/

static void mt_ci_support_power(void )
{
	cicam_pwr_ctrl(0);
	msleep(500);
	cicam_pwr_ctrl(1);
	msleep(500);
}

static int mt_ci_open(struct inode *inode, struct file *fp)
{
	struct mt_ci_dev *dev;
	u32 val = 0;

	printk(KERN_INFO "mt_ci_open start\n");
	dev = container_of(inode->i_cdev,struct mt_ci_dev,cdev);
	if (dev->flags == CI_STATUS_HOST_OPEN) {
		printk(KERN_ERR "mtci device  already opened\n");
		return -EPERM;
	}

	printk(KERN_INFO "support power\n");
	//support power
	mt_ci_support_power();

	mt_ci_cfg_unsetce(dev);
	mt_ci_tsctl_enable(dev,0);
	mt_ci_tsctl_serial(dev,0);

	printk(KERN_INFO "read REG_CI_TSCTRL reg\n");
	val = mtci_reg_read_u32(dev,0x800);
	val &= ~(3<<30);
	mtci_reg_write_u32(dev,0x800,val);

	fp->private_data = dev;
	dev->flags = CI_STATUS_HOST_OPEN;
	printk(KERN_INFO "open mtci device ok\n");

	return 0;
}

static ssize_t mt_ci_read(struct file *fp, char __user *buf,size_t len, loff_t *off)
{
	u32 i = 0;
	char * kbuf;
	struct mt_ci_dev *dev;
	int ret;

	if (!len){
		printk(KERN_WARNING "mtci_read size:0\n");
		return 0;
	}

	dev = (struct mt_ci_dev*)fp->private_data;
	kbuf = kmalloc(len,GFP_KERNEL);
	if (!kbuf) {
		printk(KERN_ERR "mtci_read kmalloc failed\n");
		return -ENOMEM;
	}
	ret = mutex_lock_interruptible(&dev->iomutex);
	if (ret < 0) {
		kfree(kbuf);
		return (ssize_t)ret;
	}
	for(i = 0;i < len;i++) {
		kbuf[i] = mt_cam_read_register(dev,0);
	}
	*off += len;
	if (copy_to_user(buf, kbuf, len)) {
		kfree(kbuf);
		mutex_unlock(&dev->iomutex);
		return -EFAULT;
	}
	kfree(kbuf);
	mutex_unlock(&dev->iomutex);

	return len;
}

static ssize_t mt_ci_write(struct file *fp, const char __user *buf,
			 size_t len, loff_t *off)
{
	u32 i = 0;
	char * kbuf;
	struct mt_ci_dev *dev;
	int rv;

	if (!len){
		printk(KERN_WARNING "mtci_write size:0");
		return 0;
	}

	dev = (struct mt_ci_dev*)fp->private_data;
	kbuf = kmalloc(len,GFP_KERNEL);
	if (!kbuf) {
		printk(KERN_ERR "mtci_write kmalloc failed\n");
		return -ENOMEM;
	}
	rv = mutex_lock_interruptible(&dev->iomutex);
	if (rv < 0) {
		kfree(kbuf);
		return (ssize_t)rv;
	}
	if (copy_from_user(kbuf, buf, len)) {
		kfree(kbuf);
		mutex_unlock(&dev->iomutex);
		return -EFAULT;
	}

	for(i = 0;i < len;i++) {
		mt_cam_write_register(dev,0,kbuf[i]);
		//printk(KERN_INFO "ciwrite,val:%d\n",kbuf[i]);
	}
	*off += len;
	kfree(kbuf);
	mutex_unlock(&dev->iomutex);

	return len;
}

static int mt_cam_memread(struct mt_ci_dev *dev,u32 addr,u32 len,void __user *ptr,uint32_t __user *retp)
{
	mt_u8 *buf;
	int rv = 0;
	int i, j;

    if(addr +2 >=  4096){
        printk(KERN_ERR "the read addr exit the one page:4k\n");
        return  -ENOMEM;
    }

	rv = mutex_lock_interruptible(&dev->iomutex);
	if (rv < 0)
		return rv;
	mt_ci_switch_memmode(dev);
	buf = kmalloc(len,GFP_KERNEL);
	if(!buf) {
		mutex_unlock(&dev->iomutex);
		return -ENOMEM;
	}

	for(i = 0, j = addr;i < len;i++, j+=2) {
		buf[i] = mt_cam_read_register(dev,j);
		printk(KERN_INFO "memread,addr:0x%x,val:0x%x\n",j,buf[i]);
	}
	if (copy_to_user(ptr,buf,i)) {
		rv = -EFAULT;
	}
	if (put_user(i,retp)) {
		rv = -EFAULT;
	}
	kfree(buf);
	mutex_unlock(&dev->iomutex);
	return rv;
}

static int mt_cam_memwrite(struct mt_ci_dev *dev,u32 addr,u32 len,void __user *ptr,uint32_t __user *retp)
{
	mt_u8 *buf;
	int rv = 0;
	int i, j;

	rv = mutex_lock_interruptible(&dev->iomutex);
	if (rv < 0)
		return rv;
	mt_ci_switch_memmode(dev);
	buf = kmalloc(len,GFP_KERNEL);
	if(!buf) {
		mutex_unlock(&dev->iomutex);
		return -ENOMEM;
	}
	if (copy_from_user(buf,ptr,len)) {
		kfree(buf);
		mutex_unlock(&dev->iomutex);
		return -EFAULT;
	}

	for(i = 0, j = addr;i < len;i++, j+=2) {
		 mt_cam_write_register(dev,j,buf[i]);
	}
	if (put_user(i,retp)) {
		rv = -EFAULT;
	}
	kfree(buf);
	mutex_unlock(&dev->iomutex);

	return rv;
}

static int mt_cam_ioread(struct mt_ci_dev *dev,u32 addr,void __user *ptr)
{
	int rv = 0;
	u8 val;
	rv = mutex_lock_interruptible(&dev->iomutex);
	if (rv < 0)
		return rv;

	mt_ci_switch_iomode(dev);
	val =  mt_cam_read_register(dev,addr);
	printk(KERN_INFO "ioread val:%d\n",val);
	if (put_user(val,(u8*)ptr)) {
		rv = -EFAULT;
	}
	mutex_unlock(&dev->iomutex);

	printk(KERN_INFO "ioread,addr:0x%x,val:%d\n",addr,val);
	return rv;
}

static int mt_cam_iowrite(struct mt_ci_dev *dev,u32 addr,void __user *ptr)
{
	int rv = 0;
	u8 val;
	rv = mutex_lock_interruptible(&dev->iomutex);
	if (rv < 0)
		return rv;

	mt_ci_switch_iomode(dev);
	if (get_user(val,(u8*)ptr)) {
		rv = -EFAULT;
		mutex_unlock(&dev->iomutex);
		return rv;
	}

	printk(KERN_INFO "iowrite,addr:0x%x,val:%d\n",addr,val);
	mt_cam_write_register(dev,addr,val);
	mutex_unlock(&dev->iomutex);
	return rv;
}

static long mt_ci_ioctl(struct file *fp,unsigned int cmd,unsigned long arg)
{
	struct mt_ci_dev *dev;
	ci_reg_struct *ci_reg;
	int ret = 0;
	unsigned int arg_kernel;

	void __user *arg_para = (void __user *)arg;
	dev = fp->private_data;
	ci_reg = dev->iobase;

	switch(cmd)
	{
		case CI_IOC_MEMREAD:
			printk(KERN_INFO "CI_IOC_MEMREAD\n");
			struct mtci_memrw mem_read;
			struct mtci_memrw __user *mem_read_arg_user = (struct mtci_memrw*)arg_para;
			if (copy_from_user(&mem_read, arg_para, sizeof(mem_read))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			//printk("addr:0x%x  len:%d\n",memrw.addr,memrw.size);
			ret = mt_cam_memread(dev,mem_read.addr,mem_read.size,mem_read.buffer,&mem_read_arg_user->rsize);
			break;
		case CI_IOC_MEMWRITE:
			printk(KERN_INFO "CI_IOC_MEMWRITE\n");
			struct mtci_memrw mem_write;
			struct mtci_memrw __user *mem_write_arg_user = (struct mtci_memrw*)arg_para;
			if (copy_from_user(&mem_write, arg_para, sizeof(mem_write))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			//printk("addr:0x%lx  size = %d\n",memrw.addr,memrw.buffer);
			ret = mt_cam_memwrite(dev,mem_write.addr,mem_write.size,mem_write.buffer,&mem_write_arg_user->rsize);
			break;
		case CI_IOC_GETPIDRAM:
			printk(KERN_INFO "CI_IOC_GETPIDRAM\n");
			struct mtci_pid_ram pid_get_ram;
			struct mtci_pid_ram __user *pid_get_arg_user = (struct mtci_pid_ram*)arg_para;
			if (copy_from_user(&pid_get_ram, arg_para, sizeof(pid_get_ram))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			arg_kernel = mt_ci_pidram_get(dev,pid_get_ram.index);
			if (put_user(arg_kernel,&pid_get_arg_user->val)) {
				printk(KERN_ERR "put data to user failed\n");
				ret = -EFAULT;
			}
			break;
		case CI_IOC_CIRESET:
			printk(KERN_INFO "CI_IOC_CIRESET\n");
			if (copy_from_user(&arg_kernel, arg_para, sizeof(arg_kernel))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			//mt_ci_slot_reset(dev,arg_kernel);
			break;
		case CI_IOC_IOREAD:
			printk(KERN_INFO "CI_IOC_IOREAD\n");
			struct mtci_iorw io_read;
			if (copy_from_user(&io_read, arg_para, sizeof(io_read))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			ret = mt_cam_ioread(dev,io_read.reg,io_read.buffer);
			break;
		case CI_IOC_IOWRITE:
			printk(KERN_INFO "CI_IOC_IOWRITE\n");
			struct mtci_iorw io_write;
			if (copy_from_user(&io_write, arg_para, sizeof(io_write))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			ret = mt_cam_iowrite(dev,io_write.reg,io_write.buffer);
			break;
		case CI_IOC_CAMSTATUS:
			printk(KERN_INFO "CI_IOC_CAMSTATUS\n");
			unsigned long flag;
			spin_lock_irqsave(&dev->spinlock,flag);
			if (copy_to_user((char __user *)arg_para,&dev->cam_status, sizeof(dev->cam_status))) {
				printk(KERN_ERR "copy to user failed\n");
				ret = -EFAULT;
			}
			spin_unlock_irqrestore(&dev->spinlock,flag);
			break;
		case CI_IOC_TSENABLE:
			printk(KERN_INFO "CI_IOC_TSENABLE\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			mt_ci_tsctl_enable(dev,arg_kernel);
			break;
		case CI_IOC_IOMEMSWITCH:
			printk(KERN_INFO "CI_IOC_IOMEMSWITCH\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			if (arg_kernel == 0 ) {
				printk(KERN_INFO "switch to memory mode\n");
				mt_ci_switch_memmode(dev);
			} else {
				printk(KERN_INFO "switch to io mode\n");
				mt_ci_switch_iomode(dev);
			}
			break;
		case CI_IOC_TS0CTRL:
			printk(KERN_INFO "CI_IOC_TS0CTRL\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel))) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
			}
			//ts0_onoff(arg_v);
			break;
		case CI_IOC_IRQCTL:
			printk(KERN_INFO "CI_IOC_IRQCTL\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			mt_ci_irq_enable(dev,arg_kernel);
			break;
		case CI_IOC_TSINCLK:
			printk(KERN_INFO "CI_IOC_TSINCLK\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			//citsin_clksel(arg_v);
			break;
		case CI_IOC_CDCFG:
			printk(KERN_INFO "CI_IOC_CDCFG\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			writel(arg_kernel,&ci_reg->reg_ci_cdcfg);
			break;
		case CI_IOC_SETTS1CTRL:
			printk(KERN_INFO "CI_IOC_SETTS1CTRL\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			writel(arg_kernel,&ci_reg->reg_ci_ts1ctrl);
			break;
		case CI_IOC_GETTS1CTRL:
			printk(KERN_INFO "CI_IOC_GETTS1CTRL\n");
			arg_kernel = readl(&ci_reg->reg_ci_ts1ctrl);
			if (copy_to_user((char __user *)arg_para,&arg_kernel, sizeof(arg_kernel))) {
				printk(KERN_ERR "copy to user failed\n");
				ret = -EFAULT;
			}
			break;
		case CI_IOC_SETPIDRAM:
			printk(KERN_INFO "CI_IOC_SETPIDRAM\n");
			struct mtci_pid_ram pid_set_ram;
			if (copy_from_user(&pid_set_ram, arg_para, sizeof(pid_set_ram))) {
				printk(KERN_ERR "copy from user failed\n");
				return -EFAULT;
			}
			mt_ci_pidram_set(dev,pid_set_ram.index,pid_set_ram.val);
			break;
		case CI_IOC_TSSERIALEN:
			printk(KERN_INFO "CI_IOC_TSSERIALEN\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			mt_ci_tsctl_serial(dev,arg_kernel);
			break;
		case CI_IOC_CAMPWR:
			printk(KERN_INFO "CI_IOC_CAMPWR\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			cicam_pwr_ctrl(arg_kernel);
			break;
		case CI_IOC_FILTEREN:
			printk(KERN_INFO "CI_IOC_FILTEREN\n");
			if (copy_from_user(&arg_kernel,arg_para,sizeof(arg_kernel)) != 0) {
				printk(KERN_ERR "copy from user failed\n");
				ret = -EFAULT;
				break;
			}
			mt_ci_pidfilter_enable(dev,arg_kernel);
			break;
		case CI_IOC_DUMPREG:
			mt_ci_reg_dump(dev);
			break;
		default:
			ret = -ENOIOCTLCMD;
			break;
	}

	return ret;
}
static int mt_ci_release(struct inode *inode, struct file *fp)
{
	struct mt_ci_dev *dev;

	printk(KERN_INFO "mtci_release\n");
	dev = container_of(inode->i_cdev,struct mt_ci_dev,cdev);
	cicam_pwr_ctrl(0);
	dev->flags = CI_STATUS_HOST_CLOSE;

	return 0;
}

#if defined(CONFIG_COMPAT)
static long mt_ci_compat_ioctl(struct file *filp, unsigned int cmd,
				unsigned long arg)
{
	return mt_ci_ioctl(filp, cmd, (unsigned long)compat_ptr(arg));
}

#endif
static const  struct file_operations mtci_fops =
{
	.owner = THIS_MODULE,
	.read = mt_ci_read,
	.write = mt_ci_write,
	.unlocked_ioctl = mt_ci_ioctl,
#if defined(CONFIG_COMPAT)
	.compat_ioctl = mt_ci_compat_ioctl,
#endif
	.open = mt_ci_open,
	.release = mt_ci_release,
};

#define BOOT_CFG 			SYMPHONY_IO_VA(0xBF140020)

static int mt_ci_get_source_by_dts(struct platform_device *pdev,struct mt_ci_dev *cidev)
{
	struct resource *res;
	int ret;
	unsigned int cam_offset;
	//get gpio :but gpio system is not complete
	/*cidev->gd_power = devm_gpiod_get(&pdev->dev,"power",GPIOD_OUT_HIGH);
	if (IS_ERR(cidev->power)) {
		printk("get ciplus power gpio pin failed\n");
		return PTR_ERR(cidev->power);
	}
	*/

	//get device node
	cidev->dev_nd = of_find_compatible_node(NULL, NULL,"montage,symphony_ci");
	if(NULL == cidev->dev_nd) {
		printk(KERN_ERR "get dts device node failed\n");
		return -EPERM;
	}


	//get ci reg base and ioremap
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		printk(KERN_ERR "platform get resource faild\n");
		return -EPERM;
	}
	cidev->flag_res = 1;
	printk(KERN_INFO "using ioremap\n");
	cidev->iobase = (ci_reg_struct *)devm_ioremap(&pdev->dev,res->start,resource_size(res));
	printk(KERN_INFO "cidev->iobase:0x%p,ci_reg start:0x%llx,ci_reg_end:0x%llx\n",cidev->iobase,res->start,res->end);


	//get cam reg base and ioremap
	ret = of_property_read_u32(cidev->dev_nd, "cam-reg-offset", &cam_offset);
	if(ret != 0) {
		printk(KERN_ERR "get cam reg base property failed\n");
		return ret;
	}
	printk(KERN_INFO "cam_offset = 0x%x\n",cam_offset);
	cidev->cam_reg_base = devm_ioremap(&pdev->dev,res->start + cam_offset,4096);
	printk(KERN_INFO "cidev->cam_reg_base:0x%p,dts_get_value:0x%x\n",cidev->cam_reg_base,cam_offset);


	//get clk resource
	cidev->pclk = devm_clk_get(&pdev->dev,NULL);
	if (IS_ERR(cidev->pclk)) {
		printk(KERN_ERR "get ciplus clk failed\n");
		return PTR_ERR(cidev->pclk);;
	}

	//get reset control handle
	cidev->reset = devm_reset_control_get(&pdev->dev,NULL);
	if (IS_ERR(cidev->reset)) {
		printk(KERN_ERR "get ciplus reset failed\n");
		return PTR_ERR(cidev->reset);
	}

	//get irq
	cidev->irq = platform_get_irq(pdev,0);
	if(cidev->irq<0){
		printk(KERN_ERR "get ciplus irq failed\n");
		return cidev->irq;
	}

	return 0;
}



static int mt_ci_setup_cdev(struct mt_ci_dev *cidev)
{
	int ret = 0;
	int devno = MKDEV(cidev->major,cidev->minor);

	// cdev_Init
	cdev_init(&cidev->cdev, &mtci_fops);
	cidev->cdev.owner = THIS_MODULE;
	//cdev add
	ret  = cdev_add (&cidev->cdev, devno, 1);
	if (ret ) {
		printk(KERN_WARNING "Error %d adding mt ci%d", ret , cidev->minor);
	}

	return ret;
}



/*
* this function be called when find the platform device
*/
static int  mt_ci_probe(struct platform_device *pdev)
{
	dev_t devno;
	struct mt_ci_dev *cidev;
	int ret;

	printk(KERN_INFO "[%s-%d]:probe start !!!!!\n",__func__,__LINE__);
	printk(KERN_INFO "MT ciplus Device Driver, Version 1.00***\n");

#ifdef CONFIG_MT_CHIP_SYMPHONY6
	/* Package ID Check */
	package_id_symphony_t pkg_id;

	pkg_id = chip_package_get();
	if (pkg_id == PACKET_CHIP_BGA_21x21_NOSIP
		|| pkg_id == PACKET_CHIP_BGA_21x21_SIPBD)
	{
		dev_err(&pdev->dev,"CI package id(0x%X) disabled!\n",pkg_id);
		return -EACCES;
	}
#endif

    /* space alloc */
    printk(KERN_INFO "ci_driver alloc space for ci_dev\n");
	cidev = devm_kmalloc(&pdev->dev, sizeof(*cidev), GFP_KERNEL);
	if (!cidev) {
		return -ENOMEM;
	}
	memset(cidev,0,sizeof(*cidev));

	// get ci_dev source by dts
	printk(KERN_INFO "get source by dts\n");
	ret = mt_ci_get_source_by_dts(pdev,cidev);
	if (ret < 0) {
		printk(KERN_ERR "get ciplus dts info failed\n");
		goto get_res_failed;
	}

	// init lock
	printk(KERN_INFO "%s-%d\n",__func__,__LINE__);
	mutex_init(&cidev->iomutex);
	spin_lock_init(&cidev->spinlock);

	// enable clk
	printk(KERN_INFO "clock enable\n");
	ret = clk_prepare_enable(cidev->pclk);
	if (ret < 0) {
		printk(KERN_ERR "enable ciplus clk failed\n");
		goto get_res_failed;
	}

    //in order to set pinmux??
	ciclk_ciahb_reset(0);
	msleep(1);
	ciclk_ciahb_reset(1);
	msleep(100);

	//ci support power pin set to out gpio mode
 	mt_ci_gpio_power();

	//relate to permission
	mt_ci_relate_to_permission();

	//detect cam status
	mt_ci_cam_detect(cidev);
	printk(KERN_INFO "CAM :%s\n",cidev->cam_status?"INSERT":"OUT");

	//relate to sym4 chip
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	writel(0xffff,ci_reg_vitual_addr(cidev->iorebase,REG_CI_CDCFG));
#endif

	// irq request:set irq function
	printk(KERN_INFO "%s-%d\n",__func__,__LINE__);
	mt_ci_intctrl_set(cidev,~(CI_INTCTRL_CAMDEC));
	ret = request_irq(cidev->irq,mt_ci_isr,IRQF_SHARED,"mtcipus irq",cidev);
	if (ret < 0) {
		printk(KERN_ERR "request ciplus irq failed\n");
		goto clk_disable_unpre;
	}

	//INIT work queue
	INIT_WORK(&(cidev->ci_wq),ci_wq_func);

	// get devno:major+minor
	ret = alloc_chrdev_region(&devno, 0, MAX_OPEN, MTCI_NAME);
	if (ret < 0) {
		printk(KERN_ERR "ciplus alloc_chrdev_region failed\n");
		goto freeirq;
	}
	cidev->major = MAJOR(devno);
	cidev->minor = MINOR(devno);
	printk(KERN_INFO "alloc devno success major:%d  minor:%d\n",cidev->major,cidev->minor);

	ret = mt_ci_setup_cdev(cidev);
	if (ret<0) {
		printk("mt_ci_setup_cdev failed\n");
		goto chr_remove;
	}

	// class create
	cidev->ci_class  = class_create("mtci-dev");
	if (IS_ERR(cidev->ci_class)) {
		ret = PTR_ERR(cidev->ci_class);
		goto del_cdev;
	}

	//device create
	cidev->dev = device_create(cidev->ci_class,NULL,devno,NULL,"mt_ci%d",cidev->minor);
	if (IS_ERR(cidev->dev)) {
		ret = PTR_ERR(cidev->dev);
		goto cls_destroy;
	}

	//  set cidev to pdev->dev
    platform_set_drvdata(pdev, cidev);

	//mt_ci_reg_dump(cidev);

	printk(KERN_INFO "mt_ci: driver initialized\n");
	return 0;

cls_destroy:
	class_destroy(cidev->ci_class);
del_cdev:
	cdev_del(&cidev->cdev);
chr_remove:
	unregister_chrdev_region(devno, MAX_OPEN);
freeirq:
	free_irq(cidev->irq,cidev);
clk_disable_unpre:
	clk_disable_unprepare(cidev->pclk);
get_res_failed:
	if (cidev->flag_res) {
		devm_iounmap(&pdev->dev,cidev->iobase);
	}
	kfree(cidev);
	printk(KERN_INFO "mt_ci: probe failed\n");
    return ret;
}

/*****************************************************************************/
static int mt_ci_remove(struct platform_device *pdev)
{
	struct mt_ci_dev *cidev = platform_get_drvdata(pdev);
	dev_t devno = MKDEV(cidev->major,cidev->minor);
	printk(KERN_DEBUG "device destroy\n");
	device_destroy(cidev->ci_class,devno);

	printk(KERN_DEBUG "class destroy\n");
	class_destroy(cidev->ci_class);

	printk(KERN_DEBUG "cdev_del\n");
	cdev_del(&cidev->cdev);

	printk(KERN_DEBUG "unregister chrdev\n");
	unregister_chrdev_region(devno, MAX_OPEN);

	printk(KERN_DEBUG "free irq\n");
	free_irq(cidev->irq,cidev);
	//devm_request_irq can free the irq when unregister the chrdev

	printk(KERN_DEBUG "clk_disable_unprepare\n");
	clk_disable_unprepare(cidev->pclk);
	if (cidev->flag_res) {
		printk(KERN_DEBUG"devm_iounmap\n");
		devm_iounmap(&pdev->dev,cidev->iobase);
	}

	printk(KERN_DEBUG "free cidev\n");
	kfree(cidev);

    return 0;
}

static int	mt_ci_suspend(struct platform_device *pltdev,pm_message_t state)
{
	unsigned long reg = 0;
	ci_reg_struct *ci_reg;

	struct mt_ci_dev * ci_dev = (struct mt_ci_dev *)platform_get_drvdata(pltdev);
	ci_reg = ci_dev->iobase;
	mt_ci_tsctl_enable(ci_dev,0);
	reg = SYMPHONY_IO_VA(TS_SRC_SEL);
	ci_dev->resume_regs[0] = HAL_GET_U32((volatile u32*)reg);

	reg = SYMPHONY_IO_VA(TSI_CLKSEL_REG);
	ci_dev->resume_regs[1] = HAL_GET_U32((volatile u32*)reg);

	reg = SYMPHONY_IO_VA(TS3_SAMPLE_CTRL);
	ci_dev->resume_regs[2] = HAL_GET_U32((volatile u32*)reg);

	reg = SYMPHONY_IO_VA(CI_CLKSEL_REG);
	ci_dev->resume_regs[3] = HAL_GET_U32((volatile u32*)reg);

	ci_dev->resume_regs[4] = readl(&ci_reg->reg_ci_fsmcnt1);
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	ci_dev->resume_regs[5] = reg_read_u32(ci_dev,REG_CI_TSCTRL);
#endif
	mt_ci_reg_dump(ci_dev);
	printk(KERN_INFO "mt_ci_suspend\n");

	return 0;
}

static int	mt_ci_resume(struct platform_device *pltdev)
{
	unsigned long reg = 0;
	ci_reg_struct *ci_reg;

	struct mt_ci_dev * ci_dev = (struct mt_ci_dev *)platform_get_drvdata(pltdev);
	ci_reg = ci_dev->iobase;
	printk(KERN_INFO "***mt_ci_resume\n");
	mt_ci_reg_dump(ci_dev);
	reg = SYMPHONY_IO_VA(TS_SRC_SEL);
	HAL_PUT_U32((volatile u32*)reg,ci_dev->resume_regs[0]);

	reg = SYMPHONY_IO_VA(TSI_CLKSEL_REG);
	HAL_PUT_U32((volatile u32*)reg,ci_dev->resume_regs[1]);

	reg = SYMPHONY_IO_VA(TS3_SAMPLE_CTRL );
	HAL_PUT_U32((volatile u32*)reg,ci_dev->resume_regs[2]);

	reg = SYMPHONY_IO_VA(CI_CLKSEL_REG );
	HAL_PUT_U32((volatile u32*)reg,ci_dev->resume_regs[3]);

	writel(ci_dev->resume_regs[4],&ci_reg->reg_ci_fsmcnt1);
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	reg_write_u32(ci_dev,REG_CI_TSCTRL,ci_dev->resume_regs[5]);
#endif

	return 0;
}

static const struct of_device_id mt_ci_match_table[] = {
    { .compatible = "montage,symphony_ci" },
    {},
};

MODULE_DEVICE_TABLE(of, mt_ci_match_table);

static struct platform_driver mt_ci_pltdrv =
{
    .driver = {
    	.name = "mt-ci",
		.of_match_table = mt_ci_match_table,
    	.owner = THIS_MODULE,
	},
    .probe = mt_ci_probe,
    .remove = mt_ci_remove,
    .suspend = mt_ci_suspend,
	.resume = mt_ci_resume,
};

module_platform_driver(mt_ci_pltdrv);


MODULE_DESCRIPTION("Driver for montage ciplus device");
MODULE_LICENSE("GPL");

