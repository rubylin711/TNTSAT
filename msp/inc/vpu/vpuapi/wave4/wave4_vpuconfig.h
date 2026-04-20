//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2013  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//		This file should be modified by some customers according to their SOC configuration.
//--=========================================================================--

#ifndef __WAVE410_VPU_CONFIGURATION_H__
#define __WAVE410_VPU_CONFIGURATION_H__

#include "vpuconfig.h"

#define WAVE4_STACK_SIZE (8 * 1024) /* DO NOT CHANGE */
#define WAVE4_MAX_CODE_BUF_SIZE (512 * 1024)
#define WAVE4_WORKBUF_SIZE (512 * 1024)
#define WAVE4_NUM_BPU_CORE 1
#define WAVE4_COMMAND_TIMEOUT 0xffff /* MAX: 0xffff, 1 == 32768 ticks, 1 sec. == vCPU clocks */
#define WAVE4_VCPU_CLOCK_IN_MHZ 345  /* Vcpu clock in MHz */

#endif /* __WAVE410_VPU_CONFIGURATION_H__ */
