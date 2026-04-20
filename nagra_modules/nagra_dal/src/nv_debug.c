/*
 * @file nv_debug.c
 * @brief Nagravision Stream Processing APIs implemetation on Montage Symphony4 platform
 *
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "nv_debug.h"

#define EMSG(fmt, ...)   printf("[ERR]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DMSG(fmt, ...)   printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

FILE *gLogFile = NULL;
static const char *sLogFileName = "/tmp/nv_debug.log";
uint8_t gSimbadTerminated = 0;

void nvLog
(
 const char*  pxMessage
)
{
#if 0
	if (gSimbadTerminated == 0) {
		if (gLogFile == NULL) {
			gLogFile = fopen(sLogFileName, "w");
			if (gLogFile == NULL) {
				printf("[FATAL-ERROR]log file open failed, please check!\n");
				return;
			}
		}

		//Implement for debug simbad libs
		if (pxMessage)
			fputs(pxMessage, gLogFile);
	}
#endif
	printf("%s\n", pxMessage);
}

uint32_t nvGetTickCount
(
 void
)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	/*to microseconds*/
	return (ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

int nvGetChar
(
 void
)
{
	char ch;

	//Read from stdin
	while (1) {

		if (read(STDIN_FILENO, &ch, 1) > 0) {
			return (int)ch;
		} else {
			return -1;
		}
	}
}

INvDebug gINvDebugFuncTable = {
	DEBUGAPI_VERSION_INT,
	nvLog,
	nvGetTickCount,
	nvGetChar,
};

const INvDebug* nvGetDebugInterface
(
  void
)
{
	return &gINvDebugFuncTable;
}
