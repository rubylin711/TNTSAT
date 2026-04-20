/*
 * @file nv_spr.c
 * @brief Nagravision Stream Processing APIs implemetation on Montage Symphony4 platform
 *
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */
#if 1
#include <err.h>
#include <ngwm_ree.h>
#include <stdio.h>
#include <string.h>
#include <tee_client_api.h>
#include <teec_ngwm.h>

#if 0
#define NGWM_REE_DBG(fmt, ...)   printf("[NGWM]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define NGWM_REE_DBG(fmt, ...)      do {} while(0)
#endif

#define TA_MT_NXGD_RT_VALUE     (0x55AAAA55)

static uint32_t ngwm_configure(const uint8_t* config, uint32_t size)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = CA_NGWM_OPS_UUID;
	uint32_t err_origin;
       NGWM_REE_DBG(" %s, %d, uuid.timeLow = 0x%x\n", __FUNCTION__, __LINE__, uuid.timeLow);
	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_InitializeContext failed with code 0x%x", res);
		goto out;
	}

	/*
	 * Open a session to the "nexguard" TA
	 */
	NGWM_REE_DBG("REE: ngwm_configure:TEEC_OpenSession\n");
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);
		goto finalize;

	}

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_MEMREF_TEMP_INPUT,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = false;	/* Indicate calling from ree */
	op.params[0].value.b = 0;
	op.params[1].tmpref.buffer = (void *)config;
	op.params[1].tmpref.size = size;

	/*
	 * PTA_CMD_REG_OPS is the actual function in the TA to be
	 * called.
	 */
	NGWM_REE_DBG("REE: ngwm_configure: TEEC_InvokeCommand==\n");
	res = TEEC_InvokeCommand(&sess, TEEC_NGWM_CMD_CONFIG, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
		goto close;
	}
	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

close:
	TEEC_CloseSession(&sess);
finalize:
	TEEC_FinalizeContext(&ctx);
out:
	return res;
}


static  uint32_t ngwm_configure_by_pipe(uint32_t pipe_source_id, const uint8_t* config,
			uint32_t  size)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = CA_NGWM_OPS_UUID;
	uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_InitializeContext failed with code 0x%x", res);
		goto out;
	}

	/*
	 * Open a session to the "nexguard" TA
	 */
	NGWM_REE_DBG("REE: ngwm_configure_by_pipe:TEEC_OpenSession\n");
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);
		goto finalize;

	}
	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,TEEC_VALUE_INPUT,
						TEEC_MEMREF_TEMP_INPUT,TEEC_NONE);
	op.params[0].value.a = false;	/* Indicate calling from ree */
	op.params[0].value.b = 0;
	op.params[1].value.a = pipe_source_id;
	op.params[1].value.b = 0;
	op.params[2].tmpref.buffer = (void *)config;
	op.params[2].tmpref.size = size;

	/*
	 * PTA_CMD_REG_OPS is the actual function in the TA to be
	 * called.
	 */
	NGWM_REE_DBG("REE: ngwm_configure_by_pipe:TEEC_InvokeCommand\n");
	res = TEEC_InvokeCommand(&sess, TEEC_NGWM_CMD_CONFIG_BYPIPE, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
		goto close;
	}
	/*
	 * We're done with the TA, close the session and
	 * destroy the context.
	 *
	 * The TA will print "Goodbye!" in the log when the
	 * session is closed.
	 */

close:
	TEEC_CloseSession(&sess);
finalize:
	TEEC_FinalizeContext(&ctx);
out:
	return res;
}

uint32_t mt_ngwm_configure_mainID(uint32_t main_tsid)
{
	TEEC_Result res;
	TEEC_Context ctx;
	TEEC_Session sess;
	TEEC_Operation op;
	TEEC_UUID uuid = CA_NGWM_OPS_UUID;
	uint32_t err_origin;

	/* Initialize a context connecting us to the TEE */
	res = TEEC_InitializeContext(NULL, &ctx);
	if (res != TEEC_SUCCESS) {
		printf( "TEEC_InitializeContext failed with code 0x%x", res);
		goto out;
	}

	/*
	 * Open a session to the "nexguard" TA
	 */
	//NGWM_REE_DBG("REE: ngwm_configure:TEEC_OpenSession\n");
	res = TEEC_OpenSession(&ctx, &sess, &uuid,
			       TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_Opensession failed with code 0x%x origin 0x%x",
			res, err_origin);
		goto finalize;

	}

	/* Clear the TEEC_Operation struct */
	memset(&op, 0, sizeof(op));

	/*
	 * Prepare the argument. Pass a value in the first parameter,
	 * the remaining three parameters are unused.
	 */
	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_OUTPUT,
					 TEEC_NONE, TEEC_NONE);
	op.params[0].value.a = main_tsid;	/* Indicate the tsid of main screen  */


	/*
	 * PTA_CMD_REG_OPS is the actual function in the TA to be
	 * called.
	 */
	//printf("REE: ngwm_configure: TEEC_InvokeCommand\n");
	res = TEEC_InvokeCommand(&sess, TEEC_NGWM_CMD_CONFIG_MAINID, &op,
				 &err_origin);
	if (res != TEEC_SUCCESS) {
		printf("TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
			res, err_origin);
		goto close;
	}
       if ((op.params[1].value.a != TA_MT_NXGD_RT_VALUE) || (op.params[1].value.b != main_tsid)) {
            printf("REE: ERROR :0x%x, 0x%x; expect: 0x%x, 0x%x \n",
                    op.params[1].value.a, op.params[1].value.b, TA_MT_NXGD_RT_VALUE, main_tsid);
            res = TEE_ERROR_MAC_INVALID;
       }

close:
	TEEC_CloseSession(&sess);
finalize:
	TEEC_FinalizeContext(&ctx);
out:
	return res;
}

static const INgwmRee ngwm_ree= {
	.version	= NGWMREEAPI_VERSION_INT,
	.configure	= ngwm_configure,
	.configureByPipe	= ngwm_configure_by_pipe,
};

const INgwmRee* ngwmGetReeInterface(void)
{
	return &ngwm_ree;
}
#endif