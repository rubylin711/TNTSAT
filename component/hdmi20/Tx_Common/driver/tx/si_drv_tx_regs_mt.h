/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _SI_DRV_TX_REGS_MT_H_
#define _SI_DRV_TX_REGS_MT_H_

typedef enum {
	REGTX_SOC_P0_MT	= (BASE_ADDRESS),
	REGTX_SOC_P1_MT	= (BASE_ADDRESS + 0x1000)
} Page_Mt_t;

//***************************************************************************
// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__BIST31_CTRL                                                (REGTX_SOC_P0_MT | 0x0235)
// (RW, Bits 7:0)
#define BIT_MSK__BIST_ERR_GEN_EN                                          (0xf<<2)
//RW
#define BIT_MSK__PCL_PTRN_EN                                        (0x2)
//RW
#define BIT_MSK__PCL_BIST31_EN                                        (0x1)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__BIST31_PATTERN_1_LOW                                                (REGTX_SOC_P0_MT | 0x0236)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__BIST31_PATTERN_1_HIGH                                                (REGTX_SOC_P0_MT | 0x0237)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__VSIF_CTRL								(REGTX_SOC_P0_MT | 0x0689)
// (RW, Bits 7:0)
#define BIT_MSK_REG__VSIF_INTR_MASK					(0x02)
#define BIT_MSK_REG__VSIF_WR_DONE						(0x01)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__HDCP2X_RAM_SELECT                                                (REGTX_SOC_P0_MT | 0x08c7)
// (RW, Bits 7:0)
#define BIT_MSK__REG_HDCP2X_RAM_SELECT                                          (0x3)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__APB2PRIF_CTRL                                                (REGTX_SOC_P1_MT | 0x0000)
// (RW, Bits 31:0)
#define BIT_MSK__APB2PRIF_EN                                          0x1

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_BUS_AD_CTRL                                                (REGTX_SOC_P1_MT | 0x0004)
// (RW, Bits 31:0)
#define BIT_MSK__PRIF_WDATA                                         ((0xff)<<16)
#define BIT_MSK__PRIF_ADDR                                         (0xffff)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_BUS_WR_CTRL                                                (REGTX_SOC_P1_MT | 0x0008)
// (RW, Bits 31:0)
//WC self-clear, can't read
#define BIT_MSK__PRIF_READ_EN                                         0x10
//WC self-clear, can't read
#define BIT_MSK__PRIF_WRITE_EN                                         (0x1)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_BUS_DATA                                               (REGTX_SOC_P1_MT | 0x000c)
// (RW, Bits 31:0)
//Read only
#define BIT_MSK__PRIF_AO_READY                                         (1<<17)
//Read only
#define BIT_MSK__PRIF_READY                                         (1<<16)
//Read only
#define BIT_MSK__PRIF_AO_RDATA                                         (0xff<<8)
//Read only
#define BIT_MSK__PRIF_RDATA                                         (0xff)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__KRAM_CTRL                                               (REGTX_SOC_P1_MT | 0x0010)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__KRAM_ADDR                                         (0x1ff<<16)
//RW 0:nvmi bus, 1:regcfg bus
#define BIT_MSK__KRAM_SEL                                         (1<<12)
//RW -- no use, refer to 0x14[0]
#define BIT_MSK__KRAM_WEN                                         (1<<8)
//RW
#define BIT_MSK__KRAM_DATA                                         (0xff)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__KRAM_W_CTRL                                                (REGTX_SOC_P1_MT | 0x0014)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__KRAM_WR                                         (0x1)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__RNG_CTRL                                                (REGTX_SOC_P1_MT | 0x0020)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__RNG_START                                         (1<<12)
//RW
#define BIT_MSK__SW_RNG_DATA                                         (1<<8)
//RW
#define BIT_MSK__SW_RNG_RDY                                         (1<<4)
//RW
#define BIT_MSK__TRNG_EN                                         (1)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_GUARD                                                (REGTX_SOC_P1_MT | 0x0024)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__PRIF_START_GUARD                                        (0xf<<4)
//RW
#define BIT_MSK__PRIF_END_GUARD                                        (0xf)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__BYTE_RW_MODE                                                (REGTX_SOC_P1_MT | 0x0030)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__BYTE_RW_MODE                                         (1)

#endif // _SI_DRV_TX_REGS_MT_H_
