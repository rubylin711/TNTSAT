#!/bin/bash

cmdpaths=$(pwd)

LINUX_MAP_FILE=$1
ROOTFS_FILE=$2
USRFS_FILE=$3
BOOTENV_FILE=$4
#echo ${LINUX_MAP_FILE}



function align4k()
{
#  echo 1=${1}
	aa=$((16#${1}+0))
#	echo aa=${aa}
	bb=`expr $aa \+ 4095`
	cc=`expr $bb \/ 4096`
	align4k_val=`expr $cc \* 4096`
#	count=$(echo "obase=16;${dd}"|bc)
#	echo align4k_val=${align4k_val}
}

start_addr=`grep "B _end" ${LINUX_MAP_FILE}| cut -d " " -f 1`
start_txt=`grep "T _text" ${LINUX_MAP_FILE}| cut -d " " -f 1`

align4k ${start_addr}
rootfs_start_tmp_d=${align4k_val}
rootfs_start_tmp_d1=$(($rootfs_start_tmp_d &268435455))
sysmap_end=$(echo "obase=16;${rootfs_start_tmp_d1}"|bc)
echo sysmap_end=${sysmap_end}
align4k ${start_txt}
rootfs_start_tmp_s=${align4k_val}
rootfs_start_tmp_s1=$(($rootfs_start_tmp_s &268435455))
sysmap_start=$(echo "obase=16;${rootfs_start_tmp_s1}"|bc)
echo sysmap_start=${sysmap_start}
sysmap_size=$(expr $rootfs_start_tmp_d1 - $rootfs_start_tmp_s1)
sysmap_size1=$(echo "obase=16;${sysmap_size}"|bc)
echo sysmap_size1=${sysmap_size1}
rootfs_start_d=`expr $sysmap_size \+ 52428800`
rootfs_start=$(echo "obase=16;${rootfs_start_d}"|bc)
echo rootfs_start=${rootfs_start}

rootfs_size_d=$(ls -l ${ROOTFS_FILE}  |awk '{printf $5}')
rootfs_size=$(echo "obase=16;${rootfs_size_d}"|bc)
echo rootfs_size=${rootfs_size}

usr_size_d=$(ls -l ${USRFS_FILE}  |awk '{printf $5}')
usr_size=$(echo "obase=16;${usr_size_d}"|bc)
echo usr_size=${usr_size}


usr_start_tmp_d=`expr $rootfs_start_d \+ $rootfs_size_d`
echo usr_start_tmp=${usr_start_tmp_d}
usr_start_tmp=$(echo "obase=16;${usr_start_tmp_d}"|bc)
align4k ${usr_start_tmp}
usr_start_d=${align4k_val}
echo usr_start_d=${usr_start_d}
usr_start=$(echo "obase=16;${usr_start_d}"|bc)
echo usr_start=${usr_start}

sed 's/RD_START0/0x'"${rootfs_start}"'/' $BOOTENV_FILE > tmp1.env
sed 's/RD_SIZE0/0x'"${rootfs_size}"'/' tmp1.env > tmp2.env
sed 's/loadimg 0 $ram1addr ram1size ram1start/loadimg 0 $ram1addr  0x'"${usr_size}"' 0x'"${usr_start}"'/' tmp2.env > tmp3.env
sed 's/setenv bootargs $bootargs rd=ram1size@ram1start,1/setenv bootargs $bootargs rd='"${usr_size_d}"'@0x'"${usr_start}"',1/' tmp3.env > tmp4.env
#sed 's/#ramdisk_flag=1/ramdisk_flag=1/' tmp4.env > tmp5.env

cp -rf tmp4.env $BOOTENV_FILE
rm tmp1.env
rm tmp2.env
rm tmp3.env
rm tmp4.env
