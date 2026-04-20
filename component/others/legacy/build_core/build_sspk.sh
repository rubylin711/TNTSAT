
#!/bin/bash
echo "start build_sspk in script"

function  build_sspk()
{
    echo "start to compile sspk==================="


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

	echo "CUR_HOST is $CUR_HOST"
	echo "CUR_CFLAGS is $CUR_CFLAGS"
	echo $GST_OUTPUT_PATH
	echo $CUR_HOST
	echo $CC
	echo $LD
	echo $CXX
	echo $ARCH
	echo $AR
	echo $RANLIB
	echo $PATH
	echo $STRIP
	export SSPK_TOP_DIR=$GST_TOP_DIR/sspk
	export SSPK_OUTPUT_PATH=$GST_OUTPUT_PATH

     cd $GST_TOP_DIR/sspk

     ./Toolchain/bin/bz
	 
     check_make_ret
     mkdir -p $STATIC_OUTPUT_PATH/sspk
     cp -far $OUTPUT_PATH/sspk/lib $STATIC_OUTPUT_PATH/sspk/

     cd $GST_TOP_DIR

     echo "finish compile sspk==================="


}


