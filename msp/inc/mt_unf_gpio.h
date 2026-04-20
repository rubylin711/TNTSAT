/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/*!
@file mt_unf_gpio.h
@brief the header file of GPIO
*/
#ifndef __MT_UNF_GPIO_H__
#define __MT_UNF_GPIO_H__

#include "mt_common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/*!
@brief GPIO Pin definition
*/
typedef enum mtUNF_GPIO_LIST_E
{
	/* All Chips */
    MT_UNF_GPIO_0 = 0,
    MT_UNF_GPIO_1, MT_UNF_GPIO_2, MT_UNF_GPIO_3,
    MT_UNF_GPIO_4, MT_UNF_GPIO_5, MT_UNF_GPIO_6, MT_UNF_GPIO_7,
    MT_UNF_GPIO_8, MT_UNF_GPIO_9, MT_UNF_GPIO_10, MT_UNF_GPIO_11,
    MT_UNF_GPIO_12, MT_UNF_GPIO_13, MT_UNF_GPIO_14, MT_UNF_GPIO_15,
    MT_UNF_GPIO_16, MT_UNF_GPIO_17, MT_UNF_GPIO_18, MT_UNF_GPIO_19,
    MT_UNF_GPIO_20, MT_UNF_GPIO_21, MT_UNF_GPIO_22, MT_UNF_GPIO_23,
    MT_UNF_GPIO_24, MT_UNF_GPIO_25, MT_UNF_GPIO_26, MT_UNF_GPIO_27,
    MT_UNF_GPIO_28, MT_UNF_GPIO_29, MT_UNF_GPIO_30, MT_UNF_GPIO_31,
    MT_UNF_GPIO_32, MT_UNF_GPIO_33, MT_UNF_GPIO_34, MT_UNF_GPIO_35,
    MT_UNF_GPIO_36, MT_UNF_GPIO_37, MT_UNF_GPIO_38, MT_UNF_GPIO_39,
    MT_UNF_GPIO_40, MT_UNF_GPIO_41, MT_UNF_GPIO_42, MT_UNF_GPIO_43,
    MT_UNF_GPIO_44, MT_UNF_GPIO_45, MT_UNF_GPIO_46, MT_UNF_GPIO_47,
    MT_UNF_GPIO_48, MT_UNF_GPIO_49, MT_UNF_GPIO_50, MT_UNF_GPIO_51,
    MT_UNF_GPIO_52, MT_UNF_GPIO_53, MT_UNF_GPIO_54, MT_UNF_GPIO_55,
    MT_UNF_GPIO_56, MT_UNF_GPIO_57, MT_UNF_GPIO_58, MT_UNF_GPIO_59,
    MT_UNF_GPIO_60, MT_UNF_GPIO_61, MT_UNF_GPIO_62, MT_UNF_GPIO_63,

	/* Aria & Symphony4 */
    MT_UNF_GPIO_64, MT_UNF_GPIO_65, MT_UNF_GPIO_66, MT_UNF_GPIO_67,
    MT_UNF_GPIO_68, MT_UNF_GPIO_69, MT_UNF_GPIO_70, MT_UNF_GPIO_71,
    MT_UNF_GPIO_72, MT_UNF_GPIO_73, MT_UNF_GPIO_74, MT_UNF_GPIO_75,
    MT_UNF_GPIO_76, MT_UNF_GPIO_77, MT_UNF_GPIO_78, MT_UNF_GPIO_79,
    MT_UNF_GPIO_80, MT_UNF_GPIO_81, MT_UNF_GPIO_82, MT_UNF_GPIO_83,
    MT_UNF_GPIO_84, MT_UNF_GPIO_85, MT_UNF_GPIO_86, MT_UNF_GPIO_87,
    MT_UNF_GPIO_88, MT_UNF_GPIO_89, MT_UNF_GPIO_90, MT_UNF_GPIO_91,
    MT_UNF_GPIO_92, MT_UNF_GPIO_93, MT_UNF_GPIO_94, MT_UNF_GPIO_95,

	/* Symphony4 */
    MT_UNF_GPIO_96, MT_UNF_GPIO_97, MT_UNF_GPIO_98, MT_UNF_GPIO_99,
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	MT_UNF_GPIO_100, MT_UNF_GPIO_101, MT_UNF_GPIO_102, MT_UNF_GPIO_103,
	MT_UNF_GPIO_104, MT_UNF_GPIO_105, MT_UNF_GPIO_106, MT_UNF_GPIO_107,
	MT_UNF_GPIO_108, MT_UNF_GPIO_109, MT_UNF_GPIO_110, MT_UNF_GPIO_111,
	MT_UNF_GPIO_112, MT_UNF_GPIO_113, MT_UNF_GPIO_114, MT_UNF_GPIO_115,
	MT_UNF_GPIO_116, MT_UNF_GPIO_117, MT_UNF_GPIO_118, MT_UNF_GPIO_119,
	MT_UNF_GPIO_120, MT_UNF_GPIO_121, MT_UNF_GPIO_122, MT_UNF_GPIO_123,
#endif

	/* Symphony 1/2/4 */
    MT_UNF_AO_GPIO_0,	/*100(sym4) or 124(sym6)*/
    MT_UNF_AO_GPIO_1, MT_UNF_AO_GPIO_2, MT_UNF_AO_GPIO_3,
    MT_UNF_AO_GPIO_4, MT_UNF_AO_GPIO_5, MT_UNF_AO_GPIO_6, MT_UNF_AO_GPIO_7,
    MT_UNF_AO_GPIO_8,
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	MT_UNF_AO_GPIO_9, MT_UNF_AO_GPIO_10, MT_UNF_AO_GPIO_11, MT_UNF_AO_GPIO_12,
	MT_UNF_AO_GPIO_13,
#endif
	MT_UNF_AO_GPIO_MAX,

	/* Symphony4 */
	MT_UNF_GPIO_IN_0,	/*110(sym4) or139 (sym6) */
	MT_UNF_GPIO_IN_1, MT_UNF_GPIO_IN_2, MT_UNF_GPIO_IN_3,
	MT_UNF_GPIO_IN_4, MT_UNF_GPIO_IN_5, MT_UNF_GPIO_IN_MAX,

    MT_UNF_GPIO_UNKNOWN

} MT_UNF_GPIO_LIST_E;


/*!
@brief Open gpio device
@Return				| Description
--------------------|--------------
MT_SUCCESS			| Open success
MT_FAILURE			| Open failed
*/
MT_S32 MT_UNF_GPIO_Init(MT_VOID);

/*!
@brief Close GPIO device
@Return				| Description
--------------------|--------------
MT_SUCCESS			| Close success
MT_FAILURE			| Close failed
*/
MT_S32 MT_UNF_GPIO_Deinit(MT_VOID);

/*!
@brief Get GPIO level

@param [in] u32GpioNo GPIO number
@param [out] pbHighVolt Level
Value				| Description
--------------------|--------------
MT_TRUE				| High level
MT_FALSE			| Low level
@Return				| Description
--------------------|--------------
MT_SUCCESS			| Read success
MT_FAILURE			| Read failed
*/
MT_S32 MT_UNF_GPIO_ReadBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL *pbHighVolt);

/*!
@brief Set GPIO level

@param [in] u32GpioNo GPIO number
@param [in] bHighVolt Level
Value				| Description
--------------------|--------------
MT_TRUE				| High level
MT_FALSE			| Low level
@Return				| Description
--------------------|--------------
MT_SUCCESS			| Set success
MT_FAILURE			| Set failed
*/
MT_S32 MT_UNF_GPIO_WriteBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL bHighVolt);

/*!
@brief Set the direction of the GPIO input and output

@param [in] u32GpioNo GPIO number
@param [in] bInput the direction of the GPIO input and output
Value				| Description
--------------------|--------------
MT_TRUE				| input
MT_FALSE			| output
@Return				| Description
--------------------|--------------
MT_SUCCESS			| Set success
MT_FAILURE			| Set failed
*/
MT_S32 MT_UNF_GPIO_SetDirBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL bInput);

/*!
@brief Get the direction of the GPIO input and output

@param [in] u32GpioNo GPIO number
@param [out] pbInput the direction of the GPIO input and output
Value				| Description
--------------------|--------------
MT_TRUE				| input
MT_FALSE			| output
@Return				| Description
--------------------|--------------
MT_SUCCESS			| Get success
MT_FAILURE			| Get failed
*/
MT_S32 MT_UNF_GPIO_GetDirBit(MT_UNF_GPIO_LIST_E u32GpioNo, MT_BOOL *pbInput);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

