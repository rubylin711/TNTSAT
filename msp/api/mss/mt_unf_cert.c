/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
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
#include "mt_unf_cert.h"
#include "drv_cert_ioctl.h"

mt_s32 mt_unf_cert_open(mt_handle * p_handle)
{
	int fd = open("/dev/mt_cert", O_RDWR);
	if (fd < 0) {
		return MT_FAILURE;
	}
	*p_handle = (mt_handle) fd;
	return MT_SUCCESS;
}

mt_s32 mt_unf_cert_close(mt_handle handle)
{
	return close((int)handle);
}

mt_s32 mt_unf_cert_lock(mt_handle handle)
{
	return ioctl((int)handle, CERT_DRV_IOC_LOCK, NULL);
}

mt_s32 mt_unf_cert_unlock(mt_handle handle)
{
	return ioctl((int)handle, CERT_DRV_IOC_UNLOCK, NULL);
}

mt_s32 mt_unf_cert_exchange(mt_handle handle, mt_u32 cmds_num,
			    const CERT_COMMAND_S * p_cmds,
			    mt_u32 * p_processed_num)
{
	mt_s32 ret = -1;
	CERT_EXCHANGE_S exchange;
	exchange.cmds_num = cmds_num;
	exchange.p_cmds = (void *)p_cmds;
	exchange.processed_num = 0;
	exchange.cert_status = 0;
	ret = ioctl((int)handle, CERT_DRV_IOC_EXCHANGE, &exchange);
	*p_processed_num = exchange.processed_num;
	ret = exchange.cert_status;
	return ret;
}

mt_s32 mt_unf_cert_export_key(mt_handle handle, mt_u32 slot_id, mt_u32 ext_attr)
{
	CERT_EXPORT_KEY_S export;
	export.slot_id = slot_id;
	export.ext_attr = ext_attr;
	return ioctl((int)handle, CERT_DRV_IOC_EXPORT_KEY, &export);
}

mt_s32 mt_unf_cert_key_ack(mt_handle handle)
{
	return ioctl((int)handle, CERT_DRV_IOC_KEY_ACK, NULL);
}

mt_s32 mt_unf_cert_reset(mt_handle handle)
{
	return ioctl((int)handle, CERT_DRV_IOC_RESET, NULL);
}
