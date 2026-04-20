include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
          -I$(INCLUDE_DIR)/testframe \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(INCLUDE_DIR)


OBJS := src/ha_libmad_wrapper.o


LIB := libHA.AUDIO.MAD.decode

include ${SDK_DIR}/build/script/Makefile-lib.rule
