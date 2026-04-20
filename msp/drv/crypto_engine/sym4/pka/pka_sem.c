/***************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/***************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_type.h"
#include "pka_sem.h"
#include "pka_regs.h"

/* #define PKA_SEM_MAX_LOOP_CNT 1000 */

uint8_t sem_cpu_id = PKA_SEM_CPU_ID_REECPU;

void sem_cpu_id_set(uint8_t cpu_id)
{
	sem_cpu_id = cpu_id;
}

mt_s32 pka_sem_wait(mt_u8 cpu_id, mt_u32 req_time)
{
	mt_u32 val;
#ifdef PKA_SEM_MAX_LOOP_CNT
	mt_u32 i = 0;
#endif

	do {
		val = (req_time << 4) | cpu_id;
		HAL_PUT_U32((volatile mt_u32 *)REG_SEMPH_CTL, val);

#ifdef PKA_SEM_MAX_LOOP_CNT
		if (++i >= PKA_SEM_MAX_LOOP_CNT) {
			printk("%s timeout!\n", __FUNCTION__);
			return -1;
		}
#endif
		if ((HAL_GET_U32((volatile mt_u32 *)REG_SEMPH_CTL) & PKA_SEM_CPU_ID_MASK) == cpu_id)
			break;
	} while (1);

	return 0;
}

mt_s32 pka_sem_post(void)
{
	HAL_PUT_U32((volatile mt_u32 *)REG_SEMPH_CTL, 0);
	return 0;
}

mt_s32 pka_sem_is_free(void)
{
	if ((HAL_GET_U32((volatile mt_u32 *)REG_SEMPH_CTL) & PKA_SEM_CPU_ID_MASK) == 0)
		return 1;

	return 0;
}
