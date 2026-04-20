/**
  @file nv_wmk.h

  @brief
  This file defines the Nagra watermark interface. 

  @details

  It defines functions required to configure the watermark IP core

  This interface is implemented by the SoC vendor of the platform

  COPYRIGHT:
    2017 Nagravision S.A.
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
/*                               CHANGE HISTORY                               */
/* ========================================================================== */

/** @page p_wmk_history Changes history
  - <b> 1.1.0 - 23-Feb-2018 </b>
    -  Added INvWatermark::getState() function
    -  NV_WMK_NOT_APPLICABLE was associated to INvWatermark::configure() instead of 
       INvWatermark::configureByPipe()
  - <b> 1.0.1 - 25-Jan-2018 </b>
    -  Added NV_WMK_NOT_APPLICABLE status
  - <b> 1.0.0 - 09-Nov-2017 </b>
    -  First issue
*/

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_watermark
  @brief Describe the Nagra watermark interface.
*/

#ifndef NV_WMK_H
#define NV_WMK_H

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
  @defgroup g_watermark    Nagra watermark interface
  @defgroup g_defs         Basic and common definitions
*/

/**
  @addtogroup g_watermark
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name Nagra watermark interface version
  @brief
  Define the version number of the Nagra watermark interface.

  @details
  This version has to be included in the watermark interface structure returned by 
  nvGetWatermarkInterface(). To do so, use the macro ::WMKAPI_VERSION_INT to 
  put it in the right format.

  @{
*/

/** @brief Nagra watermark interface version major number. */
#define WMKAPI_VERSION_MAJOR       1
/** @brief Nagra watermark interface version medium number. */
#define WMKAPI_VERSION_MEDIUM      1
/** @brief Nagra watermark interface version minor number. */
#define WMKAPI_VERSION_MINOR       0

/**
  @brief Nagra watermark interface version formatted as a single integer.
  @hideinitializer
*/
#define WMKAPI_VERSION_INT       \
    NV_INTERFACE_VERSION_INT(WMKAPI_VERSION_MAJOR, WMKAPI_VERSION_MEDIUM, WMKAPI_VERSION_MINOR)
/**
  @brief Nagra watermark interface version formatted as a string.
  @hideinitializer
*/
#define WMKAPI_VERSION_STRING    \
    NV_INTERFACE_VERSION_STRING(WMKAPI_, WMKAPI_VERSION_MAJOR, WMKAPI_VERSION_MEDIUM, WMKAPI_VERSION_MINOR)

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @brief
    Defines an invalid transport session identifier.
 
*/
#define NV_WMK_TRANSPORT_SESSION_INVALID      ((uint32_t)(-1))


/**
  @ingroup g_watermark
  @brief
  Result returned by Nagra watermark interface functions.
*/

typedef enum
{
  NV_WMK_SUCCESS,
  /**< The operation has completed successfully. */
  NV_WMK_ERROR,
  /**< An unexpected error has occurred while processing the operation. */
  NV_WMK_NOT_APPLICABLE,
  /**< The operation is not applicable */
  NV_WMK_LAST_STATUS
  /**< Not used. */
}
TNvWmkResult;


/**
  @ingroup g_watermark
  @brief
  Watermark embedder state
*/

typedef enum
{
  NV_WMK_STATE_UNKNOWN,
  /**< The state is unknown */
  NV_WMK_STATE_STOPPED,
  /**< The watermark embedder is stopped (i.e. content is not watermarked) */
  NV_WMK_STATE_RUNNING,
  /**< The watermark embedder is running (i.e. content is watermarked) */
  NV_WMK_LAST_STATE
  /**< Not used. */
}
TNvWmkState;


/**
  @ingroup g_watermark
  @brief
  This structure defines the Nagra watermark interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct {

  uint32_t version;
  /**<
    @brief
    Nagra watermark interface version number.
    @details
    Assign it to the ::WMKAPI_VERSION_INT result.
  */

  uint32_t (*configure)
  (
    const uint8_t* pxConfig,
          uint32_t  xConfigSize
  );
  /**<
    @brief
    Configures watermark IP cores on all media pipelines.
   
    @details
    This function configures all watermark IP core instances with the given
    configuration.

    @param[in] pxConfig
    Buffer containing the watermark configuration.

    @param[in] xConfigSize
    Size in bytes of the configuration.

    @retval ::NV_WMK_SUCCESS
    The operation completed successfully

    @retval ::NV_WMK_ERROR
    The operation failed
  */

  uint32_t (*configureByPipe)
  (
          uint32_t    xTransportSessionId,
          uint16_t    xKeySlotId,     
    const uint8_t*   pxConfig,
          uint32_t    xConfigSize
  );
  /**<
    @brief
    Configures watermark IP cores related to a given pipeline.
   
    @details
    This function configures watermark IP core instances associated to a given
    pipeline. The pipeline is identified by the transport session ID and key
    slot ID.

    @param[in] xTransportSessionId
    Identifier of the media pipeline associated to the decrypted stream to be
    watermarked.

    @param[in] xKeySlotId
    Identifier of the key table slot associated to the decrypted stream to be 
    watermarked.

    @param[in] pxConfig
    Buffer containing the watermark configuration.

    @param[in] xConfigSize
    Size in bytes of the configuration.

    @retval ::NV_WMK_SUCCESS
    The operation completed successfully

    @retval ::NV_WMK_ERROR
    The operation failed

    @retval ::NV_WMK_NOT_APPLICABLE
    The operation is not applicable. For instance, the operation is called for
    a pipe without any IP core embedder.
  */

  uint32_t (*getState)
  (
    uint32_t    xTransportSessionId,
    uint16_t    xKeySlotId,
    uint32_t*  pxState
  );
  /**<
    @brief
    Returns the state of the watermark embedder related to a given pipeline.
   
    @details
    This function returns the state of the watermark IP core embedder related to 
    a given pipeline to inform whether it is running or stopped.
    
    @param[in] xTransportSessionId
    Identifier of the media pipeline related to the IP core embedder for which 
    the watermark state is requested

    @param[in] xKeySlotId
    Identifier of the key table slot related to the IP core embedder for which 
    the watermark state is requested

    @param[out] pxState
    State indicating whether an IP core is running (::NV_WMK_STATE_RUNNING) or
    stopped (::NV_WMK_STATE_STOPPED). If the state is unknown, it is set to
    ::NV_WMK_STATE_UNKNOWN. See also ::TNvWmkState.

    @retval ::NV_WMK_SUCCESS
    The operation completed successfully

    @retval ::NV_WMK_ERROR
    The operation failed

    @retval ::NV_WMK_NOT_APPLICABLE
    The operation is not applicable. For instance, the operation is called for
    a pipe without any IP core embedder.
  */

} INvWatermark;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_watermark
  @brief
  Provide the watermark interface structure.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the Nagra watermark interface structure must remain 
  accessible as long as the Nagra client is running.

  @details
  This function is used by the Nagra client to retrieve the Nagra watermark 
  interface structure.

  This function should be called once during Nagra client initialization but 
  it cannot be definitively assumed. Therefore the address of the structure 
  and the memory allocated to it must be valid as long as the Nagra client 
  is loaded and running.

  @return
  A constant pointer to the Nagra watermark interface structure.
    
  @see ::INvWatermark.
*/

const INvWatermark* nvGetWatermarkInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NV_WMK_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
