include ${SDK_DIR}/build/script/base.mk

INCLUDE_PATH = -I$(COMMON_UNF_INCLUDE)             \
            -I$(COMMON_API_INCLUDE)                 \
            -I$(COMMON_DRV_INCLUDE)                 \
            -I$(MSP_UNF_INCLUDE)                    \
            -I$(MSP_API_INCLUDE)                    \
            -I$(MSP_API_VPU_VPUAPI_INCLUDE)         \
            -I$(MSP_API_VPU_VDI_INCLUDE)            \
            -I$(MSP_API_JPEG_INCLUDE_INC6B)         \
            -I$(MSP_API_JPEG_INCLUDE_INC_HARD)      \
            -I$(MSP_DRV_INCLUDE)                    \
            -I$(MSP_DRV_VFMW_VFMW_V4_INCLUDE)       \
            -I$(COMMON_DIR)/inc                     \
            -I$(COMMON_DIR)/drv/inc                 \
            -I$(COMMON_DIR)/api/inc                 \
            -I$(MSP_DIR)/inc                        \
            -I$(MSP_DIR)/api/inc                    \
            -I$(MSP_DIR)/drv/inc                    \
            -I$(INCLUDE_DIR)

ifeq ($(CFG_MT_CHIP),aria)
EXTRA_CFLAGS = -DMT_VDEC_MJPEG_SUPPORT=1
endif


OBJS := mt_codec.o
OBJS +=	mt_mpi_vdec.o
OBJS +=	mt_mpi_vdec_lcevc.o
OBJS +=	mt_mpi_vdec_adapter.o
ifeq ($(CFG_MT_CHIP),aria)
OBJS +=	mt_mpi_vdec_vpu.o
OBJS +=	mt_mpi_vdec_mjpeg.o
endif

MSP_API_VPU_VPUAPI_INCLUDE = $(MSP_DIR)/inc/vpu/vpuapi
MSP_API_VPU_VDI_INCLUDE = $(MSP_DIR)/api/vpu/vdi
MSP_API_JPEG_INCLUDE_INC6B = $(MSP_DIR)/api/jpeg/inc/inc_6b
MSP_API_JPEG_INCLUDE_INC_HARD = $(MSP_DIR)/api/jpeg/inc/inc_hard
MSP_DRV_VFMW_VFMW_V4_INCLUDE = $(MSP_DIR)/drv/vfmw/video_lib_1.0

HEADER_FILES := $(MSP_API_VPU_VPUAPI_INCLUDE)/vpuconfig.h
HEADER_FILES += $(MSP_API_VPU_VPUAPI_INCLUDE)/vpuapi.h
HEADER_FILES += $(MSP_API_VPU_VPUAPI_INCLUDE)/vpuapifunc.h
HEADER_FILES += $(MSP_UNF_INCLUDE)/mt_unf_video.h

include $(MSP_DIR)/api/Makefile.rule
