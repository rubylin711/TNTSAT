/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@file mt_pvr_addon_common.h
@brief Describes the information about the pvr addon common module
@par Version
   <table>
    <tr><th>Date        <th>Version  <th>Author    <th>Description
    <tr><td>2020-3-14 <td>1.0 <td>yichao.guan  <td>Init version
   </table>

*/
#ifndef __MT_PVR_ADDON_COMMON_H_
#define __MT_PVR_ADDON_COMMON_H_

/*!
  @brief Defines the type of data process
  */
typedef enum
{
  MPVR_PROCESS_TYPE_ENCRYPT = 0,
  MPVR_PROCESS_TYPE_DECRYPT = 1,
}mt_pvr_data_process_type_t;

/*!
  @brief Defines the type of agelimit exit
  */
#define MPVR_ADDON_COMMON_AGE_LIMIT_EXIT ((s32)-2)

/*!
  @brief Defines the paramters of data process callback for Crypto
  */
typedef struct
{
   /*!
  */
 void* handle;
   /*!
  */
 mt_pvr_data_process_type_t type;
  /*!
  */
  u8 *p_dest;
  /*!
  */
  u8 *p_src;
  /*!
  */
  ulong p_dest_phy;
  /*!
  */
  ulong p_src_phy;

  /*!
  */
  u32 length;
  /*!
  */
  u64 pos;
  /*!
  */
  void *p_args;
}mt_pvr_data_process_cb_param_t;

/*!
  @brief Defines of data process callback, encryption or decryption
  */
typedef RET_CODE (*mpvr_data_process_cb)(mt_pvr_data_process_cb_param_t* p_param);
/*!
  @brief Defines the callback of age limit check,call at pvr playback,  return MPVR_ADDON_COMMON_AGE_LIMIT_EXIT may exit play
  */
typedef RET_CODE (*mt_pvr_age_limit_check_func)(u32 handle ,void *p_args,u8 age_limit);

/*!
  @brief Defines the callback of age get, call at pvr recording
  */
typedef RET_CODE (*mt_pvr_age_limit_get_func)(u32 handle ,void *p_args,u8* age_limit);

/*!
  @brief Defines the callback of recording process
  @param out_handle, index of record channel, range (0-1).
  */
typedef RET_CODE (*mt_pvr_create_rec_channel_callback)(void *p_args,u8 *file_name,u32 name_len,u32 out_handle );

/*!
  @brief Defines the callback of playback process
  @param out_handle, index of play channel, range (0).
  */
typedef RET_CODE (*mt_pvr_create_play_channel_callback)(void *p_args,u8* file_name,u32 name_len,u32 out_handle );

/*!
  @brief Defines the initilization of pvr addon
  */
typedef struct 
{
  /*!
     ap args,may callback used
    */
   void *p_args;
  /*!
    set handle to ap, see mt_pvr_create_rec_channel_callback
    */
   mt_pvr_create_rec_channel_callback rec_handle_cbk;
  /*!
    set handle to ap, see mt_pvr_create_play_channel_callback
    */
   mt_pvr_create_play_channel_callback play_handle_cbk;
  /*!
    age limit check, see mt_pvr_age_limit_get_func
    */
   mt_pvr_age_limit_get_func get_age_limit;
  /*!
    age limit check, see mt_pvr_age_limit_check_func
    */
  mt_pvr_age_limit_check_func age_limit_check;
  /*!
    mpvr_data_process_cb, see mpvr_data_process_cb
    */
  mpvr_data_process_cb data_process_cb;
}mt_pvr_addon_init_common_t;

/*!
  @brief Defines the configuration of pvr addon 
  */
typedef struct 
{
  /*!
  NONE
  */
}mt_pvr_addon_cfg_common_t;

/*!
@brief Create a comon addon for pvr.
@param [in] args Userdata
@return Addon instance.
  - NULL Create fail.
  - Not NULL Success

@par Example:
@code
  mt_pvr_addon_init_common_t user_cbk={xxx, xxx ...};  
  void* p_addon = mt_pvr_common_addon_create(&user_cbk);
  mt_pvr_api_init_t init_param;
  init_param.p_pvr_addon = p_addon;
  mt_pvr_init(&init_param);
@endcode
*/
void* mt_pvr_common_addon_create(mt_pvr_addon_init_common_t * args);


#endif //__MT_PVR_ADDON_COMMON_H_

