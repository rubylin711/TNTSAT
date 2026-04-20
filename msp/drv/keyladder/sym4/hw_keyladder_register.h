/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HAL_KEY_LADDER_H__
#define __HAL_KEY_LADDER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Bitwise operation
 */
#define kl_bit(pos)                        (1U << (pos))
#define kl_bits(val, pos)                  ((val) << (pos))

#define kl_set_bit(reg, mask)              ((reg) |= (mask))
#define kl_clr_bit(reg, mask)              ((reg) &= ~(mask))
#define kl_get_bit(reg, mask)              ((reg) & (mask))
#define kl_get_bit_val(reg, shift, vmask)  (((unsigned int)(reg) >> (shift)) & (vmask))
#define kl_set_bit_val(reg, shift, mask)   (((unsigned int)(reg) & (mask)) << (shift))
#define kl_modify_reg(reg, clr_mask, set_mask) \
    ((reg) = (((reg) & (~(clr_mask))) | (set_mask)))

/* common define */
#define SUCCESS                                             ((int) 0)       /* Success return */
#define ERR_FAILURE                                         ((int)-1)       /* Fail for common reason */
#define ERR_TIMEOUT                                         ((int)-2)       /* Fail for waiting timeout */

#define KL_SIGNATURE_LEN                                    32
#define KEYLADDER_CMD_RAM_SIZE                              64
#define KEYLADDER_REG_BASE                                  0xbf30b000

#define KEY_TABLE_SLOT_NUM                                  128

/* KEY_LADDER register mapping */
#if 0
typedef struct {
	__IO mt_u32 SEM;           	    /* offset: 0x00, KEY_LADDER semaphore register */
	__IO mt_u32 TRIGGER;        	/* offset: 0x04, KEY_LADDER trigger register */
		 mt_u8  RESERVED1[4];
	     mt_u32 CMD_STATE[2];       /* offset: 0x0C, KEY_LADDER command state register */
	__O  mt_u32 CPUn[6][4];         /* offset: 0x10, KEY_LADDER CPU0~5 input register, which is BigEndian */
	__I  mt_u32 EXPORT_CPU[4];      /* offset: 0x70, KEY_LADDER export to CPU register, which is BigEndian */
	__I  mt_u32 TRIGGER_ACCEPT;     /* offset: 0x80, KEY_LADDER trigger accept status register */
		 mt_u8  RESERVED2[12];
	__IO mt_u32 SIGNATURE[8];       /* offset: 0x90, KEY_LADDER HMAC signature register, which is BigEndian */
	__I  mt_u32 WAIT_CPC;           /* offset: 0xB0, KEY_LADDER wait CPC to transfer key register */
		 mt_u8  RESERVED3[1868];
	__IO mt_u32 RAM_CMD[64];    	/* offset: 0x800, KEY_LADDER ram command register */
	__IO mt_u32 TDC[104];	    	/* offset: 0x900, KEY_LADDER TDC register */
} KEY_LADDER_T;
#endif

#define KEYLADDER_REG_CMD_SEM                               (handle->base_address + 0x0)
#define KEYLADDER_REG_CMD_TRI                               (handle->base_address + 0x4)
#define KEYLADDER_REG_CMD_TRI_ACCEPT                        (handle->base_address + 0x80)
#define KEYLADDER_REG_STATE_CHECK                           (handle->base_address + 0xc)
#define KEYLADDER_REG_CPU0                                  (handle->base_address + 0x10)
#define KEYLADDER_REG_CPU1                                  (handle->base_address + 0x20)
#define KEYLADDER_REG_CPU2                                  (handle->base_address + 0x30)
#define KEYLADDER_REG_CPU3                                  (handle->base_address + 0x40)
#define KEYLADDER_REG_CPU4                                  (handle->base_address + 0x50)
#define KEYLADDER_REG_CPU5                                  (handle->base_address + 0x60)
#define KEYLADDER_REG_EXPORT_KEY                            (handle->base_address + 0x70)
#define KEYLADDER_REG_SIGNATURE                             (handle->base_address + 0x90)
#define KEYLADDER_REG_WAIT_CPC                              (handle->base_address + 0xb0)
#define KEYLADDER_REG_RAM_CMD                               (handle->base_address + 0x800)
#define KEYLADDER_REG_TDC_RAM                               (handle->base_address + 0x900)

/* KEY_LADDER->KLE_SEM_REG, R/W */
#define KL_SEM_TIME_INIT_MASK       0xFFFFFFFU
#define KL_SEM_TIME_INIT_SHIFT      4
#define KL_SEM_REQ_VAL_BIT          kl_bit(3)
#define KL_SEM_ACCESS_MASK          0xFU
#define KL_SEM_ACCESS_SHIFT         0
typedef enum {
    KL_SEM_CPU_ID_APCPU             = 0x0,
    KL_SEM_CPU_ID_SECCPU            = 0x5,
    KL_SEM_CPU_ID_VSCPU             = 0x4,
} KL_SEM_CPU_ID;

/* KEY_LADDER->KLE_TRIGGER_REG, R/W */
#define KL_TRG_FLUSH_EN_BIT         kl_bit(31)
#define KL_TRG_EXPORT_SWAP_BIT      kl_bit(16)
#define KL_TRG_KEYTB_IDX_MASK       0x7FU
#define KL_TRG_KEYTB_IDX_SHIFT      8
#define KL_TRG_CA_SEL_MASK          0x7U
#define KL_TRG_CA_SEL_SHIFT         4
typedef enum {
    KL_CA_GENERIC                   = 0x0,
    KL_CA_IRDETO,
    KL_CA_CRI,
    KL_CA_CONAX,
    KL_CA_NAGRA,
    KL_CA_INVALID
} KL_CA_MODE;
#define KL_TRG_START_EN             0xFU

/* KEY_LADDER->KLE_CMD_STATE_REG, R/W */
#define KL_CMDST_BUSY               kl_bit(31)
#define KL_CMDST_HMAC_ERR           kl_bit(30)
#define KL_CMDST_SELECT_ERR         kl_bit(29)
#define KL_CMDST_NOP_ERR            kl_bit(28)
#define KL_CMDST_END_ERR            kl_bit(27)
#define KL_CMDST_FLAG1_ERR          kl_bit(24)
#define KL_CMDST_TAG_ERR            kl_bit(23)
#define KL_CMDST_PARITY_ERR         kl_bit(22)
#define KL_CMDST_STEP_PN_ERR        kl_bit(21)
#define KL_CMDST_STEP_ERR           kl_bit(20)
#define KL_CMDST_FLAG0_ERR          kl_bit(8)
#define KL_CMDST_ERR_CODE_MASK      0xFFU
#define KL_CMDST_ERR_CODE_SHIFT     0
typedef enum {
	KL_ERR_CODE_SUCCESS             = 0x0,
	KL_ERR_CODE_FAIL
} KL_ERR_CODE;

/* KEY_LADDER->KLE_CPUn_REG, W */
#define KL_DATA_CPUn_BLOCK_SIZE     16
#define KL_GET_CPUn_REG_ADDR(n)     (KEYLADDER_REG_CPU0 + (n * KL_DATA_CPUn_BLOCK_SIZE))

/* KEY_LADDER->KLE_EXPORT_CPU_REG, W */
#define KL_EXPORT_KT_BLOCK_SIZE     16

/* KEY_LADDER->KLE_TRIGGER_ACCEPT_REG, R */
#define KL_TRG_ACCEPT_VALID         1U

/* KEY_LADDER->KLE_WAIT_CPC_REG, R */
#define KL_WAIT_CPC_TKEY_VALID      1U

/* KEY_LADDER->KLE_RAM_CMD_REG, R/W */
#define KL_RAM_CMD_ODD_SHIFT        31
#define KL_RAM_CMD_EVEN_SHIFT       30
#define KL_RAM_CMD_TAG_MASK         0x3FU
#define KL_RAM_CMD_TAG_SHIFT        24
#define KL_RAM_CMD_STEP_MASK        0xFFU
#define KL_RAM_CMD_STEP_SHIFT       16
#define KL_RAM_CMD_TYPE_MASK        0x7U
#define KL_RAM_CMD_TYPE_SHIFT       13
#define KL_RAM_CMD_SRC_MASK         0x1FU
#define KL_RAM_CMD_SRC_SHIFT        8
#define KL_RAM_CMD_DST_MASK         0x1FU
#define KL_RAM_CMD_DST_SHIFT        3
#define KL_RAM_CMD_ADD_MASK         0x7U
#define KL_RAM_CMD_ADD_SHIFT        0
typedef enum {
	KL_CMD_TYPE_SELECT_SCK          = (0x1 << KL_RAM_CMD_TYPE_SHIFT),
	KL_CMD_TYPE_MOVE                = (0x2 << KL_RAM_CMD_TYPE_SHIFT),
	KL_CMD_TYPE_ExtrTDC             = (0x4 << KL_RAM_CMD_TYPE_SHIFT),
	KL_CMD_TYPE_STORE_TO_PRIVATE    = (0x5 << KL_RAM_CMD_TYPE_SHIFT),
	KL_CMD_TYPE_EXPORT_KEY          = (0x7 << KL_RAM_CMD_TYPE_SHIFT),
	KL_CMD_TYPE_MASK                = (0x7 << KL_RAM_CMD_TYPE_SHIFT)
} KL_RAM_CMD_TYPE;

/* KEY_LADDER select command parameters */
typedef enum {
    KL_CMD_SELECT_SRC_SCK0          = (0x0 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK1          = (0x1 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK2          = (0x2 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK3          = (0x3 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK4          = (0x4 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK5          = (0x5 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK6          = (0x6 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK7          = (0x7 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK8          = (0x8 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK9          = (0x9 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK10         = (0xa << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK11         = (0xb << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK12         = (0xc << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK13         = (0xd << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK14         = (0xe << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_SCK15         = (0xf << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_HWSCK0 		= (0x10 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_HWSCK1 		= (0x11 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_HWSCK2 		= (0x12 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_HWSCK3 		= (0x13 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_HWSCK4 		= (0x14 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_PRIVATE0 		= (0x18 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_PRIVATE1 		= (0x19 << KL_RAM_CMD_SRC_SHIFT),
	/* 0x1a ~ 0x1b only for CRI mode */
	KL_CMD_SELECT_SRC_CFAES_KEY		= (0x1a << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_SELECT_SRC_CFCWC 		= (0x1b << KL_RAM_CMD_SRC_SHIFT)
} KL_RAM_CMD_SECLT_SRC;
	
#define KL_CMD_SECLT_ADDs           1U

/* KEY_LADDER connect/move command parameters */
typedef enum {
	KL_CMD_MOVE_SRC_SCK_SELECTED     = (0x0 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_AES_DOUT         = (0x1 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_TDES_DOUT        = (0x2 << KL_RAM_CMD_SRC_SHIFT),
	/* 0x4 only for Irdeto mode */
	KL_CMD_MOVE_SRC_INVT_DOUT        = (0x4 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY0   = (0x5 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY1   = (0x6 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY2   = (0x7 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY3   = (0x8 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY4   = (0x9 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY5   = (0xa << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY6   = (0xb << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HARDWIRED_KEY7   = (0xc << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HASH_DOUT_L      = (0xd << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_HASH_DOUT_H      = (0xe << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_XOR_DOUT	     = (0xf << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_REG_CPU0    	 = (0x10 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_REG_CPU1		 = (0x11 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_REG_CPU2    	 = (0x12 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_REG_CPU3    	 = (0x13 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_REG_CPU4    	 = (0x14 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_REG_CPU5    	 = (0x15 << KL_RAM_CMD_SRC_SHIFT),
	/* 0x18 ~ 0x1b only for CRI mode */
	KL_CMD_MOVE_SRC_CWCW0       	 = (0x18 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_CWCW1       	 = (0x19 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_CWCW2       	 = (0x1a << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_MOVE_SRC_CWCW3       	 = (0x1b << KL_RAM_CMD_SRC_SHIFT),
} KL_RAM_CMD_MOVE_SRC;

#define KL_MOVE_SRC_REG_CPUn(n)      ((0x10 + (n)) << KL_RAM_CMD_SRC_SHIFT)
#define KL_HWKEY_SRC(n)              ((0x5 + (n)) << KL_RAM_CMD_SRC_SHIFT)

typedef enum {
	KL_CMD_MOVE_DST_AES_KEY          = (0x0 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_AES_DIN 		 = (0x1 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_TDES_KEY         = (0x2 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_TDES_DIN         = (0x3 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_HASH_DIN_L       = (0x4 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_XOR_DIN_A        = (0x5 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_XOR_DIN_B        = (0x6 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_MOVE_DST_HASH_DIN_H       = (0x7 << KL_RAM_CMD_DST_SHIFT),
	/* 0x8 only for Irdeto mode */
	KL_CMD_MOVE_DST_INVT_DIN         = (0x8 << KL_RAM_CMD_DST_SHIFT)
} KL_RAM_CMD_MOVE_DST;

#define KL_RAM_CMD_MOVE_AES_MASK     0x1U
#define KL_RAM_CMD_MOVE_AES_SHIFT    2
typedef enum {
	KL_RAM_CMD_MOVE_ADDs_TDES        = (0 << KL_RAM_CMD_MOVE_AES_SHIFT),
	KL_RAM_CMD_MOVE_ADDs_AES         = (1 << KL_RAM_CMD_MOVE_AES_SHIFT)
} KL_CMD_MOVE_AES_T;

#define KL_RAM_CMD_MOVE_ENC_MASK     0x1U
#define KL_RAM_CMD_MOVE_ENC_SHIFT    1
typedef enum {
	KL_RAM_CMD_MOVE_ADDs_DEC         = (0 << KL_RAM_CMD_MOVE_ENC_SHIFT),
	KL_RAM_CMD_MOVE_ADDs_ENC         = (1 << KL_RAM_CMD_MOVE_ENC_SHIFT)
} KL_CMD_MOVE_ENC_T;

#define KL_RAM_CMD_MOVE_TRG_MASK     0x1U
#define KL_RAM_CMD_MOVE_TRG_SHIFT    0
typedef enum {
	KL_RAM_CMD_MOVE_ADDs_MV         = (0 << KL_RAM_CMD_MOVE_TRG_SHIFT),
	KL_RAM_CMD_MOVE_ADDs_MV_TRG     = (1 << KL_RAM_CMD_MOVE_TRG_SHIFT)
} KL_CMD_MOVE_TRG_T;

/* KEY_LADDER restore command parameters */
typedef enum {
	KL_CMD_RESTORE_SRC_AES_DOUT      = (0x10 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_RESTORE_SRC_TDES_DOUT     = (0x11 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_RESTORE_SRC_HASH_DOUT_L   = (0x12 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_RESTORE_SRC_HASH_DOUT_H   = (0x13 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_RESTORE_SRC_XOR_DOUT      = (0x14 << KL_RAM_CMD_SRC_SHIFT),
	/* 0x15 only for CRI mode */
	KL_CMD_RESTORE_SRC_SCK_SELECTED  = (0x15 << KL_RAM_CMD_SRC_SHIFT)
} KL_RAM_CMD_RESTORE_SRC;

typedef enum {
	KL_CMD_RESTORE_DST_PRIVATE0      = (0x14 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_RESTORE_DST_PRIVATE1      = (0x15 << KL_RAM_CMD_DST_SHIFT),
	/* 0x18 ~ 0x1b only for CRI & Conax mode */
	KL_CMD_RESTORE_DST_CWCW0         = (0x18 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_RESTORE_DST_CWCW1         = (0x19 << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_RESTORE_DST_CWCW2         = (0x1a << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_RESTORE_DST_CWCW3         = (0x1b << KL_RAM_CMD_DST_SHIFT),
	/* 0x1e ~ 0x1f only for CRI mode */
	KL_CMD_RESTORE_DST_CFAES_KEY     = (0x1e << KL_RAM_CMD_DST_SHIFT),
	KL_CMD_RESTORE_DST_HWDECM        = (0x1f << KL_RAM_CMD_DST_SHIFT)
} KL_RAM_CMD_RESTORE_DST;

/* KEY_LADDER export command parameters */
typedef enum {
	KL_CMD_EXPORT_SRC_AES_DOUT      = (0x18 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_EXPORT_SRC_TDES_DOUT     = (0x19 << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_EXPORT_SRC_HASH_DOUT_L   = (0x1a << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_EXPORT_SRC_HASH_DOUT_H   = (0x1b << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_EXPORT_SRC_XOR_DOUT      = (0x1c << KL_RAM_CMD_SRC_SHIFT),
	KL_CMD_EXPORT_SRC_SCK_SELECTED  = (0x1d << KL_RAM_CMD_SRC_SHIFT)
} KL_RAM_CMD_EXPORT_SRC;

typedef enum {
    KL_CMD_EXPORT_DST_KT            = (0x1 << KL_RAM_CMD_DST_SHIFT),
    KL_CMD_EXPORT_DST_SCPU          = (0xa << KL_RAM_CMD_DST_SHIFT),
    KL_CMD_EXPORT_DST_JTAG          = (0x13 << KL_RAM_CMD_DST_SHIFT),
    KL_CMD_EXPORT_DST_ACPU          = (0x15 << KL_RAM_CMD_DST_SHIFT)
} KL_RAM_CMD_EXPORT_DST;
	
#define KL_CMD_EXPORT_ADDs          (0x6 << KL_RAM_CMD_ADD_SHIFT)

/* KEY_LADDER ExtrTDC command parameters */
#define KL_RAM_CMD_ExtrTDC_SRC      (0x15 << KL_RAM_CMD_SRC_SHIFT)
#define KL_RAM_CMD_ExtrTDC_DST      (0xa << KL_RAM_CMD_DST_SHIFT)
#define KL_RAM_CMD_ExtrTDC_ADDs     (0x4 << KL_RAM_CMD_ADD_SHIFT)

typedef enum {                                                                   
    KL_CMD_RESTORE_ADDs_NO_LOCK      = (0x0 << KL_RAM_CMD_ADD_SHIFT),
    KL_CMD_RESTORE_ADDs_LOCK_DOWN    = (0x1 << KL_RAM_CMD_ADD_SHIFT) 
} KL_RAM_CMD_RESTORE_LOCK;

typedef struct {
    unsigned long   key_pos;
    unsigned long   data_pos;
    unsigned long   base_address;
    unsigned short  slot_id;
    unsigned short  cmd_cnt;
	unsigned short  cmd_ram[KEYLADDER_CMD_RAM_SIZE];
    unsigned char   signature[16];
} kl_handle_t;

#ifdef __cplusplus
}
#endif

#endif //__HAL_KEY_LADDER_H__
