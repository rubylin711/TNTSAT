/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2019, Montage Technology Co., Ltd.
 *
 * File Name      : drv_ampshm_ioctl.h
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/8/22
 * Description    : AVCPU Asymmetric Share Memory DRV IO Ctrl.
 * History        :
 * 1.Date         : 2019/8/22
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifndef __INC_DRV_AMPSHM_IOCTL_H__
#define __INC_DRV_AMPSHM_IOCTL_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

/* AMPSHM map file structure */
struct mt_ampshm_file_st
{
	int fd;

	/* open args */
	char name[MAX_SHM_NAME_LEN+1];
	int oflag;

	//FIXME
	//!avoid discrepancy of sizeof(mode_t) between kernel and user space
	//mode_t mode;
	unsigned int mode;

	//FIXME
	//!avoid discrepancy of sizeof(off_t) between kernel and user space
	//off_t length;
	unsigned int length;

	/* map args */
	int prot;
	int flags;

	phys_addr_t phy_addr;
	ulong kn_vir_addr;
	ulong usr_vir_addr;	/* mapped user virtual address */
};

enum
{
	IOC_AMPSHM_OPEN,
	IOC_AMPSHM_FTRUNCATE,
	IOC_AMPSHM_MMAP,
	IOC_AMPSHM_MUNMAP,
	IOC_AMPSHM_UNLINK
};

/* AMPSHM driver IO command */
#define CMD_AMPSHM_OPEN            _IOWR(MT_ID_AMPSHM, 	IOC_AMPSHM_OPEN, 		struct mt_ampshm_file_st)
#define CMD_AMPSHM_FTRUNCATE       _IOW(MT_ID_AMPSHM, 	IOC_AMPSHM_FTRUNCATE, 	struct mt_ampshm_file_st)
#define CMD_AMPSHM_MMAP            _IOWR(MT_ID_AMPSHM, 	IOC_AMPSHM_MMAP, 		struct mt_ampshm_file_st)
#define CMD_AMPSHM_MUNMAP          _IOWR(MT_ID_AMPSHM, 	IOC_AMPSHM_MUNMAP, 		struct mt_ampshm_file_st)
#define CMD_AMPSHM_UNLINK          _IOWR(MT_ID_AMPSHM, 	IOC_AMPSHM_UNLINK, 		struct mt_ampshm_file_st)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif

