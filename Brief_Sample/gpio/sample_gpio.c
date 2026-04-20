/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt_unf_gpio.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_GPIO_DEBUG

#define MT_GPIO_PRINT   printf
#else
#define MT_GPIO_PRINT

#endif


#define SAMPLE_GPIO_FUNCTION_ENTER()    MT_GPIO_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_GPIO_FUNCTION_EXIT()     MT_GPIO_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_GPIO_FATAL_PRINT(fmt...)         MT_GPIO_PRINT(" [FATAL] " fmt)
#define SAMPLE_GPIO_ERR_PRINT(fmt...)           MT_GPIO_PRINT(" [ERROR] " fmt)
#define SAMPLE_GPIO_WARN_PRINT(fmt...)          MT_GPIO_PRINT(" [WARN] "  fmt)
#define SAMPLE_GPIO_INFO_PRINT(fmt...)          MT_GPIO_PRINT(" [INFO] "  fmt)
#define SAMPLE_GPIO_DBG_PRINT(fmt...)           MT_GPIO_PRINT(" [DEBUG] " fmt)

#define STR2ENUM_CONV(_enum, ENUM)                                             \
    static int str2##_enum(char *str)                                          \
    {                                                                          \
        int i = 0;                                                             \
        if (!str) {                                                            \
            printf("%s@%d -> wrong parameter\n", __FUNCTION__, __LINE__);      \
            return -1;                                                         \
        }                                                                      \
        while (i < NUM_OF_##ENUM) {                                            \
            if (strcmp(str, _enum##_str[i]) == 0)                              \
                break;                                                         \
            i++;                                                               \
        }                                                                      \
        if (i == NUM_OF_##ENUM) {                                              \
            SAMPLE_GPIO_ERR_PRINT("wrong parameter %s\n", str);                \
            return MT_FAILURE;                                                 \
        }                                                                      \
        return i;                                                              \
    }

#define NUM_OF_GPIO_TYPE 146
#define NUM_OF_GPIO_VALUE 2

/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/

char *gpio_type_str[NUM_OF_GPIO_TYPE] = {
    /* All Chips */
    "gpio_0",  "gpio_1",  "gpio_2",  "gpio_3",  "gpio_4",  "gpio_5",  "gpio_6",  "gpio_7",
    "gpio_8",  "gpio_9",  "gpio_10", "gpio_11", "gpio_12", "gpio_13", "gpio_14", "gpio_15",
    "gpio_16", "gpio_17", "gpio_18", "gpio_19", "gpio_20", "gpio_21", "gpio_22", "gpio_23",
    "gpio_24", "gpio_25", "gpio_26", "gpio_27", "gpio_28", "gpio_29", "gpio_30", "gpio_31",
    "gpio_32", "gpio_33", "gpio_34", "gpio_35", "gpio_36", "gpio_37", "gpio_38", "gpio_39",
    "gpio_40", "gpio_41", "gpio_42", "gpio_43", "gpio_44", "gpio_45", "gpio_46", "gpio_47",
    "gpio_48", "gpio_49", "gpio_50", "gpio_51", "gpio_52", "gpio_53", "gpio_54", "gpio_55",
    "gpio_56", "gpio_57", "gpio_58", "gpio_59", "gpio_60", "gpio_61", "gpio_62", "gpio_63",

    "gpio_64", "gpio_65", "gpio_66", "gpio_67", "gpio_68", "gpio_69", "gpio_70", "gpio_71",
    "gpio_72", "gpio_73", "gpio_74", "gpio_75", "gpio_76", "gpio_77", "gpio_78", "gpio_79",
    "gpio_80", "gpio_81", "gpio_82", "gpio_83", "gpio_84", "gpio_85", "gpio_86", "gpio_87",
    "gpio_88", "gpio_89", "gpio_90", "gpio_91", "gpio_92", "gpio_93", "gpio_94", "gpio_95",

    "gpio_96", "gpio_97", "gpio_98", "gpio_99", "gpio_100", "gpio_101", "gpio_102", "gpio_103",
    "gpio_104", "gpio_105", "gpio_106", "gpio_107", "gpio_108", "gpio_109", "gpio_110", "gpio_111",
    "gpio_112", "gpio_113", "gpio_114", "gpio_115", "gpio_116", "gpio_117", "gpio_118", "gpio_119",
    "gpio_120", "gpio_121", "gpio_122", "gpio_123",

    "ao_gpio_0", "ao_gpio_1", "ao_gpio_2", "ao_gpio_3", "ao_gpio_4", "ao_gpio_5", "ao_gpio_6", "ao_gpio_7",
    "ao_gpio_8", "ao_gpio_9", "ao_gpio_10", "ao_gpio_11", "ao_gpio_12", "ao_gpio_13", "ao_gpio_max",

    "gpio_in_0", "gpio_in_1", "gpio_in_2", "gpio_in_3", "gpio_in_4", "gpio_in_5", "gpio_in_max",
};

STR2ENUM_CONV(gpio_type, GPIO_TYPE)

char *gpio_value_str[NUM_OF_GPIO_VALUE] = {
    "down", "up"
};

STR2ENUM_CONV(gpio_value, GPIO_VALUE)

/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_GpioMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

/*!
@brief Check if the GPIO is being multiplexed
@param[in] u32RegAddr           Register address
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_GpioModeCheckPinmux(MT_U32 u32RegAddr)
{
    MT_U32 reg_value = 0;
    MT_S32 Ret = MT_FAILURE;

    Ret = mt_sys_read_register(u32RegAddr, &reg_value);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("mt_sys_read_register failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        return Ret;
    }
    SAMPLE_GPIO_INFO_PRINT("0x%x :  0x%x \n", u32RegAddr, reg_value);

    if((reg_value & 0x6) != 0)
    {
        reg_value |= 0x7;
        reg_value ^= 0x6;
        SAMPLE_GPIO_INFO_PRINT("set 0x%x:  0x%x \n", u32RegAddr, reg_value);

        Ret = mt_sys_write_register(u32RegAddr, reg_value);
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_GPIO_ERR_PRINT("mt_sys_write_register failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
            return Ret;
        }

        Ret = mt_sys_read_register(u32RegAddr, &reg_value);
        if (MT_SUCCESS != Ret)
        {
            SAMPLE_GPIO_ERR_PRINT("mt_sys_read_register failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
            return Ret;
        }
        SAMPLE_GPIO_INFO_PRINT("0x%x :  0x%x \n", u32RegAddr, reg_value);
    }

    return MT_SUCCESS;
}


/*!
@brief Set the parameters of the GPIO
@param[in] u32GpioNo            GPIO number, Pin0 is the control green light, Pin is the control red light
@param[in] u32Value             Pull the level high or low, pull low to light up, pull high to extinguish
@return::MT_SUCCESS             Success.
@return::Ret                    The return value of the error.
@*/
static MT_S32 MT_GpioModeSetPara(MT_U32 u32GpioNo, MT_U32 u32Value)
{

    MT_S32 Ret = MT_FAILURE;

    Ret = MT_UNF_GPIO_SetDirBit(u32GpioNo, (MT_BOOL)0);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("MT_UNF_GPIO_SetDirBit failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        return Ret;
    }

    Ret = MT_UNF_GPIO_WriteBit(u32GpioNo, (MT_BOOL)u32Value);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("MT_UNF_GPIO_WriteBit failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        return Ret;
    }

    return MT_SUCCESS;

}


/*!
@brief Help information
@param[in]  name     Enter the value
@return::MT_VOID
@*/
static MT_VOID MT_GpioModePrint_Help(MT_CHAR *name)
{
    MT_GPIO_PRINT("Lack of parameters\n");
    MT_GPIO_PRINT("\nUsage:\n");
    MT_GPIO_PRINT("%s\n", name);
    MT_GPIO_PRINT("example:\n");
    MT_GPIO_PRINT("    %s Pin0 down\n", name);
}


/*!
@brief gets the external input parameters
@param[in]  argc                The number of external input parameters
@param[in]  argv                External input parameter values
@param[in]  u32TestGpioNum      GPIO number, Pin0 is the control green light, Pin is the control red light
@param[in]  u32Value            Turn the LED light on or off
@return::MT_VOID
@*/
static MT_S32 MT_GpioModeParase_args(MT_S32 argc, MT_CHAR *argv[], MT_U32 *u32TestGpioNum, MT_U32 *u32Value)
{
    if (argc < 3)
    {
        (MT_VOID)MT_GpioModePrint_Help(argv[0]);
        return MT_FAILURE;
    }

    *u32TestGpioNum = str2gpio_type((char *)argv[1]);
    *u32Value = str2gpio_value((char *)argv[2]);

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_GpioMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32 Ret = MT_FAILURE;
    MT_U32 u32TestGpioNum = 0;
    MT_U32 u32Value = 0;

    Ret = MT_GpioModeParase_args(argc, argv, &u32TestGpioNum, &u32Value);
    if(MT_SUCCESS != Ret)
    {
        return MT_FAILURE;
    }

#ifndef MT_SAMPLE_APP
    Ret = mt_sys_init();
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("mt_sys_init failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        return Ret;
    }
#endif

    Ret = MT_GpioModeCheckPinmux(0xbf15b400);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("The pin(0xbf15b400) is multiplexed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        goto ERR0;
    }

    Ret = MT_GpioModeCheckPinmux(0xbf15b404);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("The pin(0xbf15b404) is multiplexed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        goto ERR0;
    }

    Ret = MT_UNF_GPIO_Init();
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("MT_UNF_GPIO_Init failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        goto ERR0;
    }

    Ret = MT_GpioModeSetPara(u32TestGpioNum, u32Value);
    if (MT_SUCCESS != Ret)
    {
        SAMPLE_GPIO_ERR_PRINT("MT_GpioModeSetPara failed, ret = %d-------<%s> line: %d\n", Ret,  __FUNCTION__, __LINE__);
        goto ERR1;
    }

ERR1:
    (MT_VOID)MT_UNF_GPIO_Deinit();

ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return Ret;
}

