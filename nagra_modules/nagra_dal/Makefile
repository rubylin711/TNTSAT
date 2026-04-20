include $(SDK_DIR)/build/script/base.mk

.PHONY: all clean install uninstall

all:
	$(MAKE) -f libnagra_dal.mk
	$(MAKE) -f libnagra_icc.mk

ifneq ($(wildcard *.a),)
	$(MAKE) -f dalts.mk
endif

clean:
	$(MAKE) -f libnagra_dal.mk clean
	$(MAKE) -f libnagra_icc.mk clean

ifneq ($(wildcard *.a),)
	$(MAKE) -f dalts.mk clean
endif