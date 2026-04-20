ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),etude2)) 
CFG_MT_STATIC_LINK = y
else
#CFG_LIBC_STATIC_LINK = y
endif

include ${SDK_DIR}/build/script/base.mk

include $(SAMPLE_DIR)/base.mk

EXTRA_CFLAGS = $(CFG_MT_SAMPLE_CFLAGS)

ifeq ($(CFG_MT_CI_DEV_CIMAX),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAX
endif
ifeq ($(CFG_MT_CI_DEV_CIMAXPLUS),y)
EXTRA_CFLAGS += -DMT_CI_DEV_CIMAXPLUS
endif
ifeq ($(CFG_MT_CI_DEV_HICI),y)
EXTRA_CFLAGS += -DMT_CI_DEV_HICI
endif

INCLUDE_PATH = -I$(SAMPLE_DIR)/common \
          -I$(COMMON_DIR)/inc \
		  -I$(COMMON_DIR)/api/inc \
		  -I$(COMMON_DIR)/drv/inc \
          -I$(INCLUDE_DIR)/testframe \
          -I$(MSP_DIR)/inc \
          -I$(MSP_DIR)/api/inc \
          -I$(MSP_DIR)/drv/inc \
          -I$(INCLUDE_DIR) \
          -I${SDK_DIR}/msp/inc

INCLUDE_PATH += -I$(COMPONENT_DIR)/hdmi20/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_PCApp/platform/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_PCApp/platform/blackbox/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_PCApp/platform/aardvark/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_PCApp/platform/cheetah/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/library/log/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/library/time/ \
				-I$(COMPONENT_DIR)/hdmi20//Tx_Common/library/obj/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/library/malloc/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/library/seq/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/driver/tx/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/driver/tx/videopath/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_PCApp/driver/cra/ \
				-I$(COMPONENT_DIR)/hdmi20/Tx_Common/application/tx/
				
OBJS = hdmi20.o hdmi20_testcase.o sdvenc_testcase.o hdvenc_testcase.o aout_testcase.o

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),etude2)) 
DEPEND_LIBS = $(SYS_LIBS) -lmt_os -lmt_common -lmt_hdmi20 -lmt_msp
else

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),symphony6))
DEPEND_LIBS = $(SYS_LIBS) -lmt_common -lmt_hdmi20 -lmt_msp
endif

endif

ifeq ($(CFG_MT_STATIC_LINK),y)
DEPEND_LIBS_PATH = -L$(STATIC_LIB_DIR)
else
DEPEND_LIBS_PATH = -L$(SHARED_LIB_DIR)
endif

ifeq ($(CFG_MT_CHIP),$(filter $(CFG_MT_CHIP),etude2))
EXTRA_LDFLAGS = -static
endif

APP = hdmi20

include ${SDK_DIR}/build/script/Makefile-app.rule
