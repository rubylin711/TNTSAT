include ${SDK_DIR}/build/script/base.mk

ifeq (${SDK_DIR}/image/$(MFRS)/board_config.mk,$(wildcard ${SDK_DIR}/image/$(MFRS)/board_config.mk))
	include ${SDK_DIR}/image/$(MFRS)/board_config.mk
endif

.PHONY: all rootfs

all: rootfs

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

	mkdir -p ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents modules.* ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)
	cd $(MODULE_DIR_STRIPED)/lib/modules/$(CFG_MT_KERNEL_VERSION); cp --remove-destination -arf --parents updates ${SDK_DIR}/image/$(MFRS)/rootfs/lib/modules/$(CFG_MT_KERNEL_VERSION)

	mkdir -p ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/external-ko
	-cp --remove-destination -af $(MODULE_DIR_STRIPED)/external-ko/mt_vofw.ko ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/external-ko

	mkdir -p ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/res
	cp --remove-destination $(SAMPLE_DIR)/cc/res/DroidSansFallbackLegacy.ttf ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/res/ -af
	cp --remove-destination $(SAMPLE_DIR)/*.info ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/res/ -af
	cp --remove-destination $(BIN_DIR_STRIPED)/bin/mt/mt_sample ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/ -af
	cp --remove-destination ${SDK_DIR}/product/$(MFRS)/start.sh ${SDK_DIR}/image/$(MFRS)/rootfs/usr/local/stb/ -af

	cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfreetype.so* ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/ -af
	cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpng16.so* ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/ -af
	cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libxml2.so* ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/ -af

	# rm unused files
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libanl.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libasan.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libatomic.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libBrokenLocale.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libcrypt.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libcrypto.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libdl.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libgcc_s.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libgfortran.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libgomp.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libitm.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libm.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libnss*.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libpthread*.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libresolv.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/librt.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libssl.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libstdc++.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libthread_db.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libubsan.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libutil.so*
	#rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libz.so*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libanl*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libBrokenLocale*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libcrypt*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libmemusage.so
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libnsl*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libnss*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libpcprofile.so
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libSegFault.so
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libthread_db*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libblkid.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/lib/libuuid.so*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libext2fs.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libcom_err.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libe2p.so*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/lib/libntfs-3g.so*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/mke2fs
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/etc/mke2fs.conf
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/mkfs.*
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/e2fsck
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/sbin/fsck*

	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/sbin/mkntfs
	rm -f ${SDK_DIR}/image/$(MFRS)/rootfs/usr/sbin/mkfs.*

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
	# 124K * 100 = 12M
	$(MKUBIFS) -r ${SDK_DIR}/image/$(MFRS)/rootfs -F -m 2048 -e 126976 -c 100 -o ${SDK_DIR}/image/$(MFRS)/rootubifs.img
	$(MKUBIIMG) -o ${SDK_DIR}/image/$(MFRS)/rootfs.img -p 128KiB -m 2KiB -s 2KiB ${SDK_DIR}/image/$(MFRS)/ubifs_rootfs_ubinize.cfg
endif
ifneq (,$(findstring ext4,$(ROOTFS_TYPE)))
	#$(MONTAGE_TOOLS_DIR)/mkext4.sh ${SDK_DIR}/image/$(MFRS)/rootfs ${SDK_DIR}/image/$(MFRS)/rootfs.ext4
	$(MONTAGE_TOOLS_DIR)/mke2fs -F -L '' -N 0 -m 1 -T ext4 -d rootfs rootfs.ext4 16M
endif
