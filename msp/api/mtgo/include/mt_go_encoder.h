/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
 
#ifndef __MT_GO_ENCODER_H__
#define __MT_GO_ENCODER_H__

#include "mt_go_comm.h"

#ifdef __cplusplus
extern "C"{
#endif  /*__cplusplus*/

/*************************** Structure Definition ****************************/
/** \addtogroup      MTGO_ENC */
/** @{ */  /** <!-- [MTGO_ENC] */

/**Encoder attributes*/
/** CNcomment:编码器属性*/
typedef struct 
{
   MTGO_IMGTYPE_E ExpectType;   /**<Type of the encoded picture*//**<CNcomment:编码图片类型*/
   mt_u32 QualityLevel;        /**<The quality level ranges from 1 to 99. The higher the level, the better the quality, and the greater the encoded picture or occupied memory. The QualityLevel parameter is valid for .jpeg pictures only.*//**<CNcomment:1-99级, 级别越高，质量越好，编码出来图像文件或内存也越大，只对JPEG有效*/ 	
}MTGO_ENC_ATTR_S;
/** @} */  /*! <!-- Structure Definition end */

/******************************* API declaration *****************************/
/** \addtogroup      MTGO_ENC */
/** @{ */  /** <!-- [MTGO_ENC] */
 /** 
\brief Initializes the encoder. CNcomment:编码器初始化 CNend
\attention \n
When ::MT_GO_Init is called, this application programming interface (API) is also called.
CNcomment: ::MT_GO_Init中已包含对该接口的调用 CNend
\param  N/A

\retval ::MT_SUCCESS 
\retval ::MT_FAILURE
\retval ::MTGO_ERR_DEPEND_TDE

\see \n
::MT_GO_Init \n
::MT_GO_DeinitDecoder
*/

mt_s32 MT_GO_InitEncoder(mt_void);


/** 
\brief Deinitializes the encoder. CNcomment:编码器去初始化 CNend
\attention \n
When ::MT_GO_Deinit is called, this API is also called.
CNcomment: ::MT_GO_Deinit中已包含对该接口的调用 CNend
\param  N/A

\retval ::MT_SUCCESS 
\retval ::MTGO_ERR_NOTINIT

\see \n
::MT_GO_Deinit \n
::MT_GO_InitDecoder
*/

mt_s32 MT_GO_DeinitEncoder(mt_void);

/**
\brief Encodes a surface in a specified format, and saves the encoded surface in a file. 
CNcomment:将一个surface编码到指定图像格式并保存到文件中 CNend
\attention \n
The .bmp encoding format is supported.
The hardware platform determines whether the .jpeg encoding foramt is supported.
CNcomment:支持编码成bmp格式文件 \n
(是否支持编码成JPEG,取决于硬件平台) CNend

\param[in] hSurface  Surface to be encoded. CNcomment:需要编码的surface, CNend
\param[in] pFile    Name of the encoded file. This parameter can be empty. If this parameter is not set, the encoded file is named [year]-[month]-[date]-[hour]-[minute]-[second].  
                        CNcomment:编码后的文件名，可以为空，为空则用当前时间[年]-[月]-[日]-[时]-[分]-[秒]来命名 CNend
\param[in] pAttr    Encoding attributes. This parameter cannot be empty. CNcomment:编码设置的属性，不可为空 CNend

\retval ::MT_SUCCESS Success.
\retval ::MT_FAILURE
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_UNSUPPORTED
\retval ::MTGO_ERR_NOMEM
\retval ::MTGO_ERR_INVFILE
\retval ::MTGO_ERR_INVSRCTYPE
\retval ::MTGO_ERR_INVIMAGETYPE
\retval ::MTGO_ERR_INVPARAM
\retval ::MTGO_ERR_INVMIRRORTYPE
\retval ::MTGO_ERR_INVROTATETYPE
\retval ::MTGO_ERR_INVCKEYTYPE
\retval ::MTGO_ERR_INVROPTYPE
\retval ::MTGO_ERR_NOCOLORKEY
\retval ::MTGO_ERR_INVPIXELFMT
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVCOMPTYPE
\retval ::MTGO_ERR_LOCKED
\retval ::MTGO_ERR_DEPEND_TDE
\retval ::MTGO_ERR_EMPTYRECT
\retval ::MTGO_ERR_OUTOFBOUNDS
\retval ::MTGO_ERR_INTERNAL
\retval ::MTGO_ERR_DEPEND_JPGE

\see \n
*/
mt_s32 MT_GO_EncodeToFile(mt_handle hSurface, const mt_char* pFile, const MTGO_ENC_ATTR_S* pAttr);

/**
\brief Encodes a surface in a specified picture format and saves it in a memory. Ensure that the memory is sufficient.
CNcomment:将一个surface编码到指定图像格式并保存到内存中，请自行保证指定内存块足够大 CNend
\attention \n
The data can be encoded as .bmp data, and saved in a specified memory.
The hardware platform determines whether the .jpeg encoding format is supported.
CNcomment:支持编码成bmp格式数据存放到指定的内存中 
(是否支持编码成JPEG,取决于硬件平台) CNend

\param[in] hSurface     Surface to be encoded. CNcomment:需要编码的surface, CNend
\param[in] pMem         Start address of the memory for storing the encoded pictures. CNcomment:保存编码后的图像数据内存块的起始地址 CNend
\param[in] MemLen       Size of a specified memory. CNcomment:指定内存的大小 CNend
\param[out] pOutLen     Actual size of the used memory, indicating the length of encoded data. Ensure that the value of pOutLen is smaller than or equal to the value of MemLen. 
                                CNcomment:实际使用的内存大小，编码后数据长度(自行确保pOutLen <= MemLen) CNend
\param[in] pAttr        Encoding attributes. This parameter cannot be empty. CNcomment:编码设置的属性，不可为空 CNend

\retval ::MT_SUCCESS
\retval ::MT_FAILURE
\retval ::MTGO_ERR_NULLPTR
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_UNSUPPORTED
\retval ::MTGO_ERR_NOMEM
\retval ::MTGO_ERR_INVFILE
\retval ::MTGO_ERR_INVSRCTYPE
\retval ::MTGO_ERR_INVFILE
\retval ::MTGO_ERR_INVIMAGETY
\retval ::MTGO_ERR_INVIMAGETYPE
\retval ::MTGO_ERR_INVPARAM
\retval ::MTGO_ERR_INVMIRRORTYPE
\retval ::MTGO_ERR_INVROTATETYPE
\retval ::MTGO_ERR_INVCKEYTYPE
\retval ::MTGO_ERR_INVROPTYPE
\retval ::MTGO_ERR_NOCOLORKEY
\retval ::MTGO_ERR_INVPIXELFMT
\retval ::MTGO_ERR_INTERNAL
\retval ::MTGO_ERR_INVHANDLE
\retval ::MTGO_ERR_INVCOMPTYPE
\retval ::MTGO_ERR_LOCKED
\retval ::MTGO_ERR_DEPEND_TDE
\retval ::MTGO_ERR_EMPTYRECT
\retval ::MTGO_ERR_OUTOFBOUNDS
\retval ::MTGO_ERR_DEPEND_JPGE

\see \n
*/

mt_s32 MT_GO_EncodeToMem(mt_handle hSurface, mt_u8* pMem, mt_u32 MemLen, mt_u32* pOutLen, const MTGO_ENC_ATTR_S* pAttr);
  
/** @} */  /** <!-- ==== API declaration end ==== */


#ifdef __cplusplus
}
#endif  /*__cplusplus*/

#endif /* __MT_GO_ENCODER_H__ */


