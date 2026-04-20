#ifndef TDE_BOOT
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <asm/barrier.h>
#else
#include "list.h"
#endif
#include "tde_hal.h"
#include "tde_define.h"
#include "wmalloc.h"

#include "mt_common.h"
#include "mt_drv_mmz.h"
//#include "tde_filterPara.h"
#include "tde_adp.h"

#include "tde_hal_aria_reg_addr.h"
#include "tde_hal_aria_reg_def.h"

#include "tde_hal_aria.h"
#include "tde_hal_aria_vsb.h"
#include "mt_module_debug.h"
#include "mt_reg_io.h"
#if (defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)|| defined(CONFIG_MT_CHIP_SYMPHONY6))
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#endif
#define USE_NORMAL_GPE

#define TDE_HAL_PRINT MT_INFO_TDE
#define TDE_REG_SIZE 0x1000

//#define TDE_DEBUG_DISABLE_2
#if defined(TDE_DEBUG_DISABLE) || defined(TDE_DEBUG_DISABLE_2)
//#if 0
#define DUMP_LOG \
    do {         \
    } while (0)
#define TDE_FUN_IN DUMP_LOG
#define TDE_FUN_OUT DUMP_LOG
#define TDE_LOG(...) DUMP_LOG
#define TDE_LINE DUMP_LOG
#else
#define TDE_FUN_IN MT_INFO_TDE("---------------in---------------\n")
#define TDE_FUN_OUT MT_INFO_TDE("---------------out-------------\n")
#define TDE_LOG MT_INFO_TDE
#define TDE_LINE MT_INFO_TDE("-----------------\n")
#endif

#define _TDE_HAL_HW_NODE_PIRNT_ON_
//#define  _TDE_HAL_REGISTERS_PIRNT_ON_

extern mt_void TdeOsiPrintDrvSurface(TDE_DRV_SURFACE_S *p_surface);
extern mt_void TdeHalAriaPrintAllReg(mt_void);
extern ulong mt_get_sys_ctrl_base(void);

#define GET_REG(A) HAL_GET_U32((volatile u32 *)((ulong)A))
#define PUT_REG(A, V) HAL_PUT_U32((volatile u32 *)((ulong)A), (u32)(V))
/*#define TDE_REG_WRITE(reg_offset, value)               \
    \
{                                               \
  mt_u32 reg = g_tde_aria_vir_base + reg_offset; \
  PUT_REG(reg, value);                           \
    \
}*/


#define TDE_REG_READ(reg_offset) GET_REG((g_tde_aria_vir_base + reg_offset))

//#define tde_aria_node_offset(s,m)   (size_t)&(((s *)0)->m)

//#define  NODE_CMD_SET_DATA(cmd, value)    cmd.data = value

#define TdeNodeSetData(p_node, NODE_NAME, value) \
    \
{                                         \
  p_node->NODE_NAME.data = value;          \
    \
}

#define TdeNodeSet_REG(p_node, NODE_NAME, value) \
    \
{                                         \
  p_node->NODE_NAME.base.bitc.reg = value; \
    \
}

#define TdeNodeSet_CMD_END(p_node, NODE_NAME, value) \
    \
{                                             \
  p_node->NODE_NAME.base.bitc.cmd_end = value; \
    \
}

#define TdeNodeSet_NEXT_NODE(p_node, NODE_NAME, value) \
    \
{                                               \
  p_node->NODE_NAME.base.bitc.next_node = value; \
    \
}

#define TdeNodeSet_SUSBEND2(p_node, NODE_NAME, value) \
    \
{                                              \
  p_node->NODE_NAME.base.bitc.suspend2 = value; \
    \
}

#define TdeNodeSet_USED(p_node, NODE_NAME, value) \
    \
{                                          \
  p_node->NODE_NAME.base.bitc.used = value; \
    \
}

#define TdeNodeGetData(p_node, NODE_NAME) p_node->NODE_NAME.data

static inline mt_void TdeHalNodeCMDSet(ARIA_TDE_CMD_t *p_cmd, u32 data)
{
    p_cmd->data = (reg_data_t)data;
}

static ulong g_tde_aria_vir_base;

/* Deflicker level, default is auto */
static TDE_DEFLICKER_LEVEL_E s_eDeflickerLevel = TDE_DEFLICKER_AUTO;

/* alpha threshold switch */
static MT_BOOL s_bEnAlphaThreshold = MT_FALSE;

/*alpha threshold value */
static mt_u8 s_u8AlphaThresholdValue = 0xff;

static mmz_buffer_s stCoeffTableBuf;
//  aria tde scale coeff table
static __attribute__((aligned(128))) mt_u32 tde_aria_scale_coeff[] = {
#include "tde_aria_scale_table.h"
};

static mt_void TdeHalNodeSetRopID(TDE_HWNode_S *pHWNode, rop_mod_t rop_c, rop_mod_t rop_a);

static mt_void TDE_REG_WRITE(mt_u32 reg_offset, mt_u32 value)
{
  ulong reg = g_tde_aria_vir_base + reg_offset;
//  MT_INFO_TDE("TDE_REG_WRITE: [0x%x] = 0x%x\n", reg_offset, value);
  PUT_REG(reg, value);

}

static mt_void TdeHalInitIrqMask(mt_void)
{
    mt_u32 int_mask = 0;
    //int_mask |= 1 << 8;
    int_mask |= 1 << 31;

    TDE_REG_WRITE(ARIA_TDE_REG_GRA_INT_EN, int_mask); // cmd fifi list end
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_INT_MOD, 0x2);     // enable interrupt, high level int
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4)   || defined(CONFIG_MT_CHIP_SYMPHONY6)  
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_INT_MOD, 0x0);     // enable interrupt, high level int
#endif
    return;
}
mt_void TdeHalStart(mt_void)
{
  TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, 1);
}
mt_void TdeHalCfStart(mt_void)
{
  TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, 1 << 6);
}

mt_void TdeHalCfSyncStart(mt_void)
{
  TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, 1 << 4);
}

static mt_void aria_tde_reg_clr_start(mt_void)
{

    REG_GRA_ENG_START gra_start;
    gra_start.all = 0x0;
    gra_start.bitc.reg_clr_start = 0x1;

    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, gra_start.all);

    return;
}
#ifndef  USE_NORMAL_GPE
static mt_void aria_tde_async_cmdfifo_start(mt_u32 u32NodePhyAddr)
{
    REG_GRA_ENG_START gra_start;
    gra_start.all = 0x0;
    gra_start.bitc.async_start = 0x1;

    TDE_REG_WRITE(ARIA_TDE_REG_CMD_FIFO_ADDR_ASYNC, u32NodePhyAddr);

    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, gra_start.all);

    return;
}
#endif 
static mt_void aria_tde_gra_state_clear_all_done(mt_void)
{
    REG_GRA_STATE gra_state;
    gra_state.all = 0x0;
    gra_state.bitc.gra_state = 0x1 << 31;
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_STATE, gra_state.all);

    return;
}

#ifndef  USE_NORMAL_GPE
static MT_BOOL aria_tde_gra_cmdfifo_read_end_check(MT_BOOL reset)
{
    mt_u32 ID1 = 0;

    if (reset)
    {
        TDE_REG_WRITE(ARIA_TDE_REG_CMD_FIFO_ID1, 0x0);
        return MT_TRUE;
    }
    else
    {
        ID1 = TDE_REG_READ(ARIA_TDE_REG_CMD_FIFO_ID1);
        return ID1 ? MT_TRUE : MT_FALSE;
    }
}
#endif 

mt_void TdeHalHwNodeInitAriaNode(ARIA_TDE_HWNODE_t *p_hw_node)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x]\n", (mt_u32)p_hw_node);

    p_hw_node->GRA_EN.base.bitc.reg = ARIA_TDE_REG_GRA_ENG_EN;
    p_hw_node->SRC0_FMT_CFG0.base.bitc.reg = ARIA_TDE_REG_SRC0_FMT_CFG0;
    p_hw_node->SRC0_FMT_CFG1.base.bitc.reg = ARIA_TDE_REG_SRC0_FMT_CFG1;
    p_hw_node->SRC0_PIC_ADDR.base.bitc.reg = ARIA_TDE_REG_SRC0_PIC_ADDR;
    p_hw_node->SRC0_PIC_STRIDE.base.bitc.reg = ARIA_TDE_REG_SRC0_PIC_STRIDE;
    p_hw_node->SRC1_FMT_CFG0.base.bitc.reg = ARIA_TDE_REG_SRC1_FMT_CFG0;
    p_hw_node->SRC1_FMT_CFG1.base.bitc.reg = ARIA_TDE_REG_SRC1_FMT_CFG1;
    p_hw_node->SRC1_KEY_MIN.base.bitc.reg = ARIA_TDE_REG_SRC1_KEY_MIN;
    p_hw_node->SRC1_KEY_MAX.base.bitc.reg = ARIA_TDE_REG_SRC1_KEY_MAX;
    p_hw_node->SRC1_PIC_ADDR.base.bitc.reg = ARIA_TDE_REG_SRC1_PIC_ADDR;
    p_hw_node->SRC1_PIC_STRIDE.base.bitc.reg = ARIA_TDE_REG_SRC1_PIC_STRIDE;
    p_hw_node->SRC1_PIC_SIZE.base.bitc.reg = ARIA_TDE_REG_SRC1_PIC_SIZE;
    p_hw_node->SRC1_OP_SIZE.base.bitc.reg = ARIA_TDE_REG_SRC1_OP_SIZE;
    p_hw_node->SRC1_OP_POS.base.bitc.reg = ARIA_TDE_REG_SRC1_OP_POS;
    p_hw_node->SRC1_CMYK_CFG.base.bitc.reg = ARIA_TDE_REG_SRC1_CMYK_CFG;
    p_hw_node->SRC2_FMT_CFG0.base.bitc.reg = ARIA_TDE_REG_SRC2_FMT_CFG0;
    p_hw_node->SRC2_FMT_CFG1.base.bitc.reg = ARIA_TDE_REG_SRC2_FMT_CFG1;
    p_hw_node->SRC2_KEY_MIN.base.bitc.reg = ARIA_TDE_REG_SRC2_KEY_MIN;
    p_hw_node->SRC2_KEY_MAX.base.bitc.reg = ARIA_TDE_REG_SRC2_KEY_MAX;
    p_hw_node->SRC2_PIC_ADDR.base.bitc.reg = ARIA_TDE_REG_SRC2_PIC_ADDR;
    p_hw_node->SRC2_PIC_STRIDE.base.bitc.reg = ARIA_TDE_REG_SRC2_PIC_STRIDE;
    p_hw_node->SRC2_OP_POS.base.bitc.reg = ARIA_TDE_REG_SRC2_OP_POS;
    p_hw_node->SRC3_FMT_CFG0.base.bitc.reg = ARIA_TDE_REG_SRC3_FMT_CFG0;
    p_hw_node->SRC3_FMT_CFG1.base.bitc.reg = ARIA_TDE_REG_SRC3_FMT_CFG1;
    p_hw_node->SRC3_KEY_MIN.base.bitc.reg = ARIA_TDE_REG_SRC3_KEY_MIN;
    p_hw_node->SRC3_KEY_MAX.base.bitc.reg = ARIA_TDE_REG_SRC3_KEY_MAX;
    p_hw_node->SRC3_PIC_ADDR.base.bitc.reg = ARIA_TDE_REG_SRC3_PIC_ADDR;
    p_hw_node->SRC3_PIC_STRIDE.base.bitc.reg = ARIA_TDE_REG_SRC3_PIC_STRIDE;
    p_hw_node->SRC3_OP_POS.base.bitc.reg = ARIA_TDE_REG_SRC3_OP_POS;
    p_hw_node->DST0_FMT_CFG0.base.bitc.reg = ARIA_TDE_REG_DST0_FMT_CFG0;
    p_hw_node->DST0_FMT_CFG1.base.bitc.reg = ARIA_TDE_REG_DST0_FMT_CFG1;
    p_hw_node->DST0_PIC_ADDR.base.bitc.reg = ARIA_TDE_REG_DST0_PIC_ADDR;
    p_hw_node->DST0_PIC_STRIDE.base.bitc.reg = ARIA_TDE_REG_DST0_PIC_STRIDE;
    p_hw_node->DST0_OP_POS.base.bitc.reg = ARIA_TDE_REG_DST0_OP_POS;
    p_hw_node->DST1_FMT_CFG0.base.bitc.reg = ARIA_TDE_REG_DST1_FMT_CFG0;
    p_hw_node->DST1_FMT_CFG1.base.bitc.reg = ARIA_TDE_REG_DST1_FMT_CFG1;
    p_hw_node->DST1_PIC_ADDR.base.bitc.reg = ARIA_TDE_REG_DST1_PIC_ADDR;
    p_hw_node->DST1_PIC_STRIDE.base.bitc.reg = ARIA_TDE_REG_DST1_PIC_STRIDE;
    p_hw_node->DST1_OP_POS.base.bitc.reg = ARIA_TDE_REG_DST1_OP_POS;
    p_hw_node->DST_PIC_SIZE.base.bitc.reg = ARIA_TDE_REG_DST_PIC_SIZE;
    p_hw_node->DST_OP_SIZE.base.bitc.reg = ARIA_TDE_REG_DST_OP_SIZE;
    p_hw_node->XYLC_CFG.base.bitc.reg = ARIA_TDE_REG_XYLC_CFG;
    p_hw_node->SCALER_CFG.base.bitc.reg = ARIA_TDE_REG_SCALER_CFG;
    p_hw_node->SCALER_COEF_11.base.bitc.reg = ARIA_TDE_REG_SCALER_COEF_11;
    p_hw_node->SCALER_COEF_21.base.bitc.reg = ARIA_TDE_REG_SCALER_COEF_21;
    p_hw_node->SCALER_COEF_31.base.bitc.reg = ARIA_TDE_REG_SCALER_COEF_31;
    p_hw_node->SCALER_COEF_22.base.bitc.reg = ARIA_TDE_REG_SCALER_COEF_22;
    p_hw_node->SCALER_COEF_23.base.bitc.reg = ARIA_TDE_REG_SCALER_COEF_23;
    p_hw_node->SCALER_INIT_PHASE.base.bitc.reg = ARIA_TDE_REG_SCALER_INIT_PHASE;
    p_hw_node->ROT_PAT_CFG.base.bitc.reg = ARIA_TDE_REG_ROT_PAT_CFG;
    p_hw_node->PAT_COLOR.base.bitc.reg = ARIA_TDE_REG_PAT_COLOR;
    p_hw_node->PAT_OFFSET_POS.base.bitc.reg = ARIA_TDE_REG_PAT_OFFSET_POS;
    p_hw_node->PAT_RATIO_X_0.base.bitc.reg = ARIA_TDE_REG_PAT_RATIO_X_0;
    p_hw_node->PAT_RATIO_X_1.base.bitc.reg = ARIA_TDE_REG_PAT_RATIO_X_1;
    p_hw_node->PAT_RATIO_Y_0.base.bitc.reg = ARIA_TDE_REG_PAT_RATIO_Y_0;
    p_hw_node->PAT_RATIO_Y_1.base.bitc.reg = ARIA_TDE_REG_PAT_RATIO_Y_1;
    p_hw_node->GRADT_CFG.base.bitc.reg = ARIA_TDE_REG_GRADT_CFG;
    p_hw_node->GRADT_X_STEP.base.bitc.reg = ARIA_TDE_REG_GRADT_X_STEP;
    p_hw_node->GRADT_Y_STEP.base.bitc.reg = ARIA_TDE_REG_GRADT_Y_STEP;
    p_hw_node->GRADT_START_V.base.bitc.reg = ARIA_TDE_REG_GRADT_START_V;
    p_hw_node->STOP0_ARGB.base.bitc.reg = ARIA_TDE_REG_STOP0_ARGB;
    p_hw_node->STOP1_ARGB.base.bitc.reg = ARIA_TDE_REG_STOP1_ARGB;
    p_hw_node->STOP2_ARGB.base.bitc.reg = ARIA_TDE_REG_STOP2_ARGB;
    p_hw_node->STOP3_ARGB.base.bitc.reg = ARIA_TDE_REG_STOP3_ARGB;
    p_hw_node->STOP_OFFSET.base.bitc.reg = ARIA_TDE_REG_STOP_OFFSET;
    p_hw_node->STOP0_FACT.base.bitc.reg = ARIA_TDE_REG_STOP0_FACT;
    p_hw_node->STOP1_FACT.base.bitc.reg = ARIA_TDE_REG_STOP1_FACT;
    p_hw_node->STOP2_FACT.base.bitc.reg = ARIA_TDE_REG_STOP2_FACT;
    p_hw_node->COMP_CFG.base.bitc.reg = ARIA_TDE_REG_COMP_CFG;
    p_hw_node->COMP_MULT_MOD.base.bitc.reg = ARIA_TDE_REG_COMP_MULT_MOD;
    p_hw_node->COMP_BLD_MOD.base.bitc.reg = ARIA_TDE_REG_COMP_BLD_MOD;
    p_hw_node->ROP_ID.base.bitc.reg = ARIA_TDE_REG_ROP_ID;
    p_hw_node->ROP_PAT.base.bitc.reg = ARIA_TDE_REG_ROP_PAT;
    p_hw_node->LOAD_EN.base.bitc.reg = ARIA_TDE_REG_LOAD_EN;
    p_hw_node->PAL_SIZE.base.bitc.reg = ARIA_TDE_REG_PAL_SIZE;
    p_hw_node->PAL1_ADDR.base.bitc.reg = ARIA_TDE_REG_PAL1_ADDR;
    p_hw_node->PAL3_ADDR.base.bitc.reg = ARIA_TDE_REG_PAL3_ADDR;
    p_hw_node->COEF_ADDR.base.bitc.reg = ARIA_TDE_REG_COEF_ADDR;
    p_hw_node->SRC1_TILE_CFG.base.bitc.reg = ARIA_TDE_REG_SRC1_TILE_CFG;
    p_hw_node->SRC1_TILE_JMP00.base.bitc.reg = ARIA_TDE_REG_SRC1_TILE_JMP00;
    p_hw_node->SRC1_TILE_JMP01.base.bitc.reg = ARIA_TDE_REG_SRC1_TILE_JMP01;
    p_hw_node->SRC1_TILE_JMP10.base.bitc.reg = ARIA_TDE_REG_SRC1_TILE_JMP10;
    p_hw_node->SRC1_TILE_JMP11.base.bitc.reg = ARIA_TDE_REG_SRC1_TILE_JMP11;
    //p_hw_node->GRA_INT_EN.base.bitc.reg = ARIA_TDE_REG_GRA_INT_EN;
    //p_hw_node->GRA_INT_MOD.base.bitc.reg = ARIA_TDE_REG_GRA_INT_MOD;
#if 0
    p_hw_node->GRP0_CSCP_0.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCP_0;
    p_hw_node->GRP0_CSCP_1.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCP_1;
    p_hw_node->GRP0_CSCP_2.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCP_2;
    p_hw_node->GRP0_CSCP_3.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCP_3;
    p_hw_node->GRP0_CSCP_4.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCP_4;
    p_hw_node->GRP0_CSCDC_0.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCDC_0;
    p_hw_node->GRP0_CSCDC_1.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCDC_1;
    p_hw_node->GRP0_CSCDC_2.base.bitc.reg = ARIA_TDE_REG_GRP0_CSCDC_2;
    p_hw_node->GRP1_CSCP_0.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCP_0;
    p_hw_node->GRP1_CSCP_1.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCP_1;
    p_hw_node->GRP1_CSCP_2.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCP_2;
    p_hw_node->GRP1_CSCP_3.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCP_3;
    p_hw_node->GRP1_CSCP_4.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCP_4;
    p_hw_node->GRP1_CSCDC_0.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCDC_0;
    p_hw_node->GRP1_CSCDC_1.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCDC_1;
    p_hw_node->GRP1_CSCDC_2.base.bitc.reg = ARIA_TDE_REG_GRP1_CSCDC_2;
    p_hw_node->GRADT_RADIUS_A_0.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_A_0;
    p_hw_node->GRADT_RADIUS_A_1.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_A_1;
    p_hw_node->GRADT_RADIUS_B_0.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_B_0;
    p_hw_node->GRADT_RADIUS_B_1.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_B_1;
    p_hw_node->GRADT_RADIUS_C_0.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_C_0;
    p_hw_node->GRADT_RADIUS_C_1.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_C_1;
    p_hw_node->GRADT_RADIUS_D_0.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_D_0;
    p_hw_node->GRADT_RADIUS_D_1.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_D_1;
    p_hw_node->GRADT_RADIUS_E_0.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_E_0;
    p_hw_node->GRADT_RADIUS_E_1.base.bitc.reg = ARIA_TDE_REG_GRADT_RADIUS_E_1;
    p_hw_node->GRA_PIN_SEL.base.bitc.reg = ARIA_TDE_REG_GRA_PIN_SEL;
    p_hw_node->GRA_MAC_SET.base.bitc.reg = ARIA_TDE_REG_GRA_MAC_SET;
    p_hw_node->GRA_REQ_CFG.base.bitc.reg = ARIA_TDE_REG_GRA_REQ_CFG;
#endif

    p_hw_node->CMD_FIFO_ID0.base.bitc.reg = ARIA_TDE_REG_CMD_FIFO_ID0;
    p_hw_node->CMD_FIFO_ID1.base.bitc.reg = ARIA_TDE_REG_CMD_FIFO_ID1;
    p_hw_node->CMD_FIFO_ADDR_ASYNC.base.bitc.reg = ARIA_TDE_REG_CMD_FIFO_ADDR_ASYNC;

    TdeNodeSetData(p_hw_node, CMD_FIFO_ADDR_ASYNC, 0);
    TdeNodeSet_REG(p_hw_node, CMD_FIFO_ADDR_ASYNC, ARIA_TDE_REG_CMD_FIFO_ADDR_ASYNC);

    TdeNodeSet_CMD_END(p_hw_node, CMD_FIFO_ADDR_ASYNC, 1);
    TdeNodeSet_NEXT_NODE(p_hw_node, CMD_FIFO_ADDR_ASYNC, 0);
    TdeNodeSet_SUSBEND2(p_hw_node, CMD_FIFO_ADDR_ASYNC, 0);
    TdeNodeSet_USED(p_hw_node, CMD_FIFO_ADDR_ASYNC, 1);

    TDE_FUN_OUT;
    return;
}

mt_void TdeHalHwNodeSetNext(ARIA_TDE_HWNODE_t *p_hw_node, mt_u32 addr)
{
    TDE_FUN_IN;
    TDE_LOG("PARAM: [%x] [%x]\n", (mt_u32)p_hw_node, addr);

    if (addr != 0)
    {
        //p_hw_node->CMD_FIFO_ADDR_ASYNC.data = addr;
        TdeNodeSetData(p_hw_node, CMD_FIFO_ADDR_ASYNC, addr);
        TdeNodeSet_CMD_END(p_hw_node, CMD_FIFO_ADDR_ASYNC, 0);
        TdeNodeSet_NEXT_NODE(p_hw_node, CMD_FIFO_ADDR_ASYNC, 1);
        TdeNodeSet_SUSBEND2(p_hw_node, CMD_FIFO_ADDR_ASYNC, 0);
        TdeNodeSet_USED(p_hw_node, CMD_FIFO_ADDR_ASYNC, 1);

        if (1)
        {
            mt_u32 reg = 0;
            mt_u32 data = 0;
            reg = p_hw_node->CMD_FIFO_ADDR_ASYNC.base.all;
            data = p_hw_node->CMD_FIFO_ADDR_ASYNC.data;
            TDE_LOG("CMD_FIFO_END:  [%x] [%x]\n", reg, data);
        }
    }

    TDE_FUN_OUT;
    return;
}

mt_void TdeHalHwNodeSetID(ARIA_TDE_HWNODE_t *p_hw_node, mt_u32 addr, mt_u32 handle)
{
    TDE_FUN_IN;
    TDE_LOG("PARAM: [%x] [%x] [%x]\n", (mt_u32)p_hw_node, addr, handle);

    p_hw_node->CMD_FIFO_ID0.data = addr;
    p_hw_node->CMD_FIFO_ID1.data = handle;
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_CMD_FIFO_ID0, addr);
    TDE_REG_WRITE(ARIA_TDE_REG_CMD_FIFO_ID1, handle);
#endif

    TDE_FUN_OUT;
    return;
}

mt_void TdeHalHwNodePrint(ARIA_TDE_HWNODE_t *p_hw_node, MT_BOOL only_used) // only print the used
{
#ifdef _TDE_HAL_HW_NODE_PIRNT_ON_
    ARIA_TDE_CMD_t *p_cmd = NULL;
    mt_u32 cmd_cnt = 0;
    mt_u32 index = 0;
    ulong buf = 0;

    cmd_cnt = sizeof(ARIA_TDE_HWNODE_t) / sizeof(ARIA_TDE_CMD_t);
    p_cmd = (ARIA_TDE_CMD_t *)p_hw_node;
    buf = (ulong)p_cmd;

    TDE_FUN_IN;

    TDE_HAL_PRINT("HW_Node_Info: [%x][%d]\n", (ulong)p_hw_node, cmd_cnt);

    if (only_used)
    {
        for (index = 0; index < cmd_cnt; index++)
        {
            p_cmd = (ARIA_TDE_CMD_t *)(buf + sizeof(ARIA_TDE_CMD_t) * index);
            if (p_cmd->base.bitc.used)
            {
                TDE_HAL_PRINT("%x: %x\n", p_cmd->base.all, p_cmd->data);
            }
        }
    }
    else
    {
        for (index = 0; index < cmd_cnt; index++)
        {
            p_cmd = (ARIA_TDE_CMD_t *)(buf + sizeof(ARIA_TDE_CMD_t) * index);
            TDE_HAL_PRINT("%x: %x\n", p_cmd->base.all, p_cmd->data);
        }
    }

    TDE_FUN_OUT;

#endif //  _TDE_HAL_HW_NODE_PIRNT_ON_
    return;
}

static mt_void TdeHalAriaSysReset(MT_BOOL reset)
{
    mt_u32 val = 0;
    mt_u32 axi_status = 0;
    mt_u32 i = 0;
#if defined(CONFIG_MT_CHIP_ARIA)
    mt_u32 sys_ctrl_base = 0;
    mt_u32 sys_ctrl_reg = 0;
    mt_u32 ctrl_data = 0;
    mt_u32 cnt = 1000;

    // reset release: bit31(gra ahb soft reset), bit30(gra axi soft reset), bit29(gra soft reset)
    sys_ctrl_base = mt_get_sys_ctrl_base();
    if (sys_ctrl_base <= 0x1000)
    {
        MT_ASSERT(0);
    }

    sys_ctrl_reg = sys_ctrl_base + 0x18;

    // need lock
    if (reset)
    {
        ctrl_data = GET_REG(sys_ctrl_reg);
        ctrl_data |= 0xe0000000;
        PUT_REG(sys_ctrl_reg, ctrl_data);

        TDE_LOG("TdeHalAriaSysReset: 0x%x, %x\n", ctrl_data, g_tde_aria_vir_base);

        val = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_CTRL);
        val &= 0xFFFFFFFC;
        val |= 0x3;
        TDE_REG_WRITE(ARIA_TDE_REG_GRA_AXI_CTRL, val); //stop on
        axi_status = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_STATUS);
        if(TdeHalReadReg(0x300))    //GPE exist?
        {
            while(0x1 != (axi_status & 0x1))  //AXI_BUS_IDLE
            {
                axi_status = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_STATUS);
            }
        }

        ctrl_data &= ~0xe0000000;
        PUT_REG(sys_ctrl_reg, ctrl_data);
        while (cnt--)
        {
            if (cnt < 2)
            {
                break;
            }
        }
    }

    ctrl_data = GET_REG(sys_ctrl_reg);
    ctrl_data |= 0xe0000000;
    PUT_REG(sys_ctrl_reg, ctrl_data);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
        val = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_CTRL);
        val &= 0xFFFFFFFC;
        val |= 0x3;
        TDE_REG_WRITE(ARIA_TDE_REG_GRA_AXI_CTRL, val); //stop on
        axi_status = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_STATUS);
        if(TdeHalReadReg(0x300))    //GPE exist?
        {
            while(0x1 != (axi_status & 0x1))  //AXI_BUS_IDLE
            {
                axi_status = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_STATUS);
            }
        }
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	val = TDE_REG_READ(ARIA_TDE_REG_GRA_CLK_GATE);
       val |= 0xf;
	TDE_REG_WRITE(ARIA_TDE_REG_GRA_CLK_GATE, val); //clk always on
#endif

    val = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_CTRL);
    val &= 0xFFFFFFFC;
    val |= 0x3;
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_AXI_CTRL, val); //stop on
    axi_status = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_STATUS);

    while(0x1 != (axi_status & 0x1))  //AXI_BUS_IDLE
    {
        msleep(1);
        axi_status = TDE_REG_READ(ARIA_TDE_REG_GRA_AXI_STATUS);
        i++;
        if(i> 10)
        {
          printk(KERN_EMERG"Warning %s %d gpe reset timeout !\n", __func__, __LINE__);
          break;
        }
    }

    HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base()+ (0xa10c)), 0);
    mdelay(1);
    HAL_PUT_U32((volatile MT_U32 *)(mt_get_crm_base() + (0xa10c)), 0x7);
   val = TDE_REG_READ(ARIA_TDE_REG_GRA_CLK_GATE);
   val &= ~(0xf);
   TDE_REG_WRITE(ARIA_TDE_REG_GRA_CLK_GATE, val); //clk gated
#endif
    TDE_LOG("TdeHalAriaSysReset: \n");

    //unlock
    return;
}

//=========================================//
//
//
//        reg config
//
//
//=========================================//
mt_u32 TdeHalGetRegVriBaseAddr(mt_void)
{
    return g_tde_aria_vir_base;
}

mt_s32 TdeHalInit(mt_u32 u32BaseAddr)
{
    TDE_FUN_IN;
    /*init the pool memery of tde*/ /*CNcomment:初始化TDE内存池*/
    if (MT_SUCCESS != wmeminit())
    {
        TDE_LINE;
        goto TDE_INIT_ERR;
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    g_tde_aria_vir_base = SYMPHONY_TDE_BASE;
#else
    /*map address for the register */ /*CNcomment:寄存器映射*/
    g_tde_aria_vir_base = (ulong)ioremap(u32BaseAddr, TDE_REG_SIZE);
#endif
    //s_pu32BaseVirAddr = aria_get_tde_base();
    TDE_LOG("tde vir base addr = 0x%lx, 0x%x, 0x%x\n", g_tde_aria_vir_base, u32BaseAddr, TDE_REG_SIZE);

    if (MT_NULL == g_tde_aria_vir_base)
    {
        TDE_LINE;
        goto TDE_INIT_ERR;
    }

    //TdeHalSetClock(MT_TRUE);

    TdeHalCtlReset();

    TdeHalAriaSysReset(MT_TRUE);

//    TdeHalInitIrqMask();

    //TdeHalInitQueue(); /* init SQ/AQ *//*CNcomment: 初始化SQ/AQ */

    //load the coeff table
    /**
     **分配coeff 内存
     **/
    {
	mt_char name_coeff[32];
	snprintf(name_coeff, sizeof(name_coeff), "TDE_Coeff");
	/**
         ** apply coeff buffer
         **/
	if (mt_drv_mmz_alloc_and_map(name_coeff, MT_NULL, sizeof(tde_aria_scale_coeff),
	                           0, &stCoeffTableBuf) != MT_SUCCESS) {
	    TDE_LOG("TDE Get coeff buffer failed!\n");
	    return MT_FAILURE;
	}

    memcpy((void *)stCoeffTableBuf.startVirAddr, tde_aria_scale_coeff, sizeof(tde_aria_scale_coeff));
    }

    TDE_FUN_OUT;
    return MT_SUCCESS;

TDE_INIT_ERR:
    return MT_FAILURE;
}

mt_s32 TdeHalOpen(mt_void)
{
    /* do sth ... */
    TDE_FUN_IN;

    TDE_FUN_OUT;

    return MT_SUCCESS;
}

mt_void TdeHalRelease(mt_void)
{
    //mt_void *pBuf;

    TDE_FUN_IN;

    /* unmap the base address*/ /*CNcomment:  反映射基地址 */
#if (!defined(CONFIG_MT_CHIP_SYMPHONY4)) && (!defined(CONFIG_MT_CHIP_SYMPHONY6))
    iounmap((mt_void *)g_tde_aria_vir_base);
#endif
    g_tde_aria_vir_base = 0x0;

    /*free the pool of memery*/ /*CNcomment:TDE内存池去初始化*/
    wmemterm();

    /*free the coeff buffer*/
    mt_drv_mmz_unmap_and_release(&stCoeffTableBuf);

    TDE_FUN_OUT;
    return;
}

MT_BOOL TdeHalCtlIsIdle(mt_void)
{
    MT_BOOL ret = MT_FALSE;
    mt_u32 ctrl_status = 0;

    ctrl_status = TDE_REG_READ(ARIA_TDE_REG_CTRL_STATUS);
    ret = ctrl_status ? MT_FALSE : MT_TRUE;

    return ret;
}

MT_BOOL TdeHalWorkFinished(mt_void)
{
    MT_BOOL ret = MT_FALSE;
    mt_u32 gra_status = 0;

    gra_status = TDE_REG_READ(ARIA_TDE_REG_GRA_STATE);
    ret = (gra_status >> 31) ? MT_TRUE : MT_FALSE;

    return ret;
}

#define TDE_MAX_READ_STATUS_TIME (100)

#if 1
MT_BOOL TdeHalCtlIsIdleSafely(mt_void)
{
    mt_u32 i = 0;
    MT_BOOL is_idle = MT_FALSE;

    TDE_FUN_IN;

    /*get the state of tde one more time ,make sure it's idle *//*CNcomment: 连续读取多次硬件状态,确保TDE完成 */
    for (i = 0; i < TDE_MAX_READ_STATUS_TIME; i++)
    {
        if (TdeHalCtlIsIdle())
        {
            is_idle = MT_TRUE;
            break;
        }
        else
        {
            continue;
        }
    }
    TDE_FUN_OUT;
    return is_idle;
}
#endif

//by Jack
#if 0
MT_BOOL TdeHalCtlIsIdleSafely(mt_void)
{
    mt_u32 i = 0;
    MT_BOOL is_idle = MT_FALSE;

    TDE_FUN_IN;

    for (i = 0; i < TDE_MAX_READ_STATUS_TIME; i++)
    {
        if (TdeHalWorkFinished())
        {
            aria_tde_gra_state_clear_all_done();
        }

        if (TdeHalCtlIsIdle())
        {
            is_idle = MT_TRUE;
            break;
        }
        else
        {
            continue;
        }
    }

    if (is_idle)
    {
        TDE_FUN_OUT;
        return MT_TRUE;
    }

    //else ,  need sys reset
    TdeHalAriaSysReset(MT_TRUE);

    TdeHalInitIrqMask();

    for (i = 0; i < TDE_MAX_READ_STATUS_TIME; i++)
    {
        if (TdeHalWorkFinished())
        {
            aria_tde_gra_state_clear_all_done();
        }

        if (TdeHalCtlIsIdle())
        {
            is_idle = MT_TRUE;
            break;
        }
        else
        {
            continue;
        }
    }

    TDE_FUN_OUT;
    return is_idle;
}
#endif

mt_u32 TdeHalCtlIntStats(mt_void)
{
    mt_u32 Value;
    TDE_FUN_IN;

    Value = TDE_REG_READ(ARIA_TDE_REG_GRA_INT_STATE);

    /* clear all status */
    //TDE_REG_WRITE(ARIA_TDE_REG_GRA_INT_STATE, (Value)/*0x800f001f*/);

    TDE_FUN_OUT;
    return Value;
}

mt_void TdeHalClearInt(mt_u32 int_value)
{
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_INT_STATE, (int_value));
}

mt_void TdeHalCtlReset(mt_void)
{
    TDE_FUN_IN;

    /*reset */ /*CNcomment: 复位 */
               //    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_RST, 0xffffffff);

    /* clear the state of interrupt*/ /*CNcomment: 请中断状态 */
                                      //    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_INT, 0x800f001f);

    /* write TDE_REQ_TH*/ /*CNcomment: 写申请门限控制寄存器*/
                          //    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_REQ_TH, 0x01fef7cf);

    /* write TDE_BUS_LIMITER*/ /*CNcomment: 写总线流量控制寄存器*/
                               //    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_BUS_LIMITER, 0x80770000);

    TDE_FUN_OUT;
    return;
}

mt_void TdeHalSetClock(MT_BOOL bEnable)
{
// in aria, no this functions
#if 0
#ifdef CONFIG_TDE_USE_SDK_CRG_ENABLE
    U_PERI_CRG37 unTempValue;

    unTempValue.u32 = g_pstRegCrg->PERI_CRG37.u32;
    TDE_FUN_IN;
    if (bEnable)
    {
        /*cancel reset*/
        unTempValue.bits.tde_srst_req = 0x0;

        /*enable clock*/
        unTempValue.bits.tde_cken = 0x1;
    }
    else
    {
        /*disable clock*/
        unTempValue.bits.tde_cken = 0x0;
    }

    g_pstRegCrg->PERI_CRG37.u32 = unTempValue.u32;
#else
    if (bEnable)
    {
        /*cancel reset*/
        *s_pu32TdeClockVir &= ~0x1;

        /*enable clock*/
        *s_pu32TdeClockVir |= 0x2;
    }
    else
    {
        /*disable clock*/
        *s_pu32TdeClockVir &= ~0x2;

        /* reset*/
        *s_pu32TdeClockVir |= 0x1;
    }
#endif

    TDE_FUN_OUT;
#endif
    return;
}

#if 0
static mt_void TdeHalInitQueue(mt_void)
{
    //TDE_AQ_CTRL_U unAqCtrl;
    TDE_FUN_IN;
    /*write 0 to Aq list start address register*/
    /*CNcomment: 将Aq链表首地址寄存器写0 */
//    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_AQ_NADDR, 0);

//    unAqCtrl.u32All = TDE_REG_READ(s_pu32BaseVirAddr, TDE_AQ_CTRL);


    /*enable Aq list*//*CNcomment: 使能Aq链表 */
    //unAqCtrl.stBits.u32AqEn = 1;/*hardware disable*//*CNcomment: 硬件暂时屏蔽*/

    /*set Aq operation mode*//*CNcomment:  配置Aq操作模式 */
    //unAqCtrl.stBits.u32AqOperMode = TDE_AQ_CTRL_COMP_LINE;
//    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_AQ_CTRL, unAqCtrl.u32All);
//    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_AXI_ID, 0x1010);

    TDE_FUN_OUT;
}
#endif

mt_void TdeHalCtlIntClear(mt_u32 u32Stats)
{
    //mt_u32 u32ReadStats = 0;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x]\n", u32Stats);

    //    u32ReadStats = TDE_REG_READ(s_pu32BaseVirAddr, TDE_INT);
    //u32ReadStats = (u32ReadStats & 0x0000ffff) | ((u32Stats << 16) & 0xffff0000);

    //    TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_INT, u32ReadStats);

    TDE_FUN_OUT;
}

//=========================================//
//
//
//        Node concfig
//
//
//=========================================//

mt_s32 TdeHalNodeInitNd(TDE_HWNode_S **pstHWNode)
{
    mt_void *pBuf = NULL;

    TDE_FUN_IN;

    pBuf = (mt_void *)TDE_MALLOC(sizeof(TDE_HWNode_S) + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE);
    if (MT_NULL == pBuf)
    {
        TDE_TRACE(TDE_KERN_INFO, "malloc (%d) failed, wgetfreenum(%d)!\n", (sizeof(TDE_HWNode_S) + TDE_NODE_HEAD_BYTE + TDE_NODE_TAIL_BYTE), wgetfreenum());
        TDE_FUN_OUT;
        return MT_ERR_TDE_NO_MEM;
    }

    *pstHWNode = (TDE_HWNode_S *)(pBuf + TDE_NODE_HEAD_BYTE);

    {
        //mt_u32 hw_addr = (mt_u32)(pBuf + TDE_NODE_HEAD_BYTE);
        //mt_u32 size = sizeof(TDE_HWNode_S);
        //TDE_LOG("TdeHalNodeInitNd: [%x] [%x]\n", hw_addr, size);
    }

    TdeHalHwNodeInitAriaNode(*pstHWNode);

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_void TdeHalFreeNodeBuf(TDE_HWNode_S *pstHWNode)
{
    mt_void *pBuf = NULL;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x]\n", (mt_u32)pstHWNode);

    pBuf = (mt_void *)pstHWNode - TDE_NODE_HEAD_BYTE;
    TDE_FREE(pBuf);

    TDE_FUN_OUT;
}

mt_void TdeHalNodeInitChildNd(TDE_HWNode_S *pHWNode, mt_u32 u32TDE_CLIP_START, mt_u32 u32TDE_CLIP_STOP)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x]\n", u32TDE_CLIP_START, u32TDE_CLIP_STOP);

    // TODO::

    TDE_FUN_OUT;

    return;
}

mt_void TdeHalNodeEnableCompleteInt(mt_void *pBuf)
{
    TDE_FUN_IN;

    // TODO::

    TDE_FUN_OUT;

    return;
}

typedef enum {
    COMP_BP_RESUTL,
    COMP_BP_SRC1,
    COMP_BP_SRC2,
    COMP_BP_SRC3,
    COMP_BP_PATTERN,
} COMP_BP_E;

static mt_void TdeAriaSetCompCfg(TDE_HWNode_S *pHWNode, COMP_BP_E bp)
{
    REG_COMP_CFG comp_cfg;

    comp_cfg.all = TdeNodeGetData(pHWNode, COMP_CFG);
    if (bp == COMP_BP_RESUTL) {
  comp_cfg.bitc.comp_bp = 0x0;
    } else if (bp == COMP_BP_SRC1) {
  comp_cfg.bitc.comp_src1_en = 0x1; // comp src1 en
  comp_cfg.bitc.comp_bp = 0x1;      //src1 bypass
    } else if (bp == COMP_BP_SRC2) {
  comp_cfg.bitc.comp_bp = 0x2;
    } else if (bp == COMP_BP_SRC3) {
  comp_cfg.bitc.comp_bp = 0x3;
    } else if (bp == COMP_BP_PATTERN) {
  comp_cfg.bitc.comp_bp = 0x8;
    }

    TdeNodeSetData(pHWNode, COMP_CFG, comp_cfg.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_CFG, comp_cfg.all);
#endif
    return;
}

mt_void TdeAriaSetPatternData(TDE_HWNode_S *pHWNode, mt_u32 u32Colorize)
{
    REG_ROP_PAT rop_pat; // pattern color

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x]\n", u32Colorize);

    rop_pat.all = TdeNodeGetData(pHWNode, ROP_PAT);
    rop_pat.bitc.rop_pat = u32Colorize;
    TdeNodeSetData(pHWNode, ROP_PAT, rop_pat.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_ROP_PAT, rop_pat.all);
#endif

    TDE_FUN_OUT;
    return;
}

static mt_void TdeAriaSetSrc0(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface, MB_INFO_TYPE_E mt_type)
{
    REG_SRC0_FMT_CFG0 src0_cfg0;
    REG_SRC0_FMT_CFG1 src0_cfg1;
    REG_SRC0_PIC_ADDR src0_pic_addr;
    REG_SRC0_PIC_STRIDE src0_pic_stride;

    ARIA_MB_INFO_S mb_info;

    TDE_FUN_IN;

    if (!tde_hal_get_MB_info(pDrvSurface, mt_type, &mb_info))
    {
        MT_ASSERT(0);
    }

    src0_cfg0.all = TdeNodeGetData(pHWNode, SRC0_FMT_CFG0);
    src0_cfg1.all = TdeNodeGetData(pHWNode, SRC0_FMT_CFG1);
    src0_pic_addr.all = TdeNodeGetData(pHWNode, SRC0_PIC_ADDR);
    src0_pic_stride.all = TdeNodeGetData(pHWNode, SRC0_PIC_STRIDE);

    // set reg
    src0_cfg0.bitc.src0_en = 0x1;
    src0_cfg0.bitc.src0_bpp_mod = mb_info.bpp;
    src0_cfg0.bitc.src0_swap_mod = mb_info.swap_mod;
    TdeNodeSetData(pHWNode, SRC0_FMT_CFG0, src0_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC0_FMT_CFG0, src0_cfg0.all);
#endif

    src0_cfg1.bitc.src0_pic_fmt = mb_info.fmt;
    TdeNodeSetData(pHWNode, SRC0_FMT_CFG1, src0_cfg1.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC0_FMT_CFG1, src0_cfg1.all);
#endif

    src0_pic_addr.bitc.src0_pic_addr = mb_info.pic_addr;
    TdeNodeSetData(pHWNode, SRC0_PIC_ADDR, src0_pic_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC0_PIC_ADDR, src0_pic_addr.all);
#endif

    src0_pic_stride.bitc.src0_pic_stride = mb_info.pic_stride;
    TdeNodeSetData(pHWNode, SRC0_PIC_STRIDE, src0_pic_stride.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC0_PIC_STRIDE, src0_pic_stride.all);
#endif

    TDE_FUN_OUT;

    return;
}

static void TdeAriaSetSrcMB_Y(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    REG_SRC1_FMT_CFG0 src1_cfg0;
    REG_SRC1_FMT_CFG1 src1_cfg1;
    REG_SRC1_PIC_ADDR pic_addr;
    REG_SRC1_PIC_STRIDE pic_stride;
    REG_SRC1_PIC_SIZE pic_size;
    REG_SRC1_OP_SIZE op_size;
    REG_SRC1_OP_POS op_pos;

    ARIA_MB_INFO_S mb_info = { 0 };

    TDE_FUN_IN;
    TdeOsiPrintDrvSurface(pDrvSurface);

    if (!tde_hal_get_MB_info(pDrvSurface, MB_TYPE_Y, &mb_info))
    {
        MT_ASSERT(0);
    }

    src1_cfg0.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG0);
    src1_cfg1.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG1);
    pic_addr.all = TdeNodeGetData(pHWNode, SRC1_PIC_ADDR);
    pic_stride.all = TdeNodeGetData(pHWNode, SRC1_PIC_STRIDE);
    pic_size.all = TdeNodeGetData(pHWNode, SRC1_PIC_SIZE);
    op_size.all = TdeNodeGetData(pHWNode, SRC1_OP_SIZE);
    op_pos.all = TdeNodeGetData(pHWNode, SRC1_OP_POS);

    src1_cfg0.bitc.src1_en = 0x1;
    src1_cfg0.bitc.src1_clut_en = 0x0;
    src1_cfg0.bitc.src1_alp_en = 0x0;
    src1_cfg0.bitc.src1_alp_en = 0x0;
    src1_cfg0.bitc.src1_bpp_mod = mb_info.bpp;
    src1_cfg0.bitc.src1_swap_mod = mb_info.swap_mod;

    TdeNodeSetData(pHWNode, SRC1_FMT_CFG0, src1_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG0, src1_cfg0.all);
#endif

    src1_cfg1.bitc.src1_pic_fmt = mb_info.fmt;
    TdeNodeSetData(pHWNode, SRC1_FMT_CFG1, src1_cfg1.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG1, src1_cfg1.all);
#endif

    pic_addr.bitc.src1_pic_addr = mb_info.pic_addr;
    pic_stride.bitc.src1_pic_stride = mb_info.pic_stride;

    TdeNodeSetData(pHWNode, SRC1_PIC_ADDR, pic_addr.all);
    TdeNodeSetData(pHWNode, SRC1_PIC_STRIDE, pic_stride.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_PIC_ADDR, pic_addr.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_PIC_STRIDE, pic_stride.all);
#endif

    pic_size.bitc.src1_pic_w = pDrvSurface->u32Width;
    pic_size.bitc.src1_pic_h = pDrvSurface->u32Height;
    TdeNodeSetData(pHWNode, SRC1_PIC_SIZE, pic_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_PIC_SIZE, pic_size.all);
#endif

    op_size.bitc.src1_op_w = pDrvSurface->u32Width;
    op_size.bitc.src1_op_h = pDrvSurface->u32Height;
    TdeNodeSetData(pHWNode, SRC1_OP_SIZE, op_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_OP_SIZE, op_size.all);
#endif

    op_pos.bitc.src1_op_x = pDrvSurface->u32Xpos;
    op_pos.bitc.src1_op_y = pDrvSurface->u32Ypos;
    TdeNodeSetData(pHWNode, SRC1_OP_POS, op_pos.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_OP_POS, op_pos.all);
#endif

    TDE_FUN_OUT;
    return;
}

static void TdeAriaSetSrc1(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    REG_SRC1_FMT_CFG0 src1_cfg0;
    REG_SRC1_FMT_CFG1 src1_cfg1;
    REG_SRC1_PIC_ADDR pic_addr;
    REG_SRC1_PIC_STRIDE pic_stride;
    REG_SRC1_PIC_SIZE pic_size;
    REG_SRC1_OP_SIZE op_size;
    REG_SRC1_OP_POS op_pos;

    gpe_img_t img;
    mt_u8 swap_data = 0;

    TDE_FUN_IN;
    TdeOsiPrintDrvSurface(pDrvSurface);

    if (!tde_hal_convert_surface_to_img(pDrvSurface, &img, SPN_SRC1))
    {
        TDE_FUN_OUT;
        MT_ASSERT(0);
        return;
    }

    swap_data = tde_hal_get_img_swap(&img);

    src1_cfg0.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG0);
    src1_cfg1.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG1);
    pic_addr.all = TdeNodeGetData(pHWNode, SRC1_PIC_ADDR);
    pic_stride.all = TdeNodeGetData(pHWNode, SRC1_PIC_STRIDE);
    pic_size.all = TdeNodeGetData(pHWNode, SRC1_PIC_SIZE);
    op_size.all = TdeNodeGetData(pHWNode, SRC1_OP_SIZE);
    op_pos.all = TdeNodeGetData(pHWNode, SRC1_OP_POS);

    src1_cfg0.bitc.src1_en = 0x1;
    //src1_cfg0.bitc.src1_clut_en = img.with_palette;
    //src1_cfg0.bitc.src1_alp_en = img.color_info.alpha_ch_en;
    src1_cfg0.bitc.src1_alp_en = img.color_info.is_pix_alpha;
    src1_cfg0.bitc.src1_bpp_mod = img.color_info.bpp;
    src1_cfg0.bitc.src1_swap_mod = swap_data;

    TdeNodeSetData(pHWNode, SRC1_FMT_CFG0, src1_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG0, src1_cfg0.all);
#endif

    src1_cfg1.bitc.src1_pic_fmt = img.color_info.color_fmt;
    TdeNodeSetData(pHWNode, SRC1_FMT_CFG1, src1_cfg1.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG1, src1_cfg1.all);
#endif

    if (pDrvSurface->enVScan == TDE_SCAN_DOWN_UP)
    {
        // stride is negative
        pic_addr.bitc.src1_pic_addr = pDrvSurface->u32PhyAddr + pDrvSurface->u32Pitch * (pDrvSurface->u32Height - 1);
        pic_stride.bitc.src1_pic_stride = pDrvSurface->u32Pitch;
        pic_stride.bitc.src1_pic_stride |= (1 << 16);
    }
    else
    {
        pic_addr.bitc.src1_pic_addr = pDrvSurface->u32PhyAddr;
        pic_stride.bitc.src1_pic_stride = pDrvSurface->u32Pitch;
    }
    TdeNodeSetData(pHWNode, SRC1_PIC_ADDR, pic_addr.all);
    TdeNodeSetData(pHWNode, SRC1_PIC_STRIDE, pic_stride.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_PIC_ADDR, pic_addr.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_PIC_STRIDE, pic_stride.all);
#endif

    pic_size.bitc.src1_pic_w = pDrvSurface->u32Width;
    pic_size.bitc.src1_pic_h = pDrvSurface->u32Height;
    TdeNodeSetData(pHWNode, SRC1_PIC_SIZE, pic_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_PIC_SIZE, pic_size.all);
#endif

    op_size.bitc.src1_op_w = pDrvSurface->u32Width;
    op_size.bitc.src1_op_h = pDrvSurface->u32Height;
    TdeNodeSetData(pHWNode, SRC1_OP_SIZE, op_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_OP_SIZE, op_size.all);
#endif

    op_pos.bitc.src1_op_x = pDrvSurface->u32Xpos;
    op_pos.bitc.src1_op_y = pDrvSurface->u32Ypos;
    TdeNodeSetData(pHWNode, SRC1_OP_POS, op_pos.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_OP_POS, op_pos.all);
#endif

    TDE_FUN_OUT;
    return;
}

static void TdeAriaSetSrc2(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    REG_SRC2_FMT_CFG0 src2_cfg0;
    REG_SRC2_FMT_CFG1 src2_cfg1;
    REG_SRC2_PIC_ADDR pic_addr;
    REG_SRC2_PIC_STRIDE pic_stride;
    REG_SRC2_OP_POS op_pos;
    gpe_img_t img;

    TDE_FUN_IN;
    TdeOsiPrintDrvSurface(pDrvSurface);

    if (!tde_hal_convert_surface_to_img(pDrvSurface, &img, SPN_SRC2))
    {
        TDE_FUN_OUT;
        MT_ASSERT(0);
        return;
    }

    src2_cfg0.all = TdeNodeGetData(pHWNode, SRC2_FMT_CFG0);
    src2_cfg1.all = TdeNodeGetData(pHWNode, SRC2_FMT_CFG1);
    pic_addr.all = TdeNodeGetData(pHWNode, SRC2_PIC_ADDR);
    pic_stride.all = TdeNodeGetData(pHWNode, SRC2_PIC_STRIDE);
    op_pos.all = TdeNodeGetData(pHWNode, SRC2_OP_POS);

    src2_cfg0.bitc.src2_en = 0x1;
    //src2_cfg0.bitc.src2_clut_en = img.with_palette;
    src2_cfg0.bitc.src2_alp_en = img.color_info.alpha_ch_en;
    src2_cfg0.bitc.src2_bpp_mod = img.color_info.bpp;
    TdeNodeSetData(pHWNode, SRC2_FMT_CFG0, src2_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC2_FMT_CFG0, src2_cfg0.all);
#endif

    src2_cfg1.bitc.src2_pic_fmt = img.color_info.color_fmt;
    TdeNodeSetData(pHWNode, SRC2_FMT_CFG1, src2_cfg1.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC2_FMT_CFG1, src2_cfg1.all);
#endif

    pic_addr.bitc.src2_pic_addr = pDrvSurface->u32PhyAddr;
    TdeNodeSetData(pHWNode, SRC2_PIC_ADDR, pic_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC2_PIC_ADDR, pic_addr.all);
#endif

    pic_stride.bitc.src2_pic_stride = pDrvSurface->u32Pitch;
    TdeNodeSetData(pHWNode, SRC2_PIC_STRIDE, pic_stride.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC2_PIC_STRIDE, pic_stride.all);
#endif

    op_pos.bitc.src2_op_x = pDrvSurface->u32Xpos;
    op_pos.bitc.src2_op_y = pDrvSurface->u32Ypos;
    TdeNodeSetData(pHWNode, SRC2_OP_POS, op_pos.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC2_OP_POS, op_pos.all);
#endif

    TDE_FUN_OUT;
    return;
}

static void TdeAriaSetSrc3(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    REG_SRC3_FMT_CFG0 src3_cfg0;
    REG_SRC3_FMT_CFG1 src3_cfg1;
    REG_SRC3_PIC_ADDR pic_addr;
    REG_SRC3_PIC_STRIDE pic_stride;
    REG_SRC3_OP_POS op_pos;
    gpe_img_t img;
    mt_u8 swap_data = 0;

    TDE_FUN_IN;
    TdeOsiPrintDrvSurface(pDrvSurface);

    if (!tde_hal_convert_surface_to_img(pDrvSurface, &img, SPN_SRC3))
    {
        TDE_FUN_OUT;
        MT_ASSERT(0);
        return;
    }

    swap_data = tde_hal_get_img_swap(&img);

    src3_cfg0.all = TdeNodeGetData(pHWNode, SRC3_FMT_CFG0);
    src3_cfg1.all = TdeNodeGetData(pHWNode, SRC3_FMT_CFG1);
    pic_addr.all = TdeNodeGetData(pHWNode, SRC3_PIC_ADDR);
    pic_stride.all = TdeNodeGetData(pHWNode, SRC3_PIC_STRIDE);
    op_pos.all = TdeNodeGetData(pHWNode, SRC3_OP_POS);

    src3_cfg0.bitc.src3_en = 0x1;
    src3_cfg0.bitc.src3_clut_en = img.with_palette;
    src3_cfg0.bitc.src3_alp_en = img.color_info.alpha_ch_en;
    src3_cfg0.bitc.src3_bpp_mod = img.color_info.bpp;
    src3_cfg0.bitc.src3_swap_mod = swap_data;
    TdeNodeSetData(pHWNode, SRC3_FMT_CFG0, src3_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC3_FMT_CFG0, src3_cfg0.all);
#endif

    src3_cfg1.bitc.src3_pic_fmt = img.color_info.color_fmt;
    TdeNodeSetData(pHWNode, SRC3_FMT_CFG1, src3_cfg1.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC3_FMT_CFG1, src3_cfg1.all);
#endif

    pic_addr.bitc.src3_pic_addr = pDrvSurface->u32PhyAddr;
    TdeNodeSetData(pHWNode, SRC3_PIC_ADDR, pic_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC3_PIC_ADDR, pic_addr.all);
#endif

    pic_stride.bitc.src3_pic_stride = pDrvSurface->u32Pitch;
    TdeNodeSetData(pHWNode, SRC3_PIC_STRIDE, pic_stride.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC3_PIC_STRIDE, pic_stride.all);
#endif

    op_pos.bitc.src3_op_x = pDrvSurface->u32Xpos;
    op_pos.bitc.src3_op_y = pDrvSurface->u32Ypos;
    TdeNodeSetData(pHWNode, SRC3_OP_POS, op_pos.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC3_OP_POS, op_pos.all);
#endif

    TDE_FUN_OUT;
    return;
}

static mt_void TdeHalNodeSetSrcCfg(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface, src_ch_t ch)
{
    if (ch == SPN_SRC1)
    {
        TdeAriaSetSrc1(pHWNode, pDrvSurface);
    }
    else if (ch == SPN_SRC2)
    {
        TdeAriaSetSrc2(pHWNode, pDrvSurface);
    }
    else if (ch == SPN_SRC3)
    {
        TdeAriaSetSrc3(pHWNode, pDrvSurface);
    }
    else
    {
        MT_ASSERT(0);
    }

    return;
}

mt_void TdeHalNodeSetSrc1(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    TdeHalNodeSetSrcCfg(pHWNode, pDrvSurface, SPN_SRC1);
}

mt_void TdeHalNodeSetSrc2(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    TdeHalNodeSetSrcCfg(pHWNode, pDrvSurface, SPN_SRC2);
}

mt_void TdeHalNodeSetSrc3(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface)
{
    TdeHalNodeSetSrcCfg(pHWNode, pDrvSurface, SPN_SRC3);
}

mt_void TdeHalNodeSetSrcMbY(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvMbY, TDE_DRV_MB_OPT_MODE_E enMbOpt)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%x][%x][%x][%x][%x][%x][%x][%x] [%x]\n", pDrvMbY->enColorFmt, pDrvMbY->enRgbOrder, pDrvMbY->u32Xpos, pDrvMbY->u32Ypos, pDrvMbY->u32Width, pDrvMbY->u32Height, pDrvMbY->u32Pitch, pDrvMbY->u32PhyAddr, pDrvMbY->u32CbCrPitch, pDrvMbY->u32CbCrPhyAddr, enMbOpt);

    TdeAriaSetSrcMB_Y(pHWNode, pDrvMbY);

    TDE_FUN_OUT;

    return;
}

mt_void TdeHalNodeSetSrcMbCbCr(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvMbCbCr, TDE_DRV_MB_OPT_MODE_E enMbOpt)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%x][%x][%x][%x][%x][%x][%x][%x] [%x]\n", pDrvMbCbCr->enColorFmt, pDrvMbCbCr->enRgbOrder, pDrvMbCbCr->u32Xpos, pDrvMbCbCr->u32Ypos, pDrvMbCbCr->u32Width, pDrvMbCbCr->u32Height, pDrvMbCbCr->u32Pitch, pDrvMbCbCr->u32PhyAddr, pDrvMbCbCr->u32CbCrPitch, pDrvMbCbCr->u32CbCrPhyAddr, enMbOpt);

    TdeAriaSetSrc0(pHWNode, pDrvMbCbCr, MT_TYPE_CbCr);

    TDE_FUN_OUT;

    return;
}

mt_void TdeHalNodeSetTgt(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *pDrvSurface, TDE_DRV_OUTALPHA_FROM_E enAlphaFrom)
{
    REG_DST0_FMT_CFG0 dts0_cfg0;
    REG_DST0_FMT_CFG1 dts0_cfg1;
    REG_DST0_PIC_ADDR pic_addr;
    REG_DST0_PIC_STRIDE pic_stride;
    REG_DST0_OP_POS op_pos;
    REG_DST_PIC_SIZE pic_size;
    REG_DST_OP_SIZE op_size;
    gpe_img_t img;

    TDE_FUN_IN;
    MT_ASSERT(pDrvSurface != NULL);

    TdeOsiPrintDrvSurface(pDrvSurface);
    TDE_LOG("PARA: [%x]\n", enAlphaFrom);

    if (!tde_hal_convert_surface_to_img(pDrvSurface, &img, SPN_DST))
    {
        TDE_FUN_OUT;
        MT_ASSERT(0);
        return;
    }

    dts0_cfg0.all = TdeNodeGetData(pHWNode, DST0_FMT_CFG0);
    dts0_cfg1.all = TdeNodeGetData(pHWNode, DST0_FMT_CFG1);
    pic_addr.all = TdeNodeGetData(pHWNode, DST0_PIC_ADDR);
    pic_stride.all = TdeNodeGetData(pHWNode, DST0_PIC_STRIDE);
    op_pos.all = TdeNodeGetData(pHWNode, DST0_OP_POS);
    pic_size.all = TdeNodeGetData(pHWNode, DST_PIC_SIZE);
    op_size.all = TdeNodeGetData(pHWNode, DST_OP_SIZE);

    dts0_cfg0.bitc.dst0_en = 0x1;
    dts0_cfg0.bitc.dst0_bpp_mod = img.color_info.bpp;
    TdeNodeSetData(pHWNode, DST0_FMT_CFG0, dts0_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST0_FMT_CFG0, dts0_cfg0.all);
#endif

    dts0_cfg1.bitc.dst0_pic_fmt = img.color_info.color_fmt;
    TdeNodeSetData(pHWNode, DST0_FMT_CFG1, dts0_cfg1.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST0_FMT_CFG1, dts0_cfg1.all);
#endif

    pic_addr.bitc.dst0_pic_addr = pDrvSurface->u32PhyAddr;
    TdeNodeSetData(pHWNode, DST0_PIC_ADDR, pic_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST0_PIC_ADDR, pic_addr.all);
#endif

    pic_stride.bitc.dst0_pic_stride = pDrvSurface->u32Pitch;
    TdeNodeSetData(pHWNode, DST0_PIC_STRIDE, pic_stride.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST0_PIC_STRIDE, pic_stride.all);
#endif

    pic_size.bitc.dst_pic_w = pDrvSurface->u32Width;
    pic_size.bitc.dst_pic_h = pDrvSurface->u32Height;
    TdeNodeSetData(pHWNode, DST_PIC_SIZE, pic_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST_PIC_SIZE, pic_size.all);
#endif

    op_pos.bitc.dst0_op_x = pDrvSurface->u32Xpos;
    op_pos.bitc.dst0_op_y = pDrvSurface->u32Ypos;
    TdeNodeSetData(pHWNode, DST0_OP_POS, op_pos.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST0_OP_POS, op_pos.all);
#endif

    op_size.bitc.dst_op_w = pDrvSurface->u32Width;
    op_size.bitc.dst_op_h = pDrvSurface->u32Height;
    TdeNodeSetData(pHWNode, DST_OP_SIZE, op_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_DST_OP_SIZE, op_size.all);
#endif

    TDE_FUN_OUT;
    return;
}

static mt_void TdeHalNodeSetMultiply(TDE_HWNode_S *pHWNode, multip_cfg_t *p_cfg)
{
    // set the rop operation
    REG_COMP_MULT_MOD mult_mod;
    //REG_COMP_BLD_MOD    comp_bld_mod;
    //REG_ROP_ID   rop_id;
    //REG_ROP_PAT   rop_pat;       // pattern color
    //REG_COMP_CFG   comp_cfg;
    //REG_GRA_EN  gra_en;

    TDE_FUN_IN;
    //TDE_LOG("PARA: [%x][%x][%x] [%x][%x][%x]\n", p_cfg->src1_mult_mod, p_cfg->src1_pre_mutil_en, p_cfg->src3_aplha_en, p_cfg->src3_mult_mod, p_cfg->src3_pre_mutil_en, p_cfg->src3_aplha_en);
    TDE_LOG("PARA: [%x][%x][%x] [%x][%x][%x]\n", p_cfg->src1_mult_mod, p_cfg->src3_mult_mod);

    //mult_mod.bitc.src1_glb_alp_en = p_cfg->src1_aplha_en;
    //mult_mod.bitc.src1_glb_alp = p_cfg->src1_aplha;

    //mult_mod.bitc.src1_premult_en = p_cfg->src1_pre_mutil_en;
    mult_mod.bitc.src1_mult_mod = p_cfg->src1_mult_mod;

    //mult_mod.bitc.src3_glb_alp_en = p_cfg->src3_aplha_en;
    //mult_mod.bitc.src3_glb_alp = p_cfg->src3_aplha;

    //mult_mod.bitc.src3_premult_en = p_cfg->src3_pre_mutil_en;
    mult_mod.bitc.src3_mult_mod = p_cfg->src3_mult_mod;
    TdeNodeSetData(pHWNode, COMP_MULT_MOD, mult_mod.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_MULT_MOD, mult_mod.all);
#endif

#if 0 // this will cfg in blend or rop
    comp_bld_mod.all = TdeNodeGetData(pHWNode, COMP_BLD_MOD);
    //comp_bld_mod.bitc.src1_color_bld_mod = GL_ONE;
    //comp_bld_mod.bitc.src1_alp_bld_mod = GL_ONE;
    //comp_bld_mod.bitc.src2_color_bld_mod = GL_ONE_MINUS_SRC_ALPHA;
    //comp_bld_mod.bitc.src2_alp_bld_mod = GL_ONE_MINUS_SRC_ALPHA;
    TdeNodeSetData(pHWNode, COMP_BLD_MOD, comp_bld_mod.all);

    comp_cfg.all = TdeNodeGetData(pHWNode, COMP_CFG);
    comp_cfg.bitc.comp_mod = 0x1;
    comp_cfg.bitc.comp_src1_en = 0x1;
    TdeNodeSetData(pHWNode, COMP_CFG, comp_cfg.all);

    gra_en.all =  TdeNodeGetData(pHWNode, GRA_EN);
    gra_en.bitc.comp_en = 0x1;
    TdeNodeSetData(pHWNode, GRA_EN, gra_en.all);
#endif

    TDE_FUN_OUT;
    return;
}

mt_s32 TdeHalNodeSetBaseOperate(TDE_HWNode_S *pHWNode, TDE_DRV_BASEOPT_MODE_E enMode,
                                TDE_DRV_ALU_MODE_E enAlu, TDE_DRV_COLORFILL_S *pstColorFill)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 u32Capability = 0;
    TdeHalGetCapability(&u32Capability);
    TDE_FUN_IN;
    if (pstColorFill == NULL)
    {
        TDE_LOG("PARA: [%x][%x]\n", enMode, enAlu);
    }
    else
    {
        TDE_LOG("PARA: [%x][%x] [%x][%x]\n", enMode, enAlu, pstColorFill->enDrvColorFmt, pstColorFill->u32FillData);
    }

    switch (enMode) {
        case TDE_QUIKE_FILL: /*quick file*/ /*CNcomment:快速填充*/
        {
//            TDE_ASSERT(MT_NULL != pstColorFill);
			if(pstColorFill == NULL)
				return MT_ERR_TDE_NULL_PTR;
            if (!(u32Capability & QUICKFILL))
            {
                TDE_TRACE(TDE_KERN_INFO, "It deos not support QuickFill\n");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }
            // quik fill, do patten
            TdeHalNodeSetRopID(pHWNode, ROP_PATCOPY, ROP_PATCOPY);
            TdeAriaSetPatternData(pHWNode, pstColorFill->u32FillData);
            TdeAriaSetCompCfg(pHWNode, COMP_BP_PATTERN);
        } break;
        case TDE_QUIKE_COPY: /*quick copy*/ /*CNcomment:快速拷贝*/
        {
            if (!(u32Capability & QUICKCOPY)) {
                TDE_TRACE(TDE_KERN_INFO, "It deos not support QuickCopy\n");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }

            TdeHalNodeSetRopID(pHWNode, ROP_COPYPEN, ROP_COPYPEN);
            TdeAriaSetCompCfg(pHWNode, COMP_BP_SRC1);
        } break;
        case TDE_NORM_FILL_1OPT: /*signal fill*/ /*CNcomment:普通单源填充*/
        {
//            TDE_ASSERT(MT_NULL != pstColorFill);
			if(pstColorFill == NULL)
				return MT_ERR_TDE_NULL_PTR;
            //TDE_FILL_DATA_BY_FMT(pHWNode->u32TDE_S2_FILL,
            //   pstColorFill->u32FillData, pstColorFill->enDrvColorFmt);
            if (TDE_ALU_NONE == enAlu) {
                //unAluMode.stBits.u32AluMod = TDE_SRC2_BYPASS;
            } else {
                //unAluMode.stBits.u32AluMod = (MT_U32)enAlu;
            }
        } break;
        case TDE_NORM_BLIT_1OPT: /*signal blit*/ /*CNcomment:普通单源操作搬移*/
        {
            if (TDE_ALU_NONE == enAlu) {
                TdeHalNodeSetRopID(pHWNode, ROP_COPYPEN, ROP_COPYPEN);
                TdeAriaSetCompCfg(pHWNode, COMP_BP_SRC1);
            } else {
                //unAluMode.stBits.u32AluMod = (MT_U32)enAlu;
            }
        } break;
        case TDE_NORM_FILL_2OPT: /*signal color with bitmap operation and blit*/ /*CNcomment:单色和位图运算操作搬移*/
        {
//            TDE_ASSERT(MT_NULL != pstColorFill);
			if(pstColorFill == NULL)
				return MT_ERR_TDE_NULL_PTR;
            //TDE_FILL_DATA_BY_FMT(pHWNode->u32TDE_S2_FILL,
            //    pstColorFill->u32FillData, pstColorFill->enDrvColorFmt);
            if (TDE_ALU_NONE == enAlu) {
            } else {
            }
        } break;
        case TDE_NORM_BLIT_2OPT: /*double blit*/ /*CNcomment:普通双源操作搬移 */
        {
            if (((TDE_ALU_MASK_ROP1 == enAlu) || (TDE_ALU_MASK_ROP2 == enAlu)) && (!(u32Capability & MASKROP))) {
                TDE_TRACE(TDE_KERN_INFO, "It deos not support MaskRop\n");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }

            if ((TDE_ALU_MASK_BLEND == enAlu) && (!(u32Capability & MASKBLEND))) {
                TDE_TRACE(TDE_KERN_INFO, "It deos not support MaskBlend\n");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }

            if (TDE_ALU_NONE == enAlu) {
                //unAluMode.stBits.u32AluMod = TDE_SRC2_BYPASS;
                TdeHalNodeSetRopID(pHWNode, ROP_COPYPEN, ROP_COPYPEN);
                TdeAriaSetCompCfg(pHWNode, COMP_BP_SRC1);
            } else {
                //unAluMode.stBits.u32AluMod = (MT_U32)enAlu;
            }
        } break;
        case TDE_MB_2OPT: /*mb combination operation*/ /*CNcomment:宏块合并操作*/
        {
            if (TDE_ALU_CONCA == enAlu || TDE_ALU_NONE == enAlu) {
                TdeHalNodeSetRopID(pHWNode, ROP_COPYPEN, ROP_COPYPEN);
                TdeAriaSetCompCfg(pHWNode, COMP_BP_SRC1);
            } else {
            }
        } break;
        case TDE_MB_C_OPT: /*mb cbcr sampling operation*/ /*CNcomment:宏块色度上采样操作*/
        {
        } break;
        case TDE_MB_Y_OPT: /*mb ligthness resize*/ /*CNcomment:宏块亮度缩放*/
        {
        } break;
        case TDE_SINGLE_SRC_PATTERN_FILL_OPT:
        {
            if (!(u32Capability & PATTERFILL)) {
                TDE_TRACE(TDE_KERN_INFO, "It deos not support PatternFill\n");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }

            if (TDE_ALU_NONE != enAlu) {
                //unAluMode.stBits.u32AluMod = enAlu;
            } else {
                //unAluMode.stBits.u32AluMod = TDE_SRC2_BYPASS;
            }
        } break;
        case TDE_DOUBLE_SRC_PATTERN_FILL_OPT:
        {
            if (!(u32Capability & PATTERFILL)) {
                TDE_TRACE(TDE_KERN_INFO, "It deos not support PatternFill\n");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }

            if (TDE_ALU_NONE != enAlu) {
                //unAluMode.stBits.u32AluMod = enAlu;
            } else {
                //unAluMode.stBits.u32AluMod = TDE_SRC2_BYPASS;
            }
        } break;
        case TDE_TREE_SRC_OPT:
        {
            multip_cfg_t mult_cfg = { 0 };

            if (TDE_ALU_MULTIPLY == enAlu) {
                mult_cfg.src1_mult_mod = 0x1;
                TdeHalNodeSetMultiply(pHWNode, &mult_cfg);
            } else if (TDE_ALU_STENCIL == enAlu) {
                mult_cfg.src3_mult_mod = 0x2;
                TdeHalNodeSetMultiply(pHWNode, &mult_cfg);
            } else if (TDE_ALU_MASK_BLEND == enAlu) {
                mult_cfg.src1_mult_mod = 0x3;
                TdeHalNodeSetMultiply(pHWNode, &mult_cfg);
            } else if (TDE_ALU_MASK_ROP1 == enAlu) {

            } else {
            }
        } break;

        default:
            break;
    }

    TDE_FUN_OUT;
    return ret;
}

mt_void TdeHalNodeSetGlobalAlpha(TDE_HWNode_S *pHWNode, mt_u8 u8Alpha, MT_BOOL bEnable)
{
    REG_COMP_MULT_MOD comp_mult_mod;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x]\n", u8Alpha, bEnable);

    comp_mult_mod.all = TdeNodeGetData(pHWNode, COMP_MULT_MOD);

    comp_mult_mod.bitc.src1_glb_alp_en = bEnable;
    comp_mult_mod.bitc.src1_glb_alp = u8Alpha;

    TdeNodeSetData(pHWNode, COMP_MULT_MOD, comp_mult_mod.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_MULT_MOD, comp_mult_mod.all);
#endif

    TDE_FUN_OUT;

    return;
}

mt_void TdeHalNodeSetExpAlpha(TDE_HWNode_S *pHWNode, TDE_DRV_SRC_E enSrc, mt_u8 u8Alpha0, mt_u8 u8Alpha1)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%x]\n", enSrc, u8Alpha0, u8Alpha1);

    TDE_FUN_OUT;

    return;
}

mt_void TdeHalNodeSetAlphaBorder(TDE_HWNode_S *pHWNode, MT_BOOL bVEnable, MT_BOOL bHEnable)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x]\n", bVEnable, bHEnable);

    TDE_FUN_OUT;

    return;
}

static mt_void TdeHalNodeSetRopID(TDE_HWNode_S *pHWNode, rop_mod_t rop_c, rop_mod_t rop_a)
{
    // set the rop operation
    REG_ROP_ID rop_id;
    //REG_ROP_PAT   rop_pat;       // pattern color
    REG_COMP_CFG comp_cfg;
    REG_GRA_EN gra_en;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x]\n", rop_c, rop_a);

    rop_id.bitc.rop_id = rop_a << 8 | rop_c;
    TdeNodeSetData(pHWNode, ROP_ID, rop_id.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_ROP_ID, rop_id.all);
#endif

    comp_cfg.all = TdeNodeGetData(pHWNode, COMP_CFG);
    comp_cfg.bitc.comp_mod = 0x0;
    TdeNodeSetData(pHWNode, COMP_CFG, comp_cfg.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_CFG, comp_cfg.all);
#endif

    gra_en.all = TdeNodeGetData(pHWNode, GRA_EN);
    gra_en.bitc.comp_en = 0x1;
    TdeNodeSetData(pHWNode, GRA_EN, gra_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_EN, gra_en.all);
#endif

    TDE_FUN_OUT;
    return;
}

mt_s32 TdeHalNodeSetRop(TDE_HWNode_S *pHWNode, TDE2_ROP_CODE_E enRgbRop, TDE2_ROP_CODE_E enAlphaRop)
{
    rop_mod_t rop_c;
    rop_mod_t rop_a;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x]\n", enRgbRop, enAlphaRop);

    rop_c = tde_hal_convert_tde_rop_id_to_aria_rop_mod(enRgbRop);
    rop_a = tde_hal_convert_tde_rop_id_to_aria_rop_mod(enAlphaRop);

    TdeHalNodeSetRopID(pHWNode, rop_c, rop_a);

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetBlend(TDE_HWNode_S *pHWNode, TDE2_BLEND_OPT_S *pstBlendOpt)
{
    REG_GRA_EN gra_en;
    REG_COMP_CFG comp_cfg;
    REG_COMP_MULT_MOD comp_mult_mod;
    REG_COMP_BLD_MOD comp_bld_mod;

    TDE2_BLEND_MODE_E mod1;
    TDE2_BLEND_MODE_E mod2;

    bld_fact_t fact_src1_c; // src1 color
    bld_fact_t fact_src2_c; // src2 color
    bld_fact_t fact_src1_a; // src1 alpha
    bld_fact_t fact_src2_a; // src2 alpha
    MT_BOOL need_convert = MT_FALSE;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%x][%x][%x][%x][%x]\n", pstBlendOpt->eBlendCmd, pstBlendOpt->bGlobalAlphaEnable, pstBlendOpt->bPixelAlphaEnable, pstBlendOpt->bSrc1AlphaPremulti, pstBlendOpt->bSrc2AlphaPremulti, pstBlendOpt->eSrc1BlendMode, pstBlendOpt->eSrc2BlendMode);
    /*set mode for src1 and src2*/ /*CNcomment:  配置Src1、Src2模式 */
    switch (pstBlendOpt->eBlendCmd) {
    /**< fs: sa      fd: 1.0-sa */
        case TDE2_BLENDCMD_NONE:
        {
            //mod1 = TDE2_BLEND_INVSRC2ALPHA;
            //mod2 = TDE2_BLEND_SRC2ALPHA;
            fact_src1_c = GL_SRC_ALPHA;
            fact_src1_a = GL_SRC_ALPHA;
            fact_src2_c = GL_ONE_MINUS_SRC_ALPHA;
            fact_src2_a = GL_ONE_MINUS_SRC_ALPHA;
            break;
        }
        /**< fs: 0.0     fd: 0.0 */
        case TDE2_BLENDCMD_CLEAR:
        {
            //mod1  = TDE2_BLEND_ZERO;
            //mod2 = TDE2_BLEND_ZERO;
            fact_src1_c = GL_ZERO;
            fact_src1_a = GL_ZERO;
            fact_src2_c = GL_ZERO;
            fact_src2_a = GL_ZERO;
            break;
        }
        /**< fs: 1.0     fd: 0.0 */
        case TDE2_BLENDCMD_SRC:
        {
            //mod1 = TDE2_BLEND_ZERO;
            //mod2 = TDE2_BLEND_ONE;
            fact_src1_c = GL_ONE;
            fact_src1_a = GL_ONE;
            fact_src2_c = GL_ZERO;
            fact_src2_a = GL_ZERO;
            break;
        }
        /**< fs: 1.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCOVER:
        {
            //mod1 = TDE2_BLEND_INVSRC2ALPHA;
            //mod2 = TDE2_BLEND_ONE;
            fact_src1_c = GL_ONE;
            fact_src1_a = GL_ONE;
            fact_src2_c = GL_ONE_MINUS_SRC_ALPHA;
            fact_src2_a = GL_ONE_MINUS_SRC_ALPHA;
            break;
        }
        /**< fs: 1.0-da  fd: 1.0 */
        case TDE2_BLENDCMD_DSTOVER:
        {
            //mod1 = TDE2_BLEND_ONE;
            //mod2 = TDE2_BLEND_INVSRC1ALPHA;
            fact_src1_c = GL_ONE_MINUS_DST_ALPHA;
            fact_src1_a = GL_ONE_MINUS_DST_ALPHA;
            fact_src2_c = GL_ONE;
            fact_src2_a = GL_ONE;
            break;
        }
        /**< fs: da      fd: 0.0 */
        case TDE2_BLENDCMD_SRCIN:
        {
            //mod1 = TDE2_BLEND_ZERO;
            //mod2 = TDE2_BLEND_SRC1ALPHA;
            fact_src1_c = GL_DST_ALPHA;
            fact_src1_a = GL_DST_ALPHA;
            fact_src2_c = GL_ZERO;
            fact_src2_a = GL_ZERO;
            break;
        }
        /**< fs: 0.0     fd: sa */
        case TDE2_BLENDCMD_DSTIN:
        {
            //mod1 = TDE2_BLEND_SRC2ALPHA;
            //mod2 = TDE2_BLEND_ZERO;
            fact_src1_c = GL_ZERO;
            fact_src1_a = GL_ZERO;
            fact_src2_c = GL_SRC_ALPHA;
            fact_src2_a = GL_SRC_ALPHA;
            break;
        }
        /**< fs: 1.0-da  fd: 0.0 */
        case TDE2_BLENDCMD_SRCOUT:
        {
            //mod1 = TDE2_BLEND_ZERO;
            //mod2 = TDE2_BLEND_INVSRC1ALPHA;
            fact_src1_c = GL_ONE_MINUS_DST_ALPHA;
            fact_src1_a = GL_ONE_MINUS_DST_ALPHA;
            fact_src2_c = GL_ZERO;
            fact_src2_a = GL_ZERO;
            break;
        }
        /**< fs: 0.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_DSTOUT:
        {
            //mod1 = TDE2_BLEND_INVSRC2ALPHA;
            //mod2 = TDE2_BLEND_ZERO;
            fact_src1_c = GL_ZERO;
            fact_src1_a = GL_ZERO;
            fact_src2_c = GL_ONE_MINUS_SRC_ALPHA;
            fact_src2_a = GL_ONE_MINUS_SRC_ALPHA;
            break;
        }
        /**< fs: da      fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCATOP:
        {
            //mod1 = TDE2_BLEND_INVSRC2ALPHA;
            //mod2 = TDE2_BLEND_SRC1ALPHA;
            fact_src1_c = GL_DST_ALPHA;
            fact_src1_a = GL_DST_ALPHA;
            fact_src2_c = GL_ONE_MINUS_SRC_ALPHA;
            fact_src2_a = GL_ONE_MINUS_SRC_ALPHA;
            break;
        }
        /**< fs: 1.0-da  fd: sa */
        case TDE2_BLENDCMD_DSTATOP:
        {
            //mod1 = TDE2_BLEND_SRC2ALPHA;
            //mod2 = TDE2_BLEND_INVSRC1ALPHA;
            fact_src1_c = GL_ONE;
            fact_src1_a = GL_ONE;
            fact_src2_c = GL_SRC_ALPHA;
            fact_src2_a = GL_SRC_ALPHA;
            break;
        }
        /**< fs: 1.0     fd: 1.0 */
        case TDE2_BLENDCMD_ADD:
        {
            //mod1 = TDE2_BLEND_ONE;
            //mod2 = TDE2_BLEND_ONE;
            fact_src1_c = GL_ONE;
            fact_src1_a = GL_ONE;
            fact_src2_c = GL_ONE;
            fact_src2_a = GL_ONE;
            break;
        }
        /**< fs: 1.0-da  fd: 1.0-sa */
        case TDE2_BLENDCMD_XOR:
        {
            //mod1 = TDE2_BLEND_INVSRC2ALPHA;
            //mod2 = TDE2_BLEND_INVSRC1ALPHA;
            fact_src1_c = GL_ONE_MINUS_DST_ALPHA;
            fact_src1_a = GL_ONE_MINUS_DST_ALPHA;
            fact_src2_c = GL_ONE_MINUS_SRC_ALPHA;
            fact_src2_a = GL_ONE_MINUS_SRC_ALPHA;
            break;
        }
        /**< fs: 0.0  fd: 1.0*/
        case TDE2_BLENDCMD_DST:
        {
            //mod1 = TDE2_BLEND_ONE;
            //mod2 = TDE2_BLEND_ZERO;
            fact_src1_c = GL_ZERO;
            fact_src1_a = GL_ZERO;
            fact_src2_c = GL_ONE;
            fact_src2_a = GL_ONE;
            break;
        }
  /*    user parameter*/ /*CNcomment:  用户自己配置参数 */
        case TDE2_BLENDCMD_CONFIG:
        default:
        {
            mod1 = pstBlendOpt->eSrc1BlendMode;
            mod2 = pstBlendOpt->eSrc2BlendMode;
            need_convert = MT_TRUE;
            break;
        }
    }

    if (need_convert)
    {
        fact_src1_a = fact_src1_c = tde_hal_convert_bld_mod_to_fact(mod2); // src2 is aria src1
        fact_src2_a = fact_src2_c = tde_hal_convert_bld_mod_to_fact(mod1); // src1 is aria src2
    }

    comp_bld_mod.all = TdeNodeGetData(pHWNode, COMP_BLD_MOD);
    comp_bld_mod.bitc.src1_color_bld_mod = fact_src1_c;
    comp_bld_mod.bitc.src2_color_bld_mod = fact_src2_c;
    comp_bld_mod.bitc.src1_alp_bld_mod = fact_src1_a;
    comp_bld_mod.bitc.src2_alp_bld_mod = fact_src2_a;
    TdeNodeSetData(pHWNode, COMP_BLD_MOD, comp_bld_mod.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_BLD_MOD, comp_bld_mod.all);
#endif

    comp_cfg.all = TdeNodeGetData(pHWNode, COMP_CFG);
    comp_cfg.bitc.comp_src1_en = 0x1;
    comp_cfg.bitc.comp_mod = 0x1;
    TdeNodeSetData(pHWNode, COMP_CFG, comp_cfg.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_CFG, comp_cfg.all);
#endif

    comp_mult_mod.all = TdeNodeGetData(pHWNode, COMP_MULT_MOD);
    comp_mult_mod.bitc.src1_premult_en = pstBlendOpt->bSrc2AlphaPremulti;
    comp_mult_mod.bitc.src1_glb_alp_en = pstBlendOpt->bGlobalAlphaEnable;
    TdeNodeSetData(pHWNode, COMP_MULT_MOD, comp_mult_mod.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COMP_MULT_MOD, comp_mult_mod.all);
#endif

    gra_en.all = TdeNodeGetData(pHWNode, GRA_EN);
    gra_en.bitc.comp_en = 0x1;
    TdeNodeSetData(pHWNode, GRA_EN, gra_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_EN, gra_en.all);
#endif

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetColorize(TDE_HWNode_S *pHWNode, mt_u32 u32Colorize)
{
    TDE_FUN_IN;
    TdeAriaSetPatternData(pHWNode, u32Colorize);
    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_void TdeHalNodeEnableAlphaRop(TDE_HWNode_S *pHWNode)
{
    TDE_FUN_IN;

    // TODO::

    TDE_FUN_OUT;
    return;
}

static mt_u8 TdeHalBlurLevelCheck(TDE2_GS_BLUR_S *p_blur_cfg)
{
    mt_u8 blur_level = 0;

    MT_ASSERT(NULL != p_blur_cfg);

    TDE_LOG("Blur Param: [%x][%x]\n", p_blur_cfg->blur_level, p_blur_cfg->edge_opt);

    blur_level = p_blur_cfg->blur_level;

    if (blur_level > TDE2_MAX_GS_BLUR_LEVEL)
    {
        blur_level = TDE2_MAX_GS_BLUR_LEVEL;
    }

    blur_level = blur_level * 2 + 3;

    return blur_level;
}

mt_s32 TdeHalNodeSetBlurInfo(TDE_HWNode_S *pHWNode, TDE2_GS_BLUR_S *p_blur_cfg)
{
    mt_u8 blur_level = 0;
    REG_SCALER_CFG scaler_cfg;
    REG_GRA_EN gra_en;

    TDE_FUN_IN;

    blur_level = TdeHalBlurLevelCheck(p_blur_cfg);

    scaler_cfg.all = TdeNodeGetData(pHWNode, SCALER_CFG);
    gra_en.all = TdeNodeGetData(pHWNode, GRA_EN); // need gra_en.bitc.scaler_en = 0x1;

    scaler_cfg.bitc.blur_en = 0x1;
    scaler_cfg.bitc.blur_num = blur_level;
    scaler_cfg.bitc.scaler_edge_option = p_blur_cfg->edge_opt;
    scaler_cfg.bitc.scaler_mod = 0x2;

    TdeNodeSetData(pHWNode, SCALER_CFG, scaler_cfg.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_CFG, scaler_cfg.all);
#endif

    gra_en.bitc.scaler_en = 0x1;
    TdeNodeSetData(pHWNode, GRA_EN, gra_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_EN, gra_en.all);
#endif

    TDE_FUN_OUT;

    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetRegularRotatorOpt(TDE_HWNode_S *pHWNode, TDE2_ROTATOR_TYPE_S rotator_type)
{
    REG_ROT_PAT_CFG rotator_cfg;
    REG_GRA_EN gra_en;

    TDE_FUN_IN;
    TDE_LOG("Param: [%x]\n", rotator_type);

    rotator_cfg.all = TdeNodeGetData(pHWNode, ROT_PAT_CFG);
    gra_en.all = TdeNodeGetData(pHWNode, GRA_EN);

    rotator_cfg.bitc.rot_type = rotator_type;
    TdeNodeSetData(pHWNode, ROT_PAT_CFG, rotator_cfg.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_ROT_PAT_CFG, rotator_cfg.all);
#endif

    gra_en.bitc.rot_en = 0x1;
    gra_en.bitc.comp_en = 0x0;
    TdeNodeSetData(pHWNode, GRA_EN, gra_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_EN, gra_en.all);
#endif

    TDE_FUN_OUT;

    return MT_SUCCESS;
}

static mt_void TdeHalNodeSetScalerInfo(TDE_HWNode_S *pHWNode, scale_cfg_t *p_scale_cfg)
{

    REG_SCALER_COEF_11 scaler_coef_11;
    REG_SCALER_COEF_21 scaler_coef_21;
    REG_SCALER_COEF_31 scaler_coef_31;
    REG_SCALER_COEF_22 scaler_coef_22;
    REG_SCALER_COEF_23 scaler_coef_23;
    REG_SCALER_INIT_PHASE scaler_init_phase;
    REG_COEF_ADDR coef_addr;

    REG_SCALER_CFG scaler_cfg;
    REG_LOAD_EN load_en;
    REG_GRA_EN gra_en;

    /*
    scaler_coef_11.all = TdeNodeGetData(pHWNode, SCALER_COEF_11);
    scaler_coef_21.all = TdeNodeGetData(pHWNode, SCALER_COEF_21);
    scaler_coef_31.all = TdeNodeGetData(pHWNode, SCALER_COEF_31);
    scaler_coef_22.all = TdeNodeGetData(pHWNode, SCALER_COEF_22);
    scaler_coef_23.all = TdeNodeGetData(pHWNode, SCALER_COEF_23);
    scaler_init_phase.all = TdeNodeGetData(pHWNode, SCALER_INIT_PHASE);
    */

    scaler_cfg.all = TdeNodeGetData(pHWNode, SCALER_CFG);
    load_en.all = TdeNodeGetData(pHWNode, LOAD_EN);
    gra_en.all = TdeNodeGetData(pHWNode, GRA_EN);

    // set registers
    scaler_coef_11.bitc.scaler_coef_11 = p_scale_cfg->coef[0];
    scaler_coef_21.bitc.scaler_coef_21 = p_scale_cfg->coef[1];
    scaler_coef_31.bitc.scaler_coef_31 = p_scale_cfg->coef[2];
    scaler_coef_22.bitc.scaler_coef_22 = p_scale_cfg->coef[3];
    scaler_coef_23.bitc.scaler_coef_23 = p_scale_cfg->coef[4];

    TdeNodeSetData(pHWNode, SCALER_COEF_11, scaler_coef_11.all);
    TdeNodeSetData(pHWNode, SCALER_COEF_21, scaler_coef_21.all);
    TdeNodeSetData(pHWNode, SCALER_COEF_31, scaler_coef_31.all);
    TdeNodeSetData(pHWNode, SCALER_COEF_22, scaler_coef_22.all);
    TdeNodeSetData(pHWNode, SCALER_COEF_23, scaler_coef_23.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_COEF_11, scaler_coef_11.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_COEF_21, scaler_coef_21.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_COEF_31, scaler_coef_31.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_COEF_22, scaler_coef_22.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_COEF_23, scaler_coef_23.all);
#endif

    scaler_init_phase.bitc.scaler_init_phase = p_scale_cfg->init_phase;
    TdeNodeSetData(pHWNode, SCALER_INIT_PHASE, scaler_init_phase.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_INIT_PHASE, scaler_init_phase.all);
#endif

    coef_addr.all = (p_scale_cfg->coef_addr & 0x1fffffff);
    TdeNodeSetData(pHWNode, COEF_ADDR, coef_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_COEF_ADDR, coef_addr.all);
#endif

    scaler_cfg.bitc.scaler_mod = p_scale_cfg->scale_mod;
    TdeNodeSetData(pHWNode, SCALER_CFG, scaler_cfg.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SCALER_CFG, scaler_cfg.all);
#endif

    load_en.bitc.coef_load_en = 0x1;
    TdeNodeSetData(pHWNode, LOAD_EN, load_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_LOAD_EN, load_en.all);
#endif

    gra_en.bitc.scaler_en = 0x1;
    if (p_scale_cfg->scale_mod != SCALE_HORI_LINE_OUT)
    {
        gra_en.bitc.rot_en = 0x1;
        gra_en.bitc.comp_en = 0x0;
    }
    else
    {
        gra_en.bitc.rot_en = 0x0;
        gra_en.bitc.comp_en = 0x0;
    }

    TdeNodeSetData(pHWNode, GRA_EN, gra_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_EN, gra_en.all);
#endif

    return;
}

mt_s32 TdeHalNodeSetScaler(TDE_HWNode_S *pHWNode, TDE_DRV_SURFACE_S *p_src, TDE_DRV_SURFACE_S *p_dst)
{
    pos_t pos_00;
    pos_t pos_10;
    pos_t pos_01;
    pos_t pos_11;
    rect_vsb_t src_rect;
    scale_type_t scale_type = SCAL_TYPE_BOTN;

    scale_cfg_t scale_cfg = { 0 };

    scale_cfg.coef_addr = (mt_u32)stCoeffTableBuf.startPhyAddr;

    TDE_FUN_IN;

    pos_00.x = p_dst->u32Xpos;
    pos_00.y = p_dst->u32Ypos;

    pos_01.x = p_dst->u32Xpos + p_dst->u32Width;
    pos_01.y = p_dst->u32Ypos;

    pos_10.x = p_dst->u32Xpos;
    pos_10.y = p_dst->u32Ypos + p_dst->u32Height;

    pos_11.x = p_dst->u32Xpos + p_dst->u32Width;
    pos_11.y = p_dst->u32Xpos + p_dst->u32Height;

    src_rect.x = p_src->u32Xpos;
    src_rect.y = p_src->u32Ypos;
    src_rect.w = p_src->u32Width;
    src_rect.h = p_src->u32Height;

    tde_hal_get_scale_coeff(&src_rect, &pos_00, &pos_01, &pos_10, &pos_11, &scale_cfg.coef[0], &scale_type);
    if (scale_type == SCAL_TYPE_BOTN)
    {
        // error
        TDE_FUN_OUT;
        return -1;
    }

    if (scale_type == SCALE_ONLY_HORI_RECT)
    {
        scale_cfg.scale_mod = SCALE_HORI_LINE_OUT; // 2d h scale
    }
    else if (scale_type == SCALE_ONLY_VERT_RECT)
    {
        scale_cfg.scale_mod = SCALE_VERT_BLK_OUT; // 2d v scale
    }
    else
    {
        scale_cfg.scale_mod = SCALE_HORI_LINE_OUT; // fixed me ?? 3d
    }

    //init_phase check
    if (scale_cfg.coef[5] == 0x1)
    {
        scale_cfg.init_phase = 0x10;
    }
    else
    {
        if (scale_cfg.scale_mod == SCALE_HORI_BLK_OUT || scale_cfg.scale_mod == SCALE_HORI_LINE_OUT)
        {
            scale_cfg.init_phase = 16 + (p_src->u32Width * 16) / p_dst->u32Width;
        }
        else if (scale_cfg.scale_mod == SCALE_VERT_BLK_OUT)
        {
            scale_cfg.init_phase = 16 + (p_src->u32Height * 16) / p_dst->u32Height;
        }
        else
        {
            scale_cfg.init_phase = 0x20;
        }
    }

    if (scale_cfg.coef[5] == 0x0)
    {
        //scale_cfg.init_phase |= (1 << 16);
    }

    TDE_LOG("PARA: [%x][%x][%x][%x] [%x][%x][%x][%x], [%x][%x][%x][%x]\n", p_src->u32Xpos, p_src->u32Ypos, p_src->u32Width, p_src->u32Height, p_dst->u32Xpos, p_dst->u32Ypos, p_dst->u32Width, p_dst->u32Height, scale_type, scale_cfg.scale_mod, scale_cfg.coef[5], scale_cfg.init_phase);
    TdeHalNodeSetScalerInfo(pHWNode, &scale_cfg);

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetClutOpt(TDE_HWNode_S *pHWNode, TDE_DRV_CLUT_CMD_S *pClutCmd, MT_BOOL bReload)
{
    REG_SRC1_FMT_CFG0 src1_cfg0;
    //REG_SRC1_FMT_CFG1  src1_cfg1;
    REG_PAL_SIZE pal_size;
    REG_LOAD_EN load_en;
    REG_PAL1_ADDR pal_addr;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%lx]\n", bReload, pClutCmd->enClutMode, pClutCmd->pu8PhyClutAddr);

    // only src1 is load palette;
    src1_cfg0.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG0);

    pal_size.all = TdeNodeGetData(pHWNode, PAL_SIZE);
    load_en.all = TdeNodeGetData(pHWNode, LOAD_EN);
    pal_addr.all = TdeNodeGetData(pHWNode, PAL1_ADDR);

    pal_size.bitc.pal1_size = 256;
    load_en.bitc.pal1_load_en = 1;
    TdeNodeSetData(pHWNode, LOAD_EN, load_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_LOAD_EN, load_en.all);
#endif

    TdeNodeSetData(pHWNode, PAL_SIZE, pal_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_PAL_SIZE, pal_size.all);
#endif

    pal_addr.bitc.pal1_addr = pClutCmd->pu8PhyClutAddr & 0x1fffffff;
    TdeNodeSetData(pHWNode, PAL1_ADDR, pal_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_PAL1_ADDR, pal_addr.all);
#endif

    src1_cfg0.bitc.src1_clut_en = 0x1;
    TdeNodeSetData(pHWNode, SRC1_FMT_CFG0, src1_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG0, src1_cfg0.all);
#endif

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetClutOpt2(TDE_HWNode_S *pHWNode, TDE_DRV_CLUT_CMD_S *pClutCmd, MT_BOOL bReload)
{
    REG_SRC3_FMT_CFG0 src3_cfg0;
    //REG_SRC1_FMT_CFG1  src1_cfg1;
    REG_PAL_SIZE pal_size;
    REG_LOAD_EN load_en;
    REG_PAL3_ADDR pal_addr;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%lx]\n", bReload, pClutCmd->enClutMode, pClutCmd->pu8PhyClutAddr);

    // src3 is load palette;
    src3_cfg0.all = TdeNodeGetData(pHWNode, SRC3_FMT_CFG0);

    pal_size.all = TdeNodeGetData(pHWNode, PAL_SIZE);
    load_en.all = TdeNodeGetData(pHWNode, LOAD_EN);
    pal_addr.all = TdeNodeGetData(pHWNode, PAL3_ADDR);

    pal_size.bitc.pal3_size = 256;
    load_en.bitc.pal3_load_en = 1;
    TdeNodeSetData(pHWNode, LOAD_EN, load_en.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_LOAD_EN, load_en.all);
#endif

    TdeNodeSetData(pHWNode, PAL_SIZE, pal_size.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_PAL_SIZE, pal_size.all);
#endif

    pal_addr.bitc.pal3_addr = pClutCmd->pu8PhyClutAddr & 0x1fffffff;
    TdeNodeSetData(pHWNode, PAL3_ADDR, pal_addr.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_PAL3_ADDR, pal_addr.all);
#endif

    src3_cfg0.bitc.src3_clut_en = 0x1;
    TdeNodeSetData(pHWNode, SRC3_FMT_CFG0, src3_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC3_FMT_CFG0, src3_cfg0.all);
#endif

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetColorKey(TDE_HWNode_S *pHWNode, TDE_COLORFMT_CATEGORY_E enFmtCat,
                             TDE_DRV_COLORKEY_CMD_S *pColorKey)
{
    REG_SRC1_FMT_CFG1 src1_fmt_cfg1;
    REG_SRC1_KEY_MIN src1_key_min;
    REG_SRC1_KEY_MAX src1_key_max;

    //REG_DST0_FMT_CFG1  dst0_fmt_cfg;

    mt_u32 key_mim = 0;
    mt_u32 key_max = 0;
    mt_u32 key_mod_reg = 0;

    ck_mod_t key_mod = KEY_MATCH_NONE;

    TDE_FUN_IN;
    TDE_LOG("PARA: [%x][%x][%x]\n", enFmtCat, pColorKey->enColorKeyMode, (mt_u32) & pColorKey->unColorKeyValue);

    TDE_ASSERT(MT_NULL != pHWNode);
    TDE_ASSERT(MT_NULL != pColorKey);

    //unIns.u32All = pHWNode->u32TDE_INS;
    //unIns.stBits.u32ColorKey = 1; /*enable color key operation*//*CNcomment:使能Color Key操作*/
    //unAluMode.u32All = pHWNode->u32TDE_ALU;
    //unAluMode.stBits.u32CkSel = (mt_u32)pColorKey->enColorKeyMode;

    key_mod_reg = 0;
    if (TDE_COLORFMT_CATEGORY_ARGB == enFmtCat)
    {
        key_mim = pColorKey->unColorKeyValue.struCkARGB.stBlue.u8CompMin | (pColorKey->unColorKeyValue.struCkARGB.stGreen.u8CompMin << 8) | (pColorKey->unColorKeyValue.struCkARGB.stRed.u8CompMin << 16) | (pColorKey->unColorKeyValue.struCkARGB.stAlpha.u8CompMin << 24);

        key_max = pColorKey->unColorKeyValue.struCkARGB.stBlue.u8CompMax | (pColorKey->unColorKeyValue.struCkARGB.stGreen.u8CompMax << 8) | (pColorKey->unColorKeyValue.struCkARGB.stRed.u8CompMax << 16) | (pColorKey->unColorKeyValue.struCkARGB.stAlpha.u8CompMax << 24);

        // in aria , there is no color key mask
        /*
        pHWNode->u32TDE_CK_MASK = pColorKey->unColorKeyValue.struCkARGB.stBlue.u8CompMask
            | (pColorKey->unColorKeyValue.struCkARGB.stGreen.u8CompMask << 8)
            | (pColorKey->unColorKeyValue.struCkARGB.stRed.u8CompMask << 16)
            | (pColorKey->unColorKeyValue.struCkARGB.stAlpha.u8CompMask << 24);
        */

        if (pColorKey->unColorKeyValue.struCkARGB.stBlue.bCompIgnore)
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkARGB.stBlue.bCompOut)
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod;

        if (pColorKey->unColorKeyValue.struCkARGB.stGreen.bCompIgnore)
        {
            //unAluMode.stBits.u32CkGMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkARGB.stGreen.bCompOut)
        {
            //unAluMode.stBits.u32CkGMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkGMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 4;

        if (pColorKey->unColorKeyValue.struCkARGB.stRed.bCompIgnore)
        {
            //unAluMode.stBits.u32CkRMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkARGB.stRed.bCompOut)
        {
            //unAluMode.stBits.u32CkRMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkRMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 8;

        if (pColorKey->unColorKeyValue.struCkARGB.stAlpha.bCompIgnore)
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkARGB.stAlpha.bCompOut)
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 12;
    }
    else if (TDE_COLORFMT_CATEGORY_CLUT == enFmtCat) /*clut format use index*/ /*CNcomment:CLUT格式只用索引*/
    {
        key_mim = pColorKey->unColorKeyValue.struCkClut.stClut.u8CompMin | (pColorKey->unColorKeyValue.struCkClut.stAlpha.u8CompMin << 24);

        key_max = pColorKey->unColorKeyValue.struCkClut.stClut.u8CompMax | (pColorKey->unColorKeyValue.struCkClut.stAlpha.u8CompMax << 24);

    /*
          pHWNode->u32TDE_CK_MASK = pColorKey->unColorKeyValue.struCkClut.stClut.u8CompMask
              | (pColorKey->unColorKeyValue.struCkClut.stAlpha.u8CompMask << 24);
          */

        if (pColorKey->unColorKeyValue.struCkClut.stClut.bCompIgnore)
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkClut.stClut.bCompOut)
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod;

        if (pColorKey->unColorKeyValue.struCkClut.stAlpha.bCompIgnore)
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkClut.stAlpha.bCompOut)
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            // unAluMode.stBits.u32CkAMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 12;
    }
    else if (TDE_COLORFMT_CATEGORY_YCbCr == enFmtCat) /*YCbCr format*/ /*CNcomment:YCbCr格式*/
    {

        key_mim = pColorKey->unColorKeyValue.struCkYCbCr.stCr.u8CompMin | (pColorKey->unColorKeyValue.struCkYCbCr.stCb.u8CompMin << 8) | (pColorKey->unColorKeyValue.struCkYCbCr.stY.u8CompMin << 16) | (pColorKey->unColorKeyValue.struCkYCbCr.stAlpha.u8CompMin << 24);

        key_max = pColorKey->unColorKeyValue.struCkYCbCr.stCr.u8CompMax | (pColorKey->unColorKeyValue.struCkYCbCr.stCb.u8CompMax << 8) | (pColorKey->unColorKeyValue.struCkYCbCr.stY.u8CompMax << 16) | (pColorKey->unColorKeyValue.struCkYCbCr.stAlpha.u8CompMax << 24);

        /*
        pHWNode->u32TDE_CK_MASK = pColorKey->unColorKeyValue.struCkYCbCr.stCr.u8CompMask
              | (pColorKey->unColorKeyValue.struCkYCbCr.stCb.u8CompMask << 8)
              | (pColorKey->unColorKeyValue.struCkYCbCr.stY.u8CompMask << 16)
              | (pColorKey->unColorKeyValue.struCkYCbCr.stAlpha.u8CompMask << 24);
        */

        if (pColorKey->unColorKeyValue.struCkYCbCr.stCr.bCompIgnore)
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkYCbCr.stCr.bCompOut)
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkBMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod;

        if (pColorKey->unColorKeyValue.struCkYCbCr.stCb.bCompIgnore)
        {
            //unAluMode.stBits.u32CkGMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkYCbCr.stCb.bCompOut)
        {
            //unAluMode.stBits.u32CkGMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkGMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 4;

        if (pColorKey->unColorKeyValue.struCkYCbCr.stY.bCompIgnore)
        {
            //unAluMode.stBits.u32CkRMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkYCbCr.stY.bCompOut)
        {
            //unAluMode.stBits.u32CkRMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkRMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 8;

        if (pColorKey->unColorKeyValue.struCkYCbCr.stAlpha.bCompIgnore)
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_IGNORE;
            key_mod = KEY_MATCH_ALL;
        }
        else if (pColorKey->unColorKeyValue.struCkYCbCr.stAlpha.bCompOut)
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_AREA_OUT;
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            //unAluMode.stBits.u32CkAMod = TDE_COLORKEY_AREA_IN;
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mod_reg |= key_mod << 12;
    }
    else
    {
        TDE_TRACE(TDE_KERN_INFO, "It deos not support ColorKey\n");
        TDE_FUN_OUT;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    if (TDE_DRV_COLORKEY_BACKGROUND == pColorKey->enColorKeyMode)
    {
        // background color key, ====>  background is  mapping to src2 or dst?
    }
    else if (TDE_DRV_COLORKEY_FOREGROUND_AFTER_CLUT == pColorKey->enColorKeyMode)
    {
        // forground color key, after convert the clut to argb , ===> forground is maping to src1
        src1_fmt_cfg1.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG1);
        src1_fmt_cfg1.bitc.src1_key_inv = 0x0;
        src1_fmt_cfg1.bitc.src1_key_mod = key_mod_reg;
        TdeNodeSetData(pHWNode, SRC1_FMT_CFG1, src1_fmt_cfg1.all);
#ifdef USE_NORMAL_GPE
        TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG1, src1_fmt_cfg1.all);
#endif

        src1_key_min.all = TdeNodeGetData(pHWNode, SRC1_KEY_MIN);
        src1_key_min.bitc.src1_key_min = key_mim;
        TdeNodeSetData(pHWNode, SRC1_KEY_MIN, src1_key_min.all);
#ifdef USE_NORMAL_GPE
        TDE_REG_WRITE(ARIA_TDE_REG_SRC1_KEY_MIN, src1_key_min.all);
#endif

        src1_key_max.all = TdeNodeGetData(pHWNode, SRC1_KEY_MAX);
        src1_key_max.bitc.src1_key_max = key_max;
        TdeNodeSetData(pHWNode, SRC1_KEY_MAX, src1_key_max.all);
#ifdef USE_NORMAL_GPE
        TDE_REG_WRITE(ARIA_TDE_REG_SRC1_KEY_MAX, src1_key_max.all);
#endif

    }
    else if (TDE_DRV_COLORKEY_FOREGROUND_BEFORE_CLUT == pColorKey->enColorKeyMode)
    {
        // forground color key, before convert the clut to argb , ===> forground is maping to src1
        // aria not support this mode ??
    } else {
        // error
        MT_ASSERT(0);
    }

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetClipping(TDE_HWNode_S *pHWNode, TDE_DRV_CLIP_CMD_S *pClip)
{
    TDE_FUN_IN;
    TDE_LOG("PARA: [%x] [%d][%d][%d][%d]\n", pClip->bInsideClip, pClip->u16ClipStartX, pClip->u16ClipStartY, pClip->u16ClipEndX, pClip->u16ClipEndY);

    // TODO::
    // in aria tde , do not support this operatoin
    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetFlicker(TDE_HWNode_S *pHWNode, TDE_DRV_FLICKER_CMD_S *pFlicker)
{
    TDE_FUN_IN;
    // TODO::
    // in aria tde , do not support this operatoin
    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_s32 TdeHalNodeSetResize(TDE_HWNode_S *pHWNode, TDE_FILTER_OPT *pstFilterOpt, TDE_NODE_SUBM_TYPE_E enNodeType)
{
    TDE_FUN_IN;

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

#define IS_FMT_MASK_DST(mask) ((mask) & (1 << 4))
#define IS_FMT_MASK_SRC1(mask) ((mask) & (1 << 1))
#define IS_FMT_MASK_SRC2(mask) ((mask) & (1 << 2))
#define IS_FMT_MASK_SRC3(mask) ((mask) & (1 << 3))

static color_space_t TdeAriaConvertSpcae(TDE_COLORFMT_CATEGORY_E cc)
{
    color_space_t ret_cs;
    ret_cs = GRAY_COLOR_SPACE;

    if (cc == TDE_COLORFMT_CATEGORY_ARGB)
    {
        ret_cs = RGB_COLOR_SPACE;

    }
    else if (cc == TDE_COLORFMT_CATEGORY_YCbCr || cc == TDE_COLORFMT_CATEGORY_MB)
    {
        ret_cs = YUV_COLOR_SPACE;
    }
    else
    {
    //
    }

    return ret_cs;
}

typedef struct
{
    mt_u32 src1_cc_mod;
    mt_u32 src2_cc_mod;
    mt_u32 src3_cc_mod;
    mt_u32 dst_cc_mod;
} ARIA_COLOR_CONVERT_S;

enum {
    ARTA_TDE_NO_CC = 0,
    ARTA_TDE_RGB_2_YUV = 2,
    ARIA_TDE_YUV_2_RGB = 3,
};

static MT_BOOL TdeAriaColorConvertModGet(TDE_DRV_CONV_MODE_CMD_S *pConv, ARIA_COLOR_CONVERT_S *pcc_mod)
{
    MT_BOOL ret = MT_FALSE;

    color_space_t src1_cc = GRAY_COLOR_SPACE;
    color_space_t src2_cc = GRAY_COLOR_SPACE;
    color_space_t src3_cc = GRAY_COLOR_SPACE;
    color_space_t dst_cc = GRAY_COLOR_SPACE;

    mt_u32 mask = pConv->fmt_mask;
    if (IS_FMT_MASK_DST(mask))
    {
        dst_cc = TdeAriaConvertSpcae(pConv->dst_fmt);
    }

    if (IS_FMT_MASK_SRC1(mask))
    {
        src1_cc = TdeAriaConvertSpcae(pConv->src1_fmt);
    }

    if (IS_FMT_MASK_SRC2(mask))
    {
        src2_cc = TdeAriaConvertSpcae(pConv->src2_fmt);
    }

    if (IS_FMT_MASK_SRC3(mask))
    {
        src3_cc = TdeAriaConvertSpcae(pConv->src3_fmt);
    }

    if (dst_cc == GRAY_COLOR_SPACE)
    {
        return ret;
    }

    if (src1_cc == YUV_COLOR_SPACE)
    {
        if (dst_cc == RGB_COLOR_SPACE) {
            pcc_mod->src1_cc_mod = ARIA_TDE_YUV_2_RGB;
            ret = MT_TRUE;
       }
    }

 //   src2_cc = src2_cc;
 //   src3_cc = src3_cc;

    return ret;
}

mt_s32 TdeHalNodeSetColorConvert(TDE_HWNode_S *pHWNode, TDE_DRV_CONV_MODE_CMD_S *pConv)
{
    REG_SRC1_FMT_CFG0 src1_cfg0;
    REG_SRC2_FMT_CFG0 src2_cfg0;
    //REG_SRC3_FMT_CFG0  src3_cfg0;
    REG_DST0_FMT_CFG0 dts0_cfg0;

    ARIA_COLOR_CONVERT_S cc_mod = { 0 };

    TDE_FUN_IN;
    //TDE_LOG("PARA: [%x][%x][%x][%x]\n", pConv->bInConv, pConv->bOutConv, pConv->bInSrc1Conv, pConv->bInRGB2YC);
    TDE_LOG("PARA: [%x][%x][%x][%x] [%x]\n", pConv->src1_fmt, pConv->src2_fmt, pConv->src3_fmt, pConv->dst_fmt, pConv->fmt_mask);

    src1_cfg0.all = TdeNodeGetData(pHWNode, SRC1_FMT_CFG0);
    src2_cfg0.all = TdeNodeGetData(pHWNode, SRC2_FMT_CFG0);
    //src3_cfg0.all = TdeNodeGetData(pHWNode, SRC3_FMT_CFG0);
    dts0_cfg0.all = TdeNodeGetData(pHWNode, DST0_FMT_CFG0);

    if (!TdeAriaColorConvertModGet(pConv, &cc_mod))
    {
        TDE_FUN_OUT;
        return MT_SUCCESS;
    }

    src1_cfg0.bitc.src1_csc_mod = cc_mod.src1_cc_mod;
    src2_cfg0.bitc.src2_csc_mod = cc_mod.src2_cc_mod;
    dts0_cfg0.bitc.dst0_csc_mod = cc_mod.dst_cc_mod;

    TdeNodeSetData(pHWNode, SRC1_FMT_CFG0, src1_cfg0.all);
    TdeNodeSetData(pHWNode, SRC2_FMT_CFG0, src2_cfg0.all);
    TdeNodeSetData(pHWNode, DST0_FMT_CFG0, dts0_cfg0.all);
#ifdef USE_NORMAL_GPE
    TDE_REG_WRITE(ARIA_TDE_REG_SRC1_FMT_CFG0, src1_cfg0.all);
    TDE_REG_WRITE(ARIA_TDE_REG_SRC2_FMT_CFG0, src2_cfg0.all);
    TDE_REG_WRITE(ARIA_TDE_REG_DST0_FMT_CFG0, dts0_cfg0.all);
#endif

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_void TdeHalNodeAddChild(TDE_HWNode_S *pHWNode, TDE_CHILD_INFO *pChildInfo)
{
    //TODO::
    TDE_FUN_IN;

    // aria tde hw do not support the child cmd fifo node
    //

    TDE_FUN_OUT;
    return;
}

mt_void TdeHalNodeSetMbMode(TDE_HWNode_S *pHWNode, TDE_DRV_MB_CMD_S *pMbCmd)
{
    TDE_FUN_IN;

    TDE_FUN_OUT;
    return;
}

// user define the csc param
mt_void TDeHalNodeSetCsc(TDE_HWNode_S *pHWNode, TDE2_CSC_OPT_S stCscOpt)
{
    TDE_FUN_IN;

    // TODO:
    //user define the  color space convert
    // now, do not support

    TDE_FUN_OUT;
    return;
}

//=========================================//
//
//
//        Gloable  concfig
//
//
//=========================================//

mt_s32 TdeHalSetDeflicerLevel(TDE_DEFLICKER_LEVEL_E eDeflickerLevel)
{
    s_eDeflickerLevel = eDeflickerLevel;
    return MT_SUCCESS;
}

mt_s32 TdeHalGetDeflicerLevel(TDE_DEFLICKER_LEVEL_E *pDeflicerLevel)
{
    *pDeflicerLevel = s_eDeflickerLevel;
    return MT_SUCCESS;
}

mt_s32 TdeHalSetAlphaThreshold(mt_u8 u8ThresholdValue)
{
    s_u8AlphaThresholdValue = u8ThresholdValue;

    return MT_SUCCESS;
}

mt_s32 TdeHalGetAlphaThreshold(mt_u8 *pu8ThresholdValue)
{
    *pu8ThresholdValue = s_u8AlphaThresholdValue;

    return MT_SUCCESS;
}

mt_s32 TdeHalSetAlphaThresholdState(MT_BOOL bEnAlphaThreshold)
{
    s_bEnAlphaThreshold = bEnAlphaThreshold;

    return MT_SUCCESS;
}

mt_s32 TdeHalGetAlphaThresholdState(MT_BOOL *pbEnAlphaThreshold)
{
    *pbEnAlphaThreshold = s_bEnAlphaThreshold;

    return MT_SUCCESS;
}

//=========================================//
//
//
//        HW flow  concfig
//
//
//=========================================//

mt_s32 TdeHalNodeExecute(mt_u32 u32NodePhyAddr, mt_u64 u64Update, MT_BOOL bAqUseBuff)
{
    //TDE_AQ_CTRL_U unAqCtrl;
    //TDE_HWNode_S *pHWNode = (TDE_HWNode_S *)wgetvrt(u32NodePhyAddr);

    TDE_FUN_IN;
    TDE_LOG("TdeHalNodeExecute addr: [%x][%x][%x]\n", (mt_u32)pHWNode, (mt_u32)u32NodePhyAddr, bAqUseBuff);

    /*tde is idle*/ /* CNcomment:TDE空闲*/
    if (TdeHalCtlIsIdleSafely())
    {
#if 0
    //init gra
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, 0x100); //clear
    //save address
    TDE_REG_WRITE(ARIA_TDE_REG_CMD_FIFO_ADDR_ASYNC, u32NodePhyAddr);

    //set regisiter
    //gpe_set_regs(pHWNode);

    //clear gra done status
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_STATE, GRA_ALL_DONE);
    //start graphic
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, 1);
#else
    if (0)
    {
        aria_tde_reg_clr_start();
    }

    aria_tde_gra_state_clear_all_done();

#ifdef USE_NORMAL_GPE
    TDE_LOG("TdeHalNodeExecute: GPE start\n");
    TDE_REG_WRITE(ARIA_TDE_REG_GRA_ENG_START, 1);
#else
    TDE_LOG("TdeHalNodeExecute: GPE cmdfifo start\n");
    aria_tde_gra_cmdfifo_read_end_check(MT_TRUE);

    aria_tde_async_cmdfifo_start(u32NodePhyAddr);
#endif

#endif

    /*modiyf the break off to finish current node when use the temp buffer */
    /* CNcomment:若需要使用临时buffer,
        则打断模式为当前节点完成打断,否则为当前节点
           当前行完成打断*/
    if (MT_TRUE == bAqUseBuff)
    {
        //unAqCtrl.stBits.u32AqOperMode = TDE_AQ_CTRL_COMP_LIST;
        TDE_TRACE(TDE_KERN_DEBUG, "Aq Ctrl use comp list mode\n");
    }
    else
    {
        // unAqCtrl.stBits.u32AqOperMode = TDE_AQ_CTRL_COMP_LINE;
        TDE_TRACE(TDE_KERN_DEBUG, "Aq Ctrl use comp node line mode\n");
    }

//        TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_AQ_CTRL, unAqCtrl.u32All);
#ifndef TDE_BOOT
    mb();
#endif
    /*start Aq list*/ /* CNcomment:启动Aq*/
    //        TDE_REG_WRITE(s_pu32BaseVirAddr, TDE_CTRL, 0x1);
    }
    else
    {
        TDE_FUN_OUT;
        return MT_FAILURE;
    }

#ifdef _TDE_HAL_REGISTERS_PIRNT_ON_
    if (1) // print the all registers for debug
    {
        mt_u32 tmp_data = 0;

        while (1)
        {
            tmp_data = TDE_REG_READ(ARIA_TDE_REG_SRC0_PIC_STRIDE);
            if (tmp_data == 0x123)
            {
                break;
            }

        // if(tmp_data == 0x999)
        if (aria_tde_gra_cmdfifo_read_end_check(MT_FALSE))
        {
            TdeHalAriaPrintAllReg();
            break;
            //TDE_REG_WRITE(ARIA_TDE_REG_SRC0_PIC_STRIDE, 0x9);
        }

        msleep(1);
        }
    }

#endif // _TDE_HAL_REGISTERS_PIRNT_ON_

    TDE_FUN_OUT;
    return MT_SUCCESS;
}

mt_u32 TdeHalCurNode(mt_void)
{
    mt_u32 u32Addr = 0;

    TDE_FUN_IN;
    //    u32Addr = TDE_REG_READ(s_pu32BaseVirAddr, TDE_AQ_ADDR);
    u32Addr = TDE_REG_READ(ARIA_TDE_REG_CMD_FIFO_ID0);
    TDE_LOG("PARA: %x\n", u32Addr);
    TDE_FUN_OUT;
    return u32Addr;
}

mt_void TdeHalResumeInit(mt_void)
{
//    TDE_FUN_IN;
    //TdeHalSetClock(MT_TRUE);

    TdeHalAriaSysReset(MT_TRUE);
    TdeHalInitIrqMask();

    TdeHalCtlReset();

    //TdeHalInitQueue();

    TDE_FUN_OUT;
    return;
}

mt_void TdeHalSuspend(mt_void)
{
    TDE_FUN_IN;
    //TdeHalSetClock(MT_FALSE);
    TDE_FUN_OUT;
    return;
}

mt_u32 TdeHalReadReg(mt_u32 offset)
{
    mt_u32 RegVal = 0;

    RegVal = TDE_REG_READ(offset);

    return RegVal;
}
