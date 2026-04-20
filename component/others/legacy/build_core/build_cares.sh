#!/bin/bash
#

#########################################################3
#  
#  build_c-ares
#
#############################################################
function  build_cares()
{

      echo "start to compile c-ares-1.13.0==================="

      cd $GST_TOP_DIR

     if [ $host_arm -eq 1 ];then
       echo "this is arm"
     elif [ $host_aarch64 -eq 1 ];then
       echo "this is aarch64"
     elif [ $host_mips -eq 1 ];then
       echo "this is mips"
     else
       echo "this is x86"
     fi



      if [ $rebuild_cares -eq 1 ];then

           # rm -fr thirdparty/opensssl-1.0.1l/
           # tar zxvf tarball_bak/openssl-1.0.1l.tar.gz  -C thirdparty/
            cd $GST_TOP_DIR/c-ares-1.13.0
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"


            if [ $host_arm -eq 1 ];then
              echo "host is arm"
              echo $GST_OUTPUT_PATH
              ./configure --prefix=$GST_OUTPUT_PATH --host=$CUR_HOST --enable-static=yes

            elif [ $host_aarch64 -eq 1 ];then
              echo "host is aarch64"
              echo $GST_OUTPUT_PATH
              ./configure --prefix=$GST_OUTPUT_PATH --host=$CUR_HOST --enable-static=yes

            elif [ $host_x86 -eq 1 ];then
              echo "host is x86"
              echo $GST_OUTPUT_PATH
              rm -fr config
              cp -fr  config.x86 config
              chmod +x config
              ./configure --prefix=$GST_OUTPUT_PATH
              awk '{gsub(/-m64/," -m64  -g");print}' Makefile > mk.tmp
              mv mk.tmp Makefile


           elif [ $host_mips -eq 1 ];then
              echo "host is mips"
              echo $GST_OUTPUT_PATH
			  echo $CUR_HOST
              ./configure --prefix=$GST_OUTPUT_PATH --host=$CUR_HOST --enable-static=yes


           fi
      fi


     cd $GST_TOP_DIR/c-ares-1.13.0


 #      make clean;
       make;
       check_make_ret
       make install
       check_make_ret


      cd $GST_TOP_DIR
      mkdir -p $STATIC_OUTPUT_PATH/lib $SHARE_OUTPUT_PATH/lib
      cp -fr $GST_OUTPUT_PATH/lib/libcares.a $STATIC_OUTPUT_PATH/lib
      cp -af $GST_OUTPUT_PATH/lib/libcares.so* $SHARE_OUTPUT_PATH/lib
      echo "finish compile c-ares-1.13.0==================="
}

