include ${SDK_DIR}/build/script/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)

INCLUDE_PATH = -I${SDK_DIR}/common/inc -I${SDK_DIR}/common/api/inc -I${SDK_DIR}/common/drv/inc
INCLUDE_PATH += -I$(CURDIR)/include
INCLUDE_PATH += -I$(CURDIR)/
INCLUDE_PATH += -I$(CURDIR)/Tx_PCApp/platform/ \
				-I$(CURDIR)/Tx_PCApp/platform/blackbox/ \
				-I$(CURDIR)/Tx_PCApp/platform/aardvark/ \
				-I$(CURDIR)/Tx_PCApp/platform/cheetah/ \
				-I$(CURDIR)/Tx_PCApp/driver/cra/ \
				-I$(CURDIR)/Tx_PCApp/driver/pebbles/ \
				-I$(CURDIR)/Tx_PCApp/application/tx/api_test/ \
				-I$(CURDIR)/Tx_Common/application/cec/ \
				-I$(CURDIR)/Tx_Common/application/tx/ \
				-I$(CURDIR)/Tx_PCApp/application/SiIMonHandle/ \
				-I$(CURDIR)/Tx_Common/driver/tx/ \
				-I$(CURDIR)/Tx_Common/driver/tx/videopath/ \
				-I$(CURDIR)/Tx_Common/driver/tx/tx_hdmi/ \
				-I$(CURDIR)/Tx_Common/driver/tx/hdcp/ \
				-I$(CURDIR)/Tx_Common/driver/tx/scdc/ \
				-I$(CURDIR)/Tx_Common/driver/cpi/ \
				-I$(CURDIR)/Tx_Common/driver/pll/ \
				-I$(CURDIR)/Tx_Common/driver/tpg/ \
				-I$(CURDIR)/Tx_Common/driver/vtg/ \
				-I$(CURDIR)/Tx_Common/library/seq/ \
				-I$(CURDIR)/Tx_Common/library/video/ \
				-I$(CURDIR)/Tx_Common/library/log/ \
				-I$(CURDIR)/Tx_Common/library/time/ \
				-I$(CURDIR)/Tx_Common/library/obj/ \
				-I$(CURDIR)/Tx_Common/library/malloc/ \
				-I$(CURDIR)/Tx_Common/library/seq/ \
				-I$(CURDIR)/Tx_Common/library/edid/ \
				-I$(CURDIR)/Tx_Common/library/mtx/ \
				-I$(CURDIR)/Tx_Common/library/video/ \
				-I$(KWARE_DIR)/include/mtos/

LIB := libmt_hdmi20

OBJS :=  \
		Tx_Common/application/cec/si_app_cec.o \
		Tx_Common/application/tx/si_app_tx.o
		
OBJS +=	Tx_Common/driver/cpi/si_drv_cpi.o

OBJS +=	Tx_Common/driver/pll/si_drv_pll_vo.o \
		Tx_Common/driver/pll/si_drv_pll_vo_cfg.o
		
OBJS +=	Tx_Common/driver/tpg/si_drv_tpg.o

OBJS +=	Tx_Common/driver/tx/si_drv_tx.o

OBJS +=	Tx_Common/driver/tx/hdcp/si_mod_tx_hdcp.o
OBJS +=	Tx_Common/driver/tx/scdc/si_mod_tx_scdc.o
OBJS +=	Tx_Common/driver/tx/tx_hdmi/si_mod_tx_hdmi.o
OBJS +=	Tx_Common/driver/tx/videopath/si_mod_tx_videopath.o \
		Tx_Common/driver/tx/videopath/si_mod_tx_videopath_cra.o
OBJS +=	Tx_Common/driver/vtg/si_drv_vtg.o

OBJS +=	Tx_Common/library/edid/si_lib_edid.o
OBJS +=	Tx_Common/library/log/si_lib_log.o
OBJS +=	Tx_Common/library/malloc/si_lib_malloc.o
OBJS +=	Tx_Common/library/mtx/si_lib_mtx.o
OBJS +=	Tx_Common/library/obj/si_lib_obj.o
OBJS +=	Tx_Common/library/seq/si_lib_seq.o
OBJS +=	Tx_Common/library/time/si_lib_time.o
OBJS +=	Tx_Common/library/video/si_lib_video.o

#OBJS +=	Tx_PCApp/application/SiIMonHandle/si_siimon_handle.o
OBJS +=	Tx_PCApp/application/tx/api_test/si_test_set_infoframes.o
OBJS +=	Tx_PCApp/application/tx/api_test/si_tx_api_testing.o
OBJS +=	Tx_PCApp/application/tx/api_test/mt_hdmi20_testcase.o
OBJS += Tx_PCApp/application/tx/api_test/mt_aout_testcase.o
OBJS +=	Tx_PCApp/driver/cra/si_drv_cra.o
OBJS +=	Tx_PCApp/driver/cra/misc_reg_op.o
OBJS +=	Tx_PCApp/driver/pebbles/si_drv_pebbles.o
OBJS +=	Tx_PCApp/platform/platform.o \
		Tx_PCApp/platform/sii_time.o \
		Tx_PCApp/platform/aardvark/aardvark.o \
		Tx_PCApp/platform/cheetah/cheetah.o \
		Tx_PCApp/platform/blackbox/BlackBox.o \
		Tx_PCApp/platform/blackbox/conio.o

HEADER_FILES_INSTALL_SUBDIR=hdmi20

include ${SDK_DIR}/build/script/Makefile-lib.rule
