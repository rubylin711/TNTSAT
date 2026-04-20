#
# Montage LZ VO FW Driver Makefile
#
include ${SDK_DIR}/build/script/base.mk

#external release dir
ifeq ($(CFG_MT_BUILD_LOADER),y)
EXT_PUB_DIR := pub/$(MT_RELEASE_DIR)/loader
else ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)
EXT_PUB_DIR := pub/$(MT_RELEASE_DIR)/loader
else
EXT_PUB_DIR := pub/$(MT_RELEASE_DIR)
endif

ifneq ($(KERNELRELEASE),)

# kbuild part of makefile

MOD_NAME := mt_vofw

EXTRA_CFLAGS += $(CFG_MT_KMOD_CFLAGS) -Wno-date-time

EXTRA_CFLAGS += '-Wno-int-to-pointer-cast' '-Wno-pointer-to-int-cast'

EXTRA_CFLAGS += -I$(COMMON_UNF_INCLUDE) \
                                      -I$(MSP_UNF_INCLUDE) \
                                      -I$(COMMON_DRV_INCLUDE) \
                                      -I$(MSP_DRV_INCLUDE) \
                                      -I$(MSP_DIR)/drv/vo_fw \
                                      -I$(MSP_DIR)/drv/vo_fw/inc \
                                      -I$(MSP_DIR)/drv/vo/vdp_v3_0/com

ifeq ($(CFG_MT_SDK_RELEASE),n)
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/vo_fw/sl_hdr \
                                      -I$(MSP_DIR)/drv/vo_fw/sl_hdr/inc
endif

EXTRA_CFLAGS += -I$(MSP_DIR)/drv/vfmw/video_lib_1.0

EXTRA_CFLAGS += -DVIDEO_DISP_4K  -DVFW_SYMPHONY -DDISP_ON_AP_LINUX

ifeq ($(CONFIG_MT_VO_FW),m)
EXTRA_CFLAGS += -DCONFIG_MT_VO_FW_KO
endif

obj-$(CONFIG_MT_VO_FW) := $(MOD_NAME).o

$(MOD_NAME)-y := src/MT_DF_ScalarPara.o
$(MOD_NAME)-y += src/MT_DF_Osd_Gra_Scaler.o

ifeq ($(CFG_MT_BUILD_LOADER),y)
EXTRA_CFLAGS += -DVO_FW_FOR_OTA
$(MOD_NAME)-y += src/MT_DF_disp_ota.o
else ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)
EXTRA_CFLAGS += -DVO_FW_FOR_OTA
$(MOD_NAME)-y += src/MT_DF_disp_ota.o
else
$(MOD_NAME)-y += src/MT_DF_disp.o
$(MOD_NAME)-y += src/MT_DF_FRC.o
$(MOD_NAME)-y += src/MT_DF_Freeze.o
$(MOD_NAME)-y += src/MT_DF_PreScale.o
$(MOD_NAME)-y += src/MT_DF_DCE.o
$(MOD_NAME)-y += src/MT_DF_hdr10p_ootf.o
$(MOD_NAME)-y += src/MT_DF_HdrVivid_ootf.o
$(MOD_NAME)-y += src/MT_DF_PullDown.o
endif

ifeq ($(CFG_MT_CHIP),aria)
$(MOD_NAME)-y += src/MT_DF_Aria_reg.o
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
$(MOD_NAME)-y += src/MT_DF_Symphony_reg.o
$(MOD_NAME)-y += src/MT_DF_2K_Disp_Set.o
endif

ifeq ($(CFG_MT_CHIP),symphony6)
ifneq ($(CFG_MT_BUILD_LOADER),y)
ifneq ($(CONFIG_MT_PRODUCT_LOADER),y)
$(MOD_NAME)-y += src/MT_DF_4K_Disp_Set.o
endif
endif
$(MOD_NAME)-y += src/MT_DF_4k_reg.o
$(MOD_NAME)-y += src/MT_DF_Osd_Gra_Scaler_4K.o
endif

$(MOD_NAME)-y += vo_fw.o

else

# normal makefile

default:

genbin:
	cp -f src/MT_DF_ScalarPara.o src/MT_DF_ScalarPara.o_shipped
	cp -f src/MT_DF_Osd_Gra_Scaler.o src/MT_DF_Osd_Gra_Scaler.o_shipped
ifeq ($(CFG_MT_BUILD_LOADER),y)
	cp -f src/MT_DF_disp_ota.o src/MT_DF_disp_ota.o_shipped
else ifeq ($(CONFIG_MT_PRODUCT_LOADER),y)
	cp -f src/MT_DF_disp_ota.o src/MT_DF_disp_ota.o_shipped
else
	cp -f src/MT_DF_disp.o src/MT_DF_disp.o_shipped
	cp -f src/MT_DF_FRC.o src/MT_DF_FRC.o_shipped
	cp -f src/MT_DF_Freeze.o src/MT_DF_Freeze.o_shipped
	cp -f src/MT_DF_PreScale.o src/MT_DF_PreScale.o_shipped
	cp -f src/MT_DF_DCE.o src/MT_DF_DCE.o_shipped
	cp -f src/MT_DF_hdr10p_ootf.o src/MT_DF_hdr10p_ootf.o_shipped
	cp -f src/MT_DF_HdrVivid_ootf.o src/MT_DF_HdrVivid_ootf.o_shipped
	cp -f src/MT_DF_PullDown.o src/MT_DF_PullDown.o_shipped
endif
ifeq ($(CFG_MT_CHIP),aria)
	cp -f src/MT_DF_Aria_reg.o src/MT_DF_Aria_reg.o_shipped
endif
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
	cp -f src/MT_DF_Symphony_reg.o src/MT_DF_Symphony_reg.o_shipped
	cp -f src/MT_DF_2K_Disp_Set.o src/MT_DF_2K_Disp_Set.o_shipped
endif
ifeq ($(CFG_MT_CHIP),symphony6)
ifneq ($(CFG_MT_BUILD_LOADER),y)
ifneq ($(CONFIG_MT_PRODUCT_LOADER),y)
	cp -f src/MT_DF_4K_Disp_Set.o src/MT_DF_4K_Disp_Set.o_shipped
endif
endif
	cp -f src/MT_DF_4k_reg.o src/MT_DF_4k_reg.o_shipped
	cp -f src/MT_DF_Osd_Gra_Scaler_4K.o src/MT_DF_Osd_Gra_Scaler_4K.o_shipped
endif

#clean .o_shipped files
binclean:
	rm -f src/*.o_shipped

#external release
#rm source files
#copy .o_shipped files to pub dir
ext_release: genbin
	mkdir -p $(EXT_PUB_DIR)
	cp -f src/*.o_shipped $(EXT_PUB_DIR)
	#rm -rf src

#external unrelease
ext_unrelease:
	rm -rf pub

#copy .o_shipped files from pub dir
ext_prepare:
	mkdir -p src
	cp -f $(EXT_PUB_DIR)/*.o_shipped src/

endif
