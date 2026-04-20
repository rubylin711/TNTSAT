/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __HW_M2M_IF_H__
#define __HW_M2M_IF_H__

#include "mt_lib.h"

#define AHB_BSRAM_ADDR  (0xfff40000)
#define AHB_BSRAM_SIZE  (128 * 1024)

#undef M2M_USE_VIRT_ADDR

#ifdef M2M_USE_VIRT_ADDR
/* addr passed to m2m is a virtual address.
 * Needs to translate to a physical address. */
#define HW_M2M_ADDR(addr) HW_M2M_PHYS_ADDR(addr)
#else
/* addr passed to m2m is a physical address already.
 * No translation any more. */
#define HW_M2M_ADDR(addr) core_phys_to_io_phys_addr((ulong)addr)
#endif

#define HW_M2M_PHYS_ADDR(addr) virt_to_io_phys_addr((ulong)addr)

#define HW_M2M_CIPHER_PROFILE(op, algo, mode) (((op) << 10) | ((algo) << 5) | (mode))
#define HW_M2M_HASH_PROFILE(algo, mode) ((1 << 9) | ((algo) << 8) | (mode))

#define HW_M2M_CIPHER_OP(profile)   (((profile) >> 10) & 0x1)
#define HW_M2M_CIPHER_ALGO(profile) (((profile) >> 5) & 0xf)
#define HW_M2M_CIPHER_MODE(profile) ((profile) & 0x1f)

#define HW_M2M_INVALID_KEY_SLOT      (255)
#define HW_M2M_MAX_CMD0_DK_SIZE      (16)
#define HW_M2M_MAX_CMD1_DK_SIZE      (32)
#define HW_M2M_MAX_CMD1_MACKEY_SIZE  (64)
#define HW_M2M_MAX_DIV_SIZE          (16)
#define HW_M2M_MAX_HASH_BLOCK_SIZE   (128)
#define HW_M2M_MAX_MAC_BLOCK_SIZE    (128)
#define HW_M2M_MAX_HASH_DIGEST_SIZE  (64)
#define HW_M2M_MAX_MAC_DIGEST_SIZE   (64)
#define HW_M2M_MAX_GHASH_DIGEST_SIZE (16)
#define HW_M2M_MAX_GCM_TAG_SIZE      (16)

typedef enum _HW_M2M_CMD_E {
    HW_M2M_CMD0 = 0,
    HW_M2M_CMD1,
    HW_M2M_BGC,
} HW_M2M_CMD_E;

typedef enum _HW_M2M_CHANNEL_E {
    HW_M2M_CH0 = 0,
    HW_M2M_CH1,
} HW_M2M_CHANNEL_E;

typedef enum _HW_M2M_CIPHER_OP_E {
    HW_M2M_CIPHER_OP_ENCRYPT = 0,
    HW_M2M_CIPHER_OP_DECRYPT = 1,
} HW_M2M_CIPHER_OP_E;

typedef enum _HW_M2M_CIPHER_ALGO_E {
    HW_M2M_CIPHER_ALGO_DES        = 1,
    HW_M2M_CIPHER_ALGO_TDES       = 2,
    HW_M2M_CIPHER_ALGO_AES128     = 3,
    HW_M2M_CIPHER_ALGO_SM4        = 5,
    HW_M2M_CIPHER_ALGO_AES192     = 6,
    HW_M2M_CIPHER_ALGO_AES256     = 7,
    HW_M2M_CIPHER_ALGO_GOST28147  = 8,
    HW_M2M_CIPHER_ALGO_GOSTR3412K = 9,
    HW_M2M_CIPHER_ALGO_GOSTR3412M = 10
} HW_M2M_CIPHER_ALGO_E;

typedef enum _HW_M2M_CIPHER_MODE_E {
    HW_M2M_CIPHER_MODE_CTR            = 1,
    HW_M2M_CIPHER_MODE_CTR64          = 2,
    HW_M2M_CIPHER_MODE_GCM            = 3,
    HW_M2M_CIPHER_MODE_ECB_CLR        = 4,
    HW_M2M_CIPHER_MODE_ECB_LCLR       = 5,
    HW_M2M_CIPHER_MODE_ECB_CTS        = 6,
    HW_M2M_CIPHER_MODE_CBC_CLR        = 8,
    HW_M2M_CIPHER_MODE_CBC_LCLR       = 9,
    HW_M2M_CIPHER_MODE_CBC_CTS_CS1    = 10,
    HW_M2M_CIPHER_MODE_CBC_CTS_CS2    = 11,
    HW_M2M_CIPHER_MODE_CBC_SCTE52     = 12,
    HW_M2M_CIPHER_MODE_CBC_SCTE52_IV2 = 13,
    HW_M2M_CIPHER_MODE_CBCS           = 14,
    HW_M2M_CIPHER_MODE_CENS           = 15,
    HW_M2M_CIPHER_MODE_CFB            = 16
} HW_M2M_CIPHER_MODE_E;

typedef enum _HW_M2M_PROFILE_E {
    /* ==============symmetric encryption algorithm profile list============= */
    DES_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 4),              /* SCTE41_DES */
    DES_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 5),
    DES_ECB_CTS_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 6),
    DES_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 8),
    DES_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 9),
    DES_CBC_CTS_CS1_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 10),
    DES_CBC_CTS_CS2_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 11),
    DES_CBC_SCTE52_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 12),          /* SCTE52_DES */
    DES_CBC_SCTE52_IV2_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 13),      /* SCTE52_IV2_DES */
    DES_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 1, 1),

    TDES_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 4),
    TDES_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 5),
    TDES_ECB_CTS_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 6),
    TDES_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 8),
    TDES_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 9),
    TDES_CBC_CTS_CS1_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 10),
    TDES_CBC_CTS_CS2_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 11),
    TDES_CBC_DVS042_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 12),         /* DVS042 */
    TDES_CBC_SCTE52_IV2_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 13),     /* SCTE52_IV2_TDES */
    TDES_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 2, 1),

    AES128_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 4),
    AES128_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 5),
    AES128_ECB_CTS_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 6),
    AES128_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 8),           /* CIPLUS_AES, DVB_CISSA */
    AES128_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 9),
    AES128_CBC_CTS_CS1_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 10),
    AES128_CBC_CTS_CS2_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 11),
    AES128_CBC_ATIS_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 12),         /* IDSA_AES */
    AES128_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 1),               /* MPEG-TS:NA */
    AES128_CTR64_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 2),             /* MPEG-TS:NA */
    AES128_CENS_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 15),
    AES128_CENS64_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 17),

    AES192_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 4),           /* MPEG-TS:DK only  RAW:DK only */
    AES192_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 5),          /* MPEG-TS:DK only  RAW:DK only */
    AES192_ECB_CTS_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 6),
    AES192_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 8),           /* MPEG-TS:DK only  RAW:DK only */
    AES192_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 9),          /* MPEG-TS:DK only  RAW:DK only */
    AES192_CBC_CTS_CS1_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 10),
    AES192_CBC_CTS_CS2_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 11),
    AES192_CBC_ATIS_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 12),
    AES192_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 1),               /* MPEG-TS:NA   RAW:DK only */
    AES192_CTR64_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 2),             /* MPEG-TS:NA   RAW:DK only */
    AES192_CENS_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 15),
    AES192_CENS64_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 17),

    AES256_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 4),           /* MPEG-TS:DK only  RAW:DK only */
    AES256_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 5),          /* MPEG-TS:DK only  RAW:DK only */
    AES256_ECB_CTS_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 6),
    AES256_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 8),           /* MPEG-TS:DK only  RAW:DK only */
    AES256_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 9),          /* MPEG-TS:DK only  RAW:DK only */
    AES256_CBC_CTS_CS1_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 10),
    AES256_CBC_CTS_CS2_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 11),
    AES256_CBC_ATIS_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 12),
    AES256_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 1),               /* MPEG-TS:NA   RAW:DK only */
    AES256_CTR64_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 2),             /* MPEG-TS:NA   RAW:DK only */
    AES256_CENS_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 15),
    AES256_CENS64_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 17),

    GCM_AES128_ENC = HW_M2M_CIPHER_PROFILE(0, 3, 3),               /* NIST 800-38d   MPEG-TS:NA */
    GCM_AES192_ENC = HW_M2M_CIPHER_PROFILE(0, 6, 3),               /* NIST 800-38d   MPEG-TS:NA   RAW:DK only */
    GCM_AES256_ENC = HW_M2M_CIPHER_PROFILE(0, 7, 3),               /* NIST 800-38d   MPEG-TS:NA   RAW:DK only */

    SM4_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 4),
    SM4_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 5),
    SM4_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 8),              /* CIPLUS_AES, DVB_CISSA */
    SM4_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 9),
    SM4_CBC_CTS_CS1_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 10),
    SM4_CBC_CTS_CS2_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 11),
    SM4_CBC_DVS042_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 12),          /* IDSA_AES */
    SM4_CBCS_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 14),
    SM4_CENS_ENC = HW_M2M_CIPHER_PROFILE(0, 5, 15),

    GOST28147_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 4),
    GOST28147_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 5),
    GOST28147_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 8),        /* CIPLUS_AES, DVB_CISSA */
    GOST28147_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 9),
    GOST28147_CBC_DVS042_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 12),    /* IDSA_AES */
    GOST28147_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 1),
    GOST28147_CFB_ENC = HW_M2M_CIPHER_PROFILE(0, 8, 16),

    GOSTR3412K_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 4),
    GOSTR3412K_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 5),
    GOSTR3412K_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 8),       /* CIPLUS_AES, DVB_CISSA */
    GOSTR3412K_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 9),
    GOSTR3412K_CBC_DVS042_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 12),   /* IDSA_AES */
    GOSTR3412K_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 1),
    GOSTR3412K_CFB_ENC = HW_M2M_CIPHER_PROFILE(0, 9, 16),

    GOSTR3412M_ECB_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 4),
    GOSTR3412M_ECB_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 5),
    GOSTR3412M_CBC_CLR_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 8),      /* CIPLUS_AES, DVB_CISSA */
    GOSTR3412M_CBC_LCLR_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 9),
    GOSTR3412M_CBC_DVS042_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 12),  /* IDSA_AES */
    GOSTR3412M_CTR_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 1),
    GOSTR3412M_CFB_ENC = HW_M2M_CIPHER_PROFILE(0, 10, 16),

    /* ============================================================= */
    DES_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 4),              /* SCTE41_DES */
    DES_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 5),
    DES_ECB_CTS_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 6),
    DES_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 8),
    DES_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 9),
    DES_CBC_CTS_CS1_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 10),
    DES_CBC_CTS_CS2_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 11),
    DES_CBC_SCTE52_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 12),          /* SCTE52_DES */
    DES_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 1, 1),

    TDES_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 4),
    TDES_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 5),
    TDES_ECB_CTS_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 6),
    TDES_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 8),
    TDES_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 9),
    TDES_CBC_CTS_CS1_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 10),
    TDES_CBC_CTS_CS2_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 11),
    TDES_CBC_DVS042_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 12),         /* DVS042 */
    TDES_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 2, 1),

    AES128_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 4),
    AES128_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 5),
    AES128_ECB_CTS_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 6),
    AES128_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 8),           /* CIPLUS_AES, DVB_CISSA */
    AES128_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 9),
    AES128_CBC_CTS_CS1_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 10),
    AES128_CBC_CTS_CS2_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 11),
    AES128_CBC_ATIS_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 12),         /* IDSA_AES */
    AES128_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 1),               /* MPEG-TS:NA */
    AES128_CTR64_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 2),             /* MPEG-TS:NA */
    AES128_CENS_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 15),
    AES128_CENS64_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 17),

    AES192_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 4),           /* MPEG-TS:DK only  RAW:DK only */
    AES192_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 5),          /* MPEG-TS:DK only  RAW:DK only */
    AES192_ECB_CTS_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 6),
    AES192_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 8),           /* MPEG-TS:DK only  RAW:DK only */
    AES192_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 9),          /* MPEG-TS:DK only  RAW:DK only */
    AES192_CBC_CTS_CS1_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 10),
    AES192_CBC_CTS_CS2_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 11),
    AES192_CBC_ATIS_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 12),
    AES192_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 1),               /* MPEG-TS:NA   RAW:DK only */
    AES192_CTR64_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 2),             /* MPEG-TS:NA   RAW:DK only */
    AES192_CENS_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 15),
    AES192_CENS64_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 17),

    AES256_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 4),           /* MPEG-TS:DK only  RAW:DK only */
    AES256_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 5),          /* MPEG-TS:DK only  RAW:DK only */
    AES256_ECB_CTS_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 6),
    AES256_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 8),           /* MPEG-TS:DK only  RAW:DK only */
    AES256_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 9),          /* MPEG-TS:DK only  RAW:DK only */
    AES256_CBC_CTS_CS1_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 10),
    AES256_CBC_CTS_CS2_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 11),
    AES256_CBC_ATIS_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 12),
    AES256_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 1),               /* MPEG-TS:NA   RAW:DK only */
    AES256_CTR64_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 2),             /* MPEG-TS:NA   RAW:DK only */
    AES256_CENS_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 15),
    AES256_CENS64_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 17),

    GCM_AES128_DEC = HW_M2M_CIPHER_PROFILE(1, 3, 3),               /* NIST 800-38d   MPEG-TS:NA */
    GCM_AES192_DEC = HW_M2M_CIPHER_PROFILE(1, 6, 3),               /* NIST 800-38d   MPEG-TS:NA   RAW:DK only */
    GCM_AES256_DEC = HW_M2M_CIPHER_PROFILE(1, 7, 3),               /* NIST 800-38d   MPEG-TS:NA   RAW:DK only */

    SM4_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 4),
    SM4_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 5),
    SM4_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 8),              /* CIPLUS_AES, DVB_CISSA */
    SM4_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 9),
    SM4_CBC_CTS_CS1_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 10),
    SM4_CBC_CTS_CS2_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 11),
    SM4_CBC_DVS042_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 12),          /* IDSA_AES */
    SM4_CBCS_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 14),
    SM4_CENS_DEC = HW_M2M_CIPHER_PROFILE(1, 5, 15),

    GOST28147_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 4),
    GOST28147_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 5),
    GOST28147_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 8),        /* CIPLUS_AES, DVB_CISSA */
    GOST28147_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 9),
    GOST28147_CBC_DVS042_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 12),    /* IDSA_AES */
    GOST28147_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 1),
    GOST28147_CFB_DEC = HW_M2M_CIPHER_PROFILE(1, 8, 16),

    GOSTR3412K_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 4),
    GOSTR3412K_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 5),
    GOSTR3412K_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 8),       /* CIPLUS_AES, DVB_CISSA */
    GOSTR3412K_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 9),
    GOSTR3412K_CBC_DVS042_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 12),   /* IDSA_AES */
    GOSTR3412K_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 1),
    GOSTR3412K_CFB_DEC = HW_M2M_CIPHER_PROFILE(1, 9, 16),

    GOSTR3412M_ECB_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 4),
    GOSTR3412M_ECB_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 5),
    GOSTR3412M_CBC_CLR_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 8),      /* CIPLUS_AES, DVB_CISSA */
    GOSTR3412M_CBC_LCLR_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 9),
    GOSTR3412M_CBC_DVS042_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 12),  /* IDSA_AES */
    GOSTR3412M_CTR_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 1),
    GOSTR3412M_CFB_DEC = HW_M2M_CIPHER_PROFILE(1, 10, 16),

    /* =================hash & mac profile list==================== */
    SHA1 = HW_M2M_HASH_PROFILE(0, 2),
    SHA2_224 = HW_M2M_HASH_PROFILE(0, 4),
    SHA2_256 = HW_M2M_HASH_PROFILE(0, 5),
    SHA2_384 = HW_M2M_HASH_PROFILE(0, 6),
    SHA2_512 = HW_M2M_HASH_PROFILE(0, 7),
    SM3 = HW_M2M_HASH_PROFILE(0, 8),

    HMAC_SHA2_224 = HW_M2M_HASH_PROFILE(1, 4),
    HMAC_SHA2_256 = HW_M2M_HASH_PROFILE(1, 5),
    HMAC_SHA2_384 = HW_M2M_HASH_PROFILE(1, 6),
    HMAC_SHA2_512 = HW_M2M_HASH_PROFILE(1, 7),
    HMAC_SM3 = HW_M2M_HASH_PROFILE(1, 8),

    CMAC_AES128 = HW_M2M_HASH_PROFILE(1, 12),
    CMAC_AES192 = HW_M2M_HASH_PROFILE(1, 13),
    CMAC_AES256 = HW_M2M_HASH_PROFILE(1, 14),
    CMAC_TDES = HW_M2M_HASH_PROFILE(1, 15),
    GHASH_AES128 = HW_M2M_HASH_PROFILE(1, 16),
    GHASH_AES192 = HW_M2M_HASH_PROFILE(1, 17),
    GHASH_AES256 = HW_M2M_HASH_PROFILE(1, 18),
    CBCMAC_AES128 = HW_M2M_HASH_PROFILE(1, 20),
    CMAC_SM4 = HW_M2M_HASH_PROFILE(1, 32),
    GHASH_SM4 = HW_M2M_HASH_PROFILE(1, 33),
    CBCMAC_SM4 = HW_M2M_HASH_PROFILE(1, 34),
    CBCMAC_TDES = HW_M2M_HASH_PROFILE(1, 35),
    MAC_GOST28147 = HW_M2M_HASH_PROFILE(1, 48),
    MAC_GOSTR3412K = HW_M2M_HASH_PROFILE(1, 49),
    MAC_GOSTR3412M = HW_M2M_HASH_PROFILE(1, 50),
    GHASH_GOSTR3412K = HW_M2M_HASH_PROFILE(1, 51),
} HW_M2M_PROFILE_E;

typedef enum _HW_M2M_KEY_MODE_E {
    HW_M2M_USE_KEYTABLE_256BIT = 0,
    HW_M2M_USE_KEYTABLE = 1,
    HW_M2M_USE_DKEY = 2,
    HW_M2M_USE_MACKEY = 3,
} HW_M2M_KEY_MODE_E;

typedef enum {
    M2M_EXT_NOTICE_NOIRQ = 0,
    M2M_EXT_NOTICE_CLEAN,
    M2M_EXT_NOTICE_TYPE_ILLEGAL
} M2M_EXTRA_NOTICE_TYPE;

typedef enum {
    M2M_EXT_NOTICE_MODE_REGISTER = 0,
    M2M_EXT_NOTICE_MODE_UNREGISTER,
} M2M_EXTRA_NOTICE_MODE;

/* Cmd0: Use single command to perform the crypto operation */
typedef struct {
    /* ================1st word(0xBF4B0000)================ */
    mt_u32 Cmd:4;      /* 0: use single command to perform the crypto operation
                        * 1: use descriptor stored in the memory to perform the crypto operation
                        * others: reserved */
    mt_u32 KeyIndx:7;  /* the key index for KeyTable */
    mt_u32 KeyEn:2;    /* HMAC:
                        * 01: use the keytable key
                        * 10: use direct key(DKey)
                        * others: use mackey from descriptor
                        * Other Profile:
                        * 01: use keytable key
                        * 10: use direct key(DKey)
                        * other: bypass */
    mt_u32 HF:1;       /* HASH/MAC:
                        * 0: the temporary result from the internal register of the M2M ciphers
                        * 1: the temporary result from the software(register) */
    mt_u32 I:1;        /* ECB/CBC/CTR:
                        * use direct IV, the IV fields are read from software register.
                        * else the IV is from KeyTable */
    mt_u32 F:1;        /* ECB/CBC/CTR:
                        * always fetch the IV. */
    mt_u32 Profile:11; /* the base algorithm, chaining residue mode
                        * [10] 0:encryption 1:decryption
                        * [9] 0:M2M  1:HASH or MAC
                        * ========M2M========
                        * [8:6] 1:DES
                        *       2:TDES
                        *       3:AES
                        *       6:AES192
                        *       7:AES256
                        * [5:0] 0:NA
                        *       1:CTR
                        *       2:CTR64
                        *       3:GCM
                        *       4:ECB_CLR
                        *       5:ECB_LCLR
                        *       6:ECB_CTS
                        *       8:CBC_CLR
                        *       9:CBC_LCLR
                        *       10:CBC_CTS(CS1)
                        *       11:CBC_CTS(CS2)
                        *       12:CBC_SCTE52/CBC_DVS042/CBC_ATIS
                        * ========HASH========
                        * [8] 0:HASH
                        * [7:0] 4:SHA2-224
                        *       5:SHA2-256
                        *       6:SHA2-384
                        *       7:SHA2-512
                        * ========MAC========
                        * [8] 1:MAC
                        * [7:0] 4:HMAC_SHA2_224
                        *       5:HMAC_SHA2_256
                        *       6:HMAC_SHA2_384
                        *       7:HMAC_SHA2_512
                        *       12:CMAC_AES128
                        *       13:CMAC_AES192
                        *       14:CMAC_AES256
                        *       15:CMAC_TDES(2Keys)
                        *       16:GHASH_AES128
                        *       17:GHASH_AES192
                        *       18:GHASH_AES256
                        *       20:CBCMAC_AES128 */
    mt_u32 Q:1;        /* Interrupt the CPU when the current command is done */
    mt_u32 E:1;        /* the data segment includes the tail of HASH or MAC */
    mt_u32 S:1;        /* the data segment includes the head of HASH or MAC */
    mt_u32 P:1;        /* 0: process all TS_PIDs  1: process specific TS_PIDs */
    mt_u32 T:1;        /* 0: RAW Mode  1: TS Mode */

    /* ================2th word(0xBF4B0000)================ */
    mt_u32 HashDataSum;  /* the data length in byte of already processed data */    

    union {
        struct {
    /* ================3nd word(0xBF4B0000)================ */
            mt_u32 TS_PID0:16;
            mt_u32 TS_PID1:16;

    /* ============= ===4rd word(0xBF4B0000)================ */
            mt_u32 TS_PID2:16;
            mt_u32 TS_PID3:16;
        };

        mt_u16 TS_PIDn[4];
    };

    /* ================5th word(0xBF4B0000)================ */
    mt_u32 WCID:5;       /* ID for protected channel if the direct key is used */
    mt_u32 SCID:5;       /* ID for protected channel if the direct key is used */
    mt_u32 SC:1;         /* 0: check the value between SCID and RDCID
                          * 1: not check the RDCID
                          * Note: SCID, WCID, SC can be configured by SCPU only.
                          * if these fields are written from ACPU, the values are forced to zero. */
    mt_u32 TDW:1;        /* TEE Designate WCID, 0: come from kt.wcid, 1: come from cmd.wcid */
    mt_u32:1;
    mt_u32 AT:1;         /* Address type in cmd0/1, 0: DRAM, 1: BSRAM */
    mt_u32 Fscb:2;       /* Force SCB value, 00: No Change, 01: Force SCB to 0, 10: Force SCB to 2, 11: Force SCB to 3 */
    mt_u32 OddKeyIndx:7; /* If Direct Key is enable, the same direct key is used for all cases.
                          * If KeyTable Key is enabled, KeyIndx is used for ClearKey or EvenKey
                          * the OddKeyIndx is used for OddKey */
    mt_u32 MSB:1;        /* MSB64 or LSB64 CW selection. 1: select MSB64 as CW, 0: select LSB64 as CW */
    mt_u32 TID:8;        /* Task ID */

    /* ================6th~9th word(0xBF4B0000)================ */
    mt_u32 DK[4];          /* 128 or 256 bits direct Key */

    /* ================10th word(0xBF4B0000)================ */
    mt_u32 SrcDataAddr:32; /* address pointer of the source data */

    /* ================11th word(0xBF4B0000)================ */
    mt_u32 DstDataAddr:32;   /* address pointer of the destination data */

    /* ================12th word(0xBF4B0000)================ */
    mt_u32 ClearDataSize:32; /* byte size of the clear data */

    /* ================13th word(0xBF4B0000)================ */
    mt_u32 ProtectedDataSize:32; /* byte size of the protected data */
} m2m_cmd0_t;

/* Cmd1: Use descriptor stored in the memory to perform the crypto operation */
typedef struct {
    /* ================1st word(0xBF4B0000)================ */
    mt_u32 Cmd:4;
    mt_u32 KeyIndx:7;
    mt_u32 KeyEn:2;
    mt_u32:3;
    mt_u32 Profile:11;
    mt_u32 Q:1;
    mt_u32 Retry:2;  /* wait time (cycle) for the paused descriptor */
    /*  0: 2^8
    *  1: 2^16
    *  2: 2^24
    *  3: 2^32
    */
    mt_u32 P:1;
    mt_u32 T:1;

    /* ================2th word(0xBF4B0000)================ */
    mt_u32 HashDataSum;  /* the data length in byte of already processed data */

    /* ================3th word(0xBF4B0000)================ */
    mt_u32 DescStartAddr:32; /* the first address of the descriptor chain */

    /* ================4nd word(0xBF4B0000)================ */
    mt_u32:32;

    /* ================5th word(0xBF4B0000)================ */
    mt_u32 WCID:5;
    mt_u32 SCID:5;
    mt_u32 SC:1;
    mt_u32 TDW:1;        /* TEE Designate WCID, 0: come from kt.wcid, 1: come from cmd.wcid */
    mt_u32:1;
    mt_u32 AT:1;         /* Address type in cmd0/1, 0: DRAM, 1: BSRAM */
    mt_u32 Fscb:2;       /* Force SCB value, 00: No Change, 01: Force SCB to 0, 10: Force SCB to 2, 11: Force SCB to 3 */
    mt_u32 OddKeyIndx:7; /* If Direct Key is enable, the same direct key is used for all cases.
                          * If KeyTable Key is enabled, KeyIndx is used for ClearKey or EvenKey
                          * the OddKeyIndx is used for OddKey */
    mt_u32 MSB:1;        /* MSB64 or LSB64 CW selection. 1: select MSB64 as CW, 0: select LSB64 as CW */
    mt_u32 TID:8;        /* Task ID */

    /* ================6th~13th word(0xBF4B0000)================ */
    mt_u32 DK[8];
} m2m_cmd1_t;

/* when the M2M command is finished, the task ID and status will be stored to a ring buffer.
 * the ring buffer is always overwritten by a new status.
 * the CPU can read any of the ring buffer to understand the situation of the crypto operation.
 * if the ErrCode is zero, the command is done without any error. Else the crypto operation is not performed.
 * the detail format for the M2M status is described below.
 */
typedef struct {
    /* ================1st word(0xBF4B0000)================ */
    mt_u32 ErrCode:24; /* [0]: algorithm is not supported
                        * [1]: Profile is unknown
                        * [2]: TDES key check is failed
                        * [3]: Key is not valid
                        * KeyAttribute:
                        * [4]: KeyUsage is not matched
                        * [5]: KeySize is not matched
                        * [6]: Key is not for encryption
                        * [7]: Key is not for decryption
                        * [8]: Key is not for M2M use
                        * [9]: Key is not for MAC use
                        * [10]: Key is not for REE-CPU
                        * Protected Buffer
                        * [12]: Source protected buffer is not matched
                        * [13]: Descriptor is from the wrong protected buffer */
    mt_u32 TID:8;
} m2m_status_t;

/* the descriptors are the pre-defined memory buffers which hold the necessary control information for the data transfers.
 * the information includes source and destination addressed, amount of data to be transferred, interrupt control, and other options.
 * the descriptors may be chained together to provide scatter-gather capability.
 * the descriptor is a unit to occupy the crypto cipher. when a descriptor is
 * complete, the occupied channel shall release the crypto ciphers then arbitrate again.
 * the definition of the descriptor is listed below.
 */
typedef struct {
    /* ================1st word(0xBF4B0000)================ */
    mt_u32 SrcDataAddr:32; /* address pointer of the source data */

    /* ================2nd word(0xBF4B0000)================ */
    mt_u32 DstDataAddr:32; /* address pointer of the destination data */

    /* ================3rd word(0xBF4B0000)================ */
    mt_u32 DesMagNum:32; /* descriptor magic number */

    /* ================4rd word(0xBF4B0000)================ */
    mt_u32 ClearDataSize:32; /* byte size of the clear data */

    /* ================5th word(0xBF4B0000)================ */
    mt_u32 ProtectedDataSize:32; /* byte size of the protected data */

    /* ================6th word(0xBF4B0000)================ */
    mt_u32 Q:1;  /* Interrupt the CPU when the current descriptor is done */
    mt_u32 T:1;  /* Terminate:
                  * 1. finish the current descriptor
                  * 2. read the next M2M command from queue */
    mt_u32 P:1;  /* Pause:
                  * 1. finish the current descriptor
                  * 2. wait the retry timer back to zero
                  * 3. read the descriptor again from the external memory, if P=1 go step2, else go step4.
                  * 4. Extract the DescNextAddr, then update to reg_desc_addr
                  * 5. use reg_desc_addr to get the new descriptor */
    mt_u32:2;
    mt_u32 I:1;  /* ECB/CBC/CTR/GCM:
                  * use direct IV, the IV fields are read from external memory below the current descriptor.
                  * if 0, use keytable IV */
    mt_u32 ES:1;  /* End of the Subsample flag:
                   * 0: this descriptor does not include the end of subsample, then the tail
                   *    partial block will be encrypted or decrypted
                   * 1: this descriptor include the end of subsample, then the tail
                   *    partial block will not be encrypted or decrypted*/
    mt_u32 HF:1;  /* HASH/MAC:
                   * 0: the temporary result from the internal register of the M2M ciphers
                   * 1: the temporary result from the software(descriptor) */
    mt_u32 F:1;  /* ECB/CBC/CTR/GCM:
                  * 0: use IVcont register to encrypt the leading block of the data segment
                  * 1: fetch a new IV to encrypt the leading block of the data segment */
    mt_u32 G:1;  /* perform GMAC only, 0:Input Plaintext or Ciphertext, 1:Input AAD or Len */
    mt_u32 E:1;  /* the data segment includes the tail of HASH or MAC */
    mt_u32 S:1;  /* The data segment includes the head of HASH or MAC */
    mt_u32 LS:1;  /* Indicate which LOS value to use
                   * 0: the LOS value comes from the internal register of the M2M ciphers
                   * 1: the LOS value comes from the software(descriptor) */
    mt_u32 AT_ND:1;  /* Next descriptor Address type in descriptor of cmd1, 0: DRAM, 1: BSRAM */
    mt_u32 AT_DAT:1;  /* DataAddress type in descriptor of cmd1, 0: DRAM, 1: BSRAM */
    mt_u32 BS:1;  /* LSkipB and LCryptoB source(only valid for Cens and Cbcs)
                      * 0: from internal register(given by previous descriptor's result)
                      * 1: from the LCryptoB field in current descriptor, and LSkipB is always 0 */
    mt_u32 CC:1;  /* Counter continuous flag, only valid in CTR/CTR64 mode of the TS packet:
                   * 0: the counter value is independent between TS packets
                   * 1: the counter value is continuous between TS packets */
    mt_u32:11;
    mt_u32 LOS:4;  /* Leading offset. for ECB, CBC mode, leave this data clear. For CTR mode, encrypt with IV. */

    /* ================7th word(0xBF4B0000)================ */
    mt_u32 DescNextAddr:32; /* address pointer to the next descriptor */

    /* ================8th~39th word(0xBF4B0000)================ */
    union {
        struct {
            mt_u32 HMAC_tempIn[16]; /* 512 bits value of the temporary result of the HMAC operation */
            mt_u32 HMAC_mackey[16]; /* 512 bits value of the key for HMAC operation */
        };

        struct {
            mt_u32 HASH_MAC_tempIn[16]; /* 512 bits value of the temporary result of the HASH/MAC operation */
        };

        struct {
            mt_u32 GCM_DIV[4]; /* 128 bits direct IV for the plaintext or ciphertext */
            mt_u32 GCM_GHASH[4]; /* 128 bits value to initialize the GHASH for GCM authentication */
            mt_u32 GCM_IVauth[4]; /* 128 bits value to initialize the GCTR for GCM authentication */
        };

        struct {
            mt_u32 ECB_CBC_CTR_DIV[4]; /* 128 bits direct IV for the plaintext or ciphertext */
            mt_u32 CryptoB:4;
            mt_u32 SkipB:4;
            mt_u32 LCryptoB:4;
            mt_u32:20;
            union {
                struct {
                    mt_u32 TS_PID0:16;
                    mt_u32 TS_PID1:16;
                    mt_u32 TS_PID2:16;
                    mt_u32 TS_PID3:16;
                };

                mt_u16 TS_PIDn[4];
            };
        };
    };

    /* ================40st word(software private data)================ */
    /* mt_u32 DescPreAddr: 32; */
} m2m_desc_t;

typedef struct {
    /* ================1st word(0xBF4B0000)================ */
    mt_u32 Cmd:4;  /* cmd2, so fix as 2 */
    mt_u32:12;
    mt_u32 Profile:11; /* ony support sha256, so fix as 0x25 */
    mt_u32 Q:1;  /* Interrupt the CPU when the current command is done */
    mt_u32 Delay:2;  /* Delay Time to execute next command
                      * 0: no delay
                      * 1: 1/4 second
                      * 2: 1/2 second
                      * 3: 1 second */
    mt_u32:2;

    /* ================2nd word(0xBF4B0000)================ */
    mt_u32 Enable:4; /* Any bit 1 denotes that the current command is valid.
                      * The SHA is calculated and the hash result is compared with golden value */
    mt_u32 TP:1; /* bgc type, 0: use HW hash result, only for TEE/VSCPU; 1: use the golden value in cmd2 */
    mt_u32:26;
    mt_u32 AT:1; /* Address type in cmd0/1, 0: DRAM, 1: BSRAM */

    /* ================3rd word(0xBF4B0000)================ */
    mt_u32 SrcDataAddr:32; /* Address pointer of the source data */

    /* ================4th word(0xBF4B0000)================ */
    mt_u32 SrcDataSize:32; /* Byte size of the clear data */

    /* ================5th~12th word(0xBF4B0000)================ */
    mt_u32 GoldenValue[8]; /* Golden value for the security application running in the external memory */
} m2m_bgc_t;

typedef struct {
    mt_u8 wcid;
    mt_u8 scid;
    mt_u8 sc;
    mt_u8 reserved;
} m2m_tee_info_t;

int m2m_check_state(mt_u32 channel, mt_u8 tid);

mt_u8 m2m_alloc_cmd_tid(mt_u32 channel);

void m2m_set_gost_param(mt_u32 channel, mt_u32 bit_inv, mt_u32 sbox_sel);

mt_s32 m2m_send_cmd(mt_u32 channel, const void *cmd, mt_u32 cmd_size);

#ifdef CONFIG_MT_CRYPTOENGINE_SYM6_HW_IRQ_SUPPORT
mt_s32 m2m_check_cmd_irq(mt_u32 channel);

mt_s32 m2m_check_desc_irq(mt_u32 channel);
#endif

mt_s32 m2m_wait_cmd_finish(mt_u32 channel, mt_u8 tid);

mt_s32 m2m_wait_cmds_finish(mt_u32 channel, mt_u8 tids[], mt_u32 tid_num);

mt_s32 m2m_wait_desc_finish(mt_u32 channel, mt_u8 tid);

mt_s32 m2m_wait_descs_finish(mt_u32 channel, mt_u8 tid, mt_u8 desc_id);

mt_s32 m2m_read_iv(mt_u32 channel, mt_u8 *iv, mt_u32 len);

mt_s32 m2m_write_iv(mt_u32 channel, const mt_u8 *iv, mt_u32 len);

mt_s32 m2m_read_hash(mt_u32 channel, mt_u8 *hash, mt_u32 len);

mt_s32 m2m_write_hash(mt_u32 channel, const mt_u8 *hash, mt_u32 len);

mt_s32 m2m_read_final_iv(mt_u32 channel, mt_u8 tid, mt_u8 *iv, mt_u32 len);

mt_s32 m2m_read_final_hash(mt_u32 channel, mt_u8 tid, mt_u8 *hash, mt_u32 len);

mt_u8 m2m_get_mapping_bus(mt_u32 phy_addr, mt_u32 len);

void m2m_print_debug(mt_u32 channel);

void m2m_extra_notice(M2M_EXTRA_NOTICE_TYPE message, M2M_EXTRA_NOTICE_MODE op);

bool is_exist_bgc_notice(M2M_EXTRA_NOTICE_TYPE message);

#endif /*__HW_M2M_IF_H__*/
