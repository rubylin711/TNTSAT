/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "string.h"
#include "mt_go.h"
#include "mt_type.h"
#include "mt_unf_disp.h"
#include <pthread.h>
#include "mt_unf_video.h"
#include "mt_adp_mpi.h"
#include "sample_mtgo_common.h"

/***************************** Macro Definition ******************************/
#define MTGO_SCROLLTEXT_PRINT        printf
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_handle hLayerSurface;
    mt_handle text_surface;
    mt_handle hLayer;
    mt_handle hFont;
    pthread_t stInjectTSThread;
}SAMPLE_MTGO_SURFACE_INFO_S;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;   //Determine exit loop
static mt_char szText[1025]={ 0 };   // = "This is MT_GO Text Test\n";
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_MtgoScrolltextMain(MT_S32 argc, mt_char *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*****************************************************************************
 *brief Printthe info of reminding
 *return ::void.
*****************************************************************************/
static MT_VOID MT_ScrolltextPrint_Help(MT_CHAR *name)
{
    MTGO_SCROLLTEXT_PRINT("Lack of parameters\n");
    MTGO_SCROLLTEXT_PRINT("\nUsage:\n");
    MTGO_SCROLLTEXT_PRINT("%s\n", name);
    MTGO_SCROLLTEXT_PRINT("    -t: Enter a string\n");
#ifdef MT_SAMPLE_APP
    MTGO_SCROLLTEXT_PRINT("    -q: Exit the background\n");
#endif
    MTGO_SCROLLTEXT_PRINT("example:\n");
    MTGO_SCROLLTEXT_PRINT("    %s -t abcd\n", name);
}


/*****************************************************************************
@brief Gets the value of the key
@return ::void
*****************************************************************************/
static MT_S32 MT_ScrolltextInjectTsTask(MT_VOID *args)
{
    MT_S32 s32Ret = 0;
    MT_U32 textSize = 0;
    MT_S32 count = 0;
    MT_RECT rc = { 0 };
    MT_RECT rcbg = { 0 };
    MT_RECT stRect = { 0 };
    MT_RECT srRect = { 0 };
    SAMPLE_MTGO_SURFACE_INFO_S *scrolltext_info = (SAMPLE_MTGO_SURFACE_INFO_S *)args;
    MTGO_BLTOPT_S blitOpt = { 0 };
    blitOpt.EnableGlobalAlpha = MT_TRUE;
    blitOpt.PixelAlphaComp = MTGO_COMPOPT_SRCOVER;

    textSize = 14*strlen(szText);
    count = textSize;
    rc.x = 90;
    rc.y = 500;
    rc.w = textSize;
    rc.h = 27;
    rcbg.x = 90;
    rcbg.y = 500;
    rcbg.w = 1110;
    rcbg.h = 27;

    while(1)
    {
        rc.x += 2;
        s32Ret = MT_GO_FillRect(scrolltext_info->hLayerSurface, &rcbg, 0xffff0000, MTGO_COMPOPT_NONE);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_FillRect error. ret=0x%x \n", s32Ret);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }

        s32Ret = MT_GO_Blit(scrolltext_info->text_surface, NULL, scrolltext_info->hLayerSurface, &rc, &blitOpt);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", s32Ret);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }

        s32Ret = MT_GO_RefreshLayer(scrolltext_info->hLayer, NULL);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }

        if((rc.x + rc.w) == 1200)
        {
            while(1)
            {
                s32Ret = MT_GO_FillRect(scrolltext_info->hLayerSurface, &rcbg, 0xffff0000, MTGO_COMPOPT_NONE);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_MTGO_ERR_PRINT("MT_GO_FillRect error. ret=0x%x \n", s32Ret);
                    g_bTaskQuit = MT_TRUE;
                    return MT_FAILURE;
                }

                count = count - 2;
                srRect.x = count;
                srRect.y = 0;
                srRect.w = textSize - count;
                srRect.h = 27;

                stRect.x = 90;
                stRect.y = 500;
                stRect.w = textSize - count;
                stRect.h = 27;
                s32Ret = MT_GO_Blit(scrolltext_info->text_surface, &srRect, scrolltext_info->hLayerSurface, &stRect, &blitOpt);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", s32Ret);
                    g_bTaskQuit = MT_TRUE;
                    return MT_FAILURE;
                }


                srRect.x = 0;
                srRect.y = 0;
                srRect.w = count;
                srRect.h = 27;

                stRect.x = 1200 - count;
                stRect.y = 500;
                stRect.w = count;
                stRect.h = 27;
                s32Ret = MT_GO_Blit(scrolltext_info->text_surface, &srRect, scrolltext_info->hLayerSurface, &stRect, &blitOpt);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", s32Ret);
                    g_bTaskQuit = MT_TRUE;
                    return MT_FAILURE;
                }

                s32Ret = MT_GO_RefreshLayer(scrolltext_info->hLayer, NULL);
                if (MT_SUCCESS != s32Ret)
                {
                    SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);
                    g_bTaskQuit = MT_TRUE;
                    return MT_FAILURE;
                }

                if(count == 2)
                {
                    count = textSize;
                    break;
                }
                if(g_bTaskQuit == MT_TRUE)
                {
                    break;
                }
                usleep(5000);    //Delay for some time
            }
            rc.x = 90;
        }
        if(g_bTaskQuit == MT_TRUE)
        {
            break;
        }
    }
    return MT_SUCCESS;
}

static MT_VOID MT_MtgoScrolltextExit(SAMPLE_MTGO_SURFACE_INFO_S *scrolltext_info)
{
    g_bTaskQuit = MT_TRUE;

    pthread_join(scrolltext_info->stInjectTSThread, NULL);

    (MT_VOID)MT_GO_DestroyText(scrolltext_info->hFont);

    (MT_VOID)MT_GO_DestroyLayer(scrolltext_info->hLayer);

    (MT_VOID)MT_GO_DeinitText();

    (MT_VOID)MT_GO_Deinit();

    memset(scrolltext_info, 0, sizeof(*scrolltext_info));
}

static MT_VOID MT_ScrolltextPrintMenu(MT_VOID)
{
    MTGO_SCROLLTEXT_PRINT("commond: \n");
#ifdef MT_SAMPLE_APP
    MTGO_SCROLLTEXT_PRINT("     b : background run \n");
#endif
    MTGO_SCROLLTEXT_PRINT("     q: quit \n");
    MTGO_SCROLLTEXT_PRINT("     h: help \n");
    MTGO_SCROLLTEXT_PRINT("MTGO>> ");
}

static MT_S32 MT_ScrolltextCmdTask(MT_VOID)
{
    char *fgetret=NULL;
    mt_s8 inputCmd[32];

    while (1)
    {
        (MT_VOID)MT_ScrolltextPrintMenu();

        /*SAMPLE_GET_INPUTCMD(InputCmd)*/
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret=fgetret;

        if ('q' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("MTGO scroll text play in back!\n");
            break;
        }
    #endif
        else if ('h' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("Print help info \n");
        }
    }
    return 0;
}


static MT_S32 MT_MtgoScrolltextParase_args(MT_S32 argc, MT_CHAR *argv[], SAMPLE_MTGO_SURFACE_INFO_S *pSurface)
{
    MT_S32 opt = 0;

    SAMPLE_MTGO_FUNCTION_ENTER();

    if(argc < 2 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_ScrolltextPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHqt:")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                (MT_VOID)MT_ScrolltextPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoScrolltextExit(pSurface);
                }
                return MT_TASK_EXIT;
            case 't':
                memcpy(szText, mt_optarg, 127);
                break;
            default:
                (MT_VOID)MT_ScrolltextPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_MtgoScrolltextMain(MT_S32 argc, mt_char *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32 s32Ret = 0;
    MT_U32 textSize = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MT_RECT rc = { 0 };
    MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;   //ID-0x1
    static SAMPLE_MTGO_SURFACE_INFO_S scrolltext_info = { 0 };

    SAMPLE_MTGO_FUNCTION_ENTER();

    s32Ret = MT_MtgoScrolltextParase_args(argc, argv, &scrolltext_info);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    SAMPLE_MTGO_INFO_PRINT("Show Test: [%s] \n", szText);

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        s32Ret = Sample_MTGO_Display_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("Sample_MTGO_Display_Init error. ret=0x%x \n", s32Ret);
            return MT_FAILURE;
        }
        sleep(3);
#endif
        g_bTaskQuit = MT_FALSE;
        s32Ret = MT_GO_Init();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_Init error. ret=0x%x \n", s32Ret);
            goto ERR2;
        }


        s32Ret = MT_GO_InitText();
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_InitText error. ret=0x%x \n", s32Ret);
            goto ERR3;
        }


        /*create layer*/
        s32Ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerDefaultParam error. ret=0x%x \n", s32Ret);
            goto ERR4;
        }


        stLayerInfo.PixelFormat = MTGO_PF_8888;
        stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
        s32Ret = MT_GO_CreateLayer(&stLayerInfo, &scrolltext_info.hLayer);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateLayer error. ret=0x%x \n", s32Ret);
            goto ERR4;
        }


        s32Ret = MT_GO_GetLayerSurface(scrolltext_info.hLayer, &scrolltext_info.hLayerSurface);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", s32Ret);
            goto ERR5;
        }

        /*Create a text output object
        *Need to test the path of the word library to the u disk:\linux\sample\mtgo\res
        */
        s32Ret = MT_GO_CreateText("./res/DroidSansFallbackLegacy.ttf", &scrolltext_info.hFont);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateText error. ret=0x%x \n", s32Ret);
            goto ERR5;
        }

        /* //set text color is write(0xffffffff)*/
        s32Ret = MT_GO_SetTextColor(scrolltext_info.hFont, 0xffffffff);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_SetTextColor error. ret=0x%x \n", s32Ret);
            goto ERR6;
        }


        s32Ret = MT_GO_SetTextStyle(scrolltext_info.hFont, MTGO_TEXT_STYLE_BOLD);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_SetTextStyle error. ret=0x%x \n", s32Ret);
            goto ERR6;
        }

        textSize = 14*strlen(szText);
        rc.x = 90;
        rc.y = 500;
        rc.w = textSize;
        rc.h = 27;
        MTGO_SCROLLTEXT_PRINT("get pixel size: %d\n", rc.w);
        if(textSize >= 1010)
        {
            SAMPLE_MTGO_ERR_PRINT("The input character is out of range\n");
            goto ERR6;
        }


        s32Ret = MT_GO_CreateSurface(rc.w, rc.h, MTGO_PF_8888, &scrolltext_info.text_surface);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", s32Ret);
            goto ERR6;
        }

        s32Ret = MT_GO_TextOutEx(scrolltext_info.hFont, scrolltext_info.text_surface, szText, NULL, MTGO_LAYOUT_LEFT);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_TextOutEx error. ret=0x%x \n", s32Ret);
            goto ERR7;
        }

        s32Ret = pthread_create(&scrolltext_info.stInjectTSThread, NULL, (MT_VOID * (*)(MT_VOID *))MT_ScrolltextInjectTsTask, &scrolltext_info);
        if (s32Ret != 0)
        {
            SAMPLE_MTGO_ERR_PRINT("Thread creation failure!");
            goto ERR7;
        }
    }

    (MT_VOID)MT_ScrolltextCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }



    (MT_VOID)pthread_join(scrolltext_info.stInjectTSThread, NULL);
ERR7:
    MT_GO_FreeSurface(scrolltext_info.text_surface);
ERR6:
    (MT_VOID)MT_GO_DestroyText(scrolltext_info.hFont);
ERR5:
    (MT_VOID)MT_GO_DestroyLayer(scrolltext_info.hLayer);
ERR4:
    (MT_VOID)MT_GO_DeinitText();
ERR3:
    (MT_VOID)MT_GO_Deinit();
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)Sample_MTGO_Display_DeInit();
#endif
    return s32Ret;
}
