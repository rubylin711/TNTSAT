include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)              \
            -I$(COMMON_API_INCLUDE)                 \
            -I$(COMMON_DRV_INCLUDE)                 \
            -I$(MSP_UNF_INCLUDE)                    \
            -I$(MSP_API_INCLUDE)                    \
            -I$(MSP_DRV_INCLUDE)                    \
            -I$(COMMON_DIR)/inc                     \
            -I$(COMMON_DIR)/drv/inc                 \
            -I$(COMMON_DIR)/api/inc                 \
            -I$(MSP_DIR)/inc                        \
            -I$(MSP_DIR)/api/inc                    \
            -I$(MSP_DIR)/drv/pq/pq_v3_0/include     \
            -I$(MSP_DIR)/drv/inc                    \
            -I$(INCLUDE_DIR)

OBJS := mt_mpi_disp_tran.o
OBJS += mt_mpi_disp.o
OBJS += mt_mpi_win.o
OBJS += mt_mpi_edid.o
OBJS += mt_unf_display.o
OBJS += mt_unf_vo.o
OBJS += mt_unf_edid.o
OBJS += mt_sdvenc_macv.o

HEADER_FILES := $(MSP_UNF_INCLUDE)/mt_unf_disp.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_vo.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_edid.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_common.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_error_mpi.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_ecs.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_mce.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_pdm.h

include $(MSP_DIR)/api/Makefile.rule
