/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <asm/io.h>
//#include <mach/hardware.h>
//#include <mach/platform.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#include "mt_type.h"
#include "mt_kernel_adapt.h"
#include "mt_module.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"

#include "mt_drv_module.h"
#include "mt_drv_mmz.h"
#include "mt_reg_common.h"
#include "mt_drv_ao.h"
#include "mt_drv_ai.h"
#include "hal_aiao.h"
#include "audio_util.h"
#include "aud_in_aria_reg.h"

#include "mt_module_debug.h"

#if defined(CONFIG_MT_CHIP_ARIA)
#include "aud_out_aria_reg.h"
#include <mach/aria_io.h>
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#include "aud_out_symphony_reg.h"
#include "mt_mach/irq.h"
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#endif
#ifdef ALSA_DEBUG_TIME
#include <linux/time.h>
#endif
#ifdef __cplusplus
 #if __cplusplus
 extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */
static atomic_t atmAiaoInitCnt = ATOMIC_INIT(0);
typedef struct
{
    mt_handle     hPort[AIAO_INT_BUTT];
    AIAO_IsrFunc *fRegIsr[AIAO_INT_BUTT];
#ifdef MT_AIAO_TIMER_SUPPORT
    AIAO_TimerIsrFunc *fTimerIsr[AIAO_INT_BUTT];
#endif
	void *sub[AIAO_INT_BUTT];//only for alsa isr MT_ALSA_AI_SUPPORT
} AIAO_GLOBAL_SOURCE_S;

static inline void REG_WRITE(volatile u32* addr, u32 val)
{
	HAL_PUT_U32(addr, val);
}

static inline u32 REG_READ(volatile u32* addr)
{
	return HAL_GET_U32(addr);
}

static AIAO_GLOBAL_SOURCE_S g_AIAORm;
extern mt_void avsync_audio_pop_apts(mt_void);
extern ulong mt_get_sys_ctrl_base(void);


MT_DECLARE_MUTEX(g_HalAiaoMutex);
///TODO:minnan debug
#if 0
static irqreturn_t AIAOIsr(int irq, void * dev_id)
{
    ///TODO:here to update audio in buffer read and write pointer
    return IRQ_HANDLED;
}
#else
mt_u32 test_mem[3840/4] = {1,2,3,4,5,6,7,8};
mt_u32 test_mem1[3840/4] = {8,7,6,5,4,3,2,1};
mt_u32 test_cnt = 0;
#if 0 //unuse code
static mt_s32 AIAOIsr( mt_void)
{
#if 0
    mt_u32 Bytes = 0;
    mt_u32 writen_Bytes = 0;
    AIAO_PORT_ID_E enPortID = AIAO_PORT_RX0;
    memset(test_mem, 0, 3840);
    memset(test_mem1, 1, 3840);
    test_mem[0] = 0x1234;
    test_mem1[0] = 0x4321;
    while(!kthread_should_stop())
    {
      set_current_state(TASK_UNINTERRUPTIBLE);
      Bytes = HAL_AIAO_P_QueryBufFree(enPortID);
      if(3840 < Bytes)
      {
          test_cnt++;
          if(0 == (test_cnt%2))
          {
            writen_Bytes = HAL_AIAO_P_WriteData(enPortID, (mt_u8 *)test_mem, 3840);
          }
          else
          {
            writen_Bytes = HAL_AIAO_P_WriteData(enPortID, (mt_u8 *)test_mem1, 3840);
          }
          MT_INFO_AIAO("AI isr put data %d free %d value 0x%x 0x%x\n",
            writen_Bytes, Bytes, *(test_mem), *(test_mem1));
      }
      else
      {
          MT_FATAL_AIAO("AI isr not put data %d free %d\n", writen_Bytes, Bytes);

      }
         schedule_timeout(HZ);
        set_current_state(TASK_RUNNING);
    }
#endif
return 0;
}
#endif
#endif
mt_void    HAL_AIAO_PowerOff(mt_void)
{
  return;
}

mt_s32    HAL_AIAO_Suspend(mt_void)
{
    return MT_SUCCESS;
}

mt_s32    HAL_AIAO_Resume(mt_void)
{
    return MT_SUCCESS;
}


mt_s32    HAL_AIAO_PowerOn(mt_void)
{
    return MT_SUCCESS;
}
//static   struct task_struct * p_ai_isr_simul_thread = NULL; //for debug

mt_s32    HAL_AIAO_RequestIsr(mt_void)
{
    ///TODO:minnan test use
#if 0
    if (request_irq(AIAO_IRQ_NUM, AIAOIsr, 0, "mt_aiao_irq", NULL) != 0)
    {
        MT_FATAL_AIAO("request_irq failed irq num =%d!\n", AIAO_IRQ_NUM);
        return MT_FAILURE;
    }
#else
	//p_ai_isr_simul_thread =   kthread_create(AIAOIsr, NULL, "ai_isr_simulate_thread");
	//wake_up_process(p_ai_isr_simul_thread);

          MT_FATAL_AIAO("minnan AIAO request_irq\n");
#endif
    return MT_SUCCESS;
}

mt_void    HAL_AIAO_FreeIsr(mt_void)
{
    ///TODO:minnan test use
#if 0
    /* free irq */
    free_irq(AIAO_IRQ_NUM, NULL);
#else
          //kthread_stop(p_ai_isr_simul_thread);
         // MT_FATAL_AIAO("minnan AIAO free irq\n");
#endif
    return ;
}

static mt_void HAL_AO_intr_init(mt_void)
{
#if defined(CONFIG_MT_CHIP_ARIA)
	//reg_aria_aud_set_audfrm_intr_mask(0);
	//reg_aria_aud_set_audfrm_intr_cfg(1);
	//reg_aria_aud_set_audfrm_intr_en(1);
	reg_aria_aud_set_audfrm_intr_2_mask(0);
	reg_aria_aud_set_audfrm_intr_2_cfg(1);
	reg_aria_aud_set_audfrm_intr_2_en(1);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	//reg_symphony_aud_set_audfrm_intr_mask(0);
	//reg_symphony_aud_set_audfrm_intr_cfg(1);
	//reg_symphony_aud_set_audfrm_intr_en(1);

	//reg_symphony_aud_set_audfrm_intr_2_mask(0);
	//reg_symphony_aud_set_audfrm_intr_2_cfg(1);
	//reg_symphony_aud_set_audfrm_intr_2_en(1);

  //      1               1                1            1               1                  1
  // aud_frm2 intr, pcm_fifo_cnt_dif, pcm_fifo_empty, buf_rw_intr, bb_buf_w_hang_up, aud_frm_intr
  // 1 is turn off, 0 is turn on
  {
    mt_u32 data = 0;
    //data = 0x3600;  // 0x3600: open aud_frm intr and pcm_fifo empty intr
    //data = 0x3e00;  // 0x3e00: only aud_frm intr
    //data = 0x1f00;  // 0x1f00: only aud_frm2 intr, after play one frame
    //data = 0x3e00;  // 0x3e00: only aud_frm intr
    data = 0x211e00;  // 0x1e00: only aud_frm intr and aud_frm2 intr
    //data = 0x1f00;  // 0x1f00: only aud_frm2 intr, after play one frame
    //data = 0x0c00;    // 0x0c00: pcm fifo cnt diff bigger & aud_frm intr
    REG_WRITE((volatile u32*)(mt_get_audioout_base() + 0xA0), data);
  }

  // pcm fifo empty time
  {
    //empty ~0.9ms
    REG_WRITE((volatile u32*)(mt_get_audioout_base() + 0x9C), 3000);
  }


#endif
}

/* global function */
mt_s32                  HAL_AIAO_Init(mt_void)
{
    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (atomic_inc_return(&atmAiaoInitCnt) == 1)
    {
    MT_INFO_AIAO("[ztq]HAL_AIAO_Init...\n");
	HAL_AO_intr_init();
      if (MT_SUCCESS!=HAL_AI_RequestIsr())
      {
          MT_FATAL_AIAO("AIAO request_irq failed\n");
          up(&g_HalAiaoMutex);
          return MT_FAILURE;
      }

    if (MT_SUCCESS!=HAL_AO_RequestIsr())
	{
	  MT_FATAL_AIAO("AIAO request_irq failed\n");
	  up(&g_HalAiaoMutex);
	  return MT_FAILURE;
	}

    }
    up(&g_HalAiaoMutex);

    return MT_SUCCESS;
}

mt_void                 HAL_AIAO_DeInit(mt_void)
{
    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (atomic_dec_return(&atmAiaoInitCnt) == 0)
    {
        HAL_AI_FreeIsr();
	HAL_AO_FreeIsr();
    }
    up(&g_HalAiaoMutex);
    return ;
}

mt_void                 HAL_AIAO_GetHwCapability(mt_u32 *pu32Capability)
{
    return ;
}


mt_void                 HAL_AIAO_GetHwVersion(mt_u32 *pu32Version)
{
    return ;
}


mt_void                 HAL_AIAO_DBG_RWReg(AIAO_Dbg_Reg_S *pstReg)
{
    return ;
}


mt_void                 HAL_AIAO_SetTopInt(mt_u32 u32Multibit)
{
    return ;
}


mt_u32                  HAL_AIAO_GetTopIntRawStatus(mt_void)
{
    return MT_SUCCESS;
}


mt_u32                  HAL_AIAO_GetTopIntStatus(mt_void)
{
    return MT_SUCCESS;
}
static mt_s32 PortBufInit(AIAO_PORT_S hPort)
{
    AIAO_PORT_USER_CFG_S *pstConfig = &hPort->stUserCongfig;
    //AIAO_BufAttr_S *pstBufConfig = &pstConfig->stBufConfig;
    mt_u32 u32StartVirAddr = 0, u32StartPhyAddr = 0;

    if(MT_TRUE==pstConfig->bExtDmaMem)//AICreateChn make bExtDmaMem is TRUE
    {

        if(!pstConfig->stExtMem.u32BufPhyAddr || !pstConfig->stExtMem.u32BufVirAddr)
        {
            MT_FATAL_AIAO("PhyAddr(0x%x) VirAddr(0x%x) invalid \n", pstConfig->stExtMem.u32BufPhyAddr,pstConfig->stExtMem.u32BufVirAddr);
            return MT_FAILURE;
        }
        if(pstConfig->stExtMem.u32BufPhyAddr%AIAO_BUFFER_ADDR_ALIGN)
        {
            MT_FATAL_AIAO("PhyAddr(0x%x) should align to (0x%x) invalid \n", pstConfig->stExtMem.u32BufPhyAddr,AIAO_BUFFER_ADDR_ALIGN);
            return MT_FAILURE;
        }
        if(0 >= pstConfig->stExtMem.u32BufSize)
        {
            MT_FATAL_AIAO("ExtMem(0x%x) \n", pstConfig->stExtMem.u32BufSize);
            return MT_FAILURE;
        }
        u32StartPhyAddr = pstConfig->stExtMem.u32BufPhyAddr;
        u32StartVirAddr = pstConfig->stExtMem.u32BufVirAddr;


    }

    // step 2.0, CIRC BUf
   // CIRC_BUF_Init(&hPort->stCB,
                //  (mt_u32 *)(0),
               //   (mt_u32 *)(0),
               //   (mt_u32 *)u32StartVirAddr,
                //  pstConfig->stExtMem.u32BufSize);

    // step 3.0, AIAO CIRC BUf Reg
    hPort->stBuf.u32BUFF_SADDR = u32StartPhyAddr;
    hPort->stBuf.u32BUFF_WPTR = 0;
    hPort->stBuf.u32BUFF_RPTR = 0;
    hPort->stBuf.u32BUFF_SIZE = pstConfig->stExtMem.u32BufSize;
    return MT_SUCCESS;
}


static mt_s32 PortBufInit_Vsb(AIAO_PORT_S hPort)
{
    AIAO_PORT_USER_CFG_S *pstConfig = &hPort->stUserCongfig;
    //AIAO_BufAttr_S *pstBufConfig = &pstConfig->stBufConfig;
    mt_u32 u32StartVirAddr = 0, u32StartPhyAddr = 0;

    if (MT_TRUE == pstConfig->bExtDmaMem) //AICreateChn make bExtDmaMem is TRUE
    {

	if (!pstConfig->stExtMem.u32BufPhyAddr || !pstConfig->stExtMem.u32BufVirAddr) {
	    MT_FATAL_AIAO("PhyAddr(0x%x) VirAddr(0x%x) invalid \n", pstConfig->stExtMem.u32BufPhyAddr, pstConfig->stExtMem.u32BufVirAddr);
	    return MT_FAILURE;
	}
	if (pstConfig->stExtMem.u32BufPhyAddr % AIAO_BUFFER_ADDR_ALIGN) {
	    MT_FATAL_AIAO("PhyAddr(0x%x) should align to (0x%x) invalid \n", pstConfig->stExtMem.u32BufPhyAddr, AIAO_BUFFER_ADDR_ALIGN);
	    return MT_FAILURE;
	}
	if (0 >= pstConfig->stExtMem.u32BufSize) {
	    MT_FATAL_AIAO("ExtMem(0x%x) \n", pstConfig->stExtMem.u32BufSize);
	    return MT_FAILURE;
	}
	u32StartPhyAddr = pstConfig->stExtMem.u32BufPhyAddr;
	u32StartVirAddr = pstConfig->stExtMem.u32BufVirAddr;
    }

#if 0
    // step 2.0, CIRC BUf
    CIRC_BUF_Init(&hPort->stCB,
                  (mt_u32 *)(0),
                  (mt_u32 *)(0),
                  (mt_u32 *)u32StartVirAddr,
                  pstConfig->stExtMem.u32bufszperch);
#endif
    // step 3.0, AIAO CIRC BUf Reg
    hPort->stBuf.u32BUFF_SADDR = u32StartPhyAddr;
    hPort->stBuf.u32BUFF_WPTR = 0;
    hPort->stBuf.u32BUFF_RPTR = 0;
    hPort->stBuf.u32BUFF_SIZE = pstConfig->stExtMem.u32bufszperch;
    return MT_SUCCESS;
}


static mt_void PortBufDeInit(AIAO_PORT_S hPort)
{
    CIRC_BUF_DeInit(&hPort->stCB);
}


/* global port function */
mt_s32                  HAL_AIAO_P_Open_Vsb(AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig)
{
	AIAO_PORT_S hPort = MT_NULL;
    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (MT_NULL == g_AIAORm.hPort[Id])
    {
        AIAO_ASSERT_NULL(pstConfig)
        // step 1 malloc AIAO_PORT_CTX_S
        hPort = (AIAO_PORT_S)AUTIL_AIAO_MALLOC(MT_ID_AIAO, sizeof(AIAO_PORT_CTX_S), GFP_KERNEL);
        if (hPort == MT_NULL)
        {
            MT_FATAL_AIAO("malloc AIAO_PORT_CTX_S failed\n");
            return MT_FAILURE;
        }

        memset(hPort, 0, sizeof(AIAO_PORT_CTX_S));
        memcpy(&hPort->stUserCongfig, pstConfig, sizeof(AIAO_PORT_USER_CFG_S));
        hPort->enPortID = enPortID;
        hPort->enStatus = AIAO_PORT_STATUS_STOP;

        if (MT_FAILURE == PortBufInit_Vsb(hPort))
        {
            AUTIL_AIAO_FREE(MT_ID_AIAO, hPort);
            MT_FATAL_AIAO("PortBufInit failed\n");
            return MT_FAILURE;
        }

        g_AIAORm.hPort[Id]   = (mt_handle)hPort;
        g_AIAORm.fRegIsr[Id] = NULL;
    }

    up(&g_HalAiaoMutex);
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_Open(AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig)
{
	AIAO_PORT_S hPort = MT_NULL;
    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (MT_NULL == g_AIAORm.hPort[Id])
    {
        AIAO_ASSERT_NULL(pstConfig)
        // step 1 malloc AIAO_PORT_CTX_S
        hPort = (AIAO_PORT_S)AUTIL_AIAO_MALLOC(MT_ID_AIAO, sizeof(AIAO_PORT_CTX_S), GFP_KERNEL);
        if (hPort == MT_NULL)
        {
            MT_FATAL_AIAO("malloc AIAO_PORT_CTX_S failed\n");
            return MT_FAILURE;
        }

        memset(hPort, 0, sizeof(AIAO_PORT_CTX_S));
        memcpy(&hPort->stUserCongfig, pstConfig, sizeof(AIAO_PORT_USER_CFG_S));
        hPort->enPortID = enPortID;
        hPort->enStatus = AIAO_PORT_STATUS_STOP;

        if (MT_FAILURE == PortBufInit(hPort))
        {
            AUTIL_AIAO_FREE(MT_ID_AIAO, hPort);
            MT_FATAL_AIAO("PortBufInit failed\n");
            return MT_FAILURE;
        }

        g_AIAORm.hPort[Id]   = (mt_handle)hPort;
        g_AIAORm.fRegIsr[Id] = NULL;
    }

    up(&g_HalAiaoMutex);
    return MT_SUCCESS;
}

mt_void                 HAL_AIAO_P_Close(AIAO_PORT_ID_E enPortID)
{
    //mt_s32 Ret;
    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (g_AIAORm.hPort[Id])
    {
        // step 1 free buffer
        PortBufDeInit((AIAO_PORT_S)g_AIAORm.hPort[Id]);

        // step 2 free dev sourece
        AUTIL_AIAO_FREE(MT_ID_AIAO, (mt_void *)g_AIAORm.hPort[Id]);

        g_AIAORm.hPort[Id] = MT_NULL;
    }
    up(&g_HalAiaoMutex);
}

mt_s32 HAL_AIAO_P_SetAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_ATTR_S *pstAttr)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_Open_Veri(AIAO_PORT_ID_E enPortID, const AIAO_PORT_USER_CFG_S *pstConfig)
{
    return MT_SUCCESS;
}

mt_void                 HAL_AIAO_P_Close_Veri(AIAO_PORT_ID_E enPortID)
{
    return ;
}

mt_s32 HAL_AIAO_P_GetAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_ATTR_S *pstAttr)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_Start(AIAO_PORT_ID_E enPortID)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_Stop(AIAO_PORT_ID_E enPortID, AIAO_PORT_STOPMODE_E enStopMode)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_Mute(AIAO_PORT_ID_E enPortID, MT_BOOL bMute)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_SetSampleRate(AIAO_PORT_ID_E enPortID, MT_UNF_SAMPLE_RATE_E enSampleRate)
{
    //to do
    return MT_SUCCESS;
}


mt_s32                  HAL_AIAO_P_SetVolume(AIAO_PORT_ID_E enPortID, mt_u32 u32VolumedB)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_SetSpdifCategoryCode(AIAO_PORT_ID_E enPortID, AIAO_SPDIF_CATEGORYCODE_E eCategoryCode)
{
    return MT_SUCCESS;
}
mt_s32                  HAL_AIAO_P_SetSpdifSCMSMode(AIAO_PORT_ID_E enPortID, AIAO_SPDIF_SCMS_MODE_E eSCMSMode)
{
    return MT_SUCCESS;
}


mt_s32                  HAL_AIAO_P_SetTrackMode(AIAO_PORT_ID_E enPortID, AIAO_TRACK_MODE_E enTrackMode)
{
    return MT_SUCCESS;
}

mt_s32 AIAO_HAL_P_SetBypass(AIAO_PORT_ID_E enPortID, MT_BOOL bByBass)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_GetUserCongfig(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pstUserConfig)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_GetStatus(AIAO_PORT_ID_E enPortID, AIAO_PORT_STAUTS_S *pstProcInfo)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_SelectSpdifSource(AIAO_PORT_ID_E enPortID, AIAO_SPDIFPORT_SOURCE_E eSrcChnId)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_SetSpdifOutPort(AIAO_PORT_ID_E enPortID, mt_s32 bEn)
{
    return MT_SUCCESS;
}

mt_s32                  HAL_AIAO_P_SetI2SSdSelect(AIAO_PORT_ID_E enPortID, AIAO_I2SDataSel_S  *pstSdSel)
{
    return MT_SUCCESS;
}

mt_u32                  HAL_AIAO_P_ReadData_NotUpRptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize, mt_u32 *pu32Rptr, mt_u32 *pu32Wptr)
{
    return MT_SUCCESS;
}

/* port buffer function */
mt_u32                  HAL_AIAO_P_ReadData(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize)
{
    mt_u32 ReadBytes = 0;
    AIAO_PORT_S hPort;
    CIRC_BUF_S *pstCB;
    AIAO_PROC_STAUTS_S *pstProc;
    mt_u32 Id = PORT2ID(enPortID);

    if (g_AIAORm.hPort[Id])
    {
        hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
        pstCB = &hPort->stCB;
        pstProc = &hPort->stStatus;
        pstProc->uTryReadCnt++;
        ReadBytes = CIRC_BUF_Read(pstCB, pu32Dest, u32DestSize);
        pstProc->uTotalByteRead += ReadBytes;
    }

    return ReadBytes;
}
mt_u32 HAL_AIAO_P_ReadData_Vsb(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Dest, mt_u32 u32DestSize)
{
    mt_u32 ReadBytes = 0;
#ifdef MT_AUDIO_AI_SUPPORT
	AIAO_PORT_S hPort;
	CIRC_BUF_S *pstCB;
	AIAO_PROC_STAUTS_S *pstProc;
    mt_u32 Id = PORT2ID(enPortID);

    if (g_AIAORm.hPort[Id]) {
	hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
	pstCB = &hPort->stCB;
	pstProc = &hPort->stStatus;
	pstProc->uTryReadCnt++;
    if(0 == reg_audin_aria_get_buf_mode_interlace_mode())
    {
	   ReadBytes = CIRC_BUF_Read_Vsb(pstCB, pu32Dest, u32DestSize);
     	pstProc->uTotalByteRead += ReadBytes;
     }
    else
    {
       ReadBytes = CIRC_BUF_Read_Vsb_interlace(pstCB, pu32Dest, u32DestSize);
      	pstProc->uTotalByteRead += ReadBytes;
     }

    }
#endif
    return ReadBytes;
}
mt_u32 HAL_AIAO_P_WriteData(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Src, mt_u32 u3SrcLen,  mt_u32 ChanExist)
{
    mt_u32 WriteBytes = 0;
    AIAO_PORT_S hPort;
    CIRC_BUF_S *pstCB;
    AIAO_PROC_STAUTS_S *pstProc;
    mt_u32 Id = PORT2ID(enPortID);

    if (g_AIAORm.hPort[Id]) {
	hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
	pstCB = &hPort->stCB;
	pstProc = &hPort->stStatus;
	pstProc->uTryWriteCnt++;
	WriteBytes = CIRC_BUF_Write(pstCB, pu32Src, u3SrcLen,  ChanExist);
	pstProc->uTotalByteWrite += WriteBytes;
    }

    return WriteBytes;
}

mt_u32 HAL_AIAO_P_WriteData_Vsb(AIAO_PORT_ID_E enPortID, mt_u8 *pu32Src, mt_u32 u3SrcLen)
{
    mt_u32 WriteBytes = 0;
#ifdef MT_AUDIO_AI_SUPPORT
	AIAO_PORT_S hPort;
	CIRC_BUF_S *pstCB;
	AIAO_PROC_STAUTS_S *pstProc;
    mt_u32 Id = PORT2ID(enPortID);

    if (g_AIAORm.hPort[Id]) {
	hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
	pstCB = &hPort->stCB;
	pstProc = &hPort->stStatus;
	pstProc->uTryWriteCnt++;
    if(0 == reg_audin_aria_get_buf_mode_interlace_mode())
	   WriteBytes = CIRC_BUF_Write_Vsb(pstCB, pu32Src, u3SrcLen);
    else
      WriteBytes = CIRC_BUF_Write_Vsb(pstCB, pu32Src, u3SrcLen * 2);

	pstProc->uTotalByteWrite += WriteBytes;
    }
#endif
    return WriteBytes;
}

mt_u32                  HAL_AIAO_P_PrepareData(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Src, mt_u32 u3SrcLen)
{
    return MT_SUCCESS;
}

mt_u32                  HAL_AIAO_P_QueryBufData_ProvideRptr(AIAO_PORT_ID_E enPortID, mt_u32 *pu32Rptr)
{
    return MT_SUCCESS;
}


mt_u32                  HAL_AIAO_P_QueryBufData(AIAO_PORT_ID_E enPortID)
{
    mt_u32 Bytes = 0;
    //mt_s32 Ret;
    AIAO_PORT_S hPort;
    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (g_AIAORm.hPort[Id])
    {
        hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
        Bytes = CIRC_BUF_QueryBusy(&hPort->stCB);
    }

    up(&g_HalAiaoMutex);

    return Bytes;
}

mt_u32                  HAL_AIAO_P_QueryBufFree(AIAO_PORT_ID_E enPortID)
{
    mt_u32 Bytes = 0;
    //mt_s32 Ret;
    AIAO_PORT_S hPort;
    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (g_AIAORm.hPort[Id])
    {
        hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
        Bytes = CIRC_BUF_QueryFree(&hPort->stCB);
    }

    up(&g_HalAiaoMutex);

    return Bytes;
}

mt_u32                  HAL_AIAO_P_UpdateRptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize)
{
    return MT_SUCCESS;
}

mt_u32                  HAL_AIAO_P_UpdateWptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Src, mt_u32 u3SrcLen)
{
    return MT_SUCCESS;
}

mt_void                  HAL_AIAO_P_GetDelayMs(AIAO_PORT_ID_E enPortID, mt_u32 * pu32Delayms)
{
    return ;
}


mt_s32                  HAL_AIAO_P_GetRbfAttr(AIAO_PORT_ID_E enPortID, AIAO_RBUF_ATTR_S *pstRbfAttr)
{
    mt_s32 Ret = MT_FAILURE;
    AIAO_PORT_S hPort;
    mt_u32 Id = PORT2ID(enPortID);

    Ret = down_interruptible(&g_HalAiaoMutex);
    if (Ret)
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    if (g_AIAORm.hPort[Id])
    {
      hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
      if(MT_TRUE==hPort->stUserCongfig.bExtDmaMem)
      {
          pstRbfAttr->u32BufPhyAddr = hPort->stUserCongfig.stExtMem.u32BufPhyAddr;
          pstRbfAttr->u32BufPhyRptr = hPort->stUserCongfig.stExtMem.u32BufPhyAddr;
          pstRbfAttr->u32BufPhyWptr = hPort->stUserCongfig.stExtMem.u32BufPhyAddr;
          pstRbfAttr->u32BufVirAddr = hPort->stUserCongfig.stExtMem.u32BufVirAddr;
          pstRbfAttr->u32BufVirRptr = hPort->stUserCongfig.stExtMem.u32BufVirAddr;
          pstRbfAttr->u32BufVirWptr = hPort->stUserCongfig.stExtMem.u32BufVirAddr;
          pstRbfAttr->u32BufSize    = hPort->stUserCongfig.stExtMem.u32BufSize;
          ///TODO:minnan no update now
          //AIAO_HW_GetRptrAndWptrRegAddr(hPort->enPortID, &pstRbfAttr->u32BufVirWptr, &pstRbfAttr->u32BufVirRptr);
          //AIAO_HW_GetRptrAndWptrRegPhyAddr(hPort->enPortID, &pstRbfAttr->u32BufPhyWptr, &pstRbfAttr->u32BufPhyRptr);
      }
    }

    up(&g_HalAiaoMutex);

    return Ret;
}

mt_void HAL_AIAO_P_ProcStatistics(AIAO_PORT_ID_E enPortID, mt_u32 u32IntStatus,void * pst)//MT_ALSA_AI_SUPPORT
{
    return ;
}
#ifdef MT_ALSA_AI_SUPPORT
mt_u32  HAL_AIAO_P_ALSA_UpdateRptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize)
{
    mt_u32 Bytes = 0;
    mt_u32 Id = PORT2ID(enPortID);
    if (g_AIAORm.hPort[Id])
    {
        Bytes = iHAL_AIAO_P_ALSA_UpdateRptr(g_AIAORm.hPort[Id], pu32Dest, u32DestSize);
    }
    return Bytes;
}
mt_u32  HAL_AIAO_P_ALSA_UpdateWptr(AIAO_PORT_ID_E enPortID, mt_u8 * pu32Dest, mt_u32 u32DestSize)
{
    mt_u32 Bytes = 0;
    mt_u32 Id = PORT2ID(enPortID);
    if (g_AIAORm.hPort[Id])
    {
        Bytes = iHAL_AIAO_P_ALSA_UpdateWptr(g_AIAORm.hPort[Id], pu32Dest, u32DestSize);
    }
    return Bytes;
}
mt_u32  HAL_AIAO_P_ALSA_FLASH(AIAO_PORT_ID_E enPortID)
{
    mt_u32 Id = PORT2ID(enPortID);

    if (g_AIAORm.hPort[Id])
    {
        iHAL_AIAO_P_ALSA_FLASH(g_AIAORm.hPort[Id]);
}

	return  MT_TRUE;
}
 mt_u32 HAL_AIAO_P_ALSA_QueryWritePos(AIAO_PORT_ID_E enPortID)
{
    mt_u32 Bytes = 0;
    mt_u32 Id = PORT2ID(enPortID);
    if (g_AIAORm.hPort[Id])
    {
        Bytes = iHAL_AIAO_P_ALSA_QueryWritePos(g_AIAORm.hPort[Id]);
    }
    return Bytes;
}
  mt_u32 HAL_AIAO_P_ALSA_QueryReadPos(AIAO_PORT_ID_E enPortID)
 {
     mt_u32 Bytes = 0;
     mt_u32 Id = PORT2ID(enPortID);
     if (g_AIAORm.hPort[Id])
     {
         Bytes = iHAL_AIAO_P_ALSA_QueryReadPos(g_AIAORm.hPort[Id]);
     }
     return Bytes;
 }
#endif
#define PERIOND_NUM 2

static AIAO_PORT_USER_CFG_S g_stAiaoTxI2SDefaultOpenAttr =
{
    .stIfAttr              =
    {
        .enCrgMode         = AIAO_CRG_MODE_MASTER,
        .enChNum           = AIAO_I2S_CHNUM_2,
        .enBitDepth        = AIAO_BIT_DEPTH_16,
        .enRiseEdge        = AIAO_MODE_EDGE_RISE,
        .enRate            = AIAO_SAMPLE_RATE_48K,
        .u32FCLK_DIV       =                     64,
        .u32BCLK_DIV       =                      4,
        .eCrgSource        = AIAO_TX_CRG0,
        .u32PcmDelayCycles =                      1,
        .enI2SMode         = AIAO_MODE_I2S,
        .enSource          = AIAO_TX0,
        .enSD0             = AIAO_I2S_SD0,
        .enSD1             = AIAO_I2S_SD1,
        .enSD2             = AIAO_I2S_SD2,
        .enSD3             = AIAO_I2S_SD3,
        .bMultislot        = MT_FALSE,
    },
    .stBufConfig           =
    {
        .u32PeriodBufSize  = AIAO_DF_PeriodBufSize,
        .u32PeriodNumber   = PERIOND_NUM,
    },
    .enTrackMode           = AIAO_TRACK_MODE_STEREO,
    .enFadeInRate          = AIAO_DF_FadeInRate,
    .enFadeOutRate         = AIAO_DF_FadeOutRate,
    .bMute                 = MT_FALSE,
    .bMuteFade             = MT_TRUE,
    .u32VolumedB           = 0x79,
    .bByBass               = MT_FALSE,
    .pIsrFunc              = HAL_AIAO_P_ProcStatistics,
};
static AIAO_PORT_USER_CFG_S g_stAiaoTxHdmiHbrSDefaultOpenAttr =
{
    .stIfAttr              =
    {
        .enCrgMode         = AIAO_CRG_MODE_MASTER,
        .enChNum           = AIAO_I2S_CHNUM_8,
        .enBitDepth        = AIAO_BIT_DEPTH_16,
        .enRiseEdge        = AIAO_MODE_EDGE_RISE,
        .enRate            = AIAO_SAMPLE_RATE_192K,
        .u32FCLK_DIV       =                     64,
        .u32BCLK_DIV       =                      4,
        .eCrgSource        = AIAO_TX_CRG0,
        .u32PcmDelayCycles =                      1,
        .enI2SMode         = AIAO_MODE_I2S,
        .enSource          = AIAO_TX0,
        .enSD0             = AIAO_I2S_SD0,
        .enSD1             = AIAO_I2S_SD1,
        .enSD2             = AIAO_I2S_SD2,
        .enSD3             = AIAO_I2S_SD3,
        .bMultislot        = MT_FALSE,
    },
    .stBufConfig           =
    {
        .u32PeriodBufSize  = AIAO_DF_PeriodBufSize*16,
        .u32PeriodNumber   = PERIOND_NUM,
    },
    .enTrackMode           = AIAO_TRACK_MODE_STEREO,
    .enFadeInRate          = AIAO_DF_FadeInRate,
    .enFadeOutRate         = AIAO_DF_FadeOutRate,
    .bMute                 = MT_FALSE,
    .bMuteFade             = MT_TRUE,
    .u32VolumedB           = 0x79,
    .bByBass               = MT_TRUE,
    .pIsrFunc              = HAL_AIAO_P_ProcStatistics,
};

static AIAO_PORT_USER_CFG_S g_stAiaoTxHdmiI2SSDefaultOpenAttr =
{
    .stIfAttr              =
    {
        .enCrgMode         = AIAO_CRG_MODE_MASTER,
        .enChNum           = AIAO_I2S_CHNUM_2,
        .enBitDepth        = AIAO_BIT_DEPTH_16,
        .enRiseEdge        = AIAO_MODE_EDGE_RISE,
        .enRate            = AIAO_SAMPLE_RATE_48K,
        .u32FCLK_DIV       =                     64,
        .u32BCLK_DIV       =                      4,
        .eCrgSource        = AIAO_TX_CRG0,
        .u32PcmDelayCycles =                      1,
        .enI2SMode         = AIAO_MODE_I2S,
        .enSource          = AIAO_TX0,
        .enSD0             = AIAO_I2S_SD0,
        .enSD1             = AIAO_I2S_SD1,
        .enSD2             = AIAO_I2S_SD2,
        .enSD3             = AIAO_I2S_SD3,
        .bMultislot        = MT_FALSE,
    },
    .stBufConfig           =
    {
        .u32PeriodBufSize  = AIAO_DF_PeriodBufSize,
        .u32PeriodNumber   = PERIOND_NUM,
    },
    .enTrackMode           = AIAO_TRACK_MODE_STEREO,
    .enFadeInRate          = AIAO_DF_FadeInRate,
    .enFadeOutRate         = AIAO_DF_FadeOutRate,
    .bMute                 = MT_FALSE,
    .bMuteFade             = MT_TRUE,
    .u32VolumedB           = 0x79,
    .bByBass               = MT_FALSE,
    .pIsrFunc              = HAL_AIAO_P_ProcStatistics,
};

static AIAO_PORT_USER_CFG_S g_stAiaoTxSpdDefaultOpenAttr =
{
    .stIfAttr             =
    {
        .enCrgMode        = AIAO_CRG_MODE_MASTER,
        .enChNum          = AIAO_I2S_CHNUM_2,
        .enBitDepth       = AIAO_BIT_DEPTH_16,
        .enRate           = AIAO_SAMPLE_RATE_48K,
        .u32FCLK_DIV      =                    128,
        .u32BCLK_DIV      =                      2,
    },
    .stBufConfig          =
    {
        .u32PeriodBufSize = AIAO_DF_PeriodBufSize,
        .u32PeriodNumber  = PERIOND_NUM,
    },
    .enTrackMode          = AIAO_TRACK_MODE_STEREO,
    .enFadeInRate         = AIAO_DF_FadeInRate,
    .enFadeOutRate        = AIAO_DF_FadeOutRate,
    .bMute                = MT_FALSE,
    .bMuteFade            = MT_TRUE,
    .u32VolumedB          = 0x79,
    .bByBass              = MT_FALSE,
    .pIsrFunc              = HAL_AIAO_P_ProcStatistics,
};


mt_void HAL_AIAO_P_SetTxI2SDfAttr(AIAO_PORT_ID_E enPortID, AIAO_IsrFunc      *pIsrFunc)//i2s only card set proc func MT_ALSA_I2S_ONLY_SUPPORT
{
    g_stAiaoTxI2SDefaultOpenAttr.pIsrFunc = pIsrFunc;
}

mt_void HAL_AIAO_P_GetTxI2SDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxI2SDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}

mt_void HAL_AIAO_P_GetHdmiHbrDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxHdmiHbrSDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}

mt_void HAL_AIAO_P_GetHdmiI2SDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxHdmiI2SSDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}

mt_void HAL_AIAO_P_GetTxSpdDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxSpdDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}


mt_void HAL_AIAO_P_GetRxAdcDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxI2SDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}

mt_void HAL_AIAO_P_GetRxSifDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxI2SDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}

mt_void HAL_AIAO_P_GetRxHdmiDfAttr(AIAO_PORT_ID_E enPortID, AIAO_PORT_USER_CFG_S *pAttr)
{
    memcpy(pAttr, &g_stAiaoTxI2SDefaultOpenAttr, sizeof(AIAO_PORT_USER_CFG_S));
}



#if defined (MT_I2S0_SUPPORT) || defined (MT_I2S1_SUPPORT)

mt_void HAL_AIAO_P_GetBorardTxI2SDfAttr(mt_u32 u32BoardI2sNum, MT_UNF_I2S_ATTR_S  *pstI2sAttr, AIAO_PORT_ID_E *penPortID,
                                        AIAO_PORT_USER_CFG_S *pAttr)
{
    return ;
}

mt_void HAL_AIAO_P_GetBorardRxI2SDfAttr(mt_u32 u32BoardI2sNum, AIAO_PORT_ID_E *penPortID,
                                        AIAO_PORT_USER_CFG_S *pAttr)
{
    return ;
}

mt_s32 HAL_AIAO_P_CheckBorardI2SOpenAttr(AIAO_PORT_ID_E enPortID, const AIAO_IfAttr_S     *pstNewIfAttr)
{
    return MT_SUCCESS;
}

mt_void HAL_AIAO_P_CrateBorardI2SOpenAttr(AIAO_PORT_ID_E enPortID, const AIAO_IfAttr_S     *pstNewIfAttr)
{
    return ;
}

mt_void HAL_AIAO_P_DestroyBorardI2SOpenAttr(AIAO_PORT_ID_E enPortID)
{
    return ;
}

#endif

#define IRQ_ARIA_AUDIO_IN_ID  49    //(17+32),Audio In
#define IRQ_ARIA_AUDIO_OUT_ID  50    //(18+32),Audio Out

#define AUD_SYS_CTRL_RST 0x18
#define AUD_SYS_CTRL_CLKEN 0x1C
#define AUD_SYS_CTRL_SRC_SEL 0x24
#define AUD_SYS_CTRL_FUNC_EN 0x2C

// audio clock 54M
#ifdef AUDIO_CLK_108M
#define MCLK_DIV_8k 0x09B583
#define MCLK_DIV_12k 0x0E9045
#define MCLK_DIV_16k 0x136B06
#define MCLK_DIV_24k 0x1D2089
#define MCLK_DIV_32k 0x26D60D
#define MCLK_DIV_48k 0x3A4114
#define MCLK_DIV_64k 0x4DAC1A
#define MCLK_DIV_96k 0x748224
#define MCLK_DIV_11k 0xD6159  //11.025
#define MCLK_DIV_22k 0x1AC2B2 //22.5
#define MCLK_DIV_44k 0x358564 //44.1
#define MCLK_DIV_88k 0x6B0AC8
#else
#define MCLK_DIV_8k 0x136B06
#define MCLK_DIV_12k 0x1D208A
#define MCLK_DIV_16k 0x26D60D
#define MCLK_DIV_24k 0x3A4113
#define MCLK_DIV_32k 0x4DAC1A
#define MCLK_DIV_48k 0x748229
#define MCLK_DIV_64k 0x9B5834
#define MCLK_DIV_96k 0xE90448
#define MCLK_DIV_11k 0x1AC2B2
#define MCLK_DIV_22k 0x358564
#define MCLK_DIV_44k 0x6B0AC8
#define MCLK_DIV_88k 0xD61590
#endif
static mt_u32 g_unClkDiv = MCLK_DIV_24k;

static inline mt_void hal_put_u32(volatile unsigned long *p_addr, unsigned long data)
{
	HAL_PUT_U32((volatile u32*)p_addr, (u32)data);
}
static inline mt_u32 hal_get_u32(volatile mt_u32 *p_addr)
{
	return (mt_u32)HAL_GET_U32((volatile u32*)p_addr);
}

#if defined(CONFIG_MT_CHIP_ARIA)
mt_s32 aria_ai_isr(mt_s32 irq, mt_void *dev_id)
{
    AIAO_PORT_ID_E enPortID = AIAO_PORT_RX0;

    mt_u32 Id = PORT2ID(enPortID);
    //down_interruptible(&g_HalAiaoMutex);

    AIAO_PORT_S hPort = g_AIAORm.hPort[Id];
    AIAO_PORT_USER_CFG_S *pstConfig = &hPort->stUserCongfig;

    MT_INFO_AIAO("audio_in clear isr\n");

    reg_audin_aria_set_audon_intr_set_i_frm_interrupt(1);
    HAL_AIAO_P_WriteData_Vsb(enPortID, NULL, pstConfig->stExtMem.u32samplecntoneframe * (pstConfig->stExtMem.u32bitdepth / 8));
    // up(&g_HalAiaoMutex);

    return IRQ_HANDLED;
}

mt_s32 aria_ao_isr(mt_s32 irq, mt_void *dev_id)
{
#if 0
	mt_u32 intr_cnt_2;
	mt_u32 frm_cnt_2;
	mt_u32 intr_cnt;
	mt_u32 frm_cnt;
	mt_u32 intr_set;
	mt_u32 clk_adj;
	mt_u32 samp_rate;
	//reg_aria_aud_clr_audfrm_intr(1);
	intr_cnt_2 = reg_aria_aud_get_audfrm_intr_2_cnt();
	//intr_cnt = reg_aria_aud_get_audfrm_intr_cnt();
	frm_cnt_2 = reg_aria_aud_get_aud_frm_cnt_2();
	//frm_cnt = reg_aria_aud_get_aud_frm_cnt();
	intr_set = reg_aria_aud_get_aud_intr_set();
	clk_adj = reg_aria_aud_get_aud_clk_adj();
	samp_rate = reg_aria_aud_get_aud_samp_frm();
	MT_INFO_AIAO("[ztq]ao_isr intr_cnt_2=%x, frm_cnt_2=%x,intr_set=%x,clk_adj=%d,samp_rate=%d\n",intr_cnt_2,frm_cnt_2,intr_set,clk_adj,samp_rate);
	//MT_INFO_AIAO("[ztq]ao_isr intr_cnt_2=%x, frm_cnt_2=%x,intr_set=%x,intr_cnt=%x, frm_cnt=%x\n",intr_cnt_2,frm_cnt_2,intr_set,intr_cnt,frm_cnt);
	#endif

	reg_aria_aud_clr_audfrm_intr_2(1);


	//MT_INFO_AIAO("[ztq]aria_ao_isr pop pts0.\n");
	//MT_INFO_AIAO("[ztq]aria_ao_isr pop pts1.\n");

    return IRQ_HANDLED;
}

#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 symphony_ai_isr(mt_s32 irq, mt_void *dev_id)
{
    irq =  irq;
    dev_id = dev_id;
    return IRQ_HANDLED;
}




irqreturn_t symphony_ao_isr(mt_s32 irq, mt_void *dev_id)
{
#if 0
	mt_u32 intr_cnt_2;
	mt_u32 frm_cnt_2;
	mt_u32 intr_cnt;
	mt_u32 frm_cnt;
	mt_u32 intr_set;
	mt_u32 clk_adj;
	mt_u32 samp_rate;
	//reg_aria_aud_clr_audfrm_intr(1);
	intr_cnt_2 = reg_aria_aud_get_audfrm_intr_2_cnt();
	//intr_cnt = reg_aria_aud_get_audfrm_intr_cnt();
	frm_cnt_2 = reg_aria_aud_get_aud_frm_cnt_2();
	//frm_cnt = reg_aria_aud_get_aud_frm_cnt();
	intr_set = reg_aria_aud_get_aud_intr_set();
	clk_adj = reg_aria_aud_get_aud_clk_adj();
	samp_rate = reg_aria_aud_get_aud_samp_frm();
	MT_INFO_AIAO("[ztq]ao_isr intr_cnt_2=%x, frm_cnt_2=%x,intr_set=%x,clk_adj=%d,samp_rate=%d\n",intr_cnt_2,frm_cnt_2,intr_set,clk_adj,samp_rate);
	//MT_INFO_AIAO("[ztq]ao_isr intr_cnt_2=%x, frm_cnt_2=%x,intr_set=%x,intr_cnt=%x, frm_cnt=%x\n",intr_cnt_2,frm_cnt_2,intr_set,intr_cnt,frm_cnt);
#endif
	//reg_symphony_aud_clr_audfrm_intr_2(1);
    {
    		ulong aout_reg_base;
            mt_u32 audio_reg = 0;

            aout_reg_base = mt_get_audioout_base();
            audio_reg = REG_READ((volatile u32*)(aout_reg_base + 0x00A0));     //AUDIO_INTR_SET
            REG_WRITE((volatile u32*)(aout_reg_base + 0x00A0), audio_reg);

            if(audio_reg & 0x1)
      	      {
               // do isr
              }

            if(audio_reg & 0x2)      // bb_buf_w_hang_up
            {
              // do isr
               MT_ASSERT(0);
               MT_INFO_AIAO("aud isr 0x2\n");
              }

           if(audio_reg & 0x4)     // buf_rw_intr
  	    {
               // do isr
               MT_INFO_AIAO("aud isr 0x4\n");
            }

           if(audio_reg & 0x8)    // pcm_fifo_empty
  	    {
                // do isr
               MT_INFO_AIAO("aud isr 0x8\n");
             }

             if(audio_reg & 0x10)    // pcm_fifo_cnt_diff bigger
  	     {
              // do isr
              MT_INFO_AIAO("aud isr 0x10\n");
               MT_ASSERT(0);
              }


             // this intr adding in B0
            if(audio_reg & 0x20)
             {

              }

	  }

    return IRQ_HANDLED;
}

#endif
mt_u32 HAL_AI_RequestIsr(mt_void)
{
    mt_u32 nRet = MT_SUCCESS;
    #if defined(CONFIG_MT_CHIP_ARIA)
    nRet = request_irq(IRQ_ARIA_AUDIO_IN_ID, aria_ai_isr, IRQF_TRIGGER_HIGH, "aria_audio_in_isr", MT_NULL);
    #elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    nRet = MT_SUCCESS;
    #endif

    MT_INFO_AIAO("\n\n request_irq  nret %d \n\n", nRet);
    return nRet;
}

mt_u32 HAL_AO_RequestIsr(mt_void)
{
    mt_u32 nRet = MT_SUCCESS;
    #if defined(CONFIG_MT_CHIP_ARIA)
    nRet = request_irq(IRQ_ARIA_AUDIO_OUT_ID, aria_ao_isr, IRQF_TRIGGER_HIGH, "aria_audio_out_isr", MT_NULL);
    #elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#if 0
    HAL_PUT_U32((volatile u32 *)0xbf10002c, (HAL_GET_U32((volatile u32 *)0xbf10002c) & 0x00ffffff) | 0x0f000000);
#endif
    nRet = request_irq((unsigned char)IRQ_AOUT_ID, symphony_ao_isr, IRQF_TRIGGER_HIGH, "symphony_audio_out_isr", MT_NULL);
    #endif
    MT_INFO_AIAO("\n\n [ztq]request_irq  nret %d \n\n", nRet);
    return nRet;
}

mt_void    HAL_AI_FreeIsr(mt_void)
{
    #if defined(CONFIG_MT_CHIP_ARIA)
    free_irq(IRQ_ARIA_AUDIO_IN_ID, MT_NULL);
    #elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    ;
    #endif
}

mt_void    HAL_AO_FreeIsr(mt_void)
{
    //MT_INFO_AIAO("\n\n [ztq]AO_isr free\n\n");
    #if defined(CONFIG_MT_CHIP_ARIA)
    free_irq(IRQ_ARIA_AUDIO_OUT_ID, MT_NULL);
    #elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    free_irq(IRQ_AOUT_ID, MT_NULL);
    #endif
}

#ifdef MT_AUDIO_AI_SUPPORT

//audio_in reset, clk gate, release clk, release reset
mt_void HAL_AI_Hw_Reset(mt_void)
{
    mt_u32 unData;

    //reset : set 0
    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST));

    unData = unData & 0xfffffffe; //audio_in_rst_n:bit0
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST), unData);

    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST));
    unData = unData & 0xfffffffd; //audio_in_axi_rst_n:bit1
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST), unData);

    //clk gate: set 1

    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN));
    unData = unData | 0x1; //audio_in_clk_gate : bit0
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN), unData);

    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN));
    unData = unData | 0x2; //audio_in_axi_gate : bit1
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN), unData);

    //clk release: set 0

    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN));
    unData = unData & 0xfffffffe; //audio_in_clk_gate : bit0
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN), unData);

    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN));
    unData = unData & 0xfffffffd; //audio_in_axi_gate : bit1
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_CLKEN), unData);

    //reset release: set 1
    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST));

    unData = unData | 0x1; //audio_in_rst_n:bit0
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST), unData);

    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST));
    unData = unData | 0x2; //audio_in_axi_rst_n:bit1
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_RST), unData);
}

//audio_in I/O cfg
mt_void HAL_AI_IO_config(mt_void)
{
    mt_u32 unData;

    //configure I/O: enable i2s port bit13
    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_FUNC_EN));
    unData = unData | (0x1 << 13);
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_FUNC_EN), unData);
}

mt_void HAL_AI_Clk_Srcsel(mt_void) //mt_void HAL_AI_Clk_src_sel()
{
    mt_u32 unData;
    //configure src_sel: reserved bit!!!!!!!!!!!!
    //when audio_out and audio_in combin,
    //this bit should be 1,means audio_out clk from pin,
    //when only use audio_out this bit should be 0,means audio_out clk from audio_out
    unData = hal_get_u32((volatile mt_u32 *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_SRC_SEL));

    unData = unData | (0x1 << 16);
    hal_put_u32((volatile unsigned long *)((ulong)mt_get_sys_ctrl_base() + AUD_SYS_CTRL_SRC_SEL), unData);
}

mt_void HAL_AI_Inter_En(mt_void)
{
    reg_audin_aria_set_audon_intr_set_frm_intr_en(1);
    reg_audin_aria_set_audon_intr_set_frm_intr_mask(0);
}

mt_void HAL_AI_Hw_Config(AIAO_PORT_ID_E enPortID)
{
    mt_u32 unAudinBuf0StartAddr;
    mt_u32 unAudinBuf1StartAddr;
    mt_u32 unAudinBufLength;
    //U32 unI2sInterlaceMode, U32 unAddrIncr, U32 unPlayByLine
    //U32 unI2sMode, U32 unEndianSel
    //U32 unFifoThd
    //U32 unWsBclkOe, U32 unMclkOe,
    //U32 unRecPcmNum,
    //U32 unClkDiv
    AIAO_PORT_S hPort;
    AIAO_PORT_USER_CFG_S *pstConfig;

    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
    pstConfig = &hPort->stUserCongfig;
    unAudinBuf0StartAddr = pstConfig->stExtMem.u32BufPhyAddr;
    unAudinBuf1StartAddr = unAudinBuf0StartAddr + pstConfig->stExtMem.u32bufszperch;
    unAudinBufLength = pstConfig->stExtMem.u32bufszperch;

    MT_INFO_AIAO("\n AI_HW buf0 [0x%x] buf1[0x%x]\n",unAudinBuf0StartAddr, unAudinBuf1StartAddr);

    reg_audin_aria_set_buf0_addr(unAudinBuf0StartAddr >> 3);
    reg_audin_aria_set_buf1_addr(unAudinBuf1StartAddr >> 3);
    reg_audin_aria_set_buf_length_buf_len(unAudinBufLength >> 3);

    reg_audin_aria_set_buf_mode_addr_incr_mode(0);  //circle buf :must be 0 for user

    if(pstConfig->stExtMem.bInterlaceMod == 0)
    {
      reg_audin_aria_set_buf_mode_audio_play_mode(1); //1:for audio out
      reg_audin_aria_set_buf_mode_interlace_mode(0);  //0:for audio out
    }
    else
   {
       reg_audin_aria_set_buf_mode_audio_play_mode(0); //1:for audio out
       reg_audin_aria_set_buf_mode_interlace_mode(1);  //0:for audio out
   }

    reg_audin_aria_set_i2s_fmt_just(0); //must match dac i2s mode
    reg_audin_aria_set_i2s_fmt_little_endian(0);
    reg_audin_aria_set_i2s_fmt_shift(0);
    reg_audin_aria_set_i2s_fmt_ws_pol(0);

    reg_audin_aria_set_bclk_oe_mclk_oe(1);
    //bclk and ws oe must be accordable,also must match dac setting
    reg_audin_aria_set_bclk_oe_bclk_oe(1);
    reg_audin_aria_set_bclk_oe_ws_oe(1);

    reg_audin_aria_set_frm_size_frm_size(pstConfig->stExtMem.u32samplecntoneframe);

    // must match dac setting
    reg_audin_aria_set_clk_div_cfg(g_unClkDiv);

    up(&g_HalAiaoMutex);
}

mt_u32 HAL_AI_Hw_Start(mt_void)
{
    mt_u32 ucptrend = 0;
    mt_u32 uaxibusend = 0;

    ucptrend = reg_audin_aria_get_cptr_en_audin_cptr_end_flag();
    uaxibusend = reg_audin_aria_get_cptr_en_axi_bus_end_flag();

    if (ucptrend && uaxibusend) {
	reg_audin_aria_set_cptr_en_audio_cptr_en(1);
	return 0;
    } else {
	return 1;
    }
}

mt_void HAL_AI_Hw_stop(mt_void)
{
    reg_audin_aria_set_cptr_en_audio_cptr_en(0);
}

mt_void HAL_AI_Hw_Pause(mt_void)
{
    reg_audin_aria_set_cptr_pause_audio_in_cptr_pause(1);
}

mt_void HAL_AI_Hw_Pause_Recovery(mt_void)
{
    reg_audin_aria_set_cptr_pause_audio_in_cptr_pause(1);
}

#endif

/************************************************************************************************************************************************************************************************/
/************************************************************************************************************************************************************************************************/
/************************************************************************************************************************************************************************************************/
/************************************************************************************************************************************************************************************************/

#define AUD_CH_SRT_CFG 0x00
#define AUD_I2S_SPDIF_CFG 0x04
#define AUD_VOL_CFG 0x08
#define AUD_PP_EN_CFG 0x0C
#define AUD_CLK_DIV_CFG 0x10
#define AUD_CLK_ADJ 0x14
#define AUD_OUT_BUF_BASE 0x18
#define AUD_OUT_BUF_LEN 0x1C
#define AUD_OUT_BUF_FUL_THD 0x20
#define AUD_OUT_BUF_WRCMD 0x24

#define AUD_PP_BUF_BASE 0x28
#define AUD_PP_BUF_LEN 0x2C
#define AUD_PP_BUF_FUL_THD 0x30
#define AUD_PP_BUF_WRCMD 0x34

#define AUD_SPD_BUF_BASE 0x38
#define AUD_SPD_BUF_LEN 0x3C
#define AUD_SPD_BUF_FUL_THD 0x40
#define AUD_SPD_BUF_WRCMD 0x44
#define AUD_OUT_BUF_JUMP 0x48
#define AUD_SRC_CFG 0x4C
#define AUD_AGC_CFG0 0x50
#define AUD_AGC_CFG1 0x54
#define AUD_AGC_CFG2 0x58
#define AUD_FADER_CFG 0x5C
#define AUD_DMX_CFG0 0x60
#define AUD_DMX_CFGn 0x7C
#define AUD_SYNC_RESET 0x80
#define AUD_DATA_PRELOAD 0x84
#define AUD_CLK_DIV_CFG_2 0x88
#define AUD_CLK_ADJ_2 0x8C
#define AUD_SAMP_NUM_FRM 0x90
#define AUD_PPBUFW_INTR_TIMER 0x94
#define AUD_BUFRW_INTR_TIMER 0x98
#define AUD_PPCMFIFO_INTR_TIMER 0x9C
#define AUD_INTR_SET 0xA0
#define AUD_SPD_BUF_JUMP 0xA4
#define AUD_VOL_CFG_2 0xA8
#define AUD_VOL_CFG_AD 0xAC
#define AUD_VOL_CFG_SPD 0xB0
#define AUD_OUT_FIFO_THD 0xB4
#define AUD_SPD_FIFO_THD 0xB8
#define AUD_AHB_DATA 0xBC
#define AUD_MIXBUF_BASE 0xC0
#define AUD_MIXBUF_LEN 0xC4
#define AUD_MIXBUF_FULTHD 0xC8
#define AUD_MIXBUF_WRCMD 0xCC
#define AUD_MIXBUF_CFG 0xD0
#define AUD_MIXBUF_ALPHA 0xD4
#define AUD_FRM_INTR_CFG 0xD8
#define AUD_PLAY_STOP 0xDC

#define AUD_OUT_BUF_RADDR 0x100
#define AUD_OUT_BUF_CNT 0x104
#define AUD_OUT_FIFO_CNT 0x108
#define AUD_OUT_BUF_WADDR 0x10C
#define AUD_PP_BUF_RADDR 0x110
#define AUD_PP_BUF0_CNT 0x114
#define AUD_PP_BUF1_CNT 0x118
#define AUD_PP_FIFO_CNT0 0x11c
#define AUD_PP_FIFO_CNT1 0x120
#define AUD_SPD_BUF_RADDR 0x124
#define AUD_SPD_BUF_CNT 0x128
#define AUD_BUF_FULL_FLAG 0x12C
#define AUD_FRMINTR_CNT 0x130
#define AUD_PPINTR_CNT 0x134
#define AUD_BUFINTR_CNT 0x138
#define AUD_PCMINTR_CNT 0x13C
#define AUD_FRM_CNT 0x140
#define AUD_FRM_CNT_2 0x144
#define AUD_PP_BUF2_CNT 0x150
#define AUD_PP_BUF3_CNT 0x154
#define AUD_PP_BUF4_CNT 0x158
#define AUD_PP_BUF5_CNT 0x15c
#define AUD_PP_BUF6_CNT 0x160
#define AUD_PP_BUF7_CNT 0x164
#define AUD_PP_SRC_CNT 0x168

#define AUD_RES_REG0 0x200
#define AUD_RES_REG1 0x204
#define AUD_RES_REG2 0x208
#define AUD_RES_REG3 0x20C

const int srcflt_coef_out[256] =
    {
        //table_2A
        0xffffff0a, 0xfffff548, 0xffffffb4, 0x0000029e, 0xfffffade, 0x000008ba, 0xfffff26a, 0x000013e7, 0xffffe43c, 0x00002566, 0xffffcf0e,
        0x00003ea9, 0xffffb12d, 0x000061df, 0xffff878b, 0x000093ae, 0xffff4a8a, 0x0000e17e, 0xfffee073, 0x00018315, 0xfffdb39f, 0x000513f2,
        0x000e18a1, 0xfffd74f2, 0x00013cf0, 0xffff4647, 0x0000731b, 0xffffb8cc, 0x000029d2, 0xffffea92, 0x00000726, 0x000002bd, 0xfffff6ab,
        0x00000d6a, 0xfffff071, 0x00001040, 0xfffff02b, 0x00000eae, 0xfffff2fb, 0x00000b30, 0xfffff67f, 0x000008aa, 0xfffff588, 0xfffffafa,
        //table_2B
        0x00000562, 0x0000550b, 0xfffe58eb, 0x00058622, 0x000c9794, 0xffff5694, 0xffffb935, 0x00002af1,
        //table_3A
        0xffffff78, 0xfffff6cc, 0xfffffede, 0x000005fb, 0xfffff9d5, 0x00000c53, 0xfffff0cf, 0x0000179c, 0xffffe2a8, 0x0000285d, 0xffffcec4,
        0x00003f42, 0xffffb44f, 0x00005d63, 0xffff91a8, 0x00008567, 0xffff6230, 0x0000bfc8, 0xffff1548, 0x000130c3, 0xfffe4b1e, 0x0003680a,
        0x000ee8da, 0xfffe3617, 0x0000c598, 0xffff9d44, 0x0000317f, 0xffffee1c, 0x00000025, 0x00000da8, 0xffffec21, 0x000019db, 0xffffe5bf,
        0x00001c27, 0xffffe6ad, 0x00001924, 0xffffeb60, 0x000013b2, 0xfffff171, 0x00000de8, 0xfffff6ec, 0x00000a2a, 0xfffff7c9, 0xfffffa7b,
        0xfffffdb7, 0xfffff51a, 0x000006cb, 0xfffffd10, 0x00000489, 0xffffff32, 0x0000007b, 0x0000055b, 0xfffff727, 0x00001219, 0xffffe5e7,
        0x0000286a, 0xffffc924, 0x00004cd2, 0xffff9ac6, 0x00008824, 0xffff4d00, 0x0000f1f6, 0xfffeb2dd, 0x0001ee02, 0xfffcac9b, 0x000a2a83,
        0x000a2a83, 0xfffcac9b, 0x0001ee02, 0xfffeb2dd, 0x0000f1f6, 0xffff4d00, 0x00008824, 0xffff9ac6, 0x00004cd2, 0xffffc924, 0x0000286a,
        0xffffe5e7, 0x00001219, 0xfffff727, 0x0000055b, 0x0000007b, 0xffffff32, 0x00000489, 0xfffffd10, 0x000006cb, 0xfffff51a, 0xfffffdb7,
        //table_3B
        0xffffff12, 0xffffec99, 0x00002a60, 0xffffcbb0, 0x000019a0, 0x00006ba9, 0xfffe07d3, 0x000c3e47, 0x000721e5, 0xfffd5c3f,
        0x00013893, 0xffff8531, 0x00001fc2, 0x000006a1, 0xfffff3c7, 0xfffffb4a, 0xffffefe7, 0x00003e56, 0xffff7a0b, 0x0000e902,
        0xfffeb6ec, 0x00019578, 0x000e5137, 0x00019578, 0xfffeb6ec, 0x0000e902, 0xffff7a0b, 0x00003e56, 0xffffefe7, 0xfffffb4a,
        0x0000017e, 0x000050ce, 0xfffeae4e, 0x00037fef, 0x000d900b, 0x000052b8, 0xffff6fce, 0x000035d6, //table_4A
        0x00000855, 0x00004d4c, 0xfffe3116, 0x000785da, 0x000b48b4, 0xfffe92aa, 0x00000759, 0x00001994,
        0xfffffb77, 0xffffb68c, 0x000a3582, 0x00069ff0, 0xffff8a6e, //table_4B
        0xffffe685, 0x0000c89d, 0x000c82d3, 0x0003277b, 0xffffb86b,
        0x00000310, 0x00036ad3, 0x000abbe4, 0x0001d7a4, //table_5A
        0x00001473, 0x0005792d, 0x00099e4f, 0x0000d590,
        0x00004d6d, 0x0007b34d, 0x0007b34d, 0x00004d6d,
        0xfffff68f, 0x00000000, 0x0000071b, 0x00000000, 0xfffff64a, 0x00000000, //table_2D
        0x00000cea, 0x00000000, 0xffffef35, 0x00000000, 0x0000157a, 0x00000000,
        0xffffe4e5, 0x00000000, 0x000021e2, 0x00000000, 0xffffd5ef, 0x00000000,
        0x00003413, 0x00000000, 0xffffbf83, 0x00000000, 0x0000504d, 0x00000000,
        0xffff9ace, 0x00000000, 0x00008267, 0x00000000, 0xffff512e, 0x00000000,
        0x0000fc7e, 0x00000000, 0xfffe5267, 0x00000000, 0x00051620, 0x00080000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x00000000, 0x00000000,
    };

mt_void HAL_AO_Hw_Start()
{
    //configure audio_out to play

    mt_u32 unPcmBufCnt = hal_get_u32((volatile mt_u32 *)(mt_get_audio_base() + AUD_OUT_BUF_CNT));

#ifdef MT_AUDIO_AI_SUPPORT
    if(0 ==  reg_audin_aria_get_buf_mode_audio_play_mode())
      return;
#endif

    while (unPcmBufCnt < 4096) {
	unPcmBufCnt = hal_get_u32((volatile mt_u32 *)(mt_get_audio_base() + AUD_OUT_BUF_CNT));
    }
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_PLAY_STOP), 0); //begin to play

    MT_INFO_AIAO("\n audio_out start to play \n");
}
mt_void HAL_AO_Hw_Config(AIAO_PORT_ID_E enPortID)
{
    mt_u32 unPPBufStartAddr;
    mt_u32 unPcmBufStartAddr;
    mt_u32 unPPBufLength;
    mt_u32 unPcmBufLength;

    mt_u32 unTemp, unCnt;
    mt_u32 unRecPcmNum;
    //U32 unI2sInterlaceMode, U32 unAddrIncr, U32 unPlayByLine
    //U32 unI2sMode, U32 unEndianSel
    //U32 unFifoThd
    //U32 unWsBclkOe, U32 unMclkOe,
    //U32 unRecPcmNum,
    //U32 unClkDiv
    AIAO_PORT_S hPort;
    AIAO_PORT_USER_CFG_S *pstConfig;

    mt_u32 Id = PORT2ID(enPortID);

    if (down_interruptible(&g_HalAiaoMutex))
    {
    	MT_ERR_AIAO("down_interruptible() failed!\n");
    }

    hPort = (AIAO_PORT_S)g_AIAORm.hPort[Id];
    pstConfig = &hPort->stUserCongfig;
    unPPBufStartAddr = pstConfig->stExtMem.u32BufPhyAddr;
    unPPBufLength = pstConfig->stExtMem.u32bufszperch;
    unPcmBufStartAddr = unPPBufStartAddr + unPPBufLength * 8;
    unPcmBufLength = unPPBufLength * 8;
    unRecPcmNum = pstConfig->stExtMem.u32samplecntoneframe; ///  /6/2

    MT_INFO_AIAO("\n AO_HW pp_buf [0x%x] pcm_buf[0x%x]\n",unPPBufStartAddr, unPcmBufStartAddr);

    //configure audio_out register

    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_CLK_DIV_CFG), g_unClkDiv);   // 2ch //48KHz
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_CLK_DIV_CFG_2), g_unClkDiv); // 2ch //48KHz

    unTemp = hal_get_u32((volatile mt_u32 *)(mt_get_audio_base() + AUD_I2S_SPDIF_CFG));
    unTemp = unTemp & 0xfffffff9;
    unTemp = unTemp | 0x04;
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_I2S_SPDIF_CFG), unTemp); // set i2s mode

    switch (g_unClkDiv) {
    case MCLK_DIV_8k: {
	unTemp = 0x0a;
	break;
    }
    case MCLK_DIV_11k: {
	unTemp = 0x08;
	break;
    }
    case MCLK_DIV_12k: {
	unTemp = 0x09;
	break;
    }
    case MCLK_DIV_16k: {
	unTemp = 0x02;
	break;
    }
    case MCLK_DIV_22k: {
	unTemp = 0x00;
	break;
    }
    case MCLK_DIV_24k: {
	unTemp = 0x01;
	break;
    }
    case MCLK_DIV_32k: {
	unTemp = 0x06;
	break;
    }
    case MCLK_DIV_44k: {
	unTemp = 0x04;
	break;
    }
    case MCLK_DIV_48k: {
	unTemp = 0x05;
	break;
    }
    default: {
	unTemp = 0x05;
	break;
    }
    }
    unTemp = unTemp | 0x300;
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_CH_SRT_CFG), unTemp); // //2 ch //48KHz
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_PP_BUF_BASE), unPPBufStartAddr >> 3);
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_PP_BUF_LEN), unPPBufLength >> 3);
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_PP_BUF_FUL_THD), 0);

    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_OUT_BUF_BASE), unPcmBufStartAddr >> 3);
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_OUT_BUF_LEN), unPcmBufLength >> 3);
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_OUT_BUF_FUL_THD), 0);
    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_SAMP_NUM_FRM), unRecPcmNum);

    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_PLAY_STOP), 1);

    for (unCnt = 0; unCnt < 256; unCnt++) {
	unTemp = srcflt_coef_out[unCnt] & 0x001fffff; //low 21bits
	unTemp |= 0x80000000;                         //wr_coef_en
	unTemp |= unCnt << 21;                        //wr_coef_addr
	hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_SRC_CFG), unTemp);
    }

    hal_put_u32((volatile unsigned long *)(mt_get_audio_base() + AUD_SYNC_RESET), 0x0); //clear  ppbuffer spdbuffer reset control

    MT_INFO_AIAO("\n configure over.... \n");

    up(&g_HalAiaoMutex);
}

#ifdef __cplusplus
#if __cplusplus

}

#endif
#endif/*__cplusplus*/
