include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(MSP_UNF_INCLUDE) -I$(MSP_DRV_INCLUDE) -I$(MSP_API_INCLUDE) -I$(COMMON_UNF_INCLUDE) -I$(COMMON_DRV_INCLUDE) -I$(COMMON_API_INCLUDE)

OBJS := mt_unf_ipc_fs.o

LIB := libmt_unf_ipc_fs

HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_ipcfs_type.h 
include ${SDK_DIR}/build/script/Makefile-lib.rule
