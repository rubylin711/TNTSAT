/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#define VICMODES 5
#define NMODES 108
#define NSM    0 // no sub. mode
#define PC_BASE 60

#define _4     1  // 4:3
#define _16    2  // 16:9
#define _4or16   3  // 16:9
#define _16_10   4  // 16:10
#define _5_4   5  // 5:4

#define _1Sec 31
#define _2Sec 62
#define ProgrVPosHPos 0x03

/// //#define ProgrVNegHPos 0x02
#define ProgrVNegHPos 0x01

/// //#define ProgrVPosHNeg 0x01
#define ProgrVPosHNeg 0x02

#define ProgrVNegHNeg 0x00
#define InterlaceVPosHPos 0x07
#define InterlaceVNgeHPos 0x06
#define InterlaceVPosHNeg 0x05
#define InterlaceVNegHNeg 0x04

#define  STANDARDTIMING_SIZE 12
extern MT_UNF_EDID_BASE_INFO_S *DRV_Get_SinkCap(MT_UNF_HDMI_ID_E enHdmi);
#define EdidCheckHdmiFlg()	({\
		MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);\
		extern void DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bSinkValid);\
		if(pSinkCap->u8ExtBlockNum == 0)\
		{\
			pSinkCap->bSupportHdmi = MT_FALSE;\
			DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_0,MT_TRUE);\
			return ;\
		}\
	})

typedef struct {
	MT_UNF_ENC_FMT_E fmt;
} HDMIVICInfoType;

typedef struct {
	uint8_t Mode_C1;
	uint8_t Mode_C2;
	uint8_t SubMode;
	MT_UNF_ENC_FMT_E  enUNFFmt_C1;
	MT_UNF_ENC_FMT_E  enUNFFmt_C2;
} ModeIdType;

typedef struct {
	uint16_t Pixels;
	uint16_t Lines;
} PxlLnTotalType;

typedef struct {
	uint16_t H;
	uint16_t V;
} HVPositionType;

typedef struct {
	uint16_t H;
	uint16_t V;
} HVResolutionType;

typedef struct {
	uint8_t IntAdjMode;
	uint16_t HLength;
	uint16_t VLength;
	uint16_t Top;
	uint16_t Dly;
	uint16_t HBit2HSync;
	uint16_t VBit2VSync;
	uint16_t Field2Offset;

}  _656Type;

typedef struct {
	uint8_t RefrTypeVHPol;
	uint16_t VFreq;
	PxlLnTotalType Total;
} TagType;

typedef struct {
	ModeIdType ModeId;
	uint16_t PixClk;
	TagType Tag;
	HVPositionType Pos;
	HVResolutionType Res;
	uint8_t AspectRatio;
	_656Type _656;
	uint8_t PixRep;
} VModeInfoType;

int g_s32VmodeOfUNFFormat[MT_UNF_ENC_FMT_BUTT + 1] = {0};
const HDMIVICInfoType VICModeTables[VICMODES] = {
	{MT_UNF_ENC_FMT_BUTT }, // 0
	{MT_UNF_ENC_FMT_3840X2160_30 }, // 1 2160P@30
	{MT_UNF_ENC_FMT_3840X2160_25 }, // 2 2160P@25
	{MT_UNF_ENC_FMT_3840X2160_24 }, // 3 2160P@24
	{MT_UNF_ENC_FMT_4096X2160_24 }, // 4 4k*2K SMTP 4096*2160@24
};

const VModeInfoType VModeTables[NMODES] = {
	// 60 Hz
	/* 00 */{{ 1, 0, NSM, MT_UNF_ENC_FMT_861D_640X480_60, MT_UNF_ENC_FMT_BUTT},    2517,   {ProgrVNegHNeg,     6000,   {800, 525}},     {144, 35},  {640, 480},      _4,     {0, 96, 2, 33, 48, 16, 10, 0},         0}, // 1. 640 x 480p @ 60 VGA
	/* 01 */{{ 2, 0, NSM, MT_UNF_ENC_FMT_480P_60, MT_UNF_ENC_FMT_480P_60/*3:MT_UNF_ENC_FMT_480P_60 16:9*/}, 2700,   {ProgrVNegHNeg,     6000,   {858, 525}},     {122, 36},  {720, 480},      _4, {0, 62, 6, 30, 60, 19, 9, 0},          0}, // 2,3 720 x 480p
	/* 01 */{{ 3, 0, NSM, MT_UNF_ENC_FMT_480P_60, MT_UNF_ENC_FMT_480P_60/*3:MT_UNF_ENC_FMT_480P_60 16:9*/}, 2700,   {ProgrVNegHNeg,     6000,   {858, 525}},     {122, 36},  {720, 480},      _16, {0, 62, 6, 30, 60, 19, 9, 0},         0}, // 2,3 720 x 480p
	/* 02 */{{ 4, 0, NSM, MT_UNF_ENC_FMT_720P_60, MT_UNF_ENC_FMT_BUTT},    7417,   {ProgrVPosHPos,     6000,   {1650, 750}},    {260, 25},  {1280, 720},     _16,    {0, 40, 5, 20, 220, 110, 5, 0},        0}, // 4   1280 x 720p
	/* 03 */{{ 5, 0, NSM, MT_UNF_ENC_FMT_1080i_60, MT_UNF_ENC_FMT_BUTT},    7417,   {InterlaceVPosHPos, 6000,   {2200, 562}},    {192, 20},   {1920, 1080},    _16,    {0, 44, 5, 15, 148, 88, 2, 1100},      0}, // 5 1920 x 1080i
	/* 04 */{{ 6, 0, NSM, MT_UNF_ENC_FMT_NTSC, MT_UNF_ENC_FMT_NTSC/*7:MT_UNF_ENC_FMT_NTSC:480i@60,16:9*/},       2700,   {InterlaceVNegHNeg, 6000,   {1716, 264}},    {119, 18},   {720, 480},      _4, {0x03, 62, 3, 15, 114, 17, 5, 429},   1}, // 6,7 720 x 480i, pix repl
	/* 04 */{{ 7, 0, NSM, MT_UNF_ENC_FMT_NTSC, MT_UNF_ENC_FMT_NTSC/*7:MT_UNF_ENC_FMT_NTSC:480i@60,16:9*/},       2700,   {InterlaceVNegHNeg, 6000,   {1716, 264}},    {119, 18},   {720, 480},      _16, {0x03, 62, 3, 15, 114, 17, 5, 429},   1}, // 6,7 720 x 480i, pix repl
	/* 05 */{{ 8, 0,  1, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     6000,   {1716, 262}},    {119, 18},   {1440, 240},     _4, {0, 124, 3, 15, 114, 38, 4, 0},        1}, // 8,9(1) 1440 x 240p
	/* 06 */{{ 8, 0,  2, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     6000,   {1716, 263}},    {119, 18},   {1440, 240},     _16, {0, 124, 3, 15, 114, 38, 4, 0},        1}, // 8,9(2) 1440 x 240p
	/* 05 */{{ 9, 0,  1, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     6000,   {1716, 262}},    {119, 18},   {1440, 240},     _4, {0, 124, 3, 15, 114, 38, 4, 0},        1}, // 8,9(1) 1440 x 240p
	/* 06 */{{ 9, 0,  2, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     6000,   {1716, 263}},    {119, 18},   {1440, 240},     _16, {0, 124, 3, 15, 114, 38, 4, 0},        1}, // 8,9(2) 1440 x 240p
	/* 07 */{{10, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {InterlaceVNegHNeg, 6000,   {3432, 525}},    {238, 18},   {2880, 480},     _4, {0, 248, 3, 15, 228, 76, 4, 1716},     1}, // 10,11 2880 x 480p
	/* 07 */{{11, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {InterlaceVNegHNeg, 6000,   {3432, 525}},    {238, 18},   {2880, 480},     _16, {0, 248, 3, 15, 228, 76, 4, 1716},     1}, // 10,11 2880 x 480p
	/* 08 */{{12, 13,  1, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVNegHNeg,     6000,   {3432, 262}},    {238, 18},   {2880, 240},     _4or16, {0, 248, 3, 15, 228, 76, 4, 0},      1}, // 12,13(1) 2280 x 240p
	/* 09 */{{12, 13,  2, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVNegHNeg,     6000,   {3432, 263}},    {238, 18},   {2880, 240},     _4or16, {0, 248, 3, 15, 228, 76, 4, 0},      1}, // 12,13(2) 2280 x 240p
	/* 0a */{{14, 15, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVNegHNeg,     6000,   {1716, 525}},    {244, 36},   {1440, 480},     _4or16, {0, 124, 6, 30, 120, 32, 9, 0},      0}, // 14, 15 1140 x 480p
	/* 0b */{{16, 0, NSM, MT_UNF_ENC_FMT_1080P_60, MT_UNF_ENC_FMT_BUTT},   14850,  {ProgrVPosHPos,     6000,   {2200, 1125}},   {192, 41},   {1920, 1080},    _16,    {0, 44, 5, 36, 148, 88, 4, 0},     0}, // 16 1920 x 1080p

	// 50 Hz
	/* 0c */{{17, 0, NSM, MT_UNF_ENC_FMT_576P_50, MT_UNF_ENC_FMT_576P_50/*18:MT_UNF_ENC_FMT_576P_50 16:9*/}, 2700,   {ProgrVNegHNeg,     5000,   {864, 625}},    {132, 44},   {720, 576},     _4, {0, 64, 5, 39, 68, 12, 5, 0},     0}, // 17,18 720 x 576p
	/* 0c */{{18, 0, NSM, MT_UNF_ENC_FMT_576P_50, MT_UNF_ENC_FMT_576P_50/*18:MT_UNF_ENC_FMT_576P_50 16:9*/}, 2700,   {ProgrVNegHNeg,     5000,   {864, 625}},    {132, 44},   {720, 576},     _16, {0, 64, 5, 39, 68, 12, 5, 0},     0}, // 17,18 720 x 576p
	/* 0d */{{19, 0, NSM, MT_UNF_ENC_FMT_720P_50, MT_UNF_ENC_FMT_BUTT},    7425,   {ProgrVPosHPos,     5000,   {1980, 750}},    {260, 25},   {1280, 720},     _16,    {0, 40, 5, 20, 220, 440, 5, 0},     0}, // 19 1280 x 720p
	/* 0e */{{20, 0, NSM, MT_UNF_ENC_FMT_1080i_50, MT_UNF_ENC_FMT_BUTT},   7425,   {InterlaceVPosHPos, 5000,   {2640, 1125}},  {192, 20},   {1920, 1080},    _16,    {0, 44, 5, 15, 148, 528, 2, 1320},    0}, // 20 1920 x 1080i
	/* 0f */{{21, 0, NSM, MT_UNF_ENC_FMT_PAL, MT_UNF_ENC_FMT_PAL/*22:MT_UNF_ENC_FMT_PAL:576i@60,16:9*/},       2700,   {InterlaceVNegHNeg, 5000,   {1728, 625}},   {132, 22},   {720, 576},     _4,     {3, 63, 3, 19, 138, 24, 2, 432},  1}, // 21,22 1440 x 576i
	/* 0f */{{22, 0, NSM, MT_UNF_ENC_FMT_PAL, MT_UNF_ENC_FMT_PAL/*22:MT_UNF_ENC_FMT_PAL:576i@60,16:9*/},       2700,   {InterlaceVNegHNeg, 5000,   {1728, 625}},   {132, 22},   {720, 576},     _16,     {3, 63, 3, 19, 138, 24, 2, 432},  1}, // 21,22 1440 x 576i
	/* 10 */{{23, 24,  1, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     5000,   {1728, 312}},    {132, 22},   {1440, 288},     _4or16, {0, 126, 3, 19, 138, 24, 2,   0},  1}, // 23,24(1) 1440 x 288p
	/* 11 */{{23, 24,  2, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     5000,   {1728, 313}},    {132, 22},   {1440, 288},     _4or16, {0, 126, 3, 19, 138, 24, 2,   0},  1}, // 23,24(2) 1440 x 288p
	/* 12 */{{23, 24,  3, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       2700,   {ProgrVNegHNeg,     5000,   {1728, 314}},    {132, 22},   {1440, 288},     _4or16, {0, 126, 3, 19, 138, 24, 2,   0},  1}, // 23,24(3) 1440 x 288p
	/* 13 */{{25, 26, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {InterlaceVNegHNeg, 5000,   {3456, 625}},   {264, 22},   {2880, 576},     _4or16, {0, 252, 3, 19, 276, 48, 2, 1728},  1}, // 25, 26 2880 x 576p
	/* 14 */{{27, 28,  1, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVNegHNeg,     5000,   {3456, 312}},    {264, 22},   {2880, 288},     _4or16, {0, 252, 3,  19, 276, 48, 2, 0},   1}, // 27,28(1) 2880 x 288p
	/* 15 */{{27, 28,  2, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVNegHNeg,     5000,   {3456, 313}},    {264, 22},   {2880, 288},     _4or16, {0, 252, 3,  19, 276, 48, 3, 0},   1}, // 27,28(2) 2880 x 288p
	/* 16 */{{27, 28,  3, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVNegHNeg,     5000,   {3456, 314}},    {264, 22},   {2880, 288},     _4or16, {0, 252, 3,  19, 276, 48, 4, 0},   1}, // 27,28(3) 2880 x 288p
	/* 17 */{{29, 30, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       5400,   {ProgrVPosHNeg,     5000,   {1728, 625}},    {264, 44},   {1440, 576},     _4or16, {0, 128, 5, 39, 136, 24, 5, 0},    0}, // 29,30 1440 x 576p
	/* 18 */{{31, 0, NSM, MT_UNF_ENC_FMT_1080P_50, MT_UNF_ENC_FMT_BUTT},   14850,  {ProgrVPosHPos,     5000,   {2640, 1125}},   {192, 41},   {1920, 1080},    _16,    {0, 44, 5, 36, 148, 528, 4, 0},     0}, // 31(1) 1920 x 1080p
	/* 19 */{{32, 0, NSM, MT_UNF_ENC_FMT_1080P_24, MT_UNF_ENC_FMT_BUTT},   7417,   {ProgrVPosHPos,     2400,   {2750, 1125}},   {192, 41},   {1920, 1080},    _16,    { 0, 44, 5, 36, 148, 638, 4, 0},    0}, // 32(2) 1920 x 1080p
	/* 1a */{{33, 0, NSM, MT_UNF_ENC_FMT_1080P_25, MT_UNF_ENC_FMT_BUTT},   7425,   {ProgrVPosHPos,     2500,   {2640, 1125}},   {192, 41},   {1920, 1080},    _16,    { 0, 44, 5, 36, 148, 528, 4, 0},    0}, // 33(3) 1920 x 1080p
	/* 1b */{{34, 0, NSM, MT_UNF_ENC_FMT_1080P_30, MT_UNF_ENC_FMT_BUTT},   7417,   {ProgrVPosHPos,     3000,   {2200, 1125}},   {192, 41},   {1920, 1080},    _16,    { 0, 44, 5, 36, 148, 528, 4, 0},    0}, // 34(4) 1920 x 1080p
	/* 1c */{{35, 36, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      10800,  {ProgrVNegHNeg,      5994,   {3432, 525}},   {488, 36},  {2880, 480},    _4or16,  {0, 248, 6, 30, 240, 64, 10, 0}, 0}, // 35, 36 2880 x 480p@59.94/60Hz
	{{37, 38, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      10800,  {ProgrVNegHNeg,      5000,   {3456, 625}},   {272, 39},  {2880, 576},    _4or16,  {0, 256, 5, 40, 272, 48, 5, 0}, 0},  // 37, 38 2880 x 576p@50Hz
	{{39, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       7200,   {InterlaceVNegHNeg,  5000,   {2304, 1250}},  {352, 62},  {1920, 1080},   _16,     {0, 168, 5, 87, 184, 32, 24, 0}, 0}, // 39 1920 x 1080i@50Hz
	{{40, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       14850,  {InterlaceVPosHPos, 10000,   {2640, 1125}},  {192, 20},  {1920, 1080},   _16,     {0, 44, 5, 15, 148, 528, 2, 1320}, 0}, // 40 1920 x 1080i@100Hz
	{{41, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       14850,  {InterlaceVPosHPos, 10000,   {1980, 750}},   {260, 25},  {1280, 720},    _16,     {0, 40, 5, 20, 220, 400, 5, 0}, 0},  // 41 1280 x 720p@100Hz
	{{42, 43, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      5400,   {ProgrVNegHNeg,     10000,   {864,  144}},   {132, 44}, {720, 576},    _4or16,  {0, 64, 5, 39, 68, 12, 5, 0}, 0},      // 42, 43, 720p x 576p@100Hz
	{{44, 45, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      5400,   {InterlaceVNegHNeg, 10000,   {864,  625}},   {132, 22}, {720, 576},    _4or16,  {0, 63, 3, 19, 69, 12, 2, 432}, 1},    // 44, 45, 720p x 576i@100Hz, pix repl
	{{46, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       14835,  {InterlaceVPosHPos, 11988,   {2200, 1125}},  {192, 20}, {1920, 1080},  _16,     {0, 44, 5, 15, 149, 88, 2, 1100}, 0},  // 46, 1920 x 1080i@119.88/120Hz
	{{47, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},       14835,  {ProgrVPosHPos,     11988,   {1650, 750}},   {260, 25}, {1280, 720},   _16,     {0, 40, 5, 20, 220, 110, 5, 1100}, 0}, // 47, 1280 x 720p@119.88/120Hz
	{{48, 49, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      5400,   {ProgrVNegHNeg,     11988,   {858, 525}},    {122, 36}, {720, 480},   _4or16,   {0, 62, 6, 30, 60, 16, 10, 0}, 0},     // 48, 49 720 x 480p@119.88/120Hz
	{{50, 51, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      5400,   {InterlaceVNegHNeg, 11988,   {858, 525}},    {119, 18}, {720, 480},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 1},    // 50, 51 720 x 480i@119.88/120Hz
	{{52, 53, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      10800,  {ProgrVNegHNeg,     20000,   {864, 625}},    {132, 44}, {720, 576},   _4or16,   {0, 64, 5, 39, 68, 12, 5, 0}, 0},      // 52, 53, 720 x 576p@200Hz
	{{54, 55, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      10800,  {InterlaceVNegHNeg, 20000,   {864, 625}},    {132, 22}, {720, 576},   _4or16,   {0, 63, 3, 19, 69, 12, 2, 432}, 1},    // 54, 55, 1440 x 720i @200Hz, pix repl
	{{56, 57, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      10800,  {ProgrVNegHNeg,     24000,   {858, 525}},    {122, 42}, {720, 480},   _4or16,   {0, 62, 6, 30, 60, 16, 9, 0}, 0},      // 56, 57, 720 x 480p @239.76/240Hz
	{{58, 59, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},      10800,  {InterlaceVNegHNeg, 24000,   {858, 525}},    {119, 18}, {720, 480},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 1},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{93, 0, NSM, MT_UNF_ENC_FMT_3840X2160_24, MT_UNF_ENC_FMT_BUTT},      29700,  {ProgrVNegHNeg, 2400,   {5500, 2250}},    {119, 18}, {3840, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{94, 0, NSM, MT_UNF_ENC_FMT_3840X2160_25, MT_UNF_ENC_FMT_BUTT},      29700,  {ProgrVNegHNeg, 2500,   {5280, 2250}},    {119, 18}, {3840, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{95, 0, NSM, MT_UNF_ENC_FMT_3840X2160_30, MT_UNF_ENC_FMT_BUTT},      29700,  {ProgrVNegHNeg, 3000,   {4400, 2250}},    {119, 18}, {3840, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{96, 0, NSM, MT_UNF_ENC_FMT_3840X2160_50, MT_UNF_ENC_FMT_BUTT},      59400,  {ProgrVNegHNeg, 5000,   {5280, 2250}},    {119, 18}, {3840, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{97, 0, NSM, MT_UNF_ENC_FMT_3840X2160_60, MT_UNF_ENC_FMT_BUTT},      59400,  {ProgrVNegHNeg, 6000,   {4400, 2250}},    {119, 18}, {3840, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{98, 0, NSM, MT_UNF_ENC_FMT_4096X2160_24, MT_UNF_ENC_FMT_BUTT},      29700,  {ProgrVNegHNeg, 2400,   {5500, 2250}},    {119, 18}, {4096, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{99, 0, NSM, MT_UNF_ENC_FMT_4096X2160_25, MT_UNF_ENC_FMT_BUTT},      29700,  {ProgrVNegHNeg, 2500,   {5280, 2250}},    {119, 18}, {4096, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{100, 0, NSM, MT_UNF_ENC_FMT_4096X2160_30, MT_UNF_ENC_FMT_BUTT},      29700,  {ProgrVNegHNeg, 3000,   {4400, 2250}},    {119, 18}, {4096, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{101, 0, NSM, MT_UNF_ENC_FMT_4096X2160_50, MT_UNF_ENC_FMT_BUTT},      59400,  {ProgrVNegHNeg, 5000,   {5280, 2250}},    {119, 18}, {4096, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	{{102, 0, NSM, MT_UNF_ENC_FMT_4096X2160_60, MT_UNF_ENC_FMT_BUTT},      59400,  {ProgrVNegHNeg, 6000,   {4400, 2250}},    {119, 18}, {4096, 2160},   _4or16,   {0, 62, 3, 15, 57, 19, 4, 429}, 0},    // 58, 59, 1440 x 480i @239.76/240Hz, pix repl
	// NOTE: DO NOT ATTEMPT INPUT RESOLUTIONS THAT REQUIRE PIXEL CLOCK FREQUENCIES HIGHER THAN THOSE SUPPOTED BY THE TRANSMITTER CHIP
	//                            1         2                 3       4    5         6 7          8   9      10    11, 13, 15
	#if defined (DVI_SUPPORT)
	{{PC_BASE, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},    3150,   {ProgrVNegHPos,     8508,   {832, 445}},    {160, 63},   {640, 350},   _16,  {0, 64, 3, 60, 96, 32, 32, 0}, 0}, // 640x350@85.08
	{{PC_BASE + 1, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  3150,   {ProgrVPosHNeg,     8508,   {832, 445}},    {160, 44},   {640, 400},   _16,  {0, 64, 3, 41, 96, 32, 1, 0},  0}, // 640x400@85.08
	{{PC_BASE + 2, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  2700,   {ProgrVPosHNeg,     7008,   {900, 449}},    {0, 0},      {720, 400},   _16,  {0, 0, 0, 0, 0, 0, 0, 0},      0}, // 720x400@70.08
	{{PC_BASE + 3, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  3500,   {ProgrVPosHNeg,     8504,   {936, 446}},    {20, 45},    {720, 400},   _16,  {0, 72, 3, 42, 108, 36, 1, 0}, 0}, // 720x400@85.04
	{{PC_BASE + 4, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  2517,   {ProgrVNegHNeg,     5994,   {800, 525}},    {144, 35},   {640, 480},   _4,  {0, 96, 2, 33, 48, 16, 10, 0},  0}, // 640x480@59.94
	{{PC_BASE + 5, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  3150,   {ProgrVNegHNeg,     7281,   {832, 520}},    {144, 31},   {640, 480},   _4,  {0, 40, 3, 28, 128, 128, 9, 0}, 0}, // 640x480@72.80
	{{PC_BASE + 6, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  3150,   {ProgrVNegHNeg,     7500,   {840, 500}},    {21, 19},    {640, 480},   _4,  {0, 64, 3, 28, 128, 24, 9, 0}, 0}, // 640x480@75.00
	{{PC_BASE + 7, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},   3600,   {ProgrVNegHNeg,     8500,   {832, 509}},    {168, 28},   {640, 480},   _4,  {0, 56, 3, 25, 128, 24, 9, 0}, 0}, // 640x480@85.00
	{{PC_BASE + 8, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},   3600,   {ProgrVPosHPos,     5625,   {1024, 625}},   {200, 24},   {800, 600},   _4,  {0, 72, 2, 22, 128, 24, 1, 0}, 0}, // 800x600@56.25
	{{PC_BASE + 9, 0, NSM, MT_UNF_ENC_FMT_VESA_800X600_60, MT_UNF_ENC_FMT_BUTT},   4000,   {ProgrVPosHPos,     6032,   {1056, 628}},   {216, 27},   {800, 600},   _4,  {0, 128, 4, 23, 88, 40, 1, 0}, 0}, // 800x600@60.317
	{{PC_BASE + 10, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  5000,   {ProgrVPosHPos,     7219,   {1040, 666}},   {184, 29},   {800, 600},   _4,  {0, 120, 6, 23, 64, 56, 37, 0}, 0}, // 800x600@72.19
	{{PC_BASE + 11, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  4950,   {ProgrVPosHPos,     7500,   {1056, 625}},   {240, 24},   {800, 600},   _4,  {0, 80, 3, 21, 160, 16, 1, 0},  0}, // 800x600@75
	{{PC_BASE + 12, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  5625,   {ProgrVPosHPos,     8506,   {1048, 631}},   {216, 30},   {800, 600},   _4,  {0, 64, 3, 27, 152, 32, 1, 0},  0}, // 800x600@85.06
	{{PC_BASE + 13, 0, NSM, MT_UNF_ENC_FMT_VESA_1024X768_60, MT_UNF_ENC_FMT_BUTT},  6500,   {ProgrVNegHNeg,     6000,   {1344, 806}},   {296, 35},   {1024, 768},  _4,  {0, 136, 6, 29, 160, 24, 3, 0}, 0}, // 1024x768@60
	{{PC_BASE + 14, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  7500,   {ProgrVNegHNeg,     7007,   {1328, 806}},   {280, 35},   {1024, 768},  _4,  {0, 136, 6, 19, 144, 24, 3, 0}, 0}, // 1024x768@70.07
	{{PC_BASE + 15, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  7875,   {ProgrVPosHPos,     7503,   {1312, 800}},   {272, 31},   {1024, 768},  _4,  {0, 96, 3, 28, 176, 16, 1, 0},  0}, // 1024x768@75.03
	{{PC_BASE + 16, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  9450,   {ProgrVPosHPos,     8500,   {1376, 808}},   {304, 39},   {1024, 768},  _4,  {0, 96, 3, 36, 208, 48, 1, 0},  0}, // 1024x768@85
	{{PC_BASE + 17, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 10800,   {ProgrVPosHPos,     7500,   {1600, 900}},   {384, 35},   {1152, 864},  _4,  {0, 128, 3, 32, 256, 64, 1, 0}, 0}, // 1152x864@75
	{{PC_BASE + 18, 0, NSM, MT_UNF_ENC_FMT_VESA_1600X1200_60, MT_UNF_ENC_FMT_BUTT}, 16200,   {ProgrVPosHPos,     6000,   {2160, 1250}},  {496, 49},   {1600, 1200}, _4,  {0, 304, 3, 46, 304, 64, 1, 0}, 0}, // 1600x1200@60
	{{PC_BASE + 19, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  6825,   {ProgrVNegHPos,     6000,   {1440, 790}},   {112, 19},   {1280, 768},  _16,  {0, 32, 7, 12, 80, 48, 3, 0},   0}, // 1280x768@59.95
	{{PC_BASE + 20, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT},  7950,   {ProgrVPosHNeg,     5987,   {1664, 798}},   {320, 27},   {1280, 768},  _16,  {0, 128, 7, 20, 192, 64, 3, 0}, 0}, // 1280x768@59.87
	{{PC_BASE + 21, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 10220,   {ProgrVPosHNeg,     6029,   {1696, 805}},   {320, 27},   {1280, 768},  _16,  {0, 128, 7, 27, 208, 80, 3, 0}, 0}, // 1280x768@74.89
	{{PC_BASE + 22, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 11750,   {ProgrVPosHNeg,     8484,   {1712, 809}},   {352, 38},   {1280, 768},  _16,  {0, 136, 7, 31, 216, 80, 3, 0}, 0}, // 1280x768@85
	{{PC_BASE + 23, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 10800,   {ProgrVPosHPos,     6000,   {1800, 1000}},  {424, 39},   {1280, 960},  _4,  {0, 112, 3, 36, 312, 96, 1, 0}, 0}, // 1280x960@60
	{{PC_BASE + 24, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 14850,   {ProgrVPosHPos,     8500,   {1728, 1011}},  {384, 50},   {1280, 960},  _4,  {0, 160, 3, 47, 224, 64, 1, 0}, 0}, // 1280x960@85
	{{PC_BASE + 25, 0, NSM, MT_UNF_ENC_FMT_VESA_1280X1024_60, MT_UNF_ENC_FMT_BUTT}, 10800,   {ProgrVPosHPos,     6002,   {1688, 1066}},  {360, 41},   {1280, 1024}, _4,  {0, 112, 3, 38, 248, 48, 1, 0}, 0}, // 1280x1024@60
	{{PC_BASE + 26, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 13500,   {ProgrVPosHPos,     7502,   {1688, 1066}},  {392, 41},   {1280, 1024}, _4,  {0, 144, 3, 38, 248, 16, 1, 0}, 0}, // 1280x1024@75
	{{PC_BASE + 27, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 15750,   {ProgrVPosHPos,     8502,   {1728, 1072}},  {384, 47},   {1280, 1024}, _4,  {0, 160, 3, 4, 224, 64, 1, 0}, 0}, // 1280x1024@85
	{{PC_BASE + 28, 0, NSM, MT_UNF_ENC_FMT_VESA_1366X768_60, MT_UNF_ENC_FMT_BUTT},  8550,   {ProgrVPosHPos,     6002,   {1792, 795}},   {368, 24},   {1360, 768},  _16,  {0, 112, 6, 18, 256, 64, 3, 0}, 0}, // 1360x768@60
	{{PC_BASE + 29, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 10100,   {ProgrVNegHPos,     5995,   {1560, 1080}},  {112, 27},   {1400, 1050}, _4,  {0, 32, 4, 23, 80, 48, 3, 0},   0}, // 1400x105@59.95
	{{PC_BASE + 30, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 12175,   {ProgrVPosHNeg,     5998,   {1864, 1089}},  {376, 36},   {1400, 1050}, _4,  {0, 144, 4, 32, 232, 88, 3, 0}, 0}, // 1400x105@59.98
	{{PC_BASE + 31, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 15600,   {ProgrVPosHNeg,     7487,   {1896, 1099}},  {392, 46},   {1400, 1050}, _4,  {0, 144, 4, 22, 248, 104, 3, 0}, 0}, // 1400x105@74.87
	{{PC_BASE + 32, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 17950,   {ProgrVPosHNeg,     8496,   {1912, 1105}},  {408, 52},   {1400, 1050}, _4,  {0, 152, 4, 48, 256, 104, 3, 0}, 0}, // 1400x105@84.96
	{{PC_BASE + 33, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 17550,   {ProgrVPosHPos,     6500,   {2160, 1250}},  {496, 49},   {1600, 1200}, _4,  {0, 192, 3, 46, 304, 64, 1, 0}, 0}, // 1600x1200@65
	{{PC_BASE + 34, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 18900,   {ProgrVPosHPos,     7000,   {2160, 1250}},  {496, 49},   {1600, 1200}, _4,  {0, 192, 3, 46, 304, 64, 1, 0}, 0}, // 1600x1200@70
	{{PC_BASE + 35, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 20250,   {ProgrVPosHPos,     7500,   {2160, 1250}},  {496, 49},   {1600, 1200}, _4,  {0, 192, 3, 46, 304, 64, 1, 0}, 0}, // 1600x1200@75
	{{PC_BASE + 36, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 22950,   {ProgrVPosHPos,     8500,   {2160, 1250}},  {496, 49},   {1600, 1200}, _4,  {0, 192, 3, 46, 304, 64, 1, 0}, 0}, // 1600x1200@85
	{{PC_BASE + 37, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 20475,   {ProgrVPosHNeg,     6000,   {2448, 1394}},  {528, 49},   {1792, 1344}, _4,  {0, 200, 3, 46, 328, 128, 1, 0}, 0}, // 1792x1344@60
	{{PC_BASE + 38, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 26100,   {ProgrVPosHNeg,     7500,   {2456, 1417}},  {568, 72},   {1792, 1344}, _4,  {0, 216, 3, 69, 352, 96, 1, 0}, 0}, // 1792x1344@74.997
	{{PC_BASE + 39, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 21825,   {ProgrVPosHNeg,     6000,   {2528, 1439}},  {576, 46},   {1856, 1392}, _4,  {0, 224, 3, 43, 352, 96, 1, 0}, 0}, // 1856x1392@60
	{{PC_BASE + 40, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 28800,   {ProgrVPosHNeg,     7500,   {2560, 1500}},  {576, 107},  {1856, 1392}, _4,  {0, 224, 3, 104, 352, 128, 1, 0}, 0}, // 1856x1392@75
	{{PC_BASE + 41, 0, NSM, MT_UNF_ENC_FMT_VESA_1920X1200_60, MT_UNF_ENC_FMT_BUTT}, 15400,   {ProgrVNegHPos,     5995,   {2080, 1235}},  {112, 32},   {1920, 1200}, _16,  {0, 32, 6, 26, 80, 48, 3, 0},    0}, // 1920x1200@59.95
	{{PC_BASE + 42, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 19325,   {ProgrVPosHNeg,     5988,   {2592, 1245}},  {536, 42},   {1920, 1200}, _16,  {0, 200, 6, 36, 336, 136, 3, 0}, 0}, // 1920x1200@59.88
	{{PC_BASE + 43, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 24525,   {ProgrVPosHNeg,     7493,   {2608, 1255}},  {552, 52},   {1920, 1200}, _16,  {0, 208, 6, 46, 344, 136, 3, 0}, 0}, // 1920x1200@74.93
	{{PC_BASE + 44, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 28125,   {ProgrVPosHNeg,     8493,   {2624, 1262}},  {560, 59},   {1920, 1200}, _16,  {0, 208, 6, 53, 352, 144, 3, 0}, 0}, // 1920x1200@84.93
	{{PC_BASE + 45, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 23400,   {ProgrVPosHNeg,     6000,   {2600, 1500}},  {552, 59},   {1920, 1440}, _4,  {0, 208, 3, 56, 344, 128, 1, 0}, 0}, // 1920x1440@60
	{{PC_BASE + 46, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 29700,   {ProgrVPosHNeg,     7500,   {2640, 1500}},  {576, 59},   {1920, 1440}, _4,  {0, 224, 3, 56, 352, 144, 1, 0}, 0}, // 1920x1440@75
	{{PC_BASE + 47, 0, NSM, MT_UNF_ENC_FMT_BUTT, MT_UNF_ENC_FMT_BUTT}, 29700,   {ProgrVPosHNeg,     7500,   {2640, 1500}},  {576, 59},   {1920, 1440}, _4,  {0, 224, 3, 56, 352, 144, 1, 0}, 0}, // 1920x1440@75
	#endif
};

typedef struct {
	MT_U8 pixel_clk[2];
	MT_U8 h_active;
	MT_U8 h_blank;
	MT_U8 h_active_blank;
	MT_U8 v_active;
	MT_U8 v_blank;
	MT_U8 v_active_blank;
	MT_U8 h_sync_offset;
	MT_U8 h_sync_pulse_width;
	MT_U8 vs_offset_pulse_width;
	MT_U8 hs_offset_vs_offset;
	MT_U8 h_image_size;
	MT_U8 v_image_size;
	MT_U8 h_v_image_size;
	MT_U8 h_border;
	MT_U8 v_border;
	MT_U8 flags;
} DETAILED_TIMING_BLOCK;

static void sStoreFmtInfo(uint32_t vic, bool_t Nflg, bool_t y420O)
{
	uint32_t hdmiModeIdx;
	MT_UNF_ENC_FMT_E enFmt, enNativeFmt = MT_UNF_ENC_FMT_BUTT;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);

	for (hdmiModeIdx = 0; hdmiModeIdx < NMODES; hdmiModeIdx++) {
		if (VModeTables[hdmiModeIdx].ModeId.Mode_C1 == vic) {
			if (VModeTables[hdmiModeIdx].ModeId.enUNFFmt_C1 >= MT_UNF_ENC_FMT_BUTT) {
				continue;
			}
			enFmt = VModeTables[hdmiModeIdx].ModeId.enUNFFmt_C1;
			pSinkCap->bSupportFormat[enFmt] = MT_TRUE | (y420O << 1);	/* support timing foramt */
			if (enNativeFmt == MT_UNF_ENC_FMT_BUTT) {
				enNativeFmt = enFmt; //if no native flag set first define format
			}

			if (Nflg) {
				pSinkCap->enNativeFormat = enFmt;
			}
			g_s32VmodeOfUNFFormat[enFmt] = hdmiModeIdx;
		}
	}

	if (MT_UNF_ENC_FMT_BUTT == pSinkCap->enNativeFormat) {
		//pSinkCap->enNativeFormat = enNativeFmt; //set The first order fmt  //According to QA's request@24204
	}
}

static void sParseVsdbHdmi14SaveFmt(uint8_t *pData)
{
	uint8_t	u8Temp;
	uint32_t	u32temp;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);

	u8Temp = pData[0];
	if (u8Temp <= 4) {
		u32temp = VICModeTables[u8Temp].fmt;
		if (u32temp < MT_UNF_ENC_FMT_BUTT) {
			pSinkCap->bSupportFormat[u32temp] = MT_TRUE;
		} else {
			EDID_WARN("Getted Butt fmt when parse 4k cap \n");
		}
	}
}

static void sParseVSVDBSaveHDR10P(MT_BOOL visif, MT_BOOL vivid)
{
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	pSinkCap->bSupportHDR10pVSIF = visif;
	pSinkCap->bSupportHDR10pVivid = vivid;
}

uint8_t Transfer_VideoTimingFromat_to_VModeTablesIndex(MT_UNF_ENC_FMT_E unfFmt)
{

	if (MT_UNF_ENC_FMT_BUTT <= unfFmt) {
		return 0;    /* 640x480 format index in VModeTables*/
	} else {
		return (g_s32VmodeOfUNFFormat[unfFmt]);
	}
}

static void sParseEdidSaveFmtByVdb(SiiLibEdidPar_t* poEdidPar)
{
	uint32_t vic;
	uint8_t i;

	for (i = 0; i < poEdidPar->vdb.size; i++) {
		vic = poEdidPar->vdb.svd[i].p[0];
		sStoreFmtInfo((uint32_t)vic, ((poEdidPar->vdb.svd[i].p[0]) & 0x80) == 0x80, false);
	}
}

static void sParseEdidSaveFmtByDTD(uint32_t pixFrq, uint16_t horTot, uint16_t verTot, uint32_t fVfreq_int, uint32_t fVfreq_dec)
{
	uint32_t hdmiModeIdx;
	MT_UNF_ENC_FMT_E enFmt;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	uint32_t frm_rate = 0;
	uint32_t frm_rate_ref = 0;

	if ( pixFrq == 0 || horTot == 0 || verTot == 0 ) {
		frm_rate = 0;
	} else {
		frm_rate = fVfreq_int * 100 + (fVfreq_dec / 10);
	}
	EDID_PRINTF(( SI_LOG_LEVEL_STRING "clk:%d,Htol:%d,Vtol:%d,frm:%d", pixFrq, (uint32_t)horTot, (uint32_t)verTot, frm_rate));

	for (hdmiModeIdx = 0; hdmiModeIdx < NMODES; hdmiModeIdx++) {
		if (VModeTables[hdmiModeIdx].PixClk == (pixFrq / 10000)) {
			if (VModeTables[hdmiModeIdx].Tag.VFreq >= 5900 && VModeTables[hdmiModeIdx].Tag.VFreq <= 6100) {
				frm_rate_ref = 6000;
			} else if (VModeTables[hdmiModeIdx].Tag.VFreq >= 2900 && VModeTables[hdmiModeIdx].Tag.VFreq <= 3100) {
				frm_rate_ref = 3000;
			} else {
				frm_rate_ref = VModeTables[hdmiModeIdx].Tag.VFreq;
			}
			if (frm_rate >= 5900 && frm_rate <= 6100) {
				frm_rate = 6000;
			} else if (frm_rate >= 2900 && frm_rate <= 3100) {
				frm_rate = 3000;
			} else if (frm_rate >= 2300 && frm_rate <= 2450) {
				frm_rate = 2400;
			} else if (frm_rate >= 4900 && frm_rate <= 5100) {
				frm_rate = 5000;
			} else if (frm_rate > 2450 && frm_rate <= 2600) {
				frm_rate = 2500;
			}
			if (frm_rate_ref == frm_rate && VModeTables[hdmiModeIdx].Tag.Total.Pixels == horTot && \
					VModeTables[hdmiModeIdx].Tag.Total.Lines == verTot) {
				enFmt = VModeTables[hdmiModeIdx].ModeId.enUNFFmt_C1;
				if ( enFmt < MT_UNF_ENC_FMT_BUTT ) {
					pSinkCap->bSupportFormat[enFmt] = MT_TRUE;	/* support timing format */
					g_s32VmodeOfUNFFormat[enFmt] = hdmiModeIdx;
				}
				break;
			}
		}
	}
}

MT_S32 ParsePreferredTiming(MT_U8 * pData)
{
	MT_U32 u32Temp;
	DETAILED_TIMING_BLOCK *pDetailed = (DETAILED_TIMING_BLOCK*)pData;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	MT_UNF_EDID_TIMING_S *pEdidTiming = &pSinkCap->stPerferTiming;

	if ( pEdidTiming->u32VFB != 0 && pEdidTiming->u32VBB != 0 ) {
		return MT_SUCCESS;
	}
	//pixel clock
	u32Temp = (pDetailed->pixel_clk[0] | (pDetailed->pixel_clk[1] << 8)) * 10; //unit HZ
	EDID_INFO("pixel clock :%d\n", u32Temp);
	pEdidTiming->u32PixelClk = u32Temp;

	//vfb
	u32Temp = pDetailed->v_border ;
	u32Temp += (pDetailed->vs_offset_pulse_width >> 4) | ((pDetailed->hs_offset_vs_offset & 0x0C) << 4);
	EDID_INFO("VFB :%d\n", u32Temp);
	pEdidTiming->u32VFB = u32Temp;

	//vbb
	//v_active_blank == vblack == vfront + vback + vsync = VFB(vfront) + VBB(vback + vsync)
	u32Temp = ((pDetailed->v_active_blank & 0x0F) << 8) | pDetailed->v_blank;
	EDID_INFO("VBB :%d\n", u32Temp);
	pEdidTiming->u32VBB = u32Temp - pEdidTiming->u32VFB;

	//vact
	u32Temp = ((pDetailed->v_active_blank & 0xF0) << 4) | pDetailed->v_active;
	EDID_INFO("VACT :%d\n", u32Temp);
	pEdidTiming->u32VACT = u32Temp;

	//HFB
	u32Temp = pDetailed->h_border;
	u32Temp += pDetailed->h_sync_offset | ((pDetailed->hs_offset_vs_offset & 0xC0) << 2);
	EDID_INFO("HFB :%d\n", u32Temp);
	pEdidTiming->u32HFB = u32Temp;
	//HBB
	u32Temp = pDetailed->h_blank;
	u32Temp += (pDetailed->h_active_blank & 0x0F) << 8;
	EDID_INFO("HBB :%d\n", u32Temp);
	//h_active_blank == hblack == hfront + hback + hsync = HFB(hfront) + HBB(hback + hsync)
	pEdidTiming->u32HBB = u32Temp - pEdidTiming->u32HFB;

	//HACT
	u32Temp = ((pDetailed->h_active_blank & 0xF0) << 4) | pDetailed->h_active;
	EDID_INFO("HACT :%d\n", u32Temp);
	pEdidTiming->u32HACT = u32Temp;

	//VPW
	u32Temp = (pDetailed->hs_offset_vs_offset & 0x03) << 4;
	u32Temp |= (pDetailed->vs_offset_pulse_width & 0x0F);
	EDID_INFO("VPW :%d\n", u32Temp);
	pEdidTiming->u32VPW = u32Temp;
	//HPW
	u32Temp = (pDetailed->hs_offset_vs_offset & 0x30) << 4;
	u32Temp |= pDetailed->h_sync_pulse_width;
	EDID_INFO("HPW :%d\n", u32Temp);
	pEdidTiming->u32HPW = u32Temp;

	// H image size
	u32Temp = pDetailed->h_image_size;
	u32Temp |= (pDetailed->h_v_image_size & 0xF0) << 4;
	EDID_INFO("H image size :%d\n", u32Temp);
	pEdidTiming->u32ImageWidth = u32Temp;

	//V image size
	u32Temp = (pDetailed->h_v_image_size & 0x0F) << 8;
	u32Temp  |= pDetailed->v_image_size ;
	EDID_INFO("V image size :%d\n", u32Temp);
	pEdidTiming->u32ImageHeight = u32Temp;
	if (pDetailed->flags & 0x80) {      /*Interlaced flag*/
		EDID_INFO("Output mode: interlaced\n");
		pEdidTiming->bInterlace = MT_TRUE;
	} else {
		EDID_INFO("Output mode: progressive\n");
		pEdidTiming->bInterlace = MT_FALSE;
	}

	/*Sync Signal Definitions Type*/
	if (0 == (pDetailed->flags & 0x10)) {  /*Analog Sync Signal Definitions*/
		switch ((pDetailed->flags & 0x0E) >> 1) {
			case 0x00:          /*Analog Composite Sync - Without Serrations - Sync On Green Signal only*/
				EDID_INFO("sync acs ws green\n");
				break;

			case 0x01:                  /*Analog Composite Sync - Without Serrations - Sync On all three (RGB) video signals*/
				EDID_INFO("sync acs ws all\n");
				break;

			case 0x02:                  /*Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On Green Signal only*/
				EDID_INFO("sync acs ds green\n");
				break;

			case 0x03:                  /*Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On all three (RGB) video signals*/
				EDID_INFO("sync acs ds all\n");
				break;

			case 0x04:                  /*Bipolar Analog Composite Sync - Without Serrations; - Sync On Green Signal only*/
				EDID_INFO("sync bacs ws green\n");
				break;

			case 0x05:                  /*Bipolar Analog Composite Sync - Without Serrations; - Sync On all three (RGB) video signals*/
				EDID_INFO("sync bacs ws all\n");
				break;

			case 0x06:                  /*Bipolar Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On Green Signal only*/
				EDID_INFO("sync bacs ds green\n");
				break;

			case 0x07:                  /*Bipolar Analog Composite Sync - With Serrations (H-sync during V-sync); - Sync On all three (RGB) video signals*/
				EDID_INFO("sync bacs ds all\n");
				break;
			default:
				break;

		}
	} else {            /*Digital Sync Signal Definitions*/
		switch ((pDetailed->flags & 0x0E) >> 1) {
			case 0x01:
			case 0x00:                  /*Digital Composite Sync - Without Serrations*/
				//new_customer->Sync_type = MT_UNF_EDID_SYNC_DCS_WS;
				pEdidTiming->bIHS = 0;
				pEdidTiming->bIVS = 0;
				break;

			case 0x02:                   /*Digital Composite Sync - With Serrations (H-sync during V-sync)*/
			case 0x03:
				//new_customer->Sync_type = MT_UNF_EDID_SYNC_DCS_DS;
				break;

			case 0x04:                  /*Digital Separate Sync Vsync(-) Hsync(-)*/
				//new_customer->Sync_type = MT_UNF_EDID_SYNC_DSS_VN_HN;
				pEdidTiming->bIHS = 0;
				pEdidTiming->bIVS = 0;
				break;

			case 0x05:                  /*Digital Separate Sync Vsync(-) Hsync(+)*/
				//new_customer->Sync_type = MT_UNF_EDID_SYNC_DSS_VN_HP;
				pEdidTiming->bIHS = 1;
				pEdidTiming->bIVS = 0;
				break;

			case 0x06:                  /*Digital Separate Sync Vsync(+) Hsync(-)*/
				//new_customer->Sync_type = MT_UNF_EDID_SYNC_DSS_VP_HN;
				pEdidTiming->bIHS = 0;
				pEdidTiming->bIVS = 1;
				break;

			case 0x07:                  /*Digital Separate Sync Vsync(+) Hsync(+)*/
				//	new_customer->Sync_type = MT_UNF_EDID_SYNC_DSS_VP_HP;
				pEdidTiming->bIHS = 1;
				pEdidTiming->bIVS = 1;
				break;

			default:
				break;

		}
	}
	pEdidTiming->bIDV = 0;
	/*Stereo Viewing Support*/
	switch (((pDetailed->flags & 0x60) >> 4) | (pDetailed->flags & 0x01)) {
		case 0x02:
			EDID_INFO("stereo sequential R\n");
			break;

		case 0x04:
			EDID_INFO("stereo sequential L\n");
			break;

		case 0x03:
			EDID_INFO("stereo interleaved 2R\n");
			break;

		case 0x05:
			EDID_INFO("stereo interleaved 2L\n");
			break;

		case 0x06:
			EDID_INFO("stereo interleaved 4\n");
			break;

		case 0x07:
			EDID_INFO("stereo interleaved SBS\n");
			break;

		default:
			EDID_INFO("stereo no\n");
			break;

	}
	return MT_SUCCESS;
}

static void sParseStandardTimingIDs(ParseData_t* p)
{
	uint8_t TmpVal, i;
	uint32_t Hor, Ver, aspect_ratio, freq;
	uint8_t* pData = sRawPointerGet(p, 0, 0x26);

	for (i = 0; i < STANDARDTIMING_SIZE; i += 2) {
		if ((pData[i] == 0x01) && (pData[i + 1] == 0x01)) {
			EDID_INFO("Mode %d wasn't defined! \n", (int)pData[i]);
		} else {
			MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
			Hor = (pData[i] + 31) * 8;
			EDID_INFO(" Hor Act pixels %d,0x%x \n", Hor,pData[i + 1]);
			TmpVal = pData[i + 1] & 0xC0;
			if (TmpVal == 0x00) {
				EDID_INFO("Aspect ratio:16:10\n");
				aspect_ratio = _16_10;
				Ver = Hor * 10 / 16;
			} else  if (TmpVal == 0x40) {
				EDID_INFO("Aspect ratio:4:3\n");
				aspect_ratio = _4;
				Ver = Hor * 3 / 4;
			} else  if (TmpVal == 0x80) {
				EDID_INFO("Aspect ratio:5:4\n");
				aspect_ratio = _5_4;
				Ver = Hor * 4 / 5;
			} else { //0xc0
				EDID_INFO("Aspect ratio:16:9\n");
				aspect_ratio = _16;
				Ver = Hor * 9 / 16;
			}
			freq = ((pData[i + 1]) & 0x3F) + 60;
			EDID_INFO(" Refresh rate %d Hz \n", freq);

			if (freq == 60) {
				if ((Hor == 1280) && (Ver == 720)) {
					EDID_INFO("1280X720 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1280X720_60] = MT_TRUE;
				} else if ((Hor == 1280) && (Ver == 800)) {
					EDID_INFO("1280X800_RB \n");
					//pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1280X800_60] = MT_TRUE;
				} else if ((Hor == 1280) && (Ver == 1024)) {
					EDID_INFO("1280X1024 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1280X1024_60] = MT_TRUE;
				} else if ((Hor == 1360) && (Ver == 768)) {
					EDID_INFO("1360X768 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1360X768_60] = MT_TRUE;
				} else if ((Hor == 1366) && (Ver == 768)) {
					EDID_INFO("1366X768 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1366X768_60] = MT_TRUE;
				} else if ((Hor == 1400) && (Ver == 1050)) {
					EDID_INFO("1400X1050_RB \n");
					//pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1400X1050_60] = MT_TRUE;
				} else if ((Hor == 1440) && (Ver == 900)) {
					EDID_INFO("1440X900_RB \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1440X900_60_RB] = MT_TRUE;
				} else if ((Hor == 1600) && (Ver == 900)) {
					EDID_INFO("1600X900_RB \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1600X900_60_RB] = MT_TRUE;
				} else if ((Hor == 1600) && (Ver == 1200)) {
					EDID_INFO("1600X1200 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1600X1200_60] = MT_TRUE;
				} else if ((Hor == 1680) && (Ver == 1050)) {
					EDID_INFO("1680X1050_RB \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1680X1050_60_RB] = MT_TRUE;
				} else if ((Hor == 1920) && (Ver == 1080)) {
					EDID_INFO("1920X1080 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1920X1080_60] = MT_TRUE;
				} else if ((Hor == 1920) && (Ver == 1200)) {
					EDID_INFO("1920X1200_RB \n");
					//pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1920X1200_60] = MT_TRUE;
				} else if ((Hor == 1920) && (Ver == 1440)) {
					EDID_INFO("1920X1440 \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_1920X1440_60] = MT_TRUE;
				} else if ((Hor == 2048) && (Ver == 1152)) {
					EDID_INFO("2048X1152_RB \n");
					//pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_2048X1152_60] = MT_TRUE;
				} else if ((Hor == 2560) && (Ver == 1440)) {
					EDID_INFO("2560X1440_RB \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_2560X1440_60_RB] = MT_TRUE;
				} else if ((Hor == 2560) && (Ver == 1600)) {
					EDID_INFO("2560X1600_RB \n");
					pSinkCap->bSupportFormat[MT_UNF_ENC_FMT_VESA_2560X1600_60_RB] = MT_TRUE;
				}
			}
		}
	}
}
