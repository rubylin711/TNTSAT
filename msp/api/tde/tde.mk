##################################
# tde source
##################################
src_tde_driver :=	\
		tde/src/gpe/driver/gpe.c
src_tde_driver += tde/src/gpe/symphony_soft/gpe_hw.c
src_tde_driver += tde/src/gpe/symphony_soft/scale.c
src_tde :=	\
		$(src_tde_driver)	\
		tde/src/mt_unf_tde.c

##################################
# tde include
##################################
MSP_INC += $(LOCAL_PATH)/tde/include
MSP_INC += $(LOCAL_PATH)/tde/src/gpe/driver
MSP_INC += $(LOCAL_PATH)/tde/src/gpe/symphony_soft
