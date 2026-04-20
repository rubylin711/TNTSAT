/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include "mt_common.h"

#define MODE_MT_MSLEEP 0
#define MODE_NANOSLEEP 1

static void test_msleep(int mode, unsigned int msecs)
{
	if (mode == MODE_MT_MSLEEP) {
		mt_msleep(msecs);
	} else {
		struct timespec req = {0};
		req.tv_sec  = msecs / 1000;
		req.tv_nsec = (msecs % 1000) * 1000000;
		while (nanosleep(&req, &req) < 0 && errno == EINTR);
	}
}

int main(int argc, char **argv)
{
	int mode = atoi(argv[1]);

	while (1) {
		test_msleep(mode, 1);
	}

	return 0;
}
