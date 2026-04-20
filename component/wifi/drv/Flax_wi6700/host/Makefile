include ${SDK_DIR}/build/script/base.mk

SHELL := /bin/bash

__REV=$(shell svn info --xml | sed -n '/commit/,/\/commit/s/ *revision=\"\([0-9]*\)\">/\1/p')

include $(PWD)/Kconfig.rule

ifneq ($(origin KLIB), undefined)
KMODPATH_ARG := "INSTALL_MOD_PATH=$(KLIB)"
else
KLIB := /lib/modules/$(shell uname -r)/
KMODPATH_ARG :=
endif
KLIB_BUILD ?= $(KLIB)/build/

MY=$(PWD)

ccflags-y += -I$(src)/include/

OS_DIR = os_dep/linux
CORE_DIR = core

ifndef BACKPORT_PWD
obj-$(CONFIG_LYNX_W2USB) += m88wi6700u.o
obj-$(CONFIG_LYNX_W2SDIO) += m88wi6700s.o
obj-$(CONFIG_LYNX_SDIO_VIP) += $(OS_DIR)/sdio_vip.o $(OS_DIR)/sdio_vip_ring.o
#obj-m += $(OS_DIR)/wifi_ctrl.o #custome01 200 7000s platform power up kernel module
#obj-m += $(OS_DIR)/wifi_ctrl2.o #custome01 510 7000s platform power up kernel module
else
ccflags-y += -DCONFIG_BACKPORT_PWD
obj-$(CPTCFG_LYNX_W2USB) += m88wi6700u.o
obj-$(CPTCFG_LYNX_W2SDIO) += m88wi6700s.o
obj-$(CPTCFG_LYNX_SDIO_VIP) += $(OS_DIR)/sdio_vip.o $(OS_DIR)/sdio_vip_ring.o
#obj-m += $(OS_DIR)/wifi_ctrl.o
#obj-m += $(OS_DIR)/wifi_ctrl2.o
ifeq ($(CPTCFG_LYNX_OS_LINUX), y)
CONFIG_LYNX_OS_LINUX = y
endif
ifeq ($(CPTCFG_LYNX_ROM2), y)
CONFIG_LYNX_ROM2 = y
endif
ifeq ($(CPTCFG_LYNX_ROM3), y)
CONFIG_LYNX_ROM3 = y
endif
ifeq ($(CPTCFG_LYNX_DEBUG), y)
CONFIG_LYNX_DEBUG = y
endif
ifeq ($(CPTCFG_LYNX_W2USB_FIRMWARE_ARRAY), y)
CONFIG_LYNX_W2USB_FIRMWARE_ARRAY = y
endif
ifeq ($(CPTCFG_LYNX_W2SDIO_FIRMWARE_ARRAY), y)
CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY = y
endif
ifeq ($(CPTCFG_LYNX_W2USB_NO_RESET), y)
CONFIG_LYNX_W2USB_NO_RESET = y
endif

ifeq ($(CPTCFG_LYNX_WM_MANAGER), y)
CONFIG_LYNX_WM_MANAGER = y
endif
ifeq ($(CPTCFG_HOST_WPS), y)
CONFIG_HOST_WPS = y
endif

ifeq ($(CPTCFG_CUST1_PRIV_BEACON_IE), y)
CONFIG_CUST1_PRIV_BEACON_IE = y
endif

ifeq ($(CPTCFG_HOST_P2P), y)
CONFIG_HOST_P2P = y
endif
ifeq ($(CPTCFG_SUPPORT_MONITOR_MODE), y)
CONFIG_SUPPORT_MONITOR_MODE = y
endif
ifeq ($(CPTCFG_LYNX_POWER_SAVING), y)
CONFIG_LYNX_POWER_SAVING = y
endif
ifeq ($(CPTCFG_SDIO_POLLING), y)
CONFIG_SDIO_POLLING = y
endif
ifeq ($(CPTCFG_LYNX_STATICS), y)
CONFIG_LYNX_STATICS = y
endif
ifeq ($(CPTCFG_DISABLE_HT40), y)
CONFIG_DISABLE_HT40 = y
endif
ifeq ($(CPTCFG_ENABLE_TXOPLIMIT_AP), y)
CONFIG_ENABLE_TXOPLIMIT_AP = y
endif
ifeq ($(CPTCFG_UNIT_TEST), y)
CONFIG_UNIT_TEST = y
endif
ifeq ($(CPTCFG_HOST_DEBUG_DEVICE), y)
CONFIG_HOST_DEBUG_DEVICE = y
endif
ifeq ($(CPTCFG_MP_FW), y)
CONFIG_MP_FW = y
endif
ifeq ($(CPTCFG_CH_15), y)
CONFIG_CH_15 = y
endif
ifeq ($(CPTCFG_LOW_FREQ_CH), y)
CONFIG_LOW_FREQ_CH = y
endif
ifeq ($(CPTCFG_HOST_ROAMING), y)
CONFIG_HOST_ROAMING=y
endif

endif

LINUX_OBJS = $(patsubst %, os_dep/linux/%, txrx.o hw.o debug.o main.o init.o ioctl.o utility.o)
LINUX_CFG80211_OBJS = $(patsubst %, os_dep/linux/%, cfg80211.o mlme_ops.o)
LINUX_WM_OBJS = $(patsubst %, os_dep/linux/%, ctrl_interface.o)
WM_MANAGER_OBJS = $(patsubst %, wireless_manager/src/%, driver_ops.o wlan_ie.o wm_main.o wm_mlme.o wm_scan.o wm_sta.o wm_util.o wm_ctrl.o wlan_util.o wm_wpa.o wm_eap.o aes.o aes_wrap.o md5.o prf.o rc4.o sha1.o)
CORE_OBJS = $(patsubst %, core/%, core.o wci.o hif.o mlme_api.o)

m88wi6700u-objs  := $(LINUX_OBJS) $(CORE_OBJS) $(OS_DIR)/usb.o
m88wi6700s-objs  := $(LINUX_OBJS) $(CORE_OBJS) $(OS_DIR)/sdio.o

ifeq ($(CONFIG_LYNX_OS_LINUX), y)
ccflags-y += -I$(src)/os_dep/linux/include/
ccflags-y += -DCONFIG_LYNX_OS_LINUX
endif

ifeq ($(CONFIG_LYNX_ROM2), y)
ccflags-y += -DCONFIG_LYNX_ROM2
endif

ifeq ($(CONFIG_LYNX_ROM3), y)
ccflags-y += -DCONFIG_LYNX_ROM3
endif

ifeq ($(CONFIG_LYNX_DEBUG), y)
ccflags-y += -DCONFIG_LYNX_DEBUG
endif

ifeq ($(CONFIG_HOST_WPS), y)
ccflags-y += -DCONFIG_HOST_WPS
endif

ifeq ($(CONFIG_CUST1_PRIV_BEACON_IE), y)
ccflags-y += -DCONFIG_CUST1_PRIV_BEACON_IE
endif

ifeq ($(CONFIG_HOST_P2P), y)
ccflags-y += -DCONFIG_HOST_P2P
endif

ifeq ($(CONFIG_SUPPORT_MONITOR_MODE), y)
ccflags-y += -DCONFIG_SUPPORT_MONITOR_MODE
endif

ifeq ($(CONFIG_LYNX_POWER_SAVING), y)
ccflags-y += -DCONFIG_LYNX_POWER_SAVING
endif

ifeq ($(CONFIG_MP_FW), y)
ccflags-y += -DCONFIG_MP_FW
endif

ifeq ($(CONFIG_CH_15), y)
ccflags-y += -DCONFIG_CH_15
endif

ifeq ($(CONFIG_LOW_FREQ_CH), y)
ccflags-y += -DCONFIG_LOW_FREQ_CH
endif

ifeq ($(CONFIG_HOST_ROAMING), y)
ccflags-y += -DCONFIG_HOST_ROAMING
endif

ifeq ($(CONFIG_SDIO_POLLING), y)
ccflags-y += -DCONFIG_SDIO_POLLING
endif

ifeq ($(CONFIG_LYNX_W2USB_FIRMWARE_ARRAY), y)
ccflags-y += -DCONFIG_LYNX_W2USB_FIRMWARE_ARRAY
endif

ifeq ($(CONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY), y)
ccflags-y += -DCONFIG_LYNX_W2SDIO_FIRMWARE_ARRAY
endif

ifeq ($(CONFIG_LYNX_W2USB_NO_RESET), y)
ccflags-y += -DCONFIG_LYNX_W2USB_NO_RESET
endif

ifeq ($(CONFIG_LYNX_WM_MANAGER), y)
m88wi6700u-objs  += $(WM_MANAGER_OBJS)
m88wi6700u-objs  += $(LINUX_WM_OBJS)
m88wi6700s-objs   += $(WM_MANAGER_OBJS)
lynx_w2sdio-objs  += $(LINUX_WM_OBJS)
ccflags-y += -DCONFIG_LYNX_WM_MANAGER
ccflags-y += -DCONFIG_WPA
ccflags-y += -I$(src)/wireless_manager/include/
else
ifeq ($(CONFIG_LYNX_OS_LINUX), y)
m88wi6700u-objs  += $(LINUX_CFG80211_OBJS)
m88wi6700s-objs  += $(LINUX_CFG80211_OBJS)
endif
endif

ifeq ($(CONFIG_LYNX_W2SDIO), m)
ccflags-y += -DCONFIG_LYNX_W2SDIO
endif

ifeq ($(CPTCFG_LYNX_W2SDIO), m)
ccflags-y += -DCONFIG_LYNX_W2SDIO
endif

ifeq ($(CONFIG_LYNX_W2SDIO), y)
ccflags-y += -DCONFIG_LYNX_W2SDIO
endif

ifeq ($(CPTCFG_LYNX_W2SDIO), y)
ccflags-y += -DCONFIG_LYNX_W2SDIO
endif

ifeq ($(CONFIG_LYNX_W2USB), m)
ccflags-y += -DCONFIG_LYNX_W2USB
endif

ifeq ($(CPTCFG_LYNX_W2USB), m)
ccflags-y += -DCONFIG_LYNX_W2USB
endif

ifeq ($(CONFIG_LYNX_W2USB), y)
ccflags-y += -DCONFIG_LYNX_W2USB
endif

ifeq ($(CPTCFG_LYNX_W2USB), y)
ccflags-y += -DCONFIG_LYNX_W2USB
endif


ifeq ($(CONFIG_LYNX_DEBUG), y)
ccflags-y += -DCONFIG_LYNX_DEBUG
endif

ifeq ($(CONFIG_HOST_WPS), y)
ccflags-y += -DCONFIG_HOST_WPS
endif

ifeq ($(CONFIG_CUST1_PRIV_BEACON_IE), y)
ccflags-y += -DCONFIG_CUST1_PRIV_BEACON_IE
endif

ifeq ($(CONFIG_HOST_P2P), y)
ccflags-y += -DCONFIG_HOST_P2P
endif


ifeq ($(CONFIG_SUPPORT_MONITOR_MODE), y)
ccflags-y += -DCONFIG_SUPPORT_MONITOR_MODE
endif

ifeq ($(CONFIG_LYNX_POWER_SAVING), y)
ccflags-y += -DCONFIG_LYNX_POWER_SAVING
endif

ifeq ($(CONFIG_SDIO_POLLING), y)
ccflags-y += -DCONFIG_SDIO_POLLING
endif

ifeq ($(CONFIG_LYNX_WPA), y)
ccflags-y += -DCONFIG_LYNX_WPA
endif

ifeq ($(CONFIG_LYNX_IBSS), y)
ccflags-y += -DCONFIG_LYNX_IBSS_MODE
endif

ifeq ($(CONFIG_TIMER_HANDLER), y)
ccflags-y += -DCONFIG_TIMER_HANDLER
endif

ifeq ($(CONFIG_LYNX_STATICS), y)
ccflags-y += -DCONFIG_LYNX_STATICS
endif

ifeq ($(CONFIG_DISABLE_HT40), y)
ccflags-y += -DCONFIG_DISABLE_HT40
endif

ifeq ($(CONFIG_ENABLE_TXOPLIMIT_AP), y)
ccflags-y += -DCONFIG_ENABLE_TXOPLIMIT_AP
endif

ifeq ($(CONFIG_UNIT_TEST), y)
ccflags-y += -DCONFIG_UNIT_TEST
endif

ifeq ($(CONFIG_HOST_DEBUG_DEVICE), y)
ccflags-y += -DCONFIG_HOST_DEBUG_DEVICE
endif

ifeq ($(CONFIG_MP_FW), y)
ccflags-y += -DCONFIG_MP_FW
endif

ifeq ($(CONFIG_CH_15), y)
ccflags-y += -DCONFIG_CH_15
endif

ifeq ($(CONFIG_LOW_FREQ_CH), y)
ccflags-y += -DCONFIG_LOW_FREQ_CH
endif

ifeq ($(CONFIG_HOST_ROAMING), y)
ccflags-y += -DCONFIG_HOST_ROAMING
endif

ccflags-y += -UCONFIG_ANDROID
KVERSION := $(shell uname -r)

export KLIB_BUILD

all:
	@$(MAKE) -C $(KLIB_BUILD) $(MT_KERNEL_OUTPUT_OPT) M=$(PWD) modules

modules_install:
	@$(MAKE) -C $(KLIB_BUILD) $(MT_KERNEL_OUTPUT_OPT) M=$(PWD) modules_install
	
clean:
	@$(MAKE) -C $(KLIB_BUILD) $(MT_KERNEL_OUTPUT_OPT) M=$(PWD) clean

driver_rev:
	@echo "#define LYNX_DRIVER_REV \"$(__REV)\"" > include/lynx_rev.h
	



