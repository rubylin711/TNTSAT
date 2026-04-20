#
# Board Configurations are used for stage of making images,
# not for compiling stage.
#

ROOTFS_RAMDISK:=n
#ROOTFS_TYPE:=initramfs cramfs squashfs ubifs
ROOTFS_TYPE:=squashfs

#USERFS_TYPE:=cramfs
USERFS_TYPE:=squashfs
#USERFS_TYPE:=jffs2
#USERFS_TYPE:=ubifs
USERFS_SIZE:=0

#DATAFS_TYPE:=yaffs2 jffs2 ubifs
DATAFS_TYPE:=jffs2
DATAFS_SIZE_NOR:=0x60000
DATAFS_SIZE_NAND:=0x400000

BOARD_WIFI_SUPPORT:=n
BOARD_WIFI_MODULE:=rtl8821cu rtl8723ds rtl8188eu rtl8188ftv

# gstreamer
BOARD_GST_SUPPORT:=n

