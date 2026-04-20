include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH += -I$(COMMON_UNF_INCLUDE)

OBJS = max-power-consumption.o
ifeq ($(CONFIG_MT_ARCH_ARM),y)
OBJS += max-power-consumption-arm.o
EXTRA_LDFLAGS = -Wl,-Ttext-segment=0x80000000
else ifeq ($(CONFIG_MT_ARCH_AARCH64),y)
OBJS += max-power-consumption-aarch64.o
endif

APP = max-power-consumption

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

include ${SDK_DIR}/build/script/Makefile-app.rule
