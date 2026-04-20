/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 201, Montage Technology Co., Ltd.
 *
 * File Name      : main.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2019/03/14
 * Description    : Monage-LZ SW Player main.
 * History        :
 * 1.Date         : 2019/03/14
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <stdio.h>

#include "mtlz_types.h"
#include "mtlzplayer.h"

int main (int argc, char ** argv)
{
	int ret;
	int player;

	if (argc < 2)
	{
		printf("Usage: %s file\n",argv[0]);
		printf("  e.g. %s mpeg2_422.m2v\n",argv[0]);
		return 1;
	}

	player = mtlzplayer_open(argv[1]);
	if (player < 0)
	{
		printf("[ERROR] player open failed!\n");
		return 1;
	}

	ret = mtlzplayer_start(player);
	if (ret != MTLZ_SUCCESS)
	{
		printf("[ERROR] player start failed!\n");
		goto PLAYER_CLOSE;
	}

	while (mtlzplayer_get_status(player) != MTLZ_PLAYER_STATUS_EOS)
		mtlz_msleep(100);

	ret = mtlzplayer_stop(player);
	if (ret != MTLZ_SUCCESS)
	{
		printf("[ERROR] player stop failed!\n");
	}

PLAYER_CLOSE:
	ret = mtlzplayer_close(player);
	if (ret != MTLZ_SUCCESS)
	{
		printf("[ERROR] player close failed!\n");
	}

	return ret;
}

