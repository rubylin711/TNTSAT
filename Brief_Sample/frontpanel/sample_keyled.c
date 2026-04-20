/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mt_unf_keyled.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_STANDBY_FRONT_DEBUG

#define MT_STANDBY_FRONT_PRINT   printf
#else

#define MT_STANDBY_FRONT_PRINT

#endif

#define SAMPLE_STANDBY_FRONT_FUNCTION_ENTER()       MT_STANDBY_FRONT_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_STANDBY_FRONT_FUNCTION_EXIT()        MT_STANDBY_FRONT_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_STANDBY_FRONT_FATAL_PRINT(fmt...)        MT_STANDBY_FRONT_PRINT(" [FATAL] " fmt)
#define SAMPLE_STANDBY_FRONT_ERR_PRINT(fmt...)          MT_STANDBY_FRONT_PRINT(" [ERROR] " fmt)
#define SAMPLE_STANDBY_FRONT_WARN_PRINT(fmt...)         MT_STANDBY_FRONT_PRINT(" [WARN] "  fmt)
#define SAMPLE_STANDBY_FRONT_INFO_PRINT(fmt...)         MT_STANDBY_FRONT_PRINT(" [INFO] "  fmt)
#define SAMPLE_STANDBY_FRONT_DBG_PRINT(fmt...)          MT_STANDBY_FRONT_PRINT(" [DEBUG] " fmt)

#define PRINTF_KEY printf
/*************************** Structure Definition ****************************/
/********************** Global Variable declaration **************************/
static mt_s32 g_s32TaskRunning = 0;
static mt_u8 g_DigDisCode_ct1642[10] = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
static mt_u8 g_DigDisCode_fd650[10]  = {0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};
const mt_char g_keyled_name[5][16] ={ "FD650", "CT1642", "NULL", "PT6393",};
static mt_u8 g_DigDisCode[10] = { 0 };
/******************************* API declaration *****************************/

/*****************************************************************************
@brief Monitor the status of keys
*param[in] args, void
@return::void
*****************************************************************************/
void * KEY_ReceiveTask(void *args)
{
    mt_s32 s32Ret;
    mt_u32 u32PressStatus, u32KeyId;

    SAMPLE_STANDBY_FRONT_FUNCTION_ENTER();

    while (g_s32TaskRunning == 1)
    {
        /*get KEY press value & press status*/
        s32Ret = MT_UNF_KEY_GetValue(&u32PressStatus, &u32KeyId);
        if (MT_SUCCESS == s32Ret)
        {
            PRINTF_KEY("KEY  KeyId : 0x%x    PressStatus :%d[%s]\n", u32KeyId, u32PressStatus
                , (0 == u32PressStatus) ? "DOWN"  : (1 == u32PressStatus) ? "HOLD" : "UP");
        }
        else
        {
            MT_USLEEP(50000);
        }

    }
    SAMPLE_STANDBY_FRONT_FUNCTION_EXIT();
    return 0;
}

/*****************************************************************************
@brief The front panel type is pt6393
@param[in] type        Frontpanel type
@param[in] len         length
@return::void
*****************************************************************************/
static void keyled_pt6393(int type, int len)
{
    MT_UNF_VFD_DIS_SPECIAL_CHAR  special_dis_cfg = { 0 };
    MT_UNF_DIS_PLAY_LBD_E lbd_data = { 0 };
    if (1 == type)
    {
        MT_UNF_LED_Vfd_Display((u8 *)"11111111111", len);
    }
    else if (2 == type)
    {
        MT_UNF_LED_Vfd_Display((u8 *)"00000000000", 11);
        sleep(2);

        MT_UNF_LED_Vfd_Display((u8 *)"11111111111", 11);
        sleep(2);
        MT_UNF_LED_Vfd_Display((u8 *)"Reset 1 ok", 11);
        sleep(2);

        MT_UNF_LED_Vfd_Display((u8 *)"22222222222", 11);
        sleep(2);
        MT_UNF_LED_Vfd_Display((u8 *)"Reset 2 ok", 11);
        sleep(2);

        MT_UNF_LED_Vfd_Display((u8 *)"33333333333", 11);
        sleep(2);
        MT_UNF_LED_Vfd_Display((u8 *)"Reset 3 ok", 11);
    }
    else
    {
        MT_UNF_LED_Vfd_Display((u8 *)"0123456789A", 11);
        sleep(2);

        MT_UNF_LED_Vfd_Display((u8 *)"abcdefghijk", 11);
        sleep(1);
        MT_UNF_LED_Vfd_Display((u8 *)"lmnopqrstuv", 11);
        sleep(1);
        MT_UNF_LED_Vfd_Display((u8 *)"wxyz", 4);
        sleep(1);

        MT_UNF_LED_Vfd_Display((u8 *)"test book g",11);
        sleep(1);
        lbd_data.enable = MT_TRUE;
        lbd_data.grid_pos= 0;
        MT_UNF_LED_Display_LBD( &lbd_data);
        sleep(1);
        lbd_data.enable = MT_FALSE;
        lbd_data.grid_pos = 0;
        MT_UNF_LED_Display_LBD(&lbd_data);
        lbd_data.enable = MT_TRUE;
        lbd_data.grid_pos = 1;
        MT_UNF_LED_Display_LBD( &lbd_data);
        sleep(1);
        lbd_data.enable = MT_FALSE;
        lbd_data.grid_pos = 1;
        MT_UNF_LED_Display_LBD(&lbd_data);
        sleep(1);

        special_dis_cfg.ch = REC;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = PLAY;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = USB;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = TSHIFT;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = MOVIE;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = MP3;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = JPG;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = ALL;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = CYCLE;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = SATTV;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = RADIO;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = STEREO;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = AUDL;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);

        special_dis_cfg.ch = AUDR;
        special_dis_cfg.on_off = 1;
        MT_UNF_LED_Vfd_Display_Special_char(&special_dis_cfg);
        sleep(1);
    }
}

int  main(int argc, char *argv[])
{
    mt_s32 s32Ret = 0;
    int type = 0;
    int strlen = 0;
    pthread_t keyTaskid = { 0 };
    MT_UNF_KEYLED_TYPE_E keyled_type = { 0 };

    PRINTF_KEY("Show led and wait key press\n");

    keyled_type = (mt_u32)strtol(argv[1], NULL, 0);
    type = (mt_u32)strtol(argv[2], NULL, 0);
    strlen = (mt_u32)strtol(argv[3], NULL, 0);


    if(argc != 2 && argc != 4)
    {
        PRINTF_KEY("usage:sample_keyled [keyled_type]\n"
                   "keyled_type = 0: fd650 \n"
                   "keyled_type = 1: CT1642 \n"
                   "keyled_type = 4: PT6393 \n");
        return MT_FAILURE;
    }

    PRINTF_KEY("Test keyled_type %s\n", g_keyled_name[keyled_type]);

    if(keyled_type == 0)
    {
        memcpy(g_DigDisCode, g_DigDisCode_fd650, sizeof(g_DigDisCode_fd650));
    }
    else if(keyled_type == 1)
    {
        memcpy(g_DigDisCode, g_DigDisCode_ct1642, sizeof(g_DigDisCode_ct1642));
    }
    else if(keyled_type == 4)
    {

    }

    s32Ret = mt_sys_init();
    if(MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("MT_SYS_Init failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    s32Ret = MT_UNF_KEYLED_Init();
    if(MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("MT_UNF_KEYLED_Init failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
        return s32Ret;
    }

    PRINTF_KEY("init ok1\n");
    PRINTF_KEY("MT_UNF_KEYLED_Open\n");

    /* open LED device */
    s32Ret = MT_UNF_LED_Open();
    if (MT_SUCCESS != s32Ret)
    {
        printf("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
        return s32Ret;
    }

    PRINTF_KEY("MT_UNF_KEYLED_Open end!\n");

    s32Ret = MT_UNF_KEYLED_SelectType(keyled_type);
    if (MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
        return s32Ret;
    }

    if(keyled_type == 4)
    {
        (void)keyled_pt6393(type,strlen);
    }

    s32Ret = MT_UNF_KEY_RepKeyTimeoutVal(200);
    if (MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
        goto ERR2;
    }

    s32Ret = MT_UNF_KEY_IsRepKey(1);
    if (MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
        goto ERR2;
    }

    s32Ret = MT_UNF_KEY_IsKeyUp(0);
    if (MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
        goto ERR2;
    }

    s32Ret = MT_UNF_LED_Display(0x3f065b4f); //0123
    if (MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("%s: %d MT_UNF_LED_Display=0x%x\n", __FILE__, __LINE__, s32Ret);
        goto ERR2;
    }

    g_s32TaskRunning = 1;

    /* create a thread for receive */
    s32Ret = pthread_create(&keyTaskid, NULL, KEY_ReceiveTask, NULL);
    if (0 != s32Ret)
    {
        PRINTF_KEY("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
        perror("pthread_create");
        goto ERR3;
    }


    pthread_join(keyTaskid, 0);
    (void)MT_UNF_LED_Close();
    (void)MT_UNF_KEYLED_DeInit();

    s32Ret = mt_sys_deinit();
    if(MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("DVB_DmxDeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    PRINTF_KEY("\nrun keyled demo success\n");
    return MT_SUCCESS;

ERR3:
    g_s32TaskRunning = 0;
ERR2:
    MT_UNF_LED_Close();
ERR1:
    MT_UNF_LED_Close();

    MT_UNF_KEYLED_DeInit();
    s32Ret = mt_sys_deinit();
    if(MT_SUCCESS != s32Ret)
    {
        PRINTF_KEY("DVB_DmxDeInit failed, ret = %d-------<%s> line: %d\n", s32Ret,  __FUNCTION__, __LINE__);
    }

    PRINTF_KEY("run keyled demo failed\n");
    return s32Ret;
}



