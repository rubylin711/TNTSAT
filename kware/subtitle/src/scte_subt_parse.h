#ifndef __SCTE_SUBT_PARSE_H__
#define __SCTE_SUBT_PARSE_H__

#include "mt_type.h"
#include "mt_unf_subt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tagSCTE_SUBT_OUTLINE_STYLE_E
{
    SCTE_SUBT_OUTLINE_DROPSHADOW = 0,
    SCTE_SUBT_OUTLINE_OUTLINED,
    SCTE_SUBT_OUTLINE_RESERVED
} SCTE_SUBT_OUTLINE_STYLE_E;

typedef enum tagSCTE_SUBT_BACKGROUND_STYLE_E
{
    SCTE_SUBT_BACKGROUD_FRAMED,
    SCTE_SUBT_BACKGROUD_TRANSPARENT
} SCTE_SUBT_BACKGROUND_STYLE_E;

typedef struct tagSCTE_SUBT_BACKGROUND_FRAMED_S
{
    mt_u32 u32FrameColor;
    mt_u32 u32TopXPos;
    mt_u32 u32TopYPos;
    mt_u32 u32ButtomXPos;
    mt_u32 u32ButtomYPos;
} SCTE_SUBT_BACKGROUND_FRAMED_S;

typedef struct tagSCTE_SUBT_OUTLINE_DROPSHADOW_S
{
    mt_u32 u32ShadowColor;
    mt_u16 u32ShadowRight;
    mt_u16 u32ShadowBottom;
} SCTE_SUBT_OUTLINE_DROPSHADOW_S;

typedef struct tagSCTE_SUBT_OUTLINE_OUTLINED_S
{
    mt_u32 u32OutlineColor;
    mt_u16 u16OutlineThickness;
} SCTE_SUBT_OUTLINE_OUTLINED_S;

typedef enum tagSCTE_DISPLAY_STANDARD_E
{
    STANDARD_720_480_30 = 0x0,
    STANDARD_720_576_25,
    STANDARD_1280_720_60,
    STANDARD_1920_1080_60
}SCTE_DISPLAY_STANDARD_E;

/** Defines the type of display mode */ /** CNcomment: 定义SCTE subtitle显示的方式 */
typedef enum tagSCTE_SUBT_DISP_E
{
    SCTE_SUBT_DISP_NORMAL = 0, /**<Normal Display *//**<CNcomment:正常显示 */
    SCTE_SUBT_DISP_PRECLEAR /**<Erase Display *//**<CNcomment:擦除显示 */
} SCTE_SUBT_DISP_E;

typedef struct tagSCTE_SUBT_OUTPUT_S
{
    SCTE_SUBT_DISP_E      enDISP;
    SCTE_SUBT_OUTLINE_STYLE_E    enOutlineStyle;
    SCTE_SUBT_BACKGROUND_STYLE_E enBackgroundStyle;
    SCTE_DISPLAY_STANDARD_E enDispStandard;

    mt_u32 u32TopXPos;
    mt_u32 u32TopYPos;
    mt_u32 u32ButtomXPos;
    mt_u32 u32ButtomYPos;

    mt_u32 u32SubtColor;
    mt_u32 u32PTS;
    mt_u32 u32Duration;
    mt_u32 u32BitWidth;

    union
    {
        SCTE_SUBT_OUTLINE_DROPSHADOW_S stDropshadow;
        SCTE_SUBT_OUTLINE_OUTLINED_S stOutline;
    } unOutlineStyle;

    SCTE_SUBT_BACKGROUND_FRAMED_S stFramed;
    mt_u32 u32FrameTopXPos;
    mt_u32 u32FrameTopYPos;
    mt_u32 u32FrameButtomXPos;
    mt_u32 u32FrameButtomYPos;

    mt_u32 u32BitmapDataLen;
    mt_u8  *pu8SCTESubtData;
} SCTE_SUBT_OUTPUT_S;

mt_s32 SCTE_SUBT_Parse_Init(mt_void);

mt_s32 SCTE_SUBT_Parse_DeInit(mt_void);

mt_s32 SCTE_SUBT_Parse_Create(MT_HANDLE hDisplay, MT_HANDLE *phParse);

mt_s32 SCTE_SUBT_Parse_Destroy(MT_HANDLE hParse);

mt_s32 SCTE_SUBT_Parse_RegGetPtsCb(MT_HANDLE hParse, MT_UNF_SUBT_GETPTS_FN pfnGetPts, ulong u32UserData);

mt_s32 SCTE_SUBT_Parse_ParseSection(MT_HANDLE hParse, mt_u8 *pu8DataSrc, mt_u32 u32DataLen);

#ifdef __cplusplus
}
#endif

#endif
