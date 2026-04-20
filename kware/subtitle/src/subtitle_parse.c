#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "mt_type.h"
#include "subtitle_debug.h"
#include "subtitle_data.h"
#include "subtitle_parse.h"

#define SUBT_REGION_SHOW_HD_LEN     (600*1024)
#define SUBT_REGION_SHOW_SD_LEN     (200*1024)
#define MIN_SUBT_PES_LEN (4)
/*#define MAX_DATA_LENGTH (720 * 576)*/
#define MAX_DATA_LENGTH (1920 * 1080)
#define MAX_REGION_WIDTH (1920)
#define MAX_REGION_HEIGTH (1080)

#define CLUT_BIT_4 (4)
#define CLUT_BIT_16 (16)
#define CLUT_BIT_256 (256)

#define SUBT_SEGMENT_IN_PES_MAX     50
#define SUBT_REGN_IN_PAGE_NUM       10
#define SUBT_OBJ_NUM_IN_REGN_MAX    50
#define SUBT_CLUT_IN_PAGE_MAX       (SUBT_REGN_IN_PAGE_NUM)


/* Ref[Subtitling system.pdf], section 7.2 table 2, for segment type*/
#define SUBT_PAGE_SEGMENT       0x10
#define SUBT_REGION_SEGMENT     0x11
#define SUBT_CLUT_SEGMENT       0x12
#define SUBT_OBJECT_SEGMENT     0x13
#define SUBT_DISPLAY_DEFINITION_SEGMENT 0x14
#define SUBT_DISPARITY_SIGNALLING_SEGMENT 0x15
#define SUBT_END_SEGMENT        0x80
#define SUBT_STUFFING_SEGMENT   0xFF
#define SUBT_SHIFT_NUM          (20)
#define SUBT_DISPARITY_NUM      (20)
#define SUBT_SUBREGIONDATA_NUM  (4)

#define SUBT_REGION_LINE_PIXEL_ADJUST_NUM 2

/* Color struct */
typedef struct tagSUBT_COLOR_S
{
    mt_u8 u8Blue;
    mt_u8 u8Green;
    mt_u8 u8Red;
    mt_u8 u8Alpha;
}SUBT_COLOR_S;


/* Subtitle object,  Object info in Region */
typedef struct tagSUBT_OBJECT_INFO_IN_REGION_S
{
    mt_u16          u16ObjectId;   /* Identifies an object */
    SUBT_OBJ_TYPE_E enObjectType;  /* Identifies the type of object */
    mt_u16          u16ObjectLeft; /* relative to the associated region */
    mt_u16          u16ObjectTop;  /* relative to the associated region */
    mt_u8           u8FrontClr;
    mt_u8           u8BackClr;
}SUBT_OBJECT_INFO_IN_REGION_S;


typedef struct tagSUBT_SHOW_REGION_S
{
    mt_u8  u8RegionId;
    mt_u16 u16XPos;
    mt_u16 u16YPos;
    mt_u16 u16Width;
    mt_u16 u16Heigth;

    mt_u8    u8RegionDepth;        /* Ref[] 7.2.2 Table 5. 0x01-2bit, 0x02-4bit, 0x03-8bit */
    mt_u8    u8ClutId;
    mt_u16   u16RegionClutLen;
    mt_void* pvRegionClut;

    mt_u16 u16RegionDataSize;
    mt_u8* pu8ShowData;

    SUBT_OBJ_TYPE_E enObjType;
    mt_u16 u16CharacterNum;
    mt_u8  u8FrontClr;
    mt_u8  u8BackClr;

    struct tagSUBT_SHOW_REGION_S *pstNextRegion;
}SUBT_SHOW_REGION_S;


/* Regin info in page */
typedef struct tagSUBT_REGION_INFO_IN_PAGE_S
{
    mt_u8   u8RegionId;
    mt_u16  u16RegionLeft;
    mt_u16  u16RegionTop;
    mt_u8   u8InPageVer;
}SUBT_REGION_INFO_IN_PAGE_S;

/* all info in region */
typedef struct tagSUBT_ALL_INFO_IN_REGION_S
{
    mt_u8  u8RegionID;           /* 8-bit field uniquely identifies the region */
    mt_u8  u8RegionFillFlag;     /* 1 means for filling the background color */
    mt_u16 u16RegionWidth;       /* range from 1 to 720 */
    mt_u16 u16RegionHeight;      /* range from 1 to 576 */
    mt_u8  u8RegionClutSel;      /* 0x01-2bit/entry, 0x02-4bit/entry, 0x03-8bit/entry. Ref[Subtitling system.pdf] section 7.2.2 Table 4*/
    mt_u8  u8RegionDepth;        /* Ref[] 7.2.2 Table 5. 0x01-2bit, 0x02-4bit, 0x03-8bit */
    mt_u8  u8ClutId;             /* Identifies the family of CLUTs that applies to this region */
    mt_u8  u8BackClr;            /* background colour */
    mt_u8  u8ObjectNum;
    SUBT_OBJECT_INFO_IN_REGION_S stObjectInfo[SUBT_OBJ_NUM_IN_REGN_MAX];
}SUBT_ALL_INFO_IN_REGION_S;


typedef struct tagSUBT_REFERING_REGION_S
{
    mt_u8     u8RegionDepth;
    mt_u16    u16RegionWidth;
    mt_u16    u16RegionHeight;
    mt_u8    *pu8RegionPixel;
    mt_u16    u16ObjVerPos;
    mt_u16    u16ObjHorPos;
    mt_u8    *pu8ObjPixelWrite;
	mt_u32 u32RegionLen;
}SUBT_REFERING_REGION_S;

typedef struct tagSUBT_DISPLAY_DEFINITION_S
{
    mt_u8  u8DdsVersionNumber;
    mt_u8  u8DisplayWindowFlag;
    mt_u16 u16DisplayWidth;
    mt_u16 u16DisplayHeight;
    mt_u16 u16DisplayWindowHorizontalPositionMinimum;
    mt_u16 u16DisplayWindowHorizontalPositionMaximum;
    mt_u16 u16DisplayWindowVerticalPositionMinimum;
    mt_u16 u16DisplayWindowVerticalPositionMaximum;
    mt_u16 u16PageID;

}SUBT_DISPLAY_DEFINITION_S;

typedef struct tagSUBT_DISPARITY_SHIFT_DATA_S
{
    mt_u8 u8IntervalCount;
    mt_u8 u8DisparityShiftUpdateIntegerPart;
}SUBT_DISPARITY_SHIFT_DATA_S;

typedef struct tagSUBT_DISPARITY_SHIFT_UPDATE_SEQUENCE_S
{
    mt_u8  u8DisparityShiftUpdateSequenceLength;
    mt_u8  u8DivisionPeriodCount;
    mt_u32 u32IntervalDuration;
    SUBT_DISPARITY_SHIFT_DATA_S stDisparityShiftData[SUBT_SHIFT_NUM];

}SUBT_DISPARITY_SHIFT_UPDATE_SEQUENCE_S;

typedef struct tagSUBT_DISPARITY_SUBREGION_DATA_S
{
    mt_u16 u16SubregionHorizontalPosition;
    mt_u16 u16SubregionWidth;
    mt_u8  u8SubregionDisparityShiftIntegerPart;
    mt_u8  u8SubregionDisparityShiftFractionalPart;
    SUBT_DISPARITY_SHIFT_UPDATE_SEQUENCE_S stDisparityShiftUpdateSequence;
}SUBT_DISPARITY_SUBREGION_DATA_S;

typedef struct tagSUBT_DISPARITY_S
{
    mt_u8 u8RegionID;
    mt_u8 u8DisparityShiftUpdateSequenceRegionFlag;
    mt_u8 u8NumberOfSubregionsMinus1;
    SUBT_DISPARITY_SUBREGION_DATA_S stDisparitySubregionData[SUBT_SUBREGIONDATA_NUM];
}SUBT_DISPARITY_S;

typedef struct tagSUBT_DISPARITY_SIGNALLING_SEGMENT_S
{
    mt_u8 u8dss_version_number;
    mt_u8 u8disparity_shift_update_sequence_page_flag;
    mt_u8 u8page_default_disparity_shift;
    SUBT_DISPARITY_SHIFT_UPDATE_SEQUENCE_S stDisparityShiftUpdateSequence;
    SUBT_DISPARITY_S stDisparity[SUBT_DISPARITY_NUM];
}SUBT_DISPARITY_SIGNALLING_SEGMENT_S;

/* subtitle segment*/
typedef struct tagSUBT_SEGMENT_S
{
    mt_u8    u8SegmentType;
    mt_u16   u16DataLength;
    mt_u8*   pu8SegmentData;

    struct tagSUBT_SEGMENT_S* pstNext;
    mt_u8 data_status;
}SUBT_SEGMENT_S;

typedef struct tagSUBT_CLUT_S
{
    mt_u8 u8ClutId;
    mt_u8 u8ClutVersion;
    SUBT_COLOR_S _2BitCLUT[CLUT_BIT_4];
    SUBT_COLOR_S _4BitCLUT[CLUT_BIT_16];
    SUBT_COLOR_S _8BitCLUT[CLUT_BIT_256];
} SUBT_CLUT_S;

typedef struct tagSUBT_PAGE_PARAM_S
{
    mt_u16                      u16PageId;   /* reserved for future */
    mt_u32                      u32PTS;
    mt_u8                       u8ShowTime;  /* in seconds */
    mt_u8                       u8PageVersion; /* The version of this page composition segment */
    SUBT_PAGE_STATE_E           enPageState; /* signals the status of the subtitling page */
    SUBT_SEGMENT_S              astSegment[SUBT_SEGMENT_IN_PES_MAX];
    SUBT_CLUT_S                 stDefaultClut;
    SUBT_CLUT_S                 astClut[SUBT_CLUT_IN_PAGE_MAX];
    SUBT_SHOW_REGION_S*         pstShowRegion;
    SUBT_DISPLAY_DEFINITION_S   stDisplayDef;
    SUBT_DISPARITY_SIGNALLING_SEGMENT_S stSignallingSegment;
}SUBT_PAGE_PARAM_S;

typedef struct tagSUBT_PARSE_INFO_S
{
    SUBT_PAGE_PARAM_S stPageParam;

    mt_u8 *pu8RegionBuffer;
    mt_u32 u32RegionBufferLen;

    mt_u8* pu8RegionShowBuf;
    mt_u32 u32RegionShowBufLen;

    SUBT_DATAPARSE_CALLBACK_FN pfnCallback;
    ulong u32UserData;
    mt_u8 error_status;
}SUBT_PARSE_INFO_S;


static mt_s32 SUBT_Parse_ParseSegment(SUBT_PARSE_INFO_S *pstParseInfo);
static mt_s32 SUBT_Parse_ParsePageSegment(SUBT_PARSE_INFO_S *pstParseInfo, SUBT_SEGMENT_S *pstSegment,SUBT_REGION_INFO_IN_PAGE_S *pstRegion,mt_u8 *pu8RegionNum);
static mt_void SUBT_Parse_ParseClutSegment(SUBT_PARSE_INFO_S *pstParseInfo, SUBT_SEGMENT_S *pstSegment, SUBT_ALL_INFO_IN_REGION_S *pstRegionInfo);
static mt_s32 SUBT_Parse_ParseObjectSegment(SUBT_PARSE_INFO_S *pstParseInfo, SUBT_ALL_INFO_IN_REGION_S *pstRegion,SUBT_SEGMENT_S *pstSegment,SUBT_OBJECT_INFO_IN_REGION_S *pstObjectInfoInRegion);
static mt_s32 SUBT_DataParse_Display(SUBT_PARSE_INFO_S *pstParseInfo);


static  const SUBT_COLOR_S s_2BitDefaultCLUT[] =
{
    {0x00,0x00,0x00,0xff},
    {0xff,0xff,0xff,0x00},
    {0x00,0x00,0x00,0x00},
    {0x80,0x80,0x80,0x00}
};

static  const SUBT_COLOR_S s_4BitDefaultCLUT[] =
{
    {0x0, 0x0, 0x00, 0xff}, {0xff, 0x0, 0x00, 0x0}, {0x0, 0xff, 0x00, 0x0}, {0xff, 0xff, 0x00, 0x0},
    {0x0, 0x0, 0xff, 0x0 }, {0xff, 0x0, 0xff, 0x0}, {0x0, 0xff, 0xff, 0x0}, {0xff, 0xff, 0xff, 0x0},
    {0x0, 0x0, 0x00, 0x0 }, {0x80, 0x0, 0x00, 0x0}, {0x0, 0x80, 0x00, 0x0}, {0x80, 0x80, 0x00, 0x0},
    {0x0, 0x0, 0x80, 0x0 }, {0x80, 0x0, 0x80, 0x0}, {0x0, 0x80, 0x80, 0x0}, {0x80, 0x80, 0x80, 0x0},
};

static  const SUBT_COLOR_S s_8BitDefaultCLUT[] =
{
    {0x00, 0x00, 0x00, 0xff}, {0xff, 0x00, 0x00, 0xbf}, {0x00, 0xff, 0x00, 0xbf}, {0xff, 0xff, 0x00, 0xbf},
    {0x00, 0x00, 0xff, 0xbf}, {0xff, 0x00, 0xff, 0xbf}, {0x00, 0xff, 0xff, 0xbf}, {0xff, 0xff, 0xff, 0xbf},
    {0x00, 0x00, 0x00, 0x80}, {0x55, 0x00, 0x00, 0x80}, {0x00, 0x55, 0x00, 0x80}, {0x55, 0x55, 0x00, 0x80},
    {0x00, 0x00, 0x55, 0x80}, {0x55, 0x00, 0x55, 0x80}, {0x0 , 0x55, 0x55, 0x80}, {0x55, 0x55, 0x55, 0x80},
    {0xaa, 0x00, 0x00, 0x00}, {0xff, 0x00, 0x00, 0x00}, {0xaa, 0x55, 0x00, 0x00}, {0xff, 0x55, 0x00, 0x00},
    {0xaa, 0x00, 0x55, 0x00}, {0xff, 0x00, 0x55, 0x00}, {0xaa, 0x55, 0x55, 0x00}, {0xff, 0x55, 0x55, 0x00},
    {0xaa, 0x00, 0x00, 0x80}, {0xff, 0x00, 0x00, 0x80}, {0xaa, 0x55, 0x00, 0x80}, {0xff, 0x55, 0x00, 0x80},
    {0xaa, 0x00, 0x55, 0x80}, {0xff, 0x00, 0x55, 0x80}, {0xaa, 0x55, 0x55, 0x80}, {0xff, 0x55, 0x55, 0x80},
    {0x00, 0xaa, 0x00, 0x00}, {0x55, 0xaa, 0x00, 0x00}, {0x00, 0xff, 0x00, 0x00}, {0x55, 0xff, 0x00, 0x00},
    {0x00, 0xaa, 0x55, 0x00}, {0x55, 0xaa, 0x55, 0x00}, {0x00, 0xff, 0x55, 0x00}, {0x55, 0xff, 0x55, 0x00},
    {0x00, 0xaa, 0x00, 0x80}, {0x55, 0xaa, 0x00, 0x80}, {0x00, 0xff, 0x00, 0x80}, {0x55, 0xff, 0x00, 0x80},
    {0x00, 0xaa, 0x55, 0x80}, {0x55, 0xaa, 0x55, 0x80}, {0x00, 0xff, 0x55, 0x80}, {0x55, 0xff, 0x55, 0x80},
    {0xaa, 0xaa, 0x00, 0x00}, {0xff, 0xaa, 0x00, 0x00}, {0xaa, 0xff, 0x00, 0x00}, {0xff, 0xff, 0x00, 0x00},
    {0xaa, 0xaa, 0x55, 0x00}, {0xff, 0xaa, 0x55, 0x00}, {0xaa, 0xff, 0x55, 0x00}, {0xff, 0xff, 0x55, 0x00},
    {0xaa, 0xaa, 0x00, 0x80}, {0xff, 0xaa, 0x00, 0x80}, {0xaa, 0xff, 0x00, 0x80}, {0xff, 0xff, 0x00, 0x80},
    {0xaa, 0xaa, 0x55, 0x80}, {0xff, 0xaa, 0x55, 0x80}, {0xaa, 0xff, 0x55, 0x80}, {0xff, 0xff, 0x55, 0x80},
    {0x00, 0x00, 0xaa, 0x00}, {0x55, 0x00, 0xaa, 0x00}, {0x00, 0x55, 0xaa, 0x00}, {0x55, 0x55, 0xaa, 0x00},
    {0x00, 0x00, 0xff, 0x00}, {0x55, 0x00, 0xff, 0x00}, {0x00, 0x55, 0xff, 0x00}, {0x55, 0x55, 0xff, 0x00},
    {0x00, 0x00, 0xaa, 0x80}, {0x55, 0x00, 0xaa, 0x80}, {0x00, 0x55, 0xaa, 0x80}, {0x55, 0x55, 0xaa, 0x80},
    {0x00, 0x00, 0xff, 0x80}, {0x55, 0x00, 0xff, 0x80}, {0x00, 0x55, 0xff, 0x80}, {0x55, 0x55, 0xff, 0x80},
    {0xaa, 0x00, 0xaa, 0x00}, {0xff, 0x00, 0xaa, 0x00}, {0xaa, 0x55, 0xaa, 0x00}, {0xff, 0x55, 0xaa, 0x00},
    {0xaa, 0x00, 0xff, 0x00}, {0xff, 0x00, 0xff, 0x00}, {0xaa, 0x55, 0xff, 0x00}, {0xff, 0x55, 0xff, 0x00},
    {0xaa, 0x00, 0xaa, 0x80}, {0xff, 0x00, 0xaa, 0x80}, {0xaa, 0x55, 0xaa, 0x80}, {0xff, 0x55, 0xaa, 0x80},
    {0xaa, 0x00, 0xff, 0x80}, {0xff, 0x00, 0xff, 0x80}, {0xaa, 0x55, 0xff, 0x80}, {0xff, 0x55, 0xff, 0x80},
    {0x00, 0xaa, 0xaa, 0x00}, {0x55, 0xaa, 0xaa, 0x00}, {0x00, 0xff, 0xaa, 0x00}, {0x55, 0xff, 0xaa, 0x00},
    {0x00, 0xaa, 0xff, 0x00}, {0x55, 0xaa, 0xff, 0x00}, {0x00, 0xff, 0xff, 0x00}, {0x55, 0xff, 0xff, 0x00},
    {0x00, 0xaa, 0xaa, 0x80}, {0x55, 0xaa, 0xaa, 0x80}, {0x00, 0xff, 0xaa, 0x80}, {0x55, 0xff, 0xaa, 0x80},
    {0x00, 0xaa, 0xff, 0x80}, {0x55, 0xaa, 0xff, 0x80}, {0x00, 0xff, 0xff, 0x80}, {0x55, 0xff, 0xff, 0x80},
    {0xaa, 0xaa, 0xaa, 0x00}, {0xff, 0xaa, 0xaa, 0x00}, {0xaa, 0xff, 0xaa, 0x00}, {0xff, 0xff, 0xaa, 0x00},
    {0xaa, 0xaa, 0xff, 0x00}, {0xff, 0xaa, 0xff, 0x00}, {0xaa, 0xff, 0xff, 0x00}, {0xff, 0xff, 0xff, 0x00},
    {0xaa, 0xaa, 0xaa, 0x80}, {0xff, 0xaa, 0xaa, 0x80}, {0xaa, 0xff, 0xaa, 0x80}, {0xff, 0xff, 0xaa, 0x80},
    {0xaa, 0xaa, 0xff, 0x80}, {0xff, 0xaa, 0xff, 0x80}, {0xaa, 0xff, 0xff, 0x80}, {0xff, 0xff, 0xff, 0x80},
    {0x80, 0x80, 0x80, 0x00}, {0xaa, 0x80, 0x80, 0x00}, {0x80, 0xaa, 0x80, 0x00}, {0xaa, 0xaa, 0x80, 0x00},
    {0x80, 0x80, 0xaa, 0x00}, {0xaa, 0x80, 0xaa, 0x00}, {0x80, 0xaa, 0xaa, 0x00}, {0xaa, 0xaa, 0xaa, 0x00},
    {0x00, 0x00, 0x00, 0x00}, {0x2a, 0x00, 0x00, 0x00}, {0x00, 0x2a, 0x00, 0x00}, {0x2a, 0x2a, 0x00, 0x00},
    {0x00, 0x00, 0x2a, 0x00}, {0x2a, 0x00, 0x2a, 0x00}, {0x00, 0x2a, 0x2a, 0x00}, {0x2a, 0x2a, 0x2a, 0x00},
    {0xd5, 0x80, 0x80, 0x00}, {0xff, 0x80, 0x80, 0x00}, {0xd5, 0xaa, 0x80, 0x00}, {0xff, 0xaa, 0x80, 0x00},
    {0xd5, 0x80, 0xaa, 0x00}, {0xff, 0x80, 0xaa, 0x00}, {0xd5, 0xaa, 0xaa, 0x00}, {0xff, 0xaa, 0xaa, 0x00},
    {0x55, 0x00, 0x00, 0x00}, {0x7f, 0x00, 0x00, 0x00}, {0x55, 0x2a, 0x00, 0x00}, {0x7f, 0x2a, 0x00, 0x00},
    {0x55, 0x00, 0x2a, 0x00}, {0x7f, 0x00, 0x2a, 0x00}, {0x55, 0x2a, 0x2a, 0x00}, {0x7f, 0x2a, 0x2a, 0x00},
    {0x80, 0xd5, 0x80, 0x00}, {0xaa, 0xd5, 0x80, 0x00}, {0x80, 0xff, 0x80, 0x00}, {0xaa, 0xff, 0x80, 0x00},
    {0x80, 0xd5, 0xaa, 0x00}, {0xaa, 0xd5, 0xaa, 0x00}, {0x80, 0xff, 0xaa, 0x00}, {0xaa, 0xff, 0xaa, 0x00},
    {0x00, 0x55, 0x00, 0x00}, {0x2a, 0x55, 0x00, 0x00}, {0x00, 0x7f, 0x00, 0x00}, {0x2a, 0x7f, 0x00, 0x00},
    {0x00, 0x55, 0x2a, 0x00}, {0x2a, 0x55, 0x2a, 0x00}, {0x00, 0x7f, 0x2a, 0x00}, {0x2a, 0x7f, 0x2a, 0x00},
    {0xd5, 0xd5, 0x80, 0x00}, {0xff, 0xd5, 0x80, 0x00}, {0xd5, 0xff, 0x80, 0x00}, {0xff, 0xff, 0x80, 0x00},
    {0xd5, 0xd5, 0xaa, 0x00}, {0xff, 0xd5, 0xaa, 0x00}, {0xd5, 0xff, 0xaa, 0x00}, {0xff, 0xff, 0xaa, 0x00},
    {0x55, 0x55, 0x00, 0x00}, {0x7f, 0x55, 0x00, 0x00}, {0x55, 0x7f, 0x00, 0x00}, {0x7f, 0x7f, 0x00, 0x00},
    {0x55, 0x55, 0x2a, 0x00}, {0x7f, 0x55, 0x2a, 0x00}, {0x55, 0x7f, 0x2a, 0x00}, {0x7f, 0x7f, 0x2a, 0x00},
    {0x80, 0x80, 0xd5, 0x00}, {0xaa, 0x80, 0xd5, 0x00}, {0x80, 0xaa, 0xd5, 0x00}, {0xaa, 0xaa, 0xd5, 0x00},
    {0x80, 0x80, 0xff, 0x00}, {0xaa, 0x80, 0xff, 0x00}, {0x80, 0xaa, 0xff, 0x00}, {0xaa, 0xaa, 0xff, 0x00},
    {0x00, 0x00, 0x55, 0x00}, {0x2a, 0x00, 0x55, 0x00}, {0x00, 0x2a, 0x55, 0x00}, {0x2a, 0x2a, 0x55, 0x00},
    {0x00, 0x00, 0x7f, 0x00}, {0x2a, 0x00, 0x7f, 0x00}, {0x00, 0x2a, 0x7f, 0x00}, {0x2a, 0x2a, 0x7f, 0x00},
    {0xd5, 0x80, 0xd5, 0x00}, {0xff, 0x80, 0xd5, 0x00}, {0xd5, 0xaa, 0xd5, 0x00}, {0xff, 0xaa, 0xd5, 0x00},
    {0xd5, 0x80, 0xff, 0x00}, {0xff, 0x80, 0xff, 0x00}, {0xd5, 0xaa, 0xff, 0x00}, {0xff, 0xaa, 0xff, 0x00},
    {0x55, 0x00, 0x55, 0x00}, {0x7f, 0x00, 0x55, 0x00}, {0x55, 0x2a, 0x55, 0x00}, {0x7f, 0x2a, 0x55, 0x00},
    {0x55, 0x00, 0x7f, 0x00}, {0x7f, 0x00, 0x7f, 0x00}, {0x55, 0x2a, 0x7f, 0x00}, {0x7f, 0x2a, 0x7f, 0x00},
    {0x80, 0xd5, 0xd5, 0x00}, {0xaa, 0xd5, 0xd5, 0x00}, {0x80, 0xff, 0xd5, 0x00}, {0xaa, 0xff, 0xd5, 0x00},
    {0x80, 0xd5, 0xff, 0x00}, {0xaa, 0xd5, 0xff, 0x00}, {0x80, 0xff, 0xff, 0x00}, {0xaa, 0xff, 0xff, 0x00},
    {0x00, 0x55, 0x55, 0x00}, {0x2a, 0x55, 0x55, 0x00}, {0x00, 0x7f, 0x55, 0x00}, {0x2a, 0x7f, 0x55, 0x00},
    {0x00, 0x55, 0x7f, 0x00}, {0x2a, 0x55, 0x7f, 0x00}, {0x00, 0x7f, 0x7f, 0x00}, {0x2a, 0x7f, 0x7f, 0x00},
    {0xd5, 0xd5, 0xd5, 0x00}, {0xff, 0xd5, 0xd5, 0x00}, {0xd5, 0xff, 0xd5, 0x00}, {0xff, 0xff, 0xd5, 0x00},
    {0xd5, 0xd5, 0xff, 0x00}, {0xff, 0xd5, 0xff, 0x00}, {0xd5, 0xff, 0xff, 0x00}, {0xff, 0xff, 0xff, 0x00},
    {0x55, 0x55, 0x55, 0x00}, {0x7f, 0x55, 0x55, 0x00}, {0x55, 0x7f, 0x55, 0x00}, {0x7f, 0x7f, 0x55, 0x00},
    {0x55, 0x55, 0x7f, 0x00}, {0x7f, 0x55, 0x7f, 0x00}, {0x55, 0x7f, 0x7f, 0x00}, {0x7f, 0x7f, 0x7f, 0x00},
};


static mt_u32 YUV2RGB(mt_u8 Y, mt_u8 Cr, mt_u8 Cb)
{
    mt_s16 R, G, B;
    mt_s16 C, D, E;

    C = (mt_s16)(Y - 16);   //Y
    D = (mt_s16)(Cr - 128); //U
    E = (mt_s16)(Cb - 128); //V

    R = (mt_s16)(((298 * C) + (409 * E) + 128) >> 8);
    G = (mt_s16)(((298 * C) - (100 * D) - (208 * E) + 128) >> 8);
    B = (mt_s16)(((298 * C) + (516 * D) + 128) >> 8);

    if (R > 255)
    {
        R = 255;
    }

    if (R < 0)
    {
        R = 0;
    }

    if (G > 255)
    {
        G = 255;
    }

    if (G < 0)
    {
        G = 0;
    }

    if (B > 255)
    {
        B = 255;
    }

    if (B < 0)
    {
        B = 0;
    }

    return (mt_u32)( (R << 16) | (G << 8) | B);
}


/*
 * Just only copy subtitling segment data from PES
 */
static mt_s32 SUBT_Parse_CreateSegment(SUBT_PARSE_INFO_S* pstParseInfo, mt_u8 *pu8SegmentDataSrc , mt_u32 u32DataLen, mt_u16 u16PageID, mt_u16 u16AncillaryID)
{
    mt_u8 *pu8SegtData = pu8SegmentDataSrc;
    mt_u32 u32ProcessedLen = 0;
    mt_u16 u16ParsedPageId = 0;
    SUBT_SEGMENT_S *pstSegment = NULL;
    SUBT_PAGE_PARAM_S *pstCurPage = &pstParseInfo->stPageParam;

    pstSegment = pstCurPage->astSegment;    

    if (pstSegment == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }
    pstSegment->u16DataLength = 0;

    /* PES_data_field() {
     *   data_identifier                    (8 bslbf)
     *   subtitle_stream_id               (8 bslbf)
     *   while nextbits() == '0000 1111' {
     *       Subtitling_segment()
     *   }
     *   end_of_PES_data_field_marker (8 bslbf)
     * }
     */
    while ( pstSegment && ((u32ProcessedLen + 6) < u32DataLen) )
    {
        if(*pu8SegtData == 0x0f)//sync_byte
        {
            /*
                    * subtitle_segment()
                    * {
                    *      sync_byte                                  (8-bit)
                    *      segment_type                               (8-bit)
                    *      page_id                                    (16-bit)
                    *      segment_length                             (16-bit)
                    *      segment_data_field()
                    * }
                    */

            pu8SegtData++; /* skip the sync byte: 0x0f */

            pstSegment->u8SegmentType = *pu8SegtData; /* segment_type */

            u16ParsedPageId = (mt_u16)((pu8SegtData[1]<<8)|pu8SegtData[2]); /* page id */

            pu8SegtData += 3; /*skip segment type and page id, in total, 3 bytes */

            pstSegment->u16DataLength = (mt_u16)((pu8SegtData[0]<<8)|pu8SegtData[1]);

            u32ProcessedLen += (mt_u32)(pstSegment->u16DataLength + 6);
            if(u32ProcessedLen > u32DataLen)
            {
                MT_ERR_SUBT("Subtitle segment is wrong, throw it!!!\n");
                pstSegment->u16DataLength = 0;
                break;
            }
            pstSegment->data_status = 0;

            pu8SegtData += 2; /* skip the length field, two bytes */

            /* segment_type:
                     * 0x10 page composition segment
                     * 0x11 region composition segment
                     * 0x12 CLUT definition segment
                     * 0x13 object data segment
                     * 0x14 DSS
                     * 0x15 DDS
                     * 0x40 - 0x7F reserved for future use
                     * 0x80 end of display set segment
                     * 0x81 - 0xEF private data
                     * 0xFF stuffing (see note)
                     * All other values reserved for future use
                     */
            /*just only deal with requesting data, and the type should be in th pointed range */
            if( (u16ParsedPageId == u16PageID || u16AncillaryID == u16ParsedPageId)
                && ( ( (pstSegment->u8SegmentType >= SUBT_PAGE_SEGMENT) && (pstSegment->u8SegmentType <= SUBT_DISPARITY_SIGNALLING_SEGMENT) )
                || (pstSegment->u8SegmentType == SUBT_END_SEGMENT)
                || (pstSegment->u8SegmentType == SUBT_STUFFING_SEGMENT) ) )
            {
                pstSegment->pu8SegmentData = pu8SegtData;

                /* skip the data length */
                pu8SegtData += pstSegment->u16DataLength;
                pstSegment = pstSegment->pstNext;
            }
            else
            {
                MT_INFO_SUBT("subtitle skipped (page id: %#x %#x %#x) (segment type: %#x) (data len: %#x) \n",
                             u16ParsedPageId, u16PageID, u16AncillaryID, pstSegment->u8SegmentType, pstSegment->u16DataLength);
                pstSegment->u8SegmentType = 0;
                pstSegment->u16DataLength = 0;
                pstSegment->pu8SegmentData = MT_NULL;

                /* skip the data length */
                pu8SegtData += pstSegment->u16DataLength;
            }
        }
        else if (*pu8SegtData == 0xff) /* end of PES data field marker */
        {
            break;
        }
        else
        {
            MT_WARN_SUBT("0x%x is not sync_byte\n", *pu8SegtData);
            break;
        }
    }

    return MT_SUCCESS;
}


/* Iterate to the last valid region data */
static SUBT_SHOW_REGION_S* SUBT_Parse_GetCurPageLastRegion(SUBT_PARSE_INFO_S* pstParseInfo)
{
    SUBT_SHOW_REGION_S * pstLastRegion = NULL;
    SUBT_PAGE_PARAM_S *pstCurPage = &pstParseInfo->stPageParam;

    pstLastRegion = pstCurPage->pstShowRegion;

    while (pstLastRegion && pstLastRegion->pstNextRegion)
    {
        pstLastRegion = pstLastRegion->pstNextRegion;
    }

    return pstLastRegion;
}

static mt_s32 SUBT_Parse_CreateShowRegion(SUBT_PARSE_INFO_S* pstParseInfo, mt_u16 u16Left, mt_u16 u16Top, SUBT_ALL_INFO_IN_REGION_S *pRegion, mt_u8 u8Index)
{
    mt_u32 u32Len = 0;
    SUBT_SHOW_REGION_S * pstLastRegion = NULL;

    if(pstParseInfo == NULL || pRegion == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    /* find the last region in queue */
    pstLastRegion = SUBT_Parse_GetCurPageLastRegion(pstParseInfo);

    if (pstLastRegion)
    {
        pstLastRegion->u8RegionId = pRegion->u8RegionID;
        pstLastRegion->u16XPos = u16Left;
        pstLastRegion->u16YPos = u16Top;
        pstLastRegion->u16Width = pRegion->u16RegionWidth;
        pstLastRegion->u16Heigth = pRegion->u16RegionHeight;

        pstLastRegion->u8ClutId = pRegion->u8ClutId;
        pstLastRegion->u8RegionDepth = pRegion->u8RegionDepth;
        switch(pRegion->u8RegionDepth)
        {
            case 2:
                pstLastRegion->pvRegionClut = pstParseInfo->stPageParam.stDefaultClut._2BitCLUT;
                pstLastRegion->u16RegionClutLen = CLUT_BIT_4;
                break;
            case 4:
                pstLastRegion->pvRegionClut = pstParseInfo->stPageParam.stDefaultClut._4BitCLUT;
                pstLastRegion->u16RegionClutLen = CLUT_BIT_16;
                break;
            default:
                pstLastRegion->pvRegionClut = pstParseInfo->stPageParam.stDefaultClut._8BitCLUT;
                pstLastRegion->u16RegionClutLen = CLUT_BIT_256;
                break;
        }

        u32Len = (sizeof(SUBT_SHOW_REGION_S) +3 ) & (mt_u32)(~3);
        pstLastRegion->pu8ShowData = (mt_u8*)pstLastRegion + u32Len;
        u32Len = ((mt_u32)pstLastRegion->u16Width * (mt_u32)pstLastRegion->u16Heigth * sizeof(mt_u8) + 3) & (mt_u32)(~0x3);

        if((mt_u32)(pstLastRegion->pu8ShowData + u32Len - pstParseInfo->pu8RegionBuffer) <= pstParseInfo->u32RegionBufferLen)
        {
            if(pRegion->u8RegionFillFlag)
            {
                memset(pstLastRegion->pu8ShowData,pRegion->u8BackClr, u32Len);
            }
            else
            {
                memset(pstLastRegion->pu8ShowData, 0, u32Len);
            }
            pstLastRegion->pstNextRegion = NULL;
            pstLastRegion->u16CharacterNum = 0;
        }
        else
        {
            pstLastRegion = NULL;
            MT_ERR_SUBT("Region show buffer is not enough!!!!!\n");

            return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

static mt_s32 SUBT_Parse_SetNextRegionAddr(SUBT_PARSE_INFO_S * pstParseInfo)
{
    SUBT_SHOW_REGION_S *pstLastRegion = NULL;
    mt_u32 u32Len = 0;
    
    pstLastRegion = SUBT_Parse_GetCurPageLastRegion(pstParseInfo);
    if (NULL == pstLastRegion)
    {
        return MT_FAILURE;
    }
    
    u32Len = ((mt_u32)pstLastRegion->u16Width * (mt_u32)pstLastRegion->u16Heigth *sizeof(mt_u8) + 3) & (mt_u32)(~3);

    pstLastRegion->pstNextRegion = (SUBT_SHOW_REGION_S *)((ulong)pstLastRegion->pu8ShowData + u32Len);

    if((ulong)(pstLastRegion->pstNextRegion)-(ulong)pstParseInfo->pu8RegionBuffer <= (ulong)pstParseInfo->u32RegionBufferLen)
    {
        /* clear the show region */
        memset(pstLastRegion->pstNextRegion, 0, sizeof(SUBT_SHOW_REGION_S));
        pstLastRegion->pstNextRegion->pstNextRegion = NULL;
    }
    else
    {
        pstLastRegion->pstNextRegion = NULL;

        MT_WARN_SUBT("Region buffer is not enough, Used size is = %"PRId64", Max size is %d!!!!!\n"
                        ,(ulong)(pstLastRegion->pstNextRegion)-(ulong)pstParseInfo->pu8RegionBuffer, pstParseInfo->u32RegionBufferLen);

        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static mt_u8 ReadNext2BitPixel(mt_u8 **ppu8SegmentDataSrc, mt_u16 *pu16ProcessedLen, mt_u8 u8ResetFlag)
{
    static mt_u8 u8Get2BitNum = 0;
    static mt_u8 u8Temp = 0;
    mt_u8 u8Result = 0;

    if (u8ResetFlag)
    {
        if(u8Get2BitNum)
        {
            u8Get2BitNum = 0;
        }

        return (0);
    }

    switch (u8Get2BitNum)
    {
    case  0:
        u8Temp = **ppu8SegmentDataSrc; /* get the first byte */
        *ppu8SegmentDataSrc += 1;
        *pu16ProcessedLen = (mt_u16)(*pu16ProcessedLen + 1);
        u8Result  = u8Temp >> 6; /* Just only 2-bit */
        u8Get2BitNum = 1;
        break;
    case 1:
        u8Result  = ((u8Temp & 0x30) >> 4);
        u8Get2BitNum = 2;
        break;
    case 2:
        u8Result  = ((u8Temp & 0x0c) >> 2);
        u8Get2BitNum = 3;
        break;
    case 3:
        u8Result  = (u8Temp & 0x03);
        u8Get2BitNum = 0;
        break;
    default:
        break;
    }

    return (u8Result);
}

static mt_u8 ReadNext4BitPixel (mt_u8 **ppDataSrc,
                     mt_u16 *      pProcessedLen,
                     mt_u8         chResetFlag)
{
    static mt_u8 chGet4BitNum = 0;
    static mt_u8 chTemp = 0;
    mt_u8 chResult = 0;

    if (chResetFlag)
    {
        if(chGet4BitNum)
        {
            chGet4BitNum = 0;
        }
        return (0);
    }

    if (chGet4BitNum == 0)
    {
        chTemp = **ppDataSrc;
        *ppDataSrc += 1;
        *pProcessedLen = (mt_u16)(*pProcessedLen + 1);
        chResult  = (chTemp >> 4);
        chGet4BitNum = 1;
    }
    else
    {
        chResult  = (chTemp & 0x0f);
        chGet4BitNum = 0;
    }

    return (chResult);
}

/* ref[Subtitling system.pdf] page28, section 7.2.4.2 */
static mt_void Decode2Pixel (mt_u8 **ppu8SegmentDataSrc, mt_u8 *pu8ColorEntry, mt_u16 *pu16NumPixel, mt_u16 *pu16ProcessedLen)
{
    mt_u8 u82BitCode = ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0);

    if (u82BitCode != 0x00)
    {
        /*  01, 10, 11  */
        *pu16NumPixel   = 1;
        *pu8ColorEntry = u82BitCode;
    }
    else
    {
        u82BitCode = ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0);
        switch(u82BitCode)
        {
            case 1://switch_1 == '0',switch_2 == '1',it signals that one pixel shall be set to pseudo-colour (entry) '00'.
                /*  00 01  */
                *pu16NumPixel   = 1;
                *pu8ColorEntry = 0x00;
                break;
            case 2://switch_1 == '1'
            case 3:
                /*  00 1L LL CC  */
                *pu16NumPixel = (mt_u16)(((u82BitCode & 1) << 2) +
                            ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) + 3);
                *pu8ColorEntry = ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0);
                break;
            default://switch_1 == '0',switch_2 == '0'
                u82BitCode = ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0);//switch 3
                switch(u82BitCode)
                {
                    case 1://switch_3 == '01'
                        /*  00 00 01  */
                        *pu16NumPixel   = 2;
                        *pu8ColorEntry = 0;
                        break;
                    case 2://switch_3 == '10'
                        /*  00 00 10 LL LL CC  */
                        *pu16NumPixel = (mt_u16)((ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) << 2) +
                                    ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) + 12);
                        *pu8ColorEntry = ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0);
                        break;
                    case 3://switch_3 == '11'
                        /*  00 00 11 LL LL LL LL CC  */
                        *pu16NumPixel = (mt_u16)((ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) << 6) +
                                    (ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) << 4) +
                                    (ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) << 2) +
                                    ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0) + 29);
                        *pu8ColorEntry = ReadNext2BitPixel(ppu8SegmentDataSrc, pu16ProcessedLen, 0);
                        break;
                    default://end of 2-bit/pixel_code_string
                        /*  00 00 00 case */
                        *pu16NumPixel   = 0;
                        *pu8ColorEntry = 0;
                        break;
                }
                break;
        }
    }
}

static mt_void Decode4Pixel (mt_u8 **ppDataSrc,
                               mt_u8 *ColorEntry, mt_u16 *NumPixel,
                               mt_u16 *pProcessedLen)
{
    mt_u8 ch4BitCode = 0;

    ch4BitCode = ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0);
    if (ch4BitCode > 0x00)
    {
        *NumPixel   = 1;
        *ColorEntry = ch4BitCode;
    }
    else
    {
        ch4BitCode = ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0);

        if (ch4BitCode == 0x00)
        {
            /*   end_of_string_signal   */
            *NumPixel   = 0x00;
            *ColorEntry = 0;
        }
        else if (ch4BitCode <= 0x07)
        {
            /* run_length_3-9   */
            *NumPixel   = (mt_u16)(ch4BitCode + 2);
            *ColorEntry = 0;
        }
        else if (ch4BitCode <= 0x0b)
        {
            /* run_length_4-7   */
            *NumPixel   = (mt_u16)((ch4BitCode & 0x03) + 4);
            *ColorEntry = ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0);
        }
        else if (ch4BitCode <= 0x0d)
        {
            *NumPixel   = (mt_u16)((ch4BitCode & 0x03) + 1);
            *ColorEntry = 0;
        }
        else if (ch4BitCode == 0x0e)
        {
            /* run_length_9-24  */
            *NumPixel   = (mt_u16)(ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0) + 9);
            *ColorEntry = ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0);
        }
        else if (ch4BitCode == 0x0f)
        {
            /* run_length_25-280    */
            *NumPixel = ((ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0) << 4)
                         & 0xf0);
            *NumPixel = (mt_u16)(*NumPixel + ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0));
            *NumPixel  = (mt_u16)(*NumPixel + 25);
            *ColorEntry = ReadNext4BitPixel(ppDataSrc, pProcessedLen, 0);
        }
    }
}

static mt_void Decode8Pixel (mt_u8 **ppu8SegmentDataSrc,
                               mt_u8 *pu8ColorEntry, mt_u16 *pu16NumPixel,
                               mt_u16 *pu16ProcessedLen)
{
    mt_u8 u8BitCode = 0;

    u8BitCode = **ppu8SegmentDataSrc;
    *ppu8SegmentDataSrc += 1;

    *pu16ProcessedLen = (mt_u16)(*pu16ProcessedLen + 1);

    if (u8BitCode != 0x00)
    {
        *pu16NumPixel = 1;
        *pu8ColorEntry = u8BitCode;
    }
    else
    {
        u8BitCode = **ppu8SegmentDataSrc;
        *ppu8SegmentDataSrc += 1;
        *pu16ProcessedLen = (mt_u16)(*pu16ProcessedLen + 1);

        if (u8BitCode == 0x00)
        {
            *pu16NumPixel  = 0x00;
            *pu8ColorEntry = 0;
        }
        else if (u8BitCode <= 0x7f)
        {
            *pu16NumPixel  = u8BitCode;
            *pu8ColorEntry = 0;
        }
        else
        {
            *pu16NumPixel  = (u8BitCode & 0x7f);
            *pu8ColorEntry = **ppu8SegmentDataSrc;
            *ppu8SegmentDataSrc += 1;
            *pu16ProcessedLen = (mt_u16)(*pu16ProcessedLen + 1);
         }
    }
}

/* ref[Subtitling system.pdf] page27 section 7.2.4.1 */
static mt_s32 SUBT_Parse_PixelBlockDecode(mt_u8 *pu8SegmentDataSrc, SUBT_REFERING_REGION_S *pstReferingRegion, mt_u16 u16BlockLen)
{
    mt_u8    u8Temp = 0;
    mt_u8    cont = 0;

    mt_u8    u8DataType = 0;

    mt_u8    ColorEntry = 0;
    mt_u8    u8LastColorEntry = 0;
    mt_u16   u16NumPixel = 0;
    mt_u16   wNumLine = 0;
    mt_u16   u16PixelAdjustNum = 0;

    mt_u16   u16ProcessLen = 0;
    ulong   nEndAddLine = 0;

    mt_u8  *pu8SegmentData = pu8SegmentDataSrc;

    /* Default data from ref[Subtitling system.pdf] page36 setcion 10.4
     *
     * 2-bit to 4-bit map table
      ----------------------------------------
     |    Input value    |    Output value    |
      ----------------------------------------
     |        00         |        0000        |
      ----------------------------------------
     |        01         |        0111        |
      ----------------------------------------
     |        10         |        1000        |
      ----------------------------------------
     |        11         |        1111        |
      ----------------------------------------
     */
    mt_u8  u8szTwoToFourBitsMapTable[4] = {0x0, 0x7, 0x8, 0xf};

    /* Default data from ref[Subtitling system.pdf] page36 setcion 10.5 */
    mt_u8  u8szTwoToEightBitsMapTable[4] = {0x00, 0x77, 0x88, 0xff};

    /* Default data from ref[Subtitling system.pdf] page37 setcion 10.6 */
    mt_u8  u8szFourToEightBitsMapTable[16] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

    if (MT_NULL == pstReferingRegion || MT_NULL == pu8SegmentDataSrc)
    {
        MT_ERR_SUBT("Param Null!");
        return MT_FAILURE;
    }

    while(u16ProcessLen < u16BlockLen)
    {
        u8Temp = *pu8SegmentData; /* The first data should be data type */
        pu8SegmentData++;

        u16ProcessLen = (mt_u16)(u16ProcessLen + 1);
        u8DataType = u8Temp;
        ///printf("u8DataType ===== 0x%x\n",u8DataType);
        switch (u8DataType)
        {
            case 0x10 : /* 2-bit/pixel code string */
            {
                do
                {
                    Decode2Pixel(&pu8SegmentData, &ColorEntry,&u16NumPixel,&u16ProcessLen);

                    if (u16NumPixel != 0)
                    {
                        switch (pstReferingRegion->u8RegionDepth)
                        {
                            case 4 :
                                ColorEntry = u8szTwoToFourBitsMapTable[ColorEntry];
                                break;

                            case 8 :
                                ColorEntry = u8szTwoToEightBitsMapTable[ColorEntry];
                                break;

                            default :
                                break;
                        }


                        nEndAddLine = (ulong)pstReferingRegion->pu8RegionPixel + (mt_u32)(pstReferingRegion->u16RegionWidth *(pstReferingRegion->u16ObjVerPos + wNumLine+1));
						
						if (nEndAddLine > ((ulong)pstReferingRegion->pu8RegionPixel + pstReferingRegion->u32RegionLen)) {
							MT_ERR_SUBT("*****out of region buffer\n");
							return MT_FAILURE;
						}
						
                        if((ulong)pstReferingRegion->pu8ObjPixelWrite + u16NumPixel > nEndAddLine)//protect when object is over region
                        {
                            u16NumPixel = (mt_u16)(nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite);
                            MT_WARN_SUBT("\nobject width is over range\n");
                            memset((mt_u8 *)(nEndAddLine-pstReferingRegion->u16RegionWidth),0,pstReferingRegion->u16RegionWidth);//\2  TODO: when data wrong,clear data buffer
                             return ERR_OUT_OF_WIDTH;
                        }

                        if (u16NumPixel > 0)
                        {
                            memset(pstReferingRegion->pu8ObjPixelWrite, ColorEntry, u16NumPixel);

                            (pstReferingRegion->pu8ObjPixelWrite) += u16NumPixel;

                            u8LastColorEntry = ColorEntry;
                        }
                    }

                } while(u16NumPixel != 0 && u16ProcessLen < u16BlockLen);

                u16PixelAdjustNum = (mt_u16)((nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite) > SUBT_REGION_LINE_PIXEL_ADJUST_NUM) ? SUBT_REGION_LINE_PIXEL_ADJUST_NUM : (mt_u16)(nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite);

                memset(pstReferingRegion->pu8ObjPixelWrite, u8LastColorEntry, u16PixelAdjustNum);

                u8Temp = ReadNext2BitPixel(&pu8SegmentData, &u16ProcessLen, 1);//reset not get data

                break;
            }

            case 0x11 : /* 4-bit/pixel code string */
            {
                do
                {
                    Decode4Pixel(&pu8SegmentData, &ColorEntry,&u16NumPixel,&u16ProcessLen);

                    if (u16NumPixel != 0)
                    {
                        switch (pstReferingRegion->u8RegionDepth)
                        {
                            case 2 :
                                ColorEntry = (mt_u8)(((ColorEntry & 0x80) >>6) |
                                            ((ColorEntry & 0x70) ? 1 : 0));
                                break;

                            case 8 :
                                ColorEntry = u8szFourToEightBitsMapTable[ColorEntry];
                                break;

                            default :
                                break;
                        }
                        nEndAddLine = (ulong)pstReferingRegion->pu8RegionPixel+(mt_u32)(pstReferingRegion->u16RegionWidth *(pstReferingRegion->u16ObjVerPos + wNumLine+1));
						
						if (nEndAddLine > ((ulong)pstReferingRegion->pu8RegionPixel + pstReferingRegion->u32RegionLen)) {
							MT_ERR_SUBT("*****out of region buffer\n");
							return MT_FAILURE;
						}
						
                        if((ulong)pstReferingRegion->pu8ObjPixelWrite+u16NumPixel > nEndAddLine)//protect when object is over region
                        {
                            u16NumPixel = (mt_u16)(nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite);
                            MT_WARN_SUBT("\nobject width is over range\n");
                            return ERR_OUT_OF_WIDTH;
                            ////TODOmemset(nEndAddLine-TmpRefReg->u16RegionWidth,0,TmpRefReg->u16RegionWidth);//2  TODO: when data wrong,clear data buffer
                        }
                        if (u16NumPixel > 0)
                        {
                            memset(pstReferingRegion->pu8ObjPixelWrite, ColorEntry, u16NumPixel);

                            (pstReferingRegion->pu8ObjPixelWrite) += u16NumPixel;

                            u8LastColorEntry = ColorEntry;
                        }
                    }
                } while(u16NumPixel != 0 && u16ProcessLen < u16BlockLen);

                u16PixelAdjustNum = (mt_u16)((nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite) > SUBT_REGION_LINE_PIXEL_ADJUST_NUM) ? SUBT_REGION_LINE_PIXEL_ADJUST_NUM : (mt_u16)(nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite);

                memset(pstReferingRegion->pu8ObjPixelWrite, u8LastColorEntry, u16PixelAdjustNum);
                u8Temp = ReadNext4BitPixel(&pu8SegmentData, &u16ProcessLen,1);

                break;
            }

            case 0x12 : /* 8-bit/pixel code string */
            {
                do
                {
                    Decode8Pixel(&pu8SegmentData, &ColorEntry,
                                             &u16NumPixel,&u16ProcessLen);

                    if (u16NumPixel != 0)
                    {
                        switch (pstReferingRegion->u8RegionDepth)
                        {
                            case 2 :
                                ColorEntry=(mt_u8)(((ColorEntry & 0x80) >>6) |
                                          ((ColorEntry & 0x70) ? 1 : 0));
                                break;

                            case 4 :
                                ColorEntry=((ColorEntry & 0xf0) >> 4);
                                break;

                            default :
                                break;
                        }
                        nEndAddLine = (ulong)pstReferingRegion->pu8RegionPixel+(mt_u32)(pstReferingRegion->u16RegionWidth *(pstReferingRegion->u16ObjVerPos + wNumLine+1));
						if (nEndAddLine > ((ulong)pstReferingRegion->pu8RegionPixel + pstReferingRegion->u32RegionLen)) {
							MT_ERR_SUBT("*****out of region buffer\n");
							return MT_FAILURE;
						}
                        if((ulong)pstReferingRegion->pu8ObjPixelWrite+u16NumPixel > nEndAddLine)//protect when object is over region
                        {
                            u16NumPixel = (mt_u16)(nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite);
                            MT_WARN_SUBT("\nobject width is over range\n");
                            return ERR_OUT_OF_WIDTH;
                            ////TODO memset(nEndAddLine-TmpRefReg->u16RegionWidth,0,TmpRefReg->u16RegionWidth);//2  TODO: when data wrong,clear data buffer
                        }
                        if (u16NumPixel > 0)
                        {

                            memset(pstReferingRegion->pu8ObjPixelWrite, ColorEntry, u16NumPixel);

                            (pstReferingRegion->pu8ObjPixelWrite) += u16NumPixel;

                            u8LastColorEntry = ColorEntry;
                        }
                    }
                } while (u16NumPixel != 0 && u16ProcessLen<u16BlockLen);

                u16PixelAdjustNum = (mt_u16)((nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite) > SUBT_REGION_LINE_PIXEL_ADJUST_NUM) ? SUBT_REGION_LINE_PIXEL_ADJUST_NUM : (mt_u16)(nEndAddLine - (ulong)pstReferingRegion->pu8ObjPixelWrite);

                memset(pstReferingRegion->pu8ObjPixelWrite, u8LastColorEntry, u16PixelAdjustNum);

                break;
            }

            case 0x20 : /* 2_to_4-bit_map-table data */
            {
                for (cont = 0; cont < 4; cont++)
                {
                    u8szTwoToFourBitsMapTable[cont] = ReadNext4BitPixel(&pu8SegmentData, &u16ProcessLen,0);
                }

                u8Temp = ReadNext4BitPixel(&pu8SegmentData, &u16ProcessLen,1);
                break;
            }

            case 0x21 : /* 2_to_8-bit_map-table data */
            {
                for (cont = 0; cont < 4; cont++)
                {
                    u8szTwoToEightBitsMapTable[cont] = *pu8SegmentData;
                    pu8SegmentData++;
                    u16ProcessLen = (mt_u16)(u16ProcessLen + 1);
                }
                break;
            }

            case 0x22 :/* 4_to_8-bit_map-table data */
            {
                for (cont = 0; cont < 16; cont++)
                {
                    u8szFourToEightBitsMapTable[cont] = *pu8SegmentData;
                    pu8SegmentData++;
                    u16ProcessLen = (mt_u16)(u16ProcessLen + 1);
                }
                break;
            }

            case 0xf0 :/* End of object line code */
            {
                wNumLine = (mt_u16)(wNumLine + 2);
                if (wNumLine > pstReferingRegion->u16RegionHeight)
                {
                    wNumLine = (mt_u16)(wNumLine - 2);
                    MT_WARN_SUBT("\nobject height is over range!!\n");
                }

                pstReferingRegion->pu8ObjPixelWrite = pstReferingRegion->pu8RegionPixel +
                                          ((pstReferingRegion->u16RegionWidth *
                                           (pstReferingRegion->u16ObjVerPos + wNumLine)) +
                                           pstReferingRegion->u16ObjHorPos) ;
                break;
            }
            default :
            {
                break;
            }
        }
    }
    return MT_SUCCESS;
}

/* ref[Subtitling system.pdf] page25 section 7.2.4 */
static mt_s32 SUBT_Parse_DecodingPixels(SUBT_PARSE_INFO_S * pstParseInfo, SUBT_ALL_INFO_IN_REGION_S *pstRegn, mt_u16 u16ObjHorPos, mt_u16 u16ObjVerPos, mt_u8 *pu8SegmentDataSrc , mt_s32 u32SegLen )
{
    mt_u8   *pu8SegmentData = pu8SegmentDataSrc;
    ulong   nOffset = 0;
    mt_u8   *pTopFieldObjStart = NULL;
    mt_u8   *pBottomFieldObjStart = NULL;
    mt_u8   *pu8RegionPixels = NULL;

    mt_u16   u16TopFieldObjVerPos = 0;
    mt_u16   u16BottomFieldObjVerPos = 0;

    mt_u16   u16TopFieldLen = 0;
    mt_u16   u16BottomFieldLen = 0;
    mt_u16   i = 0;
    mt_s32   s32Ret = 0;

    SUBT_SHOW_REGION_S * pstLastRegion = NULL;

    SUBT_REFERING_REGION_S  TopReferingRegion ;
    SUBT_REFERING_REGION_S  BottomReferingRegion ;

    if(pu8SegmentDataSrc == NULL || pstRegn == NULL || pstParseInfo == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    /* top_field_data_block_length */
    u16TopFieldLen = (mt_u16)((pu8SegmentData[0] << 8) | pu8SegmentData[1]);
    pu8SegmentData += 2;

    /* bottom_field_data_block_length */
    u16BottomFieldLen = (mt_u16)((pu8SegmentData[0] << 8) | pu8SegmentData[1]);
    pu8SegmentData += 2;

    pstLastRegion = SUBT_Parse_GetCurPageLastRegion(pstParseInfo);

    if (NULL == pstLastRegion)
    {
        MT_ERR_SUBT("pstLastRegion is null...\n");
        return MT_FAILURE;
    }

    pstLastRegion->pu8ShowData = (mt_u8 *)(((ulong)pstLastRegion + sizeof(SUBT_SHOW_REGION_S) +3 )&(ulong)(~3));

    pu8RegionPixels = pstLastRegion->pu8ShowData;

    //u16TopFieldObjVerPos = u16ObjVerPos + ((((u16ObjVerPos + 1) % 2) == 0) ? 1 : 0);
    //for bug 111174
    u16TopFieldObjVerPos = u16ObjVerPos;

    u16BottomFieldObjVerPos = (mt_u16)(u16TopFieldObjVerPos + 1);

    nOffset = (ulong)(pstRegn->u16RegionWidth * u16TopFieldObjVerPos + u16ObjHorPos);//per pixel hold one byte
    #if 0 /* avoid start field of object offset */
    pTopFieldObjStart = (mt_u8 *)(((mt_u32)pu8RegionPixels + nOffset+3)&(~3));
    #else
    pTopFieldObjStart = (mt_u8 *)((ulong)pu8RegionPixels + nOffset);
	
	if ( (ulong)pTopFieldObjStart > ((ulong)pstParseInfo->pu8RegionBuffer+pstParseInfo->u32RegionBufferLen)) {		
		MT_ERR_SUBT("*****out of memory,pu8RegionPixels:0x%"PRIx64",pTopFieldObjStart:0x%"PRIx64",nOffset:0x%"PRIx64"\n",(ulong)pu8RegionPixels,(ulong)pTopFieldObjStart,nOffset);
		return MT_FAILURE;
	}
	
    #endif

    nOffset= (ulong)(pstRegn->u16RegionWidth * u16BottomFieldObjVerPos + u16ObjHorPos);
    #if 0 /* avoid object width is over range */
    pBottomFieldObjStart = (mt_u8 *)(((mt_u32)pu8RegionPixels + nOffset+3)&(~3));
    #else
    pBottomFieldObjStart = (mt_u8 *)((ulong)pu8RegionPixels + nOffset);
	
	if ( (ulong)pBottomFieldObjStart > ((ulong)pstParseInfo->pu8RegionBuffer+pstParseInfo->u32RegionBufferLen)) {		
		MT_ERR_SUBT("*****out of memory,pu8RegionPixels:0x%"PRIx64",pTopFieldObjStart:0x%"PRIx64",nOffset:0x%"PRIx64"\n",(ulong)pu8RegionPixels,(ulong)pBottomFieldObjStart,nOffset);
		return MT_FAILURE;
	}
	
    #endif

    TopReferingRegion.pu8RegionPixel = pu8RegionPixels;

    TopReferingRegion.u16ObjVerPos = u16TopFieldObjVerPos;
    TopReferingRegion.u16ObjHorPos = u16ObjHorPos;
    TopReferingRegion.u16RegionWidth = pstRegn->u16RegionWidth;
    TopReferingRegion.u16RegionHeight = pstRegn->u16RegionHeight;
    TopReferingRegion.u8RegionDepth = pstRegn->u8RegionDepth;
    TopReferingRegion.pu8ObjPixelWrite = pTopFieldObjStart;
	TopReferingRegion.u32RegionLen = (mt_u32)(pu8RegionPixels + pstParseInfo->u32RegionBufferLen- pTopFieldObjStart);

    BottomReferingRegion = TopReferingRegion;
    BottomReferingRegion.u16ObjVerPos = u16BottomFieldObjVerPos;
    BottomReferingRegion.pu8ObjPixelWrite = pBottomFieldObjStart;
	BottomReferingRegion.u32RegionLen =  (mt_u32)(pu8RegionPixels +pstParseInfo->u32RegionBufferLen - pBottomFieldObjStart);

    s32Ret = SUBT_Parse_PixelBlockDecode (pu8SegmentData, &TopReferingRegion, u16TopFieldLen);
    if(s32Ret == ERR_OUT_OF_WIDTH)
    {
        return s32Ret;
    }

    pu8SegmentData += u16TopFieldLen;

    if (u16BottomFieldLen != 0)
    {
        SUBT_Parse_PixelBlockDecode (pu8SegmentData, &BottomReferingRegion,
                            u16BottomFieldLen);

    }
    else
    {
        for(i=2;i<((pstRegn->u16RegionHeight+1)&(~1));)
        {
            pTopFieldObjStart = TopReferingRegion.pu8RegionPixel+(i-2)*pstRegn->u16RegionWidth;
            pBottomFieldObjStart = TopReferingRegion.pu8RegionPixel+(i-1)*pstRegn->u16RegionWidth;
            memcpy(pBottomFieldObjStart,pTopFieldObjStart,pstRegn->u16RegionWidth);
            i = (mt_u16)(i + 2);
        }
    }

    return MT_SUCCESS;
}

static mt_s32 SUBT_Parse_DecodingCharacter(SUBT_PARSE_INFO_S * pstParseInfo, mt_u8 *pu8SegmentDataSrc , mt_u32 nSegLen,mt_u8 chFrontCol,mt_u8 chBackCol)
{
    SUBT_SHOW_REGION_S * pstLastRegion = NULL;
    mt_u8 *pu8SegmentData = pu8SegmentDataSrc;
    mt_u8 u8Len = 0;

    if(pu8SegmentDataSrc == NULL || pstParseInfo == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    pstLastRegion = SUBT_Parse_GetCurPageLastRegion(pstParseInfo);

    if (NULL == pstLastRegion)
    {
        MT_ERR_SUBT("pstLastRegion is null...\n");

        return MT_FAILURE;
    }

    pstLastRegion->pu8ShowData = (mt_u8 *)(((ulong)pstLastRegion + sizeof(SUBT_SHOW_REGION_S)+3+pstLastRegion->u16CharacterNum)&(ulong)(~3));

    u8Len = *pu8SegmentData; /* number of codes 8bits */

    pu8SegmentData++;

    if((ulong)(pstLastRegion->pu8ShowData)+u8Len>(ulong)pstParseInfo->pu8RegionBuffer + pstParseInfo->u32RegionBufferLen)
    {
        MT_ERR_SUBT("\ncharacter object buffer is full!!!\n");

        return MT_FAILURE;
    }

    memcpy(pstLastRegion->pu8ShowData, pu8SegmentData, u8Len);
    pstLastRegion->u16CharacterNum = (mt_u16)(pstLastRegion->u16CharacterNum + u8Len);
    pstLastRegion->u8FrontClr = chFrontCol;
    pstLastRegion->u8BackClr = chBackCol;

    return MT_SUCCESS;
}

/*
 *Find the display definition segment. return MT_SUCCESS, otherwise failed to find it
 */
static mt_u8 SUBT_Parse_FindDisplayDefinitionSegment(SUBT_SEGMENT_S **ppstSeg)
{
    while (*ppstSeg)
    {
        if ( (SUBT_DISPLAY_DEFINITION_SEGMENT == (*ppstSeg)->u8SegmentType) && ((*ppstSeg)->u16DataLength) )
        {
            return (mt_u8)MT_SUCCESS;
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;
    }

    return (mt_u8)MT_FAILURE;
}

/*
 *Find the page segment. return MT_SUCCESS, otherwise failed to find it
 */
static mt_u8 SUBT_Parse_FindPageSegment(SUBT_SEGMENT_S **ppstSeg)
{
    while (*ppstSeg)
    {
        if ( (SUBT_PAGE_SEGMENT == (*ppstSeg)->u8SegmentType) && ((*ppstSeg)->u16DataLength) )
        {
            return (mt_u8)MT_SUCCESS;
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;
    }

    return (mt_u8)MT_FAILURE;
}

#ifdef SUBT_USED_3D_SUBTITLE
/*
 *Find the disparity signalling segment. return MT_SUCCESS, otherwise failed to find it
 */
static mt_u8 SUBT_Parse_FindDisparitySignallingSegment(SUBT_SEGMENT_S **ppstSeg)
{
    while (*ppstSeg)
    {
        if ( (SUBT_DISPARITY_SIGNALLING_SEGMENT == (*ppstSeg)->u8SegmentType) && ((*ppstSeg)->u16DataLength) )
        {
            return (mt_u8)MT_SUCCESS;
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;
    }

    return (mt_u8)MT_FAILURE;
}
#endif

#if 0
static mt_u8 SUBT_Parse_FindEndSegment(SUBT_SEGMENT_S **ppstSeg)
{
    while (*ppstSeg)
    {
        if ( (SUBT_END_SEGMENT == (*ppstSeg)->u8SegmentType) && (*ppstSeg)->u16DataLength )
        {
            return (mt_u8)MT_SUCCESS;
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;
    }

    return (mt_u8)MT_FAILURE;
}
#endif

static mt_u8 SUBT_Parse_FindClutSegment(mt_u8 u8ClutId,SUBT_SEGMENT_S **ppstSeg)
{
    mt_u8 u8ClutIdInStream = 0;

    while((*ppstSeg) && (*ppstSeg)->pu8SegmentData )
    {
        u8ClutIdInStream = *((*ppstSeg)->pu8SegmentData);
        if((SUBT_CLUT_SEGMENT == (*ppstSeg)->u8SegmentType)&&(u8ClutIdInStream == u8ClutId)&&((*ppstSeg)->u16DataLength))
        {
            return (mt_u8)MT_SUCCESS;
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;

    }

    return (mt_u8)MT_FAILURE;
}

static mt_u8 SUBT_Parse_FindRegionSegment(mt_u8 u8RegionId,SUBT_SEGMENT_S **ppstSeg)
{
    while((*ppstSeg) && (*ppstSeg)->pu8SegmentData )
    {
        if((SUBT_REGION_SEGMENT == (*ppstSeg)->u8SegmentType)&&(*((*ppstSeg)->pu8SegmentData) == u8RegionId)&&(*ppstSeg)->u16DataLength)
        {
            return (mt_u8)MT_SUCCESS;
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;
    }

    return (mt_u8)MT_FAILURE;
}

static mt_u8 SUBT_Parse_FindObjectSegment(mt_u16 u16ObjectId,SUBT_SEGMENT_S **ppstSeg)
{
    mt_u16 u16ObjectIdTem = 0;

    while(*ppstSeg && (*ppstSeg)->pu8SegmentData )
    {
        if((SUBT_OBJECT_SEGMENT == (*ppstSeg)->u8SegmentType)&&(*ppstSeg)->u16DataLength)
        {
            u16ObjectIdTem = *((*ppstSeg)->pu8SegmentData);
            u16ObjectIdTem = (mt_u16)((u16ObjectIdTem<<8) + (*((*ppstSeg)->pu8SegmentData+1)));
            if(u16ObjectIdTem == u16ObjectId)
            {
                return (mt_u8)MT_SUCCESS;
            }
        }

        (*ppstSeg) = (*ppstSeg)->pstNext;
    }

    return (mt_u8)MT_FAILURE;
}

static mt_s32 _RegionBuffer_ReAlloc(SUBT_PARSE_INFO_S *pstParseInfo, mt_u32 u32Len)
{
    mt_u32 u32RegionBufferLen = 0;

    if (pstParseInfo->u32RegionBufferLen == u32Len)
    {
        return MT_SUCCESS;
    }

    if (pstParseInfo->u32RegionBufferLen > u32Len)
    {
        u32RegionBufferLen = SUBT_REGION_SHOW_SD_LEN;
    }

    if (pstParseInfo->u32RegionBufferLen < u32Len)
    {
        u32RegionBufferLen = SUBT_REGION_SHOW_HD_LEN;
    }

    if (pstParseInfo->pu8RegionBuffer)
    {
        free(pstParseInfo->pu8RegionBuffer);
    }

    pstParseInfo->u32RegionBufferLen = u32RegionBufferLen;
    pstParseInfo->pu8RegionBuffer =  (mt_u8*)malloc(pstParseInfo->u32RegionBufferLen);
    if (pstParseInfo->pu8RegionBuffer == MT_NULL)
    {
        pstParseInfo->u32RegionBufferLen = 0;
        pstParseInfo->stPageParam.pstShowRegion = MT_NULL;
        MT_ERR_SUBT("not enough memory...\n");

        return MT_FAILURE;
    }
    memset(pstParseInfo->pu8RegionBuffer, 0, pstParseInfo->u32RegionBufferLen);
    pstParseInfo->stPageParam.pstShowRegion = (SUBT_SHOW_REGION_S*)pstParseInfo->pu8RegionBuffer;

    return MT_SUCCESS;
}

/* ref[en_300743v010401o.pdf] page22 section 7.2.1 */
static mt_s32 SUBT_Parse_ParseDisplayDefinition(SUBT_PARSE_INFO_S *pstParseInfo, SUBT_SEGMENT_S *pstSegment)
{
    mt_u8   *pu8SegmentData = pstSegment->pu8SegmentData;
    mt_u32  u32SegLen = pstSegment->u16DataLength;
    SUBT_DISPLAY_DEFINITION_S *pstDisplayDef = &pstParseInfo->stPageParam.stDisplayDef;
    mt_u8 u8Version = 0;
    mt_u8 u8DisplayFlag = 0;

    if ((u32SegLen != 5) && (u32SegLen != 13))
    {
        MT_ERR_SUBT("segment length is invalid...\n");

        return MT_FAILURE;
    }

    u8Version = (pu8SegmentData[0]>>4)&0x0f;


    pstDisplayDef->u8DdsVersionNumber = u8Version;

    u8DisplayFlag = (pu8SegmentData[0]>>3)&0x01;
    if (u8DisplayFlag > 0)
    {
        pstDisplayDef->u8DisplayWindowFlag = 1;
    }
    else
    {
        pstDisplayDef->u8DisplayWindowFlag = 0;
    }

    pstDisplayDef->u16DisplayWidth = (mt_u16)((pu8SegmentData[1]<<8) | pu8SegmentData[2]);
    pstDisplayDef->u16DisplayHeight = (mt_u16)((pu8SegmentData[3]<<8) | pu8SegmentData[4]);

    if (pstDisplayDef->u16DisplayWidth > 720)
    {
        /* HD */
        if (MT_SUCCESS != _RegionBuffer_ReAlloc(pstParseInfo, SUBT_REGION_SHOW_HD_LEN))
        {
            MT_ERR_SUBT("failed to _RegionBuffer_ReAlloc...\n");

            return MT_FAILURE;
        }
    }
    else
    {
        /* SD */
        if (MT_SUCCESS != _RegionBuffer_ReAlloc(pstParseInfo, SUBT_REGION_SHOW_SD_LEN))
        {
            MT_ERR_SUBT("failed to _RegionBuffer_ReAlloc...\n");

            return MT_FAILURE;
        }
    }

    if (pstDisplayDef->u8DisplayWindowFlag)
    {
        pstDisplayDef->u16DisplayWindowHorizontalPositionMinimum = (mt_u16)((pu8SegmentData[5]<<8) | pu8SegmentData[6]);
        pstDisplayDef->u16DisplayWindowHorizontalPositionMaximum = (mt_u16)((pu8SegmentData[7]<<8) | pu8SegmentData[8]);
        pstDisplayDef->u16DisplayWindowVerticalPositionMinimum   = (mt_u16)((pu8SegmentData[9]<<8) | pu8SegmentData[10]);
        pstDisplayDef->u16DisplayWindowVerticalPositionMaximum   = (mt_u16)((pu8SegmentData[11]<<8) | pu8SegmentData[12]);
    }
    else
    {
        pstDisplayDef->u16DisplayWindowHorizontalPositionMinimum = 0;
        pstDisplayDef->u16DisplayWindowHorizontalPositionMaximum = (mt_u16)(pstDisplayDef->u16DisplayWidth - 1);
        pstDisplayDef->u16DisplayWindowVerticalPositionMinimum   = 0;
        pstDisplayDef->u16DisplayWindowVerticalPositionMaximum   = (mt_u16)(pstDisplayDef->u16DisplayHeight- 1);
    }

    return MT_SUCCESS;
}

#ifdef SUBT_USED_3D_SUBTITLE

static mt_s32 SUBT_Parse_ParseShiftUpdateSequence(SUBT_DISPARITY_SHIFT_UPDATE_SEQUENCE_S *pstUpdateSequence,
        mt_u32 *pu32ProcessedLength, mt_u8 *pu8SegmentData)
{
    mt_u32 i = 0;
    mt_u32 u32ProcessedLength = *pu32ProcessedLength;
    /*
    disparity_shift_update_sequence()
    {
        disparity_shift_update_sequence_length           8
        interval_duration[23..0]                        24
        division_period_count                            8
        for (i= 0; i< division_period_count; i ++)
        {
            interval_count                           8
            disparity_shift_update_integer_part              8
        }
    }
    */

    pstUpdateSequence->u8DisparityShiftUpdateSequenceLength = pu8SegmentData[u32ProcessedLength];
    pstUpdateSequence->u32IntervalDuration = (pu8SegmentData[u32ProcessedLength + 1]<<16)
                        | (pu8SegmentData[u32ProcessedLength + 2]<<8)
                        | (pu8SegmentData[u32ProcessedLength + 3]);
    pstUpdateSequence->u8DivisionPeriodCount = pu8SegmentData[u32ProcessedLength + 4];

    u32ProcessedLength += 5;

    if (pstUpdateSequence->u8DivisionPeriodCount > SUBT_SHIFT_NUM)
    {
        MT_WARN_SUBT("Division Period Count > %d\n", SUBT_SHIFT_NUM);

        return MT_FAILURE;
    }
    else
    {
        for (i = 0; i < SUBT_SHIFT_NUM; i++)
        {
            pstUpdateSequence->stDisparityShiftData[i].u8IntervalCount = pu8SegmentData[u32ProcessedLength];
            pstUpdateSequence->stDisparityShiftData[i].u8DisparityShiftUpdateIntegerPart
                                                = pu8SegmentData[u32ProcessedLength + 1];
            u32ProcessedLength += 2;
        }
    }

    *pu32ProcessedLength = u32ProcessedLength;

    return MT_SUCCESS;
}

/* ref[en_300743v010401o.pdf] page34 section 7.2.7 */
static mt_s32 SUBT_Parse_ParseDisparitySignallingSegment(SUBT_PARSE_INFO_S *pstParseInfo, SUBT_SEGMENT_S *pstSegment)
{
    mt_u8   *pu8SegmentData = pstSegment->pu8SegmentData;
    mt_u32  u32SegLen = pstSegment->u16DataLength;
    mt_u16  u16VersionNumber = 0;
    mt_u32  u32ProcessedLength = 0;
    mt_u8   u8DisparityShiftUpdateSequencePageFlag = 0;
    mt_u32  i = 0,j = 0;
    SUBT_DISPARITY_SIGNALLING_SEGMENT_S *pstSignallingSegment = &pstParseInfo->stPageParam.stSignallingSegment;


    /*
    * dss_version_number                                4
    * disparity_shift_update_sequence_page_flag             1
    * reserved                                          3
    * page_default_disparity_shift                      8
    * if (disparity_shift_update_sequence_page_flag ==1)
    * {
    *   disparity_shift_update_sequence()
    * }
    */
    u16VersionNumber = (pu8SegmentData[u32ProcessedLength]>>4)&0x0f;
    if (u16VersionNumber == pstSignallingSegment->u8dss_version_number)
    {
        return MT_SUCCESS;
    }
    else
    {
        pstSignallingSegment->u8dss_version_number = u16VersionNumber;
    }

    u8DisparityShiftUpdateSequencePageFlag = (pu8SegmentData[u32ProcessedLength]>>3)&0x01;

    u32ProcessedLength += 1;
    pstSignallingSegment->u8page_default_disparity_shift = pu8SegmentData[u32ProcessedLength];

    u32ProcessedLength += 1;
    if (u8DisparityShiftUpdateSequencePageFlag == 1)
    {
        if (MT_SUCCESS != SUBT_Parse_ParseShiftUpdateSequence(&pstSignallingSegment->stDisparityShiftUpdateSequence,&u32ProcessedLength,pu8SegmentData))
        {
            MT_WARN_SUBT("failed to SUBT_Parse_ParseShiftUpdateSequence\n");

            return MT_FAILURE;
        }
    }
    /*
    * while (processed_length<segment_length)
    * {
    *   region_id                                           8
    *   disparity_shift_update_sequence_region_flag                 1
    *   reserved                                                5
    *   number_of_subregions_minus_1                            2
    *   for (n=0; n<= number_of_subregions_minus_1; n++)
    *   {
    *       if (number_of_subregions_minus_1 > 0)
    *       {
    *           subregion_horizontal_position                   16
    *           subregion_width                                 16
    *       }
    *       subregion_disparity_shift_integer_part                   8
    *       subregion_disparity_shift_fractional_part                4
    *       reserved                                             4
    *       if (disparity_shift_update_sequence_region_flag ==1)
    *       {
    *           disparity_shift_update_sequence()
    *       }
    *   }
    * }
    */
    while (u32ProcessedLength < u32SegLen)
    {
        pstSignallingSegment->stDisparity[i].u8RegionID = pu8SegmentData[u32ProcessedLength];
        u8DisparityShiftUpdateSequencePageFlag = pu8SegmentData[u32ProcessedLength + 1]&0x01;
        pstSignallingSegment->stDisparity[i].u8NumberOfSubregionsMinus1 = (pu8SegmentData[u32ProcessedLength + 1]>>6) & 0x03;

        u32ProcessedLength += 2;
        for (j = 0; j < pstSignallingSegment->stDisparity[i].u8NumberOfSubregionsMinus1; j++)
        {
            if (pstSignallingSegment->stDisparity[i].u8NumberOfSubregionsMinus1 > 0)
            {
                pstSignallingSegment->stDisparity[i].stDisparitySubregionData[j].u16SubregionHorizontalPosition
                                    = (pu8SegmentData[u32ProcessedLength]<<8) | (pu8SegmentData[u32ProcessedLength + 1]);
                pstSignallingSegment->stDisparity[i].stDisparitySubregionData[j].u16SubregionWidth
                                    = (pu8SegmentData[u32ProcessedLength + 2]<<8) | (pu8SegmentData[u32ProcessedLength + 3]);
                u32ProcessedLength += 4;
            }

            pstSignallingSegment->stDisparity[i].stDisparitySubregionData[j].u8SubregionDisparityShiftIntegerPart= pu8SegmentData[u32ProcessedLength];
            pstSignallingSegment->stDisparity[i].stDisparitySubregionData[j].u8SubregionDisparityShiftFractionalPart = (pu8SegmentData[u32ProcessedLength + 1]>>4)&0x0F;

            u32ProcessedLength += 2;

            if (u8DisparityShiftUpdateSequencePageFlag == 1)
            {
                if (MT_SUCCESS != SUBT_Parse_ParseShiftUpdateSequence(&pstSignallingSegment->stDisparity[i].stDisparitySubregionData[j].stDisparityShiftUpdateSequence,&u32ProcessedLength,pu8SegmentData))
                {
                    MT_WARN_SUBT("failed to SUBT_Parse_ParseShiftUpdateSequence\n");

                    return MT_FAILURE;
                }
            }
        }

    }

    return MT_SUCCESS;
}
#endif

static SUBT_CLUT_S* SUBT_Parse_FindClutId(SUBT_PARSE_INFO_S* pstParseInfo, mt_u8 u8ClutId)
{
    mt_u8 i = 0;

    for (i = 0; i < SUBT_CLUT_IN_PAGE_MAX; i++)
    {
        if (u8ClutId == pstParseInfo->stPageParam.astClut[i].u8ClutId)
        {
            return &pstParseInfo->stPageParam.astClut[i];
        }
    }

    for (i = 0; i < SUBT_CLUT_IN_PAGE_MAX; i++)
    {
        if (/*0 == pstParseInfo->stPageParam.astClut[i].u8ClutId &&*/
            0xff == pstParseInfo->stPageParam.astClut[i].u8ClutVersion)
        {
            return &pstParseInfo->stPageParam.astClut[i];
        }
    }

    return MT_NULL;
}

/* ref[Subtitling system.pdf] page24 section 7.2.3 */
static mt_void SUBT_Parse_ParseClutSegment(SUBT_PARSE_INFO_S* pstParseInfo, SUBT_SEGMENT_S *pstSegment, SUBT_ALL_INFO_IN_REGION_S *pstRegionInfo)
{
    mt_u8   u8ClutVersion = 0;
    mt_u8   u8ClutEntryId = 0;
    mt_u8   U8EntryClutFlag = 0;
    mt_u8   Y = 0,Cr = 0,Cb = 0, T = 0;
    mt_u32  u32RGB = 0;
    mt_u8   u8FullRange = 0;
    mt_u8   *pu8SegmentData = pstSegment->pu8SegmentData;
    mt_s32  s32SegLen = (mt_s32)pstSegment->u16DataLength;
    SUBT_CLUT_S *pstClut = MT_NULL;

    pstClut = SUBT_Parse_FindClutId(pstParseInfo, pu8SegmentData[0]);
    if (MT_NULL == pstClut)
    {
        MT_WARN_SUBT("failed to SUBT_Parse_FindClutId...\n");
        return;
    }

    /* CLUT-id */
    pstClut->u8ClutId = pu8SegmentData[0];

    /* skip CLUT-id(8-bit) */
    pu8SegmentData += 1;
    s32SegLen -= 1 ;

    /* CLUT_version_number */
    u8ClutVersion = ((pu8SegmentData[0] & 0xf0) >> 4); /* 4-bit */
    if (u8ClutVersion == pstClut->u8ClutVersion)
    {
        /* Nothing to do */
        return;
    }
    else
    {
        /* We don't have this version of the CLUT: Parse it */
        pstClut->u8ClutVersion = u8ClutVersion;
    }

    /* skip CLUT_version_number(4-bit) reserved(4-bit) */
    pu8SegmentData += 1;
    s32SegLen -= 1;

    //memcpy(pstClut->_2BitCLUT, s_2BitDefaultCLUT, sizeof(s_2BitDefaultCLUT));
    //memcpy(pstClut->_4BitCLUT, s_4BitDefaultCLUT, sizeof(s_4BitDefaultCLUT));
    //memcpy(pstClut->_8BitCLUT, s_8BitDefaultCLUT, sizeof(s_8BitDefaultCLUT));

    while (s32SegLen >= 4)
    {
        /* CLUT_entry_id */
        u8ClutEntryId = pu8SegmentData[0];
        pu8SegmentData ++;

        U8EntryClutFlag = pu8SegmentData[0];

        u8FullRange = pu8SegmentData[0] & 0x01;
        pu8SegmentData ++;

        s32SegLen -= 2;

        if ( u8FullRange )
        {
            if (s32SegLen < 4)
            {
                break;
            }
            Y  = pu8SegmentData[0];
            Cr = pu8SegmentData[1];
            Cb = pu8SegmentData[2];
            T  = pu8SegmentData[3];//T=0xff means full transparency

            s32SegLen -= 4;
            pu8SegmentData += 4;
        }
        else
        {
            /* This block data should be 2 byte size */
            Y  = (mt_u8)((pu8SegmentData[0] & 0xfc) >> 2); /* 6-bit */
            Cr = (mt_u8)(((pu8SegmentData[0] & 0x03)  << 2) | (( pu8SegmentData[1] & 0xc0 ) >> 6)); /* 4-bit */
            Cb = (mt_u8)(( pu8SegmentData[1] & 0x3c ) >> 2); /* 4-bit */
            T  = (mt_u8)( pu8SegmentData[1] & 0x03 ); /* 2-bit */

            s32SegLen -= 2;
            pu8SegmentData += 2;
        }

        /* Y=0 means full transparency */
        if ( Y == 0 )
        {
            Cr = 0;
            Cb = 0;
            Y = 16; /* Y=0 is disallowed in ITU-R Recommendation BT.601, so mapped to a legal value (e.g. Y=16d) */
            T = 0xff;
        }

        u32RGB = YUV2RGB(Y, Cr, Cb);

        /* bit7--2bit entry flag, bit6--4bit entry flag, bit5--8bit entry flag bit4~bit1--reserved */
        if (( U8EntryClutFlag & 0x80 )&&(u8ClutEntryId < CLUT_BIT_4 ))
        {
            pstClut->_2BitCLUT[ u8ClutEntryId ].u8Red   = (u32RGB >> 16) & 0xff;
            pstClut->_2BitCLUT[ u8ClutEntryId ].u8Green = (u32RGB >>  8) & 0xff;
            pstClut->_2BitCLUT[ u8ClutEntryId ].u8Blue  = u32RGB & 0xff;
            pstClut->_2BitCLUT[ u8ClutEntryId ].u8Alpha = (mt_u8)(0xff - T); // app requests 0 is transparent;
        }
        if (( U8EntryClutFlag & 0x40 )&&( u8ClutEntryId < CLUT_BIT_16 ))
        {
            pstClut->_4BitCLUT[ u8ClutEntryId ].u8Red   = (u32RGB >> 16) & 0xff;
            pstClut->_4BitCLUT[ u8ClutEntryId ].u8Green = (u32RGB >>  8) & 0xff;
            pstClut->_4BitCLUT[ u8ClutEntryId ].u8Blue  = u32RGB & 0xff;
            pstClut->_4BitCLUT[ u8ClutEntryId ].u8Alpha = (mt_u8)(0xff - T); // app requests 0 is transparent;
        }
        if (U8EntryClutFlag & 0x20)
        {
            pstClut->_8BitCLUT[ u8ClutEntryId ].u8Red   = (u32RGB >> 16) & 0xff;
            pstClut->_8BitCLUT[ u8ClutEntryId ].u8Green = (u32RGB >>  8) & 0xff;
            pstClut->_8BitCLUT[ u8ClutEntryId ].u8Blue  = u32RGB & 0xff;
            pstClut->_8BitCLUT[ u8ClutEntryId ].u8Alpha = (mt_u8)(0xff - T); // app requests 0 is transparent;
        }
    }
}

/* ref[Subtitling system.pdf] page20 section 7.2.1 */
static mt_s32 SUBT_Parse_ParsePageSegment(SUBT_PARSE_INFO_S * pstParseInfo, SUBT_SEGMENT_S *pstSegment,SUBT_REGION_INFO_IN_PAGE_S *pstRegion,mt_u8 *pu8RegionNum)
{
    mt_s32 s32RegnNum = 0;
    mt_s32 i = 0;
    mt_u8  *pu8SegmentDataSrc = pstSegment->pu8SegmentData;
    mt_u32 u32Len = pstSegment->u16DataLength;
    SUBT_PAGE_PARAM_S *pstCurPage = &pstParseInfo->stPageParam;

    if(pstRegion == NULL || pu8RegionNum == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    /* page_time_out */
    pstCurPage->u8ShowTime = pu8SegmentDataSrc[0];
    if (0 == pstCurPage->u8ShowTime)
    {
        pstCurPage->u8ShowTime = 5;
    }

    pu8SegmentDataSrc++;/* skip page_time_out */

    /* page_version_number */
    pstRegion[0].u8InPageVer = (pu8SegmentDataSrc[0] & 0xF0) >> 4;

    /* page state */
    pstCurPage->enPageState = (SUBT_PAGE_STATE_E)( (pu8SegmentDataSrc[0] & 0x0c) >> 2 );
    pu8SegmentDataSrc++;

    /* u32Len is the whole segment data.
     * 2 is the page_time_out byte and page state byte
     * 6 is the region size: one byte region id, one reserved byte, two bytes address, respectively for region horizontal and vertical
     */
    if (u32Len <= 2)
    {
        s32RegnNum = 0;
    }
    else
    {
        s32RegnNum = (mt_s32)((u32Len - 2) / 6);
    }
    MT_INFO_SUBT("Data length is %d, which should have %d region in that page, page state is %d, page version %d.\n", u32Len, s32RegnNum, pstCurPage->enPageState, pstRegion[0].u8InPageVer);
    if( s32RegnNum >  SUBT_REGN_IN_PAGE_NUM )
    {
        s32RegnNum = SUBT_REGN_IN_PAGE_NUM ;
    }

    for( i = 0; i < s32RegnNum; i++)
    {
        pstRegion[i].u8RegionId =  *pu8SegmentDataSrc;
        pu8SegmentDataSrc += 2;

        pstRegion[i].u16RegionLeft = (mt_u16)(pu8SegmentDataSrc[0] << 8 | pu8SegmentDataSrc[1]);
        pu8SegmentDataSrc += 2;

        pstRegion[i].u16RegionTop = (mt_u16)(pu8SegmentDataSrc[0] << 8 | pu8SegmentDataSrc[1]);
        pu8SegmentDataSrc += 2;
    }

    *pu8RegionNum = (mt_u8)s32RegnNum;

    return MT_SUCCESS;
}

/* Ref[Subtitling system.pdf] page21 section 7.2.2 */
static mt_s32 SUBT_Parse_ParseRegionSegment(SUBT_SEGMENT_S *pstSegment, SUBT_ALL_INFO_IN_REGION_S *pstRegion, SUBT_REGION_INFO_IN_PAGE_S *pRegion,SUBT_PAGE_STATE_E enPageState)
{
    mt_u8 *pu8SegmentDataSrc = pstSegment->pu8SegmentData;
    mt_s32 s32DataLen = (mt_s32)pstSegment->u16DataLength;
    SUBT_OBJECT_INFO_IN_REGION_S    *pstObjInfoTem = NULL;
    mt_u32 u32SkipLen = 0;
    mt_u32 u32ObjectLeftOffset = 0;

    if(pstRegion == NULL || pRegion == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    /* region_id */
    pstRegion->u8RegionID = *pu8SegmentDataSrc;
    pu8SegmentDataSrc++;/* skip region id */
    u32SkipLen += 1;

    /* region_fill_flag */
    pstRegion->u8RegionFillFlag = (mt_u8)( ((*pu8SegmentDataSrc) & 0x08) >> 3 );
    pu8SegmentDataSrc++;
    u32SkipLen += 1;

    /* region_width, range 1 to 720
    if define  DDS ,the range 1 to 4095*/
    pstRegion->u16RegionWidth =  (mt_u16)((pu8SegmentDataSrc[0] << 8) | pu8SegmentDataSrc[1]);
    if (pstRegion->u16RegionWidth > MAX_REGION_WIDTH)
    {
        pstRegion->u16RegionWidth = MAX_REGION_WIDTH;
    }

    pu8SegmentDataSrc += 2;
    u32SkipLen += 2;

    /* region_height, range 1 to 576
      if define  DDS ,the range 1 to 4095*/
    pstRegion->u16RegionHeight =  (mt_u16)((pu8SegmentDataSrc[0] << 8) | pu8SegmentDataSrc[1]);
    if (pstRegion->u16RegionHeight > MAX_REGION_HEIGTH)
    {
        pstRegion->u16RegionHeight = MAX_REGION_HEIGTH;
    }

    pu8SegmentDataSrc += 2;
    u32SkipLen += 2;

    /* region level of compatibility */
    pstRegion->u8RegionClutSel = (mt_u8)(((*pu8SegmentDataSrc) & 0xe0) >> 5);

    /* region_depth: Identifies the intended pixel depth for this region
        * 0x00 reserved
        * 0x01 2 bit
        * 0x02 4 bit
        * 0x03 8 bit
        * 0x04 - 0x07 reserved
        */
    pstRegion->u8RegionDepth = (mt_u8)(((*pu8SegmentDataSrc) & 0x1c) >> 2);
    switch (pstRegion->u8RegionDepth)
    {
        case 1:
            pstRegion->u8RegionDepth = 2;
            break;
        case 2:
            pstRegion->u8RegionDepth = 4;
            break;
        case 3:
            pstRegion->u8RegionDepth = 8;
            break;
        default:
            pstRegion->u8RegionDepth = 0;
            break;
    }

    pu8SegmentDataSrc++;
    u32SkipLen += 1;

    /* CLUT_id */
    pstRegion->u8ClutId= *pu8SegmentDataSrc;
    pu8SegmentDataSrc++;
    u32SkipLen += 1;

    switch(pstRegion->u8RegionDepth)
    {
        case 2:
            pstRegion->u8BackClr = (pu8SegmentDataSrc[1] & 0x0c) >> 2; /* region_2-bit_pixel-code */
            break;
        case 4:
            pstRegion->u8BackClr = (pu8SegmentDataSrc[1] & 0xf0) >> 4; /* region_4-bit_pixel-code */
            break;
        case 8:
            pstRegion->u8BackClr = pu8SegmentDataSrc[0]; /* region_8-bit_pixel_code */
            break;
        default:
            break;
    }
    pu8SegmentDataSrc += 2;
    u32SkipLen += 2;

    s32DataLen = s32DataLen - (mt_s32)u32SkipLen;

    u32SkipLen = 0;

    pstRegion->u8ObjectNum = 0;


    /*
         *The min length should be 6 bytes. object_id(2bytes) + object_type/object_horizontal_position(2bytes) + object_vertical_position(2bytes)
         * options field:foreground_pixel_code(1byte) + background_pixel_code(1byte)
         */
    while( s32DataLen >= 6 )
    {
        pstObjInfoTem = &pstRegion->stObjectInfo[pstRegion->u8ObjectNum];

        /* object_id */
        pstObjInfoTem->u16ObjectId = (mt_u16)((pu8SegmentDataSrc[0] << 8) | pu8SegmentDataSrc[1]);
        pu8SegmentDataSrc += 2;
        u32SkipLen += 2;

        /* object_type */
        pstObjInfoTem->enObjectType = (SUBT_OBJ_TYPE_E)(((*pu8SegmentDataSrc) & 0xc0) >> 6 );

        /* object_horizontal_position */
        pstObjInfoTem->u16ObjectLeft = (mt_u16)(( (pu8SegmentDataSrc[0] & 0x0f) << 8) | pu8SegmentDataSrc[1]);
        pu8SegmentDataSrc += 2;
        u32SkipLen += 2;

        /* object_vertical_position */
        pstObjInfoTem->u16ObjectTop = (mt_u16)(( (pu8SegmentDataSrc[0] & 0x0f) << 8) | pu8SegmentDataSrc[1]);
        pu8SegmentDataSrc += 2;
        u32SkipLen += 2;
        if(pstSegment->data_status == 1)
        {
        	pstObjInfoTem->u16ObjectLeft = pstObjInfoTem->u16ObjectLeft >> 1;
        }

        if (enPageState == SUBTITLE_PAGE_NORMAL_CASE)
        {
            /* in normal case mode, just update object that are changed */
            /* transfor object left to region left, so that the region contains only the subtitle object that are changed */
            if (0 == u32ObjectLeftOffset)
            {
                u32ObjectLeftOffset = pstObjInfoTem->u16ObjectLeft;
                pstRegion->u16RegionWidth = (mt_u16)(pstRegion->u16RegionWidth - pstObjInfoTem->u16ObjectLeft);
                pRegion->u16RegionLeft = (mt_u16)(pRegion->u16RegionLeft + pstObjInfoTem->u16ObjectLeft);

                pstObjInfoTem->u16ObjectLeft = 0;
            }
            else
            {
                pstObjInfoTem->u16ObjectLeft = (mt_u16)(pstObjInfoTem->u16ObjectLeft - u32ObjectLeftOffset);
            }
        }

        s32DataLen = s32DataLen - (mt_s32)u32SkipLen;
        u32SkipLen = 0;

        if( (SUBT_OBJ_TYPE_CHARACTER == pstObjInfoTem->enObjectType)
            || (SUBT_OBJ_TYPE_STRING == pstObjInfoTem->enObjectType) )
        {
            /* foreground_pixel_code */
            pstObjInfoTem->u8FrontClr = pu8SegmentDataSrc[0];
            /* background_pixel_code */
            pstObjInfoTem->u8BackClr = pu8SegmentDataSrc[1];

            pu8SegmentDataSrc += 2;
            s32DataLen -= 2;
        }


        pstRegion->u8ObjectNum++;

        if( pstRegion->u8ObjectNum >= SUBT_OBJ_NUM_IN_REGN_MAX )
        {
            MT_WARN_SUBT("SUBT_object_is_over the SUBT_OBJ_NUM_IN_REGN_MAX!!\n");
            break;
        }
    }

    MT_INFO_SUBT("have %d objects in region\n", pstRegion->u8ObjectNum);

    return MT_SUCCESS;
}

/* ref[Subtitling system.pdf] page25 section 7.2.4 */
static mt_s32 SUBT_Parse_ParseObjectSegment(SUBT_PARSE_INFO_S * pstParseInfo, SUBT_ALL_INFO_IN_REGION_S *pstRegion, SUBT_SEGMENT_S *pstSegment, SUBT_OBJECT_INFO_IN_REGION_S *pstObjectInfoInRegion)
{
    mt_s32 s32Ret = MT_FAILURE;
    SUBT_OBJ_TYPE_E eObjCodeMethod = SUBT_OBJ_TYPE_BITMAP;
    mt_u32 u32SegLen = pstSegment->u16DataLength;
    mt_u8 *pu8Src = pstSegment->pu8SegmentData;
    SUBT_PAGE_PARAM_S *pstCurPage = &pstParseInfo->stPageParam;

    if(pstRegion == NULL || pstObjectInfoInRegion == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    pu8Src += 2; /* skip object-id 16-bit */
    u32SegLen -= 2;

    /*object_coding_method */
    eObjCodeMethod = (SUBT_OBJ_TYPE_E)((pu8Src[0] & 0x0c) >> 2 );

    pu8Src++;
    u32SegLen -= 1;

    pstCurPage->pstShowRegion->enObjType = eObjCodeMethod;
    //printf("eObjCodeMethod = %d\n",eObjCodeMethod);
    /* object_coding_method:
    * 0x00 coding of pixels
    * 0x01 coded as a string of characters
    * 0x02 reserved
    * 0x03 reserved
    */
    switch (eObjCodeMethod)
    {
        case SUBT_OBJ_TYPE_BITMAP://coding of pixels
            s32Ret = SUBT_Parse_DecodingPixels(pstParseInfo, pstRegion, pstObjectInfoInRegion->u16ObjectLeft, pstObjectInfoInRegion->u16ObjectTop, pu8Src, (mt_s32)u32SegLen) ;
            break;
        case SUBT_OBJ_TYPE_CHARACTER:
            s32Ret = SUBT_Parse_DecodingCharacter(pstParseInfo, pu8Src, u32SegLen, pstObjectInfoInRegion->u8FrontClr, pstObjectInfoInRegion->u8BackClr);
            break;
        default:
            break;
    }

    return s32Ret;
}

static mt_s32 SUBT_Parse_ParseSegment(SUBT_PARSE_INFO_S * pstParseInfo)
{
    mt_s32 s32Ret = MT_FAILURE;
    SUBT_SEGMENT_S *pstSegment = NULL;
    SUBT_REGION_INFO_IN_PAGE_S astSubtRegionInfoInPage[SUBT_REGN_IN_PAGE_NUM] = {{0}};
    SUBT_ALL_INFO_IN_REGION_S stSubtAllRegionInfo = {0};
    mt_u8 u8RegionIndex = 0;
    mt_u8 u8ObjectIndex = 0;
    mt_u8 u8RegionNum = 0;
    //mt_u8 i = 0;
    SUBT_PAGE_PARAM_S *pstCurPage;

    SUBT_SEGMENT_S *pstSegmentEnter = NULL; /* signals the page segment enter point */
    if (NULL == pstParseInfo)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    pstCurPage = &pstParseInfo->stPageParam;
    pstSegmentEnter = pstCurPage->astSegment;
    if (pstSegmentEnter->u16DataLength == 0)
    {
        MT_INFO_SUBT("no segment...\n");

        return MT_SUCCESS;
    }

    pstSegment = pstSegmentEnter;
    memset(&pstParseInfo->stPageParam.stDisplayDef, 0, sizeof(SUBT_DISPLAY_DEFINITION_S));
    if (MT_SUCCESS == SUBT_Parse_FindDisplayDefinitionSegment(&pstSegment))
    {
        /*to parse Display definition segment*/
        s32Ret = SUBT_Parse_ParseDisplayDefinition(pstParseInfo, pstSegment);
        if (s32Ret != MT_SUCCESS)
        {
            MT_ERR_SUBT("failed to SUBT_Parse_ParseDisplayDefinition...\n");

            return MT_FAILURE;
        }
    }
    else
    {
        if (MT_SUCCESS != _RegionBuffer_ReAlloc(pstParseInfo, SUBT_REGION_SHOW_HD_LEN))
        {
            MT_ERR_SUBT("failed to _RegionBuffer_ReAlloc...\n");

            return MT_FAILURE;
        }
    }

    pstSegment = pstSegmentEnter;
    if (SUBT_Parse_FindPageSegment(&pstSegment) == MT_SUCCESS)
    {
        pstSegmentEnter = pstSegment;
    }
    else
    {
        MT_INFO_SUBT("not page segment in pes data...\n");

        return MT_SUCCESS;
    }

    /* to parse page segment*/
    if(pstSegment->u8SegmentType == SUBT_PAGE_SEGMENT)
    {

        memset(astSubtRegionInfoInPage, 0, sizeof(astSubtRegionInfoInPage));
        s32Ret = SUBT_Parse_ParsePageSegment(pstParseInfo, pstSegment, astSubtRegionInfoInPage, &u8RegionNum);

        if (s32Ret == MT_SUCCESS)
        {
            if (pstCurPage->enPageState== SUBTITLE_PAGE_NORMAL_CASE)
            {
                /* The display set contains only the subtitle elements that are changed from the previous page instance. */
                if (astSubtRegionInfoInPage[0].u8InPageVer == pstCurPage->u8PageVersion)
                {
                    /* the same page version, it mead that region not changed, don't need to do anything */
                    return MT_SUCCESS;
                }
                else if (u8RegionNum)
                {
                    //TODO : update the region which are changed to show
                    MT_INFO_SUBT("TODO : update the region which are changed to show\n");
                }
            }
            else
            {
                /* The display set contains all subtitle elements needed to display the next page instance. */
            }

            pstCurPage->pstShowRegion->pstNextRegion = NULL;
            pstCurPage->u8PageVersion = astSubtRegionInfoInPage[0].u8InPageVer;
            MT_INFO_SUBT("have %d region in page\n",u8RegionNum);
        }
        else
        {
            MT_ERR_SUBT("failed to SUBT_Parse_ParsePageSegment...\n");

            return MT_FAILURE;
        }
    }
#if 0
    /* reset region clut */
    memcpy(pstParseInfo->stPageParam.stDefaultClut._2BitCLUT, s_2BitDefaultCLUT, sizeof(s_2BitDefaultCLUT));
    memcpy(pstParseInfo->stPageParam.stDefaultClut._4BitCLUT, s_4BitDefaultCLUT, sizeof(s_4BitDefaultCLUT));
    memcpy(pstParseInfo->stPageParam.stDefaultClut._8BitCLUT, s_8BitDefaultCLUT, sizeof(s_8BitDefaultCLUT));
    //memset(pstParseInfo->stPageParam.astClut, 0, sizeof(pstParseInfo->stPageParam.astClut));
    for (i = 0; i < SUBT_CLUT_IN_PAGE_MAX; i++)
    {
        pstParseInfo->stPageParam.astClut[i].u8ClutVersion = 0xff;
    }
 #endif

    /* to parse region segment */
    for (u8RegionIndex = 0; u8RegionIndex < u8RegionNum; u8RegionIndex++)
    {
        SUBT_REGION_INFO_IN_PAGE_S *pstRegionInfoInPage = &astSubtRegionInfoInPage[u8RegionIndex];

        pstSegment = pstSegmentEnter;

        if (MT_SUCCESS == SUBT_Parse_FindRegionSegment(pstRegionInfoInPage->u8RegionId,&pstSegment))
        {
            pstSegment->data_status = pstParseInfo->error_status;
            s32Ret = SUBT_Parse_ParseRegionSegment(pstSegment,&stSubtAllRegionInfo,
                                pstRegionInfoInPage, pstCurPage->enPageState);

            MT_INFO_SUBT("region info: [%d] [%d %d %d %d]\n", stSubtAllRegionInfo.u8RegionID, pstRegionInfoInPage->u16RegionLeft,
                            pstRegionInfoInPage->u16RegionTop,stSubtAllRegionInfo.u16RegionWidth,stSubtAllRegionInfo.u16RegionHeight);

            if (0 == stSubtAllRegionInfo.u8ObjectNum)
            {
                MT_INFO_SUBT("no object in region[%d]\n", stSubtAllRegionInfo.u8RegionID);
                continue;
            }

            s32Ret |= SUBT_Parse_CreateShowRegion(pstParseInfo, pstRegionInfoInPage->u16RegionLeft,
                                    pstRegionInfoInPage->u16RegionTop, &stSubtAllRegionInfo, 0);
            if (s32Ret != MT_SUCCESS)
            {
                MT_ERR_SUBT("failed to parse region segment or create show region\n");

                return MT_FAILURE;
            }

            /* to parse Clut segment */
            if(MT_SUCCESS == SUBT_Parse_FindClutSegment(stSubtAllRegionInfo.u8ClutId,&pstSegment))
            {
                SUBT_Parse_ParseClutSegment(pstParseInfo, pstSegment, &stSubtAllRegionInfo);
            }

            /* to parse object segment */

            pstSegment = pstSegmentEnter;
            for (u8ObjectIndex = 0; u8ObjectIndex < stSubtAllRegionInfo.u8ObjectNum; u8ObjectIndex++)
            {
                SUBT_OBJECT_INFO_IN_REGION_S *pstObjectInfoInRegion = &stSubtAllRegionInfo.stObjectInfo[u8ObjectIndex];

                if(MT_SUCCESS == SUBT_Parse_FindObjectSegment(pstObjectInfoInRegion->u16ObjectId, &pstSegment))
                {
                    s32Ret = SUBT_Parse_ParseObjectSegment(pstParseInfo, &stSubtAllRegionInfo, pstSegment, pstObjectInfoInRegion);
                    if (s32Ret != MT_SUCCESS)
                    {
                        MT_ERR_SUBT("failed to SUBT_Parse_ParseObjectSegment...\n");


                        return s32Ret;
                    }
                }
            }

            s32Ret = SUBT_Parse_SetNextRegionAddr(pstParseInfo);
            if (s32Ret != MT_SUCCESS)
            {
                MT_ERR_SUBT("failed to SUBT_Parse_SetNextRegionAddr...\n");

                return MT_FAILURE;
            }

        }
        else
        {
        	if( u8RegionIndex + 1 < u8RegionNum)
				continue;
            MT_INFO_SUBT("\nregion[%d] data haved be lost...\n", u8RegionIndex);
            return MT_SUCCESS;

        }
    }

#ifdef SUBT_USED_3D_SUBTITLE
    pstSegment = pstSegmentEnter;
    if (MT_SUCCESS == SUBT_Parse_FindDisparitySignallingSegment(&pstSegment))
    {
        s32Ret = SUBT_Parse_ParseDisparitySignallingSegment(pstParseInfo,pstSegment);
        /* TODO: do something for 3D subtitle */
    }
#endif
    if (pstParseInfo->pfnCallback)
    {
        s32Ret = SUBT_DataParse_Display(pstParseInfo);
        if (s32Ret != MT_SUCCESS)
        {
            MT_WARN_SUBT("failed to SUBT_DataParse_Display...\n");
        }
    }

    return MT_SUCCESS;
}

static mt_s32 SUBT_DataParse_AdjustClut(SUBT_PARSE_INFO_S *pstParseInfo)
{
    SUBT_SHOW_REGION_S *pstRegion = MT_NULL;
    SUBT_PAGE_PARAM_S *pstCurPage = &pstParseInfo->stPageParam;
    SUBT_CLUT_S *pstClut = MT_NULL, *pstDefaultClut = MT_NULL;
    mt_s32 s32ClutCmp = 0;
    mt_s32 count = 0;
    mt_u32 i = 0;

    pstRegion = pstCurPage->pstShowRegion;

    while (pstRegion->pstNextRegion)
    {
        /* used first region clut to default clut */
        pstClut = SUBT_Parse_FindClutId(pstParseInfo, pstRegion->u8ClutId);
        if (pstClut)
        {
            switch (pstRegion->u8RegionDepth)
            {
                case 2:
                    pstRegion->pvRegionClut = pstClut->_2BitCLUT;
                    break;
                case 4:
                    pstRegion->pvRegionClut = pstClut->_4BitCLUT;
                    break;
                case 8:
                default:
                    pstRegion->pvRegionClut = pstClut->_8BitCLUT;
                    break;
            }
            pstDefaultClut = pstClut;
            break;
        }

        pstRegion = pstRegion->pstNextRegion;
    }

    while (pstRegion->pstNextRegion)
    {
        pstClut = SUBT_Parse_FindClutId(pstParseInfo, pstRegion->u8ClutId);
        if (pstClut)
        {
            switch (pstRegion->u8RegionDepth)
            {
                case 2:
                    if (pstDefaultClut)
                    {
                        s32ClutCmp = memcmp(pstDefaultClut->_2BitCLUT, pstClut->_2BitCLUT, sizeof(pstClut->_2BitCLUT));
                    }
                    break;
                case 4:
                    if (pstDefaultClut)
                    {
                        s32ClutCmp = memcmp(pstDefaultClut->_4BitCLUT, pstClut->_4BitCLUT, sizeof(pstClut->_4BitCLUT));
                    }
                    break;
                case 8:
                default:
                    s32ClutCmp = 0;
                    break;
            }
            if (s32ClutCmp)
            {
                mt_u32 u32DataSize = (mt_u32)(pstRegion->u16Width * pstRegion->u16Heigth);

                /* have different clut in a page, commix clut */
                switch (pstRegion->u8RegionDepth)
                {
                    case 2:
                        if (0 == count && pstDefaultClut)
                        {
                            memcpy(pstCurPage->stDefaultClut._8BitCLUT, pstDefaultClut->_2BitCLUT, sizeof(pstDefaultClut->_2BitCLUT));
                        }
                        memcpy(&pstCurPage->stDefaultClut._8BitCLUT[CLUT_BIT_4 * (count + 1)], pstClut->_2BitCLUT, sizeof(pstClut->_2BitCLUT));

                        for (i = 0; i < u32DataSize; i++)
                        {
                            pstRegion->pu8ShowData[i] = (mt_u8)(pstRegion->pu8ShowData[i] + 4 * (count + 1));
                        }
                        break;
                    case 4:
                        if (0 == count && pstDefaultClut)
                        {
                            memcpy(pstCurPage->stDefaultClut._8BitCLUT, pstDefaultClut->_4BitCLUT, sizeof(pstDefaultClut->_4BitCLUT));
                        }
                        memcpy(&pstCurPage->stDefaultClut._8BitCLUT[CLUT_BIT_16 * (count + 1)], pstClut->_4BitCLUT, sizeof(pstClut->_4BitCLUT));

                        for (i = 0; i < u32DataSize; i++)
                        {
                            pstRegion->pu8ShowData[i] = (mt_u8)(pstRegion->pu8ShowData[i] + 16 * (count + 1));
                        }
                        break;
                    default:
                        break;
                }

                count++;
            }
        }

        pstRegion = pstRegion->pstNextRegion;
    }

    return count;
}

static mt_s32 SUBT_DataParse_CountRegionSize(SUBT_PARSE_INFO_S *pstParseInfo, mt_u32 *pu32AllRegionSize,
                                             mt_u16 *pu16X, mt_u16 *pu16W, mt_u16 *pu16H)
{
    SUBT_SHOW_REGION_S *pstRegion = MT_NULL;
    mt_u16 u16RegionX = 0, u16RegionW = 0, u16RegionH = 0;
    mt_u16 u16PrevRegionY = 0, u16PrevRegionH = 0;
    SUBT_PAGE_PARAM_S *pstCurPage = &pstParseInfo->stPageParam;

    if (MT_NULL == pstParseInfo || MT_NULL == pu32AllRegionSize
        || MT_NULL == pu16X || MT_NULL == pu16W || MT_NULL == pu16H)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    pstRegion = pstCurPage->pstShowRegion;

    /* the first region */
    u16RegionX = pstRegion->u16XPos;
    u16RegionW = pstRegion->u16Width;
    u16RegionH = pstRegion->u16Heigth;
    u16PrevRegionY = pstRegion->u16YPos;
    u16PrevRegionH = pstRegion->u16Heigth;
    pstRegion = pstRegion->pstNextRegion;

    while(pstRegion->pstNextRegion)
    {
        if (pstRegion->u16XPos < u16RegionX)
        {
            u16RegionW = (mt_u16)(u16RegionW + u16RegionX - pstRegion->u16XPos);
            u16RegionX = pstRegion->u16XPos;
        }
        if (pstRegion->u16Width > u16RegionW)
        {
            u16RegionW = pstRegion->u16Width;
        }

        if (pstRegion->u16YPos >= (u16PrevRegionY + u16PrevRegionH))
        {
            u16RegionH = (mt_u16)(u16RegionH + pstRegion->u16YPos + pstRegion->u16Heigth - (u16PrevRegionY + u16PrevRegionH));
        }
        else
        {
            u16RegionH = (mt_u16)(u16RegionH + pstRegion->u16Heigth);
        }
        
        if (pstRegion->u16YPos < u16PrevRegionY)
        {
            MT_ERR_SUBT("the region y isn't ascending!!!\n");
            return MT_FAILURE;
        }
        u16PrevRegionY = pstRegion->u16YPos;
        u16PrevRegionH = pstRegion->u16Heigth;

        pstRegion = pstRegion->pstNextRegion;
    }

    /* the all region size */
    *pu16X = u16RegionX;
    *pu16W = u16RegionW;
    *pu16H = u16RegionH;

    //printf("resize region : %d %d\n",u16RegionW,u16RegionH);

    //*pu32AllRegionSize = (mt_u32)((*pu16W) * (*pu16H) + 3) & (~3);
    *pu32AllRegionSize = (mt_u32)((*pu16W) * (*pu16H));

    return MT_SUCCESS;
}

static mt_s32 SUBT_DataParse_Display(SUBT_PARSE_INFO_S *pstParseInfo)
{
    mt_s32 s32Ret = MT_SUCCESS;
    SUBT_SHOW_REGION_S *pstRegion = NULL;
    mt_u32 u32DataSize = 0;
    mt_u32 u32CopyLen = 0;
    SUBT_PAGE_PARAM_S *pstCurPage = NULL;
    SUBT_Display_ITEM_S stDisplayItem;
    mt_u16 u16NextRegionY = 0;
    mt_u16 u16AllRegionW = 0, u16AllRegionH = 0;

    if (pstParseInfo == NULL)
    {
        MT_ERR_SUBT("parameter is invalid...\n");

        return MT_FAILURE;
    }

    memset(&stDisplayItem, 0, sizeof(SUBT_Display_ITEM_S));

    pstCurPage = &pstParseInfo->stPageParam;
    pstRegion = pstCurPage->pstShowRegion;

    if(pstRegion->pstNextRegion)
    {
        stDisplayItem.u8DataType   = pstRegion->enObjType;

        stDisplayItem.u16YPos = pstRegion->u16YPos;
        if (0 != SUBT_DataParse_AdjustClut(pstParseInfo))
        {
            stDisplayItem.u16PaletteItem = sizeof(pstCurPage->stDefaultClut._8BitCLUT);
            stDisplayItem.pvRegionClut = pstCurPage->stDefaultClut._8BitCLUT;
        }
        else
        {
            stDisplayItem.u16PaletteItem = (mt_u16)(pstRegion->u16RegionClutLen * sizeof(SUBT_COLOR_S));
            stDisplayItem.pvRegionClut = pstRegion->pvRegionClut;
        }

        stDisplayItem.u8BitDepth   = 8;
        stDisplayItem.u32PTS       = pstCurPage->u32PTS;
        stDisplayItem.u32Timeout   = (mt_u32)(pstCurPage->u8ShowTime*1000);

        s32Ret = SUBT_DataParse_CountRegionSize(pstParseInfo, &u32DataSize, &stDisplayItem.u16XPos, &u16AllRegionW, &u16AllRegionH);
        if (MT_SUCCESS != s32Ret)
        {
            MT_ERR_SUBT("failed to SUBT_DataParse_CountRegionSize!!!\n");
            return MT_FAILURE;
        }
        if (u32DataSize > MAX_DATA_LENGTH)
        {
            MT_ERR_SUBT("over the max size:%d\n", MAX_DATA_LENGTH);
            return MT_FAILURE;
        }

        /* allot buffer for show region data */
        if (u32DataSize > pstParseInfo->u32RegionShowBufLen)
        {
            if (pstParseInfo->pu8RegionShowBuf)
            {
                free((mt_void*)pstParseInfo->pu8RegionShowBuf);
                pstParseInfo->pu8RegionShowBuf = NULL;
            }
            pstParseInfo->pu8RegionShowBuf = (mt_u8*)malloc(u32DataSize);
            if (NULL == pstParseInfo->pu8RegionShowBuf)
            {
                MT_ERR_SUBT("Malloc size:%u failure!!!\n",MAX_DATA_LENGTH);

                return MT_FAILURE;
            }
            pstParseInfo->u32RegionShowBufLen = u32DataSize;
            memset(pstParseInfo->pu8RegionShowBuf, 0, u32DataSize);
        }

        /* start layout */
        MT_INFO_SUBT("layout start\n");
        u16NextRegionY = (mt_u16)(pstRegion->u16YPos + pstRegion->u16Heigth);
        u32CopyLen = 0;
        u32DataSize = 0;
        while(pstRegion->pstNextRegion)
        {
            mt_u16 u16OffsetX = (mt_u16)(pstRegion->u16XPos - stDisplayItem.u16XPos);

            if (u16AllRegionW == pstRegion->u16Width)
            {
                //u32DataSize = (pstRegion->u16Width * pstRegion->u16Heigth + 3 ) & (~3);
                u32DataSize = (mt_u32)(pstRegion->u16Width * pstRegion->u16Heigth);

                stDisplayItem.u32RegionDataSize += u32DataSize;

                memcpy(&pstParseInfo->pu8RegionShowBuf[u32CopyLen], pstRegion->pu8ShowData, u32DataSize);
                u32CopyLen += u32DataSize;

                MT_INFO_SUBT("layout region : %d %d\n",pstRegion->u16Width,pstRegion->u16Heigth);
            }
            else if (u16AllRegionW >= (pstRegion->u16Width + u16OffsetX)) /* the left size */
            {
                mt_u16 i = 0;

                memset(&pstParseInfo->pu8RegionShowBuf[u32CopyLen], 0, (size_t)(u16AllRegionW * pstRegion->u16Heigth));
                for (i = 0; i < pstRegion->u16Heigth; i++)
                {
                    memcpy(&pstParseInfo->pu8RegionShowBuf[u32CopyLen + u16OffsetX], &pstRegion->pu8ShowData[i * pstRegion->u16Width], pstRegion->u16Width);
                    u32CopyLen += u16AllRegionW;
                }
                stDisplayItem.u32RegionDataSize += (mt_u32)(u16AllRegionW * pstRegion->u16Heigth);

                MT_INFO_SUBT("layout region : %d %d\n",pstRegion->u16Width,pstRegion->u16Heigth);
            }
            else
            {
                MT_ERR_SUBT("region width size(%d)(%d) error!!!\n",pstRegion->u16Width, u16AllRegionW);

                return MT_FAILURE;
            }

            /* the interval size */
            if (pstRegion->u16YPos > u16NextRegionY)
            {
                //u32DataSize = (u16AllRegionW * (pstRegion->u16YPos - u16NextRegionY) + 3 ) & (~3);
                u32DataSize = (mt_u32)(u16AllRegionW * (pstRegion->u16YPos - u16NextRegionY));
                memset(&pstParseInfo->pu8RegionShowBuf[u32CopyLen], 0, u32DataSize);
                stDisplayItem.u32RegionDataSize += u32DataSize;
                u32CopyLen += u32DataSize;
            }
            u16NextRegionY = (mt_u16)(pstRegion->u16YPos + pstRegion->u16Heigth);

            pstRegion = pstRegion->pstNextRegion;
        }
        /* end layout */
        MT_INFO_SUBT("layout end\n");

        stDisplayItem.u16Width = u16AllRegionW;
        stDisplayItem.u16Heigth = u16AllRegionH;
        stDisplayItem.pu8ShowData = pstParseInfo->pu8RegionShowBuf;
        stDisplayItem.enPageState = pstCurPage->enPageState;

        if (0 == pstCurPage->stDisplayDef.u16DisplayWidth && 0 == pstCurPage->stDisplayDef.u16DisplayHeight)
        {
            if(720 >= stDisplayItem.u16Width)
            {
                stDisplayItem.u16DisplayWidth = 720;
                stDisplayItem.u16DisplayHeight = 576;
            }
            else  if(1280 >= stDisplayItem.u16Width)
            {
                stDisplayItem.u16DisplayWidth = 1280;
                stDisplayItem.u16DisplayHeight = 720;
            }
            else /* if(1920 >= stDisplayItem.u16Width)*/
            {
                stDisplayItem.u16DisplayWidth = 1920;
                stDisplayItem.u16DisplayHeight = 1080;
            }
        }
        else
        {
            /* have DDS */
            stDisplayItem.u16DisplayWidth = (mt_u16)((pstCurPage->stDisplayDef.u16DisplayWidth + 3) & (~3));
            stDisplayItem.u16DisplayHeight = (mt_u16)((pstCurPage->stDisplayDef.u16DisplayHeight + 3) & (~3));
        }

        if (stDisplayItem.u16Width * stDisplayItem.u16Heigth != stDisplayItem.u32RegionDataSize)
        {
            MT_ERR_SUBT("length not match, [%d][%d] != [%d]\n", stDisplayItem.u16Width,
                    stDisplayItem.u16Heigth, stDisplayItem.u32RegionDataSize);
            return MT_FAILURE;
        }

        if (pstParseInfo->pfnCallback)
        {
            s32Ret = pstParseInfo->pfnCallback(pstParseInfo->u32UserData, (mt_void *)&stDisplayItem);
            if (MT_SUCCESS != s32Ret)
            {
                MT_WARN_SUBT("failed to callback function\n");
            }
        }

        pstCurPage->pstShowRegion->pstNextRegion = NULL;
        s32Ret = MT_SUCCESS;
    }
    else
    {
        MT_INFO_SUBT("no show region !\n");

        if (pstCurPage->enPageState == SUBTITLE_PAGE_MODE_CHANGE 
            || pstCurPage->enPageState == SUBTITLE_PAGE_ACQUISITION_POINT)
        {
            /* Just need to clear osd, no region to show */
            stDisplayItem.u8DataType= SUBT_OBJ_TYPE_BITMAP;

            stDisplayItem.u16XPos   = 0; /* This indicates no data of the region to display */
            stDisplayItem.u16YPos   = 0;
            stDisplayItem.u16Width  = 0;
            stDisplayItem.u16Heigth = 0;

            stDisplayItem.pvRegionClut = pstParseInfo->stPageParam.stDefaultClut._8BitCLUT;
            stDisplayItem.u8BitDepth   = 8;
            stDisplayItem.u32PTS       = pstCurPage->u32PTS;
            stDisplayItem.u32Timeout   = (mt_u32)(pstCurPage->u8ShowTime*1000);

            stDisplayItem.u32RegionDataSize = (pstParseInfo->u32RegionShowBufLen > 8) ? 8 : pstParseInfo->u32RegionShowBufLen;
            stDisplayItem.pu8ShowData = pstParseInfo->pu8RegionShowBuf;
            stDisplayItem.enPageState = SUBTITLE_PAGE_ACQUISITION_POINT;
            stDisplayItem.u16PaletteItem = 256;
            if (pstParseInfo->pfnCallback)
            {
                s32Ret = pstParseInfo->pfnCallback(pstParseInfo->u32UserData, (mt_void *)&stDisplayItem);
                if (MT_SUCCESS != s32Ret)
                {
                    MT_WARN_SUBT("failed to callback function\n");
                }
            }
            s32Ret = MT_SUCCESS;
        }
    }

    return s32Ret;
}

/* Parse PES packet, delete the packet header and get the subtitling segment data
 *
 */
mt_s32 SUBT_DataParse_ParsePESPacket(MT_HANDLE hDataParse, mt_u8 *pu8DataSrc , mt_u32 u32Len, mt_u16 u16PageID, mt_u16 u16AncillaryID)

{
    mt_s32  s32Ret = MT_FAILURE;
    mt_u8   u8PtsExistFlag = 0;
    mt_u8   u8PesHeadLength = 0;
    mt_u8   u8PesStuffingLength = 0;
    mt_u32  u32PTSVal =  0;
    mt_u8  *pu8Data = pu8DataSrc;
    mt_u32  u32DataLen = u32Len;
    mt_u32  i = 0;

    SUBT_PARSE_INFO_S * pstParseInfo = (SUBT_PARSE_INFO_S *)hDataParse;

    SUBT_PAGE_PARAM_S *pstCurPage;

    if (NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    pstCurPage = &pstParseInfo->stPageParam;

    /* skip pes start code: 00 00 01 BD */
    pu8Data += 4;
    u8PesHeadLength = (mt_u8)(u8PesHeadLength + 4);

    pu8Data += 3; /* skip length two byte and flag one byte PTS_DTS_FLAGs, ref[13818-1] Table2-17 page.42 */
    u8PesHeadLength = (mt_u8)(u8PesHeadLength + 3);

    u8PtsExistFlag = pu8Data[0] & 0x80;

    pu8Data += 1; /* skip flag one byte, pointed to PES_header_data_length */
    u8PesHeadLength = (mt_u8)(u8PesHeadLength + 1);

    u8PesStuffingLength = pu8Data[0]; /* options field and stuffing field length */

    MT_INFO_SUBT("in this pes packet, options field and stuffing field length is :%d\n", u8PesStuffingLength);

    pu8Data += 1; /*skip header stuffing length field */
    u8PesHeadLength = (mt_u8)(u8PesHeadLength + 1);
    /* get the PTS */
    u32PTSVal = 0 ;
    if( u8PtsExistFlag )
    {
        /* '0010 PTS[32..30] 0 PTS[29..15] 0 PTS[14..0] 0' */
        u32PTSVal = (mt_u32)(((pu8Data[0] & 0x0e) << 29) | ((pu8Data[1] & 0xff) << 22 )
                | ((pu8Data[2] & 0xfe) << 14 )  | ((pu8Data[3] & 0xff) << 7 )
                | ((pu8Data[4] & 0xfe) >> 1 )) ;

        pstCurPage->u32PTS = u32PTSVal/90;//2\TODO:it up to how to save PTS
        if (pu8Data[0] & 0x08)
        {
            pstCurPage->u32PTS += 0x2D82D82;//(1 << 32) / 90;
        }
    }
    else
    {
        MT_INFO_SUBT("pts not exist in this pes packet !\n");
        return MT_SUCCESS;
    }

    pu8Data += u8PesStuffingLength;
    u8PesHeadLength = (mt_u8)(u8PesHeadLength + u8PesStuffingLength);

    u8PesHeadLength = (mt_u8)(u8PesHeadLength + 2); /* 2 = 1(data_identifier) + 1(subtitle_stream_id)  */
    if( u32DataLen >= u8PesHeadLength)
    {
        /* recv the whole pes head */
        if( (0x20 == pu8Data[0])  && (0x00 == pu8Data[1] ) ) /* 0x20--subtitle stream data_identifier, 0x00--subtitle stream_id */
        {
            pu8Data += 2;
            if ( (pu8Data != (&pu8DataSrc[u8PesHeadLength])) )
            {
                printf("\nerror, pu8Data addr:%p, 0x%02x, &pDataSrc[u8PesHeadLength] addr:%p, 0x%02x\n",
                        pu8Data, pu8Data[0], &pu8DataSrc[u8PesHeadLength], pu8DataSrc[u8PesHeadLength]);

                return MT_FAILURE;
            }
            
            pstParseInfo->error_status  = 0;
            do{
                i ++ ; 
                /* 6 is the startcode(4 bytes) plus PES_packet_length field(2 bytes), because u8PesHeadLength included that field */
                s32Ret = SUBT_Parse_CreateSegment(pstParseInfo, &pu8DataSrc[u8PesHeadLength],u32DataLen + 6 - u8PesHeadLength, u16PageID, u16AncillaryID);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_SUBT("failed to SUBT_Parse_CreateSegment...\n");
                }
                s32Ret |= SUBT_Parse_ParseSegment(pstParseInfo);
                if (s32Ret != MT_SUCCESS)
                {
                    MT_ERR_SUBT("failed to SUBT_Parse_ParseSegment....\n");
                    if(s32Ret == ERR_OUT_OF_WIDTH)
                    {   
						pstParseInfo->error_status = 1;              
                    }
                }
            }while(s32Ret == ERR_OUT_OF_WIDTH && i < 2);

        }
        else
        {
            MT_ERR_SUBT("Subt PES data identifier is error\n");
        }

    }
    else
    {
        MT_ERR_SUBT("Subt PES head is error\n");
    }

    return s32Ret;
}

mt_s32 SUBT_DataParse_Init(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SUBT_DataParse_DeInit(mt_void)
{
    return MT_SUCCESS;
}

mt_s32 SUBT_DataParse_Create(MT_HANDLE* phDataParse)
{
    mt_s32 s32Ret = MT_FAILURE;

    SUBT_PARSE_INFO_S * pstParseInfo = NULL;

    SUBT_SEGMENT_S *pstSegment = NULL;

    mt_u16 u16Index = 0;

    pstParseInfo = (SUBT_PARSE_INFO_S*)malloc(sizeof(SUBT_PARSE_INFO_S));
    if (NULL == pstParseInfo)
    {
        MT_ERR_SUBT("malloc failure....\n");
        return s32Ret;
    }


    memset(pstParseInfo, 0, sizeof(SUBT_PARSE_INFO_S));

    pstSegment = pstParseInfo->stPageParam.astSegment;
    for (u16Index = 0; u16Index < (SUBT_SEGMENT_IN_PES_MAX-1); u16Index++)
    {
        pstSegment[u16Index].pstNext = &pstSegment[u16Index+1];
    }
    pstSegment[SUBT_SEGMENT_IN_PES_MAX-1].pstNext = NULL;

    pstParseInfo->stPageParam.stDisplayDef.u8DdsVersionNumber = 0xFF;

    pstParseInfo->u32RegionBufferLen = SUBT_REGION_SHOW_SD_LEN;
    pstParseInfo->pu8RegionBuffer = (mt_u8*)malloc(pstParseInfo->u32RegionBufferLen);
    if (pstParseInfo->pu8RegionBuffer == MT_NULL)
    {
        free(pstParseInfo);
        pstParseInfo = MT_NULL;

        MT_ERR_SUBT("malloc failure....\n");

        return MT_FAILURE;
    }
    memset(pstParseInfo->pu8RegionBuffer, 0, pstParseInfo->u32RegionBufferLen);
    pstParseInfo->stPageParam.pstShowRegion = (SUBT_SHOW_REGION_S*)pstParseInfo->pu8RegionBuffer;

    *phDataParse = (MT_HANDLE)pstParseInfo;

    MT_INFO_SUBT("success with handle:0x%08x!\n",*phDataParse);

    return MT_SUCCESS;
}

mt_s32 SUBT_DataParse_Destroy(MT_HANDLE hDataParse)
{
    SUBT_PARSE_INFO_S * pstParseInfo = (SUBT_PARSE_INFO_S *)hDataParse;

    if (NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    MT_INFO_SUBT("begin using handle:0x%08x!\n", pstParseInfo);


    if (pstParseInfo->pu8RegionShowBuf)
    {
        free((mt_void*)pstParseInfo->pu8RegionShowBuf);
    }

    if (pstParseInfo->pu8RegionBuffer)
    {
        free((mt_void *)pstParseInfo->pu8RegionBuffer);
    }

    free((mt_void*)pstParseInfo);

    MT_INFO_SUBT("success!\n");

    return MT_SUCCESS;
}

mt_s32 SUBT_DataParse_Reset(MT_HANDLE hDataParse, MT_BOOL bParseFlag)
{
    SUBT_SEGMENT_S *pstSegment = NULL;
    mt_u32 i;

    mt_u16 u16Index = 0;

    SUBT_PARSE_INFO_S * pstParseInfo = (SUBT_PARSE_INFO_S *)hDataParse;

    if (NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    memset(&pstParseInfo->stPageParam, 0, sizeof(pstParseInfo->stPageParam));

    /* reset region clut */
    memcpy(pstParseInfo->stPageParam.stDefaultClut._2BitCLUT, s_2BitDefaultCLUT, sizeof(s_2BitDefaultCLUT));
    memcpy(pstParseInfo->stPageParam.stDefaultClut._4BitCLUT, s_4BitDefaultCLUT, sizeof(s_4BitDefaultCLUT));
    memcpy(pstParseInfo->stPageParam.stDefaultClut._8BitCLUT, s_8BitDefaultCLUT, sizeof(s_8BitDefaultCLUT));
    for (i = 0; i < SUBT_CLUT_IN_PAGE_MAX; i++)
    {
        pstParseInfo->stPageParam.astClut[i].u8ClutVersion = 0xff;
        pstParseInfo->stPageParam.astClut[i].u8ClutId = 0xff;
           
        memcpy(pstParseInfo->stPageParam.astClut[i]._2BitCLUT, s_2BitDefaultCLUT, sizeof(s_2BitDefaultCLUT));
        memcpy(pstParseInfo->stPageParam.astClut[i]._4BitCLUT, s_4BitDefaultCLUT, sizeof(s_4BitDefaultCLUT));
        memcpy(pstParseInfo->stPageParam.astClut[i]._8BitCLUT, s_8BitDefaultCLUT, sizeof(s_8BitDefaultCLUT));
    }
    
    memset(pstParseInfo->pu8RegionBuffer, 0, pstParseInfo->u32RegionBufferLen);
    pstParseInfo->stPageParam.pstShowRegion = (SUBT_SHOW_REGION_S*)pstParseInfo->pu8RegionBuffer;

    pstSegment = pstParseInfo->stPageParam.astSegment;
    for (u16Index = 0; u16Index < (SUBT_SEGMENT_IN_PES_MAX-1); u16Index++)
    {
        pstSegment[u16Index].pstNext = &pstSegment[u16Index+1];
    }
    pstSegment[SUBT_SEGMENT_IN_PES_MAX-1].pstNext = NULL;

    return MT_SUCCESS;
}

mt_s32 SUBT_DataParse_Update(MT_HANDLE hDataParse, SUBT_DATAPARSE_CALLBACK_FN pfnCallback, ulong u32UserData)
{
    SUBT_PARSE_INFO_S * pstParseInfo = (SUBT_PARSE_INFO_S *)hDataParse;

    if (NULL == pstParseInfo)
    {
        MT_ERR_SUBT("param is NULL...\n");

        return MT_FAILURE;
    }

    pstParseInfo->pfnCallback = pfnCallback;
    pstParseInfo->u32UserData = u32UserData;

    return MT_SUCCESS;
}



