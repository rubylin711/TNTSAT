##################################
# frontend source
##################################
src_frontend :=	\
		frontend/mt_unf_frontend.c

ifeq ($(CONFIG_MT_DISEQC_SUPPORT),y)
src_frontend +=	\
		frontend/mt_unf_diseqc.c	\
		frontend/mt_unf_unicable.c

MSP_CFLAGS += -DCONFIG_MT_DISEQC_SUPPORT
endif
