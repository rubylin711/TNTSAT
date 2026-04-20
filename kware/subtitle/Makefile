include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(CURDIR)/include

LIB := libmt_subtitle

OBJS := src/mt_unf_subt.o src/subtitle_data.o src/subtitle_parse.o src/scte_subt_data.o src/scte_subt_display.o src/scte_subt_parse.o

HEADER_FILES=$(KWARE_DIR)/subtitle/include/mt_unf_subt.h
HEADER_FILES_INSTALL_SUBDIR=subtitle

include ${SDK_DIR}/build/script/Makefile-lib.rule
