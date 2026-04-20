/**
  @file prm_lds.h

  @brief
  This file defines the license delivery session interface.

  @details

  This interface is realized by the @Ma.
  It provides a @PROD license session service including challenge exportation 
  and license importation.

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
  @addtogroup g_lds
  @brief
  Describe the License Delivery Session Manager interface of @Ma.

  @details

  The <b>License delivery Session manager</b> interface introduces definition 
  for exporting challenge message and importing license response. Please make 
  sure to have read the documentation pages for a complete description of the 
  interface constraints and requirements.
*/

#ifndef NV_LDS_H
#define NV_LDS_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Dependencies                                                               */
/* ========================================================================== */

#include "prm_client.h"
#include "prm_dsm.h"

/* ========================================================================== */
/* Definitions                                                                */
/* ========================================================================== */

/**
  @addtogroup g_lds
  @{
*/

/**
  @brief
  Values for license delivery session operation result.

  @details
  This enumeration provides all the possible values that can be returned by a
  license delivery session operation.
*/

typedef enum
{
  NV_LDS_SUCCESS                  = 0x00010000L,
  /**< The license delivery session operation has been successful. */
  NV_LDS_ERROR_BAD_PARAMETER      = 0x00010001L,
  /**< The license delivery session operation failed: One or more provided parameters are invalid. */
  NV_LDS_ERROR_BUFFER_TOO_SHORT   = 0x00010002L,
  /**< The license delivery session operation failed: A provided memory block is too short to be filled in with the expected result. */
  NV_LDS_ERROR_NO_OPERATOR        = 0x00010003L,
  /**< The license delivery session operation failed: The related application session identifier does not refer a valid application session. */
  NV_LDS_ERROR_NO_RESULTS         = 0x00010004L,
  /**< The license delivery session operation failed: No operation providing results has been processed. */
  NV_LDS_ERROR_PROCESSING         = 0x00010005L,
  /**< The license delivery session operation failed: A related operation is still processing. */
  NV_LDS_ERROR_BAD_METADATA       = 0x00010006L,
  /**< The license delivery session operation failed: Some metadata are invalid. */
  NV_LDS_ERROR_NEED_PROVISIONING  = 0x00010007L,
  /**< The license delivery session operation failed: The @PROD client need to be provisioned. */
  NV_LDS_ERROR                    = 0x0001FFFFL
  /**< The license delivery session operation failed: An unexpected error occurred. */
}
TNvLdsResult;

/**
  @brief
  Values for message importation general status.

  @details
  This enumeration provides all the possible values that can be set in the 
  general status of the ::TNvLdsStatus structure.
*/

typedef enum
{
  NV_LDS_IMPORT_SUCCESS                   = 0,
  /**< License delivery message has been imported successfully. */
  NV_LDS_IMPORT_ERROR_INVALID_MESSAGE     = 1,
  /**< License delivery message has an invalid format. */
  NV_LDS_IMPORT_ERROR_INVALID_OBJECT      = 2,
  /**< License delivery message includes at least one invalid object. */
  NV_LDS_IMPORT_ERROR_RESOURCE            = 3,
  /**< License delivery message importation has encountered errors with resource e.g. storage resource, memory resource ... */
  NV_LDS_IMPORT_ERROR                     = -1
  /**< License delivery message importation has encountered unexpected errors. */
}
TNvLdsImportStatus;

/**
  @brief
  License identifier.
**/

typedef TNvCharUuid TNvLicenseIdentifier;

/* addtrogroup g_lds */
/** @} */

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */

/**
  @addtogroup g_lds
  @{
*/

/**
  @brief
  Describes the status of a license message importation processing.
*/

typedef struct
{
  uint32_t    status;
  /**< Status of the message importation -- refer to ::TNvLdsImportStatus. */
  uint32_t    objectCount;
  /**< Number of processed objects. */
  uint32_t    errorCount;
  /**< Number of processed objects with error(s). */
}
TNvLdsStatus;

/* addtrogroup g_lds */
/** @} */

/* ========================================================================== */
/* Functions                                                                  */
/* ========================================================================== */

/**
  @addtogroup g_lds
  @{
*/

/**
  @brief
  Open an license delivery session and configure it for a specific operator.

  @pre
  The @Xa must have been successfully initialized.

  @post
  A license delivery session has been created and associated to the provided 
  application session. A unique license delivery session identifier has been 
  generated and provided back.

  This function allows opening a new license delivery session. The new license 
  delivery session is associated to the application session provided in 
  @a pxApplicationSession.

  The new license delivery session identifier is provided back in 
  @a *pxDeliverySession. This parameter must not be @c NULL. The operation 
  failed resulting with ::NV_LDS_ERROR_BAD_PARAMETER if it is @c NULL. If the 
  operation failed, ::NV_SESSION_INVALID value is provided back in 
  @a *pxDeliverySession.

  It fails also with ::NV_LDS_ERROR_NO_OPERATOR if the @a xApplicationSession 
  parameter does not refer a valid application session.

  @param[out] pxDeliverySession
  Reference to an license delivery session identifier. 
  It shall not be @c NULL.
  The referred value is set with the new application session identifier or 
  with the ::NV_SESSION_INVALID value if an error occurred.

  @param[in] xApplicationSession
  Application session identifier to associate with the new license delivery 
  session. This identifier must be valid.

  @retval ::NV_LDS_SUCCESS
  A new license delivery session has been opened and associated to the 
  application session identifier.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid @Ma 
  session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  @a pxDeliverySession parameter is @c NULL.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsClose(), @ref p_asm_appid.
*/

NV_PUBLIC_API uint32_t nvLdsOpen
(
  TNvSession* pxDeliverySession,
  TNvSession   xApplicationSession
);

/**
  @brief
  Close a license delivery session.

  @pre
  None.

  @post
  The provided license delivery session -- whether valid or not -- is 
  considered closed and its identifier shall no more be used.
  Internal license delivery session resources have been released.

  This function closes a license delivery session referred by 
  @a xDeliverySession and previously opened with nvLdsOpen(). The operation 
  releases all session-related resources. This operation shall never fail 
  whether the provided license delivery session identifier is valid or not.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @see nvLdsOpen().
*/

NV_PUBLIC_API void nvLdsClose
(
  TNvSession xDeliverySession
);

/**
  @brief
  Store external context reference within a license delivery session.

  @pre
  A license delivery session must have been successfully opened.

  @post
  The opaque reference on external context data is stored in the provided 
  license delivery session.

  This function allows storing an opaque external context reference provided by 
  @a xContext within the license delivery session identified by 
  @a xDeliverySession. This reference can be @c NULL in which case no more 
  opaque context reference is maintained with the license delivery session.

  This reference can be later retrieved from the session using 
  nvLdsGetContext().

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[in]    xContext
  External opaque context reference.

  @retval ::NV_LDS_SUCCESS
  @a xContext has been successfully stored in the license delivery session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession is not valid.

  @retval ::NV_LDS_ERROR
  An unexpected error occurs during processing.

  @see nvLdsOpen(), nvLdsGetContext().
*/

NV_PUBLIC_API uint32_t nvLdsSetContext
(
  TNvSession  xDeliverySession,
  TNvHandle   xContext
);

/**
  @brief
  Get external context reference stored in a license delivery session.

  @pre
  A license delivery session must have been successfully opened.

  @post
  None.

  This function allows retrieving the external opaque context reference value 
  previously provided with nvLdsSetContext(). The context reference is related 
  to the license delivery session identified by @a xDeliverySession. The value 
  is provided back using the value pointed by @a pxContext.

  The context reference value can be @a NULL if it has never been previously 
  set or if it has been previously set to a @a NULL value.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[out]  pxContext
  Pointer to an external opaque context reference.

  @retval ::NV_LDS_SUCCESS
  @a pxContext has been successfully filled in with the stored context 
  reference value.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession is not valid.
  + @a pxContext is @a NULL.

  @retval ::NV_LDS_ERROR
  An unexpected error occurs during processing.

  @see nvLdsOpen(), nvLdsSetContext().
*/

NV_PUBLIC_API uint32_t nvLdsGetContext
(
  TNvSession  xDeliverySession,
  TNvHandle* pxContext
);

/**
  @brief
  Set the client application data to include in the exported @PROD message.

  @pre
  The provided license delivery session must be valid.

  @post
  The client application clear data and protected data are registered in the 
  license delivery session.

  This function allows the client application to provide clear and protected 
  data in order to embed them in the @PROD message to export. The clear data is 
  provided in @a pxClearData and the protected data in @a pxProtectedData. 
  Providing application data is optional: Any of the provided data can be 
  @c NULL.

  Provided data is not analyzed by the @Ma. It is copied within the license 
  delivery session. If this function is called several times before the @PROD 
  message is exported using nvLdsExportMessage(), only the data related to the 
  last call to this function is taken into account. Any call to this function 
  replaces the data provided in the previous call. Calling this function with 
  @c NULL data parameters removes previously registered @PROD content metadata.

  The protected data is encrypted with @PROD protection before being embedded in 
  the exported @PROD message.

  @warning
  Although the provided client data is not analyzed by the @Ma, it is 
  intended to be inserted within <a href="http://www.json.org/">JSON</a> object 
  descriptions as a simple <a href="http://www.json.org/">JSON</a> string.
  Therefore the provided data must be compliant with the 
  <b><a href="http://www.json.org/">JSON</a> string</b> format e.g. character 
  set, escaped character ...

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[in]    pxClearData
  Application data to embed in clear in the @PROD message.

  @param[in]    pxProtectedData
  Application data to embed encrypted in the @PROD message.

  @retval ::NV_LDS_SUCCESS
  The client application clear and protected data provided with @a pxClearData 
  and @a pxProtectedData have been copied in the license delivery session.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation 
  does no more refer a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a pxClearData is not @c NULL but its @a data field is @c NULL 
    or its @a size field is @c 0.
  + @a pxProtectedData is not @c NULL but its @a data field is @c NULL 
    or its @a size field is @c 0.

  @retval ::NV_LDS_ERROR_PROCESSING
  A @PROD message importation is currently processing.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsOpen(), nvLdsExportMessage().
*/

NV_PUBLIC_API uint32_t nvLdsSetClientData
(
  TNvSession          xDeliverySession,
  const TNvBuffer*   pxClearData,
  const TNvBuffer*   pxProtectedData
);

/**
  @brief
  Set the @PROD content metadata to include in the exported @PROD message.

  @pre
  The provided license delivery session must be valid.

  @post
  The @PROD content metadata is registered in the license delivery session.

  This function allows the client application to provide @PROD content metadata 
  in order to embed it in the @PROD message to export. The @PROD content metadata 
  identifies specifically an entitlement to retrieve. Providing @PROD content 
  metadata is optional and can be @c NULL.

  Provided data is copied within the license delivery session. If this function 
  is called several times before the @PROD message is exported using 
  nvLdsExportMessage(), only the @PROD content metadata related to the last call 
  to this function is taken into account. Any call to this function replaces 
  the data provided in the previous call. Calling this function with 
  @a pxMetadata set to @c NULL removes previously registered @PROD content 
  metadata.

  @warning
  When this function is provided with a metadata of type ::NV_STREAM_TYPE_KEY_CONTEXT, 
  the @a data field of @a pxMetadata structure points to a ::TNvKeyContext structure.
  The @a prmSyntax field of the ::TNvKeyContext structure should not be @c NULL.
  If this field is @c NULL then this function returns ::NV_LDS_ERROR_BAD_METADATA 
  and the provided metadata structure is not taken into account.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[in]    pxMetadata
  @PROD content metadata.

  @param[in]    xStreamType
  Type of @PROD content metadata.

  @retval ::NV_LDS_SUCCESS
  The @PROD content metadata provided with @a pxMetadata has been copied in 
  the license delivery session.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation 
  does no more refer a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a pxMetadata is not @c NULL but its @a data field is @c NULL 
    or its @a size field is @c 0.
  + @a pxMetadata is not @c NULL but @a xStreamType value is not valid.

  @retval ::NV_LDS_ERROR_BAD_METADATA
  Provided metadata cannot be taken into account e.g. incomplete metadata ...
  
  @retval ::NV_LDS_ERROR_PROCESSING
  A @PROD message importation is currently processing.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsExportMessage().
*/

NV_PUBLIC_API uint32_t nvLdsUsePrmContentMetadata
(
  TNvSession          xDeliverySession,
  const TNvBuffer*   pxMetadata,
  TNvStreamType       xStreamType
);

/**
  @brief
  Set the @PROD license identifiers to include in the exported @PROD message.

  @pre
  The provided license delivery session must be valid.

  @post
  The license identifiers are registered in the license delivery session.

  This function allows the client application to provide @PROD license 
  identifiers in  order to embed them in the @PROD message to export.
  The @PROD license identifiers define entitlements to replace.
  Providing @PROD license identifiers is optional and can be @c NULL.

  Provided data is copied within the license delivery session. If this function 
  is called several times before the @PROD message is exported using 
  nvLdsExportMessage(), only the @PROD license identifiers related to the last 
  call to this function is taken into account. Any call to this function 
  replaces the data provided in the previous call. Calling this function with 
  @a xCount set to zero and @a pxIdentifiers set to @c NULL removes previously 
  registered @PROD license identifiers.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[in]    xCount
  Number of @PROD license identifiers.

  @param[in]    pxIdentifiers
  Table of @PROD license identifiers.

  @retval ::NV_LDS_SUCCESS
  The @PROD identifiers provided with @a pxIdentifiers have been copied in 
  the license delivery session.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation 
  does no more refer a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a xCount is @c 0 and @a pxIdentifiers is not @c NULL.
  + @a xCount is not @c 0 and @a pxIdentifiers is @c NULL.
  

  @retval ::NV_LDS_ERROR_PROCESSING
  A @PROD message importation is currently processing.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsExportMessage().
*/

NV_PUBLIC_API uint32_t nvLdsUseLicenseIdentifiers
(
  TNvSession             xDeliverySession,
  uint32_t               xCount,
  TNvLicenseIdentifier* pxIdentifiers
);

/**
  @brief
  Export the @PROD message to send to the @PROD server.

  @pre
  The provided license delivery session must be valid.

  @post
  None.

  This function allows to fetch the @PROD message -- generally including a @PROD 
  challenge -- to present to the @PROD server. The response of the @PROD server 
  shall be imported in the same license delivery session using 
  nvLdsImportMessage().

  The @PROD message is built with the configuration data -- e.g. application data, 
  @PROD content metadata, etc... -- currently set in the license delivery session. 
  Changing this configuration data results to a new challenge message.

  When @PROD content metadata has been provided to license delivery session using 
  nvLdsUsePrmContentMetadata() for license post-delivery, it is actually 
  checked during message exportation. Therefore if provided metadata is invalid, 
  the function fails returning with ::NV_LDS_ERROR_BAD_METADATA. In that case, 
  no message is generated and exported.

  The exported @PROD message is returned in the memory block provided with 
  @a pxMessage:
  - If @a pxMessage describes a valid memory block that can hold the @PROD 
    message, the latter is copied in the memory block and the @a size is set to 
    the size of data copied.
  - If the @a data field of the message is @c NULL, the function returns 
    successfully but does not copy anything and it sets the @a size field to 
    the required size for holding the @PROD message.
  - If @a pxMessage describes a valid memory block but that is too short to 
    hold the @PROD message, the function fails resulting with 
    ::NV_LDS_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the 
    required size for holding the @PROD message.

  @note
  The provided message data is a null-terminated string describing a 
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The message data format is described in @ref p_nva_refdoc "[SSP Portal]".
  The challenge data is the @a challenge field of this message.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[out]   pxMessage
  Buffer to be filled in with the exported message.

  @retval ::NV_LDS_SUCCESS
  The @PROD message has been successfully copied in @a pxMessage.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation
  does no more refer a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a pxMessage is @c NULL.

  @retval ::NV_LDS_ERROR_PROCESSING
  A @PROD message importation is currently processing.

  @retval ::NV_LDS_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxMessage is too short.

  @retval ::NV_LDS_ERROR_BAD_METADATA
  The previously provided @PROD content metadata are invalid.

  @retval ::NV_LDS_ERROR_NEED_PROVISIONING
  Relevant @PROD provisioned data is missing.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsImportMessage().
*/

NV_PUBLIC_API uint32_t nvLdsExportMessage
(
  TNvSession  xDeliverySession,
  TNvBuffer* pxMessage
);

/**
  @brief
  Import a @PROD message.

  @pre
  The provided license delivery session must be valid.

  @post
  The @PROD message has been taken into account. The entitlements present in it 
  will be imported asynchronously.

  The @PROD message to import is generally a message received from the @PROD server 
  in response to a previously challenge message built by the @Ma using 
  nvLdsExportMessage(). This message is provided to the @Ma in the 
  @a pxMessage parameter.

  As the data content of the message -- e.g. mainly entitlements -- can be 
  voluminous, it is actually imported after the call to this function. The 
  operation results can retrieved at the end of the operation using 
  nvLdsGetResults(). End of the operation can be polled using this previous 
  function or it can be monitored using a notification function to register 
  with nvLdsSetOnCompleteListener().

  The memory allocated to the @a pxMessage can be freed as soon as the function 
  returns. The message data has been copied internally.
  
  Once the importation is complete and the results have been analyzed, the 
  license delivery session should be closed.

  @note
  The message data to import is a <a href="http://json.org/">JSON</a> object. 
  The message data format is described in @ref p_nva_refdoc "[SSP Portal]".
  The license data is the @a entitlements field of this message.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[in]    pxMessage
  Buffer containing the message to import.

  @retval ::NV_LDS_SUCCESS
  The @PROD message response has been taken into account. The effective 
  importation process may be successful or may result with errors. 
  The actual importation errors can be retrieved with nvLdsGetResults().

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession 
  creation does no more refer a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a pxMessage is @c NULL.
  + @a pxMessage is not @c NULL but its @a data field is @c NULL or 
    its @a size field is @c 0.

  @retval ::NV_LDS_ERROR_PROCESSING
  A @PROD message importation is currently processing.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsExportMessage(), nvLdsSetOnCompleteListener(), nvLdsGetResults().
*/

NV_PUBLIC_API uint32_t nvLdsImportMessage
(
  TNvSession          xDeliverySession,
  const TNvBuffer*   pxMessage
);

/**
  @brief
  Get the results of a @PROD message importation.

  @pre
  The provided license delivery session must be valid.

  @post
  None.

  This function allows retrieving @PROD message importation basic results only if 
  this importation operation is complete. If it not complete, this operation 
  does not provide any results and returns ::NV_LDS_ERROR_PROCESSING. The 
  completion of the @PROD message importation operation can be monitored using a 
  notification function registered on the license delivery session using 
  nvLdsSetOnCompleteListener().

  The results provided back are related to the last complete message 
  importation. It does not provide the results of all message importations 
  processed with the license delivery session identified by @a xDeliverySession 
  but only the last processed one.

  Message importation general results are described with @a pxStatus parameter. 
  This parameter must always refer to a valid structure.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[out]   pxStatus
  General status data.

  @retval ::NV_LDS_SUCCESS
  The importation results have been successfully copied in @a pxStatus.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation 
  does no more refer a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a pxStatus is @c NULL.

  @retval ::NV_LDS_ERROR_NO_RESULTS
  No @PROD message has been imported in the current session.

  @retval ::NV_LDS_ERROR_PROCESSING
  The related @PROD message importation is still processing.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsImportMessage(), nvLdsSetOnCompleteListener().
*/

NV_PUBLIC_API uint32_t nvLdsGetResults
(
  TNvSession      xDeliverySession,
  TNvLdsStatus*  pxStatus
);

/**
  @brief
  Export the list of key identifiers bound to a license delivery session.

  @pre
  The provided license delivery session must be valid.

  @post
  None.

  Bound entitlements are entitlements bound to the license delivery session used to import them into
  the @Xa.

  As entitlements can deliver content keys, bound content keys are by extension content keys delivered
  with bound entitlements.

  This function returns the list of identifiers of all the content keys bound to the license delivery
  session identified by @a xDeliverySession. This is typically useful for situations where e.g. several
  keys are available for the same content and the application needs to select a specific key among
  them.

  @attention
  Only key identifiers pertaining to non-DVB content entitlements imported in the license delivery
  session are returned by this function. Key identifiers from DVB content entitlements are <em>not</em>
  returned.

  Parameters @a *pxCount and @a *pxKeyIdentifierSize must be set to relevant values so that
  the @a pxKeyIdentifiersTable array can contain @a pxCount key identifiers of size
  @a *pxKeyIdentifierSize each.

  These parameters are updated by the function:
  + The actual number of content key identifiers is provided in @a *pxCount.
  + The actual size of a content key identifier is provided in @a *pxKeyIdentifierSize.
  + The list of bound content key identifiers is put into the memory array identified by
    @a pxKeyIdentifiersTable, provided that the corresponding application-allocated memory
    block can contain that list.

  Application can determine the size of the memory block to allocate for parameter 
  @a pxKeyIdentifiersTable by first calling this function with valid (i.e. non @c NULL) @a pxCount
  and @a pxKeyIdentifierSize parameters, and with parameter @a pxKeyIdentifiersTable set to 
  @c NULL. The status ::NV_LDS_SUCCESS is then returned with @a *pxCount and @a *pxKeyIdentifierSize
  providing respectively the available number of bound key identifiers and size of a key identifier.
  Application can then allocate a memory block of at least <em>*pxCount x *pxKeyIdentifierSize</em>
  bytes to parameter @c pxKeyIdentifiersTable before calling again the functions to obtain the actual
  data.

  If the provided @a pxKeyIdentifiersTable memory array is not large enough to hold all the available
  bound key identifiers, ::NV_LDS_ERROR_BUFFER_TOO_SHORT is returned and @a *pxCount and
  @a *pxKeyIdentifierSize are updated respectively with the number of key identifiers available and the
  key identifier size.

  If no bound content keys are available in the session identified by @a xDeliverySession, then 
  @a *pxCount and @a *pxKeyIdentifierSize are set to @c 0; @a pxKeyIdentifiersTable array content is
  not relevant and should be ignored by the application.

  Application can locate each individual bound content key identifier returned in the
  @a pxKeyIdentifiersTable array by using an index value computed with the formula:
                    <center><em>n x *pxKeyIdentifierSize</em></center>
  with <em>0 &le; n &le; *pxCount - 1</em>.

  @note
  Content key identifiers are returned in binary form -- e.g. when mapped on an UUID, the key
  identifier size is 16.

  @param[in]    xDeliverySession
  License delivery session identifier for which bound content key identifiers are to be returned.

  @param[inout]   pxCount
  Number of bound key identifiers. Cannot be NULL.
  Set by application to the expected number of key identifiers; updated upon function return with
  actual number of available key identifiers.

  @param[inout]   pxKeyIdentifierSize
  Size of a key identifier. Cannot be @c NULL.
  Set by application to expected size of a key identifier; updated upon function return with actual
  size of a key identifier (see note above).

  @param[out]   pxKeyIdentifiersTable
  Address of the array of key identifiers bound to session @a xDeliverySession. Must be allocated by
  application. Can be set to @c NULL in order to retrieve size information for subsequent array
  allocation.

  @retval ::NV_LDS_SUCCESS
  The operation has been successfully performed, relevant results are available.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation does no more refer
  to a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid license delivery session.
  + @a pxCount is @c NULL.
  + @a pxKeyIdentifierSize is @c NULL.
  + @a pxKeyIdentifiersTable is not @c NULL but @a *pxCount x @a *pxKeyIdentifierSize is @c 0.

  @retval ::NV_LDS_ERROR_BUFFER_TOO_SHORT
  Operation could not be performed: buffer @a pxKeyIdentifiersTable of size 
  @a *pxCount x @a *pxKeyIdentifierSize is not large enough to contain the actual number of 
  key identifiers to export.

  @retval ::NV_LDS_ERROR_PROCESSING
  A @PROD message importation is still being processed in this session.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (@Xa not initialized, memory resource ...).
*/

NV_PUBLIC_API uint32_t nvLdsExportBoundKeyIdentifiers
(
  TNvSession         xDeliverySession,
  uint32_t*         pxCount,
  uint32_t*         pxKeyIdentifierSize,
  uint8_t*          pxKeyIdentifiersTable
);

/**
  @brief
  License delivery session listener.

  @pre
  An event occurs on a license delivery session.

  @post
  The notified event is considered processed and is not maintained.

  This type defines a notification function callback for license delivery
  session events.

  @param[in]    xDeliverySession
  Identifies the license delivery session related to the event.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not 
  processed by the license delivery session.

  @see nvLdsSetOnCompleteListener.
*/

typedef bool_t (*INvLicenseDeliveryListener)
(
  TNvSession xDeliverySession
);

/**
  @brief
  Define a listener callback function for license import completion event.

  @pre
  The provided license delivery session must be valid.

  @post
  The listener function is registered for the license delivery session.

  After being registered, the listener callback function is triggered each time 
  the @Ma achieves a license importation in the license delivery session. 
  On completion, importation results are available and can be fetched using 
  nvLdsGetResults().

  The listener registration can be canceled by calling this function again 
  with a @c NULL listener parameter. If no listener is currently registered, a 
  call with a @c NULL listener value does simply nothing.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @param[in]    onComplete
  Listener callback function to register.

  @retval ::NV_LDS_SUCCESS
  The listener callback function has been registered in the license delivery 
  session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLdsOpen(), nvLdsImportMessage(), nvLdsGetResults().
*/

NV_PUBLIC_API uint32_t nvLdsSetOnCompleteListener
(
  TNvSession                  xDeliverySession,
  INvLicenseDeliveryListener  onComplete
);

/**
  @brief
  Return the related application session identifier.

  @pre
  The provided license delivery session must be valid.

  @post
  None.

  @param[in]    xDeliverySession
  License delivery session identifier.

  @return
  The function returns the application session identifier value associated to 
  the license delivery session. It returns ::NV_SESSION_INVALID if the 
  @a xDeliverySession parameter does not refer a valid license delivery session.

  @see nvLdsOpen().
*/

NV_PUBLIC_API TNvSession nvLdsGetApplicationSession
(
  TNvSession xDeliverySession
);

/**
  @brief
  Retrieve the information of all keyIDs bounded to a license
  delivery session.

  @pre
  The provided license delivery session must be valid.

  @post
  None.

  @details
  This function shall retrieve the key status and usage rule status of all keyIDs of all 
  the licenses bounded to a license delivery session identified by xDeliverySession.

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

  @param[in]     xDeliverySession
  License delivery session identifier.

  @param[out]    pxCount
  Number of bound key identifiers. Cannot be NULL.
  Set by application to the expected number of key identifiers; updated upon function return with
  actual number of available key identifiers.

  @param[inout]    pxKeyInfoTable
  Buffer to store the TNvKeyIdInfo structure. Cannot be @c NULL.
  Set by application to expected size of a key info table; updated upon function return with actual
  size of a buffer and data.

  @retval ::NV_LDS_SUCCESS
  The operation processed successfully.

  @retval ::NV_LDS_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDeliverySession creation does no more refer
  to a valid application session.

  @retval ::NV_LDS_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDeliverySession does not refer a valid session.
  + @a pxCount is @c NULL.
  + @a pxKeyInfoTable is @c NULL.
  + @a pxKeyInfoTable is not @c NULL but @a *buffer is not @c NULL and *Size is @c 0.
  + @a pxKeyInfoTable is not @c NULL but @a *buffer is  @c NULL and *Size is not @c 0.

  @retval ::NV_LDS_ERROR_BUFFER_TOO_SHORT
  Operation cound not be performed: buffer @a pxKeyInfoTable is not 
  large enough to store the key identifiers data.

  @retval ::NV_LDS_ERROR
  An unexpected error has occurred (memory resource ...).

*/

NV_PUBLIC_API uint32_t nvLdsGetBoundKeyIdentifiersData
(
  TNvSession   xDeliverySession,
  uint32_t*   pxCount,
  TNvBuffer*  pxKeyInfoTable
);

/* addtrogroup g_lds */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* defined NV_LDS_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
