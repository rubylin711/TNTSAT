#start.sh
export PATH=$PATH:/usr/local/stb
export MT_SANITIZE="false"
export MT_SANITIZE_TAG="false"
export MT_SANITIZE_TSAN="false"
export MT_JEMALLOC="false"
export MT_GPERFTOOLS_HEAP_PROF="false"
export ulimit_size="512"

# KERN_WARNING: 4
echo 4 > /proc/sys/kernel/printk

LXCENV="`ls -l /proc/1/exe | grep "lxc\-execute"`"
if [ -n "${LXCENV}" ]; then
	echo -e "\033[31m now I'm in lxc environment \033[0m"
	export PATH=$PATH:/usr/local/bin
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/lib:/lib
	if [ "${MT_SANITIZE}" = "true" ]; then
		export ASAN_OPTIONS=detect_stack_use_after_return=1:halt_on_error=1:quarantine_size_mb=64:thread_local_quarantine_size_kb=64
	elif [ "${MT_SANITIZE_TAG}" = "true" ]; then
		export HWASAN_OPTIONS=halt_on_error=1
	elif [ "${MT_SANITIZE_TSAN}" = "true" ]; then
		export TSAN_OPTIONS="halt_on_error=1"
	elif [ "${MT_JEMALLOC}" = "true" ]; then
		export LD_PRELOAD=/lib/libjemalloc.so
		#export MALLOC_CONF="dirty_decay_ms:0,muzzy_decay_ms:0"
	elif [ "${MT_GPERFTOOLS_HEAP_PROF}" = "true" ]; then
		export LD_PRELOAD=/lib/libtcmalloc.so
		export TCMALLOC_RELEASE_RATE="1000"
		#env see gperftools-2.15/docs/heapprofile.html
		export HEAP_PROFILE_ALLOCATION_INTERVAL="10485760"
		#export HEAP_PROFILE_INUSE_INTERVAL="104857600"
		#export HEAP_PROFILE_TIME_INTERVAL="100"
		#export HEAP_PROFILE_MMAP=true
		#export HEAP_PROFILE_ONLY_MMAP=true
		#export HEAP_PROFILE_MMAP_LOG=true
	fi
fi
#config gstreamer env
if [ -f /usr/local/libexec/gstreamer-1.0/gst-plugin-scanner ]; then
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/local/lib/gstreamer:/usr/local/lib/gstreamer/gstreamer-1.0
	export GST_PLUGIN_PATH=/usr/local/lib/gstreamer
	export GIO_MODULE_DIR=/usr/local/lib/gio/modules 
	export GST_PLUGIN_SCANNER=/usr/local/libexec/gstreamer-1.0/gst-plugin-scanner
	export MT_FONTCONFIG_FILE=/usr/local/stb/Sans.ttf
	export MT_OSD_DISPLAY_POSITION=x:400_y:500
fi

#/bin/echo "ondemand" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
#/bin/echo "userspace" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

#chmod +x /usr/bin/optee_example_global
if [ -f /usr/local/stb/avfw/avfw.bin ]; then
	optee_example_global -l 1 -f /usr/local/stb/avfw/avfw.bin
	echo "audio firmware is running..."
else
	echo "Error: avfw not found!"
fi

cd /usr/local/stb

#fixme
./thttpd -d /usr/local/stb/ -p 80 -c \* -u root -nor

#echo "run audio_ta_service ."
#./audio_ta_service &

ulimit -s ${ulimit_size}
#echo "run bluetooth daemon"
#source ./bluetooth.sh rtl88x2cs 61
#echo "run mt_sample...."
#HEAPPROFILE="/media/sda1/mt_sample" ./mt_sample 2>&1

