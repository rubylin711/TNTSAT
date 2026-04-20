/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_type.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_module.h"
#include "mt_module_debug.h"

#include "mt_drv_ai.h"
#include "mt_drv_ao.h"
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */



#ifdef MT_AUDIO_AI_SUPPORT
extern mt_s32 AI_DRV_ModInit(mt_void);
extern mt_s32 AI_DRV_ModExit(mt_void);
#endif
#ifdef MT_ALSA_AO_SUPPORT
extern int AO_ALSA_ModInit(void);
extern void  AO_ALSA_ModExit(void);
#endif
#ifdef MT_ALSA_HDMI_ONLY_SUPPORT
extern int  HDMI_ALSA_ModInit(void);
extern void HDMI_ALSA_ModExit(void);
#endif
#ifdef MT_ALSA_I2S_ONLY_SUPPORT
extern int  I2S_ALSA_ModInit(void);
extern void I2S_ALSA_ModExit(void);
#endif
extern mt_s32 AO_DRV_ModInit(mt_void);
extern mt_s32 AO_DRV_ModExit(mt_void);


/*****************************************************************************
 Prototype    : AIAO_DRV_ModInit
 Description  : initialize function in AIAO module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
mt_s32 __init AIAO_DRV_ModInit(mt_void)
{
    mt_s32 s32Ret;

#ifdef MT_AUDIO_AI_SUPPORT
    //init AI module
    s32Ret = AI_DRV_ModInit();
    if(MT_SUCCESS != s32Ret)
    {
        //to do
        MT_FATAL_AO("AI_ModInit Fail \n");
        goto err_ai;

    }
#endif

    //init AO module
    s32Ret = AO_DRV_ModInit();
    if(MT_SUCCESS != s32Ret)
    {
        //to do
        MT_FATAL_AO("AO_ModInit Fail \n");
        return s32Ret;
    }

#ifdef MT_ALSA_I2S_ONLY_SUPPORT
    s32Ret = I2S_ALSA_ModInit();
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_AIAO("Init alsa drv fail!\n");
        goto err__i2s_alsa;
    }
#endif
#ifdef MT_ALSA_HDMI_ONLY_SUPPORT
    s32Ret = HDMI_ALSA_ModInit();
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_AIAO("Init HDMI alsa drv fail!\n");
        goto err__hdmi_alsa;
    }
#endif

#ifdef MT_ALSA_AO_SUPPORT
    s32Ret = AO_ALSA_ModInit();
    if (MT_SUCCESS != s32Ret)
    {
        MT_FATAL_AO("Init alsa drv fail!\n");
        goto err_alsa;
    }
#endif

    return MT_SUCCESS;

#ifdef MT_ALSA_AO_SUPPORT
err_alsa:
    AO_ALSA_ModExit();
    AO_DRV_ModExit();
    return MT_FAILURE;
#endif
#ifdef MT_ALSA_I2S_ONLY_SUPPORT
err__i2s_alsa:
    I2S_ALSA_ModExit();
    return MT_FAILURE;
#endif
#ifdef MT_ALSA_HDMI_ONLY_SUPPORT
err__hdmi_alsa:
    HDMI_ALSA_ModExit();
    return MT_FAILURE;
#endif

#ifdef MT_AUDIO_AI_SUPPORT
err_ai:
    AO_DRV_ModExit();
    return MT_FAILURE;
#endif
}


/*****************************************************************************
 Prototype    : AIAO_DRV_ModExit
 Description  : exit function in AIAO module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
mt_void __exit AIAO_DRV_ModExit(mt_void)
{

#ifdef MT_ALSA_AO_SUPPORT
    AO_ALSA_ModExit();
#endif
#ifdef MT_ALSA_HDMI_ONLY_SUPPORT
    HDMI_ALSA_ModExit();
#endif
#ifdef MT_ALSA_I2S_ONLY_SUPPORT
    I2S_ALSA_ModExit();
#endif

#ifdef MT_AUDIO_AI_SUPPORT
    //deinit AI module
    AI_DRV_ModExit();
#endif

    //deinit AO module
    AO_DRV_ModExit();

    MT_INFO_AO(" **** AIAO_DRV_ModExit OK  **** \n");
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
