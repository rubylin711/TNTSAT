
#!/bin/bash
echo "start build_ffi in script"


function  build_ffi()
{

      echo "start to compile libffi==================="

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
      if [ $rebuild_libffi -eq 1 ];then

        rm -fr libffi-3.2.1
        #tar zxvf tarball_bak/libffi-3.2.1.tar.gz  -C ./
        tar xf tarball_bak/libffi-3.2.1-soft-float.tar.bz2    -C ./
        cd $GST_TOP_DIR/libffi-3.2.1

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
        ./configure --prefix=$GST_OUTPUT_PATH --host=$CUR_HOST CFLAGS="$CUR_CFLAGS" --disable-silent-rules --enable-static

      fi


     cd $GST_TOP_DIR/libffi-3.2.1

     make;
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR
     cp -fr $GST_OUTPUT_PATH/lib/libffi-3.2.1   $GST_OUTPUT_PATH/include
     mkdir -p $STATIC_OUTPUT_PATH/lib
     if [ $host_aarch64 -eq 1 ]; then
       cp -fr $OUTPUT_PATH/lib64/libffi.a  $STATIC_OUTPUT_PATH/lib
     else
       cp -fr $OUTPUT_PATH/lib/libffi.a  $STATIC_OUTPUT_PATH/lib
     fi
     echo "finish compile libffi==================="
}


