#$language = "VBScript"
#$interface = "1.0"

crt.Screen.Synchronous = True

'-----------------------------------------Global Variables------------------------------------'

' DDR Type
Dim g_DDR_Type
g_DDR_Type = "DDR3"
'g_DDR_Type = "DDR4"

' Flash Type
Dim g_Flash_Type
g_Flash_Type = "SPINAND"
'g_Flash_Type = "RAWNAND"
'g_Flash_Type = "EMMC"
'g_Flash_Type = "SPINOR"
'g_Flash_Type = "UNKNOWN"

' SPI Nand, PNand Flash Size
Dim g_Flash_Size
g_Flash_Size = "256"
'g_Flash_Size = "128"

' Storage Type
Dim g_Storage_Type
g_Storage_Type = "USB"
'g_Storage_Type = "TFTP"

' Ethernet for TFTP
Dim g_Eth_Addr
Dim g_Gateway
Dim g_Server_IP
g_Eth_Addr = "86:21:38:41:80:02"
g_Gateway = "192.168.8.1"
g_Server_IP = "192.168.8.128"

' Storage found?
Dim g_Storage_Found
g_Storage_Found = "FALSE"

' load file result
Dim g_Load_File_Result
'g_Load_File_Result = "SUCCESS"
g_Load_File_Result = "FAIL"

'----------------------------------------Intenal Functions-------------------------------------'

' Get DDR Type
' @retval
'   DDR3, DDR4
Sub getDDRType()
	crt.Screen.Send "pri ddr_type_string" & vbcr
	crt.Screen.WaitForString "ddr_type_string="				'从 "ddr_type_string=" 起
	strResult = crt.Screen.ReadString("U-Boot >")			'到 "U-Boot >" 截止
	If InStr(strResult, "DDR4") > 0  Then
		g_DDR_Type = "DDR4"
	Else
		g_DDR_Type = "DDR3"
	End If
	crt.Screen.Send "echo DDR Type: " & g_DDR_Type & vbcr
End Sub

' Get Flash Type
' @retval
'   SPINAND, RAWNAND, EMMC, SPINOR, UNKNOWN
Sub getFlashType()
	crt.Screen.Send "pri bootmedia" & vbcr
	crt.Screen.WaitForString "bootmedia="					'从 "bootmedia=" 起
	strResult = crt.Screen.ReadString("U-Boot >")			'到 "U-Boot >" 截止
	If InStr(strResult, "spi_nand") > 0  Then
		g_Flash_Type = "SPINAND"
	ElseIf InStr(strResult, "pnand") > 0  Then
		g_Flash_Type = "RAWNAND"
	ElseIf InStr(strResult, "emmc") > 0  Then
		g_Flash_Type = "EMMC"
	ElseIf InStr(strResult, "spi_nor") > 0  Then
		g_Flash_Type = "SPINOR"
	Else
		g_Flash_Type = "UNKNOWN"
		MsgBox "Unknown Flash Type!"
	End If
	crt.Screen.Send "echo Flash Type: " & g_Flash_Type & vbcr
End Sub

' Get Flash Size
' @retval
'   128, 256
Sub getFlashSize()
	crt.Screen.Send "pri flash_size" & vbcr
	crt.Screen.WaitForString "flash_size="					'从 "flash_size=" 起
	strResult = crt.Screen.ReadString("U-Boot >")			'到 "U-Boot >" 截止
	If InStr(strResult, "128") > 0  Then
		g_Flash_Size = "128"
	Else
		g_Flash_Size = "256"
	End If
	crt.Screen.Send "echo Flash Size: " & g_Flash_Size & vbcr
End Sub

'-----------------------------------------------------------------------------'

' Setup USB ENV
Sub setupUsbEnv()
	crt.Screen.Send "usb stop" & vbcr
	crt.Screen.WaitForString "U-Boot >"
	crt.Sleep 100

	crt.Screen.Send "usb start" & vbcr
	crt.Screen.WaitForString "scanning usb for storage devices..."		'从 "scanning usb for storage devices..." 起
	strResult = crt.Screen.ReadString("U-Boot >")						'到 "U-Boot >" 截止

	'MsgBox strResult
	If InStr(strResult, "0 Storage Device(s) found") > 0 Then
		g_Storage_Found = "FALSE"
		MsgBox "USB Storage Not Found!"
	Else
		g_Storage_Found = "TRUE"
	End If
End Sub

' Setup TFTP ENV
Sub setupTftpEnv()
	crt.Screen.Send "setenv ethaddr " & g_Eth_Addr & vbcr
	crt.Screen.Send "setenv gatewayip " & g_Gateway & vbcr
	crt.Screen.Send "setenv serverip " & g_Server_IP & vbcr

	crt.Screen.Send "dhcp" & vbcr

	crt.Screen.WaitForString "BOOTP broadcast 1"		'从 "BOOTP broadcast" 起
	strResult = crt.Screen.ReadString("U-Boot >")		'到 "U-Boot >" 截止

	'MsgBox strResult
	If InStr(strResult, "DHCP client bound to address") > 0 Then
		g_Storage_Found = "TRUE"
	Else
		g_Storage_Found = "FALSE"
		MsgBox "DHCP Timeout!"
	End If
End Sub

' Setup ENV for USB or TFTP
Sub setupEnv()
	If g_Storage_Type = "USB" Then
		Call setupUsbEnv()
	ElseIf g_Storage_Type = "TFTP" Then
		Call setupTftpEnv()
	Else
		g_Storage_Found = "FALSE"
		MsgBox "Setup other Storage not Support!"
	End If
End Sub

' fatload file from USB storage
Sub usbLoadFile(filename)
	crt.Screen.Send "fatload usb 0 0x03000000 " & filename & vbcr

	crt.Screen.WaitForString "bytes read"
	g_Load_File_Result = "SUCCESS"
	crt.Sleep 100

	'crt.Screen.WaitForString "fatload"
	'strResult = crt.Screen.ReadString("U-Boot >")
	'MsgBox strResult
	'If InStr(strResult, "bytes read") > 0 Then
	'	g_Load_File_Result = "SUCCESS"
	'Else
	'	g_Load_File_Result = "FAIL"
	'	MsgBox "Load File " & filename & " Failed!"
	'End If
End Sub

' TFTP load file
Sub tftpLoadFile(filename)
	crt.Screen.Send "tftp 0x03000000 " & filename & vbcr

	'crt.Screen.WaitForString "Bytes transferred"
	'g_Load_File_Result = "SUCCESS"
	'crt.Sleep 100

	crt.Screen.WaitForString "Loading"
	strResult = crt.Screen.ReadString("U-Boot >")
	'MsgBox strResult
	If InStr(strResult, "Bytes transferred") > 0 Then
		g_Load_File_Result = "SUCCESS"
	Else
		g_Load_File_Result = "FAIL"
		MsgBox "Load File " & filename & " Failed!"
	End If
End Sub

' Load file form USB or TFTP
Sub loadFile(filename)
	If g_Storage_Type = "USB" Then
		usbLoadFile(filename)
	ElseIf g_Storage_Type = "TFTP" Then
		tftpLoadFile(filename)
	Else
		g_Load_File_Result = "FAIL"
		MsgBox "Load File from other Storage not Support!"
	End If
End Sub

'-----------------------------------------------------------------------------'

' Burn SPI Nand Flash Partition
Sub burnSpiNandFlash(Offset, Length)
	crt.Screen.Send "snf erase " & Offset & " " & Length & vbcr
	crt.Screen.WaitForString "snf erase successfully"
	crt.Sleep 100
	crt.Screen.Send "snf burn 0x03000000 " & Offset & " $filesize" & vbcr
	crt.Screen.WaitForString "snf burn successfully"
	crt.Sleep 100
End Sub

' Burn RAW Nand Flash Partition
Sub burnRawNandFlash(Offset, Length)
	crt.Screen.Send "nand erase " & Offset & " " & Length & vbcr
	crt.Screen.WaitForString "U-Boot >"
	crt.Sleep 100
	crt.Screen.Send "nand write 0x03000000 " & Offset & " $filesize" & vbcr
	crt.Screen.WaitForString "written: OK"
	crt.Sleep 100
End Sub

' Burn EMMC with $filesize
Sub burnEmmc_byFilesize(Offset, Length)
	crt.Screen.Send "mt_mmc erase " & Offset & " " & Length & vbcr
	crt.Screen.WaitForString "blocks erased: OK"
	crt.Sleep 100
	crt.Screen.Send "mt_mmc write 0x03000000 " & Offset & " $filesize" & vbcr
	crt.Screen.WaitForString "blocks written: OK"
	crt.Sleep 100
End Sub

' Burn SPI/RAW Nand Flash
Sub burnNandFlash(Offset, Length)
		If g_Flash_Type = "SPINAND" Then
			Call burnSpiNandFlash(Offset, Length)
		ElseIf g_Flash_Type = "RAWNAND" Then
			Call burnRawNandFlash(Offset, Length)
		Else
			'Error
		End If
End Sub

'------------------------------------Flash Table-----------------------------------'
'
' 128M SPI/RAW Nand Flash Table
' (0): Filename
' (1): Offset
' (2): Length
'
Dim g_flashTableCount128
g_flashTableCount128 = 2

Dim flashTable128(3, 2)

'loaderdtb: 2M, 16-18
flashTable128(0,0)="symphony6-rawnand-gmac.dtb"
flashTable128(0,1)="0x1000000"
flashTable128(0,2)="0x200000"
'otaloader: 4M, 18-22
flashTable128(1,0)="uImage-loader"
flashTable128(1,1)="0x1200000"
flashTable128(1,2)="0x400000"
'loader rootfs: 8M, 22-30
flashTable128(2,0)="loader_rootfs_squashubi.img"
flashTable128(2,1)="0x1600000"
flashTable128(2,2)="0x800000"

'------------------------------------Flash Table-----------------------------------'
'
' 256M SPI/RAW Nand Flash Table
' (0): Filename
' (1): Offset
' (2): Length
'
Dim g_flashTableCount256
g_flashTableCount256 = 2

Dim flashTable256(3, 2)

'loaderdtb: 2M, 22-24
flashTable256(0,0)="symphony6-rawnand-gmac.dtb"
flashTable256(0,1)="0x1600000"
flashTable256(0,2)="0x200000"
'otaloader: 10M, 24-34
flashTable256(1,0)="uImage-loader"
flashTable256(1,1)="0x1800000"
flashTable256(1,2)="0xA00000"
'loader rootfs: 12M, 34-46
flashTable256(2,0)="loader_rootfs_squashubi.img"
flashTable256(2,1)="0x2200000"
flashTable256(2,2)="0xC00000"

' Burn 128M/256M SPI/RAW Nand Flash all Partitions
Sub burnNandFlashAll()

	Dim i

	If g_Flash_Size = "128" Then
		For i = 0 To g_flashTableCount128
			Call loadFile(flashTable128(i, 0))
			Call burnNandFlash(flashTable128(i, 1), flashTable128(i, 2))
		Next
	Else
		For i = 0 To g_flashTableCount256
			Call loadFile(flashTable256(i, 0))
			Call burnNandFlash(flashTable256(i, 1), flashTable256(i, 2))
		Next
	End If

End Sub

'------------------------------------EMMC Table------------------------------------'
'
' (0): Filename
' (1): Offset
' (2): Length
'
Dim g_emmcTableCount
g_emmcTableCount = 2

Dim emmcTable(3, 2)

'loaderdtb: 2M, 22-24
emmcTable(0,0)="symphony6-emmc-gmac.dtb"
emmcTable(0,1)="0x1600000"
emmcTable(0,2)="0x200000"
'otaloader: 10M, 24-34
emmcTable(1,0)="uImage-loader"
emmcTable(1,1)="0x1800000"
emmcTable(1,2)="0xA00000"
'loader rootfs: 12M, 34-46
emmcTable(2,0)="loader_rootfs.ext4"
emmcTable(2,1)="0x2200000"
emmcTable(2,2)="0xC00000"

' Burn EMMC
Sub burnEmmcFlash()

	crt.Screen.Send "mt_mmc dev 0" & vbcr
	crt.Screen.WaitForString "mmc0(part 0) is current device"
	crt.Sleep 100

	Dim i
	For i = 0 To g_emmcTableCount
		Call loadFile(emmcTable(i, 0))
		Call burnEmmc_byFilesize(emmcTable(i, 1), emmcTable(i, 2))
	Next

End Sub

' Burn 16M SPI NOR Flash
Sub burnSpiNorFlash_16M()

	Call loadFile("flash_ddr3_16M_spinor.bin")

	crt.Screen.Send "sf probe" & vbcr
	crt.Screen.WaitForString "U-Boot >"
	crt.Sleep 100

	crt.Screen.Send "sf burn 0x3000000 0 0x1000000" & vbcr
	crt.Screen.WaitForString "U-Boot >"
	crt.Sleep 100

End Sub

'---------------------------------------Main--------------------------------------'

' Test Load File
Sub testLoadFile()
	'Call loadFile("logo.jpg")
	Call loadFile("testfile.dat")
End Sub

' Test Main
Sub TestMain()
	'Call setupEnv()
	Call testLoadFile()
End Sub

Sub Main()

	Call setupEnv()

	If g_Storage_Found = "TRUE" Then
		Call getDDRType()
		Call getFlashType()

		If g_Flash_Type = "EMMC" Then
			'no flash_size env
		Else
			Call getFlashSize()
		End If

		If (g_Flash_Type = "SPINAND") Or (g_Flash_Type = "RAWNAND") Then
			Call burnNandFlashAll()
		ElseIf g_Flash_Type = "EMMC" Then
			Call burnEmmcFlash()
		ElseIf g_Flash_Type = "SPINOR" Then
			'TODO
			'Call burnSpiNorFlash_16M()
		Else
			'Error
		End If

	End If

End Sub

