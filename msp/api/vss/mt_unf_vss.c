/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "drv_vss_ioctl.h"
#include "mt_common.h"

#include "mt_unf_vss.h"

mt_s32 MT_UNF_VSS_Init(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 MT_UNF_VSS_DeInit(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 MT_UNF_VSS_Reset(mt_void)
{
    int fd = -1;
    int ret;

    fd = open("/dev/mt_vss", O_RDWR);
    if(fd < 0)
	return MT_FAILURE;

    ret = ioctl(fd, VSS_IOC_RESET, 0);
    close(fd);

    if (ret < 0)
	return MT_FAILURE;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_VSS_SetPID(const mt_u32 pid)
{
    int fd = -1;
    int ret;

    CMD_VSS_SET_PID_S app_pid = {0};

    app_pid.pid = pid;

    fd = open("/dev/mt_vss", O_RDWR);
    if(fd < 0)
	    return MT_FAILURE;

    ret = ioctl(fd, VSS_IOC_SET_PID, &app_pid);
    close(fd);

    if (ret < 0)
	    return MT_FAILURE;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_VSS_CreateMailbox(mt_handle *mailbox)
{
    int fd = -1;

    fd = open("/dev/mt_vss_mb", O_RDWR);
    if(fd < 0)
	return MT_FAILURE;

    *mailbox = (mt_handle)fd;
    return MT_SUCCESS;
}

mt_s32 MT_UNF_VSS_DestroyMailbox(mt_handle mailbox)
{
    if (close((int)mailbox) < 0)
	return MT_FAILURE;

    return MT_SUCCESS;
}

mt_s32 MT_UNF_VSS_WriteMailbox(mt_handle mailbox, const mt_void *data, size_t len)
{
    ssize_t bytes;

    bytes = write((int)mailbox, data, len);

    if (bytes < 0)
	return MT_FAILURE;

    return (mt_s32)bytes;
}

mt_s32 MT_UNF_VSS_ReadMailbox(mt_handle mailbox, mt_void *data, size_t len)
{
    ssize_t bytes;

    bytes = read((int)mailbox, data, len);

    if (bytes < 0)
	return MT_FAILURE;

    return (mt_s32)bytes;
}

