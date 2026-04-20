/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_MAILBOX_H__
#define __HW_MAILBOX_H__

#include <linux/wait.h>
#include <linux/mutex.h>

#define MAILBOX_FIFO_DEPTH (64)

#define MBOX_BLOCK_READ_FAILED              (-1)
#define MBOX_NOT_READY_FOR_READ_BLOCK_DATA  (-2)

#define MBOX_BLOCK_WRITE_FAILED             (-11)
#define MBOX_NOT_READY_FOR_WRITE_BLOCK_DATA (-12)

#define HW_STATE_READY_FOR_READ             (1 << 0)
#define HW_STATE_READY_FOR_WRITE            (1 << 1)

#define IRQ_VSS_TEE_ID                      IRQ_MBOX2_VS2ACPU_ID

#define __IOM volatile

typedef struct {
    //================__IOM uint32_t Mailbox_CSR_REG(0xBF4B0000)================
    __IOM uint32_t mb_trans_length: 4;
    __IOM uint32_t : 4;
    __IOM uint32_t mb_is_wr_finished: 1;    //reg1 full
    __IOM uint32_t mb_is_rd_finished: 1;    //reg2 empty
    __IOM uint32_t : 6;
    __IOM uint32_t mb_disable_flag: 2;
    __IOM uint32_t : 14;

    //================__IOM uint32_t Mailbox_SEM_REG(0xBF4B0004)================
    __IOM uint32_t mb_sem_AccessCtrl: 4;
    __IOM uint32_t mb_sem_TimerInit: 28;

    //================__IOM uint32_t Mailbox_Data_REG(0xBF4B0008~0xBF4B0084)================
    __IOM uint32_t mb_data_reg[16];
    __IOM uint32_t mb_reserved[16];

    //================__IOM uint32_t Mailbox_AP2VS_int_set_REG(0xBF4B0088)================
    __IOM uint32_t mb_AP2VS_int_set: 1;
    __IOM uint32_t : 31;

    //================__IOM uint32_t Mailbox_VS2AP_int_clr_REG(0xBF4B008C)================
    __IOM uint32_t mb_VS2AP_int_clr: 1;
    __IOM uint32_t mb_VS2AP_rd_int_clr: 1;
    __IOM uint32_t mb_VS2AP_wr_int_clr: 1;
    __IOM uint32_t : 29;

    //================__IOM uint32_t Mailbox_VS2AP_int_mode_sel_REG(0xBF4B0090)================
    __IOM uint32_t mb_VS2AP_int_mode_sel: 1;
    __IOM uint32_t : 31;

    //================__IOM uint32_t Mailbox_VS2AP_int_mask_REG(0xBF4B0094)================
    __IOM uint32_t mb_VS2AP_int_mask: 1;		//Software trigger mode, APCPU interrupt mask
    __IOM uint32_t mb_VS2AP_rd_int_mask: 1;	//Hardware trigger mode when vscpu write full, APCPU interrupt mask
    __IOM uint32_t mb_VS2AP_wr_int_mask: 1;	//Hardware trigger mode when vscpu read empty, APCPU interrupt mask
    __IOM uint32_t mb_VS2AP_int_status: 1;
    __IOM uint32_t mb_VS2AP_rd_int_status: 1;
    __IOM uint32_t mb_VS2AP_wr_int_status: 1;
    __IOM uint32_t : 26;

    //================__IOM uint32_t Mailbox_VS2AP_int_set_REG(0xBF4B0098)================
    __IOM uint32_t mb_VS2AP_int_set: 1;
    __IOM uint32_t : 31;

    //================__IOM uint32_t Mailbox_AP2VS_int_clr_REG(0xBF4B009C)================
    __IOM uint32_t mb_AP2VS_int_clr: 1;
    __IOM uint32_t mb_AP2VS_rd_int_clr: 1;
    __IOM uint32_t mb_AP2VS_wr_int_clr: 1;
    __IOM uint32_t : 29;

    //================__IOM uint32_t Mailbox_AP2VS_int_mode_sel_REG(0xBF4B00A0)================
    __IOM uint32_t mb_AP2VS_int_mode_sel: 1;
    __IOM uint32_t : 31;

    //================__IOM uint32_t Mailbox_AP2VS_int_mask_REG(0xBF4B00A4)================
    __IOM uint32_t mb_AP2VS_int_mask: 1;		//Software trigger mode, VSCPU interrupt mask
    __IOM uint32_t mb_AP2VS_rd_int_mask: 1;	//Hardware trigger mode when apcpu write full, VSCPU interrupt mask
    __IOM uint32_t mb_AP2VS_wr_int_mask: 1;	//Hardware trigger mode when apcpu read empty, VSCPU interrupt mask
    __IOM uint32_t mb_AP2VS_int_status: 1;
    __IOM uint32_t mb_AP2VS_rd_int_status: 1;
    __IOM uint32_t mb_AP2VS_wr_int_status: 1;
    __IOM uint32_t : 26;

    //================__IOM uint32_t Mailbox_AP2VS_int_mode_sel_lock_REG(0xBF4B00A8)================
    __IOM uint32_t mb_AP2VS_int_mode_sel_lock: 1;
    __IOM uint32_t : 31;

    //================__IOM uint32_t Mailbox_VS2AP_int_mode_sel_lock_REG(0xBF4B00AC)================
    __IOM uint32_t mb_VS2AP_int_mode_sel_lock: 1;
    __IOM uint32_t : 31;

    //================__IOM uint32_t Mailbox_Reg1_state_REG(0xBF4B00B0)================
    __IOM uint32_t mb_reg1_full: 1;                  //apcpu finish writing
    __IOM uint32_t : 3;
    __IOM uint32_t mb_reg1_empty: 1;             //vscpu finish reading
    __IOM uint32_t : 11;
    __IOM uint32_t mb_reg1_write_error: 1;     //apcpu writes any data after (length+1)data have been written, the data is discarded and this bit will set 1
    __IOM uint32_t : 3;
    __IOM uint32_t mb_reg1_read_error: 1;     //vscpu reads any data after (length+1)data have been readen, the data is 0 and this bit will set 1
    __IOM uint32_t : 11;

    //================__IOM uint32_t Mailbox_Reg2_state_REG(0xBF4B00B4)================
    __IOM uint32_t mb_reg2_full: 1;                  //vscpu finish writing
    __IOM uint32_t : 3;
    __IOM uint32_t mb_reg2_empty: 1;             //apcpu finish reading
    __IOM uint32_t : 11;
    __IOM uint32_t mb_reg2_write_error: 1;     //vscpu writes any data after (length+1)data have been written, the data is discarded and this bit will set 1
    __IOM uint32_t : 3;
    __IOM uint32_t mb_reg2_read_error: 1;     //apcpu reads any data after (length+1)data have been readen, the data is 0 and this bit will set 1
    __IOM uint32_t : 11;

    //================__IOM uint32_t Mailbox_AP_data_wr_err_clr_REG(0xBF4B00B8)================
    __IOM uint32_t mb_reg1_write_error_clr: 1;  //apcpu write data error clear. this bit will clear to zero by itself after set 1
    __IOM uint32_t : 3;
    __IOM uint32_t mb_reg2_read_error_clr: 1;   //apcpu read data error clear. this bit will clear to zero by itself after set 1
    __IOM uint32_t : 27;

    //================__IOM uint32_t Mailbox_VS_data_wr_err_clr_REG(0xBF4B00BC)================
    __IOM uint32_t mb_reg2_write_error_clr: 1;  //vscpu write data error clear. this bit will clear to zero by itself after set 1
    __IOM uint32_t : 3;
    __IOM uint32_t mb_reg1_read_error_clr: 1;   //vscpu read data error clear. this bit will clear to zero by itself after set 1
    __IOM uint32_t : 27;

    //================__IOM uint32_t Mailbox_VS_data_wr_err_clr_REG(0xBF4B00C0)================
    __IOM uint32_t mb_notify_tee_int_clr: 1;  //vscpu write data to REE and notify ree invoke to tee handle data. this bit will clear to zero by itself after set 1
    __IOM uint32_t : 31;      
} mailbox_reg_t;

typedef struct {
    ulong    base;
    uint32_t irq;
    uint32_t block_rd_buf[MAILBOX_FIFO_DEPTH>>2];
    uint32_t block_wr_buf[MAILBOX_FIFO_DEPTH>>2];
    uint32_t block_rd_len;
    uint32_t block_wr_len;
    uint32_t block_rd_notice;
    uint32_t block_wr_echo;
    wait_queue_head_t wq;
    wait_queue_head_t rq;

    struct mutex read_write_mutex;

    ulong    tee_base;
    uint32_t tee_irq;
} mailbox_handle_t;

void hw_mailbox_clear_notice_interrupt(mailbox_handle_t *handle);
void hw_mailbox_clear_interrupt(mailbox_handle_t *handle);
void hw_mailbox_clear_read_interrupt(mailbox_handle_t *handle);
void hw_mailbox_clear_write_interrupt(mailbox_handle_t *handle);
void hw_mailbox_onoff_read_int_mask(mailbox_handle_t *handle, uint32_t onoff);
int32_t hw_mailbox_init(mailbox_handle_t *handle);
int32_t hw_mailbox_read_block(mailbox_handle_t *handle);
int32_t hw_mailbox_write_block(mailbox_handle_t *handle);
int32_t hw_mailbox_check_state(mailbox_handle_t *handle);

#endif	/*__HW_MAILBOX_H__*/
