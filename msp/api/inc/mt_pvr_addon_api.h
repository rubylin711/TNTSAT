/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@file mt_pvr_addon_api.h
@brief Describes the information about the pvr addon module
@details If user want to custom addon moudle, need to implement it
  mt_type.h
  mt_pvr_api.h
  mt_pvr_addon_XXX.h, "XXX" means a common addon or a special CAS addon.
@par Version
   <table>
    <tr><th>Date        <th>Version  <th>Author    <th>Description
    <tr><td>2020-3-14 <td>1.0 <td>jin.yue  <td>Init version
   </table>
*/
#ifndef __MT_PVR_ADDON_H_
#define __MT_PVR_ADDON_H_

/*
 @brief Defines return status of pvr addon module
*/
typedef enum
{
  MPVR_ADDON_STATUS_OK                    = 0,
  MPVR_ADDON_STATUS_FAILED                = 1,
  MPVR_ADDON_STATUS_INVALID_PARA          = 2,
  MPVR_ADDON_STATUS_NOT_ALLOWED           = 3,
  MPVR_ADDON_STATUS_CONTENTED_EXPIRED     = 4,
  MPVR_ADDON_STATUS_INVALID_INPUT_DATA    = 5,
  MPVR_ADDON_STATUS_NO_SC_INSERTED        = 6,
  MPVR_ADDON_STATUS_CREATE_FAIL           = 7,
  MPVR_ADDON_STATUS_EXIT_PLAY             = 8,
  MPVR_ADDON_STATUS_RUNNING_ERROR         = 9,
  MPVR_ADDON_STATUS_SEEK_NOT_ALLOWED      = 10,
  MPVR_ADDON_STATUS_TRICK_NOT_ALLOWED     = 11,
  MPVR_ADDON_STATUS_MATURITY_RATING_ERROR = 12,
  MPVR_ADDON_STATUS_EXIT_REC              = 13,
  MPVR_ADDON_STATUS_GET_SYS_TIME_FAILED   = 14,
  MPVR_ADDON_STATUS_INDEX_CHECK_FAILED    = 15,
  MPVR_ADDON_STATUS_REPLAY_FAILED         = 16,
  MPVR_ADDON_STATUS_DECRYPT_FAILED        = 17,
  MPVR_ADDON_STATUS_MAX,
}MPVR_ADDON_STATUS;

/*
 @brief Defines event type of pvr addon module
*/
typedef enum
{
  MPVR_ADDON_EVENT_TYPE_NONE              = 0,
  MPVR_ADDON_EVENT_TYPE_PRE_STOP          = 1,
  MPVR_ADDON_EVENT_TYPE_MAX,
}MPVR_ADDON_EVENT_TYPE;

/*
 @brief Defines return data type of data process
*/
typedef enum
{
  MPVR_ADDON_PROCESS_RET_TYPE_NONE        = 0,
  MPVR_ADDON_PROCESS_RET_TYPE_POS         = 1, //available position
  MPVR_ADDON_PROCESS_RET_TYPE_TIME        = 2, //available time based on current time, unit second
  MPVR_ADDON_PROCESS_RET_TYPE_STORE_24BITS     = 3, //store 24bit into index file;
  MPVR_ADDON_PROCESS_RET_TYPE_MAX,
}MPVR_ADDON_PROCESS_RET_TYPE;

#define MAX_PVR_HANDLES 4
/*
 @brief Defines version of pvr addon module
*/
typedef struct
{
  u32 major;
  u32 minor;
} mt_pvr_addon_version_t;

typedef enum
{
  /*!
  PVR TYPE of PVR.
  */
  MPVR_TYPE_PVR,
  /*!
  PVR TYPE of Timeshift with block file. deprecated.
  */
  //MPVR_TYPE_TIMESHIFT_BLOCK,
  /*!
  Timeshift with continuous recording until no storage space 
  */
  MPVR_TYPE_TIMESHITF_FILE,
  /*!
  Timeshift with a cycle time. max_ts_time in mt_pvr_rec_attr_t must config
  */
  MPVR_TYPE_TIMESHIFT_FILE_CYCLE,
  /*!
  Max pvr type
  */
  MPVR_TYPE_MAX,
}mt_pvr_type_t;

/*!
  @brief Defines the attributes of pvr addon record
  */
typedef struct
{
  mt_pvr_type_t pvr_type;
  /*
  pvr encrypt flag	
  */
  u8 pvr_encrypt_flag;
  /*!
    start time
    */
  MT_U32 start;
  /*!
  full path ts file name
  */
  u8 file_name[PVR_MAX_FILENAME_LEN];
  /*!
  full path ts file name length
  */
  u32 name_len;
  /*!
    point to pvr addon config struct (like, mt_pvr_addon_cfg_XXX_t) that is defined in cas addon 
    header file (like, mt_pvr_addon_XXX.h).
    "XXX" means a common addon or a special CAS addon.
    */
  void *p_addon_cfg;
  /*!
     pvr addon config struct length
    */
  u32 addon_cfg_len;
} mt_pvr_addon_rec_attr_t;

typedef struct
{
  /*!
  pvr type
  */
  mt_pvr_type_t pvr_type;
  /*
  pvr encrypt flag
  */
  u8 pvr_encrypt_flag;
  /*!
  full path ts file name
  */
  u8 file_name[PVR_MAX_FILENAME_LEN];
  /*!
  full path ts file name length
  */
  u32 name_len;
  /*!
    point to pvr addon config struct (like, mt_pvr_addon_XXX.h) that is defined in cas addon 
    header file (like, mt_pvr_addon_cfg_XXX_t ).
    "XXX" means a common addon or a special CAS addon.
    */
  void *p_addon_cfg;
  /*!
     pvr addon config struct length
    */
  u32 addon_cfg_len;
} mt_pvr_addon_play_attr_t;

typedef struct
{
  /*!
    src data address
    */
  u8* p_src_data;
  /*!
    dest data address
    */
  u8* p_dest_data;
  /*!
    src data address
    */
  ulong p_src_data_phy;
  /*!
    dest data address
    */
  ulong p_dest_data_phy;

  /*!
    input data lenth
    */
  u32 len;
  /*!
    file position
    */
  u64 pos;
  /*!
    current record time, ms
    */
  u32 curtime;
  /*!
    global offset of recording file
    */
  u64 global_offset;
}mt_pvr_addon_rec_param_input_t;

typedef struct
{
  /*!
  output type
    */
  MPVR_ADDON_PROCESS_RET_TYPE type;
  /*!
  output value
    */
  u64 value;
}mt_pvr_addon_rec_param_output_t;

typedef struct
{
  /*!
    reference data got from record process
    */
  //u64 ref_data;
  /*!
    current play time, ms
    */
  u32 curtime;
  /*!
    current play speed
    */
  MT_UNF_PVR_PLAY_SPEED_E  speed;
  /*!
    src data address
    */
  u8* p_src_data;
  /*!
    dest data address
    */
  u8* p_dest_data;
  /*!
    src data address
    */
  ulong p_src_data_phy;
  /*!
    dest data address
    */
  ulong p_dest_data_phy;

  /*!
    input data lenth
    */
  u32 len;
  /*!
    file position
    */
  u64 pos;
  /*!
    is seek data or not
    */
  MT_BOOL bSeek;
    /*!
    addon priv
    */
  u64 addon_priv;
	/*!
    global offset of recording file
    */
  u64 global_offset;
	/*!
    first push
    */
  u32 first_push;
}mt_pvr_addon_play_input_param_t;

typedef struct
{
  /*!
  output type
    */
  MPVR_ADDON_PROCESS_RET_TYPE type;
  /*!
  output value
    */
  u64 value;
}mt_pvr_addon_play_output_param_t;

// **************************************************************************
// * Below are pvr addon callback functions
// **************************************************************************
/*!
  @brief Initialize the pvr addon module.
  @param [in] p_args Pointer to pvr addon init configuration(like, mt_pvr_addon_XXX.h) 
            that is defined in cas addon header file (like, mt_pvr_addon_init_XXX_t ).
            "XXX" means a common addon or a special CAS addon.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_init_callback)(void *p_args);
/*!
  @brief Deinitialize the pvr addon module.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_deinit_callback)(void);
/*!
  @brief Check if pvr file valid to play.
  @param [in] p_ts_file_name  Pvr ts file full path name.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_is_content_valid_callback)(u8 *p_ts_file_name);
/*!
  @brief Check if pvr index file valid to play.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_is_index_valid_check_callback)(void);
/*!
  @brief Delete relative pvr files. 
  @param [in] p_ts_file_name  Pvr ts file full path name.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_delete_file_callback)(u8 *p_ts_file_name);

/*!
  @brief Rename relative pvr files. 
  @param [in] p_old_ts_name  Old pvr ts file full path name.
  @param [in] p_new_ts_name  New pvr ts file full path name.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rename_file_callback)(u8 *p_old_ts_name, u8 *p_new_ts_name);
// **************************************************************************
// * Below are pvr addon rec callback functions
// **************************************************************************
/*!
  @brief Create a record object.
  @param [out] p_handle Pointer to the handle of a created pvr addon record object.
  @param [in] p_rec_attr Pointer to record attributes. For details, see the description of ::mt_pvr_addon_rec_attr_t. 
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_create_callback)(mt_handle_t *p_handle, mt_pvr_addon_rec_attr_t *p_rec_attr);
/*!
  @brief Destroys record object.
  @param [in] handle Pointer to the handle of addon record object.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_destroy_callback)(mt_handle_t handle);
/*!
  @brief Start the record. 
  @param [in] handle Pointer to the handle of addon record object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_start_callback)(mt_handle_t handle);
/*!
  @brief Stop the record. 
  @param [in] handle Pointer to the handle of addon record object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_stop_callback)(mt_handle_t handle);
/*!
  @brief Pause the record. 
  @param [in] handle Pointer to the handle of addon record object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_pause_callback)(mt_handle_t handle);
/*!
  @brief Resume the record. 
  @param [in] handle Pointer to the handle of addon record object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_resume_callback)(mt_handle_t handle);
/*!
  @brief Data process of record. 
  @param [in] handle Pointer to the handle of addon record object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_EXIT_REC Not allowed to record, eixt recording.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed to record this data.
        MPVR_ADDON_STATUS_NO_SC_INSERTED No SC inserted.
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_dataprocess_callback)(mt_handle_t handle, u32 rechandle,
	MT_U32 channel_id, mt_pvr_addon_rec_param_input_t* p_param_in,
                                                                  mt_pvr_addon_rec_param_output_t* p_param_out);
/*!
  @brief Change pid of record.
  @param [in] handle Pointer to the handle of addon record object.
  @param [in] p_media Pointer to record media attributes. For details, see the description of ::mt_pvr_rec_media_t. 
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS(*mt_pvr_addon_rec_change_pid_callback)(mt_handle_t handle, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType);
/*!
  @brief Change pid of record.
  @param [in] handle Pointer to the handle of addon record object.
  @param [in] p_media Pointer to record media attributes. For details, see the description of ::mt_pvr_rec_media_t. 
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS(*mt_pvr_addon_rec_change_pid_ex_callback)(mt_handle_t handle, int pid, MT_UNF_DMX_CHAN_TYPE_E chnType, MT_BOOL add);
/*!
  @brief Get retention limit time.
  @param [in] handle Pointer to the handle of addon record object.
  @param [out] p_limit_second, Pointer to limited time, unit second.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_get_retention_limit_time_callback)(mt_handle_t handle, u32 *p_limit_second);
/*!
  @brief Send event of record.
  @param [in] handle Pointer to the handle of addon record object.
  @param [in] evet_type event type.
  @param [in] param1 param1, or the length of a struct.
  @param [in] param2 param2, or the address of a struct.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_event_callback)(mt_handle_t handle, MPVR_ADDON_EVENT_TYPE evet_type, u32 param1, u32 param2);
/*!
  @brief index file flush.
  @param [in] handle Pointer to the handle of addon record object.
  @param [in] data index data addr.
  @param [in] len index data length.
  @param [in] data_index, index of the index data.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_rec_on_index_flush_callback)(mt_handle_t handle, u8* data, u32 len, u32 data_index);



// **************************************************************************
// * Below are pvr addon play callback functions
// **************************************************************************
/*!
  @brief Create a play object.
  @param [out] p_handle Pointer to the handle of a created pvr addon play object.
  @param [in] p_play_attr Pointer to play attributes. For details, see the description of ::mt_pvr_addon_play_attr_t. 
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_create_callback)(mt_handle_t *p_handle, mt_pvr_addon_play_attr_t *p_play_attr);
/*!
  @brief Destroys play object.
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK, Success status.
        MPVR_ADDON_STATUS_FAILED, Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA, The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_destroy_callback)(mt_handle_t handle);
/*!
  @brief Start the play. 
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_start_callback)(mt_handle_t handle);
/*!
  @brief Stop the play. 
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_stop_callback)(mt_handle_t handle);
/*!
  @brief Pause the play. 
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_pause_callback)(mt_handle_t handle);
/*!
  @brief Resume the play. 
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_resume_callback)(mt_handle_t handle);
/*!
  @brief Data process of play. 
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_EXIT_PLAY Not allowed, exit the play.
        MPVR_ADDON_STATUS_TRICK_NOT_ALLOWED The trick play mode is not allowed, pvr should switch to normal mode.
        MPVR_ADDON_STATUS_SEEK_NOT_ALLOWED The seek is not allowed.
        MPVR_ADDON_STATUS_NO_SC_INSERTED No SC inserted.
        MPVR_ADDON_STATUS_MATURITY_RATING_ERROR Rating not allowed or request PIN error.
        MPVR_ADDON_STATUS_CONTENTED_EXPIRED Content expired.
        MPVR_ADDON_STATUS_RUNNING_ERROR The process is pre-stopped by event.
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_dataprocess_callback)(mt_handle_t handle, u32 dmx_id,
                                                            mt_pvr_addon_play_input_param_t* p_param_in, 
                                                            mt_pvr_addon_play_output_param_t * p_param_out);
/*!
  @brief Seek to play. 
  @param [in] handle Pointer to the handle of addon play object.
  @param [in] p_position  Pointer to play position attributes. For details, see the description of ::mt_pvr_play_position_t. 
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS(*mt_pvr_addon_play_seek_callback)(mt_handle_t handle, MT_UNF_PVR_PLAY_POSITION_S* p_position);
/*!
  @brief Trick play.
  @param [in] handle Pointer to the handle of addon play object.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS(*mt_pvr_addon_play_trickplay_callback)(mt_handle_t handle, MT_UNF_PVR_PLAY_SPEED_E trick_speed);
/*!
  @brief Trick play.
  @param [in] handle Pointer to the handle of addon play object.
  @param [in] a_pid  Audio pid.
  @param [in] fmt  Audio format. For details, see adec_src_fmt_vsb_t in aud_vsb.h
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS(*mt_pvr_addon_play_change_audio_callback)(mt_handle_t handle, u16 a_pid, u8 a_fmt);
/*!
  @brief Get available content period position.
  @param[in] handle Pointer to the handle of addon play object.
  @param[in] start_pos, start pos
  @param[in] bForward, is forward or not
  @param[out] p_pos, Pointer to available pos 
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_NOT_ALLOWED Not allowed
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS(*mt_pvr_addon_play_get_available_content_callback)(mt_handle_t handle, u64 start_pos, MT_BOOL bForward, u64 *p_pos);
/*!
  @brief Send play event.
  @param [in] handle Pointer to the handle of addon play object.
  @param [in] evet_type event type.
  @param [in] param1 param1, or the length of a struct.
  @param [in] param2 param2, or the address of a struct.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status. 
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_event_callback)(mt_handle_t handle, MPVR_ADDON_EVENT_TYPE evet_type, u32 param1, u32 param2);
/*!
  @brief index file valid check.
  @param [in] handle Pointer to the handle of addon record object.
  @param [in] data index data addr.
  @param [in] len index data length.
  @param [in] data_index, index of the index data.
  @return MPVR_ADDON_STATUS_OK Success status.
        MPVR_ADDON_STATUS_FAILED Failure status.
        MPVR_ADDON_STATUS_INVALID_PARA The input parameter is invalid.
*/
typedef MPVR_ADDON_STATUS (*mt_pvr_addon_play_on_index_check_callback)(mt_handle_t handle, u8* data, u32 len, u32 data_index);

typedef struct
{
  mt_pvr_addon_rec_create_callback create;
  mt_pvr_addon_rec_destroy_callback on_destroy;
  mt_pvr_addon_rec_start_callback on_start;
  mt_pvr_addon_rec_stop_callback on_stop;
  mt_pvr_addon_rec_pause_callback on_pause;
  mt_pvr_addon_rec_resume_callback on_resume;
  mt_pvr_addon_rec_dataprocess_callback data_process;
  mt_pvr_addon_rec_change_pid_callback on_change_pid;
  mt_pvr_addon_rec_on_index_flush_callback on_index_flush;
  mt_pvr_addon_rec_event_callback on_event;
    mt_pvr_addon_rec_change_pid_ex_callback on_change_pid_ex;
}mt_pvr_addon_rec_interface_t;

typedef struct
{
  mt_pvr_addon_play_create_callback create;
  mt_pvr_addon_play_destroy_callback on_destroy;
  mt_pvr_addon_play_start_callback on_start;
  mt_pvr_addon_play_stop_callback on_stop;
  mt_pvr_addon_play_pause_callback on_pause;
  mt_pvr_addon_play_resume_callback on_resume;
  mt_pvr_addon_play_dataprocess_callback data_process;
  mt_pvr_addon_play_seek_callback on_seek;
  mt_pvr_addon_play_trickplay_callback on_trickplay;
  mt_pvr_addon_play_change_audio_callback on_change_audio;
  mt_pvr_addon_play_on_index_check_callback on_index_check;
  mt_pvr_addon_play_event_callback on_event;
}mt_pvr_addon_play_interface_t;

typedef struct
{
  void *p_args;
  mt_pvr_addon_version_t version;
  mt_pvr_addon_init_callback init;
  mt_pvr_addon_deinit_callback deinit;
  mt_pvr_addon_rec_interface_t rec_inter;
  mt_pvr_addon_play_interface_t play_inter;
  mt_pvr_addon_is_content_valid_callback is_content_valid;
  mt_pvr_addon_is_index_valid_check_callback is_index_valid_check;
  mt_pvr_addon_delete_file_callback on_delete;
  mt_pvr_addon_rename_file_callback on_rename;
  mt_handle_t handlesrec[MAX_PVR_HANDLES];       //nagra hanlde
  mt_handle_t handlesply;
  mt_s32 handlesrec_pvr[MAX_PVR_HANDLES];   //app pvr handle
  mt_s32 handlesply_pvr;
  mt_u32 ts_cipher_size;
}mt_pvr_addon_t;
void mt_pvr_set_ca_addon(void *p_addon);
mt_pvr_addon_t *mt_pvr_get_ca_addon(void);
void mt_pvr_addon_on_delete(u8 *p_ts_file_name);
#endif //__MT_PVR_ADDON_H_

