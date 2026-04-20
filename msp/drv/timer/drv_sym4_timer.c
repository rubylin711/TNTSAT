/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage LZ Technology Co., Ltd.
 *
 * File Name      : drv_sym4_timer.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/10/22
 * Description    : Symphony4 HW Timer
 * History        :
 * 1.Date         : 2019/10/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/types.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/irq.h"

#include "mt_type.h"
#include "drv_timer.h"

#define TIMER_VERB(...)			do{}while(0)
//#define TIMER_VERB				printk
#define TIMER_ALWAYS(...)		do{}while(0)
//#define TIMER_ALWAYS			printk

#define MTIMER_CLK_HZ			1000000

/*
 * 8 at most.
 * NOTE:
 *  1. timer 7 is used by clockevent.
 *  2. timer 0-3 reserved for AVCPU.
 */
#define TIMER_RESERVED_AVCPU	1
#ifdef CONFIG_CLKEVT_SYMPHONY4_TIMER
#define TIMER_RESERVED			7
#endif
#define M_SYS_TIMER_MAX_NUM		8
#define M_SYS_TREG_MAX_NUM		3

#define REG_TIMER_BASE			(SYMPHONY_TIME_WD_REGISTER_VIRT_BASE + 0x80)

#define REG_TIMER_INIT(id)		(REG_TIMER_BASE + (id) * 0x100)
#define REG_TIMER_CAP(id)		(REG_TIMER_BASE + (id) * 0x100 + 4)
#define REG_TIMER_CW(id)		(REG_TIMER_BASE + (id) * 0x100 + 8)
#ifdef CONFIG_MT_CHIP_SYMPHONY6
#define REG_TIMER_MODE(id)		(REG_TIMER_BASE + (id) * 0x100 + 0x10)

#define DEF_TM_MODE				0x1
#endif

#define REG_TIMER(id,rn)		(REG_TIMER_BASE + (id) * 0x100 + (rn)*4)

#define CHECK_TIMER_ID(id)		((id) >= 0 && (id) < M_SYS_TIMER_MAX_NUM)

//---------------------------------------------------------------------------//

enum timer_switch_e
{
	TIMER_OFF 			= 0,
	TIMER_ON 			= 1,
};

enum timer_cycle_e
{
	TIMER_CYCLED 		= 0,
	TIMER_ONCE 			= 1,
};

enum timer_cascade_e
{
	TIMER_NON_CASCADE 	= 0,
	TIMER_CASCADE 		= 1,
};

static DEFINE_SPINLOCK(drv_timer_lock);

static mt_symp_timer_info s_mt_timer_info[M_SYS_TIMER_MAX_NUM];

static u32 mt_timer_reg_backup[M_SYS_TIMER_MAX_NUM][M_SYS_TREG_MAX_NUM];


extern irqreturn_t mtimer_hsr(int irq, void *para);

/*
 * set timer counter threshold
 * threshold=计数周期减1
 */
static void drv_timer_init(mt_u32 id, mt_u32 threshold)
{
	HAL_PUT_U32((volatile u32 *)REG_TIMER_INIT(id), (u32)threshold);
}

/* get timer counter */
static mt_u32 drv_timer_get_cap(mt_u32 id)
{
	return (mt_u32)HAL_GET_U32((volatile u32 *)REG_TIMER_CAP(id));
}

/* switch timer on/off */
static void drv_timer_switch(mt_u32 id, enum timer_switch_e on_off)
{
	mt_u32 val;

	val = (mt_u32)HAL_GET_U32((volatile u32 *)REG_TIMER_CW(id));

	if (on_off == TIMER_ON)
	{
		val |= (0x01 << 0);
	}
	else
	{
		val &= ~(0x01 << 0);
	}

	HAL_PUT_U32((volatile u32 *)REG_TIMER_CW(id), (u32)val);
}

/* set timer cycled or once */
static void drv_timer_set_cycle(mt_u32 id, enum timer_cycle_e cycle)
{
	mt_u32 val;

	val = (mt_u32)HAL_GET_U32((volatile u32 *)REG_TIMER_CW(id));

	if (cycle == TIMER_ONCE)
	{
		val |= (0x01 << 1);
	}
	else
	{
		val &= ~(0x01 << 1);
	}

	HAL_PUT_U32((volatile u32 *)REG_TIMER_CW(id), (u32)val);
}

/*
 * set timer 0/1, 2/3, 4/5, 6/7 cascaded
 * id: 1/3/5/7
 */
static void drv_timer_set_cascade(mt_u32 id, enum timer_cascade_e cascade)
{
	mt_u32 val;

	if (id != 1 && id != 3 && id != 5 && id != 7)
	{
		BUG();
		return;
	}

	val = (mt_u32)HAL_GET_U32((volatile u32 *)REG_TIMER_CW(id));

	if (cascade == TIMER_CASCADE)
	{
		val |= (0x01 << 16);
	}
	else
	{
		val &= ~(0x01 << 16);
	}

	HAL_PUT_U32((volatile u32 *)REG_TIMER_CW(id), (u32)val);
}

static void drv_timer_clear_intr(mt_u32 id)
{
	mt_u32 val;

	val = (mt_u32)HAL_GET_U32((volatile u32 *)REG_TIMER_CW(id));

	val |= (0x01 << 3);

	HAL_PUT_U32((volatile u32 *)REG_TIMER_CW(id), (u32)val);
}

#ifdef CONFIG_MT_CHIP_SYMPHONY6
static void drv_timer_set_mode(mt_u32 id, mt_u32 mode)
{
	HAL_PUT_U32((volatile u32 *)REG_TIMER_MODE(id), (u32)mode);
}
#endif

static mt_u64 ms_to_cycle(int ms)
{
    return (mt_u64)((mt_u64)MTIMER_CLK_HZ / 1000 * ms);
}

static mt_s32 allocate_timer(MT_BOOL cascade,
							int ms, timer_fn pfn, void *para, int cycled, char *name)
{
    int i;

    for (i=0; i<M_SYS_TIMER_MAX_NUM; i++)
    {
        if (!s_mt_timer_info[i].used)
        {
            break;
        }
    }

    if (cascade)
    {
        if((s_mt_timer_info[0].used || s_mt_timer_info[1].used)
            && (s_mt_timer_info[2].used || s_mt_timer_info[3].used)
            && (s_mt_timer_info[4].used || s_mt_timer_info[5].used))
        {
            MT_ERR_TIMER(KERN_ERR "Error, no hw timer available.\n");
            return (-1);
        }

        if(!(s_mt_timer_info[0].used || s_mt_timer_info[1].used))
            i = 1;
        else if(!(s_mt_timer_info[2].used || s_mt_timer_info[3].used))
            i = 3;
        else
            i = 5;
    }

    if (i >= M_SYS_TIMER_MAX_NUM)
    {
		MT_ERR_TIMER(KERN_ERR "Error, no hw timer available.\n");
        BUG();
        return -1;
    }

    s_mt_timer_info[i].used 		 = 1;
    if (cascade)
    {
		s_mt_timer_info[i].cascade	 = 1;
		s_mt_timer_info[i-1].used 	 = 1;
		s_mt_timer_info[i-1].cascade = 1;
	}
	else
	{
		s_mt_timer_info[i].cascade	 = 0;
	}
    s_mt_timer_info[i].p_fn 		 = pfn;
    s_mt_timer_info[i].p_para 		 = para;
    s_mt_timer_info[i].cycled 		 = cycled;
    s_mt_timer_info[i].name 		 = name;
    s_mt_timer_info[i].timeout 		 = ms;

	TIMER_ALWAYS("%s: %s(%d) - ms %d, cycled %d, cascaded %d\n",__FUNCTION__,
		name==NULL?"null":name,
		i,ms,cycled,cascade);

    return i;
}

static int is_timer_used(mt_u32 id)
{
	return s_mt_timer_info[id].used;
}

static void free_timer(mt_u32 id)
{
	TIMER_ALWAYS("%s: %u\n",__FUNCTION__,id);

    s_mt_timer_info[id].used    = 0;
    s_mt_timer_info[id].p_fn    = NULL;
    s_mt_timer_info[id].p_para  = NULL;
    s_mt_timer_info[id].cycled  = 0;
    s_mt_timer_info[id].timeout = 0;
    s_mt_timer_info[id].name    = NULL;

	if (s_mt_timer_info[id].cascade && id > 0)
	{
		s_mt_timer_info[id-1].used    = 0;
		s_mt_timer_info[id-1].cascade = 0;
	}

	s_mt_timer_info[id].cascade	= 0;
}

static inline unsigned int id_to_irq(mt_u32 id)
{
	return (id < 4)?(IRQ_T0_ID + id):(IRQ_T4_ID + id - 4);
}

static inline int irq_to_id(int irq)
{
	return (irq <= IRQ_T3_ID)?(irq - IRQ_T0_ID):(irq - IRQ_T4_ID + 4);
}

static irqreturn_t drv_timer_isr(int irq, void *dev_id)
{
    mt_symp_timer_info *p_info;
    int id = irq_to_id(irq);

	TIMER_VERB("%s: %u\n",__FUNCTION__,id);

	if (CHECK_TIMER_ID(id))
	{
		drv_timer_clear_intr((mt_u32)id);
	}

	if (dev_id != NULL)
	{
		p_info = (mt_symp_timer_info *)dev_id;
		if (p_info->p_fn)
		{
			p_info->p_fn(p_info->p_para);
		}
		else  //p_fn is null, for kernel send SIGIO to user_layer 
		{
			mtimer_hsr(irq, dev_id);
		}
	}


	return IRQ_HANDLED;
}

//---------------------------------------------------------------------------//

mt_s32 mt_drv_timer_request(int ms, timer_fn pfn, void *para, int cycled, char *name)
{
	mt_u64 cycle_cnt_64;
	mt_u32 threshold_hi;
	mt_u32 threshold_low;
       MT_BOOL cascade = MT_FALSE;
       mt_s32 timer_id = -1;
       int err;
       unsigned long flags;

	if (ms <= 0)
	{
		//BUG();
		MT_ERR_TIMER("mt_drv_timer_request Parameter Error ms  =%d !\n",ms);
		return MT_FAILURE;
	}

/*
	if (pfn == NULL)
	{
		BUG();
	}
*/

       spin_lock_irqsave(&drv_timer_lock, flags);
	cycle_cnt_64 = ms_to_cycle(ms);
	/* 定时器计数阈值 = 计数周期 - 1 */
	cycle_cnt_64 --;
	threshold_hi = cycle_cnt_64 >> 32;
	threshold_low = cycle_cnt_64 & 0xFFFFFFFF;
	if (threshold_hi != 0)
	{
		cascade = MT_TRUE;
	}

	if (cascade)
	{
		timer_id = allocate_timer(cascade, ms, pfn, para, cycled, name);

		if (CHECK_TIMER_ID(timer_id))
		{
			//low32
			drv_timer_init((mt_u32)(timer_id-1), threshold_low);
			//high32
			drv_timer_init((mt_u32)timer_id, threshold_hi);

			drv_timer_set_cycle((mt_u32)timer_id, cycled?TIMER_CYCLED:TIMER_ONCE);

			drv_timer_set_cascade((mt_u32)timer_id, TIMER_CASCADE);

#ifdef CONFIG_MT_CHIP_SYMPHONY6
			drv_timer_set_mode((mt_u32)timer_id, DEF_TM_MODE);
#endif
			drv_timer_switch((mt_u32)timer_id, TIMER_ON);

			err = request_irq(id_to_irq((mt_u32)timer_id),
							drv_timer_isr,
							IRQF_TRIGGER_HIGH | IRQF_SHARED | IRQF_TIMER | IRQF_IRQPOLL,
							"mtimer",
							(void*)&s_mt_timer_info[timer_id]);

			if (err != 0)
			{
				MT_ERR_TIMER("request timer(%d) irq failed!\n",timer_id);
			}
		}
		else
		{
			int i=0;
			for (i=0; i<M_SYS_TIMER_MAX_NUM; i++)
    			{
	 			printk(" s_mt_timer_info[%d].used=%d\n",i,s_mt_timer_info[i].used);
    			}
			printk("BUG at %s:%d/%s()   --- CHECK_TIMER_ID Fail  timerid = %d !\n", 
				__FILE__, __LINE__, __func__,timer_id);
		}
	}
	else
	{
		timer_id = allocate_timer(cascade, ms, pfn, para, cycled, name);
		if (CHECK_TIMER_ID(timer_id))
		{
			drv_timer_init((mt_u32)timer_id, threshold_low);
			drv_timer_set_cycle((mt_u32)timer_id, cycled?TIMER_CYCLED:TIMER_ONCE);

			if (timer_id == 1 || timer_id == 3 || timer_id == 5 || timer_id == 7)
			{
				drv_timer_set_cascade((mt_u32)timer_id, TIMER_NON_CASCADE);
			}

#ifdef CONFIG_MT_CHIP_SYMPHONY6
			drv_timer_set_mode((mt_u32)timer_id, DEF_TM_MODE);
#endif

			drv_timer_switch((mt_u32)timer_id, TIMER_ON);

			err = request_irq(id_to_irq((mt_u32)timer_id),
							drv_timer_isr,
							IRQF_TRIGGER_HIGH | IRQF_SHARED | IRQF_TIMER | IRQF_IRQPOLL,
							"mtimer",
							(void*)&s_mt_timer_info[timer_id]);
			if (err != 0)
			{
				MT_ERR_TIMER("request timer(%d) irq failed!\n",timer_id);
			}
		}
		else
		{
			int i=0;
			for (i=0; i<M_SYS_TIMER_MAX_NUM; i++)
    			{
	 			printk(" s_mt_timer_info[%d].used=%d\n",i,s_mt_timer_info[i].used);
    			}
			printk("BUG at %s:%d/%s()   --- CHECK_TIMER_ID Fail  timerid = %d !\n", 
				__FILE__, __LINE__, __func__,timer_id);
		}
	}

       spin_unlock_irqrestore(&drv_timer_lock, flags);

	return timer_id;
}
EXPORT_SYMBOL(mt_drv_timer_request);

mt_s32 mt_drv_timer_release(int id)
{
    unsigned long flags;

	if (CHECK_TIMER_ID(id))
	{
		spin_lock_irqsave(&drv_timer_lock, flags);

		/* FIX: Bug 25160, app might re-entry */
		if (is_timer_used((mt_u32)id))
		{
			drv_timer_switch((mt_u32)id, TIMER_OFF);

			free_irq(id_to_irq((mt_u32)id), (void*)&s_mt_timer_info[id]);

			free_timer((mt_u32)id);
		}
		else
		{
			//error! timer already released!!!
		}

		spin_unlock_irqrestore(&drv_timer_lock, flags);

		return MT_SUCCESS;
	}

	/* FIX: Bug 25160, app might re-entry */
	//BUG();
	return MT_FAILURE;
}
EXPORT_SYMBOL(mt_drv_timer_release);

mt_u32 mt_drv_timer_read_cnt(int id)
{
	if (CHECK_TIMER_ID(id))
    {
    	//FIXME: how about cascade Timer?
        return drv_timer_get_cap((mt_u32)id);
    }

	BUG();
	return 0;
}
EXPORT_SYMBOL(mt_drv_timer_read_cnt);


void mt_drv_timer_reg_suspend(void)
{
	int i,j;
	/*time0-3 is avcpu use */
	for(i=4;i<M_SYS_TIMER_MAX_NUM;i++)
	{
		for(j=0;j<M_SYS_TREG_MAX_NUM;j++)
		{
			mt_timer_reg_backup[i][j]=HAL_GET_U32((volatile u32 *)REG_TIMER(i,j));
			TIMER_VERB("i:%d,j:%d,val:0x%x\n",i,j,mt_timer_reg_backup[i][j]);
		}
	}
}


void mt_drv_timer_reg_resume(void)
{
	int i,j;
	/*time0-3 is avcpu use */
	for(i=4;i<M_SYS_TIMER_MAX_NUM;i++)
	{
		for(j=0;j<M_SYS_TREG_MAX_NUM;j++)
		{
			if(j==1)
				continue;
			TIMER_VERB("before i:%d,j:%d,val:0x%x\n",i,j,HAL_GET_U32((volatile u32 *)REG_TIMER(i,j)));
			HAL_PUT_U32((volatile u32 *)REG_TIMER(i,j), mt_timer_reg_backup[i][j]);
			TIMER_VERB("i:%d,j:%d,val:0x%x\n",i,j,HAL_GET_U32((volatile u32 *)REG_TIMER(i,j)));
		}
	}
}


void mt_drv_timer_init(void)
{
	int i;

    memset(&s_mt_timer_info, 0, sizeof(s_mt_timer_info));

	memset(&mt_timer_reg_backup, 0, sizeof(mt_timer_reg_backup));

#if (TIMER_RESERVED_AVCPU)
	/* reserved for AVCPU */
    for (i=0; i<4; i++)
    {
    	s_mt_timer_info[i].used = 2;
    }
#endif

#ifdef TIMER_RESERVED
	/* reserved for clockevent */
	s_mt_timer_info[TIMER_RESERVED].used = 3;
#endif
}

void mt_drv_timer_dump_state(void)
{
	int i;

	TIMER_ALWAYS("=================== dump timer state ===================\n");
	for (i=0; i<M_SYS_TIMER_MAX_NUM; i++)
	{
		TIMER_ALWAYS("Timer[%d]:\n",i);
		TIMER_ALWAYS("\t    used: %d\n",s_mt_timer_info[i].used);
		if (s_mt_timer_info[i].used == 1)
		{
			TIMER_ALWAYS("\t    name: %s\n",s_mt_timer_info[i].name==NULL?"null":s_mt_timer_info[i].name);
			TIMER_ALWAYS("\t      to: %d(ms)\n",s_mt_timer_info[i].timeout);
			TIMER_ALWAYS("\t  cycled: %d\n",s_mt_timer_info[i].cycled);
			TIMER_ALWAYS("\tcascaded: %d\n",s_mt_timer_info[i].cascade);
			TIMER_ALWAYS("\t      fn: %p\n",s_mt_timer_info[i].p_fn);
			TIMER_ALWAYS("\t   param: %p\n",s_mt_timer_info[i].p_para);
		}
	}
	TIMER_ALWAYS("========================================================\n");
}

//---------------------------------------------------------------------------//

static void testcase_timer_cb(void *param)
{
	if (param != NULL)
	{
		TIMER_ALWAYS("\n==> %s: id %d\n",__FUNCTION__,*(int*)param);
	}
}

void testcase_timer_non_cascade(void)
{
	int i;
	mt_s32 ret;
	static int timer_id[3];
	int cycle_ms[3] = {100, 1000, 30000};

	TIMER_ALWAYS("===== %s =====\n",__FUNCTION__);

	for (i=0; i<3; i++)
	{
		timer_id[i] = mt_drv_timer_request(cycle_ms[i], testcase_timer_cb, (void*)&timer_id[i], 1, "test timer");

		BUG_ON(timer_id[i] < 0);
	}

	mt_drv_timer_dump_state();

	TIMER_ALWAYS("%s: sleep 63s...\n",__FUNCTION__);
	msleep(63000);	//63s

	for (i=0; i<3; i++)
	{
		ret = mt_drv_timer_release(timer_id[i]);

		BUG_ON(ret != MT_SUCCESS);
	}

	mt_drv_timer_dump_state();

	TIMER_ALWAYS("%s: end.\n",__FUNCTION__);
}

void testcase_timer_cascade(void)
{
	static int timer_id;

	TIMER_ALWAYS("===== %s =====\n",__FUNCTION__);

	/* 72分钟 */
	timer_id = mt_drv_timer_request(4294969/*>4294967.295*/,
									testcase_timer_cb,
									(void*)&timer_id,
									1,
									"test cascade timer");

	BUG_ON(timer_id < 0);

	mt_drv_timer_dump_state();

	TIMER_ALWAYS("%s: pls wait 72min to check log of testcase_timer_cb...\n",__FUNCTION__);
}

