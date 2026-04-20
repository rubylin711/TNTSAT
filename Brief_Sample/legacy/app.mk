include ${SDK_DIR}/build/script/base.mk

SUBDIRS = backtrace cma_migrate core1-hotplug dvfs-avs float gup heap-profile jill kprobe-uprobe mmz mt_walk_watch mtest neon-instruction performance pthread sleep usdt timer typedef
ifeq ($(CONFIG_MT_LXC_SUPPORT),y)
SUBDIRS += lxc_ipc
endif

SUBDIRS += mss

ifneq ($(CONFIG_MT_SANITIZE_NONE),y)
SUBDIRS += sanitize
endif

include ${SDK_DIR}/build/script/Makefile-subdirs.rule
