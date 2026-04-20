
#!/bin/bash
echo "start xml2 in script"



function  build_xml2()
{

  echo "start to compile libxml2==================="

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
      if [ $rebuild_libxml2 -eq 1 ];then

        rm -fr libxm2-2.9.2
        tar zxvf tarball_bak/libxml2-2.9.2.tar.gz
        cd $GST_TOP_DIR/libxml2-2.9.2

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
        ./configure --prefix=$GST_OUTPUT_PATH  --with-python=no --host=$CUR_HOST CFLAGS="$CUR_CFLAGS"  --disable-silent-rules --without-zlib

      fi


     cd $GST_TOP_DIR/libxml2-2.9.2

     make;
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR
     mkdir -p $STATIC_OUTPUT_PATH/lib
     cp -fr $OUTPUT_PATH/lib/libxml2.a   $STATIC_OUTPUT_PATH/lib
     echo "finish compile libxml2==================="

}


