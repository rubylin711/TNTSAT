/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_gpio.h"
#include <linux/types.h>
#include <linux/io.h>

void gpio_set_dir(u_char gpio, gpio_dir_e dir)
{
    u_long reg32 = 0;

    if (gpio >= GPIO_0 && gpio <= GPIO_31) {
	reg32 = readl((volatile void *)R_GPIO0_WR_N);
	if (dir == GPIO_DIR_OUTPUT) {
	    reg32 &= ~(1 << gpio);
	} else {
	    reg32 |= (1 << gpio);
	}
	writel(reg32, (volatile void *)R_GPIO0_WR_N);
    } else if (gpio > GPIO_31 && gpio <= GPIO_63) {
	reg32 = readl((volatile void *)R_GPIO1_WR_N);
	if (dir == GPIO_DIR_OUTPUT) {
	    reg32 &= ~(1 << (gpio - GPIO_32));
	} else {
	    reg32 |= (1 << (gpio - GPIO_32));
	}
	writel(reg32, (volatile void *)R_GPIO1_WR_N);
    } else if (gpio >= AO_GPIO_0 && gpio <= AO_GPIO_7) {
	reg32 = readl((volatile void *)R_AO_GPIO_WR_N);
	if (dir == GPIO_DIR_OUTPUT) {
	    reg32 &= ~(1 << (gpio - AO_GPIO_0));
	} else {
	    reg32 |= (1 << (gpio - AO_GPIO_0));
	}
	writel(reg32, (volatile void *)R_AO_GPIO_WR_N);
    }
}

void gpio_get_dir(u_char gpio, gpio_dir_e *dir)
{
    u_long reg32 = 0;
    u_long dirbit = 0;
    if (gpio >= GPIO_0 && gpio <= GPIO_31) {
	reg32 = readl((volatile void *)R_GPIO0_WR_N);
	dirbit = (reg32 >> gpio) & 0x1;
    } else if (gpio > GPIO_31 && gpio <= GPIO_63) {
	reg32 = readl((volatile void *)R_GPIO1_WR_N);
	dirbit = (reg32 >> (gpio - GPIO_32)) & 0x1;
    } else if (gpio >= AO_GPIO_0 && gpio <= AO_GPIO_7) {
	reg32 = readl((volatile void *)R_AO_GPIO_WR_N);
	dirbit = (reg32 >> (gpio - AO_GPIO_0)) & 0x1;
    }
    if (dirbit == 0) {
	*dir = GPIO_DIR_OUTPUT;
    } else {
	*dir = GPIO_DIR_INPUT;
    }
}

void gpio_set_value(u_char gpio, gpio_value_e val)
{
    u_long reg32 = 0;

    if (gpio >= GPIO_0 && gpio <= GPIO_31) {
	reg32 = readl((volatile void *)R_GPIO0_WDATA);
	if (val == GPIO_VALUE_HIGH_LEVEL) {
	    reg32 |= (1 << gpio);
	} else {
	    reg32 &= ~(1 << gpio);
	}
	writel(reg32, (volatile void *)R_GPIO0_WDATA);
    } else if (gpio > GPIO_31 && gpio <= GPIO_63) {
	reg32 = readl((volatile void *)R_GPIO1_WDATA);
	if (val == GPIO_VALUE_HIGH_LEVEL) {
	    reg32 |= (1 << (gpio - GPIO_32));
	} else {
	    reg32 &= ~(1 << (gpio - GPIO_32));
	}
	writel(reg32, (volatile void *)R_GPIO1_WDATA);
    } else if (gpio >= AO_GPIO_0 && gpio <= AO_GPIO_7) {
	reg32 = readl((volatile void *)R_AO_GPIO_WDATA);
	if (val == GPIO_VALUE_HIGH_LEVEL) {
	    reg32 |= (1 << (gpio - AO_GPIO_0));
	} else {
	    reg32 &= ~(1 << (gpio - AO_GPIO_0));
	}
	writel(reg32, (volatile void *)R_AO_GPIO_WDATA);
    }
}

gpio_value_e gpio_get_value(u_char gpio)
{
    u_long reg32 = 0;
    u_char valbit = 0;
    if (gpio >= GPIO_0 && gpio <= GPIO_31) {
	reg32 = readl((volatile void *)R_GPIO0_RDATA);
	valbit = (reg32 >> gpio) & 0x1;
    } else if (gpio > GPIO_31 && gpio <= GPIO_63) {
	reg32 = readl((volatile void *)R_GPIO1_RDATA);
	valbit = (reg32 >> (gpio - GPIO_32)) & 0x1;
    } else if (gpio >= AO_GPIO_0 && gpio <= AO_GPIO_7) {
	reg32 = readl((volatile void *)R_AO_GPIO_RDATA);
	valbit = (reg32 >> (gpio - AO_GPIO_0)) & 0x1;
    }

    if (valbit == 1) {
	return GPIO_VALUE_HIGH_LEVEL;
    } else {
	return GPIO_VALUE_LOW_LEVEL;
    }
}
