#!/bin/sh
echo "this is do help script"

 
function  mont_help() 
{ 
 
  echo "****support the following param****" 
  echo "./build.sh all" 
 
  echo "********step1 ssl***********" 
  echo "./build.sh ssl" 
  echo "./build.sh ssl_clean" 
 
  echo "********step2 sqlite***********" 
  echo "./build.sh sqlite" 
  echo "./build.sh sqlite_clean" 
 
  echo "********step3 ffi***********" 
  echo "./build.sh ffi" 
  echo "./build.sh ffi_clean" 
 
  echo "********step4 xml2***********" 
  echo "./build.sh xml2" 
  echo "./build.sh xml2_clean" 
  
  echo "********step5 zlib***********" 
  echo "./build.sh zlib" 
  echo "./build.sh zlib_clean" 
 
  echo "********step6 glib***********" 
  echo "./build.sh glib" 
  echo "./build.sh glib_clean" 

  echo "********step7 neon***********" 
  echo "./build.sh neon" 
  echo "./build.sh neon_clean" 
 
  echo "********step8 gst***********" 
  echo "./build.sh gst" 
  echo "./build.sh gst_clean" 
 
  echo "********step9 base***********" 
  echo "./build.sh base" 
  echo "./build.sh base_clean" 

  echo "********step10 good***********"
  echo "./build.sh good"
  echo "./build.sh good_clean"

  echo "********step11 bad***********"
  echo "./build.sh bad"
  echo "./build.sh bad_clean"


  echo "********step12 av***********"
  echo "./build.sh av"
  echo "./build.sh av_clean"
 
  echo "********step13 av***********"
  echo "./build.sh mont"
  echo "./build.sh mont_clean"
 
  echo "********step14 good***********"
  echo "./build.sh release"


}

