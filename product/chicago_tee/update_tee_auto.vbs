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
g_Eth_Addr = "86:21:38:41:80:10"
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
g_flashTableCount128 = 10

Dim flashTable128(11, 2)

'btinit+uboot: 4M (0-4)
flashTable128(0,0)=""
flashTable128(0,1)="0"
flashTable128(0,2)="0x400000"
'bootargs: 512K
flashTable128(1,0)="uboot_symphony_demo_unified.scr"
flashTable128(1,1)="0x400000"
flashTable128(1,2)="0x80000"
'syscfg: 512K
flashTable128(2,0)="uboot_symphony_syscfg.scr"
flashTable128(2,1)="0x480000"
flashTable128(2,2)="0x80000"
'teeos: 2M, 8-10
flashTable128(3,0)="teeos.bin"
flashTable128(3,1)="0x800000"
flashTable128(3,2)="0x200000"
'teeos-dtb: 2M (10-12)
flashTable128(4,0)="tee.dtb.bin"
flashTable128(4,1)="0xA00000"
flashTable128(4,2)="0x200000"
'logo: 2M, (14-16)
flashTable128(5,0)="logo.jpg"
flashTable128(5,1)="0xE00000"
flashTable128(5,2)="0x200000"
'dtb: 2M (30-32)
flashTable128(6,0)="symphony6-rawnand-gmac.dtb"
flashTable128(6,1)="0x1E00000"
flashTable128(6,2)="0x200000"
'uImage: 16M (32-48)
flashTable128(7,0)="uImage"
flashTable128(7,1)="0x2000000"
flashTable128(7,2)="0x1000000"
'rootfs: 24M (48-72)
flashTable128(8,0)="rootfs_squashubi.img"
flashTable128(8,1)="0x3000000"
flashTable128(8,2)="0x1800000"
'usrfs: 46M (72-118)
flashTable128(9,0)="usrfs_squashubi.img"
flashTable128(9,1)="0x4800000"
flashTable128(9,2)="0x2E00000"
'data: 8M (118-126)
flashTable128(10,0)="nandubi.img"
flashTable128(10,1)="0x7600000"
flashTable128(10,2)="0x800000"

'------------------------------------Flash Table-----------------------------------'
'
' 256M SPI/RAW Nand Flash Table
' (0): Filename
' (1): Offset
' (2): Length
'
Dim g_flashTableCount256
g_flashTableCount256 = 10

Dim flashTable256(11, 2)

'btinit+uboot: 4M (0-4)
flashTable256(0,0)=""
flashTable256(0,1)="0"
flashTable256(0,2)="0x400000"
'bootargs: 512K
flashTable256(1,0)="uboot_symphony_demo_unified.scr"
flashTable256(1,1)="0x400000"
flashTable256(1,2)="0x80000"
'syscfg: 512K
flashTable256(2,0)="uboot_symphony_syscfg.scr"
flashTable256(2,1)="0x480000"
flashTable256(2,2)="0x80000"
'teeos: 2M, 8-10
flashTable256(3,0)="teeos.bin"
flashTable256(3,1)="0x800000"
flashTable256(3,2)="0x200000"
'teeos-dtb: 2M (10-12)
flashTable256(4,0)="tee.dtb.bin"
flashTable256(4,1)="0xA00000"
flashTable256(4,2)="0x200000"
'logo: 8M, 14-22
flashTable256(5,0)="logo.jpg"
flashTable256(5,1)="0xE00000"
flashTable256(5,2)="0x800000"
'dtb: 2M, 46-48
flashTable256(6,0)="symphony6-rawnand-gmac.dtb"
flashTable256(6,1)="0x2E00000"
flashTable256(6,2)="0x200000"
'uImage: 32M, 48-80
flashTable256(7,0)="uImage"
flashTable256(7,1)="0x3000000"
flashTable256(7,2)="0x2000000"
'rootfs: 64M (80-144)
flashTable256(8,0)="rootfs.img"
flashTable256(8,1)="0x5000000"
flashTable256(8,2)="0x4000000"
'usrfs: 88M (144-232)
flashTable256(9,0)="usrfs.img"
flashTable256(9,1)="0x9000000"
flashTable256(9,2)="0x5800000"
'data: 8M (232-240)
flashTable256(10,0)="nandubi.img"
flashTable256(10,1)="0xE800000"
flashTable256(10,2)="0x800000"

' Burn 128M/256M SPI/RAW Nand Flash all Partitions
Sub burnNandFlashAll()

	Dim i
	Dim bootImage

	If g_Flash_Type = "SPINAND" Then
		'erase all
		crt.Screen.Send "snf scrub" & vbcr
		crt.Screen.WaitForString "spi nand scrub successfaully!"
		crt.Sleep 100
	End If

	'btinit+uboot: 4M (0-4)
	If g_Flash_Type = "SPINAND" Then
		' SPI Nand
		If g_DDR_Type = "DDR4" Then
			bootImage = "CHIP_OF/OTP_FTA_TEE/symphony6_pkg_ddr4_2133_2DCDCxHz_spi_nand_tee_all.img"
		Else
			bootImage = "CHIP_OF/OTP_FTA_TEE/symphony6_pkg_ddr3_1866_2DCDCxHz_spi_nand_tee_all.img"
		End If
	ElseIf g_Flash_Type = "RAWNAND" Then
		' RAW Nand
		If g_DDR_Type = "DDR4" Then
			bootImage = "CHIP_OF/OTP_FTA_TEE/symphony6_pkg_ddr4_2133_2DCDCxHz_nand_3byte_2k_tee_all.img"
		Else
			bootImage = "CHIP_OF/OTP_FTA_TEE/symphony6_pkg_ddr3_1866_2DCDCxHz_nand_3byte_2k_tee_all.img"
		End If
	Else
		'Error
	End If

	If g_Flash_Size = "128" Then
		' 128M SPI/RAW Nand Flash
		flashTable128(0, 0) = bootImage

		For i = 0 To g_flashTableCount128
			Call loadFile(flashTable128(i, 0))
			Call burnNandFlash(flashTable128(i, 1), flashTable128(i, 2))
		Next
	Else
		' 256M SPI/RAW Nand Flash
		flashTable256(0, 0) = bootImage

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
g_emmcTableCount = 9

Dim emmcTable(10, 2)

'bootargs: 512K
emmcTable(0,0)="uboot_symphony_demo_unified.scr"
emmcTable(0,1)="0x400000"
emmcTable(0,2)="0x80000"
'syscfg: 512K
emmcTable(1,0)="uboot_symphony_syscfg.scr"
emmcTable(1,1)="0x480000"
emmcTable(1,2)="0x80000"
'teeos: 2M, 8-10
emmcTable(2,0)="teeos.bin"
emmcTable(2,1)="0x800000"
emmcTable(2,2)="0x200000"
'teeos-dtb: 2M (10-12)
emmcTable(3,0)="tee.dtb.bin"
emmcTable(3,1)="0xA00000"
emmcTable(3,2)="0x200000"
'logo: 8M, 14-22
emmcTable(4,0)="logo.jpg"
emmcTable(4,1)="0xE00000"
emmcTable(4,2)="0x800000"
'dtb: 2M, 46-48
emmcTable(5,0)="symphony6-emmc-gmac.dtb"
emmcTable(5,1)="0x2E00000"
emmcTable(5,2)="0x200000"
'uImage: 32M, 48-80
emmcTable(6,0)="uImage"
emmcTable(6,1)="0x3000000"
emmcTable(6,2)="0x2000000"
'rootfs: 128M, 80-208
emmcTable(7,0)="rootfs.ext4"
emmcTable(7,1)="0x5000000"
emmcTable(7,2)="0x8000000"
'usrfs: 128M, 208-336
emmcTable(8,0)="usrfs.ext4"
emmcTable(8,1)="0xD000000"
emmcTable(8,2)="0x8000000"
'data: 128M, 336-464
emmcTable(9,0)="datafs.ext4"
emmcTable(9,1)="0x15000000"
emmcTable(9,2)="0x8000000"

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
			Call burnSpiNorFlash_16M()
		Else
			'Error
		End If

	End If

End Sub

