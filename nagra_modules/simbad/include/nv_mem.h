/**
  @file nv_mem.h

  @brief
  This file defines the Nagra memory interface.

  @details

  It defines the function structure providing the required features as well as  
  the error messages to manage.

  This interface is realized by the device platform implementer.
  It provides to the Nagra client a memory service.

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
  @addtogroup g_mem
  @brief Describe the memory interface of Nagra clients.

  @details
  The Nagra <b>Memory</b> interface introduces definition for driving memory 
  allocation and operations. Please make sure to have read the documentation 
  pages for a complete description of the interface constraints and 
  requirements.
*/

#ifndef NV_MEM_H
#define NV_MEM_H

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
  @addtogroup g_mem
  @{
*/
/* ------------------------------------------------------------------------- */
/**
  @name Nagra memory interface version
  @brief Define the version number of the Nagra memory interface.

  @details
  This version has to be included in the Nagra memory interface structure 
  returned by nvGetMemoryInterface(). To do so, use the memory macro 
  ::MEMAPI_VERSION_INT to put it in the right format.

  @{
*/

/** @brief Nagra memory interface version major number. */
#define MEMAPI_VERSION_MAJOR     1
/** @brief Nagra memory interface version medium number. */
#define MEMAPI_VERSION_MEDIUM    0
/** @brief Nagra memory interface version minor number. */
#define MEMAPI_VERSION_MINOR     13

/**
  @brief Nagra memory interface version formatted as a single integer.
  @hideinitializer
*/
#define MEMAPI_VERSION_INT       \
    NV_INTERFACE_VERSION_INT(MEMAPI_VERSION_MAJOR, MEMAPI_VERSION_MEDIUM, MEMAPI_VERSION_MINOR)
/**
  @brief Nagra memory interface version formatted as a string.
  @hideinitializer
*/
#define MEMAPI_VERSION_STRING    \
    NV_INTERFACE_VERSION_STRING(MEMAPI_, MEMAPI_VERSION_MAJOR, MEMAPI_VERSION_MEDIUM, MEMAPI_VERSION_MINOR)

/**@}*/
/* ------------------------------------------------------------------------- */
/**@}*/

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_mem
  @brief
  This structure defines the Nagra memory interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct
{
  uint32_t version;
  /**<
    @brief Nagra memory interface version number.
    @details
    Assign it to the ::MEMAPI_VERSION_INT result.
  */

  void* (*allocate)
  (
          size_t    size
  );
  /**<
    @brief
    Allocate memory.

    @pre
    None.

    @post
    A memory object has been allocated. Its address has been provided back.

    @details

    This operation shall allocate unused memory space for an object whose 
    size in bytes is specified by @a size. The pointer returned shall be 
    suitably aligned so that it may be assigned to a pointer to any type of 
    C object and then used to access such an object in the space allocated.

    If the space cannot be allocated, a @c NULL pointer shall be returned.

    If the @a size of the space requested is 0, the returned value is 
    undefined: It must not be accessed by the caller application.

    @param[in]      size
    Size of the memory object to allocate.

    @return
    Upon successful completion with size not equal to @c 0, the operation shall 
    return a pointer to the allocated space. If size is @c 0, the value is 
    undefined and must not be accessed by the application. A @c NULL pointer 
    shall be returned if there is not enough memory space.

    @see INvMemory::free.
  */

  void (*free)
  (
          void*     mem
  );
  /**<
    @brief
    Free an allocated memory space.

    @pre
    The provided memory pointer must refer the first byte of an allocated 
    memory space.

    @post
    The memory space has been freed and is available for further allocation. 
    The pointer value is invalid and shall no more be accessed.

    @details

    This operation shall cause the space pointed to by @a mem to be freed, 
    that is, made available for further allocation. If @a mem is a null 
    pointer, no action shall occur. Otherwise, if the argument does not 
    match a pointer earlier returned by the @a INvMemory::allocate 
    operation, or if the space has already been freed by a call to this 
    operation, the behavior is undefined.

    Any use of a pointer that refers to freed memory space results in 
    undefined behavior.

    @param[in]      mem
    A pointer to the first byte of a valid allocated memory space to be freed.

    @see INvMemory::allocate.
  */

  void (*move)
  (
          void*     memdst,
    const void*     memsrc,
          size_t    size
  );
  /**<
    @brief
    Copy the content of a memory object to another memory object.

    @pre
    Provided pointer shall refer valid memory objects with sufficient 
    allocated size.

    @post
    The first @a size bytes of the provided input memory object are copied 
    in the output destination memory space. The input memory object is left 
    unchanged.

    @details

    This operation shall copy @a size bytes from the object pointed to by 
    @a memsrc into the object pointed to by @a memdst. Copying takes place 
    as if the @a size bytes from the object pointed to by @a memsrc are 
    first copied into a temporary array of @a size bytes that does not 
    overlap the objects pointed to by @a memdst and @a memsrc, and then the 
    @a size bytes from the temporary array are copied into the object 
    pointed to by @a memdst.

    @param[out]     memdst
    Destination pointer to valid allocated memory object. The @a size 
    bytes consecutive to the pointer value shall be part of the memory 
    space allocated to object pointed to by @a memdst.

    @param[in]      memsrc
    Source pointer to a valid allocated memory object. The @a size bytes 
    consecutive to the pointer value shall be part of the memory space 
    allocated to object pointed to by @a memsrc.

    @param[in]      size
    Number of bytes to be copied.
  */

  int (*compare)
  (
    const void*     mem1,
    const void*     mem2,
          size_t    size
  );
  /**<
    @brief
    Compare to memory objects

    @pre
    Provided pointers shall refer valid memory objects with sufficient 
    allocated size.

    @post
    Provided memory objects are left unchanged. The result of the 
    comparison has been provided back.

    @details

    This operation shall compare the first @a size bytes of the object 
    pointed to by @a mem1 to the first @a size bytes of the object pointed 
    to by @a mem2.

    The sign of a non-zero return value shall be determined by the sign of 
    the difference between the values of the first pair of bytes (both 
    interpreted as type uint8_t) that differ in the objects being compared.

    @param[in]      mem1
    Pointer to a first valid allocated memory object.

    @param[in]      mem2
    Pointer to a second valid allocated memory object.

    @param[in]      size
    Number of bytes to be compared. Each of the provided memory object shall 
    have been allocated such as the first @a size bytes are part of the 
    related allocation.

    @return
    This operation shall return an integer greater than, equal to, or less 
    than @c 0, if the object pointed to by @a mem1 is greater than, equal to, 
    or less than the object pointed to by @a mem2, respectively.
  */

  void (*set)
  (
          void*     mem,
          uint8_t   value,
          size_t    size
  );
  /**<
    @brief
    Fill a memory object with a value.

    @pre
    Provided pointer shall refer a valid memory object with sufficient 
    allocated size.
    
    @post
    The first @a size bytes of the provided memory object are filled with
    the provided @a value.

    @details

    This operation shall copy @a value into each of the first @a size bytes 
    of the memory object pointed to by @a mem.

    @param[out]     mem
    Destination pointer to valid allocated memory object. The first @a size 
    bytes shall have been allocated with the memory object.

    @param[in]      value
    Value to be set.

    @param[in]      size
    Number of bytes to be set.
  */

}
INvMemory;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_mem
  @brief
  Provide the Nagra memory interface.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the Nagra memory interface structure must remain 
  accessible as long as the Nagra client is running.

  @details
  This function is used by the Nagra client to retrieve the Nagra memory 
  interface structure.

  This function should be called once during Nagra client initialization but 
  it cannot be definitively assumed. Therefore the address of the structure and 
  the memory allocated to it must be valid as long as the Nagra client 
  library is loaded and running.

  @return
  A constant pointer to the Nagra memory interface structure.
    
  @see INvMemory.
*/

const INvMemory* nvGetMemoryInterface
(
  void
);

#ifdef __cplusplus
}
#endif

#endif /* NV_MEM_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
