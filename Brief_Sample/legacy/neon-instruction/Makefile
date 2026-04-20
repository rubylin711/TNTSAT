include ${SDK_DIR}/build/script/base.mk

OBJS = neon-instruction.o
ifeq ($(CONFIG_MT_ARCH_ARM),y)
OBJS += arm32.o
else ifeq ($(CONFIG_MT_ARCH_AARCH64),y)
OBJS += arm64.o
endif

DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)

APP = neon-instruction

include ${SDK_DIR}/build/script/Makefile-app.rule
