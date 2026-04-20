# for native builds:
# make modules modules_install
# make KERNELDIR=<not currently running kernel's build tree> modules modules_install
# make KERNEL_VERSION=<uname -r of the not currently running kernel> modules modules_install
#
# for cross builds, using standard kernel make environment, i.e.
# make KERNELDIR=<linux build tree> INSTALL_MOD_PATH=<target root fs> modules modules_install
include ${SDK_DIR}/build/script/base.mk

ifeq ($(KERNELRELEASE),)
	KERNELRELEASE = $(shell cat $(KERNEL_OUTPUT)/include/config/kernel.release 2> /dev/null)
endif

LINUX_FUSION_MAKE_OPTS = \
	$(MT_KERNEL_OUTPUT_OPT) \
	ARCH=$(KERNEL_ARCH) \
	CROSS_COMPILE=$(CONFIG_CROSS_COMPILE_KERNEL) \
	INSTALL_MOD_PATH=${MODULE_DIR}

KERNEL_VERSION   ?= ${KERNELRELEASE}
INSTALL_MOD_PATH ?= ${MODULE_DIR}
KERNELDIR        ?= $(INSTALL_MOD_PATH)/lib/modules/$(KERNEL_VERSION)/build
FUSIONCORE       ?= single
ONECORE          ?= single
# DEBUG            := yes

export FUSIONCORE
export ONECORE
export INSTALL_MOD_PATH


FUSION_CPPFLAGS += -DFUSION_CALL_INTERRUPTIBLE \
	-I`pwd`/linux/drivers/char/fusion \
	-I`pwd`/linux/drivers/char/fusion/multi \
	-I`pwd`/linux/drivers/char/fusion/$(FUSIONCORE)

ONE_CPPFLAGS += \
	-I`pwd`/one \
	-I`pwd`/one/$(ONECORE) \
	-I`pwd`/include \

KERNEL_BUILD  = $(KERNELDIR)
# works for 2.6.23
KERNEL_SOURCE = $(shell grep ^KERNELSRC $(KERNEL_BUILD)/Makefile | cut -d ' ' -f 6)
ifeq ($(KERNEL_SOURCE), )
  # works for 2.6.32
  KERNEL_SOURCE = $(shell grep '^MAKEARGS := -C ' $(KERNEL_BUILD)/Makefile | cut -d ' ' -f 4)
endif
ifneq ($(KERNEL_SOURCE), )
  K_VERSION    = $(shell grep '^VERSION =' $(KERNEL_SOURCE)/Makefile | cut -d ' ' -f 3)
  K_PATCHLEVEL = $(shell grep '^PATCHLEVEL =' $(KERNEL_SOURCE)/Makefile | cut -d ' ' -f 3)
  K_SUBLEVEL   = $(shell grep '^SUBLEVEL =' $(KERNEL_SOURCE)/Makefile | cut -d ' ' -f 3)
else
  K_VERSION    = $(shell grep '^VERSION =' $(KERNEL_BUILD)/Makefile | cut -d ' ' -f 3)
  K_PATCHLEVEL = $(shell grep '^PATCHLEVEL =' $(KERNEL_BUILD)/Makefile | cut -d ' ' -f 3)
  K_SUBLEVEL   = $(shell grep '^SUBLEVEL =' $(KERNEL_BUILD)/Makefile | cut -d ' ' -f 3)
endif

FUSION_SUB = linux/drivers/char/fusion
ONE_SUB    = one
SUBMOD     = updates/fusion

export CONFIG_FUSION_DEVICE=m
export CONFIG_LINUX_ONE=m

ifeq ($(DEBUG),yes)
  FUSION_CPPFLAGS += -DFUSION_DEBUG_SKIRMISH_DEADLOCK -DFUSION_ENABLE_DEBUG  
  ONE_CPPFLAGS    += -DONE_ENABLE_DEBUG
endif

ifeq ($(shell test -e $(KERNEL_BUILD)/include/linux/autoconf.h && echo yes),yes)
  AUTOCONF_H = -include $(KERNEL_BUILD)/include/linux/autoconf.h
endif

ifeq ($(shell test -e $(KERNEL_BUILD)/include/linux/config.h && echo yes),yes)
  CPPFLAGS += -DHAVE_LINUX_CONFIG_H
endif

KMAKEFILE = Makefile-2.6

FUSION_EXTRAFLAGS = KCPPFLAGS="$(CPPFLAGS) $(FUSION_CPPFLAGS) -I`pwd`/linux/include" INSTALL_MOD_DIR="$(SUBMOD)"
ONE_EXTRAFLAGS    = KCPPFLAGS="$(CPPFLAGS) $(ONE_CPPFLAGS) -I`pwd`/linux/include" INSTALL_MOD_DIR="$(SUBMOD)"

.PHONY: all modules modules_install install clean

all: modules install
install: modules_install headers_install

modules:
	rm -f $(FUSION_SUB)/Makefile
	rm -f $(ONE_SUB)/Makefile
	cp $(FUSION_SUB)/$(KMAKEFILE) $(FUSION_SUB)/Makefile
	cp $(ONE_SUB)/$(KMAKEFILE) $(ONE_SUB)/Makefile
	echo kernel is in $(KERNEL_SOURCE) and version is $(K_SUBLEVEL), building module in $(FUSION_SUB)
	$(MAKE) -C $(KERNEL_BUILD) \
		$(FUSION_EXTRAFLAGS) \
		${LINUX_FUSION_MAKE_OPTS} \
		M=`pwd`/$(FUSION_SUB) modules -j$(j)
	echo kernel is in $(KERNEL_SOURCE) and version is $(K_SUBLEVEL), building module in $(ONE_SUB)
	$(MAKE) -C $(KERNEL_BUILD) \
		$(ONE_EXTRAFLAGS) \
		${LINUX_FUSION_MAKE_OPTS} \
		M=`pwd`/$(ONE_SUB) modules -j$(j)

modules_install: modules
	@echo -e "\033[32;5m""===== modules_install =====""\033[00m"
	$(MAKE) -C $(KERNEL_BUILD) \
		$(FUSION_EXTRAFLAGS) \
		${LINUX_FUSION_MAKE_OPTS} \
		M=`pwd`/$(FUSION_SUB) modules_install
	$(MAKE) -C $(KERNEL_BUILD) \
		$(ONE_EXTRAFLAGS) \
		${LINUX_FUSION_MAKE_OPTS} \
		M=`pwd`/$(ONE_SUB) modules_install

headers_install:
	install -d $(BUILDROOT_SYSROOT_USR_INC_DIR)/linux
	install -m 644 linux/include/linux/fusion.h $(BUILDROOT_SYSROOT_USR_INC_DIR)/linux
	install -m 644 include/linux/one.h $(BUILDROOT_SYSROOT_USR_INC_DIR)/linux



clean:
	find $(FUSION_SUB) \( -name .git -prune \
		-o -name *.o -o -name *.ko -o -name .*.cmd \
		-o -name fusion.mod.* -o -name fusion.mod -o -name .fusion.* \
		-o -name Module.symvers -o -name modules.order \) \
		-type f -print | xargs rm -f
	find $(FUSION_SUB) -name .tmp_versions -type d -print |  xargs rm -rf
	rm -f $(FUSION_SUB)/Makefile
	find $(ONE_SUB) \( -name .git -prune \
		-o -name *.o -o -name *.ko -o -name .*.cmd \
		-o -name linux-one.mod.* -o -name linux-one.mod -o -name .linux-one.* \
		-o -name Module.symvers -o -name modules.order \) \
		-type f -print | xargs rm -f
	find $(ONE_SUB) -name .tmp_versions -type d -print |  xargs rm -rf
	rm -f $(ONE_SUB)/Makefile

distclean: clean

uninstall:
	rm -f ${MODULE_DIR}/lib/modules/$(KERNEL_VERSION)/$(SUBMOD)/fusion.ko
	rm -f ${MODULE_DIR}/lib/modules/$(KERNEL_VERSION)/$(SUBMOD)/linux-one.ko
