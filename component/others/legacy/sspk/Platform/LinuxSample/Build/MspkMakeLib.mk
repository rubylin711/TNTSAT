#*****************************************************************
#
# Copyright (C) Microsoft Corporation. All rights reserved.
#
#*****************************************************************

.PHONY : mspk_clean
.PHONY : mspk_all
.PHONY : $(dirs)

include build.pk

#
# The first target will always get built if a specific target is
# not mentioned, so we put $(VTARGET) here so that if we are building,
# we build mspk_all (the value of $(VTARGET), and if we are cleaning,
# we build mspk_clean (the value of $(VTARGET).
#
$(VTARGET):


#
# Define the TARGETNAME variable
#
TARGETNAME = $(MSPK_TARGETNAME)


#
# Convert back-slashes (Windows-style) in paths to
# forward-slashes (Unix-style)
#
TARGETPATH = $(subst \,/,$(MSPK_TARGETPATH))
SOURCES = $(subst \,/,$(MSPK_SOURCES))
INCLUDES = $(subst \,/,$(MSPK_INCLUDES);)
TARGETLIBS = $(subst \,/,$(MSPK_TARGETLIBS))


#
# Create the TARGET variable
#
TARGET = $(OUTDIROBJ)/$(TARGETNAME).$(MSPK_LIB_EXT)


#
# include the base make file
#
include $(MSPK_ROOT)/Platform/$(MSPK_PLATFORM)/Build/MspkBase.mk

#
# The "target" is a library build with the "AR" command. We create
# the folder as necessary, and we delete any existing target to make
# sure we build cleanly.
#
$(TARGET): $(OBJS)
	@/bin/echo -e \\t\\0033[`echo "$$LS_COLORS" | sed "s/^.*ex=\([^:]*\).*$$/\1/g"`m$(notdir $@)\\0033[00m 1>&3
	mkdir -p $(OUTDIROBJ)
	-$(RM) -f $(TARGET)
	$(AR) -c -r $(TARGET) $(OBJS)
	mkdir -p $(MSPK_OUTPUT_INSTALL)/lib/$(MSPK_TARGETMODULE)
	cp $(TARGET) $(MSPK_OUTPUT_INSTALL)/lib/$(MSPK_TARGETMODULE)/lib$(TARGETNAME).$(MSPK_LIB_EXT)

