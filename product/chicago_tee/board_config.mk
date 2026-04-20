#
# Board Configurations are used for stage of making images,
# not for compiling stage.
#

ROOTFS_RAMDISK:=n
ROOTFS_TYPE:=initramfs cramfs squashfs ubifs ext4

#USERFS_TYPE:=cramfs
#USERFS_TYPE:=squashfs
#USERFS_TYPE:=jffs2
USERFS_TYPE:=ubifs
USERFS_SIZE:=0

DATAFS_TYPE:=yaffs2 jffs2 ubifs ext4
DATAFS_SIZE_NOR:=0x60000
DATAFS_SIZE_NAND:=0x400000

TEEFS_TYPE:=ubifs ext4

BOARD_WIFI_SUPPORT:=y
BOARD_WIFI_MODULE:=rtl8821cu rtl8723ds rtl8188eu rtl8188ftv atbmwifi rtl88x2cu aic8800

BOARD_USBCI_SUPPORT:=y

# gstreamer
BOARD_GST_SUPPORT:=n

# OTP Revision:
#   FPGA, MINI
#BOARD_OTP_VERSION:=MT_FPGA
BOARD_OTP_VERSION:=MT_OTPMINI


#bluetooth
BOARD_BLUEZ_SUPPORT:=y
BOARD_BT_MODULE := realtek aic8800usb

