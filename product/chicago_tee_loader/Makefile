###############################################################################
#
# Montage LZ Symphony6 TEE Loader Product Makefile
#
###############################################################################

include ${SDK_DIR}/build/script/base.mk

.PHONY: all install bin update help rootfs

all: help

help:
	@echo "valid target:"
	@echo "install bin update"

rootfs:
	make -f rootfs.mk

ENVFILE := $(wildcard ${SDK_DIR}/image/$(MFRS)/bootargs/*.env)
ENVSRC := $(patsubst %.env,%.scr,$(ENVFILE))

%.scr: %.env
	${SDK_DIR}/tools/linux/mkimage -A arm -O linux -T script -C none -a 0 -e 0 -n "Boot Script" -d $^ $@

install: rootfs
	cp --remove-destination -f $(BIN_DIR_STRIPED)/bin/mt/uImage ${SDK_DIR}/image/$(MFRS)
	cp --remove-destination -f $(BIN_DIR_STRIPED)/bin/mt/*.dtb ${SDK_DIR}/image/$(MFRS)

bin: $(ENVSRC)

update:
	mkdir -p ${SDK_DIR}/image/$(MFRS)/update
	cp -af ${SDK_DIR}/image/$(MFRS)/rootfs_squashubi.img ${SDK_DIR}/image/$(MFRS)/update/loader_rootfs_squashubi.img
	-cp -af ${SDK_DIR}/image/$(MFRS)/rootfs.ext4 ${SDK_DIR}/image/$(MFRS)/update/loader_rootfs.ext4
	cp -af ${SDK_DIR}/image/$(MFRS)/symphony6*.dtb ${SDK_DIR}/image/$(MFRS)/update/
	cp -af ${SDK_DIR}/image/$(MFRS)/uImage ${SDK_DIR}/image/$(MFRS)/update/uImage-loader
	#cp -af ${SDK_DIR}/image/$(MFRS)/bootargs/*.scr ${SDK_DIR}/image/$(MFRS)/update/
	cp -af ${SDK_DIR}/image/$(MFRS)/update*.vbs ${SDK_DIR}/image/$(MFRS)/update/
