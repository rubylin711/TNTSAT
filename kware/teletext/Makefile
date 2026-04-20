include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include

INCLUDE_PATH += -I$(SAMPLE_DIR)/common  \
          -I$(COMMON_DIR)/inc \
          -I$(COMMON_DIR)/drv/inc \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          
LIB := libmt_ttx

#OBJS := src/mt_unf_ttx.o src/teletext_data.o src/ttx_db_vsb.o src/ttx_font_vsb.o src/ttx_lang_vsb.o src/ttx_hamm_vsb.o src/ttx_format_vsb.o src/ttx_dec_vsb.o src/vbi_vsb.o
OBJS := src/mt_unf_ttx.o src/teletext_data.o src/ttx_db_vsb.o src/ttx_font_vsb.o src/ttx_lang_vsb.o src/ttx_hamm_vsb.o src/ttx_format_vsb.o src/ttx_dec_vsb.o src/vbi_vsb.o src/ttx_render_vsb.o

HEADER_FILES=$(KWARE_DIR)/teletext/include/mt_unf_ttx.h 
HEADER_FILES_INSTALL_SUBDIR=teletext

include ${SDK_DIR}/build/script/Makefile-lib.rule
