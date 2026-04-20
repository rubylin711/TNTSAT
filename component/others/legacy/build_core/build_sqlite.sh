
#!/bin/bash
echo "start build_sqlite in script"

#####################################################################3
#
#
#
#
#
#######################################################################3

function  build_sqlite()
{
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



      if [ $rebuild_libsqlite -eq 1 ];then

        rm -fr ./sqlite-snapshot-201706291727/
        tar zxvf tarball_bak/sqlite-snapshot-201706291727.tar.gz  -C ./

        cd $GST_TOP_DIR/sqlite-snapshot-201706291727/

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
        ./configure --prefix=$GST_OUTPUT_PATH --host=$CUR_HOST  CFLAGS="$CUR_CFLAGS" --disable-silent-rules LDFLAGS="-ldl"



      fi

     cd $GST_TOP_DIR/sqlite-snapshot-201706291727/

      make;
      check_make_ret
      make install
      check_make_ret

      cd $GST_TOP_DIR

      mkdir -p $STATIC_OUTPUT_PATH/lib
      cp -fr $OUTPUT_PATH/lib/libsqlite3.a  $STATIC_OUTPUT_PATH/lib
      echo "finish compile sqlite==================="
}

