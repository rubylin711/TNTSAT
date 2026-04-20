include ${SDK_DIR}/build/script/base.mk

ifeq ($(CFG_MT_BUILD_LOADER),y)
CONFIG_MT_UDEV := n
CONFIG_MT_MDEV := y
endif
export SHARED_LIB_DIR_STRIPED BIN_DIR_STRIPED MODULE_DIR_STRIPED MONTAGE_ROOTFS_STRIPED DEBUG_INFO_DIR
export STRIP OBJCOPY READELF
export STRIP_KERNEL
ifeq ($(CONFIG_MT_STRIP_DEBUGINFO),y)
  ifneq ($(CFG_MT_BUILD_LOADER),y)
    export STRIP_DEBUG=--strip-debug
  endif
endif
ifeq ($(CONFIG_MT_INSTALL_DEBUG_TOOLS_INTO_USB_STORAGE),y)
DEBUG_TOOLS_INSTALL_PATH=${SDK_DIR}/debug_tools_install_path
else
DEBUG_TOOLS_INSTALL_PATH=$(MONTAGE_ROOTFS_STRIPED)
endif

.PHONY: install uninstall rootfs initramfs

install:
	mkdir -p ${SHARED_LIB_DIR_STRIPED}/mt
	mkdir -p ${BIN_DIR_STRIPED}/bin/mt
	rm -rf ${MODULE_DIR_STRIPED}
	mkdir -p ${MODULE_DIR_STRIPED}
	mkdir -p ${DEBUG_INFO_DIR}

ifeq ($(CFG_MT_BUILD_LOADER),y)
	# lib && bin
	cp --remove-destination -arf ${SHARED_LIB_DIR}/* ${SHARED_LIB_DIR_STRIPED}/mt
	cp --remove-destination -arf $(BIN_DIR)/* ${BIN_DIR_STRIPED}/bin/mt
else
	# lib && bin
	# some binary not in BUILDROOT_SYSROOT_DIR, only in BUILDROOT_TARGET_DIR, so only copy BUILDROOT_TARGET_DIR
	cp --remove-destination -arf ${BUILDROOT_TARGET_USR_LIB_DIR}/* ${SHARED_LIB_DIR_STRIPED}
	cp --remove-destination -arf ${BUILDROOT_SYSROOT_USR_LIB_DIR}/mt/* ${SHARED_LIB_DIR_STRIPED}/mt
	cp --remove-destination -arf $(EXTERNAL_BINARY_PREFIX_DIR)/lib/* ${SHARED_LIB_DIR_STRIPED}/mt
	find ${SHARED_LIB_DIR_STRIPED} -name "*.a" | xargs rm -f
	find ${SHARED_LIB_DIR_STRIPED} -name "*.la" | xargs rm -f
	cp --remove-destination -arf ${BUILDROOT_TARGET_USR_DIR}/bin ${BIN_DIR_STRIPED}
	cp --remove-destination -arf ${BUILDROOT_TARGET_USR_DIR}/sbin ${BIN_DIR_STRIPED}
	cp --remove-destination -arf ${BUILDROOT_SYSROOT_USR_DIR}/bin/mt/* ${BIN_DIR_STRIPED}/bin/mt
endif
	# ko
	mkdir -p ${MODULE_DIR_STRIPED}/lib/modules/$(CFG_MT_KERNEL_VERSION)/updates
	-cd ${MODULE_DIR}; cp --remove-destination -arf --parents lib/modules ${MODULE_DIR_STRIPED}

	#firmware
	-cd ${MODULE_DIR}; cp --remove-destination -arf --parents lib/firmware ${MODULE_DIR_STRIPED}

ifneq ($(CONFIG_MT_STRIP_NONE),y)
	./strip.sh install
else
	echo -e "\033[32m not strip, keep symtab and debug info \033[0m"
endif

initramfs:
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)
	mkdir -p ${DEBUG_INFO_DIR}
	cp --remove-destination -arf ${SDK_DIR}/rootfs/${MONTAGE_ROOTFS_PATH}/* $(MONTAGE_ROOTFS_STRIPED)
ifeq ($(CONFIG_MT_MDEV),y)
	sed -i 's|MT_MDEV="false"|MT_MDEV="true"|g' $(MONTAGE_ROOTFS_STRIPED)/etc/init.d/rcS
endif
ifeq ($(CFG_MT_BUILD_LOADER),y)
	mv $(MONTAGE_ROOTFS_STRIPED)/bin/busybox-static $(MONTAGE_ROOTFS_STRIPED)/bin/busybox
	rm -f $(MONTAGE_ROOTFS_STRIPED)/sbin/mingetty
	sed -i 's|^::respawn:/sbin/mingetty --noclear mt_console --autologin root|#::respawn:/sbin/mingetty --noclear mt_console --autologin root|g' $(MONTAGE_ROOTFS_STRIPED)/etc/inittab
	sed -i 's|^#mt_console::respawn:-/bin/login -f root|mt_console::respawn:-/bin/login -f root|g' $(MONTAGE_ROOTFS_STRIPED)/etc/inittab
	rm -rf $(MONTAGE_ROOTFS_STRIPED)/lib/*.so* $(MONTAGE_ROOTFS_STRIPED)/lib64/*.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib/*.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib64/*.so*
else
	rm -f $(MONTAGE_ROOTFS_STRIPED)/bin/busybox-static
	sed -i 's|^#::respawn:/sbin/mingetty --noclear mt_console --autologin root|::respawn:/sbin/mingetty --noclear mt_console --autologin root|g' $(MONTAGE_ROOTFS_STRIPED)/etc/inittab
	sed -i 's|^mt_console::respawn:-/bin/login -f root|#mt_console::respawn:-/bin/login -f root|g' $(MONTAGE_ROOTFS_STRIPED)/etc/inittab
	cp --remove-destination -af ${SHARED_LIB_DIR}/libc-mt.so $(MONTAGE_ROOTFS_STRIPED)/lib
endif
ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
	cp --remove-destination -arf ${SDK_DIR}/rootfs/lxc_rootfs_framework $(MONTAGE_ROOTFS_STRIPED)
endif

ifeq ($(CONFIG_MT_TEE_SUPPORT),y)
	@if [ -e $(TEE_ROOTFS_DIR) ]; then \
		echo "\033[32m To copy the TEE components to the striped rootfs \033[0m"; \
		cp --remove-destination -arf $(TEE_ROOTFS_DIR)/* $(MONTAGE_ROOTFS_STRIPED)/; \
	else \
		echo "\033[32m Can't find the TEE components rootfs directory \033[0m"; \
	fi;
endif

	find $(MONTAGE_ROOTFS_STRIPED) -name .git_cannot_add_empty_folder | xargs rm -f

ifneq ($(CONFIG_MT_STRIP_NONE),y)
	./strip.sh rootfs
else
	echo -e "\033[32m not strip initramfs, keep symtab and debug info \033[0m"
endif

ifneq ($(CFG_MT_BUILD_LOADER),y)
	mkdir -p ${SDK_DIR}/image/$(MFRS)/rootfs
	cp --remove-destination -arf $(MONTAGE_ROOTFS_STRIPED)/* ${SDK_DIR}/image/$(MFRS)/rootfs
else
	mkdir -p ${SDK_DIR}/image/$(MFRS)/loader/rootfs
	cp --remove-destination -arf $(MONTAGE_ROOTFS_STRIPED)/* ${SDK_DIR}/image/$(MFRS)/loader/rootfs
endif

rootfs: initramfs
ifneq ($(CFG_MT_BUILD_LOADER),y)
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/libcrypto.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/libssl.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/libz.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
endif
ifeq ($(CONFIG_MT_MTD_UTILS),y)
	echo "mtd-utils"
	cp --remove-destination -af ${BIN_DIR_STRIPED}/sbin/ubi* $(MONTAGE_ROOTFS_STRIPED)/usr/sbin
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/liblzo2.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/libuuid.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
endif
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/etc/Wireless/RT2870STA
	-cp --remove-destination -af $(RESOURCE_DIR)/MT7601USTA.dat $(MONTAGE_ROOTFS_STRIPED)/etc/Wireless/RT2870STA
	cd $(MONTAGE_ROOTFS_STRIPED)/etc; ln -sf ../tmp/udhcpd.conf .
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/var/lib/misc; cd $(MONTAGE_ROOTFS_STRIPED)/var/lib/misc; ln -sf ../../../tmp/udhcpd.leases .
ifeq ($(CONFIG_MT_SANITIZE),y)
	sed -i 's|export MT_SANITIZE="false"|export MT_SANITIZE="true"|g' $(MONTAGE_ROOTFS_STRIPED)/root/.profile $(MONTAGE_ROOTFS_STRIPED)/etc/init.d/rcS
else ifeq ($(CONFIG_MT_SANITIZE_TAG),y)
	sed -i 's|export MT_SANITIZE_TAG="false"|export MT_SANITIZE_TAG="true"|g' $(MONTAGE_ROOTFS_STRIPED)/root/.profile $(MONTAGE_ROOTFS_STRIPED)/etc/init.d/rcS
else ifeq ($(CONFIG_MT_SANITIZE_THREAD_RACE),y)
	sed -i 's|export MT_SANITIZE_TSAN="false"|export MT_SANITIZE_TSAN="true"|g' $(MONTAGE_ROOTFS_STRIPED)/root/.profile $(MONTAGE_ROOTFS_STRIPED)/etc/init.d/rcS
endif
ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
	@echo "prepare lxc environment in rootfs"
	sed -i 's|export MT_LXC="false"|export MT_LXC="true"|g' $(MONTAGE_ROOTFS_STRIPED)/etc/init.d/S09mount $(MONTAGE_ROOTFS_STRIPED)/root/.profile
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/lib64
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/run
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/lxc_rootfs_transfer
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/lxc_rootfs_real
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/var/lib/lxc
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/usr/share
	cp --remove-destination -arf ${BUILDROOT_TARGET_USR_DIR}/share/lxc $(MONTAGE_ROOTFS_STRIPED)/usr/share
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/usr/share/lxc/config
	cp --remove-destination -af $(RESOURCE_DIR)/conf.txt $(MONTAGE_ROOTFS_STRIPED)/usr/share/lxc/config
	cp --remove-destination -af $(RESOURCE_DIR)/conf-all.txt $(MONTAGE_ROOTFS_STRIPED)/usr/share/lxc/config
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/lxc* $(MONTAGE_ROOTFS_STRIPED)/usr/bin
	cp --remove-destination -af $(BUILDROOT_TARGET_DIR)/bin/zgrep $(MONTAGE_ROOTFS_STRIPED)/usr/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/newuidmap $(MONTAGE_ROOTFS_STRIPED)/usr/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/bin/newgidmap $(MONTAGE_ROOTFS_STRIPED)/usr/bin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/sbin/init.lxc $(MONTAGE_ROOTFS_STRIPED)/usr/sbin
	cp --remove-destination -af $(BIN_DIR_STRIPED)/sbin/init.lxc.static $(MONTAGE_ROOTFS_STRIPED)/usr/sbin
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/liblxc.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -af $(SHARED_LIB_DIR_STRIPED)/libcap.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	@echo "prepare lxc environment in rootfs done!"
endif
#ifeq ($(CONFIG_MT_PYTHON),y)
#	@echo "intall python into rootfs"
#	cp -af --remove-destination $(BIN_DIR_STRIPED)/bin/python* $(MONTAGE_ROOTFS_STRIPED)/usr/bin
#	cp -af --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpython*.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
#	cp -arf --remove-destination $(SHARED_LIB_DIR_STRIPED)/python* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
#	-cp -arf --remove-destination $(SHARED_LIB_DIR_STRIPED)/gobject-introspection $(MONTAGE_ROOTFS_STRIPED)/usr/lib
#	-cd $(BIN_DIR_STRIPED)/bin/; cp -arf --remove-destination --parents `find -type f | xargs file | grep "Python script" | awk -F':' '{print $$1}'` $(MONTAGE_ROOTFS_STRIPED)/usr/bin
#endif
ifeq ($(CONFIG_MT_UDEV),y)
	cp --remove-destination -af $(BUILDROOT_TARGET_DIR)/etc/init.d/S10udev $(MONTAGE_ROOTFS_STRIPED)/etc/init.d
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/lib/udev $(MONTAGE_ROOTFS_STRIPED)/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/lib/libudev.so* $(MONTAGE_ROOTFS_STRIPED)/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/libusb-1.0.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/libkmod.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/etc/udev $(MONTAGE_ROOTFS_STRIPED)/etc
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/etc/usb_modeswitch.d $(BUILDROOT_TARGET_DIR)/etc/usb_modeswitch.conf $(MONTAGE_ROOTFS_STRIPED)/etc
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/sbin/udevd $(BUILDROOT_TARGET_DIR)/sbin/udevadm $(MONTAGE_ROOTFS_STRIPED)/sbin
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/bin/udevadm $(MONTAGE_ROOTFS_STRIPED)/usr/bin
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/sbin/usb_modeswitch $(BUILDROOT_TARGET_USR_DIR)/sbin/usb_modeswitch_dispatcher $(MONTAGE_ROOTFS_STRIPED)/usr/sbin
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/share/usb_modeswitch $(MONTAGE_ROOTFS_STRIPED)/usr/share
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/var/lib/usb_modeswitch $(MONTAGE_ROOTFS_STRIPED)/var/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/etc/passwd $(BUILDROOT_TARGET_DIR)/etc/shadow $(BUILDROOT_TARGET_DIR)/etc/group $(MONTAGE_ROOTFS_STRIPED)/etc
endif
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/lib/libblkid.so* $(MONTAGE_ROOTFS_STRIPED)/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/lib/libuuid.so* $(MONTAGE_ROOTFS_STRIPED)/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/libext2fs.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/libcom_err.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/libe2p.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/libntfs-3g.so* $(MONTAGE_ROOTFS_STRIPED)/usr/lib

	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/sbin/mke2fs $(MONTAGE_ROOTFS_STRIPED)/sbin
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/etc/mke2fs.conf $(MONTAGE_ROOTFS_STRIPED)/etc
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/sbin/mkfs.* $(MONTAGE_ROOTFS_STRIPED)/sbin
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/sbin/e2fsck $(MONTAGE_ROOTFS_STRIPED)/sbin
	cp --remove-destination -arf $(BUILDROOT_TARGET_DIR)/sbin/fsck* $(MONTAGE_ROOTFS_STRIPED)/sbin

	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/sbin/mkntfs $(MONTAGE_ROOTFS_STRIPED)/usr/sbin
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/sbin/mkfs.* $(MONTAGE_ROOTFS_STRIPED)/usr/sbin


	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)
ifeq ($(CONFIG_MT_INSTALL_GDBSERVER),y)
	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af ${SDK_DIR}/tools/toolchain/toolbox/$(CFG_MT_ARCH)/usr/bin/gdbserver $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/bin/strace $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/bin/ltrace $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	-cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/share/ltrace $(MONTAGE_ROOTFS_STRIPED)/usr/share
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-$(CFG_MT_ARCH).so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-coredump.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-generic.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-ptrace.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-setjmp.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libelf-*.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libelf.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libdw-*.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libdw.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
endif
ifeq ($(CONFIG_MT_INSTALL_PERF),y)
	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)/etc/bash_completion.d
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/etc/bash_completion.d/perf $(DEBUG_TOOLS_INSTALL_PATH)/etc/bash_completion.d
	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/bin/perf $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/bin/trace $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/bin/eu-nm $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_DIR)/bin/eu-readelf $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/perf-lag.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/usdt-012-pos.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/usdt-xyz-pos.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/usdt-create.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/usdt-del.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/usdt-enable.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/perf/usdt-disable.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libperf-jvmti.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libasm-*.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libasm.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libdw-*.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libdw.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libelf-*.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libelf.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-$(CFG_MT_ARCH).so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-coredump.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-generic.so $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-ptrace.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-setjmp.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/elfutils/libebl_$(CFG_MT_ARCH)* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libcap.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libtraceevent.so* $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_LIB_DIR)/traceevent $(DEBUG_TOOLS_INSTALL_PATH)/usr/lib
	mkdir -p $(DEBUG_TOOLS_INSTALL_PATH)/usr/libexec
	cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/libexec/perf-core $(DEBUG_TOOLS_INSTALL_PATH)/usr/libexec
endif
ifeq ($(CONFIG_MT_INSTALL_GPERFTOOLS),y)
	mkdir -p $(MONTAGE_ROOTFS_STRIPED)/lib
ifeq ($(CONFIG_MT_GPERFTOOLS_HEAP_PROF),y)
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libtcmalloc.so* $(MONTAGE_ROOTFS_STRIPED)/lib
else
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libprofiler.so* $(MONTAGE_ROOTFS_STRIPED)/lib
endif
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind-$(CFG_MT_ARCH).so* $(MONTAGE_ROOTFS_STRIPED)/lib
	-cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libunwind.so* $(MONTAGE_ROOTFS_STRIPED)/lib
ifeq ($(CONFIG_MT_GPERFTOOLS_HEAP_PROF),y)
	sed -i 's|MT_GPERFTOOLS_HEAP_PROF="false"|MT_GPERFTOOLS_HEAP_PROF="true"|g' $(MONTAGE_ROOTFS_STRIPED)/root/.profile
endif
endif
ifeq ($(CONFIG_MT_INSTALL_JEMALLOC),y)
	sed -i 's|MT_JEMALLOC="false"|MT_JEMALLOC="true"|g' $(MONTAGE_ROOTFS_STRIPED)/root/.profile $(MONTAGE_ROOTFS_STRIPED)/etc/init.d/rcS
	cp --remove-destination -af $(BUILDROOT_TARGET_USR_LIB_DIR)/libjemalloc.so* $(MONTAGE_ROOTFS_STRIPED)/lib
endif
ifeq ($(CONFIG_MT_FTRACE),y)
	cp --remove-destination -af ${SDK_DIR}/tools/ftrace/ftrace_init.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/ftrace/ftrace_start.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
	cp --remove-destination -af ${SDK_DIR}/tools/ftrace/ftrace_end.sh $(DEBUG_TOOLS_INSTALL_PATH)/usr/bin
endif

	if [ -n "`grep "CONFIG_SOUND=y" $(KERNEL_OUTPUT)/.config`" -o -n "`grep "CONFIG_SOUND=m" $(KERNEL_OUTPUT)/.config`" ]; then \
		mkdir -p $(MONTAGE_ROOTFS_STRIPED)/usr/share; \
		cp --remove-destination -arf $(BUILDROOT_TARGET_USR_DIR)/share/alsa $(MONTAGE_ROOTFS_STRIPED)/usr/share; \
	fi;

ifeq ($(CONFIG_MT_BLUEZ_SUPPORT),y)
	cp --remove-destination -af $(BUILDROOT_TARGET_DIR)/usr/libexec/bluetooth/bluetoothd $(MONTAGE_ROOTFS_STRIPED)/usr/bin
endif

ifneq ($(CONFIG_MT_STRIP_NONE),y)
	./strip.sh rootfs
else
	echo -e "\033[32m not strip rootfs, keep symtab and debug info \033[0m"
endif

uninstall:
	rm -rf ${SHARED_LIB_DIR_STRIPED}
	rm -rf ${BIN_DIR_STRIPED}
	rm -rf ${MODULE_DIR_STRIPED}
	rm -rf $(MONTAGE_ROOTFS_STRIPED)
	rm -rf $(DEBUG_TOOLS_INSTALL_PATH)
	rm -rf ${DEBUG_INFO_DIR}
