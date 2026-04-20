###################################################################
###################################################################
##                                                               ##
## Copyright (C) Microsoft Corporation. All rights reserved.     ##
##                                                               ##
## MspkBase.mk                                                   ##
##                                                               ##
## Provides the default defines and make rules for Linux hosted  ##
## build make files.                                             ##
##                                                               ##
###################################################################
###################################################################


###################################################################
##  Preliminary stuff
###################################################################

#
# Define location for build output, if one has not been defined
# by the enviroment This location will be populated with an obj
# and bin subdir for object and binaries
#
ifndef OUTPUT_ROOT
	OUTPUT_ROOT=$(MSPK_PLATFORM_PATH)/Build
endif


MSPK_OBJ_ROOT = $(OUTPUT_ROOT)/OBJ

MSPK_OBJ_TYPE=.

MSPK_LIB_EXT=a

#
# Include the platform-agnostic and specific definition files
#
include $(MSPK_ROOT)/Platform/Common/Build/MSPK_Defines.mk

include $(MSPK_ROOT)/Platform/Common/Build/MspkGnuMakeOptions.mk


#
# A build option to indicate whether to use a platform-supplied toolchain,
# or to ignore this and just use the toolchain installed on the Linux
# machine that's doing the build. By default, we use the toolchain installed
# on the Linux build machine, not one we provide.
#
MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN=1

#
# Provide a way, using the MSPK_BUILDOPTIONS environment variable, to
# override the default behavior of MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN
#
ifneq '$(filter USE_SYSTEM_NATIVE_TOOLCHAIN:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN=1
else
ifneq '$(filter USE_SYSTEM_NATIVE_TOOLCHAIN:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN=
endif
endif



#
# Add the MSPK_CDEFINES to the CFLAGS
#
CFLAGS += $(MSPK_CDEFINES)

ifdef IN_PAL_IMPL
CFLAGS += -DMSPK_IN_PAL_IMPL
endif


#
# Build options
#
ifdef MSPKOPTION_DEBUGINFO

# Do not define _DEBUG! Although Visual Studio does this by default, doing so here causes compilation
# and linking errors in the Razzle build, so use DEBUG instead of _DEBUG (IIS OOB #31664).
CFLAGS += -g -g3 -DDEBUG

else
CFLAGS += -DNDEBUG
endif

ifdef MSPKOPTION_OPTIMIZATIONS
CFLAGS += -O3
else
CFLAGS += -O0
endif

ifdef MSPKOPTION_CODECOVERAGE
CFLAGS += -fprofile-arcs -ftest-coverage
endif

#
# Ignore default include and lib files if we're
# using a supplied toolchain
#
ifneq '$(MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN)' '1'
CFLAGS += -nostdinc
CPPFLAGS += -nostdinc++
CXXFLAGS += -nostdinc++
CXXFLAGS += -fno-rtti
PLATLINKFLAGS += -nostdlib -nostartfiles
endif


#
# If we want listing files, also make the linker give verbose output
# (will only go to the log file, so it won't pollute the default build
# output)
#
ifdef MSPKOPTION_LISTINGFILES
PLATLINKFLAGS += -Xlinker --verbose
endif


#
# If we have not specified a target distribution to build for, use
# the default
#
ifeq '$(MSPK_LINUXSAMPLE_TARGET_DISTRO)' ''
MSPK_LINUXSAMPLE_TARGET_DISTRO=Fedora10
endif


#
# Location of the toolchain, and some information about the toolchain
#
ifneq '$(MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN)' '1'
TOOLCHAIN=$(MSPK_TOOLCHAIN)/Toolchain/$(MSPK_PLATFORM)/$(MSPK_LINUXSAMPLE_TARGET_DISTRO)
TOOLCHAIN_TARGET=i386-redhat-linux

ifeq '$(MSPK_LINUXSAMPLE_TARGET_DISTRO)' 'Fedora10'
TOOLCHAIN_VERSION=4.3.2
endif

ifeq '$(MSPK_LINUXSAMPLE_TARGET_DISTRO)' 'Fedora6'
TOOLCHAIN_VERSION=4.1.1
endif

ifeq '$(MSPK_LINUXSAMPLE_TARGET_DISTRO)' 'Ubuntu8.04'
TOOLCHAIN_VERSION=4.2.4
endif

else
#
# If we are using the native toolchain, then it should come off the root
#
TOOLCHAIN=/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc
endif


#
# Add the toolchain's include file path
#
ifneq '$(MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN)' '1'
C_SYSTEM_INCLUDES += $(TOOLCHAIN)/usr/lib/gcc/$(TOOLCHAIN_TARGET)/$(TOOLCHAIN_VERSION)/include
C_SYSTEM_INCLUDES += $(TOOLCHAIN)/usr/local/include
C_SYSTEM_INCLUDES += $(TOOLCHAIN)/usr/include
C_SYSTEM_INCLUDES += $(TOOLCHAIN)/usr/include/linux
endif

#
# Include folders needed for GStreamer
#
#INCLUDES += $(TOOLCHAIN)/usr/include/gstreamer-0.10;
#INCLUDES += $(TOOLCHAIN)/usr/include/glib-2.0;
#INCLUDES += $(TOOLCHAIN)/usr/include/libxml2;
#INCLUDES += $(TOOLCHAIN)/usr/lib/i386-linux-gnu/glib-2.0/include;
#INCLUDES += $(TOOLCHAIN)/usr/lib/glib-2.0/include;

INCLUDES += $(BUILDROOT_SYSROOT_USR_INC_DIR)

#
# Linker options to set the system folders and system libraries for
# the toolchain
#
ifneq '$(MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN)' '1'


#
# Library folders to look in
#
#PLATLINKFLAGS += -L$(TOOLCHAIN)/usr/i386-redhat-linux/lib
#PLATLINKFLAGS += -L$(TOOLCHAIN)/usr/local/lib
PLATLINKFLAGS += -L$(TOOLCHAIN)/lib
PLATLINKFLAGS += -L$(TOOLCHAIN)/usr/lib
PLATLINKFLAGS += -L$(SSPK_TOP_DIR)/../../pub/shared_lib/ 


#
# Shared object library folders to look in
#
PLATLINKFLAGS += -Wl,-rpath,$(TOOLCHAIN)/lib
PLATLINKFLAGS += -Wl,-rpath,$(TOOLCHAIN)/usr/lib


#
# CRT startup modules to link
#
#CRT_STARTUP_OBJS += $(TOOLCHAIN)/usr/lib/gcc/$(TOOLCHAIN_TARGET)/$(TOOLCHAIN_VERSION)/../../../crt1.o
#CRT_STARTUP_OBJS += $(TOOLCHAIN)/usr/lib/gcc/$(TOOLCHAIN_TARGET)/$(TOOLCHAIN_VERSION)/../../../crti.o
#CRT_STARTUP_OBJS += $(TOOLCHAIN)/usr/lib/gcc/$(TOOLCHAIN_TARGET)/$(TOOLCHAIN_VERSION)/../../../crtn.o
#CRT_STARTUP_OBJS += $(TOOLCHAIN)/usr/lib/gcc/$(TOOLCHAIN_TARGET)/$(TOOLCHAIN_VERSION)/crtbegin.o
#CRT_STARTUP_OBJS += $(TOOLCHAIN)/usr/lib/gcc/$(TOOLCHAIN_TARGET)/$(TOOLCHAIN_VERSION)/crtend.o


#
# Libraries to link
#
PLATLINKFLAGS += -lgcc
PLATLINKFLAGS += -lc
PLATLINKFLAGS += -lc_nonshared
PLATLINKFLAGS += -lpthread
PLATLINKFLAGS += -lpthread_nonshared
PLATLINKFLAGS += -lrt
PLATLINKFLAGS += -lgcc_s
#PLATLINKFLAGS += -lgstaudio-0.10
#PLATLINKFLAGS += -lgstbase-0.10
#PLATLINKFLAGS += -lgstinterfaces-0.10
#PLATLINKFLAGS += -lgstreamer-0.10
#PLATLINKFLAGS += -lX11


#$(warning LD_LIBRARY_PATH: $(LD_LIBRARY_PATH))
LD_LIBRARY_PATH = :$(TOOLCHAIN)/usr/lib:$(TOOLCHAIN)/usr/lib:$(TOOLCHAIN)/lib
#$(warning LD_LIBRARY_PATH: $(LD_LIBRARY_PATH))

endif


#
# We will need to link with the librt.a library as well
#
#ifeq '$(MSPKOPTION_USE_SYSTEM_NATIVE_TOOLCHAIN)' '1'
PLATLINKFLAGS += -lrt
#endif

PLATLINKFLAGS += -L$(SSPK_TOP_DIR)/../../pub/shared_lib/ 

#
# Add the pthread library as well
#
PLATLINKFLAGS += -lpthread

PLATLINKFLAGS += -lm  -ldl

#
# Convert the list of system includes into an actual list of C/C++ compiler commmand-
# line options. We use the "-isystem" option instead of the "-I" option for system
# includes because it accomplishes two things for us:
#
# 1. It guarantees that they get included AFTER any "-I" include folders
# 2. It suppressed warnings when compiling these files regarding things like multiply
#    defined types and definitions, stuff inherently present in system include headers
#    that would cause build breaks if not included with this option
#
C_SYSTEM_INCLUDE_FLAGS = $(foreach includepath, $(C_SYSTEM_INCLUDES), -isystem$(includepath))


#
# Use the tools in the toolchain in priority over system tools
#
PATH:=$(TOOLCHAIN)/usr/bin:$(PATH)

#
# debug builds
#
ifdef MSPK_DEBUG
CFLAGS += $(MSPK_DBG_CDEFINES)
endif

###################################################################
##  Define GCC build rules
###################################################################

#
# Define the location that the final result is deposited
#
OUTDIR = $(OUTPUT_ROOT)/BIN


#
# Define the location that object files are built
#
OUTDIROBJ = $(MSPK_OBJ_ROOT)/$(TARGETPATH)


#
# Create an OBJS define that consists of all the *.cpp and *.c
# files, but replacing the extension with *.o
#
OBJT1=$(SOURCES:.cpp=.o)
OBJS1=$(OBJT1:.c=.o)
OBJS=$(patsubst %, $(OUTDIROBJ)/%,$(OBJS1))


#
# Set up the default linker flags
#
# Remember to preface each link flag with '-Xlinker' so that GCC knows who to give it to
#
ifdef MSPKOPTION_CODECOVERAGE
PLATLINKFLAGS += -lgcov
endif



#
# Set up the implicit build rules
#
# These are rules that make uses to build targets from
# dependencies automatically
#
# Note: the "-p" option to mkdir tells the system to
# make the folder quietly: make the parent folders if
# necessary, and don't complain if the folder already
# exists.
#
# Note also that when we build source code files that
# are in the PAL implementation, we compile them as C
# AND as C++ to make sure that we are providing source
# code that has syntax that works both ways.
#
ifdef IN_PAL_IMPL
ifdef MSPKOPTION_LISTINGFILES
# How to build C++ files in the current folder
$(OUTDIROBJ)/%.o:%.cpp
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi

	@/bin/echo -e \\t$(*F).c 1>&3
	$(CC) -x c                $(CFLAGS)                 $<              -o $(@D)/$(*F).c -E $(AUTODEPEND_CPPFLAGS)
	$(CC) -x c -fpreprocessed $(CFLAGS)                 $(@D)/$(*F).c   -o $(@D)/$(*F).s -S
	$(CC)                     $(CFLAGS)                 $(@D)/$(*F).s   -o $(@) -c

	@/bin/echo -e \\t$< 1>&3
	$(CXX) $(CXXFLAGS)                             $<              -o $(@D)/$(*F).cpp -E $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	$(CXX) $(CXXFLAGS)              -fpreprocessed $(@D)/$(*F).cpp -o $(@D)/$(*F).spp -S
	$(CXX) $(CXXFLAGS) -x assembler                $(@D)/$(*F).spp -o $(@) -c

# How to build C++ files in the parent folder
$(OUTDIROBJ)/%.o:../%.cpp
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi

	@/bin/echo -e \\t$(*F).c 1>&3
	$(CC) -x c                $(CFLAGS)                 $<              -o $(@D)/$(*F).c -E $(AUTODEPEND_CPPFLAGS)
	$(CC) -x c -fpreprocessed $(CFLAGS)                 $(@D)/$(*F).c   -o $(@D)/$(*F).s -S
	$(CC)                     $(CFLAGS)                 $(@D)/$(*F).s   -o $(@) -c

	@/bin/echo -e \\t$< 1>&3
	$(CXX) $(CXXFLAGS)                             $<              -o $(@D)/$(*F).cpp -E $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	$(CXX) $(CXXFLAGS)              -fpreprocessed $(@D)/$(*F).cpp -o $(@D)/$(*F).spp -S
	$(CXX) $(CXXFLAGS) -x assembler                $(@D)/$(*F).spp -o $(@) -c

# How to build C files in the current folder
$(OUTDIROBJ)/%.o:%.c
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi

	@/bin/echo -e \\t$(*F).cpp 1>&3
	$(CXX) -x c++                $(CXXFLAGS)              $<              -o $(@D)/$(*F).cpp -E $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	$(CXX) -x c++ -fpreprocessed $(CXXFLAGS)              $(@D)/$(*F).cpp -o $(@D)/$(*F).spp -S
	$(CXX)                       $(CXXFLAGS) -x assembler $(@D)/$(*F).spp -o $(@) -c

	@/bin/echo -e \\t$< 1>&3
	$(CC) $(CFLAGS)                $<              -o $(@D)/$(*F).c -E
	$(CC) $(CFLAGS) -fpreprocessed $(@D)/$(*F).c   -o $(@D)/$(*F).s -S
	$(CC) $(CFLAGS)                $(@D)/$(*F).s   -o $(@) -c

# How to build C files in the parent folder
$(OUTDIROBJ)/%.o:../%.c
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi

	@/bin/echo -e \\t$(*F).cpp 1>&3
	$(CXX) -x c++                $(CXXFLAGS)              $<              -o $(@D)/$(*F).cpp -E $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	$(CXX) -x c++ -fpreprocessed $(CXXFLAGS) -x assembler $(@D)/$(*F).cpp -o $(@D)/$(*F).spp -S
	$(CXX)                       $(CXXFLAGS)              $(@D)/$(*F).spp -o $(@) -c

	@/bin/echo -e \\t$< 1>&3
	$(CC) $(CFLAGS)                $<              -o $(@D)/$(*F).c -E
	$(CC) $(CFLAGS) -fpreprocessed $(@D)/$(*F).c   -o $(@D)/$(*F).s -S
	$(CC) $(CFLAGS)                $(@D)/$(*F).s   -o $(@) -c

else
# How to build C++ files in the current folder
$(OUTDIROBJ)/%.o:%.cpp
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@/bin/echo -e \\t$(*F).c 1>&3
	$(CC) -x c $(CFLAGS)    $< -o $(@) -c $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	@/bin/echo -e \\t$< 1>&3
	$(CXX) $(CXXFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)

# How to build C++ files in the parent folder
$(OUTDIROBJ)/%.o:../%.cpp
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@/bin/echo -e \\t$(*F).c 1>&3
	$(CC) -x c $(CFLAGS)  $< -o $(@) -c $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	@/bin/echo -e \\t$< 1>&3
	$(CXX) $(CXXFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)

# How to compile C files in the current folder
# (and we do them as C++ files)
$(OUTDIROBJ)/%.o:%.c
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@/bin/echo -e \\t$(*F).cpp 1>&3
	$(CXX) -x c++ $(CXXFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)
	@/bin/echo -e \\t$< 1>&3
	$(CC) $(CFLAGS)    $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)

# How to compile C files in the parent folder
# (and we do them as C++ files)
$(OUTDIROBJ)/%.o:../%.c
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	@/bin/echo -e \\t$(*F).cpp 1>&3
	$(CXX) -x c++ $(CXXFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)
	@/bin/echo -e \\t$< 1>&3
	$(CC) $(CFLAGS)    $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)
endif
else
ifdef MSPKOPTION_LISTINGFILES
# How to build C++ files in the current folder
$(OUTDIROBJ)/%.o:%.cpp
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
#	echo LD_LIBRARY_PATH: $$LD_LIBRARY_PATH 1>&3
#	which as
#	which $(CC)
#	touch $(@)
	$(CXX) $(CXXFLAGS)                             $<              -o $(@D)/$(*F).cpp -E $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	$(CXX) $(CXXFLAGS)              -fpreprocessed $(@D)/$(*F).cpp -o $(@D)/$(*F).spp -S
	$(CXX) $(CXXFLAGS) -x assembler                $(@D)/$(*F).spp -o $(@) -c

# How to build C++ files in the parent folder
$(OUTDIROBJ)/%.o:../%.cpp
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CXX) $(CXXFLAGS)                             $<              -o $(@D)/$(*F).cpp -E $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)
	$(CXX) $(CXXFLAGS)              -fpreprocessed $(@D)/$(*F).cpp -o $(@D)/$(*F).spp -S
	$(CXX) $(CXXFLAGS) -x assembler                $(@D)/$(*F).spp -o $(@) -c

# How to build C files in the current folder
$(OUTDIROBJ)/%.o:%.c
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CC) $(CFLAGS)                $<            -o $(@D)/$(*F).c -E $(AUTODEPEND_CPPFLAGS)
	$(CC) $(CFLAGS) -fpreprocessed $(@D)/$(*F).c -o $(@D)/$(*F).s -S
	$(CC) $(CFLAGS)                $(@D)/$(*F).s -o $(@) -c

# How to build C files in the parent folder
$(OUTDIROBJ)/%.o:../%.c
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CC) $(CFLAGS)                $<            -o $(@D)/$(*F).c -E $(AUTODEPEND_CPPFLAGS)
	$(CC) $(CFLAGS) -fpreprocessed $(@D)/$(*F).c -o $(@D)/$(*F).s -S
	$(CC) $(CFLAGS)                $(@D)/$(*F).s -o $(@) -c
else
# How to build C++ files in the current folder
$(OUTDIROBJ)/%.o:%.cpp
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CXX) $(CXXFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)

# How to build C++ files in the parent folder
$(OUTDIROBJ)/%.o:../%.cpp
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CXX) $(CXXFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS) $(if $(filter $<,$(NO_TEMPLATE_INLINE)),-fno-inline)

# How to compile C files in the current folder
# (and we do them as C++ files)
$(OUTDIROBJ)/%.o:%.c
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CC) $(CFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)

# How to compile C files in the parent folder
# (and we do them as C++ files)
$(OUTDIROBJ)/%.o:../%.c
	@/bin/echo -e \\t$< 1>&3
	@if [ ! -d $(@D) ]; then mkdir -p $(@D); fi
	$(CC) $(CFLAGS) $< -o $(@) -c $(AUTODEPEND_CPPFLAGS)
endif
endif


#
# setting up the include environment variable
#
ifdef INCLUDES
ifdef IN_PAL_IMPL
INCLUDES += ;$(MSPK_PLATFORM_PATH)/PALimpl/include;
endif
endif

#
# Parse through the semicolon-delimited $INCLUDES
# variable, and build a new variable with each item
# in a separate "-I $folder" command-line switch
#
# We do this by first removing all spaces, and then
# removing multiple semi-colons with one semi-colon
#
empty :=
space := $(empty) $(empty)
INCLUDEFLAG = $(subst ;, -I,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst ;;,;,$(subst $(space),,;$(INCLUDES)))))))))))).


# Warnings:
# See http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#Warning-Options

#
# define warnings as errors
#
#CFLAGS += -Werror

#
# inhibit all warnings
#
#CFLAGS += -w

#
# enable all good-practice warnings
#
ifeq '$(wildcard /usr/lib/i386-linux-gnu/*.so)' ''
# Ubuntu 10.04 LTS:
CFLAGS += -Wimplicit
else
# Ubuntu 11.x and greater:
CFLAGS += -Wall
endif

#
# Endianness
#
CFLAGS += -DTARGET_LITTLE_ENDIAN=1

#
# Malloc Record
#
C_DEFINES += -DMALLOC_RECORD

#
# AVRenderer
#
C_DEFINES += -DSCALE_PTS_TO_1KHZ

#
# Convert Tests' requests for certain PALs to #defines
#
C_DEFINES += $(subst MSPK_PAL_, -DMSPK_TF_PAL_NEEDED_, $(MSPK_PALS_NEEDED))

#
# Used to choose the correct TSUInt32x32To64 implementation
#
CFLAGS += -D_GNU_64_

#
# Unicode definitions
#
CFLAGS += $(C_DEFINES) -DUNICODE -D_UNICODE $(INCLUDEFLAG) $(C_SYSTEM_INCLUDE_FLAGS)

# Make wchar_t only 16 bits like Windows
# TODO: doesn't work with wstring template library - is there another option for this?
# CFLAGS += -fshort-wchar

#
# C++ definitions should be based on C definitions
#
CPPFLAGS += $(CFLAGS) $(CPP_SYSTEM_INCLUDE_FLAGS)
CXXFLAGS += $(CFLAGS) $(CPP_SYSTEM_INCLUDE_FLAGS)

#
# Enable decltype
#
CPPFLAGS += -std=c++0x
CXXFLAGS += -std=c++0x

MSPK_PAL_LIBS = \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/HALimpl/Crypto/Crypto.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/HALimpl/Decoder/DecoderHal.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/HALimpl/PhysMemMgr/PhysMemMgr.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/Compatibility/StringSafe/StringSafe.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/Compatibility/WinEmulate/WinEmulate.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/PALimpl/Executive/Executive.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/PALimpl/Sockets/Sockets.a \
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/DRMimpl/XDrm/XDrm.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libmt_os.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libAvPlayInstance.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libosal.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libmt_msp.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libmt_common.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libmt_cipher.a \
	$(SSPK_TOP_DIR)/../../pub/static_lib/libplayreadypksoftwaretee.a \

#-lmt_os -lAvPlayInstance -losal -lmt_msp -lmt_common -lmt_cipher -ldl
MSPK_PAL_TESTFRAMEWORK_LIBS =								\
	$(MSPK_OBJ_ROOT)/Platform/$(MSPK_PLATFORM)/TestFramework/TestFramework.a	\


MSPK_PAL_TESTFRAMEWORK_INCLUDES = \
	$(MSPK_PLATFORM_PATH)/TestFramework/main \


#
# Starting with Ubuntu 11.04 some libs are in the /usr/lib/i386-linux-gnu sub-directory.
# Starting with Ubuntu 12.04 all of the libs are now moved down.
#

MSPK_PAL_TESTFRAMEWORK_DYNAMIC_LIBS = \
	/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc/usr/lib/librt.so \
	/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/libc/usr/lib/libpthread.so \
	/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/mipsel-linux-gnu/lib/libstdc++.so.6 \
	$(BUILDROOT_SYSROOT_USR_LIB_DIR)/libz.so \
#
# the default build should build the target
#
mspk_all: $(TARGET)


#
# define actions to take when we wish to clean
#
mspk_clean:
	/bin/echo Cleaning: $(OBJS) $(TARGET)
	-$(RM) -f $(OBJS)
	-$(RM) -f $(TARGET)
	-$(RM) -rf $(OUTDIROBJ)
	-$(RM) -rf $(OUTDIR)


include $(MSPK_ROOT)/Platform/Common/Build/MspkMakeUtils.mk



