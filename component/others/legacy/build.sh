#!/bin/bash
###########################################################################################
#
#  
#   initialize  output path to null
#
#
######################################################################################
source build_core/do_help.sh
#source build_core/build_openssl-1.0.1l.sh
source build_core/build_openssl-1.1.0i.sh
source build_core/build_ffi.sh
source build_core/build_xml2.sh
source build_core/build_pcre.sh
source build_core/build_sqlite.sh
source build_core/build_glib.sh
source build_core/build_gmp_soup.sh
source build_core/build_zlib.sh
source build_core/build_gst.sh
source build_core/build_libav.sh
source build_core/build_tasn1_gnutls_nettle_networking.sh
source build_core/build_cares.sh
source build_core/build_curl.sh
source build_core/build_b64.sh
source build_core/build_sspk.sh
#source build_core/build_bento4.sh
#source build_core/build_srt.sh
echo "start build gstreamer_1.8"
GST_ARM_OUTPUT_PATH=""
GST_AARCH64_OUTPUT_PATH=""
GST_MIPS_OUTPUT_PATH=""
GST_X86_OUTPUT_PATH=""
GST_OUTPUT_PATH=""


##############################################################3#############
#
#
#
#
#   initialize  variables which decide wether to build one module
#
#
#
#
###############################################################################3
build_libffi=0
build_libopenssl=0
build_libglib=0
build_libsoup=0
build_libneon=0
build_libsqlite=0
build_libxml2=0
build_libpcre=0
build_libz=0
build_bento4=0
build_libpluginbase=0
build_libplugingood=0
build_libpluginbad=0
build_libgst=0
build_libav_ffmpeg=0
build_montplugins=0
build_libgnutls=0
build_libgmp=0
build_libnettle=0
build_glibnetworking=0
build_libtasn1=0
build_only_gstreamer_modules=0
build_cares=0
build_curl=0
build_b64=0
build_sspk=0
build_srt=0

rebuild_all=0
rebuild_libopenssl=0
rebuild_libpluginbase=0
rebuild_libplugingood=0
rebuild_libpluginbad=0
rebuild_libgst=0
rebuild_libav_ffmpeg=0
rebuild_montplugins=0
rebuild_libxml2=0
rebuild_libglib=0
rebuild_libz=0
rebuild_bento4=0
rebuild_libpcre=0
rebuild_libsoup=0
rebuild_libneon=0
rebuild_libgnutls=0
rebuild_libgmp=0
rebuild_libnettle=0
rebuild_glibnetworking=0
rebuild_libtasn1=0
rebuild_cares=0
rebuild_curl=0
rebuild_b64=0
rebuild_sspk=0
rebuild_srt=0
start_release=0
start_delete=0

is_help=0
is_uclibc=0
is_debug=1
is_only_gst_debug=0
support_static_plugin=1
dump_gst_memory=0

#########################################################################3######
#
#
#
#    initialize variables relative to some  platform
#
#
#
#
###############################33##########################################3######
host_arm=0
host_aarch64=0
host_mips=0
host_x86=0




#########################################################################3######
#
#
#
#
#
#     NOTICE: don't move these codes to other place !!!!!!!!!!
#
#     we should process "all" command first
#
#
#
#
#
#
#########################################################################3######
echo $1 | grep -e "all"

  if [ $? -eq 0 ]; then

    echo "start to build all gstreamer plugins ..."

    if [ "$2" == "mips" ]; then
      
        host_mips=1
        host_x86=0
        host_arm=0
        host_aarch64=0
        echo "compile mips"

        if [ "$3" == "gstreamer" ]; then

          echo "host_mips=2" > .config
          build_only_gstreamer_modules=1

        else
          build_only_gstreamer_modules=0
          echo "host_mips=1" > .config

        fi


    elif [ "$2" == "x86" ]; then

        host_x86=1
        host_mips=0
        host_arm=0
        host_aarch64=0
        echo "compile x86"
        echo "host_x86=1" > .config
 
    elif [ "$2" == "arm" ]; then
        host_x86=0
        host_mips=0
        host_arm=1
        host_aarch64=0
        echo "compile arm"
        echo "host_arm=1" > .config

    elif [ "$2" == "aarch64" ]; then
        host_x86=0
        host_mips=0
        host_arm=0
        host_aarch64=1
        echo "compile aarch64"
        echo "host_aarch64=1" > .config

    else
        host_arm=1
        host_x86=0
        host_mips=0
        host_aarch64=0
        echo "compile arm"
        echo "host_arm=1" > .config

    fi    

     

     rebuild_libgst=1
     rebuild_cares=1
     rebuild_libopenssl=1
     rebuild_libpluginbase=1
     rebuild_libplugingood=1
     rebuild_libpluginbad=1
     rebuild_libav_ffmpeg=1
     rebuild_montplugins=1
     rebuild_libglib=1
     rebuild_libz=1
     rebuild_bento4=1
     rebuild_libxml2=1
     rebuild_libffi=1
     rebuild_libsqlite=1
     rebuild_libneon=1
     rebuild_curl=1
     rebuild_all=1
     rebuild_srt=1

     rebuild_libpcre=1
     rebuild_libsoup=1
     rebuild_libgnutls=1
     rebuild_libgmp=1
     rebuild_libnettle=1
     rebuild_glibnetworking=1
     rebuild_libtasn1=1


     build_libgst=1
     build_libopenssl=1
     build_libpluginbase=1
     build_libplugingood=1
     build_libpluginbad=1
     build_libav_ffmpeg=1
     build_montplugins=1
     build_libffi=1
     build_libglib=1
     build_libz=1
     build_bento4=1
     build_libsoup=1
     build_libneon=1
     build_libsqlite=1
     build_libxml2=1
     build_cares=1
     build_curl=1
     build_b64=1
     build_sspk=1
     build_srt=1

     build_libpcre=1
     build_libsoup=1
     build_libgnutls=1
     build_libgmp=1
     build_libnettle=1
     build_glibnetworking=1
     build_libtasn1=1

    
    ###only build modules relative to gstreamer
    if [ "$3" == "gstreamer" ]; then
          echo "only compile gstreamer"

          rebuild_libgst=1
          rebuild_libopenssl=1
          rebuild_libpluginbase=1
          rebuild_libplugingood=1
          rebuild_libpluginbad=1
          rebuild_libav_ffmpeg=1
          rebuild_montplugins=1
          rebuild_libglib=1
          rebuild_libz=1
          rebuild_bento4=1
          rebuild_libxml2=1
          rebuild_libffi=1
          rebuild_libsqlite=1
          rebuild_libneon=1

          ##########3
          rebuild_libgmp=0
          rebuild_libpcre=0
          rebuild_libnettle=0
          rebuild_glibnetworking=0
          rebuild_libtasn1=0
          rebuild_cares=0
          rebuild_libcurl=0
          rebuild_all=0
          rebuild_libsoup=0
          rebuild_libgnutls=0


          build_libgst=1
          build_libopenssl=1
          build_libpluginbase=1
          build_libplugingood=1
          build_libpluginbad=1
          build_libav_ffmpeg=1
          build_montplugins=1
          build_libffi=1
          build_libglib=1
          build_libz=1
          build_bento4=1
          build_libneon=1
          build_libsqlite=1
          build_libxml2=1
          build_only_gstreamer_modules=1

          ###########
          build_libgnutls=0
          build_libpcre=0
          build_libgmp=0
          build_libnettle=0
          build_glibnetworking=0
          build_libsoup=0
          build_libtasn1=0
          build_cares=0
          build_curl=0
		  build_b64=0
		  build_sspk=0

    fi


 fi





#########################################################################3##################################33
#
#
#
#
#
#
#                    read var from .config to initialize variable about platform
#
#
#
#
#
#
#
###############################33##########################################3#######################################

if [ -f ".config" ]; then
  echo "OK, find .config!!!"
  while read wOne wTwo
  do
    [ -z $wOne ] && continue
    
    line=$wOne                      

  done < .config
else
  echo "WARNING: not find .config, please run ./build.sh all arm(mips/x86/aarch64)!!!!"
fi

if [ -f ".env" ]; then
  echo "OK, find .env!!!"
  source .env
  cat .env
else
  echo "ERROR: not find .env, please run source env.sh"
  exit -1
fi

echo "start parse .config ..."

    if [ "$line" == "host_mips=1" ]; then     
        host_x86=0
        host_mips=1
        host_arm=0
        host_aarch64=0

    elif [ "$line" == "host_mips=2" ]; then     
        host_x86=0
        host_mips=1
        host_arm=0
        host_aarch64=0
        build_only_gstreamer_modules=1
       # echo "read config ok and compile mips"
    elif [ "$line" == "host_x86=1" ]; then
        host_x86=1
        host_mips=0
        host_arm=0
        host_aarch64=0
        #echo "read config ok and compile x86"
   elif [ "$line" == "host_arm=1" ]; then
        host_x86=0
        host_mips=0
        host_arm=1
        host_aarch64=0
        #    echo "read config ok and compile arm"
   elif [ "$line" == "host_aarch64=1" ]; then
        host_x86=0
        host_mips=0
        host_arm=0
        host_aarch64=1
        #    echo "read config ok and compile aarch64"
    else
        host_arm=0
        host_x86=0
        host_mips=0
        host_aarch64=0

        echo "do nothing : can't find .config"

        echo $1 | grep -e "help"
        if [ $? -eq 0 ]; then
          echo "do help command ..."
          #is_help=1
          mont_help;
          exit 1;
        fi
    fi    

echo "end parse .config ..."
echo "the result is:"
echo "host_arm:$host_arm"
echo "host_aarch64:$host_aarch64"
echo "host_x86:$host_x86"
echo "host_mips:$host_mips"



################################3#######################################################
#
#
#
#         1, set the variable 'GST_TOP_DIR'
#
#
#
#
#
###########################################################################################3
GST_TOP_DIR=`pwd`
echo $GST_TOP_DIR









###########################################################################
#
#
# 1, ./build.sh all   (build all the modules)
# 2, ./build.sh gst   (build  gstreamer-1.8.0)
# 3, ./build.sh base  (build gst-plugins-base-1.8.0)
# 4, ./build.sh good  (build gst-plugins-good-1.8.0)
# 5, ./build.sh bad   (build gst-plugins-bad-1.8.0)
# 6, ./build.sh av    (build gst-libav-1.8.0)
#
#
#
#
#E
###########################################################################


echo $1 | grep -e "release"
  if [ $? -eq 0 ]; then
     echo "ready to release..."
     start_release=1
  fi


echo $1 | grep -e "del"
  if [ $? -eq 0 ]; then
     echo "ready to del so..."
     start_delete=1
  fi



echo $1 | grep -e "ssl"
  if [ $? -eq 0 ]; then
     echo "need to build openssl ..."
     build_libopenssl=1
  fi


echo $1 | grep -e "ssl_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild openssl ..."
     rebuild_libopenssl=1
  fi


echo $1 | grep -e "base"
  if [ $? -eq 0 ]; then
     echo "need to build gst_plugins_base ..."
     build_libpluginbase=1
  fi

echo $1 | grep -e "base_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gst_plugins_base ..."
     rebuild_libpluginbase=1
  fi


echo $1 | grep -e "good"
  if [ $? -eq 0 ]; then
     echo "need to build gst_plugins_good ..."
     build_libplugingood=1
  fi
echo $1 | grep -e "good_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gst_plugins_good ..."
     rebuild_libplugingood=1
  fi


echo $1 | grep -e "bad"
  if [ $? -eq 0 ]; then
     echo "need to build gst_plugins_bad ..."
     build_libpluginbad=1
  fi
echo $1 | grep -e "bad_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gst_plugins_bad ..."
     rebuild_libpluginbad=1
  fi


echo $1 | grep -e "gst"
  if [ $? -eq 0 ]; then
     echo "need to build gstreamer-1.8.0 ..."
     build_libgst=1
  fi
echo $1 | grep -e "gst_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gstreamer-1.8.0 ..."
     rebuild_libgst=1
  fi


echo $1 | grep -e "av"
  if [ $? -eq 0 ]; then
     echo "need to build gstreamer-1.8.0 ..."
     build_libav_ffmpeg=1
  fi
echo $1 | grep -e "av_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gstreamer-1.8.0 ..."
     rebuild_libav_ffmpeg=1
  fi


echo $1 | grep -e "sqlite"
  if [ $? -eq 0 ]; then
     echo "need to build sqlite ..."
     build_libsqlite=1
  fi
echo $1 | grep -e "sqlite_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild sqlite ..."
     rebuild_libsqlite=1
  fi

echo $1 | grep -e "ffi"
  if [ $? -eq 0 ]; then
     echo "need to build ffi ..."
     build_libffi=1
  fi
echo $1 | grep -e "ffi_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild ffi ..."
     rebuild_libffi=1
  fi

echo $1 | grep -e "pcre"
  if [ $? -eq 0 ]; then
     echo "need to build pcre..."
     build_libpcre=1
  fi
echo $1 | grep -e "pcre_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild prce ..."
     rebuild_libpcre=1
  fi


echo $1 | grep -e "soup"
  if [ $? -eq 0 ]; then
     echo "need to build soup..."
     build_libsoup=1
  fi
echo $1 | grep -e "soup_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild soup ..."
     rebuild_libsoup=1
  fi



echo $1 | grep -e "neon"
  if [ $? -eq 0 ]; then
     echo "need to build neon..."
     build_libneon=1
  fi
echo $1 | grep -e "neon_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild neon ..."
     rebuild_libneon=1
  fi



echo $1 | grep -e "glib"
  if [ $? -eq 0 ]; then
     echo "need to build glib ..."
     build_libglib=1
  fi
echo $1 | grep -e "glib_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild glib ..."
     rebuild_libglib=1
  fi


echo $1 | grep -e "gmp"
  if [ $? -eq 0 ]; then
     echo "need to build gmp ..."
     build_libgmp=1
  fi

echo $1 | grep -e "gmp_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gmp..."
     rebuild_libgmp=1
  fi



echo $1 | grep -e "gnutls"
  if [ $? -eq 0 ]; then
     echo "need to build gnutls ..."
     build_libgnutls=1
  fi

echo $1 | grep -e "gnutls_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild gnutls..."
     rebuild_libgnutls=1
  fi




echo $1 | grep -e "nettle"
  if [ $? -eq 0 ]; then
     echo "need to build nettle ..."
     build_libnettle=1
  fi
echo $1 | grep -e "nettle_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild nettle..."
     rebuild_libnettle=1
  fi


echo $1 | grep -e "tasn1"
  if [ $? -eq 0 ]; then
     echo "need to build  ..."
     build_libtasn1=1
  fi
echo $1 | grep -e "tasn1_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild tansl..."
     rebuild_libtasn1=1
  fi


echo $1 | grep -e "glibnet"
  if [ $? -eq 0 ]; then
     echo "need to build  ..."
     build_glibnetworking=1
  fi
echo $1 | grep -e "glibnet_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild glibnetworking..."
     rebuild_glibnetworking=1
  fi



echo $1 | grep -e "zlib"
  if [ $? -eq 0 ]; then
     echo "need to build zlib ..."
     build_libz=1
  fi
echo $1 | grep -e "zlib_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild zlib ..."
     rebuild_libz=1
  fi


echo $1 | grep -e "bento4"
  if [ $? -eq 0 ]; then
     echo "need to build bento4 ..."
     build_bento4=1
  fi
echo $1 | grep -e "bento4_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild bento4 ..."
     rebuild_bento4=1
  fi


echo $1 | grep -e "xml2"
  if [ $? -eq 0 ]; then
     echo "need to build xml2 ..."
     build_libxml2=1
  fi
echo $1 | grep -e "xml2_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild xml2..."
     rebuild_libxml2=1
  fi


echo $1 | grep -e "mont"
  if [ $? -eq 0 ]; then
     echo "need to build montage plugins ..."
     build_montplugins=1
  fi

echo $1 | grep -e "mont_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild montage plugins ..."
     rebuild_montplugins=1
  fi


echo $1 | grep -e "cares"
  if [ $? -eq 0 ]; then
     echo "need to build cares ..."
     build_cares=1
  fi

echo $1 | grep -e "cares_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild cares ..."
     rebuild_cares=1
  fi

echo $1 | grep -e "curl"
  if [ $? -eq 0 ]; then
     echo "need to build curl ..."
     build_curl=1
  fi

echo $1 | grep -e "curl_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild curl ..."
     rebuild_curl=1
  fi

echo $1 | grep -e "b64"
  if [ $? -eq 0 ]; then
     echo "need to build b64 ..."
     build_b64=1
  fi
  
echo $1 | grep -e "b64_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild b64 ..."
     rebuild_b64=1
  fi

echo $1 | grep -e "sspk"
  if [ $? -eq 0 ]; then
     echo "need to build sspk ..."
     build_sspk=1
  fi
  
echo $1 | grep -e "sspk_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild sspk ..."
     rebuild_sspk=1
  fi
  
echo $1 | grep -e "help"
  if [ $? -eq 0 ]; then
     echo "new list commands ..."
     is_help=1
  fi


echo $1 | grep -e "srt"
  if [ $? -eq 0 ]; then
     echo "need to build srt ..."
     build_srt=1
  fi

echo $1 | grep -e "srt_clean"
  if [ $? -eq 0 ]; then
     echo "need to rebuild srt ..."
     rebuild_srt=1
  fi

###########################################################################
#
#
#
#
#
#
# 2, set the variables  'GST_TOP_DIR'
#                       'LD_LIBRARY_PATH' 'PKG_CONFIG_PATH' 'GST_PLUGIN_PATH'
#
#
#
#
#
#
######################################################################################

if [ $host_arm -eq 1 ];then

   echo "This is Arm archietchure ..."

    if [ -z "$GST_ARM_OUTPUT_PATH" ];then
        GST_ARM_OUTPUT_PATH="$GST_TOP_DIR/output/${CFG_MT_OSS_SANITIZE_DIR}/arm_output"
        STATIC_OUTPUT_PATH="$GST_ARM_OUTPUT_PATH/../arm_output_static"
		SHARE_OUTPUT_PATH="$GST_ARM_OUTPUT_PATH/../arm_output_shared"
        if [ ! -d "$GST_ARM_OUTPUT_PATH" ]; then
           echo "create $GST_ARM_OUTPUT_PATH!!!"   
           mkdir -p $GST_ARM_OUTPUT_PATH
        fi

        if [ ! -d "$STATIC_OUTPUT_PATH" ]; then
           echo "create $STATIC_OUTPUT_PATH!!!"   
           mkdir -p $STATIC_OUTPUT_PATH/lib
           mkdir -p $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
        fi
		
		if [ ! -d "$SHARE_OUTPUT_PATH" ]; then
           echo "create $SHARE_OUTPUT_PATH!!!"   
           mkdir -p $SHARE_OUTPUT_PATH/lib
        fi

    else
      
        echo "$GST_ARM_OUTPUT_PATH already exist!!!"   
    fi
 	
    export TARGET_CPU=arm
    export STRIP=arm-linux-gnueabihf-strip
    export PATH=/usr/local/linaro/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    export CC="arm-linux-gnueabihf-gcc ${CFG_MT_OSS_ARCH_CFLAGS} ${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS}"
    export CXX="arm-linux-gnueabihf-g++ ${CFG_MT_OSS_ARCH_CFLAGS} ${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS}"
    export LD="arm-linux-gnueabihf-ld ${CFG_MT_OSS_ARCH_LDFLAGS} ${CFG_MT_OSS_SANITIZE_LD} ${CFG_MT_OSS_BASE_LDFLAGS}"
    export ARCH=arm
    export AR=arm-linux-gnueabihf-ar
    export RANLIB=arm-linux-gnueabihf-ranlib
 
    export LD_LIBRARY_PATH=$GST_ARM_OUTPUT_PATH/lib
    export PKG_CONFIG_PATH=$GST_ARM_OUTPUT_PATH/lib/pkgconfig
    export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
    export GST_PLUGIN_PATH=$GST_ARM_OUTPUT_PATH/lib
    export STATIC_OUTPUT_PATH=$STATIC_OUTPUT_PATH
    export SHARE_OUTPUT_PATH=$SHARE_OUTPUT_PATH

    export GST_OUTPUT_PATH=$GST_ARM_OUTPUT_PATH

elif [ $host_aarch64 -eq 1 ];then

   echo "This is AArch64 archietchure ..."

    if [ -z "$GST_AARCH64_OUTPUT_PATH" ];then
        GST_AARCH64_OUTPUT_PATH="$GST_TOP_DIR/output/${CFG_MT_OSS_SANITIZE_DIR}/aarch64_output"
        STATIC_OUTPUT_PATH="$GST_AARCH64_OUTPUT_PATH/../aarch64_output_static"
		SHARE_OUTPUT_PATH="$GST_AARCH64_OUTPUT_PATH/../aarch64_output_shared"
        if [ ! -d "$GST_AARCH64_OUTPUT_PATH" ]; then
           echo "create $GST_AARCH64_OUTPUT_PATH!!!"
           mkdir -p $GST_AARCH64_OUTPUT_PATH
        fi

        if [ ! -d "$STATIC_OUTPUT_PATH" ]; then
           echo "create $STATIC_OUTPUT_PATH!!!"
           mkdir -p $STATIC_OUTPUT_PATH/lib
           mkdir -p $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
        fi

		if [ ! -d "$SHARE_OUTPUT_PATH" ]; then
           echo "create $SHARE_OUTPUT_PATH!!!"
           mkdir -p $SHARE_OUTPUT_PATH/lib
        fi

    else

        echo "$GST_AARCH64_OUTPUT_PATH already exist!!!"
    fi

    export TARGET_CPU=aarch64
    export STRIP=aarch64-none-linux-gnu-strip
    export PATH=/usr/local/linaro/arm-gnu-toolchain-12.3.rel1-x86_64-aarch64-none-linux-gnu/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    export CC="aarch64-none-linux-gnu-gcc ${CFG_MT_OSS_ARCH_CFLAGS} ${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS}"
    export CXX="aarch64-none-linux-gnu-g++ ${CFG_MT_OSS_ARCH_CFLAGS} ${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS}"
    export LD="aarch64-none-linux-gnu-ld ${CFG_MT_OSS_ARCH_LDFLAGS} ${CFG_MT_OSS_SANITIZE_LD} ${CFG_MT_OSS_BASE_LDFLAGS}"
    export ARCH=aarch64
    export AR=aarch64-none-linux-gnu-ar
    export RANLIB=aarch64-none-linux-gnu-ranlib

    export LD_LIBRARY_PATH=$GST_AARCH64_OUTPUT_PATH/lib
    export PKG_CONFIG_PATH=$GST_AARCH64_OUTPUT_PATH/lib/pkgconfig
    export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
    export GST_PLUGIN_PATH=$GST_AARCH64_OUTPUT_PATH/lib
    export STATIC_OUTPUT_PATH=$STATIC_OUTPUT_PATH
    export SHARE_OUTPUT_PATH=$SHARE_OUTPUT_PATH

    export GST_OUTPUT_PATH=$GST_AARCH64_OUTPUT_PATH

elif [ $host_mips -eq 1 ];then 

    export STRIP=mipsel-linux-gnu-strip
    export TARGET_CPU=mips
    echo "This is MIPS archietchure ..."

    if [ -z "$GST_MIPS_OUTPUT_PATH" ];then

        if [ $is_uclibc -eq 0 ];then
          GST_MIPS_OUTPUT_PATH="$GST_TOP_DIR/output/${CFG_MT_OSS_SANITIZE_DIR}/mips_output"
          STATIC_OUTPUT_PATH="$GST_MIPS_OUTPUT_PATH/../mips_output_static"
		  SHARE_OUTPUT_PATH="$GST_MIPS_OUTPUT_PATH/../mips_output_shared"
        else
          GST_MIPS_OUTPUT_PATH="$GST_TOP_DIR/output/mips_output/uclibc"
        fi
        echo "create $GST_MIPS_OUTPUT_PATH!!!"   

        if [ ! -d "$GST_MIPS_OUTPUT_PATH" ]; then
           mkdir -p $GST_MIPS_OUTPUT_PATH
        fi

        if [ ! -d "$STATIC_OUTPUT_PATH" ]; then
           echo "create $STATIC_OUTPUT_PATH!!!"   
           mkdir -p $STATIC_OUTPUT_PATH/lib
           mkdir -p $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
        fi

		if [ ! -d "$SHARE_OUTPUT_PATH" ]; then
           echo "create $SHARE_OUTPUT_PATH!!!"   
           mkdir -p $SHARE_OUTPUT_PATH/lib
        fi

    else
      
        echo "$GST_MIPS_OUTPUT_PATH already exist!!!"   
    fi
 	
    export TARGET_CPU=mips
    export STRIP=mipsel-linux-gnu-strip
    #export PATH=/usr/local/codesourcery/mips-4.3/bin:/opt/mont_glib/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    #export PATH=/usr/local/codesourcery/mips-2016.05/bin:/opt/mont_glib/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    #export PATH=/usr/local/crosstool-ng/mipsel-mt-linux-gnu/bin:/opt/mont_glib/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    export PATH=/usr/local/crosstool-ng/gcc-9.3-glibc-2.28-mipsel-linux-gnu-rm2.0/bin:/opt/mont_glib/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    export CC="mipsel-linux-gnu-gcc ${CFG_MT_OSS_ARCH_CFLAGS} ${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS}"
    export CXX="mipsel-linux-gnu-g++ ${CFG_MT_OSS_ARCH_CFLAGS} ${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS}"
    export LD="mipsel-linux-gnu-ld ${CFG_MT_OSS_ARCH_LDFLAGS} ${CFG_MT_OSS_SANITIZE_LD} ${CFG_MT_OSS_BASE_LDFLAGS}"
    export ARCH=mips
    export AR=mipsel-linux-gnu-ar
    export RANLIB=mipsel-linux-gnu-ranlib
    export LD_LIBRARY_PATH=$GST_MIPS_OUTPUT_PATH/lib
    export PKG_CONFIG_PATH=$GST_MIPS_OUTPUT_PATH/lib/pkgconfig
    export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
    export GST_PLUGIN_PATH=$GST_MIPS_OUTPUT_PATH/lib
    export STATIC_OUTPUT_PATH=$STATIC_OUTPUT_PATH
	export SHARE_OUTPUT_PATH=$SHARE_OUTPUT_PATH

    export GST_OUTPUT_PATH=$GST_MIPS_OUTPUT_PATH

else
    export TARGET_CPU=x86
    #echo "host is X86...."
    echo "This is X86 archietchure ..."

    if [ -z "$GST_X86_OUTPUT_PATH" ];then
        GST_X86_OUTPUT_PATH="$GST_TOP_DIR/output/${CFG_MT_OSS_SANITIZE_DIR}/x86_output"
        STATIC_OUTPUT_PATH="$GST_X86_OUTPUT_PATH/../x86_output_static"
		SHARE_OUTPUT_PATH="$GST_X86_OUTPUT_PATH/../x86_output_shared"
        echo "create $GST_X86_OUTPUT_PATH!!!"   

        if [ ! -d "$GST_X86_OUTPUT_PATH" ]; then
           echo "create $GST_X86_OUTPUT_PATH!!!"   
           mkdir -p $GST_X86_OUTPUT_PATH
        fi

        if [ ! -d "$STATIC_OUTPUT_PATH" ]; then
           echo "create $STATIC_OUTPUT_PATH!!!"   
           mkdir -p $STATIC_OUTPUT_PATH/lib
           mkdir -p $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
        fi

		if [ ! -d "$SHARE_OUTPUT_PATH" ]; then
           echo "create $SHARE_OUTPUT_PATH!!!"   
           mkdir -p $SHARE_OUTPUT_PATH/lib
        fi

    else
      
        echo "$GST_X86_OUTPUT_PATH already exist!!!"   
    fi
 	
    export PATH=$GST_X86_OUTPUT_PATH/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
    echo $PATH
    export CC=gcc
    export STRIP=strip
    export CXX=g++
    export LD=ld
    export ARCH=x86
    export AR=ar
    export RANLIB=ranlib
    export LD_LIBRARY_PATH=$GST_X86_OUTPUT_PATH/lib
    export PKG_CONFIG_PATH=$GST_X86_OUTPUT_PATH/lib/pkgconfig
    export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
    export GST_PLUGIN_PATH=$GST_X86_OUTPUT_PATH/lib
    export STATIC_OUTPUT_PATH=$STATIC_OUTPUT_PATH
	export SHARE_OUTPUT_PATH=$SHARE_OUTPUT_PATH

    export GST_OUTPUT_PATH=$GST_X86_OUTPUT_PATH

fi


   export OUTPUT_PATH=$GST_OUTPUT_PATH
   echo "GST_OUTPUT_PATH is:$GST_OUTPUT_PATH"
   echo "PKG_CONFIG_PATH is:$PKG_CONFIG_PATH"
   echo "PKG_CONFIG_LIBDIR is:$PKG_CONFIG_LIBDIR"
   echo "GST_PLUGIN_PATH is:$GST_PLUGIN_PATH"
   echo "OUTPUT_PATH is:$OUTPUT_PATH"
   echo "STATIC_OUTPUT_PATH is:$STATIC_OUTPUT_PATH"
   echo "SHARE_OUTPUT_PATH is:$SHARE_OUTPUT_PATH"

   TARGET_VENDOR="buildroot"
   if [ $host_arm -eq 1 ];then
     BUILDROOT_HOST_DIR="arm-${TARGET_VENDOR}-linux-gnueabihf"
   elif [ $host_aarch64 -eq 1 ];then
     BUILDROOT_HOST_DIR="aarch64-${TARGET_VENDOR}-linux-gnu"
   elif [ $host_mips -eq 1 ];then
     BUILDROOT_HOST_DIR="mipsel-${TARGET_VENDOR}-linux-gnu"
   else
     BUILDROOT_HOST_DIR="x86_64-${TARGET_VENDOR}-linux-gnu"
   fi
   if [ -n "${BR2_VERSION}" ]; then
     #TARGET_DIR and STAGING_DIR be exported in buildroot top Makefile
     BUILDROOT_SYSROOT_DIR="${STAGING_DIR}"
   else
     #buildroot relative path, we can move buildroot dir to other place
     CFG_MT_BUILDROOT_REL_PATH="./buildroot"
     BUILDROOT_DIR="${SDK_DIR}/${CFG_MT_BUILDROOT_REL_PATH}"
     BUILDROOT_SYSROOT_DIR="${BUILDROOT_DIR}/output/host/$(BUILDROOT_HOST_DIR)/sysroot"
   fi
   BUILDROOT_SYSROOT_USR_DIR="${BUILDROOT_SYSROOT_DIR}/usr"
   BUILDROOT_SYSROOT_USR_INC_DIR="${BUILDROOT_SYSROOT_USR_DIR}/include"
   BUILDROOT_SYSROOT_USR_LIB_DIR="${BUILDROOT_SYSROOT_USR_DIR}/lib"
   export BUILDROOT_SYSROOT_USR_INC_DIR BUILDROOT_SYSROOT_USR_LIB_DIR
   echo "BUILDROOT_SYSROOT_USR_INC_DIR=${BUILDROOT_SYSROOT_USR_INC_DIR}"
   echo "BUILDROOT_SYSROOT_USR_LIB_DIR=${BUILDROOT_SYSROOT_USR_LIB_DIR}"




########################################################################################################################33
#
#
#
#                  set globle CFLAGS   
#
#
#
#
#
#
#
#
#
#
###########################################################################################################################


  CUR_CFLAGS="-D_LARGEFILE_SOURCE \
              -D_FILE_OFFSET_BITS=64 \
              -D_LARGEFILE64_SOURCE \
              -D_GNU_SOURCE \
              -Wall  \
              -ffunction-sections \
              -fdata-sections \
              -O2 \
              -Wl,--gc-sections"

if [ $is_debug -eq 1 ];then
  CUR_CFLAGS+=" -g "
fi

if [ $is_only_gst_debug -eq 1 ];then
  CUR_CFLAGS+=" -DONLY_GST_DEBUG "
fi

if [ $dump_gst_memory -eq 1 ];then
  CUR_CFLAGS+=" -DDUMP_GST_MEMORY "
fi

if [ $host_arm -eq 1 ];then

  echo "cur_host is arm"

  CUR_HOST="arm-linux-gnueabihf"
  CUR_CFLAGS+="  -DPLATFORM_ARM"
  CUR_CFLAGS+="  -DXXXXXXXXXXXXX_YOURSELF1111"
#  CUR_CFLAGS+="  -DMT_DEBUG_GSTBIN"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX2"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX3"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX3"


elif [ $host_aarch64 -eq 1 ];then

  echo "cur_host is aarch64"

  CUR_HOST="aarch64-none-linux-gnu"
  CUR_CFLAGS+="  -DPLATFORM_AARCH64"
  CUR_CFLAGS+="  -DXXXXXXXXXXXXX_YOURSELF1111"
#  CUR_CFLAGS+="  -DMT_DEBUG_GSTBIN"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX2"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX3"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX3"


elif [ $host_x86 -eq 1 ];then
 
 echo "cur_host is x86"
  CUR_CFLAGS+="  -DPLATFORM_X86"
  CUR_CFLAGS+="  -DXXXXXXXXXXXXX_YOURSELF2222"
#  CUR_CFLAGS+="  -DMT_DEBUG_GSTBIN"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX3"


elif [ $host_mips -eq 1 ];then

  echo "cur_host is mips"
  CUR_HOST="mipsel-linux-gnu"
  CUR_CFLAGS+="  -DPLATFORM_MIPS"
  CUR_CFLAGS+="  -DXXXXXXXXXXXXX_YOURSELF3333"

  if [ $is_uclibc -eq 1 ];then
     CUR_CFLAGS+="  -muclibc"
  fi
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX2"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX3"
#  CUR_CFLAGS+="  -DXXXXXXXXXXXXX4"


else

 echo "cur_host is unkown"

fi

echo "CUR_CFLAGS is $CUR_CFLAGS"



####################################################################################
#
#
#
#
#
#
#
#
#
#
#
####################################################################################3

function clear_output_dir()
{

  echo "here is clear_output_dir!!!!"
  cd $GST_TOP_DIR/
  find ./ -name "*.o" | xargs rm -fr
  find ./ -name "*.d" | xargs rm -fr
  find ./ -name ".libs" | xargs rm -fr
  find ./ -name "*.lo" | xargs rm -fr
  find ./ -name ".deps" | xargs rm -fr

  if [ $support_static_plugin -eq 1 ];then
   # rm -fr $GST_OUTPUT_PATH/*
   echo "not clear output"
  fi

}



function check_make_ret()
{

   if [ $? -eq 0 ]; then
        echo $1
        echo "ok compile success !"
   else
        echo $1
        echo "fail to compile  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
        exit 1
   fi

}


function build_mont_plugin()
{

      echo "start to compile mont plugin======"

      cd $GST_TOP_DIR/gst-template/gst-plugin

      if [ $rebuild_montplugins -eq 1 ];then

         echo "$CUR_HOST"
         make distclean;
         echo "$GST_OUTPUT_PATH"
         ./autogen.sh $GST_OUTPUT_PATH  $CUR_HOST
         export GST_CFLAGS="-I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  -I$GST_OUTPUT_PATH/include/gstreamer-1.0/"
         export GST_LIBS="-L$GST_OUTPUT_PATH/lib -lgstreamer-1.0 -lgstvideo-1.0 -lgstaudio-1.0 "
	 CUR_CFLAGS+="  -I$GST_OUTPUT_PATH/lib/gstreamer-1.0/include  \
                         -I$GST_OUTPUT_PATH/include/gstreamer-1.0  \
                         -I$GST_OUTPUT_PATH/lib/glib-2.0/include \
                         -I$GST_OUTPUT_PATH/include/glib-2.0  \
                         -I$GST_OUTPUT_PATH/include/gstreamer-1.0/gst \
                         -I$GST_TOP_DIR/../kware/libmonplayer/avplay_instance/ \
                         -I$GST_TOP_DIR/../common/inc/ "


        if [ $support_static_plugin -eq 1 ];then
          CUR_CFLAGS+=" -DGST_PLUGIN_BUILD_STATIC "
        fi

        export CFLAGS=$CUR_CFLAGS

        ./configure --prefix=$GST_OUTPUT_PATH --host=$CUR_HOST  --disable-silent-rules --enable-static --enable-static-plugins
 
      fi

        cd src
        make clean;
        make;
        check_make_ret
        make install
        check_make_ret
      
      cd $GST_TOP_DIR


      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmontvsink.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmontasink.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmontssink.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmonvdec.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmonadec.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstvdec.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstadec.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0
      cp -fr $OUTPUT_PATH/lib/gstreamer-1.0/libgstmonmulti_queue.a  $STATIC_OUTPUT_PATH/lib/gstreamer-1.0

     echo "finish compile mont plugin================"

}


#####################################################################3
#
#
#
#
#
#######################################################################3




##########################################


function  do_delelte_invalid_libs()
{
  echo "we will delete invalid dynamic libraries and header files!!!!!!!!!!!!!!!!"
  cd $GST_OUTPUT_PATH

  RM_FILES="lib/gstreamer-1.0/include  lib/glib-2.0/include lib/gio/modules lib/glib-2.0"
  RM_FILES+=" lib/gstreamer-1.0/libgstadder.so  \
              lib/gstreamer-1.0/libgstvolume.so  \
              lib/gstreamer-1.0/libgstvideotestsrc.so \
              lib/gstreamer-1.0/libgstvideoscale.so \
              lib/gstreamer-1.0/libgstvideorate.so \
              lib/gstreamer-1.0/libgstvideoframe_audiolevel.so \
              lib/gstreamer-1.0/libgsttcp.so \
              lib/gstreamer-1.0/libgstspeed.so \
              lib/gstreamer-1.0/libgstshm.so \
              lib/gstreamer-1.0/libgstshapewipe.so \
              lib/gstreamer-1.0/libgstnavigationtest.so \
              lib/gstreamer-1.0/libgstinterlace.so \
              lib/gstreamer-1.0/libgstfbdevsink.so \
              lib/gstreamer-1.0/libgstencodebin.so  \
              lib/gstreamer-1.0/libgstdtmf.so \
              lib/gstreamer-1.0/libgstautodetect.so \
              lib/gstreamer-1.0/libgstaudiotestsrc.so \
              lib/gstreamer-1.0/libgstaudioresample.so \
              lib/gstreamer-1.0/libgstaudiorate.so \
              lib/gstreamer-1.0/libgstadpcmdec.so \
              lib/gstreamer-1.0/libgstadder.so \
              lib/gstreamer-1.0/libgstaccurip.so  \
              lib/gstreamer-1.0/libgstflxdec.so.so  \
              lib/gstreamer-1.0/libgstsmooth.so  \
              lib/gstreamer-1.0/libgstsmoothstreaming.so  \
              lib/gstreamer-1.0/libgstsmpte.so \
              lib/gstreamer-1.0/libgstdebug.so \
              lib/gstreamer-1.0/libgstdebugutilsbad.so \
              lib/gstreamer-1.0/libgstdvbsuboverlay.so \
              lib/gstreamer-1.0/libgstautoconvert.so \
              lib/gstreamer-1.0/libgstaudiofx.so \
              lib/gstreamer-1.0/libgstapp.so \
              lib/libgstbasecamerabinsrc* \
              lib/libgstphotography-1.0* \
              lib/libgstplayer-1.0* \
              lib/libgstsdp-1.0* \
              lib/libgstrtp-1.0* \
              lib/libgstrtsp-1.0* \
              lib/libgstinsertbin* \
              lib/pkgconfig "

  echo "here are :$RM_FILES"
  rm -fr $RM_FILES
  echo "finish remove invalid files!!!"

}


#####################################################################3
#
#
#
#
#
#######################################################################3


function do_delete_so()
{

  echo "delete invalid dynamic libraries!!!!!!!!!!!!!!!!"

}

#####################################################################3
#
#
#
#
#
#######################################################################3


function  do_release()
{

  echo "release lallalala "

  do_delelte_invalid_libs;

  cd $GST_TOP_DIR
  rm -fr release/*
  cp -fr $GST_OUTPUT_PATH/*  release/
  find ./release/ -name "*.a" | xargs rm -fr
  find ./release/ -name "*.la" | xargs rm -fr
  rm -fr ./release/libexec
  rm -fr ./release/share
  rm -fr ./release/include
  chmod 750 ./release/lib -R
  
  cd ./release/bin;
 # rm -fr !(gst-play-1.0 | gst-inspect-1.0)
  cp -fr  gst-play-1.0 ~/
  cp -fr  gst-inspect-1.0 ~/
  rm -fr  *
  cp -fr ~/gst-play-1.0   .
  cp -fr ~/gst-inspect-1.0  .
  cd ../../
  
  find ./release/ -type f |xargs -I{} file "{}"|grep "ELF\|ar "|sed 's/\(.*\):.*/\1/'|xargs $STRIP

  echo "end do_release"
#  cd ./release
 



}


function install_lib_to_pub_dir()
{

  # echo "Current $FUNCNAME, \$FUNCNAME => (${FUNCNAME[@]})"
   LINUX_TOP_PATH=$GST_TOP_DIR/../
   cd $LINUX_TOP_PATH
   echo `pwd`
   source envsetup ${MFRS_CFG}
   cd $GST_TOP_DIR
   make install


}

##########################################
#
#
#
#
#   main
#
#############################################

function main()
{

   if [ $is_help -eq 1 ]; then
      mont_help;
   fi

#   if [ $build_srt -eq 1 ]; then
#      build_srt;
#   fi

   if [ $start_release -eq 1 ]; then
      do_release;
   fi

   if [ $start_delete -eq 1 ]; then
      do_delete_so;
   fi

   if [ $rebuild_all -eq 1 ]; then
      clear_output_dir;
   fi

   if [ $build_libz -eq 1 ]; then
     build_zlib;
   fi

   ###############must build openssl first#######################
   if [ $build_libopenssl -eq 1 ]; then
       build_openssl;
   fi

   if [ $build_cares -eq 1 ]; then
       build_cares;
   fi

   if [ $build_libsqlite -eq 1 ]; then
      build_sqlite;
   fi

   if [ $build_libneon -eq 1 ]; then
      build_neon;
   fi

   if [ $build_curl -eq 1 ]; then
      build_curl;
   fi

   if [ $build_b64 -eq 1 ]; then
      build_b64;
   fi

   if [ $build_sspk -eq 1 ]; then
      build_sspk
   fi

   ##################the following libs are for gstreamer##############################

   if [ $build_libffi -eq 1 ]; then
     build_ffi;
   fi

   if [ $build_libxml2 -eq 1 ]; then
      build_xml2;
   fi
   if [ $build_libpcre -eq 1 ]; then
     build_pcre;
   fi
   if [ $is_uclibc -eq 1 ]; then
     echo "build ok lalala !!!"
     return 0
   fi





   if [ $build_libglib -eq 1 ]; then
      build_glib;
   fi

   if [ $build_libgmp -eq 1 ]; then
      build_gmp;
   fi

   if [ $build_libnettle -eq 1 ]; then
      build_nettle;
   fi

   if [ $build_libtasn1 -eq 1 ]; then
      build_tasn1;
   fi

   if [ $build_libgnutls -eq 1 ]; then
      build_gnutls;
   fi

   if [ $build_glibnetworking -eq 1 ]; then
      build_glib_networking;
   fi

   if [ $build_libsoup -eq 1 ]; then
      build_soup;
   fi

   if [ $build_bento4 -eq 1 ]; then
     echo "must compile drm before bento4"
     echo "build bento4 in opensource/Makefile"
     #build_bento4;
     echo "not use gst now, compile done here"
     return
   fi

   if [ $build_libgst -eq 1 ]; then
      build_gst;
   fi

   if [ $build_libpluginbase -eq 1 ]; then
      build_plugin_base;
   fi

   if [ $build_libplugingood -eq 1 ]; then
      build_plugin_good;
   fi

   if [ $build_libpluginbad -eq 1 ]; then
      build_plugin_bad;
   fi

   if [ $build_libav_ffmpeg -eq 1 ]; then
      build_libav;
   fi

   if [ $build_montplugins -eq 1 ]; then
      build_mont_plugin;
      if [ $build_only_gstreamer_modules -eq 1 ]; then
         echo "build gstreamer relative modules ok!!!!!!!!!!!!"
         return
      fi
   fi

 #  if [ $host_mips -eq 1 ]; then

  #    install_lib_to_pub_dir;
 #  fi
  # make install
}


main "$@"
##########################################################3
#
#
