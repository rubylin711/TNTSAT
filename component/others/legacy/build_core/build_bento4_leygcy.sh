
#!/bin/bash
echo "start build_bento4 in script"

function  build_bento4()
{
    echo "start to compile bento4=================="


    if [ $host_arm -eq 1 ];then
       echo "this is arm"
       BENTO4_TARGET=arm-mont-linux
       cd ${SDK_DIR}
       git apply opensource/bento4/arm-gcc8.patch
       cd -
     elif [ $host_mips -eq 1 ];then
       echo "this is mips"
       BENTO4_TARGET=mipsel-mont-linux
     else
       echo "this is x86"
       BENTO4_TARGET=x86-unknown-linux
     fi


      cd $GST_TOP_DIR

      if [ $rebuild_bento4 -eq 1 ];then

        cd $GST_TOP_DIR/bento4
	#must keep space at tail
        sed -i "s|gcc_extra_options='|gcc_extra_options='${CFG_MT_OSS_BACKTRACE} ${CFG_MT_OSS_SSP} ${CFG_MT_OSS_SANITIZE} ${CFG_MT_OSS_BASE_CFLAGS} |g" Build/Targets/$BENTO4_TARGET/Config.scons
	LINKFLAGS_ORIG="LINKFLAGS = \['-fPIC'"
	LINKFLAGS_NEW=${LINKFLAGS_ORIG}
	for x in ${CFG_MT_OSS_SANITIZE_LD} ${CFG_MT_OSS_BASE_LDFLAGS}; do LINKFLAGS_NEW="${LINKFLAGS_NEW}, '${x}'"; done
	echo LINKFLAGS_ORIG=${LINKFLAGS_ORIG}
	echo LINKFLAGS_NEW=${LINKFLAGS_NEW}
        sed -i "s|${LINKFLAGS_ORIG}|${LINKFLAGS_NEW}|g" Build/Targets/$BENTO4_TARGET/Config.scons
        scons -u target=$BENTO4_TARGET build_config=Release

      fi


     cd $GST_TOP_DIR/bento4
     mkdir -p $OUTPUT_PATH/lib $STATIC_OUTPUT_PATH/lib
     cp -arf Build/Targets/$BENTO4_TARGET/Release/libBento4.a $OUTPUT_PATH/lib
     cp -arf Build/Targets/$BENTO4_TARGET/Release/libBento4.a $STATIC_OUTPUT_PATH/lib

     cd $GST_TOP_DIR/bento4/Source
     mkdir -p $OUTPUT_PATH/include/bento4/inc
     cp -arf --parents `find -name "*.h"` $OUTPUT_PATH/include/bento4/inc
     cd $OUTPUT_PATH/include/bento4/inc
     #cp -af C++/Codecs/Ap4AdtsParser.h C++/Codecs/Ap4AvcParser.h C++/Codecs/Ap4BitStream.h C++/Codecs/Ap4HevcParser.h C++/MetaData/Ap4MetaData.h C++/Codecs/Ap4Mp4AudioInfo.h C++/Codecs/Ap4NalParser.h $OUTPUT_PATH/include/bento4/inc/C++/Core
     cd -

     echo "finish compile libbento4==================="

     cd $GST_TOP_DIR/bento4
     git checkout .

     cd $GST_TOP_DIR
}


