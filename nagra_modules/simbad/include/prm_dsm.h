/**
  @file prm_dsm.h

  @brief
  This file defines the @PROD descrambling session interface.

  @details

  This interface is realized by the @Ma.
  It provides a descrambling management service.

  COPYRIGHT:
    2013 - 2017 Nagravision S.A.
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
  @addtogroup g_dsm
  @brief
  Describe the Descrambling Session Manager interface of @Ma.

  @details

  The <b>Descrambling Session manager</b> interface introduces definition for 
  managing content decryption. It includes a content processing part and a 
  stream processing part. It especially ensures internal key usage rules 
  enforcements. Please make sure to have read the documentation pages for a 
  complete description of the interface constraints and requirements.
*/

#ifndef NV_DSM_H
#define NV_DSM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Dependencies                                                               */
/* ========================================================================== */

#include "prm_client.h"

/* ========================================================================== */
/* Definitions                                                                */
/* ========================================================================== */

/**
  @addtogroup g_dsm
  @{
*/

/**
  @brief
  Values for descrambling session operation result.

  @details
  This enumeration provides all the possible values that can be returned by a
  descrambling session operation.
*/

typedef enum 
{
  NV_DSM_SUCCESS                  = 0x00020000L,
  /**< The descrambling session operation has been successful. */
  NV_DSM_ERROR_BAD_PARAMETER      = 0x00020001L,
  /**< The descrambling session operation failed: One or more provided parameters are invalid. */
  NV_DSM_ERROR_NO_KEY             = 0x00020002L,
  /**< The descrambling session operation failed: The related key cannot be found. */
  NV_DSM_ERROR_KEY_STATUS         = 0x00020003L,
  /**< The descrambling session operation failed: The related key cannot be used. */
  NV_DSM_ERROR_UNSUPPORTED_EMI    = 0x00020004L,
  /**< The descrambling session operation failed: The provided encryption method indicator (EMI) is not supported. */
  NV_DSM_ERROR_CRYPTOENGINE       = 0x00020005L,
  /**< The descrambling session operation failed: A cryptographic operation failed. */
  NV_DSM_ERROR_BUFFER_TOO_SHORT   = 0x00020006L,
  /**< The descrambling session operation failed: A provided memory block is too short to be filled in with the expected result. */
  NV_DSM_ERROR_NO_OPERATOR        = 0x00020007L,
  /**< The descrambling session operation failed: The related application session identifier does not refer a valid application session. */
  NV_DSM_ERROR                    = 0x0002FFFFL
  /**< The descrambling session operation failed: An unexpected error occurred. */
}
TNvDsmResult;

/**
  @brief
  Values for key status.

  @details
  This enumeration provides all the possible key status values.
*/

typedef enum 
{
  NV_KEY_STATUS_VALID                         = 0,
  /**< The related key(s) is valid e.g. the access to content is granted */
  NV_KEY_STATUS_EXPIRED                       = 1,
  /**< The related key(s) is expired 
       e.g. the access to content is denied because the validity period is in the past. */
  NV_KEY_STATUS_BEFORE_VALIDITY_PERIOD        = 2,
  /**< The related key(s) validity period has not started
       e.g. the access to content is denied because the validity period is in the future and has not begun. */
  NV_KEY_STATUS_NOT_ENTITLED                  = 3,
  /**< The related key(s) cannot be used 
       e.g. the access to content is denied because the check of Nagra 
       internal rules other than validity period failed. */
  NV_KEY_STATUS_NOT_PRESENT                   = 4,
  /**< The related key(s) does not exist e.g. the access to content is denied 
       because the related entitlement is missing */
  NV_KEY_STATUS_DENIED_USAGE_RULES_VIOLATION  = 5,
  /**< The related key(s) usage has been denied due to internal usage rules violation. */
  NV_KEY_STATUS_DENIED_BY_USER                = NV_KEY_STATUS_DENIED_USAGE_RULES_VIOLATION,
  /**< The related key(s) usage has been denied by client application 
       e.g. the access to content is denied because the usage rules control 
       by application session has been denied. */
  NV_KEY_STATUS_INVALID                       = 6,
  /**< The provided key(s) or their protection are invalid 
       e.g. the access to content is denied entitlement is inconsistent, corrupted ... */
  NV_KEY_STATUS_UNKNOWN                       = 7,
  /**< The related key status is unknown or irrelevant
       e.g. the key is aging and should still be usable for last related buffered content chunks ... */
  NV_KEY_STATUS_DENIED_USAGE_RULES_TRANSITION = 8
  /**< The related key(s) usage has been denied due to usage rules transition. 
       e.g. when the Usage rule transition from NULL to Valid or vice versa.*/
}
TNvKeyStatus;


/**
  @brief
  Invalid transport session identifier value.
*/

#define NV_DSM_TRANSPORT_SESSION_IDENTIFIER_INVALID     ((uint32_t)(-1))

/**
  @brief
  Invalid encryption method indicator (EMI) value.
*/

#define NV_DSM_EMI_INVALID                              ((uint16_t)(-1))


/* addtrogroup g_dsm */
/** @} */

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */

/* ========================================================================== */
/* Functions                                                                  */
/* ========================================================================== */

/* -------------------------------------------------------------------------- */
/* Descrambling Management                                                    */
/* -------------------------------------------------------------------------- */

/**
  @addtogroup g_dsm
  @{
*/

/**
  @name Descrambling session management
  Functions included in this group allow managing the descrambling session 
  common elements.
  @{
*/

/**
  @brief
  Open a descrambling session.

  @pre
  - The @Ma must have been initialized.
  - A valid application session must exist.

  @post
  A new descrambling session has been opened and associated to the provided 
  transport session identifier. Internal descrambling session resources have 
  been allocated.

  This function allows opening a new descrambling session. The new descrambling 
  session is associated to the following identifiers:
  - The application session identifier is provided in @a xApplicationSession. 
    Refer to @ref p_asm_appid for further details.
  - The transport session identifier is provided in 
    @a xTransportSessionIdentifier. Refer to @ref p_dsm_tsid for further details.

  The new descrambling session identifier is provided back in 
  @a *pxDescramblingSession. This parameter must not be @c NULL. The operation 
  failed resulting with ::NV_DSM_ERROR_BAD_PARAMETER if it is @c NULL or the 
  provided @a xTransportSessionIdentifier is already assigned to an existing 
  descrambling session. If the operation failed, a ::NV_SESSION_INVALID value 
  is provided back in @a *pxDescramblingSession.

  It fails also with ::NV_DSM_ERROR_NO_OPERATOR if the @a xApplicationSession 
  does not refer a valid @Ma session.

  @param[out]   pxDescramblingSession
  Reference to a descrambling session identifier. 
  It shall not be @c NULL.
  The referred value is set with the new descrambling session identifier or 
  with the ::NV_SESSION_INVALID value if an error occurred.

  @param[in]    xApplicationSession
  Application session identifier to associate with the new descrambling 
  session. This identifier must be valid.

  @param[in]    xTransportSessionIdentifier
  Transport session identifier to associate with the new descrambling session.

  @param[in]    xEmi
  Encryption method indicator (a.k.a. EMI). Refer to the @ref p_dsm_emi_handling 
  page for additional information about the EMI handling with regard to whether 
  the Player knows or does not know which EMI value to use.

  @retval ::NV_DSM_SUCCESS
  A new descrambling session has been opened and associated to the provided 
  transport session identifier.

  @retval ::NV_DSM_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid 
  application session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  - The @a xApplicationSession parameter refers to an invalid session.
  - The @a pxDescramblingSession parameter is @c NULL.
  - The @a xTransportSessionIdentifier is already assigned to an existing 
    descrambling session.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvDsmClose(), @ref p_dsm_manage, @ref p_dsm_tsid, @ref p_dsm_emi_handling.
*/

NV_PUBLIC_API uint32_t nvDsmOpen
(
  TNvSession*  pxDescramblingSession,
  TNvSession    xApplicationSession,
  uint32_t      xTransportSessionIdentifier,
  uint16_t      xEmi
);

/**
  @brief
  Close a descrambling session.

  @pre
  None.

  @post
  The provided descrambling session -- whether valid or not -- is considered 
  closed and its identifier shall no more be used. Internal descrambling 
  session resources have been released.

  This function closes a descrambling session referred by 
  @a xDescramblingSession and previously opened with nvDsmOpen(). 
  The operation releases all session-related resources and shall never fail
  whether the provided descrambling session identifier is valid or not.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @see nvDsmOpen(), @ref p_dsm_tsid, @ref p_dsm_manage.
*/

NV_PUBLIC_API void nvDsmClose
(
  TNvSession  xDescramblingSession
);

/**
  @brief
  Store external context reference within a descrambling session.

  @pre
  A descrambling session must have been successfully opened.

  @post
  The opaque reference on external context data is stored in the provided 
  descrambling session.

  This function allows storing an opaque external context reference provided by 
  @a xContext within the descrambling session identified by 
  @a xDescramblingSession. This reference can be @c NULL in which case no more 
  opaque context reference is maintained with the descrambling session.

  This reference can be later retrieved from the session using 
  nvDsmGetContext().

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    xContext
  External opaque context reference.

  @retval ::NV_DSM_SUCCESS
  @a xContext has been successfully stored in the descrambling session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession is not valid.

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing.

  @see nvDsmOpen(), nvDsmGetContext().
*/

NV_PUBLIC_API uint32_t nvDsmSetContext
(
  TNvSession  xDescramblingSession,
  TNvHandle   xContext
);

/**
  @brief
  Get external context reference stored in a descrambling session.

  @pre
  A descrambling session must have been successfully opened.

  @post
  None.

  This function allows retrieving the external opaque context reference value 
  previously provided with nvDsmSetContext(). The context reference is related 
  to the descrambling session identified by @a xDescramblingSession. The value 
  is provided back using the value pointed by @a pxContext.

  The context reference value can be @a NULL if it has never been previously 
  set or if it has been previously set to a @a NULL value.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[out]  pxContext
  Pointer to an external opaque context reference.

  @retval ::NV_DSM_SUCCESS
  @a pxContext has been successfully filled in with the stored context 
  reference value.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession is not valid.
  + @a pxContext is @a NULL.

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing.

  @see nvDsmOpen(), nvDsmSetContext().
*/

NV_PUBLIC_API uint32_t nvDsmGetContext
(
  TNvSession  xDescramblingSession,
  TNvHandle* pxContext
);

/**
  @brief
  Descrambling session listener.

  @pre
  An event occurs on a descrambling session.

  @post
  The notified event is considered processed and is not maintained.

  This type defines a notification function callback for descrambling session 
  events.

  @param[in]    xDescramblingSession
  Identifies the descrambling session related to the event.

  @return
  The function shall return ::TRUE when successful and ::FALSE otherwise.
  The returned result is reserved for future use and is currently not 
  processed by the descrambling session.

  @see nvDsmSetNewKeyListener.
*/

typedef bool_t (*INvDescramblingListener)
(
  TNvSession xDescramblingSession
);

/**
  @brief
  Define a listener callback function for new key event.

  @pre
  The provided descrambling session must be valid.

  @post
  The listener function is registered for the descrambling session.

  After being registered, the listener callback function is triggered each time 
  the @Ta caches a new or updated key in the descrambling session. Usage 
  rules must be checked again using the right key identifier. Refer to 
  @ref p_dsm_decrypt_ott and @ref p_dsm_decrypt_dvb for further details on key 
  update monitoring.

  The listener registration can be canceled by calling this function again 
  with a @c NULL listener parameter. If no listener is currently registered, a 
  call with a @c NULL listener value does simply nothing.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    onNewKey
  Listener callback function to register.

  @retval ::NV_DSM_SUCCESS
  The listener callback function has been registered in the descrambling session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvDsmOpen(), nvDsmGetKeyInformation(),
       @ref p_dsm_decrypt_ott, @ref p_dsm_decrypt_dvb.
*/

NV_PUBLIC_API uint32_t nvDsmSetNewKeyListener
(
  TNvSession               xDescramblingSession,
  INvDescramblingListener  onNewKey
);

/**
  @brief
  Define a listener callback function for no valid key event.

  @pre
  The provided descrambling session must be valid.

  @post
  The listener function is registered for the descrambling session.

  After being registered, the listener callback function is triggered each time 
  the @Ta misses a valid key in the descrambling session.

  The listener registration can be canceled by calling this function again 
  with a @c NULL listener parameter. If no listener is currently registered, a 
  call with a @c NULL listener value does simply nothing.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    onNoValidKey
  Listener callback function to register.

  @retval ::NV_DSM_SUCCESS
  The listener callback function has been registered in the descrambling session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvDsmOpen().
*/

NV_PUBLIC_API uint32_t nvDsmSetOnNoValidKeyListener
(
  TNvSession               xDescramblingSession,
  INvDescramblingListener  onNoValidKey
);


/**
  @brief
  Specify the intent (i.e. use case) of the descrambling session.

  @pre
  A descrambling session must have been successfully opened.

  @post
  The intent is stored in the provided 
  descrambling session and cannot be updated anymore.

  This function allows to define the use case of the descrambling session.@n
  It shall be used only in case of usage of the Nagra DVL for recording or time shift use cases.@n
  If this function is not called, the user intent is ::NV_USER_INTENT_WATCH : the @Ma is used to
  display the decrypted content on a TV screen.@n
  If the application wants to use the @Ma with DVL for a recording use case, then it shall call this function with ::NV_USER_INTENT_RECORD or 
  ::NV_USER_INTENT_RECORD_TIMESHIFT just after the descrambling session creation.@n
  If the application wants to transcode the content after descrambling, then it shall call this function with ::NV_USER_INTENT_EXPORT_XCODE.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    xUserIntent
  Intent describing the use case of the descrambling session.

  @retval ::NV_DSM_SUCCESS
  @a xContext has been successfully stored in the descrambling session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession is not valid.
  + @a xUserIntent is not valid (out of known range)

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing.

  @see nvDsmOpen().
*/

NV_PUBLIC_API uint32_t nvDsmSetUserIntent
(
  TNvSession    xDescramblingSession,
  TNvUserIntent xUserIntent
);

/**
  @brief
  Retrieve the information about content key(s).

  @pre
  The provided descrambling session and the provided key identifier 
  must be valid.

  @post
  None.

  This function fetches the status and the public usage rules associated to a 
  content key. The content key is related to the descrambling session referred 
  by a valid @a xDescramblingSession. 

  The key identifier allows identifying the related content key(s) within 
  the descrambling session -- refer to @ref p_dsm_procref page for further 
  details on key identifier.

  Output parameters for holding key status and usage rules can be @c NULL. In 
  that case each @c NULL output parameter is ignored.

  If the @a pxUsageRules parameter is not @c NULL, its @c data field address 
  can be valid or @c NULL:
  + If @c NULL, no data is copied but the @c size field is set with the 
    expected size for holding the whole usage rules block. The operation is 
    considered successful.
  + If valid, the @c size field must be set with the size of the valid memory 
    block. The usages rules are copied into the memory block if its size can 
    hold the exported usage rules content. The output memory block size is 
    adjusted to the actual number of bytes copied. If the memory block size is 
    too short, the operation failed with ::NV_DSM_ERROR_BUFFER_TOO_SHORT. In 
    that case, no data is copied but the memory block size is set with the 
    expected size required to hold the usage rules content.

  Note that if the @Ma does not find a related content key, no usage rules is 
  exported and the @c size field of @a pxUsageRules is set to @c 0. 
  
  @note
  The exported usage rules data is a null-terminated string describing a 
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The usage rules data format is described in the @ref p_dsm_ur page.

  @param[in]    xDescramblingSession
  Valid descrambling session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the content key(s) to be used.

  @param[out]   pxKeyStatus
  Status of the content key(s) identified by the key identifier.
  This parameter may be @c NULL. Refer to ::TNvKeyStatus for key status values.

  @param[out]   pxUsageRules
  Usage rules associated to the content key(s) identified by the processing 
  reference. This parameter may be @c NULL.

  @retval ::NV_DSM_SUCCESS
  Information have been set according to expected parameters.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.

  @retval ::NV_DSM_ERROR_BUFFER_TOO_SHORT
  The usage rules output parameter is too short.

  @retval ::NV_DSM_ERROR_NO_KEY
  The key identifier provided with @a pxKeyIdentifier does not match
  the descrambling session cached key identifier(s).
  Key status and usages rules cannot be set.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvDsmOpen(), @ref p_dsm_procref, @ref p_dsm_ur.
*/

NV_PUBLIC_API uint32_t nvDsmGetKeyInformation
(
  TNvSession       xDescramblingSession,
  TNvIdentifier*  pxKeyIdentifier,
  uint32_t*       pxKeyStatus,
  TNvBuffer*      pxUsageRules
);

/**
  @brief
  Return the related application session identifier.

  @pre
  The provided descrambling session must be valid.

  @post
  None.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @return
  The function returns the application session identifier value associated to 
  the descrambling session. It returns ::NV_SESSION_INVALID if the 
  @a xDescramblingSession parameter does not refer a valid descrambling session.

  @see nvDsmOpen().
*/

NV_PUBLIC_API TNvSession nvDsmGetApplicationSession
(
  TNvSession  xDescramblingSession
);

/**
  @brief
  Return the transport session identifier.

  @pre
  The provided descrambling session must be valid.

  @post
  None.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @return
  The function returns transport session identifier value associated to the 
  descrambling session. ::NV_DSM_TRANSPORT_SESSION_IDENTIFIER_INVALID is 
  returned if the @a xDescramblingSession parameter does not refer a valid 
  descrambling session.

  @see nvDsmOpen(), @ref p_dsm_tsid.
*/

NV_PUBLIC_API uint32_t nvDsmGetTransportSessionIdentifier
(
  TNvSession  xDescramblingSession
);

/**
  @brief
  Return the encryption method indicator.

  @pre
  The provided descrambling session must be valid.

  @post
  None.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @return
  The function returns the Encryption Method Indicator (a.k.a. EMI) value associated 
  to the descrambling session. See the @ref p_dsm_emi_handling page for additional 
  information on EMI handling.
  @return
  ::NV_DSM_EMI_INVALID is returned either: 
  - if the @a xDescramblingSession parameter does not refer a valid descrambling 
    session;
  - if nvDsmOpen() was called with ::NV_DSM_EMI_INVALID and no valid entitlement was found 
    yet for this descrambling session.

  @see nvDsmOpen(), nvDsmSetPrmContentMetadata(), @ref p_dsm_procref, @ref p_dsm_emi_handling.
*/

NV_PUBLIC_API uint16_t nvDsmGetEmi
(
  TNvSession  xDescramblingSession
);

/** @} */

/* @addtogroup g_dsm */
/** @} */

/* -------------------------------------------------------------------------- */
/* Descrambling Processing - Common Management                                */
/* -------------------------------------------------------------------------- */

/**
  @addtogroup g_dsm
  @{
*/

/**
  @name Content processing management
  Functions included in this group allow processing content stream metadata.
  @see @ref p_dsm_decrypt_ott, @ref p_dsm_decrypt_dvb.
  @{
*/

/**
  @brief
  Type of stream.

  @details
  This enumeration provides all the supported values defining stream types.
*/

typedef enum
{
  NV_STREAM_TYPE_DVB          = 0,
  /**< DVB type value. */
  NV_STREAM_TYPE_CUSTOM       = 1,
  /**< Custom stream type value. */
  NV_STREAM_TYPE_HLS          = 2,
  /**< Value for <em>HTTP Live Streaming (HLS)</em> stream type. */
  NV_STREAM_TYPE_DASH         = 3,
  /**< Value for <em>Dynamic Adaptive Streaming over HTTP (DASH)</em> stream type. */
  NV_STREAM_TYPE_SMOOTH       = 4,
  /**< Value for SMOOTH stream type. */
  NV_STREAM_TYPE_KEY_CONTEXT  = 5
  /**< Identifies a generic key context for stream technologies that supports natively key identifiers.
       The @a data field of the provided @a TNvBuffer metadata buffer points to a ::TNvKeyContext structure.
  */
}
TNvStreamType;

/**
  @brief
  Generic key context.

  @details
  This structure provides a generic key context metadata.
  The key identifier is used for selecting the appropriate entitlement for content decryption.
  The PRM syntax is used for building the challenge message for fetching related entitlements.
*/

typedef struct
{
  TNvIdentifier keyIdentifier;
  /**< Identifier of the content key that must be selected.
       This identifier is a binary identifier 
       e.g. key identifier mapped as UUID must be provided in binary form.
  */
  TNvBuffer*    prmSyntax;
  /**< PRM syntax associated to the content stream (Base64 string).
       If @c NULL, this metadata structure cannot be used to identify the related 
       entitlement to be fetched on key issue - refer to nvLdsUsePrmContentMetadata().
  */
}
TNvKeyContext;

/**
  @brief
  Retrieve the @PROD system identifier for a specific stream signaling standard.

  @pre
  The provided application session identifier must refer a valid @Ma session.

  @post
  None.

  This function provides the @PROD system identifier associated to the stream 
  type defined by @a xStreamType. It allows to identify the @PROD content 
  metadata within the stream signaling. @PROD content metadata must be provided 
  to descrambling sessions using nvDsmSetPrmContentMetadata().

  @warning
  ::NV_STREAM_TYPE_KEY_CONTEXT is not relevant for this function and
  generates ::NV_DSM_ERROR_BAD_PARAMETER error.
  This identifier is a metadata type for stream types that support key 
  identifiers natively but does not identify the stream type by itself.

  @PROD system identifier is provided back in the memory area described by 
  @a pxPrmSystemIdentifier. This parameter must not be @c NULL but its @c data 
  field address can be valid or @c NULL:
  + If @c NULL, no data is copied but the @c size field is set with the 
    expected size for holding the requested @PROD system identifier.
    The operation is considered successful.
  + If valid, the @c size field must be set with the size of the valid memory block.
    The requested @PROD system identifier is copied into the memory block if 
    its size can hold this identifier. The output memory block size is 
    adjusted to the actual number of bytes copied.
    If the memory block size is too short, the operation failed with 
    ::NV_DSM_ERROR_BUFFER_TOO_SHORT.
    In that case, no data is copied but the memory block size is set with the 
    expected size required to hold the requested @PROD system identifier.

  Refer to @ref p_dsm_decrypt_ott @ref p_dsm_decrypt_dvb for further details 
  using the @PROD system identifier, especially its format and interpretation.

  @note
  @PROD system identifier usually depends on configured operator.

  @param[in]    xApplicationSession
  Application session identifier for which must be provided back the related 
  @PROD system identifier. This identifier must be valid.

  @param[in]    xStreamType
  Type of stream, refer to ::TNvStreamType description.

  @param[out]   pxPrmSystemIdentifier
  Address of a memory area to be filled in with the @PROD system identifier. 

  @retval ::NV_DSM_SUCCESS
  @a pxPrmSystemIdentifier has been successfully filled in with the related 
  @PROD system identifier associated to this stream type.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_DSM_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid 
  application session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is ::NV_SESSION_INVALID.
  + @a xStreamType value is not known or is ::NV_STREAM_TYPE_KEY_CONTEXT.
  + @a pxPrmSystemIdentifier is @c NULL.

  @retval ::NV_DSM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxPrmSystemIdentifier is too short.

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing.

  @see nvDsmSetPrmContentMetadata(), TNvStreamType,
       @ref p_asm_appid, @ref p_dsm_decrypt_ott, @ref p_dsm_decrypt_dvb.
*/

NV_PUBLIC_API uint32_t nvDsmGetPrmSystemIdentifier
(
  TNvSession       xApplicationSession,
  TNvStreamType    xStreamType,
  TNvIdentifier*  pxPrmSystemIdentifier
);

/**
  @brief
  Process new @PROD content metadata for setting the related key.

  @pre
  The provided descrambling session must be valid.

  @post
  A key identifier has been generated and associated to the provided 
  @PROD content metadata.

  This function adds @PROD content metadata provided in @a pxMetadata for 
  processing to the descrambling session associated with @a xDescramblingSession.

  The nature of the provided @PROD content metadata depends on the stream type: 
  - A DVB ECM section must be provided for DVB streams i.e. @a xStreamType is 
    ::NV_STREAM_TYPE_DVB. 
    They are filtered out from the right ECM stream selected within the DVB PMT 
    of the program using the DVB @PROD CAS ID provided with 
    nvDsmGetPrmSystemIdentifier().
  - A @PROD specific signaling -- refer to @ref p_nva_refdoc "[SIG]" --
    is generally provided for OTT streams i.e. @a xStreamType is 
    ::NV_STREAM_TYPE_CUSTOM, ::NV_STREAM_TYPE_HLS, ::NV_STREAM_TYPE_DASH or 
    ::NV_STREAM_TYPE_SMOOTH.
    It must be extracted from OTT stream signaling using the related 
    @PROD system identifier.
    The latter has been previously provided using nvDsmGetPrmSystemIdentifier().
  - Alternatively a generic key context metadata can be provided for OTT streams
    supporting natively key identifiers e.g. DASH streams.
    In that case xStreamType must be set to ::NV_STREAM_TYPE_KEY_CONTEXT.
    The @a data field of @a pxMetadata must point to a ::TNvKeyContext structure.
    The @a size field is the size of the ::TNvKeyContext structure.
    + The @a keyIdentifier field must always be a valid data 
      providing a key identifier in its binary form.
    + The @a prmSyntax field must be the @PROD signaling -- refer to 
      @ref p_nva_refdoc "[SIG]" -- identified as stated in previous 
      bullet.
      The @a prmSyntax field can be @c NULL.
      In that case the metadata cannot be used for identifying the specific 
      content for which one or several entitlement(s) must be requested on key issue.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    pxMetadata
  Address of a valid memory block containing the @PROD content metadata e.g. 
  OTT @PROD signaling, DVB ECM section ...

  @param[in]    xStreamType
  Type of the stream which values are defined by ::TNvStreamType.

  @retval ::NV_DSM_SUCCESS
  The @PROD content metadata has been correctly processed.
  Related key identifier has been generated.

  @retval ::NV_DSM_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDescramblingSession 
  creation does no more refer a valid application session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.
  + @a pxMetadata is @c NULL.
  + @a pxMetadata is not @c NULL but its @a data field is @c NULL or its @a size field is @c 0.
  + @a pxMetadata structure is correct but its content is invalid.

  @retval ::NV_DSM_ERROR_UNSUPPORTED_EMI
  The encryption method associated to the content key(s) does not match the one 
  associated to the protected content.

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing e.g. memory allocation issue ...

  @see nvDsmGetPrmSystemIdentifier(), nvDsmGetKeyIdentifier(), 
       @ref p_dsm_procref, @ref p_dsm_decrypt_ott, @ref p_dsm_decrypt_dvb,
       @ref p_dsm_multiscrambling, @ref p_dsm_emi_handling.
*/

NV_PUBLIC_API uint32_t nvDsmSetPrmContentMetadata
(
  TNvSession       xDescramblingSession,
  TNvBuffer*      pxMetadata,
  TNvStreamType    xStreamType
);

/**
  @brief
  Gives information about access criteria and usage rules changes.

  @pre
  The provided application session must be valid.

  This function compares @PROD content metadata provided in @a pxPreviousMetadata 
  with @PROD content metadata provided in @a pxCurrentMetadata.

  The nature of the provided @PROD content metadata depends on the stream type: 
  - A DVB ECM section must be provided for DVB streams i.e. @a xStreamType is 
    ::NV_STREAM_TYPE_DVB. 
    They are filtered out from the right ECM stream selected within the DVB PMT 
    of the program using the DVB @PROD CAS ID provided with 
    nvDsmGetPrmSystemIdentifier().
  - Other stream types are not supported yet.

If the @a pxNewUsageRules parameter is not @c NULL, its @c data field address 
  can be valid or @c NULL:
  + If @c NULL, no data is copied but the @c size field is set with the 
    expected size for holding the whole usage rules block. The operation is 
    considered successful.
  + If valid, the @c size field must be set with the size of the valid memory 
    block. The usages rules are copied into the memory block if its size can 
    hold the exported usage rules content. The output memory block size is 
    adjusted to the actual number of bytes copied. If the memory block size is 
    too short, the operation failed with ::NV_DSM_ERROR_BUFFER_TOO_SHORT. In 
    that case, no data is copied but the memory block size is set with the 
    expected size required to hold the usage rules content.

@note
  The usage rules data is a null-terminated string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The usage rules data format is described in the @ref p_dsm_ur page.

  @param[in]    xApplicationSession
  Application session identifier. This identifier can be ::NV_SESSION_INVALID for ::NV_STREAM_TYPE_DVB, else it must be valid.

  @param[in]    pxPreviousMetadata
  Address of a valid memory block containing the previous @PROD content metadata e.g. 
  DVB ECM section.
  Can also be NULL if no previous metadata.

  @param[in]    pxCurrentMetadata
  Address of a valid memory block containing the current @PROD content metadata e.g. 
  DVB ECM section.

  @param[in]    xStreamType
  Type of the stream which values are defined by ::TNvStreamType.

  @param[out]    pxAccessCriteriaChanged
  TRUE if access criteria has changed. It occurs if the list of licenses allowing to process the content has changed.
  else, value is FALSE.

  @param[out]    pxNewUsageRules
  Contains the new Usage rules. pxUsageRules->size=0 if usage rules did not change. 
  This parameter may be NULL if application does not need usage rules information.

  @retval ::NV_DSM_SUCCESS
  The @PROD content metadata have been correctly analyzed.
  Related information has been generated.

  @retval ::NV_DSM_ERROR_NO_OPERATOR
  The provided application session identifier does no more refer a valid 
  application session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is ::NV_SESSION_INVALID and @a xStreamType is not ::NV_STREAM_TYPE_DVB
  + @a pxCurrentMetadata is @c NULL.
  + @a pxCurrentMetadata (or pxPreviousMetadata) is not @c NULL but its @a data field is @c NULL or its @a size field is @c 0.
  + @a pxCurrentMetadata (or pxPreviousMetadata) structure is correct but its content is invalid.
  + @a xStreamType value is invalid or not supported.
  + @a pxAccessCriteriaChanged is NULL

  @retval ::NV_DSM_ERROR_BUFFER_TOO_SHORT 
  The memory block described by pxNewUsageRules is too short. 

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing e.g. memory allocation issue ...

  @see nvAsmOpen(), @ref s_asm_control_ur.
*/

NV_PUBLIC_API uint32_t nvDsmGetMetadataChangesInformation
( 
  TNvSession                  xApplicationSession, 
  TNvBuffer*                 pxPreviousMetadata, 
  TNvBuffer*                 pxCurrentMetadata, 
  TNvStreamType               xStreamType,
  bool_t*                    pxAccessCriteriaChanged,
  TNvBuffer*                 pxNewUsageRules
);

/**
  @brief
  Gives information about Metadata.

  This function parses the @PROD content metadata provided in @a pxMetadata. 

  The nature of the provided @PROD content metadata depends on the stream type: 
  - In the case where @a xStreamType is ::NV_STREAM_TYPE_CUSTOM, only supported metadata
    is PRM syntax generated by the Nagra DVL for Home Domain purpose.
  - Other stream types are not supported yet.

  The @a pxMetadataInformation parameter must not be @c NULL. Its @c data field address 
  can be valid or @c NULL:
  + If @c NULL, no data is copied but the @c size field is set with the 
    expected size for holding the whole metadata information block. The operation is 
    considered successful.
  + If valid, the @c size field must be set with the size of the valid memory 
    block. The information are copied into the memory block if its size can 
    hold the exported usage rules content. The output memory block size is 
    adjusted to the actual number of bytes copied. If the memory block size is 
    too short, the operation fails with ::NV_DSM_ERROR_BUFFER_TOO_SHORT. In 
    that case, no data is copied but the memory block size is set with the 
    expected size required to hold the metadata information.

  @note
  The metadata information is a null-terminated string describing a
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The metadata information data format is described in the @ref p_dsm_metadata_information page.

  @param[in]    xApplicationSession
  Application session identifier (can be ::NV_SESSION_INVALID)

  @param[in]    pxMetadata
  Address of a valid memory block containing the current @PROD content metadata e.g. 
  PRM syntax generated by the Nagra DVL.

  @param[in]    xStreamType
  Type of the stream which values are defined by ::TNvStreamType.
  Only ::NV_STREAM_TYPE_CUSTOM is currently supported

  @param[out]  pxMetadataInformation
  Contains the metadata information. pxMetadataInformation->size=0 if an error occurs. 

  @retval ::NV_DSM_SUCCESS
  The @PROD content metadata have been correctly analyzed.
  Related information has been generated.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a pxMetadata is not a PRM syntax for Home Domain purpose
  + @a pxMetadata is @c NULL.
  + @a pxMetadata is not @c NULL but its @a data field is @c NULL or its @a size field is @c 0.
  + @a xStreamType value is invalid or not supported.
  + @a pxMetadataInformation is @c NULL

  @retval ::NV_DSM_ERROR_BUFFER_TOO_SHORT 
  The memory block described by pxMetadataInformation is too short. 

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing e.g. memory allocation issue ...

  @see nvDsmSetPrmContentMetadata(), @ref p_dsm_metadata_information
*/

NV_PUBLIC_API uint32_t nvDsmGetMetadataInformation
( 
  TNvSession                  xApplicationSession, 
  TNvBuffer*                 pxMetadata, 
  TNvStreamType               xStreamType,
  TNvBuffer*                 pxMetadataInformation
);

/**
  @brief
  Retrieve a key identifier to associate with content stream data.
  
  @details
  This function is the old name of nvDsmGetKeyIdentifier. Key Identifier is to be replaced by key identifier in the description of nvDsmGetKeyIdentifier
  key identifier corresponds to KID in DASH standard (16 bytes)
  @copydetails nvDsmGetKeyIdentifier
*/
#define nvDsmGetProcessingReference nvDsmGetKeyIdentifier

/**
  @brief
  Retrieve a key identifier to associate with content stream data.

  @pre
  The provided descrambling session must be valid.

  @post
  None.

  Content keys are internally cached and associated to key identifier. 
  This function allows retrieving the last cached key identifier 
  generated within the descrambling session provided in 
  @a xDescramblingSession. 

  The key identifier is returned in the memory block provided with 
  @a pxKeyIdentifier:
  - If @a pxKeyIdentifier describes a valid memory block that can hold 
    the key identifier, the latter is copied in the memory block and the 
    @a size is set to the size of data copied.
  - If the @a data field of the identifier is @c NULL, the function returns 
    successfully but does not copy anything and it sets the @a size field to 
    the required size for holding the key identifier.
  - If @a pxKeyIdentifier describes a valid memory block but that is too 
    short to hold the key identifier, the function fails resulting with 
    ::NV_DSM_ERROR_BUFFER_TOO_SHORT but the @a size field has been set with the 
    required size for holding the key identifier.

  In order to optimize calls, the following indications are provided but may 
  evolve without warning in the future:
  - key identifier associated to DVB streams are about 16 bytes.
  - key identifier associated to DASH streams are about the same size 
    than a KID used on that stream signaling.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[out]   pxKeyIdentifier
  Memory block to be filled in with a key identifier. 

  @retval ::NV_DSM_SUCCESS
  @a pxKeyIdentifier has been successfully filled in with the related 
  key identifier.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_DSM_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDescramblingSession 
  creation does not refer a valid application session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL.
  + @a pxKeyIdentifier is not @c NULL but its @a data field is 
    not @c NULL and its @a size field is @c 0.

  @retval ::NV_DSM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxKeyIdentifier is too short.

  @retval ::NV_DSM_ERROR_NO_KEY
  There is no key identifier available as no @PROD content metadata has 
  ever been set on the provided descrambling session.

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing.

  @see nvDsmSetPrmContentMetadata(), @ref p_dsm_procref,
       @ref p_dsm_decrypt_ott, @ref p_dsm_decrypt_dvb.
*/
NV_PUBLIC_API uint32_t nvDsmGetKeyIdentifier
(
  TNvSession       xDescramblingSession,
  TNvIdentifier*  pxKeyIdentifier
);

/**
  @brief
  Retrieve the key slot identifier associated to a key identifier.

  @pre
  - The provided descrambling session must be valid.
  - For an OTT stream, the key identifier must refer a valid content key.
  - For an OTT stream, there is a valid entitlement available in @Xa for the
    provided key identifier.
  - The key identifier can be @c NULL for a DVB stream.

  @post
  The returned key slot identifier is associated to the provided key identifier.

  Key slots are locations where unwrapped content keys are stored.
  This function allows retrieving the key slot identifier related to the  
  provided key identifier referred by @a pxKeyIdentifier within 
  the provided @a xDescramblingSession. 

  If @a pxKeyIdentifier is @c NULL, the related key slot is assumed to 
  be global and never changes (DVB case). Please refer to the example in 
  the @ref p_dsm_emi_handling page for a better understanding of the 
  availability of the key slot identifier within a descrambling session.

  @attention
  If the Player monitors the @em NewKey event (see nvDsmSetNewKeyListener()), then
  calling nvDsmGetKeySlotIdentifier() on notifications of that event ensures that the
  Player always get the correct identifier of the keyslot into which the new key has
  just been set.
  The monitoring frequency depends on the descrambling session type:
  - For an OTT descrambling sessions: the Player shall monitor @em each notification 
    of the @em NewKey event because, due to key rotation, the key slot may changes each
    time a new content metadata is successfully processed by the descrambling session.
  - For a DVB descrambling session open with EMI value ::NV_DSM_EMI_INVALID: as the 
    key slot allocated for a DVB stream never changes during the session lifespan, 
    the Player can monitor this event only once after the first successfully 
    processed content metadata to get the key slot.
  - For a DVB descrambling session open with a known EMI value: although possible,
    the monitoring of the @em NewKey event to obtain the key slot identifier is not
    necessary because the key slot can be requested by the Player immediately after the
    descrambling session is successfully open.
  .

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]   pxKeyIdentifier
  Address of a key identifier.
  Can be @c NULL.

  @param[out]  pxKeySlotIdentifier
  Address to be filled with the key slot identifier. 

  @retval ::NV_DSM_SUCCESS
  @a pxKeySlotIdentifier has been successfully filled in with the related 
  key slot identifier.

  @retval ::NV_DSM_ERROR_NO_OPERATOR
  The application session identifier provided for @a xDescramblingSession 
  creation does not refer a valid application session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.
  + @a pxKeyIdentifier is not @c NULL but its @a data field is 
    @c NULL or its @a size field is @c 0.

  @retval ::NV_DSM_ERROR_NO_KEY
  The current descrambling session failed to identify the key to use during 
  content metadata processing or the provided @a pxKeyIdentifier value does 
  not reference a valid key identifier for that descrambling session.

  @retval ::NV_DSM_ERROR
  An unexpected error occurs during processing.

  @see nvDsmOpen(), nvDsmSetPrmContentMetadata(), nvDsmGetKeyIdentifier(), @ref p_dsm_procref, @ref p_dsm_emi_handling.
*/

NV_PUBLIC_API uint32_t nvDsmGetKeySlotIdentifier
(
  TNvSession             xDescramblingSession,
  const TNvIdentifier*  pxKeyIdentifier,
  uint16_t*             pxKeySlotIdentifier
);

/** @} */

/* @addtogroup g_dsm */
/** @} */

/* -------------------------------------------------------------------------- */
/* Stream processing management                                               */
/* -------------------------------------------------------------------------- */

/**
  @addtogroup g_dsm
  @{
*/

/**
  @name Stream processing management
  Functions included in this group allow processing stream content data 
  decryption.

  @see @ref p_dsm_decrypt_ott, @ref p_dsm_decrypt_dvb.
  @{
*/

/**
  @brief
  Process a stream content data block (for open device)
  
  @details
  This function is the old name of nvDsmDecrypt. 
  @copydetails nvDsmDecrypt
*/
#define nvDsmProcess nvDsmDecrypt

/**
  @brief
  Process a stream content data block (for open device only)

  @pre
  The provided descrambling session and key identifier must be valid.

  @post
  The provided content data block has been processed with the content key(s) 
  related to the provided key identifier. The initialization vector, if 
  any and required, has been updated in internal resources.

  This function processes a block of data within the descrambling session 
  referred by a valid @a xDescramblingSession. The data to process is referred 
  by the @a pxInData parameter: it shall not be @c NULL and must contain a 
  valid memory block. The processed data is copied into the memory area 
  referred by the @a pxOutData parameter: it shall not be @c NULL and must 
  contain a valid memory block with enough space. However the memory area 
  referred @a pxOutData can be the same as the memory area referred by 
  @a pxInData. In case of HLS descrambling, the size of the input 
  data must be a full number of ciphered blocks -- Refer to 
  @ref p_dsm_decrypt_ott.

  The key identifier allows identifying the related content key(s) to use 
  for processing the content data. This reference is provided in 
  @a pxKeyIdentifier by the @Ma in response to a @PROD content metadata 
  analysis e.g. during a descrambling request. It shall be associated to each 
  related content data -- refer to the @ref p_dsm_procref page for further 
  details. 

  The initialization vector referred by @a pxInitializationVector should be 
  provided for encryption method requiring initialization vector. In that case 
  an initialization vector is first fetched by player and initially provided 
  to the @Ta. The initialization vector is updated internally by the 
  nvDsmDecrypt() operation. It may be provided @c NULL on the next call to 
  nvDsmDecrypt() as long as no new initialization vector is fetched. When a new 
  one is fetched, it shall be provided. Refer to @ref p_dsm_decrypt_ott for 
  further details.

  The xLastData parameter is also related to chained encryption method. It 
  allows signaling whether the content data is not the last data of the current 
  processed chain -- @a xLastData = ::FALSE -- or not -- @a xLastData = ::TRUE.
  When @a xLastData is set to ::TRUE and in case of HLS descrambling, the size of
  @a pxOutData is computed by the @Ma to remove the PKCS#7 padding.  
  When @a xLastData is set to ::TRUE and the encryption method 
  requires an initialization vector, the next call to the process operation 
  shall be provided with a newly fetched initialization vector.

  Once the related content key(s) has been found, the @Ta checks the key 
  usage rules. If the key cannot be found or if it is not entitled to be used, 
  the operation failed resulting with ::NV_DSM_ERROR_KEY_STATUS. The details of 
  the key status is set in the value referred by @a pxKeyStatus if the latter 
  is not @c NULL. If it is provided @c NULL, it is simply ignored and the key 
  status error is lost. The key status values are described in ::TNvKeyStatus.

  Content keys are internally associated to an encryption method. If ever this 
  encryption method is not supported by the descrambling processing, the 
  operation fails returning ::NV_DSM_ERROR_UNSUPPORTED_EMI.

  The processing of the content data is effective when a valid matching key has 
  been found. Data is processed according to the encryption method associated 
  to the content key(s).

  Final padding introduced with some chained encryption method -- e.g. HLS... 
  -- is processed by the @PROD implementation. Size of the output is decreased according
  to the padding value.

  @param[in]    xDescramblingSession
  Valid descrambling session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the content key(s) to be used.

  @param[in]    pxInitializationVector
  Valid reference to an initialization vector structure.

  @param[in]    xLastData
  Continuity indicator for chained encryption method. 
  This parameter may not be relevant depending on the encryption method.

  @param[in]    pxInData
  Valid input data block to process.

  @param[out]   pxOutData
  Valid output data block for processed data. 
  This structure may refer the input data block so that the processing is done
  in place.

  @param[out]   pxKeyStatus
  Status of the content key(s) identified by the key identifier.
  This parameter may be @c NULL.

  @retval ::NV_DSM_SUCCESS
  The content data block has been correctly processed.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.
  + @a pxInData or @a pxOutData is @c NULL.
  + @a pxInData->data or @a pxOutData->data is @c NULL.

  @retval ::NV_DSM_ERROR_NO_KEY
  There is currently no keys referred by the key identifier provided with
  @a pxKeyIdentifier.

  @retval ::NV_DSM_ERROR_KEY_STATUS
  The key related to the key identifier -- or selected by default -- is 
  not present or cannot be used.
  The provided content data has not been processed: a detailed key status can 
  be found in @a pxKeyStatus if not @c NULL.

  @retval ::NV_DSM_ERROR_UNSUPPORTED_EMI
  The encryption method associated to the content key(s) is not supported.

  @retval ::NV_DSM_ERROR_CRYPTOENGINE
  The underlying stream crypto-engine reports an error.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvDsmOpen(), nvDsmGetEmi(), nvDsmSetNewKeyListener(), 
       nvDsmGetKeyInformation(), @ref p_dsm_procref, @ref p_dsm_tsid, 
       @ref p_dsm_decrypt_ott.
*/

NV_PUBLIC_API uint32_t nvDsmDecrypt
(
  TNvSession       xDescramblingSession,
  TNvIdentifier*  pxKeyIdentifier,
  TNvBuffer*      pxInitializationVector,
  bool_t           xLastData,
  TNvBuffer*      pxInData,
  TNvBuffer*      pxOutData,
  uint32_t*       pxKeyStatus
);

/**
  @brief
  Process a stream content data block (for STB)

  @pre
  The provided descrambling session and key identifier must be valid.

  @post
  The provided content data block has been processed with the content key(s) 
  related to the provided key identifier. The initialization vector, if 
  any and required, has been updated in internal resources.

  This function processes a block of data within the descrambling session 
  referred by a valid @a xDescramblingSession. The data to process is referred 
  by the @a pxInData parameter: it shall not be @c NULL and must contain a 
  valid memory block. The processed data is copied into the memory area 
  referred by the @a pxOutData parameter: it shall not be @c NULL and must 
  contain a valid memory block with enough space. The opaque buffer definition
  is done by the implementer of @ref p_nva_refdoc "[SEC]" 
  as the @Ma uses the processOpaqueData to effectively decrypt data.

  The key identifier allows identifying the related content key(s) to use 
  for processing the content data. This reference is provided in 
  @a pxKeyIdentifier by the @Ma in response to a @PROD content metadata 
  analysis e.g. during a descrambling request. It shall be associated to each 
  related content data -- refer to the @ref p_dsm_procref page for further 
  details. 

  The initialization vector referred by @a pxInitializationVector should be 
  provided for encryption method requiring initialization vector. In that case 
  an initialization vector is fetched by player and provided 
  to the @Ta. 

  The xLastData parameter is also related to chained encryption method. It 
  allows signaling whether the content data is not the last data of the current 
  processed chain -- @a xLastData = ::FALSE -- or not -- @a xLastData = ::TRUE. 
  
  Once the related content key(s) has been found, the @Ta checks the key 
  usage rules. If the key cannot be found or if it is not entitled to be used, 
  the operation failed resulting with ::NV_DSM_ERROR_KEY_STATUS. The details of 
  the key status is set in the value referred by @a pxKeyStatus if the latter 
  is not @c NULL. If it is provided @c NULL, it is simply ignored and the key 
  status error is lost. The key status values are described in ::TNvKeyStatus.

  Content keys are internally associated to an encryption method. If ever this 
  encryption method is not supported by the descrambling processing, the 
  operation fails returning ::NV_DSM_ERROR_UNSUPPORTED_EMI.

  The processing of the content data is effective when a valid matching key has 
  been found. Data is processed according to the encryption method associated 
  to the content key(s).

  @param[in]    xDescramblingSession
  Valid descrambling session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the content key(s) to be used.

  @param[in]    pxInitializationVector
  Valid reference to an initialization vector structure.

  @param[in]    xLastData
  Continuity indicator for chained encryption method. 
  This parameter may not be relevant depending on the encryption method.

  @param[in]    pxInData
  Valid input data block to process.

  @param[out]   pxOutData
  Valid output data block for processed data. 
  This structure may refer the input data block so that the processing is done
  in place.

  @param[out]   pxKeyStatus
  Status of the content key(s) identified by the key identifier.
  This parameter may be @c NULL.

  @retval ::NV_DSM_SUCCESS
  The content data block has been correctly processed.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.
  + @a pxInData or @a pxOutData is @c NULL.
  + @a pxInData->data or @a pxOutData->data is @c NULL.

  @retval ::NV_DSM_ERROR_NO_KEY
  There is currently no keys referred by the key identifier provided with
  @a pxKeyIdentifier.

  @retval ::NV_DSM_ERROR_KEY_STATUS
  The key related to the key identifier -- or selected by default -- is 
  not present or cannot be used.
  The provided content data has not been processed: a detailed key status can 
  be found in @a pxKeyStatus if not @c NULL.

  @retval ::NV_DSM_ERROR_UNSUPPORTED_EMI
  The encryption method associated to the content key(s) is not supported.

  @retval ::NV_DSM_ERROR_CRYPTOENGINE
  The underlying stream crypto-engine reports an error.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvDsmOpen(), nvDsmGetEmi(), nvDsmSetNewKeyListener(), 
       nvDsmGetKeyInformation(), @ref p_dsm_procref, @ref p_dsm_tsid, 
       @ref p_dsm_decrypt_ott.
*/

NV_PUBLIC_API uint32_t nvDsmDecryptOpaque
(
  TNvSession       xDescramblingSession,
  TNvIdentifier*  pxKeyIdentifier,
  TNvBuffer*      pxInitializationVector,
  bool_t           xLastData,
  void*           pxInData,
  void*           pxOutData,
  uint32_t*       pxKeyStatus
);

/** @} */

/* @addtogroup g_dsm */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* defined NV_DSM_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
