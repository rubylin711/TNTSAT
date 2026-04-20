include ${SDK_DIR}/build/script/base.mk

USER     := $(shell whoami)
CURR_DIR := $(shell pwd)
FILE     := $(CURR_DIR)/Version.txt
VARIABLE=$(shell cat $(FILE) | awk '{print $$1}')

CFG_MT_MONTAGE_PLATFORM := n
ifeq ($(CONFIG_MT_MONTAGE_PLATFORM),y)
CFG_MT_MONTAGE_PLATFORM := y
endif

.PHONY: all clean install uninstall distclean prepare

all: prepare

ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE)_$(CFG_MT_MONTAGE_PLATFORM),y_n)
ifneq ($(CURR_DIR)/gstreamer, $(wildcard $(CURR_DIR)/gstreamer))
	@echo "Begin to download mlzplyer code";
	git clone ssh://${USER}@socsw-gitserver.montage-lz.com:29418/android/platform/vendor/montage/sdk/gstreamer --depth=50;
	cd gstreamer; git reset --hard $(VARIABLE)
endif
	-cp ./gstmtlzplayer/mtlzplayer/src/mlzp_type.h ./gstmtlzplayer/mtlz-avplug/gavplay/
	-cp ./gstmtlzplayer/mtlzplayer/src/component/mlzp_sender.h ./gstmtlzplayer/mtlz-avplug/gavplay/
endif
	$(MAKE) -f media.mk
	$(MAKE) -f media.mk install

clean:
	$(MAKE) -f media.mk clean

install:
	$(MAKE) -f media.mk install

uninstall:
	$(MAKE) -f media.mk uninstall
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE)_$(CFG_MT_MONTAGE_PLATFORM),y_n)
	-rm -rf ${SDK_DIR}/pub/inc/mlzplayer
	-rm ${SDK_DIR}/pub/shared_lib/libg*
	-rm ${SDK_DIR}/pub/static_lib/libgavplay.a
	-rm ${SDK_DIR}/pub/static_lib/libgst*
endif
	
distclean:
	$(MAKE) -f media.mk distclean
ifeq ($(CURR_DIR)/gstreamer/tools, $(wildcard $(CURR_DIR)/gstreamer/tools))
	rm -rf $(CURR_DIR)/gstreamer;
endif

download:
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE)_$(CFG_MT_MONTAGE_PLATFORM),y_n)
ifeq ($(CURR_DIR)/gstreamer, $(wildcard $(CURR_DIR)/gstreamer))
	@echo "mlzplayer code exist";
else
	@echo "Begin to download mlzplyer code";
	git clone ssh://${USER}@socsw-gitserver.montage-lz.com:29418/android/platform/vendor/montage/sdk/gstreamer --depth=50;
	cd gstreamer; git reset --hard $(VARIABLE)
endif
endif

release:
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE)_$(CFG_MT_MONTAGE_PLATFORM),y_n)
ifeq ($(CURR_DIR)/gstreamer, $(wildcard $(CURR_DIR)/gstreamer))
	@echo "mlzplayer code exist";
else
	@echo "Begin to download mlzplyer code";
	git clone ssh://${USER}@socsw-gitserver.montage-lz.com:29418/android/platform/vendor/montage/sdk/gstreamer --depth=1;
endif
	cd gstreamer/mtlzplayer; make release
endif
