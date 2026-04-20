/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
  File Name     : mt_drv_opc.h
  Version       : Initial Draft
  Author        : Montage soc software group
  Created       : 2024/06/17
  Description   :
  History       :
  1.Date        : 2024/06/17
    Author      :
    Modification: Created file

******************************************************************************/
#ifndef __MT_DRV_OPC_H__
#define __MT_DRV_OPC_H__

#include "mt_unf_common.h"
#include "mt_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* End of #ifdef __cplusplus */

#define OPC_NETLINK_ID  24

#define __IOM volatile

typedef struct __OPC_VIOLATION_STATUS_S{
	 //op_sec_flag_lo
	 __IOM uint32_t reserved0           : 3;
	 __IOM uint32_t OSD0                : 1;
	 __IOM uint32_t video_con           : 1;
	 __IOM uint32_t subv_con            : 1;
	 __IOM uint32_t hdcp_1px            : 1;
	 __IOM uint32_t hdcp_2px            : 1;
	 __IOM uint32_t res_4k              : 1;
	 __IOM uint32_t res_2k              : 1;
	 __IOM uint32_t res_hd              : 1;
	 __IOM uint32_t res_sd              : 1;
	 __IOM uint32_t dscaler             : 1;
	 __IOM uint32_t reserved1           : 3;
	 __IOM uint32_t dscaler_4k          : 1;
	 __IOM uint32_t dscaler_2k          : 1;
	 __IOM uint32_t dscaler_hd          : 1;
	 __IOM uint32_t dscaler_sd          : 1;
	 __IOM uint32_t wo_wmvm             : 1;
	 __IOM uint32_t wo_vid_wmng         : 1;
	 __IOM uint32_t wo_cgmsa            : 1;
	 __IOM uint32_t vid_4k_hdcp_2px     : 1;
	 __IOM uint32_t vid_2k_hdcp_2px     : 1;
	 __IOM uint32_t vid_hd_hdcp_2px     : 1;
	 __IOM uint32_t vid_sd_hdcp_2px     : 1;
	 __IOM uint32_t vid_4k_hdcp_1px     : 1;
	 __IOM uint32_t vid_2k_hdcp_1px     : 1;
	 __IOM uint32_t vid_hd_hdcp_1px     : 1;
	 __IOM uint32_t vid_sd_hdcp_1px     : 1;
	 __IOM uint32_t wo_macrov           : 1;
	 //---------------32--------------------
	 //op_sec_flag_hi
	 __IOM uint32_t OSD1                : 1;
	 __IOM uint32_t OSD2                : 1;
	 __IOM uint32_t wo_subv_wmng        : 1;
	 __IOM uint32_t subv_4k_hdcp_2px    : 1;
	 __IOM uint32_t subv_2k_hdcp_2px    : 1;
	 __IOM uint32_t subv_hd_hdcp_2px    : 1;
	 __IOM uint32_t subv_sd_hdcp_2px    : 1;
	 __IOM uint32_t subv_4k_hdcp_1px    : 1;
	 __IOM uint32_t subv_2k_hdcp_1px    : 1;
	 __IOM uint32_t subv_hd_hdcp_1px    : 1;
	 __IOM uint32_t subv_sd_hdcp_1px    : 1;
	 __IOM uint32_t hd_resolution_change: 1;
	 __IOM uint32_t reserved2           : 20;
}OPC_VIOLATION_STATUS_S;


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif /* End of #ifndef __MT_DRV_OPC_H__ */
