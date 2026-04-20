/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : drv_vdec_fw_helper.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/06/10
 * Description    : MT VFMW API helper interfaces(AP CPU).
 * History        :
 * 1.Date         : 2019/06/10
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/kthread.h>

#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <uapi/linux/sched/types.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/mutex.h>

#ifdef CONFIG_MT_CHIP_ARIA
#include <../arch/arm/mach-aria/clk-aria.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif

//#include "sys_define.h"
#include "drv_vdec_sys_define.h"

#include "mt_unf_common.h"
#include "mt_unf_demux.h"

#include "mt_drv_dma.h"

#include "vfmw.h"
#include "drv_timer.h"
#include "drv_vdec_ext.h"
#include "vconfig.h"

#undef LOG_TAG
#define LOG_TAG							"VFMW_HLP"
#include "Log.h"

//---------------------------------------------------------------------------//

#define VDEC_FW_MUTEX_MAX_NUM 4

static mt_s32 fw_vdec_timer_id = -1;
//static spinlock_t vdec_fw_lock;
static struct mutex vdec_fw_lock[VDEC_FW_MUTEX_MAX_NUM];
static mt_u32 mutex_state = 0;
//static unsigned long ulFlags;
static atomic_t fw_vdec_intr_atomic = ATOMIC_INIT(0);
static DECLARE_WAIT_QUEUE_HEAD(fw_vdec_thread_queue);

/* Change to share memory for AP/AV CPU */
#if (CFG_VFMW_ON_AVCPU == 1)
/* Shared Video ES Descriptor queue */
VES_INST_S *ves_buffer_inst = NULL;
#else
VES_INST_S ves_buffer_inst[CFG_VDEC_VES_INS_COUNT];
#endif

extern VDEC_FW_FUNCTION_S vdec_decoder_irq_fun;

#ifdef CONFIG_MT_CHIP_ARIA
extern unsigned int mt_get_sys_ctrl_base(void);
extern unsigned int aria_get_sec_pic_base(void);
extern unsigned int aria_get_hdvenc_base(void);
extern unsigned int aria_get_ddrmc_base(void);
#endif

//---------------------------------------------------------------------------//

static __inline mt_u32 dmx_inl_new(ulong port)
{
    //return *((volatile mt_u32 *)(port));
    return readl((volatile void*)port);
}

static __inline mt_u32 dmx_set_hw_wp_reg(ulong port, mt_u32 pti_channel, mt_u32 vdec_channel)
{
  //mt_u32 val = *((volatile mt_u32 *)(port));
  void __iomem *reg = (void*)port;
  mt_u32 val = readl(reg);
  mt_u32 mask_val[4] = {0xfffffff0, 0xffffff0f, 0xfffff0ff, 0xffff0fff};

  val = val & mask_val[vdec_channel];
  val = val | (pti_channel) << (4 * vdec_channel);
  //*((volatile mt_u32 *)(port)) = val;

  writel(val, reg);

  printk(KERN_ERR "dmx_set_hw_wp_reg, pti_channel %d vdec_channel %d  val %x \n",  pti_channel, vdec_channel, val);
  return MT_SUCCESS;
}

/* OS porting API for VFMW */
irqreturn_t VDEC_Decoder_Irq(int irq_number, void *vdec_irq_handle)
{
	if (vdec_decoder_irq_fun.pvdec_isr != NULL)
	{
		MLOGV("%s: call pvdec_isr %p\n",__FUNCTION__,vdec_decoder_irq_fun.pvdec_isr);
	    vdec_decoder_irq_fun.pvdec_isr();
	}
	else
	{
		MLOGV("[WARNING]pvdec_isr is null!\n");
	}

    return IRQ_HANDLED;
}

void request_vdec_irq(mt_u32 irq_number, void *vdec_irq_handle)
{
	int ret = -1;

#if defined(CONFIG_MT_CHIP_ARIA)
    ret = request_irq(irq_number/* + 32*//*irq_number already +32*/,
    				(irq_handler_t)vdec_irq_handle, IRQF_TRIGGER_HIGH, "aria_vdec_irq", NULL);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	ret = request_irq(irq_number/* + 32*//*irq_number already +32*/,
					(irq_handler_t)vdec_irq_handle, IRQF_TRIGGER_HIGH, "symphony4_vdec_irq", NULL);
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	ret = request_irq(irq_number/* + 32*//*irq_number already +32*/,
					(irq_handler_t)vdec_irq_handle, IRQF_TRIGGER_HIGH, "symphony6_vdec_irq", NULL);

#endif

	if (ret != 0)
	{
		MLOGE("[FATAL]request_irq failed! return %d\n",ret);
	}
	else
	{
		MLOGV("request irq %d success.\n",irq_number);
	}
}

void release_vdec_irq(mt_u32 irq_number)
{
	free_irq(irq_number, NULL);
	MLOGV("free irq %d success.\n",irq_number);
}

//---------------------------------------------------------------------------//

static long vdec_kthread_run(int (*threadfn)(void *data), void *data, const char *namefmt, ...)
{
    struct task_struct *p_struct;
    struct sched_param param;
    int sucess;

#if CFG_VDEC_RUN_CPU0//set vdec to cpu0
    struct cpumask cpumask;
    long rc;
    memset(&cpumask, 0, sizeof(cpumask));
    cpumask_set_cpu(0, &cpumask);
#endif
    p_struct = kthread_run(threadfn, data, namefmt);
    if(NULL == p_struct)
    {
      MLOGE("kthread_run failed!\n");
      return 0;
    }
#if CFG_VDEC_RUN_CPU0//set vdec to cpu0
    rc = sched_setaffinity(p_struct->pid, &cpumask);
#endif

    param.sched_priority = CFG_VDEC_THREAD_PRO;
    sucess = sched_setscheduler(p_struct, CFG_VDEC_THREAD_SCH, &param);

    return (long)p_struct;
}

static int vdec_kthread_stop(long *task_id)
{
	return kthread_stop((struct task_struct *)task_id);
}

static int vdec_kthread_should_stop(void)
{
	return (int)kthread_should_stop();
}

#if 0
static signed long vdec_schedule_timeout(signed long timeout)
{
	long ret = timeout;
	int n = atomic_read(&fw_vdec_intr_atomic);

	//if fw_vdec_intr_atomic == 0, need schedule wait.
	//if vdec isr inc fw_vdec_intr_atomic, return directly, no schedule wait.
	if (n == 0)
	{
		//timeout?
		//return schedule_timeout(HZ);	//-NG

		//FIX: Bug 124441
		//case of Interrupt -> sleep 1s
		//wait_event_interruptible_timeout might better!
		//ret = schedule_timeout_uninterruptible(HZ);
		ret = schedule_timeout_killable(HZ/20);

		//timeout
		if (ret == 0)
			return 0;
	}

	atomic_dec(&fw_vdec_intr_atomic);

	return ret;
}

static int vdec_wake_up_process(void *p)
{
    int sucess = 0;

	atomic_inc(&fw_vdec_intr_atomic);

    sucess = wake_up_process((struct task_struct *)p);

    if (sucess) {
		set_tsk_need_resched((struct task_struct *)p);
		set_tsk_thread_flag((struct task_struct *)p, TIF_SIGPENDING);
    }
    //set_tsk_need_resched((struct task_struct *)p);
    //printk("msleep_interruptible_vdec rc = %x\n", sucess);

    return sucess;
}
#else
//FIX: Bug 124441
//wait_event_interruptible_hrtimeout better!
static signed long vdec_schedule_timeout(signed long timeout)
{
	long ret = timeout;

	if (timeout <= 0)
		timeout = 50;	//hw decode maxium time

	ret = wait_event_interruptible_hrtimeout(fw_vdec_thread_queue,
				(atomic_read(&fw_vdec_intr_atomic) == 1),
				ms_to_ktime(timeout));

	atomic_set(&fw_vdec_intr_atomic, 0);

	return ret;
}

static int vdec_wake_up_process(void *p)
{
    int sucess = 0;

	atomic_set(&fw_vdec_intr_atomic, 1);

	wake_up_interruptible(&fw_vdec_thread_queue);

	return sucess;
}
#endif

//not used
#if 0
static int vdec_sched_setscheduler(void *p)
{
    struct sched_param param;
    int sucess;

    param.sched_priority = 99;
    sucess = sched_setscheduler((struct task_struct *)p, SCHED_FIFO, &param);

    return sucess;
}
#endif

static void msleep_interruptible_vdec(int ms)
{
    unsigned long rc;

    // printk("msleep_interruptible_vdec befor \n");
//    rc = msleep_interruptible(ms < 10 ? 10 : ms);
    rc = msleep_interruptible(ms <= 0 ? 1 : ms);
    clear_tsk_thread_flag(current, TIF_SIGPENDING);
    //printk("msleep_interruptible_vdec rc = %x\n", rc);
    //return rc;
    return;
}

static void msleep_vdec(int value)
{
	msleep(value <= 0 ? 1 : value);
}

static mt_s32 mt_vdec_timer_request(unsigned int ms, void *pfn, void *para)
{
    static char name[] = "vdec timer";
    mt_s32 sucess;

    fw_vdec_timer_id = sucess = mt_drv_timer_request(ms, (timer_fn)pfn, para, 1, name);
//    fw_vdec_timer_id = sucess = mt_drv_kn_timer_request(ms, (timer_fn)pfn, para, 1, name);
    MLOGV("request timer: %d\n",fw_vdec_timer_id);

    return sucess;
}

static mt_s32 mt_vdec_timer_release(void)
{
    mt_s32 sucess = MT_SUCCESS;

    if (fw_vdec_timer_id != -1)
    {
	    sucess = mt_drv_timer_release(fw_vdec_timer_id);
//    sucess = mt_drv_kn_timer_release(fw_vdec_timer_id);

	    MLOGV("release timer: %d\n",fw_vdec_timer_id);
	    fw_vdec_timer_id = -1;
	}

    return sucess;
}

static mt_s32 hw_set_es_buffer_write_point_channel(mt_s32 pti_channel, mt_s32 vdec_channel)
{
  ulong pti_base_addr = mt_get_tsi_base();

  dmx_set_hw_wp_reg(pti_base_addr + 0x6002c, pti_channel, vdec_channel);


  return MT_SUCCESS;
}

static mt_void sw_get_desc_point(mt_u32 channel_id, mt_u32 *pu32DescWrite, mt_u32 *pu32DescRead)
{
  if(channel_id < 16)
  {
    ulong channel_tsi_base_addr = mt_get_tsi_base() + 0x60000;
    ulong ch_dscrpt_offset = 0x100;
    ulong ch_dscrpt_rd_base_addr = 0x00000128;
    ulong ch_dscrpt_wr_base_addr = 0x00000130;
    ulong descriptor_readpointer;
    ulong descriptor_writepointer; // offset

    //descriptor_readpointer = *((mt_u32 *)(channel_tsi_base_addr + channel_id*ch_dscrpt_offset + ch_dscrpt_rd_base_addr));
    //descriptor_writepointer = *((mt_u32 *)(channel_tsi_base_addr + channel_id*ch_dscrpt_offset + ch_dscrpt_wr_base_addr)); // offset
    descriptor_readpointer = dmx_inl_new(channel_tsi_base_addr + channel_id*ch_dscrpt_offset + ch_dscrpt_rd_base_addr);
    descriptor_writepointer = dmx_inl_new(channel_tsi_base_addr + channel_id*ch_dscrpt_offset + ch_dscrpt_wr_base_addr);

    *pu32DescRead = descriptor_readpointer;
    *pu32DescWrite = descriptor_writepointer;
  }
  else
  {
    mt_u32 VesChannelId;
    VesChannelId = channel_id & 0xf;

    *pu32DescWrite = ves_buffer_inst[VesChannelId].u32KnlDescWriterOff;
    *pu32DescRead = ves_buffer_inst[VesChannelId].u32KnDescReadOff;
  }
  MLOGV("ch: 0x%x, DESC RD: 0x%x, WR: 0x%x\n",channel_id,*pu32DescRead,*pu32DescWrite);
}

static mt_void sw_set_desc_point(mt_u32 channel_id, mt_u32 u32DescWrite, mt_u32 u32DescRead)
{
  if(channel_id < 16)
  {
    ulong channel_tsi_base_addr = mt_get_tsi_base() + 0x60000;
    ulong ch_dscrpt_offset = 0x100;
    ulong ch_dscrpt_rd_base_addr = 0x00000128;
    ulong p_dscrpt_rd_addr;

    p_dscrpt_rd_addr = channel_tsi_base_addr + channel_id*ch_dscrpt_offset + ch_dscrpt_rd_base_addr;
    //*((mt_u32 *)p_dscrpt_rd_addr) = u32DescRead;
	writel(u32DescRead, (volatile void*)p_dscrpt_rd_addr);
  }
  else
  {
    mt_u32 VesChannelId;
    VesChannelId = channel_id & 0xf;

    ves_buffer_inst[VesChannelId].u32KnDescReadOff = u32DescRead;
  }
  MLOGV("ch: 0x%x, DESC RD: 0x%x, WR: 0x%x\n",channel_id,u32DescRead,u32DescWrite);
}

static mt_s32 vdec_fw_mutex_create(void)
{
    mt_u32 idx = 0;

    while(mutex_state & (1<<idx))
    {
        idx++;
        if(idx >= VDEC_FW_MUTEX_MAX_NUM)
        {
            return -1;
        }
    }

    mutex_init(&vdec_fw_lock[idx]);
    mutex_state |= (1<<idx);

    return idx;
}

static mt_s32 vdec_fw_mutex_lock(mt_u32 mutex_id)
{
  //spin_lock(&vdec_fw_lock);
  //spin_lock_irqsave(&vdec_fw_lock, ulFlags);//????ù?ж
  if(mutex_id >= VDEC_FW_MUTEX_MAX_NUM)
  {
    return -1;
  }

  mutex_lock(&vdec_fw_lock[mutex_id]);
  return 0;
}

static mt_s32 vdec_fw_mutex_unlock(mt_u32 mutex_id)
{
  //spin_unlock(&vdec_fw_lock);
  //spin_unlock_irqrestore(&vdec_fw_lock, ulFlags);
  if(mutex_id >= VDEC_FW_MUTEX_MAX_NUM)
  {
    return -1;
  }
  mutex_unlock(&vdec_fw_lock[mutex_id]);
  return 0;
}

static mt_s32 vdec_fw_mutex_destroy(mt_u32 mutex_id)
{
  //spin_unlock(&vdec_fw_lock);
  //spin_unlock_irqrestore(&vdec_fw_lock, ulFlags);
  if(mutex_id >= VDEC_FW_MUTEX_MAX_NUM)
  {
    return -1;
  }

  mutex_destroy(&vdec_fw_lock[mutex_id]);
  mutex_state &= ~(1<<mutex_id);

  return 0;
}

static phys_addr_t vdec_dma_memcpy(phys_addr_t p_dst, const phys_addr_t p_src, UINT32 size, mt_u32 dma_ch)
{
    mt_s32 ret;
	hal_dma_io_param_t dma_param;
	int tmo = 30000;	//3s
	dma_usize_t usize;
	dma_burst_num_t bnum;

    if(p_dst == 0 || p_src == 0 || size == 0) {
//        MLOGE("vdec dma memcpy: invalid params %p, %p, %u\n", p_dst, p_src, size);
        return 0;
    }
    
	memset(&dma_param, 0, sizeof(hal_dma_io_param_t));
	dma_param.param.len = (mt_u32)size;
	dma_param.param.phy_src_addr = (phys_addr_t)p_src;
	dma_param.param.vir_src_addr = 0;
	dma_param.param.phy_dst_addr = (phys_addr_t)p_dst;
	dma_param.param.vir_dst_addr = 0;

	//align 128
	if ((dma_param.param.phy_src_addr & (128-1)) == 0
		&& (dma_param.param.phy_dst_addr & (128-1)) == 0
		&& (dma_param.param.len & (128-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM16;
	}
	//align 64
	else if ((dma_param.param.phy_src_addr & (64-1)) == 0
		&& (dma_param.param.phy_dst_addr & (64-1)) == 0
		&& (dma_param.param.len & (64-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM8;
	}
	//align 32
	else if ((dma_param.param.phy_src_addr & (32-1)) == 0
		&& (dma_param.param.phy_dst_addr & (32-1)) == 0
		&& (dma_param.param.len & (32-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM4;
	}
	//align 16
	else if ((dma_param.param.phy_src_addr & (16-1)) == 0
		&& (dma_param.param.phy_dst_addr & (16-1)) == 0
		&& (dma_param.param.len & (16-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM2;
	}
	//align 8
	else if ((dma_param.param.phy_src_addr & (8-1)) == 0
		&& (dma_param.param.phy_dst_addr & (8-1)) == 0
		&& (dma_param.param.len & (8-1)) == 0)
	{
		usize = DMA_USIZE_64BIT;
		bnum = DMA_BURST_NUM1;
	}
	//align 4
	else if ((dma_param.param.phy_src_addr & (4-1)) == 0
		&& (dma_param.param.phy_dst_addr & (4-1)) == 0
		&& (dma_param.param.len & (4-1)) == 0)
	{
		usize = DMA_USIZE_32BIT;
		bnum = DMA_BURST_NUM1;
	}
	//align 2
	else if ((dma_param.param.phy_src_addr & (2-1)) == 0
		&& (dma_param.param.phy_dst_addr & (2-1)) == 0
		&& (dma_param.param.len & (2-1)) == 0)
	{
		usize = DMA_USIZE_16BIT;
		bnum = DMA_BURST_NUM1;
	}
	else
	{
		usize = DMA_USIZE_8BIT;
		bnum = DMA_BURST_NUM1;
	}

	dma_param.param.config.dst_peripheral = 0xf; // memory
	dma_param.param.config.src_peripheral = 0xf; // memory
	dma_param.param.config.dst_endian = 0; // big endian
	dma_param.param.config.src_endian = 0; // big endian
	dma_param.param.config.dst_clk = 0; // AXI clock
	dma_param.param.config.src_clk = 0; // AXI clock
	dma_param.param.config.dst_i = DMA_ADDR_INC;
	dma_param.param.config.src_i = DMA_ADDR_INC;
	dma_param.param.config.dst_usize = usize;
	dma_param.param.config.src_usize = usize;
	dma_param.param.config.dst_bsize = bnum;
	dma_param.param.config.src_bsize = bnum;
	dma_param.param.control.int_link_en = 1;
	dma_param.param.control.int_node_en = 1;
	dma_param.param.control.chn_param_reg_en = 0;

	dma_param.chn_id = dma_ch;
	if (dma_param.chn_id < 0 || dma_param.chn_id >= DMA_CHN_ID_MAX)
	{
		MLOGE("dma get free channel failed!\n");
		return 0;
	}

	ret = hal_dma_start(dma_param.chn_id, (mt_void *)&dma_param.param, NULL);
	if (ret != DMA_SUCCESS)
	{
		MLOGE("dma channel(%d) start failed!\n",dma_param.chn_id);
		return 0;
	}

	while (DMA_STATUS_STOP != hal_dma_check(dma_param.chn_id))
	{
		//avoid too much timer interrupt
		//usleep_range(100, 100);
		usleep_range(100, 1000);
		tmo --;
		if (tmo <= 0)
		{
			MLOGE("dma channel(%d) timeout!\n",dma_param.chn_id);
			break;
		}
	}

	ret = hal_dma_stop(dma_param.chn_id);
	if (ret != DMA_SUCCESS)
	{
		MLOGE("dma channel(%d) stop failed!\n",dma_param.chn_id);
		return 0;
	}

	return p_dst;

}

static mt_void vdec_fw_dma_memcpy(phys_addr_t p_dst, const phys_addr_t p_src, UINT32 size)
{
	vdec_dma_memcpy(p_dst, p_src, size, DMA_CHANNEL_SECURE_VIDEO_VC1);

}
mt_void * VDEC_phys_to_virt(phys_addr_t address);

static mt_void *vdec_fw_phys_to_virt(phys_addr_t address)
{
	void * va_vir = VDEC_phys_to_virt(address);
	return va_vir;
}

static __inline int bit_mask(int set_bitnumber_high, int set_bitnumber_low)
{
  return (((1<<(set_bitnumber_high - set_bitnumber_low + 1)) - 1) << set_bitnumber_low);
}

static mt_s32 vdec_fw_get_page_mode(mt_void)
{
    unsigned long addr;
    unsigned int mode;
    void __iomem *reg ;
    addr  = mt_get_ddrmc_base() + 0x0204;
//    addr &= 0xfffffffc; // Align the address to 32-bit space.

	reg = (void*) addr;

  // 	mode = readl(reg);
 //	mode = *(( volatile unsigned int *)addr);
    mode = readl((volatile void*)addr);
    mode &= bit_mask(9, 8);
    mode >>= 8;

    return mode;
}

//---------------------------------------------------------------------------//

/* Aria & Symphony4: OS porting APIs for FW */
void vdec_get_fw_input_param(inputParam_t *in_param)
{
    in_param->printf 							= _printk;
    
    in_param->page_mode                         = vdec_fw_get_page_mode();

#if defined(CONFIG_MT_CHIP_SYMPHONY4) ||  defined(CONFIG_MT_CHIP_SYMPHONY6)
	/* FW use to reset VDEC HW */
	in_param->fw_get_sys_ctrl_base				= mt_get_crm_base;
#else
    in_param->fw_get_sys_ctrl_base 				= mt_get_sys_ctrl_base;
#endif

    in_param->fw_get_vdec_base 					= mt_get_vdec_base;
    in_param->fw_get_display_base 				= mt_get_display_base;
    in_param->fw_get_tsi_base 					= mt_get_tsi_base;

    in_param->kernel_task_run 					= vdec_kthread_run;
    in_param->kernel_task_stop 					= vdec_kthread_stop;
    in_param->kernel_task_should_stop 			= vdec_kthread_should_stop;

    in_param->kernel_msleep 					= msleep_vdec;
    in_param->kernel_schedule_timeout 			= vdec_schedule_timeout;

    in_param->fw_vdec_timer_request 			= mt_vdec_timer_request;
    in_param->fw_vdec_timer_release 			= mt_vdec_timer_release;

    in_param->set_es_buffer_write_point_channel = hw_set_es_buffer_write_point_channel;

	//assigned outside
    //in_param->VdecCallback 					= VDEC_EventHandle;

#ifdef CONFIG_MT_CHIP_ARIA
    in_param->fw_get_sec_pic_base 				= aria_get_sec_pic_base;
    in_param->fw_get_hdvenc_base 				= aria_get_hdvenc_base;
    in_param->fw_get_ddrmc_base 				= aria_get_ddrmc_base;

    in_param->clock_gate_enable 				= aria_clock_gate_enable;
    in_param->clock_gate_disable 				= aria_clock_gate_disable;
    in_param->clock_gate_is_enabled 			= aria_clock_gate_is_enabled;
#else
	in_param->fw_get_sec_pic_base 				= mt_get_pic0_base;
	in_param->fw_get_hdvenc_base				= mt_get_hdvenc_base;
	in_param->fw_get_ddrmc_base 				= mt_get_ddrmc_base;

	in_param->clock_gate_enable 				= NULL;
	in_param->clock_gate_disable				= NULL;
	in_param->clock_gate_is_enabled 			= NULL;

#endif

    in_param->mutex_create 						= vdec_fw_mutex_create;
    in_param->mutex_lock 						= vdec_fw_mutex_lock;
    in_param->mutex_unlock 						= vdec_fw_mutex_unlock;
    in_param->mutex_destroy                 = vdec_fw_mutex_destroy;

    in_param->fw_sw_get_desc_point 				= sw_get_desc_point;
    in_param->fw_sw_set_desc_point 				= sw_set_desc_point;

    in_param->pav_task_sleep 					= msleep_interruptible_vdec;
    in_param->kernel_wake_up_process 			= vdec_wake_up_process;

	/* not used */
	in_param->pav_task_create					= NULL;
	in_param->fw_get_hw_timer_base				= NULL;
	in_param->fw_request_vdec_irq				= NULL;

	/* only used in AVCPU mode */
	in_param->fw_push_frame						= NULL;
	in_param->fw_release_frame					= NULL;
    in_param->dma_memcpy                        = vdec_fw_dma_memcpy;
    in_param->phys_to_virt_non_cache            = vdec_fw_phys_to_virt;
	in_param->edma_read 					   = NULL;
	in_param->edma_write						= NULL;
}

///////////////////////////////////////////////////////////////////////////////
//
// DMX Helper functions
//
///////////////////////////////////////////////////////////////////////////////
extern mt_s32 MT_DRV_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr);

mt_u32 dmx_get_avsync_flag(mt_u8 ch)
{
	ulong pti_base_addr = mt_get_tsi_base();
	
	return ((dmx_inl_new(pti_base_addr + 0x60100 + ch*0x100) & 0x300) >> 8);
}

mt_u32 dmx_get_esbuff_id(mt_handle hDmxCh)
{
	mt_s32 ret;
	MT_UNF_DMX_CHAN_ATTR_S Attr;
	mt_u8 esBuffId1 = 0;

	memset(&Attr, 0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
	Attr.esBuffId1 = &esBuffId1;

	ret = MT_DRV_DMX_GetChannelAttr(hDmxCh, &Attr);
	if (ret == MT_SUCCESS)
	{
		MLOGV("hDmxCh 0x%lx, ES Buffer ID: %u\n",hDmxCh,esBuffId1);
		return esBuffId1;
	}
	else
	{
		MLOGE("MT_DRV_DMX_GetChannelAttr failed! return %d\n",ret);
		return 0;
	}
}
extern mt_u8*  DMX_OsiGetChannel_Descbuf(mt_u32 ChanId);
mt_u8* dmx_get_desc_start_vaddr(mt_handle hDmxCh)
{
	mt_s32 ret;
	MT_UNF_DMX_CHAN_ATTR_S Attr;
	mt_u8 esBuffId1 = 0;

	memset(&Attr, 0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
	Attr.esBuffId1 = &esBuffId1;

	ret = MT_DRV_DMX_GetChannelAttr(hDmxCh, &Attr);
	if (ret == MT_SUCCESS)
	{
		MLOGV("hDmxCh 0x%lx, ES Buffer ID: %u\n",hDmxCh,esBuffId1);
	}
	else
	{
		MLOGE("MT_DRV_DMX_GetChannelAttr failed! return %d\n",ret);
	}
	return DMX_OsiGetChannel_Descbuf(esBuffId1);

}

mt_u32 dmx_get_esbuff_start_addr(mt_u32 buff_id)
{
	ulong pti_base_addr = mt_get_tsi_base();

	return (dmx_inl_new(pti_base_addr + 0x6011c + buff_id * 0x100)) & 0xfffffff0;
}

mt_u32 dmx_get_esbuff_end_addr(mt_u32 buff_id)
{
	ulong pti_base_addr = mt_get_tsi_base();

	return dmx_inl_new(pti_base_addr + 0x60124 + buff_id * 0x100);
}

mt_u32 dmx_get_desc_start_addr(mt_u32 buff_id)
{
	ulong pti_base_addr = mt_get_tsi_base();

	return (dmx_inl_new(pti_base_addr + 0x60118 + buff_id * 0x100)) & 0xfffffff0;
}

mt_u32 dmx_get_desc_end_addr(mt_u32 buff_id)
{
	ulong pti_base_addr = mt_get_tsi_base();

	return dmx_inl_new(pti_base_addr + 0x60120 + buff_id * 0x100);
}

///////////////////////////////////////////////////////////////////////////////
//
// SW PTS Descriptor functions
//
///////////////////////////////////////////////////////////////////////////////
void vdec_descriptor_init(mt_u32 ch,
							phys_addr_t u32PhyAddr, unsigned char *pu8KnlVirAddr,
							mt_u32 u32Size,
							phys_addr_t u32DescPhyAddr, unsigned char *pu8KnlVirDescAddr,
							mt_u32 u32KnlVirDescBufSize)
{
	BUG_ON(ch >= CFG_VDEC_VES_INS_COUNT);

	memset(&ves_buffer_inst[ch], 0, sizeof(VES_INST_S));
    ves_buffer_inst[ch].hBuf = ch;
    ves_buffer_inst[ch].u32PhyAddr = u32PhyAddr;
    ves_buffer_inst[ch].pu8KnlVirAddr = pu8KnlVirAddr;
    //ves_buffer_inst[ch].pu8UsrVirAddr = pu8UsrVirAddr;
    ves_buffer_inst[ch].u32Size = u32Size;

    ves_buffer_inst[ch].pu8KnlVirDescAddr = pu8KnlVirDescAddr;
    ves_buffer_inst[ch].u32KnlVirDescBufSize = u32KnlVirDescBufSize;
    //ves_buffer_inst[ch].u32KnlDescWriterOff = u32KnlDescWriterOff;
    //ves_buffer_inst[ch].u32KnDescReadOff = u32KnDescReadOff;
    //ves_buffer_inst[ch].u32VesBufferChannelId = u32VesBufferChannelId;
    ves_buffer_inst[ch].u32DescPhyAddr = u32DescPhyAddr;
}

mt_u32 vdec_descriptor_get_write_ptr(mt_u32 ch)
{
	BUG_ON(ch >= CFG_VDEC_VES_INS_COUNT);
	return ves_buffer_inst[ch].u32KnlDescWriterOff;
}

void vdec_descriptor_set_write_ptr(mt_u32 ch, mt_u32 offset)
{
	BUG_ON(ch >= CFG_VDEC_VES_INS_COUNT);
	ves_buffer_inst[ch].u32KnlDescWriterOff = offset;
}

mt_u32 vdec_descriptor_get_read_ptr(mt_u32 ch)
{
	BUG_ON(ch >= CFG_VDEC_VES_INS_COUNT);
	return ves_buffer_inst[ch].u32KnDescReadOff;
}

