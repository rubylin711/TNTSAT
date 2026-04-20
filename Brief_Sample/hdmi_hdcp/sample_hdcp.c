/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_type.h"
#include "mt_unf_hdmi.h"
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "mt_unf_hdcp.h"
#include "mt_unf_flash.h"
#include "mt_go.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_mpi.h"


/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_HDCP_DEBUG

#define MT_HDCP_PRINT   printf
#else

#define MT_HDCP_PRINT

#endif

#define SAMPLE_HDCP_FUNCTION_ENTER()    MT_HDCP_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_HDCP_FUNCTION_EXIT()     MT_HDCP_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_HDCP_FATAL_PRINT(fmt...)         MT_HDCP_PRINT(" [FATAL] " fmt)
#define SAMPLE_HDCP_ERR_PRINT(fmt...)           MT_HDCP_PRINT(" [ERROR] " fmt)
#define SAMPLE_HDCP_WARN_PRINT(fmt...)          MT_HDCP_PRINT(" [WARN] "  fmt)
#define SAMPLE_HDCP_INFO_PRINT(fmt...)          MT_HDCP_PRINT(" [INFO] "  fmt)
#define SAMPLE_HDCP_DBG_PRINT(fmt...)           MT_HDCP_PRINT(" [DEBUG] " fmt)

#define SAMPLE_HDCP_PRINT   printf

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2


/*************************** Structure Definition ****************************/
typedef struct mtHDMI_ARGS_S
{
    MT_UNF_HDMI_ID_E enHdmi;
} HDMI_ARGS_S;


/********************** Global Variable declaration **************************/

#ifdef CONFIG_MT_CHIP_SYMPHONY4
static mt_u8 hdcp_testkey[304] = {
};
#endif

static unsigned char g_hdcp_key_m2m_hdmi20[292] = {0};


static MT_BOOL g_loadflag = MT_FALSE;
static MT_BOOL g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
MT_S32 MT_HdcpMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*
@brief hdcp enable
@param[in] enHDMIId, HDMI interface ID
@param[in] hdcp_enable, hdcp on/off true or false
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_HdcpEnable(MT_UNF_HDMI_ID_E enHDMIId, MT_BOOL hdcp_enable)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_HDMI_ATTR_S stHdmiAttr;
    MT_UNF_EDID_BASE_INFO_S stSinkCap;

    memset(&stHdmiAttr, 0, sizeof(MT_UNF_HDMI_ATTR_S));
    ret = MT_UNF_HDMI_GetAttr(enHDMIId, &stHdmiAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT(" MT_UNF_HDMI_GetAttr failed!\n");
        return MT_FAILURE;
    }

    memset(&stSinkCap, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
    ret = MT_UNF_HDMI_GetSinkCapability(enHDMIId, &stSinkCap);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT(" MT_UNF_HDMI_GetSinkCapability failed, Err_Code=0x%x!\n", ret);
        return MT_FAILURE;   // if hotplug out, it will be return false
    }

    if (ret == MT_SUCCESS)
    {
        if (MT_TRUE == stSinkCap.bSupportHdmi)
        {
            stHdmiAttr.bEnableHdmi = MT_TRUE;
        }
        else
        {
            //读取到了edid，并且不支持hdmi则进入dvi模式
            //read real edid ok && sink not support hdmi,then we run in dvi mode
            stHdmiAttr.bEnableHdmi = MT_FALSE;
        }
    }
    else
    {
        //when get capability fail,use default mode
        stHdmiAttr.bEnableHdmi = MT_TRUE;
    }

    if (MT_TRUE == stHdmiAttr.bEnableHdmi)
    {
        stHdmiAttr.bEnableAudio = MT_TRUE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_TRUE;
        stHdmiAttr.bEnableAviInfoFrame = MT_TRUE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_BUTT;// MT_UNF_HDMI_VIDEO_MODE_BUTT will auto output
    }
    else
    {
        stHdmiAttr.bEnableAudio = MT_FALSE;
        stHdmiAttr.bEnableVideo = MT_TRUE;
        stHdmiAttr.bEnableAudInfoFrame = MT_FALSE;
        stHdmiAttr.bEnableAviInfoFrame = MT_FALSE;
        stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
    }

    stHdmiAttr.bHDCPEnable = hdcp_enable;

    ret = MT_UNF_HDMI_SetAttr(enHDMIId, &stHdmiAttr);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT(" MT_UNF_HDMI_SetAttr failed!\n");
        return MT_FAILURE;
    }

    ret = MT_UNF_HDMI_Start(enHDMIId);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT("MT_UNF_HDMI_Start failed\n");
    }

    return MT_SUCCESS;
}

/*
@brief hdcp key load
@param void
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_Hdcpkey_load(void)
{
    mt_s32 ret = MT_SUCCESS;
    mt_s32 key_len = 0;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_u32 hdcp_key_len = 304;
#endif
    MT_UNF_HDMI_LOAD_KEY_S stLoadKey = { 0 };

    memset(&stLoadKey, 0 , sizeof(MT_UNF_HDMI_LOAD_KEY_S));
    key_len = sizeof(g_hdcp_key_m2m_hdmi20)/sizeof(g_hdcp_key_m2m_hdmi20[0]);
    memset(g_hdcp_key_m2m_hdmi20, 0, key_len);
    (void)MTADP_HDMI_ReadHdcpKey(MT_UNF_HDMI_ID_0, g_hdcp_key_m2m_hdmi20, key_len);

    SAMPLE_HDCP_INFO_PRINT("use local test data\n");


#ifdef CONFIG_MT_CHIP_SYMPHONY4
    stLoadKey.u32KeyLength = hdcp_key_len;
    stLoadKey.pu8InputEncryptedKey = (mt_u8 *)malloc(hdcp_key_len);
    memcpy(stLoadKey.pu8InputEncryptedKey, hdcp_testkey, hdcp_key_len);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    stLoadKey.u32KeyLength = key_len;
    stLoadKey.pu8InputEncryptedKey = (mt_u8 *)malloc(key_len);
    memcpy(stLoadKey.pu8InputEncryptedKey, g_hdcp_key_m2m_hdmi20, key_len);
#endif

    ret = MT_UNF_HDMI_LoadHDCPKey(MT_UNF_HDMI_ID_0, &stLoadKey);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT(" MT_UNF_HDMI_LoadHDCPKey failed!\n");
        return MT_FAILURE;
    }
    free(stLoadKey.pu8InputEncryptedKey);
    return MT_SUCCESS;
}

/*
@brief hdcp key clear
@param void
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_Hdcpkey_clear(void)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_HDMI_LOAD_KEY_S stLoadKey;

    memset(&stLoadKey, 0 , sizeof(MT_UNF_HDMI_LOAD_KEY_S));
    ret = MT_UNF_HDMI_LoadHDCPKey(MT_UNF_HDMI_ID_0, &stLoadKey);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT(" MT_UNF_HDMI_LoadHDCPKey failed!\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_HdcpOn(void)
{
    mt_s32 ret = MT_SUCCESS;
    MT_BOOL hdcp_flag = 1;

    ret = MT_HdcpEnable(MT_UNF_HDMI_ID_0, hdcp_flag);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT("MT_HdcpEnable failed\n");

    }

    return MT_SUCCESS;
}

/*
@brief hdcp off operation
@param void
@return MT_SUCCESS
@return MT_FAILURE
*/
static mt_s32 MT_HdcpOff(void)
{
    mt_s32 ret = MT_SUCCESS;
    MT_BOOL hdcp_flag = 0;

    ret = MT_HdcpEnable(MT_UNF_HDMI_ID_0, hdcp_flag);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_HDCP_ERR_PRINT("MT_HdcpEnable failed\n");

    }

    return MT_SUCCESS;
}

static MT_VOID MT_HdcpPrintMenu(MT_VOID)
{
    SAMPLE_HDCP_PRINT("\n");
    SAMPLE_HDCP_PRINT("     s : set key \n");
    SAMPLE_HDCP_PRINT("     e : hdcp enable \n");
    SAMPLE_HDCP_PRINT("     d : hdcp disable \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_HDCP_PRINT("     b : background run \n");
#endif
    SAMPLE_HDCP_PRINT("     h : help \n");
    SAMPLE_HDCP_PRINT("     q : quit \n");
    SAMPLE_HDCP_PRINT("HDCP>> ");

}

static void MT_HdcpCmdTask(void)
{
    MT_CHAR    inputCmd[32] = { 0 };
    MT_CHAR    *pfgetret = NULL;

    while(1)
    {
        (MT_VOID)MT_HdcpPrintMenu();
        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_HDCP_INFO_PRINT("<Exit!>\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_HDCP_INFO_PRINT("HDCP in back!\n");
            break;
        }
#endif
        else if('s' == inputCmd[0])
        {
            g_loadflag = 1;
            (MT_VOID)MT_Hdcpkey_load();
            SAMPLE_HDCP_INFO_PRINT("load key\n");
            continue;
        }
        else if('e' == inputCmd[0])
        {
            (MT_VOID)MT_HdcpOn();
            (mt_void)MTADP_HDMI_Set_HdcpEnable(MT_TRUE);
            SAMPLE_HDCP_INFO_PRINT("hdcp enable\n");
            continue;
        }
        else if('d' == inputCmd[0])
        {
            (MT_VOID)MT_HdcpOff();
            (mt_void)MTADP_HDMI_Set_HdcpEnable(MT_FALSE);
            SAMPLE_HDCP_INFO_PRINT("hdcp disable\n");
            SAMPLE_HDCP_INFO_PRINT("Opening hdcp again requires a new set key \n");
            continue;

        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_HDCP_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

static MT_VOID MT_HdcpExit(void)
{
    g_bTaskQuit = MT_TRUE;
}

static mt_s32 MT_HdcpParase_args(int argc, char *argv[])
{
    int opt = 0;


    SAMPLE_HDCP_FUNCTION_ENTER();

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
                   (MT_VOID)MT_HdcpExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_HDCP_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_HdcpMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32      Ret = 0;

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }

    Ret = MT_HdcpParase_args(argc, argv);
    if (MT_FAILURE == Ret)
    {
        SAMPLE_HDCP_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == Ret)
    {
        SAMPLE_HDCP_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        Ret = mt_sys_init();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_HDCP_ERR_PRINT("mt_sys_init failed\n");
            return MT_FAILURE;
        }
        Ret = MTADP_HDMI_Init(MT_UNF_HDMI_ID_0, MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_HDCP_ERR_PRINT("MTADP_HDMI_Init failed\n");
            goto ERR1;
        }

        Ret = MTADP_Disp_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_HDCP_ERR_PRINT("MTADP_Disp_Init failed\n");
            goto ERR2;
        }
#endif
        Ret = MT_Hdcpkey_clear();
        if(MT_SUCCESS != Ret)
        {
            SAMPLE_HDCP_ERR_PRINT("sample_hdcpkey_clear failed\n");
            goto ERR3;
        }

        g_bTaskQuit = MT_FALSE;

    }

    (MT_VOID)MT_HdcpCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR3:
#ifndef MT_SAMPLE_APP
    (MT_VOID)MTADP_Disp_DeInit();
ERR2:
    (MT_VOID)MTADP_HDMI_DeInit(MT_UNF_HDMI_ID_0);
ERR1:
    (MT_VOID)mt_sys_deinit();
#endif
    return 0;
}
