
#!/bin/bash
echo "start build_gst base good bad  in script"


##########################################
#
#
#############################################

function  build_gst()
{

      echo "start to compile gst==================="
      cd $GST_TOP_DIR
      cd gstreamer-1.8.0

      if [ $support_static_plugin -eq 1 ];then
         USE_STATIC_PLUGIN="  --enable-static-plugins "
      else
         USE_STATIC_PLUGIN="   "
      fi

      if [ $rebuild_libgst -eq 1 ];then

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

       if [ $is_debug -eq 1 ];then
         ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  $USE_STATIC_PLUGIN --enable-static  --disable-silent-rules  \
           CFLAGS="$CUR_CFLAGS"  GLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0"  \
            GLIB_CFLAGS="-I$GST_OUTPUT_PATH/lib/glib-2.0/include -I$GST_OUTPUT_PATH/include/glib-2.0"  \
              GIO_LIBS="-L$GST_OUTPUT_PATH/lib -lgio-2.0"  \
                GIO_CFLAGS="-I$GST_OUTPUT_PATH/lib -I$GST_OUTPUT_PATH/include" \
                  LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"
       else
         ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --disable-debug  $USE_STATIC_PLUGIN \
           --enable-static  --disable-silent-rules  \
           CFLAGS="$CUR_CFLAGS"  GLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0"  \
            GLIB_CFLAGS="-I$GST_OUTPUT_PATH/lib/glib-2.0/include -I$GST_OUTPUT_PATH/include/glib-2.0"  \
              GIO_LIBS="-L$GST_OUTPUT_PATH/lib -lgio-2.0"  \
                GIO_CFLAGS="-I$GST_OUTPUT_PATH/lib -I$GST_OUTPUT_PATH/include" \
                  LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"
       fi


       touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi

     make

     if [ $host_x86 -eq 1 ];then

        if [ $? -eq 0 ]; then
          echo $1
          echo "ok compile auccess !"
        else
          echo $1
          echo "fail to compile  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
          exit 1
        fi

     fi

     make install
     check_make_ret


     cd $GST_TOP_DIR
     mkdir -p $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libgstreamer-1.0.a  $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libgstbase-1.0.a         $STATIC_OUTPUT_PATH/lib
   #   cp -fr $OUTPUT_PATH/lib/libgstcontroller-1.0.a   $STATIC_OUTPUT_PATH/lib
    #  cp -fr $OUTPUT_PATH/lib/libgstcheck-1.0.a      $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libgstnet-1.0.a      $STATIC_OUTPUT_PATH/lib
      mkdir -p $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstcoreelements.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstcoretracers.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/include   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     echo "finish compile gst=================="
}


##########################################
#
#
#############################################


function  build_plugin_base()
{
    echo "start to compile baseplugin==================="
      cd $GST_TOP_DIR
      cd gst-plugins-base-1.8.0

      if [ $support_static_plugin -eq 1 ];then
         USE_STATIC_PLUGIN="  --enable-static-plugins "
      else
         USE_STATIC_PLUGIN="   "
      fi

     if [ $rebuild_libpluginbase -eq 1 ];then

        if [ $host_arm -eq 1 ];then
            echo "host is arm"
        elif [ $host_aarch64 -eq 1 ];then
            echo "host is aarch64"
        elif [ $host_mips -eq 1 ];then
            echo "host is mips"
        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
        fi


             export GLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0"
             export GLIB_CFLAGS="-I$GST_OUTPUT_PATH/lib/glib-2.0/include -I$GST_OUTPUT_PATH/include/glib-2.0"
             export GIO_LIBS="-L$GST_OUTPUT_PATH/lib -lgio-2.0"
             export GIO_CFLAGS="-I$GST_OUTPUT_PATH/lib -I$GST_OUTPUT_PATH/include"

            export GST_CFLAGS="-I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  -I$GST_OUTPUT_PATH/include/gstreamer-1.0/"
            export GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0"


            export GST_NET_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_NET_LIBS="-L$GST_OUTPUT_PATH/lib -lgstnet-1.0"

            export GST_BASE_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_BASE_LIBS="-L$GST_OUTPUT_PATH/lib -lgstbase-1.0 -lglib-2.0 -lgobject-2.0"


            export GST_CONTROLLER_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CONTROLLER_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcontroller-1.0"

            export GST_CHECK_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CHECK_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcheck-1.0"

         MIPS_GST_LIBS=" "
         if [ $host_mips -eq 1 ];then
           echo "add lib for mips"
           MIPS_GST_LIBS+=" -L$GST_TOP_DIR/../pub/static_lib/ "
           CUR_CFLAGS+="  -I$GST_TOP_DIR/../kware/libmonplayer/avplay_instance/ -I$GST_TOP_DIR/../common/inc/ "
         fi
         if [ $host_arm -eq 1 ];then
           echo "add lib for arm"
           MIPS_GST_LIBS+=" -L$GST_TOP_DIR/../pub/static_lib/ "
           CUR_CFLAGS+="  -I$GST_TOP_DIR/../kware/libmonplayer/avplay_instance/ -I$GST_TOP_DIR/../common/inc/ "
         fi
         if [ $host_aarch64 -eq 1 ];then
           echo "add lib for aarch64"
           MIPS_GST_LIBS+=" -L$GST_TOP_DIR/../pub/static_lib/ "
           CUR_CFLAGS+="  -I$GST_TOP_DIR/../kware/libmonplayer/avplay_instance/ -I$GST_TOP_DIR/../common/inc/ "
         fi

         if [ $is_debug -eq 1 ];then

           ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --disable-xvideo --disable-x \
                   --disable-examples --disable-nls --disable-silent-rules --disable-alsa \
                   --enable-gtk-doc-html=no --disable-adder --disable-app --disable-audiotestsrc \
                   --disable-encoding --disable-videoconvert --disable-audioresample \
                   --disable-tcp --disable-videotestsrc --disable-videoscale \
                   --disable-videorate --disable-volume   --disable-xshm  \
                   --disable-cdparanoia --disable-libvisual --disable-opus  --disable-pango  --enable-static \
                   --disable-theora $USE_STATIC_PLUGIN \
               CFLAGS="$CUR_CFLAGS"  GST_CFLAGS="-I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  \
               -I$GST_OUTPUT_PATH/include/gstreamer-1.0/"  \
                     GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0 -lgio-2.0 -ldl $MIPS_GST_LIBS" \
              LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"
        else
 
           ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --disable-xvideo --disable-x \
                   --disable-examples --disable-nls --disable-silent-rules --disable-alsa \
                   --enable-gtk-doc-html=no --disable-adder --disable-app --disable-audiotestsrc \
                   --disable-encoding --disable-videoconvert --disable-audioresample \
                   --disable-tcp --disable-videotestsrc --disable-videoscale \
                   --disable-videorate --disable-volume   --disable-xshm  \
                   --disable-cdparanoia --disable-libvisual --disable-opus  --disable-pango  --enable-static \
                   --disable-theora $USE_STATIC_PLUGIN  --disable-debug \
                  CFLAGS="$CUR_CFLAGS"  GST_CFLAGS="-I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include \
                   -I$GST_OUTPUT_PATH/include/gstreamer-1.0/"  \
                  GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0 -lglib-2.0 -lgobject-2.0 -lgio-2.0 -ldl $MIPS_GST_LIBS" \
              LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"

        fi



        touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi

     make
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR
    #  cp -fr $OUTPUT_PATH/lib/libgstbase-1.0.a         $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libgstallocators-1.0.a   $STATIC_OUTPUT_PATH/lib
     # cp -fr $OUTPUT_PATH/lib/libgstapp-1.0.a      $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libgstpbutils-1.0.a      $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libgstriff-1.0.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/libgstrtp-1.0.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstplayback.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstsdp-1.0.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/libgsttag-1.0.a     $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstvideo-1.0.a   $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstaudio-1.0.a   $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgsttypefindfunctions.a $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
 

     echo "finish compile baseplugin=================="



}

##########################################
#
#
#############################################


function  build_plugin_good()
{
      echo "start to compile good plugin ==================="
      cd $GST_TOP_DIR
      cd gst-plugins-good-1.8.0
 

      if [ $support_static_plugin -eq 1 ];then
         USE_STATIC_PLUGIN="  --enable-static-plugins "
      else
         USE_STATIC_PLUGIN="   "
      fi

     if [ $rebuild_libplugingood -eq 1 ];then

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

            export GST_CFLAGS="-I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  -I$GST_OUTPUT_PATH/include/gstreamer-1.0/"
            export GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0  -lglib-2.0 -lgobject-2.0"


            export GST_NET_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_NET_LIBS="-L$GST_OUTPUT_PATH/lib -lgstnet-1.0"

            export GST_BASE_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_BASE_LIBS="-L$GST_OUTPUT_PATH/lib -lgstbase-1.0 -lglib-2.0 -lgobject-2.0"


            export GST_CONTROLLER_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CONTROLLER_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcontroller-1.0"

            export GST_CHECK_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CHECK_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcheck-1.0"


            CUR_CFLAGS+="  -I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  \
                  -I$GST_OUTPUT_PATH/include/gstreamer-1.0  \
                  -I$GST_OUTPUT_PATH/lib/glib-2.0/include \
                  -I$GST_OUTPUT_PATH/include/glib-2.0  -I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"



         if [ $is_debug -eq 1 ];then

          ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --enable-static \
             --disable-silent-rules \
             --disable-nls \
             --disable-pulse  \
             --disable-valgrind \
             --disable-examples \
             --disable-vpx \
             --enable-gtk-doc-html=no \
             --disable-autodetect \
             --disable-dtmf \
             --disable-rtp \
             --disable-rtpmanager \
             --disable-rtsp  \
             --disable-shapewipe \
             --disable-udp \
             --disable-dv1394 \
             --disable-libpng  \
             --disable-x  \
             --disable-speex \
             --disable-shout2   \
             --disable-libdv \
             --disable-libcaca  \
             --disable-jpeg  --disable-jack \
             --disable-gdk_pixbuf --disable-cairo --disable-aalibtest --disable-aalib \
             --enable-v4l2-probe  --disable-gst_v4l2 --disable-osx_video --disable-osx_audio \
             --disable-sunaudio  --disable-oss4  --disable-oss --disable-waveform \
             --disable-directsound  --disable-y4m  --disable-wavenc \
             --disable-videomixer --disable-videocrop --disable-videobox --disable-spectrum \
             --disable-replaygain  --disable-multipart --disable-multifile --disable-monoscope \
             --disable-level --disable-interleave  --disable-imagefreeze  --disable-icydemux \
              --disable-goom2k1  --disable-goom --disable-equalizer --disable-effectv \
             --disable-deinterlace $USE_STATIC_PLUGIN  --disable-cutter  --disable-alpha --disable-soup \
             CFLAGS="$CUR_CFLAGS" SOUP_CFLAGS="-I$GST_OUTPUT_PATH/include/libsoup-2.4"  \
             SOUP_LIBS="-L$GST_OUTPUT_PATH/lib -lsoup-2.4" \
             GLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0"  \
             GLIB_CFLAGS="-I$GST_OUTPUT_PATH/lib/glib-2.0/include  -I$GST_OUTPUT_PATH/include/glib-2.0" \
             LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"

         else

          ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --enable-static --disable-debug \
             --disable-silent-rules \
             --disable-nls \
             --disable-pulse  \
             --disable-valgrind \
             --disable-examples \
             --disable-vpx \
             --enable-gtk-doc-html=no \
             --disable-autodetect \
             --disable-dtmf \
             --disable-rtp \
             --disable-rtpmanager \
             --disable-rtsp  \
             --disable-shapewipe \
             --disable-udp \
             --disable-dv1394 \
             --disable-libpng  \
             --disable-x  \
             --disable-speex \
             --disable-shout2   \
             --disable-libdv \
             --disable-libcaca  \
             --disable-jpeg  --disable-jack \
             --disable-gdk_pixbuf --disable-cairo --disable-aalibtest --disable-aalib \
             --enable-v4l2-probe  --disable-gst_v4l2 --disable-osx_video --disable-osx_audio \
             --disable-sunaudio  --disable-oss4  --disable-oss --disable-waveform \
             --disable-directsound  --disable-y4m  --disable-wavenc \
             --disable-videomixer --disable-videocrop --disable-videobox --disable-spectrum \
             --disable-replaygain  --disable-multipart --disable-multifile --disable-monoscope \
             --disable-level --disable-interleave  --disable-imagefreeze  --disable-icydemux \
              --disable-goom2k1  --disable-goom --disable-equalizer --disable-effectv \
             --disable-deinterlace $USE_STATIC_PLUGIN  --disable-cutter  --disable-alpha --disable-soup \
             CFLAGS="$CUR_CFLAGS" SOUP_CFLAGS="-I$GST_OUTPUT_PATH/include/libsoup-2.4"  \
             SOUP_LIBS="-L$GST_OUTPUT_PATH/lib -lsoup-2.4" \
             GLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0"  \
             GLIB_CFLAGS="-I$GST_OUTPUT_PATH/lib/glib-2.0/include  -I$GST_OUTPUT_PATH/include/glib-2.0" \
             LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"



         fi



        touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi

     make
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR
     # cp -fr $OUTPUT_PATH/lib/libgstwavparse.a      $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstsmpte.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmatroska.a      $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/libgstmulaw.a      $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstalaw.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstisomp4.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/libgstalaw.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstid3demux.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstflv.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/libgstflxdec.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/libgstdebug.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/libgstnavigationtest.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstavi.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstauparse.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstaudioparsers.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstaudiofx.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstapetag.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgsttypefindfunctions.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstsubparse.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstplayback.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstgio.a     $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstaudiorate.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstaudioconvert.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/libgstaudiorate.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0

 


     echo "finish compile goodplugin=================="



}


##########################################
#
#
#############################################

function  create_demux_dash_static_lib()
{


      cd $GST_TOP_DIR/gst-plugins-bad-1.8.0/ext/dash/.libs/

      $AR cru libgstdashdemux.a libgstdashdemux_la-gstdashdemux.o \
          libgstdashdemux_la-gstmpdparser.o libgstdashdemux_la-gstplugin.o libgstdashdemux_la-gstisoff.o 
      $RANLIB libgstdashdemux.a

      echo "create demux dash lib ok"
      cd $GST_TOP_DIR/
      echo `pwd`
      cp -fr gst-plugins-bad-1.8.0/ext/dash/.libs/libgstdashdemux.a  $OUTPUT_PATH/lib/gstreamer-1.0


}

function  build_plugin_bad()
{
     echo "start to compile plugins bad=================="

      cd $GST_TOP_DIR
      cd gst-plugins-bad-1.8.0


      if [ $support_static_plugin -eq 1 ];then
         USE_STATIC_PLUGIN="  --enable-static-plugins "
      else
         USE_STATIC_PLUGIN="   "
      fi

     if [ $rebuild_libpluginbad -eq 1 ];then

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
            export GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0  -lglib-2.0 -lgobject-2.0 -lgstvideo-1.0"


            export GST_NET_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_NET_LIBS="-L$GST_OUTPUT_PATH/lib -lgstnet-1.0"

            export GST_BASE_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_BASE_LIBS="-L$GST_OUTPUT_PATH/lib -lgstbase-1.0 -lglib-2.0 -lgobject-2.0  -lgstreamer-1.0"


            export GST_CONTROLLER_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CONTROLLER_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcontroller-1.0"

            export GST_CHECK_CFLAGS="-I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"
            export GST_CHECK_LIBS="-L$GST_OUTPUT_PATH/lib -lgstcheck-1.0 -lglib-2.0 -lgobject-2.0 -lgstreamer-1.0 "

            export  OPENSSL_CFLAGS="-I$GST_OUTPUT_PATH/include"
            export  OPENSSL_LIBS="-L$GST_OUTPUT_PATH/lib -lssl -lcrypto"

            export LIBXML2_CFLAGS="-I$GST_OUTPUT_PATH/include  -I$GST_OUTPUT_PATH/include/libxml2"
            export LIBXML2_LIBS="-L$GST_OUTPUT_PATH/lib -lxml2"



            export NEON_CFLAGS="-I$GST_OUTPUT_PATH/include  -I$GST_OUTPUT_PATH/include/neon"
            export NEON_LIBS="-I$GST_OUTPUT_PATH/lib -lneon"

            CUR_CFLAGS+="  -I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  \
               -I$GST_OUTPUT_PATH/include  -I$GST_OUTPUT_PATH/include/gstreamer-1.0  \
               -I$GST_OUTPUT_PATH/lib/glib-2.0/include -I$GST_OUTPUT_PATH/include/glib-2.0  \
                -I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst"



        if [ $is_debug -eq 1 ];then

       ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  --enable-x11=no --enable-static $USE_STATIC_PLUGIN --disable-silent-rules \
                    --disable-wayland --enable-x11=no --disable-librfb \
                    --disable-opencv --disable-openexr --disable-openni2 \
                    --disable-openjpeg --disable-rsvg --disable-gtk3 \
                    --disable-qt --disable-vulkan  --disable-libvisual \
                    --disable-timidity --disable-wildmidi --disable-sdl --disable-sdltest \
                    --disable-sndfile --disable-spc --disable-gme --disable-wininet \
                    --disable-acm  --disable-vdpau --disable-sbc --disable-schro \
                    --disable-zbar --disable-spandsp --disable-sndio  --disable-adpcmenc \
                    --disable-aiff --disable-asfmux --disable-audiomixer --disable-audiofxbad \
                    --disable-compositor --disable-audiovisualizers --disable-bayer  \
                    --disable-camerabin2 --disable-cdxaparse  --disable-coloreffects  \
                    --disable-dataurisrc --disable-dccp --disable-faceoverlay \
                    --disable-festival  --disable-fieldanalysis  --disable-freeverb \
                    --disable-frei0r --disable-gaudieffects --disable-geometrictransform \
                    --disable-gdp --disable-ivtc --disable-ivfparse  --disable-jp2kdecimator \
                    --disable-jpegformat --disable-librfb --disable-mpegtsmux  --disable-mpegpsmux  \
                    --disable-mve --disable-mxf --disable-netsim --disable-onvif --disable-pnm \
                    --disable-removesilence --disable-sdi --disable-sdp --disable-segmentclip \
                    --disable-siren --disable-subenc --disable-stereo  --disable-tta  \
                    --disable-videomeasure  --disable-videosignal --disable-vmnc \
                    --disable-y4m --disable-yadif  --enable-opengl=no \
                    --enable-wgl=no --enable-glx=no  --disable-directsound --disable-wasapi \
                    --disable-direct3d --disable-winscreencap  --disable-winks \
                    --disable-apple_media --disable-bluez --disable-vcd  --disable-opensles \
                    --disable-uvch264 --disable-nvenc   --disable-voamrwbenc --disable-voaacenc \
                    --disable-apexsink --disable-chromaprint --disable-curl --disable-decklink  \
                    --disable-directfb --disable-wayland    --disable-webp  --disable-daala \
                     --disable-resindvd   --disable-flite --disable-faad --disable-gsm --disable-fluidsynth \
                    --disable-kate --disable-ladspa  --disable-lv2  --disable-libde265  --disable-linsy  \
                    --disable-modplug --disable-mimic --disable-mpeg2enc  --disable-mplex   --disable-musepack  \
                    --disable-nas  --disable-videoparsers --disable-videofilters  --disable-ofa --with-hls-crypto=openssl  --disable-rtmp CFLAGS="$CUR_CFLAGS" \
                    LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"

        else


       ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  --enable-x11=no  --disable-debug \
                    --enable-static $USE_STATIC_PLUGIN --disable-silent-rules \
                    --disable-wayland --enable-x11=no --disable-librfb \
                    --disable-opencv --disable-openexr --disable-openni2 \
                    --disable-openjpeg --disable-rsvg --disable-gtk3 \
                    --disable-qt --disable-vulkan  --disable-libvisual \
                    --disable-timidity --disable-wildmidi --disable-sdl --disable-sdltest \
                    --disable-sndfile --disable-spc --disable-gme --disable-wininet \
                    --disable-acm  --disable-vdpau --disable-sbc --disable-schro \
                    --disable-zbar --disable-spandsp --disable-sndio  --disable-adpcmenc \
                    --disable-aiff --disable-asfmux --disable-audiomixer --disable-audiofxbad \
                    --disable-compositor --disable-audiovisualizers --disable-bayer  \
                    --disable-camerabin2 --disable-cdxaparse  --disable-coloreffects  \
                    --disable-dataurisrc --disable-dccp --disable-faceoverlay \
                    --disable-festival  --disable-fieldanalysis  --disable-freeverb \
                    --disable-frei0r --disable-gaudieffects --disable-geometrictransform \
                    --disable-gdp --disable-ivtc --disable-ivfparse  --disable-jp2kdecimator \
                    --disable-jpegformat --disable-librfb --disable-mpegtsmux  --disable-mpegpsmux  \
                    --disable-mve --disable-mxf --disable-netsim --disable-onvif --disable-pnm \
                    --disable-removesilence --disable-sdi --disable-sdp --disable-segmentclip \
                    --disable-siren --disable-subenc --disable-stereo  --disable-tta  \
                    --disable-videomeasure  --disable-videosignal --disable-vmnc \
                    --disable-y4m --disable-yadif  --enable-opengl=no \
                    --enable-wgl=no --enable-glx=no  --disable-directsound --disable-wasapi \
                    --disable-direct3d --disable-winscreencap  --disable-winks \
                    --disable-apple_media --disable-bluez --disable-vcd  --disable-opensles \
                    --disable-uvch264 --disable-nvenc   --disable-voamrwbenc --disable-voaacenc \
                    --disable-apexsink --disable-chromaprint --disable-curl --disable-decklink  \
                    --disable-directfb --disable-wayland    --disable-webp  --disable-daala \
                     --disable-resindvd   --disable-flite --disable-faad --disable-gsm --disable-fluidsynth \
                    --disable-kate --disable-ladspa  --disable-lv2  --disable-libde265  --disable-linsy  \
                    --disable-modplug --disable-mimic --disable-mpeg2enc  --disable-mplex   --disable-musepack  \
                    --disable-nas --disable-ofa --with-hls-crypto=openssl  --disable-rtmp CFLAGS="$CUR_CFLAGS" \
                    LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib"


        fi



       touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi

     make
     check_make_ret
     make install
     check_make_ret

     create_demux_dash_static_lib;

     cd $GST_TOP_DIR

      cp -fr $OUTPUT_PATH/lib/libgstplayer-1.0.a    $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstbadaudio-1.0.a   $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstbadvideo-1.0.a    $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstbadbase-1.0.a    $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstmpegts-1.0.a    $STATIC_OUTPUT_PATH/lib/
#      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstinsertbin-1.0.a    $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstcodecparsers-1.0.a    $STATIC_OUTPUT_PATH/lib/
 #     cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstbasecamerabinsrc-1.0.a    $STATIC_OUTPUT_PATH/lib/
  #    cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstphotography-1.0.a    $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgstadaptivedemux-1.0.a   $STATIC_OUTPUT_PATH/lib/
      cp -fr $OUTPUT_PATH/lib/libgsturidownloader-1.0.a    $STATIC_OUTPUT_PATH/lib/
   #   cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstdtls.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgsthls.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstdashdemux.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstneonhttpsrc.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstshm.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstfbdevsink.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstvideoparsersbad.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstvideofiltersbad.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstspeed.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstsmooth.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstrawparse.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstpcapparse.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmpegtsdemux.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmidi.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstinterlace.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstinter.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstid3tag.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstdvdspu.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstdvbsuboverlay.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
    #  cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstdebugutilsbad.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstautoconvert.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstvideoframe_audiolevel.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstadpcmdec.a   $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
     # cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstaccurip.a    $STATIC_OUTPUT_PATH/lib/gstreamer-1.0

     echo "finish compile badplugin=================="

}


