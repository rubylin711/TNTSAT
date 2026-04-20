/**@@@+++@@@@******************************************************************
**
** Microsoft (r) PlayReady (r)
** Copyright (c) Microsoft Corporation. All rights reserved.
**
***@@@---@@@@******************************************************************
*/

#ifndef _DRMSECUREDELETECONSTANTS_H_
#define _DRMSECUREDELETECONSTANTS_H_ 1

#include <drmtypes.h>

ENTER_PK_NAMESPACE;

/*
** XML strings used in the construction of a secure delete challenge.
*/

extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteRootTag;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteLicenseDataTag;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteRootAttribName;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteRootAttribValue;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteChallenge1Tag;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteChallenge2Tag;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteChallenge2AttribName;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteChallenge2AttribValue;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteDataAttrib1Name;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteDataAttrib1Value;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteDataAttrib2Name;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteDataAttrib2Value;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteVersionTag;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteSessionID;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteKIDS;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteKID;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteCertificateChainTag;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteDataTag;

/*
** XML strings used in the parsing of a secure delete response.
*/

extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteVersionPath;
extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteSessionIDPath;

/*
** XML strings used in both the challenge and the response.
*/

extern DRM_GLOBAL_CONST DRM_ANSI_CONST_STRING g_dastrSecureDeleteVersionValue;


EXIT_PK_NAMESPACE;

#endif /* _DRMSECUREDELETECONSTANTS_H_ */
