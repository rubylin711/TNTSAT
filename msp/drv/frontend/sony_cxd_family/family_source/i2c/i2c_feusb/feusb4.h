// $Id: feusb4.h 35482 2015-05-08 07:43:30Z yonaga $

#ifndef FEUSB4_H
#define FEUSB4_H
//-----------------------------------------------------------------------------
//   File    Name:  feusb4.h
//   Project Name:  feusb4
//   Contents    :  General define Header
//   Used Platform : CY7C68013A-100AXC / Win WDM Kernel mode driver / Win32 User mode driver
//
//   $Revision: 35482 $
//   $Date:: 2015-05-08 16:43:30 +0900#$
//   $Author: yonaga $
//
//   Copyright 2005 Sony Corporation
//-----------------------------------------------------------------------------

#ifdef WIN32
//////////////////////////////////
// DeviceIoControl Define Table //
//////////////////////////////////
#define FEUSB_IOCTL_INDEX  0x0000


#define IOCTL_FEUSB_GET_CONFIG_DESCRIPTOR     CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_RESET_DEVICE   CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+1,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_RESET_PIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+2,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_VENDORREQUEST_IN  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+3,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_VENDORREQUEST_OUT  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+4,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_GETDESCRIPTOR  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+5,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)
#define IOCTL_FEUSB_GETSTATUS  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+8,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_ABORT_PIPE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+9,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_GET_DRIVER_VERSION  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+7,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_GET_KERNEL_BUFFER_STATUS  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+10,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

#define IOCTL_FEUSB_IOARRAY  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+6,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

// -- i2cbusy
#define IOCTL_FEUSB_I2CBUSY_STATUS  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                                                   FEUSB_IOCTL_INDEX+11,\
                                                   METHOD_BUFFERED,  \
                                                   FILE_ANY_ACCESS)

/*
Functionの値

URB_FUNCTION_VENDOR_DEVICE
	Indicates the URB is a vendor-defined request for a USB device.
URB_FUNCTION_VENDOR_INTERFACE
	Indicates the URB is a vendor-defined request for an interface on a USB device.
URB_FUNCTION_VENDOR_ENDPOINT
	Indicates the URB is a vendor-defined request for an endpoint, in an interface, on a USB device.
URB_FUNCTION_VENDOR_OTHER
	Indicates the URB is a vendor-defined request for a device-defined target.
URB_FUNCTION_CLASS_DEVICE
	Indicates the URB is a USB-defined class request for a USB device.
URB_FUNCTION_CLASS_INTERFACE
	Indicates the URB is a USB-defined class request for an interface on a USB device.
URB_FUNCTION_CLASS_ENDPOINT
	Indicates the URB is a USB-defined class request for an endpoint, in an interface, on a USB device.
URB_FUNCTION_CLASS_OTHER
	Indicates the URB is a USB-defined class request for a device-defined target.
*/

/////////////////////////////////////////////////////////////////
// DeviceIoControlのInputBufferで使用するGetDescriptorの構造体 //
/////////////////////////////////////////////////////////////////
typedef struct tagPARAM_PARAM_GET_DESCRIPTOR {
	UCHAR	DescriptorType;		// 次のいずれか
								// USB_DEVICE_DESCRIPTOR_TYPE
								// USB_CONFIGURATION_DESCRIPTOR_TYPE
								// USB_STRING_DESCRIPTOR_TYPE

	UCHAR	Index;				// Index
	USHORT	LanguageId;			// DescriptorTypeがUSB_STRING_DESCRIPTOR_TYPE
								// の時のみ有効
} PARAM_GET_DESCRIPTOR;

typedef struct tagPARAM_GET_STATUS {
	USHORT	type;
	USHORT	Index;
} PARAM_GET_STATUS;

/////////////////////////////////////////////////////////////////
// DeviceIoControlのInputBufferで使用するVendorRequestの構造体 //
/////////////////////////////////////////////////////////////////
typedef struct tagPARAM_VENDOR_REQUEST {
	IN USHORT Function;
	IN ULONG TransferFlags;		// 指定長より実際の転送長が短くてもエラー
								// としない場合USBD_SHORT_TRANSFER_OKを指定
	IN UCHAR ReservedBits;
	IN UCHAR Request;
	IN USHORT Value;
	IN USHORT Index;
} PARAM_VENDOR_REQUEST;

#define GET_STATUS_FROM_DEVICE		0
#define GET_STATUS_FROM_INTERFACE	1
#define GET_STATUS_FROM_ENDPOINT	2
#define GET_STATUS_FROM_OTHER		3

/////////////////////////////////////////////////////////////////
// DeviceIoControlのInputBufferで使用する                      //
// VendorRequestコマンドのRQCTL_FEUSB_GET_VERSION_INFOの構造体 //
/////////////////////////////////////////////////////////////////
typedef struct _FEUSB_VERSION_INFO {
    SHORT wFirmwareRevision;
    ULONG dwBoardSerialNumber;
} FEUSB_VERSION_INFO;

///////////////////////////////////////////////////////////////
// DeviceIoControlのIOCTL_FEUSB_GET_DRIVER_VERSIONで使用する //
// OutBufferの構造体                                         //
///////////////////////////////////////////////////////////////
typedef struct _FEUSB_DRIVER_VERSION {
    SHORT   MajorVersion;
    SHORT   MinorVersion;
    SHORT   BuildVersion;
} FEUSB_DRIVER_VERSION, *PFEUSB_DRIVER_VERSION;

/////////////////////////////////////////////////////////////////////
// DeviceIoControlのIOCTL_FEUSB_GET_KERNEL_BUFFER_STATUSで使用する //
// OutBufferの構造体                                               //
/////////////////////////////////////////////////////////////////////
typedef struct _FEUSB_KERNEL_BUFFER_STATUS {
    ULONG   ulBufferingLength;
    ULONG   ulMaxBufferSize;
} FEUSB_KERNEL_BUFFER_STATUS, *PFEUSB_KERNEL_BUFFER_STATUS;

////////////////////////////////////////////////////////////////
// エンドポイント0の転送でVendor Command でデバイスに送る宛先 //
// 及びShort転送フラグ                                        //
////////////////////////////////////////////////////////////////
#define URB_FUNCTION_VENDOR_DEVICE                   0x0017
#define URB_FUNCTION_VENDOR_INTERFACE                0x0018
#define URB_FUNCTION_VENDOR_ENDPOINT                 0x0019
#define URB_FUNCTION_VENDOR_OTHER                    0x0020

#ifndef USBD_SHORT_TRANSFER_OK
#define USBD_SHORT_TRANSFER_OK_BIT            1
#define USBD_SHORT_TRANSFER_OK                (1<<USBD_SHORT_TRANSFER_OK_BIT)
#endif /* USBD_SHORT_TRANSFER_OK */

#endif /* ifndef DEF_FEUSB_FW end */

//////////////////////////////////////////////////////
// DeviceIoControl / FW Vendor Request Define Table //
//////////////////////////////////////////////////////
#define RQCTL_FEUSB_WRITE                         0x20
#define RQCTL_FEUSB_READ                          0x21
#define RQCTL_FEUSB_READWOSUBADR                  0x2A
#define RQCTL_FEUSB_GET_SERIAL_TRANSFER_RESPONSE  0x25
#define RQCTL_FEUSB_GET_VERSION_INFO              0x2F
#define RQCTL_FEUSB_FIFO_RESET                    0x27
#define RQCTL_FEUSB_GPIO_WRITE                    0x40
#define RQCTL_FEUSB_GPIO_READ                     0x41
#define RQCTL_FEUSB_GPIO_WRITEDIR                 0x42
#define RQCTL_FEUSB_GPIO_READDIR                  0x43
#define RQCTL_FEUSB_GUI_ACTIVE                    0x44
#define RQCTL_FEUSB_SET_PB_FUNC                   0x45
#define RQCTL_FDL_DOWNLOAD_ENABLE                 0x30
#define RQCTL_FDL_BUSY_CHK                        0x31
#define RQCTL_FEUSB_READ2                         0x28
#define RQCTL_FEUSB_GENERIC_WRITE                 0x50
#define RQCTL_FEUSB_GENERIC_READ                  0x51
#define RQCTL_FEUSB_EEPROM_WP_ENABLE              0x58
#define RQCTL_FEUSB_SET_TS_DOWNLOAD_MODE          0x59

////////////////////////////////////////////////////////////////////////////////////
// Transfer Response Code Define ( RQCTL_FEUSB_GET_SERIAL_TRANSFER_RESPONSE Res ) //
////////////////////////////////////////////////////////////////////////////////////
#define ERR_IF_OK   0
#define ERR_ACK     1
#define ERR_NACK    2
#define ERR_IF_NG   0xFF

//////////////////////////////////////////////////////////////////
// Transfer Response Code Define ( RQCTL_FEUSB_WRITE/READ/DIR ) //
//////////////////////////////////////////////////////////////////
#define GPIO_PA			0
#define GPIO_PC			1
#define GPIO_PE			2
#define GPIO_PB			3
#define GPIO_PD			4

////////////////////////////////////
// Download Enable/Disable        //
////////////////////////////////////
#define FDL_DOWNLOAD_ENABLE  1
#define FDL_DOWNLOAD_DISABLE 0

#define FDL_DOWNLOAD_WRITE  1
#define FDL_DOWNLOAD_READ   2

////////////////////////////////////////////////////////////////////
// Transfer Response Code Define ( RQCTL_FEUSB_EEPROM_WP_ENABLE ) //
////////////////////////////////////////////////////////////////////
#define EEPROM_WP_DISABLE	(0)
#define EEPROM_WP_ENABLE	(1)


#endif /* FEUSB4_H */
