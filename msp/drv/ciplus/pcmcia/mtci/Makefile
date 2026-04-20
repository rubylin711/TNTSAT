include ${SDK_DIR}/build/script/base.mk

obj-$(CONFIG_MT_PCMCIA_CI) = mt_ciplus.o
mt_ciplus-objs = ci_drv.o

EXTRA_CFLAGS += -I$(COMMON_UNF_INCLUDE) \
                -I$(COMMON_API_INCLUDE) \
                -I$(COMMON_DRV_INCLUDE) \
		-I$(MSP_DRV_INCLUDE)    \
		-I$(INCLUDE_DIR)        \
		-include $(INCLUDE_DIR)/autoconf-old.h
