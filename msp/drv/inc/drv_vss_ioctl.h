/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_VSS_IOCTL_H__
#define __DRV_VSS_IOCTL_H__
#include "mt_type.h"
#include "mt_module.h"

typedef struct _CMD_VSS_SET_PID_S
{
    mt_u32 pid;
} CMD_VSS_SET_PID_S;


#define VSS_IOC_RESET                         _IO(MT_ID_VSS, 0x0)

#define VSS_IOC_SET_PID                       _IOWR(MT_ID_VSS, 0x1, CMD_VSS_SET_PID_S)


#endif	/*__DRV_VSS_IOCTL_H__*/
