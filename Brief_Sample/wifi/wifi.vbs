#$language = "VBScript"
#$interface = "1.0"


crt.Screen.Synchronous = True

Sub Main
	crt.Screen.Send "wifi" & chr(13)
	crt.Screen.WaitForString("[INFO] Scan done!")
	For i = 1 To 3 '修改循环次数
		cmd = "s"
		crt.Screen.Send cmd & chr(13)
		crt.sleep 10000
	Next
	crt.Screen.Send "q" & chr(13)
	MsgBox "The test is complete"
End Sub