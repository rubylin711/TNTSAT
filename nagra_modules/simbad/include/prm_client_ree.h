/**
  @file prm_client_ree.h

  @brief
  This file defines the @Ma interface.

  @details

  This interface is realized by the @Ma.
  It provides the @PROD service available within the untrusted execution
  environment.

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

#ifndef PRM_CLIENT_REE_H
#define PRM_CLIENT_REE_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "prm_asm.h"
#include "prm_lds.h"
#include "prm_dsm.h"
#include "nv_lpsm.h"
#include "nv_imsm.h"

/* ========================================================================== */
/*                                DEFINITIONS                                 */
/* ========================================================================== */

/**
  @addtogroup g_nva
  @{
*/
/* -------------------------------------------------------------------------- */
/**
  @name @Ma interfaces version
  @brief
  Define the version number of the @Ma interfaces.

  @details
  This version can be provided as an integer using the macro
  ::PRMAPI_VERSION_INT or as a string using  the macro ::PRMAPI_VERSION_STRING.

  @{
*/

/**
  @brief
  @Ma interface version formatted as a single integer.
  @hideinitializer
*/
#define PRMAPI_VERSION_INT      \
    NV_INTERFACE_VERSION_INT(   \
        PRMAPI_VERSION_MAJOR,   \
        PRMAPI_VERSION_MEDIUM,  \
        PRMAPI_VERSION_MINOR    \
    )
/**
  @brief
  @Ma interface version formatted as a string.
  @hideinitializer
*/
#define PRMAPI_VERSION_STRING       \
    NV_INTERFACE_VERSION_STRING(    \
        PRMREEAPI_,                 \
        PRMAPI_VERSION_MAJOR,       \
        PRMAPI_VERSION_MEDIUM,      \
        PRMAPI_VERSION_MINOR        \
    )

/**@}*/
/* -------------------------------------------------------------------------- */
/**@}*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* defined PRM_CLIENT_REE_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
