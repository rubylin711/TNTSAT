include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_API_INCLUDE)      \
            -I$(COMMON_DRV_INCLUDE)         \
            -I$(MSP_API_INCLUDE)            \
            -I$(MSP_DRV_INCLUDE)            \
            -I$(COMMON_DIR)/inc             \
            -I$(MSP_DIR)/inc                \
            -I$(INCLUDE_DIR)

OBJS := mt_mpi_sync.o

include $(MSP_DIR)/api/Makefile.rule
