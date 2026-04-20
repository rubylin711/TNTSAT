include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
	$(MAKE) -f app1.mk
endif
	$(MAKE) -f app2.mk
	$(MAKE) -f app3.mk
	$(MAKE) -f app_dvbt.mk

clean:
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
	$(MAKE) -f app1.mk clean
endif
	$(MAKE) -f app2.mk clean
	$(MAKE) -f app3.mk clean
	$(MAKE) -f app_dvbt.mk clean

install:
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
	$(MAKE) -f app1.mk install
endif
	$(MAKE) -f app2.mk install
	$(MAKE) -f app3.mk install
	$(MAKE) -f app_dvbt.mk install

uninstall:
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
	$(MAKE) -f app1.mk uninstall
endif
	$(MAKE) -f app2.mk uninstall
	$(MAKE) -f app3.mk uninstall
	$(MAKE) -f app_dvbt.mk uninstall

distclean:
ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony1 symphony2 symphony4))
	$(MAKE) -f app1.mk distclean
endif
	$(MAKE) -f app2.mk distclean
	$(MAKE) -f app3.mk distclean
	$(MAKE) -f app_dvbt.mk distclean
