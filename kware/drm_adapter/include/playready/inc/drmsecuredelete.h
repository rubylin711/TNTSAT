/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef _DRMSECUREDELETE_H_
#define _DRMSECUREDELETE_H_ 1

#include <drmsecurecoretypes.h>

ENTER_PK_NAMESPACE;

/*********************************************************************
**
** Function: DRM_SECUREDELETE_GenerateChallenge
**
** Synopsis: This function generates a message challenge.
**
** Arguments:
**
** [f_pSecureCore]          -- Pointer to a secure core context.
** [f_pIdSession]           -- Id for the session to be deleted.
** [f_pcbChallenge]         -- The secure stop challenge size in bytes.
** [f_ppbChallenge]         -- The secure stop challenge buffer.
**
** Returns:                 DRM_SUCCESS on success.
**                          DRM_E_INVALIDARG if the arguments are invalid.
**                          DRM_E_NOMORE if there are no messages in the HDS.
**
** Notes:                   Caller must free f_ppbChallenge using
**                          Oem_MemFree after use.
**
*********************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_SECUREDELETE_GenerateChallenge(
    __inout                                 DRM_SECURECORE_CONTEXT     *f_pSecureCore,
    __in                              const DRM_ID                     *f_pIdSession,
    __in                                    DRM_DWORD                   f_cKeyIDs,
    __in_ecount(f_cKeyIDs)            const DRM_ID                     *f_rgKeyIDs,
    __out                                   DRM_DWORD                  *f_pcbChallenge,
    __deref_out_bcount(*f_pcbChallenge)     DRM_BYTE                  **f_ppbChallenge );

/*********************************************************************
**
** Function: DRM_SECUREDELETE_ProcessResponse
**
** Synopsis: This function processes a message response.
**
** Arguments:
**
** [f_cbResponse]           -- The secure stop response size in bytes.
** [f_pbResponse]           -- The secure stop response buffer.
** [f_pIdSession]           -- The Session Id received in the response.
**
** Returns:                 DRM_SUCCESS on success.
**                          DRM_E_INVALIDARG if the arguments are invalid.
**
**********************************************************************/
DRM_NO_INLINE DRM_API DRM_RESULT DRM_CALL DRM_SECUREDELETE_ProcessResponse(
    __in                                  const DRM_DWORD                   f_cbResponse,
    __in_bcount(f_cbResponse)             const DRM_BYTE                   *f_pbResponse,
    __out                                       DRM_ID                     *f_pIdSession );

EXIT_PK_NAMESPACE;

#endif /* _DRMSECUREDELETE_H_ */
