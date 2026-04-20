/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "pthread.h"

#include "mt_go.h"
#include "ui_manager.h"
#include "demo.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static menu_attr_t all_menu_attr[] =
{
	{ROOT_ID_BASIC, 	open_basic},
	{ROOT_ID_AVPLAY, 	open_avplay},
	{ROOT_ID_DVBC, 		open_dvbc},
	{ROOT_ID_DVBS, 		open_dvbs},
	{ROOT_ID_NET, 		open_net},
	{ROOT_ID_FRONT, 	open_front}
};

mt_handle g_layer;
mt_handle g_layerSurface;
mt_handle g_font;

static pthread_mutex_t ui_mutex = PTHREAD_MUTEX_INITIALIZER;


struct MainWin  G_MAIN_WIN = {0};
struct SubWin  G_SubWin[MAX_MAIN_WIN_COUNT] = {0};

mt_s32 add_main_win(mt_char *pText, mt_s32 type, MT_RECT rc, mt_s32 (*callback_event)(mt_u16 evt_id, mt_u32 para))
{
	struct MainWin *p  = &G_MAIN_WIN;

	if(p->cnt >= MAX_MAIN_WIN_COUNT)
		return -1;

	memcpy(p->wc[p->cnt].p_dis_text, pText, strlen(pText));
	p->wc[p->cnt].type = type;
	p->wc[p->cnt].rc = rc;
	p->wc[p->cnt].callback_event = callback_event;
	p->cnt ++;
	p->m_cursor = 0;	
//	printf("main win cnt %d\n",p->cnt);
	return 0;
}

mt_s32 add_sub_win_cmd(struct SubCmd *sc)
{

    struct SubWin *p  = &G_SubWin[sc->m_cursor];

//	printf("%s(line: %d), add sub cmd %d\n", __FUNCTION__, __LINE__, p->total_cnt);
    if(p->total_cnt >= MAX_SUB_WIN_COUNT)
          return -1;

    memcpy(&p->sc[p->total_cnt],sc,sizeof(struct SubCmd));
    p->total_cnt++;
	if(sc->is_noncursor == FALSE)
		p->cursor_cnt++;
    p->s_cursor = 0xff;

    
//    printf("sub win cnt %d\n",p->cnt);
    return 0;
}

mt_s32 get_main_win_index(mt_s32 type)
{
	int i = 0 ;
	struct MainWin *p  = &G_MAIN_WIN;
	
	for(i = 0 ; i < p->cnt ; i++)
	{
		if(p->wc[i].type == type)
			return i;
	}

	return -1;
}

mt_s32 manage_surface_init(void)
{
	mt_s32 ret;
	MTGO_LAYER_INFO_S stLayerInfo = {0};
	MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;
	MT_RECT rc = {0};
	mt_char title[] = "Factory Test";
	
	ret = MT_GO_Init();
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_Init failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR0;
    }

    ret = MT_GO_InitText();
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_InitText failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR1;
    }

    ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_GetLayerDefaultParam failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR2;
    }

	printf("%s(line: %d), screen width %d, height %d, display width %d, height %d\n", __FUNCTION__, __LINE__, stLayerInfo.ScreenWidth, 
		stLayerInfo.ScreenHeight, stLayerInfo.DisplayWidth, stLayerInfo.DisplayHeight);

    stLayerInfo.PixelFormat = MTGO_PF_8888;
    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
    ret = MT_GO_CreateLayer(&stLayerInfo, &g_layer);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_CreateLayer failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR2;
    }

    ret = MT_GO_GetLayerSurface(g_layer, &g_layerSurface);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_GetLayerSurface failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR3;
    }

    ret = MT_GO_FillRect(g_layerSurface, NULL, 0xFF1E90FF, MTGO_COMPOPT_NONE);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_FillRect failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR3;
    }

	ret = MT_GO_CreateText("./res/DroidSansFallbackLegacy.ttf", &g_font);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_CreateText failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR3;
    }

	rc.x = 200;
    rc.y = 20;
    rc.w = 1020;
    rc.h = 80;
	
	ret = MT_GO_SetPixelSize(g_font, 35, 35);
	
	ret = MT_GO_SetTextColor(g_font, 0xFFFFFFFF);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_SetTextColor failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR4;
    }
//	MT_GO_DrawRect(g_layer, &rc, 0xff000000);
    ret = MT_GO_TextOutEx(g_font, g_layerSurface, title, &rc, MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_VCENTER);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_TextOutEx failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR4;
    }

    ret = MT_GO_RefreshLayer(g_layer, NULL);
    if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), MT_GO_RefreshLayer failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		goto ERR4;
    }

	goto ERR0;

ERR4:
	MT_GO_DestroyText(g_font);
ERR3:
	MT_GO_DestroyLayer(g_layer);
ERR2:
	MT_GO_DeinitText();
ERR1:
	MT_GO_Deinit();
ERR0:
	return ret;

}

void manage_menu_init(void)
{
	mt_u16 i = 0;
	printf("%s(line: %d), arry size %d.\n", __FUNCTION__, __LINE__, ARRAY_SIZE(all_menu_attr));
#if 0	
	memset(&G_MAIN_WIN, 0, sizeof(G_MAIN_WIN));
	memset(G_SubWin, 0, sizeof(G_SubWin));
#endif	
	for(i = 0; i < ARRAY_SIZE(all_menu_attr); i++)
	{
		if(all_menu_attr[i].open_func)
		{
			pthread_mutex_lock(&ui_mutex);
			all_menu_attr[i].open_func(0, 0);
			pthread_mutex_unlock(&ui_mutex);
		}
	}
//	G_MAIN_WIN.m_cursor = 1;
}


OPEN_MENU menu_get_open_func(u16 root_id)
{
	mt_u16 i = 0;

	for(i = 0; i < ARRAY_SIZE(all_menu_attr); i++)
	{
		if(all_menu_attr[i].root_id == root_id)
		{
			break;
		}
	}

	if(i >= ARRAY_SIZE(all_menu_attr))
		return NULL;

	return all_menu_attr[i].open_func;
}


mt_s32 manage_open_menu(mt_u16 root_id, mt_u32 para1, mt_u32 para2)
{
	OPEN_MENU   open_func = NULL;
	mt_s32 ret;

	open_func = menu_get_open_func(root_id);
	if((open_func == NULL))
	{
		return MT_FAILURE;
	}
	pthread_mutex_lock(&ui_mutex);
	ret = (*open_func)(para1, para2);
	pthread_mutex_unlock(&ui_mutex);
}

mt_s32 manage_set_container(MTGO_LAYER_E      eLayerID, const mt_char *pText, mt_u8 u32Size, MT_RECT rect, MT_COLOR Color, MT_COLOR BgColor, mt_u32 Style)
{
	mt_handle hFont = MTGO_INVALID_HANDLE;
	MT_HANDLE layer_surface = 0;
	MT_RECT rc;

#if 0
	if(eLayerID == MTGO_LAYER_SUB)
	{
		layer_surface = g_sub_LayerSurface;
		hLayer = g_sub_hLayer;
	}
	if(eLayerID == MTGO_LAYER_OSD0)
	{
		layer_surface = g_osd0_LayerSurface;
		hLayer = g_osd0_hLayer;
	}
	if(eLayerID == MTGO_LAYER_OSD1)
	{
		layer_surface = g_osd1_LayerSurface;
		hLayer = g_osd1_hLayer;
	}

	// textLen = strlen(pText);
	//	if(textLen >= 100)
	{
	//	textLen = 100;
	}

	//	memcpy(szText, pText, textLen);
	//  printf(" *****%d**********  \n",__LINE__);
#endif
	layer_surface = g_layerSurface;
	hFont = g_font;
	rc = rect;
	if(hFont == MTGO_INVALID_HANDLE)
	{
		printf("%s(line: %d), err: parse font err!!! %d  \n", __FUNCTION__, __LINE__, hFont);
		return 0;
	}
	if(strlen(pText))
	{
		MT_GO_SetPixelSize(hFont, u32Size, u32Size);
		MT_GO_SetTextColor(hFont, Color);
//		printf("%s(line: %d), draw rec\n", __FUNCTION__, __LINE__);
		MT_GO_DrawRect(layer_surface, &rc, BgColor);
//		printf("%s(line: %d), set text %s\n", __FUNCTION__, __LINE__, pText);
		MT_GO_TextOutEx(hFont, layer_surface, pText, &rc, Style);
	}
	else
	{
		MT_GO_DrawRect(layer_surface, &rc, BgColor);
	}
	

	return 0;

}


void manage_update_ui(mt_u8 mode, mt_u32 module)
{
	MT_RECT rect;
	int i ;

	mt_u32 color = 0xFF1E90FF;
	mt_u32 text_color = 0xFFFFFFFF;
	mt_u8 size;

	
	struct MainWin *p  = &G_MAIN_WIN;
//	p->m_cursor = 4;
	struct SubWin *pp  = &G_SubWin[G_MAIN_WIN.m_cursor];

	//   printf("---update_ui-----%x -------\n",pp);
	// printf(" p->m_cursor %x \n  pp->s_cursor %x \n",p->m_cursor,pp->s_cursor);
	
	int cur_m = p->m_cursor;
	// printf(" [MAIN] count %d p->m_cursor %x   sub index %x \n",p->cnt,p->m_cursor,pp->s_cursor);

	if(mode == UPDATE_MODULE && module != p->wc[module].type)
		return ;
	
	manage_clear_layer();
	for(i = 0; i < p->cnt; i++)
	{
		if(i == cur_m)
			color = 0xFF009ACD;
		else
			color = 0xFF1E90FF;

		size = 30;
//		printf("%s(line: %d), main win %d\n", __FUNCTION__, __LINE__, i);
		pthread_mutex_lock(&ui_mutex);
		manage_set_container(MTGO_LAYER_OSD0, p->wc[i].p_dis_text, size, p->wc[i].rc, text_color, color, MTGO_LAYOUT_HCENTER | MTGO_LAYOUT_VCENTER);
		pthread_mutex_unlock(&ui_mutex);
	}

	//printf(" ***p->type[cur_m]**%d*****MENU_PICTURE %x****MENU_MEDIA %x*  \n",p->type[cur_m],MENU_PICTURE,MENU_MEDIA);

//	  printf(" *****%d**********  \n",__LINE__);
	//pp->cnt = 0;
	for(i = 0 ; i < pp->total_cnt; i++)
	{
		struct SubCmd *sc = &pp->sc[i];

		if(sc->is_usr_define == TRUE)
			color = sc->usr_color;
		else if(i == pp->s_cursor && sc->is_noncursor == FALSE)
			color = 0xFF009ACD;
		else
			color = 0xFF1E90FF;
#if 0
		if(sc->callback_event)
		{
			printf("subcmd %d, callback 0x%x\n", i, sc->callback_event);
			sc->callback_event(0, 0);
		}
#endif		
		size = 20;
//		printf("%s(line: %d), main win %d, sub win %d\n", __FUNCTION__, __LINE__, p->m_cursor, i);
		pthread_mutex_lock(&ui_mutex);
		manage_set_container(MTGO_LAYER_OSD0, sc->p_dis_text, size, sc->rc,  text_color, color, MTGO_LAYOUT_LEFT | MTGO_LAYOUT_VCENTER);
		pthread_mutex_unlock(&ui_mutex);
	}
	//printf(" *****%d**********  \n",__LINE__);

	MT_GO_RefreshLayer(g_layer, NULL);
}

mt_s32 manage_clear_layer()
{
	MT_RECT rc = {0};
	mt_handle layer = g_layerSurface;
	
	rc.x = 0;
	rc.y = 100;
	rc.w = 1280;
	rc.h = 620;
	MT_GO_FillRect(layer, &rc, 0xFF1E90FF, MTGO_COMPOPT_NONE);

	return MT_SUCCESS;
}

mt_s32 manage_ir_proc(mt_u16 ir_id)
{
	mt_u32 i = 0;
	struct MainWin *p  = &G_MAIN_WIN;
	struct SubWin *sp  = &G_SubWin[G_MAIN_WIN.m_cursor];

	pthread_mutex_lock(&ui_mutex);
	switch(ir_id)
	{
		case IR_ID_UP:
			if(sp->s_cursor == 0xff)
			{
				sp->s_cursor = 0;
				for(i = 0; i < sp->total_cnt; i++)
				{
					sp->s_cursor = (sp->s_cursor + sp->total_cnt - 1) % sp->total_cnt;
					if(sp->sc[sp->s_cursor].is_noncursor == FALSE)
						break;
				}
				if(i >= sp->total_cnt)
					sp->s_cursor = 0xff;
			}
			else
			{
				if(sp->cursor_cnt <= 1)
					;
				else
				{
					for(i = 0; i < sp->total_cnt; i++)
					{
						sp->s_cursor = (sp->s_cursor + sp->total_cnt - 1) % sp->total_cnt;
						if(sp->sc[sp->s_cursor].is_noncursor == FALSE)
							break;
					}
					
				}	
			}
			break;
		case IR_ID_DOWN:
			if(sp->cursor_cnt <= 1)
					;
			else
			{
				if(sp->s_cursor == 0xff)
					sp->s_cursor = sp->total_cnt - 1;
	
				for(i = 0; i < sp->total_cnt; i++)
				{
					sp->s_cursor = (sp->s_cursor + 1) % sp->total_cnt;
					if(sp->sc[sp->s_cursor].is_noncursor == FALSE)
						break;
				}
				if(i >= sp->total_cnt)
					sp->s_cursor = 0xff;
			}
			break;
		case IR_ID_LEFT:
			if(p->m_cursor == 0)
				p->m_cursor= p->cnt-1;
			else        
				p->m_cursor--;
			sp = &G_SubWin[p->m_cursor];
			sp->s_cursor = 0xff;
			if(p->wc[p->m_cursor].callback_event)
				p->wc[p->m_cursor].callback_event(0, 0);
			break;
		case IR_ID_RIGHT:
			p->m_cursor = (++p->m_cursor)%p->cnt;
			sp = &G_SubWin[p->m_cursor];
			sp->s_cursor = 0xff;
			if(p->wc[p->m_cursor].callback_event)
				p->wc[p->m_cursor].callback_event(0, 0);
			break;
		case IR_ID_OK:
			if(sp->sc[sp->s_cursor].callback_event)
				sp->sc[sp->s_cursor].callback_event(IR_ID_OK, (ulong)&sp->sc[sp->s_cursor], 0);
			break;
		default:
			break;
	}
	pthread_mutex_unlock(&ui_mutex);
	manage_update_ui(UPDATE_ALL, 0);

	return MT_SUCCESS;
}

mt_s32 manage_update_sub_event(mt_u32 m_type, mt_u32 sub_type, mt_u32 param)
{
	struct SubWin *sp  = &G_SubWin[m_type];
	mt_u32 i = 0;

	for(i = 0; i < sp->total_cnt; i++)
	{
		if(sp->sc[i].type == sub_type)
			break;
	}

	if(i < sp->total_cnt)
	{
		if(sp->sc[i].callback_event)
		{
			pthread_mutex_lock(&ui_mutex);
			sp->sc[i].callback_event(0, (ulong)&sp->sc[i], param);
			pthread_mutex_unlock(&ui_mutex);
		}
	}
		
	return MT_SUCCESS;
}


