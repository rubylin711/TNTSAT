/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

#include <linux/uaccess.h>

#include <uapi/asm-generic/ioctl.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_module.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"

#include "mt_module_debug.h"
#include "mt_otp_object.h"
#include "mt_unf_otp.h"

static mt_device_s otp_RegisterData;


DEFINE_MUTEX(otp_mutex);

#if 0
#define OTP_DATA_IN	_IOW('O',0,struct otp_transfer_data)
#define OTP_DATA_OUT	_IOR('O',0,struct otp_transfer_data)
#define OTP_DATA_UNLOCK	_IOR('O',1,struct otp_transfer_data)
#endif

#define REG_SYMPHONY_OTP_BASE                       ((void *)(0xbf308000))
#define REG_SYMPHONY_OTP_STOBE_TIME                 (REG_SYMPHONY_OTP_BASE + 0x1004)
#define REG_SYMPHONY_OTP_CTRL                       (REG_SYMPHONY_OTP_BASE + 0x1000)
#define SYMPHONY_OTP_CTRL_POR33_BIT                 0
#define SYMPHONY_OTP_CTRL_LOAD_BIT                  1
#define SYMPHONY_OTP_CTRL_PGENB_BIT                 2
#define SYMPHONY_OTP_CTRL_CSB_BIT                   3
#define SYMPHONY_OTP_CTRL_PS_BIT                    4
#define SYMPHONY_OTP_CTRL_PD_BIT                    5
#define SYMPHONY_OTP_CTRL_MR_BIT                    6
#define REG_SYMPHONY_OTP_WR_ADDR_BASE               (REG_SYMPHONY_OTP_BASE)
#define REG_SYMPHONY_OTP_WR_LAST                    (REG_SYMPHONY_OTP_WR_ADDR_BASE + 0x7FC)
#define REG_SYMPHONY_OTP_RD_ADDR_BASE               (REG_SYMPHONY_OTP_BASE + 0x800)
//add for version
int (*symphony_otp_read)(u32,u8,u32*) = NULL;
int (*symphony_otp_write)(u32,u8,u32) = NULL;
void (*symphony_otp_unlock_write)(u32) = NULL;

static unsigned long chip_rev = 0;
//static int symphony2_otp_read_raw(u32 bit_addr, u8 len, u32 *p_result)	//the four will be used
//static int symphony_otp_read_raw(u32 bit_addr, u8 len, u32 *p_result)
//static int symphony2_otp_write_raw(u32 bit_addr, u8 len, u32 val)
//static int symphony_otp_write_raw(u32 bit_addr, u8 len, u32 val)
//add end

//code for sym2 otp
#define REG_SYMPHONY2_OTP_BASE				((void *)(0xBF310000))
#define REG_SYMPHONY2_OTP_WR_ADDR_BASE               (REG_SYMPHONY2_OTP_BASE)
#define REG_SYMPHONY2_OTP_WR_LAST                    (REG_SYMPHONY2_OTP_WR_ADDR_BASE + 0x3FFC)
#define REG_SYMPHONY2_OTP_RD_ADDR_BASE               (REG_SYMPHONY2_OTP_BASE)
#define REG_SYMPHONY2_OTP_RD_LAST                    (REG_SYMPHONY2_OTP_RD_ADDR_BASE + 0x3FFC)

#define REG_SYMPHONY2_OTP_REF_PSW0                   (REG_SYMPHONY2_OTP_BASE + 0x4000)
#define REG_SYMPHONY2_OTP_REF_PSW1                   (REG_SYMPHONY2_OTP_BASE + 0x400C)
#define REG_SYMPHONY2_OTP_USR_PSW0                   (REG_SYMPHONY2_OTP_BASE + 0x4020)
#define REG_SYMPHONY2_OTP_USR_PSW1                   (REG_SYMPHONY2_OTP_BASE + 0x4030)
#define REG_SYMPHONY2_OTP_CLK_CTRL                   (REG_SYMPHONY2_OTP_BASE + 0x4040)
#define REG_SYMPHONY2_OTP_LOCK_MASK                  (REG_SYMPHONY2_OTP_BASE + 0x4044)
#define REG_SYMPHONY2_OTP_RW_LOCKED                  (REG_SYMPHONY2_OTP_BASE + 0x4048)
#define REG_SYMPHONY2_OTP_RW_SUCCESS                 (REG_SYMPHONY2_OTP_BASE + 0x404C)
#define REG_SYMPHONY2_OTP_CFG0                       (REG_SYMPHONY2_OTP_BASE + 0x4080)
#define REG_SYMPHONY2_OTP_CFG1                       (REG_SYMPHONY2_OTP_BASE + 0x40A0)
#define REG_SYMPHONY2_OTP_CFG2                       (REG_SYMPHONY2_OTP_BASE + 0x40C0)
#define REG_SYMPHONY2_OTP_CFG3                       (REG_SYMPHONY2_OTP_BASE + 0x40E0)
#define REG_SYMPHONY2_OTP_CFG4                       (REG_SYMPHONY2_OTP_BASE + 0x4100)
#define REG_SYMPHONY2_OTP_CFG5                       (REG_SYMPHONY2_OTP_BASE + 0x4120)
#define REG_SYMPHONY2_OTP_CFG6                       (REG_SYMPHONY2_OTP_BASE + 0x4140)
#define REG_SYMPHONY2_OTP_CFG7                       (REG_SYMPHONY2_OTP_BASE + 0x4160)

#define REG_SYMPHONY2_OTP_CHECKNUM0                  (REG_SYMPHONY2_OTP_BASE + 0x4180)
#define REG_SYMPHONY2_OTP_CHECKNUM1                  (REG_SYMPHONY2_OTP_BASE + 0x4184)
#define REG_SYMPHONY2_OTP_CHECKNUM2                  (REG_SYMPHONY2_OTP_BASE + 0x4188)
#define REG_SYMPHONY2_OTP_CHECKNUM3                  (REG_SYMPHONY2_OTP_BASE + 0x418C)
#define REG_SYMPHONY2_OTP_CHECKNUM4                  (REG_SYMPHONY2_OTP_BASE + 0x4190)
#define REG_SYMPHONY2_OTP_CHECKNUM5                  (REG_SYMPHONY2_OTP_BASE + 0x4194)
#define REG_SYMPHONY2_OTP_CHECKNUM6                  (REG_SYMPHONY2_OTP_BASE + 0x4198)
#define REG_SYMPHONY2_OTP_CHECKNUM7                  (REG_SYMPHONY2_OTP_BASE + 0x419C)
#define REG_SYMPHONY2_OTP_CHECKNUM8                  (REG_SYMPHONY2_OTP_BASE + 0x41A0)
#define REG_SYMPHONY2_OTP_CHECKNUM9                  (REG_SYMPHONY2_OTP_BASE + 0x41A4)
#define REG_SYMPHONY2_OTP_EFF_SCK_VALID              (REG_SYMPHONY2_OTP_BASE + 0x41A8)


#define REG_SYMPHONY4_OTP_WRITEENABLE				(0x4400)
#define SYMPHONY4_OTP_WRITE_VAILD					(0x33250792)
#define REG_SYMPHONY4_OTP_LOWPOWER					(0x4500)


#define PROC_PARAM_MAXLEN (256)


#define ACPU_MB_MEMLOCK_REG(chan) (0xBF124200 + (chan) * 4)
#define AP_AV_SPINLOCK(chan)      (0xbf128200 + (chan*4))

#ifndef READ_REG32
#define READ_REG32(r)			HAL_GET_U32((volatile u32 *)((ulong)r))
#define WRITE_REG32(v, r)		HAL_PUT_U32((volatile u32 *)((ulong)r), (u32)(v))
#endif

typedef enum {
	T_MEMLOCK_SPINLOCK = 0,
	T_MSGLOCK_SPINLOCK = 1,
	T_OTPLOCK_SPINLOCK = 4,
	T_MEMLOCK_MAX = 32,
} en_memlock;

static unsigned char g_wflag = 0;
static unsigned long g_otpbaseaddr = 0;

/*
 * once you wanna access shared memory, memlock for that shared memory must be held in hand first.
 * wait forever, if memlock is not free now
 */
static unsigned int symphony_mb_get_memlock(unsigned int chan)
{
	if ((chan > 31))
		return 1;

	/* get lock before access shared memory */
	while (1 == (READ_REG32(ACPU_MB_MEMLOCK_REG(chan)) & 0x1));
	while (1 == (READ_REG32(AP_AV_SPINLOCK(chan)) & 0x1));

	return 0;
}

/*
 * after accessing shared memory, this API must be called to release memlock for that shared memory
 */
static void symphony_mb_release_memlock(unsigned int chan)
{
	if ((chan > 31))
		return;

	WRITE_REG32(1,ACPU_MB_MEMLOCK_REG(chan));
	WRITE_REG32(1,AP_AV_SPINLOCK(chan));
}

#if 0
void symphony2_otp_unlock_write_protect(u32 bit_addr)
{
	u32 pparea0 = 0;
	u32 reg = 0;

	symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
	reg = READ_REG32(0xbf308030);
	reg |= 0x1;
	WRITE_REG32(reg,0xbf308030);
	pparea0 = READ_REG32(0xbf313bb8);

	if (pparea0 == 0xcfff4080)
	{
		if(bit_addr > 4096)
		{
			WRITE_REG32(0x90c5db66, (void *)0xbf314020);
			WRITE_REG32(0x654dacee, (void *)0xbf314024);
			WRITE_REG32(0x2c83115c, (void *)0xbf314028);
			WRITE_REG32(0x0093b7cb, (void *)0xbf31402c);
		}
	}
	else if (pparea0 == 0xcfffc000)
	{
		WRITE_REG32(0x19270340, (void *)0xbf314020);
		WRITE_REG32(0x42001943, (void *)0xbf314024);
		WRITE_REG32(0xf3442808, (void *)0xbf314028);
		WRITE_REG32(0x20497802, (void *)0xbf31402c);
	}

	reg = READ_REG32(0xbf308030);
	reg &= ~0x1;
	WRITE_REG32(reg,0xbf308030);
	symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
}
#endif
void symphony2_otp_unlock_write_protect(u32 bit_addr)
{
	u32 pparea0 = 0, pparea1 = 0, hkmode = 0;
	u32 reg = 0;

	symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
	reg = READ_REG32(0xbf308030);
	reg |= 0x1;
	WRITE_REG32(reg,0xbf308030);

	pparea0 = READ_REG32(0xbf313bb8);
	pparea1 = READ_REG32(0xbf313bbc);
	hkmode = (READ_REG32(0xbf313bf8) >> 6) & 0x3;

	// 根据OTP_PPArea0判断密码来源
	if ((pparea0 == 0xcfff4080) || (pparea0 == 0xcfff4000))
	{
		WRITE_REG32(0x90c5db66, (void *)0xbf314020);
		WRITE_REG32(0x654dacee, (void *)0xbf314024);
		WRITE_REG32(0x2c83115c, (void *)0xbf314028);
		WRITE_REG32(0x0093b7cb, (void *)0xbf31402c);
	}
	else if ((pparea0 == 0xcf9cc000) && (pparea1 == 0xcfffcf9e))
	{
		if (hkmode)
		{
			WRITE_REG32(0x19270340, (void *)0xbf314020);
			WRITE_REG32(0x42001943, (void *)0xbf314024);
			WRITE_REG32(0xf3442808, (void *)0xbf314028);
			WRITE_REG32(0x20497802, (void *)0xbf31402c);
			WRITE_REG32(0x5784ed31, (void *)0xbf314030);
			WRITE_REG32(0x19462789, (void *)0xbf314034);
			WRITE_REG32(0xf221e306, (void *)0xbf314038);
			WRITE_REG32(0x14330618, (void *)0xbf31403c);
		}
		else
		{
			WRITE_REG32(0x25195589, (void *)0xbf314020);
			WRITE_REG32(0x25792804, (void *)0xbf314024);
			WRITE_REG32(0x33254408, (void *)0xbf314028);
			WRITE_REG32(0x12345678, (void *)0xbf31402c);
			WRITE_REG32(0x13571819, (void *)0xbf314030);
			WRITE_REG32(0x372d47a0, (void *)0xbf314034);
			WRITE_REG32(0x2821110f, (void *)0xbf314038);
			WRITE_REG32(0x24793310, (void *)0xbf31403c);
		}
	}
	else if (pparea0 == 0xcfffc000)
	{
		if (hkmode)
		{
			WRITE_REG32(0x19270340, (void *)0xbf314020);
			WRITE_REG32(0x42001943, (void *)0xbf314024);
			WRITE_REG32(0xf3442808, (void *)0xbf314028);
			WRITE_REG32(0x20497802, (void *)0xbf31402c);
		}
		else
		{
			WRITE_REG32(0x25195589, (void *)0xbf314020);
			WRITE_REG32(0x25792804, (void *)0xbf314024);
			WRITE_REG32(0x33254408, (void *)0xbf314028);
			WRITE_REG32(0x12345678, (void *)0xbf31402c);
		}
	}

	reg = READ_REG32(0xbf308030);
	reg &= ~0x1;
	WRITE_REG32(reg,0xbf308030);
	symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);

	return;
}


int symphony2_otp_avcpu_enable(void)
{
	u32 reg = 0;
	symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
	reg = READ_REG32(0xbf308030);
	reg |= 0x1;
	WRITE_REG32(reg,0xbf308030);
	if (READ_REG32(0xbf313a84) == 0x01ff0001)
	{
		if (((READ_REG32(0xbf313a8c) >> 8) & 0xf) != 0xf)
		{
			reg = READ_REG32(0xbf308030);
			reg &= ~0x1;
			reg |= 0x2;
			WRITE_REG32(reg,0xbf308030);
			symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
			return 0;
		}
	}
	reg = READ_REG32(0xbf308030);
	reg &= ~0x1;
	WRITE_REG32(reg,0xbf308030);
	symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	return -1;
}

int symphony2_otp_avcpu_disable(void)
{
	u32 reg = 0;
	symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
	reg = READ_REG32(0xbf308030);
	reg |= 0x1;
	WRITE_REG32(reg,0xbf308030);
	if (READ_REG32 (0xbf313a84) == 0x01ff0001)
	{
		if (((READ_REG32(0xbf313a8c) >> 8) & 0xf) != 0xf)
		{
			reg = READ_REG32(0xbf308030);
			reg &= ~0x3;
			WRITE_REG32(reg,0xbf308030);
			symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
			return 0;
		}
	}
	reg = READ_REG32(0xbf308030);
	reg &= ~0x1;
	WRITE_REG32(reg,0xbf308030);
	symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	return -1;
}


static int symphony2_otp_read_data(u32 bit_addr,  u32 *p_result)	//guess the read raw func
{
	void *reg = 0;
	unsigned int val1 = 0;
	unsigned int val2 = 0;
	unsigned int val3 = 0;

	if(bit_addr >= 4096)
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x\n", bit_addr);
		return -1;
	}

	reg = REG_SYMPHONY2_OTP_RD_ADDR_BASE + (bit_addr << 2);
	mutex_lock(&otp_mutex);
	*p_result = READ_REG32(reg);
	val1 = READ_REG32(reg);
	val2 = READ_REG32(reg);
	val3 = READ_REG32(reg);
	mutex_unlock(&otp_mutex);

	if((*p_result == val1) && (*p_result == val2) && (*p_result == val3))
	{
		return 0;
	}
	return -1;
}

static int symphony2_otp_write_data(u32 bit_addr, u32 val)		//raw write func
{
	void *reg = 0;

	if(bit_addr >= 4096)
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x\n", bit_addr);
		return -1;
	}

	if (0 == val)
	{
		MT_INFO_OTP("reg = 0x%x, bit_addr = 0x%x, val = 0x%x \n",reg, bit_addr, val);
		return 0;
	}

	reg = REG_SYMPHONY2_OTP_WR_ADDR_BASE + (bit_addr << 2);
	mutex_lock(&otp_mutex);
	WRITE_REG32(val, reg);
	mutex_unlock(&otp_mutex);

	return 0;
}


static int symphony2_otp_read_raw(u32 bit_addr, u8 len, u32 *p_result)	//sym2 call
{
	u32 i = 0;
	u32 n = 0;
	u32 val = 0;
	u32 reg = 0;
	int ret = 0;

	i = bit_addr & 0x1f;
	n = len;

	if ((i+n) >  32)
	{
		MT_ERR_OTP("\n bitnum  %lu   bitval  0x%lx   over 32 bit word aligned failed \n ", bit_addr, i);
		return -1;
	}
	else
	{
		//config the firewall before the otp read/write and after
		symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
		reg = READ_REG32(0xbf308030);
		reg |= 0x1;
		WRITE_REG32(reg,0xbf308030);
		ret = symphony2_otp_read_data(bit_addr >> 5, &val);	//bit to 4byte
		reg = READ_REG32(0xbf308030);
		reg &= ~0x1;
		WRITE_REG32(reg,0xbf308030);
		symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	}

	if(-1 == ret)
	{
		MT_ERR_OTP("\n bitnum  %lu   bitval  0x%lx read 3 times failed \n ", bit_addr, i);
		return -1;
	}

	if (len < 32 )
	{
		*p_result = (val >> i) & ((1<<len) -1);
	}
	else
	{
		*p_result = val;
	}
	return 0;
}

static int symphony2_otp_write_raw(u32 bit_addr, u8 len, u32 val)
{
	u32 i = 0;
	u32 n = 0;
	u32 reg = 0;
	int rc = 0;
	i = bit_addr & 0x1f;
	n = len;

	if ((i+n) >  32)
	{
		MT_ERR_OTP("\n write  %lu   bitval  0x%lx   over 32 bit word aligned failed \n ", bit_addr, i);
		return -1;
	}
	else
	{
		if (len < 32)
		{
			val = val&((1<<len) -1);
			val = val << i;
		}
		//fta new features , need to  Unlock PPArea0 when write > 4096 area,the value from hardware
		//symphony2_otp_unlock_write_protect(bit_addr);
		//config the firewall before the otp read/write and after
		symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
		reg = READ_REG32(0xbf308030);
		reg |= 0x1;
		WRITE_REG32(reg,0xbf308030);
		rc = symphony2_otp_write_data(bit_addr >> 5, val);
		reg = READ_REG32(0xbf308030);
		reg &= ~0x1;
		WRITE_REG32(reg,0xbf308030);
		symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	}

	return rc;
}


void symphony4_otp_unlock_write_protect(u32 bit_addr)
{
	u32 hk_mode;
	u32 pparea0_pwd;
	u32 pparea1_pwd;

	hk_mode = READ_REG32(SYMPHONY_IO_VA(0xbf313a7c)) & 0x3;
	pparea0_pwd = (READ_REG32(SYMPHONY_IO_VA(0xbf313a5c)) >> 14) & 0x3;
	pparea1_pwd = (READ_REG32(SYMPHONY_IO_VA(0xbf313a60)) >> 14) & 0x3;

	/* Unlock AHB lock. */
	WRITE_REG32(0x33250792, (void *)(SYMPHONY_IO_VA(0xbf314400)));

	/* Unlock PPArea0 if necessary. */
	switch (pparea0_pwd)
	{
	case 0x1:
		if (hk_mode)
		{
			WRITE_REG32(0x19270340, (void *)(SYMPHONY_IO_VA(0xbf314020)));
			WRITE_REG32(0x42001943, (void *)(SYMPHONY_IO_VA(0xbf314024)));
			WRITE_REG32(0xf3442808, (void *)(SYMPHONY_IO_VA(0xbf314028)));
			WRITE_REG32(0x20497802, (void *)(SYMPHONY_IO_VA(0xbf31402c)));
		}
		else
		{
			WRITE_REG32(0x25195589, (void *)(SYMPHONY_IO_VA(0xbf314020)));
			WRITE_REG32(0x25792804, (void *)(SYMPHONY_IO_VA(0xbf314024)));
			WRITE_REG32(0x33254408, (void *)(SYMPHONY_IO_VA(0xbf314028)));
			WRITE_REG32(0x12345678, (void *)(SYMPHONY_IO_VA(0xbf31402c)));
		}
		break;
	case 0x2:
		if (hk_mode)
		{
			WRITE_REG32(0x27420791, (void *)(SYMPHONY_IO_VA(0xbf314020)));
			WRITE_REG32(0x12a31856, (void *)(SYMPHONY_IO_VA(0xbf314024)));
			WRITE_REG32(0xf2057973, (void *)(SYMPHONY_IO_VA(0xbf314028)));
			WRITE_REG32(0x332589e2, (void *)(SYMPHONY_IO_VA(0xbf31402c)));
		}
		else
		{
			WRITE_REG32(0x53192714, (void *)(SYMPHONY_IO_VA(0xbf314020)));
			WRITE_REG32(0x2589af04, (void *)(SYMPHONY_IO_VA(0xbf314024)));
			WRITE_REG32(0x33278408, (void *)(SYMPHONY_IO_VA(0xbf314028)));
			WRITE_REG32(0x12355678, (void *)(SYMPHONY_IO_VA(0xbf31402c)));
		}
		break;
	case 0x3:
		if (hk_mode)
		{
			WRITE_REG32(0x39272233, (void *)(SYMPHONY_IO_VA(0xbf314020)));
			WRITE_REG32(0x320b0985, (void *)(SYMPHONY_IO_VA(0xbf314024)));
			WRITE_REG32(0xf3444214, (void *)(SYMPHONY_IO_VA(0xbf314028)));
			WRITE_REG32(0x478218f2, (void *)(SYMPHONY_IO_VA(0xbf31402c)));
		}
		else
		{
			WRITE_REG32(0x10194203, (void *)(SYMPHONY_IO_VA(0xbf314020)));
			WRITE_REG32(0x1579cd04, (void *)(SYMPHONY_IO_VA(0xbf314024)));
			WRITE_REG32(0x63257408, (void *)(SYMPHONY_IO_VA(0xbf314028)));
			WRITE_REG32(0x12365678, (void *)(SYMPHONY_IO_VA(0xbf31402c)));
		}
		break;
	default:
		break;
	}

	/* Unlock PPArea1 if necessary. */
	switch (pparea1_pwd)
	{
	case 0x1:
		if (hk_mode)
		{
			WRITE_REG32(0x5784ed31, (void *)(SYMPHONY_IO_VA(0xbf314030)));
			WRITE_REG32(0x19462789, (void *)(SYMPHONY_IO_VA(0xbf314034)));
			WRITE_REG32(0xf221e306, (void *)(SYMPHONY_IO_VA(0xbf314038)));
			WRITE_REG32(0x14330618, (void *)(SYMPHONY_IO_VA(0xbf31403c)));
		}
		else
		{
			WRITE_REG32(0x13571819, (void *)(SYMPHONY_IO_VA(0xbf314030)));
			WRITE_REG32(0x372d47a0, (void *)(SYMPHONY_IO_VA(0xbf314034)));
			WRITE_REG32(0x2821110f, (void *)(SYMPHONY_IO_VA(0xbf314038)));
			WRITE_REG32(0x24793310, (void *)(SYMPHONY_IO_VA(0xbf31403c)));
		}
		break;
	case 0x2:
		if (hk_mode)
		{
			WRITE_REG32(0x40623927, (void *)(SYMPHONY_IO_VA(0xbf314030)));
			WRITE_REG32(0x02211368, (void *)(SYMPHONY_IO_VA(0xbf314034)));
			WRITE_REG32(0x3507f03e, (void *)(SYMPHONY_IO_VA(0xbf314038)));
			WRITE_REG32(0x29034695, (void *)(SYMPHONY_IO_VA(0xbf31403c)));
		}
		else
		{
			WRITE_REG32(0x16271542, (void *)(SYMPHONY_IO_VA(0xbf314030)));
			WRITE_REG32(0x53dd87af, (void *)(SYMPHONY_IO_VA(0xbf314034)));
			WRITE_REG32(0x2921e10f, (void *)(SYMPHONY_IO_VA(0xbf314038)));
			WRITE_REG32(0xf4793510, (void *)(SYMPHONY_IO_VA(0xbf31403c)));
		}
		break;
	case 0x3:
		if (hk_mode)
		{
			WRITE_REG32(0x0189f324, (void *)(SYMPHONY_IO_VA(0xbf314030)));
			WRITE_REG32(0x10461276, (void *)(SYMPHONY_IO_VA(0xbf314034)));
			WRITE_REG32(0x42389980, (void *)(SYMPHONY_IO_VA(0xbf314038)));
			WRITE_REG32(0x14820971, (void *)(SYMPHONY_IO_VA(0xbf31403c)));
		}
		else
		{
			WRITE_REG32(0x33501761, (void *)(SYMPHONY_IO_VA(0xbf314030)));
			WRITE_REG32(0x3d1d4930, (void *)(SYMPHONY_IO_VA(0xbf314034)));
			WRITE_REG32(0x38211b0f, (void *)(SYMPHONY_IO_VA(0xbf314038)));
			WRITE_REG32(0x14783f10, (void *)(SYMPHONY_IO_VA(0xbf31403c)));
		}
		break;
	default:
		break;
	}
}

//bit_addr: dword number
static int symphony4_otp_read_data(u32 bit_addr,  u32 *p_result)	//guess the read raw func
{
	unsigned long reg = 0;
	unsigned int val1 = 0;
	unsigned int val2 = 0;
	unsigned int val3 = 0;

	BUG_ON(p_result == NULL);

	if(bit_addr >= 4096)
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x\n", bit_addr);
		return -1;
	}

	//
	// bit_addr << 2
	// = bit_addr * 4
	//   ==> byte offset
	//
	reg = (g_otpbaseaddr + (bit_addr << 2));
	mutex_lock(&otp_mutex);
	*p_result = READ_REG32(reg);
	val1 = READ_REG32(reg);
	val2 = READ_REG32(reg);
	val3 = READ_REG32(reg);
	mutex_unlock(&otp_mutex);

	if((*p_result == val1) && (*p_result == val2) && (*p_result == val3))
	{
		return 0;
	}

	MT_FATAL_OTP("otp read err!!! bit_addr: 0x%x, result 0x%x, val1 0x%x, val2 0x%x, val3 0x%x\n",
		bit_addr,
		*p_result,
		val1, val2, val3);

	return -1;
}

//bit_addr: dword number
static int symphony4_otp_write_data(u32 bit_addr, u32 val)		//raw write func
{
	unsigned long reg = 0;

	if(bit_addr >= 4096)
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x\n", bit_addr);
		return -1;
	}

	if (0 == val)
	{
		MT_INFO_OTP("reg = 0x%x, bit_addr = 0x%x, val = 0x%x \n",reg, bit_addr, val);
		return 0;
	}

	//
	// bit_addr << 2
	// = bit_addr * 4
	//   ==> byte offset
	//
	reg = g_otpbaseaddr + (bit_addr << 2);
	mutex_lock(&otp_mutex);
	WRITE_REG32(SYMPHONY4_OTP_WRITE_VAILD,g_otpbaseaddr+REG_SYMPHONY4_OTP_WRITEENABLE);
	WRITE_REG32(val, reg);
	WRITE_REG32(1,g_otpbaseaddr+REG_SYMPHONY4_OTP_WRITEENABLE);
	mutex_unlock(&otp_mutex);

	return 0;
}

//bit_addr: bit offset
static int symphony4_otp_read_raw(u32 bit_addr, u8 len, u32 *p_result)	//sym2 call
{
	u32 i = 0;
	u32 n = 0;
	u32 val = 0;
	int ret = 0;

	BUG_ON(p_result == NULL);
	BUG_ON(len == 0);

	i = bit_addr & 0x1f;
	n = len;

	if ((i+n) >  32)
	{
		MT_ERR_OTP("\n bitnum  %lu   bitval  0x%lx   over 32 bit word aligned failed \n ", bit_addr, i);
		return -1;
	}
	else
	{
    	//
    	// bit_addr >> 5
    	// = bit_addr / 32
    	//   ==> bit address to dword number
    	//
		ret = symphony4_otp_read_data(bit_addr >> 5, &val);	//bit to 4byte
	}
	if(-1 == ret)
	{
		MT_ERR_OTP("\n bitnum  %lu   bitval  0x%lx read 3 times failed \n ", bit_addr, i);
		return -1;
	}
	if (len < 32 )
	{
		*p_result = (val >> i) & ((1<<len) -1);
	}
	else
	{
		*p_result = val;
	}
	return 0;
}

//bit_addr: bit offset
static int symphony4_otp_write_raw(u32 bit_addr, u8 len, u32 val)
{
	u32 i = 0;
	u32 n = 0;
	//u32 reg = 0;
	int rc = 0;
	i = bit_addr & 0x1f;
	n = len;

	BUG_ON(len == 0);

	if ((i+n) >  32)
	{
		MT_ERR_OTP("\n write  %lu   bitval  0x%lx   over 32 bit word aligned failed \n ", bit_addr, i);
		return -1;
	}
	else
	{
		if (len < 32)
		{
			val = val&((1<<len) -1);
			val = val << i;
		}
		//fta new features , need to  Unlock PPArea0 when write > 4096 area,the value from hardware
		//symphony2_otp_unlock_write_protect(bit_addr);
		//config the firewall before the otp read/write and after

		//symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
		//reg = READ_REG32(0xbf308030);
		//reg |= 0x1;
		//WRITE_REG32(reg,0xbf308030);

    	//
    	// bit_addr >> 5
    	// = bit_addr / 32
    	//   ==> bit address to dword number
    	//
		rc = symphony4_otp_write_data(bit_addr >> 5, val);
		//reg = READ_REG32(0xbf308030);
		//reg &= ~0x1;
		//WRITE_REG32(reg,0xbf308030);
		//symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	}

	return rc;
}



//TODO	sometimes the add some functions
#if 0
static RET_CODE symphony_otp_read_byte(u32 byte_addr, u32 len, u8 *p_result)
{

    int rc = SUCCESS;
    u32 reg;

    otp_con_priv_t *p_priv = hal_find_module(SYS_HAL_TYPE_OTP);

    if ((byte_addr + len) > 4096*4)
    {
        return ERR_PARAM;
    }

    mtos_sem_take(&p_priv->mutex, 0);
    reg = REG_SYMPHONY2_OTP_RD_ADDR_BASE + byte_addr;

    while (len--)
    {
        *p_result =  REG8(reg);
        reg++;
        p_result++;
    }


   // symphony_otp_read_close();
    mtos_sem_give(&p_priv->mutex);


    return rc;
}

static RET_CODE symphony_otp_write_byte(u32 byte_addr, u32 len, u8 *p_value)
{


    int rc = SUCCESS;
    u32 reg;

    otp_con_priv_t *p_priv = hal_find_module(SYS_HAL_TYPE_OTP);

    if ((byte_addr + len) > 4096*4)
    {
        return ERR_PARAM;
    }

    mtos_sem_take(&p_priv->mutex, 0);
    reg = REG_SYMPHONY2_OTP_RD_ADDR_BASE + byte_addr;

    while (len--)
    {
        REG8(reg) = *p_value;
        if ((*p_value) !=  REG8(reg))
        {
          // rc = ERR_FAILURE;
        }
        p_value++;
        reg++;
    }


   // symphony_otp_read_close();
    mtos_sem_give(&p_priv->mutex);


    return rc;
}
#endif
//code for sym2 otp end


#if 0
struct otp_transfer_data {
	int bit_addr;
	int bit_len;
	int buf_size;
	void * buf;
	unsigned long version;
};
#endif

static void symphony_otp_timer_init(void)
{
	WRITE_REG32(0x10f63,REG_SYMPHONY_OTP_STOBE_TIME);
}




void symphony_otp_write_init(void)
{
	WRITE_REG32(0x00,REG_SYMPHONY_OTP_CTRL);

	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) | (1 << SYMPHONY_OTP_CTRL_POR33_BIT),REG_SYMPHONY_OTP_CTRL);
    	ndelay(2400);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) & ~(1 << SYMPHONY_OTP_CTRL_PD_BIT),REG_SYMPHONY_OTP_CTRL);
    	ndelay(1200);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) | (1 << SYMPHONY_OTP_CTRL_PS_BIT),REG_SYMPHONY_OTP_CTRL);
    	ndelay(120);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) & ~((1 << SYMPHONY_OTP_CTRL_CSB_BIT)|(1 << SYMPHONY_OTP_CTRL_PGENB_BIT)|(1 << SYMPHONY_OTP_CTRL_LOAD_BIT)),REG_SYMPHONY_OTP_CTRL);
    	ndelay(20);
}

void symphony_otp_write_deinit(void)
{
	ndelay(20);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) | ((1 << SYMPHONY_OTP_CTRL_CSB_BIT)|(1 << SYMPHONY_OTP_CTRL_PGENB_BIT)|(1 << SYMPHONY_OTP_CTRL_LOAD_BIT)),REG_SYMPHONY_OTP_CTRL);
	ndelay(120);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) & ~(1 << SYMPHONY_OTP_CTRL_PS_BIT),REG_SYMPHONY_OTP_CTRL);
	ndelay(20);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) | (1 << SYMPHONY_OTP_CTRL_PD_BIT),REG_SYMPHONY_OTP_CTRL);
	ndelay(2400);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) & ~(1 << SYMPHONY_OTP_CTRL_POR33_BIT),REG_SYMPHONY_OTP_CTRL);

}

void symphony_otp_read_init(void)
{
	WRITE_REG32(0x08,REG_SYMPHONY_OTP_CTRL);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) & ~((1 << SYMPHONY_OTP_CTRL_MR_BIT)|(1 << SYMPHONY_OTP_CTRL_PD_BIT)|(1 << SYMPHONY_OTP_CTRL_PS_BIT)),REG_SYMPHONY_OTP_CTRL);
	ndelay(1200);
	WRITE_REG32((READ_REG32(REG_SYMPHONY_OTP_CTRL) | (1 << SYMPHONY_OTP_CTRL_PGENB_BIT)|(1 << SYMPHONY_OTP_CTRL_LOAD_BIT))& ~(1 << SYMPHONY_OTP_CTRL_CSB_BIT),REG_SYMPHONY_OTP_CTRL);
    ndelay(200);
}

void symphony_otp_read_deinit(void)
{
	ndelay(200);
	WRITE_REG32((READ_REG32(REG_SYMPHONY_OTP_CTRL) | (1 << SYMPHONY_OTP_CTRL_CSB_BIT)) & ~((1 << SYMPHONY_OTP_CTRL_PGENB_BIT) | (1 << SYMPHONY_OTP_CTRL_LOAD_BIT)) ,REG_SYMPHONY_OTP_CTRL);
	ndelay(1200);
	WRITE_REG32(READ_REG32(REG_SYMPHONY_OTP_CTRL) | ((1 << SYMPHONY_OTP_CTRL_PD_BIT)|(1 << SYMPHONY_OTP_CTRL_PS_BIT)),REG_SYMPHONY_OTP_CTRL);
}

u8 symphony_otp_read_data(ulong reg)
{
	return READ_REG32((void *)reg) & 0xFF;
}


int  symphony_otp_write_data(ulong bit_addr, u32 data)
{
	ulong reg = 0;
	u8  addr_shift = 0;

	if(bit_addr >= 4096)
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x\n",bit_addr);
		return -1;
	}

	addr_shift = bit_addr % 8;
	reg = (ulong)REG_SYMPHONY_OTP_WR_ADDR_BASE + ((bit_addr >> 3) << 2);

	WRITE_REG32(((addr_shift << 4) | (data & 0x01)),(void *)reg);

	return 0;
}




static int symphony_otp_read_raw(u32 bit_addr, u8 len, u32 *p_result)
{
	u8 before = 0;
	u8 after = 0;
	u8 dtmp[5] = {0};
	u8 cnt = 0;
	ulong start_reg = 0;
	ulong end_reg = 0;

	if((bit_addr >= 4096) || (bit_addr + len - 1 >= 4096) || (len == 0) || (len > 32))
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x, len:%d\n",bit_addr, len);
		return -1;
	}

	before = bit_addr % 8;
	after = (bit_addr + len - 1) % 8;

	start_reg = (ulong)REG_SYMPHONY_OTP_RD_ADDR_BASE + ((bit_addr >> 3) << 2);
	end_reg = (ulong)REG_SYMPHONY_OTP_RD_ADDR_BASE + (((bit_addr + len - 1) >> 3) << 2);

	mutex_lock(&otp_mutex);
	symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
	symphony_otp_read_init();

	while(start_reg <= end_reg)
	{
		dtmp[cnt] = symphony_otp_read_data(start_reg);
		start_reg += 4;
		cnt++;
	}

	symphony_otp_read_deinit();
	symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	mutex_unlock(&otp_mutex);

	switch(cnt)
	{
		case 1:
			*p_result = ((dtmp[0] >> before) & ((1 << len) - 1));
			break;

		case 2:
			*p_result = (dtmp[0] >> before);
			*p_result |= ((dtmp[1] & ((1 << (after + 1)) - 1)) << (8 - before));
			break;

		case 3:
			*p_result = (dtmp[0] >> before);
			*p_result |= (dtmp[1] << (8 - before));
			*p_result |= ((dtmp[2] & ((1 << (after + 1)) - 1)) << (16 - before));
			break;

		case 4:
			*p_result = (dtmp[0] >> before);
			*p_result |= (dtmp[1] << (8 - before));
			*p_result |= (dtmp[2] << (16 - before));
			*p_result |= ((dtmp[3] & ((1 << (after + 1)) - 1)) << (24 - before));
			break;

		case 5:
			*p_result = (dtmp[0] >> before);
			*p_result |= (dtmp[1] << (8 - before));
			*p_result |= (dtmp[2] << (16 - before));
			*p_result |= (dtmp[3] << (24 - before));
			*p_result |= ((dtmp[4] & ((1 << (after + 1)) - 1)) << (32 - before));
			break;

		default:
			break;
	}
	return 0;
}



static int symphony_otp_write_raw(u32 bit_addr, u8 len, u32 val)
{
	u32 i = 0;

	if((bit_addr >= 4096) || (bit_addr + len - 1 >= 4096) || (len == 0) || (len > 32))
	{
		MT_ERR_OTP("parameter err!!! bit_addr: 0x%x, len:%d\n",bit_addr, len);
		return -1;
	}

	mutex_lock(&otp_mutex);
	symphony_mb_get_memlock(T_OTPLOCK_SPINLOCK);
	symphony_otp_write_init();
	for(i = 0; i < len; i ++)
	{
		if((val >> i) & 0x01)
		{
			symphony_otp_write_data(bit_addr + i, ((val >> i) & 0x01));
		}
	}
	symphony_otp_write_deinit();
	symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
	mutex_unlock(&otp_mutex);
	return 0;
}



ssize_t otp_read (struct file * file , char __user *buf , size_t size , loff_t * pos)
{
	return 0;
}
ssize_t otp_write (struct file * file , const char __user *buf , size_t size , loff_t *pos)
{
	return 0;
}

int otp_open(struct inode * inode, struct file *file)
{
	unsigned int val = 0;
	chip_rev = symphony_get_chip_rev();
	switch(chip_rev)
	{
		case CHIP_SYMPHONY_A0:
		case CHIP_SYMPHONY_A1:
		case CHIP_SYMPHONY_A2:
		case CHIP_SYMPHONY3_A0:
			symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
			symphony_otp_read = symphony_otp_read_raw;
			symphony_otp_write = symphony_otp_write_raw;
			symphony_otp_timer_init();
			//printk("sym1 \n");
			break;
		case CHIP_SYMPHONY2_A0:
		case CHIP_SYMPHONY2_A1:
		case CHIP_SYMPHONY2_A2:
		case CHIP_SYMPHONY2_A3:
			symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
			symphony_otp_read = symphony2_otp_read_raw;
			symphony_otp_write = symphony2_otp_write_raw;
			symphony_otp_unlock_write = symphony2_otp_unlock_write_protect;
			//printk("sym2 \n");
			break;
		case CHIP_SYMPHONY4_A0:
		case CHIP_SYMPHONY4_A1:
			symphony_otp_read = symphony4_otp_read_raw;
			symphony_otp_write = symphony4_otp_write_raw;
			symphony_otp_unlock_write = symphony4_otp_unlock_write_protect;
			g_otpbaseaddr = mt_get_otp_base();
			val = READ_REG32(g_otpbaseaddr+REG_SYMPHONY4_OTP_LOWPOWER);
			val = val & (~1U);
			WRITE_REG32(val,(g_otpbaseaddr+REG_SYMPHONY4_OTP_LOWPOWER));
			//printk("sym4 \n");
			break;
		default:
		{
			if (chip_rev >= CHIP_SYMPHONY6_A0 && chip_rev < CHIP_SYMPHONY6_MAX)
			{
				symphony_otp_read = symphony4_otp_read_raw;
				symphony_otp_write = symphony4_otp_write_raw;
				symphony_otp_unlock_write = symphony4_otp_unlock_write_protect;
				g_otpbaseaddr = mt_get_otp_base();
				val = READ_REG32(g_otpbaseaddr+REG_SYMPHONY4_OTP_LOWPOWER);
				val = val & (~1U);
				WRITE_REG32(val,(g_otpbaseaddr+REG_SYMPHONY4_OTP_LOWPOWER));
				//printk("sym6 \n");
				break;
			}
			else
			{
				MT_ERR_OTP("wrong chip version\n");
				return -ENXIO;
			}
		}
	}
        return 0;
}

int otp_close (struct inode *inode, struct file *file )
{
    return 0;
}

int sys_otp_read(u32 bit_addr, u8 len, u32 *p_result)
{
	if(0 == chip_rev)
	{
		otp_open(NULL,NULL);
	}
	if(symphony_otp_read)
	{
		symphony_otp_read(bit_addr,len,p_result);
	}
	else
	{
		return -1;
	}
	return MT_SUCCESS;
}

static long otp_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
	struct otp_transfer_data data;
	int res = -1;
	char tmp[8];	/* FIXME */
	int tmp_data;
	if(NULL == (void *)args)
		return -EINVAL;
	if(copy_from_user(&data,(void *)args,sizeof(data)))
		return -EINVAL;
	if((NULL == (void *)data.buf) || (data.bit_len >32) || (8*data.buf_size < data.bit_len))
		return -EINVAL;
	switch(cmd)
	{
		case OTP_DATA_OUT :
			res = symphony_otp_read(data.bit_addr,data.bit_len,(u32 *)tmp);
			if(res)
				return res;
			data.version = (int)chip_rev;
			//FIXME: data.buf_size <= sizeof(tmp)
			res = copy_to_user((void __user *)data.buf,(void *)tmp,data.buf_size);
			if(res)
				return -EINVAL;
			res = copy_to_user((void __user *)args,&data,sizeof(data));
			if(res)
				return -EINVAL;
		break;

		case OTP_DATA_IN :
			//FIXME: data.buf_size <= sizeof(tmp_data)
			res = copy_from_user((void *)&tmp_data,(void __user *)data.buf,data.buf_size);
			if(res)
				return -EINVAL;
			res = symphony_otp_write(data.bit_addr,data.bit_len,tmp_data);
			if(res)
				return res;
		break;
		case OTP_DATA_UNLOCK:
			//FIXME: data.buf_size <= sizeof(tmp_data)
			//tmp_data not used!
			res = copy_from_user((void *)&tmp_data,(void __user *)data.buf,data.buf_size);
			if(res)
				return -EINVAL;
			if(symphony_otp_unlock_write)
			{
				symphony_otp_unlock_write(data.bit_addr);
			}
		break;
		default:
			return -EINVAL;
		break;
	}

	return 0;
}

static baseops_s otp_drvops =
{
	.probe        = NULL,
	.remove       = NULL,
	.shutdown     = NULL,
	.prepare      = NULL,
	.complete     = NULL,
	.suspend      = NULL,
	.suspend_late = NULL,
	.resume_early = NULL,
	.resume       = NULL,
};


struct file_operations OTP_FOPS = {
	.owner          = THIS_MODULE,
	.open           = otp_open,
	.release        = otp_close,
	.read           = otp_read,
	.write          = otp_write,
	.unlocked_ioctl = otp_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = otp_ioctl,
#endif
};


static mt_void otp_prochelp(mt_void)
{
	mt_drv_proc_echohelp("otp proc usage:\n");
	mt_drv_proc_echohelp("echo o > /proc/mt/msp/otp  -- open otp dev\n");
	mt_drv_proc_echohelp("echo e 1 > /proc/mt/msp/otp  -- open write enable\n");
	mt_drv_proc_echohelp("echo r bitaddr length > /proc/mt/msp/otp  -- read bitaddr value\n");
	mt_drv_proc_echohelp("echo w bitaddr length value > /proc/mt/msp/otp  -- write bitaddr value\n");
}

mt_s32 otp_procread(struct seq_file *p, mt_void *v)
{
	unsigned int chipid_l = 0,chipid_h = 0;
	unsigned int tmp = 0;
	if(0 == chip_rev)
	{
		otp_open(NULL,NULL);
	}
	switch(chip_rev){
		case CHIP_SYMPHONY_A0:
		case CHIP_SYMPHONY_A1:
		case CHIP_SYMPHONY_A2:
		case CHIP_SYMPHONY3_A0:
			symphony_otp_read(4032,32,(u32 *)&chipid_l);
			symphony_otp_read(4064,32,(u32 *)&chipid_h);
			break;
		case CHIP_SYMPHONY2_A0:
		case CHIP_SYMPHONY2_A1:
		case CHIP_SYMPHONY2_A2:
		case CHIP_SYMPHONY2_A3:
			symphony_otp_read(122848,8,(u32 *)&tmp);
			if(tmp == 0xff)
			{
				symphony_otp_read(129856,32,(u32 *)&chipid_l);
				symphony_otp_read(129888,32,(u32 *)&chipid_h);
			}
			else
			{
				symphony_otp_read(123072,32,(u32 *)&chipid_l);
				symphony_otp_read(123104,32,(u32 *)&chipid_h);
			}
			break;
		case CHIP_SYMPHONY4_A0:
		case CHIP_SYMPHONY4_A1:
			//FIXME: 120224? 120256?
			symphony_otp_read(120224,32,(u32 *)&chipid_l);
			symphony_otp_read(120256,32,(u32 *)&chipid_h);
			break;
		default:
		{
			if (chip_rev >= CHIP_SYMPHONY6_A0 && chip_rev < CHIP_SYMPHONY6_MAX)
			{
#ifdef CONFIG_MT_CHIP_SYMPHONY6
				//symphony_otp_read(130624,32,(u32 *)&chipid_l);
				//symphony_otp_read(130656,32,(u32 *)&chipid_h);
				mt_otp_dump_state(p);
#endif
				break;
			}
			else
			{
				MT_ERR_OTP("wrong chip version\n");
				return MT_SUCCESS;
			}
		}
	}

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
	PROC_PRINT(p, "chipid_l = 0x%x \n",chipid_l);
	PROC_PRINT(p, "chipid_h = 0x%x \n",chipid_h);
#endif

	return MT_SUCCESS;
}

mt_s32 otp_procwrite(struct file * file,
                     const char __user * buf, size_t count, loff_t *ppos)

{
	mt_char ProcPara[PROC_PARAM_MAXLEN] = {0};
	mt_char *param = NULL;
	int res = -1,i = 0;
	unsigned int tmp = 0;
	unsigned long length = 0;
	unsigned long value = 0;

	if(count > PROC_PARAM_MAXLEN)
	{
		MT_ERR_OTP("write data is too long!\n");
		return -EFAULT;
	}

	if(copy_from_user(ProcPara, buf, count))
	{
		MT_ERR_OTP("copy user data error!\n");
		return -EFAULT;
	}
	ProcPara[PROC_PARAM_MAXLEN-1] = 0;
	param =ProcPara;
	if(ProcPara[0] == 'o')
	{
		g_wflag = 0;
		chip_rev = symphony_get_chip_rev();
		switch(chip_rev)
		{
			case CHIP_SYMPHONY_A0:
			case CHIP_SYMPHONY_A1:
			case CHIP_SYMPHONY_A2:
			case CHIP_SYMPHONY3_A0:
				symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
				symphony_otp_read = symphony_otp_read_raw;
				symphony_otp_write = symphony_otp_write_raw;
				symphony_otp_timer_init();
				printk("sym1 \n");
				break;
			case CHIP_SYMPHONY2_A0:
			case CHIP_SYMPHONY2_A1:
			case CHIP_SYMPHONY2_A2:
			case CHIP_SYMPHONY2_A3:
				symphony_mb_release_memlock(T_OTPLOCK_SPINLOCK);
				symphony_otp_read = symphony2_otp_read_raw;
				symphony_otp_write = symphony2_otp_write_raw;
				symphony_otp_unlock_write = symphony2_otp_unlock_write_protect;
				printk("sym2 \n");
				break;
			case CHIP_SYMPHONY4_A0:
			case CHIP_SYMPHONY4_A1:
				symphony_otp_read = symphony4_otp_read_raw;
				symphony_otp_write = symphony4_otp_write_raw;
				symphony_otp_unlock_write = symphony4_otp_unlock_write_protect;
				g_otpbaseaddr = mt_get_otp_base();
				printk("sym4 \n");
				break;
			default:
			{
				if (chip_rev >= CHIP_SYMPHONY6_A0 && chip_rev < CHIP_SYMPHONY6_MAX)
				{
					symphony_otp_read = symphony4_otp_read_raw;
					symphony_otp_write = symphony4_otp_write_raw;
					symphony_otp_unlock_write = symphony4_otp_unlock_write_protect;
					g_otpbaseaddr = mt_get_otp_base();
					printk("sym6 \n");
					break;
				}
				else
				{
					MT_ERR_OTP("wrong chip version\n");
					return -ENXIO;
				}
			}
		}
	}
	else if(ProcPara[0] == 'e')
	{
		param+=2;
		value = simple_strtoul(param, NULL, 0);
		if(value == 1)
		{
			g_wflag = 1;
		}
	}
	else if(ProcPara[0] == 'r')
	{
		g_wflag = 0;
		param+=2;
		value = simple_strtoul(param, NULL, 10);

		for(i = 0;i<32;i++)
		{
			if(param[i] == ' ')
			{
				break;
			}
		}
		param+=(i+1);
		length = simple_strtoul(param, NULL, 10);
		if((length > 32) || (length == 0))
		{
			MT_ERR_OTP("length = %ld is error!\n", length);
			return -EFAULT;
		}
		if(symphony_otp_read)
		{
			res = symphony_otp_read(value,length,(u32 *)&tmp);
			if(res)
			{
				MT_ERR_OTP("symphony_otp_read error!\n");
			}
			else
			{
				mt_drv_proc_echohelp("otp read bitaddr = 0x%lx length = %ld value = 0x%x\n",value,length,tmp);
			}
		}
		else
		{
			MT_ERR_OTP("Otp dev not open!\n");
		}
	}
	else if(ProcPara[0] == 'w')
	{
		param+=2;
		value = simple_strtoul(param, NULL, 10);

		for(i = 0;i<32;i++)
		{
			if(param[i] == ' ')
			{
				break;
			}
		}
		param+=(i+1);
		length = simple_strtoul(param, NULL, 10);
		if((length > 32) || (0 == length))
		{
			MT_ERR_OTP("length = %ld is error!\n", length);
			return -EFAULT;
		}

		for(i = 0;i<32;i++)
		{
			if(param[i] == ' ')
			{
				break;
			}
		}
		param+=(i+1);
		tmp = simple_strtoul(param, NULL, 0);
		if(1 == g_wflag)
		{
			g_wflag = 0;
			if(symphony_otp_write)
			{
				res = symphony_otp_write(value,length,tmp);
				if(res)
				{
					MT_ERR_OTP("symphony_otp_write error!\n");
				}
				else
				{
					mt_drv_proc_echohelp("otp write bitaddr = 0x%lx length = %ld value = 0x%x\n",value,length,tmp);
				}
			}
			else
			{
				MT_ERR_OTP("Otp dev not open!\n");
			}
		}
		else
		{
			MT_ERR_OTP("symphony_otp_write flag not open!\n");
		}
	}
	else if(strstr(ProcPara,"help") && ProcPara[0] == 'h')
	{
		g_wflag = 0;
		otp_prochelp();
	}

	return count;
}


int __init otp_modinit(void)
{
	mt_proc_entry_t *pProcItem;

	(mt_void)mt_drv_module_register(MT_ID_OTP, "MT_OTP", MT_NULL);

	snprintf(otp_RegisterData.devfs_name, sizeof(otp_RegisterData.devfs_name), UMAP_DEVNAME_OTP);
	otp_RegisterData.minor     = UMAP_MIN_MINOR_OTP;
	otp_RegisterData.owner     = THIS_MODULE;
	otp_RegisterData.fops      = &OTP_FOPS;
	otp_RegisterData.drvops	   = &otp_drvops;

	if (mt_drv_dev_register(&otp_RegisterData) < 0)
	{
		MT_ERR_OTP("register otp failed.\n");
		return MT_FAILURE;
	}

	pProcItem = mt_drv_proc_add_module(MT_MOD_OTP, MT_NULL, MT_NULL);
	if (!pProcItem)
	{
		MT_ERR_OTP("add otp proc failed.\n");
		mt_drv_dev_unregister(&otp_RegisterData);
		return MT_FAILURE;
	}
	pProcItem->read  = otp_procread;
    pProcItem->write = otp_procwrite;

	return MT_SUCCESS;
}

void __exit otp_cleanup(void)
{
	mt_drv_proc_rm_module(MT_MOD_OTP);
	mt_drv_dev_unregister(&otp_RegisterData);
	mt_drv_module_unregister(MT_ID_OTP);

}
