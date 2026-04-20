/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#ifndef __CA_CERT_IMPL_H
#define __CA_CERT_IMPL_H

#ifdef __cplusplus
extern "C" {
#endif
#include "ca_defs.h"
#include "ca_defsx.h"
#include "ca_cert.h"

/*************************** Structure Definition ****************************/
/** \addtogroup      CERT */
/** @{ */  /** <!-- [CERT] */

/**Defines the handle resource of the nagra CERT module*/
struct SCertResourceHandle {
    TUnsignedInt64 cert_handle;  /**<the handle of cert module . */
    unsigned int magic_cert; /**<the magic data of cert handle, valide data is MAGIC_CERT . */
};

/** @} */  /** <!-- ==== Structure Definition end ==== */

/******************************* API Declaration *****************************/
/** \addtogroup      CERT */
/** @{ */  /** <!-- [CERT] */

/**
\brief Export the cert key to Montage Keyslot module.
\attention \n
Before export the cert key, you must transfer the data to Nagra Cert by Nagra APIs which are called by Nagra defined function function table certGetFunctionTable.\n
After exchange data to CERT sucdess, only can call it once, othewise it will return error.
\param [in] keyslot:  the number of keyslot, the valied dta if from 0 - 127.
\retval ::CERT_NO_ERROR Success.
\retval ::CERT_ERROR  Calling this API fails.
*/
TSignedInt32 certExportKey(TUnsignedInt32 keyslot);

/** @} */  /** <!-- ==== API Declaration End ==== */

#ifdef __cplusplus
}
#endif

#endif
