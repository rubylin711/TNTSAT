/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_SCI_IOCTL_H__
#define __DRV_SCI_IOCTL_H__
#include <linux/ioctl.h>
#include <linux/types.h>
#include "mt_unf_sci.h"

typedef enum _SCI_SLOT_TYPE_E
{
    SCI_SLOT_ALWAYS_CLOSE = 0,
    SCI_SLOT_ALWAYS_OPEN,
} SCI_SLOT_TYPE_E;

typedef enum _SCI_STOP_WIDTH_E
{
    SCI_TWO_STOPS = 0,
    SCI_ONE_STOP
} SCI_STOP_WIDTH_E;

typedef enum _SCI_PINMODE_E
{
    SCI_PINMODE_BY_CHIP = 0,
    SCI_PINMODE_BY_PULLUP,
} SCI_PINMODE_E;

typedef enum _SCI_CARD_STATUS_E
{
    SCI_STATUS_CARDOUT = 0,
    SCI_STATUS_CARDIN,
} SCI_CARD_STATUS_E;

#pragma pack(4)
typedef struct _SCI_ATTR_S
{
    u32 read_timeout;
    u32 write_timeout;
	u32 rst_timeout;
	u32 rece_timeout;
	u32 blk_timeout;
    u32 etu;
    u32 Hz;
    u_char dev_id;
    u_char N;
    u_char stop_width;
    u_char error_handle_en; //
    u_char parity_en;
    u_char slot_type;
    u_char vcc_en_level;
    u_char clkpin_mode;     //
    u_char rstpin_mode;
    u_char iopin_mode;
    u_char rx_retrys;
    u_char tx_retrys;       //
    u_char tx_finish_en;
	u_char recetime_en;
	u_char blktime_en;
	u_char rsttime_en;      //
	u_char type;
	u_char reserved[3];     //
} SCI_ATTR_S;

typedef struct _SCI_STATUS_S
{
    u_char dev_id;
    u_char status;
	u_char reserved[2];     //
} SCI_STATUS_S;

typedef struct _SCI_DATA_S
{
    u_char  dev_id;
	u_char  reserved[3];     //
    u32     data_len;
    u64     data_buf;       //u_char  *data_buf
} SCI_DATA_S;
#pragma pack()

#define SCI_IOC_BASE	'S'
#define SCI_IOC_SET_ATTR            _IOW(SCI_IOC_BASE, 0, SCI_ATTR_S)
#define SCI_IOC_GET_ATTR            _IOWR(SCI_IOC_BASE, 1, SCI_ATTR_S)
#define SCI_IOC_GET_STATUS          _IOWR(SCI_IOC_BASE, 2, SCI_STATUS_S)
#define SCI_IOC_ACTIVATE            _IOW(SCI_IOC_BASE, 3, MT_UNF_SCI_PORT_E)
#define SCI_IOC_RESET               _IOW(SCI_IOC_BASE, 4, MT_UNF_SCI_PORT_E)
#define SCI_IOC_DEACTIVATE          _IOW(SCI_IOC_BASE, 5, MT_UNF_SCI_PORT_E)
#define SCI_IOC_SEND_DATA           _IOWR(SCI_IOC_BASE, 6, SCI_DATA_S)
#define SCI_IOC_RECIEVE_DATA        _IOWR(SCI_IOC_BASE, 7, SCI_DATA_S)
#define SCI_IOC_INIT                _IOWR(SCI_IOC_BASE, 8, MT_UNF_SCI_PORT_E)
#define SCI_IOC_RECIEVE_COMPLETED   _IOWR(SCI_IOC_BASE, 9, u32)
#define SCI_IOC_SOFT_RESET          _IOWR(SCI_IOC_BASE, 10, u32)                      //for fpga
#define SCI_IOC_HARDWARE_PWROFF     _IOWR(SCI_IOC_BASE, 11, MT_UNF_SCI_H_PWROFF_S)    //for fpga
#define SCI_IOC_OVERLOAD            _IOWR(SCI_IOC_BASE, 12, u32)
#endif
