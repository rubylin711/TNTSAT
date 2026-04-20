/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************
  File Name     : mt_mpi_priv_edid.h
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2015/12/25
  Description   :
  History       :
  1.Date        : 2015/12/25
    Author      :
    Modification:

*********************************************************************************************/
#ifndef __MPI_PRIV_EDID_H__
#define __MPI_PRIV_EDID_H__

#include <linux/types.h>

#include "mt_unf_common.h"
#include "mt_unf_disp.h"
#include "mt_mpi_edid.h"
#include "list.h"



/*Detailed Timing Definitions Stereo Viewing Support*/



/*==================================================================*/
/*==================================================================*/
/*==================================================================*/


#define EDID_LENGTH 128
#define DDC_ADDR 0x50

/* Define Bit Field */
#define BIT0    0x01
#define BIT1    0x02
#define BIT2    0x04
#define BIT3    0x08
#define BIT4    0x10
#define BIT5    0x20
#define BIT6    0x40
#define BIT7    0x80

#define AUDIO_DATA_BLOCK				0x01
#define VIDEO_DATA_BLOCK				0x02
#define VENDOR_SPECIFIC_DATA_BLOCK		0x03
#define SPEAKER_ALLOCATION_DATA_BLOCK	0x04
#define VESA_DTC_DATA_BLOCK				0x05
#define USE_EXTENDED_TAG				0x07

struct EST_TIMING_S
{
    mt_u8 u8T1;
    mt_u8 u8T2;
    mt_u8 u8MfgRsvd;
} __attribute__((packed));

/* 00=16:10, 01=4:3, 10=5:4, 11=16:9 */
#define EDID_TIMING_ASPECT_SHIFT 6
#define EDID_TIMING_ASPECT_MASK  (0x3 << EDID_TIMING_ASPECT_SHIFT)

/* need to add 60 */
#define EDID_TIMING_VFREQ_SHIFT  0
#define EDID_TIMING_VFREQ_MASK   (0x3f << EDID_TIMING_VFREQ_SHIFT)


/* If detailed data is pixel timing */
struct DETAILED_PIXEL_TMING_S
{
    mt_u8 u8HactiveL;
    mt_u8 u8HblankL;
    mt_u8 u8HactiveHblankH;
    mt_u8 u8VactiveL;
    mt_u8 u8VblankL;
    mt_u8 u8VactiveVblankH;
    mt_u8 u8HsyncOffsetL;
    mt_u8 u8HsyncPulseWidthL;
    mt_u8 u8VsyncOffsetPulseWidthL;
    mt_u8 u8HsyncVsyncOffsetPulseWidthH;
    mt_u8 u8WidthMmL;
    mt_u8 u8HeightMmL;
    mt_u8 u8WidthHeightHmH;
    mt_u8 u8Hborder;
    mt_u8 u8Vborder;
    mt_u8 u8Misc;
} __attribute__((packed));

/* If it's not pixel timing, it'll be one of the below */

struct COLOR_POINT_DATA_S
{
    mt_u8 u8WIndex1;
    mt_u8 u8WpIndex1[3];
    mt_u8 u8Gamma1;
    mt_u8 u8WIndex2;
    mt_u8 u8WpIndex2[3];
    mt_u8 u8Gamma2;
    mt_u8 u8Res[3];           /*Reserved*/
} __attribute__((packed));

/*Established Timings III*/
struct ESTABLISHED_TMING_DATA_S
{
    mt_u8 u8ResData;
    mt_u8 u8Timing[6];
    mt_u8 u8Res[6];

} __attribute__((packed));

struct CVT_TMING_DATA_S
{
    mt_u8 Value_L;              /*Eight Least Significant Bits*/
    mt_u8 ResDate1  : 2;     /*[1..0] Reserved Bits*/
    mt_u8 AR        : 2;     /*[3..2] Aspect Ratio*/
    mt_u8 Value_H       : 4;     /*[7..4] Four Most Significant Bits*/
    mt_u8 VerticalBlackSupport        : 5;     /*[4..0] Supported Vertical Rate and Blanking Style*/
    mt_u8 PrefVerticalRate       : 2;     /*[6..5] Preferred Vertical Rate*/
    mt_u8 ResDate2  : 1;     /*[7] Reserved Bits*/

} __attribute__((packed));

/*CVT Support Definition*/
struct CVT_TMING_DATA_FIELD_S
{
    mt_u8               u8Version;
    struct CVT_TMING_DATA_S  stTimingDataCVT[4];

} __attribute__((packed));


/*GTF Secondary Curve Block Definition*/
struct GTF_DEFINE_DATA_S
{
    mt_u8   u8HFreqStart_KHz;     /* need to multiply by 2 */
    mt_u8   u8C;                   /* need to divide by 2 */
    mt_u16  u16M;
    mt_u8   u8K;
    mt_u8   u8J;                   /* need to divide by 2 */

} __attribute__((packed));


/*CVT Support Definition*/
struct CVT_DEFINE_DATA_S
{
    mt_u8 u8Version;
    mt_u8 PixPerLineMaxH : 2;     /*[1..0]Maximum Active Pixels per Line 2 bit,Most Significant Bits*/
    mt_u8 PixClkPrecision        : 6;     /*[7..2]Pixel Clock Precision:*/
    mt_u8 PixPerLineMaxL;           /*Maximum Active Pixels per Line - Least Significant Bits:*/
    mt_u8 ResData1   : 3;     /*[2..0]Reserved Bits*/
    mt_u8 ARSupport        : 5;     /*Supported Aspect Ratios*/
    mt_u8 ResData2   : 3;     /*[2..0]Reserved Bits*/
    mt_u8 BlankSupport         : 2;     /*[4..3]Standard CVT Blanking is supported*/
    mt_u8 PreferredAR        : 3;     /*[7..5]Preferred Aspect Ratio:*/
    mt_u8 ResData3   : 4;     /*[3..0]Reserved Bits*/
    mt_u8 ScalSupport        : 4;     /*[7..4]Type of Display Scaling Supported*/
    mt_u8 PreferredRefRate;             /*Preferred Vertical Refresh Rate*/

} __attribute__((packed));

/*HDMI Support Definition*/
struct HDMI_DEFINE_DATA_S
{
    mt_u8   Len         : 5;
    mt_u8   Type          : 3;     /*Type of Data Block*/
    mt_u16  u16Idl;                 /*Identifier (0x000C03): 0c03*/
    mt_u8   u16Idh;                 /*Identifier (0x000C03): 00*/
    mt_u16  u16CEC_ID;              /*CEC ID ABCD*/
    mt_u8   DVI_Dual    : 1;     /*Sink supports DVI dual-link operation*/
    mt_u8   ResData1    : 2;     /*[2..0]Reserved Bits*/
    mt_u8   bDC_Y444     : 1;     /*supports YCBCR 4:4:4 in Deep Color modes*/
    mt_u8   bDC_30Bit    : 1;     /*Sink supports 30 bits/pixel (10 bits/color)*/
    mt_u8   bDC_36Bit    : 1;     /*Sink supports 36 bits/pixel (12 bits/color)*/
    mt_u8   bDC_48Bit    : 1;     /*Sink supports 48 bits/pixel (16 bits/color)*/
    mt_u8   bAI          : 1;     /*Sink supports 48 bits/pixel (16 bits/color)*/
    mt_u8   u8TMD_ClkMax;                 /*TMDS_ClkMax*/
    mt_u8   CNC         : 4;     /*[3..0]CNC0: Graphics; CNC1: Photo; CNC2: Cinema; CNC3: Game ; */
    mt_u8   ResData2    : 1;     /*[4]Reserved Bits*/
    mt_u8   bHvp         : 1;     /*[5]HDMI Video present*/
    mt_u8   bIle         : 1;     /*[6]interlaced Latency Fields_Present*/
    mt_u8   bLe          : 1;     /*[7]Latency Fields Present*/

} __attribute__((packed));

/*Display Range Limits & Timing Descriptor Block Definition*/
struct DETAIL_MONITOR_RANGE_DATA_S
{
    mt_u8 u8VFreqMin;
    mt_u8 u8VFreqMax;
    mt_u8 u8HFreqMin;
    mt_u8 u8HFreqMax;
    mt_u8 u8PixClk_MHz;              /* need to multiply by 10 */
    mt_u8 u8Indicate;                     /* indicate :0x20 Secondary GTF supported; :0x40 CVT supported*/
    union
    {
        struct GTF_DEFINE_DATA_S  stGTFInfo;
        struct CVT_DEFINE_DATA_S  stCVTInfo;

    } data;
} __attribute__((packed));

#define DETAILED_STRING_LEN  13
typedef struct tagEDID_DETAILED_STRING_DATA_S
{
    mt_char string[DETAILED_STRING_LEN];
} __attribute__((packed)) EDID_DETAILED_STRING_DATA_S;

struct DETAILED_NON_PIXEL_S
{
    mt_u8 u8Pad1;
    mt_u8 u8Type;     /* ff=serial, fe=string, fd=monitor range, fc=monitor name
		            fb=color point data, fa=standard timing data,
		            f9=undefined, f8=mfg. reserved */
    mt_u8 u8Pad2;
    union
    {
        EDID_DETAILED_STRING_DATA_S	        stString;
        struct DETAIL_MONITOR_RANGE_DATA_S 	    stDetailFreqRange;
        struct COLOR_POINT_DATA_S	                stColor;
        EDID_STD_TIMING_S		        stSTDTiming[6];
        struct CVT_TMING_DATA_FIELD_S	        stCVTTiming;
        struct ESTABLISHED_TMING_DATA_S             stEstTiming;
        EDID_DETAILED_COLOR_MANAGEMENT_S stColorManage;
    } data;
} __attribute__((packed));



struct DETAILED_TIMING_DATA
{
    mt_u16 u16PixClk; /* need to multiply by 10 KHz */
    union
    {
        struct DETAILED_PIXEL_TMING_S    stDetailTimingData;
        struct DETAILED_NON_PIXEL_S       stOtherData;
    } data;
} __attribute__((packed));

struct edid
{
    mt_u8 u8Header[8];/*headh*/

    /* Vendor & product info */
    mt_u8 u8MfgID[2];/*01*/
    mt_u8 u8ProductCode[2];/*03*/
    mt_u32 u32SerialNum; /* FIXME: byte order */
    mt_u8 u8MfWeek;/*10*/
    mt_u8 u8MfYear;/*11*/
    /* EDID version */
    mt_u8 u8Version;/*12h*/
    mt_u8 u8Revision;/*13h*/
    /* Display info: */
    mt_u8 u8Input;/*14h*/
    mt_u8 u8ImgeW_CM;
    mt_u8 u8ImgeH_CM;
    mt_u8 u8Gamma;
    mt_u8 u8Features;
    /* Color characteristics */
    mt_u8 u8RedGreenLowBits;/*19h*/
    mt_u8 u8BlackWhiteLowBits;
    mt_u8 u8RedX;
    mt_u8 u8RedY;
    mt_u8 u8GreenX;
    mt_u8 u8GreenY;
    mt_u8 u8BlueX;
    mt_u8 u8BlueY;
    mt_u8 u8WhiteX;
    mt_u8 u8WhiteY;

    /* Est. timings and mfg rsvd timings*/
    struct EST_TIMING_S stEstTimingData; /*23h~25h*/
    /* Standard timings 1-8*/
    EDID_STD_TIMING_S stSTDTimingData[8]; /*26h~35h*/
    /* Detailing timings 1-4 */
    struct DETAILED_TIMING_DATA stDetailTimingData[4]; /*36h~47*/
    /* Number of 128 byte ext. blocks */
    mt_u8 u8Extensions; /*7E*/
    /* Checksum */
    mt_u8 u8CheckSum;/*7F*/
} __attribute__((packed));

#define EDID_PRODUCT_ID(e) ((e)->u8ProductCode[0] | ((e)->u8ProductCode[1] << 8))
/* If detailed data is pixel timing */
struct MPI_SYSTEM_TIMING_S
{
    MT_UNF_EDID_TIMING_TYPE_E enTimingType;
    mt_u32 u32Hact;
    mt_u32 u32Vact;
    mt_u32 u32VerFreq;                      /* in Hz*/

    mt_u32 u32PixClkKHz;                 /* pixel_clock  in KHz*/
    mt_u16 u16RB;                              /*Reduced Blanking*/
} MPI_SYSTEM_TIMING;

struct MPI_SIMPLE_TIMING_S
{
    MT_UNF_EDID_TIMING_TYPE_E enTimingType;
    mt_u32 u32Hact;
    mt_u32 u32Vact;
    mt_u32 u32VerFreq;                      /* in Hz*/
    MT_BOOL                     bProgressive;
    mt_u32                    u32AspectWidth;
    mt_u32                    u32AspectHeight;
    mt_u32                      u32PixelRepetition;
} MPI_HDMI_TIMING;



#endif /* __DRM_EDID_H__ */
