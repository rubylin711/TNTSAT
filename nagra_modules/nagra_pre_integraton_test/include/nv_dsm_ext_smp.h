/**
  @file nv_dsm_ext_smp.h

  @brief
  This file defines the secure media path extension to @PROD descrambling session interface.

  @details

  This interface is realized by the @Ma.
  It provides the way to manage the secure media path enforcement on
  @PROD descrambling sessions.

  @warning
  For device without secure media path support, delivered @PROD client is 
  built without the support of this interface extension.
  It means that in this case the final link will fail if one of the below 
  function is called.

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

#ifndef NV_DSM_EXT_SMP_H
#define NV_DSM_EXT_SMP_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Dependencies                                                               */
/* ========================================================================== */

#include "prm_dsm.h"

/**
  @addtogroup g_dsm
  @{
*/

/**
  @name Secure media path management
  Functions included in this group allow managing available secure media path.
  @warning
  These definitions are relevant only for device supporting a secure media path feature.
  Otherwise this feature is even not built in the delivered @PROD client.
  @{
*/

/* ========================================================================== */
/* Definitions                                                                */
/* ========================================================================== */

/**
  @brief
  Values for secure media path enforcement status.

  @warning
  For device without secure media path support, this definition must not be used.

  @details
  This enumeration provides all the possible values for secure media path enforcement status.
  Refer to nvDsmSetOnSmpEnforcementListener() and ::INvDescramblingSmpListener for
  a detailed description about the consequences associated to each of these 
  secure media path status values.
*/

typedef enum
{
  NV_SMP_MUST_BE_ENABLED  = 0,
  /**< The secure media path is required to be enforced. */
  NV_SMP_MUST_BE_DISABLED = 1,
  /**< <DEPRECATED> SMP shall be deactivated.
   This value was used to force the secure media path to be disabled. It is 
   deprecated and shall be replaced or handled as 'NV_SMP_REQUEST_CHOICE ' */
  NV_SMP_REQUEST_CHOICE   = 2
  /**< The secure media path can be enabled or disabled. */
}
TDsmSmpEnforcementStatus;

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */

/**
  @brief
  Secure media path enforcement information.

  @warning
  For device without secure media path support, this definition must not be used.

  @details
  This structure describes information related to the secure media path information.
  The version field is reserved for future backward compatibility.
  This field is currently set to 0.
*/

typedef struct
{
  uint32_t                  version;
  /**< Version of the structure format. */
  TDsmSmpEnforcementStatus  status;
  /**< Enforcement status related to the content key. */
  TNvIdentifier             keyIdentifier;
  /**< Identifier of the related content key. */
  uint16_t                  keySlotIdentifier;
  /**< Identifier of the key slot where is located the content key. */
}
TDsmSmpEnforcementInformation;

/**
  @brief
  Secure media path enforcement listener for a descrambling session.

  @warning
  For device without secure media path support, this definition must not be used.

  @pre
  A secure media path enforcement event occurs on a descrambling session.

  @post
  The notified event is considered processed and is not maintained.

  This type defines a notification function callback for secure media path 
  enforcement events.

  @param[in]    xDescramblingSession
  Identifies the descrambling session related to the event.

  @param[in]   pxSmpEnforcementInformation
  Provides secure media path enforcement information related to the event.

  @return
  The function shall return @c ::TRUE if the event recipient intends to enable 
  secure media path management or @c ::FALSE if it intends to disable it.

  @details

  This function is called for each content key processed for the first time.
  It either notifies of the enforcement status so that the event recipient 
  manages the media stream accordingly or when the secure media path enforcement 
  is left up to the event recipient, it requests the event recipient’s decision 
  regarding this enforcement.

  Consequently, the event recipient must return the correct value depending on
  the secure media path enforcement status and further adapts the media chunks
  management in order to fulfill the secure media path constraints related to
  the hardware platform:

  + When set to ::NV_SMP_MUST_BE_ENABLED, the event recipient is notified that 
    the secure media path must be enabled.
    Therefore it MUST return @c ::TRUE and manage the media stream with 
    secure media path enabled.
    @PROD client enforces the content key with secure media path enabled.
    If the callback implementation returns @c ::FALSE in this case, 
    @PROD client does not provide the content key and descrambling will fail.

  + When set to ::NV_SMP_MUST_BE_DISABLED, the event recipient is notified that 
    the secure media path must be disabled.
    Therefore it MUST return @c ::FALSE and manage the media stream with 
    secure media path disabled.
    @PROD client enforces the content key with secure media path disabled.
    If the callback implementation returns @c ::TRUE in this case, 
    @PROD client does not provide the content key and descrambling will fail.

  + When set to ::NV_SMP_REQUEST_CHOICE, the event recipient is notified that 
    the secure media path can be enabled or disabled depending on its choice.
    Therefore it MUST return @c ::TRUE to enable it or @c ::FALSE to disable it.
    It must manage the media stream accordingly to its decision.
    @PROD client aligns content key enforcement with the event recipient decision.
  
  @warning
  The @PROD client manages a blocking call to this callback.
  Related media stream descrambling process is suspended until this callback function 
  returns with the recipient decision.
  Therefore attention should be paid to limit the amount of processing done within
  this callback function call: It should be restricted to only what is needed to provide 
  the secure media path enforcement decision and memorizes the related needed data.

  @note
  When no listener callback has been registered, @PROD client assumes a
  default @c ::FALSE "return" from event recipient.
  It means that:
  + If content key usage rules require ::NV_SMP_MUST_BE_ENABLED, the descrambling will fail.
  + If content key usage rules require ::NV_SMP_MUST_BE_DISABLED, the descrambling will succeed
    and secure media path will be disabled.
  + If content key usage rules require ::NV_SMP_REQUEST_CHOICE, the descrambling will succeed
    and secure media path will be disabled.
  
  The listener callback function registration can be canceled within the callback function call
  in order to no more be warned of secure media path enforcement events.
  In that case remind that default behavior described above will apply the next time a new 
  content key entitlement is processed.
  
  @see nvDsmSetOnSmpEnforcementListener().
*/

typedef bool_t (*INvDescramblingSmpListener)
(
  TNvSession                       xDescramblingSession,
  TDsmSmpEnforcementInformation*  pxSmpEnforcementInformation
);

/* ========================================================================== */
/* Functions                                                                  */
/* ========================================================================== */

/**
  @brief
  Define a listener callback function for secure media path enforcement event.

  @warning
  For device without secure media path support, this function must not be called
  as it is not built in the @PROD client delivery.

  @pre
  The provided descrambling session must be valid.

  @post
  The listener function is registered for the descrambling session.

  @param[in]    xDescramblingSession
  Descrambling session identifier.

  @param[in]    onSmpEnforcement
  Listener callback function to register.

  @retval ::NV_DSM_SUCCESS
  The listener callback function has been registered in the descrambling session.

  @retval ::NV_DSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xDescramblingSession does not refer a valid session.

  @retval ::NV_DSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @details

  After being registered, the listener callback function is triggered each time 
  the @Ta caches a new or an updated key in the descrambling session.
  Secure media path enforcement status must be checked
  so that the player can manage the media stream the right way e.g. allocating 
  media chunks in specific memory area or not...

  The listener registration can be canceled by calling this function again 
  with a @c NULL listener parameter. If no listener is currently registered, a 
  call with a @c NULL listener value does simply nothing.

  @note
  For device with secure media path support and if no listener callback has 
  been registered, @PROD client assumes as if a default @c ::FALSE has been 
  returned for event recipient.
  It means that:
  + If content key usage rules require ::NV_SMP_MUST_BE_ENABLED, the descrambling will fail.
  + If content key usage rules require ::NV_SMP_MUST_BE_DISABLED, the descrambling will succeed
    and secure media path will be disabled.
  + If content key usage rules require ::NV_SMP_REQUEST_CHOICE, the descrambling will succeed
    and secure media path will be disabled.

  When the listener registration is canceled, the default behavior described above will apply 
  the next time @PROD client processes a new content key entitlement or updates the content key entitlement.
  The current processing of content key -- at the time of registration canceling --
  is unaffected unless an new entitlement updates the content key rules.

  @warning
  The @PROD client manages a blocking call to the provided callback.
  Related media stream descrambling process is suspended until the callback function 
  returns with the recipient decision.
  Therefore attention should be paid to limit the amount of processing done within
  the callback function call: It should be restricted to only what is needed to provide 
  the secure media path enforcement decision and memorizes the related needed data.

  @see nvDsmOpen(), nvDsmGetKeyInformation(), nvDsmGetKeySlotIdentifier().
*/

NV_PUBLIC_API uint32_t nvDsmSetOnSmpEnforcementListener
(
  TNvSession                  xDescramblingSession,
  INvDescramblingSmpListener  onSmpEnforcement
);

/** @} */

/* addtrogroup g_dsm */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* defined NV_DSM_EXT_SMP_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
