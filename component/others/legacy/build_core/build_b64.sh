#!/bin/bash
#

#########################################################3
#  
#  build_curl
#
#############################################################
function  build_b64()
{

      echo "start to compile b64==================="

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



      if [ $rebuild_b64 -eq 1 ];then

           # rm -fr thirdparty/opensssl-1.0.1l/
           # tar zxvf tarball_bak/openssl-1.0.1l.tar.gz  -C thirdparty/
            cd $GST_TOP_DIR/libb64-1.2
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"


            if [ $host_arm -eq 1 ];then
              echo "host is arm"
              echo $GST_OUTPUT_PATH
              

            elif [ $host_aarch64 -eq 1 ];then
              echo "host is aarch64"
              echo $GST_OUTPUT_PATH


            elif [ $host_x86 -eq 1 ];then
              echo "host is x86"
              echo $GST_OUTPUT_PATH
              rm -fr config
              cp -fr  config.x86 config
              chmod +x config
              
              mv mk.tmp Makefile


           elif [ $host_mips -eq 1 ];then
              echo "host is mips in build_libb64-1.2"
              echo $GST_OUTPUT_PATH
			  echo $CUR_HOST
              


           fi
      fi


     cd $GST_TOP_DIR/libb64-1.2


       make clean;
       make;
       check_make_ret


      cd $GST_TOP_DIR
	  cp -fr $GST_TOP_DIR/libb64-1.2/include/b64 -fr $GST_OUTPUT_PATH/include/
	  cp -fr $GST_TOP_DIR/libb64-1.2/src/libb64.a  $GST_OUTPUT_PATH/lib/libb64.a
          mkdir -p $STATIC_OUTPUT_PATH/lib
	  cp -fr $GST_OUTPUT_PATH/lib/libb64.a ${STATIC_OUTPUT_PATH}/lib
      echo "finish compile libb64-1.2==================="
}

