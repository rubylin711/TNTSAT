/*
 * @file ca_os.c
 * @brief Nagravision Stream Processing APIs implemetation on Montage Symphony4 platform
 *
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "mt_os_impl.h"
#include "ca_os.h"

#define MT_MAGIC_CA_OS	(0xAAAA5555)

#define EMSG(fmt, ...)   printf("[ERR]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DMSG(fmt, ...)   printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)


               /* -------------------------------------------- */
               /*    Persistent memory management prototypes   */
               /* -------------------------------------------- */


/**
 *  @brief
 *    This function returns the list of available persistent memory storages.
 *
 *    A storage is a logical block of persistent memory that shall be mapped
 *    onto a physical block. A logical block of a given size will most probably
 *    require a larger physical block if a wear-leveling technique is
 *    implemented to increase the lifetime of the physical memory.
 *
 *    A persistent memory storage shall fulfill a certain number of requirements
 *    that depend on the usage made by the CA software (e.g. size of the
 *    storage, average number of writing cycles, etc). Refer to the system
 *    requirements of the CA software module concerned for further information.
 *
 *  @param[out] pxNumStorages
 *                Number of items returned in \c ppxStorageTable
 *
 *  @param[out] ppxStorageTable
 *                Table describing the characteristics of each persistent memory
 *                storage. The OSY layer is in charge of the allocation of the
 *                memory required for this table and shall guarantee that it
 *                will remain available at any time.
 *
 *  @retval  OS_NO_ERROR
 *              The storage table has been returned successfully
 *
 *  @retval  OS_ERROR
 *              The exportation of the storage table failed
 *
 *  @remarks
 *    -# The returned pointer \c ppxStorageTable must be the same as long as the
 *       decoder is not rebooted.
 *    .
 *    -# An update of the decoder's software shall not modify the persistent
 *       memory storages.
 *    .
 *    -# A persistent memory storage shall not be part of a sector containing
 *       code checked by the Nagravision secure boot procedure (SSA-SBP).
 *    .
 *    -# A persistent memory storage shall not be addressable by a middleware
 *       application interpreted by a virtual machine (OpenTV, Liberate, MHP,
 *       etc).
*/

#if 0 //libs before 1.0.5 already defined
NAGRA_CA_API TOsStatus osPersistentMemoryGetStorageTable
(
  TUnsignedInt8*                 pxNumStorages,
  TOsPersistentMemoryStorage**  ppxStorageTable
)
{
	//TODO
	return OS_NO_ERROR;
}
#endif




/**
 *  @brief
 *    This function reads a given number of bytes from a persistent memory
 *    storage and copy them in a buffer allocated by the caller.
 *
 *  @param[in] xStorageId
 *               ID of the storage data are read from
 *
 *  @param[in] xFirstByte
 *               Number of bytes, from the beginning of the storage, the reading
 *               shall start from. A value of 0 corresponds to the very first
 *               byte of the storage.
 *
 *  @param[in] xNumBytes
 *               Number of bytes to be copied in \c pxBuffer, starting from and
 *               including \c xFirstByte.
 *
 *  @param[out] pxBuffer
 *                Destination buffer where to copy the data from the persistent
 *                memory storage. This buffer is \c xNumBytes long and is
 *                allocated by the caller.
 *
 *  @retval  OS_NO_ERROR
 *              The reading succeeded
 *
 *  @retval  OS_ERROR
 *              The reading failed
 *
 *  @remarks
 *    -# The calling task guarantees not to access outside the available
 *       storage.
 *    .
 *    -# It is the driver responsibility to manage concurrent access to the
 *       physical persistent memory. In particular, if the application is
 *       running from the flash memory, the driver is responsible for the
 *       possible concurrent access to that memory. Same remark if two logical
 *       storages are mapped onto the same physical memory.
 *    .
 *    -# This function shall not return an error due to the fact that \c
 *       xNumBytes is equal to 0. In such a case, \c OS_NO_ERROR is returned and
 *       the destination buffer \c pxBuffer is not altered.
*/
#if 0 //libs before 1.0.5 already defined
NAGRA_CA_API TOsStatus osPersistentMemoryRead
(
  TOsPersistentStorageId   xStorageId,
  TSize                    xFirstByte,
  TSize                    xNumBytes,
  TUnsignedInt8*           pxBuffer
)
{
	//TODO
	return OS_NO_ERROR;
}
#endif




#if 0
/**
 *  @brief
 *    This function writes a given number of bytes into a persistent memory
 *    storage.
 *
 *  @param[in] xStorageId
 *               ID of the storage data are written to
 *
 *  @param[in]   xFlags
 *                 Deprecated
 *
 *  @param[in] xFirstByte
 *               Number of bytes, from the beginning of the storage, the
 *               writting shall start from. A value of 0 corresponds to the very
 *               first byte of the storage.
 *
 *  @param[in] xNumBytes
 *               Number of bytes to be written to the persistent memory storage,
 *               starting from and including \c xFirstByte.
 *
 *  @param[in] pxBuffer
 *               Source buffer containing the data to be written to the
 *               persistent memory storage. This buffer is \c xNumBytes long and
 *               is allocated by the caller.
 *
 *  @retval  OS_NO_ERROR
 *             The persistent memory storage has been written successfully
 *
 *  @retval  OS_ERROR
 *             An error occurred (bad ID, bad offset, write failed)
 *
 *  @remarks
 *    -# The calling task guarantees not to access outside the available
 *       storage.
 *    .
 *    -# It is the driver responsibility to manage concurrent access to the
 *       physical persistent memory. In particular, if the application is
 *       running from the flash memory, the driver is responsible for the
 *       possible concurrent access to that memory. Same remark if two logical
 *       storages are mapped onto the same physical memory.
 *    .
 *    -# The driver has to guarantee the integrity of the data at any time. In
 *       particular, this function shall be robust against untimely power
 *       cycles. For instance, if the decoder is switch off during a writing
 *       operation, the driver shall guarantee that the persistent memory
 *       storage gets back to the state that precedes the erroneous writing
 *       operation when the decoder is switch on again.
 *    .
 *    -# This function shall not return an error due to the fact that \c
 *       xNumBytes is equal to 0. In such a case, \c OS_NO_ERROR is returned
 *       without writing any bytes from the source buffer \c pxBuffer.
*/
NAGRA_CA_API TOsStatus osPersistentMemoryWrite
(
        TOsPersistentStorageId    xStorageId,
        TOsPersistentMemoryFlags  xFlags,
        TSize                     xFirstByte,
        TSize                     xNumBytes,
  const TUnsignedInt8*            pxBuffer
)
{
	//TODO
	return OS_NO_ERROR;
}

#endif



/**
 *  @brief
 *    This function returns the usage associated to a storage. Storages of a
 *    given usage shall fulfill specific requirements that depend on
 *    the CA software. Refer to the system requirements of the CA software
 *    concerned for further information.
 *
 *  @param[in] xStorageId
 *               ID of the storage
 *
 *  @param[out] pxUsage
 *                Usage associated to this storage.
 *
 *  @retval  OS_NO_ERROR
 *             The usage has been returned successfully
 *
 *  @retval  OS_ERROR
 *             An error occurred (bad ID, invalid parameter)
*/
#if 0 //libs before 1.0.5 already defined
NAGRA_CA_API TOsStatus osPersistentMemoryGetStorageUsage
(
  TOsPersistentStorageId      xStorageId,
  TOsPersistentMemoryUsage*   pxUsage
)
{
	//TODO
	return OS_NO_ERROR;
}
#endif

