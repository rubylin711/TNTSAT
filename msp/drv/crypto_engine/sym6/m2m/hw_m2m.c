/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "hw_m2m_if.h"
#include "hw_m2m_register.h"
#include "mt_lib.h"

#define M2M_MAX_LOOP_CNT 5000000

#ifdef CONFIG_MT_CRYPTOENGINE_SYM6_HW_IRQ_SUPPORT
DECLARE_WAIT_QUEUE_HEAD(m2m_desc_wqh);
DECLARE_WAIT_QUEUE_HEAD(m2m_cmd_wqh);
#endif

enum M2M_STATE {
    STATE_OTP_ALGO_DIS          = 1 << 0,  /* Algoriyhm is not supported according to OTP */
    STATE_PRF_UNDEF             = 1 << 1,  /* Profile is unknow */
    STATE_TDES_KEY_CHKFAIL      = 1 << 2,  /* TDES key check is failed */
    STATE_KEY_UNVLD             = 1 << 3,  /* Key is not valid */
    STATE_KEYUSAGE_FAIL         = 1 << 4,  /* Keyusage is not matched */
    STATE_KEYSIZE_FAIL          = 1 << 5,  /* Keysize is not matched */
    STATE_KT_ENC_DIS            = 1 << 6,  /* Key is not for encryption */
    STATE_KT_DEC_DIS            = 1 << 7,  /* Key is not for decryption */
    STATE_KT_M2M_DIS            = 1 << 8,  /* Key is not for M2M use */
    STATE_KT_MAC_DIS            = 1 << 9,  /* Key is not for MAC use */
    STATE_KT_ACPU_FORB          = 1 << 10, /* Key is not for REE-CPU, according to KTKeySlot.KeyAttr[17] and KTKeySlot.TP */
    STATE_DBUF_Source_RCID_ERR0 = 1 << 11, /* Data source RCID check err */
    STATE_DBUF_Source_RCID_ERR1 = 1 << 12, /* Data source RCID is forbidden by M2M_CHn_RCID_PERMIT_REG set */
    STATE_CLRDATA_RAM_REE_ERR   = 1 << 13, /* TS Mode: TS head error without "0x47"; RAM Mode: Indicate that output wcid/data of Clear Data/Head Clear/ Tail Clear/LOS is forced to 0, according to M2M REE CHs & RCID!=0 & RCID!=WCID */
    STATE_REE_CTR_DIS           = 1 << 15, /* REE CTR/CTR64/GCM is forbidden */
    STATE_DBUF_R_ADDR_ERR       = 1 << 16, /* Data source address is not allowed */
    STATE_DBUF_R_LEN_ERR        = 1 << 17, /* Data source length is 0 */
    STATE_DBUF_W_ADDR_ERR       = 1 << 18, /* Data destination address is not allowed */
    STATE_DBUF_W_LEN_ERR        = 1 << 19, /* Data destination length is 0 */
    STATE_DESC_RCID_ERR         = 1 << 20, /* Descriptor RCID is not allowed */
    STATE_DESC_ADDR_ERR         = 1 << 21, /* Descriptor address it not allowed */
    STATE_DBUF_WBACK_WCID_ERR   = 1 << 22, /* Dbuf write back to memory, WCID is forbidden */
    STATE_CMD_VALID_ERR         = 1 << 23, /* For channel 0/1, CMD must be 4b0000/4b0001 */
};

enum M2M_TS_ERR0 {
    ERR0_KT_SLOT_UNVLD       = 1 << 0,  /* KTKeySlot.SlotValid is not equal with 1 */
    ERR0_KT_AES192_ERR       = 1 << 1,  /* KTKey of M2M ALGO is AES192. */
    ERR0_KT_AES256_ERR       = 1 << 2,  /* KTKey of M2M ALGO is AES256. */
    ERR0_KT_AES_DIS          = 1 << 3,  /* M2M AES algorithm is disabled refer to kt_attribute. */
    ERR0_KT_TDES_DIS         = 1 << 4,  /* M2M TDES algorithm is disabled refer to kt_attribute. */
    ERR0_KT_AES_KSIZE_ERR    = 1 << 5,  /* AES KEY SIZE mismatch. */
    ERR0_KT_TDES_KSIZE_ERR   = 1 << 6,  /* TDES KEY SIZE mismatch. */
    ERR0_KT_M2M_DIS          = 1 << 7,  /* M2M algorithm is disabled refer to kt_attribute. */
    ERR0_KT_MAC_DIS          = 1 << 8,  /* MAC algorithm is disabled refer to kt_attribute. */
    ERR0_KT_DEC_DIS          = 1 << 9,  /* DEC is disabled refer to kt_attribute. */
    ERR0_KT_ENC_DIS          = 1 << 10, /* Field0000 Abstract. ENC is disabled refer to kt_attribute. */
    ERR0_KT_ACPU_FORB        = 1 << 11, /* ACPU is forbidden. */
    ERR0_KT_PRF_UNDEF        = 1 << 12, /* Profile is undefined in kt. */
    ERR0_DK_PRF_UNDEF        = 1 << 13, /* M2MCmd.Profile is undefined while dkey is used. */
    ERR0_TDESKEY_CHK_ERR     = 1 << 14, /* TDES KEY CHECK FAIL. */
    ERR0_TP_ERR              = 1 << 15, /* TP ERR. */
    ERR0_ALGO_OTPDIS         = 1 << 16, /* Algorithm disabled by OTP. */
    ERR0_KT_DES_DIS          = 1 << 17, /* M2M DES algorithm is disabled refer to kt_attribute. */
    ERR0_KT_DES_KSIZE_ERR    = 1 << 18, /* DES KEY SIZE mismatch. */
    ERR0_KEYSRC_OPT_DIS      = 1 << 19, /* Keysource is disabled by REG_KEYSRC_OPT. */
    ERR0_SCID_CHK_ERR        = 1 << 20, /* KTKeySlot.SCID/M2MCmd.SCID is not equal with RDCID. */
    ERR0_DATA_RCID_ERR       = 1 << 21, /* DATA RCID FORBIDDEN. */
    ERR0_CLRDATA_RAW_REE_ERR = 1 << 22, /* Indicate that output wcid/data of Clear Data/Head Clear/ Tail Clear/LOS is forced to 0, according to M2M REE CHs & RCID!=0 & RCID!=WCID. */
    ERR0_REE_CTR_DIS         = 1 << 23, /* REE CTR/CTR64/GCM is forbidden. */
};

enum M2M_TS_ERR1 {
    ERR1_TS_NOPLAYLOAD_TYPE0 = 1 << 4, /* No payload */
    ERR1_TS_NOPLAYLOAD_TYPE1 = 1 << 5, /* TS PID filter failed */
    ERR1_TS_NOPLAYLOAD_TYPE2 = 1 << 6, /* TS head error without 0x47 */
};

struct m2m_dbg_info {
    mt_u32 id;
    const char *info;
};

static struct m2m_dbg_info m2m_state_ring_infos[] = {
    {STATE_OTP_ALGO_DIS,          "Algoriyhm is not supported according to OTP."},
    {STATE_PRF_UNDEF,             "Profile is unknown."},
    {STATE_TDES_KEY_CHKFAIL,      "TDES key check is failed."},
    {STATE_KEY_UNVLD,             "Key is not valid."},
    {STATE_KEYUSAGE_FAIL,         "Keyusage is not matched."},
    {STATE_KEYSIZE_FAIL,          "Keysize is not matched."},
    {STATE_KT_ENC_DIS,            "Key is not for encryption."},
    {STATE_KT_DEC_DIS,            "Key is not for decryption."},
    {STATE_KT_M2M_DIS,            "Key is not for M2M use."},
    {STATE_KT_MAC_DIS,            "Key is not for MAC use."},
    {STATE_KT_ACPU_FORB,          "Key is not for REE-CPU, according to KTKeySlotKeyAttr[17] and KTKeySlotTP."},
    {STATE_DBUF_Source_RCID_ERR0, "Data source RCID check err."},
    {STATE_DBUF_Source_RCID_ERR1, "Data source RCID is forbidden by M2M_CHn_RCID_PERMIT_REG set."},
    {STATE_CLRDATA_RAM_REE_ERR,   "TS Mode: TS head error without 0x47; RAM Mode: Indicate that output wcid/data of Clear Data/Head Clear/ Tail Clear/LOS is forced to 0, according to M2M REE CHs & RCID!=0 & RCID!=WCID"},
    {STATE_REE_CTR_DIS,           "REE CTR/CTR64/GCM is forbidden."},
    {STATE_DBUF_R_ADDR_ERR,       "Data source address is not allowed."},
    {STATE_DBUF_R_LEN_ERR,        "Data source length is 0."},
    {STATE_DBUF_W_ADDR_ERR,       "Data destination address is not allowed."},
    {STATE_DBUF_W_LEN_ERR,        "Data destination length is 0."},
    {STATE_DESC_RCID_ERR,         "Descriptor RCID is not allowed."},
    {STATE_DESC_ADDR_ERR,         "Descriptor MagicNum it not correct."},
    {STATE_DBUF_WBACK_WCID_ERR,   "Dbuf write back to memory, WCID is forbidden."},
    {STATE_CMD_VALID_ERR,         "For channel 0/1, CMD must be 4b0000/4b0001"},
};

static struct m2m_dbg_info m2m_ts_err0_infos[] = {
    {ERR0_KT_SLOT_UNVLD,       "KTKeySlot.SlotValid is not equal with 1."},
    {ERR0_KT_AES192_ERR,       "KTKey of M2M ALGO is AES192."},
    {ERR0_KT_AES256_ERR,       "KTKey of M2M ALGO is AES256."},
    {ERR0_KT_AES_DIS,          "M2M AES algorithm is disabled refer to kt_attribute."},
    {ERR0_KT_TDES_DIS,         "M2M TDES algorithm is disabled refer to kt_attribute."},
    {ERR0_KT_AES_KSIZE_ERR,    "AES KEY SIZE mismatch."},
    {ERR0_KT_TDES_KSIZE_ERR,   "TDES KEY SIZE mismatch."},
    {ERR0_KT_M2M_DIS,          "M2M algorithm is disabled refer to kt_attribute."},
    {ERR0_KT_MAC_DIS,          "MAC algorithm is disabled refer to kt_attribute."},
    {ERR0_KT_DEC_DIS,          "DEC is disabled refer to kt_attribute."},
    {ERR0_KT_ENC_DIS,          "Field0000 Abstract. ENC is disabled refer to kt_attribute."},
    {ERR0_KT_ACPU_FORB,        "ACPU is forbidden."},
    {ERR0_KT_PRF_UNDEF,        "Profile is undefined in kt."},
    {ERR0_DK_PRF_UNDEF,        "M2MCmd.Profile is undefined while dkey is used."},
    {ERR0_TDESKEY_CHK_ERR,     "TDES KEY CHECK FAIL."},
    {ERR0_TP_ERR,              "TP ERR."},
    {ERR0_ALGO_OTPDIS,         "Algorithm disabled by OTP."},
    {ERR0_KT_DES_DIS,          "M2M DES algorithm is disabled refer to kt_attribute."},
    {ERR0_KT_DES_KSIZE_ERR,    "DES KEY SIZE mismatch."},
    {ERR0_KEYSRC_OPT_DIS,      "Keysource is disabled by REG_KEYSRC_OPT."},
    {ERR0_SCID_CHK_ERR,        "KTKeySlot.SCID/M2MCmd.SCID is not equal with RDCID."},
    {ERR0_DATA_RCID_ERR,       "DATA RCID FORBIDDEN."},
    {ERR0_CLRDATA_RAW_REE_ERR, "Indicate that output wcid/data of Clear Data/Head Clear/ Tail Clear/LOS is forced to 0, according to M2M REE CHs & RCID!=0 & RCID!=WCID."},
    {ERR0_REE_CTR_DIS,         "REE CTR/CTR64/GCM is forbidden."},
};

static struct m2m_dbg_info m2m_ts_err1_infos[] = {
    {ERR1_TS_NOPLAYLOAD_TYPE0, "No payload."},
    {ERR1_TS_NOPLAYLOAD_TYPE1, "TS PID filter failed."},
    {ERR1_TS_NOPLAYLOAD_TYPE2, "TS head error without 0x47."},
};

static mt_u8 m2m_cmd_slots[2][5] = { 0 };

static uint32_t m2m_extra_notices = 0;

/*int hexdump(const char *name, const void *addr, int len)
{
    int i = 0;
    
    if (name != 0)
    {
        printk("\n==================%s==================\n", name);
    }
    printk("Dump %08x(%d):", addr, len);
    for (i = 0; i < len; ++i)
    {
        if ((i % 16) == 0)
            printk("\n");

        printk("%02x ", *(unsigned char *)((unsigned long)addr + i));
    }
    printk("\n");

    return 0;
}*/

static mt_s32 m2m_alloc_cmd_slot(mt_u32 channel, mt_u8 tid)
{
    mt_u32 i;
    mt_u32 ch;

    ch = channel ? 1 : 0;
    for (i = 0; i < ARRAY_SIZE(m2m_cmd_slots[0]); i++)
    {
        if (m2m_cmd_slots[ch][i] == 0)
        {
            m2m_cmd_slots[ch][i] = tid;
            return HW_CE_SUCCESS;
        }
    }

    return HW_CE_ERROR_SECURITY;
}

static mt_s32 m2m_free_cmd_slot(mt_u32 channel, mt_u8 tid)
{
    mt_u32 i;
    mt_u32 ch;

    ch = channel ? 1 : 0;

    for (i = 0; i < ARRAY_SIZE(m2m_cmd_slots[0]); i++)
    {
        if (m2m_cmd_slots[ch][i] == tid)
        {
            m2m_cmd_slots[ch][i] = 0;
            return HW_CE_SUCCESS;
        }
    }

    DMSG("%s failed to kfree(((((((((((( slot(%d %d)\n", __FUNCTION__, channel, tid);
    return HW_CE_ERROR_SECURITY;
}

static inline mt_u8 m2m_get_tid_from_cmd(const void *cmd)
{
    m2m_cmd1_t *pcmd = (m2m_cmd1_t *)cmd;

    return pcmd->TID;
}

static void m2m_print_state_ring(mt_u32 channel, mt_u32 index)
{
    mt_u64 reg = M2M_CHn_STATE_RING_BUF(channel, index);
    mt_u32 val = io_read32(reg);
    mt_u32 i;

    DMSG("[%08llx] = %08x\n", reg, val);
    for (i = 0; i < ARRAY_SIZE(m2m_state_ring_infos); ++i)
    {
        if (val & m2m_state_ring_infos[i].id)
            EMSG("    ERROR: %s\n", m2m_state_ring_infos[i].info);
    }
}

static void m2m_print_ts_err0(mt_u32 channel)
{
    mt_u64 reg = M2M_CHn_TS_ERR0_REG(channel);
    mt_u32 val = io_read32(reg);
    mt_u32 i;

    DMSG("[%08llx] = %08x\n", reg, val);
    for (i = 0; i < ARRAY_SIZE(m2m_ts_err0_infos); ++i)
    {
        if (val & m2m_ts_err0_infos[i].id)
            EMSG("    ERROR: %s\n", m2m_ts_err0_infos[i].info);
    }

}

static void m2m_print_ts_err1(mt_u32 channel)
{
    mt_u64 reg = M2M_CHn_TS_ERR1_REG(channel);
    mt_u32 val = io_read32(reg);
    mt_u32 i;

    DMSG("[%08llx] = %08x\n", reg, val);
    for (i = 0; i < ARRAY_SIZE(m2m_ts_err1_infos); ++i)
    {
        if (val & m2m_ts_err1_infos[i].id)
            EMSG("    ERROR: %s\n", m2m_ts_err1_infos[i].info);
    }
}

static void m2m_print_last_cmd(mt_u32 channel)
{
    mt_u64 reg = M2M_CHn_CMD_EXTR_REG(channel);
    mt_u32 i;

    DMSG("===========m2m last cmd(CH%d)===========\n", channel);
    for (i = 0; i < 5; ++i)
    {
        DMSG("[%08llx] = %08x\n", reg, io_read32(reg));
        reg += 4;
    }
}

static void m2m_print_last_desc(mt_u32 channel)
{
    mt_u64 reg = M2M_CHn_DESC_EXTR_REG(channel);
    mt_u32 i;

    DMSG("===========m2m last desc(CH%d)==========\n", channel);
    for (i = 0; i < 7; ++i)
    {
        DMSG("[%08llx] = %08x\n", reg, io_read32(reg));
        reg += 4;
    }
}

static void m2m_print_state_desc(mt_u32 channel)
{
    mt_u64 reg = M2M_CHn_STATE_DESC(channel);

    DMSG("===========m2m state desc(CH%d)==========\n", channel);

    DMSG("[%08llx] = %08x\n", reg, io_read32(reg));
}

static int m2m_get_state(mt_u32 channel, mt_u8 tid, mt_u32 *state)
{
    int index = 0;
    mt_u32 value = 0;

    for (index = 0; index < M2M_MAX_RING_BUF_NUM; ++index)
    {
        value = io_read32(M2M_CHn_STATE_RING_BUF(channel, index));
        if ((value >> 24) == tid)
            break;
    }

    if (index >= M2M_MAX_RING_BUF_NUM)
    {
        /*DMSG("%s could not find tid 0x%02x\n", __FUNCTION__, tid);*/
        return -1;
    }

    if (state)
        *state = value;
    return index;
}

int m2m_check_state(mt_u32 channel, mt_u8 tid)
{
    int index;
    mt_u32 state;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if ((index = m2m_get_state(channel, tid, &state)) < 0)
        return -1;

    if (state & 0x00ffffff)
    {
        m2m_print_state_ring(channel, index);
        return -2;
    }

    return index;
}

mt_u8 m2m_alloc_cmd_tid(mt_u32 channel)
{
    static mt_u8 m2m_tid[2] = {0, 0};
    static mt_u8 inherit_tid[2] = {0, 0};
    mt_u8 *ptid;
    mt_u32 index = 0, val;
    mt_u8 residual_tid = 0, initial_tid = 0;

    if (!inherit_tid[channel]) {
        for (index = 0; index < M2M_MAX_RING_BUF_NUM; index++) {
            val = io_read32(M2M_CHn_STATE_RING_BUF(channel, index));
            if (((val >> 24) & 0xff) != 0)
                residual_tid = (val >> 24) & 0xff;
            if (initial_tid < residual_tid)
                initial_tid = residual_tid;
        }
        m2m_tid[channel] = initial_tid;
        inherit_tid[channel] = 1;
    }

    ptid = channel ? (m2m_tid + 1) : m2m_tid;
    if (++(*ptid) == 0)
        *ptid = 1;

    /*DMSG("tid(%d): %x\n", channel, *ptid);*/
    return (*ptid & 0xff);
}

void m2m_set_gost_param(mt_u32 channel, mt_u32 bit_inv, mt_u32 sbox_sel)
{
    io_write32(M2M_CHn_GOST_MODE_REG(channel), (bit_inv << 4) | sbox_sel);
}

mt_s32 m2m_send_cmd(mt_u32 channel, const void *cmd, mt_u32 cmd_size)
{
    const mt_u8 *p = cmd;
    mt_u32 cmd_word_size = cmd_size >> 2;
    volatile mt_u32 i = 0;

    if ((channel >= M2M_MAX_CHANNEL_NUM) ||
            !cmd ||
            !cmd_word_size ||
            (cmd_word_size > M2M_CHn_CMD_QUEUE_CNT_MAX))
        return HW_CE_ERROR_BAD_PARAMETERS;

    while (1)
    {
        if (m2m_alloc_cmd_slot(channel, m2m_get_tid_from_cmd(cmd)) == 0)
            break;
    }

    while ((io_read32(M2M_CHn_CMD_QUEUE_CNT_REG(channel)) + cmd_word_size)
            >= M2M_CHn_CMD_QUEUE_CNT_MAX)
    {
#ifdef M2M_MAX_LOOP_CNT
        if (++i > M2M_MAX_LOOP_CNT)
        {
            DMSG("%s timeout\n", __FUNCTION__);
            return HW_CE_ERROR_BUSY;
        }
#endif
    }

    /*hexdump("cmd", cmd, cmd_size);*/

    for (i = 0; i < cmd_word_size; i++)
    {
        /*DMSG("%08x %08x\n", M2M_CHn_CMD_QUEUE_REG(channel), get_u32(p));*/
        io_write32(M2M_CHn_CMD_QUEUE_REG(channel), get_u32(p));
        p += 4;
    }

    return HW_CE_SUCCESS;
}

void m2m_extra_notice(M2M_EXTRA_NOTICE_TYPE message, M2M_EXTRA_NOTICE_MODE op)
{
    if (op == M2M_EXT_NOTICE_MODE_REGISTER) {
        m2m_extra_notices |= (1 << message);
    } else {
        if (message == M2M_EXT_NOTICE_CLEAN) {
            m2m_extra_notices = 0UL;
        } else {
            m2m_extra_notices &= ~(1 << message);
        }
    }
}

bool is_exist_m2m_notice(M2M_EXTRA_NOTICE_TYPE message)
{
    return (((m2m_extra_notices >> message) & 1) == 1);
}

#ifdef CONFIG_MT_CRYPTOENGINE_SYM6_HW_IRQ_SUPPORT
mt_s32 m2m_check_cmd_irq(mt_u32 channel)
{
    mt_u32 flag = channel ? 0x4 : 0x1;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if ((io_read32(M2M_INT_FLAG_REG) & flag) == 0)
        return HW_CE_ERROR_BUSY;

    io_write32(M2M_INT_CLEAR_REG, flag);

    wake_up_interruptible(&m2m_cmd_wqh);
    return HW_CE_SUCCESS;
}
#else
static mt_s32 m2m_check_cmd_irq(mt_u32 channel)
{
    mt_u32 flag = channel ? 0x4 : 0x1;
#ifdef M2M_MAX_LOOP_CNT
    volatile mt_u32 i = 0;
#endif

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    while (0 == (io_read32(M2M_INT_FLAG_REG) & flag))
    {
#ifdef M2M_MAX_LOOP_CNT
        if (++i > M2M_MAX_LOOP_CNT)
        {
            DMSG("%s timeout\n", __FUNCTION__);
            return HW_CE_ERROR_BUSY;
        }
#endif
    }

    io_write32(M2M_INT_CLEAR_REG, flag);
    return HW_CE_SUCCESS;
}
#endif

static mt_s32 m2m_check_cmd_irq_polling(mt_u32 channel)
{
    mt_u32 flag = channel ? 0x4 : 0x1;
#ifdef M2M_MAX_LOOP_CNT
    volatile mt_u32 i = 0;
#endif

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    while (0 == (io_read32(M2M_INT_FLAG_REG) & flag))
    {
#ifdef M2M_MAX_LOOP_CNT
        if (++i > M2M_MAX_LOOP_CNT)
        {
            DMSG("%s timeout\n", __FUNCTION__);
            return HW_CE_ERROR_BUSY;
        }
#endif
    }

    io_write32(M2M_INT_CLEAR_REG, flag);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_wait_cmd_finish(mt_u32 channel, mt_u8 tid)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

#ifdef CONFIG_MT_CRYPTOENGINE_SYM6_HW_IRQ_SUPPORT
    if (is_exist_m2m_notice(M2M_EXT_NOTICE_NOIRQ)) {
        res = m2m_check_cmd_irq_polling(channel);
        if (res != HW_CE_SUCCESS)
            return res;
    } else {
        if (wait_event_interruptible(m2m_cmd_wqh, (m2m_check_state(channel, tid) != -1)) < 0)
            return HW_CE_ERROR_BUSY;
    }
#else
    res = m2m_check_cmd_irq(channel);
    if (res != HW_CE_SUCCESS)
        return res;
#endif

    res = m2m_free_cmd_slot(channel, tid);
    if (res != HW_CE_SUCCESS)
        return res;

    if (m2m_check_state(channel, tid) < 0)
        return HW_CE_ERROR_BAD_STATE;

    return HW_CE_SUCCESS;
}

#ifdef CONFIG_MT_CRYPTOENGINE_SYM6_HW_IRQ_SUPPORT
mt_s32 m2m_check_desc_irq(mt_u32 channel)
{
    mt_u32 flag = channel ? 0x8 : 0x2;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    if ((io_read32(M2M_INT_FLAG_REG) & flag) == 0)
        return HW_CE_ERROR_BUSY;

    io_write32(M2M_INT_CLEAR_REG, flag);

    wake_up_interruptible(&m2m_desc_wqh);
    return HW_CE_SUCCESS;
}
#else
static mt_s32 m2m_check_desc_irq(mt_u32 channel)
{
    mt_u32 flag = channel ? 0x8 : 0x2;
#ifdef M2M_MAX_LOOP_CNT
    volatile mt_u32 i = 0;
#endif

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    while (0 == (io_read32(M2M_INT_FLAG_REG) & flag))
    {
#ifdef M2M_MAX_LOOP_CNT
        if (++i > M2M_MAX_LOOP_CNT)
        {
            DMSG("%s timeout\n", __FUNCTION__);
            return HW_CE_ERROR_BUSY;
        }
#endif
    }

    io_write32(M2M_INT_CLEAR_REG, flag);
    return HW_CE_SUCCESS;
}
#endif

static mt_s32 m2m_check_desc_irq_polling(mt_u32 channel)
{
    mt_u32 flag = channel ? 0x8 : 0x2;
#ifdef M2M_MAX_LOOP_CNT
    volatile mt_u32 i = 0;
#endif

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    while (0 == (io_read32(M2M_INT_FLAG_REG) & flag))
    {
#ifdef M2M_MAX_LOOP_CNT
        if (++i > M2M_MAX_LOOP_CNT)
        {
            DMSG("%s timeout\n", __FUNCTION__);
            return HW_CE_ERROR_BUSY;
        }
#endif
    }

    io_write32(M2M_INT_CLEAR_REG, flag);
    return HW_CE_SUCCESS;
}

static mt_s32 m2m_check_desc_state(mt_u32 channel, mt_u8 tid)
{
    mt_u32 desc;

    desc = io_read32(M2M_CHn_STATE_DESC(channel));
    if (tid != ((desc >> 8) & 0xff))
    {
        DMSG("[%08lx] = %08x\n", M2M_CHn_STATE_DESC(channel), desc);
        DMSG("%s invalid tid: 0x%02x\n", __FUNCTION__, tid);
        return HW_CE_ERROR_GENERIC;
    }

    return HW_CE_SUCCESS;
}

mt_s32 m2m_wait_desc_finish(mt_u32 channel, mt_u8 tid)
{
    mt_s32 res = HW_CE_ERROR_GENERIC;

#ifdef CONFIG_MT_CRYPTOENGINE_SYM6_HW_IRQ_SUPPORT
    if (is_exist_m2m_notice(M2M_EXT_NOTICE_NOIRQ)) {
        res = m2m_check_desc_irq_polling(channel);
        if (res != HW_CE_SUCCESS)
            return res;

        res = m2m_check_desc_state(channel, tid);
        if (res != HW_CE_SUCCESS)
            return res;
    } else {
        if (wait_event_interruptible(m2m_desc_wqh, (m2m_check_desc_state(channel, tid) == HW_CE_SUCCESS)) < 0)
            return HW_CE_ERROR_BUSY;
    }
#else
    res = m2m_check_desc_irq(channel);
    if (res != HW_CE_SUCCESS)
        return res;

    res = m2m_check_desc_state(channel, tid);
    if (res != HW_CE_SUCCESS)
        return res;
#endif

    return HW_CE_SUCCESS;
}

mt_s32 m2m_read_final_iv(mt_u32 channel, mt_u8 tid, mt_u8 *iv, mt_u32 len)
{
    int index;

    if ((channel >= M2M_MAX_CHANNEL_NUM) ||
            !iv ||
            !len)
        return HW_CE_ERROR_BAD_PARAMETERS;

    index = m2m_check_state(channel, tid);
    if (index < 0)
        return HW_CE_ERROR_SECURITY;

    le_reg_to_buf(iv, (const void *)M2M_CHn_TEMPOUT0_REG(channel, index), len);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_read_final_hash(mt_u32 channel, mt_u8 tid, mt_u8 *hash, mt_u32 len)
{
    int index;

    if ((channel >= M2M_MAX_CHANNEL_NUM) ||
            !hash ||
            !len)
        return HW_CE_ERROR_BAD_PARAMETERS;

    index = m2m_check_state(channel, tid);
    if (index < 0)
        return HW_CE_ERROR_SECURITY;

    be_reg_to_buf(hash, (const void *)(ulong)M2M_CHn_TEMPOUT1_REG(channel, index), len);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_read_iv(mt_u32 channel, mt_u8 *iv, mt_u32 len)
{
    if ((channel >= M2M_MAX_CHANNEL_NUM) ||
            !iv ||
            !len)
        return HW_CE_ERROR_BAD_PARAMETERS;

    le_reg_to_buf(iv, (const void *)(ulong)M2M_CHn_IV_CONT_REG(channel), len);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_write_iv(mt_u32 channel, const mt_u8 *iv, mt_u32 len)
{
    if ((channel >= M2M_MAX_CHANNEL_NUM) ||
            !iv ||
            !len)
        return HW_CE_ERROR_BAD_PARAMETERS;

    buf_to_le_reg((void *)(ulong)M2M_CHn_CMD0_REG_DIV_REG(channel), iv, len);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_read_hash(mt_u32 channel, mt_u8 *hash, mt_u32 len)
{
    if (channel >= M2M_MAX_CHANNEL_NUM)
        return HW_CE_ERROR_BAD_PARAMETERS;

    be_reg_to_buf(hash, (const void *)(ulong)M2M_CHn_GHASH_CONT_REG(channel), len);
    return HW_CE_SUCCESS;
}

mt_s32 m2m_write_hash(mt_u32 channel, const mt_u8 *hash, mt_u32 len)
{
    if ((channel >= M2M_MAX_CHANNEL_NUM) ||
            !hash ||
            !len)
        return HW_CE_ERROR_BAD_PARAMETERS;

    buf_to_be_reg((void *)(ulong)M2M_CHn_CMD0_REG_TEMPIN_REG(channel), hash, len);
    return HW_CE_SUCCESS;
}

/*
 * @brief Calculate buffer 'phy_addr' is contained in area 'AXI' or 'AHB'
 *   return 0: 'phy_addr' belong to AXI bus;
 *          1: 'phy_addr' belong to AHB bus;
 */
mt_u8 m2m_get_mapping_bus(mt_u32 phy_addr, mt_u32 len)
{
    if ((phy_addr >= AHB_BSRAM_ADDR) && (phy_addr - 1 + len <= AHB_BSRAM_ADDR - 1 + AHB_BSRAM_SIZE))
        return 1;
    else
        return 0;
}

void m2m_print_debug(mt_u32 channel)
{
    mt_u32 i;

    if (channel >= M2M_MAX_CHANNEL_NUM)
        return;

    DMSG("==========m2m debug info(CH%d)==========\n", channel);
    for (i = 0; i < M2M_MAX_RING_BUF_NUM; i++)
    {
        m2m_print_state_ring(channel, i);
    }
    m2m_print_ts_err0(channel);
    m2m_print_ts_err1(channel);
    m2m_print_last_cmd(channel);
    m2m_print_last_desc(channel);
    m2m_print_state_desc(channel);
}
