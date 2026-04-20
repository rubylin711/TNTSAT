/**
  @file prm_client.h

  @brief
  This file provides the common definitions to @XA interfaces.
  @if ( PROD_SINGLE || PROD_REE )
  It includes the @MA product management interface as well.
  This interface is realized by the @MA.
  @endif

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
/*                         DOCUMENTATION ORGANISATION                         */
/* -------------------------------------------------------------------------- */
/* This file is always part of other documentation and has no independent     */
/* documentation.                                                             */
/* ========================================================================== */

/* ========================================================================== */
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_nva
  @brief
  Describe all common definitions used by @XA interfaces.

  @details

  The @XA interface includes several groups of interfaces which depends on
  the integration model. These definitions introduce common definitions to any
  kind of @PROD client deployment.
*/

#ifndef PRM_CLIENT_H
#define PRM_CLIENT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_defs.h"
#include "nv_acd.h"
#include "nv_wmk_sw.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @addtogroup g_nva
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name Generic @PROD interfaces version
  @brief
  Define the generic version number of the @PROD interfaces.

  @details
  These version fields are used by the different deployments of @PROD clients.

  @{
*/

/** @brief @PROD interface generic version major number. */
#define PRMAPI_VERSION_MAJOR        1
/** @brief @PROD interface generic version medium number. */
#define PRMAPI_VERSION_MEDIUM       20
/** @brief @PROD interface generic version minor number. */
#define PRMAPI_VERSION_MINOR        1

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/**
  @if ( PROD_SINGLE || PROD_REE )

  @addtogroup g_nva_manage
  @brief
  Describe the @MA management interface of @Ma.

  @details

  This group includes the <b> @MA manager</b>.
  The latter allows handling the @Ma resources.

  @endif
*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/** @cond ( PROD_SINGLE || PROD_REE ) */

/**
  @addtogroup g_nva_manage
  @{
*/

/**
  @brief
  Property type values.
*/

typedef enum
{
  NV_PROPERTY_TRUSTED_STORAGE_PATH = 0x7FFE0000,
  /**<
    @brief
    Trusted storage path within the file system.

    @details

    The value of the trusted storage path property is a string.
    It must be set with nvSetPropertyString().
    It must be set only once after the call to nvInitialize() 
    but before any use of @PROD features -- e.g. nvAsmOpen() returns an 
    unexpected error if trusted storage path has not been set previously.
    On the opposite, nvSetPropertyString() returns an error.

    @warning
    @ref p_intro_inter_sCERT "Single CERT @XA only"@n
    The trusted storage path is required to be set for Single CERT @XA only.
    It shall not be set for other @ref p_intro_config "roots of trust".

    @see nvSetPropertyString(), nvInitialize().
  */

  NV_PROPERTY_JAVA_VM = 0x7FFE0001,
  /**<
    @brief
    Java VM handle for platform with Java interactions.

    @details

    The value of the Java VM property is a handle.
    It must be set with nvSetPropertyHandle() when the platform requires it
    e.g. Android platforms.
    In that case it must be set only once after the call to nvInitialize() 
    but before any use of @PROD features -- e.g. nvAsmOpen() returns an 
    unexpected error if Java VM has not been set previously.
    On the opposite, nvSetPropertyHandle() returns an error.

    @see nvSetPropertyHandle(), nvInitialize().
  */
  
  NV_PROPERTY_APPLICATION_CONTEXT = 0x7FFE0002,
  /**<
    @brief
    Platform application context for specific platform.

    @details

    The value of the platform application context property is a handle.
    It must be set with nvSetPropertyHandle() when the platform requires it
    e.g. Android platforms.
    In that case it must be set only once after the call to nvInitialize() 
    but before any use of @PROD features -- e.g. nvAsmOpen() returns an 
    unexpected error if platform application context has not been set 
    previously.
    On the opposite, nvSetPropertyHandle() returns an error.

    @see nvSetPropertyHandle(), nvInitialize().
  */
  
  NV_PROPERTY_RICH_CLIENT_LOG_LEVEL = 0x7FFE0003,
  /**<
    @brief
    Log level for verbose version of the @PROD Main Agent.

    @details

    The value of the log level is an integer defined as a ::TNvLogLevel.
    Only the log level of the @PROD Main Agent is configured through this property used
    with nvSetPropertyInt().

    @see nvSetPropertyInt().
  */  

  NV_PROPERTY_TRUSTED_CLIENT_LOG_LEVEL = 0x7FFE0004,
  /**<
    @brief
    Log level for verbose version of the @PROD Trusted Agent.

    @details

    The value of the log level is an integer defined as a ::TNvLogLevel.
    Only the log level of the @PROD Trusted Agent is configured through this property used
    with nvSetPropertyInt().

    @see nvSetPropertyInt().
  */  
  
  NV_PROPERTY_RICH_CLIENT_LOG_MAX_BUFFER_SIZE = 0x7FFE0005,
  /**<
    @brief
    Maximum number of characters to print for logging of buffer in the @PROD Main Agent.

    @details

    It allows limiting the maximal buffer size to dump. The default value
    defined during @XA initialization is @c 5000.
    Only the buffer size of the @PROD Main Agent is configured through this property used
    with nvSetPropertyInt().

    @see nvSetPropertyInt().
  */    
  
  NV_PROPERTY_TRUSTED_CLIENT_LOG_MAX_BUFFER_SIZE = 0x7FFE0006,
  /**<
    @brief
    Maximum number of characters to print for logging of buffer in the @PROD Trusted Agent.

    @details

    It allows limiting the maximal buffer size to dump. The default value
    defined during @XA initialization is @c 1000.
    Only the buffer size of the @PROD Trusted Agent is configured through this property used
    with nvSetPropertyInt().

    @see nvSetPropertyInt().
  */ 
  
  NV_PROPERTY_PERSO_DATA_PATH = 0x7FFE0007,
  /**<
    @brief
    Path within the file system where personalization data file can be found.

    @details

    The value of the personalization data path property is a string.
    It must be set with nvSetPropertyString() when the platform requires it.
    It must be set only once after the call to nvInitialize() 
    but before any use of @PROD features -- e.g. nvAsmOpen() returns an 
    unexpected error if personalization data path was required and has not
    been set previously.

    @warning
    The personalization data path is required to be set only in some
    deployment configurations of the non-NOCS version of @PROD client.

    @see nvSetPropertyString(), nvInitialize().
  */
  NV_PROPERTY_STB_CA_SN = 0x7FFE0008
  /**<
    @brief
    STB CA SN for STB with non NOCS CERT block.

    @details

    @warning
    This property should not be used unless Nagra requires it.
    
    Only the first call is effective. Any other call would be silently ignored.@n
    Property is not persistent and shall be called after each STB reboot and CCL initialization.
    
    @see nvSetPropertyInt(), nvInitialize().
  */
}
TNvPropertyType;

/**@}*/

/** @endcond */
/* cond ( PROD_SINGLE || PROD_REE ) */

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/* -------------------------------------------------------------------------- */
/*                          NAGRA PRODUCT MANAGEMENT                         */
/* -------------------------------------------------------------------------- */

/** @cond ( PROD_SINGLE || PROD_REE ) */

/**
  @addtogroup g_nva_manage
  @{
*/

/**
  @brief
  @Ma initialization.

  @pre
  None.

  @post
  The @Ma is initialized.

  This function allows initializing the @Ma.
  It allocates the initial @Ma resources.

  @retval ::TRUE
  The @Ma has been successfully initialized.

  @retval ::FALSE
  The @Ma initialization failed.
  The @Ma features cannot be used.

  @see nvTerminate(), @ref p_nva_manage.
*/

NV_PUBLIC_API bool_t nvInitialize
(
  void
);

/**
  @brief
  @Ma termination.

  @pre
  The @Ma is successfully initialized.

  @post
  All @Ma resources have been released.
  The @Ma can no more be operated.

  This function allows terminating the @Ma.
  It releases all allocated @Ma resources.

  @see nvInitialize(), @ref p_nva_manage.
*/

NV_PUBLIC_API void nvTerminate
(
  void
);

/**
  @brief
  Set a @Ma property string.

  @pre
  The @Ma is initialized and running.
  No @Ma features has been used.

  @post
  The property has been taken into account.

  @param[in] xPropertyType
  Type of the property to set.

  @param[in] xPropertyValue
  Value of the property to set.

  @details

  Detailed description is described with each property.
  Refer to ::TNvPropertyType for more details.
  
  @retval ::TRUE
  The provided property has been taken into account.
  
  @retval ::FALSE
  The provided property has not been taken into account:
  - The provided property type is unknown or not supported.
  - The provided property value is not valid for the provided property type.
  - The provided property cannot be set more than once.
  - The provided property cannot be set in the current @Ma state 
    (too early or too late).

  @see ::TNvPropertyType.
*/

NV_PUBLIC_API bool_t nvSetPropertyString
(
  TNvPropertyType xPropertyType,
  const TNvString xPropertyValue
);

/**
  @brief
  Set a @Ma property handle.

  @pre
  The @Ma is initialized and running.
  No @Ma features has been used.

  @post
  The property has been taken into account.

  @param[in] xPropertyType
  Type of the property to set.

  @param[in] xPropertyValue
  Value of the property to set.

  @details

  Detailed description is described with each property.
  Refer to ::TNvPropertyType for more details.
  
  @retval ::TRUE
  The provided property has been taken into account.
  
  @retval ::FALSE
  The provided property has not been taken into account:
  - The provided property type is unknown or not supported.
  - The provided property value is not valid for the provided property type.
  - The provided property cannot be set more than once.
  - The provided property cannot be set in the current @Ma state 
    (too early or too late).

  @see ::TNvPropertyType.
*/

NV_PUBLIC_API bool_t nvSetPropertyHandle
(
  TNvPropertyType xPropertyType,
  const TNvHandle xPropertyValue
);

/**
  @brief
  Set a @Ma property integer.

  @pre
  The @Ma is initialized and running.

  @post
  The property has been taken into account.

  @param[in] xPropertyType
  Type of the property to set.

  @param[in] xPropertyValue
  Value of the property to set.

  @details

  Detailed description is described with each property.
  Refer to ::TNvPropertyType for more details.
  
  @retval ::TRUE
  The provided property has been taken into account.
  
  @retval ::FALSE
  The provided property has not been taken into account:
  - The provided property type is unknown or not supported.
  - The provided property value is not valid for the provided property type.
  - The provided property cannot be set more than once.
  - The provided property cannot be set in the current @Ma state 
    (too early or too late).

  @see ::TNvPropertyType.
*/

NV_PUBLIC_API bool_t nvSetPropertyInt
(
  TNvPropertyType xPropertyType,
  const uint32_t xPropertyValue
);

/**
  @brief
  Register listener callback functions for watermark software overlay.

  @pre
  nvInitialize() must have been called.

  @post
  The listener callback functions are registered for the system.

  @details
  After being registered, the listener callback function is triggered each time
  @Ma receives a new watermark software overlay payload.
  The listener registration can be canceled by calling this function again
  with a @c NULL listener parameter. If no listener is currently registered, a
  call with a @c NULL listener value does simply nothing.

  @param[in]    pxOnWatermark
  Implementation of the Watermark software overlay interface.

  @retval ::TRUE
  The listener callback function has been registered/unregistered for the system.

  @retval ::FALSE
  The listener callback function  registration/unregistration failed.

  @see ::INvWmkSw, nvInitialize().
*/

NV_PUBLIC_API bool_t  nvSetWatermarkSoftwareListener
(
  INvWmkSw*               pxOnWatermark
);

/**
  @brief
  Log level values.
*/

typedef enum
{
  NV_LOG_LEVEL_ERRORS   = 0x7FFF0000,
  /**< Display only consistency errors. */
  NV_LOG_LEVEL_INOUT    = 0x7FFF0001,
  /**< Display consistency errors and functions arguments. */
  NV_LOG_LEVEL_ALL      = 0x7FFF0002,
  /**< Display consistency errors, function arguments and any additional traces. */
  NV_LOG_LEVEL_PROFILING      = 0x7FFF0003
  /**< Display only timing measurements */
}
TNvLogLevel;

/**
  @brief
  Set @Ma log level.

  @pre
  The @Ma is successfully initialized.

  @post
  @Ma log level is positioned to the defined level.

  This function is available only in integration version of the @XA.
  It allows setting the @XA log level (@PROD Main and @PROD Trusted agents both).
  Log level values are defined in ::TNvLogLevel.
  The default log value defined during @XA initialization is ::NV_LOG_LEVEL_ALL.

  @note
  nvSetPropertyInt() can be used for setting different log levels to @PROD Main and @PROD Trusted agents. Refer to ::NV_PROPERTY_RICH_CLIENT_LOG_LEVEL, ::NV_PROPERTY_TRUSTED_CLIENT_LOG_LEVEL.

  @param[in] xLevel
  Log level value - Refer to ::TNvLogLevel.

  @see TNvLogLevel, nvSetPropertyInt(), TNvPropertyType, ::NV_PROPERTY_RICH_CLIENT_LOG_LEVEL, ::NV_PROPERTY_TRUSTED_CLIENT_LOG_LEVEL, 
  nvLogSetMaxDumpedBufferSize(), ::NV_PROPERTY_RICH_CLIENT_LOG_MAX_BUFFER_SIZE, ::NV_PROPERTY_TRUSTED_CLIENT_LOG_MAX_BUFFER_SIZE
*/

NV_PUBLIC_API void nvLogSetLevel
(
  uint32_t  xLevel
);

/**
  @brief
  Set @Ma maximal buffer size to dump.

  @pre
  The @Ma is successfully initialized.

  @post
  Data buffer are dumped up to the defined maximal size.

  This function is available only in integration version of the @Ma.
  It allows limiting the maximal buffer size to dump (@PROD Main and @PROD Trusted agents both).@n
  The default value defined during @PROD Main Agent initialization is @c 5000.
  The default value defined during @PROD Trusted Agent initialization is @c 1000.

  @note
  nvSetPropertyInt() can be used for setting different sizes to @PROD Main and @PROD Trusted agents. Refer to ::NV_PROPERTY_RICH_CLIENT_LOG_MAX_BUFFER_SIZE, ::NV_PROPERTY_TRUSTED_CLIENT_LOG_MAX_BUFFER_SIZE.

  @param[in] xSize
  Maximal size to dump.

  @see nvSetPropertyInt(), TNvPropertyType, ::NV_PROPERTY_RICH_CLIENT_LOG_MAX_BUFFER_SIZE, ::NV_PROPERTY_TRUSTED_CLIENT_LOG_MAX_BUFFER_SIZE,
  nvLogSetLevel(), TNvLogLevel, ::NV_PROPERTY_RICH_CLIENT_LOG_LEVEL, ::NV_PROPERTY_TRUSTED_CLIENT_LOG_LEVEL 
*/

NV_PUBLIC_API void nvLogSetMaxDumpedBufferSize
(
  uint32_t  xSize
);

/**
  @brief
  Reset the Connect persistent storage from the given storage paths and the Connect client trusted data from the trusted storage.

  @pre
  The @Ma is successfully initialized. No concurrent operation must try to write the associated storage paths. The @Ma must have the permission to read and write in this directory.

  @post
  The persisted data from given storage paths and the Connect client trusted storage has been erased.
  
  @details
  This is synchronous API and the middleware must pass all the storage paths used before to erase the persisted data and connect client trusted data. This API do not delete any application sessions opened before.
  This API will also erase the trusted data stored by the product @ref p_nva_refdoc "DVL" if used.

  @warning
  The given path is a null-terminated string describing a path to a directory. It must be terminated with the directory separator related to the underlying operating system e.g. "\" for Windows local storage, "/" for POSIX directory. The current directory must be defined using ".\" or "./".

  @note
  The @Ma will ignore the inaccessible/invalid paths and continue with the other paths from the list of storage paths.

  @param[in] xPathCount
  Number of storage paths present in pxPaths.

  @param[in] pxPaths
  Array of paths from where the persisted data to be erased.

  @retval ::TRUE
  The data persisted under all the valid and accessible paths are erased successfully.
  

  @retval ::FALSE
  - The input parameter is invalid: 
    - pxPaths is NULL and xPathCount is not 0 or
    - pxPaths is not NULL and xPathCount is 0.
  - The @Ma is not initialized.
  - If the Connect client trusted agent does not support reset storage feature.

  @see ::nvInitialize ::nvAsmResetStorage ::nvAsmFactoryReset ::nvAsmUseStorage @ref p_asm_licenses
*/
NV_PUBLIC_API bool_t nvResetStorage
(
  uint32_t          xPathCount,
  const TNvBuffer* pxPaths
);


/**@}*/

/** @endcond */
/* cond ( PROD_SINGLE || PROD_REE ) */

#ifdef __cplusplus
}
#endif

#endif /* defined PRM_CLIENT_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
