#ifndef __SUBTITLE_DATA_PARSE_H__
#define __SUBTITLE_DATA_PARSE_H__

#include "mt_type.h"

#ifdef __cplusplus
extern "C"{
#endif
#define ERR_OUT_OF_WIDTH (-100)
/* Ref[Subtitling system.pdf] section 7.2.1 Table 3, for page_state*/
typedef enum tagSUBT_PAGE_STATE_E
{
    SUBTITLE_PAGE_NORMAL_CASE       = 0,    /* page update, use previous page instance to display */
    SUBTITLE_PAGE_ACQUISITION_POINT,        /* page refresh, use next page instance to display */
    SUBTITLE_PAGE_MODE_CHANGE,              /* new page */
    SUBTITLE_RESERVED                       /* reserved for future use */
}SUBT_PAGE_STATE_E;

/* Ref[Subtitling system.pdf], section 7.2.2 table 6, for object type */
typedef enum tagSUBT_OBJ_TYPE_E
{
    SUBT_OBJ_TYPE_BITMAP = 0x00,    /* basic object, bitmap */
    SUBT_OBJ_TYPE_CHARACTER,        /* basic object, character */
    SUBT_OBJ_TYPE_STRING,           /* composite object, string of charaters */
    SUBT_OBJ_TYPE_UNKOWN            /* reserved */
}SUBT_OBJ_TYPE_E;

typedef struct tagSUBT_Display_ITEM_S
{
    mt_u16 u16XPos;
    mt_u16 u16YPos;
    mt_u16 u16Width;
    mt_u16 u16Heigth;

    mt_void*  pvRegionClut;
    mt_u8  u8BitDepth;
    mt_u16  u16PaletteItem;

    mt_u32 u32PTS;

    mt_u32 u32Timeout; /* in millisecond */

    mt_u32 u32RegionDataSize;
    mt_u8* pu8ShowData;
    mt_u8  u8DataType;
    mt_u8  u8FrontClr;
    mt_u8  u8BackClr;

    SUBT_PAGE_STATE_E enPageState;

    mt_u16 u16DisplayWidth;
    mt_u16 u16DisplayHeight;
}SUBT_Display_ITEM_S;

typedef mt_s32 (*SUBT_DATAPARSE_CALLBACK_FN)(ulong u32UserData, mt_void *pstDisplayDate);


/*
 *@brief: Initialize this module
 *@param[in] None.
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS, upon successfully.
 *@retval ::MT_FAILURE, failed.
 */
mt_s32 SUBT_DataParse_Init(mt_void);
/*
 *@brief: Destroy this module
 *@param[in] None.
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS, upon successfully.
 *@retval ::MT_FAILURE, failed.
 */
mt_s32 SUBT_DataParse_DeInit(mt_void);

/*
 *@brief:create data parse module.
 *
 *@param[out] phDataParse This module handle.
 *
 *@retval ::MT_SUCCESS, upon successfully.
 *@retval ::MT_FAILURE, failed.
 */
mt_s32 SUBT_DataParse_Create(MT_HANDLE* phDataParse);

/*
 *@brief:Destroy data parse module.
 *
 *@param[in] hDataParse This module handle.
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS, upon successful
 *@retval ::MT_FAILURE, failed
 */
mt_s32 SUBT_DataParse_Destroy(MT_HANDLE hDataParse);

/*
 *@brief:Reset parse module data
 *
 *@param[in] hDataParse This module handle.
 *@param[in] bParseFlag After resetting, whether parsing data immediately or not.
 *@param[out] None.
 *
 *@retval ::None.
 */
mt_s32 SUBT_DataParse_Reset(MT_HANDLE hDataParse, MT_BOOL bParseFlag);

/*
 *@brief: To parse PES data packet.
 *
 *@param[in] hDataParse This module handle.
 *@param[in] pu8DataSrc The data to be resolved
 *@param[in] u32Len        The data length.
 *@param[in] u16PageID   The page id
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS, upon successful
 *@retval ::MT_FAILURE, failed
 */
mt_s32 SUBT_DataParse_ParsePESPacket(MT_HANDLE hDataParse, mt_u8 *pu8DataSrc , mt_u32 u32Len, mt_u16 u16PageID, mt_u16 u16AncillaryID);

/*
 *@brief: update callback info.
 *
 *@param[in] hDataParse This module handle.
 *
 *@param[out] pstRegionItem the subtitling data.
 *
 *@retval ::MT_SUCCESS, upon successful
 *@retval ::MT_FAILURE, failed
 */
mt_s32 SUBT_DataParse_Update(MT_HANDLE hDataParse, SUBT_DATAPARSE_CALLBACK_FN pfnCallback, ulong u32UserData);

#ifdef __cplusplus
}
#endif

#endif