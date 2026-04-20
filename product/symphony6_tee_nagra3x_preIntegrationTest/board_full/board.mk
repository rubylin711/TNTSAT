include ${SDK_DIR}/build/script/base.mk

ifeq ($(CONFIG_MT_STRIP_DEBUGINFO),y)
export STRIP_DEBUG=--strip-debug
endif

ifeq (${SDK_DIR}/image/$(MFRS)/board_config.mk,$(wildcard ${SDK_DIR}/image/$(MFRS)/board_config.mk))
	include ${SDK_DIR}/image/$(MFRS)/board_config.mk
endif
ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
	include ${SDK_DIR}/product/chicago/usrfs_gstreamer.mk
endif

KERNELRELEASE ?= $(shell cat $(KERNEL_OUTPUT)/include/config/kernel.release 2> /dev/null)

.PHONY: all install fs ramdisk XMLBIN bin update rootfs update_fpga usrfs datafs help

all: help

help:
	@echo "valid target:"
	@echo "install"

#
# define make usrfs function
# [out]: usrfs_tmp.bin, usrfs_tmp.img
#
# for ubifs: 124K * 400 = 48M
#
define fun_make_usrfs_image
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_empty.bin
	@-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_squashfs.img
	#mksquashfs
	@${SDK_DIR}/tools/linux/mksquashfs $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin -comp xz || exit 1
	@$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/usrfs_squashubi.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/usrfs_ubinize.cfg || exit 1
	@if [ $(USERFS_TYPE) = cramfs ]; then \
		rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin || exit 1; \
		${SDK_DIR}/tools/linux/mkfs.cramfs $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin || exit 1; \
		$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/usrfs_ubinize.cfg || exit 1; \
		${SDK_DIR}/tools/linux/mkfs.cramfs ${SDK_DIR}/image/$(MFRS)/usrfs_empty ${SDK_DIR}/image/$(MFRS)/usrfs_empty.bin || exit 1; \
	else \
		${SDK_DIR}/tools/linux/mksquashfs ${SDK_DIR}/image/$(MFRS)/usrfs_empty ${SDK_DIR}/image/$(MFRS)/usrfs_empty.bin -comp xz || exit 1; \
	fi
	@if [ $(USERFS_TYPE) = squashfs ]; then \
		cp -f ${SDK_DIR}/image/$(MFRS)/usrfs_squashubi.img ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img || exit 1; \
	fi
	@if [ $(USERFS_TYPE) = jffs2 ]; then \
		${SDK_DIR}/tools/linux/mkfs.jffs2 -r $(USR_FS_DIR) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -l -e 0x10000 --pad=$(USERFS_SIZE) -s 0x100 || exit 1; \
	fi
	@if [ $(USERFS_TYPE) = ubifs ]; then \
		$(MKUBIFS) -r $(USR_FS_DIR) -F -m 2048 -e 126976 -c 600 -o ${SDK_DIR}/image/$(MFRS)/user_nandubifs.img || exit 1; \
		$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/user_nandubinize.cfg || exit 1; \
		rm -f ${SDK_DIR}/image/$(MFRS)/user_nandubifs.img || exit 1; \
	fi
endef

##### make root fs #####
# initramfs
# cramfs
# squashfs
# ubifs
# ext4
#######################
rootfs:
	@echo "==> make rootfs"
	mkdir -p ${SDK_DIR}/image/$(MFRS)
	rm -rf ${SDK_DIR}/image/$(MFRS)/rootfs
	cp --remove-destination -arf $(MONTAGE_ROOTFS_STRIPED) ${SDK_DIR}/image/$(MFRS)/rootfs
	sed -i 's|APP_NAME=.*|APP_NAME=mt_sample|g' ${SDK_DIR}/image/$(MFRS)/rootfs/root/.profile
	MFRS=$(MFRS) AV_BIN_TFTP_DIR=$(AV_BIN_DIR:${SDK_DIR}/%=%) KERNEL_BIN_TFTP_DIR=$(KERNEL_OUTPUT:${SDK_DIR}/%=%)/arch/$(KERNEL_ARCH)/boot ${SDK_DIR}/build/script/tftp-nfs.sh rcS
	mkdir -p ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents modules.* ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents updates ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/regulator/symphony4-regulator.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/net/ethernet/montage/sym6_gmac.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/cdns3/cdns3-symphony6.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/host/ehci-platform.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/host/ehci-hcd.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/phy/phy-generic.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/legacy/g_mass_storage.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/function/usb_f_ss_lb.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/legacy/g_zero.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/function/usb_f_mass_storage.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/mmc/host/sdhci-mt.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/serial/usbserial.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/serial/ftdi_sio.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
ifeq ($(CONFIG_MT_MALI_SUPPORT),y)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/gpu/arm/midgard/mali_kbase.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/base/arm/dma_buf_test_exporter/dma-buf-test-exporter.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/base/arm/memory_group_manager/memory_group_manager.ko  ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
else
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)/kernel/drivers/base/arm/dma_buf_test_exporter/dma-buf-test-exporter.ko
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)/kernel/drivers/base/arm/memory_group_manager/memory_group_manager.ko
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)/kernel/drivers/gpu/arm/midgard/mali_kbase.ko
endif
	-cp --remove-destination -af ${SDK_DIR}/image/$(MFRS)/usb/usbmodeswitch.sh ${SDK_DIR}/image/$(MFRS)/rootfs/sbin
	-cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents kernel/drivers/usb/gadget/udc/mt_usb_udc.ko ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	-cp --remove-destination -arf $(MODULE_DIR_STRIPED)/lib/firmware ${SDK_DIR}/image/$(MFRS)/rootfs/lib
ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
	@echo "prepare lxc environment in rootfs"
	@echo "prepare lxc environment in rootfs done!"
endif
	@echo "you can copy something into ${SDK_DIR}/image/$(MFRS)/rootfs"
	#TODO...
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
	# nand flash(ubi block)
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/rootfs_squashubi.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/rootfs_ubinize.cfg
endif
ifneq (,$(findstring ubifs,$(ROOTFS_TYPE)))
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs.img
	rm -f ${SDK_DIR}/image/$(MFRS)/rootubifs.img
	# 124K * 400 = 48M
	$(MKUBIFS) -r ${SDK_DIR}/image/$(MFRS)/rootfs -F -m 2048 -e 126976 -c 400 -o ${SDK_DIR}/image/$(MFRS)/rootubifs.img
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/rootfs.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/ubifs_rootfs_ubinize.cfg
endif
ifneq (,$(findstring ext4,$(ROOTFS_TYPE)))
	$(MONTAGE_TOOLS_DIR)/mkext4.sh ${SDK_DIR}/image/$(MFRS)/rootfs ${SDK_DIR}/image/$(MFRS)/rootfs.ext4
endif

ramdisk:
	${SDK_DIR}/build/script/ramdiskenvcfg.sh ${SDK_DIR}/out/general/kernel/linux-x.y.z/System.map ${SDK_DIR}/image/$(MFRS)/rootfs.squashfs ${SDK_DIR}/image/$(MFRS)/usrfs.bin ${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_unified_512M.env 
	${SDK_DIR}/build/script/ramdiskenvcfg.sh ${SDK_DIR}/out/general/kernel/linux-x.y.z/System.map ${SDK_DIR}/image/$(MFRS)/rootfs.squashfs ${SDK_DIR}/image/$(MFRS)/usrfs.bin ${SDK_DIR}/image/$(MFRS)/bootargs/uboot_symphony_demo_unified.env

##### make usr fs #####
# cramfs
# squashfs
# jffs2
# ubifs
# ext4
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
	@mkdir -p $(USR_FS_DIR)/stb/res
	@mkdir -p $(USR_FS_DIR)/stbdata
	mkdir -p $(USR_FS_DIR)/stb/external-ko
	@mkdir -p $(USR_FS_DIR)/stb/wifi
	@mkdir -p $(USR_FS_DIR)/stb/config
	@mkdir -p $(USR_FS_DIR)/stb/avfw
	@mkdir -p $(USR_FS_DIR)/stb/fw

#
# copy both static none-static needed so
#

ifneq ($(CFG_MT_STATIC_LINK),y)
#
# copy sample_wb depended so first for mini usrfs
#
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libexpat.so*  $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libcupnp.so  $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libsatip.so  $(USR_FS_DIR)/lib -af
#ifeq ($(BOARD_WIFI_SUPPORT),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmtwlan.so $(USR_FS_DIR)/lib -f
#ifeq ($(CONFIG_MT_MIRACAST_SUPPORT), y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmtwfd.so $(USR_FS_DIR)/lib -f
#endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libwpa_client.so $(USR_FS_DIR)/lib -f
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libnl-route-3.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libreadline.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libncurses.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmtusbdev.so $(USR_FS_DIR)/lib -af
#endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmtlzavfilter.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libBento4.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libosal.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmutils.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmonplayer_demux.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libTsSeq.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libsuplayer.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libxml2.so* $(USR_FS_DIR)/lib -af
ifeq ($(CONFIG_MT_ENABLE_MSS_PLAYER),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmsssuplayer.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libAvPlayInstance.so $(USR_FS_DIR)/lib -af
endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_common.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_msp.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_mtgo.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_sci.so $(USR_FS_DIR)/lib -af
ifeq ($(CONFIG_MT_CIPHER_SUPPORT),y)
ifeq ($(CFG_MT_CHIP),symphony1)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_cipher.so $(USR_FS_DIR)/lib -af
else
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_mss.so $(USR_FS_DIR)/lib -af
endif
endif

ifeq ($(CONFIG_MT_DRM_SUPPORT),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcurl.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libidn2.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libunistring.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcares.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(EXTERNAL_BINARY_PREFIX_DIR)/drm/data/* $(USR_FS_DIR)/stb -af
endif

ifeq ($(CONFIG_MT_MTGO_JPEG_SUPPORT),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_jpeg.so $(USR_FS_DIR)/lib -af
endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_tde.so $(USR_FS_DIR)/lib -af
ifeq ($(CFG_FFMPEG_VER_NUM),422)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libavformat.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libavcodec.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libavutil.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libswresample.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libswscale.so* $(USR_FS_DIR)/lib -af
endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_ttx.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_fastplayer.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_os.so $(USR_FS_DIR)/lib -af
	-@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libautotest.so $(USR_FS_DIR)/lib -af
ifeq ($(CONFIG_MT_LIBDOWNLOAD),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libRSSClient.so $(USR_FS_DIR)/lib -af
endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/liblzma.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libtinyxml.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_testfm.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfreetype.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_cc.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_subtitle.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_subtoutput.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_png.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpng.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpng16.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libPlaybackSeq.so $(USR_FS_DIR)/lib -af
ifeq ($(CONFIG_MT_INSTRUMENT_FUNCTIONS),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libinstrument_function-mt.so $(USR_FS_DIR)/lib -af
endif
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libusbcam_route_ts.so $(USR_FS_DIR)/lib -af

endif

	@cp --remove-destination $(SDK_DIR)/tee_bsp/arm64/usr/lib/libsmpc* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SDK_DIR)/tee_bsp/arm64/usr/lib/libteec* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_nocsapi_impl.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_nocsapi_impl_talts.so $(USR_FS_DIR)/lib -af
#
# copy mali ddk lib
#
ifeq ($(CONFIG_MT_MALI_SUPPORT), y)
	@echo "==> make CONFIG_MT_MALI_SUPPORT "
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libEGL.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libGLESv1_CM.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libGLESv2.so* $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmali.so* $(USR_FS_DIR)/lib -af
else
	rm -f $(USR_FS_DIR)/lib/libEGL.so*
	rm -f $(USR_FS_DIR)/lib/libGLESv1_CM.so*
	rm -f $(USR_FS_DIR)/lib/libGLESv2.so*
	rm -f $(USR_FS_DIR)/lib/libmali.so*
endif

ifeq ($(CONFIG_MT_VDEC_LCEVC_SUPPORT), y)
	@echo "==> make CONFIG_MT_VDEC_LCEVC_SUPPORT "
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libdvp_residuals.so $(USR_FS_DIR)/lib -af
else
	rm -f $(USR_FS_DIR)/lib/libdvp_residuals.so
endif
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
		mkdir -p $(USR_FS_DIR)/lib; cp --remove-destination -arf $(TEE_OPTEE_ARMTZ) $(USR_FS_DIR)/lib; \
		ALL_FILES=`find $(USR_FS_DIR) -type f`; for x in $${ALL_FILES}; do if [ -n "`file $${x} | grep ELF`" ]; then chmod +w $${x}; $(STRIP) $(STRIP_DEBUG) $${x}; chmod -w $${x}; fi; done; \
	else \
		echo "\033[32m Can't find the TEE components usrfs directory \033[0m"; \
	fi;
endif

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
	#copy ta - dolby(ac3 + ac4)
	cp --remove-destination -f $(TEE_AUDIO_TA_DIR)/full/nagra/ea38a79f-b962-487c-86d3-b3d1d94383f9.* $(USR_FS_DIR)/lib/optee_armtz/
endif
ifeq ($(CFG_MT_SDK_RELEASE),y)
	#avcpu fw - no dolby, no wma
	cp --remove-destination -f $(AV_BIN_DIR)/av_test_nodolby_sym6_tee_encrypt.bin $(USR_FS_DIR)/stb/avfw/avfw.bin
else
	#avcpu fw - wma
	cp --remove-destination -f $(AV_BIN_DIR)/av_test_wma_sym6_tee_encrypt.bin $(USR_FS_DIR)/stb/avfw/avfw.bin
endif

ifeq ($(CONFIG_MT_GSTPLAYER_ENABLE),y)
	$(call make_gstreamer_usrfs)
endif

# montage ko start
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_msp.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_common.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_ciplus.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_vofw.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_vfmw.ko $(USR_FS_DIR)/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_slhdr.ko $(USR_FS_DIR)/stb/external-ko
# montage ko end

# linux-fusion start

ifeq ($(CONFIG_LINUX_FUSION),y)
	cp --remove-destination -arf $(MODULE_DIR_STRIPED)/external-ko/fusion $(USR_FS_DIR)/stb/external-ko
endif
# linux-fusion end

ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
	@echo "prepare lxc environment in usrfs"
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/caller $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/caller_mp1 $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/caller_mp2 $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/caller_mp3 $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/caller_mpfs $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/stress-client $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/listener1 $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/listener2 $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/listener3 $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/listener_mp $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/listener_mpfs $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/listener_mpfs.sh $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/usb-storage-daemon $(USR_FS_DIR)/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/usb-storage-daemon.sh $(USR_FS_DIR)/bin
ifeq ($(CONFIG_MT_INSTALL_JEMALLOC),y)
	sed -i 's|MT_JEMALLOC="false"|MT_JEMALLOC="true"|g' $(USR_FS_DIR)/bin/listener_mpfs.sh
endif
ifeq ($(CONFIG_MT_GPERFTOOLS_HEAP_PROF),y)
	sed -i 's|MT_GPERFTOOLS_HEAP_PROF="false"|MT_GPERFTOOLS_HEAP_PROF="true"|g' $(USR_FS_DIR)/bin/listener_mpfs.sh
endif
ifeq ($(CONFIG_MT_SANITIZE),y)
	sed -i 's|export MT_SANITIZE="false"|export MT_SANITIZE="true"|g' $(USR_FS_DIR)/bin/listener_mpfs.sh
else ifeq ($(CONFIG_MT_SANITIZE_TAG),y)
	sed -i 's|export MT_SANITIZE_TAG="false"|export MT_SANITIZE_TAG="true"|g' $(USR_FS_DIR)/bin/listener_mpfs.sh
else ifeq ($(CONFIG_MT_SANITIZE_THREAD_RACE),y)
	sed -i 's|export MT_SANITIZE_TSAN="false"|export MT_SANITIZE_TSAN="true"|g' $(USR_FS_DIR)/bin/listener_mpfs.sh
endif
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/mt/stress-server $(USR_FS_DIR)/bin
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/mt/liblxc_ipc.so $(USR_FS_DIR)/lib
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/mt/libmt_unf_ipc_fs.so $(USR_FS_DIR)/lib
	@echo "prepare lxc environment in usrfs done"
endif

	@if [ -f $(BIN_DIR_STRIPED)/bin/mt/sample_wb ]; then \
		cp --remove-destination -af $(RESOURCE_DIR)/arial.ttf $(USR_FS_DIR)/stb -f;\
		#cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/sample_wb  $(USR_FS_DIR)/stb -f;\
		cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/start.sh  $(USR_FS_DIR)/stb -f;\
	fi

	@if [ -f $(BIN_DIR_STRIPED)/bin/mt/mt_sample ]; then \
		cp --remove-destination $(SAMPLE_DIR)/cc/res/DroidSansFallbackLegacy.ttf $(USR_FS_DIR)/stb/res -f;\
		cp --remove-destination $(SAMPLE_DIR)/*.info $(USR_FS_DIR)/stb/res -f;\
		cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/mt_sample  $(USR_FS_DIR)/stb -f;\
	fi

	if [ -f $(BIN_DIR_STRIPED)/bin/mt/audio_ta_service ]; then \
		cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/audio_ta_service $(USR_FS_DIR)/stb -f;\
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
	@cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/dlna_daemon $(USR_FS_DIR)/bin -af

ifeq ($(CONFIG_MT_VDEC_LCEVC_SUPPORT),y)
	sed -i 's|ulimit_size="512"|ulimit_size="1024"|g' $(USR_FS_DIR)/stb/start.sh
endif

#
# then copy sample_wb not depended so and wifi ko, etc.
#
ifneq ($(CFG_MT_STATIC_LINK),y)
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libnl-3.so*  $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libnl-genl-3.so*  $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmt_sample_common.so $(USR_FS_DIR)/lib -af
endif

ifeq ($(CONFIG_MT_USB3G_SUPPORT),y)
	-cp --remove-destination ${BIN_DIR_STRIPED}/sbin/chat $(USR_FS_DIR)/bin
	-cp --remove-destination ${BIN_DIR_STRIPED}/sbin/pppd $(USR_FS_DIR)/bin
	-cp --remove-destination ${SDK_DIR}/component/wifi/tools/libusb_modeswitch_ppp/usb3g_ppp_dial/* $(USR_FS_DIR)/stb
endif

ifeq ($(BOARD_USBCI_SUPPORT),y)
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/smit.ko $(USR_FS_DIR)/stb/external-ko
endif

ifeq ($(BOARD_WIFI_SUPPORT),y)
ifneq (,$(findstring rtl8188ftv,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/8188fu.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring rtl8188eu,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/8188eu.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring m88wi6700u,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/m88wi6700u.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring mt7601u,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt7601Usta.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring rtl8192fu,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/8192fu.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring rtl8189fs,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/8189fs.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring rtl8723ds,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/8723ds.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring rtl8821cu,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/8821cu.ko $(USR_FS_DIR)/stb/external-ko
endif
ifneq (,$(findstring atbmwifi,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -arf $(MODULE_DIR_STRIPED)/external-ko/hal_apollo $(USR_FS_DIR)/stb/external-ko/
endif
ifneq (,$(findstring bl602,$(BOARD_WIFI_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/bl_fdrv.ko $(USR_FS_DIR)/stb/external-ko
	-@cp --remove-destination $(MODULE_DIR_STRIPED)/firmware/wholeimg_if.bin $(USR_FS_DIR)/stb/fw -af
	-@cp --remove-destination $(MODULE_DIR_STRIPED)/firmware/bl_caldata.bin $(USR_FS_DIR)/stb/fw -af
endif
	#@cp --remove-destination $(BIN_DIR_STRIPED)/dlna_dmr $(USR_FS_DIR)/wifi -af
	@cp --remove-destination $(BIN_DIR_STRIPED)/sbin/wpa_supplicant $(USR_FS_DIR)/bin -f
	@cp --remove-destination $(BIN_DIR_STRIPED)/sbin/wpa_cli $(USR_FS_DIR)/bin -f
	#@cp --remove-destination $(BIN_DIR_STRIPED)/mtwlan_demo $(USR_FS_DIR)/wifi -f
endif

#install bt module driver
ifneq (,$(findstring realtek,$(BOARD_BT_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/hci_uart.ko $(USR_FS_DIR)/stb/external-ko/
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/rtk_btusb.ko $(USR_FS_DIR)/stb/external-ko/
	-cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/rtk_hciattach $(USR_FS_DIR)/bin -af
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/firmware/rtl*_fw $(USR_FS_DIR)/stb/fw
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/firmware/rtl*_config $(USR_FS_DIR)/stb/fw
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/firmware/rtlbt/rtl*_fw $(USR_FS_DIR)/stb/fw
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/firmware/rtlbt/rtl*_config $(USR_FS_DIR)/stb/fw		
endif

ifneq (,$(findstring aic8800usb,$(BOARD_BT_MODULE)))
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/aic_btusb.ko $(USR_FS_DIR)/stb/external-ko/
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/firmware/aic8800DC/*.bin $(USR_FS_DIR)/stb/fw
endif

#install bluez
ifeq ($(BOARD_BLUEZ_SUPPORT),y)
	@echo "==> install bluez userfs"
	@mkdir -p $(USR_FS_DIR)/share/dbus-1
	-cp --remove-destination $(BIN_DIR_STRIPED)/bin/l2ping $(USR_FS_DIR)/bin -af
	-cp --remove-destination $(BIN_DIR_STRIPED)/bin/dbus-daemon $(USR_FS_DIR)/bin -af
	-cp --remove-destination $(BIN_DIR_STRIPED)/bin/bluetoothd $(USR_FS_DIR)/bin -af
	-cp --remove-destination $(BIN_DIR_STRIPED)/bin/bluetoothctl $(USR_FS_DIR)/bin -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libdbus-1.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libglib-2.0.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgobject-2.0.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgio-2.0.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgmodule-2.0.so* $(USR_FS_DIR)/lib -af		
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libreadline.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libncurses.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libbluetooth.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libffi.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libasound.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpcre.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libtinfo.so* $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libmtbt.so $(USR_FS_DIR)/lib -af
	-cp --remove-destination $(BIN_DIR)/system.conf $(USR_FS_DIR)/share/dbus-1 -af	
	-cp --remove-destination $(BIN_DIR)/bluetooth.sh $(USR_FS_DIR)/stb/bluetooth.sh
	-chmod +x $(USR_FS_DIR)/stb/bluetooth.sh
endif

	@cp --remove-destination -af $(BIN_DIR_STRIPED)/sbin/bonnie++ $(USR_FS_DIR)/bin/bonnie++

	if [ -n "`grep "CONFIG_SOUND=y" $(KERNEL_OUTPUT)/.config`" -o -n "`grep "CONFIG_SOUND=m" $(KERNEL_OUTPUT)/.config`" ]; then \
		cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/aplay $(USR_FS_DIR)/bin/; \
		cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/libasound.so* $(USR_FS_DIR)/lib; \
	fi


#
# install python
#
ifeq ($(CONFIG_MT_PYTHON),y)
	@echo "intall python into usrfs"
	cp -af --remove-destination $(BIN_DIR_STRIPED)/bin/python* $(USR_FS_DIR)/bin
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpython*.so* $(USR_FS_DIR)/lib
	cp -arf --remove-destination $(SHARED_LIB_DIR_STRIPED)/python* $(USR_FS_DIR)/lib
	-cp -arf --remove-destination $(SHARED_LIB_DIR_STRIPED)/gobject-introspection $(USR_FS_DIR)/lib
	-cd $(BIN_DIR_STRIPED)/bin/; cp -arf --remove-destination --parents `find -type f | xargs file | grep "Python script" | awk -F':' '{print $$1}'` $(USR_FS_DIR)/bin
endif

ifeq ($(CONFIG_MT_KODIHUB),y)
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libbz2.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libz.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/liblzma.so* $(USR_FS_DIR)/lib

	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpcre.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpcrecpp.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpcreposix.so* $(USR_FS_DIR)/lib

	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libffi.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libglib-2.0.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfontconfig.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfribidi.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libharfbuzz*.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libass.so* $(USR_FS_DIR)/lib

	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libidn2.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libunistring.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcrypto.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libssl.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libnghttp2.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcares.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcurl.so* $(USR_FS_DIR)/lib

	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libsqlite3.so* $(USR_FS_DIR)/lib
	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libtinyxml2.so* $(USR_FS_DIR)/lib
	#cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfmt.so* $(USR_FS_DIR)/lib

	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libnspr4.so* $(USR_FS_DIR)/lib
endif


#
# make full features' usrfs image
#
	-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs.bin
	-rm -f ${SDK_DIR}/image/$(MFRS)/usrfs.img

	$(call fun_make_usrfs_image)

	-mv -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin ${SDK_DIR}/image/$(MFRS)/usrfs.bin
	-mv -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img ${SDK_DIR}/image/$(MFRS)/usrfs.img

	$(MONTAGE_TOOLS_DIR)/mkext4.sh $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs.ext4

#
# make gst's usr fs
#
ifeq ($(BOARD_GST_SUPPORT),y)
	@echo "==> make gst's usrfs"
	@rm $(USR_FS_DIR)/stb/sample_wb
	@rm $(USR_FS_DIR)/stb/start.sh
	@if [ -f $(BIN_DIR_STRIPED)/bin/mt/sample_wb_gst ]; then \
		cp --remove-destination -af $(RESOURCE_DIR)/arial.ttf $(USR_FS_DIR)/stb -f;\
		#cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/sample_wb_gst  $(USR_FS_DIR)/stb -f;\
		cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/startgst.sh  $(USR_FS_DIR)/stb/start.sh -f;\
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
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libAvPlayInstance.so $(USR_FS_DIR)/lib -af
	@cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libgstsuplayer.so $(USR_FS_DIR)/lib -af

	rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_gst.bin
	rm -f ${SDK_DIR}/image/$(MFRS)/usrfs_gst.img

	$(call fun_make_usrfs_image)

	-mv -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.bin ${SDK_DIR}/image/$(MFRS)/usrfs_gst.bin
	-mv -f ${SDK_DIR}/image/$(MFRS)/usrfs_tmp.img ${SDK_DIR}/image/$(MFRS)/usrfs_gst.img

	$(MONTAGE_TOOLS_DIR)/mkext4.sh $(USR_FS_DIR) ${SDK_DIR}/image/$(MFRS)/usrfs_gst.ext4


endif




##### make data fs #####
# yaffs2
# jffs2
# ubifs
# ext4
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
	# nand flash: 124K * 200 = 24M
	$(MKUBIFS) -r $(DATA_DIR) -F -m 2048 -e 126976 -c 200 -o ${SDK_DIR}/image/$(MFRS)/nandubifs.img
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/nandubi.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/nandubinize.cfg
	rm -f ${SDK_DIR}/image/$(MFRS)/nandubifs.img
endif
ifneq (,$(findstring ext4,$(DATAFS_TYPE)))
	$(MONTAGE_TOOLS_DIR)/mke2fs -F -L '' -N 0 -m 5 -T ext4 -d $(DATA_DIR) ${SDK_DIR}/image/$(MFRS)/datafs.ext4 64M
endif


fs: rootfs usrfs datafs
	cp --remove-destination -f $(BIN_DIR_STRIPED)/bin/mt/uImage ${SDK_DIR}/image/$(MFRS)
	cp --remove-destination -f $(BIN_DIR_STRIPED)/bin/mt/*.dtb ${SDK_DIR}/image/$(MFRS)
	#cp --remove-destination -f $(AV_BIN_DIR)/av_test_sym6.bin ${SDK_DIR}/image/$(MFRS)/av_cpu_sym6.bin
	#-cp --remove-destination -f $(AV_BIN_DIR)/av_test_sym6_encrypt.bin ${SDK_DIR}/image/$(MFRS)/av_cpu_sym6_encrypt.bin
ifeq ($(CONFIG_MT_FPGA),y)
	cp --remove-destination -arf ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/FPGA ${SDK_DIR}/image/$(MFRS)
else
	#mkdir -p ${SDK_DIR}/image/$(MFRS)/CHIP_OF
	#cp --remove-destination -arf ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/CHIP_OF/OTP_FTA_TEE ${SDK_DIR}/image/$(MFRS)/CHIP_OF
	cp --remove-destination -arf ${SDK_DIR}/tee_bsp/all_pss.img ${SDK_DIR}/image/$(MFRS)/update
endif


ENVFILE := $(wildcard ${SDK_DIR}/image/$(MFRS)/bootargs/*.env)
ENVSRC := $(patsubst %.env,%.scr,$(ENVFILE))
%.scr: %.env
	MFRS=$(MFRS) AV_BIN_TFTP_DIR=$(AV_BIN_DIR:${SDK_DIR}/%=%) KERNEL_BIN_TFTP_DIR=$(KERNEL_OUTPUT:${SDK_DIR}/%=%)/arch/$(KERNEL_ARCH)/boot ${SDK_DIR}/build/script/tftp-nfs.sh env $<
ifeq ($(CONFIG_MT_INSTALL_GDBSERVER),y)
	sed -i 's|#mt_io setbit 0xbf5a0008 0 1|mt_io setbit 0xbf5a0008 0 1|g' $<
endif
ifeq ($(CONFIG_MT_VDEC_PIP_SUPPORT),y)
	sed -i 's|#pip_en=1|pip_en=1|g' $<
endif
ifeq ($(CONFIG_MT_VDEC_4KPIP_SUPPORT),y)
	sed -i 's|#pip4k_en=1|pip4k_en=1|g' $<
endif
ifeq ($(CONFIG_MT_DIRECTFB),y)
	sed -i 's|#dfb_en=1|dfb_en=1|g' $<
endif
ifeq ($(CONFIG_MT_RAMDISK),y)
	sed -i 's|#ramdisk_flag=1|ramdisk_flag=1|g' $<
endif
	@logo_file_size="`stat -c "%s" logo.jpg`"; logo_size_plus="`expr $${logo_file_size} + 1048576`"; sed -i 's|logo_size=0x800000|logo_size='$${logo_size_plus}'|g' $<; echo "logo size: $${logo_size_plus}"
	@uimage_file_size="`stat -c "%s" uImage`"; uimage_size_plus="`expr $${uimage_file_size} + 1048576`"; sed -i 's|kimage_size=0x2000000|kimage_size='$${uimage_size_plus}'|g' $<; echo "kimage size: $${uimage_size_plus}"
	${SDK_DIR}/tools/linux/mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Boot Script" -d $^ $@

XMLFILE := $(wildcard ${SDK_DIR}/image/$(MFRS)/flash/*.xml)
XMLBIN: $(ENVSRC) $(XMLFILE)
	-$(foreach xmlfile,$(XMLFILE),perl ${SDK_DIR}/tools/linux/makeflashbin.pl -c $(xmlfile);)


install: fs ramdisk

bin: XMLBIN

define fun_update_common
	rm -rf ${SDK_DIR}/image/$(MFRS)/update
	mkdir -p ${SDK_DIR}/image/$(MFRS)/update
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/logo.jpg ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/uImage ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/*.dtb ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/rootfs.img ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/usrfs.img ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/nandubi.img ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/rootfs.squashfs ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/rootfs_squashubi.img ${SDK_DIR}/image/$(MFRS)/update/
	-cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/usrfs.bin ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/usrfs_squashubi.img ${SDK_DIR}/image/$(MFRS)/update/
endef

#FPGA
update_fpga:
	$(call fun_update_common)

	######## loader ########
	@if [ -f uImage-loader ]; then \
		cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/uImage-loader ${SDK_DIR}/image/$(MFRS)/update; \
	fi
	@if [ -f loader_rootfs_squashubi.img ]; then \
		cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/loader_rootfs_squashubi.img ${SDK_DIR}/image/$(MFRS)/update; \
	fi

	cp --remove-destination -f $(TEE_BIN_DIR)/* ${SDK_DIR}/image/$(MFRS)/update/

ifeq ($(CONFIG_MT_FPGA),y)
	cp --remove-destination -arf ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/FPGA ${SDK_DIR}/image/$(MFRS)/update/
else
	#mkdir -p ${SDK_DIR}/image/$(MFRS)/update/CHIP_OF
	#cp --remove-destination -arf ${SDK_DIR}/tools/prebuilts/$(CFG_MT_CHIP)/CHIP_OF/OTP_FTA_TEE ${SDK_DIR}/image/$(MFRS)/update/CHIP_OF
	cp --remove-destination -arf ${SDK_DIR}/tee_bsp/all_pss.img ${SDK_DIR}/image/$(MFRS)/update/
endif
	#cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/av_cpu_sym6.bin ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/bootargs/*.scr ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/*.vbs ${SDK_DIR}/image/$(MFRS)/update/
	# EMMC
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/rootfs.ext4 ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/usrfs.ext4 ${SDK_DIR}/image/$(MFRS)/update/
	cp --remove-destination -f ${SDK_DIR}/image/$(MFRS)/datafs.ext4 ${SDK_DIR}/image/$(MFRS)/update/
	cp -af $(BIN_DIR_STRIPED)/bin/mt/nagra_simbad_master ${SDK_DIR}/image/$(MFRS)/update/
	cp -af $(BIN_DIR_STRIPED)/bin/mt/nagra_talts_master ${SDK_DIR}/image/$(MFRS)/update/
	cp -af $(BIN_DIR_STRIPED)/bin/mt/nagra_talts_worker ${SDK_DIR}/image/$(MFRS)/update/

	rm -rf ${SDK_DIR}/image/$(MFRS)/update_fpga
ifeq ($(CONFIG_MT_FPGA),y)
	mv -f ${SDK_DIR}/image/$(MFRS)/update ${SDK_DIR}/image/$(MFRS)/update_fpga
endif

update: update_fpga

