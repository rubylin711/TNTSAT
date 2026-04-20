/**
  @file prm_asm.h

  @brief
  This file defines the application session interface.

  @details

  This interface is realized by the @Ma.
  It provides an @PROD operator client management service.

  COPYRIGHT:
    2014 - 2017 Nagravision S.A.
*/

/*
   ==========================================================================
   IMPORTANT REMARK :
   ==========================================================================

   Comments in this file use special tags to allow automatic API
   documentation generation in HTML format, using the GNU-General Public
   Licensed Doxygen tool.
   For more information about Doxygen, please check www.doxygen.org

   Depending on the platform, the CHM file may not open properly if it is
   stored on a network drive. So either the file should be moved on a local
   drive or add the following registry entry on Windows platform (regedit):
   [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\\ItssRestrictions] "MaxAllowedZone"=dword:00000003

   ==========================================================================
*/

/* ========================================================================== */
/* Internal groups                                                            */
/* ========================================================================== */

/**
  @addtogroup g_asm
  @brief
  Describe the Application Session Manager interface of @Ma.

  @details

  The <b>Application Session manager</b> interface introduces definition for
  configuring and controlling @MA. Please make sure to have read the
  documentation pages for a complete description of the interface constraints
  and requirements.
*/

#ifndef NV_ASM_H
#define NV_ASM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Dependencies                                                               */
/* ========================================================================== */

#include "prm_client.h"
#include "prm_dsm.h"
#include "prm_lds.h"

/* ========================================================================== */
/*                                    TYPES                                   */
/* ========================================================================== */

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/**
  @ingroup g_defs_common
  @brief
  Define a structure with key ID information.

  @details
  This structure provides a keyID with it's status and usage rules.
*/
typedef struct 
{
  TNvIdentifier  keyId;
  /**< key identifier. */
  TNvKeyStatus   keyStatus;
  /**< Status of the content key identified by the key identifier.
        Refer to ::TNvKeyStatus for key status values. */
  uint32_t       expiryDate;
  /**< Expiry date of the keyId */
  TNvBuffer      usageRules;
  /**< Usage rules associated to the content key */
}
TNvKeyIdInfo;

/* ========================================================================== */
/* Definitions                                                                */
/* ========================================================================== */

/**
  @addtogroup g_asm
  @{
*/

/**
  @brief
  Values for application session operation result.

  This enumeration provides all the possible values that can be returned by an
  application session operation.
*/

typedef enum
{
  NV_ASM_SUCCESS                  = 0x00000000L,
  /**< The application session operation has been successful. */
  NV_ASM_ERROR_BAD_PARAMETER      = 0x00000001L,
  /**< The application session operation failed: One or more provided parameters are invalid. */
  NV_ASM_ERROR_BUFFER_TOO_SHORT   = 0x00000002L,
  /**< The application session operation failed: A provided memory block is too short to be filled in with the expected result. */
  NV_ASM_ERROR_STORAGE            = 0x00000003L,
  /**< The application session operation failed or partially failed: A storage error -- corruption, access issue ... -- has been encountered. */
  NV_ASM_ERROR_NOT_FOUND          = 0x00000004L,
  /**< The application session operation failed: An provided or requested item was not found. */
  NV_ASM_ERROR_PROCESSING         = 0x00000005L,
  /**< The application session operation failed: The request is conflicting with another request currently processing. */
  NV_ASM_ERROR_NEED_PROVISIONING  = 0x00000006L,
  /**< The application session operation failed: The @PROD client need to be provisioned. */
  NV_ASM_ERROR                    = 0x0000FFFFL
  /**< The application session operation failed: An unexpected error occurred. */
}
TNvAsmResult;

/* addtrogroup g_asm */
/** @} */

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */

/**
  @addtogroup g_asm
  @{
*/

/**
  @brief
  Structure regrouping the information about the key used by @PROD:
  license identifier used to decrypt the content key and expiration time of the content key.
*/
typedef struct
{
  TNvLicenseIdentifier  identifier;
  /**< Identification of the item consumed. */
  uint32_t              currentKeyExpirationTime;
  /**< Expiration time of the content key currently related to the item consumed. */
  /* (as number of seconds since epoch: posix time_t). */
}
TNvConsumption;


/* addtrogroup g_asm */
/** @} */

/* ========================================================================== */
/* Functions                                                                  */
/* ========================================================================== */

/**
  @addtogroup g_asm
  @{
*/

/**
  @brief
  Retrieve the @PROD integration context.

  @pre
  The @Xa must have been successfully initialized.

  @post
  None.

  This function allows retrieving the integration context data related to the @Ta.
  Refer to @ref p_asm_vault for the integration context data format description.

  The integration context data is returned in the memory block provided with
  @a pxIntegrationContextData:
  - If @a pxIntegrationContextData describes a valid memory block that can hold
    the integration context data, the latter is copied in the memory block and
    the @a size is set to the size of data copied.
  - If the @a data field of this structure is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the integration context data.
  - If @a pxIntegrationContextData describes a valid memory block but that is
    too short to hold the integration context data, the function fails
    resulting with ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has
    been set with the required size for holding the integration context data.

  @note
  The provided integration context data is a null-terminated string describing
  a <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The integration context data format has the following @ref p_asm_integration_context "JSON schema" @n.

  @param[out]   pxIntegrationContextData
  Integration context data of the @Ma.

  @retval ::NV_ASM_SUCCESS
  @a pxIntegrationContextData has been successfully filled in with the @Ma
  integration context. Its @a size field has been adjusted to the actual size
  of the filled data.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a pxIntegrationContextData is @c NULL.
  + @a pxIntegrationContextData is not @c NULL but its @a data field is not
    @c NULL and its @a size field is @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxIntegrationContextData is too short.

  @retval ::NV_ASM_ERROR
  The integration context is not yet available or an unexpected error
  occurs during processing.

  @see @ref p_asm_vault.
*/

NV_PUBLIC_API uint32_t nvGetIntegrationContextData
(
  TNvBuffer*  pxIntegrationContextData
);

/**
  @brief
  Open an application session and configure it for a specific operator.

  @pre
  The @Xa must have been successfully initialized.

  @post
  An application session has been created and associated to the provided
  operator vault. A unique application session identifier has been generated
  and provided back.

  This function allows opening a new application session. The new application
  session is associated and configured with the operator vault provided in
  @a pxOperatorVault.

  The new application session identifier is provided back in
  @a *pxApplicationSession. This parameter must not be @c NULL. The operation
  failed resulting with ::NV_ASM_ERROR_BAD_PARAMETER if it is @c NULL. If the
  operation failed, ::NV_SESSION_INVALID value is provided back in
  @a *pxApplicationSession.

  It fails also with ::NV_ASM_ERROR_BAD_PARAMETER if the @a pxOperatorVault
  parameter is @c NULL.

  @note
  The operator vault data is a string describing a
  <a href="http://json.org/">JSON</a> object has the following @ref p_asm_opvault "JSON schema" @n

  It is described as the @c operatorVault field of the operator vault message.

  @param[out] pxApplicationSession
  Reference to an application session identifier.
  It shall not be @c NULL.
  The referred value is set with the new application session identifier
  or with the ::NV_SESSION_INVALID value if an error occurred.

  @param[in]  pxOperatorVault
  Reference to the operator vault data.
  It shall not be @c NULL.

  @retval ::NV_ASM_SUCCESS
  A new application session has been opened and associated to the provided
  operator vault.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  @a pxApplicationSession or @a pxOperatorVault parameters is @c NULL.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The operator vault data provided with @a pxOperatorVault cannot be fully
  analyzed as the @PROD client has not been provisioned with the relevant data.
  However the session is opened but is not fully functional.
  Other @PROD feature will return an error as long as the relevant data has
  not been provisioned. Once provisioned, the session becomes fully functional.
  When the session need to be provisioned, only the following interface
  functions can be called:
  - nvAsmClose() on to close this newly created session
  - nvAsmGetProvisioningParameters() can be called for feeding provisioning
    service.
  - nvGetIntegrationContextData() as it does not need any session parameter.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred:
  + A mandatory property is not set
  + @PROD Client is not started
  + Problem of Memory resources, drivers, ...

  @see nvAsmClose(), @ref p_asm_vault, @ref g_nva_manage
*/

NV_PUBLIC_API uint32_t nvAsmOpen
(
  TNvSession* pxApplicationSession,
  TNvBuffer*  pxOperatorVault
);

/**
  @brief
  Close an application session.

  @pre
  None.

  @post
  The provided application session -- whether valid or not -- is considered
  closed and its identifier shall no more be used. Internal application
  session resources have been released.

  This function closes an application session identified by
  @a xApplicationSession and previously opened with nvAsmOpen().
  The operation releases all session-related resources and shall never fail
  whether the provided application session identifier is valid or not.

  @param[in]    xApplicationSession
  Application session identifier.

  @see nvAsmOpen(), @ref p_asm_vault.
*/

NV_PUBLIC_API void nvAsmClose
(
  TNvSession  xApplicationSession
);

/**
  @brief
  Retrieve the provisioning parameters related to an application session.

  @pre
  An application session requires relevant provisioning data.

  @post
  None.

  This function allows retrieving the provisioning parameters associated to
  the session related to the @a xApplicationSession identifier.
  Provisioning parameters are provided in <a href="http://json.org/">JSON</a>
  format.
  It can be provided unmodified to the device provisioning service client.

  The provisioning parameters are returned in the memory block provided with
  @a pxParameters:
  - If @a pxParameters describes a valid memory block that can hold the
    provisioning parameters, the latter is copied in the memory block and the
    @a size is set to the size of data copied.
  - If the @a data field of this structure is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the provisioning parameters.
  - If @a pxParameters describes a valid memory block but that is too short to
    hold the provisioning parameters, the function fails resulting with
    ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the
    required size for holding the provisioning parameters.

  @note
  The provided provisioning parameters is a string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  When this data is required, it must be provided unmodified to the device
  provisioning service client using nvDpscSetClientData().

  @warning
  If the application session does not need relevant provisioning data, a call
  to this function will still be successful and will return valid provisioning
  parameters.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[out]   pxParameters
  Exported provisioning parameters.

  @retval ::NV_ASM_SUCCESS
  @a pxParameters has been successfully filled in with the related
  provisioning parameters.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is not valid.
  + @a pxParameters is @c NULL.
  + @a pxParameters is not @c NULL but its @a data field is not @c NULL and
    its @a size field is @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxParameters is too short.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.
*/

NV_PUBLIC_API uint32_t nvAsmGetProvisioningParameters
(
  TNvSession  xApplicationSession,
  TNvBuffer* pxParameters
);

/**
  @brief
  Store external context reference within an application session.

  @pre
  An application session must have been successfully opened.

  @post
  The opaque reference on external context data is stored in the provided
  application session.

  This function allows storing an opaque external context reference provided by
  @a xContext within the application session identified by
  @a xApplicationSession. This reference can be @c NULL in which case no more
  opaque context reference is maintained with the application session.

  This reference can be later retrieved from the session using nvAsmGetContext().

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xContext
  External opaque context reference.

  @retval ::NV_ASM_SUCCESS
  @a xContext has been successfully stored in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is not valid.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.

  @see nvAsmOpen(), nvAsmGetContext().
*/

NV_PUBLIC_API uint32_t nvAsmSetContext
(
  TNvSession  xApplicationSession,
  TNvHandle   xContext
);

/**
  @brief
  Get external context reference stored in an application session.

  @pre
  An application session must have been successfully opened.

  @post
  None.

  This function allows retrieving the external opaque context reference value
  previously provided with nvAsmSetContext(). The context reference is related
  to the application session identified by @a xApplicationSession. The value is
  provided back using the pointer provided by @a pxContext.

  The context reference value can be @a NULL if it has never been previously
  set or if it has been previously set to a @a NULL value.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[out]  pxContext
  Pointer to an external opaque context reference.

  @retval ::NV_ASM_SUCCESS
  @a pxContext has been successfully filled in with the stored context
  reference value.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is not valid.
  + @a pxContext is @a NULL.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.

  @see nvAsmOpen(), nvAsmSetContext().
*/

NV_PUBLIC_API uint32_t nvAsmGetContext
(
  TNvSession  xApplicationSession,
  TNvHandle* pxContext
);

/**
  @brief
  Retrieve the operator data related to an application session.

  @pre
  An application session must have been successfully opened.

  @post
  None.

  This function allows retrieving the operator data associated to the session
  related to the @a xApplicationSession identifier and identified by @a xKind.
  The format of operator data is proprietary to the operator defining the
  application.

  The operator data are returned in the memory block provided with @a pxData:
  - If @a pxData describes a valid memory block that can hold the @PROD data, the
    latter is copied in the memory block and the @a size is set to the size of
    data copied.
  - If the @a data field of this structure is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the operator data.
  - If @a pxData describes a valid memory block but that is too short to hold
    the @PROD operator data, the function fails resulting with
    ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the
    required size for holding the operator data.

  @note
  The provided operator data is a null-terminated string describing a
  <a href="http://json.org/">JSON</a> string value.
  The related @a size field does not include the terminating null character.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xKind
  Kind of operator data to export.

  @param[out]   pxData
  Exported operator data.

  @retval ::NV_ASM_SUCCESS
  @a pxData has been successfully filled in with the related operator data.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is not valid.
  + @a xKind is @c NULL.
  + @a pxData is @c NULL.
  + @a pxData is not @c NULL but its @a data field is not @c NULL and
    its @a size field is @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxData is too short.

  @retval ::NV_ASM_ERROR_NOT_FOUND
  The provided string with @a xKind does not refer a valid operator metadata.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.
*/

NV_PUBLIC_API uint32_t nvAsmGetOperatorData
(
  TNvSession  xApplicationSession,
  TNvString   xKind,
  TNvBuffer* pxData
);

/**
  @brief
  Retrieve all the @PROD properties.

  @pre
  The Connect client must have been successfully initialized.

  @post
  None.

  This function allows retrieving all the @PROD properties associated to the
  session related to the @a xApplicationSession identifier.

  The @PROD properties are returned in the memory block provided with
  @a pxProperties:
  - If @a pxProperties describes a valid memory block that can hold the @PROD
    properties, the latter is copied in the memory block and the @a size is
    set to the size of data copied.
  - If the @a data field of this structure is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the @PROD properties.
  - If @a pxProperties describes a valid memory block but that is too short to
    hold the @PROD properties, the function fails resulting with
    ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the
    required size for holding the @PROD properties.

  If the application session requires provisioning or if no application session is available,
  the application session identifier NV_SESSION_INVALID can be used. Each non-available property
  will have its default value.

  @note
  The provided properties is a null-terminated string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The properties format is described in the @ref p_asm_properties page.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[out]   pxProperties
  Exported @PROD properties.

  @retval ::NV_ASM_SUCCESS
  @a pxProperties has been successfully filled in with the @PROD properties.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a pxProperties is @c NULL.
  + @a pxProperties is not @c NULL but its @a data field is not @c NULL and
    its @a size field is @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxProperties is too short.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.
  In this case the application should perform the nvAsmGetProperties with the
  application session identifier equal to NV_SESSION_INVALID

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.

  @see @ref p_asm_properties.
*/

NV_PUBLIC_API uint32_t nvAsmGetProperties
(
  TNvSession  xApplicationSession,
  TNvBuffer* pxProperties
);

/**
  @brief
  Provide the @PROD persistent storage path for this application session.

  @pre
  An application session must have been successfully opened.
  The provided path must exist. The @Ma must have the permission to read and
  write in this directory.

  @post
  None.

  This function configures the @Ma persistent storage for this application
  session:
  - It initializes the persistent storage function of the @Ma on the
    provided application session.
  - It provides the @Ma the path to the persistent storage space that is
    specific to the application session.

  The @PROD persistent storage path defines a path to a directory where @Ma
  can persist all its materials e.g. licenses ...
  It must be set only once for a specific application session.

  @warning
  The path is a null-terminated string describing a path to a directory. It
  must be terminated with the directory separator related to the underlying
  operating system e.g. "\" for Windows local storage, "/" for POSIX
  directory, etc... Current directory must be defined using ".\" or "./".

  @note
  Persistent storage path defined for different application session can be
  different or identical. The @Ma creates a domain within the persistent
  storage space related to the operator defined by the operator vault
  provided with nvAsmOpen(). Therefore a @Ma application session can
  successfully retrieve data previously persisted when configured with the
  same operator vault and the same persistent storage path than those used
  when data were persisted.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xPath
  Path where to store the @Ma material.

  @retval ::NV_ASM_SUCCESS
  @a xPath is a valid existing path.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.
  + @a xPath is not terminated by "/" or "\".
  + @a xPath does not define an existing path.

  @retval ::NV_ASM_ERROR_STORAGE
  The storage path has not been set for the provided application session
  (storage path already set, storage access...).

  @retval ::NV_ASM_ERROR_PROCESSING
  License information are currently being imported so that storage cannot be
  set safely. Consider setting storage after application session creation and
  before any other operation on this session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.

  @see nvAsmOpen(), @ref p_asm_licenses.
*/

NV_PUBLIC_API uint32_t nvAsmUseStorage
(
  TNvSession  xApplicationSession,
  TNvString   xPath
);

/**
  @brief
  Reset the @PROD persistent storage related to this application session.

  @pre
  An application session must have been successfully opened.
  No concurrent operation must try to write the associated storage path.

  @post
  The persistent storage related to the application session has been erased
  if any storage path has been defined.

  @param[in]    xApplicationSession
  Application session identifier.

  @retval ::NV_ASM_SUCCESS
  The persistent storage related to the application session has been
  successfully erased (or storage not defined).

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.

  @retval ::NV_ASM_ERROR_STORAGE
  The storage has not been reset for the provided application session
  (storage defined but not accessible...).

  @retval ::NV_ASM_ERROR_PROCESSING
  License information are currently being imported so that storage cannot be
  accessed safely. Consider resetting storage after importation completion.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs.

  @see nvAsmOpen(), @ref p_asm_licenses.
*/

NV_PUBLIC_API uint32_t nvAsmResetStorage
(
  TNvSession  xApplicationSession
);

/**
  @brief
  Retrieve a digest of the current content of the storage related to the
  application.

  @pre
  An application session must have been successfully opened.
  The storage path has been previously defined with nvAsmUseStorage().
  No storage operation is in progress (import, deletion, ...).

  @post
  None.

  This function allows retrieving a digest of the current content of the
  storage related to the @a xApplicationSession identifier.
  The digest is returned in the memory block provided with @a pxHash.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[out]   pxHash
  Exported digest.

  @retval ::NV_ASM_SUCCESS
  @a pxHash has been successfully filled in with the digest.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.
  + @a pxHash is @c NULL.

  @retval ::NV_ASM_ERROR_PROCESSING
  License information are currently being imported so that storage cannot be
  accessed safely.
  Consider retrieving storage hash after importation completion.

  @retval ::NV_ASM_ERROR_STORAGE
  No or partial information is provided back as storage errors have been
  encountered (for instance storage was not configured by a previous call to
  nvAsmUseStorage())

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.
*/

NV_PUBLIC_API uint32_t nvAsmStorageHash
(
  TNvSession   xApplicationSession,
  TNvHash256* pxHash
);


/**
  @brief
  Remove @PROD Licenses from memory cache.

  @pre
  An application session must have been successfully opened.

  @details
  This function allows the client application to remove @PROD licenses
  related to the application session from memory cache in order to release
  no more used memory. There is no impact on licenses stored in the persistent storage.

  The @PROD license identifiers define entitlements to remove.
  They can be provided by the callback of the function nvAsmSetOnLicenseUsedListener(),
  or by the field credentialId in the licenses returned by nvAsmExportLicensesInformation().

  Providing @PROD license identifier is optional and can be @c NULL. In
  this case @a xCount must be @c 0, and all @PROD licenses related to
  @a xApplicationSession and contained in memory cache are removed.


  @param[in]     xApplicationSession
  Application session identifier.

  @param[in]     xCount
  Number of @PROD licenses to remove from memory cache.

  @param[in]    pxIdentifiers
  Table containing the @a xCount @PROD license identifiers corresponding to
  the @PROD licenses to remove from memory cache.

  @retval ::NV_ASM_SUCCESS
  The operation processed successfully.
  All specified @PROD licenses have been removed from memory whether they
  were present or not before the function call.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.
  + @a xCount is @c 0 and @a pxIdentifiers is not @c NULL.
  + @a xCount is not @c 0 and @a pxIdentifiers is @c NULL.

  @retval ::NV_ASM_ERROR_PROCESSING
  A @PROD message importation is currently being processed.
  Consider recalling this function after importation completion.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
   One of the targeted licenses is used by a descrambling session, or an unexpected error has occurred during processing.

  @warning
  If a license targeted by the @PROD license identifiers table is in use
  in a descrambling session, and is not also present in the persistent
  storage, the call to this function will stop the descrambling.

  @see nvAsmSetOnLicenseUsedListener(), nvAsmExportLicensesInformation()
*/

NV_PUBLIC_API uint32_t nvAsmRemoveLicensesFromCache
(
  TNvSession             xApplicationSession,
  uint32_t               xCount,
  TNvLicenseIdentifier* pxIdentifiers
);


/**
  @brief
  Remove all persisted data related to this application session.

  @pre
  An application session must have been successfully opened.

  @post
  All the persisted data related to the application session -- and therefore the
  operator associated to the operator vault provided during nvAsmOpen() -- has
  been erased.

  @param[in]    xApplicationSession
  Application session identifier.

  This operation removes all the data targeted by nvAsmResetStorage() as well
  as additional data related to the @a xApplicationSession -- e.g. provisioned
  data.

  @retval ::NV_ASM_SUCCESS
  All the persisted data related to the application session has been
  successfully erased.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.

  @retval ::NV_ASM_ERROR_STORAGE
  Not all data have been removed for the provided application session
  (storage access error...).

  @retval ::NV_ASM_ERROR_PROCESSING
  Persisted data are currently being locked -- e.g. license importation -- so
  that it cannot be accessed safely.
  Consider resetting storage after locking operation completion.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs.
*/

NV_PUBLIC_API uint32_t nvAsmFactoryReset
(
  TNvSession  xApplicationSession
);

/**
  @brief
  Retrieve a digest of the current content of the trusted storage related to
  the operator application.

  @pre
  An application session must have been successfully opened.
  No storage operation is in progress (import, deletion, ...).

  @post
  None.

  This function allows retrieving a digest of the current content of the
  trusted storage related to @a xApplicationSession identifier.
  The digest is returned in the memory block provided with @a pxHash.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[out]   pxHash
  Exported digest.

  @retval ::NV_ASM_SUCCESS
  @a pxHash has been successfully filled in with the digest.
  Its @a size field has been adjusted to the actual size of the filled data
  (16 bytes).

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.
  + @a pxHash is @c NULL.

  @retval ::NV_ASM_ERROR_STORAGE
  No or partial information is provided back as storage errors have been
  encountered.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.
*/

NV_PUBLIC_API uint32_t nvAsmTrustedStorageHash
(
  TNvSession   xApplicationSession,
  TNvHash256* pxHash
);

/**
  @brief
  Retrieve all the current @PROD licenses information.

  @pre
  An application session must have been successfully opened.

  @post
  None.

  This function allows retrieving all the @PROD licenses information associated
  to the session related to the @a xApplicationSession identifier.

  The @PROD licenses information are returned in the memory block provided with
  @a pxInformation:
  - If @a pxInformation describes a valid memory block that can hold the @PROD
    licenses information, the latter is copied in the memory block and the
    @a size is set to the size of data copied.
  - If the @a data field of this structure is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the @PROD licenses information.
  - If @a pxInformation describes a valid memory block but that is too short
    to hold the @PROD licenses information, the function fails resulting with
    ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the
    required size for holding the @PROD licenses information.

  The stored @PROD licenses information is exported only if the @PROD storage
  path has been previously set with nvAsmUseStorage().

  @note
  The provided @PROD licenses information is a null-terminated string
  describing a <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The @PROD licenses information format is described in
  @ref p_asm_licenses page.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[out]   pxInformation
  Exported @PROD licenses information.

  @retval ::NV_ASM_SUCCESS
  @a pxInformation has been successfully filled in with the @PROD licenses
  information. Its @a size field has been adjusted to the actual size of the
  filled data.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.
  + @a pxInformation is @c NULL.
  + @a pxInformation is not @c NULL but its @a data field is not @c NULL
    and its @a size field is @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxInformation is too short.

  @retval ::NV_ASM_ERROR_STORAGE
  No or partial information is provided back as storage errors have been
  encountered.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.

  @see nvAsmUseStorage(), @ref p_asm_licenses.
*/

NV_PUBLIC_API uint32_t nvAsmExportLicensesInformation
(
  TNvSession  xApplicationSession,
  TNvBuffer* pxInformation
);

/**
  @brief
  Retrieve the requested kind data from the license's protected part.

  @pre
  An application session must have been successfully opened.

  @post
  None.

  This function allows retrieving requested kind data from the license's protected part.
  The license is identified by the credential id and associated to the session related to the @a xApplicationSession identifier.

  The kind data is returned in the memory block provided with
  @a pxData:
  - If @a pxData describes a valid memory block that can hold the kind data,
    the latter is copied in the memory block and the @a size is set to the size
    of data copied.
  - If the @a data field of this structure is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the @PROD license kind data.
  - If @a pxData describes a valid memory block but that is too short
    to hold the kind license kind data, the function fails resulting with
    ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the
    required size for holding the @PROD license kind data.

  The kind data from the stored license will be exported only if the @PROD storage path has been previously
  set with nvAsmUseStorage().

  @note
  The kind data is returned as a null-terminated string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The kind data format is described in @ref p_asm_licenses page.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    pxCredentialID
  Credential ID to be matched to get kind data from the license. It should be a NULL terminated string.

  @param[in]    pxKind
  Kind string to be matched to get kind data from the license. It can not be NULL and
  the kind string should be terminated by NULL. If license has multiple kind data for
  the provided kind string, it will return error.

  @param[out]   pxData
  Exported @PROD protected kind license data

  @retval ::NV_ASM_SUCCESS
  @a pxData has been successfully filled in with the @PROD license kind data.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid application session.
  + @a pxCredentialId is @c NULL or Invalid.
  + @a pxKind is @c NULL.
  + @a pxData is @c NULL.
  + @a pxData is not @c NULL but its @a data field is not @c NULL
    and its @a size field is @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxData is too short.

  @retval ::NV_ASM_ERROR_STORAGE
  No or partial data is provided back as storage errors have been
  encountered.

  @retval ::NV_ASM_ERROR_NOT_FOUND
  Data is not found for the provided pxCredentialID & pxKind.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error occurs during processing.

  @see nvAsmExportLicensesInformation(), nvAsmUseStorage(), @ref p_asm_licenses.
*/

NV_PUBLIC_API uint32_t nvAsmGetLicenseProtectedKindData
(
  TNvSession  xApplicationSession,
  TNvString  pxCredentialID,
  TNvString  pxKind,
  TNvBuffer* pxData
);

/**
  @brief
  Fingerprint listener.

  @pre
  A new content key is to be used on a descrambling session.

  @post
  The fingerprint information is available to the middleware.
  Middleware shall parse @a pxFingerprintData and start/stop rendering the fingerprint.

  @note
  The exported fingerprint Data is a binary data, its format is described in the @ref p_nva_refdoc "[FPR]".
  The related @a size field indicates the number of bytes returned.
  A value @c NULL for @a data field of @a pxFingerprintData, indicates that middleware shall stop rendering the fingerprint.
  Detailed behaviours of @Ma in different use cases of fingerprint are defined in @ref p_asm_fp page.


  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    pxFingerprintData
  Fingerprint Data.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not processed
  by the application session.

  @see nvAsmSetOnFingerprintListener(), @ref p_asm_fp
*/

typedef bool_t (*INvFingerprintListener)
(
  TNvSession  xApplicationSession,
  TNvSession  xDescramblingSession,
  TNvBuffer* pxFingerprintData
);

/**
  @brief
  Define a listener callback function for fingerprint event.

  @pre
  The provided application session must be valid.

  @post
  The listener function is registered for the application session.

  After being registered, the listener callback function is triggered each time
  @Ma receives a new fingerprint information to be rendered for a descrambling session.

  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xOnFingerprint
  Listener callback function to register.

  @retval ::NV_ASM_SUCCESS
  The listener callback function has been registered in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvAsmOpen()
*/

NV_PUBLIC_API uint32_t nvAsmSetOnFingerprintListener
(
  TNvSession             xApplicationSession,
  INvFingerprintListener xOnFingerprint
);

/**
  @brief
  Usage rules listener.

  @pre
  A new content key is to be used on a descrambling session.
  Usage rules must be checked.

  @post
  The descrambling process using the content key resumes if usage rules have
  been fulfilled or stops otherwise. The allocated resources for the event
  are released. No reference must be kept on them. If provided information
  must be persisted, it must copied during the notification processing.

  This type defines a notification function callback when usage rules --
  mainly operator usage rules -- needs to be checked. This notification
  function is called synchronously to the entitlement processing. The latter is
  blocked until the function returns.

  Depending on the function returned value, the entitlement processing will
  be resumed -- ::TRUE -- or discarded -- ::FALSE. The application receiving
  the notification is expected to block related descrambling process -- stream
  processing -- check provided usage rules and return accordingly if
  content descrambling must be resumed or not.

  @note
  The exported usage rules data is a null-terminated string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The usage rules data format is described in the @ref p_dsm_ur page.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    pxUsageRules
  Usage rules to check.

  @return
  The function shall return ::TRUE if the usage rules are fulfilled or ::FALSE
  otherwise. When ::FALSE is returned the related entitlement and content key
  will not be used. In that case, the related descrambling will fail.

  @see nvAsmSetOnUsageRulesListener().
*/

typedef bool_t (*INvUsageRulesListener)
(
  TNvSession  xApplicationSession,
  TNvSession  xDescramblingSession,
  TNvBuffer* pxUsageRules
);

/**
  @brief
  Define a listener callback function for usage rules event.

  @pre
  The provided application session must be valid.

  @post
  The listener function is registered for the application session.

  After being registered, the listener callback function is triggered each time
  the @Ma is about to use a new entitlement in a descrambling session.

  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xOnUsageRules
  Listener callback function to register.

  @retval ::NV_ASM_SUCCESS
  The listener callback function has been registered in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvAsmOpen(), @ref s_asm_control_ur.
*/

NV_PUBLIC_API uint32_t nvAsmSetOnUsageRulesListener
(
  TNvSession              xApplicationSession,
  INvUsageRulesListener   xOnUsageRules
);

/**
  @brief
  License consumption listener.

  @pre
  A new license has been selected for consumption in a descrambling session.

  @post
  The notified event is considered processed and is not maintained. The
  allocated resources for the event are released. No reference must be kept on
  them. If provided information must be persisted, it must be copied during the
  notification processing.

  This type defines a notification function callback when a descrambling
  session consumes successfully a new license.
  @a pxConsumption referred structure contains identifier of license used
  as well as the computed expiration time of the content key currently
  selected for descrambling.
  The license identifier is expressed as a character array.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    pxConsumption
  Consumption information.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not processed
  by the application session.

  @see nvAsmSetOnLicenseUsedListener().
*/

typedef bool_t (*INvLicenseUsedListener)
(
  TNvSession             xApplicationSession,
  TNvSession             xDescramblingSession,
  const TNvConsumption* pxConsumption
);

/**
  @brief
  Define a listener callback function for license consumption event.

  @pre
  The provided application session must be valid.

  @post
  The listener function is registered for the application session.

  After being registered, the listener callback function is triggered each time
  @Ma uses a new license for successful descrambling.

  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xOnLicenseUsed
  Listener callback function to register.

  @retval ::NV_ASM_SUCCESS
  The listener callback function has been registered in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvAsmOpen().
*/

NV_PUBLIC_API uint32_t nvAsmSetOnLicenseUsedListener
(
  TNvSession              xApplicationSession,
  INvLicenseUsedListener  xOnLicenseUsed
);

/**
  @brief
  Key issue listener.

  @pre
  A key issue occurs on a descrambling session.

  @post
  The notified event is considered processed and is not maintained. The
  allocated resources for the event are released. No reference must be kept on
  them. If provided information must be persisted, it must copied during the
  notification processing.

  This type defines a notification function callback when a descrambling
  session has not or no more valid content keys for content processing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    xKeyStatus
  Current key status of the related descrambling session.

  @param[in]    pxPrmContentMetadata
  @PROD content metadata associated to the required content key.

  @param[in]    xStreamType
  Type of the stream being currently processed by the descrambling session.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not processed
  by the application session.

  @see nvAsmSetOnNeedKeyListener().
*/

typedef bool_t (*INvNeedKeyListener)
(
  TNvSession      xApplicationSession,
  TNvSession      xDescramblingSession,
  TNvKeyStatus    xKeyStatus,
  TNvBuffer*     pxPrmContentMetadata,
  TNvStreamType   xStreamType
);

/**
  @brief
  Define a listener callback function for key issue event.

  @pre
  The provided application session must be valid.

  @post
  The listener function is registered for the application session.

  After being registered, the listener callback function is triggered each time
  the @Ma fails to descramble and needs a valid content key.

  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xOnNeedKey
  Listener callback function to register.

  @retval ::NV_ASM_SUCCESS
  The listener callback function has been registered in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvAsmOpen(), @ref s_asm_control_key.
*/

NV_PUBLIC_API uint32_t nvAsmSetOnNeedKeyListener
(
  TNvSession          xApplicationSession,
  INvNeedKeyListener  xOnNeedKey
);

/**
  @brief
  Renewal listener.

  @pre
  Licenses need to be renewed.

  @post
  The notified event is considered processed and is not maintained.

  This type defines a notification function callback when a renewal date of a
  @PROD license is reached. The application is notified so that it should connect
  to the server in order to renew the licenses of the @Ma.

  @param[in]    xApplicationSession
  Application session identifier.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not
  processed by the application session.

  @see nvAsmSetOnRenewalListener().
*/

typedef bool_t (*INvRenewalListener)
(
  TNvSession  xApplicationSession
);

/**
  @brief
  Define a listener callback function for renewal event.

  @pre
  The provided application session must be valid.

  @post
  The listener function is registered for the application session.

  After being registered, the listener callback function is triggered each time
  the lowest renewal date of @Ma licenses is reached.

  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xOnRenewal
  Listener callback function to register.

  @retval ::NV_ASM_SUCCESS
  The listener callback function has been registered in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvAsmOpen(), @ref p_asm_renewal.
*/

NV_PUBLIC_API uint32_t nvAsmSetOnRenewalListener
(
  TNvSession              xApplicationSession,
  INvRenewalListener      xOnRenewal
);

/**
  @brief
  Access changed listener.

  @pre
  An change of access status occurs on a descrambling session.

  @post
  The notified event is considered processed and is not maintained.

  This type defines a notification function callback when access/key status changes
  on a descrambling session.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    xKeyStatus
  New access/key status.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not
  processed by the application session.

  @see nvAsmSetOnAccessChangedListener().
*/

typedef bool_t (*INvAccessChangedListener)
(
  TNvSession      xApplicationSession,
  TNvSession      xDescramblingSession,
  TNvKeyStatus    xKeyStatus
);

/**
  @brief
  Define a listener callback function for access status change event.

  @pre
  The provided application session must be valid.

  @post
  The listener function is registered for the application session.

  After being registered, the listener callback function is triggered each time
  a descrambling status changes among the @Ma descrambling sessions.

  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    xApplicationSession
  Application session identifier.

  @param[in]    xOnAccessChanged
  Listener callback function to register.

  @retval ::NV_ASM_SUCCESS
  The listener callback function has been registered in the application session.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvAsmOpen(), @ref s_asm_control_access.
*/

NV_PUBLIC_API uint32_t nvAsmSetOnAccessChangedListener
(
  TNvSession                  xApplicationSession,
  INvAccessChangedListener    xOnAccessChanged
);

/**
  @brief
  Retrieve identifiers of all inactive secure stop objects.

  @pre
  The provided application session must be valid.

  @post
  None.

  @details

  Return all inactive secure stop object identifiers related to
  @a xApplicationSession.
  A secure stop object is related to a license that has already been
  used for content descrambling and which was configured for secure stop
  management.

  An inactive secure stop object is a secure stop object:
  - which was managed and stopped during the current operator session,
  - which was managed and stopped during another operator session related to
    the same operator vault configuration but never deleted,
  - which was managed but not stopped during a previous operator session
    related to the same operator vault configuration -- e.g. a reboot occurs...
    -- and never deleted.

  Inactive secure stop objects must generally be reported and deleted.
  Secure stop objects are identified by the related license identifier.

  The table of secure stop identifiers is returned in the memory block provided
  with @a pxIdentifiers and @a pxCount. @a pxCount provides in input the number
  of secure stop identifiers that can be written in @a pxIdentifiers.
  It is updated with the number of secure stop identifiers actually written.

  @a pxIdentifiers is interpreted as follows:
  - If @a pxIdentifiers describes a valid memory block that can hold the table
    of secure stop identifiers, the latter is copied in the memory block and
    the @a pxCount is set to the number of identifiers actually copied.
  - If @a pxIdentifiers is @c NULL, the function returns successfully but does
    not copy anything and it sets @a pxCount to the required count for holding
    the table of secure stop identifiers.
  - If @a pxIdentifiers describes a valid memory block but that is too short
    to hold the table of secure stop identifiers, the function fails resulting
    with ::NV_ASM_ERROR_BUFFER_TOO_SHORT but @a pxCount has been set with the
    required count for holding the table of secure stop identifiers.

  @param[in]     xApplicationSession
  Application session identifier.

  @param[inout] pxCount
  Number of secure stop identifiers returned with @a pxIdentifiers.

  @param[inout] pxIdentifiers
  Table of secure stop identifiers.
  The number of secure stop identifiers is provided in @a pxCount.

  @retval ::NV_ASM_SUCCESS
  The list of secure stop identifiers has been provided back.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.
  + @a pxCount is @c NULL.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxIdentifiers is too short.
  @a pxCount has been updated with the required number of secure stop
  identifiers to allocate.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @warning
  Secure stop objects are related to the operator identifier.
  This identifier is extracted from the operator vault provided during
  application session creation -- refer to nvAsmOpen().
  Therefore several application sessions configured with the same operator
  vault and consequently related to the same operator identifier share the
  same internal secure stop object list.

  @see nvAsmGetSecureStopReports(), nvAsmDeleteSecureStops().
*/

NV_PUBLIC_API uint32_t nvAsmGetSecureStopIdentifiers
(
  TNvSession             xApplicationSession,
  uint32_t*             pxCount,
  TNvLicenseIdentifier* pxIdentifiers
);

/**
  @brief
  Export the secure stop report message to send to the @PROD servers.

  @pre
  The provided application session must be valid.

  @post
  None.

  @details

  This function allows to fetch a secure stop report for a list of secure stop
  identifiers provided with @a pxIdentifiers and @a pxCount related to
  @a xApplicationSession to present to the @PROD server.
  If @a xCount is @c 0 and @a pxIdentifiers is @c NULL, all secure stop objects
  are exported within the message.

  The exported secure stop report is returned in the memory block provided with
  @a pxReport:
  - If @a pxReport describes a valid memory block that can hold the secure stop
    report, the latter is copied in the memory block and the @a size is set to
    the size of data copied.
  - If the @a data field of the report is @c NULL, the function returns
    successfully but does not copy anything and it sets the @a size field to
    the required size for holding the secure stop report.
  - If @a pxReport describes a valid memory block but that is too short to
    hold the secure stop information, the function fails resulting with
    ::NV_ASM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the
    required size for holding the related secure stop report.

  @warning
  If active secure stop objects are present in the list of secure stop objects
  to report, they are stopped (enforce) prior to report generation.

  @note
  The provided report message data is a null-terminated string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The message data format has the following  @ref p_asm_secure_stop_message_format "JSON schema" @n.

  @param[in]     xApplicationSession
  Application session identifier.

  @param[in]     xCount
  Number of secure stop identifiers to report.

  @param[in]    pxIdentifiers
  Table of secure stop identifiers to report.

  @param[out]   pxReport
  Secure stop report content.

  @retval ::NV_ASM_SUCCESS
  The operation processed successfully.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.
  + @a xCount is @c 0 and @a pxIdentifiers is not @c NULL.
  + @a xCount is not @c 0 and @a pxIdentifiers is @c NULL.
  + @a pxReport is @c NULL.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxReport is too short.
  The @a size field of @a pxReport has been updated with the required size to
  allocate.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @warning
  Secure stop objects are related to the operator identifier.
  This identifier is extracted from the operator vault provided during
  application session creation -- refer to nvAsmOpen().
  Therefore several application sessions configured with the same operator
  vault and consequently related to the same operator identifier share the
  same internal secure stop object list.

  @see nvAsmGetSecureStopIdentifiers(), nvAsmDeleteSecureStops().
*/

NV_PUBLIC_API uint32_t nvAsmGetSecureStopReports
(
  TNvSession             xApplicationSession,
  uint32_t               xCount,
  TNvLicenseIdentifier* pxIdentifiers,
  TNvBuffer*            pxReport
);

/**
  @brief
  Delete secure stop objects.

  @pre
  The provided application session must be valid.

  @post
  All referred secure stop objects do no more exist.

  @details
  All secure stop objects identified in the list specified with
  @a pxIdentifiers and @a xCount and related to @a xApplicationSession have
  been deleted.
  If @a pxIdentifiers is @c NULL and @a xCount is @c 0,
  all available secured stops related to @a xApplicationSession are deleted
  whatever their state is.

  @param[in]     xApplicationSession
  Application session identifier.

  @param[in]     xCount
  Number of secure stop identifiers to delete.

  @param[in]    pxIdentifiers
  Table of secure stop identifiers to delete.

  @retval ::NV_ASM_SUCCESS
  The operation processed successfully.
  All specified secure stop objects are no more present whether they were
  present or not before the function call.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.
  + @a xCount is @c 0 and @a pxIdentifiers is not @c NULL.
  + @a xCount is not @c 0 and @a pxIdentifiers is @c NULL.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

  @warning
  Secure stop objects are related to the operator identifier.
  This identifier is extracted from the operator vault provided during
  application session creation -- refer to nvAsmOpen().
  Therefore several application sessions configured with the same operator
  vault and consequently related to the same operator identifier share the
  same internal secure stop object list.

  @see nvAsmGetSecureStopIdentifiers(), nvAsmGetSecureStopReports().
*/

NV_PUBLIC_API uint32_t nvAsmDeleteSecureStops
(
  TNvSession             xApplicationSession,
  uint32_t               xCount,
  TNvLicenseIdentifier* pxIdentifiers
);


/**
  @brief
  Allows the application to request a re-evaluation of the usage rules received in an associated descrambling session.

  @pre
  - The application has previously called @ref nvAsmSetOnUsageRulesListener() to set a valid usage rules
    listener.
  - The provided descrambling session must be valid and associated to the application session.
  - A Player call to the @ref nvDsmSetPrmContentMetadata() function on the provided descrambling session
    returned one of the following key status:
    - ::NV_KEY_STATUS_VALID
    - ::NV_KEY_STATUS_DENIED_BY_USER

  @post
  The application is called asynchronously on the @ref INvUsageRulesListener interface to re-evaluate
  the usage rules for the specified descrambling session.

  @details
  - This function provides an opportunity for the application to modify the status of the usage rules
    evaluation previously performed during the latest call to the @ref nvDsmSetPrmContentMetadata()
    function on the provided descrambling session.
  - This function does not perform the re-evaluation by itself. Rather, the re-evaluation is delegated
    to the @ref INvUsageRulesListener set by the application.
  - This function is non-blocking and does not wait for the usage rules re-evaluation result before
    returning.
  - The usage rules re-evaluation occurs only if a valid entitlement matching the metadata previously
    provided to the specified descrambling session can be found.
  - Whether such an entitlement is found or not, the usage rules re-evaluation process is subject to
    the same notifications as the ones generated by the @ref nvDsmSetPrmContentMetadata() function.

  @param[in]  xDescramblingSession
  The descrambling session for which usage rules are to be re-evaluated.

  @retval  ::NV_ASM_SUCCESS
  No error.

  @retval  ::NV_ASM_ERROR_BAD_PARAMETER
  The descrambling session is closed or invalid.

  @retval  ::NV_ASM_ERROR
  An unexpected error has occurred (asynchronous task creation failed, ...).

  @warning
  Being called asynchronously, the usage rules listener must not call any @PROD API function in order to
  prevent deadlocks.

  @see nvDsmSetPrmContentMetadata(), nvAsmSetOnUsageRulesListener(), INvUsageRulesListener
*/

NV_PUBLIC_API uint32_t nvAsmReconsiderUsageRules
(
    TNvSession xDescramblingSession
);

/**
  @brief
  Retrieve the information of all keyIDs of the persistent license.

  @pre
  The provided application session must be valid.

  @post
  None.

  @details
  This function shall retrieve the key status and Usage rule for all the keyIDs of all 
  the licenses persisted in the storage path.

  @attention
  Only the information of the key identifiers pertaining to non-DVB content entitlements 
  imported are returned by this function. Information of the key identifiers from DVB content 
  entitlements are <em>not</em> returned.

  These parameters are updated by the function:
  + The actual number of content key identifiers is provided in @a *pxCount.
  + The keyId, key status and usage rules are updated in  @a *pxKeyInfoTable.
    + Output Memory block returns the keyID information as an array of structure.
    + The exported usage rules data is a null-terminated string describing a 
      <a href="http://json.org/">JSON</a> object.
      The related @a size field does not include the terminating null character.
      The usage rules data format is described in the @ref p_dsm_ur page.

  If the @a pxKeyInfoTable parameter is not @c NULL, its @c data field address 
  can be valid or @c NULL:
  + If @c NULL, no data is copied but the @c size field is set with the 
    expected size for holding the whole key status and usage rules block. The operation is 
    considered successful.
  + If valid, the @c size field must be set with the size of the valid memory 
    block. The structure TNvKeyIdInfo updated with the key status and usage rules 
    if its size can hold the exported key information. The output memory block size is 
    adjusted to the actual number of bytes copied. If the memory block size is 
    too short, the operation failed with ::NV_ASM_ERROR_BUFFER_TOO_SHORT. In 
    that case, no data is copied but the memory block size is set with the 
    expected size required to hold the usage rules content.

  @param[in]     xApplicationSession
  Application session identifier.

  @param[out]    pxCount
  Number of key identifiers. Cannot be NULL.
  Set by application to the expected number of key identifiers; updated upon function return with
  actual number of available key identifiers.

  @param[inout]    pxKeyInfoTable
  Buffer to store the TNvKeyIdInfo structure. Cannot be @c NULL.
  Set by application to expected size of a key info table; updated upon function return with actual
  size of a buffer and data.

  @retval ::NV_ASM_SUCCESS
  The operation processed successfully.

  @retval ::NV_ASM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession does not refer a valid session.
  + @a pxCount is @c NULL.
  + @a pxKeyInfoTable is @c NULL.
  + @a pxKeyInfoTable is not @c NULL but @a *buffer is not @c NULL and *Size is @c 0.
  + @a pxKeyInfoTable is not @c NULL but @a *buffer is  @c NULL and *Size is not @c 0.

  @retval ::NV_ASM_ERROR_BUFFER_TOO_SHORT
  Operation could not be performed: buffer @a pxKeyInfoTable is not 
  large enough to store the key identifiers data.

  @retval ::NV_ASM_ERROR_NEED_PROVISIONING
  The application session provided with @a xApplicationSession parameter is
  not fully functional as relevant provisioned data misses.

  @retval ::NV_ASM_ERROR
  An unexpected error has occurred (memory resource ...).

*/

NV_PUBLIC_API uint32_t nvAsmGetStoredKeyIdentifiersData
(
  TNvSession   xApplicationSession,
  uint32_t*   pxCount,
  TNvBuffer*  pxKeyInfoTable
);

/* addtrogroup g_asm */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* defined NV_ASM_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
