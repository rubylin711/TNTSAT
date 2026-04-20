/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_SCI_PRIV_H__
#define __DRV_SCI_PRIV_H__
#include "mt_drv_mmz.h"

#if defined(CONFIG_MT_CHIP_ARIA)
//#define APB_CLOCK_FREQ  (548500000 / 8) 
#define APB_CLOCK_FREQ 100000000//54000000
#define IRQ_SMC0_ID (51 + 32)
#define IRQ_SMC1_ID (52 + 32)
#endif
#define RX_FIFO_SIZE  512


enum sci_baseclk
{
    SCI_BASECLK_APB = 0,
    SCI_BASECLK_12MHz
};

enum sci_status 
{
    SCI_STATUS_DEACTIVATE = 0,
    SCI_STATUS_RESET,
    SCI_STATUS_ACTIVATE
};

typedef struct _reg_sta
{
    unsigned char rx_sta : 1;
    unsigned char card_sta : 1;
    unsigned char reserve0 : 2;
    unsigned char cmd_req : 1;
    unsigned char derr_type1 : 1;
    unsigned char derr_type0 : 1;
    unsigned char conversion : 1;
} reg_sta;

typedef struct _reg_freq_cfg
{
    unsigned char slot_type : 1;
    unsigned char freq_div : 6;
    unsigned char reserve0 : 1;
} reg_freq_cfg;

typedef struct _reg_trans_ctrl
{
    unsigned char ie_cmd : 1;
    unsigned char ie_rx : 1;
    unsigned char ie_remove: 1;
    unsigned char ie_insert : 1;
    unsigned char parity_en : 1;
    unsigned char check_en : 1;
    unsigned char ncset_en : 1;
    unsigned char stop_width : 1;
} reg_trans_ctrl;

typedef struct _reg_etu1_set
{
    unsigned char etu_17_16: 2;
    unsigned char reserve0: 4;
    unsigned char cvtset_value : 1;
    unsigned char cvtset_en : 1;
} reg_etu1_set;

typedef struct _reg_cpctrl
{
    unsigned char pow_ctrl : 1;
    unsigned char fifo_rst : 1;
    unsigned char card_rst : 1;
    unsigned char reserve1 : 1;
    unsigned char sync_en : 1;
    unsigned char clko_en : 1;
    unsigned char data_en : 1;
    unsigned char reserve0 : 1;
} reg_cpctrl;

typedef struct _reg_buf_cnt_h
{
    unsigned char reserve0: 7;
    unsigned char buf_cnt : 1;
} reg_buf_cnt_h;

typedef struct _reg_pin_cfg
{
    unsigned char iopin_mode : 1;
    unsigned char rstpin_mode : 1;
    unsigned char clkpin_mode : 1;
    unsigned char reserve0 : 5; 
} reg_pin_cfg;

typedef struct _reg_resend_cfg
{
    unsigned char txd_num : 4;
    unsigned char rxd_num : 4;
} reg_resend_cfg;

typedef struct _reg_mode_cfg
{
    unsigned char sonata_en : 1;
    unsigned char p_mode : 1;
    unsigned char reserve : 2;
    unsigned char txfinsh_en : 1;
    unsigned char rece_time_en : 1;
    unsigned char blk_time_en : 1;
    unsigned char rst_time_en : 1;
} reg_mode_cfg;

typedef struct _reg_discard_recv_c8
{
    unsigned char discardrecv : 1;
    unsigned char reserve : 7;
} reg_discard_recv_c8;

struct smc_rx_fifo
{
    unsigned char buff[RX_FIFO_SIZE];
    int wp;
    int rp;
    int count;
};

struct sci_info
{   
    u_char id;
    u_char status;
    u_char readable;
    u_char writeend;
    u_char intstatus;
    u_char paritystatus;
    u_char set_attr_cnt;
    u_char init;      //bit0: smc isr   bit1: kadc2 isr
    u_long clkbase;
    u_long chipid;
    u_long regbase;
    struct mutex ioctl_lock;
    spinlock_t wrlock;
    spinlock_t sndlock;
    wait_queue_head_t wq;
    SCI_ATTR_S attr;
    struct smc_rx_fifo rx_fifo;
    mmz_buffer_s dmammzbuf;
};

struct sci_priv_data
{
    ulong membase;
	struct clk *smc0phyclk;
	atomic_t sci_open_cnt_atomic;
};

#endif
