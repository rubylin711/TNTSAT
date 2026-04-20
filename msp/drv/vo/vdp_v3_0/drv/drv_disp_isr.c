
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_x.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/
#include <linux/sizes.h>	/*SZ_1K*/

#include <linux/compiler.h>
#if defined(CONFIG_MT_CHIP_ARIA)
#include <mach/aria_io.h>
#else
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/irq.h"
#endif

#include "drv_disp_isr.h"
#include "drv_disp_osal.h"
#include "mt_drv_sys.h"
#include "drv_display.h"
#include "hd_enc_aria_reg.h"
#include "sd_enc_aria_reg.h"
#include "mt_mach/chipinfo.h"

#if defined(CONFIG_MT_CHIP_ARIA)
#include "MT_DF_Aria_reg.h"
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
#include "drv_disp_Symphony_reg.h"
#endif
#include "mt_module_debug.h"


#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

static mt_s32 s_DispISRMngrInitFlag = -1;
static DISP_ISR_M_S s_DispISRMngr;

#define DispCheckNullPointer(ptr) \
    {                                \
        if (!ptr)                    \
        {                            \
            DISP_ERROR("DISP ERROR! Input null pointer in %s!\n", __FUNCTION__); \
            return MT_ERR_DISP_NULL_PTR;  \
        }                             \
    }

mt_s32 DISP_ISR_SwitchIntterrup(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType, MT_BOOL bEnable)
{
#if 0 //disp_hal
    DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    //printk("Open enDisp=%d int %d\n", enDisp, eType);
    DispCheckNullPointer( pfOpt);
    DispCheckNullPointer( pfOpt->PF_SetIntEnable);
    switch(eType)
    {
        case MT_DRV_DISP_C_INTPOS_0_PERCENT:
            if (MT_DRV_DISPLAY_0 ==  enDisp)
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_D0_0_PERCENT, bEnable);
            }
            else if (MT_DRV_DISPLAY_1 ==  enDisp)
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_D1_0_PERCENT, bEnable);
            }
            break;
        case MT_DRV_DISP_C_INTPOS_90_PERCENT:
            if (MT_DRV_DISPLAY_0 ==  enDisp)
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_D0_90_PERCENT, bEnable);
            }
            else if (MT_DRV_DISPLAY_1 ==  enDisp)
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_D1_90_PERCENT, bEnable);
            }
            break;
        case MT_DRV_DISP_C_INTPOS_100_PERCENT:
            if (MT_DRV_DISPLAY_0 ==  enDisp)
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_D0_100_PERCENT, bEnable);
            }
            else if (MT_DRV_DISPLAY_1 ==  enDisp)
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_D1_100_PERCENT, bEnable);
            }
            break;
        case MT_DRV_DISP_C_DHD0_WBC:
            {
                pfOpt->PF_SetIntEnable((mt_u32)DISP_INTERRUPT_WBCDHD_PARTFNI, bEnable);
            }
        default :
            break;
    }
#endif
    return MT_SUCCESS;
}


mt_s32 DISP_ISR_ResetChn(MT_DRV_DISPLAY_E enDisp)
{
    DISP_ISR_CHN_S *pstChn;

    if (s_DispISRMngrInitFlag >= 0)
    {
        return MT_FAILURE;
    }

    pstChn = &s_DispISRMngr.stDispChn[enDisp];

    DISP_MEMSET(pstChn, 0, sizeof(DISP_ISR_CHN_S));

    pstChn->enDisp = enDisp;
    pstChn->stCBInfo.eEventType = MT_DRV_DISP_C_EVET_NONE;

    pstChn->bEnable = MT_FALSE;

    return MT_SUCCESS;
}



mt_s32 DISP_ISR_Init(mt_void)
{
    MT_DRV_DISPLAY_E enDisp;

    if (s_DispISRMngrInitFlag >= 0)
    {
        return MT_SUCCESS;
    }

    DISP_MEMSET(&s_DispISRMngr, 0, sizeof(DISP_ISR_M_S));

    for (enDisp=MT_DRV_DISPLAY_0; enDisp<MT_DRV_DISPLAY_BUTT; enDisp++)
    {
        DISP_ISR_ResetChn(enDisp);
    }

    s_DispISRMngrInitFlag++;

    return MT_SUCCESS;
}


mt_s32 DISP_ISR_DeInit(mt_void)
{
    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_SUCCESS;
    }

    DISP_MEMSET(&s_DispISRMngr, 0, sizeof(DISP_ISR_M_S));

    s_DispISRMngrInitFlag--;

    return MT_SUCCESS;
}

mt_s32 DISP_ISR_Suspend(mt_void)
{
#if 0 //disp_hal
    DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_FAILURE;
    }
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer( pfOpt->PF_GetIntSetting);

    pfOpt->PF_GetIntSetting(&s_DispISRMngr.u32IntMaskSave4Suspend);
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_ISR_Resume(mt_void)
{
#if 0 //disp_hal
    DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_FAILURE;
    }
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_SetIntEnable);

    pfOpt->PF_SetIntEnable(s_DispISRMngr.u32IntMaskSave4Suspend, MT_TRUE);
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_ISR_OpenChn(MT_DRV_DISPLAY_E enDisp)
{
    if ( (s_DispISRMngrInitFlag < 0) || (enDisp >= MT_DRV_DISPLAY_BUTT))
    {
        return MT_FAILURE;
    }

    if (s_DispISRMngr.stDispChn[enDisp].bEnable == MT_TRUE)
    {
        return MT_SUCCESS;
    }

    DISP_ISR_ResetChn(enDisp);

    s_DispISRMngr.stDispChn[enDisp].bEnable = MT_TRUE;

    s_DispISRMngr.u32ChnNumber++;

    return MT_SUCCESS;
}

mt_s32 DISP_ISR_CloseChn(MT_DRV_DISPLAY_E enDisp)
{
    if ( (s_DispISRMngrInitFlag < 0) || (enDisp >= MT_DRV_DISPLAY_BUTT))
    {
        return MT_FAILURE;
    }

    if (s_DispISRMngr.stDispChn[enDisp].bEnable == MT_FALSE)
    {
        return MT_SUCCESS;
    }

    s_DispISRMngr.stDispChn[enDisp].bEnable = MT_FALSE;

    s_DispISRMngr.u32ChnNumber--;

    return MT_SUCCESS;
}

mt_s32 DISP_ISR_SearchNode(DISP_ISR_CHN_S *pstChn, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                           MT_DRV_DISP_CALLBACK_S *pstCB)
{
    mt_u32 u, v;

    v = pstChn->stList[eType].u32NodeFlag;

    for (u = 0; u < DEF_DISP_ISR_LIST_LENGTH && v; u++)
    {
        if (    ( v & (1<<u) )
            && (pstChn->stList[eType].stNode[u].pfDISP_Callback == pstCB->pfDISP_Callback)
            && (pstChn->stList[eType].stNode[u].hDst== pstCB->hDst)
            )
        {
            return (mt_s32)u;
        }
    }

    return -1;
}

mt_s32 DISP_ISR_SearchNullNode(DISP_ISR_CHN_S *pstChn, MT_DRV_DISP_CALLBACK_TYPE_E eType)
{
    mt_s32 i, v;

    for (i=0; i<DEF_DISP_ISR_LIST_LENGTH; i++)
    {
        v = 1 << i;
        if (!(pstChn->stList[eType].u32NodeFlag & v) )
        {
            return i;
        }
    }

    return -1;
}


typedef enum tagDISP_INTERRUPT_E
{
    DISP_INTERRUPT_NONE = 0,
    DISP_INTERRUPT_D0_0_PERCENT  = 0x10,
    DISP_INTERRUPT_D0_90_PERCENT = 0x20,
    DISP_INTERRUPT_D0_100_PERCENT = 0x40,
    DISP_INTERRUPT_D0_UFINT      = 0x80,

    DISP_INTERRUPT_D1_0_PERCENT  = 0x1,
    DISP_INTERRUPT_D1_90_PERCENT = 0x2,
    DISP_INTERRUPT_D1_100_PERCENT = 0x4,
    DISP_INTERRUPT_D1_UFINT      = 0x8,

    DISP_INTERRUPT_WBCDHD_PARTFNI  = 0x10000000,

    DISP_INTERRUPT_GP1_RES       = 0x100,
    DISP_INTERRUPT_MC1_UFINT     = 0x200,

    DISP_INTERRUPT_ALL           = 0xFFFFFFFF,
}DISP_INTERRUPT_E;

static mt_u32 s_DispIntTable[MT_DRV_DISPLAY_BUTT][MT_DRV_DISP_C_TYPE_BUTT] =
{
//NONE, SHOW_MODE, INTPOS_0_PERCENT, INTPOS_90_PERCENT,       GFX_WBC, REG_UP
{0, DISP_INTERRUPT_D0_0_PERCENT, DISP_INTERRUPT_D0_90_PERCENT,DISP_INTERRUPT_D0_100_PERCENT, 0, 0,0},
{0, DISP_INTERRUPT_D1_0_PERCENT, DISP_INTERRUPT_D1_90_PERCENT, DISP_INTERRUPT_D1_100_PERCENT,DISP_INTERRUPT_WBCDHD_PARTFNI, 0,0},
{0, 0, 0, 0, 0,0,0},
};

mt_s32 DISP_ISR_RegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                            MT_DRV_DISP_CALLBACK_S *pstCB)
{
    DISP_ISR_CHN_S *pstChn;
    mt_u32 u32NodeFlagNew;
    mt_s32 index;
    mt_u32 irq_mask_state;
    
    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_FAILURE;
    }
    DispCheckID(enDisp);

    //add because not_need
    if(enDisp != MT_DRV_DISPLAY_1)
    {
        DISP_DEBUGK("%s %d this isr is not needed \n",__FUNCTION__,__LINE__);
        return MT_SUCCESS;
    }

    irq_mask_state = reg_aria_hd_encoder_get_mask_irq();
    reg_aria_hd_encoder_set_mask_irq(0xff000000); //mask disp irq
    pstChn = &s_DispISRMngr.stDispChn[enDisp];
    if (pstChn->bEnable != MT_TRUE)
    {
        DISP_ERROR("DISP %d is not add to ISR manager!\n", enDisp);
        return MT_FAILURE;
    }

    //printk("DISP_ISR_RegCallback  disp=%d, type=%d\n", enDisp, eType);
    index = DISP_ISR_SearchNullNode(pstChn, eType);
    if (index < 0)
    {
        DISP_ERROR("DISP %d  callback reach max number!\n", enDisp);
        return MT_FAILURE;
    }

    // record callback info
    pstChn->stList[eType].stNode[index].hDst = pstCB->hDst;
    pstChn->stList[eType].stNode[index].pfDISP_Callback = pstCB->pfDISP_Callback;

    //update display channel node flag
    u32NodeFlagNew = pstChn->stList[eType].u32NodeFlag;
    u32NodeFlagNew = u32NodeFlagNew | (1 << index);
    pstChn->stList[eType].u32NodeFlag = u32NodeFlagNew;
    reg_aria_hd_encoder_set_mask_irq(irq_mask_state); //restore irq mask
    
    DISP_DEBUGK("[%s] line %d, idx %d,etype %d,node %x\n",__FUNCTION__,__LINE__,index,eType,pstChn->stList[eType].u32NodeFlag);

    // enable interrupt
    if (pstChn->stList[eType].u32NodeFlag)
    {
        DISP_ISR_SwitchIntterrup(enDisp, eType, MT_TRUE);
    }

    return MT_SUCCESS;
}

mt_s32 DISP_ISR_UnRegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
                              MT_DRV_DISP_CALLBACK_S *pstCB)
{
    DISP_ISR_CHN_S *pstChn;
    mt_u32 u32NodeFlagNew;
    mt_s32 index;
    mt_u32 irq_mask_state;

    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_FAILURE;
    }

    DispCheckID(enDisp);

    //add because not_need
    if(enDisp != MT_DRV_DISPLAY_1)
    {
        DISP_DEBUGK("%s %d this isr is not needed \n",__FUNCTION__,__LINE__);
        return MT_SUCCESS;
    }

    irq_mask_state = reg_aria_hd_encoder_get_mask_irq();
    reg_aria_hd_encoder_set_mask_irq(0xff000000);  //mask disp irq

    pstChn = &s_DispISRMngr.stDispChn[enDisp];
#if  0
    if (pstChn->bEnable != MT_TRUE)
    {
        DISP_ERROR("DISP %d is not add to ISR manager!\n", enDisp);
        return MT_FAILURE;
    }
#endif
    index = DISP_ISR_SearchNode(pstChn, eType, pstCB);
    if (index < 0)
    {
        DISP_ERROR("Callback is not exist!\n");
        return MT_FAILURE;
    }

    // update node flag
    u32NodeFlagNew = pstChn->stList[eType].u32NodeFlag;
    u32NodeFlagNew = u32NodeFlagNew & (~(1 << index));
    pstChn->stList[eType].u32NodeFlag = u32NodeFlagNew;

    /* clear node record */
    pstChn->stList[eType].stNode[index].pfDISP_Callback = MT_NULL;
    pstChn->stList[eType].stNode[index].hDst = MT_NULL;
    reg_aria_hd_encoder_set_mask_irq(irq_mask_state);  //restore irq mask
    DISP_DEBUGK("[%s] line %d, idx %d,etype %d,node %x\n",__FUNCTION__,__LINE__,index,eType,pstChn->stList[eType].u32NodeFlag);

    if (!pstChn->stList[eType].u32NodeFlag)
    {
        DISP_ISR_SwitchIntterrup(enDisp, eType, MT_FALSE);
        //printk("close int\n");
    }

    return MT_SUCCESS;
}

mt_s32 DISP_ISR_SetEvent(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_EVENT_E eEvent)
{
    DISP_ISR_CHN_S *pstChn;

    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_FAILURE;
    }

    pstChn = &s_DispISRMngr.stDispChn[enDisp];
    if (pstChn->bEnable != MT_TRUE)
    {
        DISP_WARN("DISP %d is not add to ISR manager!\n", enDisp);
        return MT_FAILURE;
    }

    pstChn->stCBInfo.eEventType = eEvent;

    return MT_SUCCESS;
}

mt_s32 DISP_ISR_SetDispInfo(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstDispInfo)
{
    DISP_ISR_CHN_S *pstChn;

    if (s_DispISRMngrInitFlag < 0)
    {
        return MT_FAILURE;
    }

    pstChn = &s_DispISRMngr.stDispChn[enDisp];
    if (pstChn->bEnable != MT_TRUE)
    {
        DISP_ERROR("DISP %d is not add to ISR manager!\n", enDisp);
        return MT_FAILURE;
    }

    pstChn->stCBInfo.stDispInfo = *pstDispInfo;

    return MT_SUCCESS;
}

//#define DEF_DISP_ISR_Main_RETURN_VALUE MT_SUCCESS
#define DEF_DISP_ISR_Main_RETURN_VALUE IRQ_HANDLED

static mt_u32 s_DebugMyIntCount = 0;
mt_u32 s_DebugTopIntCount = 0;
static mt_u32 s_DebugG1IntCount = 0;
static mt_u32 s_DebugG0IntCount = 0;
static mt_u32 s_DebugBotIntCount = 0;
//static mt_u32 s_DebugsdTopIntCount = 0;
//static mt_u32 s_DebugsdG1IntCount = 0;
//static mt_u32 s_DebugsdG0IntCount = 0;
static mt_u32 s_DebugsdBotIntCount = 0;
extern volatile mt_u32 *p_intcnt;

/* Disp ISR Debug: enable debug Display ISR for some long time issue */
int debug_disp_isr = 0;
module_param(debug_disp_isr, int, S_IRUGO | S_IWUSR);

#define DEF_DEBUG_DISP_INT_MAX_NUMBER 60

static void aria_disp_get_vbi_data_size(mt_s32 *vbi_data)
{
    disp_vbi_buffer_s *p_disp_vbi_buffer = &disp_vbi_buffer;

    if(p_disp_vbi_buffer->vbi_wp >= p_disp_vbi_buffer->vbi_rp)
    {
        *vbi_data = (p_disp_vbi_buffer->vbi_wp - p_disp_vbi_buffer->vbi_rp);
    }
    else
    {
        *vbi_data = (VBI_DATA_BUF_LEN - p_disp_vbi_buffer->vbi_rp + p_disp_vbi_buffer->vbi_wp);
    }
}

irqreturn_t aria_disp_isr_PS_end(mt_s32 irq, mt_void *dev_id)
{
#ifdef CONFIG_EMU
    MT_INFO_VO("PS ISR\n");
#endif
#ifdef PS_TEST//PS test
    reg_aria1_disp_set_pres_irq_pres_end_irq(1);
    MT_INFO_VO("PS ISR\n");
#elif defined(CONFIG_MT_CHIP_ARIA)
    PS_Loop();
#endif
    return IRQ_HANDLED;
}

#ifdef CABLE_DETECT
#if CABLE_DETECT
extern void aria_disp_isr_cable_detect(const reg_aria_hdvenc_mask_riq_t state);
#endif
#endif
//default isr grop0s
irqreturn_t aria_disp_isr_group0(mt_s32 irq, mt_void *dev_id)
{
    reg_aria_hdvenc_mask_riq_t state;
    mt_u32 value = 0;
    MT_DRV_DISPLAY_E enDisp = MT_DRV_DISPLAY_BUTT;
    MT_DRV_DISP_CALLBACK_TYPE_E eIntType = MT_DRV_DISP_C_TYPE_BUTT;
    DISP_ISR_CHN_S *pstDisp = NULL;
    mt_u32 u32IntState = 0x0;//should get from hw_mask:pfOpt->PF_GetMaskedIntState
    mt_u32 n = 0, v = 0;
    mt_u32 interlace = 0;
    mt_u32 value2 = 0;
		
    state.all = HAL_GET_U32((volatile u32 *)(REG_ARIA_HD_ENCODER_BASE+0xf4));//group 0 status

    //value = value & 0xf;
    value = state.all >> 16;
    interlace = HAL_GET_U32((volatile u32*)REG_ARIA_HD_ENCODER_HD_CFG_INFO) & 0x1;
    value2 = HAL_GET_U32((volatile u32*)(REG_ARIA_HD_ENCODER_BASE+0xf4)) >>16;

	//BUG
    if (((value & 0xf) & (value2 & 0xf)) != 0)
    {
		printk(KERN_ERR "\r\n *[BUG]aria_disp_isr_group0: %08x, %08x \r\n", value, value2);
	}

    //s_DebugG0IntCount++;
    //if(value&0x7)
    s_DebugTopIntCount++;
    p_intcnt[0]=s_DebugG0IntCount;
    //ISR_FUNCTION();
    p_intcnt[1]=s_DebugTopIntCount;
    p_intcnt[2]=value;

    if( (value & 0x8) == 0x8 || (value & 0x4) == 0x4 )  //top_start and bot_start
    {
        u32IntState = 1;
    	//debug
    	if (debug_disp_isr)
    	{
    		if ((s_DebugG0IntCount++ % DEF_DEBUG_DISP_INT_MAX_NUMBER) == 0)
    		{
    			printk(KERN_EMERG "\r\n aria_disp_isr_group0: %u\r\n", s_DebugG0IntCount);
    		}
    	} 
    }
   
    if( (value & 0x2) == 0x2 )  //top_field
    {
        if(interlace)
            u32IntState |= 2;
        else
            u32IntState |= 6;
    }
    if( (value & 0x1) == 0x1 )  //bop_field
    {
        u32IntState |= 4;
    }
#if 0
    if(HAL_GET_U32((volatile u32*)REG_4K_DISP_VIDEO_CTRL_1) & (0x1 << 27))  //fpga verify --- dean
    {
        printk("%x %x %x %x\n", value, interlace, u32IntState, state.all);
    }
#endif     
//printk("\r\n u32IntState:0x%08x, value:0x%08x, i:%d", u32IntState, value, interlace);
//printk("ISR: %x\n", value);
#if 0 //disp_hal
    DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    // if get int ops failed, return
    DispCheckNullPointer( pfOpt);
    DispCheckNullPointer( pfOpt->PF_GetMaskedIntState);
    DispCheckNullPointer( pfOpt->PF_CleanIntState);
    DispCheckNullPointer( pfOpt->FP_GetChnBottomFlag);

    // s1 get interrupt state
    pfOpt->PF_GetMaskedIntState(&u32IntState);

    // s2 clear interrupt state
    pfOpt->PF_CleanIntState(u32IntState);
#endif
    s_DebugMyIntCount++;
    /*
    if (s_DebugMyIntCount < DEF_DEBUG_DISP_INT_MAX_NUMBER)
    {
    printk("<I>");
    }
    */

    //return DEF_DISP_ISR_Main_RETURN_VALUE;

    // s3 check and recode underload interrupt

    // s5 process interrupt one by one
    //printk("[0x%x]", u32IntState);


    // s5.0 if display is not open, return
    if (!s_DispISRMngr.u32ChnNumber)
    {
        //means no isr callback function so do not need excute
        //return 0;
    }

    //DISP_DEBUGK("DISP DISP_ISR_Main_VB \n");
    {
        enDisp = MT_DRV_DISPLAY_1;
        pstDisp = &s_DispISRMngr.stDispChn[enDisp];
        DISP_ISR_SetEvent(enDisp, MT_DRV_DISP_C_VT_INT);

        if (pstDisp->bEnable != MT_TRUE)
        {
            //break;
        }

        for (eIntType=MT_DRV_DISP_C_INTPOS_0_PERCENT; eIntType<MT_DRV_DISP_C_TYPE_BUTT; eIntType++)
        {
            //MT_BOOL bBtm;
            //mt_u32 vcnt;

            if(!(s_DispIntTable[enDisp][eIntType] & u32IntState) )
            {
                continue;
            }

            if( s_DispIntTable[enDisp][MT_DRV_DISP_C_INTPOS_0_PERCENT] & u32IntState)
            {
                mt_u32 Ct;

                mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

                // get top and bottom flag
                #if 0 //disp_hal
                pfOpt->FP_GetChnBottomFlag(enDisp, &bBtm, &vcnt);
                #endif
                if( (value & 0x8) == 0x8)  //top_start
                {
                  pstDisp->stCBInfo.stDispInfo.bIsBottomField = MT_FALSE;
                }
                else
                {
                  pstDisp->stCBInfo.stDispInfo.bIsBottomField = MT_TRUE;
                }
                pstDisp->stCBInfo.stDispInfo.u32Vline = 1;
                //printk("[%d, %d, %d]\n", enDisp, bBtm, Ct);
            }

            v = pstDisp->stList[eIntType].u32NodeFlag;
            for (n = DEF_DISP_ISR_LIST_LENGTH - 1; (n >=0 ) && v; n--)  //excute callback in reverse order, DF_ISR_Update should be excuted before csc_update
            {
                if (v & (1 << n))
                {
                    #if 0
                    if (pstDisp->eEvent != MT_DRV_DISP_C_VT_INT)
                    {
                        printk("##n=%d, id=%d, event=%d\n", n, enDisp,pstDisp->eEvent);
                    }
                    #endif

                    if (pstDisp->stList[eIntType].stNode[n].pfDISP_Callback)
                        pstDisp->stList[eIntType].stNode[n].pfDISP_Callback(pstDisp->stList[eIntType].stNode[n].hDst, &pstDisp->stCBInfo);

                    v = v - (1<<n);

                }

                if (!v)
                {
                    break;
                }
            }

            u32IntState = u32IntState & (~(mt_u32)s_DispIntTable[enDisp][eIntType]);
        }
    }

    if (u32IntState)
    {
        //DISP_DEBUGK("Unespexted interrup 0x%x happened!\n", u32IntState);
    }
    //printk("[%s] line %d, interrupt top %d,bot %d,total %d\n",__FUNCTION__,__LINE__,s_DebugTopIntCount,s_DebugBotIntCount,s_DebugG1IntCount);

    return DEF_DISP_ISR_Main_RETURN_VALUE;
}
//default isr group1
irqreturn_t aria_disp_isr_group1(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 value = HAL_GET_U32((volatile u32*)(REG_ARIA_HD_ENCODER_BASE+0x90))>>16; //group1 status

    //value = value & 0xf;
    s_DebugG1IntCount++;
    //if(value&0x7)
    s_DebugBotIntCount++;
    p_intcnt[4]=s_DebugG1IntCount;
    p_intcnt[5]=s_DebugBotIntCount;
    p_intcnt[6]=value;

    //printk("[%s]line %d, value %08x\n",__FUNCTION__,__LINE__,value);
    return DEF_DISP_ISR_Main_RETURN_VALUE;
}
irqreturn_t aria_disp_isr_top_sd(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 n_fifo =HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_FIFO_CNT);

    #if defined(CONFIG_MT_CHIP_SYMPHONY4)  || defined(CONFIG_MT_CHIP_SYMPHONY6)// read this reg to clear isr
    __attribute__((unused)) unsigned int state = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_INT_STATE0);
    #endif

    mt_s32 vbi_data = 0;
    mt_s32 num = 0;
    mt_s32 have_data = 0;
    mt_s32 i = 0;

    mt_u8 ccdata[3] = {0};
    mt_u8 wssdata[3] = {0};
    mt_u8 vpsdata[14] = {0};
    mt_u32 data = 0;
    mt_u32 data1 = 0;
    mt_u32 data2 = 0;
    mt_u32 data3 = 0;
    mt_u32 field_offset = 0;
    mt_u32 odd_flag = 0;

    disp_vbi_buffer_s *p_disp_vbi_buffer = &disp_vbi_buffer;

    if((p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_TTX) ||(p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_CC_PES))
    {
        //printk("linda debug ISR [%s]line %d, mask %08x,clr %08x,state %08x fifo_cnt=%08x\n",__FUNCTION__,__LINE__,value0,value1,value2, n_fifo);
        if(p_disp_vbi_buffer->vbi_wp >= p_disp_vbi_buffer->vbi_rp)
        {
            vbi_data = (p_disp_vbi_buffer->vbi_wp - p_disp_vbi_buffer->vbi_rp)/64;
        }
        else
        {
            vbi_data = (VBI_DATA_BUF_LEN - p_disp_vbi_buffer->vbi_rp + p_disp_vbi_buffer->vbi_wp)/64;
        }
        num = n_fifo/16;
#if 0
        if(vbi_data>0)
            printk("linda debug ISR [%s]line %d, mask %08x,clr %08x,state %08x num=0x%x vbi_data=0x%x fifo_cnt=%08x\n",__FUNCTION__,__LINE__,value0,value1,value2, num, vbi_data, n_fifo);
#endif
        while(num > 0 && vbi_data>0)
        {
            //  printk("linda debug ISR [%s]line %d, mask %08x,clr %08x,state %08x num=0x%x vbi_data=0x%x fifo_cnt=%08x\n",__FUNCTION__,__LINE__,value0,value1,value2, num, vbi_data, n_fifo);
            have_data = 1;
            for(i=0; i<16; i++)
            {
                HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_FIFO_REG, BIGL(*(unsigned int *)(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_rp)));
                p_disp_vbi_buffer->vbi_rp =  p_disp_vbi_buffer->vbi_rp+4;
                if( p_disp_vbi_buffer->vbi_rp >= VBI_DATA_BUF_LEN)
                {
                    p_disp_vbi_buffer->vbi_rp = 0;
                }
            }
            vbi_data --;
            num --;
        }
        if(have_data != 0)
        {
            //printk("linda debug ISR [%s]line %d, mask %08x,clr %08x,state %08x num=0x%x vbi_data=0x%x fifo_cnt=%08x vbi_type=%d\n",__FUNCTION__,__LINE__,value0,value1,value2, num, vbi_data, n_fifo, p_disp_vbi_buffer->vbi_type);

            //why?
            //HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, (1 << 8));

            // according to  ucos ,change to follow
            unsigned int value = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL);
            value |= (1 << 8);
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, value);
        }
    }
    else if(p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_CC)
    {
        aria_disp_get_vbi_data_size((mt_s32 *)&vbi_data);
        if(vbi_data >= 3)
        {
            for(i=0; i<3; i++)
            {
                ccdata[i]=*(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_rp);
                p_disp_vbi_buffer->vbi_rp =  p_disp_vbi_buffer->vbi_rp+1;
                if( p_disp_vbi_buffer->vbi_rp >= VBI_DATA_BUF_LEN)
                {
                    p_disp_vbi_buffer->vbi_rp = 0;
                }
            }

            if((ccdata[0] & 0xff) == 0xfd)
            {
                ccdata[0] = 0x15;
            }
            else if((ccdata[0] & 0xff) == 0xfc)
            {
                ccdata[0] = 0x35;
            }
            field_offset = ccdata[0];
            data = ccdata[1];
            data = (data << 8) + ccdata[2];

            if(field_offset & 0x20)
                odd_flag = 1;

            if(odd_flag == 1)
            {
                mt_u32 data_tmp = 0;
                mt_u32 ctrl_tmp = 0;
                data_tmp = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_CC_DATA_REG);
                data_tmp = (data ) | (data_tmp & 0xffff0000);
                HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_CC_DATA_REG, data_tmp);
                ctrl_tmp = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL);
                field_offset = (field_offset | (1 << 16) |(ctrl_tmp & 0x100000));
                HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, field_offset);
                //printk("send cc odd data[0x%08x] ctrl[0x%08x]\n", data_tmp,field_offset);
            }
            else
            {
                u32 data_tmp = 0;
                u32 ctrl_tmp = 0;
                data_tmp = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_CC_DATA_REG);
                data_tmp = (data << 16) | (data_tmp & 0x0000ffff);
                HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_CC_DATA_REG, data_tmp);
                ctrl_tmp = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL);
                field_offset = (field_offset | (1 << 20) |(ctrl_tmp & 0x10000));
                HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, field_offset);
                //printk("send cc even data[0x%08x] ctrl[0x%08x]\n", data_tmp,field_offset);
            }
        }
    }
    else if(p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_WSS)
    {
        aria_disp_get_vbi_data_size((mt_s32 *)&vbi_data);
        //printk("linda debug vbi_data=%d %s %s %d vbi_wp=0x%x vbi_rp=0x%x\n", vbi_data, __FILE__, __func__, __LINE__, p_disp_vbi_buffer->vbi_wp, p_disp_vbi_buffer->vbi_rp);
        if(vbi_data >= 3)
        {
            for(i=0; i<3; i++)
            {
                wssdata[i]=*(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_rp);
                p_disp_vbi_buffer->vbi_rp =  p_disp_vbi_buffer->vbi_rp+1;
                if( p_disp_vbi_buffer->vbi_rp == VBI_DATA_BUF_LEN)
                {
                    p_disp_vbi_buffer->vbi_rp = 0;
                }
            }
            //printk("linda debug %s %d wssdata[0]=0x%x wssdata[1]=0x%x wssdata[2]=0x%x value2=0x%x\n", __func__, __LINE__, wssdata[0], wssdata[1], wssdata[2], value2);
            field_offset = wssdata[0];
            data1 = wssdata[1] << 6;
            data2 = wssdata[2] >> 2;
            data = data1 + data2;
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG, data);
            field_offset = field_offset | (1 << 28);
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, field_offset);
        }
    }
    else if(p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_VPS)
    {
        aria_disp_get_vbi_data_size((mt_s32 *)&vbi_data);
          //       printk("linda debug ISR [%s]line %d, mask %08x,clr %08x,state %08x fifo_cnt=%08x\n",__FUNCTION__,__LINE__,value0,value1,value2, n_fifo);
        if(vbi_data >= 14)
        {
            for(i=0; i<14; i++)
            {
                vpsdata[i]=*(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_rp);
                p_disp_vbi_buffer->vbi_rp =  p_disp_vbi_buffer->vbi_rp+1;
                if( p_disp_vbi_buffer->vbi_rp >= VBI_DATA_BUF_LEN)
                {
                  p_disp_vbi_buffer->vbi_rp = 0;
                }
            }
             //      printk("linda debug %s %d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n", __func__, __LINE__, vpsdata[0], vpsdata[1], vpsdata[2], vpsdata[3], vpsdata[4], vpsdata[5], vpsdata[6], vpsdata[7], vpsdata[8], vpsdata[9], vpsdata[10], vpsdata[11], vpsdata[12], vpsdata[13]);
            field_offset = vpsdata[0];
            data = vpsdata[1];
            data = (data << 8) + vpsdata[2];
            data = (data << 8) + vpsdata[3];
            data = (data << 8) + vpsdata[4];
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_VPS_DATA1_REG, data);
            //  printk("linda debug %s %d 0x%x ", __func__, __LINE__, data);
                  data1 = vpsdata[5];
                  data1 = (data1 << 8) + vpsdata[6];
                  data1 = (data1 << 8) + vpsdata[7];
                  data1 = (data1 << 8) + vpsdata[8];
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_VPS_DATA2_REG, data1);
            //  printk("0x%x ", data1);
                  data2 = vpsdata[9];
                  data2 = (data2 << 8) + vpsdata[10];
                  data2 = (data2 << 8) + vpsdata[11];
                  data2 = (data2 << 8) + vpsdata[12];
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_VPS_DATA3_REG, data2);
            //  printk("0x%x ",data2);
            data3 = vpsdata[13];
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_VPS_DATA4_REG, data3);
            //  printk("0x%x\n", data3);
            field_offset = field_offset | (1 << 24);
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, field_offset);
        }
    }
#if 0
    else if(p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_CGMS_A)
    {
//        printk("linda debug %s %d\n", __func__, __LINE__);
        HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_CFIG6, 0xc60012);
        HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
        HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
        HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, 0xb00a3);
    }
#endif
    else if(p_disp_vbi_buffer->vbi_type == MT_DISP_VBI_TYPE_TTX_ES)
    {
        aria_disp_get_vbi_data_size((mt_s32 *)&vbi_data);
        while((n_fifo > 0) && (num < vbi_data) && ((vbi_data - num) >= 4))
        {
            have_data = 1;           
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_FIFO_REG, BIGL(*(unsigned int *)(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_rp)));
            p_disp_vbi_buffer->vbi_rp =  p_disp_vbi_buffer->vbi_rp + 4;
            if( p_disp_vbi_buffer->vbi_rp >= VBI_DATA_BUF_LEN)
            {
                p_disp_vbi_buffer->vbi_rp = 0;
            }                
            n_fifo--;
            num += 4;   
        }
        if(have_data == 1)
        {
            field_offset  = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL);
            field_offset  |= (1 << 8);
            HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, field_offset);
        }    
        
    }
    //s_DebugsdTopIntCount++;
    //p_intcnt[8]=s_DebugsdTopIntCount;
    return DEF_DISP_ISR_Main_RETURN_VALUE;
}

irqreturn_t aria_disp_isr_bot_sd(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 value2 = HAL_GET_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_INT_STATE1);
    //printk("[%s]line %d, mask %08x,clr %08x,state %08x\n",__FUNCTION__,__LINE__,value0,value1,value2);
    HAL_PUT_U32((volatile u32*)REG_ARIA_SD_ENC_VBI_INT_STATE_CLR1, (value2&0xffffff80));
    s_DebugsdBotIntCount++;
    p_intcnt[12]=s_DebugsdBotIntCount;
    return DEF_DISP_ISR_Main_RETURN_VALUE;
}

mt_s32 DISP_ISR_Main_VB( mt_void)
{

    MT_DRV_DISPLAY_E enDisp;
    MT_DRV_DISP_CALLBACK_TYPE_E eIntType;
    DISP_ISR_CHN_S *pstDisp;
    mt_u32 u32IntState = 0x7;//should get from hw_mask:pfOpt->PF_GetMaskedIntState
    mt_u32 n, v;

#if 0 //disp_hal
    DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    // if get int ops failed, return
    DispCheckNullPointer( pfOpt);
    DispCheckNullPointer( pfOpt->PF_GetMaskedIntState);
    DispCheckNullPointer( pfOpt->PF_CleanIntState);
    DispCheckNullPointer( pfOpt->FP_GetChnBottomFlag);

    // s1 get interrupt state
    pfOpt->PF_GetMaskedIntState(&u32IntState);

    // s2 clear interrupt state
    pfOpt->PF_CleanIntState(u32IntState);
#endif
    s_DebugMyIntCount++;
    /*
    if (s_DebugMyIntCount < DEF_DEBUG_DISP_INT_MAX_NUMBER)
    {
    printk("<I>");
    }
    */

    //return DEF_DISP_ISR_Main_RETURN_VALUE;

    // s3 check and recode underload interrupt

    // s5 process interrupt one by one
    //printk("[0x%x]", u32IntState);

    // s5.0 if display is not open, return
    while(!kthread_should_stop())
    {
        set_current_state(TASK_UNINTERRUPTIBLE);

        if (!s_DispISRMngr.u32ChnNumber)
        {
            //means no isr callback function so do not need excute
            //return 0;
        }

        //DISP_DEBUGK("DISP DISP_ISR_Main_VB \n");

        {
            enDisp = MT_DRV_DISPLAY_1;
            pstDisp = &s_DispISRMngr.stDispChn[enDisp];

            if (pstDisp->bEnable != MT_TRUE)
            {
                break;
            }

            //just for debug: mask the interrupt artificially:should remove this operation in practical situation
            if(u32IntState == 0x0)
            {
                 u32IntState = 0x7;
            }

            for (eIntType=MT_DRV_DISP_C_INTPOS_0_PERCENT; eIntType<MT_DRV_DISP_C_TYPE_BUTT; eIntType++)
            {
                //MT_BOOL bBtm;
                //mt_u32 vcnt;

                if(!(s_DispIntTable[enDisp][eIntType] & u32IntState) )
                {

                    continue;
                }

                if( s_DispIntTable[enDisp][MT_DRV_DISP_C_INTPOS_0_PERCENT] & u32IntState)
                {
                    mt_u32 Ct;

                    mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

                    // get top and bottom flag
                    #if 0 //disp_hal
                    pfOpt->FP_GetChnBottomFlag(enDisp, &bBtm, &vcnt);

                    pstDisp->stCBInfo.stDispInfo.bIsBottomField = bBtm;
                    pstDisp->stCBInfo.stDispInfo.u32Vline = vcnt;
                    #endif
                    //printk("[%d, %d, %d]\n", enDisp, bBtm, Ct);
                }

                v = pstDisp->stList[eIntType].u32NodeFlag;
                for (n = 0; (n < DEF_DISP_ISR_LIST_LENGTH) && v; n++)
                {
                    if (v & (1 << n))
                    {
                        #if 0
                        if (pstDisp->eEvent != MT_DRV_DISP_C_VT_INT)
                        {
                            printk("##n=%d, id=%d, event=%d\n", n, enDisp,pstDisp->eEvent);
                        }
                        #endif

                        if (pstDisp->stList[eIntType].stNode[n].pfDISP_Callback)
                            pstDisp->stList[eIntType].stNode[n].pfDISP_Callback(pstDisp->stList[eIntType].stNode[n].hDst, &pstDisp->stCBInfo);

                        v = v - (1<<n);

                    }

                    if (!v)
                    {
                        break;
                    }
                }

                u32IntState = u32IntState & (~(mt_u32)s_DispIntTable[enDisp][eIntType]);
            }
       }

        if (u32IntState)
        {
            DISP_DEBUGK("Unespexted interrup 0x%x happened!\n", u32IntState);
        }
        //printk("[%s] line %d, interrupt top %d,bot %d,total %d\n",__FUNCTION__,__LINE__,s_DebugTopIntCount,s_DebugBotIntCount,s_DebugG1IntCount);

        schedule_timeout(HZ);
        set_current_state(TASK_RUNNING);
    }

    return 0;
}

irqreturn_t aria_disp_isr_op_ree(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 mask_lo = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448088));
    mt_u32 mask_hi = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf44808c));
    mt_u32 clr_lo = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448080));
    mt_u32 clr_hi = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448084));
    mt_u32 state_lo = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448090));
    mt_u32 state_hi = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448094));
    
    printk(KERN_EMERG"[%s]line %d, mask_lo:%08x,mask_hi:%08x,clr_lo:%08x,clr_hi:%08x,state_Lo:%08x,state_hi:0x%08x\n",
        __FUNCTION__,__LINE__,
        mask_lo,mask_hi,clr_lo,clr_hi,state_lo,state_hi);
    
    //set mask    
    HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448088), 0xffffffff);
    HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf44808c), 0xffffffff);

    //clear irq
    HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448080), state_lo);
    HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf448084), state_hi);

    return DEF_DISP_ISR_Main_RETURN_VALUE;
}

irqreturn_t aria_disp_isr_osdc(mt_s32 irq, mt_void *dev_id)
{
    mt_u32 mask = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf443038));
    mt_u32 state = HAL_GET_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf44303c));
    drv_reg_4k_disp_set_osdc_irq_en_osdc_end_irq_en(0);

    //drv_reg_4k_disp_set_osdc_rst_osdc_rst_h(1);
    //udelay(10);
    //drv_reg_4k_disp_set_osdc_rst_osdc_rst_h(0);
    
    printk(KERN_EMERG"[%s]line %d, mask:%08x,state:%08x\n",__FUNCTION__,__LINE__, mask,state);
    
    //clear irq        
    HAL_PUT_U32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf44303c), state);

    return DEF_DISP_ISR_Main_RETURN_VALUE;
}

mt_s32 DISP_ISR_Main(mt_s32 irq, mt_void *dev_id)
{

#if 0 //disp_hal :debug by other way
    MT_DRV_DISPLAY_E enDisp;
    MT_DRV_DISP_CALLBACK_TYPE_E eIntType;
    DISP_ISR_CHN_S *pstDisp;
    mt_u32 u32IntState = 0;
    mt_u32 n, v;
    mt_s32 i;
    DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    // if get int ops failed, return
    DispCheckNullPointer( pfOpt);
    DispCheckNullPointer( pfOpt->PF_GetMaskedIntState);
    DispCheckNullPointer( pfOpt->PF_CleanIntState);
    DispCheckNullPointer( pfOpt->FP_GetChnBottomFlag);

    // s1 get interrupt state
    pfOpt->PF_GetMaskedIntState(&u32IntState);

    // s2 clear interrupt state
    pfOpt->PF_CleanIntState(u32IntState);

    s_DebugMyIntCount++;
    /*
    if (s_DebugMyIntCount < DEF_DEBUG_DISP_INT_MAX_NUMBER)
    {
    printk("<I>");
    }
    */

    //return DEF_DISP_ISR_Main_RETURN_VALUE;

    // s3 check and recode underload interrupt

    // s5 process interrupt one by one
    //printk("[0x%x]", u32IntState);


    // s5.0 if display is not open, return
    if (!s_DispISRMngr.u32ChnNumber)
    {
        return DEF_DISP_ISR_Main_RETURN_VALUE;
    }

    for(i = 1; i >= 0; i--)
    {
        enDisp = (MT_DRV_DISPLAY_E)i;
        pstDisp = &s_DispISRMngr.stDispChn[enDisp];

        if (pstDisp->bEnable != MT_TRUE)
        {
            continue;
        }

        for (eIntType=MT_DRV_DISP_C_INTPOS_0_PERCENT; eIntType<MT_DRV_DISP_C_TYPE_BUTT; eIntType++)
        {
            MT_BOOL bBtm;
            mt_u32 vcnt;

            if(!(s_DispIntTable[enDisp][eIntType] & u32IntState) )
            {
                continue;
            }

            if( s_DispIntTable[enDisp][MT_DRV_DISP_C_INTPOS_0_PERCENT] & u32IntState )
            {
                mt_u32 Ct;

                mt_drv_sys_gettimestampms((mt_u32 *)&Ct);

                // get top and bottom flag
                pfOpt->FP_GetChnBottomFlag(enDisp, &bBtm, &vcnt);

                pstDisp->stCBInfo.stDispInfo.bIsBottomField = bBtm;
                pstDisp->stCBInfo.stDispInfo.u32Vline = vcnt;
                //printk("[%d, %d, %d]\n", enDisp, bBtm, Ct);
            }

            v = pstDisp->stList[eIntType].u32NodeFlag;
            for (n = 0; (n < DEF_DISP_ISR_LIST_LENGTH) && v; n++)
            {
                if (v & (1 << n))
                {
#if 0
                    if (pstDisp->eEvent != MT_DRV_DISP_C_VT_INT)
                    {
                        printk("##n=%d, id=%d, event=%d\n", n, enDisp,pstDisp->eEvent);
                    }
#endif
                    if (pstDisp->stList[eIntType].stNode[n].pfDISP_Callback)
                        pstDisp->stList[eIntType].stNode[n].pfDISP_Callback(pstDisp->stList[eIntType].stNode[n].hDst, &pstDisp->stCBInfo);

                    v = v - (1<<n);

                    if( !irqs_disabled() )
                    {
                        DISP_PRINT("#######$$$$$$$$$$$............eIntType=%u, n=%d\n",eIntType, n);
                    }

                }

                if (!v)
                {
                    break;
                }
            }

            u32IntState = u32IntState & (~(mt_u32)s_DispIntTable[enDisp][eIntType]);
        }
    }

    if (u32IntState)
    {
        DISP_FATAL("Unespexted interrup 0x%x happened!\n", u32IntState);
    }
#endif
    return DEF_DISP_ISR_Main_RETURN_VALUE;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */












