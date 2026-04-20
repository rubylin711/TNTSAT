1.How to compile	
./patch_sdk.sh  ../linux	
cd ../linux && source envsetup symphony4_512.cfg && make 

the images will be generated in linux/image/boston/output-board_lqfp_std_v30/update_fta30
©À©¤©¤ av_cpu512m_sym4_encrypt.bin
©À©¤©¤ boot.img
©À©¤©¤ dte_boot.img
©À©¤©¤ logo.jpg
©À©¤©¤ nanddatafs.yaffs2
©À©¤©¤ nandubi.img
©À©¤©¤ rootfs.img
©À©¤©¤ symphony4_ddr3_1866M_autoboot_all.img
©À©¤©¤ uboot_symphony_demo512_rawnand_encrypt.scr
©À©¤©¤ uboot_symphony_demo512_spinand_encrypt.scr
©À©¤©¤ uboot_symphony_pinctrl.scr
©À©¤©¤ uImage-dtb
©À©¤©¤ update_512_rawnand_encrypt.vbs
©À©¤©¤ update_512_spinand_encrypt.vbs
©¸©¤©¤ usrfs.img



2.How to fuse the images
  1)copy linux/image/boston/output-board_lqfp_std_v30/update_fta30/* files to USB Device root directory

  2)Plug in the USB Device into usb interface, and reboot to entry uboot shell

  3)Use SecureCrt script to run  update_512_spinand_encrypt.vbs


3.How to test
  1)copy linux\pub\shared_lib_striped\ to USB Device
  2)folow sample readme copy sample app and resource to USB Device
  3)Plug in the USB Device into usb interface,and mount it to Device system
  4)set LD_LIBRARY_PATH to USB device lib path # export LD_LIBRARY_PATH=/mnt/lib/:/usr/local/lib
  5)run sample app folow sample readme




