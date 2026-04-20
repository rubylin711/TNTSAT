/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/types.h>

#include "hw_mailbox.h"

void hw_mailbox_clear_interrupt(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);

    addr->mb_VS2AP_int_clr = 1;
}
void hw_mailbox_clear_read_interrupt(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);

    addr->mb_VS2AP_rd_int_clr = 1;
}
void hw_mailbox_clear_write_interrupt(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);

    addr->mb_VS2AP_wr_int_clr = 1;
}


void hw_mailbox_clear_notice_interrupt(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->tee_base);

    addr->mb_notify_tee_int_clr = 1;
}


void hw_mailbox_onoff_read_int_mask(mailbox_handle_t *handle, uint32_t onoff)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);

    if (onoff)
        addr->mb_VS2AP_rd_int_mask = 1;
    else
        addr->mb_VS2AP_rd_int_mask = 0;
}

int32_t hw_mailbox_read_block(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);
    uint32_t i,trans_word;

    if ((addr->mb_reg2_full == 1) && (addr->mb_reg2_empty == 0)) {
        trans_word = 1+(addr->mb_trans_length);
        handle->block_rd_len = trans_word << 2;
        for (i=0; i<trans_word; i++) {
            handle->block_rd_buf[i] = addr->mb_data_reg[i];
        }

        if (addr->mb_reg2_empty == 1 && addr->mb_reg2_read_error == 0) {
            return handle->block_rd_len;
        } else {
            if (addr->mb_reg2_read_error == 1)
                addr->mb_reg2_read_error_clr = 1;

            return MBOX_BLOCK_READ_FAILED;
        }
    } else {
        return MBOX_NOT_READY_FOR_READ_BLOCK_DATA;
    }
}

int32_t hw_mailbox_write_block(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);
    uint32_t i, trans_word;

    if ((addr->mb_reg1_empty == 1) && (addr->mb_reg1_full == 0)) {
        trans_word = handle->block_wr_len >> 2;
        addr->mb_trans_length = trans_word-1;
        for (i=0; i<trans_word; i++) {
            addr->mb_data_reg[i] = handle->block_wr_buf[i];
        }

        //if (addr->mb_reg1_full == 1 && addr->mb_reg1_write_error == 0)
        if (addr->mb_reg1_write_error == 0) {
            return handle->block_wr_len;
        } else {
            if (addr->mb_reg1_write_error == 1)
                addr->mb_reg1_write_error_clr = 1;

            return MBOX_BLOCK_WRITE_FAILED;
        }
    } else {
        return MBOX_NOT_READY_FOR_WRITE_BLOCK_DATA;
    }
}

int32_t hw_mailbox_check_state(mailbox_handle_t *handle)
{
    mailbox_reg_t *addr = (mailbox_reg_t *)(handle->base);
    int32_t state = 0;

    /*if (addr->mb_reg2_full==1 && addr->mb_reg2_empty==0)*/
    if (!addr->mb_VS2AP_rd_int_mask && addr->mb_VS2AP_rd_int_status)
        state |= HW_STATE_READY_FOR_READ;

    /*if (addr->mb_reg1_full==0 && addr->mb_reg1_empty==1)*/
    if (!addr->mb_VS2AP_wr_int_mask && addr->mb_VS2AP_wr_int_status)
        state |= HW_STATE_READY_FOR_WRITE;

    return state;
}
