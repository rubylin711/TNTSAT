/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mt_go.h"
#include "mt_type.h"
#include "mt_unf_disp.h"
#include "mt_adp_mpi.h"

#include "mt_unf_video.h"

#include "sample_mtgo_common.h"

/***************************** Macro Definition ******************************/
#define MTGO_TEXT_PRINT        printf
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

/*************************** Structure Definition ****************************/
typedef struct
{
    mt_handle hFont;
    MT_RECT   rect;
    MT_COLOR  textColor;
    MT_COLOR  bgColor;
    MTGO_TEXT_STYLE_E style;
    MTGO_LAYOUT_E   layout;
    mt_char   szText[128];
}SAMPLE_MTGO_TEXT_INFO_S;

typedef struct
{
    mt_handle hLayer;
    mt_handle hSurface;
    MT_RECT   rect;
    MT_COLOR  color;
    SAMPLE_MTGO_TEXT_INFO_S stext;
}SAMPLE_MTGO_SURFACE_INFO_S;


/********************** Global Variable declaration **************************/
static MT_BOOL    g_bTaskQuit = MT_TRUE;
mt_char g_szText[1025]={0}; // = "This is MT_GO Text Test\n";


/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoTextMain(mt_s32 argc, mt_char *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_VOID MT_MtgoTextExit(mt_handle hLayer, mt_handle hFont)
{

        MT_GO_DestroyText(hFont);
        MT_GO_DestroyLayer(hLayer);
        MT_GO_DeinitText();
        MT_GO_Deinit();

        g_bTaskQuit = MT_TRUE;
}

static MT_VOID MT_MtgoTextPrintMenu(void)
{
    MTGO_TEXT_PRINT("\n");
    MTGO_TEXT_PRINT("     c : text color \n");
    MTGO_TEXT_PRINT("     g : background color \n");
    MTGO_TEXT_PRINT("     t : text \n");
    MTGO_TEXT_PRINT("     l : location \n");
    MTGO_TEXT_PRINT("     h : help \n");
#ifdef MT_SAMPLE_APP
    MTGO_TEXT_PRINT("     b : background run \n");
#endif
    MTGO_TEXT_PRINT("     q : quit \n");
    MTGO_TEXT_PRINT("=============================\n");
    MTGO_TEXT_PRINT("MTGO>> ");

}


static void MT_MtgoTextPrint_usage(MT_CHAR *name)
{
    MTGO_TEXT_PRINT("\n");
    MTGO_TEXT_PRINT("Options:\n");
    MTGO_TEXT_PRINT(" ?/-h/-H         print this help\n");
    MTGO_TEXT_PRINT(" -q              Quit back play\n");
    MTGO_TEXT_PRINT(" -t              Show text value \n");
    MTGO_TEXT_PRINT("example: \n");
    MTGO_TEXT_PRINT(" %s -t abcdef\n", name);
}

#if 0
static MT_U32 MT_MtgoTextParseColor(char  *colorStr)
{
    MT_COLOR color = 0;
    char *p = colorStr;

    SAMPLE_MTGO_INFO_PRINT("Pase color: %s [%lu] \n", colorStr, strlen(p));

    if(*p == '0' && *(p+1) == 'x')
    {
        p += 2;
    }
    printf("text: %s\n", p);
    if(strlen(p) != 9)
    {
        SAMPLE_MTGO_ERR_PRINT("Inpur color length error.[%lu] \n", strlen(p));
        return color;
    }

    while('\0' != *p)
    {
        if((*p >= '0' && *p <= '9') ||
            (*p >= 'a' && *p <= 'f') ||
            (*p >= 'A' && *p <= 'F'))
        {
            p++;
            continue;
        }
        else
        {
            SAMPLE_MTGO_ERR_PRINT("Inpur color chart error.\n");
            return color;
        }
    }


    color = strtoul(colorStr, NULL, 16);


    return color;
}
#endif

static MT_S32 MT_MtgoTextInit(mt_handle *phLayer, mt_handle *phSurface, mt_handle *phFont)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    mt_handle hSurface = MTGO_INVALID_HANDLE;
    mt_handle hFont = MTGO_INVALID_HANDLE;
    mt_handle hLayer = MTGO_INVALID_HANDLE;

    SAMPLE_MTGO_FUNCTION_ENTER();

    s32Ret = MT_GO_Init();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_Init error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    s32Ret = MT_GO_GetLayerDefaultParam(MTGO_LAYER_OSD0, &stLayerInfo);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerDefaultParam error. ret=0x%x \n", s32Ret);
        goto ERR1;
    }

    stLayerInfo.PixelFormat = MTGO_PF_8888;
    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
    s32Ret = MT_GO_CreateLayer(&stLayerInfo, &hLayer);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateLayer error. ret=0x%x \n", s32Ret);
        goto ERR1;
    }

    s32Ret = MT_GO_GetLayerSurface(hLayer, &hSurface);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", s32Ret);
        goto ERR2;
    }

    s32Ret = MT_GO_InitText();
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_InitText error. ret=0x%x \n", s32Ret);
        goto ERR2;
    }

    s32Ret = MT_GO_CreateText("./res/DroidSansFallbackLegacy.ttf", &hFont);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateText error. ret=0x%x \n", s32Ret);
        goto ERR3;
    }

    *phLayer = hLayer;
    *phSurface = hSurface;
    *phFont = hFont;

    g_bTaskQuit = MT_FALSE;

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;

ERR3:
    MT_GO_DeinitText();
ERR2:
    MT_GO_DestroyLayer(hLayer);

ERR1:
    MT_GO_Deinit();

    return MT_FAILURE;
}

static mt_s32 MT_MtgoTextDrawSurface(SAMPLE_MTGO_SURFACE_INFO_S *phSurface)
{
    mt_s32 s32Ret = MT_SUCCESS;

    phSurface->color = 0x55ffffff;
    s32Ret = MT_GO_FillRect(phSurface->hSurface, NULL, phSurface->color, MTGO_COMPOPT_NONE);  //set background color is write(0xffffffff)
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_FillRect error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_MtgoTextDrawText(mt_handle hSurface, SAMPLE_MTGO_TEXT_INFO_S *pTextInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;

    SAMPLE_MTGO_INFO_PRINT("Show Test: [%s] \n", pTextInfo->szText);  //??? printf ascII

    s32Ret = MT_GO_SetTextColor(pTextInfo->hFont, pTextInfo->textColor);  //set text color is red(0xffff0000)
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_SetTextColor error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    s32Ret = MT_GO_SetTextStyle(pTextInfo->hFont, pTextInfo->style);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_SetTextStyle error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }


    s32Ret = MT_GO_DrawRect(hSurface, &pTextInfo->rect, pTextInfo->bgColor);  //set text ground area and color green(0xff00ff00)
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DrawRect error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    s32Ret = MT_GO_TextOutEx(pTextInfo->hFont, hSurface, pTextInfo->szText, &pTextInfo->rect, pTextInfo->layout);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_TextOutEx error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


static mt_s32 MT_MtgoTextCmdTask(SAMPLE_MTGO_SURFACE_INFO_S *phSurface)
{
    mt_s32 s32Ret = MT_SUCCESS;
    char *fgetret=NULL;
    mt_s8 inputCmd[128];
    MT_COLOR color = 0;
    MT_S32 x = 0;
    MT_S32 y = 0;

    while (1)
    {
        (MT_VOID)MT_MtgoTextPrintMenu();
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);//SAMPLE_GET_INPUTCMD(InputCmd);
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
            SAMPLE_MTGO_INFO_PRINT("MTGO text play in back!\n");
            break;
        }
#endif
        else if('c' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("MTGO text set color!\n"); //text color
            MTGO_TEXT_PRINT("Input text color(eg: 0xff00ff00): \n>> ");
            scanf("%x", &color);
            getchar();
            if(color != 0)
            {
                phSurface->stext.textColor = color;
            }
            (MT_VOID)MT_MtgoTextDrawText(phSurface->hSurface, &phSurface->stext);
        }
        else if('g' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("MTGO text set color!\n"); //background color
            MTGO_TEXT_PRINT("Input text background color(eg: 0xffff0000): \n>> ");
            scanf("%x", &color);
            getchar();
            if(color != 0)
            {
                phSurface->stext.bgColor = color;
            }
            (MT_VOID)MT_MtgoTextDrawText(phSurface->hSurface, &phSurface->stext);
        }
        else if('t' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("MTGO text set text! \n");
            MTGO_TEXT_PRINT("Input text, MAX length is 64.\n");
            MTGO_TEXT_PRINT("Input text: \n>> ");
            scanf("%s", inputCmd);
            getchar();
            if(strlen((mt_char*)inputCmd) > 65)
            {
                SAMPLE_MTGO_ERR_PRINT("Input text has exceed MAX chart.");
                continue;
            }
            SAMPLE_MTGO_INFO_PRINT("inputCmd: %s \n", inputCmd);

            memcpy(phSurface->stext.szText,  inputCmd, 64);
            (MT_VOID)MT_MtgoTextDrawText(phSurface->hSurface, &phSurface->stext);
        }
        else if('l' == inputCmd[0])
        {
            s32Ret = MT_MtgoTextDrawSurface(phSurface);
            if (MT_FAILURE == s32Ret)
            {
                continue;
            }
            SAMPLE_MTGO_INFO_PRINT("MTGO text set text location!\n");
            SAMPLE_MTGO_INFO_PRINT("Input x: \n");
            scanf("%d", &x);
            getchar();
            phSurface->stext.rect.x = x;
            SAMPLE_MTGO_INFO_PRINT("Input y: \n");
            scanf("%d", &y);
            getchar();
            phSurface->stext.rect.y = y;
            (MT_VOID)MT_MtgoTextDrawText(phSurface->hSurface, &phSurface->stext);
        }

        s32Ret = MT_GO_RefreshLayer(phSurface->hLayer, NULL);
        if (MT_SUCCESS != s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);
        }
    }

    return MT_SUCCESS;
}

static MT_S32 MT_MtgoTextParase_args(MT_S32 argc, MT_CHAR *argv[], SAMPLE_MTGO_SURFACE_INFO_S *pSurface)
{
    MT_S32 opt = 0;
    SAMPLE_MTGO_FUNCTION_ENTER();

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        MT_MtgoTextPrint_usage(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHqt:")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                SAMPLE_MTGO_INFO_PRINT("Get opt[%c], run help info.\n", opt);
                (MT_VOID)MT_MtgoTextPrint_usage(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoTextExit(pSurface->hLayer, pSurface->stext.hFont);
                }
                return MT_TASK_EXIT;
            case 't':
                memcpy(pSurface->stext.szText, mt_optarg, 127);
                break;
            default:
                SAMPLE_MTGO_ERR_PRINT("ERR opt[%c], print help info.\n", opt);
                (MT_VOID)MT_MtgoTextPrint_usage(argv[0]);
                return MT_FAILURE;
        }
    }

    if(pSurface->stext.szText[0] == 0)
    {
        SAMPLE_MTGO_ERR_PRINT("Parse argv err. no text.\n");
        return MT_FAILURE;
    }
    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoTextMain(mt_s32 argc, mt_char *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    mt_s32 s32Ret = MT_SUCCESS;
    static SAMPLE_MTGO_SURFACE_INFO_S  surface;

    SAMPLE_MTGO_FUNCTION_ENTER();

    s32Ret = MT_MtgoTextParase_args(argc, argv, &surface);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Recv stop command. stop window.\n");

        memset(&surface, 0, sizeof(surface));

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        s32Ret = Sample_MTGO_Display_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_FAILURE == s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MTGO_Display_Init err. ret= 0x%x.\n", s32Ret);
            return MT_FAILURE;
        }
#endif
        s32Ret = MT_MtgoTextInit(&surface.hLayer, &surface.hSurface, &surface.stext.hFont);
        if (MT_FAILURE == s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
            goto ERR1;
        }
        s32Ret = MT_MtgoTextDrawSurface(&surface);
        if (MT_FAILURE == s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
            goto ERR2;
        }
        surface.stext.textColor = 0xffff0000;
        surface.stext.rect.x = 200;
        surface.stext.rect.y = 100;
        surface.stext.rect.w = 800;
        surface.stext.rect.h = 30;
        surface.stext.bgColor = 0xff00ff00;
        surface.stext.style = MTGO_TEXT_STYLE_BOLD;
        surface.stext.layout = MTGO_LAYOUT_RIGHT;
        s32Ret = MT_MtgoTextDrawText(surface.hSurface, &surface.stext);
        if (MT_FAILURE == s32Ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
            goto ERR2;
        }
    }

    s32Ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);
    }

    (MT_VOID)MT_MtgoTextCmdTask(&surface);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR2:
    MT_MtgoTextExit(surface.hLayer, surface.stext.hFont);
    memset(&surface, 0, sizeof(surface));

ERR1:
#ifndef MT_SAMPLE_APP
    Sample_MTGO_Display_DeInit();
#endif

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}
