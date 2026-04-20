/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "mt_type.h"
#include "demo.h"
#include "mt_unf_pm.h"
#include "mt_unf_demux.h"
#include "mt_common.h"
#include "mt_adp_mpi.h"


#define MT_SAMPLE_DEMO_FAC_DEBUG

#ifdef  MT_SAMPLE_DEMO_FAC_DEBUG 
#define MT_DEMO_FAC_PRINT   printf
#else
#define MT_DEMO_FAC_PRINT 
#endif

#define SAMPLE_DEMO_FAC_FUNCTION_ENTER()	        MT_DEMO_FAC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DEMO_FAC_FUNCTION_EXIT()		        MT_DEMO_FAC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_DEMO_FAC_FATAL_PRINT(fmt...) 		MT_DEMO_FAC_PRINT(" [FATAL] " fmt)
#define SAMPLE_DEMO_FAC_ERR_PRINT(fmt...)		    MT_DEMO_FAC_PRINT(" [ERROR] " fmt)
#define SAMPLE_DEMO_FAC_WARN_PRINT(fmt...)		    MT_DEMO_FAC_PRINT(" [WARN] "  fmt)
#define SAMPLE_DEMO_FAC_INFO_PRINT(fmt...)		    MT_DEMO_FAC_PRINT(" [INFO] "  fmt)
#define SAMPLE_DEMO_FAC_DBG_PRINT(fmt...)			MT_DEMO_FAC_PRINT(" [DEBUG] " fmt)


void main(void)
{

	mt_s32                 s32Ret = MT_SUCCESS;
	MT_CHAR            inPutCmd[32] ={ 0 };
	printf("%s(line: %d), ota demo start.\n", __FUNCTION__, __LINE__);
	
	/** System initialization */
    s32Ret = mt_sys_init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DEMO_FAC_ERR_PRINT("failed to mt_sys_init\n");
        return s32Ret;
    }

	/** HDMI initialization */
    s32Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_1080i_60);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DEMO_FAC_ERR_PRINT("failed to StartDmx\n");
        goto ERR0;
    }

    /** Display initialization */
    s32Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_1080i_60);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DEMO_FAC_ERR_PRINT("failed to MTADP_Disp_Init\n");
        goto ERR1;
    }


	demo_usb_init();
	demo_tuner_start();	
	SAMPLE_DEMO_FAC_INFO_PRINT("demo_tuner_start end..\n");
	demo_ir_init();
	demo_frontend_init();
	demo_net_init();
	demo_ui_init();
	SAMPLE_DEMO_FAC_INFO_PRINT("demo_ui_init end..\n");
	demo_playback_init(MT_UNF_DMX_PORT_TSI_1);
	demo_av_play();
	SAMPLE_DEMO_FAC_INFO_PRINT("demo_av_play end..\n");

	while(inPutCmd[0] != 'q')
    {
 //       MT_DVBC_PRINT("Please enter 1-%d to select the program\n", g_pProgTbl->prog_num);
        MT_DEMO_FAC_PRINT("input the 'q' to quit............!\n"
                      	  "CMD>> ");

        fgets((char *)(inPutCmd), (sizeof(inPutCmd) - 1), stdin);

        if(inPutCmd[0] == 'q')
        {
            break;
        }
	}
	printf("%s(line: %d), start stop......\n", __FUNCTION__, __LINE__);
	demo_tuner_stop();
	demo_ir_stop();
	demo_frontend_stop();
	demo_usb_stop();
	printf("%s(line: %d), ota test end.\n", __FUNCTION__, __LINE__);
//ERR2:
    /** Display deinitialization */
    s32Ret = MTADP_Disp_DeInit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DEMO_FAC_ERR_PRINT("MTADP_Disp_DeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR1:
    /** HDMI deinitialization */
    s32Ret = MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DEMO_FAC_ERR_PRINT("MTADP_HDMI_DeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
ERR0:
    /** system deinitialized */
    s32Ret = mt_sys_deinit();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_DEMO_FAC_ERR_PRINT("DVB_DmxDeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }
    

}
