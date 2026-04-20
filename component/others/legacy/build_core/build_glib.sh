
#!/bin/bash
echo "start build_glib in script"

#####################################################################3
#
#
#
#
#
#######################################################################3


function  build_glib()
{
    echo "start to compile glib-2.48==================="


      cd $GST_TOP_DIR
      if [ $rebuild_libglib -eq 1 ];then

       # rm -fr thirdparty/glib-2.48.0
       # tar xvf tarball_bak/glib-2.48.0.tar.xz  -C thirdparty/
        cd $GST_TOP_DIR/glib-2.48.0
        
        if [ $host_arm -eq 1 ];then
      
            echo "host is arm"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
            cp -fr $GST_TOP_DIR/tarball_bak/arm-linux-glib.cache   .
            ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --without-pcre  --enable-static  --disable-silent-rules \
            CFLAGS="$CUR_CFLAGS -Wno-format-overflow" ZLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lz" \
            LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib" \
            ZLIB_CFLAGS="-I$GST_OUTPUT_PATH/include" \
            LIBFFI_CFLAGS="-I$GST_OUTPUT_PATH/include/libffi-3.2.1/include"  \
            LIBFFI_LIBS="-L$GST_OUTPUT_PATH/lib -lffi"  \
            --cache-file=arm-linux-glib.cache
            touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

        elif [ $host_aarch64 -eq 1 ];then
      
            echo "host is aarch64"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
            cp -fr $GST_TOP_DIR/tarball_bak/aarch64-linux-glib.cache   .
            ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --without-pcre  --enable-static  --disable-silent-rules \
            CFLAGS="$CUR_CFLAGS -Wno-format-overflow" ZLIB_LIBS="-L$GST_OUTPUT_PATH/lib -L$GST_OUTPUT_PATH/lib64 -lz" \
            LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib -Wl,-rpath-link=$GST_OUTPUT_PATH/lib64" \
            ZLIB_CFLAGS="-I$GST_OUTPUT_PATH/include" \
            LIBFFI_CFLAGS="-I$GST_OUTPUT_PATH/include/libffi-3.2.1/include"  \
            LIBFFI_LIBS="-L$GST_OUTPUT_PATH/lib -L$GST_OUTPUT_PATH/lib64 -lffi"  \
            --cache-file=aarch64-linux-glib.cache
            touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

        elif [ $host_mips -eq 1 ];then
            echo "host is mips"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
            cp -fr $GST_TOP_DIR/tarball_bak/mips-linux-glib.cache   .
            ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  --without-pcre --enable-static  --disable-silent-rules \
            CFLAGS="$CUR_CFLAGS -Wno-format-overflow" ZLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lz" \
            LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib" \
            ZLIB_CFLAGS="-I$GST_OUTPUT_PATH/include" \
            LIBFFI_CFLAGS="-I$GST_OUTPUT_PATH/include/libffi-3.2.1/include"  \
            LIBFFI_LIBS="-L$GST_OUTPUT_PATH/lib -lffi"  \
            --cache-file=mips-linux-glib.cache
            touch configure.ac aclocal.m4 configure Makefile.am Makefile.in


        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
            ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  --without-pcre  --enable-static --disable-silent-rules \
            CFLAGS="$CUR_CFLAGS -Wno-format-overflow" ZLIB_LIBS="-L$GST_OUTPUT_PATH/lib -lz" \
            LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib" \
            ZLIB_CFLAGS="-I$GST_OUTPUT_PATH/include" \
            LIBFFI_CFLAGS="-I$GST_OUTPUT_PATH/lib/libffi-3.2.1/include"  \
            LIBFFI_LIBS="-L$GST_OUTPUT_PATH/lib -lffi"  \

            touch configure.ac aclocal.m4 configure Makefile.am Makefile.in
        fi

      fi


     cd $GST_TOP_DIR/glib-2.48.0

     make;
     check_make_ret
     make install
     check_make_ret


     if [ $host_arm -eq 1 ];then
       rm -fr $GST_OUTPUT_PATH/bin/glib-genmarshal
       cp -fr  $GST_TOP_DIR/tmp_x86_bin/glib-genmarshal  $GST_OUTPUT_PATH/bin/
     elif [ $host_aarch64 -eq 1 ];then
       rm -fr $GST_OUTPUT_PATH/bin/glib-genmarshal
       cp -fr  $GST_TOP_DIR/tmp_x86_bin/glib-genmarshal  $GST_OUTPUT_PATH/bin/
     elif [ $host_mips -eq 1 ];then
       rm -fr $GST_OUTPUT_PATH/bin/glib-genmarshal
       cp -fr  $GST_TOP_DIR/tmp_x86_bin/glib-genmarshal  $GST_OUTPUT_PATH/bin/
     fi

     cd $GST_TOP_DIR

    mkdir -p $STATIC_OUTPUT_PATH/lib
    cp -fr $OUTPUT_PATH/lib/libglib-2.0.a  $STATIC_OUTPUT_PATH/lib
    cp -fr $OUTPUT_PATH/lib/libgmodule-2.0.a  $STATIC_OUTPUT_PATH/lib
    cp -fr $OUTPUT_PATH/lib/libgobject-2.0.a    $STATIC_OUTPUT_PATH/lib
    cp -fr $OUTPUT_PATH/lib/libgio-2.0.a        $STATIC_OUTPUT_PATH/lib
    cp -fr $OUTPUT_PATH/lib/glib-2.0        $STATIC_OUTPUT_PATH/lib
    mkdir -p $SHARE_OUTPUT_PATH/lib
    cp -af $OUTPUT_PATH/lib/libglib-2.0.so  $SHARE_OUTPUT_PATH/lib
    cp -af $OUTPUT_PATH/lib/libglib-2.0.so.0  $SHARE_OUTPUT_PATH/lib
    cp -af $OUTPUT_PATH/lib/libglib-2.0.so.0.4800.0  $SHARE_OUTPUT_PATH/lib

     echo "finish compile libglib==================="

}


