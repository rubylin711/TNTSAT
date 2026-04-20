include ${SDK_DIR}/build/script/base.mk

TARGET := jpeg

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)              \
            -I$(COMMON_API_INCLUDE)                 \
            -I$(COMMON_DRV_INCLUDE)                 \
            -I$(MSP_UNF_INCLUDE)                    \
            -I$(MSP_API_INCLUDE)                    \
            -I$(MSP_DRV_INCLUDE)                    \
            -I$(COMMON_DIR)/inc                     \
            -I$(COMMON_DIR)/drv/inc                 \
            -I$(MSP_DIR)/inc                        \
            -I$(MSP_DIR)/api/inc                    \
            -I$(MSP_DIR)/api/tde/include	    \
            -I$(MSP_DIR)/drv/inc                    \
            -I$(MSP_DIR)/drv/jpeg/include           \
            -I$(MSP_DIR)/drv/jpeg/grc_cmm_inc       \
            -I$(MSP_DIR)/api/jpeg/grc_cmm_inc       \
            -I$(MSP_DIR)/api/$(TARGET)/inc/inc_6b   \
            -I$(MSP_DIR)/api/$(TARGET)/inc/inc_hard \
            -I$(MSP_DIR)/api/$(TARGET)/src_6b       \
            -I$(MSP_DIR)/api/$(TARGET)/src_hard     \
            -I$(INCLUDE_DIR)

LIB := libmt_jpeg

#OBJS := src_6b/jcapimin.o
#OBJS += src_6b/jcapistd.o
#OBJS += src_6b/jccoefct.o
#OBJS += src_6b/jccolor.o
#OBJS += src_6b/jcdctmgr.o
#OBJS += src_6b/jchuff.o
#OBJS += src_6b/jcinit.o
#OBJS += src_6b/jcmainct.o
#OBJS += src_6b/jcmarker.o
#OBJS += src_6b/jcmaster.o
OBJS += src_6b/jcomapi.o
#OBJS += src_6b/jcparam.o
#OBJS += src_6b/jcphuff.o
#OBJS += src_6b/jcprepct.o
#OBJS += src_6b/jcsample.o
#OBJS += src_6b/jctrans.o
OBJS += src_6b/jdapimin.o
OBJS += src_6b/jdapistd.o
#OBJS += src_6b/jdatadst.o
OBJS += src_6b/jdatasrc.o
OBJS += src_6b/jdcoefct.o
OBJS += src_6b/jdcolor.o
OBJS += src_6b/jdcolor_userbuf.o
OBJS += src_6b/jddctmgr.o
OBJS += src_6b/jdhuff.o
OBJS += src_6b/jdinput.o
OBJS += src_6b/jdmainct.o
OBJS += src_6b/jdmarker.o
OBJS += src_6b/jdmaster.o
OBJS += src_6b/jdmerge.o
OBJS += src_6b/jdphuff.o
OBJS += src_6b/jdpostct.o
OBJS += src_6b/jdsample.o
OBJS += src_6b/jdtrans.o
OBJS += src_6b/jerror.o
#OBJS += src_6b/jfdctflt.o
#OBJS += src_6b/jfdctfst.o
#OBJS += src_6b/jfdctint.o
OBJS += src_6b/jidctflt.o
OBJS += src_6b/jidctfst.o
OBJS += src_6b/jidctint.o
OBJS += src_6b/jidctred.o
OBJS += src_6b/jmemmgr.o
OBJS += src_6b/jmemnobs.o
#OBJS += src_6b/jpegtran.o
OBJS += src_6b/jquant1.o
OBJS += src_6b/jquant2.o
OBJS += src_6b/jutils.o
#OBJS += src_6b/transupp.o
OBJS += src_hard/jpeg_hdec_adp.o
OBJS += src_hard/jpeg_hdec_api.o
OBJS += src_hard/jpeg_hdec_csc.o
OBJS += src_hard/jpeg_hdec_mem.o
OBJS += src_hard/jpeg_hdec_rwreg.o
OBJS += src_hard/jpeg_hdec_sentstream.o
OBJS += src_hard/jpeg_hdec_setpara.o
OBJS += src_hard/jpeg_hdec_suspend.o
OBJS += src_hard/jpeg_hdec_table.o
OBJS += src_hard/mt_jpeg_hdec_api.o

HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/inc/inc_hard/mt_jpeg_api.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/inc/inc_6b/mt_jmorecfg.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/inc/inc_6b/mt_jerror.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/inc/inc_6b/mt_jpeglib.h
HEADER_FILES += $(MSP_DIR)/api/$(TARGET)/inc/inc_6b/mt_jconfig.h

include ${SDK_DIR}/build/script/Makefile-lib.rule
