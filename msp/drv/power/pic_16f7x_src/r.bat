/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

@echo off
(for /f "tokens=2" %%i in (standby_ap_switch_test.rom) do echo %%i) > .\..\standby_ap_switch_test.rom
pause
