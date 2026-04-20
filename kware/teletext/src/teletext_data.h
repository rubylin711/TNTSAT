#ifndef __TELETEXT_DATA_H__
#define __TELETEXT_DATA_H__

#include "mt_type.h"

#ifdef __cplusplus
extern "C"{
#endif

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
mt_s32 TTX_DataRecv_Create(MT_HANDLE *phDataRecv);

/*
 *@brief:Destroy data receiving moudle.
 *
 *@param[in] hDataRecv - this module handle
 *@param[out] None.
 *
 *@retval ::MT_SUCCESS - upon successful
 *@retval ::MT_FAILURE - failed.
 */
mt_s32 TTX_DataRecv_Destroy(MT_HANDLE hDataRecv);

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
mt_s32 TTX_DataRecv_Inject(MT_HANDLE hDataRecv, mt_u8 *pu8Data, mt_u32 u32DataSize);


#ifdef __cplusplus
}
#endif

#endif
