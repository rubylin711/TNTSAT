#!/bin/bash
WORK_PATH=`pwd`
tag_name=$1
product=aria
RELEASE_PATH=/homegit/upload/release/temporary/Aria/STB/
RELEASE_offi_PATH=/homegit/upload/release/official/Aria/STB/
montage_ver=$WORK_PATH/montage_ver.txt

for i in `cat $montage_ver`
do
git_path=`echo $i |awk -F ":" '{print $1}'`
git_ver=`echo $i |awk -F ":" '{print $2}'`
cd $WORK_PATH/../$git_path
git tag $tag_name
done
make kernel_clean
make mclean
make mall
if [ ! -d $RELEASE_PATH ];then
mkdir $RELEASE_PATH
chmod -R 775 $RELEASE_PATH
fi

if [ ! -d $RELEASE_offi_PATH ];then
mkdir $RELEASE_offi_PATH
chmod -R 775 $RELEASE_PATH
fi
cd $WORK_PATH
RELEASE_PATH=$RELEASE_PATH${tag_name%_*}
release_name=${tag_name%_*}
if [ ! -d ./$release_name ];then
mkdir ./$release_name
fi
#mkdir -p ./$release_name/kernel/linux3.18/arch/arm/boot/uImage-dtb
#mkdir -p ./$release_name/wb/sample_wb
#mkdir -p ./$release_name/tools/chip
#cp -rf ./kernel/linux3.18/arch/arm/boot/uImage-dtb ./$release_name/kernel/linux3.18/arch/arm/boot/uImage-dtb
mkdir -p ./$release_name/testcase
cp -rf $WORK_PATH/../$git_path/common ./$release_name/common
cp -rf $WORK_PATH/../$git_path/kernel ./$release_name/kernel
cp -rf $WORK_PATH/../$git_path/msp ./$release_name/msp
cp -rf $WORK_PATH/../$git_path/pub ./$release_name/pub
cp -rf $WORK_PATH/../$git_path/wb ./$release_name/wb
cp -rf $WORK_PATH/../$git_path/sample ./$release_name/sample
cp -rf $WORK_PATH/../$git_path/scripts ./$release_name/scripts
cp -rf $WORK_PATH/../$git_path/testfm ./$release_name/testfm
cp -rf $WORK_PATH/../$git_path/tools ./$release_name/tools
cp $WORK_PATH/../$git_path/kernel/linux3.18/arch/arm/boot/uImage-dtb ./$release_name/testcase/
cp $WORK_PATH/../$git_path/wb/sample_wb ./$release_name/testcase/
cp $WORK_PATH/../$git_path/tools/chip/flash_boot_ddr3.bin ./$release_name/testcase/
cp $WORK_PATH/../$git_path/tools/chip/flash_boot_ddr4.bin ./$release_name/testcase/
rm -rf ./$release_name/common/drv
rm -rf ./$release_name/msp/drv
#rsync -av --exclude ../linux/common/drv --exclude ../linux/msp/drv --exclude ../linux/kware ../linux ./$release_name
chmod -R 775 ./$release_name/
cp -r -f -p ./$release_name $RELEASE_PATH
rm -rf ./$release_name
exit 0
