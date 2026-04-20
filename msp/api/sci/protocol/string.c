/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "globals.h"

char *cs_hexdump(int32_t m, const uchar *buf, int32_t n, char *target, int32_t len)
{
	int32_t i = 0;
	target[0] = '\0';
	m = m ? 3 : 2;
	if(m * n >= len)
		{ n = (len / m) - 1; }
	while(i < n)
	{
		snprintf(target + (m * i), len - (m * i), "%02X%s", *buf++, m > 2 ? " " : "");
		i++;
	}
	return target;
}
