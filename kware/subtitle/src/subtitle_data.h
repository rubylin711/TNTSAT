#ifndef __SUBTITLE_DATA_H__
#define __SUBTITLE_DATA_H__

#include "mt_type.h"

#ifdef __cplusplus
extern "C"{
#endif


/*
 *@brief: Initialize this module
 *
 *@retval ::MT_SUCCESS - upon successful.
 *@retval ::MT_FAILURE - failed.
 */
mt_s32 SUBT_DataRecv_Init(mt_void);
/*
 *@brief: Destroy this module
 *
 *@retval ::MT_SUCCESS - upon successful.
 *@retval ::MT_FAILURE - failed.
 */
 mt_s32 SUBT_DataRecv_DeInit(mt_void);

/*
 *@brief: Create the data receiving module.
 *
 *@param[in] u16PageID - page ID
 *@param[in] u16AncillaryID - ancillary page ID
 *@param[out] phDataRecv - this module handle.
 *
 *@retval ::MT_SUCCESS - upon successful.
 *@retval ::MT_FAILURE - failed.
 */
mt_s32 SUBT_DataRecv_Create(mt_u16 u16PageID, mt_u16 u16AncillaryID, MT_HANDLE *phDataRecv);

/*
 *@brief:Destroy data receiving moudle.
 *
 *@param[in] hDataRecv - this module handle
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful
 *@retval ::MT_FAILURE - failed.
 */
mt_s32 SUBT_DataRecv_Destroy(MT_HANDLE hDataRecv);

/*
 *@brief:Reset data receiving moudle.
 *
 *@param[in] hDataRecv - this module handle
 *@param[in] bRecvFlag - whether recv data or not
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful
 *@retval ::MT_FAILURE - failed to reset.
 */
mt_s32 SUBT_DataRecv_Reset(MT_HANDLE hDataRecv, MT_BOOL bRecvFlag);

/*
 *@brief:updata page id and ancillary id.
 *
 *@param[in] hDataRecv - this module handle
 *@param[in] u16PageID - page id
 *@param[in] u16AncillaryID - ancillary id
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful
 *@retval ::MT_FAILURE - failed to updata page id.
 */
mt_s32 SUBT_DataRecv_Updata(MT_HANDLE hDataRecv, mt_u16 u16PageID, mt_u16 u16AncillaryID);

/*
 *@brief:bind parsing module.
 *
 *@param[in] hDataRecv - this module handle
 *@param[in] hDataParse - the handle of data parsing module
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful.
 *@retval ::MT_FAILURE - failed to bind parsing module.
 */
mt_s32 SUBT_DataRecv_BindParsing(MT_HANDLE hDataRecv, MT_HANDLE hDataParse);

/*
 *@brief:unbind parsing module.
 *
 *@param[in] hDataRecv - this module handle
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful.
 *@retval ::MT_FAILURE - failed to unbind.
 */
mt_s32 SUBT_DataRecv_UnbindParsing(MT_HANDLE hDataRecv);

/*
 *@brief:redo the operation with cache data.
 *
 *@param[in] hDataRecv - this module handle
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful.
 *@retval ::MT_FAILURE - failed to redo.
 */
mt_s32 SUBT_DataRecv_Redo(MT_HANDLE hDataRecv);

/*
 *@brief:inject data.
 *
 *@param[in] hDataRecv - this module handle
 *@param[in] pu8Data - the pes packet data for subtitle
 *@param[in] u32DataSize - pes data len
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful, have finished parsing pes data.
 *@retval ::MT_FAILURE - cannot finish pes data parsing.
 */
mt_s32 SUBT_DataRecv_Inject(MT_HANDLE hDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize);


#ifdef __cplusplus
}
#endif

#endif
