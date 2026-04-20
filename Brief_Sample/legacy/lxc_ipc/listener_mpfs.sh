#!/bin/sh

echo -e "\033[31m run listener_mpfs \033[0m"
export PATH=$PATH:/usr/local/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib
export MT_SANITIZE="false"
export MT_SANITIZE_TAG="false"
export MT_SANITIZE_TSAN="false"
export MT_JEMALLOC="false"
export MT_GPERFTOOLS_HEAP_PROF="false"

LXCENV="`ls -l /proc/1/exe | grep "lxc\-execute"`"
if [ -n "${LXCENV}" ]; then
	echo -e "\033[31m now I'm in lxc environment \033[0m"
	export PATH=$PATH:/usr/local/bin
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/lib:/lib
	if [ "${MT_SANITIZE}" = "true" ]; then
		export ASAN_OPTIONS=detect_stack_use_after_return=1:quarantine_size_mb=64:thread_local_quarantine_size_kb=64
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
	/usr/local/bin/usb-storage-daemon.sh
fi

/usr/local/bin/listener_mpfs
