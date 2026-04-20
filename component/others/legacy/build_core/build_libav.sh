
#!/bin/bash
echo "start build_libav in script"



function  build_libav()
{
  echo "start to compile libav=================="
      #CUR_HOST=aarch64-linux-gnu
      cd $GST_TOP_DIR
      cd gst-libav-1.8.0
      

      if [ $support_static_plugin -eq 1 ];then
         USE_STATIC_PLUGIN="  --enable-static-plugins "
      else
         USE_STATIC_PLUGIN="   "
      fi

     if [ $rebuild_libav_ffmpeg -eq 1 ];then

        rm -fr gst-libs/ext/libav/libavcodec/*.o
        rm -fr gst-libs/ext/libav/libavcodec/*.d
        rm -fr gst-libs/ext/libav/libavcodec/*.la
        rm -fr gst-libs/ext/libav/libavcodec/*.a

        if [ $host_arm -eq 1 ];then
            echo "host is arm"
        elif [ $host_aarch64 -eq 1 ];then
            echo "host is aarch64"
        elif [ $host_mips -eq 1 ];then
            echo "host is mips"
        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
        fi

        echo "CUR_HOST is $CUR_HOST"
        echo "CUR_CFLAGS is $CUR_CFLAGS"


         export GLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0"
         export GLIB_CFLAGS="-I$GST_OUTPUT_PATH/lib/glib-2.0/include -I$GST_OUTPUT_PATH/include/glib-2.0"
         export GIO_LIBS="-L$GST_OUTPUT_PATH/lib -lgio-2.0"
         export GIO_CFLAGS="-I$GST_OUTPUT_PATH/lib -I$GST_OUTPUT_PATH/include"

         export GST_CFLAGS="-I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  -I$GST_OUTPUT_PATH/include/gstreamer-1.0/  \
                   -I$GST_OUTPUT_PATH/include/glib-2.0   -I$GST_OUTPUT_PATH/lib/glib-2.0/include"
         export GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0  -lglib-2.0 -lgobject-2.0"


            export GST_NET_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_NET_LIBS="-L$GST_OUTPUT_PATH/lib -lgstnet-1.0"

            export GST_BASE_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_BASE_LIBS="-L$GST_OUTPUT_PATH/lib -lgstbase-1.0 -lglib-2.0 -lgobject-2.0  -lgstreamer-1.0"


            export GST_CONTROLLER_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CONTROLLER_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcontroller-1.0"

            export GST_CHECK_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CHECK_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcheck-1.0 -lglib-2.0 -lgobject-2.0 -lgstreamer-1.0 "


            CUR_CFLAGS+="  -I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  -I$GST_OUTPUT_PATH/include/gstreamer-1.0 \
                 -I$GST_OUTPUT_PATH/lib/glib-2.0/include \
                 -I$GST_OUTPUT_PATH/include/glib-2.0  \
                  -I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst \
                  -I$GST_TOP_DIR/gst-libav-1.8.0/Mp4Demux/ \
                  -I$GST_OUTPUT_PATH/include/bento4/inc/C++/Core \
                  -I$GST_OUTPUT_PATH/include/bento4/inc/C++/MetaData     \
                  -I$GST_OUTPUT_PATH/include/bento4/inc/C++/Codec "

            LIBAV_CONFIG_STR=" \
               --extra-cflags=-I$GST_TOP_DIR/gst-libav-1.8.0/Mp4Demux/  \
               --disable-indevs  --disable-outdevs   \
               --disable-devices  --disable-hwaccels \
               --disable-w32threads \
               --disable-lsp --disable-lzo --disable-mdct --disable-rdft \
               --disable-fft  --disable-pixelutils  --disable-power8  --disable-avx2 \
               --disable-iconv --disable-sdl --disable-schannel \
               --disable-protocols  --disable-swscale  --disable-lsp \
               --disable-lzo --disable-d3d11va --disable-dxva2 --disable-xlib \
               --disable-everything \
               --enable-bsf=h264_mp4toannexb,hevc_mp4toannexb,aac_adtstoasc,mp3_header_decompress \
               --enable-demuxer=mov,mpegvideo,mp3,m4v,matroska,mpegtsraw,mgsts,avi,flv,mpegts,mpegps,ogg,vobsub,asf,wav \
	       --enable-decoder=pcm_bluray \
               "

           if [ $is_debug -eq 0 ];then

              LIBAV_CONFIG_STR+=" --disable-debug "


           fi

            EXTRA_CC="$CC"
            EXTRA_CXX="$CXX"

           if [ $host_mips -eq 1 ];then
               LIBAV_CONFIG_STR+=" --disable-mipsfpu --extra-cflags=$EXTRA_CFLAGS --extra-cxxflags=$EXTRA_CXXFLAGS "

           else

               LIBAV_CONFIG_STR+=" --extra-cflags=$EXTRA_CFLAGS --extra-cxxflags=$EXTRA_CXXFLAGS "
           fi

          # LIBAV_CONFIG_STR+=" --cc=mips-linux-gnu-gcc  --cxx=mips-linux-gnu-g++ "

           ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  CFLAGS="$CUR_CFLAGS"  \
               --disable-silent-rules $USE_STATIC_PLUGIN  --enable-static --enable-shared=no -with-libav-extra-configure="$LIBAV_CONFIG_STR"


            touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi
      
      cd Mp4Demux
      echo "cd Mp4Demuxv=================="
      make clean
      make
      check_make_ret
      make install
      cd ..

     make
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR/gst-libav-1.8.0;
     mkdir -p $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     cp -fr ./ext/libav/.libs/libgstlibav.a ./gst-libs/ext/libav/libavutil/libavutil.a  ./gst-libs/ext/libav/libavformat/libavformat.a  \
      ./gst-libs/ext/libav/libavcodec/libavcodec.a  ./gst-libs/ext/libav/libavfilter/libavfilter.a \
          $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     echo "finish compile libav=================="

    cd $GST_TOP_DIR/

}

