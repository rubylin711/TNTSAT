#$language = "VBScript" 
#$interface = "1.0" 
crt.Screen.Synchronous = True 

Sub Main 

	 crt.Screen.Send "usb start" & chr(13)
	 crt.Screen.WaitForString "USB Device(s) found"

	 'all.img
	 crt.Screen.Send "fatload usb 0 0x4000000 update/all.img" & chr(13)
	 crt.Screen.WaitForString "bytes read"

	 crt.Screen.Send "snf scrub" & chr(13)
	 crt.Screen.WaitForString "spi nand scrub successfaully!"
	 crt.Sleep 100	 
	 
	 crt.Screen.Send "snf burn 0x4000000 0 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"
	 
	 'bl3.img
	 crt.Screen.Send "fatload usb 0 0x3000000 update/bl3.img" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf burn 0x3000000 0x200000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

	 'tee(dtb+os)
	 crt.Screen.Send "fatload usb 0 0x3000000 update/tee.nasc" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf erase 0x680000 0x300000" & chr(13)
	 crt.Screen.WaitForString "snf erase successfully"
	 crt.Screen.Send "snf burn 0x3000000 0x680000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

	 'logo
	 crt.Screen.Send "fatload usb 0 0x3000000 update/logo.jpg.nasc" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf erase 0x1c00000 0x100000" & chr(13)
	 crt.Screen.WaitForString "snf erase successfully"
	 crt.Screen.Send "snf burn 0x3000000 0x1c80000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

	 'kernel(dtb+uImage):
	 crt.Screen.Send "fatload usb 0 0x3000000 update/kernel.nasc" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf erase 0x1d00000 0xC00000" & chr(13)
	 crt.Screen.WaitForString "snf erase successfully"
	 crt.Screen.Send "snf burn 0x3000000 0x1d00000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

	 'rootfs
	 crt.Screen.Send "fatload usb 0 0x3000000 update/rootfs_enc.img" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf erase 0x2900000 0x1800000" & chr(13)
	 crt.Screen.WaitForString "snf erase successfully"
	 crt.Screen.Send "snf burn 0x3000000 0x2900000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

	 'usrfs
	 crt.Screen.Send "fatload usb 0 0x3000000 update/usrfs_enc.img" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf erase 0x4100000 0x3000000" & chr(13)
	 crt.Screen.WaitForString "snf erase successfully" 
	 crt.Screen.Send "snf burn 0x3000000 0x4100000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

	 'datafs
	 crt.Screen.Send "fatload usb 0 0x3000000 update/nandubi.img" & chr(13)
	 crt.Screen.WaitForString "bytes read"
	 crt.Screen.Send "snf erase 0x7100000 0x800000" & chr(13) 
	 crt.Screen.Send "snf burn 0x3000000 0x7100000 $filesize" & chr(13)
	 crt.Screen.WaitForString "snf burn successfully"

End Sub 
