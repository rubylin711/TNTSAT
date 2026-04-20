#$language = "VBScript"
#$interface = "1.0"

dim imageDir
imageDir = "update"

crt.Screen.Synchronous = True

' This automatically generated script may need to be
' edited in order to work correctly.

Sub Main

	crt.Screen.Send "usb start" & chr(13)
	crt.Screen.WaitForString "Storage Device(s) found"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc dev 0" & chr(13)
	crt.Screen.WaitForString "mmc0(part 0) is current device"
	crt.Sleep 100

	'bootargs: 512K
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/uboot_symphony_demo_unified.scr" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x400000 0x80000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x400000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'syscfg: 512K
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/uboot_symphony_syscfg.scr" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x480000 0x80000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x480000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'teeos: 2M, 8-10
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/teeos.bin" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x800000 0x200000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x800000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100
	
	'teeos-dtb: 2M (10-12)
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/tee.dtb.bin" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0xA00000 0x200000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0xA00000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'logo: 8M, 14-22
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/logo.jpg" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0xE00000 0x800000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0xE00000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'av_cpu: 2M, 22-24(move to usrfs)
	'crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/av_cpu_sym6_encrypt.bin" & chr(13)
	'crt.Screen.WaitForString "bytes read"
	'crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x1600000 0x200000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	'crt.Screen.Send "mt_mmc write 0x03000000 0x1600000 $filesize" & chr(13)
	'crt.Screen.WaitForString "blocks written: OK"
	'crt.Sleep 100

	'dtb: 2M, 46-48
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/symphony6-emmc-gmac.dtb" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x2E00000 0x200000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x2E00000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'uImage: 32M, 48-80
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/uImage" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x3000000 0x2000000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x3000000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'rootfs: 128M, 80-208
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/rootfs.ext4" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x5000000 0x8000000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x5000000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'usrfs: 128M, 208-336
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/usrfs.ext4" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0xD000000 0x8000000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0xD000000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	'data: 128M, 336-464
	crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/datafs.ext4" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc erase 0x15000000 0x8000000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mt_mmc write 0x03000000 0x15000000 $filesize" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100


End Sub
