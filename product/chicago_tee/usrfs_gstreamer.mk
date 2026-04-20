include ${SDK_DIR}/build/script/base.mk

ifeq ($(CONFIG_MT_STRIP_DEBUGINFO),y)
export STRIP_DEBUG=--strip-debug
endif

ifeq (${SDK_DIR}/image/$(MFRS)/board_config.mk,$(wildcard ${SDK_DIR}/image/$(MFRS)/board_config.mk))
	include ${SDK_DIR}/image/$(MFRS)/board_config.mk
endif

define make_gstreamer_usrfs
# component/media/framework/gstreamer, component/media/gstreamer, kware/media
# gstreamer support lib
	@mkdir -p $(USR_FS_DIR)/lib/gstreamer
	@mkdir -p $(USR_FS_DIR)/lib/gio/modules
	@mkdir -p $(USR_FS_DIR)/libexec/gstreamer-1.0
	@mkdir -p $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(BUILDROOT_TARGET_DIR)/usr/libexec/gstreamer-1.0/gst-plugin-scanner      -af $(USR_FS_DIR)/libexec/gstreamer-1.0/
	ALL_FILES=`find $(USR_FS_DIR)/libexec/ -type f`; for x in $${ALL_FILES}; do if [ -n "`file $${x} | grep ELF`" ]; then chmod +w $${x}; $(STRIP) $(STRIP_DEBUG) $${x}; chmod -w $${x}; fi; done
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstbase-1.0.*                                -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstpbutils-1.0.*                             -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstaudio-1.0.*                               -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstvideo-1.0.*                               -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgsttag-1.0.*                                 -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstriff-1.0.*                                -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstcodecparsers-1.0.*                        -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstrtp-1.0.so*                               -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstmpegts-1.0.so*                            -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstsdp-1.0.so*                               -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstrtsp-1.0.so*                              -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstisoff-1.0.so*                             -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgsturidownloader-1.0.so*                     -af $(USR_FS_DIR)/lib/gstreamer
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstadaptivedemux-1.0.so*                     -af $(USR_FS_DIR)/lib/gstreamer

	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstcoreelements.so             -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstmatroska.so                 -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgsttypefindfunctions.so        -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstvideoparsersbad.so          -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstaudioparsers.so             -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstsoup.so                     -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstogg.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstavi.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstasf.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstflv.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstisomp4.so                   -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstid3demux.so                 -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstwavparse.so                 -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstmpegpsdemux.so              -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstmpegtsdemux.so              -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgsthls.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstdash.so                     -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
# RTSP
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstrtp.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgsttcp.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstudp.so                      -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstrtsp.so                     -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstrealmedia.so                -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstrtpmanager.so               -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
#RTP
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstrtpmanagerbad.so            -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/
#RTMP
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gstreamer-1.0/libgstrtmp*                       -af $(USR_FS_DIR)/lib/gstreamer/gstreamer-1.0/

	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libglib-2.0*                                    -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgobject-2.0*                                 -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgmodule-2.0*                                 -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstreamer-1.0.*                              -af $(USR_FS_DIR)/lib

	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/mt/libgstmlzplayer.so                           -af $(USR_FS_DIR)/lib
#libpcre need by libglib-2.0.so for Regular Expressions
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpcre.so*                                     -af $(USR_FS_DIR)/lib
#libnettle need by libgsthls.so for low-level cryptographic library
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libnettle.so*                                   -af $(USR_FS_DIR)/lib
#libidn2 is a package designed for internationalized string handling based on standards from the Internet Engineering Task Force (IETF)'s IDN working group, designed for internationalized domain name
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libidn2.so*                                     -af $(USR_FS_DIR)/lib
#libunistring is a library that provides functions for manipulating Unicode strings and for manipulating C strings according to the Unicode standard
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libunistring.so*                                -af $(USR_FS_DIR)/lib
	
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libz.so                                         -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/liborc-0.4.*                                    -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgio-2.0.*                                    -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libfreetype.so*                                 -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libsoup-2.4.*                                   -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libxml2.*                                       -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libsqlite3.*                                    -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpsl.*                                        -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libffi*                                         -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpng.so*                                      -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libpng16.so*                                    -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcurl.so*                                     -af $(USR_FS_DIR)/lib
#A library that performs asynchronous DNS operations
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcares.so*                                    -af $(USR_FS_DIR)/lib

	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstadaptivedemux-1.0.so*                     -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstisoff-1.0.so*                             -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgstnet-1.0.so*                               -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libgsturidownloader-1.0.so*                     -af $(USR_FS_DIR)/lib

	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libogg.so*                                      -af $(USR_FS_DIR)/lib
#	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libssl.so*                                      -af $(USR_FS_DIR)/lib
#	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/libcrypto.so*                                   -af $(USR_FS_DIR)/lib
	-cp --remove-destination $(SHARED_LIB_DIR_STRIPED)/gio/modules/libgioopenssl.so                    -af $(USR_FS_DIR)/lib/gio/modules

endef
