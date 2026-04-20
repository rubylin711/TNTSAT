/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include "hw_keyladder_bare_api.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "../keytable/sym6/hw_kt_register.h"
#include "../keytable/sym6/hw_kt_if.h"

#define KL_DBG_ON  0
#define KL_WRN_ON  0
#define KL_ERR_ON  0

#define KL_LOG(flags, fmt, arg...) \
    do {                            \
        if (flags)                  \
            printk(fmt, ##arg); \
    } while (0)

#define KL_DBG(fmt, arg...) \
    KL_LOG(KL_DBG_ON, "[KL] "fmt, ##arg)

#define KL_WRN(fmt, arg...) \
    KL_LOG(KL_WRN_ON, "[KL WRN] "fmt, ##arg)

#define KL_ERR(fmt, arg...) \
    KL_LOG(KL_ERR_ON, "[KL ERR] %s():%d, "fmt, __func__, __LINE__, ##arg)

//#define KL_DEBUG_INTERNAL
#ifdef KL_DEBUG_INTERNAL
/*
 * @brief this is for internal debug.
 */
typedef struct {
    unsigned short  slot_id;
    unsigned short  cmd_level;
    KL_CA_MODE      ca_mode;
	unsigned long   cmd_ram[64];
    unsigned char   data[6][16];
    unsigned char   signature[32];
    unsigned char   trigger_pos[8];
    unsigned char   tdc_data[416];
} debug_cmd_t;
static debug_cmd_t *debug_kl_cmd_t = NULL;

static int debug_kl_start(void)
{
    debug_kl_cmd_t = (debug_cmd_t *)kmalloc(sizeof(debug_cmd_t), GFP_KERNEL);
    if (!debug_kl_cmd_t) {
        return -1;
    }
    memset(debug_kl_cmd_t, 0, sizeof(debug_cmd_t));
    return 0;
}

static void debug_kl_end(void)
{
    kfree((void *)debug_kl_cmd_t);
}

static void kl_hex_dump_bytes(const void *addr, unsigned int len)
{
	unsigned int i;
	const unsigned char *add = addr;

	if ((unsigned int)add & 0x0f) {
		printk("[%p]: ", add);
	}

	for (i = 0; i < len; ++i) {
		if (((unsigned int)add & 0x0f) == 0x0) {
			printk("\n[%p]: ", add);
		}
		printk("%02x ", *add++);
	}
	printk("\n");
}
#endif

extern void kl_delay_function(unsigned long ms);

static kl_handle_t kl_handle;
static unsigned int g_kl_extra_condition = 0UL;

#define kl_ARRAY_SIZE(n)       (sizeof(n)/sizeof(n[0]))

static inline mt_u32 kl_readl(ulong addr)
{
    return HAL_GET_U32((volatile void *)addr);
}

static inline void kl_writel(mt_u32 val, ulong addr)
{
    HAL_PUT_U32((volatile void *)addr, val);
}

static void memcpy_from_reg(unsigned char *dest, unsigned char *reg, int len)
{
    int i = 0;
    unsigned long value = 0;

    for (i = 0; i < len / 4; i++) {
        value = HAL_GET_U32((volatile u32 *)(reg + i * 4));
        dest[i * 4 + 0] = (value >> 0) & 0xFF;
        dest[i * 4 + 1] = (value >> 8) & 0xFF;
        dest[i * 4 + 2] = (value >> 16) & 0xFF;
        dest[i * 4 + 3] = (value >> 24) & 0xFF;
    }
}

static void memcpy_to_reg(unsigned char *reg, unsigned char *src, int len)
{
    int i = 0;
    unsigned long value = 0;

    for (i = 0; i < len / 4; i++) {
        value = ((unsigned long)src[i * 4 + 0] << 0) | ((unsigned long)src[i * 4 + 1] << 8) | ((unsigned long)src[i * 4 + 2] << 16) | ((unsigned long)src[i * 4 + 3] << 24);
        HAL_PUT_U32((volatile u32 *)(reg + i * 4), value);
    }
}

/**
 * @brief check even 1+1 = 0 (even), so if parity is 1, then return 1.
 */
static mt_u8 kl_cmd_parity(mt_u32 cmd)
{
    mt_u8 i = 0, parity = 0;

    for (i = 0; i < 32; i++) {
        if (cmd & (1 << i)) {
            parity++;
        }
    }

    return (parity & 0x1);
}

static void kl_cmd_ram_init(kl_handle_t *handle)
{
    handle->cmd_cnt = 0;
    handle->data_pos = 0;

    memset(handle->cmd_ram, 0, sizeof(handle->cmd_ram));
}

static int kl_get_ca_mode(mt_u16 *ram_cmd, mt_u32 cmd_cnt, KL_CA_MODE *ca_mode)
{
    mt_u32 i = 0, j = 0;
    KL_RAM_CMD_TYPE cmd_type;
    mt_u16 cmd_src, cmd_dst, *cmd_value = ram_cmd;
    KL_SCK_SOURCE_E seclt_src[] = {KL_SELECT_SRC_CFAES_KEY, KL_SELECT_SRC_CFCWC};
    KL_MOVE_SRC_E move_src[] = {KL_MOVE_SRC_INVT_DOUT, KL_MOVE_SRC_CWCW0, KL_MOVE_SRC_CWCW1, KL_MOVE_SRC_CWCW2, KL_MOVE_SRC_CWCW3};
    KL_STORE_DST_E rstr_dst[] = {KL_STORE_DST_PRIVATE0, KL_STORE_DST_PRIVATE1};

    if (!ram_cmd) {
        return ERR_FAILURE;
    }

    for (j = 0; j < cmd_cnt; j++, cmd_value++) {
        *ca_mode = KL_CA_GENERIC;
        cmd_type = *cmd_value & KL_CMD_TYPE_MASK;
        cmd_src = kl_get_bit_val(*cmd_value, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
        cmd_dst = kl_get_bit_val(*cmd_value, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);

        switch (cmd_type) {
        case KL_CMD_TYPE_SELECT_SCK:
            for (i = 0; i < kl_ARRAY_SIZE(seclt_src); i++) {
                if (cmd_src == seclt_src[i]) {
                    *ca_mode = KL_CA_CRI;
                    break;
                }
            }
            break;
        case KL_CMD_TYPE_MOVE:
            if ((cmd_src == move_src[0]) || (cmd_dst == KL_MOVE_DST_INVT_DIN)) {
                *ca_mode = KL_CA_IRDETO;
                break;
            }
            for (i = 1; i < kl_ARRAY_SIZE(move_src) - 1; i++) {
                if (cmd_src == move_src[i]) {
                    *ca_mode = KL_CA_CRI;
                    break;
                }
            }
            break;
        case KL_CMD_TYPE_STORE_TO_PRIVATE:
            if (cmd_src == KL_CMD_RESTORE_SRC_SCK_SELECTED) {
                *ca_mode = KL_CA_CRI;
                break;
            }
            if ((cmd_dst != rstr_dst[0]) && (cmd_dst != rstr_dst[1])) {
                *ca_mode = KL_CA_CRI;
            }
            break;
        case KL_CMD_TYPE_ExtrTDC:
            *ca_mode = KL_CA_IRDETO;
            break;
        default:
            break;
        }

        if (*ca_mode != KL_CA_GENERIC) {
            break;
        }
    }

    return SUCCESS;
}

static int kl_exec_cmd(kl_handle_t *handle)
{
    mt_u32 i = 0, cmd = 0, tag = 0, step = 0;
    mt_u16 *p_input_cmd = handle->cmd_ram;
    mt_u8  parity = 0;
    KL_CA_MODE ca_mode = KL_CA_GENERIC;

#if 0
	/* Wrong way to clear cmd RAM!! Should use kl_writel!!! */
    //memset((void *)KEYLADDER_REG_RAM_CMD, 0, (KEYLADDER_CMD_RAM_SIZE * 4));
#else
	for (i = 0; i < KEYLADDER_CMD_RAM_SIZE; i++) {
		kl_writel(0, (mt_u64)(KEYLADDER_REG_RAM_CMD + 4 * i));
	}
#endif

    if (kl_get_ca_mode(p_input_cmd, handle->cmd_cnt, &ca_mode)) {
        return ERR_FAILURE;
    }

    for (i = 0; i < KEYLADDER_CMD_RAM_SIZE; i++) {
        if ((p_input_cmd[i] & KL_CMD_TYPE_MASK) == KL_CMD_TYPE_SELECT_SCK) {
            tag++;
            step = 0;
        } else {
            step++;
        }

        cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step, KL_RAM_CMD_STEP_SHIFT) | p_input_cmd[i];
        parity = kl_cmd_parity(cmd);
        cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
        kl_writel(cmd, (mt_u64)(KEYLADDER_REG_RAM_CMD + 4 * i));
    }

    /* 0 generic;  1 irdeto; 2 cri */
    cmd = kl_set_bit_val(handle->slot_id, KL_TRG_KEYTB_IDX_SHIFT, KL_TRG_KEYTB_IDX_MASK) | \
    kl_set_bit_val(ca_mode, KL_TRG_CA_SEL_SHIFT, KL_TRG_CA_SEL_MASK) | KL_TRG_START_EN;
    if (kl_get_bit_val(g_kl_extra_condition, KL_ADDT_EXPORTSWAP, 1)) {
        cmd |= KL_TRG_EXPORT_SWAP_BIT;
    }
    kl_writel(cmd, KEYLADDER_REG_CMD_TRI);

    if (!(kl_readl(KEYLADDER_REG_CMD_TRI_ACCEPT) & KL_TRG_ACCEPT_VALID)) {
        return ERR_FAILURE;
    }

    return SUCCESS;
}

static int kl_set_cmd(kl_handle_t *handle, unsigned short cmd)
{
    int ret = 0;

    if (handle->cmd_cnt < KEYLADDER_CMD_RAM_SIZE) {
        handle->cmd_ram[handle->cmd_cnt] = cmd;
        handle->cmd_cnt++;

        if (((cmd & KL_CMD_TYPE_MASK) == KL_CMD_TYPE_STORE_TO_PRIVATE)
			|| ((cmd & KL_CMD_TYPE_MASK) == KL_CMD_TYPE_EXPORT_KEY)
			|| ((cmd & KL_CMD_TYPE_MASK) == KL_CMD_TYPE_ExtrTDC)) {
#ifdef KL_DEBUG_INTERNAL
            extern int debug_exec_cmd(kl_handle_t *handle);
            ret = debug_exec_cmd(handle);
#else
            ret = kl_exec_cmd(handle);
#endif
            kl_cmd_ram_init(handle);
            return ret;
        } else {
            return SUCCESS;
        }
    }

    return ERR_FAILURE;
}

static int kl_set_cmd2(kl_handle_t *handle, unsigned short cmd, unsigned char exec)
{
    int ret = 0;

    if (handle->cmd_cnt < KEYLADDER_CMD_RAM_SIZE) {
        handle->cmd_ram[handle->cmd_cnt] = cmd;
        handle->cmd_cnt++;

        if ((exec) || ((cmd & KL_CMD_TYPE_MASK) == KL_CMD_TYPE_EXPORT_KEY)) {
#ifdef KL_DEBUG_INTERNAL
jg
            extern int debug_exec_cmd(kl_handle_t *handle);
            ret = debug_exec_cmd(handle);
#else
            ret = kl_exec_cmd(handle);
#endif
            kl_cmd_ram_init(handle);

            return ret;
        } else {
            return SUCCESS;
        }
    }

    return ERR_FAILURE;
}

static int kl_set_tdc_input(kl_handle_t *handle, unsigned char *p_input)
{
    if (p_input) {
        memcpy_to_reg((mt_u8 *)KEYLADDER_REG_TDC_RAM, p_input, 416);
#ifdef KL_DEBUG_INTERNAL
        memcpy(debug_kl_cmd_t->tdc_data, p_input, 416);
#endif
        return SUCCESS;
    }

    return ERR_FAILURE;
}

int kl_set_input(kl_handle_t *handle, unsigned char *p_input)
{
    if ((p_input != NULL) && (handle->data_pos < 6)) {
        memcpy_to_reg((mt_u8 *)KL_GET_CPUn_REG_ADDR(handle->data_pos), p_input, KL_DATA_CPUn_BLOCK_SIZE);
#ifdef KL_DEBUG_INTERNAL
        memcpy(&debug_kl_cmd_t->data[handle->data_pos][0], p_input, KL_DATA_CPUn_BLOCK_SIZE);
        debug_kl_cmd_t->cmd_level = handle->data_pos;
#endif
        handle->data_pos++;
        return SUCCESS;
    }

    return ERR_FAILURE;
}

static int kl_temporary_save_key(kl_handle_t *handle)
{
    mt_u16 cmd = 0;
    mt_u8  zero_input[16];

    if (handle->key_pos == KL_CMD_MOVE_SRC_XOR_DOUT) {
        return ERR_FAILURE;
    }

    memset(zero_input, 0, sizeof(zero_input));
    if (kl_set_input(handle, zero_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    /* connect cmd step1: move */
    cmd = KL_CMD_TYPE_MOVE | (KL_RAM_CMD_MOVE_SRC)handle->key_pos | KL_CMD_MOVE_DST_XOR_DIN_A | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    /* connect cmd step1: move & trigger */
    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | KL_CMD_MOVE_DST_XOR_DIN_B | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    handle->key_pos = KL_CMD_MOVE_SRC_XOR_DOUT;

    return SUCCESS;
}

int bare_kl_addt_condition_op(KL_ADDITIONS_E addt, KL_FUNC_ENABLE_E en)
{
    if (en == KL_ENABLE) {
        g_kl_extra_condition |= 1 << (mt_u32)addt;
    } else {
        g_kl_extra_condition &= ~(1 << (mt_u32)addt);
    }

    return SUCCESS;
}

/*
 *KEY table  driver
 */
#define KT_EXPORT_KEY   KT_RD_KEY_127T96
int bare_kl_read_key_from_keytable(unsigned int slot_id, unsigned char *p_key_buffer)
{
	int valid = 0;
	/*debug mode for key table can be read*/
	kl_writel(1, KT_RD_KEY_DEBUG);

	hw_kt_read_valid(slot_id, (HW_KT_SLOT_VALID_E *)&valid);
	if(p_key_buffer) {
		while(kl_readl(KT_START) & 1);
		kl_writel(0, KT_ENDIAN);
		kl_writel((HW_KT_OPER_R | (slot_id << 8)), KT_OPERATION);
		kl_writel(1, KT_START);
		while(kl_readl(KT_START) & 1);
		memcpy_from_reg(p_key_buffer, (unsigned char *)KT_EXPORT_KEY, 16);
		return SUCCESS;
	}
	return ERR_FAILURE;
}

/**
 * @brief This function do not care semaphore timeout, you should consider it in upper interface.
 */
int bare_kl_sem_request(void *bhandle, KL_SEM_CPU_ID cpu_id)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 value = 0;

    value = kl_readl(KEYLADDER_REG_CMD_SEM);
    if (kl_get_bit_val(value, KL_SEM_ACCESS_SHIFT, KL_SEM_ACCESS_MASK)) {
        return ERR_TIMEOUT;
    }

    /* the occupy mechanism only works for REE CPU, and occupy time in units of 256 clocks, so ignore it temporary. */
    kl_writel(kl_bits(0xfffffff, KL_SEM_TIME_INIT_SHIFT) | KL_SEM_REQ_VAL_BIT | cpu_id, KEYLADDER_REG_CMD_SEM);

    value = kl_readl(KEYLADDER_REG_CMD_SEM);
    if((KL_SEM_REQ_VAL_BIT | cpu_id) != kl_set_bit_val(value, KL_SEM_ACCESS_SHIFT, KL_SEM_ACCESS_MASK)) {
        return ERR_FAILURE;
    }

    return SUCCESS;
}

/**
 * @brief This function care semaphore timeout, the thread execution permission
 *         is granted every 20 milliseconds until the release is successful.
 */
int bare_kl_sem_release(void *bhandle)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    while (1) {
        if (!(kl_readl(KEYLADDER_REG_STATE_CHECK) & KL_CMDST_BUSY)) {
            break;
        }
        kl_delay_function(20);
    }

    kl_writel(0, KEYLADDER_REG_CMD_SEM);

    return SUCCESS;
}

int bare_kl_state_busy(void *bhandle)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 cmd = 0, time_out = 20;

    while (time_out--) {
        cmd = kl_readl(KEYLADDER_REG_STATE_CHECK);
        if (!(cmd & KL_CMDST_BUSY)) {
            break;
        }

        kl_delay_function(1);
    }

    return cmd == 0 ? SUCCESS : cmd;
}

int bare_kl_init(void **p_bhandle)
{
    memset(&kl_handle, 0, sizeof(kl_handle_t));

    kl_handle.base_address = SYMPHONY_KEYLADDER_REGISTER_VIRT_BASE;
    *p_bhandle = (void *)&kl_handle;

    kl_cmd_ram_init(&kl_handle);

#ifdef KL_DEBUG_INTERNAL
    if (debug_kl_start()) {
        return ERR_FAILURE;
    }
#endif

    return SUCCESS;
}

int bare_kl_deinit(void *bhandle)
{
    g_kl_extra_condition = 0UL;

    kl_cmd_ram_init((kl_handle_t *)bhandle);

    return SUCCESS;
}

int bare_kl_set_signature(void *bhandle, unsigned char *p_signature)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    memcpy_to_reg((mt_u8 *)(KEYLADDER_REG_SIGNATURE), p_signature, KL_SIGNATURE_LEN);
#ifdef KL_DEBUG_INTERNAL
    memcpy(debug_kl_cmd_t->signature, p_signature, KL_SIGNATURE_LEN);
#endif

    return SUCCESS;
}

int bare_kl_select_rootkey(void *bhandle, KL_SCK_SOURCE_E rootkey_src)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 rootkey = kl_set_bit_val(rootkey_src, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
    mt_u32 sect_dst = kl_set_bit_val((mt_u32)~rootkey_src, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);
    mt_u16 cmd = KL_CMD_TYPE_SELECT_SCK | rootkey | sect_dst | KL_CMD_SECLT_ADDs;

    kl_set_cmd(handle, cmd);
    handle->key_pos = KL_CMD_MOVE_SRC_SCK_SELECTED;

    return SUCCESS;
}

void bare_kl_select_keypos(void *bhandle, KL_MOVE_SRC_E key_pos)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    handle->key_pos = kl_set_bit_val(key_pos, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
}

int bare_kl_store_tdc(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u16 cmd = 0;
    unsigned long err_code = 0;

    /* Sym4 add tdc update mode, but irdeto not used, close it. */
    kl_writel(0, KEYLADDER_REG_UPDATE_MODE);

    /* set input to reg_cpu0(fixed) */
    if (kl_set_tdc_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    /* set aes_key */
    cmd = KL_CMD_TYPE_MOVE | handle->key_pos | KL_CMD_MOVE_DST_AES_KEY | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    /* extr tdc */
    cmd = KL_CMD_TYPE_ExtrTDC | KL_RAM_CMD_ExtrTDC_SRC | KL_RAM_CMD_ExtrTDC_DST | KL_RAM_CMD_ExtrTDC_ADDs;
    kl_set_cmd(handle, cmd);

    return bare_kl_wait_complete(bhandle, &err_code);
}

int bare_kl_execute_tdc(void *bhandle, unsigned char *p_input, KL_MOVE_ALGO_E algo_type, KL_MOVE_ENC_E enc_type)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 alog_t = kl_set_bit_val(algo_type, KL_RAM_CMD_MOVE_AES_SHIFT, KL_RAM_CMD_MOVE_AES_MASK);
    mt_u32 enc_t = kl_set_bit_val(enc_type, KL_RAM_CMD_MOVE_ENC_SHIFT, KL_RAM_CMD_MOVE_ENC_MASK);
    mt_u16 cmd = 0;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | \
    KL_CMD_MOVE_DST_INVT_DIN | alog_t | enc_t | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    return SUCCESS;
}

/* CEK_pT -> CEK_p -> CEK  by AES */
int bare_kl_execute_TDC_AES(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;

    /* 1,do TDC  CEK_pT -> CEK_p */
    /* set input */
    if(kl_set_input(handle, p_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }
    cmd = KL_CMD_TYPE_MOVE|KL_MOVE_SRC_REG_CPUn(handle->data_pos-1)|KL_CMD_MOVE_DST_INVT_DIN|KL_RAM_CMD_MOVE_ADDs_MV_TRG|4;
    kl_set_cmd(handle, cmd);

    /* 2,get CEK    CEK_p -> CEK ,by AES decryption */
    /* set input to aes_din */
    cmd = KL_CMD_TYPE_MOVE|KL_CMD_MOVE_SRC_INVT_DOUT|KL_CMD_MOVE_DST_AES_DIN|KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    /* set AES_key */
    cmd = KL_CMD_TYPE_MOVE|handle->key_pos|KL_CMD_MOVE_DST_AES_KEY|KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    /* update AES_out as key for next level */
    handle->key_pos = KL_CMD_MOVE_SRC_AES_DOUT;

    return SUCCESS;
}

/* CEK_pT -> CEK_p -> CEK  by TDES */
int bare_kl_execute_TDC_TDES(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    unsigned short cmd = 0;

    /* set input */
    if(kl_set_input(handle, p_input) != SUCCESS)
    {
        return ERR_FAILURE;
    }
    cmd = KL_CMD_TYPE_MOVE|KL_MOVE_SRC_REG_CPUn(handle->data_pos-1)|KL_CMD_MOVE_DST_INVT_DIN|KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    /* get CEK */
    /* set input to aes_din */
    cmd = KL_CMD_TYPE_MOVE|KL_CMD_MOVE_SRC_INVT_DOUT|KL_CMD_MOVE_DST_TDES_DIN|KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    /* set TDES_key */
    cmd = KL_CMD_TYPE_MOVE|handle->key_pos|KL_CMD_MOVE_DST_TDES_KEY|KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    /* update TDES_out as key for next level */
    handle->key_pos = KL_CMD_MOVE_SRC_TDES_DOUT;

    return SUCCESS;
}

int bare_kl_link_aes(void *bhandle, unsigned char *p_input, KL_MOVE_ENC_E enc_type)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 enc_t = kl_set_bit_val(enc_type, KL_RAM_CMD_MOVE_ENC_SHIFT, KL_RAM_CMD_MOVE_ENC_MASK);
    mt_u16 cmd = 0;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | KL_CMD_MOVE_DST_AES_DIN | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    cmd = KL_CMD_TYPE_MOVE | handle->key_pos | KL_CMD_MOVE_DST_AES_KEY | enc_t | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    handle->key_pos = KL_CMD_MOVE_SRC_AES_DOUT;

    return SUCCESS;
}

int bare_kl_link_sm4(void *bhandle, unsigned char *p_input, KL_MOVE_ENC_E enc_type)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 enc_t = kl_set_bit_val(enc_type, KL_RAM_CMD_MOVE_ENC_SHIFT, KL_RAM_CMD_MOVE_ENC_MASK);
    mt_u16 cmd = 0;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | KL_CMD_MOVE_DST_SM4_DIN | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    cmd = KL_CMD_TYPE_MOVE | handle->key_pos | KL_CMD_MOVE_DST_SM4_KEY | enc_t | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    handle->key_pos = KL_CMD_MOVE_SRC_SM4_DOUT;

    return SUCCESS;
}

int bare_kl_link_tdes(void *bhandle, unsigned char *p_input, KL_MOVE_ENC_E enc_type)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 enc_t = kl_set_bit_val(enc_type, KL_RAM_CMD_MOVE_ENC_SHIFT, KL_RAM_CMD_MOVE_ENC_MASK);
    mt_u16 cmd = 0;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | KL_CMD_MOVE_DST_TDES_DIN | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    cmd = KL_CMD_TYPE_MOVE | handle->key_pos | KL_CMD_MOVE_DST_TDES_KEY | enc_t | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    handle->key_pos = KL_CMD_MOVE_SRC_TDES_DOUT;

    return SUCCESS;
}

int bare_kl_link_xor(void *bhandle, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u16 cmd = 0;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }
    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | KL_CMD_MOVE_DST_XOR_DIN_A | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    cmd = KL_CMD_TYPE_MOVE | handle->key_pos | KL_CMD_MOVE_DST_XOR_DIN_B | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    handle->key_pos = KL_CMD_MOVE_SRC_XOR_DOUT;

    return SUCCESS;
}

int bare_kl_link_hash(void *bhandle, unsigned char *p_input, KL_HASH_POSITION_E input_data_pos, KL_HASH_POSITION_E output_key_pos)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u16 cmd = 0;
    mt_u32 cmd_hash_pos = 0;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    cmd_hash_pos = input_data_pos == KL_HASH_FIRST_HALF ? KL_CMD_MOVE_DST_HASH_DIN_H : KL_CMD_MOVE_DST_HASH_DIN_L;
    cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | cmd_hash_pos | KL_RAM_CMD_MOVE_ADDs_MV;
    kl_set_cmd(handle, cmd);

    cmd_hash_pos = input_data_pos == KL_HASH_FIRST_HALF ? KL_CMD_MOVE_DST_HASH_DIN_L : KL_CMD_MOVE_DST_HASH_DIN_H;
    cmd = KL_CMD_TYPE_MOVE | handle->key_pos | cmd_hash_pos | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
    kl_set_cmd(handle, cmd);

    handle->key_pos = output_key_pos == KL_HASH_FIRST_HALF ? KL_CMD_MOVE_SRC_HASH_DOUT_H : KL_CMD_MOVE_SRC_HASH_DOUT_L;

    return SUCCESS;
}

int bare_kl_link_seedv(void *bhandle, unsigned char *p_input, KL_HARDWIRED_SOURCE_E mask_key, KL_STANDARD_PROFILE_E profile)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u16 cmd = 0;
    mt_u32 cmd_in = 0, cmd_out = 0, cmd_key = 0;
    KL_CMD_MOVE_ENC_T enc_type = KL_RAM_CMD_MOVE_ADDs_ENC;

    if (kl_set_input(handle, p_input) != SUCCESS) {
        return ERR_FAILURE;
    }

    if (profile == KL_ETSI_TS_103_162) {
        /* move mask key to XORb */
        cmd = KL_CMD_TYPE_MOVE | KL_HWKEY_SRC(mask_key) | KL_CMD_MOVE_DST_XOR_DIN_B | KL_RAM_CMD_MOVE_ADDs_MV;
        kl_set_cmd(handle, cmd);

        /* move vendor_id to XORa, XOR */
        cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 1) | KL_CMD_MOVE_DST_XOR_DIN_A | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        /* move SCKv to AES key */
        cmd = KL_CMD_TYPE_MOVE | KL_CMD_MOVE_SRC_HASH_DOUT_L | KL_CMD_MOVE_DST_AES_KEY | KL_RAM_CMD_MOVE_ADDs_MV;
        kl_set_cmd(handle, cmd);

        /* move seedv to AES in */
        cmd = KL_CMD_TYPE_MOVE | KL_CMD_MOVE_SRC_XOR_DOUT | KL_CMD_MOVE_DST_AES_DIN | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        handle->key_pos = KL_CMD_MOVE_SRC_AES_DOUT;
    } else if (profile == KL_GY_T_255_2012) {
        /* move AES_out to HASH_H */
        cmd = KL_CMD_TYPE_MOVE | KL_CMD_MOVE_SRC_AES_DOUT | KL_CMD_MOVE_DST_HASH_DIN_H | KL_RAM_CMD_MOVE_ADDs_MV;
        kl_set_cmd(handle, cmd);

        /* select mask key to AES key */
        cmd = KL_CMD_TYPE_MOVE | KL_HWKEY_SRC(mask_key) | KL_CMD_MOVE_DST_AES_KEY | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        /* move seedv to HASH_L */
        cmd = KL_CMD_TYPE_MOVE | KL_CMD_MOVE_SRC_AES_DOUT | KL_CMD_MOVE_DST_HASH_DIN_L | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        handle->key_pos = KL_CMD_MOVE_SRC_HASH_DOUT_L;
    } else {
        switch (profile) {
        case KL_SCTE_201_2013_P1:
        case KL_SCTE_201_2013_P1A:
            cmd_in = KL_CMD_MOVE_DST_TDES_DIN;
            cmd_out = KL_CMD_MOVE_SRC_TDES_DOUT;
            cmd_key = KL_CMD_MOVE_DST_TDES_KEY;
            break;
        case KL_SCTE_201_2013_P2:
        case KL_SCTE_201_2013_P2A:
            cmd_in = KL_CMD_MOVE_DST_AES_DIN;
            cmd_out = KL_CMD_MOVE_SRC_AES_DOUT;
            cmd_key = KL_CMD_MOVE_DST_AES_KEY;
            break;
        case KL_SCTE_201_2013_P2B:
            cmd_in = KL_CMD_MOVE_DST_AES_DIN;
            cmd_out = KL_CMD_MOVE_SRC_AES_DOUT;
            cmd_key = KL_CMD_MOVE_DST_AES_KEY;
            enc_type = KL_RAM_CMD_MOVE_ADDs_DEC;
            break;
        default:
            return ERR_FAILURE;
        }

        if (kl_temporary_save_key(handle) != SUCCESS) {
            return ERR_FAILURE;
        }

        /* select mask key */
        cmd = KL_CMD_TYPE_MOVE | KL_HWKEY_SRC(mask_key) | cmd_key | KL_RAM_CMD_MOVE_ADDs_MV;
        kl_set_cmd(handle, cmd);

        /* move vendor_id to data_in */
        cmd = KL_CMD_TYPE_MOVE | KL_MOVE_SRC_REG_CPUn(handle->data_pos - 2) | cmd_in | enc_type | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        /* move seedv to XORb */
        cmd = KL_CMD_TYPE_MOVE | cmd_out | KL_CMD_MOVE_DST_XOR_DIN_B | KL_RAM_CMD_MOVE_ADDs_MV;
        kl_set_cmd(handle, cmd);

        /* move seedv to data_in */
        cmd = KL_CMD_TYPE_MOVE | cmd_out | cmd_in | KL_RAM_CMD_MOVE_ADDs_MV;
        kl_set_cmd(handle, cmd);

        /* move SCKv to key_in */
        cmd = KL_CMD_TYPE_MOVE | handle->key_pos | cmd_key | enc_type | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        /* move data_out to XORa, XOR */
        cmd = KL_CMD_TYPE_MOVE | cmd_out | KL_CMD_MOVE_DST_XOR_DIN_A | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
        kl_set_cmd(handle, cmd);

        /* update the xor_dout as key_source for next level */
        handle->key_pos = KL_CMD_MOVE_SRC_XOR_DOUT;
    }

    return SUCCESS;
}

int bare_kl_export_key(void *bhandle, KL_EXPORT_DST_E dst, unsigned long slot_id)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    KL_RAM_CMD_EXPORT_SRC export_key_pos = 0;
    mt_u32 export_dst = kl_set_bit_val(dst, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);
    mt_u16 cmd = 0;

    switch (handle->key_pos) {
    case KL_CMD_MOVE_SRC_SCK_SELECTED:
        export_key_pos = KL_CMD_EXPORT_SRC_SCK_SELECTED;
        break;
    case KL_CMD_MOVE_SRC_AES_DOUT:
        export_key_pos = KL_CMD_EXPORT_SRC_AES_DOUT;
        break;
    case KL_CMD_MOVE_SRC_TDES_DOUT:
        export_key_pos = KL_CMD_EXPORT_SRC_TDES_DOUT;
		break;
	case KL_CMD_MOVE_SRC_SM4_DOUT:
		export_key_pos = KL_CMD_EXPORT_SRC_SM4_DOUT;
        break;
    case KL_CMD_MOVE_SRC_XOR_DOUT:
        export_key_pos = KL_CMD_EXPORT_SRC_XOR_DOUT;
        break;
    case KL_CMD_MOVE_SRC_HASH_DOUT_L:
        export_key_pos = KL_CMD_EXPORT_SRC_HASH_DOUT_L;
        break;
    case KL_CMD_MOVE_SRC_HASH_DOUT_H:
        export_key_pos = KL_CMD_EXPORT_SRC_HASH_DOUT_H;
        break;
    default:
        return ERR_FAILURE;
    }

    if (dst == KL_EXPORT_DST_KT) {
        handle->slot_id = slot_id & (KEY_TABLE_SLOT_NUM - 1);
    }

    cmd = KL_CMD_TYPE_EXPORT_KEY | export_key_pos | export_dst | KL_CMD_EXPORT_ADDs;

    return kl_set_cmd(handle, cmd);
}

int bare_kl_store_key(void *bhandle, KL_STORE_DST_E store_dst)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 store_key_pos = 0, store_dst_pos = 0;
    mt_u16 cmd = 0;

    switch (handle->key_pos) {
    case KL_CMD_MOVE_SRC_AES_DOUT:
        store_key_pos = KL_CMD_RESTORE_SRC_AES_DOUT;
        break;
    case KL_CMD_MOVE_SRC_TDES_DOUT:
        store_key_pos = KL_CMD_RESTORE_SRC_TDES_DOUT;
		break;
	case KL_CMD_MOVE_SRC_SM4_DOUT:
		store_key_pos = KL_CMD_RESTORE_SRC_SM4_DOUT;
        break;
    case KL_CMD_MOVE_SRC_XOR_DOUT:
        store_key_pos = KL_CMD_RESTORE_SRC_XOR_DOUT;
        break;
    case KL_CMD_MOVE_SRC_HASH_DOUT_L:
        store_key_pos = KL_CMD_RESTORE_SRC_HASH_DOUT_L;
        break;
    case KL_CMD_MOVE_SRC_HASH_DOUT_H:
        store_key_pos = KL_CMD_RESTORE_SRC_HASH_DOUT_H;
        break;
    default:
        return ERR_FAILURE;
    }

    store_dst_pos = kl_set_bit_val((mt_u32)store_dst, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);

    cmd = KL_CMD_TYPE_STORE_TO_PRIVATE | store_key_pos | store_dst_pos | kl_bits(KL_STORE_ADDS_LOCK, KL_RAM_CMD_ADD_SHIFT);

    return kl_set_cmd(handle, cmd);
}

/**
 * @brief restore will lead to excute
 */
int bare_kl_store_cmd(void *bhandle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 restore_src, restore_dst, cmd;

    restore_src = kl_set_bit_val(store_src, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
    restore_dst = kl_set_bit_val(store_dst, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);

    cmd = KL_CMD_TYPE_STORE_TO_PRIVATE | restore_src | restore_dst | kl_set_bit_val(lock, KL_RAM_CMD_ADD_SHIFT, KL_RAM_CMD_ADD_MASK);

    return kl_set_cmd(handle, cmd);
}

/* user can choose whether to excute */
int bare_kl_store_cmd2(void *bhandle, KL_STORE_SRC_E store_src, KL_STORE_DST_E store_dst, KL_STORE_LOCK_E lock, unsigned char exec)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 restore_src, restore_dst, cmd;

    restore_src = kl_set_bit_val(store_src, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
    restore_dst = kl_set_bit_val(store_dst, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);

    cmd = KL_CMD_TYPE_STORE_TO_PRIVATE | restore_src | restore_dst | kl_set_bit_val(lock, KL_RAM_CMD_ADD_SHIFT, KL_RAM_CMD_ADD_MASK);

    return kl_set_cmd2(handle, cmd, exec);
}

int bare_kl_move_cmd(void *bhandle, KL_MOVE_SRC_E move_src, KL_MOVE_DST_E move_dst, KL_MOVE_TRIGGER_E trigger, KL_MOVE_ENC_E enc_type)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 mv_src, mv_dst, mv_enc, mv_trg;
    mt_u16 cmd;

    if (move_src == KL_MOVE_SRC_AUTO_DATA) {
        mv_src = handle->data_pos;
    } else if (move_src == KL_MOVE_SRC_AUTO_KEY) {
        mv_src = handle->key_pos;
    } else {
        mv_src = kl_set_bit_val(move_src, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
    }
    mv_enc = kl_set_bit_val(enc_type, KL_RAM_CMD_MOVE_ENC_SHIFT, KL_RAM_CMD_MOVE_ENC_MASK);
    mv_dst = kl_set_bit_val(move_dst, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);
    mv_trg = kl_set_bit_val(trigger, KL_RAM_CMD_MOVE_TRG_SHIFT, KL_RAM_CMD_MOVE_TRG_MASK);

    if (move_dst == KL_MOVE_DST_AES_KEY) {
        handle->key_pos = KL_CMD_MOVE_SRC_AES_DOUT;
    } else if (move_dst == KL_MOVE_DST_TDES_KEY) {
        handle->key_pos = KL_CMD_MOVE_SRC_TDES_DOUT;
	} else if (move_dst == KL_MOVE_DST_SM4_KEY) {
		handle->key_pos = KL_CMD_MOVE_SRC_SM4_DOUT;
    }

    cmd = KL_CMD_TYPE_MOVE | mv_src | mv_dst | mv_enc | mv_trg;

    return kl_set_cmd(handle, cmd);
}

int bare_kl_export_cmd(void *bhandle, KL_EXPORT_SOURCE_E key_src, KL_EXPORT_DST_E key_dst, mt_u32 slot_id)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 export_src, export_dst;
    mt_u16 cmd;

    if (key_dst == KL_EXPORT_DST_KT) {
        handle->slot_id = slot_id & (KEY_TABLE_SLOT_NUM - 1);
    }

    export_src = kl_set_bit_val(key_src, KL_RAM_CMD_SRC_SHIFT, KL_RAM_CMD_SRC_MASK);
    export_dst = kl_set_bit_val(key_dst, KL_RAM_CMD_DST_SHIFT, KL_RAM_CMD_DST_MASK);

    cmd = KL_CMD_TYPE_EXPORT_KEY | export_src | export_dst | KL_CMD_EXPORT_ADDs;

    return kl_set_cmd(handle, cmd);
}

int bare_kl_wait_complete(void *bhandle, unsigned long *p_error)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;
    mt_u32 cmd = 0, time_out = 20;

    while (time_out--) {
        cmd = kl_readl(KEYLADDER_REG_STATE_CHECK);
        if (!(cmd & KL_CMDST_BUSY)) {
            break;
        }

        kl_delay_function(1);
    }
    *p_error = cmd;

    if (*p_error) {
        KL_ERR("error code -> 0x%08x\n", (mt_u32)*p_error);
        return ERR_FAILURE;
    }

    return time_out == 0 ? ERR_TIMEOUT : SUCCESS;
}

int bare_kl_read_key(void *bhandle, unsigned char *p_key_buffer)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    if (p_key_buffer) {
        memcpy_from_reg(p_key_buffer, (mt_u8 *)KEYLADDER_REG_EXPORT_KEY, KL_EXPORT_KT_BLOCK_SIZE);
        return SUCCESS;
    }

    return ERR_FAILURE;
}

int bare_kl_input_data(void *bhandle, KL_INPUT_POSITION_E postion, unsigned char *p_input)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    if (p_input) {
        memcpy_to_reg((mt_u8 *)KL_GET_CPUn_REG_ADDR(postion), p_input, KL_DATA_CPUn_BLOCK_SIZE);
#ifdef KL_DEBUG_INTERNAL
        memcpy(&debug_kl_cmd_t->data[postion][0], p_input, KL_DATA_CPUn_BLOCK_SIZE);
        debug_kl_cmd_t->cmd_level = postion;
#endif
        return SUCCESS;
    }

    return ERR_FAILURE;
}

/**
 * @brief kle enter the state of waiting for cpc
 */
int bare_kl_ready_for_cpc(void *bhandle)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    if (kl_readl(KEYLADDER_REG_WAIT_CPC) & KL_WAIT_CPC_TKEY_VALID) {
        return SUCCESS;
    }

    return ERR_FAILURE;
}

/**
 * @brief cpc has send key to kle
 */
int bare_kl_cpc_ready_for_kl(void *bhandle)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    if (!(kl_readl(KEYLADDER_REG_WAIT_CPC) & KL_WAIT_CPC_TKEY_VALID)) {
        return SUCCESS;
    }

    return ERR_FAILURE;
}

/**
 * @brief Set kl cmd auth key source
 */
int bare_kl_set_signature_keysrc(void *bhandle, KL_SIG_CPU_E cpu, KL_SIG_KEYSRC_E keysrc)
{
	kl_handle_t *handle = (kl_handle_t *)bhandle;
	unsigned int reg_sig_keysrc_ori = 0;

	reg_sig_keysrc_ori= kl_readl(KEYLADDER_REG_SIGNATURE_KEYSRC);
	if(keysrc == KL_SIG_KEYSRC_HWKEY0)
	{
		kl_writel(reg_sig_keysrc_ori & ~(0x3 << (cpu * 2)), KEYLADDER_REG_SIGNATURE_KEYSRC);
	}
	else
	{
		kl_writel(reg_sig_keysrc_ori | (0x3 << (cpu * 2)), KEYLADDER_REG_SIGNATURE_KEYSRC);
	}

	return MT_SUCCESS;
}

/**
 * @brief Lock kl cmd auth key source
 */
int bare_kl_set_signature_lock_keysrc(void *bhandle, KL_SIG_CPU_E cpu, unsigned char lock)
{
	kl_handle_t *handle = (kl_handle_t *)bhandle;
	unsigned int reg_sig_keysrc_ori = 0;

	reg_sig_keysrc_ori = kl_readl(KEYLADDER_REG_SIGNATURE_KEYSRC_LOCK);
	if(lock)
	{
		kl_writel(KEYLADDER_REG_SIGNATURE_KEYSRC_LOCK, reg_sig_keysrc_ori | (0xf << (cpu * 4)));
	}
	else
	{
		kl_writel(KEYLADDER_REG_SIGNATURE_KEYSRC_LOCK, reg_sig_keysrc_ori & ~(0xf << (cpu * 4)));
	}

	return MT_SUCCESS;
}

/* for test: 0 released; 1 occuppied */
int kl_check_semaphore(void *bhandle)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    return kl_get_bit_val(kl_readl(KEYLADDER_REG_CMD_SEM), KL_SEM_ACCESS_SHIFT, KL_SEM_ACCESS_MASK);
}

int kl_check_error_state(void *bhandle)
{
    kl_handle_t *handle = (kl_handle_t *)bhandle;

    return kl_readl(KEYLADDER_REG_STATE_CHECK);
}

#ifdef KL_DEBUG_INTERNAL
/*
 * @brief this function for internal debug.
 *   param[in] debug_level, after the debug_level generate key, export the key result to acpu_reg.
 */
static int debug_kl_exec(kl_handle_t *handle, mt_u32 debug_level)
{
    mt_u32 i = 0, j, ram_cmd, step, tag, parity, err_code, mkey[4] = {1, 2, 3, 4};
    KL_RAM_CMD_EXPORT_SRC export_src;
    mt_u32 move_dst;

    memcpy_to_reg((mt_u8 *)(KEYLADDER_REG_SIGNATURE), debug_kl_cmd_t->signature, KL_SIGNATURE_LEN);
    printk("\nsignature -> \n");
    kl_hex_dump_bytes(debug_kl_cmd_t->signature, KL_SIGNATURE_LEN);

    printk("\data -> \n");
    for (i = 0; i <= debug_kl_cmd_t->cmd_level; i++) {
        memcpy_to_reg((mt_u8 *)KL_GET_CPUn_REG_ADDR(i), &debug_kl_cmd_t->data[i][0], KL_DATA_CPUn_BLOCK_SIZE);
        kl_hex_dump_bytes(&debug_kl_cmd_t->data[i][0], KL_DATA_CPUn_BLOCK_SIZE);
    }

    printk("debug_level -> %d\n", debug_level);
    for (i = 0; i < KEYLADDER_CMD_RAM_SIZE; i++) {
        printk("debug ram cmd[%d] -> 0x%08x\n", i, debug_kl_cmd_t->cmd_ram[i]);
        kl_writel(debug_kl_cmd_t->cmd_ram[i], (mt_u32 *)KEYLADDER_REG_RAM_CMD + i);

#if 0 //debug SCK, export SCK to cpu_reg directly.
        ram_cmd = KL_CMD_TYPE_EXPORT_KEY | KL_CMD_EXPORT_SRC_SCK_SELECTED | KL_CMD_EXPORT_DST_ACPU | KL_CMD_EXPORT_ADDs;

        tag = kl_get_bit_val(debug_kl_cmd_t->cmd_ram[i], KL_RAM_CMD_TAG_SHIFT, KL_RAM_CMD_TAG_MASK);
        step = kl_get_bit_val(debug_kl_cmd_t->cmd_ram[i], KL_RAM_CMD_STEP_SHIFT, KL_RAM_CMD_STEP_MASK);
        ram_cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step + 1, KL_RAM_CMD_STEP_SHIFT) | ram_cmd;
        parity = kl_cmd_parity(ram_cmd);
        ram_cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
        kl_writel(ram_cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i + 1);
        printk("debug ram cmd[%d] -> 0x%08x\n", i+1, ram_cmd);
        printk("debug ram cmd end.............................\n");
        break;
#endif
        if (i == debug_level) {
            tag = kl_get_bit_val(debug_kl_cmd_t->cmd_ram[i], KL_RAM_CMD_TAG_SHIFT, KL_RAM_CMD_TAG_MASK);
            step = kl_get_bit_val(debug_kl_cmd_t->cmd_ram[i], KL_RAM_CMD_STEP_SHIFT, KL_RAM_CMD_STEP_MASK);
            move_dst = debug_kl_cmd_t->cmd_ram[i] & kl_bits(KL_RAM_CMD_DST_MASK, KL_RAM_CMD_DST_SHIFT);

            switch (move_dst) {
            case KL_CMD_MOVE_DST_AES_KEY:
            case KL_CMD_MOVE_DST_AES_DIN:
                export_src = KL_CMD_EXPORT_SRC_AES_DOUT;
                break;
            case KL_CMD_MOVE_DST_TDES_KEY:
            case KL_CMD_MOVE_DST_TDES_DIN:
                export_src = KL_CMD_EXPORT_SRC_TDES_DOUT;
                break;
            case KL_CMD_MOVE_DST_SM4_KEY:
            case KL_CMD_MOVE_DST_SM4_DIN:
                export_src = KL_CMD_EXPORT_SRC_SM4s_DOUT;
                break;
            case KL_CMD_MOVE_DST_INVT_DIN:
                i++;
                step++;
                ram_cmd = KL_CMD_TYPE_MOVE | (KL_RAM_CMD_MOVE_SRC)KL_CMD_MOVE_SRC_INVT_DOUT | KL_CMD_MOVE_DST_AES_DIN | KL_RAM_CMD_MOVE_ADDs_MV;
                ram_cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step, KL_RAM_CMD_STEP_SHIFT) | ram_cmd;
                parity = kl_cmd_parity(ram_cmd);
                ram_cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
                kl_writel(ram_cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i);
                printk("debug ram cmd[%d] -> 0x%08x\n", i, ram_cmd);

                i++;
                step++;
                ram_cmd = KL_CMD_TYPE_MOVE | KL_CMD_MOVE_SRC_REG_CPU5 | KL_CMD_MOVE_DST_AES_KEY | KL_RAM_CMD_MOVE_ADDs_ENC | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
                ram_cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step, KL_RAM_CMD_STEP_SHIFT) | ram_cmd;
                parity = kl_cmd_parity(ram_cmd);
                ram_cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
                kl_writel(ram_cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i);
                printk("debug ram cmd[%d] -> 0x%08x\n", i, ram_cmd);

                i++;
                step++;
                ram_cmd = KL_CMD_TYPE_MOVE | (KL_RAM_CMD_MOVE_SRC)KL_CMD_MOVE_SRC_AES_DOUT | KL_CMD_MOVE_DST_AES_DIN | KL_RAM_CMD_MOVE_ADDs_MV;
                ram_cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step, KL_RAM_CMD_STEP_SHIFT) | ram_cmd;
                parity = kl_cmd_parity(ram_cmd);
                ram_cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
                kl_writel(ram_cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i);
                printk("debug ram cmd[%d] -> 0x%08x\n", i, ram_cmd);

                i++;
                step++;
                ram_cmd = KL_CMD_TYPE_MOVE | KL_CMD_MOVE_SRC_REG_CPU5 | KL_CMD_MOVE_DST_AES_KEY | KL_RAM_CMD_MOVE_ADDs_DEC | KL_RAM_CMD_MOVE_ADDs_MV_TRG;
                ram_cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step, KL_RAM_CMD_STEP_SHIFT) | ram_cmd;
                parity = kl_cmd_parity(ram_cmd);
                ram_cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
                kl_writel(ram_cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i);
                printk("debug ram cmd[%d] -> 0x%08x\n", i, ram_cmd);

                export_src = KL_CMD_EXPORT_SRC_AES_DOUT;
                break;
            default:
                printk("error: move_trig ram cmd[%d] -> 0x%08x\n", i, debug_kl_cmd_t->cmd_ram[i] & kl_bits(KL_RAM_CMD_DST_MASK, KL_RAM_CMD_DST_SHIFT));
                return SUCCESS;
            }

            ram_cmd = KL_CMD_TYPE_EXPORT_KEY | export_src | KL_CMD_EXPORT_DST_ACPU | KL_CMD_EXPORT_ADDs;

            ram_cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step + 1, KL_RAM_CMD_STEP_SHIFT) | ram_cmd;
            parity = kl_cmd_parity(ram_cmd);
            ram_cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
            kl_writel(ram_cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i + 1);
            printk("debug ram cmd[%d] -> 0x%08x\n", i+1, ram_cmd);
            printk("debug ram cmd end.............................\n");
            break;
        }
    }
    for (j = i + 2; j < KEYLADDER_CMD_RAM_SIZE; j++) {
        kl_writel(0, (mt_u32 *)KEYLADDER_REG_RAM_CMD + j);
    }

    printk("\command ram -> \n");
    kl_hex_dump_bytes(debug_kl_cmd_t->cmd_ram, sizeof(debug_kl_cmd_t->cmd_ram));

    /* 0 generic;  1 irdeto; 2 cri; KL_TRG_FLUSH_EN_BIT */
    ram_cmd = kl_set_bit_val(debug_kl_cmd_t->slot_id, KL_TRG_KEYTB_IDX_SHIFT, KL_TRG_KEYTB_IDX_MASK) | \
    kl_set_bit_val(debug_kl_cmd_t->ca_mode, KL_TRG_CA_SEL_SHIFT, KL_TRG_CA_SEL_MASK) | KL_TRG_START_EN;
    kl_writel(ram_cmd, KEYLADDER_REG_CMD_TRI);

    if (!(kl_readl(KEYLADDER_REG_CMD_TRI_ACCEPT) & KL_TRG_ACCEPT_VALID)) {
        return ERR_FAILURE;
    }

    while (1) {
        err_code = kl_readl(KEYLADDER_REG_STATE_CHECK);
        if (!(err_code & KL_CMDST_BUSY)) {
            break;
        }

        kl_delay_function(1);
    }

    if (err_code) {
        printk("error code -> 0x%08x\n", err_code);
        return ERR_FAILURE;
    }

    bare_kl_read_key(handle, mkey);
    printk("middle process key -> 0x%08x,0x%08x,0x%08x,0x%08x\n", mkey[0], mkey[1], mkey[2], mkey[3]);
    return SUCCESS;
}

/*
 * @brief this function will loop to print each time produce in the middle of the keyladder,
 *          therefore, we can be intuitive to check out the error occurred to which level.
 */
int debug_exec_cmd(kl_handle_t *handle)
{
    mt_u32 i = 0, j = 0, cmd = 0, tag = 0, step = 0;
    mt_u16 *p_input_cmd = handle->cmd_ram;
    mt_u8  parity = 0;
    KL_CA_MODE ca_mode = KL_CA_GENERIC;

    memset((void *)KEYLADDER_REG_RAM_CMD, 0, (KEYLADDER_CMD_RAM_SIZE * 4));

    if (kl_get_ca_mode(p_input_cmd, handle->cmd_cnt, &ca_mode)) {
        return ERR_FAILURE;
    }
    debug_kl_cmd_t->ca_mode = ca_mode;
    debug_kl_cmd_t->slot_id = handle->slot_id;

    for (i = 0; i < KEYLADDER_CMD_RAM_SIZE; i++) {
        if ((p_input_cmd[i] & KL_CMD_TYPE_MASK) == KL_CMD_TYPE_SELECT_SCK) {
            tag++;
            step = 0;
        } else {
            step++;
        }

        cmd = kl_bits(tag, KL_RAM_CMD_TAG_SHIFT) | kl_bits(step, KL_RAM_CMD_STEP_SHIFT) | p_input_cmd[i];
        parity = kl_cmd_parity(cmd);
        cmd |= kl_bits(parity, KL_RAM_CMD_EVEN_SHIFT) | kl_bits(~parity, KL_RAM_CMD_ODD_SHIFT);
        kl_writel(cmd, (mt_u32 *)KEYLADDER_REG_RAM_CMD + i);

        debug_kl_cmd_t->cmd_ram[i] = cmd;
        printk("cmd index[%d] -> 0x%08x\n", i, debug_kl_cmd_t->cmd_ram[i]);
        if (kl_get_bit(cmd, kl_bits(KL_RAM_CMD_TYPE_MASK, KL_RAM_CMD_TYPE_SHIFT)) == KL_CMD_TYPE_MOVE) {
            if (kl_get_bit(cmd, kl_bits(KL_RAM_CMD_MOVE_TRG_MASK, KL_RAM_CMD_MOVE_TRG_SHIFT)) == KL_RAM_CMD_MOVE_ADDs_MV_TRG) {
                debug_kl_cmd_t->trigger_pos[j] = i;
                j++;
            }
        }
    }
    debug_kl_cmd_t->trigger_pos[j] = 0xfa;
    printk("debug_kl_cmd_t->trigger_pos :\n");
    kl_hex_dump_bytes(debug_kl_cmd_t->trigger_pos, sizeof(debug_kl_cmd_t->trigger_pos));

    for (i = 0; i < 8; i++) {
        if (debug_kl_cmd_t->trigger_pos[i] != 0xfa) {
            printk("\n debug gen key level: %d, cmd index: %d\n", i, debug_kl_cmd_t->trigger_pos[i]);
            if (debug_kl_exec(handle, debug_kl_cmd_t->trigger_pos[i])) {
                return ERR_FAILURE;
            }
        } else {
            printk("\n debug gen key finish, total %d times.\n", i);
            break;
        }
    }
    debug_kl_end();

    return SUCCESS;
}
#endif
