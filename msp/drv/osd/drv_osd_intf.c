/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/cpufreq.h>


#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "mt_drv_avplay.h"
#include "mt_error_mpi.h"
#include "mt_drv_module.h"
#include "drv_osd_comm.h"
#include "mt_module.h"
//#include "drv_avplay_ext.h"
#include "mt_kernel_adapt.h"
//#include "drv_avplay_ioctl.h"
#include "mt_osal.h"
#include "mt_module_debug.h"


//#include "adp_layer.h"
//#include "mt_go_gdev.h"
//#include "mtgo_adp_sys.h"
//#include "mtgo_gdev.h"
//#include "mt_common.h"

#include "mtfb.h"
//#include "mt_drv_pdm.h"
#include "optm_mtfb.h"
//#include "mtfb_debug.h"
//#include "mt_tde_api.h"
//#include "adp_gfx.h"
//#include "mt_drv_disp.h"
//#include "drv_display.h"


#define MREAD(A) (*((volatile unsigned int *)((ulong)mt_get_display_base() + A)))
#define MWRITE(A, V) *((volatile unsigned int *)((ulong)mt_get_display_base() + A)) = (V)

#define    REG_ARIA_DISP_VIDEO_CTRL_1_OFS  0x0
#define    REG_ARIA_DISP_VIDEO_CTRL_2_OFS  0x4

#define    REG_ARIA_DISP_OSDL_OSD0_CMD_OFS  0x2000
#define    REG_ARIA_DISP_OSDL_OSD1_CMD_OFS  0x2004
#define    REG_ARIA_DISP_OSDL_SUB_CMD_OFS  0x2008
#define    REG_ARIA_DISP_OSDL_CMD_OFS  0x200c
#define    REG_ARIA_DISP_OSDL_OSD0_INI_ADDR_OFS  0x2010
#define    REG_ARIA_DISP_OSDL_OSD1_INI_ADDR_OFS  0x2014
#define    REG_ARIA_DISP_OSDL_SUB_INI_ADDR_OFS  0x2018


#include "mt_go_surface.h"
//#include "mtgo_gdev.h"

#include "drv_osd_intf.h"
#include "drv_osd_ioctrl.h"
#include "optm_hal.h"
#ifdef CFG_MTGO_PROC_SUPPORT
#include "mt_drv_proc.h"
#include "mt_gfx_comm_k.h"

#endif
#define OSD_BUF_SIZE  (1024 * 1024 * 20)


#define MT_OSD_DRV_LOG  MT_INFO_OSD

static OPTM_GFX_OPS_S g_stGfxOps;

static mmz_buffer_s g_osd_configbuf;
/***********************module ops **************************/

#ifdef CFG_MTGO_PROC_SUPPORT

MT_MTGO_PROC_INFO_S g_stMtgoProcInfo;
#endif


static mt_u32 g_osd_regs[] = \
{
  REG_ARIA_DISP_VIDEO_CTRL_1_OFS,
  REG_ARIA_DISP_VIDEO_CTRL_2_OFS,

  REG_ARIA_DISP_OSDL_OSD0_CMD_OFS,
  REG_ARIA_DISP_OSDL_OSD1_CMD_OFS,
  REG_ARIA_DISP_OSDL_SUB_CMD_OFS,
  REG_ARIA_DISP_OSDL_CMD_OFS,
  REG_ARIA_DISP_OSDL_OSD0_INI_ADDR_OFS,
  REG_ARIA_DISP_OSDL_OSD1_INI_ADDR_OFS,
  REG_ARIA_DISP_OSDL_SUB_INI_ADDR_OFS,
};

static mt_u32 g_osd_regs_len = sizeof(g_osd_regs) / 4;

static mt_void osd_reg_print(mt_void)
{
   mt_u32 regaddr = 0;
   mt_u32 regval = 0;
   mt_u32 idx = 0;

   for(idx = 0; idx < g_osd_regs_len; idx++)
   {
     regaddr = g_osd_regs[idx];
     regval =  MREAD(regaddr);
     printk("0x%x: %x\n", regaddr, regval);
   }

   return;
}


static mt_void osd_cw_print(ulong osd_phy_addr)
{
  //mt_u32 *vir_addr = 0;
//  mt_u32 cw[8];

#define HIGH_ADDR (890 * 1024 * 1024)   // 890M

  if(osd_phy_addr > HIGH_ADDR)
  {
     MT_ERR_OSD("osd_cw_print err: can not access hight addr : [%px]\n", osd_phy_addr);
     return;
  }

  //vir_addr = phy_to_virt(osd_phy_addr);
	return;
	/*
  printk("cw add: [%x] [%x]\n",  (ulong)vir_addr, osd_phy_addr);

  cw[0] = MREAD(vir_addr++);
  cw[1] = MREAD(vir_addr++);
  cw[2] = MREAD(vir_addr++);
  cw[3] = MREAD(vir_addr++);
  cw[4] = MREAD(vir_addr++);
  cw[5] = MREAD(vir_addr++);
  cw[6] = MREAD(vir_addr++);
  cw[7] = MREAD(vir_addr++);

  printk("cw[0]: [%x]\n",  cw[0]);
  printk("cw[1]: [%x]\n",  cw[1]);
  printk("cw[2]: [%x]\n",  cw[2]);
  printk("cw[3]: [%x]\n",  cw[3]);
  printk("cw[4]: [%x]\n",  cw[4]);
  printk("cw[5]: [%x]\n",  cw[5]);
  printk("cw[6]: [%x]\n",  cw[6]);
  printk("cw[6]: [%x]\n",  cw[7]);

  return;*/
}



static mt_void osd_info_print(mt_u32 osd_layer_id)
{
    mt_u32 osd_phy_addr = 0;
    mt_u32 osd_addr_reg = 0;

    osd_reg_print();

    if(osd_layer_id == 0)
    {
      printk("osd1 info: \n");
      osd_addr_reg = REG_ARIA_DISP_OSDL_OSD0_INI_ADDR;
    }
    else if(osd_layer_id == 1)
    {
      printk("osd1 info: \n");
      osd_addr_reg = REG_ARIA_DISP_OSDL_OSD1_INI_ADDR;
    }
    else if(osd_layer_id == 2)
    {
      printk("sub info: \n");
      osd_addr_reg = REG_ARIA_DISP_OSDL_SUB_INI_ADDR;
    }
    else
    {
      return;
    }

    osd_phy_addr = MREAD(osd_addr_reg);
    osd_reg_print();

    //if(osd_addr != 0)
    {
      osd_cw_print(osd_phy_addr);
    }

    return;
}



static mt_s32 osd_config_info_init(mt_void)
{
    //mt_u32 i;

    if (MT_SUCCESS == mt_drv_mmz_alloc_and_map("osd", MMZ_OTHERS,
            OSD_BUF_SIZE, 0, &g_osd_configbuf))
    {
        memset((mt_u8 *)g_osd_configbuf.startVirAddr, 0, OSD_BUF_SIZE);
        //g_log_configinfo = (log_config_info_s *)g_log_configbuf.u32StartVirAddr;
        MT_OSD_DRV_LOG("osd_config_info_init ok.\n");
    }
    else
    {
        MT_ERR_OSD("osd_config_info_init failed.\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}



static mt_s32 _drv_osd_layer_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg)
{
    mt_s32           ret = 0;
    OSD_IOC_PARAM_S *pPara = (OSD_IOC_PARAM_S *)arg;
    MTFB_ALPHA_S stAlpha;
    MTFB_RECT stRect;
    MTFB_COLORKEYEX_S stColorKey;

//    MT_OSD_DRV_LOG("_drv_osd_layer_Ioctl IN: [%x], [%x]\n", cmd, (mt_u32)arg);

     switch (cmd)
     {
         case CMD_IOC_GFX_INIT:
             ret = g_stGfxOps.OPTM_GfxInit();
             break;
         case CMD_IOC_GFX_OPEN_LAYER:
             ret = g_stGfxOps.OPTM_GfxOpenLayer(pPara->LayerID, pPara->EnableOsdc);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.layerUsed[pPara->LayerID] = MT_TRUE;
#endif
             break;
         case CMD_IOC_GFX_CLOSE_LAYER:
             ret = g_stGfxOps.OPTM_GfxCloseLayer(pPara->LayerID);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.layerUsed[pPara->LayerID] = MT_FALSE;
              g_stMtgoProcInfo.surfaceUsed[pPara->LayerID] = MT_FALSE;

#endif
             break;
         case CMD_IOC_GFX_OPEN_SLV_LAYER:
             break;
         case CMD_IOC_GFX_SET_LAYER_ALPHA:
             ret = copy_from_user(&stAlpha, (void __user *)pPara->pstAlpha, sizeof(MTFB_ALPHA_S));
			 if (ret != 0)
			    break;
             ret = g_stGfxOps.OPTM_GfxSetLayerAlpha(pPara->LayerID, &stAlpha);
#ifdef CFG_MTGO_PROC_SUPPORT
             if(stAlpha.bAlphaEnable)
               g_stMtgoProcInfo.alpha[pPara->LayerID] = stAlpha.u8GlobalAlpha;
             else
               g_stMtgoProcInfo.alpha[pPara->LayerID] = 0;
             if(stAlpha.bRegionAlphaEnable)
              g_stMtgoProcInfo.surAlpha[pPara->LayerID] = stAlpha.u8RegionAlpha;
             else
              g_stMtgoProcInfo.surAlpha[pPara->LayerID] = 0;
#endif
             break;
         case CMD_IOC_GFX_SET_LAYER_ADDR:
             ret = g_stGfxOps.OPTM_GfxSetLayerAddr(pPara->LayerID, pPara->u32Addr);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.layerAddr[pPara->LayerID] = pPara->u32Addr;
#endif
             break;
         case CMD_IOC_GFX_SET_LAYER_STRIDE:
             ret = g_stGfxOps.OPTM_GfxSetLayerStride(pPara->LayerID, pPara->u32Stride);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.layerPitch[pPara->LayerID] = pPara->u32Stride;
#endif
             break;
         case CMD_IOC_GFX_SET_LAYER_DATA_FMT:
             ret = g_stGfxOps.OPTM_GfxSetLayerDataFmt(pPara->LayerID, pPara->enDataFmt);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.format[pPara->LayerID] = pPara->enDataFmt;
#endif
             break;
         case CMD_IOC_GFX_SET_LAYER_RECT:
             ret = copy_from_user(&stRect, (void __user *)pPara->pstRect, sizeof(MTFB_RECT));
		     if (ret != 0)
			     break;
             ret = g_stGfxOps.OPTM_GfxSetLayerRect(pPara->LayerID, &stRect);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.rect[pPara->LayerID].x = stRect.x;
              g_stMtgoProcInfo.rect[pPara->LayerID].y = stRect.y;
              g_stMtgoProcInfo.rect[pPara->LayerID].w = stRect.w;
              g_stMtgoProcInfo.rect[pPara->LayerID].h = stRect.h;
#endif
             break;
         case CMD_IOC_GFX_SET_LAYKEY_MASK:
             ret = copy_from_user(&stColorKey, (void __user *)pPara->pstColorKey, sizeof(MTFB_COLORKEYEX_S));
		     if (ret != 0)
			     break;
             ret = g_stGfxOps.OPTM_GfxSetLayKeyMask(pPara->LayerID, &stColorKey);
#ifdef CFG_MTGO_PROC_SUPPORT
             if(stColorKey.bKeyEnable)
               g_stMtgoProcInfo.colorkey[pPara->LayerID] = stColorKey.u32Key;
             else
              g_stMtgoProcInfo.colorkey[pPara->LayerID] = 0;
#endif
             break;
         case CMD_IOC_WBC2ISR:
             break;
         case CMD_IOC_GFX_SET_ENABLE:
             ret = g_stGfxOps.OPTM_GfxSetEnable(pPara->LayerID, pPara->bEnable);
#ifdef CFG_MTGO_PROC_SUPPORT
             g_stMtgoProcInfo.layerVisible[pPara->LayerID] = pPara->bEnable;

#endif
             break;
         case CMD_IOC_GFX_SET_GP_RECT:
             break;
         case CMD_IOC_GFX_UP_LAYER_REG:
             ret = g_stGfxOps.OPTM_GfxSetColorReg(pPara->LayerID, pPara->u32OffSet, pPara->u32Color, pPara->UpFlag);
             break;
         case CMD_IOC_GET_GFX_WORK_MODE:
             break;
         case CMD_IOC_GP_INIT_FROM_DISP:
             break;
         case CMD_IOC_GFX_SET_DISP_FMT_SIZE:
             break;
         case CMD_IOC_GFX_SET_TC_FLAG:
             break;
         case CMD_IOC_PRINT_OSD_INFO:
            osd_reg_print();
            break;
         case CMD_IOC_PRINT_OSD_CW_INFO:
            osd_info_print(pPara->LayerID);
            break;
         case CMD_IOC_GFX_CMP_DECMP_PROCESS:
            ret = g_stGfxOps.OPTM_GFX_CMP_DECMP_Process(pPara->LayerID, pPara->u32RdAddr,
                                                            pPara->pic_width, pPara->pic_height, pPara->u32Stride, pPara->u32Addr);
            break;
#ifdef CFG_MTGO_PROC_SUPPORT
         case CMD_IOC_GFX_PROC_SURFACE_INFO:
            g_stMtgoProcInfo.surfaceUsed[pPara->LayerID] = MT_TRUE;
            g_stMtgoProcInfo.surPhyAddr[pPara->LayerID] = pPara->u32Addr;
            g_stMtgoProcInfo.surFormat[pPara->LayerID] = pPara->enDataFmt;
            g_stMtgoProcInfo.surPitch[pPara->LayerID] = pPara->u32Stride;
            g_stMtgoProcInfo.surWidth[pPara->LayerID] = pPara->pic_width;
            g_stMtgoProcInfo.surHeight[pPara->LayerID] = pPara->pic_height;

            break;
#endif
        case CMD_IOC_GFX_WAIT_SYNC:
            ret = g_stGfxOps.OPTM_GfxWaitSync();
            break;

         default:
             break;

     }

//    MT_OSD_DRV_LOG("_drv_osd_layer_Ioctl OUT: [%x]\n", cmd);
    return ret;
}

static mt_s32 drv_osd_layer_open(struct inode *finode, struct file  *ffile)
{
/*
    mt_s32            Ret;

    Ret = down_interruptible(&g_AvplayMutex);

    if (1 == atomic_inc_return(&g_AvplayCount))
    {
    }

    up(&g_AvplayMutex);
*/
    return 0;
}

static mt_s32 drv_osd_layer_close(struct inode *finode, struct file  *ffile)
{
    return 0;
}

static long drv_osd_layer_ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    mt_s32 Ret;

    Ret = mt_drv_usercopy(ffile->f_path.dentry->d_inode, ffile, cmd, arg, _drv_osd_layer_Ioctl);

    return Ret;
}

static struct file_operations g_drv_osd_layer_FOPS =
{
    .owner          =  THIS_MODULE,
    .open           =  drv_osd_layer_open,
    .unlocked_ioctl =  drv_osd_layer_ioctl,
    .release        =  drv_osd_layer_close,
};


static mt_s32 drv_osd_layer_suspend(basedev_s *pdev, pm_message_t state)
{
    MT_PRINT("drv_osd_layer_suspend OK\n");
    return 0;
}


static mt_s32 drv_osd_layer_resume(basedev_s *pdev)
{
    MT_PRINT("drv_osd_layer_resume OK\n");

    return 0;
}


static mt_s32 drv_osd_get_header_info(MT_DRV_DISP_LAYER_ID_E disp_layer, osd_header_info_s *info)
{
    mt_s32           ret = 0;
    MTFB_LAYER_ID_E  mtfb_layer = MTFB_LAYER_OSD0;

    switch (disp_layer)
    {
        case DISP_LAYER_ID_OSD0:
            mtfb_layer = MTFB_LAYER_OSD0;
            break;      
        case DISP_LAYER_ID_OSD1:
            mtfb_layer = MTFB_LAYER_OSD1;
            break;        
        case DISP_LAYER_ID_SUBTITL:
            mtfb_layer = MTFB_LAYER_SUB;
            break;
        default:
             MT_PRINT("layer is not supported!!\n");
            break;
    }
    
    ret = g_stGfxOps.OPTM_GfxGetOsdHeader(mtfb_layer, info);
    
    return ret;
}

static mt_device_s          g_osd_layer_device;
#define OSD_LAYER_NAME         "MT_OSD_LAYER"



static baseops_s g_osd_layer_DRVOPS = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = drv_osd_layer_suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = drv_osd_layer_resume,
};

#ifdef CFG_MTGO_PROC_SUPPORT

//#define PROC_PRINT(s,arg...) ({printk(arg) ;})

void osd_mem_dump(struct seq_file *p, phys_addr_t phys_addr, mt_u32 mem_size)
{
    //unsigned long phys_addr = p_info->layerAddr[i]);
    void  *virt_addr;
    mt_u32 offset = 0;
    //unsigned int mem_size = p_info->layerPitch[i] * p_info->rect[i].h;
    //unsigned int palette_size = 0;
    // Ó³ÉäÎïÀíµØÖ·µ½ÐéÄâµØÖ·
    virt_addr = __va((ulong)phys_addr);
    if (!virt_addr) {
        PROC_PRINT(p, "ioremap failed\n");
        return ;
    }
    
    for(offset = 0; offset < mem_size / 4; offset++)
    {   
        if(offset%16 == 0)
        {   
            PROC_PRINT(p,"\n");                  
        } 
        PROC_PRINT(p,"0x%08x ", *((mt_u32 *)(virt_addr + (4 *offset)))); 
    }
    

}

static mt_s32 MTGO_Read_Proc(struct seq_file *p, mt_void *v)
{
  mt_proc_entry_t *item = (mt_proc_entry_t *)(p->private);
  MT_MTGO_PROC_INFO_S* p_info    = (MT_MTGO_PROC_INFO_S *)(item->data);
  int i;
  int tmp;
  phys_addr_t phys_addr = 0;
  mt_u32 *header_addr = 0;
  unsigned int mem_size = 0;
  unsigned int palette_size = 0;

  mt_u8* layer[] =
  {
    "MTFB_LAYER_BACKGROUND",
    "MTFB_LAYER_OSD0",
    "MTFB_LAYER_OSD1",
    "MTFB_LAYER_SUB",
    "MTFB_LAYER_STILL",
    "MTFB_LAYER_HD_0",
    "MTFB_LAYER_HD_1",
    "MTFB_LAYER_HD_2",
    "MTFB_LAYER_HD_3",
    "MTFB_LAYER_SD_0",
    "MTFB_LAYER_SD_1",
    "MTFB_LAYER_SD_2",
    "MTFB_LAYER_SD_3",
    "MTFB_LAYER_AD_0",
    "MTFB_LAYER_AD_1",
    "MTFB_LAYER_AD_2",
    "MTFB_LAYER_AD_3",
    "MTFB_LAYER_CURSOR"
  };

  tmp = 0;
  for(i = MTFB_LAYER_BACKGROUND; i < MTFB_LAYER_ID_BUTT; i++)
  {
    if(p_info->layerUsed[i] == MT_TRUE)
      tmp++;
  }
  PROC_PRINT(p,"LayerUsed\t%d\n", tmp);
  tmp = 0;
  for(i = MTFB_LAYER_BACKGROUND; i < MTFB_LAYER_ID_BUTT; i++)
  {
    if(p_info->surfaceUsed[i] == MT_TRUE)
      tmp++;
  }
  PROC_PRINT(p,"SurfaceUsed\t%d\n", tmp);

  PROC_PRINT(p,"cat layer\n");
  for(i = MTFB_LAYER_BACKGROUND; i < MTFB_LAYER_ID_BUTT; i++)
  {
    if(p_info->layerUsed[i] == MT_TRUE)
    {
      PROC_PRINT(p,"---------%s---------\n",layer[i]);
      PROC_PRINT(p,"Visible\t:%s\n", (p_info->layerVisible[i] == MT_TRUE) ? "TRUE" : "FALSE");
      PROC_PRINT(p,"layerAddr\t:0x%08llx\n", p_info->layerAddr[i]);
      PROC_PRINT(p,"Pitch\t:%d\n", p_info->layerPitch[i]);
      PROC_PRINT(p,"Format\t:%d\n", p_info->format[i]);
      PROC_PRINT(p,"FlushType\t:%s\n", "DOUBLE");
      PROC_PRINT(p,"Position\t:(%d, %d)\n", p_info->rect[i].x, p_info->rect[i].y);
      PROC_PRINT(p,"CanvasSurface\t:resolution(%d, %d)\n", p_info->surWidth[i], p_info->surHeight[i]);
      PROC_PRINT(p,"DisplaySurface\t:resolution(%d, %d)\n", p_info->rect[i].w, p_info->rect[i].h);
      PROC_PRINT(p,"Alpha\t:%d\n", p_info->alpha[i]);
      PROC_PRINT(p,"ColorKey\t:%d\n", p_info->colorkey[i]);
      if(p_info->layerAddr[i] != 0)
      {
          phys_addr = p_info->layerAddr[i];
          mem_size = p_info->surPitch[i] * p_info->rect[i].h;
          header_addr = __va((ulong)p_info->headerAddr[i]);
          PROC_PRINT(p,"OSD size:%x, cw[0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x]:\n", 
            mem_size,
            header_addr[0],header_addr[1],header_addr[2],header_addr[3],
            header_addr[4],header_addr[5],header_addr[6],header_addr[7]);
          
          osd_mem_dump(p, phys_addr, mem_size);
          
          switch (p_info->format[i])
          {
              case MTFB_FMT_1BPP:          /**<  clut1 */
                  palette_size = 2 * 4;
                  break;
              
              case MTFB_FMT_2BPP:          /**<  clut2 */ 
              case MTFB_FMT_2BPP_ABGR:
              case MTFB_FMT_2BPP_RGBA:
              case MTFB_FMT_2BPP_BGRA:
              case MTFB_FMT_2BPP_ARGB:  
              case MTFB_FMT_2BPP_AVUY:
              case MTFB_FMT_2BPP_YUVA:
                  palette_size = 4 * 4;
                  break;

              case MTFB_FMT_4BPP:            /**<  clut4 */
                  palette_size = 16 * 4;
                  break;

              case MTFB_FMT_8BPP:          /**< clut8 */                  
              case MTFB_FMT_ACLUT44:       /**< AClUT44*/                  
              case MTFB_FMT_8BPP_ABGR:
              case MTFB_FMT_8BPP_RGBA:
              case MTFB_FMT_8BPP_BGRA:
              case MTFB_FMT_8BPP_ARGB:
              case MTFB_FMT_8BPP_AVUY:
              case MTFB_FMT_8BPP_YUVA:
              case MTFB_FMT_8BPP_VUYA:
              case MTFB_FMT_8BPP_AYUV:
              case MTFB_FMT_ACLUT44_ABGR:
              case MTFB_FMT_ACLUT44_RGBA:
              case MTFB_FMT_ACLUT44_BGRA:
              case MTFB_FMT_ACLUT44_ARGB:
              case MTFB_FMT_ACLUT44_AVUY:
              case MTFB_FMT_ACLUT44_YUVA:
              case MTFB_FMT_ACLUT44_VUYA:
              case MTFB_FMT_ACLUT44_AYUV: 
                  palette_size = 256 * 4;
                  break;

              case MTFB_FMT_ACLUT88:      /**< ACLUT88 */
              case MTFB_FMT_ACLUT88_ABGR:
              case MTFB_FMT_ACLUT88_RGBA:
              case MTFB_FMT_ACLUT88_BGRA:
              case MTFB_FMT_ACLUT88_ARGB:
              case MTFB_FMT_ACLUT88_AVUY:
              case MTFB_FMT_ACLUT88_YUVA:
              case MTFB_FMT_ACLUT88_VUYA:
              case MTFB_FMT_ACLUT88_AYUV:	                
                  palette_size = 256 * 256 * 4;
                  break;
              default:
                  palette_size = 0;

          }
                             
          if(palette_size != 0 && p_info->headerAddr[i] != 0)
          {   
              PROC_PRINT(p,"\n p data size:%x:\n", palette_size); 
              phys_addr = (p_info->headerAddr[i] + (4 * 16));            
              osd_mem_dump(p, phys_addr, palette_size);
          }

      }
    }
  }
  PROC_PRINT(p,"\n cat surface\n");
  for(i = MTFB_LAYER_BACKGROUND; i < MTFB_LAYER_ID_BUTT; i++)
  {
    if(p_info->surfaceUsed[i] == MT_TRUE)
    {
      PROC_PRINT(p,"width\theight\tpitch\tphyaddr\tformat\talpha \n:%d\t%d\t%d\t0x%08x\t%d\t%d\n",
        p_info->surWidth[i], p_info->surHeight[i], p_info->surPitch[i], p_info->surPhyAddr[i], p_info->surFormat[i], p_info->surAlpha[i]);
    }
  }
  return 0;
}


/*****************************************************************************
* Function     : OSD_Proc_init
* Description  :
* param[in]    : NA
* retval       : NA
*****************************************************************************/
MT_VOID OSD_Proc_init(MT_VOID)
{


		GFX_PROC_ITEM_S pProcItem;
		MT_CHAR *pEntry_name = "mtgo";

		pProcItem.fnRead   = MTGO_Read_Proc;
		pProcItem.fnWrite  = NULL;
		pProcItem.fnIoctl  = NULL;

		MT_GFX_PROC_AddModule(pEntry_name,&pProcItem,(MT_VOID *)&g_stMtgoProcInfo);

}
/*****************************************************************************
* Function     : OSD_Proc_Cleanup
* Description  :
* param[in]    :
* param[in]    :
* Output       :
* retval       :
* retval       :
* others:	   :notmtng
*****************************************************************************/

MT_VOID OSD_Proc_Cleanup(MT_VOID)
{
     MT_CHAR *pEntry_name =  "mtgo";
     MT_GFX_PROC_RemoveModule(pEntry_name);
}

#endif


static OSD_EXPORT_FUNC_S s_stDispExportFuncs = {
    .pfnOsdGetHeaderInfo = drv_osd_get_header_info,
};

mt_s32 __init drv_osd_layer_ModInit(mt_void)
{
    mt_s32      Ret;

    MT_INFO_OSD("drv_osd_layer_ModInit\n");
    Ret = mt_drv_module_register(MT_ID_OSD, OSD_LAYER_NAME, &s_stDispExportFuncs);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_OSD("ERR: MT_DRV_OSD_MODULE_Register, Ret = %#x!\n", Ret);
    }


    mt_osal_snprintf(g_osd_layer_device.devfs_name, sizeof(g_osd_layer_device.devfs_name), UMAP_DEVNAME_OSD);
    g_osd_layer_device.fops = &g_drv_osd_layer_FOPS;
    g_osd_layer_device.minor = UMAP_MIN_MINOR_OSD;
    g_osd_layer_device.owner  = THIS_MODULE;
    g_osd_layer_device.drvops = &g_osd_layer_DRVOPS;

    if (mt_drv_dev_register(&g_osd_layer_device) < 0)
    {
        MT_ERR_OSD("register osd layer failed.\n");
        return MT_FAILURE;
    }
    OPTM_GFX_GetOps(&g_stGfxOps);

    if(0)
    {
      osd_config_info_init();
    }
#ifdef CFG_MTGO_PROC_SUPPORT
     OSD_Proc_init();
#endif

    MT_INFO_OSD("drv_osd_layer_ModInit out \n");

    return  0;
}


mt_void __exit drv_osd_layer_ModExit(mt_void)
{
    MT_INFO_OSD("drv_osd_layer_ModExit: in\n");
#ifdef CFG_MTGO_PROC_SUPPORT
     OSD_Proc_Cleanup();
#endif
    mt_drv_dev_unregister(&g_osd_layer_device);
    mt_drv_module_unregister(MT_ID_OSD);

    MT_INFO_OSD("drv_osd_layer_ModExit: out\n");
    return;
}

