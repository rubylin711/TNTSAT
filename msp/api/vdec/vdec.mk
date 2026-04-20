##################################
# vdec source
##################################
src_vdec :=	\
		vdec/mt_codec.c				\
		vdec/mt_mpi_vdec.c			\
		vdec/mt_mpi_vdec_adapter.c	\

#		vdec/mt_mpi_vdec_mjpeg.c	\
#		vdec/mt_mpi_vdec_vpu.c

##################################
# vdec config
##################################
#MSP_CFLAGS += -DMT_VDEC_VPU_SUPPORT=1
#MSP_CFLAGS += -DMT_VDEC_MJPEG_SUPPORT=1
