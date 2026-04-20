/**
**  @file asm_prov.h
**
**  @brief
**    Application Session Manager related to provisioning
**
**  @ingroup PSM
**
** COPYRIGHT:
**   2015 Nagravision S.A.
**
*/

#ifndef ASM_PROV_H
#define ASM_PROV_H

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/
#include "nv_defs.h"

/******************************************************************************/
/*                                                                            */
/*                                  DEFINES                                   */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/*                              TYPES & STRUCTURES                            */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                              INTERNAL ASM FUNCTIONS                        */
/*                                                                            */
/******************************************************************************/
/**
 *  @brief
 *    Set provisioning from DMM that corresponds to a given credential id
 * 
 *  @pre
 *    None
 *
 *  @post
 *    Provisioning has been set and 3rd party provisioning data has been returned (if present).
 *
 *  @param[in]  xApplicationSession
 *            Application session
 *  @param[in]  xCredentialsId
 *            Credential id of the DMM to get
 *  @param[in]  pxOpVault
 *            OpVault to be used to verify DMM Signature. If NULL use opVault from xApplicationSession's xOperatorContext.
 *  @param[out] pxProvisioningData
 *            3rd party provisioning retrieved from DMM
 *
 *  @retval  NV_ASM_SUCCESS
 *             Provisioning data has been gotten and:
 *                - 3rd party metadata has been written in pxProvisioningData.
 *                OR
 *                - pxProvisioningData.data is NULL and pxProvisioningData.size == 0: 
 *                In this case, pxProvisioningData.size has been updated with size necessary to store 
 *                3rd party provisioning data.
 *
 *  @retval  NV_ASM_BAD_PARAMETER
 *             In the following cases:
 *              - xApplicationSession is NV_SESSION_INVALID
 *              - Application Session does not exist
 *              - xCredentialsId is NULL
 *              - pxProvisioningData is NULL
 *              - pxProvisioningData.data is NULL and pxProvisioningData.size != 0
 *              - pxProvisioningData.data is not NULL and pxProvisioningData.size == 0
 *
 *  @retval  NV_ASM_ERROR_NOT_FOUND
 *             In the following cases:
 *              - No DMM corresponding to the ceredential id has been found
 *
 *  @retval  NV_ASM_ERROR_BUFFER_TOO_SHORT
 *             In the following cases:
 *              - pxProvisioningData.data is NULL and pxProvisioningData.size != 0
 *              - pxProvisioningData.data is not NULL and pxProvisioningData.size == 0
 *
 *  @retval  NV_ASM_ERROR
 *             In the other cases
*/
uint32_t asmProcessProvisioning
(
  TNvSession   xApplicationSession,   /* IN */
  TNvString    xCredentialsId,        /* IN */
  TNvBuffer*  pxOpVault,              /* IN */
  TNvBuffer*  pxProvisioningData      /* OUT */
);


/**
  @brief
    Remove an entitlement with a specific cred id from the cache.

  @param[in] xApplicationSession
              Current ASM session 

  @param[in] xCredentialsId
              Credential id. 
*/
NV_PUBLIC_API uint32_t asmRemoveLicenseWithCredIdFromCache
(
  TNvSession  xApplicationSession,
  TNvString   xCredentialsId
);

#endif /* ASM_PROV_H */

/* asm_prov.h */
