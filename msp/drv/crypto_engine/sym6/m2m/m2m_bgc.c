/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/delay.h>
#include "m2m_bgc.h"
#include "hw_m2m_if.h"
#include "hw_m2m_register.h"

#define SHA256_DIGEST_SIZE (32)
#define bgc_readl(addr)         HAL_GET_U32((volatile u32 *)(ulong)addr)
#define bgc_writel(v, addr)     HAL_PUT_U32((volatile u32 *)(ulong)addr, v)

static mt_u32 g_bgc_slot_ctrl = 0;
#define BGC_SLOT_LOCK_INDEX_BIT (16)

static void m2m_bgc_get_init_status(void)
{
    mt_u8 slot_index = 0;

    for (slot_index = 0; slot_index < M2M_BGC_CMD_SLOT_BUTT; slot_index++)
    {
        if (bgc_readl(M2M_BGC_VALID_SET_REG) & (1 << slot_index))
        {
            g_bgc_slot_ctrl |= 1 << slot_index;
        }
    }
}

mt_s32 m2m_bgc_slot_request(void)
{
    mt_u8 slot_index = 0;
    static mt_u8 one_time_flag = 0;

    if (one_time_flag == 0)
    {
        m2m_bgc_get_init_status();
        one_time_flag = 1;
    }

    for (slot_index = 0; slot_index < M2M_BGC_CMD_SLOT_BUTT; slot_index++)
    {
        if ((g_bgc_slot_ctrl >> (BGC_SLOT_LOCK_INDEX_BIT + slot_index)) & 1UL)
        {
            continue;
        }
        else
        {
            if (!((g_bgc_slot_ctrl >> slot_index) & 1UL))
            {
                g_bgc_slot_ctrl |= 1 << slot_index;
                break;
            }
        }
    }

    if (slot_index >= M2M_BGC_CMD_SLOT_BUTT)
    {
        return HW_CE_ERROR_GENERIC;
    }
    return slot_index;
}

/**
 * @brief This function will care semaphore timeout,
 *          how to do with timeout should designed by user.
 */
mt_s32 m2m_bgc_sem_wait(mt_u32 cpu_id, MT_BGC_SEM_REQ_S req_timeout)
{
    if (!(bgc_readl(M2M_BGC_SEMAPHORE_REG) & M2M_BGC_SEM_CPU_ID_MASK))
    {
        bgc_writel(cpu_id & M2M_BGC_SEM_CPU_ID_MASK, M2M_BGC_SEMAPHORE_REG);
        if ((bgc_readl(M2M_BGC_SEMAPHORE_REG) & M2M_BGC_SEM_CPU_ID_MASK) == cpu_id)
        {
            return HW_CE_SUCCESS;
        }
    }

    if (req_timeout == MT_BGC_SEM_REQ_ONE_TIME)
    {
        return HW_CE_ERROR_GENERIC;
    }

    while (1)
    {
        if (bgc_readl(M2M_BGC_SEMAPHORE_REG) & M2M_BGC_SEM_CPU_ID_MASK)
        {
            continue;
        }

        bgc_writel(cpu_id & M2M_BGC_SEM_CPU_ID_MASK, M2M_BGC_SEMAPHORE_REG);

        if ((bgc_readl(M2M_BGC_SEMAPHORE_REG) & M2M_BGC_SEM_CPU_ID_MASK) == cpu_id)
        {
            break;
        }
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_bgc_sem_post(void)
{
    bgc_writel(0, M2M_BGC_SEMAPHORE_REG);

    return HW_CE_SUCCESS;
}

mt_s32 m2m_bgc_sem_check(mt_u32 cpu_id)
{
    if ((bgc_readl(M2M_BGC_SEMAPHORE_REG) & M2M_BGC_SEM_CPU_ID_MASK) == cpu_id)
    {
        return HW_CE_SUCCESS;
    }

    return HW_CE_ERROR_GENERIC;
}

/*
 * @brief the LOCK REG is unreadable, it is inconsistent with the datasheet.
 */
static mt_s32 m2m_bgc_slot_is_lock(mt_u32 slot)
{
#if 1
    if ((g_bgc_slot_ctrl >> (BGC_SLOT_LOCK_INDEX_BIT + slot)) & 1UL)
    {
        return MT_TRUE;
    }
#else
    if (bgc_readl(M2M_BGC_LOCK_REG) & (0xf << (slot << 2)))
    {
        return MT_TRUE;
    }
#endif

    return MT_FALSE;
}

static mt_s32 m2m_bgc_slot_set_cmd(mt_u32 slot, const m2m_bgc_t *bgc)
{
    mt_s32 retry_times = 10;

    bgc_writel(slot, M2M_BGC_SET_SEL_REG);
    bgc_writel(bgc_readl(M2M_BGC_VALID_SET_REG) & ~(1 << slot),
               M2M_BGC_VALID_SET_REG);

    while (retry_times--) {
        if ((bgc_readl(M2M_BGC_UPDATE_ALLOW_REG) & 0x1) == 1)
        {
            break;
        }
        msleep(100);
    }
    if (retry_times <= 0) {
        return HW_CE_ERROR_GENERIC;
    }

    buf_to_be_reg((void *)M2M_BGC_SETn_REG(slot), bgc, sizeof(m2m_bgc_t));

    bgc_writel(bgc_readl(M2M_BGC_VALID_SET_REG) | (1 << slot),
               M2M_BGC_VALID_SET_REG);
    if (!(bgc_readl(M2M_BGC_VALID_SET_REG) & (1 << slot)))
    {
        return HW_CE_ERROR_GENERIC;
    }

    return HW_CE_SUCCESS;
}

static void m2m_bgc_clear_state(mt_u32 slot)
{
    bgc_writel(bgc_readl(M2M_BGC_STATE_CLEAR_REG) | (1 << slot),
               M2M_BGC_STATE_CLEAR_REG);
    bgc_writel(bgc_readl(M2M_INT_CLEAR_REG) | (1 << (slot + 4)), M2M_INT_CLEAR_REG);
}

mt_s32 m2m_bgc_slot_start(mt_u32 slot, const mt_u8 *addr, mt_u32 len,
                          const mt_u8 *digest, mt_u32 delay)
{
    m2m_bgc_t bgc;

    UNUSED(digest);

    if (slot >= M2M_BGC_CMD_SLOT_BUTT)
    {
        return HW_CE_ERROR_GENERIC;
    }

    if (m2m_bgc_slot_is_lock(slot))
    {
        return HW_CE_ERROR_GENERIC;
    }

    m2m_bgc_clear_state(slot);

    if (delay > M2M_BGC_CMD_DELAY_ONE_SEC)
    {
        delay = M2M_BGC_CMD_DELAY_ONE_SEC;
    }

    memset(&bgc, 0, sizeof(bgc));
    bgc.Cmd = HW_M2M_BGC;
    bgc.Q = 1;
    bgc.Profile = SHA2_256;
    bgc.Delay = delay;
    bgc.Enable = 0xf;
    bgc.TP = 0;
    bgc.SrcDataAddr = (mt_u32)(ulong)addr;
    bgc.SrcDataSize = len;

    if (((ulong)addr >= AHB_BSRAM_ADDR)
        && ((ulong)addr < AHB_BSRAM_ADDR + AHB_BSRAM_SIZE)) {
        bgc.AT = 1;
    }

#ifdef M2M_USE_VIRT_ADDR
    flush_dcache_range((ulong)addr, len);
#endif
    return m2m_bgc_slot_set_cmd(slot, &bgc);
}

mt_s32 m2m_bgc_slot_stop(mt_u32 slot)
{
    if (slot >= M2M_BGC_CMD_SLOT_BUTT)
    {
        return HW_CE_ERROR_GENERIC;
    }

    if (m2m_bgc_slot_is_lock(slot) == MT_TRUE)
    {
        return HW_CE_ERROR_GENERIC;
    }

    bgc_writel(slot, M2M_BGC_SET_SEL_REG);
    bgc_writel(bgc_readl(M2M_BGC_VALID_SET_REG) & ~(1 << slot),
               M2M_BGC_VALID_SET_REG);
    if (bgc_readl(M2M_BGC_VALID_SET_REG) & (1 << slot))
    {
        return HW_CE_ERROR_GENERIC;
    }

    g_bgc_slot_ctrl &= ~(1 << slot);

    return HW_CE_SUCCESS;
}

mt_s32 m2m_bgc_slot_lock(mt_u32 slot)
{
    if (slot >= M2M_BGC_CMD_SLOT_BUTT)
    {
        return HW_CE_ERROR_GENERIC;
    }

    bgc_writel(bgc_readl(M2M_BGC_LOCK_REG) | (0xf << (slot << 2)),
               M2M_BGC_LOCK_REG);
    g_bgc_slot_ctrl |= 1 << (BGC_SLOT_LOCK_INDEX_BIT + slot);

    if (m2m_bgc_slot_is_lock(slot) != MT_TRUE)
    {
        return HW_CE_ERROR_GENERIC;
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_bgc_get_state(mt_u32 slot, mt_u8 *checked_count,
                         mt_u8 *checked_result)
{
    mt_u32 state;
    mt_u32 slot_state;
    mt_u32 int_flag;

    if (slot >= M2M_BGC_CMD_SLOT_BUTT)
    {
        return HW_CE_ERROR_GENERIC;
    }

    state = bgc_readl(M2M_BGC_STATE_REG);
    slot_state = (state >> (slot << 3)) & 0xff;
    *checked_count = slot_state & 0x7f;
    *checked_result = slot_state >> 7;

    int_flag = (bgc_readl(M2M_INT_FLAG_REG) >> (4 + slot)) & 0x1;
    if (!(*checked_result) ^ !int_flag)
    {
        m2m_bgc_print_debug();
        return HW_CE_ERROR_GENERIC;
    }

    return HW_CE_SUCCESS;
}

void m2m_bgc_print_debug(void)
{
    mt_u64 reg;
    mt_u32 val;

    DMSG("==========m2m bgc debug info==========\n");
    reg = M2M_BGC_STATE_REG;
    val = bgc_readl(reg);
    DMSG("[%08llx] = %08x\n", reg, val);
    reg = M2M_INT_FLAG_REG;
    val = bgc_readl(reg);
    DMSG("[%08llx] = %08x\n", reg, val);
}
