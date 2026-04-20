include ${SDK_DIR}/build/script/base.mk

MSP_LIB := libmt_msp

MSP_LIB_A = $(call out_lib,$(MSP_LIB).a)
MSP_LIB_SO = $(call out_lib,$(MSP_LIB).so)

ALL_STATIC_OBJS_NOTE=$(DIR_REAL)/all_static_objs
ALL_SHARED_OBJS_NOTE=$(DIR_REAL)/all_shared_objs
export ALL_STATIC_OBJS_NOTE ALL_SHARED_OBJS_NOTE

sinclude $(ALL_STATIC_OBJS_NOTE)
sinclude $(ALL_SHARED_OBJS_NOTE)

include $(MSP_API_OBJECTS)


objects_clean       := $(addsuffix _clean, $(objects))
objects_install     := $(addsuffix _install, $(objects))
objects_uninstall   := $(addsuffix _uninstall, $(objects))

ifneq ($(LIBC_MT_ITSELF),y)
DEPEND_LIBS_PATH += -L$(SHARED_LIB_DIR)
DEPEND_LIBS += -lc-mt -lpthread
endif

DEPEND_LIBS_PATH_ALL = $(DEPEND_LIBS_PATH)

LDFLAGS = $(EXTRA_LDFLAGS) $(MT_SHARED_LIB_LDFLAGS) $(DEPEND_LIBS_PATH_ALL)

ifneq ($(DEPEND_LIBS),)
LDFLAGS += -Wl,--start-group $(DEPEND_LIBS) -Wl,--end-group
endif


.PHONY: all clean install uninstall $(objects) $(objects_clean) $(objects_install) $(objects_uninstall) objects_done all_objs_phony

all: all_objs_phony
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r objects_done
	$(AT)$(MAKE) -f $(CURRENT_MAKEFILES) -r install

clean: $(objects_clean)
	$(AT)rm -rf $(MSP_LIB_A) $(MSP_LIB_SO)
	$(AT)rm -f $(ALL_STATIC_OBJS_NOTE)
	$(AT)rm -f $(ALL_SHARED_OBJS_NOTE)

install: $(objects_install) $(MSP_LIB_A)
	$(AT)mkdir -p $(SHARED_LIB_DIR)
	$(AT)cp -af  $(MSP_LIB_SO)  $(SHARED_LIB_DIR)/
	$(AT)mkdir -p $(STATIC_LIB_DIR)
	$(AT)cp -af  $(MSP_LIB_A)   $(STATIC_LIB_DIR)/
ifneq ($(INSTALL_EXTRA_SCRIPT),)
	$(AT)mkdir -p $(BIN_DIR)
	$(AT)cp -af $(INSTALL_EXTRA_SCRIPT) $(BIN_DIR)/
endif
ifneq ($(INSTALL_EXTRA_RESOURCE),)
	$(AT)cp -af $(INSTALL_EXTRA_RESOURCE) $(RESOURCE_DIR)/
endif
	mkdir -p ${INCLUDE_DIR}
	cd $(MSP_UNF_INCLUDE); header_files="`ls`"; cp -arf $${header_files} ${INCLUDE_DIR}/

uninstall: $(objects_uninstall)
	$(AT)rm -rf $(STATIC_LIB_DIR)/$(MSP_LIB).a
	$(AT)rm -rf $(SHARED_LIB_DIR)/$(MSP_LIB).so
ifneq ($(INSTALL_EXTRA_SCRIPT),)
	$(AT)$(foreach install_extra, $(notdir $(INSTALL_EXTRA_SCRIPT)), rm -f $(BIN_DIR)/$(install_extra))
endif
ifneq ($(INSTALL_EXTRA_RESOURCE),)
	$(AT)$(foreach install_extra, $(notdir $(INSTALL_EXTRA_RESOURCE)), rm -f $(RESOURCE_DIR)/$(install_extra))
endif
	cd $(MSP_UNF_INCLUDE); header_files="`ls`"; if [ -d "${INCLUDE_DIR}" ]; then cd ${INCLUDE_DIR}; rm -rf $${header_files}; fi

$(objects):
	$(AT)$(MAKE) -C $@

$(objects_clean):
	$(AT)$(MAKE) -C $(patsubst %_clean,%,$@) clean

$(objects_install):
	$(AT)$(MAKE) -C $(patsubst %_install,%,$@) install

$(objects_uninstall):
	$(AT)$(MAKE) -C $(patsubst %_uninstall,%,$@) uninstall

$(MSP_LIB_A): $(ALL_STATIC_OBJS_NOTE)
	$(AR) -rs $@ $(sort $(ALL_STATIC_OBJS))

$(MSP_LIB_SO): $(ALL_SHARED_OBJS_NOTE)
	$(CC) -o $(@:.a=.so) $(sort $(ALL_SHARED_OBJS)) $(LDFLAGS) -Wl,-soname=$(notdir $@)

objects_done: $(objects)
	$(AT)$(MAKE) -r $(MSP_LIB_A)
	$(AT)$(MAKE) -r $(MSP_LIB_SO)

all_objs_phony:
	$(AT)mkdir -p $(DIR_REAL)
	$(AT)echo "ALL_STATIC_OBJS:=" > $(ALL_STATIC_OBJS_NOTE)
	$(AT)echo "ALL_SHARED_OBJS:=" > $(ALL_SHARED_OBJS_NOTE)
	$(foreach dir,$(sort $(objects)),$(MAKE) -C $(dir) all_objs_phony;)
