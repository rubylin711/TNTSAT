
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
 ******************************************************************************
 File Name     : drv_display.h
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
 ******************************************************************************/

#include <linux/types.h>
#include <linux/sizes.h>	/*SZ_1K*/

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/irq.h"
#include "drv_display.h"
#include "drv_disp_com.h"
#include "drv_disp_priv.h"
#include "drv_win_priv.h"
#include "drv_osd_comm.h"

#include "mt_error_mpi.h"
#include "drv_disp_isr.h"
#include "drv_disp_cast.h"
#include "mt_drv_module.h"
#include "sd_enc_aria_reg.h"
#include "hd_enc_aria_reg.h"

#include "drv_disp_Symphony_reg.h"
#include "../../../dma/hal_dma_regs.h"


#include "drv_timer.h"
#include "mt_module_debug.h"

//#include "drv_disp_da.h"
#include "drv_hdmi_ext.h"
//#include "mt_drv_sys.h"
#include "drv_hdmi_ioctl.h"

#include "mt_mach/chipinfo.h"
#include "mt_common.h"
#include "mt_cache.h"
#include "drv_hdmi.h"

#include "../../../inc/analog/mt_analog_parameter.h"

#ifndef CONFIG_HDMI
#define CONFIG_HDMI
#endif

#ifndef MT_32BYTE_ALIGN
#define MT_32BYTE_ALIGN 32
#endif

/******************************************************************************
  global object
 ******************************************************************************/

#define CLOCK_DIV_1001

//vdac detect
#define REG_SYMPHONY_ANA_AO_REG1_ADDR   0xBF157004

extern MT_BOOL b_disp_coeff_update; 
static volatile mt_s32 s_s32DisplayGlobalFlag = DISP_DEVICE_STATE_CLOSE;
static DISP_DEV_S s_stDisplayDevice;
ulong p_hdenc_addr;
ulong p_sdenc_addr;
ulong p_disp_addr;
ulong p_vbi_addr;

mt_u32 no_sd = 0;
mt_u32 g_vdec_width;
mt_u32 g_vdec_height;
atomic_t smallwindow_flag[MAX_WIN_NUM];
mt_rect_s smallwindow_rect[MAX_WIN_NUM];
atomic_t cropwindow_flag[MAX_WIN_NUM];
mt_rect_s cropwindow_rect[MAX_WIN_NUM];
atomic_t tvsyschange_flag;

MT_DRV_DISP_PPMODE_E PPMode = MT_DRV_DISP_PPMODE_DEFAULT;
MT_BOOL LayerEnable[DISP_LAYER_ID_MAX] = {0};
MT_BOOL AfdEnable = MT_FALSE;
MT_BOOL SdScalerEnable = MT_FALSE;
disp_sys_t SdVidSys = VID_SYS_PAL;
disp_sys_t HdVidSys = VID_SYS_1080I_50HZ;
MT_BOOL HdOutEnable = MT_TRUE;
MT_BOOL SdOutEnable = MT_TRUE;
MT_DRV_ASPECT_RATIO_E out_ar;
/* OPTM ISR handle */
mt_u32 g_DispIrqHandle = 0;
static HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;

static volatile mt_u32 reset_delay_cnt = 0;
static volatile mt_u32 update_sys_flag = 0;
static volatile mt_s32 g_slhdr_enable = 0;

static disp_priv_t s_stDisplayPriv;
disp_vbi_buffer_s disp_vbi_buffer;
static MT_DRV_DISP_HDMI_MODE_E g_tv_capability = MT_DRV_DISP_HDMI_MODE_SDR;

__attribute__((unused))static tv_mode_t old_used_tv_mode = TV_MODE_SDR;
static phys_addr_t u32SdWbAddr = 0;


int hdr10p_vsif_mode = 1;
module_param_named(hdr10p_vsif_mode, hdr10p_vsif_mode, int, 0644);
EXPORT_SYMBOL(hdr10p_vsif_mode);

int usr_force_bitdepth = 0;
module_param_named(usr_force_bitdepth, usr_force_bitdepth, int, 0644);
EXPORT_SYMBOL(usr_force_bitdepth);

int usr_force_hdr10 = 0;
module_param_named(usr_force_hdr10, usr_force_hdr10, int, 0644);
EXPORT_SYMBOL(usr_force_hdr10);


bool b_uboot_tvsys_uninit = TRUE;
EXPORT_SYMBOL(b_uboot_tvsys_uninit);

int __init uboot_tvsys_enable(char *str)
{
    b_uboot_tvsys_uninit = FALSE;
    return 1;
}
__setup("uboot_tvsys_set", uboot_tvsys_enable);
module_param_named(uboot_tvsys_set, b_uboot_tvsys_uninit,
        bool, S_IRUGO | S_IWUSR);

bool b_sd_wrback_422 = FALSE;
EXPORT_SYMBOL(b_sd_wrback_422);

int __init set_sd_wrback_422(char *str)
{
    b_sd_wrback_422 = TRUE;
    return 1;
}
__setup("sd_wrback_422", set_sd_wrback_422);
module_param_named(sd_wrback_422, b_sd_wrback_422,
        bool, S_IRUGO | S_IWUSR);

//SD Write Back Addr: priority use the module parameter,
//                    if module param is zero,
//                    then allocate from the MMZ.
static ulong sd_wrback_addr = 0;
module_param(sd_wrback_addr, ulong, S_IRUGO | S_IWUSR);

static ulong sd_wrback_field_num = 6;
module_param(sd_wrback_field_num, ulong, S_IRUGO | S_IWUSR);

extern DF_DRV_SETTING_S g_stDrvSetting[];



typedef enum{
    CLKEN_OFF = 0,
    CLKEN_ON = 1,
}crm_clken_status_e;

typedef enum{
    DISPVDC_CLKSEL_594M=0,
    DISPVDC_CLKSEL_480M,
    DISPVDC_CLKSEL_615M,
    DISPVDC_CLKSEL_AXI
}dispvdc_clksel_e;

typedef enum{
    DISPDI_CLKSEL_288M=0,
    DISPDI_CLKSEL_262M,
    DISPDI_CLKSEL_160M,
    DISPDI_CLKSEL_60M
}dispdi_clksel_e;

typedef enum{
    DISPOSDC_CLKSEL_262M=0,
    DISPOSDC_CLKSEL_247M,
    DISPOSDC_CLKSEL_206M,
    DISPOSDC_CLKSEL_144M
}disposdc_clksel_e;


typedef enum{
    DISPCORE_CLKSEL_615M=0,
    DISPCORE_CLKSEL_594M,
    DISPCORE_CLKSEL_320M,
    DISPCORE_CLKSEL_206M,
    DISPCORE_CLKSEL_130M,
    DISPCORE_CLKSEL_60M
}dispcore_clksel_e;



#define SYMPHONY_CRM_REG_BASE SYMPHONY_IO_VA(0xBF500000)

#define DISP_CLKEN_REG (SYMPHONY_CRM_REG_BASE + 0xA300)
#define DISP_CLKSEL_REG (SYMPHONY_CRM_REG_BASE + 0xA304)
#define DISP_SRSTN_REG (SYMPHONY_CRM_REG_BASE + 0xA30C)
#define DISP_SLOCK_REG (SYMPHONY_CRM_REG_BASE + 0xA318)
#define DISP_LOCK_REG (SYMPHONY_CRM_REG_BASE + 0xA31C)
#define VOUT_CLKEN_REG (SYMPHONY_CRM_REG_BASE + 0xA600)
#define VOUT_CLKSEL_REG (SYMPHONY_CRM_REG_BASE + 0xA604)
#define VOUT_SRSTN_REG (SYMPHONY_CRM_REG_BASE + 0xA60C)
#define VOUT_SLOCK_REG (SYMPHONY_CRM_REG_BASE + 0xA618)
#define VOUT_LOCK_REG (SYMPHONY_CRM_REG_BASE + 0xA61C)


#define BIT_DISPVDC_MASK (0x20)
#define BIT_DISPOSDC_MASK (0x10)
#define BIT_DISPDI_MASK (0x8)
#define BIT_DISPCORE_MASK (0x4)
#define BIT_DISPAXI_MASK (0x2)
#define BIT_DISPAHB_MASK (0x1)

#define BIT_DISPVDC_LOCK (0x1 <<10)
#define BIT_DISPOSDC_LOCK (0x1 <<9)
#define BIT_DISPDI_LOCK (0x1 <<8)
#define BIT_DISPCORE_LOCK (0x1 <<7)
#define BIT_DISPAXI_LOCK (0x1 <<6)
#define BIT_DISPAHB_LOCK (0x1 <<5)
#define BIT_DISPVDC_CLKSEL_LOCK (0x1 <<4)
#define BIT_DISPOSDC_CLKSEL_LOCK (0x1 <<3)
#define BIT_DISPDI_CLKSEL_LOCK (0x1 <<2)
#define BIT_DISPCORE_CLKSEL_LOCK (0x1 <<1)
#define BIT_DISP_CLKEN_LOCK (0x1 <<0)

#define BIT_VOUT_HDMI_CLKEN_MASK (0x10)
//#define BIT_VOUT_LCD_CLKEN_MASK (0x8)
#define BIT_VOUT_HDVENC_CLKEN_MASK (0x4)
#define BIT_VOUT_VBI_CLKEN_MASK (0x2)
#define BIT_VOUT_SDVENC_CLKEN_MASK (0x1)

#define BIT_VOUT_HDVENC_HD_MASK (0x1 <<4)
#define BIT_VOUT_HDVENC_AHB_MASK (0x1 <<3)
#define BIT_VOUT_VBI_MASK (0x1 <<2)
#define BIT_VOUT_SDVENC_AHB_MASK (0x1 <<1)
#define BIT_VOUT_SDVENC_CORE_MASK (0x1 <<0)

#define BIT_VOUT_HDVENC_HD_LOCK (0x1 <<18)
#define BIT_VOUT_HDVENC_AHB_LOCK (0x1 <<17)
#define BIT_VOUT_SDVENC_AHB_LOCK (0x1 <<15)
#define BIT_VOUT_SDVENC_CORE_LOCK (0x1 <<14)
//#define BIT_VOUT_LCDC_CLK_DIVCFG_LOCK (0x1 <<8)
//#define BIT_VOUT_LCDHD_CLKSEL_LOCK (0x1 <<7)
//#define BIT_VOUT_LCD_CLKSEL_LOCK (0x1 <<6)
#define BIT_VOUT_HDVENC_CLKSEL_LOCK (0x1 <<5)
#define BIT_VOUT_HDMI_CLKEN_LOCK (0x1 <<4)
//#define BIT_VOUT_LCD_CLKEN_LOCK (0x1 <<3)
#define BIT_VOUT_HDVENC_CLKEN_LOCK (0x1 <<2)
#define BIT_VOUT_VBI_CLKEN_LOCK (0x1 <<1)
#define BIT_VOUT_SDVENC_CLKEN_LOCK (0x1 <<0)


typedef  struct
{
    mt_u32 start;
    mt_u32 size;
}regs_map_t;

#define DISP_TOTAL_REGS_BLOCKS  11

#define DISP_REG_OFFSET_1   0x0
#define DISP_REG_OFFSET_2   0x1000
#define DISP_REG_OFFSET_3   0x2000
#define DISP_REG_OFFSET_4   0x3000
#define DISP_REG_OFFSET_5   0x4000
#define DISP_REG_OFFSET_6   0x6000
#define DISP_REG_OFFSET_7   0x7000
#define DISP_REG_OFFSET_8   0x8000
#define DISP_REG_OFFSET_9   0x9090
#define DISP_REG_OFFSET_10  0x9300
#define DISP_REG_OFFSET_11  0xc000

#define SIZE_DISP_REG_SECTOR_1  (0x800 / 4)     //0xbf440000 ~ 0xbf4407fc
#define SIZE_DISP_REG_SECTOR_2  (0x7c / 4)      //0xbf441000 ~ 0xbf441078
#define SIZE_DISP_REG_SECTOR_3  (0x3c0 / 4)     //0xbf442000 ~ 0xbf4423bc
#define SIZE_DISP_REG_SECTOR_4  (0x128 / 4)     //0xbf443000 ~ 0xbf443124
#define SIZE_DISP_REG_SECTOR_5  (0x1cc / 4)     //0xbf444000 ~ 0xbf4441c8
#define SIZE_DISP_REG_SECTOR_6  (0x2e8 / 4)     //0xbf446000 ~ 0xbf4462e4
#define SIZE_DISP_REG_SECTOR_7  (0x154 / 4)     //0xbf447000 ~ 0xbf447154
#define SIZE_DISP_REG_SECTOR_8  (0x64 / 4)      //0xbf448000 ~ 0xbf448060
#define SIZE_DISP_REG_SECTOR_9  (0xc4 / 4)      //0xbf449090 ~ 0xbf4490c0
#define SIZE_DISP_REG_SECTOR_10 (0xd8 / 4)      //0xbf449300 ~ 0xbf4493d4
#define SIZE_DISP_REG_SECTOR_11 (0x10 / 4)      //0xbf44c000 ~ 0xbf44c00c

#define SIZE_DISP_REG_ALL (SIZE_DISP_REG_SECTOR_1 + SIZE_DISP_REG_SECTOR_2 +\
        SIZE_DISP_REG_SECTOR_3 + SIZE_DISP_REG_SECTOR_4 + SIZE_DISP_REG_SECTOR_5 +\
        SIZE_DISP_REG_SECTOR_6 + SIZE_DISP_REG_SECTOR_7 + SIZE_DISP_REG_SECTOR_8 +\
        SIZE_DISP_REG_SECTOR_9 + SIZE_DISP_REG_SECTOR_10 + SIZE_DISP_REG_SECTOR_11)
static mt_u32 disp_regs[SIZE_DISP_REG_ALL];
regs_map_t disp_regs_map[] = {
    {DISP_REG_OFFSET_1, SIZE_DISP_REG_SECTOR_1},
    {DISP_REG_OFFSET_2, SIZE_DISP_REG_SECTOR_2},
    {DISP_REG_OFFSET_3, SIZE_DISP_REG_SECTOR_3},
    {DISP_REG_OFFSET_4, SIZE_DISP_REG_SECTOR_4},
    {DISP_REG_OFFSET_5, SIZE_DISP_REG_SECTOR_5},
    {DISP_REG_OFFSET_6, SIZE_DISP_REG_SECTOR_6},
    {DISP_REG_OFFSET_7, SIZE_DISP_REG_SECTOR_7},
    {DISP_REG_OFFSET_8, SIZE_DISP_REG_SECTOR_8},
    {DISP_REG_OFFSET_9, SIZE_DISP_REG_SECTOR_9},
    {DISP_REG_OFFSET_10, SIZE_DISP_REG_SECTOR_10},
    {DISP_REG_OFFSET_11, SIZE_DISP_REG_SECTOR_11}
};


static mt_u32 hdvenc_regs[0x40];
static mt_u32 sdvenc_regs[0x40];

static MT_DRV_DISP_FMT_E get_tvsys_from_reg(mt_u32 ch);
static void disp_csc_update(disp_priv_t *p_dp);
//static void disp_denoise_update(void);
static void disp_dce_cfg(MT_DRV_DISP_PPMODE_E ppMode);
void disp_hal_set_sd_venc(disp_priv_t *p_dp);

static void disp_hal_set_sd_venc_ram(void);
static void disp_hal_set_sd_venc_ram_for_pal(void);
static void disp_hal_set_sd_venc_ram_for_ntsc(void);

static void disp_update_cvbs_for_sd_tvsys(disp_priv_t *p_dp, MT_BOOL b_on);
static mt_u32 add_crc6(mt_u32 data);
static mt_void disp_backup(mt_void);
static mt_void disp_restore(mt_void);
static mt_s32 disp_hdmi_video_config(disp_priv_t *p_dp);


//static HDMI_EXPORT_FUNC_S* s_pstHDMIFunc;
disp_priv_t *get_disp_priv_handle(void);
mt_void disp_st_vid_get_vout_size(disp_channel_t ch, mt_u32 *p_height, mt_u32 *p_width);
mt_s32 disp_aria_cvbs_onoff(disp_priv_t *p_disp, cvbs_dacgrp_t grp_id, MT_BOOL b_on);
void drv_disp_get_hdmi_edid (void);

/******************************************************************************
  local function and macro
 ******************************************************************************/
#define DEF_DRV_DISP_INTER_FUNCTION_AND_MACRO_START_HERE

#define DispCheckDeviceState()                                                                                 \
{                                                                                                              \
    if (DISP_DEVICE_STATE_OPEN != s_s32DisplayGlobalFlag)                                                      \
    {                                                                                                          \
        DISP_ERROR("DISP ERROR! DISP is not inited in %s,status:%d!\n", __FUNCTION__, s_s32DisplayGlobalFlag); \
        return MT_ERR_DISP_NO_INIT;                                                                            \
    }                                                                                                          \
}

#define DispCheckCastHandleValid(handle)                                                                       \
{                                                                                                              \
    if (((handle >> 16) != MT_ID_DISP) || (((handle & 0xffff) != MT_DRV_DISPLAY_0) &&                          \
                ((handle & 0xffff) != MT_DRV_DISPLAY_1) && ((handle & 0xffff) != MT_DRV_DISPLAY_2)))                     \
    {                                                                                                          \
        DISP_ERROR("DISP ERROR! bad cast handle in  %s!\n", __FUNCTION__);                                     \
        return MT_ERR_DISP_INVALID_PARA;                                                                       \
    }                                                                                                          \
}

#define DispCheckNullPointer(ptr)                                            \
{                                                                            \
    if (!ptr)                                                                \
    {                                                                        \
        DISP_ERROR("DISP ERROR! Input null pointer in %s!\n", __FUNCTION__); \
        return MT_ERR_DISP_NULL_PTR;                                         \
    }                                                                        \
}

#define DispGetPointerByID(id, ptr)                                          \
{                                                                            \
    if (id >= MT_DRV_DISPLAY_BUTT)                                           \
    {                                                                        \
        DISP_ERROR("DISP ERROR! Invalid display in %s!\n", __FUNCTION__);    \
        return MT_ERR_DISP_INVALID_PARA;                                     \
    }                                                                        \
    ptr = &s_stDisplayDevice.stDisp[id - MT_DRV_DISPLAY_0];                  \
}

#define DispGetPointerByIDNoReturn(id, ptr)                                  \
{                                                                            \
    if (id >= MT_DRV_DISPLAY_BUTT)                                           \
    {                                                                        \
        DISP_ERROR("DISP ERROR! Invalid display in %s!\n", __FUNCTION__);    \
    }                                                                        \
    ptr = &s_stDisplayDevice.stDisp[id - MT_DRV_DISPLAY_0];                  \
}

#define DispShouldBeOpened(id)                                 \
{                                                              \
    if (!DISP_IsOpened(id))                                    \
    {                                                          \
        DISP_WARN("DISP ERROR! Display is not opened!\n");     \
        return MT_ERR_DISP_NOT_OPEN;                           \
    }                                                          \
}

#define NUM2PERCENT(Num)   ((Num) * 100 + 127) / 255

static void disp_st_tvsys_update(MT_DRV_DISPLAY_E enDisp);
static void disp_aria_gra_scale_update(MT_DRV_DISPLAY_E enDisp);
static void disp_set_hdmi(disp_priv_t *p_dp);

static void disp_aria_hdmi_tasklet_cb_work(struct work_struct *work);

static void disp_hdmi_setformat_work(struct work_struct *work);


/*!
  Get 32 bits register value

  \param[in] p_addr register address

  \return register value
  */
static inline mt_u32 disp_hal_get_u32(volatile mt_u32 *p_addr)
{
    /*!
      Get 32 bits register value
      */
    return HAL_GET_U32((volatile mt_u32*)p_addr);
}

/*!
  Write 32 bits register

  \param[in] addr register address
  \param[in] p_addr data to write
  */
static inline void disp_hal_put_u32(volatile mt_u32 *p_addr, mt_u32 data)
{
    /*!
      Write 32 bits register
      */
    HAL_PUT_U32((volatile mt_u32*)p_addr, (mt_u32)data);
}

static int disp_reg_set_valid_bit(volatile mt_u32 *reg, u8 sbit, u8 size, u32 val)
{
    u32 tmp = 0, vbit = 0, cur_val = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = disp_hal_get_u32(reg);
    cur_val = (tmp >> sbit) & vbit; //get old value from register
    if(val == cur_val)
        return 0;
    tmp &= ~(vbit << sbit);
    tmp |= (val << sbit);
    disp_hal_put_u32(reg, tmp);

    return 0;
}

__attribute__((unused)) static unsigned long disp_reg_get_valid_bit(volatile mt_u32 *reg, unsigned int sbit, unsigned int size, unsigned long *val)
{
    unsigned long tmp = 0, vbit = 0;

    vbit = ((u32)(0xffffffff) >> (32 - size));
    tmp = disp_hal_get_u32(reg);
    *val = (tmp >> sbit) & vbit; //get old value from register
    return 0;
}
//Unified CRM sub module reset interface
static void crm_reset(volatile mt_u32 *addr, unsigned int bit_mask)
{
    u32 val = 0;

    val = disp_hal_get_u32((volatile mt_u32 *)addr);
    val &= ~(bit_mask);
    disp_hal_put_u32((volatile mt_u32 *)addr, val);
}

static void crm_release(volatile mt_u32 *addr, unsigned int bit_mask)
{
    u32 val = 0;

    val = disp_hal_get_u32((volatile mt_u32 *)addr);
    val |= (bit_mask);
    disp_hal_put_u32((volatile mt_u32 *)addr, val);
}

static MT_BOOL crm_feature_is_locked(volatile mt_u32 *reg, mt_u32 mask)
{
    if ((disp_hal_get_u32(reg) & mask) != 0)
        return MT_TRUE;
    return MT_FALSE;
}

static MT_BOOL disp_crm_locked(mt_u32 mask)
{
    if(crm_feature_is_locked((volatile mt_u32 *)DISP_LOCK_REG, mask)
            ||crm_feature_is_locked((volatile mt_u32 *)DISP_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

__attribute__((unused)) static int disp_crm_clken(crm_clken_status_e status)
{
    if(disp_crm_locked(BIT_DISP_CLKEN_LOCK))
        return 0;
    disp_hal_put_u32((volatile mt_u32 *)DISP_CLKEN_REG, status);
    return 1;
}

__attribute__((unused)) static int disp_crm_clk_enabled(void)
{
    return (disp_hal_get_u32((volatile mt_u32 *)DISP_CLKEN_REG)& 0x1);
}

__attribute__((unused)) static int disp_crm_vdc_clksel(dispvdc_clksel_e type)
{
    if(disp_crm_locked(BIT_DISPVDC_CLKSEL_LOCK))
        return 0;

    disp_reg_set_valid_bit((volatile mt_u32 *)DISP_CLKSEL_REG, 12, 2, type);
    return 1;
}

__attribute__((unused)) static int disp_crm_osdc_clksel(disposdc_clksel_e type)
{
    if(disp_crm_locked(BIT_DISPOSDC_CLKSEL_LOCK))
        return 0;

    disp_reg_set_valid_bit((volatile mt_u32 *)DISP_CLKSEL_REG, 6, 2, type);
    return 1;
}

__attribute__((unused)) static int disp_crm_di_clksel(dispdi_clksel_e type)
{
    if(disp_crm_locked(BIT_DISPDI_CLKSEL_LOCK))
        return 0;

    disp_reg_set_valid_bit((volatile mt_u32 *)DISP_CLKSEL_REG, 3, 2, type);
    return 1;
}

__attribute__((unused)) static int disp_crm_core_clksel(dispcore_clksel_e type)
{
    if(disp_crm_locked(BIT_DISPCORE_CLKSEL_LOCK))
        return 0;

    disp_reg_set_valid_bit((volatile mt_u32 *)DISP_CLKSEL_REG, 0, 3, type);

    return 1;
}

__attribute__((unused)) static int disp_crm_vdc_reset(void)
{
    if(disp_crm_locked(BIT_DISPVDC_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPVDC_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_vdc_release(void)
{
    if(disp_crm_locked(BIT_DISPVDC_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPVDC_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_osdc_reset(void)
{
    if(disp_crm_locked(BIT_DISPOSDC_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPOSDC_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_osdc_release(void)
{
    if(disp_crm_locked(BIT_DISPOSDC_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPOSDC_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_di_reset(void)
{
    if(disp_crm_locked(BIT_DISPDI_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPDI_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_di_release(void)
{
    if(disp_crm_locked(BIT_DISPDI_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPDI_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_core_reset(void)
{
    if(disp_crm_locked(BIT_DISPCORE_LOCK))
        return 0;
    crm_reset((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPAHB_MASK); //fpga test temply disable -- dean debug
    crm_reset((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPAXI_MASK);
    crm_reset((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPCORE_MASK);

    return 1;
}

__attribute__((unused)) static int disp_crm_core_release(void)
{
    if(disp_crm_locked(BIT_DISPCORE_LOCK))
        return 0;
    crm_release((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPCORE_MASK);
    crm_release((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPAXI_MASK);
    crm_release((volatile mt_u32 *)DISP_SRSTN_REG, BIT_DISPAHB_MASK);    //fpga test temply disable -- dean debug

    return 1;
}

__attribute__((unused)) static MT_BOOL vout_crm_locked(mt_u32 mask)
{
    if(crm_feature_is_locked((volatile mt_u32 *)VOUT_LOCK_REG, mask)
            ||crm_feature_is_locked((volatile mt_u32 *)VOUT_SLOCK_REG, mask))
        return MT_TRUE;
    return MT_FALSE;
}

__attribute__((unused)) static int vout_crm_vbi_clken(crm_clken_status_e status)
{
    if(vout_crm_locked(BIT_VOUT_VBI_CLKEN_LOCK))
        return 0;

    disp_reg_set_valid_bit((volatile mt_u32 *)VOUT_CLKEN_REG, 1, 1, status);
    return 1;
}

__attribute__((unused)) static int vout_crm_hdvenc_clken(crm_clken_status_e status)
{
    if(vout_crm_locked(BIT_VOUT_HDVENC_CLKEN_LOCK))
        return 0;

    disp_reg_set_valid_bit((volatile mt_u32 *)VOUT_CLKEN_REG, 2, 1, status);
    return 1;
}

__attribute__((unused)) static int vout_crm_sdvenc_clken(crm_clken_status_e status)
{
    if(vout_crm_locked(BIT_VOUT_SDVENC_CLKEN_LOCK))
        return 0;
    disp_reg_set_valid_bit((volatile mt_u32 *)VOUT_CLKEN_REG, 0, 1, status);
    return 1;
}


__attribute__((unused)) static int vout_crm_sdvenc_core_reset(void)
{
    if(vout_crm_locked(BIT_VOUT_SDVENC_CORE_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_SDVENC_CORE_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_sdvenc_core_release(void)
{
    if(vout_crm_locked(BIT_VOUT_SDVENC_CORE_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_SDVENC_CORE_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_sdvenc_ahb_reset(void)
{
    if(vout_crm_locked(BIT_VOUT_SDVENC_AHB_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_SDVENC_AHB_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_sdvenc_ahb_release(void)
{
    if(vout_crm_locked(BIT_VOUT_SDVENC_AHB_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_SDVENC_AHB_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_hdvenc_hd_reset(void)
{
    if(vout_crm_locked(BIT_VOUT_HDVENC_HD_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_HDVENC_HD_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_hdvenc_hd_release(void)
{
    if(vout_crm_locked(BIT_VOUT_HDVENC_HD_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_HDVENC_HD_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_hdvenc_ahb_reset(void)
{
    if(vout_crm_locked(BIT_VOUT_HDVENC_AHB_LOCK))
        return 0;

    crm_reset((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_HDVENC_AHB_MASK);
    return 1;
}

__attribute__((unused)) static int vout_crm_hdvenc_ahb_release(void)
{
    if(vout_crm_locked(BIT_VOUT_HDVENC_AHB_LOCK))
        return 0;

    crm_release((volatile mt_u32 *)VOUT_SRSTN_REG, BIT_VOUT_HDVENC_AHB_MASK);
    return 1;
}

disp_priv_t *get_disp_priv_handle(void)
{
    disp_priv_t *p_dp = &s_stDisplayPriv;
    return p_dp;
}

mt_void DispSetHardwareState(mt_void)
{
    // s_stDisplayDevice.bHwReseted = MT_TRUE;
    return;
}

mt_void DispClearHardwareState(mt_void)
{
    // s_stDisplayDevice.bHwReseted = MT_FALSE;
    return;
}

mt_s32  DispResetHardware(MT_BOOL b_clock_hi)
{
    /****************
     *1. backup layer stat
     *2. close all layer
     *3. set disposdc_clk / dispdi_clk / dispcore_clk
     *4. Clock Domain Reset: SDVENC HDVENC DISP_AXI DISP_AHB DISP_CORE DISP_DI DISP_VDC DISP_OSDC
     *5. open layer
     ****************/
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 ret = 0;

    MT_BOOL osd0_en = drv_reg_4k_disp_get_osdl_osd0_cmd_osd_layer_en();
    MT_BOOL osd1_en = drv_reg_4k_disp_get_osdl_osd1_cmd_osd_layer_en();
    MT_BOOL sub_en = drv_reg_4k_disp_get_osdl_sub_cmd_osd_layer_en();
    MT_BOOL video_en = drv_reg_4k_disp_get_video_ctrl_1_video_sel();
    MT_BOOL still_en = drv_reg_4k_disp_get_still_control_still_select();
    MT_BOOL sd_wr_back_forbidden = drv_reg_4k_disp_get_sd_wr_ctrl_sd_wr_back_forbidden();
    DISP_DEBUGK("%s hi_speed:%d\n", __FUNCTION__, b_clock_hi);

    free_irq(IRQ_ARIA_HD_GROUP0_ID, MT_NULL);
    free_irq(IRQ_ARIA_SD_TOP_START_ID, MT_NULL);
    mdelay(50);

    drv_reg_4k_disp_set_osdl_osd0_cmd_osd_layer_en(0);
    drv_reg_4k_disp_set_osdl_osd1_cmd_osd_layer_en(0);
    drv_reg_4k_disp_set_osdl_sub_cmd_osd_layer_en(0);
    drv_reg_4k_disp_set_video_ctrl_1_video_sel(0);
    drv_reg_4k_disp_set_still_control_still_select(0);
    drv_reg_4k_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(1);
    mdelay(100);

    disp_backup();
    if(b_clock_hi)
    {
        disp_crm_vdc_clksel(DISPVDC_CLKSEL_615M);
        disp_crm_osdc_clksel(DISPOSDC_CLKSEL_262M);
        disp_crm_di_clksel(DISPDI_CLKSEL_288M);
        disp_crm_core_clksel(DISPCORE_CLKSEL_615M);
    }
    else
    {
        disp_crm_vdc_clksel(DISPVDC_CLKSEL_480M);
        disp_crm_osdc_clksel(DISPOSDC_CLKSEL_144M);
        disp_crm_di_clksel(DISPDI_CLKSEL_160M);
        disp_crm_core_clksel(DISPCORE_CLKSEL_130M);
    }

    mdelay(50);
    vout_crm_sdvenc_clken(CLKEN_OFF);
    vout_crm_hdvenc_clken(CLKEN_OFF);
    disp_crm_core_reset();
    disp_crm_di_reset();
    disp_crm_vdc_reset();
    disp_crm_osdc_reset();

    mdelay(10);
    vout_crm_sdvenc_clken(CLKEN_ON);
    vout_crm_hdvenc_clken(CLKEN_ON);
    disp_crm_osdc_release();
    disp_crm_vdc_release();
    disp_crm_di_release();
    disp_crm_core_release();

    mdelay(50);
    disp_restore();
    drv_reg_4k_disp_set_osdl_osd0_cmd_osd_layer_en(osd0_en);
    drv_reg_4k_disp_set_osdl_osd1_cmd_osd_layer_en(osd1_en);
    drv_reg_4k_disp_set_osdl_sub_cmd_osd_layer_en(sub_en);
    drv_reg_4k_disp_set_video_ctrl_1_video_sel(video_en);
    drv_reg_4k_disp_set_still_control_still_select(still_en);
    drv_reg_4k_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(sd_wr_back_forbidden);

    ret = request_irq(IRQ_ARIA_HD_GROUP0_ID, aria_disp_isr_group0, IRQF_TRIGGER_HIGH, "aria_disp_group0", MT_NULL);
    if(ret)
    {
        printk("%s request_irq %d faild!!!\n", __FUNCTION__, IRQ_ARIA_HD_GROUP0_ID);
        nRet = MT_FAILURE;
    }
    ret = request_irq(IRQ_ARIA_SD_TOP_START_ID, aria_disp_isr_top_sd, IRQF_TRIGGER_HIGH, "aria_disp_top_sd", MT_NULL);
    if(ret)
    {
        printk("%s request_irq %d faild!!!\n", __FUNCTION__, IRQ_ARIA_SD_TOP_START_ID);
        nRet = MT_FAILURE;
    }

    return nRet;
}
mt_void DispCloseClkResetModule(mt_void)
{
    //s_stDisplayDevice.stIntfOpt.PF_CloseClkResetModule();
}

#ifdef CLOCK_DIV_1001
/* the configuration of clock div 1.001
 * 0xbf5d005c[31:8] must be 0xfdf4b9, do not change 0xbf5d005c[7:0]
 * 0xbf5d0058[7:4] the value shoule be 0x2
 * 0xbf5d000c bit8 must be 1, not care the bit16 and bit0
 * 0xbf5d00b0[31:16] must be 0xfdf4
 * 0xbf5d00b4[31:16] must be 0xfdf4
 */
void clock_div_1p001_enable(disp_priv_t *p_dp)
{
    mt_u32 val = 0;
    ulong reg = 0;
    DISP_DEBUGK("%s 1p001_enable fmt:%d \n", __FUNCTION__, p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt);

    reg = R_CLKGEN_VIDEO_SW_REG1;	//0xbf5d005c
	val = HAL_GET_U32((volatile u32 *)reg);
	if ((VID_SYS_3840X2160_60HZ == p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt) 
        || (VID_SYS_4096X2160_60HZ == p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt) )
	{
		val &= 0xf1;//clear bit31-8 and bit3-1
	    val |= 0xfdf4b906;//bit31-8 set to 0xfdf4b9, bit3-1 set to 011
	}
	else
	{
	    val &= 0xff;		//clear bit31-8
	    val |= 0xfdf4b900;	//bit31-8 set to 0xfdf4b9
	}
    HAL_PUT_U32((volatile u32 *)reg, val);	//0xbf5d005c

    reg = R_CLKGEN_VIDEO_SW_REG0;	//0xbf5d0058	    
    val = HAL_GET_U32((volatile u32 *)reg);
	val &= 0xffffff0f;	//clear bit7~bit4	
	val |= 0x20;
    HAL_PUT_U32((volatile u32 *)reg, val);	//0xbf5d0058	    
    
    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG0, 0xfdf48000);	//0xbf5d00b0
    HAL_PUT_U32((volatile u32 *)R_CLKGEN_VIDEO_SW_HD_REG1, 0xfdf48000);	//0xbf5d00b4		

	//must set R_INTP_CTRL_REG1 at last
	val = HAL_GET_U32((volatile u32 *)R_INTP_CTRL_REG1);	//0xbf5d000c
    val = val | (1<<16);  //needn't
    val = val | (1<<8);  //bit8 must be 1
    val = val & (~(1<<0));  //needn't
    HAL_PUT_U32((volatile u32 *)R_INTP_CTRL_REG1, val);	//0xbf5d000c    

    return;
}

void clock_div_1p001_disable(disp_priv_t *p_dp)
{
    mt_u32 val = 0;

    DISP_DEBUGK("%s 1p001_disable\n", __FUNCTION__);
    
    val = HAL_GET_U32((volatile u32 *)(R_CLKGEN_VIDEO_SW_REG1));
    val &= 0xff;
    val |= 0x0000b800;
    HAL_PUT_U32((volatile u32 *)(R_CLKGEN_VIDEO_SW_REG1), val);
    
    val = disp_hal_get_u32((volatile MT_U32 *)(R_INTP_CTRL_REG1));
    val = val & (~(1<<16));
    val = val & (~(1<<8));
    val = val | (1<<0);
    disp_hal_put_u32((volatile MT_U32 *)(R_CLKGEN_VIDEO_SW_HD_REG0), 0x0);
    disp_hal_put_u32((volatile MT_U32 *)(R_CLKGEN_VIDEO_SW_HD_REG1), 0x0);
    disp_hal_put_u32((volatile MT_U32 *)(R_INTP_CTRL_REG1), val);
}
#endif

static mt_void reg_backup(volatile mt_u32 *base, volatile mt_u32 *dst, mt_u32 len)
{
    mt_u32 i = 0;
    if(base == NULL || dst == NULL)
        return;

    for(i = 0; i < len; i++)
    {
        dst[i] = disp_hal_get_u32(base + i);
    }

    return;
}

static mt_void reg_restore(volatile mt_u32 *base, volatile mt_u32 *src, mt_u32 len)
{
    mt_u32 i = 0;
    if(base == NULL || src == NULL)
        return;

    for(i = 0; i < len; i++)
    {
        disp_hal_put_u32((base + i), src[i]);
    }

    return;
}

__attribute__((unused)) static mt_void hdvenc_backup(mt_void)
{
    reg_backup((mt_u32 *)REG_ARIA_HD_ENCODER_BASE, hdvenc_regs, sizeof(hdvenc_regs) / sizeof(hdvenc_regs[0]));
}

__attribute__((unused)) static mt_void sdvenc_backup(mt_void)
{
    reg_backup((mt_u32 *)REG_ARIA_SD_ENCODER_BASE, sdvenc_regs, sizeof(sdvenc_regs) / sizeof(sdvenc_regs[0]));
}

__attribute__((unused)) static mt_void hdvenc_restore(mt_void)
{
    reg_restore((mt_u32 *)REG_ARIA_HD_ENCODER_BASE, hdvenc_regs, sizeof(hdvenc_regs) / sizeof(hdvenc_regs[0]));
}

__attribute__((unused)) static mt_void sdvenc_restore(mt_void)
{
    reg_restore((mt_u32 *)REG_ARIA_SD_ENCODER_BASE, sdvenc_regs, sizeof(sdvenc_regs) / sizeof(sdvenc_regs[0]));
}

__attribute__((unused)) static mt_void hdvenc_reset(mt_void)
{
    volatile mt_u32 dtmp = 0;
    u8 reset_bit = 3;

    dtmp = disp_hal_get_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)));
    dtmp &= ~ (0x3 << (reset_bit));
    disp_hal_put_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)), dtmp);

    mdelay(3);
    dtmp = disp_hal_get_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)));
    dtmp |= (0x3 << (reset_bit));
    disp_hal_put_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)), dtmp);
}

__attribute__((unused)) static mt_void sdvenc_reset(mt_void)
{

    volatile mt_u32 dtmp = 0;


    dtmp = disp_hal_get_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)));
    dtmp &= ~ ((0x1 << 2) + (0x1 << 0));
    //printk("dtmp1 %p\n", dtmp);
    disp_hal_put_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)), dtmp);
    dtmp &= ~ (0x1 << 1);
    //printk("dtmp2 %p\n", dtmp);
    disp_hal_put_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)), dtmp);

    mdelay(3);
    dtmp = disp_hal_get_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)));
    dtmp |= (0x1 << 1);
    //printk("dtmp3 %p\n", dtmp);
    disp_hal_put_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)), dtmp);
    dtmp |= ((0x1 << 2) + (0x1 << 0));
    //printk("dtmp4 %p\n", dtmp);
    disp_hal_put_u32((volatile MT_U32 *)(mt_get_crm_base() + (0xa60c)), dtmp);
}

__attribute__((unused)) static mt_void disp_backup(mt_void)
{
    mt_u32 offset = 0;
    mt_u32 i = 0;

    for(i = 0; i < DISP_TOTAL_REGS_BLOCKS; i++)
    {
        reg_backup((mt_u32 *)(REG_SYMPHONY_DISP_DRV_BASE + disp_regs_map[i].start), &disp_regs[offset], disp_regs_map[i].size);
        offset += disp_regs_map[i].size;
    }
}

__attribute__((unused)) static mt_void disp_restore(mt_void)
{
    mt_u32 offset = 0;
    mt_u32 i = 0;

    for(i = 0; i < DISP_TOTAL_REGS_BLOCKS; i++)
    {
        reg_restore((mt_u32 *)(REG_SYMPHONY_DISP_DRV_BASE + disp_regs_map[i].start), &disp_regs[offset], disp_regs_map[i].size);
        offset += disp_regs_map[i].size;
    }
}


mt_s32 DispGetHdmiFunction(void)
{
    mt_s32 nRet = MT_SUCCESS;
#if 0
    s_pstHDMIFunc = MT_NULL;
    nRet = MT_DRV_MODULE_GetFunction(MT_ID_HDMI, (mt_void**)&s_pstHDMIFunc);

    if ((nRet != MT_SUCCESS) || (s_pstHDMIFunc == MT_NULL))
    {
        DISP_ERROR("DISP_get HDMI func failed!");
        return MT_FAILURE;
    }
#endif
    return nRet;
}

mt_s32 DispSearchCastHandle(mt_handle *cast_ptr, MT_DRV_DISPLAY_E enDisp)
{
    DISP_S *pstDisp;

    if (DISP_IsOpened(enDisp))
    {
        DispGetPointerByID(enDisp, pstDisp);

        if (pstDisp->Cast_ptr != 0)
        {
            *cast_ptr = pstDisp->Cast_ptr;
            return MT_SUCCESS;
        }
    }
    else
    {
        DISP_ERROR("Disp: %d not opened!\n", enDisp);
        return MT_ERR_DISP_NOT_OPEN;
    }

    return MT_ERR_DISP_NULL_PTR;
}

mt_s32 DispCheckMaster(MT_DRV_DISPLAY_E enDisp, MT_BOOL bDetach)
{

    return MT_SUCCESS;
}

mt_s32 DispCheckSlace(MT_DRV_DISPLAY_E enDisp, MT_BOOL bDetach)
{

    return MT_SUCCESS;
}

MT_BOOL DispFmtIsStandDefinition(MT_DRV_DISP_FMT_E enEncFmt)
{
    if ((enEncFmt >= MT_DRV_DISP_FMT_PAL) && (enEncFmt <= MT_DRV_DISP_FMT_1440x480i_60))
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}

MT_DRV_DISP_FMT_E DispTransferFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E enEncFmt)
{
#if 0

    return enEncFmt;
#else

    if (MT_DRV_DISPLAY_1 == enDisp)
    {
        if ((MT_DRV_DISP_FMT_PAL <= enEncFmt) && (MT_DRV_DISP_FMT_PAL_Nc >= enEncFmt))
        {
            return MT_DRV_DISP_FMT_PAL;
        }

        if ((MT_DRV_DISP_FMT_SECAM_SIN <= enEncFmt) && (MT_DRV_DISP_FMT_SECAM_H >= enEncFmt))
        {
            return MT_DRV_DISP_FMT_PAL;
        }

        if ((MT_DRV_DISP_FMT_PAL_M <= enEncFmt) && (MT_DRV_DISP_FMT_NTSC_443 >= enEncFmt))
        {
            return MT_DRV_DISP_FMT_NTSC;
        }

        return enEncFmt;
    }
    else
    {
        return enEncFmt;
    }
#endif
}

disp_sys_t transfer_vid_sys_fmt(MT_DRV_DISP_FMT_E enEncFmt)
{
    disp_sys_t vid_fmt = VID_SYS_PAL;
    DISP_DEBUGK("%s %d enEncFmt:%d 1080i:%d ntsc_j:%d\n", __FUNCTION__, __LINE__,
            enEncFmt,
            MT_DRV_DISP_FMT_1080i_60,
            MT_DRV_DISP_FMT_NTSC_J);
    switch (enEncFmt)
    {
    case MT_DRV_DISP_FMT_1080P_60:
    case MT_DRV_DISP_FMT_1080P_59_94:
        vid_fmt = VID_SYS_1080P; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_50:
        vid_fmt = VID_SYS_1080P_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_30:
    case MT_DRV_DISP_FMT_1080P_29_97:
        vid_fmt = VID_SYS_1080P_30HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_25:
        vid_fmt = VID_SYS_1080P_25HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_24:
        vid_fmt = VID_SYS_1080P_24HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080i_60:
    case MT_DRV_DISP_FMT_1080i_59_94:
        vid_fmt = VID_SYS_1080I; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080i_50:
        vid_fmt = VID_SYS_1080I_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_720P_60:
    case MT_DRV_DISP_FMT_720P_59_94:
        vid_fmt = VID_SYS_720P; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_720P_50:
        vid_fmt = VID_SYS_720P_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_576P_50:
        vid_fmt = VID_SYS_576P_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_480P_60:
        vid_fmt = VID_SYS_480P; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL:
        vid_fmt = VID_SYS_PAL; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL_N:
        vid_fmt = VID_SYS_PAL_N; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL_Nc:
        vid_fmt = VID_SYS_PAL_NC; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_60:
    case MT_DRV_DISP_FMT_3840X2160_59_94:
        vid_fmt = VID_SYS_3840X2160_60HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_50:
        vid_fmt = VID_SYS_3840X2160_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_30:
    case MT_DRV_DISP_FMT_3840X2160_29_97:
        vid_fmt = VID_SYS_3840X2160_30HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_25:
        vid_fmt = VID_SYS_3840X2160_25HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_24:
        vid_fmt = VID_SYS_3840X2160_24HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_24:
        vid_fmt = VID_SYS_4096X2160_24HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_25:
        vid_fmt = VID_SYS_4096X2160_25HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_30:
    case MT_DRV_DISP_FMT_4096X2160_29_97:
        vid_fmt = VID_SYS_4096X2160_30HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_50:
        vid_fmt = VID_SYS_4096X2160_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_60:
    case MT_DRV_DISP_FMT_4096X2160_59_94:
        vid_fmt = VID_SYS_4096X2160_60HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_NTSC:
        vid_fmt = VID_SYS_NTSC_M; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_NTSC_J:
        vid_fmt = VID_SYS_NTSC_J; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_NTSC_443:
        vid_fmt = VID_SYS_NTSC_443; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL_M:
        vid_fmt = VID_SYS_PAL_M; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_SECAM_SIN:
    case MT_DRV_DISP_FMT_SECAM_COS:
        vid_fmt = VID_SYS_SECAM; //enEncFmt2;
        break;
    default:
        vid_fmt = VID_SYS_1080I_50HZ; //enEncFmt2;
        break;
    }
    DISP_DEBUGK("%s %d enEncFmt:%d  vid_fmt:%d\n", __FUNCTION__, __LINE__,enEncFmt, vid_fmt);
    return vid_fmt;
}

mt_s32 DispSetMasterAndSlace(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    return MT_SUCCESS;
}

mt_s32 DispCheckReadyForOpen(MT_DRV_DISPLAY_E enDisp)
{
    return MT_SUCCESS;
}

/*==========================================================================
  interface control
  */
mt_void DispCleanIntf(DISP_INTF_S *pstIntf)
{
#if 0
    DISP_MEMSET(pstIntf, 0, sizeof(DISP_INTF_S));

    pstIntf->bOpen = MT_FALSE;
    pstIntf->bLinkVenc = MT_FALSE;
    pstIntf->eVencId = DISP_VENC_MAX;

    /*
       for(i=0; i<DISP_VENC_SIGNAL_MAX_NUMBER; i++)
       {
       pstIntf->eSignal[i] = MT_DRV_DISP_VDAC_NONE;
       }
       */
    pstIntf->stIf.eID = MT_DRV_DISP_INTF_ID_MAX;
    pstIntf->stIf.u8VDAC_Y_G  = MT_DISP_VDAC_INVALID_ID;
    pstIntf->stIf.u8VDAC_Pb_B = MT_DISP_VDAC_INVALID_ID;
    pstIntf->stIf.u8VDAC_Pr_R = MT_DISP_VDAC_INVALID_ID;
    pstIntf->stIf.bDacSync = MT_TRUE;
#endif
    return;
}

mt_void DispCleanAllIntf(DISP_S *pstDisp)
{
#if 0
    mt_s32 i;

    for (i = 0; i < (mt_s32)MT_DRV_DISP_INTF_ID_MAX; i++)
    {
        DispCleanIntf(&pstDisp->stSetting.stIntf[i]);
    }
#endif
    return;
}

/*if dac is busy ,release the  interface ,in order to used for new config is valid!*/
mt_s32 DispPrepareVDAC(mt_u32 u32DacId)
{
#if 0
    mt_u32 i, j;
    DISP_S* pstDisp;
    DISP_INTF_S* pstIt;

    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_ReleaseIntf2);

    for ( i = MT_DRV_DISPLAY_0; i < MT_DRV_DISPLAY_2 ; i++)
    {
        DispGetPointerByID((MT_DRV_DISPLAY_E) i, pstDisp);
        //printk("check  dac *********(%d)\n",u32DacId);

        for ( j = MT_DRV_DISP_INTF_YPBPR0; j <= MT_DRV_DISP_INTF_VGA0 ; j++)
        {
            pstIt = &pstDisp->stSetting.stIntf[j];
            //printk("*disp%d*[%d]***(%d)(%d)(%d)***(%d)\n",i,j,pstIt->stIf.u8VDAC_Y_G,pstIt->stIf.u8VDAC_Pb_B,pstIt->stIf.u8VDAC_Pr_R,pstDisp->stSetting.stIntf[i].bOpen);

            if ( (pstIt->stIf.u8VDAC_Y_G == u32DacId)
                    || (pstIt->stIf.u8VDAC_Pb_B == u32DacId)
                    || (pstIt->stIf.u8VDAC_Pr_R == u32DacId)
               )
            {
                // s1 release vdac
                //printk("del **disp%d*[%d]***(%d)(%d)(%d)***(%d)\n", i, j, pstIt->stIf.u8VDAC_Y_G, pstIt->stIf.u8VDAC_Pb_B, pstIt->stIf.u8VDAC_Pr_R, pstDisp->stSetting.stIntf[j].bOpen);
                pfOpt->PF_ReleaseIntf2(i, pstIt);
                /* clean */

                DispCleanIntf(pstIt);
                continue;
            }
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DispPrepareHDMI(MT_DRV_DISP_INTF_ID_E enHDMIId)
{
#if 0
    mt_u32 i;
    DISP_S* pstDisp;
    DISP_INTF_S* pstIt;

    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_ReleaseIntf2);

    for ( i = MT_DRV_DISPLAY_0; i < MT_DRV_DISPLAY_2 ; i++)
    {
        DispGetPointerByID((MT_DRV_DISPLAY_E) i, pstDisp);
        pstIt = &pstDisp->stSetting.stIntf[enHDMIId];

        if (!DispGetHdmiFunction()) {
            if (s_pstHDMIFunc->pfnHdmiDetach && s_pstHDMIFunc->pfnHdmiAttach)
            {
                s_pstHDMIFunc->pfnHdmiDetach(MT_UNF_HDMI_ID_0);
            }
        }

        pfOpt->PF_ReleaseIntf2(i, pstIt);
        DispCleanIntf(pstIt);
        continue;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DispPrepareLCD_BT1120(MT_DRV_DISP_INTF_ID_E enIntfId)
{
#if 0
    mt_u32 i, j;
    DISP_S* pstDisp;
    DISP_INTF_S* pstIt;

    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_ReleaseIntf2);

    /*Because SOC  GPIO limit ,at same time LCD or BT1120 only support by DHD1 ,*/
    for ( i = MT_DRV_DISPLAY_1; i < MT_DRV_DISPLAY_2 ; i++)
    {
        DispGetPointerByID((MT_DRV_DISPLAY_E) i, pstDisp);
        for ( j = MT_DRV_DISP_INTF_BT1120_0; j <= MT_DRV_DISP_INTF_LCD2 ; j++)
        {
            pstIt = &pstDisp->stSetting.stIntf[j];
            printk("prepare  del **disp%d*[%d]**status**(%d)\n", i, j,  pstDisp->stSetting.stIntf[j].bOpen);
            pfOpt->PF_ReleaseIntf2(i, pstIt);
            DispCleanIntf(pstIt);
            continue;
        }
    }
#endif
    return MT_SUCCESS;
}

#define DEF_DRV_DISP_INTERFACE_CONTROL_START_HERE

mt_s32 DispCheckIntfValid(MT_DRV_DISP_INTF_S *pstIntf)
{
#if 0
    if (pstIntf->eID >= MT_DRV_DISP_INTF_ID_MAX)
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (  (pstIntf->eID == MT_DRV_DISP_INTF_YPBPR0)
            || (pstIntf->eID == MT_DRV_DISP_INTF_VGA0)
            || (pstIntf->eID == MT_DRV_DISP_INTF_RGB0)
       )
    {
        if (  (pstIntf->u8VDAC_Y_G  >= MT_DISP_VDAC_MAX_NUMBER)
                || (pstIntf->u8VDAC_Pb_B >= MT_DISP_VDAC_MAX_NUMBER)
                || (pstIntf->u8VDAC_Pr_R >= MT_DISP_VDAC_MAX_NUMBER)
           )
        {
            return MT_ERR_DISP_INVALID_PARA;
        }
    }

    if (pstIntf->eID == MT_DRV_DISP_INTF_SVIDEO0)
    {
        if (  (pstIntf->u8VDAC_Y_G  >= MT_DISP_VDAC_MAX_NUMBER)
                || (pstIntf->u8VDAC_Pb_B >= MT_DISP_VDAC_MAX_NUMBER)
                || (pstIntf->u8VDAC_Pr_R != MT_DISP_VDAC_INVALID_ID)
           )
        {
            return MT_ERR_DISP_INVALID_PARA;
        }
    }

    if (pstIntf->eID == MT_DRV_DISP_INTF_CVBS0)
    {
        if (  (pstIntf->u8VDAC_Y_G  >= MT_DISP_VDAC_MAX_NUMBER)
                || (pstIntf->u8VDAC_Pb_B != MT_DISP_VDAC_INVALID_ID)
                || (pstIntf->u8VDAC_Pr_R != MT_DISP_VDAC_INVALID_ID)
           )
        {
            return MT_ERR_DISP_INVALID_PARA;
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DispPrepareInft(MT_DRV_DISP_INTF_S *pstIntf)
{
#if 0
    switch (pstIntf->eID)
    {
    case MT_DRV_DISP_INTF_YPBPR0:
    case MT_DRV_DISP_INTF_VGA0:
    case MT_DRV_DISP_INTF_RGB0:
        DispPrepareVDAC(pstIntf->u8VDAC_Y_G);
        DispPrepareVDAC(pstIntf->u8VDAC_Pb_B);
        DispPrepareVDAC(pstIntf->u8VDAC_Pr_R);
        break;

    case MT_DRV_DISP_INTF_SVIDEO0:
        DispPrepareVDAC(pstIntf->u8VDAC_Y_G);
        DispPrepareVDAC(pstIntf->u8VDAC_Pb_B);
        break;

    case MT_DRV_DISP_INTF_CVBS0:
        DispPrepareVDAC(pstIntf->u8VDAC_Y_G);
        break;

    case MT_DRV_DISP_INTF_HDMI0:
    case MT_DRV_DISP_INTF_HDMI1:
    case MT_DRV_DISP_INTF_HDMI2:
        DispPrepareHDMI(pstIntf->eID);
        break;

    case MT_DRV_DISP_INTF_BT1120_0:
    case MT_DRV_DISP_INTF_BT1120_1:
    case MT_DRV_DISP_INTF_BT1120_2:
    case MT_DRV_DISP_INTF_LCD0:
    case MT_DRV_DISP_INTF_LCD1:
    case MT_DRV_DISP_INTF_LCD2:
        DispPrepareLCD_BT1120(pstIntf->eID);
        break;

    default:
        break;
    }
#endif
    return MT_SUCCESS;
}

MT_BOOL DispCheckIntfExist(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
#if 0
    DISP_S* pstDisp;
    MT_DRV_DISP_INTF_S* pstIntf2;

    DispGetPointerByIDNoReturn(enDisp, pstDisp);

    pstIntf2 = &pstDisp->stSetting.stIntf[pstIntf->eID].stIf;

    if (pstIntf->eID >= MT_DRV_DISP_INTF_ID_MAX)
        return MT_FALSE;

    if ((pstIntf->eID >= MT_DRV_DISP_INTF_BT1120_0) && (pstIntf->eID <= MT_DRV_DISP_INTF_LCD2))
    {
        if (  (MT_TRUE == pstDisp->stSetting.stIntf[pstIntf->eID].bOpen) && (pstIntf->eID == pstIntf2->eID))
            return MT_TRUE;
        else
            return MT_FALSE;
    }

    if (  (MT_TRUE == pstDisp->stSetting.stIntf[pstIntf->eID].bOpen)
            && (pstIntf->eID         == pstIntf2->eID)
            && (pstIntf->u8VDAC_Y_G  == pstIntf2->u8VDAC_Y_G)
            && (pstIntf->u8VDAC_Pb_B == pstIntf2->u8VDAC_Pb_B)
            && (pstIntf->u8VDAC_Pr_R == pstIntf2->u8VDAC_Pr_R)
       )
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
#endif
    return MT_FALSE;
}

MT_BOOL DispCheckIntfExistByType(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
#if 0
    DISP_S* pstDisp;

    DispGetPointerByIDNoReturn(enDisp, pstDisp);

    if (MT_TRUE == pstDisp->stSetting.stIntf[pstIntf->eID].bOpen)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
#endif
    return MT_FALSE;
}

DISP_INTF_S *DispGetIntfPtr(DISP_S *pstDisp, MT_DRV_DISP_INTF_ID_E eID)
{
    return &pstDisp->stSetting.stIntf[eID];
}

mt_s32 DispAddIntf(DISP_S *pstDisp, MT_DRV_DISP_INTF_S *pstIntf)
{
    mt_s32 nRet = MT_SUCCESS;
#if 0
    DISP_INTF_S* pstIt = &pstDisp->stSetting.stIntf[pstIntf->eID];
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    MT_DRV_DISP_INTF_S stBackup;
    MT_BOOL bBkFlag = MT_FALSE;
    DispCheckNullPointer(pfOpt);

    //printk("DispAddIntf ***0**add (%d)----%d (%d)(%d)(%d)\n",pstDisp->enDisp,pstIntf->eID,pstIntf->u8VDAC_Y_G,pstIntf->u8VDAC_Pb_B,pstIntf->u8VDAC_Pr_R);
    /* if intf exist, release firstly */
    if (pstIt->bOpen)
    {
        bBkFlag = MT_TRUE;

        stBackup = pstIt->stIf;

        // s1 release vdac
        DispCheckNullPointer(pfOpt->PF_ReleaseIntf2);
        pfOpt->PF_ReleaseIntf2(pstDisp->enDisp, pstIt);
    }

    /* clean */
    DispCleanIntf(pstIt);

    DISP_PRINT("DispAddIntf  pstIntf->u8VDAC_Y_G = %d\n", pstIntf->u8VDAC_Y_G);

    pstIt->stIf = *pstIntf;
    DispCheckNullPointer(pfOpt->PF_AcquireIntf2);
    nRet = pfOpt->PF_AcquireIntf2(pstDisp->enDisp, pstIt);

    if (nRet)
    {
        DISP_ERROR("DISP %d acquire  (%d) failed\n", pstDisp->enDisp, pstIt->stIf.eID);
        goto __SET_BACKUP__;
    }
#if 0
    if ((pstIt->stIf.eID >= MT_DRV_DISP_INTF_HDMI0 ) && (pstIt->stIf.eID <= MT_DRV_DISP_INTF_HDMI2 ) && (!DispGetHdmiFunction()))
    {
        if (s_pstHDMIFunc->pfnHdmiDetach && s_pstHDMIFunc->pfnHdmiAttach)
        {
            s_pstHDMIFunc->pfnHdmiAttach(MT_UNF_HDMI_ID_0, pstDisp->stSetting.enFormat, pstDisp->stSetting.eDispMode);
        }
    }
#endif
    pstIt->bOpen = MT_TRUE;

    return MT_SUCCESS;

__SET_BACKUP__:

    if (bBkFlag == MT_TRUE)
    {
        DispAddIntf(pstDisp, &stBackup);
    }
#endif
    return nRet;
}

#define DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH 1920
#define DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT 1080

/*==========================================================================
  video encoding
  */
mt_s32 DispProduceDisplayInfo(DISP_S *pstDisp, MT_DISP_DISPLAY_INFO_S *pstInfo)
{

    MT_DRV_DISP_FMT_E eFmt;
    MT_PQ_PICTURE_SETTING_S stPictureSetting;
    //disp_priv_t *p_dp = get_disp_priv_handle();
    rect_vsb_t hd_rect_cur = { 0 };
    if (!pstDisp)
    {
        DISP_ERROR("Found null pointer in %s\n", __FUNCTION__);
        return MT_ERR_DISP_NULL_PTR;
    }
    eFmt = pstDisp->stSetting.enFormat;

    //get hd output size
    disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &(hd_rect_cur.h), &(hd_rect_cur.w));
    pstInfo->bIsMaster = pstDisp->bIsMaster;
    //pstInfo->stVirtaulScreen = pstDisp->stSetting.stVirtaulScreen;
    pstInfo->stVirtaulScreen.s32X = 0;
    pstInfo->stVirtaulScreen.s32Y = 0;
    pstInfo->stVirtaulScreen.s32Width = hd_rect_cur.w;
    pstInfo->stVirtaulScreen.s32Height = hd_rect_cur.h;
    if ((pstInfo->stVirtaulScreen.s32Width == 0) || (pstInfo->stVirtaulScreen.s32Height == 0))
    {
        pstInfo->stVirtaulScreen.s32Width = DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH;
        pstInfo->stVirtaulScreen.s32Height = DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT;
    }
    //pstInfo->u32RefreshRate = 5000;
    pstInfo->stPixelFmtResolution.s32X = 0;
    pstInfo->stPixelFmtResolution.s32Y = 0;
    pstInfo->stPixelFmtResolution.s32Width = hd_rect_cur.w;
    pstInfo->stPixelFmtResolution.s32Height = hd_rect_cur.h;
    if ((pstInfo->stPixelFmtResolution.s32Width == 0) || (pstInfo->stPixelFmtResolution.s32Height == 0))
    {
        pstInfo->stPixelFmtResolution.s32Width = DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH;
        pstInfo->stPixelFmtResolution.s32Height = DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT;
    }

    pstInfo->stFmtResolution.s32X = 0;
    pstInfo->stFmtResolution.s32Y = 0;

    pstInfo->stFmtResolution.s32Width = hd_rect_cur.w;
    pstInfo->stFmtResolution.s32Height = hd_rect_cur.h;
    if ((pstInfo->stFmtResolution.s32Width == 0) || (pstInfo->stFmtResolution.s32Height == 0))
    {
        pstInfo->stFmtResolution.s32Width = DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH;
        pstInfo->stFmtResolution.s32Height = DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT;
    }
    pstInfo->stOffsetInfo = pstDisp->stSetting.stOffsetInfo;

    if (!pstDisp->stSetting.bCustomRatio)
    {
        pstInfo->stAR.u32ARh = 9;
        pstInfo->stAR.u32ARw = 16;
        //pstInfo->stAR = stFmt.stAR;
    }
    else
    {
        pstInfo->stAR.u32ARh = pstDisp->stSetting.u32CustomRatioHeight;
        pstInfo->stAR.u32ARw = pstDisp->stSetting.u32CustomRatioWidth;
    }

    if (pstDisp->enDisp == MT_DRV_DISPLAY_1)
    {
        DRV_PQ_GetHDPictureSetting(&stPictureSetting);
    }
    else
    {
        DRV_PQ_GetSDPictureSetting(&stPictureSetting);
    }

    pstInfo->u32Bright = stPictureSetting.u16Brightness;
    pstInfo->u32Contrst = stPictureSetting.u16Contrast;
    pstInfo->u32Hue = stPictureSetting.u16Hue;
    pstInfo->u32Satur = stPictureSetting.u16Saturation;

#if 0
    DISP_HAL_ENCFMT_PARAM_S stFmt;
    MT_DRV_DISP_FMT_E eFmt;
    mt_s32 nRet;
    mt_s32 u32TcFreq;
#if 0
    MT_PQ_PICTURE_SETTING_S stPictureSetting;
#endif

    if (!pstDisp)
    {
        DISP_ERROR("Found null pointer in %s\n", __FUNCTION__);
        return MT_ERR_DISP_NULL_PTR;
    }

    eFmt = pstDisp->stSetting.enFormat;
    if (eFmt < MT_DRV_DISP_FMT_CUSTOM)
    {
        nRet = DISP_HAL_GetEncFmtPara(eFmt, &stFmt);

        pstInfo->bIsMaster = pstDisp->bIsMaster;
        pstInfo->bIsSlave  = pstDisp->bIsSlave;
        //printk("id=%d, bm=%d, bs=%d\n", pstDisp->enDisp, pstInfo->bIsMaster,  pstInfo->bIsSlave);
        pstInfo->enAttachedDisp = pstDisp->enAttachedDisp;


        pstInfo->eDispMode = pstDisp->stSetting.eDispMode;
        pstInfo->bRightEyeFirst = pstDisp->stSetting.bRightEyeFirst;
        pstInfo->bInterlace = stFmt.bInterlace;

        pstInfo->stVirtaulScreen = pstDisp->stSetting.stVirtaulScreen;


        if ((pstInfo->stVirtaulScreen.s32Width == 0)
                || (pstInfo->stVirtaulScreen.s32Height == 0))
        {
            pstInfo->stVirtaulScreen.s32Width = DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH;
            pstInfo->stVirtaulScreen.s32Height = DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT;
        }

        pstInfo->stOffsetInfo    = pstDisp->stSetting.stOffsetInfo;
        pstInfo->stFmtResolution = stFmt.stRefRect;
        pstInfo->stPixelFmtResolution = stFmt.stOrgRect;


        pstDisp->stSetting.stColor.enInCS = stFmt.enColorSpace;
        pstInfo->eColorSpace = stFmt.enColorSpace;
        //printk(">>>>>>>>>>>>>>>> 002 cs =%d\n",  pstInfo->eColorSpace);

        if (!pstDisp->stSetting.bCustomRatio)
        {
            pstInfo->stAR = stFmt.stAR;
        }
        else
        {
            pstInfo->stAR.u32ARh = pstDisp->stSetting.u32CustomRatioHeight;
            pstInfo->stAR.u32ARw = pstDisp->stSetting.u32CustomRatioWidth;
        }

        pstInfo->u32RefreshRate = stFmt.u32RefreshRate;

#if 0

        if (pstDisp->enDisp == MT_DRV_DISPLAY_1)
        {
            DRV_PQ_GetHDPictureSetting(&stPictureSetting);
        }
        else
        {
            DRV_PQ_GetSDPictureSetting(&stPictureSetting);
        }

        pstInfo->u32Bright = stPictureSetting.u16Brightness;
        pstInfo->u32Contrst = stPictureSetting.u16Contrast;
        pstInfo->u32Hue = stPictureSetting.u16Hue;
        pstInfo->u32Satur = stPictureSetting.u16Saturation;

#endif

    }
    else if (eFmt == MT_DRV_DISP_FMT_CUSTOM)
    {
        pstInfo->bIsMaster = pstDisp->bIsMaster;
        pstInfo->bIsSlave  = pstDisp->bIsSlave;
        pstInfo->enAttachedDisp = pstDisp->enAttachedDisp;


        pstInfo->eDispMode = pstDisp->stSetting.eDispMode;
        pstInfo->bRightEyeFirst = pstDisp->stSetting.bRightEyeFirst;

        pstInfo->bInterlace = pstDisp->stSetting.stCustomTimg.bInterlace;


        pstInfo->stVirtaulScreen = pstDisp->stSetting.stVirtaulScreen;

        if ((pstInfo->stVirtaulScreen.s32Width == 0)
                || (pstInfo->stVirtaulScreen.s32Height == 0))
        {
            pstInfo->stVirtaulScreen.s32Width = DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH;
            pstInfo->stVirtaulScreen.s32Height = DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT;
        }

        pstInfo->stOffsetInfo    = pstDisp->stSetting.stOffsetInfo;

        pstInfo->stPixelFmtResolution.s32X = 0;
        pstInfo->stPixelFmtResolution.s32Y = 0;
        pstInfo->stPixelFmtResolution.s32Width = pstDisp->stSetting.stCustomTimg.u32HACT;
        pstInfo->stPixelFmtResolution.s32Height = pstDisp->stSetting.stCustomTimg.u32VACT;

        pstInfo->stFmtResolution.s32X = 0;
        pstInfo->stFmtResolution.s32Y = 0;

        pstInfo->stFmtResolution.s32Width = pstDisp->stSetting.stCustomTimg.u32HACT;
        pstInfo->stFmtResolution.s32Height = pstDisp->stSetting.stCustomTimg.u32VACT;

        /**/
        /*set  DHDx  in Color Space*/
        pstDisp->stSetting.stColor.enInCS = MT_DRV_CS_BT709_RGB_FULL;
        pstInfo->eColorSpace = MT_DRV_CS_BT709_RGB_FULL;

        pstInfo->stAR.u32ARh = pstDisp->stSetting.stCustomTimg.u32AspectRatioH;
        pstInfo->stAR.u32ARw = pstDisp->stSetting.stCustomTimg.u32AspectRatioW;

        /*set  Rate*/
        /*tc set freq is *1000  but VDP module need *100*/
        u32TcFreq = pstDisp->stSetting.stCustomTimg.u32VertFreq;
        pstInfo->u32RefreshRate = u32TcFreq/10;

#if 0
        if (pstDisp->enDisp == MT_DRV_DISPLAY_1)
        {
            DRV_PQ_GetHDPictureSetting(&stPictureSetting);
        }
        else
        {
            DRV_PQ_GetSDPictureSetting(&stPictureSetting);
        }


        pstInfo->u32Bright = stPictureSetting.u16Brightness;
        pstInfo->u32Contrst = stPictureSetting.u16Contrast;
        pstInfo->u32Hue = stPictureSetting.u16Hue;
        pstInfo->u32Satur = stPictureSetting.u16Saturation;

#endif
    }
    else
    {
        DISP_WARN("Invalid display encoding format now\n");
        return MT_ERR_DISP_NOT_SUPPORT_FMT;
    }
#endif
    return MT_SUCCESS;
}

mt_void DispInitCSC(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_SETTING_S *pstColor)
{
#if 0
    // s1 CSC

    DISP_MEMSET(pstColor, 0, sizeof(MT_DRV_DISP_COLOR_SETTING_S));

    pstColor->enInCS  = MT_DRV_CS_DEFAULT;
    pstColor->enOutCS = MT_DRV_CS_DEFAULT;

    pstColor->pReserve = MT_NULL;
    pstColor->u32Reserve = 0;
#endif
    return;
}

mt_void DispGetTestInitParam(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_SETTING_S *pstSetting)
{
    if (enDisp == MT_DRV_DISPLAY_0)
    {
        pstSetting->bIsMaster = MT_TRUE;
        pstSetting->enAttachedDisp = MT_DRV_DISPLAY_1;

        pstSetting->enFormat = MT_DRV_DISP_FMT_1080i_50;
        pstSetting->stIntf[MT_DRV_DISP_INTF_YPBPR0].eID = MT_DRV_DISP_INTF_YPBPR0;
        pstSetting->stIntf[MT_DRV_DISP_INTF_YPBPR0].u8VDAC_Y_G = 1;
        pstSetting->stIntf[MT_DRV_DISP_INTF_YPBPR0].u8VDAC_Pb_B = 2;
        pstSetting->stIntf[MT_DRV_DISP_INTF_YPBPR0].u8VDAC_Pr_R = 0;
    }

    if (enDisp == MT_DRV_DISPLAY_1)
    {
        pstSetting->bIsSlave = MT_TRUE;
        pstSetting->enAttachedDisp = MT_DRV_DISPLAY_0;

        pstSetting->enFormat = MT_DRV_DISP_FMT_PAL;
        pstSetting->stIntf[MT_DRV_DISP_INTF_CVBS0].eID = MT_DRV_DISP_INTF_CVBS0;
        pstSetting->stIntf[MT_DRV_DISP_INTF_CVBS0].u8VDAC_Y_G = 3;
        pstSetting->stIntf[MT_DRV_DISP_INTF_CVBS0].u8VDAC_Pb_B = MT_DISP_VDAC_INVALID_ID;
        pstSetting->stIntf[MT_DRV_DISP_INTF_CVBS0].u8VDAC_Pr_R = MT_DISP_VDAC_INVALID_ID;
    }

    return;
}

extern mt_s32 DispGetInitParam(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INIT_PARAM_S *pstSetting);

mt_s32 DispGetInitParamPriv(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_SETTING_S *pstSetting)
{
    MT_DRV_DISP_INTF_ID_E enIf;
    MT_DRV_DISP_INIT_PARAM_S stInitParam;

    DISP_MEMSET(pstSetting, 0, sizeof(MT_DRV_DISP_SETTING_S));

    if (enDisp > MT_DRV_DISPLAY_1)
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    // s1 get init param form PDM

    // s2 check para, if(ok){return;} else {get default param;}
    pstSetting->u32BootVersion = 0xfffffffful;

    pstSetting->bGetPDMParam = MT_FALSE;

    pstSetting->bIsMaster = MT_FALSE;
    pstSetting->bIsSlave = MT_FALSE;
    pstSetting->enAttachedDisp = MT_DRV_DISPLAY_BUTT;

    /* output format */
    pstSetting->eDispMode = DISP_STEREO_NONE;

    pstSetting->enFormat = MT_DRV_DISP_FMT_BUTT;
    pstSetting->bIsMaster = MT_FALSE;
    pstSetting->bIsSlave = MT_FALSE;
    pstSetting->enAttachedDisp = MT_DRV_DISPLAY_BUTT;

    /* about color */
    DispInitCSC(enDisp, &pstSetting->stColor);

    /* background color */
    pstSetting->stBgColor.u8Red = DISP_DEFAULT_COLOR_RED;
    pstSetting->stBgColor.u8Green = DISP_DEFAULT_COLOR_GREEN;
    pstSetting->stBgColor.u8Blue = DISP_DEFAULT_COLOR_BLUE;

    /*zorder */
    pstSetting->enLayer[0] = MT_DRV_DISP_LAYER_GFX;
    pstSetting->enLayer[1] = MT_DRV_DISP_LAYER_VIDEO;

    /* interface setting */
    for (enIf = MT_DRV_DISP_INTF_YPBPR0; enIf < MT_DRV_DISP_INTF_ID_MAX; enIf++)
    {
        pstSetting->stIntf[enIf].eID = MT_DRV_DISP_INTF_ID_MAX;
        pstSetting->stIntf[enIf].u8VDAC_Y_G = MT_DISP_VDAC_INVALID_ID;
        pstSetting->stIntf[enIf].u8VDAC_Pb_B = MT_DISP_VDAC_INVALID_ID;
        pstSetting->stIntf[enIf].u8VDAC_Pr_R = MT_DISP_VDAC_INVALID_ID;
    }

    pstSetting->u32LayerNumber = 0;
    //MT_DRV_DISP_LAYER_E enLayer[MT_DRV_DISP_LAYER_BUTT]; /* Z-order is from bottom to top */

    pstSetting->bCustomRatio = MT_FALSE;
    pstSetting->u32CustomRatioWidth = 0;
    pstSetting->u32CustomRatioHeight = 0;

    pstSetting->stVirtaulScreen.s32X = 0;
    pstSetting->stVirtaulScreen.s32Y = 0;
    pstSetting->stVirtaulScreen.s32Width = DISPLAY_DEFAULT_VIRT_SCREEN_WIDTH;
    pstSetting->stVirtaulScreen.s32Height = DISPLAY_DEFAULT_VIRT_SCREEN_HEIGHT;

    pstSetting->stOffsetInfo.u32Left = 0;
    pstSetting->stOffsetInfo.u32Right = 0;
    pstSetting->stOffsetInfo.u32Top = 0;
    pstSetting->stOffsetInfo.u32Bottom = 0;

    //pstSetting->u32Reseve;
    //pstSetting->pRevData;
    //DispGetTestInitParam(enDisp, pstSetting);
    //DispGetInitParam(enDisp, pstSetting);
    if (DispGetInitParam(enDisp, &stInitParam) == MT_SUCCESS)
    {
        pstSetting->bGetPDMParam = MT_TRUE;
        pstSetting->u32BootVersion = stInitParam.u32Version;

        pstSetting->bIsMaster = stInitParam.bIsMaster;
        pstSetting->bIsSlave = stInitParam.bIsSlave;
        pstSetting->enAttachedDisp = stInitParam.enAttachedDisp;

        pstSetting->enFormat = DispTransferFormat(enDisp, stInitParam.enFormat);
        pstSetting->stCustomTimg = stInitParam.stDispTiming;

        pstSetting->stBgColor = stInitParam.stBgColor;

        pstSetting->bCustomRatio = stInitParam.bCustomRatio;
        pstSetting->u32CustomRatioWidth = stInitParam.u32CustomRatioWidth;
        pstSetting->u32CustomRatioHeight = stInitParam.u32CustomRatioHeight;

        pstSetting->stVirtaulScreen.s32X = 0;
        pstSetting->stVirtaulScreen.s32Y = 0;
        pstSetting->stVirtaulScreen.s32Width = stInitParam.u32VirtScreenWidth;
        pstSetting->stVirtaulScreen.s32Height = stInitParam.u32VirtScreenHeight;

        pstSetting->stOffsetInfo = stInitParam.stOffsetInfo;

        for (enIf = MT_DRV_DISP_INTF_YPBPR0; enIf < MT_DRV_DISP_INTF_ID_MAX; enIf++)
        {
            if (stInitParam.stIntf[enIf].eID != MT_DRV_DISP_INTF_ID_MAX)
            {
                pstSetting->stIntf[enIf] = stInitParam.stIntf[enIf];
                DISP_PRINT(">>>>>>>>>> intf %d id=%d\n", enIf,
                        stInitParam.stIntf[enIf].eID);
            }
        }

        //stInitParam.stDispTiming;
    }

    return MT_SUCCESS;
}

mt_void DispParserInitParam(DISP_S *pstDisp, MT_DRV_DISP_SETTING_S *pstSetting)
{
    DISP_SETTING_S *pstS = &pstDisp->stSetting;
    pstDisp->bIsMaster = pstSetting->bIsMaster;
    pstS->stVirtaulScreen = pstSetting->stVirtaulScreen;
    pstS->enFormat = pstSetting->enFormat;
#if 0
    DISP_INTF_OPERATION_S* pstIntfOpt;
    DISP_SETTING_S* pstS = &pstDisp->stSetting;
    mt_s32 t = 0;

    pstIntfOpt = DISP_HAL_GetOperationPtr();
    //DISP_ASSERT(pstIntfOpt);
    pstS->u32Version = DISP_DRVIER_VERSION;
    pstS->u32BootVersion = pstSetting->u32BootVersion;
    //pstS->bSelfStart = pstSetting->bSelfStart;
    pstS->bGetPDMParam = pstSetting->bGetPDMParam;

    pstS->eDispMode = pstSetting->eDispMode;
    pstS->bRightEyeFirst = MT_FALSE;
    pstS->enFormat  = pstSetting->enFormat;
    //pstS->bFmtChanged = MT_FALSE;  // TODO

    pstS->stCustomTimg = pstSetting->stCustomTimg;

    /* about color */
    pstS->stColor = pstSetting->stColor;

    /* background color */
    pstS->stBgColor = pstSetting->stBgColor;

    /* interface setting */
    DispCleanAllIntf(pstDisp);

    pstS->u32LayerNumber = 0;
    //MT_DRV_DISP_LAYER_E enLayer[MT_DRV_DISP_LAYER_BUTT]; /* Z-order is from bottom to top */

    pstS->bCustomRatio = pstSetting->bCustomRatio;

    if (pstS->bCustomRatio)
    {
        pstS->u32CustomRatioWidth = pstSetting->u32CustomRatioWidth;
        pstS->u32CustomRatioHeight = pstSetting->u32CustomRatioHeight;
    }

    pstS->u32Reseve = 0;
    pstS->pRevData  = MT_NULL;
    pstS->stVirtaulScreen = pstSetting->stVirtaulScreen;
    pstS->stOffsetInfo    = pstSetting->stOffsetInfo;

    for (t = 0; t < MT_DRV_DISP_INTF_ID_MAX; t++)
    {
        if (pstSetting->stIntf[t].eID < MT_DRV_DISP_INTF_ID_MAX)
        {
            //todo
            DispAddIntf(pstDisp, &pstSetting->stIntf[t]);
            if (( !pstIntfOpt) || (!pstIntfOpt->PF_GetChnEnable) )
            {
                DISP_ERROR(" %s has null ptr!\n", __FUNCTION__);
                return ;
            }
            pstIntfOpt->PF_InitDacDetect(&pstSetting->stIntf[t]);
        }
    }

    /* for attach display */
    pstDisp->bIsMaster = pstSetting->bIsMaster;
    pstDisp->bIsSlave  = pstSetting->bIsSlave;
    pstDisp->enAttachedDisp = pstSetting->enAttachedDisp;

#if 0
    DISP_PRINT("FOLLOW INFO: DISP %d, M=%d,S=%d,ATT=%d......\n",
            pstDisp->enDisp,
            pstDisp->bIsMaster,
            pstDisp->bIsSlave,
            pstDisp->enAttachedDisp);
#endif

#ifdef MT_DISP_BUILD_FULL

    if (pstS->bGetPDMParam)
    {
        MT_BOOL bOutput;

        // todo
        if (( !pstIntfOpt) || (!pstIntfOpt->PF_GetChnEnable) )
        {
            DISP_ERROR(" %s has null ptr!\n", __FUNCTION__);
            return ;
        }
        pstIntfOpt->PF_GetChnEnable(pstDisp->enDisp, &bOutput);

        if (bOutput)
        {
            pstDisp->bEnable = MT_TRUE;
            DISP_PRINT("DISP %d is working......\n", pstDisp->enDisp);

            // VDP is working, not to reset.
            DispSetHardwareState();
        }
    }

#endif
#endif
    pstDisp->bEnable = MT_TRUE;
    return;
}

extern mt_u32 s_u32VdpBaseAddr;

mt_void DispInitDisplay(MT_DRV_DISPLAY_E enDisp)
{
    DISP_S *pstDisp;
    MT_DRV_DISP_SETTING_S stDefSetting;

    DispGetPointerByIDNoReturn(enDisp, pstDisp);

    DISP_MEMSET(pstDisp, 0, sizeof(DISP_S));

    // s1 set id
    pstDisp->enDisp = enDisp;

    // s2 get base parameters
    if (DispGetInitParamPriv(enDisp, &stDefSetting))
    {
        pstDisp->bBaseExist = MT_FALSE;
        DISP_PRINT("DispGetInitParam  failed\n");
        return;
    }

    pstDisp->bBaseExist = MT_TRUE;

    //component operation
    pstDisp->pstIntfOpt = &s_stDisplayDevice.stIntfOpt;

    /*we delete the pq,hue,contrast not initialied.*/
    DispParserInitParam(pstDisp, &stDefSetting);

    pstDisp->eState = DISP_PRIV_STATE_DISABLE;
    pstDisp->bOpen = MT_FALSE;
    ;

    DispProduceDisplayInfo(pstDisp, &pstDisp->stDispInfo);
    pstDisp->hCast = MT_NULL;

    return;
}

mt_void DispDeInitDisplay(MT_DRV_DISPLAY_E enDisp)
{
#if 0
    DISP_S* pstDisp;
    mt_s32 t;

    DispGetPointerByIDNoReturn(enDisp, pstDisp);

#ifdef MT_DISP_BUILD_FULL

    if (pstDisp->hCast)
    {
        DISP_CastSetEnable(pstDisp->hCast, MT_FALSE);

        DISP_CastDestroy(pstDisp->hCast);
    }

#endif

    for (t = 0; t < MT_DRV_DISP_INTF_ID_MAX; t++)
    {
        if (pstDisp->stSetting.stIntf[t].bOpen)
        {
            DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
            if (pfOpt)
            {
                if ( pfOpt->PF_ReleaseIntf2)
                    pfOpt->PF_ReleaseIntf2(pstDisp->enDisp, &pstDisp->stSetting.stIntf[t]);
            }
            // s3 set intf
            DispCleanIntf(&pstDisp->stSetting.stIntf[t]);
        }
    }
#endif
    return;
}

mt_s32 DispSetFormat(MT_DRV_DISPLAY_E eDisp, MT_DRV_DISP_FMT_E eFmt, MT_DRV_DISP_STEREO_E enStereo)
{
#if 0
    DISP_S* pstDisp;
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    mt_u32 u;
    mt_s32 nRet;

    DispGetPointerByID(eDisp, pstDisp);
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_ResetIntfFmt2);

    // s1 set channel
    if (MT_DRV_DISP_FMT_CUSTOM == eFmt)
        nRet = pstDisp->pstIntfOpt->PF_SetChnTiming(pstDisp->enDisp, &pstDisp->stSetting.stCustomTimg);
    else
        nRet = pstDisp->pstIntfOpt->PF_SetChnFmt(pstDisp->enDisp, eFmt, enStereo);

    if (MT_SUCCESS != nRet)
    {
        DISP_ERROR("set disp%d  fmt %d err !(%d)\n",eDisp,eFmt,nRet);
        return nRet;
    }

    // s2 set interface  if necessarily
    for (u = 0; u < MT_DRV_DISP_INTF_ID_MAX; u++)
    {
        if (pstDisp->stSetting.stIntf[u].bOpen)
        {
            pfOpt->PF_ResetIntfFmt2(pstDisp->enDisp, &pstDisp->stSetting.stIntf[u], eFmt,&pstDisp->stSetting.stCustomTimg);
        }
    }

    DispProduceDisplayInfo(pstDisp, &pstDisp->stDispInfo);
#endif
    return MT_SUCCESS;
}

MT_VOID DISP_ALG_CscRgb2Yuv(ALG_COLOR_S *pstRgbColor, ALG_COLOR_S *pYuvColor)
{
    mt_u8 Red;
    mt_u8 Green;
    mt_u8 Blue;

    Red = pstRgbColor->u8Red;
    Green = pstRgbColor->u8Green;
    Blue = pstRgbColor->u8Blue;

    pYuvColor->u8Y = 257 * Red / 1000 + 504 * Green / 1000 + 98 * Blue / 1000 + 16;
    pYuvColor->u8Cb = -148 * Red / 1000 - 291 * Green / 1000 + 439 * Blue / 1000 + 128;
    pYuvColor->u8Cr = 439 * Red / 1000 - 368 * Green / 1000 - 71 * Blue / 1000 + 128;

    if (pYuvColor->u8Y < 16)
    {
        pYuvColor->u8Y = 16;
    }

    if (pYuvColor->u8Y > 235)
    {
        pYuvColor->u8Y = 235;
    }

    if (pYuvColor->u8Cb < 16)
    {
        pYuvColor->u8Cb = 16;
    }

    if (pYuvColor->u8Cb > 240)
    {
        pYuvColor->u8Cb = 240;
    }

    if (pYuvColor->u8Cr < 16)
    {
        pYuvColor->u8Cr = 16;
    }

    if (pYuvColor->u8Cr > 240)
    {
        pYuvColor->u8Cr = 240;
    }
}

mt_s32 DispSetColor(DISP_S *pstDisp)
{
    mt_s32 nRet = MT_SUCCESS;
    DISP_HAL_COLOR_S stColor;
    MT_DRV_DISP_COLOR_SETTING_S *pstC = &pstDisp->stSetting.stColor;
    ALG_COLOR_S stAlgC;
    mt_u32 bgColor;

    stColor.enInputCS = pstC->enInCS;
    stColor.enOutputCS = MT_DRV_CS_BT709_RGB_FULL;

    stAlgC.u8Red = pstDisp->stSetting.stBgColor.u8Red;
    stAlgC.u8Green = pstDisp->stSetting.stBgColor.u8Green;
    stAlgC.u8Blue = pstDisp->stSetting.stBgColor.u8Blue;

    //    nRet = pstDisp->pstIntfOpt->PF_SetChnColor(pstDisp->enDisp, &stColor);

    //    pstDisp->pstIntfOpt->PF_SetChnBgColor(pstDisp->enDisp,
    //                                          pstDisp->stSetting.stColor.enInCS,
    //                                          &pstDisp->stSetting.stBgColor);

    DISP_ALG_CscRgb2Yuv(&stAlgC, &stAlgC);
    bgColor = (0x10000000 | (stAlgC.u8Y << 16) | (stAlgC.u8Cb << 8) | (stAlgC.u8Cr));
    drv_reg_4k_disp_set_background_color(bgColor);

    return nRet;
}

mt_s32 DispSetIntfEnable(DISP_S *pstDisp, MT_BOOL bEnable)
{
#if 0
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    mt_u32 u;

    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_SetIntfEnable2);
    // s1 set interface if necessarily
    for (u = 0; u < MT_DRV_DISP_INTF_ID_MAX; u++)
    {
        if (pstDisp->stSetting.stIntf[u].bOpen)
        {
            //DispSetIntfLink(pstDisp, u);
            pfOpt->PF_SetIntfEnable2(pstDisp->enDisp, &pstDisp->stSetting.stIntf[u], bEnable);
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DispSetEnable(DISP_S *pstDisp, MT_BOOL bEnable)
{
#if 0
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    mt_u32 u;
    mt_s32 nRet;
    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_SetIntfEnable2);
    DispCheckNullPointer(pfOpt->PF_SetChnEnable);

    // s1 set interface if necessarily
    for (u = 0; u < MT_DRV_DISP_INTF_ID_MAX; u++)
    {
        if (pstDisp->stSetting.stIntf[u].bOpen)
        {
            //DispSetIntfLink(pstDisp, u);
            pfOpt->PF_SetIntfEnable2(pstDisp->enDisp, &pstDisp->stSetting.stIntf[u], bEnable);
        }
    }

    // s2 set channel
    nRet = pfOpt->PF_SetChnEnable(pstDisp->enDisp, bEnable);
#endif
    return MT_SUCCESS;
}

/******************************************************************************
  display function
 *****************************************************************************/
#define DEF_DRV_DISP_API_FUNCTION_START_HERE

mt_s32 DISP_GetInitFlag(MT_BOOL *pbInited)
{
    DispCheckNullPointer(pbInited);

    *pbInited = (s_s32DisplayGlobalFlag == DISP_DEVICE_STATE_OPEN) ? MT_TRUE : MT_FALSE;

    return MT_SUCCESS;
}

mt_s32 DISP_GetVersion(MT_DRV_DISP_VERSION_S *pstVersion)
{
#if 0
    DispCheckNullPointer(pstVersion);

    // check whether display is inited.
    DispCheckDeviceState();

    // return version
    DISP_HAL_GetVersion(pstVersion);
#endif
    return MT_SUCCESS;
}

MT_BOOL DISP_IsOpened(MT_DRV_DISPLAY_E enDisp)
{

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);
    // s2 return display OPEN state
    return s_stDisplayDevice.stDisp[enDisp - MT_DRV_DISPLAY_0].bOpen;

    return MT_TRUE;
}

MT_BOOL DISP_IsSameSource(MT_DRV_DISPLAY_E enDisp)
{
#if 0
    MT_DRV_DISPLAY_E  enSlave;
    DISP_S* pstM, *pstS;

    DispGetPointerByID(enDisp, pstM);
    enSlave = pstM->enAttachedDisp;

    if (enSlave >= MT_DRV_DISPLAY_BUTT)
        return MT_FALSE;

    DispGetPointerByID(enSlave, pstS);
    if (pstS->enAttachedDisp == enDisp)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
#endif
    return MT_TRUE;
}

MT_BOOL DISP_IsFollowed(MT_DRV_DISPLAY_E enDisp)
{
#if 0
    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);

    // s2 return display OPEN state
    return s_stDisplayDevice.stDisp[enDisp - MT_DRV_DISPLAY_0].bIsMaster;
#endif
    return MT_TRUE;
}


#if 0//def CONFIG_HDMI
RET_CODE disp_hdmi_notify(mt_u32 event, mt_u32 param, ulong context)
{
    disp_priv_t *p_dp = (disp_priv_t *)context;

    //printk("%s, %d, [0x%x],[0x%x],[0x%x],[0x%x],[0x%x]\n", __FUNCTION__, __LINE__, event, param, p_dp->hdmi_vcfg_flag, p_dp->tasklet_flag, p_dp->hdmi_vcfg.hdcp_on_off);

    //MT_INFO_VO("----------------\n");
#if 0
    if (HDMI_EVENT_CONNECTION_STATUS == (event & HDMI_EVENT_CONNECTION_STATUS))
    {
        if (param)
        {
            if (p_dp->hdmi_vcfg_flag && p_dp->tasklet_flag)
            {
                schedule_work(&p_dp->work_hdmi_cb);
                //p_dp->hdmi_vcfg_flag = MT_FALSE;
            }

            MT_INFO_VO("HDMI video connected\n");
        }
        else
        {
            p_dp->hdmi_vcfg_flag = MT_TRUE;
            DISP_DEBUGK("HDMI video disconnected\n");
        }
    }
#endif


    if ((HMDI_VIDEO_CONFIG_EVENT == (event & HMDI_VIDEO_CONFIG_EVENT)))
    {
        p_dp->hdmi_vcfg_flag = MT_TRUE;
        if (p_dp->hdmi_vcfg_flag && p_dp->tasklet_flag)
        {
            DISP_DEBUGK("HDMI HMDI_VIDEO_CONFIG_EVENT: start do hdmi video config \n");
            schedule_work(&p_dp->work_hdmi_cb);
            //p_dp->hdmi_vcfg_flag = MT_FALSE;
        }
    }

#if 0  // no used form 2021-5-19, new notify event instead
    if ((HDMI_EVENT_INNER_CONNECTION_STATUS == (event & HDMI_EVENT_INNER_CONNECTION_STATUS)))
    {
        if (param)
        {
            if (p_dp->hdmi_vcfg_flag && p_dp->tasklet_flag)
            {
                schedule_work(&p_dp->work_hdmi_cb);
                //p_dp->hdmi_vcfg_flag = MT_FALSE;
            }

            DISP_DEBUGK("HDMI video connected\n");
        }
        else
        {
            p_dp->hdmi_vcfg_flag = MT_TRUE;
            DISP_DEBUGK("HDMI video disconnected\n");
        }
    }
#endif

#if 0
    if(HDMI_EVENT_HDCP_CFG_CHG == (event & HDMI_EVENT_HDCP_CFG_CHG))
    {
        if(param)
        {
            p_dp->hdmi_vcfg.hdcp_on_off = 1;
        }
        else
        {
            p_dp->hdmi_vcfg.hdcp_on_off = 0;
        }
        DISP_DEBUGK("disp drv HDMI hdcp onoff %d\n", p_dp->hdmi_vcfg.hdcp_on_off);
    }

    if (HDMI_EVENT_VIDEO_FMT_CHG == (event & HDMI_EVENT_VIDEO_FMT_CHG))
    {
        DISP_DEBUGK("HDMI input video format change\n");
    }

    if (HDMI_EVENT_VIDEO_CLK_CHG == (event & HDMI_EVENT_VIDEO_CLK_CHG))
    {
        DISP_DEBUGK("HDMI input video clock change\n");
    }

    if (HDMI_EVENT_HDCP_AUTH_STATUS == (event & HDMI_EVENT_HDCP_AUTH_STATUS))
    {
        if (param)
        {
            DISP_DEBUGK("HDCP authentication fail\n");
        }
        else
        {
            DISP_DEBUGK("HDCP authentication pass\n");
        }
    }
#endif
    return MT_SUCCESS;
}
#endif


void  DISP_HAL_Init(disp_priv_t *p_dp)
{
    mt_u32 sd_wb_field_no = sd_wrback_field_num; //ARIA_SD_WR_BACK_FIELD_NO;
    mt_u32 value = 0;
    mt_u32 value2 = 0;
    //mzhu_fpga
    disp_hal_put_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf140010), 0);


    value = disp_hal_get_u32((volatile mt_u32 *)SYMPHONY_IO_VA(REG_SYMPHONY_ANA_AO_REG1_ADDR));
    value = value & 0xf0ffffff;//bit24~bit27 set 0
    disp_hal_put_u32((volatile mt_u32 *)SYMPHONY_IO_VA(REG_SYMPHONY_ANA_AO_REG1_ADDR), value);

    //enable vbi in default
    value = disp_hal_get_u32((volatile mt_u32 *)(VOUT_CLKEN_REG));
    value |= (0x1 << 1);//bit1 set 0
    disp_hal_put_u32((volatile mt_u32 *)(VOUT_CLKEN_REG), value);

    //set vbi priority
    reg_symphony_sd_encoder_set_dac_offset_field1_other_vbi_first(0xf);
    reg_symphony_sd_encoder_set_dac_offset_field2_other_vbi_first(0xf);
    reg_symphony_sd_encoder_set_dac_offset_vbi_priority_mode(1);

    //enable hdenv auto clk gate
    value = disp_hal_get_u32((volatile mt_u32 *)mt_get_hdvenc_base());
    value |= (0x1 << 31);//bit31 set 1
    disp_hal_put_u32((volatile mt_u32 *)mt_get_hdvenc_base(), value);

    //enalbe sdenv auto clk gate
    value = disp_hal_get_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec));
    value |= (0x1 << 8);//bit8 set 1
    disp_hal_put_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec), value);

    drv_reg_4k_disp_set_osdl_osd0_cmd_force_progressive_mode(1);
    drv_reg_4k_disp_set_osdl_osd1_cmd_force_progressive_mode(1);
    drv_reg_4k_disp_set_osdl_sub_cmd_force_progressive_mode(1);

    drv_reg_4k_disp_set_sd_wrback_addr_odd(u32SdWbAddr >> 3);
    drv_reg_4k_disp_set_sd_wrback_addr_even(0xca80);
    drv_reg_4k_disp_set_sd_wr_ctrl_new_sd_buffer_ctrl_mode(0);  //hdmi buffer mode
    if (sd_wb_field_no == 1)
    {
        drv_reg_4k_disp_set_sd_wr_ctrl(0x10078);
        p_dp->clock_1001_enable = 1;
    }
    else
    {
        drv_reg_4k_disp_set_sd_wr_ctrl_sd_buffer_number(sd_wb_field_no / 2 - 1);
    }
    drv_reg_4k_disp_set_sd_blankscreen_mode_cfg_sd_blankscreen_mode(1);

    //default set to 2scaler mode
    drv_reg_4k_disp_set_sd_video_path_ctrl_sd_video_path_en(0);

    if(p_dp->b_wrback_422)
    {
        drv_reg_4k_disp_set_sd_wr_ctrl_sd_wrback_yuv444(0);
    }
    else
    {
        drv_reg_4k_disp_set_sd_wr_ctrl_sd_wrback_yuv444(1);
    }

    drv_reg_4k_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(0);
    
    reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(1);

    disp_hal_set_sd_venc_ram();
    drv_reg_4k_disp_set_hd_video_effect_coef2_adj_update_flag(0);
    drv_reg_4k_disp_set_sd_video_effect_coef2_adj_update_flag(0);

	drv_reg_4k_disp_set_osdm_cmd_osd_sub_mux_sel(0);

    //config dce before isr turned on
    disp_dce_cfg(MT_DRV_DISP_PPMODE_DEFAULT);

    //dac chal sel
    value = disp_hal_get_u32((volatile mt_u32 *)(REG_ARIA_SD_ENC_CFIG6));
    value &= 0xff00ffff;
    value |= 0x00c60000;
    disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_CFIG6, value);

    /*nRet = DISP_ISR_Init();*/
    value = disp_hal_get_u32((volatile mt_u32 *)(REG_ARIA_HD_ENCODER_BASE + 0xf4));
    value2 = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT) & 0xfff;
    value &= 0xfffff000;
    value |= 0xf0000000;
    value |= value2;
    //value |= 0xf0000200;

    DISP_DEBUGK("[%s]line %d, value %08x\n", __FUNCTION__, __LINE__, value);
    disp_hal_put_u32((volatile mt_u32 *)(REG_ARIA_HD_ENCODER_BASE + 0xf4), value & 0xf0ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x18);
    disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N1, 0x0);

    //video quality
    drv_reg_4k_disp_set_disp_di_spatial_ctrl_1_di_enable_new_sr_flag(0);

    //top start num enable,for av_ap top isr sync
    disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_HD_ENCODER_TOP_START_DEBUG, 0x80000000);

    //drv_reg_4k_disp_set_osdc_irq_en_osdc_end_irq_en(1); //only for fpga osdc irq test - dean
}

mt_s32 DISP_Init(mt_void)
{
    mt_s32 nRet = 0;
    mt_u32 i = 0;
    //MT_VDP_PQ_INFO_S stTimingInfo;
    MT_DRV_DISPLAY_E eDisp;
    MT_DRV_DISP_VERSION_S stDispVersion;
    DISP_MMZ_BUF_S *p_stMem = NULL;
    disp_priv_t *p_dp = &s_stDisplayPriv;
    DISP_S *pstDisp = MT_NULL;
    MT_DRV_DISP_FMT_E sd_drv_tvsys = MT_DRV_DISP_FMT_PAL;
    MT_DRV_DISP_FMT_E hd_drv_tvsys = MT_DRV_DISP_FMT_1080i_50;
#ifdef CONFIG_HDMI

    //    HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;
    //    hdmi_notify_info_t notify_info = { 0 };
    s32 ret = 0;
#endif

    DISP_DEBUGK("[%s]line%d\n", __FUNCTION__, __LINE__);
    u32SdWbAddr = (phys_addr_t)sd_wrback_addr;
    disp_vbi_buffer.vbi_type = MT_DISP_VBI_TYPE_BUTT;

    for(i = 0; i < MAX_WIN_NUM; i++)
    {
        atomic_set(&smallwindow_flag[i], 0);
        atomic_set(&cropwindow_flag[i], 0);
    }
    
    atomic_set(&tvsyschange_flag, 0);

    DISP_MEMSET(&s_stDisplayPriv, 0, sizeof(disp_priv_t));
    p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt = VID_SYS_PAL;
    p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt = VID_SYS_1080I_50HZ;
    p_dp->b_uninit = b_uboot_tvsys_uninit;
    p_dp->b_wrback_422 = b_sd_wrback_422;
    p_dp->pp_mode = PP_MODE_DEFAULT;
    p_dp->csc_info.csc_enable = MT_TRUE;
    p_dp->vdac0_in = MT_TRUE;
    p_dp->vdac3_in = MT_TRUE;

    memset(&stDispVersion, 0x0, sizeof(MT_DRV_DISP_VERSION_S));
    if (s_s32DisplayGlobalFlag != DISP_DEVICE_STATE_CLOSE)
    {
        DISP_INFO("DISPLAY has been inited");
        return MT_SUCCESS;
    }

    DISP_MEMSET(&s_stDisplayDevice, 0, sizeof(DISP_DEV_S));

    // s1 get interface operation
#if 0 //disp_hal
    nRet = DISP_HAL_Init(DISP_BASE_ADDRESS);
    if (nRet)
    {
        DISP_ERROR("DISP_HAL_Init failed!");
        goto __ERR_EXIT__;
    }

    nRet = DISP_HAL_GetOperation(&s_stDisplayDevice.stIntfOpt);
    nRet = DISP_HAL_GetVersion(&stDispVersion);

    // s1.1 init alg
    nRet = DISP_DA_Init(&stDispVersion);
    if (nRet)
    {
        DISP_ERROR("DISP_DA_Init failed!");
        goto __ERR_EXIT__;
    }

#endif


    p_hdenc_addr = mt_get_hdvenc_base();
    p_sdenc_addr = mt_get_sdvenc_base();
    p_disp_addr = mt_get_display_base();
    //p_sysctl_addr = mt_get_sys_ctrl_base();
    p_vbi_addr = mt_get_vbi_base();

    DISP_DEBUGK("[%s]line%d,virtual addr hd %lx, sd %lx,disp %lx\n", __FUNCTION__, __LINE__, p_hdenc_addr, p_sdenc_addr, p_disp_addr);

    // s2 inited display
    for (eDisp = MT_DRV_DISPLAY_0; eDisp < MT_DRV_DISPLAY_2; eDisp++)
    {
        DispInitDisplay(eDisp);
        s_stDisplayDevice.stDisp[eDisp].p_dp = p_dp;
    }
    if (u32SdWbAddr == 0)
    {
        if (MT_SUCCESS != BP_CreateSDWriteBackMem(p_dp->b_wrback_422, sd_wrback_field_num))
        {
            DISP_DEBUGK("!!!!Create SDWriteBackMem failed! %s but ignore, there will be bug\n", __FUNCTION__);
            // return MT_ERR_VO_MALLOC_FAILED;
        }
        p_stMem = BP_GetSDWriteBackMemInfo();

        u32SdWbAddr = p_stMem->u32StartPhyAddr;
    }
    //debug
    printk("%s: sd wrback addr 0x%lx\n",__FUNCTION__,(ulong)u32SdWbAddr);

    nRet = BP_CreateDebugTestMem();
    if (nRet)
    {
        DISP_DEBUGK("[%s]Create DebugTestMem fail\n", __FUNCTION__);
    }

    DISP_ISR_Init();
    DISP_HAL_Init(p_dp);

    //    DispResetHardware();

    s_s32DisplayGlobalFlag = DISP_DEVICE_STATE_OPEN;

    for (eDisp = MT_DRV_DISPLAY_0; eDisp < MT_DRV_DISPLAY_2; eDisp++)
    {
        DISP_S *pstDisp = MT_NULL;
        DispGetPointerByID(eDisp, pstDisp);

        nRet = DISP_Open(eDisp);
        DISP_DEBUGK("disp_open nRet %d ch %d %s %d\n", nRet, eDisp, __FUNCTION__, __LINE__);
    }

    p_dp->hdr_para.transparent_mode = 0;
    p_dp->hdr_para.hdr_display_Brightness = 0;
    p_dp->hdr_para.sdr_display_Brightness = 0;
    p_dp->hdr_para.target_display_adaptation_tuning_level = 0;
    p_dp->hdr_para.display_OETF = 0;

    //to improve cvbs
    //disp_hal_put_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xbf5d0010), 0xb32ae0);

    //disp_aria_cvbs_onoff(p_dp, CVBS_GRP0, MT_TRUE);
    p_dp->venc_info.cvbs_dac[CVBS_GRP0].cvbs_grp_id = CVBS_GRP0;
    p_dp->venc_info.cvbs_dac[CVBS_GRP0].b_on = MT_TRUE;

    if (p_dp->b_uninit == TRUE)
    {
        p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt = VID_SYS_1080I_50HZ;
        p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt = VID_SYS_PAL;
    }
    else
    {
        sd_drv_tvsys = get_tvsys_from_reg(DISP_CHANNEL_SD);
        hd_drv_tvsys = get_tvsys_from_reg(DISP_CHANNEL_HD);

        DispGetPointerByID(MT_DRV_DISPLAY_1, pstDisp);
        pstDisp->stSetting.enFormat = hd_drv_tvsys;
        p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt = transfer_vid_sys_fmt(hd_drv_tvsys);

        DispGetPointerByID(MT_DRV_DISPLAY_0, pstDisp);
        pstDisp->stSetting.enFormat = sd_drv_tvsys;
        p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt = transfer_vid_sys_fmt(sd_drv_tvsys);

        disp_hal_set_sd_venc(p_dp);
    }
    SdVidSys = p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt;
    HdVidSys = p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt;

    g_stDrvSetting[0].eSdVidSys = SdVidSys;
    g_stDrvSetting[0].eHdVidSys = HdVidSys;


#ifdef CONFIG_HDMI
    mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);
    disp_set_hdmi(p_dp);
    p_dp->hdmi_vcfg.hdcp_on_off = FALSE;

#if 0
    mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);
    DISP_DEBUGK("\n\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@ display getfunction from hdmi @@@@@@@@@@@@@@@@@\n\n\n");

    if (pstHDMIFunc && pstHDMIFunc->pfnHdmiNotifyRegister)
    {
        /*  // no used from 2021-5-9 new notify event instead
            notify_info.events =
            HDMI_EVENT_CONNECTION_STATUS | HDMI_EVENT_VIDEO_FMT_CHG |
            HDMI_EVENT_VIDEO_CLK_CHG | HDMI_EVENT_HDCP_AUTH_STATUS |
            HDMI_EVENT_HDCP_CFG_CHG | HDMI_EVENT_INNER_CONNECTION_STATUS;
            */
        notify_info.events = HMDI_VIDEO_CONFIG_EVENT;   // new notify event

        notify_info.notify = disp_hdmi_notify;
        notify_info.context = (ulong)p_dp;
        notify_info.id_type = HDMI_NOTIFY_ID_TYPE_DISPLAY;  // new add for indicate the module identity
        ret = pstHDMIFunc->pfnHdmiNotifyRegister(0, (mt_u32 *)&notify_info);
        DISP_INFO("\n\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@ hdmi notify@@@@@@@@@@@@@@@@@ ret 0x%x \n\n\n", ret);
    }
#endif

    if (SUCCESS == ret)
    {
        // tasklet_init(&p_dp->tasklet, disp_aria_hdmi_tasklet_cb, (unsigned long)p_dp);
        INIT_WORK(&p_dp->work_hdmi_cb, disp_aria_hdmi_tasklet_cb_work);
        p_dp->tasklet_flag = MT_TRUE;
    }
    else
    {
        DISP_ERROR("hdmi_notify_register failed: ret = 0x%x\n", ret);
    }
    p_dp->hdmi_vcfg_flag = MT_TRUE;

#endif

    INIT_WORK(&p_dp->work_hdmi_setformat, disp_hdmi_setformat_work);

    drv_disp_get_hdmi_edid();
    if (p_dp->b_uninit == TRUE)
    {
        DISP_SetFormat(MT_DRV_DISPLAY_1, MT_DRV_DISP_STEREO_NONE, MT_DRV_DISP_FMT_1080i_50);
        //DISP_SetFormat(MT_DRV_DISPLAY_0, MT_DRV_DISP_STEREO_NONE, MT_DRV_DISP_FMT_PAL);
        p_dp->b_uninit = FALSE;
    }

    nRet = request_irq(IRQ_ARIA_HD_GROUP0_ID, aria_disp_isr_group0, IRQF_TRIGGER_HIGH, "aria_disp_group0", MT_NULL);
    nRet = request_irq(IRQ_ARIA_SD_TOP_START_ID, aria_disp_isr_top_sd, IRQF_TRIGGER_HIGH, "aria_disp_top_sd", MT_NULL);
    //nRet = request_irq(IRQ_DISP_OP_REE_ID, aria_disp_isr_op_ree, IRQF_TRIGGER_HIGH, "aria_disp_op_ree", MT_NULL); // only for fpga test - dean
    //nRet = request_irq(IRQ_OSDC_ID, aria_disp_isr_osdc, IRQF_TRIGGER_RISING, "aria_disp_osdc", MT_NULL); // only for fpga test - dean
    DISP_DEBUGK("%s %d request_irq aria_disp_isr_top_sd !!!\n", __FUNCTION__, __LINE__);




    return MT_SUCCESS;
}


mt_s32 DISP_DeInit(mt_void)
{
    MT_DRV_DISPLAY_E eDisp;
    //disp_priv_t *p_dp = &s_stDisplayPriv;
    DISP_DEBUGK("%s %d \n", __FUNCTION__, __LINE__);

    DISP_PRINT("DISP_DeInit 001\n");

    if (DISP_DEVICE_STATE_CLOSE == s_s32DisplayGlobalFlag)
    {
        DISP_INFO("DISPLAY has NOT inited");
        return MT_SUCCESS;
    }

    DISP_PRINT("DISP_DeInit 002\n");


    // s2 inited display
    for (eDisp = MT_DRV_DISPLAY_0; eDisp < MT_DRV_DISPLAY_2; eDisp++)
    {
        DISP_Close(eDisp);
        DispDeInitDisplay(eDisp);
    }

    DISP_PRINT("DISP_DeInit 003\n");
    //DISP_MSLEEP(40);
    //s_stDisplayDevice.stIntfOpt.PF_ResetHardware();
    free_irq(IRQ_ARIA_HD_GROUP0_ID, MT_NULL);
    free_irq(IRQ_ARIA_SD_TOP_START_ID, MT_NULL);
    //free_irq(IRQ_DISP_OP_REE_ID, MT_NULL); // only for fpga test - dean

    if (sd_wrback_addr == 0)
    {
        BP_DestroySDWriteBackMem();
    }
    BP_DestroyDebugTestMem();
#if 0 //disp_hal
    free_irq(DISP_INT_NUMBER, &g_DispIrqHandle);
#endif

    DISP_ISR_DeInit();
    DISP_PRINT("DISP_DeInit 004\n");

#if 0 //disp_hal
    DISP_DA_DeInit();
#endif

#if 0 //disp_hal
    DISP_HAL_DeInit();
#endif

    DISP_MEMSET(&s_stDisplayDevice, 0, sizeof(DISP_DEV_S));

    DISP_PRINT("DISP_DeInit 005\n");

    s_s32DisplayGlobalFlag = DISP_DEVICE_STATE_CLOSE;

    return MT_SUCCESS;
}

mt_s32 DISP_Close_AllLayer(mt_void)
{
    DISP_DEBUGK("%s %d \n",__FUNCTION__,__LINE__);
    drv_reg_4k_disp_set_video_ctrl_1_video_sel(0);
    drv_reg_4k_disp_set_osdl_osd0_cmd_osd_layer_en(0);
    drv_reg_4k_disp_set_osdl_osd1_cmd_osd_layer_en(0);
    drv_reg_4k_disp_set_osdl_sub_cmd_osd_layer_en(0);
    drv_reg_4k_disp_set_still_control_still_select(0);

    return MT_SUCCESS;
}

/*here is just for fastbootup, because, android fastbootup, make image and  bootup,
  they use the same code, but runs in a different code branch.*/
volatile mt_u32 *pDispUoot2KernelFlag = MT_NULL;
mt_u32 Disp_GetFastbootupFlag(mt_void)
{
    pDispUoot2KernelFlag = (volatile mt_u32 *)DISP_IOADDRESS(DISP_USED_COMMON_REGISTER);
    return *pDispUoot2KernelFlag;
}

mt_u32 Disp_SetFastbootupFlag(mt_u32 u32Value)
{
    pDispUoot2KernelFlag = (volatile mt_u32 *)DISP_IOADDRESS(DISP_USED_COMMON_REGISTER);
    *pDispUoot2KernelFlag = u32Value;
    return MT_SUCCESS;
}

mt_s32 DISP_Suspend(mt_void)
{
#if 1
    DISP_S *pstDisp;

    DispGetPointerByID(MT_DRV_DISPLAY_1, pstDisp);
    b_uboot_tvsys_uninit = TRUE;
    pstDisp->p_dp->b_uninit = TRUE;
    DISP_DEBUGK(" func [%s] line %d \n", __FUNCTION__, __LINE__);

    free_irq(IRQ_ARIA_HD_GROUP0_ID, MT_NULL);
    free_irq(IRQ_ARIA_SD_TOP_START_ID, MT_NULL);
    //free_irq(IRQ_DISP_OP_REE_ID, MT_NULL); // only for fpga test - dean

    disp_update_cvbs_for_sd_tvsys(pstDisp->p_dp, MT_FALSE);

    disp_backup();
#endif
#if 0
    DISP_S* pstDisp;
    MT_DRV_DISPLAY_E enD;
    DISP_INTF_OPERATION_S* pfHal = DISP_HAL_GetOperationPtr();
    //DISP_INTF_OPERATION_S *pfOpt = DISP_HAL_GetOperationPtr();

    DispCheckDeviceState();
    DispCheckNullPointer(pfHal);
    DispCheckNullPointer(pfHal->PF_VDP_RegSave);

    if (DISP_DEVICE_STATE_OPEN == s_s32DisplayGlobalFlag)
    {
        for (enD = MT_DRV_DISPLAY_0; enD < MT_DRV_DISPLAY_BUTT; enD++)
        {
            DispGetPointerByIDNoReturn(enD, pstDisp);

            pstDisp->bStateBackup = pstDisp->bEnable;

            if (pstDisp->bEnable == MT_TRUE)
            {
                DISP_SetEnable(enD, MT_FALSE);
            }
        }

        DISP_ISR_Suspend();

        /*save VDP reg*/
        if (MT_SUCCESS != pfHal->PF_VDP_RegSave())
        {
            DISP_ERROR("Display save registers for suspend failed!\n");
            return MT_ERR_DISP_MALLOC_MAP_ERR;
        }
        /*dts:DTS2013080709083, we should reset the module and close all the clk.*/
        DispCloseClkResetModule();

        DispClearHardwareState();

        s_s32DisplayGlobalFlag = DISP_DEVICE_STATE_SUSPEND;
    }

    /*clear the flag passed from uboot to kernel, which indicates a android fastbootup mode.
     * since the reg is stored for disp for ever, so even in non-fastbootup mode ,we can operate the reg eigther.*/
    Disp_SetFastbootupFlag(0);

#endif

    return MT_SUCCESS;
}

mt_s32 DISP_Resume(mt_void)
{
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 ret = 0;

    DISP_S *pstDisp;
    disp_priv_t *p_dp = &s_stDisplayPriv;

    DISP_DEBUGK(" func [%s] line %d \n", __FUNCTION__, __LINE__);
    DISP_HAL_Init(p_dp);

    p_dp->b_table_load = MT_FALSE;
    p_dp->tasklet_flag = MT_TRUE;
    p_dp->hdmi_vcfg_flag = MT_TRUE;
    p_dp->csc_info.old_resolution = 0xff;

    disp_restore();

    DispGetPointerByID(MT_DRV_DISPLAY_1, pstDisp);
    DISP_SetFormat(MT_DRV_DISPLAY_1, (int)pstDisp->stSetting.eDispMode, pstDisp->stSetting.enFormat);

    DispGetPointerByID(MT_DRV_DISPLAY_0, pstDisp);
    DISP_SetFormat(MT_DRV_DISPLAY_0, (int)pstDisp->stSetting.eDispMode, pstDisp->stSetting.enFormat);

    p_dp->b_uninit = FALSE;

    ret = request_irq(IRQ_ARIA_HD_GROUP0_ID, aria_disp_isr_group0, IRQF_TRIGGER_HIGH, "aria_disp_group0", MT_NULL);
    if(ret)
    {
        DISP_DEBUGK("%s request_irq %d faild!!!\n", __FUNCTION__, IRQ_ARIA_HD_GROUP0_ID);
        nRet = MT_FAILURE;
    }
    ret = request_irq(IRQ_ARIA_SD_TOP_START_ID, aria_disp_isr_top_sd, IRQF_TRIGGER_HIGH, "aria_disp_top_sd", MT_NULL);
    if(ret)
    {
        DISP_DEBUGK("%s request_irq %d faild!!!\n", __FUNCTION__, IRQ_ARIA_SD_TOP_START_ID);
        nRet = MT_FAILURE;
    }

    disp_update_cvbs_for_sd_tvsys(p_dp, MT_TRUE);

    return nRet;

}

mt_s32 DISP_Attach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    DISP_S *pstM, *pstS;
    mt_rect_s stvirscreen;
#if 0
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckNullPointer(pfOpt);
    DispCheckNullPointer(pfOpt->PF_TestChnAttach);
    if ( !pfOpt->PF_TestChnAttach(enMaster, enSlave) )
    {
        DISP_ERROR("Display NOT support %d attach to %d!\n", enSlave, enMaster);
        return MT_ERR_DISP_INVALID_OPT;
    }
#endif

    DispGetPointerByID(enMaster, pstM);
    DispGetPointerByID(enSlave, pstS);

    if (pstM->bIsMaster && (pstM->enAttachedDisp == enSlave)) {
        DISP_INFO("Display has been ATTACHED!\n");
        return MT_SUCCESS;
    }

    if (pstM->bIsMaster || pstM->bIsSlave || pstS->bIsMaster || pstS->bIsSlave) {
        DISP_ERROR("Display has been opened!\n");
        return MT_ERR_DISP_INVALID_OPT;
    }

    pstM->bIsMaster = MT_TRUE;
    pstM->bIsSlave = MT_FALSE;
    pstM->enAttachedDisp = enSlave;

    pstS->bIsMaster = MT_FALSE;
    pstS->bIsSlave = MT_TRUE;
    pstS->enAttachedDisp = enMaster;
    memset(&stvirscreen, 0x0, sizeof(mt_rect_s));

    DISP_GetVirtScreen(enMaster, &stvirscreen);
    DISP_SetVirtScreen(enMaster, stvirscreen);
    return MT_SUCCESS;
}

mt_s32 DISP_Detach(MT_DRV_DISPLAY_E enMaster, MT_DRV_DISPLAY_E enSlave)
{
    DISP_S *pstM, *pstS;

    // s1 check input parameters
    DispCheckDeviceState();

    // s2 set detach
    DispGetPointerByID(enMaster, pstM);
    DispGetPointerByID(enSlave, pstS);

    if (!pstM->bIsMaster || (pstM->enAttachedDisp != enSlave)) {
        DISP_INFO("Display has NOT been ATTACHED!\n");
        return MT_SUCCESS;
    }

    pstM->bIsMaster = MT_FALSE;
    pstM->enAttachedDisp = MT_DRV_DISPLAY_BUTT;
    pstM->bDispSettingChange = MT_TRUE;

    pstS->bIsSlave = MT_FALSE;
    pstS->enAttachedDisp = MT_DRV_DISPLAY_BUTT;
    pstS->bDispSettingChange = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 DISP_SetDACDetEn(MT_BOOL bDACDetEn)
{
#if 0
    DISP_INTF_OPERATION_S* pfHal = DISP_HAL_GetOperationPtr();
    if (MT_NULL == pfHal)
    {
        DISP_ERROR("get null ptr!\n");
        return MT_ERR_DISP_NULL_PTR;
    }
    pfHal->PF_SetDACDetEn(bDACDetEn);
#endif
    return MT_SUCCESS;
}
mt_s32 DISP_SetAllDacEn(MT_BOOL bDacEn)
{
    return MT_SUCCESS;
}
mt_s32 DISP_GetDACAttr(MT_DRV_VDAC_ATTR_S *pDACAttr)
{

    return MT_SUCCESS;
}

#define DISPLAY0_BUS_UNDERFLOW_INT 0x00000080UL
#define DISPLAY1_BUS_UNDERFLOW_INT 0x00000008UL
mt_s32 DISP_Update_Setting(DISP_S *pstDisp)
{
    return MT_SUCCESS;
}

//#define CHECK_SD_WB_STATUS

#if 0  //#ifdef CONFIG_EMU
mt_s32 disp_timer_id = -1;
void disp_timer(void *para)
{
    if(reg_aria1_disp_get_sd_status_sdwr_fifo_full())
    {printk("PEF-feature8: SDwrite overflow \n");}

}
#endif //#endif

mt_void DISP_CB_PreProcess(mt_handle hHandle, const MT_DRV_DISP_CALLBACK_INFO_S *pstInfo)
{
    disp_priv_t *p_dp = &s_stDisplayPriv;
    DISP_S *pstDisp = (DISP_S *)hHandle;
    pstDisp->stDispInfo = pstInfo->stDispInfo;
    /** �������� **/
    if (!pstInfo->stDispInfo.bIsBottomField)
    {
        DISP_Update_Setting(pstDisp);
        disp_st_tvsys_update(MT_DRV_DISPLAY_1);

        //schedule_work(&p_dp->work_set_usrinfo);
        //update denoise setting
        //disp_denoise_update();

        //disp_aria_gra_scale_update(MT_DRV_DISPLAY_1);

        DispProduceDisplayInfo(pstDisp, &pstDisp->stDispInfo);
        DISP_ISR_SetDispInfo(pstDisp->enDisp, &pstDisp->stDispInfo);
    }

    //update csc setting, EMP should be configured at both top and bottom field for hdr10+
    disp_csc_update(p_dp);
    disp_aria_gra_scale_update(MT_DRV_DISPLAY_1);
    if(b_disp_coeff_update == 1)
    {
        DISP_DEBUGK("%s %d reset b_disp_coeff_update \n", __FUNCTION__, __LINE__);
        b_disp_coeff_update = 0;
    }
    return;
}

mt_s32 DISP_init_cfg(MT_DRV_DISPLAY_E enDisp)
{
    //DISP_MMZ_BUF_S stFilterMem;
    DISP_S *pstDisp;
    //mt_s32 i = 0;
    mt_s32 nRet = MT_SUCCESS;

    //disp_priv_t *p_dp = &s_stDisplayPriv;
    //mt_s32 *p_coeff_addr = NULL;
    DispGetPointerByID(enDisp, pstDisp);
    DISP_DEBUGK("[%s]line%d\n", __FUNCTION__, __LINE__);

    //Cut the scaled video window
    //reg_aria_disp_set_hd_post_cfg_hd_leverage(0);

    pstDisp->v_aspect = AR_43;
    pstDisp->ar_mode = VID_ASPECT_MODE_ORIG;
    //pstDisp->vid_fmt = VID_SYS_PAL;

    //disp_aria_set_tv_sys(MT_DRV_DISPLAY_E enDisp, disp_sys_t fmt);

    // todo set sd writeback addr
    // dce cfg

    // request irq
    return nRet;
}

mt_s32 DISP_Open(MT_DRV_DISPLAY_E enDisp)
{

    DISP_S *pstDisp;
    mt_s32 nRet = MT_SUCCESS;

    MT_DRV_DISP_CALLBACK_S stCB;

    DISP_DEBUGK("%s %d\n", __FUNCTION__, __LINE__);

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);

    // s2 check whether display opened
    if (DISP_IsOpened(enDisp))
    {
        DISP_DEBUGK("Display has been opened!\n");
        return MT_SUCCESS;
    }

    if (MT_SUCCESS != DispCheckReadyForOpen(enDisp))
    {
        DISP_DEBUGK("format has not  been set!\n");
        return MT_ERR_DISP_NOT_SUPPORT_FMT;
    }
    // s2.0 reset hardware
    //DispResetHardware();

    // s2.1 get display channel
    DispGetPointerByID(enDisp, pstDisp);
#if 0
    if (( !pstDisp) || (!pstDisp->pstIntfOpt) ||  (!pstDisp->pstIntfOpt->PF_TestChnSupport))
    {
        DISP_ERROR(" %s has null ptr!\n", __FUNCTION__);
        return MT_ERR_DISP_NULL_PTR;
    }

    if (!pstDisp->pstIntfOpt->PF_TestChnSupport(enDisp))
    {
        DISP_ERROR("DISP ERROR! This version does not support display %d\n", (mt_s32)enDisp);
        return MT_ERR_DISP_INVALID_OPT;
    }
#endif

    // s3 check whether necessory attributes are configed

    if (enDisp != MT_DRV_DISPLAY_1)
    {
        DISP_DEBUGK("should return ch %d %s %d\n", enDisp, __FUNCTION__, __LINE__);

        return MT_SUCCESS;
    }

    DISP_init_cfg(enDisp);

    // s3.2 open display channel isr
    DISP_ISR_OpenChn(enDisp);

    // set display info for first time
    DISP_ISR_SetDispInfo(enDisp, &pstDisp->stDispInfo);

    // register display callback
    stCB.hDst = (mt_handle)pstDisp;
    stCB.pfDISP_Callback = DISP_CB_PreProcess;

    nRet = DISP_ISR_RegCallback(enDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB);
    DISP_ASSERT(!nRet);

    DISP_SetEnable(enDisp, MT_TRUE);
    // s4 set open state
    pstDisp->bOpen = MT_TRUE;

    return nRet;
}

mt_void DispReleaseIntf(MT_DRV_DISPLAY_E enDisp)
{
    return;
}

mt_s32 DISP_Close(MT_DRV_DISPLAY_E enDisp)
{
    DISP_S *pstDisp;

#ifdef MT_DISP_BUILD_FULL
    MT_DRV_DISP_CALLBACK_S stCB;
    mt_s32 nRet;
#endif

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);

    // s2 check whether display opened
    if (!DISP_IsOpened(enDisp))
    {
        DISP_INFO("Display is not opened!\n");
        return MT_SUCCESS;
    }

    // s4 set close state
    DispGetPointerByID(enDisp, pstDisp);

    // s3 Product ask for that display must be enabled at the same time.
    DISP_SetEnable(enDisp, MT_FALSE);
#ifdef MT_DISP_BUILD_FULL
    stCB.hDst = (mt_handle)pstDisp;
    stCB.pfDISP_Callback = DISP_CB_PreProcess;
    nRet = DISP_ISR_UnRegCallback(enDisp, MT_DRV_DISP_C_INTPOS_0_PERCENT, &stCB);
    DISP_ASSERT(!nRet);

    // s4.2 delete isr
    DISP_ISR_CloseChn(enDisp);
#endif

    pstDisp->bOpen = MT_FALSE;

    return MT_SUCCESS;
}

mt_s32 DispGetVactTime(DISP_S *pstDisp)
{
    mt_s32 vtime;

    if (pstDisp->stDispInfo.u32RefreshRate)
    {
        vtime = (1000 * 100) / pstDisp->stDispInfo.u32RefreshRate;
    }
    else
    {
        vtime = 50;
    }

    if (vtime > 50)
    {
        vtime = 50;
    }
    else if (vtime < 20)
    {
        vtime = 20;
    }

    return vtime;
}

mt_s32 DISP_SetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
#if 0
    DISP_S* pstDisp;
    DISP_S* pstDispS;
    mt_s32 nRet, u;
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);

    // s2 check state
    if (bEnable == pstDisp->bEnable)
    {
        DISP_PRINT(" DISP Set enable return!\n");
        return MT_SUCCESS;
    }

    //printk("Set enable 0002  DISP=%d, IS M= %d\n", enDisp, pstDisp->bIsMaster);

    if (pstDisp->bIsSlave)
    {
        return DISP_SetEnable(pstDisp->enAttachedDisp, bEnable);
    }

    if (pstDisp->bIsMaster)
    {
        DispGetPointerByID(pstDisp->enAttachedDisp, pstDispS);
        DispCheckNullPointer(pfOpt);
        DispCheckNullPointer(pfOpt->PF_SetMSChnEnable);

        // s3 if Enable, set all config
        if (bEnable)
        {
            nRet = DispCheckReadyForOpen(enDisp);
            if (nRet)
            {
                DISP_ERROR("not  Ready to Open!\n");
                return nRet;
            }

            nRet = DispCheckReadyForOpen(pstDisp->enAttachedDisp);

            if (nRet)
            {
                DISP_ERROR("not  Ready to Open!\n");
                return nRet;
            }

            // s1 set FMT
            nRet = DispSetFormat(enDisp, pstDisp->stSetting.enFormat, pstDisp->stSetting.eDispMode);

            if (nRet)
            {
                DISP_ERROR("Set format failed\n");
                return nRet;
            }

            // s2 set CSC
            nRet = DispSetColor(pstDisp);

            if (nRet)
            {
                DISP_ERROR("Set color failed\n");
                return nRet;
            }


            // s1 set FMT
            nRet = DispSetFormat(pstDisp->enAttachedDisp, pstDispS->stSetting.enFormat,pstDispS->stSetting.eDispMode);

            if (nRet)
            {
                DISP_ERROR("Set format failed\n");
                return nRet;
            }

            // s2 set CSC
            nRet = DispSetColor(pstDispS);

            if (nRet)
            {
                DISP_ERROR("Set color failed\n");
                return nRet;
            }

            // s3 set enable
            pstDisp->eState = DISP_PRIV_STATE_DISABLE;
            DispSetIntfEnable(pstDisp, bEnable);

            pstDispS->eState = DISP_PRIV_STATE_DISABLE;
            DispSetIntfEnable(pstDispS, bEnable);

            pfOpt->PF_SetMSChnEnable(pstDisp->enDisp, pstDisp->enAttachedDisp, 0, bEnable);

#if 0
            mt_drv_sys_gettimestampms(&pstDisp->u32StartTime);
#endif

            pstDisp->bEnable = bEnable;
            pstDispS->bEnable = bEnable;
        }
        else
        {
            mt_s32 vtime;

            // addtional 2ms delay for safe
            vtime = DispGetVactTime(pstDisp) + 2;

            // s1 set state and wait ISR Process
            pstDisp->bEnable = bEnable;
            pstDispS->bEnable = bEnable;

            DISP_MSLEEP(2 * vtime);

            u = 0;

            while (pstDisp->eState != DISP_PRIV_STATE_DISABLE)
            {
                DISP_MSLEEP(vtime);
                u++;

                if (u > DISP_SET_TIMEOUT_THRESHOLD)
                {
                    DISP_WARN("Set enable timeout\n");
                    break;
                }
            }

            // s2 set disable
            //DispSetEnable(pstDisp, bEnable);
            //DispSetEnable(pstDispS, bEnable);
            DispSetIntfEnable(pstDisp, bEnable);
            DispSetIntfEnable(pstDispS, bEnable);

            pfOpt->PF_SetMSChnEnable(pstDisp->enDisp, pstDisp->enAttachedDisp, vtime, bEnable);

#if 0
            mt_drv_sys_gettimestampms(&pstDisp->u32StartTime);
#endif
            //printk(">>>>>>>>>>>> close disp%d =0x%x\n", pstDisp->enDisp,pstDisp->u32StartTime);
        }
    }
    else
    {
        // s3 if Enable, set all config
        if (bEnable)
        {
            nRet = DispCheckReadyForOpen(enDisp);

            if (nRet)
            {
                return nRet;
            }

            // s1 set FMT
            nRet = DispSetFormat(enDisp, pstDisp->stSetting.enFormat, pstDisp->stSetting.eDispMode);

            if (nRet)
            {
                DISP_ERROR("Set format failed\n");
                return nRet;
            }

            // s2 set CSC
            nRet = DispSetColor(pstDisp);

            if (nRet)
            {
                DISP_ERROR("Set color failed\n");
                return nRet;
            }

            // s3 set enable
            pstDisp->eState = DISP_PRIV_STATE_DISABLE;
            DispSetEnable(pstDisp, bEnable);

#if 0
            mt_drv_sys_gettimestampms(&pstDisp->u32StartTime);
#endif
            pstDisp->bEnable = bEnable;

            //printk("disp%d =0x%x, ", pstDisp->enDisp,pstDisp->u32StartTime);
        }
        else
        {
            mt_s32 vtime;

            // addtional 2ms delay for safe
            vtime = DispGetVactTime(pstDisp) + 2;

            // s1 set state and wait ISR Process
            pstDisp->bEnable = bEnable;

            DISP_MSLEEP(2 * vtime);

            u = 0;

            while (pstDisp->eState != DISP_PRIV_STATE_DISABLE)
            {
                DISP_MSLEEP(vtime);
                u++;

                if (u > DISP_SET_TIMEOUT_THRESHOLD)
                {
                    DISP_WARN("Set enable timeout\n");
                    break;
                }
            }

            // s2 set disable
            DispSetEnable(pstDisp, bEnable);

            // s3 wait vdp diable really
            DISP_MSLEEP(vtime);
        }
    }
#endif
    return MT_SUCCESS;
}

MT_DRV_DISPLAY_E DISPGetIntfChannel(MT_DRV_DISP_INTF_ID_E enIntfID)
{
    MT_DRV_DISPLAY_E enDisp = MT_DRV_DISPLAY_BUTT;
    mt_u32 i = 0;
    DISP_S *pstDisp = MT_NULL;

    for (i = (mt_u32)MT_DRV_DISPLAY_0; i <= (mt_u32)MT_DRV_DISPLAY_1; i++)
    {
        DispGetPointerByID((MT_DRV_DISPLAY_E)i, pstDisp);
        if (pstDisp->stSetting.stIntf[enIntfID].bOpen)
        {
            enDisp = (MT_DRV_DISPLAY_E)i;
            break;
        }
    }
    return enDisp;
}

mt_s32 DISP_ExternSetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
#if 0
    DISP_S* pstDisp;
    DISP_S* pstDispS;

    mt_s32 nRet;
    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);
    DispGetPointerByID(pstDisp->enAttachedDisp, pstDispS);

    if (bEnable == pstDisp->bEnable)
    {
        DISP_PRINT(" DISP Set enable return!\n");
        return MT_SUCCESS;
    }

    if (bEnable)
    {
        if ((MT_DRV_DISPLAY_0 == DISPGetIntfChannel(MT_DRV_DISP_INTF_CVBS0)))
        {
            //printk("CVBS=====YPbPr======Delay=======500ms\n");
            DISP_MSLEEP(400);
        }
    }
    else
    {
        if ((enDisp == DISPGetIntfChannel(MT_DRV_DISP_INTF_HDMI0)) && !DispGetHdmiFunction())
        {
            if (s_pstHDMIFunc->pfnHdmiPreFormat && s_pstHDMIFunc->pfnHdmiSetFormat)
            {
                //printk("%s, %d-------hdmi-----disp%d--------close\n",__FUNCTION__, __LINE__,pstDisp->enDisp);
                s_pstHDMIFunc->pfnHdmiPreFormat(MT_UNF_HDMI_ID_0, pstDisp->stSetting.enFormat);
            }
        }
    }

    nRet = DISP_SetEnable(enDisp,bEnable);
    if (MT_SUCCESS != nRet)
    {
        DISP_ERROR("Display enable is failed\n");
        return nRet;
    }

    if (bEnable)
    {
        if ((enDisp == DISPGetIntfChannel(MT_DRV_DISP_INTF_HDMI0)) && !DispGetHdmiFunction())
        {
            if (s_pstHDMIFunc->pfnHdmiPreFormat && s_pstHDMIFunc->pfnHdmiSetFormat)
            {
                //printk("%s, %d-------hdmi-----disp%d--------open\n",__FUNCTION__, __LINE__,pstDisp->enDisp);
                s_pstHDMIFunc->pfnHdmiSetFormat(MT_UNF_HDMI_ID_0, pstDisp->stSetting.enFormat, pstDisp->stSetting.eDispMode);
            }
        }
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_GetEnable(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbEnable)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);
    DispCheckNullPointer(pbEnable);

    // s2 check whether display opened
    DispShouldBeOpened(enDisp);

    // s3 get ENABLE state and return
    DispGetPointerByID(enDisp, pstDisp);

    *pbEnable = pstDisp->bEnable;

    return MT_SUCCESS;
}

#ifndef __DISP_PLATFORM_BOOT__
//#define DISP_DEBUGK_TEST_SET_FORMAT_TIME 1
#endif
mt_s32 DISP_ReviseIntfRGB_VGA(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_FMT_E enEncFmt)
{
    DISP_S *pstDisp;
    DISP_INTF_S stDispIntfRGB;
    DISP_INTF_S stDispIntfVGA;

    DispGetPointerByID(enDisp, pstDisp);

    stDispIntfRGB = pstDisp->stSetting.stIntf[MT_DRV_DISP_INTF_RGB0];
    stDispIntfVGA = pstDisp->stSetting.stIntf[MT_DRV_DISP_INTF_VGA0];

    if ((enEncFmt >= MT_DRV_DISP_FMT_861D_640X480_60) && (enEncFmt <= MT_DRV_DISP_FMT_CUSTOM))
    {
        /*VESA or Custom FMT*/
        if (stDispIntfRGB.bOpen)
        {
            DISP_DelIntf(enDisp, &stDispIntfRGB.stIf);
            stDispIntfRGB.stIf.eID = MT_DRV_DISP_INTF_VGA0;
            DISP_AddIntf(enDisp, &stDispIntfRGB.stIf);
        }
    } else {
        if (stDispIntfVGA.bOpen)
        {
            DISP_DelIntf(enDisp, &stDispIntfVGA.stIf);
            stDispIntfVGA.stIf.eID = MT_DRV_DISP_INTF_RGB0;
            DISP_AddIntf(enDisp, &stDispIntfVGA.stIf);
        }
    }
    return MT_SUCCESS;
}

static disp_sys_t tvsys_hd_to_sd(disp_sys_t hd_fmt)
{
    disp_sys_t sd_fmt;
    switch(hd_fmt)
    {
    case VID_SYS_PAL:
    case VID_SYS_PAL_N:
    case VID_SYS_PAL_NC:
    case VID_SYS_PAL_M:
    case VID_SYS_NTSC_J:
    case VID_SYS_NTSC_M:
    case VID_SYS_NTSC_443:
    case VID_SYS_SECAM:
    case VID_SYS_AUTO:
        sd_fmt = hd_fmt;
        break;
    case VID_SYS_1080I_50HZ:
    case VID_SYS_1080P_25HZ:
    case VID_SYS_1080P_50HZ:
    case VID_SYS_1250I_50HZ:
    case VID_SYS_720P_25HZ:
    case VID_SYS_720P_50HZ:
    case VID_SYS_576P_50HZ:
    case VID_SYS_288P_50HZ:
    case VID_SYS_3840X2160_25HZ:
    case VID_SYS_3840X2160_50HZ:
    case VID_SYS_4096X2160_25HZ:
    case VID_SYS_4096X2160_50HZ:
        sd_fmt = VID_SYS_PAL;
        break;
    case VID_SYS_1080I:
    case VID_SYS_1080P:
    case VID_SYS_1080P_24HZ:
    case VID_SYS_1080P_30HZ:
    case VID_SYS_720P:
    case VID_SYS_720P_30HZ:
    case VID_SYS_720P_24HZ:
    case VID_SYS_480P:
    case VID_SYS_240P_60HZ:
    case VID_SYS_3840X2160_24HZ:
    case VID_SYS_3840X2160_30HZ:
    case VID_SYS_3840X2160_60HZ:
    case VID_SYS_4096X2160_24HZ:
    case VID_SYS_4096X2160_30HZ:
    case VID_SYS_4096X2160_60HZ:
        sd_fmt = VID_SYS_NTSC_J;
        break;
    default:
        sd_fmt = VID_SYS_PAL;
        break;
    }
    return sd_fmt;
}

mt_s32 DISP_SetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E enStereo, MT_DRV_DISP_FMT_E enEncFmt)
{
    DISP_S *pstDisp;
    MT_DRV_DISP_FMT_E enEncFmt2;
    mt_s32 nRet;
    disp_priv_t *p_dp;
    volatile mt_u32 dtmp = 0;
    mt_u32 i = 0;
#ifdef DISP_DEBUGK_TEST_SET_FORMAT_TIME
    struct timeval tv;
    mt_u32 t2, t1, t0;
#endif
    static MT_DRV_DISP_FMT_E old_tvsys_sd = MT_DRV_DISP_FMT_BUTT;
    static MT_DRV_DISP_FMT_E old_tvsys_hd = MT_DRV_DISP_FMT_BUTT;
    static disp_sys_t cur_tvsys_sd = VID_SYS_MAX;

    //if (old_tvsys_sd == MT_DRV_DISP_FMT_BUTT)
    old_tvsys_sd = get_tvsys_from_reg(DISP_CHANNEL_SD);

    //if (old_tvsys_hd == MT_DRV_DISP_FMT_BUTT)
    old_tvsys_hd = get_tvsys_from_reg(DISP_CHANNEL_HD);

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckID(enDisp);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);
    enEncFmt2 = DispTransferFormat(enDisp, enEncFmt);
    p_dp = pstDisp->p_dp;

    if (enEncFmt2 > MT_DRV_DISP_FMT_CUSTOM)
    {
        DISP_DEBUGK("Display fmt is invalid %s\n", __FUNCTION__);
        return MT_ERR_DISP_INVALID_PARA;
    }

    DISP_DEBUGK("%s: enDisp=%d, enStereo=%d, enEncFmt2=%d\n",__FUNCTION__,enDisp,enStereo,enEncFmt2);
    DISP_DEBUGK("%s: old_tvsys_sd=%d, old_tvsys_hd=%d, b_uninited=%d\n",__FUNCTION__,old_tvsys_sd,old_tvsys_hd,p_dp->b_uninit);

    if (enDisp == MT_DRV_DISPLAY_0)
    {
        if ((enEncFmt2 == old_tvsys_sd) && (p_dp->b_uninit == FALSE))
        {
            DISP_DEBUGK(" func [%s] lin %d, return ! \n", __FUNCTION__, __LINE__);

            return MT_SUCCESS;
        }
        else if ((MT_DRV_DISP_FMT_NTSC_J == enEncFmt2 || MT_DRV_DISP_FMT_NTSC_443 == enEncFmt2) &&
                (MT_DRV_DISP_FMT_NTSC_J == old_tvsys_sd ||
                 MT_DRV_DISP_FMT_NTSC_443 == old_tvsys_sd))
        {
            pstDisp->stSetting.enFormat = enEncFmt2;
            //old_tvsys_sd = enEncFmt2;
            DISP_DEBUGK(" func [%s] lin %d, return ! \n", __FUNCTION__, __LINE__);

            return MT_SUCCESS;
        }
        else if ((MT_DRV_DISP_FMT_PAL_N == enEncFmt2 || MT_DRV_DISP_FMT_PAL_Nc == enEncFmt2) &&
                (MT_DRV_DISP_FMT_PAL_N == old_tvsys_sd ||
                 MT_DRV_DISP_FMT_PAL_Nc == old_tvsys_sd))
        {
            pstDisp->stSetting.enFormat = enEncFmt2;
            //old_tvsys_sd = enEncFmt2;
            DISP_DEBUGK(" func [%s] lin %d, return ! \n", __FUNCTION__, __LINE__);

            return MT_SUCCESS;
        }
    }

    if (enDisp == MT_DRV_DISPLAY_1)
    {
        //if ((enEncFmt2 != MT_DRV_DISP_FMT_PAL) && (enEncFmt2 != MT_DRV_DISP_FMT_NTSC))
        {
            if ((enEncFmt2 == old_tvsys_hd)
                    && (p_dp->b_uninit == FALSE)
                    && (enStereo == (MT_DRV_DISP_STEREO_MODE_E)pstDisp->stSetting.eDispMode))
            {
                DISP_DEBUGK(" func [%s] lin %d, return ! \n", __FUNCTION__, __LINE__);
                return MT_SUCCESS;
            }
        }
    }

    pstDisp->stSetting.eDispMode = (int)enStereo;
    pstDisp->stSetting.enFormat = enEncFmt2;

#if 0 //disp_hal
    if (!pstDisp->pstIntfOpt->PF_TestChnEncFmt(pstDisp->enDisp, enEncFmt2))
    {
        DISP_ERROR("Display %d does not support fmt %d\n", (mt_s32)enDisp, (mt_s32)enEncFmt2);
        return MT_ERR_DISP_INVALID_PARA;
    }
#endif

    switch (enEncFmt2)
    {
    case MT_DRV_DISP_FMT_1080P_60:
    case MT_DRV_DISP_FMT_1080P_59_94:
        
        pstDisp->vid_fmt = VID_SYS_1080P; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_50:
        pstDisp->vid_fmt = VID_SYS_1080P_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_30:
    case MT_DRV_DISP_FMT_1080P_29_97:        
        pstDisp->vid_fmt = VID_SYS_1080P_30HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_25:
        pstDisp->vid_fmt = VID_SYS_1080P_25HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080P_24:
        pstDisp->vid_fmt = VID_SYS_1080P_24HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080i_60:
    case MT_DRV_DISP_FMT_1080i_59_94:        
        pstDisp->vid_fmt = VID_SYS_1080I; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_1080i_50:
        pstDisp->vid_fmt = VID_SYS_1080I_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_720P_60:
    case MT_DRV_DISP_FMT_720P_59_94:
        pstDisp->vid_fmt = VID_SYS_720P; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_720P_50:
        pstDisp->vid_fmt = VID_SYS_720P_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_576P_50:
        pstDisp->vid_fmt = VID_SYS_576P_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_480P_60:
        pstDisp->vid_fmt = VID_SYS_480P; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL:
        pstDisp->vid_fmt = VID_SYS_PAL; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL_N:
        pstDisp->vid_fmt = VID_SYS_PAL_N; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL_Nc:
        pstDisp->vid_fmt = VID_SYS_PAL_NC; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_60:
    case MT_DRV_DISP_FMT_3840X2160_59_94:
        pstDisp->vid_fmt = VID_SYS_3840X2160_60HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_50:
        pstDisp->vid_fmt = VID_SYS_3840X2160_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_30:
    case MT_DRV_DISP_FMT_3840X2160_29_97:
        pstDisp->vid_fmt = VID_SYS_3840X2160_30HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_25:
        pstDisp->vid_fmt = VID_SYS_3840X2160_25HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_3840X2160_24:
        pstDisp->vid_fmt = VID_SYS_3840X2160_24HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_24:
        pstDisp->vid_fmt = VID_SYS_4096X2160_24HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_25:
        pstDisp->vid_fmt = VID_SYS_4096X2160_25HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_30:
    case MT_DRV_DISP_FMT_4096X2160_29_97:
        pstDisp->vid_fmt = VID_SYS_4096X2160_30HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_50:
        pstDisp->vid_fmt = VID_SYS_4096X2160_50HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_4096X2160_60:
    case MT_DRV_DISP_FMT_4096X2160_59_94:
        pstDisp->vid_fmt = VID_SYS_4096X2160_60HZ; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_NTSC:
        pstDisp->vid_fmt = VID_SYS_NTSC_M; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_NTSC_J:
        pstDisp->vid_fmt = VID_SYS_NTSC_J; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_NTSC_443:
        pstDisp->vid_fmt = VID_SYS_NTSC_443; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_PAL_M:
        pstDisp->vid_fmt = VID_SYS_PAL_M; //enEncFmt2;
        break;
    case MT_DRV_DISP_FMT_SECAM_SIN:
    case MT_DRV_DISP_FMT_SECAM_COS:
        pstDisp->vid_fmt = VID_SYS_SECAM; //enEncFmt2;
        break;
    default:
        pstDisp->vid_fmt = VID_SYS_1080I_50HZ; //enEncFmt2;
        break;
    }
    if (enDisp == MT_DRV_DISPLAY_1)
    {
        if (enStereo != MT_DRV_DISP_STEREO_NONE)
        {
            p_dp->cur_3d_flag = 1;
            p_dp->cur_mode_3d = (int)enStereo;
            p_dp->usr_force_3d = 1;
        }
        else
        {
            p_dp->cur_3d_flag = 0;
            p_dp->usr_force_3d = 0;
            p_dp->cur_mode_3d = (int)MT_DRV_DISP_STEREO_NONE;
        }
    }

    MT_INFO_DISP(" [%s]channel %d,format %d\n", __FUNCTION__, enDisp, pstDisp->vid_fmt);
    if (MT_DRV_DISPLAY_1 == enDisp)
    {
        cur_tvsys_sd = tvsys_hd_to_sd(pstDisp->vid_fmt);

        if(cur_tvsys_sd == VID_SYS_PAL
                && ( (p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt == VID_SYS_PAL_N)
                    || (p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt == VID_SYS_PAL_NC)
                    || (p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt == VID_SYS_SECAM) ))
        {
            cur_tvsys_sd = p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt;
        }

        if(cur_tvsys_sd == VID_SYS_NTSC_J
                &&  ((p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt == VID_SYS_PAL_M)
                    || (p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt == VID_SYS_NTSC_443 )))
        {
            cur_tvsys_sd = p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt;
        }

        p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt = cur_tvsys_sd;
        //old_tvsys_sd = cur_tvsys_sd;
    }

    // If hd work at follow-mode, set display0 sd format
    if ((enDisp == MT_DRV_DISPLAY_1) && pstDisp->bIsMaster && DispFmtIsStandDefinition(enEncFmt))
    {
        DISP_SetFormat(pstDisp->enAttachedDisp, MT_DRV_DISP_STEREO_NONE, enEncFmt);
    }

    p_dp->disp_out_info[enDisp].vid_fmt = pstDisp->vid_fmt;
    p_dp->disp_out_info[enDisp].update_flag |= UPDATE_FLAG_TVSYS;
    pstDisp->update_flag |= UPDATE_FLAG_TVSYS;
    DispProduceDisplayInfo(pstDisp, &pstDisp->stDispInfo); //bug25322
    if (MT_DRV_DISPLAY_0 == enDisp)
    {
        //disable SD_VENC auto clk gate
        dtmp = disp_hal_get_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec));
        dtmp = dtmp & 0xFFFFFEFF;//bit8 set 0
        disp_hal_put_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec), dtmp);    
        
        //old_tvsys_sd = enEncFmt2;
        DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->vdac off ! \n", __FUNCTION__, __LINE__);
        disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);

        reset_delay_cnt = 0;
        update_sys_flag = 1;

    }
    else if (MT_DRV_DISPLAY_1 == enDisp)
    {
        //disable SD_VENC auto clk gate
        dtmp = disp_hal_get_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec));
        dtmp = dtmp & 0xFFFFFEFF;//bit8 set 0
        disp_hal_put_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec), dtmp);    
        
        // old_tvsys_hd = enEncFmt2;
        DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->vdac off ! \n", __FUNCTION__, __LINE__);
        disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);

      //  if(pstDisp->stSetting.enFormat == MT_DRV_DISP_FMT_PAL_M)  //HDMI don't support PAL-M
      //      pstDisp->stSetting.enFormat = MT_DRV_DISP_FMT_NTSC;

        if(MT_DRV_DISP_FMT_1080P_59_94 == pstDisp->stSetting.enFormat 
            || MT_DRV_DISP_FMT_1080i_59_94 == pstDisp->stSetting.enFormat
            || MT_DRV_DISP_FMT_1080P_29_97 == pstDisp->stSetting.enFormat
            || MT_DRV_DISP_FMT_720P_59_94 == pstDisp->stSetting.enFormat
            || MT_DRV_DISP_FMT_3840X2160_29_97 == pstDisp->stSetting.enFormat
            || MT_DRV_DISP_FMT_3840X2160_59_94 == pstDisp->stSetting.enFormat
            || MT_DRV_DISP_FMT_4096X2160_29_97 == pstDisp->stSetting.enFormat
            || MT_DRV_DISP_FMT_4096X2160_59_94 == pstDisp->stSetting.enFormat)
        {
            disp_set_1001_enable(MT_TRUE);
        }
        else
        {
            disp_set_1001_enable(MT_FALSE);
        }

        reset_delay_cnt = 0;
        update_sys_flag = 1;        
    }

    SdVidSys = p_dp->disp_out_info[MT_DRV_DISPLAY_0].vid_fmt;
    HdVidSys = p_dp->disp_out_info[MT_DRV_DISPLAY_1].vid_fmt;

    for(i = 0; i < MAX_WIN_NUM; i++)
    {
        g_stDrvSetting[i].eSdVidSys = SdVidSys;
        g_stDrvSetting[i].eHdVidSys = HdVidSys;
    }
    // s4 if display is enabled, disable it and enable it,
    //    and new format will work.
    if (pstDisp->bEnable)
    {
#if 0 //disp_hal
        if (pstDisp->stSetting.stIntf[MT_DRV_DISP_INTF_HDMI0].bOpen && !DispGetHdmiFunction())
        {
            if (s_pstHDMIFunc->pfnHdmiPreFormat && s_pstHDMIFunc->pfnHdmiSetFormat)
            {
                s_pstHDMIFunc->pfnHdmiPreFormat(MT_UNF_HDMI_ID_0, enEncFmt);
            }
        }

#endif

#ifdef DISP_DEBUGK_TEST_SET_FORMAT_TIME
        mt_drv_sys_gettimestampms(&t0);
#endif

        nRet = DISP_SetEnable(enDisp, MT_FALSE);
        /*
           if interface is RGB mode ;
           when fmt switch to VESA or Custom Fmt :
1:change to VGA interface!
2:HDMI Data tpye use RGB ;TV FMT use YUV!
*/

#ifdef DISP_DEBUGK_TEST_SET_FORMAT_TIME
        mt_drv_sys_gettimestampms(&t1);
#endif

        if (enDisp == MT_DRV_DISPLAY_1)
        {
            // DTS2013060905670 : if time between setdisable and set enable is
            // less than 160ms, the screen on TV linked in CVBS flicker.
            // Increase time interval, flicker disappear.
            // set diable use time 60ms
            //DISP_MSLEEP(400);
            DISP_MSLEEP(500);
        }

        nRet = DISP_SetEnable(enDisp, MT_TRUE);

#ifdef DISP_DEBUGK_TEST_SET_FORMAT_TIME
        mt_drv_sys_gettimestampms(&t2);
        DISP_FATAL("disable use time=%d, enable use time=%d ms\n", t1 - t0, t2 - t1);
#endif

#if 0 // //disp_hal

        if (pstDisp->stSetting.stIntf[MT_DRV_DISP_INTF_HDMI0].bOpen && !DispGetHdmiFunction())
        {
            if (s_pstHDMIFunc->pfnHdmiPreFormat && s_pstHDMIFunc->pfnHdmiSetFormat)
            {
                s_pstHDMIFunc->pfnHdmiSetFormat(MT_UNF_HDMI_ID_0, enEncFmt, enStereo);
            }
        }

#endif
    }

    return MT_SUCCESS;
}

mt_s32 DISP_GetFormat(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_STEREO_MODE_E *penStereo, MT_DRV_DISP_FMT_E *penEncFmt)
{
    DISP_S *pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);
    DispCheckNullPointer(penStereo);
    DispCheckNullPointer(penEncFmt);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);

    // s3 if display is not enabled, set format and return
    *penStereo = (int)pstDisp->stSetting.eDispMode;
    *penEncFmt = pstDisp->stSetting.enFormat;

    return MT_SUCCESS;
}

mt_s32 DISP_SetRightEyeFirst(MT_DRV_DISPLAY_E enDisp, MT_BOOL bEnable)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);

    // s3 check state
    DispGetPointerByID(enDisp, pstDisp);

    // s4 if display is enabled, disable it and enable it,
    //    and new format will work.
    pstDisp->bDispSettingChange = MT_FALSE;
    pstDisp->stSetting.bRightEyeFirst = bEnable;
    pstDisp->bDispSettingChange = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 DISP_SetVirtScreen(MT_DRV_DISPLAY_E enDisp, mt_rect_s virtscreen)
{
    DISP_S *pstDisp = MT_NULL, *pstDisp_attach = MT_NULL;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);

#if 0
    if (pstDisp->bOpen)
    {
        if ((pstDisp->stSetting.stVirtaulScreen.s32Width
                    == (virtscreen.s32Width & 0xFFFFFFFEul))
                && (pstDisp->stSetting.stVirtaulScreen.s32Height
                    == (virtscreen.s32Height & 0xFFFFFFFCul)))
        {

        }
        else
        {
            DISP_ERROR("Disp %d is opened ,can't set VirtScreen.\n",enDisp);
            return MT_ERR_DISP_NOT_SUPPORT;
        }
    }
#endif
    if ((virtscreen.s32Height < MT_DRV_DISP_VIRTSCREEN_MIN) || (virtscreen.s32Height > MT_DRV_DISP_VIRTSCREEN_MAX)
            || (virtscreen.s32Width < MT_DRV_DISP_VIRTSCREEN_MIN) || (virtscreen.s32Width > MT_DRV_DISP_VIRTSCREEN_MAX))
    {
        DISP_ERROR("screen window too small ,must be within [480,3840].\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (pstDisp->bIsMaster)
    {
        pstDisp->bDispSettingChange = MT_FALSE;
        pstDisp->stSetting.stVirtaulScreen.s32Width = virtscreen.s32Width & 0xFFFFFFFEul;
        pstDisp->stSetting.stVirtaulScreen.s32Height = virtscreen.s32Height & 0xFFFFFFFCul;
        pstDisp->bDispSettingChange = MT_TRUE;

        DispGetPointerByID(pstDisp->enAttachedDisp, pstDisp_attach);
        if (pstDisp_attach->bIsSlave)
        {
            pstDisp_attach->bDispSettingChange = MT_FALSE;
            pstDisp_attach->stSetting.stVirtaulScreen.s32Width = virtscreen.s32Width & 0xFFFFFFFEul;
            pstDisp_attach->stSetting.stVirtaulScreen.s32Height = virtscreen.s32Height & 0xFFFFFFFCul;
            pstDisp_attach->bDispSettingChange = MT_TRUE;
        }
    }
    else if (pstDisp->bIsSlave)
    {
        /*non same-source, vscreen can be set.*/
        if (pstDisp->enAttachedDisp >= MT_DRV_DISPLAY_BUTT)
        {
            pstDisp->bDispSettingChange = MT_FALSE;
            pstDisp->stSetting.stVirtaulScreen.s32Width = virtscreen.s32Width & 0xFFFFFFFEul;
            pstDisp->stSetting.stVirtaulScreen.s32Height = virtscreen.s32Height & 0xFFFFFFFCul;
            pstDisp->bDispSettingChange = MT_TRUE;
        }
        else
        {
            DispGetPointerByID(pstDisp->enAttachedDisp, pstDisp_attach);
            if (!pstDisp_attach->bIsMaster)
            {
                pstDisp->bDispSettingChange = MT_FALSE;
                pstDisp->stSetting.stVirtaulScreen.s32Width = virtscreen.s32Width & 0xFFFFFFFEul;
                pstDisp->stSetting.stVirtaulScreen.s32Height = virtscreen.s32Height & 0xFFFFFFFCul;
                pstDisp->bDispSettingChange = MT_TRUE;
            }
            else
            {
                return MT_ERR_DISP_NOT_SUPPORT;
            }
        }
    }
    else
    {
        pstDisp->bDispSettingChange = MT_FALSE;
        pstDisp->stSetting.stVirtaulScreen.s32Width = virtscreen.s32Width & 0xFFFFFFFEul;
        pstDisp->stSetting.stVirtaulScreen.s32Height = virtscreen.s32Height & 0xFFFFFFFCul;
        pstDisp->bDispSettingChange = MT_TRUE;
    }

    return MT_SUCCESS;
}

mt_s32 DISP_GetVirtScreen(MT_DRV_DISPLAY_E enDisp, mt_rect_s *virtscreen)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);

    *virtscreen = pstDisp->stSetting.stVirtaulScreen;

    return MT_SUCCESS;
}

mt_s32 DISP_SetSmallWindow(MT_DRV_DISPLAY_E enDisp, mt_rect_s virtscreen)
{
    DISP_S *pstDisp = MT_NULL;
    rect_vsb_t hd_rect_cur = { 0 };

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);
    disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &(hd_rect_cur.h), &(hd_rect_cur.w));

    DISP_DEBUGK("[%s] enDisp:%d ,x,y,w,h[%d,%d,%d,%d]\n", __FUNCTION__, enDisp,
            virtscreen.s32X, virtscreen.s32Y,virtscreen.s32Width, virtscreen.s32Height);

    if (virtscreen.s32Height > hd_rect_cur.h || virtscreen.s32Width > hd_rect_cur.w || virtscreen.s32X < 0 || virtscreen.s32Y < 0)
        return MT_FAILURE;
    if (MT_DRV_DISPLAY_1 == enDisp)
    {
        smallwindow_rect[0] = virtscreen;
        barrier();
        atomic_set(&smallwindow_flag[0], 1);
    }
    else
    {
        cropwindow_rect[0] = virtscreen;
        barrier();
        atomic_set(&cropwindow_flag[0], 1);
    }
    return MT_SUCCESS;
}

mt_s32 DISP_GetSmallWindow(MT_DRV_DISPLAY_E enDisp, mt_rect_s *virtscreen)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);

    *virtscreen = pstDisp->stSetting.stVirtaulScreen;

    return MT_SUCCESS;
}
#define MT_DRV_DISP_OFFSET_MAX 200
#define MT_DRV_DISP_OFFSET_HORIZONTAL_ALIGN 0xFFFFFFFEul
#define MT_DRV_DISP_OFFSET_VERTICAL_ALIGN 0xFFFFFFFCul

mt_s32 DISP_SetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstScreenOffset)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstScreenOffset);

    if ((pstScreenOffset->u32Bottom > MT_DRV_DISP_OFFSET_MAX) || (pstScreenOffset->u32Left > MT_DRV_DISP_OFFSET_MAX)
            || (pstScreenOffset->u32Right > MT_DRV_DISP_OFFSET_MAX) || (pstScreenOffset->u32Top > MT_DRV_DISP_OFFSET_MAX))
    {
        DISP_ERROR("screen offset must less then 200: %d,%d,%d,%d.\n",
                pstScreenOffset->u32Left, pstScreenOffset->u32Right,
                pstScreenOffset->u32Top, pstScreenOffset->u32Bottom);
        return MT_ERR_DISP_INVALID_PARA;
    }

    /*
       if ((screenoffset.u32Bottom & 0x3)
       ||(screenoffset.u32Left & 0x3)
       ||(screenoffset.u32Right & 0x3)
       ||(screenoffset.u32Top   & 0x3))
       {
       DISP_ERROR("screen offset not aligned:%d,%d,%d,%d.\n", screenoffset.u32Bottom,
       screenoffset.u32Left,
       screenoffset.u32Right,
       screenoffset.u32Top);
       return MT_ERR_DISP_INVALID_PARA;
       }
       */
    pstDisp->bDispSettingChange = MT_FALSE;
    pstDisp->stSetting.stOffsetInfo.u32Left = pstScreenOffset->u32Left & MT_DRV_DISP_OFFSET_HORIZONTAL_ALIGN;
    pstDisp->stSetting.stOffsetInfo.u32Right = pstScreenOffset->u32Right & MT_DRV_DISP_OFFSET_HORIZONTAL_ALIGN;
    pstDisp->stSetting.stOffsetInfo.u32Top = pstScreenOffset->u32Top & MT_DRV_DISP_OFFSET_VERTICAL_ALIGN;
    pstDisp->stSetting.stOffsetInfo.u32Bottom = pstScreenOffset->u32Bottom & MT_DRV_DISP_OFFSET_VERTICAL_ALIGN;
    pstDisp->bDispSettingChange = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 DISP_GetScreenOffset(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_OFFSET_S *pstScreenOffset)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstScreenOffset);

    *pstScreenOffset = pstDisp->stSetting.stOffsetInfo;
    return MT_SUCCESS;
}

bool b_set_ar_to_hdmi = MT_TRUE;
EXPORT_SYMBOL(b_set_ar_to_hdmi);

int __init uboot_set_ar_to_hdmi(char *str)
{
    b_set_ar_to_hdmi = MT_TRUE;
    return 1;
}

__setup("ar_set", uboot_set_ar_to_hdmi);

//set aspect ratio
mt_s32 DISP_SetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 u32Ratio_h, mt_u32 u32Ratio_v)
{
    DISP_S *pstDisp;
#if 0
    HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;
    HDMI_AVMUTE_S avmute_t;
#endif
    disp_priv_t *p_dp = &s_stDisplayPriv;

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);

    // s3 check state
    DispGetPointerByID(enDisp, pstDisp);

    // s4 if display is enabled, disable it and enable it,
    //    and new format will work.
    pstDisp->bDispSettingChange = MT_FALSE;

    if (u32Ratio_h && u32Ratio_v)
    {
        pstDisp->stSetting.bCustomRatio = MT_TRUE;
        pstDisp->stSetting.u32CustomRatioWidth = u32Ratio_h;
        pstDisp->stSetting.u32CustomRatioHeight = u32Ratio_v;
        if ((u32Ratio_h == 4) && (u32Ratio_v == 3))
        {
            out_ar = MT_DRV_AR_4TO3;
        }
        else if ((u32Ratio_h == 16) && (u32Ratio_v == 9))
        {
            out_ar = MT_DRV_AR_16TO9;
        }
        else if ((u32Ratio_h == 1) && (u32Ratio_v == 1))
        {
            out_ar = MT_DRV_AR_SQUARE;
        }
    }
    else
    {
        pstDisp->stSetting.bCustomRatio = MT_FALSE;
        out_ar = MT_DRV_AR_AUTO;
    }

    if (b_set_ar_to_hdmi == MT_TRUE)
    {
        if ((out_ar == MT_DRV_AR_16TO9) && (p_dp->hdmi_vcfg.output_v_cfg.shape != HDMI_SHAPE_16X9))
        {
            p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
#if 0
            mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);
            avmute_t.enHdmi = MT_UNF_HDMI_ID_0;
            avmute_t.AVMuteEnable = 1;
            if (pstHDMIFunc && pstHDMIFunc->pfnHdmiAvMute)
            {
                pstHDMIFunc->pfnHdmiAvMute(0, (mt_u32 *)&avmute_t);
                mdelay(100);
                p_dp->hdmi_vcfg_flag = MT_TRUE;
            }

            if (p_dp->hdmi_vcfg_flag && p_dp->tasklet_flag)
            {
                schedule_work(&p_dp->work_hdmi_cb);
                //p_dp->hdmi_vcfg_flag = MT_FALSE;
            }
#endif
            disp_hdmi_video_config(p_dp);
        }
        else if (out_ar == MT_DRV_AR_4TO3 && p_dp->hdmi_vcfg.output_v_cfg.shape != HDMI_SHAPE_4X3)
        {
            p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_4X3;
#if 0
            mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);
            avmute_t.enHdmi = MT_UNF_HDMI_ID_0;
            avmute_t.AVMuteEnable = 1;
            if (pstHDMIFunc && pstHDMIFunc->pfnHdmiAvMute)
            {
                pstHDMIFunc->pfnHdmiAvMute(0, (mt_u32 *)&avmute_t);
                mdelay(100);
                p_dp->hdmi_vcfg_flag = MT_TRUE;
            }

            if (p_dp->hdmi_vcfg_flag && p_dp->tasklet_flag)
            {
                schedule_work(&p_dp->work_hdmi_cb);
                //p_dp->hdmi_vcfg_flag = MT_FALSE;
            }
#endif
            disp_hdmi_video_config(p_dp);
        }
    }

    pstDisp->bDispSettingChange = MT_TRUE;
    return MT_SUCCESS;
}

mt_s32 DISP_GetAspectRatio(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Ratio_h, mt_u32 *pu32Ratio_v)
{
    DISP_S *pstDisp;

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);
    DispCheckNullPointer(pu32Ratio_h);
    DispCheckNullPointer(pu32Ratio_v);

    // s3 check state
    DispGetPointerByID(enDisp, pstDisp);

    // s4 if display is enabled, disable it and enable it,
    //    and new format will work.
    if (pstDisp->stSetting.bCustomRatio)
    {
        *pu32Ratio_h = pstDisp->stSetting.u32CustomRatioWidth;
        *pu32Ratio_v = pstDisp->stSetting.u32CustomRatioHeight;
    }
    else
    {
        *pu32Ratio_h = 0;
        *pu32Ratio_v = 0;
    }

    return MT_SUCCESS;
}

mt_s32 DISPCheckCustomTiming(MT_DRV_DISP_TIMING_S *pstTiming)
{
    if ((pstTiming->bIDV != MT_TRUE) && (pstTiming->bIDV != MT_FALSE))
    {
        DISP_ERROR("para pstTiming->IDV is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstTiming->bIHS != MT_TRUE) && (pstTiming->bIHS != MT_FALSE))
    {
        DISP_ERROR("para pstTiming->IHS is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstTiming->bIVS != MT_TRUE) && (pstTiming->bIVS != MT_FALSE))
    {
        DISP_ERROR("para pstTiming->IVS is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstTiming->bClkReversal != MT_TRUE) && (pstTiming->bClkReversal != MT_FALSE))
    {
        DISP_ERROR("para pstTiming->ClockReversal is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if (pstTiming->eDataFmt >= MT_DRV_DISP_INTF_DATA_FMT_BUTT)
    {
        DISP_ERROR("para pstTiming->ItfFormat is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((pstTiming->bDitherEnable != MT_TRUE) && (pstTiming->bDitherEnable != MT_FALSE))
    {
        DISP_ERROR("para pstTiming->DitherEnable is invalid.\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    //printk("custom  ------------H,W(%d,%d)",pstTiming->u32AspectRatioH,pstTiming->u32AspectRatioW);
    if (((pstTiming->u32AspectRatioW) && (pstTiming->u32AspectRatioH)) && ((pstTiming->u32AspectRatioW >= (pstTiming->u32AspectRatioH * 16))
                || (pstTiming->u32AspectRatioH >= (pstTiming->u32AspectRatioW * 16))))
    {
        DISP_ERROR(" DISP Set Custom Timing AspectRatio err!\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    if ((!pstTiming->u32ClkPara0) || (!pstTiming->u32ClkPara1))
    {
        if (pstTiming->u32PixFreq)
        {
            if ((20000 > pstTiming->u32PixFreq) || (600000 < pstTiming->u32PixFreq))
            {
                DISP_ERROR("u32PixClk (%d)out of rang  (20000~600000)!!\n", pstTiming->u32PixFreq);
                return MT_ERR_DISP_INVALID_PARA;
            }
        }
        else
        {
            if ((pstTiming->u32VertFreq <= 20000) || (pstTiming->u32VertFreq >= 120000))
            {
                DISP_ERROR(" para pstTiming->u32VertFreq err!\n");
                return MT_ERR_DISP_INVALID_PARA;
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 DISPCheckCustomTimingIsSet(MT_DRV_DISP_TIMING_S *pstTiming, MT_DRV_DISP_TIMING_S *pstSetting)
{
    if ((pstSetting->u32HBB != pstTiming->u32HBB) || (pstSetting->u32HACT != pstTiming->u32HACT) ||
            (pstSetting->u32HFB != pstTiming->u32HFB) || (pstSetting->u32HPW != pstTiming->u32HPW) ||
            (pstSetting->u32VACT != pstTiming->u32VACT) || (pstSetting->u32VBB != pstTiming->u32VBB) ||
            (pstSetting->u32VFB != pstTiming->u32VFB) || (pstSetting->u32VPW != pstTiming->u32VPW) ||
            (pstSetting->bIDV != pstTiming->bIDV) || (pstSetting->bIHS != pstTiming->bIHS) ||
            (pstSetting->bIVS != pstTiming->bIVS) || (pstSetting->bInterlace != pstTiming->bInterlace) ||
            (pstSetting->u32VertFreq != pstTiming->u32VertFreq))
    {
        return MT_ERR_DISP_INVALID_PARA;
    }

    return MT_SUCCESS;
}

mt_s32 DISP_SetCustomTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming)
{
    DISP_S *pstDisp;
    mt_s32 nRet;
    MT_DRV_DISP_STEREO_MODE_E enStereo;
    MT_DRV_DISP_FMT_E enEncFmt;

    DispCheckDeviceState();

    DispCheckID(enDisp);
    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstTiming);

    nRet = DISPCheckCustomTiming(pstTiming);
    if (MT_SUCCESS != nRet)
    {
        DISP_ERROR("Check Custom Timing Para err! (%d)\n", nRet);
        return nRet;
    }

    if (MT_DRV_DISP_FMT_CUSTOM == pstDisp->stSetting.enFormat)
    {
        nRet = DISPCheckCustomTimingIsSet(&pstDisp->stSetting.stCustomTimg, pstTiming);
        if (MT_SUCCESS == nRet)
        {
            DISP_ERROR("set same custom timing!\n");
            return nRet;
        }
    }

    enStereo = MT_DRV_DISP_STEREO_NONE;
    enEncFmt = MT_DRV_DISP_FMT_CUSTOM;

    if ((!pstTiming->u32AspectRatioW) || (!pstTiming->u32AspectRatioH))
    {
        pstTiming->u32AspectRatioW = pstTiming->u32HACT;
        pstTiming->u32AspectRatioH = pstTiming->u32VACT;
    }

    pstDisp->stSetting.eDispMode = (int)enStereo;
    pstDisp->stSetting.enFormat = enEncFmt;
    pstDisp->stSetting.stCustomTimg = *pstTiming;

#if 0 //other_module
    if (pstDisp->bEnable)
    {
        // s4 if display is enabled, disable it and enable it,
        //    and new format will work.
        if (pstDisp->stSetting.stIntf[MT_DRV_DISP_INTF_HDMI0].bOpen && !DispGetHdmiFunction())
        {
            if (s_pstHDMIFunc->pfnHdmiPreFormat && s_pstHDMIFunc->pfnHdmiSetFormat)
            {
                s_pstHDMIFunc->pfnHdmiPreFormat(MT_UNF_HDMI_ID_0, enEncFmt);
            }
        }
    }
#endif

    nRet = DISP_SetEnable(enDisp, MT_FALSE);
    if (MT_SUCCESS != nRet)
    {
        DISP_ERROR("set custom timing disenable disp%d err \n", enDisp);
        goto TIMING_EXIT0;
    }

    nRet = DISP_SetEnable(enDisp, MT_TRUE);
    if (MT_SUCCESS != nRet)
    {
        DISP_ERROR("set custom timing enable disp%d err \n", enDisp);
        goto TIMING_EXIT0;
    }

#if 0 //othrer_module
    if (pstDisp->stSetting.stIntf[MT_DRV_DISP_INTF_HDMI0].bOpen && !DispGetHdmiFunction())
    {
        if (s_pstHDMIFunc->pfnHdmiPreFormat && s_pstHDMIFunc->pfnHdmiSetFormat)
        {
            s_pstHDMIFunc->pfnHdmiSetFormat(MT_UNF_HDMI_ID_0, enEncFmt, enStereo);
        }
    }
#endif
    return nRet;

TIMING_EXIT0:
    pstDisp->stSetting.enFormat = MT_DRV_DISP_FMT_BUTT;
    return nRet;
}
mt_s32 DISP_GetCustomTiming(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_TIMING_S *pstTiming)
{
    DISP_S *pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);
    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstTiming);

    *pstTiming = pstDisp->stSetting.stCustomTimg;

    return MT_SUCCESS;
}

mt_s32 DISP_SetBGColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBGColor)
{
    DISP_S *pstDisp;
    mt_s32 Ret = MT_SUCCESS;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstBGColor);

    //todo check color setting
    /*
       DISP_PRINT("xxxxxxDISP_SetBGColor R=%d,G=%d,B=%d\n",
       pstBGColor->u8Red,
       pstBGColor->u8Green, pstBGColor->u8Blue);
       */
    pstDisp->stSetting.stBgColor = *pstBGColor;

    if (pstDisp->bEnable)
    {
        Ret = DispSetColor(pstDisp);
        if (Ret != MT_SUCCESS)
        {
            goto __SET_BGCOLOR__;
        }
    }

__SET_BGCOLOR__:
    if (pstDisp->bIsMaster)
    {
        //DISP_PRINT("DISP_SetColor, attech  disp ID = %d\n", pstDisp->enAttachedDisp);
        Ret = DISP_SetBGColor(pstDisp->enAttachedDisp, pstBGColor);
        if (Ret != MT_SUCCESS)
        {
            goto __SET_BGCOLOR_EXIT_;
        }
    }

__SET_BGCOLOR_EXIT_:
    return Ret;
}
mt_s32 DISP_GetBGColor(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_COLOR_S *pstBGColor)
{
    DISP_S *pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstBGColor);

    //todo check color setting

    *pstBGColor = pstDisp->stSetting.stBgColor;

    return MT_SUCCESS;
}

mt_s32 DISP_SetLayerZorder(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_ZORDER_ABS_E enZFlag)
{
    DISP_DEBUGK("%s %d enZFlag:%d\n", __FUNCTION__, __LINE__, enZFlag);
    drv_reg_4k_disp_set_osdm_cmd_osd_sub_mux_sel(enZFlag);

    return MT_SUCCESS;
}

mt_s32 DISP_GetLayerZorder(MT_DRV_DISPLAY_E enDisp, mt_u32 *pu32Zorder)
{
    MT_DRV_DISP_ZORDER_E enZFlag;
    enZFlag = drv_reg_4k_disp_get_osdm_cmd_osd_sub_mux_sel();

    *pu32Zorder = enZFlag;
    return MT_SUCCESS;
}

mt_s32 DISP_TestMacrovisionSupport(MT_DRV_DISPLAY_E enDisp, MT_BOOL *pbSupport)
{

    return MT_SUCCESS;
}

#if 1 //def MT_DISP_BUILD_FULL
//snapshot
mt_s32 DISP_AcquireSnapshot(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S *pstSnapShotFrame, mt_handle *snapshotHandleOut)
{
    DISP_CAST_S *pstCast;
    mt_s32 Ret;
    mt_s32 i = 0;
    DISP_S *pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckID(enDisp);

    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstSnapShotFrame);
    if (!pstDisp->bOpen || !pstDisp->bEnable)
    {
        DISP_ERROR("Disp not open, cannot snapshot!\n");
        return MT_ERR_DISP_NOT_OPEN;
    }

    if (!pstDisp->pstIntfOpt->PF_TestChnSupportCast(pstDisp->enDisp))
    {
        DISP_ERROR("Disp %d not support cast!\n", (mt_s32)enDisp);
        return MT_ERR_DISP_INVALID_OPT;
    }

    /* reget pstCast for user may call cast_destroy. */
    pstCast = (DISP_CAST_S *)pstDisp->Cast_ptr;

    if (pstCast)
    {
        /* notify cast to shedule wbc. */
        pstCast->bScheduleWbc = MT_TRUE;
        ;
        pstCast->u32Ref++;
retry:

        if (pstCast->bEnable)
        {
            msleep(40);

            if (!pstCast->bScheduleWbcStatus && ++i < 3)
            {
                goto retry;
            }

            if (!pstCast->bScheduleWbcStatus)
            {
                Ret = MT_ERR_DISP_TIMEOUT;
                goto out;
            }
        }
    }

    Ret = DISP_Acquire_Snapshot(enDisp, snapshotHandleOut, pstSnapShotFrame);

out:
    pstCast = (DISP_CAST_S *)pstDisp->Cast_ptr;
    if (pstCast)
    {
        pstCast->bScheduleWbc = MT_FALSE;
        pstCast->bScheduleWbcStatus = MT_FALSE;
        /* In order to decrease pstCast->u32Ref. Maybe do some destroy actually */
        DISP_DestroyCast(pstDisp->hCast);
    }

    return Ret;
}
mt_s32 DISP_ReleaseSnapshot(MT_DRV_DISPLAY_E enDisp, MT_DRV_VIDEO_FRAME_S *pstSnapShotFrame, mt_handle snapshotHandle)
{
    mt_s32 Ret;
    DISP_S *pstDisp;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispGetPointerByID(enDisp, pstDisp);
    DispCheckNullPointer(pstSnapShotFrame);
    if (!pstDisp->bOpen || !pstDisp->bEnable)
    {
        DISP_ERROR("Disp not open, cannot snapshot!\n");
        return MT_ERR_DISP_NOT_OPEN;
    }

    if (!pstDisp->pstIntfOpt->PF_TestChnSupportCast(pstDisp->enDisp))
    {
        DISP_ERROR("Disp %d not support cast!\n", (mt_s32)enDisp);
        return MT_ERR_DISP_INVALID_OPT;
    }

    Ret = DISP_Release_Snapshot(enDisp, snapshotHandle, pstSnapShotFrame);
    return Ret;
}

mt_s32 DISP_DestroySnapshot(mt_handle hSnapshot)
{
    mt_s32 nRet;

    DispCheckDeviceState();

    if (!hSnapshot)
        return MT_ERR_DISP_NULL_PTR;

    nRet = DISP_SnapshotDestroy(hSnapshot);

    return nRet;
}

//miracast
mt_s32 DISP_CreateCast(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CAST_CFG_S *pstCfg, mt_handle *phCast)
{
    DISP_S *pstDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = MT_NULL;
    MT_DISP_DISPLAY_INFO_S pstInfo;

    /* check input parameters*/
    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispCheckNullPointer(pstCfg);
    DispCheckNullPointer(phCast);

    /*check open status.*/
    DispShouldBeOpened(enDisp);

    /*check cast support .*/
    DispGetPointerByID(enDisp, pstDisp);

#if 0
    if (!pstDisp->pstIntfOpt->PF_TestChnSupportCast(pstDisp->enDisp))
    {
        DISP_ERROR("Disp %d not support cast!\n", (mt_s32)enDisp);
        return MT_ERR_DISP_INVALID_OPT;
    }
#endif

    (mt_void) DISP_GetDisplayInfo(enDisp, &pstInfo);

    nRet = DISP_CastCreate(enDisp, &pstInfo, pstCfg, &cast_ptr);

    if (!nRet)
    {
        /*hcast is a user handle containing of mod id info and display channel.
         *for there is 1:1 relationship bettween display channel and cast handle.*/
        pstDisp->hCast = (MT_ID_DISP << 16) | enDisp;
        *phCast = pstDisp->hCast;

        /*this is a cast instance, we can get it from DISP_S struct definition.*/
        pstDisp->Cast_ptr = cast_ptr;
    }

    DISP_WARN("DISP_CreateCast  pstDisp->hCast = 0x%lx, cast_ptr:0x%lx\n", (mt_u32)pstDisp->hCast, pstDisp->Cast_ptr);

    return nRet;
}

mt_s32 DISP_DestroyCast(mt_handle hCast)
{
    MT_DRV_DISPLAY_E enDisp;
    DISP_S *pstDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;
    DISP_CAST_S *pstCast;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);

    // s2 TODO: search display
    enDisp = hCast & 0xff;
    nRet = DispSearchCastHandle(&cast_ptr, enDisp);

    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    // s3 check whether display opened
    DispShouldBeOpened(enDisp);
    // s4 get pointer
    DispGetPointerByID(enDisp, pstDisp);

    pstCast = (DISP_CAST_S *)cast_ptr;

    if (--pstCast->u32Ref > 0)
    {
        return MT_SUCCESS;
    }

    // s5 destroy cast
    nRet = DISP_CastDestroy(cast_ptr);

    pstDisp->hCast = MT_NULL;
    pstDisp->Cast_ptr = MT_NULL;

    return nRet;
}

mt_s32 DISP_SetCastEnable(mt_handle hCast, MT_BOOL bEnable)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);
    DISP_WARN("DISP_SetCastEnable  hCast = 0x%lx\n", hCast);

    // s2 TODO: search display
    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    // s3 check whether display opened
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_CastSetEnable(cast_ptr, bEnable);
    return nRet;
}

mt_s32 DISP_GetCastEnable(mt_handle hCast, MT_BOOL *pbEnable)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);
    DispCheckNullPointer(pbEnable);

    // s2 TODO: search display
    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    // s3 check whether display opened
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_CastGetEnable(cast_ptr, pbEnable);
    return nRet;
}

mt_s32 DISP_AcquireCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);

    // s2 TODO: search display
    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    // s3 check whether display opened
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_CastAcquireFrame(cast_ptr, pstCastFrame);
    return nRet;
}

mt_s32 DISP_ReleaseCastFrame(mt_handle hCast, MT_DRV_VIDEO_FRAME_S *pstCastFrame)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);
    DispCheckNullPointer(pstCastFrame);

    // s2 TODO: search display
    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    // s3 check whether display opened
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_CastReleaseFrame(cast_ptr, pstCastFrame);
    return nRet;
}

mt_s32 DISP_External_Attach(mt_handle hCast, mt_handle hsink)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);

    // s2 TODO: search display
    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    // s3 check whether display opened
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_Cast_AttachSink(cast_ptr, hsink);
    return nRet;
}

mt_s32 DISP_External_DeAttach(mt_handle hCast, mt_handle hsink)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    // s1 check input parameters
    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);

    // s2 TODO: search display
    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    //s3 check whether display opened
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_Cast_DeAttachSink(cast_ptr, hsink);
    return nRet;
}

/*currently, this function only called by venc to change cast resolution. */
mt_s32 DRV_DISP_SetCastAttr(mt_handle hCast, MT_DRV_DISP_Cast_Attr_S *castAttr)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);

    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_Cast_SetAttr(cast_ptr, castAttr);
    return nRet;
}

mt_s32 DRV_DISP_GetCastAttr(mt_handle hCast, MT_DRV_DISP_Cast_Attr_S *castAttr)
{
    MT_DRV_DISPLAY_E enDisp;
    mt_s32 nRet;
    mt_handle cast_ptr = 0;

    DispCheckDeviceState();
    DispCheckCastHandleValid(hCast);

    enDisp = (MT_DRV_DISPLAY_E)(hCast & 0xff);
    DispShouldBeOpened(enDisp);

    nRet = DispSearchCastHandle(&cast_ptr, enDisp);
    if (nRet)
    {
        DISP_ERROR("DISP cast not exist!\n");
        return MT_ERR_DISP_NOT_EXIST;
    }

    nRet = DISP_Cast_GetAttr(cast_ptr, castAttr);
    return nRet;
}
mt_s32 DispGetCastHandle(MT_DRV_DISPLAY_E enDisp, mt_handle *phCast, mt_handle *phCast_ptr)
{
    DISP_S *pstDisp = MT_NULL;
    DispGetPointerByID(enDisp, pstDisp);

    *phCast = pstDisp->hCast;
    *phCast_ptr = pstDisp->Cast_ptr;
    return MT_SUCCESS;
}
#endif

mt_s32 DISP_SetMacrovisionCustomer(MT_DRV_DISPLAY_E enDisp, mt_void *pData)
{

    return MT_SUCCESS;
}

mt_s32 DISP_SetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E enMode)
{
    DISP_S *pstDisp = MT_NULL;
    mt_u32 reg_val = 0;
    mt_u32 vbi_control_reg_val = 0;
    disp_priv_t *p_dp = MT_NULL;
    disp_sys_t tv_sys_sd;

    p_dp = &s_stDisplayPriv;
    tv_sys_sd = p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt;
    DispGetPointerByID(enDisp, pstDisp);
    pstDisp->enMacrovisionMode = enMode;
    DISP_DEBUGK("%s line:%d macv_type:%d\n", __FUNCTION__, __LINE__, enMode);

    //reset common config
    reg_symphony_sd_encoder_set_cfig5(0x12e0000);  //0x30

    switch(enMode)
    {
    case MT_DRV_DISP_MACROVISION_TYPE0:

        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0);

        //set to default value
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP, 0x1a1d);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, 0x2211);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, 0x4a522a25);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0xf2211);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, 0x2302251);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, 0x2302127);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, 0x17028400);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, 0x1c1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, 0x3d1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, 0x1424);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, 0x3fe07f8);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, 0x1540000);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, 0xfe0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, 0x7e0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, 0x6060);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, 0x2402448b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, 0x3600);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, 0xec0d745);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, 0x4074);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, 0x541553ff);
        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M) || (tv_sys_sd == VID_SYS_NTSC_443))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N, 0x1420114);	  //adjust black level
            }
            if(tv_sys_sd == VID_SYS_PAL_M)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x750101); //0x24
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P, 0x1400110);	  //adjust black level //0x0c
            }

            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
            reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
            reg_val = reg_val & 0xfffffcff;  //refer to IEC-61880, bit8 stand for Bit No.9, bit9 stand for Bit No.10
            reg_val |= 0;
            reg_val = add_crc6(reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
        }
        else  //PAL
        {
            if(tv_sys_sd == VID_SYS_PAL_N)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x890128); //0x24
            }

        }
        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
        reg_val |= 0x10;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6, reg_val);

        break;
    case MT_DRV_DISP_MACROVISION_TYPE1:

        vbi_control_reg_val = (VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL,vbi_control_reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
        reg_val |= 0x1;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
        reg_val &= 0xFFFFFFEF;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6, reg_val);

        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136101);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x00132211);
            if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M) || (tv_sys_sd == VID_SYS_NTSC_443))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N, 0x1060114);	  //adjust black level
            }
            if(tv_sys_sd == VID_SYS_PAL_M)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x750101); //0x24
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P, 0x1400110);    //adjust black level //0x0c
            }

            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
            reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
            reg_val = reg_val & 0xfffffcff;  //refer to IEC-61880, bit8 stand for Bit No.9, bit9 stand for Bit No.10
            reg_val |= 0x2 << 8;
            reg_val = add_crc6(reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
        }
        else  //PAL
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136111);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0xf2211);

            if(tv_sys_sd == VID_SYS_SECAM)
            {
                reg_symphony_sd_encoder_set_cfig5(0x12e000c);  //0x30
            }
            else if(tv_sys_sd == VID_SYS_PAL_N)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x890128); //0x24
            }                
        }

        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP, 0x1a1d);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, 0x2211);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, 0x4a522a25);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, 0x2302251);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, 0x2602127);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, 0x1802b000);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, 0x1c1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, 0x3d1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, 0x1424);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, 0x3fe07f8);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, 0x1540000);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, 0xfe0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, 0x7e0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, 0x6060);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, 0x26023c8a);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, 0x3600);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, 0xff0d745);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, 0x4074);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, 0x581553ff);

        break;
    case MT_DRV_DISP_MACROVISION_TYPE2:

        vbi_control_reg_val = (VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL,vbi_control_reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
        reg_val |= 0x1;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
        reg_val &= 0xFFFFFFEF;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6, reg_val);

        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            if((tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136101);
            }
            else
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136111);
            }
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x00102211);
            if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M) || (tv_sys_sd == VID_SYS_NTSC_443))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N, 0x1060114);    //adjust black level
            }
            if(tv_sys_sd == VID_SYS_PAL_M)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x750101); //0x24
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P, 0x1400110);    //adjust black level //0x0c
            }

            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
            reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
            reg_val = reg_val & 0xfffffcff;  //refer to IEC-61880, bit8 stand for Bit No.9, bit9 stand for Bit No.10
            reg_val |= 0x1 << 8;
            reg_val = add_crc6(reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
        }
        else
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136111);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0xf2211);
            if(tv_sys_sd == VID_SYS_PAL_N)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x890128); //0x24
            }
        }

        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP, 0x1a1d);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, 0x2211);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, 0x4a522a25);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, 0x2302251);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, 0x2602127);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, 0x1802b000);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, 0x1c1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, 0x3d1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, 0x1424);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, 0x3fe07f8);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, 0x1540000);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, 0xfe0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, 0x7e0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, 0x6060);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, 0x26023c8a);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, 0x3600);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, 0xff0d745);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, 0x4074);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, 0x581553ff);


        break;
    case MT_DRV_DISP_MACROVISION_TYPE3:

        vbi_control_reg_val = (VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL,vbi_control_reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
        reg_val |= 0x1;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
        reg_val &= 0xFFFFFFEF;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6, reg_val);

        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            if((tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136101);
            }
            else
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136111);
            }
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x00102215);
            if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M) || (tv_sys_sd == VID_SYS_NTSC_443))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N, 0x1060114);	  //adjust black level
            }
            if(tv_sys_sd == VID_SYS_PAL_M)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x750101); //0x24
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P, 0x1400110);    //adjust black level //0x0c
            }

            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
            reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
            reg_val = reg_val & 0xfffffcff;  //refer to IEC-61880, bit8 stand for Bit No.9, bit9 stand for Bit No.10
            reg_val |= 0x3 << 8;
            reg_val = add_crc6(reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
        }
        else
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136111);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0xf2215);
            if(tv_sys_sd == VID_SYS_PAL_N)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x890128); //0x24
            }                
        }

        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP, 0x1a17);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, 0x2215);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, 0x4a522a21);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, 0x2302255);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, 0x2602125);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, 0x1802b002);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, 0x1c1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, 0x3d1b);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, 0x1424);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, 0x3fe07f8);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, 0x1540000);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, 0xfe0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, 0x7e0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, 0x6050);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, 0x26023c8a);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, 0x3600);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, 0xff0d745);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, 0x4074);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, 0x581553ff);

        break;
    case MT_DRV_DISP_MACROVISION_CUSTOMER_01:
        vbi_control_reg_val = (VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL,vbi_control_reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
        reg_val |= 0x1;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
        reg_val &= 0xFFFFFFEF;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6, reg_val);

        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136111);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x00102215);
            if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M) || (tv_sys_sd == VID_SYS_NTSC_443))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N, 0x1060114);    //adjust black level
            }
            if(tv_sys_sd == VID_SYS_PAL_M)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x750101); //0x24
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P, 0x1400110);    //adjust black level //0x0c
            }
        }
        else
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136131);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x152215);
            if(tv_sys_sd == VID_SYS_PAL_N)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x890128); //0x24
            }
        }
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP, 0x1a17);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, 0x2215);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, 0x4a522a21);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, 0x2302255);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, 0x2602125);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, 0x1802b003);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, 0x1c19);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, 0x3d1c);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, 0x1423);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, 0x3fe0ff8);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, 0x1547e07);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, 0xfe0f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, 0x7e0e);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, 0x6091);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, 0x26023cd5);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, 0x360f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, 0xff0d743);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, 0x5272);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, 0x581553c3);
        break;
    case MT_DRV_DISP_MACROVISION_CUSTOMER_02:
        vbi_control_reg_val = (VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL,vbi_control_reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
        reg_val |= 0x1;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, reg_val);

        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6);
        reg_val &= 0xFFFFFFEF;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG6, reg_val);

        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136011);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x00132236);
            if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M) || (tv_sys_sd == VID_SYS_NTSC_443))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_N, 0x1060114);    //adjust black level
            }
            if(tv_sys_sd == VID_SYS_PAL_M)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x750101); //0x24
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_BLANK_P, 0x1400110);    //adjust black level //0x0c
            }
        }
        else
        {
            reg_symphony_sd_encoder_set_cfig5(0x12e0004);  //0x30
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, 0x23136331);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N4_CS_SPC_1_2_BOT, 0x152236);
            if(tv_sys_sd == VID_SYS_PAL_N)
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_CFIG2, 0x890128); //0x24
            }
        }
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N1_CS_FST_LIN_TOP, 0x1a2f);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N2_CS_SPC_1_2_TOP, 0x222a);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N3_CS_FST_LN_BOT, 0x4a522a1a);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N5_CS_SPC_ELSE, 0x2302252);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N6_CS_NUM_IN_FIELD, 0x2602124);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N7_LN_NUM_IN_CS, 0x1802b033);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N8_PS_DURA, 0x2324);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N9_FST_PS_START, 0x1225);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N10_PS_SPC, 0x2b1d);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N11_PS_AGC_LN_CHS, 0x78c636b8);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N12_PS_AGC_FMT_CHS, 0x1f436dcf);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N13_PS_AGC_IVK_A, 0x5323);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N14_PS_AGC_IVK_B, 0xa313);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N15_BP_LN_CHS, 0xf070);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N17_CS_ZONE1, 0x26023c77);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N18_CS_ZONE2, 0x36c3);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N19_CS_ZONE3, 0xff0d73a);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N20_CS_PHS_MDF, 0x5225);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_N21_CS_PH_MDF_LN, 0x583853c0);
        break;
    default:
        reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG);
        reg_val &= ~0x1;
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG, reg_val);
        DISP_DEBUGK("%s %d disable macrovision!!\n", __FUNCTION__, __LINE__);
        break;
    }

    return MT_SUCCESS;
}

mt_s32 DISP_GetMacrovision(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_MACROVISION_E *penMode)
{
    DISP_S *pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);
    DispCheckNullPointer(penMode);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);

    // get mode and return
    *penMode = pstDisp->enMacrovisionMode;

    return MT_SUCCESS;
}

//cgms-a
mt_s32 DISP_SetCGMS_A(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CGMSA_CFG_S *pstCfg)
{
    mt_u32 dtmp =0;
    mt_u32 reg_val =0;
    disp_priv_t *p_dp = &s_stDisplayPriv;
    disp_sys_t tv_sys_sd = p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt;

    if(pstCfg->bEnable == MT_TRUE)
    {
        switch(pstCfg->enMode)
        {
        case MT_DRV_DISP_CGMSA_COPY_FREELY:
            if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                    || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
                reg_val = reg_val & 0xffffff3f;  //refer to IEC-61880, bit6 stand for Bit No.7, bit7 stand for Bit No.8
                reg_val |= 0;
                reg_val = add_crc6(reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            }
            else  // PAL
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x10000001);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG);
                reg_val &= 0xffffffcf;
                reg_val |= 0;
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG, reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x10000037);
            }
            break;
        case MT_DRV_DISP_CGMSA_COPY_NO_MORE:
            if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                    || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
                reg_val = reg_val & 0xffffff3f;  //refer to IEC-61880, bit6 stand for Bit No.7, bit7 stand for Bit No.8
                reg_val |= 0x80;
                reg_val = add_crc6(reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            }
            else  // PAL
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x10000001);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG);
                reg_val &= 0xffffffcf;
                reg_val |= 0x20;
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG, reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x10000037);
            }
            break;
        case MT_DRV_DISP_CGMSA_COPY_ONCE:
            if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                    || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
                reg_val = reg_val & 0xffffff3f;  //refer to IEC-61880, bit6 stand for Bit No.7, bit7 stand for Bit No.8
                reg_val |= 0x40;
                reg_val = add_crc6(reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            }
            else  // PAL
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x10000001);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG);
                reg_val &= 0xffffffcf;
                reg_val |= 0x10;
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG, reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x10000037);
            }
            break;
        case MT_DRV_DISP_CGMSA_COPY_NEVER:
            if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                    || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
                reg_val = reg_val & 0xffffff3f;  //refer to IEC-61880, bit6 stand for Bit No.7, bit7 stand for Bit No.8
                reg_val |= 0xc0;
                reg_val = add_crc6(reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            }
            else  // PAL
            {
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x10000001);
                reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG);
                reg_val &= 0xffffffcf;
                reg_val |= 0x30;
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG, reg_val);
                disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x10000037);
            }
            break;
        default:
            break;
        }

        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            dtmp = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SDVENC_SET);
            dtmp &= ~(1 << 4);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SDVENC_SET, dtmp);  //CGMSA output policy issue
        }
        else
        {
            dtmp = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SDVENC_SET);
            dtmp |= (1 << 4);
            disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SDVENC_SET, dtmp);  //CGMSA output policy issue
        }
    }
    else
    {
        dtmp = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL);
        dtmp &= ~(VBI_EN_BIT | CGMSA_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, dtmp);

        dtmp = disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SDVENC_SET);
        dtmp &= ~(1 << 4);
        disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_SDVENC_SET, dtmp);  //CGMSA output policy issue
    }

    return MT_SUCCESS;
}

//vbi
mt_s32 DISP_CreateVBIChannel(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_VBI_CFG_S *pstCfg, mt_handle *phVbi)
{
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 vbi_control_reg_val = 0;

    volatile mt_u32 value = 0;
    value = disp_hal_get_u32((volatile mt_u32 *)(mt_get_clk_base() + 0xA600));
    value |= (0x1 << 1);//bit1 set 1
    disp_hal_put_u32((volatile mt_u32 *)(mt_get_clk_base() + 0xA600), value);

    memset(&disp_vbi_buffer, 0, sizeof(disp_vbi_buffer_s));
    disp_vbi_buffer.vbi_type  = (int)pstCfg->eType;

    if(pstCfg->eType == MT_DRV_DISP_VBI_TTX)
    {
        vbi_control_reg_val = (TTTEX_DEC_EN_BIT + TTTEX_EN_BIT + VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
        //enable fifo half empty and fifo empty interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x7);
    }
    else if(pstCfg->eType == MT_DRV_DISP_VBI_CC_PES)
    {
        vbi_control_reg_val = (CC_DEC_EN_BIT +CC_EN_BIT+ VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
        //enable fifo half empty and fifo empty interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x7);
    }
    else if(pstCfg->eType == MT_DRV_DISP_VBI_CC)
    {
        vbi_control_reg_val = (CC_EN_BIT+ VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
        //enable top start, bot start interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x18);
    }
    else if(pstCfg->eType == MT_DRV_DISP_VBI_WSS)
    {
        vbi_control_reg_val = (WSS_EN_BIT + VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
        //enable top start interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x10);
    }
    else if(pstCfg->eType == MT_DRV_DISP_VBI_VPS)
    {
        vbi_control_reg_val = (VPS_EN_BIT + VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
        //enable top start interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x10);
    }
    else if(pstCfg->eType == MT_DRV_DISP_VBI_CGMS_A)
    {
        //enable top start interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x10);
    }
    else if(pstCfg->eType == MT_DRV_DISP_VBI_TTX_ES)
    {
        vbi_control_reg_val = (TTTEX_EN_BIT + VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
        //enable fifo half empty and fifo empty interrupt
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0x7);
        printk("start  sw ttx\n");
    }
    disp_vbi_buffer.bEnable = MT_TRUE;

    *phVbi = 1;
    return nRet;
}

mt_s32 DISP_DestroyVBIChannel(mt_handle hVbi)
{
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 vbi_control_reg_val = 0;
    volatile mt_u32 value = 0;
    disp_vbi_buffer.vbi_type = MT_DISP_VBI_TYPE_BUTT;

    vbi_control_reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL);
    vbi_control_reg_val &= ~(VBI_EN_BIT);
    disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, vbi_control_reg_val);
    //disable vbi interrupt
    disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_INT_MASK_N0, 0);
    disp_vbi_buffer.bEnable = MT_FALSE;

    value = disp_hal_get_u32((volatile mt_u32 *)(mt_get_clk_base() + 0xA600));
    value &= ~(0x1 << 1);//bit1 set 0
    disp_hal_put_u32((volatile mt_u32 *)(mt_get_clk_base() + 0xA600), value);

    return nRet;
}

mt_s32 DISP_SendVbiData(mt_handle hVbi, MT_DRV_DISP_VBI_DATA_S *pstVbiData)
{
    disp_vbi_buffer_s *p_disp_vbi_buffer = &disp_vbi_buffer;
    mt_s32 RemainSpaceSize = 0;

    if(disp_vbi_buffer.bEnable == MT_TRUE)
    {
        RemainSpaceSize = VBI_DATA_BUF_LEN - p_disp_vbi_buffer->vbi_wp;
        //printk("JANNY debug %s %d pu8DataAddr=0x%x u32DataLen=0x%x vbi_wp=0x%x\n", __FUNCTION__, __LINE__, pstVbiData->pu8DataAddr, pstVbiData->u32DataLen, p_disp_vbi_buffer->vbi_wp);

        p_disp_vbi_buffer->vbi_type = (int)pstVbiData->eType;

        //printk("vbi debug %s %d vbi_type=0x%x\n", __FUNCTION__, __LINE__, p_disp_vbi_buffer->vbi_type);
        if (p_disp_vbi_buffer->vbi_type != MT_DISP_VBI_TYPE_CGMS_A)
        {
            if (pstVbiData->u32DataLen <= RemainSpaceSize)
            {
                if (copy_from_user(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_wp, pstVbiData->pu8DataAddr, pstVbiData->u32DataLen))
                {
                    DISP_DEBUGK("vbi debug DISP_SendVbiData %s %s %d\n", __FILE__, __func__, __LINE__);
                    return MT_FAILURE;
                }
                p_disp_vbi_buffer->vbi_wp = p_disp_vbi_buffer->vbi_wp + pstVbiData->u32DataLen;
            }
            else
            {
                if (copy_from_user(p_disp_vbi_buffer->u8DispVbiBuffer + p_disp_vbi_buffer->vbi_wp, pstVbiData->pu8DataAddr, RemainSpaceSize))
                {
                    DISP_DEBUGK("vbi debug DISP_SendVbiData %s %s %d\n", __FILE__, __func__, __LINE__);
                    return MT_FAILURE;
                }
                if (copy_from_user(p_disp_vbi_buffer->u8DispVbiBuffer, pstVbiData->pu8DataAddr + RemainSpaceSize, pstVbiData->u32DataLen - RemainSpaceSize))
                {
                    DISP_DEBUGK("vbi debug DISP_SendVbiData %s %s %d\n", __FILE__, __func__, __LINE__);
                    return MT_FAILURE;
                }
                p_disp_vbi_buffer->vbi_wp = pstVbiData->u32DataLen - RemainSpaceSize;
            }
            //DISP_DEBUGK("vbi debug p_disp_vbi_buffer->vbi_wp=0x%x %s %s %d \n", p_disp_vbi_buffer->vbi_wp, __FILE__, __func__, __LINE__);
            p_disp_vbi_buffer->u8DispVbiBuffer[p_disp_vbi_buffer->vbi_wp] = 0;
        }
    }
    return MT_SUCCESS;
}

mt_s32 DISP_SetWss(MT_DRV_DISPLAY_E enDisp,  MT_DRV_DISP_WSS_DATA_S *pstWssData)
{
    mt_u32 dtmp =0;
    mt_u32 reg_val =0;
    mt_u32 tmp_data =0;

    disp_priv_t *p_dp = &s_stDisplayPriv;
    disp_sys_t tv_sys_sd = p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt;

    if(pstWssData->bEnable == MT_TRUE)
    {
        if ((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
                || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x1001);
            reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA);
            reg_val &= 0xfffffffc;
            tmp_data = (mt_u32)pstWssData->u16Data;
            tmp_data &= 0x3;
            reg_val |= tmp_data;
            reg_val = add_crc6(reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CGMSA_DATA, reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x1014);
        }
        else   //PAL
        {
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, 0x10000001);
            reg_val = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG);
            reg_val &= 0xfffffc3f;;
            tmp_data = ((mt_u32)pstWssData->u16Data) << 6;
            reg_val |= tmp_data;
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_WSS_DATA_REG, reg_val);
            disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_SOFT_CTRL, 0x10000037);
        }
    }
    else
    {
        dtmp = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL);
        dtmp &= ~(VBI_EN_BIT);
        disp_hal_put_u32((volatile mt_u32 *)REG_ARIA_SD_ENC_VBI_CTRL, dtmp);
    }
    return MT_SUCCESS;
}

//may be deleted
mt_s32 DISP_SetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg)
{

    return MT_SUCCESS;
}

mt_s32 DISP_GetHdmiIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_HDMI_S *pstCfg)
{

    return MT_SUCCESS;
}

mt_s32 DISP_SetSetting(MT_DRV_DISPLAY_E enDisp, DISP_SETTING_S *pstSetting)
{

    return MT_SUCCESS;
}

mt_s32 DISP_GetSetting(MT_DRV_DISPLAY_E enDisp, DISP_SETTING_S *pstSetting)
{

    return MT_SUCCESS;
}

mt_s32 DISP_AddIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{

    DISP_DEBUGK("[%s]line[%d]\n", __FUNCTION__, __LINE__);
    disp_set_dacmode(enDisp, DISP_DAC_CVBS_YPBPR_HD);
    return MT_SUCCESS;
}

mt_s32 DISP_DelIntf(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_INTF_S *pstIntf)
{

    return MT_SUCCESS;
}

mt_s32 DISP_GetSlave(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penSlave)
{
#if 0 //not_need
    DISP_S* pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);
    DispCheckNullPointer(penSlave);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);

    // s3 check if eIntf exists
    if (!pstDisp->bIsMaster)
    {
        return MT_ERR_DISP_INVALID_OPT;
    }

    *penSlave = pstDisp->enAttachedDisp;
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_GetMaster(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISPLAY_E *penMaster)
{
#if 0 //not_need
    DISP_S* pstDisp;

    // s1 check input parameters
    DispCheckDeviceState();

    DispCheckID(enDisp);
    DispCheckNullPointer(penMaster);

    // s2 get pointer
    DispGetPointerByID(enDisp, pstDisp);

    // s3 check if eIntf exists
    if (!pstDisp->bIsSlave)
    {
        return MT_ERR_DISP_INVALID_OPT;
    }

    *penMaster = pstDisp->enAttachedDisp;
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_GetDisplaySetting(MT_DRV_DISPLAY_E enDisp,
        MT_DRV_DISP_FMT_E *penFormat,
        MT_DRV_DISP_STEREO_E *peDispMode)
{
#if 0 //disp_hal
    DISP_S* pstDisp;
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();

    DispCheckDeviceState();
    // s1 check input parameters
    DispCheckID(enDisp);

    // s2 check whether display opened
    DispShouldBeOpened(enDisp);

    // s3 get ENABLE state and return
    DispGetPointerByID(enDisp, pstDisp);

    if ( (pstDisp->stSetting.enFormat < MT_DRV_DISP_FMT_BUTT) && pfOpt )
    {
        *penFormat = pstDisp->stSetting.enFormat;
        *peDispMode = pstDisp->stSetting.eDispMode;
    }
    else
    {
        DISP_ERROR("Display %d info not available!\n", enDisp);
        return MT_ERR_DISP_NO_INIT;
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DISP_GetDisplayInfo(MT_DRV_DISPLAY_E enDisp, MT_DISP_DISPLAY_INFO_S *pstInfo)
{
    DISP_S *pstDisp;
    // s1 check input parameters
    DispCheckID(enDisp);
    DispCheckNullPointer(pstInfo);

    // s2 check whether display opened
    DispShouldBeOpened(enDisp);

    // s3 get ENABLE state and return
    DispGetPointerByID(enDisp, pstDisp);
    *pstInfo = pstDisp->stDispInfo;

#if 0
    DISP_S* pstDisp;
    DISP_INTF_OPERATION_S* pfOpt = DISP_HAL_GetOperationPtr();
    MT_BOOL bBtm;
    mt_u32 vcnt;

    //DISP_HAL_ENCFMT_PARAM_S stFmt;
    //mt_s32 nRet;

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);
    DispCheckNullPointer(pstInfo);

    // s2 check whether display opened
    DispShouldBeOpened(enDisp);

    // s3 get ENABLE state and return
    DispGetPointerByID(enDisp, pstDisp);

    if ( (pstDisp->stSetting.enFormat < MT_DRV_DISP_FMT_BUTT) && pfOpt )
    {
        *pstInfo = pstDisp->stDispInfo;
        DispCheckNullPointer(pfOpt->FP_GetChnBottomFlag);

        pfOpt->FP_GetChnBottomFlag(enDisp, &bBtm, &vcnt);

        pstInfo->bIsBottomField = bBtm;
        pstInfo->u32Vline = vcnt;
    }
    else
    {
        DISP_ERROR("Display %d info not available!\n", enDisp);
        return MT_ERR_DISP_NO_INIT;
    }
#endif
    return MT_SUCCESS;
}

#if 1 //def MT_DISP_BUILD_FULL
mt_s32 DISP_RegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
        MT_DRV_DISP_CALLBACK_S *pstCB)
{
    //    DISP_S *pstDisp;
    //    DISP_HAL_ENCFMT_PARAM_S stFmt;
    mt_s32 nRet;
    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);
    DispCheckNullPointer(pstCB);

    //add because not_need
    if (enDisp != MT_DRV_DISPLAY_1)
    {
        DISP_DEBUGK("%s %d this isr is not needed \n", __FUNCTION__, __LINE__);
        return MT_SUCCESS;
    }

    // s2 check whether display opened
    DispShouldBeOpened(enDisp);

    if (!pstCB->pfDISP_Callback)
    {
        DISP_ERROR("Callback function is null!\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    nRet = DISP_ISR_RegCallback(enDisp, eType, pstCB);

    return nRet;
}

mt_s32 DISP_UnRegCallback(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CALLBACK_TYPE_E eType,
        MT_DRV_DISP_CALLBACK_S *pstCB)
{
    //    DISP_S *pstDisp;
    //    DISP_HAL_ENCFMT_PARAM_S stFmt;
    mt_s32 nRet;

    DispCheckDeviceState();

    // s1 check input parameters
    DispCheckID(enDisp);
    DispCheckNullPointer(pstCB);

    //add because not_need
    if (enDisp != MT_DRV_DISPLAY_1)
    {
        DISP_DEBUGK("%s %d this isr is not needed \n", __FUNCTION__, __LINE__);
        return MT_SUCCESS;
    }

    // s2 check whether display opened
    //DispShouldBeOpened(enDisp);

    if (!pstCB->pfDISP_Callback)
    {
        DISP_ERROR("Callback function is null!\n");
        return MT_ERR_DISP_INVALID_PARA;
    }

    nRet = DISP_ISR_UnRegCallback(enDisp, eType, pstCB);

    return nRet;
}

mt_void DISP_GetCastInfor(DISP_CAST_S *pstCast_info, DISP_Cast_PROC_INFO_S *pstCastProc)
{
    mt_u32 i = 0;

    if (pstCast_info)
    {
        pstCastProc->u32CastEmptyBufferNum = 0;
        pstCastProc->u32CastFullBufferNum = 0;
        pstCastProc->u32CastWriteBufferNum = 0;
        pstCastProc->u32CastUsedBufferNum = 0;

        for (i = 0; i < pstCast_info->stBP.u32BufNum; i++)
        {
            switch (pstCast_info->stBP.pstBufQueue[i].enState)
            {
            case BUF_STATE_EMPTY:
                pstCastProc->u32CastEmptyBufferNum++;
                break;
            case BUF_STATE_FULL:
                pstCastProc->u32CastFullBufferNum++;
                break;
            case BUF_STATE_WRITING:
                pstCastProc->u32CastWriteBufferNum++;
                break;
            case BUF_STATE_READING:
            case BUF_STATE_DONE:
                pstCastProc->u32CastUsedBufferNum++;
                break;
            default:
                break;
            }
        }

        pstCastProc->bEnable = pstCast_info->bEnable;
        pstCastProc->bLowDelay = pstCast_info->bLowDelay;
        pstCastProc->bUserAllocate = pstCast_info->stConfig.bUserAlloc;
        pstCastProc->u32OutResolutionWidth = pstCast_info->stAttr.stOut.s32Width;
        pstCastProc->u32OutResolutionHeight = pstCast_info->stAttr.stOut.s32Height;
        pstCastProc->u32CastOutFrameRate = pstCast_info->stDispInfo.u32RefreshRate;
        pstCastProc->u32TotalBufNum = pstCast_info->stBP.u32BufNum;
        pstCastProc->u32BufSize = pstCast_info->stBP.u32BufSize;
        pstCastProc->u32BufStride = pstCast_info->stBP.u32BufStride;

        pstCastProc->u32CastAcquireTryCnt = pstCast_info->u32CastAcquireTryCnt;
        pstCastProc->u32CastAcquireOkCnt = pstCast_info->u32CastAcquireOkCnt;
        pstCastProc->u32CastReleaseTryCnt = pstCast_info->u32CastReleaseTryCnt;
        pstCastProc->u32CastReleaseOkCnt = pstCast_info->u32CastReleaseOkCnt;
        pstCastProc->u32CastIntrCnt = pstCast_info->u32CastIntrCnt;
        pstCastProc->bAttached = pstCast_info->bAttached;

        for (i = 0; i < pstCastProc->u32TotalBufNum; i++)
        {
            pstCastProc->enState[i] = (mt_u32)pstCast_info->stBP.pstBufQueue[i].enState;
            pstCastProc->u32FrameIndex[i] = pstCast_info->stBP.pstBufQueue[i].stFrame.slotInfo.frm_cnt;
        }
    }

    return;
}

mt_s32 DISP_GetProcInto(MT_DRV_DISPLAY_E enDisp, DISP_PROC_INFO_S *pstInfo)
{
    DISP_S *pstDisp;
    mt_s32 i;

    DispCheckDeviceState();
    DispCheckID(enDisp);
    DispCheckNullPointer(pstInfo);
    DispShouldBeOpened(enDisp);
    DispGetPointerByID(enDisp, pstDisp);

    /*the channel such as display0,or 1 enable or not.*/
    pstInfo->bEnable = pstDisp->bEnable;
    pstInfo->eFmt = pstDisp->stSetting.enFormat;
    pstInfo->eDispMode = pstDisp->stSetting.eDispMode;
    pstInfo->bRightEyeFirst = pstDisp->stSetting.bRightEyeFirst;
    pstInfo->stVirtaulScreen = pstDisp->stSetting.stVirtaulScreen;
    pstInfo->stOffsetInfo = pstDisp->stSetting.stOffsetInfo;

    /*get the aspect setting.*/
    pstInfo->bCustAspectRatio = pstDisp->stSetting.bCustomRatio;
    pstInfo->u32AR_w = pstDisp->stDispInfo.stAR.u32ARw;
    pstInfo->u32AR_h = pstDisp->stDispInfo.stAR.u32ARh;

    /*get the csc space transfer.*/
    pstInfo->eDispColorSpace = pstDisp->stDispInfo.eColorSpace;

    /*get the display csc setting.*/
    pstInfo->u32Bright = pstDisp->stDispInfo.u32Bright;
    pstInfo->u32Hue = pstDisp->stDispInfo.u32Hue;
    pstInfo->u32Satur = pstDisp->stDispInfo.u32Satur;
    pstInfo->u32Contrst = pstDisp->stDispInfo.u32Contrst;

    /*get the background setting.*/
    pstInfo->stBgColor = pstDisp->stSetting.stBgColor;

    /*get the zorder, which one is on the top or bottom.*/
    for (i = 0; i < MT_DRV_DISP_LAYER_BUTT; i++)
    {
        pstInfo->enLayer[i] = pstDisp->stSetting.enLayer[i];
    }

    /*get the master or slave role, and attached layer.*/
    pstInfo->bMaster = pstDisp->bIsMaster;
    pstInfo->bSlave = pstDisp->bIsSlave;
    pstInfo->enAttachedDisp = pstDisp->enAttachedDisp;

    /*get the unflow times.*/
    pstInfo->u32Underflow = pstDisp->u32Underflow;

    /*FIXME: i don't know what does it means.*/
    pstInfo->u32StartTime = pstDisp->u32StartTime;
    pstInfo->stTiming = pstDisp->stSetting.stCustomTimg;
    pstInfo->stColorSetting = pstDisp->stSetting.stColor;

    /*get the cast information.*/
    pstInfo->pstCastInfor = pstDisp->Cast_ptr;

    DISP_GetCastInfor((DISP_CAST_S *)pstDisp->Cast_ptr, &pstInfo->stCastInfor);

    pstInfo->u32IntfNumber = 0;
    for (i = 0; i < MT_DRV_DISP_INTF_ID_MAX; i++)
    {
        if (pstDisp->stSetting.stIntf[i].bOpen)
        {
            pstInfo->stIntf[pstInfo->u32IntfNumber] = pstDisp->stSetting.stIntf[i].stIf;
            pstInfo->u32Link[pstInfo->u32IntfNumber] = pstDisp->stSetting.stIntf[i].eVencId;
            pstInfo->u32IntfNumber++;
        }
    }

    return MT_SUCCESS;
}

#if 0 //disp_hal

mt_s32 DISP_SetGammaCtrl(MT_DRV_DISPLAY_E eDisp, GAMMA_MODE_E enGammaMode, MT_BOOL bEnable)
{
    mt_s32 s32Ret;
    DISP_S* pstDisp;
    GAMMA_CS_E enGammCsMode  ;
    // s1 check input parameters
    // DispCheckDeviceState();
    DispCheckID(eDisp);

    if (GAMMA_PQ_MODE_BUTT <= enGammaMode)
    {
        DISP_PRINT("DISP_SetGammaCtrl, Gamma mode = %d error\r\n", enGammaMode);
        return MT_ERR_DISP_INVALID_PARA;
    }
    // s2 get pointer
    DispGetPointerByID(eDisp, pstDisp);

    //s3 get disp color space mode
    if (MT_DRV_DISP_FMT_861D_640X480_60 > pstDisp->stSetting.enFormat)
    {
        enGammCsMode = GAMMA_MODE_YUV;
    }
    else
    {
        enGammCsMode = GAMMA_MODE_RGB;
    }

    //check para
    s32Ret = pstDisp->pstIntfOpt->PF_SetGammaCtrl(eDisp, enGammCsMode, enGammaMode, bEnable);

    return s32Ret;
}
mt_s32 DISP_UpdateGamma(GAMMA_CS_E  enGammaCsMode, PQ_GAMMA_RGB_MODE_S* pstPqGammaModeData)
{
    mt_s32 s32Ret;
    MT_DRV_DISPLAY_E eDisp;
    DISP_S* pstDisp;

    eDisp = MT_DRV_DISPLAY_0;
    // s1 check input parameters
    //DispCheckDeviceState();
    DispCheckID(eDisp);

    if (GAMMA_MODE_YUV < enGammaCsMode)
    {
        DISP_PRINT("DISP_UpdateGamma, cs mode = %d error\r\n", enGammaCsMode);
        return MT_ERR_DISP_INVALID_PARA;
    }

    // s2 get pointer
    DispGetPointerByID(eDisp, pstDisp);
    DispCheckNullPointer(pstPqGammaModeData);

    s32Ret =  pstDisp->pstIntfOpt->PF_UpdateGamma(enGammaCsMode, pstPqGammaModeData);

    return s32Ret;
}
#endif

#endif

/* ****************************************************
 *  O5 data
 * *****************************************************/
mt_u32 g_O5_u32DacPower = 0;
mt_u32 g_O5_u32DacType = 0;
mt_u32 g_O5_u32Signal = 0;
mt_u8 g_O5_u8MacvTable[18];
mt_u32 g_O5_u8MacvFlag = 0;
/**************************************************************/
mt_s32 DRV_DISP_O5_EnableDacPower(MT_BOOL bEnable)
{
    return MT_SUCCESS;
}
mt_s32 DRV_DISP_O5_GetDacPower(MT_BOOL *bEnable)
{
    *bEnable = (g_O5_u32DacPower) ? MT_TRUE : MT_FALSE;
    return MT_SUCCESS;
}
mt_u32 *DRV_DISP_O5_GetSignal(mt_void)
{
    return &g_O5_u32Signal;
}
mt_s32 DRV_DISP_O5_SetSignal(mt_u32 u32DacType, mt_u32 flag)
{

    return MT_SUCCESS;
}

mt_s32 DRV_DISP_O5_SetMacv(MT_BOOL bEnable)
{

    mt_s32 ret = 0;

    return ret;
}

mt_u32 DRV_DISP_O5_GetMacv(mt_void)
{
    return g_O5_u8MacvFlag;
}

mt_s32 DRV_DISP_O5_SetMacvCps(mt_char *ptable)
{

    return MT_SUCCESS;
}

mt_s32 DRV_DISP_O5_GetCGMS(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CGMSA_CFG_S *pstcgms)
{
    return MT_SUCCESS;
}

mt_s32 DRV_DISP_O5_GetWSS(MT_DRV_DISP_WSS_DATA_S *pstWssData)
{
    return MT_SUCCESS;
}

mt_s32 DRV_DISP_O5_SetCGMS(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_CGMSA_CFG_S *pstCgmsCfg)
{
    (mt_void) DISP_SetCGMS_A(enDisp, pstCgmsCfg);

    return MT_SUCCESS;
}
mt_s32 DRV_DISP_O5_SetWss(MT_DRV_DISPLAY_E enDisp, MT_DRV_DISP_WSS_DATA_S *pstWssData)
{
    (mt_void) DISP_SetWss(enDisp, pstWssData);

    return MT_SUCCESS;
}
// for hd/sd tvencoder setting

void set_hd_tvsys_regs(disp_sys_t tv_sys_hd)
{
    mt_u8 frame_rate = 4;     // 1:24, 2:25, 3:30, 4:50, 5:60
    mt_u8 resolution = 0;     //0:1920*1080,1:1280*720,2:720*576,3:720*480,4:3840x2160,5:4096x2160
    mt_u8 interlace_mode = 0; //0:progressive,1:interlace

    switch(tv_sys_hd)
    {
    case VID_SYS_PAL:
    case VID_SYS_PAL_N:
    case VID_SYS_PAL_NC:
        frame_rate = 4;
        resolution = 2;
        interlace_mode = 1;
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x18);            //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x7e);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x8a);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x5a0);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x2);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x3);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x13);           //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x120);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x258);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x0);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x0);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x7e0018);          //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x5a0008a);         //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x8a0018);   //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x025805a0); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x60004);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x2400026);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x5a00001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x2400002);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x5a00001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x2400002);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xb4);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe0000009);             //reg 0x14


        break;
    case VID_SYS_NTSC_J:
    case VID_SYS_NTSC_M:
    case VID_SYS_NTSC_443:
    case VID_SYS_PAL_M:
        frame_rate = 5;
        resolution = 3;
        interlace_mode = 1;

        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x26);            //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x7c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x72);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x5a0);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x4);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x3);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0xf);            //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0xf0);           //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x26c);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x0);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x0);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x7c0026);          //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x5a00072);         //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x720026);   //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x026c05a0); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x60008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x1e0001e);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x5a00001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x1e00001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x5a00001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x1e00001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xb4);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe000001b);             //reg 0x14


        break;
    case VID_SYS_576P_50HZ:
        frame_rate = 4;
        resolution = 2;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x200200); //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x295295); //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02db); //reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x0113);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x1ef0008);      //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x100d59f); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x100d59f); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0xc);           //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x40);            //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x44);          //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x2d0);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x5);          //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);            //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x27);         //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x240);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x0);              //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x0);              //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x403c400c);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x2d00044);       //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x440010); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x102d0);  //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x50005);         //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x2400027);       //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x2d00001);    //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x2400001);    //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x2d00001);  //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x2400001);  //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0x5a);               //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe0100208);           //reg 0x14

        break;
    case VID_SYS_480P:
        frame_rate = 5;
        resolution = 3;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x205205);   //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x293293);   //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02e5);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x010a);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x1ef0008);           //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x100d59f); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1009f30); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x10);          //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x3e);            //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x3c);          //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x2d0);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x9);          //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x6);            //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x1e);         //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x1e0);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x0);              //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x0);              //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x403e4010);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x2d0003c);       //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x3c0010); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x102d0);  //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x60009);         //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x1e0001e);       //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x2d00001);    //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x1e00001);    //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x2d00001);  //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x1e00001);  //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0x5a);               //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe010021a);           //reg 0x14

        break;
    case VID_SYS_720P:
        frame_rate = 5;
        resolution = 1;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x208208);   //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x2a02a0);   //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02d6);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x010b);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x2160003);           //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x1006f00); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1006f00); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x6e);          //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x28);            //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0xdc);          //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x500);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x5);          //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);            //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x14);         //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x2d0);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);              //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);              //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x20502046);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x50000dc);       //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0xdc0046); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x10500);  //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x50005);         //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x2d00014);       //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x5000001);    //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x2d00001);    //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x5000001);  //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x2d00001);  //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xa0);               //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe010022e);           //reg 0x14

        break;
    case VID_SYS_720P_50HZ:
        frame_rate = 4;
        resolution = 1;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x20a20a);   //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x2a02a0);   //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02d7);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x010d);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x2160004);           //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x1008692); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1008692); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x1b8);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x28);            //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0xdc);          //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x500);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x5);          //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);            //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x14);         //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x2d0);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);              //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);              //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x20502190);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x50000dc);       //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0xdc0190); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x10500);  //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x50005);         //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x2d00014);       //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x5000001);    //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x2d00001);    //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x5000001);  //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x2d00001);  //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xa0);               //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe010022c);           //reg 0x14

        break;
    case VID_SYS_1080I:
        frame_rate = 5;
        resolution = 0;
        interlace_mode = 1;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x205205);     //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x2a02a0);     //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02d6);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x010f);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x2180004);             //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x1008692); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1008692); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x58);            //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x2);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0xf);            //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x21c);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x38c);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x2058202c);          //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x7bc0058);         //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x94002c);   //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x038c0780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0xa0004);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x438001e);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe010002f);             //reg 0x14

        break;
    case VID_SYS_1080I_50HZ:
        frame_rate = 4;
        resolution = 0;
        interlace_mode = 1;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x208208);     //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x2a02a0);     //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02da);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x010d);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x2170002);             //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x1008692); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1008692); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x210);           //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x2);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0xf);            //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x21c);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x468);         //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x105811e4);          //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x7bc0058);         //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x9401e4);   //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x04680780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0xa0004);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x438001e);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe010002d);             //reg 0x14

        break;
    case VID_SYS_1080P:
        frame_rate = 5;
        resolution = 0;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x20a20a);     //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x2a02a0);     //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02d5);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x010d);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x2170003);             //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x1003f5d); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1009f3a); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x58);            //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x4);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x24);           //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x438);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);           //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x1058102C);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07BC0058);        //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x0094002C); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00010780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x00050004);        //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x04380024);        //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe011002e);             //reg 0x14

        break;
    case VID_SYS_1080P_50HZ:
        frame_rate = 4;
        resolution = 0;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x205205);     //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x2a02a0);     //reg 0x10
        reg_aria_hd_encoder_set_y_parameter_level0_y_a_level(0x02d6);//reg 0x4
        reg_aria_hd_encoder_set_y_parameter_level0_y_blank_level(0x0110);//reg 0x4
        reg_aria_hd_encoder_set_y_sync_level(0x1f40006);             //reg 0x8
        reg_aria_hd_encoder_set_vdac_adjusting_y(0x1003f5d); //reg 0xec
        reg_aria_hd_encoder_set_vdac_adjusting_uv(0x1004f35); //reg 0xf0
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x210);           //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x4);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x24);           //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x438);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);           //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x105811E4);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07BC0058);        //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x009401E4); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00010780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x50004);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x4380024);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe011002c);             //reg 0x14

        break;
    case VID_SYS_1080P_25HZ:
        frame_rate = 2;
        resolution = 0;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x210);           //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x4);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x24);           //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x438);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);           //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x005801E4);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07BC0058);        //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x009401E4); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00010780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x50004);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x4380024);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe001002c);             //reg 0x14

        break;
    case VID_SYS_1080P_30HZ:
        frame_rate = 3;
        resolution = 0;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x204204);     //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x280280);     //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x58);            //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x4);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x24);           //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x438);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);           //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x0058002C);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07BC0058);        //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x0094002C); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00010780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x00050004);        //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x04380024);        //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe001002e);             //reg 0x14

        break;
    case VID_SYS_1080P_24HZ:
        frame_rate = 1;
        resolution = 0;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x204204);     //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x280280);     //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x27e);           //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x2c);              //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x94);            //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x780);           //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x4);            //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x5);              //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x24);           //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x438);          //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x0);           //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x1);                //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x1);                //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00580252);        //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07BC0058);        //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x00940252); //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00010780); //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x50004);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x4380024);         //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x7800001);      //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x4380001);      //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x7800001);    //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x4380001);    //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                 //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xe001002c);             //reg 0x14
        break;

    case VID_SYS_3840X2160_60HZ:
        frame_rate = 5;
        resolution = 4;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x000000b0);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000128);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00000f00);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x005800b0);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x012800b0);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00050f00);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x0f000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x0f000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_3840X2160_50HZ:
        frame_rate = 4;
        resolution = 4;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x00000420);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000128);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00000f00);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00580420);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x01280420);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00050f00);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x0f000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x0f000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_3840X2160_30HZ:
        frame_rate = 3;
        resolution = 4;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x000000b0);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000128);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00000f00);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x005800b0);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x012800b0);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00050f00);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x0f000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x0f000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_3840X2160_25HZ:
        frame_rate = 2;
        resolution = 4;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x00000420);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000128);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00000f00);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00580420);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x01280420);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00050f00);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x0f000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x0f000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_3840X2160_24HZ:
        frame_rate = 1;
        resolution = 4;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x000004fc);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000128);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00000f00);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x005804fc);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x012804fc);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00050f00);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x0f000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x0f000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_4096X2160_24HZ:
        frame_rate = 1;
        resolution = 5;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x000003fc);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000128);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00001000);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00587000);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x012803fc);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00051000);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x10000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x10000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_4096X2160_25HZ:
        frame_rate = 2;
        resolution = 5;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x000003c8);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000080);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00001000);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00587000);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x008003c8);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00051000);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x10000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x10000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_4096X2160_30HZ:
        frame_rate = 3;
        resolution = 5;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x00000058);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000080);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00001000);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00587000);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x00800058);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00051000);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x10000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x10000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;


    case VID_SYS_4096X2160_50HZ:
        frame_rate = 4;
        resolution = 5;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x000003c8);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000080);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00001000);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00587000);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x008003c8);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00051000);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x10000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x10000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    case VID_SYS_4096X2160_60HZ:
        frame_rate = 5;
        resolution = 5;
        interlace_mode = 0;

        reg_aria_hd_encoder_set_pbpr_parameter0_level(0x00208208);      //reg 0xc
        reg_aria_hd_encoder_set_pbpr_parameter1_level(0x002a02a0);      //reg 0x10
        reg_aria_hd_encoder_set_hdmi_h_fporchwidth(0x00000058);         //reg 0x50
        reg_aria_hd_encoder_set_hdmi_h_syncwidth(0x00000058);           //reg 0x54
        reg_aria_hd_encoder_set_hdmi_h_bporchwidth(0x00000080);         //reg 0x58
        reg_aria_hd_encoder_set_hdmi_h_activewidth(0x00001000);         //reg 0x5c
        reg_aria_hd_encoder_set_hdmi_v_fporchheight(0x00000008);        //reg 0x60
        reg_aria_hd_encoder_set_hdmi_v_syncheight(0x0000000a);          //reg 0x64
        reg_aria_hd_encoder_set_hdmi_v_bporchheight(0x00000048);        //reg 0x68
        reg_aria_hd_encoder_set_hdmi_v_activeheight(0x00000870);        //reg 0x6c
        reg_aria_hd_encoder_set_hdmi_h_halflinewidth(0x00000468);       //reg 0x70
        reg_aria_hd_encoder_set_hdmi_hsync_pola(0x00000001);            //reg 0x74
        reg_aria_hd_encoder_set_hdmi_vsync_pola(0x00000001);            //reg 0x78
        reg_aria_hd_encoder_set_hsync_parameter0(0x00587000);           //reg 0x18
        reg_aria_hd_encoder_set_hsync_parameter1(0x07bc0058);           //reg 0x1c
        reg_aria_hd_encoder_set_reg_h_active_parameter0(0x00800058);    //reg 0x20
        reg_aria_hd_encoder_set_reg_h_active_parameter1(0x00051000);    //reg 0x24
        reg_aria_hd_encoder_set_reg_v_parameter0(0x000a0008);           //reg 0x28
        reg_aria_hd_encoder_set_reg_v_parameter1(0x08700048);           //reg 0x2c
        reg_aria_hd_encoder_set_reg_h_osd_parameter(0x10000001);        //reg 0x30
        reg_aria_hd_encoder_set_reg_v_osd_parameter(0x08700001);        //reg 0x34
        reg_aria_hd_encoder_set_reg_h_video_parameter(0x10000001);      //reg 0x38
        reg_aria_hd_encoder_set_reg_v_video_parameter(0x08700001);      //reg 0x3c
        reg_aria_hd_encoder_set_reg_color_bar(0xf0);                    //reg 0x40
        reg_aria_hd_encoder_set_hd_cfg_info(0xa100002c);                //reg 0x14
        break;

    default:
        return;
    }

    drv_reg_4k_disp_set_video_ctrl_2_frame_rate(frame_rate);
    reg_aria_hd_encoder_set_basic_cfg_resolusion(resolution);
    reg_aria_hd_encoder_set_basic_cfg_interlace_mode(interlace_mode);

}

MT_BOOL is_pal(disp_sys_t tv_sys)
{
    if ((tv_sys == VID_SYS_PAL) || (tv_sys == VID_SYS_PAL_N) || (tv_sys == VID_SYS_PAL_NC))
        return MT_TRUE;
    else
        return MT_FALSE;
}

MT_BOOL is_ntsc(disp_sys_t tv_sys)
{
    if ((tv_sys == VID_SYS_NTSC_J) || (tv_sys == VID_SYS_NTSC_M) || (tv_sys == VID_SYS_NTSC_443) || (tv_sys == VID_SYS_PAL_M))
        return MT_TRUE;
    else
        return MT_FALSE;
}

void disp_hal_set_dac_fmt(dac_index_t index, dac_fmt_t fmt, disp_channel_t ch)
{
    mt_u32 mode = 0;

    if (DISP_CHANNEL_SD == ch)
    {
        switch (fmt)
        {
        case DRV_DAC_CVBS:
            mode = 0;
            break;
        case DAC_LUMA:
            mode = 1;
            break;
        case DAC_CHROMA:
            mode = 2;
            break;
            //      case DAC_Y:
        case DAC_G:
            mode = 3;
            break;
            //      case DAC_U:
        case DAC_B:
            mode = 4;
            break;
            //      case DAC_V:
        case DAC_R:
            mode = 5;
            break;
        case DAC_Y:
            mode = 6;
            break;
        case DAC_U:
            mode = 7;
            break;
        case DAC_V:
            mode = 8;
            break;
        default:
            break;
        }
        //4dac
        reg_aria_sd_enc_set_sd_enc_dacnum_dac_switch(0);
        //all cvbs in sd
        //        reg_aria_sd_enc_set_sd_enc_mode_vid_mode(3);
        //if scart, 4dacs all used with cvbs 0, G 1,B 2,R 3
        if ((3 == mode) || (4 == mode) || (5 == mode))
        {
            //            reg_aria_sd_enc_set_sd_enc_mode_vid_mode(2);
        }
        //if ypbpr, 4dacs all used with cvbs 0, Y 1,Pb 2,Pr 3
        if ((6 == mode) || (7 == mode) || (8 == mode))
        {
            //            reg_aria_sd_enc_set_sd_enc_mode_vid_mode(1);
        }
        //if s-video, 4dacs all used with cvbs 0, s-Y 1,s-UV 2,cvbs 3
        if ((1 == mode) || (2 == mode))
        {
            //            reg_aria_sd_enc_set_sd_enc_mode_vid_mode(0);
        }

        switch (index)
        {
        case DAC_0:
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_0_hdsd_sel(1);
            break;
        case DAC_1:
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_1_hdsd_sel(1);
            break;
        case DAC_2:
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_2_hdsd_sel(1);
            break;
        case DAC_3:
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_3_hdsd_sel(1);
            break;
        default:
            break;
        }
    }
    else if (DISP_CHANNEL_HD == ch)
    {
        //mode: 0-Y/G,1-U/B,2-V/R
        switch (fmt)
        {
        case DAC_Y:
        case DAC_G:
            mode = 0;
            break;
        case DAC_U:
        case DAC_B:
            mode = 1;
            break;
        case DAC_V:
        case DAC_R:
            mode = 2;
            break;
        case DRV_DAC_CVBS:
        case DAC_LUMA:
        case DAC_CHROMA:
        default:
            //hd do not support cvbs and svideo
            break;
        }

        switch (index)
        {
        case DAC_0:
            reg_aria_hd_encoder_set_dac_sel_dac_0_mode(mode);
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_0_hdsd_sel(0);
            break;
        case DAC_1:
            reg_aria_hd_encoder_set_dac_sel_dac_1_mode(mode);
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_1_hdsd_sel(0);
            break;
        case DAC_2:
            reg_aria_hd_encoder_set_dac_sel_dac_2_mode(mode);
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_2_hdsd_sel(0);
            break;
        case DAC_3:
            reg_aria_hd_encoder_set_dac_sel_dac_3_mode(mode);
            //0:hd, 1: sd
            reg_aria_hd_encoder_set_dac_sel_dac_3_hdsd_sel(0);
            break;
        default:
            break;
        }
    }

    return;
}
/*!
  Turn on/off the video DACs.

  \param[in] is_on TRUE to turn on video DACs and FALSE to turn off them.
  */
static void disp_hal_set_dac_onoff(dac_index_t index, MT_BOOL b_on)
{
    mt_u32 value = 0;
    DISP_DEBUGK("[%s]line%d index:%d b_on:%d\n", __FUNCTION__, __LINE__, index, b_on);

    value = disp_hal_get_u32((volatile mt_u32 *)(REG_CLK_ANALOG));

    if (MT_FALSE == b_on)
    {
        value = value | (1 << 6);
    }
    else
    {
        value = value & (~(1 << 6));
    }

    disp_hal_put_u32((volatile mt_u32 *)(REG_CLK_ANALOG), value);

    return;
}

static void disp_update_cvbs_for_sd_tvsys(disp_priv_t *p_dp, MT_BOOL b_on)
{
    mt_u32 i;
    for(i = 0; i < CVBS_GRP_MAX; i++)
    {
        if(p_dp->venc_info.cvbs_dac[i].b_on == MT_TRUE)
        {
            disp_hal_set_dac_onoff((int)p_dp->venc_info.cvbs_dac[i].cvbs_grp_id, b_on);
        }
    }
}

/*!
  Turn on/off the video DACs.

  \param[in] is_on MT_TRUE to turn on video DACs and FALSE to turn off them.
  */
void disp_hal_set_dac_pll_clk(dac_index_t index, mt_u32 sel_sd_clk)
{
    mt_u32 value = 0;
    unsigned long chip_rev = symphony_get_chip_rev();;

    if(index == DAC_0)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10));
        if(1 == sel_sd_clk)
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value |= (1 << 13);
                value |= (1 << 15);
            }
            else
            {
                value |= (1 << 13);
                value |= (1 << 7);
            }
        }
        else
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value &= (~(1 << 13));
                value &= (~(1 << 15));
            }
            else
            {
                value &= (~(1 << 13));
                value &= (~(1 << 7));
            }
        }
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10), value);
    }
    else if(index == DAC_1)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10));
        if(1 == sel_sd_clk)
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value |= (1 <<29);
                value |= (1 <<31);
            }
            else
            {
                value |= (1 <<29);
                value |= (1 <<23);
            }
        }
        else
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value &= (~(1 << 29));
                value &= (~(1 << 31));
            }
            else
            {
                value &= (~(1 << 29));
                value &= (~(1 << 23));
            }
        }
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10), value);
    }
    else if(index == DAC_2)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14));
        if(1 == sel_sd_clk)
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value |= (1 <<13);
                value |= (1 << 15);
            }
            else
            {
                value |= (1 <<13);
                value |= (1 << 7);
            }
        }
        else
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value &= (~(1 << 13));
                value &= (~(1 << 15));
            }
            else
            {
                value &= (~(1 << 13));
                value &= (~(1 << 7));
            }
        }
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14), value);
    }
    else if(index == DAC_3)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14));
        if(1 == sel_sd_clk)
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value |= (1 <<29);
                value |= (1 <<31);
            }
            else
            {
                value |= (1 <<29);
                value |= (1 <<23);
            }
        }
        else
        {
            if(chip_rev >= CHIP_SYMPHONY3_A0)
            {
                value &= (~(1 << 29));
                value &= (~(1 << 31));
            }
            else
            {
                value &= (~(1 << 29));
                value &= (~(1 << 23));
            }
        }
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14), value);
    }

    return;
}

void disp_hal_set_dac_pll_polarity(dac_index_t index, mt_u32 polarity)
{
    mt_u32 value = 0;
    if(index == DAC_0)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10));
        if(1 == polarity)
            value = value | (1 <<11);
        else
            value = value & (~(1 << 11));
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10), value);
    }
    else if(index == DAC_1)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10));
        if(1 == polarity)
            value = value | (1 <<27);
        else
            value = value & (~(1 << 27));
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10), value);
    }
    else if(index == DAC_2)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14));
        if(1 == polarity)
            value = value | (1 <<11);
        else
            value = value & (~(1 << 11));
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14), value);
    }
    else if(index == DAC_3)
    {
        value = disp_hal_get_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14));
        if(1 == polarity)
            value = value | (1 <<27);
        else
            value = value & (~(1 << 27));
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x14), value);
    }

    return;
}

/*!
  On or Off specific CVBS output.

  \param[in] p_disp  The handle of display module.
  \param[in] grp_id  the group id of CVBS
  \param[in] b_on   TRUE for on, FALSE for off
  */
mt_s32 disp_aria_cvbs_onoff(disp_priv_t *p_disp, cvbs_dacgrp_t grp_id, MT_BOOL b_on)
{
    disp_priv_t *p_dp = (disp_priv_t *)p_disp;

    switch (grp_id)
    {
    case CVBS_GRP0:
        DISP_DEBUGK("[%s]line%d\n", __FUNCTION__, __LINE__);
        disp_hal_set_dac_fmt(DAC_0, DRV_DAC_CVBS, DISP_CHANNEL_SD);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case CVBS_GRP1:
        DISP_DEBUGK("[%s]line%d\n", __FUNCTION__, __LINE__);
        disp_hal_set_dac_fmt(DAC_1, DRV_DAC_CVBS, DISP_CHANNEL_SD);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    case CVBS_GRP2:
        DISP_DEBUGK("[%s]line%d\n", __FUNCTION__, __LINE__);
        disp_hal_set_dac_fmt(DAC_2, DRV_DAC_CVBS, DISP_CHANNEL_SD);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case CVBS_GRP3:
        DISP_DEBUGK("[%s]line%d\n", __FUNCTION__, __LINE__);
        disp_hal_set_dac_fmt(DAC_3, DRV_DAC_CVBS, DISP_CHANNEL_SD);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    default:
        //only support dac 0 as cvbs
        break;
    }
    p_dp->venc_info.cvbs_dac[grp_id].cvbs_grp_id = grp_id;
    p_dp->venc_info.cvbs_dac[grp_id].b_on = b_on;

    return MT_SUCCESS;
}

/*!
  Set the colorspace of component output.

  \param[in] p_disp  The handle of display module.
  \param[in] grp_id  the group id of component
  \param[in] type the type of colorspace
  */
mt_s32 disp_aria_component_set_type(disp_priv_t *p_disp, component_dacgrp_t grp_id, colorspace_t type)
{
    disp_priv_t *p_dp = (disp_priv_t *)p_disp;

    //disp_hal_set_component_type(type);
    p_dp->venc_info.comp_dac[grp_id].comp_grp_id = grp_id;
    p_dp->venc_info.comp_dac[grp_id].comp_colorspace = type;

    return MT_SUCCESS;
}

/*!
  On or Off specific componen output.

  \param[in] p_disp  The handle of display module.
  \param[in] grp_id  the group id of component
  \param[in] b_on   TRUE for on, FALSE for off
  */
mt_s32 disp_aria_component_onoff(disp_priv_t *p_disp, component_dacgrp_t grp_id, MT_BOOL b_on)
{
    disp_priv_t *p_dp = (disp_priv_t *)p_disp;
    colorspace_t color_sp;

    color_sp = p_dp->venc_info.comp_dac[grp_id].comp_colorspace;
    switch (grp_id)
    {
    case COMPONENT_GRP0:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_SD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_SD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_SD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_SD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_SD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_SD);
        }
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP1:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_HD);
        }
        //disp_hal_set_dac_onoff(DAC_1, b_on);
        //disp_hal_set_dac_onoff(DAC_2, b_on);
        //disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP2:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case COMPONENT_GRP3:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case COMPONENT_GRP4:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case COMPONENT_GRP5:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP6:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_1, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case COMPONENT_GRP7:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case COMPONENT_GRP8:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP9:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    case COMPONENT_GRP10:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP11:
        if (COLOR_YUV == color_sp)
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_V, DISP_CHANNEL_HD);
        }
        else
        {
            disp_hal_set_dac_fmt(DAC_0, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    case COMPONENT_GRP12:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_0, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_0, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case COMPONENT_GRP13:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_2, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_2, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    case COMPONENT_GRP14:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_2, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_2, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP15:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_2, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_2, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case COMPONENT_GRP16:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_2, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_2, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        break;
    case COMPONENT_GRP17:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_2, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_2, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case COMPONENT_GRP18:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_2, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_2, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_3, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    case COMPONENT_GRP19:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_3, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_3, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    case COMPONENT_GRP20:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_3, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_3, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case COMPONENT_GRP21:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_3, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_3, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case COMPONENT_GRP22:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_3, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_3, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case COMPONENT_GRP23:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_3, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_3, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_0, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_0, b_on);
        break;
    case COMPONENT_GRP24:
        if (COLOR_YUV == color_sp) {
            disp_hal_set_dac_fmt(DAC_3, DAC_Y, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_U, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_V, DISP_CHANNEL_HD);
        } else {
            disp_hal_set_dac_fmt(DAC_3, DAC_G, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_2, DAC_B, DISP_CHANNEL_HD);
            disp_hal_set_dac_fmt(DAC_1, DAC_R, DISP_CHANNEL_HD);
        }
        disp_hal_set_dac_onoff(DAC_3, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        break;
    default:
        //only support dac 123 as component
        break;
    }

    p_dp->venc_info.comp_dac[grp_id].comp_grp_id = grp_id;
    p_dp->venc_info.comp_dac[grp_id].b_on = b_on;

    return MT_SUCCESS;
}
/*!
  On or Off specific Svideo output.

  \param[in] p_disp  The handle of display module.
  \param[in] grp_id  the group id of svideo
  \param[in] b_on   TRUE for on, FALSE for off
  */
mt_s32 disp_aria_svideo_onoff(disp_priv_t *p_disp, svideo_dacgrp_t grp_id, MT_BOOL b_on)
{
    disp_priv_t *p_dp = (disp_priv_t *)p_disp;

    switch (grp_id)
    {
    case SVIDEO_GRP1:
        disp_hal_set_dac_fmt(DAC_1, DAC_LUMA, DISP_CHANNEL_SD);
        disp_hal_set_dac_fmt(DAC_2, DAC_CHROMA, DISP_CHANNEL_SD);
        disp_hal_set_dac_onoff(DAC_1, b_on);
        disp_hal_set_dac_onoff(DAC_2, b_on);
        break;
    case SVIDEO_GRP0:
    case SVIDEO_GRP2:
    case SVIDEO_GRP3:
    default:
        break;
    }

    p_dp->venc_info.svideo_dac[grp_id].svideo_grp_id = grp_id;
    p_dp->venc_info.svideo_dac[grp_id].b_on = b_on;

    return MT_SUCCESS;
}


static void disp_hal_set_sd_venc_ram(void)
{
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA0,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA1,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA2,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA3,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA4,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA5,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA6,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA7,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA8,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA9,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA10,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA11,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA12,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA13,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA14,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA15,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA16,0x00000000);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA17,0x00010101);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA18,0x00020202);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA19,0x00030303);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA20,0x00040405);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA21,0x00050506);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA22,0x00070707);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA23,0x00080808);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA24,0x00090909);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA25,0x000a0a0a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA26,0x000b0b0c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA27,0x000c0c0d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA28,0x000d0d0e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA29,0x000f0f0f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA30,0x00101010);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA31,0x00111111);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA32,0x00121213);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA33,0x00131314);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA34,0x00141415);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA35,0x00151516);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA36,0x00171717);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA37,0x00181818);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA38,0x0019191a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA39,0x001a1a1b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA40,0x001b1b1c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA41,0x001c1c1d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA42,0x001d1d1e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA43,0x001f1f1f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA44,0x00202021);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA45,0x00212122);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA46,0x00222223);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA47,0x00232324);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA48,0x00242425);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA49,0x00252526);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA50,0x00272728);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA51,0x00282829);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA52,0x0029292a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA53,0x002a2a2b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA54,0x002b2b2c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA55,0x002c2c2d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA56,0x002d2d2f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA57,0x002f2f30);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA58,0x00303031);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA59,0x00313132);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA60,0x00323233);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA61,0x00333334);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA62,0x00343436);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA63,0x00363637);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA64,0x00373738);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA65,0x00383839);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA66,0x0039393a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA67,0x003a3a3b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA68,0x003b3b3d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA69,0x003c3c3e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA70,0x003e3e3f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA71,0x003f3f40);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA72,0x00404041);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA73,0x00414142);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA74,0x00424243);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA75,0x00434345);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA76,0x00444446);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA77,0x00464647);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA78,0x00474748);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA79,0x00484849);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA80,0x0049494a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA81,0x004a4a4c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA82,0x004b4b4d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA83,0x004c4c4e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA84,0x004e4e4f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA85,0x004f4f50);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA86,0x00505051);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA87,0x00515153);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA88,0x00525254);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA89,0x00535355);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA90,0x00545456);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA91,0x00565657);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA92,0x00575758);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA93,0x0058585a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA94,0x0059595b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA95,0x005a5a5c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA96,0x005b5b5d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA97,0x005c5c5e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA98,0x005f5f5f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA99,0x00606061);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA100,0x00616162);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA101,0x00626263);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA102,0x00636364);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA103,0x00646465);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA104,0x00656566);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA105,0x00676768);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA106,0x00686869);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA107,0x0069696a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA108,0x006a6a6b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA109,0x006b6b6c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA110,0x006c6c6d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA111,0x006e6e6f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA112,0x006f6f70);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA113,0x00707071);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA114,0x00707072);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA115,0x00717173);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA116,0x00727274);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA117,0x00737376);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA118,0x00757577);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA119,0x00767678);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA120,0x00777779);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA121,0x0078787a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA122,0x0079797b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA123,0x007a7a7d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA124,0x007b7b7e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA125,0x007d7d7f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA126,0x007e7e80);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA127,0x007f7f81);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA128,0x00808082);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA129,0x00818183);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA130,0x00828285);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA131,0x00838386);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA132,0x00858587);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA133,0x00868688);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA134,0x00878789);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA135,0x0088888a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA136,0x0089898c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA137,0x008a8a8d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA138,0x008b8b8e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA139,0x008d8d8f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA140,0x008e8e90);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA141,0x008f8f91);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA142,0x00909093);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA143,0x00919194);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA144,0x00919195);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA145,0x00929296);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA146,0x00949497);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA147,0x00959598);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA148,0x0096969a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA149,0x0097979b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA150,0x0098989c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA151,0x0099999d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA152,0x009b9b9e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA153,0x009c9c9f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA154,0x009d9da1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA155,0x009e9ea2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA156,0x009f9fa3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA157,0x00a0a0a4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA158,0x00a1a1a5);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA159,0x00a3a3a6);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA160,0x00a5a5a8);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA161,0x00a6a6a9);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA162,0x00a7a7aa);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA163,0x00a8a8ab);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA164,0x00a9a9ac);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA165,0x00aaaaad);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA166,0x00acacaf);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA167,0x00adadb0);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA168,0x00aeaeb1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA169,0x00afafb2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA170,0x00b0b0b3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA171,0x00b1b1b4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA172,0x00b2b2b6);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA173,0x00b4b4b7);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA174,0x00b5b5b8);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA175,0x00b6b6b9);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA176,0x00b7b7ba);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA177,0x00b8b8bb);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA178,0x00b9b9bd);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA179,0x00bababe);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA180,0x00bcbcbf);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA181,0x00bdbdc0);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA182,0x00bebec1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA183,0x00bfbfc2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA184,0x00c0c0c3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA185,0x00c1c1c5);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA186,0x00c2c2c6);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA187,0x00c4c4c7);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA188,0x00c5c5c8);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA189,0x00c6c6c9);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA190,0x00c7c7ca);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA191,0x00c8c8cc);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA192,0x00c9c9cd);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA193,0x00cacace);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA194,0x00cccccf);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA195,0x00cdcdd0);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA196,0x00ceced1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA197,0x00cfcfd3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA198,0x00d0d0d4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA199,0x00d1d1d5);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA200,0x00d3d3d6);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA201,0x00d4d4d7);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA202,0x00d5d5d8);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA203,0x00d6d6da);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA204,0x00d7d7db);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA205,0x00d8d8dc);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA206,0x00d9d9dd);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA207,0x00dbdbde);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA208,0x00dcdcdf);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA209,0x00dddde1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA210,0x00dedee2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA211,0x00dfdfe3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA212,0x00e0e0e4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA213,0x00e1e1e5);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA214,0x00e3e3e6);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA215,0x00e4e4e8);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA216,0x00e5e5e9);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA217,0x00e6e6ea);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA218,0x00e7e7eb);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA219,0x00e8e8ec);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA220,0x00e9e9ed);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA221,0x00ebebef);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA222,0x00ececf0);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA223,0x00ededf1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA224,0x00eeeef2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA225,0x00efeff3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA226,0x00f0f0f4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA227,0x00f1f1f6);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA228,0x00f3f3f7);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA229,0x00f4f4f8);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA230,0x00f5f5f9);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA231,0x00f6f6fa);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA232,0x00f7f7fb);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA233,0x00f8f8fd);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA234,0x00f9f9fe);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA235,0x00fbfbff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA236,0x00fcfcff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA237,0x00fdfdff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA238,0x00fefeff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA239,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA240,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA241,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA242,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA243,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA244,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA245,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA246,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA247,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA248,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA249,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA250,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA251,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA252,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA253,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA254,0x00ffffff);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA255,0x00ffffff);

    return;
}

static void disp_hal_set_sd_venc_ram_for_pal(void)
{
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA53,0x002a2a2b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA54,0x002b2b2c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA55,0x002c2c2d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA56,0x002d2d2f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA57,0x002f2f30);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA58,0x00303031);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA59,0x00313132);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA60,0x00323233);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA61,0x00333334);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA62,0x00343436);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA63,0x00363637);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA64,0x00373738);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA65,0x00383839);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA66,0x0039393a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA67,0x003a3a3b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA68,0x003b3b3d);

    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA98,0x005f5f5f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA99,0x00606061);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA100,0x00616162);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA101,0x00626263);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA102,0x00636364);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA103,0x00646465);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA104,0x00656566);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA105,0x00676768);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA106,0x00686869);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA107,0x0069696a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA108,0x006a6a6b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA109,0x006b6b6c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA110,0x006c6c6d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA111,0x006e6e6f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA112,0x006f6f70);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA113,0x00707071);

    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA128, 0x00808082);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA129, 0x00818183);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA130, 0x00828285);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA131, 0x00838386);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA132, 0x00858587);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA133, 0x00868688);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA134, 0x00878789);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA135, 0x0088888A);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA136, 0x0089898C);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA137, 0x008A8A8D);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA138, 0x008B8B8E);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA139, 0x00908E90);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA140, 0x00908E90);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA141, 0x00918F91);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA142, 0x00929093);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA143, 0x00939194);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA144, 0x00939195);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA145, 0x00949296);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA146, 0x00959497);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA147, 0x00979598);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA148, 0x0098969A);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA149, 0x0099979B);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA150, 0x0099989C);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA151, 0x0099989C);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA152, 0x009d9B9E);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA153, 0x009e9C9F);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA154, 0x009D9DA1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA155, 0x009E9EA2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA156, 0x009F9FA3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA157, 0x00A0A0A4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA158, 0x00A1A1A5);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA159, 0x00A3A3A6);
    return;
}

static void disp_hal_set_sd_venc_ram_for_ntsc(void)
{
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA53,0x002a2a2c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA54,0x002b2b2d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA55,0x002c2c2e);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA56,0x002d2d30);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA57,0x002f2f31);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA58,0x00303032);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA59,0x00313133);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA60,0x00323234);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA61,0x00333335);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA62,0x00343437);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA63,0x00363638);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA64,0x00373739);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA65,0x0038383a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA66,0x0039393b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA67,0x003a3a3c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA68,0x003b3b3e);

    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA98,0x005d5d5f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA99,0x005e5e61);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA100,0x005f5f62);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA101,0x00606063);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA102,0x00616164);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA103,0x00626265);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA104,0x00636366);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA105,0x00656568);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA106,0x00666669);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA107,0x0067676a);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA108,0x0068686b);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA109,0x0069696c);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA110,0x006a6a6d);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA111,0x006c6c6f);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA112,0x006d6d70);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA113,0x006e6e71);

    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA128,0x00808083);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA129,0x00818184);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA130,0x00828286);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA131,0x00838387);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA132,0x00858588);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA133,0x00868689);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA134,0x0087878A);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA135,0x0088888B);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA136,0x0089898D);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA137,0x008A8A8E);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA138,0x008B8B8F);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA139,0x008D8D90);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA140,0x008E8E91);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA141,0x008F8F92);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA142,0x00909094);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA143,0x00919195);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA144,0x00929395);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA145,0x00939496);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA146,0x00959697);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA147,0x00969798);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA148,0x0097989A);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA149,0x0098999B);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA150,0x00999A9C);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA151,0x009A9B9D);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA152,0x009C9D9E);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA153,0x009D9E9F);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA154,0x009E9FA1);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA155,0x009FA0A2);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA156,0x00A0A1A3);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA157,0x00A1A2A4);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA158,0x00A2A3A5);
    disp_hal_put_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VID_RAM_DATA159,0x00A4A5A6);
    return;
}

static int get_vdac_fsr_calibration_data(void)
{
    mt_u32 reg_val = disp_hal_get_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF313FD4));
    int v1_code = (reg_val >> 8) & 0xFF;
    int v3_code = (reg_val >> 16) & 0xFF;
    int v1_ref = 920;
    int v3_ref = 1160;
    int v1_test = v1_ref + v1_code;
    int v3_test = v3_ref + v3_code;
    int r_contact = 1;
    int r_series = 51;
    int r_load = 75;
    int v_target = 1200000;

    int v1_actual = v1_test * (r_series + r_load + r_contact) * 1000 / (r_series + r_load);
    int v3_actual = v3_test * (r_series + r_load + r_contact) * 1000 / (r_series + r_load);
    int code_step = (v3_actual - v1_actual) / (0x5c - 0x48);

    int vdac_code = ((v_target - v1_actual) * 10 / code_step + 5) / 10;

    DISP_DEBUGK("%s %d %x, %x, %x, %d \n", __FUNCTION__, __LINE__,
            reg_val, v1_code, v3_code, vdac_code);

    return vdac_code;
}
void disp_hal_set_sd_venc(disp_priv_t *p_dp)
{
    disp_sys_t tv_sys_sd = p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt;
    vdac_type_t dacMode = DISP_DAC_TYPE_BUTT;
    mt_u32 dtmp = 0;
    mt_u32 reg_val = 0;
    //	u32 dtmp = 0;
    mt_u32 vdac_reg0 = 0;
    mt_u32 vdac_cali = 0;

    unsigned long chip_rev;
    package_id_symphony_t chip_pkg;

    chip_rev = symphony_get_chip_rev();
    chip_pkg = chip_package_get();
    dacMode = p_dp->dacMode;

    reg_symphony_sd_encoder_set_coef11(0x10206); //0x80
    reg_symphony_sd_encoder_set_coef10(0x2040205); //0x84
    reg_symphony_sd_encoder_set_coef9(0x2010004); //0x88
    reg_symphony_sd_encoder_set_coef8(0x80012); //0x8c
    reg_symphony_sd_encoder_set_coef7(0x2030013); //0x90
    reg_symphony_sd_encoder_set_coef6(0x20f0207); //0x94
    reg_symphony_sd_encoder_set_coef5(0xe022d); //0x98
    reg_symphony_sd_encoder_set_coef4(0x170235); //0x9c
    reg_symphony_sd_encoder_set_coef3(0x0229000a); //0xa0
    reg_symphony_sd_encoder_set_coef2(0x21d008c); //0xa4
    reg_symphony_sd_encoder_set_coef1(0x9f0111); //0xa8
    reg_symphony_sd_encoder_set_coef0(0x1200148); //0xac

    if((tv_sys_sd == VID_SYS_NTSC_J) || (tv_sys_sd == VID_SYS_NTSC_M)
            || (tv_sys_sd == VID_SYS_NTSC_443) || (tv_sys_sd == VID_SYS_PAL_M))
    {

        //analog config
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10), 0xb32ae0);

        reg_symphony_sd_encoder_set_dac_offset_dac0_offset(0x80); //0xe4
        reg_symphony_sd_encoder_set_dac123_para(0x41fb85dc); //0xe8
        reg_symphony_sd_encoder_set_instm(0xf0011b); //0x68
        if(tv_sys_sd == VID_SYS_NTSC_443)
        {
            reg_symphony_sd_encoder_set_vdac_ncarry(0xa8262b2); //0x1c
            reg_symphony_sd_encoder_set_nincrement(0xc92e3da1); //0x5c
        }
        else
        {
            reg_symphony_sd_encoder_set_vdac_ncarry(0x87c1f07); //0x1c
            reg_symphony_sd_encoder_set_nincrement(0x80000000); //0x5c
        }
        //      if(tv_sys_sd == VID_SYS_PAL_M)
        //          reg_symphony_sd_encoder_set_dacnum(0x0); //0x64
        //      else
        //          reg_symphony_sd_encoder_set_dacnum(0x1); //0x64
        reg_symphony_sd_encoder_set_set1(0x16);  //0x70
        reg_symphony_sd_encoder_set_lum_dly_108m(0x1150820); //0x7c
        reg_symphony_sd_encoder_set_compress(0x9a9090); //0x6c
        reg_symphony_sd_encoder_set_gcontrol(0x062b062b); //0x60
        reg_symphony_sd_encoder_set_delay(0x431212d); //0x08
        reg_symphony_sd_encoder_set_cfig1(0x16a0105); //0x20
        reg_symphony_sd_encoder_set_cfig2(0x7d012b); //0x24
        reg_symphony_sd_encoder_set_cfig3(0x13d00e3); //0x28
        reg_symphony_sd_encoder_set_cfig4(0x6d0104); //0x2c
        reg_symphony_sd_encoder_set_curve(0x13b0004); //0x04
        reg_symphony_sd_encoder_set_blank_p(0x118012e); //0x0c
        reg_symphony_sd_encoder_set_blank_n(0x1460118); //0x10
        reg_symphony_sd_encoder_set_cfig7(0x0); //0x38

        if(PP_MODE_STANDARD == p_dp->pp_mode)
        {
            /*
               if((disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG) & 0x1) == 0)   //macv disabled
               reg_symphony_sd_encoder_set_blank_n(0x135010a); //0x10
               else
               reg_symphony_sd_encoder_set_blank_n(0x1060114); //0x10
               */
            reg_symphony_sd_encoder_set_set(0x10011); //0x3c
            /*reg_symphony_sd_encoder_set_curve_over_sample_mode(0);//reg_symphony_sd_encoder_set_curve(0x13b0000); //0x04*/
            reg_symphony_sd_encoder_set_dacnum(0x10000); //0x64
            reg_symphony_sd_encoder_set_set2(0x3701); //0x74
        }
        else
        {
            /*
               if((disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG) & 0x1) == 0)   //macv disabled
               reg_symphony_sd_encoder_set_blank_n(0x135010b); //0x10
               else
               reg_symphony_sd_encoder_set_blank_n(0x1060114); //0x10
               */
            reg_symphony_sd_encoder_set_set(0x10011); //0x3c
            /*reg_symphony_sd_encoder_set_curve_over_sample_mode(1);//reg_symphony_sd_encoder_set_curve(0x13b1000); //0x04*/
            reg_symphony_sd_encoder_set_dacnum(0x10000); //0x64
            reg_symphony_sd_encoder_set_set2(0x3701); //0x74
        }

        reg_symphony_sd_encoder_set_dac0_para(0x11bb83dc); //0x40
        reg_symphony_sd_encoder_set_dac0_anti_coef1_0(0x1f000c0); //0xf0
        reg_symphony_sd_encoder_set_dac0_anti_coef3_2(0x1ff0004); //0xf4

        //be comment out for sym4
        /*
           if((chip_rev >= CHIP_SYMPHONY_A1 && chip_pkg == PACKET_CHIP_SIP68S_DDR2)) //Package ID=2/3
           {
           if(PP_MODE_STANDARD == p_dp->pp_mode)
           {
           reg_symphony_sd_encoder_set_cfig3(0x15f0120); //0x28
           reg_symphony_sd_encoder_set_set(0x10011); //0x3c
           reg_symphony_sd_encoder_set_blank_n(0x1470123); //0x10
           reg_symphony_sd_encoder_set_curve_over_sample_mode(1);//reg_symphony_sd_encoder_set_curve(0x13b1000); //0x04
           reg_symphony_sd_encoder_set_dac123_anti_coef1_0(0x1f000c0); //0xf8
           reg_symphony_sd_encoder_set_dac123_anti_coef3_2(0x1ff0004); //0xfc
           reg_symphony_sd_encoder_set_gcontrol(0x062b062b); //0x60

           reg_symphony_sd_encoder_set_dac123_para(0x41fb85dc); //0xe8
           reg_symphony_sd_encoder_set_set2(0x2801); //0x74
           reg_symphony_sd_encoder_set_cfig4(0x73011c); //0x2c

           }
           else
           {
           reg_symphony_sd_encoder_set_cfig3(0x14f0109); //0x28
           reg_symphony_sd_encoder_set_set(0x10011); //0x3c
           reg_symphony_sd_encoder_set_set2(0x4101); //0x74
           reg_symphony_sd_encoder_set_cfig4(0x70011c); //0x2c
           reg_symphony_sd_encoder_set_dac123_para(0x41fb85dc); //0xe8
           reg_symphony_sd_encoder_set_blank_n(0x1470123); //0x10
           reg_symphony_sd_encoder_set_curve_over_sample_mode(1);//reg_symphony_sd_encoder_set_curve(0x13b1000); //0x04
           reg_symphony_sd_encoder_set_dac123_anti_coef1_0(0x1f000c0); //0xf8
           reg_symphony_sd_encoder_set_dac123_anti_coef3_2(0x1ff0004); //0xfc
           reg_symphony_sd_encoder_set_gcontrol(0x062b062b); //0x60
           }

           }
           if((chip_rev >= CHIP_SYMPHONY_A1 && chip_pkg == PACKET_CHIP_SIP68C_DDR3) ||
           (chip_rev >= CHIP_SYMPHONY_A1 && chip_pkg == PACKET_CHIP_SIP68C_DDR2)) //Package ID=0/1
           {
           reg_symphony_sd_encoder_set_curve_over_sample_mode(1);//reg_symphony_sd_encoder_set_curve(0x13b1000); //0x04
           reg_symphony_sd_encoder_set_cfig4(0x730138); //0x2c
           reg_symphony_sd_encoder_set_gcontrol(0x162b07ff); //0x60
           reg_symphony_sd_encoder_set_dac0_anti_coef1_0(0x1e200f6); //0xf0
           reg_symphony_sd_encoder_set_dac0_anti_coef3_2(0x1f601e5); //0xf4
           reg_symphony_sd_encoder_set_blank_n(0x13c010d); //0x10
           reg_symphony_sd_encoder_set_lum_dly_108m(0x1001); //0x7c
           reg_symphony_sd_encoder_set_cfig3(0x15f0118); //0x28
           }
           if(tv_sys_sd == VID_SYS_PAL_M)
           {
           reg_symphony_sd_encoder_set_cfig2(0x780108); //0x24
           if((disp_hal_get_u32((volatile mt_u32 *)REG_SYMPHONY_SD_ENCODER_VBI_MACV_CFG) & 0x1) == 0)   //macv disabled
           reg_symphony_sd_encoder_set_blank_p(0x1400118); //0x0c
           else
           reg_symphony_sd_encoder_set_blank_p(0x1040118); //0x0c
           }
           */
        disp_hal_set_sd_venc_ram_for_ntsc();

        reg_val = disp_hal_get_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF313FD4));
        if(reg_val & (1 << 6))
        {
            vdac_reg0 = disp_hal_get_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF5D0010));
            vdac_cali = get_vdac_fsr_calibration_data();
            vdac_reg0 &= ~(0x7F);
            vdac_reg0 |= 0x51 + vdac_cali;
            disp_hal_put_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF5D0010), vdac_reg0);
        }

    }
    else   //PAL
    {
        //analog config
        disp_hal_put_u32((volatile mt_u32 *)(SYMPHONY_ANALOG_SW_BASE + 0x10), 0xb32ad7);

        //symphony will mod later
        reg_symphony_sd_encoder_set_dac_offset_dac0_offset(0x80); //0xe4
        reg_symphony_sd_encoder_set_dac123_para(0x41fb85dc); //0xe8
        reg_symphony_sd_encoder_set_set(0x00000011); //0x3c
        reg_symphony_sd_encoder_set_set1(0x16);  //0x70
        reg_symphony_sd_encoder_set_lum_dly_108m(0x1260a00); //0x7c
        reg_symphony_sd_encoder_set_compress(0x9f9595); //0x6c
        reg_symphony_sd_encoder_set_curve(0x13b0004); //0x04
        reg_symphony_sd_encoder_set_cfig7(0x0); //0x38
        reg_symphony_sd_encoder_set_dac0_para(0x11bb83dc); //0x40
        reg_symphony_sd_encoder_set_gcontrol(0x062b062b); //0x60
        if(PP_MODE_STANDARD == p_dp->pp_mode)
        {
            reg_symphony_sd_encoder_set_delay(0x431212f); //0x08
            reg_symphony_sd_encoder_set_cfig1(0x16a0105); //0x20
            reg_symphony_sd_encoder_set_cfig2(0x7c012c); //0x24
            reg_symphony_sd_encoder_set_cfig3(0x13d00e3); //0x28
            reg_symphony_sd_encoder_set_cfig4(0x6d0104); //0x2c
            /*reg_symphony_sd_encoder_set_curve_over_sample_mode(0);//reg_symphony_sd_encoder_set_curve(0x13b0000); //0x04*/
            /*reg_symphony_sd_encoder_set_cfig7(0x0); //0x38*/
            reg_symphony_sd_encoder_set_instm(0x00f0011b); //0x68
            reg_symphony_sd_encoder_set_dacnum(0x00010000); //0x64
            reg_symphony_sd_encoder_set_dac0_anti_coef1_0(0x1f000c0); //0xf0
            reg_symphony_sd_encoder_set_dac0_anti_coef3_2(0x1ff0004); //0xf4

        }
        else
        {
            reg_symphony_sd_encoder_set_delay(0x431212f); //0x08
            reg_symphony_sd_encoder_set_cfig1(0x16a0105); //0x20
            reg_symphony_sd_encoder_set_cfig2(0x7d012b); //0x24
            reg_symphony_sd_encoder_set_cfig3(0x13d00e3); //0x28
            reg_symphony_sd_encoder_set_cfig4(0x6d0104); //0x2c
            /*reg_symphony_sd_encoder_set_curve_over_sample_mode(1);//reg_symphony_sd_encoder_set_curve(0x13b1000); //0x04*/
            /*
               if(p_dp->b_cvbs_smooth_on == TRUE)
               {
               reg_symphony_sd_encoder_set_cfig7(0x0); //0x38
               }
               else
               {
               reg_symphony_sd_encoder_set_cfig7(0xf); //0x38
               }
               */
            reg_symphony_sd_encoder_set_instm(0x00f0011b); //0x68
            reg_symphony_sd_encoder_set_dacnum(0x00010000); //0x64
            reg_symphony_sd_encoder_set_dac0_anti_coef1_0(0x1e200f6); //0xf0
            reg_symphony_sd_encoder_set_dac0_anti_coef3_2(0x1f601e5); //0xf4
        }
        /*reg_symphony_sd_encoder_set_cfig3(0x014600e8); //0x28*/
        /*reg_symphony_sd_encoder_set_cfig4(0x730114); //0x2c*/
        reg_symphony_sd_encoder_set_blank_p(0x118012e); //0x0c
        reg_symphony_sd_encoder_set_blank_n(0x1460118); //0x10

        if(tv_sys_sd == VID_SYS_PAL)
        {
            reg_symphony_sd_encoder_set_vdac_pcarry(0xa8262b2); //0x18
            reg_symphony_sd_encoder_set_pincrement(0xc068db8c); //0x58
            dtmp = reg_symphony_sd_encoder_get_set();
            dtmp = dtmp & 0xfffffffc;
            dtmp = dtmp | 0x1;
            reg_symphony_sd_encoder_set_set(dtmp); //0x3c
        }
        else if((tv_sys_sd == VID_SYS_PAL_N) || (tv_sys_sd == VID_SYS_PAL_NC) )
        {
            reg_symphony_sd_encoder_set_vdac_pcarry(0x87da512); //0x18
            reg_symphony_sd_encoder_set_pincrement(0x4068db8c); //0x58
            dtmp = reg_symphony_sd_encoder_get_set();
            dtmp = dtmp & 0xfffffffc;
            dtmp = dtmp | 0x2;
            reg_symphony_sd_encoder_set_set(dtmp); //0x3c
        }

        disp_hal_set_sd_venc_ram_for_pal();

        reg_val = disp_hal_get_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF313FD4));
        if(reg_val & (1 << 6))
        {
            vdac_reg0 = disp_hal_get_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF5D0010));
            vdac_cali = get_vdac_fsr_calibration_data();
            vdac_reg0 &= ~(0x7F);
            vdac_reg0 |= 0x48 + vdac_cali;
            disp_hal_put_u32((volatile mt_u32 *)SYMPHONY_IO_VA(0xBF5D0010), vdac_reg0);
        }
    }
    //cvbs chroma delay
}


mt_u32 disp_hal_set_sd_venc_reg(DISP_SD_ENC_PQ_PARA_S* p_pq_para)
{
    switch(p_pq_para->item)
    {
    case    DISP_SD_ENC_MODE:
        {
            reg_symphony_sd_encoder_set_mode(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CURVE:
        {
            reg_symphony_sd_encoder_set_curve(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DELAY:
        {
            reg_symphony_sd_encoder_set_delay(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_BLANK_P:
        {
            reg_symphony_sd_encoder_set_blank_p(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_BLANK_N:
        {
            reg_symphony_sd_encoder_set_blank_n(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CBLANK:
        {
            reg_symphony_sd_encoder_set_cblank(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_VDAC_PCARRY:
        {
            reg_symphony_sd_encoder_set_vdac_pcarry(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_VDAC_NCARRY:
        {
            reg_symphony_sd_encoder_set_vdac_ncarry(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG1:
        {
            reg_symphony_sd_encoder_set_cfig1(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG2:
        {
            reg_symphony_sd_encoder_set_cfig2(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG3:
        {
            reg_symphony_sd_encoder_set_cfig3(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG4:
        {
            reg_symphony_sd_encoder_set_cfig4(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG5:
        {
            reg_symphony_sd_encoder_set_cfig5(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG6:
        {
            reg_symphony_sd_encoder_set_cfig6(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG7:
        {
            reg_symphony_sd_encoder_set_cfig7(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SET:
        {
            reg_symphony_sd_encoder_set_set(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC0_PARA:
        {
            reg_symphony_sd_encoder_set_dac0_para(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DRCOEF:
        {
            reg_symphony_sd_encoder_set_drcoef(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DBCOEF:
        {
            reg_symphony_sd_encoder_set_dbcoef(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_CFIG8:
        {
            reg_symphony_sd_encoder_set_cfig8(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SCARRYDR:
        {
            reg_symphony_sd_encoder_set_scarrydr(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SCARRYDB:
        {
            reg_symphony_sd_encoder_set_scarrydb(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_PINCREMENT:
        {
            reg_symphony_sd_encoder_set_pincrement(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_NINCREMENT:
        {
            reg_symphony_sd_encoder_set_nincrement(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_GCONTROL:
        {
            reg_symphony_sd_encoder_set_gcontrol(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DACNUM:
        {
            reg_symphony_sd_encoder_set_dacnum(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_INSTM:
        {
            reg_symphony_sd_encoder_set_instm(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COMPRESS:
        {
            reg_symphony_sd_encoder_set_compress(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SET1:
        {
            reg_symphony_sd_encoder_set_set1(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SET2:
        {
            reg_symphony_sd_encoder_set_set2(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SET3:
        {
            reg_symphony_sd_encoder_set_set3(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_LUM_DLY_108M:
        {
            reg_symphony_sd_encoder_set_lum_dly_108m(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF11:
        {
            reg_symphony_sd_encoder_set_coef11(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF10:
        {
            reg_symphony_sd_encoder_set_coef10(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF9:
        {
            reg_symphony_sd_encoder_set_coef9(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF8:
        {
            reg_symphony_sd_encoder_set_coef8(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF7:
        {
            reg_symphony_sd_encoder_set_coef7(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF6:
        {
            reg_symphony_sd_encoder_set_coef6(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF5:
        {
            reg_symphony_sd_encoder_set_coef5(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF4:
        {
            reg_symphony_sd_encoder_set_coef4(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF3:
        {
            reg_symphony_sd_encoder_set_coef3(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF2:
        {
            reg_symphony_sd_encoder_set_coef2(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF1:
        {
            reg_symphony_sd_encoder_set_coef1(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF0:
        {
            reg_symphony_sd_encoder_set_coef0(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT3_U:
        {
            reg_symphony_sd_encoder_set_coef_c_lut3_u(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT2_U:
        {
            reg_symphony_sd_encoder_set_coef_c_lut2_u(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT1_U:
        {
            reg_symphony_sd_encoder_set_coef_c_lut1_u(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT0_U:
        {
            reg_symphony_sd_encoder_set_coef_c_lut0_u(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT3_V:
        {
            reg_symphony_sd_encoder_set_coef_c_lut3_v(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT2_V:
        {
            reg_symphony_sd_encoder_set_coef_c_lut2_v(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT1_V:
        {
            reg_symphony_sd_encoder_set_coef_c_lut1_v(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_C_LUT0_V:
        {
            reg_symphony_sd_encoder_set_coef_c_lut0_v(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_LUT3_Y:
        {
            reg_symphony_sd_encoder_set_coef_lut3_y(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_LUT2_Y:
        {
            reg_symphony_sd_encoder_set_coef_lut2_y(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_LUT1_Y:
        {
            reg_symphony_sd_encoder_set_coef_lut1_y(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_COEF_LUT0_Y:
        {
            reg_symphony_sd_encoder_set_coef_lut0_y(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_SIGN_CTL:
        {
            reg_symphony_sd_encoder_set_sign_ctl(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC_OFFSET:
        {
            reg_symphony_sd_encoder_set_dac_offset(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC123_PARA:
        {
            reg_symphony_sd_encoder_set_dac123_para(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC0_ANTI_COEF1_0:
        {
            reg_symphony_sd_encoder_set_dac0_anti_coef1_0(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC0_ANTI_COEF3_2:
        {
            reg_symphony_sd_encoder_set_dac0_anti_coef3_2(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC123_ANTI_COEF1_0:
        {
            reg_symphony_sd_encoder_set_dac123_anti_coef1_0(p_pq_para->val);
            break;
        }
    case    DISP_SD_ENC_DAC123_ANTI_COEF3_2:
        {
            reg_symphony_sd_encoder_set_dac123_anti_coef3_2(p_pq_para->val);
            break;
        }

    default:
        DISP_DEBUGK("unsupported SD ENC PQ cmd! [%s] \n", __FUNCTION__);

    }
    return MT_SUCCESS;
}

mt_void disp_set_dacmode(MT_DRV_DISPLAY_E enDisp, vdac_type_t dacMode)
{
    DISP_S *pstDisp;
    disp_priv_t *p_disp;
    DispGetPointerByIDNoReturn(enDisp, pstDisp);
    p_disp = (disp_priv_t *)pstDisp->p_dp;
    p_disp->dacMode = dacMode;

    disp_aria_cvbs_onoff(p_disp, CVBS_GRP0, MT_TRUE);
    return;
}

static MT_DRV_DISP_FMT_E get_tvsys_from_reg(mt_u32 ch)
{
    mt_u32 dtmp = 0;
    
    mt_u32 b_1001_enable = 0;  //clock div 1001 for 59.94/29.97
    mt_u32 i_flag = 0;     //0:progressive,1:interlace
    mt_u32 frame_rate = 0; // 1:24, 2:25, 3:30, 4:50, 5:60
    mt_u32 resolution = 0; //0:1920*1080,1:1280*720,2:720*576,3:720*480,4:3840x2160,5:4096x2160
    MT_DRV_DISP_FMT_E tvsys = MT_DRV_DISP_FMT_1080i_50;
    
    if (ch == DISP_CHANNEL_SD)
    {
        dtmp = reg_aria_sd_enc_get_cfg6_video_fmt();
        if (dtmp == 2)
            tvsys = MT_DRV_DISP_FMT_NTSC_J;
        else if (dtmp == 6)
            tvsys = MT_DRV_DISP_FMT_NTSC_443;
        else if (dtmp == 1)
            tvsys = MT_DRV_DISP_FMT_PAL;
        else if (dtmp == 4)
            tvsys = MT_DRV_DISP_FMT_PAL_M;
        else if (dtmp == 5)
            tvsys = MT_DRV_DISP_FMT_PAL_N;
        else if (dtmp == 3)
            tvsys = MT_DRV_DISP_FMT_SECAM_SIN;
        else
            tvsys = MT_DRV_DISP_FMT_PAL;
    }
    else
    {
        i_flag = reg_aria_hd_encoder_get_hd_cfg_info_reg_field_mode();
        frame_rate = drv_reg_4k_disp_get_video_ctrl_2_frame_rate();
        resolution = reg_aria_hd_encoder_get_basic_cfg_resolusion();
        b_1001_enable = (HAL_GET_U32((volatile u32 *)R_INTP_CTRL_REG1) & 0x1) ? 0 : 1;

        if (resolution == 0)
        {
            if (frame_rate == 1)
                tvsys = MT_DRV_DISP_FMT_1080P_24;
            else if (frame_rate == 2)
                tvsys = MT_DRV_DISP_FMT_1080P_25;
            else if (frame_rate == 3)
            {
                tvsys = b_1001_enable ? MT_DRV_DISP_FMT_1080P_29_97 : MT_DRV_DISP_FMT_1080P_30;
                
            }
            else if (frame_rate == 4)
            {
                if (i_flag)
                    tvsys = MT_DRV_DISP_FMT_1080i_50;
                else
                    tvsys = MT_DRV_DISP_FMT_1080P_50;
            }
            else if (frame_rate == 5)
            {
                if (i_flag)
                    tvsys = b_1001_enable ? MT_DRV_DISP_FMT_1080i_59_94 : MT_DRV_DISP_FMT_1080i_60;
                else
                    tvsys = b_1001_enable ? MT_DRV_DISP_FMT_1080P_59_94 : MT_DRV_DISP_FMT_1080P_60;
            }
            else
            {
                DISP_DEBUGK("unknown frame_rate:%d\n", frame_rate);
                tvsys = MT_DRV_DISP_FMT_1080i_50;
            }
        }
        else if (resolution == 1)
        {
            if (frame_rate == 4)
            {
                tvsys = MT_DRV_DISP_FMT_720P_50;
            }
            else if (frame_rate == 5)
            {
                tvsys = b_1001_enable ? MT_DRV_DISP_FMT_720P_59_94 :  MT_DRV_DISP_FMT_720P_60;
            }
            else
            {
                DISP_DEBUGK("unknown frame_rate:%d\n", frame_rate);
                tvsys = MT_DRV_DISP_FMT_720P_50;
            }
        }
        else if (resolution == 2)
        {
            if (i_flag == 0)
                tvsys = MT_DRV_DISP_FMT_576P_50;
            else
                tvsys = MT_DRV_DISP_FMT_PAL; //576i50
        }
        else if (resolution == 3)
        {
            if (i_flag == 0)
                tvsys = MT_DRV_DISP_FMT_480P_60;
            else
                tvsys = MT_DRV_DISP_FMT_NTSC; //480i60
        }
        else if (resolution == 4)
        {
            if (frame_rate == 1)
                tvsys = MT_DRV_DISP_FMT_3840X2160_24;
            else if (frame_rate == 2)
                tvsys = MT_DRV_DISP_FMT_3840X2160_25;
            else if (frame_rate == 3)
                tvsys = b_1001_enable ? MT_DRV_DISP_FMT_3840X2160_29_97 : MT_DRV_DISP_FMT_3840X2160_30;
            else if (frame_rate == 4)
                tvsys = MT_DRV_DISP_FMT_3840X2160_50;
            else if (frame_rate == 5)
                tvsys = b_1001_enable ? MT_DRV_DISP_FMT_3840X2160_59_94 : MT_DRV_DISP_FMT_3840X2160_60;
            else
            {
                DISP_DEBUGK("unknown frame_rate:%d\n", frame_rate);
                tvsys = MT_DRV_DISP_FMT_3840X2160_25;
            }
        }
        else if (resolution == 5)
        {
            if (frame_rate == 1)
                tvsys = MT_DRV_DISP_FMT_4096X2160_24;
            else if (frame_rate == 2)
                tvsys = MT_DRV_DISP_FMT_4096X2160_25;
            else if (frame_rate == 3)
                tvsys = b_1001_enable ? MT_DRV_DISP_FMT_4096X2160_29_97 : MT_DRV_DISP_FMT_4096X2160_30;
            else if (frame_rate == 4)
                tvsys = MT_DRV_DISP_FMT_4096X2160_50;
            else if (frame_rate == 5)
                tvsys = b_1001_enable ? MT_DRV_DISP_FMT_4096X2160_59_94 : MT_DRV_DISP_FMT_4096X2160_60;
        } else
            tvsys = MT_DRV_DISP_FMT_BUTT;
        DISP_DEBUGK("i_flag:%d, frame_rate:%d, resolution:%d b_1001_enable:%d \n", i_flag, frame_rate, resolution, b_1001_enable);
    }
    DISP_DEBUGK("tvsys:%d\n", tvsys);
    return tvsys;
}

mt_void disp_st_vid_get_vdec_size(mt_u32 *p_height, mt_u32 *p_width)
{
    *p_width = g_vdec_width;
    *p_height = g_vdec_height;
}

mt_void disp_st_vid_set_vdec_size(mt_u32 height, mt_u32 width)
{
    g_vdec_width = width;
    g_vdec_height = height;
}

mt_void disp_st_vid_get_vout_size(disp_channel_t ch, mt_u32 *p_height, mt_u32 *p_width)
{
    disp_priv_t *p_dp = &s_stDisplayPriv;
    disp_sys_t out_fmt = p_dp->disp_out_info[ch].vid_fmt;

    if ((VID_SYS_NTSC_M == out_fmt) ||
            (VID_SYS_NTSC_J == out_fmt) ||
            (VID_SYS_NTSC_443 == out_fmt) ||
            (VID_SYS_PAL_M == out_fmt) ||
            //hd
            (VID_SYS_480P == out_fmt))
    {
        *p_width = DISP_VID_FULLSCR_SD_WIDTH;
        *p_height = DISP_VID_FULLSCR_NTSC_HEIGHT;
    }
    else if ((VID_SYS_PAL == out_fmt) ||
            (VID_SYS_PAL_N == out_fmt) ||
            (VID_SYS_PAL_NC == out_fmt) ||
            (VID_SYS_SECAM == out_fmt) ||
            //hd
            (VID_SYS_576P_50HZ == out_fmt))
    {
        *p_width = DISP_VID_FULLSCR_SD_WIDTH;
        *p_height = DISP_VID_FULLSCR_PAL_HEIGHT;
    }
    else if ((out_fmt >= VID_SYS_1080I) &&
            (out_fmt <= VID_SYS_1080P_50HZ))
    {
        *p_width = DISP_VID_FULLSCR_1080_WIDTH;
        *p_height = DISP_VID_FULLSCR_1080_HEIGHT;
    }
    else if ((out_fmt >= VID_SYS_720P) &&
            (out_fmt <= VID_SYS_720P_50HZ))
    {
        *p_width = DISP_VID_FULLSCR_720P_WIDTH;
        *p_height = DISP_VID_FULLSCR_720P_HEIGHT;
    }
    else if ((out_fmt >= VID_SYS_3840X2160_30HZ) &&
            (out_fmt <= VID_SYS_3840X2160_60HZ))
    {
        *p_width = DISP_VID_FULLSCR_3840_WIDTH;
        *p_height = DISP_VID_FULLSCR_2160_HEIGHT;
    }
    else if ((out_fmt >= VID_SYS_4096X2160_24HZ) &&
            (out_fmt <= VID_SYS_4096X2160_60HZ))
    {
        *p_width = DISP_VID_FULLSCR_4096_WIDTH;
        *p_height = DISP_VID_FULLSCR_2160_HEIGHT;
    }
    else
    {
        *p_width = 0;
        *p_height = 0;
        DISP_DEBUGK("[%s]vout error,line [%d]\n", __FUNCTION__, __LINE__);
    }
}


static void disp_aria_gra_scale_update(MT_DRV_DISPLAY_E enDisp)
{
    mt_s32 ret = MT_SUCCESS;
    DISP_S *pstDisp = NULL;
    disp_priv_t *p_dp = &s_stDisplayPriv;
    DISP_CAST_S *pstCast = NULL;
    rect_vsb_t sd_rect_cur = { 0 };
    rect_vsb_t hd_rect_cur = { 0 };
    rect_vsb_t cast_rect_cur = { 0 };

    static rect_vsb_t cast_rect_old = { 0 };
    static rect_vsb_t gra_sd_rect_old = { 0 };
    static rect_vsb_t gra_hd_rect_old = { 0 };

    static disp_sys_t hd_vout_tvsys_old = VID_SYS_MAX;

    static mt_u8 sd_reinterlace_en_old = 0;
    static mt_u8 hd_reinterlace_en_old = 0;
    static mt_u8 hd_field_mode_old = 0;
    mt_u8  sd_reinterlace_en = drv_reg_4k_disp_get_sd_reinterlace_ctrl_reinterlace_en() || drv_reg_4k_disp_get_sd_wr_ctrl_cfg_rate_conversion_enable();
    mt_u8  hd_reinterlace_en = drv_reg_4k_disp_get_hd_reinterlace_ctrl_reinterlace_en();
    mt_u8  hd_field_mode = drv_reg_4k_disp_get_hdenc_bot_field_hd_field_mode();


    MT_BOOL b_4k = MT_FALSE;

    // s1 check input parameters
    //DispCheckDeviceState();
    //DispCheckID(enDisp);

    // s2 get pointer
    DispGetPointerByIDNoReturn(enDisp, pstDisp);

    //get sd output size
    disp_st_vid_get_vout_size(DISP_CHANNEL_SD, &(sd_rect_cur.h), &(sd_rect_cur.w));
    //get hd output size
    disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &(hd_rect_cur.h), &(hd_rect_cur.w));

    if (hd_rect_cur.w == 3840 || hd_rect_cur.w == 4096)
    {
        b_4k = MT_TRUE;
    }

    pstCast = (DISP_CAST_S *)(pstDisp->Cast_ptr);

    if (pstCast != NULL)
    {
        if (pstCast->bEnable)
        {

            cast_rect_cur.h = pstCast->stAttr.stOut.s32Height;
            cast_rect_cur.w = pstCast->stAttr.stOut.s32Width;

            // printk("%d,%d,%d,%d\n",  pstCast->stConfig.u32Width, pstCast->stConfig.u32Height, pstCast->stAttr.stOut.s32Width,pstCast->stAttr.stOut.s32Height);
            // printk("cur[%d,%d],old[%d,%d]\n", cast_rect_old.w, cast_rect_old.h,cast_rect_cur.w, cast_rect_cur.h);

            if (cast_rect_cur.h > hd_rect_cur.h)
                cast_rect_cur.h = hd_rect_cur.h;

            if (cast_rect_cur.w > hd_rect_cur.w)
                cast_rect_cur.w = hd_rect_cur.w;
        }
    }
    else
    {
        cast_rect_cur.h = 0;
        cast_rect_cur.w = 0;
    }

    if (b_4k) //b_4k
    {
        cast_rect_cur.w = GRA_SCALE0_OUTSZ_W;
        cast_rect_cur.h = GRA_SCALE0_OUTSZ_H;
    }

    if ((hd_vout_tvsys_old != p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt)
            || (gra_hd_rect_old.w != hd_rect_cur.w)
            || (gra_hd_rect_old.h != hd_rect_cur.h)
            || (gra_sd_rect_old.w != sd_rect_cur.w)
            || (gra_sd_rect_old.h != sd_rect_cur.h)
            || (cast_rect_cur.w != cast_rect_old.w)
            || (cast_rect_cur.h != cast_rect_old.h)
            || (sd_reinterlace_en_old != sd_reinterlace_en)
            || (hd_reinterlace_en_old != hd_reinterlace_en)
            || (hd_field_mode_old != hd_field_mode)
            || b_disp_coeff_update)
    {

        DISP_DEBUGK("%s %d old hd[%d, %d],sd[%d, %d],cst[%d,%d] reinterlace[%d %d]\n", __FUNCTION__, __LINE__,
                gra_hd_rect_old.w, gra_hd_rect_old.h,
                gra_sd_rect_old.w, gra_sd_rect_old.h, 
                cast_rect_old.w, cast_rect_old.h,
                hd_reinterlace_en_old, sd_reinterlace_en_old);
        DISP_DEBUGK("%s %d hd[%d, %d],sd[%d, %d],cst[%d,%d]  reinterlace[%d %d]\n", __FUNCTION__, __LINE__,
                hd_rect_cur.w, hd_rect_cur.h, 
                sd_rect_cur.w, sd_rect_cur.h, 
                cast_rect_cur.w, cast_rect_cur.h,
                hd_reinterlace_en, sd_reinterlace_en);
				
        if(!drv_reg_4k_disp_get_video_ctrl_1_video_sel())
        {
            ret = DF_GraScaler_Update(&g_stDrvSetting[0]);
            if((ret == MT_SUCCESS) && (update_sys_flag != 1))
            {
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->vdac on! \n", __FUNCTION__, __LINE__);
                //if uboot don't show logo, DF_GraScaler_Update will fail before vofw init, can not enable vdac
                disp_update_cvbs_for_sd_tvsys(p_dp, MT_TRUE);
            }
        }
        
        gra_hd_rect_old.w = hd_rect_cur.w;
        gra_hd_rect_old.h = hd_rect_cur.h;
        gra_sd_rect_old.w = sd_rect_cur.w;
        gra_sd_rect_old.h = sd_rect_cur.h;
        cast_rect_old.w = cast_rect_cur.w;
        cast_rect_old.h = cast_rect_cur.h;
        sd_reinterlace_en_old = sd_reinterlace_en;
        hd_reinterlace_en_old = hd_reinterlace_en;
        hd_field_mode_old = hd_field_mode;

        hd_vout_tvsys_old = p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt;
    }
}

void drv_disp_get_hdmi_edid (void)
{
#ifdef CONFIG_HDMI
    disp_priv_t *p_dp = &s_stDisplayPriv;
    mt_s32 u32Ret = MT_FAILURE;
    HDMI_EDID_S edid;
    mt_u32 i = 0;
    memset(&edid, 0, sizeof(HDMI_EDID_S));
    u32Ret = DRV_HDMI_Force_GetEDID(&edid);
    if(u32Ret != MT_SUCCESS)
    {
        DISP_DEBUGK("HDMI disp: Can not get EDID\n");
        return;
    }

    if(edid.u8EdidValid == MT_TRUE)
    {
#if 0
        p_dp->hdmi_info.supported_hdr10p_emp = edid.stEdidParsed.hdr10p_emp;
        p_dp->hdmi_info.supported_hdr10p_vsif = edid.stEdidParsed.supported_hdr10p_vsif;
        p_dp->hdmi_info.supported_hdr10 = edid.stEdidParsed.supported_hdr10;
        p_dp->hdmi_info.supported_hlg = edid.stEdidParsed.supported_hlg;
        p_dp->hdmi_info.rgb30bit = edid.stEdidParsed.rgb30bit;
        p_dp->hdmi_info.rgb36bit = edid.stEdidParsed.rgb36bit;

        p_dp->hdmi_info.supported_3840x2160p_50Hz = edid.stEdidParsed.supported_3840x2160p_50Hz;
        p_dp->hdmi_info.supported_3840x2160p_60Hz = edid.stEdidParsed.supported_3840x2160p_60Hz;
        p_dp->hdmi_info.supported_4096x2160p_50Hz = edid.stEdidParsed.supported_4096x2160p_50Hz;
        p_dp->hdmi_info.supported_4096x2160p_60Hz = edid.stEdidParsed.supported_4096x2160p_60Hz;

        if((edid.stEdidParsed.supported_bt2020cycc == 1)
                || (edid.stEdidParsed.supported_bt2020ycc == 1)
                || (edid.stEdidParsed.supported_bt2020rgb == 1))
        {
            p_dp->hdmi_info.supported_bt2020 = TRUE;
        }
        DISP_DEBUGK("disp get edid: \n");
        DISP_DEBUGK("supported_hdr10p_emp: %d\n", p_dp->hdmi_info.supported_hdr10p_emp);
        DISP_DEBUGK("supported_hdr10p_vsif: %d\n", p_dp->hdmi_info.supported_hdr10p_vsif);
        DISP_DEBUGK("supported_hdr10: %d\n", p_dp->hdmi_info.supported_hdr10);
        DISP_DEBUGK("supported_hlg: %d\n", p_dp->hdmi_info.supported_hlg);
        DISP_DEBUGK("supported_30bit_pixel: %d\n", p_dp->hdmi_info.supported_30bit_pixel);
        DISP_DEBUGK("supported_36bit_pixel: %d\n", p_dp->hdmi_info.supported_36bit_pixel);
        DISP_DEBUGK("supported_bt2020: %d\n", p_dp->hdmi_info.supported_bt2020);

        DISP_DEBUGK("supported_3840x2160p_50Hz: %d\n", p_dp->hdmi_info.supported_3840x2160p_50Hz);
        DISP_DEBUGK("supported_3840x2160p_60Hz: %d\n", p_dp->hdmi_info.supported_3840x2160p_60Hz);
        DISP_DEBUGK("supported_4096x2160p_50Hz: %d\n", p_dp->hdmi_info.supported_4096x2160p_50Hz);
        DISP_DEBUGK("supported_4096x2160p_60Hz: %d\n", p_dp->hdmi_info.supported_4096x2160p_60Hz);
#else
        p_dp->hdmi_info.edid = edid.stEdidParsed;
        if((edid.stEdidParsed.supported_bt2020cycc == 1)
                || (edid.stEdidParsed.supported_bt2020ycc == 1)
                || (edid.stEdidParsed.supported_bt2020rgb == 1))
        {
            p_dp->hdmi_info.supported_bt2020 = TRUE;
        }
        else
        {
            p_dp->hdmi_info.supported_bt2020 = FALSE;
        }

        p_dp->hdmi_info.brightness_max = edid.stEdidParsed.maxlum;

        for(i = 0; i < MAX_WIN_NUM; i ++)
        {     
                g_stDrvSetting[i].hdr_info_from_drv.display_brightness_max = p_dp->hdmi_info.brightness_max;
        }                

        DISP_DEBUGK("disp get edid: \n");
        DISP_DEBUGK("brightness_max: %d, EdidParsed.maxlum: %d\n", p_dp->hdmi_info.brightness_max, edid.stEdidParsed.maxlum);       
        DISP_DEBUGK("supported_hdr10p_emp: %d\n", p_dp->hdmi_info.edid.hdr10p_emp);
        DISP_DEBUGK("supported_hdr10p_vsif: %d\n", p_dp->hdmi_info.edid.supported_hdr10p_vsif);
        DISP_DEBUGK("supported_hdr10: %d\n", p_dp->hdmi_info.edid.supported_hdr10);
        DISP_DEBUGK("supported_hlg: %d\n", p_dp->hdmi_info.edid.supported_hlg);

        DISP_DEBUGK("supported_bt2020: %d\n", p_dp->hdmi_info.supported_bt2020);
        DISP_DEBUGK("supported_bt2020cycc: %d\n", p_dp->hdmi_info.edid.supported_bt2020cycc);
        DISP_DEBUGK("supported_bt2020rgb: %d\n", p_dp->hdmi_info.edid.supported_bt2020rgb);
        DISP_DEBUGK("supported_bt2020ycc: %d\n", p_dp->hdmi_info.edid.supported_bt2020ycc);

        DISP_DEBUGK("supported_rgb_30bit: %d\n", p_dp->hdmi_info.edid.rgb30bit);
        DISP_DEBUGK("supported_rgb_36bit: %d\n", p_dp->hdmi_info.edid.rgb36bit);
        DISP_DEBUGK("supported_rgb_48bit: %d\n", p_dp->hdmi_info.edid.rgb48bit);
        DISP_DEBUGK("supported_yuv444_dc: %d\n", p_dp->hdmi_info.edid.dc_y444);
        DISP_DEBUGK("supported_yuv420_30bit: %d\n", p_dp->hdmi_info.edid.y420_30bit);
        DISP_DEBUGK("supported_yuv420_36bit: %d\n", p_dp->hdmi_info.edid.y420_36bit);
        DISP_DEBUGK("supported_yuv420_48bit: %d\n", p_dp->hdmi_info.edid.y420_48bit);

        DISP_DEBUGK("supported_3840x2160p_50Hz: %d\n", p_dp->hdmi_info.edid.supported_3840x2160p_50Hz);
        DISP_DEBUGK("supported_3840x2160p_60Hz: %d\n", p_dp->hdmi_info.edid.supported_3840x2160p_60Hz);
        DISP_DEBUGK("supported_4096x2160p_50Hz: %d\n", p_dp->hdmi_info.edid.supported_4096x2160p_50Hz);
        DISP_DEBUGK("supported_4096x2160p_60Hz: %d\n", p_dp->hdmi_info.edid.supported_4096x2160p_60Hz);

        if(usr_force_bitdepth == 12)
        {
            p_dp->hdmi_info.edid.y420_36bit = 1;
            p_dp->hdmi_info.edid.y420_30bit = 0;
            p_dp->hdmi_info.edid.rgb36bit = 1;
            p_dp->hdmi_info.edid.rgb30bit = 0;
        }
        else if(usr_force_bitdepth == 10)
        {
            p_dp->hdmi_info.edid.y420_36bit = 0;
            p_dp->hdmi_info.edid.y420_30bit = 1;
            p_dp->hdmi_info.edid.rgb36bit = 0;
            p_dp->hdmi_info.edid.rgb30bit = 1;
        }
        else if(usr_force_bitdepth == 8)
        {
            p_dp->hdmi_info.edid.y420_36bit = 0;
            p_dp->hdmi_info.edid.y420_30bit = 0;
            p_dp->hdmi_info.edid.rgb36bit = 0;
            p_dp->hdmi_info.edid.rgb30bit = 0;
        }

        if(usr_force_hdr10 == 1)
        {
            p_dp->hdmi_info.edid.supported_hdr10p_vsif = 0;
            p_dp->hdmi_info.edid.hdr10p_emp = 0;        
            p_dp->hdmi_info.edid.supported_hdr10 = 1;
            p_dp->hdmi_info.supported_bt2020 = 1;
        }
        else if(usr_force_hdr10 == 2)
        {
            p_dp->hdmi_info.edid.supported_hdr10p_vsif = 1;
            p_dp->hdmi_info.edid.hdr10p_emp = 0;
            p_dp->hdmi_info.edid.supported_hdr10 = 1;
            p_dp->hdmi_info.supported_bt2020 = 1;
        }
        else if(usr_force_hdr10 == 3)
        {
            p_dp->hdmi_info.edid.supported_hdr10p_vsif = 0;
            p_dp->hdmi_info.edid.hdr10p_emp = 1;
            p_dp->hdmi_info.edid.supported_hdr10 = 1;
            p_dp->hdmi_info.supported_bt2020 = 1;
        }

        DISP_DEBUGK("usr force config edid: usr_force_bitdepth:%d usr_force_hdr10:%d\n",
                usr_force_bitdepth, usr_force_hdr10);

#endif
    }
#endif
}

static void disp_print_hdmi_vcfg_info(disp_priv_t *p_dp)
{
    /*int i;*/
    DISP_DEBUGK("hdmi_vcfg input, \r\n csc:%d,\r\n std:%d,\r\n bitDepth:%d,\r\n pixel_rpt:%d,\r\n full_range:%d\n",
            p_dp->hdmi_vcfg.input_v_cfg.csc,
            p_dp->hdmi_vcfg.input_v_cfg.std,
            p_dp->hdmi_vcfg.input_v_cfg.bitDepth,
            p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt,
            p_dp->hdmi_vcfg.input_v_cfg.video_full_range);
    DISP_DEBUGK("hdmi_vcfg output, \n csc:%d,\n std:%d,\n bitDepth:%d,\n pixel_rpt:%d,\n shape:%d,\n hdr_onoff:%d,\n fmt:%d,\n length:%d,\n type:%d,\n length:%d,\n type:%d \n",
            p_dp->hdmi_vcfg.output_v_cfg.csc,
            p_dp->hdmi_vcfg.output_v_cfg.std,
            p_dp->hdmi_vcfg.output_v_cfg.bitDepth,
            p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt,
            p_dp->hdmi_vcfg.output_v_cfg.shape,
            p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff,
            p_dp->hdmi_vcfg.output_v_cfg.fmt,
            p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0].length,
            p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0].type,
            p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[1].length,
            p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[1].type          );

    DISP_DEBUGK("hdmi_vcfg input, \r\n hdmi_3d_res:%d,\r\n hdmi_3d_ext:%d,\r\n",
            p_dp->hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_res,
            p_dp->hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_ext);
#if 0
    for(i = 0; i < 27;i++)
        DISP_DEBUGK("\r\n pb_byte[%d]:%d", i, p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0].pb_byte[i]);
#endif
    // for(i = 0; i < 27;i++)
    //     DISP_DEBUGK("\r\n pb_byte1[%d]:%d", i, p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[1].pb_byte[i]);

    return;
}

static mt_s32 disp_hdmi_video_config(disp_priv_t *p_dp)
{
    mt_s32 ret =  MT_SUCCESS;
    /*HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;*/
    MT_ASSERT(NULL != p_dp);

    /*mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);*/

    if (pstHDMIFunc && pstHDMIFunc->pfnHdmiConfigVid)
    {
        disp_print_hdmi_vcfg_info(p_dp);

        if (SUCCESS == pstHDMIFunc->pfnHdmiConfigVid(0, (void *)(&p_dp->hdmi_vcfg)))
        {
            p_dp->hdmi_vcfg_flag = MT_FALSE;
            ret =  MT_SUCCESS;
            DISP_DEBUGK("disp concerto HDMI cfg Success, hdmi_vcfg_addr:%p hdmi_vcfg_size:%ld \n", &p_dp->hdmi_vcfg, (ulong)sizeof(hdmi_video_config_t));
        }
        else
        {
            p_dp->hdmi_vcfg_flag = MT_FALSE;
            ret = MT_FAILURE;
            DISP_DEBUGK("disp concerto HDMI cfg fail\n");
        }
    }
    return ret;
}

static void disp_aria_hdmi_tasklet_cb_work(struct work_struct *work)
{
    mt_s32 ret;

    disp_priv_t *p_dp = container_of(work, disp_priv_t, work_hdmi_cb);

    ret = disp_hdmi_video_config(p_dp);
    if(ret != MT_SUCCESS)
    {
        msleep(30);
        disp_hdmi_video_config(p_dp);
    }
    
    drv_disp_get_hdmi_edid();
    return;
}

static MT_DRV_DISP_FMT_E fmt_disp2hdmi(MT_DRV_DISP_FMT_E fmt)
{
    MT_DRV_DISP_FMT_E fmt2hdmi = fmt;
    switch (fmt)
    {
        case MT_DRV_DISP_FMT_1080i_59_94:
            fmt2hdmi = MT_DRV_DISP_FMT_1080i_60;
            break;
        case MT_DRV_DISP_FMT_1080P_59_94:
            fmt2hdmi = MT_DRV_DISP_FMT_1080P_60;
            break;
        case MT_DRV_DISP_FMT_1080P_29_97:
            fmt2hdmi = MT_DRV_DISP_FMT_1080P_30;
            break;
        case MT_DRV_DISP_FMT_720P_59_94:
            fmt2hdmi = MT_DRV_DISP_FMT_720P_60;
            break;
        case MT_DRV_DISP_FMT_3840X2160_29_97:
            fmt2hdmi = MT_DRV_DISP_FMT_3840X2160_30;
            break;
        case MT_DRV_DISP_FMT_3840X2160_59_94:
            fmt2hdmi = MT_DRV_DISP_FMT_3840X2160_60;
            break;
        case MT_DRV_DISP_FMT_4096X2160_29_97:
            fmt2hdmi = MT_DRV_DISP_FMT_4096X2160_30;
            break;
        case MT_DRV_DISP_FMT_4096X2160_59_94:
            fmt2hdmi = MT_DRV_DISP_FMT_4096X2160_60;
            break;
        default:
            break;
    }

    return fmt2hdmi;
}

static void disp_hdmi_setformat_work(struct work_struct *work)
{
    DISP_S *pstDisp = NULL;
    /*HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;*/
    mt_s32 u32Ret = MT_FAILURE;
    disp_priv_t *p_dp = container_of(work, disp_priv_t, work_hdmi_setformat);
    MT_DRV_DISP_FMT_E fmt2hdmi = MT_DRV_DISP_FMT_1080i_50;

    DispGetPointerByIDNoReturn(MT_DRV_DISPLAY_1, pstDisp);
    /*mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);*/

    disp_hdmi_video_config(p_dp);

    drv_disp_get_hdmi_edid();

    if(pstHDMIFunc && pstHDMIFunc->pfnHdmiSetFormat)
    {
        fmt2hdmi = fmt_disp2hdmi(pstDisp->stSetting.enFormat);       
        u32Ret = pstHDMIFunc->pfnHdmiSetFormat(0, fmt2hdmi, pstDisp->stSetting.eDispMode);
        if(u32Ret != MT_SUCCESS)
        {
            DISP_DEBUGK("%s %d pfnHdmiSetFormat fialed!! \n", __FUNCTION__, __LINE__);
        }
        DISP_DEBUGK("[%s]line %d,hdmi_setformat enFormat:[%d %d] eDispMode;%d\n", __FUNCTION__, __LINE__, 
            pstDisp->stSetting.enFormat, fmt2hdmi, pstDisp->stSetting.eDispMode);        
    }
    else
    {
        DISP_DEBUGK("%s %d pfnHdmiSetFormat==NULL \n", __FUNCTION__, __LINE__);
    }

}

hdmi_3d_resolution_t disp_stereo_mode_to_hdmi_3d_res(MT_DRV_DISP_STEREO_E stereo_mode)
{
    hdmi_3d_resolution_t hdmi_3d_res = HDMI_3D_UNDEFINE;

    switch (stereo_mode)
    {
    case DISP_STEREO_NONE:
        hdmi_3d_res = HDMI_3D_UNDEFINE;
        break;
    case DISP_STEREO_FPK:
        hdmi_3d_res = HDMI_3D_FRAME_PACKING;
        break;
    case DISP_STEREO_SBS_HALF:
        hdmi_3d_res = HDMI_3D_SIDE_BY_SIDE_HALF;
        break;
    case DISP_STEREO_TAB:
        hdmi_3d_res = HDMI_3D_TOP_AND_BOTTOM;
        break;
    case DISP_STEREO_FIELD_ALTE:
        hdmi_3d_res = HDMI_3D_FIELD_ALT;
        break;
    case DISP_STEREO_LINE_ALTE:
        hdmi_3d_res = HDMI_3D_LINE_ALT;
        break;
    case DISP_STEREO_SBS_FULL:
        hdmi_3d_res = HDMI_3D_SIDE_BY_SIDE_FULL;
        break;
    case DISP_STEREO_L_DEPT:
        hdmi_3d_res = HDMI_3D_L_DEPTH;
        break;
    case DISP_STEREO_L_DEPT_G_DEPT:
        hdmi_3d_res = HDMI_3D_L_DEPTH_GRAPHICS;
        break;

    default:
        hdmi_3d_res = HDMI_3D_UNDEFINE;
        break;
    }

    return hdmi_3d_res;
}

static void disp_set_hdmi_hdr_cfg(disp_priv_t *p_dp)
{
#ifdef SUPPORT_HDR

    if(p_dp->hdmi_info.used_tv_mode != TV_MODE_SDR)
    {
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_2020;
        p_dp->hdmi_vcfg.output_v_cfg.std = p_dp->csc_info.sub_colour_primaries_2020 ? SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS : SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 1;
    }
    else
    {
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
    }
#endif
}

static void disp_set_hdmi(disp_priv_t *p_dp)
{
    //printk("[%s]line %d\n",__FUNCTION__,__LINE__);
    DISP_S *pstDisp = NULL;
    DispGetPointerByIDNoReturn(MT_DRV_DISPLAY_1, pstDisp);

#if defined(CONFIG_HDMI)
    switch (p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt)
    {
    case VID_SYS_PAL:
    case VID_SYS_PAL_N:
    case VID_SYS_PAL_NC:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_2_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_2_TIMES;
        
        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_NTSC_J:
    case VID_SYS_NTSC_M:
    case VID_SYS_NTSC_443:
    case VID_SYS_PAL_M:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_2_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_2_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_576P_50HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_480P:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_720P:
    case VID_SYS_720P_30HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_720P_50HZ:
    case VID_SYS_720P_24HZ:
    case VID_SYS_720P_25HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_1080I:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_1080I_50HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_1080P:
    case VID_SYS_1080P_30HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;
 
        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_1080P_50HZ:
    case VID_SYS_1080P_25HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_1080P_24HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;
 
        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_3840X2160_60HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_3840X2160_50HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_3840X2160_30HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_3840X2160_25HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_3840X2160_24HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_4096X2160_24HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_4096X2160_25HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_4096X2160_30HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_4096X2160_50HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;
    case VID_SYS_4096X2160_60HZ:
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;
        break;

    default: //1080i50
        p_dp->hdmi_vcfg.input_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__12_BIT;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_601;
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;

        p_dp->hdmi_vcfg.output_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.output_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        p_dp->hdmi_vcfg.output_v_cfg.bitDepth = SII_DRV_BIT_DEPTH__8_BIT;
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;
        p_dp->hdmi_vcfg.output_v_cfg.pixel_rpt = PIXEL_RPT_1_TIMES;
        p_dp->hdmi_vcfg.output_v_cfg.fmt = VID_SYS_1080I_50HZ;
        p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff = 0;

        break;
    }

    if(p_dp->hdmi_info.used_tv_mode != TV_MODE_SDR)
    {
        if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT) )
        {
            if(p_dp->csc_info.cur_colour_primaries == 9 && p_dp->csc_info.sub_colour_primaries_2020)
            {
                p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_2020;
                p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
            }
            else if(p_dp->csc_info.cur_colour_primaries == 9 && !p_dp->csc_info.sub_colour_primaries_2020)
            {
                p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_2020;
                p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
            }
#if 0            
            else if(p_dp->csc_info.cur_colour_primaries == 1)
            {
                p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
                p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_709;
            }
#endif            
            else
            {
                p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_2020;
                p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS;
            }
        }
        else
        {
            p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
            p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_709;
        }
    }
    else
    {
        p_dp->hdmi_vcfg.input_v_cfg.csc = SII_DRV_CLRSPC__YC444_709;
        p_dp->hdmi_vcfg.input_v_cfg.std = SII_DRV_CONV_STD__BT_709;
    }

    if (p_dp->cur_3d_flag)
    {
        p_dp->hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_res = disp_stereo_mode_to_hdmi_3d_res(p_dp->cur_mode_3d);
        p_dp->hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_ext = p_dp->cur_extpara_3d;
    }
    else
    {
        p_dp->hdmi_vcfg.hdmi_3d_cfg.hdmi_3d_res = HDMI_3D_UNDEFINE;
    }

    if(pstDisp->stSetting.u32CustomRatioWidth == 4 && pstDisp->stSetting.u32CustomRatioHeight == 3)
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_4X3;
    else
        p_dp->hdmi_vcfg.output_v_cfg.shape = HDMI_SHAPE_16X9;

    p_dp->hdmi_vcfg.output_v_cfg.fmt = p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt;

    disp_set_hdmi_hdr_cfg(p_dp);

#endif

}

void disp_set_hdmi_phy_reset(void)
{
    ulong reg_analog = 0xfd157000;
    ulong reg_value = 0;
    u32 mask = 0x80;

    reg_value = disp_hal_get_u32((volatile mt_u32*) reg_analog);
    reg_value |= mask;
    disp_hal_put_u32((volatile mt_u32*) reg_analog,reg_value);

    // need wait for reset
    msleep(1);
    reg_value = disp_hal_get_u32((volatile mt_u32*) reg_analog);
    //    DISP_PRINT(" hdmi phy reset   reg 0x%x, value 0x%x ticks %d \n", reg_analog,reg_value,mt_ticks_get());
    reg_value &=(~mask);
    disp_hal_put_u32((volatile mt_u32*) reg_analog,reg_value);

    reg_value = disp_hal_get_u32((volatile mt_u32*) reg_analog);
    //    DISP_PRINT("  hdmi phy reset  reg 0x%x, value 0x%x ticks %d \n", reg_analog,reg_value,mt_ticks_get());
    //mtos_task_sleep(10);   no used at isr
}


/*!
  Input video interlaced or not.

*/
/*!
  Input video interlaced or not.

*/

void disp_st_vid_update_info(disp_priv_t *p_dp)
{
    //static mt_u32 load_flag = 0;
    //get video cur display size
    disp_st_vid_get_vdec_size(&(p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h),
            &(p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w));
    //get video frame or field mode
    p_dp->disp_in_info.old_rect[DISP_CHANNEL_HD].h = p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h;
    p_dp->disp_in_info.old_rect[DISP_CHANNEL_HD].w = p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w;
    p_dp->old_3d_flag = p_dp->cur_3d_flag;
    p_dp->old_mode_3d = p_dp->cur_mode_3d;
    p_dp->old_extpara_3d = p_dp->cur_extpara_3d;

#if 0
    p_dp->disp_in_info.b_frame = disp_hal_vid_input_frame_mode();
    //���b_frame�����л�ʱ�Ķ������⣬jqw
    if(p_dp->disp_in_info.b_frame == 0)
    {
        if(p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w <= DISP_VID_FULLSCR_SD_WIDTH)
        {
            p_dp->disp_in_info.b_frame = 1;
            p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h <<= 1;
        }
    }

    //get video progressive or interleaved
    p_dp->disp_in_info.b_progressive = disp_hal_vid_get_progressive_flag();

    //get video firmware di force disable status
    p_dp->di_info.di_firmware_force_disable_cur = disp_hal_vid_get_firmware_force_di_disable();

    //get input ar
    //  disp_hal_vid_get_input_ar(&(p_dp->disp_in_info.ar));
    disp_hal_vid_get_input_ar_new(p_dp);

    if(load_flag != 0)
    {
        mt_u32 table_sel = reg_concerto_disp_get_vscaler_table_sel() & (~load_flag);
        reg_concerto_disp_set_vscaler_table_sel(table_sel);
    }
    //clear the load table status
    load_flag = reg_concerto_disp_get_vscaler_table_sel() & 0x7ff;
#endif
    p_dp->disp_in_info.cur_rect[DISP_CHANNEL_SD].h = p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h;
    p_dp->disp_in_info.cur_rect[DISP_CHANNEL_SD].w = p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w;
}

static mt_u32 hd_osclk = 0;
static mt_u32 hd_clock = 0;
#define R_CLKGEN_VHDINTP SYMPHONY_IO_VA(0xBF5D005C)
#define R_CLKGEN_VSDINTP SYMPHONY_IO_VA(0xBF5D0064)
#define R_CLKGEN_VSDPLL  SYMPHONY_IO_VA(0xBF5D0060)
#define R_CLKGEN_PDSYS_REG    SYMPHONY_IO_VA(0xBF5D009C)
#define R_CLKGEN_VHD_REG   SYMPHONY_IO_VA(0xBF5D0058)
#define R_VOUT_CLKSEL_REG  SYMPHONY_IO_VA(0xBF50A604)


void disp_symphony_hd_clk_cfg(disp_priv_t *p_dp, mt_u32 clk)
{
    /*
     * HDMI config
     * hd  clkgen_vhdintp_reg[1:0] hdmi clock      ÅäÖÃclkgen_vhdintp_reg[5:4]
     * 74.25  (0xbf5d005c[1:0]=2'b00)      742.5 (0xbf5d005c[5:4]=2'b01)
     * 27 (0xbf5d005c[1:0]=2'b11)        270 (0xbf5d005c[5:4]=2'b11)
     * 148.5 (0xbf5d005c[1:0]=2'b01)     1485 (0xbf5d005c[5:4]=2'b10)
     */

    /*HDMI_EXPORT_FUNC_S *pstHDMIFunc = MT_NULL;*/
    HDMI_CLK_CFG_S hdmi_clk_cfg_info = {0};

    DISP_DEBUGK("\n[%s_%d] hd_osclk:%d, clk:%d \n",__func__,__LINE__, hd_osclk, clk);

    /*mt_drv_module_getfunction(MT_ID_HDMI, (mt_void **)&pstHDMIFunc);*/
    hdmi_clk_cfg_info.enHdmi = MT_UNF_HDMI_ID_0;
    hdmi_clk_cfg_info.clk = clk;
    hdmi_clk_cfg_info.ssc_on = 0;
    hdmi_clk_cfg_info.pdpi_on = 0;
    hdmi_clk_cfg_info.vout_clk_gate_on = 0;
    hdmi_clk_cfg_info.clk_os = hd_osclk;
    if (pstHDMIFunc && pstHDMIFunc->pfnHdmiClkCfg)
    {
        pstHDMIFunc->pfnHdmiClkCfg(hdmi_clk_cfg_info.enHdmi, (mt_u32 *)&hdmi_clk_cfg_info);
    }else{
        DISP_DEBUGK("\n[%s_%d]%px,%px(333)\n",__func__,__LINE__,pstHDMIFunc,pstHDMIFunc?pstHDMIFunc->pfnHdmiClkCfg: NULL);
    }

    return;

}

static void disp_st_tvsys_update(MT_DRV_DISPLAY_E enDisp)
{
    DISP_S *pstDisp;
    DISP_S *pstDisp_sd;
    struct hdmiphy_param param = {0};
    
    disp_priv_t *p_dp = &s_stDisplayPriv;

    disp_sys_t tv_sys_sd = p_dp->disp_out_info[DISP_CHANNEL_SD].vid_fmt;
    disp_sys_t tv_sys_hd = p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt;
    static disp_sys_t old_tv_sys_sd = VID_SYS_MAX;
    static disp_sys_t old_tv_sys_hd = VID_SYS_MAX;
    
    mt_u8 video_fmt = 0;
    static mt_u8 rate_conversion_enable = 0;
    static mt_u8 old_rate_conversion_enable = 0;
    static mt_u32 hd_screen_width = 0;
    static mt_u32 hd_screen_height = 0;
    static mt_u32 sd_screen_width = 0;
    static mt_u32 sd_screen_height = 0;

    mt_bool b_clock_div_1p001_enable = 0;
    mt_u32 temp_value = 0;
    mt_u32 temp_value2 = 0;
    MT_DRV_DISPLAY_E enDisp_tmp;
    // unsigned long chip_rev = symphony_get_chip_rev();
    volatile mt_u32 dtmp = 0;

    static mt_u8 old_sd_reinterlace_en = 0;
    static mt_u8 old_hd_reinterlace_en = 0;
    mt_u8 sd_reinterlace_en = drv_reg_4k_disp_get_sd_reinterlace_ctrl_reinterlace_en();
    mt_u8 hd_reinterlace_en = drv_reg_4k_disp_get_hd_reinterlace_ctrl_reinterlace_en();

    //printk("[%s]line [%d]\n",__FUNCTION__,__LINE__);
    //DispCheckID(enDisp);
    //printk("[%s]line [%d]\n",__FUNCTION__,__LINE__);
    enDisp_tmp = (MT_DRV_DISPLAY_E)DISP_CHANNEL_HD;
    DispGetPointerByIDNoReturn(enDisp_tmp, pstDisp);
    enDisp_tmp = (MT_DRV_DISPLAY_E)DISP_CHANNEL_SD;
    DispGetPointerByIDNoReturn(enDisp_tmp, pstDisp_sd);

    if ((0 != p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h) &&
            (0 != p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w) &&
            (p_dp->disp_in_info.old_rect[DISP_CHANNEL_HD].h !=
             p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h))
    {
        switch (p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h)
        {
        case DISP_VID_FULLSCR_PAL_HEIGHT:
        case DISP_VID_FULLSCR_PAL_HEIGHT_HALF:
            if (VID_SYS_AUTO == tv_sys_sd)
            {
                tv_sys_sd = VID_SYS_PAL;
                p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            if (VID_SYS_AUTO == tv_sys_hd)
            {
                tv_sys_hd = VID_SYS_PAL;
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            break;
        case DISP_VID_FULLSCR_NTSC_HEIGHT:
        case DISP_VID_FULLSCR_NTSC_HEIGHT_HALF:
            if (VID_SYS_AUTO == tv_sys_sd)
            {
                tv_sys_sd = VID_SYS_NTSC_M;
                p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            if (VID_SYS_AUTO == tv_sys_hd)
            {
                tv_sys_hd = VID_SYS_NTSC_M;
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            break;
        case DISP_VID_FULLSCR_720P_HEIGHT:
            if (VID_SYS_AUTO == tv_sys_sd)
            {
                tv_sys_sd = VID_SYS_PAL;
                p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            if (VID_SYS_AUTO == tv_sys_hd)
            {
                tv_sys_hd = VID_SYS_720P;
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            break;
        case DISP_VID_FULLSCR_1080_HEIGHT:
        case DISP_VID_FULLSCR_1080_HEIGHT_1088:
            if (VID_SYS_AUTO == tv_sys_sd)
            {
                tv_sys_sd = VID_SYS_PAL;
                p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            if (VID_SYS_AUTO == tv_sys_hd)
            {
                tv_sys_hd = VID_SYS_1080I;
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
            }
            break;
        }
    }
    if ((!p_dp->usr_force_3d)
            && (p_dp->cur_3d_flag != p_dp->old_3d_flag || p_dp->cur_mode_3d != p_dp->old_mode_3d || p_dp->cur_extpara_3d != p_dp->old_extpara_3d))
    {
        pstDisp->stSetting.eDispMode = p_dp->cur_mode_3d;
        p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
        update_sys_flag = 1;
        reset_delay_cnt = 0;
        DISP_DEBUGK("3d info changed! \n old_3d_flag:%d old_mode_3d:%d old_extpara_3d:%d\n", p_dp->old_3d_flag, p_dp->old_mode_3d, p_dp->old_extpara_3d);
        DISP_DEBUGK(" cur_3d_flag:%d cur_mode_3d:%d cur_extpara_3d:%d\n", p_dp->cur_3d_flag, p_dp->cur_mode_3d, p_dp->cur_extpara_3d);
    }

    if(update_sys_flag == 1)
    {
        reset_delay_cnt++;
        DISP_DEBUGK(" func [%s] lin %d reset_delay_cnt:%d \n", __FUNCTION__, __LINE__, reset_delay_cnt);

        if ((0 == drv_reg_4k_disp_get_sd_wr_ctrl_sd_wr_back_forbidden()) && (15 == reset_delay_cnt) && (no_sd == 0))
        {
#if 0
            if(p_dp->csc_info.csc_enable == MT_TRUE) //fix 128817
            {
                p_dp->csc_info.old_resolution = 0xff; //force config again
            }
#endif
            mt_s32 ret = DF_GraScaler_Update(&g_stDrvSetting[0]);
            if(ret == MT_SUCCESS)
            {
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->vdac on! \n", __FUNCTION__, __LINE__);
                disp_update_cvbs_for_sd_tvsys(p_dp, MT_TRUE);
            }

            //enable SD_VENC auto clk gate
            dtmp = disp_hal_get_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec));
            dtmp |= (0x1 << 8);//bit8 set 1
            disp_hal_put_u32((volatile mt_u32 *)(mt_get_sdvenc_base() + 0xec), dtmp);

            reset_delay_cnt = 0;
            update_sys_flag = 0;
        }
    }

    if (UPDATE_FLAG_TVSYS ==
            (p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag & UPDATE_FLAG_TVSYS))
    {
        //delay 1frame to change
        if (6 == reset_delay_cnt)
        {
            switch (tv_sys_sd)
            {
            case VID_SYS_PAL:
                video_fmt = 1;
                sd_screen_width = 720;
                sd_screen_height = 288;
                break;
            case VID_SYS_PAL_N:
            case VID_SYS_PAL_NC:
                video_fmt = 5;
                sd_screen_width = 720;
                sd_screen_height = 288;
                break;
            case VID_SYS_PAL_M:
                video_fmt = 4;
                sd_screen_width = 720;
                sd_screen_height = 240;
                break;
            case VID_SYS_NTSC_J:
            case VID_SYS_NTSC_M:
                video_fmt = 2;
                sd_screen_width = 720;
                sd_screen_height = 240;
                break;
            case VID_SYS_NTSC_443:
                video_fmt = 6;
                sd_screen_width = 720;
                sd_screen_height = 240;
                break;
            case VID_SYS_SECAM:
                video_fmt = 3;
                sd_screen_width = 720;
                sd_screen_height = 288;
                break;
            default:
                //sd only support pal and ntsc
                DISP_PRINT("unsupported SD format\n");
                return;
                //sd only support pal and ntsc
                //MT_ASSERT(0);
            }

            reg_aria_sd_enc_set_cfg6_video_fmt(video_fmt);
            drv_reg_4k_disp_set_sd_screen_out_size_sd_screen_width(sd_screen_width);
            if(is_pal(tv_sys_sd))
            {
                drv_reg_4k_disp_set_sd_wr_ctrl_sd_pal_format(1);
            }
            else
            {
                drv_reg_4k_disp_set_sd_wr_ctrl_sd_pal_format(0);
            }
            DISP_DEBUGK(" func [%s] lin %d sd_reinterlace_en:%d rate_conversion_enable:%d  \n", __FUNCTION__, __LINE__, sd_reinterlace_en, rate_conversion_enable);
            if((sd_reinterlace_en || rate_conversion_enable)&&
                    ((sd_screen_height == 240) || (sd_screen_height == 288)))
            {
                drv_reg_4k_disp_set_sd_screen_out_size_sd_screen_height(sd_screen_height * 2);
            }
            else
            {
                drv_reg_4k_disp_set_sd_screen_out_size_sd_screen_height(sd_screen_height);
            }


            disp_hal_set_sd_venc(p_dp);

            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->sd wrback forbidden 0 ! \n", __FUNCTION__, __LINE__);

            drv_reg_4k_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(0);

            p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag &= ~UPDATE_FLAG_TVSYS;
            pstDisp_sd->update_flag &= ~UPDATE_FLAG_TVSYS;
        }
    }

    if (UPDATE_FLAG_TVSYS ==
            (p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag & UPDATE_FLAG_TVSYS))
    {
        disp_set_hdmi(p_dp);

        DISP_DEBUGK("[%s]line [%d],hd %d,sd %d\n", __FUNCTION__, __LINE__, tv_sys_hd, tv_sys_sd);
#if 0
        hdmi_av_mute(MT_TRUE); // mute hdmi av when update disp resolution
        mdelay(32);
#endif

        if (1 == reset_delay_cnt)
        {
            old_tv_sys_sd = transfer_vid_sys_fmt(get_tvsys_from_reg(DISP_CHANNEL_SD));
            old_tv_sys_hd = transfer_vid_sys_fmt(get_tvsys_from_reg(DISP_CHANNEL_HD));
            if (pstHDMIFunc && pstHDMIFunc->pfnHdmiPreFormat)
            {
                MT_DRV_DISP_FMT_E fmt2hdmi = fmt_disp2hdmi(pstDisp->stSetting.enFormat);
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->HdmiPreFormat:[%d %d] ! \n", __FUNCTION__, __LINE__, pstDisp->stSetting.enFormat, fmt2hdmi);
                
                pstHDMIFunc->pfnHdmiPreFormat(0, fmt2hdmi);
            }

        }
        if (4 == reset_delay_cnt)
        {
            mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
            param.mute = 1;
            mt_analog_set_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
        }
        if (5 == reset_delay_cnt)
        {   
            if(old_tv_sys_sd != tv_sys_sd)
            {
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->sd wrback forbidden 1 ! \n", __FUNCTION__, __LINE__);
                drv_reg_4k_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(1);
            }
#ifdef CLOCK_DIV_1001
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_disable ! \n", __FUNCTION__, __LINE__);
            clock_div_1p001_disable(p_dp);
#endif

            if(tv_sys_hd == VID_SYS_PAL_M)
                set_hd_tvsys_regs(VID_SYS_NTSC_J);
            else
                set_hd_tvsys_regs(tv_sys_hd);

            switch (tv_sys_hd)
            {
            case VID_SYS_PAL:
            case VID_SYS_PAL_N:
            case VID_SYS_PAL_NC:
                hd_screen_width = 720;
                hd_screen_height = 288;
                rate_conversion_enable = 0;

                hd_clock = 27000000;
                hd_osclk = 27000000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);

                break;
            case VID_SYS_NTSC_J:
            case VID_SYS_NTSC_M:
            case VID_SYS_NTSC_443:
            case VID_SYS_PAL_M:
                hd_screen_width = 720;
                hd_screen_height = 240;
                rate_conversion_enable = 0;

                hd_clock = 27000000;
                hd_osclk = 27000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);

                break;
            case VID_SYS_576P_50HZ:
                hd_screen_width = 720;
                hd_screen_height = 576;
                rate_conversion_enable = 0;

                hd_clock = 27000000;

#ifdef OVERSAMPLE
                hd_osclk = 108000000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 27000000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif

                break;
            case VID_SYS_480P:
                hd_screen_width = 720;
                hd_screen_height = 480;
                rate_conversion_enable = 0;

                hd_clock = 27000000;
#ifdef OVERSAMPLE
                hd_osclk = 108000000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 27000000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif

                break;
            case VID_SYS_720P:
                hd_screen_width = 1280;
                hd_screen_height = 720;
                rate_conversion_enable = 0;

                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif

#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif

                break;
            case VID_SYS_720P_50HZ:
                hd_screen_width = 1280;
                hd_screen_height = 720;
                rate_conversion_enable = 0;
                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif
                break;
            case VID_SYS_1080I:
                hd_screen_width = 1920;
                hd_screen_height = 540;
                rate_conversion_enable = 0;

                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif

#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif

                break;
            case VID_SYS_1080I_50HZ:
                hd_screen_width = 1920;
                hd_screen_height = 540;
                rate_conversion_enable = 0;

                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif

                break;
            case VID_SYS_1080P:
                hd_screen_width = 1920;
                hd_screen_height = 1080;
                rate_conversion_enable = 0;

                hd_clock = 148500000;
                hd_osclk = 148500000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);

#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif

                break;
            case VID_SYS_1080P_50HZ:
                hd_screen_width = 1920;
                hd_screen_height = 1080;
                rate_conversion_enable = 0;

                hd_clock = 148500000;
                hd_osclk = 148500000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_1080P_25HZ:
                hd_screen_width = 1920;
                hd_screen_height = 1080;
                rate_conversion_enable = 1;

                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif
                break;
            case VID_SYS_1080P_30HZ:
                hd_screen_width = 1920;
                hd_screen_height = 1080;
                rate_conversion_enable = 1;

                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif

#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif

                break;
            case VID_SYS_1080P_24HZ:
                hd_screen_width = 1920;
                hd_screen_height = 1080;
                rate_conversion_enable = 1;
                hd_clock = 74250000;

#ifdef OVERSAMPLE
                hd_osclk = 148500000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(1);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(1);
#else
                hd_osclk = 74250000;
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#endif
                break;

            case VID_SYS_3840X2160_60HZ:
                hd_screen_width = 3840;
                hd_screen_height = 2160;
                rate_conversion_enable = 0;
                hd_clock = 594000000;
                hd_osclk = 594000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);

#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif                
                break;

            case VID_SYS_3840X2160_50HZ:
                hd_screen_width = 3840;
                hd_screen_height = 2160;
                rate_conversion_enable = 0;
                hd_clock = 594000000;
                hd_osclk = 594000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_3840X2160_30HZ:
                hd_screen_width = 3840;
                hd_screen_height = 2160;
                rate_conversion_enable = 1;
                hd_clock = 297000000;
                hd_osclk = 297000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif                  
                break;
            case VID_SYS_3840X2160_25HZ:
                hd_screen_width = 3840;
                hd_screen_height = 2160;
                rate_conversion_enable = 1;
                hd_clock = 297000000;
                hd_osclk = 297000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_3840X2160_24HZ:
                hd_screen_width = 3840;
                hd_screen_height = 2160;
                rate_conversion_enable = 1;
                hd_clock = 297000000;
                hd_osclk = 297000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_4096X2160_24HZ:
                hd_screen_width = 4096;
                hd_screen_height = 2160;
                rate_conversion_enable = 1;
                hd_clock = 297000000;
                hd_osclk = 297000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_4096X2160_25HZ:
                hd_screen_width = 4096;
                hd_screen_height = 2160;
                rate_conversion_enable = 1;
                hd_clock = 297000000;
                hd_osclk = 297000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_4096X2160_30HZ:
                hd_screen_width = 4096;
                hd_screen_height = 2160;
                rate_conversion_enable = 1;
                hd_clock = 297000000;
                hd_osclk = 297000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif                  
                break;
            case VID_SYS_4096X2160_50HZ:
                hd_screen_width = 4096;
                hd_screen_height = 2160;
                rate_conversion_enable = 0;

                hd_clock = 594000000;
                hd_osclk = 594000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);
                break;
            case VID_SYS_4096X2160_60HZ:
                hd_screen_width = 4096;
                hd_screen_height = 2160;
                rate_conversion_enable = 0;

                hd_clock = 594000000;
                hd_osclk = 594000000;

                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_0(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_mux_sdp_2_1(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_os_en(0);
                reg_aria_hd_encoder_set_os_filter_ctrl_sample_ctrl(0);

#ifdef CLOCK_DIV_1001
                DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->clock_div_1p001_enable ! \n", __FUNCTION__, __LINE__);
                b_clock_div_1p001_enable = 1;
#endif                
                break;

            default:
                DISP_PRINT("unsupported HD format\n");
                return;
            }

            drv_reg_4k_disp_set_sd_wr_ctrl_cfg_rate_conversion_enable(rate_conversion_enable);
            drv_reg_4k_disp_set_hd_screen_out_size_hd_screen_width(hd_screen_width);

            DISP_DEBUGK(" func [%s] lin %d sd_reinterlace_en:%d rate_conversion_enable:%d  \n", __FUNCTION__, __LINE__, sd_reinterlace_en, rate_conversion_enable);
            if((hd_reinterlace_en || rate_conversion_enable)&&
                    ((hd_screen_height == 240) || (hd_screen_height == 288) || (hd_screen_height == 540)))
            {
                drv_reg_4k_disp_set_hd_screen_out_size_hd_screen_height(hd_screen_height * 2);
            }
            else
            {
                drv_reg_4k_disp_set_hd_screen_out_size_hd_screen_height(hd_screen_height);
            }


            //need to cfg hdmi
            p_dp->hdmi_vcfg_flag = MT_TRUE;
            if (p_dp->cur_3d_flag && p_dp->cur_mode_3d == DISP_STEREO_FPK)
            {
                reg_aria_hd_encoder_set_y_parameter_level0_is_frame_packing(1);
                reg_aria_hd_encoder_set_y_parameter_level0_de_mode(1);
                reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(0);
                reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(1);
                DISP_DEBUGK("[%s]line %d,framepacking enable\n", __FUNCTION__, __LINE__);

            }
            else
            {
                reg_aria_hd_encoder_set_y_parameter_level0_is_frame_packing(0);
                DISP_DEBUGK("[%s]line %d,framepacking disable\n", __FUNCTION__, __LINE__);
            }

            if (p_dp->hdmi_vcfg_flag && p_dp->tasklet_flag)
            {
                DISP_DEBUGK("[%s]line %d,hdmi_setformat\n", __FUNCTION__, __LINE__);
                schedule_work(&p_dp->work_hdmi_setformat);

                //DISP_DEBUGK("[%s]line %d,hdmi vid_config\n", __FUNCTION__, __LINE__);
                //schedule_work(&p_dp->work_hdmi_cb);
            }

            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->hd_cfg_info_reg_disp_on 0 ! \n", __FUNCTION__, __LINE__);
            reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(0);

            if(pstDisp->stSetting.eDispMode == DISP_STEREO_FPK)
            {
                hd_clock *= 2;
                hd_osclk *= 2;
            }
            DISP_DEBUGK("hd_clock=%d used_tv_mode=%d, 1p001:%d\n", hd_clock, p_dp->hdmi_info.used_tv_mode, b_clock_div_1p001_enable);

            //sd should also reset
            if(old_tv_sys_sd != tv_sys_sd)
            {
                p_dp->disp_out_info[DISP_CHANNEL_SD].update_flag |= UPDATE_FLAG_TVSYS;
                old_tv_sys_sd = tv_sys_sd;
            }
            if(old_tv_sys_hd != tv_sys_hd)
            {
                DISP_DEBUGK(" func [%s] lin %d tvsys changed[%d %d]\n", __FUNCTION__, __LINE__, old_tv_sys_hd, tv_sys_hd);
                old_tv_sys_hd = tv_sys_hd;
                atomic_set(&tvsyschange_flag, 1);
            }
            p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag &= ~UPDATE_FLAG_TVSYS;
            pstDisp_sd->update_flag |= UPDATE_FLAG_TVSYS;
            pstDisp->update_flag &= ~UPDATE_FLAG_TVSYS;

            disp_symphony_hd_clk_cfg(p_dp, hd_clock);

            if((b_clock_div_1p001_enable == 1) && (p_dp->clock_1001_enable == 1))
            {
                clock_div_1p001_enable(p_dp);
            }
#if 0
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->hdvenc_backup! \n", __FUNCTION__, __LINE__);
            hdvenc_backup();
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->sdvenc_backup! \n", __FUNCTION__, __LINE__);
            sdvenc_backup();
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->hdvenc_reset! \n", __FUNCTION__, __LINE__);
            hdvenc_reset();
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->sdvenc_reset! \n", __FUNCTION__, __LINE__);
            sdvenc_reset();
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->hdvenc_restore! \n", __FUNCTION__, __LINE__);
            hdvenc_restore();
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->sdvenc_restore! \n", __FUNCTION__, __LINE__);
            sdvenc_restore();
#endif 
            DISP_DEBUGK(" func [%s] lin %d ---TVSYS--->hd_cfg_info_reg_disp_on 1 ! \n", __FUNCTION__, __LINE__);
            reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(1);
        }

        temp_value = disp_hal_get_u32((volatile mt_u32 *)(REG_ARIA_HD_ENCODER_BASE + 0xf4));
        temp_value2 = disp_hal_get_u32((volatile mt_u32 *)REG_ARIA_HD_ENCODER_HDMI_V_ACTIVEHEIGHT) & 0xfff;
        temp_value &= 0xfffff000;
        temp_value |= temp_value2;
        disp_hal_put_u32((volatile mt_u32 *)(REG_ARIA_HD_ENCODER_BASE + 0xf4), temp_value);
    }
    disp_st_vid_update_info(p_dp);

    if(old_sd_reinterlace_en != sd_reinterlace_en 
            || (old_hd_reinterlace_en != hd_reinterlace_en)
            || (old_rate_conversion_enable != rate_conversion_enable))
    {
        if((sd_reinterlace_en || rate_conversion_enable)&&
                ((sd_screen_height == 240) || (sd_screen_height == 288)))
        {
            drv_reg_4k_disp_set_sd_screen_out_size_sd_screen_height(sd_screen_height * 2);
        }
        else if(sd_screen_height != 0)
        {
            drv_reg_4k_disp_set_sd_screen_out_size_sd_screen_height(sd_screen_height);
        }
        else // (sd_screen_height == 0)
        {
            sd_screen_height = drv_reg_4k_disp_get_sd_screen_out_size_sd_screen_height();
            if((sd_reinterlace_en || rate_conversion_enable)&&
                    ((sd_screen_height == 240) || (sd_screen_height == 288) || (sd_screen_height == 540)))
            {
                drv_reg_4k_disp_set_sd_screen_out_size_sd_screen_height(sd_screen_height * 2);
            }            
        }

        if((hd_reinterlace_en || rate_conversion_enable)&&
                ((hd_screen_height == 240) || (hd_screen_height == 288) || (hd_screen_height == 540)))
        {
            drv_reg_4k_disp_set_hd_screen_out_size_hd_screen_height(hd_screen_height * 2);
        }
        else if(hd_screen_height != 0)
        {
            drv_reg_4k_disp_set_hd_screen_out_size_hd_screen_height(hd_screen_height);
        }
        else // (hd_screen_height == 0)
        {
            hd_screen_height = drv_reg_4k_disp_get_hd_screen_out_size_hd_screen_height();
            if((hd_reinterlace_en || rate_conversion_enable)&&
                    ((hd_screen_height == 240) || (hd_screen_height == 288) || (hd_screen_height == 540)))
            {
                drv_reg_4k_disp_set_hd_screen_out_size_hd_screen_height(hd_screen_height * 2);
            }
        }
        DF_GraScaler_Update(&g_stDrvSetting[0]);

        DISP_DEBUGK(" func [%s] lin %d  rate_conversion_enable:%d  \n", __FUNCTION__, __LINE__, rate_conversion_enable);
        DISP_DEBUGK("reinterlace updated, sd_reinterlace_en:%d sd_screen_height:%d \n", sd_reinterlace_en, sd_screen_height);
        DISP_DEBUGK("reinterlace updated, hd_reinterlace_en:%d hd_screen_height:%d \n", hd_reinterlace_en, hd_screen_height);
    }

    old_sd_reinterlace_en = sd_reinterlace_en;
    old_hd_reinterlace_en = hd_reinterlace_en;
    old_rate_conversion_enable = rate_conversion_enable;
}

mt_s32 disp_set_colorbar(MT_DRV_DISPLAY_E enDisp, MT_BOOL bOn)
{
    mt_s32 nRet = MT_SUCCESS;
    mt_u8  bit_val = 0x0;

    DISP_DEBUGK("%s %d disp:%d bOn:%d \n", __FUNCTION__, __LINE__, enDisp, bOn);
    if (bOn)
        bit_val = 0x1;
    else
        bit_val = 0;

    if (enDisp == MT_DRV_DISPLAY_1)
        reg_aria_hd_encoder_set_reg_color_bar_cbar_enable(bit_val);
    else if (enDisp == MT_DRV_DISPLAY_0)
        reg_symphony_sd_encoder_set_mode_colorbar_sel(bit_val);

    return nRet;
}

mt_s32 disp_set_output_enable(MT_DRV_DISPLAY_E enDisp, MT_BOOL bOn)
{
    mt_s32 nRet = MT_SUCCESS;
    mt_u8  bit_val = 0x0;

    DISP_DEBUGK("%s %d disp:%d bOn:%d \n", __FUNCTION__, __LINE__, enDisp, bOn);
    if (bOn)
        bit_val = 0x1;
    else
        bit_val = 0;

    if (enDisp == MT_DRV_DISPLAY_1)
        reg_aria_hd_encoder_set_hd_cfg_info_reg_disp_on(bit_val);
    else if (enDisp == MT_DRV_DISPLAY_0)
        drv_reg_4k_disp_set_sd_wr_ctrl_sd_wr_back_forbidden(bit_val ^ 0x1);

    return nRet;
}

mt_s32 disp_set_bright(MT_DRV_DISPLAY_E enDisp, mt_u32 percent)
{
    DISP_S *pstDisp;
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 bright = 0x0;
    DispGetPointerByID(enDisp, pstDisp);
    DISP_DEBUGK("\n%s@%d: percent(%d)\n",__FUNCTION__,__LINE__,percent);

    //both channels share the same bright
    pstDisp->stDispInfo.u32Bright = percent;
    if (percent == 50)
        bright = 0x80;
    else
        bright = (percent * 0xFF) / 100;

    if (enDisp == MT_DRV_DISPLAY_1)
        drv_reg_4k_disp_set_hd_video_effect_coef_bright_coeff(bright);
    else if (enDisp == MT_DRV_DISPLAY_0)
        drv_reg_4k_disp_set_sd_video_effect_coef_sd_bright_coeff(bright);

    return nRet;
}

mt_s32 disp_get_bright(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent)
{
    mt_u32 bright = 0x0;

    if (enDisp == MT_DRV_DISPLAY_1)
        bright = drv_reg_4k_disp_get_hd_video_effect_coef_bright_coeff();
    else if (enDisp == MT_DRV_DISPLAY_0)
        bright = drv_reg_4k_disp_get_sd_video_effect_coef_sd_bright_coeff();

    *percent = NUM2PERCENT(bright);
    DISP_DEBUGK("\n%s@%d: percent(%d)\n",__FUNCTION__,__LINE__,*percent);

    return MT_SUCCESS;
}

mt_s32 disp_set_contrast(MT_DRV_DISPLAY_E enDisp, mt_u32 percent)
{
    DISP_S *pstDisp;
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 contrast = 0x0;
    DispGetPointerByID(enDisp, pstDisp);
    DISP_DEBUGK("\n%s@%d: percent(%d)\n",__FUNCTION__,__LINE__,percent);

    //both channels share the same bright
    pstDisp->stDispInfo.u32Contrst = percent;
    if (percent == 50)
        contrast = 0x80;
    else
        contrast = (percent * 0xFF) / 100;

    if (enDisp == MT_DRV_DISPLAY_1)
        drv_reg_4k_disp_set_hd_video_effect_coef_contrast_coeff(contrast);
    else if (enDisp == MT_DRV_DISPLAY_0)
        drv_reg_4k_disp_set_sd_video_effect_coef_sd_contrast_coeff(contrast);

    return nRet;
}

mt_s32 disp_get_contrast(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent)
{
    mt_u32 contrast = 0x0;

    if (enDisp == MT_DRV_DISPLAY_1)
        contrast = drv_reg_4k_disp_get_hd_video_effect_coef_contrast_coeff();
    else if (enDisp == MT_DRV_DISPLAY_0)
        contrast = drv_reg_4k_disp_get_sd_video_effect_coef_sd_contrast_coeff();

    *percent = NUM2PERCENT(contrast);
    DISP_DEBUGK("\n%s@%d: percent(%d)\n",__FUNCTION__,__LINE__, *percent);

    return MT_SUCCESS;
}

mt_s32 disp_set_saturation(MT_DRV_DISPLAY_E enDisp, mt_u32 percent)
{
    DISP_S *pstDisp;
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 saturation = 0x0;
    DispGetPointerByID(enDisp, pstDisp);
    DISP_DEBUGK("\n%s@%d: percent(%d)\n",__FUNCTION__,__LINE__,percent);

    //both channels share the same bright
    pstDisp->stDispInfo.u32Satur = percent;
    if (percent == 50)
        saturation = 0x80;
    else
        saturation = (percent * 0xFF) / 100;

    if (enDisp == MT_DRV_DISPLAY_1)
        drv_reg_4k_disp_set_hd_video_effect_coef_saturation_coeff(saturation);
    else if (enDisp == MT_DRV_DISPLAY_0)
        drv_reg_4k_disp_set_sd_video_effect_coef_sd_saturation_coeff(saturation);

    return nRet;
}

mt_s32 disp_get_saturation(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent)
{
    mt_u32 saturation = 0x0;

    if (enDisp == MT_DRV_DISPLAY_1)
        saturation = drv_reg_4k_disp_get_hd_video_effect_coef_saturation_coeff();
    else if (enDisp == MT_DRV_DISPLAY_0)
        saturation = drv_reg_4k_disp_get_sd_video_effect_coef_sd_saturation_coeff();

    *percent = NUM2PERCENT(saturation);
    DISP_DEBUGK("\n%s@%d: percent(%d)\n",__FUNCTION__,__LINE__, *percent);

    return MT_SUCCESS;
}


//need floating-point calculation, this function haven't been ready yet~~
mt_s32 disp_set_hue(MT_DRV_DISPLAY_E enDisp, mt_u32 percent)
{
    DISP_S *pstDisp;
    mt_s32 nRet = MT_FAILURE;
    DispGetPointerByID(enDisp, pstDisp);

    DISP_DEBUGK("\n%s@%d: can not set hue for now\n",__FUNCTION__,__LINE__);

    //both channels share the same bright
    pstDisp->stDispInfo.u32Hue = percent;

    return nRet;
}

mt_s32 disp_get_hue(MT_DRV_DISPLAY_E enDisp, mt_u32 *percent)
{
    DISP_S *pstDisp;
    mt_s32 nRet = MT_FAILURE;

    DispGetPointerByID(enDisp, pstDisp);
    DISP_DEBUGK("\n%s@%d: can not get hue for now\n",__FUNCTION__,__LINE__);

    return nRet;
}

mt_s32 disp_set_layershow(MT_DRV_DISPLAY_E enDisp, DISP_LAYERSHOW_S *arg)
{
    DISP_S *pstDisp;
    mt_s32 nRet = MT_SUCCESS;
    MT_DRV_DISP_LAYER_ID_E elayer = arg->elayer;
    MT_BOOL b_on = arg->b_on;
    DispGetPointerByID(enDisp, pstDisp);
    
    switch (elayer)
    {
    case DISP_LAYER_ID_VIDEO_SD:
    case DISP_LAYER_ID_VIDEO_HD:
        LayerEnable[DISP_LAYER_ID_VIDEO_SD] = b_on;
        LayerEnable[DISP_LAYER_ID_VIDEO_HD] = b_on;
        DISP_DEBUGK("\n%s@%d: call vid_sel(%d)\n",__FUNCTION__,__LINE__,b_on);
        break;
    case DISP_LAYER_ID_STILL_SD:
    case DISP_LAYER_ID_STILL_HD:
        LayerEnable[DISP_LAYER_ID_STILL_SD] = b_on;
        LayerEnable[DISP_LAYER_ID_STILL_HD] = b_on;
        DISP_DEBUGK("\n%s@%d: call subvid_sel(%d)\n",__FUNCTION__,__LINE__,b_on);
        break;        
    default:
        break;
    }
    return nRet;
}

static void disp_dce_cfg(MT_DRV_DISP_PPMODE_E ppMode)
{
    MT_U32 addr;
    if(ppMode == MT_DRV_DISP_PPMODE_STANDARD)
    {
        //pass frequnce responce test
        drv_reg_4k_disp_set_chroma_upscale_ctrl_chroma_hori_ip_mode(2);
        drv_reg_4k_disp_set_video_ctrl_2_small_picture_upscaling_en(0);
        drv_reg_4k_disp_set_video_dce_config_hd_video_dce_en(0);
        drv_reg_4k_disp_set_video_dce_config_sd_video_dce_en(0);
        return;
    }
    if(ppMode == MT_DRV_DISP_PPMODE_DEFAULT)
    {
        drv_reg_4k_disp_set_chroma_upscale_ctrl_chroma_hori_ip_mode(1);
        drv_reg_4k_disp_set_video_ctrl_2_small_picture_upscaling_en(0);

        drv_reg_4k_disp_set_video_dce_config_hd_video_dce_en(0);
        drv_reg_4k_disp_set_video_dce_config_sd_video_dce_en(0);
        return;
    }

    if(ppMode == MT_DRV_DISP_PPMODE_VIVID)
    {
        addr = DF_GetVideoCoeffTable(VIDEO_SCALE_COEFF_TABLE_DCE_10BIT);
    }
    else
    {
        addr = DF_GetVideoCoeffTable(VIDEO_SCALE_COEFF_TABLE_DCE_NO_ECO_0);
    }

    drv_reg_4k_disp_set_video_dce_map_addr(addr);
    drv_reg_4k_disp_set_coeff_table_sel_video_dce_map_load_en(1);
    drv_reg_4k_disp_set_chroma_upscale_ctrl_chroma_hori_ip_mode(1);
    drv_reg_4k_disp_set_video_ctrl_2_small_picture_upscaling_en(0);

    //enable dce when not to pass AE freq resp
    drv_reg_4k_disp_set_video_dce_config_hd_video_dce_en(1);
    drv_reg_4k_disp_set_video_dce_config_sd_video_dce_en(1);
}

mt_s32 disp_set_set_postprocess_mode(MT_DRV_DISPLAY_E enDisp, DISP_PP_S *arg)
{
    DISP_S *pstDisp;
    disp_priv_t *p_dp;
    mt_s32 nRet = MT_SUCCESS;
    MT_DRV_DISP_PPMODE_E ppMode = arg->enPPMode;
    DispGetPointerByID(enDisp, pstDisp);
    p_dp = (disp_priv_t *)pstDisp->p_dp;

    switch(ppMode)
    {
    case MT_DRV_DISP_PPMODE_STANDARD:
        p_dp->pp_mode = PP_MODE_STANDARD;
        break;
    case MT_DRV_DISP_PPMODE_DEFAULT:
        p_dp->pp_mode = PP_MODE_DEFAULT;
        break;
    case MT_DRV_DISP_PPMODE_VIVID:
        p_dp->pp_mode = PP_MODE_VIVID;
        break;
    default:
        p_dp->pp_mode = PP_MODE_DEFAULT;
        break;
    }

    PPMode = ppMode;

    //dce cfg
    disp_dce_cfg(ppMode);

    if (ppMode == MT_DRV_DISP_PPMODE_STANDARD)
    {
        drv_reg_4k_disp_set_hd_video_post_config(0);
        drv_reg_4k_disp_set_sd_video_post_config(0);
        drv_reg_4k_disp_set_hd_video_effect_coef(0x808080);
        drv_reg_4k_disp_set_sd_video_effect_coef(0x808080);
        drv_reg_4k_disp_set_color_enhance_ctrl(0x47f04000);

        disp_set_csc_onoff(MT_FALSE);
        disp_set_denoise_onoff(MT_FALSE);
    }
    else if (ppMode == MT_DRV_DISP_PPMODE_DEFAULT)
    {
        drv_reg_4k_disp_set_hd_video_post_config(0);
        drv_reg_4k_disp_set_sd_video_post_config(0);
        drv_reg_4k_disp_set_hd_video_effect_coef(0x808080);
        drv_reg_4k_disp_set_sd_video_effect_coef(0x808080);
        drv_reg_4k_disp_set_color_enhance_ctrl(0x47f04000);

        disp_set_csc_onoff(MT_TRUE);
        disp_set_denoise_onoff(MT_FALSE);
    }
    else if (ppMode == MT_DRV_DISP_PPMODE_VIVID)
    {

        drv_reg_4k_disp_set_hd_video_post_config(0x20080800);
        drv_reg_4k_disp_set_sd_video_post_config(0);
        drv_reg_4k_disp_set_hd_video_effect_coef(0x808080);
        drv_reg_4k_disp_set_sd_video_effect_coef(0x808080);
        drv_reg_4k_disp_set_color_enhance_ctrl(0x47f04001);

        disp_set_csc_onoff(MT_TRUE);
        disp_set_denoise_onoff(MT_TRUE);
    }
    else
    {
        MT_ASSERT(0);
    }

    disp_hal_set_sd_venc(p_dp);

    return nRet;
}

mt_s32 disp_set_vid_layer_show(MT_BOOL bEnable)
{
    mt_s32 nRet = MT_SUCCESS;

    LayerEnable[DISP_LAYER_ID_VIDEO_SD] = bEnable;
    LayerEnable[DISP_LAYER_ID_VIDEO_HD] = bEnable;

    LayerEnable[DISP_LAYER_ID_STILL_SD] = bEnable;
    LayerEnable[DISP_LAYER_ID_STILL_HD] = bEnable;    
    DISP_DEBUGK("%s: enable=%d\n",__FUNCTION__,bEnable);

    return nRet;
}

mt_s32 disp_get_vid_layer_show(MT_BOOL *pbEnable)
{
    mt_s32 nRet = MT_SUCCESS;

    *pbEnable = (MT_BOOL)drv_reg_4k_disp_get_video_ctrl_1_video_sel();

 //   DISP_DEBUGK("%s: get vid layer enable=%d\n",__FUNCTION__,*pbEnable);

    return nRet;
}

mt_s32 disp_get_dce_percent(mt_s32 ChanID, mt_u32 *pDce)
{
    mt_s32 nRet = MT_SUCCESS;

    switch (ChanID)
    {
    case 0:
        *pDce = DSP_DCE_PERCENT_CHANNEL0;
        break;
    case 1:
        *pDce = DSP_DCE_PERCENT_CHANNEL1;
        break;
    case 2:
        *pDce = DSP_DCE_PERCENT_CHANNEL2;
        break;
    case 3:
        *pDce = DSP_DCE_PERCENT_CHANNEL3;
        break;
    }

    return nRet;
}

mt_u32 disp_set_layer_alpha(MT_DRV_DISPLAY_E enDisp, mt_u32 alpha)
{
    mt_s32 nRet = MT_SUCCESS;

    switch (enDisp)
    {
    case MT_DRV_DISPLAY_1:
    case MT_DRV_DISPLAY_0:
        drv_reg_4k_disp_set_video_ctrl_2_video_plane_alpha(alpha);
        drv_reg_4k_disp_set_sd_video_plane_alpha(alpha);
        break;
    default:
        break;
    }

    return nRet;
}

mt_u32 disp_get_layer_alpha(MT_DRV_DISPLAY_E enDisp, mt_u32 *alpha)
{
    mt_s32 nRet = MT_SUCCESS;

    switch (enDisp)
    {
    case MT_DRV_DISPLAY_1:
    case MT_DRV_DISPLAY_0:
        *alpha = drv_reg_4k_disp_get_video_ctrl_2_video_plane_alpha();
        break;
    default:
        break;
    }

    return nRet;
}

mt_u32 disp_set_csc_onoff(MT_BOOL  bEnable)
{
    disp_priv_t *p_dp = &s_stDisplayPriv;

    MT_ASSERT(NULL != p_dp);

    p_dp->csc_info.csc_enable = bEnable;
    p_dp->csc_info.old_resolution = 0xff;
    if(p_dp->csc_info.csc_enable == MT_FALSE)
    {
        drv_reg_4k_disp_set_still_csc_ctrl_still_csc_en(0x0);
        drv_reg_4k_disp_set_hd_csc_ctrl_hd_csc_en(0);
        drv_reg_4k_disp_set_sd_csc_ctrl_sd_csc_en(0);
        drv_reg_4k_disp_set_sd_csc_ctrl_gra_csc_en(0);

        drv_reg_4k_disp_set_sd_new_csc_ctrl_new_csc_en(0);
        drv_reg_4k_disp_set_adv_csc_ctrl_adv_csc_en(0x0);
        drv_reg_4k_disp_set_osd_new_csc_ctrl_new_csc_en(0x0);
    }
    return MT_SUCCESS;

}

mt_u32 disp_set_denoise_onoff(MT_BOOL  bEnable)
{
    disp_priv_t *p_dp = &s_stDisplayPriv;

    MT_ASSERT(NULL != p_dp);
    p_dp->denoise_info.denoise_enable = bEnable;
    p_dp->denoise_info.old_resolution = 0xff;
    if(p_dp->denoise_info.denoise_enable == MT_FALSE)
    {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
        //todo...

#endif
    }

    return MT_SUCCESS;

}

mt_u32 disp_set_afd_onoff(MT_BOOL  bEnable)
{
    mt_s32 nRet = MT_SUCCESS;

    AfdEnable = bEnable;

    return nRet;
}

extern mt_s32 MT_DRV_HDMI_EMP_Cfg(MT_UNF_HDMI_ID_E enHdmi, void *p_param, mt_u32 len);
mt_void HdmiEmpRun(disp_priv_t *p_dp, MT_DF_VIDEO_INFO* p_vid_info)
{
    EMP_DATA_IN_T emp_data = {0};
    HDR_DATA_TYPE_T type = HDR_DATA_TYPE_HDR;
    hdmi_vsif_infoframe_t * p_vsif_infoframe = &p_dp->hdmi_vcfg.output_v_cfg.vsif_infoframe;
    hdr_metadata_t *p_hdr_metadata = (hdr_metadata_t*)p_vid_info->hdr10p_common_info_addr;
    mt_u8 graphics_overlay_flag = 0;
    mt_u8 i = 0, check_sum = 0;

    if(!p_dp || !p_vid_info || !p_vsif_infoframe || !p_hdr_metadata)
    {
        DISP_DEBUGK("%s Invalid param!!! \n", __FUNCTION__);
        return;
    }

    if((p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10P)
            && (p_dp->csc_info.hdr10p_metadata_flag || p_dp->csc_info.sl_hdr_metadata_flag))
    {
        if(p_hdr_metadata->hdr_dynmaic_metadata_common.dynamic_metadata_vsif.vsif_data_valid && (hdr10p_vsif_mode == 1))
            type = HDR_DATA_TYPE_VSIF;
        else
            type = HDR_DATA_TYPE_EMP;
    }
    else if(p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10)
        type = HDR_DATA_TYPE_HDR;
    else if(p_dp->hdmi_info.used_tv_mode == TV_MODE_HLG)
        type = HDR_DATA_TYPE_HLG;

    if(type == HDR_DATA_TYPE_EMP)
    {
        emp_data.mode = type;
        emp_data.datap = &p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0];
        emp_data.datal = sizeof(hdmi_usrdf_infoframe_t);
        emp_data.data1p = p_hdr_metadata->hdr_dynmaic_metadata_common.sei_payload;
        emp_data.data1l = p_hdr_metadata->hdr_dynmaic_metadata_common.payload_size;
        emp_data.emp_type = EMP_TYPE_HDR_DYNAMIC;
        emp_data.itype = p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type;
        emp_data.end = 0;
    }
    else if(type == HDR_DATA_TYPE_VSIF)
    {
        dynamic_metadata_vsif_t *p_dynamic_metadata_vsif = &p_hdr_metadata->hdr_dynmaic_metadata_common.dynamic_metadata_vsif;
        graphics_overlay_flag = drv_reg_4k_disp_get_osdl_osd0_cmd_osd_layer_en() | \
                                drv_reg_4k_disp_get_osdl_osd1_cmd_osd_layer_en() | \
                                drv_reg_4k_disp_get_osdl_sub_cmd_osd_layer_en();
        p_vsif_infoframe->type = 0x81;
        p_vsif_infoframe->version = 0x01;
        p_vsif_infoframe->length = 27;

        p_vsif_infoframe->pb_byte[1] = 0x8B;
        p_vsif_infoframe->pb_byte[2] = 0x84;
        p_vsif_infoframe->pb_byte[3] = 0x90;
        p_vsif_infoframe->pb_byte[4] = (1<<6) | ((p_dynamic_metadata_vsif->targeted_system_display_maximum_luminance & 0x1f) << 1);

        p_vsif_infoframe->pb_byte[5] =  p_dynamic_metadata_vsif->average_maxrgb;
        p_vsif_infoframe->pb_byte[6] =  p_dynamic_metadata_vsif->distribution_values[0];
        p_vsif_infoframe->pb_byte[7] =  p_dynamic_metadata_vsif->distribution_values[1];
        p_vsif_infoframe->pb_byte[8] =  p_dynamic_metadata_vsif->distribution_values[2];
        p_vsif_infoframe->pb_byte[9] =  p_dynamic_metadata_vsif->distribution_values[3];
        p_vsif_infoframe->pb_byte[10] =  p_dynamic_metadata_vsif->distribution_values[4];
        p_vsif_infoframe->pb_byte[11] =  p_dynamic_metadata_vsif->distribution_values[5];
        p_vsif_infoframe->pb_byte[12] =  p_dynamic_metadata_vsif->distribution_values[6];
        p_vsif_infoframe->pb_byte[13] =  p_dynamic_metadata_vsif->distribution_values[7];
        p_vsif_infoframe->pb_byte[14] =  p_dynamic_metadata_vsif->distribution_values[8];
        p_vsif_infoframe->pb_byte[15] =  (p_dynamic_metadata_vsif->num_bezier_curve_anchors << 4) | \
                                         ((p_dynamic_metadata_vsif->knee_point_x >> 6) & 0xf);
        p_vsif_infoframe->pb_byte[16] =  ((p_dynamic_metadata_vsif->knee_point_x & 0x3f) << 2) | \
                                         ((p_dynamic_metadata_vsif->knee_point_y >> 8) & 0x3);
        p_vsif_infoframe->pb_byte[17] =  p_dynamic_metadata_vsif->knee_point_y & 0xff;
        p_vsif_infoframe->pb_byte[18] =  p_dynamic_metadata_vsif->bezier_curve_anchors[0];
        p_vsif_infoframe->pb_byte[19] =  p_dynamic_metadata_vsif->bezier_curve_anchors[1];
        p_vsif_infoframe->pb_byte[20] =  p_dynamic_metadata_vsif->bezier_curve_anchors[2];
        p_vsif_infoframe->pb_byte[21] =  p_dynamic_metadata_vsif->bezier_curve_anchors[3];
        p_vsif_infoframe->pb_byte[22] =  p_dynamic_metadata_vsif->bezier_curve_anchors[4];
        p_vsif_infoframe->pb_byte[23] =  p_dynamic_metadata_vsif->bezier_curve_anchors[5];
        p_vsif_infoframe->pb_byte[24] =  p_dynamic_metadata_vsif->bezier_curve_anchors[6];
        p_vsif_infoframe->pb_byte[25] =  p_dynamic_metadata_vsif->bezier_curve_anchors[7];
        p_vsif_infoframe->pb_byte[26] =  p_dynamic_metadata_vsif->bezier_curve_anchors[8];
        p_vsif_infoframe->pb_byte[27] =  graphics_overlay_flag << 7 | 1<<6 ;

        check_sum = p_vsif_infoframe->type;
        check_sum += p_vsif_infoframe->version;
        check_sum += p_vsif_infoframe->length;
        for(i = 1; i <= 27; i++)
            check_sum += p_vsif_infoframe->pb_byte[i];

        p_vsif_infoframe->pb_byte[0] = 256 - check_sum;

        emp_data.mode = type;
        emp_data.datap = &p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0];
        emp_data.datal = sizeof(hdmi_usrdf_infoframe_t);
        emp_data.data1p = p_vsif_infoframe;
        emp_data.data1l = sizeof(hdmi_vsif_infoframe_t);
        emp_data.emp_type = EMP_TYPE_HDR_DYNAMIC;
        emp_data.itype = p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type;
        emp_data.end = 0;
    }
    else 
    {
        return;
    }

    MT_DRV_HDMI_EMP_Cfg(0, &emp_data, sizeof(EMP_DATA_IN_T));

    // This function is only used to test EMP DMA on FPGA
    //MT_DRV_HDMI_EMP_Cfg(0, &emp_data, sizeof(EMP_DATA_IN_T));
}

MT_DRV_DISP_STEREO_E packing_type_to_3d_mode(MT_BOOL is_3D_flag, DISP_FRAME_PACKING_TYPE_E packing_type)
{
    MT_DRV_DISP_STEREO_E stereo_mode = DISP_STEREO_NONE;
    if(is_3D_flag)
    {
        switch (packing_type)
        {
        case DISP_FRAME_PACKING_TYPE_NONE:
            stereo_mode = DISP_STEREO_NONE;
            break;
        case DISP_FRAME_PACKING_TYPE_SIDE_BY_SIDE:
            stereo_mode = DISP_STEREO_SBS_HALF;
            break;
        case DISP_FRAME_PACKING_TYPE_TOP_BOTTOM:
            stereo_mode = DISP_STEREO_TAB;
            break;
        case DISP_FRAME_PACKING_TYPE_TIME_INTERLACED:
            stereo_mode = DISP_STEREO_FPK;  //TBD
            break;

        default:
            stereo_mode = DISP_STEREO_NONE;
            break;

        }
    }
    else
    {
        stereo_mode = DISP_STEREO_NONE;
    }

    return stereo_mode;
}


extern WINDOW_S *g_pstWin[];
extern mt_u32 DRV_HDMI_GetDeepColor(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_HDMI_DEEP_COLOR_E *penDeepColor);
static void disp_csc_update(disp_priv_t *p_dp)
{
    MT_BOOL b_on = 0;
    static MT_BOOL b_old_on = 0;
    MT_DF_VIDEO_INFO curr_vid_info = {0};
    MT_BOOL b_hdr_info_changed = MT_FALSE;
    hdr_metadata_t *p_hdr_metadata = NULL;
    u8 vsif_data_valid = 0; 
    static u8 old_vsif_data_valid = 0;
    static MT_U8 cur_hdr10p_vsif_mode = 1;
#ifdef SUPPORT_HDR
    hdmi_usrdf_infoframe_t hdmi_infoframe = {0};
    PicHdrInfo_t *g_p_hdr_info = &(p_dp->hdr_info);
    static fw_mastering_disp_colour_volume_t old_colour_volume = {0};
    static fw_content_light_level_info_t old_light_level_info = {0};
#endif
    mt_u32 index_video = Win_GetVideoWinIndex();
    mt_u32 index_still = Win_GetStillWinIndex();

    b_on = drv_reg_4k_disp_get_video_ctrl_1_video_sel();
    if(!b_on)
    {
        g_slhdr_enable = 0;
    }

    if((index_video < MAX_WIN_NUM) && (g_pstWin[index_video] != MT_NULL))
    {
        DF_GetCurrVideoInfo(g_pstWin[index_video]->stBuffer.stDispBP, &curr_vid_info);
        if(!curr_vid_info.bValid)
            return;

        p_dp->csc_info.video_full_range = curr_vid_info.video_range;
        p_dp->hdmi_vcfg.input_v_cfg.video_full_range = p_dp->csc_info.video_full_range;
    }

    if(p_dp->csc_info.csc_enable == MT_FALSE && g_tv_capability == MT_DRV_DISP_HDMI_MODE_SDR)
    {
        if(p_dp->csc_info.video_full_range != p_dp->csc_info.old_video_full_range)
        {
            schedule_work(&p_dp->work_hdmi_cb);
            p_dp->csc_info.old_video_full_range = p_dp->csc_info.video_full_range;
        }
        return;
    }
    
    
    if(!b_on)
    {
        p_dp->csc_info.cur_colour_primaries = 1;
    }
    else if(index_video < MAX_WIN_NUM)
    {        
        if(b_disp_coeff_update)  //force update hdr config
        {
            memset(&old_colour_volume, 0, sizeof(old_colour_volume));
            old_used_tv_mode = TV_MODE_SDR;
            p_dp->csc_info.old_transfer_characteristics = 0;
            p_dp->csc_info.old_colour_primaries = 0;
            p_dp->csc_info.old_hdr10p_metadata_flag = 0;
            p_dp->csc_info.old_sl_hdr_metadata_flag = 0;
        }
    
        p_hdr_metadata = (hdr_metadata_t*)curr_vid_info.hdr10p_common_info_addr;
        if(!p_dp->usr_force_3d)
        {
            p_dp->cur_3d_flag = curr_vid_info.bIs3D; //3D
            p_dp->cur_mode_3d = packing_type_to_3d_mode(curr_vid_info.bIs3D, curr_vid_info.frame_packing_type); //3D
            p_dp->cur_extpara_3d = HDMI_3D_HORIZONTAL; //should be get from fw, temply fixed ---dean
        }

        p_dp->csc_info.cur_transfer_characteristics = curr_vid_info.transfer_characteristics;
        p_dp->csc_info.cur_colour_primaries = curr_vid_info.colour_primaries;
        p_dp->csc_info.sub_colour_primaries_2020 = curr_vid_info.sub_colour_primaries_2020;
        if(p_hdr_metadata && curr_vid_info.hdr10p_dynamic_metadata_valid_flag)
        {
            p_dp->csc_info.hdr10p_metadata_flag = curr_vid_info.hdr10p_dynamic_metadata_valid_flag && \
                                                  (p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type == 4);
            p_dp->csc_info.sl_hdr_metadata_flag = curr_vid_info.hdr10p_dynamic_metadata_valid_flag && \
                                                  (p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type == 2);
            p_dp->csc_info.vivid_hdr_metadata_flag = curr_vid_info.hdr10p_dynamic_metadata_valid_flag && \
                                                  (p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type != 2) && \
                                                  (p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type != 4);
            vsif_data_valid = p_hdr_metadata->hdr_dynmaic_metadata_common.dynamic_metadata_vsif.vsif_data_valid;
        }
        else
        {
            p_dp->csc_info.hdr10p_metadata_flag = 0;
            p_dp->csc_info.sl_hdr_metadata_flag = 0;
            p_dp->csc_info.vivid_hdr_metadata_flag = 0;
            vsif_data_valid = 0;
        }

        p_dp->hdr_info.colour_primaries = curr_vid_info.colour_primaries;
        p_dp->hdr_info.transfer_characteristics = curr_vid_info.transfer_characteristics;
#ifdef SUPPORT_HDR
        p_dp->hdr_info.colour_volume.colour_volume_enable = curr_vid_info.colour_volume.colour_volume_enable;
        p_dp->hdr_info.colour_volume.white_point_x = curr_vid_info.colour_volume.white_point_x;
        p_dp->hdr_info.colour_volume.white_point_y = curr_vid_info.colour_volume.white_point_y;
        p_dp->hdr_info.colour_volume.max_luminance = curr_vid_info.colour_volume.max_luminance;
        p_dp->hdr_info.colour_volume.min_luminance = curr_vid_info.colour_volume.min_luminance;
        p_dp->hdr_info.colour_volume.display_primaries_x[0] = curr_vid_info.colour_volume.display_primaries_x[0];
        p_dp->hdr_info.colour_volume.display_primaries_x[1] = curr_vid_info.colour_volume.display_primaries_x[1];
        p_dp->hdr_info.colour_volume.display_primaries_x[2] = curr_vid_info.colour_volume.display_primaries_x[2];
        p_dp->hdr_info.colour_volume.display_primaries_y[0] = curr_vid_info.colour_volume.display_primaries_y[0];
        p_dp->hdr_info.colour_volume.display_primaries_y[1] = curr_vid_info.colour_volume.display_primaries_y[1];
        p_dp->hdr_info.colour_volume.display_primaries_y[2] = curr_vid_info.colour_volume.display_primaries_y[2];
        p_dp->hdr_info.light_level.light_level_info_enable = curr_vid_info.light_level.light_level_info_enable;
        p_dp->hdr_info.light_level.max_light_level = curr_vid_info.light_level.max_light_level;
        p_dp->hdr_info.light_level.max_pic_ave_light_level = curr_vid_info.light_level.max_pic_ave_light_level;

        if(g_p_hdr_info->colour_volume.display_primaries_x[0] != old_colour_volume.display_primaries_x[0]
                || g_p_hdr_info->colour_volume.display_primaries_x[1] != old_colour_volume.display_primaries_x[1]
                || g_p_hdr_info->colour_volume.display_primaries_x[2] != old_colour_volume.display_primaries_x[2]
                || g_p_hdr_info->colour_volume.display_primaries_y[0] != old_colour_volume.display_primaries_y[0]
                || g_p_hdr_info->colour_volume.display_primaries_y[1] != old_colour_volume.display_primaries_y[1]
                || g_p_hdr_info->colour_volume.display_primaries_y[2] != old_colour_volume.display_primaries_y[2]
                || g_p_hdr_info->colour_volume.white_point_x != old_colour_volume.white_point_x
                || g_p_hdr_info->colour_volume.white_point_y != old_colour_volume.white_point_y
                || g_p_hdr_info->colour_volume.max_luminance != old_colour_volume.max_luminance
                || g_p_hdr_info->colour_volume.min_luminance != old_colour_volume.min_luminance
                || g_p_hdr_info->light_level.max_light_level != old_light_level_info.max_light_level
                || g_p_hdr_info->light_level.max_pic_ave_light_level!= old_light_level_info.max_pic_ave_light_level
                || (p_dp->hdr_para.hdr_display_Brightness != 0 && p_dp->hdr_para.hdr_display_Brightness != g_stDrvSetting[index_video].hdr_info_from_drv.hdr_display_brightness_from_user)
                || (p_dp->hdr_para.sdr_display_Brightness != 0 && p_dp->hdr_para.sdr_display_Brightness != g_stDrvSetting[index_video].hdr_info_from_drv.sdr_display_brightness_from_user)
                || g_stDrvSetting[index_video].hdr_info_from_drv.tuning_level != p_dp->hdr_para.target_display_adaptation_tuning_level
                || g_stDrvSetting[index_video].hdr_info_from_drv.bTransparent != p_dp->hdr_para.transparent_mode
                || (curr_vid_info.hdr10p_dynamic_metadata_valid_flag && (old_vsif_data_valid != vsif_data_valid))
                || (cur_hdr10p_vsif_mode != hdr10p_vsif_mode)
          )
        {
            old_colour_volume = g_p_hdr_info->colour_volume;
            old_light_level_info = g_p_hdr_info->light_level;
            cur_hdr10p_vsif_mode = hdr10p_vsif_mode;
            old_vsif_data_valid = vsif_data_valid;
            b_hdr_info_changed = MT_TRUE;
            DISP_DEBUGK("%s hdr info changed!! \n", __FUNCTION__);
        }
#endif
    }

    DRV_HDMI_GetDeepColor(MT_UNF_HDMI_ID_0, &p_dp->csc_info.cur_deep_color);

    p_dp->csc_info.cur_resolution = reg_aria_hd_encoder_get_basic_cfg_resolusion();
    //0:1920*1080,1:1280*720,2:720*576,3:720*480
    p_dp->csc_info.cur_use_3scaler = drv_reg_4k_disp_get_sd_video_path_ctrl_sd_video_path_en();

    p_dp->csc_info.cur_tv_capability =  g_tv_capability;
    p_dp->csc_info.cur_tv_sys = p_dp->disp_out_info[DISP_CHANNEL_HD].vid_fmt;
    if(((0 != p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h) &&
                (0 != p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w) &&
                ((p_dp->csc_info.cur_transfer_characteristics != p_dp->csc_info.old_transfer_characteristics) ||
                 (p_dp->csc_info.cur_colour_primaries != p_dp->csc_info.old_colour_primaries) ||
                 (p_dp->csc_info.cur_use_3scaler != p_dp->csc_info.old_use_3scaler) ||
                 (p_dp->csc_info.cur_tv_sys != p_dp->csc_info.old_tv_sys) ||
                 (p_dp->csc_info.sub_colour_primaries_2020 != p_dp->csc_info.old_sub_colour_primaries_2020) ||
                 (p_dp->csc_info.hdr10p_metadata_flag != p_dp->csc_info.old_hdr10p_metadata_flag) ||
                 (p_dp->csc_info.sl_hdr_metadata_flag != p_dp->csc_info.old_sl_hdr_metadata_flag) ||
                 (p_dp->csc_info.vivid_hdr_metadata_flag != p_dp->csc_info.old_vivid_hdr_metadata_flag)  ))
            || (p_dp->csc_info.cur_tv_capability != p_dp->csc_info.old_tv_capability)
            || (p_dp->csc_info.cur_resolution != p_dp->csc_info.old_resolution)
            || (p_dp->csc_info.video_full_range != p_dp->csc_info.old_video_full_range)
            || (b_hdr_info_changed == MT_TRUE)
            || (b_old_on != b_on)
            || (p_dp->csc_info.old_deep_color != p_dp->csc_info.cur_deep_color))
    {
        DISP_DEBUGK("csc_update: \n");
        DISP_DEBUGK("old_transfer_characteristics=%d, old_colour_primaries=%d, old_tv_capability=%d, old_full_range:%d\n",
                p_dp->csc_info.old_transfer_characteristics,
                p_dp->csc_info.old_colour_primaries,
                p_dp->csc_info.old_tv_capability,
                p_dp->csc_info.old_video_full_range);
                
        DISP_DEBUGK("cur_transfer_characteristics=%d, cur_colour_primaries=%d, cur_tv_capability=%d, cur_full_range:%d\n",
                p_dp->csc_info.cur_transfer_characteristics,
                p_dp->csc_info.cur_colour_primaries,
                p_dp->csc_info.cur_tv_capability,
                p_dp->csc_info.video_full_range);

        DISP_DEBUGK("old_use_3scaler=%d, old_tv_sys=%d, old_resolution=%d\n",
                        p_dp->csc_info.old_use_3scaler,
                        p_dp->csc_info.old_tv_sys,
                        p_dp->csc_info.old_resolution);  

        DISP_DEBUGK("cur_use_3scaler=%d, cur_tv_sys=%d, cur_resolution=%d\n",
                p_dp->csc_info.cur_use_3scaler,
                p_dp->csc_info.cur_tv_sys,
                p_dp->csc_info.cur_resolution);   

         DISP_DEBUGK("old_sub_colour_primaries_2020=%d, old_sl_hdr_flag=%d, old_hdr10p_flag=%d, old_vivid_flag=%d\n",
                p_dp->csc_info.old_sub_colour_primaries_2020,
                p_dp->csc_info.old_sl_hdr_metadata_flag,
                p_dp->csc_info.old_hdr10p_metadata_flag,
                p_dp->csc_info.old_vivid_hdr_metadata_flag);
         
        DISP_DEBUGK("cur_sub_colour_primaries_2020=%d, cur_sl_hdr_flag=%d, cur_hdr10p_flag=%d vivid_flag=%d\n",
                p_dp->csc_info.sub_colour_primaries_2020,
                p_dp->csc_info.sl_hdr_metadata_flag,
                p_dp->csc_info.hdr10p_metadata_flag,
                p_dp->csc_info.vivid_hdr_metadata_flag);
        if(p_hdr_metadata)
        {
            DISP_DEBUGK("hdr10p_dynamic_metadata_valid_flag=%d, hdr_dynamic_metadata_type=%d, vsif_data_valid=%d\n",
                    curr_vid_info.hdr10p_dynamic_metadata_valid_flag,
                    p_hdr_metadata->hdr_dynmaic_metadata_common.hdr_dynamic_metadata_type,
                    p_hdr_metadata->hdr_dynmaic_metadata_common.dynamic_metadata_vsif.vsif_data_valid);
        }
        DISP_DEBUGK("3d info: cur_3d_flag:%d cur_mode_3d:%d cur_extpara_3d:%d\n", p_dp->cur_3d_flag, p_dp->cur_mode_3d, p_dp->cur_extpara_3d);

#if defined(SUPPORT_HDR) && defined(CONFIG_HDMI)
        DISP_DEBUGK("metadata: %d, x=[%d, %d, %d], y=[%d, %d, %d], white_point=[%d, %d]\n",
                g_p_hdr_info->colour_volume.colour_volume_enable,
                g_p_hdr_info->colour_volume.display_primaries_x[0],g_p_hdr_info->colour_volume.display_primaries_x[1], g_p_hdr_info->colour_volume.display_primaries_x[2],
                g_p_hdr_info->colour_volume.display_primaries_y[0], g_p_hdr_info->colour_volume.display_primaries_y[1], g_p_hdr_info->colour_volume.display_primaries_y[2],
                g_p_hdr_info->colour_volume.white_point_x, g_p_hdr_info->colour_volume.white_point_y);
        DISP_DEBUGK("metadata: lum=[%d, %d], light_level_enable:%d, max_light=%d, ave_light=%d\n",
                g_p_hdr_info->colour_volume.max_luminance, g_p_hdr_info->colour_volume.min_luminance,
                g_p_hdr_info->light_level.light_level_info_enable, g_p_hdr_info->light_level.max_light_level, g_p_hdr_info->light_level.max_pic_ave_light_level);

        if(g_tv_capability == MT_DRV_DISP_HDMI_MODE_AUTO)
        {
            if((p_dp->hdmi_info.edid.hdr10p_emp || p_dp->hdmi_info.edid.supported_hdr10p_vsif)
                    && (p_dp->csc_info.hdr10p_metadata_flag))
            {
                if((vsif_data_valid && (p_dp->hdmi_info.edid.supported_hdr10p_vsif))
                    || ((0 == vsif_data_valid) && (p_dp->hdmi_info.edid.hdr10p_emp)))
                {
                    if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))
                        p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10P;
                    else
                        p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10P_BT709;
                }
                else 
                {
                    if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))
                        p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10;
                    else
                        p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10_BT709;
                }
            }
            else if((p_dp->hdmi_info.edid.supported_hdr10 == TRUE)
                    && ((g_p_hdr_info->transfer_characteristics == 16)
                         || p_dp->csc_info.sl_hdr_metadata_flag 
                         || p_dp->csc_info.vivid_hdr_metadata_flag))
            {
                if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))  //for sl-hdr bt709
                    p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10;
                else
                    p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10_BT709;
            }
            else if((p_dp->hdmi_info.edid.supported_hlg == TRUE)
                    && (g_p_hdr_info->transfer_characteristics == 18))
            {
                if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))
                    p_dp->hdmi_info.used_tv_mode = TV_MODE_HLG;
                else
                    p_dp->hdmi_info.used_tv_mode = TV_MODE_HLG_BT709;
            }
            else
            {
                if((p_dp->hdmi_info.edid.supported_hdr10 == TRUE)
                    && (g_p_hdr_info->transfer_characteristics == 18))
                {
                    if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))
                        p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10;
                    else
                        p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10_BT709;                
                }
                else
                {
                    p_dp->hdmi_info.used_tv_mode = TV_MODE_SDR;
                }
            }
        }  
        else if((g_tv_capability == MT_DRV_DISP_HDMI_MODE_HDR10) && (p_dp->hdmi_info.edid.supported_hdr10 == TRUE))
        {
            if(p_dp->hdmi_info.supported_bt2020 && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))                
                p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10;
            else
                p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10_BT709;
        }
        else if((g_tv_capability == MT_DRV_DISP_HDMI_MODE_HLG) && (p_dp->hdmi_info.edid.supported_hlg == TRUE) 
            && (p_dp->csc_info.cur_deep_color > MT_UNF_HDMI_DEEP_COLOR_24BIT))
        {
            if(p_dp->hdmi_info.supported_bt2020)
                p_dp->hdmi_info.used_tv_mode = TV_MODE_HLG;
            else
                p_dp->hdmi_info.used_tv_mode = TV_MODE_HLG_BT709;
        }
        else
        {
            p_dp->hdmi_info.used_tv_mode = TV_MODE_SDR;
        }


        DISP_DEBUGK("old_used_tv_mode:%d cur_used_tv_mode:%d\n", old_used_tv_mode, p_dp->hdmi_info.used_tv_mode);
        if((old_used_tv_mode != p_dp->hdmi_info.used_tv_mode) 
            || b_hdr_info_changed)
        {
            if(p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10P || p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10P_BT709)
            {
                //DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                //msleep(50);

                hdmi_infoframe.type = 0x87;
                hdmi_infoframe.version = 1;
                hdmi_infoframe.length = 26;
                hdmi_infoframe.pb_byte[0] = 2;  //EOTF
                hdmi_infoframe.pb_byte[1] = 0;  //metadata_description_ID
                if(g_p_hdr_info->colour_volume.colour_volume_enable)
                {
                    hdmi_infoframe.pb_byte[2] = g_p_hdr_info->colour_volume.display_primaries_x[0] & 0xff;
                    hdmi_infoframe.pb_byte[3] = (g_p_hdr_info->colour_volume.display_primaries_x[0] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[4] = g_p_hdr_info->colour_volume.display_primaries_y[0] & 0xff;
                    hdmi_infoframe.pb_byte[5] = (g_p_hdr_info->colour_volume.display_primaries_y[0] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[6] = g_p_hdr_info->colour_volume.display_primaries_x[1] & 0xff;
                    hdmi_infoframe.pb_byte[7] = (g_p_hdr_info->colour_volume.display_primaries_x[1] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[8] = g_p_hdr_info->colour_volume.display_primaries_y[1] & 0xff;
                    hdmi_infoframe.pb_byte[9] = (g_p_hdr_info->colour_volume.display_primaries_y[1] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[10] = g_p_hdr_info->colour_volume.display_primaries_x[2] & 0xff;
                    hdmi_infoframe.pb_byte[11] = (g_p_hdr_info->colour_volume.display_primaries_x[2] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[12] = g_p_hdr_info->colour_volume.display_primaries_y[2] & 0xff;
                    hdmi_infoframe.pb_byte[13] = (g_p_hdr_info->colour_volume.display_primaries_y[2] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[14] = g_p_hdr_info->colour_volume.white_point_x & 0xff;
                    hdmi_infoframe.pb_byte[15] = (g_p_hdr_info->colour_volume.white_point_x & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[16] = g_p_hdr_info->colour_volume.white_point_y & 0xff;
                    hdmi_infoframe.pb_byte[17] = (g_p_hdr_info->colour_volume.white_point_y & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[18] = (g_p_hdr_info->colour_volume.max_luminance /10000) & 0xff;
                    hdmi_infoframe.pb_byte[19] = ((g_p_hdr_info->colour_volume.max_luminance /10000) & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[20] = g_p_hdr_info->colour_volume.min_luminance & 0xff;
                    hdmi_infoframe.pb_byte[21] = (g_p_hdr_info->colour_volume.min_luminance & 0xff00) >> 8;
                }
                if(g_p_hdr_info->light_level.light_level_info_enable)
                {
                    hdmi_infoframe.pb_byte[22] = g_p_hdr_info->light_level.max_light_level & 0xff;
                    hdmi_infoframe.pb_byte[23] = (g_p_hdr_info->light_level.max_light_level & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[24] = g_p_hdr_info->light_level.max_pic_ave_light_level & 0xff;
                    hdmi_infoframe.pb_byte[25] = (g_p_hdr_info->light_level.max_pic_ave_light_level & 0xff00) >> 8;
                }

                memcpy(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0],  &hdmi_infoframe,  sizeof(hdmi_usrdf_infoframe_t));
                //p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10P;
                //DRV_HDMI_UserDefinedInfoFrameSet(MT_UNF_HDMI_ID_0, HDMI_USRDF_INFO_HDR, &hdmi_infoframe); //dean----hdmi not ready

                DISP_DEBUGK("%s %d: used_tv_mode: %d\n", __FUNCTION__, __LINE__,p_dp->hdmi_info.used_tv_mode);
                //disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);
                //p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                //reset_delay_cnt = 0;
                //update_sys_flag = 1;


            }
            else if(p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10 || p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10_BT709)
            {
                //DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                //msleep(50);

                hdmi_infoframe.type = 0x87;
                hdmi_infoframe.version = 1;
                hdmi_infoframe.length = 26;
                hdmi_infoframe.pb_byte[0] = 2;  //EOTF
                hdmi_infoframe.pb_byte[1] = 0;  //metadata_description_ID
                if(g_p_hdr_info->colour_volume.colour_volume_enable)
                {
                    hdmi_infoframe.pb_byte[2] = g_p_hdr_info->colour_volume.display_primaries_x[0] & 0xff;
                    hdmi_infoframe.pb_byte[3] = (g_p_hdr_info->colour_volume.display_primaries_x[0] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[4] = g_p_hdr_info->colour_volume.display_primaries_y[0] & 0xff;
                    hdmi_infoframe.pb_byte[5] = (g_p_hdr_info->colour_volume.display_primaries_y[0] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[6] = g_p_hdr_info->colour_volume.display_primaries_x[1] & 0xff;
                    hdmi_infoframe.pb_byte[7] = (g_p_hdr_info->colour_volume.display_primaries_x[1] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[8] = g_p_hdr_info->colour_volume.display_primaries_y[1] & 0xff;
                    hdmi_infoframe.pb_byte[9] = (g_p_hdr_info->colour_volume.display_primaries_y[1] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[10] = g_p_hdr_info->colour_volume.display_primaries_x[2] & 0xff;
                    hdmi_infoframe.pb_byte[11] = (g_p_hdr_info->colour_volume.display_primaries_x[2] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[12] = g_p_hdr_info->colour_volume.display_primaries_y[2] & 0xff;
                    hdmi_infoframe.pb_byte[13] = (g_p_hdr_info->colour_volume.display_primaries_y[2] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[14] = g_p_hdr_info->colour_volume.white_point_x & 0xff;
                    hdmi_infoframe.pb_byte[15] = (g_p_hdr_info->colour_volume.white_point_x & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[16] = g_p_hdr_info->colour_volume.white_point_y & 0xff;
                    hdmi_infoframe.pb_byte[17] = (g_p_hdr_info->colour_volume.white_point_y & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[18] = (g_p_hdr_info->colour_volume.max_luminance /10000) & 0xff;
                    hdmi_infoframe.pb_byte[19] = ((g_p_hdr_info->colour_volume.max_luminance /10000) & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[20] = g_p_hdr_info->colour_volume.min_luminance & 0xff;
                    hdmi_infoframe.pb_byte[21] = (g_p_hdr_info->colour_volume.min_luminance & 0xff00) >> 8;
                }
                if(g_p_hdr_info->light_level.light_level_info_enable)
                {
                    hdmi_infoframe.pb_byte[22] = g_p_hdr_info->light_level.max_light_level & 0xff;
                    hdmi_infoframe.pb_byte[23] = (g_p_hdr_info->light_level.max_light_level & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[24] = g_p_hdr_info->light_level.max_pic_ave_light_level & 0xff;
                    hdmi_infoframe.pb_byte[25] = (g_p_hdr_info->light_level.max_pic_ave_light_level & 0xff00) >> 8;
                }

                memcpy(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0],  &hdmi_infoframe,  sizeof(hdmi_usrdf_infoframe_t));
                //p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10;

                DISP_DEBUGK("%s %d: used_tv_mode: %d\n", __FUNCTION__, __LINE__,p_dp->hdmi_info.used_tv_mode);
                //disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);
                //p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                //reset_delay_cnt = 0;
                //update_sys_flag = 1;
            }
            else if(p_dp->hdmi_info.used_tv_mode == TV_MODE_HLG || p_dp->hdmi_info.used_tv_mode == TV_MODE_HLG_BT709)
            {
                //DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                //msleep(50);

                hdmi_infoframe.type = 0x87;
                hdmi_infoframe.version = 1;
                hdmi_infoframe.length = 2;
                hdmi_infoframe.pb_byte[0] = 3;  //EOTF
                hdmi_infoframe.pb_byte[1] = 0;  //static_metadata_descriptor_ID
                memcpy(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0],  &hdmi_infoframe,  sizeof(hdmi_usrdf_infoframe_t));
                //p_dp->hdmi_info.used_tv_mode = TV_MODE_HLG;

                DISP_DEBUGK("%s %d: used_tv_mode: %d\n", __FUNCTION__, __LINE__,p_dp->hdmi_info.used_tv_mode);
                //disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);
                //p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                //reset_delay_cnt = 0;
                //update_sys_flag = 1;
            }
            else if(p_dp->hdmi_info.used_tv_mode == TV_MODE_SDR)
            {
                //DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                //msleep(50);

                //p_dp->hdmi_info.used_tv_mode = TV_MODE_SDR;
                DRV_HDMI_UserPacketRptEnable(MT_UNF_HDMI_ID_0, 0, MT_FALSE);
                memset(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0], 0, sizeof(hdmi_usrdf_infoframe_t));

                DISP_DEBUGK("%s %d: used_tv_mode: %d\n", __FUNCTION__, __LINE__,p_dp->hdmi_info.used_tv_mode);
                //disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);
                //p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                //reset_delay_cnt = 0;
                //update_sys_flag = 1;
            }
            
        }

#endif

        if(index_video < MAX_WIN_NUM || index_still < MAX_WIN_NUM)
        {

            if(index_video < MAX_WIN_NUM)
            {
                g_stDrvSetting[index_video].hdr_info_from_drv.hdr_display_brightness_from_user = p_dp->hdr_para.hdr_display_Brightness;
                g_stDrvSetting[index_video].hdr_info_from_drv.sdr_display_brightness_from_user = p_dp->hdr_para.sdr_display_Brightness;
                g_stDrvSetting[index_video].hdr_info_from_drv.display_brightness_max = p_dp->hdmi_info.brightness_max;

                g_stDrvSetting[index_video].hdr_info_from_drv.bTransparent = p_dp->hdr_para.transparent_mode;
                g_stDrvSetting[index_video].hdr_info_from_drv.tuning_level = p_dp->hdr_para.target_display_adaptation_tuning_level;
                g_stDrvSetting[index_video].hdr_info_from_drv.tv_mode = p_dp->hdmi_info.used_tv_mode;
                g_stDrvSetting[index_video].hdr_info_from_drv.vsif_timing_mode = hdr10p_vsif_mode ? 1 : 0; //can be get from hdmi_info in future  ---dean
                
                DISP_DEBUGK("%s  used_tv_mode:%d bTransparent:%d brightness[%d %d %d]tuning_level:%d vsif_timing_mode:%d\n", 
                        __FUNCTION__,
                        p_dp->hdmi_info.used_tv_mode,
                        p_dp->hdr_para.transparent_mode,
                        g_stDrvSetting[index_video].hdr_info_from_drv.display_brightness_max,
                        g_stDrvSetting[index_video].hdr_info_from_drv.hdr_display_brightness_from_user,
                        g_stDrvSetting[index_video].hdr_info_from_drv.sdr_display_brightness_from_user,                        
                        g_stDrvSetting[index_video].hdr_info_from_drv.tuning_level,
                        g_stDrvSetting[index_video].hdr_info_from_drv.vsif_timing_mode);

                //DF_SetHdrSdrCfg(g_pstWin[index_video]->stBuffer.stDispBP, p_dp->hdmi_info.used_tv_mode);
            }
            
            if(index_still < MAX_WIN_NUM)
            {
                g_stDrvSetting[index_still].hdr_info_from_drv.hdr_display_brightness_from_user = p_dp->hdr_para.hdr_display_Brightness;
                g_stDrvSetting[index_still].hdr_info_from_drv.sdr_display_brightness_from_user = p_dp->hdr_para.sdr_display_Brightness;
                g_stDrvSetting[index_still].hdr_info_from_drv.display_brightness_max = p_dp->hdmi_info.brightness_max;

                g_stDrvSetting[index_still].hdr_info_from_drv.bTransparent = p_dp->hdr_para.transparent_mode;
                g_stDrvSetting[index_still].hdr_info_from_drv.tuning_level = p_dp->hdr_para.target_display_adaptation_tuning_level;
                g_stDrvSetting[index_still].hdr_info_from_drv.tv_mode = p_dp->hdmi_info.used_tv_mode;
                g_stDrvSetting[index_still].hdr_info_from_drv.vsif_timing_mode = hdr10p_vsif_mode ? 1 : 0; //can be get from hdmi_info in future  ---dean
                
                DISP_DEBUGK("%s  used_tv_mode:%d bTransparent:%d brightness[%d %d %d] tuning_level:%d vsif_timing_mode:%d\n", 
                        __FUNCTION__,
                        p_dp->hdmi_info.used_tv_mode,
                        p_dp->hdr_para.transparent_mode,
                        g_stDrvSetting[index_still].hdr_info_from_drv.display_brightness_max,
                        g_stDrvSetting[index_still].hdr_info_from_drv.hdr_display_brightness_from_user,
                        g_stDrvSetting[index_still].hdr_info_from_drv.sdr_display_brightness_from_user,                      
                        g_stDrvSetting[index_still].hdr_info_from_drv.tuning_level,
                        g_stDrvSetting[index_still].hdr_info_from_drv.vsif_timing_mode);

                DISP_DEBUGK("%s  used_tv_mode:%d bTransparent:%d brightness_max:%d tuning_level:%d vsif_timing_mode:%d\n", __FUNCTION__,
                        p_dp->hdmi_info.used_tv_mode,
                        p_dp->hdr_para.transparent_mode,
                        g_stDrvSetting[index_still].hdr_info_from_drv.display_brightness_max,
                        g_stDrvSetting[index_still].hdr_info_from_drv.tuning_level,
                        g_stDrvSetting[index_still].hdr_info_from_drv.vsif_timing_mode);            

                //DF_SetHdrSdrCfg(g_pstWin[index_still]->stBuffer.stDispBP, p_dp->hdmi_info.used_tv_mode);
            }

            if((p_dp->csc_info.old_colour_primaries != p_dp->csc_info.cur_colour_primaries)
                    ||((p_dp->csc_info.old_colour_primaries == 9)
                        && (p_dp->csc_info.cur_colour_primaries == 9)
                        && (p_dp->csc_info.old_sub_colour_primaries_2020 != p_dp->csc_info.sub_colour_primaries_2020))
                    ||(p_dp->csc_info.old_video_full_range != p_dp->csc_info.video_full_range) 
                    ||(old_used_tv_mode != p_dp->hdmi_info.used_tv_mode)
                    || b_hdr_info_changed)
            {
                disp_set_hdmi(p_dp);
                disp_hdmi_video_config(p_dp);
                DISP_DEBUGK("%s :colour_primaries: %d sub_colour_primaries_2020:%d full_range:%d\n", __FUNCTION__,
                        p_dp->csc_info.cur_colour_primaries,
                        p_dp->csc_info.sub_colour_primaries_2020,
                        p_dp->csc_info.video_full_range);
            }
        }

        //only OSD and SD CSC
        {
            DISP_DEBUGK("%s only OSD/SD, used_tv_mode:%d \n", __FUNCTION__,  p_dp->hdmi_info.used_tv_mode);
            DF_UpdateOsdCsc(p_dp->hdmi_info.used_tv_mode);
            DF_UpdateSdCsc(p_dp->hdmi_info.used_tv_mode);
        }       

        p_dp->csc_info.old_transfer_characteristics = p_dp->csc_info.cur_transfer_characteristics;
        p_dp->csc_info.old_colour_primaries = p_dp->csc_info.cur_colour_primaries;
        p_dp->csc_info.old_sub_colour_primaries_2020 = p_dp->csc_info.sub_colour_primaries_2020;
        p_dp->csc_info.old_hdr10p_metadata_flag = p_dp->csc_info.hdr10p_metadata_flag;
        p_dp->csc_info.old_tv_capability = p_dp->csc_info.cur_tv_capability;
        p_dp->csc_info.old_resolution = p_dp->csc_info.cur_resolution;
        p_dp->csc_info.old_tv_sys = p_dp->csc_info.cur_tv_sys;
        p_dp->csc_info.old_use_3scaler = p_dp->csc_info.cur_use_3scaler;
        p_dp->csc_info.old_sl_hdr_metadata_flag = p_dp->csc_info.sl_hdr_metadata_flag;
        p_dp->csc_info.old_video_full_range = p_dp->csc_info.video_full_range;
        p_dp->csc_info.old_vivid_hdr_metadata_flag = p_dp->csc_info.vivid_hdr_metadata_flag;
        old_used_tv_mode = p_dp->hdmi_info.used_tv_mode;
        b_hdr_info_changed = FALSE;
        b_old_on = b_on;
        p_dp->csc_info.old_deep_color = p_dp->csc_info.cur_deep_color;
    }

#if defined(SUPPORT_HDR) && defined(CONFIG_HDMI)

    if(p_dp->csc_info.sl_hdr_metadata_flag 
        //&& ((TV_MODE_HDR10 == p_dp->hdmi_info.used_tv_mode) || (TV_MODE_HDR10P_BT709 == p_dp->hdmi_info.used_tv_mode))
      )
    {
        g_slhdr_enable = 1;
    }
    else
    {
        g_slhdr_enable = 0;
    }

    if(index_video < MAX_WIN_NUM)
    {
        if((p_dp->hdmi_info.used_tv_mode == TV_MODE_HDR10P) && b_on)
        {
            HdmiEmpRun(p_dp, &curr_vid_info);
        }
    }
#endif

}

#if 0
static void disp_denoise_update()
{
    disp_priv_t *p_dp = &s_stDisplayPriv;

    if(p_dp->denoise_info.denoise_enable == MT_FALSE)
        return;

    p_dp->denoise_info.cur_resolution = reg_aria_hd_encoder_get_basic_cfg_resolusion();
    //0:1920*1080,1:1280*720,2:720*576,3:720*480
    if((0 != p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h) &&
            (0 != p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].w) &&
            ((p_dp->disp_in_info.old_rect[DISP_CHANNEL_HD].h !=
              p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h) ||
             (p_dp->denoise_info.cur_resolution != p_dp->denoise_info.old_resolution)
            ) )
    {
        //0:1920*1080,1:1280*720,2:720*576,3:720*480
        //sd input
        if(p_dp->disp_in_info.cur_rect[DISP_CHANNEL_HD].h < DISP_VID_FULLSCR_720P_HEIGHT)
        {
            //hd output
            if(p_dp->denoise_info.cur_resolution == 0 || p_dp->denoise_info.cur_resolution == 1)
            {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
                //to do ...
#else
                reg_symphony_disp_drv_set_nlmeans_denoise_ctrl(0x1e320100);
#endif
                DISP_PRINT("denoise update: sd input, hd output\n");
            }
            else//sd output
            {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
                //to do ...
#else
                reg_symphony_disp_drv_set_nlmeans_denoise_ctrl(0x28320100);
#endif
                DISP_PRINT("denoise update: sd input, sd output\n");
            }
        }
        else //hd input
        {
            //hd output
            if(p_dp->denoise_info.cur_resolution == 0 || p_dp->denoise_info.cur_resolution == 1)
            {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
                //todo
#else
                reg_symphony_disp_drv_set_nlmeans_denoise_ctrl(0xff320100);
#endif
                DISP_PRINT("denoise update: hd input, hd output\n");
            }
            else
            {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
                //to do ...
#else
                reg_symphony_disp_drv_set_nlmeans_denoise_ctrl(0xff320100);
#endif
                DISP_PRINT("denoise update: hd input, sd output\n");
            }
        }
        p_dp->denoise_info.old_resolution = p_dp->denoise_info.cur_resolution;
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY6) //sym6
    //to do ...
#else
    reg_symphony_disp_drv_set_nlmeans_parameter_1(0x00130008);
    reg_symphony_disp_drv_set_nlmeans_parameter_2(0x000207de);
    reg_symphony_disp_drv_set_nlmeans_parameter_3(0x05a0026a);
    reg_symphony_disp_drv_set_denoise_range_6_vid_denoise_en(0x1);
#endif
}
#endif

mt_s32 disp_set_hd_video_onoff(MT_BOOL  bEnable)
{
    mt_s32 nRet = MT_SUCCESS;

    HdOutEnable = bEnable;

    return nRet;
}

mt_s32 disp_set_sd_video_onoff(MT_BOOL  bEnable)
{
    mt_s32 nRet = MT_SUCCESS;

    SdOutEnable = bEnable;

    return nRet;
}

mt_s32 disp_tvsys_force_update(void)
{
    disp_priv_t *p_dp = &s_stDisplayPriv;

    DISP_DEBUGK("%s %d: \n", __FUNCTION__, __LINE__);

    drv_disp_get_hdmi_edid();

    p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
    p_dp->csc_info.old_resolution = 0xff;
    reset_delay_cnt = 0;
    update_sys_flag = 1;

    return MT_SUCCESS;
}

mt_s32 disp_set_vac_onoff(dac_index_t dac_id, MT_BOOL bEnable)
{
    disp_hal_set_dac_onoff(dac_id, bEnable);
    return MT_SUCCESS;
}

static mt_u32 add_crc6(mt_u32 data)
{
    mt_u8 i;
    mt_u8 d0,d1,d2,d3,d4,d5;
    mt_u8 d5_new, d4_new;
    mt_u8 d_in;
    mt_u32 datin = data;

    d0 = 1;
    d1 = 1;
    d2 = 1;
    d3 = 1;
    d4 = 1;
    d5 = 1;

    for(i = 1; i <= 14; i++)
    {
        d_in = datin & 1;
        datin =datin >> 1;
        d5_new= d0 ^ d_in;
        d4_new = d5 ^ d5_new;
        d0 = d1;
        d1 = d2;
        d2 = d3;
        d3 = d4;
        d4 = d4_new;
        d5 = d5_new;
    }

    return (data & 0x3fff) | (d0 << 14) | (d1 << 15) | (d2 << 16) | (d3 << 17) | (d4 << 18) | (d5 << 19);
}

mt_s32 disp_set_tv_capability(MT_DRV_DISP_HDMI_MODE_E enTvCap)
{
    RET_CODE ret = SUCCESS;
#if defined(SUPPORT_HDR) && defined(CONFIG_HDMI)

    disp_priv_t *p_dp = &s_stDisplayPriv;
    PicHdrInfo_t *g_p_hdr_info = &(p_dp->hdr_info);
#if 0
    u8 i = 0;
    hdmi_usrdf_infoframe_t hdmi_infoframe = {0};

    if (NULL == p_dp)
    {
        return ERR_PARAM;
    }
#endif
    drv_disp_get_hdmi_edid();
    DISP_DEBUGK("[set_tv_capability]metadata: %d, x=[%d, %d, %d], y=[%d, %d, %d], white_point=[%d, %d], lum=[%d, %d], %d, max_light=%d, ave_light=%d\n",
            g_p_hdr_info->colour_volume.colour_volume_enable,
            g_p_hdr_info->colour_volume.display_primaries_x[0],g_p_hdr_info->colour_volume.display_primaries_x[1], g_p_hdr_info->colour_volume.display_primaries_x[2],
            g_p_hdr_info->colour_volume.display_primaries_y[0], g_p_hdr_info->colour_volume.display_primaries_y[1], g_p_hdr_info->colour_volume.display_primaries_y[2],
            g_p_hdr_info->colour_volume.white_point_x, g_p_hdr_info->colour_volume.white_point_y, g_p_hdr_info->colour_volume.max_luminance, g_p_hdr_info->colour_volume.min_luminance,
            g_p_hdr_info->light_level.light_level_info_enable, g_p_hdr_info->light_level.max_light_level, g_p_hdr_info->light_level.max_pic_ave_light_level);
    DISP_DEBUGK("TvCap:%d hdr10:%d hlg:%d 30bit:%d 36bit:%d bt2020:%d\n", enTvCap, p_dp->hdmi_info.edid.supported_hdr10, p_dp->hdmi_info.edid.supported_hlg,
            p_dp->hdmi_info.edid.rgb30bit, p_dp->hdmi_info.edid.rgb36bit, p_dp->hdmi_info.supported_bt2020);
#if 0
    switch(enTvCap)
    {
    case MT_DRV_DISP_HDMI_MODE_SDR:
        if ((p_dp->hdmi_info.edid.supported_hdr10 || p_dp->hdmi_info.edid.supported_hlg)
                && p_dp->hdmi_info.used_tv_mode != TV_MODE_SDR)
        {
            DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
            msleep(50);
            p_dp->hdmi_info.used_tv_mode = TV_MODE_SDR;
            DRV_HDMI_UserPacketRptEnable(MT_UNF_HDMI_ID_0, 0, MT_FALSE);
            memset(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[0], 0, sizeof(hdmi_usrdf_infoframe_t));

            disp_update_cvbs_for_sd_tvsys(p_dp, MT_FALSE);
            p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
            //OS_PRINTF("%s: %d \n", __FUNCTION__, __LINE__);
            reset_delay_cnt = 0;
            update_sys_flag = 1;
        }

        break;
    case MT_DRV_DISP_HDMI_MODE_HDR10:
        if (p_dp->hdmi_info.edid.supported_hdr10)
        {
            /*if(old_used_tv_mode != TV_MODE_HDR10)*/
            {
                DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                msleep(50);

                //OS_PRINTF("%s: %d \n", __FUNCTION__, __LINE__);
                hdmi_infoframe.type = 0x87;
                hdmi_infoframe.version = 1;
                hdmi_infoframe.length = 26;
                hdmi_infoframe.pb_byte[0] = 2;  //EOTF
                hdmi_infoframe.pb_byte[1] = 0;  //metadata_description_ID
                if(g_p_hdr_info->colour_volume.colour_volume_enable)
                {
                    hdmi_infoframe.pb_byte[2] = g_p_hdr_info->colour_volume.display_primaries_x[0] & 0xff;
                    hdmi_infoframe.pb_byte[3] = (g_p_hdr_info->colour_volume.display_primaries_x[0] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[4] = g_p_hdr_info->colour_volume.display_primaries_y[0] & 0xff;
                    hdmi_infoframe.pb_byte[5] = (g_p_hdr_info->colour_volume.display_primaries_y[0] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[6] = g_p_hdr_info->colour_volume.display_primaries_x[1] & 0xff;
                    hdmi_infoframe.pb_byte[7] = (g_p_hdr_info->colour_volume.display_primaries_x[1] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[8] = g_p_hdr_info->colour_volume.display_primaries_y[1] & 0xff;
                    hdmi_infoframe.pb_byte[9] = (g_p_hdr_info->colour_volume.display_primaries_y[1] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[10] = g_p_hdr_info->colour_volume.display_primaries_x[2] & 0xff;
                    hdmi_infoframe.pb_byte[11] = (g_p_hdr_info->colour_volume.display_primaries_x[2] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[12] = g_p_hdr_info->colour_volume.display_primaries_y[2] & 0xff;
                    hdmi_infoframe.pb_byte[13] = (g_p_hdr_info->colour_volume.display_primaries_y[2] & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[14] = g_p_hdr_info->colour_volume.white_point_x & 0xff;
                    hdmi_infoframe.pb_byte[15] = (g_p_hdr_info->colour_volume.white_point_x & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[16] = g_p_hdr_info->colour_volume.white_point_y & 0xff;
                    hdmi_infoframe.pb_byte[17] = (g_p_hdr_info->colour_volume.white_point_y & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[18] = (g_p_hdr_info->colour_volume.max_luminance /10000) & 0xff;
                    hdmi_infoframe.pb_byte[19] = ((g_p_hdr_info->colour_volume.max_luminance /10000) & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[20] = g_p_hdr_info->colour_volume.min_luminance & 0xff;
                    hdmi_infoframe.pb_byte[21] = (g_p_hdr_info->colour_volume.min_luminance & 0xff00) >> 8;

                }
                if(g_p_hdr_info->light_level.light_level_info_enable)
                {
                    hdmi_infoframe.pb_byte[22] = g_p_hdr_info->light_level.max_light_level & 0xff;
                    hdmi_infoframe.pb_byte[23] = (g_p_hdr_info->light_level.max_light_level & 0xff00) >> 8;
                    hdmi_infoframe.pb_byte[24] = g_p_hdr_info->light_level.max_pic_ave_light_level & 0xff;
                    hdmi_infoframe.pb_byte[25] = (g_p_hdr_info->light_level.max_pic_ave_light_level & 0xff00) >> 8;
                }

                memcpy(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[i],  &hdmi_infoframe,  sizeof(hdmi_usrdf_infoframe_t));

                p_dp->hdmi_info.used_tv_mode = TV_MODE_HDR10;
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                reset_delay_cnt = 0;
                update_sys_flag = 1;
            }
        }
        else
        {
            if (p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff)
            {
                DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                msleep(50);
                p_dp->hdmi_info.used_tv_mode = TV_MODE_SDR;
                memset(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[i], 0, sizeof(hdmi_usrdf_infoframe_t));
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                reset_delay_cnt = 0;
                update_sys_flag = 1;
            }
            else
            {
#if 0
                if (is_first == 0)
                {
                    DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                    msleep(500);
                    DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_FALSE);
                }
#endif
            }
        }

        break;

    case MT_DRV_DISP_HDMI_MODE_HLG:
        if (p_dp->hdmi_info.edid.supported_hlg)
        {
            if(old_used_tv_mode != TV_MODE_HLG)
            {
                DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                msleep(50);

                p_dp->hdmi_info.used_tv_mode = TV_MODE_HLG;
                hdmi_infoframe.type = 0x87;
                hdmi_infoframe.version = 1;
                hdmi_infoframe.length = 2;
                hdmi_infoframe.pb_byte[0] = 3;  //EOTF
                hdmi_infoframe.pb_byte[1] = 0;  //static_metadata_descriptor_ID
                memcpy(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[i],  &hdmi_infoframe,  sizeof(hdmi_usrdf_infoframe_t));
                //p_dp->hdmi_vcfg_flag = TRUE;
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                reset_delay_cnt = 0;
                update_sys_flag = 1;
            }
        }
        else
        {
            if (p_dp->hdmi_vcfg.output_v_cfg.hdr_onoff)
            {
                DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                msleep(50);

                p_dp->hdmi_info.used_tv_mode = TV_MODE_SDR;
                memset(&p_dp->hdmi_vcfg.output_v_cfg.usrdf_info[i], 0, sizeof(hdmi_usrdf_infoframe_t));
                p_dp->disp_out_info[DISP_CHANNEL_HD].update_flag |= UPDATE_FLAG_TVSYS;
                reset_delay_cnt = 0;
                update_sys_flag = 1;
            }
            else
            {
#if 0
                if (is_first == 0)
                {
                    DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_TRUE);
                    msleep(500);
                    DRV_HDMI_SetAVMute(MT_UNF_HDMI_ID_0, MT_FALSE);
                }
#endif
            }
        }
        break;

    case MT_DRV_DISP_HDMI_MODE_AUTO:
        ;//do it in CSC update
    default:
        break;
    }
#endif

    /*if(update_sys_flag)*/
    {
        g_tv_capability = enTvCap;
        //old_used_tv_mode = p_dp->hdmi_info.used_tv_mode;
        /*is_first = 0;*/
    }
    // fixed bug 114805, the hdmi config flow need some time, so sleep to watint for the config finish.
    msleep(500);
#endif
    return ret;
}

mt_s32 disp_set_sl_hdr(MT_DRV_DISPLAY_E enDisp, mt_u32 transparent_mode, mt_u32 display_Brightness, mt_u32 tuning_level, mt_u32 display_OETF)
{
    DISP_S *pstDisp;

    disp_priv_t *p_dp;

    //  mt_u32 dtmp = 0;
    //  disp_sys_t tv_sys_sd;
    DispGetPointerByIDNoReturn(enDisp, pstDisp);

    p_dp = (disp_priv_t *)pstDisp->p_dp;
    p_dp->hdr_para.transparent_mode = transparent_mode;
    p_dp->hdr_para.sdr_display_Brightness = (display_Brightness >> 16) & 0xffff;
    p_dp->hdr_para.hdr_display_Brightness = display_Brightness & 0xffff;
    p_dp->hdr_para.target_display_adaptation_tuning_level = tuning_level;
    //p_dp->hdr_para.display_OETF = display_OETF;   //Not yet used
    DISP_DEBUGK("%s transparent_mode:%d sdr_display_Brightness:%d hdr_display_Brightness:%d tuning_level:%d\n",
    __FUNCTION__, 
    transparent_mode,
    p_dp->hdr_para.sdr_display_Brightness, 
    p_dp->hdr_para.hdr_display_Brightness,
    tuning_level);
    
    return MT_SUCCESS;
}


//FIXME: set TDE video capture size
mt_void disp_update_tde_video_size(mt_void)
{
    //store video size in tde reg for video capture
    disp_hal_put_u32((volatile u32*)(mt_get_tde_base() + 0x110),
            (u32)((g_vdec_width << 16) | g_vdec_height));
}

mt_s32 disp_dump_scaler_to_osd(mt_handle pstDispBP, MT_DRV_DISP_DUMP_SCALER_PARA_S *pstParam)
{
    mt_u32 stride = 1920;
    mt_u32 in_width = 1920;
    mt_u32 in_height = 1080;
    mt_u32 out_width = 1280;
    mt_u32 out_height = 720;
    mt_u8  in_source = 0;
    MT_DRV_DISP_DUMP_SCALER_SOURCE_E source = MT_DRV_DISP_DSCALER_IN_HD_SCREEN;
    OSD_EXPORT_FUNC_S *pstOSDFunc = MT_NULL;
    osd_header_info_s osd_header_info = {0};

    if(!pstParam)
    {
        DISP_DEBUGK("null param!!\n");
        return MT_ERR_PARAM;
    }

    if(pstParam->b_enable == MT_FALSE)
    {
        drv_reg_4k_disp_set_ds_scale0_ctrl_dscaler_disable(1);
        drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_en_0(0);
        drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_en_2(0);

        return MT_SUCCESS;
    }

    out_width = pstParam->out_width;
    out_height = pstParam->out_height;
    source = pstParam->source;

    mt_drv_module_getfunction(MT_ID_OSD, (mt_void **)&pstOSDFunc);

    if (pstOSDFunc && pstOSDFunc->pfnOsdGetHeaderInfo)
    {
        pstOSDFunc->pfnOsdGetHeaderInfo(pstParam->dst_layer, &osd_header_info);
        DISP_DEBUGK("m_left:%d m_top:%d m_right:%d m_bottom:%d  \n",
                osd_header_info.m_left,
                osd_header_info.m_top,
                osd_header_info.m_right,
                osd_header_info.m_bottom);

        DISP_DEBUGK("m_colormode:%d m_enable:%d m_pitch:%d m_start_addr:0x%08x \n",
                osd_header_info.m_colormode,
                osd_header_info.m_enable,
                osd_header_info.m_pitch,
                osd_header_info.m_start_addr);
    }
    else
    {
        DISP_DEBUGK("can not get osd info!! \n");
        return MT_ERR_PARAM;
    }


    switch (source)
    {
    case MT_DRV_DISP_DSCALER_IN_HD_SCREEN:
        disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &in_height, &in_width);
        in_source = 0;
        break;

    case MT_DRV_DISP_DSCALER_IN_HD_VIDEO:
        disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &in_height, &in_width);
        in_source = 1;
        break;

    case MT_DRV_DISP_DSCALER_IN_SD_SCREEN:
        disp_st_vid_get_vout_size(DISP_CHANNEL_SD, &in_height, &in_width);
        in_source = 2;
        break;
    default:
        DISP_DEBUGK("dscaler don't support this input source!!\n");
        return MT_ERR_PARAM;

    }

    stride = (pstParam->out_width + 15) >> 4 << 4;

    DISP_DEBUGK("in_width:%d in_height:%d out_width:%d out_height:%d, stride:%d \n",
            in_width, in_height, out_width, out_height, stride);

    DF_DumpScaler_Update(pstDispBP, in_width, in_height, out_width, out_height);

    drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_progressive_flag_0(1);
    drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_progressive_flag_2(1);
    drv_reg_4k_disp_set_ds_scaler_wr_addr_luma_0(osd_header_info.m_start_addr);
    drv_reg_4k_disp_set_ds_scaler_wr_addr_chro_0(osd_header_info.m_start_addr + stride * out_height);
    drv_reg_4k_disp_set_ds_scaler_wr_addr_luma_2(osd_header_info.m_start_addr);
    drv_reg_4k_disp_set_ds_scaler_wr_addr_chro_2(osd_header_info.m_start_addr + stride * out_height);
    drv_reg_4k_disp_set_ds_scale0_ctrl_dscaler_in_source(in_source);

    drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_nv21(1);
    drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_endian(0xf);
    drv_reg_4k_disp_set_ds_scaler_wr_cfg_1_cfg_wr_stride(stride);
    drv_reg_4k_disp_set_ds_scale0_ctrl_dscaler_disable(0);
    drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_en_0(1);
    drv_reg_4k_disp_set_ds_scaler_wr_cfg_0_cfg_wr_en_2(1);

    return MT_SUCCESS;
}

mt_s32 disp_set_1001_enable(MT_BOOL bEnable)
{
    RET_CODE Ret = MT_SUCCESS;
    disp_priv_t *p_dp = &s_stDisplayPriv;

    DISP_DEBUGK("%s bEnable:%d\n", __FUNCTION__, bEnable);

    p_dp->clock_1001_enable = bEnable;
    
    return Ret;  
}

mt_s32 DRV_DISP_Is_SLHDR_Enable(void)
{
    return g_slhdr_enable;
}

//mt_s32 DRV_DISP_Set_SLHDR_Enable(mt_s32 enable)
//{
//    g_slhdr_enable = enable;
//    return MT_SUCCESS;
//}
EXPORT_SYMBOL(DRV_DISP_Is_SLHDR_Enable);
//EXPORT_SYMBOL(DRV_DISP_Set_SLHDR_Enable);
EXPORT_SYMBOL(DRV_DISP_O5_EnableDacPower);
EXPORT_SYMBOL(DRV_DISP_O5_GetDacPower);
EXPORT_SYMBOL(DRV_DISP_O5_GetSignal);
EXPORT_SYMBOL(DRV_DISP_O5_SetSignal);
EXPORT_SYMBOL(DRV_DISP_O5_SetMacv);
EXPORT_SYMBOL(DRV_DISP_O5_GetMacv);
EXPORT_SYMBOL(DRV_DISP_O5_SetMacvCps);
EXPORT_SYMBOL(DRV_DISP_O5_GetCGMS);
EXPORT_SYMBOL(DRV_DISP_O5_GetWSS);
EXPORT_SYMBOL(DRV_DISP_O5_SetCGMS);
EXPORT_SYMBOL(DRV_DISP_O5_SetWss);
//EXPORT_SYMBOL(disp_get_dce_percent);

