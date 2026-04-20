/**
  @file nv_tstore.h

  @brief
  This file defines the Nagra trusted storage interface. 

  @details

  It defines the function structure providing the required features as well as 
  the error messages to manage.

  This interface is realized by the device platform implementer.
  It provides to the Nagra client a trusted storage service.

  COPYRIGHT:
    2014 - 2016 Nagravision S.A.
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
/*                              INTERNAL GROUPS                               */
/* ========================================================================== */

/**
  @addtogroup g_tstore
	@brief
  Describe the Nagra trusted storage interface of Nagra clients.

  @details
  The Nagra <b>Trusted Storage</b> interface introduces definition for driving 
  sensitive data storage. Please make sure to have read the documentation pages 
  for a complete description of the interface constraints and requirements.
*/

#ifndef NV_TSTORE_H
#define NV_TSTORE_H

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
  @addtogroup g_tstore
  @{
*/
/* ------------------------------------------------------------------------- */
/**
  @name Nagra trusted storage interface version
  @brief
  Define the version number of the Nagra trusted storage interface.

  @details
  This version has to be included in the trusted storage interface structure 
  returned by nvGetTrustedStorageInterface(). To do so, use the trusted storage 
  macro ::TSTOREAPI_VERSION_INT to put it in the right format.

  @{
*/

/** @brief Nagra trusted storage interface version major number. */
#define TSTOREAPI_VERSION_MAJOR    1
/** @brief Nagra trusted storage interface version medium number. */
#define TSTOREAPI_VERSION_MEDIUM   1
/** @brief Nagra trusted storage interface version minor number. */
#define TSTOREAPI_VERSION_MINOR    9

/**
  @brief Nagra trusted storage interface version formatted as a single integer.
  @hideinitializer
*/
#define TSTOREAPI_VERSION_INT      \
    NV_INTERFACE_VERSION_INT(TSTOREAPI_VERSION_MAJOR, TSTOREAPI_VERSION_MEDIUM, TSTOREAPI_VERSION_MINOR)
/**
  @brief Nagra trusted storage interface version formatted as a string.
  @hideinitializer
*/
#define TSTOREAPI_VERSION_STRING   \
    NV_INTERFACE_VERSION_STRING(TSTOREAPI_, TSTOREAPI_VERSION_MAJOR, TSTOREAPI_VERSION_MEDIUM, TSTOREAPI_VERSION_MINOR)

/**@}*/
/* ------------------------------------------------------------------------- */
/**@}*/

/**
  @ingroup g_store
  @brief
  Trusted storage result returned by Nagra trusted storage interface functions.

  @details
  These values can be returned by any trusted storage interface.
*/

typedef enum
{
  NV_STORAGE_SUCCESS,
  /**< The operation has processed successfully. */
  NV_STORAGE_ERROR_BAD_PARAMETER,
  /**< One or more operation parameters are invalid. */
  NV_STORAGE_ERROR_NO_SPACE,
  /**< There is not enough trusted storage space to perform the requested operation. */
  NV_STORAGE_ERROR_ACCESS_CONFLICT,
  /**< The operation has encountered an error accessing to trusted storage space. */
  NV_STORAGE_ERROR_ITEM_NOT_FOUND,
  /**< An item referred by the operation cannot be found. */
  NV_STORAGE_ERROR_INCONSISTENT,
  /**< An item referred by the operation has become inconsistent. */
  NV_STORAGE_ERROR,
  /**< An unexpected error has been encountered while processing the operation. */
  NV_STORAGE_ERROR_LAST_STATUS
  /**< Not used. */
}
TNvTrustedStorageResult;

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_tstore
  @brief
  Trusted storage process callback function.

  @details
  Refer to ::INvTrustedStorage::traverse() for usage details.

  @param[in]   pxDomain
  Trusted storage domain identifier of the notified trusted storage item.

  @param[in]   pxKey
  Trusted storage key identifier of the notified trusted storage item.

  @param[in]    xSize
  Size of the notified trusted storage item content.

  @param[in]    xContext
  Previously provided client context handle.

  @retval       not-0
  The iterating request can resume with the next trusted storage item.

  @retval       0
  The iterating request must be stopped.
  
  @see ::INvTrustedStorage::traverse().
*/

typedef
uint32_t (*INvTrustedStorageProcess)
(
    const TNvIdentifier*   pxDomain,
    const TNvIdentifier*   pxKey,
          uint32_t          xSize,
          TNvHandle         xContext
);

/**
  @ingroup g_tstore
  @brief
  This structure defines the Nagra trusted storage interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct
{
  uint32_t version;
  /**<
    @brief Trusted storage version number.
  */

  uint32_t (*store)
  (
    const TNvIdentifier*   pxDomain,
    const TNvIdentifier*   pxKey,
    const TNvBuffer*       pxData
  );
  /**<
    @brief
    Store a data array in a trusted storage item.

    @pre
    None.

    @post
    The storage space is consistent.
    The data has been written in the storage space and associated with the 
    provided trusted storage identifier {domain, key}.

    @details

    This operation requests to write a byte array provided by @a pxData and 
    identified be @a pxDomain / @a pxKey identifier pair in the rusted storage 
    space.

    There are two cases: 
    - If the provided identifiers do not refer any existing data within the 
      trusted storage space, a new trusted storage item must be created and 
      associated to the provided identifiers. The provided data must be stored 
      in the newly created storage item.
    - If the provided identifiers refer an existing trusted storage item, the 
      related storage item must be updated with the provided data. 
      The latter fully replaces the old data. Note that newer data may not 
      have the same size than the older one.

    The operation must be atomic.
    The consistency of the trusted storage space must be ensured whether other 
    trusted storage operations are requested or not during this operation 
    processing, or, whether the data has been successfully written or not.
    The data shall not be partially written or inconsistent at the end of the 
    operation.
    In case an error occurred, the trusted storage space must be left 
    unchanged in the state it was before the request.

    @param[in]   pxDomain
    Trusted storage domain identifier of the trusted storage item.

    @param[in]   pxKey
    Trusted storage key identifier of the trusted storage item.

    @param[in]   pxData
    Array of bytes to be stored.

    @retval ::NV_STORAGE_SUCCESS
    The provided data has been successfully written defining the trusted 
    storage item identified by the provided {domain, key} pair.

    @retval ::NV_STORAGE_ERROR_BAD_PARAMETER
    A parameter is invalid:
    - @a pxDomain is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxKey is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxData is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.

    @retval ::NV_STORAGE_ERROR_NO_SPACE
    There is not sufficient trusted storage space to store the provided data 
    with the provided {domain, key} identifier.

    @retval ::NV_STORAGE_ERROR_ACCESS_CONFLICT
    The related trusted storage item cannot be written or updated due to 
    missing access permission.

    @retval ::NV_STORAGE_ERROR
    An unexpected error occurs during the operation.
    The trusted storage space must be in the same state than before the 
    operation call.

    @see ::TNvTrustedStorageResult, INvTrustedStorage::fetch(), 
         INvTrustedStorage::remove(), INvTrustedStorage::move(),
         @ref p_tstore_persist "Nagra trusted storage basic operations".
  */

  uint32_t (*fetch)
  (
    const TNvIdentifier*   pxDomain,
    const TNvIdentifier*   pxKey,
          TNvBuffer*       pxData
  );
  /**<
    @brief
    Fetch a data array from a trusted storage item.

    @pre
    Data must have previously written in the trusted storage space.

    @post
    Data read from trusted storage is filled in the provided data buffer and 
    its size adjusted, or, only its size is filled in if no memory array has 
    been provided.

    @details

    This operation requests to fetch from the trusted storage space a data 
    array identified by the provided @a pxDomain / @a pxKey identifier pair. 
    The stored data is copied in the provided @a pxData memory area and its 
    @a pxData size adjusted down to the actual size of the fetched data.

    The operation must be atomic.
    The consistency of the trusted storage space must be ensured whether other 
    trusted storage operations are requested or not during this operation 
    processing.
    The fetched data must also be consistent after the operation.

    If there is not enough space in the provided memory buffer, it must be 
    filled in with data starting from first byte up to the available size of 
    the memory buffer; the ::NV_STORAGE_ERROR_NO_SPACE must be returned.

    When the data field of the data memory buffer is NULL, no data must be 
    copied but the size field of the data structure must be filled in with the 
    size of the stored data.

    @param[in]   pxDomain
    Trusted storage domain identifier of the trusted storage item.

    @param[in]   pxKey
    Trusted storage key identifier of the trusted storage item.

    @param[out]  pxData
    Array of bytes to be filled in with the fetched data.
    The related memory area may be @c NULL.

    @retval ::NV_STORAGE_SUCCESS
    The provided data has been successfully fetched, or, the stored data size 
    has been correctly retrieved.

    @retval ::NV_STORAGE_ERROR_BAD_PARAMETER
    A parameter is invalid:
    - @a pxDomain is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxKey is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxData is @c NULL or, its @a data field is not @c NULL and its @a size field is @c 0.

    @retval ::NV_STORAGE_ERROR_NO_SPACE
    The size of the stored data is greater than the size of the provided 
    memory buffer. The memory buffer has been filled with the starting 
    bytes up to the size of the memory buffer.

    @retval ::NV_STORAGE_ERROR_ACCESS_CONFLICT
    The related trusted storage item cannot be fetched due to missing access 
    permission.

    @retval ::NV_STORAGE_ERROR_ITEM_NOT_FOUND
    The requested trusted storage item identified by the provided trusted 
    storage identifier does not exist in the trusted storage space.

    @retval ::NV_STORAGE_ERROR_INCONSISTENT
    The requested trusted storage item identified by the provided trusted 
    storage identifier has been detected inconsistent e.g. corrupted ...
 
    @retval ::NV_STORAGE_ERROR
    An unexpected error occurs during the operation.
    The trusted storage space must be in the same state than before the 
    operation call.

    @see ::TNvTrustedStorageResult, INvTrustedStorage::store(), 
         INvTrustedStorage::remove(), INvTrustedStorage::move(),
         @ref p_tstore_persist "Nagra trusted storage basic operations".
  */

  uint32_t (*remove)
  (
    const TNvIdentifier*   pxDomain,
    const TNvIdentifier*   pxKey
  );
  /**<
    @brief
    Remove a trusted storage item.

    @pre
    Data must have previously written in the trusted storage space.

    @post
    The trusted storage item identified by the provided trusted storage 
    {domain, key} identifier must no more exist.

    @details

    This operation requests to remove from the trusted storage space the 
    trusted storage item identified by the provided @a pxDomain / @a pxKey 
    identifier pair.
    All its content data must be erased from the trusted storage space.

    The operation must be atomic.
    The consistency of the trusted storage space must be ensured whether other 
    trusted storage operations are requested or not during this operation 
    processing.

    @param[in]    pxDomain
    Trusted storage domain identifier of the trusted storage item.

    @param[in]   pxKey
    Trusted storage key identifier of the trusted storage item.

    @retval ::NV_STORAGE_SUCCESS
    The trusted storage item identified by the provided storage identifier 
    {domain, key} has been successfully removed.

    @retval ::NV_STORAGE_ERROR_BAD_PARAMETER
    A parameter is invalid:
    - @a pxDomain is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxKey is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.

    @retval ::NV_STORAGE_ERROR_ACCESS_CONFLICT
    The related trusted storage item cannot be removed due to missing access 
    permission.

    @retval ::NV_STORAGE_ERROR_ITEM_NOT_FOUND
    The provided trusted storage identifier {domain, item} does not exist in 
    the trusted storage space.

    @retval ::NV_STORAGE_ERROR_INCONSISTENT
    The requested trusted storage item identified by the provided trusted 
    storage identifier has been detected inconsistent e.g. corrupted ...
 
    @retval ::NV_STORAGE_ERROR
    An unexpected error occurs during the operation.
    The trusted storage space must be in the same state than before the 
    operation call.

    @see ::TNvTrustedStorageResult, INvTrustedStorage::store(), 
         INvTrustedStorage::fetch(), INvTrustedStorage::move(),
         @ref p_tstore_persist "Nagra trusted storage basic operations".
  */

  uint32_t (*move)
  (
    const TNvIdentifier*    pxOldDomain,
    const TNvIdentifier*    pxOldKey,
    const TNvIdentifier*    pxNewDomain,
    const TNvIdentifier*    pxNewKey
  );
  /**<
    @brief
    Move a trusted storage item.

    @pre
    Data identified by the provided old trusted storage identifier {oldDomain, 
    oldKey} must have been previously written in the trusted storage space. 
    The provided new trusted storage identifier {newDomain, newKey} must not 
    refer an existing trusted storage item in the trusted storage space.

    @post
    The trusted storage item identified by the provided old trusted storage 
    identifier {oldDomain, oldKey} must not exist.
    The trusted storage item identified by the provided new trusted storage 
    identifier {newDomain, newKey} must exist and refer the data previously 
    associated to the provided old trusted storage identifier.

    @details

    This operation requests to move data within the trusted storage space. 
    This data was previously referred by the provided trusted storage 
    @a pxOldDomain / @a pxOldKey identifier pair.
    It must be moved to the trusted storage location identified by the 
    provided trusted storage @a pxNewDomain / @a pxNewKey identifier pair. 
    The operation failed if the new trusted storage identifier refers an 
    existing trusted storage item.

    The operation must be atomic.
    The consistency of the trusted storage space must be ensured whether other 
    trusted storage operations are requested or not during this operation 
    processing.

    @param[in]   pxOldDomain
    Old trusted storage domain identifier of the trusted storage item to move.

    @param[in]   pxOldKey
    Old trusted storage key identifier of the trusted storage item to move.

    @param[in]   pxNewDomain
    New trusted storage domain identifier for the moved trusted storage item.

    @param[in]   pxNewKey
    New trusted storage key identifier for the moved trusted storage item.

    @retval ::NV_STORAGE_SUCCESS
    The trusted storage item has been successfully moved.

    @retval ::NV_STORAGE_ERROR_BAD_PARAMETER
    A parameter is invalid:
    - @a pxOldDomain is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxOldKey is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxNewDomain is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a pxNewKey is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.

    @retval ::NV_STORAGE_ERROR_NO_SPACE
    There is not sufficient trusted storage space to store the identified 
    trusted storage item to the new location identified with the provided new 
    trusted storage identifier.

    @retval ::NV_STORAGE_ERROR_ACCESS_CONFLICT
    The related trusted storage item cannot be moved due to missing access 
    permission, or, the provided new trusted storage identifier refers an 
    existing trusted storage item.

    @retval ::NV_STORAGE_ERROR_ITEM_NOT_FOUND
    The provided old trusted storage identifier does not refer any existing 
    trusted storage item in the storage space.

    @retval ::NV_STORAGE_ERROR_INCONSISTENT
    The requested trusted storage item identified by the provided trusted 
    storage identifier has been detected inconsistent e.g. corrupted ...
 
    @retval ::NV_STORAGE_ERROR
    An unexpected error occurs during the operation.
    The trusted storage space must be in the same state than before the 
    operation call.

    @see ::TNvTrustedStorageResult, INvTrustedStorage::store(), 
         INvTrustedStorage::fetch(), INvTrustedStorage::remove(),
         @ref p_tstore_move "Migrating trusted storage item".
  */

  uint32_t (*traverse)
  (
    const TNvIdentifier*            pxDomain,
          TNvHandle                  xContext,
          INvTrustedStorageProcess  onProcess
  );
  /**<
    @brief
    Iterate through a trusted storage domain.

    @pre
    None.

    @post
    All trusted storage items related tot he selected trusted storage domain 
    must have been processed unless the client explicitly stops the iteration.
    The trusted storage space must remain consistent.

    @details

    This operation requests to iterate through all trusted storage items 
    related to the selected trusted storage domain identified by the 
    @a pxDomain identifier.
    For each trusted storage item found in the selected domain, the provided 
    @a onProcess function must be synchronously called back with the following 
    arguments:
    - The selected domain identifier referred by @a pxDomain.
    - The trusted storage key identifier of the notified trusted storage item.
      This identifier can be deallocated when the @a onProcess call returns.
    - The size of this trusted storage item content.
      If inconsistency -- e.g. corruption, access ... -- has been detected on 
      the trusted storage item, its size is set to 0.
    - The provided client context handle referred by @a xContext.

    During each @a onProcess call, the Nagra client may perform any of trusted 
    storage action on this element i.e. ::INvTrustedStorage::fetch(), 
    ::INvTrustedStorage::store(), ::INvTrustedStorage::remove() or 
    ::INvTrustedStorage::move().

    When @a onProcess call returns, the value returned must be checked:
    - If the returned value is not @c 0, the next trusted storage item related 
      to the selected trusted storage domain must be processed the same way.
    - If the returned value is @c 0, the iteration must be stopped. Iteration 
      resources can be freed and ::NV_STORAGE_SUCCESS can be returned.

    @param[in]    pxDomain
    Trusted storage domain identifier.

    @param[in]     xContext
    Client context handle to provide back with each found trusted storage item.

    @param[in]    onProcess
    Process callback to be called for each found trusted storage item.

    @retval ::NV_STORAGE_SUCCESS
    The trusted storage domain has been successfully traversed.

    @retval ::NV_STORAGE_ERROR_BAD_PARAMETER
    A parameter is invalid:
    - @a pxDomain is @c NULL, or, its @a data field is @c NULL or its @a size field is @c 0.
    - @a onProcess is @c NULL.

    @retval ::NV_STORAGE_ERROR_ACCESS_CONFLICT
    The related trusted storage domain cannot be accessed due to missing 
    access permission.

    @retval ::NV_STORAGE_ERROR_ITEM_NOT_FOUND
    The related trusted storage domain identifier does not refer any existing 
    trusted storage domain in the storage space.

    @retval ::NV_STORAGE_ERROR
    An unexpected error occurs during the traverse operation.

    @see ::TNvTrustedStorageResult, ::INvTrustedStorageProcess,
         @ref p_tstore_iterate "Iterating through trusted storage space".
  */
}
INvTrustedStorage;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_tstore

  @brief
  Provide the Nagra trusted storage interface.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the interface structure must remain accessible 
  as long as the Nagra client is running.

  @details
  This function is used by the Nagra client to retrieve the Nagra trusted 
  storage interface.

  This function should be called once during Nagra client initialization but 
  it cannot be definitively assumed. Therefore the address of the structure and 
  the memory allocated to it must be valid as long as Nagra client library is 
  loaded and running.

  @return
  A constant pointer to the Nagra trusted storage interface structure.
    
  @see ::INvTrustedStorage.
*/

const INvTrustedStorage* nvGetTrustedStorageInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NV_TSTORE_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
