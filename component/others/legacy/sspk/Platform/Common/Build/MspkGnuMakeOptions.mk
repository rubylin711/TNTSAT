##############################################################################
#
# Copyright (C) Microsoft Corporation. All rights reserved.
#
##############################################################################
##############################################################################
#
# If the flavor is not set, then set it to chk
#
##############################################################################
ifeq '$(MSPK_FLAVOR)' ''
MSPK_FLAVOR=chk
endif

##############################################################################
#
# set default build options based on the build flavor
#
##############################################################################
ifeq '$(MSPK_FLAVOR)' 'chk'
MSPKOPTION_ENABLE_ASSERTS=1
MSPKOPTION_OPTIMIZATIONS=
MSPKOPTION_PRINTMSG_ENABLED=1
MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF=1
MSPKOPTION_AVCAPTURE=1
else
ifeq '$(MSPK_FLAVOR)' 'fre'
MSPKOPTION_ENABLE_ASSERTS=
MSPKOPTION_OPTIMIZATIONS=1
MSPKOPTION_PRINTMSG_ENABLED=1
MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF=
MSPKOPTION_AVCAPTURE=
else
ifeq '$(MSPK_FLAVOR)' 'shp'
MSPKOPTION_ENABLE_ASSERTS=
MSPKOPTION_OPTIMIZATIONS=1
MSPKOPTION_PRINTMSG_ENABLED=
MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF=
MSPKOPTION_AVCAPTURE=
endif
endif
endif


##############################################################################
#
# set default build options that are independent of the build flavor
#
##############################################################################
MSPKOPTION_AUTODEPEND=1
MSPKOPTION_CODECOVERAGE=
MSPKOPTION_DEBUGINFO=1
MSPKOPTION_ENABLE_ALL_COMPONENTS_SPEW=
MSPKOPTION_LISTINGFILES=
MSPKOPTION_MAPFILE=1
MSPKOPTION_MEMORYTRACKING=
MSPKOPTION_PROFILEINFO=
MSPKOPTION_REMOVE_CODE_FROM_RELEASE=
MSPKOPTION_ENABLE_APP=0

##############################################################################
#
# Provide a way for the developer to override the default behavior
# of the build options
#
##############################################################################
ifneq '$(filter AUTODEPEND:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_AUTODEPEND=1
else
ifneq '$(filter AUTODEPEND:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_AUTODEPEND=
endif
endif

ifneq '$(filter CODECOVERAGE:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_CODECOVERAGE=1
else
ifneq '$(filter CODECOVERAGE:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_CODECOVERAGE=
endif
endif

ifneq '$(filter ALL_COMPONENTS_SPEW:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_ENABLE_ALL_COMPONENTS_SPEW=1
else
ifneq '$(filter ALL_COMPONENTS_SPEW:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_ENABLE_ALL_COMPONENTS_SPEW=
endif
endif

ifneq '$(filter DEBUGINFO:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_DEBUGINFO=1
else
ifneq '$(filter DEBUGINFO:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_DEBUGINFO=
endif
endif


ifneq '$(filter ENABLE_ASSERTS:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_ENABLE_ASSERTS=1
else
ifneq '$(filter ENABLE_ASSERTS:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_ENABLE_ASSERTS=
endif
endif


ifneq '$(filter LISTINGFILES:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_LISTINGFILES=1
else
ifneq '$(filter LISTINGFILES:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_LISTINGFILES=
endif
endif


ifneq '$(filter MAPFILE:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_MAPFILE=1
else
ifneq '$(filter MAPFILE:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_MAPFILE=
endif
endif


ifneq '$(filter MEMORYTRACKING:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_MEMORYTRACKING=1
else
ifneq '$(filter MEMORYTRACKING:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_MEMORYTRACKING=
endif
endif


ifneq '$(filter OPTIMIZATIONS:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_OPTIMIZATIONS=1
else
ifneq '$(filter OPTIMIZATIONS:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_OPTIMIZATIONS=
endif
endif


ifneq '$(filter PRINTMSG_ENABLED:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_PRINTMSG_ENABLED=1
else
ifneq '$(filter PRINTMSG_ENABLED:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_PRINTMSG_ENABLED=
endif
endif


ifneq '$(filter PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF=1
else
ifneq '$(filter PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF=
endif
endif


ifneq '$(filter PROFILEINFO:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_PROFILEINFO=1
else
ifneq '$(filter PROFILEINFO:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_PROFILEINFO=
endif
endif


ifneq '$(filter AVCAPTURE:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_AVCAPTURE=1
else
ifneq '$(filter AVCAPTURE:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_AVCAPTURE=
endif
endif


ifneq '$(filter REMOVE_CODE_FROM_RELEASE:ON, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_REMOVE_CODE_FROM_RELEASE=1
else
ifneq '$(filter REMOVE_CODE_FROM_RELEASE:OFF, $(MSPK_BUILDOPTIONS))' ''
MSPKOPTION_REMOVE_CODE_FROM_RELEASE=
endif
endif

##############################################################################
# Definition for the build flavors
##############################################################################
C_DEFINES += -DMSPK_FLAVOR=$(MSPK_FLAVOR)


##############################################################################
#
# Now, go through the options, and convert options that require a C/C++
# macro definition that the PAK expects into the corresponding definition
# in C_DEFINES
#
# Note that we don't do things like set compiler build options for things
# like optimizations here, because while they are usually pretty consistent
# with Gnu compilers, they still are platform specific, and some platforms
# may want to tweak the way these work.
#
##############################################################################
#
# First step: clear ALL #define's that we may or may not need for the PAK
# so that they are not defined if we don't want them, and are not multiply
# defined when we add them below if we do need them
#
##############################################################################
C_DEFINES := $(filter-out -D_DEBUG,$(C_DEFINES))
C_DEFINES := $(filter-out -DMCX_BUILDOPTION_MEMORYTRACKING,$(C_DEFINES))
C_DEFINES := $(filter-out -DNDEBUG,$(C_DEFINES))
C_DEFINES := $(filter-out -DPRINTMSG_ENABLED,$(C_DEFINES))
C_DEFINES := $(filter-out -DPRINTMSG_USE_EXECUTIVE_DEBUGPRINTF,$(C_DEFINES))
C_DEFINES := $(filter-out -DSUPPORT_AV_CAPTURE,$(C_DEFINES))


#
# MSPKOPTION_AUTODEPEND:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_CODECOVERAGE:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_ENABLE_ALL_COMPONENTS_SPEW:
# Defines: ENABLE_ALL_COMPONENTS_SPEW
#
ifeq '$(MSPKOPTION_ENABLE_ALL_COMPONENTS_SPEW)' '1'
C_DEFINES += -DENABLE_ALL_COMPONENTS_SPEW
endif


#
# MSPKOPTION_DEBUGINFO:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_ENABLE_ASSERTS:
# Defines: DEBUG (and NDEBUG otherwise)
#
ifeq '$(MSPKOPTION_ENABLE_ASSERTS)' '1'

# Do not define _DEBUG! Although Visual Studio does this by default, doing so here causes compilation
# and linking errors in the Razzle build, so use DEBUG instead of _DEBUG (IIS OOB #31664).
C_DEFINES += -DDEBUG

else
C_DEFINES += -DNDEBUG
endif



#
# MSPKOPTION_LISTINGFILES:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_MAPFILE:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_MEMORYTRACKING:
# Memory Tracking macros and functionality. Requires platform support.
#
ifeq '$(MSPKOPTION_MEMORYTRACKING)' '1'
C_DEFINES += -DMCX_BUILDOPTION_MEMORYTRACKING
endif




#
# MSPKOPTION_OPTIMIZATIONS:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_PRINTMSG_ENABLED:
# Defines: PRINTMSG_ENABLED
#
ifeq '$(MSPKOPTION_PRINTMSG_ENABLED)' '1'
C_DEFINES += -DPRINTMSG_ENABLED
endif


#
# MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF:
# Defines: PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF
# Will cause PrintMsg to do an Executive_DebugPrintf
#
ifeq '$(MSPKOPTION_PRINTMSG_USE_EXECUTIVE_DEBUGPRINTF)' '1'
C_DEFINES += -DPRINTMSG_USE_EXECUTIVE_DEBUGPRINTF
endif


#
# MSPKOPTION_PROFILEINFO:
# Platform-specific make option. No C/C++ definition necessary
#


#
# MSPKOPTION_AVCAPTURE:
# Defines: SUPPORT_AV_CAPTURE
#
ifeq '$(MSPKOPTION_AVCAPTURE)' '1'
C_DEFINES += -DSUPPORT_AV_CAPTURE
endif


#
# MSPKOPTION_REMOVE_CODE_FROM_RELEASE:
# Defines: SSPK_REMOVE_CODE_FROM_RELEASE
#
ifeq '$(MSPKOPTION_REMOVE_CODE_FROM_RELEASE)' '1'
C_DEFINES += -DREMOVE_CODE_FROM_RELEASE
endif

#
# MSPKOPTION_ENABLE_APP:
# Defines: MSPKOPTION_ENABLE_APP
#
ifeq '$(MSPKOPTION_ENABLE_APP)' '1'
C_DEFINES += -DENABLE_APP
endif

##############################################################################
#
# add build options for Components spew control from MSPK_SPEWOPTIONS environment variable
#
##############################################################################

ifneq '$(filter CDIAGS_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DCDIAGS_TRACE_ENABLE
endif

ifneq '$(filter DECODER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DDECODER_SPEW
endif

ifneq '$(filter DRMMANAGER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DDRMMANAGER_SPEW
endif

ifneq '$(filter CHUNKMANIFEST_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DCHUNKMANIFEST_SPEW
endif

ifneq '$(filter MSD_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DMSD_SPEW
endif

ifneq '$(filter HEURISTICS_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DHEURISTICS_SPEW
endif

ifneq '$(filter HEURISTICS_EXTRA_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DHEURISTICS_EXTRA_SPEW
endif

ifneq '$(filter MANIFESTPARSER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DMANIFESTPARSER_SPEW
endif

# TODO: change DEBUG_SPEW_PROPERTIES to MP4_PROPERTIES_SPEW

ifneq '$(filter MP4_PROPERTIES_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DDEBUG_SPEW_PROPERTIES
endif

# TODO: change DEBUG_SPEW_FRAME_INFO to MP4_FRAME_INFO_SPEW

ifneq '$(filter MP4_FRAME_INFO_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DDEBUG_SPEW_FRAME_INFO
endif

ifneq '$(filter LANGUAGE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DLANGUAGE_SPEW
endif

ifneq '$(filter FACTORY_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DFACTORY_SPEW
endif

ifneq '$(filter RECEIVER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DRECEIVER_SPEW
endif

ifneq '$(filter RATECONTROL_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DRATECONTROL_SPEW
endif

ifneq '$(filter ACCESSCONTROL_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DACCESSCONTROL_SPEW
endif

ifneq '$(filter CLOCK_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DCLOCK_SPEW
endif

ifneq '$(filter CLOCK_EXTRA_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DCLOCK_EXTRA_SPEW
endif

ifneq '$(filter RENDERER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DRENDERER_SPEW
endif

ifneq '$(filter RENDERER_SPEW_EXTRA, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DRENDERER_SPEW_EXTRA
endif

ifneq '$(filter STREAMINFO_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSTREAMINFO_SPEW
endif

ifneq '$(filter TIMESLICE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DTIMESLICE_SPEW
endif

ifneq '$(filter SOCKET_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSOCKET_SPEW
endif

ifneq '$(filter SOCKETMBR_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSOCKETMBR_SPEW
endif

ifneq '$(filter SOCKETMBR_SPEW_EXTRA, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSOCKETMBR_SPEW_EXTRA
endif

ifneq '$(filter SOCKETMBRMANIFEST_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSOCKETMBRMANIFEST_SPEW
endif

ifneq '$(filter SOCKETMBRMANIFEST_SPEW_VERBOSE, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSOCKETMBRMANIFEST_SPEW_VERBOSE
endif

ifneq '$(filter THUMBNAIL_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DTHUMBNAIL_SPEW
endif

ifneq '$(filter STREAMER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSTREAMER_SPEW
endif

ifneq '$(filter SmoothTransport_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSmoothTransport_SPEW
endif

ifneq '$(filter SmoothTransportSTATUSHANDLER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSmoothTransportSTATUSHANDLER_SPEW
endif

ifneq '$(filter FRAGMENTFETCHER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DFRAGMENTFETCHER_SPEW
endif

ifneq '$(filter SEGMENTMANIFESTFETCHER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DSEGMENTMANIFESTFETCHER_SPEW
endif

##############################################################################
#
# add build options for HAL and PAL spew control from MSPK_SPEWOPTIONS environment variable
#
##############################################################################

ifneq '$(filter HAL_DECODER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DHAL_TRACE_ENABLE_DECODER
endif

ifneq '$(filter HAL_CLOCK_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DHAL_TRACE_ENABLE_CLOCK
endif

ifneq '$(filter HAL_CLOCK_GETTIME_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DHAL_TRACE_ENABLE_CLOCK_GETTIME
C_DEFINES += -DHAL_TRACE_ENABLE_CLOCK
endif

ifneq '$(filter PAL_ERROR_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_ERROR=1
endif

ifneq '$(filter PAL_WARNING_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_WARNING=1
endif

ifneq '$(filter PAL_AVRENDERER_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_AVRENDERER=1
endif

ifneq '$(filter PAL_AVRENDERER_VERBOSE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_AVRENDERER_VERBOSE=1
endif

ifneq '$(filter PAL_AVRENDERER_BUFFERPOOL_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_AVRENDERER_BUFFERPOOL=1
endif

ifneq '$(filter PAL_AVRENDERER_BUFFERPOOL_VERBOSE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_AVRENDERER_BUFFERPOOL_VERBOSE=1
endif

ifneq '$(filter PAL_EXECUTIVE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_EXECUTIVE=1
endif

ifneq '$(filter PAL_EXECUTIVE_VERBOSE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_EXECUTIVE_VERBOSE=1
endif

ifneq '$(filter PAL_SOCKETS_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_SOCKETS=1
endif

ifneq '$(filter PAL_SOCKETS_VERBOSE_, $(MSPK_SPEWOPTIONS))' ''
C_DEFINES += -DPALPRINT_SOCKETS_VERBOSE=1
endif

##############################################################################
#
# Define the list of all PALs that tests may or may not need. It's up to the
# platform's build files to determine how to use this information, if at all.
#
# NOTE: We don't want to actually define these here, because this makefile
# include is included AFTER build.pk, so that would effectively override
# the test's settings.
#
##############################################################################
#MSPK_PALS_NEEDED =                                             \
#       MSPK_PAL_EXECUTIVE                                      \
#       MSPK_PAL_SOCKETS                                        \
#


