/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_unf_hdmi.h"
#include "mt_unf_video.h"

#define MT_DAC_CVBS 0
#define MT_DAC_YPBPR_Y 1
#define MT_DAC_YPBPR_PB 2
#define MT_DAC_YPBPR_PR 3

#define PROG_MAX_AUDIO 	8
#define SUBTDES_INFO_MAX 10
#define SUBTITLING_MAX 15

#define INVALID_TSPID (0x1fff)

typedef void (*User_HDMI_CallBack)(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData);

typedef struct mtHDMI_ARGS_S
{
    MT_UNF_HDMI_ID_E enHdmi;
} HDMI_ARGS_S;

typedef struct
{
	mt_u16			key_id;
	mt_u64			key_value;     //the second step.Called in manage_open_menu
}ir_attr_t;


enum ir_id
{
	IR_ID_POWER,
	IR_ID_UP,
	IR_ID_DOWN,
	IR_ID_LEFT,
	IR_ID_RIGHT,
	IR_ID_OK,
	IR_ID_OTHERS
};

typedef struct mtProg_Info
{
    mt_u16 v_pid;
	mt_u32 v_type;
    mt_u16 a_pid;
	mt_u32 a_type;
	mt_u16 pcr_pid;
}prog_info_t;

typedef struct s_net_info{
	mt_char net_mac[18];
	mt_char net_ip_addr[16];
	mt_char net_ip_mask[16];
	mt_char net_gateway[16];
	mt_char net_gate_mask[16];
	mt_char net_dns[16];
	mt_u16 net_status;
}net_info_t;

/*!
  DVBC lock info
  */
typedef struct
{
  /*!
    freq
    */
  u32 tp_freq;
  /*!
    sym
    */
  u32 tp_sym;
  /*!
    nim modulate
    */
  u32 nim_modulate;
} dvbc_lock_info_t;



extern mt_s32 demo_ui_init(void);
extern mt_s32 demo_usb_init(void);
extern mt_s32 demo_tuner_start(void);
extern mt_s32 demo_playback_init(mt_u8 port);
extern mt_s32 demo_av_play(void);
extern mt_s32 manage_ir_proc(mt_u16 ir_id);
extern mt_s32 manage_ir_proc_test(mt_u16 ir_id);

mt_s32 demo_ir_init(void);
mt_s32 demo_frontend_init(void);
mt_s32 demo_hdmi_init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt);
mt_s32 demo_disp_init(MT_UNF_ENC_FMT_E enFormat);
mt_s32 demo_disp_deinit(mt_void);
mt_void demo_get_hdmi_status(mt_s32 *status);



