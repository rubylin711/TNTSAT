/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef __MT_OS_IMPL_H__
#define __MT_OS_IMPL_H__

struct SOsTaskId {
	unsigned int magic_os;
	pthread_t tid;
};

#endif
