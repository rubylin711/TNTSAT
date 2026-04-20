
#!/bin/bash
echo "start build_zlib in script"

function  build_zlib()
{
    echo "start to compile zlib-1.2.11==================="


    if [ $host_arm -eq 1 ];then
       echo "this is arm"
    elif [ $host_aarch64 -eq 1 ];then
       echo "this is aarch64"
     elif [ $host_mips -eq 1 ];then
       echo "this is mips"
     else
       echo "this is x86"
     fi


      cd $GST_TOP_DIR

      if [ $rebuild_libz -eq 1 ];then

        tar xvf tarball_bak/zlib-1.2.11.tar.xz
        cd $GST_TOP_DIR/zlib-1.2.11

        if [ $host_arm -eq 1 ];then

           echo "host is arm"
           ./configure --prefix=$GST_OUTPUT_PATH

        elif [ $host_aarch64 -eq 1 ];then

           echo "host is aarch64"
           ./configure --prefix=$GST_OUTPUT_PATH

        elif [ $host_mips -eq 1 ];then

           echo "host is mips"
           ./configure --prefix=$GST_OUTPUT_PATH
           if [ $is_uclibc -eq 0 ];then

             if [ $is_debug -eq 0 ];then
               awk '{gsub(/-O3/,"-O3 -msoft-float -fPIC ");print}' Makefile > mk.tmp
             else
               awk '{gsub(/-O3/,"-O3 -msoft-float  -g  -fPIC ");print}' Makefile > mk.tmp
             fi
           else
             awk '{gsub(/-O3/,"-O3 -msoft-float -muclibc");print}' Makefile > mk.tmp

           fi
           mv mk.tmp Makefile


        elif [ $host_x86 -eq 1 ];then
           echo "host is x86"
          ./configure --prefix=$GST_OUTPUT_PATH

       fi

         echo "CUR_HOST is $CUR_HOST"
         echo "CUR_CFLAGS is $CUR_CFLAGS"


      fi


     cd $GST_TOP_DIR/zlib-1.2.11

     make;
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR

     echo "finish compile libzlib==================="
     mkdir -p $STATIC_OUTPUT_PATH/lib
     cp -fr $OUTPUT_PATH/lib/libz.a  $STATIC_OUTPUT_PATH/lib


}


