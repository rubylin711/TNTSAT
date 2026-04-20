#!/bin/bash

if [ "${mt_chip}" = "symphony1" ]; then
	if [ "${size}" = "128" ]; then
		mfrs_cfg=symphony1_128.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony1.cfg
	else
		mfrs_cfg=symphony1_512.cfg
	fi
elif [ "${mt_chip}" = "symphony2" ]; then
	if [ "${size}" = "128" ]; then
		mfrs_cfg=symphony2_128.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony2.cfg
	else
		mfrs_cfg=symphony2_512.cfg
	fi
elif [ "${mt_chip}" = "symphony4" ]; then
	if [ "${size}" = "128" ]; then
		mfrs_cfg=symphony4_128.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony4.cfg
	elif [ "${size}" = "512" ]; then
		mfrs_cfg=symphony4_512.cfg
	elif [ "${size}" = "512-tee" ]; then
		mfrs_cfg=symphony4_512_tee.cfg
	fi
elif [ "${mt_chip}" = "symphony4-tee" ]; then
	if [ "${size}" = "512" ]; then
		mfrs_cfg=symphony4_512_tee.cfg
	elif [ "${size}" = "256" ]; then
		mfrs_cfg=symphony4_tee.cfg
	fi
elif [ "${mt_chip}" = "symphony6" ]; then
	if [ "${size}" = "256" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "512" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "1024" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "2048" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "4096" ]; then
		mfrs_cfg=symphony6.cfg
	elif [ "${size}" = "256_gst" ]; then
		mfrs_cfg=symphony6_gst.cfg
	elif [ "${size}" = "512_gst" ]; then
		mfrs_cfg=symphony6_gst.cfg
	elif [ "${size}" = "1024_gst" ]; then
		mfrs_cfg=symphony6_gst.cfg
	elif [ "${size}" = "2048_gst" ]; then
		mfrs_cfg=symphony6_gst.cfg
	elif [ "${size}" = "4096_gst" ]; then
		mfrs_cfg=symphony6_gst.cfg
	elif [ "${size}" = "512_otpmini" ]; then
		mfrs_cfg=symphony6_otpmini.cfg
	elif [ "${size}" = "1024_otpmini" ]; then
		mfrs_cfg=symphony6_otpmini.cfg
	elif [ "${size}" = "2048_otpmini" ]; then
		mfrs_cfg=symphony6_otpmini.cfg
	elif [ "${size}" = "4096_otpmini" ]; then
		mfrs_cfg=symphony6_otpmini.cfg
	fi
elif [ "${mt_chip}" = "symphony6-tee" ]; then
	if [ "${size}" = "256" ]; then
		mfrs_cfg=symphony6_tee.cfg
	elif [ "${size}" = "512" ]; then
		mfrs_cfg=symphony6_tee.cfg
	elif [ "${size}" = "1024" ]; then
		mfrs_cfg=symphony6_tee.cfg
	elif [ "${size}" = "2048" ]; then
		mfrs_cfg=symphony6_tee.cfg
	elif [ "${size}" = "4096" ]; then
		mfrs_cfg=symphony6_tee.cfg
	elif [ "${size}" = "256_nor" ]; then
		mfrs_cfg=symphony6_tee_nor.cfg
	elif [ "${size}" = "512_nor" ]; then
		mfrs_cfg=symphony6_tee_nor.cfg
	elif [ "${size}" = "1024_nor" ]; then
		mfrs_cfg=symphony6_tee_nor.cfg
	elif [ "${size}" = "2048_nor" ]; then
		mfrs_cfg=symphony6_tee_nor.cfg
	elif [ "${size}" = "4096_nor" ]; then
		mfrs_cfg=symphony6_tee_nor.cfg
	elif [ "${size}" = "256_gst" ]; then
		mfrs_cfg=symphony6_tee_gst.cfg
	elif [ "${size}" = "512_gst" ]; then
		mfrs_cfg=symphony6_tee_gst.cfg
	elif [ "${size}" = "1024_gst" ]; then
		mfrs_cfg=symphony6_tee_gst.cfg
	elif [ "${size}" = "2048_gst" ]; then
		mfrs_cfg=symphony6_tee_gst.cfg
	elif [ "${size}" = "4096_gst" ]; then
		mfrs_cfg=symphony6_tee_gst.cfg
	fi
fi

source envsetup ${mfrs_cfg} ccache

make uninstall
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make uninstall failed!"
	exit 1
fi

make uninstall DOING_MT_SDK_RELEASE=1
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make uninstall for sdk release failed!"
	exit 1
fi

make clean
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make clean failed!"
	exit 1
fi

make -C loader uninstall
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader uninstall failed!"
	exit 1
fi

make -C loader uninstall DOING_MT_SDK_RELEASE=1
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader uninstall for sdk release failed!"
	exit 1
fi

make -C loader clean
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader clean failed!"
	exit 1
fi

make
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make failed!"
	exit 1
fi

make install DOING_MT_SDK_RELEASE=1
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make install DOING_MT_SDK_RELEASE=1 failed!"
	exit 1
fi

make -C loader
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader failed!"
	exit 1
fi

make -C loader install DOING_MT_SDK_RELEASE=1
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader install DOING_MT_SDK_RELEASE=1 failed!"
	exit 1
fi

cp -arf image image-${mt_chip}-${size}
cp -arf pub pub-${mt_chip}-${size}

make clean
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make clean failed!"
	exit 1
fi

make -C loader clean
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader clean failed!"
	exit 1
fi




make uninstall
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make uninstall failed!"
	exit 1
fi

make uninstall DOING_MT_SDK_RELEASE=1
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make uninstall DOING_MT_SDK_RELEASE=1 failed!"
	exit 1
fi

make -C loader uninstall
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader uninstall failed!"
	exit 1
fi

make -C loader uninstall DOING_MT_SDK_RELEASE=1
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "make -C loader uninstall DOING_MT_SDK_RELEASE=1 failed!"
	exit 1
fi

exit 0
