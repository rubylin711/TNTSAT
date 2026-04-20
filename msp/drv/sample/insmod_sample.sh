#!/bin/sh

#mkdir /lib/modules/3.18.0 -p
#mkdir /lib/modules/ko -p
#cp /mnt/sda1/ko/* /lib/modules/ko/ -f

insmod /lib/modules/ko/mt_mmz.ko
insmod /lib/modules/ko/mt_common.ko 
insmod /lib/modules/ko/mt_sync.ko
insmod /lib/modules/ko/msp_base.ko
insmod /lib/modules/ko/ir.ko
insmod /lib/modules/ko/avplay.ko
insmod /lib/modules/ko/gpio.ko
insmod /lib/modules/ko/mtest.ko
insmod /lib/modules/ko/keyled.ko
insmod /lib/modules/ko/mali.ko
insmod /lib/modules/ko/mt_adec.ko 
insmod /lib/modules/ko/power.ko 
insmod /lib/modules/ko/pq.ko 
insmod /lib/modules/ko/sci.ko 
insmod /lib/modules/ko/vfmw.ko 
insmod /lib/modules/ko/matimer.ko
insmod /lib/modules/ko/demux.ko
insmod /lib/modules/ko/jpeg.ko 
insmod /lib/modules/ko/jpge.ko
insmod /lib/modules/ko/mtimer.ko
insmod /lib/modules/ko/mt_dma.ko
insmod /lib/modules/ko/mt_fe.ko
insmod /lib/modules/ko/mt_tde.ko
insmod /lib/modules/ko/wdg.ko
insmod /lib/modules/ko/vpasss.ko
insmod /lib/modules/ko/venc.ko
insmod /lib/modules/ko/hdmi.ko
insmod /lib/modules/ko/vdec.ko
insmod /lib/modules/ko/aiao.ko
insmod /lib/modules/ko/vo.ko
insmod /lib/modules/ko/mtfb.ko
insmod /lib/modules/ko/mtosd.ko
