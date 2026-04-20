include ${SDK_DIR}/build/script/base.mk

ifeq ($(CFG_MT_SDK_RELEASE),y)
BOARD_ALL := board_full board_dd board_none
else
BOARD_ALL := board_full board_dd board_wma board_none
endif

PHONY: help

.PHONY: all install bin update

all: help

help:
	@echo "valid target:"
	@echo "install"

#
# $1: BOARD name
#
define fun_build_common
	echo "############## build $(1) images ##############"
	cp --remove-destination -arf $(1)/* ./
	make -f board.mk install
	make -f board.mk bin
	make -f board.mk update
	rm -rf output-$(1)
	mkdir output-$(1)
	if [ -d "update" ]; then \
		mv update output-$(1)/; \
	fi
	echo "########## build $(1) images - done. ##########"
endef

install:
	@$(foreach BOARD,$(BOARD_ALL),$(call fun_build_common,$(BOARD));)

bin:

update:
