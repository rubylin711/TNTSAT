

#ifdef __cplusplus
extern "C" {
#endif

#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_video.h"



typedef struct mt_disp_fmt
{
    MT_BOOL enable;
    MT_UNF_ENC_FMT_E format;
} disp_fmt_config_t;


typedef struct mt_ratio
{
    MT_BOOL enable;
    MT_UNF_DISP_ASPECT_RATIO_E enDispAspectRatio;
    MT_UNF_VO_ASPECT_CVRS_E   enAspectCvrs;
} ratio_config_t;


typedef struct mt_config
{
    MT_BOOL is_use;
    disp_fmt_config_t disp_fmt;
    ratio_config_t ratio;
    MT_U32 hdcp;
    MT_BOOL heaac;
    MT_UNF_HDMI_DEEP_COLOR_E enDeepColor;
}config_t;


mt_s32 MTADP_Read_All_Config(config_t *config);

mt_s32 MTADP_Write_All_Config(config_t *config);

mt_s32 MTADP_Config_Set_Default(config_t *config);

mt_s32 MTADP_Find_Device(mt_char *mtdedv) ;


