include ${SDK_DIR}/build/script/base.mk

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
            -I$(MSP_DIR)/drv/inc                    \
            -I$(MSP_DIR)/../common/api/inc          \
            -I$(MSP_DIR)/drv/demux                  \
            -I$(INCLUDE_DIR)

OBJS := mt_mpi_demux.o     \
        mt_unf_demux.o

OBJS += mt_mpi_descrambler.o   \
        mt_unf_descrambler.o

OBJS += mpi_demux_pts_descriptor.o

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_unf_demux.h       \
                $(MSP_UNF_INCLUDE)/mt_unf_descrambler.h

include $(MSP_DIR)/api/Makefile.rule
