/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _IP_DISPLAY_0518_WUKE_H
#define _IP_DISPLAY_0518_WUKE_H

#include"mt_type.h"

#ifdef DISP_ON_AP_LINUX
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#define REG_SYMPHONY_DISP_BASE SYMPHONY_IO_VA(0xbf440000)
#elif defined(VFW_TANGO)
#define REG_SYMPHONY_DISP_BASE 0xbfd20000
#else
#define REG_SYMPHONY_DISP_BASE 0xbf440000
#endif

enum
{
    REG_SYMPHONY_DISP_DISPLAY_CTRL                = (REG_SYMPHONY_DISP_BASE + 0x0000),
    REG_SYMPHONY_DISP_VSCALER_RATIO_HD            = (REG_SYMPHONY_DISP_BASE + 0x0004),
    REG_SYMPHONY_DISP_VSCALER_RATIO_INIT_HD       = (REG_SYMPHONY_DISP_BASE + 0x0008),
    REG_SYMPHONY_DISP_VID_DISP_FIELD              = (REG_SYMPHONY_DISP_BASE + 0x000c),
    REG_SYMPHONY_DISP_VID_WINDOW_CUT_SD           = (REG_SYMPHONY_DISP_BASE + 0x0010),
    REG_SYMPHONY_DISP_VID_WINDOW_X_HD             = (REG_SYMPHONY_DISP_BASE + 0x0014),
    REG_SYMPHONY_DISP_VID_WINDOW_Y_HD             = (REG_SYMPHONY_DISP_BASE + 0x0018),
    REG_SYMPHONY_DISP_VID_WINDOW_CUT_HD           = (REG_SYMPHONY_DISP_BASE + 0x001c),
    REG_SYMPHONY_DISP_VSCALER_RATIO_SD            = (REG_SYMPHONY_DISP_BASE + 0x0020),
    REG_SYMPHONY_DISP_VSCALER_RATIO_INIT_SD       = (REG_SYMPHONY_DISP_BASE + 0x0024),
    REG_SYMPHONY_DISP_VID_WINDOW_X_SD             = (REG_SYMPHONY_DISP_BASE + 0x0028),
    REG_SYMPHONY_DISP_VID_WINDOW_Y_SD             = (REG_SYMPHONY_DISP_BASE + 0x002c),
    REG_SYMPHONY_DISP_GRAPHIC_CTRL                = (REG_SYMPHONY_DISP_BASE + 0x0030),
    REG_SYMPHONY_DISP_BG_COLOR                    = (REG_SYMPHONY_DISP_BASE + 0x0034),
    REG_SYMPHONY_DISP_STILL_X_HD                  = (REG_SYMPHONY_DISP_BASE + 0x0038),
    REG_SYMPHONY_DISP_STILL_Y_HD                  = (REG_SYMPHONY_DISP_BASE + 0x003c),
    REG_SYMPHONY_DISP_OSD0_CMD_HD                 = (REG_SYMPHONY_DISP_BASE + 0x0040),
    REG_SYMPHONY_DISP_SUB_CMD_HD                  = (REG_SYMPHONY_DISP_BASE + 0x0044),
    REG_SYMPHONY_DISP_LAYER_ALPHA                 = (REG_SYMPHONY_DISP_BASE + 0x0048),
    REG_SYMPHONY_DISP_VID_INPUT_SIZE              = (REG_SYMPHONY_DISP_BASE + 0x004c),
    REG_SYMPHONY_DISP_VID_SD_DROP_LINE            = (REG_SYMPHONY_DISP_BASE + 0x0050),
    REG_SYMPHONY_DISP_VID_CROP_MODE_EN            = (REG_SYMPHONY_DISP_BASE + 0x0054),
    REG_SYMPHONY_DISP_VID_CROP_HORI               = (REG_SYMPHONY_DISP_BASE + 0x0058),
    REG_SYMPHONY_DISP_VID_CROP_VERT               = (REG_SYMPHONY_DISP_BASE + 0x005c),
    REG_SYMPHONY_DISP_VID_PROCESS_MODE            = (REG_SYMPHONY_DISP_BASE + 0x0060),
    REG_SYMPHONY_DISP_DATA_ARRANGE_1              = (REG_SYMPHONY_DISP_BASE + 0x0064),
    REG_SYMPHONY_DISP_DATA_ARRANGE_2              = (REG_SYMPHONY_DISP_BASE + 0x0068),
    REG_SYMPHONY_DISP_OSD1_CMD_HD                 = (REG_SYMPHONY_DISP_BASE + 0x006c),
    REG_SYMPHONY_DISP_OSD1_CK_HD                  = (REG_SYMPHONY_DISP_BASE + 0x0070),
    REG_SYMPHONY_DISP_STILL_X_SD                  = (REG_SYMPHONY_DISP_BASE + 0x0074),
    REG_SYMPHONY_DISP_STILL_Y_SD                  = (REG_SYMPHONY_DISP_BASE + 0x0078),
    REG_SYMPHONY_DISP_STILL_STRIDE_SD             = (REG_SYMPHONY_DISP_BASE + 0x007c),
    REG_SYMPHONY_DISP_RGB2Y_COEF                  = (REG_SYMPHONY_DISP_BASE + 0x0080),
    REG_SYMPHONY_DISP_RGB2CB_COEF                 = (REG_SYMPHONY_DISP_BASE + 0x0084),
    REG_SYMPHONY_DISP_RGB2CR_COEF                 = (REG_SYMPHONY_DISP_BASE + 0x0088),
    REG_SYMPHONY_DISP_VID_DECOMP_CFG              = (REG_SYMPHONY_DISP_BASE + 0x008C),
    REG_SYMPHONY_DISP_OSD0_CK_HD                  = (REG_SYMPHONY_DISP_BASE + 0x0090),
    REG_SYMPHONY_DISP_RGB2YUV_YOFFSET             = (REG_SYMPHONY_DISP_BASE + 0x0094),
    REG_SYMPHONY_DISP_RGB2YUV_UVOFFSET            = (REG_SYMPHONY_DISP_BASE + 0x0098),
    REG_SYMPHONY_DISP_VID_VERF_CFG                = (REG_SYMPHONY_DISP_BASE + 0x00A8),
    REG_SYMPHONY_DISP_VID_HORF_CFG                = (REG_SYMPHONY_DISP_BASE + 0x00AC),
    REG_SYMPHONY_DISP_LAYER_MIX_CFG               = (REG_SYMPHONY_DISP_BASE + 0x00B0),
    REG_SYMPHONY_DISP_HD_SIZE_OUT                 = (REG_SYMPHONY_DISP_BASE + 0x00B8),
    REG_SYMPHONY_DISP_SD_SIZE_OUT                 = (REG_SYMPHONY_DISP_BASE + 0x00BC),
    REG_SYMPHONY_DISP_STILL_STRIDE_HD             = (REG_SYMPHONY_DISP_BASE + 0x00C0),
    REG_SYMPHONY_DISP_HDTV_CFG                    = (REG_SYMPHONY_DISP_BASE + 0x00C4),
    REG_SYMPHONY_DISP_HD_POST_CFG                 = (REG_SYMPHONY_DISP_BASE + 0x00CC),
    REG_SYMPHONY_DISP_SD_POST_CFG                 = (REG_SYMPHONY_DISP_BASE + 0x00D0),
    REG_SYMPHONY_DISP_HD_EFFECT_COEF              = (REG_SYMPHONY_DISP_BASE + 0x00D4),
    REG_SYMPHONY_DISP_SD_EFFECT_COEF              = (REG_SYMPHONY_DISP_BASE + 0x00D8),
    REG_SYMPHONY_DISP_GRA_FIFO_THRESHOLD          = (REG_SYMPHONY_DISP_BASE + 0x00F4),
    REG_SYMPHONY_DISP_GRA_SCALER_CTRL             = (REG_SYMPHONY_DISP_BASE + 0x00F8),
    REG_SYMPHONY_DISP_GRA_SCALER_HRATIO           = (REG_SYMPHONY_DISP_BASE + 0x00FC),
    REG_SYMPHONY_DISP_GRA_SCALER_VRATIO           = (REG_SYMPHONY_DISP_BASE + 0x0100),
    REG_SYMPHONY_DISP_GRA_SCALER_H_START_FRA      = (REG_SYMPHONY_DISP_BASE + 0x0104),
    REG_SYMPHONY_DISP_GRA_SCALER_V_START_FRA      = (REG_SYMPHONY_DISP_BASE + 0x0108),
    REG_SYMPHONY_DISP_GRA_CTL                     = (REG_SYMPHONY_DISP_BASE + 0x010C),
    REG_SYMPHONY_DISP_OSD_SCALE_HSIZE             = (REG_SYMPHONY_DISP_BASE + 0x0110),
    REG_SYMPHONY_DISP_OSD_SCALE_RATIO             = (REG_SYMPHONY_DISP_BASE + 0x0114),
    REG_SYMPHONY_DISP_OSD_ALPHA                   = (REG_SYMPHONY_DISP_BASE + 0x0118),
    REG_SYMPHONY_DISP_OSD_VERT_START_LINE         = (REG_SYMPHONY_DISP_BASE + 0x0120),
    REG_SYMPHONY_DISP_OSD_VERTICAL_CTRL           = (REG_SYMPHONY_DISP_BASE + 0x0124),
    REG_SYMPHONY_DISP_OSD_VERTICAL_SIZE           = (REG_SYMPHONY_DISP_BASE + 0x0128),
    REG_SYMPHONY_DISP_OSD_VERTICAL_RATIO          = (REG_SYMPHONY_DISP_BASE + 0x012C),
    REG_SYMPHONY_DISP_OSD_V_START_FRA             = (REG_SYMPHONY_DISP_BASE + 0x0130),
    REG_SYMPHONY_DISP_OSD_V_TAP_NUM               = (REG_SYMPHONY_DISP_BASE + 0x0134),
    REG_SYMPHONY_DISP_CHROMA_COEF0                = (REG_SYMPHONY_DISP_BASE + 0x0150),
    REG_SYMPHONY_DISP_CHROMA_COEF1                = (REG_SYMPHONY_DISP_BASE + 0x0154),
    REG_SYMPHONY_DISP_CHROMA_COEF2                = (REG_SYMPHONY_DISP_BASE + 0x0158),
    REG_SYMPHONY_DISP_CHROMA_COEF3                = (REG_SYMPHONY_DISP_BASE + 0x015C),
    REG_SYMPHONY_DISP_SCALE_INIT_PHASE_OFFSET     = (REG_SYMPHONY_DISP_BASE + 0x0160),
    REG_SYMPHONY_DISP_SMALL_PIC_UPSCALE_CTRL      = (REG_SYMPHONY_DISP_BASE + 0x0164),
    REG_SYMPHONY_DISP_ALISING_PROB_REG1           = (REG_SYMPHONY_DISP_BASE + 0x0168),
    REG_SYMPHONY_DISP_ALISING_PROB_REG2           = (REG_SYMPHONY_DISP_BASE + 0x016C),
    REG_SYMPHONY_DISP_ALISING_PROB_REG3           = (REG_SYMPHONY_DISP_BASE + 0x0170),
    REG_SYMPHONY_DISP_ALISING_PROB_REG4           = (REG_SYMPHONY_DISP_BASE + 0x0174),
    REG_SYMPHONY_DISP_CSC_CTRL                    = (REG_SYMPHONY_DISP_BASE + 0x0178),
    REG_SYMPHONY_DISP_CSC_HD_COEF1                = (REG_SYMPHONY_DISP_BASE + 0x017c),
    REG_SYMPHONY_DISP_CSC_HD_COEF2                = (REG_SYMPHONY_DISP_BASE + 0x0180),
    REG_SYMPHONY_DISP_CSC_HD_COEF3                = (REG_SYMPHONY_DISP_BASE + 0x0184),
    REG_SYMPHONY_DISP_CSC_HD_COEF4                = (REG_SYMPHONY_DISP_BASE + 0x0188),
    REG_SYMPHONY_DISP_CSC_HD_COEF5                = (REG_SYMPHONY_DISP_BASE + 0x018c),
    REG_SYMPHONY_DISP_CSC_SD_COEF1                = (REG_SYMPHONY_DISP_BASE + 0x0190),
    REG_SYMPHONY_DISP_CSC_SD_COEF2                = (REG_SYMPHONY_DISP_BASE + 0x0194),
    REG_SYMPHONY_DISP_CSC_SD_COEF3                = (REG_SYMPHONY_DISP_BASE + 0x0198),
    REG_SYMPHONY_DISP_CSC_SD_COEF4                = (REG_SYMPHONY_DISP_BASE + 0x019c),
    REG_SYMPHONY_DISP_CSC_SD_COEF5                = (REG_SYMPHONY_DISP_BASE + 0x01a0),
    REG_SYMPHONY_DISP_CSC_STILL_CTRL              = (REG_SYMPHONY_DISP_BASE + 0x01a8),
    REG_SYMPHONY_DISP_CSC_STILL_COEF1             = (REG_SYMPHONY_DISP_BASE + 0x01ac),
    REG_SYMPHONY_DISP_CSC_STILL_COEF2             = (REG_SYMPHONY_DISP_BASE + 0x01b0),
    REG_SYMPHONY_DISP_CSC_STILL_COEF3             = (REG_SYMPHONY_DISP_BASE + 0x01b4),
    REG_SYMPHONY_DISP_CSC_STILL_COEF4             = (REG_SYMPHONY_DISP_BASE + 0x01b8),
    REG_SYMPHONY_DISP_CSC_STILL_COEF5             = (REG_SYMPHONY_DISP_BASE + 0x01bc),
    REG_SYMPHONY_DISP_ROW_JUMP_00                 = (REG_SYMPHONY_DISP_BASE + 0x01c0),
    REG_SYMPHONY_DISP_ROW_JUMP_01                 = (REG_SYMPHONY_DISP_BASE + 0x01c4),
    REG_SYMPHONY_DISP_ROW_JUMP_10                 = (REG_SYMPHONY_DISP_BASE + 0x01c8),
    REG_SYMPHONY_DISP_ROW_JUMP_11                 = (REG_SYMPHONY_DISP_BASE + 0x01cc),
    REG_SYMPHONY_DISP_DENOISE_DOMAIN_1            = (REG_SYMPHONY_DISP_BASE + 0x01d0),
    REG_SYMPHONY_DISP_DENOISE_DOMAIN_2            = (REG_SYMPHONY_DISP_BASE + 0x01d4),
    REG_SYMPHONY_DISP_DENOISE_DOMAIN_3            = (REG_SYMPHONY_DISP_BASE + 0x01d8),
    REG_SYMPHONY_DISP_DENOISE_RANGE_1             = (REG_SYMPHONY_DISP_BASE + 0x01dc),
    REG_SYMPHONY_DISP_DENOISE_RANGE_2             = (REG_SYMPHONY_DISP_BASE + 0x01e0),
    REG_SYMPHONY_DISP_DENOISE_RANGE_3             = (REG_SYMPHONY_DISP_BASE + 0x01e4),
    REG_SYMPHONY_DISP_DENOISE_RANGE_4             = (REG_SYMPHONY_DISP_BASE + 0x01e8),
    REG_SYMPHONY_DISP_DENOISE_RANGE_5             = (REG_SYMPHONY_DISP_BASE + 0x01ec),
    REG_SYMPHONY_DISP_DENOISE_RANGE_6             = (REG_SYMPHONY_DISP_BASE + 0x01f0),
    REG_SYMPHONY_DISP_COLOR_ENHANCE_CTRL          = (REG_SYMPHONY_DISP_BASE + 0x01f4),
    REG_SYMPHONY_DISP_VERSION_ID                  = (REG_SYMPHONY_DISP_BASE + 0x01fc),
    REG_SYMPHONY_DISP_SD_WRBACK_ADDR_ODD          = (REG_SYMPHONY_DISP_BASE + 0x1000),
    REG_SYMPHONY_DISP_SD_BASE_ADDR_EVEN           = (REG_SYMPHONY_DISP_BASE + 0x1004),
    REG_SYMPHONY_DISP_SD_WRBACK_CTRL              = (REG_SYMPHONY_DISP_BASE + 0x1008),
    REG_SYMPHONY_DISP_SD_BACK_COLOR               = (REG_SYMPHONY_DISP_BASE + 0x1014),
    REG_SYMPHONY_DISP_STILL_UV_START_ADDR_HD      = (REG_SYMPHONY_DISP_BASE + 0x101C),
    REG_SYMPHONY_DISP_STILL_Y_START_ADDR_HD       = (REG_SYMPHONY_DISP_BASE + 0x1020),
    REG_SYMPHONY_DISP_SUB_START_ADDR_HD           = (REG_SYMPHONY_DISP_BASE + 0x1024),
    REG_SYMPHONY_DISP_OSD1_START_ADDR_HD          = (REG_SYMPHONY_DISP_BASE + 0x1028),
    REG_SYMPHONY_DISP_STILL_START_ADDR_SD         = (REG_SYMPHONY_DISP_BASE + 0x102C),
    REG_SYMPHONY_DISP_OSD0_START_ADDR_HD          = (REG_SYMPHONY_DISP_BASE + 0x1030),
    REG_SYMPHONY_DISP_VIDEO_DISPLAY_INFO          = (REG_SYMPHONY_DISP_BASE + 0x1034),
    REG_SYMPHONY_DISP_MOTION_PRE_ADDR             = (REG_SYMPHONY_DISP_BASE + 0x1040),
    REG_SYMPHONY_DISP_MOTION_CUR_ADDR             = (REG_SYMPHONY_DISP_BASE + 0x1044),
    REG_SYMPHONY_DISP_LUMA_PRE_ADDR               = (REG_SYMPHONY_DISP_BASE + 0x1048),
    REG_SYMPHONY_DISP_LUMA_CUR_ADDR               = (REG_SYMPHONY_DISP_BASE + 0x104C),
    REG_SYMPHONY_DISP_LUMA_NEXT_ADDR              = (REG_SYMPHONY_DISP_BASE + 0x1050),
    REG_SYMPHONY_DISP_CHROMA_PPRE_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x1054),
    REG_SYMPHONY_DISP_CHROMA_PRE_ADDR             = (REG_SYMPHONY_DISP_BASE + 0x1058),
    REG_SYMPHONY_DISP_CHROMA_CUR_ADDR             = (REG_SYMPHONY_DISP_BASE + 0x105C),
    REG_SYMPHONY_DISP_CHROMA_NEXT_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x1060),
    REG_SYMPHONY_DISP_MOTION_PRE_ADDR2            = (REG_SYMPHONY_DISP_BASE + 0x1064),
    REG_SYMPHONY_DISP_MOTION_CUR_ADDR2            = (REG_SYMPHONY_DISP_BASE + 0x1068),
    REG_SYMPHONY_DISP_LUMA_PRE_ADDR2              = (REG_SYMPHONY_DISP_BASE + 0x106C),
    REG_SYMPHONY_DISP_LUMA_CUR_ADDR2              = (REG_SYMPHONY_DISP_BASE + 0x1070),
    REG_SYMPHONY_DISP_LUMA_NEXT_ADDR2             = (REG_SYMPHONY_DISP_BASE + 0x1074),
    REG_SYMPHONY_DISP_CHROMA_PPRE_ADDR2           = (REG_SYMPHONY_DISP_BASE + 0x1078),
    REG_SYMPHONY_DISP_CHROMA_PRE_ADDR2            = (REG_SYMPHONY_DISP_BASE + 0x107C),
    REG_SYMPHONY_DISP_CHROMA_CUR_ADDR2            = (REG_SYMPHONY_DISP_BASE + 0x1080),
    REG_SYMPHONY_DISP_CHROMA_NEXT_ADDR2           = (REG_SYMPHONY_DISP_BASE + 0x1084),
    REG_SYMPHONY_DISP_FIELD_PIC_FMT               = (REG_SYMPHONY_DISP_BASE + 0x108C),
    REG_SYMPHONY_DISP_VID_HD_VF_COEF_ADDR         = (REG_SYMPHONY_DISP_BASE + 0x1098),
    REG_SYMPHONY_DISP_VID_HD_HF_COEF_ADDR         = (REG_SYMPHONY_DISP_BASE + 0x109C),
    REG_SYMPHONY_DISP_VID_SD_VF_COEF_ADDR         = (REG_SYMPHONY_DISP_BASE + 0x10A0),
    REG_SYMPHONY_DISP_VID_SD_HF_COEF_ADDR         = (REG_SYMPHONY_DISP_BASE + 0x10A4),
    REG_SYMPHONY_DISP_GRA_VF_COEF_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x10A8),
    REG_SYMPHONY_DISP_GRA_HF_COEF_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x10AC),
    REG_SYMPHONY_DISP_VID_DCE_MAP_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x10B0),
    REG_SYMPHONY_DISP_VSCALER_TABLE_SEL           = (REG_SYMPHONY_DISP_BASE + 0x10B4),
    REG_SYMPHONY_DISP_OSD_VF_COEF_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x10C0),
    REG_SYMPHONY_DISP_OSD_HF_COEF_ADDR            = (REG_SYMPHONY_DISP_BASE + 0x10C4),
    REG_SYMPHONY_DISP_CHROMA_HD_HF_COEF_ADDR      = (REG_SYMPHONY_DISP_BASE + 0x10C8),
    REG_SYMPHONY_DISP_CHROMA_SD_HF_COEF_ADDR      = (REG_SYMPHONY_DISP_BASE + 0x10CC),
    REG_SYMPHONY_DISP_LUMA_BOT_CUR_ADDR           = (REG_SYMPHONY_DISP_BASE + 0x10D0),
    REG_SYMPHONY_DISP_CHROMA_BOT_CUR_ADDR         = (REG_SYMPHONY_DISP_BASE + 0x10D4),
    REG_SYMPHONY_DISP_LUMA_BOT_CUR_ADDR2          = (REG_SYMPHONY_DISP_BASE + 0x10D8),
    REG_SYMPHONY_DISP_CHROMA_BOT_CUR_ADDR2        = (REG_SYMPHONY_DISP_BASE + 0x10DC),
    REG_SYMPHONY_DISP_NLMEANS_DENOISE_CTRL        = (REG_SYMPHONY_DISP_BASE + 0x10E0),
    REG_SYMPHONY_DISP_NLMEANS_PARAMETER_1         = (REG_SYMPHONY_DISP_BASE + 0x10E4),
    REG_SYMPHONY_DISP_NLMEANS_PARAMETER_2         = (REG_SYMPHONY_DISP_BASE + 0x10E8),
    REG_SYMPHONY_DISP_NLMEANS_PARAMETER_3         = (REG_SYMPHONY_DISP_BASE + 0x10EC),
    REG_SYMPHONY_DISP_OUTPUT_DATA_SEL             = (REG_SYMPHONY_DISP_BASE + 0x10F0),
    REG_SYMPHONY_DISP_HDMI_DEFAULT_DATA           = (REG_SYMPHONY_DISP_BASE + 0x10F4),
    REG_SYMPHONY_DISP_HD2SD_DEFAULT_DATA          = (REG_SYMPHONY_DISP_BASE + 0x10F8),
    REG_SYMPHONY_DISP_OSDM_CSC_CTRL               = (REG_SYMPHONY_DISP_BASE + 0x11c0),
    REG_SYMPHONY_DISP_OSDM_CSC_COEF01             = (REG_SYMPHONY_DISP_BASE + 0x11c4),
    REG_SYMPHONY_DISP_OSDM_CSC_COEF23             = (REG_SYMPHONY_DISP_BASE + 0x11c8),
    REG_SYMPHONY_DISP_OSDM_CSC_COEF45             = (REG_SYMPHONY_DISP_BASE + 0x11cc),
    REG_SYMPHONY_DISP_OSDM_CSC_COEF67             = (REG_SYMPHONY_DISP_BASE + 0x11d0),
    REG_SYMPHONY_DISP_OSDM_CSC_COEF8              = (REG_SYMPHONY_DISP_BASE + 0x11d4),
    REG_SYMPHONY_DISP_SUB_CSC_CTRL                = (REG_SYMPHONY_DISP_BASE + 0x11d8),
    REG_SYMPHONY_DISP_SUB_CSC_COEF01              = (REG_SYMPHONY_DISP_BASE + 0x11dc),
    REG_SYMPHONY_DISP_SUB_CSC_COEF23              = (REG_SYMPHONY_DISP_BASE + 0x11e0),
    REG_SYMPHONY_DISP_SUB_CSC_COEF45              = (REG_SYMPHONY_DISP_BASE + 0x11e4),
    REG_SYMPHONY_DISP_SUB_CSC_COEF67              = (REG_SYMPHONY_DISP_BASE + 0x11e8),
    REG_SYMPHONY_DISP_SUB_CSC_COEF8               = (REG_SYMPHONY_DISP_BASE + 0x11ec),
    REG_SYMPHONY_DISP_TILE_COL_SIZE               = (REG_SYMPHONY_DISP_BASE + 0x11fc),
    REG_SYMPHONY_DISP_DI_ENABLE                   = (REG_SYMPHONY_DISP_BASE + 0x2000),
    REG_SYMPHONY_DISP_VIDEO_PDD_EN                = (REG_SYMPHONY_DISP_BASE + 0x2004),
    REG_SYMPHONY_DISP_VIDEO_IS_MOVIE_TYPE         = (REG_SYMPHONY_DISP_BASE + 0x2008),
    REG_SYMPHONY_DISP_DI_P_OR_N_PAIRED            = (REG_SYMPHONY_DISP_BASE + 0x200C),
    REG_SYMPHONY_DISP_DI_OPER_MODE                = (REG_SYMPHONY_DISP_BASE + 0x2010),
    REG_SYMPHONY_DISP_DI_PARA                     = (REG_SYMPHONY_DISP_BASE + 0x2014),
    REG_SYMPHONY_DISP_DI_DATA_SFIFO_THR           = (REG_SYMPHONY_DISP_BASE + 0x2018),
    REG_SYMPHONY_DISP_VIDEO_SCALER_DATA_SFIFO_THR = (REG_SYMPHONY_DISP_BASE + 0x201C),
    REG_SYMPHONY_DISP_DI_LOUT_AFIFO_THR           = (REG_SYMPHONY_DISP_BASE + 0x2020),
    REG_SYMPHONY_DISP_VSCALER_AXI_CMD_SFIFO_THR   = (REG_SYMPHONY_DISP_BASE + 0x2024),
    REG_SYMPHONY_DISP_VSCALER_AXI_REQ_SFIFO_THR   = (REG_SYMPHONY_DISP_BASE + 0x2028),
    REG_SYMPHONY_DISP_DI_PDD_NOISE_THR            = (REG_SYMPHONY_DISP_BASE + 0x202C),
    REG_SYMPHONY_DISP_DI_ACC_RESULT_ODD           = (REG_SYMPHONY_DISP_BASE + 0x2030),
    REG_SYMPHONY_DISP_DI_ACC_RESULT_EVEN          = (REG_SYMPHONY_DISP_BASE + 0x2034),
    REG_SYMPHONY_DISP_DI_PAUSE_EN                 = (REG_SYMPHONY_DISP_BASE + 0x203C),
    REG_SYMPHONY_DISP_DI_ALPHA_PARA               = (REG_SYMPHONY_DISP_BASE + 0x2040),
    REG_SYMPHONY_DISP_DI_MOTION_CTRL_1            = (REG_SYMPHONY_DISP_BASE + 0x2044),
    REG_SYMPHONY_DISP_DI_MOTION_CTRL_2            = (REG_SYMPHONY_DISP_BASE + 0x2048),
    REG_SYMPHONY_DISP_DI_HEVC_FLAG                = (REG_SYMPHONY_DISP_BASE + 0x204C),
    REG_SYMPHONY_DISP_NONE_DI_FIELDS_FLAG         = (REG_SYMPHONY_DISP_BASE + 0x2050),
    REG_SYMPHONY_DISP_NONE_DI_HEVC_FLAG           = (REG_SYMPHONY_DISP_BASE + 0x2054),
    REG_SYMPHONY_DISP_NONE_DI_PROGRESSIVE_FLAG    = (REG_SYMPHONY_DISP_BASE + 0x2058),
    REG_SYMPHONY_DISP_DI_MOTION_CTRL_3            = (REG_SYMPHONY_DISP_BASE + 0x205C),
    REG_SYMPHONY_DISP_DI_MOTION_CTRL_4            = (REG_SYMPHONY_DISP_BASE + 0x2060),
    REG_SYMPHONY_DISP_DI_CHROMA_PARA              = (REG_SYMPHONY_DISP_BASE + 0x2064),
    REG_SYMPHONY_DISP_CLOCK_GATE_CTRL              = (REG_SYMPHONY_DISP_BASE + 0x2080),
    REG_SYMPHONY_DISP_VIDEO_BURST_LENGTH_SEL      = (REG_SYMPHONY_DISP_BASE + 0x3000),
    REG_SYMPHONY_DISP_VIDEO_BURST_INFO            = (REG_SYMPHONY_DISP_BASE + 0x3004),
    REG_SYMPHONY_DISP_VIDEO_LINE_RD_CNT_MAX       = (REG_SYMPHONY_DISP_BASE + 0x3008),
    REG_SYMPHONY_DISP_PRESCALE_CMD                = (REG_SYMPHONY_DISP_BASE + 0x3030),
    REG_SYMPHONY_DISP_PRESCALE_FRAME_SIZE         = (REG_SYMPHONY_DISP_BASE + 0x3034),
    REG_SYMPHONY_DISP_PRESCALE_TILING_PARAMS      = (REG_SYMPHONY_DISP_BASE + 0x3038),
    REG_SYMPHONY_DISP_PRESCALE_LUMA_RD_ADDR       = (REG_SYMPHONY_DISP_BASE + 0x303c),
    REG_SYMPHONY_DISP_PRESCALE_LUMA_RD_ADDR2      = (REG_SYMPHONY_DISP_BASE + 0x3040),
    REG_SYMPHONY_DISP_PRESCALE_LUMA_WR_ADDR       = (REG_SYMPHONY_DISP_BASE + 0x3044),
    REG_SYMPHONY_DISP_PRESCALE_LUMA_WR_ADDR2      = (REG_SYMPHONY_DISP_BASE + 0x3048),
    REG_SYMPHONY_DISP_PRESCALE_CHROMA_RD_ADDR     = (REG_SYMPHONY_DISP_BASE + 0x304c),
    REG_SYMPHONY_DISP_PRESCALE_CHROMA_RD_ADDR2    = (REG_SYMPHONY_DISP_BASE + 0x3050),
    REG_SYMPHONY_DISP_PRESCALE_CHROMA_WR_ADDR     = (REG_SYMPHONY_DISP_BASE + 0x3054),
    REG_SYMPHONY_DISP_PRESCALE_CHROMA_WR_ADDR2    = (REG_SYMPHONY_DISP_BASE + 0x3058),
    REG_SYMPHONY_DISP_PRESCALE_FIFO_THR           = (REG_SYMPHONY_DISP_BASE + 0x305c),
    REG_SYMPHONY_DISP_HD_HUE_ADJUST               = (REG_SYMPHONY_DISP_BASE + 0x3068),
    REG_SYMPHONY_DISP_SD_HUE_ADJUST               = (REG_SYMPHONY_DISP_BASE + 0x306c),
    REG_SYMPHONY_DISP_HALF_SCALE_CTR              = (REG_SYMPHONY_DISP_BASE + 0x3070),
    REG_SYMPHONY_DISP_PIX_ALIGN_CTR               = (REG_SYMPHONY_DISP_BASE + 0x3074),
    REG_SYMPHONY_DISP_DISP_PROT_LIMIT             = (REG_SYMPHONY_DISP_BASE + 0x3088),

#ifdef CONFIG_MT_CHIP_SYMPHONY2
    REG_SYMPHONY_DISP_NEW_CSC_COEF_TABLE_1_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x30b8),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_TABLE_2_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x30bc),
#else
    REG_SYMPHONY_DISP_SD_NEW_CSC_LITE_RGB_OUT_ADDR    = (REG_SYMPHONY_DISP_BASE + 0x30b8),
    REG_SYMPHONY_DISP_SD_NEW_CSC_LITE_RGB_IN_ADDR     = (REG_SYMPHONY_DISP_BASE + 0x30bc),
#endif

    REG_SYMPHONY_DISP_NEW_CSC_CTRL                    = (REG_SYMPHONY_DISP_BASE + 0x30c0),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_1                  = (REG_SYMPHONY_DISP_BASE + 0x30c4),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_2                  = (REG_SYMPHONY_DISP_BASE + 0x30c8),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_3                  = (REG_SYMPHONY_DISP_BASE + 0x30cc),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_4                  = (REG_SYMPHONY_DISP_BASE + 0x30d0),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_5                  = (REG_SYMPHONY_DISP_BASE + 0x30d4),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_6                  = (REG_SYMPHONY_DISP_BASE + 0x30d8),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_7                  = (REG_SYMPHONY_DISP_BASE + 0x30dc),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_8                  = (REG_SYMPHONY_DISP_BASE + 0x30e0),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_9                  = (REG_SYMPHONY_DISP_BASE + 0x30e4),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_10                 = (REG_SYMPHONY_DISP_BASE + 0x30e8),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_11                 = (REG_SYMPHONY_DISP_BASE + 0x30ec),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_12                 = (REG_SYMPHONY_DISP_BASE + 0x30f0),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_13                 = (REG_SYMPHONY_DISP_BASE + 0x30f4),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_14                 = (REG_SYMPHONY_DISP_BASE + 0x30f8),

    REG_SYMPHONY_DISP_DECOMP_CTRL_0_REG           = (REG_SYMPHONY_DISP_BASE + 0x3100),
    REG_SYMPHONY_DISP_DECOMP_1BGS_STR_REG         = (REG_SYMPHONY_DISP_BASE + 0x3104),
    REG_SYMPHONY_DISP_DECOMP_LTOP_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3108),
    REG_SYMPHONY_DISP_DECOMP_LBOT_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x310c),
    REG_SYMPHONY_DISP_DECOMP_LPRE_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3110),
    REG_SYMPHONY_DISP_DECOMP_LNXT_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3114),
    REG_SYMPHONY_DISP_DECOMP_CTOP_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3118),
    REG_SYMPHONY_DISP_DECOMP_CBOT_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x311c),
    REG_SYMPHONY_DISP_DECOMP_CPPR_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3120),
    REG_SYMPHONY_DISP_DECOMP_CPRE_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3124),
    REG_SYMPHONY_DISP_DECOMP_CNXT_LUT_BASE_REG    = (REG_SYMPHONY_DISP_BASE + 0x3128),
    REG_SYMPHONY_DISP_DECOMP_LTOP_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x312c),
    REG_SYMPHONY_DISP_DECOMP_LBOT_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x3130),
    REG_SYMPHONY_DISP_DECOMP_LPRE_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x3134),
    REG_SYMPHONY_DISP_DECOMP_LNXT_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x3138),
    REG_SYMPHONY_DISP_DECOMP_CTOP_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x313c),
    REG_SYMPHONY_DISP_DECOMP_CBOT_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x3140),
    REG_SYMPHONY_DISP_DECOMP_CPPR_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x3144),
    REG_SYMPHONY_DISP_DECOMP_CPRE_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x3148),
    REG_SYMPHONY_DISP_DECOMP_CNXT_LUT_BASE_2_REG  = (REG_SYMPHONY_DISP_BASE + 0x314c),
    REG_SYMPHONY_DISP_DECOMP_PIC_RESOLUTION_REG   = (REG_SYMPHONY_DISP_BASE + 0x3150),
#ifdef CONFIG_MT_CHIP_SYMPHONY2
    REG_SYMPHONY_DISP_NEW_CSC_COEF_TABLE_3_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x31c0),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_TABLE_4_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x31c4),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_TABLE_5_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x31c8),
    REG_SYMPHONY_DISP_NEW_CSC_COEF_TABLE_6_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x31cc),
#else
    REG_SYMPHONY_DISP_NEW_CSC_LITE_GAIN_ADDR          = (REG_SYMPHONY_DISP_BASE + 0x31c0),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_LITE_GAIN_ADDR      = (REG_SYMPHONY_DISP_BASE + 0x31c4),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_LITE_RGB_OUT_ADDR   = (REG_SYMPHONY_DISP_BASE + 0x31c8),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_LITE_RGB_IN_ADDR    = (REG_SYMPHONY_DISP_BASE + 0x31cc),
#endif

    REG_SYMPHONY_DISP_NEW_CSC_MATRIX_OFFSET_R         = (REG_SYMPHONY_DISP_BASE + 0x31e8),
    REG_SYMPHONY_DISP_NEW_CSC_MATRIX_OFFSET_G         = (REG_SYMPHONY_DISP_BASE + 0x31ec),
    REG_SYMPHONY_DISP_NEW_CSC_MATRIX_OFFSET_B         = (REG_SYMPHONY_DISP_BASE + 0x31f0),
    REG_SYMPHONY_DISP_NEW_CSC_GAIN_CTRL               = (REG_SYMPHONY_DISP_BASE + 0x31f4),

    REG_SYMPHONY_DISP_ADV_CSC_CTRL                    = (REG_SYMPHONY_DISP_BASE + 0x3280),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_1                  = (REG_SYMPHONY_DISP_BASE + 0x3284),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_2                  = (REG_SYMPHONY_DISP_BASE + 0x3288),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_3                  = (REG_SYMPHONY_DISP_BASE + 0x328c),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_4                  = (REG_SYMPHONY_DISP_BASE + 0x3290),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_5                  = (REG_SYMPHONY_DISP_BASE + 0x3294),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_6                  = (REG_SYMPHONY_DISP_BASE + 0x3298),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_7                  = (REG_SYMPHONY_DISP_BASE + 0x329c),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_8                  = (REG_SYMPHONY_DISP_BASE + 0x32a0),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_9                  = (REG_SYMPHONY_DISP_BASE + 0x32a4),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_10                 = (REG_SYMPHONY_DISP_BASE + 0x32a8),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_11                 = (REG_SYMPHONY_DISP_BASE + 0x32ac),
    REG_SYMPHONY_DISP_ADV_CSC_COEF_12                 = (REG_SYMPHONY_DISP_BASE + 0x32b0),
    REG_SYMPHONY_DISP_ADV_CSC_LUT_TABLE_ADDR          = (REG_SYMPHONY_DISP_BASE + 0x32bc),

    REG_SYMPHONY_DISP_OSD_NEW_CSC_CTRL                = (REG_SYMPHONY_DISP_BASE + 0x32c0),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_1              = (REG_SYMPHONY_DISP_BASE + 0x32c4),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_2              = (REG_SYMPHONY_DISP_BASE + 0x32c8),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_3              = (REG_SYMPHONY_DISP_BASE + 0x32cc),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_4              = (REG_SYMPHONY_DISP_BASE + 0x32d0),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_5              = (REG_SYMPHONY_DISP_BASE + 0x32d4),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_6              = (REG_SYMPHONY_DISP_BASE + 0x32d8),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_7              = (REG_SYMPHONY_DISP_BASE + 0x32dc),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_8              = (REG_SYMPHONY_DISP_BASE + 0x32e0),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_9              = (REG_SYMPHONY_DISP_BASE + 0x32e4),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_10             = (REG_SYMPHONY_DISP_BASE + 0x32e8),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_11             = (REG_SYMPHONY_DISP_BASE + 0x32ec),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_12             = (REG_SYMPHONY_DISP_BASE + 0x32f0),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_13             = (REG_SYMPHONY_DISP_BASE + 0x32f4),
    REG_SYMPHONY_DISP_OSD_NEW_CSC_COEF_14             = (REG_SYMPHONY_DISP_BASE + 0x32f8),

    REG_SYMPHONY_DISP_OSD_NEW_CSC_GAIN_CTRL           = (REG_SYMPHONY_DISP_BASE + 0x3328),
};

/*!
  the union of register reg_symphony_disp_display_ctrl
  */
typedef union reg_symphony_disp_display_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_hf_sel                   : 1;
        MT_U32 sd_hf_sel                   : 1;
        MT_U32 hd_cut_en                   : 1;
        MT_U32 sd_cut_en                   : 1;
        MT_U32 hd_vf_sel                   : 1;
        MT_U32 sd_vf_sel                   : 1;
        MT_U32                             : 6;
        MT_U32 disp_fw_mode                : 2;
        MT_U32                             : 6;
        MT_U32 vf_in_frame                 : 1;
        MT_U32                             : 3;
        MT_U32 vid_sel                     : 1;
        MT_U32                             : 2;
        MT_U32 reg_latch_en                : 1;
        MT_U32 reg_latch_timing            : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_display_ctrl_t;

/*!
  the union of register reg_symphony_disp_vscaler_ratio_hd
  */
typedef union reg_symphony_disp_vscaler_ratio_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 hratio_int_hd               : 4;
        MT_U32 hratio_fra_hd               : 12;
        MT_U32 vratio_int_hd               : 4;
        MT_U32 vratio_fra_hd               : 12;
    } bitc;
} reg_symphony_disp_vscaler_ratio_hd_t;

/*!
  the union of register reg_symphony_disp_vscaler_ratio_init_hd
  */
typedef union reg_symphony_disp_vscaler_ratio_init_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 top_int_hd                  : 4;
        MT_U32 top_fra_hd                  : 12;
        MT_U32 bot_int_hd                  : 4;
        MT_U32 bot_fra_hd                  : 12;
    } bitc;
} reg_symphony_disp_vscaler_ratio_init_hd_t;

/*!
  the union of register reg_symphony_disp_vid_disp_field
  */
typedef union reg_symphony_disp_vid_disp_field
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_hf_phase                 : 12;
        MT_U32 hd_hf_tapnum                : 3;
        MT_U32                             : 1;
        MT_U32 hd_hf_phase                 : 12;
        MT_U32 vid_field                   : 2;
        MT_U32                             : 2;
    } bitc;
} reg_symphony_disp_vid_disp_field_t;

/*!
  the union of register reg_symphony_disp_vid_window_cut_sd
  */
typedef union reg_symphony_disp_vid_window_cut_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 left_cut                    : 8;
        MT_U32 right_cut                   : 8;
        MT_U32 top_cut                     : 8;
        MT_U32 bot_cut                     : 8;
    } bitc;
} reg_symphony_disp_vid_window_cut_sd_t;

/*!
  the union of register reg_symphony_disp_vid_window_x_hd
  */
typedef union reg_symphony_disp_vid_window_x_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_x_left_hd               : 11;
        MT_U32                             : 5;
        MT_U32 vid_x_right_hd              : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_window_x_hd_t;

/*!
  the union of register reg_symphony_disp_vid_window_y_hd
  */
typedef union reg_symphony_disp_vid_window_y_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_y_start_hd              : 11;
        MT_U32                             : 5;
        MT_U32 vid_y_end_hd                : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_window_y_hd_t;

/*!
  the union of register reg_symphony_disp_vid_window_cut_hd
  */
typedef union reg_symphony_disp_vid_window_cut_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 left_cut                    : 8;
        MT_U32 right_cut                   : 8;
        MT_U32 top_cut                     : 8;
        MT_U32 bot_cut                     : 8;
    } bitc;
} reg_symphony_disp_vid_window_cut_hd_t;

/*!
  the union of register reg_symphony_disp_vscaler_ratio_sd
  */
typedef union reg_symphony_disp_vscaler_ratio_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 hratio_int_sd               : 4;
        MT_U32 hratio_fra_sd               : 12;
        MT_U32 vratio_int_sd               : 4;
        MT_U32 vratio_fra_sd               : 12;
    } bitc;
} reg_symphony_disp_vscaler_ratio_sd_t;

/*!
  the union of register reg_symphony_disp_vscaler_ratio_init_sd
  */
typedef union reg_symphony_disp_vscaler_ratio_init_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 top_int_sd                  : 4;
        MT_U32 top_fra_sd                  : 12;
        MT_U32 bot_int_sd                  : 4;
        MT_U32 bot_fra_sd                  : 12;
    } bitc;
} reg_symphony_disp_vscaler_ratio_init_sd_t;

/*!
  the union of register reg_symphony_disp_vid_window_x_sd
  */
typedef union reg_symphony_disp_vid_window_x_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_x_left_sd               : 11;
        MT_U32                             : 5;
        MT_U32 vid_x_right_sd              : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_window_x_sd_t;

/*!
  the union of register reg_symphony_disp_vid_window_y_sd
  */
typedef union reg_symphony_disp_vid_window_y_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_y_start_sd              : 11;
        MT_U32                             : 5;
        MT_U32 vid_y_end_sd                : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_window_y_sd_t;

/*!
  the union of register reg_symphony_disp_graphic_ctrl
  */
typedef union reg_symphony_disp_graphic_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 8;
        MT_U32 back_sel                    : 1;
        MT_U32 still_sel_sd                : 1;
        MT_U32 still_sd_format_444         : 1;
        MT_U32                             : 1;
        MT_U32 still_sel_hd                : 1;
        MT_U32 still_hd_format             : 3;
        MT_U32 mix_layer_mode              : 3;
        MT_U32                             : 13;
    } bitc;
} reg_symphony_disp_graphic_ctrl_t;

/*!
  the union of register reg_symphony_disp_bg_color
  */
typedef union reg_symphony_disp_bg_color
{
    MT_U32 all;
    struct
    {
        MT_U32 bg_cr                       : 8;
        MT_U32 bg_cb                       : 8;
        MT_U32 bg_y                        : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_bg_color_t;

/*!
  the union of register reg_symphony_disp_still_x_hd
  */
typedef union reg_symphony_disp_still_x_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_still_startx             : 11;
        MT_U32                             : 5;
        MT_U32 hd_still_endx               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_still_x_hd_t;

/*!
  the union of register reg_symphony_disp_still_y_hd
  */
typedef union reg_symphony_disp_still_y_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_still_starty             : 11;
        MT_U32                             : 5;
        MT_U32 hd_still_endy               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_still_y_hd_t;

/*!
  the union of register reg_symphony_disp_osd0_cmd_hd
  */
typedef union reg_symphony_disp_osd0_cmd_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 osd0_sel                    : 1;
        MT_U32                             : 3;
        MT_U32 force_progressive           : 1;
        MT_U32                             : 3;
        MT_U32 plane_alpha_en              : 1;
        MT_U32                             : 7;
        MT_U32 plane_alpha                 : 8;
        MT_U32 disable_hori_process        : 1;
        MT_U32                             : 3;
        MT_U32 osd0_big_endian             : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_osd0_cmd_hd_t;

/*!
  the union of register reg_symphony_disp_sub_cmd_hd
  */
typedef union reg_symphony_disp_sub_cmd_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 sub_sel                     : 1;
        MT_U32                             : 3;
        MT_U32 force_progressive           : 1;
        MT_U32                             : 3;
        MT_U32 plane_alpha_en              : 1;
        MT_U32                             : 7;
        MT_U32 plane_alpha                 : 8;
        MT_U32 disable_hori_process        : 1;
        MT_U32                             : 3;
        MT_U32 sub_big_endian              : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_sub_cmd_hd_t;

/*!
  the union of register reg_symphony_disp_layer_alpha
  */
typedef union reg_symphony_disp_layer_alpha
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 vid_plane_alpha             : 8;
        MT_U32 still_plane_alpha           : 8;
    } bitc;
} reg_symphony_disp_layer_alpha_t;

/*!
  the union of register reg_symphony_disp_vid_input_size
  */
typedef union reg_symphony_disp_vid_input_size
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_frame_h                 : 11;
        MT_U32                             : 5;
        MT_U32 vid_frame_w                 : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_input_size_t;

/*!
  the union of register reg_symphony_disp_vid_sd_drop_line
  */
typedef union reg_symphony_disp_vid_sd_drop_line
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_top_field_drop           : 4;
        MT_U32                             : 12;
        MT_U32 sd_bot_field_drop           : 4;
        MT_U32                             : 12;
    } bitc;
} reg_symphony_disp_vid_sd_drop_line_t;

/*!
  the union of register reg_symphony_disp_vid_crop_mode_en
  */
typedef union reg_symphony_disp_vid_crop_mode_en
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_crop_en                 : 1;
        MT_U32                             : 31;
    } bitc;
} reg_symphony_disp_vid_crop_mode_en_t;

/*!
  the union of register reg_symphony_disp_vid_crop_hori
  */
typedef union reg_symphony_disp_vid_crop_hori
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_crop_endx               : 11;
        MT_U32                             : 5;
        MT_U32 vid_crop_startx             : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_crop_hori_t;

/*!
  the union of register reg_symphony_disp_vid_crop_vert
  */
typedef union reg_symphony_disp_vid_crop_vert
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_crop_endy               : 11;
        MT_U32                             : 5;
        MT_U32 vid_crop_starty             : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_vid_crop_vert_t;

/*!
  the union of register reg_symphony_disp_vid_process_mode
  */
typedef union reg_symphony_disp_vid_process_mode
{
    MT_U32 all;
    struct
    {
        MT_U32 di_process                  : 1;
        MT_U32                             : 31;
    } bitc;
} reg_symphony_disp_vid_process_mode_t;

/*!
  the union of register reg_symphony_disp_data_arrange_1
  */
typedef union reg_symphony_disp_data_arrange_1
{
    MT_U32 all;
    struct
    {
        MT_U32 data_arrange_1                  : 1;
        MT_U32                                 : 7;
        MT_U32 coeff_table_endian              : 2;
        MT_U32                                 : 22;
    } bitc;
} reg_symphony_disp_data_arrange_1_t;

/*!
  the union of register reg_symphony_disp_data_arrange_2
  */
typedef union reg_symphony_disp_data_arrange_2
{
    MT_U32 all;
    struct
    {
        MT_U32 data_arrange_2                  : 1;
        MT_U32                                 : 31;
    } bitc;
} reg_symphony_disp_data_arrange_2_t;

/*!
  the union of register reg_symphony_disp_osd1_cmd_hd
  */
typedef union reg_symphony_disp_osd1_cmd_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 osd1_sel                    : 1;
        MT_U32                             : 3;
        MT_U32 force_progressive           : 1;
        MT_U32                             : 3;
        MT_U32 plane_alpha_en              : 1;
        MT_U32                             : 7;
        MT_U32 plane_alpha                 : 8;
        MT_U32 disable_hori_process        : 1;
        MT_U32                             : 3;
        MT_U32 osd1_big_endian             : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_osd1_cmd_hd_t;

/*!
  the union of register reg_symphony_disp_osd1_ck_hd
  */
typedef union reg_symphony_disp_osd1_ck_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 ck_yuv                      : 24;
        MT_U32 ck_en                       : 1;
        MT_U32                             : 7;
    } bitc;
} reg_symphony_disp_osd1_ck_hd_t;

/*!
  the union of register reg_symphony_disp_still_x_sd
  */
typedef union reg_symphony_disp_still_x_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_still_startx             : 11;
        MT_U32                             : 5;
        MT_U32 sd_still_endx               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_still_x_sd_t;

/*!
  the union of register reg_symphony_disp_still_y_sd
  */
typedef union reg_symphony_disp_still_y_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_still_starty             : 11;
        MT_U32                             : 5;
        MT_U32 sd_still_endy               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_still_y_sd_t;

/*!
  the union of register reg_symphony_disp_still_stride_sd
  */
typedef union reg_symphony_disp_still_stride_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_still_stride             : 10;
        MT_U32                             : 6;
        MT_U32 cr_byte_sel                 : 2;
        MT_U32                             : 2;
        MT_U32 cb_byte_sel                 : 2;
        MT_U32                             : 2;
        MT_U32 y_byte_sel                  : 2;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_still_stride_sd_t;

/*!
  the union of register reg_symphony_disp_rgb2y_coef
  */
typedef union reg_symphony_disp_rgb2y_coef
{
    MT_U32 all;
    struct
    {
        MT_U32 r2y_coef                    : 9;
        MT_U32                             : 3;
        MT_U32 g2y_coef                    : 10;
        MT_U32                             : 2;
        MT_U32 b2y_coef                    : 7;
        MT_U32                             : 1;
    } bitc;
} reg_symphony_disp_rgb2y_coef_t;

/*!
  the union of register reg_symphony_disp_rgb2cb_coef
  */
typedef union reg_symphony_disp_rgb2cb_coef
{
    MT_U32 all;
    struct
    {
        MT_U32 b2cb_coef                   : 10;
        MT_U32                             : 2;
        MT_U32 g2cb_coef                   : 9;
        MT_U32                             : 3;
        MT_U32 r2cb_coef                   : 8;
    } bitc;
} reg_symphony_disp_rgb2cb_coef_t;

/*!
  the union of register reg_symphony_disp_rgb2cr_coef
  */
typedef union reg_symphony_disp_rgb2cr_coef
{
    MT_U32 all;
    struct
    {
        MT_U32 r2cr_coef                   : 10;
        MT_U32                             : 2;
        MT_U32 g2cr_coef                   : 9;
        MT_U32                             : 3;
        MT_U32 b2cr_coef                   : 7;
        MT_U32                             : 1;
    } bitc;
} reg_symphony_disp_rgb2cr_coef_t;

/*!
  the union of register reg_symphony_disp_vid_decomp_cfg
  */
typedef union reg_symphony_disp_vid_decomp_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32 config                      : 32;
    } bitc;
} reg_symphony_disp_vid_decomp_cfg_t;

/*!
  the union of register reg_symphony_disp_osd0_ck_hd
  */
typedef union reg_symphony_disp_osd0_ck_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 ck_yuv                      : 24;
        MT_U32 ck_en                       : 1;
        MT_U32                             : 7;
    } bitc;
} reg_symphony_disp_osd0_ck_hd_t;

/*!
  the union of register reg_symphony_disp_rgb2yuv_yoffset
  */
typedef union reg_symphony_disp_rgb2yuv_yoffset
{
    MT_U32 all;
    struct
    {
        MT_U32 yoffset                     : 18;
        MT_U32                             : 14;
    } bitc;
} reg_symphony_disp_rgb2yuv_yoffset_t;

/*!
  the union of register reg_symphony_disp_rgb2yuv_uvoffset
  */
typedef union reg_symphony_disp_rgb2yuv_uvoffset
{
    MT_U32 all;
    struct
    {
        MT_U32 uvoffset                    : 18;
        MT_U32                             : 14;
    } bitc;
} reg_symphony_disp_rgb2yuv_uvoffset_t;

/*!
  the union of register reg_symphony_disp_vid_verf_cfg
  */
typedef union reg_symphony_disp_vid_verf_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 28;
        MT_U32 hd_dce_en                   : 1;
        MT_U32 sd_dce_en                   : 1;
        MT_U32                             : 2;
    } bitc;
} reg_symphony_disp_vid_verf_cfg_t;

/*!
  the union of register reg_symphony_disp_vid_horf_cfg
  */
typedef union reg_symphony_disp_vid_horf_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_infl_thr                 : 8;
        MT_U32 hd_infl_thr                 : 8;
        MT_U32                             : 16;
    } bitc;
} reg_symphony_disp_vid_horf_cfg_t;

/*!
  the union of register reg_symphony_disp_layer_mix_cfg
  */
typedef union reg_symphony_disp_layer_mix_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32 bypass_sd_vid_scaler        : 1;
        MT_U32                             : 23;
        MT_U32 sd_yuv444                   : 1;
        MT_U32                             : 3;
        MT_U32 sd_wr_back_forbidden        : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_layer_mix_cfg_t;

/*!
  the union of register reg_symphony_disp_hd_size_out
  */
typedef union reg_symphony_disp_hd_size_out
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_width_out                : 11;
        MT_U32                             : 5;
        MT_U32 hd_height_out               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_hd_size_out_t;

/*!
  the union of register reg_symphony_disp_sd_size_out
  */
typedef union reg_symphony_disp_sd_size_out
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_width_out                : 11;
        MT_U32                             : 5;
        MT_U32 sd_height_out               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_sd_size_out_t;

/*!
  the union of register reg_symphony_disp_still_stride_hd
  */
typedef union reg_symphony_disp_still_stride_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_still_stride             : 11;
    #if defined CONFIG_MT_CHIP_SYMPHONY1 || defined CONFIG_MT_CHIP_SYMPHONY2
        MT_U32                             : 5;
    #else
        MT_U32                              : 1;
        MT_U32 still_endian                 : 3;
        MT_U32                              : 1;
    #endif
        MT_U32 cr_byte_sel                 : 2;
        MT_U32                             : 2;
        MT_U32 cb_byte_sel                 : 2;
        MT_U32                             : 2;
        MT_U32 y_byte_sel                  : 2;
        MT_U32                             : 2;
        MT_U32 cr_first                    : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_still_stride_hd_t;

/*!
  the union of register reg_symphony_disp_hdtv_cfg
  */
typedef union reg_symphony_disp_hdtv_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 8;
        MT_U32 hd_vid_fmt                  : 4;
        MT_U32                             : 20;
    } bitc;
} reg_symphony_disp_hdtv_cfg_t;

/*!
  the union of register reg_symphony_disp_hd_post_cfg
  */
typedef union reg_symphony_disp_hd_post_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_leverage                 : 8;
        MT_U32 hd_hp_enha                  : 8;
        MT_U32 hd_hori_enha                : 8;
        MT_U32 hd_shoot_chg                : 8;
    } bitc;
} reg_symphony_disp_hd_post_cfg_t;

/*!
  the union of register reg_symphony_disp_sd_post_cfg
  */
typedef union reg_symphony_disp_sd_post_cfg
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_leverage                 : 8;
        MT_U32 sd_hp_enha                  : 8;
        MT_U32 sd_hori_enha                : 8;
        MT_U32 sd_shoot_chg                : 8;
    } bitc;
} reg_symphony_disp_sd_post_cfg_t;

/*!
  the union of register reg_symphony_disp_hd_effect_coef
  */
typedef union reg_symphony_disp_hd_effect_coef
{
    MT_U32 all;
    struct
    {
        MT_U32 bright_coef                 : 8;
        MT_U32 contrast_coef               : 8;
        MT_U32 saturation_coef             : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_hd_effect_coef_t;

/*!
  the union of register reg_symphony_disp_sd_effect_coef
  */
typedef union reg_symphony_disp_sd_effect_coef
{
    MT_U32 all;
    struct
    {
        MT_U32 bright_coef                 : 8;
        MT_U32 contrast_coef               : 8;
        MT_U32 saturation_coef             : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_sd_effect_coef_t;

/*!
  the union of register reg_symphony_disp_gra_fifo_threshold
  */
typedef union reg_symphony_disp_gra_fifo_threshold
{
    MT_U32 all;
    struct
    {
        MT_U32 fifo64_lo                   : 6;
        MT_U32                             : 2;
        MT_U32 fifo64_hi                   : 6;
        MT_U32                             : 2;
        MT_U32 fifo128_lo                  : 7;
        MT_U32                             : 1;
        MT_U32 fifo128_hi                  : 7;
        MT_U32                             : 1;
    } bitc;
} reg_symphony_disp_gra_fifo_threshold_t;

/*!
  the union of register reg_symphony_disp_gra_scaler_ctrl
  */
typedef union reg_symphony_disp_gra_scaler_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 hori_filter_en              : 1;
        MT_U32 vert_filter_en              : 1;
        MT_U32 hori_phase_type             : 1;
        MT_U32 vert_phase_type             : 1;
        MT_U32 vert_table_num              : 2;
        MT_U32                             : 2;
        MT_U32 hori_table_num              : 3;
        MT_U32                             : 1;
        MT_U32 vert_start_line_odd         : 3;
        MT_U32                             : 1;
        MT_U32 vert_start_line_even        : 3;
        MT_U32                             : 13;
    } bitc;
} reg_symphony_disp_gra_scaler_ctrl_t;

/*!
  the union of register reg_symphony_disp_gra_scaler_hratio
  */
typedef union reg_symphony_disp_gra_scaler_hratio
{
    MT_U32 all;
    struct
    {
        MT_U32 hratio_int                  : 3;
        MT_U32                             : 5;
        MT_U32 hratio_fra                  : 12;
        MT_U32                             : 12;
    } bitc;
} reg_symphony_disp_gra_scaler_hratio_t;

/*!
  the union of register reg_symphony_disp_gra_scaler_vratio
  */
typedef union reg_symphony_disp_gra_scaler_vratio
{
    MT_U32 all;
    struct
    {
        MT_U32 vratio_int                  : 3;
        MT_U32                             : 5;
        MT_U32 vratio_fra                  : 12;
        MT_U32                             : 12;
    } bitc;
} reg_symphony_disp_gra_scaler_vratio_t;

/*!
  the union of register reg_symphony_disp_gra_scaler_h_start_fra
  */
typedef union reg_symphony_disp_gra_scaler_h_start_fra
{
    MT_U32 all;
    struct
    {
        MT_U32 h_start_fra                 : 12;
        MT_U32                             : 20;
    } bitc;
} reg_symphony_disp_gra_scaler_h_start_fra_t;

/*!
  the union of register reg_symphony_disp_gra_scaler_v_start_fra
  */
typedef union reg_symphony_disp_gra_scaler_v_start_fra
{
    MT_U32 all;
    struct
    {
        MT_U32 v_start_fra_odd             : 12;
        MT_U32                             : 4;
        MT_U32 v_start_fra_even            : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_gra_scaler_v_start_fra_t;

/*!
  the union of register reg_symphony_disp_gra_ctl
  */
typedef union reg_symphony_disp_gra_ctl
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_prog_mode                : 1;
        MT_U32                             : 3;
        MT_U32 reduce_fr_osd0              : 1;
        MT_U32 reduce_fr_osd1              : 1;
        MT_U32 reduce_fr_sub               : 1;
        MT_U32 reduce_fr_hd_still          : 1;
        MT_U32                             : 24;
    } bitc;
} reg_symphony_disp_gra_ctl_t;

/*!
  the union of register reg_symphony_disp_osd_scale_hsize
  */
typedef union reg_symphony_disp_osd_scale_hsize
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_dst_hsize               : 11;
        MT_U32                             : 5;
        MT_U32 osd_src_hsize               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_osd_scale_hsize_t;

/*!
  the union of register reg_symphony_disp_osd_scale_ratio
  */
typedef union reg_symphony_disp_osd_scale_ratio
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_ratio_fra               : 12;
        MT_U32                             : 4;
        MT_U32 osd_ratio_int               : 3;
        MT_U32                             : 13;
    } bitc;
} reg_symphony_disp_osd_scale_ratio_t;

/*!
  the union of register reg_symphony_disp_osd_alpha
  */
typedef union reg_symphony_disp_osd_alpha
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 8;
        MT_U32 osd_alpha_filter            : 1;
        MT_U32 border_cfg                  : 1;
        MT_U32                             : 6;
        MT_U32 osd_start_fra               : 12;
        MT_U32 osd_no_filter               : 1;
        MT_U32                             : 2;
        MT_U32 osd_sub_mix_first           : 1;
    } bitc;
} reg_symphony_disp_osd_alpha_t;

/*!
  the union of register reg_symphony_disp_osd_vert_start_line
  */
typedef union reg_symphony_disp_osd_vert_start_line
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_odd_start_line          : 4;
        MT_U32 osd_even_start_line         : 4;
        MT_U32                             : 24;
    } bitc;
} reg_symphony_disp_osd_vert_start_line_t;

/*!
  the union of register reg_symphony_disp_osd_vertical_ctrl
  */
typedef union reg_symphony_disp_osd_vertical_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_vert_phase_type         : 1;
        MT_U32                             : 3;
        MT_U32 osd_vert_no_filter          : 1;
        MT_U32                             : 3;
        MT_U32 osd_vert_bypass             : 1;
        MT_U32                             : 23;
    } bitc;
} reg_symphony_disp_osd_vertical_ctrl_t;

/*!
  the union of register reg_symphony_disp_osd_vertical_size
  */
typedef union reg_symphony_disp_osd_vertical_size
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_dst_vsize               : 11;
        MT_U32                             : 5;
        MT_U32 osd_ori_vsize               : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_osd_vertical_size_t;

/*!
  the union of register reg_symphony_disp_osd_vertical_ratio
  */
typedef union reg_symphony_disp_osd_vertical_ratio
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_vratio_fra              : 12;
        MT_U32                             : 4;
        MT_U32 osd_vratio_int              : 4;
        MT_U32                             : 12;
    } bitc;
} reg_symphony_disp_osd_vertical_ratio_t;

/*!
  the union of register reg_symphony_disp_osd_v_start_fra
  */
typedef union reg_symphony_disp_osd_v_start_fra
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_v_start_fra_odd         : 12;
        MT_U32                             : 4;
        MT_U32 osd_v_start_fra_even        : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_osd_v_start_fra_t;

/*!
  the union of register reg_symphony_disp_osd_v_tap_num
  */
typedef union reg_symphony_disp_osd_v_tap_num
{
    MT_U32 all;
    struct
    {
        MT_U32 tap_num                     : 3;
        MT_U32                             : 29;
    } bitc;
} reg_symphony_disp_osd_v_tap_num_t;

/*!
  the union of register reg_symphony_disp_chroma_coef0
  */
typedef union reg_symphony_disp_chroma_coef0
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_hori_coef1           : 11;
        MT_U32                             : 5;
        MT_U32 chroma_hori_coef0           : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_chroma_coef0_t;

/*!
  the union of register reg_symphony_disp_chroma_coef1
  */
typedef union reg_symphony_disp_chroma_coef1
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_hori_coef3           : 11;
        MT_U32                             : 5;
        MT_U32 chroma_hori_coef2           : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_chroma_coef1_t;

/*!
  the union of register reg_symphony_disp_chroma_coef2
  */
typedef union reg_symphony_disp_chroma_coef2
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_hori_coef5           : 11;
        MT_U32                             : 5;
        MT_U32 chroma_hori_coef4           : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_chroma_coef2_t;

/*!
  the union of register reg_symphony_disp_chroma_coef3
  */
typedef union reg_symphony_disp_chroma_coef3
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 chroma_hori_coef6           : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_chroma_coef3_t;

/*!
  the union of register reg_symphony_disp_scale_init_phase_offset
  */
typedef union reg_symphony_disp_scale_init_phase_offset
{
    MT_U32 all;
    struct
    {
        MT_U32 offset                      : 1;
        MT_U32                             : 11;
        MT_U32 vid_fmt                     : 3;
        MT_U32                             : 1;
        MT_U32 input_ar                    : 8;
        MT_U32 input_fr                    : 8;
    } bitc;
} reg_symphony_disp_scale_init_phase_offset_t;

/*!
  the union of register reg_symphony_disp_small_pic_upscale_ctrl
  */
typedef union reg_symphony_disp_small_pic_upscale_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 upscale_en                  : 1;
        MT_U32                             : 3;
        MT_U32 chroma_hori_ip_mode         : 2;
        MT_U32                             : 26;
    } bitc;
} reg_symphony_disp_small_pic_upscale_ctrl_t;

/*!
  the union of register reg_symphony_disp_alising_prob_reg1
  */
typedef union reg_symphony_disp_alising_prob_reg1
{
    MT_U32 all;
    struct
    {
        MT_U32 alpha_2nd_method_sel        : 1;
        MT_U32                             : 7;
        MT_U32 alpha_2nd_diff_ratio_sel    : 2;
        MT_U32                             : 6;
        MT_U32 alpha_2nd_diff_shift_sel    : 1;
        MT_U32                             : 7;
        MT_U32 alpha_2nd_diff              : 8;
    } bitc;
} reg_symphony_disp_alising_prob_reg1_t;

/*!
  the union of register reg_symphony_disp_alising_prob_reg2
  */
typedef union reg_symphony_disp_alising_prob_reg2
{
    MT_U32 all;
    struct
    {
        MT_U32 alpha_enlarge               : 10;
        MT_U32                             : 2;
        MT_U32 alpha_vdv_sel               : 2;
        MT_U32                             : 2;
        MT_U32 alpha_vdv                   : 8;
        MT_U32 alpha_angle                 : 8;
    } bitc;
} reg_symphony_disp_alising_prob_reg2_t;

/*!
  the union of register reg_symphony_disp_alising_prob_reg3
  */
typedef union reg_symphony_disp_alising_prob_reg3
{
    MT_U32 all;
    struct
    {
        MT_U32 base_blending_factor        : 8;
        MT_U32 default_prob                : 8;
        MT_U32 op                          : 2;
        MT_U32                             : 6;
        MT_U32 slope_diff1_sel             : 2;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_alising_prob_reg3_t;

/*!
  the union of register reg_symphony_disp_alising_prob_reg4
  */
typedef union reg_symphony_disp_alising_prob_reg4
{
    MT_U32 all;
    struct
    {
        MT_U32 pict_enhance_pix_sel        : 2;
        MT_U32 adaptive_alpha_sel          : 1;
        MT_U32 diff_2nd_sel                : 1;
        MT_U32 interp_factor               : 4;
        MT_U32 prob_coef3                  : 8;
        MT_U32 prob_coef2                  : 8;
        MT_U32 prob_coef1                  : 8;
    } bitc;
} reg_symphony_disp_alising_prob_reg4_t;

/*!
  the union of register reg_symphony_disp_csc_ctrl
  */
typedef union reg_symphony_disp_csc_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_bound_out_en             : 1;
        MT_U32                             : 6;
        MT_U32 osd_hd2sd_csc_en            : 1;
        MT_U32 hd_bound_out_en             : 1;
        MT_U32                             : 6;
        MT_U32 sd_csc_en                   : 1;
        MT_U32 sd_bound_in_en              : 1;
        MT_U32                             : 7;
        MT_U32 hd_bound_in_en              : 1;
        MT_U32                             : 6;
        MT_U32 hd_csc_en                   : 1;
    } bitc;
} reg_symphony_disp_csc_ctrl_t;

/*!
  the union of register reg_symphony_disp_csc_hd_coef1
  */
typedef union reg_symphony_disp_csc_hd_coef1
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_a_01                     : 12;
        MT_U32                             : 4;
        MT_U32 hd_a_00                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_hd_coef1_t;

/*!
  the union of register reg_symphony_disp_csc_hd_coef2
  */
typedef union reg_symphony_disp_csc_hd_coef2
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_a_10                     : 12;
        MT_U32                             : 4;
        MT_U32 hd_a_02                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_hd_coef2_t;

/*!
  the union of register reg_symphony_disp_csc_hd_coef3
  */
typedef union reg_symphony_disp_csc_hd_coef3
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_a_12                     : 12;
        MT_U32                             : 4;
        MT_U32 hd_a_11                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_hd_coef3_t;

/*!
  the union of register reg_symphony_disp_csc_hd_coef4
  */
typedef union reg_symphony_disp_csc_hd_coef4
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_a_21                     : 12;
        MT_U32                             : 4;
        MT_U32 hd_a_20                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_hd_coef4_t;

/*!
  the union of register reg_symphony_disp_csc_hd_coef5
  */
typedef union reg_symphony_disp_csc_hd_coef5
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 hd_a_22                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_hd_coef5_t;

/*!
  the union of register reg_symphony_disp_csc_sd_coef1
  */
typedef union reg_symphony_disp_csc_sd_coef1
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_a_01                     : 12;
        MT_U32                             : 4;
        MT_U32 sd_a_00                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_sd_coef1_t;

/*!
  the union of register reg_symphony_disp_csc_sd_coef2
  */
typedef union reg_symphony_disp_csc_sd_coef2
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_a_10                     : 12;
        MT_U32                             : 4;
        MT_U32 sd_a_02                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_sd_coef2_t;

/*!
  the union of register reg_symphony_disp_csc_sd_coef3
  */
typedef union reg_symphony_disp_csc_sd_coef3
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_a_12                     : 12;
        MT_U32                             : 4;
        MT_U32 sd_a_11                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_sd_coef3_t;

/*!
  the union of register reg_symphony_disp_csc_sd_coef4
  */
typedef union reg_symphony_disp_csc_sd_coef4
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_a_21                     : 12;
        MT_U32                             : 4;
        MT_U32 sd_a_20                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_sd_coef4_t;

/*!
  the union of register reg_symphony_disp_csc_sd_coef5
  */
typedef union reg_symphony_disp_csc_sd_coef5
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 sd_a_22                     : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_sd_coef5_t;

/*!
  the union of register reg_symphony_disp_csc_still_ctrl
  */
typedef union reg_symphony_disp_csc_still_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 8;
        MT_U32 hd_still_bund_out_en        : 1;
        MT_U32                             : 15;
        MT_U32 hd_still_bund_in_en         : 1;
        MT_U32                             : 6;
        MT_U32 hd_still_csc__en            : 1;
    } bitc;
} reg_symphony_disp_csc_still_ctrl_t;

/*!
  the union of register reg_symphony_disp_csc_still_coef1
  */
typedef union reg_symphony_disp_csc_still_coef1
{
    MT_U32 all;
    struct
    {
        MT_U32 still_a_01                  : 12;
        MT_U32                             : 4;
        MT_U32 still_a_00                  : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_still_coef1_t;

/*!
  the union of register reg_symphony_disp_csc_still_coef2
  */
typedef union reg_symphony_disp_csc_still_coef2
{
    MT_U32 all;
    struct
    {
        MT_U32 still_a_10                  : 12;
        MT_U32                             : 4;
        MT_U32 still_a_02                  : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_still_coef2_t;

/*!
  the union of register reg_symphony_disp_csc_still_coef3
  */
typedef union reg_symphony_disp_csc_still_coef3
{
    MT_U32 all;
    struct
    {
        MT_U32 still_a_12                  : 12;
        MT_U32                             : 4;
        MT_U32 still_a_11                  : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_still_coef3_t;

/*!
  the union of register reg_symphony_disp_csc_still_coef4
  */
typedef union reg_symphony_disp_csc_still_coef4
{
    MT_U32 all;
    struct
    {
        MT_U32 still_a_21                  : 12;
        MT_U32                             : 4;
        MT_U32 still_a_20                  : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_still_coef4_t;

/*!
  the union of register reg_symphony_disp_csc_still_coef5
  */
typedef union reg_symphony_disp_csc_still_coef5
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 still_a_22                  : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_csc_still_coef5_t;

/*!
  the union of register reg_symphony_disp_row_jump_00
  */
typedef union reg_symphony_disp_row_jump_00
{
    MT_U32 all;
    struct
    {
        MT_U32 rowjump_00                  : 32;
    } bitc;
} reg_symphony_disp_row_jump_00_t;

/*!
  the union of register reg_symphony_disp_row_jump_01
  */
typedef union reg_symphony_disp_row_jump_01
{
    MT_U32 all;
    struct
    {
        MT_U32 rowjump_01                  : 32;
    } bitc;
} reg_symphony_disp_row_jump_01_t;

/*!
  the union of register reg_symphony_disp_row_jump_10
  */
typedef union reg_symphony_disp_row_jump_10
{
    MT_U32 all;
    struct
    {
        MT_U32 rowjump_10                  : 32;
    } bitc;
} reg_symphony_disp_row_jump_10_t;

/*!
  the union of register reg_symphony_disp_row_jump_11
  */
typedef union reg_symphony_disp_row_jump_11
{
    MT_U32 all;
    struct
    {
        MT_U32 rowjump_11                  : 32;
    } bitc;
} reg_symphony_disp_row_jump_11_t;

/*!
  the union of register reg_symphony_disp_denoise_domain_1
  */
typedef union reg_symphony_disp_denoise_domain_1
{
    MT_U32 all;
    struct
    {
        MT_U32 domain_c2                   : 8;
        MT_U32                             : 8;
        MT_U32 domain_c1                   : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_denoise_domain_1_t;

/*!
  the union of register reg_symphony_disp_denoise_domain_2
  */
typedef union reg_symphony_disp_denoise_domain_2
{
    MT_U32 all;
    struct
    {
        MT_U32 domain_c4                   : 8;
        MT_U32                             : 8;
        MT_U32 domain_c3                   : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_denoise_domain_2_t;

/*!
  the union of register reg_symphony_disp_denoise_domain_3
  */
typedef union reg_symphony_disp_denoise_domain_3
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 domain_c5                   : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_denoise_domain_3_t;

/*!
  the union of register reg_symphony_disp_denoise_range_1
  */
typedef union reg_symphony_disp_denoise_range_1
{
    MT_U32 all;
    struct
    {
        MT_U32 range_3                     : 8;
        MT_U32 range_2                     : 8;
        MT_U32 range_1                     : 8;
        MT_U32 range_0                     : 8;
    } bitc;
} reg_symphony_disp_denoise_range_1_t;

/*!
  the union of register reg_symphony_disp_denoise_range_2
  */
typedef union reg_symphony_disp_denoise_range_2
{
    MT_U32 all;
    struct
    {
        MT_U32 range_7                     : 8;
        MT_U32 range_6                     : 8;
        MT_U32 range_5                     : 8;
        MT_U32 range_4                     : 8;
    } bitc;
} reg_symphony_disp_denoise_range_2_t;

/*!
  the union of register reg_symphony_disp_denoise_range_3
  */
typedef union reg_symphony_disp_denoise_range_3
{
    MT_U32 all;
    struct
    {
        MT_U32 range_11                    : 8;
        MT_U32 range_10                    : 8;
        MT_U32 range_9                     : 8;
        MT_U32 range_8                     : 8;
    } bitc;
} reg_symphony_disp_denoise_range_3_t;

/*!
  the union of register reg_symphony_disp_denoise_range_4
  */
typedef union reg_symphony_disp_denoise_range_4
{
    MT_U32 all;
    struct
    {
        MT_U32 range_15                    : 8;
        MT_U32 range_14                    : 8;
        MT_U32 range_13                    : 8;
        MT_U32 range_12                    : 8;
    } bitc;
} reg_symphony_disp_denoise_range_4_t;

/*!
  the union of register reg_symphony_disp_denoise_range_5
  */
typedef union reg_symphony_disp_denoise_range_5
{
    MT_U32 all;
    struct
    {
        MT_U32 range_19                    : 8;
        MT_U32 range_18                    : 8;
        MT_U32 range_17                    : 8;
        MT_U32 range_16                    : 8;
    } bitc;
} reg_symphony_disp_denoise_range_5_t;

/*!
  the union of register reg_symphony_disp_denoise_range_6
  */
typedef union reg_symphony_disp_denoise_range_6
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_denoise_en              : 1;
        MT_U32                             : 7;
        MT_U32 z_reg                       : 8;
        MT_U32                             : 8;
        MT_U32 range_20                    : 8;
    } bitc;
} reg_symphony_disp_denoise_range_6_t;

/*!
  the union of register reg_symphony_disp_color_enhance_ctrl
  */
typedef union reg_symphony_disp_color_enhance_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 color_enhance_en            : 1;
        MT_U32                             : 7;
        MT_U32 color_enhance_reg_red_dec   : 7;
        MT_U32                             : 5;
        MT_U32 color_enhance_reg_length    : 7;
        MT_U32                             : 1;
        MT_U32 color_enhance_reg_thr       : 4;
    } bitc;
} reg_symphony_disp_color_enhance_ctrl_t;

/*!
  the union of register reg_symphony_disp_sd_wrback_addr_odd
  */
typedef union reg_symphony_disp_sd_wrback_addr_odd
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_wrback_addr_odd          : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_sd_wrback_addr_odd_t;

/*!
  the union of register reg_symphony_disp_sd_base_addr_even
  */
typedef union reg_symphony_disp_sd_base_addr_even
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_base_addr_even           : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_sd_base_addr_even_t;

/*!
  the union of register reg_symphony_disp_sd_wrback_ctrl
  */
typedef union reg_symphony_disp_sd_wrback_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_start_lines              : 9;
        MT_U32                             : 7;
        MT_U32 one_field_buffer            : 1;
        MT_U32                             : 7;
        MT_U32 sd_buffer_num               : 2;
        MT_U32                             : 2;
        MT_U32 sd_blankscreen_mode         : 1;
        MT_U32                             : 2;
        MT_U32 sd_softctrl_en              : 1;
    } bitc;
} reg_symphony_disp_sd_wrback_ctrl_t;

/*!
  the union of register reg_symphony_disp_sd_back_color
  */
typedef union reg_symphony_disp_sd_back_color
{
    MT_U32 all;
    struct
    {
        MT_U32 v                           : 8;
        MT_U32 u                           : 8;
        MT_U32 y                           : 8;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_disp_sd_back_color_t;

/*!
  the union of register reg_symphony_disp_still_uv_start_addr_hd
  */
typedef union reg_symphony_disp_still_uv_start_addr_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_still_uv_start_addr      : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_still_uv_start_addr_hd_t;

/*!
  the union of register reg_symphony_disp_still_y_start_addr_hd
  */
typedef union reg_symphony_disp_still_y_start_addr_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_still_y_start_addr       : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_still_y_start_addr_hd_t;

/*!
  the union of register reg_symphony_disp_sub_start_addr_hd
  */
typedef union reg_symphony_disp_sub_start_addr_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 sub_header_addr             : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_sub_start_addr_hd_t;

/*!
  the union of register reg_symphony_disp_osd1_start_addr_hd
  */
typedef union reg_symphony_disp_osd1_start_addr_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 osd1_header_addr            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_osd1_start_addr_hd_t;

/*!
  the union of register reg_symphony_disp_still_start_addr_sd
  */
typedef union reg_symphony_disp_still_start_addr_sd
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_still_start_addr         : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_still_start_addr_sd_t;

/*!
  the union of register reg_symphony_disp_osd0_start_addr_hd
  */
typedef union reg_symphony_disp_osd0_start_addr_hd
{
    MT_U32 all;
    struct
    {
        MT_U32 osd0_header_addr            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_osd0_start_addr_hd_t;

/*!
  the union of register reg_symphony_disp_video_display_info
  */
typedef union reg_symphony_disp_video_display_info
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 5;
        MT_U32 firmware_force_di_disable   : 1;
        MT_U32 progressive_flag            : 1;
        MT_U32 frame_field_flag            : 1;
        MT_U32                             : 2;
        MT_U32 height_low2bits             : 2;
        MT_U32                             : 1;
        MT_U32 height_mid3bits             : 3;
        MT_U32                             : 1;
        MT_U32 height_high3bits            : 3;
        MT_U32                             : 2;
        MT_U32 width_low2bits              : 2;
        MT_U32                             : 1;
        MT_U32 width_mid3bits              : 3;
        MT_U32                             : 1;
        MT_U32 width_high3bits             : 3;
    } bitc;
} reg_symphony_disp_video_display_info_t;

/*!
  the union of register reg_symphony_disp_motion_pre_addr
  */
typedef union reg_symphony_disp_motion_pre_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 motion_pre_addr             : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_motion_pre_addr_t;

/*!
  the union of register reg_symphony_disp_motion_cur_addr
  */
typedef union reg_symphony_disp_motion_cur_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 motion_cur_addr             : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_motion_cur_addr_t;

/*!
  the union of register reg_symphony_disp_luma_pre_addr
  */
typedef union reg_symphony_disp_luma_pre_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_pre_addr               : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_pre_addr_t;

/*!
  the union of register reg_symphony_disp_luma_cur_addr
  */
typedef union reg_symphony_disp_luma_cur_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_cur_addr               : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_cur_addr_t;

/*!
  the union of register reg_symphony_disp_luma_next_addr
  */
typedef union reg_symphony_disp_luma_next_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_next_addr              : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_next_addr_t;

/*!
  the union of register reg_symphony_disp_chroma_ppre_addr
  */
typedef union reg_symphony_disp_chroma_ppre_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_ppre_addr            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_ppre_addr_t;

/*!
  the union of register reg_symphony_disp_chroma_pre_addr
  */
typedef union reg_symphony_disp_chroma_pre_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_pre_addr             : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_pre_addr_t;

/*!
  the union of register reg_symphony_disp_chroma_cur_addr
  */
typedef union reg_symphony_disp_chroma_cur_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_cur_addr             : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_cur_addr_t;

/*!
  the union of register reg_symphony_disp_chroma_next_addr
  */
typedef union reg_symphony_disp_chroma_next_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_next_addr            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_next_addr_t;

/*!
  the union of register reg_symphony_disp_motion_pre_addr2
  */
typedef union reg_symphony_disp_motion_pre_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 motion_pre_addr2            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_motion_pre_addr2_t;

/*!
  the union of register reg_symphony_disp_motion_cur_addr2
  */
typedef union reg_symphony_disp_motion_cur_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 motion_cur_addr2            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_motion_cur_addr2_t;

/*!
  the union of register reg_symphony_disp_luma_pre_addr2
  */
typedef union reg_symphony_disp_luma_pre_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_pre_addr2              : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_pre_addr2_t;

/*!
  the union of register reg_symphony_disp_luma_cur_addr2
  */
typedef union reg_symphony_disp_luma_cur_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_cur_addr2              : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_cur_addr2_t;

/*!
  the union of register reg_symphony_disp_luma_next_addr2
  */
typedef union reg_symphony_disp_luma_next_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_next_addr2             : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_next_addr2_t;

/*!
  the union of register reg_symphony_disp_chroma_ppre_addr2
  */
typedef union reg_symphony_disp_chroma_ppre_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_ppre_addr2           : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_ppre_addr2_t;

/*!
  the union of register reg_symphony_disp_chroma_pre_addr2
  */
typedef union reg_symphony_disp_chroma_pre_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_pre_addr2            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_pre_addr2_t;

/*!
  the union of register reg_symphony_disp_chroma_cur_addr2
  */
typedef union reg_symphony_disp_chroma_cur_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_cur_addr2            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_cur_addr2_t;

/*!
  the union of register reg_symphony_disp_chroma_next_addr2
  */
typedef union reg_symphony_disp_chroma_next_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_next_addr2           : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_next_addr2_t;

/*!
  the union of register reg_symphony_disp_field_pic_fmt
  */
typedef union reg_symphony_disp_field_pic_fmt
{
    MT_U32 all;
    struct
    {
        MT_U32 field_pic_fmt               : 1;
        MT_U32                             : 31;
    } bitc;
} reg_symphony_disp_field_pic_fmt_t;

/*!
  the union of register reg_symphony_disp_vid_hd_vf_coef_addr
  */
typedef union reg_symphony_disp_vid_hd_vf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_hd_vf_coef_addr         : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_vid_hd_vf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_vid_hd_hf_coef_addr
  */
typedef union reg_symphony_disp_vid_hd_hf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_hd_hf_coef_addr         : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_vid_hd_hf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_vid_sd_vf_coef_addr
  */
typedef union reg_symphony_disp_vid_sd_vf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_sd_vf_coef_addr         : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_vid_sd_vf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_vid_sd_hf_coef_addr
  */
typedef union reg_symphony_disp_vid_sd_hf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_sd_hf_coef_addr         : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_vid_sd_hf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_gra_vf_coef_addr
  */
typedef union reg_symphony_disp_gra_vf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 vf_coef_addr                : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_gra_vf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_gra_hf_coef_addr
  */
typedef union reg_symphony_disp_gra_hf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 hf_coef_addr                : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_gra_hf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_vid_dce_map_addr
  */
typedef union reg_symphony_disp_vid_dce_map_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 dce_map_addr                : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_vid_dce_map_addr_t;

/*!
  the union of register reg_symphony_disp_vscaler_table_sel
  */
typedef union reg_symphony_disp_vscaler_table_sel
{
    MT_U32 all;
    struct
    {
        MT_U32 dce_table_sel               : 1;
        MT_U32 osd_hori_table_sel          : 1;
        MT_U32 osd_vert_table_sel          : 1;
        MT_U32 gra_hori_table_sel          : 1;
        MT_U32 gra_vert_table_sel          : 1;
        MT_U32 chroma_sd_hori_table_sel    : 1;
        MT_U32 chroma_hd_hori_table_sel    : 1;
        MT_U32 sd_hori_table_sel           : 1;
        MT_U32 sd_vert_table_sel           : 1;
        MT_U32 hd_hori_table_sel           : 1;
        MT_U32 hd_vert_table_sel           : 1;

#ifdef CONFIG_MT_CHIP_SYMPHONY2
        MT_U32 new_csc_coef_table_2_load_en: 1;
        MT_U32 new_csc_coef_table_1_load_en: 1;
        MT_U32 new_csc_coef_table_6_load_en: 1;
        MT_U32 new_csc_coef_table_5_load_en: 1;
        MT_U32 new_csc_coef_table_4_load_en: 1;
        MT_U32 new_csc_coef_table_3_load_en: 1;
#else
        MT_U32 sd_new_csc_rgb_table_in: 1;
        MT_U32 sd_new_csc_rgb_table_out: 1;
        MT_U32 osd_new_csc_table_in: 1;
        MT_U32 osd_new_csc_table_out: 1;
        MT_U32 osd_new_csc_gain_table_en: 1;
        MT_U32 sd_new_csc_gain_table_en: 1;
#endif

        //sym4
        MT_U32 dscaler_h_coeff_table_en    : 1;
        MT_U32 dscaler_v_coeff_table_en    : 1;
        MT_U32 adv_3d_lut_table_en         : 1;
        MT_U32 tch_is_lut_table_en         : 1;
        MT_U32 tch_p_lut_table_en          : 1;
        MT_U32 tch_c_lut_table_en          : 1;
        MT_U32 tch_r_d_lut_table_en        : 1;
        MT_U32 tch_g_d_lut_table_en        : 1;
        MT_U32 tch_b_d_lut_table_en        : 1;

        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_vscaler_table_sel_t;

/*!
  the union of register reg_symphony_disp_osd_vf_coef_addr
  */
typedef union reg_symphony_disp_osd_vf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_vf_coef_addr            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_osd_vf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_osd_hf_coef_addr
  */
typedef union reg_symphony_disp_osd_hf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 osd_hf_coef_addr            : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_osd_hf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_chroma_hd_hf_coef_addr
  */
typedef union reg_symphony_disp_chroma_hd_hf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_hd_hf_coef_addr      : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_hd_hf_coef_addr_t;

/*!
  the union of register reg_symphony_disp_chroma_sd_hf_coef_addr
  */
typedef union reg_symphony_disp_chroma_sd_hf_coef_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_sd_hf_coef_addr      : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_sd_hf_coef_addr_t;

//hmccccccccccccccccccccccccccccccccccccc
typedef union reg_symphony_disp_luma_bot_cur_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_bot_cur_addr               : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_bot_cur_addr_t;

typedef union reg_symphony_disp_chroma_bot_cur_addr
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_bot_cur_addr               : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_bot_cur_addr_t;

typedef union reg_symphony_disp_luma_bot_cur_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 luma_bot_cur_addr               : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_luma_bot_cur_addr2_t;

typedef union reg_symphony_disp_chroma_bot_cur_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32 chroma_bot_cur_addr               : 26;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_chroma_bot_cur_addr2_t;

/*!
  the union of register reg_symphony_disp_nlmeans_denoise_ctrl
  */
typedef union reg_symphony_disp_nlmeans_denoise_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 nlmeans_alpha               : 9;
        MT_U32                             : 7;
        MT_U32 denoise_thrn                : 4;
        MT_U32 denoise_thrb                : 4;
        MT_U32 denoise_thrg                : 8;
    } bitc;
} reg_symphony_disp_nlmeans_denoise_ctrl_t;

/*!
  the union of register reg_symphony_disp_nlmeans_parameter_1
  */
typedef union reg_symphony_disp_nlmeans_parameter_1
{
    MT_U32 all;
    struct
    {
        MT_U32 reg_a2                      : 12;
        MT_U32                             : 4;
        MT_U32 reg_a1                      : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_nlmeans_parameter_1_t;

/*!
  the union of register reg_symphony_disp_nlmeans_parameter_2
  */
typedef union reg_symphony_disp_nlmeans_parameter_2
{
    MT_U32 all;
    struct
    {
        MT_U32 reg_b1                      : 12;
        MT_U32                             : 4;
        MT_U32 reg_a3                      : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_nlmeans_parameter_2_t;

/*!
  the union of register reg_symphony_disp_nlmeans_parameter_3
  */
typedef union reg_symphony_disp_nlmeans_parameter_3
{
    MT_U32 all;
    struct
    {
        MT_U32 reg_b3                      : 12;
        MT_U32                             : 4;
        MT_U32 reg_b2                      : 12;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_nlmeans_parameter_3_t;

/*!
  the union of register reg_symphony_disp_nlmeans_parameter_3
  */
typedef union reg_symphony_disp_output_data_sel
{
    MT_U32 all;
    struct
    {
        MT_U32 hd2sd_data_sel                                         : 2;
        MT_U32                                                                    : 2;
        MT_U32 hdmi_data_sel_in_gralayer_mix              : 2;
        MT_U32                                                                    : 2;
        MT_U32 hdmi_data_sel_in_screen_mix                 : 2;
        MT_U32                                                                    : 2;
        MT_U32 osd_mode_sel                                            : 1;
        MT_U32                                                                    : 3;
        MT_U32 sd_osd_mode_sel                                      : 1;
        MT_U32                                                                    : 15;
    } bitc;
} reg_symphony_disp_output_data_sel_t;

/*!
  the union of register reg_symphony_disp_di_enable
  */
typedef union reg_symphony_disp_di_enable
{
    MT_U32 all;
    struct
    {
        MT_U32 enable                      : 1;
        MT_U32                             : 31;
    } bitc;
} reg_symphony_disp_di_enable_t;

/*!
  the union of register reg_symphony_disp_video_pdd_en
  */
typedef union reg_symphony_disp_video_pdd_en
{
    MT_U32 all;
    struct
    {
        MT_U32 di_pdd_en                   : 1;
        MT_U32                             : 31;
    } bitc;
} reg_symphony_disp_video_pdd_en_t;

/*!
  the union of register reg_symphony_disp_video_is_movie_type
  */
typedef union reg_symphony_disp_video_is_movie_type
{
    MT_U32 all;
    struct
    {
        MT_U32 is_movie_type               : 1;
        MT_U32                             : 31;
    } bitc;
} reg_symphony_disp_video_is_movie_type_t;

/*!
  the union of register reg_symphony_disp_di_p_or_n_paired
  */
typedef union reg_symphony_disp_di_p_or_n_paired
{
    MT_U32 all;
    struct
    {
        MT_U32 p_or_n_paired               : 5;
        MT_U32                             : 27;
    } bitc;
} reg_symphony_disp_di_p_or_n_paired_t;

/*!
  the union of register reg_symphony_disp_di_oper_mode
  */
typedef union reg_symphony_disp_di_oper_mode
{
    MT_U32 all;
    struct
    {
        MT_U32 mix_output_mode             : 2;
        MT_U32                             : 6;
        MT_U32 spatial_ip_mode             : 3;
        MT_U32                             : 1;
        MT_U32 hori_ip_en                  : 1;
        MT_U32                             : 3;
        MT_U32 temporal_ip_mode            : 1;
        MT_U32                             : 7;
        MT_U32 lbam_en                     : 1;
        MT_U32 motion_est_mode             : 2;
        MT_U32                             : 1;
        MT_U32 motion_rd_en                : 1;
        MT_U32 motion_wr_en                : 1;
        MT_U32                             : 2;
    } bitc;
} reg_symphony_disp_di_oper_mode_t;

/*!
  the union of register reg_symphony_disp_di_para
  */
typedef union reg_symphony_disp_di_para
{
    MT_U32 all;
    struct
    {
        MT_U32 g_alpha_k                   : 8;
        MT_U32 g_alpha_0                   : 8;
        MT_U32 p_tl                        : 5;
        MT_U32                             : 11;
    } bitc;
} reg_symphony_disp_di_para_t;

/*!
  the union of register reg_symphony_disp_di_data_sfifo_thr
  */
typedef union reg_symphony_disp_di_data_sfifo_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 di_data_sfifo_thr           : 6;
        MT_U32                             : 26;
    } bitc;
} reg_symphony_disp_di_data_sfifo_thr_t;

/*!
  the union of register reg_symphony_disp_video_scaler_data_sfifo_thr
  */
typedef union reg_symphony_disp_video_scaler_data_sfifo_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 video_scaler_data_sfifo_thr : 6;
        MT_U32                             : 26;
    } bitc;
} reg_symphony_disp_video_scaler_data_sfifo_thr_t;

/*!
  the union of register reg_symphony_disp_di_lout_afifo_thr
  */
typedef union reg_symphony_disp_di_lout_afifo_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 di_lout_afifo_thr           : 5;
        MT_U32                             : 27;
    } bitc;
} reg_symphony_disp_di_lout_afifo_thr_t;

/*!
  the union of register reg_symphony_disp_vscaler_axi_cmd_sfifo_thr
  */
typedef union reg_symphony_disp_vscaler_axi_cmd_sfifo_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 vscaler_axi_cmd_sfifo_thr   : 4;
        MT_U32                             : 28;
    } bitc;
} reg_symphony_disp_vscaler_axi_cmd_sfifo_thr_t;

/*!
  the union of register reg_symphony_disp_vscaler_axi_req_sfifo_thr
  */
typedef union reg_symphony_disp_vscaler_axi_req_sfifo_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 vscaler_axi_req_sfifo_thr   : 5;
        MT_U32                             : 27;
    } bitc;
} reg_symphony_disp_vscaler_axi_req_sfifo_thr_t;

/*!
  the union of register reg_symphony_disp_di_pdd_noise_thr
  */
typedef union reg_symphony_disp_di_pdd_noise_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 di_pdd_noise_thr            : 8;
        MT_U32                             : 24;
    } bitc;
} reg_symphony_disp_di_pdd_noise_thr_t;

/*!
  the union of register reg_symphony_disp_di_acc_result_odd
  */
typedef union reg_symphony_disp_di_acc_result_odd
{
    MT_U32 all;
    struct
    {
        MT_U32 di_acc_result_odd           : 28;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_di_acc_result_odd_t;

/*!
  the union of register reg_symphony_disp_di_acc_result_even
  */
typedef union reg_symphony_disp_di_acc_result_even
{
    MT_U32 all;
    struct
    {
        MT_U32 di_acc_result_even          : 28;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_di_acc_result_even_t;

/*!
  the union of register reg_symphony_disp_di_pause_en
  */
typedef union reg_symphony_disp_di_pause_en
{
    MT_U32 all;
    struct
    {
        MT_U32 di_pause_en                 : 1;
        MT_U32                             : 3;
        MT_U32 di_pause_bot_field_flag     : 1;
        MT_U32                             : 3;
        MT_U32 di_pause_top_field          : 1;
        MT_U32                             : 23;
    } bitc;
} reg_symphony_disp_di_pause_en_t;

/*!
  the union of register reg_symphony_disp_di_alpha_para
  */
typedef union reg_symphony_disp_di_alpha_para
{
    MT_U32 all;
    struct
    {
        MT_U32 g_alpha_0_min               : 8;
        MT_U32 g_alpha_0_max               : 8;
        MT_U32 luma_diff_k                 : 8;
        MT_U32 diff_sel                    : 1;
        MT_U32                             : 3;
        MT_U32 new_alpha_en                : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_di_alpha_para_t;

/*!
  the union of register reg_symphony_disp_di_motion_ctrl_1
  */
typedef union reg_symphony_disp_di_motion_ctrl_1
{
    MT_U32 all;
    struct
    {
        MT_U32 motion_propa_type           : 1;
        MT_U32                             : 7;
        MT_U32 motion_damping2             : 4;
        MT_U32 motion_damping1             : 4;
        MT_U32 medrsp_thr                  : 8;
        MT_U32 difdamping                  : 8;
    } bitc;
} reg_symphony_disp_di_motion_ctrl_1_t;

/*!
  the union of register reg_symphony_disp_di_motion_ctrl_2
  */
typedef union reg_symphony_disp_di_motion_ctrl_2
{
    MT_U32 all;
    struct
    {
        MT_U32 half_motion_en              : 1;
        MT_U32                             : 7;
        MT_U32 motion_data_mode            : 1;
        MT_U32                             : 7;
        MT_U32 motion_estmethod            : 1;
        MT_U32 l0l2_motion_mode            : 1;
        MT_U32                             : 6;
        MT_U32 ip_smallmotion              : 2;
        MT_U32 ip_average                  : 2;
        MT_U32 ip_l0_or_l2                 : 2;
        MT_U32                             : 2;
    } bitc;
} reg_symphony_disp_di_motion_ctrl_2_t;

/*!
  the union of register reg_symphony_disp_di_hevc_flag
  */
typedef union reg_symphony_disp_di_hevc_flag
{
    MT_U32 all;
    struct
    {
        MT_U32 nxt_hevc_flag2              : 1;
        MT_U32                             : 3;
        MT_U32 cur_hevc_flag2              : 1;
        MT_U32                             : 3;
        MT_U32 pre_hevc_flag2              : 1;
        MT_U32                             : 3;
        MT_U32 ppre_hevc_flag2             : 1;
        MT_U32                             : 3;
        MT_U32 nxt_hevc_flag               : 1;
        MT_U32                             : 3;
        MT_U32 cur_hevc_flag               : 1;
        MT_U32                             : 3;
        MT_U32 pre_hevc_flag               : 1;
        MT_U32                             : 3;
        MT_U32 ppre_hevc_flag              : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_di_hevc_flag_t;

/*!
  the union of register reg_symphony_disp_none_di_fields_flag
  */
typedef union reg_symphony_disp_none_di_fields_flag
{
    MT_U32 all;
    struct
    {
        MT_U32 cur_bot_field_flag_2        : 1;
        MT_U32                             : 3;
        MT_U32 cur_top_field_flag_2        : 1;
        MT_U32                             : 3;
        MT_U32 cur_bot_field_flag_0        : 1;
        MT_U32                             : 3;
        MT_U32 cur_top_field_flag_0        : 1;
        MT_U32                             : 19;
    } bitc;
} reg_symphony_disp_none_di_fields_flag_t;

/*!
  the union of register reg_symphony_disp_none_di_hevc_flag
  */
typedef union reg_symphony_disp_none_di_hevc_flag
{
    MT_U32 all;
    struct
    {
        MT_U32 cur_bot_hevc_flag_2         : 1;
        MT_U32                             : 3;
        MT_U32 cur_top_hevc_flag_2         : 1;
        MT_U32                             : 3;
        MT_U32 cur_bot_hevc_flag_0         : 1;
        MT_U32                             : 3;
        MT_U32 cur_top_hevc_flag_0         : 1;
        MT_U32                             : 19;
    } bitc;
} reg_symphony_disp_none_di_hevc_flag_t;

/*!
  the union of register reg_symphony_disp_none_di_progressive_flag
  */
typedef union reg_symphony_disp_none_di_progressive_flag
{
    MT_U32 all;
    struct
    {
        MT_U32 progressive_flag_2          : 1;
        MT_U32                             : 3;
        MT_U32 progressive_flag_0          : 1;
        MT_U32                             : 3;
        MT_U32 only_use_set_0_en           : 1;
        MT_U32                             : 23;
    } bitc;
} reg_symphony_disp_none_di_progressive_flag_t;

/*!
  the union of register reg_symphony_disp_di_motion_ctrl_3
  */
typedef union reg_symphony_disp_di_motion_ctrl_3
{
    MT_U32 all;
    struct
    {
        MT_U32 new_algorithm_en            : 1;
        MT_U32                             : 3;
        MT_U32 small_motion_magnify        : 1;
        MT_U32                             : 3;
        MT_U32 small_motion_thr2           : 8;
        MT_U32 small_motion_thr1           : 8;
        MT_U32 medrsp7dir_thr              : 2;
        MT_U32                             : 6;
    } bitc;
} reg_symphony_disp_di_motion_ctrl_3_t;

/*!
  the union of register reg_symphony_disp_di_motion_ctrl_4
  */
typedef union reg_symphony_disp_di_motion_ctrl_4
{
    MT_U32 all;
    struct
    {
        MT_U32 uv_motion_gain              : 8;
        MT_U32 ip_l0andl2                  : 2;
        MT_U32 ip_l1_difthr                : 2;
        MT_U32                             : 4;
        MT_U32 preserve_all_motion         : 1;
        MT_U32 preserve_small_motion       : 1;
        MT_U32                             : 6;
        MT_U32 motion_magnify              : 4;
        MT_U32                             : 4;
    } bitc;
} reg_symphony_disp_di_motion_ctrl_4_t;

/*!
  the union of register reg_symphony_disp_di_chroma_para
  */
typedef union reg_symphony_disp_di_chroma_para
{
    MT_U32 all;
    struct
    {
        MT_U32 g_alpha_k_chroma            : 8;
        MT_U32 g_alpha_0_chroma            : 8;
        MT_U32                             : 16;
    } bitc;
} reg_symphony_disp_di_chroma_para_t;

/*!
  the union of register reg_symphony_disp_video_burst_length_sel
  */
#if defined CONFIG_MT_CHIP_SYMPHONY1 && defined CONFIG_MT_CHIP_SYMPHONY2
typedef union reg_symphony_disp_video_burst_length_sel
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_burst_length            : 1;
        MT_U32                             : 3;
        MT_U32 vid_rd_stride_sel           : 2;
        MT_U32                             : 2;
        MT_U32 vid_linear_addr_en          : 1;
        MT_U32                             : 7;
        MT_U32 v_half_en                   : 1;
        MT_U32 h_half_en                   : 1;
        MT_U32                             : 14;
    } bitc;
} reg_symphony_disp_video_burst_length_sel_t;
#else
typedef union reg_symphony_disp_video_burst_length_sel
{
    MT_U32 all;
    struct
    {
        MT_U32 vid_burst_length            : 1;
        MT_U32 vid_rd_stride_sel           : 5;
        MT_U32                             : 2;
        MT_U32 vid_linear_addr_en        : 1;
        MT_U32                             : 12;
        MT_U32 mot_rd_stride_sel          : 5;
        MT_U32                             : 2;
        MT_U32 mot_linear_addr_en       : 1;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_video_burst_length_sel_t;
#endif

/*!
  the union of register reg_symphony_disp_video_burst_info
  */
typedef union reg_symphony_disp_video_burst_info
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 16;
        MT_U32 last_burst_length           : 4;
        MT_U32 first_burst_length          : 4;
        MT_U32 line_rd_max                 : 6;
        MT_U32 burst_info_en               : 1;
        MT_U32                             : 1;
    } bitc;
} reg_symphony_disp_video_burst_info_t;

/*!
  the union of register reg_symphony_disp_video_line_rd_cnt_max
  */
typedef union reg_symphony_disp_video_line_rd_cnt_max
{
    MT_U32 all;
    struct
    {
        MT_U32 line_cnt_max                : 8;
        MT_U32                             : 24;
    } bitc;
} reg_symphony_disp_video_line_rd_cnt_max_t;

/*!
  the union of register reg_symphony_disp_prescale_cmd
  */
typedef union reg_symphony_disp_prescale_cmd
{
    MT_U32 all;
    struct
    {
        MT_U32 prescale_en                 : 1;
        MT_U32                             : 3;
        MT_U32 prescale_field_mode         : 1;
        MT_U32                             : 3;
        MT_U32 prescale_hscale_mode        : 2;
        MT_U32                             : 2;
        MT_U32 prescale_vscale_mode        : 2;
        MT_U32                             : 2;
        MT_U32 prescale_wr_stride_sel      : 2;
        MT_U32                             : 14;
    } bitc;
} reg_symphony_disp_prescale_cmd_t;

/*!
  the union of register reg_symphony_disp_prescale_frame_size
  */
typedef union reg_symphony_disp_prescale_frame_size
{
    MT_U32 all;
    struct
    {
        MT_U32 prescale_frame_height       : 11;
        MT_U32                             : 5;
        MT_U32 prescale_frame_width        : 11;
        MT_U32                             : 5;
    } bitc;
} reg_symphony_disp_prescale_frame_size_t;

/*!
  the union of register reg_symphony_disp_prescale_tiling_params
  */
typedef union reg_symphony_disp_prescale_tiling_params
{
    MT_U32 all;
    struct
    {
        MT_U32 prescale_field_pic          : 1;
        MT_U32                             : 3;
        MT_U32 prescale_col_size_mode      : 2;
        MT_U32                             : 2;
        MT_U32 prescale_hd_map_mode        : 1;
        MT_U32                             : 23;
    } bitc;
} reg_symphony_disp_prescale_tiling_params_t;

/*!
  the union of register reg_symphony_disp_prescale_luma_rd_addr
  */
typedef union reg_symphony_disp_prescale_luma_rd_addr
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 luma_rd_addr                : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_luma_rd_addr_t;

/*!
  the union of register reg_symphony_disp_prescale_luma_rd_addr2
  */
typedef union reg_symphony_disp_prescale_luma_rd_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 luma_rd_addr2               : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_luma_rd_addr2_t;

/*!
  the union of register reg_symphony_disp_prescale_luma_wr_addr
  */
typedef union reg_symphony_disp_prescale_luma_wr_addr
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 luma_wr_addr                : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_luma_wr_addr_t;

/*!
  the union of register reg_symphony_disp_prescale_luma_wr_addr2
  */
typedef union reg_symphony_disp_prescale_luma_wr_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 luma_wr_addr2               : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_luma_wr_addr2_t;

/*!
  the union of register reg_symphony_disp_prescale_chroma_rd_addr
  */
typedef union reg_symphony_disp_prescale_chroma_rd_addr
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 chroma_rd_addr              : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_chroma_rd_addr_t;

/*!
  the union of register reg_symphony_disp_prescale_chroma_rd_addr2
  */
typedef union reg_symphony_disp_prescale_chroma_rd_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 chroma_rd_addr2             : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_chroma_rd_addr2_t;

/*!
  the union of register reg_symphony_disp_prescale_chroma_wr_addr
  */
typedef union reg_symphony_disp_prescale_chroma_wr_addr
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 chroma_wr_addr              : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_chroma_wr_addr_t;

/*!
  the union of register reg_symphony_disp_prescale_chroma_wr_addr2
  */
typedef union reg_symphony_disp_prescale_chroma_wr_addr2
{
    MT_U32 all;
    struct
    {
        MT_U32                             : 3;
        MT_U32 chroma_wr_addr2             : 26;
        MT_U32                             : 3;
    } bitc;
} reg_symphony_disp_prescale_chroma_wr_addr2_t;

/*!
  the union of register reg_symphony_disp_prescale_fifo_thr
  */
typedef union reg_symphony_disp_prescale_fifo_thr
{
    MT_U32 all;
    struct
    {
        MT_U32 wr_fifo_thr                 : 5;
        MT_U32                             : 3;
        MT_U32 proc_mode_sfifo_thr         : 5;
        MT_U32                             : 3;
        MT_U32 req_sfifo_thr               : 5;
        MT_U32                             : 3;
        MT_U32 block_rate_cnt_max          : 8;
    } bitc;
} reg_symphony_disp_prescale_fifo_thr_t;

/*!
  the union of register reg_symphony_disp_hd_hue_adjust
  */
typedef union reg_symphony_disp_hd_hue_adjust
{
    MT_U32 all;
    struct
    {
        MT_U32 hd_sina                     : 11;
        MT_U32                             : 5;
        MT_U32 hd_cosa                     : 11;
        MT_U32                             : 4;
        MT_U32 hd_hue_en                   : 1;
    } bitc;
} reg_symphony_disp_hd_hue_adjust_t;

/*!
  the union of register reg_symphony_disp_sd_hue_adjust
  */
typedef union reg_symphony_disp_sd_hue_adjust
{
    MT_U32 all;
    struct
    {
        MT_U32 sd_sina                     : 11;
        MT_U32                             : 5;
        MT_U32 sd_cosa                     : 11;
        MT_U32                             : 4;
        MT_U32 sd_hue_en                   : 1;
    } bitc;
} reg_symphony_disp_sd_hue_adjust_t;

/*!
  the union of register reg_symphony_disp_half_scale_ctr
  */
typedef union reg_symphony_disp_half_scale_ctr
{
    MT_U32 all;
    struct
    {
        MT_U32 h_enable                    : 1;
        MT_U32                             : 3;
        MT_U32 v_enable                    : 1;
        MT_U32                             : 27;
    } bitc;
} reg_symphony_disp_half_scale_ctr_t;

/*!
  the union of register reg_symphony_disp_pix_align_ctr
  */
typedef union reg_symphony_disp_pix_align_ctr
{
    MT_U32 all;
    struct
    {
        MT_U32 last_cut_pix_num            : 4;
        MT_U32                             : 4;
        MT_U32 pre_cut_pix_num             : 4;
        MT_U32                             : 20;
    } bitc;
} reg_symphony_disp_pix_align_ctr_t;

/*!
  the union of register reg_symphony_disp_disp_prot_limit
  */
typedef union reg_symphony_disp_disp_prot_limit
{
    MT_U32 all;
    struct
    {
        MT_U32 back_prot_en                : 1;
        MT_U32 still_prot_en               : 1;
        MT_U32 vid_prot_en                 : 1;
        MT_U32 osd0_rpot_en                : 1;
        MT_U32 osd1_prot_en                : 1;
        MT_U32 sub_prot_en                 : 1;
        MT_U32                             : 26;
    } bitc;
} reg_symphony_disp_disp_prot_limit_t;

/*!
  the union of register reg_symphony_hdvenc_mask_riq
  */
typedef union reg_symphony_hdvenc_mask_riq
{
    MT_U32 all;
    struct
    {
        MT_U32 line_cnt                : 12;
        MT_U32 cable_dct_fall_dac0_irq               : 1;
        MT_U32 cable_dct_rise_dac0_irq               : 1;
        MT_U32 cable_dct_fall_dac1_irq               : 1;
        MT_U32 cable_dct_rise_dac1_irq               : 1;
        MT_U32 bot_field_irq                 : 1;
        MT_U32 top_field_irq                 : 1;
        MT_U32 bot_start_irq                 : 1;
        MT_U32 top_start_irq                 : 1;
        MT_U32 cable_dct_fall_dac2_irq               : 1;
        MT_U32 cable_dct_rise_dac2_irq               : 1;
        MT_U32 cable_dct_fall_dac3_irq               : 1;
        MT_U32 cable_dct_rise_dac3_irq               : 1;
        MT_U32                             : 8;
    } bitc;
} reg_symphony_hdvenc_mask_riq_t;

typedef struct reg_symphony_disp_chroma_lut_addr
{
  MT_U32 all;
}reg_symphony_disp_chroma_lut_addr_t;


typedef struct reg_symphony_disp_luma_lut_addr
{
  MT_U32 all;
}reg_symphony_disp_luma_lut_addr_t;

typedef union REG_SYMPHONY_DISP_DECOMP_CTRL_0_REG
{
  MT_U32 all;
  struct
  {
    MT_U32 en : 1;
    MT_U32 decomp_4k_rd_proc_en : 1;
    MT_U32 reserved : 2;
    MT_U32 bit_depth_luma : 2;
    MT_U32 bit_depth_chroma : 2;
    MT_U32 : 24;
  }bitc;
}reg_symphony_disp_decomp_ctrl_0_t;

typedef union reg_symphony_disp_decomp_1bgs_str
{
  MT_U32 all;
  struct
  {
    MT_U32 stride_data_luma : 16;
    MT_U32 stride_data_chroma : 16;
  }bitc;
}reg_symphony_disp_decomp_1bgs_str_t;

typedef union reg_symphony_disp_decomp_pic_resl
{
  MT_U32 all;
  struct
  {
    MT_U32 pic_width : 13;
    MT_U32 res : 3;
    MT_U32 pic_height : 12;
    MT_U32 : 4;
  }bitc;
}reg_symphony_disp_decomp_pic_resl_t;

typedef union reg_symphony_disp_clock_gate_ctrl
{
  MT_U32 all;
  struct
  {
    MT_U32 vid_layer_clk_gate_en : 1;
    MT_U32 res : 15;
    MT_U32 vid_decomp_clk_gate_en : 2;
    MT_U32 res2 : 7;
    MT_U32 sd_vid_scaler_clk_gate_en: 1;
    MT_U32 : 6;
  }bitc;
}reg_symphony_disp_clock_gate_ctrl_t;

typedef union reg_symphony_disp_new_csc_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 new_csc_en                  : 1;
        MT_U32                             : 3;
        MT_U32 new_csc_bound_output_en     : 1;
        MT_U32                             : 3;
        MT_U32 new_csc_bound_input_en      : 1;
        MT_U32                             : 3;
        MT_U32 new_csc_shift_en            : 1;

#ifdef CONFIG_MT_CHIP_SYMPHONY2
        MT_U32                             : 3;
        MT_U32 new_csc_or_old_csc          : 1;
        MT_U32 sd_new_csc_or_old_csc_en    : 1;
        MT_U32 new_csc_in_sel              : 1;
        MT_U32                             : 1;
        MT_U32 color_correction_flag       : 1;
        MT_U32                             : 11;
#else
        MT_U32                             : 4;
        MT_U32 new_csc_or_old_csc          : 1;
        MT_U32                             : 1;
        MT_U32 color_correction_flag       : 1;
        MT_U32 new_csc_factor              : 12;
#endif
    } bitc;
} reg_symphony_disp_new_csc_ctrl_t;

typedef union reg_symphony_disp_osdm_csc_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 enable                      : 1;
        MT_U32 bound_input_en              : 1;
        MT_U32 bound_output_en             : 1;
        MT_U32                             : 29;
    } bitc;
} reg_symphony_disp_osdm_csc_ctrl_t;

typedef union reg_symphony_disp_sub_csc_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 enable                      : 1;
        MT_U32 bound_input_en              : 1;
        MT_U32 bound_output_en             : 1;
        MT_U32                             : 29;
    } bitc;
} reg_symphony_disp_sub_csc_ctrl_t;

typedef union reg_symphony_disp_adv_csc_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 adv_csc_en                  : 1;
        MT_U32                             : 3;
        MT_U32 adv_csc_bound_output_en     : 1;
        MT_U32 rgb_in                      : 1;
        MT_U32                             : 2;
        MT_U32 adv_csc_bound_input_en      : 1;
        MT_U32                             : 3;
        MT_U32 adv_csc_shift_en            : 1;
        MT_U32                             : 3;
        MT_U32 adv_csc_or_old_csc_en       : 1;
        MT_U32 sd_adv_csc_or_old_csc_en    : 1;
        MT_U32 adv_csc_in_sel              : 2;
        MT_U32 osd_adv_csc_or_new_csc_en   : 1;
        MT_U32 osd_alpha_div_en            : 1;
        MT_U32                             : 10;
    } bitc;
} reg_symphony_disp_adv_csc_ctrl_t;

typedef union reg_symphony_disp_osd_new_csc_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 new_csc_en                  : 1;
        MT_U32                             : 3;
        MT_U32 new_csc_bound_output_en     : 1;
        MT_U32                             : 3;
        MT_U32 new_csc_bound_input_en      : 1;
        MT_U32                             : 3;
        MT_U32 new_csc_shift_en            : 1;
        MT_U32                             : 6;
        MT_U32 color_correction_flag       : 1;
        MT_U32 osd_new_csc_factor          : 12;
    } bitc;
} reg_symphony_disp_osd_new_csc_ctrl_t;

typedef union reg_symphony_disp_new_csc_gain_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 pix_gain_sat_en             : 1;
        MT_U32                             : 7;
        MT_U32 sat_value                   : 9;
        MT_U32                             : 15;
    } bitc;
} reg_symphony_disp_new_csc_gain_ctrl_t;

typedef union reg_symphony_disp_osd_new_csc_gain_ctrl
{
    MT_U32 all;
    struct
    {
        MT_U32 pix_gain_sat_en             : 1;
        MT_U32                             : 7;
        MT_U32 sat_value                   : 9;
        MT_U32                             : 15;
    } bitc;
} reg_symphony_disp_osd_new_csc_gain_ctrl_t;

#ifdef __cplusplus
extern "C" {
#endif

/*!
  register REGSYMPHONY_DISP_DISPLAY_CTRL (read/write)
  */
void reg_symphony_disp_set_display_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_display_ctrl(void);
void reg_symphony_disp_set_display_ctrl_hd_hf_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_hd_hf_sel(void);
void reg_symphony_disp_set_display_ctrl_sd_hf_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_sd_hf_sel(void);
void reg_symphony_disp_set_display_ctrl_hd_cut_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_hd_cut_en(void);
void reg_symphony_disp_set_display_ctrl_sd_cut_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_sd_cut_en(void);
void reg_symphony_disp_set_display_ctrl_hd_vf_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_hd_vf_sel(void);
void reg_symphony_disp_set_display_ctrl_sd_vf_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_sd_vf_sel(void);
void reg_symphony_disp_set_display_ctrl_disp_fw_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_disp_fw_mode(void);
void reg_symphony_disp_set_display_ctrl_vf_in_frame(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_vf_in_frame(void);
void reg_symphony_disp_set_display_ctrl_vid_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_vid_sel(void);
void reg_symphony_disp_set_display_ctrl_reg_latch_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_reg_latch_en(void);
void reg_symphony_disp_set_display_ctrl_reg_latch_timing(MT_U8 data);
MT_U8   reg_symphony_disp_get_display_ctrl_reg_latch_timing(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_RATIO_HD (read/write)
  */
void reg_symphony_disp_set_vscaler_ratio_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_ratio_hd(void);
void reg_symphony_disp_set_vscaler_ratio_hd_hratio_int_hd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_hd_hratio_int_hd(void);
void reg_symphony_disp_set_vscaler_ratio_hd_hratio_fra_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_hd_hratio_fra_hd(void);
void reg_symphony_disp_set_vscaler_ratio_hd_vratio_int_hd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_hd_vratio_int_hd(void);
void reg_symphony_disp_set_vscaler_ratio_hd_vratio_fra_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_hd_vratio_fra_hd(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_RATIO_INIT_HD (read/write)
  */
void reg_symphony_disp_set_vscaler_ratio_init_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_ratio_init_hd(void);
void reg_symphony_disp_set_vscaler_ratio_init_hd_top_int_hd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_init_hd_top_int_hd(void);
void reg_symphony_disp_set_vscaler_ratio_init_hd_top_fra_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_init_hd_top_fra_hd(void);
void reg_symphony_disp_set_vscaler_ratio_init_hd_bot_int_hd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_init_hd_bot_int_hd(void);
void reg_symphony_disp_set_vscaler_ratio_init_hd_bot_fra_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_init_hd_bot_fra_hd(void);

/*!
  register REGSYMPHONY_DISP_VID_DISP_FIELD (read/write)
  */
void reg_symphony_disp_set_vid_disp_field(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_disp_field(void);
void reg_symphony_disp_set_vid_disp_field_sd_hf_phase(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_disp_field_sd_hf_phase(void);
void reg_symphony_disp_set_vid_disp_field_hd_hf_tapnum(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_disp_field_hd_hf_tapnum(void);
void reg_symphony_disp_set_vid_disp_field_hd_hf_phase(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_disp_field_hd_hf_phase(void);
void reg_symphony_disp_set_vid_disp_field_vid_field(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_disp_field_vid_field(void);

/*!
  register REGSYMPHONY_DISP_VID_WINDOW_CUT_SD (read/write)
  */
void reg_symphony_disp_set_vid_window_cut_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_window_cut_sd(void);
void reg_symphony_disp_set_vid_window_cut_sd_left_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_sd_left_cut(void);
void reg_symphony_disp_set_vid_window_cut_sd_right_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_sd_right_cut(void);
void reg_symphony_disp_set_vid_window_cut_sd_top_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_sd_top_cut(void);
void reg_symphony_disp_set_vid_window_cut_sd_bot_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_sd_bot_cut(void);

/*!
  register REGSYMPHONY_DISP_VID_WINDOW_X_HD (read/write)
  */
void reg_symphony_disp_set_vid_window_x_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_window_x_hd(void);
void reg_symphony_disp_set_vid_window_x_hd_vid_x_left_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_x_hd_vid_x_left_hd(void);
void reg_symphony_disp_set_vid_window_x_hd_vid_x_right_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_x_hd_vid_x_right_hd(void);

/*!
  register REGSYMPHONY_DISP_VID_WINDOW_Y_HD (read/write)
  */
void reg_symphony_disp_set_vid_window_y_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_window_y_hd(void);
void reg_symphony_disp_set_vid_window_y_hd_vid_y_start_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_y_hd_vid_y_start_hd(void);
void reg_symphony_disp_set_vid_window_y_hd_vid_y_end_hd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_y_hd_vid_y_end_hd(void);

/*!
  register REGSYMPHONY_DISP_VID_WINDOW_CUT_HD (read/write)
  */
void reg_symphony_disp_set_vid_window_cut_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_window_cut_hd(void);
void reg_symphony_disp_set_vid_window_cut_hd_left_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_hd_left_cut(void);
void reg_symphony_disp_set_vid_window_cut_hd_right_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_hd_right_cut(void);
void reg_symphony_disp_set_vid_window_cut_hd_top_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_hd_top_cut(void);
void reg_symphony_disp_set_vid_window_cut_hd_bot_cut(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_window_cut_hd_bot_cut(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_RATIO_SD (read/write)
  */
void reg_symphony_disp_set_vscaler_ratio_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_ratio_sd(void);
void reg_symphony_disp_set_vscaler_ratio_sd_hratio_int_sd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_sd_hratio_int_sd(void);
void reg_symphony_disp_set_vscaler_ratio_sd_hratio_fra_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_sd_hratio_fra_sd(void);
void reg_symphony_disp_set_vscaler_ratio_sd_vratio_int_sd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_sd_vratio_int_sd(void);
void reg_symphony_disp_set_vscaler_ratio_sd_vratio_fra_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_sd_vratio_fra_sd(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_RATIO_INIT_SD (read/write)
  */
void reg_symphony_disp_set_vscaler_ratio_init_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_ratio_init_sd(void);
void reg_symphony_disp_set_vscaler_ratio_init_sd_top_int_sd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_init_sd_top_int_sd(void);
void reg_symphony_disp_set_vscaler_ratio_init_sd_top_fra_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_init_sd_top_fra_sd(void);
void reg_symphony_disp_set_vscaler_ratio_init_sd_bot_int_sd(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_ratio_init_sd_bot_int_sd(void);
void reg_symphony_disp_set_vscaler_ratio_init_sd_bot_fra_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vscaler_ratio_init_sd_bot_fra_sd(void);

/*!
  register REGSYMPHONY_DISP_VID_WINDOW_X_SD (read/write)
  */
void reg_symphony_disp_set_vid_window_x_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_window_x_sd(void);
void reg_symphony_disp_set_vid_window_x_sd_vid_x_left_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_x_sd_vid_x_left_sd(void);
void reg_symphony_disp_set_vid_window_x_sd_vid_x_right_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_x_sd_vid_x_right_sd(void);

/*!
  register REGSYMPHONY_DISP_VID_WINDOW_Y_SD (read/write)
  */
void reg_symphony_disp_set_vid_window_y_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_window_y_sd(void);
void reg_symphony_disp_set_vid_window_y_sd_vid_y_start_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_y_sd_vid_y_start_sd(void);
void reg_symphony_disp_set_vid_window_y_sd_vid_y_end_sd(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_window_y_sd_vid_y_end_sd(void);

/*!
  register REGSYMPHONY_DISP_GRAPHIC_CTRL (read/write)
  */
void reg_symphony_disp_set_graphic_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_graphic_ctrl(void);
void reg_symphony_disp_set_graphic_ctrl_back_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_graphic_ctrl_back_sel(void);
void reg_symphony_disp_set_graphic_ctrl_still_sel_sd(MT_U8 data);
MT_U8   reg_symphony_disp_get_graphic_ctrl_still_sel_sd(void);
void reg_symphony_disp_set_graphic_ctrl_still_sd_format_444(MT_U8 data);
MT_U8   reg_symphony_disp_get_graphic_ctrl_still_sd_format_444(void);
void reg_symphony_disp_set_graphic_ctrl_still_sel_hd(MT_U8 data);
MT_U8   reg_symphony_disp_get_graphic_ctrl_still_sel_hd(void);
void reg_symphony_disp_set_graphic_ctrl_still_hd_format(MT_U8 data);
MT_U8   reg_symphony_disp_get_graphic_ctrl_still_hd_format(void);
void reg_symphony_disp_set_graphic_ctrl_mix_layer_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_graphic_ctrl_mix_layer_mode(void);

/*!
  register REGSYMPHONY_DISP_BG_COLOR (read/write)
  */
void reg_symphony_disp_set_bg_color(MT_U32 data);
MT_U32  reg_symphony_disp_get_bg_color(void);
void reg_symphony_disp_set_bg_color_bg_cr(MT_U8 data);
MT_U8   reg_symphony_disp_get_bg_color_bg_cr(void);
void reg_symphony_disp_set_bg_color_bg_cb(MT_U8 data);
MT_U8   reg_symphony_disp_get_bg_color_bg_cb(void);
void reg_symphony_disp_set_bg_color_bg_y(MT_U8 data);
MT_U8   reg_symphony_disp_get_bg_color_bg_y(void);

/*!
  register REGSYMPHONY_DISP_STILL_X_HD (read/write)
  */
void reg_symphony_disp_set_still_x_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_x_hd(void);
void reg_symphony_disp_set_still_x_hd_hd_still_startx(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_x_hd_hd_still_startx(void);
void reg_symphony_disp_set_still_x_hd_hd_still_endx(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_x_hd_hd_still_endx(void);

/*!
  register REGSYMPHONY_DISP_STILL_Y_HD (read/write)
  */
void reg_symphony_disp_set_still_y_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_y_hd(void);
void reg_symphony_disp_set_still_y_hd_hd_still_starty(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_y_hd_hd_still_starty(void);
void reg_symphony_disp_set_still_y_hd_hd_still_endy(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_y_hd_hd_still_endy(void);

/*!
  register REGSYMPHONY_DISP_OSD0_CMD_HD (read/write)
  */
void reg_symphony_disp_set_osd0_cmd_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd0_cmd_hd(void);
void reg_symphony_disp_set_osd0_cmd_hd_osd0_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_cmd_hd_osd0_sel(void);
void reg_symphony_disp_set_osd0_cmd_hd_force_progressive(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_cmd_hd_force_progressive(void);
void reg_symphony_disp_set_osd0_cmd_hd_plane_alpha_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_cmd_hd_plane_alpha_en(void);
void reg_symphony_disp_set_osd0_cmd_hd_plane_alpha(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_cmd_hd_plane_alpha(void);
void reg_symphony_disp_set_osd0_cmd_hd_disable_hori_process(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_cmd_hd_disable_hori_process(void);
void reg_symphony_disp_set_osd0_cmd_hd_osd0_big_endian(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_cmd_hd_osd0_big_endian(void);

/*!
  register REGSYMPHONY_DISP_SUB_CMD_HD (read/write)
  */
void reg_symphony_disp_set_sub_cmd_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_cmd_hd(void);
void reg_symphony_disp_set_sub_cmd_hd_sub_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_cmd_hd_sub_sel(void);
void reg_symphony_disp_set_sub_cmd_hd_force_progressive(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_cmd_hd_force_progressive(void);
void reg_symphony_disp_set_sub_cmd_hd_plane_alpha_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_cmd_hd_plane_alpha_en(void);
void reg_symphony_disp_set_sub_cmd_hd_plane_alpha(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_cmd_hd_plane_alpha(void);
void reg_symphony_disp_set_sub_cmd_hd_disable_hori_process(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_cmd_hd_disable_hori_process(void);
void reg_symphony_disp_set_sub_cmd_hd_sub_big_endian(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_cmd_hd_sub_big_endian(void);

/*!
  register REGSYMPHONY_DISP_LAYER_ALPHA (read/write)
  */
void reg_symphony_disp_set_layer_alpha(MT_U32 data);
MT_U32  reg_symphony_disp_get_layer_alpha(void);
void reg_symphony_disp_set_layer_alpha_vid_plane_alpha(MT_U8 data);
MT_U8   reg_symphony_disp_get_layer_alpha_vid_plane_alpha(void);
void reg_symphony_disp_set_layer_alpha_still_plane_alpha(MT_U8 data);
MT_U8   reg_symphony_disp_get_layer_alpha_still_plane_alpha(void);

/*!
  register REGSYMPHONY_DISP_VID_INPUT_SIZE (read/write)
  */
void reg_symphony_disp_set_vid_input_size(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_input_size(void);
void reg_symphony_disp_set_vid_input_size_vid_frame_h(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_input_size_vid_frame_h(void);
void reg_symphony_disp_set_vid_input_size_vid_frame_w(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_input_size_vid_frame_w(void);

/*!
  register REGSYMPHONY_DISP_VID_SD_DROP_LINE (read/write)
  */
void reg_symphony_disp_set_vid_sd_drop_line(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_sd_drop_line(void);
void reg_symphony_disp_set_vid_sd_drop_line_sd_top_field_drop(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_sd_drop_line_sd_top_field_drop(void);
void reg_symphony_disp_set_vid_sd_drop_line_sd_bot_field_drop(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_sd_drop_line_sd_bot_field_drop(void);

/*!
  register REGSYMPHONY_DISP_VID_CROP_MODE_EN (read/write)
  */
void reg_symphony_disp_set_vid_crop_mode_en(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_crop_mode_en(void);
void reg_symphony_disp_set_vid_crop_mode_en_vid_crop_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_crop_mode_en_vid_crop_en(void);

/*!
  register REGSYMPHONY_DISP_VID_CROP_HORI (read/write)
  */
void reg_symphony_disp_set_vid_crop_hori(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_crop_hori(void);
void reg_symphony_disp_set_vid_crop_hori_vid_crop_endx(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_crop_hori_vid_crop_endx(void);
void reg_symphony_disp_set_vid_crop_hori_vid_crop_startx(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_crop_hori_vid_crop_startx(void);

/*!
  register REGSYMPHONY_DISP_VID_CROP_VERT (read/write)
  */
void reg_symphony_disp_set_vid_crop_vert(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_crop_vert(void);
void reg_symphony_disp_set_vid_crop_vert_vid_crop_endy(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_crop_vert_vid_crop_endy(void);
void reg_symphony_disp_set_vid_crop_vert_vid_crop_starty(MT_U16 data);
MT_U16  reg_symphony_disp_get_vid_crop_vert_vid_crop_starty(void);

/*!
  register REGSYMPHONY_DISP_VID_PROCESS_MODE (read/write)
  */
void reg_symphony_disp_set_vid_process_mode(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_process_mode(void);
void reg_symphony_disp_set_vid_process_mode_di_process(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_process_mode_di_process(void);

/*!
  register REG_SYMPHONY_DISP_DATA_ARRANGE_1 (read/write)
  */
void reg_symphony_disp_set_data_arrange_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_data_arrange_1(void);
void reg_symphony_disp_set_data_arrange_1_data_arrange_1(MT_U8 data);
MT_U8   reg_symphony_disp_get_data_arrange_1_data_arrange_1(void);

/*!
  register SYMPHONY_DISP_data_arrange_2 (read/write)
  */
void reg_symphony_disp_set_data_arrange_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_data_arrange_2(void);
void reg_symphony_disp_set_data_arrange_2_data_arrange_2(MT_U8 data);
MT_U8   reg_symphony_dis2_get_data_arrange_2_data_arrange_2(void);

/*!
  register REGSYMPHONY_DISP_OSD1_CMD_HD (read/write)
  */
void reg_symphony_disp_set_osd1_cmd_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd1_cmd_hd(void);
void reg_symphony_disp_set_osd1_cmd_hd_osd1_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_cmd_hd_osd1_sel(void);
void reg_symphony_disp_set_osd1_cmd_hd_force_progressive(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_cmd_hd_force_progressive(void);
void reg_symphony_disp_set_osd1_cmd_hd_plane_alpha_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_cmd_hd_plane_alpha_en(void);
void reg_symphony_disp_set_osd1_cmd_hd_plane_alpha(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_cmd_hd_plane_alpha(void);
void reg_symphony_disp_set_osd1_cmd_hd_disable_hori_process(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_cmd_hd_disable_hori_process(void);
void reg_symphony_disp_set_osd1_cmd_hd_osd1_big_endian(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_cmd_hd_osd1_big_endian(void);

/*!
  register REGSYMPHONY_DISP_OSD1_CK_HD (read/write)
  */
void reg_symphony_disp_set_osd1_ck_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd1_ck_hd(void);
void reg_symphony_disp_set_osd1_ck_hd_ck_yuv(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd1_ck_hd_ck_yuv(void);
void reg_symphony_disp_set_osd1_ck_hd_ck_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd1_ck_hd_ck_en(void);

/*!
  register REGSYMPHONY_DISP_STILL_X_SD (read/write)
  */
void reg_symphony_disp_set_still_x_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_x_sd(void);
void reg_symphony_disp_set_still_x_sd_sd_still_startx(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_x_sd_sd_still_startx(void);
void reg_symphony_disp_set_still_x_sd_sd_still_endx(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_x_sd_sd_still_endx(void);

/*!
  register REGSYMPHONY_DISP_STILL_Y_SD (read/write)
  */
void reg_symphony_disp_set_still_y_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_y_sd(void);
void reg_symphony_disp_set_still_y_sd_sd_still_starty(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_y_sd_sd_still_starty(void);
void reg_symphony_disp_set_still_y_sd_sd_still_endy(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_y_sd_sd_still_endy(void);

/*!
  register REGSYMPHONY_DISP_STILL_STRIDE_SD (read/write)
  */
void reg_symphony_disp_set_still_stride_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_stride_sd(void);
void reg_symphony_disp_set_still_stride_sd_sd_still_stride(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_stride_sd_sd_still_stride(void);
void reg_symphony_disp_set_still_stride_sd_cr_byte_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_sd_cr_byte_sel(void);
void reg_symphony_disp_set_still_stride_sd_cb_byte_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_sd_cb_byte_sel(void);
void reg_symphony_disp_set_still_stride_sd_y_byte_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_sd_y_byte_sel(void);

/*!
  register REGSYMPHONY_DISP_RGB2Y_COEF (read/write)
  */
void reg_symphony_disp_set_rgb2y_coef(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2y_coef(void);
void reg_symphony_disp_set_rgb2y_coef_r2y_coef(MT_U16 data);
MT_U16  reg_symphony_disp_get_rgb2y_coef_r2y_coef(void);
void reg_symphony_disp_set_rgb2y_coef_g2y_coef(MT_U16 data);
MT_U16  reg_symphony_disp_get_rgb2y_coef_g2y_coef(void);
void reg_symphony_disp_set_rgb2y_coef_b2y_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_rgb2y_coef_b2y_coef(void);

/*!
  register REGSYMPHONY_DISP_RGB2CB_COEF (read/write)
  */
void reg_symphony_disp_set_rgb2cb_coef(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2cb_coef(void);
void reg_symphony_disp_set_rgb2cb_coef_b2cb_coef(MT_U16 data);
MT_U16  reg_symphony_disp_get_rgb2cb_coef_b2cb_coef(void);
void reg_symphony_disp_set_rgb2cb_coef_g2cb_coef(MT_U16 data);
MT_U16  reg_symphony_disp_get_rgb2cb_coef_g2cb_coef(void);
void reg_symphony_disp_set_rgb2cb_coef_r2cb_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_rgb2cb_coef_r2cb_coef(void);

/*!
  register REGSYMPHONY_DISP_RGB2CR_COEF (read/write)
  */
void reg_symphony_disp_set_rgb2cr_coef(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2cr_coef(void);
void reg_symphony_disp_set_rgb2cr_coef_r2cr_coef(MT_U16 data);
MT_U16  reg_symphony_disp_get_rgb2cr_coef_r2cr_coef(void);
void reg_symphony_disp_set_rgb2cr_coef_g2cr_coef(MT_U16 data);
MT_U16  reg_symphony_disp_get_rgb2cr_coef_g2cr_coef(void);
void reg_symphony_disp_set_rgb2cr_coef_b2cr_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_rgb2cr_coef_b2cr_coef(void);

/*!
  register REGSYMPHONY_DISP_VID_DECOMP_CFG (read/write)
  */
void reg_symphony_disp_set_vid_decomp_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_decomp_cfg(void);
void reg_symphony_disp_set_vid_decomp_cfg_config(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_decomp_cfg_config(void);

/*!
  register REGSYMPHONY_DISP_OSD0_CK_HD (read/write)
  */
void reg_symphony_disp_set_osd0_ck_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd0_ck_hd(void);
void reg_symphony_disp_set_osd0_ck_hd_ck_yuv(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd0_ck_hd_ck_yuv(void);
void reg_symphony_disp_set_osd0_ck_hd_ck_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd0_ck_hd_ck_en(void);

/*!
  register REGSYMPHONY_DISP_RGB2YUV_YOFFSET (read/write)
  */
void reg_symphony_disp_set_rgb2yuv_yoffset(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2yuv_yoffset(void);
void reg_symphony_disp_set_rgb2yuv_yoffset_yoffset(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2yuv_yoffset_yoffset(void);

/*!
  register REGSYMPHONY_DISP_RGB2YUV_UVOFFSET (read/write)
  */
void reg_symphony_disp_set_rgb2yuv_uvoffset(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2yuv_uvoffset(void);
void reg_symphony_disp_set_rgb2yuv_uvoffset_uvoffset(MT_U32 data);
MT_U32  reg_symphony_disp_get_rgb2yuv_uvoffset_uvoffset(void);

/*!
  register REGSYMPHONY_DISP_VID_VERF_CFG (read/write)
  */
void reg_symphony_disp_set_vid_verf_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_verf_cfg(void);
void reg_symphony_disp_set_vid_verf_cfg_hd_dce_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_verf_cfg_hd_dce_en(void);
void reg_symphony_disp_set_vid_verf_cfg_sd_dce_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_verf_cfg_sd_dce_en(void);

/*!
  register REGSYMPHONY_DISP_VID_HORF_CFG (read/write)
  */
void reg_symphony_disp_set_vid_horf_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_horf_cfg(void);
void reg_symphony_disp_set_vid_horf_cfg_sd_infl_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_horf_cfg_sd_infl_thr(void);
void reg_symphony_disp_set_vid_horf_cfg_hd_infl_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_vid_horf_cfg_hd_infl_thr(void);

/*!
  register REGSYMPHONY_DISP_LAYER_MIX_CFG (read/write)
  */
void reg_symphony_disp_set_layer_mix_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_layer_mix_cfg(void);
void reg_symphony_disp_set_layer_mix_cfg_bypass_sd_vid_scaler(MT_U8 data);
MT_U8   reg_symphony_disp_get_layer_mix_cfg_bypass_sd_vid_scaler(void);
void reg_symphony_disp_set_layer_mix_cfg_sd_yuv444(MT_U8 data);
MT_U8   reg_symphony_disp_get_layer_mix_cfg_sd_yuv444(void);
void reg_symphony_disp_set_layer_mix_cfg_sd_wr_back_forbidden(MT_U8 data);
MT_U8   reg_symphony_disp_get_layer_mix_cfg_sd_wr_back_forbidden(void);

/*!
  register REGSYMPHONY_DISP_HD_SIZE_OUT (read/write)
  */
void reg_symphony_disp_set_hd_size_out(MT_U32 data);
MT_U32  reg_symphony_disp_get_hd_size_out(void);
void reg_symphony_disp_set_hd_size_out_hd_width_out(MT_U16 data);
MT_U16  reg_symphony_disp_get_hd_size_out_hd_width_out(void);
void reg_symphony_disp_set_hd_size_out_hd_height_out(MT_U16 data);
MT_U16  reg_symphony_disp_get_hd_size_out_hd_height_out(void);

/*!
  register REGSYMPHONY_DISP_SD_SIZE_OUT (read/write)
  */
void reg_symphony_disp_set_sd_size_out(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_size_out(void);
void reg_symphony_disp_set_sd_size_out_sd_width_out(MT_U16 data);
MT_U16  reg_symphony_disp_get_sd_size_out_sd_width_out(void);
void reg_symphony_disp_set_sd_size_out_sd_height_out(MT_U16 data);
MT_U16  reg_symphony_disp_get_sd_size_out_sd_height_out(void);

/*!
  register REGSYMPHONY_DISP_STILL_STRIDE_HD (read/write)
  */
void reg_symphony_disp_set_still_stride_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_stride_hd(void);
void reg_symphony_disp_set_still_stride_hd_hd_still_stride(MT_U16 data);
MT_U16  reg_symphony_disp_get_still_stride_hd_hd_still_stride(void);
void reg_symphony_disp_set_still_endian(MT_U8 data);
MT_U16  reg_symphony_disp_get_still_endian(void);
void reg_symphony_disp_set_still_stride_hd_cr_byte_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_hd_cr_byte_sel(void);
void reg_symphony_disp_set_still_stride_hd_cb_byte_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_hd_cb_byte_sel(void);
void reg_symphony_disp_set_still_stride_hd_y_byte_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_hd_y_byte_sel(void);
void reg_symphony_disp_set_still_stride_hd_cr_first(MT_U8 data);
MT_U8   reg_symphony_disp_get_still_stride_hd_cr_first(void);

/*!
  register REGSYMPHONY_DISP_HDTV_CFG (read/write)
  */
void reg_symphony_disp_set_hdtv_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_hdtv_cfg(void);
void reg_symphony_disp_set_hdtv_cfg_hd_vid_fmt(MT_U8 data);
MT_U8   reg_symphony_disp_get_hdtv_cfg_hd_vid_fmt(void);

/*!
  register REGSYMPHONY_DISP_HD_POST_CFG (read/write)
  */
void reg_symphony_disp_set_hd_post_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_hd_post_cfg(void);
void reg_symphony_disp_set_hd_post_cfg_hd_leverage(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_post_cfg_hd_leverage(void);
void reg_symphony_disp_set_hd_post_cfg_hd_hp_enha(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_post_cfg_hd_hp_enha(void);
void reg_symphony_disp_set_hd_post_cfg_hd_hori_enha(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_post_cfg_hd_hori_enha(void);
void reg_symphony_disp_set_hd_post_cfg_hd_shoot_chg(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_post_cfg_hd_shoot_chg(void);

/*!
  register REGSYMPHONY_DISP_SD_POST_CFG (read/write)
  */
void reg_symphony_disp_set_sd_post_cfg(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_post_cfg(void);
void reg_symphony_disp_set_sd_post_cfg_sd_leverage(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_post_cfg_sd_leverage(void);
void reg_symphony_disp_set_sd_post_cfg_sd_hp_enha(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_post_cfg_sd_hp_enha(void);
void reg_symphony_disp_set_sd_post_cfg_sd_hori_enha(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_post_cfg_sd_hori_enha(void);
void reg_symphony_disp_set_sd_post_cfg_sd_shoot_chg(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_post_cfg_sd_shoot_chg(void);

/*!
  register REGSYMPHONY_DISP_HD_EFFECT_COEF (read/write)
  */
void reg_symphony_disp_set_hd_effect_coef(MT_U32 data);
MT_U32  reg_symphony_disp_get_hd_effect_coef(void);
void reg_symphony_disp_set_hd_effect_coef_bright_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_effect_coef_bright_coef(void);
void reg_symphony_disp_set_hd_effect_coef_contrast_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_effect_coef_contrast_coef(void);
void reg_symphony_disp_set_hd_effect_coef_saturation_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_effect_coef_saturation_coef(void);

/*!
  register REGSYMPHONY_DISP_SD_EFFECT_COEF (read/write)
  */
void reg_symphony_disp_set_sd_effect_coef(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_effect_coef(void);
void reg_symphony_disp_set_sd_effect_coef_bright_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_effect_coef_bright_coef(void);
void reg_symphony_disp_set_sd_effect_coef_contrast_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_effect_coef_contrast_coef(void);
void reg_symphony_disp_set_sd_effect_coef_saturation_coef(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_effect_coef_saturation_coef(void);

/*!
  register REGSYMPHONY_DISP_GRA_FIFO_THRESHOLD (read/write)
  */
void reg_symphony_disp_set_gra_fifo_threshold(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_fifo_threshold(void);
void reg_symphony_disp_set_gra_fifo_threshold_fifo64_lo(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_fifo_threshold_fifo64_lo(void);
void reg_symphony_disp_set_gra_fifo_threshold_fifo64_hi(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_fifo_threshold_fifo64_hi(void);
void reg_symphony_disp_set_gra_fifo_threshold_fifo128_lo(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_fifo_threshold_fifo128_lo(void);
void reg_symphony_disp_set_gra_fifo_threshold_fifo128_hi(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_fifo_threshold_fifo128_hi(void);

/*!
  register REGSYMPHONY_DISP_GRA_SCALER_CTRL (read/write)
  */
void reg_symphony_disp_set_gra_scaler_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_scaler_ctrl(void);
void reg_symphony_disp_set_gra_scaler_ctrl_hori_filter_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_hori_filter_en(void);
void reg_symphony_disp_set_gra_scaler_ctrl_vert_filter_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_vert_filter_en(void);
void reg_symphony_disp_set_gra_scaler_ctrl_hori_phase_type(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_hori_phase_type(void);
void reg_symphony_disp_set_gra_scaler_ctrl_vert_phase_type(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_vert_phase_type(void);
void reg_symphony_disp_set_gra_scaler_ctrl_vert_table_num(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_vert_table_num(void);
void reg_symphony_disp_set_gra_scaler_ctrl_hori_table_num(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_hori_table_num(void);
void reg_symphony_disp_set_gra_scaler_ctrl_vert_start_line_odd(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_vert_start_line_odd(void);
void reg_symphony_disp_set_gra_scaler_ctrl_vert_start_line_even(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_ctrl_vert_start_line_even(void);

/*!
  register REGSYMPHONY_DISP_GRA_SCALER_HRATIO (read/write)
  */
void reg_symphony_disp_set_gra_scaler_hratio(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_scaler_hratio(void);
void reg_symphony_disp_set_gra_scaler_hratio_hratio_int(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_hratio_hratio_int(void);
void reg_symphony_disp_set_gra_scaler_hratio_hratio_fra(MT_U16 data);
MT_U16  reg_symphony_disp_get_gra_scaler_hratio_hratio_fra(void);

/*!
  register REGSYMPHONY_DISP_GRA_SCALER_VRATIO (read/write)
  */
void reg_symphony_disp_set_gra_scaler_vratio(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_scaler_vratio(void);
void reg_symphony_disp_set_gra_scaler_vratio_vratio_int(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_scaler_vratio_vratio_int(void);
void reg_symphony_disp_set_gra_scaler_vratio_vratio_fra(MT_U16 data);
MT_U16  reg_symphony_disp_get_gra_scaler_vratio_vratio_fra(void);

/*!
  register REGSYMPHONY_DISP_GRA_SCALER_H_START_FRA (read/write)
  */
void reg_symphony_disp_set_gra_scaler_h_start_fra(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_scaler_h_start_fra(void);
void reg_symphony_disp_set_gra_scaler_h_start_fra_h_start_fra(MT_U16 data);
MT_U16  reg_symphony_disp_get_gra_scaler_h_start_fra_h_start_fra(void);

/*!
  register REGSYMPHONY_DISP_GRA_SCALER_V_START_FRA (read/write)
  */
void reg_symphony_disp_set_gra_scaler_v_start_fra(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_scaler_v_start_fra(void);
void reg_symphony_disp_set_gra_scaler_v_start_fra_v_start_fra_odd(MT_U16 data);
MT_U16  reg_symphony_disp_get_gra_scaler_v_start_fra_v_start_fra_odd(void);
void reg_symphony_disp_set_gra_scaler_v_start_fra_v_start_fra_even(MT_U16 data);
MT_U16  reg_symphony_disp_get_gra_scaler_v_start_fra_v_start_fra_even(void);

/*!
  register REGSYMPHONY_DISP_GRA_CTL (read/write)
  */
void reg_symphony_disp_set_gra_ctl(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_ctl(void);
void reg_symphony_disp_set_gra_ctl_hd_prog_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_ctl_hd_prog_mode(void);
void reg_symphony_disp_set_gra_ctl_reduce_fr_osd0(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_ctl_reduce_fr_osd0(void);
void reg_symphony_disp_set_gra_ctl_reduce_fr_osd1(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_ctl_reduce_fr_osd1(void);
void reg_symphony_disp_set_gra_ctl_reduce_fr_sub(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_ctl_reduce_fr_sub(void);
void reg_symphony_disp_set_gra_ctl_reduce_fr_hd_still(MT_U8 data);
MT_U8   reg_symphony_disp_get_gra_ctl_reduce_fr_hd_still(void);

/*!
  register REGSYMPHONY_DISP_OSD_SCALE_HSIZE (read/write)
  */
void reg_symphony_disp_set_osd_scale_hsize(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_scale_hsize(void);
void reg_symphony_disp_set_osd_scale_hsize_osd_dst_hsize(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_scale_hsize_osd_dst_hsize(void);
void reg_symphony_disp_set_osd_scale_hsize_osd_src_hsize(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_scale_hsize_osd_src_hsize(void);

/*!
  register REGSYMPHONY_DISP_OSD_SCALE_RATIO (read/write)
  */
void reg_symphony_disp_set_osd_scale_ratio(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_scale_ratio(void);
void reg_symphony_disp_set_osd_scale_ratio_osd_ratio_fra(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_scale_ratio_osd_ratio_fra(void);
void reg_symphony_disp_set_osd_scale_ratio_osd_ratio_int(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_scale_ratio_osd_ratio_int(void);

/*!
  register REGSYMPHONY_DISP_OSD_ALPHA (read/write)
  */
void reg_symphony_disp_set_osd_alpha(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_alpha(void);
void reg_symphony_disp_set_osd_alpha_osd_alpha_filter(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_alpha_osd_alpha_filter(void);
void reg_symphony_disp_set_osd_alpha_border_cfg(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_alpha_border_cfg(void);
void reg_symphony_disp_set_osd_alpha_osd_start_fra(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_alpha_osd_start_fra(void);
void reg_symphony_disp_set_osd_alpha_osd_no_filter(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_alpha_osd_no_filter(void);
void reg_symphony_disp_set_osd_alpha_osd_sub_mix_first(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_alpha_osd_sub_mix_first(void);

/*!
  register REGSYMPHONY_DISP_OSD_VERT_START_LINE (read/write)
  */
void reg_symphony_disp_set_osd_vert_start_line(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_vert_start_line(void);
void reg_symphony_disp_set_osd_vert_start_line_osd_odd_start_line(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_vert_start_line_osd_odd_start_line(void);
void reg_symphony_disp_set_osd_vert_start_line_osd_even_start_line(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_vert_start_line_osd_even_start_line(void);

/*!
  register REGSYMPHONY_DISP_OSD_VERTICAL_CTRL (read/write)
  */
void reg_symphony_disp_set_osd_vertical_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_vertical_ctrl(void);
void reg_symphony_disp_set_osd_vertical_ctrl_osd_vert_phase_type(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_vertical_ctrl_osd_vert_phase_type(void);
void reg_symphony_disp_set_osd_vertical_ctrl_osd_vert_no_filter(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_vertical_ctrl_osd_vert_no_filter(void);
void reg_symphony_disp_set_osd_vertical_ctrl_osd_vert_bypass(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_vertical_ctrl_osd_vert_bypass(void);

/*!
  register REGSYMPHONY_DISP_OSD_VERTICAL_SIZE (read/write)
  */
void reg_symphony_disp_set_osd_vertical_size(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_vertical_size(void);
void reg_symphony_disp_set_osd_vertical_size_osd_dst_vsize(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_vertical_size_osd_dst_vsize(void);
void reg_symphony_disp_set_osd_vertical_size_osd_ori_vsize(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_vertical_size_osd_ori_vsize(void);

/*!
  register REGSYMPHONY_DISP_OSD_VERTICAL_RATIO (read/write)
  */
void reg_symphony_disp_set_osd_vertical_ratio(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_vertical_ratio(void);
void reg_symphony_disp_set_osd_vertical_ratio_osd_vratio_fra(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_vertical_ratio_osd_vratio_fra(void);
void reg_symphony_disp_set_osd_vertical_ratio_osd_vratio_int(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_vertical_ratio_osd_vratio_int(void);

/*!
  register REGSYMPHONY_DISP_OSD_V_START_FRA (read/write)
  */
void reg_symphony_disp_set_osd_v_start_fra(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_v_start_fra(void);
void reg_symphony_disp_set_osd_v_start_fra_osd_v_start_fra_odd(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_v_start_fra_osd_v_start_fra_odd(void);
void reg_symphony_disp_set_osd_v_start_fra_osd_v_start_fra_even(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_v_start_fra_osd_v_start_fra_even(void);

/*!
  register REGSYMPHONY_DISP_OSD_V_TAP_NUM (read/write)
  */
void reg_symphony_disp_set_osd_v_tap_num(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_v_tap_num(void);
void reg_symphony_disp_set_osd_v_tap_num_tap_num(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_v_tap_num_tap_num(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_COEF0 (read/write)
  */
void reg_symphony_disp_set_chroma_coef0(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_coef0(void);
void reg_symphony_disp_set_chroma_coef0_chroma_hori_coef1(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef0_chroma_hori_coef1(void);
void reg_symphony_disp_set_chroma_coef0_chroma_hori_coef0(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef0_chroma_hori_coef0(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_COEF1 (read/write)
  */
void reg_symphony_disp_set_chroma_coef1(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_coef1(void);
void reg_symphony_disp_set_chroma_coef1_chroma_hori_coef3(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef1_chroma_hori_coef3(void);
void reg_symphony_disp_set_chroma_coef1_chroma_hori_coef2(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef1_chroma_hori_coef2(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_COEF2 (read/write)
  */
void reg_symphony_disp_set_chroma_coef2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_coef2(void);
void reg_symphony_disp_set_chroma_coef2_chroma_hori_coef5(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef2_chroma_hori_coef5(void);
void reg_symphony_disp_set_chroma_coef2_chroma_hori_coef4(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef2_chroma_hori_coef4(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_COEF3 (read/write)
  */
void reg_symphony_disp_set_chroma_coef3(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_coef3(void);
void reg_symphony_disp_set_chroma_coef3_chroma_hori_coef6(MT_U16 data);
MT_U16  reg_symphony_disp_get_chroma_coef3_chroma_hori_coef6(void);

/*!
  register REGSYMPHONY_DISP_SCALE_INIT_PHASE_OFFSET (read/write)
  */
void reg_symphony_disp_set_scale_init_phase_offset(MT_U32 data);
MT_U32  reg_symphony_disp_get_scale_init_phase_offset(void);
void reg_symphony_disp_set_scale_init_phase_offset_offset(MT_U8 data);
MT_U8   reg_symphony_disp_get_scale_init_phase_offset_offset(void);

/*!
   comments
   */
MT_U8   reg_symphony_disp_get_vid_fmt(void);
/*!
   comments
   */
MT_U8   reg_symphony_disp_get_input_ar(void);
/*!
   comments
   */
MT_U8   reg_symphony_disp_get_input_fr(void);

/*!
  register REGSYMPHONY_DISP_SMALL_PIC_UPSCALE_CTRL (read/write)
  */
void reg_symphony_disp_set_small_pic_upscale_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_small_pic_upscale_ctrl(void);
void reg_symphony_disp_set_small_pic_upscale_ctrl_upscale_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_small_pic_upscale_ctrl_upscale_en(void);
void reg_symphony_disp_set_small_pic_upscale_ctrl_chroma_hori_ip_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_small_pic_upscale_ctrl_chroma_hori_ip_mode(void);

/*!
  register REGSYMPHONY_DISP_ALISING_PROB_REG1 (read/write)
  */
void reg_symphony_disp_set_alising_prob_reg1(MT_U32 data);
MT_U32  reg_symphony_disp_get_alising_prob_reg1(void);
void reg_symphony_disp_set_alising_prob_reg1_alpha_2nd_method_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg1_alpha_2nd_method_sel(void);
void reg_symphony_disp_set_alising_prob_reg1_alpha_2nd_diff_ratio_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg1_alpha_2nd_diff_ratio_sel(void);
void reg_symphony_disp_set_alising_prob_reg1_alpha_2nd_diff_shift_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg1_alpha_2nd_diff_shift_sel(void);
void reg_symphony_disp_set_alising_prob_reg1_alpha_2nd_diff(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg1_alpha_2nd_diff(void);

/*!
  register REGSYMPHONY_DISP_ALISING_PROB_REG2 (read/write)
  */
void reg_symphony_disp_set_alising_prob_reg2(MT_U32 data);
MT_U32  reg_symphony_disp_get_alising_prob_reg2(void);
void reg_symphony_disp_set_alising_prob_reg2_alpha_enlarge(MT_U16 data);
MT_U16  reg_symphony_disp_get_alising_prob_reg2_alpha_enlarge(void);
void reg_symphony_disp_set_alising_prob_reg2_alpha_vdv_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg2_alpha_vdv_sel(void);
void reg_symphony_disp_set_alising_prob_reg2_alpha_vdv(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg2_alpha_vdv(void);
void reg_symphony_disp_set_alising_prob_reg2_alpha_angle(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg2_alpha_angle(void);

/*!
  register REGSYMPHONY_DISP_ALISING_PROB_REG3 (read/write)
  */
void reg_symphony_disp_set_alising_prob_reg3(MT_U32 data);
MT_U32  reg_symphony_disp_get_alising_prob_reg3(void);
void reg_symphony_disp_set_alising_prob_reg3_base_blending_factor(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg3_base_blending_factor(void);
void reg_symphony_disp_set_alising_prob_reg3_default_prob(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg3_default_prob(void);
void reg_symphony_disp_set_alising_prob_reg3_op(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg3_op(void);
void reg_symphony_disp_set_alising_prob_reg3_slope_diff1_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg3_slope_diff1_sel(void);

/*!
  register REGSYMPHONY_DISP_ALISING_PROB_REG4 (read/write)
  */
void reg_symphony_disp_set_alising_prob_reg4(MT_U32 data);
MT_U32  reg_symphony_disp_get_alising_prob_reg4(void);
void reg_symphony_disp_set_alising_prob_reg4_pict_enhance_pix_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_pict_enhance_pix_sel(void);
void reg_symphony_disp_set_alising_prob_reg4_adaptive_alpha_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_adaptive_alpha_sel(void);
void reg_symphony_disp_set_alising_prob_reg4_diff_2nd_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_diff_2nd_sel(void);
void reg_symphony_disp_set_alising_prob_reg4_interp_factor(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_interp_factor(void);
void reg_symphony_disp_set_alising_prob_reg4_prob_coef3(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_prob_coef3(void);
void reg_symphony_disp_set_alising_prob_reg4_prob_coef2(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_prob_coef2(void);
void reg_symphony_disp_set_alising_prob_reg4_prob_coef1(MT_U8 data);
MT_U8   reg_symphony_disp_get_alising_prob_reg4_prob_coef1(void);

/*!
  register REGSYMPHONY_DISP_CSC_CTRL (read/write)
  */
void reg_symphony_disp_set_csc_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_ctrl(void);
void reg_symphony_disp_set_csc_ctrl_sd_bound_out_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_sd_bound_out_en(void);
void reg_symphony_disp_set_csc_ctrl_hd_bound_out_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_hd_bound_out_en(void);
void reg_symphony_disp_set_csc_ctrl_sd_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_sd_csc_en(void);
void reg_symphony_disp_set_csc_ctrl_sd_bound_in_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_sd_bound_in_en(void);
void reg_symphony_disp_set_csc_ctrl_hd_bound_in_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_hd_bound_in_en(void);
void reg_symphony_disp_set_csc_ctrl_hd_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_hd_csc_en(void);
void reg_symphony_disp_set_csc_ctrl_osd_hd2sd_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_ctrl_osd_hd2sd_csc_en(void);

/*!
  register REGSYMPHONY_DISP_CSC_HD_COEF1 (read/write)
  */
void reg_symphony_disp_set_csc_hd_coef1(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_hd_coef1(void);
void reg_symphony_disp_set_csc_hd_coef1_hd_a_01(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef1_hd_a_01(void);
void reg_symphony_disp_set_csc_hd_coef1_hd_a_00(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef1_hd_a_00(void);

/*!
  register REGSYMPHONY_DISP_CSC_HD_COEF2 (read/write)
  */
void reg_symphony_disp_set_csc_hd_coef2(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_hd_coef2(void);
void reg_symphony_disp_set_csc_hd_coef2_hd_a_10(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef2_hd_a_10(void);
void reg_symphony_disp_set_csc_hd_coef2_hd_a_02(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef2_hd_a_02(void);

/*!
  register REGSYMPHONY_DISP_CSC_HD_COEF3 (read/write)
  */
void reg_symphony_disp_set_csc_hd_coef3(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_hd_coef3(void);
void reg_symphony_disp_set_csc_hd_coef3_hd_a_12(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef3_hd_a_12(void);
void reg_symphony_disp_set_csc_hd_coef3_hd_a_11(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef3_hd_a_11(void);

/*!
  register REGSYMPHONY_DISP_CSC_HD_COEF4 (read/write)
  */
void reg_symphony_disp_set_csc_hd_coef4(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_hd_coef4(void);
void reg_symphony_disp_set_csc_hd_coef4_hd_a_21(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef4_hd_a_21(void);
void reg_symphony_disp_set_csc_hd_coef4_hd_a_20(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef4_hd_a_20(void);

/*!
  register REGSYMPHONY_DISP_CSC_HD_COEF5 (read/write)
  */
void reg_symphony_disp_set_csc_hd_coef5(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_hd_coef5(void);
void reg_symphony_disp_set_csc_hd_coef5_hd_a_22(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_hd_coef5_hd_a_22(void);

/*!
  register REGSYMPHONY_DISP_CSC_SD_COEF1 (read/write)
  */
void reg_symphony_disp_set_csc_sd_coef1(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_sd_coef1(void);
void reg_symphony_disp_set_csc_sd_coef1_sd_a_01(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef1_sd_a_01(void);
void reg_symphony_disp_set_csc_sd_coef1_sd_a_00(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef1_sd_a_00(void);

/*!
  register REGSYMPHONY_DISP_CSC_SD_COEF2 (read/write)
  */
void reg_symphony_disp_set_csc_sd_coef2(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_sd_coef2(void);
void reg_symphony_disp_set_csc_sd_coef2_sd_a_10(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef2_sd_a_10(void);
void reg_symphony_disp_set_csc_sd_coef2_sd_a_02(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef2_sd_a_02(void);

/*!
  register REGSYMPHONY_DISP_CSC_SD_COEF3 (read/write)
  */
void reg_symphony_disp_set_csc_sd_coef3(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_sd_coef3(void);
void reg_symphony_disp_set_csc_sd_coef3_sd_a_12(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef3_sd_a_12(void);
void reg_symphony_disp_set_csc_sd_coef3_sd_a_11(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef3_sd_a_11(void);

/*!
  register REGSYMPHONY_DISP_CSC_SD_COEF4 (read/write)
  */
void reg_symphony_disp_set_csc_sd_coef4(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_sd_coef4(void);
void reg_symphony_disp_set_csc_sd_coef4_sd_a_21(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef4_sd_a_21(void);
void reg_symphony_disp_set_csc_sd_coef4_sd_a_20(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef4_sd_a_20(void);

/*!
  register REGSYMPHONY_DISP_CSC_SD_COEF5 (read/write)
  */
void reg_symphony_disp_set_csc_sd_coef5(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_sd_coef5(void);
void reg_symphony_disp_set_csc_sd_coef5_sd_a_22(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_sd_coef5_sd_a_22(void);

/*!
  register REGSYMPHONY_DISP_CSC_STILL_CTRL (read/write)
  */
void reg_symphony_disp_set_csc_still_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_still_ctrl(void);
void reg_symphony_disp_set_csc_still_ctrl_hd_still_bund_out_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_still_ctrl_hd_still_bund_out_en(void);
void reg_symphony_disp_set_csc_still_ctrl_hd_still_bund_in_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_still_ctrl_hd_still_bund_in_en(void);
void reg_symphony_disp_set_csc_still_ctrl_hd_still_csc__en(MT_U8 data);
MT_U8   reg_symphony_disp_get_csc_still_ctrl_hd_still_csc__en(void);

/*!
  register REGSYMPHONY_DISP_CSC_STILL_COEF1 (read/write)
  */
void reg_symphony_disp_set_csc_still_coef1(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_still_coef1(void);
void reg_symphony_disp_set_csc_still_coef1_still_a_01(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef1_still_a_01(void);
void reg_symphony_disp_set_csc_still_coef1_still_a_00(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef1_still_a_00(void);

/*!
  register REGSYMPHONY_DISP_CSC_STILL_COEF2 (read/write)
  */
void reg_symphony_disp_set_csc_still_coef2(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_still_coef2(void);
void reg_symphony_disp_set_csc_still_coef2_still_a_10(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef2_still_a_10(void);
void reg_symphony_disp_set_csc_still_coef2_still_a_02(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef2_still_a_02(void);

/*!
  register REGSYMPHONY_DISP_CSC_STILL_COEF3 (read/write)
  */
void reg_symphony_disp_set_csc_still_coef3(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_still_coef3(void);
void reg_symphony_disp_set_csc_still_coef3_still_a_12(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef3_still_a_12(void);
void reg_symphony_disp_set_csc_still_coef3_still_a_11(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef3_still_a_11(void);

/*!
  register REGSYMPHONY_DISP_CSC_STILL_COEF4 (read/write)
  */
void reg_symphony_disp_set_csc_still_coef4(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_still_coef4(void);
void reg_symphony_disp_set_csc_still_coef4_still_a_21(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef4_still_a_21(void);
void reg_symphony_disp_set_csc_still_coef4_still_a_20(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef4_still_a_20(void);

/*!
  register REGSYMPHONY_DISP_CSC_STILL_COEF5 (read/write)
  */
void reg_symphony_disp_set_csc_still_coef5(MT_U32 data);
MT_U32  reg_symphony_disp_get_csc_still_coef5(void);
void reg_symphony_disp_set_csc_still_coef5_still_a_22(MT_U16 data);
MT_U16  reg_symphony_disp_get_csc_still_coef5_still_a_22(void);

/*!
  register REGSYMPHONY_DISP_ROW_JUMP_00 (read/write)
  */
void reg_symphony_disp_set_row_jump_00(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_00(void);
void reg_symphony_disp_set_row_jump_00_rowjump_00(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_00_rowjump_00(void);

/*!
  register REGSYMPHONY_DISP_ROW_JUMP_01 (read/write)
  */
void reg_symphony_disp_set_row_jump_01(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_01(void);
void reg_symphony_disp_set_row_jump_01_rowjump_01(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_01_rowjump_01(void);

/*!
  register REGSYMPHONY_DISP_ROW_JUMP_10 (read/write)
  */
void reg_symphony_disp_set_row_jump_10(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_10(void);
void reg_symphony_disp_set_row_jump_10_rowjump_10(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_10_rowjump_10(void);

/*!
  register REGSYMPHONY_DISP_ROW_JUMP_11 (read/write)
  */
void reg_symphony_disp_set_row_jump_11(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_11(void);
void reg_symphony_disp_set_row_jump_11_rowjump_11(MT_U32 data);
MT_U32  reg_symphony_disp_get_row_jump_11_rowjump_11(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_DOMAIN_1 (read/write)
  */
void reg_symphony_disp_set_denoise_domain_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_domain_1(void);
void reg_symphony_disp_set_denoise_domain_1_domain_c2(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_domain_1_domain_c2(void);
void reg_symphony_disp_set_denoise_domain_1_domain_c1(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_domain_1_domain_c1(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_DOMAIN_2 (read/write)
  */
void reg_symphony_disp_set_denoise_domain_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_domain_2(void);
void reg_symphony_disp_set_denoise_domain_2_domain_c4(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_domain_2_domain_c4(void);
void reg_symphony_disp_set_denoise_domain_2_domain_c3(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_domain_2_domain_c3(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_DOMAIN_3 (read/write)
  */
void reg_symphony_disp_set_denoise_domain_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_domain_3(void);
void reg_symphony_disp_set_denoise_domain_3_domain_c5(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_domain_3_domain_c5(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_RANGE_1 (read/write)
  */
void reg_symphony_disp_set_denoise_range_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_range_1(void);
void reg_symphony_disp_set_denoise_range_1_range_3(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_1_range_3(void);
void reg_symphony_disp_set_denoise_range_1_range_2(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_1_range_2(void);
void reg_symphony_disp_set_denoise_range_1_range_1(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_1_range_1(void);
void reg_symphony_disp_set_denoise_range_1_range_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_1_range_0(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_RANGE_2 (read/write)
  */
void reg_symphony_disp_set_denoise_range_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_range_2(void);
void reg_symphony_disp_set_denoise_range_2_range_7(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_2_range_7(void);
void reg_symphony_disp_set_denoise_range_2_range_6(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_2_range_6(void);
void reg_symphony_disp_set_denoise_range_2_range_5(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_2_range_5(void);
void reg_symphony_disp_set_denoise_range_2_range_4(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_2_range_4(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_RANGE_3 (read/write)
  */
void reg_symphony_disp_set_denoise_range_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_range_3(void);
void reg_symphony_disp_set_denoise_range_3_range_11(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_3_range_11(void);
void reg_symphony_disp_set_denoise_range_3_range_10(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_3_range_10(void);
void reg_symphony_disp_set_denoise_range_3_range_9(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_3_range_9(void);
void reg_symphony_disp_set_denoise_range_3_range_8(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_3_range_8(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_RANGE_4 (read/write)
  */
void reg_symphony_disp_set_denoise_range_4(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_range_4(void);
void reg_symphony_disp_set_denoise_range_4_range_15(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_4_range_15(void);
void reg_symphony_disp_set_denoise_range_4_range_14(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_4_range_14(void);
void reg_symphony_disp_set_denoise_range_4_range_13(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_4_range_13(void);
void reg_symphony_disp_set_denoise_range_4_range_12(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_4_range_12(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_RANGE_5 (read/write)
  */
void reg_symphony_disp_set_denoise_range_5(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_range_5(void);
void reg_symphony_disp_set_denoise_range_5_range_19(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_5_range_19(void);
void reg_symphony_disp_set_denoise_range_5_range_18(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_5_range_18(void);
void reg_symphony_disp_set_denoise_range_5_range_17(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_5_range_17(void);
void reg_symphony_disp_set_denoise_range_5_range_16(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_5_range_16(void);

/*!
  register REGSYMPHONY_DISP_DENOISE_RANGE_6 (read/write)
  */
void reg_symphony_disp_set_denoise_range_6(MT_U32 data);
MT_U32  reg_symphony_disp_get_denoise_range_6(void);
void reg_symphony_disp_set_denoise_range_6_vid_denoise_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_6_vid_denoise_en(void);
void reg_symphony_disp_set_denoise_range_6_z_reg(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_6_z_reg(void);
void reg_symphony_disp_set_denoise_range_6_range_20(MT_U8 data);
MT_U8   reg_symphony_disp_get_denoise_range_6_range_20(void);

/*!
  register REGSYMPHONY_DISP_COLOR_ENHANCE_CTRL (read/write)
  */
void reg_symphony_disp_set_color_enhance_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_color_enhance_ctrl(void);
void reg_symphony_disp_set_color_enhance_ctrl_color_enhance_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_color_enhance_ctrl_color_enhance_en(void);
void reg_symphony_disp_set_color_enhance_ctrl_color_enhance_reg_red_dec(MT_U8 data);
MT_U8   reg_symphony_disp_get_color_enhance_ctrl_color_enhance_reg_red_dec(void);
void reg_symphony_disp_set_color_enhance_ctrl_color_enhance_reg_length(MT_U8 data);
MT_U8   reg_symphony_disp_get_color_enhance_ctrl_color_enhance_reg_length(void);
void reg_symphony_disp_set_color_enhance_ctrl_color_enhance_reg_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_color_enhance_ctrl_color_enhance_reg_thr(void);

/*!
  register REGSYMPHONY_DISP_SD_WRBACK_ADDR_ODD (read/write)
  */
void reg_symphony_disp_set_sd_wrback_addr_odd(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_wrback_addr_odd(void);
void reg_symphony_disp_set_sd_wrback_addr_odd_sd_wrback_addr_odd(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_wrback_addr_odd_sd_wrback_addr_odd(void);

/*!
  register REGSYMPHONY_DISP_SD_BASE_ADDR_EVEN (read/write)
  */
void reg_symphony_disp_set_sd_base_addr_even(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_base_addr_even(void);
void reg_symphony_disp_set_sd_base_addr_even_sd_base_addr_even(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_base_addr_even_sd_base_addr_even(void);

/*!
  register REGSYMPHONY_DISP_SD_WRBACK_CTRL (read/write)
  */
void reg_symphony_disp_set_sd_wrback_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_wrback_ctrl(void);
void reg_symphony_disp_set_sd_wrback_ctrl_sd_start_lines(MT_U16 data);
MT_U16  reg_symphony_disp_get_sd_wrback_ctrl_sd_start_lines(void);
void reg_symphony_disp_set_sd_wrback_ctrl_one_field_buffer(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_wrback_ctrl_one_field_buffer(void);
void reg_symphony_disp_set_sd_wrback_ctrl_sd_buffer_num(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_wrback_ctrl_sd_buffer_num(void);
void reg_symphony_disp_set_sd_wrback_ctrl_sd_blankscreen_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_wrback_ctrl_sd_blankscreen_mode(void);
void reg_symphony_disp_set_sd_wrback_ctrl_sd_softctrl_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_wrback_ctrl_sd_softctrl_en(void);

/*!
  register REGSYMPHONY_DISP_SD_BACK_COLOR (read/write)
  */
void reg_symphony_disp_set_sd_back_color(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_back_color(void);
void reg_symphony_disp_set_sd_back_color_v(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_back_color_v(void);
void reg_symphony_disp_set_sd_back_color_u(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_back_color_u(void);
void reg_symphony_disp_set_sd_back_color_y(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_back_color_y(void);

/*!
  register REGSYMPHONY_DISP_STILL_UV_START_ADDR_HD (read/write)
  */
void reg_symphony_disp_set_still_uv_start_addr_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_uv_start_addr_hd(void);
void reg_symphony_disp_set_still_uv_start_addr_hd_hd_still_uv_start_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_uv_start_addr_hd_hd_still_uv_start_addr(void);

/*!
  register REGSYMPHONY_DISP_STILL_Y_START_ADDR_HD (read/write)
  */
void reg_symphony_disp_set_still_y_start_addr_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_y_start_addr_hd(void);
void reg_symphony_disp_set_still_y_start_addr_hd_hd_still_y_start_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_y_start_addr_hd_hd_still_y_start_addr(void);

/*!
  register REGSYMPHONY_DISP_SUB_START_ADDR_HD (read/write)
  */
void reg_symphony_disp_set_sub_start_addr_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_start_addr_hd(void);
void reg_symphony_disp_set_sub_start_addr_hd_sub_header_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_start_addr_hd_sub_header_addr(void);

/*!
  register REGSYMPHONY_DISP_OSD1_START_ADDR_HD (read/write)
  */
void reg_symphony_disp_set_osd1_start_addr_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd1_start_addr_hd(void);
void reg_symphony_disp_set_osd1_start_addr_hd_osd1_header_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd1_start_addr_hd_osd1_header_addr(void);

/*!
  register REGSYMPHONY_DISP_STILL_START_ADDR_SD (read/write)
  */
void reg_symphony_disp_set_still_start_addr_sd(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_start_addr_sd(void);
void reg_symphony_disp_set_still_start_addr_sd_sd_still_start_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_still_start_addr_sd_sd_still_start_addr(void);

/*!
  register REGSYMPHONY_DISP_OSD0_START_ADDR_HD (read/write)
  */
void reg_symphony_disp_set_osd0_start_addr_hd(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd0_start_addr_hd(void);
void reg_symphony_disp_set_osd0_start_addr_hd_osd0_header_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd0_start_addr_hd_osd0_header_addr(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_DISPLAY_INFO (read/write)
  */
void reg_symphony_disp_set_video_display_info(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_display_info(void);
void reg_symphony_disp_set_video_display_info_firmware_force_di_disable(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_firmware_force_di_disable(void);
void reg_symphony_disp_set_video_display_info_progressive_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_progressive_flag(void);
void reg_symphony_disp_set_video_display_info_frame_field_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_frame_field_flag(void);
void reg_symphony_disp_set_video_display_info_height_low2bits(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_height_low2bits(void);
void reg_symphony_disp_set_video_display_info_height_mid3bits(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_height_mid3bits(void);
void reg_symphony_disp_set_video_display_info_height_high3bits(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_height_high3bits(void);
void reg_symphony_disp_set_video_display_info_width_low2bits(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_width_low2bits(void);
void reg_symphony_disp_set_video_display_info_width_mid3bits(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_width_mid3bits(void);
void reg_symphony_disp_set_video_display_info_width_high3bits(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_display_info_width_high3bits(void);

/*!
  register REGSYMPHONY_DISP_MOTION_PRE_ADDR (read/write)
  */
void reg_symphony_disp_set_motion_pre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_pre_addr(void);
void reg_symphony_disp_set_motion_pre_addr_motion_pre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_pre_addr_motion_pre_addr(void);

/*!
  register REGSYMPHONY_DISP_MOTION_CUR_ADDR (read/write)
  */
void reg_symphony_disp_set_motion_cur_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_cur_addr(void);
void reg_symphony_disp_set_motion_cur_addr_motion_cur_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_cur_addr_motion_cur_addr(void);

/*!
  register REGSYMPHONY_DISP_LUMA_PRE_ADDR (read/write)
  */
void reg_symphony_disp_set_luma_pre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_pre_addr(void);
void reg_symphony_disp_set_luma_pre_addr_luma_pre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_pre_addr_luma_pre_addr(void);

/*!
  register REGSYMPHONY_DISP_LUMA_CUR_ADDR (read/write)
  */
void reg_symphony_disp_set_luma_cur_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_cur_addr(void);
void reg_symphony_disp_set_luma_cur_addr_luma_cur_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_cur_addr_luma_cur_addr(void);

/*!
  register REGSYMPHONY_DISP_LUMA_NEXT_ADDR (read/write)
  */
void reg_symphony_disp_set_luma_next_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_next_addr(void);
void reg_symphony_disp_set_luma_next_addr_luma_next_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_next_addr_luma_next_addr(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_PPRE_ADDR (read/write)
  */
void reg_symphony_disp_set_chroma_ppre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_ppre_addr(void);
void reg_symphony_disp_set_chroma_ppre_addr_chroma_ppre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_ppre_addr_chroma_ppre_addr(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_PRE_ADDR (read/write)
  */
void reg_symphony_disp_set_chroma_pre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_pre_addr(void);
void reg_symphony_disp_set_chroma_pre_addr_chroma_pre_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_pre_addr_chroma_pre_addr(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_CUR_ADDR (read/write)
  */
void reg_symphony_disp_set_chroma_cur_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_cur_addr(void);
void reg_symphony_disp_set_chroma_cur_addr_chroma_cur_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_cur_addr_chroma_cur_addr(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_NEXT_ADDR (read/write)
  */
void reg_symphony_disp_set_chroma_next_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_next_addr(void);
void reg_symphony_disp_set_chroma_next_addr_chroma_next_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_next_addr_chroma_next_addr(void);

/*!
  register REGSYMPHONY_DISP_MOTION_PRE_ADDR2 (read/write)
  */
void reg_symphony_disp_set_motion_pre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_pre_addr2(void);
void reg_symphony_disp_set_motion_pre_addr2_motion_pre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_pre_addr2_motion_pre_addr2(void);

/*!
  register REGSYMPHONY_DISP_MOTION_CUR_ADDR2 (read/write)
  */
void reg_symphony_disp_set_motion_cur_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_cur_addr2(void);
void reg_symphony_disp_set_motion_cur_addr2_motion_cur_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_motion_cur_addr2_motion_cur_addr2(void);

/*!
  register REGSYMPHONY_DISP_LUMA_PRE_ADDR2 (read/write)
  */
void reg_symphony_disp_set_luma_pre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_pre_addr2(void);
void reg_symphony_disp_set_luma_pre_addr2_luma_pre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_pre_addr2_luma_pre_addr2(void);

/*!
  register REGSYMPHONY_DISP_LUMA_CUR_ADDR2 (read/write)
  */
void reg_symphony_disp_set_luma_cur_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_cur_addr2(void);
void reg_symphony_disp_set_luma_cur_addr2_luma_cur_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_cur_addr2_luma_cur_addr2(void);

/*!
  register REGSYMPHONY_DISP_LUMA_NEXT_ADDR2 (read/write)
  */
void reg_symphony_disp_set_luma_next_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_next_addr2(void);
void reg_symphony_disp_set_luma_next_addr2_luma_next_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_luma_next_addr2_luma_next_addr2(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_PPRE_ADDR2 (read/write)
  */
void reg_symphony_disp_set_chroma_ppre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_ppre_addr2(void);
void reg_symphony_disp_set_chroma_ppre_addr2_chroma_ppre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_ppre_addr2_chroma_ppre_addr2(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_PRE_ADDR2 (read/write)
  */
void reg_symphony_disp_set_chroma_pre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_pre_addr2(void);
void reg_symphony_disp_set_chroma_pre_addr2_chroma_pre_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_pre_addr2_chroma_pre_addr2(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_CUR_ADDR2 (read/write)
  */
void reg_symphony_disp_set_chroma_cur_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_cur_addr2(void);
void reg_symphony_disp_set_chroma_cur_addr2_chroma_cur_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_cur_addr2_chroma_cur_addr2(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_NEXT_ADDR2 (read/write)
  */
void reg_symphony_disp_set_chroma_next_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_next_addr2(void);
void reg_symphony_disp_set_chroma_next_addr2_chroma_next_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_next_addr2_chroma_next_addr2(void);

/*!
  register REGSYMPHONY_DISP_FIELD_PIC_FMT (read/write)
  */
void reg_symphony_disp_set_field_pic_fmt(MT_U32 data);
MT_U32  reg_symphony_disp_get_field_pic_fmt(void);
void reg_symphony_disp_set_field_pic_fmt_field_pic_fmt(MT_U8 data);
MT_U8   reg_symphony_disp_get_field_pic_fmt_field_pic_fmt(void);

/*!
  register REGSYMPHONY_DISP_VID_HD_VF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_vid_hd_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_hd_vf_coef_addr(void);
void reg_symphony_disp_set_vid_hd_vf_coef_addr_vid_hd_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_hd_vf_coef_addr_vid_hd_vf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_VID_HD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_vid_hd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_hd_hf_coef_addr(void);
void reg_symphony_disp_set_vid_hd_hf_coef_addr_vid_hd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_hd_hf_coef_addr_vid_hd_hf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_VID_SD_VF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_vid_sd_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_sd_vf_coef_addr(void);
void reg_symphony_disp_set_vid_sd_vf_coef_addr_vid_sd_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_sd_vf_coef_addr_vid_sd_vf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_VID_SD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_vid_sd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_sd_hf_coef_addr(void);
void reg_symphony_disp_set_vid_sd_hf_coef_addr_vid_sd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_sd_hf_coef_addr_vid_sd_hf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_GRA_VF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_gra_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_vf_coef_addr(void);
void reg_symphony_disp_set_gra_vf_coef_addr_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_vf_coef_addr_vf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_GRA_HF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_gra_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_hf_coef_addr(void);
void reg_symphony_disp_set_gra_hf_coef_addr_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_gra_hf_coef_addr_hf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_VID_DCE_MAP_ADDR (read/write)
  */
void reg_symphony_disp_set_vid_dce_map_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_dce_map_addr(void);
void reg_symphony_disp_set_vid_dce_map_addr_dce_map_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vid_dce_map_addr_dce_map_addr(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_TABLE_SEL (read/write)
  */
void reg_symphony_disp_set_vscaler_table_sel(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_dce_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_dce_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_osd_hori_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_osd_hori_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_osd_vert_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_osd_vert_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_gra_hori_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_gra_hori_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_gra_vert_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_gra_vert_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_chroma_sd_hori_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_chroma_sd_hori_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_chroma_hd_hori_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_chroma_hd_hori_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_sd_hori_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_sd_hori_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_sd_vert_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_sd_vert_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_hd_hori_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_hd_hori_table_sel(void);
void reg_symphony_disp_set_vscaler_table_sel_hd_vert_table_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_hd_vert_table_sel(void);

void reg_symphony_disp_set_vscaler_table_sel_new_csc_coef_table_2_load_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_new_csc_coef_table_2_load_en(void);
void reg_symphony_disp_set_vscaler_table_sel_new_csc_coef_table_1_load_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_new_csc_coef_table_1_load_en(void);
void reg_symphony_disp_set_vscaler_table_sel_new_csc_coef_table_6_load_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_new_csc_coef_table_6_load_en(void);
void reg_symphony_disp_set_vscaler_table_sel_new_csc_coef_table_5_load_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_new_csc_coef_table_5_load_en(void);
void reg_symphony_disp_set_vscaler_table_sel_new_csc_coef_table_4_load_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_new_csc_coef_table_4_load_en(void);
void reg_symphony_disp_set_vscaler_table_sel_new_csc_coef_table_3_load_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_new_csc_coef_table_3_load_en(void);

void reg_symphony_disp_set_vscaler_table_sel_adv_3d_lut_table_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_adv_3d_lut_table_en(void);
void reg_symphony_disp_set_vscaler_table_sel_sd_new_csc_rgb_table_in(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_sd_new_csc_rgb_table_in(void);
void reg_symphony_disp_set_vscaler_table_sel_sd_new_csc_rgb_table_out(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_sd_new_csc_rgb_table_out(void);
void reg_symphony_disp_set_vscaler_table_sel_sd_new_csc_gain_table_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_sd_new_csc_gain_table_en(void);
void reg_symphony_disp_set_vscaler_table_sel_osd_new_csc_table_in(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_osd_new_csc_table_in(void);
void reg_symphony_disp_set_vscaler_table_sel_osd_new_csc_table_out(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_osd_new_csc_table_out(void);
void reg_symphony_disp_set_vscaler_table_sel_osd_new_csc_gain_table_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_table_sel_osd_new_csc_gain_table_en(void);

/*!
  register REGSYMPHONY_DISP_OSD_VF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_osd_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_vf_coef_addr(void);
void reg_symphony_disp_set_osd_vf_coef_addr_osd_vf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_vf_coef_addr_osd_vf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_OSD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_osd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_hf_coef_addr(void);
void reg_symphony_disp_set_osd_hf_coef_addr_osd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_hf_coef_addr_osd_hf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_HD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_chroma_hd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_hd_hf_coef_addr(void);
void reg_symphony_disp_set_chroma_hd_hf_coef_addr_chroma_hd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_hd_hf_coef_addr_chroma_hd_hf_coef_addr(void);

/*!
  register REGSYMPHONY_DISP_CHROMA_SD_HF_COEF_ADDR (read/write)
  */
void reg_symphony_disp_set_chroma_sd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_sd_hf_coef_addr(void);
void reg_symphony_disp_set_chroma_sd_hf_coef_addr_chroma_sd_hf_coef_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_chroma_sd_hf_coef_addr_chroma_sd_hf_coef_addr(void);

//hmccccccccccccccccccccccccccccccccccccccc

void reg_symphony_disp_set_luma_bot_cur_addr(MT_U32 data);

void reg_symphony_disp_set_chroma_bot_cur_addr(MT_U32 data);

void reg_symphony_disp_set_luma_bot_cur_addr2(MT_U32 data);

void reg_symphony_disp_set_chroma_bot_cur_addr2(MT_U32 data);

/*!
  register REGSYMPHONY_DISP_NLMEANS_DENOISE_CTRL (read/write)
  */
void reg_symphony_disp_set_nlmeans_denoise_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_nlmeans_denoise_ctrl(void);
void reg_symphony_disp_set_nlmeans_denoise_ctrl_nlmeans_alpha(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_denoise_ctrl_nlmeans_alpha(void);
void reg_symphony_disp_set_nlmeans_denoise_ctrl_denoise_thrn(MT_U8 data);
MT_U8   reg_symphony_disp_get_nlmeans_denoise_ctrl_denoise_thrn(void);
void reg_symphony_disp_set_nlmeans_denoise_ctrl_denoise_thrb(MT_U8 data);
MT_U8   reg_symphony_disp_get_nlmeans_denoise_ctrl_denoise_thrb(void);
void reg_symphony_disp_set_nlmeans_denoise_ctrl_denoise_thrg(MT_U8 data);
MT_U8   reg_symphony_disp_get_nlmeans_denoise_ctrl_denoise_thrg(void);

/*!
  register REGSYMPHONY_DISP_NLMEANS_PARAMETER_1 (read/write)
  */
void reg_symphony_disp_set_nlmeans_parameter_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_nlmeans_parameter_1(void);
void reg_symphony_disp_set_nlmeans_parameter_1_reg_a2(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_parameter_1_reg_a2(void);
void reg_symphony_disp_set_nlmeans_parameter_1_reg_a1(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_parameter_1_reg_a1(void);

/*!
  register REGSYMPHONY_DISP_NLMEANS_PARAMETER_2 (read/write)
  */
void reg_symphony_disp_set_nlmeans_parameter_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_nlmeans_parameter_2(void);
void reg_symphony_disp_set_nlmeans_parameter_2_reg_b1(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_parameter_2_reg_b1(void);
void reg_symphony_disp_set_nlmeans_parameter_2_reg_a3(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_parameter_2_reg_a3(void);

/*!
  register REGSYMPHONY_DISP_NLMEANS_PARAMETER_3 (read/write)
  */
void reg_symphony_disp_set_nlmeans_parameter_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_nlmeans_parameter_3(void);
void reg_symphony_disp_set_nlmeans_parameter_3_reg_b3(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_parameter_3_reg_b3(void);
void reg_symphony_disp_set_nlmeans_parameter_3_reg_b2(MT_U16 data);
MT_U16  reg_symphony_disp_get_nlmeans_parameter_3_reg_b2(void);

void reg_symphony_disp_set_hdmi_data_sel_in_hd_screen_mix(MT_U16 data);
void reg_symphony_disp_set_hdmi_data_sel_in_hd_gralayer_mix(MT_U16 data);
void reg_symphony_disp_set_hd2sd_data_sel(MT_U16 data);
void reg_symphony_disp_set_output_data_sel_hd_osd_mode(MT_U8 data);
MT_U8 reg_symphony_disp_get_output_data_sel_hd_osd_mode(void);
void reg_symphony_disp_set_output_data_sel_sd_osd_mode(MT_U8 data);
MT_U8 reg_symphony_disp_get_output_data_sel_sd_osd_mode(void);

/*!
  register REGSYMPHONY_DISP_DI_ENABLE (read/write)
  */
void reg_symphony_disp_set_di_enable(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_enable(void);
void reg_symphony_disp_set_di_enable_enable(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_enable_enable(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_PDD_EN (read/write)
  */
void reg_symphony_disp_set_video_pdd_en(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_pdd_en(void);
void reg_symphony_disp_set_video_pdd_en_di_pdd_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_pdd_en_di_pdd_en(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_IS_MOVIE_TYPE (read/write)
  */
void reg_symphony_disp_set_video_is_movie_type(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_is_movie_type(void);
void reg_symphony_disp_set_video_is_movie_type_is_movie_type(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_is_movie_type_is_movie_type(void);

/*!
  register REGSYMPHONY_DISP_DI_P_OR_N_PAIRED (read/write)
  */
void reg_symphony_disp_set_di_p_or_n_paired(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_p_or_n_paired(void);
void reg_symphony_disp_set_di_p_or_n_paired_p_or_n_paired(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_p_or_n_paired_p_or_n_paired(void);

/*!
  register REGSYMPHONY_DISP_DI_OPER_MODE (read/write)
  */
void reg_symphony_disp_set_di_oper_mode(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_oper_mode(void);
void reg_symphony_disp_set_di_oper_mode_mix_output_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_mix_output_mode(void);
void reg_symphony_disp_set_di_oper_mode_spatial_ip_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_spatial_ip_mode(void);
void reg_symphony_disp_set_di_oper_mode_hori_ip_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_hori_ip_en(void);
void reg_symphony_disp_set_di_oper_mode_temporal_ip_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_temporal_ip_mode(void);
void reg_symphony_disp_set_di_oper_mode_lbam_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_lbam_en(void);
void reg_symphony_disp_set_di_oper_mode_motion_est_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_motion_est_mode(void);
void reg_symphony_disp_set_di_oper_mode_motion_rd_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_motion_rd_en(void);
void reg_symphony_disp_set_di_oper_mode_motion_wr_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_oper_mode_motion_wr_en(void);

/*!
  register REGSYMPHONY_DISP_DI_PARA (read/write)
  */
void reg_symphony_disp_set_di_para(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_para(void);
void reg_symphony_disp_set_di_para_g_alpha_k(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_para_g_alpha_k(void);
void reg_symphony_disp_set_di_para_g_alpha_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_para_g_alpha_0(void);
void reg_symphony_disp_set_di_para_p_tl(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_para_p_tl(void);

/*!
  register REGSYMPHONY_DISP_DI_DATA_SFIFO_THR (read/write)
  */
void reg_symphony_disp_set_di_data_sfifo_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_data_sfifo_thr(void);
void reg_symphony_disp_set_di_data_sfifo_thr_di_data_sfifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_data_sfifo_thr_di_data_sfifo_thr(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_SCALER_DATA_SFIFO_THR (read/write)
  */
void reg_symphony_disp_set_video_scaler_data_sfifo_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_scaler_data_sfifo_thr(void);
void reg_symphony_disp_set_video_scaler_data_sfifo_thr_video_scaler_data_sfifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_scaler_data_sfifo_thr_video_scaler_data_sfifo_thr(void);

/*!
  register REGSYMPHONY_DISP_DI_LOUT_AFIFO_THR (read/write)
  */
void reg_symphony_disp_set_di_lout_afifo_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_lout_afifo_thr(void);
void reg_symphony_disp_set_di_lout_afifo_thr_di_lout_afifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_lout_afifo_thr_di_lout_afifo_thr(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_AXI_CMD_SFIFO_THR (read/write)
  */
void reg_symphony_disp_set_vscaler_axi_cmd_sfifo_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_axi_cmd_sfifo_thr(void);
void reg_symphony_disp_set_vscaler_axi_cmd_sfifo_thr_vscaler_axi_cmd_sfifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_axi_cmd_sfifo_thr_vscaler_axi_cmd_sfifo_thr(void);

/*!
  register REGSYMPHONY_DISP_VSCALER_AXI_REQ_SFIFO_THR (read/write)
  */
void reg_symphony_disp_set_vscaler_axi_req_sfifo_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_vscaler_axi_req_sfifo_thr(void);
void reg_symphony_disp_set_vscaler_axi_req_sfifo_thr_vscaler_axi_req_sfifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_vscaler_axi_req_sfifo_thr_vscaler_axi_req_sfifo_thr(void);

/*!
  register REGSYMPHONY_DISP_DI_PDD_NOISE_THR (read/write)
  */
void reg_symphony_disp_set_di_pdd_noise_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_pdd_noise_thr(void);
void reg_symphony_disp_set_di_pdd_noise_thr_di_pdd_noise_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_pdd_noise_thr_di_pdd_noise_thr(void);

/*!
  register REGSYMPHONY_DISP_DI_ACC_RESULT_ODD (read/write)
  */
void reg_symphony_disp_set_di_acc_result_odd(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_acc_result_odd(void);
void reg_symphony_disp_set_di_acc_result_odd_di_acc_result_odd(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_acc_result_odd_di_acc_result_odd(void);

/*!
  register REGSYMPHONY_DISP_DI_ACC_RESULT_EVEN (read/write)
  */
void reg_symphony_disp_set_di_acc_result_even(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_acc_result_even(void);
void reg_symphony_disp_set_di_acc_result_even_di_acc_result_even(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_acc_result_even_di_acc_result_even(void);

/*!
  register REGSYMPHONY_DISP_DI_PAUSE_EN (read/write)
  */
void reg_symphony_disp_set_di_pause_en(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_pause_en(void);
void reg_symphony_disp_set_di_pause_en_di_pause_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_pause_en_di_pause_en(void);
void reg_symphony_disp_set_di_pause_en_di_pause_bot_field_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_pause_en_di_pause_bot_field_flag(void);
void reg_symphony_disp_set_di_pause_en_di_pause_top_field(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_pause_en_di_pause_top_field(void);

/*!
  register REGSYMPHONY_DISP_DI_ALPHA_PARA (read/write)
  */
void reg_symphony_disp_set_di_alpha_para(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_alpha_para(void);
void reg_symphony_disp_set_di_alpha_para_g_alpha_0_min(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_alpha_para_g_alpha_0_min(void);
void reg_symphony_disp_set_di_alpha_para_g_alpha_0_max(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_alpha_para_g_alpha_0_max(void);
void reg_symphony_disp_set_di_alpha_para_luma_diff_k(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_alpha_para_luma_diff_k(void);
void reg_symphony_disp_set_di_alpha_para_diff_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_alpha_para_diff_sel(void);
void reg_symphony_disp_set_di_alpha_para_new_alpha_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_alpha_para_new_alpha_en(void);

/*!
  register REGSYMPHONY_DISP_DI_MOTION_CTRL_1 (read/write)
  */
void reg_symphony_disp_set_di_motion_ctrl_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_motion_ctrl_1(void);
void reg_symphony_disp_set_di_motion_ctrl_1_motion_propa_type(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_1_motion_propa_type(void);
void reg_symphony_disp_set_di_motion_ctrl_1_motion_damping2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_1_motion_damping2(void);
void reg_symphony_disp_set_di_motion_ctrl_1_motion_damping1(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_1_motion_damping1(void);
void reg_symphony_disp_set_di_motion_ctrl_1_medrsp_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_1_medrsp_thr(void);
void reg_symphony_disp_set_di_motion_ctrl_1_difdamping(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_1_difdamping(void);

/*!
  register REGSYMPHONY_DISP_DI_MOTION_CTRL_2 (read/write)
  */
void reg_symphony_disp_set_di_motion_ctrl_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_motion_ctrl_2(void);
void reg_symphony_disp_set_di_motion_ctrl_2_half_motion_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_half_motion_en(void);
void reg_symphony_disp_set_di_motion_ctrl_2_motion_data_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_motion_data_mode(void);
void reg_symphony_disp_set_di_motion_ctrl_2_motion_estmethod(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_motion_estmethod(void);
void reg_symphony_disp_set_di_motion_ctrl_2_l0l2_motion_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_l0l2_motion_mode(void);
void reg_symphony_disp_set_di_motion_ctrl_2_ip_smallmotion(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_ip_smallmotion(void);
void reg_symphony_disp_set_di_motion_ctrl_2_ip_average(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_ip_average(void);
void reg_symphony_disp_set_di_motion_ctrl_2_ip_l0_or_l2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_2_ip_l0_or_l2(void);

/*!
  register REGSYMPHONY_DISP_DI_HEVC_FLAG (read/write)
  */
void reg_symphony_disp_set_di_hevc_flag(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_hevc_flag(void);
void reg_symphony_disp_set_di_hevc_flag_nxt_hevc_flag2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_nxt_hevc_flag2(void);
void reg_symphony_disp_set_di_hevc_flag_cur_hevc_flag2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_cur_hevc_flag2(void);
void reg_symphony_disp_set_di_hevc_flag_pre_hevc_flag2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_pre_hevc_flag2(void);
void reg_symphony_disp_set_di_hevc_flag_ppre_hevc_flag2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_ppre_hevc_flag2(void);
void reg_symphony_disp_set_di_hevc_flag_nxt_hevc_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_nxt_hevc_flag(void);
void reg_symphony_disp_set_di_hevc_flag_cur_hevc_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_cur_hevc_flag(void);
void reg_symphony_disp_set_di_hevc_flag_pre_hevc_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_pre_hevc_flag(void);
void reg_symphony_disp_set_di_hevc_flag_ppre_hevc_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_hevc_flag_ppre_hevc_flag(void);

/*!
  register REGSYMPHONY_DISP_NONE_DI_FIELDS_FLAG (read/write)
  */
void reg_symphony_disp_set_none_di_fields_flag(MT_U32 data);
MT_U32  reg_symphony_disp_get_none_di_fields_flag(void);
void reg_symphony_disp_set_none_di_fields_flag_cur_bot_field_flag_2(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_fields_flag_cur_bot_field_flag_2(void);
void reg_symphony_disp_set_none_di_fields_flag_cur_top_field_flag_2(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_fields_flag_cur_top_field_flag_2(void);
void reg_symphony_disp_set_none_di_fields_flag_cur_bot_field_flag_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_fields_flag_cur_bot_field_flag_0(void);
void reg_symphony_disp_set_none_di_fields_flag_cur_top_field_flag_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_fields_flag_cur_top_field_flag_0(void);

/*!
  register REGSYMPHONY_DISP_NONE_DI_HEVC_FLAG (read/write)
  */
void reg_symphony_disp_set_none_di_hevc_flag(MT_U32 data);
MT_U32  reg_symphony_disp_get_none_di_hevc_flag(void);
void reg_symphony_disp_set_none_di_hevc_flag_cur_bot_hevc_flag_2(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_hevc_flag_cur_bot_hevc_flag_2(void);
void reg_symphony_disp_set_none_di_hevc_flag_cur_top_hevc_flag_2(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_hevc_flag_cur_top_hevc_flag_2(void);
void reg_symphony_disp_set_none_di_hevc_flag_cur_bot_hevc_flag_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_hevc_flag_cur_bot_hevc_flag_0(void);
void reg_symphony_disp_set_none_di_hevc_flag_cur_top_hevc_flag_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_hevc_flag_cur_top_hevc_flag_0(void);

/*!
  register REGSYMPHONY_DISP_NONE_DI_PROGRESSIVE_FLAG (read/write)
  */
void reg_symphony_disp_set_none_di_progressive_flag(MT_U32 data);
MT_U32  reg_symphony_disp_get_none_di_progressive_flag(void);
void reg_symphony_disp_set_none_di_progressive_flag_progressive_flag_2(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_progressive_flag_progressive_flag_2(void);
void reg_symphony_disp_set_none_di_progressive_flag_progressive_flag_0(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_progressive_flag_progressive_flag_0(void);
void reg_symphony_disp_set_none_di_progressive_flag_only_use_set_0_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_none_di_progressive_flag_only_use_set_0_en(void);

/*!
  register REGSYMPHONY_DISP_DI_MOTION_CTRL_3 (read/write)
  */
void reg_symphony_disp_set_di_motion_ctrl_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_motion_ctrl_3(void);
void reg_symphony_disp_set_di_motion_ctrl_3_new_algorithm_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_3_new_algorithm_en(void);
void reg_symphony_disp_set_di_motion_ctrl_3_small_motion_magnify(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_3_small_motion_magnify(void);
void reg_symphony_disp_set_di_motion_ctrl_3_small_motion_thr2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_3_small_motion_thr2(void);
void reg_symphony_disp_set_di_motion_ctrl_3_small_motion_thr1(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_3_small_motion_thr1(void);
void reg_symphony_disp_set_di_motion_ctrl_3_medrsp7dir_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_3_medrsp7dir_thr(void);

/*!
  register REGSYMPHONY_DISP_DI_MOTION_CTRL_4 (read/write)
  */
void reg_symphony_disp_set_di_motion_ctrl_4(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_motion_ctrl_4(void);
void reg_symphony_disp_set_di_motion_ctrl_4_uv_motion_gain(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_4_uv_motion_gain(void);
void reg_symphony_disp_set_di_motion_ctrl_4_ip_l0andl2(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_4_ip_l0andl2(void);
void reg_symphony_disp_set_di_motion_ctrl_4_ip_l1_difthr(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_4_ip_l1_difthr(void);
void reg_symphony_disp_set_di_motion_ctrl_4_preserve_all_motion(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_4_preserve_all_motion(void);
void reg_symphony_disp_set_di_motion_ctrl_4_preserve_small_motion(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_4_preserve_small_motion(void);
void reg_symphony_disp_set_di_motion_ctrl_4_motion_magnify(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_motion_ctrl_4_motion_magnify(void);

/*!
  register REGSYMPHONY_DISP_DI_CHROMA_PARA (read/write)
  */
void reg_symphony_disp_set_di_chroma_para(MT_U32 data);
MT_U32  reg_symphony_disp_get_di_chroma_para(void);
void reg_symphony_disp_set_di_chroma_para_g_alpha_k_chroma(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_chroma_para_g_alpha_k_chroma(void);
void reg_symphony_disp_set_di_chroma_para_g_alpha_0_chroma(MT_U8 data);
MT_U8   reg_symphony_disp_get_di_chroma_para_g_alpha_0_chroma(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_BURST_LENGTH_SEL (read/write)
  */
void reg_symphony_disp_set_video_burst_length_sel(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_burst_length_sel(void);
void reg_symphony_disp_set_video_burst_length_sel_vid_burst_length(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_length_sel_vid_burst_length(void);
void reg_symphony_disp_set_video_burst_length_sel_vid_rd_stride_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_length_sel_vid_rd_stride_sel(void);
void reg_symphony_disp_set_video_burst_length_sel_vid_linear_addr_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_length_sel_vid_linear_addr_en(void);
void reg_symphony_disp_set_video_burst_length_sel_v_half_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_length_sel_v_half_en(void);
void reg_symphony_disp_set_video_burst_length_sel_h_half_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_length_sel_h_half_en(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_BURST_INFO (read/write)
  */
void reg_symphony_disp_set_video_burst_info(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_burst_info(void);
void reg_symphony_disp_set_video_burst_info_last_burst_length(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_info_last_burst_length(void);
void reg_symphony_disp_set_video_burst_info_first_burst_length(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_info_first_burst_length(void);
void reg_symphony_disp_set_video_burst_info_line_rd_max(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_info_line_rd_max(void);
void reg_symphony_disp_set_video_burst_info_burst_info_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_burst_info_burst_info_en(void);

/*!
  register REGSYMPHONY_DISP_VIDEO_LINE_RD_CNT_MAX (read/write)
  */
void reg_symphony_disp_set_video_line_rd_cnt_max(MT_U32 data);
MT_U32  reg_symphony_disp_get_video_line_rd_cnt_max(void);
void reg_symphony_disp_set_video_line_rd_cnt_max_line_cnt_max(MT_U8 data);
MT_U8   reg_symphony_disp_get_video_line_rd_cnt_max_line_cnt_max(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_CMD (read/write)
  */
void reg_symphony_disp_set_prescale_cmd(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_cmd(void);
void reg_symphony_disp_set_prescale_cmd_prescale_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_cmd_prescale_en(void);
void reg_symphony_disp_set_prescale_cmd_prescale_field_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_cmd_prescale_field_mode(void);
void reg_symphony_disp_set_prescale_cmd_prescale_hscale_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_cmd_prescale_hscale_mode(void);
void reg_symphony_disp_set_prescale_cmd_prescale_vscale_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_cmd_prescale_vscale_mode(void);
void reg_symphony_disp_set_prescale_cmd_prescale_wr_stride_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_cmd_prescale_wr_stride_sel(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_FRAME_SIZE (read/write)
  */
void reg_symphony_disp_set_prescale_frame_size(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_frame_size(void);
void reg_symphony_disp_set_prescale_frame_size_prescale_frame_height(MT_U16 data);
MT_U16  reg_symphony_disp_get_prescale_frame_size_prescale_frame_height(void);
void reg_symphony_disp_set_prescale_frame_size_prescale_frame_width(MT_U16 data);
MT_U16  reg_symphony_disp_get_prescale_frame_size_prescale_frame_width(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_TILING_PARAMS (read/write)
  */
void reg_symphony_disp_set_prescale_tiling_params(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_tiling_params(void);
void reg_symphony_disp_set_prescale_tiling_params_prescale_field_pic(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_tiling_params_prescale_field_pic(void);
void reg_symphony_disp_set_prescale_tiling_params_prescale_col_size_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_tiling_params_prescale_col_size_mode(void);
void reg_symphony_disp_set_prescale_tiling_params_prescale_hd_map_mode(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_tiling_params_prescale_hd_map_mode(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_LUMA_RD_ADDR (read/write)
  */
void reg_symphony_disp_set_prescale_luma_rd_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_rd_addr(void);
void reg_symphony_disp_set_prescale_luma_rd_addr_luma_rd_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_rd_addr_luma_rd_addr(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_LUMA_RD_ADDR2 (read/write)
  */
void reg_symphony_disp_set_prescale_luma_rd_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_rd_addr2(void);
void reg_symphony_disp_set_prescale_luma_rd_addr2_luma_rd_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_rd_addr2_luma_rd_addr2(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_LUMA_WR_ADDR (read/write)
  */
void reg_symphony_disp_set_prescale_luma_wr_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_wr_addr(void);
void reg_symphony_disp_set_prescale_luma_wr_addr_luma_wr_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_wr_addr_luma_wr_addr(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_LUMA_WR_ADDR2 (read/write)
  */
void reg_symphony_disp_set_prescale_luma_wr_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_wr_addr2(void);
void reg_symphony_disp_set_prescale_luma_wr_addr2_luma_wr_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_luma_wr_addr2_luma_wr_addr2(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_CHROMA_RD_ADDR (read/write)
  */
void reg_symphony_disp_set_prescale_chroma_rd_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_rd_addr(void);
void reg_symphony_disp_set_prescale_chroma_rd_addr_chroma_rd_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_rd_addr_chroma_rd_addr(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_CHROMA_RD_ADDR2 (read/write)
  */
void reg_symphony_disp_set_prescale_chroma_rd_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_rd_addr2(void);
void reg_symphony_disp_set_prescale_chroma_rd_addr2_chroma_rd_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_rd_addr2_chroma_rd_addr2(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_CHROMA_WR_ADDR (read/write)
  */
void reg_symphony_disp_set_prescale_chroma_wr_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_wr_addr(void);
void reg_symphony_disp_set_prescale_chroma_wr_addr_chroma_wr_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_wr_addr_chroma_wr_addr(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_CHROMA_WR_ADDR2 (read/write)
  */
void reg_symphony_disp_set_prescale_chroma_wr_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_wr_addr2(void);
void reg_symphony_disp_set_prescale_chroma_wr_addr2_chroma_wr_addr2(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_chroma_wr_addr2_chroma_wr_addr2(void);

/*!
  register REGSYMPHONY_DISP_PRESCALE_FIFO_THR (read/write)
  */
void reg_symphony_disp_set_prescale_fifo_thr(MT_U32 data);
MT_U32  reg_symphony_disp_get_prescale_fifo_thr(void);
void reg_symphony_disp_set_prescale_fifo_thr_wr_fifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_fifo_thr_wr_fifo_thr(void);
void reg_symphony_disp_set_prescale_fifo_thr_proc_mode_sfifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_fifo_thr_proc_mode_sfifo_thr(void);
void reg_symphony_disp_set_prescale_fifo_thr_req_sfifo_thr(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_fifo_thr_req_sfifo_thr(void);
void reg_symphony_disp_set_prescale_fifo_thr_block_rate_cnt_max(MT_U8 data);
MT_U8   reg_symphony_disp_get_prescale_fifo_thr_block_rate_cnt_max(void);

/*!
  register REGSYMPHONY_DISP_HD_HUE_ADJUST (read/write)
  */
void reg_symphony_disp_set_hd_hue_adjust(MT_U32 data);
MT_U32  reg_symphony_disp_get_hd_hue_adjust(void);
void reg_symphony_disp_set_hd_hue_adjust_hd_sina(MT_U16 data);
MT_U16  reg_symphony_disp_get_hd_hue_adjust_hd_sina(void);
void reg_symphony_disp_set_hd_hue_adjust_hd_cosa(MT_U16 data);
MT_U16  reg_symphony_disp_get_hd_hue_adjust_hd_cosa(void);
void reg_symphony_disp_set_hd_hue_adjust_hd_hue_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_hd_hue_adjust_hd_hue_en(void);

/*!
  register REGSYMPHONY_DISP_SD_HUE_ADJUST (read/write)
  */
void reg_symphony_disp_set_sd_hue_adjust(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_hue_adjust(void);
void reg_symphony_disp_set_sd_hue_adjust_sd_sina(MT_U16 data);
MT_U16  reg_symphony_disp_get_sd_hue_adjust_sd_sina(void);
void reg_symphony_disp_set_sd_hue_adjust_sd_cosa(MT_U16 data);
MT_U16  reg_symphony_disp_get_sd_hue_adjust_sd_cosa(void);
void reg_symphony_disp_set_sd_hue_adjust_sd_hue_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_sd_hue_adjust_sd_hue_en(void);

/*!
  register REGSYMPHONY_DISP_HALF_SCALE_CTR (read/write)
  */
void reg_symphony_disp_set_half_scale_ctr(MT_U32 data);
MT_U32  reg_symphony_disp_get_half_scale_ctr(void);
void reg_symphony_disp_set_half_scale_ctr_h_enable(MT_U8 data);
MT_U8   reg_symphony_disp_get_half_scale_ctr_h_enable(void);
void reg_symphony_disp_set_half_scale_ctr_v_enable(MT_U8 data);
MT_U8   reg_symphony_disp_get_half_scale_ctr_v_enable(void);

/*!
  register REGSYMPHONY_DISP_PIX_ALIGN_CTR (read/write)
  */
void reg_symphony_disp_set_pix_align_ctr(MT_U32 data);
MT_U32  reg_symphony_disp_get_pix_align_ctr(void);
void reg_symphony_disp_set_pix_align_ctr_last_cut_pix_num(MT_U8 data);
MT_U8   reg_symphony_disp_get_pix_align_ctr_last_cut_pix_num(void);
void reg_symphony_disp_set_pix_align_ctr_pre_cut_pix_num(MT_U8 data);
MT_U8   reg_symphony_disp_get_pix_align_ctr_pre_cut_pix_num(void);

/*!
  register REGSYMPHONY_DISP_DISP_PROT_LIMIT (read/write)
  */
void reg_symphony_disp_set_disp_prot_limit(MT_U32 data);
MT_U32  reg_symphony_disp_get_disp_prot_limit(void);
void reg_symphony_disp_set_disp_prot_limit_back_prot_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_disp_prot_limit_back_prot_en(void);
void reg_symphony_disp_set_disp_prot_limit_still_prot_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_disp_prot_limit_still_prot_en(void);
void reg_symphony_disp_set_disp_prot_limit_vid_prot_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_disp_prot_limit_vid_prot_en(void);
void reg_symphony_disp_set_disp_prot_limit_osd0_rpot_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_disp_prot_limit_osd0_rpot_en(void);
void reg_symphony_disp_set_disp_prot_limit_osd1_prot_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_disp_prot_limit_osd1_prot_en(void);
void reg_symphony_disp_set_disp_prot_limit_sub_prot_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_disp_prot_limit_sub_prot_en(void);


/* MFBC function */
void reg_symphony_disp_set_luma_lut_pre_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_cur_top_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_cur_bot_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_nxt_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_ppre_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_pre_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_cur_top_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_cur_bot_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_nxt_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_pre_2_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_cur_top_2_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_cur_bot_2_addr(MT_U32 data);
void reg_symphony_disp_set_luma_lut_nxt_2_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_ppre_2_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_pre_2_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_cur_top_2_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_cur_bot_2_addr(MT_U32 data);
void reg_symphony_disp_set_chroma_lut_nxt_2_addr(MT_U32 data);
void reg_symphony_disp_set_decomp_ctrl_en(MT_U8 data);
void reg_symphony_disp_set_decomp_4k_rd_proc_en(MT_U8 data);
void reg_symphony_disp_set_luma_bit_depth(MT_U8 data);
void reg_symphony_disp_set_chroma_bit_depth(MT_U8 data);
void reg_symphony_disp_set_1bgs_luma_stride_data(MT_U16 data);
void reg_symphony_disp_set_1bgs_chroma_stride_data(MT_U16 data);
void reg_symphony_disp_set_pic_resolution_width(MT_U16 data);
void reg_symphony_disp_set_pic_resolution_height(MT_U16 data);

MT_U8 reg_symphony_disp_get_version_id_major(void);
MT_U8 reg_symphony_disp_get_version_id_minor(void);

MT_U8 reg_symphony_disp_get_decomp_ctrl_en(void);

void reg_symphony_disp_set_vid_layer_clk_gate_en(MT_U8 data);
MT_U8 reg_symphony_disp_get_vid_layer_clk_gate_en(void);

void reg_symphony_disp_set_vid_decomp_clk_gate_en(MT_U8 data);
MT_U8 reg_symphony_disp_get_vid_decomp_clk_gate_en(void);

void reg_symphony_disp_set_sd_vid_scaler_clk_gate_en(MT_U8 data);
MT_U8 reg_symphony_disp_get_sd_vid_scaler_clk_gate_en(void);

//new csc
void reg_symphony_disp_set_new_csc_coef_table_1_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_table_1_addr(void);
void reg_symphony_disp_set_new_csc_coef_table_2_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_table_2_addr(void);

void reg_symphony_disp_set_sd_new_csc_lite_rgb_out_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_new_csc_lite_rgb_out_addr(void);
void reg_symphony_disp_set_sd_new_csc_lite_rgb_in_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_sd_new_csc_lite_rgb_in_addr(void);

void reg_symphony_disp_set_new_csc_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_ctrl(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_new_csc_en(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_bound_output_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_new_csc_bound_output_en(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_bound_input_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_new_csc_bound_input_en(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_shift_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_new_csc_shift_en(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_or_old_csc(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_new_csc_or_old_csc(void);
void reg_symphony_disp_set_new_csc_ctrl_sd_new_csc_or_old_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_sd_new_csc_or_old_csc_en(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_in_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_new_csc_in_sel(void);
void reg_symphony_disp_set_new_csc_ctrl_new_csc_factor(MT_U16 data);
MT_U16  reg_symphony_disp_get_new_csc_ctrl_new_csc_factor(void);
void reg_symphony_disp_set_new_csc_ctrl_color_correction_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_ctrl_color_correction_flag(void);

void reg_symphony_disp_set_new_csc_coef_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_1(void);
void reg_symphony_disp_set_new_csc_coef_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_2(void);
void reg_symphony_disp_set_new_csc_coef_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_3(void);
void reg_symphony_disp_set_new_csc_coef_4(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_4(void);
void reg_symphony_disp_set_new_csc_coef_5(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_5(void);
void reg_symphony_disp_set_new_csc_coef_6(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_6(void);
void reg_symphony_disp_set_new_csc_coef_7(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_7(void);
void reg_symphony_disp_set_new_csc_coef_8(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_8(void);
void reg_symphony_disp_set_new_csc_coef_9(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_9(void);
void reg_symphony_disp_set_new_csc_coef_10(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_10(void);
void reg_symphony_disp_set_new_csc_coef_11(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_11(void);
void reg_symphony_disp_set_new_csc_coef_12(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_12(void);
void reg_symphony_disp_set_new_csc_coef_13(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_13(void);
void reg_symphony_disp_set_new_csc_coef_14(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_14(void);

void reg_symphony_disp_set_new_csc_coef_table_3_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_table_3_addr(void);
void reg_symphony_disp_set_new_csc_coef_table_4_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_table_4_addr(void);
void reg_symphony_disp_set_new_csc_coef_table_5_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_table_5_addr(void);
void reg_symphony_disp_set_new_csc_coef_table_6_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_coef_table_6_addr(void);

void reg_symphony_disp_set_new_csc_lite_gain_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_lite_gain_addr(void);
void reg_symphony_disp_set_osd_new_csc_lite_gain_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_lite_gain_addr(void);
void reg_symphony_disp_set_osd_new_csc_lite_rgb_out_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_lite_rgb_out_addr(void);
void reg_symphony_disp_set_osd_new_csc_lite_rgb_in_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_lite_rgb_in_addr(void);

void reg_symphony_disp_set_osdm_csc_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_osdm_csc_ctrl(void);
void reg_symphony_disp_set_osdm_csc_ctrl_enable(MT_U8 data);
MT_U8   reg_symphony_disp_get_osdm_csc_ctrl_enable(void);
void reg_symphony_disp_set_osdm_csc_coef01(MT_U32 data);
MT_U32  reg_symphony_disp_get_osdm_csc_coef01(void);
void reg_symphony_disp_set_osdm_csc_coef23(MT_U32 data);
MT_U32  reg_symphony_disp_get_osdm_csc_coef23(void);
void reg_symphony_disp_set_osdm_csc_coef45(MT_U32 data);
MT_U32  reg_symphony_disp_get_osdm_csc_coef45(void);
void reg_symphony_disp_set_osdm_csc_coef67(MT_U32 data);
MT_U32  reg_symphony_disp_get_osdm_csc_coef67(void);
void reg_symphony_disp_set_osdm_csc_coef8(MT_U32 data);
MT_U32  reg_symphony_disp_get_osdm_csc_coef8(void);

/*!
  register REG_SYMPHONY_DISP_SUB_CSC_CTRL (read/write)
  */
void reg_symphony_disp_set_sub_csc_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_csc_ctrl(void);
void reg_symphony_disp_set_sub_csc_ctrl_enable(MT_U8 data);
MT_U8   reg_symphony_disp_get_sub_csc_ctrl_enable(void);
void reg_symphony_disp_set_sub_csc_coef01(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_csc_coef01(void);
void reg_symphony_disp_set_sub_csc_coef23(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_csc_coef23(void);
void reg_symphony_disp_set_sub_csc_coef45(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_csc_coef45(void);
void reg_symphony_disp_set_sub_csc_coef67(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_csc_coef67(void);
void reg_symphony_disp_set_sub_csc_coef8(MT_U32 data);
MT_U32  reg_symphony_disp_get_sub_csc_coef8(void);

void reg_symphony_disp_set_adv_csc_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_ctrl(void);
void reg_symphony_disp_set_adv_csc_ctrl_adv_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_adv_csc_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_adv_csc_bound_output_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_adv_csc_bound_output_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_rgb_in(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_rgb_in(void);
void reg_symphony_disp_set_adv_csc_ctrl_adv_csc_bound_input_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_adv_csc_bound_input_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_adv_csc_shift_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_adv_csc_shift_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_adv_csc_or_old_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_adv_csc_or_old_csc_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_sd_adv_csc_or_old_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_sd_adv_csc_or_old_csc_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_adv_csc_in_sel(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_adv_csc_in_sel(void);
void reg_symphony_disp_set_adv_csc_ctrl_osd_adv_csc_or_new_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_osd_adv_csc_or_new_csc_en(void);
void reg_symphony_disp_set_adv_csc_ctrl_osd_alpha_div_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_adv_csc_ctrl_osd_alpha_div_en(void);

void reg_symphony_disp_set_adv_csc_coef_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_1(void);
void reg_symphony_disp_set_adv_csc_coef_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_2(void);
void reg_symphony_disp_set_adv_csc_coef_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_3(void);
void reg_symphony_disp_set_adv_csc_coef_4(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_4(void);
void reg_symphony_disp_set_adv_csc_coef_5(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_5(void);
void reg_symphony_disp_set_adv_csc_coef_6(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_6(void);
void reg_symphony_disp_set_adv_csc_coef_7(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_7(void);
void reg_symphony_disp_set_adv_csc_coef_8(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_8(void);
void reg_symphony_disp_set_adv_csc_coef_9(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_9(void);
void reg_symphony_disp_set_adv_csc_coef_10(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_10(void);
void reg_symphony_disp_set_adv_csc_coef_11(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_11(void);
void reg_symphony_disp_set_adv_csc_coef_12(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_coef_12(void);

void reg_symphony_disp_set_adv_csc_lut_table_addr(MT_U32 data);
MT_U32  reg_symphony_disp_get_adv_csc_lut_table_addr(void);

void reg_symphony_disp_set_osd_new_csc_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_ctrl(void);
void reg_symphony_disp_set_osd_new_csc_ctrl_new_csc_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_new_csc_ctrl_new_csc_en(void);
void reg_symphony_disp_set_osd_new_csc_ctrl_new_csc_bound_output_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_new_csc_ctrl_new_csc_bound_output_en(void);
void reg_symphony_disp_set_osd_new_csc_ctrl_new_csc_bound_input_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_new_csc_ctrl_new_csc_bound_input_en(void);
void reg_symphony_disp_set_osd_new_csc_ctrl_new_csc_shift_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_new_csc_ctrl_new_csc_shift_en(void);
void reg_symphony_disp_set_osd_new_csc_ctrl_color_correction_flag(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_new_csc_ctrl_color_correction_flag(void);
void reg_symphony_disp_set_osd_new_csc_ctrl_osd_new_csc_factor(MT_U16 data);
MT_U16  reg_symphony_disp_get_osd_new_csc_ctrl_osd_new_csc_factor(void);

void reg_symphony_disp_set_new_csc_gain_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_new_csc_gain_ctrl(void);
void reg_symphony_disp_set_new_csc_gain_ctrl_pix_gain_sat_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_new_csc_gain_ctrl_pix_gain_sat_en(void);
void reg_symphony_disp_set_new_csc_gain_ctrl_sat_value(MT_U16 data);
MT_U16  reg_symphony_disp_get_new_csc_gain_ctrl_sat_value(void);

void reg_symphony_disp_set_osd_new_csc_coef_1(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_1(void);
void reg_symphony_disp_set_osd_new_csc_coef_2(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_2(void);
void reg_symphony_disp_set_osd_new_csc_coef_3(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_3(void);
void reg_symphony_disp_set_osd_new_csc_coef_4(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_4(void);
void reg_symphony_disp_set_osd_new_csc_coef_5(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_5(void);
void reg_symphony_disp_set_osd_new_csc_coef_6(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_6(void);
void reg_symphony_disp_set_osd_new_csc_coef_7(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_7(void);
void reg_symphony_disp_set_osd_new_csc_coef_8(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_8(void);
void reg_symphony_disp_set_osd_new_csc_coef_9(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_9(void);
void reg_symphony_disp_set_osd_new_csc_coef_10(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_10(void);
void reg_symphony_disp_set_osd_new_csc_coef_11(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_11(void);
void reg_symphony_disp_set_osd_new_csc_coef_12(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_12(void);
void reg_symphony_disp_set_osd_new_csc_coef_13(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_13(void);
void reg_symphony_disp_set_osd_new_csc_coef_14(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_coef_14(void);

void reg_symphony_disp_set_osd_new_csc_gain_ctrl(MT_U32 data);
MT_U32  reg_symphony_disp_get_osd_new_csc_gain_ctrl(void);
void reg_symphony_disp_set_osd_new_csc_gain_ctrl_pix_gain_sat_en(MT_U8 data);
MT_U8   reg_symphony_disp_get_osd_new_csc_gain_ctrl_pix_gain_sat_en(void);

void reg_symphony_disp_set_tile_col_size(MT_U32 data);
MT_U32  reg_symphony_disp_get_tile_col_size(void);

/*!
  SYMPHONY_DISP reg init function
  */
void reg_symphony_disp_init(void);


#ifdef __cplusplus
}
#endif

#endif /* _IP_DISPLAY_0518_WUKE_H */

