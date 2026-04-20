include ${SDK_DIR}/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f app_sample_mss.mk
	$(MAKE) -f tsplay/sample_mss_tsplay.mk

clean:
	$(MAKE) -f app_sample_mss.mk clean
	$(MAKE) -f tsplay/sample_mss_tsplay.mk clean

install:
	$(MAKE) -f app_sample_mss.mk install
	$(MAKE) -f tsplay/sample_mss_tsplay.mk install

uninstall:
	$(MAKE) -f app_sample_mss.mk uninstall
	$(MAKE) -f tsplay/sample_mss_tsplay.mk uninstall