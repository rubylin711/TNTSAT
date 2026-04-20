/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/*********************************add include here******************************/

#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_unf_flash.h"
#include "mt_adp_mpi.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_config.h"






/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_CONFIG_DEBUG

#define MT_CONFIG_PRINT   printf
#else

#define MT_CONFIG_PRINT

#endif

#define SAMPLE_CONFIG_FUNCTION_ENTER()      MT_CONFIG_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_CONFIG_FUNCTION_EXIT()       MT_CONFIG_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_CONFIG_FATAL_PRINT(fmt...)       MT_CONFIG_PRINT(" [FATAL] " fmt)
#define SAMPLE_CONFIG_ERR_PRINT(fmt...)        MT_CONFIG_PRINT(" [ERROR] " fmt)
#define SAMPLE_CONFIG_WARN_PRINT(fmt...)       MT_CONFIG_PRINT(" [WARN] "  fmt)
#define SAMPLE_CONFIG_INFO_PRINT(fmt...)       MT_CONFIG_PRINT(" [INFO] "  fmt)
#define SAMPLE_CONFIG_DBG_PRINT(fmt...)        MT_CONFIG_PRINT(" [DEBUG] " fmt)

#define SAMPLE_CONFIG_PRINT   printf


#ifdef MT_SAMPLE_APP
extern MT_AVPLAY_INFO avplayHandle;
#endif


#ifdef MT_SAMPLE_APP
MT_S32 MT_ConfigMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


#ifdef MT_SAMPLE_APP
MT_S32 MT_ConfigMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32     ret = MT_SUCCESS;
    config_t config = { 0 };
    mt_handle hwin = 0;
    MT_UNF_WINDOW_ATTR_S getWindowAttr = { 0 };
    MT_UNF_DISP_ASPECT_RATIO_S getDispAspectRatio = { 0 };
    MT_BOOL heaac = {0};
    MT_UNF_HDMI_DEEP_COLOR_E enDeepColor = MT_UNF_HDMI_DEEP_COLOR_24BIT;

#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_sys_init failed, ret = %x\n", ret);
        return ret;
    }
#endif

    config.is_use = MT_TRUE;

    ret = MT_UNF_DISP_GetFormat(MT_UNF_DISPLAY1, &config.disp_fmt.format);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("MT_UNF_DISP_GetFormat failed. ret = 0x%x\n", ret);
    }
    SAMPLE_CONFIG_INFO_PRINT("current format is %d \n", config.disp_fmt.format);
    config.disp_fmt.enable = MT_TRUE;
#ifdef MT_SAMPLE_APP
	hwin = avplayHandle.hWin;
#endif
    ret = MT_UNF_VO_GetWindowAttr(hwin, &getWindowAttr);
    if(MT_SUCCESS == ret)
    {
        MT_UNF_DISP_GetAspectRatio(MT_UNF_DISPLAY1, &getDispAspectRatio);
        config.ratio.enDispAspectRatio = getDispAspectRatio.enDispAspectRatio;
        config.ratio.enAspectCvrs = getWindowAttr.stWinAspectAttr.enAspectCvrs;
        config.ratio.enable = MT_TRUE;
    }
    else
    {
        config.ratio.enable = MT_FALSE;
    }

    config.hdcp = MTADP_HDMI_Get_HdcpEnable();
    
    ret = MT_UNF_HDMI_GetDeepColor(MT_UNF_HDMI_ID_0, &enDeepColor);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CONFIG_INFO_PRINT("MT_UNF_HDMI_GetDeepColor failed!  enDeepColor:%d \n", enDeepColor);
    }
    config.enDeepColor = enDeepColor;
    
    MTADP_Get_Heaac_Enable(&heaac);
    config.heaac = heaac;

    ret = MTADP_Write_All_Config(&config);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("MTADP_Flash_Write failed.ret = %#x\n",ret);
    }
    SAMPLE_CONFIG_INFO_PRINT("Write  succes <Exit>!\n");

#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return MT_SUCCESS;
}
