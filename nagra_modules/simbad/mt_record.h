/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef __MT_RECORD_H__
#define __MT_RECORD_H__
#include "mt_common.h"
#include "mt_spr_ext.h"

mt_s32 mt_record_start(struct transportSession *ts, mt_char *filename, mt_u16 emi, mt_u32 vidPid, mt_u32 audPid);

mt_s32 mt_record_stop(struct transportSession *ts);

#endif
