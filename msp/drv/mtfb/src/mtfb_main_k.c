/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */

#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/delay.h>

#include <linux/slab.h>
#include <linux/mm.h>

#include <linux/fb.h>
#include <linux/uaccess.h>

#include <asm/types.h>
#include <asm/stat.h>
#include <asm/fcntl.h>

#include <linux/interrupt.h>
#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_module_debug.h"

#include "drv_mtfb_ext.h"
#include "drv_pdm_ext.h"
#include "mtfb_p.h"
#include "optm_define.h"

/**
 **寄存器相关配置
 **/
#include "mtfb_config.h"
#include "mt_gfx_comm_k.h"



#define MTFB_NAME "MT_FB"


#ifndef MT_ADVCA_FUNCTION_RELEASE
#define MTFB_PRINT_INFO  MT_INFO_MTFB
#else
#define MTFB_PRINT_INFO(x...)
#endif

#if 0
#define MTFB_FUN_IN        printk("%s, LINE IN: %d\n", __FUNCTION__, __LINE__)
#define MTFB_FUN_OUT     printk("%s, LINE OUT: %d\n", __FUNCTION__, __LINE__)
#define MTFB_LOG              printk
#define  MTFB_LINE            printk("%s, LINE: %d\n", __FUNCTION__, __LINE__)
#else
#define  DUMP_LOG            do{}while(0);
#define MTFB_FUN_IN        DUMP_LOG
#define MTFB_FUN_OUT     DUMP_LOG
#define MTFB_LOG(...)         DUMP_LOG
#define  MTFB_LINE            DUMP_LOG
#endif

mt_void MtfbSetLogoLayerEnable(MT_BOOL bEnable);
mt_s32 MtfbUpdatePqData(mt_u32 u32UpdateType, PQ_PARAM_S* pstPqParam);

static volatile mt_u32 *g_u32HDLogoCtrlReg = MT_NULL;
static volatile mt_u32 *g_u32SDLogoCtrlReg = MT_NULL;
static volatile mt_u32 *g_u32SDCtrlReg   = MT_NULL;
static volatile mt_u32 *g_u32WBCCtrlReg  = MT_NULL;

/// temp , need remove
#define CONFIG_MTFB_HD_LOGO_REG_BASEADDR        (0x00)
#define CONFIG_MTFB_SD_LOGO_REG_BASEADDR        (0x00)
#define CONFIG_MTFB_WBC_SLAYER_REG_BASEADDR  (0x00)
#define CONFIG_MTFB_WBC_GP0_REG_BASEADDR        (0x00)

  


static MTFB_EXPORT_FUNC_S s_MtfbExportFuncs =
{
    .pfnMtfbSetLogoLayerEnable             = MtfbSetLogoLayerEnable,
    .pfnMtfbUpdatePqData                   = MtfbUpdatePqData,    
};

static MT_BOOL gs_bLogoOff = MT_FALSE;

mt_void MtfbSetLogoLayerEnable(MT_BOOL bEnable)
{
    PDM_EXPORT_FUNC_S *ps_PdmExportFuncs = MT_NULL;

    MTFB_FUN_IN;
    MT_INFO_MTFB("err in\n");
    MTFB_FUN_OUT;
    return;
    
    if (bEnable) 
    {
        *g_u32HDLogoCtrlReg |= (0x1 << 31);
		*g_u32SDLogoCtrlReg |= (0x1 << 31);
        *g_u32SDCtrlReg   |= (0x1 << 31);
        *g_u32WBCCtrlReg  |= (0x1 << 31);

        *(g_u32HDLogoCtrlReg + 1) = 0x1;
		*(g_u32SDLogoCtrlReg + 1) = 0x1;
        *(g_u32SDCtrlReg + 1)   = 0x1;
        *(g_u32WBCCtrlReg + 1)  = 0x1;

    }
    else
    {
    	if (gs_bLogoOff)
		{
		   MTFB_FUN_OUT;
			return;
		}
		
        *g_u32HDLogoCtrlReg &= ~(0x1 << 31);
		*g_u32SDLogoCtrlReg &= ~(0x1 << 31);
        *g_u32SDCtrlReg   &= ~(0x1 << 31);
        *g_u32WBCCtrlReg  &= ~(0x1 << 31);

        *(g_u32HDLogoCtrlReg + 1) = 0x1;
		*(g_u32SDLogoCtrlReg + 1) = 0x1;
        *(g_u32SDCtrlReg + 1)   = 0x1;
        *(g_u32WBCCtrlReg + 1)  = 0x1;

		gs_bLogoOff = MT_TRUE;
	
        msleep(40);

        /* free the reserve memory*/
        if (MT_SUCCESS != mt_drv_module_getfunction(MT_ID_PDM, (mt_void**)&ps_PdmExportFuncs))
        {
          MTFB_FUN_OUT;
        	return;
        }
		
        if(MT_NULL != ps_PdmExportFuncs)
        {
            ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(DISPLAY_BUFFER_HD);
			ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(DISPLAY_BUFFER_SD);
            ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(OPTM_GFX_WBC2_BUFFER);
            ps_PdmExportFuncs->pfnPDM_ReleaseReserveMem(MTFB_ZME_COEF_BUFFER);
        }
    }

    MTFB_FUN_OUT;
    return;
}


/***************************************************************************************
* func          : mtfb_init_module_k
* description   : CNcomment: 加载KO的初始化 CNend\n
* param[in]     : mt_void
* retval        : NA
* others:       : NA
***************************************************************************************/
mt_s32 mtfb_init_module_k(mt_void)
{
    mt_s32 ret;

    MTFB_FUN_IN;
    
#if 0    
	/** 高清层logo地址映射,只映射32位 **/
    g_u32HDLogoCtrlReg  = (mt_u32*)ioremap_nocache(CONFIG_MTFB_HD_LOGO_REG_BASEADDR, 8);
	g_u32SDLogoCtrlReg  = (mt_u32*)ioremap_nocache(CONFIG_MTFB_SD_LOGO_REG_BASEADDR, 8);
	g_u32SDCtrlReg       = (mt_u32*)ioremap_nocache(CONFIG_MTFB_WBC_SLAYER_REG_BASEADDR, 8);
	g_u32WBCCtrlReg      = (mt_u32*)ioremap_nocache(CONFIG_MTFB_WBC_GP0_REG_BASEADDR, 8);
#endif

	/** 函数导出 **/
   ret = MT_GFX_MODULE_Register(MTGFX_FB_ID, MTFB_NAME, &s_MtfbExportFuncs);

    if (MT_SUCCESS != ret)
    {
        MTFB_PRINT_INFO("MT_DRV_MODULE_Register failed\n");
        
        mtfb_cleanup_module_k();
        MTFB_FUN_OUT;
        return ret;
    }

    MTFB_FUN_OUT;
    return MT_SUCCESS;
}

mt_void mtfb_cleanup_module_k(mt_void)
{
    MTFB_FUN_IN;
    
    MT_GFX_MODULE_UnRegister(MTGFX_FB_ID);
    
#if 0    
    iounmap(g_u32HDLogoCtrlReg);
	iounmap(g_u32SDLogoCtrlReg);
	iounmap(g_u32SDCtrlReg);
	iounmap(g_u32WBCCtrlReg);
#endif

   MTFB_FUN_OUT;
   return;
}

extern MTFB_DRV_OPS_S s_stDrvOps;
#define MTFB_DISPCHN_HD 1
#define MTFB_DISPCHN_SD 0


mt_s32 MtfbUpdatePqData(mt_u32 u32UpdateType, PQ_PARAM_S* pstPqParam)
{/** 98M/hifone should no set defliker,pq set by self**/
#ifdef MT_PQ_V1_0
    switch (u32UpdateType)
    {
        case PQ_CMD_VIRTUAL_GFX_DEFLICKER:
        {
            if(pstPqParam->stPQCoef.stGfxCoef.u32HdCtrlEn)
            {
                s_stDrvOps.MTFB_DRV_SetGpDeflicker(MTFB_DISPCHN_HD, MT_TRUE);
            }
            else
            {
                s_stDrvOps.MTFB_DRV_SetGpDeflicker(MTFB_DISPCHN_HD, MT_FALSE);
            }
            
            if(pstPqParam->stPQCoef.stGfxCoef.u32SdCtrlEn)
            {
                s_stDrvOps.MTFB_DRV_SetGpDeflicker(MTFB_DISPCHN_SD, MT_TRUE);
            }
            else
            {
                s_stDrvOps.MTFB_DRV_SetGpDeflicker(MTFB_DISPCHN_SD, MT_FALSE);
            }
            break;
        }
        default:
            break;
     }
#endif

    return MT_SUCCESS;
} 


