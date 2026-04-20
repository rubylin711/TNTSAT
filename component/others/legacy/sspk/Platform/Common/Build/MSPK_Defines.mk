#####################################################################################
#
# Copyright (C) Microsoft Corporation. All rights reserved.
#
#####################################################################################
##
# All platforms must define the following environment variables:
#
# MSPK_ROOT - location of sources tree (e.g. SSPK/Source)
# MSPK_PLATFORM - name of platform directory  (e.g. LinuxStub)
##

#
# Define paths useful in the kit based on required enviroment variables
#

# Defines full path to Platform directory
MSPK_PLATFORM_PATH=$(MSPK_ROOT)/Platform/$(MSPK_PLATFORM)

# Defines full path to components directory
MSPK_COMPONENTS_ROOT=$(MSPK_ROOT)/Components

# Defines full path to PAL directory
MSPK_PAL_ROOT=$(MSPK_ROOT)/Platform/inc

# Defines full path to the common includes directory
MSPK_COMMON_INCLUDES=$(MSPK_ROOT)/inc


#####################################################################################
# MSPK_PAL_INCLUDES
#
# Defines the list of default common include folders that PAL implementation modules
# have access to.
#
# This includes, in the following order (for overrides):
#  > the Platform's public include folder
#  > the PAL folder (for PAL interface headers)
#  > the PAK's common includes for public interfaces to the components
#
#####################################################################################
MSPK_PAL_INCLUDES = \
    $(MSPK_PLATFORM_PATH)/inc; \
    $(MSPK_PAL_ROOT); \
    $(MSPK_COMMON_INCLUDES); \


#####################################################################################
# Include paths for test projects
#####################################################################################

MSPK_TEST_INCLUDES =                                                                 \
    $(MSPK_ROOT)/Test/inc;                                                          \


#####################################################################################
# Lib Paths for TestFramework libs
#####################################################################################

MSPK_TEST_LIBS = \
	$(MSPK_OBJ_ROOT)/Test/Library/Common/$(MSPK_OBJ_TYPE)/TLCommon.$(MSPK_LIB_EXT)                      \


#####################################################################################
# EVERY lib defined in this file must be referenced here in case a platform needs to know them for name conversion
#####################################################################################
MSPK_PAK_ALL_COMMON_LIBS =										\
	$(MSPK_TEST_LIBS)                                                                                \


#####################################################################################
# Libs for Components projects
#####################################################################################
MSPK_COMPONENT_LIBS = \
	$(MSPK_OBJ_ROOT)/Components/Decoder/$(MSPK_OBJ_TYPE)/Decoder.$(MSPK_LIB_EXT)	\
 	$(MSPK_OBJ_ROOT)/Components/DRM/$(MSPK_OBJ_TYPE)/DRM.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/MBR/$(MSPK_OBJ_TYPE)/MBR.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/MP4Parser/$(MSPK_OBJ_TYPE)/MP4Parser.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/Receiver/$(MSPK_OBJ_TYPE)/Receiver.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/Socket/$(MSPK_OBJ_TYPE)/Socket.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/Renderer/$(MSPK_OBJ_TYPE)/Renderer.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/Streamer/$(MSPK_OBJ_TYPE)/Streamer.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/Transport/$(MSPK_OBJ_TYPE)/Transport.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Components/Utilities/$(MSPK_OBJ_TYPE)/Utilities.$(MSPK_LIB_EXT) \


#####################################################################################
# Libs for Test Integration projects
#####################################################################################
MSPK_INTEGRATION_TEST_LIBS=\
	$(MSPK_OBJ_ROOT)/Test/Apps/Integration/MediaTransportBaseTest/$(MSPK_OBJ_TYPE)/MediaTransportBaseTest.$(MSPK_LIB_EXT) \
	$(MSPK_OBJ_ROOT)/Test/Apps/Integration/MediaTransportTestFramework/$(MSPK_OBJ_TYPE)/MediaTransportTestFramework.$(MSPK_LIB_EXT) \

