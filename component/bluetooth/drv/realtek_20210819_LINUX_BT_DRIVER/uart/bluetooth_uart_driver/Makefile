include ${SDK_DIR}/build/script/base.mk

EXTRA_CFLAGS += -I$(MSP_DIR)/drv/inc -I$(SDK_DIR)/pub/inc

ifneq ($(KERNELRELEASE),)
	obj-m		:= hci_uart.o
	hci_uart-y	:= hci_ldisc.o hci_h4.o hci_rtk_h5.o rtk_coex.o
	#EXTRA_CFLAGS += -DDEBUG

else
	PWD := $(shell pwd)

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE="$(CONFIG_CROSS_COMPILE)" -C $(MT_KERNEL_DIR) M=$(shell pwd)  $(MT_KERNEL_OUTPUT_OPT) modules -j$(j)

clean:
	rm -rf *.o *.mod.c *.mod.o *.ko *.symvers *.order *.a .*.cmd
	rm -rf .tmp_versions 

endif
