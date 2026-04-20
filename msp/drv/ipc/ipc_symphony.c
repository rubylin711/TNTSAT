/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/proc_fs.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/kdev_t.h>
#include <asm/page.h>
#include <linux/cdev.h>
#include <linux/compiler.h>
#include <linux/device.h>
#include <linux/clocksource.h>
#include <linux/jiffies.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/kobject.h>
#include<linux/interrupt.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <asm/cacheflush.h>

#include "mt_mach/irq.h"

#include "mt_type.h"
#include "mt_cache.h"
#include "ipc.h"

#include "mt_drv_log.h"
#include "mt_debug.h"

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "concerto_regs_av.h"

#include "mt_drv_mmz.h"

#ifndef MT_ASSERT
#define MT_ASSERT(x) do {} while(0)
#endif
#define MAX_PIPE_NUMS    36
#define MAX_PIPE_DEPTH   36
#define MAX_CHANNEL_NUMS 32

/* Secure: only channel 16-31 */
#define REE2AV_CHANNEL_FROM 16
#define REE2AV_CHANNEL_TO   31

#ifdef CONFIG_MT_CHIP_SYMPHONY4
/* IPC has ACK register */
//#define IPC_HAVE_ACK
/* enable secure ipc plugin */
#define IPC_SECURE_PLUGIN
#endif

//#define IPC_DEBUG_ENABLE

#ifdef IPC_DEBUG_ENABLE
#define IPC_PRINTF printk
//#define IPC_PRINTF MT_WARN_LOG

#else
#define IPC_PRINTF(...)  do{}while(0)
#endif

/* Test Case for Bug 109271  */
//#define TESTCASE_IPC_AVTOAP_MULTI_ASYNC_MSG
//#define TESTCASE_IPC_APTOAV_MULTI_ASYNC_MSG

extern void av_irq_request(u8 irq, void (*handle)(void));
#ifndef __KERNEL__
extern void  mtos_irq_request(u8 irq, void (*handle)(void), u8 type);
#endif

extern void   hal_dcache_flush(void *p_addr, u32 len);
extern void   hal_dcache_invalidate(void *p_addr, u32 len);

extern void *ipc_get_irq_handler(unsigned int irq);

#ifdef IPC_SECURE_PLUGIN
extern void *ipc_secure_malloc(unsigned int size);
extern mt_u32 ipc_secure_get_int_state(void);
extern mt_u32 ipc_secure_get_av_msg(int ch);
#endif

/*
 *  we rewrite CPU_IS_AP
 *  new added from here.
 */
#undef AP_CPU_ID
#undef AV_CPU_ID
#undef CPU_IS_AP
#undef LOCAL_CPU_ID

#define AP_CPU_ID  0
#define AV_CPU_ID  1
#define CPU_ID_MASK  0x3FF
#define INVALID_CPU_ID 0xFF


#if defined (CONFIG_MIPS)
extern u32 get_ebase(void);
//#define LOCAL_CPU_ID  (get_ebase() & CPU_ID_MASK)
#define LOCAL_CPU_ID  (read_c0_ebase() & CPU_ID_MASK)
#elif defined (CONFIG_ARM) || defined(CONFIG_ARM64)
//TODO-20190705
#define LOCAL_CPU_ID	AP_CPU_ID
#endif

#define CPU_IS_AP  ((AP_CPU_ID == LOCAL_CPU_ID) ? 1 : 0)
#define CPU_NOT_READY  0
#define CPU_READY      1
/*
 *  new added end here.
 */

static  ulong g_av_ipc_struct_addr = 0;
static  ulong g_av_mb_msg_addr = 0;
static  u32 g_av_max_pipe_num = 0;
//static  pipe_info_t g_av_pipe_info[MAX_PIPE_NUMS];
static  ipc_msg_t  g_av_ipc_msg[MAX_PIPE_NUMS][MAX_PIPE_DEPTH];
//static  mb_msg_t  g_av_mb_msg[1];

static ulong g_ap_ipc_struct_addr = 0;
ulong g_ap_mb_msg_addr = 0;
static u32 g_ap_max_pipe_num = 0;

static mmz_buffer_s g_mb_msg_mmz_buf = {0};

#define REG32_READ(addr) 		HAL_GET_U32((volatile u32 *)(ulong)addr)
#define REG32_WRITE(addr,val) 	HAL_PUT_U32((volatile u32 *)(ulong)addr, (u32)val)

static void* _VA_MSG(phys_addr_t pa)
{
	return mmz_va(&g_mb_msg_mmz_buf, pa);
}

static phys_addr_t _PA_MSG(void *va)
{
	return mmz_pa(&g_mb_msg_mmz_buf, va);
}

/*
 * new added for ready flag for AP and AV core
 * ready - return 1
 * not ready - return 0
 *
 * cpuid - AP_CPU_ID, or AV_CPU_ID
 */
u32 symphony_check_cpu_ready(u32 cpuid)
{
  ulong addr = (AP_CPU_ID == cpuid) ? MCPU_MB_INIT : SCPU_MB_INIT;

  return (CPU_READY == REG32_READ(addr));

}

/*
 * once this function is called, the peer cpu will know it's ready
 * this is the only way the peer cpu know it's ready
 * so, this function must be called only after all the necessay work has been done by the local cpu
 *
 * fail - return 1
 * succeed - return 0
 *
 * generally,  cpuid - LOCAL_CPU_ID, 'cause this function only could set itself in READY state
 */
u32 symphony_set_local_cpu_ready(void)
{
  ulong addr = (AP_CPU_ID == LOCAL_CPU_ID) ? (ulong)MCPU_MB_INIT : (ulong)SCPU_MB_INIT;

  REG32_WRITE(addr, CPU_READY);

  return 0;
}
/*
 * new added for new mailbox, end here
 */

static RET_CODE Writepipe(pipe_info_t *p_pipe,ipc_msg_t *p_msg)
{
    if((p_pipe->WritePt + 1) % p_pipe->pipe_depth == p_pipe->ReadPt){

        IPC_PRINTF("[AP][IPC]Writepipe: alert-->pipe full\n");
        MT_ERR_LOG("\nalert-->pipe full\n");
        return ERR_FAILURE;
    }

    memcpy((char *)(p_pipe->Datapipe + p_pipe->WritePt),(char *)p_msg,sizeof(ipc_msg_t));
    p_pipe->WritePt = (p_pipe->WritePt + 1) % p_pipe->pipe_depth;
    return SUCCESS;
}

static RET_CODE Readpipe(pipe_info_t *p_pipe,ipc_msg_t *p_msg)
{

    if(p_pipe->ReadPt == p_pipe->WritePt) {

        return ERR_FAILURE;
    }
    memcpy((char *)p_msg,(char *)(p_pipe->Datapipe + p_pipe->ReadPt),sizeof(ipc_msg_t));
    p_pipe->ReadPt = (p_pipe->ReadPt + 1) % p_pipe->pipe_depth;
    return SUCCESS;
}

// msg send function can run in task context and irq context
static RET_CODE __in_irq_ipc_msg_send(u32 local_id, ipc_msg_t *p_msg,pipe_info_t *p_g_pipe, int max_pipe_num)
{
    RET_CODE ret = ERR_FAILURE;
    pipe_info_t *p_pipe = NULL;
    int i =0 ;

    for(i = 0; i < max_pipe_num; i++)    {
          p_pipe =p_g_pipe + i;
          if (p_pipe->used == 0){
			  MT_ERR_LOG("[AP][IPC]__in_irq_ipc_msg_send: pipe(%x) not found! \n\n",local_id);
              return  ERR_FAILURE;
          }

          if((p_pipe->pipe_info.p_sid == local_id))
        //   && ((p_pipe->pipe_info.msg_id_mask&p_msg->msg_id) == p_msg->msg_id))
          {
              ret = Writepipe(p_pipe,p_msg);
              return ret;
          }
    }
	MT_ERR_LOG("[AP][IPC]__in_irq_ipc_msg_send: pipe(%x) not found! \n\n",local_id);
    return ERR_FAILURE;
}


// msg receive function can only run in task context
static RET_CODE __ipc_msg_receive(u32 local_id, ipc_msg_t *p_msg,pipe_info_t *p_g_pipe, int max_pipe_num)
{
    RET_CODE ret = ERR_FAILURE;
    pipe_info_t *p_pipe = NULL;
    int i = 0;


    for(i = 0; i < max_pipe_num; i++){
        p_pipe = p_g_pipe + i;

        if (p_pipe->used == 0){
            return  ERR_FAILURE;
        }

        if((p_pipe->pipe_info.p_did == local_id)) {
#ifdef __KERNEL__ //peacer add
            ret = Readpipe(p_pipe,p_msg);
#else
            if(CPU_IS_AP)
                mtos_critical_enter(&sr);

            ret = Readpipe(p_pipe,p_msg);

            if(CPU_IS_AP)
                mtos_critical_exit(sr);
#endif
            if(ret == ERR_FAILURE)
                continue;
            else
                return ret;

        }
    }
    return ret;
}

#if 0
static void av_isr(void)
{
	mb_msg_t *p_mb = NULL;
    RET_CODE ret = ERR_FAILURE;
	int i = 0;
    u32 state = REG32_READ(SCPU_MB_INT_STATE);
    MT_INFO_LOG(" AV_ISR \n");
	for(i = 0; i < MAX_CHANNEL_NUMS; i++){
        if ((state & (0x1 << i)) != 0){
            p_mb = (mb_msg_t *)(REG32_READ(SCPU_MB_INFO_REG(i)));
            break;
        }
    }

    hal_dcache_invalidate((void * )(p_mb), sizeof(mb_msg_t));

    if(p_mb->local_id == 0xdead)
    {
        __ipc_msg_receive(p_mb->local_id, &p_mb->p_msg, (pipe_info_t *)g_av_ipc_struct_addr, g_av_max_pipe_num);

		REG32_WRITE(SCPU_MB_INTCLR, REG32_READ(SCPU_MB_INTCLR) | state);
        REG32_WRITE(SCPU_MB_ENACLR, REG32_READ(SCPU_MB_ENACLR) | state);/* clear enable bit */

	    return ;

    }

    ret =  __in_irq_ipc_msg_send(p_mb->local_id, &p_mb->p_msg, (pipe_info_t *)g_av_ipc_struct_addr, g_av_max_pipe_num);

	if(ret == SUCCESS){
        MT_INFO_LOG("__in_irq_ipc_msg_send down\n");
		/* this must be done last */
        REG32_WRITE(SCPU_MB_INTCLR, REG32_READ(SCPU_MB_INTCLR) | state);
        REG32_WRITE(SCPU_MB_ENACLR, REG32_READ(SCPU_MB_ENACLR) | state);/* clear enable bit */
    }
    else{
        MT_INFO_LOG("#av-irq send-error\n");
        MT_ASSERT(0); //FIX ME  PIPE FULL OR NO THIS PIPE
       // MT_ASSERT1(0); //FIX ME  PIPE FULL OR NO THIS PIPE
    }
}
#endif

static void mt_flush_data_cache_range(mt_u8 *addr, unsigned int size)
{
    mt_dcache_flush(((void *)addr), (size_t)size);
}

static void mt_invaild_data_cache_range(mt_u8 *addr, unsigned int size)
{
    mt_dcache_invalid(((void *)addr), (size_t)size);
}

/*
 * ACK usage:
 *    AP CPU          AVCPU
 *   set_ack()  ->
 *              <-   clear_ack()
 *   wait_ack()
 */
#ifdef IPC_HAVE_ACK
static void set_ack(int channel)
{
	mt_u32 val;

	IPC_PRINTF("[AP]%s: channel=%d\n",__FUNCTION__,channel);

	val = REG32_READ(MCPU_MB_ACK);
	val |= (0x01 << channel);
	REG32_WRITE(MCPU_MB_ACK, val);
}

/* clear AVCPU ACK */
static void clear_ack(int channel)
{
	mt_u32 val;

	IPC_PRINTF("[AP]%s: channel=%d\n",__FUNCTION__,channel);

	val = REG32_READ(SCPU_MB_ACK);
	val &= ~(0x01 << channel);
	REG32_WRITE(SCPU_MB_ACK, val);
}

static unsigned int wait_ack(int channel, unsigned int to_ms)
{
	mt_u32 val;
	mt_u32 mask = (0x01 << channel);
	mt_u32 debug_to = 0;

	IPC_PRINTF("[AP]%s: channel=%d, to=%u(ms)\n",__FUNCTION__,channel,to_ms);

	while (to_ms > 0)
	{
        /*
          ap send to av cpu need av ack ,
          Symphony4: avcpu clear the ACK register for this transf is ok.
        */
		val = REG32_READ(MCPU_MB_ACK);
		if ((val & mask) == 0)	/*cleared by avcpu*/
			break;
		else
			usleep_range(100, 1000);

		to_ms --;
		debug_to ++;

		//avcpu ack timeout!!!
		if ((debug_to % 3000) == 0)
		{
			printk("[WARNING]%s: __ap_send_to_av() wait ack %u!\n", __FUNCTION__,debug_to);
		}
	}

	IPC_PRINTF("[AP]%s: channel=%d, return %u(ms)\n",__FUNCTION__,channel,to_ms);
	return to_ms;
}
#else
static void set_ack(int channel)
{
	//do nothing
}
static void clear_ack(int channel)
{
	//do nothing
}
static unsigned int wait_ack(int channel, unsigned int to_ms)
{
	mt_u32 val;
	mt_u32 mask = (0x01 << channel);
	mt_u32 debug_to = 0;

	IPC_PRINTF("[AP]%s: channel=%d, to=%u(ms)\n",__FUNCTION__,channel,to_ms);

	while (to_ms > 0)
	{
        /*
          ap send to av cpu need av ack ,
          Symphony1/2: must polling mailbox intr state,if state=0 for this transf is ok.
        */
		val = REG32_READ(SCPU_MB_INT_STATE);
		if ((val & mask) == 0)
			break;
		else
			usleep_range(100, 1000);

		to_ms --;
		debug_to ++;

		//avcpu ack timeout!!!
		if ((debug_to % 3000) == 0)
		{
			printk("[WARNING]%s: __ap_send_to_av() wait ack %u!\n", __FUNCTION__,debug_to);
			dump_stack();
		}
	}

	IPC_PRINTF("[AP]%s: channel=%d, return %u(ms)\n",__FUNCTION__,channel,to_ms);
	return to_ms;
}
#endif

static irqreturn_t ap_isr(int irq,void* dev)
{
/* mips cache address */
#define mips_phys_to_cache_addr(a)	(((a)&0x1fffffff) | 0x80000000)

    mb_msg_t *p_mb = NULL;
    phys_addr_t p_mb_phy_addr;
    RET_CODE ret = ERR_FAILURE;
	int i = 0;
	/* for IRQ from AV CPU */
	//unsigned int irq;
#if 0
	void (*irq_handler)(int irq, void *dev_id, struct pt_regs *regs);
#endif

	u32 clear_bit = 0;
    u32 state = REG32_READ(MCPU_MB_INT_STATE);

    IPC_PRINTF("[AP][IPC]***************** AP_ISR [%x]\n",state);

	/* Secure: Symphony4 REE could not read MCPU_MB_INT_STATE */
	if (state == 0)
	{
#ifdef IPC_SECURE_PLUGIN
		state = ipc_secure_get_int_state();
#else
		//FIXME: clear which interrupt?
		MT_ERR_LOG("[AP][IPC]AP_ISR: state is zero!\n\n");
		return IRQ_HANDLED;
#endif
	}

	for(i = REE2AV_CHANNEL_FROM; i <= REE2AV_CHANNEL_TO; i++){
        if ((state & (0x1 << i)) != 0){
            //p_mb = (mb_msg_t *)((ulong)REG32_READ(MCPU_MB_INFO_REG(i)));
            p_mb_phy_addr = (phys_addr_t)REG32_READ(MCPU_MB_INFO_REG(i));

#ifdef IPC_SECURE_PLUGIN
            //if (p_mb == NULL)
			//    p_mb = (mb_msg_t *)ipc_secure_get_av_msg(i);
			if (p_mb_phy_addr == 0)
				p_mb_phy_addr = (phys_addr_t)ipc_secure_get_av_msg(i);
#endif
            break;
        }
    }

	if (i >= MAX_CHANNEL_NUMS || p_mb == NULL)
	{
		MT_ERR_LOG("[AP][IPC]AP_ISR: NO CHANNEL\n\n");

		/* this must be done last */
		REG32_WRITE(MCPU_MB_INTCLR, REG32_READ(MCPU_MB_INTCLR) | state);
		REG32_WRITE(MCPU_MB_ENACLR, REG32_READ(MCPU_MB_ENACLR) | state);	/* clear enable bit */

		return IRQ_HANDLED;
	}

	//convert p_mb physical address to virtual address(ARM) or cache address(MIPS)
	//IPC_PRINTF("[AP][IPC]ap_isr: received msg addr = %p\n",p_mb);
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
	//p_mb = (mb_msg_t *)__va((ulong)p_mb);
	//p_mb = (mb_msg_t *)__va(p_mb_phy_addr);
	p_mb = (mb_msg_t *)_VA_MSG(p_mb_phy_addr);
#elif defined(CONFIG_MIPS)
	//p_mb = (mb_msg_t *)mips_phys_to_cache_addr((u32)p_mb);
	p_mb = (mb_msg_t *)mips_phys_to_cache_addr((u32)p_mb_phy_addr);
#else
#error "Please config select one arch!"
#endif

    //hal_dcache_invalidate((void * )(p_mb), sizeof(mb_msg_t));
    mt_invaild_data_cache_range((mt_u8 *)(p_mb), sizeof(mb_msg_t));

#if defined(TESTCASE_IPC_AVTOAP_MULTI_ASYNC_MSG) || defined(IPC_DEBUG_ENABLE)
	printk("[AP][IPC]%s: stat 0x%x, ch %d, mb %p, lid 0x%x, msg 0x%x\n",__FUNCTION__,
		state,
		i,
		p_mb,
		p_mb->local_id,
		p_mb->p_msg.msg_id);

#if defined(TESTCASE_IPC_AVTOAP_MULTI_ASYNC_MSG)
	mdelay(10);
#endif
#endif

    if(p_mb->local_id == 0xdeadbeef) {  // when av cpu in except mode
        panic("[AP][IPC]****av cpu in exception mode ,ap cpu stop run ****");
    }

//something bad, system not stable!?
#if 0
	if ((p_mb->p_msg.app_id & IPC_APP_INTR_FLAG) != 0)
	{
		irq = p_mb->p_msg.msg_id;

		irq_handler = ipc_get_irq_handler(irq);
        IPC_PRINTF("%s: irq(%u), handler(%p)\n",__FUNCTION__,irq,irq_handler);

		if (irq_handler != NULL)
		{
			irq_handler(irq, NULL, NULL);
		}
		else
		{
	        IPC_PRINTF("%s: irq(%u) handler is null!\n",__FUNCTION__,irq);
		}
		ret = SUCCESS;
	}
	else
#endif
	{
    	ret = __in_irq_ipc_msg_send(p_mb->local_id, &p_mb->p_msg, (pipe_info_t *)g_ap_ipc_struct_addr, g_ap_max_pipe_num);
    }

	/* FIX: just clear channle[i] */
	clear_bit = 0x01 << i;

    if(ret == SUCCESS){
        IPC_PRINTF("[AP][IPC]__in_irq_ipc_msg_send down\n");
		/* this must be done last */
        REG32_WRITE(MCPU_MB_INTCLR, REG32_READ(MCPU_MB_INTCLR) | clear_bit);
        REG32_WRITE(MCPU_MB_ENACLR, REG32_READ(MCPU_MB_ENACLR) | clear_bit);	 /* clear enable bit */
    }else{
        MT_ERR_LOG("[AP][IPC]#ap-irq-send-error\n");

		/* this must be done last */
        REG32_WRITE(MCPU_MB_INTCLR, REG32_READ(MCPU_MB_INTCLR) | clear_bit);
        REG32_WRITE(MCPU_MB_ENACLR, REG32_READ(MCPU_MB_ENACLR) | clear_bit);	/* clear enable bit */

        MT_ASSERT(0);   //FIX ME  PIPE FULL OR NO THIS PIPE
    }

	clear_ack(i);

	/* check if state is zero after isr process */
	if (1)
	{
		u32 state_new = state & (~clear_bit);
		if (state_new != 0)
		{
	        MT_ERR_LOG("[WARNING][AP][IPC]STATE(0x%x) is not zero after isr process!\n",state_new);

			/* just for test: symphony1 re-trigger the isr */
#if 0
			REG32_WRITE(MCPU_MB_ENASET, REG32_READ(MCPU_MB_ENASET) | state_new);
			REG32_WRITE(MCPU_MB_INTSET, REG32_READ(MCPU_MB_INTSET) | state_new);
#endif

			//TODO:
			//symphony4: do not clear the interrupt till the state is zero?
		}
	}

	return IRQ_HANDLED;
}


static void av_symphony_ipc_init(int max_pipe_num)
{

}


#ifdef __KERNEL__
#define DRV_NAME "ap_ipc"
#endif

/* for sym4 secure ipc call ampshm, ampshm call ipc, might dead lock */
static void _ap_symphony_ipc_late_init(void)
{
#define AVCPU_PHY_ADDR_LIMIT			(480UL*0x100000)

	mb_msg_t* p_mb_msg = NULL;
	phys_addr_t p_mb_msg_phy_addr = 0;

	if (g_ap_mb_msg_addr != 0)
		return;

//#ifdef IPC_SECURE_PLUGIN
#if 0
	p_mb_msg = (mb_msg_t*)ipc_secure_malloc(sizeof(mb_msg_t)*2*MAX_CHANNEL_NUMS);

	//TODO
	p_mb_msg_phy_addr = ?;

	IPC_PRINTF("ipc_init: secure IPC enabled!\n");
#else

/* kmalloc might >= 480M, avcpu can't access >= 480M */
#if 0
	/* FIX: Bug 109271, async msg conflict! */
    //mb_msg_t* p_mb_msg = (mb_msg_t*)kmalloc(sizeof(mb_msg_t)*2,GFP_KERNEL);
	p_mb_msg = (mb_msg_t*)kmalloc(sizeof(mb_msg_t)*2*MAX_CHANNEL_NUMS,GFP_KERNEL);

	p_mb_msg_phy_addr = (phys_addr_t)__pa(p_mb_msg);
#else

	{
		mt_s32 ret;

		ret = mt_drv_mmz_alloc_and_map("MB_MSG", NULL, sizeof(mb_msg_t)*2*MAX_CHANNEL_NUMS, CACHE_LINE_SIZE, &g_mb_msg_mmz_buf);
		if (ret == MT_SUCCESS)
		{
			p_mb_msg = (mb_msg_t*)g_mb_msg_mmz_buf.startVirAddr;
			p_mb_msg_phy_addr = (phys_addr_t)g_mb_msg_mmz_buf.startPhyAddr;
		}
	}


#endif

#endif

	if (p_mb_msg == NULL)
	{
        IPC_PRINTF("%s: allocate mb_msg failed!\n",__FUNCTION__);
        BUG();
	}

	mt_flush_data_cache_range((mt_u8*)p_mb_msg, 2*MAX_CHANNEL_NUMS*sizeof(mb_msg_t));

#if defined (CONFIG_MIPS)
    p_mb_msg = (mb_msg_t*)(u32 *)(KSEG1ADDR((u32)p_mb_msg));
#endif

	memset(p_mb_msg, 0, 2*MAX_CHANNEL_NUMS*sizeof(mb_msg_t));
	g_ap_mb_msg_addr = (ulong)p_mb_msg;

	/*
	 * g_ap_mb_msg_addr: Must <= 480MB, AVCPU can only access 0 ~ 480MB!!!
	 */
	if (p_mb_msg_phy_addr >= (phys_addr_t)AVCPU_PHY_ADDR_LIMIT)
	{
        IPC_PRINTF("%s: mb_msg addr(0x%X) >= AVCPU limit(0x%X)!\n",__FUNCTION__,p_mb_msg_phy_addr,AVCPU_PHY_ADDR_LIMIT);
        BUG();
	}

	MT_INFO_LOG(" \n DebugInfo:  \n ap request irq[0x%x], pipenum[0x%x], pipe_startaddr [0x%x], msg_addr [0x%x], msg_size [0x%x]\n",
		IRQ_MAILBOX_ID,g_ap_max_pipe_num,g_ap_ipc_struct_addr,g_ap_mb_msg_addr,sizeof(mb_msg_t));
}

static void _ap_symphony_ipc_init(int max_pipe_num)
{
	int ret;
	pipe_info_t* p_pipe = NULL;

    if (g_ap_ipc_struct_addr != 0)
    {
        return;// be inited
	}

	p_pipe = (pipe_info_t*)kmalloc(sizeof(pipe_info_t)*max_pipe_num,GFP_KERNEL);
	if (p_pipe == NULL)
	{
        IPC_PRINTF("%s: allocate pipe failed!\n",__FUNCTION__);
        BUG();
	}

	mt_flush_data_cache_range((mt_u8*)p_pipe, sizeof(pipe_info_t)*max_pipe_num);

#if defined (CONFIG_MIPS)
	p_pipe = (pipe_info_t*)(u32 *)(KSEG1ADDR((u32)p_pipe));
#endif

	MT_INFO_LOG("ap_symphony_ipc_init$$$$$$$$$$$$$$$\n");
    if (max_pipe_num <= 0)
    {
        IPC_PRINTF("%s: max pipe num error\n",__FUNCTION__);
        BUG();
    }

	/* clear  ENA state and INT state */
	REG32_WRITE(MCPU_MB_ENACLR, 0xFFFFFFFF);
	REG32_WRITE(MCPU_MB_INTCLR, 0xFFFFFFFF);

#ifdef IPC_HAVE_ACK
	REG32_WRITE(MCPU_MB_ACK, 0);
	IPC_PRINTF("ipc_init: has ACK.\n");
#else
	IPC_PRINTF("ipc_init: has no ACK.\n");
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
	ret = request_irq(IRQ_MAILBOX_ID, (irq_handler_t)ap_isr, IRQF_TRIGGER_RISING, DRV_NAME, NULL);
#else
	ret = request_irq(IRQ_MAILBOX_ID, (irq_handler_t)ap_isr, IRQF_TRIGGER_HIGH, DRV_NAME, NULL);
#endif

    g_ap_max_pipe_num = max_pipe_num;

    memset(p_pipe, 0, sizeof(pipe_info_t)*max_pipe_num);
    g_ap_ipc_struct_addr = (ulong)p_pipe;
}

static void ap_symphony_ipc_init(int max_pipe_num)
{
	_ap_symphony_ipc_init(max_pipe_num);

#if !defined(IPC_SECURE_PLUGIN)
	_ap_symphony_ipc_late_init();
#endif
}

//  ipc create in dest pipe   for example: ipc ap cpu ----->av cpu ,you must call av_ipc_pipe_create function in av cpu
static RET_CODE __ap_ipc_pipe_create(ipc_pipe_t *p_pipe_t,int pipe_depth)
{
    pipe_info_t* p_g_pipe;
    pipe_info_t *p_pipe = NULL;
    int index = -1;
	int i = 0;

    p_g_pipe = (pipe_info_t*)g_ap_ipc_struct_addr;

    for(i = 0; i < g_ap_max_pipe_num; i++){
        p_pipe = p_g_pipe + i;
        if (p_pipe->used == 0){
            index = i;
            p_pipe->used = 1;
            break;
        }
    }

    if(index == -1)
        return ERR_FAILURE;

    p_pipe->pipe_depth = (pipe_depth + 1);
    p_pipe = p_g_pipe + index;
#ifdef __KERNEL__
    p_pipe->Datapipe = (ipc_msg_t *)kmalloc((p_pipe->pipe_depth)*sizeof(ipc_msg_t),GFP_KERNEL);
#else
    p_pipe->Datapipe = (ipc_msg_t *)mtos_malloc((p_pipe->pipe_depth)*sizeof(ipc_msg_t));
#endif
    MT_ASSERT(NULL != p_pipe->Datapipe);
    memset(p_pipe->Datapipe, 0, (p_pipe->pipe_depth)*sizeof(ipc_msg_t));
    p_pipe->Datapipe = (ipc_msg_t *)(((ulong) p_pipe->Datapipe));
    p_pipe->ReadPt = p_pipe->WritePt = 0;
    memcpy((char *)&p_pipe->pipe_info,(char *)p_pipe_t,sizeof(ipc_pipe_t));

    return SUCCESS;
}


static RET_CODE ap_symphony_ipc_pipe_create(ipc_pipe_t *p_pipe_t,int pipe_depth)
{
    RET_CODE ret;

    if(!CPU_IS_AP)
        MT_ASSERT(0);

    if(g_ap_ipc_struct_addr == 0)
        return -1;    // ipc not init

    IPC_PRINTF("AP cpu create a pipe for 0x%x----->0x%x\n",p_pipe_t->p_sid,p_pipe_t->p_did);
#ifdef __KERNEL__
    ret =  __ap_ipc_pipe_create(p_pipe_t,pipe_depth) ;

 #else
    mtos_task_lock();
    ret =  __ap_ipc_pipe_create(p_pipe_t,pipe_depth) ;
    mtos_task_unlock();

 #endif

    if (ret == ERR_FAILURE)
        IPC_PRINTF("can not create ap_ipc_pipe_create \n");

    return  ret ;
}


static RET_CODE av_symphony_ipc_pipe_create(ipc_pipe_t *p_pipe_t,int pipe_depth)
{
    pipe_info_t* p_g_pipe;
    pipe_info_t *p_pipe = NULL;
    int index = -1;
	int i = 0;

    //if(CPU_IS_AP)
    //    MT_ASSERT(0);
       // MT_ASSERT1(0); //FIX ME  PIPE FULL OR NO THIS PIPE

    p_g_pipe = (pipe_info_t*)g_av_ipc_struct_addr;
    MT_INFO_LOG("AV cpu create a pipe for 0x%x----->0x%x\n",p_pipe_t->p_sid,p_pipe_t->p_did);

    for(i = 0; i < g_av_max_pipe_num; i++){
        p_pipe = p_g_pipe + i;
        if (p_pipe->used == 0){
            index = i;
            p_pipe->used = 1;
            break;
        }
    }

    if(index == -1)
        return ERR_FAILURE;

    p_pipe->pipe_depth = (pipe_depth + 1);
    p_pipe = p_g_pipe + index;
    p_pipe->Datapipe = &g_av_ipc_msg[i][0];
    p_pipe->Datapipe = (ipc_msg_t *)(((ulong) p_pipe->Datapipe));
    p_pipe->ReadPt = p_pipe->WritePt = 0;
    memcpy((char *)&p_pipe->pipe_info,(char *)p_pipe_t,sizeof(ipc_pipe_t));

    return SUCCESS;
}


static RET_CODE check_symphony_av_ready(void)
{
    if(symphony_check_cpu_ready(AV_CPU_ID)){
  //      IPC_PRINTF("####av cpu is ready for ######\n");
        return SUCCESS;
    }else{
        return ERR_FAILURE;
    }
}


static RET_CODE check_symphony_ap_ready(void)
{
    if(symphony_check_cpu_ready(AP_CPU_ID)){
   //     mtos_MT_INFO_LOG("####ap cpu is ready for ######\n");
        return SUCCESS;
    }else{
        return ERR_FAILURE;
    }
}


static void av_symphony_recv_down_ack(ipc_msg_t *p_msg)
{
   return;
}


static void ap_symphony_recv_down_ack(void)    //  run in ap cpu
{
   return;
}

#if 0
/*!
  * ap send to av cpu need av ack ,
  * must polling mailbox intr state,if state=0 for this transf is ok.
  *
  * ms: 0 - wait forever
  */
static int ap_symphony_wait_av_ack(unsigned int chan, unsigned int ms)
{
  u32 time_out = ms, index = 0;
  /*!
    * if set, that interrupt is not already cleared
    */
  //u32 tmp_flag = readl(SCPU_MB_INT_STATE) & (0x1 << chan);

  /*!
    * wait forever
    */
  if (0 == time_out)
  {
    while(readl((void *)SCPU_MB_INT_STATE) & (0x1 << chan))
      ;
  }
  else
  {
    while(readl((void *)SCPU_MB_INT_STATE) & (0x1 << chan))
    {
      udelay(1000);

      if(++index > time_out)
        return -ETIME;
    }
  }

  return 0;
}
#endif

//static u32 debug_18393 ;
static RET_CODE __ap_send_to_av(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag)    //  run in ap cpu
{
    int channel = -1;
	int i = 0;
	u32 time_out;
	mb_msg_t *p_mb_array = NULL;
	mb_msg_t *p_mb = NULL;
	phys_addr_t p_mb_phy_addr;
//no used
//	mb_msg_t *test;
	u32 state = REG32_READ(SCPU_MB_INT_STATE);
    u32 enable = REG32_READ(SCPU_MB_ENA_STATE);
	//u8 tmp_flag;
	//u32 counter = 0;
	u32 ret;

	//IPC_PRINTF("__ap_send_to_av!!!!!!!!!\n");

#if defined(IPC_SECURE_PLUGIN)
	_ap_symphony_ipc_late_init();
#endif

	p_mb_array = (mb_msg_t *)g_ap_mb_msg_addr;

	if(p_mb_array == NULL)
	{
		MT_FATAL_LOG("\n[AP][IPC]bomb!!!!!!!!\n\n");
        return ERR_FAILURE;
	}

	if(ack_flag)
	{
	    p_msg->msg_id = (p_msg->msg_id | 0x80000000);
	}

	/* scpu mailbox intr state must be determined first */
//no used
//	test = (mb_msg_t *)CKSEG1ADDR(p_mb);
    /*
    * if intr for that channel has still not been dealt by SCPU yet,
    * we could not send one more intr.
    */
    for(i = REE2AV_CHANNEL_FROM; i <= REE2AV_CHANNEL_TO; i++){
        if ((state & (0x1 << i)) == 0){
            channel = i;
            //IPC_PRINTF("channel is %d\n",channel);
            break;
        }
    }
    if(channel == -1){
        MT_ERR_LOG("[AP][IPC]__ap_send_to_av.. all channel state is 1\n");
        return ERR_FAILURE;
    }

	p_mb = &p_mb_array[channel];

	memcpy((char *)(&p_mb->p_msg),(char *)p_msg,sizeof(ipc_msg_t));
	p_mb->local_id = local_id;
	//hal_dcache_flush((void * )(p_mb), sizeof(mb_msg_t));
	mt_flush_data_cache_range((mt_u8*)(p_mb), sizeof(mb_msg_t));

#if defined(TESTCASE_IPC_APTOAV_MULTI_ASYNC_MSG) || defined(IPC_DEBUG_ENABLE)
	printk("[AP][IPC]%s: ch %d, mb %p, lid 0x%x, msg 0x%x\n",__FUNCTION__,
	    channel,
		p_mb,
		p_mb->local_id,
		p_mb->p_msg.msg_id);
#endif

    /* if intr for that channel in peer CPU is not enable, let's enable it */
    if (!(enable & (0x1 << channel)))
    {
        //IPC_PRINTF("\n __ap_send_to_av: set ENASET\n");
        //REG32_WRITE(SCPU_MB_ENASET, REG32_READ(SCPU_MB_ENASET) | (0x1 << channel));
        REG32_WRITE(SCPU_MB_ENASET, REG32_READ(SCPU_MB_ENASET) | (0x1 << channel));
    }

    /* tell peer CPU where the msg is */
    //REG32_WRITE((SCPU_MB_INFO_REG(channel)), (u32)p_mb);
#if defined(CONFIG_ARM) || defined(CONFIG_ARM64)
    //IPC_PRINTF("[AP][IPC]__ap_send_to_av: msg addr = %p\n",p_mb);
	//p_mb = (mb_msg_t *)__pa((ulong)p_mb);
	//p_mb_phy_addr = (phys_addr_t)__pa((ulong)p_mb);
	p_mb_phy_addr = (phys_addr_t)_PA_MSG((void*)p_mb);
    //IPC_PRINTF("[AP][IPC]__ap_send_to_av: physical msg addr = %p\n",p_mb);
#else
	/*MIPS*/
	p_mb_phy_addr = (phys_addr_t)p_mb;
#endif

	//
	// @@@@@@@@@@@@@@@@@@!!!!!!!!!!!!!!!!!!
	// <CNComment>×î¹Ø¼üµã
	//
    //REG32_WRITE(SCPU_MB_INFO_REG(channel), (ulong)p_mb);
    REG32_WRITE(SCPU_MB_INFO_REG(channel), (u32)p_mb_phy_addr);

	set_ack(channel);

    /* now, start */
    REG32_WRITE(SCPU_MB_INTSET, REG32_READ(SCPU_MB_INTSET) | (0x1 << channel));


    time_out = p_msg->time_out_ms;
    //MT_INFO_LOG("time out is %d\n",time_out);
#if 0
	if (1 == ack_flag)
	  ret = ap_symphony_wait_av_ack(channel, p_msg->time_out_ms);
	return ret;
#else

#if 0
    while(1){
        /*
        ap send to av cpu need av ack ,
        must polling mailbox intr state,if state=0 for this transf is ok.
        */
        state = REG32_READ(SCPU_MB_INT_STATE);
        tmp_flag = (ack_flag == 1) ? ((state & (0x1 << channel)) == 0):1;

        if(tmp_flag)
		{
            return SUCCESS;
		}
        else{
#ifdef __KERNEL__
			//mdelay(1);
			usleep_range(100, 1000);
#else
            mtos_task_delay_ms(1);

 #endif
            if (time_out != 0)
			{
                index++;
			}

			counter ++;
			if (counter % 5000 == 0)
			{
				MT_ERR_LOG("[AP][IPC]__ap_send_to_av.. %u\n",counter);
				//WARN_ON(1);
				//dump_stack();
			}

	        if(index > time_out)
			{
				MT_ERR_LOG("\n[AP][IPC]__ap_send_to_av.. timeout!\n");
                return ERR_TIMEOUT;
			}
        }
   }
#else
	if (ack_flag)
	{
		ret = wait_ack(channel, time_out);
		if (ret == 0)
		{
			MT_ERR_LOG("\n[AP][IPC]__ap_send_to_av.. timeout!\n");
			return ERR_TIMEOUT;
		}
	}

	return SUCCESS;
#endif
#endif

}


RET_CODE ap_symphony_send_to_av(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag)    //  run in ap cpu
{
    RET_CODE ret;

    //MT_INFO_LOG("ap_symphony_send_to_av pipeID[0x%x]msgID[0x%x]P1[0x%x]p2[0x%x]\n",
      //                      local_id,p_msg->msg_id,p_msg->param1,p_msg->param2);

#ifdef __KERNEL__
    ret = __ap_send_to_av(local_id, p_msg, ack_flag);
#else
    mtos_task_lock();
    ret = __ap_send_to_av(local_id, p_msg, ack_flag);
    mtos_task_unlock();
#endif
    //MT_INFO_LOG("ap_symphony_send_to_av pipeID[0x%x]msgID[0x%x]P1[0x%x]p2[0x%x]!!!!!!!!!!!!!!!!!\n",
    //                        local_id,p_msg->msg_id,p_msg->param1,p_msg->param2);

#ifdef TESTCASE_IPC_APTOAV_MULTI_ASYNC_MSG
	int i;
	ipc_msg_t testcase_msg;

	printk("[AP][IPC]TESTCASE_APTOAV_MULTI_ASYNC_MSG\n");
	memset(&testcase_msg, 0, sizeof(ipc_msg_t));

	for (i=0; i<32; i++)
	{
		testcase_msg.msg_id = i;
		__ap_send_to_av(local_id, &testcase_msg, 0);
	}
#endif

    return ret;
}


static RET_CODE av_symphony_send_to_ap(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag)
{
	u32 state = REG32_READ(MCPU_MB_INT_STATE);
    u32 enable = REG32_READ(MCPU_MB_ENA_STATE);
    int channel = -1;
	int i = 0;
	mb_msg_t *p_mb;
    if(CPU_IS_AP)
    {
#if defined (CONFIG_MIPS)
        MT_INFO_LOG("ebase 2 =0x%x\n", read_c0_ebase());
#endif
        MT_ASSERT(0);
       // MT_ASSERT1(0);
     }

    MT_INFO_LOG("av_symphony_send_to_ap pipeID[0x%x]msgID[0x%x]P1[0x%x]p2[0x%x]\n",
            local_id,p_msg->msg_id,p_msg->param1,p_msg->param2);

	p_mb = (mb_msg_t *)g_av_mb_msg_addr;
	memcpy((char *)(&p_mb->p_msg),(char *)p_msg,sizeof(ipc_msg_t));
	p_mb->local_id = local_id;

	//hal_dcache_flush((void * )(p_mb), sizeof(mb_msg_t));

    /* mcpu mailbox intr state must be determined first */

    /*
    * if intr for that channel has still not been dealt by MCPU yet,
    * we could not send one more intr.
    */

    for(i = 0; i < MAX_CHANNEL_NUMS; i++){
        if ((state & (0x1 << i)) == 0){
            channel = i;
            break;
        }
    }

    if(channel == -1){
        MT_INFO_LOG("\n av_symphony_send_to_ap.. all channel state is 1\n");
        return ERR_FAILURE;
    }

    /* if intr for that channel in peer CPU is not enable, let's enable it */
    if (!(enable & (0x1 << channel)))
    {
        MT_INFO_LOG("\n av_symphony_send_to_ap: set ENASET\n");
        REG32_WRITE(MCPU_MB_ENASET, REG32_READ(MCPU_MB_ENASET) | (0x1 << channel));
    }

    /* tell peer CPU where the msg is */
	REG32_WRITE(MCPU_MB_INFO_REG(channel), (ulong)p_mb);

    /* now, start */
	REG32_WRITE(MCPU_MB_INTSET, REG32_READ(MCPU_MB_INTSET) | (0x1 << channel));
#if 0
    int delay_cnt = 10000000;

    while(1){
        state = REG32_READ(MCPU_MB_INT_STATE);
        u8 tmp_flag = (ack_flag == 1) ? ((state & (0x1 << channel)) == 0):1;

        if(tmp_flag)
            return SUCCESS;
        else{
                if((--delay_cnt) == 0)
                {
                    delay_cnt = 10000000;
                    MT_INFO_LOG("\n\n   local_id 0x%x ack_flag 0x%x\n",local_id,ack_flag);
                    MT_INFO_LOG("ipc_msg ID[%x]parm[%x]parm2[%x]\n",p_msg->msg_id,p_msg->param1,p_msg->param2);
                    MT_INFO_LOG("#IPC suspend av_symphony_send_to_ap need ack\n");
                }
                ;
        }
    }
#endif
	return 0;
}


RET_CODE ap_symphony_recv_from_av(u32 local_id, ipc_msg_t *p_msg)
{
  if(!CPU_IS_AP)
      MT_ASSERT(0);
  return
    __ipc_msg_receive(local_id,p_msg, (pipe_info_t *)g_ap_ipc_struct_addr, g_ap_max_pipe_num);

}


static RET_CODE av_symphony_recv_from_ap(u32 local_id, ipc_msg_t *p_msg)
{
  if(CPU_IS_AP)
      MT_ASSERT(0);
     // MT_ASSERT1(0);
  return
   __ipc_msg_receive(local_id,p_msg, (pipe_info_t *)g_av_ipc_struct_addr, g_av_max_pipe_num);

}


static RET_CODE ap_symphony_send_to_ap(u32 local_id, ipc_msg_t *p_msg)
{
    if(!CPU_IS_AP)
        MT_ASSERT(0);

    return ERR_FAILURE;
}


static RET_CODE av_symphony_send_to_av(u32 local_id, ipc_msg_t *p_msg)
{
    if(CPU_IS_AP)
        MT_ASSERT(0);
       // MT_ASSERT1(0);

    return ERR_FAILURE;
}


static RET_CODE ap_symphony_recv_from_ap(u32 local_id, ipc_msg_t *p_msg)
{
    if(!CPU_IS_AP)
        MT_ASSERT(0);

    return ERR_FAILURE;
}


static RET_CODE av_symphony_recv_from_av(u32 local_id, ipc_msg_t *p_msg)
{
    if(CPU_IS_AP)
        MT_ASSERT(0);

    return ERR_FAILURE;

}


u32 attach_ipcfw_fun_set_symphony(ipc_fw_fun_set_t * p_funset)
{
  if (p_funset == NULL)
  {
  	MT_ERR_LOG("attach_ipcfw_fun_set_symphony error\n");
  	return ERR_FAILURE;
  }

  MT_INFO_LOG("enter attach_ipcfw_fun_set_symphony\n");
  p_funset->check_ap_ready_set=     check_symphony_ap_ready;

  p_funset->check_av_ready_set=     check_symphony_av_ready;

  p_funset->ap_ipc_init_set=       ap_symphony_ipc_init;

  p_funset->av_ipc_init_set=        av_symphony_ipc_init;

  p_funset->av_ipc_pipe_create_set= av_symphony_ipc_pipe_create;

  p_funset->ap_ipc_pipe_create_set= ap_symphony_ipc_pipe_create;

  p_funset->ap_send_to_av_set=      ap_symphony_send_to_av;

  p_funset->av_send_to_ap_set=      av_symphony_send_to_ap;

  p_funset->ap_recv_from_av_set=    ap_symphony_recv_from_av;

  p_funset->av_recv_from_ap_set=    av_symphony_recv_from_ap;

  p_funset->ap_send_to_ap_set=      ap_symphony_send_to_ap;

  p_funset->av_send_to_av_set=      av_symphony_send_to_av;

  p_funset->ap_recv_from_ap_set=    ap_symphony_recv_from_ap;

  p_funset->av_recv_from_av_set=    av_symphony_recv_from_av;

  p_funset->ap_recv_down_ack_set=   ap_symphony_recv_down_ack;

  p_funset->av_recv_down_ack_set=   av_symphony_recv_down_ack;

  p_funset->ap_ipc_init_set(32);

  return SUCCESS;
}

#ifdef __KERNEL__
EXPORT_SYMBOL(ap_symphony_send_to_av);
EXPORT_SYMBOL(ap_symphony_recv_from_av);
EXPORT_SYMBOL(attach_ipcfw_fun_set_symphony);

#endif

