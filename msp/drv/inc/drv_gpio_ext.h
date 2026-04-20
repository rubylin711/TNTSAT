/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _DRV_GPIO_EXT_H
#define _DRV_GPIO_EXT_H

#include "mt_type.h"
#ifdef __KERNEL__
#include "mt_drv_dev.h"
#endif
//#include "mt_drv_gpio.h"
//#include "mt_unf_gpio.h"

mt_s32 GPIO_DRV_ModInit(mt_void);
mt_void GPIO_DRV_ModExit(mt_void);

typedef mt_s32 (*FN_GPIO_Get_Bit)(mt_u32, mt_u32*);
typedef mt_s32 (*FN_GPIO_Set_Bit)(mt_u32, mt_u32);
typedef mt_s32 (*FN_GPIO_Get_Num)(GPIO_GET_GPIONUM_S*);

typedef mt_s32 (*FN_GPIO_Register_Server_Func)(mt_u32, mt_void (*func)(mt_u32));
typedef mt_s32 (*FN_GPIO_UnRegister_Server_Func)(mt_u32);
typedef mt_s32 (*FN_GPIO_Set_Int_Type)(mt_u32, MT_UNF_GPIO_INTTYPE_E);
typedef mt_s32 (*FN_GPIO_Set_Int_Enable)(mt_u32, MT_BOOL);
typedef mt_s32 (*FN_GPIO_Clear_GroupInt)(mt_u32);
typedef mt_s32 (*FN_GPIO_Clear_BitInt)(mt_u32);
#ifdef __KERNEL__
typedef mt_s32 (*FN_GPIO_Suspend)(basedev_s* , pm_message_t );
typedef mt_s32 (*FN_GPIO_Resume)(basedev_s* );
#endif

typedef struct
{
    FN_GPIO_Get_Bit                pfnGpioDirGetBit;
    FN_GPIO_Set_Bit                pfnGpioDirSetBit;
    FN_GPIO_Get_Bit                pfnGpioReadBit;
    FN_GPIO_Set_Bit                pfnGpioWriteBit;
    FN_GPIO_Get_Num                pfnGpioGetNum;
    FN_GPIO_Register_Server_Func   pfnGpioRegisterServerFunc;
    FN_GPIO_UnRegister_Server_Func pfnGpioUnRegisterServerFunc;
    FN_GPIO_Set_Int_Type           pfnGpioSetIntType;
    FN_GPIO_Set_Int_Enable         pfnGpioSetIntEnable;
    FN_GPIO_Clear_GroupInt         pfnGpioClearGroupInt;
    FN_GPIO_Clear_BitInt           pfnGpioClearBitInt;
#ifdef __KERNEL__
    FN_GPIO_Suspend				   pfnGpioSuspend;
    FN_GPIO_Resume				   pfnGpioResume;
#endif

} GPIO_EXT_FUNC_S;

#endif
