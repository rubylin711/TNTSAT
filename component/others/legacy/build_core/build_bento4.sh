
#!/bin/bash
echo "start build_bento4 in script"

function  build_bento4()
{
    echo "start to compile bento4=================="

      cd $GST_TOP_DIR

      if [ $rebuild_bento4 -eq 1 ];then

        cd $GST_TOP_DIR/bento4
        make clean && make

      fi


     cd $GST_TOP_DIR/bento4
     make
     STATIC_LIB_DIR=$GST_TOP_DIR/../pub/static_lib
     SHARED_LIB_DIR=$GST_TOP_DIR/../pub/shared_lib
     SHARED_STRIP_LIB_DIR=$GST_TOP_DIR/../pub/shared_lib_striped
     #echo $STATIC_LIB_DIR
     rm -f  ${SHARED_LIB_DIR}/libBento4.so
     rm -f  ${SHARED_STRIP_LIB_DIR}/libBento4.so
     echo "cp libBento4.a ==================="
     mkdir -p $OUTPUT_PATH/lib $STATIC_OUTPUT_PATH/lib
     cp -arf ${STATIC_LIB_DIR}/libBento4.a $OUTPUT_PATH/lib
     cp -arf ${STATIC_LIB_DIR}/libBento4.a $STATIC_OUTPUT_PATH/lib

		 
     echo "finish compile libbento4==================="

     cd $GST_TOP_DIR/bento4
     git checkout .

     cd $GST_TOP_DIR
}


