
#!/bin/bash
echo "start build_tasn1 in script"


#######################################################################3



function  build_tasn1()
{
    echo "start to compile tasn1==================="


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
      if [ $rebuild_libtasn1 -eq 1 ];then

        rm -fr thirdparty/libtasn1-4.8
        mkdir -p thirdparty
        cd thirdparty
        tar xvf $GST_TOP_DIR/tarball_bak/libtasn1-4.8.tar.gz
        cd -
        cd $GST_TOP_DIR/thirdparty/libtasn1-4.8

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

        ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST CFLAGS="$CUR_CFLAGS" --disable-doc --disable-silent-rules
        touch configure.ac aclocal.m4 configure Makefile.am Makefile.in
      fi


     cd $GST_TOP_DIR/thirdparty/libtasn1-4.8

     make;
     check_make_ret
     make install
     check_make_ret



     cd $GST_TOP_DIR

     echo "finish compile libtasn1==================="

}







#####################################################################3
#
#
#
#
#
#######################################################################3





function  build_nettle()
{
    echo "start to compile nettle==================="

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
      if [ $rebuild_libnettle -eq 1 ];then

        rm -fr thirdparty/nettle-3.2
        mkdir -p thirdparty
        cd thirdparty
        tar xvf $GST_TOP_DIR/tarball_bak/nettle-3.2.tar.gz
        cd -
        cd $GST_TOP_DIR/thirdparty/nettle-3.2

        if [ $host_arm -eq 1 ];then
            echo "host is arm"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
           # cp -fr $GST_TOP_DIR/tarball_bak/arm-linux-glib.cache   .
        elif [ $host_aarch64 -eq 1 ];then
            echo "host is aarch64"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
           # cp -fr $GST_TOP_DIR/tarball_bak/aarch64-linux-glib.cache   .
        elif [ $host_mips -eq 1 ];then
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
            echo "host is mips"
        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
        fi

        ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST CFLAGS="$CUR_CFLAGS"  --disable-silent-rules \
             -with-lib-path=$GST_OUTPUT_PATH/lib --with-include-path=$GST_OUTPUT_PATH/include

         touch configure.ac aclocal.m4 configure Makefile.am Makefile.in

      fi


     cd $GST_TOP_DIR/thirdparty/nettle-3.2

     make;
     check_make_ret
     make install
     check_make_ret



     cd $GST_TOP_DIR

     echo "finish compile libnettle==================="

}





#####################################################################3
#
#
#
#
#
#######################################################################3





function  build_gnutls()
{
    echo "start to compile gnutls==================="


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
      if [ $rebuild_libgnutls -eq 1 ];then

        rm -fr thirdparty/gnutls-3.5.0
        mkdir -p thirdparty
        cd thirdparty
        tar xvf $GST_TOP_DIR/tarball_bak/gnutls-3.5.0.tar.xz
        cd -
        cd $GST_TOP_DIR/thirdparty/gnutls-3.5.0

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
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
       fi

        ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST CFLAGS="$CUR_CFLAGS"  --disable-silent-rules \
             LDFLAGS="-Wl,-rpath-link=$GST_OUTPUT_PATH/lib" \
             --disable-libdane --disable-non-suiteb-curves --disable-tests \
             --disable-ocsp    \
             --disable-srp-authentication --disable-heartbeat-support --disable-alpn-support \
             --disable-dtls-srtp-support --disable-largefile  --without-idn \
             GMP_LIBS="-L$GST_OUTPUT_PATH/lib" GMP_CFLAGS="-I$GST_OUTPUT_PATH/include" --without-p11-kit --disable-doc

         touch configure.ac aclocal.m4 configure Makefile.am Makefile.in
      fi


     cd $GST_TOP_DIR/thirdparty/gnutls-3.5.0

     make;
     check_make_ret
     make install
     check_make_ret



     cd $GST_TOP_DIR

     echo "finish compile gnutls==================="

}






#####################################################################3
#
#
#
#
#
#######################################################################3



function  build_glib_networking()
{
    echo "start to compile glib_networking==================="

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
      if [ $rebuild_glibnetworking -eq 1 ];then

        rm -fr thirdparty/glib-networking-2.36.2
        mkdir -p thirdparty
        cd thirdparty
        tar xvf $GST_TOP_DIR/tarball_bak/glib-networking-2.36.2.tar.xz
        cd -
        cd $GST_TOP_DIR/thirdparty/glib-networking-2.36.2


        if [ $host_arm -eq 1 ];then
            echo "host is arm"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
           # cp -fr $GST_TOP_DIR/tarball_bak/arm-linux-glib.cache   .
        elif [ $host_aarch64 -eq 1 ];then
            echo "host is aarch64"
            echo "CUR_HOST is $CUR_HOST"
            echo "CUR_CFLAGS is $CUR_CFLAGS"
           # cp -fr $GST_TOP_DIR/tarball_bak/aarch64-linux-glib.cache   .
        elif [ $host_mips -eq 1 ];then
            echo "host is mips"
        elif [ $host_x86 -eq 1 ];then
            echo "host is x86"
           #  ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --disable-glibtest --disable-silent-rules
        fi

        CUR_CFLAGS+=" -I$GST_OUTPUT_PATH/include/glib-2.0   \
               -I$GST_OUTPUT_PATH/lib/glib-2.0/include   \
               -I$GST_OUTPUT_PATH/include/"


        ./configure --prefix=$GST_OUTPUT_PATH  --host=$CUR_HOST --disable-silent-rules --with-pkcs11=no  --disable-glibtest CFLAGS="$CUR_CFLAGS"

         touch configure.ac aclocal.m4 configure Makefile.am Makefile.in
      fi


     cd $GST_TOP_DIR/thirdparty/glib-networking-2.36.2

     make;
     check_make_ret
     make install
     check_make_ret



     cd $GST_TOP_DIR

     echo "finish compile glib_networking==================="

}

