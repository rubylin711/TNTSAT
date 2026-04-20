/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.													*/
/* Montage Proprietary and Confidential														*/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies			*/
/********************************************************************************************/
#ifndef _SI_DRV_TX_REGS_MT_H_
#define _SI_DRV_TX_REGS_MT_H_

typedef enum {
	REGTX_SOC_P0_MT	= (BASE_ADDRESS),				//cypress
	REGTX_SOC_P1_MT	= (BASE_ADDRESS + 0x1000),	//regmisc
	REGTX_SOC_P2_MT	= (BASE_ADDRESS + 0x2000),	//emp_ram
	REGTX_SOC_HDCP_KRAM_MT	= (0xBF130000UL)	//key_ram
} Page_Mt_t;

//***************************************************************************
// CEA-861 VSI InfoFrame MHL IEEE No #0 Register
#define REG_ADDR__KEEP_OUT_WIN_SIZE_0_MT								(REGTX_SOC_P0 | 0x0072)
// (ReadWrite, Bits 7:0)
// Read CEA-861 for detailed description of this register OUI=0x7CA61D
#define BIT_MSK__KEEP_OUT_WIN_MAX_B12_B8								0xF1
#define BIT_MSK__KEEP_OUT_WIN_MIN_B8									0x4

// CEA-861 VSI InfoFrame MHL IEEE No #0 Register
#define REG_ADDR__KEEP_OUT_WIN_CTRL_MT									(REGTX_SOC_P0 | 0x0073)
// (ReadWrite, Bits 7:0)
// Read CEA-861 for detailed description of this register OUI=0x7CA61D
#define BIT_MSK__PIX_X4_MODE											0x80
#define BIT_MSK__KEEP_OUT_WIN_EN_REQ_SEL								0x40

// CEA-861 VSI InfoFrame MHL IEEE No #0 Register
#define REG_ADDR__KEEP_OUT_WIN_MAX_LOW_MT								(REGTX_SOC_P0 | 0x0074)
// (ReadWrite, Bits 7:0)
// Read CEA-861 for detailed description of this register OUI=0x7CA61D
#define BIT_MSK__KEEP_OUT_WIN_MAX_B7_B0									0xFF

// SCDT Holdoff MSB Register
#define REG_ADDR__KEEP_OUT_WIN_MIN_LOW_MT								(REGTX_SOC_P0 | 0x00A4)
// (ReadWrite, Bits 7:0)
// MSB of 24 bit holdoff counter threshold that deglitches SCDT high active status.
#define BIT_MSK__KEEP_OUT_WIN_MIN_B7_B0							0xFF

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__HDR10PLUS_CFG											(REGTX_SOC_P0_MT | 0x00FC)
// (RW, Bits 7:0)
#define BIT_MSK__HDR10P_WIN										(0x3)		// 00: Inside MTW, for DMA mode EMP application
//01: Inside MTW, for DMA mode VSIF application
//1x: for CPU_ONLY mode

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__BIST31_CTRL											(REGTX_SOC_P0_MT | 0x0235)
// (RW, Bits 7:0)
#define BIT_MSK__BIST_ERR_GEN_EN										(0xf<<2)
//RW
#define BIT_MSK__PCL_PTRN_EN										(0x2)
//RW
#define BIT_MSK__PCL_BIST31_EN										(0x1)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__BIST31_PATTERN_1_LOW									(REGTX_SOC_P0_MT | 0x0236)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__BIST31_PATTERN_1_HIGH									(REGTX_SOC_P0_MT | 0x0237)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__PHY_BIST_CFG											(REGTX_SOC_P0_MT | 0x0240)
// (RW, Bits 31:0)
#define BIT_MSK__REG_BIST_SKP_EN 										(1<<26)
#define BIT_MSK__REG_BIST_SKP_RATIO 									(3<<24)
#define BIT_MSK__REG_BIST_SCRMB_DIS 									(1<<19)
#define BIT_MSK__REG_BIST_8B10B_ENC_BP									(1<<18)
#define BIT_MSK__REG_BIST_RUN											(1<<15)
#define BIT_MSK__REG_BIST_CLR											(1<<12)
#define BIT_MSK__REG_BIST_PKT_TYPE										(0xf<<1)
#define BIT_MSK__REG_BIST_EN											(1<<0)
// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__PHY_BIST_STATUS										(REGTX_SOC_P0_MT | 0x0248)
// (RW, Bits 31:0)
#define BIT_MSK__REG_BIST_TX_SKP_ERR									(1<<11)
#define BIT_MSK__REG_BIST_TX_DONE										(1<<10)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__PHY_BIST_PKT_CFG0										(REGTX_SOC_P0_MT | 0x0250)
// (RW, Bits 31:0)
#define BIT_MSK__REG_BIST_LOOP_NUM										(0xffff<<16)
#define BIT_MSK__REG_BIST_COM_LEN										(0xf<<12)
#define BIT_MSK__REG_BIST_DATA_LEN										(0x7ff<<0)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__PHY_BIST_PKT_CFG1										(REGTX_SOC_P0_MT | 0x0254)
// (RW, Bits 31:0)
#define BIT_MSK__REG_BIST_LOOP_NUM_PRE									(0xffff<<16)
#define BIT_MSK__REG_BIST_COM_LEN_PRE									(0xf<<12)
#define BIT_MSK__REG_BIST_DATA_LEN_PRE									(0x7ff<<0)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__PHY_BIST_PKT_CFG2										(REGTX_SOC_P0_MT | 0x0258)
// (RW, Bits 31:0)
#define BIT_MSK__REG_BIST_SKP_LEN_PRE									(0x7ff<<0)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__PHY_BIST_CFG1											(REGTX_SOC_P0_MT | 0x025C)
// (RW, Bits 31:0)
#define BIT_MSK__REG_BIST_LOAD											(0x1<<0)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__EMP_START_LINE_LOW									(REGTX_SOC_P0_MT | 0x0682)
// (RW, Bits 7:0)
#define BIT_MSK__REG_EMP_START_LINE_LOW									(0xff)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__EMP_START_LINE_HIGH									(REGTX_SOC_P0_MT | 0x0683)
// (RW, Bits 7:0)
#define BIT_MSK__REG_EMP_START_LINE_HIGH								(0xff)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__EMP_END_LINE_LOW										(REGTX_SOC_P0_MT | 0x0684)
// (RW, Bits 7:0)
#define BIT_MSK__REG_EMP_END_LINE_LOW									(0xff)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__EMP_END_LINE_HIGH										(REGTX_SOC_P0_MT | 0x0685)
// (RW, Bits 7:0)
#define BIT_MSK__REG_EMP_END_LINE_HIGH									(0xff)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__EMP_CTRL								(REGTX_SOC_P0_MT | 0x0686)
// (RW, Bits 7:0)
#define BIT_MSK__REG_EMP_REPEAT_1						(0x80)
#define BIT_MSK__REG_DMA_EN								(0x40)
#define BIT_MSK__REG_EMP_EN								(0x20)
#define BIT_MSK__REG_AUD_EMP_PRIORITY					(0x10)
#define BIT_MSK__REG_EMP_REPEAT							(0x08)
#define BIT_MSK__REG_EMP_FINISH							(0x04)
#define BIT_MSK__REG_EMP_START							(0x02)
#define BIT_MSK__REG_EMP_NEW_SET_EN						(0x01)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__INTR_MASK							(REGTX_SOC_P0_MT | 0x0687)
// (RW, Bits 7:0)
#define BIT_MSK_REG__DMA_DONE_MASK					(0x20)
#define BIT_MSK_REG__MTW_FALLING_EDGE_MASK			(0x10)
#define BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK			(0x04)
#define BIT_MSK_REG__EMP_ERR_HDMI_MASK				(0x02)
#define BIT_MSK_REG__EMP_ERR_CPU_MASK				(0x01)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__INTR_STATUS							(REGTX_SOC_P0_MT | 0x0688)
// (RW, Bits 7:0)
#define BIT_MSK_REG__DMA_DONE							(0x20)
#define BIT_MSK_REG__MTW_FALLING_EDGE					(0x10)
#define BIT_MSK_REG__EMP_SUCCESS_HDMI					(0x04)
#define BIT_MSK_REG__EMP_ERR_HDMI						(0x02)
#define BIT_MSK_REG__EMP_ERR_CPU						(0x01)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__VSIF_CTRL								(REGTX_SOC_P0_MT | 0x0689)
// (RW, Bits 7:0)
#define BIT_MSK_REG__VSIF_INTR_MASK						(0x02)
#define BIT_MSK_REG__VSIF_WR_DONE						(0x01)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__VSIF_INTR_STATUS						(REGTX_SOC_P0_MT | 0x068a)
// (RW, Bits 7:0)
#define BIT_MSK_REG__VSIF_SEND_DONE_STATE				(0x02)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__HDCP_CTRL_OWNER_CFG					(REGTX_SOC_P0_MT | 0x068b)
// (RW, Bits 7:0)
#define BIT_MSK_REG__HDCP_CTRL_OWNER_CFG				(0x02)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__EMP_CTRL1													(REGTX_SOC_P0_MT | 0x06A0)
// (RW, Bits 31:0)	DMA mode only
#define BIT_MSK__REG_EMP_DONE												(1<<31)
#define BIT_MSK__REG_DMA_DONE												(1<<30)
#define BIT_MSK__REG_EMP_MTW_MODE											(1<<20)
#define BIT_MSK__REG_WR_MEM_BANK_CFG										(1<<19)
#define BIT_MSK__REG_WR_MEM_BANK_MODE										(1<<18)
#define BIT_MSK__REG_MEM_BANK_CFG											(1<<17)
#define BIT_MSK__REG_MEM_BANK_MODE											(1<<16)
#define BIT_MSK__REG_DMA_PKT_NUM											(0x3F<<8)
#define BIT_MSK__REG_CPU_EN													(1<<2)
#define BIT_MSK__REG_DMA_STOP												(1<<1)
#define BIT_MSK__REG_EMP_MODE												(1<<0)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__INT_MUX													(REGTX_SOC_P0_MT | 0x06A4)
// (RW, Bits 31:0)	DMA mode only
#define BIT_MSK__REG_INT_MUX												(3<<0)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__EMP_MTW_START_CFG											(REGTX_SOC_P0_MT | 0x06B0)
// (RW, Bits 31:0)	DMA mode only
// CNT >= reg_mtw_start_pix (con1)
// CNT <= reg_mtw_end_pix	(con2)
// emp_mtw_mode = 1: (window is in one frame), con1&&con2 window is valid
// emp_mtw_mode = 0: (window is in two frames), con1||con2 window is valid
#define BIT_MSK__REG_MTW_START_PIX 											(0xFFFFFF)

// REGTX_SOC_P0_MT. 4bytes data
#define REG_ADDR__EMP_MTW_END_CFG											(REGTX_SOC_P0_MT | 0x06B4)
// (RW, Bits 31:0)	DMA mode only
// CNT >= reg_mtw_start_pix (con1)
// CNT <= reg_mtw_end_pix	(con2)
// emp_mtw_mode = 1: (window is in one frame), con1&&con2 window is valid
// emp_mtw_mode = 0: (window is in two frames), con1||con2 window is valid
#define BIT_MSK__REG_MTW_END_PIX 											(0xFFFFFF)

// REGTX_SOC_P0_MT. 1bytes data
#define REG_ADDR__HDCP2X_RAM_SELECT						(REGTX_SOC_P0_MT | 0x08c7)
// (RW, Bits 7:0)
#define BIT_MSK__REG_HDCP2X_RAM_SELECT					(0x3)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__APB2PRIF_CTRL							(REGTX_SOC_P1_MT | 0x0000)
// (RW, Bits 31:0)
#define BIT_MSK__APB2PRIF_EN							0x1

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_BUS_AD_CTRL						(REGTX_SOC_P1_MT | 0x0004)
// (RW, Bits 31:0)
#define BIT_MSK__PRIF_WDATA								((0xff)<<16)
#define BIT_MSK__PRIF_ADDR								(0xffff)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_BUS_WR_CTRL						(REGTX_SOC_P1_MT | 0x0008)
// (RW, Bits 31:0)
//WC self-clear, can't read
#define BIT_MSK__PRIF_READ_EN							0x10
//WC self-clear, can't read
#define BIT_MSK__PRIF_WRITE_EN							(0x1)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_BUS_DATA							(REGTX_SOC_P1_MT | 0x000c)
// (RW, Bits 31:0)
//Read only
#define BIT_MSK__PRIF_AO_READY							(1<<17)
//Read only
#define BIT_MSK__PRIF_READY								(1<<16)
//Read only
#define BIT_MSK__PRIF_AO_RDATA							(0xff<<8)
//Read only
#define BIT_MSK__PRIF_RDATA								(0xff)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PRIF_GUARD							(REGTX_SOC_P1_MT | 0x0024)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__PRIF_START_GUARD						(0xf<<4)
//RW
#define BIT_MSK__PRIF_END_GUARD							(0xf)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__VIDEO_BYPASS							(REGTX_SOC_P1_MT | 0x0034)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__VIDEO_PATH_CORE_BYPASS					(1)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__VIDGEN_HWCG							(REGTX_SOC_P1_MT | 0x0038)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__VIDGEN_HWCG							(1)

// REGTX_SOC_P1_MT. 4bytes data
#define REG_ADDR__PWLI_HWCG								(REGTX_SOC_P1_MT | 0x003C)
// (RW, Bits 31:0)
//RW
#define BIT_MSK__PWLI_HWCG								(1)

// REGTX_SOC_P2_MT. 4bytes data
#define REG_ADDR__EMP_RAM								(REGTX_SOC_P2_MT | 0x0000)
// (RW, Bits 31:0)
#define BIT_MSK__EMP_RAM								(0xFFFFFFFF)

#define REG__EMP_RAM_SIZE								(287)	// 4byte * size
#define REG__EMP_DMA_RAM_SIZE							(32*37+4)	// 4byte * size

// REGTX_SOC_HDCP_KEY_RAM_MT. 4bytes data
#define REG_ADDR__HDCP_KRAM_BASE						(REGTX_SOC_HDCP_KRAM_MT | 0x0000)
// (RW, Bits 31:0)

// REGTX_SOC_HDCP_KEY_RAM_MT. 4bytes data
#define REG_ADDR__READ_EN								(REGTX_SOC_HDCP_KRAM_MT | 0x6000)
// (RW, Bits 31:0)
#define BIT_MSK__READ_EN								(0x1)

// REGTX_SOC_HDCP_KEY_RAM_MT. 4bytes data
#define REG_ADDR__KEY_LOCK_CTRL							(REGTX_SOC_HDCP_KRAM_MT | 0x6004)
// (RW, Bits 31:0)
#define BIT_MSK__HDCP_KEYWR_LOCK						(0x3 << 28)

// REGTX_SOC_HDCP_KEY_RAM_MT. 4bytes data
#define REG_ADDR__HCLK_EN								(REGTX_SOC_HDCP_KRAM_MT | 0x7000)
// (RW, Bits 31:0)
#define BIT_MSK__HCLK_EN								(0x1)

#endif // _SI_DRV_TX_REGS_MT_H_
