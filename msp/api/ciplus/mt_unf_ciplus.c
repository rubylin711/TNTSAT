/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <linux/ioctl.h>
#include <mtd/mtd-abi.h>
#include <linux/sched.h>
#include <errno.h>
#include <malloc.h>
#include <linux/fs.h>
#include <semaphore.h>

#include "mt_type.h"

#include "mt_module_debug.h"

#include "drv_ci_ioctl.h"
#include "mt_unf_ciplus.h"


/*Define Debug Level For FLASH                 */
#define MT_FATAL_CIPLUS(fmt...)		MT_FATAL_PRINT(MT_ID_FLASH, fmt)
#define MT_ERR_CIPLUS(fmt...)		MT_ERR_PRINT(MT_ID_FLASH, fmt)
#define MT_WARN_CIPLUS(fmt...)		MT_WARN_PRINT(MT_ID_FLASH, fmt)
#define MT_INFO_CIPLUS(fmt...)		MT_INFO_PRINT(MT_ID_FLASH, fmt)
#define MT_DBG_CIPLUS(fmt...)		MT_DBG_PRINT(MT_ID_FLASH, fmt)

#define   STATUS_MASK       0xD3


#define   NO_DATA     -0x02
#define   HAVE_DATA   -0x03

struct mtci_info
{
	MT_S32 devfd;
	sem_t sem;
};


/*
static MT_S32 mt_unf_ciplus_reset(MT_HANDLE pdevhandle)
{
	int ret = 0;
	int reset = 1;
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	if (!pdevinfo)
		return MT_FAILURE;
	ret = ioctl(pdevinfo->devfd,CI_IOC_CIRESET,&reset);  //start reset
	usleep(500000);
	reset = 0;
	ret |= ioctl(pdevinfo->devfd,CI_IOC_CIRESET,&reset); //stop reset
	usleep(500000);
	return ret;
}
*/

MT_S32 mt_unf_ciplus_init(void)
{
	MT_DBG_CIPLUS("mt_unf_ciplus_init\n");
	return MT_SUCCESS;
}
MT_S32 mt_unf_ciplus_deinit(void)
{

	MT_DBG_CIPLUS("mt_unf_ciplus_deinit\n");
	return MT_SUCCESS;
}


MT_S32 mt_unf_ciplus_open(const MT_CHAR* pdevfile, MT_HANDLE* pdevhandle)
{
	int fd = -1;
	struct mtci_info *pdevinfo;
	if (!pdevfile || !pdevhandle) {
		return MT_FAILURE;
	}
	if ( (fd = open(pdevfile,O_RDWR|O_CLOEXEC)) < 0) {
		return MT_FAILURE;
	}
	pdevinfo = (struct mtci_info *)malloc(sizeof(struct mtci_info));
	if (!pdevinfo) {
		close(fd);
		return MT_FAILURE;
	}
	pdevinfo->devfd = fd;
	if (sem_init(&pdevinfo->sem,0,1) < 0) {
		free(pdevinfo);
		close(fd);
		return MT_FAILURE;
	}
	*pdevhandle = (MT_HANDLE)pdevinfo;
	return MT_SUCCESS;
}

MT_S32 mt_unf_ciplus_close(MT_HANDLE pdevhandle)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	if (!pdevinfo)
		return MT_FAILURE;
	close(pdevinfo->devfd);
	free(pdevinfo);
	return MT_SUCCESS;
}

MT_S32 mt_unf_ciplus_write(MT_HANDLE pdevhandle,MT_U8 *pdata,MT_U32 len)
{
	int ret = 0;
	int count = 0;
	struct mtci_iorw 	iorw_param;
	MT_U8 iodata[1];
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;

	if (!pdevinfo || !pdata || len == 0)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);

	iorw_param.reg = CIC_CSR;
	iorw_param.buffer = iodata;

	ret = ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("read cic_csr failed\n");
		ret = MT_FAILURE;
		goto WRITE_DONE;
	}
	iodata[0] &= STATUS_MASK;
	if ((iodata[0] & CI_REG_DA) != 0) {
		MT_ERR_CIPLUS("cam has some data wait to be readed\n");
		ret = MT_FAILURE;
		goto WRITE_DONE;
	}

	count = 0;
SET_HC:
	iodata[0] |= CI_REG_HC;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOWRITE, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("send host control failed\n");
		ret = MT_FAILURE;
		goto WRITE_DONE;
	}

	ret = ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("read cic_csr failed\n");
		ret = MT_FAILURE;
		goto WRITE_DONE;
	}
	iodata[0] &= STATUS_MASK;
	if ((iodata[0] & CI_REG_FR) == 0) {
		count++;
		usleep(1000);
		if(count>1000) {
			MT_ERR_CIPLUS("%s%d wait FR set :time out \n",__func__,__LINE__);
			ret = MT_FAILURE;
			goto HC_CLEAR;
		}
		iodata[0] = (0XFF-CI_REG_HC);
		ret |= ioctl(pdevinfo->devfd, CI_IOC_IOWRITE, (MT_HANDLE)&iorw_param);
		goto SET_HC;
	}

	/* Set Data Size */
	iodata[0] = ((len >> 8) & 0xff);
	iorw_param.reg = CIC_SIZEMS;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOWRITE, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("write CIC_SIZEMS failed\n");
		ret = MT_FAILURE;
		goto HC_CLEAR;
	}
	iodata[0] = (len & 0xff);
	iorw_param.reg = CIC_SIZELS;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOWRITE, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("write CIC_SIZELS failed\n");
		ret = MT_FAILURE;
		goto HC_CLEAR;
	}
	/* Write Data to CI card */
	ret  = write(pdevinfo->devfd,pdata,len);
	printf("ret = 0x%x,len = 0x%x\n",ret,len);
	if (ret < 0) {
		MT_ERR_CIPLUS("write 0x%x bytes data failed\n",len);
		ret = MT_FAILURE;
		goto HC_CLEAR;
	}

	iorw_param.reg = CIC_CSR;
	iorw_param.buffer = iodata;
	if (ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param) < 0) {
		MT_ERR_CIPLUS("read cic_csr failed\n");
		ret = MT_FAILURE;
		goto HC_CLEAR;
	}
	iodata[0] &= STATUS_MASK;
	if ((iodata[0] & CI_REG_WE) != 0) {
		MT_ERR_CIPLUS("CSR WE  error\n");
		ret = MT_FAILURE;
	}

HC_CLEAR:
	iorw_param.reg = CIC_CSR;
	iorw_param.buffer = iodata;
	iodata[0] &= (0xff - CI_REG_HC);
	ioctl(pdevinfo->devfd, CI_IOC_IOWRITE, (MT_HANDLE)&iorw_param);
WRITE_DONE:
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_read(MT_HANDLE pdevhandle,MT_U8 *pdata,MT_U32 len)
{
	int ret = 0;
	struct mtci_iorw 	iorw_param;
	MT_U8 iodata[1];
	int bsize = 0;
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;

	if (!pdevinfo || !pdata || len == 0)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);

	/*test DA*/
	iorw_param.buffer = iodata;
	iorw_param.reg = CIC_CSR;
	if (ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param) < 0) {
		MT_ERR_CIPLUS("read cic_csr failed\n");
		goto READ_DONE;
	}
	iodata[0] &= STATUS_MASK;
	if (iodata[0] & CI_REG_IIR) {
		MT_ERR_CIPLUS("card not ready 0x%x!\n",iodata[0]);
		ret = MT_FAILURE;
		goto READ_DONE;
	}
	if ((iodata[0] & CI_REG_DA) == 0) {
		MT_ERR_CIPLUS("data not available\n");
		sem_post(&pdevinfo->sem);
		return NO_DATA;
	}

	iorw_param.reg = CIC_SIZEMS;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("read CIC_SIZEMS failed\n");
		ret = MT_FAILURE;
		goto READ_DONE;
	}
	bsize |= (iodata[0]<<8);

	iorw_param.reg = CIC_SIZELS;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param);
	if (ret < 0) {
		MT_ERR_CIPLUS("read CIC_SIZELS failed\n");
		ret = MT_FAILURE;
		goto READ_DONE;
	}
	bsize |= iodata[0];
	printf("the bsize = %d\n",bsize);
	if (bsize < len) {
		MT_ERR_CIPLUS("cam buffer size longer than user buffer size:%d\n",bsize);
		ret = MT_FAILURE;
		goto READ_DONE;
	}

	ret = read(pdevinfo->devfd,pdata,len);
	printf("ret = %d\n,bsize = %d,len = %d\n",ret,bsize,len);
	if (ret < 0) {
		MT_ERR_CIPLUS("CAM read failed\n");
		ret = MT_FAILURE;
		goto READ_DONE;
	}

	iorw_param.reg = CIC_CSR;
	ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param);
READ_DONE:
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_ioread(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8* pdata)
{
	struct mtci_iorw 	iorw_param;
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle || !pdata)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	iorw_param.reg = addr;
	iorw_param.buffer = pdata;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOREAD, (MT_HANDLE)&iorw_param);
	sem_post(&pdevinfo->sem);
	return ret;
}


MT_S32 mt_unf_ciplus_iowrite(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8 data)
{
	struct mtci_iorw 	iorw_param;
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	iorw_param.reg = addr;
	iorw_param.buffer = &data;
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOWRITE, (MT_HANDLE)&iorw_param);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_memread(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8* pdata,MT_U32 len)
{
	struct mtci_memrw 	memrw_param;
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;

	if (!pdevhandle || !pdata)
		return MT_FAILURE;

	sem_wait(&pdevinfo->sem);

	memrw_param.addr = addr;
	memrw_param.buffer = pdata;
	memrw_param.size = len;
	ret = ioctl(pdevinfo->devfd, CI_IOC_MEMREAD, (MT_HANDLE)&memrw_param);
	sem_post(&pdevinfo->sem);
	if (ret < 0) {
		return ret;
	}
	return (MT_S32)memrw_param.rsize;
}

MT_S32 mt_unf_ciplus_memwrite(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8* pdata,MT_U32 len)
{
	struct mtci_memrw 	memrw_param;
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle || !pdata)
		return MT_FAILURE;

	sem_wait(&pdevinfo->sem);

	memrw_param.addr = addr;
	memrw_param.buffer = pdata;
	memrw_param.size = len;
	ret = ioctl(pdevinfo->devfd, CI_IOC_MEMWRITE, (MT_HANDLE)&memrw_param);
	sem_post(&pdevinfo->sem);
	if (ret < 0) {
		return ret;
	}
	return (MT_S32)memrw_param.rsize;
}


MT_S32 mt_unf_ciplus_cfg_detect_time(MT_HANDLE pdevhandle,MT_U32 time)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_CDCFG, (MT_HANDLE)&time);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_get_camst(MT_HANDLE pdevhandle,
		enum mtci_cam_status* pcamst)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle || !pcamst)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_CAMSTATUS, (MT_HANDLE)pcamst);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_setts1(MT_HANDLE pdevhandle,MT_U32 ts1val)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_SETTS1CTRL, (MT_HANDLE)&ts1val);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_getts1(MT_HANDLE pdevhandle,MT_U32* ts1val)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle || !ts1val)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_GETTS1CTRL, (MT_HANDLE)ts1val);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_set_pidram(MT_HANDLE pdevhandle,MT_U8 index,
		MT_U16 pid1,MT_U16 pid0)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	struct mtci_pid_ram pidram;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	pidram.index = index;
	pidram.val = (MT_U32)((pid1<<16)&0x1fff0000)| (pid0&0x1fff);
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_SETPIDRAM, (MT_HANDLE)&pidram);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_get_pidram(MT_HANDLE pdevhandle,MT_U8 index,
		MT_U16* pid1,MT_U16* pid0)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	struct mtci_pid_ram pidram;
	int ret;
	if (!pdevhandle || !pid1 || !pid0)
		return MT_FAILURE;
	pidram.index = index;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_GETPIDRAM, (MT_HANDLE)&pidram);
	sem_post(&pdevinfo->sem);
	*pid1 = (pidram.val>>16)&0x1fff;
	*pid0 = pidram.val&0x1fff;
	return ret;
}

MT_S32 mt_unf_ciplus_pidfiler_enable(MT_HANDLE pdevhandle,
		MT_S32 isenable)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_FILTEREN, (MT_HANDLE)&isenable);
	sem_post(&pdevinfo->sem);
	return ret;
}


MT_S32 mt_unf_ciplus_tsenable(MT_HANDLE pdevhandle,MT_S32 isenable)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_TSENABLE, (MT_HANDLE)&isenable);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_serial_enable(MT_HANDLE pdevhandle,MT_S32 isenable)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_SETPIDRAM, (MT_HANDLE)&isenable);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_tsin_clksel(MT_HANDLE pdevhandle,
		enum mtci_tsin_clk tsin_clk)
{
	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_TSINCLK, (MT_HANDLE)&tsin_clk);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_campower(MT_HANDLE pdevhandle,int ison)
{

	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_CAMPWR, (MT_HANDLE)&ison);
	sem_post(&pdevinfo->sem);
	return ret;

}

MT_S32 mt_unf_ciplus_regdump(MT_HANDLE pdevhandle)
{

	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_DUMPREG,0);
	sem_post(&pdevinfo->sem);
	return ret;
}

MT_S32 mt_unf_ciplus_iomem_switch(MT_HANDLE pdevhandle,int iospace)
{

	struct mtci_info *pdevinfo = (struct mtci_info*)pdevhandle;
	int ret;
	if (!pdevhandle)
		return MT_FAILURE;
	sem_wait(&pdevinfo->sem);
	ret = ioctl(pdevinfo->devfd, CI_IOC_IOMEMSWITCH, (MT_HANDLE)&iospace);
	sem_post(&pdevinfo->sem);
	return ret;

}

