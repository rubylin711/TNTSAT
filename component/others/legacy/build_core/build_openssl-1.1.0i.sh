#!/bin/bash
#

#########################################################3
#
#  build_openssl
#
#############################################################
function  build_openssl()
{

      echo "start to compile openssl==================="

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



      if [ $rebuild_libopenssl -eq 1 ];then

           # rm -fr thirdparty/opensssl-1.0.1l/
           # tar zxvf tarball_bak/openssl-1.0.1l.tar.gz  -C thirdparty/
            cd $GST_TOP_DIR/openssl-1.1.0i
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"


            if [ $host_arm -eq 1 ];then

              echo "host is arm"
              echo $GST_OUTPUT_PATH
              ./config no-asm  -shared --prefix=$GST_OUTPUT_PATH --openssldir=$GST_OUTPUT_PATH/ssl
              awk '{gsub(/-fPIC/," -g -fPIC ");print}' Makefile > mk.tmp
              mv mk.tmp Makefile

            elif [ $host_aarch64 -eq 1 ];then

              echo "host is aarch64"
              echo $GST_OUTPUT_PATH
              ./config no-asm  -shared --prefix=$GST_OUTPUT_PATH --openssldir=$GST_OUTPUT_PATH/ssl
              awk '{gsub(/-fPIC/," -g -fPIC ");print}' Makefile > mk.tmp
              mv mk.tmp Makefile

            elif [ $host_x86 -eq 1 ];then

              echo "host is x86"
              echo $GST_OUTPUT_PATH
              rm -fr config
              cp -fr  config.x86 config
              chmod +x config
              ./config no-asm -shared --prefix=$GST_OUTPUT_PATH --openssldir=$GST_OUTPUT_PATH/ssl
              awk '{gsub(/-m64/," -m64  -g");print}' Makefile > mk.tmp
              mv mk.tmp Makefile


           elif [ $host_mips -eq 1 ];then

              echo "host is mips"
              echo $GST_OUTPUT_PATH
              rm -fr mk.tmp mk.tmp1 mk.tmp2 mk.tmp3
              ./config no-asm no-threads -shared --prefix=$GST_OUTPUT_PATH --openssldir=$GST_OUTPUT_PATH/ssl


                if [ $is_debug -eq 1 ];then
                  awk '{gsub(/-fPIC/," -msoft-float -g -fPIC ");print}' Makefile > mk.tmp
                else
                  awk '{gsub(/-fPIC/," -msoft-float  -fPIC ");print}' Makefile > mk.tmp
                fi

               # awk '{gsub(/CROSS_COMPILE11111111111111= /,"CROSS_COMPILE=mips-linux-gnu-");print}' mk.tmp > mk.tmp1
                awk '{gsub(/-m64/," -g ");print}' mk.tmp > mk.tmp2
                awk '{gsub(/CROSS_COMPILE)gcc/,"CROSS_COMPILE)gcc -EL");print}' mk.tmp2 > mk.tmp3
                mv mk.tmp3 Makefile

           fi
      fi


     cd $GST_TOP_DIR/openssl-1.1.0i


 #      make clean;
       make;
       check_make_ret
       make install
       check_make_ret


      cd $GST_TOP_DIR
      mkdir -p $STATIC_OUTPUT_PATH/lib $SHARE_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libssl.a $OUTPUT_PATH/lib/libcrypto.a  $STATIC_OUTPUT_PATH/lib
      cp -arf $OUTPUT_PATH/lib/libssl.so $SHARE_OUTPUT_PATH/lib
      rm -f $SHARE_OUTPUT_PATH/lib/libssl.so.1.0.0
      cp -arf $OUTPUT_PATH/lib/libssl.so.1.1  $SHARE_OUTPUT_PATH/lib
      cp -arf $OUTPUT_PATH/lib/libcrypto.so $SHARE_OUTPUT_PATH/lib
      rm -f $SHARE_OUTPUT_PATH/lib/libcrypto.so.1.0.0
      cp -arf $OUTPUT_PATH/lib/libcrypto.so.1.1 $SHARE_OUTPUT_PATH/lib
      echo "finish compile openssl==================="
}

