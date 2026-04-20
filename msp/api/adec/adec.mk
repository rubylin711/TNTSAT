##################################
# adec source
##################################


src_adec :=	\
		adec/mpi_adec.c				\
	

##################################
# adec config
##################################
MSP_CFLAGS += -DMT_ADEC_AUDSPECTRUM_SUPPORT
