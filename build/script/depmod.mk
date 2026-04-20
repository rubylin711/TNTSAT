include ${SDK_DIR}/build/script/base.mk

export KERNEL_OUTPUT MODULE_DIR_STRIPED CFG_MT_KERNEL_VERSION

.PHONY: all

all:
	${SDK_DIR}/build/script/depmod.sh
