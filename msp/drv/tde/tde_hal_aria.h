/*****************************************************************************
*             Copyright 2006 - 2014, Montage Tech. Co., Ltd.
*                           ALL RIGHTS RESERVED
* FileName: tde_osictl.c
* Description:  TDE osi ctl
*
* History:
* Version   Date          Author        DefectNum       Description
*
*****************************************************************************/


#ifndef __TDE_HAL_ARIA_H__
#define __TDE_HAL_ARIA_H__






typedef union 
{
    u32 all;
    struct
    {
        u32 cmd_end                  : 1;
        u32 next_node               : 1;
        u32 suspend2                 : 1;
        u32                                 : 5;
        u32 reg                           : 10;
        u32                                 : 13;
        u32 used                        : 1;
    } bitc;
} reg_base_t;


typedef mt_u32  reg_data_t;


typedef  struct 
{
   reg_base_t      base;
   reg_data_t      data;
}ARIA_TDE_CMD_t;


typedef struct mtARIA_TDE_HWNODE_t
{
		//ARIA_TDE_CMD_t     GRA_ALL_START        ;
		ARIA_TDE_CMD_t     GRA_EN                      ;
		//ARIA_TDE_CMD_t     GRA_ENG_CFG           ;
		//ARIA_TDE_CMD_t     GRA_CORE_DONE        ;           
		ARIA_TDE_CMD_t     SRC0_FMT_CFG0        ;
		ARIA_TDE_CMD_t     SRC0_FMT_CFG1        ;
		ARIA_TDE_CMD_t     SRC0_PIC_ADDR        ;
		ARIA_TDE_CMD_t     SRC0_PIC_STRIDE      ;            
		ARIA_TDE_CMD_t     SRC1_FMT_CFG0        ;
		ARIA_TDE_CMD_t     SRC1_FMT_CFG1        ;
		ARIA_TDE_CMD_t     SRC1_KEY_MIN         ;
		ARIA_TDE_CMD_t     SRC1_KEY_MAX         ;
		ARIA_TDE_CMD_t     SRC1_PIC_ADDR        ;
		ARIA_TDE_CMD_t     SRC1_PIC_STRIDE      ;
		ARIA_TDE_CMD_t     SRC1_PIC_SIZE        ;
		ARIA_TDE_CMD_t     SRC1_OP_SIZE         ;
		ARIA_TDE_CMD_t     SRC1_OP_POS          ;
		ARIA_TDE_CMD_t     SRC1_CMYK_CFG        ;
		//ARIA_TDE_CMD_t     SRC1_STATUS          ;        
		ARIA_TDE_CMD_t     SRC2_FMT_CFG0        ;
		ARIA_TDE_CMD_t     SRC2_FMT_CFG1        ;
		ARIA_TDE_CMD_t     SRC2_KEY_MIN         ;
		ARIA_TDE_CMD_t     SRC2_KEY_MAX         ;
		ARIA_TDE_CMD_t     SRC2_PIC_ADDR        ;
		ARIA_TDE_CMD_t     SRC2_PIC_STRIDE      ;
		ARIA_TDE_CMD_t     SRC2_OP_POS          ;
		//ARIA_TDE_CMD_t     SRC2_STATUS          ;           
		ARIA_TDE_CMD_t     SRC3_FMT_CFG0        ;
		ARIA_TDE_CMD_t     SRC3_FMT_CFG1        ;
		ARIA_TDE_CMD_t     SRC3_KEY_MIN         ;
		ARIA_TDE_CMD_t     SRC3_KEY_MAX         ;
		ARIA_TDE_CMD_t     SRC3_PIC_ADDR        ;
		ARIA_TDE_CMD_t     SRC3_PIC_STRIDE      ;
		ARIA_TDE_CMD_t     SRC3_OP_POS          ;
		//ARIA_TDE_CMD_t     SRC3_STATUS          ;              
		ARIA_TDE_CMD_t     DST0_FMT_CFG0        ;
		ARIA_TDE_CMD_t     DST0_FMT_CFG1        ;
		ARIA_TDE_CMD_t     DST0_PIC_ADDR        ;
		ARIA_TDE_CMD_t     DST0_PIC_STRIDE      ;
		ARIA_TDE_CMD_t     DST0_OP_POS          ;                
		ARIA_TDE_CMD_t     DST1_FMT_CFG0        ;
		ARIA_TDE_CMD_t     DST1_FMT_CFG1        ;
		ARIA_TDE_CMD_t     DST1_PIC_ADDR        ;
		ARIA_TDE_CMD_t     DST1_PIC_STRIDE      ;               
		ARIA_TDE_CMD_t     DST1_OP_POS          ;              
		ARIA_TDE_CMD_t     DST_PIC_SIZE         ;
		ARIA_TDE_CMD_t     DST_OP_SIZE          ;
		//ARIA_TDE_CMD_t     DST_STATUS           ;                                               
		//ARIA_TDE_CMD_t     CMD_FIFO_CTRL        ;
		//ARIA_TDE_CMD_t     CMD_FIFO_TRIG_CFG    ;
		//ARIA_TDE_CMD_t     CMD_FIFO_ADDR_SYNC   ;
		//ARIA_TDE_CMD_t     CMD_FIFO_ADDR_ASYNC  ;
       //ARIA_TDE_CMD_t     CMD_ID0              ;
		//ARIA_TDE_CMD_t     CMD_ID1              ;
		//ARIA_TDE_CMD_t     CMD_FIFO_STATUS      ;                  
		//ARIA_TDE_CMD_t     GRA_AXI_CTRL         ;
		//ARIA_TDE_CMD_t     GRA_AXI_ATATUS       ;                
		ARIA_TDE_CMD_t     XYLC_CFG             ;
		//ARIA_TDE_CMD_t     XYLC_ERR             ;
		//ARIA_TDE_CMD_t     XYLC_STATUS          ;                 
		ARIA_TDE_CMD_t     SCALER_CFG           ;
		ARIA_TDE_CMD_t     SCALER_COEF_11       ;
		ARIA_TDE_CMD_t     SCALER_COEF_21       ;
		ARIA_TDE_CMD_t     SCALER_COEF_31       ;
		ARIA_TDE_CMD_t     SCALER_COEF_22       ;
		ARIA_TDE_CMD_t     SCALER_COEF_23       ;
		ARIA_TDE_CMD_t     SCALER_INIT_PHASE    ;
		//ARIA_TDE_CMD_t     SCALER_STATUS        ;                        
		ARIA_TDE_CMD_t     ROT_PAT_CFG          ;
		ARIA_TDE_CMD_t     PAT_COLOR            ;
		ARIA_TDE_CMD_t     PAT_OFFSET_POS       ;
		ARIA_TDE_CMD_t     PAT_RATIO_X_0        ;
		ARIA_TDE_CMD_t     PAT_RATIO_X_1        ;
		ARIA_TDE_CMD_t     PAT_RATIO_Y_0        ;
		ARIA_TDE_CMD_t     PAT_RATIO_Y_1        ;                 
		ARIA_TDE_CMD_t     GRADT_CFG            ;
		ARIA_TDE_CMD_t     GRADT_X_STEP         ;
		ARIA_TDE_CMD_t     GRADT_Y_STEP         ;
		ARIA_TDE_CMD_t     GRADT_START_V        ;
		ARIA_TDE_CMD_t     STOP0_ARGB           ;
		ARIA_TDE_CMD_t     STOP1_ARGB           ;
		ARIA_TDE_CMD_t     STOP2_ARGB           ;
		ARIA_TDE_CMD_t     STOP3_ARGB           ;
		ARIA_TDE_CMD_t     STOP_OFFSET          ;
		ARIA_TDE_CMD_t     STOP0_FACT           ;
		ARIA_TDE_CMD_t     STOP1_FACT           ;
		ARIA_TDE_CMD_t     STOP2_FACT           ;                    
		ARIA_TDE_CMD_t     COMP_CFG             ;
		ARIA_TDE_CMD_t     COMP_MULT_MOD        ;
		ARIA_TDE_CMD_t     COMP_BLD_MOD         ;
		ARIA_TDE_CMD_t     ROP_ID               ;
		ARIA_TDE_CMD_t     ROP_PAT              ;
		//ARIA_TDE_CMD_t     COMP_STATUS    ;                         
		ARIA_TDE_CMD_t     LOAD_EN              ;
		ARIA_TDE_CMD_t     PAL_SIZE             ;
		ARIA_TDE_CMD_t     PAL1_ADDR            ;
		ARIA_TDE_CMD_t     PAL3_ADDR            ;
		ARIA_TDE_CMD_t     COEF_ADDR            ;
		//ARIA_TDE_CMD_t     CTRL_STATUS          ;                     
		ARIA_TDE_CMD_t     SRC1_TILE_CFG        ;
		ARIA_TDE_CMD_t     SRC1_TILE_JMP00      ;
		ARIA_TDE_CMD_t     SRC1_TILE_JMP01      ;
		ARIA_TDE_CMD_t     SRC1_TILE_JMP10      ;
		ARIA_TDE_CMD_t     SRC1_TILE_JMP11      ;                 
		//ARIA_TDE_CMD_t     GRA_INT_EN           ;
		//ARIA_TDE_CMD_t     GRA_INT_STATE        ;
		//ARIA_TDE_CMD_t     GRA_STATE            ;
		//ARIA_TDE_CMD_t     GRA_INT_MOD          ;    
#if 0		
		ARIA_TDE_CMD_t     GRP0_CSCP_0          ;
		ARIA_TDE_CMD_t     GRP0_CSCP_1          ;
		ARIA_TDE_CMD_t     GRP0_CSCP_2          ;
		ARIA_TDE_CMD_t     GRP0_CSCP_3          ;
		ARIA_TDE_CMD_t     GRP0_CSCP_4          ;
		ARIA_TDE_CMD_t     GRP0_CSCDC_0         ;
		ARIA_TDE_CMD_t     GRP0_CSCDC_1         ;
		ARIA_TDE_CMD_t     GRP0_CSCDC_2         ;
		ARIA_TDE_CMD_t     GRP1_CSCP_0          ;
		ARIA_TDE_CMD_t     GRP1_CSCP_1          ;
		ARIA_TDE_CMD_t     GRP1_CSCP_2          ;
		ARIA_TDE_CMD_t     GRP1_CSCP_3          ;
		ARIA_TDE_CMD_t     GRP1_CSCP_4          ;
		ARIA_TDE_CMD_t     GRP1_CSCDC_0         ;
		ARIA_TDE_CMD_t     GRP1_CSCDC_1         ;
		ARIA_TDE_CMD_t     GRP1_CSCDC_2         ;               
		ARIA_TDE_CMD_t     GRADT_RADIUS_A_0     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_A_1     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_B_0     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_B_1     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_C_0     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_C_1     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_D_0     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_D_1     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_E_0     ;
		ARIA_TDE_CMD_t     GRADT_RADIUS_E_1     ;                 
		ARIA_TDE_CMD_t     GRA_PIN_SEL          ;
		ARIA_TDE_CMD_t     GRA_MAC_SET          ;
		ARIA_TDE_CMD_t     GRA_REQ_CFG          ;      
 #endif
    
       ARIA_TDE_CMD_t     CMD_FIFO_ID0              ;        // current node addr
		ARIA_TDE_CMD_t     CMD_FIFO_ID1              ;        //  current node handle
    	ARIA_TDE_CMD_t     CMD_FIFO_ADDR_ASYNC  ;    // next node addr
}ARIA_TDE_HWNODE_t;

typedef ARIA_TDE_HWNODE_t TDE_HWNode_S;



mt_void TdeHalHwNodeInitAriaNode(ARIA_TDE_HWNODE_t *p_hw_node);

mt_void TdeHalHwNodeSetNext(ARIA_TDE_HWNODE_t *p_hw_node, mt_u32 addr);

mt_void TdeHalHwNodeSetID(ARIA_TDE_HWNODE_t *p_hw_node, mt_u32 addr, mt_u32 handle);


#endif    //  __TDE_HAL_ARIA_H__


