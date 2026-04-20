/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_AIAO_FUNC_H__
#define __MT_AIAO_FUNC_H__

#include "mt_type.h"
#include "hal_aoe_common.h"
//#include "mt_audsp_aoe.h"
#include "mt_drv_ao.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

/***************************** Macro Definition ******************************/

/* global function */
/* global function */
mt_s32					iHAL_AOE_Init(MT_BOOL bSwAoeFlag);
mt_void					iHAL_AOE_DeInit(mt_void);
mt_void					iHAL_AOE_GetHwCapability(mt_u32 *pu32Capability);
mt_void					iHAL_AOE_GetHwVersion(mt_u32 *pu32Version);

/* AIP function */
mt_s32					iHAL_AOE_AIP_Create(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr);
mt_void					iHAL_AOE_AIP_Destroy(AOE_AIP_ID_E enAIP);
mt_s32					iHAL_AOE_AIP_SetAttr(AOE_AIP_ID_E enAIP, AOE_AIP_CHN_ATTR_S *pstAttr);
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32                  iHAL_Init_Out_Buf_Reg(AOE_AIP_CHN_ATTR_NEW_S *pstAttr, AOE_AIP_CHN_ATTR_NEW_S *pstAttr2, AOE_AIP_CHN_ATTR_NEW_S *pstAttr3, MT_BOOL b_spdif);
#else
mt_s32                  iHAL_Init_Out_Buf_Reg(AOE_AIP_CHN_ATTR_NEW_S *pstAttr, AOE_AIP_CHN_ATTR_NEW_S *pstAttr2, MT_BOOL b_spdif);
#endif
void 					iHAL_AOE_AIP_SetPcmSampleRate(mt_u32 sample);
//mt_s32					iHAL_AOE_AIP_SetCmd(AOE_AIP_ID_E enAIP, AOE_AIP_CMD_E newcmd);
//mt_u32                  iHAL_AOE_AIP_Group_SetCmd(AOE_AIP_CMD_E newcmd, mt_u32 u32SetCmdMask);
mt_void					iHAL_AOE_AIP_SetVolume(AOE_AIP_ID_E enAIP, mt_u32 u32VolumedB);
mt_s32					iHAL_AOE_AIP_SetSpeed(AOE_AIP_ID_E enAIP, mt_s32 u32AdjSpeed);
mt_s32					iHAL_AOE_AIP_GetStatus(AOE_AIP_ID_E enAIP, mt_void *pstStatus);
mt_void 				iHAL_AOE_AIP_SetLRVolume(AOE_AIP_ID_E enAIP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB);
mt_void                 iHAL_AOE_AIP_SetMute(AOE_AIP_ID_E enAIP, MT_BOOL bMute);
mt_void 				iHAL_AOE_AIP_SetChannelMode(AOE_AIP_ID_E enAIP, mt_u32 u32ChannelMode);

//mt_u32					iHAL_AOE_AIP_WriteData(AOE_AIP_ID_E enAIP, mt_u8 * pu32Src, mt_u32 u32SrcBytes);
//mt_u32					iHAL_AOE_AIP_QueryBufData(AOE_AIP_ID_E enAIP);
//mt_u32					iHAL_AOE_AIP_QueryBufFree(AOE_AIP_ID_E enAIP);
mt_u32                  iHAL_AOE_AIP_GetFiFoDelayMs(AOE_AIP_ID_E enAIP);
mt_void					iHAL_AOE_AIP_GetRptrAndWptrRegAddr(AOE_AIP_ID_E enAIP, mt_u32 *pu32WptrReg, mt_u32 *pu32RptrReg);
mt_void                 iHAL_AOE_AIP_ReSetRptrAndWptrReg(AOE_AIP_ID_E enAIP);
//for ALSA
mt_void                 iHAL_AOE_AIP_EnableAlsa(AOE_AIP_ID_E enAIP, MT_BOOL bEnableAlsa);           //USED

/* AOP function */
mt_void                 iHAL_AOE_AOP_SetLRVolume(AOE_AOP_ID_E enAOP, mt_u32 u32VolumeLdB, mt_u32 u32VolumeRdB);
mt_void                 iHAL_AOE_AOP_SetMute(AOE_AOP_ID_E enAOP, MT_BOOL bMute);

mt_s32					iHAL_AOE_AOP_Create(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr);
mt_void					iHAL_AOE_AOP_Destroy(AOE_AOP_ID_E enAOP);
mt_s32					iHAL_AOE_AOP_SetAttr(AOE_AOP_ID_E enAOP, AOE_AOP_CHN_ATTR_S *pstAttr);
mt_void					iHAL_AOE_AOP_GetRptrAndWptrRegAddr(AOE_AOP_ID_E enAOP, mt_u32 *pu32WptrReg, mt_u32 *pu32RptrReg);
//mt_s32					iHAL_AOE_AOP_SetCmd(AOE_AOP_ID_E enAOP, AOE_AOP_CMD_E newcmd);
mt_s32					iHAL_AOE_AOP_GetStatus(AOE_AOP_ID_E enAOP, mt_void *pstStatus);
mt_s32                  iHAL_AOE_AOP_SetAefBypass(AOE_AOP_ID_E enAOP, MT_BOOL bBypass);
#if 0   //not Mirror Here
mt_u32					iHAL_AOE_AOP_ReadData(AOE_AOP_ID_E enAOP, mt_u8 * pu32Dest, mt_u32 u32DestSize);  // mirror
mt_u32					iHAL_AOE_AOP_QueryBufData(AOE_AOP_ID_E enAOP);
mt_u32					iHAL_AOE_AOP_QueryBufFree(AOE_AOP_ID_E enAOP);
#endif

/* ENGINE function */
mt_s32					iHAL_AOE_ENGINE_Create(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S *pstAttr);
mt_void					iHAL_AOE_ENGINE_Destroy(AOE_ENGINE_ID_E enENGINE);
//mt_s32					iHAL_AOE_ENGINE_SetCmd(AOE_ENGINE_ID_E enEngine, AOE_ENGINE_CMD_E newcmd);
mt_s32					iHAL_AOE_ENGINE_GetStatus(AOE_ENGINE_ID_E enENGINE, mt_void *pstStatus);
mt_s32					iHAL_AOE_ENGINE_SetAttr(AOE_ENGINE_ID_E enENGINE, AOE_ENGINE_CHN_ATTR_S stAttr);
mt_s32					iHAL_AOE_ENGINE_AttachAip(AOE_ENGINE_ID_E enENGINE, AOE_AIP_ID_E enAIP);
mt_s32					iHAL_AOE_ENGINE_DetachAip(AOE_ENGINE_ID_E enENGINE, AOE_AIP_ID_E enAIP);
mt_s32					iHAL_AOE_ENGINE_AttachAop(AOE_ENGINE_ID_E enENGINE, AOE_AOP_ID_E enAOP);
mt_s32					iHAL_AOE_ENGINE_DetachAop(AOE_ENGINE_ID_E enENGINE, AOE_AOP_ID_E enAOP);
mt_s32                  iHAL_AOE_ENGINE_AttachAef(AOE_ENGINE_ID_E enEngine, mt_u32 u32AefId);
mt_s32                  iHAL_AOE_ENGINE_DetachAef(AOE_ENGINE_ID_E enEngine, mt_u32 u32AefId);


extern ulong g_audio_reg_base;

#define AUDIO_VOL_COEF_FULL_REG_32BIT 0x80008000
#define AUDIO_VOL_COEF_HALF_REG_32BIT 0x40004000

#define AUDIO_VOL_COEF_FULL_REG 0x8000

#define AUDIO_VOL_COEF_HALF_REG (AUDIO_VOL_COEF_FULL_REG / 2)


#define PP_BUF_CONFIG_LEN (96 * 1024) // 96K  8K * 8CHAN * 1.5


/*!
  the enum of AUDIO_OUT registers
  */
enum
{
    REG_AUD_SNT_CH_SRT                  = 0x0,
    REG_AUD_SNT_I2S_SPDIF               = 0x4,
    REG_AUD_SNT_VOL                     = 0x8,
    REG_AUD_SNT_PP_EN                   = 0xc,
    REG_AUD_SNT_CLK_DIV                 = 0x10,
    REG_AUD_SNT_CLK_ADJ                 = 0x14,
    REG_AUD_SNT_PCM_BUF_BASE            = 0x18,
    REG_AUD_SNT_PCM_BUF_LEN             = 0x1c,
    REG_AUD_SNT_PCM_BUF_FUL_THD         = 0x20,
    REG_AUD_SNT_PCM_BUF_WRCMD           = 0x24,
    REG_AUD_SNT_PP_BUF_BASE             = 0x28,
    REG_AUD_SNT_PP_ONE_BUF_LEN          = 0x2c,
    REG_AUD_SNT_PP_BUF_FUL_THD          = 0x30,
    REG_AUD_SNT_PP_BUF_WRCMD            = 0x34,
    REG_AUD_SNT_SPD_BUF_BASE            = 0x38,
    REG_AUD_SNT_SPD_BUF_LEN             = 0x3c,
    REG_AUD_SNT_SPD_BUF_FUL_THD         = 0x40,
    REG_AUD_SNT_SPD_BUF_WRCMD           = 0x44,
    REG_AUD_SNT_PCM_BUF_JUMP_EN         = 0xbfd40048,
    REG_AUD_SNT_SRC                     = 0x4c,
    REG_AUD_SNT_AGC0                    = 0xbfd40050,
    REG_AUD_SNT_AGC1                    = 0xbfd40054,
    REG_AUD_SNT_AGC2                    = 0xbfd40058,
    REG_AUD_SNT_FADER                   = 0x5c,
    REG_AUD_SNT_DOWNMIX_COEF_LEFT       = 0x60,
    REG_AUD_SNT_DOWNMIX_COEF_RIGHT      = 0x64,
    REG_AUD_SNT_DOWNMIX_COEF_BASS       = 0x68,
    REG_AUD_SNT_DOWNMIX_COEF_CENTER     = 0x6c,
    REG_AUD_SNT_DOWNMIX_COEF_SL         = 0x70,
    REG_AUD_SNT_DOWNMIX_COEF_SR         = 0x74,
    REG_AUD_SNT_DOWNMIX_COEF_RSL        = 0x78,
    REG_AUD_SNT_DOWNMIX_COEF_RSR        = 0x7c,
    REG_AUD_SNT_SYNC_RESET              = 0x80,
    REG_AUD_SNT_DATA_PRELOAD            = 0xbfd40084,
    REG_AUD_SNT_CLK_DIV2                = 0x88,
    REG_AUD_SNT_CLK_ADJ2                = 0xbfd4008c,
    REG_AUD_SNT_SAMP_NUM_FRM            = 0x90,
    REG_AUD_SNT_PPBUFW_INTR_TIMER       = 0xbfd40094,
    REG_AUD_SNT_BUFRW_INTR_TIMER        = 0xbfd40098,
    REG_AUD_SNT_PCMFIFO_INTR_TIMER      = 0xbfd4009c,
    REG_AUD_SNT_INTR_SET                = 0xbfd400a0,
    REG_AUD_SNT_SPD_BUF_JUMP_EN         = 0xbfd400a4,
    REG_AUD_SNT_INDIVIDUAL_VOL_CTL         = 0xbfd400a8,
    REG_AUD_SNT_AUD_VOL_CFG_SPDIF       = 0xB0,
    REG_AUD_SNT_MIX_BUF_BASE_ADDR       = 0xC0,
    REG_AUD_SNT_MIX_BUF_LEN                     = 0xC4,
    REG_AUD_SNT_MIX_BUF_WRCMD              = 0xCC,
    REG_AUD_SNT_MIX_FUNC_CONFIG            = 0xD0,
    REG_AUD_SNT_MIX_FUNC_ALPHA            = 0xD4,
    REG_AUD_SNT_AUD_PLAY_STOP           = 0xDC,
    REG_AUD_SNT_PCM_BUF_CNT                 = 0x104,
    REG_AUD_SNT_AUD_PP_BUF0_CNT         = 0x114,
    REG_AUD_SNT_AUD_PP_BUF1_CNT         = 0x118,
    REG_AUD_SNT_SPDIF_BUF_CNT              = 0x128,    
    REG_AUD_SNT_AUD_BUF_FULL_FLAG       = 0x12c,
    REG_AUD_SNT_MIX_BUF_CNT                  = 0x148, 
    REG_AUD_SNT_AUD_PP_BUF2_CNT         = 0x150,
    REG_AUD_SNT_AUD_PP_BUF3_CNT         = 0x154,
    REG_AUD_SNT_AUD_PP_BUF4_CNT         = 0x158,
    REG_AUD_SNT_AUD_PP_BUF5_CNT         = 0x15c,
    REG_AUD_SNT_AUD_PP_BUF6_CNT         = 0x160,
    REG_AUD_SNT_AUD_PP_BUF7_CNT         = 0x164,
    REG_AUD_SNT_AUD_RES_REG0                = 0x200,
    REG_AUD_SNT_AUD_RES_REG3                = 0x20c,
};

/*!
  the union of register reg_aud_snt_ch_srt
  */
typedef union reg_aud_snt_ch_srt
{
    u32 all;
    struct
    {
        u32 sample_rate                 : 4;
        u32 pcm_32b_flag                : 1;
        u32                             : 3;
        u32 ch_input_mode               : 8;
        u32                             : 16;
    } bitc;
} reg_aud_snt_ch_srt_t;

/*!
  the union of register reg_aud_snt_i2s_spdif
  */
typedef union reg_aud_snt_i2s_spdif
{
    u32 all;
    struct
    {
        u32 right_flag                  : 1;
        u32 justified_mode              : 2;
        u32                             : 1;
        u32 i2s_ch_sel                  : 3;
        u32                             : 1;
        u32 spdif_audout_buf            : 1;
        u32 spdif_spd_buf               : 1;
        u32 spdif_path_sel              : 1;  //spdif
        u32 spdif_path_hdmi             : 1;  //sym4.hdmi:bit11
        u32 dac_ch_sel                  : 3;
        u32                             : 1;
        u32 dac_vld_cnt                 : 7;
        u32 adc_vld_flg                 : 1;
        u32 reserved                    : 8;
    } bitc;
} reg_aud_snt_i2s_spdif_t;

/*!
  the union of register reg_aud_snt_vol
  */
typedef union reg_aud_snt_vol
{
    u32 all;
    struct
    {
        u32 volume_scale                : 16;
        u32 gainq                       : 3;
        u32                             : 2;
        u32 spdif_hdmi_mute                  : 1;
        u32 spdif_coax_mute                  : 1;
        u32 adec_2ch_mute                  : 1;
        u32 left_mute                   : 1;
        u32 right_mute                  : 1;
        u32 bass_mute                   : 1;
        u32 center_mute                 : 1;
        u32 sl_mute                     : 1;
        u32 sr_mute                     : 1;
        u32 rsl_mute                    : 1;
        u32 rsr_mute                    : 1;
    } bitc;
} reg_aud_snt_vol_t;

/*!
  the union of register reg_aud_snt_pp_en
  */
typedef union reg_aud_snt_pp_en
{
    u32 all;
    struct
    {
        u32 src_en                      : 1;
        u32 downmix_en                  : 1;
        u32 agc_en                     : 1;
        u32 chan_mod_pcm          : 1;        
        u32 ahbdata_en                : 2; 
        u32             : 1;
        u32 old_src_flag            : 1;
        u32 downmix2hdmi            : 1;
        u32                             : 3;
        u32 audio_ch_copy               : 2;
        u32                             : 2;
        u32 mono_copy                   : 2;
        u32                             : 2;
        u32 pcmbuf_axi_burstsize        : 1;
        u32                             : 11;
    } bitc;
} reg_aud_snt_pp_en_t;

/*!
  the union of register reg_aud_snt_clk_div
  */
typedef union reg_aud_snt_clk_div
{
    u32 all;
    struct
    {
        u32 clk_divider_factor          : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_clk_div_t;

/*!
  the union of register reg_aud_snt_clk_adj
  */
typedef union reg_aud_snt_clk_adj
{
    u32 all;
    struct
    {
        u32 sample_rate_offset          : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_clk_adj_t;

/*!
  the union of register reg_aud_snt_pcm_buf_base
  */
typedef union reg_aud_snt_pcm_buf_base
{
    u32 all;
    struct
    {
        u32 pcm_buf_base                : 26;
        u32                             : 6;
    } bitc;
} reg_aud_snt_pcm_buf_base_t;

/*!
  the union of register reg_aud_snt_pcm_buf_len
  */
typedef union reg_aud_snt_pcm_buf_len
{
    u32 all;
    struct
    {
        u32 pcm_buf_len                 : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_pcm_buf_len_t;

/*!
  the union of register reg_aud_snt_mix_buf_base
  */
typedef union reg_aud_snt_mix_buf_base
{
    u32 all;
    struct
    {
        u32 mix_buf_base                : 26;
        u32                             : 6;
    } bitc;
} reg_aud_snt_mix_buf_base_t;

/*!
  the union of register reg_aud_snt_mix_buf_len
  */
typedef union reg_aud_snt_mix_buf_len
{
    u32 all;
    struct
    {
        u32 mix_buf_len                 : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_mix_buf_len_t;

/*!
  the union of register reg_aud_snt_mix_func_config
  */
typedef union reg_aud_snt_mix_func_config
{
    u32 all;
    struct
    {
        u32 mix_func_en      : 1;
        u32 ssrc_enable        : 1;
        u32                             : 2;
        u32 ssrc_int               : 3;
        u32                             : 1;
        u32 ssrc_frac             : 12;
        u32                             : 12;
    } bitc;
} reg_aud_snt_mix_func_config_t;

/*!
  the union of register reg_aud_vol_cfg_spdif
  */
typedef union reg_aud_vol_cfg_spdif
{
    u32 all;
    struct
    {
        u32 apdif_pcm_vol_ctrl      : 16;
        u32 mix_spdif_alpha        : 16;
    } bitc;
} reg_aud_vol_cfg_spdif_t;

/*!
  the union of register reg_mix_func_alpha
  */
typedef union reg_aud_mix_func_alpha
{
    u32 all;
    struct
    {
        u32 mix_hdmi_alpha      : 16;
        u32 mix_adac_alpha        : 16;
    } bitc;
} reg_aud_mix_func_alpha_t;

/*!
  the union of register reg_snt_res_regs
  */
typedef union reg_aud_snt_res_regs
{
    u32 all;
    struct
    {
        u32                             : 32;
    } bitc;
} reg_aud_snt_res_regs_t;

/*!
  the union of register reg_aud_snt_pcm_buf_ful_thd
  */
typedef union reg_aud_snt_pcm_buf_ful_thd
{
    u32 all;
    struct
    {
        u32 audout_buf_full_thd         : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_pcm_buf_ful_thd_t;

/*!
  the union of register reg_aud_snt_pcm_buf_wrcmd
  */
typedef union reg_aud_snt_pcm_buf_wrcmd
{
    u32 all;
    struct
    {
        u32 pcm_buf_wr_len              : 25;
        u32                             : 6;
        u32 pcm_buf_wr_cmd              : 1;
    } bitc;
} reg_aud_snt_pcm_buf_wrcmd_t;

/*!
  the union of register reg_aud_snt_pp_buf_base
  */
typedef union reg_aud_snt_pp_buf_base
{
    u32 all;
    struct
    {
        u32 pp_buf_base                 : 26;
        u32                             : 6;
    } bitc;
} reg_aud_snt_pp_buf_base_t;

/*!
  the union of register reg_aud_snt_pp_one_buf_len
  */
typedef union reg_aud_snt_pp_one_buf_len
{
    u32 all;
    struct
    {
        u32 pp_one_buf_len              : 20;
        u32                             : 12;
    } bitc;
} reg_aud_snt_pp_one_buf_len_t;

/*!
  the union of register reg_aud_snt_pp_buf_ful_thd
  */
typedef union reg_aud_snt_pp_buf_ful_thd
{
    u32 all;
    struct
    {
        u32 pp_buf_ful_thd              : 20;
        u32                             : 12;
    } bitc;
} reg_aud_snt_pp_buf_ful_thd_t;

/*!
  the union of register reg_aud_snt_pp_buf_wrcmd
  */
typedef union reg_aud_snt_pp_buf_wrcmd
{
    u32 all;
    struct
    {
        u32 pp_buf_wr_len               : 20;
        u32                             : 4;
        u32 pp_buf_wr_sel               : 3;
        u32                             : 4;
        u32 pp_buf_wr_cmd               : 1;
    } bitc;
} reg_aud_snt_pp_buf_wrcmd_t;

/*!
  the union of register reg_aud_snt_spd_buf_base
  */
typedef union reg_aud_snt_spd_buf_base
{
    u32 all;
    struct
    {
        u32 spd_buf_base                : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_spd_buf_base_t;

/*!
  the union of register reg_aud_snt_spd_buf_len
  */
typedef union reg_aud_snt_spd_buf_len
{
    u32 all;
    struct
    {
        u32 spd_buf_len                 : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_spd_buf_len_t;

/*!
  the union of register reg_aud_snt_spd_buf_ful_thd
  */
typedef union reg_aud_snt_spd_buf_ful_thd
{
    u32 all;
    struct
    {
        u32 spd_buf_ful_thd             : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_spd_buf_ful_thd_t;

/*!
  the union of register reg_aud_snt_spd_buf_wrcmd
  */
typedef union reg_aud_snt_spd_buf_wrcmd
{
    u32 all;
    struct
    {
        u32 spdbuf_wr_len               : 25;
        u32                             : 6;
        u32 spdbuf_wr_cmd               : 1;
    } bitc;
} reg_aud_snt_spd_buf_wrcmd_t;

/*!
  the union of register reg_aud_snt_pcm_buf_jump_en
  */
typedef union reg_aud_snt_pcm_buf_jump_en
{
    u32 all;
    struct
    {
        u32 pcm_buf_jump_cnt            : 25;
        u32                             : 6;
        u32 pcm_buf_jump_en             : 1;
    } bitc;
} reg_aud_snt_pcm_buf_jump_en_t;

/*!
  the union of register reg_aud_snt_src
  */
typedef union reg_aud_snt_src
{
    u32 all;
    struct
    {
        u32 src_coef                    : 21;
        u32 coef_address                : 9;
        u32                             : 1;
        u32 coef_wr_en                  : 1;
    } bitc;
} reg_aud_snt_src_t;

/*!
  the union of register reg_aud_snt_agc0
  */
typedef union reg_aud_snt_agc0
{
    u32 all;
    struct
    {
        u32 env_mini_cnt                : 8;
        u32 attack_time                 : 3;
        u32                             : 1;
        u32 release_time                : 3;
        u32                             : 1;
        u32 attack_step                 : 2;
        u32 release_step                : 2;
        u32 target_level                : 12;
    } bitc;
} reg_aud_snt_agc0_t;

/*!
  the union of register reg_aud_snt_agc1
  */
typedef union reg_aud_snt_agc1
{
    u32 all;
    struct
    {
        u32 gain_max                    : 14;
        u32                             : 2;
        u32 gain_min                    : 14;
        u32 chan_restriction              : 1;
        u32 fs_restriction              : 1;
    } bitc;
} reg_aud_snt_agc1_t;

/*!
  the union of register reg_aud_snt_agc2
  */
typedef union reg_aud_snt_agc2
{
    u32 all;
    struct
    {
        u32 attack_level                : 10;
        u32                             : 6;
        u32 release_level               : 10;
        u32                             : 6;
    } bitc;
} reg_aud_snt_agc2_t;

/*!
  the union of register reg_aud_snt_fader
  */
typedef union reg_aud_snt_fader
{
    u32 all;
    struct
    {
        u32 fader_enable                : 1;
        u32 fade_in                     : 1;
        u32 fade_step                   : 2;
        u32 target_gain                 : 12;
        u32 fade_timestep               : 16;
    } bitc;
} reg_aud_snt_fader_t;

/*!
  the union of register reg_aud_snt_downmix_coef_left
  */
typedef union reg_aud_snt_downmix_coef_left
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_left_t;

/*!
  the union of register reg_aud_snt_downmix_coef_right
  */
typedef union reg_aud_snt_downmix_coef_right
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_right_t;

/*!
  the union of register reg_aud_snt_downmix_coef_bass
  */
typedef union reg_aud_snt_downmix_coef_bass
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_bass_t;

/*!
  the union of register reg_aud_snt_downmix_coef_center
  */
typedef union reg_aud_snt_downmix_coef_center
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_center_t;

/*!
  the union of register reg_aud_snt_downmix_coef_sl
  */
typedef union reg_aud_snt_downmix_coef_sl
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_sl_t;

/*!
  the union of register reg_aud_snt_downmix_coef_sr
  */
typedef union reg_aud_snt_downmix_coef_sr
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_sr_t;

/*!
  the union of register reg_aud_snt_downmix_coef_rsl
  */
typedef union reg_aud_snt_downmix_coef_rsl
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_rsl_t;

/*!
  the union of register reg_aud_snt_downmix_coef_rsr
  */
typedef union reg_aud_snt_downmix_coef_rsr
{
    u32 all;
    struct
    {
        u32 to_left                     : 16;
        u32 to_right                    : 16;
    } bitc;
} reg_aud_snt_downmix_coef_rsr_t;

/*!
  the union of register reg_aud_play_stop
  */
typedef union reg_aud_play_stop
{
    u32 all;
    struct
    {
        u32 pcm_play_stop               : 1;
        u32                             : 31;
    } bitc;
} reg_aud_play_stop_t;

/*!
  the union of register reg_aud_snt_sync_reset
  */
typedef union reg_aud_snt_sync_reset
{
    u32 all;
    struct
    {
        u32 sync_reset                  : 1;    //pp and pcm buf
        u32 mix_reset                    :1;    //mix buf reset
        u32                             : 2;
        u32 spdif_reset               : 1;      //spdif buf reset
        u32                             : 19;
        u32 sync_reset_wait                  : 1;    //wait for  pp and pcm buf      
        u32 mix_reset_wait                    :1;    //wait for mix buf reset
        u32                             : 2;
        u32 spdif_reset_wait               : 1;      //wait for spdif buf reset
        u32                             : 3;
    } bitc;
} reg_aud_snt_sync_reset_t;

/*!
  the union of register reg_aud_snt_data_preload
  */
typedef union reg_aud_snt_data_preload
{
    u32 all;
    struct
    {
        u32 ppbuf_preload_en            : 1;
        u32                             : 1;
        u32 pcmbuf_ch_mode              : 1;
        u32 pcmbuf_preload_en           : 1;
        u32 spdbuf_preload_en           : 1;
        u32                             : 27;
    } bitc;
} reg_aud_snt_data_preload_t;

/*!
  the union of register reg_aud_snt_clk_div2
  */
typedef union reg_aud_snt_clk_div2
{
    u32 all;
    struct
    {
        u32 clk_divider_factor          : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_clk_div2_t;

/*!
  the union of register reg_aud_snt_clk_adj2
  */
typedef union reg_aud_snt_clk_adj2
{
    u32 all;
    struct
    {
        u32 sample_rate_offset          : 25;
        u32                             : 7;
    } bitc;
} reg_aud_snt_clk_adj2_t;

/*!
  the union of register reg_aud_snt_samp_num_frm
  */
typedef union reg_aud_snt_samp_num_frm
{
    u32 all;
    struct
    {
        u32 samp_num_perfrm             : 14;
        u32                             : 18;
    } bitc;
} reg_aud_snt_samp_num_frm_t;

/*!
  the union of register reg_aud_snt_ppbufw_intr_timer
  */
typedef union reg_aud_snt_ppbufw_intr_timer
{
    u32 all;
    struct
    {
        u32 ppbuf_w_intr_timer          : 31;
        u32                             : 1;
    } bitc;
} reg_aud_snt_ppbufw_intr_timer_t;

/*!
  the union of register reg_aud_snt_bufrw_intr_timer
  */
typedef union reg_aud_snt_bufrw_intr_timer
{
    u32 all;
    struct
    {
        u32 buf_rw_intr_timer           : 31;
        u32                             : 1;
    } bitc;
} reg_aud_snt_bufrw_intr_timer_t;

/*!
  the union of register reg_aud_snt_pcmfifo_intr_timer
  */
typedef union reg_aud_snt_pcmfifo_intr_timer
{
    u32 all;
    struct
    {
        u32 pcmfifo_empty_intr_timer    : 31;
        u32                             : 1;
    } bitc;
} reg_aud_snt_pcmfifo_intr_timer_t;

/*!
  the union of register reg_aud_snt_intr_set
  */
typedef union reg_aud_snt_intr_set
{
    u32 all;
    struct
    {
        u32 audfrm_intr                 : 1;
        u32 ppbuf_w_intr                : 1;
        u32 buf_rw_intr                 : 1;
        u32 pcmfifo_emp_intr            : 1;
        u32 pcmfifo_diff_intr           : 1;
        u32 audfrm_intr_2               : 1;
        u32                             : 2;
        u32 audfrm_intr_mask            : 1;
        u32 ppbufw_intr_mask            : 1;
        u32 bufrw_intr_mask             : 1;
        u32 pcmfifo_emp_intr_mask       : 1;
        u32 pcmfifo_diff_intr_mask      : 1;
        u32 audfrm_intr_mask_2          : 1;
        u32                             : 18;
    } bitc;
} reg_aud_snt_intr_set_t;

/*!
  the union of register reg_aud_snt_spd_buf_jump_en
  */
typedef union reg_aud_snt_spd_buf_jump_en
{
    u32 all;
    struct
    {
        u32 spd_frm_size                : 16;
        u32                             : 15;
        u32 spd_buf_jump_en             : 1;
    } bitc;
} reg_aud_snt_spd_buf_jump_en_t;

/*!
  the union of register reg_aud_snt_individual_vol_ctl
  */
typedef union reg_aud_snt_individual_vol_ctl
{
    u32 all;
    struct
    {
        u32 hdmi                : 16;        
        u32 adac                 : 16;
    } bitc;
} reg_aud_snt_individual_vol_ctl_t;

/*!
  the union of register reg_aud_snt_aud_pp_buf0_cnt
  */
typedef union reg_aud_snt_aud_pp_buf0_cnt
{
    u32 all;
    struct
    {
        u32 counter                     : 20;
        u32 channel                    : 8;
        u32 fs                             :4;
    } bitc;
} reg_aud_snt_aud_pp_buf0_cnt_t;

/*!
  the union of register reg_aud_snt_aud_buf_full_flag
  */
typedef union reg_aud_snt_aud_buf_full_flag
{
    u32 all;
    struct
    {
        u32 left                        : 1;
        u32 right                       : 1;
        u32 bass                        : 1;
        u32 center                      : 1;
        u32 sl                          : 1;
        u32 sr                          : 1;
        u32 rsl                         : 1;
        u32 rsr                         : 1;
        u32 pcm                         : 1;
        u32 spdif                       : 1;
        u32                             : 22;
    } bitc;
} reg_aud_snt_aud_buf_full_flag_t;

/*!
  the union of register reg_aud_snt_aud_buf_full_flag
  */
typedef union reg_aud_snt_aud_res_reg0
{
    u32 all;
    struct
    {
        u32 chimaera_mode           : 1;
        u32 dbg_hdmi_in_mux          : 1;
        u32 chan_lr_reverse             : 1;
        u32 res                                  : 1;
        u32 chimaera_stat                : 1;    //  4
        u32 dbg_pcm_reset                : 1;    //  5
        u32 dbg_spd_reset                : 1;    //  6
        u32 flg_ddp                           : 1;    //  7  0:dd  1:ddp  
        u32 res2                                : 8;
        u32 dbg_aout                          : 1;// for dolby        
        u32                             : 15;
    } bitc;
} reg_aud_snt_aud_res_reg0_t;



/*!
  the union of register reg_sys_ctrl_reg_128
  */
typedef union reg_sys_ctrl_reg_2c
{
    u32 all;
    struct
    {
        u32            : 28;
        u32 spdif_mux          : 1;      // 1:enable spdif 
        u32              : 3;        
    } bitc;
} reg_sys_ctrl_reg_2c_t;

/*!
  the union of register reg_sys_ctrl_reg_128
  */
typedef union reg_sys_ctrl_reg_128
{
    u32 all;
    struct
    {
        u32            : 3;
        u32 mute_mux          : 1;      // 1:open mute function   0:other fuction
        u32              : 28;        
    } bitc;
} reg_sys_ctrl_reg_128_t;

/*!
  the union of register reg_sys_ctrl_reg_144
  */
typedef union reg_sys_ctrl_reg_144
{
    u32 all;
    struct
    {
        u32 mute          : 1;      // 1:umute   0:mute
        u32              : 31;        
    } bitc;
} reg_sys_ctrl_reg_144_t;



void aria_set_audio_reg_0_channel(ulong u32RegBase, mt_u8 value);


void reg_sym_linux_00_reg_32_bit(mt_u8 data);

void reg_sym_linux_00_reg_sample_rate_bit(mt_u8 data);

void reg_sym_linux_04_reg_justified_bit(mt_u8 data);

void reg_sym_linux_volume_reg_gainq_bit(mt_u8 data);

void reg_sym_linux_volume_reg_scale_bit(mt_u16 data);

void reg_sym_linux_volume_reg_spdif_mute_hdmi_bit(mt_u8 data);
void reg_sym_linux_volume_reg_spdif_mute_coax_bit(mt_u8 data);

void reg_sym_linux_volume_reg(mt_u8 gainq, mt_u16 volume_scale, mt_u8 mute);

void reg_sym_linux_fade_reg(mt_u8 fader_enable, mt_u8 fade_in/*1: fade in, 0: fade out*/, mt_u16 target_gain, mt_u16 fade_timestep);

void reg_sym_linux_pp_reg_src_bit(mt_u8 data);

void reg_sym_linux_pp_reg_src_mod_bit(mt_u8 data);

void reg_sym_linux_pp_reg_chan_mod_bit(mt_u8 data);

void reg_sym_linux_pp_reg_downmix_bit(mt_u8 data);
void reg_sym_linux_pp_reg_downmix_hdmi_bit(mt_u8 data);

void reg_sym_linux_clk1_reg_div_bit(mt_u32 data);
mt_u32 reg_sym_linux_get_clk1_reg_div_bit(void);

void reg_sym_linux_clk1_reg_div_bit2(mt_u32 data);
mt_u32 reg_sym_linux_get_clk1_reg_div_bit2(void);

mt_u32 reg_sym_linux_get_clk1_reg_div_bit2(void);

void reg_sym_linux_downmix_coef_left_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_left_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_left_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_right_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_right_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_right_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_bass_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_bass_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_bass_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_center_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_center_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_center_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_sl_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_sl_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_sl_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_sr_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_sr_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_sr_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_rsl_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_rsl_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_rsl_2_all_bit(mt_u32 data);

void reg_sym_linux_downmix_coef_rsr_2_left_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_rsr_2_right_bit(mt_u16 data);
void reg_sym_linux_downmix_coef_rsr_2_all_bit(mt_u32 data);

void reg_sym_linux_sample_num_reg_sample_num_bit(mt_u32 data);


 void reg_sym_linux_04_reg_spdif_chan_spdif_bit(mt_u8 data);
 void reg_sym_linux_04_reg_spdif_chan_sel_bit(mt_u8 data);
 void reg_sym_linux_04_reg_spdif_chan_sel_hdmi_bit(mt_u8 data);

 void reg_sym_linux_mix_func_cfg_mix_en_bit(mt_u32 data); 
void reg_sym_linux_mix_func_cfg_aud_res_reg0_bit(mt_u32 data);
void reg_sym_linux_mix_func_cfg_aud_res_reg3_bit(mt_u32 data);
void reg_sym_linux_aud_vol_cfg_spdif_mix_spdif_alpha_bit(mt_u32 data);
void reg_sym_linux_aud_vol_cfg_spdif_pcm_vol_ctrl_bit(mt_u32 data);
void reg_sym_linux_mix_func_alpha_mix_adac_bit(mt_u32 data);
void reg_sym_linux_mix_func_alpha_mix_hdmi_bit(mt_u32 data);

mt_void iHAL_AOE_AIP_Reset_PCMBuf(mt_void);
mt_void iHAL_AOE_AIP_Reset_SPDBuf(mt_void);
mt_void iHAL_AOE_AIP_Reset_MIXBuf(mt_void);

mt_void iHAL_AOE_AIP_reset_aout_buffer(mt_void);

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif  // __MT_AIAO_FUNC_H__
