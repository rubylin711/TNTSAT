#$language = "VBScript"
#$interface = "1.0"

' /dev/mtd7 这个参数看启动打印中分区表，如下举例：
' Creating 11 MTD partitions on "mt_snf": 表示分区数量
' 从上往下，"btinit_uboot" 分区使用"/dev/mtd0"表示，"boot.scr" 分区使用"/dev/mtd1"表示；以此类推！

' [    1.457790] ===============MT_spi_nand_init start........===============
' [    1.465620] SNF DRV: Detected GD5F1GQ5UEYIG with page size: 2048, oob size: 64, total 128 M bytes
' [    1.474293] mt_snf: parser cmdlinepart: 11
' [    1.478280] 11 cmdlinepart partitions found on MTD device mt_snf
' [    1.484269] Creating 11 MTD partitions on "mt_snf":
' [    1.489203] 0x000000000000-0x000000400000 : "btinit_uboot"
' [    1.498440] 0x000000400000-0x000000480000 : "boot.scr"
' [    1.503105] 0x000000480000-0x000000500000 : "pin.scr"
' [    1.508326] 0x000000500000-0x000000700000 : "logo"
' [    1.512778] 0x000000700000-0x000000900000 : "av_cpu.img"
' [    1.518109] 0x000000900000-0x000001800000 : "kernel.img"
' [    1.523465] 0x000001800000-0x000002a00000 : "rootfs"
' [    1.528455] 0x000002a00000-0x000005c00000 : "usrfs.img"
' [    1.533498] 0x000005c00000-0x000006b00000 : "datafs.img"
' [    1.538833] 0x000006b00000-0x000006f00000 : "loaderdb"
' [    1.543799] 0x000006f00000-0x000008000000 : "user"
' [    1.548775] 0x000000000000-0x000008000000 : "all_flash"
' [    1.554008] ===============MT_spi_nand_init end..........===============



crt.Screen.Synchronous = True

Dim keywords(8)
keywords(0) = "case img succeeded"
keywords(1) = "case A5 data succeeded"
keywords(2) = "case block succeeded"
keywords(3) = "case interrupt succeeded"
keywords(4) = "[ERROR] case img failed"
keywords(5) = "[ERROR] case A5 data failed"
keywords(6) = "[ERROR] case block failed"
keywords(7) = "[ERROR] case interrupt failed"
keywords(8) = "[ERROR] file open error!!"

Dim Info

Function RunImg(a, b)
	'添加跳过执行判断
	If a = 0 Then
		RunImg = True
		Exit Function
	End If
	Dim cmd
	cmd = "flashavl -c /dev/mtd7 /media/casetest/usrfs.img "            '(能修改，各参数含义见readme)case1 镜像文件烧录
	cmd = cmd & b

	crt.Screen.Send " " & chr(13)
	For i = 1 To a '修改循环次数
		If(crt.Screen.WaitForString("MTCMD>> ", 60) <> False) Then
			crt.Screen.Send " " & chr(13)
			crt.Screen.WaitForString("MTCMD>> ")
			crt.Screen.Send cmd & chr(13)   
			index = crt.Screen.WaitForStrings(keywords)

			Select Case index
				Case 1
					crt.Screen.Send "exit" & chr(13)
					crt.sleep 3000
					crt.Screen.Send "reboot" & chr(13)
				Case 5                                                  'case1出错
					RunImg = False
					Info = Info & vbLf & keywords(4)
					Exit Function
				Case 9                                                  '镜像文件烧录case的镜像文件无法打开，可能是文件名输入错误，需要修改正确的镜像文件路径，然后重新运行脚本。
					'crt.sleep 3000                                     '也有可能是板子未识别到U盘，可以把下面重启命令打开，板子将重启重新去识别U盘
					'crt.Screen.Send "exit" & chr(13)
					'crt.Screen.Send "reboot" & chr(13)                 '当板子出现识别不到/media/casetes/路径的时候将上面三行命令打开，并把下三行代码屏蔽
					RunImg = False                                      '当板子不会出现识别不到的情况把下面三行退出函数代码打开，把上三行代码屏蔽
					Info = Info & vbLf & keywords(4) & vbLf & keywords(8)
					Exit Function
			End Select
		Else
			crt.Screen.Send "The board does not start normally, and the img write fails" & chr(13) '镜像文件烧录失败，程序无法正常运行
			RunImg = False
			Info = Info & vbLf & keywords(4)
			Exit Function
		End If
	Next
	
	Info = Info & vbLf & keywords(0)
	RunImg = True
End Function

Function RunData(a, b)
	If a = 0 Then
		RunData = True
		Exit Function
	End If
	Dim cmd
	cmd = "flashavl -n /dev/mtd8 "                                      '(能修改，各参数含义见readme)case2 5A和A5数据读写
	cmd = cmd & b
	
	crt.Screen.Send " " & chr(13)
	For i = 1 To a '修改循环次数
		crt.Screen.WaitForString("MTCMD>> ")
		crt.Screen.Send " " & chr(13)
		crt.Screen.WaitForString("MTCMD>> ")
		crt.Screen.Send cmd & chr(13)             
		index = crt.Screen.WaitForStrings(keywords)
		Select Case index
			Case 6                                                      'case2出错
				RunData = False
				Info = Info & vbLf & keywords(5)
				Exit Function
		End Select
	Next
	crt.Screen.WaitForString("MTCMD>> ")
	crt.Screen.Send "exit" & chr(13)
	crt.sleep 3000
	crt.Screen.Send "reboot" & chr(13)
	Info = Info & vbLf & keywords(1)
	RunData = True
End Function

Function RunBlock(a, b)
	If a = 0 Then
		RunBlock = True
		Exit Function
	End If
	Dim cmd
	cmd = "flashavl -f /dev/mtd8 "                                      '(能修改，各参数含义见readme)case3测试写入block是否影响其他block
	cmd = cmd & b
	
	crt.Screen.Send " " & chr(13)
	For i = 1 To a '修改循环次数
		crt.Screen.WaitForString("MTCMD>> ")
		crt.Screen.Send " " & chr(13)
		crt.Screen.WaitForString("MTCMD>> ")
		crt.Screen.Send cmd & chr(13)             
		index = crt.Screen.WaitForStrings(keywords)
		Select Case index
			Case 7                                                      'case3出错
				RunBlock = False
				Info = Info & vbLf & keywords(6)
				Exit Function
		End Select
	Next
	crt.Screen.WaitForString("MTCMD>> ")
	crt.Screen.Send "exit" & chr(13)
	crt.sleep 3000
	crt.Screen.Send "reboot" & chr(13)
	Info = Info & vbLf & keywords(2)
	RunBlock = True
End Function

Function RunInterrupt(a)
	If a = 0 Then
		RunInterrupt = True
		Exit Function
	End If
	
	Dim regexPattern, CRCInit, CrcReadValue
	Dim loopCount
	
	Dim cmd, addr
	addr = "flashavl -i /dev/mtd8 "                                       '修改block区域
    
	loopCount = 0
	
	crt.Screen.Send " " & chr(13)
	crt.Screen.WaitForString("MTCMD>> ")
	cmd = addr & "0 1"
	crt.Screen.Send cmd & chr(13)
	
	regexPattern = crt.Screen.ReadString(">.")
	index = crt.Screen.WaitForStrings(keywords)
	Select Case index
		Case 8                                                            'case4出错
			RunInterrupt = False
			Info = Info & vbLf & keywords(7)
			Exit Function
	End Select
	CRCInit = Right(regexPattern, 10)

	For i = 1 To a '修改循环次数
		loopCount = loopCount + 1
		crt.Screen.WaitForString("MTCMD>> ")
		crt.Screen.Send " " & chr(13)
		crt.Screen.WaitForString("MTCMD>> ")
		cmd = addr & "0 1"
		crt.Screen.Send cmd & chr(13)
		index = crt.Screen.WaitForStrings(keywords)
		Select Case index
			Case 8                                                        'case4出错
				RunInterrupt = False
				Info = Info & vbLf & keywords(7)
				Exit Function
		End Select
		If loopCount = Int(a / 2) Then
			crt.Screen.WaitForString("MTCMD>> ")
			cmd = addr & "1 1"
			crt.Screen.Send cmd & chr(13)
			crt.Screen.WaitForString("MTCMD>> ")
			cmd = addr & "0 1"
			crt.Screen.Send cmd & chr(13)
			regexPattern = crt.Screen.ReadString(">.")
			index = crt.Screen.WaitForStrings(keywords)
			Select Case index
				Case 8                                                    'case4出错
					RunInterrupt = False
					Info = Info & vbLf & keywords(7)
					Exit Function
			End Select
			
			CrcReadValue = Right(regexPattern, 10)
			If CrcReadValue <> CRCInit Then
				RunInterrupt = False
				Info = Info & vbLf & keywords(7) & vbLf & "After the interruption, the block is changed"
				Exit Function
			End If
		End If
	Next
	crt.Screen.WaitForString("MTCMD>> ")
	crt.Screen.Send "exit" & chr(13)
	crt.sleep 3000
	crt.Screen.Send "reboot" & chr(13)
	Info = Info & vbLf & keywords(3)
	RunInterrupt = True
End Function

Sub Main
	Dim Skip
	Skip = True
'case1 镜像文件烧录
	RunStatu = RunImg(0, 2)        'param1: 循环次数，填 0 表示跳过执行   param2: 程序内部执行次数
	If Skip <> True Then
		If RunStatu = False Then
			crt.Screen.WaitForString("MTCMD>> ")
			MsgBox Info
			Exit Sub
		End If
	End If
	
'case2 5A和A5数据读写
	RunStatu = RunData(0, 2)       'param1: 循环次数，填 0 表示跳过执行   param2: 程序内部执行次数
	If Skip <> True Then
		If RunStatu = False Then
			crt.Screen.WaitForString("MTCMD>> ")
			MsgBox Info
			Exit Sub
		End If
	End If
	
'case3 测试写入block是否影响其他block
	RunStatu = RunBlock(0, 2)      'param1: 循环次数，填 0 表示跳过执行   param2: 程序内部执行次数
	If Skip <> True Then
		If RunStatu = False Then
			crt.Screen.WaitForString("MTCMD>> ")
			MsgBox Info
			Exit Sub
		End If
	End If
	
'case4 测试写入block过程中中断是否影响其他block
	RunStatu = RunInterrupt(20)     'param: 循环次数，填 0 表示跳过执行
	If Skip <> True Then
		If RunStatu = False Then
			crt.Screen.WaitForString("MTCMD>> ")
			MsgBox Info
			Exit Sub
		End If
	End If
	
	crt.Screen.WaitForString("Welcome to Montage-tech SOC")
	'添加结果输出到打印中
	tsv = vbNewLine & "###############################################" & vbNewLine
	crt.Screen.Send tsv & Info & vbNewLine & tsv, true
	MsgBox Info
End Sub