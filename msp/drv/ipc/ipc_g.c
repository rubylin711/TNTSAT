/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
//
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
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
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/kobject.h>

#include <linux/sysfs.h>

#include "mt_type.h"
#include "sys_define.h"
#include "ipc.h"
#include "ipc_common.h"
//ipc client & test case
#include "drv_test_ipc.c"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_kernel_adapt.h"

//#define TEST_AUDIO_VIDEO

typedef struct
{
    unsigned int  vdec_fw_cmd;
    unsigned int  vdec_chan_id;
    unsigned int  vdec_control_cmd;
    unsigned int  vdec_args_addr;
    unsigned int  reserved[4];
}FW_IPC_ARGS_T;

typedef enum{
	AP_VDEC_FW_CMD_INIT = 1,
	AP_VDEC_FW_CMD_CONTROL,
	AP_VDEC_FW_CMD_EXIT,
	AP_VDEC_FW_CMD_MAX,
}AP_VDEC_FW_CMD_T;

static struct semaphore ipc_sem;

//MT_DECLARE_MUTEX(ipc_sem);

ipc_fw_fun_set_t g_ipcfw_f;

//Bug 111156: down_interruptible() return -EINTR!
static inline int DOWN_SEM(struct semaphore *sem, int line)
{
	int ret = 0;

	do {
		//ret = down_interruptible(sem);
		ret = down_killable(sem);
		if (ret == 0) {
			break;
		} else {
			printk("[IPC-%d]Semaphore is not acquired try again!\n",line);
			continue;
		}
	} while (1);

	return ret;
}

RET_CODE check_ap_ready()
{
   return g_ipcfw_f.check_ap_ready_set();
}

RET_CODE check_av_ready()
{
   return g_ipcfw_f.check_av_ready_set();
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%IPC--API%%%%%%%%%%%%%%%%%%%/
void ap_ipc_init(int max_pipe_num)
{
	int res;
	MT_INFO_LOG("ap_ipc_init!!!!!!!!!!!!!!!!!!!\n\n");
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_INFO_LOG("down sem return %d\n",res);
	}
	g_ipcfw_f.ap_ipc_init_set(max_pipe_num);
	up(&ipc_sem);
}

RET_CODE ap_ipc_pipe_create(ipc_pipe_t *p_pipe_t,int pipe_depth)
{
	RET_CODE ret;
	int res;
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_INFO_LOG("down sem return %d\n",res);
	}
	ret = g_ipcfw_f.ap_ipc_pipe_create_set(p_pipe_t, pipe_depth);
	up(&ipc_sem);
	return ret;
}

/*
 this fun can only run in task context
*/
RET_CODE ap_send_to_av(u32 local_id, ipc_msg_t *p_msg, u8 ack_flag)    //  run in ap cpu
{
	RET_CODE ret;
	int res;
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_ERR_LOG("down sem return %d\n",res);
		BUG();
	}
	ret = g_ipcfw_f.ap_send_to_av_set(local_id, p_msg, ack_flag);
	up(&ipc_sem);
   	return ret;
}

void ap_recv_down_ack()    //  run in ap cpu
{
	//RET_CODE ret;
	int res;
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_INFO_LOG("down sem return %d\n",res);
	}
	g_ipcfw_f.ap_recv_down_ack_set();
	up(&ipc_sem);
	return;
}



RET_CODE ap_recv_from_av(u32 local_id, ipc_msg_t *p_msg)
{
	RET_CODE ret;
	int res;
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_INFO_LOG("down sem return %d\n",res);
	}
	ret = g_ipcfw_f.ap_recv_from_av_set(local_id, p_msg);
	up(&ipc_sem);

	return ret;
}


RET_CODE ap_send_to_ap(u32 local_id, ipc_msg_t *p_msg)
{
	RET_CODE ret;
	int res;
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_INFO_LOG("down sem return %d\n",res);
	}
	ret = g_ipcfw_f.ap_send_to_ap_set(local_id, p_msg);
	up(&ipc_sem);

	return ret;
}


RET_CODE ap_recv_from_ap(u32 local_id, ipc_msg_t *p_msg)
{
	RET_CODE ret;
	int res;
	//res = down_interruptible(&ipc_sem);
	res = DOWN_SEM(&ipc_sem, __LINE__);
	if (res != 0)
	{
		MT_INFO_LOG("down sem return %d\n",res);
	}
	ret = g_ipcfw_f.ap_recv_from_ap_set(local_id, p_msg);
	up(&ipc_sem);
	return ret;
}

/*
 * Some module such as AO's irq not work in AP CPU,
 * but work in AV CPU.
 * so need use IPC to transfer AV CPU's irq to AP CPU's IPC irq.
 */
#if 1
#define MAX_IRQ_COUNT			128
DEFINE_SPINLOCK(kIpcIrqLock);
typedef void (*irq_handler_fn)(int irq, void *dev_id, struct pt_regs *regs);
static irq_handler_fn g_ipc_irq_handler[MAX_IRQ_COUNT] = {0};

int ipc_request_av_irq(unsigned int irq,
						void (*handler)(int irq, void *dev_id, struct pt_regs *regs),
						unsigned long irqflags,
						const char * devname,
						void *dev_id)
{
	unsigned long flags;

	MT_INFO_LOG("irq=%u, handler=%p\n",irq,handler);

	spin_lock_irqsave(&kIpcIrqLock, flags);
	if (irq >= 0 && irq < MAX_IRQ_COUNT)
	{
		if (g_ipc_irq_handler[irq] != NULL
			&& g_ipc_irq_handler[irq] != handler)
		{
			//Warning
		}

		g_ipc_irq_handler[irq] = handler;
	}
	spin_unlock_irqrestore(&kIpcIrqLock, flags);

	return 0;
}

void ipc_free_av_irq(unsigned int irq,void *dev_id)
{
	unsigned long flags;

	MT_INFO_LOG("irq=%u\n",irq);

	spin_lock_irqsave(&kIpcIrqLock, flags);
	if (irq >= 0 && irq < MAX_IRQ_COUNT)
	{
		g_ipc_irq_handler[irq] = NULL;
	}
	spin_unlock_irqrestore(&kIpcIrqLock, flags);
}

void *ipc_get_irq_handler(unsigned int irq)
{
	if (irq >= 0 && irq < MAX_IRQ_COUNT)
	{
		return (void*)g_ipc_irq_handler[irq];
	}
	else
	{
		return NULL;
	}
}

#ifdef __KERNEL__
EXPORT_SYMBOL(ipc_request_av_irq);
EXPORT_SYMBOL(ipc_free_av_irq);
#endif

#endif

#ifdef __KERNEL__
EXPORT_SYMBOL(ap_send_to_av);
EXPORT_SYMBOL(ap_recv_from_av);
EXPORT_SYMBOL(ap_recv_down_ack);
EXPORT_SYMBOL(ap_ipc_pipe_create);
EXPORT_SYMBOL(g_ipcfw_f);
EXPORT_SYMBOL(ap_ipc_init);
#endif


extern u32 attach_ipcfw_fun_set_symphony(ipc_fw_fun_set_t * p_funset);

#ifdef TEST_AUDIO_VIDEO


#define CPU_BASE_ID_AP 0x10000000
#define CPU_BASE_ID_AV 0x20000000

#define IPC_MSG_AUD_FW_MSG_DEPTH 8
#define IPC_MSG_VDEC_FW_MSG_DEPTH 8
#define SYS_LAYER_DEV       0x04
#define SYS_DEV_TYPE_AUDIO ((SYS_LAYER_DEV << 8) + 0x07)
/*!
   Video decoder device type
  */
#define SYS_DEV_TYPE_VDEC_VSB ((SYS_LAYER_DEV << 8) + 0x13)
#define AP_SYS_DEV_AUD (CPU_BASE_ID_AP + SYS_DEV_TYPE_AUDIO)
#define AV_SYS_DEV_AUD (CPU_BASE_ID_AV + SYS_DEV_TYPE_AUDIO)
/*!
//example for SRC&DST_ID
//src_id of vdec dev on ap cpu is
  */
#define AP_SYS_DEV_VDEC (CPU_BASE_ID_AP + SYS_DEV_TYPE_VDEC_VSB)
/*!
//example for SRC&DST_ID
//dst_id of vdec dev on av cpu is
  */
#define AV_SYS_DEV_VDEC (CPU_BASE_ID_AV +  SYS_DEV_TYPE_VDEC_VSB)

#define AUD_FW_CMD_ATTACH 0
#define IPC_MSG_AUD_FW_CMD_ATTACH        ((AUD_FW_CMD_ATTACH << 16) + \
                                              SYS_DEV_TYPE_AUDIO)
/*!
//example for SRC&DST_ID
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_START ((VDEC_FW_CMD_START << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_STOP ((VDEC_FW_CMD_STOP << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_PAUSE ((VDEC_FW_CMD_PAUSE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_RESUME ((VDEC_FW_CMD_RESUME << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_FREEZE ((VDEC_FW_CMD_FREEZE << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_INIT ((VDEC_FW_CMD_INIT << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_CONTROL ((VDEC_FW_CMD_CONTROL << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)
/*!
//dst_id of vdec dev on av cpu is
  */
#define IPC_MSG_VDEC_FW_CMD_EXIT ((VDEC_FW_CMD_EXIT << 16) + \
                                      SYS_DEV_TYPE_VDEC_VSB)

static void test_audio_ipc_cmd(void)
{
  ipc_pipe_t pipe_recv;
  ipc_msg_t msg_send;
  RET_CODE ret;
  MT_INFO_LOG("test_audio_ipc_cmd start\n");

  pipe_recv.msg_id_mask = 0xFFFFFFFF;

  pipe_recv.p_sid = AV_SYS_DEV_AUD;
  pipe_recv.p_did = AP_SYS_DEV_AUD;

  //create pipe for msg send/recv
  ret = ap_ipc_pipe_create(&pipe_recv, IPC_MSG_AUD_FW_MSG_DEPTH);
  MT_INFO_LOG("test_audio_ipc_cmd create ret %d\n",ret);

  mdelay(10);

  MT_INFO_LOG("test_audio_ipc_cmd send attach cmd\n");
  //send msg to fw for start decode
  msg_send.msg_id = IPC_MSG_AUD_FW_CMD_ATTACH;
  msg_send.param1 = 0;//type
  msg_send.param2 = 0;
  msg_send.time_out_ms = 10000;

  ret = ap_send_to_av(AP_SYS_DEV_AUD, &msg_send, 1);

  MT_INFO_LOG("test_audio_ipc_cmd attach ret %d\n",ret);
  if(ret != SUCCESS)
  {
    //MT_INFO_LOG("timeout ap_send_to_av ret%d\n",ret);
    return;
  }



  return;
}
static void test_video_ipc_cmd(void)
{
  ipc_pipe_t pipe_recv;
  ipc_msg_t msg_send;
  RET_CODE ret;
  MT_INFO_LOG("test_video_ipc_cmd start\n");

  pipe_recv.msg_id_mask = 0xFFFFFFFF;

  pipe_recv.p_sid = AV_SYS_DEV_VDEC;
  pipe_recv.p_did = AP_SYS_DEV_VDEC;

  //create pipe for msg send/recv
  ret = ap_ipc_pipe_create(&pipe_recv, IPC_MSG_VDEC_FW_MSG_DEPTH);
  MT_INFO_LOG("test_video_ipc_cmd create ret %d\n",ret);

  mdelay(10);

  MT_INFO_LOG("test_video_ipc_cmd send test cmd\n");
  //send msg to fw for start decode
  msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_START;
  msg_send.param1 = 0;//type
  msg_send.param2 = 0;
  msg_send.time_out_ms = 10000;

  ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);

  MT_INFO_LOG("test_video_ipc_cmd test ret %d\n",ret);
  //send msg to fw for start decode
  msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_INIT;
  msg_send.param1 = 0;//type
  msg_send.param2 = 0;
  msg_send.time_out_ms = 10000;

  ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);

  MT_INFO_LOG("test_video_ipc_cmd test ret %d\n",ret);
  //send msg to fw for start decode
  msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_CONTROL;
  msg_send.param1 = 0;//type
  msg_send.param2 = 0;
  msg_send.time_out_ms = 10000;

  ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);

  MT_INFO_LOG("test_video_ipc_cmd test ret %d\n",ret);
  //send msg to fw for start decode
  msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_EXIT;
  msg_send.param1 = 0;//type
  msg_send.param2 = 0;
  msg_send.time_out_ms = 10000;

  ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);

  MT_INFO_LOG("test_video_ipc_cmd test ret %d\n",ret);
  if(ret != SUCCESS)
  {
    //MT_INFO_LOG("timeout ap_send_to_av ret%d\n",ret);
    return;
  }



  return;
}


#endif

extern u32 symphony_set_local_cpu_ready(void);

int ipc_modinit(void)
{
    MT_INFO_LOG("ipc_init start \n");
    attach_ipcfw_fun_set_symphony(&g_ipcfw_f);
    sema_init(&ipc_sem, 1);

	symphony_set_local_cpu_ready();

#ifdef TEST_AUDIO_VIDEO

    mdelay(10);

   	test_audio_ipc_cmd();

	if(0)
	test_video_ipc_cmd();
    //drv_test_ipc();
#endif

    return 0;
}

void ipc_cleanup(void)
{
	return;
}

int fw_ipc_ap_send_to_av(void *para)
{
    FW_IPC_ARGS_T *p_ipc_info = (FW_IPC_ARGS_T *)para;
    ipc_pipe_t pipe_recv;
    ipc_msg_t msg_send;
    //int i;
    RET_CODE ret = SUCCESS;
    //return ret;

    pipe_recv.msg_id_mask = 0xFFFFFFFF;

    pipe_recv.p_sid = AV_SYS_DEV_VDEC;
    pipe_recv.p_did = AP_SYS_DEV_VDEC;

    if(!p_ipc_info)
    {
        MT_ERR_LOG("fw_ipc_err\n");
        return -1;
    }
    switch(p_ipc_info->vdec_fw_cmd)
    {
        case AP_VDEC_FW_CMD_INIT:
            msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_INIT;
            msg_send.param1 = (ulong)p_ipc_info;//info addr
            msg_send.param2 = 0;
            msg_send.time_out_ms = 10000;
            ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);
            break;
        case AP_VDEC_FW_CMD_CONTROL:
            msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_CONTROL;
            msg_send.param1 = (ulong)p_ipc_info;//info addr
            msg_send.param2 = 0;
            msg_send.time_out_ms = 10000;
            ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);
            break;
        case AP_VDEC_FW_CMD_EXIT:
            msg_send.msg_id = IPC_MSG_VDEC_FW_CMD_EXIT;
            msg_send.param1 = (ulong)p_ipc_info;//info addr
            msg_send.param2 = 0;
            msg_send.time_out_ms = 10000;
            ret = ap_send_to_av(AP_SYS_DEV_VDEC, &msg_send, 1);
            break;
        default:
            break;

    }
    return ret;
}
EXPORT_SYMBOL(fw_ipc_ap_send_to_av);

