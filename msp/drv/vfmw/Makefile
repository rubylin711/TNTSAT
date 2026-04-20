#
# Montage LZ VFMW(VDEC) FW Makefile
#
include ${SDK_DIR}/build/script/base.mk

VLIB_DIR := video_lib_1.0

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

MOD_NAME := mt_vfmw

EXTRA_CFLAGS += $(CFG_MT_KMOD_CFLAGS) -Wno-date-time

EXTRA_CFLAGS += '-Wno-int-to-pointer-cast' '-Wno-pointer-to-int-cast'

EXTRA_CFLAGS += -I$(COMMON_UNF_INCLUDE) \
                                      -I$(MSP_UNF_INCLUDE) \
                                      -I$(COMMON_DRV_INCLUDE) \
                                      -I$(MSP_DRV_INCLUDE) \
                                      -I$(MSP_DIR)/drv/vfmw/$(VLIB_DIR) \

ifeq ($(CFG_MT_SDK_RELEASE),n)
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/vfmw/$(VLIB_DIR)/common/inc
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/vfmw/$(VLIB_DIR)/middle/inc
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/vfmw/$(VLIB_DIR)/api/inc
EXTRA_CFLAGS += -I$(MSP_DIR)/drv/vfmw/$(VLIB_DIR)/decoder/inc
endif

EXTRA_CFLAGS +=-DVDEC_SYMPHONY_AP
EXTRA_CFLAGS +=-DVFW_ENABLE_RV

ifeq ($(CFG_MT_CHIP),aria)
EXTRA_CFLAGS += -DARIA_LINUX
else
EXTRA_CFLAGS += -DSYMPHONY_LINUX
endif

EXTRA_CFLAGS += -DVFW_SYMPHONY

obj-$(CONFIG_MT_VFMW) := $(MOD_NAME).o

$(MOD_NAME)-y := $(VLIB_DIR)/api/src/fw_api.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/bitstream.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/DPB_AVS_MPEG.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/fw_clock.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/fw_dpb_output.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/fw_es_descriptor.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/fw_hw_reset.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/fw_timeout.o
$(MOD_NAME)-y += $(VLIB_DIR)/common/src/fw_utility.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/avs.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/h264_dec.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/h264_dpb.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/h264_header.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/h264_image.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/h264_mbuffer.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/h264_output.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_comdpb.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_compic.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_comrom.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_comslice.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_deccavlc.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_decsei.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/hevc_dectop.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/mpeg4.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/mpeg.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/rv.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vc1_dec.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vc1_dpb.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp8.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9_entropy.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9_entropymode.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9_entropymv.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9_prob.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9_quant_common.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vp9_seg_common.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/avs2_dec.o
$(MOD_NAME)-y += $(VLIB_DIR)/decoder/src/vdec_common.o
$(MOD_NAME)-y += $(VLIB_DIR)/middle/src/fw_middle.o
$(MOD_NAME)-y += $(VLIB_DIR)/middle/src/fw_scheduler.o

$(MOD_NAME)-y += $(VLIB_DIR)/vfmw_intf.o

else

# normal makefile

default:

genbin:
	cp -f $(VLIB_DIR)/api/src/fw_api.o $(VLIB_DIR)/api/src/fw_api.o_shipped
	cp -f $(VLIB_DIR)/common/src/bitstream.o $(VLIB_DIR)/common/src/bitstream.o_shipped
	cp -f $(VLIB_DIR)/common/src/DPB_AVS_MPEG.o $(VLIB_DIR)/common/src/DPB_AVS_MPEG.o_shipped
	cp -f $(VLIB_DIR)/common/src/fw_clock.o $(VLIB_DIR)/common/src/fw_clock.o_shipped
	cp -f $(VLIB_DIR)/common/src/fw_dpb_output.o $(VLIB_DIR)/common/src/fw_dpb_output.o_shipped
	cp -f $(VLIB_DIR)/common/src/fw_es_descriptor.o $(VLIB_DIR)/common/src/fw_es_descriptor.o_shipped
	cp -f $(VLIB_DIR)/common/src/fw_hw_reset.o $(VLIB_DIR)/common/src/fw_hw_reset.o_shipped
	cp -f $(VLIB_DIR)/common/src/fw_timeout.o $(VLIB_DIR)/common/src/fw_timeout.o_shipped
	cp -f $(VLIB_DIR)/common/src/fw_utility.o $(VLIB_DIR)/common/src/fw_utility.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/avs.o $(VLIB_DIR)/decoder/src/avs.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/h264_dec.o $(VLIB_DIR)/decoder/src/h264_dec.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/h264_dpb.o $(VLIB_DIR)/decoder/src/h264_dpb.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/h264_header.o $(VLIB_DIR)/decoder/src/h264_header.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/h264_image.o $(VLIB_DIR)/decoder/src/h264_image.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/h264_mbuffer.o $(VLIB_DIR)/decoder/src/h264_mbuffer.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/h264_output.o $(VLIB_DIR)/decoder/src/h264_output.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_comdpb.o $(VLIB_DIR)/decoder/src/hevc_comdpb.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_compic.o $(VLIB_DIR)/decoder/src/hevc_compic.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_comrom.o $(VLIB_DIR)/decoder/src/hevc_comrom.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_comslice.o $(VLIB_DIR)/decoder/src/hevc_comslice.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_deccavlc.o $(VLIB_DIR)/decoder/src/hevc_deccavlc.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_decsei.o $(VLIB_DIR)/decoder/src/hevc_decsei.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/hevc_dectop.o $(VLIB_DIR)/decoder/src/hevc_dectop.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/mpeg4.o $(VLIB_DIR)/decoder/src/mpeg4.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/mpeg.o $(VLIB_DIR)/decoder/src/mpeg.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/rv.o $(VLIB_DIR)/decoder/src/rv.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vc1_dec.o $(VLIB_DIR)/decoder/src/vc1_dec.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vc1_dpb.o $(VLIB_DIR)/decoder/src/vc1_dpb.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp8.o $(VLIB_DIR)/decoder/src/vp8.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9.o $(VLIB_DIR)/decoder/src/vp9.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9_entropy.o $(VLIB_DIR)/decoder/src/vp9_entropy.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9_entropymode.o $(VLIB_DIR)/decoder/src/vp9_entropymode.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9_entropymv.o $(VLIB_DIR)/decoder/src/vp9_entropymv.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9_prob.o $(VLIB_DIR)/decoder/src/vp9_prob.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9_quant_common.o $(VLIB_DIR)/decoder/src/vp9_quant_common.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vp9_seg_common.o $(VLIB_DIR)/decoder/src/vp9_seg_common.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/avs2_dec.o $(VLIB_DIR)/decoder/src/avs2_dec.o_shipped
	cp -f $(VLIB_DIR)/decoder/src/vdec_common.o $(VLIB_DIR)/decoder/src/vdec_common.o_shipped
	cp -f $(VLIB_DIR)/middle/src/fw_middle.o $(VLIB_DIR)/middle/src/fw_middle.o_shipped
	cp -f $(VLIB_DIR)/middle/src/fw_scheduler.o $(VLIB_DIR)/middle/src/fw_scheduler.o_shipped

#clean .o_shipped files
binclean:
	rm -f $(VLIB_DIR)/api/src/*.o_shipped
	rm -f $(VLIB_DIR)/common/src/*.o_shipped
	rm -f $(VLIB_DIR)/decoder/src/*.o_shipped
	rm -f $(VLIB_DIR)/middle/src/*.o_shipped

#external release
#rm source files
#copy .o_shipped files to pub dir
ext_release: genbin
	mkdir -p $(EXT_PUB_DIR)
	cp -arf --parents $(VLIB_DIR)/api/src/*.o_shipped $(EXT_PUB_DIR)
	cp -arf --parents $(VLIB_DIR)/common/src/*.o_shipped $(EXT_PUB_DIR)
	cp -arf --parents $(VLIB_DIR)/decoder/src/*.o_shipped $(EXT_PUB_DIR)
	cp -arf --parents $(VLIB_DIR)/middle/src/*.o_shipped $(EXT_PUB_DIR)
	cp -f $(VLIB_DIR)/vfmw.h $(EXT_PUB_DIR)/$(VLIB_DIR)
	cp -f $(VLIB_DIR)/vfmw_intf.c $(EXT_PUB_DIR)/$(VLIB_DIR)

#external unrelease
ext_unrelease:
	rm -rf pub

#copy .o_shipped files from pub dir
ext_prepare:
	mkdir -p $(VLIB_DIR)/api/src
	mkdir -p $(VLIB_DIR)/common/src
	mkdir -p $(VLIB_DIR)/decoder/src
	mkdir -p $(VLIB_DIR)/middle/src
	cp -f $(EXT_PUB_DIR)/$(VLIB_DIR)/api/src/*.o_shipped $(VLIB_DIR)/api/src/
	cp -f $(EXT_PUB_DIR)/$(VLIB_DIR)/common/src/*.o_shipped $(VLIB_DIR)/common/src/
	cp -f $(EXT_PUB_DIR)/$(VLIB_DIR)/decoder/src/*.o_shipped $(VLIB_DIR)/decoder/src/
	cp -f $(EXT_PUB_DIR)/$(VLIB_DIR)/middle/src/*.o_shipped $(VLIB_DIR)/middle/src/
	#FIXME: need check vfmw.h or vfmw_intf.c modified?
	cp -f $(EXT_PUB_DIR)/$(VLIB_DIR)/vfmw.h $(VLIB_DIR)
	cp -f $(EXT_PUB_DIR)/$(VLIB_DIR)/vfmw_intf.c $(VLIB_DIR)

endif
