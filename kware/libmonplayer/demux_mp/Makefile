include ${SDK_DIR}/build/script/base.mk

include $(KWARE_DIR)/libmonplayer/base.mk

EXTRA_CFLAGS = $(CFG_MT_KWARE_CFLAGS)

EXTRA_CFLAGS += -D__LINUX__ -D__ANDRIOD__ -DHWDISABLE -DENABLE_DEMUX_HTTP

ifeq ($(WITH_TCPIP_PROTOCOL),y)
EXTRA_CFLAGS += -DWITH_TCPIP_PROTOCOL
endif

ifeq ($(ENABLE_OPEN_HTTPS),y)
EXTRA_CFLAGS += -DENABLE_OPEN_HTTPS
endif

ifeq ($(CFG_FFMPEG_VER_NUM),422)
INCLUDE_PATH += -I$(BUILDROOT_SYSROOT_USR_INC_DIR)
endif

INCLUDE_PATH += -Ilive555/liveMedia/include \
        -Ilive555/groupsock/include \
        -Ilive555/UsageEnvironment/include \
        -Ilive555/BasicUsageEnvironment/include

INCLUDE_PATH += \
        -I$(KWARE_DIR)/include/mtos \
        -I$(KWARE_DIR)/libmonplayer/include \
        -I$(KWARE_DIR)/libmonplayer/ts_seq \
        -I$(KWARE_DIR)/libmonplayer/osal \
        -I$(KWARE_DIR)/libmonplayer/file_playback \
        -I$(KWARE_DIR)/libmonplayer/include/file_playback \
        -I$(KWARE_DIR)/libmonplayer/include/file_playback \
        -I$(KWARE_DIR)/libmonplayer/moplayer \
        -I$(KWARE_DIR)/libmonplayer/moplayer/include \
        -I$(KWARE_DIR)/libmonplayer/include/demux_mp \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/ffmpeg \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/libmpdemux \
        -I$(KWARE_DIR)/libmonplayer/demux_mp/mplayer/stream \
        -I$(COMMON_UNF_INCLUDE) \
	-I$(MSP_API_INCLUDE) \
        -I$(MSP_UNF_INCLUDE)

ifeq ($(CONFIG_MT_DRM_SUPPORT), y)
EXTRA_CFLAGS += -DMT_DRM_SUPPORT
INCLUDE_PATH += -I${EXTERNAL_BINARY_PREFIX_DIR}/drm
endif

ifeq ($(CONFIG_MT_DRM_TEE_SUPPORT),y)
EXTRA_CFLAGS +=  -DDRM_SMP_ENABLE
endif

OBJS := demux_mp.o \
        demux_mp_misc.o \
        pts_list.o \
        mplayer/bstr.o \
        mplayer/path.o \
        mplayer/av_helpers.o \
        mplayer/mpcommon.o \
        mplayer/mp_msg.o \
        mplayer/mp_strings.o \
        mplayer/mp_func_trans.o \
        mplayer/m_struct.o \
        mplayer/m_option.o \
        mplayer/av_opts.o \
        mplayer/libmpdemux/mp3_hdr.o \
        mplayer/libmpdemux/demuxer.o \
        mplayer/libmpdemux/parse_es.o \
        mplayer/libmpdemux/aac_hdr.o \
        mplayer/libmpdemux/demux_aac.o \
        mplayer/libmpdemux/demux_lavf.o \
        mplayer/libmpdemux/demux_demuxers.o \
        mplayer/libmpdemux/demux_ts.o \
        mplayer/libmpdemux/demux_audio.o \
        mplayer/libmpdemux/mp_taglists.o \
        mplayer/libmpdemux/aviprint.o \
        mplayer/libmpdemux/extension.o \
        mplayer/libmpdemux/mpeg_hdr.o \
        mplayer/libmpdemux/demux_mpg.o \
        mplayer/libmpdemux/asfheader.o \
        mplayer/libass/ass_fontconfig.o \
        mplayer/libass/ass.o \
        mplayer/libass/ass_drawing.o \
        mplayer/libass/ass_shaper.o \
        mplayer/libass/ass_font.o \
        mplayer/libass/ass_utils.o \
        mplayer/libass/ass_render.o \
        mplayer/libass/ass_render_api.o \
        mplayer/libass/ass_library.o \
        mplayer/libass/ass_bitmap.o \
        mplayer/libass/ass_cache.o \
        mplayer/libass/ass_parse.o \
        mplayer/libass/ass_strtod.o \
        mplayer/stream/stream.o \
        mplayer/stream/cache2.o \
        mplayer/stream/register_net_stream.o \
        mplayer/stream/http.o \
        mplayer/stream/stream_file.o \
        mplayer/stream/open.o \
        mplayer/stream/network.o \
        mplayer/stream/tcp_mp.o \
        mplayer/stream/cookies.o \
        mplayer/stream/url.o \
        mplayer/stream/rtp.o \
        mplayer/stream/stream_rtp.o \
        mplayer/stream/asf_mmst_streaming.o \
        mplayer/stream/asf_streaming.o \
        mplayer/stream/stream_ffmpeg.o \
        mplayer/stream/stream_fifo.o \
        mplayer/stream/freesdp/common.o \
        mplayer/stream/freesdp/errorlist.o \
        mplayer/stream/freesdp/freesdp_parser.o \
        mplayer/stream/librtsp/rtsp.o \
        mplayer/stream/librtsp/rtsp_rtp.o \
        mplayer/stream/librtsp/rtsp_session.o \
        mplayer/stream/realrtsp/asmrp.o \
        mplayer/stream/realrtsp/real.o \
        mplayer/stream/realrtsp/rmff.o \
        mplayer/stream/realrtsp/sdpplin.o \
        mplayer/stream/realrtsp/xbuffer.o \
        mplayer/sub/subreader.o \
        mplayer/sub/find_sub.o \
        mplayer/sub/subassconvert.o \
        mplayer/sub/eosd.o \
        mplayer/sub/ass_mp.o

ifeq ($(WITH_TCPIP_PROTOCOL),y)
OBJS +=	mplayer/stream/stream_udp.o \
        mplayer/stream/udp.o
endif

ifeq ($(SUPPROT_SRT_PROTOCOL),y)
OBJS += mplayer/stream/stream_srt.o
endif

ifeq ($(EXT_MEM_POOL_ON),y)
OBJS += ext_mem_interface.o
endif

ifeq ($(ENABLE_LIVE555_STREAM),y)
OBJS += mplayer/libmpdemux/demux_rtp.o \
        mplayer/libmpdemux/demux_rtp_codec.o \
        mplayer/stream/stream_live555.o
endif

ifeq ($(CFG_FFMPEG_VER_NUM),422)
OBJS += mplayer/stream/stream_misc.o
endif

LIB := libmonplayer_demux
include ${SDK_DIR}/build/script/Makefile-lib.rule
