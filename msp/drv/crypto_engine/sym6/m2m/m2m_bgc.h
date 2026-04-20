/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _M2M_BGC_H_
#define _M2M_BGC_H_

#include "mt_lib.h"

enum M2M_BGC_SEM_CPU_ID
{
    M2M_BGC_SEM_CPU_ID_REECPU = 0x8,
    M2M_BGC_SEM_CPU_ID_TEECPU = 0xd,
    M2M_BGC_SEM_CPU_ID_VSCPU = 0xc,

    M2M_BGC_SEM_CPU_ID_MASK = 0xf,
};

enum M2M_BGC_CMD_SLOT
{
    M2M_BGC_CMD_SLOT0 = 0,
    M2M_BGC_CMD_SLOT1,
    M2M_BGC_CMD_SLOT2,
    M2M_BGC_CMD_SLOT3,

    M2M_BGC_CMD_SLOT_BUTT
};

enum M2M_BGC_CMD_DELAY_TIME
{
    M2M_BGC_CMD_DELAY_NONE = 0,
    M2M_BGC_CMD_DELAY_QUARTER_SEC,
    M2M_BGC_CMD_DELAY_HALF_SEC,
    M2M_BGC_CMD_DELAY_ONE_SEC,
};

typedef enum
{
    MT_BGC_SEM_REQ_ONE_TIME = 0,
    MT_BGC_SEM_REQ_WAIT_FOREVER,
} MT_BGC_SEM_REQ_S;

mt_s32 m2m_bgc_sem_wait(mt_u32 cpu_id, MT_BGC_SEM_REQ_S req_timeout);

mt_s32 m2m_bgc_sem_post(void);

mt_s32 m2m_bgc_sem_check(mt_u32 cpu_id);

mt_s32 m2m_bgc_slot_start(mt_u32 slot, const mt_u8 *addr, mt_u32 len,
                          const mt_u8 *digest, mt_u32 delay);

mt_s32 m2m_bgc_slot_stop(mt_u32 slot);

mt_s32 m2m_bgc_slot_lock(mt_u32 slot);

mt_s32 m2m_bgc_get_state(mt_u32 slot, mt_u8 *checked_count,
                         mt_u8 *checked_result);

void m2m_bgc_print_debug(void);

mt_s32 m2m_bgc_slot_request(void);

#endif /* end of include guard: _M2M_BGC_H_ */
