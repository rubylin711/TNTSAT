#ifndef __SCTE_SUBT_DATA_H__
#define __SCTE_SUBT_DATA_H__

#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif


mt_s32 SCTE_SUBT_Data_Init(mt_void);

mt_s32 SCTE_SUBT_Data_DeInit(mt_void);

mt_s32 SCTE_SUBT_Data_Create(MT_HANDLE hParse, MT_HANDLE *phData);

mt_s32 SCTE_SUBT_Data_Destroy(MT_HANDLE hData);

mt_s32 SCTE_SUBT_Data_Reset(MT_HANDLE hData);

mt_s32 SCTE_SUBT_Data_Inject(MT_HANDLE hData, const mt_u8 *pu8Data, mt_u32 u32DataSize);

#ifdef __cplusplus
}
#endif

#endif
