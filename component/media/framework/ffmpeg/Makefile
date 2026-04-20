include ${SDK_DIR}/build/script/base.mk

.PHONY: all install uninstall clean distclean

ifeq ($(CFG_FFMPEG_VER_NUM),422)
FFMPEG_FOLDER_NAME=ffmpeg-4.2.2
else
FFMPEG_FOLDER_NAME=ffmpeg-$(CFG_FFMPEG_VER_NUM)
endif

all:
ifeq ($(FFMPEG_FOLDER_NAME),$(wildcard $(FFMPEG_FOLDER_NAME)))
	$(MAKE) -f 16.set_env_ffmpeg.mk CODE_PATH=ffmpeg/$(FFMPEG_FOLDER_NAME) MT_BUILD_DIR=$(COMPONENT_BUILD_DIR)/media/framework MT_SRC_DIR=$(COMPONENT_DIR)/media/framework
endif

install:

uninstall:

clean:
ifeq ($(FFMPEG_FOLDER_NAME),$(wildcard $(FFMPEG_FOLDER_NAME)))
	-$(MAKE) -f 16.set_env_ffmpeg.mk clean CODE_PATH=ffmpeg/$(FFMPEG_FOLDER_NAME) MT_BUILD_DIR=$(COMPONENT_BUILD_DIR)/media/framework MT_SRC_DIR=$(COMPONENT_DIR)/media/framework
endif

distclean:
ifeq ($(FFMPEG_FOLDER_NAME),$(wildcard $(FFMPEG_FOLDER_NAME)))
	-$(MAKE) -f 16.set_env_ffmpeg.mk distclean CODE_PATH=ffmpeg/$(FFMPEG_FOLDER_NAME) MT_BUILD_DIR=$(COMPONENT_BUILD_DIR)/media/framework MT_SRC_DIR=$(COMPONENT_DIR)/media/framework
endif
