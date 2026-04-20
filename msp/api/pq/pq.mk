##################################
# pq source
##################################
src_pq :=	\
		pq/mt_mpi_pq.c	\
		pq/mt_unf_pq.c

##################################
# pq include
##################################
MSP_INC += $(LOCAL_PATH)/pq/include
MSP_INC += $(LOCAL_PATH)/../drv/pq/pq_v3_0/include
