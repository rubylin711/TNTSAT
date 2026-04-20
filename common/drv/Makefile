include ${SDK_DIR}/build/script/base.mk



MOD_NAME_CMN   := mt_common
MOD_NAME_MEDIA := mt_media

#obj-$(CONFIG_MT_COMMON) += $(MOD_NAME_CMN).o $(MOD_NAME_MEDIA).o
obj-$(CONFIG_MT_COMMON) += $(MOD_NAME_CMN).o



EXTRA_CFLAGS += $(CFG_MT_KMOD_CFLAGS) -Werror -Wno-date-time -Wuninitialized

EXTRA_CFLAGS += -I$(COMMON_DIR)/inc        \
                -I$(COMMON_DIR)/api/inc    \
                -I$(COMMON_DIR)/drv/inc    \
                -I$(COMMON_DIR)/api/log    \
                -I$(COMMON_DIR)/api/stat   \
                -I$(COMMON_DIR)/api/sys    \
                -I$(COMMON_DIR)/api/mmz    \
                -I$(COMMON_DIR)/drv/module \
                -I$(COMMON_DIR)/drv/mem    \
                -I$(COMMON_DIR)/drv/log    \
                -I$(COMMON_DIR)/drv/mmz    \
                -I$(COMMON_DIR)/api/mem    \
                -I$(MSP_DIR)/inc           \
                -I$(MSP_DIR)/api/inc       \
                -I$(MSP_DIR)/drv/inc       \
                -I$(INCLUDE_DIR)

ifdef CFG_MT_LOG_LEVEL
EXTRA_CFLAGS += -DCFG_MT_LOG_LEVEL=$(CFG_MT_LOG_LEVEL)
else
EXTRA_CFLAGS += -DCFG_MT_LOG_LEVEL=1
endif

ifeq (y,$(CFG_MT_LOG_NETWORK_SUPPORT))
EXTRA_CFLAGS += -DLOG_NETWORK_SUPPORT
endif

ifeq (y,$(CFG_MT_LOG_UDISK_SUPPORT))
EXTRA_CFLAGS += -DLOG_UDISK_SUPPORT
endif

ifeq (y,$(CFG_MT_MEMMGR_SUPPORT))
EXTRA_CFLAGS += -DCMN_MMGR_SUPPORT
endif



$(MOD_NAME_CMN)-y := sys/drv_base_ext_k.o  \
                        sys/drv_sys_ext_k.o   \
                        proc/drv_proc_ext_k.o \
                        log/drv_log_ext_k.o

$(MOD_NAME_CMN)-y += cache/mt_cache.o

$(MOD_NAME_CMN)-y += sys/drv_base_ext.o          \
                        sys/drv_sys_ext.o           \
                        sys/drv_reg_common.o        \
                        sys/mt_ftrace.o             \
                        osal/drv_osal.o	            \
			proc/drv_proc_ext.o         \
			stat/drv_stat_ext_k.o       \
			stat/drv_stat_ext.o         \
                        proc/drv_userproc.o         \
			log/drv_log_ext.o           \
                        module/drv_module_ext.o     \
                        mem/drv_mutils.o            \
                        memdev/drv_memdev.o         \
                        file/drv_file_ext.o         \
                        dump/drv_dump_ext.o

$(MOD_NAME_MEDIA)-y := mtmedia/mtmedia_base.o  \
                          mtmedia/mtmedia.o

ifeq ($(CFG_MT_MEMMGR_SUPPORT),y)
$(MOD_NAME_CMN)-y += mem/drv_mmgr_ext.o mem/drv_mem_ext.o
endif

ifeq (y,$(CFG_MT_TEST_SUPPORT))
EXTRA_CFLAGS += -DCMN_TEST_SUPPORTED
$(MOD_NAME_CMN)-y += test/drv_test_ext.o
endif


$(MOD_NAME_CMN)-y += mmz/drv_mmz_ext_k.o  \
                        mmz/drv_mmz_intf.o   \
                        base/mtdev.o         \
               		base/mtdev_base.o

ifeq ($(CFG_MT_MMZ_V2_SUPPORT),y)
EXTRA_CFLAGS += -DMMZ_V2_SUPPORT
$(MOD_NAME_CMN)-y += mmz/drv_mmz_userdev_v2.o mmz/drv_media_mem_v2.o
else
$(MOD_NAME_CMN)-y += mmz/drv_mmz_userdev.o mmz/drv_media_mem.o
endif
