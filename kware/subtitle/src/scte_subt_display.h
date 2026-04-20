#ifndef __SCTE_SUBT_DISPLAY_H__
#define __SCTE_SUBT_DISPLAY_H__

#include "mt_type.h"
#include "scte_subt_parse.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagSCTE_SUBT_DISPALY_PARAM_S
{
    SCTE_SUBT_DISP_E enDISP;
    mt_u32           u32x;
    mt_u32           u32y;
    mt_u32           u32w;
    mt_u32           u32h;
    mt_u32           u32BitWidth;
    mt_u32           u32PTS;
    mt_u32           u32Duration;

    mt_u32           u32DataLen;
    mt_u8*           pu8SubtData;
    mt_u32           u32DisplayWidth;
    mt_u32           u32DisplayHeight;
} SCTE_SUBT_DISPALY_PARAM_S;

typedef mt_s32 (*SCTE_SUBT_DISPLAY_CALLBACK_FN)(ulong u32UserData, mt_void *pstDisplayDate);

mt_s32 SCTE_SUBT_Display_Init(mt_void);

mt_s32 SCTE_SUBT_Display_DeInit(mt_void);

mt_s32 SCTE_SUBT_Display_Create(SCTE_SUBT_DISPLAY_CALLBACK_FN pfnCallback, ulong u32UserData, MT_HANDLE *phDisplay);

mt_s32 SCTE_SUBT_Display_Destroy(MT_HANDLE hDisplay);

mt_s32 SCTE_SUBT_Display_DisplaySubt(MT_HANDLE hDisplay, SCTE_SUBT_OUTPUT_S *pstOutData);

#ifdef __cplusplus
}
#endif
#endif
