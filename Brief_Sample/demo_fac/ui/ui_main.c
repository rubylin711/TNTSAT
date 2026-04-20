/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "ui_manager.h"

mt_s32 demo_ui_init(void)
{
	mt_s32 ret = MT_SUCCESS;
	printf("%s(line: %d), init surface\n", __FUNCTION__, __LINE__);
	ret = manage_surface_init();
	if (MT_SUCCESS != ret)
	{
		printf("%s(line: %d), manage_surface_init failed, ret = 0x%x.\n", __FUNCTION__, __LINE__, ret);
		return ret;
    }
	printf("%s(line: %d), create menu.\n", __FUNCTION__, __LINE__);
	manage_menu_init();
	printf("%s(line: %d), update ui.\n", __FUNCTION__, __LINE__);
	manage_update_ui(UPDATE_ALL, 0);

	return ret;
}

