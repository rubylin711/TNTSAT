/**
  @file nv_lpsm.h

  @brief
  This file defines the @PROD link protection session interface.

  @details

  This interface is realized by the @Ma.
  It provides a link protection management service.

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
  @addtogroup g_nva_lpsm
  @brief
  Describe the Link Protection Session Manager interface of @Ma.

  @details

  The <b>Link Protection Session manager</b> interface introduces definition for 
  managing data encryption/decryption and authentication (MAC generation and verification).

  Please make sure to have read the documentation pages for a complete 
  description of the interface constraints and requirements.
*/

#ifndef NV_LPSM_H
#define NV_LPSM_H

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
  @addtogroup g_nva_lpsm
  @{
*/

/**
  @brief
  Values for link protection session operation result.

  @details
  This enumeration provides all the possible values that can be returned by a
  link protection session operation.
*/

typedef enum 
{
  NV_LPSM_SUCCESS                  = 0x00030000L,
  /**< The link protection session operation has been successful. */
  NV_LPSM_ERROR_BAD_PARAMETER      = 0x00030001L,
  /**< The link protection session operation failed: One or more provided parameters are invalid. */
  NV_LPSM_ERROR_NO_KEY             = 0x00030002L,
  /**< The link protection session operation failed: The related key cannot be found. */
  NV_LPSM_ERROR_KEY_STATUS         = 0x00030003L,
  /**< The link protection session operation failed: The related key cannot be used. */
  NV_LPSM_ERROR_UNKNOWN_CIPHERSUITE    = 0x00030004L,
  /**< The link protection session operation failed: The provided cipher suite is not supported. */
  NV_LPSM_ERROR_CRYPTOENGINE       = 0x00030005L,
  /**< The link protection session operation failed: A cryptographic operation failed. */
  NV_LPSM_ERROR_BUFFER_TOO_SHORT   = 0x00030006L,
  /**< The link protection session operation failed: A provided memory block is too short to be filled in with the expected result. */
  NV_LPSM_ERROR_NO_OPERATOR        = 0x00030007L,
  /**< The link protection session operation failed: The related application session identifier does not refer a valid application session. */
  NV_LPSM_ERROR_UNSUPPORTED_EMI    = 0x00030008L,
  /**< The link protection session operation failed: The EMI in the license is not supported. */
  NV_LPSM_ERROR                    = 0x0003FFFFL
  /**< The link protection session operation failed: An unexpected error occurred. */
}
TNvLpsmResult;

/**
  @brief
  Values for the cipher suites supported by the link protection session.

  @details
  This enumeration provides all the possible cipher suites that can be used with the Link protection session.
*/
typedef enum 
{
  NV_PSK_AES128_CMAC                     = 0,
  /**< The keys are retrieved from license(s) (pre-shared key), the cipher to encrypt/decrypt is AES-128 in CBC mode with PKCS7 padding,
       the authentication algorithm is CMAC with AES128*/
  NV_PSK_AES128_HMAC                     = 1
  /**< The keys are retrieved from license(s) (pre-shared key), the cipher to encrypt/decrypt is AES-128 in CBC mode with PKCS7 padding,
       the authentication algorithm is HMAC*/
}
TNvCipherSuiteType;

/**
  @brief Define a pointer on a structure providing the parameters to open a Link Protection Session.
  @ingroup g_nva_lpsm

  @details
  The real type of the structure depends of the cipher suite type .
*/
typedef void* TCipherSuiteParameters;

/**
  @brief Define the structure providing the parameters to open a Link Protection Session for the ::NV_PSK_AES128_CMAC cipher suite type.
  @ingroup g_nva_lpsm

  @details
  The structure simply maps to a TNvIdentifier.
*/
typedef struct
{
	TNvIdentifier preSharedKeySetIdentifier;
    /**< String allowing to retrieve the persisted license containing the encryption key and the CMAC key. 
	     The \0 character is not included in length */
} TNvPSKAES128CMACParameters;

/**
  @brief Define the structure providing the parameters to open a Link Protection Session for the ::NV_PSK_AES128_HMAC cipher suite type.
  @ingroup g_nva_lpsm

  @details
  The structure simply maps to a TNvIdentifier.
*/
typedef struct
{
	TNvIdentifier preSharedKeySetIdentifier;
    /**< String allowing to retrieve the persisted license containing the encryption key and the HMAC key. 
	     The \0 character is not included in length */
} TNvPSKAES128HMACParameters;

/* addtrogroup g_nva_lpsm */
/** @} */

/* ========================================================================== */
/* Types                                                                      */
/* ========================================================================== */

/* ========================================================================== */
/* Functions                                                                  */
/* ========================================================================== */

/**
  @addtogroup g_nva_lpsm
  @{
*/

/**
  @brief
  Open a link protection session.

  @pre
  @Ma must have been initialized.  

  @post
  A new link protection session has been opened and associated to the cipher suite and initialization parameter.
  Internal link protection session resources have been allocated.

  This function allows opening a new link protection session. The new link protection 
  session is associated to the application session identifier provided in @a xApplicationSession. 
  
  The new link protection session identifier is provided back in 
  @a *pxLinkProtectionSession. This parameter must not be @c NULL. The operation 
  failed resulting with ::NV_LPSM_ERROR_BAD_PARAMETER if it is @c NULL.

  It fails also with ::NV_LPSM_ERROR_NO_OPERATOR if the @a xApplicationSession 
  does not refer a valid @Ma session.
  
  It fails also if the cipher suite is unknown or the cipher suite parameters are incorrect.
  
  If the operation failed, a ::NV_SESSION_INVALID value 
  is provided back in @a *pxLinkProtectionSession.
  

  @param[out]   pxLinkProtectionSession
  Reference to a link protection session identifier. 
  It shall not be @c NULL.
  The referred value is set with the new link protection session identifier or 
  with the ::NV_SESSION_INVALID value if an error occurred.

  @param[in]    xApplicationSession
  Application session identifier to associate with the new link protection 
  session. This identifier must be valid.

  @param[in]    xCipherSuiteType
  Type of cipher suite (the key derivation, the encryption/decryption and authentication algorithm), 
  refer to ::TNvCipherSuiteType description.

  @param[in]    pxCipherSuiteParameters
  Structure providing the cipher suite parameters which will be used in this session.
  Structure to be provided for ::NV_PSK_AES128_CMAC is a TNvPSKAES128CMACParameters.
  Structure to be provided for ::NV_PSK_AES128_HMAC is a TNvPSKAES128HMACParameters.

  @retval ::NV_LPSM_SUCCESS
  A new link protection session has been opened and the initial keys have been found.

  @retval ::NV_LPSM_ERROR_NO_OPERATOR
  The provided @a xApplicationSession identifier does not refer a valid 
  application session.

  @retval ::NV_LPSM_ERROR_BAD_PARAMETER
  A parameter is invalid if:
  + The @a pxLinkProtectionSession parameter is @c NULL.
  + The @a pxCipherSuiteParameters parameter is @c NULL.

  @retval ::NV_LPSM_ERROR_UNKNOWN_CIPHERSUITE
  The provided @a xCipherSuiteType identifying the algorithms to be used in the 
  link protection function is not supported by the @Ma.

  @retval ::NV_LPSM_ERROR_NO_KEY
  Error returned when xCipherSuiteType value is ::NV_PSK_AES128_CMAC or ::NV_PSK_AES128_HMAC and  the key-set identifier provided with @a pxCipherSuiteParameters didn't allow to 
  retrieve a stored license (and a key set) for further operations.

  @retval ::NV_LPSM_ERROR
  An unexpected error has occurred (memory resource ...).

  @see nvLpsmClose(),
*/

NV_PUBLIC_API uint32_t nvLpsmOpen
(
  TNvSession*  pxLinkProtectionSession,
  TNvSession   xApplicationSession,
  TNvCipherSuiteType xCipherSuiteType,
  TCipherSuiteParameters   pxCipherSuiteParameters
);

/**
  @brief
  Close a link protection session.

  @pre
  None.

  @post
  The provided link protection session -- whether valid or not -- is considered 
  closed and its identifier shall no more be used. Internal link protection 
  session resources have been released.

  This function closes a link protection session referred by 
  @a xLinkProtectionSession and previously opened with nvLpsmOpen(). 
  The operation releases all session-related resources and shall never fail
  whether the provided link protection session identifier is valid or not.

  @param[in]    xLinkProtectionSession
  link protection session identifier.

  @see nvLpsmOpen()
*/

NV_PUBLIC_API void nvLpsmClose
(
  TNvSession  xLinkProtectionSession
);



/**
  @brief
  Encrypt data block.

  @pre
  The provided link protection session and key identifier must be valid.

  @post
  The provided data block has been encrypted with the key
  related to the provided key identifier. 

  This function encrypts a block of data within the link protection session 
  referred by a valid @a xLinkProtectionSession. The data to encrypt is referred 
  by the @a pxInData parameter: it shall not be @c NULL and must contain a 
  valid memory block. The encrypted data is copied into the memory area 
  referred by the @a pxOutData parameter: it shall not be @c NULL and must 
  contain a valid memory block with enough space. However the memory area 
  referred @a pxOutData can be the same as the memory area referred by 
  @a pxInData.

  The key identifier allows identifying the related key to use 
  for processing the data. 

  The initialization vector referred by @a pxInitializationVector should be 
  provided for encryption method requiring initialization vector. In that case 
  an initialization vector is first fetched by the application and initially provided 
  to the @Ma. 

  The xLastData parameter is related to padding management. It 
  allows signalling whether the data is not the last data of the current 
  processed chain -- @a xLastData = ::FALSE -- or not -- @a xLastData = ::TRUE. The
  @Ma will add the padding when @a xLastData = ::TRUE. The caller shall allocate one block more than
  the input size to be sure that the additional bytes due to the padding can fit into the output buffer.
  The size of the @a pxOutData buffer will be modified by the @Ma to indicate the final size of the encrypted data.
  Moreover when @a xLastData is set to ::TRUE and the encryption method 
  requires an initialization vector, the next call to the encrypt operation 
  shall be provided with a newly fetched initialization vector.

  Once the related key has been found, @Ta checks the key 
  usage rules. If the key cannot be found or if it is not entitled to be used, 
  the operation failed resulting with ::NV_LPSM_ERROR_KEY_STATUS. The details of 
  the key status is set in the value referred by @a pxKeyStatus if the latter 
  is not @c NULL. If it is provided @c NULL, it is simply ignored and the key 
  status error is lost. The key status values are described in ::TNvKeyStatus.

  @param[in]    xLinkProtectionSession
  Valid link protection session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the key to be used.
  This key identifier is a UUID in binary format (16 byte long)

  @param[in]    pxInitializationVector
  Optional valid reference to an initialization vector buffer structure.  
  If @a xLast is set to false, next calls may have @c NULL @a pxInitializationVector.

  @param[in]    xLastData
  Continuity indicator for padding management. 
  This parameter may not be relevant depending on the encryption method.

  @param[in]    pxInData
  Valid input data block to process.

  @param[out]   pxOutData
  Valid output data block for processed data. 
  This structure may refer the input data block so that the processing is done
  in place. The length of the output is increased to take into account the padding
  hence the actual amount of bytes used in buffer is set in the @c size field of
  this buffer.

  @param[out]   pxKeyStatus
  Status of the key identified by the key identifier.
  This parameter may be @c NULL.

  @retval ::NV_LPSM_SUCCESS
  The data block has been correctly processed.

  @retval ::NV_LPSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xLinkProtectionSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.
  + @a pxInitializationVector is NULL at first invocation.
  + @a pxInData or @a pxOutData is @c NULL.
  + @a pxInData->data or @a pxOutData->data is @c NULL.

  @retval ::NV_LPSM_ERROR_NO_KEY
  There is currently no keys referred by the key identifier provided with
  @a pxKeyIdentifier.

  @retval ::NV_LPSM_ERROR_KEY_STATUS
  The key related to the key identifier -- or selected by default -- is 
  not present or cannot be used.
  The provided data has not been processed: a detailed key status can 
  be found in @a pxKeyStatus if not @c NULL.

  @retval ::NV_LPSM_ERROR_UNSUPPORTED_EMI
  For pre-shared key: the encryption method associated to the key is not supported.

  @retval ::NV_LPSM_ERROR_CRYPTOENGINE
  The underlying stream crypto-engine reports an error.

  @retval ::NV_LPSM_ERROR_BUFFER_TOO_SHORT
  The buffer provided for receiving the encrypted data @a pxOutData is short in
  size. Processing was canceled and the requested data size was set in the @c size 
  field of the @a pxOutData buffer.

  @retval ::NV_LPSM_ERROR
  An unexpected error has occurred (memory resource, ...).

  @see nvLpsmOpen()

*/
NV_PUBLIC_API uint32_t nvLpsmEncrypt
(
  TNvSession      xLinkProtectionSession,
  TNvIdentifier*  pxKeyIdentifier,
  TNvBuffer*      pxInitializationVector,
  bool_t          xLastData,
  TNvBuffer*      pxInData,
  TNvBuffer*      pxOutData,
  uint32_t*       pxKeyStatus
);


/**
  @brief
  Decrypt data block.

  @pre
  The provided link protection session and key identifier must be valid.

  @post
  The provided data block has been decrypted with the key
  related to the provided key identifier. 

  This function decrypts a block of data within the link protection session 
  referred by a valid @a xLinkProtectionSession. The data to decrypt is referred 
  by the @a pxInData parameter: it shall not be @c NULL and must contain a 
  valid memory block. The decrypted data is copied into the memory area 
  referred by the @a pxOutData parameter: it shall not be @c NULL and must 
  contain a valid memory block with enough space. However the memory area 
  referred @a pxOutData can be the same as the memory area referred by 
  @a pxInData.

  The key identifier allows identifying the related key to use 
  for processing the data. 

  The initialization vector referred by @a pxInitializationVector should be 
  provided for decryption method requiring initialization vector. In that case 
  an initialization vector is first fetched by the application and initially provided 
  to the @Ma. 

  The xLastData parameter is related to padding management. It 
  allows signalling whether the data is not the last data of the current 
  processed chain -- @a xLastData = ::FALSE -- or not -- @a xLastData = ::TRUE. The
  @Ma will remove the padding when @a xLastData = ::TRUE. 
  The size of the @a pxOutData buffer will be modified by the @Ma to indicate the final size of the decrypted data.
  Moreover when @a xLastData is set to ::TRUE and the decryption method 
  requires an initialization vector, the next call to the decrypt operation 
  shall be provided with a newly fetched initialization vector.

  Once the related key has been found, @Ta checks the key 
  usage rules. If the key cannot be found or if it is not entitled to be used, 
  the operation failed resulting with ::NV_LPSM_ERROR_KEY_STATUS. The details of 
  the key status is set in the value referred by @a pxKeyStatus if the latter 
  is not @c NULL. If it is provided @c NULL, it is simply ignored and the key 
  status error is lost. The key status values are described in ::TNvKeyStatus.

  @param[in]    xLinkProtectionSession
  Valid link protection session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the key to be used.
  This key identifier is a UUID in binary format (16 byte long)

  @param[in]    pxInitializationVector
  Valid reference to an initialization vector structure.
  If @a xLast is set to false, next calls may have @c NULL @a pxInitializationVector.

  @param[in]    xLastData
  Continuity indicator for padding management. 
  This parameter may not be relevant depending on the encryption method.

  @param[in]    pxInData
  Valid input data block to process.

  @param[out]   pxOutData
  Valid output data block for processed data. 
  This structure may refer the input data block so that the processing is done
  in place. The length of the output is reduced to take into account the padding
  hence the actual amount of bytes used in buffer is set in the @c size field of
  this buffer.

  @param[out]   pxKeyStatus
  Status of the key identified by the key identifier.
  This parameter may be @c NULL.

  @retval ::NV_LPSM_SUCCESS
  The data block has been correctly processed.

  @retval ::NV_LPSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xLinkProtectionSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.
  + @a pxInitializationVector is NULL at first invocation.
  + @a pxInData or @a pxOutData is @c NULL.
  + @a pxInData->data or @a pxOutData->data is @c NULL.

  @retval ::NV_LPSM_ERROR_NO_KEY
  There is currently no keys referred by the key identifier provided with
  @a pxKeyIdentifier.

  @retval ::NV_LPSM_ERROR_KEY_STATUS
  The key related to the key identifier -- or selected by default -- is 
  not present or cannot be used.
  The provided data has not been processed: a detailed key status can 
  be found in @a pxKeyStatus if not @c NULL.

  @retval ::NV_LPSM_ERROR_UNSUPPORTED_EMI
  For pre-shared key: the decryption method associated to the key is not supported.

  @retval ::NV_LPSM_ERROR_CRYPTOENGINE
  The underlying stream crypto-engine reports an error.

  @retval ::NV_LPSM_ERROR_BUFFER_TOO_SHORT
  The buffer provided for receiving the deciphered data @a pxOutData is short in
  size. Processing was canceled and the requested data size was set in the @c size 
  field of the @a pxOutData buffer.

  @retval ::NV_LPSM_ERROR
  An unexpected error has occurred (memory resource, error in padding ...).

  @see nvLpsmOpen()

*/
NV_PUBLIC_API uint32_t nvLpsmDecrypt
(
  TNvSession      xLinkProtectionSession,
  TNvIdentifier*  pxKeyIdentifier,
  TNvBuffer*      pxInitializationVector,
  bool_t          xLastData,
  TNvBuffer*      pxInData,
  TNvBuffer*      pxOutData,
  uint32_t*       pxKeyStatus
);

/**
  @brief
  Produce an authentication tag of the given data block.

  @pre
  The provided link protection session and key identifier must be valid.

  @post
  The provided data block has been authenticated with the key
  related to the provided key identifier. 

  This function authenticates a block of data within the link protection session 
  referred by a valid @a xLinkProtectionSession. The data to authenticate is referred 
  by the @a pxInData parameter: it shall not be @c NULL and must contain a 
  valid memory block. The authentication tag is copied into the memory area 
  referred by the @a pxSignature parameter: it shall not be @c NULL and must 
  contain a valid memory block with enough space. The size of this block depends on the 
  cipher suite.

  The key identifier allows identifying the related key to use 
  for authenticating the data. 

  Once the related key has been found, @Ta checks the key 
  usage rules. If the key cannot be found or if it is not entitled to be used, 
  the operation failed resulting with ::NV_LPSM_ERROR_KEY_STATUS. The details of 
  the key status is set in the value referred by @a pxKeyStatus if the latter 
  is not @c NULL. If it is provided @c NULL, it is simply ignored and the key 
  status error is lost. The key status values are described in ::TNvKeyStatus.

  @param[in]    xLinkProtectionSession
  Valid link protection session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the key to be used.
  This key identifier is a UUID in binary format (16 byte long)

  @param[in]    pxInData
  Valid input data block to process.

  @param[out]   pxSignature
  Valid output data block for the authentication tag. 
  The actual amount of bytes used in buffer is set in the @c size field of
  this buffer.
  @note The block size to be provided for ::NV_PSK_AES128_CMAC is at least 16 bytes.
  @note The block size to be provided for ::NV_PSK_AES128_HMAC is exactly 32 bytes.

  @param[out]   pxKeyStatus
  Status of the key identified by the key identifier.
  This parameter may be @c NULL.

  @retval ::NV_LPSM_SUCCESS
  The data block has been correctly processed.

  @retval ::NV_LPSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xLinkProtectionSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.
  + @a pxInData or @a pxSignature is @c NULL.
  + @a pxInData->data or @a pxSignature->data is @c NULL.
  + @a pxSignature->size is incorrect for the corresponding cipher suite.

  @retval ::NV_LPSM_ERROR_NO_KEY
  There is currently no keys referred by the key identifier provided with
  @a pxKeyIdentifier.

  @retval ::NV_LPSM_ERROR_KEY_STATUS
  The key related to the key identifier -- or selected by default -- is 
  not present or cannot be used.
  The provided data has not been processed: a detailed key status can 
  be found in @a pxKeyStatus if not @c NULL.

  @retval ::NV_LPSM_ERROR_UNSUPPORTED_EMI
  For pre-shared key: the authentication method associated to the key is not supported.

  @retval ::NV_LPSM_ERROR_CRYPTOENGINE
  The underlying stream crypto-engine reports an error.

  @retval ::NV_LPSM_ERROR_BUFFER_TOO_SHORT
  The buffer provided for receiving the computed tag @a pxSignature is short in
  size. Processing was canceled and the requested data size was set in the @c size 
  field of the @a pxSignature buffer.

  @retval ::NV_LPSM_ERROR
  An unexpected error has occurred (memory resource, ...).

  @see nvLpsmOpen()

*/
NV_PUBLIC_API uint32_t nvLpsmAuthenticate
(
  TNvSession      xLinkProtectionSession,
  TNvIdentifier*  pxKeyIdentifier,
  TNvBuffer*      pxInData,
  TNvBuffer*      pxSignature,
  uint32_t*       pxKeyStatus
);

/**
  @brief
  Verify an authentication tag for a given data block.

  @pre
  The provided link protection session and key identifier must be valid.

  @post
  The result (::TRUE or ::FALSE) of the verification of the authentication is available at the address
  pointed by @a pxResult. 

  This function checks the authenticity of a block of data within the link protection session 
  referred by a valid @a xLinkProtectionSession. The data and the authentication tag to check are referred 
  respectively by the @a pxInData parameter and by the @a pxSignature parameter: they shall not be @c NULL and must contain a 
  valid memory block. The size of the authentication tag depends on the 
  cipher suite.

  The key identifier allows identifying the related key to use 
  for authenticating the data. 

  Once the related key has been found, @Ta checks the key 
  usage rules. If the key cannot be found or if it is not entitled to be used, 
  the operation failed resulting with ::NV_LPSM_ERROR_KEY_STATUS. The details of 
  the key status is set in the value referred by @a pxKeyStatus if the latter 
  is not @c NULL. If it is provided @c NULL, it is simply ignored and the key 
  status error is lost. The key status values are described in ::TNvKeyStatus.

  @param[in]    xLinkProtectionSession
  Valid link protection session identifier.

  @param[in]    pxKeyIdentifier
  Valid key identifier allowing identifying the key to be used.
  This key identifier is a UUID in binary format (16 byte long)

  @param[in]    pxInData
  Valid input data block to process.

  @param[in]   pxSignature
  Valid input data block for the authentication tag to check. 
  @note The block size to be provided for ::NV_PSK_AES128_CMAC is exactly 16 bytes.
  @note The block size to be provided for ::NV_PSK_AES128_HMAC is exactly 32 bytes.

  @param[out]   pxKeyStatus
  Status of the key identified by the key identifier.
  This parameter may be @c NULL.

  @param[out]   pxResult
  Pointer on the boolean variable holding the result of the verification.
  
  @retval ::NV_LPSM_SUCCESS
  The data block has been correctly processed.

  @retval ::NV_LPSM_ERROR_BAD_PARAMETER
  A parameter is invalid:
  + @a xLinkProtectionSession does not refer a valid session.
  + @a pxKeyIdentifier is @c NULL or its @c data field is 
    @c NULL or its @c size field is 0.
  + @a pxInData or @a pxSignature is @c NULL.
  + @a pxInData->data or @a pxSignature->data is @c NULL.
  + @a pxSignature->size is incorrect for the corresponding cipher suite.

  @retval ::NV_LPSM_ERROR_NO_KEY
  There is currently no keys referred by the key identifier provided with
  @a pxKeyIdentifier.

  @retval ::NV_LPSM_ERROR_KEY_STATUS
  The key related to the key identifier -- or selected by default -- is 
  not present or cannot be used.
  The provided data has not been processed: a detailed key status can 
  be found in @a pxKeyStatus if not @c NULL.

  @retval ::NV_LPSM_ERROR_UNSUPPORTED_EMI
  For pre-shared key: the authentication method associated to the key is not supported.

  @retval ::NV_LPSM_ERROR_CRYPTOENGINE
  The underlying stream crypto-engine reports an error.

  @retval ::NV_LPSM_ERROR
  An unexpected error has occurred (memory resource, ...).

  @see nvLpsmOpen()

*/
NV_PUBLIC_API uint32_t nvLpsmVerify
(
  TNvSession      xLinkProtectionSession,
  TNvIdentifier*  pxKeyIdentifier,
  TNvBuffer*      pxInData,
  TNvBuffer*      pxSignature,
  uint32_t*       pxKeyStatus,
  bool_t*         pxResult
);

/* @addtogroup g_nva_lpsm */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* NV_LPSM_H */

/* ========================================================================== */
/* End of File                                                                */
/* ========================================================================== */
