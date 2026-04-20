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
TARGET = $(OUTDIR)/$(TARGETNAME)



#
# include the base make file
#
include $(MSPK_ROOT)/Platform/$(MSPK_PLATFORM)/Build/MspkBase.mk


#
# Add a map file if desired
#
ifdef MSPKOPTION_MAPFILE
PLATLINKFLAGS += -Wl,-Map=$(OUTDIR)/$(TARGETNAME).map
endif

#
# Support supplementaty test files that an application can request to have
# copied into the output folder under "TestFiles"
#
SUPPLEMENTARY_TEST_FILES=$(subst \,/,$(MSPK_SUPPLEMENTARY_TEST_FILES))
SUPPLEMENTARY_OUTPUT_TEST_FILES=$(patsubst %, $(OUTDIR)/TestFiles/%,$(notdir $(SUPPLEMENTARY_TEST_FILES)))

ifneq '$(SUPPLEMENTARY_OUTPUT_TEST_FILES)' ''
$(SUPPLEMENTARY_OUTPUT_TEST_FILES): $(SUPPLEMENTARY_TEST_FILES)
	mkdir -p $(OUTDIR)/TestFiles/
	cp $? $(OUTDIR)/TestFiles/
endif

#
# The MAKE rule to make the final target executable application, which is done by linking the object files
# and libraries. Here, we do whatever it takes to build a binary that's runable on the platform. That could
# be a firmware image, a single executable file, or anything else. On PC Linux systems, just an executable
# file is needed that can be run from the command line, so that's what we do here. We're just using "gcc"
# to do the linking, since it manages all the preliminary and intermediate processes that needs to be run
# before calling "ld" in addition to calling "ld" with all the correct platform-specific command-line options.
#
$(TARGET): $(OBJS) $(TARGETLIBS) $(SUPPLEMENTARY_OUTPUT_TEST_FILES)
	@/bin/echo -e \\tLinking: \\0033[`echo "$$LS_COLORS" | sed "s/^.*ex=\([^:]*\).*$$/\1/g"`m$(notdir $@)\\0033[00m 1>&3
	-$(RM) -f $(TARGET)
	mkdir -p $(OUTDIR)
	$(CC) -g -g3 -o $(TARGET) -Xlinker --start-group $(CRT_STARTUP_OBJS) $(OBJS) $(TARGETLIBS) -Xlinker --end-group $(PLATLINKFLAGS) $(DYNAMIC_LIBS)



