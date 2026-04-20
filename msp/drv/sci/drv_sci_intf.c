/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
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
#include <linux/delay.h>
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
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/of.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include "mt_mach/irq.h"

#include "drv_sci_ioctl.h"
#include "drv_sci_priv.h"
#include "drv_sci_regs.h"
#include "mt_mach/chipinfo.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mt_type.h"
#include "mt_osal.h"
#include "mt_drv_dev.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dma.h"
#include "mt_module_debug.h"
#include "mt_drv_proc.h"
#include "mt_cache.h"
#include "drv_gpio_ioctl.h"
#include "mach/otp_simple.h"

#define R_KEY_ADC2_CFG  	SYMPHONY_IO_VA(0xbf520000)
#define R_KEY_ADC2_DATA  	SYMPHONY_IO_VA(0xbf520004)
#define R_KEY_ADC2_INT  	SYMPHONY_IO_VA(0xbf520008)
#define R_KEY_ADC2_RANGE  	SYMPHONY_IO_VA(0xbf52000c)
#define R_KEY_ADC2_SAMPL  	SYMPHONY_IO_VA(0xbf520010)

#define R_KEY_5V_CLK  	SYMPHONY_IO_VA(0xbf13c034)
#define R_KEY_5V_RST  	SYMPHONY_IO_VA(0xbf13c038)
#define R_KEY_5V_DAT  	SYMPHONY_IO_VA(0xbf13c03c)

#define CHIP_VERIFY_LOG

#ifdef CHIP_VERIFY_LOG
#define chipverify_log(fmt...)     printk(KERN_EMERG fmt)
#else
#define chipverify_log(fmt...)     do{}while(0)
#endif

#ifdef CONFIG_MT_FPGA
//#define FPGA_SMC_BASE_CLOCK  13500000//24000000
#define FPGA_SMC_BASE_CLOCK  24000000
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "mt_mach/clock.h"
#endif

#ifndef READ_REG8
#define READ_REG8(r)			HAL_GET_U8((volatile u8 *)(r))
#define WRITE_REG8(v, r)		HAL_PUT_U8((volatile u8 *)(r), (u8)(v))
#endif

#ifndef READ_REG32
#define READ_REG32(r)			HAL_GET_U32((volatile u32 *)(r))
#define WRITE_REG32(v, r)		HAL_PUT_U32((volatile u32 *)(r), (u32)(v))
#endif

#define SCI_NAME "sci"
#define SCI_MINOR   UMAP_MIN_MINOR_SCI
#define SCI_MINORS  UMAP_DEV_NUM_SCI

//#define SYM6_SMC_DMA

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
//#define SMC_DETECT_KADC2        //can't test hardware poweroff
#define SMC_ENABLE_5VIO
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define SMC_DETECT_KADC2        //can't test hardware poweroff
#define SMC_ENABLE_5VIO
#endif

#define SMC_INSERT	(1)
#define SMC_REMOVE	(0)

#define SMC_DATA_BUFF_SIZE	(512)
#if 0
#define R_SMC_RST_ADDR  	SYMPHONY_IO_VA(0xbf50900c)
#define R_SMC_CLKEN_ADDR  	SYMPHONY_IO_VA(0xbf509000)
#define R_SMC_CLKCTR_ADDR  	SYMPHONY_IO_VA(0xbf50f804)
#endif
static mt_device_s g_SciRegisterData;
static unsigned char g_smc_buff[SMC_DATA_BUFF_SIZE]= {0};
//static atomic_t g_SciCount = ATOMIC_INIT(0);

#define PROC_NAME_LEN (8)
#define PROC_PARAM_MAXLEN (64)
#define DEBUG 0

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
typedef struct mt_smc_resume {
    unsigned long reg_soft;
} mt_smc_resume_t;

//static struct mt_smc_resume g_smc_suspend_info;
#endif

static char* gInit_str = "init";
static char* gDetect_str = "detect";
static char* gVcc_str = "vcc";
static char* gFrq_str = "frq";
static char* gNguard_str = "nguard";
static char* gStopwidth_str = "stopwidth";
static char* gParity_str="parity";
static char* gTimeout_str = "timeout";
static char* gAtr_str = "atr";
static char* gData_str = "data";
static char* gHelp_str = "help";

#if defined(CONFIG_MT_CHIP_ARIA)
extern unsigned int mt_get_sys_ctrl_base(void);//0xffaf0000
extern unsigned int aria_get_pads0_ctrl_base(void);//0xffad0000
#endif

#define R_PINMUX_ADDR1			  SYMPHONY_IO_VA(0xBF15B400)
#define R_PINMUX_ADDR2			  SYMPHONY_IO_VA(0xBF15B404)
#define R_PINMUX_SWPIN1_ADDR	SYMPHONY_IO_VA(0xBF13C004)
#define R_PINMUX_SWPIN2_ADDR	SYMPHONY_IO_VA(0xBF13C008)
#ifndef R_CHIP_ID_RDEN
#define R_CHIP_ID_RDEN			  SYMPHONY_IO_VA(0xBF140008)
#endif
#define R_SYMPHONY_CHIP_ID 		SYMPHONY_IO_VA(0xBF140004)
#define R_CLK_SMC_CFG 			  SYMPHONY_IO_VA(0xBF500048)

#define R_SMC_SOFTRESET       SYMPHONY_IO_VA(0xBF50900c)

#define SMC_CLK_ENABLE	(0x1)
#define KADC_POWER_DOWN	(0x2)
#define KADC_RANGE (0x10)

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
static void sci_clock_config(u_char sci0_clkbase, u_char sci1_clkbase);
#endif

extern mt_s32 mt_drv_module_unregister(mt_u32 u32ModuleID);
extern mt_s32 mt_drv_module_register(mt_u32 u32ModuleID, const mt_u8* pu8ModuleName, mt_void* pFunc);
static void sci_config_code_convention(struct sci_info *info);
irqreturn_t sci_interrupt(int irq, void *dev_id);
static void sci_interrupt_enable(struct sci_info *info);
#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
static void sci_dump_registers(struct sci_info *info);
#endif
static void sci_interrupt_disable(struct sci_info *info);
static void sci_deactivate(struct sci_info *info,int flag_sequnce);
static u_char sci_get_card_status(struct sci_info *info);
mt_s32 mt_osal_snprintf(mt_char *pszstr, mt_size_t ullen, const mt_char *pszformat, ...);

static SCI_ATTR_S default_sciattr = {
    .read_timeout = (9600 * 372 * 100) /357,
    .write_timeout = 0,
    .rst_timeout = (40000+1600),
    .rece_timeout = 0,
    .blk_timeout = 0,
    .etu = 372,
    .Hz = 3570000,
    .N = 0,
    .stop_width = SCI_TWO_STOPS,
    .error_handle_en = 0,
    .parity_en = 1,
    .slot_type = SCI_SLOT_ALWAYS_OPEN,
    .vcc_en_level = 0,
#if defined(CONFIG_MT_FPGA) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    .clkpin_mode = SCI_PINMODE_BY_CHIP,
    .rstpin_mode = SCI_PINMODE_BY_CHIP,
    .iopin_mode = SCI_PINMODE_BY_CHIP,
#else
    .clkpin_mode = SCI_PINMODE_BY_PULLUP,
    .rstpin_mode = SCI_PINMODE_BY_PULLUP,
    .iopin_mode = SCI_PINMODE_BY_PULLUP,
#endif
    .rx_retrys = 0,
    .tx_retrys = 0,
    .tx_finish_en = 0,
    .recetime_en = 0,
    .blktime_en = 0,
    .rsttime_en = 0,
    .type = 0,
};

struct sci_device {
    dev_t devt;
    u32 idx;
    struct device *p_dev;
    struct device *dev;
    struct sci_priv_data priv;
};

struct sci_driver {
    struct cdev cdev;
    struct sci_device *sci_dev[SCI_MINORS];
    dev_t devt;
    dev_t major;
    dev_t minor;
    u32 minors;
};

static struct sci_driver *sci_drv=NULL;
static struct class *sci_class=NULL;
static struct sci_info sciinfo[2];
static u32 gatingoff=0;

static void sci_set_clock_freq(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_FRQ_CFG);
    reg_freq_cfg *tmp = (reg_freq_cfg *)&reg;

    if(info->attr.Hz == attr->Hz && info->set_attr_cnt > 0) {
        return;
    }

    info->attr.Hz = attr->Hz;
    tmp->freq_div = info->clkbase / (2 * info->attr.Hz);
    /*
    if((info->clkbase % (2 * info->attr.Hz)) > 0)
    tmp->freq_div ++;*/
    WRITE_REG8(reg, info->regbase + R_SMC_FRQ_CFG);
}

static void sci_set_slot_type(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_FRQ_CFG);
    reg_freq_cfg *tmp = (reg_freq_cfg *)&reg;

    if(info->attr.slot_type == attr->slot_type && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.slot_type = attr->slot_type;
    tmp->slot_type = info->attr.slot_type;
    //#ifdef SMC_DETECT_KADC2
    //tmp->slot_type = 0;     //if use kadc, this bit must be 0!
    //#endif
    WRITE_REG8(reg, info->regbase + R_SMC_FRQ_CFG);
}

static void sci_set_N(struct sci_info *info, SCI_ATTR_S *attr)
{
    u_char reg = READ_REG8(info->regbase + R_SMC_NC_SET);
    volatile u_char reg1 = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    reg_trans_ctrl *tmp = (reg_trans_ctrl *)&reg1;
    if(info->attr.N == attr->N && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.N = attr->N;
    if(info->attr.N == 0) {
        tmp->ncset_en = 0;
    } else {
        reg = info->attr.N;
        tmp->ncset_en = 1;
    }

    WRITE_REG8(reg, info->regbase + R_SMC_NC_SET);
    WRITE_REG8(reg1, info->regbase + R_SMC_TRANS_CTRL);
}

static void sci_set_stop_width(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    reg_trans_ctrl *tmp = (reg_trans_ctrl *)&reg;
    if(info->attr.stop_width == attr->stop_width && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.stop_width = attr->stop_width;
    tmp->stop_width = info->attr.stop_width;
    WRITE_REG8(reg, info->regbase + R_SMC_TRANS_CTRL);

}

static void sci_set_error_handle_enable(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    reg_trans_ctrl *tmp = (reg_trans_ctrl *)&reg;
    if(info->attr.error_handle_en == attr->error_handle_en && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.error_handle_en = attr->error_handle_en;
    tmp->check_en = info->attr.error_handle_en;
    WRITE_REG8(reg, info->regbase + R_SMC_TRANS_CTRL);
}

static void sci_set_parity_enable(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    reg_trans_ctrl *tmp = (reg_trans_ctrl *)&reg;
    if(info->attr.parity_en == attr->parity_en && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.parity_en = attr->parity_en;
    tmp->parity_en = info->attr.parity_en;
    WRITE_REG8(reg, info->regbase + R_SMC_TRANS_CTRL);
}

static void sci_set_etu(struct sci_info *info, SCI_ATTR_S *attr)
{
    u_long etu = 0;
    volatile u_char reg1 = READ_REG8(info->regbase + R_SMC_ETU1_SET);
    reg_etu1_set *tmp = (reg_etu1_set *)&reg1;
    u_char reg2 = READ_REG8(info->regbase + R_SMC_ETU2_SET);
    u_char reg3 = READ_REG8(info->regbase + R_SMC_ETU3_SET);

    if(info->attr.etu == attr->etu && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.etu = attr->etu;
    etu = info->attr.etu ;
    tmp->etu_17_16 = (etu >> 16) & 0x3;
    //tmp->cvtset_en = 1;
    //tmp->cvtset_value = 0;
    WRITE_REG8(reg1, info->regbase + R_SMC_ETU1_SET);
    reg2 = (etu >> 8) & 0xff;
    WRITE_REG8(reg2, info->regbase + R_SMC_ETU2_SET);
    reg3 =  etu & 0xff;
    WRITE_REG8(reg3, info->regbase + R_SMC_ETU3_SET);
}

static void sci_set_type(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg1 = READ_REG8(info->regbase + R_SMC_ETU1_SET);
    reg_etu1_set *tmp = (reg_etu1_set *)&reg1;
    if((attr->type & 0x80) == 0x80) {
        tmp->cvtset_en = 1;
        if((attr->type & 0x1) == 1) {
            tmp->cvtset_value = 0;
        } else {
            tmp->cvtset_value = 1;
        }
        WRITE_REG8(reg1, info->regbase + R_SMC_ETU1_SET);
    }
    info->attr.type = attr->type;
}

static void sci_set_clkpin_mode(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_PIN_CFG);
    reg_pin_cfg *tmp = (reg_pin_cfg *)&reg;
    if(info->attr.clkpin_mode == attr->clkpin_mode && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.clkpin_mode = attr->clkpin_mode;
    tmp->clkpin_mode = info->attr.clkpin_mode;
    WRITE_REG8(reg, info->regbase + R_SMC_PIN_CFG);
}

static void sci_set_rstpin_mode(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_PIN_CFG);
    reg_pin_cfg *tmp = (reg_pin_cfg *)&reg;
    if(info->attr.rstpin_mode == attr->rstpin_mode && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.rstpin_mode = attr->rstpin_mode;
    tmp->rstpin_mode = info->attr.rstpin_mode;
    WRITE_REG8(reg, info->regbase + R_SMC_PIN_CFG);
}

static void sci_set_iopin_mode(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_PIN_CFG);
    reg_pin_cfg *tmp = (reg_pin_cfg *)&reg;
    if(info->attr.iopin_mode == attr->iopin_mode && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.iopin_mode = attr->iopin_mode;
    tmp->iopin_mode = info->attr.iopin_mode;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    tmp->reserve0 = 0;
#else
    tmp->reserve0 = 1;
#endif
    WRITE_REG8(reg, info->regbase + R_SMC_PIN_CFG);
}

static void sci_set_rxd_resends(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_RESEND_CFG);
    reg_resend_cfg *tmp = (reg_resend_cfg *)&reg;
    if(info->attr.rx_retrys == attr->rx_retrys && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.rx_retrys = attr->rx_retrys;
    tmp->rxd_num = info->attr.rx_retrys;
    WRITE_REG8(reg, info->regbase + R_SMC_RESEND_CFG);
}

static void sci_set_txd_resends(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_RESEND_CFG);
    reg_resend_cfg *tmp = (reg_resend_cfg *)&reg;
    if(info->attr.tx_retrys == attr->tx_retrys && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.tx_retrys = attr->tx_retrys;
    tmp->txd_num = info->attr.tx_retrys;
    WRITE_REG8(reg, info->regbase + R_SMC_RESEND_CFG);
}

static void sci_set_txfinish_enable(struct sci_info *info, SCI_ATTR_S *attr)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_MODE_CFG);
    reg_mode_cfg *tmp = (reg_mode_cfg *)&reg;
    if(info->attr.tx_finish_en == attr->tx_finish_en && info->set_attr_cnt > 0) {
        return;
    }
    info->attr.tx_finish_en = attr->tx_finish_en;
    tmp->txfinsh_en = info->attr.tx_finish_en;
    if(gatingoff){
        reg |= (1<<2);
        printk(KERN_EMERG "++gating off\n");
    }
    WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);
}

static void sci_set_rxtimeout_enable(struct sci_info *info, SCI_ATTR_S *attr)
{
    u_long time = 0;
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_MODE_CFG);
    reg_mode_cfg *tmp = (reg_mode_cfg *)&reg;

    if((info->attr.recetime_en == attr->recetime_en) && (info->attr.rece_timeout == attr->rece_timeout)) {
        return;
    }

    if(0 == attr->rece_timeout) {
        info->attr.recetime_en = 0;
        tmp->rece_time_en = info->attr.recetime_en;
        WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);
        return;
    }
    info->attr.recetime_en = attr->recetime_en;
    tmp->rece_time_en = info->attr.recetime_en;
    WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);

    info->attr.rece_timeout = attr->rece_timeout;
    time = attr->rece_timeout;
    WRITE_REG8((time & 0xff), info->regbase + R_SMC_RCV_TIMEOUT_CNT0);
    WRITE_REG8(((time >> 8) & 0xff), info->regbase + R_SMC_RCV_TIMEOUT_CNT1);
    WRITE_REG8(((time >> 16) & 0xff), info->regbase + R_SMC_RCV_TIMEOUT_CNT2);
    WRITE_REG8(((time >> 24) & 0xff), info->regbase + R_SMC_RCV_TIMEOUT_CNT3);
}

static void sci_set_blktimeout_enable(struct sci_info *info, SCI_ATTR_S *attr)
{
    u_long time = 0;
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_MODE_CFG);
    reg_mode_cfg *tmp = (reg_mode_cfg *)&reg;
    if((info->attr.blktime_en == attr->blktime_en) && (info->attr.blk_timeout == attr->blk_timeout)) {
        return;
    }
    if(0 == attr->blk_timeout) {
        info->attr.blktime_en = 0;
        tmp->blk_time_en = info->attr.blktime_en;
        WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);
        return;
    }
    info->attr.blktime_en = attr->blktime_en;
    tmp->blk_time_en = info->attr.blktime_en;
    WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);

    info->attr.blk_timeout = attr->blk_timeout;
    time = attr->blk_timeout;
    WRITE_REG8((time & 0xff), info->regbase + R_SMC_BLK_TIMEOUT_CNT0);
    WRITE_REG8(((time >> 8) & 0xff), info->regbase + R_SMC_BLK_TIMEOUT_CNT1);
    WRITE_REG8(((time >> 16) & 0xff), info->regbase + R_SMC_BLK_TIMEOUT_CNT2);
    WRITE_REG8(((time >> 24) & 0xff), info->regbase + R_SMC_BLK_TIMEOUT_CNT3);
}

static void sci_set_atrtimeout_enable(struct sci_info *info, SCI_ATTR_S *attr)
{
    u_long time = 0;

    volatile u_char reg_check = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_MODE_CFG);
    reg_trans_ctrl *tmp_check = (reg_trans_ctrl *)&reg_check;
    reg_mode_cfg *tmp = (reg_mode_cfg *)&reg;
    if((info->attr.rsttime_en == attr->rsttime_en) && (info->attr.rst_timeout == attr->rst_timeout)) {
        return;
    }

    if(0 == attr->rst_timeout) {
        info->attr.rsttime_en = 0;
        info->attr.rst_timeout = 0;
        tmp->rst_time_en = info->attr.rsttime_en;
        WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);
        tmp_check->check_en = 0;
        WRITE_REG8(reg_check, info->regbase + R_SMC_TRANS_CTRL);
        return;
    }
    info->attr.rsttime_en = attr->rsttime_en;
    tmp->rst_time_en = info->attr.rsttime_en;
    WRITE_REG8(reg, info->regbase + R_SMC_MODE_CFG);

    info->attr.rst_timeout = attr->rst_timeout;
    time = attr->rst_timeout;
    WRITE_REG8((time & 0xff), info->regbase + R_SMC_RST_TIMEOUT_CNT0);
    WRITE_REG8(((time >> 8) & 0xff), info->regbase + R_SMC_RST_TIMEOUT_CNT1);
    WRITE_REG8(((time >> 16) & 0xff), info->regbase + R_SMC_RST_TIMEOUT_CNT2);
    WRITE_REG8(((time >> 24) & 0xff), info->regbase + R_SMC_RST_TIMEOUT_CNT3);

    tmp_check->check_en = 0;
    WRITE_REG8(reg_check, info->regbase + R_SMC_TRANS_CTRL);
}

static void sci_set_recvtimeout_discarding(struct sci_info *info)
{
    //1:discard. outo clear!
    volatile u8 regv = READ_REG8(info->regbase + R_SMC_RCVINT_DISCARD);
    reg_discard_recv_c8 *tmp = (reg_discard_recv_c8 *)&regv;
    tmp->discardrecv=1;
    WRITE_REG8(regv, info->regbase + R_SMC_RCVINT_DISCARD);
    chipverify_log("+k.recv completed\n");
}
static int sci_set_softreset(struct sci_info *info)
{
    volatile u8 regv8 = 0xff;
    volatile u32 regv32 = 0xff;

    chipverify_log("+k.s.rset in\n");

    WRITE_REG8(regv8, info->regbase + R_SMC_ETU2_SET);
    regv32=READ_REG32(R_SMC_SOFTRESET);
    regv32=(regv32&(~0x03));   //bit01 down
    WRITE_REG32(regv32,R_SMC_SOFTRESET);
    udelay(2);
    regv32=(regv32|0x03);      //bit01 up
    WRITE_REG32(regv32,R_SMC_SOFTRESET);

    regv8 = READ_REG8(info->regbase + R_SMC_ETU2_SET);
    if(1 != regv8) {
        return -1;
    }
    return 0;
}
static int sci_set_hardwarepoweroff(struct sci_info *info,MT_UNF_SCI_H_PWROFF_S *p_deactive_timing)
{
    if(p_deactive_timing->enable) {
        u32 apb_clk = 0;
        u32 smc_clksel = 0;
        u32 regval = 0;

#ifdef CONFIG_MT_FPGA
        smc_clksel=0;           //for compile
        apb_clk = FPGA_SMC_BASE_CLOCK;
#else
        smc_clksel = READ_REG32(SYMPHONY_IO_VA(0xbf509004));
        chipverify_log("++k.clksel=%x\n",smc_clksel);
        if((smc_clksel & 0x1) == 0) {   //xtal/2
            regval=READ_REG32(SYMPHONY_IO_VA(0xbf140020));
            regval=(regval>>30);
            if(0==regval){//27M
                apb_clk = (27000000>>1);
            }else{
                apb_clk = (24000000>>1);
            }
        } else {
            apb_clk = 90000000;
        }
#endif
        regval = (apb_clk /1000000) * p_deactive_timing->T_cnt;
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_T_CNT_20), (u8)(regval & 0xff));
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_T_CNT_21), (u8)((regval >> 8) & 0xff));
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_T_CNT_22), (u8)((regval >> 16) & 0xff));
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_T_CNT_23), (u8)((regval >> 24) & 0xff));

        regval = (p_deactive_timing->enable & 0x1) << 24 | (p_deactive_timing->remove_mode & 0x1) << 26 |
                 (p_deactive_timing->rst_to_clk_cnt & 0xff) |(p_deactive_timing->rst_to_io_cnt & 0xff) << 8 |
                 (p_deactive_timing->rst_to_vcc_cnt & 0xff) << 16;
        {//rising or  failing according to open or close.
            u8 regv8 = READ_REG8(info->regbase + R_SMC_FRQ_CFG);
            if (regv8&0x1) {
                regval |= (0x4<<24);    //set 98 bit2
            }
        }
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_0), (u8)(regval & 0xff));
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_1), (u8)((regval >> 8) & 0xff));
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_2), (u8)((regval >> 16) & 0xff));
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_3), (u8)((regval >> 24) & 0xff));
        chipverify_log("++k.hpoweroff enbaled\n");
    } else {
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_0), 0x21);
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_1), 0x03);
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_2), 0);
        HAL_PUT_U8((volatile u8 *)(info->regbase + R_SMC_CFG_3), 0);
    }
    return 0;
}

static int sci_set_overload(struct sci_info *info, u32 overload)
{
    volatile u8 regv8 = 0;

    chipverify_log("+k.s.overload in %x\n",(mt_u32)overload);

    if(1 == overload) {         //open func
        regv8 = READ_REG8(info->regbase + R_SMC_5VIO_CTRL2);
        regv8 |= (0x3<<3);  //bit3 bit4
        WRITE_REG8(regv8, info->regbase + R_SMC_5VIO_CTRL2);

        WRITE_REG8(0xa0, info->regbase + R_SMC_OVERLOAD_CNT0);
        WRITE_REG8(0x0f, info->regbase + R_SMC_OVERLOAD_CNT1);
        regv8 = READ_REG8(info->regbase + R_SMC_MODE_CFG);
        regv8 |= (0x1<<3);  //bit3
        WRITE_REG8(regv8, info->regbase + R_SMC_MODE_CFG);
    } else if (0 == overload){  //close func
        regv8 = READ_REG8(info->regbase + R_SMC_5VIO_CTRL2);
        regv8 &= ~(0x3<<3);  //bit3 bit4
        WRITE_REG8(regv8, info->regbase + R_SMC_5VIO_CTRL2);
        regv8 = READ_REG8(info->regbase + R_SMC_MODE_CFG);
        regv8 &= ~(0x1<<3);  //bit3
        WRITE_REG8(regv8, info->regbase + R_SMC_MODE_CFG);
    } else if (2 == overload){  //realease 'overload'
        regv8 = READ_REG8(info->regbase + R_SMC_5VIO_CTRL2);
        regv8 |= (0x1<<2);  //bit2
        WRITE_REG8(regv8, info->regbase + R_SMC_5VIO_CTRL2);
    }

    return 0;
}


static void sci_set_attr(struct sci_info *info, SCI_ATTR_S *attr)
{
    sci_set_clock_freq(info, attr);
    sci_set_slot_type(info, attr);
    sci_set_N(info,attr);
    sci_set_stop_width(info, attr);
    sci_set_error_handle_enable(info,attr);
    sci_set_parity_enable(info,attr);
    sci_set_etu(info, attr);
    sci_set_type(info, attr);
    sci_set_clkpin_mode(info, attr);
    sci_set_rstpin_mode(info, attr);
    sci_set_iopin_mode(info, attr);
    sci_set_rxd_resends(info, attr);
    sci_set_txd_resends(info,attr);
    if(info->chipid) {
        sci_set_txfinish_enable(info,attr);
        sci_set_rxtimeout_enable(info,attr);
        sci_set_blktimeout_enable(info,attr);
        sci_set_atrtimeout_enable(info,attr);
        info->attr.write_timeout = attr->write_timeout;
    }
    info->attr.read_timeout = attr->read_timeout;
    info->attr.vcc_en_level = attr->vcc_en_level;
    info->set_attr_cnt = 1;
}

static u_char sci_get_card_status(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_STA);
    reg_sta *tmp = (reg_sta *)&reg;
    return tmp->card_sta;
}

static u_char sci_get_reg_status(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_STA);
    return reg;
}

static u_char sci_get_smc_status(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_INT_STA0);
    return reg;
}

static void sci_enable_clk(struct sci_info *info, u_char enable)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_CPCTRL);
    reg_cpctrl *tmp = (reg_cpctrl *)&reg;
    tmp->clko_en = enable;
    WRITE_REG8(reg, info->regbase + R_SMC_CPCTRL);
}

static void sci_set_rstpin_level(struct sci_info *info, u_char level)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_CPCTRL);
    reg_cpctrl *tmp = (reg_cpctrl *)&reg;
    tmp->card_rst = (level == 0 ? 1 : 0);
    WRITE_REG8(reg, info->regbase + R_SMC_CPCTRL);
}

static void sci_set_iopin_level(struct sci_info *info, u_char level)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_CPCTRL);
    reg_cpctrl *tmp = (reg_cpctrl *)&reg;
    tmp->data_en = level;
    WRITE_REG8(reg, info->regbase + R_SMC_CPCTRL);
}

static void sci_enable_vcc(struct sci_info *info, u_char enable)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_CPCTRL);
    reg_cpctrl *tmp = (reg_cpctrl *)&reg;
    tmp->pow_ctrl = enable == 1 ? info->attr.vcc_en_level : !info->attr.vcc_en_level;
    WRITE_REG8(reg, info->regbase + R_SMC_CPCTRL);
}

static void sci_hwfifo_reset(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_CPCTRL);
    reg_cpctrl *tmp = (reg_cpctrl *)&reg;
    tmp->fifo_rst = 1;
    WRITE_REG8(reg, info->regbase + R_SMC_CPCTRL);
    udelay(100);
    tmp->fifo_rst = 0;
    WRITE_REG8(reg, info->regbase + R_SMC_CPCTRL);
    udelay(100);
}

static void sci_set_5v_vcc(struct sci_info *info, u_char vl)
{
    WRITE_REG8(vl, info->regbase + R_SMC_5VIO_CTRL0);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
/*
static void sci_regs_print(struct sci_info *info)
{
	u32 ii=0,addr=0;
	u8 tmp_reg=0;
	for(ii=1; ii<50; ii++){
		addr=(ii<<2);
		if(((R_SMC_STA<=addr) && (R_SMC_CPCTRL>=addr)) ||
			 ((R_SMC_PIN_CFG<=addr) && (R_SMC_RCV_TIMEOUT_CNT3>=addr)) ||
			 ((R_SMC_T_CNT_20<=addr) && (R_SMC_CFG_3>=addr)) ||
			 ((R_SMC_5VIO_CTRL0<=addr) && (R_SMC_OVERLOAD_CNT3>=addr))){
			tmp_reg=READ_REG8(info->regbase + addr);
			printk(KERN_EMERG "[%x]=%x\n",addr,tmp_reg);
		}
	}
}*/

static void sci_regs_rw(struct sci_info *info,u8 *regs,int rw)
{
	u32 ii,addr;
	if(0==rw){
		for(ii=1; ii<50; ii++){
			addr=(ii<<2);
			if(((R_SMC_STA<=addr) && (R_SMC_CPCTRL>=addr)) ||
				 ((R_SMC_PIN_CFG<=addr) && (R_SMC_RCV_TIMEOUT_CNT3>=addr)) ||
				 ((R_SMC_T_CNT_20<=addr) && (R_SMC_CFG_3>=addr)) ||
				 ((R_SMC_5VIO_CTRL0<=addr) && (R_SMC_OVERLOAD_CNT3>=addr))){
				regs[ii]=READ_REG8(info->regbase + addr);
				//printk(KERN_EMERG "++r[%x]=%x\n",addr,regs[ii]);
			}
		}
	}else{
		for(ii=1; ii<50; ii++){
			addr=(ii<<2);
			if(((R_SMC_STA<=addr) && (R_SMC_CPCTRL>=addr)) ||
				 ((R_SMC_PIN_CFG<=addr) && (R_SMC_RCV_TIMEOUT_CNT3>=addr)) ||
				 ((R_SMC_T_CNT_20<=addr) && (R_SMC_CFG_3>=addr)) ||
				 ((R_SMC_5VIO_CTRL0<=addr) && (R_SMC_OVERLOAD_CNT3>=addr))){
				WRITE_REG8(regs[ii],info->regbase + addr);
				//printk(KERN_EMERG "++w[%x]=%x\n",addr,regs[ii]);
			}
		}
	}
}

static void sci_clear_interrupt_status(struct sci_info *info,u_char flag)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_INT_CLR0);
    reg |= flag;
    WRITE_REG8(reg, info->regbase + R_SMC_INT_CLR0);
}
static void sci_clear_interrupt_status_overload(struct sci_info *info,u_char flag)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_INT_CLR1);
    reg |= flag;
    reg &= 0x01;
    WRITE_REG8(reg, info->regbase + R_SMC_INT_CLR1);
}

static void sci_deactivate_poweroff(struct sci_info *info)
{
    int flag_sequnce=1;
    chipverify_log("%s %d\n",__FUNCTION__,__LINE__);
    if (READ_REG8(info->regbase + R_SMC_CFG_3) & 0x01) {
        if(0) {
            u8 reg04=READ_REG8(info->regbase + R_SMC_STA);
            u8 reg10=READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
            u8 reg20=READ_REG8(info->regbase + R_SMC_CPCTRL);
            u8 reg70=READ_REG8(info->regbase + R_SMC_INT_STA0);
            u8 reg7c=READ_REG8(info->regbase + R_SMC_T_CNT_20);
            u8 reg80=READ_REG8(info->regbase + R_SMC_T_CNT_21);
            u8 reg84=READ_REG8(info->regbase + R_SMC_T_CNT_22);
            u8 reg88=READ_REG8(info->regbase + R_SMC_T_CNT_23);
            u8 reg8c=READ_REG8(info->regbase + R_SMC_CFG_0);
            u8 reg90=READ_REG8(info->regbase + R_SMC_CFG_1);
            u8 reg94=READ_REG8(info->regbase + R_SMC_CFG_2);
            u8 reg98=READ_REG8(info->regbase + R_SMC_CFG_3);
            //u8 reg9c=READ_REG8(info->regbase + R_SMC_ACTVT_FINISH);

            chipverify_log("reg04 10 20 70=%x,%x,%x,%x\n",reg04,reg10,reg20,reg70);
            chipverify_log("reg7c..9c=%x,%x,%x,%x,%x,%x,%x,%x\n",reg7c,reg80,reg84,reg88,reg8c,reg90,reg94,reg98);
        }
        chipverify_log("%s %d waiting hardware poweroff\n",__FUNCTION__,__LINE__);
        while (READ_REG8(info->regbase + R_SMC_ACTVT_FINISH) != 1) {
            udelay(10);
        }
        flag_sequnce=0;
    }

    sci_deactivate(info,flag_sequnce);

    if (READ_REG8(info->regbase + R_SMC_CFG_3) & 0x01) {
        u8 tmp = 0;
        tmp = READ_REG8(info->regbase + R_SMC_CFG_3);
        tmp |= (0x02);
        WRITE_REG8(tmp,(info->regbase + R_SMC_CFG_3));

        tmp = READ_REG8(info->regbase + R_SMC_CFG_3);
        tmp &= ~(0x02);
        WRITE_REG8(tmp,(info->regbase + R_SMC_CFG_3));
        //tmp = READ_REG8(info->regbase + R_SMC_ACTVT_FINISH);
        //udelay(5);
        //tmp = READ_REG8(info->regbase + R_SMC_ACTVT_FINISH);

        chipverify_log("%s %d hardware poweroff signal owner ok\n",__FUNCTION__,__LINE__);
    }
    chipverify_log("%s %d\n",__FUNCTION__,__LINE__);
}

#if defined(SMC_DETECT_KADC2)
static void smc_kadc_insert_remove(u32 flag_insert, u32 dect)
{
    volatile u_char reg = READ_REG8(sciinfo[0].regbase + R_SMC_CPCTRL);
    if (0 == dect) {
        if(SMC_INSERT == flag_insert){
            reg |= (1<<3);
        }else{
            reg &= ~(1<<3);
        }
    } else {
        if(SMC_INSERT == flag_insert){
            reg &= ~(1<<3);
        }else{
            reg |= (1<<3);
        }
    }
    WRITE_REG8(reg,(sciinfo[0].regbase + R_SMC_CPCTRL));
    if(SMC_REMOVE == flag_insert) {
        sci_deactivate_poweroff(&(sciinfo[0]));
    }
    //printk(KERN_EMERG "insert_remove w20:%x\n",reg);
}
static irqreturn_t smc_kadc_int_handler(int irq, void *dev_id)
{
    u32 int_sta = 0, kval = 0, dect = 0;
    int_sta = READ_REG32((void*)R_KEY_ADC2_INT);
    kval = READ_REG32((void*)R_KEY_ADC2_DATA);
    dect = READ_REG32((void*)(sciinfo[0].regbase+R_SMC_FRQ_CFG));
    dect &= 1;
    printk(KERN_EMERG "in kadc2 %x,%x,%x\n",int_sta,kval,dect);

    if (0 == dect) {
        if(int_sta&0x20){   //<range .remove
            int_sta &= ~(7<<4);
            int_sta |=0x10;
            WRITE_REG32(int_sta,(void*)R_KEY_ADC2_INT);
            if(kval < KADC_RANGE){    //sometime , int=22, kval>rang.
                if (sci_get_card_status(&(sciinfo[0])) == SMC_INSERT) {
                    smc_kadc_insert_remove(SMC_REMOVE, dect);
                    printk(KERN_EMERG "do out\n");
                }
            }
        }else{
            int_sta &= ~(7<<4);
            int_sta |=0x20;
            WRITE_REG32(int_sta,(void*)R_KEY_ADC2_INT);
            if (sci_get_card_status(&(sciinfo[0])) != SMC_INSERT) {
                smc_kadc_insert_remove(SMC_INSERT, dect);
                printk(KERN_EMERG "do in\n");
            }
        }
    } else {
        if(int_sta&0x20){//>range .remove
            int_sta &= ~(7<<4);
            int_sta |=0x10;
            WRITE_REG32(int_sta,(void*)R_KEY_ADC2_INT);
            if (sci_get_card_status(&(sciinfo[0])) != SMC_INSERT) {
                smc_kadc_insert_remove(SMC_INSERT, dect);
                printk(KERN_EMERG "do in\n");
            }
        }else{
            int_sta &= ~(7<<4);
            int_sta |=0x20;
            WRITE_REG32(int_sta,(void*)R_KEY_ADC2_INT);
            if (sci_get_card_status(&(sciinfo[0])) == SMC_INSERT) 
            {
                smc_kadc_insert_remove(SMC_REMOVE, dect);
                printk(KERN_EMERG "do out\n");
            }
        }
    }
    return IRQ_HANDLED;
}

static int kadc_interrupt_init(void)
{
    int ret = 0;
    u32 val = 0;
    u32 data = 0,dect = 0;
    u32 int_enable = 0;

    val = READ_REG32((void*)R_KEY_ADC2_INT);
    val &= ~(7<<4);
    WRITE_REG32(val,(void*)R_KEY_ADC2_INT);

    val = READ_REG32((void*)R_KEY_ADC2_RANGE);	        //bit 0~5
    val &= ~0x3f;
    val |= KADC_RANGE;
    val |= (1<<6);                                      //d1>d0 && (d1-d0)>KADC_RANGE
    WRITE_REG32(val,(void*)R_KEY_ADC2_RANGE);

    WRITE_REG32(0x1ff0001,(void*)R_KEY_ADC2_SAMPL);     //every 511 compare once

    chipverify_log("%s %d\n",__FUNCTION__,__LINE__);
    if((sciinfo[0].init & 0x02) != 0x02) {
        //printk(KERN_EMERG "reset kadc1 isr\n");
        ret = request_irq(IRQ_KADC1_ID, smc_kadc_int_handler,IRQF_TRIGGER_HIGH,"kadc2", NULL);
        if(ret != 0) {
            MT_ERR_SCI("cannot request kadc2 interrupt (err=%d)\n", ret);
            return -1;
        }
        sciinfo[0].init |= 0x02;
    }

    val = READ_REG32((void*)R_KEY_ADC2_CFG);
    val &= ~(1<<1); //clear pwr
    val |= 1<<4;
    val &= ~(1<<8);
    WRITE_REG32(val,(void*)R_KEY_ADC2_CFG);
    mdelay(1);
    data = READ_REG32((void*)R_KEY_ADC2_DATA);
    printk(KERN_EMERG "firstkey val %x\n",data);
    dect = READ_REG32((void*)(sciinfo[0].regbase+R_SMC_FRQ_CFG));
    dect &= 1;
    if (0 == dect) {
        if (data >= KADC_RANGE) {        //expect > value
            smc_kadc_insert_remove(SMC_INSERT, dect);
            //printk(KERN_EMERG "do in.init. %x\n",data);
            int_enable = 0x20;
        } else {                        //expect diff>range
            smc_kadc_insert_remove(SMC_REMOVE, dect);
            //printk(KERN_EMERG "do in.init. %x\n",data);
            int_enable = 0x10;
        }
    } else {
        if (data > KADC_RANGE) {
            smc_kadc_insert_remove(SMC_REMOVE, dect);
            int_enable = 0x20;
        } else {
            smc_kadc_insert_remove(SMC_INSERT, dect);
            int_enable = 0x10;
        }
    }
    val = READ_REG32((void*)R_KEY_ADC2_INT);
    val &= ~(7<<4);
    val |= int_enable;
    WRITE_REG32(val,(void*)R_KEY_ADC2_INT);

    chipverify_log("%s %d\n",__FUNCTION__,__LINE__);
    return 0;
}
#endif
#endif

#if 0
static void sci_delay_clocks(struct sci_info *info, u_long ticks)
{
    u_long ticks_us = info->attr.Hz /1000000;
    udelay((ticks + ticks_us - 1) / ticks_us);
}
#endif

static void sci_reset_rxfifo_and_hwfifo(struct sci_info *info)
{
    sci_hwfifo_reset(info);
    memset(&info->rx_fifo, 0, sizeof(struct smc_rx_fifo));
    info->readable = 0;
    __smp_mb();
}

static void sci_activate(struct sci_info *info)
{
    sci_reset_rxfifo_and_hwfifo(info);
    sci_enable_vcc(info, 1);
    mdelay(1);          //10
    sci_set_iopin_level(info, 1);
    udelay(120);
    sci_enable_clk(info, 1);
    sci_set_rstpin_level(info, 0);
    info->status = SCI_STATUS_ACTIVATE;
}

static void sci_reset(struct sci_info *info)
{
    info->paritystatus = 0;
    sci_reset_rxfifo_and_hwfifo(info);
    sci_set_rstpin_level(info, 0);
    udelay(2000);
    sci_set_rstpin_level(info, 1);
    info->status = SCI_STATUS_RESET;
}

static void sci_deactivate(struct sci_info *info,int flag_sequnce)
{
    unsigned long oneclock_time = 0;
    sci_set_rstpin_level(info, 0);
    //some card need 185~260/2==222 clock bwteen (res , io).info->attr.Hz
    //1000000000/3570000=280ns. 1 clock=280ns
    if (flag_sequnce) {
        oneclock_time = (1000000/(info->attr.Hz/1000)); //a clock = oneclock_time ns
        oneclock_time *= 222;                           //222 clock = x ns
        oneclock_time = oneclock_time/1000;             //x ns = x/1000 us
        udelay(oneclock_time/2);
    }

    sci_enable_clk(info, 0);

    if (flag_sequnce) {
        udelay(oneclock_time/2);
    }
    sci_set_iopin_level(info, 0);
    sci_enable_vcc(info, 0);
    sci_reset_rxfifo_and_hwfifo(info);

    //printk(KERN_EMERG "deactive %ld,%d\n", oneclock_time, info->attr.Hz);
    info->status = SCI_STATUS_DEACTIVATE;
}

static u_long sci_get_rxfifo(struct sci_info *info, u_char *buf, u_long len)
{
    u_long i = 0;

    while(info->rx_fifo.count && len) {
        buf[i ++] = info->rx_fifo.buff[info->rx_fifo.rp ++];
        info->rx_fifo.rp %= RX_FIFO_SIZE;
        info->rx_fifo.count --;
        len --;
    }
    return i;
}

static int sci_hwfifo_is_empty(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_STA);
    reg_sta *tmp = (reg_sta *)&reg;
    return tmp->rx_sta ? 0 : 1;
}

static mt_u32 sci_get_hwfifo_count(struct sci_info *info)
{
    volatile u_char countH = READ_REG8(info->regbase + R_SMC_BUF_CNT_H);
    reg_buf_cnt_h *tmp = (reg_buf_cnt_h *)&countH;
    u_char countL = READ_REG8(info->regbase + R_SMC_BUF_CNT_L);
    return (tmp->buf_cnt << 8) | countL;
}
#ifdef SYM6_SMC_DMA
static u_long sci_hwfifo_to_rxfifo_dma(struct sci_info *info)
{
    mt_u32 count = 0,loops=0;
    mt_u32 j = 0;
    mt_u8 *vir_dst_addr = (mt_u8*)info->dmammzbuf.startVirAddr;
    hal_dma_param_t dma_param= {0};
    int chn_id = 0;

    if (NULL==vir_dst_addr) {
        MT_FATAL_SCI(KERN_EMERG "smc dma mmz buf==NULL\n");
        return 0;
    }

    if(sci_hwfifo_is_empty(info)) {
        return 0;
    }

    count = sci_get_hwfifo_count(info);
    if(RX_FIFO_SIZE <= (count+info->rx_fifo.count)) {
        count=RX_FIFO_SIZE-info->rx_fifo.count;
    }

    dma_param.mode = DMA_MODE_2D;
    dma_param.alu_fill_data = 0;
    dma_param.alu_fill_width = 0;
    dma_param.len = count;
    dma_param.dst_width = 0;
    dma_param.phy_dst_addr=info->dmammzbuf.startPhyAddr;
    dma_param.vir_dst_addr=info->dmammzbuf.startVirAddr;
    dma_param.dst_leap = 0;
    dma_param.src_width = 0;
    dma_param.phy_src_addr = 0xBF590000;
    dma_param.vir_src_addr = (void *)SYMPHONY_IO_VA(0xBF590000);
    dma_param.src_leap = 0;
    dma_param.config.dst_peripheral = 0xf; // memory
    dma_param.config.src_peripheral = 0xc; // SMC0
    dma_param.config.dst_endian = 0; // big endian
    dma_param.config.src_endian = 0; // big endian
    dma_param.config.dst_clk = 0; // AXI clock
    dma_param.config.src_clk = 1; // AHB clock
    dma_param.config.dst_i = DMA_ADDR_INC;
    dma_param.config.src_i = DMA_ADDR_FIX;
    dma_param.config.dst_usize = DMA_USIZE_64BIT;
    dma_param.config.src_usize = DMA_USIZE_8BIT;
    dma_param.config.dst_bsize = DMA_BURST_NUM16;
    dma_param.config.src_bsize = DMA_BURST_NUM1;
    dma_param.control.int_link_en = 1;
    dma_param.control.int_node_en = 1;
    dma_param.control.chn_param_reg_en = 0;
    dma_param.p_next = NULL;
    chn_id = 0;//hal_dma_get_free_channel(0);
    if(0) {
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf121000),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf121004),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf121008),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf12100c),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf121010),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf121014),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf121018),0xffffffff);
        HAL_PUT_U32((volatile u32 *)SYMPHONY_IO_VA(0xbf12101c),0xffffffff);
    }
    printk(KERN_EMERG "r.smc dma start 1 %x,%x\n",count,chn_id);
    hal_dma_start(chn_id, &dma_param, NULL);
    printk(KERN_EMERG "r.smc dma start 2\n");
    loops=0;
    while (DMA_STATUS_STOP != hal_dma_check(chn_id)) {
        loops++;
        if(loops>1000000) {
            MT_FATAL_SCI(KERN_EMERG "smc dma recv timeout\n");
            break;
        }
    }
    hal_dma_stop(chn_id);
    mt_dcache_invalid(vir_dst_addr,ALIGN(count,CACHE_LINE_SIZE));

    while(count) {
        printk(KERN_EMERG "r=%x \n",vir_dst_addr[j]);
        info->rx_fifo.buff[info->rx_fifo.wp ++] = vir_dst_addr[j];
        info->rx_fifo.wp %= RX_FIFO_SIZE;
        info->rx_fifo.count ++;
        count --;
        j++;
    }
    return j;
}
static mt_s32 sci_rxfifo_to_hwfifo_dma(struct sci_info *info, u_char *buf, mt_u32 len)
{
    mt_s32 ret=0;
    mt_u32 count = 0,loops=0;
    mt_u8 *vir_src_addr = (mt_u8*)info->dmammzbuf.startVirAddr;
    hal_dma_param_t dma_param= {0};
    int chn_id = 0;

    count = len;

    if (NULL==vir_src_addr) {
        MT_FATAL_SCI(KERN_EMERG "smc dma mmz buf==NULL\n");
        return -1;
    }
    {
        if(1==len) {
            chipverify_log("++s:%x\n",buf[0]);
        } else if(2==len) {
            chipverify_log("++s:%x,%x\n",buf[0],buf[1]);
        } else if(3==len) {
            chipverify_log("++s:%x,%x,%x\n",buf[0],buf[1],buf[2]);
        } else if(4==len) {
            chipverify_log("++s:%x,%x,%x,%x\n",buf[0],buf[1],buf[2],buf[3]);
        } else if(5==len) {
            chipverify_log("++s:%x,%x,%x,%x,%x\n",buf[0],buf[1],buf[2],buf[3],buf[4]);
        } else if(6==len) {
            chipverify_log("++s:%x,%x,%x,%x,%x,%x\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
        }
    }
    memcpy(vir_src_addr,buf,len);
    mt_dcache_flush(vir_src_addr,ALIGN(len, CACHE_LINE_SIZE));

    dma_param.mode = DMA_MODE_2D;
    dma_param.alu_fill_data = 0;
    dma_param.alu_fill_width = 0;
    dma_param.len = count;
    dma_param.dst_width = 0;
    dma_param.phy_dst_addr=0xBF590000;
    dma_param.vir_dst_addr=(void *)SYMPHONY_IO_VA(0xBF590000);
    dma_param.dst_leap = 0;
    dma_param.src_width = 0;
    dma_param.phy_src_addr = info->dmammzbuf.startPhyAddr;
    dma_param.vir_src_addr = info->dmammzbuf.startVirAddr;
    dma_param.src_leap = 0;
    dma_param.config.dst_peripheral = 0xc; // SMC0
    dma_param.config.src_peripheral = 0xf; // memory
    dma_param.config.dst_endian = 0; // big endian
    dma_param.config.src_endian = 0; // big endian
    dma_param.config.dst_clk = 1; // AHB clock
    dma_param.config.src_clk = 0; // AXI clock
    dma_param.config.dst_i = DMA_ADDR_FIX;
    dma_param.config.src_i = DMA_ADDR_INC;
    dma_param.config.dst_usize = DMA_USIZE_8BIT;
    dma_param.config.src_usize = DMA_USIZE_64BIT;
    dma_param.config.dst_bsize = DMA_BURST_NUM1;
    dma_param.config.src_bsize = DMA_BURST_NUM16;
    dma_param.control.int_link_en = 1;
    dma_param.control.int_node_en = 1;
    dma_param.control.chn_param_reg_en = 0;
    dma_param.p_next = NULL;
    chn_id = 0;//hal_dma_get_free_channel(0);

    printk(KERN_EMERG "s.smc dma start 1 %x,%x\n",count,chn_id);
    hal_dma_start(chn_id, &dma_param, NULL);
    printk(KERN_EMERG "s.smc dma start 2\n");
    loops=0;
    while (DMA_STATUS_STOP != hal_dma_check(chn_id)) {
        loops++;
        if(loops>1000000) {
            MT_FATAL_SCI(KERN_EMERG "smc dma send timeout\n");
            ret=-1;
            break;
        }
    }
    hal_dma_stop(chn_id);

    return ret;
}
#endif
static u_long sci_hwfifo_to_rxfifo(struct sci_info *info)
{
    mt_u32 count = 0;
    if(sci_hwfifo_is_empty(info)) {
        return 0;
    }
    count = sci_get_hwfifo_count(info);

    //chipverify_log("++r:%x\n",count);

    MT_INFO_SCI(KERN_INFO "info->regbase: 0x%x, sci hwfifo count: %d\n", info->regbase, count);

#ifndef SYM6_SMC_DMA
    {
        mt_u32 i = 0;
        while(count && info->rx_fifo.count < RX_FIFO_SIZE) {
            info->rx_fifo.buff[info->rx_fifo.wp ++] = READ_REG8(info->regbase + R_SMC_DBUF);

            MT_INFO_SCI(KERN_INFO "hwfifo item: 0x%02x\n",	info->rx_fifo.buff[info->rx_fifo.wp - 1]);

            info->rx_fifo.wp %= RX_FIFO_SIZE;
            info->rx_fifo.count ++;
            count --;
            i ++;
        }
        return i;
    }
#else
    return count;
#endif
}

static void sci_put_hwfifo(struct sci_info *info, u_char *buf, u_long len)
{
    sci_reset_rxfifo_and_hwfifo(info);
    WRITE_REG8((len >> 8) & 0xff, info->regbase + R_SMC_CMDLEN_H);
    WRITE_REG8(len & 0xff, info->regbase + R_SMC_CMDLEN_L);

#ifndef SYM6_SMC_DMA
    {
        int i = 0;
        unsigned long flags;
        spin_lock_irqsave(&info->sndlock,flags);
        while(len --) {
            while(sci_get_hwfifo_count(info) >= 256);
            WRITE_REG8(buf[i ++], info->regbase + R_SMC_DBUF);
        }
        spin_unlock_irqrestore(&info->sndlock,flags);
    }
#else
    {
        mt_u32 send_cell=0,send_all=0;
        mt_u32 cached_num=0;

        while(len) {
            cached_num=sci_get_hwfifo_count(info);
            if(cached_num<RX_FIFO_SIZE) {
                send_cell=(RX_FIFO_SIZE-cached_num);
                if(len<send_cell) {
                    send_cell=len;
                }
                sci_rxfifo_to_hwfifo_dma(info,buf+send_all,send_cell);
                send_all+=send_cell;
                len-=send_cell;
            }
        }
    }
#endif
}

static int sci_read(struct sci_info *info, SCI_DATA_S *data)
{
    u_long rdcnt = 0;
    unsigned long flags;

    ktime_t timeout = ns_to_ktime(0ULL);
    timeout = ktime_add_us(timeout, info->attr.read_timeout);
    if(!sci_get_card_status(info)) {
        return -ENXIO;
    }

    info->intstatus = 0;

    if(-ETIME == wait_event_hrtimeout(info->wq, info->readable, timeout)) {
        #if 0
        printk(KERN_EMERG "%s %d timeout! timeout = %lld\n",__FUNCTION__,__LINE__,timeout);
        if(!sci_hwfifo_is_empty(info)) {
            count = sci_get_hwfifo_count(info);
            MT_ERR_SCI("read timeout,but there is %d bytes in buffer\n",count);
        }
        return -EIO;
        #else
		spin_lock_irqsave(&info->wrlock, flags);
		sci_hwfifo_to_rxfifo(info);
		rdcnt = sci_get_rxfifo(info, g_smc_buff, data->data_len);
		spin_unlock_irqrestore(&info->wrlock, flags);
        //MT_ERR_SCI("read timeout,but there is %d bytes in buffer\n",rdcnt);
		if(rdcnt > 0)
		{
			if (copy_to_user((void __user *)data->data_buf, g_smc_buff, rdcnt) != 0)
			{
				return -EIO;
			}
			return rdcnt;
		}
		return -EIO;
        #endif
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    if((info->attr.recetime_en) && (0x80 == (info->intstatus & 0x80))) {
        MT_ERR_SCI("%s %d rece timeout status = 0x%x\n",__FUNCTION__,__LINE__,info->intstatus);
        return 0;
    }

    if((info->attr.blktime_en) && (0x20 == (info->intstatus & 0x20))) {
        MT_ERR_SCI("%s %d blk timeout status = 0x%x\n",__FUNCTION__,__LINE__,info->intstatus);
        return 0;
    }

    if(info->attr.rsttime_en) {
        if(1 == info->paritystatus) {
            MT_ERR_SCI("%s %d parity is error!\n",__FUNCTION__,__LINE__);
            info->paritystatus = 2;
            return 0;
        }
        if(0x40 == (info->intstatus & 0x40)) {
            MT_ERR_SCI("%s %d rst timeout status = 0x%x\n",__FUNCTION__,__LINE__,info->intstatus);
            return 0;
        }
    }
#else
    if((info->attr.recetime_en) && (0x08 == (info->intstatus & 0x08))) {
        MT_ERR_SCI("%s %d rece timeout status = 0x%x\n",__FUNCTION__,__LINE__,info->intstatus);
        return 0;
    }

    if((info->attr.blktime_en) && (0x04 == (info->intstatus & 0x04))) {
        MT_ERR_SCI("%s %d blk timeout status = 0x%x\n",__FUNCTION__,__LINE__,info->intstatus);
        return 0;
    }

    if(info->attr.rsttime_en) {
        if(1 == info->paritystatus) {
            MT_ERR_SCI("%s %d parity is error!\n",__FUNCTION__,__LINE__);
            info->paritystatus = 2;
            return 0;
        }
        if(0x02 == (info->intstatus & 0x02)) {
            MT_ERR_SCI("%s %d rst timeout status = 0x%x\n",__FUNCTION__,__LINE__,info->intstatus);
            return 0;
        }
    }
#endif

#ifdef SYM6_SMC_DMA
    sci_hwfifo_to_rxfifo_dma(info);
#endif

    spin_lock_irqsave(&info->wrlock,flags);
    rdcnt = sci_get_rxfifo(info, g_smc_buff, data->data_len);
    if(info->rx_fifo.count == 0) {        //cache==0
#ifdef SYM6_SMC_DMA
        if(0==sci_get_hwfifo_count(info))
#endif
        {
            info->readable = 0;
            __smp_mb();
        }
    }
    spin_unlock_irqrestore(&info->wrlock, flags);

    if(rdcnt > 0) {
        /*int t=0;
        for(t=0; t<rdcnt; t++) {
            chipverify_log("r.[%x]=%x\n",t,g_smc_buff[t]);
        }*/

        if (copy_to_user((void __user *)data->data_buf, g_smc_buff, rdcnt) != 0) {
            return -EIO;
        }
    }

    return rdcnt;
}

static int sci_write(struct sci_info *info, SCI_DATA_S *data)
{
    ktime_t timeout = ns_to_ktime(0ULL);

    if(!sci_get_card_status(info)) {
        return -ENXIO;
    }

    if (copy_from_user(g_smc_buff, (const void __user *)data->data_buf, data->data_len) != 0) {
        return -EIO;
    }

    timeout = ns_to_ktime(0ULL);
    if(info->attr.tx_finish_en && (info->attr.write_timeout > 0)) {
        timeout = ktime_add_us(timeout, info->attr.write_timeout);
        info->writeend=0;
        __smp_mb();
    }
    sci_put_hwfifo(info, g_smc_buff, data->data_len);
    if(info->attr.tx_finish_en && (info->attr.write_timeout > 0)) {
        if(-ETIME == wait_event_hrtimeout(info->wq, info->writeend, timeout)) {
            if (0 == info->writeend) {  //check again(whether still zero).
                MT_ERR_SCI("%s %d timeout!\n",__FUNCTION__,__LINE__);
                return -EIO;
            }
        }
    }
    return data->data_len;
}

/* Init smc0 */
static int  sci_init_0(void)
{
    int ret = 0;
    unsigned long ir_flag = IRQF_TRIGGER_RISING;
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    unsigned long clk = 0;
#endif
    u32 regvalue = 0;

#if defined(CONFIG_MT_CHIP_ARIA)
    u32 sys_ctrl_base_addr = mt_get_sys_ctrl_base();
    u32 pads_base_reg = aria_get_pads0_ctrl_base();
#endif
    gatingoff = 0;

    sciinfo[0].id = 0;

    sciinfo[0].readable = 0;

    sciinfo[0].status = SCI_STATUS_DEACTIVATE;

    sciinfo[0].set_attr_cnt = 0;

    sciinfo[0].chipid = 0;

#if defined(CONFIG_MT_CHIP_ARIA)
    /* Last reg R_SMC_RESEND_CFG offset 0x38, so 0x40 size is enough  */
    sciinfo[0].regbase = ioremap(R_SMC0_BASE_ADDR, 0x40);//(void *)R_SMC0_BASE_ADDR;

    //sysctrl_regbase = ioremap(SYSCTRL_REG_BASE, 64 * 1024);
    /* bit 1 set 1 enable smc0 bit15 set 1 enable smc1 */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x002c) |0x1, sys_ctrl_base_addr+0x002c);
    /* bit 4~5 0 */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0128) & ~(0x30), sys_ctrl_base_addr+0x0128);
#if DEBUG
    MT_INFO_SCI("sys_ctrl reg 0x%x, value 0x%x\n",sys_ctrl_base_addr+0x002c, READ_REG32(sys_ctrl_base_addr + 0x002c));
    MT_INFO_SCI("sys_ctrl reg 0x%x, value 0x%x\n",sys_ctrl_base_addr+0x0128, READ_REG32(sys_ctrl_base_addr + 0x0128));
#endif
    /* Set pads for smc0 */
    /* bit 0 clk_ds1, bit 2 clk_ode, bit 4 dat_ds1, bit 6 dat_ode */
    WRITE_REG8(0x77, pads_base_reg);
    /* bit 0 det_ds1, bit 3 det_smt,  bit 4 rst_ds1, bit 6 rst_ode */
    WRITE_REG8(0x79, pads_base_reg + 0x4);
    /* bit 0 vcc_ds1*/
    WRITE_REG8(0x1, pads_base_reg + 0x8);
#if DEBUG
    MT_INFO_SCI("pads reg 0x%x, value 0x%x\n",pads_base_reg, READ_REG8(pads_base_reg));
    MT_INFO_SCI("pads reg 0x%x, value 0x%x\n",pads_base_reg + 0x4, READ_REG8(pads_base_reg + 0x4));
    MT_INFO_SCI("pads reg 0x%x, value 0x%x\n",pads_base_reg + 0x8, READ_REG8(pads_base_reg + 0x8));
#endif
    sciinfo[0].clkbase = APB_CLOCK_FREQ;
#endif


#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    regvalue = READ_REG32((void*)R_PINMUX_ADDR1);
#if DEBUG
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_ADDR1,regvalue);
#endif
    regvalue &= ~(0xf << 28);
    regvalue |= (2 << 28);
    WRITE_REG32(regvalue,(void*)R_PINMUX_ADDR1);
    regvalue = READ_REG32((void*)R_PINMUX_ADDR2);
#if DEBUG
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_ADDR2,regvalue);
#endif
    regvalue &= ~(0xf << 0);
    regvalue |= (2 << 0);
    regvalue &= ~(0xf << 4);
    regvalue |= (2 << 4);
    WRITE_REG32(regvalue,(void*)R_PINMUX_ADDR2);
    regvalue = READ_REG32((void*)R_PINMUX_SWPIN1_ADDR);
#if DEBUG
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_SWPIN1_ADDR,regvalue);
#endif
    regvalue &= ~(0xf << 12);
    regvalue |= (1 << 12);
    regvalue &= ~(0xf << 16);
    regvalue |= (1 << 16);
    WRITE_REG32(regvalue,(void*)R_PINMUX_SWPIN1_ADDR);
    regvalue = READ_REG32((void*)R_PINMUX_SWPIN2_ADDR);
#if DEBUG
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_SWPIN2_ADDR,regvalue);
#endif
    regvalue &= ~(0xf << 4);
    regvalue |= (2 << 4);
    regvalue &= ~(0xf << 8);
    regvalue |= (2 << 8);
    WRITE_REG32(regvalue,(void*)R_PINMUX_SWPIN2_ADDR);

    sciinfo[0].regbase = R_SMC0_BASE_ADDR;
    sci_clock_config(SCI_BASECLK_APB, SCI_BASECLK_APB);
    symphony_get_clock(HAL_PB, &clk);
#ifdef CONFIG_MT_FPGA
    clk = FPGA_SMC_BASE_CLOCK;
#endif
    sciinfo[0].clkbase = clk;
#if DEBUG
    MT_INFO_SCI(" \33[44;31m clk = %d\33[0m \n",clk);
#endif
    WRITE_REG32(0xffff,(void*)R_CHIP_ID_RDEN);
    regvalue = READ_REG32((void*)R_SYMPHONY_CHIP_ID);
    WRITE_REG32(0,(void*)R_CHIP_ID_RDEN);
    if((regvalue == 0x8080) || (regvalue == 0x9000) || (regvalue == 0x9001) || (regvalue == 0x9003)) {
        sciinfo[0].chipid = 1;
        default_sciattr.tx_finish_en = 1;
        default_sciattr.blktime_en = 1;
        default_sciattr.recetime_en = 1;
        default_sciattr.rsttime_en = 1;
        WRITE_REG8(0x08, sciinfo[0].regbase + R_SMC_PIN_CFG);
    }
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    if((READ_REG32(R_SMC_CLK_ADDR) & 0x1) == 0) {
        sciinfo[0].clkbase = 90000000;
    } else {
        sciinfo[0].clkbase = 12000000;
        MT_INFO_SCI("use xtal_clk\n");
    }
    regvalue = READ_REG32((void*)(R_SMC0_BASE_ADDR+R_SMC_MODE_CFG));
    regvalue &= (~0x4);
    WRITE_REG32(regvalue,(void*)(R_SMC0_BASE_ADDR+R_SMC_MODE_CFG));

#if defined(SMC_ENABLE_5VIO)
    regvalue = READ_REG32((void*)R_SMC_PIN1_ADDR);
    regvalue &= ~(0x3);
    WRITE_REG32(regvalue,(void*)R_SMC_PIN1_ADDR);

    regvalue = READ_REG32((void*)R_SMC_PIN2_ADDR);
    regvalue &= ~(0x3);
    regvalue &= ~(1 << 13);
    regvalue &= ~(0x3 << 9);
    regvalue |= (0x3 << 9);
    WRITE_REG32(regvalue,(void*)R_SMC_PIN2_ADDR);

    regvalue = READ_REG32((void*)R_SMC_PIN3_ADDR);
    regvalue &= ~(0x3);
    regvalue &= ~(1 << 13);
    regvalue &= ~(0x3 << 9);
    regvalue |= (1 << 10);
    WRITE_REG32(regvalue,(void*)R_SMC_PIN3_ADDR);

    regvalue = READ_REG32((void*)R_SMC_PIN4_ADDR);
    regvalue &= ~(0x3);
    regvalue &= ~(1 << 13);
    regvalue &= ~(0x3 << 9);
    regvalue |= (1 << 10);
    regvalue |= 0x5000;
    WRITE_REG32(regvalue,(void*)R_SMC_PIN4_ADDR);
#endif

    sciinfo[0].regbase = R_SMC0_BASE_ADDR;
    //pr_err("++base=%x,%x\n",sciinfo[0].regbase,R_SMC0_BASE_ADDR);
    sciinfo[0].chipid = 1;
    default_sciattr.tx_finish_en = 1;
    default_sciattr.blktime_en = 1;
    default_sciattr.recetime_en = 1;
    default_sciattr.rsttime_en = 1;

    ir_flag = IRQF_TRIGGER_HIGH;
#if defined(SMC_DETECT_KADC2)
    regvalue = READ_REG32((void*)R_SMC_PIN1_ADDR);
    regvalue &= ~0x7;
    regvalue |= 0x1;
    WRITE_REG32(regvalue,(void*)R_SMC_PIN1_ADDR);

    regvalue = READ_REG32((void*)R_SMC_DETECTPIN_CFG);
    regvalue |= 0x40;
    WRITE_REG32(regvalue,(void*)R_SMC_DETECTPIN_CFG);

    regvalue = READ_REG32((void*)R_SMC_DETECTPIN_INPUT);
    regvalue |= 0x40;
    WRITE_REG32(regvalue,(void*)R_SMC_DETECTPIN_INPUT);
    kadc_interrupt_init();
#endif
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#ifdef CONFIG_MT_FPGA
    sciinfo[0].clkbase = FPGA_SMC_BASE_CLOCK;
#else
    if (!IS_ERR_OR_NULL(sci_drv->sci_dev[0]->priv.smc0phyclk)) {
        sciinfo[0].clkbase = clk_get_rate(sci_drv->sci_dev[0]->priv.smc0phyclk);
    }
#endif
    sciinfo[0].regbase = SYMPHONY_IO_VA(sci_drv->sci_dev[0]->priv.membase);
    regvalue = READ_REG32((void*)(sciinfo[0].regbase+R_SMC_MODE_CFG));
    regvalue &= (~0x4);
    WRITE_REG32(regvalue,(void*)(sciinfo[0].regbase+R_SMC_MODE_CFG));

    sciinfo[0].chipid = 1;
    default_sciattr.tx_finish_en = 1;
    default_sciattr.blktime_en = 1;
    default_sciattr.recetime_en = 1;
    default_sciattr.rsttime_en = 1;

    ir_flag = IRQF_TRIGGER_HIGH;
#if defined(SMC_DETECT_KADC2)
	//pinmux set in dtsi
	//mt_pinctrl_set_function(INDEX_SW_PIN_CTRL008, 1);
	drv_gpio_io_enable(GPIO_8,GPIO_MASK_ENABLE);
	drv_gpio_set_dir(GPIO_8,GPIO_DIR_INPUT);
    {
        volatile u_char reg = READ_REG8(sciinfo[0].regbase + R_SMC_CPCTRL);
        reg |= (1<<4);
        WRITE_REG8(reg,(sciinfo[0].regbase + R_SMC_CPCTRL));
    }
    kadc_interrupt_init();
#endif
#if defined(SMC_ENABLE_5VIO)
    sci_set_5v_vcc(&sciinfo[0],7);  //ic say 7
    {
        u32 regvalue = READ_REG32((void*)R_KEY_5V_CLK);
        regvalue |= 0xb000; //AE require:Reg0xbf13c034=0xb100
        WRITE_REG32(regvalue,(void*)R_KEY_5V_CLK);
    }
#endif
#endif

    sci_config_code_convention(&sciinfo[0]);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    {
        //hardware poweroff
        MT_UNF_SCI_H_PWROFF_S tmp= {0};
        sci_set_hardwarepoweroff(&sciinfo[0],&tmp);
    }
#endif
    sciinfo[0].dmammzbuf.startVirAddr=NULL;
    //mt_drv_mmz_alloc_and_map("s6_sci_dma", NULL, 4096, 4096, &sciinfo[0].dmammzbuf);
    mt_drv_mmz_alloc("s6_sci_dma",NULL,4096,4096,&sciinfo[0].dmammzbuf);
    mt_drv_mmz_map_cache(&sciinfo[0].dmammzbuf);

    if(0x01 != (sciinfo[0].init & 0x01)) {
        mutex_init(&sciinfo[0].ioctl_lock);
        spin_lock_init(&sciinfo[0].wrlock);
        spin_lock_init(&sciinfo[0].sndlock);
        init_waitqueue_head(&sciinfo[0].wq);

        ret = request_irq(IRQ_SMC0_ID, sci_interrupt,ir_flag,"sci0", &sciinfo[0]);
        if(ret != 0) {
            MT_ERR_SCI(KERN_ERR
                       "cannot request sci0 interrupt (err=%d)\n", ret);
            goto sci_intr_fail;
        }

        sci_interrupt_enable(&sciinfo[0]);
        sciinfo[0].init |= 0x01;
    }
    sci_set_attr(&sciinfo[0], &default_sciattr);

    sciinfo[1].attr.dev_id = 0;

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
#if DEBUG
    regvalue = READ_REG32((void*)R_PINMUX_ADDR1);
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_ADDR1,regvalue);

    regvalue = READ_REG32((void*)R_PINMUX_ADDR2);
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_ADDR2,regvalue);

    regvalue = READ_REG32((void*)R_PINMUX_SWPIN1_ADDR);
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_SWPIN1_ADDR,regvalue);

    regvalue = READ_REG32((void*)R_PINMUX_SWPIN2_ADDR);
    MT_INFO_SCI(" \33[44;31m regaddr = %x regvalue = %x\33[0m \n",R_PINMUX_SWPIN2_ADDR,regvalue);
#endif
#endif

    return 0;

sci_intr_fail:
#if defined(CONFIG_MT_CHIP_ARIA)
    iounmap(sciinfo[0].regbase);
#endif
    return ret;
}

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
/* Init smc0 & 1 */
static int  sci_init_1(void)
{
    int ret = 0;
    unsigned long ir_flag = IRQF_TRIGGER_RISING;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    return 0;
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    unsigned long clk = 0;
#endif
#if defined(CONFIG_MT_CHIP_ARIA)
    u32 sys_ctrl_base_addr = mt_get_sys_ctrl_base();
#endif

    sciinfo[1].id = 1;

    sciinfo[1].readable = 0;

    sciinfo[1].status = SCI_STATUS_DEACTIVATE;

    sciinfo[1].set_attr_cnt = 0;

    sciinfo[1].chipid = 0;

#if defined(CONFIG_MT_CHIP_ARIA)
    /* Last reg R_SMC_RESEND_CFG offset 0x38, so 0x40 size is enough  */
    sciinfo[1].regbase = ioremap(R_SMC1_BASE_ADDR, 0x40);//(void *)R_SMC1_BASE_ADDR;

    //sysctrl_regbase = ioremap(SYSCTRL_REG_BASE, 64 * 1024);
    /* bit 1 set 1 enable smc0 bit15 set 1 enable smc1 */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x002c) |0x8000, sys_ctrl_base_addr+0x002c);
    /* bit4:0  01011 clk_obs */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  & ~(0x1f), sys_ctrl_base_addr+0x054);
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  | 0xb, sys_ctrl_base_addr+0x054);
    /* bit9:5  01111 data */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  & ~(0x1f << 5), sys_ctrl_base_addr+0x054);
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  | ( 0xf<< 5), sys_ctrl_base_addr+0x054);
    /* bit14:10  10100 data   */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  & ~(0x1f << 10), sys_ctrl_base_addr+0x054);
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  | ( 0x14<< 10), sys_ctrl_base_addr+0x054);
    /* bit19:15  01011 data */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  & ~(0x1f << 15), sys_ctrl_base_addr+0x054);
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  | ( 0xb<< 15), sys_ctrl_base_addr+0x054);
    /* bit23:20  0001 data */
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  & ~(0xf << 20), sys_ctrl_base_addr+0x054);
    WRITE_REG32(READ_REG32(sys_ctrl_base_addr+0x0054)  | ( 0x1<< 20), sys_ctrl_base_addr+0x054);
#if DEBUG
    MT_INFO_SCI("regbase reg 0x%x\n",sciinfo[1].regbase);

    MT_INFO_SCI("sys_ctrl reg 0x%x, value 0x%x\n", sys_ctrl_base_addr+0x002c, READ_REG32(sys_ctrl_base_addr + 0x002c));
    MT_INFO_SCI("sys_ctrl reg 0x%x, value 0x%x\n", sys_ctrl_base_addr+0x0054, READ_REG32(sys_ctrl_base_addr + 0x0054));
#endif
    sciinfo[1].clkbase = APB_CLOCK_FREQ;
#endif


#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    sciinfo[1].regbase = R_SMC1_BASE_ADDR;
    sci_clock_config(SCI_BASECLK_APB, SCI_BASECLK_APB);
    symphony_get_clock(HAL_PB, &clk);
    sciinfo[1].clkbase = clk;
    MT_INFO_SCI(" \33[44;31m clk = %d\33[0m \n",clk);
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    ir_flag = IRQF_TRIGGER_HIGH;
#endif

    sci_config_code_convention(&sciinfo[1]);

    if(1 != sciinfo[1].init) {
        mutex_init(&sciinfo[1].ioctl_lock);
        spin_lock_init(&sciinfo[1].wrlock);
        init_waitqueue_head(&sciinfo[1].wq);

        ret = request_irq(IRQ_SMC1_ID, sci_interrupt,ir_flag,"sci1", &sciinfo[1]);
        if(ret != 0) {
            MT_ERR_SCI(KERN_ERR
                       "cannot request sci1 interrupt (err=%d)\n", ret);
            goto sci_intr_fail;
        }

        sci_interrupt_enable(&sciinfo[1]);
        sciinfo[1].init = 1;
    }

    sci_set_attr(&sciinfo[1], &default_sciattr);

    sciinfo[1].attr.dev_id = 1;

    //ToDo need delay ?
    //mdelay(10);
    sci_dump_registers(&sciinfo[1]);

    return 0;

sci_intr_fail:
#if defined(CONFIG_MT_CHIP_ARIA)
    iounmap(sciinfo[1].regbase);
#endif
    return ret;
}
#endif

static long sci_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    void __user *argp = (void __user *)arg;
    // struct sci_info *info = (struct sci_info *)file->private_data;
    SCI_ATTR_S attr;
    SCI_STATUS_S status;
    MT_UNF_SCI_PORT_E dev_id = 0;
    SCI_DATA_S data;
    unsigned long flags = 0;
    int ret = 0;
    switch(cmd) {
    case SCI_IOC_INIT:
        if (copy_from_user(&dev_id, argp, sizeof(MT_UNF_SCI_PORT_E)) != 0) {
            return -EIO;
        }
        if(dev_id == MT_UNF_SCI_PORT0) {
            sci_init_0();
        }
#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
        else if(dev_id == MT_UNF_SCI_PORT1) {
            sci_init_1();
        }
#endif
        break;
    case SCI_IOC_SET_ATTR:
        if (copy_from_user(&attr, argp, sizeof(SCI_ATTR_S)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[attr.dev_id].ioctl_lock);
        sci_set_attr(&sciinfo[attr.dev_id], &attr);
        mutex_unlock(&sciinfo[attr.dev_id].ioctl_lock);
        break;
    case SCI_IOC_GET_ATTR:
        if (copy_from_user(&attr, argp, sizeof(SCI_ATTR_S)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[attr.dev_id].ioctl_lock);
        if (copy_to_user(argp, &sciinfo[attr.dev_id].attr, sizeof(SCI_ATTR_S)) != 0) {
            mutex_unlock(&sciinfo[attr.dev_id].ioctl_lock);
            return -EIO;
        }
        mutex_unlock(&sciinfo[attr.dev_id].ioctl_lock);
        break;
    case SCI_IOC_GET_STATUS:
        if (copy_from_user(&status, argp, sizeof(SCI_STATUS_S)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[status.dev_id].ioctl_lock);
        status.status = sci_get_card_status(&sciinfo[status.dev_id]);
        if (copy_to_user(argp, &status, sizeof(SCI_STATUS_S)) != 0) {
            mutex_unlock(&sciinfo[status.dev_id].ioctl_lock);
            return -EIO;
        }
        mutex_unlock(&sciinfo[status.dev_id].ioctl_lock);
        break;
    case SCI_IOC_ACTIVATE:
        if (copy_from_user(&dev_id, argp, sizeof(MT_UNF_SCI_PORT_E)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[dev_id].ioctl_lock);
        spin_lock_irqsave(&(sciinfo[dev_id].wrlock), flags);
        sci_activate(&sciinfo[dev_id]);
        spin_unlock_irqrestore(&(sciinfo[dev_id].wrlock), flags);
        mutex_unlock(&sciinfo[dev_id].ioctl_lock);
        break;
    case SCI_IOC_RESET:
        if (copy_from_user(&dev_id, argp, sizeof(MT_UNF_SCI_PORT_E)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[dev_id].ioctl_lock);
        spin_lock_irqsave(&(sciinfo[dev_id].wrlock), flags);
        sci_reset(&sciinfo[dev_id]);
        spin_unlock_irqrestore(&(sciinfo[dev_id].wrlock), flags);
        mutex_unlock(&sciinfo[dev_id].ioctl_lock);
        break;
    case SCI_IOC_DEACTIVATE:
        if (copy_from_user(&dev_id, argp, sizeof(MT_UNF_SCI_PORT_E)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[dev_id].ioctl_lock);
        spin_lock_irqsave(&(sciinfo[dev_id].wrlock), flags);
        sci_deactivate(&sciinfo[dev_id],1);
        spin_unlock_irqrestore(&(sciinfo[dev_id].wrlock), flags);
        mutex_unlock(&sciinfo[dev_id].ioctl_lock);
        mdelay(10);
        break;
    case SCI_IOC_SEND_DATA:
        if (copy_from_user(&data, argp, sizeof(SCI_DATA_S)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[data.dev_id].ioctl_lock);
        ret = sci_write(&sciinfo[data.dev_id], &data);
        mutex_unlock(&sciinfo[data.dev_id].ioctl_lock);
        break;
    case SCI_IOC_RECIEVE_DATA:
        if (copy_from_user(&data, argp, sizeof(SCI_DATA_S)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[data.dev_id].ioctl_lock);
        ret = sci_read(&sciinfo[data.dev_id], &data);
        mutex_unlock(&sciinfo[data.dev_id].ioctl_lock);
        break;
    case SCI_IOC_RECIEVE_COMPLETED:
        mutex_lock(&sciinfo[0].ioctl_lock);
        sci_set_recvtimeout_discarding(&sciinfo[0]);
        mutex_unlock(&sciinfo[0].ioctl_lock);
        ret=0;
        break;
    case SCI_IOC_SOFT_RESET:
        mutex_lock(&sciinfo[0].ioctl_lock);
        ret=sci_set_softreset(&sciinfo[0]);
        mutex_unlock(&sciinfo[0].ioctl_lock);
        break;
    case SCI_IOC_HARDWARE_PWROFF: {
        MT_UNF_SCI_H_PWROFF_S hpwroff= {0};
        if (copy_from_user(&hpwroff, argp, sizeof(MT_UNF_SCI_H_PWROFF_S)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[0].ioctl_lock);
        ret=sci_set_hardwarepoweroff(&sciinfo[0], &hpwroff);
        mutex_unlock(&sciinfo[0].ioctl_lock);
    }
    break;
    case SCI_IOC_OVERLOAD: {
        u32 koverload = 0;
        if (copy_from_user(&koverload, argp, sizeof(u32)) != 0) {
            return -EIO;
        }
        mutex_lock(&sciinfo[0].ioctl_lock);
        ret=sci_set_overload(&sciinfo[0], koverload);
        mutex_unlock(&sciinfo[0].ioctl_lock);
    }
    break;
    default:
        ret = -EINVAL;
        break;
    }
    return ret;
}

static void  sci_exit(void)
{
    if(sciinfo[0].init && (R_SMC0_BASE_ADDR == sciinfo[0].regbase)) {
        sci_interrupt_disable(&sciinfo[0]);
        free_irq(IRQ_SMC0_ID, &sciinfo[0]);
        printk("%s %d\n",__FUNCTION__,__LINE__);
#if defined(SMC_DETECT_KADC2)
        free_irq(IRQ_KADC1_ID, NULL);
#endif
#if defined(CONFIG_MT_CHIP_ARIA)
        iounmap(sciinfo[0].regbase);
#endif
        mutex_destroy(&sciinfo[0].ioctl_lock);
        if(sciinfo[0].dmammzbuf.startVirAddr) {
            mt_drv_mmz_unmap(&sciinfo[0].dmammzbuf);
            mt_drv_mmz_release(&sciinfo[0].dmammzbuf);
            //mt_drv_mmz_unmap_and_release(&sciinfo[0].dmammzbuf);
            sciinfo[0].dmammzbuf.startVirAddr=NULL;
        }
        sciinfo[0].init = 0;
    }

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(sciinfo[1].init && (R_SMC1_BASE_ADDR == sciinfo[1].regbase)) {
        sci_interrupt_disable(&sciinfo[1]);
        free_irq(IRQ_SMC1_ID, &sciinfo[1]);
#if defined(CONFIG_MT_CHIP_ARIA)
        iounmap(sciinfo[1].regbase);
#endif
        mutex_destroy(&sciinfo[1].ioctl_lock);
        sciinfo[1].init = 0;
    }
#endif
}

irqreturn_t sci_interrupt(int irq, void *dev_id)
{
    struct sci_info *info = (struct sci_info *)dev_id;
    u_char plugin_status = 0;
    u_char status04 = 0;
    u_char int_sta70 = 0,int_staA0 = 0;
    int flag_new_recv=0;

    int_sta70 = sci_get_smc_status(info);
    int_staA0 = READ_REG8(info->regbase + R_SMC_INT_STA1);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    sci_clear_interrupt_status(info,int_sta70);
    if(int_staA0 & 0x01){
        sci_clear_interrupt_status_overload(info,int_staA0);
    }
#endif
    status04 = sci_get_reg_status(info);
#if DEBUG
    MT_INFO_SCI("status reg 0x%x\n",status04);
#endif

    if(int_sta70) {
        ;//chipverify_log("int70=%x\n",int_sta70);              //for fpga test .isr status
    }

    if(status04 & 0x60) {
        chipverify_log("DERR_TYPE=%x\n",(status04 & 0x60));  //for fpga test .paritystatus
        if(0==info->paritystatus) {
            info->paritystatus = 1;
        }
    }

    plugin_status = ((status04 >> 1) & 0x01);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#if defined(SMC_DETECT_KADC2)
    //use kadc. move check to smc_kadc_int_handler
#else
    if(plugin_status && (0x2 == (int_sta70 & 0x2))) {
#if DEBUG
        MT_INFO_SCI("smc %d plug-in\n",info->id);
#endif
        // sci_activate(info);
    } else if((!plugin_status) && (0x1 == (int_sta70 & 0x1))) {
#if DEBUG
        MT_INFO_SCI("smc %d plug-out\n",info->id);
#endif
        sci_deactivate_poweroff(info);
    }
#endif
#else
    if(plugin_status && info->status == SCI_STATUS_DEACTIVATE) {
#if DEBUG
        MT_INFO_SCI("smc %d plug-in\n",info->id);
#endif
        // sci_activate(info);
    } else if((!plugin_status) && (info->status == SCI_STATUS_RESET)) {
#if DEBUG
        MT_INFO_SCI("smc %d plug-out\n",info->id);
#endif
        sci_deactivate(info,1);
    }
#endif

    if(info->attr.tx_finish_en || info->attr.recetime_en || info->attr.blktime_en || info->attr.rsttime_en) {
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        if(1 == info->attr.tx_finish_en) {
            if(int_sta70 == 0x10) {
                info->writeend = 1;
                __smp_mb();
                wake_up(&info->wq);
                return IRQ_HANDLED;
            } else if(int_sta70 & 0x10) {
                info->writeend = 1;
                __smp_mb();
            }
        }

        if(1 == info->attr.recetime_en) {
            if(int_sta70 & 0x80) {
                spin_lock(&info->wrlock);
                info->intstatus = int_sta70;
                info->readable = 1;
                __smp_mb();
                spin_unlock(&info->wrlock);
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }

        if(1 == info->attr.blktime_en) {
            if(int_sta70 & 0x20) {
                spin_lock(&info->wrlock);
                info->intstatus = int_sta70;
                info->readable = 1;
                __smp_mb();
                spin_unlock(&info->wrlock);
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }

        if(1 == info->attr.rsttime_en) {
            if(int_sta70 & 0x40) {
                spin_lock(&info->wrlock);
                info->intstatus = int_sta70;
                info->readable = 1;
                __smp_mb();
                spin_unlock(&info->wrlock);
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }
#else
        if(1 == info->attr.tx_finish_en) {
            if(int_sta70 & 0x01) {
                info->writeend = 1;
                __smp_mb();
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }

        if(1 == info->attr.recetime_en) {
            if(int_sta70 & 0x08) {
                spin_lock(&info->wrlock);
                info->intstatus = int_sta70;
                info->readable = 1;
                __smp_mb();
                spin_unlock(&info->wrlock);
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }

        if(1 == info->attr.blktime_en) {
            if(int_sta70 & 0x04) {
                spin_lock(&info->wrlock);
                info->intstatus = int_sta70;
                info->readable = 1;
                __smp_mb();
                spin_unlock(&info->wrlock);
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }

        if(1 == info->attr.rsttime_en) {
            if(int_sta70 & 0x02) {
                spin_lock(&info->wrlock);
                info->intstatus = int_sta70;
                info->readable = 1;
                __smp_mb();
                spin_unlock(&info->wrlock);
                wake_up(&info->wq);
                return IRQ_HANDLED;
            }
        }
#endif
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(int_sta70 & 0x4) { //recv new
        flag_new_recv=1;
    }
#endif

    if((1==flag_new_recv) || (0==sci_hwfifo_is_empty(info))) {
        spin_lock(&info->wrlock);
        if(sci_hwfifo_to_rxfifo(info) > 0) {
            info->readable = 1;
            __smp_mb();
            wake_up(&info->wq);
        }
        spin_unlock(&info->wrlock);
    } else if(info->writeend == 1) {
        wake_up(&info->wq);
        return IRQ_HANDLED;
    }

    if((info->readable != 1) && (info->writeend == 1)) {
        wake_up(&info->wq);
        return IRQ_HANDLED;
    }

    return IRQ_HANDLED;
}


static void sci_interrupt_enable(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    reg_trans_ctrl *tmp = (reg_trans_ctrl *)&reg;
    tmp->ie_cmd = 0;
#if defined(SMC_DETECT_KADC2)
    tmp->ie_insert = 0;
    tmp->ie_remove = 0;
#else
    tmp->ie_insert = 1;
    tmp->ie_remove = 1;
#endif
    tmp->ie_rx = 1;
    WRITE_REG8(reg, info->regbase + R_SMC_TRANS_CTRL);
}

static void sci_interrupt_disable(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_TRANS_CTRL);
    reg_trans_ctrl *tmp = (reg_trans_ctrl *)&reg;
    tmp->ie_cmd = 0;
    tmp->ie_insert = 0;
    tmp->ie_remove = 0;
    tmp->ie_rx = 0;
    WRITE_REG8(reg, info->regbase + R_SMC_TRANS_CTRL);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
static void sci_clock_config(u_char sci0_clkbase, u_char sci1_clkbase)
{
    u32 reg32 = READ_REG32((void *)R_CLK_SMC_CFG);
    if(sci0_clkbase == SCI_BASECLK_APB) {
        reg32 &= ~0x1;
    } else {
        reg32 |= 0x1;
    }

    if(sci1_clkbase == SCI_BASECLK_APB) {
        reg32 &= ~(0x1 << 16);
    } else {
        reg32 |= (0x1 << 16);
    }

    WRITE_REG32(reg32,(void *)R_CLK_SMC_CFG);
}
#endif

static void sci_config_code_convention(struct sci_info *info)
{
    volatile u_char reg = READ_REG8(info->regbase + R_SMC_ETU1_SET);
    reg_etu1_set *tmp = (reg_etu1_set *)&reg;
    tmp->cvtset_en = 0;
    WRITE_REG8(reg, info->regbase + R_SMC_ETU1_SET);
}

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
static void sci_dump_registers(struct sci_info *info)
{
#if 1//DEBUG
    //   printk("smc %d@R_SMC_DBUF=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_DBUF));
    printk("smc %d@R_SMC_STA=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_STA));
    printk("smc %d@R_SMC_FRQ_CFG=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_FRQ_CFG));
    printk("smc %d@R_SMC_NC_SET=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_NC_SET));
    printk("smc %d@R_SMC_TRANS_CTRL=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_TRANS_CTRL));

    printk("smc %d@R_SMC_ETU1_SET=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_ETU1_SET));
    printk("smc %d@R_SMC_ETU2_SET=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_ETU2_SET));
    printk("smc %d@R_SMC_ETU3_SET=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_ETU3_SET));
    printk("smc %d@R_SMC_CMDLEN_H=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_CMDLEN_H));

    printk("smc %d@R_SMC_CMDLEN_L=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_CMDLEN_L));
    printk("smc %d@R_SMC_BUF_CNT_L=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_BUF_CNT_L));
    printk("smc %d@R_SMC_PIN_CFG=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_PIN_CFG));
    printk("smc %d@R_SMC_CPCTRL=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_CPCTRL));
    printk("smc %d@R_SMC_RESEND_CFG=0x%02x\n", info->id,  READ_REG8(info->regbase + R_SMC_RESEND_CFG));

    if(info->chipid == 1) {
        printk("smc %d@R_SMC0_MODE_CFG=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_MODE_CFG));
        printk("smc %d@R_SMC0_BLK_TIMEOUT_CNT0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_BLK_TIMEOUT_CNT0));
        printk("smc %d@R_SMC0_BLK_TIMEOUT_CNT1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_BLK_TIMEOUT_CNT1));
        printk("smc %d@R_SMC0_BLK_TIMEOUT_CNT2=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_BLK_TIMEOUT_CNT2));
        printk("smc %d@R_SMC0_BLK_TIMEOUT_CNT3=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_BLK_TIMEOUT_CNT3));

        printk("smc %d@R_SMC_RST_TIMEOUT_CNT0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RST_TIMEOUT_CNT0));
        printk("smc %d@R_SMC_RST_TIMEOUT_CNT1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RST_TIMEOUT_CNT1));
        printk("smc %d@R_SMC_RST_TIMEOUT_CNT2=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RST_TIMEOUT_CNT2));
        printk("smc %d@R_SMC_RST_TIMEOUT_CNT3=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RST_TIMEOUT_CNT3));

        printk("smc %d@R_SMC0_RCV_TIMEOUT_CNT0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RCV_TIMEOUT_CNT0));
        printk("smc %d@R_SMC0_RCV_TIMEOUT_CNT1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RCV_TIMEOUT_CNT1));
        printk("smc %d@R_SMC0_RCV_TIMEOUT_CNT2=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RCV_TIMEOUT_CNT2));
        printk("smc %d@R_SMC0_RCV_TIMEOUT_CNT3=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RCV_TIMEOUT_CNT3));

        printk("smc %d@R_SMC_INT_STA0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_INT_STA0));
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        printk("smc %d@R_SMC_RCV_DATA_CNTL=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RCV_DATA_CNTL));
        printk("smc %d@R_SMC_RCV_DATA_CNTH=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_RCV_DATA_CNTH));

        printk("smc %d@R_SMC_T_CNT_20=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_T_CNT_20));
        printk("smc %d@R_SMC_T_CNT_21=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_T_CNT_21));
        printk("smc %d@R_SMC_T_CNT_22=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_T_CNT_22));
        printk("smc %d@R_SMC_T_CNT_23=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_T_CNT_23));

        printk("smc %d@R_SMC_CFG_0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_CFG_0));
        printk("smc %d@R_SMC_CFG_1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_CFG_1));
        printk("smc %d@R_SMC_CFG_2=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_CFG_2));
        printk("smc %d@R_SMC_CFG_3=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_CFG_3));

        printk("smc %d@R_SMC_ACTVT_FINISH=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_ACTVT_FINISH));
        printk("smc %d@R_SMC_INT_STA1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_INT_STA1));
        printk("smc %d@R_SMC_INT_CLR0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_INT_CLR0));
        printk("smc %d@R_SMC_INT_CLR1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_INT_CLR1));

        printk("smc %d@R_SMC_5VIO_CTRL0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_5VIO_CTRL0));
        printk("smc %d@R_SMC_5VIO_CTRL1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_5VIO_CTRL1));
        printk("smc %d@R_SMC_5VIO_CTRL2=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_5VIO_CTRL2));

        printk("smc %d@R_SMC_OVERLOAD_CNT0=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_OVERLOAD_CNT0));
        printk("smc %d@R_SMC_OVERLOAD_CNT1=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_OVERLOAD_CNT1));
        printk("smc %d@R_SMC_OVERLOAD_CNT2=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_OVERLOAD_CNT2));
        printk("smc %d@R_SMC_OVERLOAD_CNT3=0x%02x\n", info->id, READ_REG8(info->regbase + R_SMC_OVERLOAD_CNT3));
#endif
    }
#endif
}
#endif

static mt_void sci_prochelp(mt_void)
{
    mt_drv_proc_echohelp("cat  /proc/mt/msp/sci0                  -- display all sci0 information\n");
    mt_drv_proc_echohelp("cat  /proc/mt/msp/sci1                  -- display all sci1 information\n");
    mt_drv_proc_echohelp("echo %s0 > /proc/mt/msp/sci0  -- init sci0\n",gInit_str);
    mt_drv_proc_echohelp("echo %s0 0/1 > /proc/mt/msp/sci0  -- modify sci0 detect cfg\n",gDetect_str);
    mt_drv_proc_echohelp("echo %s0 0/1 > /proc/mt/msp/sci0  -- modify sci0 vcc cfg\n",gVcc_str);
    mt_drv_proc_echohelp("echo %s0 value(eg:3570000~6000000) > /proc/mt/msp/sci0  -- modify sci0 frqdiv cfg\n",gFrq_str);
    mt_drv_proc_echohelp("echo %s0 0/1 > /proc/mt/msp/sci0  -- modify sci0 stopwidth cfg\n",gStopwidth_str);
    mt_drv_proc_echohelp("echo %s0 0/1 > /proc/msp/sci0  -- modify sci0 parity cfg\n",gParity_str);
    mt_drv_proc_echohelp("echo %s0 value(max 255 etu) > /proc/mt/msp/sci0  -- modify sci0 N cfg\n",gNguard_str);
    mt_drv_proc_echohelp("echo %s0 value > /proc/mt/msp/sci0  -- modify sci0 read timeout cfg\n",gTimeout_str);
    mt_drv_proc_echohelp("echo %s0 > /proc/mt/msp/sci0  -- sci0 get atr\n",gAtr_str);
    mt_drv_proc_echohelp("echo %s0 data(decimal data)> /proc/mt/msp/sci0  -- sci0 data send\n",gData_str);
    mt_drv_proc_echohelp("note:all echo information length must be no more than 63\n");
}

mt_s32 sci_procread(struct seq_file *p, mt_void *v)
{
    mt_proc_entry_t    *pProcItem;
    MT_UNF_SCI_PORT_E enSciPort = MT_UNF_SCI_PORT0;
    mt_u8 ii = 0;
    mt_char s8Buff[PROC_NAME_LEN];

    pProcItem = p->private;

    for (ii = MT_UNF_SCI_PORT0; ii < MT_UNF_SCI_PORT_BUTT; ii++) {
        memset(s8Buff, 0, sizeof(s8Buff));
        mt_osal_snprintf(s8Buff, PROC_NAME_LEN, "sci%d", ii);

        if (0 == strncmp(pProcItem->entry_name, s8Buff, PROC_NAME_LEN)) {
            enSciPort = ii;
            break;
        }
    }

    if(enSciPort>=2) {
        PROC_PRINT(p,"---------SCI%d Port is error!---------\n", enSciPort);
        return MT_SUCCESS;
    }

    PROC_PRINT(p,"---------SCI%d Info---------\n", sciinfo[enSciPort].id);
    if(0==sciinfo[enSciPort].init) {
        PROC_PRINT(p,"SCI%d have not open!\n", sciinfo[enSciPort].id);
        return MT_SUCCESS;
    }

    if((R_SMC0_BASE_ADDR != sciinfo[enSciPort].regbase) && (R_SMC1_BASE_ADDR != sciinfo[enSciPort].regbase)) {
        PROC_PRINT(p,"SCI%d have not init!\n", sciinfo[enSciPort].id);
        return MT_SUCCESS;
    }

    PROC_PRINT(p,"SCI%d Card Base Reg:%lx\n", sciinfo[enSciPort].id,sciinfo[enSciPort].regbase);
    PROC_PRINT(p,"SCI%d Card Base CLK:%ld\n", sciinfo[enSciPort].id,sciinfo[enSciPort].clkbase);

    if(sci_get_card_status(&sciinfo[enSciPort])) {
        PROC_PRINT(p,"SCI%d Card Status:Insert\n", sciinfo[enSciPort].id);
    } else {
        PROC_PRINT(p,"SCI%d Card Status:Remove\n", sciinfo[enSciPort].id);
    }

    PROC_PRINT(p,"SCI%d Card Logic Status:%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].status);

    PROC_PRINT(p,"SCI%d Card Detect :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.slot_type);

    PROC_PRINT(p,"SCI%d Card Vcc :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.vcc_en_level);
    PROC_PRINT(p,"SCI%d Card Hz :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.Hz);
    PROC_PRINT(p,"SCI%d Card Etu :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.etu);
    PROC_PRINT(p,"SCI%d Card N :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.N);
    PROC_PRINT(p,"SCI%d Card Parity :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.parity_en);
    PROC_PRINT(p,"SCI%d Card Error_handle_en :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.error_handle_en);
    PROC_PRINT(p,"SCI%d Card Stop_width :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.stop_width);
    PROC_PRINT(p,"SCI%d Card Clkpin_mode :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.clkpin_mode);
    PROC_PRINT(p,"SCI%d Card Iopin_mode :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.iopin_mode);
    PROC_PRINT(p,"SCI%d Card Rstpin_mode :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.rstpin_mode);
    PROC_PRINT(p,"SCI%d Card Read_timeout :%d\n", sciinfo[enSciPort].id,sciinfo[enSciPort].attr.read_timeout);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    PROC_PRINT(p,"SCI%d R_SMC_ENABLE_CLK_ADDR(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_SMC_ENABLE_CLK_ADDR, READ_REG32((void*)R_SMC_ENABLE_CLK_ADDR));
    PROC_PRINT(p,"SCI%d R_SMC_CLK_ADDR(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_SMC_CLK_ADDR, READ_REG32((void*)R_SMC_CLK_ADDR));
#else
    PROC_PRINT(p,"SCI%d CLK_REG(%x)=0x%08x\n", sciinfo[enSciPort].id, 0xBF500000, READ_REG32((void*)SYMPHONY_IO_VA(0xBF500000)));
    PROC_PRINT(p,"SCI%d CLK_REG(%x)=0x%08x\n", sciinfo[enSciPort].id, 0xBF500004, READ_REG32((void*)SYMPHONY_IO_VA(0xBF500004)));
    PROC_PRINT(p,"SCI%d CLK_REG(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_CLK_SMC_CFG, READ_REG32((void*)R_CLK_SMC_CFG));
    PROC_PRINT(p,"SCI%d R_PINMUX_ADDR1(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_PINMUX_ADDR1, READ_REG32((void*)R_PINMUX_ADDR1));
    PROC_PRINT(p,"SCI%d R_PINMUX_ADDR2(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_PINMUX_ADDR2, READ_REG32((void*)R_PINMUX_ADDR2));
    PROC_PRINT(p,"SCI%d R_PINMUX_SWPIN1_ADDR(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_PINMUX_SWPIN1_ADDR, READ_REG32((void*)R_PINMUX_SWPIN1_ADDR));
    PROC_PRINT(p,"SCI%d R_PINMUX_SWPIN2_ADDR(%lx)=0x%08x\n", sciinfo[enSciPort].id, R_PINMUX_SWPIN2_ADDR, READ_REG32((void*)R_PINMUX_SWPIN2_ADDR));
#endif
    //PROC_PRINT(p,"SCI%d R_SMC_DBUF=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_DBUF));
    PROC_PRINT(p,"SCI%d R_SMC_STA=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_STA));
    PROC_PRINT(p,"SCI%d R_SMC_FRQ_CFG=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_FRQ_CFG));
    PROC_PRINT(p,"SCI%d R_SMC_NC_SET=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_NC_SET));
    PROC_PRINT(p,"SCI%d R_SMC_TRANS_CTRL=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_TRANS_CTRL));

    PROC_PRINT(p,"SCI%d R_SMC_ETU1_SET=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_ETU1_SET));
    PROC_PRINT(p,"SCI%d R_SMC_ETU2_SET=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_ETU2_SET));
    PROC_PRINT(p,"SCI%d R_SMC_ETU3_SET=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_ETU3_SET));
    PROC_PRINT(p,"SCI%d R_SMC_CMDLEN_H=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CMDLEN_H));

    PROC_PRINT(p,"SCI%d R_SMC_CMDLEN_L=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CMDLEN_L));
    PROC_PRINT(p,"SCI%d R_SMC_BUF_CNT_L=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_BUF_CNT_L));
    PROC_PRINT(p,"SCI%d R_SMC_PIN_CFG=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_PIN_CFG));
    PROC_PRINT(p,"SCI%d R_SMC_CPCTRL=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CPCTRL));
    PROC_PRINT(p,"SCI%d R_SMC_RESEND_CFG=0x%02x\n", sciinfo[enSciPort].id,  READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RESEND_CFG));

    if(sciinfo[enSciPort].chipid == 1) {
        PROC_PRINT(p,"SCI%d R_SMC0_MODE_CFG=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_MODE_CFG));
        PROC_PRINT(p,"SCI%d @R_SMC0_BLK_TIMEOUT_CNT0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_BLK_TIMEOUT_CNT0));
        PROC_PRINT(p,"SCI%d @R_SMC0_BLK_TIMEOUT_CNT1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_BLK_TIMEOUT_CNT1));
        PROC_PRINT(p,"SCI%d @R_SMC0_BLK_TIMEOUT_CNT2=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_BLK_TIMEOUT_CNT2));
        PROC_PRINT(p,"SCI%d @R_SMC0_BLK_TIMEOUT_CNT3=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_BLK_TIMEOUT_CNT3));

        PROC_PRINT(p,"SCI%d @R_SMC_RST_TIMEOUT_CNT0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RST_TIMEOUT_CNT0));
        PROC_PRINT(p,"SCI%d @R_SMC_RST_TIMEOUT_CNT1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RST_TIMEOUT_CNT1));
        PROC_PRINT(p,"SCI%d @R_SMC_RST_TIMEOUT_CNT2=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RST_TIMEOUT_CNT2));
        PROC_PRINT(p,"SCI%d @R_SMC_RST_TIMEOUT_CNT3=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RST_TIMEOUT_CNT3));

        PROC_PRINT(p,"SCI%d @R_SMC0_RCV_TIMEOUT_CNT0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RCV_TIMEOUT_CNT0));
        PROC_PRINT(p,"SCI%d @R_SMC0_RCV_TIMEOUT_CNT1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RCV_TIMEOUT_CNT1));
        PROC_PRINT(p,"SCI%d @R_SMC0_RCV_TIMEOUT_CNT2=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RCV_TIMEOUT_CNT2));
        PROC_PRINT(p,"SCI%d @R_SMC0_RCV_TIMEOUT_CNT3=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RCV_TIMEOUT_CNT3));

        PROC_PRINT(p,"SCI%d @R_SMC_INT_STA0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_INT_STA0));
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        PROC_PRINT(p,"SCI%d @R_SMC_RCV_DATA_CNTL=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RCV_DATA_CNTL));
        PROC_PRINT(p,"SCI%d @R_SMC_RCV_DATA_CNTH=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_RCV_DATA_CNTH));

        PROC_PRINT(p,"SCI%d @R_SMC_T_CNT_20=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_T_CNT_20));
        PROC_PRINT(p,"SCI%d @R_SMC_T_CNT_21=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_T_CNT_21));
        PROC_PRINT(p,"SCI%d @R_SMC_T_CNT_22=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_T_CNT_22));
        PROC_PRINT(p,"SCI%d @R_SMC_T_CNT_23=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_T_CNT_23));

        PROC_PRINT(p,"SCI%d @R_SMC_CFG_0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CFG_0));
        PROC_PRINT(p,"SCI%d @R_SMC_CFG_1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CFG_1));
        PROC_PRINT(p,"SCI%d @R_SMC_CFG_2=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CFG_2));
        PROC_PRINT(p,"SCI%d @R_SMC_CFG_3=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_CFG_3));

        PROC_PRINT(p,"SCI%d @R_SMC_ACTVT_FINISH=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_ACTVT_FINISH));
        PROC_PRINT(p,"SCI%d @R_SMC_INT_STA1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_INT_STA1));
        PROC_PRINT(p,"SCI%d @R_SMC_INT_CLR0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_INT_CLR0));
        PROC_PRINT(p,"SCI%d @R_SMC_INT_CLR1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_INT_CLR1));

        PROC_PRINT(p,"SCI%d @R_SMC_5VIO_CTRL0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_5VIO_CTRL0));
        PROC_PRINT(p,"SCI%d @R_SMC_5VIO_CTRL1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_5VIO_CTRL1));
        PROC_PRINT(p,"SCI%d @R_SMC_5VIO_CTRL2=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_5VIO_CTRL2));

        PROC_PRINT(p,"SCI%d @R_SMC_OVERLOAD_CNT0=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_OVERLOAD_CNT0));
        PROC_PRINT(p,"SCI%d @R_SMC_OVERLOAD_CNT1=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_OVERLOAD_CNT1));
        PROC_PRINT(p,"SCI%d @R_SMC_OVERLOAD_CNT2=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_OVERLOAD_CNT2));
        PROC_PRINT(p,"SCI%d @R_SMC_OVERLOAD_CNT3=0x%02x\n", sciinfo[enSciPort].id, READ_REG8(sciinfo[enSciPort].regbase + R_SMC_OVERLOAD_CNT3));
#endif
    }

    return MT_SUCCESS;
}



mt_s32 sci_procwrite(struct file * file,
                     const char __user * buf, size_t count, loff_t *ppos)

{
    mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
    mt_char *param = NULL;
    unsigned long id = 0;
    unsigned long value = 0;
    SCI_ATTR_S attr;

    if(count > PROC_PARAM_MAXLEN) {
        MT_ERR_SCI("write data is too long!\n");
        return -EFAULT;
    }

    if(copy_from_user(ProcPara, buf, count)) {
        MT_ERR_SCI("write data is too long!\n");
        return -EFAULT;
    }
    ProcPara[PROC_PARAM_MAXLEN-1] = 0;
    param =ProcPara;
    if(strstr(ProcPara,"halfxtal")) {
        u32 apb_clk=0,regval=READ_REG32(SYMPHONY_IO_VA(0xbf140020));
        regval=(regval>>30);
        if(0==regval){//27M
            apb_clk = (27000000>>1);
        }else{
            apb_clk = (24000000>>1);
        }
        clk_set_rate(sci_drv->sci_dev[0]->priv.smc0phyclk, apb_clk);
    } else if(strstr(ProcPara,"gating off")){
        gatingoff=1;
    } else if(strstr(ProcPara,gInit_str) && ProcPara[0] == 'i') {
        param+=strlen(gInit_str);
        id = simple_strtoul(param, NULL, 0);
        switch(id) {
#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
        case 1:
            sci_init_1();
            break;
#endif
        case 0:
        default:
            sci_init_0();
            break;
        }
    } else if(strstr(ProcPara,gDetect_str) && ProcPara[0] == 'd') {
        param+=strlen(gDetect_str);
        id = simple_strtoul(param, NULL, 10);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        memcpy(&attr,&(sciinfo[id].attr),sizeof(SCI_ATTR_S));
        param+=2;
        value = simple_strtoul(param, NULL, 10);
        if(value >=2) {
            MT_ERR_SCI("value = %d is error!\n", value);
            return -EFAULT;
        }
        attr.slot_type= (unsigned char)value;
        sci_set_slot_type(&sciinfo[id],&attr);
    } else if(strstr(ProcPara,gVcc_str) && ProcPara[0] == 'v') {
        param+=strlen(gVcc_str);
        id = simple_strtoul(param, NULL, 10);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        param+=2;
        value = simple_strtoul(param, NULL, 0);
        if(value >=2) {
            MT_ERR_SCI("value = %d is error!\n", value);
            return -EFAULT;
        }
        sciinfo[id].attr.vcc_en_level= (unsigned char)value;
    } else if(strstr(ProcPara,gFrq_str) && ProcPara[0] == 'f') {
        param+=strlen(gFrq_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        memcpy(&attr,&(sciinfo[id].attr),sizeof(SCI_ATTR_S));
        param+=2;
        value = simple_strtoul(param, NULL, 0);
        attr.Hz = value;
        sci_set_clock_freq(&sciinfo[id],&attr);
    } else if(strstr(ProcPara,gStopwidth_str) && ProcPara[0] == 's') {
        param+=strlen(gStopwidth_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        memcpy(&attr,&(sciinfo[id].attr),sizeof(SCI_ATTR_S));
        param+=2;
        value = simple_strtoul(param, NULL, 0);
        if(value >=2) {
            MT_ERR_SCI("value = %d is error!\n", value);
            return -EFAULT;
        }
        attr.stop_width = (unsigned char)value;
        sci_set_stop_width(&sciinfo[id],&attr);
    } else if(strstr(ProcPara,gParity_str) && ProcPara[0] == 'p') {
        param+=strlen(gParity_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        memcpy(&attr,&(sciinfo[id].attr),sizeof(SCI_ATTR_S));
        param+=2;
        value = simple_strtoul(param, NULL, 0);
        if(value >=2) {
            MT_ERR_SCI("value = %d is error!\n", value);
            return -EFAULT;
        }
        attr.parity_en = (unsigned char)value;
        sci_set_parity_enable(&sciinfo[id],&attr);
    } else if(strstr(ProcPara,gNguard_str) && ProcPara[0] == 'n') {
        param+=strlen(gNguard_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        memcpy(&attr,&(sciinfo[id].attr),sizeof(SCI_ATTR_S));
        param+=2;
        value = simple_strtoul(param, NULL, 0);
        if(value >255) {
            MT_ERR_SCI("value = %d is error!\n", value);
            return -EFAULT;
        }
        attr.N = (unsigned char)value;
        sci_set_N(&sciinfo[id],&attr);
    } else if(strstr(ProcPara,gTimeout_str) && ProcPara[0] == 't') {
        param+=strlen(gTimeout_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        param+=2;
        value = simple_strtoul(param, NULL, 0);
        sciinfo[id].attr.read_timeout = value;
    } else if(strstr(ProcPara,gAtr_str) && ProcPara[0] == 'a') {
        param+=strlen(gAtr_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        sci_activate(&sciinfo[id]);
        sci_reset(&sciinfo[id]);
        //to do
    } else if(strstr(ProcPara,gData_str) && ProcPara[0] == 'd') {
        param+=strlen(gData_str);
        id = simple_strtoul(param, NULL, 0);
        if(id >=2) {
            MT_ERR_SCI("id = %d is error!\n", id);
            return -EFAULT;
        }
        if((R_SMC0_BASE_ADDR != sciinfo[id].regbase) && (R_SMC1_BASE_ADDR != sciinfo[id].regbase)) {
            MT_ERR_SCI("SCI%d have not init!\n", id);
            return -EFAULT;
        }
        //to do
    } else if(strstr(ProcPara,gHelp_str) && ProcPara[0] == 'h') {
        sci_prochelp();
    }

    return count;
}

static int sci_open(struct inode *inode, struct file *file)
{
    int idx = iminor(inode) - sci_drv->minor;
    struct sci_priv_data *sci_priv_data=&sci_drv->sci_dev[idx]->priv;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    u32 regvalue = 0;

    regvalue = READ_REG32((void*)R_SMC_ENABLE_CLK_ADDR);
    if(SMC_CLK_ENABLE != (regvalue & SMC_CLK_ENABLE)) {
        regvalue |= SMC_CLK_ENABLE;
        WRITE_REG32(regvalue,(void*)R_SMC_ENABLE_CLK_ADDR);
    }
#endif

    //printk(KERN_EMERG "%s,%d\n",__FUNCTION__,__LINE__);

    file->private_data = sci_priv_data;
    nonseekable_open(inode, file);
    atomic_inc(&sci_priv_data->sci_open_cnt_atomic);
#if DEBUG
    MT_INFO_SCI("open.ok.count %d\n", atomic_read(&sci_priv_data->sci_open_cnt_atomic));
#endif
    return 0;
}

static int sci_release(struct inode *inode, struct file *file)
{
    struct sci_priv_data *sci_priv_data=file->private_data;
    //int idx = iminor(inode) - sci_drv->minor;
    //struct sci_priv_data *sci_priv_data=&sci_drv->sci_dev[idx]->priv;
    //printk(KERN_EMERG "%s,%d\n",__FUNCTION__,__LINE__);
#if DEBUG
    MT_INFO_SCI("release.before.count %d\n",  atomic_read(&sci_priv_data->sci_open_cnt_atomic));
#endif
    if(atomic_dec_and_test(&sci_priv_data->sci_open_cnt_atomic)) {
        sci_exit();
    }

    return 0;
}

static struct file_operations sci_fops = {
    .owner = THIS_MODULE,
    .llseek = no_llseek,
    .unlocked_ioctl = sci_ioctl,
#ifdef CONFIG_COMPAT
    .compat_ioctl = sci_ioctl,
#endif
    .open = sci_open,
    .read = NULL,
    .write = NULL,
    .release = sci_release,
};
static u8 regs_resume[100];
static mt_s32 sci_suspend(struct platform_device *pdev, pm_message_t state)
{
    struct sci_device *sci_dev = platform_get_drvdata(pdev);

    if(sciinfo[0].init)
        sci_regs_rw(&sciinfo[0],regs_resume,0);

    if (!IS_ERR_OR_NULL(sci_dev->priv.smc0phyclk)) {
        if (__clk_is_enabled(sci_dev->priv.smc0phyclk)) {
            clk_disable_unprepare(sci_dev->priv.smc0phyclk);    //clk_disable(sci_dev->priv.smc0phyclk);
        }
    }

    printk("SCI OK\n");

    return MT_SUCCESS;
}

static mt_s32 sci_resume(struct platform_device *pdev)
{
    struct sci_device *sci_dev = platform_get_drvdata(pdev);

    if (!IS_ERR_OR_NULL(sci_dev->priv.smc0phyclk)) {
        clk_prepare_enable(sci_dev->priv.smc0phyclk);//clk_enable(sci_dev->priv.smc0phyclk);
    }
    if(sciinfo[0].init)
        sci_regs_rw(&sciinfo[0],regs_resume,1);

    {
        #if defined(SMC_ENABLE_5VIO)
        u32 regvalue;
        regvalue = READ_REG32((void*)R_KEY_5V_CLK);
        regvalue &= ~(0x7);
        regvalue |= 0xb000; //AE require:Reg0xbf13c034=0xb100
        WRITE_REG32(regvalue,(void*)R_KEY_5V_CLK);

        regvalue = READ_REG32((void*)R_KEY_5V_RST);
        regvalue &= ~(0x7);
        WRITE_REG32(regvalue,(void*)R_KEY_5V_RST);

        regvalue = READ_REG32((void*)R_KEY_5V_DAT);
        regvalue &= ~(0x7);
        WRITE_REG32(regvalue,(void*)R_KEY_5V_DAT);
        #endif

#if defined(SMC_DETECT_KADC2)
		//pinmux set in dtsi
        //mt_pinctrl_set_function(INDEX_SW_PIN_CTRL008, 1);
        drv_gpio_io_enable(GPIO_8,GPIO_MASK_ENABLE);
        drv_gpio_set_dir(GPIO_8,GPIO_DIR_INPUT);
#endif
    }
    {
        //for STR
        u32 val = READ_REG32((void*)R_KEY_ADC2_CFG);
        val &= ~(1<<1);
        WRITE_REG32(val,(void*)R_KEY_ADC2_CFG);
        if(sciinfo[0].init){
            kadc_interrupt_init();
        }
    }
    printk("SCI OK\n");

    return MT_SUCCESS;
}

static ssize_t sci_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
    ssize_t count = 0;
    struct sci_device *sci_dev = dev_get_drvdata(dev);
    count += snprintf(buffer + count, PAGE_SIZE - count, "sci: %d\n", sci_dev->idx);
    return count;
}

static DEVICE_ATTR_RO(sci);
static int sci_probe(struct platform_device *pdev)
{
    struct sci_device *sci_dev = NULL;
    int ret = 0;
    u32 idx = 0;
    u32 devidx = -1;
    struct resource *mem = NULL;
    //printk(KERN_EMERG "sci probe\n");
   	{//enable spi top clk
		u32 tmp = readl((volatile u32 *)SYMPHONY_IO_VA(0xbf50f804));
		tmp |=(3<<8);
		writel(tmp,(volatile u32 *)SYMPHONY_IO_VA(0xbf50f804));
	}
    of_property_read_u32(pdev->dev.of_node, "devidx", &devidx);
    idx = (devidx == -1) ? 0 : devidx;

    if (idx >= sci_drv->minors) {
        ret = -ENODEV;
        goto fail_alloc_minor;
    }

    sci_dev = kzalloc(sizeof(struct sci_device), GFP_KERNEL);
    if (!sci_dev) {
        ret = -ENOMEM;
        goto fail_kzalloc_lxc_ipc_dev;
    }
    sci_dev->devt = MKDEV(MAJOR(sci_drv->devt), sci_drv->minor + idx);
    sci_dev->idx = idx;
    sci_drv->sci_dev[idx] = sci_dev;
    sci_dev->p_dev = &pdev->dev;

    if (devidx == -1) {
        sci_dev->dev = device_create(sci_class, NULL, sci_dev->devt, NULL, UMAP_DEVNAME_SCI);
    } else {
        sci_dev->dev = device_create(sci_class, NULL, sci_dev->devt, NULL, UMAP_DEVNAME_SCI"%d", idx);
    }
    if (IS_ERR(sci_dev->dev)) {
        ret = PTR_ERR(sci_dev->dev);
        goto fail_device_create;
    }

    if (device_create_file(&pdev->dev, &dev_attr_sci)) {
        ret = -ENOENT;
        goto fail_device_create_file;
    }

    platform_set_drvdata(pdev, sci_dev);
    dev_set_drvdata(sci_dev->dev, sci_dev);

    atomic_set(&sci_dev->priv.sci_open_cnt_atomic,0);
    mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    sci_dev->priv.membase=(ulong)(mem->start);
    sci_dev->priv.smc0phyclk = devm_clk_get(&pdev->dev, NULL);
    if (sci_dev->priv.smc0phyclk) {
        ulong clk_tmp=0;
        clk_prepare_enable(sci_dev->priv.smc0phyclk);       //sure enable
        //clk_disable_unprepare(sci_dev->priv.smc0phyclk);    //sure disable
        //clk_set_rate(sci_dev->priv.smc0phyclk, 90000000);
        clk_tmp=clk_get_rate(sci_dev->priv.smc0phyclk);
        printk(KERN_EMERG "clk_tmp=%d\n", (int)clk_tmp);
    }

    /* register SCI proc function*/
    {
        mt_proc_entry_t  *pProcItem;
        mt_u8 ii = 0;
        mt_char s8Buff[MT_UNF_SCI_PORT1][PROC_NAME_LEN]= {{0}};
        for (ii = 0; ii <= MT_UNF_SCI_PORT0; ii++) {
            memset(&sciinfo[ii], 0, sizeof(struct sci_info));
            memset(s8Buff[ii], 0, sizeof(s8Buff[ii]));
            mt_osal_snprintf(s8Buff[ii], PROC_NAME_LEN, "sci%d", ii);

            pProcItem = mt_drv_proc_add_module(s8Buff[ii], MT_NULL, MT_NULL);
            if (!pProcItem) {
                MT_ERR_SCI("add sci%d proc failed.\n", ii);
                mt_drv_dev_unregister(&g_SciRegisterData);
                return MT_FAILURE;
            }

            pProcItem->read  = sci_procread;
            pProcItem->write = sci_procwrite;
        }
    }

    return 0;

fail_device_create_file:
    device_destroy(sci_class, sci_dev->devt);
    sci_dev->dev = NULL;
fail_device_create:
    sci_drv->sci_dev[idx] = NULL;
    kfree(sci_dev);
fail_kzalloc_lxc_ipc_dev:
fail_alloc_minor:
    return ret;
}

static int sci_remove(struct platform_device *pdev)
{
    int idx;
    struct sci_device *sci_dev = platform_get_drvdata(pdev);
    //printk(KERN_EMERG "%s,%d\n",__FUNCTION__,__LINE__);
    if(NULL == sci_dev) {
        return -1;
    }

    device_remove_file(&pdev->dev, &dev_attr_sci);
    device_destroy(sci_class, sci_dev->devt);
    idx = sci_dev->idx;
    kfree(sci_drv->sci_dev[idx]);
    sci_drv->sci_dev[idx] = NULL;
    sci_dev->dev = NULL;

    platform_set_drvdata(pdev, NULL);

    mt_drv_proc_rm_module("sci0");

    if (!IS_ERR_OR_NULL(sci_dev->priv.smc0phyclk)) {
        clk_disable_unprepare(sci_dev->priv.smc0phyclk);
        clk_put(sci_dev->priv.smc0phyclk);
    }

    kfree(sci_dev);

    return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id sci_of_match[] = {
    { .compatible = "mt_tech,sci-mt" },
    {},
};
MODULE_DEVICE_TABLE(of, sci_of_match);
#endif

static struct platform_driver sci_platform_driver = {
    .probe = sci_probe,
    .remove = sci_remove,
    .suspend = sci_suspend,
    .resume = sci_resume,
    .driver = {
        .name = SCI_NAME,
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(sci_of_match),
    }
};

int __init sci_drv_modinit(void)
{
    int ret=0;

    if (0 != mt_otp_get_hw_bonding(MT_OTP_SMCDis)) {
        MT_FATAL_SCI("OTP_SMC disabled\n");
        return -EACCES;
    }

    //printk(KERN_EMERG "sci in\n");
    sci_drv = kzalloc(sizeof(struct sci_driver), GFP_KERNEL);
    if (!sci_drv) {
        ret = -ENOMEM;
        goto fail_kzalloc_drv;
    }

    sci_class = class_create(SCI_NAME);
    if (IS_ERR(sci_class)) {
        ret = PTR_ERR(sci_class);
        goto fail_class_create;
    }

    sci_drv->minor = SCI_MINOR;
    sci_drv->minors = SCI_MINORS;
    sci_drv->devt = MKDEV(MT_DEVICE_MAJOR, sci_drv->minor);
    sci_drv->major = MAJOR(sci_drv->devt);

    cdev_init(&sci_drv->cdev, &sci_fops);
    sci_drv->cdev.owner = THIS_MODULE;

    ret = cdev_add(&sci_drv->cdev, sci_drv->devt, sci_drv->minors);
    if (ret) {
        ret = -EINVAL;
        goto fail_cdev_add;
    }

    ret = platform_driver_register(&sci_platform_driver);
    if (ret) {
        goto fail_platform_driver_register;
    }

    return 0;

fail_platform_driver_register:
    cdev_del(&sci_drv->cdev);
fail_cdev_add:
    class_destroy(sci_class);
    sci_class = NULL;
fail_class_create:
    kfree(sci_drv);
    sci_drv = NULL;
fail_kzalloc_drv:
    return ret;
}

void __exit sci_drv_modexit(void)
{
    if (NULL == sci_drv) {
        return;
    }

    cdev_del(&sci_drv->cdev);

    platform_driver_unregister(&sci_platform_driver);

    class_destroy(sci_class);
    sci_class = NULL;

    kfree(sci_drv);
    sci_drv = NULL;
}

