/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_DRV_PDM_H__
#define __MT_DRV_PDM_H__

#include "mt_type.h"
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_go_surface.h"
#include "mt_unf_mce.h"
#include "mt_unf_pdm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MCE_DEF_BASEPARAM_SIZE       (8*1024)
#define MCE_DEF_LOGOPARAM_SIZE		 (8*1024)
#define MCE_DEF_PLAYPARAM_SIZE		 (8*1024)


#define PARSER_BASE_FLASH_NAME         "baseparam"
#define PARSER_LOGO_FLASH_NAME         "logo"
#define PARSER_FASTPLAY_FLASH_NAME     "fastplay"

#define MCE_BASE_TABLENAME_DISP0          "BASE_TABLE_DISP0"
#define MCE_BASE_TABLENAME_DISP1          "BASE_TABLE_DISP1"

#define MCE_BASE_KEYNAME_FMT            "BASE_KEY_FMT"
#define MCE_BASE_KEYNAME_INTF_HDMI      "BASE_KEY_HDMI"
#define MCE_BASE_KEYNAME_INTF_YPBPR     "BASE_KEY_YPBPR"
#define MCE_BASE_KEYNAME_INTF_CVBS      "BASE_KEY_CVBS"
#define MCE_BASE_KEYNAME_INTF_RGB	    "BASE_KEY_RGB"
#define MCE_BASE_KEYNAME_INTF_SVIDEO    "BASE_KEY_SVIDEO"
#define MCE_BASE_KEYNAME_INTF_BT656     "BASE_KEY_BT656"
#define MCE_BASE_KEYNAME_INTF_BT1120    "BASE_KEY_BT1120"
#define MCE_BASE_KEYNAME_INTF_LCD       "BASE_KEY_LCD"
#define MCE_BASE_KEYNAME_INTF_VGA       "BASE_KEY_VGA"
#define MCE_BASE_KEYNAME_PF             "BASE_KEY_PF"
#define MCE_BASE_KEYNAME_DISPW          "BASE_KEY_DISPW"
#define MCE_BASE_KEYNAME_DISPH          "BASE_KEY_DISPH"
#define MCE_BASE_KEYNAME_SCRX           "BASE_KEY_SCRX"
#define MCE_BASE_KEYNAME_SCRY           "BASE_KEY_SCRY"
#define MCE_BASE_KEYNAME_SCRW           "BASE_KEY_SCRW"
#define MCE_BASE_KEYNAME_SCRH           "BASE_KEY_SCRH"
#define MCE_BASE_KEYNAME_HULEP          "BASE_KEY_HULEP"
#define MCE_BASE_KEYNAME_SATU           "BASE_KEY_SATU"
#define MCE_BASE_KEYNAME_CONTR          "BASE_KEY_CONTR"
#define MCE_BASE_KEYNAME_BRIG           "BASE_KEY_BRIG"
#define MCE_BASE_KEYNAME_BGCOLOR        "BASE_KEY_BGCOLOR"
#define MCE_BASE_KEYNAME_MACRSN         "BASE_KEY_MACRSN"
#define MCE_BASE_KEYNAME_TIMING         "BASE_KEY_TIMING"
#define MCE_BASE_KEYNAME_GAMA           "BASE_KEY_GAMA"
#define MCE_BASE_KEYNAME_ASPECT         "BASE_KEY_ASPECT"

#define MCE_BASE_KEYNAME_SRC_DISP       "BASE_KEY_SOURCE_DISP"
#define MCE_BASE_KEYNAME_VIRSCW         "BASE_KEY_VIRSCW"
#define MCE_BASE_KEYNAME_VIRSCH         "BASE_KEY_VIRSCH"
#define MCE_BASE_KEYNAME_DISP_L         "BASE_KEY_DISP_L"
#define MCE_BASE_KEYNAME_DISP_T         "BASE_KEY_DISP_T"
#define MCE_BASE_KEYNAME_DISP_R         "BASE_KEY_DISP_R"
#define MCE_BASE_KEYNAME_DISP_B         "BASE_KEY_DISP_B"
#define MCE_LOGO_TABLENAME              "LOGO_TABLE"
#define MCE_LOGO_KEYNAME_FLAG           "LOGO_KEY_FLAG"
#define MCE_LOGO_KEYNAME_CONTLEN        "LOGO_KEY_LEN"

#define MCE_PLAY_TABLENAME              "PLAY_TABLE"
#define MCE_PLAY_KEYNAME_FLAG           "PLAY_KEY_FLAG"
#define MCE_PLAY_KEYNAME_DATALEN        "PLAY_KEY_DATALEN"
#define MCE_PLAY_KEYNAME_PARAM          "PLAY_KEY_PARAM"

#define MCE_BASE_TABLENAME_SOUND0       "BASE_TABLE_SOUND0"
#define MCE_BASE_TABLENAME_SOUND1       "BASE_TABLE_SOUND1"
#define MCE_BASE_TABLENAME_SOUND2       "BASE_TABLE_SOUND2"

#define MCE_BASE_KEYNAME_SND_DAC0        "BASE_KEY_SND_DAC0"
#define MCE_BASE_KEYNAME_SND_I2S0        "BASE_KEY_SND_I2S0"
#define MCE_BASE_KEYNAME_SND_I2S1        "BASE_KEY_SND_I2S1"
#define MCE_BASE_KEYNAME_SND_SPDIF0      "BASE_KEY_SND_SPDIF0"
#define MCE_BASE_KEYNAME_SND_HDMI0       "BASE_KEY_SND_HDMI0"
#define MCE_BASE_KEYNAME_SND_ARC0        "BASE_KEY_SND_ARC0"

#define MCE_BASE_KEYNAME_SND_DAC0_VOLUME        "BASE_KEY_SND_DAC_VOLUME0"
#define MCE_BASE_KEYNAME_SND_I2S0_VOLUME       "BASE_KEY_SND_I2S_VOLUME0"
#define MCE_BASE_KEYNAME_SND_I2S1_VOLUME       "BASE_KEY_SND_I2S_VOLUME1"
#define MCE_BASE_KEYNAME_SND_SPDIF0_VOLUME      "BASE_KEY_SND_SPDIF_VOLUME0"
#define MCE_BASE_KEYNAME_SND_HDMI0_VOLUME       "BASE_KEY_SND_HDMI_VOLUME0"
#define MCE_BASE_KEYNAME_SND_ARC0_VOLUME        "BASE_KEY_SND_ARC_VOLUME0"

/*timing key*/
#define MCE_BASE_KEYNAME_TIMING_VFB         "BASE_KEY_TIMING_VFB"   //VFB
#define MCE_BASE_KEYNAME_TIMING_VBB         "BASE_KEY_TIMING_VBB"   //VBB
#define MCE_BASE_KEYNAME_TIMING_VACT         "BASE_KEY_TIMING_VACT" //VACT

#define MCE_BASE_KEYNAME_TIMING_HFB         "BASE_KEY_TIMING_HFB"	//HFB
#define MCE_BASE_KEYNAME_TIMING_HBB         "BASE_KEY_TIMING_HBB"	//HBB
#define MCE_BASE_KEYNAME_TIMING_HACT         "BASE_KEY_TIMING_HACT"	//HACT

#define MCE_BASE_KEYNAME_TIMING_VPW         "BASE_KEY_TIMING_VPW"   //VPW
#define MCE_BASE_KEYNAME_TIMING_HPW         "BASE_KEY_TIMING_HPW"   //HPW
#define MCE_BASE_KEYNAME_TIMING_IDV         "BASE_KEY_TIMING_IDV"   //IDV
#define MCE_BASE_KEYNAME_TIMING_IHS         "BASE_KEY_TIMING_IHS"   //IHS
#define MCE_BASE_KEYNAME_TIMING_IVS         "BASE_KEY_TIMING_IVS"   //IVS


#define MCE_BASE_KEYNAME_TIMING_CLKR         "BASE_KEY_TIMING_CLKR" //ClockReversal
#define MCE_BASE_KEYNAME_TIMING_DW         "BASE_KEY_TIMING_DW"     //DataWidth
#define MCE_BASE_KEYNAME_TIMING_ITFMT         "BASE_KEY_TIMING_ITFMT" //ItfFormat

#define MCE_BASE_KEYNAME_TIMING_DITE         "BASE_KEY_TIMING_DITE" //DitherEnable
#define MCE_BASE_KEYNAME_TIMING_CLK0         "BASE_KEY_TIMING_CLK0" //ClkPara0
#define MCE_BASE_KEYNAME_TIMING_CLK1         "BASE_KEY_TIMING_CLK1" //ClkPara1

#define MCE_BASE_KEYNAME_TIMING_INTERL         "BASE_KEY_TIMING_INTERL" //bInterlace
#define MCE_BASE_KEYNAME_TIMING_FIXF         "BASE_KEY_TIMING_FIXF"     //PixFreq
#define MCE_BASE_KEYNAME_TIMING_VERTF         "BASE_KEY_TIMING_VERTF"   //VertFreq
#define MCE_BASE_KEYNAME_TIMING_AW         "BASE_KEY_TIMING_AW"         //AspectRatioW
#define MCE_BASE_KEYNAME_TIMING_AH         "BASE_KEY_TIMING_AH"         //AspectRatioH
#define MCE_BASE_KEYNAME_TIMING_USEGAMMA         "BASE_KEY_TIMING_USEGAMMA" //bUseGamma
#define MCE_BASE_KEYNAME_TIMING_RSV0         "BASE_KEY_TIMING_RSV0"     //Reserve0
#define MCE_BASE_KEYNAME_TIMING_RSV1         "BASE_KEY_TIMING_RSV1"     //Reserve1

#define MCE_BASE_TABLENAME_PMOC             "BASE_TABLE_PMOC"
#define MCE_BASE_KEYNAME_PMOC_FLAG          "BASE_KEY_PMOC_FLAG"    

#define MCE_BASE_TABLENAME_HDMI             "BASE_TABLE_HDMI"
#define MCE_BASE_KEYNAME_EDID_LENTH         "BASE_KEY_EDID_LENTH" 
#define MCE_BASE_KEYNAME_EDID_CONTENT       "BASE_KEY_EDID_CONTENT" 

#define MT_FATAL_PDM(format...)    MT_FATAL_PRINT(MT_ID_PDM, format)
#define MT_ERR_PDM(format...)      MT_ERR_PRINT(MT_ID_PDM, format)
#define MT_WARN_PDM(format...)     MT_WARN_PRINT(MT_ID_PDM, format)
#define MT_INFO_PDM(format...)     MT_INFO_PRINT(MT_ID_PDM, format)

#define PDM_BASEPARAM_BUFNAME "baseparam"
#define PDM_PLAYPARAM_BUFNAME "playparam"
#define PDM_PLAYDATA_BUFNAME  "playdata"


typedef struct mtDISP_PARAM_S
{
    MT_UNF_DISP_E				enSrcDisp;
    MT_UNF_ENC_FMT_E            enFormat;   
    mt_u32                      u32Brightness;
    mt_u32                      u32Contrast;
    mt_u32                      u32Saturation;
    mt_u32                      u32HuePlus;
    MT_BOOL                     bGammaEnable; 
    mt_u32                      u32VirtScreenWidth;
    mt_u32                      u32VirtScreenHeight; 
    MT_UNF_DISP_OFFSET_S        stOffsetInfo;
    MT_UNF_DISP_BG_COLOR_S      stBgColor;
    MT_UNF_DISP_ASPECT_RATIO_S  stAspectRatio;
    MT_UNF_DISP_INTF_S          stIntf[MT_UNF_DISP_INTF_TYPE_BUTT];
    MT_UNF_DISP_TIMING_S        stDispTiming;
	MTGO_PF_E                   enPixelFormat;
}MT_DISP_PARAM_S;

typedef struct mtLOGO_PARAM_S
{
    mt_u32                      u32CheckFlag; 
    mt_u32                      u32LogoLen;
}MT_LOGO_PARAM_S;

typedef struct mtMCE_PARAM_S
{
    mt_u32                      u32CheckFlag;
    mt_u32                      u32PlayDataLen;
    MT_UNF_MCE_PLAY_PARAM_S     stPlayParam;
}MT_MCE_PARAM_S;

#define MT_FLASH_NAME_LEN   32

/*Flash pation info for mce use*/
typedef struct mtPDM_FLASH_INFO_S
{
    mt_char             Name[MT_FLASH_NAME_LEN];
    mt_u32              u32Offset;
    mt_u32              u32Size;
    MT_BOOL             bShared;  /*whether share one partition with other data*/
}PDM_FLASH_INFO_S;

mt_s32 MT_DRV_PDM_Init(mt_void);
mt_s32 MT_DRV_PDM_DeInit(mt_void);
mt_s32 MT_DRV_PDM_GetDispParam(MT_UNF_DISP_E enDisp, MT_DISP_PARAM_S *pstDispParam);
mt_s32 MT_DRV_PDM_GetSoundParam(MT_UNF_SND_E enSound, MT_UNF_PDM_SOUND_PARAM_S * pstSoundParam);

/*Ex interface for compatibility, there is no volume in old baseparam image, 
pbVolumeExistInBase can distinguish a image is an old baseparam or a new baseparam;
and there are volume in both old and new fastplay image, fastplay will select using
volume in baseparam first, if baseparam is old image, fastplay will using volume in fastplay image*/
mt_s32 MT_DRV_PDM_GetSoundParamEx(MT_UNF_SND_E enSound, MT_UNF_PDM_SOUND_PARAM_S * pstSoundParam, MT_BOOL *pbVolumeExistInBase);
mt_s32 MT_DRV_PDM_GetMceParam(MT_MCE_PARAM_S *pMceParam);
mt_s32 MT_DRV_PDM_GetMceData(mt_u32 u32Size, mt_u32 *pAddr);
mt_s32 MT_DRV_PDM_ReleaseReserveMem(const mt_char *BufName);
mt_s32 MT_DRV_PDM_GetData(const mt_char *BufName, mt_u32 *pu32DataAddr, mt_u32 *pu32DataLen);

/************** reserve mem ***************/
//void pdm_reserve_mem(void);


#ifdef __cplusplus
}
#endif
#endif


