/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "mt_type.h"
#include "mt_common.h"
#include "download_api.h"
/***************************** Macro Definition ******************************/
#define MT_SAMPLE_DOWNLOAD_DEBUG
#ifdef  MT_SAMPLE_DOWNLOAD_DEBUG
#define MT_DOWNLOAD_PRINT   printf
#else
#define MT_DOWNLOAD_PRINT
#endif

#define SAMPLE_DOWNLOAD_FUNCTION_ENTER()        MT_DOWNLOAD_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_DOWNLOAD_FUNCTION_EXIT()         MT_DOWNLOAD_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_DOWNLOAD_FATAL_PRINT(fmt...)     MT_DOWNLOAD_PRINT(" [FATAL] " fmt)
#define SAMPLE_DOWNLOAD_ERR_PRINT(fmt...)       MT_DOWNLOAD_PRINT(" [ERROR] " fmt)
#define SAMPLE_DOWNLOAD_WARN_PRINT(fmt...)      MT_DOWNLOAD_PRINT(" [WARN] "  fmt)
#define SAMPLE_DOWNLOAD_INFO_PRINT(fmt...)      MT_DOWNLOAD_PRINT(" [INFO] "  fmt)
#define SAMPLE_DOWNLOAD_DBG_PRINT(fmt...)       MT_DOWNLOAD_PRINT(" [DEBUG] " fmt)

#define SAMPLE_DOWNLOAD_PRINT   printf

#define MAX_DOWNLOAD_TIMEOUT    60
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
extern int Nw_DownloadURLTimeout(const char * url,
                              const char * tempFile,
                              unsigned int timeoutSec,
                              void * response,
                              void * arg,
                              const char * extraHeaders,
                              const char  * body,
                              unsigned int bodyLen);

static MT_S32 MT_DownloadFileName(MT_CHAR *url, MT_CHAR *fileName)
{
    MT_S32  i = 0;
    MT_CHAR name[1024] = { 0 };
    MT_CHAR *p = NULL;

    p = url;

    while(*p != '\0')
    {
        if(*p == '/')
        {
            i = 0;
            p++;
            if(*p == '\0')
            {
                SAMPLE_DOWNLOAD_ERR_PRINT("No file specified\n");
                return MT_FAILURE;
            }
            memset(name, 0, 1024);
        }
        name[i] = *p;
        i++;
        p++;
    }
    strcpy(fileName, name);

    return MT_SUCCESS;
}


static void MT_DownloadDownloadFile(MT_CHAR *url)
{
    MT_S32  ret = MT_FAILURE;
    MT_CHAR fileName[1024] = { 0 };
    MT_CHAR filePath[1024] = { 0 };

    SAMPLE_DOWNLOAD_PRINT("Please enter the path to save it(eg: /media/casetest/): ");
    scanf("%s", filePath);
    getchar();
    ret = MT_DownloadFileName(url, fileName);
    if(MT_SUCCESS != ret)
    {
        return;
    }

    SAMPLE_DOWNLOAD_INFO_PRINT("download start......\n");
    SAMPLE_DOWNLOAD_INFO_PRINT("url: %s\n", url);
    SAMPLE_DOWNLOAD_INFO_PRINT("filename: %s\n", fileName);
    strcat(filePath, fileName);
    SAMPLE_DOWNLOAD_INFO_PRINT("filePath: %s\n", filePath);


    ret = Nw_DownloadURLTimeout(url, filePath, MAX_DOWNLOAD_TIMEOUT, NULL, NULL, NULL, NULL, 0);
    if(ret == 1)
    {
        SAMPLE_DOWNLOAD_INFO_PRINT("http/https get download to file success\n");
    }
    else
    {
        SAMPLE_DOWNLOAD_ERR_PRINT("http/https get download to file failed\n");
    }

    return;
}


static MT_VOID MT_DownloadNetConnect(MT_VOID)
{
    if(system("udhcpc") < 0)
    {
        SAMPLE_DOWNLOAD_ERR_PRINT("error occured\n");
    }
}

static MT_VOID MT_DownloadPrintMenu(MT_VOID)
{
    SAMPLE_DOWNLOAD_PRINT("commond: \n");
    SAMPLE_DOWNLOAD_PRINT("     d: download the file\n");
    SAMPLE_DOWNLOAD_PRINT("     c: change the download file path\n");
    SAMPLE_DOWNLOAD_PRINT("     q: quit \n");
    SAMPLE_DOWNLOAD_PRINT("     h: help \n");
    SAMPLE_DOWNLOAD_PRINT("DOWNLOAD>> ");
}


static MT_VOID MT_DownloadCmdTask(MT_CHAR *getUrl)
{
    MT_CHAR inputCmd[32] = { 0 };
    MT_CHAR url[1024] = { 0 };

    strcpy(url, getUrl);

    (MT_VOID)MT_DownloadNetConnect();

    while(1)
    {
        (MT_VOID)MT_DownloadPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_DOWNLOAD_INFO_PRINT("now exit!\n");
            break;
        }
        else if('d' == inputCmd[0])
        {
            SAMPLE_DOWNLOAD_INFO_PRINT("The path of the current download file: %s\n", url);

            (MT_VOID)MT_DownloadDownloadFile(url);
        }
        else if('c' == inputCmd[0])
        {
            memset(url, 0, 1024);
            SAMPLE_DOWNLOAD_PRINT("Please enter the file path:\n");
            SAMPLE_DOWNLOAD_PRINT("DOWNLOAD>> ");
            scanf("%s", url);
            getchar();
            SAMPLE_DOWNLOAD_INFO_PRINT("Change the download file path: %s\n", url);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_DOWNLOAD_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

static MT_VOID MT_DownloadPrint_Help(MT_CHAR *name)
{
    SAMPLE_DOWNLOAD_PRINT("Lack of parameters\n");
    SAMPLE_DOWNLOAD_PRINT("\nUsage:\n");
    SAMPLE_DOWNLOAD_PRINT("%s\n", name);
    SAMPLE_DOWNLOAD_PRINT("    -f: File path\n");
    SAMPLE_DOWNLOAD_PRINT("example:\n");
    SAMPLE_DOWNLOAD_PRINT("    %s -f http://192.168.32.83:8080/download/test.mp3\n", name);
}

static MT_S32 MT_DownloadParase_args(MT_S32 argc, MT_CHAR *argv[], MT_CHAR url[4096])
{
    MT_S32 opt = 0;

    if(argc != 3)
    {
        (MT_VOID)MT_DownloadPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_DownloadPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                strcpy(url, argv[2]);
                break;
            default:
                (MT_VOID)MT_DownloadPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_DownloadMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32  ret = MT_FAILURE;
    MT_CHAR url[1024] = { 0 };

    ret = MT_DownloadParase_args(argc, argv, url);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_DOWNLOAD_ERR_PRINT("MT_SYS_Init failed, ret = %d-------<%s> line: %d\n", ret,  __FUNCTION__, __LINE__);
        return ret;
    }
#endif

    (MT_VOID)MT_DownloadCmdTask(url);

#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return 0;
}


