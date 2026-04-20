/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@file mt_pvr_api.h
@brief Describes the information about the pvr module
*/
#ifndef __MPVR__TYPES_H_
#define __MPVR__TYPES_H_
#include "mt_type.h"

#ifndef utc_time_t
typedef struct
{
  /*!
    Year
    */
  u16 year;
  /*!
    Month
    */
  u8 month;
  /*!
    Day
    */
  u8 day;
  /*!
    Hour
    */
  u8 hour;
  /*!
    Minute
    */
  u8 minute;
  /*!
    Sec
    */
  u8 second;  
  /*!
    researved.
    */
  u8 reserved;
} utc_time_t;
#endif
typedef struct tag_extern_pid
{
    /*!
    pid
    */
    u32 pid : 16;
    /*!
    extern_pid_type_t
    */
    u32 type : 8;
    /*!
    fmt if video or audio type
    */
    u32 fmt : 8;
} extern_pid_t;
#endif
