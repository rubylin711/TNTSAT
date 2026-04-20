#bash
mode=$1

if [ $mode -eq 1 ];then
        lsmod | grep ehci_hcd
        echo $?
        if [ $? -eq 0 ];then
                rmmod ehci_hcd
        fi

        if [ ! -f "/tmp/disk.img" ]; then
                dd if=/dev/zero of=/tmp/disk.img  bs=1M  count=20
                mkfs.vfat /tmp/disk.img
        fi
#        if [ ! -f "/lib/modules/udc-core.ko" ]; then
#                echo "module does not exist."
#                exit
#        fi
#        modprobe udc-core
        modprobe mt_usb_udc
#        modprobe libcomposite
#        modprobe usb_f_mass_storage
        modprobe g_mass_storage file=/tmp/disk.img  stall=0 removable=1
        echo "====switched to device mode ====="
else

        lsmod | grep g_mass_storage
        echo $?
        if [ $? -eq 0 ];then
                rmmod g_mass_storage
        fi

#        lsmod | grep usb_f_mass_storage
#        echo $?
#        if [ $? -eq 0 ];then
#                rmmod usb_f_mass_storage
#        fi
#        lsmod | grep libcomposite
#        echo $?
#        if [ $? -eq 0 ];then
#                rmmod libcomposite
#        fi
        lsmod | grep mt_usb_udc
        echo $?
        if [ $? -eq 0 ];then
                rmmod mt_usb_udc
        fi
#        lsmod | grep udc_core
#        echo $?
#        if [ $? -eq 0 ];then
#                rmmod udc_core
#        fi
        modprobe ehci-hcd
        echo "====switched to host mode ====="
fi
