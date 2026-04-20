/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __INSTRUMENT_FUNCTION__
#define __INSTRUMENT_FUNCTION__

#define no_instrument_function __attribute__((no_instrument_function))

extern void __cyg_profile_func_enter(void *this_fn, void *call_site) no_instrument_function;
extern void __cyg_profile_func_exit(void *this_fn, void *call_site) no_instrument_function;

#endif /* __INSTRUMENT_FUNCTION__ */
