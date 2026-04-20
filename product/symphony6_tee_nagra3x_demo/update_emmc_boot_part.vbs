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

	'bootimage: 4MB
	crt.Screen.WaitForString "U-Boot >"
	crt.Screen.Send "pri ddr_type_string" & chr(13)
	receivedData = crt.Screen.ReadString ("U-Boot >",3)
	'crt.Dialog.MessageBox receivedData
	If InStr(receivedData, "DDR4") > 0  Then
		crt.Screen.Send "echo DDR4" & chr(13)
		' fta otp
		crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/all_pss.img" & chr(13)
	Else
		crt.Screen.Send "echo DDR3" & chr(13)
		' fta otp
		crt.Screen.Send "fatload usb 0 0x03000000 "&imageDir&"/all_pss.img" & chr(13)
	End If
	crt.Screen.WaitForString "bytes read"
	crt.Sleep 100

	crt.Screen.Send "mmc dev 0" & chr(13)
	crt.Screen.WaitForString "mmc0(part 0) is current device"
	crt.Sleep 100

	crt.Screen.Send "mmc partconf 0 1 1 1" & chr(13)
	crt.Screen.WaitForString "U-Boot >"
	crt.Sleep 100

	crt.Screen.Send "mmc erase 0 2000" & chr(13)
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100

	crt.Screen.Send "mmc write 0x3000000 0 2000" & chr(13)
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100

	crt.Screen.Send "mmc partconf 0 1 1 0" & chr(13)
	crt.Screen.WaitForString "U-Boot >"
	crt.Sleep 100

End Sub
