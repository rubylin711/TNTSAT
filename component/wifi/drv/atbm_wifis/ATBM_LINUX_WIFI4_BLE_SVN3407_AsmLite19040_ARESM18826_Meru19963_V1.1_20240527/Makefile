include ${SDK_DIR}/build/script/base.mk
#
#makefile for build atbm_wifi.ko
#
###############################################################################
#
# when release to customer ,the CUSTOMER_SUPPORT_USED must set to y!!!!!!!!!!!!!
#
###############################################################################
CUSTOMER_SUPPORT_USED=y
###############################################################################
#PLATFORM_XUNWEI    		1
#PLATFORM_SUN6I					2
#PLATFORM_FRIENDLY			3
#PLATFORM_SUN6I_64			4
#PLATFORM_HI3798M				5
#PLATFORM_HI3518E				6
#PLATFORM_X86PC         7
#PLATFORM_AMLOGIC				8
#PLATFORM_AMLOGIC_905X	9
#PLATFORM_ROCKCHIP      10
#PLATFORM_MSTAR					11
#PLATFORM_CDLINUX				12
#PLATFORM_AMLOGIC_S805	13
#PLATFORM_HIS_LINUX_3_4				14
#PLATFORM_ROCKCHIP_3229				15
#PLATFORM_ROCKCHIP_3229_ANDROID8		16
#PLATFORM_HS_IPC				17
#PLATFORM_SIGMASTAR                           18
#PLATFORM_HI3516EV200                         19
#PLATFORM_XUNWEI_2G                            20
#PLATFORM_NVT98517       					21
#PLATFORM_AK_IPC       						22
#PLATFORM_SUN8I       						23
#PLATFORM_INGENICT31				24
#PLATFORM_INGENICT41				25
#PLATFORM_SUN50IWP1						26
#PLATFORM_KALI                                  27
#PLATFORM_FH_8856                   28
#PLATFORM_MT_9255                   29
#PLATFORM_HISI3571                  30
#PLATFORM_RV1106                    33
#PLATFORM_MONTAGE                               35
platform ?=PLATFORM_MONTAGE

export

platform ?= PLATFORM_SIGMASTAR
sys ?= linux
arch ?= arm
wifi_platform ?= 2


$(warning "============>wifi_platform=$(wifi_platform))
$(warning KERNELRELEASE is $(KERNELRELEASE))

ifeq ($(CUSTOMER_SUPPORT_USED),y)
MAKEFILE_SUB ?= Makefile.build.customer
else
MAKEFILE_SUB ?= Makefile.build
endif

ifeq ($(KERNELRELEASE),)
$(warning "============>0")
export
ATBM_BUILD_IN_KERNEL=n
export DRIVER_PATH ?= $(shell pwd)



KERDIR ?= /usr/lchome/yuzhihuang/Mstar/IPC_I3/linux3.18_i3/
CROSS_COMPILE ?= /usr/lchome/yuzhihuang/Mstar/IPC_I3/arm-linux-gnueabihf-4.8.3-201404/bin/arm-linux-gnueabihf-
CROSS_COMPILE_BLE_STACK ?= /usr/lchome/yuzhihuang/Mstar/IPC_I3/arm-linux-gnueabihf-4.8.3-201404/bin/arm-linux-gnueabihf-


ifeq ($(platform),PLATFORM_MONTAGE)
KERDIR:=$(MT_KERNEL_DIR)
export
ATBM_WIFI__EXT_CCFLAGS = -DATBM_WIFI_PLATFORM=35
export ARCH:=arm64
export arch:=arm64
MAKEFILE_SUB = Makefile.build.local
CROSS_COMPILE:="$(CONFIG_CROSS_COMPILE_KERNEL)"
EXTRA_LDFLAGS += --strip-debug
endif

all:install

install:
	@echo "make PLATFORM_CROSS=$(platform)"
	$(MAKE) all -f $(MAKEFILE_SUB) ARCH=$(arch)  CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KDIR=$(KERDIR) SYS=$(sys) PLAT=$(platform) -j8

install:
modules_install:
	@echo "make PLATFORM_CROSS=$(platform)"
	$(MAKE)  -f $(MAKEFILE_SUB) M=$(shell pwd) $(MT_KERNEL_OUTPUT_OPT) INSTALL_MOD_PATH=$(MODULE_DIR) modules_install
clean:
	$(MAKE) -f $(MAKEFILE_SUB) KDIR=$(KERDIR) ARCH=$(arch) clean
strip:	
	#$(MAKE) -f $(MAKEFILE_SUB) KDIR=$(KERDIR) ARCH=$(arch) SYS=$(sys) PLAT=$(platform) strip
get_ver:
	$(MAKE) -f $(MAKEFILE_SUB) KDIR=$(KERDIR) ARCH=$(arch) SYS=$(sys) PLAT=$(platform) get_ver
ble_stack:
	@echo "start ble host"
	$(MAKE) ble_stack -f $(MAKEFILE_SUB) ARCH=$(arch)  CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KDIR=$(KERDIR) SYS=$(sys) PLAT=$(platform) -j8
ble_stack_clean:
	$(MAKE) ble_stack_clean -f $(MAKEFILE_SUB) ARCH=$(arch)  CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)" KDIR=$(KERDIR) SYS=$(sys) PLAT=$(platform) -j8

buid_config:
	$(MAKE) -C atbm_kconf clean
	$(MAKE) -C atbm_kconf mconf -f Makefile
menuconfig:buid_config
	@./atbm_kconf/mconf ./atbm_kconf/Kconfig



else
$(warning "============>1")

ifeq ($(ATBM_BUILD_IN_KERNEL),)
$(warning "============>2")
obj-m += hal_apollo/
export ATBM_BUILD_IN_KERNEL=y
else
$(warning "============>3")
export
include $(src)/Makefile.build.kernel
endif


endif

