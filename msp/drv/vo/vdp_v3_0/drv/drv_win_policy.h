#ifndef __WIN_POLICY_H_
#define __WIN_POLICY_H_

#include "drv_display.h"

#include "drv_window.h"
#include "drv_win_policy.h"
#include "drv_win_priv.h"
#include "mt_drv_sys.h"
#include "mt_drv_stat.h"
//#include "drv_vdec_ext.h"
#include "drv_disp_hal.h"
//#include "drv_disp_alg_service.h"
#include "mt_drv_module.h"
#include "drv_pq_ext.h"


int Win_Pre_ScalerDistribute(WINDOW_S *pstWin,
                             mt_rect_s *pstVpssZmeSize,
                             mt_rect_s *pstFinalZmeSize,
                             mt_u32 u32WinNum,
                             MT_BOOL *pbHorSrEnable,
                             MT_BOOL *pbVerSrEnable,
                             mt_rect_s *pstFmtResolution,
                             MT_DRV_DISP_STEREO_E enStereo);
int Win_Post_ScalerProcess( WINDOW_S *pstWin,
                            mt_rect_s *pstFinalDisPosition,
                            mt_rect_s *pstV0DisPosition,
                            MT_BOOL    *pbHorSrEnable,
                            MT_BOOL    *pbVerSrEnable,
                            mt_u32 u32WinNum,
                            mt_rect_s *pstSourceFrameRect,
                            const mt_rect_s *pstFmtResolution,
                            MT_DRV_DISP_STEREO_E enStereo);

mt_void Win_DciEnable_Policy(WINDOW_S *pstWin,
                         mt_rect_s *pstOriginDciArea,/*vpss's effective area.*/
                         mt_rect_s *pstV0DisPosition,/*secondary coordinate*/
                         mt_rect_s *pstVpssGive,/*the original rect give by vpss.*/
                         mt_u32    u32WinNum);

mt_s32 Win_Revise_OutOfScreenWin_OutRect(mt_rect_s *pstInRect,
                        mt_rect_s *pstOutRect,
                        mt_rect_s stScreen,
                        MT_DRV_DISP_OFFSET_S stOffsetRect,
                        WIN_HAL_PARA_S *pstLayerPara);

mt_s32 Win_TestDci(mt_void);
mt_s32 Win_TestPost(mt_void);
mt_s32 Win_TestSR(mt_void);

#endif




