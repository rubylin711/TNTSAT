#start.sh
export PATH=$PATH:/usr/local/stb

# KERN_WARNING: 4
echo 4 > /proc/sys/kernel/printk

LXCENV="`ls -l /proc/1/exe | grep "lxc\-execute"`"
if [ -n "${LXCENV}" ]; then
	echo -e "\033[31m now I'm in lxc environment \033[0m"
	export PATH=$PATH:/usr/local/bin
	export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib:/usr/lib:/lib
	export LD_PRELOAD=$LD_PRELOAD:/lib/libc-mt.so
fi

#load MTK7601
#mkdir -p /etc/Wireless/RT2870STA/
#cp /usr/local/stb/wifi/MT7601USTA.dat /etc/Wireless/RT2870STA/
#insmod /usr/local/stb/wifi/mt7601Usta.ko
#load RTL8188
#insmod /usr/local/stb/wifi/8188eu.ko
##load wi6700
#insmod /usr/local/stb/wifi/m88wi6700u.ko

#/bin/echo "ondemand" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor
#/bin/echo "userspace" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

cd /usr/local/stb
#./thttpd -d /usr/local/stb/ -p 80 -c \* -u root -nor
echo "run fac sample...."
./sample_fac

