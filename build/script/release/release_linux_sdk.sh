#!/bin/bash
set -x

WORK_PATH=`pwd`
product=linux-drv-sdk
chip=$1
project=$2
tag_name=$3
package=$4
RELEASE_PATH=/homegit/upload/release/$chip/$project
RELEASE_offi_PATH=/homegit/upload/release/$chip/$project
montage_ver=$WORK_PATH/montage_ver.txt

if [ "${chip}" = "Symphony4-tee" ]; then
	RELEASE_PATH=/homegit/upload/release/Symphony4/$project
	RELEASE_offi_PATH=/homegit/upload/release/Symphony4/$project
fi

#prepare code env.
for i in `cat $montage_ver`
do
git_path=`echo $i |awk -F ":" '{print $1}'`
git_ver=`echo $i |awk -F ":" '{print $2}'`
cd $WORK_PATH/../$git_path
if [ "${package}" != "tar" ]; then
	git tag $tag_name
fi
done

#add for release version
KERNEL_TAGVERINFO=$tag_name
UIMAGE_NAME=$KERNEL_TAGVERINFO
export KERNEL_TAGVERINFO UIMAGE_NAME
echo "KERNEL_TAGVERINFO = $KERNEL_TAGVERINFO"

cd $WORK_PATH/vendor/montage/sdk/linux

git clean -dxf
git reset --hard

#get commit id
git log -1 > version.txt

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
elif [ "${chip}" = "All" ]; then
	mt_chip_all="symphony1 symphony2 symphony4 symphony4-tee symphony6"
fi

#add for Uboot verson
git log tools/prebuilts/${mt_chip} | grep Version: | awk "{print $1}" | cut -c 4-  >> module_list.txt

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
		size_all="4096 2048 1024 512 256"
	fi

	for size in ${size_all}; do

		cd $WORK_PATH/vendor/montage/sdk/linux
		mt_chip=${mt_chip} size=${size} ./build/script/release/release_linux_for_symphony.sh
		RESULT=$?
		if [ "${RESULT}" != "0" ]; then
			echo "mt_chip=${mt_chip} size=${size} ./build/script/release/release_linux_for_symphony.sh failed!"
			exit 1
		fi

		if [ ! -d $RELEASE_PATH ];then
		mkdir -p $RELEASE_PATH
		chmod -R 750 $RELEASE_PATH
		fi

		if [ ! -d $RELEASE_offi_PATH ];then
		mkdir -p $RELEASE_offi_PATH
		chmod -R 750 $RELEASE_offi_PATH
		fi

		cd $WORK_PATH
		release_name=${tag_name%_*}
		rm -rf ./$release_name ./${release_name}_step2.log
		mkdir ./$release_name

		cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/image-${mt_chip}-${size} ./$release_name/image-${mt_chip}-${size}
		cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/pub-${mt_chip}-${size} ./$release_name/pub-${mt_chip}-${size}
		cp -arf $WORK_PATH/vendor/montage/sdk/linux/version.txt ./$release_name/version.txt
		cp -arf $WORK_PATH/vendor/montage/sdk/linux/module_list.txt ./$release_name/module_list.txt

		if [ "${package}" = "tar" ]; then
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/sample ./$release_name/sample
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/wb ./$release_name/wb
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/pub ./$release_name/pub
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/tools ./$release_name/tools
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/build ./$release_name/build
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/docs ./$release_name/docs
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/envsetup ./$release_name/envsetup
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/opensource ./$release_name/opensource
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/opensource ./$release_name/opensource-2nd
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/msp ./$release_name/msp
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/kware ./$release_name/kware
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/common ./$release_name/common
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/testfm ./$release_name/testfm
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/component ./$release_name/component
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/loader ./$release_name/loader
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/product ./$release_name/product
			mkdir -p ./$release_name/kernel
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/kernel/linux-x.y.z ./$release_name/kernel/linux-x.y.z
			if [ "${mt_chip}" = "symphony4" ]; then
				cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/kernel/rootfs_arm ./$release_name/kernel/rootfs_arm
			elif [ "${mt_chip}" = "symphony6" ]; then
				cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/kernel/rootfs_aarch64 ./$release_name/kernel/rootfs_aarch64
			else
				cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/kernel/rootfs_mips ./$release_name/kernel/rootfs_mips
				cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/kernel/rootfs_mips_128 ./$release_name/kernel/rootfs_mips_128
			fi
			cp -arf $WORK_PATH/vendor/montage/sdk/linux/Release/linux/Makefile ./$release_name/Makefile
		fi

		rm -rf $WORK_PATH/vendor/montage/sdk/linux/Release

		chmod -R 750 ./$release_name/

		if [ "${package}" = "tar" ]; then
			rm -rf $release_name/image-${mt_chip}-${size}
			rm -rf $release_name/pub-${mt_chip}-${size}
			mv $release_name/pub $release_name/pub-${mt_chip}-${size}

			cd $release_name
			mt_chip=${mt_chip} size=${size} ./build/script/release/compile-check-release.sh > ${release_name}_step2.log 2>&1
			RESULT=$?
			cd -
			if [ "${RESULT}" != "0" ]; then
				echo "mt_chip=${mt_chip} size=${size} ./build/script/release/compile-check-release.sh failed!"
				if [ -d "${release_name}_tar_pub" ]; then
					mv ${release_name}_tar_pub/* ${release_name}
					rm -rf ${release_name}_tar_pub
				fi
				mv ${release_name} ${release_name}_step2
				mv ${release_name}_step2/${release_name}_step2.log $RELEASE_PATH
				mv ${release_name}_step2 $RELEASE_PATH
				exit 1
			else
				mkdir -p ${release_name}_tar_pub
				mv $release_name/pub-${mt_chip}-${size} ${release_name}_tar_pub
				rm $release_name/${release_name}_step2.log
			fi
		else
			mkdir -p $RELEASE_PATH/$release_name
			mv $release_name/image-${mt_chip}-${size} $RELEASE_PATH/$release_name
			mv $release_name/pub-${mt_chip}-${size} $RELEASE_PATH/$release_name
			mv $release_name/version.txt $RELEASE_PATH/$release_name
			mv $release_name/module_list.txt $RELEASE_PATH/$release_name
		fi

	done
done

if [ "${package}" = "tar" ]; then
	mv ${release_name}_tar_pub/* $release_name
	tar cJf $release_name.tar.xz $release_name
	mv $release_name.tar.xz $RELEASE_PATH
	rm -rf $release_name ${release_name}_tar_pub
	exit 0
else
	rm -rf $release_name
	exit 0
fi
