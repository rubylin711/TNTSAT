include ${SDK_DIR}/build/script/base.mk

kernel_config_check:
	make syncconfig ARCH=$(KERNEL_ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE_KERNEL)"

user_config_check:
