include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)      \
            -I$(COMMON_API_INCLUDE)         \
            -I$(COMMON_DRV_INCLUDE)         \
            -I$(MSP_UNF_INCLUDE)            \
            -I$(MSP_API_INCLUDE)            \
            -I$(MSP_DRV_INCLUDE)            \
            -I$(COMMON_DIR)/inc             \
            -I$(COMMON_DIR)/drv/inc         \
            -I$(COMMON_DIR)/api/inc         \
            -I$(MSP_DIR)/inc                \
            -I$(MSP_DIR)/api/inc            \
            -I$(MSP_DIR)/drv/inc            \
            -I$(MSP_DIR)/api/ffmpeg         \
            -I$(INCLUDE_DIR)                \
			-I$(EXTERNAL_BINARY_PREFIX_DIR)/include

OBJS := RingBuffer.o ext_decoder.o


#HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_audio.h

include $(MSP_DIR)/api/Makefile.rule
