/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __UI_MENU_MANAGER_H__
#define __UI_MENU_MANAGER_H__

#include "mtgo_common.h"

enum ui_root_id
{
	ROOT_ID_BASIC,
	ROOT_ID_AVPLAY,
	ROOT_ID_DVBC,
	ROOT_ID_DVBS,
	ROOT_ID_NET,
	ROOT_ID_FRONT
};

typedef enum
{
	INT_TYPE =0,
	STRING_TYPE,
}ARG_TYPE;

typedef enum
{
	UPDATE_ALL =0,
	UPDATE_MODULE,
}UPDATE_TYPE;


#define MENU_STARTX (250)
#define MENU_STARTY (72)
#define MENU_WIDTH  (750)
#define MENU_HEIGHT (600)
#define MAX_MAIN_WIN_COUNT (20)
#define MAX_SUB_WIN_COUNT (20)
#define MAX_TEXT_LEN (50)

struct WinCmd{

	mt_char p_dis_text[MAX_TEXT_LEN];
	mt_s32 type;
	MT_RECT rc;

	mt_s32 (*callback_event)(mt_u16 evt_id, mt_u32 para);
};

struct MainWin{
	struct WinCmd wc[MAX_SUB_WIN_COUNT];

	mt_s32 cnt;
	mt_s32 m_cursor;
};

struct SubCmd{
	// ARG_TYPE arg_type;
	mt_char *string_type_pp[32];
	//  int  type_cnt;
	mt_char p_dis_text[MAX_TEXT_LEN];
	mt_s32 type;
	MT_RECT rc;
	mt_s32 m_cursor;  // belong to main cmd
	mt_s32 c_cursor;
	mt_s32 cnt;

	mt_bool is_noncursor;
	mt_bool is_usr_define;
	mt_s32 usr_color;

	mt_s32 (*callback_event)(mt_u16 evt_id, mt_u32 para1, mt_u32 para2);
};

struct SubWin{
	struct SubCmd sc[MAX_SUB_WIN_COUNT];
	
	mt_s32 total_cnt;
	mt_s32 cursor_cnt;
	mt_s32 s_cursor;

};

typedef  mt_s32 (*OPEN_MENU)(mt_u32 para1, mt_u32 para2);

typedef struct
{
	mt_u16		root_id;
	OPEN_MENU		open_func;     //the second step.Called in manage_open_menu
}menu_attr_t;

mt_s32 manage_open_menu(mt_u16 root_id, mt_u32 para1, mt_u32 para2);
mt_s32 manage_set_surface(mt_handle layer);
mt_s32 manage_set_font(mt_handle font);
mt_handle manage_get_surface(void);
mt_handle manage_get_font(void);
void manage_menu_init(void);
mt_s32 add_main_win(mt_char *pText, mt_s32 type, MT_RECT rc, mt_s32 (*callback_event)(mt_u16 evt_id, mt_u32 para));
mt_s32 get_main_win_index(mt_s32 type);
mt_s32 add_sub_win_cmd(struct SubCmd *sc);
void manage_update_ui(mt_u8 mode, mt_u32 module);

mt_s32 open_basic(mt_u32 para1, mt_u32 para2);
mt_s32 open_avplay(mt_u32 para1, mt_u32 para2);
mt_s32 open_dvbc(mt_u32 para1, mt_u32 para2);
mt_s32 open_dvbs(mt_u32 para1, mt_u32 para2);
mt_s32 open_net(mt_u32 para1, mt_u32 para2);
mt_s32 open_front(mt_u32 para1, mt_u32 para2);

extern mt_s32 demo_set_vo_window(void *p_rect);
extern mt_s32 demo_av_set_mute(MT_BOOL mute);
extern mt_s32 demo_rebuild_playback(mt_u8 port);
extern mt_s32 demo_net_fetch_info(mt_char *net_info);



#endif

