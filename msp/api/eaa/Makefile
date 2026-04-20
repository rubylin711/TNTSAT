include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)      \
            -I$(COMMON_API_INCLUDE)         \
            -I$(COMMON_DRV_INCLUDE)         \
            -I$(MSP_UNF_INCLUDE)            \
            -I$(MSP_API_INCLUDE)            \
            -I$(MSP_DRV_INCLUDE)            \
            -I$(COMMON_DIR)/inc             \
            -I$(COMMON_DIR)/drv/inc         \
            -I$(COMMON_DIR)/api/inc         \
            -I$(MSP_DIR)/inc                \
            -I$(MSP_DIR)/api/inc            \
            -I$(MSP_DIR)/drv/inc            \
            -I$(MSP_DIR)/api/eaa         \
            -I${SDK_DIR}/pub/inc

LIB := libespeak
CFG_MT_CFLAGS = -D_LZ_LINUX_

ifeq ($(CONFIG_MT_OPTIMIZE_OS),y)
	CFG_MT_CFLAGS += -Os 
	else ifeq ($(CONFIG_MT_OPTIMIZE_O2),y)
	CFG_MT_CFLAGS += -O2 
	else
	CFG_MT_CFLAGS += -O0 
	endif
#don't use -ffast-math, is not safe, it will not check nan

	ifeq ($(CONFIG_MT_DEBUG),y)
	CFG_MT_CFLAGS += -g
	endif

	ifeq ($(CONFIG_MT_INSTRUMENT_FUNCTIONS),y)
	CFG_MT_CFLAGS += -finstrument-functions
	endif

	CFG_MT_CFLAGS += -fno-common

	CFG_MT_CFLAGS += -fno-strict-aliasing

OBJS := compiledict.o dictionary.o espeak.o intonation.o klatt.o numbers.o \
		phonemelist.o readclause.o resample.o setlengths.o sonic.o speak_lib.o \
		synthdata.o synthesize.o tr_languages.o translate.o voices.o wavegen.o

include ${SDK_DIR}/build/script/Makefile-lib.rule                                                                                                                                                                                                           
