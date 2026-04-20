##################################
# jpeg source
##################################
src_jpeg_6b :=	\
		jpeg/src_6b/jcapimin.c			\
		jpeg/src_6b/jcapistd.c			\
		jpeg/src_6b/jccoefct.c			\
		jpeg/src_6b/jccolor.c			\
		jpeg/src_6b/jcdctmgr.c			\
		jpeg/src_6b/jchuff.c			\
		jpeg/src_6b/jcinit.c			\
		jpeg/src_6b/jcmainct.c			\
		jpeg/src_6b/jcmarker.c			\
		jpeg/src_6b/jcmaster.c			\
		jpeg/src_6b/jcomapi.c			\
		jpeg/src_6b/jcparam.c			\
		jpeg/src_6b/jcphuff.c			\
		jpeg/src_6b/jcprepct.c			\
		jpeg/src_6b/jcsample.c			\
		jpeg/src_6b/jctrans.c			\
		jpeg/src_6b/jdapimin.c			\
		jpeg/src_6b/jdapistd.c			\
		jpeg/src_6b/jdatadst.c			\
		jpeg/src_6b/jdatasrc.c			\
		jpeg/src_6b/jdcoefct.c			\
		jpeg/src_6b/jdcolor.c			\
		jpeg/src_6b/jdcolor_userbuf.c	\
		jpeg/src_6b/jddctmgr.c			\
		jpeg/src_6b/jdhuff.c			\
		jpeg/src_6b/jdinput.c			\
		jpeg/src_6b/jdmainct.c			\
		jpeg/src_6b/jdmarker.c			\
		jpeg/src_6b/jdmaster.c			\
		jpeg/src_6b/jdmerge.c			\
		jpeg/src_6b/jdphuff.c			\
		jpeg/src_6b/jdpostct.c			\
		jpeg/src_6b/jdsample.c			\
		jpeg/src_6b/jdtrans.c			\
		jpeg/src_6b/jerror.c			\
		jpeg/src_6b/jfdctflt.c			\
		jpeg/src_6b/jfdctfst.c			\
		jpeg/src_6b/jfdctint.c			\
		jpeg/src_6b/jidctflt.c			\
		jpeg/src_6b/jidctfst.c			\
		jpeg/src_6b/jidctint.c			\
		jpeg/src_6b/jidctred.c			\
		jpeg/src_6b/jmemmgr.c			\
		jpeg/src_6b/jmemnobs.c			\
		jpeg/src_6b/jquant1.c			\
		jpeg/src_6b/jquant2.c			\
		jpeg/src_6b/jutils.c			\
		jpeg/src_6b/transupp.c

#		jpeg/src_6b/jpegtran.c			\

src_jpeg_hard :=	\
		jpeg/src_hard/jpeg_hdec_adp.c			\
		jpeg/src_hard/jpeg_hdec_api.c			\
		jpeg/src_hard/jpeg_hdec_csc.c			\
		jpeg/src_hard/jpeg_hdec_mem.c			\
		jpeg/src_hard/jpeg_hdec_rwreg.c			\
		jpeg/src_hard/jpeg_hdec_sentstream.c	\
		jpeg/src_hard/jpeg_hdec_setpara.c		\
		jpeg/src_hard/jpeg_hdec_suspend.c		\
		jpeg/src_hard/jpeg_hdec_table.c			\
		jpeg/src_hard/mt_jpeg_hdec_api.c
src_jpeg := $(src_jpeg_6b) $(src_jpeg_hard)

##################################
# jpeg include
##################################
MSP_INC += $(LOCAL_PATH)/jpeg/grc_cmm_inc
MSP_INC += $(LOCAL_PATH)/jpeg/inc/inc_6b
MSP_INC += $(LOCAL_PATH)/jpeg/inc/inc_hard
MSP_INC += $(LOCAL_PATH)/jpeg/src_6b
MSP_INC += $(LOCAL_PATH)/jpeg/src_hard
MSP_INC += $(LOCAL_PATH)/../drv/jpeg/include
