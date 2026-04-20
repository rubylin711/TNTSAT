#!/bin/bash
#

#########################################################3
#  
#  build_srt
#
#############################################################
function  build_srt()
{

      echo "start to compile srt==================="

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



      if [ $rebuild_srt -eq 1 ];then

            cd $GST_TOP_DIR/srt-1.4.1
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"


            if [ $host_arm -eq 1 ];then
              echo "host is arm"
              echo $GST_OUTPUT_PATH
              ./configure --prefix=$GST_OUTPUT_PATH  

            if [ $host_aarch64 -eq 1 ];then
              echo "host is aarch64"
              echo $GST_OUTPUT_PATH
              ./configure --prefix=$GST_OUTPUT_PATH  

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
              echo "host is mips in srt"
              echo $GST_OUTPUT_PATH
			  echo $CUR_HOST
              ./configure --prefix=$GST_OUTPUT_PATH  


           fi
      fi


     cd $GST_TOP_DIR/srt-1.4.1


       make;
       check_make_ret
       make install
       check_make_ret


      cd $GST_TOP_DIR
      cp -fr $GST_OUTPUT_PATH/lib/libsrt.a $STATIC_OUTPUT_PATH/lib
      cp -af $GST_OUTPUT_PATH/lib/libsrt.so* $SHARE_OUTPUT_PATH/lib
      echo "finish compile srt==================="
}

