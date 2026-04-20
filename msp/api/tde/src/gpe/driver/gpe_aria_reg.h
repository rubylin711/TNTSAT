/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __GRA_ARIA_REG_H__
#define __GRA_ARIA_REG_H__

#define GPE_ARIA_MAX_CMDFIFO_SIZE 96
/*!
  comments
  */
#define GPE_ARIA_BASE                   0x00000000 
/*!
  define the reg of symphony gra engine
  */
typedef enum
{  
  GPE_ARIA_GRA_ENG_START         = (GPE_ARIA_BASE + 0x0),
  GPE_ARIA_GRA_EN                = (GPE_ARIA_BASE + 0x4),
  GPE_ARIA_GRA_ENG_CFG           = (GPE_ARIA_BASE + 0x8),
  GPE_ARIA_GRA_CORE_DONE         = (GPE_ARIA_BASE + 0xC),

  GPE_ARIA_SRC0_FMT_CFG0         = (GPE_ARIA_BASE + 0x10),
  GPE_ARIA_SRC0_FMT_CFG1         = (GPE_ARIA_BASE + 0x14),
  GPE_ARIA_SRC0_PIC_ADDR         = (GPE_ARIA_BASE + 0x18),
  GPE_ARIA_SRC0_PIC_STRIDE       = (GPE_ARIA_BASE + 0x1C),

  GPE_ARIA_SRC1_FMT_CFG0         = (GPE_ARIA_BASE + 0x20),
  GPE_ARIA_SRC1_FMT_CFG1         = (GPE_ARIA_BASE + 0x24),
  GPE_ARIA_SRC1_KEY_MIN          = (GPE_ARIA_BASE + 0x28),
  GPE_ARIA_SRC1_KEY_MAX          = (GPE_ARIA_BASE + 0x2C),
  GPE_ARIA_SRC1_PIC_ADDR         = (GPE_ARIA_BASE + 0x30),
  GPE_ARIA_SRC1_PIC_STRIDE       = (GPE_ARIA_BASE + 0x34),
  GPE_ARIA_SRC1_PIC_SIZE         = (GPE_ARIA_BASE + 0x38),
  GPE_ARIA_SRC1_OP_SIZE          = (GPE_ARIA_BASE + 0x3C),
  GPE_ARIA_SRC1_OP_POS           = (GPE_ARIA_BASE + 0x40),
  GPE_ARIA_SRC1_CMYK_CFG         = (GPE_ARIA_BASE + 0x44),
  GPE_ARIA_SRC1_STATUS           = (GPE_ARIA_BASE + 0x4C),

  GPE_ARIA_SRC2_FMT_CFG0         = (GPE_ARIA_BASE + 0x50),
  GPE_ARIA_SRC2_FMT_CFG1         = (GPE_ARIA_BASE + 0x54),
  GPE_ARIA_SRC2_KEY_MIN          = (GPE_ARIA_BASE + 0x58),
  GPE_ARIA_SRC2_KEY_MAX          = (GPE_ARIA_BASE + 0x5C),
  GPE_ARIA_SRC2_PIC_ADDR         = (GPE_ARIA_BASE + 0x60),
  GPE_ARIA_SRC2_PIC_STRIDE       = (GPE_ARIA_BASE + 0x64),
  GPE_ARIA_SRC2_OP_POS           = (GPE_ARIA_BASE + 0x70),
  GPE_ARIA_SRC2_STATUS           = (GPE_ARIA_BASE + 0x74),

  GPE_ARIA_SRC3_FMT_CFG0         = (GPE_ARIA_BASE + 0x80),
  GPE_ARIA_SRC3_FMT_CFG1         = (GPE_ARIA_BASE + 0x84),
  GPE_ARIA_SRC3_KEY_MIN          = (GPE_ARIA_BASE + 0x88),
  GPE_ARIA_SRC3_KEY_MAX          = (GPE_ARIA_BASE + 0x8C),
  GPE_ARIA_SRC3_PIC_ADDR         = (GPE_ARIA_BASE + 0x90),
  GPE_ARIA_SRC3_PIC_STRIDE       = (GPE_ARIA_BASE + 0x94),
  GPE_ARIA_SRC3_OP_POS           = (GPE_ARIA_BASE + 0xA0),
  GPE_ARIA_SRC3_STATUS           = (GPE_ARIA_BASE + 0xA4),

  GPE_ARIA_DST0_FMT_CFG0         = (GPE_ARIA_BASE + 0xB0),
  GPE_ARIA_DST0_FMT_CFG1         = (GPE_ARIA_BASE + 0xB4),
  GPE_ARIA_DST0_PIC_ADDR         = (GPE_ARIA_BASE + 0xB8),
  GPE_ARIA_DST0_PIC_STRIDE       = (GPE_ARIA_BASE + 0xBC),
  GPE_ARIA_DST0_OP_POS           = (GPE_ARIA_BASE + 0xC0),

  GPE_ARIA_DST1_FMT_CFG0         = (GPE_ARIA_BASE + 0xE0),
  GPE_ARIA_DST1_FMT_CFG1         = (GPE_ARIA_BASE + 0xE4),
  GPE_ARIA_DST1_PIC_ADDR         = (GPE_ARIA_BASE + 0xE8),
  GPE_ARIA_DST1_PIC_STRIDE       = (GPE_ARIA_BASE + 0xEC),

  GPE_ARIA_DST1_OP_POS           = (GPE_ARIA_BASE + 0xF0),
  GPE_ARIA_DST_PIC_SIZE          = (GPE_ARIA_BASE + 0xF4),
  GPE_ARIA_DST_OP_SIZE           = (GPE_ARIA_BASE + 0xF8),
  GPE_ARIA_DST_STATUS            = (GPE_ARIA_BASE + 0xFC),

  GPE_ARIA_CMD_FIFO_CTRL         = (GPE_ARIA_BASE + 0x100),
  GPE_ARIA_CMD_FIFO_TRIG_CFG     = (GPE_ARIA_BASE + 0x104),
  GPE_ARIA_CMD_FIFO_ADDR_SYNC    = (GPE_ARIA_BASE + 0x108),
  GPE_ARIA_CMD_FIFO_ADDR_ASYNC   = (GPE_ARIA_BASE + 0x10C),
  GPE_ARIA_CMD_ID0               = (GPE_ARIA_BASE + 0x110),
  GPE_ARIA_CMD_ID1               = (GPE_ARIA_BASE + 0x114),
  GPE_ARIA_CMD_FIFO_STATUS       = (GPE_ARIA_BASE + 0x11C),

  GPE_ARIA_GRA_AXI_CTRL          = (GPE_ARIA_BASE + 0x120),
  GPE_ARIA_GRA_AXI_ATATUS        = (GPE_ARIA_BASE + 0x124),
  GPE_ARIA_GRA_AXI_CFG           = (GPE_ARIA_BASE + 0x128),

  GPE_ARIA_XYLC_CFG              = (GPE_ARIA_BASE + 0x130),
  GPE_ARIA_XYLC_ERR              = (GPE_ARIA_BASE + 0x134),
  GPE_ARIA_XYLC_STATUS           = (GPE_ARIA_BASE + 0x138),

  GPE_ARIA_SCALER_CFG            = (GPE_ARIA_BASE + 0x140),
  GPE_ARIA_SCALER_COEF_11        = (GPE_ARIA_BASE + 0x144),
  GPE_ARIA_SCALER_COEF_21        = (GPE_ARIA_BASE + 0x148),
  GPE_ARIA_SCALER_COEF_31        = (GPE_ARIA_BASE + 0x14C),
  GPE_ARIA_SCALER_COEF_22        = (GPE_ARIA_BASE + 0x150),
  GPE_ARIA_SCALER_COEF_23        = (GPE_ARIA_BASE + 0x154),
  GPE_ARIA_SCALER_INIT_PHASE     = (GPE_ARIA_BASE + 0x158),
  GPE_ARIA_SCALER_STATUS         = (GPE_ARIA_BASE + 0x160),
  GPE_ARIA_FAST_SCALER_INIT_PHASE        = (GPE_ARIA_BASE + 0x164),

  GPE_ARIA_ROT_PAT_CFG           = (GPE_ARIA_BASE + 0x180),
  GPE_ARIA_PAT_COLOR             = (GPE_ARIA_BASE + 0x184),
  GPE_ARIA_PAT_OFFSET_POS        = (GPE_ARIA_BASE + 0x188),
  GPE_ARIA_PAT_RATIO_X_0         = (GPE_ARIA_BASE + 0x18C),
  GPE_ARIA_PAT_RATIO_X_1         = (GPE_ARIA_BASE + 0x190),
  GPE_ARIA_PAT_RATIO_Y_0         = (GPE_ARIA_BASE + 0x194),
  GPE_ARIA_PAT_RATIO_Y_1         = (GPE_ARIA_BASE + 0x198),

  GPE_ARIA_GRADT_CFG             = (GPE_ARIA_BASE + 0x1A0),
  GPE_ARIA_GRADT_X_STEP          = (GPE_ARIA_BASE + 0x1A4),
  GPE_ARIA_GRADT_Y_STEP          = (GPE_ARIA_BASE + 0x1A8),
  GPE_ARIA_GRADT_START_V         = (GPE_ARIA_BASE + 0x1Ac),
  GPE_ARIA_STOP0_ARGB            = (GPE_ARIA_BASE + 0x1B0),
  GPE_ARIA_STOP1_ARGB            = (GPE_ARIA_BASE + 0x1B4),
  GPE_ARIA_STOP2_ARGB            = (GPE_ARIA_BASE + 0x1B8),
  GPE_ARIA_STOP3_ARGB            = (GPE_ARIA_BASE + 0x1Bc),
  GPE_ARIA_STOP_OFFSET           = (GPE_ARIA_BASE + 0x1C0),
  GPE_ARIA_STOP0_FACT            = (GPE_ARIA_BASE + 0x1C4),
  GPE_ARIA_STOP1_FACT            = (GPE_ARIA_BASE + 0x1C8),
  GPE_ARIA_STOP2_FACT            = (GPE_ARIA_BASE + 0x1CC),

  GPE_SPN_REGION_CFG             = (GPE_ARIA_BASE + 0x1D0),
  GPE_SPN_REGION_START           = (GPE_ARIA_BASE + 0x1D4),
  GPE_SPN_REGION_STOP            = (GPE_ARIA_BASE + 0x1D8),

  GPE_ARIA_COMP_CFG              = (GPE_ARIA_BASE + 0x1E0),
  GPE_ARIA_COMP_MULT_MOD         = (GPE_ARIA_BASE + 0x1E4),
  GPE_SPN_COMP_MULT_MOD2         = (GPE_ARIA_BASE + 0x1E8),
  GPE_ARIA_COMP_BLD_MOD          = (GPE_ARIA_BASE + 0x1EC),
  GPE_ARIA_ROP_ID                = (GPE_ARIA_BASE + 0x1F0),
  GPE_ARIA_ROP_PAT               = (GPE_ARIA_BASE + 0x1F4),
  GPE_ARIA_COMP_STATUS           = (GPE_ARIA_BASE + 0x1F8),
  GPE_SPN_COMP_COLORIZE          = (GPE_ARIA_BASE + 0x1FC),

  GPE_ARIA_LOAD_EN               = (GPE_ARIA_BASE + 0x200),
  GPE_ARIA_PAL_SIZE              = (GPE_ARIA_BASE + 0x204),
  GPE_ARIA_PAL1_ADDR             = (GPE_ARIA_BASE + 0x208),
  GPE_ARIA_PAL3_ADDR             = (GPE_ARIA_BASE + 0x20c),
  GPE_ARIA_COEF_ADDR             = (GPE_ARIA_BASE + 0x210),
  GPE_ARIA_CTRL_STATUS           = (GPE_ARIA_BASE + 0x214),

  GPE_ARIA_SRC1_TILE_CFG         = (GPE_ARIA_BASE + 0x260),
  GPE_ARIA_SRC1_TILE_JMP00       = (GPE_ARIA_BASE + 0x264),
  GPE_ARIA_SRC1_TILE_JMP01       = (GPE_ARIA_BASE + 0x268),
  GPE_ARIA_SRC1_TILE_JMP10       = (GPE_ARIA_BASE + 0x26C),
  GPE_ARIA_SRC1_TILE_JMP11       = (GPE_ARIA_BASE + 0x270),

  GPE_ARIA_GRA_INT_EN            = (GPE_ARIA_BASE + 0x2E0),
  GPE_ARIA_GRA_INT_STATE         = (GPE_ARIA_BASE + 0x2E4),
  GPE_ARIA_GRA_STATE             = (GPE_ARIA_BASE + 0x2E8),
  GPE_ARIA_GRA_INT_MOD           = (GPE_ARIA_BASE + 0x2EC),
  GPE_ARIA_GRA_CLK_GATED         = (GPE_ARIA_BASE + 0x2F0),

  GPE_ARIA_GRP0_CSCP_0           = (GPE_ARIA_BASE + 0x300),
  GPE_ARIA_GRP0_CSCP_1           = (GPE_ARIA_BASE + 0x304),
  GPE_ARIA_GRP0_CSCP_2           = (GPE_ARIA_BASE + 0x308),
  GPE_ARIA_GRP0_CSCP_3           = (GPE_ARIA_BASE + 0x30C),
  GPE_ARIA_GRP0_CSCP_4           = (GPE_ARIA_BASE + 0x310),
  GPE_ARIA_GRP0_CSCDC_0          = (GPE_ARIA_BASE + 0x314),
  GPE_ARIA_GRP0_CSCDC_1          = (GPE_ARIA_BASE + 0x318),
  GPE_ARIA_GRP0_CSCDC_2          = (GPE_ARIA_BASE + 0x31C),
  GPE_ARIA_GRP1_CSCP_0           = (GPE_ARIA_BASE + 0x320),
  GPE_ARIA_GRP1_CSCP_1           = (GPE_ARIA_BASE + 0x324),
  GPE_ARIA_GRP1_CSCP_2           = (GPE_ARIA_BASE + 0x328),
  GPE_ARIA_GRP1_CSCP_3           = (GPE_ARIA_BASE + 0x32C),
  GPE_ARIA_GRP1_CSCP_4           = (GPE_ARIA_BASE + 0x330),
  GPE_ARIA_GRP1_CSCDC_0          = (GPE_ARIA_BASE + 0x334),
  GPE_ARIA_GRP1_CSCDC_1          = (GPE_ARIA_BASE + 0x338),
  GPE_ARIA_GRP1_CSCDC_2          = (GPE_ARIA_BASE + 0x33C),

  GPE_ARIA_GRADT_RADIUS_A_0      = (GPE_ARIA_BASE + 0x360),
  GPE_ARIA_GRADT_RADIUS_A_1      = (GPE_ARIA_BASE + 0x364),
  GPE_ARIA_GRADT_RADIUS_B_0      = (GPE_ARIA_BASE + 0x368),
  GPE_ARIA_GRADT_RADIUS_B_1      = (GPE_ARIA_BASE + 0x36C),
  GPE_ARIA_GRADT_RADIUS_C_0      = (GPE_ARIA_BASE + 0x370),
  GPE_ARIA_GRADT_RADIUS_C_1      = (GPE_ARIA_BASE + 0x374),
  GPE_ARIA_GRADT_RADIUS_D_0      = (GPE_ARIA_BASE + 0x378),
  GPE_ARIA_GRADT_RADIUS_D_1      = (GPE_ARIA_BASE + 0x37C),
  GPE_ARIA_GRADT_RADIUS_E_0      = (GPE_ARIA_BASE + 0x380),
  GPE_ARIA_GRADT_RADIUS_E_1      = (GPE_ARIA_BASE + 0x384),
  GPE_ARIA_WCH_REQ_MSK_SYNC_MODE      = (GPE_ARIA_BASE + 0x388),

  GPE_ARIA_GRA_PIN_SEL           = (GPE_ARIA_BASE + 0x3B0),
  GPE_ARIA_GRA_MAC_SET           = (GPE_ARIA_BASE + 0x3B4),
  GPE_ARIA_GRA_REQ_CFG           = (GPE_ARIA_BASE + 0x3B8),
  GPE_ARIA_GRA_REQ_SEL           = (GPE_ARIA_BASE + 0x3BC),
  
  GPE_ARIA_PFM_MODE              = (GPE_ARIA_BASE + 0x3C0),  
  GPE_ARIA_SRC0_PIX_STATUS0      = (GPE_ARIA_BASE + 0x3D0),
  GPE_ARIA_SRC0_PIX_STATUS1      = (GPE_ARIA_BASE + 0x3D4),
  GPE_ARIA_SRC1_PIX_STATUS0      = (GPE_ARIA_BASE + 0x3D8),
  GPE_ARIA_SRC1_PIX_STATUS1      = (GPE_ARIA_BASE + 0x3DC),
  GPE_ARIA_SRC2_PIX_STATUS0      = (GPE_ARIA_BASE + 0x3E0),
  GPE_ARIA_SRC2_PIX_STATUS1      = (GPE_ARIA_BASE + 0x3E4),
  GPE_ARIA_SRC3_PIX_STATUS0      = (GPE_ARIA_BASE + 0x3E8),
  GPE_ARIA_SRC3_PIX_STATUS1      = (GPE_ARIA_BASE + 0x3EC),
}spn_gpe_reg_t;


/*!
  comment
  */
void aria_gpe_init_reg(MT_BOOL hw_init);

/*!
  comment
  */
void aria_gpe_print_reg(void);

/*!
  comment
  */
void aria_gpe_reg_test(void);

#endif 


