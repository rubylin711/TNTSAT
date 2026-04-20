#ifndef     _SAMPLE_MT_FRONTEND_H
#define     _SAMPLE_MT_FRONTEND_H

#include "mt_type.h"
#include "mt_unf_frontend.h"
#include "mt_adp_boardcfg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef struct {
    mt_u32 tunerId;
    mt_unf_fe_sig_type_t signal_type;
    mt_u32 freq;
    mt_u32 sym_rate;
    mt_u32 band_width;
    mt_u32 qam_type;
}mtadp_dvbc_info_t;

typedef struct {
    mt_u32 tunerId;
    mt_unf_fe_sig_type_t signal_type;
    mt_u32 freq;
    mt_u32 sym_rate;
    mt_u32 band_width;
    mt_u32 qam_type;
}mtadp_dvbs_info_t;

typedef struct {
    mt_u32 tunerId;
    mt_unf_fe_sig_type_t signal_type;
    mt_u32 freq;
    mt_u32 sym_rate;
    mt_u32 band_width;
    mt_u32 qam_type;
	mt_unf_fe_ter_mode_t channel_mode;
	mt_u8 noneed_setpara_flag;
	mt_u8 plp_id;
	mt_u8 plp_num;
	mt_u32 lock_timeout;
	
}mtadp_dvbt_info_t;

typedef struct {
    mt_u32 tunerId;
    mt_unf_fe_sig_type_t signal_type;
    mt_u32 freq;
    mt_u32 sym_rate;
    mt_u32 band_width;
    mt_u32 qam_type;
}mtadp_j83b_info_t;


/* **********************************public interface of Tuner********************************/
mt_s32 MTADP_Fe_Init(mt_u32 tuner_id);

mt_s32 MTADP_Fe_DeInit(mt_u32 tuner_id);

mt_s32 MTADP_Fe_Connect_Dvbc(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 tuner_qam);

mt_s32 MTADP_Fe_Connect_Dvbs(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 onoff_22k, mt_u32 polar, mt_u32 port_type);

mt_s32 MTADP_Fe_Connect_Dvbt(mt_u32 tuner_id, mt_u32 freq, mt_u32 band_width);

mt_s32 MTADP_Fe_Connect_Dvbt_Certainty_Signal(mtadp_dvbt_info_t *p_dvbt_info);

mt_s32 MTADP_Fe_Connect_J83b(mt_u32 tuner_id, mt_u32 freq, mt_u32 sym_rate, mt_u32 tuner_qam);

mt_s32 MTADP_Fe_Get_Signal_Info(mt_u32 tuner_id);

#endif
