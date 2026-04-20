include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

.PHONY: all clean install uninstall distclean version prepare

all: prepare version
	$(MAKE) -f sample_lib.mk
	$(MAKE) -f sample_app.mk
ifeq ($(CFG_MT_SAMPLE), y)
	$(MAKE) -f sample_mt.mk
endif
clean:
	$(MAKE) -f sample_lib.mk clean
	$(MAKE) -f sample_app.mk clean
ifeq ($(CFG_MT_SAMPLE), y)
	$(MAKE) -f sample_mt.mk clean
	@if [ -e commit.info ] ; then \
		rm -f commit.info; \
	fi
	@if [ -e prolist.info ] ; then \
		rm -f prolist.info; \
	fi
endif
install:
	$(MAKE) -f sample_lib.mk install
	$(MAKE) -f sample_app.mk install
ifeq ($(CFG_MT_SAMPLE), y)
	$(MAKE) -f sample_mt.mk install
endif
uninstall:
	$(MAKE) -f sample_lib.mk uninstall
	$(MAKE) -f sample_app.mk uninstall
ifeq ($(CFG_MT_SAMPLE), y)
	$(MAKE) -f sample_mt.mk uninstall
endif
distclean:
	$(MAKE) -f sample_lib.mk distclean
	$(MAKE) -f sample_app.mk distclean
ifeq ($(CFG_MT_SAMPLE), y)
	$(MAKE) -f sample_mt.mk distclean
endif
version:
	@echo "Build sample version..."
ifeq ($(CFG_MT_SAMPLE), y)
	@if [ -d ${SDK_DIR}/../.repo ]; then \
		repo forall -c "git log -1" | grep commit > commit.info; \
		sed -i '/^commit/!d' commit.info; \
		repo list > prolist.info; \
		sed -i -r 's/(.[:]).*/\1/' prolist.info; \
	fi
endif
