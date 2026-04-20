include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all: 
	$(MAKE) -f sample_ca_ird_lib.mk
	$(MAKE) -f sample_ca_ird.mk

clean:
	$(MAKE) -f sample_ca_ird_lib.mk clean
	$(MAKE) -f sample_ca_ird.mk clean

install:
	$(MAKE) -f sample_ca_ird_lib.mk install
	$(MAKE) -f sample_ca_ird.mk install

uninstall:
	$(MAKE) -f sample_ca_ird_lib.mk uninstall
	$(MAKE) -f sample_ca_ird.mk uninstall

distclean:
	$(MAKE) -f sample_ca_ird_lib.mk distclean
	$(MAKE) -f sample_ca_ird.mk distclean
