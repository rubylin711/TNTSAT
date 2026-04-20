/**
  @file nv_dpsc.h

  @brief
  This file defines the @DPSC API

  This interface is realized by the @Client.
  It provides a @Client provisioning session service including challenge 
  exportation and license importation for acquiring provisioned data.

  COPYRIGHT:
    2014 - 2015 Nagravision S.A.
*/

/*
  ==========================================================================
  IMPORTANT REMARK :
  ==========================================================================

  Comments in this file use special tags to allow automatic API
  documentation generation in HTML format, using the GNU-General Public
  licensed Doxygen tool.
  For more information about Doxygen, please check www.doxygen.org

  Depending on the platform, the CHM file may not open properly if it is
  stored on a network drive. So either the file should be moved on a local
  drive or add the following registry entry on Windows platform (regedit):
  [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\HTMLHelp\1.x\\ItssRestrictions] "MaxAllowedZone"=dword:00000003

  ==========================================================================
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_dpsc

  @brief
  Describe the @DPSC interface.

  @details
  The <b>@DPSC</b> interface allows using provisioning session for retrieving 
  provisioned data from a @PROD provisioning server.
  Please make sure to have read the documentation pages for a complete 
  description of the interface constraints and requirements.
*/

#ifndef NV_DPSC_H
#define NV_DPSC_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_defs.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @addtogroup g_dpsc_version
  @{
*/

/** @brief @c Dpsc interface generic version major number. */
#define DPSCAPI_VERSION_MAJOR        1


/** @brief @c Dpsc interface generic version medium number. */
#define DPSCAPI_VERSION_MEDIUM       0

/** @brief @c Dpsc interface generic version minor number. */
#define DPSCAPI_VERSION_MINOR        6

/**
  @brief @c Dpsc interface version formatted as a single integer.
  @hideinitializer
*/
#define DPSCAPI_VERSION_INT      \
  NV_INTERFACE_VERSION_INT(      \
    DPSCAPI_VERSION_MAJOR,       \
    DPSCAPI_VERSION_MEDIUM,      \
    DPSCAPI_VERSION_MINOR        \
  )

/**
  @brief @c Dpsc interface version formatted as a string.
  @hideinitializer
*/
#define DPSCAPI_VERSION_STRING  \
  NV_INTERFACE_VERSION_STRING(  \
    DPSCAPI_,                   \
    DPSCAPI_VERSION_MAJOR,      \
    DPSCAPI_VERSION_MEDIUM,     \
    DPSCAPI_VERSION_MINOR       \
  )

/** @} */

/* ========================================================================== */

/**
  @addtogroup g_dpsc
  @{
*/

/**
  @brief
  Values for provisioning session operation status.

  This enumeration provides all the possible values that can be returned by a
  provisioning session operation.
*/
typedef enum
{
  NV_DPSC_SUCCESS                  = 0x00040000L,
  /**< The provisioning session operation has been successful. */
  NV_DPSC_ERROR_BAD_PARAMS         = 0x00040001L,
  /**< The provisioning session operation failed: One or more provided parameters are invalid. */
  NV_DPSC_ERROR_BUFFER_TOO_SMALL   = 0x00040002L,
  /**< The provisioning session operation failed: A provided memory block is too short to be filled in with the expected result. */
  NV_DPSC_ERROR_NO_RESULTS         = 0x00040003L,
  /**< The provisioning session operation failed: No operation providing results has been processed. */
  NV_DPSC_ERROR_BAD_METADATA       = 0x00040004L,
  /**< The provisioning session operation failed: Some metadata are invalid. */
  NV_DPSC_ERROR_MEM                = 0x00040005L,
  /**< The provisioning session operation failed: A memory allocation failed. */
  NV_DPSC_ERROR_INVALID_MESSAGE    = 0x00040006L,
  /**< The operation failed: imported message had an invalid format. */
  NV_DPSC_ERROR_INVALID_OBJECT     = 0x00040007L,
  /**< The operation failed: message includes at least one invalid object. */
  NV_DPSC_ERROR_RESOURCE           = 0x00040008L,
  /**< The operation failed: message importation has encountered errors with resource e.g. storage resource, memory resource. */
  NV_DPSC_ERROR_UNKNOWN            = 0x0004FFFFL
  /**< The provisioning session operation failed: An unexpected error occurred. */
} TNvDpscStatus;

/** @} */

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @addtogroup g_dpsc
  @{
*/

/**
  @brief
  Create a provisioning session.

  @pre
  The @Client must have been successfully initialized.

  @post
  An provisioning session has been created.

  @param[out]    pxProvSession
  The provisioning session that can be used for creating payloads data.
  @a pxProvSession cannot be @c NULL.

  @retval ::NV_DPSC_SUCCESS
  Operation was successful and provisioning request is ready.

  @retval ::NV_DPSC_ERROR_BAD_PARAMS
  @a pxProvSession is @c NULL.

  @retval ::NV_DPSC_ERROR_UNKNOWN
  An unexpected error has occurred:
  + A mandatory property is not set
  + @PROD Client is not started
  + Problem of Memory resources, drivers, ...

  @retval ::NV_DPSC_ERROR_MEM
  A memory allocation failed and operation was canceled.

  @see nvDpscClose().
*/  
NV_PUBLIC_API TNvDpscStatus nvDpscOpen
(
  TNvSession*  pxProvSession
);

/**
  @brief
  Close a provisioning session.

  @pre
  The provisioning session has been created with nvDpscOpen().

  @post
  The underlying session for the provided handle is disposed of 
  and cannot be used anymore.

  @param[in]    xProvSession
  The provisioning session handle to close.

  @retval ::NV_DPSC_SUCCESS
  Operation was successful and provisioning session has been reclaimed.

  @retval ::NV_DPSC_ERROR_BAD_PARAMS
  The @a xProvSession handle is not a valid provisioning session.

  @retval ::NV_DPSC_ERROR_UNKNOWN
  An unknown error occurs

  @see nvDpscOpen().
*/  
NV_PUBLIC_API TNvDpscStatus nvDpscClose
(
  TNvSession  xProvSession
);
  
/**
  @brief
  Set client data for the provisioning challenge generation.

  @pre
  The @a xProvSession was opened successfully through nvDpscOpen()
  The @a pxClientData is a valid pointer (not @c NULL) and contains a valid 
  memory buffer holding data to be provided (data pointer not @c NULL and 
  size not @c NULL).
  The @a pxClientData is a @c NULL pointer for removing last client data
  provided.

  @post
  Data provided is saved for further challenge generation through nvDpscExportMessage().

  @note
  The provided data should be fetched using nvAsmGetProvisioningParameters(). Any other
  data would result in error.
  
  @param[in]    xProvSession
  The provisioning session handle.

  @param[in]    pxClientData
  A valid buffer pointer fetched using nvAsmGetProvisioningParameters(), or a
  @c NULL pointer for erasing client data previously saved. 

  @retval ::NV_DPSC_SUCCESS
  Operation was successful. Client Data saved for further challenge generation.
  Data provided will be used until further

  @retval ::NV_DPSC_ERROR_BAD_PARAMS
  Some input parameters are invalid:
  + @a xProvSession is not a valid provisioning session
  + @a pxClientData is not fetched from nvAsmGetProvisioningParameters()

  @retval ::NV_DPSC_ERROR_MEM
  A memory allocation failed and operation was canceled.

  @retval ::NV_DPSC_ERROR_UNKNOWN
  Other error occurred that prevented the generation of the server request.

  @see nvDpscExportMessage().
  @see nvAsmGetProvisioningParameters().
*/  

NV_PUBLIC_API TNvDpscStatus nvDpscSetClientData
(
  TNvSession   xProvSession,
  TNvBuffer*  pxClientData
);
  
/**
  @brief
  Get message payload to send to server for provisioning request

  @pre
  The @a xProvSession was opened successfully through nvDpscOpen()
  The @a pxMessage is a valid pointer (not @c NULL).

  @post
  The data provided back in @a pxMessage contains the provisioning challenge.
  The provided message data is a null-terminated string describing a 
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  The message data format is described in <em>Client-PVS IRS v0.0.2</em> document.
  The challenge data is the @a challenge field of this message.

  @details

  This function allows to fetch the provisioning message -- generally including 
  a @PROD challenge -- to present to the provisioning server. The response of 
  the provisioning server shall be imported in the same provisioning session 
  using nvDpscImportMessage().

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
   ::NV_DPSC_ERROR_BUFFER_TOO_SMALL but the @a size field has been set with the 
   required size for holding the @PROD message.

  @param[in]    xProvSession
  The provisioning session handle.

  @param[out]    pxMessage
  - If @a pxMessage describes a valid memory block that can hold the @PROD 
    message, the latter is copied in the memory block and the @a size is set to 
    the size of data copied.
    The message buffer contains JSON with:
      + a key "challenge" containing the payload (JSON) to provide to server
  - If the @a data field of the message is @c NULL, the function returns 
    successfully but does not copy anything and it sets the @a size field to 
    the required size for holding the @PROD message.
  - If @a pxMessage describes a valid memory block but that is too short to 
    hold the @PROD message, the function fails resulting with 
    ::NV_DPSC_ERROR_BUFFER_TOO_SMALL but the @a size field has been set with the 
    required size for holding the @PROD message.

  @retval ::NV_DPSC_SUCCESS
  Operation was successful. Challenge is available.

  @retval ::NV_DPSC_ERROR_BAD_PARAMS
  Some input parameters are invalid:
  + @a xProvSession is not a valid provisioning session
  + @a pxMessage is @c NULL

  @retval ::NV_DPSC_ERROR_BUFFER_TOO_SMALL
  The memory buffer provided for getting message back is too small to contain
  the whole of it. The size value in the TNvBuffer structure was set to 
  required size.

  @retval ::NV_DPSC_ERROR_MEM
  A memory allocation failed and operation was canceled.

  @retval ::NV_DPSC_ERROR_UNKNOWN
  Other error occurred that prevented the generation of the server request.

  @see nvDpscImportMessage().
*/  
NV_PUBLIC_API TNvDpscStatus nvDpscExportMessage
(
  TNvSession   xProvSession,
  TNvBuffer*  pxMessage
);

/**
  @brief
  Return the session properties.

  @pre
  The @a xProvSession was opened successfully through nvDpscOpen()
  The @a pxProps is a valid pointer (not @c NULL).

  @post
  The provided properties data is a null-terminated string describing a 
  <a href="http://json.org/">JSON</a> object.
  The related @a size field does not include the terminating null character.
  This JSON contains a key "url" with the server url as text value.
  This JSON might contains a key "provdata" with the exportable provisioned
  data as base64 encoded value.

  @details

  This function allows the client to retrieve the @PROD provisioning server 
  default url and the provisioned data after a response was imported through 
  nvDpscImportMessage().
  Url is provided in a string value with the key @c url.
  Provisioned data are provided in an array with the key @c provdata. The array 
  contains one element per imported message. Each element is an object with
  service id as key and provisioned data as string value (base64 encoded).

  The provisioning properties are returned in the memory block provided with 
  @a pxProps:
  - If @a pxProps describes a valid memory block that can hold the provisioning
   properties, the latter is copied in the memory block and the @a size is set to 
   the size of data copied.
  - If the @a data field of the properties is @c NULL, the function returns 
   successfully but does not copy anything and it sets the @a size field to 
   the required size for holding the properties buffer.
  - If @a pxProps describes a valid memory block but that is too short to 
   hold the properties, the function fails resulting with 
   ::NV_DPSC_ERROR_BUFFER_TOO_SMALL but the @a size field has been set with the 
   required size for holding the @PROD message.

  @param[in]    xProvSession
  The provisioning session handle.

  @param[out]    pxProps
  - If @a pxProps describes a valid memory block that can hold the provisioning
    properties, the latter is copied in the memory block and the @a size is set to 
    the size of data copied.
    The properties buffer contains JSON with:
      + a key @c url containing the url of the server to provide the message to.
        This url is hardcoded and points to Nagra's PVS instance at Cloud. This url 
        is only informative and could be overridden by provisioning client.
  		+ a key @c provdata containing an array. Each element of this array contain
        a JSON object with service id as key and provisioned data as string value 
        (base64 encoded).
  - If the @a data field of the properties buffer is @c NULL, the function returns 
    successfully but does not copy anything and it sets the @a size field to 
    the required size for holding the provisioning properties.
  - If @a pxProps describes a valid memory block but that is too short to 
    hold the provisioning properties, the function fails resulting with 
    ::NV_DPSC_ERROR_BUFFER_TOO_SMALL but the @a size field has been set with the 
    required size for holding the provisioning properties.

  @retval ::NV_DPSC_SUCCESS
  Operation was successful. Properties are available.

  @retval ::NV_DPSC_ERROR_BAD_PARAMS
  Some input parameters are invalid:
  + @a xProvSession is not a valid provisioning session
  + @a pxProps is @c NULL

  @retval ::NV_DPSC_ERROR_BUFFER_TOO_SMALL
  The memory buffer provided for getting properties back is too small to contain
  the whole of it. The size value in the TNvBuffer structure was set to 
  required size.

  @retval ::NV_DPSC_ERROR_MEM
  A memory allocation failed and operation was canceled.

  @retval ::NV_DPSC_ERROR_UNKNOWN
  Other error occurred that prevented the generation of the server request.

  @see nvDpscExportMessage().
  @see nvDpscImportMessage().
*/
NV_PUBLIC_API TNvDpscStatus nvDpscGetProperties
(
  TNvSession   xProvSession,
  TNvBuffer*  pxProps
);

/**
  @brief
  Import in provisioning session the provisioning payload received from @PROD 
  provisioning server.

  @pre
  @a xProvSession is a valid provisioning session opened through nvDpscOpen().
  @a pxMessage is a valid pointer to an allocated TNvBuffer structure filled
  with the payload value obtained from server.

  @post
  The provided data is imported and referred in the provisioning session.

  @param[in]    xProvSession
  The provisioning session.

  @param[out]    pxMessage
  A @c TNvBuffer containing the payload obtained from the server response,
  triggered by sending the challenge obtained with nvDpscExportMessage().
  The TNvBuffer and associated memory buffer are allocated by the caller. 
  The caller will have to release TNvBuffer structure and data buffer in it
  after the operation succeed and as soon as not needed anymore.

  @retval ::NV_DPSC_SUCCESS
  Operation was successful and message was taken into account. The actual 
  importation process may be successful or may result with errors. 

  @retval ::NV_DPSC_ERROR_BAD_PARAMS
  Some input parameters are invalid: 
  + @a xProvSession is not a valid provisioning session
  + @a pxMessage is @c NULL.

  @retval ::NV_DPSC_ERROR_MEM
  A memory allocation failed and operation was cancelled.

  @retval ::NV_DPSC_ERROR_INVALID_MESSAGE
  The operation failed: imported message had an invalid format.

  @retval ::NV_DPSC_ERROR_INVALID_OBJECT
  The operation failed: message includes at least one invalid object.

  @retval ::NV_DPSC_ERROR_RESOURCE    
  The operation failed: message importation has encountered errors with resource e.g. storage resource.

  @retval ::NV_DPSC_ERROR_UNKNOWN
  Other error occured that prevented the processing of the server response.

  @see nvDpscOpen().
  @see nvDpscExportMessage().
*/

NV_PUBLIC_API TNvDpscStatus nvDpscImportMessage
(
  TNvSession     xProvSession,
  TNvBuffer*    pxMessage
);

/** @} */ 

#ifdef __cplusplus
}
#endif

#endif /* NV_DPSC_H */

