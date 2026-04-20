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

#define MTGO_DRAW_PRINT        printf
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

/*************************** Structure Definition ****************************/
typedef struct
{
    MT_U32 x; /* frequency kHz */
    MT_U32 y;
    MT_S32 color;
    MT_U32 width;
    MT_U32 height;
    MT_U32 radius;
} mt_input_draw_para_t;

typedef struct
{
    mt_handle hLayer;
    mt_handle hLayerSurface;
    mt_handle hMemSurface;
}SAMPLE_MTGO_SURFACE_INFO_S;
/************************ Global Variable declaration ************************/
static MT_BOOL g_bTaskQuit = MT_TRUE;
/****************************** API declaration ******************************/
#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoDrawMain(mt_s32 argc, mt_char *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_VOID MT_MtgoDrowExit(SAMPLE_MTGO_SURFACE_INFO_S *surface)
{
    (MT_VOID)MT_GO_FreeSurface(surface->hMemSurface);

    (MT_VOID)MT_GO_DestroyLayer(surface->hLayer);

    (MT_VOID)MT_GO_Deinit();

    memset(surface, 0, sizeof(*surface));

    g_bTaskQuit = MT_TRUE;
}

/*!
@brief Draws a circle. CNcomment:绘制圆 CNend
@param[in] x             the position of the center x on the surface. CNcomment:圆心x在surface上的位置 CNend
@param[in] y             the position of the center y on the surface. CNcomment:圆心y在surface上的位置 CNend
@param[in] radius        circle radius. CNcomment:圆半径 CNend
@param[in] setColor      circle color.CNcomment:圆颜色 CNend
@param[in] Surface       destination surface handle.CNcomment:目的surface句柄 CNend
@return ::MT_SUCCESS
@return ::MT_FALSE
@*/
static MT_S32 MT_DrawModeDrawCircle(MT_S32 x, MT_S32 y, MT_S32 radius, MT_COLOR setColor, MT_HANDLE surface)
{
    MT_S32 tx = 0;
    MT_S32 ty = radius;
    MT_S32 d = 3 - 2 * radius;
    MT_S32 ret = MT_FALSE;
    while(tx < ty)
    {
        /** Less than a 45-degree horizontal line */
        ret = MT_GO_DrawLine(surface, x - ty, y - tx, x + ty, y - tx, setColor);
        if(MT_SUCCESS != ret)
        {
            return MT_FALSE;
        }
        /** Prevent horizontal lines from being drawn repeatedly*/
        if(tx != 0)
        {
            ret = MT_GO_DrawLine(surface, x - ty, y + tx, x + ty, y + tx, setColor);
            if(MT_SUCCESS != ret)
            {
                return MT_FALSE;
            }
        }
        /** Take the dots above */
        if(d < 0)
        {
            d += 4 * tx + 6;
        }
        /** Take the dots below */
        else
        {
            /** Greater than the 45-degree horizontal line */
            ret = MT_GO_DrawLine(surface, x - tx, y - ty, x + tx, y - ty, setColor);
            ret &= MT_GO_DrawLine(surface, x - tx, y + ty, x + tx, y + ty, setColor);
            if(MT_SUCCESS != ret)
            {
                return MT_FALSE;
            }
            d += 4 * (tx - ty) + 10;
            ty--;
        }
        tx++;
    }
    /** 45degree horizontal line*/
    if(tx == ty)
    {
        ret = MT_GO_DrawLine(surface, x - ty, y - tx, x + ty, y - tx, setColor);
        ret &= MT_GO_DrawLine(surface, x - ty, y + tx, x + ty, y + tx, setColor);
        if(MT_SUCCESS != ret)
        {
            return MT_FALSE;
        }
    }
    return MT_SUCCESS;
}

static MT_VOID MT_DrawPrintMenu(MT_VOID)
{
    MTGO_DRAW_PRINT("commond: \n");
    MTGO_DRAW_PRINT("     l: x,y position\n");
    MTGO_DRAW_PRINT("     c: color, 0xff00ff00 is green, 0xffff0000 is red\n");
    MTGO_DRAW_PRINT("     d: rectangle width and height\n");
    MTGO_DRAW_PRINT("     r: radius\n");
    MTGO_DRAW_PRINT("     y: draw the circle\n");
    MTGO_DRAW_PRINT("     s: draw the rectangle\n");
    MTGO_DRAW_PRINT("     k: clear \n");
#ifdef MT_SAMPLE_APP
    MTGO_DRAW_PRINT("     b : background run \n");
#endif
    MTGO_DRAW_PRINT("     h: help \n");
    MTGO_DRAW_PRINT("     q: quit \n");
    MTGO_DRAW_PRINT("MTGO>> ");
}

static MT_VOID MT_DrawModeCmdTask(SAMPLE_MTGO_SURFACE_INFO_S surface)
{
    MT_S32            ret = MT_FAILURE;
    MT_S32            cmd = 0;
    MTGO_BLTOPT_S     stBlitOpt = { 0 };
    MT_RECT           stRect = { 0 };
    MT_RECT           stBlitRect = { 0 };
    MT_CHAR           *fgetret=NULL;
    MT_S8             inputCmd[32];
    mt_input_draw_para_t param = { 320, 160, 0xff00ff00, 160, 160, 160 };

    memset(&stBlitOpt,0,sizeof(MTGO_BLTOPT_S));
    stBlitOpt.EnableGlobalAlpha = MT_TRUE;
    /** should set the pixel alpha mix mode when enable global alpha */
    stBlitOpt.PixelAlphaComp = MTGO_COMPOPT_SRCOVER;
    /** Blit operation, move the source Surface to the area on the destination Surface(regions are customizable), param[in] [1]Source surface handle [2]Source surface size and position [3]Destination surface handle [4]Destination surface size and position [5]Blending operation */
    stBlitRect.x = 0;
    stBlitRect.y = 0;
    stBlitRect.w = 1280;
    stBlitRect.h = 720;
    ret = MT_GO_Blit(surface.hMemSurface, NULL, surface.hLayerSurface, &stBlitRect, &stBlitOpt);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Blit failed!\n", __FUNCTION__, __LINE__);
    }

    /** Refresh the layer, param[2]refreshed rectangular area, NULL is full screen */
    ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: RefreshLayer failed!\n", __FUNCTION__, __LINE__);
    }

    while (1)
    {
        (MT_VOID)MT_DrawPrintMenu();

        fgetret=fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);//SAMPLE_GET_INPUTCMD(InputCmd);
        fgetret=fgetret;

        SAMPLE_MTGO_INFO_PRINT(">>> input: :%s  \n", inputCmd);

        if('q' == inputCmd[0])
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
        else if('l' == inputCmd[0])
        {
            MTGO_DRAW_PRINT("x>> ");
            scanf("%d", &cmd);
            param.x = cmd;
            MTGO_DRAW_PRINT("y>> ");
            scanf("%d", &cmd);
            param.y = cmd;
        }
        else if('c' == inputCmd[0])
        {
            MTGO_DRAW_PRINT("color>> ");
            scanf("%x", &cmd);
            param.color = cmd;
        }
        else if('d' == inputCmd[0])
        {
            MTGO_DRAW_PRINT("height>> ");
            scanf("%d", &cmd);
            param.height = cmd;
            MTGO_DRAW_PRINT("width>> ");
            scanf("%d", &cmd);
            param.width = cmd;

            printf("w: %d\n", param.width);
            printf("h: %d\n", param.height);
        }
        else if('r' == inputCmd[0])
        {
            MTGO_DRAW_PRINT("radius>> ");
            scanf("%d", &cmd);
            param.radius = cmd;
        }
        else if('y' == inputCmd[0])
        {
            /** Draw a circle, param[in] [1]center x [2]center y [3]radius [4]color [5]destination surface handle, 0xff00ff00 is green */
            ret = MT_DrawModeDrawCircle(param.x, param.y, param.radius, param.color, surface.hMemSurface);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Draw circle failed!\n", __FUNCTION__, __LINE__);
                break;
            }

            /** Blit operation, move the source Surface to the area on the destination Surface(regions are customizable), param[in] [1]Source surface handle [2]Source surface size and position [3]Destination surface handle [4]Destination surface size and position [5]Blending operation */
            ret = MT_GO_Blit(surface.hMemSurface, NULL, surface.hLayerSurface, &stBlitRect, &stBlitOpt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Blit failed!\n", __FUNCTION__, __LINE__);
                break;
            }

            /** Refresh the layer, param[2]refreshed rectangular area, NULL is full screen */
            ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: RefreshLayer failed!\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        else if('s' == inputCmd[0])
        {
            stRect.x = param.x;
            stRect.y = param.y;
            stRect.w = param.width;
            stRect.h = param.height;

            ret = MT_GO_DrawRect(surface.hMemSurface, &stRect, param.color);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Draw rectangle failed!\n", __FUNCTION__, __LINE__);
                break;
            }

            /** Blit operation, move the source Surface to the area on the destination Surface(regions are customizable), param[in] [1]Source surface handle [2]Source surface size and position [3]Destination surface handle [4]Destination surface size and position [5]Blending operation */
            ret = MT_GO_Blit(surface.hMemSurface, NULL, surface.hLayerSurface, &stBlitRect, &stBlitOpt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Blit failed!\n", __FUNCTION__, __LINE__);
                break;
            }

            /** Refresh the layer, param[2]refreshed rectangular area, NULL is full screen */
            ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: RefreshLayer failed!\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        else if('k' == inputCmd[0])
        {
            stRect.x = 0;
            stRect.y = 0;
            stRect.w = 1280;
            stRect.h = 720;
            ret = MT_GO_DrawRect(surface.hMemSurface, &stRect, 0x55ffffff);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Draw rectangle failed!\n", __FUNCTION__, __LINE__);
                break;
            }

            /** Blit operation, move the source Surface to the area on the destination Surface(regions are customizable), param[in] [1]Source surface handle [2]Source surface size and position [3]Destination surface handle [4]Destination surface size and position [5]Blending operation */
            ret = MT_GO_Blit(surface.hMemSurface, NULL, surface.hLayerSurface, &stBlitRect, &stBlitOpt);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Blit failed!\n", __FUNCTION__, __LINE__);
                break;
            }

            /** Refresh the layer, param[2]refreshed rectangular area, NULL is full screen */
            ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: RefreshLayer failed!\n", __FUNCTION__, __LINE__);
                break;
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("Print help info \n");
        }
    }
}


static void MT_MtgoDrawPrint_usage(MT_CHAR *name)
{
    MTGO_DRAW_PRINT("Lack of parameters\n");
    MTGO_DRAW_PRINT("\nUsage:\n");
    MTGO_DRAW_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    MTGO_DRAW_PRINT("    -q: Exit the background\n");
#endif
    MTGO_DRAW_PRINT("example:\n");
    MTGO_DRAW_PRINT("    %s\n", name);
}


static MT_S32 MT_MtgoDrawParase_args(MT_S32 argc, MT_CHAR *argv[], SAMPLE_MTGO_SURFACE_INFO_S *pSurface)
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
                (MT_VOID)MT_MtgoDrawPrint_usage(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoDrowExit(pSurface);
                }
                return MT_TASK_EXIT;
            default:
                return MT_SUCCESS;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
mt_s32 MT_MtgoDrawMain(mt_s32 argc, mt_char *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32            ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MT_RECT           stRect = { 0 };
    MTGO_LAYER_E      eLayerID = MTGO_LAYER_OSD0;
    static SAMPLE_MTGO_SURFACE_INFO_S surface = { 0 };

    ret = MT_MtgoDrawParase_args(argc, argv, &surface);
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
    /************************END*****************************/


    /*************Create Layer and LayerSurface**************/
        /** MT_GO_GetLayerDefaultParam gets the default parameters for the creation of the corresponding layer(SD,HD) based on the layer ID, if you need to use non-default values, you can directly set each member of pLayerInfo, used before createlayer */
        ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetLayerDefaultParam failed!\n", __FUNCTION__, __LINE__);
            goto ERR3;
        }

        /** Use the palette to set the PixelFormat value to MTGO_PF_8888 */
        stLayerInfo.PixelFormat = MTGO_PF_8888;
        stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_DOUBBUFER;

        /** Create layers based on LayerInfo */
        ret = MT_GO_CreateLayer(&stLayerInfo, &surface.hLayer);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: CreateLayer failed!\n", __FUNCTION__, __LINE__);
            goto ERR3;
        }

        /** Get the layer surface */
        ret = MT_GO_GetLayerSurface(surface.hLayer, &surface.hLayerSurface);
        if(MT_SUCCESS != ret)
        {
           SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: GetLayerSurface failed!\n", __FUNCTION__, __LINE__);
           goto ERR4;
        }

        /** Set the surface color to blue, 0xff0000ff is blue, param[stRect] is the surface size and positio */
        stRect.x = 0;
        stRect.y = 0;
        stRect.w = 1280;
        stRect.h = 720;
        ret = MT_GO_FillRect(surface.hLayerSurface, &stRect, 0x55ffffff, MTGO_COMPOPT_NONE);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: FillRect failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }
    /************************END*****************************/


    /********************Draw a graph************************/
        /** Create a Surface for drawing graphics, param[in] [1]The width of the surface [2]The height of the surface [3]Pixel format [4]Surface handle pointer */
        ret = MT_GO_CreateSurface(1280, 720, MTGO_PF_8888, &surface.hMemSurface);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: CreateSurface failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }
        /** Set the alpha value of the Surface */
        ret = MT_GO_SetSurfaceAlpha(surface.hMemSurface,255);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetSurfaceAlpha failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        ret = MT_GO_SetRegionAlpha(surface.hLayer, 1, 100);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: SetRegionAlpha failed!\n", __FUNCTION__, __LINE__);
            goto ERR4;
        }

        /** Set the rectangle color to red, 0xffff0000 is red, param[stRect] is the surface size and position */
        stRect.x = 160;
        stRect.y = 160;
        stRect.w = 320;
        stRect.h = 320;
        ret = MT_GO_DrawRect(surface.hMemSurface, &stRect, 0xffff0000);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Draw rectangle failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }

        /** Draw a circle, param[in] [1]center x [2]center y [3]radius [4]color [5]destination surface handle, 0xff00ff00 is green */
        ret = MT_DrawModeDrawCircle(960, 320, 170, 0xff00ff00, surface.hMemSurface);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("<%s>line [%d]: Draw circle failed!\n", __FUNCTION__, __LINE__);
            goto ERR5;
        }
        g_bTaskQuit = MT_FALSE;
    }
/*************************END****************************/


/*****************Display to screen**********************/


    /** Enter characters to exit the program */
    (MT_VOID)MT_DrawModeCmdTask(surface);
/************************END*****************************/
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
    return ret;
}


