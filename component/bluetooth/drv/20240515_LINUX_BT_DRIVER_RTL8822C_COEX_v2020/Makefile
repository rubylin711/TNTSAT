include ${SDK_DIR}/build/script/base.mk

objects := usb uart

objects_clean:=$(addsuffix _clean,$(objects))
objects_install:=$(addsuffix _install,$(objects))
objects_uninstall:=$(addsuffix _uninstall,$(objects))
objects_distclean:=$(addsuffix _distclean,$(objects))
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#   rules
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#==================================================================
#                          all
#==================================================================
.PHONY: all clean install uninstall distclean $(objects) $(objects_clean) $(objects_install) $(objects_uninstall) $(objects_distclean)
all:
	$(foreach d,$(objects), make -C $(d) all;)

clean: 
	$(foreach d,$(objects), make -C $(d) clean;)

install: $(objects_install)
#install:

uninstall: $(objects_uninstall)
#uninstall:

distclean:

#==================================================================
#                          modules
#==================================================================
$(objects):
	make $(CFLAGS) -C $@ all

$(objects_clean):
	make -C $(patsubst %_clean,%,$@) clean

$(objects_install):
	make $(CFLAGS) -C $(patsubst %_install,%,$@) install

$(objects_uninstall):
	make -C $(patsubst %_uninstall,%,$@) uninstall

$(objects_distclean):
	make -C $(patsubst %_distclean,%,$@) distclean
