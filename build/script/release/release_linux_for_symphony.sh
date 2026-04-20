#!/bin/bash

WORK_PATH=`pwd`
#make release folder name as Release.
RELEASE_PATH=Release

rm -rf ${RELEASE_PATH}
mkdir -p ${RELEASE_PATH}/linux

echo -e "\e[0;32mrsync!\e[m"
rsync -avzh --exclude=".git" --ignore-errors ../linux/ ${RELEASE_PATH}/linux
#should delete release folder itself.
rm ${RELEASE_PATH}/linux/${RELEASE_PATH} -fr
echo -e "\e[0;32mrsync done!\e[m"

cd ${RELEASE_PATH}/linux

echo -e "\e[0;32mbuild!\e[m"
mt_chip=${mt_chip} size=${size} ./build/script/release/release-build-linux.sh
RESULT=$?
if [ "${RESULT}" != "0" ]; then
	echo "mt_chip=${mt_chip} size=${size} ./build/script/release/release-build-linux.sh failed!"
	exit 1
fi

mkdir -p bak
cp -af --parents msp/drv/vfmw/video_lib_1.0/vfmw.h msp/drv/vo_fw/vo_fw.h tools/toolchain/gdb-mt-jtag/arm-mcj-linux-gnueabihf-gdb bak
rm -rf sample/gpu_mali sample/gpu_mali_11 sample/gpu_mali_openvg sample/cdmi
rm -rf msp/api/gpu msp/api/jpge msp/api/venc msp/api/vpu_enc
rm -rf msp/drv/gpu msp/drv/jpge msp/drv/venc msp/drv/vpu_enc msp/drv/vfmw msp/drv/vo_fw
rm -rf component/network component/fs
rm -rf kware/netapp
if [ "${mt_chip}" = "symphony4" ]; then
	rm -rf product/alaska product/alaska_128 product/alaska_rearch product/chicago product/dongle product/intek product/lotus product/lotus_sym4 product/se_alaska product/suresoft
elif [ "${mt_chip}" = "symphony6" ]; then
	rm -rf product/alaska product/alaska_128 product/alaska_rearch product/boston product/boston_128 product/boston_tee product/dongle product/intek product/lotus product/lotus_sym4 product/se_alaska product/suresoft
else
	rm -rf product/alaska_rearch product/boston product/boston_128 product/boston_tee product/chicago product/dongle product/intek product/lotus product/lotus_sym4 product/se_alaska product/suresoft
fi
rm -rf tools/crash tools/ftrace/*.c tools/perf/FlameGraph tools/gperftools tools/toolchain/crosstool-ng tools/toolchain/gdb-mt-jtag tools/toolchain/gcc-arm-src-snapshot-8.3-2019.03.tar.xz tools/sanitizer tools/commit_template
find docs/ -name "*\.doc*" -exec rm -f {} \;
rm -f docs/porting-note.txt docs/Symphony_Linux_Network_Performance.xlsx
cp -arf bak/* .
rm -rf bak

exit 0
