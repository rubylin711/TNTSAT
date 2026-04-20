/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __CERT_IOCTL_H__
#define __CERT_IOCTL_H__

#define CERT_EXCHANGE_MAX_CMD_NUM 64

typedef struct _CERT_EXCHANGE_STRUCT
{
    mt_u32 cmds_num;
    mt_u32 processed_num;
    mt_u32 cert_status;
    void *p_cmds; /*CERT_COMMAND_S*/
} CERT_EXCHANGE_S;

typedef struct _CERT_EXPORT_KEY_STRUCT
{
    mt_u32 slot_id;
    mt_u32 ext_attr;
} CERT_EXPORT_KEY_S;

#define MT_ID_CERT 0xBF

#define CERT_DRV_IOC_LOCK _IO(MT_ID_CERT, 0x0)
#define CERT_DRV_IOC_UNLOCK _IO(MT_ID_CERT, 0x1)
#define CERT_DRV_IOC_EXCHANGE _IOWR(MT_ID_CERT, 0x2, CERT_EXCHANGE_S)
#define CERT_DRV_IOC_EXPORT_KEY _IOW(MT_ID_CERT, 0x3, CERT_EXPORT_KEY_S)
#define CERT_DRV_IOC_KEY_ACK _IO(MT_ID_CERT, 0x4)
#define CERT_DRV_IOC_RESET _IO(MT_ID_CERT, 0x5)

#endif
