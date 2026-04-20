include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	
	$(MAKE) -f sample_pvr_play.mk
	$(MAKE) -f sample_pvr_timeshift.mk

clean:
	
	$(MAKE) -f sample_pvr_play.mk clean
	$(MAKE) -f sample_pvr_timeshift.mk clean

install:
	
	$(MAKE) -f sample_pvr_play.mk install
	$(MAKE) -f sample_pvr_timeshift.mk install

uninstall:
	
	$(MAKE) -f sample_pvr_play.mk uninstall
	$(MAKE) -f sample_pvr_timeshift.mk uninstall
