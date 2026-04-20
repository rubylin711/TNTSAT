
#!/bin/bash
echo "start build_gmp in script"

function  build_neon()
{
    echo "start to compile neon==================="

      cd $GST_TOP_DIR

      if [ $rebuild_libneon -eq 1 ];then

        cd $GST_TOP_DIR/neon-0.30.2

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
        export PKG_CONFIG_PATH=$GST_OUTPUT_PATH/lib/pkgconfig
        export PKG_CONFIG_LIBDIR=$PKG_CONFIG_PATH
        export LD_LIBRARY_PATH=$GST_OUTPUT_PATH/lib
        CUR_CFLAGS+="  -I$GST_OUTPUT_PATH/include"
        echo "PKG_CONFIG_PATH is $PKG_CONFIG_PATH"
        echo "PKG_CONFIG_LIBDIR is $PKG_CONFIG_LIBDIR"
        ./configure --prefix=$GST_OUTPUT_PATH --enable-shared=yes --enable-static=yes --host=$CUR_HOST  \
         CFLAGS="$CUR_CFLAGS" --with-ssl=openssl  --disable-webdav

        touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

     fi


     cd $GST_TOP_DIR/neon-0.30.2

     make;
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR
     mkdir -p $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libneon.a  $STATIC_OUTPUT_PATH/lib
     echo "finish compile neon==================="

}




function  build_soup()
{
    echo "start to compile soup==================="

      cd $GST_TOP_DIR
      if [ $rebuild_libsoup -eq 1 ];then

        CUR_CFLAGS+="  -I$GST_OUTPUT_PATH/include/glib-2.0   -I$GST_OUTPUT_PATH/lib/glib-2.0/include"
        #rm -fr thirdparty/soup-2.48.1
        #mkdir -p thirdparty
        #cd thirdparty
        #tar xvf $GST_TOP_DIR/tarball_bak/libsoup-2.48.1.tar.xz
        #cd -
        #cd $GST_TOP_DIR/thirdparty/libsoup-2.48.1
        cd $GST_TOP_DIR/libsoup-2.48.1

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
        ./configure --prefix=$GST_OUTPUT_PATH --disable-silent-rules  --host=$CUR_HOST --disable-tls-check \
           CFLAGS="$CUR_CFLAGS" --disable-glibtest  XML_CFLAGS="-I$GST_OUTPUT_PATH/include/libxml2" \
            XML_LIBS="-L$GST_OUTPUT_PATH/lib/ -lxml2"  \
              SQLITE_CFLAGS="-I$GST_OUTPUT_PATH/include" \
                 SQLITE_LIBS="-L$GST_OUTPUT_PATH/lib/ -lsqlite3"

        touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

     fi


     #cd $GST_TOP_DIR/thirdparty/libsoup-2.48.1
     cd $GST_TOP_DIR/libsoup-2.48.1

     make;
     check_make_ret
     make install
     check_make_ret


     cd $GST_TOP_DIR

     echo "finish compile libsoup==================="

}



function  build_gmp()
{
    echo "start to compile gmp==================="


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
      if [ $rebuild_libgmp -eq 1 ];then

        #rm -fr thirdparty/gmp-6.1.2
        #mkdir -p thirdparty
        #cd thirdparty
        #tar xvf $GST_TOP_DIR/tarball_bak/gmp-6.1.2.tar.xz
        #cd -
        #cd $GST_TOP_DIR/thirdparty/gmp-6.1.2
        cd $GST_TOP_DIR/gmp-6.1.2

        if [ $host_arm -eq 1 ];then
            echo "host is arm"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
        elif [ $host_aarch64 -eq 1 ];then
            echo "host is aarch64"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
        elif [ $host_mips -eq 1 ];then
            echo "host is mips"
        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
       fi

       ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST  --disable-silent-rules\
            CFLAGS="$CUR_CFLAGS"  --with-lib-path="$GST_OUTPUT_PATH/lib" \
            --with-include-path="$GST_OUTPUT_PATH/include"
       touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi


     #cd $GST_TOP_DIR/thirdparty/gmp-6.1.2
     cd $GST_TOP_DIR/gmp-6.1.2

     make;
     check_make_ret
     make install
     check_make_ret



     cd $GST_TOP_DIR

     echo "finish compile libgmp==================="

}



