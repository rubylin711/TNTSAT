include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)              \
            -I$(COMMON_API_INCLUDE)                 \
            -I$(COMMON_DRV_INCLUDE)                 \
            -I$(MSP_UNF_INCLUDE)                    \
            -I$(MSP_API_INCLUDE)                    \
            -I$(MSP_DRV_INCLUDE)                    \
            -I$(COMMON_DIR)/inc                     \
            -I$(COMMON_DIR)/drv/inc                 \
            -I$(MSP_DIR)/inc                        \
            -I$(MSP_DIR)/api/inc                    \
            -I$(MSP_DIR)/drv/inc                    \
            -I$(MSP_DIR)/api/mtgo/include      \
            -I$(MSP_DIR)/api/tde/include                \
            -I$(MSP_DIR)/drv/mtfb/include               \
            -I$(MSP_DIR)/drv/mtfb/src                   \
            -I$(MSP_DIR)/drv/mtfb/adp/include           \
            -I$(MSP_DIR)/drv/mtfb/adp/platform/include  \
            -I$(MSP_DIR)/drv/osd 	\
            -I$(MSP_DIR)/api/jpeg/inc/inc_6b	\
            -I$(MSP_DIR)/api/jpeg/inc/inc_hard  \
            -I$(MSP_DIR)/api/jpeg/grc_cmm_inc		\
            -I$(MSP_DIR)/api/jpeg/src_hard		\
            -I$(MSP_DIR)/drv/jpeg/include  \
            -I$(MSP_DIR)/api/png 	\
            -I$(INCLUDE_DIR)                            \
            -I$(BUILDROOT_SYSROOT_USR_INC_DIR) \
            -I$(BUILDROOT_SYSROOT_USR_INC_DIR)/freetype2

LIB := libmt_mtgo

OBJS := src/adp_gfx.o
OBJS += src/mt_go_comm.o
OBJS += src/mt_go_surface.o
OBJS += src/mt_go_gdev.o
OBJS += src/mt_go_decoder.o
OBJS += src/mt_go_bliter.o
OBJS += src/mtgo_surface.o
OBJS += src/mtgo_memsurface.o
OBJS += src/mtgo_memory.o
OBJS += src/mtgo_io.o
OBJS += src/mtgo_blit.o
OBJS += src/mtgo_decbmp.o
OBJS += src/mtgo_decgif.o
OBJS += src/adp_layer.o
OBJS += src/adp_png.o
#OBJS += src/mt_api_mmz.o
OBJS += src/adp_mtfb.o
OBJS += src/mt_go_text.o
OBJS += src/uni2gb.o
OBJS += src/adp_jpeg.o

HEADER_FILES := $(wildcard $(MSP_DIR)/api/mtgo/include/*.h)
HEADER_FILES += $(MSP_DIR)/api/mtgo/src/mtgo_common.h
HEADER_FILES += $(MSP_DIR)/api/mtgo/src/mtgo_surface.h
HEADER_FILES += $(MSP_DIR)/api/mtgo/src/mtgo_memory.h
HEADER_FILES += $(MSP_DIR)/api/mtgo/src/mtgo_adp_sys.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
