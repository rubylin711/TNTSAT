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

OBJS := mpi_adec.o

ifeq ($(CFG_MT_CHIP),symphony4)
#OBJS += hal_adec_buffer.o
EXTRA_CFLAGS += -DLINUX
endif

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_audio_codec.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.AAC.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.AAC.encode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.AC3PASSTHROUGH.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.AMRNB.codec.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.AMRWB.codec.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.BLURAYLPCM.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.COOK.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.DOLBYPLUS.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.DOLBYTRUEHD.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.DRA.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.DTSHD.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.DTSM6.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.DTSPASSTHROUGH.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.FFMPEG_DECODE.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.FFMPEG_WMAPRO.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.G711.codec.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.G722.codec.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.MP2.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.MP3.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.PCM.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.TRUEHDPASSTHROUGH.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/HA.AUDIO.WMA9STD.decode.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_audio.h

include $(MSP_DIR)/api/Makefile.rule
