#ifndef __MT_DF_PULLLDOWN_H
#define __MT_DF_PULLLDOWN_H

#include "mt_type.h"

#define ODD_FIELD 0
#define EVEN_FIELD 1
#define MAXFRAMENO (2^24)

#define MAXCAPTURE 12
#define FILM_MODE_CAP 6
#define DOWNGAP 2
#define STILL_DIFF_START 0
#define NOISE_THR 100000
#define PULLDOWN_EO_GAP 2 //3

#define MAXCAPTURE22 6
#define FILM_MODE_CAP22 3
#define DOWNGAP22 1
#define SCALE_OF_DIFF2  3 //10
#define DETECTNUM_2_2   10

#define SCALE_OF_DIFF4  3  // 3 //3 //10
#define SCALE_OF_DIFF3  1 //10

#define SCALE_OF_DIFF5  1 // 3 //3 //10
#define SCALE_OF_DIFF6  116 //128 //115 //3 //3 //10
#define SCALE_OF_DIFF7  80  //90 //
#define SCALE_OF_DIFF8  80  //90 //
#define SCALE_OF_DIFF9  80  //90 //

typedef struct pdd_diffs
{
  MT_U32 cur_diff_odd;
  MT_U32 cur_diff_even;
  MT_U32 field_diff_top;
  MT_U32 field_diff_top1;
  MT_U32 field_diff_top2;
  MT_U32 field_diff_top3;
  MT_U32 field_diff_bot;
  MT_U32 field_diff_bot1;
  MT_U32 field_diff_bot2;
  MT_U32 field_diff_bot3;
  MT_U32 diff_sum_top;
  MT_U32 diff_sum_top1;
  MT_U32 diff_sum_top2;
  MT_U32 diff_sum_top3;
  MT_U32 diff_sum_bot;
  MT_U32 diff_sum_bot1;
  MT_U32 diff_sum_bot2;
  MT_U32 diff_sum_bot3;
  MT_U32 sum_diff_top;
  MT_U32 sum_diff_bot;
}MT_DF_PDD_DIFFS_T;

typedef struct {
  MT_DF_BOOL process_start;
  MT_U8 pos_last_odd;
  MT_S8 capture_s_odd;
  MT_U8 pos_last_even;
  MT_S8 capture_s_even;
}MT_DF_MOVIEMODE_3_2_PARAM_T;

typedef struct {
  MT_DF_BOOL process_start;

  MT_U32 diff_cv_region[4][12];
  MT_U32 diff_dv_region[4][10];
  MT_U32 diff_field_region[4][10];
  MT_S8 capture_region[3];
  MT_DF_BOOL movie_mode_region[3];
  MT_S8 static_region[3];
}MT_DF_MOVIEMODE_2_2_PARAM_T;

typedef struct moviemode_context
{
  MT_DF_PDD_DIFFS_T pdd_diffs;

  MT_U8 odd_fieldno;
  MT_U8 even_fieldno;

  MT_U32 diff_odd[5];
  MT_U32 diff_even[5];
  MT_U32 diff_cont[5];
  MT_U32 frame_sum_diff[5];
  
  MT_U8 diff_pos_cont;
  MT_U8 min_pos_cont;

  MT_U8 min_pos_odd;
  MT_U8 min_pos_even;

  MT_DF_BOOL block_skip_3_2;
  MT_DF_BOOL block_skip_2_2;

  MT_DF_MOVIEMODE_3_2_PARAM_T param_32;
  MT_DF_MOVIEMODE_2_2_PARAM_T param_22;

  MT_U32 weave_flag;

  MT_U8 pos_to_weave;

  MT_DF_BOOL movie_mode_3_2;
  MT_DF_BOOL movie_mode_2_2;
  MT_DF_BOOL movie_mode_flag;
}MT_DF_MOVIEMODE_CONTEXT_T;

void DF_MovieMode_Init(void);
void DF_MovieMode_Process(DISP_POOL_S* pstDispBP);

#endif
