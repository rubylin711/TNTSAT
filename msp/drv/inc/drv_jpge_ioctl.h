/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_JPGE_IOCTL_H__
#define __MT_JPGE_IOCTL_H__

#include <linux/ioctl.h>
#include "mt_jpge_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/* Use 'j' as magic number */
#define JPGE_IOC_MAGIC 'j'

#define JPGE_CREATE_CMD _IOWR(JPGE_IOC_MAGIC, 100, Jpge_EncCfgInfo_S)
#define JPGE_ENCODE_CMD _IOWR(JPGE_IOC_MAGIC, 101, Jpge_EncInfo_S)
#define JPGE_DESTROY_CMD _IOW(JPGE_IOC_MAGIC, 102, JPGE_HANDLE)

//Validaiton only
#define JPGE_GET_CMD     _IOW(JPGE_IOC_MAGIC, 103, Jpge_EncHWInfo_S)
#define JPGE_RESET_CMD   _IO(JPGE_IOC_MAGIC, 104)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */


#endif /* End of #ifndef __MT_JPGE_IOCTL_H__ */

