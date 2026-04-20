include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_DIR)/inc
INCLUDE_PATH += -I$(MSP_DIR)/inc
INCLUDE_PATH += -I$(CURDIR)/libmpeg2-0.5.1/include

LIB := libmtlzswplayer

OBJS := libmpeg2-0.5.1/libmpeg2/alloc.o \
		libmpeg2-0.5.1/libmpeg2/cpu_accel.o \
		libmpeg2-0.5.1/libmpeg2/cpu_state.o \
		libmpeg2-0.5.1/libmpeg2/decode.o \
		libmpeg2-0.5.1/libmpeg2/header.o \
		libmpeg2-0.5.1/libmpeg2/idct.o \
		libmpeg2-0.5.1/libmpeg2/motion_comp.o \
		libmpeg2-0.5.1/libmpeg2/slice.o

OBJS += mtlz_comp_static.o \
		mtlz_display_sink.o \
		mtlz_dmx_source.o \
		mtlz_fake_sink.o \
		mtlz_file_source.o \
		mtlz_mpeg2_codec.o \
		mtlz_packer.o \
		mtlzplayer.o \
		mtlzplayer_os_linux.o

# ffmpeg
#INCLUDE_PATH += -I./ffmpeg-2.8.15
#OBJS += mtlz_ffmpeg_codec.o
#DEPEND_LIBS += -lavcodec-2.8.15 -lavutil-2.8.15

EXTRA_CFLAGS += -DLINUX

HEADER_FILES=$(KWARE_DIR)/mtlzswplayer/mtlz_types.h $(KWARE_DIR)/mtlzswplayer/mtlzplayer.h
HEADER_FILES_INSTALL_SUBDIR=mtlzswplayer

include ${SDK_DIR}/build/script/Makefile-lib.rule
