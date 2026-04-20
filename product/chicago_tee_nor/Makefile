include ${SDK_DIR}/build/script/base.mk

ifeq (${SDK_DIR}/image/$(MFRS)/board_config.mk,$(wildcard ${SDK_DIR}/image/$(MFRS)/board_config.mk))
	include ${SDK_DIR}/image/$(MFRS)/board_config.mk
endif
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
	include ${SDK_DIR}/product/chicago/usrfs_gstreamer.mk
endif

KERNELRELEASE ?= $(shell cat $(KERNEL_OUTPUT)/include/config/kernel.release 2> /dev/null)

.PHONY: all install fs ramdisk XMLBIN bin update rootfs update_fpga usrfs teefs datafs help

all: help

help:
	@echo "valid target:"
	@echo "install"

#
# define make usrfs function
# [out]: usrfs_tmp.bin, usrfs_tmp.img
#
# for ubifs: 124K * 264 = 32M
#
define fun_make_usrfs_image
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_empty.bin
	@if [ $(USERFS_TYPE) = cramfs ]; then \
		${SDK_DIR}/tools/linux/mkfs.cramfs $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin || exit 1; \
		$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/usrfs_ubinize.cfg || exit 1; \
		${SDK_DIR}/tools/linux/mkfs.cramfs ${SDK_DIR}/image/$(MFRS)/usrfs_empty ${SDK_DIR}/image/$(MFRS)/usrfs_empty.bin || exit 1; \
	fi
	@if [ $(USERFS_TYPE) = squashfs ]; then \
		${SDK_DIR}/tools/linux/mksquashfs $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin -comp xz || exit 1; \
		$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/usrfs_ubinize.cfg || exit 1; \
		${SDK_DIR}/tools/linux/mksquashfs ${SDK_DIR}/image/$(MFRS)/usrfs_empty ${SDK_DIR}/image/$(MFRS)/usrfs_empty.bin -comp xz || exit 1; \
	fi
	@if [ $(USERFS_TYPE) = jffs2 ]; then \
		${SDK_DIR}/tools/linux/mkfs.jffs2 -r $(USR_FS_DIR) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -l -e 0x10000 --pad=$(USERFS_SIZE) -s 0x100 || exit 1; \
	fi
	@if [ $(USERFS_TYPE) = ubifs ]; then \
		$(MKUBIFS) -r $(USR_FS_DIR) -F -m 2048 -e 126976 -c 264 -o ${SDK_DIR}/image/$(MFRS)/user_nandubifs.img || exit 1; \
		$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/user_nandubinize.cfg || exit 1; \
		rm -f ${SDK_DIR}/image/$(MFRS)/user_nandubifs.img || exit 1; \
	fi
endef

##### make root fs #####
# initramfs
# cramfs
# squashfs
# ubifs
#######################
rootfs:
	@echo "==> make rootfs"
	mkdir -p ${SDK_DIR}/image/$(MFRS)
	rm -rf ${SDK_DIR}/image/$(MFRS)/rootfs
	cp --remove-destination -arf $(MONTAGE_ROOTFS_STRIPED) ${SDK_DIR}/image/$(MFRS)/rootfs
	#sed -i 's|APP_NAME=.*|APP_NAME=mt_sample|g' ${SDK_DIR}/image/$(MFRS)/rootfs/root/.profile
	MFRS=$(MFRS) AV_BIN_TFTP_DIR=$(AV_BIN_DIR:${SDK_DIR}/%=%) KERNEL_BIN_TFTP_DIR=$(KERNEL_OUTPUT:${SDK_DIR}/%=%)/arch/$(KERNEL_ARCH)/boot ${SDK_DIR}/build/script/tftp-nfs.sh rcS
	mkdir -p ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents modules.* ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents updates ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/regulator/symphony4-regulator.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/host/ehci-platform.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/host/ehci-hcd.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/phy/phy-generic.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	##-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/legacy/g_mass_storage.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	##-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/function/usb_f_ss_lb.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	##-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/legacy/g_zero.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	##-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/function/usb_f_mass_storage.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	##-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/mmc/host/sdhci-mt.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cp --remove-destination -af ${SDK_DIR}/image/$(MFRS)/usb/usbmodeswitch.sh ${SDK_DIR}/image/$(MFRS)/rootfs/sbin
	##-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/udc/mt_usb_udc.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cp --remove-destination -arf $(MODULE_DIR_STRIPED)/lib/firmware ${SDK_DIR}/image/$(MFRS)/rootfs/lib
	@echo "you can copy something into ${SDK_DIR}/image/$(MFRS)/rootfs"
	#TODO...

#
# FIXME: remove so
#
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libcrypto.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libe2p.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libext2fs.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libntfs-3g.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libinproctrace.*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libssl.*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/libasan.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/libgfortran.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/libgomp.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/libhwasan.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/liblsan.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/libtsan.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib64/libubsan.*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/e2fsck
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/fsck
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/fsck.ext*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/mke2fs
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/mkfs.ext*

ifneq (,$(findstring initramfs,$(ROOTFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/ramdisk.cpio.gz
	rm -f ${SDK_DIR}/image/$(MFRS)/ramdisk.cpio.lzma
	cd ${SDK_DIR}/image/$(MFRS)/rootfs; find . | cpio -o -H newc | gzip > ${SDK_DIR}/image/$(MFRS)/ramdisk.cpio.gz
	cd ${SDK_DIR}/image/$(MFRS)/rootfs; find . | cpio -o -H newc | lzma > ${SDK_DIR}/image/$(MFRS)/ramdisk.cpio.lzma
endif
ifneq (,$(findstring cramfs,$(ROOTFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs.cramfs
	${SDK_DIR}/tools/linux/mkfs.cramfs ${SDK_DIR}/image/$(MFRS)/rootfs ${SDK_DIR}/image/$(MFRS)/rootfs.cramfs
endif
ifneq (,$(findstring squashfs,$(ROOTFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs.squashfs
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs_squashubi.img
	# nor flash
	${SDK_DIR}/tools/linux/mksquashfs ${SDK_DIR}/image/$(MFRS)/rootfs ${SDK_DIR}/image/$(MFRS)/rootfs.squashfs -comp xz
	# nand flash(plus ubi hub)
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/rootfs_squashubi.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/rootfs_ubinize.cfg
endif
ifneq (,$(findstring ubifs,$(ROOTFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs.img
	rm -f ${SDK_DIR}/image/$(MFRS)/rootubifs.img
	# 124K * 148 = 18M
	$(MKUBIFS) -r ${SDK_DIR}/image/$(MFRS)/rootfs -F -m 2048 -e 126976 -c 300 -o ${SDK_DIR}/image/$(MFRS)/rootubifs.img
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/rootfs.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/ubifs_rootfs_ubinize.cfg
endif
ifneq (,$(findstring ext4,$(ROOTFS_TYPE)))
	$(MONTAGE_TOOLS_DIR)/mkext4.sh ${SDK_DIR}/image/$(MFRS)/rootfs ${SDK_DIR}/image/$(MFRS)/rootfs.ext4
endif

ramdisk:
#roofs env
ifeq ($(wildcard ${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_spinand_ramdisk.env),${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_spinand_ramdisk.env)
ifeq ($(ROOTFS_RAMDISK),y)
ifneq (,$(findstring initramfs,$(ROOTFS_TYPE)))
	${SDK_DIR}/tools/linux/mkramdiskenv.bin -a arm -t 0 -f ${SDK_DIR}/image/$(MFRS)/ramdisk.cpio.gz -e ${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_spinand_ramdisk.env -c ${SDK_DIR}/tools/linux/mkimage
endif
endif
#other env
	${SDK_DIR}/tools/linux/mkramdiskenv.bin -a arm -t 1 -f ${SDK_DIR}/image/$(MFRS)/usrfs.bin -e ${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_spinand_ramdisk.env -c ${SDK_DIR}/tools/linux/mkimage
	${SDK_DIR}/tools/linux/mkramdiskenv.bin -a arm -t 100 -e ${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_spinand_ramdisk.env
endif

##### make usr fs #####
# cramfs
# squashfs
# jffs2
# ubifs
#######################
usrfs:
	@echo "==> make usrfs"
	@mkdir -p ${SDK_DIR}/image/$(MFRS)/usrfs_empty
	@mkdir -p $(USR_FS_DIR)/lib
	@mkdir -p $(USR_FS_DIR)/bin
	@mkdir -p $(USR_FS_DIR)/sbin
	@mkdir -p $(USR_FS_DIR)/etc
	@mkdir -p $(USR_FS_DIR)/libexec
	@mkdir -p $(USR_FS_DIR)/share
	@mkdir -p $(USR_FS_DIR)/stb
	@mkdir -p $(USR_FS_DIR)/stbdata
	mkdir -p $(USR_FS_DIR)/stb/external-ko
	@mkdir -p $(USR_FS_DIR)/stb/wifi
	@mkdir -p $(USR_FS_DIR)/stb/config
	@mkdir -p $(USR_FS_DIR)/stb/avfw

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
	@if [ -e $(TEE_USRFS_DIR) ]; then \
		echo "\033[32m To copy the TEE components to the striped usrfs \033[0m"; \
		if [ -e $(TEE_OPTEE_ARMTZ) ] && [ -e $(USR_FS_DIR)/lib/optee_armtz ]; then \
			rm -rf $(USR_FS_DIR)/lib/optee_armtz; \
		fi; \
		if [ -e $(TEE_USRFS_DIR)/usr/lib/tee-supplicant ] && [ -e $(USR_FS_DIR)/usr/lib/tee-supplicant ]; then \
			rm -rf $(USR_FS_DIR)/usr/lib/tee-supplicant; \
		fi; \
		cp --remove-destination -arf $(TEE_USRFS_DIR)/* $(USR_FS_DIR)/; \
		mkdir -p $(USR_FS_DIR)/sbin; cp --remove-destination -arf $(TEE_SUPPLICANT) $(USR_FS_DIR)/sbin; \
		mkdir -p $(USR_FS_DIR)/lib; cp --remove-destination -arf $(TEE_OPTEE_ARMTZ) $(USR_FS_DIR)/lib; \
		ALL_FILES=`find $(USR_FS_DIR) -type f`; for x in $${ALL_FILES}; do if [ -n "`file $${x} | grep ELF`" ]; then chmod +w $${x}; $(STRIP) $(STRIP_DEBUG) $${x}; chmod -w $${x}; fi; done; \
	else \
		echo "\033[32m Can't find the TEE components usrfs directory \033[0m"; \
	fi;

	#remove unused files
	rm -rf $(USR_FS_DIR)/bin/optee_example_hello_world
	rm -rf $(USR_FS_DIR)/lib/libckteec*
	rm -rf $(USR_FS_DIR)/lib/optee_armtz/*
	rm -rf $(USR_FS_DIR)/lib/tee-supplicant/plugins/*
	rm -rf $(USR_FS_DIR)/sanitize
endif

# montage ko start
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_msp.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_common.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_ciplus.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_vofw.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_vfmw.ko $(USR_FS_DIR)/stb/external-ko
	##-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_slhdr.ko $(USR_FS_DIR)/stb/external-ko
# montage ko end

#
# copy both static none-static needed so
#

#
# FIXME
#
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libavformat.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libavcodec.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libavutil.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libswresample.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libswscale.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libexpat.so*  $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libwpa_client.so $(USR_FS_DIR)/lib -f
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libtinyxml.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfreetype.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpng16.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libxml2.so* $(USR_FS_DIR)/lib -af

	# TODO
	#copy ta - dolby(ac3 + ac4)
	#cp --remove-destination -f $(TEE_AUDIO_TA_DIR)/full/fta/ea38a79f-b962-487c-86d3-b3d1d94383f9.* $(USR_FS_DIR)/lib/optee_armtz/

	#avcpu fw
ifeq ($(CFG_MT_SDK_RELEASE),y)
	cp --remove-destination -f $(AV_BIN_DIR)/av_test_nodolby_sym6_tee_encrypt.bin $(USR_FS_DIR)/stb/avfw/avfw.bin
else
	cp --remove-destination -f $(AV_BIN_DIR)/av_test_wma_sym6_tee_encrypt.bin $(USR_FS_DIR)/stb/avfw/avfw.bin
endif

	@if [ -f $(BIN_DIR_STRIPED)/bin/mt/sample_wb ]; then \
		cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/sample_wb  $(USR_FS_DIR)/stb -f;\
		cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/start.sh  $(USR_FS_DIR)/stb -f;\
	fi
	if [ -f ${SDK_DIR}/product/$(MFRS)/start.sh ]; then\
		cp --remove-destination ${SDK_DIR}/product/$(MFRS)/start.sh $(USR_FS_DIR)/stb -f;\
	fi
ifeq ($(CONFIG_MT_INSTALL_JEMALLOC),y)
	sed -i 's|MT_JEMALLOC="false"|MT_JEMALLOC="true"|g' $(USR_FS_DIR)/stb/start.sh
endif
ifeq ($(CONFIG_MT_GPERFTOOLS_HEAP_PROF),y)
	sed -i 's|MT_GPERFTOOLS_HEAP_PROF="false"|MT_GPERFTOOLS_HEAP_PROF="true"|g' $(USR_FS_DIR)/stb/start.sh
endif
ifeq ($(CONFIG_MT_SANITIZE),y)
	sed -i 's|export MT_SANITIZE="false"|export MT_SANITIZE="true"|g' $(USR_FS_DIR)/stb/start.sh
else ifeq ($(CONFIG_MT_SANITIZE_TAG),y)
	sed -i 's|export MT_SANITIZE_TAG="false"|export MT_SANITIZE_TAG="true"|g' $(USR_FS_DIR)/stb/start.sh
else ifeq ($(CONFIG_MT_SANITIZE_THREAD_RACE),y)
	sed -i 's|export MT_SANITIZE_TSAN="false"|export MT_SANITIZE_TSAN="true"|g' $(USR_FS_DIR)/stb/start.sh
endif

#
# make full features' usrfs image
#
	-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs.bin
	-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs.img

	$(call fun_make_usrfs_image)

	-mv -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin ${SDK_DIR}/image/$(MFRS)/usrfs.bin
	-mv -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img ${SDK_DIR}/image/$(MFRS)/usrfs.img

	##$(MONTAGE_TOOLS_DIR)/mkext4.sh $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs.ext4


##### make data fs #####
# yaffs2
# jffs2
# ubifs
########################
datafs:
	@echo "==> make datafs"
	@echo $(DATA_DIR)
	@mkdir -p $(DATA_DIR)
ifneq (,$(findstring yaffs2,$(DATAFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/datafs.yaffs2
	${SDK_DIR}/tools/linux/mkyaffs2image spinand $(DATA_DIR) ${SDK_DIR}/image/$(MFRS)/datafs.yaffs2 2k GD5F1GQ4UAW -b 0x20000
endif
ifneq (,$(findstring jffs2,$(DATAFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/datafs.jffs2
	${SDK_DIR}/tools/linux/mkfs.jffs2 -r $(DATA_DIR) -o ${SDK_DIR}/image/$(MFRS)/datafs.jffs2 -l -e 0x10000 --pad=$(DATAFS_SIZE_NOR) -s 0x100
	rm -f ${SDK_DIR}/image/$(MFRS)/datafs_nand.jffs2
	${SDK_DIR}/tools/linux/mkfs.jffs2 -r $(DATA_DIR) -o ${SDK_DIR}/image/$(MFRS)/datafs_nand.jffs2 -l -e 0x20000 --pad=$(DATAFS_SIZE_NAND) -s 0x800 -n
endif
ifneq (,$(findstring ubifs,$(DATAFS_TYPE)))
	@echo "make datafs in ubi mode"
	rm -f ${SDK_DIR}/image/$(MFRS)/ubi.img
	rm -f ${SDK_DIR}/image/$(MFRS)/nandubi.img
	# nor flash: 64K * 32 = 2M
	$(MKUBIFS) -r $(DATA_DIR) -m 1 -e 65408 -c 32 -o ${SDK_DIR}/image/$(MFRS)/ubifs.img
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/ubi.img -p 65536 -m 1 -s 1 ${SDK_DIR}/image/$(MFRS)/ubinize.cfg
	rm -f ${SDK_DIR}/image/$(MFRS)/ubifs.img
	# nand flash: 124K * 313 = 38M
	$(MKUBIFS) -r $(DATA_DIR) -F -m 2048 -e 126976 -c 313 -o ${SDK_DIR}/image/$(MFRS)/nandubifs.img
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/nandubi.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/nandubinize.cfg
	rm -f ${SDK_DIR}/image/$(MFRS)/nandubifs.img
endif
ifneq (,$(findstring ext4,$(DATAFS_TYPE)))
	$(MONTAGE_TOOLS_DIR)/mke2fs -F -L '' -N 0 -m 5 -T ext4 -d $(DATA_DIR) ${SDK_DIR}/image/$(MFRS)/datafs.ext4 15M
endif


fs: rootfs usrfs datafs
	cp --remove-destination -f $(BIN_DIR_STRIPED)/bin/mt/uImage ${SDK_DIR}/image/$(MFRS)
	cp --remove-destination -f $(BIN_DIR_STRIPED)/bin/mt/*.dtb ${SDK_DIR}/image/$(MFRS)
	cp --remove-destination -f $(TEE_BIN_DIR)/teeos.bin ${SDK_DIR}/image/$(MFRS)
	cp --remove-destination -f $(TEE_BIN_DIR)/tee.dtb.bin ${SDK_DIR}/image/$(MFRS)
	mkdir -p ${SDK_DIR}/image/$(MFRS)/CHIP_OF
	cp --remove-destination -arf ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/CHIP_OF/OTP_FTA_TEE ${SDK_DIR}/image/$(MFRS)/CHIP_OF


ENVFILE := $(wildcard ${SDK_DIR}/image/$(MFRS)/bootargs/*.env)
ENVSRC := $(patsubst %.env,%.scr,$(ENVFILE))
%.scr: %.env
	MFRS=$(MFRS) AV_BIN_TFTP_DIR=$(AV_BIN_DIR:${SDK_DIR}/%=%) KERNEL_BIN_TFTP_DIR=$(KERNEL_OUTPUT:${SDK_DIR}/%=%)/arch/$(KERNEL_ARCH)/boot ${SDK_DIR}/build/script/tftp-nfs.sh env $<
ifeq ($(CONFIG_MT_INSTALL_GDBSERVER),y)
	sed -i 's|#mt_io setbit 0xbf5a0008 0 1|mt_io setbit 0xbf5a0008 0 1|g' $<
endif
ifeq ($(CONFIG_MT_DIRECTFB),y)
	sed -i 's|#dfb_en=1|dfb_en=1|g' $<
endif
ifeq ($(CONFIG_MT_RAMDISK),y)
	sed -i 's|#ramdisk_flag=1|ramdisk_flag=1|g' $<
endif
	${SDK_DIR}/tools/linux/mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Boot Script" -d $^ $@

XMLFILE := $(wildcard ${SDK_DIR}/image/$(MFRS)/flash/*.xml)
XMLBIN: $(ENVSRC) $(XMLFILE)
	-$(foreach xmlfile,$(XMLFILE),perl ${SDK_DIR}/tools/linux/makeflashbin.pl -c $(xmlfile);)


install: fs ramdisk

bin: XMLBIN

update:

