/* This file is the implementation of the Nagra trusted framework layers
 * interface on the GP TEE standard. It defines the Nagra trusted client
 * interface adapter related to the Nagra trusted application integration
 * model. It assumes that the Nagra libraries are the only featured elements
 * linked in the trusted application. It provide also the Nagra trusted
 * client context interface adapter. It uses the trusted application functions
 * as well as the memory instance data functions defined in tee_internal_api.h
 * GP TEE internal header.
 *
 * This file must compiled and linked on the REE side.
 *
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated
 * companies. All rights reserved.
 *
 * Copyright 2015 Nagravision S.A.
 *
 * This program is confidential and proprietary to Montage Technology Group
 * Limited and its affiliated companies(Montage), and may not be copied,
 * reproduced, modified, disclosed to others, published or used, in whole or
 * in part, without the express prior written permission of Montage.
 */

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#ifdef _NV_REMAP_DEFS_
# error ISO C99 definitions must be used.
#endif

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tee_client_api.h>
#include "mt_common.h"
#include "nv_tflts.h"
#include "nvta_tflts_gptee.h"

//#define LOCAL_CMA_MEMREF

#define NAGRA_TA_UUID \
	{ 0xBC2F95BC, 0x14B6, 0x4445, \
		{ 0xA4, 0x3C, 0xA1, 0x79, 0x6e, 0x7C, 0xAC, 0x31} }

#define TA_HELLO_WORLD_UUID \
	{ 0x8aaaf200, 0x2450, 0x11e4, \
		{ 0xab, 0xe2, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }


#define DMSG(fmt, ...)   //printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define EMSG(fmt, ...)   printf("[ERR]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define TA_IRD_SMP_CMD_RW_IO 0x0000001F
#define   STRNUB       4096

MT_BOOL MTCommandGetArgc(const char* pInput, char* pOutput, int argcNum)
{
	char* ptr = (char*)pInput;
	char  curNum=0;
	char  buffer[STRNUB];
	char* tp=NULL;

	while(*ptr)
	{
		while(*(ptr) == 0x20)  //delete all space char
		{
			ptr++;
		}
		curNum++;

		tp = buffer;
		while((*ptr!=0) && (*ptr!=0x20))  //null or space char
		{
			*(tp++) = *(ptr++);
		}
		*tp = 0; //add null at the end

		if(*buffer == 0)
			return MT_FAILURE;
		if(curNum == argcNum)
		{
			strcpy(pOutput,buffer);
			return MT_SUCCESS;
		}
	}
	return MT_FAILURE;
}

static void ird_sec_io_rw(void *cmd)
{
	TEEC_Result res;
    TEEC_Context ta_ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = TA_HELLO_WORLD_UUID;
	uint32_t err_origin;
    char sub_cmd[5][16];
    uint32_t option;
    uint32_t addr;
    uint32_t value;
    uint32_t wlen;
    TEEC_Operation operation;
    uint32_t select = 555;

    MTCommandGetArgc(cmd, sub_cmd[0], 1);
    MTCommandGetArgc(cmd, sub_cmd[1], 2);
    MTCommandGetArgc(cmd, sub_cmd[2], 3);
    MTCommandGetArgc(cmd, sub_cmd[3], 4);

    if (!strcmp(sub_cmd[0], "read") || !strcmp(sub_cmd[0], "r")) {
        option = 0;
    } else if (!strcmp(sub_cmd[0], "write") || !strcmp(sub_cmd[0], "w")) {
        option = 1;
    } else {
        printf("error arg, [%s]\n", (char *)cmd);
        return;
    }

    addr = (uint32_t)strtoul(sub_cmd[1], (char **)NULL, 16);
    value = (uint32_t)strtoul(sub_cmd[2], (char **)NULL, 16);
    wlen = (uint32_t)strtoul(sub_cmd[3], (char **)NULL, 16);
    wlen = wlen == 0 ? 4 : wlen;

	res = TEEC_InitializeContext(NULL, &ta_ctx);
	if (res != TEEC_SUCCESS)
		printf("TEEC_InitializeContext failed with code 0x%x", res);

	memset(&operation, 0, sizeof(TEEC_Operation));
	operation.started = 0;

	operation.paramTypes =
	TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
				 TEEC_NONE,
				 TEEC_NONE,
				 TEEC_NONE);
	operation.params[0].value.a = select;
	operation.params[0].value.b = 0;

	res = TEEC_OpenSession(&ta_ctx, &sess, &uuid,
				   TEEC_LOGIN_PUBLIC, NULL, &operation, &err_origin);
	if (res != TEEC_SUCCESS)
		printf("TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
	else
	printf("\n TEEC_OpenSession open success!\n");

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].value.a = option;
	op.params[0].value.b = addr;
	op.params[1].value.a = value;
	op.params[1].value.b = wlen;

	printf("\n TEEC_InvokeCommand!\n");

	res = TEEC_InvokeCommand(&sess, TA_IRD_SMP_CMD_RW_IO, &op, &err_origin);
	if (res != TEEC_SUCCESS)
		printf("TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
	else
	printf("\n TEEC_InvokeCommand end !\n");

	TEEC_CloseSession(&sess);
	printf("\n TEEC_CloseSession !\n");

    TEEC_FinalizeContext(&ta_ctx);

		printf("\n TEEC_FinalizeContext !\n");

}


#define MAX_CMDLINE_LEN 1280
static mt_char g_CmdLine[MAX_CMDLINE_LEN];

mt_s32 main(mt_s32 argc, mt_char *argv[])
{
	mt_char *pCmdLine=NULL;

	printf("hello debug ~~~~~~~~~~~~\n");

	while(1)
	{
		 printf("\n>");
	        pCmdLine = fgets(g_CmdLine, MAX_CMDLINE_LEN, stdin);
	        if (MT_NULL == pCmdLine)
		 {
	            continue;
	        }

	        printf("pCmdLine = %s\n", pCmdLine);
		 ird_sec_io_rw(pCmdLine);
	}
}

/* ========================================================================== */
/*                                END OF FILE                                 */
/* ========================================================================== */


