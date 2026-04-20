/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __GPIO_ARIA_H__
#define __GPIO_ARIA_H__
#include <linux/types.h>
//#include <../../arch/arm/mach-aria/aria_reg_base_addr.h>
#include "mt_type.h"
#include "mt_module.h"

//#define u8 unsigned char
//#define u32 unsigned int

typedef enum gpio_list
{
	/* All Chips */
    GPIO_0 = 0,
    GPIO_1, GPIO_2, GPIO_3,
    GPIO_4, GPIO_5, GPIO_6, GPIO_7,
    GPIO_8, GPIO_9, GPIO_10, GPIO_11,
    GPIO_12, GPIO_13, GPIO_14, GPIO_15,
    GPIO_16, GPIO_17, GPIO_18, GPIO_19,
    GPIO_20, GPIO_21, GPIO_22, GPIO_23,
    GPIO_24, GPIO_25, GPIO_26, GPIO_27,
    GPIO_28, GPIO_29, GPIO_30, GPIO_31,
    GPIO_32, GPIO_33, GPIO_34, GPIO_35,
    GPIO_36, GPIO_37, GPIO_38, GPIO_39,
    GPIO_40, GPIO_41, GPIO_42, GPIO_43,
    GPIO_44, GPIO_45, GPIO_46, GPIO_47,
    GPIO_48, GPIO_49, GPIO_50, GPIO_51,
    GPIO_52, GPIO_53, GPIO_54, GPIO_55,
    GPIO_56, GPIO_57, GPIO_58, GPIO_59,
    GPIO_60, GPIO_61, GPIO_62, GPIO_63,

	/* Aria & Symphony4 */
    GPIO_64, GPIO_65, GPIO_66, GPIO_67,
    GPIO_68, GPIO_69, GPIO_70, GPIO_71,
    GPIO_72, GPIO_73, GPIO_74, GPIO_75,
    GPIO_76, GPIO_77, GPIO_78, GPIO_79,
    GPIO_80, GPIO_81, GPIO_82, GPIO_83,
    GPIO_84, GPIO_85, GPIO_86, GPIO_87,
    GPIO_88, GPIO_89, GPIO_90, GPIO_91,
    GPIO_92, GPIO_93, GPIO_94, GPIO_95,

	/* Symphony4 */
    GPIO_96, GPIO_97, GPIO_98, GPIO_99,
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	 GPIO_100, GPIO_101, GPIO_102, GPIO_103,
	 GPIO_104, GPIO_105, GPIO_106, GPIO_107,
	 GPIO_108, GPIO_109, GPIO_110, GPIO_111,
	 GPIO_112, GPIO_113, GPIO_114, GPIO_115,
	 GPIO_116, GPIO_117, GPIO_118, GPIO_119,
	 GPIO_120, GPIO_121, GPIO_122, GPIO_123,
#endif

	/* Symphony 1/2/4 */
	AO_GPIO_0,
	AO_GPIO_1, AO_GPIO_2, AO_GPIO_3,
	AO_GPIO_4, AO_GPIO_5, AO_GPIO_6, AO_GPIO_7,
	AO_GPIO_8,
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	AO_GPIO_9, AO_GPIO_10, AO_GPIO_11, AO_GPIO_12,
	AO_GPIO_13,
#endif
	AO_GPIO_MAX,

	/* Symphony 4/6 */
	GPIO_IN_0,
	GPIO_IN_1, GPIO_IN_2, GPIO_IN_3,
	GPIO_IN_4, GPIO_IN_5, GPIO_IN_MAX,

    GPIO_UNKNOWN
} gpio_list_e;

typedef enum gpio_dir
{
    GPIO_DIR_OUTPUT = 0,
    GPIO_DIR_INPUT,
    GPIO_DIR_UNKNOWN
} gpio_dir_e;

typedef enum gpio_value
{
    GPIO_VALUE_LOW_LEVEL = 0,
    GPIO_VALUE_HIGH_LEVEL,
    GPIO_VALUE_UNKNOWN
} gpio_value_e;

typedef enum gpio_mask
{
    GPIO_MASK_UNENABLE = 0,
	GPIO_MASK_DISABLE = 0,
    GPIO_MASK_ENABLE,
    GPIO_MASK_UNKNOWN
} gpio_mask_e;


typedef struct gpio_dir_param
{
    u8 gpio;
    gpio_dir_e dir;
} gpio_dir_param_s;

typedef struct gpio_value_param
{
    u8 gpio;
    gpio_value_e val;
} gpio_value_param_s;

typedef struct gpio_mask_param
{
    u8 gpio;
    gpio_mask_e val;
} gpio_mask_param_s;

#define GPIO_DRV_IOC_SET_DIR     _IOW(MT_ID_GPIO, 0, struct gpio_dir_param)
#define GPIO_DRV_IOC_GET_DIR     _IOWR(MT_ID_GPIO, 1, struct gpio_dir_param)
#define GPIO_DRV_IOC_SET_VALUE _IOW(MT_ID_GPIO, 2, struct gpio_value_param)
#define GPIO_DRV_IOC_GET_VALUE _IOWR(MT_ID_GPIO, 3, struct gpio_value_param)
#define GPIO_DRV_IOC_IO_ENABLE  _IOWR(MT_ID_GPIO, 4, struct gpio_mask_param)

void drv_gpio_io_enable(u8 gpio, gpio_mask_e enable);
void drv_gpio_set_dir(u8 gpio, gpio_dir_e dir);
void drv_gpio_get_dir(u8 gpio, gpio_dir_e *dir);
void drv_gpio_set_value(u8 gpio, gpio_value_e val);
void drv_gpio_get_value(u8 gpio, gpio_value_e *val);

#endif

