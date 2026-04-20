/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_analog.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/05/14
 * Description    : Montage Analog driver.
 * History        :
 * 1.Date         : 2021/05/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#ifdef __UBOOT__
#include "mt_type.h"
#else
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/delay.h>

#include "mt_type.h"
#include "mt_drv_proc.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif
#include "mt_reg_io.h"
#include "mt_analog_config.h"
#include "mt_analog.h"
#include "mt_analog_module.h"

#define LEVEL_ERR			"[E]"

//#define DEBUG
#undef DEBUG

#ifdef __UBOOT__
#ifdef DEBUG
#define ANA_DEBUG			printf
#else
#define ANA_DEBUG(...)		do{}while(0)
#endif

#define ANA_ERROR(...)		printf(__VA_ARGS__)

#define PROC_PRINT(p, ...)	printf(__VA_ARGS__)

#else

#ifdef DEBUG
#define ANA_DEBUG			printk
#else
#define ANA_DEBUG(...)		do{}while(0)
#endif

#define ANA_ERROR(...)		printk(KERN_ERR __VA_ARGS__)
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) sizeof(a)/sizeof((a)[0])
#endif

#define CHECK_NULL_RETURN(arg, ret)	\
			do {					\
				if ((void*)(arg) == NULL) {	\
					ANA_ERROR(LEVEL_ERR "%s@%d: param is null or zero!\n", __FUNCTION__, __LINE__);	\
					return ret;	\
				}	\
			} while (0)

/* check if this analog module is locked */
#define CHECK_LOCK_RETURN(com, bit_idx, ret)	\
			do {					\
				if (is_locked((void*)(com), bit_idx)) { \
					ANA_ERROR(LEVEL_ERR "%s@%d: locked!\n", __FUNCTION__, __LINE__); \
					return ret; \
				}	\
			} while (0)

#define MT_ANA_UDELAY(us)	do {						\
								if ((us) > 2000)		\
									mdelay((us)/1000);	\
								else					\
									udelay(us);			\
							} while(0)

/**
 * Analog module common fields
 */
struct mt_ana_common
{
	MT_ANA_COMMON_FIELDS
};

//---------------------------------------------------------------------------//

#ifdef __UBOOT__
#define MUTEX_LOCK()		do{}while(0)
#define MUTEX_UNLOCK()		do{}while(0)
#else
DEFINE_MUTEX(mt_ana_mutex);

#define MUTEX_LOCK()		do{mutex_lock(&mt_ana_mutex);}while(0)
#define MUTEX_UNLOCK()		do{mutex_unlock(&mt_ana_mutex);}while(0)
#endif

static MT_BOOL _is_locked(mt_u32 reg_addr, mt_u32 bit_idx)
{
	if (reg_addr != 0
		&& bit_idx != NA_LOCK_BIT_IDX
		&& mt_reg32_get_bit((volatile mt_u32 *)reg_addr, bit_idx) != 0)
		return MT_TRUE;

	return MT_FALSE;
}

/* check if this analog module some bit is locked */
static MT_BOOL is_locked(void *ptr, mt_u32 bit_idx)
{
	struct mt_ana_common *com = (struct mt_ana_common *)ptr;

	if (_is_locked(com->reg_slock, bit_idx)
		|| _is_locked(com->reg_lock, bit_idx))
		return MT_TRUE;

	return MT_FALSE;
}

//---------------------------------------------------------------------------//

static struct mt_analog *mt_ana_get_nb(void *dev, const char *id)
{
	int i;
	int count = ARRAY_SIZE(mt_ana_table);

	CHECK_NULL_RETURN(id, NULL);

	for (i=0; i<count; i++)
	{
		if (mt_ana_table[i].name != NULL
			&& strcmp(id, mt_ana_table[i].name) == 0)
		{
			ANA_DEBUG("%s: find id(%s) ana(%d).\n", __FUNCTION__, id, i);

			return &mt_ana_table[i];
		}
	}

	ANA_ERROR(LEVEL_ERR "%s: get ana(%s) failed!\n", __FUNCTION__, id);
	return NULL;
}

struct mt_analog *mt_ana_get(void *dev, const char *id)
{
	struct mt_analog *ana;

	MUTEX_LOCK();
	ana = mt_ana_get_nb(dev, id);
	MUTEX_UNLOCK();

	return ana;
}

mt_s32 mt_ana_enable(struct mt_analog *ana)
{
	CHECK_NULL_RETURN(ana, MT_ERR_PARAM);
	CHECK_NULL_RETURN(ana->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((ana->flags & FLAG_ANA_GATE), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(&ana->gate, ana->gate.bit_lock, MT_FAILURE);

	MUTEX_LOCK();

	ana->ref_count ++;

	ANA_DEBUG("%s: ana %s ref_count %u.\n", __FUNCTION__, ana->name, ana->ref_count);

	//enable multi times
	if (ana->ref_count != 1)
	{
		MUTEX_UNLOCK();
		return MT_SUCCESS;
	}

	if (ana->gate.width == 1)
	{
		if (ana->gate.reverse)
			mt_reg32_clear_bit((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift);
		else
			mt_reg32_set_bit((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift);
	}
	else
	{
		if (ana->gate.reverse)
			mt_reg32_set_bits((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift, ana->gate.width, 0);
		else
			mt_reg32_set_bits((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift, ana->gate.width,
				MASK[ana->gate.width-1]);
	}

	MUTEX_UNLOCK();

	ANA_DEBUG("%s: ana %s enabled.\n", __FUNCTION__, ana->name);

	return MT_SUCCESS;
}

void mt_ana_disable(struct mt_analog *ana)
{
	if (ana == NULL || ana->name == NULL) {
		ANA_ERROR(LEVEL_ERR "%s@%d: param is null!\n", __FUNCTION__, __LINE__);
		return;
	}

	if ((ana->flags & FLAG_ANA_GATE) == 0) {
		ANA_ERROR(LEVEL_ERR "%s@%d: ana is not gate!\n", __FUNCTION__, __LINE__);
		return;
	}

	if (is_locked((void*)&ana->gate, ana->gate.bit_lock))
	{
		ANA_ERROR(LEVEL_ERR "%s: locked!\n", __FUNCTION__);
		return;
	}

	MUTEX_LOCK();

	if (ana->ref_count == 0)
	{
		ANA_ERROR(LEVEL_ERR "%s: ref_count is zero!\n", __FUNCTION__);
		MUTEX_UNLOCK();
		return;
	}

	ana->ref_count --;

	ANA_DEBUG("%s: ana %s ref_count %u.\n", __FUNCTION__, ana->name, ana->ref_count);

	//disable multi times
	if (ana->ref_count != 0)
	{
		MUTEX_UNLOCK();
		return;
	}

	if (ana->gate.width == 1)
	{
		if (ana->gate.reverse)
			mt_reg32_set_bit((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift);
		else
			mt_reg32_clear_bit((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift);
	}
	else
	{
		if (ana->gate.reverse)
			mt_reg32_set_bits((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift, ana->gate.width,
							MASK[ana->gate.width-1]);
		else
			mt_reg32_set_bits((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift, ana->gate.width, 0);
	}

	MUTEX_UNLOCK();

	ANA_DEBUG("%s: ana %s disabled.\n", __FUNCTION__, ana->name);
}

mt_u32 mt_ana_get_enable(struct mt_analog *ana)
{
	mt_u32 status = 0;

	CHECK_NULL_RETURN(ana, 0);
	CHECK_NULL_RETURN((ana->flags & FLAG_ANA_GATE), 0);

	MUTEX_LOCK();

	if (ana->gate.width == 1)
		status = mt_reg32_get_bit((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift);
	else
		status = mt_reg32_get_bits((volatile mt_u32 *)ana->gate.reg_addr, ana->gate.shift, ana->gate.width);

	if (ana->gate.reverse)
		status = (status==0)?1:0;

	MUTEX_UNLOCK();

	return status;
}

mt_u32 mt_ana_get_mux(struct mt_analog *ana)
{
	mt_u32 value = 0;

	CHECK_NULL_RETURN(ana, 0);
	CHECK_NULL_RETURN((ana->flags & FLAG_ANA_MUX), 0);

	MUTEX_LOCK();

	value = mt_reg32_get_bits((volatile mt_u32 *)ana->mux.reg_addr, ana->mux.shift, ana->mux.width);

	MUTEX_UNLOCK();

	return value;
}

mt_s32 mt_ana_set_mux(struct mt_analog *ana, mt_u32 value)
{
	CHECK_NULL_RETURN(ana, MT_ERR_PARAM);
	CHECK_NULL_RETURN(ana->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((ana->flags & FLAG_ANA_MUX), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(&ana->mux, ana->mux.bit_lock, MT_FAILURE);

	MUTEX_LOCK();

	mt_reg32_set_bits((volatile mt_u32 *)ana->mux.reg_addr, ana->mux.shift, ana->mux.width, value);

	MUTEX_UNLOCK();

	ANA_DEBUG("%s: ana %s mux %u.\n", __FUNCTION__, ana->name, value);

	return MT_SUCCESS;
}

mt_s32 mt_ana_reset(struct mt_analog *ana)
{
	CHECK_NULL_RETURN(ana, MT_ERR_PARAM);
	CHECK_NULL_RETURN(ana->name, MT_ERR_PARAM);
	CHECK_NULL_RETURN((ana->flags & FLAG_ANA_RESET), MT_ERR_PARAM);

	CHECK_LOCK_RETURN(&ana->reset, ana->reset.bit_lock, MT_FAILURE);

	MUTEX_LOCK();

	if (ana->reset.width == 1)
	{
		mt_reg32_set_bit((volatile mt_u32 *)ana->reset.reg_addr, ana->reset.shift);
		MT_ANA_UDELAY(CFG_MT_ANA_RESET_UDELAY);
		mt_reg32_clear_bit((volatile mt_u32 *)ana->reset.reg_addr, ana->reset.shift);
	}
	else
	{
		//TODO
		ANA_ERROR("TODO - %s: ana %s reset bits[%u] > 1!\n", __FUNCTION__,
				ana->name, ana->reset.width);
	}

	MUTEX_UNLOCK();

	ANA_DEBUG("%s: ana %s reset done.\n", __FUNCTION__, ana->name);

	return MT_SUCCESS;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY1)

static void mt_ana_batch_config_sym1(void)
{
	mt_u32 value;

	//ANA_AO_REG0
	value = mt_reg32_read((volatile mt_u32 *)ANA_AO_REG0);

#ifdef CFG_MT_ANA_PD_USB0
	value |= (0x01 << 1);
#else
	value &= ~(0x01 << 1);
#endif
#ifdef CFG_MT_ANA_PD_USB1
	value |= (0x01 << 2);
#else
	value &= ~(0x01 << 2);
#endif
#ifdef CFG_MT_ANA_PD_ADAC_L		//diff with sym4
	value |= (0x01 << 4);
#else
	value &= ~(0x01 << 4);
#endif
#ifdef CFG_MT_ANA_PD_ADAC_R		//diff with sym4
	value |= (0x01 << 5);
#else
	value &= ~(0x01 << 5);
#endif
#ifdef CFG_MT_ANA_ADAC_MUTE		//diff with sym4
	value |= (0x01 << 6);
#else
	value &= ~(0x01 << 6);
#endif
#ifdef CFG_MT_ANA_PD_HDMITX_CH
	value |= (0x01 << 8);
	value |= (0x01 << 9);
	value |= (0x01 << 10);
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 8);
	value &= ~(0x01 << 9);
	value &= ~(0x01 << 10);
	value &= ~(0x01 << 11);
#endif
//diff with sym4
#ifdef CFG_MT_ANA_PD_VDAC0
	value |= (0x01 << 12);
#else
	value &= ~(0x01 << 12);
#endif
#ifdef CFG_MT_ANA_PD_VDAC1
	value |= (0x01 << 13);
#else
	value &= ~(0x01 << 13);
#endif
#ifdef CFG_MT_ANA_PD_VDAC2
	value |= (0x01 << 14);
#else
	value &= ~(0x01 << 14);
#endif
#ifdef CFG_MT_ANA_PD_VDAC3
	value |= (0x01 << 15);
#else
	value &= ~(0x01 << 15);
#endif
#ifdef CFG_MT_ANA_PD_VDAC0_DET
	value |= (0x01 << 16);
#else
	value &= ~(0x01 << 16);
#endif
#ifdef CFG_MT_ANA_PD_VDAC1_DET
	value |= (0x01 << 17);
#else
	value &= ~(0x01 << 17);
#endif
#ifdef CFG_MT_ANA_PD_VDAC2_DET
	value |= (0x01 << 18);
#else
	value &= ~(0x01 << 18);
#endif
#ifdef CFG_MT_ANA_PD_VDAC3_DET
	value |= (0x01 << 19);
#else
	value &= ~(0x01 << 19);
#endif

#ifdef CFG_MT_ANA_PD_CADC
	value |= (0x01 << 20);
#else
	value &= ~(0x01 << 20);
#endif
#ifdef CFG_MT_ANA_PD_SADC
	value |= (0x01 << 21);
#else
	value &= ~(0x01 << 21);
#endif
#ifdef CFG_MT_ANA_PD_PROC_MON
	value |= (0x01 << 22);
#else
	value &= ~(0x01 << 22);
#endif
#ifdef CFG_MT_ANA_PD_RNG1
	value |= (0x01 << 24);
#else
	value &= ~(0x01 << 24);
#endif

	mt_reg32_write((volatile mt_u32 *)ANA_AO_REG0, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		ANA_AO_REG0, value);

	//ANA_AO_REG1
	value = mt_reg32_read((volatile mt_u32 *)ANA_AO_REG1);

#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 24);
	value |= (0x01 << 25);
#else
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
#endif
#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 26);
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 26);
	value &= ~(0x01 << 27);
#endif
#ifdef CFG_MT_ANA_PD_USB_PLL
	value |= (0x01 << 28);
	value |= (0x01 << 29);
	value |= (0x01 << 30);
#else
	value &= ~(0x01 << 28);
	value &= ~(0x01 << 29);
	value &= ~(0x01 << 30);
#endif
#ifdef CFG_MT_ANA_PD_CPU_PLL	//diff with sym4
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)ANA_AO_REG1, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		ANA_AO_REG1, value);

	//REG_CLKGEN_PDSYS
	//value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_PDSYS);
	value = 0;	//default

#if defined(CFG_MT_ANA_PD_USB_PLL)
	//BIT [5:0], [25:24], [29:28]
	value |= 0x3F;
	value |= (0x01 << 24);
	value |= (0x01 << 25);
	value |= (0x01 << 28);
	value |= (0x01 << 29);
#else
	value &= ~(0x3F);
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
	value &= ~(0x01 << 28);
	value &= ~(0x01 << 29);
#endif
#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 11);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 19);
#else
	value &= ~(0x01 << 19);
#endif
#ifdef CFG_MT_ANA_PD_CPU_PLL
	value |= (0x01 << 30);
#else
	value &= ~(0x01 << 30);
#endif
#ifdef CFG_MT_ANA_PD_CLK_CAL
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_PDSYS, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_PDSYS, value);

	//REG_CLKGEN_TEST
	value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_TEST);

#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 16);
	value |= (0x01 << 17);
	value |= (0x01 << 19);
	value |= (0x01 << 20);
	value |= (0x01 << 21);
#else
	value &= ~(0x01 << 16);
	value &= ~(0x01 << 17);
	value &= ~(0x01 << 19);
	value &= ~(0x01 << 20);
	value &= ~(0x01 << 21);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 22);
	value |= (0x01 << 23);
	value |= (0x01 << 24);
	value |= (0x01 << 25);
	value |= (0x01 << 26);
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 22);
	value &= ~(0x01 << 23);
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
	value &= ~(0x01 << 26);
	value &= ~(0x01 << 27);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_TEST, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_TEST, value);
}

#elif defined(CONFIG_MT_CHIP_SYMPHONY2)

static void mt_ana_batch_config_sym2(void)
{
	mt_u32 value;

	//ANA_AO_REG0
	value = mt_reg32_read((volatile mt_u32 *)ANA_AO_REG0);

#ifdef CFG_MT_ANA_PD_USB0
	value |= (0x01 << 1);
#else
	value &= ~(0x01 << 1);
#endif
#ifdef CFG_MT_ANA_PD_USB1
	value |= (0x01 << 2);
#else
	value &= ~(0x01 << 2);
#endif
#ifdef CFG_MT_ANA_PD_ADAC_L
	value |= (0x01 << 4);
#else
	value &= ~(0x01 << 4);
#endif
#ifdef CFG_MT_ANA_PD_ADAC_R
	value |= (0x01 << 5);
#else
	value &= ~(0x01 << 5);
#endif
#ifdef CFG_MT_ANA_ADAC_MUTE		//diff with sym4
	value |= (0x01 << 6);
#else
	value &= ~(0x01 << 6);
#endif
#ifdef CFG_MT_ANA_PD_HDMITX_CH
	value |= (0x01 << 8);
	value |= (0x01 << 9);
	value |= (0x01 << 10);
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 8);
	value &= ~(0x01 << 9);
	value &= ~(0x01 << 10);
	value &= ~(0x01 << 11);
#endif
//diff with sym4
#ifdef CFG_MT_ANA_PD_VDAC0
	value |= (0x01 << 12);
#else
	value &= ~(0x01 << 12);
#endif
#ifdef CFG_MT_ANA_PD_VDAC1
	value |= (0x01 << 13);
#else
	value &= ~(0x01 << 13);
#endif
#ifdef CFG_MT_ANA_PD_VDAC2
	value |= (0x01 << 14);
#else
	value &= ~(0x01 << 14);
#endif
#ifdef CFG_MT_ANA_PD_VDAC3
	value |= (0x01 << 15);
#else
	value &= ~(0x01 << 15);
#endif
#ifdef CFG_MT_ANA_PD_VDAC0_DET
	value |= (0x01 << 16);
#else
	value &= ~(0x01 << 16);
#endif
#ifdef CFG_MT_ANA_PD_VDAC1_DET
	value |= (0x01 << 17);
#else
	value &= ~(0x01 << 17);
#endif
#ifdef CFG_MT_ANA_PD_VDAC2_DET
	value |= (0x01 << 18);
#else
	value &= ~(0x01 << 18);
#endif
#ifdef CFG_MT_ANA_PD_VDAC3_DET
	value |= (0x01 << 19);
#else
	value &= ~(0x01 << 19);
#endif

#ifdef CFG_MT_ANA_PD_CADC
	value |= (0x01 << 20);
#else
	value &= ~(0x01 << 20);
#endif
#ifdef CFG_MT_ANA_PD_SADC
	value |= (0x01 << 21);
#else
	value &= ~(0x01 << 21);
#endif
#ifdef CFG_MT_ANA_PD_PROC_MON
	value |= (0x01 << 22);
#else
	value &= ~(0x01 << 22);
#endif
#ifdef CFG_MT_ANA_PD_RNG1
	value |= (0x01 << 24);
#else
	value &= ~(0x01 << 24);
#endif
#ifdef CFG_MT_ANA_PD_TEMP_SENSOR
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 27);
#endif
#ifdef CFG_MT_ANA_PD_RNG2
	value |= (0x01 << 29);
#else
	value &= ~(0x01 << 29);
#endif
#ifdef CFG_MT_ANA_PD_EPHY_PLL
	value |= (0x01 << 30);
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 30);
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)ANA_AO_REG0, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		ANA_AO_REG0, value);

	//ANA_AO_REG1
	value = mt_reg32_read((volatile mt_u32 *)ANA_AO_REG1);

#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 24);
	value |= (0x01 << 25);
#else
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
#endif
#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 26);
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 26);
	value &= ~(0x01 << 27);
#endif
#ifdef CFG_MT_ANA_PD_USB_PLL
	value |= (0x01 << 28);
	value |= (0x01 << 29);
	value |= (0x01 << 30);
#else
	value &= ~(0x01 << 28);
	value &= ~(0x01 << 29);
	value &= ~(0x01 << 30);
#endif
#ifdef CFG_MT_ANA_PD_CPU_PLL	//diff with sym4
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)ANA_AO_REG1, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		ANA_AO_REG1, value);

	//REG_CLKGEN_PDSYS
	//value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_PDSYS);
	value = 0;	//default

#if defined(CFG_MT_ANA_PD_USB_PLL)
	//BIT [5:0], [25:24], [29:28]
	value |= 0x3F;
	value |= (0x01 << 24);
	value |= (0x01 << 25);
	value |= (0x01 << 28);
	value |= (0x01 << 29);
#else
	value &= ~(0x3F);
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
	value &= ~(0x01 << 28);
	value &= ~(0x01 << 29);
#endif
#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 11);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 19);
#else
	value &= ~(0x01 << 19);
#endif
#ifdef CFG_MT_ANA_PD_CPU_PLL
	value |= (0x01 << 30);
#else
	value &= ~(0x01 << 30);
#endif
#ifdef CFG_MT_ANA_PD_CLK_CAL
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_PDSYS, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_PDSYS, value);

	//REG_CLKGEN_CPUPLL
	value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_CPUPLL);

#ifdef CFG_MT_ANA_PD_CADC
	value |= (0x01 << 4);
	value |= (0x01 << 5);
#else
	value &= ~(0x01 << 4);
	value &= ~(0x01 << 5);
#endif
#ifdef CFG_MT_ANA_PD_SADC
	value |= (0x01 << 6);
#else
	value &= ~(0x01 << 6);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_CPUPLL, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_CPUPLL, value);

	//REG_CLKGEN_TEST
	value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_TEST);

#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 16);
	value |= (0x01 << 17);
	value |= (0x01 << 19);
	value |= (0x01 << 20);
	value |= (0x01 << 21);
#else
	value &= ~(0x01 << 16);
	value &= ~(0x01 << 17);
	value &= ~(0x01 << 19);
	value &= ~(0x01 << 20);
	value &= ~(0x01 << 21);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 22);
	value |= (0x01 << 23);
	value |= (0x01 << 24);
	value |= (0x01 << 25);
	value |= (0x01 << 26);
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 22);
	value &= ~(0x01 << 23);
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
	value &= ~(0x01 << 26);
	value &= ~(0x01 << 27);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_TEST, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_TEST, value);
}

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)

static void mt_ana_batch_config_sym4(void)
{
	mt_u32 value;

	//ANA_AO_REG0
	//value = mt_reg32_read((volatile mt_u32 *)ANA_AO_REG0);
	value = 0;	//open all analog modules

#ifdef CFG_MT_ANA_PD_USB0
	value |= (0x01 << 1);
#else
	value &= ~(0x01 << 1);
#endif
#ifdef CFG_MT_ANA_PD_USB1
	value |= (0x01 << 2);
#else
	value &= ~(0x01 << 2);
#endif
#ifdef CFG_MT_ANA_PD_AADC_0		//diff with sym2
	value |= (0x01 << 4);
#else
	value &= ~(0x01 << 4);
#endif
#ifdef CFG_MT_ANA_PD_AADC_1		//diff with sym2
	value |= (0x01 << 5);
#else
	value &= ~(0x01 << 5);
#endif
#ifdef CFG_MT_ANA_PD_VDAC0
	value |= (0x01 << 6);
#else
	value &= ~(0x01 << 6);
#endif
#ifdef CFG_MT_ANA_PD_HDMITX_CH
	value |= (0x01 << 8);
	value |= (0x01 << 9);
	value |= (0x01 << 10);
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 8);
	value &= ~(0x01 << 9);
	value &= ~(0x01 << 10);
	value &= ~(0x01 << 11);
#endif
#ifdef CFG_MT_ANA_PD_ADAC_BUF	//diff with sym2
	value |= (0x01 << 12);
	value |= (0x01 << 13);
	value |= (0x01 << 14);
	value |= (0x01 << 15);
	value |= (0x01 << 16);
	value |= (0x01 << 17);
	value |= (0x01 << 18);
	value |= (0x01 << 19);
#else

#ifdef CFG_MT_ANA_ADAC_2VRMS
	//2Vrms mode
	value &= ~(0x01 << 12);
	value |= (0x01 << 13);
	value &= ~(0x01 << 16);
	value |= (0x01 << 17);
#else
	//0dBu mode
	value |= (0x01 << 12);
	value &= ~(0x01 << 13);
	value |= (0x01 << 16);
	value &= ~(0x01 << 17);
#endif

	value |= (0x01 << 14);
	value &= ~(0x01 << 15);

	value |= (0x01 << 18);
	value &= ~(0x01 << 19);
#endif
#ifdef CFG_MT_ANA_PD_CADC
	value |= (0x01 << 20);
#else
	value &= ~(0x01 << 20);
#endif
#ifdef CFG_MT_ANA_PD_SADC
	value |= (0x01 << 21);
#else
	value &= ~(0x01 << 21);
#endif
#ifdef CFG_MT_ANA_PD_PROC_MON
	value |= (0x01 << 22);
#else
	value &= ~(0x01 << 22);
#endif
#ifdef CFG_MT_ANA_PD_RNG1
	value |= (0x01 << 24);
#else
	value &= ~(0x01 << 24);
#endif
#ifdef CFG_MT_ANA_PD_TEMP_SENSOR
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 27);
#endif
#ifdef CFG_MT_ANA_PD_RNG2
	value |= (0x01 << 29);
#else
	value &= ~(0x01 << 29);
#endif
#ifdef CFG_MT_ANA_PD_EPHY_PLL
	value |= (0x01 << 30);
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 30);
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)ANA_AO_REG0, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		ANA_AO_REG0, value);

	//ANA_AO_REG1
	value = mt_reg32_read((volatile mt_u32 *)ANA_AO_REG1);

#ifdef CFG_MT_ANA_PD_OTP_REG
	value |= 0x01;
#else
	value &= ~(0x01);
#endif
#ifdef CFG_MT_ANA_PD_VDAC0_DET
	value |= (0x01 << 1);
#else
	value &= ~(0x01 << 1);
#endif
#ifdef CFG_MT_ANA_ADAC_MUTE
	value |= (0x01 << 2);
#else
	value &= ~(0x01 << 2);
#endif
#ifdef CFG_MT_ANA_PD_AADC_MIC
	value |= (0x01 << 4);
#else
	value &= ~(0x01 << 4);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 24);
	value |= (0x01 << 25);
#else
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
#endif
#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 26);
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 26);
	value &= ~(0x01 << 27);
#endif
#ifdef CFG_MT_ANA_PD_USB_PLL
	value |= (0x01 << 28);
	value |= (0x01 << 29);
	value |= (0x01 << 30);
#else
	value &= ~(0x01 << 28);
	value &= ~(0x01 << 29);
	value &= ~(0x01 << 30);
#endif
#ifdef CFG_MT_ANA_PD_ADC_PLL
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)ANA_AO_REG1, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		ANA_AO_REG1, value);

	//REG_CLKGEN_PDSYS
	//value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_PDSYS);
	value = 0;	//default

#if defined(CFG_MT_ANA_PD_USB_PLL)
	//BIT [5:0], [25:24], [29:28]
	value |= 0x3F;
	value |= (0x01 << 24);
	value |= (0x01 << 25);
	value |= (0x01 << 28);
	value |= (0x01 << 29);
#else
	value &= ~(0x3F);
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
	value &= ~(0x01 << 28);
	value &= ~(0x01 << 29);
#endif
#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 11);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 19);
#else
	value &= ~(0x01 << 19);
#endif
#ifdef CFG_MT_ANA_PD_CPU_PLL
	value |= (0x01 << 30);
#else
	value &= ~(0x01 << 30);
#endif
#ifdef CFG_MT_ANA_PD_CLK_CAL
	value |= (0x01 << 31);
#else
	value &= ~(0x01 << 31);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_PDSYS, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_PDSYS, value);

	//REG_CLKGEN_CPUPLL
	value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_CPUPLL);

#ifdef CFG_MT_ANA_PD_CADC
	value |= (0x01 << 4);
	value |= (0x01 << 5);
#else
	value &= ~(0x01 << 4);
	value &= ~(0x01 << 5);
#endif
#ifdef CFG_MT_ANA_PD_SADC
	value |= (0x01 << 6);
	value |= (0x01 << 7);
#else
	value &= ~(0x01 << 6);
	value &= ~(0x01 << 7);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_CPUPLL, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_CPUPLL, value);

	//REG_CLKGEN_TEST
	value = mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_TEST);

#ifdef CFG_MT_ANA_PD_VHD_PLL
	value |= (0x01 << 16);
	value |= (0x01 << 17);
	value |= (0x01 << 19);
	value |= (0x01 << 20);
	value |= (0x01 << 21);
#else
	value &= ~(0x01 << 16);
	value &= ~(0x01 << 17);
	value &= ~(0x01 << 19);
	value &= ~(0x01 << 20);
	value &= ~(0x01 << 21);
#endif
#ifdef CFG_MT_ANA_PD_VSD_PLL
	value |= (0x01 << 22);
	value |= (0x01 << 23);
	value |= (0x01 << 24);
	value |= (0x01 << 25);
	value |= (0x01 << 26);
	value |= (0x01 << 27);
#else
	value &= ~(0x01 << 22);
	value &= ~(0x01 << 23);
	value &= ~(0x01 << 24);
	value &= ~(0x01 << 25);
	value &= ~(0x01 << 26);
	value &= ~(0x01 << 27);
#endif

	mt_reg32_write((volatile mt_u32 *)REG_CLKGEN_TEST, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		REG_CLKGEN_TEST, value);

	//CLKGEN_USBACLK_REG0
	value = mt_reg32_read((volatile mt_u32 *)CLKGEN_USBACLK_REG0);

#ifdef CFG_MT_ANA_PD_ACLK_TOP
	value |= (0x01 << 11);
#else
	value &= ~(0x01 << 11);
#endif

	mt_reg32_write((volatile mt_u32 *)CLKGEN_USBACLK_REG0, value);
	ANA_DEBUG("%s: Reg %x, Value %x\n",__FUNCTION__,
		CLKGEN_USBACLK_REG0, value);
}
#endif

void mt_ana_batch_config(void)
{
	MUTEX_LOCK();

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	mt_ana_batch_config_sym1();
#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
	mt_ana_batch_config_sym2();
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	mt_ana_batch_config_sym4();
#else
	#error "Please config select one correct chip!"
#endif

	MUTEX_UNLOCK();
}

#ifdef __UBOOT__
void mt_ana_dump_state(void *p)
#else
void mt_ana_dump_state(struct seq_file *p)
#endif
{
	int i;
	int count;
	struct mt_analog *ana;
	struct mt_analog *p_ana_table;

	char str_gate[16];
	char str_mux[16];
	char str_reset[8];

	//printk(KERN_INFO "==================== DUMP ANA ==================\r\n");
	//printk(KERN_INFO "[Analog]             \t[Gate]\t[Mux]\t[Reset]\r\n");
	PROC_PRINT(p, "==================== DUMP ANA ==================\r\n");
	PROC_PRINT(p, "        ANA_AO_REG0[%x]: %08x\r\n", ANA_AO_REG0, mt_reg32_read((volatile mt_u32 *)ANA_AO_REG0));
	PROC_PRINT(p, "        ANA_AO_REG1[%x]: %08x\r\n", ANA_AO_REG1, mt_reg32_read((volatile mt_u32 *)ANA_AO_REG1));
	PROC_PRINT(p, "       ANA_TOP_REG1[%x]: %08x\r\n", ANA_TOP_REG1, mt_reg32_read((volatile mt_u32 *)ANA_TOP_REG1));
	PROC_PRINT(p, "  REG_CLKGEN_CPUPLL[%x]: %08x\r\n", REG_CLKGEN_CPUPLL, mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_CPUPLL));
	PROC_PRINT(p, "    REG_CLKGEN_TEST[%x]: %08x\r\n", REG_CLKGEN_TEST, mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_TEST));
	PROC_PRINT(p, "   REG_CLKGEN_PDSYS[%x]: %08x\r\n", REG_CLKGEN_PDSYS, mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_PDSYS));
	PROC_PRINT(p, " REG_CLKGEN_EPHYPLL[%x]: %08x\r\n", REG_CLKGEN_EPHYPLL, mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_EPHYPLL));
	PROC_PRINT(p, "    REG_CLKGEN_EPHY[%x]: %08x\r\n", REG_CLKGEN_EPHY, mt_reg32_read((volatile mt_u32 *)REG_CLKGEN_EPHY));
	PROC_PRINT(p, "CLKGEN_USBACLK_REG0[%x]: %08x\r\n", CLKGEN_USBACLK_REG0, mt_reg32_read((volatile mt_u32 *)CLKGEN_USBACLK_REG0));
	PROC_PRINT(p, "------------------------------------------------\r\n");
	PROC_PRINT(p, "[Analog]             \t[Gate]\t[Mux]\t[Reset]\r\n");

	p_ana_table = mt_ana_table;
	count = ARRAY_SIZE(mt_ana_table);

	for (i=0; i<count; i++)
	{
		ana = &p_ana_table[i];

		//printk(KERN_INFO "ANA[%d]: %s\n", i, ana->name);
		//printk(KERN_INFO "\tSLOCK: %X LOCK: %X BIT: %u\n", ana->gate.reg_slock, ana->gate.reg_lock, ana->gate.bit_lock);

		if ((ana->flags & FLAG_ANA_GATE) != 0)
		{
			if (ana->gate.reverse)
			{
				/*printk(KERN_INFO "\tGATE[%X][%u:%u]: %u (Reversed) (0: OFF, 1: ON)\r\n",
					ana->gate.reg_addr, ana->gate.shift + ana->gate.width - 1,
					ana->gate.shift,
					mt_ana_get_enable(ana));*/
				snprintf(str_gate, sizeof(str_gate), "%u (Rev)", mt_ana_get_enable(ana));
			}
			else
			{
				/*printk(KERN_INFO "\tGATE[%X][%u:%u]: %u (0: OFF, 1: ON)\r\n",
					ana->gate.reg_addr, ana->gate.shift + ana->gate.width - 1,
					ana->gate.shift,
					mt_ana_get_enable(ana));*/
				snprintf(str_gate, sizeof(str_gate), "%u", mt_ana_get_enable(ana));
			}
		}
		else
		{
			snprintf(str_gate, sizeof(str_gate), "NA");
		}

		if ((ana->flags & FLAG_ANA_MUX) != 0)
		{
			snprintf(str_mux, sizeof(str_mux), "%u", mt_ana_get_mux(ana));
		}
		else
		{
			snprintf(str_mux, sizeof(str_mux), "NA");
		}

		if ((ana->flags & FLAG_ANA_RESET) != 0)
		{
			snprintf(str_reset, sizeof(str_reset), "*");
		}
		else
		{
			snprintf(str_reset, sizeof(str_reset), "NA");
		}

		//printk(KERN_INFO "%-19s\t%-7s\t%s\t%s\r\n",
		//	ana->name, str_gate, str_mux, str_reset);
		PROC_PRINT(p, "%-19s\t%-7s\t%s\t%s\r\n",
			ana->name, str_gate, str_mux, str_reset);
	}

	//printk(KERN_INFO "================================================\r\n");
	PROC_PRINT(p, "================================================\r\n");
}

EXPORT_SYMBOL(mt_ana_batch_config);

#if defined(CONFIG_MT_CHIP_SYMPHONY4)

#define MT_SYMPHONY_RST_RE02_REG ((0xbf510008))
#define MT_SYMPHONY_RST_CTRL02_REG ((0xbf510018))
#define MT_SYMPHONY_RST_ALLOW02_REG ((0xbf510028))
#define MT_SYMPHONY_PHY_REG		(SYMPHONYIO_PA(0xBF5D0030))
#define MT_SYMPHONY_RST_LEN (4)
#define MT_SYMPHONY_RESETERRA0 ((0xbf5d0030))
#define MT_SYMPHONY_RESETERRA1 ((0xbf5d0034))

#define MT_SYMPHONY_ERRLOG0	((0xbf5d0038))
#define MT_SYMPHONY_ERRLOG1	((0xbf5d003c))

#define MT_SYMPHONY_ANA_OP9	((0xbf5d0138))

#define MT_SYMPHONY_USB_FIFO_UNDERFLOW	(1)
#define MT_SYMPHONY_USB_FIFO_OVERFLOW	(2)


int mt_ana_usb_init_mode(int port)
{
	unsigned int __iomem *reg_op9 = 0, *reg_res = 0; 
	unsigned int val = 0;
	//unsigned int mode = 0;
	unsigned int flag = 0;
	if((port < 0) || (port > 1))
	{
		printk("mt_usb_init_mode port = %d is error!\n",port);
		return -1;
	}

	reg_op9 = ioremap_nocache(MT_SYMPHONY_ANA_OP9, MT_SYMPHONY_RST_LEN);
	if(port == 1)
	{
		reg_res = ioremap_nocache(MT_SYMPHONY_RESETERRA1, MT_SYMPHONY_RST_LEN);
	}
	else
	{
		reg_res = ioremap_nocache(MT_SYMPHONY_RESETERRA0, MT_SYMPHONY_RST_LEN);
	}

	val = readl(reg_op9);
	if(((val >> 6) & 0x7) != 3)
	{
		val &= ~(0x7 << 6);
		val |= (3 << 6);
		writel(val,reg_op9);
		flag++;
	}	

	val = readl(reg_res);
	if(((val >> 18) & 0x3f) != 4)
	{
		val &= ~(0x3f << 18);
		val |= (4 << 18);
		writel(val,reg_res);
		flag++;
	}
	if(0 != flag)
	{
		printk("%d mt_usb_init_mode 0x%x = %x\n",__LINE__, (unsigned int)reg_op9,readl(reg_op9));
		printk("%d mt_usb_init_mode 0x%x = %x\n",__LINE__, (unsigned int)reg_res,readl(reg_res));		
	}
	val = readl(reg_res);
	val |= (1 << 30);
	writel(val,reg_res);
	mdelay(1);
	val &= ~(1 << 30);
	writel(val,reg_res);

	iounmap(reg_op9);
	iounmap(reg_res);
	return 0;
}


int mt_ana_usb_switch_mode(int port)
{
	unsigned int __iomem *reg_op9 = 0, *reg_res = 0, *reg_errlog = 0; 
	unsigned int val = 0;
	unsigned int mode = 0;
	if((port < 0) || (port > 1))
	{
		printk("usb_switch_mode port = %d is error!\n",port);
		return -1;
	}
	reg_op9 = ioremap_nocache(MT_SYMPHONY_ANA_OP9, MT_SYMPHONY_RST_LEN);
	if(port == 1)
	{
		reg_res = ioremap_nocache(MT_SYMPHONY_RESETERRA1, MT_SYMPHONY_RST_LEN);
		reg_errlog = ioremap_nocache(MT_SYMPHONY_ERRLOG1, MT_SYMPHONY_RST_LEN);
	}
	else
	{
		reg_res = ioremap_nocache(MT_SYMPHONY_RESETERRA0, MT_SYMPHONY_RST_LEN);
		reg_errlog = ioremap_nocache(MT_SYMPHONY_ERRLOG0, MT_SYMPHONY_RST_LEN);
	}

	val = readl(reg_errlog);
	if(((val >> 5) & 0x01) == 1)	//underflow 
	{
		mode = MT_SYMPHONY_USB_FIFO_UNDERFLOW;
	}
	else if(((val >> 6) & 0x01) == 1)	//overflow 
	{
		mode = MT_SYMPHONY_USB_FIFO_OVERFLOW;
	}
	
	switch(mode){
		case MT_SYMPHONY_USB_FIFO_UNDERFLOW:
			val = readl(reg_op9);
			
			if(((val >> 6) & 0x7) != 2)
			{
				val &= ~(0x7 << 6);
				val |= (2 << 6);
				writel(val,reg_op9);
			}	

			val = readl(reg_res);
			if(((val >> 18) & 0x3f) != 10)
			{
				val &= ~(0x3f << 18);
				val |= (10 << 18);
				writel(val,reg_res);
			}
			printk("%d usb_mode 0x%x = %x\n",__LINE__, (unsigned int)reg_op9,readl(reg_op9));
			printk("%d usb_mode 0x%x = %x\n",__LINE__, (unsigned int)reg_res,readl(reg_res));		
			break;
		case MT_SYMPHONY_USB_FIFO_OVERFLOW:
			val = readl(reg_op9);
			if(((val >> 6) & 0x7) != 3)
			{
				val &= ~(0x7 << 6);
				val |= (3 << 6);
				writel(val,reg_op9);
			}	

			val = readl(reg_res);
			if(((val >> 18) & 0x3f) != 4)
			{
				val &= ~(0x3f << 18);
				val |= (4 << 18);
				writel(val,reg_res);
			}
			printk("%d usb_mode 0x%x = %x\n",__LINE__, (unsigned int)reg_op9,readl(reg_op9));
			printk("%d usb_mode 0x%x = %x\n",__LINE__, (unsigned int)reg_res,readl(reg_res));		
			break;
		default:
			iounmap(reg_op9);
			iounmap(reg_res);
			iounmap(reg_errlog);
			return -1;
			break;
	}
	val = readl(reg_res);
	val |= (1 << 30);
	writel(val,reg_res);
	mdelay(1);
	val &= ~(1 << 30);
	writel(val,reg_res);

	iounmap(reg_op9);
	iounmap(reg_res);
	iounmap(reg_errlog);
	return 0;
}
EXPORT_SYMBOL(mt_ana_usb_init_mode);
EXPORT_SYMBOL(mt_ana_usb_switch_mode);
#endif
