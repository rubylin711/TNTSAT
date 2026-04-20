/******************************************************************************
 *  Copyright (C), 2017, Montage Technologies Co., Ltd.
 *
 *******************************************************************************
 * File name     : drv_disp.h
 * Version       : 1.0
 * Author        : xxx
 * Created       : 2017-01-19
 * Last Modified :
 * Description   :
 * Function List :
 * History       :
 * 1 Date        :
 * Author        : xxx
 * Modification  : Create file
 *******************************************************************************/
#ifndef __DRV_DISP_H__
#define __DRV_DISP_H__

#include "mt_drv_proc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */


typedef struct mtDISP_PROC_FN_S
{
    mt_proc_read_func  rdproc;
    mt_drv_proc_write_func wtproc;
}DISP_PROC_FN_S;

typedef struct hiWIN_PROC_FN_S{
    mt_proc_read_func  rdProc;
    mt_drv_proc_write_func wtProc;
    mt_proc_read_func  winRdProc;
    mt_drv_proc_write_func winWtProc;
}WIN_PROC_FN_S;


typedef struct disp_priv_data_s
{
    struct clk *disclk;
    struct clk *diclk;
    struct clk *osdclk;
    struct clk *presclk;
    struct clk *disaxiclk;
    struct clk *hdclk;
    struct clk *sdclk_27m;
} disp_priv_data;

typedef struct mtDRV_DISP_STATE_S
{
    MT_BOOL bDispOpen[MT_DRV_DISPLAY_BUTT];
    mt_handle hCastHandle[MT_DRV_DISPLAY_BUTT];
    mt_handle hSnapshot[MT_DRV_DISPLAY_BUTT];
}DRV_DISP_STATE_S;

typedef struct mtDRV_DISP_GLOBAL_STATE_S
{
    mt_u32 DispOpenNum[MT_DRV_DISPLAY_BUTT];
}DRV_DISP_GLOBAL_STATE_S;


//#define  DISP_SLOT_FIFO_CNT         7

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*  __DRV_DISP_H__  */

