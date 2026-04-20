#$language = "VBScript"
#$interface = "1.0"


crt.Screen.Synchronous = True

Sub Main
	crt.Screen.Send "suplay -t ./ts_video" & chr(13)
	For i = 1 To 2 '修改循环次数
		crt.Screen.WaitForString("This file playback is complete")
		cmd = "t"
		crt.Screen.Send cmd & chr(13)
		crt.Screen.WaitForString("SUPLAY>> ")
		cmd = "y"
		crt.Screen.Send cmd & chr(13)
	Next
	crt.Screen.WaitForString("This file playback is complete")
	crt.Screen.Send "q" & chr(13)
	MsgBox "The test is complete"
End Sub