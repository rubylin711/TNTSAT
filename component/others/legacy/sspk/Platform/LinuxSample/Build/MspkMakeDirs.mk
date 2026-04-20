#*****************************************************************
#
# Copyright (C) Microsoft Corporation. All rights reserved.
#
#*****************************************************************

MSPKOPTION_ENABLE_APP=0

# Need to be able to include or exclude dirs (above the PAL layer) based on the platform.
# GNUMake-based systems can override dirs via dirs.mk (IIS OOB #31626).
-include dirs.mk

ifndef dirs
include dirs
endif

.PHONY: $(dirs)     # .PHONY will ignore files named mspk_all
.PHONY : mspk_clean # and will run the command unconditionally.
.PHONY : mspk_all   # see www.gnu.org/software/make/manual/make.html for more info

#
# The first target will always get built if a specific target is
# not mentioned, so we put $(VTARGET) here so that if we are building,
# we build mspk_all (the value of $(VTARGET), and if we are cleaning,
# we build mspk_clean (the value of $(VTARGET).
#
$(VTARGET):

mspk_all: $(dirs)

mspk_clean: $(dirs)

$(dirs):
	@/bin/echo -e \\0033[`echo "$$LS_COLORS" | sed "s/^.*di=\([^:]*\).*$$/\1/g"`m`pwd`/$@\\0033[00m: 1>&3
	@$(MAKE) -f makefile.pk -C $@ VTARGET=$(VTARGET) $(VTARGET);

