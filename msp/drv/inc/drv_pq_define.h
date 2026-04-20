/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/********************************************************************************************
  File Name     : drv_pq_define.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      : 
    Modification: 

*********************************************************************************************/

#ifndef _DRV_PQ_DEFINE_H_
#define _DRV_PQ_DEFINE_H_

#define MT_PQ_V3_0

#ifdef MT_PQ_V1_0
#include "../pq/pq_v1_0/include/drv_pq_define_v1.h"
#endif

#ifdef MT_PQ_V2_0
#include "../pq/pq_v2_0/include/drv_pq_define_v2.h"
#endif

#ifdef MT_PQ_V3_0
#include "../pq/pq_v3_0/include/drv_pq_define_v3.h"
#endif

#endif

