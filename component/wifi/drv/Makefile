include ${SDK_DIR}/build/script/base.mk

CFLAGS := CROSS_COMPILE="$(CROSS_COMPILE)"
CFLAGS += ROOTFS_DIR=$(ROOTFS_DIR)
CFLAGS += CFG_MT_KERNEL_VERSION=$(CFG_MT_KERNEL_VERSION)
ifeq ($(CONFIG_MT_WIFI_MODE_STA),y)
CFLAGS += CONFIG_MT_WIFI_MODE_STA=y
endif
ifeq ($(CONFIG_MT_WIFI_MODE_AP),y)
CFLAGS += CONFIG_MT_WIFI_MODE_AP=y
endif
ifeq ($(ANDROID_BUILD),y)
CFLAGS += ANDROID_BUILD=y
endif

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   variable
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#objects := Flax_wi6700
objects :=

ifeq ($(CFG_MT_WIFI_DEVICE_RTL8188CUS),y)
objects += usb_rtl8188cu
endif

ifeq ($(CFG_MT_WIFI_DEVICE_RTL8192CU),y)
objects += usb_rtl8188cu
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL8192FU),y)
objects += usb_rtl8192fu
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL8188EUS),y)
objects += usb_rtl8188eu
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL8188FTV),y)
objects += usb_rtl8188ftv
endif

ifeq ($(CFG_MT_WIFI_DEVICE_RTL8192EU),y)
objects += usb_rtl8192eu
endif

ifeq ($(CFG_MT_WIFI_DEVICE_RTL8192DU),y)
objects += usb_rtl8192du
endif

ifeq ($(CFG_MT_WIFI_DEVICE_RTL8812AU),y)
objects += usb_rtl8812au
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL8723BU),y)
objects += rtl8723BU_WiFi_linux_v4.4.5_24888.20171101_BTCOEX20151228-664a
endif

ifeq ($(CFG_MT_WIFI_DEVICE_RT5370),y)
objects += mtk
endif

ifeq ($(CFG_MT_WIFI_DEVICE_RT5572),y)
objects += usb_rt5572
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_MT7601U),y)
#objects += usb_mt7601u
objects += mt7601
endif

ifeq ($(CFG_MT_WIFI_DEVICE_AR9271),y)
objects += usb_ar9271
endif

ifeq ($(CFG_MT_WIFI_DEVICE_AR9374),y)
objects += usb_ar9374
endif

ifeq ($(CFG_MT_WIFI_DEVICE_QCA1021G),y)
objects += usb_ar9374
endif

ifeq ($(CFG_MT_WIFI_DEVICE_QCA1021X),y)
objects += usb_ar9374
endif

ifeq ($(CFG_MT_WIFI_DEVICE_BCM43236),y)
objects += usb_bcm43xx
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL8821CU),y)
objects += usb_rtl8821cu
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL8723DS),y)
objects += rtl8723ds
endif


ifeq ($(CONFIG_MT_WIFI_DEVICE_ATBM6032),y)
objects += atbm_wifis
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL88X2CU),y)
objects += usb_rtl88x2cu
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_RTL88X2CS),y)
objects += rtl88x2cs
endif

ifeq ($(CONFIG_MT_WIFI_DEVICE_AIC8800),y)
objects += aic8800
endif

objects_clean:=$(addsuffix _clean,$(objects))
objects_install:=$(addsuffix _install,$(objects))
objects_uninstall:=$(addsuffix _uninstall,$(objects))
objects_distclean:=$(addsuffix _distclean,$(objects))

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean install uninstall distclean $(objects) $(objects_clean) $(objects_install) $(objects_uninstall) $(objects_distclean)
all:
	$(foreach d,$(objects), make -C $(d) all;)

clean: 
	rm -rf `find ./ -name "*.mod.c"`
	rm -rf `find ./ -name "*.mod"`
	rm -rf `find ./ -name "*.cmd"`
	rm -rf `find ./ -name "*.o"`
	rm -rf `find ./ -name ".*.d"`
	rm -rf `find ./ -name "*.ko"`
	rm -rf `find ./ -name "*.symvers"`
	rm -rf `find ./ -name "*.markers"`
	rm -rf `find ./ -name "*.order"`
	rm -rf `find ./ -name "*.tmp_versions"`
	rm -f `find ./ -name "\.config"`
	rm -f mt7601/tools/bin2h

install: $(objects_install)
#install:

uninstall: $(objects_uninstall)
#uninstall:

distclean:

#==================================================================
#                          modules
#==================================================================
$(objects):
	make $(CFLAGS) -C $@ all

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean

$(objects_install):
	make $(CFLAGS) -C $(patsubst %_install,%,$@) install

$(objects_uninstall):
	make -C $(patsubst %_uninstall,%,$@) uninstall

$(objects_distclean):
	make -C $(patsubst %_distclean,%,$@) distclean
