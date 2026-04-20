#########################################################################################################################
#
# release_linux_sdk_sym6.sh
#
# $1: chip, e.g. Symphony6
# $2: project, e.g. linux-drv-sdk
# $3: tag_name, e.g. MT_Symphony6_linux_drv_sdk_00xxx_xxx
#
#  Usage:
#   ./build/script/release/release_linux_sdk_sym6.sh Symphony6 linux-drv-sdk MT_Symphony6_linux_drv_sdk_00001_xxx
#   ./build/script/release/release_linux_sdk_sym6.sh Symphony6-tee linux-drv-sdk MT_Symphony6-tee_linux_drv_sdk_00001_xxx
#
#########################################################################################################################


#!/bin/bash
set -x

WORK_PATH=`pwd`
product=linux-drv-sdk
chip=$1
project=$2
tag_name=$3
package=$4

# debug
#RELEASE_PATH=~/release/$chip/$project

# release
RELEASE_PATH=/homegit/upload/release/$chip/$project

montage_ver=version.txt


# move to here for CI test requirement
release_name=${tag_name%_*}
mkdir -p $RELEASE_PATH/$release_name

#repo manifest -r > $montage_ver
#mv -f $montage_ver $RELEASE_PATH/$release_name
cd ../.repo/manifests; git log -1 > $montage_ver; mv -f $montage_ver $RELEASE_PATH/$release_name; cd -


#debug
#exit 0


#add for release version
KERNEL_TAGVERINFO=$tag_name
UIMAGE_NAME=$KERNEL_TAGVERINFO
export KERNEL_TAGVERINFO UIMAGE_NAME
echo "KERNEL_TAGVERINFO = $KERNEL_TAGVERINFO"

#compile image
if [ "${chip}" = "Symphony" ]; then
	mt_chip_all="symphony1"
elif [ "${chip}" = "Symphony2" ]; then
	mt_chip_all="symphony2"
elif [ "${chip}" = "Symphony4" ]; then
	mt_chip_all="symphony4"
elif [ "${chip}" = "Symphony4-tee" ]; then
	mt_chip_all="symphony4-tee"
elif [ "${chip}" = "Symphony6" ]; then
	mt_chip_all="symphony6"
elif [ "${chip}" = "Symphony6-tee" ]; then
	mt_chip_all="symphony6-tee"
elif [ "${chip}" = "All" ]; then
	mt_chip_all="symphony1 symphony2 symphony4 symphony4-tee symphony6 symphony6-tee"
fi

for mt_chip in ${mt_chip_all}; do

	if [ "${mt_chip}" = "symphony1" ]; then
		size_all="512 256 128"
	elif [ "${mt_chip}" = "symphony2" ]; then
		size_all="512 256 128"
	elif [ "${mt_chip}" = "symphony4" ]; then
		size_all="512-tee 512 256 128"
	elif [ "${mt_chip}" = "symphony4-tee" ]; then
		size_all="512 256"
	elif [ "${mt_chip}" = "symphony6" ]; then
		size_all="512 512_gst 512_otpmini"
	elif [ "${mt_chip}" = "symphony6-tee" ]; then
		size_all="512 512_gst"
	fi

	for size in ${size_all}; do

		mt_chip=${mt_chip} size=${size} ./build/script/release/release-build-linux.sh
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "mt_chip=${mt_chip} size=${size} ./build/script/release/release-build-linux failed!"
			exit 1
		fi

		if [ ! -d $RELEASE_PATH ];then
		mkdir -p $RELEASE_PATH
		chmod -R 750 $RELEASE_PATH
		fi

		cd $WORK_PATH
		rm -rf ./$release_name
		mkdir ./$release_name

		cp -arf $WORK_PATH/image-${mt_chip}-${size} ./$release_name/image-${mt_chip}-${size}

		# rm un-used folders for size
		rm -rf ./$release_name/image-${mt_chip}-${size}/*/rootfs
		rm -rf ./$release_name/image-${mt_chip}-${size}/*/usrfs
		rm -rf ./$release_name/image-${mt_chip}-${size}/*/loader/rootfs

		rm -rf ./pub-${mt_chip}-${size}
		rm -rf ./image-${mt_chip}-${size}

		mv $release_name/image-${mt_chip}-${size} $RELEASE_PATH/$release_name

	done
done

rm -rf $release_name
exit 0
