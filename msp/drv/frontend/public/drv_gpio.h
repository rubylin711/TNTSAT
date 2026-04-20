/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_GPIO_H__
#define __DRV_GPIO_H__
#include <linux/types.h>

#define R_GPIO_BASE_ADDR 0xBF0A0000
#define R_GPIO0_WDATA R_GPIO_BASE_ADDR
#define R_GPIO0_WR_N (R_GPIO_BASE_ADDR + 0x4)
#define R_GPIO0_RDATA (R_GPIO_BASE_ADDR + 0x8)
#define R_GPIO0_MASK_N (R_GPIO_BASE_ADDR + 0xc)
#define R_GPIO1_WDATA (R_GPIO_BASE_ADDR + 0x10)
#define R_GPIO1_WR_N (R_GPIO_BASE_ADDR + 0x14)
#define R_GPIO1_RDATA (R_GPIO_BASE_ADDR + 0x18)
#define R_GPIO1_MASK_N (R_GPIO_BASE_ADDR + 0x1c)
#define R_AO_GPIO_WDATA (R_GPIO_BASE_ADDR + 0x20)
#define R_AO_GPIO_WR_N (R_GPIO_BASE_ADDR + 0x24)
#define R_AO_GPIO_RDATA (R_GPIO_BASE_ADDR + 0x28)
#define R_AO_GPIO_MASK_N (R_GPIO_BASE_ADDR + 0x2c)
#define R_GPIO0_MODE (R_GPIO_BASE_ADDR + 0x100)
#define R_GPIO0_STA_CLR (R_GPIO_BASE_ADDR + 0x104)
#define R_GPIO0_STA_CLR_R (R_GPIO_BASE_ADDR + 0x108)
#define R_GPIO0_STATE (R_GPIO_BASE_ADDR + 0x10c)
#define R_GPIO1_MODE (R_GPIO_BASE_ADDR + 0x110)
#define R_GPIO1_STA_CLR (R_GPIO_BASE_ADDR + 0x114)
#define R_GPIO1_STA_CLR_R (R_GPIO_BASE_ADDR + 0x118)
#define R_GPIO1_STATE (R_GPIO_BASE_ADDR + 0x11c)
#define R_AO_GPIO_MODE (R_GPIO_BASE_ADDR + 0x120)
#define R_AO_GPIO_STA_CLR (R_GPIO_BASE_ADDR + 0x124)
#define R_AO_GPIO_STA_CLR_R (R_GPIO_BASE_ADDR + 0x128)
#define R_AO_GPIO_STATE (R_GPIO_BASE_ADDR + 0x12c)
#define R_GPIO_INT0_SEL (R_GPIO_BASE_ADDR + 0x130)
#define R_GPIO_INT1_SEL (R_GPIO_BASE_ADDR + 0x134)

typedef enum gpio_list {
    GPIO_0 = 0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    GPIO_15,
    GPIO_16,
    GPIO_17,
    GPIO_18,
    GPIO_19,
    GPIO_20,
    GPIO_21,
    GPIO_22,
    GPIO_23,
    GPIO_24,
    GPIO_25,
    GPIO_26,
    GPIO_27,
    GPIO_28,
    GPIO_29,
    GPIO_30,
    GPIO_31,
    GPIO_32,
    GPIO_33,
    GPIO_34,
    GPIO_35,
    GPIO_36,
    GPIO_37,
    GPIO_38,
    GPIO_39,
    GPIO_40,
    GPIO_41,
    GPIO_42,
    GPIO_43,
    GPIO_44,
    GPIO_45,
    GPIO_46,
    GPIO_47,
    GPIO_48,
    GPIO_49,
    GPIO_50,
    GPIO_51,
    GPIO_52,
    GPIO_53,
    GPIO_54,
    GPIO_55,
    GPIO_56,
    GPIO_57,
    GPIO_58,
    GPIO_59,
    GPIO_60,
    GPIO_61,
    GPIO_62,
    GPIO_63,
    AO_GPIO_0,
    AO_GPIO_1,
    AO_GPIO_2,
    AO_GPIO_3,
    AO_GPIO_4,
    AO_GPIO_5,
    AO_GPIO_6,
    AO_GPIO_7,
    GPIO_UNKNOWN
} gpio_list_e;

typedef enum gpio_dir {
    GPIO_DIR_OUTPUT = 0,
    GPIO_DIR_INPUT,
    GPIO_DIR_UNKNOWN
} gpio_dir_e;

typedef enum gpio_value {
    GPIO_VALUE_LOW_LEVEL = 0,
    GPIO_VALUE_HIGH_LEVEL,
    GPIO_VALUE_UNKNOWN
} gpio_value_e;

void gpio_set_dir(unsigned char gpio, gpio_dir_e dir);
void gpio_get_dir(unsigned char gpio, gpio_dir_e *dir);
void gpio_set_value(unsigned char gpio, gpio_value_e val);
gpio_value_e gpio_get_value(unsigned char gpio);

#endif
