/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_MISC_H__
#define __DRV_MISC_H__

/**
 @deprecate
 */
#if 0
#define SW_PIN_COUNT	106

/*!
 symphony PINMUX reg
  */
enum
{
  /*!
    0.	SW_PIN0, symphony SW_PIN_BASE + 0x00
    */
  SW_PIN0 = 0,
  /*!
    1.	SW_PIN1, symphony SW_PIN_BASE + 0x04
    */
  SW_PIN1 ,
   /*!
    2.	SW_PIN2, symphony SW_PIN_BASE + 0x08
    */
  SW_PIN2 ,
  /*!
    3.	SW_PIN3, symphony SW_PIN_BASE + 0x0c
    */
  SW_PIN3 ,
   /*!
    4.	SW_PIN4, symphony SW_PIN_BASE + 0x10
    */
  SW_PIN4 ,
     /*!
    5.	SW_PIN5, symphony SW_PIN_BASE + 0x14
    */
  SW_PIN5 ,
     /*!
    6.	SW_PIN5, symphony SW_PIN_BASE + 0x18
    */
  SW_PIN6 ,
     /*!
    7.	SW_PIN5, symphony SW_PIN_BASE + 0x1c
    */
  SW_PIN7,

  /* Symphony4 */
  SW_PIN_MAX = SW_PIN0 + SW_PIN_COUNT,

  /*!
    8.	AO_PIN0, symphony AO_PIN_BASE + 0x00
    */
  AO_PIN0 = 0x100,
    /*!
    9.	AO_PIN1, symphony AO_PIN_BASE+ 0x04
    */
  AO_PIN1 ,

  /* Symphony4 */
  AO_PIN2,
  AO_PIN3,
  AO_PIN4,
  AO_PIN5,
  AO_PIN6,
  AO_PIN7,
  AO_PIN8,

  AO_PIN_MAX,

  /*!
    10.	I2C_PIN2, symphony1/2 I2C_PIN_BASE + 0x00
    */
  I2C_PIN0 = 0x200,

  /* Symphony4 */
  SMC_PIN0 = 0x300

};

/*!
 symphony PINMUX application
  */
enum
{
  /*!
    0.	DVB_S2_Only,  symphony 144/128s/SIP68s support
    */
  PINMUX_DVB_S2_Only = 0,
  /*!
    1.	S2INT_T2,  symphony 144 support
    */
  PINMUX_S2INT_T2 ,
   /*!
    2.	S2EXT_T2,  symphony 144/128s/SIP68s  support
    */
  PINMUX_S2EXT_T2 ,
  /*!
    3.	T2_T2 ,   symphony 144/128s support
    */
  PINMUX_T2_T2 ,
   /*!
    4.	DVB_C symphony SIP68C support
    */
  PINMUX_DVB_C
};

/*!
 symphony PINMUX application
  */
enum{
   /*!
    0.	PINMUX debug config for JTAG function
    */
    PINMUX_DBG_JTAG = 0,
   /*!
    1.	Max pinmux debug macro, not used
    */
    PINMUX_DBG_MAX
};
#endif

struct drv_misc_ioctl{
    unsigned long id;
    unsigned int offset;
    unsigned int len;
    unsigned long val;
};

/**
 @deprecate
 */
#if 0
typedef enum
{
  /*!
    symphony1
    */
  CHIP_SYMPHONY1 = 8,
  /*!
    symphony2
    */
  CHIP_SYMPHONY2,
  /*!
    symphony3
    */
  CHIP_SYMPHONY3,
}chip_family_t;


typedef struct _chip_feature
{
	/*
	Product type
	0:DVB-S/S2/S2X+C
	1:DVB-C
	2:DVB-T/T2+C
	3:S/S2/S2X+T/T2+C
	255:invalid
	*/
	unsigned char type;
	/*
	Chip family
	CHIP_SYMPHONY1:symphony1
	CHIP_SYMPHONY2:symphony2
	CHIP_SYMPHONY3:symphony3
	255:invalid
	*/
	unsigned char family;
	/*
	ChipGenernation
	0:A0
	1:A1
	2:A2
	255:invalid
	*/
	unsigned char genernation;
	/*
	DisplayResolution
	0:SD/HD H.264
	1:HD H.264
	2:HD HEVC 8bits
	3:HD HEVC 10bits
	5:HD HEVC HDR
	6:4K
	7:4K Premium
	8:8K
	255:invalid
	*/
	unsigned char display;
	/*
	Security Type
	0:FTA
	4:Advanced CA
	8:DRM
	c:Advanced CA + DRM
	2:2203 H265 10bit + ETH Disable
	255:invalid
	*/
	unsigned char security;
	/*
	PackageInfo
	0:LQFP
	1:TQFP
	2:QFN
	3:MQFN
	4:TFBGA
	5:FCCSP
	255:invalid
	*/
	unsigned char package;
	/*
	SipDRAMSize
	0:Non-SIP
	1:256Mb
	2:512Mb
	3:1Gb
	4:2Gb
	5:4Gb
	b:DDR3 1Gb
	255:invalid
	*/
	unsigned char sipdramsize;
	/*
	CAVendor
	0:No CA
	1:Nagra
	2:Conax
	3:CTI
	4:ABV
	5:VO
	6:Panaccess
	7:Verimatrix
	255:invalid
	*/
	unsigned char cavendor;
	/*
	CAVersion
	255:invalid
	*/
	unsigned char caversion;
	/*
	Dolby Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char dolbyenable;
	/*
	DRMInfo
	0:No DRM
	1:Playready
	2:Widevine
	3:Marlin
	4:All
	255:invalid
	*/
	unsigned char drminfo;
	/*
	IPLicense
	0:No limitation for any License
	3:Dolby+DTS
	5:Dolby+Macrovision
	7:Dolby
	8:Dolby vision+DTS+Macrovision
	9:DTS+Macrovision
	10:Dolby vision+DTS
	11:DTS
	13:Macrovision
	14:Dolby vision
	15:No-Dolby,all disable
	255:invalid
	*/
	unsigned char iplicense;
	/*
	HDDec Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char hddecenable;
	/*
	DRA Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char draenable;
	/*
	H65 Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char h265;
	/*
	Ethernet Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char ethernet;
	/*
	Internal ephy Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char ephy;
	/*
	HD Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char hd;
	/*
	macrovision
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char macrovision;
	/*
	Usb port count
	255:invalid
	*/
	unsigned char usbcount;
	/*
	Usb port0 Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char usb0;
	/*
	Usb port1 Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char usb1;
	/*
	HDR Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char hdr;
	/*
	10bit vdec Enable
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char vdec10b;
	/*
	J83B
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char j83b;
	/*
	Internal C demod
	1:Open
	0:Close
	255:invalid
	*/
	unsigned char dvbc;
	/*
	Internal S demod
	bit0:1:S/S2/S2X 0:Close
	bit1:1: S2X 0:Close
	255:invalid
	*/
	unsigned char dvbs;
	/*
	Internal T demod
	bit0:1:T 0:Close
	bit1:1:T2	0:Close
	255:invalid
	*/
	unsigned char dvbt;
	/*
	dram kgd info
	*/
	unsigned char dramkgd;
	/*
	Internal Chip ID
	*/
	unsigned int inter_chip_id;
	/*
	Fuse_version
	*/
	unsigned int version;
}chip_feature_t;

#endif

#define MISC_IOC_BASE	'M'

#define MISC_IOCTL_PINMUX_PRESET			_IO(MISC_IOC_BASE, 0)
#define MISC_IOCTL_PINMUX_SET				_IOW(MISC_IOC_BASE, 1, struct drv_misc_ioctl)
#define MISC_IOCTL_PINMUX_GET				_IOWR(MISC_IOC_BASE, 2, struct drv_misc_ioctl)
#define MISC_IOCTL_MODULE_ONOFF				_IOW(MISC_IOC_BASE, 3, struct drv_misc_ioctl)
#define MISC_IOCTL_MODULE_RESET				_IOW(MISC_IOC_BASE, 4, struct drv_misc_ioctl)
//#define MISC_IOCTL_MODULE_DBG				_IO(MISC_IOC_BASE, 5)
#define MISC_IOCTL_MODULE_CLK_SET			_IOW(MISC_IOC_BASE, 6, struct drv_misc_ioctl)
#define MISC_IOCTL_MODULE_CLK_GET			_IOWR(MISC_IOC_BASE, 7, struct drv_misc_ioctl)
#define MISC_IOCTL_MODULE_CLK_IS_ENABLED	_IOWR(MISC_IOC_BASE, 8, struct drv_misc_ioctl)
#define MISC_IOCTL_REG_SET					_IOW(MISC_IOC_BASE, 9, struct drv_misc_ioctl)
#define MISC_IOCTL_REG_GET					_IOWR(MISC_IOC_BASE, 10, struct drv_misc_ioctl)
#define MISC_IOCTL_TEMPERATURE_GET			_IOWR(MISC_IOC_BASE, 11, struct drv_misc_ioctl)
#define MISC_IOCTL_CHIP_PRODUCTINFO_GET		_IOR(MISC_IOC_BASE, 12, char)
#define MISC_IOCTL_PINMUX_ODE_SET			_IOWR(MISC_IOC_BASE, 13, struct drv_pin_ode)
#endif


