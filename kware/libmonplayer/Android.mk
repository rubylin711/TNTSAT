
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
#include ${SDK_DIR}/Android.def

LOCAL_MODULE_TAGS := optional
#LOCAL_MODULE := libhiplayer_adapter
ALL_DEFAULT_INSTALLED_MODULES += $(LOCAL_MODULE)
BUILD_BD := true
LOCAL_CFLAGS := $(CFG_MT_CFLAGS)
LOCAL_CFLAGS += -D__LINUX__
LOCAL_CFLAGS += -D__ANDRIOD__
LOCAL_CFLAGS += -DHWDISABLE
LOCAL_CFLAGS += -DENABLE_DEMUX_HTTP
LOCAL_CFLAGS += -DWITH_TCPIP_PROTOCOL
LOCAL_SRC_FILES := \
        moplayer/moplayer.c \
		moplayer/moplayer_internal.c \
		moplayer/moplayer_hw.c \
		file_playback/file_playback_sequence.c \
		file_playback/mplayer_type.c \
		file_playback/file_seq_misc.c                       \
		file_playback/file_seq_internal.c                       \
		file_playback/file_out_subtitle.c                       \                   \
		demux_mp/demux_mp.c \
		demux_mp/demux_mp_misc.c \
		demux_mp/mplayer/bstr.c \
		demux_mp/mplayer/path.c \
		demux_mp/mplayer/av_helpers.c \
		demux_mp/mplayer/mpcommon.c \
		demux_mp/mplayer/mp_msg.c \
		demux_mp/mplayer/mp_strings.c \
		demux_mp/mplayer/mp_func_trans.c \
		demux_mp/mplayer/codec-cfg.c \
		demux_mp/mplayer/m_struct.c \
		demux_mp/mplayer/m_option.c \
		demux_mp/mplayer/av_opts.c \
		demux_mp/mplayer/libmpdemux/mp3_hdr.c \
		demux_mp/mplayer/libmpdemux/demuxer.c \
		demux_mp/mplayer/libmpdemux/parse_es.c \
		demux_mp/mplayer/libmpdemux/demux_lavf.c \
		demux_mp/mplayer/libmpdemux/demux_real.c \
		demux_mp/mplayer/libmpdemux/demux_demuxers.c \
		demux_mp/mplayer/libmpdemux/demux_ts.c \
		demux_mp/mplayer/libmpdemux/demux_audio.c \
		demux_mp/mplayer/libmpdemux/demux_avi.c \
		demux_mp/mplayer/libmpdemux/mp_taglists.c \
		demux_mp/mplayer/libmpdemux/aviprint.c \
		demux_mp/mplayer/libmpdemux/aviheader.c \
		demux_mp/mplayer/libmpdemux/extension.c \
		demux_mp/mplayer/libmpdemux/mpeg_hdr.c \
		demux_mp/mplayer/libmpdemux/demux_mpg.c \
		demux_mp/mplayer/libmpdemux/asfheader.c \
		demux_mp/mplayer/libmpdemux/demux_asf.c \
		demux_mp/mplayer/ffmpeg/libavformat/options.c \
		demux_mp/mplayer/ffmpeg/libavformat/id3v1.c \
		demux_mp/mplayer/ffmpeg/libavformat/rmsipr.c \
		demux_mp/mplayer/ffmpeg/libavformat/matroskadec.c \
		demux_mp/mplayer/ffmpeg/libavformat/flvenc.c \
		demux_mp/mplayer/ffmpeg/libavformat/matroska.c \
		demux_mp/mplayer/ffmpeg/libavformat/id3v2.c \
		demux_mp/mplayer/ffmpeg/libavformat/metadata.c \
		demux_mp/mplayer/ffmpeg/libavformat/riff.c \
		demux_mp/mplayer/ffmpeg/libavformat/dv.c \
		demux_mp/mplayer/ffmpeg/libavformat/formatutils.c \
		demux_mp/mplayer/ffmpeg/libavformat/avidec.c \
		demux_mp/mplayer/ffmpeg/libavformat/mpegts.c \
		demux_mp/mplayer/ffmpeg/libavformat/flvdec.c \
		demux_mp/mplayer/ffmpeg/libavformat/isom.c \
		demux_mp/mplayer/ffmpeg/libavformat/rtmppkt.c \
		demux_mp/mplayer/ffmpeg/libavformat/rtmpproto.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_network.c \
		demux_mp/mplayer/ffmpeg/libavformat/os_support.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_tcp.c \
		demux_mp/mplayer/ffmpeg/libavformat/mov.c \
		demux_mp/mplayer/ffmpeg/libavformat/aviobuf.c \
		demux_mp/mplayer/ffmpeg/libavformat/avlanguage.c \
		demux_mp/mplayer/ffmpeg/libavformat/asfcrypt.c \
		demux_mp/mplayer/ffmpeg/libavformat/asf.c \
		demux_mp/mplayer/ffmpeg/libavformat/asfdec.c \
		demux_mp/mplayer/ffmpeg/libavformat/allformats.c \
		demux_mp/mplayer/ffmpeg/libavformat/mov_chan.c \
		demux_mp/mplayer/ffmpeg/libavformat/avio.c \
		demux_mp/mplayer/ffmpeg/libavformat/urldecode.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_hls.c \
		demux_mp/mplayer/ffmpeg/expat/lib/xmlparse.c \
		demux_mp/mplayer/ffmpeg/expat/lib/xmltok.c \
		demux_mp/mplayer/ffmpeg/expat/lib/xmlrole.c \
		demux_mp/mplayer/ffmpeg/libavformat/f4mmanifest.c \
		demux_mp/mplayer/ffmpeg/libavformat/amfmetadata.c \
		demux_mp/mplayer/ffmpeg/libavformat/f4fbox.c \
		demux_mp/mplayer/ffmpeg/libavformat/flvtag.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_hds.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_crypto.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_http.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_httpauth.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_ac3dec.c \
		demux_mp/mplayer/ffmpeg/libavformat/ff_rawdec.c \
		demux_mp/mplayer/ffmpeg/libavformat/mp3dec.c \
		demux_mp/mplayer/ffmpeg/libavformat/wav.c \
		demux_mp/mplayer/ffmpeg/libavformat/pcm.c \
		demux_mp/mplayer/ffmpeg/libavutil/dict.c \
		demux_mp/mplayer/ffmpeg/libavutil/ff_aes.c \
		demux_mp/mplayer/ffmpeg/libavutil/imgutils.c \
		demux_mp/mplayer/ffmpeg/libavutil/timecode.c \
		demux_mp/mplayer/ffmpeg/libavutil/rational.c \
		demux_mp/mplayer/ffmpeg/libavutil/pixdesc.c \
		demux_mp/mplayer/ffmpeg/libavutil/audioconvert.c \
		demux_mp/mplayer/ffmpeg/libavutil/utilutils.c \
		demux_mp/mplayer/ffmpeg/libavutil/parseutils.c \
		demux_mp/mplayer/ffmpeg/libavutil/bprint.c \
		demux_mp/mplayer/ffmpeg/libavutil/opt.c \
		demux_mp/mplayer/ffmpeg/libavutil/mathematics.c \
		demux_mp/mplayer/ffmpeg/libavutil/utiltime.c \
		demux_mp/mplayer/ffmpeg/libavutil/samplefmt.c \
		demux_mp/mplayer/ffmpeg/libavutil/utilmem.c \
		demux_mp/mplayer/ffmpeg/libavutil/base644.c \
		demux_mp/mplayer/ffmpeg/libavutil/crc.c \
		demux_mp/mplayer/ffmpeg/libavutil/avstring.c \
		demux_mp/mplayer/ffmpeg/libavutil/util_log.c \
		demux_mp/mplayer/ffmpeg/libavutil/eval.c \
		demux_mp/mplayer/ffmpeg/libavutil/rc4.c \
		demux_mp/mplayer/ffmpeg/libavutil/des.c \
		demux_mp/mplayer/ffmpeg/libavutil/sha.c \
		demux_mp/mplayer/ffmpeg/libavutil/lfg.c \
		demux_mp/mplayer/ffmpeg/libavutil/ff_md5.c \
		demux_mp/mplayer/ffmpeg/libavutil/random_seed.c \
		demux_mp/mplayer/ffmpeg/libavcodec/rawdec.c \
		demux_mp/mplayer/ffmpeg/libavcodec/imgconvert.c \
		demux_mp/mplayer/ffmpeg/libavcodec/codec_desc.c \
		demux_mp/mplayer/ffmpeg/libavcodec/allcodecs.c \
		demux_mp/mplayer/ffmpeg/libavcodec/h264.c \
		demux_mp/mplayer/ffmpeg/libavcodec/mpegvideo.c \
		demux_mp/mplayer/ffmpeg/libavcodec/h264_parser.c \
		demux_mp/mplayer/ffmpeg/libavcodec/golomb.c \
		demux_mp/mplayer/ffmpeg/libavcodec/avpacket.c \
		demux_mp/mplayer/ffmpeg/libavcodec/parser.c \
		demux_mp/mplayer/ffmpeg/libavcodec/bitstream_filter.c \
		demux_mp/mplayer/ffmpeg/libavcodec/codecutils.c \
		demux_mp/mplayer/ffmpeg/libavcodec/h264_mp4toannexb_bsf.c \
		demux_mp/mplayer/ffmpeg/libavcodec/mpeg4audio.c \
		demux_mp/mplayer/ffmpeg/libavcodec/codecraw.c \
		demux_mp/mplayer/ffmpeg/libavcodec/codecoptions.c \
		demux_mp/mplayer/ffmpeg/libavcodec/dv_profile.c \
		demux_mp/mplayer/ffmpeg/libavcodec/ac3_parser.c \
		demux_mp/mplayer/ffmpeg/libavcodec/ac3tab.c \
		demux_mp/mplayer/ffmpeg/libavcodec/mpegaudiodecheader.c \
		demux_mp/mplayer/ffmpeg/libavcodec/mpegaudiodata.c \
		demux_mp/mplayer/stream/stream.c \
		demux_mp/mplayer/stream/cache2.c \
		demux_mp/mplayer/stream/register_net_stream.c \
		demux_mp/mplayer/stream/http.c \
		demux_mp/mplayer/stream/stream_file.c \
		demux_mp/mplayer/stream/open.c \
		demux_mp/mplayer/stream/network.c \
		demux_mp/mplayer/stream/tcp_mp.c \
		demux_mp/mplayer/stream/cookies.c \
		demux_mp/mplayer/stream/url.c \
		demux_mp/mplayer/stream/rtp.c \
		demux_mp/mplayer/stream/stream_rtp.c \
		demux_mp/mplayer/stream/stream_rtsp.c \
		demux_mp/mplayer/stream/asf_mmst_streaming.c \
		demux_mp/mplayer/stream/asf_streaming.c \
		demux_mp/mplayer/stream/stream_ffmpeg.c \
		demux_mp/mplayer/stream/stream_live555.c \
		demux_mp/mplayer/stream/stream_fifo.c \
		demux_mp/mplayer/stream/freesdp/common.c \
		demux_mp/mplayer/stream/freesdp/errorlist.c \
		demux_mp/mplayer/stream/freesdp/freesdp_parser.c \
		demux_mp/mplayer/stream/librtsp/rtsp.c \
		demux_mp/mplayer/stream/librtsp/rtsp_rtp.c \
		demux_mp/mplayer/stream/librtsp/rtsp_session.c \
		demux_mp/mplayer/stream/realrtsp/asmrp.c \
		demux_mp/mplayer/stream/realrtsp/real.c \
		demux_mp/mplayer/stream/realrtsp/rmff.c \
		demux_mp/mplayer/stream/realrtsp/sdpplin.c \
		demux_mp/mplayer/stream/realrtsp/xbuffer.c \


LOCAL_C_INCLUDES := $(LOCAL_PATH)/include\
	$(LOCAL_PATH)/include/file_playback\
	$(LOCAL_PATH)/include/file_playback\
	$(LOCAL_PATH)/moplayer/include \
	$(LOCAL_PATH)/include/demux_mp\
	$(LOCAL_PATH)/demux_mp/mplayer \
    $(LOCAL_PATH)/demux_mp/mplayer \
	$(LOCAL_PATH)/demux_mp/mplayer/ffmpeg \
	$(LOCAL_PATH)/demux_mp/mplayer/libmpdemux \
	$(LOCAL_PATH)/demux_mp/mplayer/stream \
	$(LOCAL_PATH)/../../../../../external/skia/include/core \
	$(LOCAL_PATH)/../../hardware/gpu/android/gralloc \
	$(LOCAL_PATH)/../../sdk/source/msp/include \
	$(LOCAL_PATH)/../../sdk/source/common/include \
	$(LOCAL_PATH)/../../sdk/pub/include \
	$(LOCAL_PATH)/../../../prebuilts/37xx/inc/msp/include \
	$(LOCAL_PATH)/../../../prebuilts/37xx/inc/common/include \
	$(LOCAL_PATH)/../../../../../../device/korimako/bigfish/sdk/pub/include \




LOCAL_SHARED_LIBRARIES := liblog libcutils  libhi_common libhi_msp 
LOCAL_MODULE := libmonPlayer
LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)
