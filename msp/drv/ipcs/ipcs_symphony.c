/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
//
#include <linux/kernel.h>   /* printk() */
#include <linux/slab.h>   /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include <linux/proc_fs.h>

#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/mm.h>
#include <linux/types.h>

:#include "mach/symphony_reg_base_addr.h"
#include "mt_type.h"
#include "ipcs_symphony.h"
#include "sys_define.h"
#include "mt_module_debug.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

/*
 * global parameters
 */
typedef struct _mbx_priv_struct
{
    struct semaphore sem[MAX_CHANNEL_INDEX];
} mbx_priv;

static mbx_priv g_mbx_priv;

#define MAX_PIPE_DEPTH 2
static pipe_info_t  g_mcpu_pipe_info[MAX_CHANNEL_INDEX];
static ipc_msg_t    g_mcpu_ipc_msg[MAX_CHANNEL_INDEX][MAX_PIPE_DEPTH];

static void mcpu_mb_msg_pipe_create(void)
{
    u8 i = 0;
    pipe_info_t *p_pipe = NULL;
    for(i = 0; i < MAX_CHANNEL_INDEX; i++)
    {
        p_pipe = &g_mcpu_pipe_info[i];
        p_pipe->pipe_depth = (MAX_PIPE_DEPTH);
        p_pipe->Datapipe = g_mcpu_ipc_msg[i];
        p_pipe->ReadPt = p_pipe->WritePt = 0;
    }
}
static int mcpu_writepipe(pipe_info_t *p_pipe, ipc_msg_t *p_msg)
{
    if((p_pipe->WritePt + 1) % p_pipe->pipe_depth == p_pipe->ReadPt)
    {
        return 1;
    }

    memcpy((char *)(p_pipe->Datapipe + p_pipe->WritePt),(char *)p_msg, sizeof(ipc_msg_t));
    p_pipe->WritePt = (p_pipe->WritePt + 1) % p_pipe->pipe_depth;
    return 0;
}

static int mcpu_readpipe(pipe_info_t *p_pipe, ipc_msg_t *p_msg)
{
    if(p_pipe->ReadPt == p_pipe->WritePt)
    {
        return 1;
    }
    memcpy((char *)p_msg,(char *)(p_pipe->Datapipe + p_pipe->ReadPt),sizeof(ipc_msg_t));
    p_pipe->ReadPt = (p_pipe->ReadPt + 1) % p_pipe->pipe_depth;
    return 0;
}

static int mcpu_mb_msg_set(u8 index, ipc_msg_t *p_msg)
{
    pipe_info_t *p_pipe = NULL;
    if(index >= MAX_CHANNEL_INDEX)
        return -1;

    p_pipe = &g_mcpu_pipe_info[index];

    return mcpu_writepipe(p_pipe, p_msg);
}

static irqreturn_t mcpu_mb_handler(int a, void *b)
{
  unsigned int state = HAL_GET_U32((volatile u32 *)MECPU_MB_INT_STATE);
  ipc_msg_t msg = {0};
  unsigned char index = 0;
  for(index = 0; index < MAX_CHANNEL_INDEX; index++)
  {
      if (state & (1 << index))
      {
        msg.msg_id = HAL_GET_U32((volatile u32 *)E2MCPU_MB_ADDR_REG(0 + index * 4));
        msg.param1 = HAL_GET_U32((volatile u32 *)E2MCPU_MB_ADDR_REG(1 + index * 4));
        msg.param2 = HAL_GET_U32((volatile u32 *)E2MCPU_MB_ADDR_REG(2 + index * 4));

        mcpu_mb_msg_set(index, &msg);
        /* this must be done last */
		HAL_PUT_U32((volatile u32 *)M2ECPU_MB_INTCLR, 1 << index);
		HAL_PUT_U32((volatile u32 *)M2ECPU_MB_ENACLR, 1 << index);
      }
  }

  return IRQ_HANDLED;

}

static int mcpu_mb_msg_receive(u8 index, ipc_msg_t *p_msg)
{
    int ret = -1;
    pipe_info_t *p_pipe = NULL;

    if(index >= MAX_CHANNEL_INDEX)
        return ret;

    down(&g_mbx_priv.sem[index]);

    p_pipe = &g_mcpu_pipe_info[index];
    ret = mcpu_readpipe(p_pipe, p_msg);

    up(&g_mbx_priv.sem[index]);

    return ret;
}

int ipcs_recv_mbx_msg(unsigned char channel, ipc_msg_t *p_msg)
{
    //unsigned long  timeout = jiffies + 4*HZ;
    u32 cpu_status = 0;
#if defined (CONFIG_MIPS)
#define GET_SR(sr) { __asm__ __volatile__ ("mfc0 %0, $12" : "=d"(sr));}
#elif defined (CONFIG_ARM)
//TODO-20190705
#define GET_SR(sr)	do{sr = 1;}while(0)
#endif

    /*
     * Wait max 5s, if exceeds 5s, system would be reset.
     * So we set max wait time:4s.
     */
    //while(time_before(jiffies, timeout)
    //endless waiting msg from firmware.
    //If blocked, means firmware in trouble, the whole system should be reset
    while(1)
    {
        if(!mcpu_mb_msg_receive(channel, p_msg))
        {
            /*Get the msg and to do the job*/
            return 0;
        }
        GET_SR(cpu_status);
        if(cpu_status & 0x1)
        {
            //Bug: occupy cpu too much!
            msleep(10);
        }
        else
        {
            mcpu_mb_handler(-1, NULL);
        }
    }
    return -1;
}

/*
 *  This is one method used to notify another CPU
 *
 */
int ipcs_send_mbx_msg(unsigned int channel, ipc_msg_t *msg)
{
    /* mcpu mailbox intr state must be determined first */
    unsigned int state = 0;
    unsigned int enable = 0;
	u32 temp;

    down(&g_mbx_priv.sem[channel]);

    state  =  HAL_GET_U32((volatile u32 *)MECPU_MB_INT_STATE);
    enable = HAL_GET_U32((volatile u32 *)MECPU_MB_ENA_STATE);
    /*
     * if intr for that channel has still not been dealt by MCPU yet,
     * we could not send one more intr.
     */
    if (state & (0x1 << channel))
    {
        up(&g_mbx_priv.sem[channel]);
        return 1;
    }

    /* if intr for that channel in peer CPU is not enable, let's enable it */
    if (!(enable & (0x1 << channel)))
    {
		temp = HAL_GET_U32((volatile u32 *)M2ECPU_MB_ENASET);
		temp |= (0x1 << channel);
		HAL_PUT_U32((volatile u32 *)M2ECPU_MB_ENASET, temp);
    }

    /* tell peer CPU where the msg is */
    HAL_PUT_U32((volatile u32 *)M2ECPU_MB_ADDR_REG(0+ channel * 4), msg->msg_id);
    HAL_PUT_U32((volatile u32 *)M2ECPU_MB_ADDR_REG(1+ channel * 4), msg->param1);
    HAL_PUT_U32((volatile u32 *)M2ECPU_MB_ADDR_REG(2+ channel * 4), msg->param2);  /* now, start */

	temp = HAL_GET_U32((volatile u32 *)M2ECPU_MB_INTSET);
	temp |= (0x1 << channel);
	HAL_PUT_U32((volatile u32 *)M2ECPU_MB_INTSET, temp);

    /*Waiting scpu get the msg*/
    while(1)
    {
        state = (HAL_GET_U32((volatile u32 *)EMCPU_MB_INT_STATE) & (0x1 << channel)) >> channel;
        if(!state)
        {
            up(&g_mbx_priv.sem[channel]);
            return 0;
        }
		//Bug: occupy cpu too much at runtime checking.
        usleep_range(100, 1000);
    }

    up(&g_mbx_priv.sem[channel]);
    return 0;
}

int ipcs_check_seccpu_status(void)
{
    return HAL_GET_U32((volatile u32 *)E2MCPU_MB_INIT);
}

void ipcs_set_local_status(int status)
{
    if (status == 0)
        HAL_PUT_U32((volatile u32 *)M2ECPU_MB_INIT, CPU_NOT_READY);
    else
        HAL_PUT_U32((volatile u32 *)M2ECPU_MB_INIT, CPU_READY);
}

void ipcs_mbx_init(void)
{
    int ret = 0;
    int i = 0;

    mcpu_mb_msg_pipe_create();
    //mcpu_pic_intr_mask(IRQ_E2MCPU_MB_ID, 0);
    ret = request_irq(16,mcpu_mb_handler, IRQF_TRIGGER_RISING, "mailbox", NULL);
    if (ret != 0) {
        MT_ERR_CIPHER("request_irq failed\n");
    }
    /* clear  ENA state and INT state */
    HAL_PUT_U32((volatile u32 *)M2ECPU_MB_ENACLR, 0xFFFFFFFF);
    HAL_PUT_U32((volatile u32 *)M2ECPU_MB_INTCLR, 0xFFFFFFFF);

    for (i = 0; i < MAX_CHANNEL_INDEX; i++)
    {
        sema_init(&g_mbx_priv.sem[i], MT_TRUE);
    }
}

void ipcs_mbx_deinit(void)
{
    free_irq(16, mcpu_mb_handler);
}

