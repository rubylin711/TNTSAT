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


	'btinit+uboot: 4M (0-4)
	crt.Screen.WaitForString "U-Boot >"
	crt.Screen.Send "pri ddr_type_string" & chr(13)
	receivedData = crt.Screen.ReadString ("U-Boot >",3)
	'crt.Dialog.MessageBox receivedData
	If InStr(receivedData, "DDR4") > 0  Then
		crt.Screen.Send "echo DDR4" & chr(13)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/all_pss.img" & chr(13)
	Else
		crt.Screen.Send "echo DDR3" & chr(13)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/all_pss.img" & chr(13)
	End If
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	'erase all
	crt.Screen.Send "snf scrub" & chr(13)
	crt.Screen.WaitForString "spi nand scrub successfaully!"
	crt.Sleep 100
	
	
	crt.Screen.Send "snf erase 0 0x400000" & chr(13)
	crt.Screen.WaitForString "snf erase successfully"
	crt.Sleep 100
	crt.Screen.Send "snf burn 0x04000000 0 $filesize" & chr(13)
	crt.Screen.WaitForString "snf burn successfully"
	crt.Sleep 100

	'bootargs: 512K, 64k in fact (4-4.5)
	crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/uboot_symphony_demo_unified.scr" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "snf erase 0x400000 0x80000" & chr(13)
	crt.Screen.WaitForString "snf erase successfully"
	crt.Sleep 100
	crt.Screen.Send "snf burn 0x04000000 0x400000 $filesize" & chr(13)
	crt.Screen.WaitForString "snf burn successfully"
	crt.Sleep 100

	'syscfg: 512K, 64k in fact (4.5-5)
	crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/uboot_symphony_syscfg.scr" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "snf erase 0x480000 0x80000" & chr(13)
	crt.Screen.WaitForString "snf erase successfully"
	crt.Sleep 100
	crt.Screen.Send "snf burn 0x04000000 0x480000 $filesize" & chr(13)
	crt.Screen.WaitForString "snf burn successfully"
	crt.Sleep 100

	'teeos: 2M (8-10)
	crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/teeos.bin" & chr(13)
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "snf erase 0x800000 0x200000" & chr(13)
	crt.Screen.WaitForString "snf erase successfully"
	crt.Sleep 100
	crt.Screen.Send "snf burn 0x04000000 0x800000 $filesize" & chr(13)
	crt.Screen.WaitForString "snf burn successfully"
	crt.Sleep 100
	
	'teeos-dtb: 2M (10-12)
	crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/tee.dtb.bin" & chr(13)
	crt.Screen.WaitForString "bytes read"

	crt.Screen.Send "snf erase 0xA00000 0x200000" & chr(13)
	crt.Screen.WaitForString "snf erase successfully"
	crt.Sleep 100
	crt.Screen.Send "snf burn 0x04000000 0xA00000 $filesize" & chr(13)
	crt.Screen.WaitForString "snf burn successfully!"
	crt.Sleep 100

	crt.Screen.WaitForString "U-Boot >"
	crt.Screen.Send "pri flash_size" & chr(13)
	receivedData = crt.Screen.ReadString ("U-Boot >",3)
	'crt.Dialog.MessageBox receivedData
	If InStr(receivedData, "128") > 0  Then
		'logo: 2M, (14-16)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/logo.jpg" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0xE00000 0x200000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0xE00000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'av_cpu: 2M (16-18)(move to usrfs)
		'crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/av_cpu_sym6.bin" & chr(13)
		'crt.Screen.WaitForString "bytes read"
		'crt.Sleep 100

		crt.Screen.Send "snf erase 0x1000000 0x200000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		'crt.Screen.Send "snf burn 0x04000000 0x1000000 $filesize" & chr(13)
		'crt.Screen.WaitForString "snf burn successfully"
		'crt.Sleep 100

		'dtb: 2M (22-24)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/symphony6-rawnand-gmac.dtb" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x1600000 0x200000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x1600000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'uImage: 16M (24-40)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/uImage" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x1800000 0x1000000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x1800000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'rootfs: 32M (40-72)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/rootfs_squashubi.img" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x2800000 0x2000000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x2800000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'usrfs: 46M (72-118)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/usrfs_squashubi.img" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x4800000 0x2E00000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x4800000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'data: 8M (118-126)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/nandubi.img" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x7600000 0x800000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x7600000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

	Else
		'logo: 8M, (14-22)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/logo.jpg" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0xE00000 0x800000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0xE00000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'av_cpu: 2M (22-24)(move to usrfs)
		'crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/av_cpu_sym6.bin" & chr(13)
		'crt.Screen.WaitForString "bytes read"
		'crt.Sleep 100

		crt.Screen.Send "snf erase 0x1600000 0x200000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		'crt.Screen.Send "snf burn 0x04000000 0x1600000 $filesize" & chr(13)
		'crt.Screen.WaitForString "snf burn successfully"
		'crt.Sleep 100

		'dtb: 2M (46-48)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/symphony6-rawnand-gmac.dtb" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x2E00000 0x200000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x2E00000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'uImage: 32M (48-80)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/uImage" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x3000000 0x2000000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x3000000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'rootfs: 64M (80-144)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/rootfs.img" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x5000000 0x4000000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x5000000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'usrfs: 88M (144-232)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/usrfs.img" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0x9000000 0x5800000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0x9000000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'data: 8M (232-240)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/nandubi.img" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0xE800000 0x0800000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0xE800000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'pk: 1M (118-126)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/0303.pk" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0xf100000 0x100000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0xf100000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

		'csc: 1M (118-126)
		crt.Screen.Send "fatload usb 0 0x04000000 "&imageDir&"/csc.dat" & chr(13)
		crt.Screen.WaitForString "bytes read"
		crt.Sleep 100

		crt.Screen.Send "snf erase 0xf200000 0x100000" & chr(13)
		crt.Screen.WaitForString "snf erase successfully"
		crt.Sleep 100
		crt.Screen.Send "snf burn 0x04000000 0xf200000 $filesize" & chr(13)
		crt.Screen.WaitForString "snf burn successfully"
		crt.Sleep 100

	End If

End Sub
