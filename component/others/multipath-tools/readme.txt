https://github.com/openSUSE/multipath-tools

losetup /dev/loop0 image
losetup -a
kpartx -av /dev/loop0
ls /dev/mapper/
mount /dev/mapper/loop0p1 /mnt
ls /mnt
touch /mnt/xxoo
sync
umount /mnt
kpartx -d /dev/loop0
ls /dev/mapper/
losetup -d /dev/loop0
losetup -a
