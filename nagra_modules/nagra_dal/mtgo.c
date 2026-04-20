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

#define MT_MTGO_DEBUG
#ifdef MT_MTGO_DEBUG 
#define MTGO_PRINT   printf
#else
#define MTGO_PRINT 
#endif

/***************************** Macro Definition ******************************/

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
static SAMPLE_MTGO_SURFACE_INFO_S  surface;

/******************************* API declaration *****************************/


static MT_VOID MT_MtgoTextExit(mt_handle hLayer, mt_handle hFont)
{

        MT_GO_DestroyText(hFont);
        MT_GO_DestroyLayer(hLayer);
        MT_GO_DeinitText();
        MT_GO_Deinit();
}


static MT_S32 MT_MtgoTextInit(mt_handle *phLayer, mt_handle *phSurface, mt_handle *phFont)
{
    mt_s32 s32Ret = MT_SUCCESS;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    mt_handle hSurface = MTGO_INVALID_HANDLE;
    mt_handle hFont = MTGO_INVALID_HANDLE;
    mt_handle hLayer = MTGO_INVALID_HANDLE;

    s32Ret = MT_GO_Init();
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_Init error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    s32Ret = MT_GO_GetLayerDefaultParam(MTGO_LAYER_OSD0, &stLayerInfo);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_GetLayerDefaultParam error. ret=0x%x \n", s32Ret);
        goto ERR1;
    }

    stLayerInfo.PixelFormat = MTGO_PF_8888;
    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
    s32Ret = MT_GO_CreateLayer(&stLayerInfo, &hLayer);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_CreateLayer error. ret=0x%x \n", s32Ret);
        goto ERR1;
    }

    s32Ret = MT_GO_GetLayerSurface(hLayer, &hSurface);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", s32Ret);
        goto ERR2;
    }

    s32Ret = MT_GO_InitText();
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_InitText error. ret=0x%x \n", s32Ret);
        goto ERR2;
    }

    s32Ret = MT_GO_CreateText("/usr/local/stb/res/DroidSansFallbackLegacy.ttf", &hFont);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_CreateText error. ret=0x%x \n", s32Ret);
        goto ERR3;
    }
    *phLayer = hLayer;
    *phSurface = hSurface;
    *phFont = hFont;

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

    phSurface->color = 0;
    s32Ret = MT_GO_FillRect(phSurface->hSurface, NULL, phSurface->color, MTGO_COMPOPT_NONE);  //set background color is write(0xffffffff)
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_FillRect error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_s32 MT_MtgoTextDrawText(mt_handle hSurface, SAMPLE_MTGO_TEXT_INFO_S *pTextInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;

    s32Ret = MT_GO_SetTextColor(pTextInfo->hFont, pTextInfo->textColor);  //set text color is red(0xffff0000)
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_SetTextColor error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    s32Ret = MT_GO_SetTextStyle(pTextInfo->hFont, pTextInfo->style);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_SetTextStyle error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }


    s32Ret = MT_GO_DrawRect(hSurface, &pTextInfo->rect, pTextInfo->bgColor);  //set text ground area and color green(0xff00ff00)
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_DrawRect error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    s32Ret = MT_GO_TextOutEx(pTextInfo->hFont, hSurface, pTextInfo->szText, &pTextInfo->rect, pTextInfo->layout);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_TextOutEx error. ret=0x%x \n", s32Ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

MT_S32 MT_DrawInit()
{
    mt_s32 s32Ret = MT_SUCCESS;	
	memset(&surface, 0, sizeof(surface));
	s32Ret = MT_MtgoTextInit(&surface.hLayer, &surface.hSurface, &surface.stext.hFont);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR1;
	}
	s32Ret = MT_MtgoTextDrawSurface(&surface);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR2;
	}
	return MT_SUCCESS;
ERR2:
	MT_MtgoTextExit(surface.hLayer, surface.stext.hFont);
	memset(&surface, 0, sizeof(surface));
ERR1:	
	return MT_FAILURE;

}
#if 0
MT_S32 MT_DrawText(mt_char *inputCmd,mt_s32 x, mt_s32 y, mt_s32 w, mt_s32 h)
{
    mt_s32 s32Ret = MT_SUCCESS;	
	memset(&surface, 0, sizeof(surface));
	s32Ret = MT_MtgoTextInit(&surface.hLayer, &surface.hSurface, &surface.stext.hFont);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR1;
	}
	s32Ret = MT_MtgoTextDrawSurface(&surface);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR2;
	}
	memcpy(surface.stext.szText,  inputCmd, 128);
	surface.stext.textColor = 0xffff0000;
	surface.stext.rect.x = x;//200;
	surface.stext.rect.y = y;//100;
	surface.stext.rect.w = w;//800;
	surface.stext.rect.h = h;//60;
	surface.stext.bgColor = 0xff00ff00;//0xFF404040;//0xff00ff00;
	surface.stext.style = MTGO_TEXT_STYLE_NORMAL;//MTGO_TEXT_STYLE_BOLD;
	surface.stext.layout = MTGO_LAYOUT_LEFT;//MTGO_LAYOUT_RIGHT;
	
	s32Ret = MT_MtgoTextDrawText(surface.hSurface, &surface.stext);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR2;
	}
    s32Ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);		
		goto ERR2;
    }
	
    return MT_SUCCESS;
ERR2:
	MT_MtgoTextExit(surface.hLayer, surface.stext.hFont);
	memset(&surface, 0, sizeof(surface));
ERR1:	
    return MT_FAILURE;

}
#else
MT_S32 MT_DrawText(mt_char *inputCmd,mt_s32 x, mt_s32 y, mt_s32 w, mt_s32 h)
{
    mt_s32 s32Ret = MT_SUCCESS;	
	memcpy(surface.stext.szText,  inputCmd, 128);
	surface.stext.textColor = 0xFFFFFFFF;
	surface.stext.rect.x = x;//200;
	surface.stext.rect.y = y;//100;
	surface.stext.rect.w = w;//800;
	surface.stext.rect.h = h;//60;
	surface.stext.bgColor = 0xFF0000FF;
	surface.stext.style = MTGO_TEXT_STYLE_NORMAL;//MTGO_TEXT_STYLE_BOLD;
	surface.stext.layout = MTGO_LAYOUT_LEFT;//MTGO_LAYOUT_RIGHT;
	
	s32Ret = MT_MtgoTextDrawText(surface.hSurface, &surface.stext);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR1;
	}
    s32Ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);		
		goto ERR1;
    }
	
    return MT_SUCCESS;
ERR1:	
    return MT_FAILURE;

}

MT_S32 MT_DrawText_Ext(mt_char *inputCmd,mt_s32 x, mt_s32 y, mt_s32 w, mt_s32 h, mt_u32 txt_color, mt_u32 bg_color)
{
    mt_s32 s32Ret = MT_SUCCESS;	
	memcpy(surface.stext.szText,  inputCmd, 128);
	surface.stext.textColor = txt_color;
	surface.stext.rect.x = x;//200;
	surface.stext.rect.y = y;//100;
	surface.stext.rect.w = w;//800;
	surface.stext.rect.h = h;//60;
	surface.stext.bgColor = bg_color;//0xFF404040;//0xff00ff00;
	surface.stext.style = MTGO_TEXT_STYLE_NORMAL;//MTGO_TEXT_STYLE_BOLD;
	surface.stext.layout = MTGO_LAYOUT_LEFT;//MTGO_LAYOUT_RIGHT;
	
	s32Ret = MT_MtgoTextDrawText(surface.hSurface, &surface.stext);
	if (MT_FAILURE == s32Ret)
	{
		MTGO_PRINT("MT_MtgoTextInit err. ret= 0x%x.\n", s32Ret);
		goto ERR1;
	}
    s32Ret = MT_GO_RefreshLayer(surface.hLayer, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        MTGO_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", s32Ret);		
		goto ERR1;
    }
	
    return MT_SUCCESS;
ERR1:	
    return MT_FAILURE;

}


#endif
void MT_DrawExit()
{
	if(surface.hLayer != NULL){
		MT_MtgoTextExit(surface.hLayer, surface.stext.hFont);	
		memset(&surface, 0, sizeof(surface));
	}
	return;
}
