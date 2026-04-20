/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_OPC_H__
#define __HW_OPC_H__

#include <linux/wait.h>
#include <linux/mutex.h>

#include <mt_mach/symphony_regs.h>
#include "mt_mach/symphony_io.h"
#include "mt_drv_dev.h"


#define OPC_REG32(x)                    *(u32 *)((volatile u32 *)(x))

//#define DISP_OPC_REGISTER_BASE            SYMPHONY_DISPLAY_REGISTER_VIRT_BASE
#define DISP_OPC_CLR_REG_OFFSET           0x8080
#define DISP_OPC_MASK_REG_OFFSET          0x8088
#define DISP_OPC_STATUS_REG_OFFSET        0x8090
#define OPC_REE_IRQ_CLR_REG(base)         (base + DISP_OPC_CLR_REG_OFFSET)
#define OPC_REE_IRQ_MASK_REG(base)        (base + DISP_OPC_MASK_REG_OFFSET)
#define OPC_VIOLATION_STATUS_REG(base)    (base + DISP_OPC_STATUS_REG_OFFSET)

#define IRQ_DISP_OPC_REE                  IRQ_DISP_OP_REE_ID
#define DISP_OPC_MASK_LO_VALUE            0xFFFF1FF8
#define DISP_OPC_MASK_HI_VALUE            0x107FF

typedef enum{
	OPC_IRQ_ON = 0,
	OPC_IRQ_OFF,
}MT_OPC_IRQ_ST_E;


typedef struct {
    ulong                base;
    uint32_t             irq;
	wait_queue_head_t    opcq;
    uint32_t             block_rd_len;
	uint64_t             opc_status;
	ulong                interval_ticks;
	struct timer_list    timer;
    //wait_queue_head_t rq;
    mt_device_s          dev;
    struct mutex         opc_mutex;
	struct work_struct   work;
	struct delayed_work  delay_work;
	struct workqueue_struct *workqueue;
} opc_handle_t;

void hw_opc_clear_interrupt(opc_handle_t *handle);
void hw_opc_onoff_int_mask(opc_handle_t *handle, uint32_t onoff);
uint64_t hw_opc_get_status(opc_handle_t *handle);
#endif	/*__HW_OPC_H__*/
