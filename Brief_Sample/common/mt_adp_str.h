/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __MT_ADP_STR_H__
#define __MT_ADP_STR_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_demux.h"
#include "mt_adp_mpi.h"
#include "mt_adp_frontend.h"
#include "mt_adp_hdmi.h"
/***************************** Macro Definition ******************************/
#define MAX_PMT_NUM        32
#define MAX_TP_NUM         32
#define DMX_ID_0           0
/*************************** Structure Definition ****************************/
/*define nim play use tuner id*/
typedef enum
{
    MT_NIM_TUNER_TYPE_DVBS_IN,
    MT_NIM_TUNER_TYPE_DVBS_OUT,
    MT_NIM_TUNER_TYPE_DVBT,
    MT_NIM_TUNER_TYPE_DVBC,
    MT_NIM_TUNER_TYPE_J83B,
    MT_NIM_TUNER_TYPE_BUTT
} mt_nim_tuner_type;

/*define tuner connect status*/
typedef enum
{
    MT_NIM_TUNER_NOT_CONNECTED,
    MT_NIM_TUNER_CONNECTED,
    MT_NIM_TUNER_CONNECTED_STATUS_BUTT
} mt_tuner_connect_status;

/*define nim play use dvbc input info */
typedef struct
{
    mt_u32 tuner_id; /**<tuner id*/
    mt_u32 freq;     /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 mod_type; /**<QAM mode*/
    mt_u32 positon;  /**Find the position of dvbc in the parameters*/
    MT_UNF_DMX_PORT_E port; /** tsi port*/
    mt_u32 connect_status; /* dvbc connect status*/
} mt_nim_dvbc_input;

/*define nim play use dvbt input info */
typedef struct
{
    mt_u32 tuner_id;  /**<tuner id*/
    mt_u32 freq;      /**<Frequency, in kHz*/
    mt_u32 bandwidth; /**<Symbol rate, in bit/s*/
    mt_u32 positon;  /**Find the position of dvbc in the parameters*/
    MT_UNF_DMX_PORT_E port; /** tsi port*/
    mt_u32 connect_status; /* dvbt connect status*/
} mt_nim_dvbt_input;

/*define nim play use dvbs input info */
typedef struct
{
    mt_s32 tuner_id;  /**<tuner id*/
    mt_u32 freq;      /**<Frequency, in kHz*/
    mt_u32 sym_rate;  /**<Symbol rate, in bit/s*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar;     /**<Polarization mode>*/
    mt_u32 dvbs_type; /**<dvbs type>*/
    mt_u32 positon;  /**Find the position of dvbc in the parameters*/
    MT_UNF_DMX_PORT_E port; /** tsi port*/
    mt_u32 connect_status; /* dvbs connect status*/
} mt_nim_dvbs_input;

/*define nim play use jb38 input info */
typedef struct
{
    mt_u32 tuner_id;   /**<tuner id*/
    mt_u32 freq; /**<Frequency, in kHz*/              /**<CNcomment:频率，单位：kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/      /**<CNcomment:符号率，单位bps */
    mt_u32 mod_type; /**<QAM mode*/                   /**<CNcomment:QAM调制方式*/
    mt_u32 positon;  /**Find the position of dvbc in the parameters*/
    MT_UNF_DMX_PORT_E port; /** tsi port*/
    mt_u32 connect_status; /* j83b connect status*/
} mt_nim_j83b_input;

typedef struct
{
    mt_s32 program_num;
    PMT_COMPACT_PROG prog[MAX_PMT_NUM];
} mt_prog_info;

typedef struct
{
    mt_nim_dvbc_input dvbc;
    mt_nim_dvbt_input dvbt;
    mt_nim_dvbs_input dvbs_in;
    mt_nim_dvbs_input dvbs_out;
    mt_nim_j83b_input j83b;
    mt_u32 nim_total;
    mt_u32 nim_use;
    mt_u32 *nim_type_arr;
} mt_nim_input_para_info;

typedef struct
{
    MT_HANDLE hAvPlay;
    MT_HANDLE hSoundTrack;
    MT_HANDLE hWin;
    mt_u32 tpIndex;
    mt_u32 total_prog_num;
    mt_prog_info prog_info[MAX_TP_NUM];

    mt_nim_input_para_info sInputParam;
} mt_nim_play_info;

typedef struct
{
    MT_UNF_ENC_FMT_E fmt;
    MT_BOOL hdcp_enable;
} mt_str_play_status;

extern cec_config_t g_cec_cfg;
extern mt_nim_play_info g_nimPlayInfo;

void MTADP_STR_StartSigStrStandby(void);
void MTADP_STR_FrontEnterStandby(void);
mt_s32 MTADP_STR_FrontExitStandby(void);
mt_void MTADP_STR_SetPlayStatus(mt_str_play_status play_status);
mt_void MTADP_STR_GetPlayStatus(mt_str_play_status *play_status);

#endif

