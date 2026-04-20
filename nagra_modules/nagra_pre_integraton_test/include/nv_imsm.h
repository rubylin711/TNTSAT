/**
  @file nv_imsm.h

  @brief
  This file defines the @PROD in-band messaging session interface.

  @details

  This interface is realized by the @Ma.
  It provides a service to get filter information for EMM and to decrypt the received EMM.

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
/* Internal groups                                                            */
/* ========================================================================== */

/**
  @addtogroup g_nva_imsm
  @brief
  Describe the In-band Message Session Manager interface of @Ma.

  @details

  The <b>In-band Message Session manager</b> interface introduces definition for 
  retrieving filtering information for EMM in DVB streams and for decrypting the received EMM.

  Please make sure to have read the @ref p_imsm_overview and @ref p_imsm_EMM_processing documentation 
  pages for a complete description of the interface constraints and requirements.

*/

#ifndef NV_IMSM_H
#define NV_IMSM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/* Dependencies                                                               */
/* ========================================================================== */

#include "nv_defs.h"

/* ========================================================================== */
/* Definitions                                                                */
/* ========================================================================== */

/**
  @addtogroup g_nva_imsm
  @{
*/

/**
  @brief
  Values for In-band Message session operation result.

  @details
  This enumeration provides all the possible values that can be returned by a
  In-band Message session operation.
*/

typedef enum 
{
  NV_IMSM_SUCCESS                  = 0x00040000L,
  /**< The In-band Message session operation has been successful. */
  NV_IMSM_ERROR_BAD_PARAMETER      = 0x00040001L,
  /**< The In-band Message session operation failed: One or more provided parameters are invalid. */
  NV_IMSM_ERROR_CRYPTOENGINE       = 0x00040002L,
  /**< The In-band Message session operation failed: A cryptographic operation failed. */
  NV_IMSM_ERROR_NO_OPERATOR        = 0x00040003L,
  /**< The In-band Message session operation failed: The related application session identifier does not refer a valid application session. */
  NV_IMSM_ERROR_BUFFER_TOO_SHORT   = 0x00040004L,
  /**< The In-band Message session operation failed: A provided memory block is too short to be filled in with the expected result. */
  NV_IMSM_ERROR_INVALID_MESSAGE    = 0x00040005L,
  /**< The In-band Message session operation failed: The provided EMM is not valid. */
  NV_IMSM_ERROR_NEED_PROVISIONING  = 0x00040006L,
  /**< The In-band Message session operation failed: The @PROD client need to be provisioned. */
  NV_IMSM_ERROR                    = 0x0004FFFFL
  /**< The In-band Message session operation failed: An unexpected error occurred. */
}
TNvImsmResult;


/* addtrogroup g_nva_imsm */
/** @} */

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */

/**
  @addtogroup g_nva_imsm
  @{
*/

/**
  @brief
  Define the binary representation of a CASID.

  @details
  Value of the CASID represented as a raw 2-bytes array (16 bits).
*/
typedef uint8_t TNvCASID[2];

/**
  @brief
  Define value binary representation of a filter.

  @details
  Value of a filter represented as a raw 11-bytes array (88 bits).
*/
typedef uint8_t TNvBinFilterValue[11];

/**
  @brief
  Define mask binary representation.

  @details
  Mask represented as a raw 11-bytes array (88 bits).
*/
typedef uint8_t TNvBinFilterMask[11];

/**
  @brief
  Structure regrouping the information about the filter to be used by the application.
*/
typedef struct
{
  TNvBinFilterValue  value;
  /**< Value to be matched in the EMM header */
  TNvBinFilterMask   mask;
  /**< Mask to select the appropriate bits to be matched in the EMM header */
  uint8_t filterSize;
  /**< Effective size of the filter: only the filterSize first bytes of value and mask can be taken into account */
}
TNvFilter;


/* addtrogroup g_nva_imsm */
/** @} */

/* ========================================================================== */
/* Functions                                                                  */
/* ========================================================================== */

/**
  @addtogroup g_nva_imsm
  @{
*/

/**
  @brief
  Open a In-band Message session.

  @pre
  @Ma must have been initialized.  

  @post
  A new In-band Message session has been opened and associated 
  to the application session identifier provided in @a xApplicationSession. 
  
  The new In-band Message session identifier is provided back in 
  @a *pxInBandMessageSession. This parameter must not be @c NULL. The operation 
  failed resulting with ::NV_IMSM_ERROR_BAD_PARAMETER if it is @c NULL.

  It fails also with ::NV_IMSM_ERROR_NO_OPERATOR if the @a xApplicationSession 
  does not refer a valid @Ma session.
  
  If the operation failed, a ::NV_SESSION_INVALID value 
  is provided back in @a *pxInBandMessageSession.
  

  @param[out]   pxInBandMessageSession
  Reference to a In-band Message session identifier. 
  It shall not be @c NULL.
  The referred value is set with the new In-band Message session identifier or 
  with the ::NV_SESSION_INVALID value if an error occurred.

  @param[in]    xApplicationSession
  Application session identifier to associate with the new In-band Message 
  session. This identifier must be valid.

  @retval ::NV_IMSM_SUCCESS
  A new In-band Message session has been opened and linked to the operator keys.

  @retval ::NV_IMSM_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid 
  application session.

  @retval ::NV_IMSM_ERROR_BAD_PARAMETER
  A parameter is invalid if:
  + The @a pxInBandMessageSession parameter is @c NULL.

  @retval ::NV_IMSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvImsmClose(), @ref p_imsm_overview, @ref p_imsm_EMM_processing
*/

NV_PUBLIC_API uint32_t nvImsmOpen
(
  TNvSession* pxInBandMessageSession,
  TNvSession   xApplicationSession
);

/**
  @brief
  Close an In-band Message session.

  @pre
  None.

  @post
  The provided In-band Message session -- whether valid or not -- is considered 
  closed and its identifier shall no more be used. Internal In-band Message 
  session resources have been released.

  This function closes an In-band Message session referred by 
  @a pxInBandMessageSession and previously opened with nvImsmOpen(). 
  The operation releases all session-related resources and shall never fail
  whether the provided In-band Message session identifier is valid or not.

  @param[in]    xInBandMessageSession
  In-band Message session identifier.

  @see nvImsmOpen(), @ref p_imsm_overview, @ref p_imsm_EMM_processing
*/

NV_PUBLIC_API void nvImsmClose
(
  TNvSession  xInBandMessageSession
);

/**
  @brief
  Retrieve the @PROD CAS identifier for a specific application session.

  @pre
  The provided application session identifier must refer a valid @Ma session.

  @post
  None.

  This function provides the @PROD CAS identifier associated to application session
  defined by @Ma session. It allows to identify the PID for the @PROD EMMs within the CAT (Control Access Table) of the DVB stream. 
  Filters will be set on this PID.

  @PROD CAS identifier is provided back in the memory area described by 
  @a pxCASIdentifier. This parameter must not be @c NULL and must be the address of a buffer of size equal (or greater) to 2.

  @note
  @PROD CAS identifier usually depends on configured operator.

  @param[in]    xApplicationSession
  Application session identifier for which must be provided back the related 
  @PROD CAS identifier. This identifier must be valid.

  @param[out]   xCASIdentifier
  Address of a memory area (2 bytes long) to be filled in with the @PROD CAS identifier. 

  @retval ::NV_IMSM_SUCCESS
  @a pxCASIdentifier has been successfully filled in with the related 
  @PROD CAS identifier associated to this application session.
  Its @a size field has been adjusted to the actual size of the filled data.

  @retval ::NV_IMSM_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid 
  application session.

  @retval ::NV_IMSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is ::NV_SESSION_INVALID.
  + @a pxCASIdentifier is @c NULL.

  @retval ::NV_IMSM_ERROR
  An unexpected error occurs during processing.

  @see nvImsmGetFilters(), @ref p_imsm_overview, @ref p_imsm_EMM_processing, @ref p_asm_appid
*/
NV_PUBLIC_API uint32_t nvImsmGetCASIdentifier
(
  TNvSession xApplicationSession,
  TNvCASID   xCASIdentifier
);

/**
  @brief
  Retrieve the @PROD filters for a specific application session.

  @pre
  The provided application session identifier must refer a valid @Ma session.

  @post
  None.

  This function provides the @PROD filters to set on the PID retrieved from the analysis of the CAT. 
  @PROD EMM retrieved by filtering must be provided 
  to the In-band Message session using nvImsmDecryptEMM(). 

  @PROD filters are provided back in the memory area described by 
  @a pxFiltersArray. This parameter can be valid or @c NULL:
  + If @c NULL, no data is copied but the @c pxNumberOfFilters parameter is set with the 
    expected number of filters for holding the requested @PROD filters.
    The operation is considered successful.
  + If valid, the @c pxNumberOfFilters field must be set with the number of filters allocated at address @c pxFiltersArray.
    The requested @PROD filters are copied into the memory block if 
    the number indicated at address @c pxNumberOfFilters is greater than the needed number. The output @c pxNumberOfFilters is 
    adjusted to the actual number of filters copied.
    If the number of allocated filters is too short, the operation failed with 
    ::NV_IMSM_ERROR_BUFFER_TOO_SHORT.
    In that case, no data is copied but the @c pxNumberOfFilters is set with the 
    expected number required to hold the @PROD filters.


  @param[in]    xApplicationSession
  Application session identifier for which must be provided back the related 
  @PROD filters. This identifier must be valid.

  @param[out]   pxFiltersArray
  Address of a memory area to be filled in with the @PROD filters. 

  @param[inout]   pxNumberOfFilters
  Number of filters returned by the @PROD. 

  @retval ::NV_IMSM_SUCCESS
  @a pxFiltersArray has been successfully filled in with the related 
  @PROD filters associated to this application session and/or the @a pxNumberOfFilters parameter has been adjusted to the actual number of filters to be returned.

  @retval ::NV_IMSM_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid 
  application session.

  @retval ::NV_IMSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xApplicationSession is ::NV_SESSION_INVALID.
  + @a pxFiltersArray is @c NULL.

  @retval ::NV_IMSM_ERROR_BUFFER_TOO_SHORT
  The memory block described by @a pxFiltersArray is too short.

  @retval ::NV_IMSM_ERROR
  An unexpected error occurs during processing.

  @see nvImsmDecryptEMM(), @ref p_imsm_overview, @ref p_imsm_EMM_processing, @ref p_asm_appid
*/
NV_PUBLIC_API uint32_t nvImsmGetFilters
(
  TNvSession   xApplicationSession,
  TNvFilter*  pxFiltersArray,
  uint8_t*    pxNumberOfFilters
);

/**
  @brief
  Process received EMM.

  @pre
  The provided In-band Message session must be valid.

  @post
  The provided EMM has been decrypted and the private CCL commands processed. 
  The commands for the application (a.k.a. IRD commands) are provided back in the input buffer.

  This function decrypts an EMM within the In-band Message session 
  referred by a valid @a xInBandMessageSession. The data to decrypt is referred 
  by the @a pxInOutData parameter: it shall not be @c NULL and must contain a 
  valid memory block. The decrypted EMM is copied into the same memory area 
  referred by the @a pxOutData parameter. The clear data of the EMM being smaller
  than the encrypted data (the header is also removed), the size field of the @a pxInOutData parameter
  is adjusted by the @Ma.
  In case of command for the @Ma, the size of the @a pxOutData parameter is equal to 0.

  @param[in]    xInBandMessageSession
  Valid In-band Message session identifier.

  @param[inout]    pxInOutData
  Valid EMM to process.

  @retval ::NV_IMSM_SUCCESS
  The EMM has been correctly processed.

  @retval ::NV_IMSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xInBandMessageSession does not refer a valid session.
  + @a pxInOutData is @c NULL.
  + @a pxInOutData->data is @c NULL.

  @retval ::NV_IMSM_ERROR_CRYPTOENGINE
  The underlying crypto-engine reports an error.
  
  @retval ::NV_IMSM_ERROR_INVALID_MESSAGE
  The provided EMM is not correct (truncated or too large or corrupted or not for the corresponding application session... ).
  
  @retval ::NV_IMSM_ERROR_NEED_PROVISIONING
  The provided EMM cannot be processed since the application session is not provisioned (secrets are not available for decryption/signature verification)
  
  @retval ::NV_IMSM_ERROR
  An unexpected error has occurred (memory resource, error in padding ...).

  @see nvImsmOpen(), @ref p_imsm_overview, @ref p_imsm_EMM_processing

*/
NV_PUBLIC_API uint32_t nvImsmDecryptEMM
(
  TNvSession   xInBandMessageSession,
  TNvBuffer*  pxInOutData
);


/* @addtogroup g_nva_imsm */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_IMSM_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
