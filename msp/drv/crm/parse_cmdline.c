/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2022 Montage LZ Technology Group Limited and its affiliated companies      */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2022, Montage LZ Technology Co., Ltd.
 *
 * File Name      : parse_cmdline.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2022/06/29
 * Description    : Parse cmd line for clock config.
 * History        :
 * 1.Date         : 2022/06/29
 *   Author       : LZ100218
 *   Modification : Created file
 *
*****************************************************************************/
#if defined(__UBOOT__)
#include <common.h>
#include <asm/arch-symphony6/mt_common.h>
#elif defined(__KERNEL__)
#include <linux/string.h>

#include "mt_log.h"
#include "mt_drv_clock.h"

#define CONFIG_SYS_MAXARGS	64
#define CONFIG_SYS_CBSIZE 	256
#endif

/* cmd line seperator */
#define C_SEPERATOR			','

/* local arguments */
static int l_argc;

/* local cmd line string buffer */
static char l_cmd_line[CONFIG_SYS_CBSIZE];

/**
 * parse cmd line to argc and argv.
 *
 * @param[out] argc/argv output arguemnts
 *
 * @param[in] cmd_line cmd line, such as:
 *    "clk=?,gate=[0|1],rate=?,mux=?,div=?,attr=?"
 *
 * @retval
 *    <0: error,
 *    others: argc
 */
int parse_cmdline(int OUT *argc, char OUT **argv, const char IN *cmd_line)
{
	char *begin;
	char *next;

	CHECK_NULL_PTR(argc);
	CHECK_NULL_PTR(argv);
	CHECK_NULL_PTR(cmd_line);

	*argc = 0;

	if (strlen(cmd_line) > (sizeof(l_cmd_line) - 1))
	{
//		PRINTF("error: cmd line string(%lu) overflow!\n", strlen(cmd_line));
		return (-1);
	}

	strncpy(l_cmd_line, cmd_line, sizeof(l_cmd_line)-1);
	l_cmd_line[sizeof(l_cmd_line)-1] = '\0';

	begin = NULL;
	next = l_cmd_line;
	l_argc = 0;

	for (;;)
	{
		if (*next == 0x20
			|| *next == C_SEPERATOR
			|| *next == '\0'
			|| *next == '\r'
			|| *next == '\n')
		{
			if (l_argc < CONFIG_SYS_MAXARGS)
			{
				if (begin != NULL)
				{
					argv[l_argc++] = begin;
				}
				else
				{
					/* all space */
				}
			}
			else
			{
				PRINTF("error: cmd line argc(%d) overflow!\n", l_argc);
				return (-1);
			}

			/* complete */
			if (*next == '\0'
				|| *next == '\r'
				|| *next == '\n')
			{
				*next = '\0';
				*argc = l_argc;
				return l_argc;
			}

			*next = '\0';
			begin = NULL;
		}
		else if (*next > 0x20 && *next < 0x80)
		{
			if (begin == NULL)
				begin = next;
		}
		else
		{
			/* invalid char */
		}

		next ++;
	}
}

