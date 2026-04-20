/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "hw_ce_common.h"

void hw_ce_data_print(char *string, u8 *data, u32 length, u8 align)
{
	u32 i;

	if (NULL != string)
	{
		printk("%s \n", string);
	}

	if (NULL != data)
	{
		for(i=0; i<length; i++)
		{
			printk("%02x ", data[i]);
			if(0 == ((i+1) & (align-1)))
			{
				printk("\n");
			}
		}
		printk("\n");
	}
}

void hw_ce_msdelay_customize(u32 ms)
{
#if 0
	U32  tick_mark;
	tick_mark = tick;
	while((tick-tick_mark) < ms)
	{
		tick++;
	}
#elif 0
	u32 cnt = 0;
	while(cnt < ms)
	{
		cnt++;
	}
#else
	udelay(ms);
#endif
}
