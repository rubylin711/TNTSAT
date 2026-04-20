/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include "instrument_function.h"

void __cyg_profile_func_enter(void *this_fn, void *call_site)
{
	//printf("enter: 0x%x, 0x%x\n", (unsigned int)this_fn, (unsigned int)call_site);
}

void __cyg_profile_func_exit(void *this_fn, void *call_site)
{
	//printf("exit: 0x%x, 0x%x\n", (unsigned int)this_fn, (unsigned int)call_site);
}
