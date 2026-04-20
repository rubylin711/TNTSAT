/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <unistd.h>
#include "mt_go.h"
#include "mt_unf_hdmi.h"
#include "mt_adp_mpi.h"
#include "sample_mtgo_common.h"
/***************************** Macro Definition ******************************/
#define MTGO_OPTLAYER_PRINT      printf
#define MT_TASK_RUN              1
#define MT_TASK_EXIT             2
/*************************** Structure Definition ****************************/
typedef struct
{
    mt_handle hLayer;
    mt_handle hLayerSurface;
    mt_handle hMemSurface;
}SAMPLE_MTGO_SURFACE_INFO_S;
/********************** Global Variable declaration **************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;

static MT_U32 g_rgb_clut_palette[6] =
{
    0xFF0000FF,  //blue
    0xFF00FF00,  //green
    0xFFFF0000,  //red
    0xFFFFFFFF,  //white
    0xFFFFFF00,  //yellow
    0xFF000000   //black
};
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoOpacityMain(mt_s32 argc, mt_char *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_VOID MT_MtgoOpacityExit(SAMPLE_MTGO_SURFACE_INFO_S *surface)
{
    (MT_VOID)MT_GO_FreeSurface(surface->hMemSurface);

    (MT_VOID)MT_GO_DestroyLayer(surface->hLayer);

    (MT_VOID)MT_GO_Deinit();

    memset(surface, 0, sizeof(*surface));

    g_bTaskQuit = MT_TRUE;
}

static MT_VOID MT_MtgoOpacityPrintMenu(MT_VOID)
{
    MTGO_OPTLAYER_PRINT("commond: \n");
    MTGO_OPTLAYER_PRINT("     a: set the transparency\n");
#ifdef MT_SAMPLE_APP
    MTGO_OPTLAYER_PRINT("     b: background run \n");
#endif
    MTGO_OPTLAYER_PRINT("     q: quit \n");
    MTGO_OPTLAYER_PRINT("     h: help \n");
    MTGO_OPTLAYER_PRINT("MTGO>> ");
}

static void MT_MtgoOpacityPrint_usage(MT_CHAR *name)
{

    MTGO_OPTLAYER_PRINT("Lack of parameters\n");
    MTGO_OPTLAYER_PRINT("\nUsage:\n");
    MTGO_OPTLAYER_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    MTGO_OPTLAYER_PRINT("    -q: Exit the background\n");
#endif
    MTGO_OPTLAYER_PRINT("example:\n");
    MTGO_OPTLAYER_PRINT("    %s\n", name);

}

static MT_S32 MT_MtgoOpacityParase_args(MT_S32 argc, MT_CHAR *argv[], SAMPLE_MTGO_SURFACE_INFO_S *pSurface)
{
    MT_S32 opt = 0;
    SAMPLE_MTGO_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHqt:")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                SAMPLE_MTGO_INFO_PRINT("Get opt[%c], run help info.\n", opt);
                (MT_VOID)MT_MtgoOpacityPrint_usage(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoOpacityExit(pSurface);
                }
                return MT_TASK_EXIT;
            default:
                return MT_SUCCESS;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


static MT_VOID MT_MtgoOpacityCmdTask(SAMPLE_MTGO_SURFACE_INFO_S surface)
{
    MT_S8   inputCmd[32];
    MT_U32   alpha = 0;
    MT_S32  s32Ret = MT_FAILURE;

    while(1)
    {
        (MT_VOID)MT_MtgoOpacityPrintMenu();

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);//SAMPLE_GET_INPUTCMD(InputCmd);

        MTGO_OPTLAYER_PRINT(">>> input:%s  \n", inputCmd);

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
            g_bTaskQuit = MT_FALSE;
            break;
        }
        #endif
        else if('a' == inputCmd[0])
        {
            MTGO_OPTLAYER_PRINT("Set the transparency(0-255):");
            scanf("%d", &alpha);
            getchar();

            if(alpha > 255)
            {
                MTGO_OPTLAYER_PRINT("The input is out of range!\n");
                continue;
            }
            /** Sets the region alpha value of the layer's surface, param[in] [1]Layer handle [2]Enable region alpha [3]Alpha region alpha value, region alpha and globle alpha together affect the transparency of display colors */
            s32Ret = MT_GO_SetRegionAlpha(surface.hLayer, 1, (MT_U8)alpha);
            if (MT_SUCCESS != s32Ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetRegionAlpha failed!\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("Print help info \n");
        }
    }
}



#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoOpacityMain(mt_s32 argc, mt_char *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_U8             alpha = 0;
    MT_S32            ret = 0;
    MTGO_LAYER_E      eLayerID = MTGO_LAYER_OSD0;
    MT_RECT           stRect = { 0 };
    MTGO_LAYER_KEY_S  setColorKey = { 0 };
    MTGO_LAYER_KEY_S  getColorKey = { 0 };
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_BLTOPT_S     stBlitOpt = { 0 };
    static SAMPLE_MTGO_SURFACE_INFO_S surface = { 0 };

    ret = MT_MtgoOpacityParase_args(argc, argv, &surface);
    if (MT_FAILURE == ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        /** Display initialization */
        ret = Sample_MTGO_Display_Init(MT_UNF_ENC_FMT_720P_60);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Display init failed!\n", __FUNCTION__, __LINE__);
            return MT_SUCCESS;
        }

        sleep(1);
#endif

        /** MtGo initialization */
        ret = MT_GO_Init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: MtGo init failed!\n", __FUNCTION__, __LINE__);
            goto ERR2;
        }

        /** MT_GO_GetLayerDefaultParam gets the default parameters for the creation of the corresponding layer(SD,HD) based on the layer ID, if you need to use non-default values, you can directly set each member of pLayerInfo, used before createlayer */
        ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetLayerDefaultParam failed!\n", __FUNCTION__, __LINE__);
            goto ERR3;
        }

        /**Use the palette to set the PixelFormat value to MTGO_PF_CLUT8*/
        stLayerInfo.PixelFormat = MTGO_PF_CLUT8;
        stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_DOUBBUFER;

        /** Create layers based on LayerInfo */
        ret = MT_GO_CreateLayer(&stLayerInfo, &surface.hLayer);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: CreateLayer failed!\n", __FUNCTION__, __LINE__);
            goto ERR3;
        }

        /** Set the color palette, param[in] [1]Layer handle [2]Color palette pointer [3]The start position of the color palette [4]The number of palettes starting with start */
        ret = MT_GO_SetLayerPalette(surface.hLayer, g_rgb_clut_palette, 0, 6);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetLayerPalette failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }

        /** Get the layer surface */
        ret = MT_GO_GetLayerSurface(surface.hLayer, &surface.hLayerSurface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetLayerSurface failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }

        /** Set colors according to the palette index, 0 is blue on the palette, param[stRect] is the surface size and positio */
        stRect.x = 0;
        stRect.y = 0;
        stRect.w = 1280;
        stRect.h = 720;
        ret = MT_GO_FillRect(surface.hLayerSurface, &stRect, 0, MTGO_COMPOPT_NONE);

        /** 3 is white on the palette */
        stRect.x = 200;
        stRect.y = 150;
        stRect.w = 50;
        stRect.h = 50;
        ret = MT_GO_FillRect(surface.hLayerSurface, &stRect, 3, MTGO_COMPOPT_NONE);

        /** 2 is red on the palette */
        stRect.x = 250;
        stRect.y = 200;
        stRect.w = 50;
        stRect.h = 50;
        ret = MT_GO_FillRect(surface.hLayerSurface, &stRect, 2, MTGO_COMPOPT_NONE);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: FillRect failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        /** Sets the global alpha value of the layer's surface, affects the color brightness of the final display of the screen, 0 is transparent and 255 is the brightest, region alpha and globle alpha together affect the transparency of display colors */
        ret = MT_GO_SetLayerAlpha(surface.hLayer, 255);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetLayerAlpha failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        /** Gets the global alpha value of the layer's surface, the obtained value is saved to [pAlpha] */
        ret = MT_GO_GetLayerAlpha(surface.hLayer, &alpha);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetLayerAlphas failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }
        SAMPLE_MTGO_INFO_PRINT("<%s>line [%d]: get layer alpha: %d\n", __FUNCTION__, __LINE__, alpha);

        /** Sets the region alpha value of the layer's surface, param[in] [1]Layer handle [2]Enable region alpha [3]Alpha region alpha value, region alpha and globle alpha together affect the transparency of display colors */
        ret = MT_GO_SetRegionAlpha(surface.hLayer, 1, 50);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetRegionAlpha failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }

        /** Gets the region alpha value of the layer's surface, the obtained value is saved to [pAlpha] */
        ret = MT_GO_GetRegionAlpha(surface.hLayer, &alpha);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetRegionAlpha failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }
        SAMPLE_MTGO_INFO_PRINT("<%s>line [%d]: get region alpha: %d\n", __FUNCTION__, __LINE__, alpha);

        /** Set 0xffffff00 to Colorkey, and the 0xffffff00 on the Layer will not be displayed */
        setColorKey.ColorKey = 0xffffff00;
        setColorKey.bEnableCK = MT_TRUE;
        ret = MT_GO_SetLayerColorkey(surface.hLayer, &setColorKey);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetLayerColorkey failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }

        /** Get the colorkey and save the value in getColokey */
        ret = MT_GO_GetLayerColorkey(surface.hLayer, &getColorKey);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetLayerColorkey failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }
        SAMPLE_MTGO_INFO_PRINT("<%s>line [%d]: get LayerColorKey: 0x%x\n", __FUNCTION__, __LINE__, getColorKey.ColorKey);

        /** Create a Surface for drawing graphics, param[in] [1]The width of the surface [2]The height of the surface [3]Pixel format [4]Surface handle pointer */
        ret = MT_GO_CreateSurface(200, 150, MTGO_PF_CLUT8, &surface.hMemSurface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: CreateSurface failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }

        /** Set the alpha value of the Surface */
        ret = MT_GO_SetSurfaceAlpha(surface.hMemSurface,255);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetSurfaceAlpha failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        /** Set the rectangle color to green, 1 is green on the palette, param[stRect] is the surface size and position */
        stRect.x = 0;
        stRect.y = 0;
        stRect.w = 200;
        stRect.h = 150;
        ret = MT_GO_DrawRect(surface.hMemSurface, &stRect, 1);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: DrawFillRect failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        /** 4 is yellow on the palette */
        stRect.x = 50;
        stRect.y = 50;
        stRect.w = 100;
        stRect.h = 50;
        ret = MT_GO_DrawRect(surface.hMemSurface, &stRect, 4);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: DrawFillRect failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        memset(&stBlitOpt,0,sizeof(MTGO_BLTOPT_S));
        if(stLayerInfo.PixelFormat == MTGO_PF_CLUT8)
        {
            stBlitOpt.EnableGlobalAlpha = MT_FALSE;

            /** should set the pixel alpha mix mode when enable global alpha */
            stBlitOpt.PixelAlphaComp = MTGO_COMPOPT_NONE;
        }
        else
        {
            stBlitOpt.EnableGlobalAlpha = MT_TRUE;

            /** should set the pixel alpha mix mode when enable global alpha */
            stBlitOpt.PixelAlphaComp = MTGO_COMPOPT_SRCOVER;
        }

        stRect.x = 0;
        stRect.y = 0;
        stRect.w = 200;
        stRect.h = 150;
        ret = MT_GO_Blit(surface.hMemSurface, NULL, surface.hLayerSurface, &stRect, &stBlitOpt);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Blit failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        /** Refresh the layer, param[2]refreshed rectangular area, NULL is full screen */
        ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: RefreshLayer failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }
        g_bTaskQuit = MT_FALSE;
    }

    (MT_VOID)MT_MtgoOpacityCmdTask(surface);

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR5:
    (MT_VOID)MT_GO_FreeSurface(surface.hMemSurface);
ERR4:
    (MT_VOID)MT_GO_DestroyLayer(surface.hLayer);
ERR3:
    (MT_VOID)MT_GO_Deinit();
ERR2:
#ifndef MT_SAMPLE_APP
    (MT_VOID)Sample_MTGO_Display_DeInit();
#endif
    return MT_SUCCESS;
}

