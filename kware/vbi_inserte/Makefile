include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I${SDK_DIR}/common/inc
INCLUDE_PATH += -I$(KWARE_DIR)/vbi_inserte/include -I$(KWARE_DIR)/include/mtos -I${SDK_DIR}/msp/drv/inc

LIB := libvbi_inserte

OBJS := src/vbi_inserter_aria.o src/vbi_inserter.o

HEADER_FILES=$(wildcard $(KWARE_DIR)/vbi_inserte/include/*.h)
HEADER_FILES_INSTALL_SUBDIR=vbi_inserte

include ${SDK_DIR}/build/script/Makefile-lib.rule
