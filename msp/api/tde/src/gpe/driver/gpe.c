#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/types.h>
#include <sys/mman.h>
#include <inttypes.h>
#include "mt_drv_tde.h"
#include "mt_tde_api.h"
#include "mt_debug.h"
#include "mt_tde_type.h"
#include "mt_common.h"
#include "gpe.h"
#include "gpe_aria_reg.h"
#include "mt_module_debug.h"
#include "mpi_memdev.h"

#include "tde_proc.h"
#include "drv_disp_ioctl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif  /* __cplusplus */
#endif  /* __cplusplus */

#undef MOD_NAME
#define MOD_NAME		"gpe"

#define GPE_DEBUG

#ifdef GPE_DEBUG
#define GPE_PRINT(fmt,...) printf("%s:%d  "fmt" ",__FUNCTION__,__LINE__,##__VA_ARGS__)
#else
#define GPE_PRINT(...)   do{}while(0)
#endif

#ifdef GPE_DEBUG
#define GPE_REG_PRINT(fmt,...) printf(fmt,##__VA_ARGS__)
#else
#define GPE_REG_PRINT(...)
#endif

//gavin.s for fpga
//#ifndef CONFIG_MT_FPGA_GPE
#define GPE_ARIA_HARDWARE_ISR_ON
//#endif
#define msleep(n) usleep(n*1000)

#define GPE_MAX_HANDLE 4
#define BAK_BUFFER_NUM_MAX 10

extern mt_s32 get_tde_handle(mt_void);

#ifdef CONFIG_MT_FPGA_GPE
extern void gpe_core(aria_gpe_context_t *p_ctx, MT_BOOL sw_comp);
extern MT_BOOL gpe_sw_compare(aria_gpe_context_t *p_ctx);
#endif

static hdl_gpe_t g_hdl_gpe[GPE_MAX_HANDLE];
static mt_mmz_buf_s g_psMBuf = {0};
static ulong g_addr_offset = 0;
static phys_addr_t u32CoeffPhyAddr = 0;
// yuv scale coeff table
static __attribute__ ((aligned (128))) mt_u32 gpe_scale_coeff[] = {
#include "gpe_aria_scale_table.h"
};
#define CONFIG_TDE_PROC_DISABLE

extern ulong gpe_reg_virt_addr;
extern ulong sys_reg_virt_addr;
extern MT_CHIP_VERSION_E   ChipVersion;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
#define FAST_2D_SCALE
#endif

#ifndef CONFIG_TDE_PROC_DISABLE
static MT_TDE_PROC_INFO_S tde_proc = {0};
extern mt_s32 g_s32TdeFd;
#endif

static const float EPSINON = 0.00001;
#define SUPPORT_CMDFIFO  

#ifdef SUPPORT_CMDFIFO
//static MT_BOOL g_is_sync_list = MT_TRUE;
static MT_BOOL g_create_cmdfifo_flag = MT_FALSE;
static cmdfifo_hdl_t *g_cmdfifo_hdl = NULL;
static mt_mmz_buf_s g_psCFBuf = {0};
static mt_u32 g_cfId = 0;
static phys_addr_t p_gradt_buf_bak[BAK_BUFFER_NUM_MAX] = {0};
#endif

#ifdef CONFIG_MT_FPGA_GPE
spn_debug_t g_debug = 
{
  	0,  //print_reg0
  	0, //print_reg1
  	0, //compare_with_sw
  	0, //show_sw_result
  	0, //key_set
  	0, //key_msk
  	0, //zero_edge
  	0, //pause_on
  	0, //reset_on
  	0, //clip_en
  	0, //rotator_op
  	0, //no_rst
  	0, //cmyk_max
  	1, //src1_bitswap
  	1, //src3_bitswap
  	0, //use_new_expmode
  	0, //dst_premult_en
  	0, //mirror
  	0, //tmp_mem
  	0, //ds_mod
  	0, //mask_test_en
  	0,  //new_blend_en
  	0, //wr_last_mod
  	0, //old_pfm_mod
  	0, //demultiply_en
    0, //chroma_vir_addr
    {0}, //comset
    .int_mode_en = 1,
};

void gpe_symphony_set_debug(void *p_info)
{
    spn_debug_t *p_debug = (spn_debug_t *)p_info;
  
#if 0   
    u32 i = 0;
    g_debug.compare_with_sw = p_debug->compare_with_sw;
    g_debug.show_sw_result = p_debug->show_sw_result;
    g_debug.print_reg0 = p_debug->print_reg0;
    g_debug.print_reg1 = p_debug->print_reg1;
    g_debug.key_msk = p_debug->key_msk;
    g_debug.key_set = p_debug->key_set;
    g_debug.zero_edge = p_debug->zero_edge;
    g_debug.pause_on = p_debug->pause_on;
    g_debug.reset_on = p_debug->reset_on;
    g_debug.clip_en = p_debug->clip_en;
    g_debug.rotator_op = p_debug->rotator_op;
    g_debug.no_rst = p_debug->no_rst;
    g_debug.cmyk_max = p_debug->cmyk_max;
    g_debug.src1_bitswap = p_debug->src1_bitswap;
    g_debug.src3_bitswap = p_debug->src3_bitswap;
    g_debug.use_new_expmode = p_debug->use_new_expmode;
    g_debug.dst_premult_en = p_debug->dst_premult_en;
    g_debug.mirror = p_debug->mirror;
    g_debug.tmp_mem = p_debug->tmp_mem;
    g_debug.ds_mod = p_debug->ds_mod;
    g_debug.mask_test_en = p_debug->mask_test_en;
    for(i = 0; i < 10; i++)
        g_debug.comset[i] = p_debug->comset[i];
    g_debug.new_blend_en = p_debug->new_blend_en;
    g_debug.wr_last_mod = p_debug->wr_last_mod;
    g_debug.old_pfm_mod = p_debug->old_pfm_mod;
    g_debug.demultiply_en = p_debug->demultiply_en;
    g_debug.chroma_vir_addr = p_debug->chroma_vir_addr;
#else
    if(p_debug)
    {
        memcpy(&g_debug,p_debug,sizeof(spn_debug_t));
    }
#endif
}

void gpe_symphony_get_debug(void *p_info)
{
    spn_debug_t *p_debug = (spn_debug_t *)p_info;
#if 0  
    u32 i = 0;
    p_debug->compare_with_sw = g_debug.compare_with_sw;
    p_debug->show_sw_result = g_debug.show_sw_result;
    p_debug->print_reg0 = g_debug.print_reg0;
    p_debug->print_reg1 = g_debug.print_reg1;
    p_debug->key_msk = g_debug.key_msk;
    p_debug->key_set = g_debug.key_set;
    p_debug->zero_edge = g_debug.zero_edge;
    p_debug->pause_on = g_debug.pause_on;
    p_debug->reset_on = g_debug.reset_on;
    p_debug->clip_en = g_debug.clip_en;
    p_debug->rotator_op = g_debug.rotator_op;
    p_debug->no_rst = g_debug.no_rst;
    p_debug->cmyk_max = g_debug.cmyk_max;
    p_debug->src1_bitswap = g_debug.src1_bitswap;
    p_debug->src3_bitswap = g_debug.src3_bitswap;
    p_debug->use_new_expmode = g_debug.use_new_expmode;
    p_debug->dst_premult_en = g_debug.dst_premult_en;
    p_debug->mirror = g_debug.mirror;
    p_debug->tmp_mem = g_debug.tmp_mem;
    p_debug->ds_mod = g_debug.ds_mod;
    p_debug->mask_test_en = g_debug.mask_test_en;
    for(i = 0; i < 10; i++)
        p_debug->comset[i] = g_debug.comset[i] ;    
    p_debug->new_blend_en = g_debug.new_blend_en;    
    p_debug->wr_last_mod = g_debug.wr_last_mod;
    p_debug->old_pfm_mod = g_debug.old_pfm_mod;
    p_debug->demultiply_en = g_debug.demultiply_en;
#else
    if(p_debug)
    {
        memcpy(p_debug,&g_debug,sizeof(spn_debug_t));
    }    
#endif    
}

#endif

#ifdef SUPPORT_CMDFIFO
static mt_s32 p_gradt_buf_backup(phys_addr_t src_addr, phys_addr_t *backup, mt_u32 backup_max)
{
    mt_u32 i = 0;

    for(i = 0; i < backup_max; i++)
    {
        if(backup[i] == 0)
            break;        
    }   
    if(i < backup_max)
    {        
  //      printf("<%s> : <%d> : <%lx>\n", __FUNCTION__, i , src_addr);
        backup[i] = src_addr;
        return MT_SUCCESS;
     }
    return MT_FAILURE;
}

static mt_s32 p_gradt_buf_free(phys_addr_t *backup, mt_u32 backup_max)
{
    mt_u32 i = 0;

    for(i = 0; i < backup_max; i++)
    {
        if(backup[i] != 0)
        {
      //      printf("<%s> : <%d> : <%lx>\n", __FUNCTION__, i , backup[i]);
            mt_mmz_delete(backup[i]);
            backup[i] = 0;
        }
    } 
    return MT_SUCCESS;
}
#endif

mt_s32 gpe_create_mid_memory(mt_u32 mem_size)
{
    if(g_psMBuf.phyaddr == 0 && g_psMBuf.user_viraddr == 0)
    {
        memset(&g_psMBuf, 0x00, sizeof(mt_mmz_buf_s));
        strncpy(g_psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
        g_psMBuf.bufsize = mem_size;
        mt_mmz_malloc(&g_psMBuf);
        if(g_psMBuf.phyaddr)
        {
            g_addr_offset = 0;
            return 0;
        }
        else
        {
            GPE_PRINT("\r\n gpe_create_mid_memory failed");
            g_psMBuf.bufsize = 0;
            return 1;
        }
    }
    return 1;
}

mt_s32 gpe_destroy_mid_memory(mt_void)
{
    if(g_psMBuf.phyaddr && g_psMBuf.user_viraddr)
    {
        mt_mmz_free(&g_psMBuf);
        g_psMBuf.phyaddr = 0;
        g_psMBuf.user_viraddr = 0;
        g_addr_offset = 0;
        return 0;
    }
    return 1;
}
#ifdef SUPPORT_CMDFIFO
mt_s32 gpe_create_cmdfifo_memory(mt_void)
{
    if(g_psCFBuf.phyaddr == 0 && g_psCFBuf.user_viraddr == 0)
    {
        memset(&g_psCFBuf, 0x00, sizeof(mt_mmz_buf_s));
        strncpy(g_psCFBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
        g_psCFBuf.bufsize = GPE_ARIA_MAX_CMDFIFO_SIZE * MAX_NODE_NUM * 8;
        mt_mmz_malloc(&g_psCFBuf);
        if(g_psCFBuf.phyaddr)
        {
            return MT_SUCCESS;
        }
        else
        {
            GPE_PRINT("\r\n gpe_create_mid_memory failed");
            g_psCFBuf.bufsize = 0;
            return MT_FAILURE;
        }
    }
    return 1;
}

mt_s32 gpe_destroy_cmdfifo_memory(mt_void)
{
    if(g_psCFBuf.phyaddr && g_psCFBuf.user_viraddr)
    {
        mt_mmz_free(&g_psCFBuf);
        g_psCFBuf.phyaddr = 0;
        g_psCFBuf.user_viraddr = 0;
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}

mt_s32 get_cmdfifo_buf(ulong *p_viraddr, phys_addr_t *p_phyaddr)
{
    if(g_cfId >= MAX_NODE_NUM
            || g_psCFBuf.phyaddr == 0 || g_psCFBuf.user_viraddr == 0)
    {
        *p_viraddr = (ulong)NULL;
        *p_phyaddr = (phys_addr_t)0;
        return 1;
    }
    else
    {
        *p_viraddr =(ulong)(g_psCFBuf.user_viraddr + g_cfId * GPE_ARIA_MAX_CMDFIFO_SIZE * 8);
        *p_phyaddr = g_psCFBuf.phyaddr + g_cfId * GPE_ARIA_MAX_CMDFIFO_SIZE * 8;
        g_cfId++;
        return 0;
    }
}
#endif

static void gpe_write_register(unsigned int addr, unsigned int data)
{
#ifdef USE_USER_SPACE_GPE

    ulong *reg_addr;

    //GPE_PRINT("write_reg[0x%08x]=0x%08x\n", (0xffd60000+addr), data);
    reg_addr = (ulong *)(addr + (ulong)gpe_reg_virt_addr);
	mpi_write_reg32(reg_addr, data);
#endif
}

static unsigned int gpe_read_register(unsigned int addr)
{
#ifdef USE_USER_SPACE_GPE

    ulong *reg_addr;

    reg_addr = (ulong *)(addr + (ulong)gpe_reg_virt_addr);
    return mpi_read_reg32(reg_addr);
#else
    return 0;
#endif
}


#ifndef CONFIG_TDE_PROC_DISABLE
static MT_S32 gpe_set_proc_info(aria_param_t *p_param, mt_u32 state)
{
    mt_u32 data;
    mt_u32 i;

    if(state == 0)
    {
        memset(&tde_proc, 0, sizeof(MT_TDE_PROC_INFO_S));
        tde_proc.tde_param.src_img_en = p_param->src_img_en;
        tde_proc.tde_param.bg_img_en = p_param->bg_img_en;
        tde_proc.tde_param.ex_img_en = p_param->ex_img_en;
        tde_proc.tde_param.no_dithering = p_param->no_dithering;
        tde_proc.tde_param.clip_en = p_param->clip_en;
        tde_proc.tde_param.gpe_op = p_param->gpe_op;
        tde_proc.tde_param.alpha_map_mod = p_param->alpha_map_mod;
        tde_proc.tde_param.rotator_op = p_param->rotator_op;
        tde_proc.tde_param.paint_type = p_param->paint.paint_type;

        tde_proc.tde_param.src_img.buf = p_param->src_img.buf;
        tde_proc.tde_param.src_img.pitch = p_param->src_img.pitch;
        tde_proc.tde_param.src_img.width = p_param->src_img.width;
        tde_proc.tde_param.src_img.height = p_param->src_img.height;
        memcpy(&(tde_proc.tde_param.src_img.rect), &p_param->src_img.rect, sizeof(rect_vsb_t));
        tde_proc.tde_param.src_img.pix_format = p_param->src_img.pix_format;
        tde_proc.tde_param.src_img.ck_en = p_param->src_img.ck_en;
        tde_proc.tde_param.src_img.ck_min = p_param->src_img.ck_min;
        tde_proc.tde_param.src_img.ck_max = p_param->src_img.ck_max;
        tde_proc.tde_param.src_img.key_color_mod = p_param->src_img.key_color_mod;
        tde_proc.tde_param.src_img.key_color_select = p_param->src_img.key_color_select;

        tde_proc.tde_param.dst_img.buf = p_param->dst_img.buf;
        tde_proc.tde_param.dst_img.pitch = p_param->dst_img.pitch;
        tde_proc.tde_param.dst_img.width = p_param->dst_img.width;
        tde_proc.tde_param.dst_img.height = p_param->dst_img.height;
        memcpy(&(tde_proc.tde_param.dst_img.rect), &p_param->dst_img.rect, sizeof(rect_vsb_t));

        tde_proc.tde_param.dst_img.pix_format = p_param->dst_img.pix_format;
        tde_proc.tde_param.dst_img.ck_en = p_param->dst_img.ck_en;
        tde_proc.tde_param.dst_img.ck_min = p_param->dst_img.ck_min;
        tde_proc.tde_param.dst_img.ck_max = p_param->dst_img.ck_max;
        tde_proc.tde_param.dst_img.key_color_mod = p_param->dst_img.key_color_mod;
        tde_proc.tde_param.dst_img.key_color_select = p_param->dst_img.key_color_select;

        tde_proc.tde_param.ex_img.buf = p_param->ex_img.buf;
        tde_proc.tde_param.ex_img.pitch = p_param->ex_img.pitch;
        tde_proc.tde_param.ex_img.width = p_param->ex_img.width;
        tde_proc.tde_param.ex_img.height = p_param->ex_img.height;
        memcpy(&(tde_proc.tde_param.ex_img.rect), &p_param->ex_img.rect, sizeof(rect_vsb_t));
        tde_proc.tde_param.ex_img.pix_format = p_param->ex_img.pix_format;
        tde_proc.tde_param.ex_img.ck_en = p_param->ex_img.ck_en;
        tde_proc.tde_param.ex_img.ck_min = p_param->ex_img.ck_min;
        tde_proc.tde_param.ex_img.ck_max = p_param->ex_img.ck_max;
        tde_proc.tde_param.ex_img.key_color_mod = p_param->ex_img.key_color_mod;
        tde_proc.tde_param.ex_img.key_color_select = p_param->ex_img.key_color_select;

        tde_proc.tde_param.bg_img.buf = p_param->bg_img.buf;
        tde_proc.tde_param.bg_img.pitch = p_param->bg_img.pitch;
        tde_proc.tde_param.bg_img.width = p_param->bg_img.width;
        tde_proc.tde_param.bg_img.height = p_param->bg_img.height;
        memcpy(&(tde_proc.tde_param.bg_img.rect), &p_param->bg_img.rect, sizeof(rect_vsb_t));
        tde_proc.tde_param.bg_img.pix_format = p_param->bg_img.pix_format;
        tde_proc.tde_param.bg_img.ck_en = p_param->bg_img.ck_en;
        tde_proc.tde_param.bg_img.ck_min = p_param->bg_img.ck_min;
        tde_proc.tde_param.bg_img.ck_max = p_param->bg_img.ck_max;
        tde_proc.tde_param.bg_img.key_color_mod = p_param->bg_img.key_color_mod;
        tde_proc.tde_param.bg_img.key_color_select = p_param->bg_img.key_color_select;

        memcpy(&(tde_proc.tde_param.blend), &p_param->blend, sizeof(blend_cfg_t));
        memcpy(&(tde_proc.tde_param.rop), &p_param->rop, sizeof(rop_cfg_t));
        memcpy(&(tde_proc.tde_param.blend_alpha), &p_param->blend_alpha, sizeof(blend_cfg_t));

    }


    if(state == 1)
    {
        for(i = 0; i < TDE_REG_MAX; i += 4)
        {
            data = gpe_read_register(GPE_ARIA_GRA_ENG_START + i);
            tde_proc.tde_reg[i/4].u32RegAddr = GPE_ARIA_GRA_ENG_START + i;
            tde_proc.tde_reg[i/4].u32RegVal = data;
        }
    }

    if(state == 2)
    {
        tde_proc.timeout = MT_TRUE;
        tde_proc.axi_state = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
        tde_proc.tde_state = gpe_read_register(GPE_ARIA_GRA_STATE);
    }

    if(state == 3)
    {
        tde_proc.timeout = MT_FALSE;
        tde_proc.axi_state = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
        tde_proc.tde_state = gpe_read_register(GPE_ARIA_GRA_STATE);
    }

    if(state == 2 || state == 3)
    {
        ioctl(g_s32TdeFd, TDE_SET_PROC, &tde_proc);
    }
    return MT_SUCCESS;
}
#endif
static color_space_t aria_get_color_space(mt_u32 pix_fmt)
{
    color_space_t color_space = RGB_COLOR_SPACE;
    switch(pix_fmt)
    {
    case PIX_FMT_XY:
    case PIX_FMT_XYL:
    case PIX_FMT_XYC:
    case PIX_FMT_XYLC:
    case PIX_FMT_XY_SMALL:
    case PIX_FMT_XYL_SMALL:
    case PIX_FMT_XYC_SMALL:
    case PIX_FMT_XYLC_SMALL:
    case PIX_FMT_RGBPALETTE1_PALETTE_BGRA:
    case PIX_FMT_RGBPALETTE1:
    case PIX_FMT_RGBPALETTE2:
    case PIX_FMT_RGBPALETTE4:
    case PIX_FMT_RGBPALETTE8:
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_ARGBPALETTE88:
    case PIX_FMT_RGB565:
    case PIX_FMT_ARGB1555:
    case PIX_FMT_RGBA5551:
    case PIX_FMT_ARGB4444:
    case PIX_FMT_RGBA4444:
    case PIX_FMT_ARGB8888:
    case PIX_FMT_RGBA8888:
    case PIX_FMT_RGBPALETTE2_PALETTE_BGRA:
    case PIX_FMT_RGBPALETTE4_PALETTE_BGRA:
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
    case PIX_FMT_RGBAPALETTE88:
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
    case PIX_FMT_RGB565_SMALL_ENDIAN:
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
    case PIX_FMT_ARGB8888_SMALL_ENDIAN:
    case PIX_FMT_RGBA8888_SMALL_ENDIAN:
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
    case PIX_FMT_RGBPALETTE2_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE2_PALETTE_ABGR:
    case PIX_FMT_RGBPALETTE4_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE4_PALETTE_ABGR:
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
    case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
    case PIX_FMT_RGB233:
    case PIX_FMT_RGB888:
    case PIX_FMT_BGR888:
    case PIX_FMT_CMYK:
    case PIX_FMT_KYMC:
    case PIX_FMT_SP_CMYK:
    case PIX_FMT_SP_CMYK_SWAP:
        color_space = RGB_COLOR_SPACE;
        break;
    case PIX_FMT_TILE:
    case PIX_FMT_YUVPALETTE1:
    case PIX_FMT_YUVPALETTE2:
    case PIX_FMT_YUVPALETTE4:
    case PIX_FMT_YUVPALETTE8:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_AYUVPALETTE88:
    case PIX_FMT_Y1CRY0CB8888:
    case PIX_FMT_CBY0CRY18888:
    case PIX_FMT_AYCBCR8888:
    case PIX_FMT_CRCBYA8888:
    case PIX_FMT_YCBCRA8888:
    case PIX_FMT_ACRCBY8888:
    case PIX_FMT_YUVPALETTE2_PALETTE_VUYA:
    case PIX_FMT_YUVPALETTE4_PALETTE_VUYA:
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
    case PIX_FMT_YUVAPALETTE88:
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
    case PIX_FMT_YUVPALETTE2_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE2_PALETTE_AVUY:
    case PIX_FMT_YUVPALETTE4_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE4_PALETTE_AVUY:
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
    case PIX_FMT_SP_YUV444:
    case PIX_FMT_SP_YUV422:
    case PIX_FMT_SP_YUV420:
    case PIX_FMT_SP_YUV444_UVSWAP:
    case PIX_FMT_SP_YUV422_UVSWAP:
    case PIX_FMT_SP_YUV420_UVSWAP:
    case PIX_FMT_SP_YUV422_1x2:
    case PIX_FMT_SP_YUV422_2x1:
    case PIX_FMT_SP_YUV422_1x2_UVSWAP:
    case PIX_FMT_SP_YUV422_2x1_UVSWAP:
        color_space = YUV_COLOR_SPACE;
        break;
    default:
        color_space = RGB_COLOR_SPACE;
        break;
    }
    return color_space;
}

#define KEY_MIN(color, bit_num) (color & (((1 << bit_num) - 1) << (8 - bit_num)))
#define KEY_MAX(color, bit_num) (color | ((1 << (8 - bit_num)) - 1))

mt_s32 getColorkeyMinMax(mt_u32 colorkey,TDE2_COLOR_FMT_E enColorFmt, TDE2_COLORKEY_U *pKeyValue)
{
    mt_u8 a = (mt_u8)((colorkey >> 24) & 0xff);
    mt_u8 r = (mt_u8)((colorkey >> 16) & 0xff);
    mt_u8 g = (mt_u8)((colorkey >> 8) & 0xff);
    mt_u8 b = (mt_u8)(colorkey & 0xff);
    mt_u32 AMin = 0, AMax = 0, RMin = 0, RMax = 0, GMin = 0, GMax = 0, BMin = 0, BMax = 0;

    switch(enColorFmt)
    {
    case TDE2_COLOR_FMT_RGB233:
        AMin = KEY_MIN(a, 8);
        RMin = KEY_MIN(r, 2);
        GMin = KEY_MIN(g, 3);
        BMin = KEY_MIN(b, 3);
        AMax = KEY_MAX(a, 8);
        RMax = KEY_MAX(r, 2);
        GMax = KEY_MAX(g, 3);
        BMax = KEY_MAX(b, 3);
        pKeyValue->struCkARGB.stAlpha.bCompIgnore = MT_TRUE;
        break;
    case TDE2_COLOR_FMT_RGB565:
        AMin = KEY_MIN(a, 8);
        RMin = KEY_MIN(r, 5);
        GMin = KEY_MIN(g, 6);
        BMin = KEY_MIN(b, 5);
        AMax = KEY_MAX(a, 8);
        RMax = KEY_MAX(r, 5);
        GMax = KEY_MAX(g, 6);
        BMax = KEY_MAX(b, 5);
        pKeyValue->struCkARGB.stAlpha.bCompIgnore = MT_TRUE;
        break;
    case TDE2_COLOR_FMT_RGBA1555:
    case TDE2_COLOR_FMT_ARGB1555:
    case TDE2_COLOR_FMT_ABGR1555:
    case TDE2_COLOR_FMT_BGRA1555:
        AMin = KEY_MIN(a, 1);
        RMin = KEY_MIN(r, 5);
        GMin = KEY_MIN(g, 5);
        BMin = KEY_MIN(b, 5);
        AMax = KEY_MAX(a, 1);
        RMax = KEY_MAX(r, 5);
        GMax = KEY_MAX(g, 5);
        BMax = KEY_MAX(b, 5);
        break;
    case TDE2_COLOR_FMT_ARGB4444:
    case TDE2_COLOR_FMT_RGBA4444:
    case TDE2_COLOR_FMT_BGRA4444:
    case TDE2_COLOR_FMT_ABGR4444:
        AMin = KEY_MIN(a, 4);
        RMin = KEY_MIN(r, 4);
        GMin = KEY_MIN(g, 4);
        BMin = KEY_MIN(b, 4);
        AMax = KEY_MAX(a, 4);
        RMax = KEY_MAX(r, 4);
        GMax = KEY_MAX(g, 4);
        BMax = KEY_MAX(b, 4);
        break;
    default:
        AMin = a;
        RMin = r;
        GMin = g;
        BMin = b;
        AMax = a;
        RMax = r;
        GMax = g;
        BMax = b;
        break;
    }
    if(enColorFmt == TDE2_COLOR_FMT_AYCbCr8888 || enColorFmt == TDE2_COLOR_FMT_YCbCr422 || enColorFmt == TDE2_COLOR_FMT_CbY0CrY1)
    {
        pKeyValue->struCkYCbCr.stAlpha.u8CompMin = (mt_u8)AMin;
        pKeyValue->struCkYCbCr.stAlpha.u8CompMax = (mt_u8)AMax;
        pKeyValue->struCkYCbCr.stY.u8CompMin = (mt_u8)RMin;
        pKeyValue->struCkYCbCr.stCb.u8CompMax = (mt_u8)GMax;
        pKeyValue->struCkYCbCr.stCr.u8CompMin = (mt_u8)BMin;
        pKeyValue->struCkYCbCr.stY.u8CompMax = (mt_u8)RMax;
        pKeyValue->struCkYCbCr.stCb.u8CompMin = (mt_u8)GMin;
        pKeyValue->struCkYCbCr.stCr.u8CompMax = (mt_u8)BMax;
    }
    else
    {
        pKeyValue->struCkARGB.stAlpha.u8CompMin = (mt_u8)AMin;
        pKeyValue->struCkARGB.stAlpha.u8CompMax = (mt_u8)AMax;
        pKeyValue->struCkARGB.stRed.u8CompMin = (mt_u8)RMin;
        pKeyValue->struCkARGB.stRed.u8CompMax = (mt_u8)RMax;
        pKeyValue->struCkARGB.stGreen.u8CompMin = (mt_u8)GMin;
        pKeyValue->struCkARGB.stGreen.u8CompMax = (mt_u8)GMax;
        pKeyValue->struCkARGB.stBlue.u8CompMin = (mt_u8)BMin;
        pKeyValue->struCkARGB.stBlue.u8CompMax = (mt_u8)BMax;
    }
    return MT_SUCCESS;
}

#if 0
static mt_u32 get_key_min(pix_fmt_t fmt, mt_u32 key_argb8888)
{
    mt_u8 a = (mt_u8)((key_argb8888 >> 24) & 0xff);
    mt_u8 r = (mt_u8)((key_argb8888 >> 16) & 0xff);
    mt_u8 g = (mt_u8)((key_argb8888 >> 8) & 0xff);
    mt_u8 b = (mt_u8)(key_argb8888 & 0xff);
    mt_u32 key_min = key_argb8888;
    switch(fmt)
    {
    case PIX_FMT_RGB233:
        key_min =  (mt_u32)((KEY_MIN(a, 8) << 24) | (KEY_MIN(r, 2) << 16) | (KEY_MIN(g, 3) << 8) | KEY_MIN(b, 3));
        break;
    case PIX_FMT_RGB565:
    case PIX_FMT_RGB565_SMALL_ENDIAN:
        key_min =  (mt_u32)((KEY_MIN(a, 8) << 24) | (KEY_MIN(r, 5) << 16) | (KEY_MIN(g, 6) << 8) | KEY_MIN(b, 5));
        break;
    case PIX_FMT_RGBA5551:
    case PIX_FMT_ARGB1555:
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
        key_min =  (mt_u32)((KEY_MIN(a, 1) << 24) | (KEY_MIN(r, 5) << 16) | (KEY_MIN(g, 5) << 8) | KEY_MIN(b, 5));
        break;
    case PIX_FMT_ARGB4444:
    case PIX_FMT_RGBA4444:
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
        key_min =  (mt_u32)((KEY_MIN(a, 4) << 24) | (KEY_MIN(r, 4) << 16) | (KEY_MIN(g, 4) << 8) | KEY_MIN(b, 4));
        break;
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
        key_min =  (mt_u32)((KEY_MIN(a, 4) << 24) | b);
        break;
    default:
        key_min = key_argb8888;
        break;
    }
    return key_min;
}

static mt_u32 get_key_max(pix_fmt_t fmt, mt_u32 key_argb8888)
{
    mt_u8 a = (mt_u8)((key_argb8888 >> 24) & 0xff);
    mt_u8 r = (mt_u8)((key_argb8888 >> 16) & 0xff);
    mt_u8 g = (mt_u8)((key_argb8888 >> 8) & 0xff);
    mt_u8 b = (mt_u8)(key_argb8888 & 0xff);
    mt_u32 key_min = key_argb8888;
    switch(fmt)
    {
    case PIX_FMT_RGB233:
        key_min =  (mt_u32)((KEY_MAX(a, 8) << 24) | (KEY_MAX(r, 2) << 16) | (KEY_MAX(g, 3) << 8) | KEY_MAX(b, 3));
        break;
    case PIX_FMT_RGB565:
    case PIX_FMT_RGB565_SMALL_ENDIAN:
        key_min =  (mt_u32)((KEY_MAX(a, 8) << 24) | (KEY_MAX(r, 5) << 16) | (KEY_MAX(g, 6) << 8) | KEY_MAX(b, 5));
        break;
    case PIX_FMT_RGBA5551:
    case PIX_FMT_ARGB1555:
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
        key_min =  (mt_u32)((KEY_MAX(a, 1) << 24) | (KEY_MAX(r, 5) << 16) | (KEY_MAX(g, 5) << 8) | KEY_MAX(b, 5));
        break;
    case PIX_FMT_ARGB4444:
    case PIX_FMT_RGBA4444:
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
        key_min =  (mt_u32)((KEY_MAX(a, 4) << 24) | (KEY_MAX(r, 4) << 16) | (KEY_MAX(g, 4) << 8) | KEY_MAX(b, 4));
        break;
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
        key_min =  (mt_u32)((KEY_MAX(a, 4) << 24) | b);
        break;
    default:
        key_min = key_argb8888;
        break;
    }
    return key_min;
}
#endif
static MT_BOOL aria_gpe_set_format(aria_param_t *param, src_ch_t ch, aria_gpe_context_t *p_ctx)
{
    MT_BOOL is_pix_alpha = MT_FALSE;
    MT_BOOL is_support = MT_FALSE;
    MT_BOOL with_palette = MT_FALSE;
    MT_BOOL is_xylc = MT_FALSE;
    MT_BOOL is_tile = MT_FALSE;
    MT_BOOL is_sp = MT_FALSE;
    MT_BOOL pix_little_endian = 0;
    MT_BOOL palt_little_endian = 0;
    mt_u32  bpp = 0;
    color_space_t  color_space = RGB_COLOR_SPACE;
    palette_format_t palt_format = GPE_ARIA_PALT_ARGB8888;
    color_format_t color_fmt = CLUT_1;
    mt_u32 pix_fmt = PIX_FMT_RGBPALETTE1;

    if(ch == SPN_SRC1)
        pix_fmt = param->src_img.pix_format;
    else if(ch == SPN_DST)
        pix_fmt = param->dst_img.pix_format;
    else if(ch == SPN_SRC3)
        pix_fmt = param->ex_img.pix_format;
    else  if(ch == SPN_SRC2)
        pix_fmt = param->bg_img.pix_format;

    MT_INFO_TDE("ch=%d, pix_fmt=%d\n", ch, pix_fmt);
    switch(pix_fmt)
    {
    case PIX_FMT_TILE:
        is_tile = MT_TRUE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = TILE_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XY:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XY;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYL:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XYL;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYC:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XYC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYLC:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XYLC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XY_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XY;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYL_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XYL;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYC_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XYC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_XYLC_SMALL:
        is_xylc = MT_TRUE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = XYLC;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_RGBPALETTE1_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_1BIT;
        color_fmt = CLUT_1;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_GRAY_8:
        is_xylc = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = GRAY_8;
        color_space = GRAY_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE1:
        bpp = GPE_ARIA_BPP_1BIT;
        color_fmt = CLUT_1;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE2:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE8:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE1:
        bpp = GPE_ARIA_BPP_1BIT;
        color_fmt = CLUT_1;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE2:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE8:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB565:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = RGB565;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB1555:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ARGB1555;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA5551:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = RGBA5551;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB4444:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ARGB4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA4444:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = RGBA4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB8888:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = ARGB8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA8888:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = RGBA8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_Y1CRY0CB8888:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = Y1VY0U;//UY0VY1;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_CBY0CRY18888:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = Y1VY0U;//UY0VY1;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYCBCR8888:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = AYUV8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_CRCBYA8888:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = AYUV8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YCBCRA8888:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = YUVA8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ACRCBY8888:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = YUVA8888;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE2_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE2_PALETTE_VUYA:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4_PALETTE_VUYA:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_AYUV8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB565_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = RGB565;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ARGB1555;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ARGB4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGB8888_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = ARGB8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA8888_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = RGBA8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = RGBA5551;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = RGBA4444;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE2_PALETTE_RGBA:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE2_PALETTE_ABGR:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4_PALETTE_RGBA:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE4_PALETTE_ABGR:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE2_PALETTE_YUVA:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE2_PALETTE_AVUY:
        bpp = GPE_ARIA_BPP_2BIT;
        color_fmt = CLUT_2;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4_PALETTE_YUVA:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE4_PALETTE_AVUY:
        bpp = GPE_ARIA_BPP_4BIT;
        color_fmt = CLUT_4;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        if(ch == SPN_DST)
        {
            is_support = MT_FALSE;
        }
        break;
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = CLUT_8;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_RGBA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_ARGB8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = ALUT44;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 0;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 0;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = ALUT88;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_TRUE;
        with_palette = MT_TRUE;
        pix_little_endian = 1;
        palt_little_endian = 1;
        palt_format = GPE_ARIA_PALT_YUVA8888;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB233:
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = RGB233;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_RGB888:
        bpp = GPE_ARIA_BPP_24BIT;
        color_fmt = RGB888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_BGR888:
        bpp = GPE_ARIA_BPP_24BIT;
        color_fmt = BGR888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_GRAY_16:
        is_xylc = MT_FALSE;
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = GRAY_16;
        color_space = GRAY_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        if(ch == SPN_SRC1)
            is_support = MT_TRUE;
        else
            is_support = MT_FALSE;
        break;
    case PIX_FMT_SP_YUV444:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV444_Y;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422:
    case PIX_FMT_SP_YUV422_1x2:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV422_Y;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422_2x1:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV422_Y2;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV420:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV420_Y;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV444_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV444_Y;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422_UVSWAP:
    case PIX_FMT_SP_YUV422_1x2_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV422_Y;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV422_2x1_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV422_Y2;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_YUV420_UVSWAP:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_8BIT;
        color_fmt = SP_YUV420_Y;//SP_YUV_Y;
        color_space = YUV_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_CMYK:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = CMYK8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_FALSE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_KYMC:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_32BIT;
        color_fmt = CMYK8888;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_FALSE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_CMYK:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = SP_CMYK8888_CM;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 0;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
    case PIX_FMT_SP_CMYK_SWAP:
        is_tile = MT_FALSE;
        bpp = GPE_ARIA_BPP_16BIT;
        color_fmt = SP_CMYK8888_CM;
        color_space = RGB_COLOR_SPACE;
        is_pix_alpha = MT_FALSE;
        with_palette = MT_FALSE;
        pix_little_endian = 1;
        is_sp = MT_TRUE;
        is_support = MT_TRUE;
        break;
        //RGBA5551,RGBA4444,alut88 small endian
        //gray-8,gray-16, tile, xylc
    case PIX_FMT_ARGBPALETTE11:
    case PIX_FMT_ARGBPALETTE22:
    case PIX_FMT_AYUVPALETTE11:
    case PIX_FMT_AYUVPALETTE22:
    case PIX_FMT_Y0CBY1CR8888:
    case PIX_FMT_Y0CRY1CB8888:
    case PIX_FMT_Y1CBY0CR8888:
    case PIX_FMT_CBY1CRY08888:
    case PIX_FMT_CRY1CBY08888:
    case PIX_FMT_CRY0CBY18888:
    case PIX_FMT_X2C10Y10CB10:
    case PIX_FMT_YCBCR444:
    case PIX_FMT_YCBCR422:
    case PIX_FMT_YCBCR420:
        is_support = MT_FALSE;
        break;
    default:
        is_support = MT_FALSE;
        break;
    }

    if(is_support)
    {
        if(ch == SPN_SRC1)
        {
            p_ctx->src_img.color_info.bpp = bpp;
            p_ctx->src_img.color_info.color_fmt = color_fmt;
            p_ctx->src_img.color_info.color_space = color_space;
            p_ctx->src_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->src_img.color_info.little_endian = pix_little_endian;
            p_ctx->src_img.with_palette = with_palette;
            p_ctx->src_img.palt_format = palt_format;
            p_ctx->src_img.palt_little_endian = palt_little_endian;
            p_ctx->src_img.palt_size = param->src_img.palette_size;
            p_ctx->src_img.palt_buf = param->src_img.palette_base;
            if(MT_TRUE == is_xylc)
            {
                p_ctx->src_is_xylc = MT_TRUE;
                p_ctx->xylc_cfg.xylc_num = param->src_img.xylc_num;
                p_ctx->xylc_cfg.xylc_color = param->src_img.xylc_color;
            }
            else
            {
                p_ctx->src_is_xylc = MT_FALSE;
            }
            if(MT_TRUE == is_tile)
            {
                p_ctx->src_is_tile = MT_TRUE;
            }
            if((MT_TRUE == is_tile) || (MT_TRUE == is_sp))
            {
                p_ctx->src0_buf = param->src_img.chroma_addr;
                p_ctx->src0_pitch = param->src_img.chroma_pitch;
#ifdef CONFIG_MT_FPGA_GPE
                 p_ctx->src0_buf_vir = g_debug.chroma_vir_addr;
#endif				 
            }
        }
        else if(ch == SPN_DST)
        {
            p_ctx->dst_img.color_info.bpp = bpp;
            p_ctx->dst_img.color_info.color_fmt = color_fmt;
            p_ctx->dst_img.color_info.color_space = color_space;
            p_ctx->dst_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->dst_img.color_info.little_endian = pix_little_endian;
            p_ctx->dst_img.with_palette = with_palette;
            p_ctx->dst_img.palt_format = palt_format;
            p_ctx->dst_img.palt_little_endian = palt_little_endian;
        }
        else if(ch == SPN_SRC3)
        {
            p_ctx->ex_img.color_info.bpp = bpp;
            p_ctx->ex_img.color_info.color_fmt = color_fmt;
            p_ctx->ex_img.color_info.color_space = color_space;
            p_ctx->ex_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->ex_img.color_info.little_endian = pix_little_endian;
            p_ctx->ex_img.with_palette = with_palette;
            p_ctx->ex_img.palt_format = palt_format;
            p_ctx->ex_img.palt_little_endian = palt_little_endian;
            p_ctx->ex_img.palt_size = param->ex_img.palette_size;
            p_ctx->ex_img.palt_buf = param->ex_img.palette_base;
        }
        else if(ch == SPN_SRC2)
        {
            p_ctx->bg_img.color_info.bpp = bpp;
            p_ctx->bg_img.color_info.color_fmt = color_fmt;
            p_ctx->bg_img.color_info.color_space = color_space;
            p_ctx->bg_img.color_info.is_pix_alpha = is_pix_alpha;
            p_ctx->bg_img.color_info.little_endian = pix_little_endian;
        }
    }
    return is_support;
}

static MT_BOOL aria_gpe_check_palette(aria_gpe_context_t *p_ctx)
{
    MT_BOOL is_support = MT_TRUE;


    if(p_ctx->src_img_en  &&  p_ctx->src_img.with_palette)
        p_ctx->src1_palt_load_en = MT_TRUE;

    if(p_ctx->ex_img_en  &&  p_ctx->ex_img.with_palette)
        p_ctx->src3_palt_load_en = MT_TRUE;

    //if dst is lut, src must be lut to, no need to load palette
    if(p_ctx->dst_img.with_palette)
    {
        if(p_ctx->src_img_en)
        {
            if((p_ctx->src_img.with_palette == MT_FALSE)  ||
                    (p_ctx->dst_img.palt_format != p_ctx->src_img.palt_format)  ||
                    (p_ctx->dst_img.palt_little_endian != p_ctx->src_img.palt_little_endian))
            {
                is_support = MT_FALSE;
                MT_ERR_TDE("[ERROR] src and dst not use the same palette!\n");
            }
        }
        p_ctx->src1_palt_load_en = MT_FALSE;
        p_ctx->src3_palt_load_en = MT_FALSE;
    }
    return is_support;
}

#if 0
static void aria_gpe_get_ck_min_max(gpe_img_t *p_img, mt_u32 key_min, mt_u32 key_max, pix_fmt_t pix_format)
{
    mt_u32 ck_min = 0;
    mt_u32 ck_max = 0;
    if(is_lut(pix_format))
    {
        ck_min = aria_lut_patterncolor(pix_format, key_min);
        ck_max = aria_lut_patterncolor(pix_format, key_max);
    }
    else
    {
        ck_min = aria_color_expend(pix_format, key_min, MT_TRUE);
        ck_max = aria_color_expend(pix_format, key_max, MT_TRUE);
    }

    p_img->ck_min= get_key_min(pix_format, ck_min);
    p_img->ck_max= get_key_max(pix_format, ck_max);
}
#endif
static MT_BOOL aria_gpe_set_img(gpe_img_t *p_img, image_info_t *img_info)
{
    MT_BOOL ret = MT_TRUE;

    p_img->buf = img_info->buf;
    p_img->width = img_info->width;
    p_img->height = img_info->height;
    p_img->pitch = img_info->pitch;
    p_img->negative_stride = img_info->negative_stride;
    p_img->rect.w = img_info->rect.w;
    p_img->rect.h = img_info->rect.h;
    p_img->rect.x = img_info->rect.x;
    p_img->rect.y = img_info->rect.y;
    p_img->ck_en = img_info->ck_en;
    p_img->ck_mod= img_info->key_color_mod;
    p_img->ck_select = img_info->key_color_select;
    p_img->plane_alpha_en = img_info->plane_alpha_en;
    p_img->plane_alpha = img_info->plane_alpha;
    p_img->color_info.alpha_ch_en = img_info->alpha_ch_en;
    p_img->color_info.alpha_pre_mult_en = !(img_info->alpha_pre_multed);

    if(!p_img->buf)
        ret = MT_FALSE;

    if(p_img->ck_en)
    {
        if((p_img->color_info.is_pix_alpha == MT_FALSE) || (p_img->color_info.alpha_ch_en == MT_FALSE))
            p_img->ck_mod = ((p_img->ck_mod) & 0x0fff) | (KEY_MATCH_ALL << 12);
        if(p_img->with_palette)
            p_img->ck_mod = ((p_img->ck_mod) & 0xf00f) | (KEY_MATCH_ALL << 4) | (KEY_MATCH_ALL << 8);
    }
    if((!p_img->width)  ||  (!p_img->height))
    {
        MT_ERR_TDE("\n\r img size error! width=%d, height=%d",
                p_img->width, p_img->height);
        ret = MT_FALSE;
    }

    if((!p_img->rect.w)  ||  (!p_img->rect.h)
            ||  (p_img->rect.x >= p_img->width)
            ||  (p_img->rect.y >= p_img->height))
    {
        MT_ERR_TDE("\n\r img rect start pos error! pos=(%d,%d),w=%d,h=%d,img_w=%d,img_h=%d",
                p_img->rect.x, p_img->rect.y, p_img->rect.w,
                p_img->rect.h, p_img->width, p_img->height);
        ret = MT_FALSE;
    }

    if(p_img->rect.w + p_img->rect.x > p_img->width)
    {
        MT_INFO_TDE("\n\r img rect width too large! x=%d, w=%d, img_w=%d",
                p_img->rect.x, p_img->rect.w, p_img->width);
        p_img->rect.w = p_img->width - 1 - p_img->rect.x;
    }

    if(p_img->rect.h + p_img->rect.y > p_img->height)
    {
        MT_INFO_TDE("\n\r img rect height too large! y=%d, h=%d, img_h=%d",
                p_img->rect.y, p_img->rect.h, p_img->height);
        p_img->rect.h = p_img->height - 1 - p_img->rect.y;
    }

    return ret;
}

static void aria_gpe_calc_liner_gradient(paint_cfg_t *p_paint)
{
    mt_u32 x0 = p_paint->begin.x;
    mt_u32 x1 = p_paint->end.x;
    mt_u32 y0 = p_paint->begin.y;
    mt_u32 y1 = p_paint->end.y;
    int xd = 0;
    int yd = 0;
    int numerator = 0;
    int denominator = 0;

    xd = (int)x0 - (int)x1;
    yd = (int)y0 - (int)y1;
    if((xd == 0)  &&  (yd == 0))
    {
        p_paint->gradt_start = (1   <<   20);
        p_paint->step_x = 0;
        p_paint->step_y = 0;
    }
    else
    {
        denominator = xd * xd + yd * yd;
        numerator = (int)x0 * xd + (int)y0 * yd;
        p_paint->gradt_start = (int)(((double)numerator / (double)denominator) * (1   <<   20));
        p_paint->step_x = (int)(((double)(- xd) / (double)denominator) * (1   <<   21));
        p_paint->step_y = (int)(((double)(- yd) / (double)denominator) * (1   <<   21));
    }
}

static void aria_gpe_calc_gradt_stop_fact(paint_cfg_t *p_paint)
{
    mt_u32 data = 0;
    mt_u32 offset0 = p_paint->stop0.offset;
    mt_u32 offset1 = p_paint->stop1.offset;
    mt_u32 offset2 = p_paint->stop2.offset;
    mt_u32 offset3 = p_paint->stop3.offset;


    if(offset1 == offset0)
    {
        data = 0;
    }
    else
    {
        data = ((1   <<   24) / (offset1 - offset0)) & 0xffffff;
    }
    p_paint->stop_fact0 = (mt_s32)data;

    if(offset2 == offset1)
    {
        data = 0;
    }
    else
    {
        data = ((1   <<   24) / (offset2 - offset1)) & 0xffffff;
    }
    p_paint->stop_fact1 = (mt_s32)data;

    if(offset3 == offset2)
    {
        data = 0;
    }
    else
    {
        data = ((1   <<   24) / (offset3 - offset2)) & 0xffffff;
    }
    p_paint->stop_fact2 = (mt_s32)data;
}


static MT_BOOL aria_gpe_set_paint(paint_cfg_t *p_paint, paint_info_t *paint_info)
{
    MT_BOOL ret = MT_TRUE;
    mt_u32 mod = 0;
    mt_u32 mask_mod = 0;

    switch(paint_info->paint_type)
    {
    case VG_PAINT_TYPE_COLOR:
        p_paint->is_pattern_paint = MT_FALSE;
        p_paint->true_liner_gradt = MT_FALSE;
        //hardware theat flat color paint as liner gradient paint
        p_paint->gradt_type = GPE_ARIA_LINER_GRADT;
        p_paint->paint_color = paint_info->paint_color;
        p_paint->spread_mod = GPE_ARIA_FLAT_COLOR_FILL;
        break;
    case VG_PAINT_TYPE_LINEAR_GRADIENT:
        p_paint->is_pattern_paint = MT_FALSE;
        p_paint->true_liner_gradt = MT_TRUE;
        p_paint->gradt_type = GPE_ARIA_LINER_GRADT;
        p_paint->begin.x = paint_info->paint_liner_gradt.begin.x;
        p_paint->begin.y = paint_info->paint_liner_gradt.begin.y;
        p_paint->end.x = paint_info->paint_liner_gradt.end.x;
        p_paint->end.y = paint_info->paint_liner_gradt.end.y;
        p_paint->stop0.argb = paint_info->paint_liner_gradt.stop0.argb;
        p_paint->stop1.argb = paint_info->paint_liner_gradt.stop1.argb;
        p_paint->stop2.argb = paint_info->paint_liner_gradt.stop2.argb;
        p_paint->stop3.argb = paint_info->paint_liner_gradt.stop3.argb;
        p_paint->stop0.offset = paint_info->paint_liner_gradt.stop0.offset;
        p_paint->stop1.offset = paint_info->paint_liner_gradt.stop1.offset;
        p_paint->stop2.offset = paint_info->paint_liner_gradt.stop2.offset;
        p_paint->stop3.offset = paint_info->paint_liner_gradt.stop3.offset;
        aria_gpe_calc_liner_gradient(p_paint);
        aria_gpe_calc_gradt_stop_fact(p_paint);
        break;
    case VG_PAINT_TYPE_RADIAL_GRADIENT:
        p_paint->is_pattern_paint = MT_FALSE;
        p_paint->true_liner_gradt = MT_FALSE;
        p_paint->gradt_type = GPE_ARIA_RADIAL_GRADT;
        p_paint->center.x = paint_info->paint_radial_gradt.center.x;
        p_paint->center.y = paint_info->paint_radial_gradt.center.y;
        p_paint->focus.x = paint_info->paint_radial_gradt.focus.x;
        p_paint->focus.y = paint_info->paint_radial_gradt.focus.y;
        p_paint->radius = paint_info->paint_radial_gradt.radius;
        p_paint->long_a = p_paint->radius;
        p_paint->short_b = p_paint->radius;
        p_paint->stop0.argb = paint_info->paint_radial_gradt.stop0.argb;
        p_paint->stop1.argb = paint_info->paint_radial_gradt.stop1.argb;
        p_paint->stop2.argb = paint_info->paint_radial_gradt.stop2.argb;
        p_paint->stop3.argb = paint_info->paint_radial_gradt.stop3.argb;
        p_paint->stop0.offset = paint_info->paint_radial_gradt.stop0.offset;
        p_paint->stop1.offset = paint_info->paint_radial_gradt.stop1.offset;
        p_paint->stop2.offset = paint_info->paint_radial_gradt.stop2.offset;
        p_paint->stop3.offset = paint_info->paint_radial_gradt.stop3.offset;
        aria_gpe_calc_gradt_stop_fact(p_paint);
        break;
    case VG_PAINT_TYPE_ELLIPSE_GRADIENT:
        p_paint->is_pattern_paint = MT_FALSE;
        p_paint->true_liner_gradt = MT_FALSE;
        p_paint->gradt_type = GPE_ARIA_ELLIPSE_GRADT;
        p_paint->center.x = paint_info->paint_ellipse_gradt.center.x;
        p_paint->center.y = paint_info->paint_ellipse_gradt.center.y;
        p_paint->focus.x = paint_info->paint_ellipse_gradt.focus.x;
        p_paint->focus.y = paint_info->paint_ellipse_gradt.focus.y;
        p_paint->long_a = paint_info->paint_ellipse_gradt.long_a;
        p_paint->short_b = paint_info->paint_ellipse_gradt.short_b;
        p_paint->stop0.argb = paint_info->paint_ellipse_gradt.stop0.argb;
        p_paint->stop1.argb = paint_info->paint_ellipse_gradt.stop1.argb;
        p_paint->stop2.argb = paint_info->paint_ellipse_gradt.stop2.argb;
        p_paint->stop3.argb = paint_info->paint_ellipse_gradt.stop3.argb;
        p_paint->stop0.offset = paint_info->paint_ellipse_gradt.stop0.offset;
        p_paint->stop1.offset = paint_info->paint_ellipse_gradt.stop1.offset;
        p_paint->stop2.offset = paint_info->paint_ellipse_gradt.stop2.offset;
        p_paint->stop3.offset = paint_info->paint_ellipse_gradt.stop3.offset;
        p_paint->stop_num = paint_info->paint_ellipse_gradt.stop_num;
        aria_gpe_calc_gradt_stop_fact(p_paint);
        break;
    case VG_PAINT_TYPE_PATTERN:
        p_paint->is_pattern_paint = MT_TRUE;
        p_paint->true_liner_gradt = MT_FALSE;
        p_paint->pat_beg.x = paint_info->paint_pattern.pat_beg.x;
        p_paint->pat_beg.y = paint_info->paint_pattern.pat_beg.y;
        if(paint_info->paint_pattern.tiling_mod == VG_TILE_FILL)
        {
            p_paint->paint_color = paint_info->paint_pattern.fill_color;
        }
        break;
    default:
        ret = MT_FALSE;
        break;
    }

    if((paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)  ||
            (paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT) ||
            (paint_info->paint_type == VG_PAINT_TYPE_ELLIPSE_GRADIENT))
    {
        if(paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)
            mod = paint_info->paint_liner_gradt.spread_mod;
        else if(paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT)
            mod = paint_info->paint_radial_gradt.spread_mod;
        else
            mod = paint_info->paint_ellipse_gradt.spread_mod;
        switch(mod)
        {
        case VG_COLOR_RAMP_SPREAD_PAD:
            p_paint->spread_mod = GPE_ARIA_SPREAD_PAD;
            break;
        case VG_COLOR_RAMP_SPREAD_REPEAT:
            p_paint->spread_mod = GPE_ARIA_SPREAD_REPEAT;
            break;
        case VG_COLOR_RAMP_SPREAD_REFLECT:
            p_paint->spread_mod = GPE_ARIA_SPREAD_REFLECT;
            break;
        default:
            ret = MT_FALSE;
            break;
        }
    }
    if((paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)  ||
            (paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT) ||
            (paint_info->paint_type == VG_PAINT_TYPE_ELLIPSE_GRADIENT))
    {
        if(paint_info->paint_type == VG_PAINT_TYPE_LINEAR_GRADIENT)
            mask_mod = paint_info->paint_liner_gradt.mask_mod;
        else if(paint_info->paint_type == VG_PAINT_TYPE_RADIAL_GRADIENT)
            mask_mod = paint_info->paint_radial_gradt.mask_mod;
        else
            mask_mod = paint_info->paint_ellipse_gradt.mask_mod;
        switch(mask_mod)
        {
        case VG_GRADIENT_MASK_0:
            p_paint->mask_mod = GPE_ARIA_MASK_0;
            break;
        case VG_GRADIENT_MASK_1:
            p_paint->mask_mod = GPE_ARIA_MASK_1;
            break;
        case VG_GRADIENT_MASK_2:
            p_paint->mask_mod = GPE_ARIA_MASK_2;
            break;
        default:
            ret = MT_FALSE;
            break;
        }
    }
    if(paint_info->paint_type == VG_PAINT_TYPE_PATTERN)
    {
        switch(paint_info->paint_pattern.tiling_mod)
        {
        case VG_TILE_FILL:
            p_paint->tiling_mod = GPE_ARIA_TILE_FILL;
            break;
        case VG_TILE_PAD:
            p_paint->tiling_mod = GPE_ARIA_TILE_PAD;
            break;
        case VG_TILE_REPEAT:
            p_paint->tiling_mod = GPE_ARIA_TILE_REPEAT;
            break;
        case VG_TILE_REFLECT:
            p_paint->tiling_mod = GPE_ARIA_TILE_REFLECT;
            break;
        default:
            ret = MT_FALSE;
            break;
        }
    }
    return ret;
}

static MT_BOOL aria_gpe_set_rop_mod(mt_u32 rop_mod_in, rop_mod_t *rop_mod, aria_rop_ch_t *rop_ch)
{
    MT_BOOL ret = MT_TRUE;
    rop_ch->src_in = MT_FALSE;
    rop_ch->dst_in = MT_FALSE;

    *rop_mod = rop_mod_in;
    switch(rop_mod_in)
    {
    case ROP_COPYPEN:
    case ROP_NOTCOPYPEN:
    case ROP_MERGEPAINT:
        rop_ch->src_in = MT_TRUE;
        break;
    case ROP_MERGEPEN:
    case ROP_MASKPEN:
    case ROP_XORPEN:
    case ROP_MASKPENNOT:
    case ROP_NOTMERGEPEN:
    case ROP_MERGENOTPEN:
    case ROP_PATPAINT:
    case ROP_MASKNOTPEN:
    case ROP_NOTMASKPEN:
    case ROP_NOTXORPEN:
    case ROP_MERGEPENNOT:
        rop_ch->src_in = MT_TRUE;
        rop_ch->dst_in = MT_TRUE;
        break;
    case ROP_PATINVERT:
    case ROP_NOT:
    case ROP_NOP:
        rop_ch->dst_in = MT_TRUE;
        break;
    case ROP_BLACK:
    case ROP_WHITE:
    case ROP_PATCOPY:
        break;
    default:
        ret = MT_FALSE;
        break;
    }
    return ret;
}

static MT_BOOL aria_gpe_set_blend_mod(mt_u32 blend_mod_in,
        blend_fact_t *blend_mod,
        MT_BOOL stencil_en)
{
    MT_BOOL ret = MT_TRUE;

    switch(blend_mod_in)
    {
    case GL_ZERO:
        *blend_mod = GPE_ARIA_GL_ZERO;
        break;
    case GL_ONE:
        *blend_mod = GPE_ARIA_GL_ONE;
        break;
    case GL_DST_COLOR:
        *blend_mod = GPE_ARIA_GL_DST_COLOR;
        break;
    case GL_SRC_COLOR:
        if(stencil_en)
            *blend_mod = GPE_ARIA_ST_COLOR;
        else
            *blend_mod = GPE_ARIA_GL_SRC_COLOR;
        break;
    case GL_ONE_MINUS_DST_COLOR:
        *blend_mod = GPE_ARIA_GL_ONE_MINUS_DST_COLOR;
        break;
    case GL_ONE_MINUS_SRC_COLOR:
        if(stencil_en)
            *blend_mod = GPE_ARIA_ST_ONE_MINUS_COLOR;
        else
            *blend_mod = GPE_ARIA_GL_ONE_MINUS_SRC_COLOR;
        break;
    case GL_SRC_ALPHA:
        if(stencil_en)
            *blend_mod = GPE_ARIA_ST_ALPHA;
        else
            *blend_mod = GPE_ARIA_GL_SRC_ALPHA;
        break;
    case GL_ONE_MINUS_SRC_ALPHA:
        if(stencil_en)
            *blend_mod = GPE_ARIA_ST_ONE_MINUS_ALPHA;
        else
            *blend_mod = GPE_ARIA_GL_ONE_MINUS_SRC_ALPHA;
        break;
    case GL_DST_ALPHA:
        *blend_mod = GPE_ARIA_GL_DST_ALPHA;
        break;
    case GL_ONE_MINUS_DST_ALPHA:
        *blend_mod = GPE_ARIA_GL_ONE_MINUS_DST_ALPHA;
        break;
    case GL_SRC_ALPHA_SATURATE:
        if(stencil_en)
            *blend_mod = GPE_ARIA_ST_ALPHA_SATURATE;
        else
            *blend_mod = GPE_ARIA_GL_SRC_ALPHA_SATURATE;
        break;
    default:
        ret = MT_FALSE;
        break;
    }
    return ret;
}

static MT_BOOL aria_gpe_check_rect_size(aria_gpe_context_t *p_ctx)
{
    MT_BOOL ret = MT_TRUE;
    mt_u32 dst_w = p_ctx->dst_img.rect.w;
    mt_u32 dst_h = p_ctx->dst_img.rect.h;

    if(p_ctx->ex_img_en)
    {
        if((p_ctx->ex_img.rect.w != dst_w)  ||  (p_ctx->ex_img.rect.h != dst_h))
        {
            MT_INFO_TDE("\n\rex image rect size should equal to dst rect size!");
            /*ret = MT_FALSE;*/
            p_ctx->ex_img.rect.w = dst_w;
            p_ctx->ex_img.rect.h = dst_h;
        }
    }

    if(p_ctx->src_img_en)
    {
        if((!p_ctx->scale_en)  &&  (!p_ctx->paint_en)  &&  (!p_ctx->rotator_en)
                &&  (!p_ctx->src_is_xylc))
        {
            if((p_ctx->src_img.rect.w != dst_w)  ||  (p_ctx->src_img.rect.h != dst_h))
            {
                MT_INFO_TDE("\n\rsrc image rect size should equal to dst rect size!");
                //ret = MT_FALSE;
                if(p_ctx->src_img.rect.w < dst_w)
                {
                    p_ctx->dst_img.rect.w = p_ctx->src_img.rect.w;
                }
                else
                {
                    p_ctx->src_img.rect.w = dst_w;
                }

                if(p_ctx->src_img.rect.h < dst_h)
                {
                    p_ctx->dst_img.rect.h = p_ctx->src_img.rect.h;
                }
                else
                {
                    p_ctx->src_img.rect.h = dst_h;
                }
                ret = MT_TRUE;
            }
        }
    }
    return ret;
}

static MT_BOOL aria_gpe_global_check(aria_gpe_context_t *p_ctx)
{
    if(!aria_gpe_check_rect_size(p_ctx))
    {
        return MT_FALSE;
    }
    //two draw image mode can't be enabled at the same time
    if(p_ctx->is_draw_multiply  &&  p_ctx->is_draw_stencil)
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] two draw image mode can't be enabled at the same time!\n",
                __FUNCTION__, __LINE__);
        return MT_FALSE;
    }

    //alpha map and draw img can't be enabled at the same time
    if(p_ctx->alpha_map_en  &&  p_ctx->is_draw_multiply)
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] draw image multiply mode can't be enabled with alpha map!\n",
                __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
    if(p_ctx->alpha_map_en  &&  p_ctx->is_draw_stencil)
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] draw image stencil mode can't be enabled with alpha map!\n",
                __FUNCTION__, __LINE__);
        return MT_FALSE;
    }

    //rop and blend can't be enabled at the same time
    if(p_ctx->blend_en  &&  p_ctx->rop_en)
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] blend and rop can't be enabled at the same time!\n",
                __FUNCTION__, __LINE__);
        return MT_FALSE;
    }

    //only simple rop can be enabled when dst format is LUT!
    if(p_ctx->dst_img.with_palette == MT_TRUE)
    {
        if(p_ctx->blend_en)
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] blend can't be enabled for lut format!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        if(p_ctx->rop_en)
        {
            if(!((p_ctx->rop_a_mod == ROP_COPYPEN)
                        ||  (p_ctx->rop_a_mod == ROP_PATCOPY)
                        ||  (p_ctx->rop_a_mod == ROP_NOP)))
            {
                MT_ERR_TDE("[%s][%d]: [ERROR] only simple rop can be used for lut format!\n",
                        __FUNCTION__, __LINE__);
                return MT_FALSE;
            }
            if(!((p_ctx->rop_c_mod == ROP_COPYPEN)
                        ||  (p_ctx->rop_c_mod == ROP_PATCOPY)
                        ||  (p_ctx->rop_c_mod == ROP_NOP)))
            {
                MT_ERR_TDE("[%s][%d]: [ERROR] only simple rop can be used for lut format!\n",
                        __FUNCTION__, __LINE__);
                return MT_FALSE;
            }
        }
    }
    return MT_TRUE;
}
#ifdef SUPPORT_CMDFIFO
static void write_cmdfifo(cmdfifo_node_t *p_node, mt_u32 reg_addr, mt_u32 reg_val)
{
    mt_u32 *p_addr = NULL;
    p_addr = (mt_u32 *)p_node->p_node_addr_vir;
    p_addr[p_node->node_size++] = (reg_addr - GPE_ARIA_BASE) << 8;
    p_addr[p_node->node_size++] = reg_val;
}
#endif

static void aria_gpe_set_gradt_regs(paint_cfg_t *p_paint)
{
    mt_u32 tmp = 0;

    if(p_paint->gradt_start < 0)
    {
        tmp = (mt_u32)((~(-p_paint->gradt_start) + 1) | 0x80000000);
    }
    else
        tmp = (mt_u32)(p_paint->gradt_start);

    gpe_write_register(GPE_ARIA_GRADT_START_V, tmp);

    if(p_paint->step_x < 0)
    {
        tmp = (mt_u32)(((~(-p_paint->step_x) + 1) | 0x400000) & 0x7fffff);
    }
    else
        tmp = (mt_u32)(p_paint->step_x & 0x7fffff);

    gpe_write_register(GPE_ARIA_GRADT_X_STEP, tmp);
    if(p_paint->step_y < 0)
    {
        tmp = (mt_u32)(((~(-p_paint->step_y) + 1) | 0x400000) & 0x7fffff);
    }
    else
        tmp = (mt_u32)(p_paint->step_y & 0x7fffff);
    gpe_write_register(GPE_ARIA_GRADT_Y_STEP, tmp);

}
#ifdef SUPPORT_CMDFIFO
static void spn_gpe_cmdfifo_set_gradt_regs(cmdfifo_node_t *p_node, paint_cfg_t *p_paint)
{
    mt_u32 tmp = 0;

    if(p_paint->gradt_start < 0)
    {
        tmp = (~(-p_paint->gradt_start) + 1) | 0x80000000;
    }
    else
        tmp = p_paint->gradt_start;

    write_cmdfifo(p_node, GPE_ARIA_GRADT_START_V, tmp);

    if(p_paint->step_x < 0)
    {
        tmp = ((~(-p_paint->step_x) + 1) | 0x400000) & 0x7fffff;
    }
    else
        tmp = p_paint->step_x & 0x7fffff;

    write_cmdfifo(p_node, GPE_ARIA_GRADT_X_STEP, tmp);
    if(p_paint->step_y < 0)
    {
        tmp = ((~(-p_paint->step_y) + 1) | 0x400000) & 0x7fffff;
    }
    else
        tmp = p_paint->step_y & 0x7fffff;
    write_cmdfifo(p_node, GPE_ARIA_GRADT_Y_STEP, tmp);

}
#endif

static void aria_gpe_set_gradt_stop_regs(paint_cfg_t *p_paint)
{
    mt_u32 data = 0;
    mt_u32 offset1 = p_paint->stop1.offset;
    mt_u32 offset2 = p_paint->stop2.offset;
    data = (offset2 << 16) | offset1;
    gpe_write_register(GPE_ARIA_STOP_OFFSET, data);

    gpe_write_register(GPE_ARIA_STOP0_FACT, (mt_u32)(p_paint->stop_fact0));
    gpe_write_register(GPE_ARIA_STOP1_FACT, (mt_u32)(p_paint->stop_fact1));
    gpe_write_register(GPE_ARIA_STOP2_FACT, (mt_u32)(p_paint->stop_fact2));

    gpe_write_register(GPE_ARIA_STOP0_ARGB, p_paint->stop0.argb);
    gpe_write_register(GPE_ARIA_STOP1_ARGB, p_paint->stop1.argb);
    gpe_write_register(GPE_ARIA_STOP2_ARGB, p_paint->stop2.argb);
    gpe_write_register(GPE_ARIA_STOP3_ARGB, p_paint->stop3.argb);
}
#ifdef SUPPORT_CMDFIFO
static void spn_gpe_cmdfifo_set_gradt_stop_regs(cmdfifo_node_t *p_node, paint_cfg_t *p_paint)
{
    mt_u32 data = 0;
    mt_u32 offset1 = p_paint->stop1.offset;
    mt_u32 offset2 = p_paint->stop2.offset;
    data = (offset2 << 16) | offset1;
    write_cmdfifo(p_node, GPE_ARIA_STOP_OFFSET, data);

    write_cmdfifo(p_node, GPE_ARIA_STOP0_FACT, p_paint->stop_fact0);
    write_cmdfifo(p_node, GPE_ARIA_STOP1_FACT, p_paint->stop_fact1);
    write_cmdfifo(p_node, GPE_ARIA_STOP2_FACT, p_paint->stop_fact2);

    write_cmdfifo(p_node, GPE_ARIA_STOP0_ARGB, p_paint->stop0.argb);
    write_cmdfifo(p_node, GPE_ARIA_STOP1_ARGB, p_paint->stop1.argb);
    write_cmdfifo(p_node, GPE_ARIA_STOP2_ARGB, p_paint->stop2.argb);
    write_cmdfifo(p_node, GPE_ARIA_STOP3_ARGB, p_paint->stop3.argb);
}
#endif

static void aria_gpe_set_gradt_radius_regs(paint_cfg_t *p_paint)
{
    int cir_x = (int)p_paint->center.x;
    int cir_y = (int)p_paint->center.y;
    long long A = (long long)p_paint->long_a;
    long long B = (long long)p_paint->short_b;
    long long ELEA,ELEB,ELED,ELEE,ELEC;
    long long coef1 = 17592186044416; //44bit
    long long coef2 = 16777216; //24bit
    mt_u32 a0, a1, b0, b1, c0, c1, d0, d1, e0, e1;

    ELEA = (long long) (1.0*(A*A*cir_y*cir_y + B*B*cir_x*cir_x)/(A*A*B*B)*coef2);
    ELEB  = (long long) (1.0*(2*cir_x+1)/(A*A)*coef1);
    ELED  = (long long) (1.0*(2*cir_y+1)/(B*B)*coef1);
    ELEC = (long long) (2.0/(A*A)*coef1);
    ELEE = (long long) (2.0/(B*B)*coef1);

    a0 = (mt_u32)(ELEA & 0xffffffff);
    a1 = (mt_u32)((ELEA >> 32) & 0xfff);
    b0 = (mt_u32)(ELEB & 0xffffffff);
    b1 = (mt_u32)((ELEB >> 32) & 0x1fffff);
    c0 = (mt_u32)(ELEC & 0xffffffff);
    c1 = (mt_u32)((ELEC >> 32) & 0xfff);
    d0 = (mt_u32)(ELED & 0xffffffff);
    d1 = (mt_u32)((ELED >> 32) & 0x1fffff);
    e0 = (mt_u32)(ELEE & 0xffffffff);
    e1 = (mt_u32)((ELEE >> 32) & 0xfff);

    gpe_write_register(GPE_ARIA_GRADT_RADIUS_A_0, a0);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_A_1, a1);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_B_0, b0);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_B_1, b1);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_C_0, c0);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_C_1, c1);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_D_0, d0);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_D_1, d1);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_E_0, e0);
    gpe_write_register(GPE_ARIA_GRADT_RADIUS_E_1, e1);
}

#ifdef SUPPORT_CMDFIFO
static void spn_gpe_cmdfifo_set_gradt_radius_regs(cmdfifo_node_t *p_node, paint_cfg_t *p_paint)
{
    int cir_x = (int)p_paint->center.x;
    int cir_y = (int)p_paint->center.y;
    long long A = (long long)p_paint->long_a;
    long long B = (long long)p_paint->short_b;
    long long ELEA,ELEB,ELED,ELEE,ELEC;
    long long coef1 = 17592186044416; //44bit
    long long coef2 = 16777216; //24bit
    mt_u32 a0, a1, b0, b1, c0, c1, d0, d1, e0, e1;

    ELEA = (long long) (1.0*(A*A*cir_y*cir_y + B*B*cir_x*cir_x)/(A*A*B*B)*coef2);
    ELEB  = (long long) (1.0*(2*cir_x+1)/(A*A)*coef1);
    ELED  = (long long) (1.0*(2*cir_y+1)/(B*B)*coef1);
    ELEC = (long long) (2.0/(A*A)*coef1);
    ELEE = (long long) (2.0/(B*B)*coef1);

    a0 = (mt_u32)(ELEA & 0xffffffff);
    a1 = (mt_u32)((ELEA >> 32) & 0xfff);
    b0 = (mt_u32)(ELEB & 0xffffffff);
    b1 = (mt_u32)((ELEB >> 32) & 0x1fffff);
    c0 = (mt_u32)(ELEC & 0xffffffff);
    c1 = (mt_u32)((ELEC >> 32) & 0xfff);
    d0 = (mt_u32)(ELED & 0xffffffff);
    d1 = (mt_u32)((ELED >> 32) & 0x1fffff);
    e0 = (mt_u32)(ELEE & 0xffffffff);
    e1 = (mt_u32)((ELEE >> 32) & 0xfff);

    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_A_0, a0);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_A_1, a1);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_B_0, b0);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_B_1, b1);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_C_0, c0);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_C_1, c1);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_D_0, d0);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_D_1, d1);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_E_0, e0);
    write_cmdfifo(p_node, GPE_ARIA_GRADT_RADIUS_E_1, e1);
}
#endif
static pix_fmt_t aria_gpe_convert_fmt(TDE2_COLOR_FMT_E enColorFmt, MT_BOOL *palpha_en)
{
    pix_fmt_t fmt = PIX_FMT_ARGB8888;

    *palpha_en = MT_TRUE;
    switch(enColorFmt)
    {
    case TDE2_COLOR_FMT_RGB233:
        fmt = PIX_FMT_RGB233;
        break;
    case TDE2_COLOR_FMT_RGB444:
        fmt = PIX_FMT_ARGB4444;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_BGR444:
        fmt = PIX_FMT_ARGB4444_SMALL_ENDIAN;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_RGB555:
        fmt = PIX_FMT_ARGB1555;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_BGR555:
        fmt = PIX_FMT_ARGB1555_SMALL_ENDIAN;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_RGB565:
        fmt = PIX_FMT_RGB565;
        break;
    case TDE2_COLOR_FMT_BGR565:
        fmt = PIX_FMT_RGB565_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_RGB888:
        fmt = PIX_FMT_ARGB8888;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_BGR888:
        fmt = PIX_FMT_RGBA8888_SMALL_ENDIAN;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_ARGB4444:
        fmt = PIX_FMT_ARGB4444;
        break;
    case TDE2_COLOR_FMT_ABGR4444:
        fmt = PIX_FMT_RGBA4444_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_RGBA4444:
        fmt = PIX_FMT_RGBA4444;
        break;
    case TDE2_COLOR_FMT_BGRA4444:
        fmt = PIX_FMT_ARGB4444_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_ARGB1555:
        fmt = PIX_FMT_ARGB1555;
        break;
    case TDE2_COLOR_FMT_ABGR1555:
        fmt = PIX_FMT_RGBA5551_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_RGBA1555:
        fmt = PIX_FMT_RGBA5551;
        break;
    case TDE2_COLOR_FMT_BGRA1555:
        fmt = PIX_FMT_ARGB1555_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_ARGB8565:
        //        fmt = ;
        break;
    case  TDE2_COLOR_FMT_ABGR8565:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_RGBA8565:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_BGRA8565:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_ARGB8888:
        fmt = PIX_FMT_ARGB8888;
        break;
    case TDE2_COLOR_FMT_ABGR8888:
        fmt = PIX_FMT_RGBA8888_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_RGBA8888:
        fmt = PIX_FMT_RGBA8888;
        break;
    case TDE2_COLOR_FMT_BGRA8888:
        fmt = PIX_FMT_ARGB8888_SMALL_ENDIAN;
        break;
    case TDE2_COLOR_FMT_RABG8888:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_CLUT1:
        fmt = PIX_FMT_RGBPALETTE1;
        break;
    case TDE2_COLOR_FMT_CLUT2:
        fmt = PIX_FMT_RGBPALETTE2;
        break;
    case TDE2_COLOR_FMT_CLUT4:
        fmt = PIX_FMT_RGBPALETTE4;
        break;
    case TDE2_COLOR_FMT_CLUT8:
        fmt = PIX_FMT_RGBPALETTE8;
        break;
    case TDE2_COLOR_FMT_ACLUT44:
        fmt = PIX_FMT_ARGBPALETTE44;
        break;
    case TDE2_COLOR_FMT_ACLUT88:
        fmt = PIX_FMT_ARGBPALETTE88;
        break;
    case TDE2_COLOR_FMT_A1:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_A8:
        fmt = PIX_FMT_GRAY_8;
        break;
    case TDE2_COLOR_FMT_YCbCr888:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_CbCrY888:
        fmt = PIX_FMT_ACRCBY8888;
        *palpha_en = MT_FALSE;
        break;
    case TDE2_COLOR_FMT_AYCbCr8888:
        fmt = PIX_FMT_AYCBCR8888;
        break;
    case TDE2_COLOR_FMT_CbY0CrY1:
        fmt = PIX_FMT_CBY0CRY18888;
        break;
    case TDE2_COLOR_FMT_YCbCr422:
        fmt = PIX_FMT_Y1CRY0CB8888;
        break;
    case TDE2_COLOR_FMT_byte:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_halfword:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_JPG_YCbCr400MBP:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_JPG_YCbCr422MBHP:
        fmt = PIX_FMT_SP_YUV422_1x2;
        break;
    case TDE2_COLOR_FMT_JPG_YCbCr422MBVP:
        fmt = PIX_FMT_SP_YUV422_2x1;
        break;
    case TDE2_COLOR_FMT_MP1_YCbCr420MBP:
        fmt = PIX_FMT_SP_YUV420_UVSWAP;
        break;
    case TDE2_COLOR_FMT_MP2_YCbCr420MBP:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_MP2_YCbCr420MBI:
        //        fmt = ;
        break;
    case TDE2_COLOR_FMT_JPG_YCbCr420MBP:
        fmt = PIX_FMT_SP_YUV420;
        break;
    case TDE2_COLOR_FMT_JPG_YCbCr444MBP:
        fmt = PIX_FMT_SP_YUV444;
        break;
    case TDE2_COLOR_FMT_XY:
        fmt = PIX_FMT_XY;
        break;
    case TDE2_COLOR_FMT_XYL:
        fmt = PIX_FMT_XYL;
        break;
    case TDE2_COLOR_FMT_XYC:
        fmt = PIX_FMT_XYC;
        break;
    case TDE2_COLOR_FMT_XYLC:
        fmt = PIX_FMT_XYLC;
        break;
    case TDE2_COLOR_FMT_TILE:
        fmt = PIX_FMT_TILE;
        break;
    case TDE2_COLOR_FMT_JPG_SP_CMYK:
        fmt = PIX_FMT_SP_CMYK;
        break;
    case TDE2_COLOR_FMT_RGB24:
        fmt = PIX_FMT_RGB888;
        break;
    case TDE2_COLOR_FMT_BGR24:
        fmt = PIX_FMT_BGR888;
        break;
    default:
        break;
    }

    return fmt;
}

static rop_mod_t aria_gpe_convert_rop(TDE2_ROP_CODE_E rop)
{
    rop_mod_t rop_mod = 0;
    switch (rop)
    {
    case TDE2_ROP_BLACK:
        rop_mod = ROP_BLACK;
        break;
    case TDE2_ROP_NOTMERGEPEN:
        rop_mod = ROP_NOTMERGEPEN;
        break;
    case TDE2_ROP_MASKNOTPEN:
        rop_mod = ROP_MASKNOTPEN;
        break;
    case TDE2_ROP_NOTCOPYPEN:
        rop_mod = ROP_NOTCOPYPEN;
        break;
    case TDE2_ROP_MASKPENNOT:
        rop_mod = ROP_MASKPENNOT;
        break;
    case TDE2_ROP_NOT:
        rop_mod = ROP_NOT;
        break;
    case TDE2_ROP_XORPEN:
        rop_mod = ROP_XORPEN;
        break;
    case TDE2_ROP_NOTMASKPEN:
        rop_mod = ROP_NOTMASKPEN;
        break;
    case TDE2_ROP_MASKPEN:
        rop_mod = ROP_MASKPEN;
        break;
    case TDE2_ROP_NOTXORPEN:
        rop_mod = ROP_NOTXORPEN;
        break;
    case TDE2_ROP_NOP:
        rop_mod = ROP_NOP;
        break;
    case TDE2_ROP_MERGENOTPEN:
        rop_mod = ROP_MERGENOTPEN;
        break;
    case TDE2_ROP_COPYPEN:
        rop_mod = ROP_COPYPEN;
        break;
    case TDE2_ROP_MERGEPENNOT:
        rop_mod = ROP_MERGEPENNOT;
        break;
    case TDE2_ROP_MERGEPEN:
        rop_mod = ROP_MERGEPEN;
        break;
    case TDE2_ROP_WHITE:
        rop_mod = ROP_WHITE;
        break;
    case TDE2_ROP_PATINVERT:
        rop_mod = ROP_PATINVERT;
        break;
    case TDE2_ROP_MERGEPAINT:
        rop_mod = ROP_MERGEPAINT;
        break;
    case TDE2_ROP_PATCOPY:
        rop_mod = ROP_PATCOPY;
        break;
    case TDE2_ROP_PATPAINT:
        rop_mod = ROP_PATPAINT;
        break;
    default:
        MT_ASSERT(0);
        break;
    }

    return rop_mod;
}

static bld_fact_t aria_gpe_convert_blend(TDE2_BLEND_MODE_E bld_mod)
{
    bld_fact_t fact = GL_ZERO;
    switch (bld_mod)
    {
    case TDE2_BLEND_ZERO:
        fact = GL_ZERO;
        break;
    case TDE2_BLEND_ONE:
        fact = GL_ONE;
        break;
    case TDE2_BLEND_SRC2COLOR:
        fact = GL_SRC_COLOR;
        break;
    case TDE2_BLEND_INVSRC2COLOR:
        fact = GL_ONE_MINUS_SRC_COLOR;
        break;
    case TDE2_BLEND_SRC2ALPHA:
        fact = GL_SRC_ALPHA;
        break;
    case TDE2_BLEND_INVSRC2ALPHA:
        fact = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case TDE2_BLEND_SRC1COLOR:
        fact = GL_DST_COLOR;
        break;
    case TDE2_BLEND_INVSRC1COLOR:
        fact = GL_ONE_MINUS_DST_COLOR;
        break;
    case TDE2_BLEND_SRC1ALPHA:
        fact = GL_DST_ALPHA;
        break;
    case TDE2_BLEND_INVSRC1ALPHA:
        fact = GL_ONE_MINUS_DST_ALPHA;
        break;
    case TDE2_BLEND_SRC2ALPHASAT:
        fact = GL_SRC_ALPHA_SATURATE;
        break;
    default:
        MT_ASSERT(0);
        break;
    }

    return fact;
}

static int my_floor(double x)
{
    if(x >= 0)
        return (int)x;
    else if(fabs(x - (int)x) <= EPSINON)
        return (int)x;
    else
        return (int)x - 1;
}

static int  linx_int( float a )
{
    int b;
    if( a < 0 )
        b = (int)(a) * ( -1 );
    else
        b = (int)(a);
    return b;
}

static void aria_gpe_generate_radial_gradient(aria_gpe_context_t *p_ctx)
{
    mt_u32 width = p_ctx->dst_img.rect.w;
    mt_u32 height = p_ctx->dst_img.rect.h;
    mt_u32 cx = p_ctx->paint.center.x;
    mt_u32 cy = p_ctx->paint.center.y;
    mt_u32 fx = p_ctx->paint.focus.x;
    mt_u32 fy = p_ctx->paint.focus.y;
    mt_u32 r = p_ctx->paint.radius;
    int fc_x = (int)fx - (int)cx;
    int fc_y = (int)fy - (int)cy;
    mt_u32 fc_len = 0;
    mt_u32 x = 0;
    mt_u32 y = 0;
    mt_u32 d1 = 0;
    mt_u32 d2 = 0;
    double gradt = 0;
    int dx = 0;
    int dy = 0;
    int tmp1 = 0;
    int tmp2 = 0;
    mt_u32 dy_dy = 0;
    int dy_fcy = 0;
    mt_u32 r_r = 0;
    mt_u16 *p_ptr = NULL;
    mt_u64 val = 0;
    mt_u32 pitch = 0;

    pitch = width * 2;
    p_ctx->p_gradt_buf = mt_mmz_new(pitch * height, 8, NULL, MOD_NAME);

    if(p_ctx->p_gradt_buf == 0)
    {
        MT_ERR_TDE("\n\rgradt_buf malloc failed!");
        MT_ASSERT(0);
    }
#ifdef SUPPORT_CMDFIFO
    if(MT_SUCCESS != p_gradt_buf_backup(p_ctx->p_gradt_buf, p_gradt_buf_bak , BAK_BUFFER_NUM_MAX))
        printf("<%s> : <%d> Warning : cmdfifo mode will memory overlack\n", __FUNCTION__, __LINE__);
#endif
    p_ptr = (mt_u16 *)mt_mmz_map(p_ctx->p_gradt_buf, 0);
    //MT_ERR_TDE("\r\n f:[%d,%d]", fx, fy);
    val = (mt_u64)(fc_x * fc_x + fc_y * fc_y);
    fc_len = (mt_u32)(sqrt(val));
    if(fc_len >= r) //the focus point is outsize the circle, clamp it
    {
        if(fc_x > 0)
        {
            fx = (mt_u32)((fc_x * r * 1023 / 1024) / fc_len + cx);
        }
        else
        {
            fx = (mt_u32)(cx - ((cx - fx) * r * 1023 / 1024 / fc_len));
        }
        if(fc_y > 0)
        {
            fy = (mt_u32)((fc_y * r * 1023 / 1024) / fc_len + cy);
        }
        else
        {
            fy = (mt_u32)(cy - ((cy - fy) * r * 1023 / 1024 / fc_len));
        }
        fc_x = (int)fx - (int)cx;
        fc_y = (int)fy - (int)cy;
    }

    MT_ERR_TDE("\r\n c[%d,%d], f:[%d,%d]", cx, cy, fx, fy);
    r_r = r * r;

    for(y = 0; y < height; y ++)
    {
        dy = y - fy;
        dy_dy = dy * dy;
        dy_fcy = dy * fc_y;
        for(x = 0; x < width; x ++)
        {
            dx = x - fx;
            d1 = (mt_u32)(dx * dx) + dy_dy;
            tmp1 = dx * fc_y - dy * fc_x;
            tmp2 = dx * fc_x + dy_fcy;
            val = (mt_u64)r_r * (mt_u64)d1 - (mt_u64)tmp1 * (mt_u64)tmp1;
            d2 = (mt_u32)sqrt(val) - tmp2;
            if(d2 == 0)
                gradt = 0.0;
            else
                gradt = (double)d1 / d2;
            switch(p_ctx->paint.spread_mod)
            {
                case GPE_ARIA_SPREAD_PAD:
                    if(gradt > 1.0)
                      gradt = 1.0;
                    else if(gradt < 0.0)
                      gradt = 0.0;
                    break;
                case GPE_ARIA_SPREAD_REPEAT:
                    if((gradt > 1.0)  ||  (gradt < 0.0))
                      gradt = gradt - my_floor(gradt);
                    break;
                case GPE_ARIA_SPREAD_REFLECT:
                    if((gradt > 1.0)  ||  (gradt < 0.0))
                    {
                      if((my_floor(gradt) & 0x1) == 0)
                        gradt = gradt - my_floor(gradt);
                      else
                        gradt = 1.0 - (gradt - my_floor(gradt));
                    }
                    break;
                default:
                    break;
            }
            if(fabs(gradt - 1.0) <= EPSINON)
                p_ptr[y * pitch / 2 + x] = (1  <<  12) - 1;
            else
                p_ptr[y * pitch / 2 + x] = (mt_u16)(gradt * (1  <<  12));
        }
    }
    mt_mmz_unmap(p_ptr);

    //  hal_dcache_flush((void *)(p_ctx->p_gradt_buf), pitch * height);

    p_ctx->src_img_en = MT_TRUE;
    p_ctx->src_img.buf = p_ctx->p_gradt_buf;
    p_ctx->src_img.width = p_ctx->dst_img.rect.w;
    p_ctx->src_img.height = p_ctx->dst_img.rect.h;
    p_ctx->src_img.pitch = pitch;
    p_ctx->src_img.rect.w = p_ctx->src_img.width;
    p_ctx->src_img.rect.h = p_ctx->src_img.height;
    p_ctx->src_img.rect.x = 0;
    p_ctx->src_img.rect.y = 0;
    p_ctx->src_img.ck_en = MT_FALSE;
    p_ctx->src_img.plane_alpha_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_ch_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_pre_mult_en = MT_FALSE;

    p_ctx->src_img.color_info.bpp = GPE_ARIA_BPP_16BIT;
    p_ctx->src_img.color_info.color_fmt = GRAY_16;
    p_ctx->src_img.color_info.color_space = GRAY_COLOR_SPACE;
    p_ctx->src_img.color_info.is_pix_alpha = MT_FALSE;
    p_ctx->src_img.color_info.little_endian = MT_FALSE;
    p_ctx->src_img.with_palette = MT_FALSE;
    p_ctx->src_is_xylc = MT_FALSE;
    p_ctx->src_is_tile = MT_FALSE;

}
#ifdef CONFIG_MT_FPGA_GPE
static void spn_gpe_generate_ellipse_gradient_hw(aria_gpe_context_t *p_ctx)
{
    u32 width = p_ctx->dst_img.rect.w;
    u32 height = p_ctx->dst_img.rect.h;
    int cir_x = (int)p_ctx->paint.center.x;
    int cir_y = (int)p_ctx->paint.center.y;
    //int foc_x = (int)p_ctx->paint.focus.x;
    //int foc_y = (int)p_ctx->paint.focus.y;
    int mask_mod = (int)p_ctx->paint.mask_mod;
    int grant_cof, grant_cof1;
    int row, col;
    long long A = (long long)p_ctx->paint.long_a;
    long long B = (long long)p_ctx->paint.short_b;
    long long ELEA1,ELEA,ELEB,ELED,ELEE1,ELEC1;
    long long ELEC = 0,ELEE = 0;
    long long num_sqrt = 0,num_sqrt1 = 0,num_sqrt2 = 0;
    long long coef1 = 17592186044416; //44bit
    long long coef2 = 16777216; //24bit
    long long coef3 = 1048576; //20bit
    int  odd_flg,gradt_msk;

    u16 *p_ptr = NULL;
    u32 pitch = 0;
     pitch = width * 2;
  //  printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
    p_ctx->p_gradt_buf = mt_mmz_new(pitch * height, 8, NULL, MOD_NAME);
    
     if(p_ctx->p_gradt_buf == 0)
     {
         MT_ERR_TDE("\n\rgradt_buf malloc failed!");
         MT_ASSERT(0);
     }
#ifdef SUPPORT_CMDFIFO
    if(MT_SUCCESS != p_gradt_buf_backup(p_ctx->p_gradt_buf, p_gradt_buf_bak , BAK_BUFFER_NUM_MAX))
        printf("<%s> : <%d> Warning : cmdfifo mode will memory overlack\n", __FUNCTION__, __LINE__);
#endif
     p_ptr = (mt_u16 *)mt_mmz_map(p_ctx->p_gradt_buf, 0);

    ELEA1 = (long long) (1.0*(A*A*cir_y*cir_y + B*B*cir_x*cir_x)/(A*A*B*B)*coef2);
    ELEA  = ELEA1*coef3;
    ELEB  = (long long) (1.0*(2*cir_x+1)/(A*A)*coef1);
    ELED  = (long long) (1.0*(2*cir_y+1)/(B*B)*coef1);
    ELEC1 = (long long) (2.0/(A*A)*coef1);
    ELEE1 = (long long) (2.0/(B*B)*coef1);

    num_sqrt1 = ELEA;
    for( row = 0; row < height; row++)
    {  		
        for( col = 0; col < width; col++)
        {
            if(col == 0)
                num_sqrt = num_sqrt1;
            else
                num_sqrt = num_sqrt -ELEB + ELEC;

            num_sqrt2 = num_sqrt >> 20;
            //grant_cof2 = num_sqrt2;
            
            if(num_sqrt2 < 0)
                num_sqrt2 = 0;

            grant_cof = (int)(sqrt(num_sqrt2));

            //select mask mode depend on gradt value
            if(mask_mod == 1)
            {
                if(grant_cof > 4095)
                    gradt_msk = 1;
                else 
                    gradt_msk = 0;
            }
            else if(mask_mod == 2)
            {
                if(grant_cof > 4095)
                    gradt_msk = 0;
                else 
                    gradt_msk = 1;
            }
            else
                gradt_msk = 0;

            //clip gradient(x,y) according to different modes such as pad,repeat and reflect
            //gradient(x,y) is not in [0,1], take them to [0,1] for pad mode repeat mode and reflect mode
            switch(p_ctx->paint.spread_mod)
            {
                case GPE_ARIA_SPREAD_PAD:
                    if(grant_cof > 4095)
                        grant_cof = 4095;
                    break;
                case GPE_ARIA_SPREAD_REPEAT:
                    grant_cof = grant_cof & 0xfff;
                    break;
                case GPE_ARIA_SPREAD_REFLECT:
                    if(( grant_cof > 4095 ) || ( grant_cof < 0 ))
                    {
                        odd_flg = (grant_cof & 0x1000) >> 12;
                        if( odd_flg )
                            grant_cof = 4095 - (grant_cof & 0xfff);
                        else
                            grant_cof = (grant_cof & 0xfff);
                    }
                    break;
                default:
                    break;
            }

            grant_cof1 = grant_cof;
            p_ptr[row * pitch / 2 + col] =  (u16)(gradt_msk * 4096 + (grant_cof1 & 0xfff));     
            ELEC = ELEC + ELEC1;
        }
        ELEC = 0;
        ELEE = ELEE + ELEE1;
        num_sqrt1 = num_sqrt1 - ELED + ELEE;
    }
    mt_mmz_unmap(p_ptr);


    p_ctx->src_img_en = MT_TRUE;
    p_ctx->src_img.buf = p_ctx->p_gradt_buf;
    p_ctx->src_img.width = p_ctx->dst_img.rect.w;
    p_ctx->src_img.height = p_ctx->dst_img.rect.h;
    p_ctx->src_img.pitch = pitch;
    p_ctx->src_img.rect.w = p_ctx->src_img.width;
    p_ctx->src_img.rect.h = p_ctx->src_img.height;
    p_ctx->src_img.rect.x = 0;
    p_ctx->src_img.rect.y = 0;
    p_ctx->src_img.ck_en = MT_FALSE;
    p_ctx->src_img.plane_alpha_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_ch_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_pre_mult_en = MT_FALSE;
    p_ctx->src_img.color_info.bpp = GPE_ARIA_BPP_16BIT;
    p_ctx->src_img.color_info.color_fmt = GRAY_16;
    p_ctx->src_img.color_info.color_space = GRAY_COLOR_SPACE;
    p_ctx->src_img.color_info.is_pix_alpha = MT_FALSE;
    p_ctx->src_img.color_info.little_endian = MT_FALSE;
    p_ctx->src_img.with_palette = MT_FALSE;
    p_ctx->src_is_xylc = MT_FALSE;
    p_ctx->src_is_tile = MT_FALSE;
}
#endif

static void aria_gpe_generate_ellipse_gradient(aria_gpe_context_t *p_ctx)
{
    mt_u32 width = p_ctx->dst_img.rect.w;
    mt_u32 height = p_ctx->dst_img.rect.h;
    int cx = (int)p_ctx->paint.center.x;
    int cy = (int)p_ctx->paint.center.y;
    int foc_x = (int)p_ctx->paint.focus.x;
    int foc_y = (int)p_ctx->paint.focus.y;
    int A = (int)p_ctx->paint.long_a;
    int B = (int)p_ctx->paint.short_b;
    int beyond_flg;
    int foc_x0, foc_y0;
    float temp, part0, part1;
    float tmp0, tmp1, tmp2;
    float k0, b, h, a, c, p, m, n, x1, x2, y1, y2;
    float grant_cof_denominator, grant_cof_numerator;
    int fx,fy;
    int x = 0;
    int y = 0;
    int grant_cof_int;
    float gradt_cof = 0;
    int gradt_cof1;
    int mask_mod = (int)p_ctx->paint.mask_mod;
    int gradt_msk=0;
    float dx,dy;
    mt_u16 *p_ptr = NULL;
    mt_u32 pitch = 0;
    pitch = width * 2;
    p_ctx->p_gradt_buf = mt_mmz_new(pitch * height, 8, NULL, MOD_NAME);

    if(p_ctx->p_gradt_buf == 0)
    {
        MT_ERR_TDE("\n\rgradt_buf malloc failed!");
        MT_ASSERT(0);
    }
#ifdef SUPPORT_CMDFIFO
    if(MT_SUCCESS != p_gradt_buf_backup(p_ctx->p_gradt_buf, p_gradt_buf_bak , BAK_BUFFER_NUM_MAX))
         printf("<%s> : <%d> Warning : cmdfifo mode will memory overlack\n", __FUNCTION__, __LINE__);
#endif
//    MT_ERR_TDE("\r\n f:[%d,%d]", foc_x, foc_y);
    p_ptr = (mt_u16 *)mt_mmz_map(p_ctx->p_gradt_buf, 0);

    foc_x0 = foc_x;
    foc_y0 = foc_y;

    fx = ( foc_x - cx );
    fy = ( foc_y - cy );
    part0 = (float)(fx*fx)/(A*A);
    part1 = (float)(fy*fy)/(B*B);
    temp = part0 + part1;
    if(temp > 1)
        beyond_flg = 1;
    else
        beyond_flg = 0;

    if(beyond_flg) //the focus point is outsize the ellipse, clamp it
    {
        if( fx != 0 )
            k0 = (float)fy/fx;
        else
            k0 = 1;
        b = cy - k0*cx;
        h = ( 2*k0*b/(B*B) - 2*k0*cy/(B*B) - (float)2*cx/(A*A));
        a = ( (float)1/(A*A) + k0*k0/(B*B) );
        c = ( (float)cx*cx/(A*A) + (float)cy*cy/(B*B) + (float)b*b/(B*B) - (float)2*b*cy/(B*B) - 1 );

        p = ( h*h - 4*a*c ) ;
        m = ( -h )/( 2*a );
        n = sqrt(p)/( 2*a );

        if( fx == 0 )
        {
            x1 = foc_x;
            x2 = foc_x;
            y1 = cy + B;
            y2 = cy - B;
        }
        else
        {
            x1 = m + n;
            x2 = m - n;
            y1 = k0*x1 + b;
            y2 = k0*x2 + b;
        }

        int r_distance = (int)p_ctx->paint.stop_num;

        if( fx == 0 )
        {
            foc_x0 = foc_x;
            if( fy > 0 )
                foc_y0 =  y1 - r_distance;
            else
                foc_y0 =  y2 + r_distance;
        }
        else if( fx > 0 )
        {
            foc_x0 = x1- r_distance ;
            foc_y0 = k0*foc_x0 + b;
        }
        else
        {
            foc_x0 = x2 + r_distance ;
            foc_y0 = k0*foc_x0 + b;
        }

        fx  = ( foc_x0 - cx );
        fy = ( foc_y0 - cy );

    }

   // MT_ERR_TDE("\r\n c[%d,%d], f:[%d,%d]", cx, cy, foc_x0, foc_y0);

    for(y = 0; y < height; y ++)
    {
        for(x = 0; x < width; x ++)
        {
            dx = ( x - foc_x0 );
            dy = ( y - foc_y0 );
            grant_cof_numerator = ( B*B*dx*dx + A*A*dy*dy );

            tmp0 = ( dx*fy - dy*fx ) * ( dx*fy - dy*fx );
            tmp1 = ( B*B*dx*dx + A*A*dy*dy );
            tmp2 = sqrt(tmp1 - tmp0)*A*B;
            grant_cof_denominator =  tmp2 - ( B*B*dx*fx + A*A*dy*fy );

            if(fabs(tmp1) <= EPSINON)
                gradt_cof = 0.0;
            else
                gradt_cof = grant_cof_numerator/grant_cof_denominator;

            //select mask mode depend on gradt value
            if(mask_mod == 1)
            {
                if(gradt_cof > 1)
                    gradt_msk = 1;
                else
                    gradt_msk = 0;
            }
            else if(mask_mod == 2)
            {
                if(gradt_cof > 1)
                    gradt_msk = 0;
                else
                    gradt_msk = 1;
            }
            else
                gradt_msk = 0;

            //clip gradient(x,y) according to different modes such as pad,repeat and reflect
            //gradient(x,y) is not in [0,1], take them to [0,1] for pad mode repeat mode and reflect mode
            switch(p_ctx->paint.spread_mod)
            {
                case GPE_ARIA_SPREAD_PAD:
                    if(gradt_cof > 1.0)
                        gradt_cof = 1.0;
                    else if(gradt_cof < 0.0)
                        gradt_cof = 0.0;
                    break;
                case GPE_ARIA_SPREAD_REPEAT:
                    grant_cof_int = linx_int( gradt_cof );
                    if( gradt_cof > 1.0 )
                        gradt_cof = gradt_cof - grant_cof_int;
                    else if( gradt_cof < 0 )
                        gradt_cof = grant_cof_int + 1 + gradt_cof;
                    break;
                case GPE_ARIA_SPREAD_REFLECT:
                    grant_cof_int = linx_int( gradt_cof );
                    if( gradt_cof > 1.0 )
                    {
                        if( grant_cof_int%2 == 0 )
                            gradt_cof = gradt_cof - grant_cof_int;
                        else
                            gradt_cof = 1 - ( gradt_cof - grant_cof_int );
                    }
                    else if( gradt_cof < 0  )
                    {
                        gradt_cof = grant_cof_int + 1 + gradt_cof;
                        if( grant_cof_int%2 == 0 )
                            gradt_cof = 1 - gradt_cof;
                    }
                    break;
                default:
                    break;
            }

            gradt_cof1 = (int)(gradt_cof * 4095);
            p_ptr[y * pitch / 2 + x] =  (mt_u16)(gradt_msk * 4096 + (gradt_cof1 & 0xfff));
        }
    }
    mt_mmz_unmap((void *)p_ptr);

    //    hal_dcache_flush((void *)(p_ctx->p_gradt_buf), pitch * height);

    p_ctx->src_img_en = MT_TRUE;
    p_ctx->src_img.buf = (phys_addr_t)p_ctx->p_gradt_buf;
    p_ctx->src_img.width = p_ctx->dst_img.rect.w;
    p_ctx->src_img.height = p_ctx->dst_img.rect.h;
    p_ctx->src_img.pitch = pitch;
    p_ctx->src_img.rect.w = p_ctx->src_img.width;
    p_ctx->src_img.rect.h = p_ctx->src_img.height;
    p_ctx->src_img.rect.x = 0;
    p_ctx->src_img.rect.y = 0;
    p_ctx->src_img.ck_en = MT_FALSE;
    p_ctx->src_img.plane_alpha_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_ch_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_pre_mult_en = MT_FALSE;
    p_ctx->src_img.color_info.bpp = GPE_ARIA_BPP_16BIT;
    p_ctx->src_img.color_info.color_fmt = GRAY_16;
    p_ctx->src_img.color_info.color_space = GRAY_COLOR_SPACE;
    p_ctx->src_img.color_info.is_pix_alpha = MT_FALSE;
    p_ctx->src_img.color_info.little_endian = MT_FALSE;
    p_ctx->src_img.with_palette = MT_FALSE;
    p_ctx->src_is_xylc = MT_FALSE;
    p_ctx->src_is_tile = MT_FALSE;

}
#if 0
static void aria_gpe_generate_ellipse_gradient_hw(aria_gpe_context_t *p_ctx)
{
    mt_u32 width = p_ctx->dst_img.rect.w;
    mt_u32 height = p_ctx->dst_img.rect.h;
    int cir_x = (int)p_ctx->paint.center.x;
    int cir_y = (int)p_ctx->paint.center.y;
    int foc_x = (int)p_ctx->paint.focus.x;
    int foc_y = (int)p_ctx->paint.focus.y;
    int mask_mod = (int)p_ctx->paint.mask_mod;
    int grant_cof, grant_cof1;
    int row, col;
    long long A = (long long)p_ctx->paint.long_a;
    long long B = (long long)p_ctx->paint.short_b;
    long long ELEA1,ELEA,ELEB,ELED,ELEE1,ELEC1;
    long long ELEC = 0,ELEE = 0;
    long long num_sqrt = 0,num_sqrt1 = 0,num_sqrt2 = 0;
    long long coef1 = 17592186044416; //44bit
    long long coef2 = 16777216; //24bit
    long long coef3 = 1048576; //20bit
    int  odd_flg,gradt_msk;

    mt_u16 *p_ptr = NULL;
    mt_u32 pitch = 0;
    pitch = width * 2;
    p_ctx->p_gradt_buf = (mt_u16 *)mt_mmz_new(pitch * height, 8, NULL, MOD_NAME);

    if(p_ctx->p_gradt_buf == NULL)
    {
        MT_ERR_TDE("\n\rgradt_buf malloc failed!");
        MT_ASSERT(0);
    }
    MT_ERR_TDE("\r\n f:[%d,%d]", foc_x, foc_y);
    p_ptr = (mt_u16 *)mt_mmz_map((mt_u32)(p_ctx->p_gradt_buf), 0);

    ELEA1 = (long long) (1.0*(A*A*cir_y*cir_y + B*B*cir_x*cir_x)/(A*A*B*B)*coef2);
    ELEA  = ELEA1*coef3;
    ELEB  = (long long) (1.0*(2*cir_x+1)/(A*A)*coef1);
    ELED  = (long long) (1.0*(2*cir_y+1)/(B*B)*coef1);
    ELEC1 = (long long) (2.0/(A*A)*coef1);
    ELEE1 = (long long) (2.0/(B*B)*coef1);

    num_sqrt1 = ELEA;
    for( row = 0; row < height; row++)
    {
        for( col = 0; col < width; col++)
        {
            if(col == 0)
                num_sqrt = num_sqrt1;
            else
                num_sqrt = num_sqrt -ELEB + ELEC;

            num_sqrt2 = num_sqrt >> 20;
            if(num_sqrt2 < 0)
                num_sqrt2 = 0;

            grant_cof = (int)(sqrt(num_sqrt2));

            //select mask mode depend on gradt value
            if(mask_mod == 1)
            {
                if(grant_cof > 4095)
                    gradt_msk = 1;
                else
                    gradt_msk = 0;
            }
            else if(mask_mod == 2)
            {
                if(grant_cof > 4095)
                    gradt_msk = 0;
                else
                    gradt_msk = 1;
            }
            else
                gradt_msk = 0;

            //clip gradient(x,y) according to different modes such as pad,repeat and reflect
            //gradient(x,y) is not in [0,1], take them to [0,1] for pad mode repeat mode and reflect mode
            switch(p_ctx->paint.spread_mod)
            {
                case GPE_ARIA_SPREAD_PAD:
                    if(grant_cof > 4095)
                        grant_cof = 4095;
                    break;
                case GPE_ARIA_SPREAD_REPEAT:
                    grant_cof = grant_cof & 0xfff;
                    break;
                case GPE_ARIA_SPREAD_REFLECT:
                    if(( grant_cof > 4095 ) || ( grant_cof < 0 ))
                    {
                        odd_flg = (grant_cof & 0x1000) >> 12;
                        if( odd_flg )
                            grant_cof = 4095 - (grant_cof & 0xfff);
                        else
                            grant_cof = (grant_cof & 0xfff);
                    }
                    break;
                default:
                    break;
            }

            grant_cof1 = grant_cof;
            p_ptr[row * pitch / 2 + col] =  (mt_u16)(gradt_msk * 4096 + (grant_cof1 & 0xfff));
            ELEC = ELEC + ELEC1;
        }
        ELEC = 0;
        ELEE = ELEE + ELEE1;
        num_sqrt1 = num_sqrt1 - ELED + ELEE;
    }
    mt_mmz_unmap(p_ptr);

    //    hal_dcache_flush((void *)(p_ctx->p_gradt_buf), pitch * height);

    p_ctx->src_img_en = MT_TRUE;
    p_ctx->src_img.buf = (mt_u32)(p_ctx->p_gradt_buf);
    p_ctx->src_img.width = p_ctx->dst_img.rect.w;
    p_ctx->src_img.height = p_ctx->dst_img.rect.h;
    p_ctx->src_img.pitch = pitch;
    p_ctx->src_img.rect.w = p_ctx->src_img.width;
    p_ctx->src_img.rect.h = p_ctx->src_img.width;
    p_ctx->src_img.rect.x = 0;
    p_ctx->src_img.rect.y = 0;
    p_ctx->src_img.ck_en = MT_FALSE;
    p_ctx->src_img.plane_alpha_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_ch_en = MT_FALSE;
    p_ctx->src_img.color_info.alpha_pre_mult_en = MT_FALSE;
    p_ctx->src_img.color_info.bpp = GPE_ARIA_BPP_16BIT;
    p_ctx->src_img.color_info.color_fmt = GRAY_16;
    p_ctx->src_img.color_info.color_space = GRAY_COLOR_SPACE;
    p_ctx->src_img.color_info.is_pix_alpha = MT_FALSE;
    p_ctx->src_img.color_info.little_endian = MT_FALSE;
    p_ctx->src_img.with_palette = MT_FALSE;
    p_ctx->src_is_xylc = MT_FALSE;
    p_ctx->src_is_tile = MT_FALSE;

}
#endif
static void aria_gpe_calc_pattern(aria_gpe_context_t *p_ctx)
{
    mt_u32 beg_x = p_ctx->paint.pat_beg.x;
    mt_u32 end_x = p_ctx->dst_img.rect.w - p_ctx->paint.pat_beg.x;
    mt_u32 beg_y = p_ctx->paint.pat_beg.y;
    mt_u32 end_y = p_ctx->dst_img.rect.h - p_ctx->paint.pat_beg.y;
    mt_u32 src_w = p_ctx->src_img.rect.w;
    mt_u32 src_h = p_ctx->src_img.rect.h;

    p_ctx->paint.pat_remd1 = beg_x % src_w;
    p_ctx->paint.pat_quot1 = beg_x / src_w;
    p_ctx->paint.pat_remd2 = end_x % src_w;
    p_ctx->paint.pat_quot2 = end_x / src_w;
    p_ctx->paint.pat_remd3 = beg_y % src_h;
    p_ctx->paint.pat_quot3 = beg_y / src_h;
    p_ctx->paint.pat_remd4 = end_y % src_h;
    p_ctx->paint.pat_quot4 = end_y / src_h;
}

static MT_BOOL is_simple_rop(mt_u32 rop_mod)
{
    if((rop_mod == ROP_COPYPEN) || (rop_mod == ROP_BLACK)
            || (rop_mod == ROP_NOP) || (rop_mod == ROP_WHITE)
            || (rop_mod == ROP_PATCOPY))
        return MT_TRUE;
    else
        return MT_FALSE;
}

static void aria_yuv_to_rgb_enable_check(aria_gpe_context_t *p_context)
{
    MT_BOOL simple_rop = MT_FALSE;
    color_space_t src2_cs = RGB_COLOR_SPACE;

    if(p_context->dst_img.with_palette == MT_TRUE)
        return;

    simple_rop = is_simple_rop(p_context->rop_a_mod) && is_simple_rop(p_context->rop_c_mod);

    if(p_context->bg_img_en)
        src2_cs = p_context->bg_img.color_info.color_space;
    else
        src2_cs = p_context->dst_img.color_info.color_space;


    //trans to rgb before comp
    if(p_context->is_draw_multiply
            || p_context->is_draw_stencil
            || p_context->scale_en
            || p_context->blend_en
            || (p_context->rop_en && p_context->src1_sel && p_context->src_img_en && (!simple_rop)))
    {
        if(p_context->src3_sel && p_context->ex_img.color_info.color_space == YUV_COLOR_SPACE)
        {
            p_context->src3_yuv2rgb_en = MT_TRUE;
        }
        if((p_context->src_img_en) && (p_context->src_img.color_info.color_space == YUV_COLOR_SPACE))
        {
            p_context->src1_yuv2rgb_en = MT_TRUE;
        }
        if(p_context->src2_sel)
        {
            if(src2_cs == YUV_COLOR_SPACE)
            {
                p_context->src2_yuv2rgb_en = MT_TRUE;
            }
        }
        if(p_context->dst_img.color_info.color_space == YUV_COLOR_SPACE)
        {
            p_context->dst_rgb2yuv_en = MT_TRUE;
        }
    }
    //trans to color space of dst
    else
    {
        if((p_context->src_img_en) &&
                (p_context->src_img.color_info.color_space !=
                 p_context->dst_img.color_info.color_space))
        {
            if(p_context->src_img.color_info.color_space == YUV_COLOR_SPACE)
            {
                p_context->src1_yuv2rgb_en = MT_TRUE;
            }
            else if(p_context->src_img.color_info.color_space == RGB_COLOR_SPACE)
            {
                p_context->src1_rgb2yuv_en = MT_TRUE;
            }
        }
        else if((p_context->src_img_en == MT_FALSE) && (p_context->dst_img.color_info.color_space == YUV_COLOR_SPACE))
        {
            p_context->dst_rgb2yuv_en = MT_TRUE;
        }

        if(p_context->src2_sel && (src2_cs != p_context->dst_img.color_info.color_space))
        {
            if(src2_cs == YUV_COLOR_SPACE)
            {
                p_context->src2_yuv2rgb_en = MT_TRUE;
            }
            else
            {
                p_context->src2_rgb2yuv_en = MT_TRUE;
            }
        }
    }
#if 0
    GPE_PRINT("\r\n src1_rgb2yuv_en:%d, \r\n src1_yuv2rgb_en:%d, \r\n src2_rgb2yuv_en:%d, \
            \r\n  src2_yuv2rgb_en:%d, \r\n src3_rgb2yuv_en:%d, \r\n src3_yuv2rgb_en:%d, \
            \r\n dst_rgb2yuv_en:%d, \r\n dst_yuv2rgb_en:%d",   p_context->src1_rgb2yuv_en,
            p_context->src1_yuv2rgb_en,
            p_context->src2_rgb2yuv_en,
            p_context->src2_yuv2rgb_en,
            p_context->src3_rgb2yuv_en,
            p_context->src3_yuv2rgb_en,
            p_context->dst_rgb2yuv_en,
            p_context->dst_yuv2rgb_en);
#endif
}

const char *reg_name[] = {
    "GPE_ARIA_GRA_ENG_START"  ,
    "GPE_ARIA_GRA_EN       "  ,
    "GPE_ARIA_GRA_ENG_CFG      "  ,
    "GPE_ARIA_GRA_CORE_DONE"  ,
    "GPE_ARIA_SRC0_FMT_CFG0"  ,
    "GPE_ARIA_SRC0_FMT_CFG1"  ,
    "GPE_ARIA_SRC0_PIC_ADDR"  ,
    "GPE_ARIA_SRC0_PIC_STRIDE",
    "GPE_ARIA_SRC1_FMT_CFG0  ",
    "GPE_ARIA_SRC1_FMT_CFG1  ",
    "GPE_ARIA_SRC1_KEY_MIN   ",
    "GPE_ARIA_SRC1_KEY_MAX   ",
    "GPE_ARIA_SRC1_PIC_ADDR  ",
    "GPE_ARIA_SRC1_PIC_STRIDE",
    "GPE_ARIA_SRC1_PIC_SIZE  ",
    "GPE_ARIA_SRC1_OP_SIZE   ",
    "GPE_ARIA_SRC1_OP_POS    ",
    "GPE_ARIA_SRC1_CMYK_CFG  ",
    "RESERVED"  ,
    "GPE_ARIA_SRC1_STATUS    ",
    "GPE_ARIA_SRC2_FMT_CFG0  ",
    "GPE_ARIA_SRC2_FMT_CFG1  ",
    "GPE_ARIA_SRC2_KEY_MIN   ",
    "GPE_ARIA_SRC2_KEY_MAX   ",
    "GPE_ARIA_SRC2_PIC_ADDR  ",
    "GPE_ARIA_SRC2_PIC_STRIDE",
    "RESERVED"  ,
    "RESERVED"  ,
    "GPE_ARIA_SRC2_OP_POS    ",
    "GPE_ARIA_SRC2_STATUS    ",
    "RESERVED"  ,
    "RESERVED"  ,
    "GPE_ARIA_SRC3_FMT_CFG0  ",
    "GPE_ARIA_SRC3_FMT_CFG1  ",
    "GPE_ARIA_SRC3_KEY_MIN   ",
    "GPE_ARIA_SRC3_KEY_MAX   ",
    "GPE_ARIA_SRC3_PIC_ADDR  ",
    "GPE_ARIA_SRC3_PIC_STRIDE",
    "RESERVED"  ,
    "RESERVED"  ,
    "GPE_ARIA_SRC3_OP_POS    ",
    "GPE_ARIA_SRC3_STATUS    ",
    "RESERVED"  ,
    "RESERVED"  ,
    "GPE_ARIA_DST0_FMT_CFG0  ",
    "GPE_ARIA_DST0_FMT_CFG1  ",
    "GPE_ARIA_DST0_PIC_ADDR  ",
    "GPE_ARIA_DST0_PIC_STRIDE",
    "GPE_ARIA_DST0_OP_POS"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "GPE_ARIA_DST1_FMT_CFG0  ",
    "GPE_ARIA_DST1_FMT_CFG1  ",
    "GPE_ARIA_DST1_PIC_ADDR  ",
    "GPE_ARIA_DST1_PIC_STRIDE",
    "GPE_ARIA_DST1_OP_POS   ",
    "GPE_ARIA_DST_PIC_SIZE    ",
    "GPE_ARIA_DST_OP_SIZE     ",
    "GPE_ARIA_DST_STATUS     ",
    "GPE_ARIA_CMD_FIFO_CTRL  ",
    "GPE_ARIA_CMD_FIFO_TRIG_CFG  "   ,
    "GPE_ARIA_CMD_FIFO_ADDR_SYNC "   ,
    "GPE_ARIA_CMD_FIFO_ADDR_ASYNC"   ,
    "GPE_ARIA_CMD_ID0"      ,
    "GPE_ARIA_CMD_ID1"      ,
    "RESERVED"      ,
    "GPE_ARIA_CMD_FIFO_STATUS    "   ,
    "GPE_ARIA_GRA_AXI_CTRL       "   ,
    "GPE_ARIA_GRA_AXI_ATATUS     "   ,
    "GPE_ARIA_GRA_AXI_CFG"      ,
    "RESERVED"      ,
    "GPE_ARIA_XYLC_CFG               ",
    "GPE_ARIA_XYLC_ERR           ",
    "GPE_ARIA_XYLC_STATUS            "       ,
    "RESERVED"      ,
    "GPE_ARIA_SCALER_CFG             "       ,
    "GPE_ARIA_SCALER_COEF_11     "       ,
    "GPE_ARIA_SCALER_COEF_21     "       ,
    "GPE_ARIA_SCALER_COEF_31     "       ,
    "GPE_ARIA_SCALER_COEF_22     "       ,
    "GPE_ARIA_SCALER_COEF_23     "       ,
    "GPE_ARIA_SCALER_INIT_PHASE  "       ,
    "RESERVED"      ,
    "GPE_ARIA_SCALER_STATUS      "       ,
    "GPE_ARIA_FAST_SCALER_INIT_PHASE"      ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "RESERVED"  ,
    "GPE_ARIA_ROT_PAT_CFG            "   ,
    "GPE_ARIA_PAT_COLOR              "   ,
    "GPE_ARIA_PAT_OFFSET_POS     ",
    "GPE_ARIA_PAT_RATIO_X_0      ",
    "GPE_ARIA_PAT_RATIO_X_1      ",
    "GPE_ARIA_PAT_RATIO_Y_0      ",
    "GPE_ARIA_PAT_RATIO_Y_1      ",
    "RESERVED"  ,
    "GPE_ARIA_GRADT_CFG          "           ,
    "GPE_ARIA_GRADT_X_STEP       "           ,
    "GPE_ARIA_GRADT_Y_STEP       "           ,
    "GPE_ARIA_GRADT_START_V  "           ,
    "GPE_ARIA_STOP0_ARGB         "           ,
    "GPE_ARIA_STOP1_ARGB         "           ,
    "GPE_ARIA_STOP2_ARGB     "           ,
    "GPE_ARIA_STOP3_ARGB         "           ,
    "GPE_ARIA_STOP_OFFSET    "           ,
    "GPE_ARIA_STOP0_FACT         "           ,
    "GPE_ARIA_STOP1_FACT         "           ,
    "GPE_ARIA_STOP2_FACT         "           ,
    "GPE_SPN_REGION_CFG"          ,     
    "GPE_SPN_REGION_START"          ,     
    "GPE_SPN_REGION_STOP"          ,  
    "RESERVED"          ,
    "GPE_ARIA_COMP_CFG               "           ,
    "GPE_ARIA_COMP_MULT_MOD  "           ,
    "GPE_SPN_COMP_MULT_MOD2"          , 
    "GPE_ARIA_COMP_BLD_MOD   "           ,
    "GPE_ARIA_ROP_ID             "           ,
    "GPE_ARIA_ROP_PAT            "           ,
    "GPE_ARIA_COMP_STATUS    "           ,
    "GPE_SPN_COMP_COLORIZE"          ,  
    "GPE_ARIA_LOAD_EN        "           ,
    "GPE_ARIA_PAL_SIZE       "           ,
    "GPE_ARIA_PAL1_ADDR          ",
    "GPE_ARIA_PAL3_ADDR          ",
    "GPE_ARIA_COEF_ADDR      ",
    "GPE_ARIA_CTRL_STATUS    ",
};


void aria_gpe_print_reg(void)
{
    mt_u32 reg;
    GPE_REG_PRINT("\n");
    for(reg = GPE_ARIA_BASE; reg <= GPE_ARIA_CTRL_STATUS; reg+=4)
    {
        GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, %s",  reg, gpe_read_register(reg), reg_name[(reg - GPE_ARIA_BASE) / 4]);
    }
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_TILE_CFG", GPE_ARIA_SRC1_TILE_CFG, gpe_read_register(GPE_ARIA_SRC1_TILE_CFG));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_TILE_JMP00", GPE_ARIA_SRC1_TILE_JMP00, gpe_read_register(GPE_ARIA_SRC1_TILE_JMP00));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_TILE_JMP01", GPE_ARIA_SRC1_TILE_JMP01, gpe_read_register(GPE_ARIA_SRC1_TILE_JMP01));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_TILE_JMP10", GPE_ARIA_SRC1_TILE_JMP10, gpe_read_register(GPE_ARIA_SRC1_TILE_JMP10));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_TILE_JMP11", GPE_ARIA_SRC1_TILE_JMP11, gpe_read_register(GPE_ARIA_SRC1_TILE_JMP11));

    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_INT_EN", GPE_ARIA_GRA_INT_EN, gpe_read_register(GPE_ARIA_GRA_INT_EN));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_INT_STATE", GPE_ARIA_GRA_INT_STATE, gpe_read_register(GPE_ARIA_GRA_INT_STATE));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_STATE", GPE_ARIA_GRA_STATE, gpe_read_register(GPE_ARIA_GRA_STATE));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_INT_MOD", GPE_ARIA_GRA_INT_MOD, gpe_read_register(GPE_ARIA_GRA_INT_MOD));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_CLK_GATED", GPE_ARIA_GRA_CLK_GATED, gpe_read_register(GPE_ARIA_GRA_CLK_GATED));

    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCP_0", GPE_ARIA_GRP0_CSCP_0, gpe_read_register(GPE_ARIA_GRP0_CSCP_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCP_1", GPE_ARIA_GRP0_CSCP_1, gpe_read_register(GPE_ARIA_GRP0_CSCP_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCP_2", GPE_ARIA_GRP0_CSCP_2, gpe_read_register(GPE_ARIA_GRP0_CSCP_2));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCP_3", GPE_ARIA_GRP0_CSCP_3, gpe_read_register(GPE_ARIA_GRP0_CSCP_3));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCP_4", GPE_ARIA_GRP0_CSCP_4, gpe_read_register(GPE_ARIA_GRP0_CSCP_4));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCDC_0", GPE_ARIA_GRP0_CSCDC_0, gpe_read_register(GPE_ARIA_GRP0_CSCDC_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCDC_1", GPE_ARIA_GRP0_CSCDC_1, gpe_read_register(GPE_ARIA_GRP0_CSCDC_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP0_CSCDC_2", GPE_ARIA_GRP0_CSCDC_2, gpe_read_register(GPE_ARIA_GRP0_CSCDC_2));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCP_0", GPE_ARIA_GRP1_CSCP_0, gpe_read_register(GPE_ARIA_GRP1_CSCP_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCP_1", GPE_ARIA_GRP1_CSCP_1, gpe_read_register(GPE_ARIA_GRP1_CSCP_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCP_2", GPE_ARIA_GRP1_CSCP_2, gpe_read_register(GPE_ARIA_GRP1_CSCP_2));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCP_3", GPE_ARIA_GRP1_CSCP_3, gpe_read_register(GPE_ARIA_GRP1_CSCP_3));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCP_4", GPE_ARIA_GRP1_CSCP_4, gpe_read_register(GPE_ARIA_GRP1_CSCP_4));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCDC_0", GPE_ARIA_GRP1_CSCDC_0, gpe_read_register(GPE_ARIA_GRP1_CSCDC_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCDC_1", GPE_ARIA_GRP1_CSCDC_1, gpe_read_register(GPE_ARIA_GRP1_CSCDC_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRP1_CSCDC_2", GPE_ARIA_GRP1_CSCDC_2, gpe_read_register(GPE_ARIA_GRP1_CSCDC_2));

    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_A_0", GPE_ARIA_GRADT_RADIUS_A_0, gpe_read_register(GPE_ARIA_GRADT_RADIUS_A_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_A_1", GPE_ARIA_GRADT_RADIUS_A_1, gpe_read_register(GPE_ARIA_GRADT_RADIUS_A_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_B_0", GPE_ARIA_GRADT_RADIUS_B_0, gpe_read_register(GPE_ARIA_GRADT_RADIUS_B_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_B_1", GPE_ARIA_GRADT_RADIUS_B_1, gpe_read_register(GPE_ARIA_GRADT_RADIUS_B_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_C_0", GPE_ARIA_GRADT_RADIUS_C_0, gpe_read_register(GPE_ARIA_GRADT_RADIUS_C_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_C_1", GPE_ARIA_GRADT_RADIUS_C_1, gpe_read_register(GPE_ARIA_GRADT_RADIUS_C_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_D_0", GPE_ARIA_GRADT_RADIUS_D_0, gpe_read_register(GPE_ARIA_GRADT_RADIUS_D_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_D_1", GPE_ARIA_GRADT_RADIUS_D_1, gpe_read_register(GPE_ARIA_GRADT_RADIUS_D_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_E_0", GPE_ARIA_GRADT_RADIUS_E_0, gpe_read_register(GPE_ARIA_GRADT_RADIUS_E_0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRADT_RADIUS_E_1", GPE_ARIA_GRADT_RADIUS_E_1, gpe_read_register(GPE_ARIA_GRADT_RADIUS_E_1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_WCH_REQ_MSK_SYNC_MODE", GPE_ARIA_WCH_REQ_MSK_SYNC_MODE, gpe_read_register(GPE_ARIA_WCH_REQ_MSK_SYNC_MODE));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_PIN_SEL", GPE_ARIA_GRA_PIN_SEL, gpe_read_register(GPE_ARIA_GRA_PIN_SEL));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_MAC_SET", GPE_ARIA_GRA_MAC_SET, gpe_read_register(GPE_ARIA_GRA_MAC_SET));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_REQ_CFG", GPE_ARIA_GRA_REQ_CFG, gpe_read_register(GPE_ARIA_GRA_REQ_CFG));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_GRA_REQ_SEL", GPE_ARIA_GRA_REQ_SEL, gpe_read_register(GPE_ARIA_GRA_REQ_SEL));    

    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_PFM_MODE", GPE_ARIA_PFM_MODE, gpe_read_register(GPE_ARIA_PFM_MODE));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC0_PIX_STATUS0", GPE_ARIA_SRC0_PIX_STATUS0, gpe_read_register(GPE_ARIA_SRC0_PIX_STATUS0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC0_PIX_STATUS1", GPE_ARIA_SRC0_PIX_STATUS1, gpe_read_register(GPE_ARIA_SRC0_PIX_STATUS1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_PIX_STATUS0", GPE_ARIA_SRC1_PIX_STATUS0, gpe_read_register(GPE_ARIA_SRC1_PIX_STATUS0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC1_PIX_STATUS1", GPE_ARIA_SRC1_PIX_STATUS1, gpe_read_register(GPE_ARIA_SRC1_PIX_STATUS1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC2_PIX_STATUS0", GPE_ARIA_SRC2_PIX_STATUS0, gpe_read_register(GPE_ARIA_SRC2_PIX_STATUS0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC2_PIX_STATUS1", GPE_ARIA_SRC2_PIX_STATUS1, gpe_read_register(GPE_ARIA_SRC2_PIX_STATUS1));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC3_PIX_STATUS0", GPE_ARIA_SRC3_PIX_STATUS0, gpe_read_register(GPE_ARIA_SRC3_PIX_STATUS0));
    GPE_REG_PRINT("\r\n [0x%08x]:0x%08x, GPE_ARIA_SRC3_PIX_STATUS1", GPE_ARIA_SRC3_PIX_STATUS1, gpe_read_register(GPE_ARIA_SRC3_PIX_STATUS1));
    GPE_REG_PRINT("\n");
}

void aria_gpe_init_reg(MT_BOOL hw_init)
{
    mt_u32 axi_status = 0;
    mt_u32 val = 0;
    
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    mt_u32 dtmp = 0;    
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
    /*close gate clock this is  GPE_ARIA_GRA_CLK_GATED for hardware issue*/
    mt_u32 gate_val = 0;
    gate_val = gpe_read_register(GPE_ARIA_GRA_CLK_GATED);
    gate_val |= 0xf;
    gpe_write_register(GPE_ARIA_GRA_CLK_GATED, gate_val);
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    mt_u32 gate_val = 0;
    gate_val = gpe_read_register(GPE_ARIA_GRA_CLK_GATED);
    gate_val &= ~(0xf);
    gpe_write_register(GPE_ARIA_GRA_CLK_GATED, gate_val);
#endif

    if(hw_init == MT_FALSE)
    {
        gpe_write_register(GPE_ARIA_GRA_ENG_START, 0x100); //clear
#ifdef CONFIG_MT_FPGA_GPE
        if(g_debug.wr_last_mod == TRUE)
        {
            val = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
            val &= 0xEFFFFFFF;
            val |= 0x10000000;
            gpe_write_register(GPE_ARIA_GRA_REQ_CFG, val); //AXI_WR_LAST_MOD set to 1
        }
#endif
    }
    else
    {
        MT_INFO_TDE("\nGPE RESET\n");
        int i = 0;
        for(i=0; i<5; i++)
        {
            val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
            val &= 0xFFFFFFFC;
            val |= 0x3;
            gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //stop on
            axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);

            if((axi_status & 0x1) != 1) //AXI_BUS_IDLE
            {
                MT_USLEEP(1000);
                GPE_PRINT("GPE_ARIA_GRA_AXI_CTRL set 0x3 failed \n");
            }
            else
            {
                break;
            }
        }
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        //        mt_sys_read_register(0xbf50a10c, &val);
        val = mpi_read_reg32((void *)sys_reg_virt_addr);
        //axi
        val &= ~0x2;
        //        mt_sys_write_register(0xbf50a10c, val);
        mpi_write_reg32((void *)sys_reg_virt_addr, val);
        //ahb
        val &= ~0x1;
        //        mt_sys_write_register(0xbf50a10c, val);
        mpi_write_reg32((void *)sys_reg_virt_addr, val);
        //core
        val &= ~0x4;
        //       mt_sys_write_register(0xbf50a10c, val);
        mpi_write_reg32((void *)sys_reg_virt_addr, val);

        //MT_USLEEP(1000);    //   delete for 12451

        /*cancle reset */


        //core
        val |= 0x4;
        //        mt_sys_write_register(0xbf50a10c, val);
        mpi_write_reg32((void *)sys_reg_virt_addr, val);
        //ahb
        val |= 0x1;
        //        mt_sys_write_register(0xbf50a10c, val);
        mpi_write_reg32((void *)sys_reg_virt_addr, val);
        //axi
        val |= 0x2;
        //        mt_sys_write_register(0xbf50a10c, val);
        mpi_write_reg32((void *)sys_reg_virt_addr, val);
        gpe_write_register(GPE_ARIA_GRA_CLK_GATED, gate_val);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
        val = (1 << 22);
        dtmp = mpi_read_reg32((mt_u32)sys_reg_virt_addr);
        /*  set AXI reset cmd */
        dtmp &= ~(val);
        //mt_sys_write_register(0x1f510014, dtmp);
        mpi_write_reg32((mt_u32)sys_reg_virt_addr, dtmp);

        val = (1 << 21);
        /*  set AHB reset cmd */
        dtmp &= ~(val);
        mpi_write_reg32((void*)sys_reg_virt_addr, dtmp);

        val = (1 << 20);
        /*  set CORE reset cmd */
        dtmp &= ~(val);
        mpi_write_reg32((void*)sys_reg_virt_addr, dtmp);

//        mtos_task_delay_ms(1);
        /*******************************************/
        /*  cancel CORE reset */
        val = (1 << 20);
        dtmp = mpi_read_reg32((void*)sys_reg_virt_addr);
        dtmp |= val;
        mpi_write_reg32((void*)sys_reg_virt_addr, dtmp);

        /*  cancel AHB reset */
        val = (1 << 21);
        dtmp |= val;
        mpi_write_reg32((void*)sys_reg_virt_addr, dtmp);

        /*  cancel AXI reset */
        val = (1 << 22);
        dtmp |= val;
        mpi_write_reg32((void*)sys_reg_virt_addr, dtmp);
#endif
    }
    return;
}

#ifdef SUPPORT_CMDFIFO
mt_s32 spn_cmdfifo_create_list_begin(TDE_HANDLE *p_cf_hdl, cmdfifo_cfg_t *p_cmdfifo_cfg)
{
    cmdfifo_hdl_t *p_cmdfifo_hdl = malloc(sizeof(cmdfifo_hdl_t));
    if(p_cmdfifo_hdl == NULL)
    {
        GPE_PRINT("\r\n malloc cmdfifo handle failed");
        *p_cf_hdl = (TDE_HANDLE)NULL;
        return MT_FAILURE;
    }
    memset(p_gradt_buf_bak, 0, sizeof(p_gradt_buf_bak));
    memset(p_cmdfifo_hdl, 0, sizeof(cmdfifo_hdl_t));
    memcpy(&(p_cmdfifo_hdl->cmdfifo_cfg), p_cmdfifo_cfg, sizeof(cmdfifo_cfg_t));
    *p_cf_hdl = (TDE_HANDLE)p_cmdfifo_hdl;
    g_create_cmdfifo_flag = MT_TRUE;
    g_cmdfifo_hdl = p_cmdfifo_hdl;
    return MT_SUCCESS;
}


static cmdfifo_node_t *get_tail_node(cmdfifo_node_t *pp_node_list)
{
    cmdfifo_node_t *p_cur_node = NULL;
    p_cur_node = pp_node_list;
    if (p_cur_node == NULL)
        return p_cur_node;

    while(NULL != p_cur_node->p_next)
    {
        p_cur_node = p_cur_node->p_next;
    }
    return p_cur_node;
}


static cmdfifo_node_t *cmdfifo_create_node(MT_BOOL need_suspend)
{
    cmdfifo_node_t *p_new_node = NULL;
    cmdfifo_node_t *p_tail_node = NULL;
    mt_s32 ret = 0;
    p_new_node = (cmdfifo_node_t *)malloc(sizeof(cmdfifo_node_t));
    if(p_new_node == NULL)
    {
        GPE_PRINT("\r\n malloc new node failed");
        return NULL;
    }

    ret = get_cmdfifo_buf(&p_new_node->p_node_addr_vir, &p_new_node->p_node_addr_phy);
    if(ret)
    {
        GPE_PRINT("\r\n malloc new node addr failed");
        free(p_new_node);
        p_new_node = NULL;
        return NULL;
    }

    memset((void *)(p_new_node->p_node_addr_vir), 0, GPE_ARIA_MAX_CMDFIFO_SIZE * 8);
    p_new_node->node_size = 0;
    p_new_node->p_next = NULL;
    p_new_node->p_prev = NULL;
    p_new_node->need_suspend = need_suspend;

    if(g_cmdfifo_hdl->p_list == NULL) //head node
    {
        g_cmdfifo_hdl->p_list = (mt_u32 *)p_new_node;
    }
    else
    {
        p_tail_node = get_tail_node((cmdfifo_node_t *)g_cmdfifo_hdl->p_list);
        p_new_node->p_prev = p_tail_node;
        p_tail_node->p_next = p_new_node;
    }

    return p_new_node;
}

mt_s32 spn_cmdfifo_destroy(TDE_HANDLE p_cf_hdl)
{
    cmdfifo_node_t *p_cur_node = NULL;
    cmdfifo_node_t *p_prev_node = NULL;
    cmdfifo_node_t **p_list = NULL;
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    if (p_cf_hdl == (TDE_HANDLE)NULL)
        return MT_FAILURE;

    p_list = (cmdfifo_node_t **)(&(p_cmdfifo_hdl->p_list));
    p_cur_node = get_tail_node(*p_list);
    while(p_cur_node != NULL)
    {
        p_cur_node->p_node_addr_vir = (ulong)NULL;
        p_cur_node->p_node_addr_phy = (phys_addr_t)0;
        p_prev_node = p_cur_node->p_prev;
        free(p_cur_node);
        p_cur_node = p_prev_node;
    }
    *p_list = NULL;
    free(p_cmdfifo_hdl);
    g_cfId = 0;
    return MT_SUCCESS;
}

mt_s32 spn_cmdfifo_create_list_end(TDE_HANDLE p_cf_hdl)
{
    cmdfifo_node_t *p_cur_node = NULL;
    cmdfifo_node_t *p_list = NULL;
    mt_u32 *p_buf = NULL;
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    cmdfifo_attr_t *p_attr = &p_cmdfifo_hdl->cmdfifo_attr;
    if (p_cf_hdl == (TDE_HANDLE)NULL)
        return MT_FAILURE;

    p_list = (cmdfifo_node_t *)p_cmdfifo_hdl->p_list;
    p_cur_node = p_list;
    if(p_cur_node == NULL)
    {
        GPE_PRINT("\r\n empty cmdfifo list!");
        return MT_FAILURE;
    }

    p_attr->is_sync = p_cmdfifo_hdl->cmdfifo_cfg.is_sync;
    while(p_cur_node != NULL)
    {
#ifdef CONFIG_MT_FPGA_GPE
        p_attr->node_addr[p_attr->node_num] = p_cur_node->p_node_addr_vir;
        p_attr->dst_addr[p_attr->node_num] = p_cur_node->dst_addr;
        p_attr->dst_pitch[p_attr->node_num] = p_cur_node->dst_pitch;
        p_attr->dst_rect[p_attr->node_num].x = p_cur_node->dst_rect.x;
        p_attr->dst_rect[p_attr->node_num].y = p_cur_node->dst_rect.y;
        p_attr->dst_rect[p_attr->node_num].w = p_cur_node->dst_rect.w;
        p_attr->dst_rect[p_attr->node_num].h = p_cur_node->dst_rect.h;
#endif   
        p_attr->node_num++;

        MT_ASSERT(p_attr->node_num <= MAX_NODE_NUM);
        p_buf = (mt_u32 *)p_cur_node->p_node_addr_vir;
        if(p_cur_node->p_next == NULL)
        {
            if(p_cmdfifo_hdl->cmdfifo_cfg.is_round == MT_TRUE)
            {
                if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
                    p_buf[p_cur_node->node_size++] = 3 | (0x108 << 8);  //cmd end
                else
                    p_buf[p_cur_node->node_size++] = 3 | (0x10c << 8);
                p_buf[p_cur_node->node_size++] = p_list->p_node_addr_phy;
            }
            else
                p_buf[p_cur_node->node_size - 2] |= 1; //cmd end
        }
        else
        {
            if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
                p_buf[p_cur_node->node_size++] = 3 | (0x108 << 8); //cmd end, next code
            else
                p_buf[p_cur_node->node_size++] = 3 | (0x10c << 8); //cmd end, next code
            p_buf[p_cur_node->node_size++] = p_cur_node->p_next->p_node_addr_phy;
        }
        if(p_cur_node->need_suspend == MT_TRUE)
        {
            p_buf[p_cur_node->node_size - 2] |= (1 << 2); //suspend2
        }
  /*

        GPE_PRINT("\r\n p_cur_node->node_size:%d, addr:0x%lx,0x%lx", p_cur_node->node_size, (ulong)p_buf,
            (ulong)p_cur_node->p_node_addr_phy);

        
#ifdef GPE_DEBUG
        {
            mt_u32 i;
            for(i = 0; i < p_cur_node->node_size; i+= 2)
            {
                GPE_PRINT("\r\n [%d]0x%08x, 0x%08x",i,  (p_buf[i]>>8) + GPE_ARIA_BASE, p_buf[i + 1]);
            }
        }
#endif
*/
        p_cur_node = p_cur_node->p_next;
    }
    g_create_cmdfifo_flag = MT_FALSE;
    g_cmdfifo_hdl = NULL;
    return MT_SUCCESS;
}

#if 0
mt_s32 spn_cmdfifo_run(TDE_HANDLE p_cf_hdl)
{
    cmdfifo_node_t *p_list = NULL;
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    mt_s32 ret;

    p_list = (cmdfifo_node_t *)p_cmdfifo_hdl->p_list;
    if(p_list == NULL)
    {
        GPE_PRINT("\r\n p_list is null");
        return MT_FAILURE;
    }


    if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
    {
#ifdef CONFIG_MT_FPGA_GPE
        u32 data;
               gpe_write_register(GPE_ARIA_CMD_FIFO_ADDR_SYNC, p_list->p_node_addr_phy);  
               data = (p_cmdfifo_hdl->cmdfifo_cfg.sync_signal << 28) 
                   | (p_cmdfifo_hdl->cmdfifo_cfg.sync_num << 24) 
                   | (p_cmdfifo_hdl->cmdfifo_cfg.sync_delay_counter);
               gpe_write_register(GPE_ARIA_CMD_FIFO_TRIG_CFG, data);
               data = p_cmdfifo_hdl->cmdfifo_cfg.halt_mode << 5;
               gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
             //  gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 4));

#else           

        GPE_PRINT("\r\n not support sync cmdfifo yet");
        return MT_FAILURE;
#endif        
    }
    else
    {
        mt_u32 data;
        gpe_write_register(GPE_ARIA_CMD_FIFO_ADDR_ASYNC, p_list->p_node_addr_phy);
        data = (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_rst << 15)
            | (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_recovery << 14)
            | (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_mod << 13);
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        //        gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 6));
    }
#ifdef CONFIG_MT_FPGA_GPE  
    gpe_write_register(GPE_ARIA_GRA_INT_EN, 0xffffffff);
#else
    gpe_write_register(GPE_ARIA_GRA_INT_EN, CF_ASYNC_LIST_FINISH);
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 0);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 2);
#endif
#ifdef CONFIG_MT_FPGA_GPE  

	if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
         gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 4));
	else
        gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 6));
#endif
    ret = ioctl(get_tde_handle(), TDE_CF_WAIT_FINISH);
    if(ret == MT_FAILURE)
    {
        // time out, the HW is error, so need check all the register
        mt_u32 data = gpe_read_register(GPE_ARIA_GRA_STATE);
        MT_INFO_TDE("gpe aria timeout status = 0x%x\n", data);

        // use sys block reset
        aria_gpe_init_reg(MT_TRUE);
    }
#ifdef SUPPORT_CMDFIFO 
    p_gradt_buf_free(p_gradt_buf_bak, BAK_BUFFER_NUM_MAX);
#endif 
    return ret;
}
#endif

#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_run(TDE_HANDLE p_cf_hdl)
{
    cmdfifo_node_t *p_list = NULL;
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    mt_s32 ret;
    mt_u32 data = 0;

    p_list = (cmdfifo_node_t *)p_cmdfifo_hdl->p_list;
    if(p_list == NULL)
    {
        GPE_PRINT("\r\n p_list is null");
        return MT_FAILURE;
    }
    mt_sys_read_register(SYMPHONY_IO_PA(0xBF138140), &data);  //read auto start(auto clk)
    if(data &  (0x1 << 3))
    {
        mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &data);  //read auto start(auto clk)
        data |=  (0x1 << 3);  //enable gra auto start to enbale png IP clk start
        mt_sys_write_register(SYMPHONY_IO_PA(0xBF138148), data);        
        mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &data);  //read auto start(auto clk)
    }


    if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
    {
#ifdef CONFIG_MT_FPGA_GPE
        u32 data;
               gpe_write_register(GPE_ARIA_CMD_FIFO_ADDR_SYNC, p_list->p_node_addr_phy);  
               data = (p_cmdfifo_hdl->cmdfifo_cfg.sync_signal << 28) 
                   | (p_cmdfifo_hdl->cmdfifo_cfg.sync_num << 24) 
                   | (p_cmdfifo_hdl->cmdfifo_cfg.sync_delay_counter);
               gpe_write_register(GPE_ARIA_CMD_FIFO_TRIG_CFG, data);
               data = p_cmdfifo_hdl->cmdfifo_cfg.halt_mode << 5;
               gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
             //  gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 4));

#else           

        GPE_PRINT("\r\n not support sync cmdfifo yet");
        return MT_FAILURE;
#endif        
    }
    else
    {
        mt_u32 data;
        gpe_write_register(GPE_ARIA_CMD_FIFO_ADDR_ASYNC, p_list->p_node_addr_phy);
        data = (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_rst << 15)
            | (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_recovery << 14)
            | (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_mod << 13);
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        //        gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 6));
    }
#ifdef CONFIG_MT_FPGA_GPE  
    gpe_write_register(GPE_ARIA_GRA_INT_EN, 0xffffffff);
#else
    gpe_write_register(GPE_ARIA_GRA_INT_EN, CF_ASYNC_LIST_FINISH);
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 0);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 2);
#endif
#ifdef CONFIG_MT_FPGA_GPE  

	if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
         gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 4));
	else
        gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 6));
#endif
//#ifdef GPE_ARIA_HARDWARE_ISR_ON
    if(g_debug.int_mode_en)
    {
        ret = ioctl(get_tde_handle(), TDE_CF_WAIT_FINISH);
        if(ret == MT_FAILURE)
        {
            // time out, the HW is error, so need check all the register
            mt_u32 data = gpe_read_register(GPE_ARIA_GRA_STATE);
            MT_INFO_TDE("gpe aria timeout status = 0x%x\n", data);

            // use sys block reset
            aria_gpe_init_reg(MT_TRUE);
        }
    }
    else
    {    
//#else
        struct timeval t1, t2;
        mt_u32 gra_eng_status = 0;
#ifdef CONFIG_MT_FPGA_GPE
        mt_u32 axi_status = 0;
        mt_u32 pause_on = 0;
        mt_u32 reset_on = 0;
        mt_u32 val = 0;
#endif    
        // if no define GPE_WARROIRS_CHECK_IDLE_SYNC, not waite for the HW finish
        //  ticks = mt_ticks_get();
        gettimeofday(&t1, NULL);
        gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
        //if status equals GPE_HW_CLOSE_PROCESS, means finished
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gra_eng_status);
        while(GRA_ALL_DONE != (gra_eng_status & GRA_ALL_DONE))
        {
            gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
            
#ifdef CONFIG_MT_FPGA_GPE
              if(g_debug.pause_on == TRUE)
              { 
                if(pause_on == 0)
                {
                  pause_on = 1;
                  msleep(1);
                  GPE_PRINT("\nGPE_PAUSE on\n");
                  val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
                  val &= 0xFFFFFFCF;
                  val |= 0x30;
                  gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause on
                  axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
                  while(0x10 != (axi_status & 0x10))  //AXI_BUS_EMPTY
                  {
                    axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
                  }
                  msleep(1000);
                  val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
                  val &= 0xFFFFFFCF;
                  gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause off
                  GPE_PRINT("\nGPE_PAUSE off\n");
                }
              }
              if(g_debug.reset_on == TRUE)
              {
                if(reset_on == 0)
                {
                  reset_on = 1;
                  return FALSE;
                }
              }
#endif
            // cost too much time, so think the gra eng is error
            gettimeofday(&t2, NULL);
            //      if(10000 < (mt_ticks_get() - ticks))
            MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x,tm:%d ms", gpe_read_register(GPE_ARIA_GRA_STATE),(((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000));
            if(10000 < (((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000))
            {
                ret = MT_FALSE;

                break;
            }
            msleep(100);
        }
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_AXI_ATATUS:0x%08x", gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS));
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
        //clear gra done status
        gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
//#endif
    }
#ifdef SUPPORT_CMDFIFO 
    p_gradt_buf_free(p_gradt_buf_bak, BAK_BUFFER_NUM_MAX);
#endif 	
    return ret;
}
#else
mt_s32 spn_cmdfifo_run(TDE_HANDLE p_cf_hdl)
{
    cmdfifo_node_t *p_list = NULL;
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    mt_s32 ret;
    mt_u32 data = 0;

    p_list = (cmdfifo_node_t *)p_cmdfifo_hdl->p_list;
    if(p_list == NULL)
    {
        GPE_PRINT("\r\n p_list is null");
        return MT_FAILURE;
    }
    
    mt_sys_read_register(SYMPHONY_IO_PA(0xBF138140), &data);  //read auto start(auto clk)
    if(data &  (0x1 << 3))
    {
        mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &data);  //read auto start(auto clk)
        data |=  (0x1 << 3);  //enable gra auto start to enbale gra IP clk start
        mt_sys_write_register(SYMPHONY_IO_PA(0xBF138148), data);        
        mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &data);  //read auto start(auto clk)
    }

    aria_gpe_init_reg(MT_FALSE);

    if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
    {		   
    	mt_u32 data;
        gpe_write_register(GPE_ARIA_CMD_FIFO_ADDR_SYNC, p_list->p_node_addr_phy);
        data = (p_cmdfifo_hdl->cmdfifo_cfg.sync_signal << 28)
            | (p_cmdfifo_hdl->cmdfifo_cfg.sync_num << 24)
            | (p_cmdfifo_hdl->cmdfifo_cfg.sync_delay_counter );
		
        gpe_write_register(GPE_ARIA_CMD_FIFO_TRIG_CFG, data);
     	data = (p_cmdfifo_hdl->cmdfifo_cfg.halt_mode << 5 );
     	gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
     	gpe_write_register(GPE_ARIA_GRA_INT_EN, CF_SYNC_LIST_FINISH);
		
    }
    else
    {
        mt_u32 data;
        gpe_write_register(GPE_ARIA_CMD_FIFO_ADDR_ASYNC, p_list->p_node_addr_phy);
        data = (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_rst << 15)
            | (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_recovery << 14)
            | (p_cmdfifo_hdl->cmdfifo_cfg.async_conf_mod << 13);
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
//        gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 6));
  		gpe_write_register(GPE_ARIA_GRA_INT_EN, CF_ASYNC_LIST_FINISH);

    }
#ifdef CONFIG_MT_FPGA_GPE  
    gpe_write_register(GPE_ARIA_GRA_INT_EN, 0xffffffff);
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 0);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 2);
#endif
#ifdef CONFIG_MT_FPGA_GPE  

	if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
         gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 4));
	else
        gpe_write_register(GPE_ARIA_GRA_ENG_START, (1 << 6));
#endif
#ifdef GPE_ARIA_HARDWARE_ISR_ON
    if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync == MT_TRUE)
	{
		 ret = ioctl(get_tde_handle(), TDE_CF_WAIT_SYNC_FINISH);
	}
	else
	{
		ret = ioctl(get_tde_handle(), TDE_CF_WAIT_FINISH);
	}
    if(ret == MT_FAILURE)
    {
        // time out, the HW is error, so need check all the register
        data = gpe_read_register(GPE_ARIA_GRA_STATE);
        MT_INFO_TDE("gpe aria timeout status = 0x%x\n", data);

        // use sys block reset
        aria_gpe_init_reg(MT_TRUE);
    }
#else
    struct timeval t1, t2;
    mt_u32 gra_eng_status = 0;
#ifdef CONFIG_MT_FPGA_GPE
    mt_u32 axi_status = 0;
    mt_u32 pause_on = 0;
    mt_u32 reset_on = 0;
    mt_u32 val = 0;
#endif    
    // if no define GPE_WARROIRS_CHECK_IDLE_SYNC, not waite for the HW finish
    //  ticks = mt_ticks_get();
    gettimeofday(&t1, NULL);
    gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
    //if status equals GPE_HW_CLOSE_PROCESS, means finished
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gra_eng_status);
    while(GRA_ALL_DONE != (gra_eng_status & GRA_ALL_DONE))
    {
        gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
        
#ifdef CONFIG_MT_FPGA_GPE
          if(g_debug.pause_on == TRUE)
          { 
            if(pause_on == 0)
            {
              pause_on = 1;
              msleep(1);
              GPE_PRINT("\nGPE_PAUSE on\n");
              val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
              val &= 0xFFFFFFCF;
              val |= 0x30;
              gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause on
              axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
              while(0x10 != (axi_status & 0x10))  //AXI_BUS_EMPTY
              {
                axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
              }
              msleep(1000);
              val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
              val &= 0xFFFFFFCF;
              gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause off
              GPE_PRINT("\nGPE_PAUSE off\n");
            }
          }
          if(g_debug.reset_on == TRUE)
          {
            if(reset_on == 0)
            {
              reset_on = 1;
              return FALSE;
            }
          }
#endif
        // cost too much time, so think the gra eng is error
        gettimeofday(&t2, NULL);
        //      if(10000 < (mt_ticks_get() - ticks))
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x,tm:%d ms", gpe_read_register(GPE_ARIA_GRA_STATE),(((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000));
        if(10000 < (((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000))
        {
            ret = MT_FALSE;

            break;
        }
        msleep(100);
    }
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_AXI_ATATUS:0x%08x", gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS));
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
    //clear gra done status
    gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
#endif
#ifdef SUPPORT_CMDFIFO 
    p_gradt_buf_free(p_gradt_buf_bak, BAK_BUFFER_NUM_MAX);
#endif 
    return ret;
}
#endif


//#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_stop(ulong p_cf_hdl)
{
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    mt_u32 data = gpe_read_register(GPE_ARIA_CMD_FIFO_CTRL);
    GPE_PRINT("\r\n spn_cmdfifo_stop");
    if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync)
    {
        data |= (1<< 4);
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        while(1)
        {
            if((gpe_read_register(GPE_ARIA_GRA_STATE) & CF_SYNC_LIST_EXIT) == CF_SYNC_LIST_EXIT)
            {
                //            GPE_PRINT("\r\n --2e0:0x%08x,0x%08x,0x%08x,0x%08x", GET_REG(0xbf4302e0),  GET_REG(0xbf4302e4),  GET_REG(0xbf4302e8),  GET_REG(0xbf4302ec));
                data &= ~(1<< 4);
                gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
                //clear status
                gpe_write_register(GPE_ARIA_GRA_STATE, CF_SYNC_LIST_EXIT);
                GPE_PRINT("\r\n release sync exit status~~~~");
                break;
            }
        }
    }
    else
    {
        data |= (1<< 12);
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        while(1)
        {
            if((gpe_read_register(GPE_ARIA_GRA_STATE) & CF_ASYNC_LIST_EXIT) == CF_ASYNC_LIST_EXIT)
            {
                //           GPE_PRINT("\r\n --2e0:0x%08x,0x%08x,0x%08x,0x%08x", GET_REG(0xbf4302e0),  GET_REG(0xbf4302e4),  GET_REG(0xbf4302e8),  GET_REG(0xbf4302ec));
                data &= ~(1<< 12);
                gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
                //clear status
                gpe_write_register(GPE_ARIA_GRA_STATE, CF_ASYNC_LIST_EXIT);
                GPE_PRINT("\r\n release async exit status~~~~~");
                break;
            }
        }
    }
    return MT_SUCCESS;
}
//#endif


#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_cpu_suspend(ulong p_cf_hdl, MT_BOOL suspend_immediate)
{
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    mt_u32 data = gpe_read_register(GPE_ARIA_CMD_FIFO_CTRL);
    if(suspend_immediate)
    {
        data |= 1;
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
    }
    else
    {
        if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync)
        {
            data |= (1 << 1);
            gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        }
        else
        {
            data |= (1 << 9);
            gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        }
    }
    return MT_SUCCESS;
}
#endif

#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_cpu_unsuspend(ulong p_cf_hdl, MT_BOOL suspend_immediate)
{
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    mt_u32 data = gpe_read_register(GPE_ARIA_CMD_FIFO_CTRL);
    if(suspend_immediate)
    {
        data &= ~1;
        gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
    }
    else
    {
        if(p_cmdfifo_hdl->cmdfifo_cfg.is_sync)
        {
            data &= ~(1 << 1);
            gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        }
        else
        {
            data &= ~(1 << 9);
            gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
        }
    }
    return MT_SUCCESS;
}
#endif

#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_hang_req(mt_void)
{
    mt_u32 data = gpe_read_register(GPE_ARIA_CMD_FIFO_CTRL);
    data |= (1 << 31);
    gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
    return MT_SUCCESS;
}
#endif

#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_hang_release(mt_void)
{
    mt_u32 data = gpe_read_register(GPE_ARIA_CMD_FIFO_CTRL);
    data &= ~(1 << 31);
    gpe_write_register(GPE_ARIA_CMD_FIFO_CTRL, data);
    return MT_SUCCESS;
}
#endif

#ifdef CONFIG_MT_FPGA_GPE
mt_s32 spn_cmdfifo_get_info(ulong p_cf_hdl, cmdfifo_attr_t *p_attr)
{
    cmdfifo_hdl_t *p_cmdfifo_hdl = (cmdfifo_hdl_t *)p_cf_hdl;
    memcpy(p_attr, &p_cmdfifo_hdl->cmdfifo_attr, sizeof(cmdfifo_attr_t));
    return MT_SUCCESS;
}
#endif

mt_u32 spn_cmdfifo_get_node_cnt(mt_void)
{
    return g_cfId;
}

#endif

static MT_BOOL aria_gpe_set_parameter(TDE_HANDLE s32Handle, aria_param_t *p_param)
{
    aria_gpe_context_t *p_ctx = NULL;
    hdl_gpe_t *p_gpe = &g_hdl_gpe[s32Handle - 1];

    if((p_gpe == NULL) || (p_param == NULL))
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] p_gpe or p_param is NULL!\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
#ifndef CONFIG_TDE_PROC_DISABLE
    gpe_set_proc_info(p_param, 0);
#endif
    p_ctx = &(p_gpe->gpe_ctx);
    if(p_ctx == NULL)
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] p_gpe->p_priv is NULL!\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
    memset(p_ctx, 0x00, sizeof(aria_gpe_context_t));
#ifdef SUPPORT_CMDFIFO
    if(g_create_cmdfifo_flag == MT_TRUE)
    {
        p_ctx->is_cmdfifo = MT_TRUE;
        p_ctx->p_node = cmdfifo_create_node(MT_FALSE);
        if(p_ctx->p_node == NULL)
        {
            GPE_PRINT("\r\n p_node is null");
            return MT_FALSE;
        }
        if(p_ctx->p_node->p_node_addr_vir == (ulong)NULL)
        {
            GPE_PRINT("\r\n p_node_addr is null");
            return MT_FALSE;
        }
    }
#endif

    //src
    p_ctx->src_img_en = p_param->src_img_en;
    if(MT_TRUE == p_ctx->src_img_en)
    {
        if(!aria_gpe_set_format(p_param, SPN_SRC1, p_ctx))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] src format not supported!\n", __FUNCTION__, __LINE__);
            return MT_FALSE;
        }

#if 0
        aria_gpe_get_ck_min_max(&(p_ctx->src_img), p_param->src_img.ck_min, p_param->src_img.ck_max,
                p_param->src_img.pix_format);
#else
        p_ctx->src_img.ck_min = p_param->src_img.ck_min;
        p_ctx->src_img.ck_max = p_param->src_img.ck_max;
#endif
        if(!aria_gpe_set_img(&(p_ctx->src_img), &(p_param->src_img)))
        {
            MT_ERR_TDE("src1 aria_gpe_set_img failed!\n");
            return MT_FALSE;
        }
        if(p_ctx->src_img.width > 8191|| p_ctx->src_img.height > 8191)
        {
            MT_ERR_TDE("src1 width or height larger than 8191 <%d %d>!\n", p_ctx->src_img.width, p_ctx->src_img.height);
            return MT_FALSE;
        }

        if((p_ctx->src_img.color_info.color_fmt == CMYK8888) || (p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM))
        {
                    //will modify later
#ifdef CONFIG_MT_FPGA_GPE
            p_ctx->cmyk_max = g_debug.cmyk_max;
#else
            p_ctx->cmyk_max = 0;
#endif
            if(p_ctx->cmyk_max == 0)
                p_ctx->cmyk_coef = 0x101;
            else
                p_ctx->cmyk_coef = (255 << 16) / (p_ctx->cmyk_max * p_ctx->cmyk_max);
        }
    }
#if 0
    if(p_ctx->src_img.ck_en)
    {
        GPE_PRINT("\r\n src, ck_min:0x%08x, ck_max:0x%08x, ck_mod:0x%08x, ck_select:%d",
                p_ctx->src_img.ck_min, p_ctx->src_img.ck_max, p_ctx->src_img.ck_mod, p_ctx->src_img.ck_select);
    }
#endif
    //dst
    if(!aria_gpe_set_format(p_param, SPN_DST, p_ctx))
    {
        MT_ERR_TDE("[%s][%d]: [ERROR] dst format not supported!\n", __FUNCTION__, __LINE__);
        return MT_FALSE;
    }
#if 0
    if(p_param->bg_img_en)
    {
        aria_gpe_get_ck_min_max(&(p_ctx->dst_img), p_param->dst_img.ck_min, p_param->dst_img.ck_max,
                p_param->bg_img.pix_format);
    }
    else
    {
        aria_gpe_get_ck_min_max(&(p_ctx->dst_img), p_param->dst_img.ck_min, p_param->dst_img.ck_max,
                p_param->dst_img.pix_format);
    }
#else
    p_ctx->dst_img.ck_min = p_param->dst_img.ck_min;
    p_ctx->dst_img.ck_max = p_param->dst_img.ck_max;
#endif
    if(!aria_gpe_set_img(&(p_ctx->dst_img), &(p_param->dst_img)))
    {
        MT_ERR_TDE("dst aria_gpe_set_img failed!\n");
        return MT_FALSE;
    }
    if(p_ctx->dst_img.width > 0x1fff || p_ctx->dst_img.height > 0x1fff)
    {
        MT_ERR_TDE("dst size larger than 0x1fff*0x1fff!\n");
        return MT_FALSE;
    }
#if 0
    if(p_ctx->dst_img.ck_en)
    {
        GPE_PRINT("\r\n dst, ck_min:0x%08x, ck_max:0x%08x, ck_mod:0x%08x, ck_select:%d",
                p_ctx->dst_img.ck_min, p_ctx->dst_img.ck_max, p_ctx->dst_img.ck_mod, p_ctx->dst_img.ck_select);
    }
#endif
    //ex
    p_ctx->ex_img_en = p_param->ex_img_en;
    if(MT_TRUE == p_ctx->ex_img_en)
    {
        if(!aria_gpe_set_format(p_param, SPN_SRC3, p_ctx))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] ex format not supported!\n", __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
#if 0
        aria_gpe_get_ck_min_max(&(p_ctx->ex_img), p_param->ex_img.ck_min, p_param->ex_img.ck_max,
                p_param->ex_img.pix_format);
#else
        p_ctx->ex_img.ck_min = p_param->ex_img.ck_min;
        p_ctx->ex_img.ck_max = p_param->ex_img.ck_max;
#endif
        if(!aria_gpe_set_img(&(p_ctx->ex_img), &(p_param->ex_img)))
        {
            MT_ERR_TDE("ex aria_gpe_set_img failed!\n");
            return MT_FALSE;
        }
    }
#if 0
    if(p_ctx->ex_img.ck_en)
    {
        GPE_PRINT("\r\n ex, ck_min:0x%08x, ck_max:0x%08x, ck_mod:0x%08x, ck_select:%d",
                p_ctx->ex_img.ck_min, p_ctx->ex_img.ck_max, p_ctx->ex_img.ck_mod, p_ctx->ex_img.ck_select);
    }
#endif
    //bg
    p_ctx->bg_img_en = p_param->bg_img_en;
    if(MT_TRUE == p_ctx->bg_img_en)
    {
        if(!aria_gpe_set_format(p_param, SPN_SRC2, p_ctx))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] BG format not supported!\n", __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        if(!aria_gpe_set_img(&(p_ctx->bg_img), &(p_param->bg_img)))
        {
            MT_ERR_TDE("bg aria_gpe_set_img failed!\n");
            return MT_FALSE;
        }
    }
#if 0
    if(p_ctx->bg_img.ck_en)
    {
        GPE_PRINT("\r\n bg, ck_min:0x%08x, ck_max:0x%08x, ck_mod:0x%08x, ck_select:%d",
                p_ctx->bg_img.ck_min, p_ctx->bg_img.ck_max, p_ctx->bg_img.ck_mod, p_ctx->bg_img.ck_select);
    }
#endif
    //palette
    if(!aria_gpe_check_palette(p_ctx))
    {
        return MT_FALSE;
    }

    //paint
    if((p_param->gpe_op & GPE_OP_PAINT) == GPE_OP_PAINT)
    {
        p_ctx->paint_en = MT_TRUE;
        if(!aria_gpe_set_paint(&(p_ctx->paint), &(p_param->paint)))
        {
            return MT_FALSE;
        }
        p_ctx->paint_pattern_en = p_ctx->paint.is_pattern_paint;
        p_ctx->paint_gradt_en = !p_ctx->paint.is_pattern_paint;
        if(p_ctx->paint_pattern_en)
        {
            if(p_ctx->src_img_en == MT_FALSE)
                MT_ERR_TDE("[%s][%d]: [ERROR] no src image exist when pattern paint is enabled!\n",
                        __FUNCTION__, __LINE__);
            aria_gpe_calc_pattern(p_ctx);
        }
        else
        {
            //we need to generate the gradient image for radial gradient paint,
            //and the gradient image will be put in through src1 channel
            if(p_ctx->paint.gradt_type == GPE_ARIA_RADIAL_GRADT)
            {
                if((p_ctx->paint.center.x != p_ctx->paint.focus.x)
                        || (p_ctx->paint.center.y != p_ctx->paint.focus.y))
                    aria_gpe_generate_radial_gradient(p_ctx);
            }
            else if(p_ctx->paint.gradt_type == GPE_ARIA_ELLIPSE_GRADT)
            {
                if((p_ctx->paint.center.x != p_ctx->paint.focus.x) || (p_ctx->paint.center.y != p_ctx->paint.focus.y))
                {
                    aria_gpe_generate_ellipse_gradient(p_ctx);
                }
#ifdef CONFIG_MT_FPGA_GPE
                else  if((p_ctx->paint.center.x == p_ctx->paint.focus.x) 
                	&& (p_ctx->paint.center.y == p_ctx->paint.focus.y))
                {
                    if(g_debug.compare_with_sw == TRUE || g_debug.show_sw_result == TRUE)
                    {
                        spn_gpe_generate_ellipse_gradient_hw(p_ctx);
                     }

                }
#endif
            }
        }
    }

    //alpha map
    if((p_param->gpe_op & GPE_OP_ALPHAMAP) == GPE_OP_ALPHAMAP)
    {
        p_ctx->alpha_map_en = MT_TRUE;
        if((p_ctx->src_img_en == MT_FALSE) && (p_ctx->paint_en == MT_FALSE))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] no src image exist and no paint when alpha map!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        if(p_ctx->ex_img_en == MT_FALSE)
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] no ex image exist when alpha map!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        if(p_ctx->ex_img.color_info.color_fmt != GRAY_8)
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] ex image is not GRAY-8 when alpha map!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        p_ctx->alpha_map_mod = p_param->alpha_map_mod;
    }

    //draw image
    p_ctx->is_draw_multiply = ((p_param->gpe_op & GPE_OP_DMULT) == GPE_OP_DMULT);
    p_ctx->is_draw_stencil =  ((p_param->gpe_op & GPE_OP_DSTEN) == GPE_OP_DSTEN);
    if((MT_TRUE == p_ctx->is_draw_multiply)  ||  (MT_TRUE == p_ctx->is_draw_stencil))
    {
        if((p_ctx->src_img_en == MT_FALSE) && (p_ctx->paint_en == MT_FALSE))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] no src image exist and no paint when dmult or dsten!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        if(p_ctx->ex_img_en == MT_FALSE)
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] no ex image exist when dmult or dsten!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
    }

    //rotator
    p_ctx->rotator_en = ((p_param->gpe_op & GPE_OP_ROTATE) == GPE_OP_ROTATE);
    p_ctx->rotator_op = p_param->rotator_op;
    if(MT_TRUE == p_ctx->rotator_en)
    {
        if(p_ctx->src_img_en == MT_FALSE)
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] no src image exist when rotate!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
    }

    //    GPE_PRINT("\r\n gpe_op:0x%08x", p_param->gpe_op);
    //scaler
    if((p_param->gpe_op & GPE_OP_SCALE) == GPE_OP_SCALE)
    {
        p_ctx->p_scale_tab = &gpe_scale_coeff[0];
        p_ctx->scale_en = MT_TRUE;
        p_ctx->scale_cfg.scale_mod = p_param->scale_mod;
        p_ctx->scale_cfg.coef = p_param->coef;
        //p_ctx->scale_cfg.disable_color_anti_flicker = MT_TRUE;
        if(((p_param->gpe_op & GPE_OP_SCALE_TRAPZ) != GPE_OP_SCALE_TRAPZ)
                || (p_ctx->dst_img.with_palette == MT_TRUE))
        {
            p_ctx->scale_cfg.disable_alpha_anti_flicker = MT_TRUE;
            p_ctx->scale_cfg.disable_color_anti_flicker = MT_TRUE;
        }
        if(p_ctx->dst_img.with_palette == MT_TRUE)
        {
            p_ctx->scale_cfg.disable_alpha_filter = MT_TRUE;
            p_ctx->scale_cfg.disable_color_filter = MT_TRUE;
        }
        if(p_param->coef[5] == 1)
        {
            p_ctx->scale_cfg.init_phase = 0x10;
        }
        else
        {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
            if(1)
            {
                p_ctx->scale_cfg.init_phase_x = 16 + (p_ctx->src_img.rect.w * 16) / p_ctx->dst_img.rect.w;
                p_ctx->scale_cfg.init_phase_y = 16 + (p_ctx->src_img.rect.h * 16) / p_ctx->dst_img.rect.h;
            }
            else
#else
            if(p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
            {
                p_ctx->scale_cfg.init_phase_x = 16 + (p_ctx->src_img.rect.w * 16) / p_ctx->dst_img.rect.w;
                p_ctx->scale_cfg.init_phase_y = 16 + (p_ctx->src_img.rect.h * 16) / p_ctx->dst_img.rect.h;
            }
            else
#endif
#endif
            {
                if((p_param->gpe_op & GPE_OP_SCALE_HORI) == GPE_OP_SCALE_HORI)
                {
                    //p_ctx->scale_cfg.init_phase = 32 - ((p_ctx->dst_img.rect.w - 1) * p_ctx->src_img.rect.w * 256 /
                    //p_ctx->dst_img.rect.w - p_ctx->src_img.rect.w * 256 + 256) / 16;
                    p_ctx->scale_cfg.init_phase = 16 + (p_ctx->src_img.rect.w * 16) / p_ctx->dst_img.rect.w;
                    MT_INFO_TDE("\r\n %s,%d,init_phase:%d", __FUNCTION__, __LINE__,  p_ctx->scale_cfg.init_phase);
                }
                else if((p_param->gpe_op & GPE_OP_SCALE_VERT) == GPE_OP_SCALE_VERT)
                {
                    //      p_ctx->scale_cfg.init_phase = 32 - ((p_ctx->dst_img.rect.h - 1) * p_ctx->src_img.rect.h * 256 / p_ctx->dst_img.rect.h - p_ctx->src_img.rect.h * 256 + 256) / 16;
                    p_ctx->scale_cfg.init_phase = 16 + (p_ctx->src_img.rect.h * 16) / p_ctx->dst_img.rect.h;
                    MT_INFO_TDE("\r\n %s,%d,init_phase:%d,%d,%d", __FUNCTION__, __LINE__,  p_ctx->scale_cfg.init_phase, p_ctx->dst_img.rect.h,
                            p_ctx->src_img.rect.h);
                }
                else
                    p_ctx->scale_cfg.init_phase = 0x20;
            }
        }
#ifdef FAST_2D_SCALE
        if(p_param->coef[5] == 0)
        {
            if(p_ctx->dst_img.with_palette == MT_TRUE)
            {
                p_ctx->scale_cfg.disable_alpha_filter   = MT_TRUE;
                p_ctx->scale_cfg.disable_color_filter   = MT_TRUE;
                p_ctx->scale_cfg.disable_alpha_filter_h = MT_TRUE;
                p_ctx->scale_cfg.disable_color_filter_h = MT_TRUE;                
            }
            else
            {
#ifdef CONFIG_MT_FPGA_GPE            
                p_ctx->scale_cfg.disable_alpha_filter   = (g_debug.fast2d_flt_val & 0x01)?MT_TRUE:MT_FALSE;
                p_ctx->scale_cfg.disable_color_filter   = (g_debug.fast2d_flt_val & 0x02)?MT_TRUE:MT_FALSE;
                p_ctx->scale_cfg.disable_alpha_filter_h = (g_debug.fast2d_flt_val & 0x04)?MT_TRUE:MT_FALSE;
                p_ctx->scale_cfg.disable_color_filter_h = (g_debug.fast2d_flt_val & 0x08)?MT_TRUE:MT_FALSE;
#else
               p_ctx->scale_cfg.disable_alpha_filter   = MT_FALSE;
               p_ctx->scale_cfg.disable_color_filter   = MT_FALSE;
               p_ctx->scale_cfg.disable_alpha_filter_h = MT_FALSE;
               p_ctx->scale_cfg.disable_color_filter_h = MT_FALSE;   
#endif
                MT_INFO_TDE("\n\r[warning] : <%s> : <%d> p_ctx->scale_cfg need check\n", __FUNCTION__, __LINE__);
            }
        }
 
#endif  
        MT_INFO_TDE("\r\n scale enable!!!!!!!");
    }

    if((p_param->gpe_op & GPE_OP_BLUR) == GPE_OP_BLUR)
    {
        p_ctx->gaussian_blur_en = MT_TRUE;
        if((p_param->blur_tap > 55) || (p_param->blur_tap < 3) || ((p_param->blur_tap % 2) == 0))
        {
            MT_ERR_TDE("\r\n blur tap number should be in range [0~55], and must be odd value!!!!:%d", p_param->blur_tap);
            return MT_FALSE;
        }
        else
            p_ctx->blur_cfg.blur_tap = p_param->blur_tap;
    }

    if(p_ctx->scale_en && p_ctx->gaussian_blur_en)
    {
        MT_ERR_TDE("\r\n scale and gaussian blur can't be enabled at the same time!!!!");
        return MT_FALSE;
    }

    p_ctx->dst_with_mask = p_param->dst_with_mask;
    if(p_ctx->dst_with_mask)
    {
        p_ctx->dst_mask.mbuf_addr = p_param->dst_mask.mbuf_addr;
        p_ctx->dst_mask.mbuf_pitch = p_param->dst_mask.mbuf_pitch;
        p_ctx->dst_mask.mask2alpha = p_param->dst_mask.mask2alpha;
    }

    if(p_param->src_with_mask)
    {
        p_ctx->src_with_mask = MT_TRUE;
        p_ctx->src0_buf = p_param->src_mask_buf;
        p_ctx->src0_pitch = p_param->src_mask_pitch;
    }


    //blend
    if((p_param->gpe_op & GPE_OP_BLEND) == GPE_OP_BLEND)
    {
        p_ctx->blend_en =  MT_TRUE;
        if((p_ctx->src_img_en == MT_FALSE) && (p_ctx->paint_en == MT_FALSE))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] no src image exist and no paint when blend!\n",
                    __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        if(!aria_gpe_set_blend_mod(p_param->blend.src_blend_fact,
                    &(p_ctx->src_blend_fact),
                    p_ctx->is_draw_stencil))
        {
            MT_ERR_TDE("\n\rsrc blending fact error!");
            return MT_FALSE;
        }
        if(!aria_gpe_set_blend_mod(p_param->blend.dst_blend_fact,
                    &(p_ctx->dst_blend_fact),
                    p_ctx->is_draw_stencil))
        {
            MT_ERR_TDE("\n\rdst blending fact error!");
            return MT_FALSE;
        }
        if(!aria_gpe_set_blend_mod(p_param->blend_alpha.src_blend_fact,
                    &(p_ctx->asrc_blend_fact),
                    p_ctx->is_draw_stencil))
        {
            MT_ERR_TDE("\n\rsrcA blending fact error!");
            return MT_FALSE;
        }
        if(!aria_gpe_set_blend_mod(p_param->blend_alpha.dst_blend_fact,
                    &(p_ctx->adst_blend_fact),
                    p_ctx->is_draw_stencil))
        {
            MT_ERR_TDE("\n\rdstA blending fact error!");
            return MT_FALSE;
        }

        p_ctx->demultiply_en = p_param->blend.demultiply_en;    
    }

    //rop
    p_ctx->rop_ch.src_in = MT_FALSE;
    p_ctx->rop_ch.dst_in = MT_FALSE;
    if((p_param->gpe_op & GPE_OP_ROP) == GPE_OP_ROP)
    {
        aria_rop_ch_t rop_a_ch = {0};
        aria_rop_ch_t rop_c_ch = {0};
        p_ctx->rop_en = MT_TRUE;
        if(!aria_gpe_set_rop_mod(p_param->rop.rop_a_id, &(p_ctx->rop_a_mod), &rop_a_ch))
        {
            MT_ERR_TDE("\n\ralpha rop id error!");
            return MT_FALSE;
        }

        if(!aria_gpe_set_rop_mod(p_param->rop.rop_c_id, &(p_ctx->rop_c_mod), &rop_c_ch))
        {
            MT_ERR_TDE("\n\rcolor rop id error!");
            return MT_FALSE;
        }

        if((rop_a_ch.src_in)  ||  (rop_c_ch.src_in))
            p_ctx->rop_ch.src_in = MT_TRUE;
        if((rop_a_ch.dst_in)  ||  (rop_c_ch.dst_in))
            p_ctx->rop_ch.dst_in = MT_TRUE;

        p_ctx->rop_pattern = p_param->rop.rop_pattern;
    }

    //mask
    if((p_param->gpe_op & GPE_OP_MASK) == GPE_OP_MASK)
    {
        if((p_ctx->rop_en) || (p_ctx->blend_en))
        {
            MT_ERR_TDE("[%s][%d]: [ERROR] mask enabled with rop or blend\n", __FUNCTION__, __LINE__);
            return MT_FALSE;
        }
        switch(p_param->mask_mod)
        {
        case VG_CLEAR_MASK:
            p_ctx->rop_en = MT_TRUE;
            p_ctx->rop_c_mod = ROP_BLACK;
            p_ctx->rop_a_mod = ROP_BLACK;
            break;
        case VG_FILL_MASK:
            p_ctx->rop_en = MT_TRUE;
            p_ctx->rop_c_mod = ROP_WHITE;
            p_ctx->rop_a_mod = ROP_WHITE;
            break;
        case VG_SET_MASK:
            p_ctx->rop_en = MT_TRUE;
            p_ctx->rop_c_mod = ROP_COPYPEN;
            p_ctx->rop_a_mod = ROP_COPYPEN;
            p_ctx->rop_ch.src_in = MT_TRUE;
            break;
        case VG_UNION_MASK:
            p_ctx->blend_en = MT_TRUE;
            p_ctx->dst_blend_fact = GPE_ARIA_GL_ONE;
            p_ctx->src_blend_fact = GPE_ARIA_GL_ONE_MINUS_DST_COLOR;
            p_ctx->adst_blend_fact = p_ctx->dst_blend_fact;
            p_ctx->asrc_blend_fact = p_ctx->src_blend_fact;
            break;
        case VG_INTERSECT_MASK:
            p_ctx->blend_en = MT_TRUE;
            p_ctx->dst_blend_fact = GPE_ARIA_GL_SRC_COLOR;
            p_ctx->src_blend_fact = GPE_ARIA_GL_ZERO;
            p_ctx->adst_blend_fact = p_ctx->dst_blend_fact;
            p_ctx->asrc_blend_fact = p_ctx->src_blend_fact;
            break;
        case VG_SUBTRACT_MASK:
            p_ctx->blend_en = MT_TRUE;
            p_ctx->dst_blend_fact = GPE_ARIA_GL_ONE_MINUS_SRC_COLOR;
            p_ctx->src_blend_fact = GPE_ARIA_GL_ZERO;
            p_ctx->adst_blend_fact = p_ctx->dst_blend_fact;
            p_ctx->asrc_blend_fact = p_ctx->src_blend_fact;
            break;
        default:
            break;
        }
    }

    //we will disable the channel if it has nothing to do with the result,
    //even it is enabled by the user
    p_ctx->src1_sel = p_ctx->src_img_en;
    p_ctx->src2_sel = MT_TRUE;
    p_ctx->src3_sel = p_ctx->ex_img_en;
    //  p_ctx->region_mod = 0;
    if(p_ctx->rop_en)
    {
        if((p_ctx->rop_ch.src_in)  ||
                ((p_ctx->src_img_en == MT_TRUE)  &&  (p_ctx->src_img.ck_en == MT_TRUE)))
        {
            if((p_ctx->src_img_en == MT_FALSE) && (p_ctx->paint_en == MT_FALSE))
            {
                MT_ERR_TDE("[%s][%d]: [ERROR] no src image exist and no paint when rop which \
                        need src in!\n", __FUNCTION__, __LINE__);
                return MT_FALSE;
            }
            p_ctx->src1_sel = MT_TRUE;
        }
        else
        {
            p_ctx->src1_sel = MT_FALSE;
            //disable the operation done on src1
            p_ctx->scale_en = MT_FALSE;
            p_ctx->rotator_en = MT_FALSE;
            p_ctx->is_draw_multiply = MT_FALSE;
            p_ctx->is_draw_stencil = MT_FALSE;
            if(p_ctx->alpha_map_en  &&
                    (p_ctx->alpha_map_mod != GPE_CCT_ALPHA_MAP_LOGICAL))
            {
                p_ctx->alpha_map_en = MT_FALSE;
            }
            if(!(p_ctx->alpha_map_en  &&  (p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_LOGICAL)))
            {
                p_ctx->src3_sel = MT_FALSE;
            }
        }

        if(!(p_ctx->alpha_map_en  &&  (p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_LOGICAL)))
        {
            p_ctx->src2_sel = p_ctx->rop_ch.dst_in;
        }
    }

    if(p_ctx->blend_en)
    {
        p_ctx->src1_sel = MT_TRUE;
    }

    //if rotator enable, rop and blend can't be enable at the same time
    if(p_ctx->rotator_en)
    {
        p_ctx->rop_en = MT_FALSE;
        p_ctx->blend_en = MT_FALSE;
    }

    //compositor
    if((!p_ctx->rop_en)  &&  (!p_ctx->blend_en))
        p_ctx->comp_en = MT_FALSE;
    else
        p_ctx->comp_en = MT_TRUE;

    if(MT_TRUE == p_ctx->rotator_en)
    {
        p_ctx->src2_sel = MT_FALSE;
    }
    if(p_ctx->dst_img.ck_en == MT_TRUE)
    {
        p_ctx->src2_sel = MT_TRUE;
    }

    if(p_ctx->rop_en)
    {
        if((p_ctx->rop_a_mod == ROP_PATCOPY)
                && (p_ctx->rop_c_mod == ROP_PATCOPY)
                && (p_ctx->src1_sel == MT_FALSE)
                && (p_ctx->src2_sel == MT_FALSE)
                && (p_ctx->src3_sel == MT_FALSE))
            p_ctx->comp_bp = 0x8;
        else
            p_ctx->comp_bp = 0;
    }

    aria_yuv_to_rgb_enable_check(p_ctx);

    p_ctx->no_dithering = p_param->no_dithering;
    p_ctx->comp_key_set = p_param->comp_key_set;
    p_ctx->dst_ds_mod = p_param->dst_ds_mod;
    p_ctx->dst_ds_choose = p_param->dst_ds_choose;
    p_ctx->clip_en = p_param->clip_en;

#ifdef CONFIG_MT_FPGA_GPE
#ifndef FAST_2D_SCALE
    if(g_debug.ds_mod == 0)
    {
        p_ctx->dst_ds_mod = 1;
        p_ctx->dst_ds_choose = 1;
    }
    else if(g_debug.ds_mod == 1)
    {
        p_ctx->dst_ds_mod = 1;
        p_ctx->dst_ds_choose = 0;
    }
    else if(g_debug.ds_mod == 2)
    {
        p_ctx->dst_ds_mod = 0;
        p_ctx->dst_ds_choose = 1;
    }
    else if(g_debug.ds_mod == 3)
    {
        p_ctx->dst_ds_mod = 0;
        p_ctx->dst_ds_choose = 0;
    }
#endif    
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(p_ctx->clip_en)
    {
        p_ctx->clip.clip_mode = p_param->clip.clip_mode;
        p_ctx->clip.clip_rect.s32Xpos = p_param->clip.clip_rect.s32Xpos;
        p_ctx->clip.clip_rect.s32Ypos = p_param->clip.clip_rect.s32Ypos;
        if(p_param->clip.clip_rect.s32Xpos + p_param->clip.clip_rect.u32Width> p_ctx->dst_img.width)
            p_ctx->clip.clip_rect.u32Width = p_ctx->dst_img.width - p_param->clip.clip_rect.s32Xpos;
        else
            p_ctx->clip.clip_rect.u32Width = p_param->clip.clip_rect.u32Width;
        if(p_param->clip.clip_rect.s32Ypos + p_param->clip.clip_rect.s32Ypos > p_ctx->dst_img.height)
            p_ctx->clip.clip_rect.u32Height= p_ctx->dst_img.height - p_param->clip.clip_rect.s32Ypos;
        else    
            p_ctx->clip.clip_rect.u32Height = p_param->clip.clip_rect.u32Height;
    }
#endif
    p_ctx->colorize_en = p_param->colorize_en;
    p_ctx->color = p_param->color;

#ifdef CONFIG_MT_FPGA_GPE
    if(g_debug.use_new_expmode == TRUE)
        p_ctx->color_exp_mode = EXP_PAD_LOW_BIT;
    else
        p_ctx->color_exp_mode = EXP_PAD_OLD_MODE;    
#else
#ifdef OLD_EXP_MODE   //consistent with concerto
    p_ctx->color_exp_mode = EXP_PAD_OLD_MODE;
#else
    p_ctx->color_exp_mode = EXP_PAD_LOW_BIT;
#endif
#endif

    //global check
    if(!aria_gpe_global_check(p_ctx))
        return MT_FALSE;

    return MT_TRUE;
}

static void aria_gpe_print_context(TDE_HANDLE s32Handle)
{
    aria_gpe_context_t *p_ctx = NULL;
    hdl_gpe_t *p_gpe = &g_hdl_gpe[s32Handle - 1];

    MT_ASSERT(NULL != p_gpe);

    p_ctx = &(p_gpe->gpe_ctx);
    MT_ASSERT(NULL != p_ctx);

    GPE_PRINT("\r\np_ctx->src_img_en:%d",p_ctx->src_img_en);
    GPE_PRINT("\r\np_ctx->ex_img_en:%d",p_ctx->ex_img_en);
    GPE_PRINT("\r\np_ctx->bg_img_en:%d",p_ctx->bg_img_en);
    GPE_PRINT("\r\np_ctx->src1_sel:%d",p_ctx->src1_sel);
    GPE_PRINT("\r\np_ctx->src2_sel:%d",p_ctx->src2_sel);
    GPE_PRINT("\r\np_ctx->src3_sel:%d",p_ctx->src3_sel);

    GPE_PRINT("\r\np_ctx->src_img.buf:0x%lx",(ulong)p_ctx->src_img.buf);
    GPE_PRINT("\r\np_ctx->dst_img.buf:0x%lx",(ulong)p_ctx->dst_img.buf);
    GPE_PRINT("\r\np_ctx->ex_img.buf:0x%lx",(ulong)p_ctx->ex_img.buf);
    GPE_PRINT("\r\np_ctx->bg_img.buf:0x%lx",(ulong)p_ctx->bg_img.buf);
 
    GPE_PRINT("\r\nsrc_format:%d, dst_format:%d, ex_format:%d",
            p_ctx->src_img.color_info.color_fmt, p_ctx->dst_img.color_info.color_fmt,
            p_ctx->ex_img.color_info.color_fmt);
    GPE_PRINT("\r\nsrc_pitch:%d, dst_picth:%d, ex_pitch:%d",
            p_ctx->src_img.pitch, p_ctx->dst_img.pitch, p_ctx->ex_img.pitch);
    GPE_PRINT("\r\np_ctx->src_img.width:%d, p_ctx->src_img.height:%d",
            p_ctx->src_img.width, p_ctx->src_img.height);
    GPE_PRINT("\r\np_ctx->dst_img.width:%d, p_ctx->dst_img.height:%d",
            p_ctx->dst_img.width, p_ctx->dst_img.height);
    GPE_PRINT("\r\np_ctx->ex_img.width:%d, p_ctx->ex_img.height:%d",
            p_ctx->ex_img.width, p_ctx->ex_img.height);
    GPE_PRINT("\r\np_ctx->src_img.rect [%d,%d,%d,%d]",
            p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_ctx->src_img.rect.w, p_ctx->src_img.rect.h);
    GPE_PRINT("\r\np_ctx->dst_img.rect [%d,%d,%d,%d]",
            p_ctx->dst_img.rect.x, p_ctx->dst_img.rect.y, p_ctx->dst_img.rect.w, p_ctx->dst_img.rect.h);
    GPE_PRINT("\r\np_ctx->ex_img.rect [%d,%d,%d,%d]",
            p_ctx->ex_img.rect.x, p_ctx->ex_img.rect.y, p_ctx->ex_img.rect.w, p_ctx->ex_img.rect.h);
    GPE_PRINT("\r\np_ctx->bg_img.rect [%d,%d,%d,%d]",
            p_ctx->bg_img.rect.x, p_ctx->bg_img.rect.y, p_ctx->bg_img.rect.w, p_ctx->bg_img.rect.h);
    GPE_PRINT("\r\np_ctx->src_img.plane_alpha_en:0x%08x, p_ctx->src_img.plane_alpha:0x%08x",
            p_ctx->src_img.plane_alpha_en, p_ctx->src_img.plane_alpha);
    GPE_PRINT("\r\np_ctx->ex_img.plane_alpha_en:0x%08x, p_ctx->ex_img.plane_alpha:0x%08x",
            p_ctx->ex_img.plane_alpha_en, p_ctx->ex_img.plane_alpha);
    GPE_PRINT("\r\np_ctx->src_is_tile:%d",p_ctx->src_is_tile);
    GPE_PRINT("\r\np_ctx->src_is_xylc:%d",p_ctx->src_is_xylc);
    GPE_PRINT("\r\np_ctx->paint_en:%d",p_ctx->paint_en);
    GPE_PRINT("\r\np_ctx->paint_pattern_en:%d",p_ctx->paint_pattern_en);
    GPE_PRINT("\r\np_ctx->paint_gradt_en:%d",p_ctx->paint_gradt_en);
    GPE_PRINT("\r\np_ctx->alpha_map_en:%d",p_ctx->alpha_map_en);
    GPE_PRINT("\r\np_ctx->alpha_map_mod:%d",p_ctx->alpha_map_mod);
    GPE_PRINT("\r\np_ctx->blend_en:%d",p_ctx->blend_en);
    GPE_PRINT("\r\np_ctx->src_blend_fact:%d",p_ctx->src_blend_fact);
    GPE_PRINT("\r\np_ctx->dst_blend_fact:%d",p_ctx->dst_blend_fact);
    GPE_PRINT("\r\np_ctx->rop_en:%d",p_ctx->rop_en);
    GPE_PRINT("\r\np_ctx->rop_a_mod:%d",p_ctx->rop_a_mod);
    GPE_PRINT("\r\np_ctx->rop_c_mod:%d",p_ctx->rop_c_mod);
    GPE_PRINT("\r\np_ctx->rop_pattern:0x%08x",p_ctx->rop_pattern);
    GPE_PRINT("\r\np_ctx->scale_en:%d",p_ctx->scale_en);
    GPE_PRINT("\r\np_ctx->is_draw_stencil:%d",p_ctx->is_draw_stencil);
    GPE_PRINT("\r\np_ctx->is_draw_multiply:%d",p_ctx->is_draw_multiply);
    GPE_PRINT("\r\np_ctx->rotator_en:%d",p_ctx->rotator_en);
    GPE_PRINT("\r\np_ctx->rotator_op:%d",p_ctx->rotator_op);
}

static void aria_gpe_get_scale_coeff(rect_vsb_t *p_src ,
        pos_t *p_dst00,
        pos_t *p_dst10,
        pos_t *p_dst01,
        pos_t * p_dst11,
        mt_s32 *p_coeff,
        scale_type_t *scale_type)
{
    mt_s64 x1,x2,x3,x4,y1,y3,u1,u2,v1,v3;
    int tmp;
    mt_u32 para_id = 0;
    mt_s64 para11[3];
    mt_s64 para21[3];
    mt_s64 para22[3];
    mt_s64 para23[3];
    mt_s64 para31[3];

    if ((p_dst00->y == p_dst10->y) && (p_dst01->y == p_dst11->y))
    {
        tmp = (int)((p_dst00->x > p_dst01->x) ? p_dst01->x : p_dst00->x);
        u1 = 0;
        v1 = 0;
        u2 = p_src->w;
        v3 = p_src->h;

        x1 = p_dst00->x -tmp;
        y1 = 0;
        x2 = p_dst10->x - tmp;
        x3 = p_dst01->x - tmp;
        y3 = p_dst01->y - p_dst00->y;
        x4 = p_dst11->x - tmp;

        if((p_dst00->x == p_dst01->x) && (p_dst10->x == p_dst11->x))
        {
            if(v3 - v1 == y3 - y1)
                *scale_type = SCALE_ONLY_HORI_RECT;
            else if(u2 - u1 == x2 - x1)
                *scale_type = SCALE_ONLY_VERT_RECT;
            else
                *scale_type = SCALE_RECT;
            para_id = 0;
            p_coeff[5] = 0;
        }
        else
        {
            if(v3 - v1 == y3 - y1)
                *scale_type = SCALE_ONLY_HORI_TRAPZ;
            else
                *scale_type = SCALE_TRAPZ;
            para_id = 1;
            p_coeff[5] = 1;
        }
    }
    else if ((p_dst00->x == p_dst01->x) && (p_dst10->x == p_dst11->x))
    {
        tmp = (p_dst01->y > p_dst11->y) ? p_dst01->y : p_dst11->y;
        u1=0;
        v1=0;
        u2 = p_src->h;
        v3 = p_src->w;

        x3 = tmp - p_dst11->y;
        y1 = 0;
        x4 = tmp - p_dst10->y;
        x1 = tmp - p_dst01->y;
        y3 = p_dst10->x - p_dst00->x;
        x2 = tmp - p_dst00->y;
        *scale_type = SCALE_TRANS_TRAPZ;
        para_id = 2;
        p_coeff[5] = 1;
    }
    else
    {
        MT_ERR_TDE("\r\n scale type not unkwon!");
        return;
    }
    if((x2 - x1 == 0) || (y3 == 0))
    {
        MT_ERR_TDE("\r\n scale para Invalid!\n");
        return;
    }
    para11[0] = ((u2<<PARA_FRA)+(x2-x1)/2)/(x2-x1);
    para11[1] = para11[0];
    para11[2] = para11[0];
    if ((x3-x4+x2-x1) == 0)
        para21[0] = (((u2*(x3-x1))<<SOR_FRA)+(y3*(x1-x2))/2)/(y3*(x1-x2));
    else
        para21[0] = (((u2*(x3-x1))<<PARA_FRA_22)+(x3-x4+x2-x1)/2)/(x3-x4+x2-x1);
    para21[1]=para21[0];
    para21[2]=para21[0];
    if ((x3-x4+x2-x1) == 0)
    {
        para22[0] = ((v3<<SOR_FRA))/y3;
        para22[1] = para22[0];
        para22[2] = para22[0];
    }
    else
    {
        para22[0] = (((v3*(x3-x4))<<PARA_FRA_22)+(x3-x4+x2-x1)/2)/(x3-x4+x2-x1);
        para22[1] = para22[0];
        para22[2] = para22[0];
    }
    para31[0] = (((u2*x1)<<PARA_FRA)+(x1-x2)/2)/(x1-x2);
    para31[1] = para31[0];
    para31[2] = para31[0];
    if ((x3-x4+x2-x1) == 0)
        para23[0]=0;
    else
        para23[0] = (((x3-x4+x2-x1)<<PARA_FRA_23_33)+(y3*(x1-x2))/2)/(y3*(x1-x2));
    para23[1] = para23[0];
    para23[2] = para23[0];

    p_coeff[0] = (mt_s32)para11[para_id];
    p_coeff[1] = (mt_s32)para21[para_id];
    p_coeff[2] = (mt_s32)para31[para_id];
    p_coeff[3] = (mt_s32)para22[para_id];
    p_coeff[4] = (mt_s32)para23[para_id];
}
#ifdef SUPPORT_CMDFIFO
static void spn_cmdfifo_write_node(aria_gpe_context_t *p_ctx, cmdfifo_node_t *p_node)
{
    mt_u32 data = 0;
    mt_u32 swap_mod = 0;
    mt_u32 rot_pat_cfg = 0;
    mt_u32 src0_fmt_cfg0 = 0;
    mt_u32 src1_fmt_cfg0 = 0;
    mt_u32 src1_op_pos = 0;

    if(NULL == p_node)
        return;

    //dst
    if(p_ctx->dst_img.color_info.little_endian)
    {
        if(p_ctx->dst_img.color_info.color_fmt == Y1VY0U)
            swap_mod = 1;
        else
        {
            if(p_ctx->dst_img.color_info.bpp == GPE_ARIA_BPP_32BIT)
                swap_mod = 1;
            else if(p_ctx->dst_img.color_info.bpp == GPE_ARIA_BPP_16BIT)
                swap_mod = 2;
            else
                swap_mod = 0;
        }
    }
    else
        swap_mod = 0;
    data = (swap_mod << 24);
    if(p_ctx->dst_rgb2yuv_en)
        data |= 2 << 20;
    else if(p_ctx->dst_yuv2rgb_en)
        data |= 3 << 20;
    data |= (p_ctx->dst_img.color_info.bpp << 16);
    data |= (p_ctx->dst_ds_mod << 8);
    data |= ((!p_ctx->no_dithering) << 4);
    data |= 1;
    data |= p_ctx->dst_ds_choose << 10; //相邻两个像素有一个无效，整个都无效

    write_cmdfifo(p_node, GPE_ARIA_DST0_FMT_CFG0, data);
    data = p_ctx->dst_img.color_info.color_fmt & 0x7f;
    write_cmdfifo(p_node, GPE_ARIA_DST0_FMT_CFG1, data);

    data = (p_ctx->dst_img.height   <<   16) | p_ctx->dst_img.width;
    write_cmdfifo(p_node, GPE_ARIA_DST_PIC_SIZE, data);

    if(p_ctx->dst_img.negative_stride == MT_FALSE)
    {
        write_cmdfifo(p_node, GPE_ARIA_DST0_PIC_ADDR, p_ctx->dst_img.buf);
        write_cmdfifo(p_node, GPE_ARIA_DST0_PIC_STRIDE, p_ctx->dst_img.pitch & 0xffff);
    }
    else
    {
        phys_addr_t base_addr;
        base_addr = p_ctx->dst_img.buf + p_ctx->dst_img.pitch * (p_ctx->dst_img.height  - 1);
        write_cmdfifo(p_node, GPE_ARIA_DST0_PIC_ADDR, base_addr );
        write_cmdfifo(p_node, GPE_ARIA_DST0_PIC_STRIDE, (p_ctx->dst_img.pitch & 0xffff) | (1 << 16));
    }

    data = (p_ctx->dst_img.rect.y   <<   16) | p_ctx->dst_img.rect.x;
    write_cmdfifo(p_node, GPE_ARIA_DST0_OP_POS, data);

    data = (p_ctx->dst_img.rect.h   <<   16) | p_ctx->dst_img.rect.w;
    write_cmdfifo(p_node, GPE_ARIA_DST_OP_SIZE, data);

 #ifdef CONFIG_MT_FPGA_GPE
    p_node->dst_addr = p_ctx->dst_img.buf;
    p_node->dst_pitch = p_ctx->dst_img.pitch;
    p_node->dst_rect.x = p_ctx->dst_img.rect.x;
    p_node->dst_rect.y = p_ctx->dst_img.rect.y;
    p_node->dst_rect.w = p_ctx->dst_img.rect.w;
    p_node->dst_rect.h = p_ctx->dst_img.rect.h;
 /*   GPE_PRINT("\r\n node info.............dst rect:[%d,%d,%d,%d]", p_ctx->dst_img.rect.x, p_ctx->dst_img.rect.y, 
        p_ctx->dst_img.rect.w, p_ctx->dst_img.rect.h);
    GPE_PRINT("\r\n node info.............dst pitch:%d", p_ctx->dst_img.pitch);

    GPE_PRINT("\r\n node info.............dst addr:0x%"PRIx64, (ulong)p_ctx->dst_img.buf);    
    */

#endif

    if(p_ctx->dst_with_mask)
    {
      //  GPE_PRINT("\r\n ~~~~~~~~~dst_with_mask :%d", __LINE__);
         write_cmdfifo(p_node,GPE_ARIA_DST1_FMT_CFG0, 0x3001);
        if(p_ctx->dst_mask.mask2alpha == 2)
            write_cmdfifo(p_node, GPE_ARIA_DST1_FMT_CFG1, 0x73);
        else
            write_cmdfifo(p_node, GPE_ARIA_DST1_FMT_CFG1, 0x70);

        if(p_ctx->dst_img.negative_stride == MT_FALSE)
        {
            write_cmdfifo(p_node, GPE_ARIA_DST1_PIC_STRIDE, p_ctx->dst_mask.mbuf_pitch & 0xffff);
            data = (p_ctx->dst_mask.mbuf_addr) ;
            write_cmdfifo(p_node, GPE_ARIA_DST1_PIC_ADDR, data);
        }
        else
        {
            write_cmdfifo(p_node, GPE_ARIA_DST1_PIC_STRIDE, (p_ctx->dst_mask.mbuf_pitch& 0xffff) | (1 << 16));
            data = (p_ctx->dst_mask.mbuf_addr + p_ctx->dst_mask.mbuf_pitch * (p_ctx->dst_img.height - 1));
            write_cmdfifo(p_node, GPE_ARIA_DST1_PIC_ADDR, data);
        }
    }
    else
    {
        write_cmdfifo(p_node, GPE_ARIA_DST1_FMT_CFG0, 0);
    }

    //src1
    if(p_ctx->src1_sel)
    {
        if(p_ctx->src_img.color_info.little_endian)
        {
            if(p_ctx->src_img.color_info.color_fmt == Y1VY0U)
                swap_mod = 1;
            else if(p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
                swap_mod = 0;
            else
            {
                if(p_ctx->src_img.color_info.bpp == GPE_ARIA_BPP_32BIT)
                    swap_mod = 1;
                else if(p_ctx->src_img.color_info.bpp == GPE_ARIA_BPP_16BIT)
                    swap_mod = 2;
                else
                    swap_mod = 0;
            }
        }
        else
            swap_mod = 0;
        //  GPE_PRINT("\r\n src1 little endian:%d, fmt:%d, bpp:%d, swap_mod:%d", p_ctx->src_img.color_info.little_endian,
        //    p_ctx->src_img.color_info.color_fmt,p_ctx->src_img.color_info.bpp, swap_mod);
        data = (swap_mod << 24);
        if(p_ctx->src1_rgb2yuv_en)
            data |= 2 << 20;
        else if(p_ctx->src1_yuv2rgb_en)
            data |= 3 << 20;
        data |= (p_ctx->src_img.color_info.bpp << 16);
        if((p_ctx->src_img.color_info.is_pix_alpha) || (p_ctx->src_img.with_palette))
            data |= (p_ctx->src_img.color_info.alpha_ch_en << 8);
        data |= ((p_ctx->src_img.with_palette && p_ctx->src1_palt_load_en) << 4);
        if(p_ctx->src_img_en)
            data |= 1;

        //bit swap clut1/clut2/clut4 to keep consistent with concerto
        if(p_ctx->src_img.color_info.color_fmt == CLUT_1)
            data |= (3 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_2)
            data |= (2 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_4)
            data |= (1 << 28);
#ifdef CONFIG_MT_FPGA_GPE
      if(g_debug.src1_bitswap == 1)
      {
        if(p_ctx->src_img.color_info.color_fmt == CLUT_1)
          data |= (3 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_2)
          data |= (2 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_4)
          data |= (1 << 28);        
        else if(p_ctx->src_img.color_info.color_fmt == ALUT44)
          data |= (1 << 28);         
      }
      else if(g_debug.src1_bitswap == 0)
        data &= 0x0fffffff;            
#endif  

        src1_fmt_cfg0 = data;
        write_cmdfifo(p_node, GPE_ARIA_SRC1_FMT_CFG0, data);
    }
    else
    {
        src1_fmt_cfg0 = 0;
        write_cmdfifo(p_node, GPE_ARIA_SRC1_FMT_CFG0, 0);
    }
    if((p_ctx->src1_sel)  &&  (p_ctx->src_img_en))
    {
        data = (p_ctx->src_img.height   <<   16) | p_ctx->src_img.width;
        write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_SIZE, data);

        if(p_ctx->src_img.negative_stride == MT_FALSE)
        {
            write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_ADDR, p_ctx->src_img.buf);
            if(p_ctx->src_is_xylc)
                write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_STRIDE, 0x4000);
            else
                write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_STRIDE, p_ctx->src_img.pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            base_addr = p_ctx->src_img.buf + p_ctx->src_img.pitch * (p_ctx->src_img.height  - 1);
            write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_ADDR, base_addr);
            if(p_ctx->src_is_xylc)
                write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_STRIDE, 0x4000);
            else
                write_cmdfifo(p_node, GPE_ARIA_SRC1_PIC_STRIDE, (p_ctx->src_img.pitch & 0xffff) | (1 << 16));
        }

#ifdef FAST_2D_SCALE
        data = 0;
#ifdef CONFIG_MT_FPGA_GPE
        if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0)
#else
        if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0
            && p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w
            && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
        {
#if 0            
            if(p_ctx->src_img.rect.x && (p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w))
            {
                data = p_ctx->scale_cfg.init_phase_x / 32 + p_ctx->src_img.rect.x - 1;
            }
            else
            {
                data = p_ctx->src_img.rect.x;
            }

            if(p_ctx->src_img.rect.y && (p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h))
            {
                data |= (p_ctx->scale_cfg.init_phase_y / 32 + p_ctx->src_img.rect.y - 1) << 16;
            }
            else
            {
                data |= (p_ctx->src_img.rect.y   <<   16);
            }
#else  //from jiaoqiaowei
            if(p_ctx->src_img.rect.x && (p_ctx->src_img.rect.w < p_ctx->dst_img.rect.w))
            {
                data = p_ctx->scale_cfg.init_phase_x / 32 + p_ctx->src_img.rect.x - 1;
            }
            else
            {
                data = p_ctx->src_img.rect.x;
            }

            if(p_ctx->src_img.rect.y && (p_ctx->src_img.rect.h < p_ctx->dst_img.rect.h))
            {
                data |= (p_ctx->scale_cfg.init_phase_y / 32 + p_ctx->src_img.rect.y - 1) << 16;
            }
            else
            {
                data |= p_ctx->src_img.rect.y<<16; 
            }
#endif
        }
        else
        {
            data = (p_ctx->src_img.rect.y   <<   16) | p_ctx->src_img.rect.x;
        }
#else
        data = (p_ctx->src_img.rect.y   <<   16) | p_ctx->src_img.rect.x;
#endif
        write_cmdfifo(p_node, GPE_ARIA_SRC1_OP_POS, data);
        src1_op_pos = data;
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
        if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0)
#else
      if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0
                && p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w
                && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
        {
            mt_u32 x, y, w, h;
            // data = gpe_read_register(GPE_ARIA_SRC1_OP_POS);
            x = src1_op_pos & 0xffff;
            y = (src1_op_pos >> 16) & 0xffff;
            if(x + p_ctx->src_img.rect.w > p_ctx->src_img.width)
                w = p_ctx->src_img.width - x;
            else
                w = p_ctx->src_img.rect.w;
            if(p_ctx->dst_img.rect.w >= w && (w % 64 == 0) && ((x+w) < p_ctx->src_img.width))
                w = w + 1;
            if(y + p_ctx->src_img.rect.h > p_ctx->src_img.height)
                h = p_ctx->src_img.height - y;
            else
                h = p_ctx->src_img.rect.h;
            data = (h   <<   16) | w;
        }
        else
            data = (p_ctx->src_img.rect.h   <<   16) | p_ctx->src_img.rect.w;
#else
        data = (p_ctx->src_img.rect.h   <<   16) | p_ctx->src_img.rect.w;
#endif
        write_cmdfifo(p_node, GPE_ARIA_SRC1_OP_SIZE, data);

        if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
            data = SP_YUV422_Y;
        else
            data = p_ctx->src_img.color_info.color_fmt & 0x7f;
        if(p_ctx->src_img.ck_en)
        {
            data |= (p_ctx->src_img.ck_mod << 16) | (p_ctx->src_img.ck_select << 8);
            write_cmdfifo(p_node, GPE_ARIA_SRC1_KEY_MIN, p_ctx->src_img.ck_min);
            write_cmdfifo(p_node, GPE_ARIA_SRC1_KEY_MAX, p_ctx->src_img.ck_max);
        }
        //exp mode
        data |= (p_ctx->color_exp_mode << 12) | (p_ctx->color_exp_mode << 14);
        write_cmdfifo(p_node, GPE_ARIA_SRC1_FMT_CFG1, data);
        if((p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
                || (p_ctx->src_img.color_info.color_fmt == CMYK8888))
        {
            data = 0x1 | ((p_ctx->cmyk_max & 0xff) << 8) | (p_ctx->cmyk_coef << 16);
            write_cmdfifo(p_node, GPE_ARIA_SRC1_CMYK_CFG, data);
        }
    }

    //src0
    if((p_ctx->src_with_mask)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
            || (p_ctx->src_img.color_info.color_fmt == TILE_Y)
            || (p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM))
    {
        if((p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
                || (p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
                || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2))
        {
            data = (0x4 << 16) | 0x1;
        }
        else
        {
            data = (0x3 << 16) | 0x1;
        }
        if(p_ctx->src_img.color_info.little_endian)
            data |= (0x2 << 24);
        src0_fmt_cfg0 = data;
        write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG0, data);

        if(p_ctx->src_with_mask)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, GRAY_8);
        else if(p_ctx->src_img.color_info.color_fmt == TILE_Y)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, TILE_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, SP_YUV444_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, SP_YUV422_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, SP_YUV420_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, SP_YUV420_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
            write_cmdfifo(p_node, GPE_ARIA_SRC0_FMT_CFG1, SP_CMYK8888_YK);

        if(p_ctx->src_img.negative_stride == MT_FALSE)
        {
            write_cmdfifo(p_node, GPE_ARIA_SRC0_PIC_ADDR, p_ctx->src0_buf);
            write_cmdfifo(p_node, GPE_ARIA_SRC0_PIC_STRIDE, p_ctx->src0_pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            if((p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
                    || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2))
                base_addr = p_ctx->src0_buf + p_ctx->src0_pitch * (p_ctx->src_img.height / 2  - 1);
            else
                base_addr = p_ctx->src0_buf + p_ctx->src0_pitch * (p_ctx->src_img.height  - 1);
            write_cmdfifo(p_node, GPE_ARIA_SRC0_PIC_ADDR, base_addr);
            write_cmdfifo(p_node, GPE_ARIA_SRC0_PIC_STRIDE, (p_ctx->src0_pitch & 0xffff) | (1 << 16));
        }
    }

    //src2
    if(p_ctx->src2_sel)
    {
        gpe_img_t *p_img = NULL;

        if(p_ctx->bg_img_en)
            p_img = &(p_ctx->bg_img);
        else
            p_img = &(p_ctx->dst_img);

        if(p_img->color_info.little_endian)
        {
            if(p_img->color_info.color_fmt == Y1VY0U)
                swap_mod = 1;
            else
            {
                if(p_img->color_info.bpp == GPE_ARIA_BPP_32BIT)
                    swap_mod = 1;
                else if(p_img->color_info.bpp == GPE_ARIA_BPP_16BIT)
                    swap_mod = 2;
                else
                    swap_mod = 0;
            }
        }
        else
            swap_mod = 0;
        data = (swap_mod << 24);
        if(p_ctx->src2_rgb2yuv_en)
            data |= 2 << 20;
        else if(p_ctx->src2_yuv2rgb_en)
            data |= 3 << 20;
        data |= (p_img->color_info.bpp << 16);
        if((p_img->color_info.is_pix_alpha) || (p_img->with_palette))
            data |= (p_img->color_info.alpha_ch_en << 8);
        data |= 1;
        write_cmdfifo(p_node, GPE_ARIA_SRC2_FMT_CFG0, data);

        if(p_img->negative_stride == MT_FALSE)
        {
            write_cmdfifo(p_node, GPE_ARIA_SRC2_PIC_ADDR, p_img->buf);
            write_cmdfifo(p_node, GPE_ARIA_SRC2_PIC_STRIDE, p_img->pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            base_addr = p_img->buf + p_img->pitch * (p_img->height  - 1);
            write_cmdfifo(p_node, GPE_ARIA_SRC2_PIC_ADDR, base_addr);
            write_cmdfifo(p_node, GPE_ARIA_SRC2_PIC_STRIDE, (p_img->pitch & 0xffff) | (1 << 16));
        }

        data = (p_img->rect.y << 16) | p_img->rect.x;
        write_cmdfifo(p_node, GPE_ARIA_SRC2_OP_POS, data);

        data = p_img->color_info.color_fmt & 0x7f;
        if(p_ctx->dst_img.ck_en)
        {
            data |= (p_ctx->dst_img.ck_mod << 16) | (p_ctx->dst_img.ck_select << 8);
            write_cmdfifo(p_node, GPE_ARIA_SRC2_KEY_MIN, p_ctx->dst_img.ck_min);
            write_cmdfifo(p_node, GPE_ARIA_SRC2_KEY_MAX, p_ctx->dst_img.ck_max);
        }
        //exp mode
        data |= (p_ctx->color_exp_mode << 12) | (p_ctx->color_exp_mode << 14);
        write_cmdfifo(p_node, GPE_ARIA_SRC2_FMT_CFG1, data);
    }
    else
    {
        write_cmdfifo(p_node, GPE_ARIA_SRC2_FMT_CFG0, 0);
    }

    //src3
    if(p_ctx->src3_sel)
    {
        if(p_ctx->ex_img.color_info.little_endian)
        {
            if(p_ctx->ex_img.color_info.color_fmt == Y1VY0U)
                swap_mod = 1;
            else if(p_ctx->ex_img.color_info.color_fmt == SP_YUV444_Y)
                swap_mod = 0;
            else
            {
                if(p_ctx->ex_img.color_info.bpp == GPE_ARIA_BPP_32BIT)
                    swap_mod = 1;
                else if(p_ctx->ex_img.color_info.bpp == GPE_ARIA_BPP_16BIT)
                    swap_mod = 2;
                else
                    swap_mod = 0;
            }
        }
        else
            swap_mod = 0;
        data = (swap_mod << 24);
        if(p_ctx->src3_rgb2yuv_en)
            data |= 2 << 20;
        else if(p_ctx->src3_yuv2rgb_en)
            data |= 3 << 20;
        data |= (p_ctx->ex_img.color_info.bpp << 16);
        if((p_ctx->ex_img.color_info.is_pix_alpha) || (p_ctx->ex_img.with_palette))
            data |= (p_ctx->ex_img.color_info.alpha_ch_en << 8);
        data |= ((p_ctx->ex_img.with_palette && p_ctx->src3_palt_load_en) << 4);
        data |= 1;
        //bit swap clut1/clut2/clut4 to keep consistent with concerto
        if(p_ctx->ex_img.color_info.color_fmt == CLUT_1)
            data |= (3 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_2)
            data |= (2 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_4)
            data |= (1 << 28);

#ifdef CONFIG_MT_FPGA_GPE
      if(g_debug.src3_bitswap == 1)
      {
        if(p_ctx->ex_img.color_info.color_fmt == CLUT_1)
          data |= (3 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_2)
          data |= (2 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_4)
          data |= (1 << 28);        
        else if(p_ctx->ex_img.color_info.color_fmt == ALUT44)
          data |= (1 << 28);         
      }
      else if(g_debug.src3_bitswap == 0)
        data &= 0x0fffffff;            
#endif 

        write_cmdfifo(p_node, GPE_ARIA_SRC3_FMT_CFG0, data);

        if(p_ctx->ex_img.negative_stride == MT_FALSE)
        {
            write_cmdfifo(p_node, GPE_ARIA_SRC3_PIC_ADDR, p_ctx->ex_img.buf);
            write_cmdfifo(p_node, GPE_ARIA_SRC3_PIC_STRIDE, p_ctx->ex_img.pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            base_addr = p_ctx->ex_img.buf + p_ctx->ex_img.pitch * (p_ctx->ex_img.height  - 1);
            write_cmdfifo(p_node, GPE_ARIA_SRC3_PIC_ADDR, base_addr);
            write_cmdfifo(p_node, GPE_ARIA_SRC3_PIC_STRIDE, (p_ctx->ex_img.pitch & 0xffff) | (1 << 16));
        }

        data = (p_ctx->ex_img.rect.y   <<   16) | p_ctx->ex_img.rect.x;
        write_cmdfifo(p_node, GPE_ARIA_SRC3_OP_POS, data);
        data = p_ctx->ex_img.color_info.color_fmt & 0x7f;
        if(p_ctx->ex_img.ck_en)
        {
            data |= (p_ctx->ex_img.ck_mod << 16) | (p_ctx->ex_img.ck_select << 8);
            write_cmdfifo(p_node, GPE_ARIA_SRC3_KEY_MIN, p_ctx->ex_img.ck_min);
            write_cmdfifo(p_node, GPE_ARIA_SRC3_KEY_MAX, p_ctx->ex_img.ck_max);
        }
        //exp mode
        data |= (p_ctx->color_exp_mode << 12) | (p_ctx->color_exp_mode << 14);
        write_cmdfifo(p_node, GPE_ARIA_SRC3_FMT_CFG1, data);
    }
    else
        write_cmdfifo(p_node, GPE_ARIA_SRC3_FMT_CFG0, 0);

    //palette
    data = 0;
    if(p_ctx->src1_palt_load_en)
    {
        data |= p_ctx->src_img.palt_size & 0x1ff;
        write_cmdfifo(p_node, GPE_ARIA_PAL1_ADDR, p_ctx->src_img.palt_buf);
    }
    if(p_ctx->src3_palt_load_en)
    {
        data |= (p_ctx->ex_img.palt_size & 0x1ff) << 16;
        write_cmdfifo(p_node, GPE_ARIA_PAL3_ADDR, p_ctx->ex_img.palt_buf);
    }
    write_cmdfifo(p_node, GPE_ARIA_PAL_SIZE, data);

    //table load en
    data = (p_ctx->scale_en << 8) | (p_ctx->src3_palt_load_en << 4) | p_ctx->src1_palt_load_en;
    if(p_ctx->src1_palt_load_en)
    {
        if((p_ctx->src_img.palt_format == GPE_ARIA_PALT_ARGB8888)
                || (p_ctx->src_img.palt_format == GPE_ARIA_PALT_AYUV8888))
        {
            if(p_ctx->src_img.palt_little_endian)
                data |= 2 << 16;
        }
        else
        {
            if(p_ctx->src_img.palt_little_endian)
                data |= 3 << 16;
            else
                data |= 1 << 16;
        }
    }
    if(p_ctx->src3_palt_load_en)
    {
        if((p_ctx->ex_img.palt_format == GPE_ARIA_PALT_ARGB8888)
                || (p_ctx->ex_img.palt_format == GPE_ARIA_PALT_AYUV8888))
        {
            if(p_ctx->ex_img.palt_little_endian)
                data |= 2 << 20;
        }
        else
        {
            if(p_ctx->ex_img.palt_little_endian)
                data |= 3 << 20;
            else
                data |= 1 << 20;
        }
    }
    write_cmdfifo(p_node, GPE_ARIA_LOAD_EN, data);

    //rop
    if(p_ctx->rop_en)
    {
        data = (p_ctx->rop_a_mod   <<   8) | p_ctx->rop_c_mod;
        write_cmdfifo(p_node, GPE_ARIA_ROP_ID, data);
        write_cmdfifo(p_node, GPE_ARIA_ROP_PAT, p_ctx->rop_pattern);
    }
    //blend
    if(p_ctx->blend_en)
    {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) 
        if(p_ctx->demultiply_en)
            data = 1 << 16;
        else
#endif      
            data = 0;
        data |= (p_ctx->dst_blend_fact  <<   4) | p_ctx->src_blend_fact;
        data |= (p_ctx->adst_blend_fact  <<   12) | (p_ctx->asrc_blend_fact << 8);
        write_cmdfifo(p_node, GPE_ARIA_COMP_BLD_MOD, data);
    }

    //rotator
    if(p_ctx->rotator_en)
    {
        rot_pat_cfg = p_ctx->rotator_op & 0x7;
    }

    //xylc
    if(p_ctx->src_is_xylc)
    {
        write_cmdfifo(p_node, GPE_ARIA_XYLC_CFG, p_ctx->xylc_cfg.xylc_num);
        write_cmdfifo(p_node, GPE_ARIA_PAT_COLOR, p_ctx->xylc_cfg.xylc_color);
    }

    //tile
    if(p_ctx->src_is_tile)
    {
#if defined(CONFIG_MT_CHIP_ARIA)
        mt_sys_read_register(0xffd202e0, &data);
        mt_u32 tile_size = data & 0x3; //col_size_mode
        mt_u32 hd_map_mode = (data >> 12) & 0x1;
        mt_u32 field_flag = (data >> 8) & 0x1;
        mt_u32 tile_cfg = (data >> 16) & 0x3;

        mt_sys_read_register(0xffd202e4, &data);
        write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP00, data);
        mt_sys_read_register(0xffd202e8, &data);
        write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP01, data);
        mt_sys_read_register(0xffd202ec, &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP10, data);
        mt_sys_read_register(0xffd202f0, &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP11, data);
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402e0), &data);
        mt_u32 tile_size = data & 0x3; //col_size_mode
        mt_u32 hd_map_mode = (data >> 12) & 0x1;
        mt_u32 field_flag = (data >> 8) & 0x1;
        mt_u32 tile_cfg = (data >> 16) & 0x3;
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402e4), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP00, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402e8), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP01, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402ec), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP10, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402f0), &data);
        write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP11, data);
#else
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf000204), &data);
        mt_u32 tile_size = (data >> 8) & 0x3; //col_size_mode
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf441090), &data);
        mt_u32 hd_map_mode = data & 0x1;
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf44108c), &data);
        mt_u32 field_flag = data & 0x1;
        mt_u32 tile_cfg = (data >> 16) & 0x3;

        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401c0), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP00, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401c4), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP01, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401c8), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP10, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401cc), &data);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_JMP11, data);

#endif
        data = (hd_map_mode << 8) | (field_flag << 12) | (tile_size << 4) | tile_cfg;
        write_cmdfifo(p_node,GPE_ARIA_SRC1_TILE_CFG, data);

         write_cmdfifo(p_node,GPE_ARIA_SRC0_PIC_STRIDE, 4096);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_PIC_STRIDE, 4096);

        //tile y 8byte swap
        data = src1_fmt_cfg0  | (3 << 24);
         write_cmdfifo(p_node,GPE_ARIA_SRC1_FMT_CFG0, data);
        //tile uv 2byte swap
        data = src0_fmt_cfg0 | (4 << 24);
         write_cmdfifo(p_node,GPE_ARIA_SRC0_FMT_CFG0, data);       
    }
        

    //scale
    if(p_ctx->scale_en)
    {
        if(g_psMBuf.phyaddr == 0)
        {
            printf("\r\n ~~~~~~~~~scale memory is null!!!!!!");
        }
#ifdef CONFIG_MT_FPGA_GPE
        data = p_ctx->scale_cfg.scale_mod |
            ((p_ctx->scale_cfg.disable_alpha_filter) << 5) |
            ((p_ctx->scale_cfg.disable_color_filter) << 4) |
            (p_ctx->scale_cfg.disable_color_anti_flicker << 12) |
            (p_ctx->scale_cfg.disable_alpha_anti_flicker << 13);
#else
        data = p_ctx->scale_cfg.scale_mod |
            ((p_ctx->scale_cfg.disable_alpha_filter) << 5) |
            ((p_ctx->scale_cfg.disable_color_filter) << 4) |
            (p_ctx->scale_cfg.disable_color_anti_flicker << 12) |
            (p_ctx->scale_cfg.disable_alpha_anti_flicker << 13) |
            (2 << 8);
#endif
        MT_INFO_TDE("\r\n scale_cfg:%d", data);
        write_cmdfifo(p_node, GPE_ARIA_SCALER_CFG, data);
        write_cmdfifo(p_node, GPE_ARIA_SCALER_COEF_11, (mt_u32)(p_ctx->scale_cfg.coef[0]));
        write_cmdfifo(p_node, GPE_ARIA_SCALER_COEF_21, (mt_u32)(p_ctx->scale_cfg.coef[1]));
        write_cmdfifo(p_node, GPE_ARIA_SCALER_COEF_31, (mt_u32)(p_ctx->scale_cfg.coef[2]));
        write_cmdfifo(p_node, GPE_ARIA_SCALER_COEF_22, (mt_u32)(p_ctx->scale_cfg.coef[3]));
        write_cmdfifo(p_node, GPE_ARIA_SCALER_COEF_23, (mt_u32)(p_ctx->scale_cfg.coef[4]));
        if(p_ctx->scale_cfg.coef[5] == 0)
        {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
            if(1)
#else
            if(p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif                
            {
#if 0                
                //      data = gpe_read_register(GPE_ARIA_SRC1_OP_POS);
                data = src1_op_pos; //(p_ctx->src_img.rect.y   <<   16) | p_ctx->src_img.rect.x;
                src_x = data & 0xffff;
                src_y = (data >> 16) & 0xffff;
                if(p_ctx->src_img.rect.x && (src_x != 0))
                    data = p_ctx->scale_cfg.init_phase_x + 32;
                else
                    data = p_ctx->scale_cfg.init_phase_x;

                if(p_ctx->src_img.rect.y && (src_y != 0))
                    data |= (p_ctx->scale_cfg.init_phase_y + 32) << 16;
                else
                    data |= p_ctx->scale_cfg.init_phase_y << 16;
#else     //from jiaoqiaowei
                if(p_ctx->src_img.rect.x && (p_ctx->src_img.rect.w < p_ctx->dst_img.rect.w))
                {
                    data = p_ctx->scale_cfg.init_phase_x + 32;
                }
                else
                {
                    data = p_ctx->scale_cfg.init_phase_x;
                }
                
                if(p_ctx->src_img.rect.y && (p_ctx->src_img.rect.h < p_ctx->dst_img.rect.h))
                {
                    data |= (p_ctx->scale_cfg.init_phase_y + 32) << 16;
                }
                else
                {
                    data |= p_ctx->scale_cfg.init_phase_y << 16;  
                }
#endif                
                write_cmdfifo(p_node, GPE_ARIA_FAST_SCALER_INIT_PHASE, data);
            }
            else
#endif
            {
                write_cmdfifo(p_node, GPE_ARIA_SCALER_INIT_PHASE, p_ctx->scale_cfg.init_phase | (1 << 16));
            }
        }
        else
            write_cmdfifo(p_node, GPE_ARIA_SCALER_INIT_PHASE, p_ctx->scale_cfg.init_phase);
        data = (mt_u32)u32CoeffPhyAddr;
        write_cmdfifo(p_node, GPE_ARIA_COEF_ADDR, data);

        //to improve performance
        if(p_ctx->scale_cfg.scale_mod == SCALE_VERT_BLK_OUT)
            write_cmdfifo(p_node, GPE_ARIA_GRA_REQ_CFG, 0x40000cc);


        if((ChipVersion >= MT_CHIP_SYMPHONY_A1)
                && (p_ctx->src_is_tile == MT_FALSE))
        {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
            if(p_ctx->scale_cfg.coef[5] == 0)
#else
            if(p_ctx->scale_cfg.coef[5] == 0
                    && p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w
                    && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
            {
                data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
                data &= 0xf0ffff00;
                data |= 0x020000cc;
                write_cmdfifo(p_node, GPE_ARIA_GRA_REQ_CFG, data);
            }
            else
#endif
            {
                if(p_ctx->scale_cfg.scale_mod == SCALE_HORI_LINE_OUT)
                {
                    data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
                    data &= 0xf0ffff00;
                    data |= 0x020000cc;
                    write_cmdfifo(p_node, GPE_ARIA_GRA_REQ_CFG, data);
                }
                else
                {
                    data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
                    data &= 0xf0ffff00;
                    data |= 0x040000cc;
                    write_cmdfifo(p_node, GPE_ARIA_GRA_REQ_CFG, data);
                }
            }
        }
    }
    else
    {
        if(p_ctx->src_is_tile == MT_FALSE)
        {
            data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
            data &= 0xf0000000;
            data |= 0x02cccccc;
            write_cmdfifo(p_node, GPE_ARIA_GRA_REQ_CFG, data);
        }
    }

    if(p_ctx->src_is_tile == MT_FALSE)
    {
        data = gpe_read_register(GPE_ARIA_GRA_PIN_SEL);
        data |= 0x6;
        write_cmdfifo(p_node, GPE_ARIA_GRA_PIN_SEL, data);
    }

    //blur
    if(p_ctx->gaussian_blur_en)
    {
        data = SCALE_HORI_LINE_OUT | (p_ctx->blur_cfg.blur_tap << 20) |
            (p_ctx->gaussian_blur_en << 16);
        write_cmdfifo(p_node, GPE_ARIA_SCALER_CFG, data);
    }

    //paint
    if(p_ctx->paint_en)
    {
        write_cmdfifo(p_node, GPE_ARIA_PAT_COLOR, p_ctx->paint.paint_color);

        if(p_ctx->paint_pattern_en)
        {
            //mt_u32 data = 0;
            data = p_ctx->paint.pat_beg.x | (p_ctx->paint.pat_beg.y << 16);
            write_cmdfifo(p_node, GPE_ARIA_PAT_OFFSET_POS, data);

            rot_pat_cfg |= (p_ctx->paint.tiling_mod & 0x3) << 4;

            if((p_ctx->paint.tiling_mod == GPE_ARIA_TILE_REPEAT)
                    || (p_ctx->paint.tiling_mod == GPE_ARIA_TILE_REFLECT))
            {
                data = p_ctx->paint.pat_remd1 | (p_ctx->paint.pat_quot1 << 16);
                write_cmdfifo(p_node, GPE_ARIA_PAT_RATIO_X_0, data);
                data = p_ctx->paint.pat_remd2 | (p_ctx->paint.pat_quot2 << 16);
                write_cmdfifo(p_node, GPE_ARIA_PAT_RATIO_X_1, data);
                data = p_ctx->paint.pat_remd3 | (p_ctx->paint.pat_quot3 << 16);
                write_cmdfifo(p_node, GPE_ARIA_PAT_RATIO_Y_0, data);
                data = p_ctx->paint.pat_remd4 | (p_ctx->paint.pat_quot4 << 16);
                write_cmdfifo(p_node, GPE_ARIA_PAT_RATIO_Y_1, data);
            }
        }
        else
        {
            //data = ((p_ctx->paint.gradt_type & 0x1) << 2) | (p_ctx->paint.spread_mod & 0x3);
            if(p_ctx->paint.gradt_type == GPE_ARIA_LINER_GRADT)
                data = (p_ctx->paint.mask_mod << 4) | (0 << 3) | (0 << 2) | (p_ctx->paint.spread_mod & 0x3);
            else
            {
                if((p_ctx->paint.center.x == p_ctx->paint.focus.x) && (p_ctx->paint.center.y == p_ctx->paint.focus.y))
                {
                    data = (p_ctx->paint.mask_mod << 4) | (0 << 3) | (1 << 2) | (p_ctx->paint.spread_mod & 0x3);
                }
                else
                {
                    data = (p_ctx->paint.mask_mod << 4) | (1 << 3) | (1 << 2) | (p_ctx->paint.spread_mod & 0x3);
                }
            }
            write_cmdfifo(p_node, GPE_ARIA_GRADT_CFG, data);

            if(p_ctx->paint.true_liner_gradt)
            {
                spn_gpe_cmdfifo_set_gradt_regs(p_node, &(p_ctx->paint));
            }
            if(p_ctx->paint.true_liner_gradt  || (p_ctx->paint.gradt_type == GPE_ARIA_RADIAL_GRADT) || (p_ctx->paint.gradt_type == GPE_ARIA_ELLIPSE_GRADT))
            {
                spn_gpe_cmdfifo_set_gradt_stop_regs(p_node, &(p_ctx->paint));
            }
            if((p_ctx->paint.center.x == p_ctx->paint.focus.x) && (p_ctx->paint.center.y == p_ctx->paint.focus.y))
            {
                spn_gpe_cmdfifo_set_gradt_radius_regs(p_node, &(p_ctx->paint));
            }
        }
    }

    //rot_pat_cfg
    write_cmdfifo(p_node, GPE_ARIA_ROT_PAT_CFG, rot_pat_cfg);

    if(p_ctx->comp_en)
    {
        mt_u32 src1_mult_mod = 0;
        mt_u32 src3_mult_mod = 0;

        if(p_ctx->is_draw_multiply)
        {
            src1_mult_mod = 0x1;
        }
        else if(p_ctx->alpha_map_en)
        {
            if(p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_MIX_NORMAL)
            {
                src1_mult_mod = 0x2;
            }
            else if(p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_MIX_EX)
            {
                src1_mult_mod = 0x3;
            }
        }
        //    GPE_PRINT("\r\n is_draw_stencil:%d", p_ctx->is_draw_stencil);
        if(p_ctx->is_draw_stencil)
        {
            src3_mult_mod = 0x2;
        }

        data = (src3_mult_mod << 28) | (src1_mult_mod << 12);
        if(p_ctx->src_img.plane_alpha_en)
            data |= (1 << 8) | (p_ctx->src_img.plane_alpha & 0xff);
        if(p_ctx->ex_img.plane_alpha_en)
            data |= (1 << 24) | ((p_ctx->ex_img.plane_alpha & 0xff) << 16);
        data |= (p_ctx->src_img.color_info.alpha_pre_mult_en<< 9)
            | (p_ctx->ex_img.color_info.alpha_pre_mult_en << 25);
        write_cmdfifo(p_node, GPE_ARIA_COMP_MULT_MOD, data);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        if(p_ctx->src2_sel)
        {
            gpe_img_t *p_img = NULL;

            if(p_ctx->bg_img_en)
                p_img = &(p_ctx->bg_img);
            else
                p_img = &(p_ctx->dst_img);
            data = 0;
            if(p_img->color_info.alpha_pre_mult_en)
                data = 1 << 9;       
            if(p_img->plane_alpha_en)
                data |= p_img->plane_alpha | (1 << 8);

			write_cmdfifo(p_node, GPE_SPN_COMP_MULT_MOD2, data);
        }
#endif

        if(p_ctx->blend_en)
            data = 1;
        else
            data = 0;
        if(p_ctx->alpha_map_en && (p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_LOGICAL))
            data |= (1 << 8);
        if(p_ctx->comp_key_set)
            data |= (1 << 4) | (1 << 12);
        data |= (p_ctx->src1_sel << 16);
        data |= p_ctx->comp_bp << 24;
        write_cmdfifo(p_node, GPE_ARIA_COMP_CFG, data);
    }

    //yuv to rgb enable
    aria_yuv_to_rgb_enable_check(p_ctx);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(p_ctx->clip_en)
    { 
		write_cmdfifo(p_node, GPE_SPN_REGION_CFG, p_ctx->clip.clip_mode); 
        data = p_ctx->clip.clip_rect.s32Xpos| (p_ctx->clip.clip_rect.s32Ypos<< 16);
		write_cmdfifo(p_node, GPE_SPN_REGION_START, data); 
        data = (p_ctx->clip.clip_rect.s32Xpos+ p_ctx->clip.clip_rect.u32Width - 1) 
            | ((p_ctx->clip.clip_rect.s32Ypos+ p_ctx->clip.clip_rect.u32Height- 1) << 16);
		write_cmdfifo(p_node, GPE_SPN_REGION_STOP, data);
    }

    if(p_ctx->colorize_en)
    {
		write_cmdfifo(p_node, GPE_SPN_COMP_COLORIZE, (p_ctx->color & 0x00ffffff) | (1 << 24));
    }
    else
	{
        write_cmdfifo(p_node, GPE_SPN_COMP_COLORIZE, 0);
    }
	
#endif

    //GPE_PRINT("\r\n rotator_en:%d", p_ctx->rotator_en);
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
    if(1)
#else
    if(p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
    {
        data = 0;
        if(p_ctx->scale_en)
        {
            if(p_ctx->scale_cfg.coef[5] == 0)
                data |= (1 << 28);
            else
                data |= (1 << 4);
        }

        data |= p_ctx->comp_en
            | (p_ctx->gaussian_blur_en << 4)
            | (p_ctx->rotator_en << 8)
            | (p_ctx->paint_pattern_en << 12)
            | (p_ctx->src_is_xylc << 16)
            | (p_ctx->paint_gradt_en << 20)
            | (p_ctx->clip_en << 24);
        write_cmdfifo(p_node, GPE_ARIA_GRA_EN, data);
    }
    else
#endif
    {
        data = p_ctx->comp_en
            | ((p_ctx->scale_en | p_ctx->gaussian_blur_en) << 4)
            | (p_ctx->rotator_en << 8)
            | (p_ctx->paint_pattern_en << 12)
            | (p_ctx->src_is_xylc << 16)
            | (p_ctx->paint_gradt_en << 20)
            | (p_ctx->clip_en << 24);
        write_cmdfifo(p_node, GPE_ARIA_GRA_EN, data);
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
  write_cmdfifo(p_node, GPE_ARIA_WCH_REQ_MSK_SYNC_MODE , 0x00000100); 
  if((p_ctx->src_img_en&& (p_ctx->src_img.with_palette&& p_ctx->src1_palt_load_en)) 
      || (p_ctx->ex_img_en&& (p_ctx->ex_img.with_palette && p_ctx->src3_palt_load_en)))
  {    
      write_cmdfifo(p_node, GPE_ARIA_GRA_AXI_CFG, 0x00000000); //clut ->clut ,GPE_ARIA_GRA_AXI_CFG must be set 1
  }
  else
  {
      write_cmdfifo(p_node, GPE_ARIA_GRA_AXI_CFG, 0x00000001);
  }
#endif
}
#endif

static void aria_gpe_set_regs(aria_gpe_context_t *p_ctx)
{
    mt_u32 data = 0;
    mt_u32 swap_mod = 0;
    mt_u32 rot_pat_cfg = 0;
    
    //dst
    if(p_ctx->dst_img.color_info.little_endian)
    {
        if(p_ctx->dst_img.color_info.color_fmt == Y1VY0U)
            swap_mod = 1;
        else
        {
            if(p_ctx->dst_img.color_info.bpp == GPE_ARIA_BPP_32BIT)
                swap_mod = 1;
            else if(p_ctx->dst_img.color_info.bpp == GPE_ARIA_BPP_16BIT)
                swap_mod = 2;
            else
                swap_mod = 0;
        }
    }
    else
        swap_mod = 0;

    data = (swap_mod << 24);
    if(p_ctx->dst_rgb2yuv_en)
        data |= 2 << 20;
    else if(p_ctx->dst_yuv2rgb_en)
        data |= 3 << 20;
    data |= (p_ctx->dst_img.color_info.bpp << 16);
    data |= (p_ctx->dst_ds_mod << 8);
    data |= (mt_u32)((!p_ctx->no_dithering) << 4);
    data |= 1;
    data |= p_ctx->dst_ds_choose << 10; //???????????????????效??????????效
    gpe_write_register(GPE_ARIA_DST0_FMT_CFG0, data);
#if 0
    if(p_ctx->dst_img.color_info.color_fmt == AYUV8888)
        data = ARGB8888;
    else if(p_ctx->dst_img.color_info.color_fmt == YUVA8888)
        data = RGBA8888;
    else
#endif
        data = p_ctx->dst_img.color_info.color_fmt & 0x7f;
    gpe_write_register(GPE_ARIA_DST0_FMT_CFG1, data);
    data = (p_ctx->dst_img.height   <<   16) | p_ctx->dst_img.width;
    gpe_write_register(GPE_ARIA_DST_PIC_SIZE, data);
    if(p_ctx->dst_img.negative_stride == MT_FALSE)
    {
        gpe_write_register(GPE_ARIA_DST0_PIC_ADDR, (p_ctx->dst_img.buf) );
        gpe_write_register(GPE_ARIA_DST0_PIC_STRIDE, p_ctx->dst_img.pitch & 0xffff);
    }
    else
    {
        phys_addr_t base_addr;
        base_addr = p_ctx->dst_img.buf + p_ctx->dst_img.pitch * (p_ctx->dst_img.height  - 1);
        gpe_write_register(GPE_ARIA_DST0_PIC_ADDR, base_addr );
        gpe_write_register(GPE_ARIA_DST0_PIC_STRIDE, (p_ctx->dst_img.pitch & 0xffff) | (1 << 16));
    }

    data = (p_ctx->dst_img.rect.y   <<   16) | p_ctx->dst_img.rect.x;
    gpe_write_register(GPE_ARIA_DST0_OP_POS, data);
    data = (p_ctx->dst_img.rect.h   <<   16) | p_ctx->dst_img.rect.w;
    gpe_write_register(GPE_ARIA_DST_OP_SIZE, data);

    if(p_ctx->dst_with_mask)
    {
        gpe_write_register(GPE_ARIA_DST1_FMT_CFG0, 0x3001);
        if(p_ctx->dst_mask.mask2alpha == 2)
            gpe_write_register(GPE_ARIA_DST1_FMT_CFG1, 0x73);
        else
            gpe_write_register(GPE_ARIA_DST1_FMT_CFG1, 0x70);

        if(p_ctx->dst_img.negative_stride == MT_FALSE)
        {
            gpe_write_register(GPE_ARIA_DST1_PIC_STRIDE, p_ctx->dst_mask.mbuf_pitch & 0xffff);
            data = (p_ctx->dst_mask.mbuf_addr) ;
            gpe_write_register(GPE_ARIA_DST1_PIC_ADDR, data);
        }
        else
        {
            gpe_write_register(GPE_ARIA_DST1_PIC_STRIDE, (p_ctx->dst_mask.mbuf_pitch& 0xffff) | (1 << 16));
            data = (p_ctx->dst_mask.mbuf_addr + p_ctx->dst_mask.mbuf_pitch * (p_ctx->dst_img.height - 1)) & 0x1fffffff;
            gpe_write_register(GPE_ARIA_DST1_PIC_ADDR, data);
        }
    }
    else
        gpe_write_register(GPE_ARIA_DST1_FMT_CFG0, 0);

    //src1
    if(p_ctx->src1_sel)
    {
        if(p_ctx->src_img.color_info.little_endian)
        {
            if(p_ctx->src_img.color_info.color_fmt == Y1VY0U)
                swap_mod = 1;
            else if(p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
                swap_mod = 0;
            else
            {
                if(p_ctx->src_img.color_info.bpp == GPE_ARIA_BPP_32BIT)
                    swap_mod = 1;
                else if(p_ctx->src_img.color_info.bpp == GPE_ARIA_BPP_16BIT)
                    swap_mod = 2;
                else
                    swap_mod = 0;
            }
        }
        else
            swap_mod = 0;
        //  GPE_PRINT("\r\n src1 little endian:%d, fmt:%d, bpp:%d, swap_mod:%d", p_ctx->src_img.color_info.little_endian,
        //   p_ctx->src_img.color_info.color_fmt,p_ctx->src_img.color_info.bpp, swap_mod);
        data = (swap_mod << 24);
        if(p_ctx->src1_rgb2yuv_en)
            data |= 2 << 20;
        else if(p_ctx->src1_yuv2rgb_en)
            data |= 3 << 20;
        data |= (p_ctx->src_img.color_info.bpp << 16);
        if((p_ctx->src_img.color_info.is_pix_alpha) || (p_ctx->src_img.with_palette))
            data |= (p_ctx->src_img.color_info.alpha_ch_en << 8);
        data |= (mt_u32)((p_ctx->src_img.with_palette && p_ctx->src1_palt_load_en) << 4);
        if(p_ctx->src_img_en)
            data |= 1;

        //bit swap clut1/clut2/clut4 to keep consistent with concerto
        if(p_ctx->src_img.color_info.color_fmt == CLUT_1)
            data |= (3 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_2)
            data |= (2 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_4)
            data |= (1 << 28);

#ifdef CONFIG_MT_FPGA_GPE
      if(g_debug.src1_bitswap == 1)
      {
        if(p_ctx->src_img.color_info.color_fmt == CLUT_1)
          data |= (3 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_2)
          data |= (2 << 28);
        else if(p_ctx->src_img.color_info.color_fmt == CLUT_4)
          data |= (1 << 28);        
        else if(p_ctx->src_img.color_info.color_fmt == ALUT44)
          data |= (1 << 28);         
      }
      else if(g_debug.src1_bitswap == 0)
        data &= 0x0fffffff;            
#endif
        gpe_write_register(GPE_ARIA_SRC1_FMT_CFG0, data);
    }
    else
    {
        gpe_write_register(GPE_ARIA_SRC1_FMT_CFG0, 0);
    }
    if((p_ctx->src1_sel)  &&  (p_ctx->src_img_en))
    {
        data = (p_ctx->src_img.height   <<   16) | p_ctx->src_img.width;
        gpe_write_register(GPE_ARIA_SRC1_PIC_SIZE, data);
        if(p_ctx->src_img.negative_stride == MT_FALSE)
        {
            gpe_write_register(GPE_ARIA_SRC1_PIC_ADDR, (p_ctx->src_img.buf) );
            if(p_ctx->src_is_xylc)
                gpe_write_register(GPE_ARIA_SRC1_PIC_STRIDE, 0x4000);
            else
                gpe_write_register(GPE_ARIA_SRC1_PIC_STRIDE, p_ctx->src_img.pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            base_addr = p_ctx->src_img.buf + p_ctx->src_img.pitch * (p_ctx->src_img.height  - 1);
            gpe_write_register(GPE_ARIA_SRC1_PIC_ADDR, base_addr );
            if(p_ctx->src_is_xylc)
                gpe_write_register(GPE_ARIA_SRC1_PIC_STRIDE, 0x40000);
            else
                gpe_write_register(GPE_ARIA_SRC1_PIC_STRIDE, (p_ctx->src_img.pitch & 0xffff) | (1 << 16));
        }
#ifdef FAST_2D_SCALE
        data = 0;
#ifdef CONFIG_MT_FPGA_GPE
        if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0)
#else
         if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0
                && p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w
                && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif                
        {
#if 0
            if(p_ctx->src_img.rect.x && (p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w))
            {
                data = p_ctx->scale_cfg.init_phase_x / 32 + p_ctx->src_img.rect.x - 1;
            }
            else
            {
                data = p_ctx->src_img.rect.x;
            }

            if(p_ctx->src_img.rect.y && (p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h))
            {
                data |= (p_ctx->scale_cfg.init_phase_y / 32 + p_ctx->src_img.rect.y - 1) << 16;
            }
            else
            {
                data |= (p_ctx->src_img.rect.y   <<   16);
            }
#else 

            if(p_ctx->src_img.rect.x && (p_ctx->src_img.rect.w < p_ctx->dst_img.rect.w))
            {
                data = p_ctx->scale_cfg.init_phase_x / 32 + p_ctx->src_img.rect.x - 1;
            }
            else
            {
                data = p_ctx->src_img.rect.x;
            }

            if(p_ctx->src_img.rect.y && (p_ctx->src_img.rect.h < p_ctx->dst_img.rect.h))
            {
                data |= (p_ctx->scale_cfg.init_phase_y / 32 + p_ctx->src_img.rect.y - 1) << 16;
            }
            else
            {
                data |= p_ctx->src_img.rect.y<<16; 
            }
#endif
        }
        else
        {
            data = (p_ctx->src_img.rect.y   <<   16) | p_ctx->src_img.rect.x;
        }
#else
        data = (p_ctx->src_img.rect.y   <<   16) | p_ctx->src_img.rect.x;
#endif
        gpe_write_register(GPE_ARIA_SRC1_OP_POS, data);
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
        if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0)
#else
    if(p_ctx->scale_en && p_ctx->scale_cfg.coef[5] == 0
            && p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w
            && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)

#endif
        {
            mt_u32 x, y, w, h;
            data = gpe_read_register(GPE_ARIA_SRC1_OP_POS);
            x = data & 0xffff;
            y = (data >> 16) & 0xffff;
            if(x + p_ctx->src_img.rect.w > p_ctx->src_img.width)
                w = p_ctx->src_img.width - x;
            else
                w = p_ctx->src_img.rect.w;
            if(p_ctx->dst_img.rect.w >= w && (w % 64 == 0) && ((x+w) < p_ctx->src_img.width))
                w = w + 1;
            if(y + p_ctx->src_img.rect.h > p_ctx->src_img.height)
                h = p_ctx->src_img.height - y;
            else
                h = p_ctx->src_img.rect.h;
            data = (h   <<   16) | w;
        }
        else
            data = (p_ctx->src_img.rect.h   <<   16) | p_ctx->src_img.rect.w;
#else
        data = (p_ctx->src_img.rect.h   <<   16) | p_ctx->src_img.rect.w;
#endif
        gpe_write_register(GPE_ARIA_SRC1_OP_SIZE, data);
#if 0
        if(p_ctx->src_img.color_info.color_fmt == AYUV8888)
            data = ARGB8888;
        else if(p_ctx->src_img.color_info.color_fmt == YUVA8888)
            data = RGBA8888;
        else
#endif
            if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
                data = SP_YUV422_Y;
            else
                data = p_ctx->src_img.color_info.color_fmt & 0x7f;
        //delete this workaround for bug 104326
#if 0
        //workaround for bug 84958
        data |= (0x1333 << 16) | (0 << 8);
        gpe_write_register(GPE_ARIA_SRC1_KEY_MIN, 0);
        gpe_write_register(GPE_ARIA_SRC1_KEY_MAX, 0);
#endif
        if(p_ctx->src_img.ck_en)
        {
            data = p_ctx->src_img.color_info.color_fmt & 0x7f;
            data |= (p_ctx->src_img.ck_mod << 16) | (p_ctx->src_img.ck_select << 8);
            gpe_write_register(GPE_ARIA_SRC1_KEY_MIN, p_ctx->src_img.ck_min);
            gpe_write_register(GPE_ARIA_SRC1_KEY_MAX, p_ctx->src_img.ck_max);
        }
        //exp mode
        data |= (p_ctx->color_exp_mode << 12) | (p_ctx->color_exp_mode << 14);
        gpe_write_register(GPE_ARIA_SRC1_FMT_CFG1, data);
        if((p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
                || (p_ctx->src_img.color_info.color_fmt == CMYK8888))
        {
            data = 0x1 | ((p_ctx->cmyk_max & 0xff) << 8) | (p_ctx->cmyk_coef << 16);
            gpe_write_register(GPE_ARIA_SRC1_CMYK_CFG, data);
        }
    }

    //src0
    if((p_ctx->src_with_mask)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
            || (p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
            || (p_ctx->src_img.color_info.color_fmt == TILE_Y)
            || (p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM))
    {
        if((p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
                || (p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
                || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2))
        {
            data = (0x4 << 16) | 0x1;
        }
        else
        {
            data = (0x3 << 16) | 0x1;
        }
        if(p_ctx->src_img.color_info.little_endian)
            data |= (0x2 << 24);
        gpe_write_register(GPE_ARIA_SRC0_FMT_CFG0, data);

        if(p_ctx->src_with_mask)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, GRAY_8);
        else if(p_ctx->src_img.color_info.color_fmt == TILE_Y)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, TILE_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV444_Y)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, SP_YUV444_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, SP_YUV422_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, SP_YUV420_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, SP_YUV420_C);
        else if(p_ctx->src_img.color_info.color_fmt == SP_CMYK8888_CM)
            gpe_write_register(GPE_ARIA_SRC0_FMT_CFG1, SP_CMYK8888_YK);
        if(p_ctx->src_img.negative_stride == MT_FALSE)
        {
            gpe_write_register(GPE_ARIA_SRC0_PIC_ADDR, p_ctx->src0_buf );
            gpe_write_register(GPE_ARIA_SRC0_PIC_STRIDE, p_ctx->src0_pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            if((p_ctx->src_img.color_info.color_fmt == SP_YUV420_Y)
                    || (p_ctx->src_img.color_info.color_fmt == SP_YUV422_Y2))
                base_addr = p_ctx->src0_buf + p_ctx->src0_pitch * (p_ctx->src_img.height / 2  - 1);
            else
                base_addr = p_ctx->src0_buf + p_ctx->src0_pitch * (p_ctx->src_img.height  - 1);
            gpe_write_register(GPE_ARIA_SRC0_PIC_ADDR, base_addr );
            gpe_write_register(GPE_ARIA_SRC0_PIC_STRIDE, (p_ctx->src0_pitch & 0xffff) | (1 << 16));
        }
    }

    //src2
    if(p_ctx->src2_sel)
    {
        gpe_img_t *p_img = NULL;

        if(p_ctx->bg_img_en)
            p_img = &(p_ctx->bg_img);
        else
            p_img = &(p_ctx->dst_img);

        if(p_img->color_info.little_endian)
        {
            if(p_img->color_info.color_fmt == Y1VY0U)
                swap_mod = 1;
            else
            {
                if(p_img->color_info.bpp == GPE_ARIA_BPP_32BIT)
                    swap_mod = 1;
                else if(p_img->color_info.bpp == GPE_ARIA_BPP_16BIT)
                    swap_mod = 2;
                else
                    swap_mod = 0;
            }
        }
        else
            swap_mod = 0;

        data = (swap_mod << 24);
        if(p_ctx->src2_rgb2yuv_en)
            data |= 2 << 20;
        else if(p_ctx->src2_yuv2rgb_en)
            data |= 3 << 20;
        data |= (p_img->color_info.bpp << 16);
        if((p_img->color_info.is_pix_alpha) ||(p_img->with_palette))
            data |= (p_img->color_info.alpha_ch_en << 8);
        data |= 1;
        gpe_write_register(GPE_ARIA_SRC2_FMT_CFG0, data);

        if(p_img->negative_stride == MT_FALSE)
        {
            gpe_write_register(GPE_ARIA_SRC2_PIC_ADDR, (p_img->buf) );
            gpe_write_register(GPE_ARIA_SRC2_PIC_STRIDE, p_img->pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            base_addr = p_img->buf + p_img->pitch * (p_img->height  - 1);
            gpe_write_register(GPE_ARIA_SRC2_PIC_ADDR, base_addr );
            gpe_write_register(GPE_ARIA_SRC2_PIC_STRIDE, (p_img->pitch & 0xffff) | (1 << 16));
        }
        data = (p_img->rect.y << 16) | p_img->rect.x;
        gpe_write_register(GPE_ARIA_SRC2_OP_POS, data);
#if 0
        if(p_img->color_info.color_fmt == AYUV8888)
            data = ARGB8888;
        else if(p_img->color_info.color_fmt == YUVA8888)
            data = RGBA8888;
        else
#endif
            data = p_img->color_info.color_fmt & 0x7f;
        MT_INFO_TDE("\r\n p_ctx->dst_img.ck_select :%d", p_ctx->dst_img.ck_select );
        if(p_ctx->dst_img.ck_en)
        {
            data |= (p_ctx->dst_img.ck_mod << 16) | (p_ctx->dst_img.ck_select << 8);
            gpe_write_register(GPE_ARIA_SRC2_KEY_MIN, p_ctx->dst_img.ck_min);
            gpe_write_register(GPE_ARIA_SRC2_KEY_MAX, p_ctx->dst_img.ck_max);
        }
        //exp mode
        data |= (p_ctx->color_exp_mode << 12) | (p_ctx->color_exp_mode << 14);
        gpe_write_register(GPE_ARIA_SRC2_FMT_CFG1, data);
    }
    else
        gpe_write_register(GPE_ARIA_SRC2_FMT_CFG0, 0);

    //src3
    if(p_ctx->src3_sel)
    {
        if(p_ctx->ex_img.color_info.little_endian)
        {
            if(p_ctx->ex_img.color_info.color_fmt == Y1VY0U)
                swap_mod = 1;
            else if(p_ctx->ex_img.color_info.color_fmt == SP_YUV444_Y)
                swap_mod = 0;
            else
            {
                if(p_ctx->ex_img.color_info.bpp == GPE_ARIA_BPP_32BIT)
                    swap_mod = 1;
                else if(p_ctx->ex_img.color_info.bpp == GPE_ARIA_BPP_16BIT)
                    swap_mod = 2;
                else
                    swap_mod = 0;
            }
        }
        else
            swap_mod = 0;
        data = (swap_mod << 24);
        if(p_ctx->src3_rgb2yuv_en)
            data |= 2 << 20;
        else if(p_ctx->src3_yuv2rgb_en)
            data |= 3 << 20;
        data |= (p_ctx->ex_img.color_info.bpp << 16);
        if((p_ctx->ex_img.color_info.is_pix_alpha) || (p_ctx->ex_img.with_palette))
            data |= (p_ctx->ex_img.color_info.alpha_ch_en << 8);
        data |= (mt_u32)((p_ctx->ex_img.with_palette && p_ctx->src3_palt_load_en) << 4);
        data |= 1;
        //bit swap clut1/clut2/clut4 to keep consistent with concerto
        if(p_ctx->ex_img.color_info.color_fmt == CLUT_1)
            data |= (3 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_2)
            data |= (2 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_4)
            data |= (1 << 28);
#ifdef CONFIG_MT_FPGA_GPE
      if(g_debug.src3_bitswap == 1)
      {
        if(p_ctx->ex_img.color_info.color_fmt == CLUT_1)
          data |= (3 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_2)
          data |= (2 << 28);
        else if(p_ctx->ex_img.color_info.color_fmt == CLUT_4)
          data |= (1 << 28);        
        else if(p_ctx->ex_img.color_info.color_fmt == ALUT44)
          data |= (1 << 28);         
      }
      else if(g_debug.src3_bitswap == 0)
        data &= 0x0fffffff;            
#endif 
        gpe_write_register(GPE_ARIA_SRC3_FMT_CFG0, data);

        if(p_ctx->ex_img.negative_stride == MT_FALSE)
        {
            gpe_write_register(GPE_ARIA_SRC3_PIC_ADDR, (p_ctx->ex_img.buf) );
            gpe_write_register(GPE_ARIA_SRC3_PIC_STRIDE, p_ctx->ex_img.pitch & 0xffff);
        }
        else
        {
            phys_addr_t base_addr;
            base_addr = p_ctx->ex_img.buf + p_ctx->ex_img.pitch * (p_ctx->ex_img.height  - 1);
            gpe_write_register(GPE_ARIA_SRC3_PIC_ADDR, base_addr);
            gpe_write_register(GPE_ARIA_SRC3_PIC_STRIDE, (p_ctx->ex_img.pitch & 0xffff) | (1 << 16));
        }
        data = (p_ctx->ex_img.rect.y   <<   16) | p_ctx->ex_img.rect.x;
        gpe_write_register(GPE_ARIA_SRC3_OP_POS, data);
#if 0
        if(p_ctx->ex_img.color_info.color_fmt == AYUV8888)
            data = ARGB8888;
        else if(p_ctx->ex_img.color_info.color_fmt == YUVA8888)
            data = RGBA8888;
        else
#endif
            data = p_ctx->ex_img.color_info.color_fmt & 0x7f;
        if(p_ctx->ex_img.ck_en)
        {
            data |= (p_ctx->ex_img.ck_mod << 16) | (p_ctx->ex_img.ck_select << 8);
            gpe_write_register(GPE_ARIA_SRC3_KEY_MIN, p_ctx->ex_img.ck_min);
            gpe_write_register(GPE_ARIA_SRC3_KEY_MAX, p_ctx->ex_img.ck_max);
        }
        //exp mode
        data |= (p_ctx->color_exp_mode << 12) | (p_ctx->color_exp_mode << 14);
        gpe_write_register(GPE_ARIA_SRC3_FMT_CFG1, data);
    }
    else
        gpe_write_register(GPE_ARIA_SRC3_FMT_CFG0, 0);

    //palette
    data = 0;
    if(p_ctx->src1_palt_load_en)
    {
        data |= p_ctx->src_img.palt_size & 0x1ff;
        gpe_write_register(GPE_ARIA_PAL1_ADDR, p_ctx->src_img.palt_buf );
    }
    if(p_ctx->src3_palt_load_en)
    {
        data |= (p_ctx->ex_img.palt_size & 0x1ff) << 16;
        gpe_write_register(GPE_ARIA_PAL3_ADDR, p_ctx->ex_img.palt_buf);
    }
    gpe_write_register(GPE_ARIA_PAL_SIZE, data);

    //table load en
    data = (p_ctx->scale_en << 8) | (p_ctx->src3_palt_load_en << 4) | p_ctx->src1_palt_load_en;
    if(p_ctx->src1_palt_load_en)
    {
        if((p_ctx->src_img.palt_format == GPE_ARIA_PALT_ARGB8888)
                || (p_ctx->src_img.palt_format == GPE_ARIA_PALT_AYUV8888))
        {
            if(p_ctx->src_img.palt_little_endian)
                data |= 2 << 16;
        }
        else
        {
            if(p_ctx->src_img.palt_little_endian)
                data |= 3 << 16;
            else
                data |= 1 << 16;
        }
    }
    if(p_ctx->src3_palt_load_en)
    {
        if((p_ctx->ex_img.palt_format == GPE_ARIA_PALT_ARGB8888)
                || (p_ctx->ex_img.palt_format == GPE_ARIA_PALT_AYUV8888))
        {
            if(p_ctx->ex_img.palt_little_endian)
                data |= 2 << 20;
        }
        else
        {
            if(p_ctx->ex_img.palt_little_endian)
                data |= 3 << 20;
            else
                data |= 1 << 20;
        }
    }
    gpe_write_register(GPE_ARIA_LOAD_EN, data);

    //rop
    if(p_ctx->rop_en)
    {
        data = (p_ctx->rop_a_mod   <<   8) | p_ctx->rop_c_mod;
        gpe_write_register(GPE_ARIA_ROP_ID, data);
        gpe_write_register(GPE_ARIA_ROP_PAT, p_ctx->rop_pattern);
    }
    //blend
    if(p_ctx->blend_en)
    {
#if defined(CONFIG_MT_CHIP_SYMPHONY6) 
        if(p_ctx->demultiply_en)
            data = 1 << 16;
        else
#endif      
            data = 0;    
        data |= (p_ctx->dst_blend_fact  <<   4) | p_ctx->src_blend_fact;
 #ifdef CONFIG_MT_FPGA_GPE
        if(g_debug.new_blend_en)
        {    
            data |= (g_debug.comset[1] << 12) | (g_debug.comset[0] << 8);
        }
        else
 #endif
        { 
            data |= (p_ctx->adst_blend_fact  <<   12) | (p_ctx->asrc_blend_fact << 8);
        }
        gpe_write_register(GPE_ARIA_COMP_BLD_MOD, data);
    }

    //rotator
    if(p_ctx->rotator_en)
    {
        rot_pat_cfg = p_ctx->rotator_op & 0x7;
    }

    //xylc
    if(p_ctx->src_is_xylc)
    {
        gpe_write_register(GPE_ARIA_XYLC_CFG, p_ctx->xylc_cfg.xylc_num);
        gpe_write_register(GPE_ARIA_PAT_COLOR, p_ctx->xylc_cfg.xylc_color);
    }

    //tile
    if(p_ctx->src_is_tile)
    {
        //        mpi_memdev_init();
        //		mt_sys_init();
#if defined(CONFIG_MT_CHIP_ARIA)

        mt_sys_read_register(0xffd202e0, &data);
        mt_u32 tile_size = data & 0x3; //col_size_mode
        mt_u32 hd_map_mode = (data >> 12) & 0x1;
        mt_u32 field_flag = (data >> 8) & 0x1;
        mt_u32 tile_cfg = (data >> 16) & 0x3;

        mt_sys_read_register(0xffd202e4, &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP00, data);
        mt_sys_read_register(0xffd202e8, &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP01, data);
        mt_sys_read_register(0xffd202ec, &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP10, data);
        mt_sys_read_register(0xffd202f0, &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP11, data);
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402e0), &data);
        mt_u32 tile_size = data & 0x3; //col_size_mode
        mt_u32 hd_map_mode = (data >> 12) & 0x1;
        mt_u32 field_flag = (data >> 8) & 0x1;
        mt_u32 tile_cfg = (data >> 16) & 0x3;
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402e4), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP00, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402e8), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP01, data);
        
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402ec), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP10, data);
        
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4402f0), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP11, data);
        
#else
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf000204), &data);
        mt_u32 tile_size = (data >> 8) & 0x3; //col_size_mode
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf441090), &data);
        mt_u32 hd_map_mode = data & 0x1;
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf44108c), &data);
        mt_u32 field_flag = data & 0x1;
        mt_u32 tile_cfg = (data >> 16) & 0x3;

        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401c0), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP00, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401c4), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP01, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401c8), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP10, data);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf4401cc), &data);
        gpe_write_register(GPE_ARIA_SRC1_TILE_JMP11, data);

#endif

        //        mpi_memdev_deinit();
        //		mt_sys_deinit();

        data = (hd_map_mode << 8) | (field_flag << 12) | (tile_size << 4) | tile_cfg;
        gpe_write_register(GPE_ARIA_SRC1_TILE_CFG, data);

        gpe_write_register(GPE_ARIA_SRC0_PIC_STRIDE, 4096);
        gpe_write_register(GPE_ARIA_SRC1_PIC_STRIDE, 4096);

        //tile y 8byte swap
        data = (gpe_read_register(GPE_ARIA_SRC1_FMT_CFG0) & 0x00ffffff) | (3 << 24);
        gpe_write_register(GPE_ARIA_SRC1_FMT_CFG0, data);

        //tile uv 2byte swap
        data = (gpe_read_register(GPE_ARIA_SRC0_FMT_CFG0) & 0x00ffffff) | (4 << 24);
        gpe_write_register(GPE_ARIA_SRC0_FMT_CFG0, data);
    }

    //scale
    if(p_ctx->scale_en)
    {
        //scale?????些?????surfaceblit?
        //surface??????surface???
        //???
#ifdef CONFIG_MT_FPGA_GPE
        data = p_ctx->scale_cfg.scale_mod |
            ((p_ctx->scale_cfg.disable_alpha_filter) << 5) |
            ((p_ctx->scale_cfg.disable_color_filter) << 4) |
            (p_ctx->scale_cfg.disable_color_anti_flicker << 12) |
            (p_ctx->scale_cfg.disable_alpha_anti_flicker << 13);
#else
        data = p_ctx->scale_cfg.scale_mod |
            ((p_ctx->scale_cfg.disable_alpha_filter) << 5) |
            ((p_ctx->scale_cfg.disable_color_filter) << 4) |
            (p_ctx->scale_cfg.disable_color_anti_flicker << 12) |
            (p_ctx->scale_cfg.disable_alpha_anti_flicker << 13) |
            (2 << 8);
#endif
        if(p_ctx->scale_cfg.coef[5] == 0) //fast2d scaler h/v filter is split.
        {
            data |= ((p_ctx->scale_cfg.disable_alpha_filter_h) << 29) |
                    ((p_ctx->scale_cfg.disable_color_filter_h) << 28) ;
            MT_INFO_TDE("\r\n scale_cfg:0x%x \r\n ", data);
        }
        MT_INFO_TDE("\r\n scale_cfg:0x%x ", data);
        gpe_write_register(GPE_ARIA_SCALER_CFG, data);
        MT_INFO_TDE("\r\n read scl_cfg:0x%x \n", gpe_read_register(GPE_ARIA_SCALER_CFG));
        gpe_write_register(GPE_ARIA_SCALER_COEF_11, (mt_u32)(p_ctx->scale_cfg.coef[0]));
        gpe_write_register(GPE_ARIA_SCALER_COEF_21, (mt_u32)(p_ctx->scale_cfg.coef[1]));
        gpe_write_register(GPE_ARIA_SCALER_COEF_31, (mt_u32)(p_ctx->scale_cfg.coef[2]));
        gpe_write_register(GPE_ARIA_SCALER_COEF_22, (mt_u32)(p_ctx->scale_cfg.coef[3]));
        gpe_write_register(GPE_ARIA_SCALER_COEF_23, (mt_u32)(p_ctx->scale_cfg.coef[4]));
        if(p_ctx->scale_cfg.coef[5] == 0)
        {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
            if(1)
#else
            if(p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
            {
#if 0                
                data = gpe_read_register(GPE_ARIA_SRC1_OP_POS);
                src_x = data & 0xffff;
                src_y = (data >> 16) & 0xffff;
                if(p_ctx->src_img.rect.x && (src_x != 0) )
                    data = p_ctx->scale_cfg.init_phase_x + 32;
                else
                    data = p_ctx->scale_cfg.init_phase_x;

                if(p_ctx->src_img.rect.y && (src_y != 0) )
                    data |= (p_ctx->scale_cfg.init_phase_y + 32) << 16;
                else
                    data |= p_ctx->scale_cfg.init_phase_y << 16;
                gpe_write_register(GPE_ARIA_FAST_SCALER_INIT_PHASE, data);
#else
                data = 0;
                if(p_ctx->src_img.rect.x && (p_ctx->src_img.rect.w < p_ctx->dst_img.rect.w))
                {
                    data = p_ctx->scale_cfg.init_phase_x + 32;
                }
                else
                {
                    data = p_ctx->scale_cfg.init_phase_x;
                }
                
                if(p_ctx->src_img.rect.y && (p_ctx->src_img.rect.h < p_ctx->dst_img.rect.h))
                {
                    data |= (p_ctx->scale_cfg.init_phase_y + 32) << 16;
                }
                else
                {
                    data |= p_ctx->scale_cfg.init_phase_y << 16;   
                }

                gpe_write_register(GPE_ARIA_FAST_SCALER_INIT_PHASE, data);
#endif
            }
            else
#endif
            {
                gpe_write_register(GPE_ARIA_SCALER_INIT_PHASE, p_ctx->scale_cfg.init_phase | (1 << 16));
            }
        }
        else
            gpe_write_register(GPE_ARIA_SCALER_INIT_PHASE, p_ctx->scale_cfg.init_phase);
        data = (mt_u32)u32CoeffPhyAddr;
        //        gpe_aria_flush_buf((void *)data,sizeof(gpe_scale_coeff));
        gpe_write_register(GPE_ARIA_COEF_ADDR, data);

        //to improve performance
        if(p_ctx->scale_cfg.scale_mod == SCALE_VERT_BLK_OUT)
            gpe_write_register(GPE_ARIA_GRA_REQ_CFG, 0x40000cc);


        if((ChipVersion >= MT_CHIP_SYMPHONY_A1)
                && (p_ctx->src_is_tile == MT_FALSE))
        {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
            if(p_ctx->scale_cfg.coef[5] == 0)
#else
            if(p_ctx->scale_cfg.coef[5] == 0
                    && p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w
                    && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
            {
                data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
                data &= 0xf0ffff00;
                data |= 0x020000cc;
                gpe_write_register(GPE_ARIA_GRA_REQ_CFG, data);
            }
            else
#endif
            {
                if(p_ctx->scale_cfg.scale_mod == SCALE_HORI_LINE_OUT)
                {
                    data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
                    data &= 0xf0ffff00;
                    data |= 0x020000cc;
                    gpe_write_register(GPE_ARIA_GRA_REQ_CFG, data);
                }
                else
                {
                    data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
                    data &= 0xf0ffff00;
                    data |= 0x040000cc;
                    gpe_write_register(GPE_ARIA_GRA_REQ_CFG, data);
                }
            }
        }
    }
    else
    {
        if((ChipVersion >= MT_CHIP_SYMPHONY_A1)
                && (p_ctx->src_is_tile == MT_FALSE))
        {
            data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
            data &= 0xf0000000;
            data |= 0x02cccccc;
            gpe_write_register(GPE_ARIA_GRA_REQ_CFG, data);
        }
    }
    if((ChipVersion >= MT_CHIP_SYMPHONY_A1)
            && (p_ctx->src_is_tile == MT_FALSE))
    {
        data = gpe_read_register(GPE_ARIA_GRA_PIN_SEL);
        data |= 0x6;
        gpe_write_register(GPE_ARIA_GRA_PIN_SEL, data);
    }
#ifdef CONFIG_MT_FPGA_GPE
    //data = gpe_read_register(GPE_ARIA_GRA_REQ_CFG);
    //data &= 0xfff00000;
    //data |= 0xccccc;
    //gpe_write_register(GPE_ARIA_GRA_REQ_CFG, data);
#endif
    //blur
    if(p_ctx->gaussian_blur_en)
    {
        data = SCALE_HORI_LINE_OUT | (p_ctx->blur_cfg.blur_tap << 20) |
            (p_ctx->gaussian_blur_en << 16);
        gpe_write_register(GPE_ARIA_SCALER_CFG, data);
    }

    //paint
    if(p_ctx->paint_en)
    {
        gpe_write_register(GPE_ARIA_PAT_COLOR, p_ctx->paint.paint_color);

        if(p_ctx->paint_pattern_en)
        {
            data = p_ctx->paint.pat_beg.x | (p_ctx->paint.pat_beg.y << 16);
            gpe_write_register(GPE_ARIA_PAT_OFFSET_POS, data);

            rot_pat_cfg |= (p_ctx->paint.tiling_mod & 0x3) << 4;

            if((p_ctx->paint.tiling_mod == GPE_ARIA_TILE_REPEAT)
                    || (p_ctx->paint.tiling_mod == GPE_ARIA_TILE_REFLECT))
            {
                data = p_ctx->paint.pat_remd1 | (p_ctx->paint.pat_quot1 << 16);
                gpe_write_register(GPE_ARIA_PAT_RATIO_X_0, data);
                data = p_ctx->paint.pat_remd2 | (p_ctx->paint.pat_quot2 << 16);
                gpe_write_register(GPE_ARIA_PAT_RATIO_X_1, data);
                data = p_ctx->paint.pat_remd3 | (p_ctx->paint.pat_quot3 << 16);
                gpe_write_register(GPE_ARIA_PAT_RATIO_Y_0, data);
                data = p_ctx->paint.pat_remd4 | (p_ctx->paint.pat_quot4 << 16);
                gpe_write_register(GPE_ARIA_PAT_RATIO_Y_1, data);
            }
        }
        else
        {
            if(p_ctx->paint.gradt_type == GPE_ARIA_LINER_GRADT)
                data = (p_ctx->paint.mask_mod << 4) | (0 << 3) | (0 << 2) | (p_ctx->paint.spread_mod & 0x3);
            else
            {
                if((p_ctx->paint.center.x == p_ctx->paint.focus.x) && (p_ctx->paint.center.y == p_ctx->paint.focus.y))
                {
                    data = (p_ctx->paint.mask_mod << 4) | (0 << 3) | (1 << 2) | (p_ctx->paint.spread_mod & 0x3);
                }
                else
                {
                    data = (p_ctx->paint.mask_mod << 4) | (1 << 3) | (1 << 2) | (p_ctx->paint.spread_mod & 0x3);
                }
            }
            gpe_write_register(GPE_ARIA_GRADT_CFG, data);

            if(p_ctx->paint.true_liner_gradt)
            {
                aria_gpe_set_gradt_regs(&(p_ctx->paint));
            }
            if(p_ctx->paint.true_liner_gradt  || (p_ctx->paint.gradt_type == GPE_ARIA_RADIAL_GRADT) || (p_ctx->paint.gradt_type == GPE_ARIA_ELLIPSE_GRADT))
            {
                aria_gpe_set_gradt_stop_regs(&(p_ctx->paint));
            }
            if((p_ctx->paint.center.x == p_ctx->paint.focus.x) && (p_ctx->paint.center.y == p_ctx->paint.focus.y))
            {
                aria_gpe_set_gradt_radius_regs(&(p_ctx->paint));
            }
        }
    }

    //rot_pat_cfg
    gpe_write_register(GPE_ARIA_ROT_PAT_CFG, rot_pat_cfg);
    if(p_ctx->comp_en)
    {
        mt_u32 src1_mult_mod = 0;
        mt_u32 src3_mult_mod = 0;

        if(p_ctx->is_draw_multiply)
        {
            src1_mult_mod = 0x1;
        }
        else if(p_ctx->alpha_map_en)
        {
            if(p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_MIX_NORMAL)
            {
                src1_mult_mod = 0x2;
            }
            else if(p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_MIX_EX)
            {
                src1_mult_mod = 0x3;
            }
        }
        //      GPE_PRINT("\r\n is_draw_stencil:%d", p_ctx->is_draw_stencil);
        if(p_ctx->is_draw_stencil)
        {
            src3_mult_mod = 0x2;
        }
        data = (src3_mult_mod << 28) | (src1_mult_mod << 12);
        if(p_ctx->src_img.plane_alpha_en)
            data |= (1 << 8) | (p_ctx->src_img.plane_alpha & 0xff);
        if(p_ctx->ex_img.plane_alpha_en)
            data |= (1 << 24) | ((p_ctx->ex_img.plane_alpha & 0xff) << 16);
        data |= (p_ctx->src_img.color_info.alpha_pre_mult_en<< 9)
            | (p_ctx->ex_img.color_info.alpha_pre_mult_en << 25);
        gpe_write_register(GPE_ARIA_COMP_MULT_MOD, data);
        //  GPE_PRINT("\r\n data:0x%08x, reg:0x%08x", data, gpe_read_register(GPE_ARIA_COMP_MULT_MOD));

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
        if(p_ctx->src2_sel)
        {
            gpe_img_t *p_img = NULL;

            if(p_ctx->bg_img_en)
                p_img = &(p_ctx->bg_img);
            else
                p_img = &(p_ctx->dst_img);
            data = 0;
            if(p_img->color_info.alpha_pre_mult_en)
                data = 1 << 9;       
            if(p_img->plane_alpha_en)
                data |= p_img->plane_alpha | (1 << 8);

            gpe_write_register(GPE_SPN_COMP_MULT_MOD2, data);
          //  GPE_PRINT("\r\n GPE_SPN_COMP_MULT_MOD2:%x", data);
        }
#endif

        if(p_ctx->blend_en)
            data = 1;
        else
            data = 0;
        if(p_ctx->alpha_map_en && (p_ctx->alpha_map_mod == GPE_CCT_ALPHA_MAP_LOGICAL))
            data |= (1 << 8);
        if(p_ctx->comp_key_set)
            data |= (1 << 4) | (1 << 12);
        data |= (p_ctx->src1_sel << 16);
        data |= p_ctx->comp_bp << 24;
        gpe_write_register(GPE_ARIA_COMP_CFG, data);
    }

    //delete this workaround for bug 104326
#if 0
    //workaround for bug 84958
    data = gpe_read_register(GPE_ARIA_SRC1_FMT_CFG1);
    if( ((data & 0xffff0000) == 0x13330000) &&
            (gpe_read_register(GPE_ARIA_SRC1_KEY_MIN) == 0) &&
            (gpe_read_register(GPE_ARIA_SRC1_KEY_MAX) == 0) )
    {
        data = gpe_read_register(GPE_ARIA_DST0_FMT_CFG0);
        data |= 3 << 12;  //all data write to memory
        gpe_write_register(GPE_ARIA_DST0_FMT_CFG0, data);
    }
#endif
    //yuv to rgb enable
    aria_yuv_to_rgb_enable_check(p_ctx);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(p_ctx->clip_en)
    { 
        gpe_write_register(GPE_SPN_REGION_CFG, p_ctx->clip.clip_mode);  
        data = p_ctx->clip.clip_rect.s32Xpos| (p_ctx->clip.clip_rect.s32Ypos<< 16);
        gpe_write_register(GPE_SPN_REGION_START, data);  
        data = (p_ctx->clip.clip_rect.s32Xpos + p_ctx->clip.clip_rect.u32Width- 1) 
            | ((p_ctx->clip.clip_rect.s32Ypos+ p_ctx->clip.clip_rect.u32Height- 1) << 16);
        gpe_write_register(GPE_SPN_REGION_STOP, data); 
        GPE_PRINT("\r\n clip %08x,%08x,%08x",p_ctx->clip.clip_mode,p_ctx->clip.clip_rect.s32Xpos | (p_ctx->clip.clip_rect.s32Ypos << 16),
                (p_ctx->clip.clip_rect.s32Xpos + p_ctx->clip.clip_rect.u32Width - 1) 
                | ((p_ctx->clip.clip_rect.s32Ypos + p_ctx->clip.clip_rect.u32Height - 1) << 16));
        GPE_PRINT("\r\n clip enable, [%d,%d,%d,%d] GPE_SPN_REGION_CFG:%08x,GPE_SPN_REGION_START:%08x,GPE_SPN_REGION_STOP:%08x", 
                p_ctx->clip.clip_rect.s32Xpos ,p_ctx->clip.clip_rect.s32Ypos,p_ctx->clip.clip_rect.u32Width , p_ctx->clip.clip_rect.u32Height,
                gpe_read_register(GPE_SPN_REGION_CFG),gpe_read_register(GPE_SPN_REGION_START),gpe_read_register(GPE_SPN_REGION_STOP));

    }

    if(p_ctx->colorize_en)
    {
        gpe_write_register(GPE_SPN_COMP_COLORIZE, (p_ctx->color & 0x00ffffff) | (1 << 24));    
        
        GPE_PRINT("\r\n colorize:%x:%x:%d", gpe_read_register(GPE_SPN_COMP_COLORIZE),(p_ctx->color & 0x00ffffff) | (1 << 24),p_ctx->colorize_en);
    }
    else
        gpe_write_register(GPE_SPN_COMP_COLORIZE, 0);
//    GPE_PRINT("\r\n colorize:%x:%x:%d", gpe_read_register(GPE_SPN_COMP_COLORIZE),(p_ctx->color & 0x00ffffff) | (1 << 24),p_ctx->colorize_en);
#endif

    //  GPE_PRINT("\r\n rotator_en:%d", p_ctx->rotator_en);
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
    if(1)
#else
    if(p_ctx->src_img.rect.w != p_ctx->dst_img.rect.w && p_ctx->src_img.rect.h != p_ctx->dst_img.rect.h)
#endif
    {
        data = 0;
        if(p_ctx->scale_en)
        {
            if(p_ctx->scale_cfg.coef[5] == 0)
                data |= (1 << 28);
            else
                data |= (1 << 4);
        }

        data |= p_ctx->comp_en
            | (p_ctx->gaussian_blur_en << 4)
            | (p_ctx->rotator_en << 8)
            | (p_ctx->paint_pattern_en << 12)
            | (p_ctx->src_is_xylc << 16)
            | (p_ctx->paint_gradt_en << 20)
            | (p_ctx->clip_en << 24);
        gpe_write_register(GPE_ARIA_GRA_EN, data);
    }
    else
#endif
    {
        data = p_ctx->comp_en
            | ((p_ctx->scale_en | p_ctx->gaussian_blur_en) << 4)
            | (p_ctx->rotator_en << 8)
            | (p_ctx->paint_pattern_en << 12)
            | (p_ctx->src_is_xylc << 16)
            | (p_ctx->paint_gradt_en << 20)
            | (p_ctx->clip_en << 24);
        gpe_write_register(GPE_ARIA_GRA_EN, data);
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY6)	
     gpe_write_register(GPE_ARIA_WCH_REQ_MSK_SYNC_MODE , 0x00000000); 
     gpe_write_register(GPE_ARIA_GRA_AXI_CFG,0x00000001);
     if( (p_ctx->src_img_en&& (p_ctx->src_img.with_palette&& p_ctx->src1_palt_load_en)) || (p_ctx->ex_img_en&& (p_ctx->ex_img.with_palette && p_ctx->src3_palt_load_en)))
     {
         gpe_write_register(GPE_ARIA_GRA_AXI_CFG,0x00000000); //clut ->clut ,GPE_ARIA_GRA_AXI_CFG must be set 1
     }
#endif
    //    GPE_PRINT("\r\n GPE_ARIA_GRA_EN:0x%08x", gpe_read_register(GPE_ARIA_GRA_EN));
}
#ifdef CONFIG_MT_FPGA_GPE
static MT_BOOL aria_gpe_command_start(void)
{
    MT_BOOL ret = MT_TRUE;

#ifdef GPE_ARIA_TEST_CYCLE
    mt_u32 cycle = 0;
#endif

    mt_u32 gra_eng_status = 0;
#ifdef CONFIG_MT_FPGA_GPE
  mt_u32 axi_status = 0;
  mt_u32 pause_on = 0;
  mt_u32 reset_on = 0;
  mt_u32 val = 0;
#endif

    //  mt_u32 ticks = 0;
//#ifndef GPE_ARIA_HARDWARE_ISR_ON
    if(!(g_debug.int_mode_en))
    {
        //clear gra done status
        gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
        //start graphic
        gpe_write_register(GPE_ARIA_GRA_ENG_START, 1);
        
        MT_INFO_TDE("\r\n wait mode GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
        // if no define GPE_WARROIRS_CHECK_IDLE_SYNC, not waite for the HW finish
        //  ticks = mt_ticks_get();
        struct timeval t1, t2;    
        gettimeofday(&t1, NULL);
        gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
        //if status equals GPE_HW_CLOSE_PROCESS, means finished
        while(GRA_ALL_DONE != (gra_eng_status & GRA_ALL_DONE))
        {
            gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
            
#ifdef CONFIG_MT_FPGA_GPE
              if(g_debug.pause_on == TRUE)
              { 
                if(pause_on == 0)
                {
                  pause_on = 1;
                  msleep(1);
                  GPE_PRINT("\nGPE_PAUSE on\n");
                  val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
                  val &= 0xFFFFFFCF;
                  val |= 0x30;
                  gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause on
                  axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
                  while(0x10 != (axi_status & 0x10))  //AXI_BUS_EMPTY
                  {
                    axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
                  }
                  msleep(1000);
                  val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
                  val &= 0xFFFFFFCF;
                  gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause off
                  GPE_PRINT("\nGPE_PAUSE off\n");
                }
              }
              if(g_debug.reset_on == TRUE)
              {
                if(reset_on == 0)
                {
                  reset_on = 1;
                  return FALSE;
                }
              }
#endif
            
            // cost too much time, so think the gra eng is error
            gettimeofday(&t2, NULL);
            //      if(10000 < (mt_ticks_get() - ticks))
            MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x,tm:%d ms", gpe_read_register(GPE_ARIA_GRA_STATE),(((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000));
            if(10000 < (((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000))
            {
                ret = MT_FALSE;
                break;
            }
            msleep(100);
            
        }
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_AXI_ATATUS:0x%08x", gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS));
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
        GPE_REG_PRINT("**************reg result *****************\n");
        GPE_REG_PRINT("GPE_ARIA_GRA_INT_EN:0x%08x\n",gpe_read_register(GPE_ARIA_GRA_INT_EN));
        GPE_REG_PRINT("GPE_ARIA_GRA_INT_STATE:0x%08x\n",gpe_read_register(GPE_ARIA_GRA_INT_STATE));
        GPE_REG_PRINT("GPE_ARIA_GRA_STATE:0x%08x\n",gpe_read_register(GPE_ARIA_GRA_STATE));
        //clear gra done status
        gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);

//#else
    }
    else
    {
        MT_INFO_TDE("\r\n int mode GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
        //clear gra done status
        gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
#ifdef GPE_ARIA_HARDWARE_ISR_ON
        //	  ret = ioctl(get_tde_handle(), TDE_SET_FINISH_FLAG);
        gpe_write_register(GPE_ARIA_GRA_INT_EN, GRA_ALL_DONE);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        gpe_write_register(GPE_ARIA_GRA_INT_MOD, 0);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
        gpe_write_register(GPE_ARIA_GRA_INT_MOD, 2);
#endif
#endif

#ifdef CONFIG_MT_FPGA_GPE     
                  if(g_debug.reset_on == TRUE)
                  {
                    if(reset_on == 0)
                    {
                      reset_on = 1;
                      return FALSE;
                    }
                  }
#endif

        ret = ioctl(get_tde_handle(), TDE_WAIT_FINISH);
        if(ret ==  MT_SUCCESS)
            return MT_TRUE;
        else
        {
            printf("<%s> : <%d> : hw timeout Err ret(%d)\n", __FUNCTION__, __LINE__, ret);
            return MT_FALSE;
        }
    }
//#endif

    return ret;
}
#else
static MT_BOOL aria_gpe_command_start(void)
{
    MT_BOOL ret = MT_TRUE;

#ifdef GPE_ARIA_TEST_CYCLE
    mt_u32 cycle = 0;
#endif

#ifdef CONFIG_MT_FPGA_GPE
  mt_u32 axi_status = 0;
  mt_u32 pause_on = 0;
  mt_u32 reset_on = 0;
  mt_u32 val = 0;
#endif

    //  mt_u32 ticks = 0;

#ifdef GPE_ARIA_HARDWARE_ISR_ON
    //	  ret = ioctl(get_tde_handle(), TDE_SET_FINISH_FLAG);
    gpe_write_register(GPE_ARIA_GRA_INT_EN, GRA_ALL_DONE);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 0);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 2);
#endif
#endif
    //clear gra done status
    gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
#ifndef GPE_ARIA_HARDWARE_ISR_ON
    //start graphic
    gpe_write_register(GPE_ARIA_GRA_ENG_START, 1);
    
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
    // if no define GPE_WARROIRS_CHECK_IDLE_SYNC, not waite for the HW finish
    //  ticks = mt_ticks_get();
    struct timeval t1, t2;        
    mt_u32 gra_eng_status = 0;
    gettimeofday(&t1, NULL);
    gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
    //if status equals GPE_HW_CLOSE_PROCESS, means finished
    while(GRA_ALL_DONE != (gra_eng_status & GRA_ALL_DONE))
    {
        gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
        
#ifdef CONFIG_MT_FPGA_GPE
          if(g_debug.pause_on == TRUE)
          { 
            if(pause_on == 0)
            {
              pause_on = 1;
              msleep(1);
              GPE_PRINT("\nGPE_PAUSE on\n");
              val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
              val &= 0xFFFFFFCF;
              val |= 0x30;
              gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause on
              axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
              while(0x10 != (axi_status & 0x10))  //AXI_BUS_EMPTY
              {
                axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
              }
              msleep(1000);
              val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
              val &= 0xFFFFFFCF;
              gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause off
              GPE_PRINT("\nGPE_PAUSE off\n");
            }
          }
          if(g_debug.reset_on == TRUE)
          {
            if(reset_on == 0)
            {
              reset_on = 1;
              return FALSE;
            }
          }
#endif
        
        // cost too much time, so think the gra eng is error
        gettimeofday(&t2, NULL);
        //      if(10000 < (mt_ticks_get() - ticks))
        MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x,tm:%d ms", gpe_read_register(GPE_ARIA_GRA_STATE),(((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000));
        if(10000 < (((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000))
        {
            ret = MT_FALSE;
            break;
        }
        msleep(100);
        
    }
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_AXI_ATATUS:0x%08x", gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS));
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
    GPE_REG_PRINT("**************reg result *****************\n");
    GPE_REG_PRINT("GPE_ARIA_GRA_INT_EN:0x%08x\n",gpe_read_register(GPE_ARIA_GRA_INT_EN));
    GPE_REG_PRINT("GPE_ARIA_GRA_INT_STATE:0x%08x\n",gpe_read_register(GPE_ARIA_GRA_INT_STATE));
    GPE_REG_PRINT("GPE_ARIA_GRA_STATE:0x%08x\n",gpe_read_register(GPE_ARIA_GRA_STATE));
    //clear gra done status
    gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);

#else
#ifdef CONFIG_MT_FPGA_GPE     
              if(g_debug.reset_on == TRUE)
              {
                if(reset_on == 0)
                {
                  reset_on = 1;
                  return FALSE;
                }
              }
#endif

    ret = ioctl(get_tde_handle(), TDE_WAIT_FINISH);
    if(ret ==  MT_SUCCESS)
        return MT_TRUE;
    else
    {
        printf("<%s> : <%d> : hw timeout Err ret(%d)\n", __FUNCTION__, __LINE__, ret);
        return MT_FALSE;
    }
#endif

    return ret;
}
#endif


#if 0
static MT_BOOL aria_gpe_command_start(void)
{
    MT_BOOL ret = MT_TRUE;

#ifdef GPE_ARIA_TEST_CYCLE
    mt_u32 cycle = 0;
#endif

    mt_u32 gra_eng_status = 0;
#ifdef CONFIG_MT_FPGA_GPE
  mt_u32 axi_status = 0;
  mt_u32 pause_on = 0;
  mt_u32 reset_on = 0;
  mt_u32 val = 0;
#endif

    //  mt_u32 ticks = 0;
    struct timeval t1, t2;

#ifdef GPE_ARIA_HARDWARE_ISR_ON
    //	  ret = ioctl(get_tde_handle(), TDE_SET_FINISH_FLAG);
    gpe_write_register(GPE_ARIA_GRA_INT_EN, GRA_ALL_DONE);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 0);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    gpe_write_register(GPE_ARIA_GRA_INT_MOD, 2);
#endif
#endif
    //clear gra done status
    gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
#ifndef GPE_ARIA_HARDWARE_ISR_ON
    //start graphic
    gpe_write_register(GPE_ARIA_GRA_ENG_START, 1);
#endif

#ifndef GPE_ARIA_HARDWARE_ISR_ON
    // if no define GPE_WARROIRS_CHECK_IDLE_SYNC, not waite for the HW finish
    //  ticks = mt_ticks_get();
    gettimeofday(&t1, NULL);
    gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
    //if status equals GPE_HW_CLOSE_PROCESS, means finished
    while(GRA_ALL_DONE != (gra_eng_status & GRA_ALL_DONE))
    {
        gra_eng_status = gpe_read_register(GPE_ARIA_GRA_STATE);
        
#ifdef CONFIG_MT_FPGA_GPE
          if(g_debug.pause_on == TRUE)
          { 
            if(pause_on == 0)
            {
              pause_on = 1;
              msleep(1);
              GPE_PRINT("\nGPE_PAUSE on\n");
              val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
              val &= 0xFFFFFFCF;
              val |= 0x30;
              gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause on
              axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
              while(0x10 != (axi_status & 0x10))  //AXI_BUS_EMPTY
              {
                axi_status = gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS);
              }
              msleep(1000);
              val = gpe_read_register(GPE_ARIA_GRA_AXI_CTRL);
              val &= 0xFFFFFFCF;
              gpe_write_register(GPE_ARIA_GRA_AXI_CTRL, val); //pause off
              GPE_PRINT("\nGPE_PAUSE off\n");
            }
          }
          if(g_debug.reset_on == TRUE)
          {
            if(reset_on == 0)
            {
              reset_on = 1;
              return FALSE;
            }
          }
#endif
        // cost too much time, so think the gra eng is error
        gettimeofday(&t2, NULL);
        //      if(10000 < (mt_ticks_get() - ticks))
        if(10000 < (((t2.tv_sec*1000000 + t2.tv_usec) - (t1.tv_sec*1000000 + t1.tv_usec))/1000))
        {
            ret = MT_FALSE;

            break;
        }
    }
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_AXI_ATATUS:0x%08x", gpe_read_register(GPE_ARIA_GRA_AXI_ATATUS));
    MT_INFO_TDE("\r\n GPE_ARIA_GRA_STATE:0x%08x", gpe_read_register(GPE_ARIA_GRA_STATE));
    //clear gra done status
    gpe_write_register(GPE_ARIA_GRA_STATE, GRA_ALL_DONE);
#else
#ifdef CONFIG_MT_FPGA_GPE     
              if(g_debug.reset_on == TRUE)
              {
                if(reset_on == 0)
                {
                  reset_on = 1;
                  return FALSE;
                }
              }
#endif

    ret = ioctl(get_tde_handle(), TDE_WAIT_FINISH);
    if(ret ==  MT_SUCCESS)
        return MT_TRUE;
    else
    {
        printf("<%s> : <%d> : hw timeout Err ret(%d)\n", __FUNCTION__, __LINE__, ret);
        return MT_FALSE;
    }
#endif

    return ret;
}
#endif 

static MT_BOOL aria_gpe_start(TDE_HANDLE s32Handle)
{
    MT_BOOL ret = MT_TRUE;
    hdl_gpe_t *p_hdl_gpe = &g_hdl_gpe[s32Handle - 1];
    aria_gpe_context_t *p_ctx = NULL;
    mt_u32 data = 0;

    p_ctx = &(p_hdl_gpe->gpe_ctx);
    MT_ASSERT(NULL != p_ctx);

#ifdef CONFIG_MT_FPGA_GPE  
     MT_BOOL comp_ret = TRUE;
     p_ctx->demultiply_en = g_debug.demultiply_en;
#endif
#ifdef SUPPORT_CMDFIFO
    if(p_ctx->is_cmdfifo == MT_TRUE)
    {
        spn_cmdfifo_write_node(p_ctx, p_ctx->p_node);
        return MT_TRUE;
    }
#endif

#ifdef CONFIG_MT_FPGA_GPE
        if(g_debug.compare_with_sw == TRUE)
        {
            aria_gpe_print_context(s32Handle);
            gpe_core(p_ctx, TRUE); //sw gpe, dst is dst_buf2, and it will be compared with hw result which is in screen buffer
        }
        else if(g_debug.show_sw_result == TRUE)        
            gpe_core(p_ctx, FALSE); //sw gpe, dst is the screen buffer
#endif

#ifdef CONFIG_MT_FPGA_GPE
        if(g_debug.show_sw_result == FALSE)
#endif
    {
        //init gra
        aria_gpe_init_reg(MT_FALSE);

        //set regisiter
        aria_gpe_set_regs(p_ctx);

        
#ifdef CONFIG_MT_FPGA_GPE      
        if(g_debug.key_msk ||  g_debug.key_set)
        {
            data = gpe_read_register(GPE_ARIA_COMP_CFG);
            if(g_debug.key_msk)
            data |= 1 << 12;
            if(g_debug.key_set)
            data |= 1 << 4;
            gpe_write_register(GPE_ARIA_COMP_CFG, data);
        }

        //if(g_debug.zero_edge != 0)
        {
            data = gpe_read_register(GPE_ARIA_SCALER_CFG);
            data &= ~(3 << 8);
            if(g_debug.zero_edge)
                data |= (g_debug.zero_edge << 8);
         //   printf("<%s> : g_debug Scaler cfg :  %x g_debug.zero_edge : %d\n", __FUNCTION__, data, g_debug.zero_edge);
            gpe_write_register(GPE_ARIA_SCALER_CFG, data); 
        }

        data = gpe_read_register(GPE_ARIA_GRA_EN);
        if(g_debug.clip_en)
        {
            //data &= ~(1 << 24);
            data |= (1 << 24);

            gpe_write_register(GPE_ARIA_SRC1_OP_SIZE, (p_ctx->src_img.height << 16) | p_ctx->src_img.width);
         /*   gpe_write_register(GPE_ARIA_DST_OP_SIZE, (p_ctx->src_img.height << 16) | p_ctx->src_img.width);
            if((p_ctx->rotator_op == GPE_ARIA_TRANS) || (p_ctx->rotator_op == GPE_ARIA_HORI_MIRROR_TRANS) 
                || (p_ctx->rotator_op == GPE_ARIA_VERT_MIRROR_TRANS) || (p_ctx->rotator_op == GPE_ARIA_HORI_VERT_MIRROR_TRANS))
                gpe_write_register(GPE_ARIA_DST_OP_SIZE, (p_ctx->src_img.width << 16) | p_ctx->src_img.height);*/
        }
        else
        {
            data &= ~(1 << 24);
        }
        gpe_write_register(GPE_ARIA_GRA_EN, data);

        if((p_ctx->rotator_en) && (g_debug.rotator_op != 0))
        {
            data = gpe_read_register(GPE_ARIA_ROT_PAT_CFG);
            data &= ~3;
            data |= g_debug.rotator_op;
            gpe_write_register(GPE_ARIA_ROT_PAT_CFG, data); 
        }
        if(g_debug.dst_premult_en)
        {
            data = gpe_read_register(GPE_ARIA_DST0_FMT_CFG1);
            data |= (1 << 8);
            gpe_write_register(GPE_ARIA_DST0_FMT_CFG1, data); 
        }
#endif
#ifdef CONFIG_MT_FPGA_GPE  
      if(g_debug.print_reg0 == TRUE)
        {
            printf("<%s> : <%d>\n", __FUNCTION__, __LINE__);
            aria_gpe_print_reg();
        }
#endif

#ifndef CONFIG_TDE_PROC_DISABLE
        gpe_set_proc_info(NULL, 1);
#endif
        //        aria_gpe_print_reg();
        MT_INFO_TDE("========start to run hardware gpe========\n");
        ret = aria_gpe_command_start();
        // error found in hw, so need reset the hw
        if(ret == MT_FALSE)
        {
#ifndef CONFIG_TDE_PROC_DISABLE
            gpe_set_proc_info(NULL, 2);
#endif
            // time out, the HW is error, so need check all the register
            data = gpe_read_register(GPE_ARIA_GRA_STATE);
            MT_INFO_TDE("gpe aria timeout status = 0x%x\n", data);

            aria_gpe_print_reg();
 #ifdef CONFIG_MT_FPGA_GPE  
        if(g_debug.no_rst == 0)   
        	data = gpe_read_register(GPE_ARIA_GRA_CLK_GATED);
#endif
            // use sys block reset
            aria_gpe_init_reg(MT_TRUE);
#ifdef CONFIG_MT_FPGA_GPE
		if(g_debug.no_rst == 0)
        	gpe_write_register(GPE_ARIA_GRA_CLK_GATED, data); 
#endif

        }
        else
        {
            MT_INFO_TDE("========hardware gpe finished successsully\n");
#ifndef CONFIG_TDE_PROC_DISABLE
            gpe_set_proc_info(NULL, 3);
#endif
            //    GPE_PRINT("========hardware gpe finished successsully, status = 0x%0x========\n",
            //        gpe_read_register(REG_GPE_ARIA_GRA_STATUS));
        }
    }

#ifdef CONFIG_MT_FPGA_GPE  
      if(g_debug.compare_with_sw)
      {
          comp_ret = gpe_sw_compare(p_ctx);
          if((comp_ret == FALSE) && (g_debug.print_reg1 == TRUE))
          {
            printf("<%s> : <%d> :\n", __FUNCTION__, __LINE__);
            aria_gpe_print_reg();
          }
      }
#endif

#ifdef FAST_2D_SCALE
    if(p_ctx->scale_en) //
    {
#if !(defined(CONFIG_MT_CHIP_SYMPHONY6))  //sym6 don't need reset after fast2d scale.
        printf("%s,%d here \n",__FUNCTION__,__LINE__);
        aria_gpe_init_reg(MT_TRUE); //  
#endif        
    }
#endif

    // after operation finished, reset the context
    p_ctx->src_img_en = MT_FALSE;
    p_ctx->ex_img_en = MT_FALSE;
    p_ctx->bg_img_en = MT_FALSE;
    p_ctx->paint_en = MT_FALSE;
    p_ctx->paint_pattern_en = MT_FALSE;
    p_ctx->paint_gradt_en = MT_FALSE;
    p_ctx->rop_en = MT_FALSE;
    p_ctx->alpha_map_en = MT_FALSE;
    p_ctx->blend_en = MT_FALSE;
    p_ctx->scale_en = MT_FALSE;
    p_ctx->rotator_en = MT_FALSE;
    p_ctx->comp_en = MT_FALSE;
    p_ctx->is_draw_multiply = MT_FALSE;
    p_ctx->is_draw_stencil = MT_FALSE;
    p_ctx->src_is_tile = MT_FALSE;
    p_ctx->src_is_xylc = MT_FALSE;
    p_ctx->src1_sel = MT_FALSE;
    p_ctx->src2_sel = MT_FALSE;
    p_ctx->src3_sel = MT_FALSE;
    p_ctx->src_img.plane_alpha_en = MT_FALSE;
    p_ctx->ex_img.plane_alpha_en = MT_FALSE;
    p_ctx->dst_with_mask = MT_FALSE;
    p_ctx->src_with_mask = MT_FALSE;
    p_ctx->src1_rgb2yuv_en = MT_FALSE;
    p_ctx->src1_yuv2rgb_en = MT_FALSE;
    p_ctx->src2_rgb2yuv_en = MT_FALSE;
    p_ctx->src2_yuv2rgb_en = MT_FALSE;
    p_ctx->src3_rgb2yuv_en = MT_FALSE;
    p_ctx->src3_yuv2rgb_en = MT_FALSE;
    p_ctx->dst_rgb2yuv_en = MT_FALSE;
    p_ctx->dst_yuv2rgb_en = MT_FALSE;
    p_ctx->src1_palt_load_en = MT_FALSE;
    p_ctx->src3_palt_load_en = MT_FALSE;

    if(p_ctx->p_gradt_buf != 0)
    {
        MT_INFO_TDE("========free gradt buf========< %lx> \n", p_ctx->p_gradt_buf);
        mt_mmz_delete(p_ctx->p_gradt_buf);
        p_ctx->p_gradt_buf = 0;
    }

    //GPE_PRINT("========start over========\n");
    return ret;
}

///////////////////////////////pattern//////////////////////////////////////
#ifdef OLD_EXP_MODE
#define GPE_DEFAULT_ALPHA 0xFF

static mt_u32 rgb233_expend(mt_u8 pk)
{
    mt_u8 r2 =  pk & 0xC0;      // xx00 0000
    mt_u8 g3 = (mt_u8)((pk & 0x38) << 2); // 00xx x000 => xxx0 000
    mt_u8 b3 = (mt_u8)((pk & 0x07) << 5); // 0000 0xxx => xxx0 000

    mt_u32 dst_pk = 0;

    dst_pk = GPE_DEFAULT_ALPHA << 24;

    dst_pk |= (mt_u32)(r2 << 16);
    dst_pk |= (mt_u32)(g3 << 8);
    dst_pk |= (mt_u32)b3;

    return dst_pk;
}


static mt_u32 rgb565_expend(mt_u16 pk)
{
    mt_u8 r5 = (mt_u8)((pk & 0xF800) >> 8);    // xxxx x000 0000 0000
    mt_u8 g6 = (mt_u8)((pk & 0x07E0) >> 3);    // 0000 0xxx xxx0 0000
    mt_u8 b5 = (mt_u8)((pk & 0x001F) << 3);    // 0000 0000 000x xxxx

    mt_u32 rgb888 = 0;

    rgb888 = (mt_u32)((GPE_DEFAULT_ALPHA << 24) | (r5 <<  16) | (g6 << 8) | b5);

    return rgb888;
}


static mt_u32 argb1555_expend(mt_u16 pk)
{
    mt_u8 a1 = (mt_u8)((pk & 0x8000) >> 15);
    mt_u8 r5 = (mt_u8)(((pk & 0x7C00) << 1) >> 8);    // 0xxx xx00 0000 0000
    mt_u8 g5 = (mt_u8)((pk & 0x03E0) >> 2);           // 0000 00xx xxx 0000
    mt_u8 b5 = (mt_u8)((pk & 0x001F) << 3);           // 0000 0000 000x xxxx
    mt_u8 alpha = 0;

    mt_u32 rgb888 = 0;

    if(a1)
    {
        alpha = GPE_DEFAULT_ALPHA;
    }
    else
    {
        alpha = 0x0;
    }

    rgb888 = (mt_u32)((alpha << 24) | (r5 <<  16) | (g5 << 8) | b5);

    return rgb888;
}


static mt_u32 rgba5551_expend(mt_u16 pk)
{
    mt_u8 r5 = (mt_u8)((pk & 0xF800) >> 8);          // 0xxx xx00 0000 0000
    mt_u8 g5 = (mt_u8)(((pk & 0x07C0) << 1) >> 4);           // 0000 00xx xxx 0000
    mt_u8 b5 = (mt_u8)((pk & 0x003E) << 2);           // 0000 0000 000x xxxx

    mt_u8 a1 = (mt_u8)((pk & 0x1)) ;
    mt_u8 alpha = 0;

    mt_u32 rgb888 = 0;

    if(a1)
    {
        alpha = GPE_DEFAULT_ALPHA;
    }
    else
    {
        alpha = 0x00;
    }

    rgb888 = (mt_u32)((alpha << 24) | (r5 <<  16) | (g5 << 8) | b5);

    return rgb888;
}


static mt_u32 argb4444_expend(mt_u16 ck)
{
    mt_u8 a4 = (mt_u8)((ck & 0xF000) >> 8);
    mt_u8 r4 = (mt_u8)((ck & 0x0F00) >> 4);
    mt_u8 g4 = (mt_u8)(ck & 0x00F0);
    mt_u8 b4 = (mt_u8)((ck & 0x000F) << 4);

    mt_u32 rgb888 = 0;

    a4 = (mt_u8)((a4 << 4) | a4);
    r4 = (mt_u8)((a4 << 4) | r4);
    g4 = (mt_u8)((a4 << 4) | g4);
    b4 = (mt_u8)((a4 << 4) | b4);

    rgb888 = (mt_u32)((a4 << 24) | (r4 << 16) | (g4 << 8) | b4);

    return rgb888;
}


static mt_u32 rgba4444_expend(mt_u16 ck)
{
    mt_u8 r4 = (mt_u8)((ck & 0xF000) >> 8);
    mt_u8 g4 = (mt_u8)((ck & 0x0F00) >> 4);
    mt_u8 b4 = (mt_u8)(ck & 0x00F0);
    mt_u8 a4 = (mt_u8)((ck & 0x000F) << 4);

    mt_u32 rgb888 = 0;

    a4 = (mt_u8)((a4 << 4) | a4);
    r4 = (mt_u8)((a4 << 4) | r4);
    g4 = (mt_u8)((a4 << 4) | g4);
    b4 = (mt_u8)((a4 << 4) | b4);

    rgb888 = (mt_u32)((a4 << 24) | (r4 << 16) | (g4 << 8) | b4);

    return rgb888;
}


#else
#define EXPAND(color, bit_num) ((color & 0x1) ? ((1 << (8 - bit_num)) - 1) : 0)

static mt_u32 rgb233_expend(mt_u8 pk)
{
    mt_u8 a = 0xff;
    mt_u8 r = (pk >> 6) & 0x3;
    mt_u8 g = (pk >> 3) & 0x7;
    mt_u8 b = pk & 0x7;
    mt_u32 expand = 0;
    mt_u32 argb8888 = 0;

    expand = (mt_u32)((EXPAND(r, 2) << 16) | (EXPAND(g, 3) << 8) | EXPAND(b, 3));
    argb8888 = (mt_u32)((a << 24) | (r << 22) | (g << 13) | (b << 5) | expand);

    return argb8888;
}

static mt_u32 rgb565_expend(mt_u16 pk)
{
    mt_u8 a = 0xff;
    mt_u8 r = (mt_u8)((pk >> 11) & 0x1f);
    mt_u8 g = (mt_u8)((pk >> 5) & 0x3f);
    mt_u8 b = (mt_u8)(pk & 0x1f);
    mt_u32 expand = 0;
    mt_u32 argb8888 = 0;

    expand = (mt_u32)((EXPAND(r, 5) << 16) | (EXPAND(g, 6) << 8) | EXPAND(b, 5));
    argb8888 =  (mt_u32)((a << 24) | (r << 19) | (g << 10) | (b << 3) | expand);

    return argb8888;
}


static mt_u32 argb1555_expend(mt_u16 pk)
{
    mt_u8 a = (mt_u8)((((pk >> 15) & 0x1)  == 1) ? 0xff : 0);
    mt_u8 r = (mt_u8)((pk >> 10) & 0x1f);
    mt_u8 g = (mt_u8)((pk >> 5) & 0x1f);
    mt_u8 b = (mt_u8)(pk & 0x1f);
    mt_u32 expand = 0;
    mt_u32 argb8888 = 0;

    expand = (mt_u32)((EXPAND(r, 5) << 16) | (EXPAND(g, 5) << 8) | EXPAND(b, 5));
    argb8888 =  (mt_u32)((a << 24) | (r << 19) | (g << 11) | (b << 3) | expand);

    return argb8888;
}


static mt_u32 rgba5551_expend(mt_u16 pk)
{
    mt_u8 a = (mt_u8)(((pk & 0x1) == 1) ? 0xff : 0);
    mt_u8 b = (mt_u8)((pk >> 1) & 0x1f);
    mt_u8 g = (mt_u8)((pk >> 6) & 0x1f);
    mt_u8 r = (mt_u8)((pk >> 11) & 0x1f);
    mt_u32 expand = (mt_u32)((EXPAND(r, 5) << 16) | (EXPAND(g, 5) << 8) | EXPAND(b, 5));
    mt_u32 argb8888 = (mt_u32)((a << 24) | (r << 19) | (g << 11) | (b << 3) | expand);

    return argb8888;
}


static mt_u32 argb4444_expend(mt_u16 pk)
{
    mt_u8 b = (mt_u8)(pk & 0xf);
    mt_u8 g = (mt_u8)((pk >> 4) & 0xf);
    mt_u8 r = (mt_u8)((pk >> 8) & 0xf);
    mt_u8 a = (mt_u8)((pk >> 12) & 0xf);
    mt_u32 expand = (mt_u32)((EXPAND(a, 4) << 24) | (EXPAND(r, 4) << 16) | (EXPAND(g, 4) << 8) | EXPAND(b, 4));
    mt_u32 argb8888 = (mt_u32)((a << 28)  | (r << 20)  | (g << 12) | (b << 4) | expand);
    return argb8888;
}


static mt_u32 rgba4444_expend(mt_u16 pk)
{
    mt_u8 a = (mt_u8)(pk & 0xf);
    mt_u8 b = (mt_u8)((pk >> 4) & 0xf);
    mt_u8 g = (mt_u8)((pk >> 8) & 0xf);
    mt_u8 r = (mt_u8)((pk >> 12) & 0xf);
    mt_u32 expand = (mt_u32)((EXPAND(a, 4) << 24) | (EXPAND(r, 4) << 16) | (EXPAND(g, 4) << 8) | EXPAND(b, 4));
    mt_u32 argb8888 = (mt_u32)((a << 28)  | (r << 20) | (g << 12)| (b << 4) | expand);

    return argb8888;
}

#endif
static mt_u32 bit32_little_endian_cnt(mt_u32 src)
{
    mt_u32 dst = 0;
    mt_u8 s1 = (mt_u8)((src & 0xFF000000) >> 24);
    mt_u8 s2 = (mt_u8)((src & 0x00FF0000) >> 16);
    mt_u8 s3 = (mt_u8)((src & 0x0000FF00) >> 8);
    mt_u8 s4 = (mt_u8)(src & 0x000000FF);
    dst = (mt_u32)(s4 << 24 | s3 << 16 | s2 << 8 | s1);
    //GPE_PRINT("32cnt : src 0x%x, dst 0x%x",src,dst);
    return dst;
}

static mt_u16 bit16_little_endian_cnt(mt_u32 src)
{
    mt_u16 dst = 0;
    mt_u8 s1 = (mt_u8)((src & 0xFF00) >> 8);
    mt_u8 s2 = (mt_u8)(src & 0x00FF) ;

    dst = (mt_u16)(s2 << 8 | s1);

    //GPE_PRINT("16cnt : src 0x%x, dst 0x%x",src,dst);
    return dst;
}

#define CLIP(x) ((x < 0) ? (0) : ((x > 255) ? 255 : x))

static mt_u32 ayuv2argb(mt_u32 src)
{
    mt_u8 a = 0, y = 0, u = 0, v = 0;
    int r = 0, g = 0, b = 0;
    mt_u32 dst = 0;

    a = (mt_u8)((src >> 24) & 0xff);
    y = (mt_u8)((src >> 16) & 0xff);
    u = (mt_u8)((src >> 8) & 0xff);
    v = (mt_u8)(src & 0xff);

    r = (((298 * y >> 4) + (0 * u >> 4) + (459 * v >> 4) +  ((-63522) >> 4)))>> 4;
    g = (((298 * y >> 4) + ((-55) * u >> 4) + ((-136) * v >> 4) +  (19659 >> 4)))>> 4;
    b = (((298 * y >> 4) + (541 * u >> 4) + (0 * v >> 4) +  ((-74002) >> 4)))>> 4;
    r = CLIP(r);
    g = CLIP(g);
    b = CLIP(b);

    dst = (mt_u32)((a << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff));
    return dst;
}

static mt_u32 uyvy2ayuv(mt_u32 src)
{
    mt_u32 dst = 0;
    mt_u8 y0 = 0, y1 = 0, u = 0, v = 0;

    u = (mt_u8)((src >> 24) & 0xff);
    y0 = (mt_u8)((src >> 16) & 0xff);
    v = (mt_u8)((src >> 8) & 0xff);
    y1 = (mt_u8)(src & 0xff);
    dst = (mt_u32)((0xff << 24) | (((y0 + y1 + 1) >> 1) << 16) | (u << 8) | v);
    return dst;
}

//color->ARGB8888
mt_u32 colorkey_expend(TDE2_COLOR_FMT_E enColorFmt, mt_u32 pk)
{
    mt_u32 tmppk = 0;
    MT_BOOL alpha_en;
    pix_fmt_t fmt = aria_gpe_convert_fmt(enColorFmt, &alpha_en);
    //???????????argb??colorkey??rgb?????argb????yuv?????ayuv
    switch(fmt)
    {
    case PIX_FMT_RGB233:
        tmppk = rgb233_expend((mt_u8)pk);
        break;
    case PIX_FMT_RGB565:
        tmppk = rgb565_expend((mt_u16)pk);
        break;
    case PIX_FMT_ARGB1555:
        tmppk = argb1555_expend((mt_u16)pk);
        break;
    case PIX_FMT_ARGB4444:
        tmppk = argb4444_expend((mt_u16)pk);
        break;
    case PIX_FMT_RGBA5551:
        tmppk = rgba5551_expend((mt_u16)pk);
        break;
    case PIX_FMT_RGBA4444:
        tmppk = rgba4444_expend((mt_u16)pk);
        break;
    case PIX_FMT_RGB565_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = rgb565_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = argb1555_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = argb4444_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = rgba5551_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = rgba4444_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_ARGB8888_SMALL_ENDIAN:
        tmppk = bit32_little_endian_cnt(pk);
        break;
    case PIX_FMT_RGBA8888_SMALL_ENDIAN:
        tmppk = bit32_little_endian_cnt(pk);
        tmppk = ((tmppk & 0x000000FF) << 24) | ((tmppk & 0xFFFFFF00) >> 8);
        break;
    case PIX_FMT_CRCBYA8888://34
        tmppk = bit32_little_endian_cnt(pk);
        break;
    case PIX_FMT_ACRCBY8888://36
        tmppk = bit32_little_endian_cnt(pk);
        tmppk = ((tmppk & 0x000000FF) << 24) | ((tmppk & 0xFFFFFF00) >> 8);
        break;
    case PIX_FMT_RGBA8888:
        tmppk = ((pk & 0x000000FF) << 24) | ((pk & 0xFFFFFF00) >> 8);
        break;
    case PIX_FMT_YCBCRA8888://35
        tmppk = ((pk & 0x000000FF) << 24) | ((pk & 0xFFFFFF00) >> 8);
        break;
    case PIX_FMT_Y1CRY0CB8888: //27
        tmppk = bit32_little_endian_cnt(pk);
        tmppk = uyvy2ayuv(tmppk);
        break;
    case PIX_FMT_CBY0CRY18888://28
        tmppk = uyvy2ayuv(pk);
        break;
    case PIX_FMT_ARGB8888:
        tmppk = pk;
        break;
    case PIX_FMT_AYCBCR8888://33
        tmppk = pk;
        break;

    default:
        tmppk = pk;
    }
    return tmppk;
}

//color->ARGB8888
mt_u32 aria_color_expend(pix_fmt_t fmt, mt_u32 pk, MT_BOOL ck_exp)
{
    mt_u32 tmppk = 0;
    //???????????argb??colorkey??rgb?????argb????yuv?????ayuv
    switch(fmt)
    {
    case PIX_FMT_RGB233:
        tmppk = rgb233_expend((mt_u8)pk);
        break;
    case PIX_FMT_RGB565:
        tmppk = rgb565_expend((mt_u16)pk);
        break;
    case PIX_FMT_ARGB1555:
        tmppk = argb1555_expend((mt_u16)pk);
        break;
    case PIX_FMT_ARGB4444:
        tmppk = argb4444_expend((mt_u16)pk);
        break;
    case PIX_FMT_RGBA5551:
        tmppk = rgba5551_expend((mt_u16)pk);
        break;
    case PIX_FMT_RGBA4444:
        tmppk = rgba4444_expend((mt_u16)pk);
        break;
    case PIX_FMT_RGB565_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = rgb565_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = argb1555_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = argb4444_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = rgba5551_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
        tmppk = bit16_little_endian_cnt(pk);
        tmppk = rgba4444_expend((mt_u16)tmppk);
        break;
    case PIX_FMT_ARGB8888_SMALL_ENDIAN:
        tmppk = bit32_little_endian_cnt(pk);
        break;
    case PIX_FMT_RGBA8888_SMALL_ENDIAN:
        tmppk = bit32_little_endian_cnt(pk);
        tmppk = ((tmppk & 0x000000FF) << 24) | ((tmppk & 0xFFFFFF00) >> 8);
        break;
    case PIX_FMT_CRCBYA8888://34
        tmppk = bit32_little_endian_cnt(pk);
        if(ck_exp == MT_FALSE)
            tmppk = ayuv2argb(tmppk);
        break;
    case PIX_FMT_ACRCBY8888://36
        tmppk = bit32_little_endian_cnt(pk);
        tmppk = ((tmppk & 0x000000FF) << 24) | ((tmppk & 0xFFFFFF00) >> 8);
        if(ck_exp == MT_FALSE)
            tmppk = ayuv2argb(tmppk);
        break;
    case PIX_FMT_RGBA8888:
        tmppk = ((pk & 0x000000FF) << 24) | ((pk & 0xFFFFFF00) >> 8);
        break;
    case PIX_FMT_YCBCRA8888://35
        tmppk = ((pk & 0x000000FF) << 24) | ((pk & 0xFFFFFF00) >> 8);
        if(ck_exp == MT_FALSE)
            tmppk = ayuv2argb(tmppk);
        break;
    case PIX_FMT_Y1CRY0CB8888: //27
        tmppk = bit32_little_endian_cnt(pk);
        tmppk = uyvy2ayuv(tmppk);
        if(ck_exp == MT_FALSE)
            tmppk = ayuv2argb(tmppk);
        break;
    case PIX_FMT_CBY0CRY18888://28
        tmppk = uyvy2ayuv(pk);
        if(ck_exp == MT_FALSE)
            tmppk = ayuv2argb(tmppk);
        break;
    case PIX_FMT_ARGB8888:
        tmppk = pk;
        break;
    case PIX_FMT_AYCBCR8888://33
        if(ck_exp == MT_FALSE)
            tmppk = ayuv2argb(pk);
        else
            tmppk = pk;
        break;

    default:
        tmppk = pk;
    }
    return tmppk;
}

#ifdef OLD_EXP_MODE
mt_u32 aria_lut_patterncolor(pix_fmt_t fmt, mt_u32 color)
{
    mt_u32 pat_color = 0;
    mt_u8 a = 0;
    mt_u8 idx = 0;
    switch(fmt)
    {
    case PIX_FMT_ARGBPALETTE88:
    case PIX_FMT_AYUVPALETTE88:
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
        a = (mt_u8)((color >> 8) & 0xff);
        idx = (mt_u8)(color & 0xff);
        pat_color = (mt_u32)((a << 24) | idx);
        break;
    case PIX_FMT_RGBAPALETTE88:
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
    case PIX_FMT_YUVAPALETTE88:
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
        a = (mt_u8)(color & 0xff);
        idx = (mt_u8)((color >> 8) & 0xff);
        pat_color = (mt_u32)((a << 24) | idx);
        break;
    case PIX_FMT_RGBPALETTE8:
    case PIX_FMT_YUVPALETTE8:
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
        pat_color = color;
        break;
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
        a = (mt_u8)((color >> 4) & 0xf);
        a = (mt_u8)(a << 4);
        idx = (mt_u8)(color & 0xf);
        pat_color = (mt_u32)((a << 24) | idx);
        break;
    default:
        pat_color = color;
        break;
    }
    return pat_color;
}
#else
mt_u32 aria_lut_patterncolor(pix_fmt_t fmt, mt_u32 color)
{
    /*
       For CLUT8 , use rop_pat[23:16] copy to DDR, others are ignored
       For ACLUT44, use rop_pat[27:24] and rop_pat[19:16]
       For ACLUT88, use rop_pat[31:16]
       */
    mt_u32 pat_color = 0;
    mt_u8 a = 0;
    mt_u8 idx = 0;
    switch(fmt)
    {
    case PIX_FMT_ARGBPALETTE88:
    case PIX_FMT_AYUVPALETTE88:
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
        a = (mt_u8)((color >> 8) & 0xff);
        idx = (mt_u8)(color & 0xff);
        pat_color = (mt_u32)((a << 24) | idx);
        break;
    case PIX_FMT_RGBAPALETTE88:
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
    case PIX_FMT_YUVAPALETTE88:
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
        a = (mt_u8)(color & 0xff);
        idx = (mt_u8)((color >> 8) & 0xff);
        pat_color = (mt_u32)((a << 24) | idx);
        break;
    case PIX_FMT_RGBPALETTE8:
    case PIX_FMT_YUVPALETTE8:
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
        pat_color = color;
        break;
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
        a = (mt_u8)((color >> 4) & 0xf);
        a = (a << 4) | EXPAND(a, 4);
        idx = (mt_u8)(color & 0xf);
        pat_color = (mt_u32)((a << 24) | idx);
        break;
    default:
        pat_color = color;
        break;
    }
    return pat_color;
}
#endif
#if 0
static mt_u32 aria_expend(pix_fmt_t fmt, mt_u32 pk)
{
    if(is_lut(fmt))
        return aria_lut_patterncolor(fmt, pk);
    else
        return aria_color_expend(fmt, pk, MT_FALSE);
}
#endif
static mt_u32 get_bpp(mt_u32 pix_fmt)
{
    mt_u32 bpp = 0;
    switch(pix_fmt)
    {
    case PIX_FMT_XY:
    case PIX_FMT_XYL:
    case PIX_FMT_XYC:
    case PIX_FMT_XYLC:
    case PIX_FMT_XY_SMALL:
    case PIX_FMT_XYL_SMALL:
    case PIX_FMT_XYC_SMALL:
    case PIX_FMT_XYLC_SMALL:
        bpp = 32;
        break;
    case PIX_FMT_ARGB8888:
    case PIX_FMT_RGBA8888:
    case PIX_FMT_AYCBCR8888:
    case PIX_FMT_CRCBYA8888:
    case PIX_FMT_YCBCRA8888:
    case PIX_FMT_ACRCBY8888:
    case PIX_FMT_ARGB8888_SMALL_ENDIAN:
    case PIX_FMT_RGBA8888_SMALL_ENDIAN:
    case PIX_FMT_CMYK:
    case PIX_FMT_KYMC:
        bpp = 32;
        break;
    case PIX_FMT_ARGBPALETTE88:
    case PIX_FMT_AYUVPALETTE88:
    case PIX_FMT_RGB565:
    case PIX_FMT_ARGB1555:
    case PIX_FMT_RGBA5551:
    case PIX_FMT_ARGB4444:
    case PIX_FMT_RGBA4444:
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
    case PIX_FMT_RGBAPALETTE88:
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
    case PIX_FMT_YUVAPALETTE88:
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
    case PIX_FMT_RGB565_SMALL_ENDIAN:
    case PIX_FMT_ARGB1555_SMALL_ENDIAN:
    case PIX_FMT_ARGB4444_SMALL_ENDIAN:
    case PIX_FMT_RGBA5551_SMALL_ENDIAN:
    case PIX_FMT_RGBA4444_SMALL_ENDIAN:
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
    case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
        bpp = 16;
        break;
    case PIX_FMT_CBY0CRY18888:
    case PIX_FMT_Y1CRY0CB8888:
    case PIX_FMT_Y0CBY1CR8888:
    case PIX_FMT_Y0CRY1CB8888:
    case PIX_FMT_Y1CBY0CR8888:
    case PIX_FMT_CBY1CRY08888:
    case PIX_FMT_CRY1CBY08888:
    case PIX_FMT_CRY0CBY18888:
        bpp = 16;
        break;
    case PIX_FMT_RGB888:
    case PIX_FMT_BGR888:
        bpp = 24;
        break;
    case PIX_FMT_TILE:
    case PIX_FMT_GRAY_8:
    case PIX_FMT_RGBPALETTE8:
    case PIX_FMT_YUVPALETTE8:
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
    case PIX_FMT_RGB233:
        bpp = 8;
        break;
    case PIX_FMT_RGBPALETTE4:
    case PIX_FMT_YUVPALETTE4:
    case PIX_FMT_RGBPALETTE4_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE4_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE4_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE4_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE4_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE4_PALETTE_AVUY:
    case PIX_FMT_ARGBPALETTE22:
    case PIX_FMT_AYUVPALETTE22:
        bpp = 4;
        break;
    case PIX_FMT_RGBPALETTE2:
    case PIX_FMT_YUVPALETTE2:
    case PIX_FMT_RGBPALETTE2_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE2_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE2_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE2_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE2_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE2_PALETTE_AVUY:
    case PIX_FMT_ARGBPALETTE11:
    case PIX_FMT_AYUVPALETTE11:
        bpp = 2;
        break;
    case PIX_FMT_RGBPALETTE1_PALETTE_BGRA:
    case PIX_FMT_RGBPALETTE1:
    case PIX_FMT_YUVPALETTE1:
        bpp = 1;
        break;
    default:
        bpp = 32;
        break;
    }
    return bpp;
}
//----------fix bug 99703 beg, add soft scale when src rect is less than 8x8

static MT_BOOL is_semiplanner(mt_u32 pix_fmt)
{
    MT_BOOL is_semi = MT_FALSE;
    switch(pix_fmt)
    {
    case PIX_FMT_SP_YUV444:
    case PIX_FMT_SP_YUV422:
    case PIX_FMT_SP_YUV420:
    case PIX_FMT_SP_YUV422_1x2:
    case PIX_FMT_SP_YUV422_2x1:
    case PIX_FMT_SP_YUV444_UVSWAP:
    case PIX_FMT_SP_YUV422_UVSWAP:
    case PIX_FMT_SP_YUV420_UVSWAP:
    case PIX_FMT_SP_YUV422_1x2_UVSWAP:
    case PIX_FMT_SP_YUV422_2x1_UVSWAP:
    case PIX_FMT_SP_CMYK:
    case PIX_FMT_SP_CMYK_SWAP:
        is_semi = MT_TRUE;
        break;
    default:
        break;
    }
    return is_semi;
}
//----------fix bug 99703 end

MT_BOOL is_lut(mt_u32 pix_fmt)
{
    MT_BOOL is_lut = MT_FALSE;
    switch(pix_fmt)
    {
    case PIX_FMT_ARGBPALETTE88:
    case PIX_FMT_AYUVPALETTE88:
    case PIX_FMT_ARGBPALETTE88_PALETTE_BGRA:
    case PIX_FMT_RGBAPALETTE88:
    case PIX_FMT_RGBAPALETTE88_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_VUYA:
    case PIX_FMT_YUVAPALETTE88:
    case PIX_FMT_YUVAPALETTE88_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE88_PALETTE_ABGR:
    case PIX_FMT_RGBAPALETTE88_PALETTE_RGBA:
    case PIX_FMT_RGBAPALETTE88_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE88_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE88_PALETTE_AVUY:
    case PIX_FMT_YUVAPALETTE88_PALETTE_YUVA:
    case PIX_FMT_YUVAPALETTE88_PALETTE_AVUY:
    case PIX_FMT_RGBPALETTE8:
    case PIX_FMT_YUVPALETTE8:
    case PIX_FMT_ARGBPALETTE44:
    case PIX_FMT_AYUVPALETTE44:
    case PIX_FMT_RGBPALETTE8_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE8_PALETTE_VUYA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_BGRA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE8_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE8_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE8_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE8_PALETTE_AVUY:
    case PIX_FMT_ARGBPALETTE44_PALETTE_RGBA:
    case PIX_FMT_ARGBPALETTE44_PALETTE_ABGR:
    case PIX_FMT_AYUVPALETTE44_PALETTE_YUVA:
    case PIX_FMT_AYUVPALETTE44_PALETTE_AVUY:
    case PIX_FMT_RGBPALETTE4:
    case PIX_FMT_YUVPALETTE4:
    case PIX_FMT_RGBPALETTE4_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE4_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE4_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE4_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE4_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE4_PALETTE_AVUY:
    case PIX_FMT_ARGBPALETTE22:
    case PIX_FMT_AYUVPALETTE22:
    case PIX_FMT_RGBPALETTE2:
    case PIX_FMT_YUVPALETTE2:
    case PIX_FMT_RGBPALETTE2_PALETTE_BGRA:
    case PIX_FMT_YUVPALETTE2_PALETTE_VUYA:
    case PIX_FMT_RGBPALETTE2_PALETTE_RGBA:
    case PIX_FMT_RGBPALETTE2_PALETTE_ABGR:
    case PIX_FMT_YUVPALETTE2_PALETTE_YUVA:
    case PIX_FMT_YUVPALETTE2_PALETTE_AVUY:
    case PIX_FMT_ARGBPALETTE11:
    case PIX_FMT_AYUVPALETTE11:
    case PIX_FMT_RGBPALETTE1_PALETTE_BGRA:
    case PIX_FMT_RGBPALETTE1:
    case PIX_FMT_YUVPALETTE1:
        is_lut = MT_TRUE;
        break;
    default:
        is_lut = MT_FALSE;
        break;
    }
    return is_lut;
}

static void param_init(aria_param_t *p_param)
{

    p_param->no_dithering = MT_FALSE;
    p_param->comp_key_set = MT_FALSE; //0:keep dst value when colorkey math, 1: output 0xffffffff when colorkey match

#if !defined(CONFIG_MT_CHIP_SYMPHONY6)
    p_param->clip_en = MT_TRUE;
#endif

#ifdef FAST_2D_SCALE
    p_param->dst_ds_mod = DS_EVEN_UV;
    p_param->dst_ds_choose = 0;
#else
    p_param->dst_ds_mod = DS_MEDIA_UV;
    p_param->dst_ds_choose = 1;
#endif
    p_param->src_img.negative_stride = MT_FALSE;
    p_param->src_img.alpha_pre_multed = MT_TRUE;
    p_param->src_img.alpha_ch_en = MT_TRUE;
    p_param->src_img.plane_alpha_en = MT_FALSE;
    p_param->src_img.ck_en = MT_FALSE;
    p_param->src_with_mask = MT_FALSE;

    p_param->dst_img.negative_stride = MT_FALSE;
    p_param->dst_img.alpha_ch_en = MT_TRUE;
    p_param->dst_img.ck_en = MT_FALSE;
    p_param->dst_with_mask = MT_FALSE;
    p_param->dst_img.alpha_pre_multed = MT_TRUE;

    p_param->ex_img.negative_stride = MT_FALSE;
    p_param->ex_img.alpha_pre_multed = MT_TRUE;
    p_param->ex_img.alpha_ch_en = MT_TRUE;
    p_param->ex_img.plane_alpha_en = MT_FALSE;
    p_param->ex_img.ck_en = MT_FALSE;

    p_param->bg_img.negative_stride = MT_FALSE;
    p_param->bg_img.alpha_pre_multed = MT_TRUE;
    p_param->bg_img.alpha_ch_en = MT_TRUE;
    p_param->bg_img.plane_alpha_en = MT_FALSE;
    p_param->bg_img.ck_en = MT_FALSE;

    p_param->gpe_op = 0;

    p_param->src_img_en = MT_FALSE;
    p_param->ex_img_en = MT_FALSE;
    p_param->bg_img_en = MT_FALSE;

    p_param->src_img.palette_size = 256;
    p_param->bg_img.palette_size = 256;
    p_param->ex_img.palette_size = 256;
    p_param->dst_img.palette_size = 256;

}

static GPE_COLORFMT_CATEGORY_E GpeOsiGetFmtCategory(TDE2_COLOR_FMT_E enFmt)
{
    MT_INFO_TDE("%s:%d [%x]\n", __FUNCTION__, __LINE__, enFmt);
    /* target is ARGB format */
    if (enFmt <= TDE2_COLOR_FMT_RABG8888 || (enFmt == TDE2_COLOR_FMT_RGB24)) 
    {
        return GPE_COLORFMT_CATEGORY_ARGB;
    }
    /* target is CLUT table format */
    else if (enFmt <= TDE2_COLOR_FMT_ACLUT88)
    {
        return GPE_COLORFMT_CATEGORY_CLUT;
    }
    /* target is alpha CLUT table format */
    else if (enFmt <= TDE2_COLOR_FMT_A8)
    {
        return GPE_COLORFMT_CATEGORY_An;
    }
    /* target is YCbCr format */
    else if (enFmt <= TDE2_COLOR_FMT_YCbCr422 ||(enFmt == TDE2_COLOR_FMT_JPG_YCbCr420MBP))
    {
        return GPE_COLORFMT_CATEGORY_YCbCr;
    }
    /* byte format */
    else if (enFmt == TDE2_COLOR_FMT_byte)
    {
        return GPE_COLORFMT_CATEGORY_BYTE;
    }
    /* halfword  format */
    else if (enFmt == TDE2_COLOR_FMT_halfword)
    {
        return GPE_COLORFMT_CATEGORY_HALFWORD;
    }
    else if (enFmt <= TDE2_COLOR_FMT_JPG_YCbCr444MBP)
    {
        return GPE_COLORFMT_CATEGORY_YCbCr;
    }
    /* error format */
    else
    {
        return GPE_COLORFMT_CATEGORY_BUTT;
    }
}

static mt_u32 GpeOsiGetKeyMode(TDE2_COLORKEY_U *punColorKeyValue, GPE_COLORFMT_CATEGORY_E enFmtCategory)
{
    ck_mod_t key_mod;
    mt_u32 key_mode = (KEY_MATCH_INSIDE_MIN_MAX << 12)
        | (KEY_MATCH_INSIDE_MIN_MAX << 8)
        | (KEY_MATCH_INSIDE_MIN_MAX << 4)
        | (KEY_MATCH_INSIDE_MIN_MAX);

    if (GPE_COLORFMT_CATEGORY_ARGB == enFmtCategory)
    {
        if (punColorKeyValue->struCkARGB.stBlue.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkARGB.stBlue.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode = key_mod;
        if (punColorKeyValue->struCkARGB.stGreen.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkARGB.stGreen.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 4;

        if (punColorKeyValue->struCkARGB.stRed.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkARGB.stRed.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 8;

        if (punColorKeyValue->struCkARGB.stAlpha.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkARGB.stAlpha.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 12;
    }
    else if (GPE_COLORFMT_CATEGORY_CLUT == enFmtCategory)
    {
        if (punColorKeyValue->struCkClut.stClut.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkClut.stClut.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode = key_mod;

        if (punColorKeyValue->struCkClut.stAlpha.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkClut.stAlpha.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 12;

    }
    else if (GPE_COLORFMT_CATEGORY_YCbCr == enFmtCategory)
    {
        if (punColorKeyValue->struCkYCbCr.stCr.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkYCbCr.stCr.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode = key_mod;

        if (punColorKeyValue->struCkYCbCr.stCb.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkYCbCr.stCb.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 4;

        if (punColorKeyValue->struCkYCbCr.stY.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkYCbCr.stY.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 8;

        if (punColorKeyValue->struCkYCbCr.stAlpha.bCompIgnore)
        {
            key_mod = KEY_MATCH_ALL;
        }
        else if (punColorKeyValue->struCkYCbCr.stAlpha.bCompOut)
        {
            key_mod = KEY_MATCH_OUTSIDE_MIN_MAX;
        }
        else
        {
            key_mod = KEY_MATCH_INSIDE_MIN_MAX;
        }

        key_mode |= key_mod << 12;

    }
    return key_mode;
}

static mt_u32 GpeOsiGetCkeyMin(TDE2_COLORKEY_U *punColorKeyValue, GPE_COLORFMT_CATEGORY_E enFmtCategory)
{
    mt_u32 ck_min = 0;
    if (GPE_COLORFMT_CATEGORY_ARGB == enFmtCategory)
    {
        ck_min = (mt_u32)(punColorKeyValue->struCkARGB.stBlue.u8CompMin
                | (punColorKeyValue->struCkARGB.stGreen.u8CompMin << 8)
                | (punColorKeyValue->struCkARGB.stRed.u8CompMin << 16)
                | (punColorKeyValue->struCkARGB.stAlpha.u8CompMin << 24));

    }
    else if (GPE_COLORFMT_CATEGORY_CLUT == enFmtCategory)
    {
        ck_min = (mt_u32)(punColorKeyValue->struCkClut.stClut.u8CompMin
                | (punColorKeyValue->struCkClut.stAlpha.u8CompMin << 24));
    }
    else if (GPE_COLORFMT_CATEGORY_YCbCr == enFmtCategory)
    {
        ck_min = (mt_u32)(punColorKeyValue->struCkYCbCr.stCr.u8CompMin
                | (punColorKeyValue->struCkYCbCr.stCb.u8CompMin << 8)
                | (punColorKeyValue->struCkYCbCr.stY.u8CompMin << 16)
                | (punColorKeyValue->struCkYCbCr.stAlpha.u8CompMin << 24));
    }
    return ck_min;
}

static mt_u32 GpeOsiGetCkeyMax(TDE2_COLORKEY_U *punColorKeyValue, GPE_COLORFMT_CATEGORY_E enFmtCategory)
{
    mt_u32 ck_max = 0;
    if (GPE_COLORFMT_CATEGORY_ARGB == enFmtCategory)
    {
        ck_max = (mt_u32)(punColorKeyValue->struCkARGB.stBlue.u8CompMax
                | (punColorKeyValue->struCkARGB.stGreen.u8CompMax << 8)
                | (punColorKeyValue->struCkARGB.stRed.u8CompMax << 16)
                | (punColorKeyValue->struCkARGB.stAlpha.u8CompMax << 24));

    }
    else if (GPE_COLORFMT_CATEGORY_CLUT == enFmtCategory)
    {
        ck_max = (mt_u32)(punColorKeyValue->struCkClut.stClut.u8CompMax
                | (punColorKeyValue->struCkClut.stAlpha.u8CompMax << 24));

    }
    else if (GPE_COLORFMT_CATEGORY_YCbCr == enFmtCategory)
    {
        ck_max = (mt_u32)(punColorKeyValue->struCkYCbCr.stCr.u8CompMax
                | (punColorKeyValue->struCkYCbCr.stCb.u8CompMax << 8)
                | (punColorKeyValue->struCkYCbCr.stY.u8CompMax << 16)
                | (punColorKeyValue->struCkYCbCr.stAlpha.u8CompMax << 24));
    }
    return ck_max;
}



static MT_BOOL gpe_2d_vert_scale(TDE_HANDLE s32Handle,
        rect_vsb_t *p_src_rect,
        rect_vsb_t *p_dst_rect,
        TDE2_SURFACE_S* pstForeGround,
        TDE2_OPT_S* pstOpt,
        phys_addr_t p_tmp_buf,
        mt_u32 tmpbuf_pitch,
        mt_u32 tmpbuf_fmt,
        phys_addr_t p_mask_buf,
        mt_u32 maskbuf_pitch,
        MT_BOOL src_ck_en,
        mt_s32 *scale_coef)
{
    mt_s32 ret = MT_ERR_TDE_UNSUPPORTED_OPERATION;
    aria_param_t param = {0};
    GPE_COLORFMT_CATEGORY_E enFmtCategory;

    MT_ASSERT(NULL != pstForeGround);
    MT_ASSERT(0 != p_tmp_buf);
    MT_ASSERT(NULL != p_src_rect);
    MT_ASSERT(NULL != p_dst_rect);
    if(src_ck_en)
        MT_ASSERT(0  != p_mask_buf);

    param_init(&param);

    param.src_img_en = MT_TRUE;
    param.src_img.buf = pstForeGround->u32PhyAddr;
    param.src_img.pitch = pstForeGround->u32Stride;
    param.src_img.width = pstForeGround->u32Width;
    param.src_img.height = pstForeGround->u32Height;
    param.src_img.pix_format = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &param.src_img.alpha_ch_en);
    param.src_img.rect.x = p_src_rect->x;
    param.src_img.rect.y = p_src_rect->y;
    param.src_img.rect.w = p_src_rect->w;
    param.src_img.rect.h = p_src_rect->h;

    if(pstOpt->stSurfaceCfg.enSrcFlip)
        param.src_img.negative_stride = MT_TRUE;

    if((param.src_img.pix_format == PIX_FMT_TILE)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
            || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
    {
        param.src_img.chroma_addr = (pstForeGround->u32CbCrPhyAddr);
        param.src_img.chroma_pitch = pstForeGround->u32CbCrStride;
#ifdef CONFIG_MT_FPGA_GPE		
        g_debug.chroma_vir_addr = pstForeGround->u32CbCrVirAddr;
#endif		
    }

    param.dst_img.buf = p_tmp_buf;
    param.dst_img.pitch = tmpbuf_pitch;
    param.dst_img.width = p_dst_rect->w;
    param.dst_img.height = p_dst_rect->h;
    param.dst_img.pix_format = tmpbuf_fmt;
    param.dst_img.rect.x = p_dst_rect->x;
    param.dst_img.rect.y = p_dst_rect->y;
    param.dst_img.rect.w = p_dst_rect->w;
    param.dst_img.rect.h = p_dst_rect->h;
    MT_INFO_TDE("\r\n src[%d,%d,%d,%d], dst[%d,%d,%d,%d]", param.src_img.rect.x,param.src_img.rect.y,param.src_img.rect.w,param.src_img.rect.h,
            param.dst_img.rect.x,param.dst_img.rect.y,param.dst_img.rect.w,param.dst_img.rect.h);

    if(src_ck_en)
    {
        if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_FOREGROUND)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstForeGround->enColorFmt);
            param.src_img.ck_en = MT_TRUE;
            param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
            param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
            param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);
            param.src_img.key_color_select = pstOpt->enColorKeySelect;
        }
        else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
        {
            param.src_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_fg;
            if(param.src_img.ck_en)
            {
                enFmtCategory = GpeOsiGetFmtCategory(pstForeGround->enColorFmt);
                param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                param.src_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_fg;
            }
        }

        param.dst_with_mask = MT_TRUE;
        param.dst_mask.mbuf_addr = p_mask_buf;
        param.dst_mask.mbuf_pitch = maskbuf_pitch;
    }

    //3d scale????????????????????????????????校????????mask????????蔚?????
    if(scale_coef[5] == 1)
    {
        param.dst_with_mask = MT_TRUE;
        param.dst_mask.mbuf_addr =p_mask_buf;
        param.dst_mask.mbuf_pitch = maskbuf_pitch;
    }
    param.gpe_op = GPE_OP_SCALE | GPE_OP_ROTATE | GPE_OP_SCALE_VERT;
    param.rotator_op = GPE_ARIA_NO_OP;
    param.scale_mod = SCALE_VERT_BLK_OUT;
    param.coef[0] = scale_coef[0];
    param.coef[1] = scale_coef[1];
    param.coef[2] = scale_coef[2];
    param.coef[3] = scale_coef[3];
    param.coef[4] = scale_coef[4];
    param.coef[5] = scale_coef[5];

    param.need_suspend = MT_FALSE;
    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        return MT_FALSE;
    }
    ret = aria_gpe_start(s32Handle);
    return ret;
}
#if 0 //clear warning remove here
static void gpe_core_clk_high(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    volatile mt_u32 value = 0;
    mt_sys_read_register(SYMPHONY_IO_PA(0xBF50A104), &value);
    value &= ~(0x3);//clear bit[1:0]
    value |= (0x1 << 0);//bit[1:0] set 1
    mt_sys_write_register(SYMPHONY_IO_PA(0xBF50A104), value);
    //printf("gpe clk high\n");
#endif
}

static void gpe_core_clk_low(void)
{
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)

    volatile mt_u32 value = 0;
    mt_sys_read_register(SYMPHONY_IO_PA(0xbf50f818), &value);
    value |= (0x1 << 7);//bit7 set 1
    mt_sys_write_register(SYMPHONY_IO_PA(0xbf50f818), value);

    mt_sys_read_register(SYMPHONY_IO_PA(0xBF50A104), &value);
    value &= ~(0x3);//clear bit[1:0]
    value |= (0x0 << 0);//bit[1:0] set 0
    mt_sys_write_register(SYMPHONY_IO_PA(0xBF50A104), value);
    //printf("gpe clk low\n");
#endif
}
#endif

mt_s32 GpeOsiBeginJob(TDE_HANDLE *ps32Handle)
{
    ulong i = 0;
    mt_u32 data = 0;

    MT_INFO_TDE("----------------in------------\n");
    if (NULL == ps32Handle)
    {
        return MT_ERR_TDE_NULL_PTR;
    }

    /* find handle */
    for(i=0; i < GPE_MAX_HANDLE; i++)
    {
        if(!g_hdl_gpe[i].bInUse)
        {
            break;
        }
    }

    /* no handle */
    if(GPE_MAX_HANDLE == i)
    {
        return MT_ERR_TDE_INVALID_HANDLE;
    }
    mt_sys_read_register(SYMPHONY_IO_PA(0xBF138140), &data);  //read auto start(auto clk)
    if(data &  (0x1 << 3))
    {
        mt_sys_read_register(SYMPHONY_IO_PA(0xBF138148), &data);  //read auto start(auto clk)
        data |=  (0x1 << 3);  //enable gra auto start to enbale png IP clk start
        mt_sys_write_register(SYMPHONY_IO_PA(0xBF138148), data);
    }

    //gpe_core_clk_high();

    memset(&g_hdl_gpe[i], 0, sizeof(hdl_gpe_t));

    g_hdl_gpe[i].bInUse = MT_TRUE;

    MT_INFO_TDE("gpe handle=%d\n", i);

    *ps32Handle = (TDE_HANDLE)(i + 1);

    gpe_write_register(GPE_ARIA_GRA_INT_EN, 0);
    MT_INFO_TDE("----------------out------------\n");
    return MT_SUCCESS;

}

mt_s32 GpeOsiEndJob(TDE_HANDLE s32Handle, MT_BOOL bBlock, mt_u32 u32TimeOut,
        MT_BOOL bSync, TDE_FUNC_CB pFuncComplCB, mt_void *pFuncPara)
{
    MT_INFO_TDE("----------------in------------\n");

    if ((s32Handle <= 0) || (s32Handle > GPE_MAX_HANDLE))
    {
        //        GPE_ERROR("Invalid handle:%d!\n", s32Handle);
        return MT_ERR_TDE_INVALID_HANDLE;
    }

    if(!g_hdl_gpe[s32Handle - 1].bInUse)
    {
        //        GPE_ERROR("handle %d not in use!\n", s32Handle);
        return MT_ERR_TDE_INVALID_HANDLE;
    }

    /* clear instance*/
    memset(&g_hdl_gpe[s32Handle-1], 0, sizeof(hdl_gpe_t));
    //gpe_core_clk_low();
    //    gpe_write_register(GPE_ARIA_GRA_INT_EN, 0x80000000);
    MT_INFO_TDE("----------------out------------\n");
    return MT_SUCCESS;
}

//----------fix bug 99703 beg, add soft scale when src rect is less than 9x9

#define MIN_W 9
#define MIN_H 9
static void ver_scale_no_filter_dw(mt_u32 *p_inImg,
                             mt_u32 *p_outImg,
                             mt_u32 imgW,
                             mt_u32 imgH,
                             mt_u32 nHSize)
{
    unsigned int i = 0, j = 0, nRow = 0;

    for(j = 0; j < nHSize; j++)
    {
        nRow = j * imgH / nHSize;
        //    if(nRow < 0)
        //      nRow = - nRow;
        if(nRow >= imgH)
            nRow = imgH - 2 - (nRow - imgH);
        for(i = 0; i < imgW; i ++)
            p_outImg[j * imgW + i] = p_inImg[nRow * imgW + i];
    }
}

static void hori_scale_no_filter_dw(mt_u32 *p_inImg,
                              mt_u32 *p_outImg,
                              mt_u32 imgW,
                              mt_u32 imgH,
                              mt_u32 nWSize)
{
  mt_u32 i = 0, j = 0, nCol = 0;

    for(i = 0; i < nWSize; i++)
    {
        nCol = i * imgW / nWSize;

        //    if(nCol < 0)
        //      nCol = - nCol;
        if(nCol >= imgW)
            nCol = imgW - 2 - (nCol - imgW);
        for(j = 0; j < imgH; j++)
            p_outImg[j * nWSize + i] = p_inImg[j * imgW + nCol];
    }
}

static void ver_scale_no_filter_c(unsigned char *p_inImg,
        unsigned char *p_outImg,
        unsigned int imgW,
        unsigned int imgH,
        unsigned int nHSize)
{
    unsigned int i = 0, j = 0, nRow = 0;

    for(j = 0; j < nHSize; j++)
    {
        nRow = j * imgH / nHSize;

        //    if(nRow < 0)
        //      nRow = - nRow;
        if(nRow >= imgH)
            nRow = imgH - 2 - (nRow - imgH);
        for(i = 0; i < imgW; i ++)
            p_outImg[j * imgW + i] = p_inImg[nRow * imgW + i];
    }
}

static void hori_scale_no_filter_c(unsigned char *p_inImg,
        unsigned char *p_outImg,
        unsigned int imgW,
        unsigned int imgH,
        unsigned int nWSize)
{
    unsigned int i = 0, j = 0, nCol = 0;
    for(i = 0; i < nWSize; i++)
    {
        nCol = i * imgW / nWSize;

        //    if(nCol < 0)
        //      nCol = - nCol;
        if(nCol >= imgW)
            nCol = imgW - 2 - (nCol - imgW);
        for(j = 0; j < imgH; j++)
            p_outImg[j * nWSize + i] = p_inImg[j * imgW + nCol];
    }
}

static void ver_scale_no_filter_w(mt_u16 *p_inImg,
        mt_u16 *p_outImg,
        unsigned int imgW,
        unsigned int imgH,
        unsigned int nHSize)
{
    unsigned int i = 0, j = 0, nRow = 0;

    for(j = 0; j < nHSize; j++)
    {
        nRow = j * imgH / nHSize;

        //    if(nRow < 0)
        //      nRow = - nRow;
        if(nRow >= imgH)
            nRow = imgH - 2 - (nRow - imgH);
        for(i = 0; i < imgW; i ++)
        {
            p_outImg[j * imgW + i] = p_inImg[nRow * imgW + i];
        }
    }
}

static void hori_scale_no_filter_w(mt_u16 *p_inImg,
        mt_u16 *p_outImg,
        unsigned int imgW,
        unsigned int imgH,
        unsigned int nWSize)
{
    unsigned int i = 0, j = 0, nCol = 0;

    for(i = 0; i < nWSize; i++)
    {
        nCol = i * imgW / nWSize;

        //    if(nCol < 0)
        //      nCol = - nCol;
        if(nCol >= imgW)
            nCol = imgW - 2 - (nCol - imgW);
        for(j = 0; j < imgH; j++)
        {
            p_outImg[j * nWSize + i] = p_inImg[j * imgW + nCol];
        }
    }
}

static mt_s32 soft_scale_to_mid_region(TDE2_SURFACE_S *pstForeGround,
        rect_vsb_t *p_src_rect,
        mt_u32 dw,
        mt_u32 dh,
        TDE2_SURFACE_S *pstMidSurface)
{
	mt_u32 *ps_32 = NULL;
	mt_u16 *ps_16 = NULL;
	mt_u8 *ps_8 = NULL;
	mt_u32 *ptr_32 = NULL;
	mt_u16 *ptr_16 = NULL;
	mt_u8 *ptr_8 = NULL;
	mt_u32 *pd_32 = NULL;
	mt_u16 *pd_16 = NULL;
	mt_u8 *pd_8 = NULL;
	mt_u32 *pmid_32 = NULL;
	mt_u16 *pmid_16 = NULL;
	mt_u8 *pmid_8 = NULL;
	MT_BOOL alpha_en;
	mt_u32 bpp = get_bpp(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en));
	mt_u32 pixel_stride = pstForeGround->u32Stride / (bpp / 8);
	mt_u32 i = 0;
	mt_u32 j = 0;
	pos_t pos = {0};
	mt_u32 sw = p_src_rect->w;
	mt_u32 sh = p_src_rect->h;
	mt_mmz_buf_s  psMBuf;
	mt_mmz_buf_s  midMBuf;
	ulong size0 = 0;
	ulong size1 = 0;

    memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));
    memset(&midMBuf, 0x00, sizeof(mt_mmz_buf_s));

	if(bpp == 32)
	{
		ptr_32 = (mt_u32 *)pstForeGround->u32VirAddr;
		pd_32 = (mt_u32 *)pstMidSurface->u32VirAddr;

		size0 = sw * sh * 4;
		if((g_psMBuf.bufsize > size0 + g_addr_offset) && (g_psMBuf.user_viraddr))
		{
			ps_32 = (mt_u32 *)((ulong)g_psMBuf.user_viraddr + g_addr_offset);
		}
		else
		{
            strncpy(psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
			psMBuf.bufsize = sw * sh * 4;
			mt_mmz_malloc(&psMBuf);
			ps_32 = (mt_u32*)psMBuf.user_viraddr;
			if(ps_32 == NULL)
			{
			  MT_INFO_TDE("\r\n malloc memory failed:%s, %d", __FUNCTION__, __LINE__);
			  return MT_ERR_TDE_NO_MEM;
			}
		}
		for(i = 0; i < sh; i++)
			for(j = 0; j < sw; j++)
		{
			pos.x = j + p_src_rect->x;
			pos.y = i + p_src_rect->y;

			ps_32[i * sw + j] = ptr_32[pixel_stride * pos.y + pos.x];
		}

		if((sw < dw) && (sh < dh))
		{
		  size1 = dw * sh * 4;
		  if((g_psMBuf.bufsize > (size0 + size1 + g_addr_offset)) && (g_psMBuf.user_viraddr))
		  {
	  	  	  pmid_32 = ps_32 + size0;
		  }
		  else
		  {
              strncpy(midMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
	   		  midMBuf.bufsize = dw * sh * 4;
	   		  mt_mmz_malloc(&midMBuf);
	   		  pmid_32 = (mt_u32*)midMBuf.user_viraddr;
			  if(pmid_32 == NULL)
			  {
			  	if(psMBuf.user_viraddr)
				    mt_mmz_free(&psMBuf);
				MT_INFO_TDE("\r\n malloc memory failed:%s, %d", __FUNCTION__, __LINE__);
				return MT_ERR_TDE_NO_MEM;
			  }
		  }
		  hori_scale_no_filter_dw(ps_32,
									pmid_32,
									sw,
									sh,
									dw);


            ver_scale_no_filter_dw(pmid_32,
                    pd_32,
                    dw,
                    sh,
                    dh);
            if(midMBuf.user_viraddr)
                mt_mmz_free(&midMBuf);
        }
        else if(sw < dw)
        {
            hori_scale_no_filter_dw(ps_32,
                    pd_32,
                    sw,
                    sh,
                    dw);


        }
        else
        {
            ver_scale_no_filter_dw(ps_32,
                    pd_32,
                    dw,
                    sh,
                    dh);
        }
        if(psMBuf.user_viraddr)
            mt_mmz_free(&psMBuf);
    }
    else if(bpp == 16)
    {
        ptr_16 = (mt_u16 *)pstForeGround->u32VirAddr;
        pd_16 = (mt_u16 *)pstMidSurface->u32VirAddr;
        size0 = sw * sh * 2;
        if((g_psMBuf.bufsize > size0 + g_addr_offset) && (g_psMBuf.user_viraddr))
        {
            ps_16 = (mt_u16 *)((ulong)g_psMBuf.user_viraddr + g_addr_offset);
        }
        else
        {
            strncpy(psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
            psMBuf.bufsize = sw * sh * 2;
            mt_mmz_malloc(&psMBuf);
            ps_16 = (mt_u16*)psMBuf.user_viraddr;
            if(ps_16 == NULL)
            {
                MT_INFO_TDE("\r\n malloc memory failed:%s, %d", __FUNCTION__, __LINE__);
                return MT_ERR_TDE_NO_MEM;
            }
        }
        for(i = 0; i < sh; i++)
            for(j = 0; j < sw; j++)
            {
                pos.x = j + p_src_rect->x;
                pos.y = i + p_src_rect->y;

                ps_16[i * sw + j] = ptr_16[pixel_stride * pos.y + pos.x];
            }

        if((sw < dw) && (sh < dh))
        {
            size1 = dw * sh * 2;
            if((g_psMBuf.bufsize > size0 + size1 + g_addr_offset) && (g_psMBuf.user_viraddr))
            {
                pmid_16 = ps_16 + size0;
            }
            else
            {
                strncpy(midMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
                midMBuf.bufsize = dw * sh * 2;
                mt_mmz_malloc(&midMBuf);
                pmid_16 = (mt_u16*)midMBuf.user_viraddr;
                if(pmid_16 == NULL)
                {
                    if(psMBuf.user_viraddr)
                        mt_mmz_free(&psMBuf);
                    MT_INFO_TDE("\r\n malloc memory failed:%s, %d", __FUNCTION__, __LINE__);
                    return MT_ERR_TDE_NO_MEM;
                }
            }
            hori_scale_no_filter_w(ps_16,
                    pmid_16,
                    sw,
                    sh,
                    dw);


            ver_scale_no_filter_w(pmid_16,
                    pd_16,
                    dw,
                    sh,
                    dh);
            if(midMBuf.user_viraddr)
                mt_mmz_free(&midMBuf);
        }
        else if(sw < dw)
        {
            hori_scale_no_filter_w(ps_16,
                    pd_16,
                    sw,
                    sh,
                    dw);


        }
        else
        {
            ver_scale_no_filter_w(ps_16,
                    pd_16,
                    dw,
                    sh,
                    dh);
        }
        if(psMBuf.user_viraddr)
            mt_mmz_free(&psMBuf);
    }
    else if(bpp == 8)
    {
        ptr_8 = (mt_u8 *)pstForeGround->u32VirAddr;
        pd_8 = (mt_u8 *)pstMidSurface->u32VirAddr;
        size0 = sw * sh;
        if((g_psMBuf.bufsize > size0 + g_addr_offset) && (g_psMBuf.user_viraddr))
        {
            ps_8 = g_psMBuf.user_viraddr + g_addr_offset;
        }
        else
        {
            strncpy(psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
            psMBuf.bufsize = sw * sh;
            mt_mmz_malloc(&psMBuf);
            ps_8 = (mt_u8*)psMBuf.user_viraddr;
            if(ps_8 == NULL)
            {
                MT_INFO_TDE("\r\n malloc memory failed:%s, %d", __FUNCTION__, __LINE__);
                return MT_ERR_TDE_NO_MEM;
            }
        }
        for(i = 0; i < sh; i++)
            for(j = 0; j < sw; j++)
            {
                pos.x = j + p_src_rect->x;
                pos.y = i + p_src_rect->y;

                ps_8[i * sw + j] = ptr_8[pixel_stride * pos.y + pos.x];
            }

        if((sw < dw) && (sh < dh))
        {
            size1 = dw * sh;
            if((g_psMBuf.bufsize > size0 + size1 + g_addr_offset) && (g_psMBuf.user_viraddr))
            {
                pmid_8 = ps_8 + size0;
            }
            else
            {
                strncpy(midMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
                midMBuf.bufsize = dw * sh;
                mt_mmz_malloc(&midMBuf);
                pmid_8 = (mt_u8*)midMBuf.user_viraddr;
                if(pmid_8 == NULL)
                {
                    if(psMBuf.user_viraddr)
                        mt_mmz_free(&psMBuf);
                    MT_INFO_TDE("\r\n malloc memory failed:%s, %d", __FUNCTION__, __LINE__);
                    return MT_ERR_TDE_NO_MEM;
                }
            }
            hori_scale_no_filter_c(ps_8,
                    pmid_8,
                    sw,
                    sh,
                    dw);


            ver_scale_no_filter_c(pmid_8,
                    pd_8,
                    dw,
                    sh,
                    dh);
            if(midMBuf.user_viraddr)
                mt_mmz_free(&midMBuf);
        }
        else if(sw < dw)
        {
            hori_scale_no_filter_c(ps_8,
                    pd_8,
                    sw,
                    sh,
                    dw);


        }
        else
        {
            ver_scale_no_filter_c(ps_8,
                    pd_8,
                    dw,
                    sh,
                    dh);
        }
        if(psMBuf.user_viraddr)
            mt_mmz_free(&psMBuf);
    }
    else
    {
        MT_INFO_TDE("\r\n not support format yet,%s, %d", __FUNCTION__, __LINE__);
        return MT_ERR_TDE_INVALID_PARA;
    }

    return MT_SUCCESS;

}
//----------fix bug 99703 end

static aria_rotator_op_t rotate_mirror_tab[TDE2_MIRROR_BUTT][TDE2_ROTATE_BUTT] = {
    {GPE_ARIA_NO_OP, GPE_ARIA_VERT_MIRROR_TRANS, GPE_ARIA_HORI_VERT_MIRROR, GPE_ARIA_HORI_MIRROR_TRANS},
    {GPE_ARIA_HORI_MIRROR, GPE_ARIA_HORI_VERT_MIRROR_TRANS, GPE_ARIA_VERT_MIRROR, GPE_ARIA_TRANS},
    {GPE_ARIA_VERT_MIRROR, GPE_ARIA_TRANS, GPE_ARIA_HORI_MIRROR, GPE_ARIA_HORI_VERT_MIRROR_TRANS},
    {GPE_ARIA_HORI_VERT_MIRROR,GPE_ARIA_HORI_MIRROR_TRANS,GPE_ARIA_NO_OP,GPE_ARIA_VERT_MIRROR_TRANS}
};

mt_s32 GpeOsiBlit(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
        TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect, TDE2_SURFACE_S* pstDst,
        TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt)
{
    mt_s32 ret = 0;
    aria_param_t param = {0};
    rect_vsb_t src_rect = {0};
    rect_vsb_t dst_rect = {0};
    rect_vsb_t bg_rect = {0};
    MT_BOOL scale_flag = MT_FALSE;
    MT_BOOL need_tmpbuf = MT_FALSE;
    mt_s32 scale_coef[6] = {0};
    scale_type_t scale_type = SCALE_TYPE_MAX;
    phys_addr_t p_tmpbuf = 0;
    pix_fmt_t tmpbuf_fmt = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch = 0;
    phys_addr_t p_mask_buf = 0;
    mt_u32 maskbuf_pitch = 0;
    rect_vsb_t tmp_src_rect = {0};
    rect_vsb_t tmp_dst_rect = {0};
    mt_u32 bpp = 0;
    MT_BOOL src_ck_en = MT_FALSE;
    GPE_COLORFMT_CATEGORY_E enFmtCategory;
    MT_BOOL paint_src = MT_FALSE;
    MT_BOOL alpha_en = MT_FALSE;
    mt_u32 size = 0;
    mt_u32 tmp_buf_used = 0;
    mt_u32 msk_buf_used = 0;

    MT_INFO_TDE("----------------in------------\n");

    if(pstDst == NULL || pstDstRect == NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstOpt->enPaint == MT_TRUE && pstOpt->stPaintOpt.paint_type != TDE2_PAINT_TYPE_PATTERN)
        paint_src = MT_TRUE;
    else if(pstForeGroundRect == NULL || pstForeGround == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstForeGroundRect != NULL)
    {
        if((pstForeGroundRect->u32Width == 0) && (pstForeGroundRect->u32Height == 0) && (pstForeGround != NULL))
        {
            src_rect.x = 0;
            src_rect.y = 0;
            src_rect.w = pstForeGround->u32Width;
            src_rect.h = pstForeGround->u32Height;
        }
        else
        {
            src_rect.x = (mt_u32)(pstForeGroundRect->s32Xpos);
            src_rect.y = (mt_u32)(pstForeGroundRect->s32Ypos);
            src_rect.w = pstForeGroundRect->u32Width;
            src_rect.h = pstForeGroundRect->u32Height;
        }
    }

    if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
    {
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.w = pstDst->u32Width;
        dst_rect.h = pstDst->u32Height;
    }
    else
    {
        dst_rect.x = (mt_u32)(pstDstRect->s32Xpos);
        dst_rect.y = (mt_u32)(pstDstRect->s32Ypos);
        dst_rect.w = pstDstRect->u32Width;
        dst_rect.h = pstDstRect->u32Height;
    }

    if(pstBackGroundRect != NULL && pstBackGround != NULL)
    {
        if((pstBackGroundRect->u32Width == 0) && (pstBackGroundRect->u32Height == 0))
        {
            bg_rect.x = 0;
            bg_rect.y = 0;
            bg_rect.w = pstBackGround->u32Width;
            bg_rect.h = pstBackGround->u32Height;
        }
        else
        {
            bg_rect.x = (mt_u32)(pstBackGroundRect->s32Xpos);
            bg_rect.y = (mt_u32)(pstBackGroundRect->s32Ypos);
            bg_rect.w = pstBackGroundRect->u32Width;
            bg_rect.h = pstBackGroundRect->u32Height;
        }
    }

    scale_flag = pstOpt->bResize || pstOpt->b3dResize;
    if(((src_rect.w != dst_rect.w) ||(src_rect.h != dst_rect.h)) && (pstOpt->enRotator == TDE2_ROTATE_NONE)
            && (!(pstOpt->enPaint == MT_TRUE && pstOpt->stPaintOpt.paint_type == TDE2_PAINT_TYPE_PATTERN)))
        scale_flag = MT_TRUE;
    if(pstForeGround == NULL)
        scale_flag = MT_FALSE;

    if(ChipVersion <= MT_CHIP_SYMPHONY_A2)
    {
        //----------fix bug 99703 beg, add soft scale when src rect is less than 9x9
        if((scale_flag == MT_TRUE) && ((src_rect.w < MIN_W) || (src_rect.h < MIN_H)))
        {
            mt_u32 dw = 0;
            mt_u32 dh = 0;
            TDE2_SURFACE_S mid_surface = {0};
            TDE2_RECT_S mid_rect = {0};
            mt_mmz_buf_s  psMBuf;

            memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));

            if(pstForeGround == NULL)
                return MT_ERR_TDE_NULL_PTR;

            bpp = get_bpp(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en));
            if(((bpp != 8) && (bpp != 16) && (bpp != 32))
                    || (is_semiplanner(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en))))
            {
                MT_INFO_TDE("\r\n bpp or fmt not supported :%s, %d", __FUNCTION__, __LINE__);
                return MT_ERR_TDE_INVALID_PARA;
            }

            dw = (src_rect.w > MIN_W) ? src_rect.w : MIN_W;
            dh = (src_rect.h > MIN_H) ? src_rect.h : MIN_H;

            memcpy(&mid_surface, pstForeGround, sizeof(TDE2_SURFACE_S));

            size = dw * dh * bpp / 8;
            if((g_psMBuf.bufsize > size) && g_psMBuf.user_viraddr)
            {
                mid_surface.u32PhyAddr = g_psMBuf.phyaddr;
                mid_surface.u32VirAddr = (ulong)g_psMBuf.user_viraddr;
                g_addr_offset = size;
            }
            else
            {
                strncpy(psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
                psMBuf.bufsize = dw * dh * bpp / 8;
                mt_mmz_malloc(&psMBuf);
                mid_surface.u32PhyAddr = psMBuf.phyaddr;
                mid_surface.u32VirAddr = (ulong)psMBuf.user_viraddr;
                if(mid_surface.u32PhyAddr == 0)
                {
                    MT_INFO_TDE("\r\n malloc failed :%s, %d", __FUNCTION__, __LINE__);
                    return MT_ERR_TDE_NO_MEM;
                }

            }
            mid_surface.u32Stride = dw * bpp / 8;
            mid_surface.u32Width = dw;
            mid_surface.u32Height = dh;

            ret = soft_scale_to_mid_region(pstForeGround, &src_rect, dw, dh, &mid_surface);
            if(ret != MT_SUCCESS)
            {
                if(psMBuf.user_viraddr)
                    mt_mmz_free(&psMBuf);
                g_addr_offset = 0;
                return ret;
            }

            mid_rect.s32Xpos = 0;
            mid_rect.s32Ypos = 0;
            mid_rect.u32Width = mid_surface.u32Width;
            mid_rect.u32Height = mid_surface.u32Height;
            ret = GpeOsiBlit(s32Handle, pstBackGround, pstBackGroundRect,
                    &mid_surface, &mid_rect, pstDst,
                    pstDstRect, pstOpt);
            if(psMBuf.user_viraddr)
                mt_mmz_free(&psMBuf);
            g_addr_offset = 0;
            return ret;
        }
        //------------fix bug 99703 end
    }

    if(scale_flag == MT_TRUE)
    {
        pos_t dst00;
        pos_t dst10;
        pos_t dst01;
        pos_t dst11;
        need_tmpbuf = MT_TRUE;
        if(pstOpt->b3dResize == MT_FALSE)
        {
            dst00.x = dst_rect.x;
            dst00.y = dst_rect.y;
            dst10.x = dst_rect.x + dst_rect.w;
            dst10.y = dst_rect.y;
            dst01.x = dst_rect.x;
            dst01.y = dst_rect.y + dst_rect.h;
            dst11.x = dst_rect.x + dst_rect.w;
            dst11.y = dst_rect.y + dst_rect.h;
        }
        else
        {
            dst00.x = pstOpt->Scale3dDst.pos00.x;
            dst00.y = pstOpt->Scale3dDst.pos00.y;
            dst10.x = pstOpt->Scale3dDst.pos10.x;
            dst10.y = pstOpt->Scale3dDst.pos10.y;
            dst01.x = pstOpt->Scale3dDst.pos01.x;
            dst01.y = pstOpt->Scale3dDst.pos01.y;
            dst11.x = pstOpt->Scale3dDst.pos11.x;
            dst11.y = pstOpt->Scale3dDst.pos11.y;
        }

        aria_gpe_get_scale_coeff(&src_rect, &dst00, &dst10, &dst01, &dst11, scale_coef, &scale_type);
        if(scale_type == SCALE_TYPE_MAX)
        {
            MT_ERR_TDE("get scale coeff error!\n");
            return MT_ERR_TDE_INVALID_PARA;
        }
        MT_INFO_TDE("\r\n scale_type:%d", scale_type);
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
        if(scale_coef[5] == 0)
#else
    if(scale_coef[5] == 0
               && (src_rect.w != dst_rect.w)
               &&(src_rect.h != dst_rect.h))
#endif
        {
            need_tmpbuf = MT_FALSE;
        }
        else
#endif
        {
            if((scale_type == SCALE_ONLY_HORI_RECT) || (scale_type == SCALE_ONLY_HORI_TRAPZ))
                need_tmpbuf = MT_FALSE;
            else if(scale_type == SCALE_ONLY_VERT_RECT)
            {
                if((pstOpt->enAluCmd == TDE2_ALUCMD_NONE)
                        || ((pstOpt->enAluCmd & TDE2_ALUCMD_ROP)
                            && (pstOpt->enRopCode_Alpha == TDE2_ROP_COPYPEN) && (pstOpt->enRopCode_Color == TDE2_ROP_COPYPEN)))
                    need_tmpbuf = MT_FALSE;
            }
        }
    }


    MT_INFO_TDE("\r\n need_tmpbuf:%d", need_tmpbuf);
    if(need_tmpbuf == MT_TRUE && pstForeGround != NULL)
    {
        if(is_lut(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en)))
        {
            tmpbuf_fmt = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en);
            if(tmpbuf_fmt == PIX_FMT_RGBPALETTE1 || tmpbuf_fmt == PIX_FMT_RGBPALETTE2
                    || tmpbuf_fmt == PIX_FMT_RGBPALETTE4)
                tmpbuf_fmt = PIX_FMT_RGBPALETTE8;
        }
        else if(aria_get_color_space(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en)) == RGB_COLOR_SPACE)
            tmpbuf_fmt = PIX_FMT_ARGB8888;
        else
            tmpbuf_fmt = PIX_FMT_AYCBCR8888;
        bpp = get_bpp(tmpbuf_fmt);
        if(scale_type == SCALE_RECT)
        {
            tmp_src_rect.y = src_rect.y;
            tmp_src_rect.w = src_rect.w;
            tmp_src_rect.h = src_rect.h;
            if(src_rect.x >= 4)
            {
                tmp_src_rect.x = src_rect.x - 4;
                tmp_src_rect.w += 4;
            }
            else
            {
                tmp_src_rect.x = 0;
                tmp_src_rect.w += src_rect.x;
            }

            if(src_rect.x + src_rect.w + 4 < pstForeGround->u32Width)
                tmp_src_rect.w += 4;
            else
                tmp_src_rect.w += pstForeGround->u32Width - src_rect.x - src_rect.w;
        }
        else
        {
            tmp_src_rect.x = src_rect.x;
            tmp_src_rect.y = src_rect.y;
            tmp_src_rect.w = src_rect.w;
            tmp_src_rect.h = src_rect.h;
        }

        tmp_dst_rect.x = 0;
        tmp_dst_rect.y = 0;
        tmp_dst_rect.w = tmp_src_rect.w;
        tmp_dst_rect.h = dst_rect.h;

        tmpbuf_pitch = (tmp_dst_rect.w * bpp + 7) / 8 ;
        size = tmpbuf_pitch * tmp_dst_rect.h;
        if((g_psMBuf.bufsize > size + g_addr_offset) && g_psMBuf.user_viraddr)
        {
            p_tmpbuf = g_psMBuf.phyaddr + g_addr_offset;
            g_addr_offset += size;
        }
        else
        {
            p_tmpbuf = mt_mmz_new(tmpbuf_pitch * tmp_dst_rect.h, 8, NULL, MOD_NAME);
            MT_INFO_TDE("\r\n p_tmpbuf:0x%08x", p_tmpbuf);
            if(p_tmpbuf == 0)
            {
                g_addr_offset = 0;
                MT_ERR_TDE("\r\n malloc p_tmpbuf failed!");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }
            tmp_buf_used = 1;
        }
        if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_FOREGROUND)
        {
            src_ck_en = MT_TRUE;
        }
        else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
        {
            if(pstOpt->stColorKeyCfg.enColorKey_fg)
                src_ck_en = MT_TRUE;
        }

        if((src_ck_en) || (scale_coef[5] == 1))
        {
#ifdef CONFIG_MT_FPGA_GPE
            if(g_debug.mask_test_en == TRUE)
                maskbuf_pitch = tmp_dst_rect.w + g_debug.comset[1];
            else
                maskbuf_pitch = tmp_dst_rect.w;
#else        
            maskbuf_pitch = tmp_dst_rect.w;
#endif
            size = maskbuf_pitch * tmp_dst_rect.h;
            if((g_psMBuf.bufsize > size + g_addr_offset) && g_psMBuf.user_viraddr)
            {
                p_mask_buf = g_psMBuf.phyaddr + g_addr_offset;
                g_addr_offset += size;
            }
            else
            {
                p_mask_buf = mt_mmz_new(maskbuf_pitch * tmp_dst_rect.h, 64, NULL, MOD_NAME);
                if(p_mask_buf == 0)
                {
                    g_addr_offset = 0;
                    MT_ERR_TDE("\r\n malloc p_mask_buf failed!");
                    if(tmp_buf_used)
                    {
                        mt_mmz_delete(p_tmpbuf);
                        p_tmpbuf = 0;
                    }
                    return MT_ERR_TDE_UNSUPPORTED_OPERATION;
                }
                msk_buf_used = 1;
            }
        }

        ret = gpe_2d_vert_scale(s32Handle, &tmp_src_rect, &tmp_dst_rect, pstForeGround, pstOpt, p_tmpbuf,
                tmpbuf_pitch, tmpbuf_fmt,p_mask_buf, maskbuf_pitch, src_ck_en, scale_coef);
        if(ret != MT_TRUE)
        {
            g_addr_offset = 0;
            if(tmp_buf_used)
            {
                mt_mmz_delete(p_tmpbuf);
                tmp_buf_used = 0;
                p_tmpbuf = 0;
            }
            if(msk_buf_used)
            {
                mt_mmz_delete(p_mask_buf);
                msk_buf_used  = 0;
                p_mask_buf = 0;
            }
            return MT_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    }

    param_init(&param);

    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    if((scale_flag) && (need_tmpbuf))
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = p_tmpbuf;
        param.src_img.pitch = tmpbuf_pitch;
        param.src_img.width = tmp_dst_rect.w;
        param.src_img.height = tmp_dst_rect.h;
        param.src_img.pix_format = tmpbuf_fmt;
        param.src_img.rect.x = src_rect.x - tmp_src_rect.x;
        param.src_img.rect.y = 0;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = dst_rect.h;
        if(p_mask_buf != 0)
        {
            param.src_with_mask = MT_TRUE;
            param.src_mask_buf = p_mask_buf;
            param.src_mask_pitch = maskbuf_pitch;
        }
    }
    else if(paint_src == MT_FALSE && pstForeGround != NULL)
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = pstForeGround->u32PhyAddr;
        param.src_img.pitch = pstForeGround->u32Stride;
        param.src_img.width = pstForeGround->u32Width;
        param.src_img.height = pstForeGround->u32Height;
        param.src_img.pix_format = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &param.src_img.alpha_ch_en);
        param.src_img.rect.x = src_rect.x;
        param.src_img.rect.y = src_rect.y;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = src_rect.h;

        if((param.src_img.pix_format == PIX_FMT_TILE)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
        {
            param.src_img.chroma_addr = pstForeGround->u32CbCrPhyAddr;
            param.src_img.chroma_pitch = pstForeGround->u32CbCrStride;
#ifdef CONFIG_MT_FPGA_GPE			
             g_debug.chroma_vir_addr = pstForeGround->u32CbCrVirAddr;
#endif
        }
        if((param.src_img.pix_format >= PIX_FMT_XY) && (param.src_img.pix_format <= PIX_FMT_XYLC))
        {
            param.src_img.xylc_num = pstOpt->xylc_cmd_num;
            param.src_img.xylc_color = pstOpt->xylc_color;
        }
    }


    // ====== dst image attribute =====  //
    param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    param.dst_img.rect.x = dst_rect.x;
    param.dst_img.rect.y = dst_rect.y;
    param.dst_img.rect.w = dst_rect.w;
    param.dst_img.rect.h = dst_rect.h;

    if((pstBackGround != MT_NULL)&&(pstBackGroundRect != MT_NULL))
    {
        param.bg_img_en = MT_TRUE;
        param.bg_img.buf = pstBackGround->u32PhyAddr;
        param.bg_img.pitch = pstBackGround->u32Stride;
        param.bg_img.width = pstBackGround->u32Width;
        param.bg_img.height = pstBackGround->u32Height;
        param.bg_img.pix_format = aria_gpe_convert_fmt(pstBackGround->enColorFmt, &param.bg_img.alpha_ch_en);
        param.bg_img.rect.x = bg_rect.x;
        param.bg_img.rect.y = bg_rect.y;
        param.bg_img.rect.w = bg_rect.w;
        param.bg_img.rect.h = bg_rect.h;
    }

    if(pstForeGround != NULL)
    {
        if((pstForeGround->pu8ClutPhyAddr != 0))
            param.src_img.palette_base = (phys_addr_t)pstForeGround->pu8ClutPhyAddr;
    }
    else
    {
        if((pstDst->pu8ClutPhyAddr != 0))
            param.dst_img.palette_base = (phys_addr_t)pstDst->pu8ClutPhyAddr;
    }
    if((scale_flag == MT_FALSE) || (need_tmpbuf == MT_FALSE))
    {
        if(pstOpt->stSurfaceCfg.enSrcFlip)
            param.src_img.negative_stride = MT_TRUE;

        if(pstForeGround != NULL)
        {
            if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_FOREGROUND)
            {
                enFmtCategory = GpeOsiGetFmtCategory(pstForeGround->enColorFmt);
                param.src_img.ck_en = MT_TRUE;
                param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
                param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
                param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);
                //        param.src_img.key_color_select = MASK_KEY_MATCH;
                param.src_img.key_color_select = pstOpt->enColorKeySelect;
            }
            else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
            {
                param.src_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_fg;
                if(param.src_img.ck_en)
                {
                    enFmtCategory = GpeOsiGetFmtCategory(pstForeGround->enColorFmt);
                    param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                    param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                    param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                    param.src_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_fg;
                }
            }
        }
    }

    if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_BACKGROUND)
    {
        enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
        param.dst_img.ck_en = MT_TRUE;
        param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);

        //        param.dst_img.key_color_select = MASK_KEY_MISMATCH;
        param.dst_img.key_color_select = pstOpt->enColorKeySelect;
    }
    else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
    {
        param.dst_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_bg;
        if(param.dst_img.ck_en)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
            param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_bg;
        }
    }

    if(pstOpt->stSurfaceCfg.enSrcGlobalAlpha)
    {
        param.src_img.plane_alpha_en = MT_TRUE;
        param.src_img.plane_alpha = pstOpt->u8GlobalAlpha;
    }

    if((pstOpt->stSurfaceCfg.enSrcPreMult))
    {
        param.src_img.alpha_pre_multed = MT_FALSE;
    }

    if((pstOpt->stSurfaceCfg.disSrcAlpha))
    {
        param.src_img.alpha_ch_en = MT_FALSE;
    }

    if(pstOpt->stSurfaceCfg.disDstAlpha)
    {
        if(param.bg_img_en)
            param.bg_img.alpha_ch_en = MT_FALSE;
        else
            param.dst_img.alpha_ch_en = MT_FALSE;
    }

    if(pstOpt->stSurfaceCfg.enDstPreMult)
    {
        if(param.bg_img_en)
            param.bg_img.alpha_pre_multed = FALSE;
        else
            param.dst_img.alpha_pre_multed = FALSE;
    }

    if(pstOpt->stSurfaceCfg.enDstFlip)
        param.dst_img.negative_stride = MT_TRUE;


    param.gpe_op = GPE_OP_NONE;

    if(scale_flag == MT_TRUE)
    {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
        if(scale_coef[5] == 0)
#else
    if(scale_coef[5] == 0
        && (src_rect.w != dst_rect.w)
        && (src_rect.h != dst_rect.h))
#endif
        {
            param.gpe_op |= GPE_OP_SCALE;
            param.coef[0] = scale_coef[0];
            param.coef[1] = scale_coef[1];
            param.coef[2] = scale_coef[2];
            param.coef[3] = scale_coef[3];
            param.coef[4] = scale_coef[4];
            param.coef[5] = scale_coef[5];
        }
        else
#endif
        {
            if((pstOpt->b3dResize == MT_TRUE) || (param.src_img.rect.w != param.dst_img.rect.w)
                    || ((scale_type == SCALE_ONLY_VERT_RECT) && (need_tmpbuf == MT_FALSE)))
            {
                if((scale_type == SCALE_ONLY_VERT_RECT) && (need_tmpbuf == MT_FALSE))
                {
                    param.scale_mod = SCALE_VERT_BLK_OUT;
                    param.gpe_op |= GPE_OP_ROTATE | GPE_OP_SCALE_VERT;
                    param.rotator_op = GPE_ARIA_NO_OP;
                }
                else
                {
                    param.scale_mod = SCALE_HORI_LINE_OUT;
                    if(pstOpt->b3dResize == MT_TRUE)
                        param.gpe_op |= GPE_OP_SCALE_TRAPZ;
                    else
                        param.gpe_op |= GPE_OP_SCALE_HORI;
                }
                param.gpe_op |= GPE_OP_SCALE;
                param.coef[0] = scale_coef[0];
                param.coef[1] = scale_coef[1];
                param.coef[2] = scale_coef[2];
                param.coef[3] = scale_coef[3];
                param.coef[4] = scale_coef[4];
                param.coef[5] = scale_coef[5];
            }
        }
    }
    else
    {
        if(pstOpt->enGsBlur)
        {
            param.gpe_op |= GPE_OP_BLUR;
            param.blur_tap = (mt_u32)(pstOpt->stBlurOpt.blur_level * 2 + 3);
        }
    }

    param.rotator_op = rotate_mirror_tab[pstOpt->enMirror][pstOpt->enRotator];
    if(param.rotator_op != GPE_ARIA_NO_OP)
        param.gpe_op |= GPE_OP_ROTATE;


    if(pstOpt->enAluCmd == TDE2_ALUCMD_NONE)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id =  ROP_COPYPEN;
        param.rop.rop_c_id =  ROP_COPYPEN;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_ROP)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id = aria_gpe_convert_rop(pstOpt->enRopCode_Alpha);
        param.rop.rop_c_id = aria_gpe_convert_rop(pstOpt->enRopCode_Color);
        param.rop.rop_pattern = pstOpt->u32Colorize;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_BLEND)
    {
        param.gpe_op |= GPE_OP_BLEND;

        switch (pstOpt->stBlendOpt.eBlendCmd) {
            /**< fs: sa      fd: 1.0-sa */
        case TDE2_BLENDCMD_NONE:
            {
                param.blend.src_blend_fact = GL_SRC_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0     fd: 0.0 */
        case TDE2_BLENDCMD_CLEAR:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 0.0 */
        case TDE2_BLENDCMD_SRC:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCOVER:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0 */
        case TDE2_BLENDCMD_DSTOVER:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: da      fd: 0.0 */
        case TDE2_BLENDCMD_SRCIN:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: sa */
        case TDE2_BLENDCMD_DSTIN:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 0.0 */
        case TDE2_BLENDCMD_SRCOUT:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_DSTOUT:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: da      fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCATOP:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: sa */
        case TDE2_BLENDCMD_DSTATOP:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0     fd: 1.0 */
        case TDE2_BLENDCMD_ADD:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0-sa */
        case TDE2_BLENDCMD_XOR:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0  fd: 1.0*/
        case TDE2_BLENDCMD_DST:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /* user parameter*/ /*CNcomment:  ???????? */
        case TDE2_BLENDCMD_CONFIG:
        default:
            {
                param.blend.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendMode); // src2 is aria src1
                param.blend.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendMode); // src1 is aria src2
                break;
            }
        }

        if(pstOpt->stBlendOpt.bBlendModeAlphaEnable && (pstOpt->stBlendOpt.eBlendCmd == TDE2_BLENDCMD_CONFIG))
        {
            param.blend_alpha.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendModeAlpha);
            param.blend_alpha.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendModeAlpha);
        }
        else
        {
            param.blend_alpha.src_blend_fact = param.blend.src_blend_fact;
            param.blend_alpha.dst_blend_fact = param.blend.dst_blend_fact;
        }
        if((pstForeGround != NULL) && (pstOpt->stBlendOpt.bGlobalAlphaEnable))
        {
            param.src_img.plane_alpha_en = MT_TRUE;
            param.src_img.plane_alpha = pstOpt->u8GlobalAlpha;
        }

        if((pstForeGround != NULL) && pstOpt->stBlendOpt.bSrc2AlphaPremulti)
        {
            param.src_img.alpha_pre_multed = MT_FALSE;
        }

        param.blend.demultiply_en = pstOpt->stBlendOpt.demultiply_en;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    if(pstOpt->enableClip)
    {
        param.clip_en = TRUE;
        param.clip.clip_mode = pstOpt->clipCfg.clip_mode;
        param.clip.clip_rect.s32Xpos = pstOpt->clipCfg.clip_rect.s32Xpos;
        param.clip.clip_rect.s32Ypos = pstOpt->clipCfg.clip_rect.s32Ypos;
        param.clip.clip_rect.u32Width = pstOpt->clipCfg.clip_rect.u32Width;
        param.clip.clip_rect.u32Height = pstOpt->clipCfg.clip_rect.u32Height;
        MT_INFO_TDE("<%s> enableClip : clip_mode : %d <%d %d %d %d>\n", __FUNCTION__, param.clip.clip_mode, param.clip.clip_rect.s32Xpos,
            param.clip.clip_rect.s32Ypos,  param.clip.clip_rect.u32Width, param.clip.clip_rect.u32Height);
    }
    else
        param.clip_en = FALSE;
#endif
    if(pstOpt->enableColorize)
    {
        param.colorize_en = TRUE;
        param.color = pstOpt->color;
    }

    if(pstOpt->stSurfaceCfg.enDstGlobalAlpha)
    {
        if(param.bg_img_en)
        {
            param.bg_img.plane_alpha_en = TRUE;
            param.bg_img.plane_alpha = pstOpt->u8DstGlobalAlpha;
        }
        else
        {
            param.dst_img.plane_alpha_en = TRUE;
            param.dst_img.plane_alpha = pstOpt->u8DstGlobalAlpha;      
        }
    }

    if(pstOpt->enPaint)
    {
        param.gpe_op |= GPE_OP_PAINT;
        memcpy(&param.paint, &pstOpt->stPaintOpt, sizeof(TDE2_PAINT_CFG_S));
    }
    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(tmp_buf_used)
        {
            mt_mmz_delete(p_tmpbuf);
            p_tmpbuf = 0;
            tmp_buf_used = 0;
            
        }
        if(msk_buf_used)
        {
            mt_mmz_delete(p_mask_buf);
            p_mask_buf = 0;
            msk_buf_used = 0;
        }
        g_addr_offset = 0;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    ret = aria_gpe_start(s32Handle);

    if(tmp_buf_used)
    {
        mt_mmz_delete(p_tmpbuf);
        p_tmpbuf = 0;
        tmp_buf_used = 0;        
    }
    if(msk_buf_used)
    {
        mt_mmz_delete(p_mask_buf);
        p_mask_buf = 0;
        msk_buf_used = 0;        
    }
    g_addr_offset = 0;
    return (ret == MT_TRUE ?MT_SUCCESS: MT_FAILURE);


}

mt_s32 GpeOsiBlit_3src(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstBackGround, TDE2_RECT_S  *pstBackGroundRect,
        TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
        TDE2_SURFACE_S* pstExGround, TDE2_RECT_S  *pstExGroundRect,
        TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt)
{
    mt_s32 ret = 0;
    aria_param_t param = {0};
    rect_vsb_t src_rect = {0};
    rect_vsb_t dst_rect = {0};
    rect_vsb_t ex_rect = {0};
    rect_vsb_t bg_rect = {0};
    MT_BOOL scale_flag = MT_FALSE;
    MT_BOOL need_tmpbuf = MT_FALSE;
    mt_s32 scale_coef[6] = {0};
    scale_type_t scale_type = SCALE_TYPE_MAX;
    phys_addr_t p_tmpbuf = 0;
    pix_fmt_t tmpbuf_fmt = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch = 0;
    phys_addr_t p_mask_buf = 0;
    mt_u32 maskbuf_pitch = 0;
    rect_vsb_t tmp_src_rect = {0};
    rect_vsb_t tmp_dst_rect = {0};
    mt_u32 bpp = 0;
    MT_BOOL src_ck_en = MT_FALSE;
    GPE_COLORFMT_CATEGORY_E enFmtCategory;
    MT_BOOL paint_src = MT_FALSE;
    MT_BOOL alpha_en;
    mt_u32 size = 0;
    mt_u32 tmp_buf_used = 0;
    mt_u32 msk_buf_used = 0;

    MT_INFO_TDE("----------------in------------\n");

#if 0
    // pstBackGround, pstBackGroundRect maybe is NULL
    printf(" handle=%x, bg.w=%d, bg.h=%d, fg.w=%d, fg.h=%d, dst.w=%d, dst.h=%d\n",s32Handle, (int)pstBackGround->u32Width, (int)pstBackGround->u32Height, (int)pstForeGround->u32Width, (int)pstForeGround->u32Height,
            (int)pstDst->u32Width, (int)pstDst->u32Height);
    printf(" bgRect.w=%d, bgRect.h=%d, fgRect.w=%d, fgRect.h=%d, dstRect.w=%d, dstRect.h=%d\n", (int)pstBackGroundRect->u32Width, (int)pstBackGroundRect->u32Height, (int)pstForeGroundRect->u32Width, (int)pstForeGroundRect->u32Height,
            (int)pstDstRect->u32Width, (int)pstDstRect->u32Height);
    printf(" bg.fmt=%d, fg.fmt=%d, dst.fmt=%d\n", pstBackGround->enColorFmt, pstForeGround->enColorFmt, pstDst->enColorFmt);
    printf(" pstOpt->enAluCmd=0x%x\n", pstOpt->enAluCmd);
    //pstOpt->enAluCmd = 1;
#endif

    if(pstDst == NULL || pstDstRect == NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstExGround == NULL || pstExGroundRect == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstOpt->enPaint == MT_TRUE && pstOpt->stPaintOpt.paint_type != TDE2_PAINT_TYPE_PATTERN)
        paint_src = MT_TRUE;
    else if(pstForeGroundRect == NULL || pstForeGround == NULL)
        return MT_ERR_TDE_NULL_PTR;


    if(pstForeGroundRect != NULL)
    {
        if((pstForeGroundRect->u32Width == 0) && (pstForeGroundRect->u32Height == 0) && (pstForeGround != NULL))
        {

            src_rect.x = 0;
            src_rect.y = 0;
            src_rect.w = pstForeGround->u32Width;
            src_rect.h = pstForeGround->u32Height;
        }
        else
        {
            src_rect.x = (mt_u32)(pstForeGroundRect->s32Xpos);
            src_rect.y = (mt_u32)(pstForeGroundRect->s32Ypos);
            src_rect.w = pstForeGroundRect->u32Width;
            src_rect.h = pstForeGroundRect->u32Height;
        }
    }
    if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
    {
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.w = pstDst->u32Width;
        dst_rect.h = pstDst->u32Height;
    }
    else
    {
        dst_rect.x = (mt_u32)(pstDstRect->s32Xpos);
        dst_rect.y = (mt_u32)(pstDstRect->s32Ypos);
        dst_rect.w = pstDstRect->u32Width;
        dst_rect.h = pstDstRect->u32Height;
    }

    if((pstExGroundRect->u32Width == 0) && (pstExGroundRect->u32Height == 0))
    {
        ex_rect.x = 0;
        ex_rect.y = 0;
        ex_rect.w = pstExGround->u32Width;
        ex_rect.h = pstExGround->u32Height;
    }
    else
    {
        ex_rect.x = (mt_u32)(pstExGroundRect->s32Xpos);
        ex_rect.y = (mt_u32)(pstExGroundRect->s32Ypos);
        ex_rect.w = pstExGroundRect->u32Width;
        ex_rect.h = pstExGroundRect->u32Height;
    }

    if(pstBackGroundRect != NULL && pstBackGround != NULL)
    {
        if((pstBackGroundRect->u32Width == 0) && (pstBackGroundRect->u32Height == 0))
        {
            bg_rect.x = 0;
            bg_rect.y = 0;
            bg_rect.w = pstBackGround->u32Width;
            bg_rect.h = pstBackGround->u32Height;
        }
        else
        {
            bg_rect.x = (mt_u32)(pstBackGroundRect->s32Xpos);
            bg_rect.y = (mt_u32)(pstBackGroundRect->s32Ypos);
            bg_rect.w = pstBackGroundRect->u32Width;
            bg_rect.h = pstBackGroundRect->u32Height;
        }
    }

    scale_flag = pstOpt->bResize || pstOpt->b3dResize;
    if(((src_rect.w != dst_rect.w) ||(src_rect.h != dst_rect.h)) && (pstOpt->enRotator == TDE2_ROTATE_NONE)
            && (!(pstOpt->enPaint == MT_TRUE && pstOpt->stPaintOpt.paint_type == TDE2_PAINT_TYPE_PATTERN)))
        scale_flag = MT_TRUE;
    if(pstForeGround == NULL)
        scale_flag = MT_FALSE;

    if(ChipVersion <= MT_CHIP_SYMPHONY_A2)
    {

        //----------fix bug 99703 beg, add soft scale when src rect is less than 9x9
        if((scale_flag == MT_TRUE) && ((src_rect.w < MIN_W) || (src_rect.h < MIN_H)))
        {
            mt_u32 dw = 0;
            mt_u32 dh = 0;
            TDE2_SURFACE_S mid_surface = {0};
            TDE2_RECT_S mid_rect = {0};
            mt_mmz_buf_s  psMBuf;

            memset(&psMBuf, 0x00, sizeof(mt_mmz_buf_s));

            if(pstForeGround == NULL)
                return MT_ERR_TDE_NULL_PTR;

            bpp = get_bpp(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en));
            if(((bpp != 8) && (bpp != 16) && (bpp != 32))
                    || (is_semiplanner(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en))))
            {
                MT_INFO_TDE("\r\n bpp or fmt not supported :%s, %d", __FUNCTION__, __LINE__);
                return MT_ERR_TDE_INVALID_PARA;
            }

            dw = (src_rect.w > MIN_W) ? src_rect.w : MIN_W;
            dh = (src_rect.h > MIN_H) ? src_rect.h : MIN_H;

            memcpy(&mid_surface, pstForeGround, sizeof(TDE2_SURFACE_S));

            size = dw * dh * bpp / 8;
            if((g_psMBuf.bufsize > size) && g_psMBuf.user_viraddr)
            {
                mid_surface.u32PhyAddr = g_psMBuf.phyaddr;
                mid_surface.u32VirAddr = (ulong)g_psMBuf.user_viraddr;
                g_addr_offset = size;
            }
            else
            {
                strncpy(psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
                psMBuf.bufsize = dw * dh * bpp / 8;
                mt_mmz_malloc(&psMBuf);
                mid_surface.u32PhyAddr = psMBuf.phyaddr;
                mid_surface.u32VirAddr = (ulong)psMBuf.user_viraddr;
                if(mid_surface.u32PhyAddr == 0)
                {
                    MT_INFO_TDE("\r\n malloc failed :%s, %d", __FUNCTION__, __LINE__);
                    return MT_ERR_TDE_NO_MEM;
                }
            }
            mid_surface.u32Stride = dw * bpp / 8;
            mid_surface.u32Width = dw;
            mid_surface.u32Height = dh;

            ret = soft_scale_to_mid_region(pstForeGround, &src_rect, dw, dh, &mid_surface);
            if(ret != MT_SUCCESS)
            {
                if(psMBuf.user_viraddr)
                    mt_mmz_free(&psMBuf);
                g_addr_offset = 0;
                return ret;
            }

            mid_rect.s32Xpos = 0;
            mid_rect.s32Ypos = 0;
            mid_rect.u32Width = mid_surface.u32Width;
            mid_rect.u32Height = mid_surface.u32Height;
            ret = GpeOsiBlit_3src(s32Handle, pstBackGround, pstBackGroundRect,
                    &mid_surface, &mid_rect,
                    pstExGround, pstExGroundRect,
                    pstDst, pstDstRect, pstOpt);
            if(psMBuf.user_viraddr)
                mt_mmz_free(&psMBuf);
            g_addr_offset = 0;
            return ret;
        }
        //------------fix bug 99703 end
    }
    if(scale_flag == MT_TRUE)
    {
        pos_t dst00;
        pos_t dst10;
        pos_t dst01;
        pos_t dst11;
        need_tmpbuf = MT_TRUE;
        if(pstOpt->b3dResize == MT_FALSE)
        {
            dst00.x = dst_rect.x;
            dst00.y = dst_rect.y;
            dst10.x = dst_rect.x + dst_rect.w;
            dst10.y = dst_rect.y;
            dst01.x = dst_rect.x;
            dst01.y = dst_rect.y + dst_rect.h;
            dst11.x = dst_rect.x + dst_rect.w;
            dst11.y = dst_rect.y + dst_rect.h;
        }
        else
        {
            dst00.x = pstOpt->Scale3dDst.pos00.x;
            dst00.y = pstOpt->Scale3dDst.pos00.y;
            dst10.x = pstOpt->Scale3dDst.pos10.x;
            dst10.y = pstOpt->Scale3dDst.pos10.y;
            dst01.x = pstOpt->Scale3dDst.pos01.x;
            dst01.y = pstOpt->Scale3dDst.pos01.y;
            dst11.x = pstOpt->Scale3dDst.pos11.x;
            dst11.y = pstOpt->Scale3dDst.pos11.y;
        }

        aria_gpe_get_scale_coeff(&src_rect, &dst00, &dst10, &dst01, &dst11, scale_coef, &scale_type);
        if(scale_type == SCALE_TYPE_MAX)
        {
            MT_ERR_TDE("get scale coeff error!\n");
            return MT_ERR_TDE_INVALID_PARA;
        }
        MT_INFO_TDE("\r\n scale_type:%d", scale_type);
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
        if(scale_coef[5] == 0)
#else
         if(scale_coef[5] == 0
                && (src_rect.w != dst_rect.w)
                && (src_rect.h != dst_rect.h))
#endif
        {
            need_tmpbuf = MT_FALSE;
        }
        else
#endif
        {
            if((scale_type == SCALE_ONLY_HORI_RECT) || (scale_type == SCALE_ONLY_HORI_TRAPZ))
                need_tmpbuf = MT_FALSE;
            else if(scale_type == SCALE_ONLY_VERT_RECT)
            {
                if((pstOpt->enAluCmd == TDE2_ALUCMD_NONE)
                        || ((pstOpt->enAluCmd & TDE2_ALUCMD_ROP)
                            && (pstOpt->enRopCode_Alpha == TDE2_ROP_COPYPEN) && (pstOpt->enRopCode_Color == TDE2_ROP_COPYPEN)))
                    need_tmpbuf = MT_FALSE;
            }
        }
    }

    MT_INFO_TDE("\r\n need_tmpbuf:%d", need_tmpbuf);
    if(need_tmpbuf == MT_TRUE && pstForeGround != NULL)
    {
        if(is_lut(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en)))
        {
            tmpbuf_fmt = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en);
            if(tmpbuf_fmt == PIX_FMT_RGBPALETTE1 || tmpbuf_fmt == PIX_FMT_RGBPALETTE2
                    || tmpbuf_fmt == PIX_FMT_RGBPALETTE4)
                tmpbuf_fmt = PIX_FMT_RGBPALETTE8;
        }
        else if(aria_get_color_space(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en)) == RGB_COLOR_SPACE)
            tmpbuf_fmt = PIX_FMT_ARGB8888;
        else
            tmpbuf_fmt = PIX_FMT_AYCBCR8888;
        bpp = get_bpp(tmpbuf_fmt);
        if(scale_type == SCALE_RECT)
        {
            tmp_src_rect.y = src_rect.y;
            tmp_src_rect.w = src_rect.w;
            tmp_src_rect.h = src_rect.h;
            if(src_rect.x >= 4)
            {
                tmp_src_rect.x = src_rect.x - 4;
                tmp_src_rect.w += 4;
            }
            else
            {
                tmp_src_rect.x = 0;
                tmp_src_rect.w += src_rect.x;
            }

            if(src_rect.x + src_rect.w + 4 < pstForeGround->u32Width)
                tmp_src_rect.w += 4;
            else
                tmp_src_rect.w += pstForeGround->u32Width - src_rect.x - src_rect.w;
        }
        else
        {
            tmp_src_rect.x = src_rect.x;
            tmp_src_rect.y = src_rect.y;
            tmp_src_rect.w = src_rect.w;
            tmp_src_rect.h = src_rect.h;
        }

        tmp_dst_rect.x = 0;
        tmp_dst_rect.y = 0;
        tmp_dst_rect.w = tmp_src_rect.w;
        tmp_dst_rect.h = dst_rect.h;

        tmpbuf_pitch = (tmp_dst_rect.w * bpp + 7) / 8 ;
        size = tmpbuf_pitch * tmp_dst_rect.h;
        if((g_psMBuf.bufsize > size + g_addr_offset) && g_psMBuf.user_viraddr)
        {
            p_tmpbuf = g_psMBuf.phyaddr + g_addr_offset;
            g_addr_offset += size;
        }
        else
        {
            p_tmpbuf = mt_mmz_new(tmpbuf_pitch * tmp_dst_rect.h, 8, NULL, MOD_NAME);
            MT_INFO_TDE("\r\n p_tmpbuf:0x%08x", p_tmpbuf);
            if(p_tmpbuf == 0)
            {
                g_addr_offset = 0;
                MT_ERR_TDE("\r\n malloc p_tmpbuf failed!");
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            }
            tmp_buf_used = 1;
        }
        if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_FOREGROUND)
        {
            src_ck_en = MT_TRUE;
        }
        else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
        {
            if(pstOpt->stColorKeyCfg.enColorKey_fg)
                src_ck_en = MT_TRUE;
        }

        if((src_ck_en) || (scale_coef[5] == 1))
        {
            maskbuf_pitch = tmp_dst_rect.w;
            size = maskbuf_pitch * tmp_dst_rect.h;
            if((g_psMBuf.bufsize > size + g_addr_offset) && g_psMBuf.user_viraddr)
            {
                p_mask_buf = g_psMBuf.phyaddr + g_addr_offset;
                g_addr_offset += size;
            }
            else
            {
                p_mask_buf = mt_mmz_new(maskbuf_pitch * tmp_dst_rect.h, 64, NULL, MOD_NAME);
                if(p_mask_buf == 0)
                {
                    g_addr_offset = 0;
                    MT_ERR_TDE("\r\n malloc p_mask_buf failed!");
                    if(tmp_buf_used)
                    {
                        mt_mmz_delete(p_tmpbuf);                        
                    }
                    return MT_ERR_TDE_UNSUPPORTED_OPERATION;
                }
                msk_buf_used = 1;
            }
        }

        ret = gpe_2d_vert_scale(s32Handle, &tmp_src_rect, &tmp_dst_rect, pstForeGround, pstOpt, p_tmpbuf,
                tmpbuf_pitch, tmpbuf_fmt, p_mask_buf, maskbuf_pitch, src_ck_en, scale_coef);
        if(ret != MT_TRUE)
        {
            g_addr_offset = 0;
            if(tmp_buf_used)
            {
                mt_mmz_delete(p_tmpbuf);
            }
            if(msk_buf_used)
            {
                mt_mmz_delete(p_mask_buf);
            }
            return MT_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    }

    param_init(&param);

    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    if((scale_flag) && (need_tmpbuf))
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = p_tmpbuf;
        param.src_img.pitch = tmpbuf_pitch;
        param.src_img.width = tmp_dst_rect.w;
        param.src_img.height = tmp_dst_rect.h;
        param.src_img.pix_format = tmpbuf_fmt;
        param.src_img.rect.x = src_rect.x - tmp_src_rect.x;
        param.src_img.rect.y = 0;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = dst_rect.h;
        if(p_mask_buf != 0)
        {
            param.src_with_mask = MT_TRUE;
            param.src_mask_buf = p_mask_buf;
            param.src_mask_pitch = maskbuf_pitch;
        }
    }
    else if(paint_src == MT_FALSE && pstForeGround != NULL)
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = pstForeGround->u32PhyAddr;
        param.src_img.pitch = pstForeGround->u32Stride;
        param.src_img.width = pstForeGround->u32Width;
        param.src_img.height = pstForeGround->u32Height;
        param.src_img.pix_format = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &param.src_img.alpha_ch_en);
        param.src_img.rect.x = src_rect.x;
        param.src_img.rect.y = src_rect.y;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = src_rect.h;

        if((param.src_img.pix_format == PIX_FMT_TILE)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
        {
            param.src_img.chroma_addr = pstForeGround->u32CbCrPhyAddr;
            param.src_img.chroma_pitch = pstForeGround->u32CbCrStride;
#ifdef CONFIG_MT_FPGA_GPE			
            g_debug.chroma_vir_addr = pstForeGround->u32CbCrVirAddr;
#endif			
        }
        if((param.src_img.pix_format >= PIX_FMT_XY) && (param.src_img.pix_format <= PIX_FMT_XYLC))
        {
            param.src_img.xylc_num = pstOpt->xylc_cmd_num;
            param.src_img.xylc_color = pstOpt->xylc_color;
        }
    }
    // ====== dst image attribute =====  //
    param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    param.dst_img.rect.x = dst_rect.x;
    param.dst_img.rect.y = dst_rect.y;
    param.dst_img.rect.w = dst_rect.w;
    param.dst_img.rect.h = dst_rect.h;

    // ======= ex image attribute ======= //
    param.ex_img_en = MT_TRUE;
    param.ex_img.buf = pstExGround->u32PhyAddr;
    param.ex_img.pitch = pstExGround->u32Stride;
    param.ex_img.width = pstExGround->u32Width;
    param.ex_img.height = pstExGround->u32Height;
    param.ex_img.pix_format = aria_gpe_convert_fmt(pstExGround->enColorFmt, &param.ex_img.alpha_ch_en);
    param.ex_img.rect.x = ex_rect.x;
    param.ex_img.rect.y = ex_rect.y;
    param.ex_img.rect.w = ex_rect.w;
    param.ex_img.rect.h = ex_rect.h;

    if((pstBackGround != MT_NULL)&&(pstBackGroundRect != MT_NULL))
    {
        param.bg_img_en = MT_TRUE;
        param.bg_img.buf = pstBackGround->u32PhyAddr;
        param.bg_img.pitch = pstBackGround->u32Stride;
        param.bg_img.width = pstBackGround->u32Width;
        param.bg_img.height = pstBackGround->u32Height;
        param.bg_img.pix_format = aria_gpe_convert_fmt(pstBackGround->enColorFmt, &param.bg_img.alpha_ch_en);
        param.bg_img.rect.x = bg_rect.x;
        param.bg_img.rect.y = bg_rect.y;
        param.bg_img.rect.w = bg_rect.w;
        param.bg_img.rect.h = bg_rect.h;
    }

    if(pstForeGround != NULL)
    {
        if((pstForeGround->pu8ClutPhyAddr != 0))
            param.src_img.palette_base = (phys_addr_t)pstForeGround->pu8ClutPhyAddr;
    }

    if((pstExGround->pu8ClutPhyAddr != 0))
        param.ex_img.palette_base = (phys_addr_t)pstExGround->pu8ClutPhyAddr;

    if((scale_flag == MT_FALSE) || (need_tmpbuf == MT_FALSE))
    {
        if(pstOpt->stSurfaceCfg.enSrcFlip)
            param.src_img.negative_stride = MT_TRUE;

        if(pstForeGround != NULL)
        {
            if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_FOREGROUND)
            {
                enFmtCategory = GpeOsiGetFmtCategory(pstForeGround->enColorFmt);
                param.src_img.ck_en = MT_TRUE;
                param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
                param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
                param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);
                //		  param.src_img.key_color_select = MASK_KEY_MATCH;
                param.src_img.key_color_select = pstOpt->enColorKeySelect;
            }
            else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
            {
                param.src_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_fg;
                if(param.src_img.ck_en)
                {
                    enFmtCategory = GpeOsiGetFmtCategory(pstForeGround->enColorFmt);
                    param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                    param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                    param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
                    param.src_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_fg;
                }
            }
        }
    }
    if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_BACKGROUND)
    {
        enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
        param.dst_img.ck_en = MT_TRUE;
        param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);

        //		  param.dst_img.key_color_select = MASK_KEY_MISMATCH;
        param.dst_img.key_color_select = pstOpt->enColorKeySelect;
    }
    else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
    {
        param.dst_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_bg;
        if(param.dst_img.ck_en)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
            param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_bg;
        }
    }

    if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_EX)
    {
        enFmtCategory = GpeOsiGetFmtCategory(pstExGround->enColorFmt);
        param.ex_img.ck_en = MT_TRUE;
        param.ex_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
        param.ex_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
        param.ex_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
        param.ex_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);

        //		  param.dst_img.key_color_select = MASK_KEY_MISMATCH;
        param.ex_img.key_color_select = pstOpt->enColorKeySelect;
    }
    else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
    {
        param.ex_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_bg;
        if(param.ex_img.ck_en)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstExGround->enColorFmt);
            param.ex_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_ex, enFmtCategory);
            param.ex_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_ex, enFmtCategory);
            param.ex_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_ex, enFmtCategory);
            param.ex_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_ex;
        }
    }

    if(pstOpt->stSurfaceCfg.enSrcGlobalAlpha)
    {
        param.src_img.plane_alpha_en = MT_TRUE;
        param.src_img.plane_alpha = pstOpt->u8GlobalAlpha;
    }
    if((pstOpt->stSurfaceCfg.enExGlobalAlpha))
    {
        param.ex_img.plane_alpha_en = MT_TRUE;
        param.ex_img.plane_alpha = pstOpt->u8ExGlobalAlpha;
    }
    if(pstOpt->stSurfaceCfg.enDstGlobalAlpha)
    {
        if(param.bg_img_en)
        {
            param.bg_img.plane_alpha_en = TRUE;
            param.bg_img.plane_alpha = pstOpt->u8DstGlobalAlpha;
        }
        else
        {
            param.dst_img.plane_alpha_en = TRUE;
            param.dst_img.plane_alpha = pstOpt->u8DstGlobalAlpha;      
        }
    }
    if(pstOpt->stSurfaceCfg.enSrcPreMult)
    {
        param.src_img.alpha_pre_multed = MT_FALSE;
    }
    if((pstOpt->stSurfaceCfg.enExPreMult))
    {
        param.ex_img.alpha_pre_multed = MT_FALSE;
    }

    if(pstOpt->stSurfaceCfg.enDstPreMult)
    {
        if(param.bg_img_en)
        {
            param.bg_img.alpha_pre_multed = MT_FALSE;
        }
        else
        {
            param.dst_img.alpha_pre_multed = MT_FALSE;    
        }
    }

    if(pstOpt->stSurfaceCfg.disSrcAlpha)
    {
        param.src_img.alpha_ch_en = MT_FALSE;
        MT_INFO_TDE("\r\n ~~~~~src alpha ch en :%d", param.src_img.alpha_ch_en);
    }
    if((pstOpt->stSurfaceCfg.disExAlpha))
    {
        param.ex_img.alpha_ch_en = MT_FALSE;
        MT_INFO_TDE("\r\n ~~~`ex alpha ch en :%d", param.ex_img.alpha_ch_en);
    }
    if(pstOpt->stSurfaceCfg.disDstAlpha)
    {
        if(param.bg_img_en)
            param.bg_img.alpha_ch_en = MT_FALSE;
        else
            param.dst_img.alpha_ch_en = MT_FALSE;
    }

    if(pstOpt->stSurfaceCfg.enDstFlip)
        param.dst_img.negative_stride = MT_TRUE;
    if(pstOpt->stSurfaceCfg.enExFlip)
        param.ex_img.negative_stride = MT_TRUE;

    param.gpe_op = GPE_OP_NONE;

    if(scale_flag == MT_TRUE)
    {
#ifdef FAST_2D_SCALE
#ifdef CONFIG_MT_FPGA_GPE
        if(scale_coef[5] == 0)
#else
        if(scale_coef[5] == 0
                && (src_rect.w != dst_rect.w)
                && (src_rect.h != dst_rect.h))
#endif
        {
            param.gpe_op |= GPE_OP_SCALE;
            param.coef[0] = scale_coef[0];
            param.coef[1] = scale_coef[1];
            param.coef[2] = scale_coef[2];
            param.coef[3] = scale_coef[3];
            param.coef[4] = scale_coef[4];
            param.coef[5] = scale_coef[5];
        }
        else
#endif
        {
            if((pstOpt->b3dResize == MT_TRUE) || (param.src_img.rect.w != param.dst_img.rect.w)
                    || ((scale_type == SCALE_ONLY_VERT_RECT) && (need_tmpbuf == MT_FALSE)))
            {
                if((scale_type == SCALE_ONLY_VERT_RECT) && (need_tmpbuf == MT_FALSE))
                {
                    param.scale_mod = SCALE_VERT_BLK_OUT;
                    param.gpe_op |= GPE_OP_ROTATE | GPE_OP_SCALE_VERT;
                    param.rotator_op = GPE_ARIA_NO_OP;
                }
                else
                {
                    param.scale_mod = SCALE_HORI_LINE_OUT;
                    if(pstOpt->b3dResize == MT_TRUE)
                        param.gpe_op |= GPE_OP_SCALE_TRAPZ;
                    else
                        param.gpe_op |= GPE_OP_SCALE_HORI;
                }
                param.gpe_op |= GPE_OP_SCALE;
                param.coef[0] = scale_coef[0];
                param.coef[1] = scale_coef[1];
                param.coef[2] = scale_coef[2];
                param.coef[3] = scale_coef[3];
                param.coef[4] = scale_coef[4];
                param.coef[5] = scale_coef[5];
            }
        }
    }
    else
    {
        if(pstOpt->enGsBlur)
        {
            param.gpe_op |= GPE_OP_BLUR;
            param.blur_tap = (mt_u32)(pstOpt->stBlurOpt.blur_level * 2 + 3);
        }
    }

    param.rotator_op = rotate_mirror_tab[pstOpt->enMirror][pstOpt->enRotator];
    if(param.rotator_op != GPE_ARIA_NO_OP)
        param.gpe_op |= GPE_OP_ROTATE;

    if(pstOpt->enAluCmd == TDE2_ALUCMD_NONE)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id =  ROP_COPYPEN;
        param.rop.rop_c_id =  ROP_COPYPEN;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_ROP)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id = aria_gpe_convert_rop(pstOpt->enRopCode_Alpha);
        param.rop.rop_c_id = aria_gpe_convert_rop(pstOpt->enRopCode_Color);
        param.rop.rop_pattern = pstOpt->u32Colorize;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_BLEND)
    {
        param.gpe_op |= GPE_OP_BLEND;

        switch (pstOpt->stBlendOpt.eBlendCmd) {
            /**< fs: sa      fd: 1.0-sa */
        case TDE2_BLENDCMD_NONE:
            {
                param.blend.src_blend_fact = GL_SRC_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0     fd: 0.0 */
        case TDE2_BLENDCMD_CLEAR:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 0.0 */
        case TDE2_BLENDCMD_SRC:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCOVER:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0 */
        case TDE2_BLENDCMD_DSTOVER:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: da      fd: 0.0 */
        case TDE2_BLENDCMD_SRCIN:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: sa */
        case TDE2_BLENDCMD_DSTIN:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 0.0 */
        case TDE2_BLENDCMD_SRCOUT:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_DSTOUT:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: da      fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCATOP:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: sa */
        case TDE2_BLENDCMD_DSTATOP:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0     fd: 1.0 */
        case TDE2_BLENDCMD_ADD:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0-sa */
        case TDE2_BLENDCMD_XOR:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0  fd: 1.0*/
        case TDE2_BLENDCMD_DST:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /* user parameter*/ /*CNcomment:  ???????? */
        case TDE2_BLENDCMD_CONFIG:
        default:
            {
                param.blend.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendMode); // src2 is aria src1
                param.blend.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendMode); // src1 is aria src2
                break;
            }

        }
        if(pstOpt->stBlendOpt.bBlendModeAlphaEnable && (pstOpt->stBlendOpt.eBlendCmd == TDE2_BLENDCMD_CONFIG))
        {
            param.blend_alpha.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendModeAlpha);
            param.blend_alpha.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendModeAlpha);
        }
        else
        {
            param.blend_alpha.src_blend_fact = param.blend.src_blend_fact;
            param.blend_alpha.dst_blend_fact = param.blend.dst_blend_fact;
        }
        if((pstForeGround != NULL) && (pstOpt->stBlendOpt.bGlobalAlphaEnable))
        {
            param.src_img.plane_alpha_en = MT_TRUE;
            param.src_img.plane_alpha = pstOpt->u8GlobalAlpha;
        }

        if((pstForeGround != NULL) && pstOpt->stBlendOpt.bSrc2AlphaPremulti)
        {
            param.src_img.alpha_pre_multed = MT_FALSE;
        }
    }

    if(pstOpt->enMultiply)
    {
        param.gpe_op |= GPE_OP_DMULT;
    }

    if(pstOpt->enStencil)
    {
        param.gpe_op |= GPE_OP_DSTEN;
    }

    if(pstOpt->enPaint)
    {
        param.gpe_op |= GPE_OP_PAINT;
        memcpy(&param.paint, &pstOpt->stPaintOpt, sizeof(TDE2_PAINT_CFG_S));
    }

    if(pstOpt->enAMapLogical)
    {
        param.gpe_op |= GPE_OP_ALPHAMAP;
        param.alpha_map_mod = GPE_CCT_ALPHA_MAP_LOGICAL;
    }

    if(pstOpt->enAMapMix)
    {
        param.gpe_op |= GPE_OP_ALPHAMAP;
        param.alpha_map_mod = GPE_CCT_ALPHA_MAP_MIX_NORMAL;
    }
    if(pstOpt->enAMapMixEx)
    {
        param.gpe_op |= GPE_OP_ALPHAMAP;
        param.alpha_map_mod = GPE_CCT_ALPHA_MAP_MIX_EX;
    }

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(tmp_buf_used)
        {
            mt_mmz_delete(p_tmpbuf);
        }
        if(msk_buf_used)
        {
            mt_mmz_delete(p_mask_buf);
        }
        g_addr_offset = 0;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    ret = aria_gpe_start(s32Handle);

    if(tmp_buf_used)
    {
        mt_mmz_delete(p_tmpbuf);
    }
    if(msk_buf_used)
    {
        mt_mmz_delete(p_mask_buf);
    }
    g_addr_offset = 0;
    MT_INFO_TDE("----------------out------------\n");
    return (ret == MT_TRUE ?MT_SUCCESS: MT_FAILURE);


}

mt_s32     GpeOsiQuickFill(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S *pstDstRect,
        mt_u32 u32FillData)
{
    mt_s32 ret = 0;
    aria_param_t param = {0};
    rect_vsb_t rect;

    MT_INFO_TDE("----------------in------------\n");

    if(pstDst == NULL || pstDstRect == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
    {
        rect.x = 0;
        rect.y = 0;
        rect.w = pstDst->u32Width;
        rect.h = pstDst->u32Height;
    }
    else
    {
        rect.x = (mt_u32)(pstDstRect->s32Xpos);
        rect.y = (mt_u32)(pstDstRect->s32Ypos);
        rect.w = pstDstRect->u32Width;
        rect.h = pstDstRect->u32Height;
    }

    param_init(&param);
    param.no_dithering = MT_TRUE;
    param.dst_img.palette_base = (phys_addr_t)(pstDst->pu8ClutPhyAddr);
    param.dst_img.rect.x = rect.x;
    param.dst_img.rect.y = rect.y;
    param.dst_img.rect.w = rect.w;
    param.dst_img.rect.h = rect.h;
    param.gpe_op = GPE_OP_ROP;
    param.rop.rop_a_id = ROP_PATCOPY;
    param.rop.rop_c_id = ROP_PATCOPY;
    param.rop.rop_pattern = u32FillData;
    param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    if(is_lut(param.dst_img.pix_format))
        param.rop.rop_pattern = aria_lut_patterncolor(param.dst_img.pix_format, u32FillData);
    else
        param.rop.rop_pattern = u32FillData;

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        return MT_FAILURE;
    }

    ret = aria_gpe_start(s32Handle);

    return (ret == MT_TRUE ?MT_SUCCESS: MT_FAILURE);

}

mt_s32 GpeOsiQuickResize(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
        TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)
{
    mt_s32 ret = 0;
    TDE2_OPT_S stOpt = { 0 };

    MT_INFO_TDE("----------------in------------\n");
    if(pstDst == NULL || pstDstRect == NULL)
        return MT_ERR_TDE_NULL_PTR;
    if(pstSrc == NULL || pstSrcRect == NULL)
        return MT_ERR_TDE_NULL_PTR;

    stOpt.bResize = MT_TRUE;

    ret = GpeOsiBlit(s32Handle, NULL, NULL, pstSrc, pstSrcRect,
            pstDst, pstDstRect, &stOpt);

    return ret;

}

mt_s32 GpeOsiQuickCopy(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
        TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect)
{
    mt_s32 ret = 0;
    TDE2_OPT_S stOpt = { 0 };

    MT_INFO_TDE("----------------in------------\n");
    if(pstDst == NULL || pstDstRect == NULL)
        return MT_ERR_TDE_NULL_PTR;
    if(pstSrc == NULL || pstSrcRect == NULL)
        return MT_ERR_TDE_NULL_PTR;

    ret = GpeOsiBlit(s32Handle, NULL, NULL, pstSrc, pstSrcRect,
            pstDst, pstDstRect, &stOpt);

    return ret; 

}

mt_s32 GpeOsiDraw_3d_trapez(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstSrc, TDE2_RECT_S *pstSrcRect,
        TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect, TDE2_TRAPEZ_OPT_S *pstOpt)
{
    mt_s32 ret = MT_ERR_TDE_UNSUPPORTED_OPERATION;
    aria_param_t param = {0};
    rect_vsb_t src_rect = {0};
    rect_vsb_t dst_rect = {0};
    MT_BOOL scale_flag = 0;
    mt_s32 scale_coef[6] = {0};

    phys_addr_t p_tmpbuf = 0;
    pix_fmt_t tmpbuf_fmt = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch = 0;
    rect_vsb_t tmp_dst_rect = {0};
    phys_addr_t p_mask_buf = 0;
    mt_u32 maskbuf_pitch = 0;

    phys_addr_t p_tmpbuf2 = 0;
    pix_fmt_t tmpbuf_fmt2 = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch2 = 0;
    rect_vsb_t tmp_dst_rect2 = {0};
    phys_addr_t p_mask_buf2 = 0;
    mt_u32 maskbuf_pitch2 = 0;

    phys_addr_t p_tmpbuf3 = 0;
    pix_fmt_t tmpbuf_fmt3 = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch3 = 0;
    rect_vsb_t tmp_dst_rect3 = {0};
    phys_addr_t p_mask_buf3 = 0;
    mt_u32 maskbuf_pitch3 = 0;

    mt_u32 bpp = 0;
    scale_type_t scale_type;
    pos_t dst00;
    pos_t dst10;
    pos_t dst01;
    pos_t dst11;

    GPE_COLORFMT_CATEGORY_E enFmtCategory;
    MT_BOOL alpha_en;

    if(pstDst == NULL || pstDstRect == NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstSrc == NULL || pstSrcRect == NULL)
        return MT_ERR_TDE_NULL_PTR;


    if((pstSrcRect->u32Width == 0) && (pstSrcRect->u32Height == 0))
    {
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.w = pstSrc->u32Width;
        src_rect.h = pstSrc->u32Height;
    }
    else
    {
        src_rect.x = (mt_u32)(pstSrcRect->s32Xpos);
        src_rect.y = (mt_u32)(pstSrcRect->s32Ypos);
        src_rect.w = pstSrcRect->u32Width;
        src_rect.h = pstSrcRect->u32Height;
    }

    if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
    {
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.w = pstDst->u32Width;
        dst_rect.h = pstDst->u32Height;
    }
    else
    {
        dst_rect.x = (mt_u32)(pstDstRect->s32Xpos);
        dst_rect.y = (mt_u32)(pstDstRect->s32Ypos);
        dst_rect.w = pstDstRect->u32Width;
        dst_rect.h = pstDstRect->u32Height;
    }

    if(pstOpt->u32Direction == 0)
    {
        dst00.x = 0;
        dst00.y = 0;
        dst10.x = dst_rect.w;
        dst10.y = pstOpt->stTrapez.top_start_x;
        dst01.x = 0;
        dst01.y = dst_rect.h;
        dst11.x = dst_rect.w;
        dst11.y = pstOpt->stTrapez.top_start_x + pstOpt->stTrapez.top_len;
    }
    else
    {
        dst00.x = 0;
        dst00.y = dst_rect.h - pstOpt->stTrapez.top_start_x - pstOpt->stTrapez.top_len;
        dst10.x = dst_rect.w;
        dst10.y = 0;
        dst01.x = 0;
        dst01.y = dst_rect.h - pstOpt->stTrapez.top_start_x;
        dst11.x = dst_rect.w;
        dst11.y = dst_rect.h;
    }

    MT_INFO_TDE("\r\n [%d,%d],[%d,%d],[%d,%d],[%d,%d]", dst00.x,dst00.y,
            dst10.x, dst10.y, dst01.x,dst01.y, dst11.x, dst11.y);

    aria_gpe_get_scale_coeff(&src_rect, &dst00, &dst10, &dst01, &dst11, scale_coef, &scale_type);

    //step1-------------------------------------------horizontal scale and rotate
    //src: src_rect.w, src_rect.h
    //dst: src_rect.h, dst_rect.w

    if((src_rect.w != dst_rect.w))
        scale_flag = MT_TRUE;
    else
        scale_flag = MT_FALSE;

    if(is_lut(aria_gpe_convert_fmt(pstDst->enColorFmt, &alpha_en)))
    {
        if(is_lut(aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en)) == MT_FALSE)
            return MT_ERR_TDE_UNSUPPORTED_OPERATION;
        tmpbuf_fmt = aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en);
    }
    else if(aria_get_color_space(aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en)) == RGB_COLOR_SPACE)
        tmpbuf_fmt = PIX_FMT_ARGB8888;
    else
        tmpbuf_fmt = PIX_FMT_AYCBCR8888;
    bpp = get_bpp(tmpbuf_fmt);

    tmp_dst_rect.x = 0;
    tmp_dst_rect.y = 0;
    tmp_dst_rect.w = src_rect.h;
    tmp_dst_rect.h = dst_rect.w;
    tmpbuf_pitch = (tmp_dst_rect.w * bpp + 7) / 8;

    p_tmpbuf =  mt_mmz_new(tmpbuf_pitch * tmp_dst_rect.h, 8, NULL, MOD_NAME);
    if(p_tmpbuf == 0)
    {
        MT_ERR_TDE("\r\n malloc p_tmpbuf failed!");
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    maskbuf_pitch = tmp_dst_rect.w;
    p_mask_buf = mt_mmz_new(maskbuf_pitch * tmp_dst_rect.h, 8, NULL, MOD_NAME);
    if(p_mask_buf == 0)
    {
        MT_ERR_TDE("\r\n malloc p_mask_buf failed!");
        if(p_tmpbuf)
        {
            mt_mmz_delete(p_tmpbuf);
        }
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    param_init(&param);
    param.src_img_en = MT_TRUE;
    param.src_img.buf = pstSrc->u32PhyAddr;
    param.src_img.pitch = pstSrc->u32Stride;
    param.src_img.width = pstSrc->u32Width;
    param.src_img.height = pstSrc->u32Height;
    param.src_img.pix_format = aria_gpe_convert_fmt(pstSrc->enColorFmt, &param.src_img.alpha_ch_en);
    param.src_img.rect.x = src_rect.x;
    param.src_img.rect.y = src_rect.y;
    param.src_img.rect.w = src_rect.w;
    param.src_img.rect.h = src_rect.h;

    if((param.src_img.pix_format == PIX_FMT_TILE)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
            || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
            || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
    {
        param.src_img.chroma_addr = pstSrc->u32CbCrPhyAddr;
        param.src_img.chroma_pitch = pstSrc->u32CbCrStride;
#ifdef CONFIG_MT_FPGA_GPE		
         g_debug.chroma_vir_addr = pstSrc->u32CbCrVirAddr;
#endif		 
    }

    param.dst_img.buf = p_tmpbuf;
    param.dst_img.pitch = tmpbuf_pitch;
    param.dst_img.width = tmp_dst_rect.w;
    param.dst_img.height = tmp_dst_rect.h;
    param.dst_img.pix_format = tmpbuf_fmt;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect.w;
    param.dst_img.rect.h = tmp_dst_rect.h;


    if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_FOREGROUND)
    {
        enFmtCategory = GpeOsiGetFmtCategory(pstSrc->enColorFmt);
        param.src_img.ck_en = MT_TRUE;
        param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
        param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
        param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);
        param.src_img.key_color_select = pstOpt->enColorKeySelect;
    }
    else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
    {
        param.src_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_fg;
        if(param.src_img.ck_en)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstSrc->enColorFmt);
            param.src_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
            param.src_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
            param.src_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_fg, enFmtCategory);
            param.src_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_fg;
        }
    }

    param.dst_with_mask = MT_TRUE;
    param.dst_mask.mbuf_addr = p_mask_buf;
    param.dst_mask.mbuf_pitch = maskbuf_pitch;


    if(scale_flag)
        param.gpe_op = GPE_OP_SCALE | GPE_OP_ROTATE | GPE_OP_SCALE_HORI;
    else
        param.gpe_op = GPE_OP_ROTATE;
    param.rotator_op = GPE_ARIA_VERT_MIRROR_TRANS;
    param.scale_mod = SCALE_HORI_BLK_OUT;
    param.coef[0] = scale_coef[0];
    param.coef[1] = scale_coef[1];
    param.coef[2] = scale_coef[2];
    param.coef[3] = scale_coef[3];
    param.coef[4] = scale_coef[4];
    param.coef[5] = scale_coef[5];

    param.need_suspend = MT_FALSE;
    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {        
        mt_mmz_delete(p_tmpbuf);
        mt_mmz_delete(p_mask_buf);
        aria_gpe_print_context(s32Handle);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    //    gpe_spn_invalidate_buf((void *)param.dst_img.buf, param.dst_img.pitch * param.dst_img.height);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf);
        mt_mmz_delete(p_mask_buf);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    GPE_PRINT("\r\n step1 horizontal scale and rotate [%d,%d,%d,%d],[%d,%d,%d,%d]",
            param.src_img.rect.x,param.src_img.rect.y,param.src_img.rect.w,param.src_img.rect.h,
            param.dst_img.rect.x,param.dst_img.rect.y,param.dst_img.rect.w,param.dst_img.rect.h);

    //step2-------------------------------------------rect to trapz scale
    tmpbuf_fmt2 = tmpbuf_fmt;
    bpp = get_bpp(tmpbuf_fmt2);
    tmp_dst_rect2.x = 0;
    tmp_dst_rect2.y = 0;
    tmp_dst_rect2.w = dst_rect.h;
    tmp_dst_rect2.h = dst_rect.w;
    tmpbuf_pitch2 = (tmp_dst_rect2.w * bpp + 7) / 8;
    p_tmpbuf2 = mt_mmz_new(tmpbuf_pitch2 * tmp_dst_rect2.h, 8, NULL, MOD_NAME);
    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    param.src_img_en = MT_TRUE;
    param.src_img.buf = p_tmpbuf;
    param.src_img.pitch = tmpbuf_pitch;
    param.src_img.width = tmp_dst_rect.w;
    param.src_img.height = tmp_dst_rect.h;
    param.src_img.pix_format = tmpbuf_fmt;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect.w;
    param.src_img.rect.h = tmp_dst_rect.h;
    param.src_with_mask = MT_TRUE;
    param.src_mask_buf = p_mask_buf;
    param.src_mask_pitch = maskbuf_pitch;

    maskbuf_pitch2 = tmp_dst_rect2.w;
    p_mask_buf2 = mt_mmz_new(maskbuf_pitch2 * tmp_dst_rect2.h, 8, NULL, MOD_NAME);
    if(p_mask_buf2 == ((phys_addr_t)0))
    {
        GPE_PRINT("\r\n malloc p_mask_buf failed!");
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf);
        mt_mmz_delete(p_tmpbuf);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    param.dst_with_mask = MT_TRUE;
    param.dst_mask.mbuf_addr = p_mask_buf2;
    param.dst_mask.mbuf_pitch = maskbuf_pitch2;

    // ====== dst image attribute =====  //
    param.dst_img.buf = p_tmpbuf2;
    param.dst_img.pitch = tmpbuf_pitch2;
    param.dst_img.width = tmp_dst_rect2.w;
    param.dst_img.height = tmp_dst_rect2.h;
    param.dst_img.pix_format = tmpbuf_fmt2;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect2.w;
    param.dst_img.rect.h = tmp_dst_rect2.h;

    param.gpe_op = GPE_OP_SCALE | GPE_OP_SCALE_TRAPZ;
    param.scale_mod = SCALE_HORI_LINE_OUT;
    param.coef[0] = scale_coef[0];
    param.coef[1] = scale_coef[1];
    param.coef[2] = scale_coef[2];
    param.coef[3] = scale_coef[3];
    param.coef[4] = scale_coef[4];
    param.coef[5] = scale_coef[5];
    param.gpe_op |= GPE_OP_ROP;
    param.rop.rop_a_id = ROP_COPYPEN;
    param.rop.rop_c_id = ROP_COPYPEN;

    //param.need_suspend = need_suspend();
    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf);
        mt_mmz_delete(p_mask_buf);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        aria_gpe_print_context(s32Handle);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf);
        mt_mmz_delete(p_mask_buf);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        aria_gpe_print_context(s32Handle);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    mt_mmz_delete(p_tmpbuf);
    mt_mmz_delete(p_mask_buf);
    p_tmpbuf = 0;
    p_mask_buf = 0;
    
    GPE_PRINT("\r\n step2 rect to trapz [%d,%d,%d,%d],[%d,%d,%d,%d]",
            param.src_img.rect.x,param.src_img.rect.y,param.src_img.rect.w,param.src_img.rect.h,
            param.dst_img.rect.x,param.dst_img.rect.y,param.dst_img.rect.w,param.dst_img.rect.h);

    //step3-----------------------------rotate
    tmpbuf_fmt3 = tmpbuf_fmt;
    bpp = get_bpp(tmpbuf_fmt3);
    tmp_dst_rect3.x = 0;
    tmp_dst_rect3.y = 0;
    tmp_dst_rect3.w = dst_rect.w;
    tmp_dst_rect3.h = dst_rect.h;
    tmpbuf_pitch3 = (tmp_dst_rect3.w * bpp + 7) / 8;
    p_tmpbuf3 = mt_mmz_new(tmpbuf_pitch3 * tmp_dst_rect3.h, 8, NULL, MOD_NAME);

    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info

    param.src_img_en = MT_TRUE;
    param.src_img.buf = p_tmpbuf2;
    param.src_img.pitch = tmpbuf_pitch2;
    param.src_img.width = tmp_dst_rect2.w;
    param.src_img.height = tmp_dst_rect2.h;
    param.src_img.pix_format = tmpbuf_fmt2;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect2.w;
    param.src_img.rect.h = tmp_dst_rect2.h;

    param.src_with_mask = MT_TRUE;
    param.src_mask_buf = p_mask_buf2;
    param.src_mask_pitch = maskbuf_pitch2;

    maskbuf_pitch3 = tmp_dst_rect3.w;
    p_mask_buf3 = mt_mmz_new(maskbuf_pitch3 * tmp_dst_rect3.h, 8, NULL, MOD_NAME);
    if(p_mask_buf3 == ((phys_addr_t)0))
    {
        GPE_PRINT("\r\n malloc p_mask_buf failed!");
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    param.dst_with_mask = MT_TRUE;
    param.dst_mask.mbuf_addr = p_mask_buf3;
    param.dst_mask.mbuf_pitch = maskbuf_pitch3;

    // ====== dst image attribute =====  //
    param.dst_img.buf = p_tmpbuf3;
    param.dst_img.pitch = tmpbuf_pitch3;
    param.dst_img.width = tmp_dst_rect3.w;
    param.dst_img.height = tmp_dst_rect3.h;
    param.dst_img.pix_format = tmpbuf_fmt3;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect3.w;
    param.dst_img.rect.h = tmp_dst_rect3.h;

    param.gpe_op = GPE_OP_ROTATE;
    param.rotator_op = GPE_ARIA_HORI_MIRROR_TRANS;

#ifdef CONFIG_MT_FPGA_GPE  
    if(g_debug.mirror)
    {
        param.rotator_op = GPE_ARIA_TRANS;       
    }
#endif

    //param.need_suspend = need_suspend();
    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        aria_gpe_print_context(s32Handle);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    mt_mmz_delete(p_tmpbuf2);
    mt_mmz_delete(p_mask_buf2);
    GPE_PRINT("\r\n step3 rotate [%d,%d,%d,%d],[%d,%d,%d,%d]",
            param.src_img.rect.x,param.src_img.rect.y,param.src_img.rect.w,param.src_img.rect.h,
            param.dst_img.rect.x,param.dst_img.rect.y,param.dst_img.rect.w,param.dst_img.rect.h);

    //step4-----------------------------------------------------blend/rop
    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    param.src_img_en = MT_TRUE;
    param.src_img.buf = p_tmpbuf3;
    param.src_img.pitch = tmpbuf_pitch3;
    param.src_img.width = tmp_dst_rect3.w;
    param.src_img.height = tmp_dst_rect3.h;
    param.src_img.pix_format = tmpbuf_fmt3;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect3.w;
    param.src_img.rect.h = tmp_dst_rect3.h;
    param.src_with_mask = MT_TRUE;
    param.src_mask_buf = p_mask_buf3;
    param.src_mask_pitch = maskbuf_pitch3;

    // ====== dst image attribute =====  //
    param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    param.dst_img.rect.x = dst_rect.x;
    param.dst_img.rect.y = dst_rect.y;
    param.dst_img.rect.w = dst_rect.w;
    param.dst_img.rect.h = dst_rect.h;
    GPE_PRINT("\r\n step4 src[%d,%d,%d,%d], dst[%d,%d,%d,%d]",
            param.src_img.rect.x,param.src_img.rect.y,param.src_img.rect.w,param.src_img.rect.h,
            param.dst_img.rect.x,param.dst_img.rect.y,param.dst_img.rect.w,param.dst_img.rect.h);

    if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_BACKGROUND)
    {
        enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
        param.dst_img.ck_en = MT_TRUE;
        param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->unColorKeyValue, enFmtCategory);

        //		  param.dst_img.key_color_select = MASK_KEY_MISMATCH;
        param.dst_img.key_color_select = pstOpt->enColorKeySelect;
    }
    else if(pstOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
    {
        param.dst_img.ck_en = pstOpt->stColorKeyCfg.enColorKey_bg;
        if(param.dst_img.ck_en)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
            param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_min = GpeOsiGetCkeyMin(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_max = GpeOsiGetCkeyMax(&pstOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.key_color_select = pstOpt->stColorKeyCfg.enColorKeySelect_bg;
        }
    }

    param.gpe_op = GPE_OP_NONE;

    if(pstOpt->enAluCmd == TDE2_ALUCMD_NONE)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id =  ROP_COPYPEN;
        param.rop.rop_c_id =  ROP_COPYPEN;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_ROP)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id = aria_gpe_convert_rop(pstOpt->enRopCode_Alpha);
        param.rop.rop_c_id = aria_gpe_convert_rop(pstOpt->enRopCode_Color);
        param.rop.rop_pattern = pstOpt->u32Colorize;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_BLEND)
    {
        param.gpe_op |= GPE_OP_BLEND;

        switch (pstOpt->stBlendOpt.eBlendCmd) {
            /**< fs: sa      fd: 1.0-sa */
        case TDE2_BLENDCMD_NONE:
            {
                param.blend.src_blend_fact = GL_SRC_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0     fd: 0.0 */
        case TDE2_BLENDCMD_CLEAR:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 0.0 */
        case TDE2_BLENDCMD_SRC:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCOVER:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0 */
        case TDE2_BLENDCMD_DSTOVER:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: da      fd: 0.0 */
        case TDE2_BLENDCMD_SRCIN:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: sa */
        case TDE2_BLENDCMD_DSTIN:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 0.0 */
        case TDE2_BLENDCMD_SRCOUT:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_DSTOUT:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: da      fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCATOP:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: sa */
        case TDE2_BLENDCMD_DSTATOP:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0     fd: 1.0 */
        case TDE2_BLENDCMD_ADD:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0-sa */
        case TDE2_BLENDCMD_XOR:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0  fd: 1.0*/
        case TDE2_BLENDCMD_DST:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /* user parameter*/ /*CNcomment:  ???????? */
        case TDE2_BLENDCMD_CONFIG:
        default:
            {
                param.blend.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendMode); // src2 is aria src1
                param.blend.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendMode); // src1 is aria src2
                break;
            }
        }
        param.blend_alpha.src_blend_fact = param.blend.src_blend_fact;
        param.blend_alpha.dst_blend_fact = param.blend.dst_blend_fact;
    }

    //param.need_suspend = need_suspend();
    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        aria_gpe_print_context(s32Handle);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    mt_mmz_delete(p_tmpbuf3);
    mt_mmz_delete(p_mask_buf3);
    return (ret == MT_TRUE ?MT_SUCCESS: MT_FAILURE);
}

//step1: 3d scale
//step2: rotate, GPE_SPN_VERT_MIRROR_TRANS
//step3: 3d scale
//step4: rotate, GPE_SPN_HORI_MIRROR_TRANS
//step5: rop or blend
/*
   alpha = cos(A) * 2048
   beta = sin(A) * 2048
   if input angle larger than 45, A should be changed to the value in range of (0,45]
   angle_plus=0: orginal angle in range of (0, 45]
   angle_plus=1: orginal angle in range of (45, 90)
   angle_plus=2: orginal angle in range of (90, 135]
   angle_plus=3: orginal angle in range of (135, 180)
   angle_plus=4: orginal angle in range of (180, 225]
   angle_plus=5: orginal angle in range of (225, 270)
   angle_plus=6: orginal angle in range of (270, 315]
   angle_plus=7: orginal angle in range of (315, 360)
   */

mt_s32 GpeOsiRotateAribtraryAngle(TDE_HANDLE s32Handle,
        TDE2_SURFACE_S *pstSrc,
        TDE2_SURFACE_S *pstDst,
        TDE2_RECT_S *pstSrcRect,
        TDE2_POS_S *pDstPos,
        TDE2_ROTATOR_ANGLE_S *pAngle,
        TDE2_TRAPEZ_OPT_S *pOpt)
{
    mt_s32 ret = MT_ERR_TDE_UNSUPPORTED_OPERATION;
    aria_param_t param = {0};
    rect_vsb_t src_rect = {0};
    //    u32 start_pos = 0;
    mt_s32 scale_coef[6] = {0};

    phys_addr_t p_tmpbuf = 0;
    pix_fmt_t tmpbuf_fmt = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch = 0;
    rect_vsb_t tmp_dst_rect = {0};

    phys_addr_t p_tmpbuf2 = 0;
    pix_fmt_t tmpbuf_fmt2 = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch2 = 0;
    rect_vsb_t tmp_dst_rect2 = {0};
    phys_addr_t p_mask_buf2 = 0;
    mt_u32 maskbuf_pitch2 = 0;

    phys_addr_t p_tmpbuf3 = 0;
    pix_fmt_t tmpbuf_fmt3 = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch3 = 0;
    rect_vsb_t tmp_dst_rect3 = {0};
    phys_addr_t p_mask_buf3 = 0;
    mt_u32 maskbuf_pitch3 = 0;

    phys_addr_t p_tmpbuf4 = 0;
    pix_fmt_t tmpbuf_fmt4 = PIX_FMT_ARGB8888;
    mt_u32 tmpbuf_pitch4 = 0;
    rect_vsb_t tmp_dst_rect4 = {0};
    phys_addr_t p_mask_buf4 = 0;
    mt_u32 maskbuf_pitch4 = 0;

    mt_u32 bpp = 0;
    scale_type_t scale_type;
    pos_t dst00;
    pos_t dst10;
    pos_t dst01;
    pos_t dst11;
    mt_u32 sina = pAngle->beta;
    mt_u32 cosa = pAngle->alpha;
    mt_u32 angle_plus = pAngle->angle_plus;
    mt_u32 w = 0;
    mt_u32 h = 0;
    mt_u32 hsin = 0;
    mt_u32 wsin = 0;
    mt_u32 hcos = 0;
    mt_u32 wcos = 0;
    mt_u32 w_cos = 0;
    mt_u32 h_cos = 0;
    GPE_COLORFMT_CATEGORY_E enFmtCategory;
    MT_BOOL alpha_en;

    if(pstDst == NULL || pDstPos == NULL || pOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstSrc == NULL || pstSrcRect == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if((pstSrcRect->u32Width == 0) && (pstSrcRect->u32Height == 0))
    {
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.w = pstSrc->u32Width;
        src_rect.h = pstSrc->u32Height;
    }
    else
    {
        src_rect.x = (mt_u32)(pstSrcRect->s32Xpos);
        src_rect.y = (mt_u32)(pstSrcRect->s32Ypos);
        src_rect.w = pstSrcRect->u32Width;
        src_rect.h = pstSrcRect->u32Height;
    }

    w = src_rect.w;
    h = src_rect.h;

    hsin = h * sina / 2048;
    wsin = w * sina / 2048;
    hcos = h * cosa / 2048;
    wcos = w * cosa / 2048;
    w_cos = w * 2048 / cosa;
    h_cos = h * 2048 / cosa;

    //step1-------------------------------------------rotate
    if((angle_plus != 1) && (angle_plus != 2))
    {
        if((angle_plus == 0) || (angle_plus == 3)
                || (angle_plus == 4) || (angle_plus == 7))
        {
            tmp_dst_rect.w = h;
            tmp_dst_rect.h = w;
        }
        else
        {
            tmp_dst_rect.w = w;
            tmp_dst_rect.h = h;
        }

        if(is_lut(aria_gpe_convert_fmt(pstDst->enColorFmt, &alpha_en)))
        {
            if(is_lut(aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en)) == MT_FALSE)
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            tmpbuf_fmt = aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en);
        }
        else if(aria_get_color_space(aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en)) == RGB_COLOR_SPACE)
            tmpbuf_fmt = PIX_FMT_ARGB8888;
        else
            tmpbuf_fmt = PIX_FMT_AYCBCR8888;

        bpp = get_bpp(tmpbuf_fmt);
        tmp_dst_rect.x = 0;
        tmp_dst_rect.y = 0;
        tmpbuf_pitch = (tmp_dst_rect.w * bpp + 7) / 8;

        p_tmpbuf = mt_mmz_new(tmpbuf_pitch * tmp_dst_rect.h, 8, NULL, MOD_NAME);
        if(p_tmpbuf == 0)
        {
            GPE_PRINT("\r\n malloc p_tmpbuf failed!");
            return MT_ERR_TDE_UNSUPPORTED_OPERATION;
        }

        param_init(&param);
        param.src_img_en = MT_TRUE;
        param.src_img.buf = pstSrc->u32PhyAddr;
        param.src_img.pitch = pstSrc->u32Stride;
        param.src_img.width = pstSrc->u32Width;
        param.src_img.height = pstSrc->u32Height;
        param.src_img.pix_format = aria_gpe_convert_fmt(pstSrc->enColorFmt, &param.src_img.alpha_ch_en);
        param.src_img.rect.x = src_rect.x;
        param.src_img.rect.y = src_rect.y;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = src_rect.h;

        if((param.src_img.pix_format == PIX_FMT_TILE)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
        {
            param.src_img.chroma_addr = pstSrc->u32CbCrPhyAddr;
            param.src_img.chroma_pitch = pstSrc->u32CbCrStride;
#ifdef CONFIG_MT_FPGA_GPE			
			g_debug.chroma_vir_addr = pstSrc->u32CbCrVirAddr;
#endif				
        }

        param.dst_img.buf = p_tmpbuf;
        param.dst_img.pitch = tmpbuf_pitch;
        param.dst_img.width = tmp_dst_rect.w;
        param.dst_img.height = tmp_dst_rect.h;
        param.dst_img.pix_format = tmpbuf_fmt;
        param.dst_img.rect.x = 0;
        param.dst_img.rect.y = 0;
        param.dst_img.rect.w = tmp_dst_rect.w;
        param.dst_img.rect.h = tmp_dst_rect.h;

        GPE_PRINT("\r\nstep1:src:[%d,%d,%d,%d], dst:[%d,%d,%d,%d]",
                param.src_img.rect.x, param.src_img.rect.y, param.src_img.rect.w, param.src_img.rect.h,
                param.dst_img.rect.x, param.dst_img.rect.y, param.dst_img.rect.w, param.dst_img.rect.h);

        param.gpe_op = GPE_OP_ROTATE;
        if((angle_plus == 0) || (angle_plus == 3))
            param.rotator_op = rotate_mirror_tab[TDE2_MIRROR_NONE][TDE2_ROTATE_90];
        else if((angle_plus == 5) || (angle_plus == 6))
            param.rotator_op = rotate_mirror_tab[TDE2_MIRROR_NONE][TDE2_ROTATE_180];
        else if((angle_plus == 4) || (angle_plus == 7))
            param.rotator_op = rotate_mirror_tab[TDE2_MIRROR_NONE][TDE2_ROTATE_270];

        ret = aria_gpe_set_parameter(s32Handle, &param);
        if(ret != MT_TRUE)
        {
            mt_mmz_delete(p_tmpbuf);
            aria_gpe_print_context(s32Handle);
            return MT_ERR_TDE_UNSUPPORTED_OPERATION;
        }
        ret = aria_gpe_start(s32Handle);
        //       gpe_spn_invalidate_buf((void *)param.dst_img.buf, param.dst_img.pitch * param.dst_img.height);
        if(ret != MT_TRUE)
        {
            mt_mmz_delete(p_tmpbuf);
            return MT_ERR_TDE_UNSUPPORTED_OPERATION;
        }
    }

    //step2-------------------------------------------scale

    if((angle_plus == 0) || (angle_plus == 4))
    {
        dst00.x = wsin;
        dst00.y = 0;
        dst10.x = dst00.x + hcos;
        dst10.y = 0;
        dst01.x = 0;
        dst01.y = w;
        dst11.x = dst10.x - dst00.x;
        dst11.y = w;
        tmp_dst_rect2.w = dst10.x;
        tmp_dst_rect2.h = dst11.y;
    }
    else if((angle_plus == 1) || (angle_plus == 5))
    {
        dst00.x = 0;
        dst00.y = 0;
        dst10.x = wcos;
        dst10.y = 0;
        dst01.x = hsin;
        dst01.y = h;
        dst11.x = dst10.x + dst01.x;
        dst11.y = h;
        tmp_dst_rect2.w = dst11.x;
        tmp_dst_rect2.h = dst11.y;
    }
    else if((angle_plus == 2) || (angle_plus == 6))
    {
        dst00.x = hsin;
        dst00.y = 0;
        dst10.x = dst00.x + wcos;
        dst10.y = 0;
        dst01.x = 0;
        dst01.y = h;
        dst11.x = dst10.x - dst00.x;
        dst11.y = h;
        tmp_dst_rect2.w = dst10.x;
        tmp_dst_rect2.h = dst11.y;
    }
    else
    {
        dst00.x = 0;
        dst00.y = 0;
        dst10.x = hcos;
        dst10.y = 0;
        dst01.x = wsin;
        dst01.y = w;
        dst11.x = dst10.x + dst01.x;
        dst11.y = w;
        tmp_dst_rect2.w = dst11.x;
        tmp_dst_rect2.h = dst11.y;
    }

    if((angle_plus != 1) && (angle_plus != 2))
    {
        aria_gpe_get_scale_coeff(&tmp_dst_rect, &dst00, &dst10, &dst01, &dst11, scale_coef, &scale_type);
        GPE_PRINT("\r\nstep2:src:[%d,%d,%d,%d], dst:[%d,%d],[%d,%d],[%d,%d],[%d,%d]",
                tmp_dst_rect.x, tmp_dst_rect.y, tmp_dst_rect.w, tmp_dst_rect.h, dst00.x,dst00.y,
                dst10.x, dst10.y, dst01.x,dst01.y, dst11.x, dst11.y);
    }
    else
    {
        aria_gpe_get_scale_coeff(&src_rect, &dst00, &dst10, &dst01, &dst11, scale_coef, &scale_type);
        GPE_PRINT("\r\nstep2:src:[%d,%d,%d,%d], dst:[%d,%d],[%d,%d],[%d,%d],[%d,%d]",
                src_rect.x, src_rect.y, src_rect.w, src_rect.h, dst00.x,dst00.y,
                dst10.x, dst10.y, dst01.x,dst01.y, dst11.x, dst11.y);
    }

    if((angle_plus != 1) && (angle_plus != 2))
    {
        tmpbuf_fmt2 = tmpbuf_fmt;
    }
    else
    {
        if(is_lut(aria_gpe_convert_fmt(pstDst->enColorFmt, &alpha_en)))
        {
            if(is_lut(aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en)) == MT_FALSE)
                return MT_ERR_TDE_UNSUPPORTED_OPERATION;
            tmpbuf_fmt2 = aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en);
        }
        else if(aria_get_color_space(aria_gpe_convert_fmt(pstSrc->enColorFmt, &alpha_en)) == RGB_COLOR_SPACE)
            tmpbuf_fmt2 = PIX_FMT_ARGB8888;
        else
            tmpbuf_fmt2 = PIX_FMT_AYCBCR8888;
    }
    bpp = get_bpp(tmpbuf_fmt2);

    tmp_dst_rect2.x = 0;
    tmp_dst_rect2.y = 0;
    tmpbuf_pitch2 = (tmp_dst_rect2.w * bpp + 7) / 8;

    p_tmpbuf2 = mt_mmz_new(tmpbuf_pitch2 * tmp_dst_rect2.h, 8, NULL, MOD_NAME);
    if(p_tmpbuf2 == 0)
    {
        GPE_PRINT("\r\n malloc p_tmpbuf2 failed!");
        if(p_tmpbuf != 0)
            mt_mmz_delete(p_tmpbuf);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    maskbuf_pitch2 = tmp_dst_rect2.w;
    p_mask_buf2 = mt_mmz_new(maskbuf_pitch2 * tmp_dst_rect2.h, 8, NULL, MOD_NAME);
    if(p_mask_buf2 == 0)
    {
        GPE_PRINT("\r\n malloc p_mask_buf2 failed!");
        mt_mmz_delete(p_tmpbuf2);
        if(p_tmpbuf != 0)
            mt_mmz_delete(p_tmpbuf);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    param_init(&param);
    if((angle_plus != 1) && (angle_plus != 2))
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = p_tmpbuf;
        param.src_img.pitch = tmpbuf_pitch;
        param.src_img.width = tmp_dst_rect.w;
        param.src_img.height = tmp_dst_rect.h;
        param.src_img.pix_format = tmpbuf_fmt;
        param.src_img.rect.x = 0;
        param.src_img.rect.y = 0;
        param.src_img.rect.w = tmp_dst_rect.w;
        param.src_img.rect.h = tmp_dst_rect.h;
    }
    else
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = pstSrc->u32PhyAddr;
        param.src_img.pitch = pstSrc->u32Stride;
        param.src_img.width = pstSrc->u32Width;
        param.src_img.height = pstSrc->u32Height;
        param.src_img.pix_format = aria_gpe_convert_fmt(pstSrc->enColorFmt, &param.src_img.alpha_ch_en);
        param.src_img.rect.x = src_rect.x;
        param.src_img.rect.y = src_rect.y;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = src_rect.h;

        if((param.src_img.pix_format == PIX_FMT_TILE)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
                || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
        {
            param.src_img.chroma_addr = pstSrc->u32CbCrPhyAddr;
            param.src_img.chroma_pitch = pstSrc->u32CbCrStride;
#ifdef CONFIG_MT_FPGA_GPE			
             g_debug.chroma_vir_addr = pstSrc->u32CbCrVirAddr;
#endif			 
        }
    }
    param.dst_img.buf = p_tmpbuf2;
    param.dst_img.pitch = tmpbuf_pitch2;
    param.dst_img.width = tmp_dst_rect2.w;
    param.dst_img.height = tmp_dst_rect2.h;
    param.dst_img.pix_format = tmpbuf_fmt2;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect2.w;
    param.dst_img.rect.h = tmp_dst_rect2.h;

    param.dst_with_mask = MT_TRUE;
    param.dst_mask.mbuf_addr = p_mask_buf2;
    param.dst_mask.mbuf_pitch = maskbuf_pitch2;

    param.gpe_op = GPE_OP_SCALE | GPE_OP_SCALE_TRAPZ;
    param.scale_mod = SCALE_HORI_LINE_OUT;
    param.coef[0] = scale_coef[0];
    param.coef[1] = scale_coef[1];
    param.coef[2] = scale_coef[2];
    param.coef[3] = scale_coef[3];
    param.coef[4] = scale_coef[4];
    param.coef[5] = scale_coef[5];

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(p_tmpbuf != 0)
            mt_mmz_delete(p_tmpbuf);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    if(ret != MT_TRUE)
    {
        if(p_tmpbuf != 0)
            mt_mmz_delete(p_tmpbuf);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    if(p_tmpbuf != ((phys_addr_t)0))
        mt_mmz_delete(p_tmpbuf);

    //step3-------------------------------------------rotate
    tmpbuf_fmt3 = tmpbuf_fmt2;
    bpp = get_bpp(tmpbuf_fmt3);
    tmp_dst_rect3.x = 0;
    tmp_dst_rect3.y = 0;
    tmp_dst_rect3.w = tmp_dst_rect2.h;
    tmp_dst_rect3.h = tmp_dst_rect2.w;
    tmpbuf_pitch3 = (tmp_dst_rect3.w * bpp + 7) / 8;

    p_tmpbuf3 = mt_mmz_new(tmpbuf_pitch3 * tmp_dst_rect3.h, 8, NULL, MOD_NAME);
    if(p_tmpbuf3 == 0)
    {
        GPE_PRINT("\r\n malloc p_tmpbuf3 failed!");
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    param.src_img_en = MT_TRUE;
    param.src_img.buf = p_tmpbuf2;
    param.src_img.pitch = tmpbuf_pitch2;
    param.src_img.width = tmp_dst_rect2.w;
    param.src_img.height = tmp_dst_rect2.h;
    param.src_img.pix_format = tmpbuf_fmt2;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect2.w;
    param.src_img.rect.h = tmp_dst_rect2.h;
    param.src_with_mask = MT_TRUE;
    param.src_mask_buf = p_mask_buf2;
    param.src_mask_pitch = maskbuf_pitch2;

    maskbuf_pitch3 = tmp_dst_rect3.w;

    p_mask_buf3 = mt_mmz_new(maskbuf_pitch3 * tmp_dst_rect3.h, 8, NULL, MOD_NAME);
    if(p_mask_buf3 == 0)
    {
        GPE_PRINT("\r\n malloc p_mask_buf3 failed!");
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        mt_mmz_delete(p_tmpbuf3);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    param.dst_with_mask = MT_TRUE;
    param.dst_mask.mbuf_addr = p_mask_buf3;
    param.dst_mask.mbuf_pitch = maskbuf_pitch3;

    // ====== dst image attribute =====  //
    param.dst_img.buf = p_tmpbuf3;
    param.dst_img.pitch = tmpbuf_pitch3;
    param.dst_img.width = tmp_dst_rect3.w;
    param.dst_img.height = tmp_dst_rect3.h;
    param.dst_img.pix_format = tmpbuf_fmt3;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect3.w;
    param.dst_img.rect.h = tmp_dst_rect3.h;

    GPE_PRINT("\r\nstep3:src:[%d,%d,%d,%d], dst:[%d,%d,%d,%d]",
            param.src_img.rect.x, param.src_img.rect.y, param.src_img.rect.w, param.src_img.rect.h,
            param.dst_img.rect.x, param.dst_img.rect.y, param.dst_img.rect.w, param.dst_img.rect.h);

    param.gpe_op = GPE_OP_ROTATE;
    if((angle_plus == 0) || (angle_plus == 4))
        param.rotator_op = rotate_mirror_tab[TDE2_MIRROR_NONE][TDE2_ROTATE_270];
    else
        param.rotator_op = rotate_mirror_tab[TDE2_MIRROR_NONE][TDE2_ROTATE_90];

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf2);
        mt_mmz_delete(p_mask_buf2);
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);

        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    mt_mmz_delete(p_tmpbuf2);
    mt_mmz_delete(p_mask_buf2);

    //step4-----------------------------scale
    if((angle_plus == 0) || (angle_plus == 4))
    {
        dst00.x = w_cos - wcos + hsin;
        dst00.y = 0;
        dst10.x = dst00.x + w_cos;
        dst10.y = 0;
        dst01.x = 0;
        dst01.y = wsin + hcos;
        dst11.x = dst10.x - dst00.x;
        dst11.y = dst01.y;
        tmp_dst_rect4.w = dst10.x;
        tmp_dst_rect4.h = dst11.y;
    }
    else if((angle_plus == 1) || (angle_plus == 5))
    {
        dst00.x = 0;
        dst00.y = 0;
        dst10.x = h_cos;
        dst10.y = 0;
        dst01.x = dst10.x - hcos + wsin;
        dst01.y = hsin + wcos;
        dst11.x = dst01.x + dst10.x;
        dst11.y = dst01.y;
        tmp_dst_rect4.w = dst11.x;
        tmp_dst_rect4.h = dst11.y;
    }
    else if((angle_plus == 2) || (angle_plus == 6))
    {
        dst00.x = h_cos - hcos + wsin;
        dst00.y = 0;
        dst10.x = dst00.x + h_cos;
        dst10.y = 0;
        dst01.x = 0;
        dst01.y = hsin + wcos;
        dst11.x = dst10.x - dst00.x;
        dst11.y = dst01.y;
        tmp_dst_rect4.w = dst10.x;
        tmp_dst_rect4.h = dst11.y;
    }
    else
    {
        dst00.x = 0;
        dst00.y = 0;
        dst10.x = w_cos;
        dst10.y = 0;
        dst01.x = dst10.x - wcos + hsin;
        dst01.y = wsin + hcos;
        dst11.x = dst01.x + w_cos;
        dst11.y = dst01.y;
        tmp_dst_rect4.w = dst11.x;
        tmp_dst_rect4.h = dst11.y;
    }
    aria_gpe_get_scale_coeff(&tmp_dst_rect3, &dst00, &dst10, &dst01, &dst11, scale_coef, &scale_type);

    tmpbuf_fmt4 = tmpbuf_fmt3;
    bpp = get_bpp(tmpbuf_fmt4);
    tmp_dst_rect4.x = 0;
    tmp_dst_rect4.y = 0;
    tmpbuf_pitch4 = (tmp_dst_rect4.w * bpp + 7) / 8;
    p_tmpbuf4 = mt_mmz_new(tmpbuf_pitch4 * tmp_dst_rect4.h, 8, NULL, MOD_NAME);
    if(p_tmpbuf4 == 0)
    {
        GPE_PRINT("\r\n malloc p_tmpbuf4 failed!");
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info

    param.src_img_en = MT_TRUE;
    param.src_img.buf = p_tmpbuf3;
    param.src_img.pitch = tmpbuf_pitch3;
    param.src_img.width = tmp_dst_rect3.w;
    param.src_img.height = tmp_dst_rect3.h;
    param.src_img.pix_format = tmpbuf_fmt3;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w =  tmp_dst_rect3.w;
    param.src_img.rect.h =  tmp_dst_rect3.h;

    param.src_with_mask = MT_TRUE;
    param.src_mask_buf = p_mask_buf3;
    param.src_mask_pitch = maskbuf_pitch3;

    GPE_PRINT("\r\nstep4:src:[%d,%d,%d,%d], dst:[%d,%d],[%d,%d],[%d,%d],[%d,%d]",
            param.src_img.rect.x, param.src_img.rect.y, param.src_img.rect.w, param.src_img.rect.h, dst00.x,dst00.y,
            dst10.x, dst10.y, dst01.x,dst01.y, dst11.x, dst11.y);

    maskbuf_pitch4 = tmp_dst_rect4.w;
    p_mask_buf4 = mt_mmz_new(maskbuf_pitch4 * tmp_dst_rect4.h, 8, NULL, MOD_NAME);
    if(p_mask_buf4 == ((phys_addr_t)0))
    {
        GPE_PRINT("\r\n malloc p_mask_buf4 failed!");
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        mt_mmz_delete(p_tmpbuf4);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    param.dst_with_mask = MT_TRUE;
    param.dst_mask.mbuf_addr = p_mask_buf4;
    param.dst_mask.mbuf_pitch = maskbuf_pitch4;

    // ====== dst image attribute =====  //
    param.dst_img.buf = p_tmpbuf4;
    param.dst_img.pitch = tmpbuf_pitch4;
    param.dst_img.width = tmp_dst_rect4.w;
    param.dst_img.height = tmp_dst_rect4.h;
    param.dst_img.pix_format = tmpbuf_fmt4;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect4.w;
    param.dst_img.rect.h = tmp_dst_rect4.h;

    param.gpe_op = GPE_OP_SCALE | GPE_OP_SCALE_TRAPZ;
    param.scale_mod = SCALE_HORI_LINE_OUT;
    param.coef[0] = scale_coef[0];
    param.coef[1] = scale_coef[1];
    param.coef[2] = scale_coef[2];
    param.coef[3] = scale_coef[3];
    param.coef[4] = scale_coef[4];
    param.coef[5] = scale_coef[5];

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        mt_mmz_delete(p_tmpbuf4);
        mt_mmz_delete(p_mask_buf4);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    if(ret != MT_TRUE)
    {
        mt_mmz_delete(p_tmpbuf3);
        mt_mmz_delete(p_mask_buf3);
        mt_mmz_delete(p_tmpbuf4);
        mt_mmz_delete(p_mask_buf4);

        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    mt_mmz_delete(p_tmpbuf3);
    mt_mmz_delete(p_mask_buf3);


    //step5-----------------------------------------------------blend / rop
    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    param.src_img_en = MT_TRUE;
    param.src_img.buf = p_tmpbuf4;
    param.src_img.pitch = tmpbuf_pitch4;
    param.src_img.width = tmp_dst_rect4.w;
    param.src_img.height = tmp_dst_rect4.h;
    param.src_img.pix_format = tmpbuf_fmt4;
    if((angle_plus == 0) || (angle_plus == 4)
            || (angle_plus == 3) || (angle_plus == 7))
    {
        param.src_img.rect.x = w_cos - wcos;
        param.src_img.rect.y = 0;
        param.src_img.rect.w = hsin + wcos;
        param.src_img.rect.h = wsin + hcos;
    }
    else
    {
        param.src_img.rect.x = h_cos - hcos;
        param.src_img.rect.y = 0;
        param.src_img.rect.w = hcos + wsin;
        param.src_img.rect.h = hsin + wcos;
    }
    param.src_with_mask = MT_TRUE;
    param.src_mask_buf = p_mask_buf4;
    param.src_mask_pitch = maskbuf_pitch4;

    // ====== dst image attribute =====  //
    param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    param.dst_img.rect.x = pDstPos->x;
    param.dst_img.rect.y = pDstPos->y;
    param.dst_img.rect.w = param.src_img.rect.w;
    param.dst_img.rect.h = param.src_img.rect.h;
    GPE_PRINT("\r\nstep5:src:[%d,%d,%d,%d], dst:[%d,%d,%d,%d]",
            param.src_img.rect.x, param.src_img.rect.y, param.src_img.rect.w, param.src_img.rect.h,
            param.dst_img.rect.x, param.dst_img.rect.y, param.dst_img.rect.w, param.dst_img.rect.h);

    if(pOpt->enColorKeyMode == TDE2_COLORKEY_MODE_BACKGROUND)
    {
        enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
        param.dst_img.ck_en = MT_TRUE;
        param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_min = GpeOsiGetCkeyMin(&pOpt->unColorKeyValue, enFmtCategory);
        param.dst_img.ck_max = GpeOsiGetCkeyMax(&pOpt->unColorKeyValue, enFmtCategory);

        //		  param.dst_img.key_color_select = MASK_KEY_MISMATCH;
        param.dst_img.key_color_select = pOpt->enColorKeySelect;
    }
    else if(pOpt->enColorKeyMode == TDE2_COLORKEY_MODE_CONFIG)
    {
        param.dst_img.ck_en = pOpt->stColorKeyCfg.enColorKey_bg;
        if(param.dst_img.ck_en)
        {
            enFmtCategory = GpeOsiGetFmtCategory(pstDst->enColorFmt);
            param.dst_img.key_color_mod = GpeOsiGetKeyMode(&pOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_min = GpeOsiGetCkeyMin(&pOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.ck_max = GpeOsiGetCkeyMax(&pOpt->stColorKeyCfg.unColorKeyValue_bg, enFmtCategory);
            param.dst_img.key_color_select = pOpt->stColorKeyCfg.enColorKeySelect_bg;
        }
    }

    param.gpe_op = GPE_OP_NONE;

    if(pOpt->enAluCmd == TDE2_ALUCMD_NONE)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id =  ROP_COPYPEN;
        param.rop.rop_c_id =  ROP_COPYPEN;
    }
    if (pOpt->enAluCmd & TDE2_ALUCMD_ROP)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id = aria_gpe_convert_rop(pOpt->enRopCode_Alpha);
        param.rop.rop_c_id = aria_gpe_convert_rop(pOpt->enRopCode_Color);
        param.rop.rop_pattern = pOpt->u32Colorize;
    }
    if (pOpt->enAluCmd & TDE2_ALUCMD_BLEND)
    {
        param.gpe_op |= GPE_OP_BLEND;

        switch (pOpt->stBlendOpt.eBlendCmd) {
            /**< fs: sa 	 fd: 1.0-sa */
        case TDE2_BLENDCMD_NONE:
            {
                param.blend.src_blend_fact = GL_SRC_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0	 fd: 0.0 */
        case TDE2_BLENDCMD_CLEAR:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0	 fd: 0.0 */
        case TDE2_BLENDCMD_SRC:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0	 fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCOVER:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0 */
        case TDE2_BLENDCMD_DSTOVER:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: da 	 fd: 0.0 */
        case TDE2_BLENDCMD_SRCIN:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0	 fd: sa */
        case TDE2_BLENDCMD_DSTIN:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 0.0 */
        case TDE2_BLENDCMD_SRCOUT:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0	 fd: 1.0-sa */
        case TDE2_BLENDCMD_DSTOUT:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: da 	 fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCATOP:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: sa */
        case TDE2_BLENDCMD_DSTATOP:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0	 fd: 1.0 */
        case TDE2_BLENDCMD_ADD:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0-sa */
        case TDE2_BLENDCMD_XOR:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0  fd: 1.0*/
        case TDE2_BLENDCMD_DST:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /* user parameter*/ /*CNcomment:  ???????? */
        case TDE2_BLENDCMD_CONFIG:
        default:
            {
                param.blend.src_blend_fact = aria_gpe_convert_blend(pOpt->stBlendOpt.eSrc2BlendMode); // src2 is aria src1
                param.blend.dst_blend_fact = aria_gpe_convert_blend(pOpt->stBlendOpt.eSrc1BlendMode); // src1 is aria src2
                break;
            }
        }
        param.blend_alpha.src_blend_fact = param.blend.src_blend_fact;
        param.blend_alpha.dst_blend_fact = param.blend.dst_blend_fact;
    }

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        mt_mmz_delete(p_tmpbuf4);
        mt_mmz_delete(p_mask_buf4);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    mt_mmz_delete(p_tmpbuf4);
    mt_mmz_delete(p_mask_buf4);

    return (ret == MT_TRUE ?MT_SUCCESS: MT_FAILURE);
}


mt_void GpeOsiScaleCoeff_New(mt_void)
{
    mt_u8 *pVirt;

    u32CoeffPhyAddr = mt_mmz_new(sizeof(gpe_scale_coeff), 8, NULL, MOD_NAME);
    pVirt = mt_mmz_map(u32CoeffPhyAddr, 0);
    memcpy((void *)pVirt, gpe_scale_coeff,sizeof(gpe_scale_coeff));
    mt_mmz_unmap(pVirt);
}

mt_void GpeOsiScaleCoeff_Del(mt_void)
{
    if(u32CoeffPhyAddr)
    {
        mt_mmz_delete(u32CoeffPhyAddr);
    }
}

#define REG_ARIA1_DISP_LUMA_TOP_CUR_ADDR_0    (0xffd20204)
#define REG_ARIA1_DISP_CHROMA_TOP_CUR_ADDR_0  (0xffd20248)
#define REG_ARIA1_DISP_VIDEO_INPUT_FRAME_SIZE (0xffd20008)

static mt_s32 DISP_ForceShowDS(mt_bool bStillLayer, MT_BOOL bDsEnable)
{
    mt_s32      Ret;
    mt_s32      DispDevFd = -1;
    DISP_SHOW_DS_S   DispShowDsPicPara;
    
    DispDevFd = open("/dev/mt_disp", O_RDWR|O_NONBLOCK| O_CLOEXEC, 0);
    if (DispDevFd < 0)
    {
        MT_ERR_TDE("gpe open DISP err.\n");
        return MT_FAILURE;
    }

    DispShowDsPicPara.bStillLayer = bStillLayer ? 1 : 0;
    DispShowDsPicPara.bShowDsPic = bDsEnable ? 1 : 0;
    Ret = ioctl(DispDevFd, CMD_FORCE_SHOW_DS_PIC, &DispShowDsPicPara);
    if (Ret != MT_SUCCESS) {
        MT_ERR_TDE("Call CMD_FORCE_SHOW_DS_PIC failed,ret=0x%x.", Ret);
    }
    close(DispDevFd);

    return Ret;
}

mt_s32 GpeOsiVideoScreenCapture(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt)
{
    mt_s32 ret = MT_SUCCESS;
    TDE2_RECT_S rect_src = {0};
    TDE2_RECT_S rect_dst = {0};
    TDE2_SURFACE_S stForeGround;
    rect_size_t rect_size = {0};
    mt_u32 dtmp;

#if  defined(CONFIG_MT_CHIP_SYMPHONY6)
    mt_bool decomp_en_flag = 0;
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    mt_u32 flag_addr = 0;
    mt_u32 flag_value = 0;
    int timeout = 0;
#endif

    if(pstDst == NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstDst->enColorFmt == TDE2_COLOR_FMT_TILE)
    {
#if defined(CONFIG_MT_CHIP_ARIA)
        mt_sys_read_register(REG_ARIA1_DISP_LUMA_TOP_CUR_ADDR_0, &pstDst->u32PhyAddr);
        mt_sys_read_register(REG_ARIA1_DISP_CHROMA_TOP_CUR_ADDR_0, &pstDst->u32CbCrPhyAddr);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
        mt_sys_read_register(0x1f44104c, &stForeGround.u32PhyAddr);
        mt_sys_read_register(0x1f44105c, &stForeGround.u32CbCrPhyAddr);

        pstDst->u32PhyAddr = pstDst->u32PhyAddr * 8;
        pstDst->u32CbCrPhyAddr = pstDst->u32CbCrPhyAddr * 8;
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
        mt_sys_read_register(0xbf44104c, (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register(0xbf44105c, (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

        pstDst->u32PhyAddr = pstDst->u32PhyAddr * 8;
        pstDst->u32CbCrPhyAddr = pstDst->u32CbCrPhyAddr * 8;
#elif  defined(CONFIG_MT_CHIP_SYMPHONY6)
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf440204), (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register(SYMPHONY_IO_PA(0xbf440248), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);     
#endif
        return MT_SUCCESS;
    }
    dtmp = gpe_read_register(GPE_ARIA_CMD_ID0);
    rect_size.w = (dtmp >> 16) & 0xffff;
    rect_size.h = dtmp & 0xffff;
    //	MT_MPI_DISP_GetVideoSize(&rect_size.w, &rect_size.h);

    if((rect_size.w == 0) || (rect_size.h == 0))
    {
        GPE_PRINT("\r\n video width or height can't be 0, w:%d, h:%d", rect_size.w, rect_size.h);
        return MT_FAILURE;
    }

    memset(&stForeGround, 0, sizeof(TDE2_SURFACE_S));

    stForeGround.enColorFmt = TDE2_COLOR_FMT_TILE;
#if defined(CONFIG_MT_CHIP_ARIA)
    mt_sys_read_register(REG_ARIA1_DISP_LUMA_TOP_CUR_ADDR_0, &stForeGround.u32PhyAddr);
    mt_sys_read_register(REG_ARIA1_DISP_CHROMA_TOP_CUR_ADDR_0, &stForeGround.u32CbCrPhyAddr);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    /*mt_sys_read_register(0x1f410064, &dtmp);*/
    mt_sys_read_register(0x1f443100, &dtmp);
    if(dtmp & 0x1) //decomp_en_flag ---bug 123901
    {
        mt_sys_read_register(0x1f4400bc, &dtmp);
        rect_size.w = 720;
        /*rect_size.h = 288;*/
        rect_size.h = (dtmp & 0x07ff0000) >> 16;

        mt_sys_read_register(0x1f441000, &stForeGround.u32PhyAddr);
        mt_sys_read_register(0x1f441000, &stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
        stForeGround.u32Stride = 720 * 4;
        stForeGround.enColorFmt = TDE2_COLOR_FMT_CbCrY888;
        printf("%s %d video frame compressed, capture form sd buffer!\n", __FUNCTION__, __LINE__);

        //wait for sd write back finish
        flag_addr = stForeGround.u32PhyAddr + 720 * rect_size.h * 2 - 8;
        mt_sys_write_register(flag_addr, 0xFFFFFFFF);
        mt_sys_read_register(flag_addr, &flag_value);
        while(flag_value  == 0xFFFFFFFF)
        {
            if(timeout++ > 200)
                break;
            MT_USLEEP(2000);
            mt_sys_read_register(flag_addr, &flag_value);
        }
    }
    else
    {
        mt_sys_read_register(0x1f44104c, &stForeGround.u32PhyAddr);
        mt_sys_read_register(0x1f44105c, &stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) 
    /*mt_sys_read_register(0xbf410064, &dtmp);*/
    mt_sys_read_register(0xbf443100, &dtmp);
    if(dtmp & 0x1) //decomp_en_flag ---bug 123901
    {
        mt_sys_read_register(0xbf4400bc, &dtmp);
        rect_size.w = 720;
        /*rect_size.h = 288;*/
        rect_size.h = (dtmp & 0x07ff0000) >> 16;

        mt_sys_read_register(0xbf441000, (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register(0xbf441000, (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
        stForeGround.u32Stride = 720 * 4;
        stForeGround.enColorFmt = TDE2_COLOR_FMT_CbCrY888;
        printf("%s %d video frame compressed, capture form sd buffer!\n", __FUNCTION__, __LINE__);

        //wait for sd write back finish
        flag_addr = stForeGround.u32PhyAddr + 720 * rect_size.h * 2 - 8;
        mt_sys_write_register(flag_addr, 0xFFFFFFFF);
        mt_sys_read_register(flag_addr, &flag_value);
        while(flag_value  == 0xFFFFFFFF)
        {
            if(timeout++ > 200)
                break;
            MT_USLEEP(2000);
            mt_sys_read_register(flag_addr, &flag_value);
        }
    }
    else
    {
        mt_sys_read_register(0xbf44104c, (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register(0xbf44105c, (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
    mt_sys_read_register((0xbf441000), &dtmp);
    if(dtmp & 0x1) //decomp_en_flag ---bug 123901
    {
        decomp_en_flag = 1;
        DISP_ForceShowDS(0, 1);
        GPE_PRINT("%s disp decompress is enabled, showed capture from ds!!\n", __FUNCTION__);
        MT_USLEEP(50000);
    }

    mt_sys_read_register((0xbf440204), (mt_u32 *)&stForeGround.u32PhyAddr);
    mt_sys_read_register((0xbf440248), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);
    mt_sys_read_register(SYMPHONY_IO_PA(0xbf440008), (mt_u32 *)&dtmp); 
    rect_size.w = (dtmp >> 16) & 0xffff;
    rect_size.h = dtmp & 0xffff;

    stForeGround.u32Stride = rect_size.w;
    stForeGround.u32CbCrStride = rect_size.w;

    mt_sys_read_register((0xbf44001c), &dtmp);  //get video data edian
    stForeGround.enColorFmt = ((dtmp & 0xf) == 0xf) ? TDE2_COLOR_FMT_MP1_YCbCr420MBP : TDE2_COLOR_FMT_TILE; 

    GPE_PRINT("%s %d video frame format:%d w:%d h:%d\n", __FUNCTION__, __LINE__, stForeGround.enColorFmt, rect_size.w, rect_size.h);
#endif

    stForeGround.u32Width = rect_size.w;
    stForeGround.u32Height = rect_size.h;

    rect_src.s32Xpos = 0;
    rect_src.s32Ypos = 0;
    rect_src.u32Width = rect_size.w;
    rect_src.u32Height = rect_size.h;
    if(pstDstRect == NULL)
    {
        rect_dst.s32Xpos = 0;
        rect_dst.s32Ypos = 0;
        rect_dst.u32Width = 0;
        rect_dst.u32Height = 0;
    }
    else
    {
        rect_dst.s32Xpos = pstDstRect->s32Xpos;
        rect_dst.s32Ypos = pstDstRect->s32Ypos;
        rect_dst.u32Width = pstDstRect->u32Width;
        rect_dst.u32Height = pstDstRect->u32Height;
    }

    ret = GpeOsiBlit(s32Handle, NULL, NULL,
            &stForeGround, &rect_src, pstDst, &rect_dst,
            pstOpt);

#if  defined(CONFIG_MT_CHIP_SYMPHONY6)
   if(decomp_en_flag)
    {
        MT_USLEEP(20000);    
        DISP_ForceShowDS(0, 0);
    }

#endif

    return ret;
}

#ifdef CONFIG_MT_FPGA_GPE
//step1: rotate
//step2: blur vertical
//step3: rotate
//step4: blur horizontal and blend/rop
mt_s32 gpe_gaussian_blur(TDE_HANDLE s32Handle, 
        TDE2_SURFACE_S* pstForeGround, TDE2_RECT_S  *pstForeGroundRect,
        TDE2_SURFACE_S* pstExGround, TDE2_RECT_S  *pstExGroundRect,
        TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_OPT_S* pstOpt)
{
    RET_CODE ret = ERR_NOFEATURE;
    aria_param_t param = {0};
    rect_vsb_t src_rect = {0};
    rect_vsb_t dst_rect = {0};
    rect_vsb_t ex_rect = {0};

    u32 *p_tmpbuf = NULL;
    pix_fmt_t tmpbuf_fmt = PIX_FMT_ARGB8888;
    u32 tmpbuf_pitch = 0;
    rect_vsb_t tmp_dst_rect = {0};
    
    u32 *p_tmpbuf2 = NULL;
    pix_fmt_t tmpbuf_fmt2 = PIX_FMT_ARGB8888;
    u32 tmpbuf_pitch2 = 0;
    rect_vsb_t tmp_dst_rect2 = {0};    
    
    u32 *p_tmpbuf3 = NULL;
    pix_fmt_t tmpbuf_fmt3 = PIX_FMT_ARGB8888;
    u32 tmpbuf_pitch3 = 0;
    rect_vsb_t tmp_dst_rect3 = {0};    
    u32 bpp = 0;
//    u32 start_pos = 0;
    MT_BOOL alpha_en = MT_FALSE;

        
    if(pstDst == NULL || pstDstRect == NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

 //   if(pstExGround == NULL || pstExGroundRect == NULL)
 //        return MT_ERR_TDE_NULL_PTR;

    else if(pstForeGroundRect == NULL || pstForeGround == NULL)
        return MT_ERR_TDE_NULL_PTR;


    if((pstForeGroundRect->u32Width== 0) && (pstForeGroundRect->u32Height== 0))
    {
        src_rect.x = 0;
        src_rect.y = 0;
        src_rect.w = pstForeGround->u32Width;
        src_rect.h = pstForeGround->u32Height;
    }
    else
    {
        src_rect.x = pstForeGroundRect->s32Xpos;
        src_rect.y = pstForeGroundRect->s32Xpos;
        src_rect.w = pstForeGroundRect->u32Width;
        src_rect.h = pstForeGroundRect->u32Height;
    }

      if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
    {
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.w = pstDst->u32Width;
        dst_rect.h = pstDst->u32Height;
    }
    else
    {
        dst_rect.x = (mt_u32)(pstDstRect->s32Xpos);
        dst_rect.y = (mt_u32)(pstDstRect->s32Ypos);
        dst_rect.w = pstDstRect->u32Width;
        dst_rect.h = pstDstRect->u32Height;
    }

    if(pstExGround != NULL && pstExGroundRect != NULL)
    {
         if((pstExGroundRect->u32Width == 0) && (pstExGroundRect->u32Height == 0))
        {
            ex_rect.x = 0;
            ex_rect.y = 0;
            ex_rect.w = pstExGround->u32Width;
            ex_rect.h = pstExGround->u32Height;
        }
        else
        {
            ex_rect.x = (mt_u32)(pstExGroundRect->s32Xpos);
            ex_rect.y = (mt_u32)(pstExGroundRect->s32Ypos);
            ex_rect.w = pstExGroundRect->u32Width;
            ex_rect.h = pstExGroundRect->u32Height;
        }
    }

    if((src_rect.w != dst_rect.w) ||(src_rect.h != dst_rect.h))
    {
        
        GPE_PRINT("<%s> : <%d> MT_ERR_TDE_INVALID_PARA----\n", __FUNCTION__, __LINE__);
         return MT_ERR_TDE_INVALID_PARA;
    }
    if(pstExGround != NULL && pstExGroundRect != NULL)
    {
         if((ex_rect.w != dst_rect.w) ||(ex_rect.h != dst_rect.h))
        {
            
            GPE_PRINT("<%s> : <%d> MT_ERR_TDE_INVALID_PARA----\n", __FUNCTION__, __LINE__);
             return MT_ERR_TDE_INVALID_PARA;
        }
    }

    
    //step1-------------------------------------------rotate
    //src: src_rect.w, src_rect.h
    //dst: src_rect.h, dst_rect.w

    GPE_PRINT("\r\n step1 rotate-----------\n");
    if(is_lut(aria_gpe_convert_fmt(pstDst->enColorFmt, &alpha_en)))
    {
        tmpbuf_fmt = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en);
        if(is_lut(tmpbuf_fmt) == FALSE)
            return ERR_NOFEATURE;
        if(tmpbuf_fmt == PIX_FMT_RGBPALETTE1 || tmpbuf_fmt == PIX_FMT_RGBPALETTE2
                    || tmpbuf_fmt == PIX_FMT_RGBPALETTE4)
                tmpbuf_fmt = PIX_FMT_RGBPALETTE8;
    }
    else if(aria_get_color_space(aria_gpe_convert_fmt(pstForeGround->enColorFmt, &alpha_en)) == RGB_COLOR_SPACE)
        tmpbuf_fmt = PIX_FMT_ARGB8888;
    else 
        tmpbuf_fmt = PIX_FMT_AYCBCR8888;
    bpp = get_bpp(tmpbuf_fmt);

    tmp_dst_rect.x = 0;
    tmp_dst_rect.y = 0;
    tmp_dst_rect.w = src_rect.h;
    tmp_dst_rect.h = dst_rect.w;
    tmpbuf_pitch = (tmp_dst_rect.w * bpp + 7) / 8;


    p_tmpbuf = (mt_u32*)mt_mmz_new(tmpbuf_pitch * tmp_dst_rect.h, 8, NULL, MOD_NAME);
   MT_INFO_TDE("\r\n p_tmpbuf:0x%08x", p_tmpbuf);
   if(p_tmpbuf == NULL)
   {
       g_addr_offset = 0;
       MT_ERR_TDE("\r\n malloc p_tmpbuf failed!");
       return MT_ERR_TDE_UNSUPPORTED_OPERATION;
   }


        param_init(&param);

    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    param.src_img_en = MT_TRUE;
    param.src_img.buf = pstForeGround->u32PhyAddr;
    param.src_img.pitch = pstForeGround->u32Stride;
    param.src_img.width = pstForeGround->u32Width;
    param.src_img.height = pstForeGround->u32Height;
    param.src_img.pix_format = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &param.src_img.alpha_ch_en);
    param.src_img.rect.x = src_rect.x;
    param.src_img.rect.y = src_rect.y;
    param.src_img.rect.w = src_rect.w;
    param.src_img.rect.h = src_rect.h;

    if((param.src_img.pix_format == PIX_FMT_TILE)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV444)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV422)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV420)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV444_UVSWAP)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV422_UVSWAP)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV420_UVSWAP)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV422_1x2_UVSWAP)
        || (param.src_img.pix_format == PIX_FMT_SP_YUV422_2x1_UVSWAP)
        || (param.src_img.pix_format == PIX_FMT_SP_CMYK)
        || (param.src_img.pix_format == PIX_FMT_SP_CMYK_SWAP))
    {
        param.src_img.chroma_addr = (mt_u32)(pstForeGround->u32CbCrPhyAddr);
        param.src_img.chroma_pitch = pstForeGround->u32CbCrStride; 
        g_debug.chroma_vir_addr = pstForeGround->u32CbCrVirAddr;
    }
    if((param.src_img.pix_format >= PIX_FMT_XY) && (param.src_img.pix_format <= PIX_FMT_XYLC))
    {
        param.src_img.xylc_num = pstOpt->xylc_cmd_num;
        param.src_img.xylc_color = pstOpt->xylc_color;
    }

    param.dst_img.buf = (phys_addr_t)p_tmpbuf;
    param.dst_img.pitch = tmpbuf_pitch;
    param.dst_img.width = tmp_dst_rect.w;
    param.dst_img.height = tmp_dst_rect.h;
    param.dst_img.pix_format = tmpbuf_fmt;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w =  tmp_dst_rect.w;
    param.dst_img.rect.h =  tmp_dst_rect.h;
       
    param.gpe_op = GPE_OP_ROTATE;
    param.rotator_op = GPE_CCT_TRANS;
#ifdef SUPPORT_CMDFIFO    
    param.need_suspend = FALSE;
#endif

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(p_tmpbuf != NULL)
        {
            mt_mmz_delete((phys_addr_t)p_tmpbuf);
            p_tmpbuf = NULL; 
        }        
        g_addr_offset = 0;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    ret = aria_gpe_start(s32Handle);

    if(ret != TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(p_tmpbuf)
        {
          mt_mmz_delete((phys_addr_t)p_tmpbuf);
          p_tmpbuf = NULL; 
        }
        g_addr_offset = 0;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

//step2-------------------------------------------blur vertical
    GPE_PRINT("\r\n step2 blur vertical-------------------------");
    tmpbuf_fmt2 = tmpbuf_fmt;
    bpp = get_bpp(tmpbuf_fmt2);    
    tmp_dst_rect2.x = 0;
    tmp_dst_rect2.y = 0;
    tmp_dst_rect2.w = tmp_dst_rect.w;
    tmp_dst_rect2.h = tmp_dst_rect.h;
    tmpbuf_pitch2 = (tmp_dst_rect2.w * bpp + 7) / 8;    

    p_tmpbuf2 = (mt_u32*)mt_mmz_new( tmpbuf_pitch2 * tmp_dst_rect2.h, 8, NULL, MOD_NAME);
     
      GPE_PRINT("\r\n p_tmpbuf2:0x%"PRIx64, (ulong)p_tmpbuf2);
      if(p_tmpbuf2 == NULL)
      {
          g_addr_offset = 0;
          MT_ERR_TDE("\r\n malloc p_tmpbuf failed!");
          return MT_ERR_TDE_UNSUPPORTED_OPERATION;
      }


    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info    

            
    param.src_img_en = TRUE;
    param.src_img.buf = (phys_addr_t)p_tmpbuf;
    param.src_img.pitch = tmpbuf_pitch;
    param.src_img.width = tmp_dst_rect.w;
    param.src_img.height = tmp_dst_rect.h;
    param.src_img.pix_format = tmpbuf_fmt;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect.w;
    param.src_img.rect.h = tmp_dst_rect.h;  

    // ====== dst image attribute =====  //
    param.dst_img.buf = (phys_addr_t)(p_tmpbuf2);
    param.dst_img.pitch = tmpbuf_pitch2;
    param.dst_img.width = tmp_dst_rect2.w;
    param.dst_img.height = tmp_dst_rect2.h;
    param.dst_img.pix_format = tmpbuf_fmt2;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect2.w;
    param.dst_img.rect.h = tmp_dst_rect2.h;

    if(pstOpt->enGsBlur)
    {
        param.gpe_op |= GPE_OP_BLUR;
        param.blur_tap = (mt_u32)(pstOpt->stBlurOpt.blur_level * 2 + 3);
        
        printf("-------------------------------------------<%s> : <%d> enGsBlur\n", __FUNCTION__, __LINE__);
    }

    param.gpe_op |= GPE_OP_ROP;
    param.rop.rop_a_id = ROP_COPYPEN;
    param.rop.rop_c_id = ROP_COPYPEN;            
#ifdef SUPPORT_CMDFIFO
    param.need_suspend = FALSE;//need_suspend();
#endif

    ret = aria_gpe_set_parameter(s32Handle, &param);

    if(ret != MT_TRUE)
    {
     aria_gpe_print_context(s32Handle);
     if(p_tmpbuf != NULL)
     {
         mt_mmz_delete((phys_addr_t)p_tmpbuf);
     }  
      if(p_tmpbuf2)
     {
         mt_mmz_delete((phys_addr_t)p_tmpbuf2);
     }  
     g_addr_offset = 0;
     return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }

    ret = aria_gpe_start(s32Handle);

    if(ret != TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(p_tmpbuf)
        {
            mt_mmz_delete((phys_addr_t)p_tmpbuf);
        }
        if(p_tmpbuf2)
        {
            mt_mmz_delete((phys_addr_t)p_tmpbuf2);
        }  
        g_addr_offset = 0;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    //free p_tmpbuf
     if(p_tmpbuf)
     {
         mt_mmz_delete((phys_addr_t)p_tmpbuf);
         p_tmpbuf = NULL;
     }  

//step3-----------------------------rotate
    GPE_PRINT("\r\n step3 rotate-------------------------");
    tmpbuf_fmt3 = tmpbuf_fmt2;
    bpp = get_bpp(tmpbuf_fmt3);    
    tmp_dst_rect3.x = 0;
    tmp_dst_rect3.y = 0;
    tmp_dst_rect3.w = tmp_dst_rect2.h;
    tmp_dst_rect3.h = tmp_dst_rect2.w;
    tmpbuf_pitch3 = (tmp_dst_rect3.w * bpp + 7) / 8;    

    p_tmpbuf3 = (mt_u32*)mt_mmz_new(tmpbuf_pitch3 * tmp_dst_rect3.h, 8, NULL, MOD_NAME);
     GPE_PRINT("\r\n p_tmpbuf3:0x%"PRIx64, (ulong)p_tmpbuf3);
     if(p_tmpbuf3 == NULL)
     {
         g_addr_offset = 0;
         MT_ERR_TDE("\r\n malloc p_tmpbuf failed!");
         return MT_ERR_TDE_UNSUPPORTED_OPERATION;
     }


    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info
    
    param.src_img_en = TRUE;
    param.src_img.buf = (phys_addr_t)(p_tmpbuf2);
    param.src_img.pitch = tmpbuf_pitch2;
    param.src_img.width = tmp_dst_rect2.w;
    param.src_img.height = tmp_dst_rect2.h;
    param.src_img.pix_format = tmpbuf_fmt2;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect2.w;
    param.src_img.rect.h = tmp_dst_rect2.h;     

    // ====== dst image attribute =====  //
    param.dst_img.buf = (phys_addr_t)(p_tmpbuf3);
    param.dst_img.pitch = tmpbuf_pitch3;
    param.dst_img.width = tmp_dst_rect3.w;
    param.dst_img.height = tmp_dst_rect3.h;
    param.dst_img.pix_format = tmpbuf_fmt3;
    param.dst_img.rect.x = 0;
    param.dst_img.rect.y = 0;
    param.dst_img.rect.w = tmp_dst_rect3.w;
    param.dst_img.rect.h = tmp_dst_rect3.h;

    param.gpe_op = GPE_OP_ROTATE;
    param.rotator_op = GPE_CCT_TRANS;       
#ifdef SUPPORT_CMDFIFO
    param.need_suspend = FALSE;//need_suspend();
#endif

    ret = aria_gpe_set_parameter(s32Handle, &param);
    
      if(ret != MT_TRUE)
      {
       aria_gpe_print_context(s32Handle);
       if(p_tmpbuf != NULL)
       {
           mt_mmz_delete((phys_addr_t)p_tmpbuf);
       }  
        if(p_tmpbuf2)
       {
           mt_mmz_delete((phys_addr_t)p_tmpbuf2);
       }  
        if(p_tmpbuf3)
       {
           mt_mmz_delete((phys_addr_t)p_tmpbuf3);
       }  
       g_addr_offset = 0;
       return MT_ERR_TDE_UNSUPPORTED_OPERATION;
      }
    
      ret = aria_gpe_start(s32Handle);
    
      if(ret != TRUE)
      {
          aria_gpe_print_context(s32Handle);
          if(p_tmpbuf)
          {
              mt_mmz_delete((phys_addr_t)p_tmpbuf);
          }
          if(p_tmpbuf2)
          {
              mt_mmz_delete((phys_addr_t)p_tmpbuf2);
          }  
           if(p_tmpbuf3)
           {
               mt_mmz_delete((phys_addr_t)p_tmpbuf3);
           }  
          g_addr_offset = 0;
          return MT_ERR_TDE_UNSUPPORTED_OPERATION;
      }

   //free p_tmpbuf2
     if(p_tmpbuf2)
     {
         mt_mmz_delete((phys_addr_t)p_tmpbuf2);
         p_tmpbuf2 = NULL;
     }  


//step4-----------------------------------------------------blur horizontal
    GPE_PRINT("\r\n step4 blur horizontal-------------------------");
    param_init(&param);
    //// ======= src image attribute ======= //
    // src image buffer and pitch info    
    param.src_img_en = TRUE;
    param.src_img.buf = (phys_addr_t)(p_tmpbuf3);
    param.src_img.pitch = tmpbuf_pitch3;
    param.src_img.width = tmp_dst_rect3.w;
    param.src_img.height = tmp_dst_rect3.h;
    param.src_img.pix_format = tmpbuf_fmt3;
    param.src_img.rect.x = 0;
    param.src_img.rect.y = 0;
    param.src_img.rect.w = tmp_dst_rect3.w;
    param.src_img.rect.h = tmp_dst_rect3.h;

    // ====== dst image attribute =====  //
     param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    param.dst_img.rect.x = dst_rect.x;
    param.dst_img.rect.y = dst_rect.y;
    param.dst_img.rect.w = dst_rect.w;
    param.dst_img.rect.h = dst_rect.h;
    

 GPE_PRINT("\r\n src[%d,%d,%d,%d], dst[%d,%d,%d,%d]", param.src_img.rect.x,param.src_img.rect.y,param.src_img.rect.w,param.src_img.rect.h,
       param.dst_img.rect.x,param.dst_img.rect.y,param.dst_img.rect.w,param.dst_img.rect.h);

    // ======= ex image attribute ======= //
    if(pstExGround != NULL)
    {
        param.ex_img_en = MT_TRUE;
        param.ex_img.buf = pstExGround->u32PhyAddr;
        param.ex_img.pitch = pstExGround->u32Stride;
        param.ex_img.width = pstExGround->u32Width;
        param.ex_img.height = pstExGround->u32Height;
        param.ex_img.pix_format = aria_gpe_convert_fmt(pstExGround->enColorFmt, &param.ex_img.alpha_ch_en);
        param.ex_img.rect.x = ex_rect.x;
        param.ex_img.rect.y = ex_rect.y;
        param.ex_img.rect.w = ex_rect.w;
        param.ex_img.rect.h = ex_rect.h;
    }
    
    
    if(pstOpt->enGsBlur)
      {
          param.gpe_op |= GPE_OP_BLUR;
          param.blur_tap = (mt_u32)(pstOpt->stBlurOpt.blur_level * 2 + 3);
          
          printf("-------------------------------------------<%s> : <%d> enGsBlur\n", __FUNCTION__, __LINE__);
      }

    if(pstOpt->enAluCmd == TDE2_ALUCMD_NONE)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id =  ROP_COPYPEN;
        param.rop.rop_c_id =  ROP_COPYPEN;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_ROP)
    {
        param.gpe_op |= GPE_OP_ROP;
        param.rop.rop_a_id = aria_gpe_convert_rop(pstOpt->enRopCode_Alpha);
        param.rop.rop_c_id = aria_gpe_convert_rop(pstOpt->enRopCode_Color);
        param.rop.rop_pattern = pstOpt->u32Colorize;
    }
    if (pstOpt->enAluCmd & TDE2_ALUCMD_BLEND)
    {
        param.gpe_op |= GPE_OP_BLEND;

        switch (pstOpt->stBlendOpt.eBlendCmd) {
            /**< fs: sa      fd: 1.0-sa */
        case TDE2_BLENDCMD_NONE:
            {
                param.blend.src_blend_fact = GL_SRC_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0     fd: 0.0 */
        case TDE2_BLENDCMD_CLEAR:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 0.0 */
        case TDE2_BLENDCMD_SRC:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 1.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCOVER:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0 */
        case TDE2_BLENDCMD_DSTOVER:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: da      fd: 0.0 */
        case TDE2_BLENDCMD_SRCIN:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: sa */
        case TDE2_BLENDCMD_DSTIN:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: 0.0 */
        case TDE2_BLENDCMD_SRCOUT:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ZERO;
                break;
            }
            /**< fs: 0.0     fd: 1.0-sa */
        case TDE2_BLENDCMD_DSTOUT:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: da      fd: 1.0-sa */
        case TDE2_BLENDCMD_SRCATOP:
            {
                param.blend.src_blend_fact = GL_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0-da  fd: sa */
        case TDE2_BLENDCMD_DSTATOP:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_SRC_ALPHA;
                break;
            }
            /**< fs: 1.0     fd: 1.0 */
        case TDE2_BLENDCMD_ADD:
            {
                param.blend.src_blend_fact = GL_ONE;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /**< fs: 1.0-da  fd: 1.0-sa */
        case TDE2_BLENDCMD_XOR:
            {
                param.blend.src_blend_fact = GL_ONE_MINUS_DST_ALPHA;
                param.blend.dst_blend_fact = GL_ONE_MINUS_SRC_ALPHA;
                break;
            }
            /**< fs: 0.0  fd: 1.0*/
        case TDE2_BLENDCMD_DST:
            {
                param.blend.src_blend_fact = GL_ZERO;
                param.blend.dst_blend_fact = GL_ONE;
                break;
            }
            /* user parameter*/ /*CNcomment:  ???????? */
        case TDE2_BLENDCMD_CONFIG:
        default:
            {
                param.blend.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendMode); // src2 is aria src1
                param.blend.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendMode); // src1 is aria src2
                break;
            }

        }
        if(pstOpt->stBlendOpt.bBlendModeAlphaEnable && (pstOpt->stBlendOpt.eBlendCmd == TDE2_BLENDCMD_CONFIG))
        {
            param.blend_alpha.src_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc2BlendModeAlpha);
            param.blend_alpha.dst_blend_fact = aria_gpe_convert_blend(pstOpt->stBlendOpt.eSrc1BlendModeAlpha);
        }
        else
        {
            param.blend_alpha.src_blend_fact = param.blend.src_blend_fact;
            param.blend_alpha.dst_blend_fact = param.blend.dst_blend_fact;
        }
        if((pstForeGround != NULL) && (pstOpt->stBlendOpt.bGlobalAlphaEnable))
        {
            param.src_img.plane_alpha_en = MT_TRUE;
            param.src_img.plane_alpha = pstOpt->u8GlobalAlpha;
        }

        if((pstForeGround != NULL) && pstOpt->stBlendOpt.bSrc2AlphaPremulti)
        {
            param.src_img.alpha_pre_multed = MT_FALSE;
        }
    }
   
   if(pstOpt->enMultiply)
      {
          param.gpe_op |= GPE_OP_DMULT;
      }
   
      if(pstOpt->enStencil)
      {
          param.gpe_op |= GPE_OP_DSTEN;
      }
   
      if(pstOpt->enPaint)
      {
          param.gpe_op |= GPE_OP_PAINT;
          memcpy(&param.paint, &pstOpt->stPaintOpt, sizeof(TDE2_PAINT_CFG_S));
      }
   
      if(pstOpt->enAMapLogical)
      {
          param.gpe_op |= GPE_OP_ALPHAMAP;
          param.alpha_map_mod = GPE_CCT_ALPHA_MAP_LOGICAL;
      }
   
      if(pstOpt->enAMapMix)
      {
          param.gpe_op |= GPE_OP_ALPHAMAP;
          param.alpha_map_mod = GPE_CCT_ALPHA_MAP_MIX_NORMAL;
      }
      if(pstOpt->enAMapMixEx)
      {
          param.gpe_op |= GPE_OP_ALPHAMAP;
          param.alpha_map_mod = GPE_CCT_ALPHA_MAP_MIX_EX;
      }

   
#ifdef SUPPORT_CMDFIFO
    param.need_suspend = FALSE;//need_suspend();
#endif
    ret = aria_gpe_set_parameter(s32Handle, &param);      
    if(ret != MT_TRUE)
    {
         aria_gpe_print_context(s32Handle);
         if(p_tmpbuf != NULL)
         {
             mt_mmz_delete((phys_addr_t)p_tmpbuf);
         }  
          if(p_tmpbuf2)
         {
             mt_mmz_delete((phys_addr_t)p_tmpbuf2);
         }  
          if(p_tmpbuf3)
         {
             mt_mmz_delete((phys_addr_t)p_tmpbuf3);
         }  
         g_addr_offset = 0;
         return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
  
    ret = aria_gpe_start(s32Handle);
  
    if(ret != TRUE)
    {
        aria_gpe_print_context(s32Handle);
        if(p_tmpbuf)
        {
            mt_mmz_delete((phys_addr_t)p_tmpbuf);
        }
        if(p_tmpbuf2)
        {
            mt_mmz_delete((phys_addr_t)p_tmpbuf2);
        }  
         if(p_tmpbuf3)
         {
             mt_mmz_delete((phys_addr_t)p_tmpbuf3);
         }  
        g_addr_offset = 0;
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
     if(p_tmpbuf3)
     {
         mt_mmz_delete((phys_addr_t)p_tmpbuf3);
     }  
 
    return (ret == TRUE ?SUCCESS: ERR_NOFEATURE);    
}
#endif
#ifdef CONFIG_MT_FPGA_GPE
static void get_vout_size(u32 *p_width, u32 *p_height)  //need test
{
    u32 activeheight = 0;

     //0xbf47002c[26:16]
	// activeheight = (*((volatile u32 *)0xbf47002c) >> 16) & 0x7ff;
     mt_sys_read_register(0xbf47002c, &activeheight);

     activeheight = (activeheight >> 16)  & 0x7ff;

     printf("<%s> :  : <%d>\n", __FUNCTION__, activeheight);
    if(activeheight == 1080)
    {
		*p_width = 1920;
		*p_height = 1080;
    }
    else if(activeheight == 720)
    {
		*p_width = 1280;
		*p_height = 720;
    }
    else if(activeheight == 576)
    {
		*p_width = 720;
		*p_height = 576;
    }
    else if(activeheight == 480)
    {
		*p_width = 720;
		*p_height = 480;
    }
    else
    {
    	*p_width = 0;
    	*p_height = 0;
    }
}
mt_s32 GpeOsiVideoScreenCaptureFpga(TDE_HANDLE s32Handle, TDE2_SURFACE_S* pstDst, TDE2_RECT_S  *pstDstRect, TDE2_RECT_S  *pstSrcRect, TDE2_OPT_S* pstOpt)
{
    mt_s32 ret = MT_SUCCESS;
    TDE2_RECT_S rect_src = {0};
    TDE2_RECT_S rect_dst = {0};
    TDE2_SURFACE_S stForeGround;
    rect_size_t rect_size = {0};
    mt_u32 dtmp;

    mt_u32 flag_addr = 0;
    mt_u32 flag_value = 0;
    int timeout = 0;
    
    mt_u32 vout_w = 1920;
    mt_u32 vout_h = 1080;

    if(pstDst == NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstDst->enColorFmt == TDE2_COLOR_FMT_TILE)
    {
#if defined(CONFIG_MT_CHIP_ARIA)
        mt_sys_read_register(REG_ARIA1_DISP_LUMA_TOP_CUR_ADDR_0, &pstDst->u32PhyAddr);
        mt_sys_read_register(REG_ARIA1_DISP_CHROMA_TOP_CUR_ADDR_0, &pstDst->u32CbCrPhyAddr);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
        mt_sys_read_register(0x1f44104c, &stForeGround.u32PhyAddr);
        mt_sys_read_register(0x1f44105c, &stForeGround.u32CbCrPhyAddr);

        pstDst->u32PhyAddr = pstDst->u32PhyAddr * 8;
        pstDst->u32CbCrPhyAddr = pstDst->u32CbCrPhyAddr * 8;
#elif defined(CONFIG_MT_CHIP_SYMPHONY4) 
        mt_sys_read_register((0xbf44104c), (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register((0xbf44105c), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

        pstDst->u32PhyAddr = pstDst->u32PhyAddr * 8;
        pstDst->u32CbCrPhyAddr = pstDst->u32CbCrPhyAddr * 8;
#elif  defined(CONFIG_MT_CHIP_SYMPHONY6)
        mt_sys_read_register((0xbf440204), (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register((0xbf440248), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);     
#endif
        return MT_SUCCESS;
    }
    dtmp = gpe_read_register(GPE_ARIA_CMD_ID0);
    rect_size.w = (dtmp >> 16) & 0xffff;
    rect_size.h = dtmp & 0xffff;
        get_vout_size(&vout_w , &vout_h);

    if((rect_size.w == 0) || (rect_size.h == 0) || (vout_w == 0) || (vout_h == 0))
    {
        GPE_PRINT("\r\n video width or height can't be 0, w:%d, h:%d", rect_size.w, rect_size.h);
        return MT_FAILURE;
    }

    memset(&stForeGround, 0, sizeof(TDE2_SURFACE_S));

    stForeGround.enColorFmt = TDE2_COLOR_FMT_TILE;
#if defined(CONFIG_MT_CHIP_ARIA)
    mt_sys_read_register(REG_ARIA1_DISP_LUMA_TOP_CUR_ADDR_0, &stForeGround.u32PhyAddr);
    mt_sys_read_register(REG_ARIA1_DISP_CHROMA_TOP_CUR_ADDR_0, &stForeGround.u32CbCrPhyAddr);
#elif defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2)
    /*mt_sys_read_register(0x1f410064, &dtmp);*/
    mt_sys_read_register(0x1f443100, &dtmp);
    if(dtmp & 0x1) //decomp_en_flag ---bug 123901
    {
        mt_sys_read_register(0x1f4400bc, &dtmp);
        rect_size.w = 720;
        /*rect_size.h = 288;*/
        rect_size.h = (dtmp & 0x07ff0000) >> 16;

        mt_sys_read_register(0x1f441000, &stForeGround.u32PhyAddr);
        mt_sys_read_register(0x1f441000, &stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
        stForeGround.u32Stride = 720 * 4;
        stForeGround.enColorFmt = TDE2_COLOR_FMT_CbCrY888;
        printf("%s %d video frame compressed, capture form sd buffer!\n", __FUNCTION__, __LINE__);

        //wait for sd write back finish
        flag_addr = stForeGround.u32PhyAddr + 720 * rect_size.h * 2 - 8;
        mt_sys_write_register(flag_addr, 0xFFFFFFFF);
        mt_sys_read_register(flag_addr, &flag_value);
        while(flag_value  == 0xFFFFFFFF)
        {
            if(timeout++ > 200)
                break;
            MT_USLEEP(2000);
            mt_sys_read_register(flag_addr, &flag_value);
        }
    }
    else
    {
        mt_sys_read_register(0x1f44104c, &stForeGround.u32PhyAddr);
        mt_sys_read_register(0x1f44105c, &stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
    /*mt_sys_read_register(0xbf410064, &dtmp);*/
    mt_sys_read_register((0xbf443100), &dtmp);
    if(dtmp & 0x1) //decomp_en_flag ---bug 123901
    {
        mt_sys_read_register((0xbf4400bc), &dtmp);
        rect_size.w = 720;
        /*rect_size.h = 288;*/
        rect_size.h = (dtmp & 0x07ff0000) >> 16;

        mt_sys_read_register((0xbf441000), (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register((0xbf441000), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
        stForeGround.u32Stride = 720 * 4;
        stForeGround.enColorFmt = TDE2_COLOR_FMT_CbCrY888;
        printf("%s %d video frame compressed, capture form sd buffer!\n", __FUNCTION__, __LINE__);

        //wait for sd write back finish
        flag_addr = stForeGround.u32PhyAddr + 720 * rect_size.h * 2 - 8;
        mt_sys_write_register(flag_addr, 0xFFFFFFFF);
        mt_sys_read_register(flag_addr, &flag_value);
        while(flag_value  == 0xFFFFFFFF)
        {
            if(timeout++ > 200)
                break;
            MT_USLEEP(2000);
            printf("<%s> : <%d> : timeout <%d>\n", __FUNCTION__, __LINE__,  timeout);
            mt_sys_read_register(flag_addr, &flag_value);
        }
    }
    else
    {
        mt_sys_read_register((0xbf44104c), (mt_u32 *)&stForeGround.u32PhyAddr);
        mt_sys_read_register((0xbf44105c), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

        stForeGround.u32PhyAddr = stForeGround.u32PhyAddr * 8;
        stForeGround.u32CbCrPhyAddr = stForeGround.u32CbCrPhyAddr * 8;
    }
#elif  defined(CONFIG_MT_CHIP_SYMPHONY6)
   mt_sys_read_register((0xbf441000), &dtmp);
 //  disp_st_vid_get_vout_size(DISP_CHANNEL_HD, &vout_h, &vout_w);
 //dtmp = 0x1;

   if(dtmp & 0x1) //decomp_en_flag ---bug 123901
   {
       mt_sys_read_register((0xbf448004), &dtmp);
       rect_size.w = 720;
       /*rect_size.h = 288;*/
       rect_size.h = (dtmp & 0xFFF);

       mt_sys_read_register((0xbf447008), (mt_u32 *)&stForeGround.u32PhyAddr);
       mt_sys_read_register((0xbf447008), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);

       stForeGround.u32PhyAddr = (stForeGround.u32PhyAddr & 0x1FFFFFFF) * 8;
       stForeGround.u32CbCrPhyAddr = (stForeGround.u32CbCrPhyAddr & 0x1FFFFFFF) * 8;
       stForeGround.u32Stride = 720 * 4;
       stForeGround.enColorFmt = TDE2_COLOR_FMT_CbCrY888;
       printf("%s %d video frame compressed, capture form sd buffer!\n", __FUNCTION__, __LINE__);

       //wait for sd write back finish
       flag_addr = stForeGround.u32PhyAddr + 720 * rect_size.h * 2 - 8;
       mt_sys_write_register((flag_addr), 0xFFFFFFFF);
       mt_sys_read_register((flag_addr), &flag_value);
       while(flag_value  == 0xFFFFFFFF)
       {
           if(timeout++ > 200)
               break;
           MT_USLEEP(2000);
       //    printf("<%s> : <%d> : timeout <%d>\n", __FUNCTION__, __LINE__,  timeout);
           mt_sys_read_register(flag_addr, &flag_value);
       }
   }
   else
   {
      // stForeGround.u32Stride = rect_size.w * 4;
       mt_sys_read_register((0xbf440204), (mt_u32 *)&stForeGround.u32PhyAddr);
       mt_sys_read_register((0xbf440248), (mt_u32 *)&stForeGround.u32CbCrPhyAddr);
   }

#endif

    stForeGround.u32Width = rect_size.w;
    stForeGround.u32Height = rect_size.h;


    rect_src.s32Xpos = pstSrcRect->s32Xpos * rect_size.w / vout_w;
    rect_src.s32Ypos = pstSrcRect->s32Ypos * rect_size.h / vout_h;
    rect_src.u32Width = pstSrcRect->u32Width * rect_size.w / vout_w;
    rect_src.u32Height = pstSrcRect->u32Height* rect_size.h / vout_h; 
   /* rect_src.s32Xpos = 0;
    rect_src.s32Ypos = 0;
    rect_src.u32Width = rect_size.w;
    rect_src.u32Height = rect_size.h;*/
    if(pstDstRect == NULL)
    {
        rect_dst.s32Xpos = 0;
        rect_dst.s32Ypos = 0;
        rect_dst.u32Width = 0;
        rect_dst.u32Height = 0;
    }
    else
    {
        rect_dst.s32Xpos = pstDstRect->s32Xpos;
        rect_dst.s32Ypos = pstDstRect->s32Ypos;
        rect_dst.u32Width = pstDstRect->u32Width;
        rect_dst.u32Height = pstDstRect->u32Height;
    }
    printf("<%s> : <%d> : start blit,src_rect <%d %d %d %d>,dst_rect <%d %d %d %d>, <%d %d> %llx %llx\n", __FUNCTION__, __LINE__, rect_src.s32Xpos,rect_src.s32Ypos,rect_src.u32Width, rect_src.u32Height, 
         rect_dst.s32Xpos,rect_dst.s32Ypos,rect_dst.u32Width, rect_dst.u32Height, stForeGround.u32Width, stForeGround.u32Height, stForeGround.u32PhyAddr, stForeGround.u32CbCrPhyAddr);

    ret = GpeOsiBlit(s32Handle, NULL, NULL,
            &stForeGround, &rect_src, pstDst, &rect_dst,
            pstOpt);
    printf("<%s> : <%d> api leave(%d>\n", __FUNCTION__, __LINE__, ret);

    return ret;
}
#endif


mt_s32 GpeClipMask(TDE_HANDLE s32Handle,
        TDE2_SURFACE_S *pstForeGround,
        TDE2_SURFACE_S *pstDst,
        TDE2_RECT_S *pstForeGroundRect,
        TDE2_RECT_S *pstDstRect,
        TDE2_MASK_OPT_E enMaskOpt)
{
    mt_s32 ret = MT_SUCCESS;
    aria_param_t param = {0};
    rect_vsb_t dst_rect = {0};
    rect_vsb_t src_rect = {0};

    if(pstDst == NULL || pstDstRect == NULL)
        return MT_ERR_TDE_NULL_PTR;


    if((pstForeGroundRect != NULL) && (enMaskOpt != TDE2_CLEAR_MASK) && (enMaskOpt != TDE2_FILL_MASK))
    {
        if((pstForeGroundRect->u32Width == 0) && (pstForeGroundRect->u32Height == 0) && (pstForeGround != NULL))
        {
            src_rect.x = 0;
            src_rect.y = 0;
            src_rect.w = pstForeGround->u32Width;
            src_rect.h = pstForeGround->u32Height;
        }
        else
        {
            src_rect.x = (mt_u32)(pstForeGroundRect->s32Xpos);
            src_rect.y = (mt_u32)(pstForeGroundRect->s32Ypos);
            src_rect.w = pstForeGroundRect->u32Width;
            src_rect.h = pstForeGroundRect->u32Height;
        }
    }

    if((pstDstRect->u32Width == 0) && (pstDstRect->u32Height == 0))
    {
        dst_rect.x = 0;
        dst_rect.y = 0;
        dst_rect.w = pstDst->u32Width;
        dst_rect.h = pstDst->u32Height;
    }
    else
    {
        dst_rect.x = (mt_u32)(pstDstRect->s32Xpos);
        dst_rect.y = (mt_u32)(pstDstRect->s32Ypos);
        dst_rect.w = pstDstRect->u32Width;
        dst_rect.h = pstDstRect->u32Height;
    }

    param_init(&param);

    // ====== dst image attribute =====  //
    param.dst_img.buf = pstDst->u32PhyAddr;
    param.dst_img.pitch = pstDst->u32Stride;
    param.dst_img.width = pstDst->u32Width;
    param.dst_img.height = pstDst->u32Height;
    param.dst_img.pix_format = aria_gpe_convert_fmt(pstDst->enColorFmt, &param.dst_img.alpha_ch_en);
    param.dst_img.rect.x = dst_rect.x;
    param.dst_img.rect.y = dst_rect.y;
    param.dst_img.rect.w = dst_rect.w;
    param.dst_img.rect.h = dst_rect.h;


    if((pstForeGround != NULL) && (enMaskOpt != TDE2_CLEAR_MASK) && (enMaskOpt != TDE2_FILL_MASK))
    {
        param.src_img_en = MT_TRUE;
        param.src_img.buf = pstForeGround->u32PhyAddr;
        param.src_img.pitch = pstForeGround->u32Stride;
        param.src_img.width = pstForeGround->u32Width;
        param.src_img.height = pstForeGround->u32Height;
        param.src_img.pix_format = aria_gpe_convert_fmt(pstForeGround->enColorFmt, &param.src_img.alpha_ch_en);
        param.src_img.rect.x = src_rect.x;
        param.src_img.rect.y = src_rect.y;
        param.src_img.rect.w = src_rect.w;
        param.src_img.rect.h = src_rect.h;
    }
    param.gpe_op = GPE_OP_MASK;
    param.mask_mod = enMaskOpt;

    ret = aria_gpe_set_parameter(s32Handle, &param);
    if(ret != MT_TRUE)
    {
        aria_gpe_print_context(s32Handle);
        return MT_ERR_TDE_UNSUPPORTED_OPERATION;
    }
    ret = aria_gpe_start(s32Handle);
    return (ret == MT_TRUE ?MT_SUCCESS: MT_FAILURE);
}

#ifdef CONFIG_MT_FPGA_GPE
#if 0 
void gpe_usr_write_reg(unsigned int offset, unsigned int data)
{
    gpe_write_register(offset,data);
}

unsigned int gpe_usr_read_register(unsigned int offset)
{
    return gpe_read_register(offset);
}
#endif
/*
void *pData,virtual addr
void *p_palette,virtual addr
*/
mt_s32 GpeOsiImageDraw(TDE_HANDLE s32Handle, TDE2_SURFACE_S *pstDst, TDE2_RECT_S *pstDstRect,
    void *pData,void *p_palette,mt_s32 pal_entris,mt_s32 s32Stride,mt_u32 in_size,
    mt_s32 image_w,mt_s32 image_h,TDE2_COLOR_FMT_E color_fmt,TDE2_RECT_S *pstSrcRect,TDE2_OPT_S *pstOpt)
{
    mt_s32 ret = 0;
    //TDE2_RECT_S  src_rect;    

    TDE2_SURFACE_S image_surface = {0};
    mt_mmz_buf_s  psMBuf = {0};
    mt_mmz_buf_s  pal_psMBuf = {0};
    
    MT_INFO_TDE("----------------in------------\n");    
    if(pstDst == NULL || pstDstRect == NULL||pData == NULL
        ||pstSrcRect==NULL || pstOpt == NULL)
        return MT_ERR_TDE_NULL_PTR;

    if(pstSrcRect->u32Width == 0 || pstSrcRect->u32Height== 0)
    {
        return MT_ERR_TDE_INVALID_PARA;
    }
    memset(&image_surface,0x00,sizeof(TDE2_SURFACE_S));
    strncpy(psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
    psMBuf.bufsize = in_size;
    ret = mt_mmz_malloc(&psMBuf);
    if(ret != MT_SUCCESS)
    {
        return MT_ERR_TDE_NO_MEM;
    }
    memcpy(psMBuf.user_viraddr,pData,in_size);
    image_surface.u32PhyAddr  = psMBuf.phyaddr;
    image_surface.u32VirAddr = (ulong)psMBuf.user_viraddr;
    image_surface.enColorFmt = color_fmt;
    image_surface.u32Width = image_w;
    image_surface.u32Height= image_h;
    image_surface.u32Stride=s32Stride;
    
    if(p_palette != NULL)
    {
        strncpy(pal_psMBuf.bufname, MOD_NAME,(MAX_BUFFER_NAME_SIZE-1));
        pal_psMBuf.bufsize = pal_entris*4;        
        ret = mt_mmz_malloc(&pal_psMBuf);
        if(ret != MT_SUCCESS)
        {
            mt_mmz_free(&psMBuf);
            return MT_ERR_TDE_NO_MEM;
        }
        memcpy(pal_psMBuf.user_viraddr,p_palette,pal_entris*4);
        image_surface.pu8ClutVirAddr=pal_psMBuf.user_viraddr;
        image_surface.pu8ClutPhyAddr=pal_psMBuf.phyaddr;
    }
    
    
    ret = GpeOsiBlit(s32Handle, NULL, NULL, &image_surface, pstSrcRect,
            pstDst, pstDstRect, pstOpt);

    mt_mmz_free(&psMBuf);
    if(p_palette != NULL)
    {
        mt_mmz_free(&pal_psMBuf);
    }
    return ret;

}

#endif


#ifdef __cplusplus
#if __cplusplus
}
#endif  /* __cplusplus */
#endif  /* __cplusplus */
