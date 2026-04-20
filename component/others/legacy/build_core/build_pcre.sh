
#!/bin/bash
echo "start  pcre  in script"

function  build_pcre()
{

  echo "start to compile libpcre==================="

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
      if [ $rebuild_libpcre -eq 1 ];then

        rm -fr thirdparty/pcre-8.37
        mkdir -p thirdparty
        cd thirdparty
        tar zxvf $GST_TOP_DIR/tarball_bak/pcre-8.37.tar.gz
        cd -
        cd $GST_TOP_DIR/thirdparty/pcre-8.37

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
       ./configure --prefix=$GST_OUTPUT_PATH  --enable-unicode-properties  --enable-utf8  \
           --host=$CUR_HOST CFLAGS="$CUR_CFLAGS" --disable-silent-rules

      fi


     cd $GST_TOP_DIR/thirdparty/pcre-8.37

     make;
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR

     echo "finish compile libpcre==================="
}



