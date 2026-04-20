/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef __MT_HDMI_H__
#define __MT_HDMI_H__
#include "mt_unf_hdmi.h"
#include "mt_unf_edid.h"

struct HDMI_ARGS_S {
    MT_UNF_HDMI_ID_E enHdmi;
	MT_UNF_HDMI_ATTR_S hdmiAttr;
};

typedef void (*pfnHdmiUserCallback_t)(MT_UNF_HDMI_EVENT_TYPE_E event, mt_void *pPrivateData);

mt_s32 mt_hdmi_init(MT_UNF_HDMI_ID_E enHDMIId, MT_UNF_ENC_FMT_E enWantFmt);

mt_s32 mt_hdmi_setHdcp(mt_u8 hdcp_level);

#endif
