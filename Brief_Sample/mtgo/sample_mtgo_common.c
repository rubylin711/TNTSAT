#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "mt_go.h"
#include "mt_go_decoder.h"

#include "mt_unf_disp.h"
#include "mt_common.h"
#include "mt_adp_mpi.h"
#include "mt_adp_hdmi.h"

#include "sample_mtgo_common.h"


/***************************** Macro Definition ******************************/


/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/


mt_s32 Sample_MTGO_Display_Init(mt_u32 tv_sys)
{
    mt_s32 ret = MT_FAILURE;

    SAMPLE_MTGO_FUNCTION_ENTER();

    ret = mt_sys_init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("mt_sys_init error. ret=0x%x \n", ret);
        return ret;
    }

    SAMPLE_MTGO_INFO_PRINT("<%s>line [%d]: Sample_MTGO_Display_Init: %d\n", __FUNCTION__, __LINE__, tv_sys);
    ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, tv_sys);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MTADP_HDMI_Init error. ret=0x%x \n", ret);
        goto ERR1;
    }

    ret = MTADP_Disp_Init((MT_UNF_ENC_FMT_E)tv_sys);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MTADP_Disp_Init error. ret=0x%x \n", ret);
        goto ERR2;
    }

    SAMPLE_MTGO_FUNCTION_EXIT();
    return MT_SUCCESS;

ERR2:
    (MT_VOID)MTADP_Disp_DeInit();
ERR1:
    (MT_VOID)mt_sys_deinit();

    return MT_FAILURE;
}

mt_s32 Sample_MTGO_Display_DeInit()
{
    SAMPLE_MTGO_FUNCTION_ENTER();

    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);


    (MT_VOID)MTADP_Disp_DeInit();

    (MT_VOID)mt_sys_deinit();

    SAMPLE_MTGO_FUNCTION_EXIT();

    return  MT_SUCCESS;
}


