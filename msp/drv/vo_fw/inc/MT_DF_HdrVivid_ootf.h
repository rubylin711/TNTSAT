#ifndef HDR_VIVID_H
#define HDR_VIVID_H

#include "mt_drv_video.h"

#define VIVID_PIX_W 10


#define VIVID_PIX_MIN 0
#define VIVID_PIX_MAX ((1<<DB_PIX_W)-1)

#ifndef __PIX_D__
#define __PIX_D__
#if (VIVID_PIX_W == 8)
typedef unsigned char pix;
#else
typedef unsigned short pix;
#endif
#endif

typedef unsigned short pix_out_hdr10;

#define SATURATION_PRECISION 10


//#ifdef _WIN32
//typedef MT_S64  MT_S64;
//#endif


#define true 1
#define false 0


#define NUM_PRECESS                               1
#define MAX_DISPLAY_LUMINANCE                     1000 // 500
#define YUV420_ENABLED                             1
#define SDR_ENABLED                                0
#if SDR_ENABLED
#define MAX_DISPLAY_LUMINANCE_SDR                 100
#endif
#define TONE_MAPPING_SPEC_WIDTH                  3840
#define TONE_MAPPING_SPEC_HEIGHT                 2160
#define HLGtoSDRStatic                              0
#define HLG_STATIC_BT2020toBT709                    0    //



#define TPA_NUM      4

#define FIXED_SEL  1

////////////////////////////////////////////////////////////

//******************************************************************************
// CUVA HDR Definitions
//******************************************************************************
#define system_start_code_BIT        8
#define minimum_maxrgb_BIT           12
#define average_maxrgb_BIT           12
#define variance_maxrgb_BIT          12
#define maximum_maxrgb_BIT           12
#define tone_mapping_mode_BIT        1
#define tone_mapping_param_num_BIT   1
#define targeted_system_display_BIT  12
#define Base_flag_BIT                1
#define Base_param_m_p_BIT           14
#define Base_param_m_m_BIT           6
#define Base_param_m_a_BIT           10
#define Base_param_m_b_BIT           10
#define Base_param_m_n_BIT           6
#define Base_param_K1_BIT            2
#define Base_param_K2_BIT            2
#define Base_param_K3_BIT            4
#define Base_param_Delta_mode_BIT    3
#define Base_param_Delta_BIT         7
#define P3Spline_flag_BIT            1
#define P3Spline_num_BIT             1
#define P3Spline_TH_mode_BIT         2
#define P3Spline_TH_MB_BIT           8
#define P3Spline_TH_OFFSET_BIT       2
#define P3Spline_TH1_BIT             12
#define P3Spline_TH2_BIT             10
#define P3Spline_TH3_BIT             10
#define P3Spline_Strength_BIT        8
#define color_saturation_BIT         1
#define color_saturation_num_BIT     3
#define color_saturation_gain_BIT    8
#define CUVAHDR_METADATA_BASE_S      13
#define CUVAHDR_METADATA_BASE_B      10

typedef struct CuvaCurve_int
{
    MT_S64  m_p;
    MT_S64  m_m;
    MT_S64  m_a;
    MT_S64  m_b;
    MT_S64  m_n;
    MT_S64  K1;
    MT_S64  K2;
    MT_S64  K3;
    MT_S64  curve_mintiao;
    MT_S64  TH1, TH2, TH3;
    MT_S64  md1, mc1, mb1, ma1;
    MT_S64  md2, mc2, mb2, ma2;
    MT_S64  DARKcurble_S1;
    MT_S64  DARKcurble_offset;
    MT_S32  curve_mintiao_high_area;
    MT_S64  TH1_HIGH, TH2_HIGH, TH3_HIGH;
    MT_S64  md1_high, mc1_high, mb1_high, ma1_high;
    MT_S64  md2_high, mc2_high, mb2_high, ma2_high;
    MT_S64  high_area_flag;
    MT_S64  curve_adjust;
    MT_S64  m_p_T;
    MT_S64  m_a_T;

    MT_S64  m_inputMaxE_m;

    MT_S32  m_m_10 ;
    MT_S32  m_n_10 ;

    MT_S32 norm_value;

}CuvaCurve_int ;

typedef struct Cuva_tone_mapping_para_int
{

    MT_S64  m_p;
    MT_S64  m_m;
    MT_S64  m_a;
    MT_S64  m_b;
    MT_S64  m_n;
    MT_S64  K1;
    MT_S64  K2;
    MT_S64  K3;
    MT_S64  DARKcurble_S1;
    MT_S64  DARKcurble_offset;
    MT_U32 base_param_Delta_mode;

    MT_S64  m_maxEtemp_store;
    MT_S64  m_inputMaxEtemp_store;
    MT_S64  m_maxE;
    MT_S64  m_inputMaxE;
    MT_S64  m_minE;
    MT_S64  m_inputMinE;

    MT_S64  maximum_maxrgb_noLine;
    MT_S64  minimum_maxrgb_noLine;
    MT_S64  average_maxrgb_noLine;
    MT_S64  variance_maxrgb_noLine;

    MT_S64  TH1, TH2, TH3;
    MT_S64  TH1_HIGH, TH2_HIGH, TH3_HIGH;
    MT_S64  md1, mc1, mb1, ma1;
    MT_S64  md1_high, mc1_high, mb1_high, ma1_high;
    MT_S64  md2, mc2, mb2, ma2;
    MT_S64  md2_high, mc2_high, mb2_high, ma2_high;
    MT_BOOL curve_mintiao;
    MT_BOOL curve_mintiao_high_area;
    MT_S32	high_area_flag;

    MT_S64  TML;
    MT_S64  TML_linear;
    MT_S64  RML;
    MT_S64  RML_linear;

    MT_S64  m_inputMaxE_m;
    MT_S32 norm_value;

} Cuva_tone_mapping_para_int ;

void hdrvivid_ootf_process(CuvaMetadata* metadata,MT_S32 dest_max_luminace_disp, MT_U8 hdr_vivid_mode_out, MT_U16* ootf_table, MT_U32* hdr10p_eotf_normal_value, MT_U8 bHLGInput);

#endif

