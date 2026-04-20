#$language = "VBScript"
#$interface = "1.0"


crt.Screen.Synchronous = True

Sub Main
	crt.Screen.Send "audio -t ./audio_file" & chr(13)
	For i = 1 To 2 '修改循环次数
		crt.Screen.WaitForString("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER")
		cmd = "t"
		crt.Screen.Send cmd & chr(13)
		crt.Screen.WaitForString("AUDIO>> ")
		cmd = "y"
		crt.Screen.Send cmd & chr(13)
	Next
	crt.Screen.WaitForString("Read ts file end and rewind and Reset TS BUFFER and AVPLAYER")
	crt.Screen.Send "q" & chr(13)
	MsgBox "The test is complete"
End Sub