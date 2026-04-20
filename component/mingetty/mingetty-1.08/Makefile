include ${SDK_DIR}/build/script/base.mk

CFLAGS = -O2 -Wall -W -pipe -D_GNU_SOURCE
ifeq ($(CFG_MT_BUILD_LOADER),y)
CFLAGS += -static
endif

all: mingetty
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

install:
	$(AT)mkdir -p ${BIN_DIR}
	cp -af mingetty ${BIN_DIR}

mingetty: mingetty.c
	${CC} $(CFLAGS) -o $@ $<

clean:
	rm -f *.o mingetty

distclean:

uninstall:
	rm -f ${BIN_DIR}/mingetty
