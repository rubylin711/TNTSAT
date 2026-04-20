enable TEST_MT_CACHE_API and TEST_LOCAL_BUS in common/drv/cache/mt_cache.c

bridge test:
symphony4: enable TEST_MT_CACHE_API and TEST_LOCAL_BUS and TEST_AVPLAY
http://socsw-gitserver.montage-lz.com:8081/#/c/13255/
symphony6: enable TEST_MT_CACHE_API and TEST_LOCAL_BUS
http://socsw-gitserver.montage-lz.com:8081/#/c/18927/

found a place to reserve 2M from lowmem
[    0.000000] Virtual kernel memory layout:
[    0.000000]     vector  : 0xffff0000 - 0xffff1000   (   4 kB)
[    0.000000]     fixmap  : 0xffc00000 - 0xfff00000   (3072 kB)
[    0.000000]     vmalloc : 0xd0000000 - 0xff800000   ( 760 MB)
[    0.000000]     lowmem  : 0xc0000000 - 0xcfa00000   ( 250 MB)
[    0.000000]     modules : 0xbf000000 - 0xc0000000   (  16 MB)
[    0.000000]       .text : 0xc0008000 - 0xc0a00000   (10208 kB)
[    0.000000]       .init : 0xc0e00000 - 0xc1000000   (2048 kB)
[    0.000000]       .data : 0xc1000000 - 0xc10ac7dc   ( 690 kB)
[    0.000000]        .bss : 0xc10b4e28 - 0xc1722440   (6582 kB)
after bss 0xc1722440, use va 0xc2000000, pa 0x2000000

modify bootargs, reduct top 2M and mid 2M
- setenv memargs mem=250M mmz=pcm,0,0,4M mmz=av,0,0,88M mmz=ddr,0,0,56M
+ setenv memargs mem=32M@0 mem=214M@34M mmz=pcm,0,0,4M mmz=av,0,0,88M mmz=ddr,0,0,56M bus_test=32M,248M
keep the same size and addr with comments in function __mt_dcache_api_test

echo 1300000 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed
x="1"
while true; do echo 0x2 > /sys/kernel/debug/cache_and_bus_test ; echo "$x"; x="`expr $x + 1`"; done

echo 0 > /sys/kernel/debug/cache_and_bus_test_enable
echo 1 > /sys/kernel/debug/cache_and_bus_test_enable





adjust-voltage.sh high 180 2
adjust-voltage.sh low 180 2

benchmark.sh high
benchmark.sh high one
benchmark.sh low

#set cpu as 0 or 1 in uboot
setenv dvfs_test_percpu dvfs_test_percpu=0; boot
setenv dvfs_test_percpu dvfs_test_percpu=1; boot
benchmark-percpu.sh high
benchmark-percpu.sh high one
benchmark-percpu.sh low

hotplug.sh

power.sh
power-percpu.sh

linpack-orig.sh
linpack.sh

max-power-consumption.sh
max-power-consumption-percpu.sh

power-emu.sh

avs-stable.sh

switch_cpufreq random
switch_cpufreq sequence

kernel/rootfs_arm/root/.profile
add the following
mount -t jffs2 /dev/mtdblock7 /usr/local
mkdir -p /usr/local/stbdata
#if [ -f "/usr/local/stbdata/core_ringo" ]; then
#	core_ringo="`cat /usr/local/stbdata/core_ringo`"
#else
#	core_ringo="255"
#fi
#if [ -f "/usr/local/stbdata/offset_vcode" ]; then
#	offset_vcode="`cat /usr/local/stbdata/offset_vcode`"
#else
#	offset_vcode="5"
#fi
#echo ${core_ringo} 1 ${offset_vcode} ${vcode_initial} ${vcode_min} ${vcode_max} > /sys/kernel/debug/regulator/vcc_core/ringo
echo ${cpu_ringo} 1 ${offset_vcode} ${vcode_initial} ${vcode_min} ${vcode_max} > /sys/kernel/debug/regulator/vcc_cpu/ringo
adjust-voltage.sh high 250 1 &


kernel/rootfs_arm/etc/init.d/rcS
/bin/echo "userspace" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

wb/start.sh
./sample_wb 2>&1 < /root/autorun-symphony6.txt



make flash; cp -af pub/bin_striped/bin/mt/switch_cpufreq pub/bin_striped/bin/mt/dvfs-long-term-test.sh pub/bin_striped/bin/mt/avs-stable.sh pub/bin_striped/bin/mt/power-emu pub/bin_striped/bin/mt/power-emu.sh pub/bin_striped/bin/mt/cpufreq_limit.sh pub/bin_striped/bin/mt/power.sh pub/bin_striped/bin/mt/power-percpu.sh pub/bin_striped/bin/mt/linpack-orig.sh pub/bin_striped/bin/mt/linpack.sh pub/bin_striped/bin/mt/max-power-consumption.sh pub/bin_striped/bin/mt/max-power-consumption-percpu.sh pub/bin_striped/bin/mt/adjust-voltage.sh pub/bin_striped/bin/mt/benchmark.sh pub/bin_striped/bin/mt/benchmark-percpu.sh pub/bin_striped/bin/mt/hotplug.sh pub/bin_striped/bin/mt/read_avs_data pub/bin_striped/bin/mt/nbench pub/bin_striped/bin/mt/dhrystone pub/bin_striped/bin/mt/pi_css5 pub/bin_striped/bin/mt/coremark.exe pub/bin_striped/bin/mt/linpack pub/bin_striped/bin/mt/linpack-orig pub/bin_striped/bin/mt/max-power-consumption pub/bin_striped/bin/mt/stream_c.exe pub/bin_striped/bin/djpeg pub/rootfs_striped/usr/bin/; cp -af pub/resource/COM.DAT pub/resource/NNET.DAT pub/resource/debugbit.good.gz pub/resource/1.jpg pub/resource/2.jpg pub/resource/1.jpg-x86.result pub/resource/2.jpg-x86.result pub/resource/pi4194304-x86.txt pub/resource/pi32768-x86.txt pub/resource/autorun-symphony4.txt pub/resource/autorun-symphony6.txt pub/rootfs_striped/root/; cp -af pub/shared_lib_striped/libjpeg.so* pub/rootfs_striped/usr/lib/; make flash;
