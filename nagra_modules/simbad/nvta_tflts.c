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
//#include <linux/delay.h>
#include <sys/mman.h>

#define LOCAL_CMA_MEMREF

#define NAGRA_TA_UUID \
	{ 0xBC2F95BC, 0x14B6, 0x4445, \
		{ 0xA4, 0x3C, 0xA1, 0x79, 0x6e, 0x7C, 0xAC, 0x31} }

#define DMSG(fmt, ...)   //printf("[MT_DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define EMSG(fmt, ...)   //printf("[MT_ERR]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)


/* ========================================================================== */
/*                          TRUSTED SESSION ADAPTERS                          */
/* ========================================================================== */

/*  Local functions wrappers */
static uint32_t nv_open(void **ppSession, uint32_t select);
static void nv_close(void *pSession);
static uint32_t nv_command(void *pSession, uint32_t nBlocks, TNvTrustedBlock *pBlocks);


/* -------------------------------------------------------------------------- */
/*                  NAGRA TRUSTED SESSION INTERFACE STRUCTURE                 */
/* -------------------------------------------------------------------------- */
const INvTrustedSession *nvGetTrustedSessionInterface(void)
{
	static const INvTrustedSession iTrustedSession = {
		TFLAPI_VERSION_INT,
		nv_open,
		nv_close,
		nv_command
	};

	return &iTrustedSession;
}

/* ========================================================================== */
/*                           PRIVATE LOCAL FUNCTIONS                          */
/* ========================================================================== */

static pthread_mutex_t *p_tfl_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutexattr_t tfl_mutex_attr;
#define tflLock() (void) pthread_mutex_lock(p_tfl_mutex);
#define tflUnlock() (void) pthread_mutex_unlock(p_tfl_mutex);

/*
static pthread_mutex_t tfl_mutex = PTHREAD_MUTEX_INITIALIZER;
#define tflLock() (void) pthread_mutex_lock(&tfl_mutex);
#define tflUnlock() (void) pthread_mutex_unlock(&tfl_mutex);
*/

static uint32_t convertParamType(uint32_t nvParamType);
static uint32_t convertGpTeeError(TEEC_Result gpError);


void mt_tfl_command_mutext_init()
{

    p_tfl_mutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,  MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    pthread_mutexattr_init(&tfl_mutex_attr);
    pthread_mutexattr_setpshared(&tfl_mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(p_tfl_mutex, &tfl_mutex_attr);

}


/* ========================================================================== */
/*                                PRIVATE DATA                                */
/* ========================================================================== */

/*
 * Store the GP TEE context provided to the GP TEE adapter with nvGpTeeConfigure().
 */
static TEEC_Context* gGpContext = NULL;

#define MT_NVTA_PRE_SIZE_DEFAULT    (4 * 1024)
#define MT_NVTA_BLOCK_MAX   (4)

struct MT_NVTA_CMA_infor
{

    phys_addr_t cma_pre_addr_phy;
    phys_addr_t cma_pre_addr_vir;
    uint32_t cma_pre_size;
};
static struct MT_NVTA_CMA_infor mt_nvta_cma_infor[MT_NVTA_BLOCK_MAX] = {0,};

#if 0
//max 4 blocks
static phys_addr_t cma_pre_addr_phy[4] = {0};
static phys_addr_t cma_pre_addr_vir[4] = {0};
 /*
* Using static CMA blocks
*/
static uint32_t cma_pre_size[4] = {0,};
#endif


/* -------------------------------------------------------------------------- */
/*                              nvGpTeeConfigure                              */
/* -------------------------------------------------------------------------- */

/*
 * This function is a specific additional entry points of the Nagra trusted
 * session adapter over GP TEE interfaces (related to the Nagra trusted
 * application integration model).
 *
 * The implementation of this function store the provided GP TEE context in
 * the gGpContext adapter global variable. The provided TEE context must have
 * been created by the caller using TEEC_InitializeContext() GP function.
 */
void nvGpTeeConfigure(TEEC_Context* pxTeeContext)
{
	gGpContext = pxTeeContext;

	DMSG("====GpTeeConfigure called====  pxTeeContext = %p gGpContext = %x\n", pxTeeContext, gGpContext);
}

/* -------------------------------------------------------------------------- */
/*                                   nv_open                                  */
/* -------------------------------------------------------------------------- */

/* Adapt the INvTrustedSession.open() interface function to the
 * GP TEEC_OpenSession() function.
 *
 * The parameters of the GP TEE call are mapped as follow:
 * - The context parameter is the the GP TEE context previously provided withi
 *   nvGpTeeConfigure().
 * - The session parameter is dynamically allocated and is map on the ppSession
 *   interface parameter.
 * - The destination UUID parameter is assigned to a Nagra TA UUID.
 *   It has been arbitrarily defined as bc2f95bc-14b6-4445-a43c-a1796e7cac31.
 *   It assumes that the targets trusted application features only Nagra trusted client.
 * - The connection parameters - connectionMethod and connectionData - are
 *   defaulted to the GP TEE public login.
 *   Refer to GP TEE TEEC_LOGIN_PUBLIC description.
 * - The optional operation parameter is used to transmit the select interface parameter.
 *   The provided TEEC_Operation structure defines only one input integer parameter.
 *   The first integer is assigned with the select value.
 * - The optional returnOrigin parameter is not used and is set to NULL.
 *
 * The error returned by TEEC_OpenSessiont() call is converted to
 * TNvTrustedResult using the local convertGpTeeError() function.
 */
static uint32_t nv_open(void **ppSession, uint32_t select)
{
	static const TEEC_UUID lNagraUuid = NAGRA_TA_UUID;

	uint32_t result = NV_TRUSTED_ERROR_BAD_PARAMETER;
	TEEC_Operation operation;
	TEEC_Result gpStatus = TEEC_ERROR_GENERIC;

	DMSG("%s %d nv_open ppSession = %x, select = %d\n",__FUNCTION__,__LINE__, ppSession, select);

	/* Checks parameters */
	if (!ppSession)
	{
		EMSG("[err]nv_open NV_TRUSTED_ERROR_BAD_PARAMETER \n");
		return NV_TRUSTED_ERROR_BAD_PARAMETER;
	}

	/* Check context has been configured */
	if (!gGpContext)
	{
		EMSG("[err]nv_open NV_TRUSTED_ERROR  gGpContext == NULL  nv_spr init fail!\n");
		return NV_TRUSTED_ERROR;
	}

	/* Allocate GP session */

	result = NV_TRUSTED_ERROR_CLIENT_MEMORY;
	*ppSession = malloc(sizeof(TEEC_Session));

	if (NULL != *ppSession) {
		result = NV_TRUSTED_ERROR;

		/*
		 * Define first operation i.e. select transport
		 * assumed for this implementation
		 */

		memset(&operation, 0, sizeof(TEEC_Operation));
		operation.started = 0;

		operation.paramTypes =
			TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					 TEEC_NONE,
					 TEEC_NONE,
					 TEEC_NONE);
		operation.params[0].value.a = select;
		operation.params[0].value.b = 0;

		/* Do the GP call */

		tflLock();
		DMSG("%s %d gGpContext = %x \n",__FUNCTION__,__LINE__,gGpContext);

		gpStatus = TEEC_OpenSession(gGpContext,
					    (TEEC_Session*)*ppSession,
					    &lNagraUuid,
					    TEEC_LOGIN_PUBLIC,
					    NULL,
					    &operation,
					    NULL);
		DMSG("%s %d gpStatus = %d  \n",__FUNCTION__,__LINE__,gpStatus);

		tflUnlock();

		/* Analyze results */
		if (TEEC_SUCCESS != gpStatus) {
			EMSG("[ERROR]  gpStatus = %x  \n", gpStatus);

			free( *ppSession );
			*ppSession = NULL;
			//while(1);
		}

		result = convertGpTeeError(gpStatus);

	}

	DMSG("nv_open result = %d \n", result);

	return result;
}

/* -------------------------------------------------------------------------- */
/*                                  nv_close                                  */
/* -------------------------------------------------------------------------- */

/*  Adapt the INvTrustedSession.close() interface function to the
 *  GP TEEC_CloseSession() function.
 */
static void nv_close(void *pSession)
{
	int i = 0;
       mt_s32 ret = MT_SUCCESS;

	DMSG("%s %d \n",__FUNCTION__,__LINE__);

	/* Do the GP call */
	tflLock();
	TEEC_CloseSession((TEEC_Session *)pSession);


	if (NULL != pSession) {
		free(pSession);
	}

#ifdef LOCAL_CMA_MEMREF
	/* Free CMA */
	for (i = 0; i < 4; i++) {
		if (mt_nvta_cma_infor[i].cma_pre_addr_phy != 0) {
			DMSG("%s %d  cma_pre_addr_phy[%d] = %x\n",__FUNCTION__,__LINE__, i, mt_nvta_cma_infor[i].cma_pre_addr_phy);
			ret = mt_mmz_unmap((void*)mt_nvta_cma_infor[i].cma_pre_addr_vir);
			ret |= mt_mmz_delete(mt_nvta_cma_infor[i].cma_pre_addr_phy);
			mt_nvta_cma_infor[i].cma_pre_addr_phy = 0;
                    mt_nvta_cma_infor[i].cma_pre_addr_vir = 0;
                    mt_nvta_cma_infor[i].cma_pre_size = 0;
		}
	}
#endif
    tflUnlock();

}

static uint32_t convertMemrefType(uint32_t type)
{
	switch (type) {
	case TEEC_MEMREF_PARTIAL_INPUT:
		return TEEC_MEM_INPUT;
	case TEEC_MEMREF_PARTIAL_OUTPUT:
		return TEEC_MEM_OUTPUT;
	case TEEC_MEMREF_PARTIAL_INOUT:
		return (TEEC_MEM_INPUT | TEEC_MEM_OUTPUT);
	default:
		return TEEC_MEM_INPUT;
	}
}

#define CACHE_LINE	(128)

static TEEC_SharedMemory *teec_cma_malloc_and_copy(uint32_t size, void *pAddr, uint32_t type, uint32_t i)
{
    void *vir_addr;
    phys_addr_t phy_addr;
    TEEC_SharedMemory *pShm = NULL;

    uint32_t flush_size = 0;

    if (size == 0) {
        return NULL;
    }
	flush_size = ((size + (CACHE_LINE - 1)) / CACHE_LINE) * CACHE_LINE;

       DMSG("size = 0x%x, flush_size = 0x%x \n", size, flush_size);
	if (mt_nvta_cma_infor[i].cma_pre_addr_phy == 0) {
		mt_nvta_cma_infor[i].cma_pre_addr_phy = mt_mmz_new(MT_NVTA_PRE_SIZE_DEFAULT, 0, "ddr", "teec_shm");
		if (mt_nvta_cma_infor[i].cma_pre_addr_phy == 0) {
			EMSG("[%s:%d]new mmz buffer failed\n", __FUNCTION__, __LINE__);
			return NULL;
		}
              mt_nvta_cma_infor[i].cma_pre_size = MT_NVTA_PRE_SIZE_DEFAULT;
		DMSG("%s %d  cma_pre_addr_phy[%d] = %x \n",__FUNCTION__,__LINE__, i, mt_nvta_cma_infor[i].cma_pre_addr_phy);

		mt_nvta_cma_infor[i].cma_pre_addr_vir = (phys_addr_t)mt_mmz_map(mt_nvta_cma_infor[i].cma_pre_addr_phy, 1);
		if (mt_nvta_cma_infor[i].cma_pre_addr_vir == 0) {
			EMSG("dzb mt_mmz_map fail cma_pre_addr_phy[%d]= %x cma_pre_addr_vir[%d]=%x \n",
                            i, mt_nvta_cma_infor[i].cma_pre_addr_phy, i, mt_nvta_cma_infor[i].cma_pre_addr_vir);
			mt_mmz_delete(mt_nvta_cma_infor[i].cma_pre_addr_phy);
			return NULL;
		}
		DMSG("%s %d i = %d cma_pre_addr_vir[i]=%x ma_pre_addr_phy[i]=%x \n",__FUNCTION__,__LINE__,
                i, mt_nvta_cma_infor[i].cma_pre_addr_vir, mt_nvta_cma_infor[i].cma_pre_addr_phy);
	}

	if (size > mt_nvta_cma_infor[i].cma_pre_size) {

		mt_mmz_unmap((void *)mt_nvta_cma_infor[i].cma_pre_addr_phy);
		mt_mmz_delete(mt_nvta_cma_infor[i].cma_pre_addr_phy);
		mt_nvta_cma_infor[i].cma_pre_addr_phy = mt_mmz_new(flush_size, 0, "ddr", "teec_shm");
		if (mt_nvta_cma_infor[i].cma_pre_addr_phy == 0) {
			EMSG("[%s:%d]new mmz buffer failed\n", __FUNCTION__, __LINE__);
			return NULL;
		}
             mt_nvta_cma_infor[i].cma_pre_size = flush_size;
		DMSG("%s %d i = %d  cma_pre_addr_phy[i] = %x \n",__FUNCTION__,__LINE__, i, mt_nvta_cma_infor[i].cma_pre_addr_phy);

		mt_nvta_cma_infor[i].cma_pre_addr_vir = (phys_addr_t)mt_mmz_map(mt_nvta_cma_infor[i].cma_pre_addr_phy, 1);
		//mt_nvta_cma_infor[i].cma_pre_addr_vir = (void*)mt_mmz_map(mt_nvta_cma_infor[i].cma_pre_addr_phy, 0);
		if (mt_nvta_cma_infor[i].cma_pre_addr_vir == 0) {
			EMSG(" dzb mt_mmz_map fail cma_pre_addr_phy[%d]= %x cma_pre_addr_vir[%d]=%x \n",
                            i, mt_nvta_cma_infor[i].cma_pre_addr_phy, i, mt_nvta_cma_infor[i].cma_pre_addr_vir);
			mt_mmz_delete(mt_nvta_cma_infor[i].cma_pre_addr_phy);
                    mt_nvta_cma_infor[i].cma_pre_addr_phy = 0;
			return NULL;
		}
	}

	phy_addr = mt_nvta_cma_infor[i].cma_pre_addr_phy;
	vir_addr = (void*)mt_nvta_cma_infor[i].cma_pre_addr_vir;
       memset(vir_addr, 0x00, flush_size);

#if 0 //use static mmz buffer, no need to dynamic allocate
    phy_addr = (mt_u32)mt_mmz_new(size, 0, "ddr", "teec_shm");
    if (phy_addr == 0) {
		DMSG("[%s:%d]new mmz buffer failed\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    //map,and cached
    vir_addr = (mt_u32)mt_mmz_map(phy_addr, 1);
    if (vir_addr == 0) {
        mt_mmz_delete(phy_addr);
		DMSG("[%s:%d]map mmz buffer failed\n", __FUNCTION__, __LINE__);
        return NULL;
    }
#endif

	pShm = (TEEC_SharedMemory *)malloc(sizeof(TEEC_SharedMemory));
	if (pShm == NULL) {
		EMSG("[%s:%d]malloc SHM buffer failed\n", __FUNCTION__, __LINE__);
		mt_mmz_unmap((void *)phy_addr);
		mt_mmz_delete(phy_addr);
		return NULL;
	}
	memset(pShm, 0x0, sizeof(TEEC_SharedMemory));

	pShm->buffer = (void *)((mt_u64)vir_addr);
	pShm->size = size;
	pShm->flags = convertMemrefType(type);
	memcpy(pShm->buffer, pAddr, size);
	mt_mmz_flush((void *)((mt_u64)vir_addr), 0, flush_size);

    return pShm;
}

static mt_s32 teec_cma_free_shm(TEEC_SharedMemory *pShm)
{
     mt_s32 ret = MT_SUCCESS;
     //phys_addr_t phy_addr;
     //ulong phy_size;
     //int i = 0;

      //void *pVir = pShm->buffer; //buffer will be set 0 after TEEC_ReleaseSharedMemory


	TEEC_ReleaseSharedMemory(pShm);
    /* // if delete mmz here, SEC-SPS-2002-0020 will be faied, so move it to nv_close
       ret = mt_mmz_get_phyaddr(pVir, &phy_addr, &phy_size);
	DMSG("%s %d pShm = %x pVir = %x phy_addr = %x\n",__FUNCTION__,__LINE__, pShm, pVir, phy_addr);

	for (i = 0; i < 4; i++) {
		if (mt_nvta_cma_infor[i].cma_pre_addr_phy == phy_addr) {
			DMSG("%s %d  cma_pre_addr_phy[%d] = %x\n",__FUNCTION__,__LINE__, i, mt_nvta_cma_infor[i].cma_pre_addr_phy);
			mt_nvta_cma_infor[i].cma_pre_addr_phy = 0;
		}
	}

    ret |= mt_mmz_unmap((void *)pVir);
    ret |= mt_mmz_delete(phy_addr);

*/
	free(pShm);

	return ret;
}

static mt_s32 teec_cma_copy_back(void *pAddr, uint32_t size, TEEC_SharedMemory *pShm, uint32_t refsize)
{
    mt_s32 ret;
    phys_addr_t phy_addr;
    ulong phy_size;

	uint32_t inv_size = ((size + (CACHE_LINE - 1)) / CACHE_LINE) * CACHE_LINE;

    if (pAddr == NULL || pShm == NULL) {
		/* Invalid memref to be freed */
        DMSG("\n", __FUNCTION__, __LINE__);
        return -1;
    }

	if (pShm->buffer == NULL) {
		/* Already freed */
            DMSG("\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if (refsize > size) {
            DMSG("\n", __FUNCTION__, __LINE__);
		return 0; //Do nothing
	}
    DMSG("pShm->buffer = 0x%p \n", pShm->buffer);
    ret = mt_mmz_get_phyaddr(pShm->buffer, (phys_addr_t *)(&phy_addr), (ulong *)(&phy_size));
	if (ret == MT_SUCCESS) {
            DMSG("inv_size = 0x%x, pShm->buffer=0x%p, size = 0x%x, phy_addr = 0x%x \n", inv_size, pShm->buffer, size, phy_addr);
		mt_mmz_invalidate((void *)((mt_u64)pShm->buffer), 0, inv_size);
		memcpy(pAddr, pShm->buffer, size);
	}
    DMSG("\n", __FUNCTION__, __LINE__);
    return ret;
}




/* -------------------------------------------------------------------------- */
/*                                 nv_command                                 */
/* -------------------------------------------------------------------------- */

/* Adapt the INvTrustedSession.command() interface function to the
 * GP TEEC_InvokeCommand() function.
 *
 * The parameters of the GP TEE call are mapped as follow:
 * + The pSession parameter has been mapped directly on a TEEC_Session object
 *   when provided back by nv_open(). Therefore it can be directly provided
 *   as the session parameter of TEEC_InvokeCommand().
 * + The commandID parameter is not used and is set to 0.
 * + The operation parameter is build from the provided nBlocks and pBlocks
 *   parameters:
 *   - Each provided TNvTrustedBlock memory area is mapped directly on a
 *     params field of the GP operation. It is mapped as a temporary
 *     memory reference. Its pAddr and size fields are mapped directly as
 *     tmpref.buffer and tmpref.size fields of the operation parameter.
 *   - All TNvTrustedBlock directions are consolidated in the paramTypes
 *     field of operation as temporary memory references.
 * + The optional returnOrigin parameter is not used and is set to NULL.
 *
 * When the TEE_InvokeTACommand() returns, the size field of each
 * TNvTrustedBlock defined ad output or input/output is updated with the
 * size field of the related operation parameter.
 *
 * The error returned by TEEC_InvokeCommand() call is converted to
 * TNvTrustedResult using the local convertGpTeeError() function.
 *
 * The commandID parameter is not used.
 * It may be set to a default value stated a Nagra command is issued for
 * debugging purpose.
 */

/*void delay( unsigned long time )
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint32_t k = 0;


	while( time --)
	{
		i++;
		j++;
		k = i+j;
	}
}*/

static uint32_t nv_command(void* pSession,
		uint32_t nBlocks, TNvTrustedBlock* pBlocks)
{
	uint32_t result = NV_TRUSTED_ERROR_BAD_PARAMETER;
	TEEC_SharedMemory *pShm = NULL;

       //DMSG("%s %d pSession = %x nBlocks = %d pBlocks = %x \n",__FUNCTION__,__LINE__, pSession, nBlocks, pBlocks);
       DMSG("grant %d, %d \n", sizeof(TEEC_Parameter), sizeof(TEEC_TempMemoryReference));

	if(pBlocks)
	{
		//DMSG("direction = %d pAddr = 0x%08x size = %d\n", pBlocks->direction, pBlocks->pAddr, pBlocks->size);
	}

	/* Check context has been configured */
	if (!gGpContext)
	{
		EMSG("%s %d NV_TRUSTED_ERROR\n",__FUNCTION__,__LINE__);
		return NV_TRUSTED_ERROR;
	}

	/* Nagra commands provides at least one buffer */
	if (0 != nBlocks && 4 >= nBlocks && NULL != pBlocks) {
		TEEC_Operation operation;
		TEEC_Result gpStatus = TEEC_ERROR_GENERIC;
		uint32_t types[4] = {0,};
		size_t i;

        for (i = 0; i < nBlocks; i++) {
            if (pBlocks[i].pAddr == NULL)
            {
            		EMSG("%s %d result = %d \n",__FUNCTION__,__LINE__, result);
                	return result;
            }

            if (i == 2 && pBlocks[i].size == 0)
            {
            	     EMSG("%s %d result = %d \n",__FUNCTION__,__LINE__, result);
            	     return result;
            }

            if (i == 3 && pBlocks[i].size == 0)
            {
            		EMSG("%s %d result = %d \n",__FUNCTION__,__LINE__, result);
             		return result;
            }


        }

		/* Maps TFL trsuted blocks to the GP TEEC operation */
		for (i = 0; i < 4; i++)

		types[i] = TEEC_NONE;
		memset(&operation, 0, sizeof(TEEC_Operation));

		tflLock();

		for (i = 0; i < nBlocks; i++) {
			//DMSG("%s %d direction = %d \n",__FUNCTION__,__LINE__, pBlocks[i].direction);
			types[i] = convertParamType(pBlocks[i].direction);
			//DMSG("%s %d i = %d \n",__FUNCTION__,__LINE__, i);
			if (TEEC_NONE == types[i])
			{
				EMSG("%s %d i = %d \n",__FUNCTION__,__LINE__, i);
				break;  /* Bad parameter */
			}
#ifdef LOCAL_CMA_MEMREF
			//DMSG("%s %d i = %d  size = %d pBlocks = %x types=%d pBlocks[%d]\n",__FUNCTION__,__LINE__, i, pBlocks[i].size, pBlocks[i].pAddr,types[i], i  );
			pShm = teec_cma_malloc_and_copy(pBlocks[i].size, pBlocks[i].pAddr, types[i], i);
			//DMSG("%s %d i = %d \n",__FUNCTION__,__LINE__, i);
			//DMSG("%s \n",__FUNCTION__);
			//DMSG("c");
			//mdelay(50);
			//msleep(50);
			//MT_USLEEP(1000);
			//delay(10000000);
											//DMSG("c");

			gpStatus = TEEC_RegisterSharedMemory(gGpContext, pShm);

			//DMSG("%s %d i = %d \n",__FUNCTION__,__LINE__, i);
			operation.params[i].memref.parent = pShm;
			operation.params[i].memref.offset = 0;
			operation.params[i].memref.size = pBlocks[i].size;
			//DMSG("%s %d i = %d \n",__FUNCTION__,__LINE__, i);
#else
			/* Maps directly the memory array */
			operation.params[i].tmpref.size = pBlocks[i].size;
			operation.params[i].tmpref.buffer = pBlocks[i].pAddr;
#endif


		}

		if (i == nBlocks) {
			operation.started = 0;
			/* Defines the parameters types with previous type set */
			operation.paramTypes = TEEC_PARAM_TYPES(types[0],
								types[1],
								types[2],
								types[3]);
			/* Do the GP call */
			gpStatus = TEEC_InvokeCommand(pSession, 0, &operation, NULL);

			/* Update output sizes.
			 * Content is up-to-date as direct memory reference had been provided
			 */
#ifdef LOCAL_CMA_MEMREF
			for (i = 0; i < nBlocks; i++) {
				if (NV_TRUSTED_BLOCK_OUTPUT == pBlocks[i].direction ||
					NV_TRUSTED_BLOCK_INPUT_OUTPUT == pBlocks[i].direction) {
					teec_cma_copy_back(pBlocks[i].pAddr, pBlocks[i].size, operation.params[i].memref.parent, operation.params[i].memref.size);
					pBlocks[i].size = operation.params[i].memref.size;
				}

				if (operation.params[i].memref.parent != NULL) {
					teec_cma_free_shm(operation.params[i].memref.parent);
					operation.params[i].memref.parent = NULL;
				}
			}
#else
			for (i = 0; i < nBlocks; i++) {
				if (NV_TRUSTED_BLOCK_OUTPUT == pBlocks[i].direction ||
					NV_TRUSTED_BLOCK_INPUT_OUTPUT == pBlocks[i].direction) {
					pBlocks[i].size = operation.params[i].tmpref.size;;
				}
			}
#endif
			result = convertGpTeeError(gpStatus);
		} else {

#ifdef LOCAL_CMA_MEMREF
			//the 'for-break' case: free any allocated buffers
			for (i = 0; i < nBlocks; i++) {
				if (operation.params[i].memref.parent != NULL) {
					teec_cma_free_shm(operation.params[i].memref.parent);
					operation.params[i].memref.parent = NULL;
				}
			}
#endif

		}

		tflUnlock();
	}

       DMSG("%s %d result = %d \n",__FUNCTION__,__LINE__, result);

	return result;
}

/* -------------------------------------------------------------------------- */
/*                             convertParamType                               */
/* -------------------------------------------------------------------------- */

/*
 * Convert TNvTrustedBlockDirection to GP parameter type values.
 * Only temporary memory reference are used.
 * This may be adapted if the platform has another preferred way of mapping
 * REE memory in TEE communication.
 * Unknown value are mapped to TEEC_NONE which acts as an error value here.
 * The converted value is returned directly.
 */

static uint32_t convertParamType(uint32_t nvParamType)
{
	uint32_t result = TEEC_NONE;

	switch(nvParamType) {
	case NV_TRUSTED_BLOCK_INPUT:
#ifdef LOCAL_CMA_MEMREF
		result = TEEC_MEMREF_PARTIAL_INPUT;
#else
		result = TEEC_MEMREF_TEMP_INPUT;
#endif
		break;
	case NV_TRUSTED_BLOCK_OUTPUT:
#ifdef LOCAL_CMA_MEMREF
		result = TEEC_MEMREF_PARTIAL_OUTPUT;
#else
		result = TEEC_MEMREF_TEMP_OUTPUT;
#endif
		break;
	case NV_TRUSTED_BLOCK_INPUT_OUTPUT:
#ifdef LOCAL_CMA_MEMREF
		result = TEEC_MEMREF_PARTIAL_INOUT;
#else
		result = TEEC_MEMREF_TEMP_INOUT;
#endif
		break;  /* Return TEEC_NONE which is an error for Nagra blocks */
	default:
		break;
	}

	return result;
}

/* -------------------------------------------------------------------------- */
/*                             convertGpTeeError                              */
/* -------------------------------------------------------------------------- */

/*  Convert GP TEE result value to TNvTrustedResult.
 *  The converted value is returned directly.
 */
static uint32_t convertGpTeeError(TEEC_Result gpError)
{
	uint32_t result = NV_TRUSTED_ERROR;

	switch (gpError)  {
	case TEEC_SUCCESS:
		result = NV_TRUSTED_SUCCESS;
		break;
	case TEEC_ERROR_BAD_FORMAT:          /* no break */
	case TEEC_ERROR_ITEM_NOT_FOUND:      /* no break */
	case TEEC_ERROR_EXCESS_DATA:         /* no break */
	case TEEC_ERROR_NO_DATA:             /* no break */
	case TEEC_ERROR_BAD_PARAMETERS:
		result = NV_TRUSTED_ERROR_BAD_PARAMETER;
		break;
	case TEEC_ERROR_NOT_IMPLEMENTED:
		result = NV_TRUSTED_ERROR_INVALID_OPERATION;
		break;
	case TEEC_ERROR_NOT_SUPPORTED:
		result = NV_TRUSTED_ERROR_NOT_SUPPORTED;
		break;
	case TEEC_ERROR_OUT_OF_MEMORY:
		result = NV_TRUSTED_ERROR_TRUSTED_MEMORY;
		break;
	case TEEC_ERROR_ACCESS_CONFLICT:     /* no break */
	case TEEC_ERROR_BUSY:                /* no break */
	case TEEC_ERROR_COMMUNICATION:
		result = NV_TRUSTED_ERROR_COMMUNICATION;
		break;
	case TEEC_ERROR_ACCESS_DENIED:       /* no break */
	case TEEC_ERROR_SECURITY:
		result = NV_TRUSTED_ERROR_SECURITY;
		break;
	case TEEC_ERROR_SHORT_BUFFER:
		result = NV_TRUSTED_ERROR_BLOCK_TOO_SHORT;
		break;
	case TEEC_ERROR_GENERIC:             /* no break */
	case TEEC_ERROR_BAD_STATE:           /* no break */
	case TEEC_ERROR_CANCEL:              /* no break */
	default:
		break;
	}

	return result;
}

/* ========================================================================== */
/*                                END OF FILE                                 */
/* ========================================================================== */
