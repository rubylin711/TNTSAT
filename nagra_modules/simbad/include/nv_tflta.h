/**
  @file nv_tflta.h

  @brief
  Define the Nagra trusted framework layers interfaces related to the trusted 
  execution environment deployment.

  @details
  The interfaces related to the trusted execution environment include the 
  following elements:
  - the trusted client interface is realized by Nagra with the Nagra trusted 
    client. It must be called and managed by the trusted execution environment 
    framework.
  - the trusted client context interface is realized by device platform 
    implementer. It provides to the Nagra trusted client a trusted client 
    context management service.

  COPYRIGHT:
    2013 - 2016 Nagravision S.A.
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
  @addtogroup g_tfl_ta
  @brief
  Describe the Nagra trusted client interfaces related to Nagra trusted 
  clients.These interfaces exist only within the trusted execution environment.

  @details

  The Nagra <b>Trusted Client</b> interfaces introduce definitions for 
  managing trusted client context and for handling trusted client run cycle.
  Please make sure to have read the documentation pages for a complete 
  description of the interface constraints and requirements.
*/

/**
  @defgroup g_tfl_tam   Nagra trusted client interface
  @ingroup g_tfl_ta
  @brief
  Define the Nagra trusted client interface entry points.

  @details

  This interface is realized by the Nagra trusted client.
  It allows the trusted execution environment to manage the Nagra trusted 
  client run cycle.

  @see @ref p_tfl_manage "Trusted client management" description,
       @ref p_tfl_invoke "Trusted client invocation" description.
*/

/**
  @defgroup g_tfl_tac   Nagra trusted client context interface
  @ingroup g_tfl_ta
  @brief
  Define the Nagra trusted client context interface.

  @details

  This interface provides to the Nagra trusted client a service for handling a 
  trusted client memory context. It must be implemented by device providers.

  @see @ref p_tfl_context "Maintain trusted client context" description.
*/

#ifndef NV_TFLTA_H
#define NV_TFLTA_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                               INCLUDE FILES                                */
/* ========================================================================== */

#include "nv_tfl.h"

/* ========================================================================== */
/*                                   TYPES                                    */
/* ========================================================================== */

/**
  @ingroup g_tfl_tac
  @brief
  This structure defines the trusted client context interface content.

  @details
  It is a collection of function pointers composing the interface.
*/

typedef struct
{
  uint32_t version;
  /**<
    @brief
    Nagra trusted client context interface version number.
    @details
    Assign it to the ::TFLAPI_VERSION_INT result.
  */

  void* (*setInstanceContext)
  (
    void*     mem
  );
  /**<
    @brief
    Define the trusted client context with the provided memory object.

    @pre
    The provided memory object, if not @c NULL, shall be a valid allocated 
    memory object located in the trusted memory space e.g. not including 
    shared memory.

    @post
    The memory object is registered as the trusted client context. It shall be 
    held as long as the trusted client is not destroyed or unless it is 
    explicitly redefined using this operation. Therefore any previously 
    defined trusted client context is replaced and returned by the operation.

    @details

    Some trusted execution environment specifications do not allow using 
    writable global data. This operation provides an alternative to that 
    restriction. The trusted environment framework must hold a reference 
    to the provided memory object pointed to by @a mem and associates it to 
    the current trusted client. The reference must be held as long as the 
    trusted client is loaded whether it is actively processing or not. The 
    provided memory block should contain any writable data that requires 
    trusted client global scope.

    This operation always provides back the previously registered trusted 
    client context. The returned can be @c NULL if none was previously 
    defined. It can even be equal to the provided memory object if the 
    latter was already registered as the trusted client context.

    This operation can be called with a @c NULL pointer canceling the 
    trusted client context registration.

    The reference held by trusted environment framework can be retrieved by 
    the trusted client by calling the 
    ::INvTrustedClientContext::getInstanceContext() operation.

    @param[in]      mem
    Pointer to a valid allocated trusted memory object or a @c NULL value: 
    The memory object must be located in the trusted memory space e.g. 
    not including shared memory. However this pointer can be @c NULL 
    canceling the registration of the trusted client context.

    @return
    The value returned is the memory object previously set as the client 
    context. It can be @c NULL if none was previously defined. 
    It can be also the same as the provided memory object if the latter 
    was already defined as the client context.

    @see ::INvTrustedClientContext::getInstanceContext(), 
         @ref p_tfl_context "Trusted client context".
  */

  void* (*getInstanceContext)
  (
    void
  );
  /**<
    @brief
    Retrieve the trusted client context.

    @pre
    None.

    @post
    None.

    @details

    This operation retrieves the reference to the memory object defined 
    for a trusted client context. It can be @c NULL or a reference to a 
    valid trusted memory object registered by a previous call to the 
    ::INvTrustedClientContext::setInstanceContext() operation.

    @return
    The returned value is the reference to the memory object currently 
    registered as the trusted client context. The return value can be @c NULL 
    if no memory object has been set yet.

    @see ::INvTrustedClientContext::setInstanceContext(), 
         @ref p_tfl_context "Trusted client context".
  */

}
INvTrustedClientContext;

/* ========================================================================== */
/*                                 FUNCTIONS                                  */
/* ========================================================================== */

/**
  @ingroup g_tfl_tac
  @brief
  Provide the trusted client context interface.

  @pre
  Interface functions must have been defined.

  @post
  The memory allocated to the Narga trusted client context interface structure 
  must remain accessible as long as the Nagra trusted client is running.

  @details
  This function is used by the Nagra trusted client to retrieve the trusted 
  client context interface structure.

  This function should be called once during Nagra trusted client 
  initialization but it cannot definitively be assumed. Therefore the address 
  of the structure and the memory allocated to it must be valid as long as the 
  Nagra trusted client is loaded and running.

  @return
  A constant pointer to a Nagra trusted client context interface structure.
    
  @see ::INvTrustedClientContext, 
       @ref p_tfl_context "Trusted client context".
*/

const INvTrustedClientContext* nvGetTrustedClientContextInterface
(
  void
);

/**
  @ingroup g_tfl_tam
  @brief
  Nagra trusted client create entry point.

  This function must be called by the trusted framework services when the Nagra 
  trusted client is loaded. It typically allocates, loads or initializes 
  Nagra trusted client general resources.

  When successful, Nagra trusted client must be ready to be called by the any 
  rich client.

  @note
  The time the Nagra trusted client is loaded is not directly related to a 
  call from rich execution environment. It is related to the specific trusted 
  client execution cycle as managed by trusted framework services.

  @pre
  The trusted framework services must be initialized.
	The Nagra trusted client is loaded but uninitialized.

  @post
  The Nagra trusted trusted client is initialized.
  
  @retval ::NV_TRUSTED_SUCCESS
  The Nagra trusted client is correctly initialized.

  @retval ::NV_TRUSTED_ERROR_TRUSTED_MEMORY
  The Nagra trusted client runs out of memory.

  @retval ::NV_TRUSTED_ERROR
  The Nagra trusted client cannot initialize.

  @see nvTrustedDestroyEntry(), nvTrustedSessionOpenEntry(), 
       @ref p_tfl_manage "Trusted client management", 
       @ref p_tfl_context "Trusted client context".
*/

NV_PUBLIC_API uint32_t nvTrustedCreateEntry
( 
  void 
);

/**
  @ingroup g_tfl_tam
  @brief
  Nagra trusted client destroy entry point.

  This function must be called by the trusted framework services when the Nagra 
  trusted client is unloaded. All resources used by the Nagra trusted client 
  are freed during this operation. When this function returns, the Nagra 
  trusted client can no more be used. In order to do so, a new initialization 
  must be performed using nvTrustedCreateEntry().

  @note
  The time the Nagra trusted client is unloaded is not directly related to a 
  call from rich execution environment. It is related to the specific trusted 
  client execution cycle as managed by trusted framework services.

  @warning
  Nagra trusted client shall free as much as possible resources it has 
  previously allocated. However Resources specific to trusted sessions are 
  not reachable from this point. Trusted sessions should be ensured to be 
  closed before the Nagra trusted client is destroyed.

  @pre
  The Nagra trusted client is loaded, initialized and running.

  @post
  All Nagra trusted client reachable resources are freed.
	Nagra trusted client is no more functional.

  @see nvTrustedCreateEntry(), 
       @ref p_tfl_manage "Trusted client management", 
       @ref p_tfl_context "Trusted client context".
*/

NV_PUBLIC_API void nvTrustedDestroyEntry
( 
  void
);

/**
  @ingroup g_tfl_tam
  @brief
  Nagra trusted client session opening entry point.

  @pre
  The Nagra trusted client must have been previously initialized using 
	nvTrustedCreateEntry().

  @post
  The Nagra trusted client has allocated and initialized the provided 
  session context reference.

  This function is called in relation with the use ::INvTrustedSession::open() 
  by the rich execution environment. Refer to ::INvTrustedSession::open() for 
  a detailed description of the relation. The present function is called for a 
  selected trusted client.

  When successful, the operation implementation may allocate and initialize a 
  session context. This context must be associated with the trusted session 
  managed by the trusted framework services. Note that the latter also 
  associates the trusted session with the targeted trusted client. When a 
  subsequent trusted operation is called using this trusted session, the 
  related trusted client must be called; the related session context must also 
  be provided to the operation.
  
  @note
  A trusted client may define no session context providing back a @c NULL 
  value. In that case, this value must be provided as context to any 
  subsequent operation call related to this session.

  @param[out] ppContext
  Reference to a context to define and associate with the session.

  @param[in]    select
  Internal selection parameter.

  @retval ::NV_TRUSTED_SUCCESS
  A trusted session has been successfully opened returning a valid session 
  context.

  @retval ::NV_TRUSTED_ERROR_TRUSTED_MEMORY
  The Nagra trusted client runs out of memory.

  @retval ::NV_TRUSTED_ERROR_BAD_PARAMETER
  The ppContext parameter is NULL.

  @retval ::NV_TRUSTED_ERROR_SECURITY
  The session has not been opened for security reason e.g. a maximum number of 
  session has been reached.

  @retval ::NV_TRUSTED_ERROR
  An unexpected error has occurred.

  @see nvTrustedCreateEntry(), nvTrustedSessionCloseEntry(), 
       nvTrustedSessionCommandEntry(), ::INvTrustedSession::open(),
       @ref p_tfl_manage, 
       @ref p_tfl_invoke "Trusted client invocation".
*/

NV_PUBLIC_API uint32_t nvTrustedSessionOpenEntry
(
  void**   ppContext,
  uint32_t   select
);

/**
  @ingroup g_tfl_tam
  @brief
  Nagra trusted client session close entry point.

  @pre
  A valid session must have previously been opened successfully using 
  nvTrustedSessionOpenEntry().

  @post
  The session associated to the provided context has been closed. All the 
  related session resources have been freed. The provided session context is 
  invalid and can no more be used with trusted operation.

  This function is called in relation with the use ::INvTrustedSession::close() 
  by the untrusted execution environment. Refer to ::INvTrustedSession::close() 
  for a detailed description of the relation. The present function is called 
  for the trusted client associated to the trusted session.

  When the operation finally returns, the session context is no more valid; 
  it must no more be used with any trusted operation call.

  @warning
  This function should not be called independently i.e. without resulting of
  the call to ::INvTrustedSession::close() from the untrusted environment.
  
  @param[in]  pContext
  Context associated with the session.

  @see nvTrustedSessionOpenEntry(), nvTrustedSessionCommandEntry(), 
       ::INvTrustedSession::close(), 
       @ref p_tfl_manage "Trusted client management", 
       @ref p_tfl_invoke "Trusted client invocation".
*/

NV_PUBLIC_API void nvTrustedSessionCloseEntry
(
  void* pContext
);

/**
  @ingroup g_tfl_tam
  @brief
  Nagra trusted client session command entry point.

  @pre
  A valid session must have previously been opened successfully using 
  nvTrustedSessionOpenEntry().

  @post
  The Nagra trusted client has processed the required command.
  The memory block parameters are updated according to their direction.

  This function is called in relation with the use ::INvTrustedSession::command() 
  by the untrusted environment. Refer to ::INvTrustedSession::command() for a 
  detailed description of the relation. The present function is called for the 
  trusted client associated to the trusted session.

  When successful, the operation implementation may output and input/output 
  memory blocks; their block sizes can be adjusted down. Output data must be
  provided back to the untrusted environment. Refer to @ref s_tfl_invoke_outputs 
  section for further details.

  @param[in]    pContext
  Context associated with the session as provided by 
  nvTrustedSessionOpenEntry().

  @param[in]    nBlocks
  Number of memory blocks provided in the pBlocks parameter.
  This number cannot be @c 0.
 
  @param[inout] pBlocks
  An array of ::TNvTrustedBlock; the number of blocks is provided with the 
  nBlocks parameter. It cannot be @c NULL.

  @retval ::NV_TRUSTED_SUCCESS
  The requested command has been successfully processed.
  All memory blocks have been updated according to their direction.

  @retval ::NV_TRUSTED_ERROR_TRUSTED_MEMORY
  The Nagra trusted client runs out of memory.

  @retval ::NV_TRUSTED_ERROR_BAD_PARAMETER
  A parameter is invalid or inconsistent:
  + The session context is @c NULL or invalid.
  + The number of memory blocks is @c 0.
  + The address of memory block array is @c NULL.
  + The number of memory blocks is not the expected one for this command.
  + One of the memory block provided is invalid 
    i.e. its @a size field is @c NULL or its @a pAddr field is NULL.

  @retval ::NV_TRUSTED_ERROR_BLOCK_TOO_SHORT
  An output block is too short to hold the expected result.
  Refer to @ref s_tfl_invoke_outputs section for further details.

  @retval ::NV_TRUSTED_ERROR_INVALID_OPERATION
  The requested command is invalid.

  @retval ::NV_TRUSTED_ERROR_NOT_SUPPORTED
  The requested command is not supported.

  @retval ::NV_TRUSTED_ERROR_SECURITY
  The command has not been processed or stopped due to security reason.

  @retval ::NV_TRUSTED_ERROR
  An unexpected error has occurred.

  @see nvTrustedSessionOpenEntry(), nvTrustedSessionCloseEntry(), 
       ::INvTrustedSession::command(), 
       @ref p_tfl_manage "Trusted client management", 
       @ref p_tfl_invoke "Trusted client invocation".
*/

NV_PUBLIC_API uint32_t nvTrustedSessionCommandEntry
(
  void*             pContext,
  uint32_t          nBlocks,
  TNvTrustedBlock*  pBlocks
);

#ifdef __cplusplus
}
#endif

#endif /* NV_TFLTA_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
