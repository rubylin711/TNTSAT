/*
 * This file defines additional specific entry points for the Nagra trusted
 * session adapter on the GP TEE standard. It is related to the implementation
 * example of the Nagra trusted framework layers interface on the GP TEE standard.
 * It is also related to the Nagra trusted application integration model.
 * It assumes that the Nagra libraries are the only featured elements linked in
 * the trusted application. It uses the trusted session functions defined in
 * tee_client_api.h GP TEE client header.
 *
 * This file must be included on the REE side.
 *
 * Copyright 2015 Nagravision S.A.
 */

#ifndef NVTA_TFLTS_GPTEE_H
#define NVTA_TFLTS_GPTEE_H
#ifdef __cplusplus
extern "C" {
#endif

#include "tee_client_api.h"

/* ========================================================================== */
/*              ADDITIONAL GP TRUSTED SESSION ADAPTER FUNCTIONS               */
/* ========================================================================== */

/*
 * GP TEE identification depends on the platform architecture.
 * A GP TEE context must be initialized on the right platform TEE.
 * It must be provided to GP Nagra trusted session adapter with the GP TEE context.
 *
 * This interface must be called prior to initialization of Nagra rich clients
 * within the REE.
 * The adapter maintain the value of the reference to the GP TEE context.
 * It does not copy the context structure content.
 * The GP TEE context is persisted as long as the Nagra libraries are running.
 *
 * Refer to TEEC_InitializeContext() and TEEC_FinalizeContext() for
 * GP TEE context management.
 */

extern void nvGpTeeConfigure(TEEC_Context *pxTeeContext);

#ifdef __cplusplus
}
#endif

#endif /* NVTA_TFLTS_GPTEE_H */
