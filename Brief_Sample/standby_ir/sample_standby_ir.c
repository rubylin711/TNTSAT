#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mt_unf_ir.h>
#include <mt_unf_keyled.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mt_unf_pm.h"
#include "mt_unf_ir.h"
#include "mt_unf_hdmi.h"
#include "mt_unf_disp.h"
#include "mt_unf_hdmi.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"
#include <pthread.h>


#ifdef MT_SAMPLE_STANDBY_IR_DEBUG

#define MT_STANDBY_IR_PRINT   printf
#else

#define MT_STANDBY_IR_PRINT

#endif

#define SAMPLE_STANDBY_IR_FUNCTION_ENTER()      MT_STANDBY_IR_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_STANDBY_IR_FUNCTION_EXIT()       MT_STANDBY_IR_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_STANDBY_IR_FATAL_PRINT(fmt...)       MT_STANDBY_IR_PRINT(" [FATAL] " fmt)
#define SAMPLE_STANDBY_IR_ERR_PRINT(fmt...)         MT_STANDBY_IR_PRINT(" [ERROR] " fmt)
#define SAMPLE_STANDBY_IR_WARN_PRINT(fmt...)        MT_STANDBY_IR_PRINT(" [WARN] "  fmt)
#define SAMPLE_STANDBY_IR_INFO_PRINT(fmt...)        MT_STANDBY_IR_PRINT(" [INFO] "  fmt)
#define SAMPLE_STANDBY_IR_DBG_PRINT(fmt...)         MT_STANDBY_IR_PRINT(" [DEBUG] " fmt)

#define SAMPLE_STANDBY_IR_PRINT printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define KEY_STANDBY_IR_OLD 0xf50a7f80

#define KEY_STANDBY_IR_NEW 0xb748fd01
#elif defined CONFIG_MT_CHIP_SYMPHONY6
#define KEY_STANDBY_IR_OLD 0x800a

#define KEY_STANDBY_IR_NEW 0x1fd48
#endif

typedef struct
{
    pthread_t  key;
} MT_Standby_IR_RUN_INFO;

static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_Standby_IR_RUN_INFO    g_stStandby_irRunInfo;

#ifdef MT_SAMPLE_APP
MT_S32 MT_Standby_IrMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif


/*!
@brief Enter false standby
@param  void
@return MT_SUCCESS
@return MT_FAILURE
@*/
static MT_S32 MT_Standby_ir_Standby(void)
{
    MT_S32 Ret = MT_FAILURE;

    Ret = MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_FALSE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_FALSE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_FALSE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_FALSE);
    Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_FALSE);
    Ret |= MT_UNF_HDMI_Stop(MT_UNF_HDMI_ID_0);
    Ret |=MT_UNF_HDMI_Close(MT_UNF_HDMI_ID_0);

    if (MT_SUCCESS != Ret)
    {
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*
@brief False standby wake-up
@param  void
@return MT_SUCCESS
@return MT_FAILURE
*/
static MT_S32 MT_Standby_ir_Wakeup(void)
{
    MT_S32                   Ret = MT_FAILURE;
    MT_UNF_HDMI_OPEN_PARA_S  DefaultMode = { 0 };

    Ret = MT_UNF_HDMI_Open(MT_UNF_HDMI_ID_0, &DefaultMode);
    Ret |= MT_UNF_HDMI_Start(MT_UNF_HDMI_ID_0);
    Ret |= MT_UNF_HDMI_Output_Set(MT_UNF_HDMI_ID_0, MT_TRUE);
    Ret |= MT_UNF_DISP_SetSdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetHdVideoEnable(MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_3, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_2, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_1, MT_TRUE);
    Ret |= MT_UNF_DISP_SetVdacOnOff(MT_UNF_VDAC_0, MT_TRUE);
    if (Ret != MT_SUCCESS)
    {
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/*
@brief Obtain the key status and information of the remote control, and enter standby and wake up according to the key content
@return MT_SUCCESS
@return MT_FAILURE
*/
static void *MT_Standby_irkey(void *param)
{
    mt_s32 Ret =  0;
    MT_UNF_KEY_STATUS_E press_status = { 0 };
    MT_U64 u64KeyId = 0;
    MT_U64 Keyflag = 0;
    char name[64] = { 0 };
    ir_wavefilter_config_s wavefiler = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_U8 protocol = IRDA_SW_MAX;
    MT_U8 last_protocol = IRDA_SW_MAX;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_U8 protocol = RC_PROTO_MAX_;
    MT_U8 last_protocol = RC_PROTO_MAX_;
#endif

    SAMPLE_STANDBY_IR_INFO_PRINT("Use the power button of the remote control to wake up and standby \n");
    while(g_bTaskQuit != MT_TRUE)
    {
        Ret = MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId, name, sizeof(name),3000);
        if(MT_SUCCESS == Ret)
        {
            SAMPLE_STANDBY_IR_INFO_PRINT("%llx\n",u64KeyId);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            protocol = IRDA_NEC;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            protocol = RC_PROTO_NEC_;
#endif

            if(last_protocol != protocol)
            {
                last_protocol = protocol;
            }
            wavefiler.irda_protocol = protocol;
            wavefiler.irda_wfilt_channel = 4;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
            Ret = MT_UNF_IR_SetWaveFilter(wavefiler);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_SetWaveFilter Ret = %d\n", Ret);
                return NULL;
            }
            Ret = MT_UNF_IR_SetKeycode(0);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_SetKeycode Ret = %d\n", Ret);
                return NULL;
            }
            Ret = MT_UNF_IR_SetUsercode(0);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_SetUsercode Ret = %d\n", Ret);
                return NULL;
            }
#elif defined CONFIG_MT_CHIP_SYMPHONY6
            Ret = MT_UNF_IR_SetWaveFilter(&wavefiler);
            if (MT_SUCCESS != Ret)
            {
                SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_SetWaveFilter Ret = %d\n", Ret);
                return NULL;
            }
#endif
            if((Keyflag == 0) && (u64KeyId == KEY_STANDBY_IR_OLD || u64KeyId == KEY_STANDBY_IR_NEW))
            {
                SAMPLE_STANDBY_IR_INFO_PRINT("enter standby\n");
                Keyflag = 1;
                MT_Standby_ir_Standby();
            }
            else if((Keyflag = 1) && (u64KeyId == KEY_STANDBY_IR_OLD || u64KeyId == KEY_STANDBY_IR_NEW))
            {
                SAMPLE_STANDBY_IR_INFO_PRINT("exit standy,wakeup\n");
                Keyflag = 0;
                MT_Standby_ir_Wakeup();
            }
            else
            {
                SAMPLE_STANDBY_IR_INFO_PRINT("Press the standby wake up button on the remote control to use sample\n");
            }
        }

        usleep(5000);
    }
    return NULL;
}

static MT_VOID MT_Standby_irPrintMenu(void)
{

#ifdef MT_SAMPLE_APP
    SAMPLE_STANDBY_IR_PRINT("     b : background run \n");
#endif
    SAMPLE_STANDBY_IR_PRINT("     h : help \n");
    SAMPLE_STANDBY_IR_PRINT("     q : quit \n");
    SAMPLE_STANDBY_IR_PRINT("STANDBY_IR>> ");

}


/*
@brief quit
@return void
*/
static void MT_Standby_ir_CmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };

    while (1)
    {
        MT_Standby_irPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if ('q' == inputCmd[0])
        {
            SAMPLE_STANDBY_IR_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        #ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_STANDBY_IR_INFO_PRINT("ir in back!\n");
            break;
        }
        #endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_STANDBY_IR_INFO_PRINT("help info\n");
        }
    }
}

static MT_VOID MT_Standby_IrExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stStandby_irRunInfo.key, NULL);

    (MT_VOID)MT_UNF_IR_DeInit();
    memset(&g_stStandby_irRunInfo, 0xff, sizeof(g_stStandby_irRunInfo));
}

static mt_s32 MT_Standby_IrParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_STANDBY_IR_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHq")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_Standby_IrExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_STANDBY_IR_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_Standby_IrMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = MT_FAILURE;
     MT_UNF_KEY_STATUS_E press_status = MT_UNF_KEY_STATUS_BUTT;
    MT_U64 u64KeyId = 0;
    char name[64] = { 0 };

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }
    ret = MT_Standby_IrParase_args(argc, argv);
    if (MT_FAILURE == ret)
    {
        SAMPLE_STANDBY_IR_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_STANDBY_IR_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
            return ret;
        }

        ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
            goto ERR1;
        }



        ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
            goto ERR2;
        }
#endif
        ret = MT_UNF_IR_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
            goto ERR3;
        }

        ret = MT_UNF_IR_SetRepKeyTimeoutAttr(500);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_SetRepKeyTimeoutAttr ret = %d\n", ret);
            goto ERR4;
        }

        ret = MT_UNF_IR_EnableKeyUp(MT_FALSE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_EnableKeyUp ret = %d\n", ret);
            goto ERR4;
        }

        ret = MT_UNF_IR_EnableRepKey(MT_FALSE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_EnableRepKey ret = %d\n", ret);
            goto ERR4;
        }

        ret = MT_UNF_IR_SetFetchMode(0);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_SetFetchMode ret = %d\n", ret);
            goto ERR4;
        }
#ifdef CONFIG_MT_CHIP_SYMPHONY4
        ret = MT_UNF_IR_Enable(MT_TRUE, IRDA_NEC);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
        ret = MT_UNF_IR_Enable(MT_TRUE, RC_PROTO_NEC_);
#endif
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("MT_UNF_IR_Enable ret = %d\n", ret);
            goto ERR4;
        }

        while(0 == MT_UNF_IR_GetValueWithProtocol(&press_status, &u64KeyId,name, sizeof(name),1000)) //Clear IR DataCache
        {
            MT_USLEEP(5000);
        }
        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_stStandby_irRunInfo.key, NULL, (void * (*)(void *))MT_Standby_irkey, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_STANDBY_IR_ERR_PRINT("pthread_create ret = %d\n", ret);
            goto ERR4;
        }
    }

    (void)MT_Standby_ir_CmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_stStandby_irRunInfo.key, NULL);

ERR4:
    (MT_VOID)MT_UNF_IR_DeInit();

ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();

ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);

ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    memset(&g_stStandby_irRunInfo, 0xff, sizeof(g_stStandby_irRunInfo));
    return 0;
}

