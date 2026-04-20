/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Metadata moudule
 \author Montage Co., Ltd.
 \date 2008-2018
 \version 1.0
 \author
 \date 2017-11-10
 */

#ifndef __SVR_METADATA_H__
#define __SVR_METADATA_H__

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/***************************** Structs Definition ******************************/
/** \addtogroup     suplayer */
/** @{ */ /** <!--[suplayer] */

#define MT_META_MAX_KEY_NUM (100)
#define MT_META_MAX_BUF_LEN (1024)

/** Metadata value type numeration. It is used to differentiate value types. */
/** CNcomment:Metadata 值类型枚举，用来区分不同的数值类型 */
typedef enum mtSVR_PLAYER_VALUE_E {
    MT_SVR_PLAYER_VALUE_U8 = 0,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_U8.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为MT_U8 */
    MT_SVR_PLAYER_VALUE_CHAR,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_CHAR.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为MT_CHAR */
    MT_SVR_PLAYER_VALUE_S32,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_S32.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为mt_s32 */
    MT_SVR_PLAYER_VALUE_S64,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_S64.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为MT_S64 */
    MT_SVR_PLAYER_VALUE_U8P,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_U8 *.The array length is MT_SVR_PLAYERKVP_S::u32ValueNum.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为MT_U8*,数组长度为MT_SVR_PLAYERKVP_S::u32ValueNum */
    MT_SVR_PLAYER_VALUE_CHARP,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_CHAR *. The array length is MT_SVR_PLAYERKVP_S::u32ValueNum.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为MT_CHAR*,数组长度为MT_SVR_PLAYERKVP_S::u32ValueNum */
    MT_SVR_PLAYER_VALUE_S32P,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_S32 *. The array length is MT_SVR_PLAYERKVP_S::u32ValueNum.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为mt_s32*,数组长度为MT_SVR_PLAYERKVP_S::u32ValueNum */
    MT_SVR_PLAYER_VALUE_S64P,
    /**<The type of MT_SVR_PLAYERKVP_S::unValue is MO_S64 *.The array length is MT_SVR_PLAYERKVP_S::u32ValueNum.*/ /**< CNcomment:MT_SVR_PLAYERKVP_S::unValue的类型为MT_S64*,数组长度为MT_SVR_PLAYERKVP_S::u32ValueNum */
    MT_SVR_PLAYER_VALUE_BUTT,
    /**<Unknown types*/ /**< CNcomment:其它未知类型 */
} MT_SVR_PLAYER_VALUE_E;

/** Metadata value union */
/** CNcomment:Metadata值联合体 */
typedef union mtSVR_PLAYER_VALUE_U
{
    MO_U8 u8Value;
    MO_U8 *pu8Value;
    MO_CHAR cValue;
    MO_CHAR *pszValue;
    MO_S32 s32Value;
    MO_S32 *ps32Value;
    MO_S64 s64Value;
    MO_S64 *ps64Value;
} MT_SVR_PLAYER_VALUE_U;

/** Data structure of the metadata key value pair (KVP) */
/** CNcomment:Metadata键值对结构体 */
typedef struct mtSVR_PLAYER_KVP_S
{
    MO_CHAR *pszKey; /**< Metadata keyword pointer, ended with '\0'*/                                               /**< CNcomment: Metadata关键字指针，以'\0'结尾 */
    MT_SVR_PLAYER_VALUE_U unValue; /**<Value union structure. The value type is defined by eValueType.*/            /**< CNcomment:value联合结构，可以通过eValueType确定类型 */
    MO_U32 u32ValueNum; /**<When unValue is pointer, it indicates the number of elements. The default value is 1.*/ /**< CNcomment:当unValue为指针时，为所指元素的个数，默认为1 */
    MT_SVR_PLAYER_VALUE_E eValueType; /**<unValue type*/                                                            /**< CNcomment:unValue的类型 */
} MT_SVR_PLAYERKVP_S;

/**Metadata data structure*/
/** CNcomment:Metadata结构体 */
typedef struct mtSVR_PLAYER_METADATA_S
{
    MT_SVR_PLAYERKVP_S *pstKvp; /**<Point to the allocated KVP array*/  /**< CNcomment:指向分配的键值对(KVP)数组 */
    MO_U32 u32KvpUsedNum; /**<Number of KVPs in the pstKvp array*/      /**< CNcomment:pstKvp数组中键值对(KVP)的数目 */
    MO_U32 u32KvpTotalNum; /**<Size of the pre-allocated pstKvp array*/ /**< CNcomment:预分配的pstKvp数组的大小 */

    MO_U8 *pu8Buff; /**<Buffer address*/                      /**< CNcomment:缓存空间地址 */
    MO_U32 u32BuffLen; /**<Buffer length pointed by pu8Buff*/ /**< CNcomment:pu8Buff指向的缓存区长度 */
    MO_U32 u32BuffPos; /**<Current position of pu8Buff*/      /**< CNcomment:pu8Buff缓存区的当前位置 */
} MT_SVR_PLAYER_METADATA_S;

/******************************* Interface declaration *****************************/
#ifdef MT_ADVCA_FUNCTION_RELEASE
#define META_PRINTF(fmt, ...)
#else
#define META_PRINTF printf
#endif

#define MT_SVR_META_INIT(_meta)                                   \
    do {                                                          \
	if (NULL != (_meta)) {                                    \
	    memset((_meta), 0, sizeof(MT_SVR_PLAYER_METADATA_S)); \
	}                                                         \
    } while (0)

#define MT_SVR_META_PRINT(_meta) /*lint -save -e774*/                               \
    do {                                                                            \
	MT_SVR_PLAYER_METADATA_S *_pstMeta = (MT_SVR_PLAYER_METADATA_S *)(_meta);   \
	MO_U32 i = 0, j = 0;                                                        \
	MO_U32 _s32ValLen = 0;                                                      \
	if (NULL == _pstMeta)                                                       \
	    break;                                                                  \
	META_PRINTF("Metadata Begin:\n");                                           \
	for (i = 0; i < _pstMeta->u32KvpUsedNum; i++) {                             \
	    META_PRINTF("    key:%-16s value:", _pstMeta->pstKvp[i].pszKey);        \
	    _s32ValLen = _pstMeta->pstKvp[i].u32ValueNum;                           \
	    switch (_pstMeta->pstKvp[i].eValueType) {                               \
	    case MT_SVR_PLAYER_VALUE_U8:                                            \
		META_PRINTF("0x%x", _pstMeta->pstKvp[i].unValue.u8Value);           \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_U8P:                                           \
		for (j = 0; j < _s32ValLen; j++)                                    \
		    META_PRINTF("0x%x ", _pstMeta->pstKvp[i].unValue.pu8Value[j]);  \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_CHAR:                                          \
		META_PRINTF(":%c", _pstMeta->pstKvp[i].unValue.cValue);             \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_CHARP:                                         \
		META_PRINTF("%s", _pstMeta->pstKvp[i].unValue.pszValue);            \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_S32:                                           \
		META_PRINTF("%d", _pstMeta->pstKvp[i].unValue.s32Value);            \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_S32P:                                          \
		for (j = 0; j < _s32ValLen; j++)                                    \
		    META_PRINTF("%d ", _pstMeta->pstKvp[i].unValue.ps32Value[j]);   \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_S64:                                           \
		META_PRINTF("%lld ", _pstMeta->pstKvp[i].unValue.s64Value);         \
		break;                                                              \
	    case MT_SVR_PLAYER_VALUE_S64P:                                          \
		for (j = 0; j < _s32ValLen; j++)                                    \
		    META_PRINTF("%lld ", _pstMeta->pstKvp[i].unValue.ps64Value[j]); \
		break;                                                              \
	    default:                                                                \
		META_PRINTF("Unkown type!\n");                                      \
		break;                                                              \
	    }                                                                       \
	    META_PRINTF("\n");                                                      \
	}                                                                           \
	META_PRINTF("Metadata End.\n");                                             \
    } while (0) /*lint -restore*/

#define MT_SVR_META_COUNT(_meta, _key, _num)                                      \
    do {                                                                          \
	MT_SVR_PLAYER_METADATA_S *_pstMeta = (MT_SVR_PLAYER_METADATA_S *)(_meta); \
	MO_CHAR *_pszKey = (MO_CHAR *)(_key);                                     \
	if (NULL == _pstMeta || NULL == _pszKey)                                  \
	    break;                                                                \
	MO_U32 i = 0;                                                             \
	for (i = 0; i < _pstMeta->u32KvpUsedNum; i++) {                           \
	    if (!strncmp(_pszKey, _pstMeta->pstKvp[i].pszKey, strlen(_pszKey)))   \
		(_num)++;                                                         \
	}                                                                         \
    } while (0)

#define MT_SVR_META_CLEAR(_meta)                                                  \
    do {                                                                          \
	MT_SVR_PLAYER_METADATA_S *_pstMeta = (MT_SVR_PLAYER_METADATA_S *)(_meta); \
	if (NULL == _pstMeta) {                                                   \
	    break;                                                                \
	}                                                                         \
	if (NULL != _pstMeta->pu8Buff) {                                          \
	    free(_pstMeta->pu8Buff);                                              \
	    _pstMeta->pu8Buff = NULL;                                             \
	    _pstMeta->u32BuffLen = 0;                                             \
	    _pstMeta->u32BuffPos = 0;                                             \
	}                                                                         \
	if (NULL != _pstMeta->pstKvp) {                                           \
	    free(_pstMeta->pstKvp);                                               \
	    _pstMeta->pstKvp = NULL;                                              \
	    _pstMeta->u32KvpTotalNum = 0;                                         \
	    _pstMeta->u32KvpUsedNum = 0;                                          \
	}                                                                         \
	(_meta) = 0;                                                              \
    } while (0)

#define MT_SVR_META_INSERT(_meta, _key, _valtype, _vallen, _value, _ret) /*lint -save -e944 -e774 -e644 -e737*/                                                \
    do {                                                                                                                                                       \
	MT_SVR_PLAYER_METADATA_S *_pstMeta = (MT_SVR_PLAYER_METADATA_S *)(_meta);                                                                              \
	const MO_CHAR *_pszKey = (const MO_CHAR *)(_key);                                                                                                      \
	MT_SVR_PLAYER_VALUE_E _eValType = (MT_SVR_PLAYER_VALUE_E)(_valtype);                                                                                   \
	MT_SVR_PLAYER_VALUE_U _unValue;                                                                                                                        \
	MO_U8 *_pu8Value = (MO_U8 *)(_value);                                                                                                                  \
	MO_U32 _s32ValLen = (MO_U32)(_vallen);                                                                                                                 \
	MO_U32 _s32ValNum = (MO_U32)(_vallen);                                                                                                                 \
	MO_U32 _s32KeyLen = 0;                                                                                                                                 \
	MO_U8 *_pu8Tmp = NULL;                                                                                                                                 \
	MO_U32 i = 0;                                                                                                                                          \
	if (NULL == _pstMeta || NULL == _pszKey || NULL == _pu8Value) {                                                                                        \
	    _ret = MT_FAILURE;                                                                                                                                 \
	    break;                                                                                                                                             \
	}                                                                                                                                                      \
	_s32KeyLen = strlen(_pszKey);                                                                                                                          \
	if (_s32ValLen <= 0 || _s32KeyLen <= 0) {                                                                                                              \
	    _ret = MT_FAILURE;                                                                                                                                 \
	    break;                                                                                                                                             \
	}                                                                                                                                                      \
	if (_eValType < MT_SVR_PLAYER_VALUE_U8 || _eValType > MT_SVR_PLAYER_VALUE_S64P) {                                                                      \
	    _ret = MT_FAILURE;                                                                                                                                 \
	    break;                                                                                                                                             \
	}                                                                                                                                                      \
	switch (_eValType) {                                                                                                                                   \
	case MT_SVR_PLAYER_VALUE_U8:                                                                                                                           \
	    _s32ValLen = sizeof(MO_U8);                                                                                                                        \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_U8P:                                                                                                                          \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_CHAR:                                                                                                                         \
	    _unValue.cValue = *(MO_CHAR *)_pu8Value;                                                                                                           \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_CHARP:                                                                                                                        \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_S32:                                                                                                                          \
	    _unValue.s32Value = *(MO_S32 *)_pu8Value;                                                                                                          \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_S32P:                                                                                                                         \
	    _s32ValLen *= sizeof(MO_S32);                                                                                                                      \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_S64:                                                                                                                          \
	    _unValue.s64Value = *(MO_S64 *)_pu8Value;                                                                                                          \
	    break;                                                                                                                                             \
	case MT_SVR_PLAYER_VALUE_S64P:                                                                                                                         \
	    _s32ValLen *= sizeof(MO_S64);                                                                                                                      \
	    break;                                                                                                                                             \
	default:                                                                                                                                               \
	    META_PRINTF("Unkown type!\n");                                                                                                                     \
	    break;                                                                                                                                             \
	}                                                                                                                                                      \
	if (_pstMeta->u32KvpUsedNum >= _pstMeta->u32KvpTotalNum) {                                                                                             \
	    _pstMeta->pstKvp = (MT_SVR_PLAYERKVP_S *)realloc(_pstMeta->pstKvp, sizeof(MT_SVR_PLAYERKVP_S) * (_pstMeta->u32KvpTotalNum + MT_META_MAX_KEY_NUM)); \
	    if (NULL == _pstMeta->pstKvp) {                                                                                                                    \
		_ret = MT_FAILURE;                                                                                                                             \
		break;                                                                                                                                         \
	    }                                                                                                                                                  \
	    _pstMeta->u32KvpTotalNum = _pstMeta->u32KvpTotalNum + MT_META_MAX_KEY_NUM;                                                                         \
	}                                                                                                                                                      \
	if (_pstMeta->u32BuffPos + _s32ValLen + _s32KeyLen + 2 >= _pstMeta->u32BuffLen) {                                                                      \
	    _pu8Tmp = (MO_U8 *)malloc(_pstMeta->u32BuffLen + _s32ValLen + _s32KeyLen + 2 + MT_META_MAX_BUF_LEN);                                               \
	    if (NULL == _pu8Tmp) {                                                                                                                             \
		_ret = MT_FAILURE;                                                                                                                             \
		break;                                                                                                                                         \
	    }                                                                                                                                                  \
	    memset(_pu8Tmp, 0, _pstMeta->u32BuffLen + _s32ValLen + _s32KeyLen + 2 + MT_META_MAX_BUF_LEN);                                                      \
	    memcpy(_pu8Tmp, _pstMeta->pu8Buff, _pstMeta->u32BuffLen);                                                                                          \
	    for (i = 0; i < _pstMeta->u32KvpUsedNum; i++) {                                                                                                    \
		_pstMeta->pstKvp[i].pszKey = (MO_CHAR *)(_pu8Tmp + ((MO_U8 *)_pstMeta->pstKvp[i].pszKey - _pstMeta->pu8Buff));                                 \
		if (_pstMeta->pstKvp[i].eValueType >= MT_SVR_PLAYER_VALUE_U8P)                                                                                 \
		    _pstMeta->pstKvp[i].unValue.pu8Value = _pu8Tmp + (_pstMeta->pstKvp[i].unValue.pu8Value - _pstMeta->pu8Buff);                               \
	    }                                                                                                                                                  \
	    free(_pstMeta->pu8Buff);                                                                                                                           \
	    _pstMeta->pu8Buff = _pu8Tmp;                                                                                                                       \
	    _pstMeta->u32BuffLen = _pstMeta->u32BuffLen + _s32ValLen + _s32KeyLen + 2 + MT_META_MAX_BUF_LEN;                                                   \
	}                                                                                                                                                      \
	_pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].pszKey = (MO_CHAR *)memcpy(_pstMeta->pu8Buff + _pstMeta->u32BuffPos, _pszKey, _s32KeyLen);                   \
	_pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].pszKey[_s32KeyLen] = 0;                                                                                      \
	_pstMeta->u32BuffPos += _s32KeyLen + 1;                                                                                                                \
	if (_eValType < MT_SVR_PLAYER_VALUE_U8P) {                                                                                                             \
	    _pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].unValue = _unValue;                                                                                      \
	    _pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].u32ValueNum = _s32ValLen;                                                                                \
	    _pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].eValueType = _eValType;                                                                                  \
	    _pstMeta->u32KvpUsedNum++;                                                                                                                         \
	    break;                                                                                                                                             \
	}                                                                                                                                                      \
	_pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].unValue.pu8Value = (MO_U8 *)memcpy(_pstMeta->pu8Buff + _pstMeta->u32BuffPos, _pu8Value, _s32ValLen);         \
	_pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].unValue.pu8Value[_s32ValLen] = 0;                                                                            \
	_pstMeta->u32BuffPos += _s32ValLen + 1;                                                                                                                \
	_pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].u32ValueNum = _s32ValNum;                                                                                    \
	_pstMeta->pstKvp[_pstMeta->u32KvpUsedNum].eValueType = _eValType;                                                                                      \
	_pstMeta->u32KvpUsedNum++;                                                                                                                             \
    } while (0) /*lint -restore*/

/**
\brief Creates a metadata data structure.CNcomment:创建Metadata结构体CNend
\attention \n
None.
\param None
\retval Pointer to the ::MT_SVR_PLAYER_METADATA_S data structure. CNcomment: ::MT_SVR_PLAYER_METADATA_S结构体指针CNend
\see \n
None.
*/

static inline MT_SVR_PLAYER_METADATA_S *MT_SVR_META_Create()
{
    MT_SVR_PLAYER_METADATA_S *pstMetadata = NULL;

    pstMetadata = (MT_SVR_PLAYER_METADATA_S *)malloc(sizeof(MT_SVR_PLAYER_METADATA_S));

    if (NULL != pstMetadata) {
	MT_SVR_META_INIT(pstMetadata);
    }

    return pstMetadata;
}

/**
\brief Releases the metadata data structure. CNcomment:释放Metadata结构体CNend
\attention \n
None.
\param None
\retval None.
\see \n
None.
*/

static inline MO_VOID MT_SVR_META_Free(MT_SVR_PLAYER_METADATA_S **ppstMetadata)
{
    MT_SVR_PLAYER_METADATA_S *pstMetadata = NULL;

    if (NULL == ppstMetadata)
	return;

    pstMetadata = *ppstMetadata;

    MT_SVR_META_CLEAR(pstMetadata);

    free(*ppstMetadata);
    *ppstMetadata = NULL;
}

    /** @} */ /** <!-- ==== Structure Definition end ==== */

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __SVR_METADATA_H__ */
