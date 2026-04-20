//--=========================================================================--
//  This file is a part of VPU Reference API project
//-----------------------------------------------------------------------------
//
//       This confidential and proprietary software may be used only
//     as authorized by a licensing agreement from Chips&Media Inc.
//     In the event of publication, the following notice is applicable:
//
//            (C) COPYRIGHT 2006 - 2011  CHIPS&MEDIA INC.
//                      ALL RIGHTS RESERVED
//
//       The entire notice above must be reproduced on all authorized
//       copies.
//
//--=========================================================================--

#ifndef VPUAPI_UTIL_H_INCLUDED
#define VPUAPI_UTIL_H_INCLUDED

#include "vpuapi.h"

// COD_STD
enum {
    HEVC_DEC = 0,
    AVC_DEC = 0,
    VC1_DEC = 1,
    MP2_DEC = 2,
    MP4_DEC = 3,
    DV3_DEC = 3,
    RV_DEC = 4,
    AVS_DEC = 5,
#ifdef SUPPORT_PPU_STANDALONE
    PPU_STAND_ALONE = 6,
#endif
    VPX_DEC = 7,
    AVC_ENC = 8,
    MP4_ENC = 11,
    MAX_CODECS,
};

// AUX_COD_STD
enum {
    MP4_AUX_MPEG4 = 0,
    MP4_AUX_DIVX3 = 1
};

enum {
    VPX_AUX_THO = 0,
    VPX_AUX_VP6 = 1,
    VPX_AUX_VP8 = 2
};

enum {
    AVC_AUX_AVC = 0,
    AVC_AUX_MVC = 1
};

// BIT_RUN command
enum {
    SEQ_INIT = 1,
    SEQ_END = 2,
    PIC_RUN = 3,
    SET_FRAME_BUF = 4,
    ENCODE_HEADER = 5,
    ENC_PARA_SET = 6,
    DEC_PARA_SET = 7,
    DEC_BUF_FLUSH = 8,
    RC_CHANGE_PARAMETER = 9,
    VPU_SLEEP = 10,
    VPU_WAKE = 11,
    ENC_ROI_INIT = 12,
    FIRMWARE_GET = 0xf
};
#define MAX_GDI_IDX 31
#define MAX_REG_FRAME MAX_GDI_IDX * 2 // 2 for WTL

/**
 * PRODUCT: CODA960/CODA980/WAVE320
 */
typedef struct
{
    union
    {
	struct
	{
	    int useBitEnable;
	    int useIpEnable;
	    int useDbkYEnable;
	    int useDbkCEnable;
	    int useOvlEnable;
	    int useBtpEnable;
	    PhysicalAddress bufBitUse;
	    PhysicalAddress bufIpAcDcUse;
	    PhysicalAddress bufDbkYUse;
	    PhysicalAddress bufDbkCUse;
	    PhysicalAddress bufOvlUse;
	    PhysicalAddress bufBtpUse;
	} coda9;
	struct
	{
	    int useIpEnable;
	    int useLfRowEnable;
	    int useBitEnable;
	    PhysicalAddress bufIp;
	    PhysicalAddress bufLfRow;
	    PhysicalAddress bufBit;
	} wave4;
    } u;
    int bufSize;
    PhysicalAddress bufBase;
} SecAxiInfo;

typedef struct
{
    DecOpenParam openParam;
    DecInitialInfo initialInfo;
    PhysicalAddress streamWrPtr;
    PhysicalAddress streamRdPtr;
    Int32 streamBufFull;
    int streamEndflag;
    int frameDisplayFlag;
    int clearDisplayIndexes;
    int setDisplayIndexes;
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    PhysicalAddress frameDisplayFlagRegAddr;
    PhysicalAddress currentPC;
    PhysicalAddress busyFlagAddr;
    int streamBufSize;
    FrameBuffer frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t vbFrame;
    vpu_buffer_t vbWTL;
    vpu_buffer_t vbPPU;
    vpu_buffer_t vbMV;
    vpu_buffer_t vbFbcYTbl;
    vpu_buffer_t vbFbcCTbl;

    int frameAllocExt;
    int ppuAllocExt;
    int numFrameBuffers;
    int stride;
    int frameBufferHeight;
    int rotationEnable;
    int mirrorEnable;
    int deringEnable;
    MirrorDirection mirrorDirection;
    int rotationAngle;
    FrameBuffer rotatorOutput;
    int rotatorStride;
    int rotatorOutputValid;
    int initialInfoObtained;
    int vc1BframeDisplayValid;
    int mapType;
    int tiled2LinearEnable;
    int tiled2LinearMode;
    int wtlEnable;
    int wtlMode;
    FrameBufferFormat wtlFormat; /*!<< default value: FORMAT_420 8bit */
    SecAxiInfo secAxiInfo;
    MaverickCacheConfig cacheConfig;
    int chunkSize;
    int seqInitEscape;

    // Report Information
    PhysicalAddress userDataBufAddr;
    vpu_buffer_t vbUserData;
    Uint32 seiMask;     /* Supported on WAVE4xx  */
    int userDataEnable; // User Data
    int userDataBufSize;
    int userDataReportMode; // User Data report mode (0 : interrupt mode, 1 interrupt disable mode)

#ifdef SUPPORT_MEM_PROTECT
    WriteMemProtectCfg writeMemProtectCfg;
#endif /* SUPPORT_MEM_PROTECT */

    LowDelayInfo lowDelayInfo;
    int frameStartPos;
    int frameEndPos;
    int frameDelay;
    vpu_buffer_t vbSlice; // AVC, VP8 only
    vpu_buffer_t vbWork;
    vpu_buffer_t vbTemp;
    vpu_buffer_t vbReport;
    int workBufferAllocExt;
    DecOutputInfo decOutInfo[MAX_GDI_IDX];
    TiledMapConfig mapCfg;
    int reorderEnable;
    DRAMConfig dramCfg; //coda960 only
    int thumbnailMode;
    int seqChangeMask;                 // WAVE410
    MT_BOOL repeatEmptyInterruptInterval; /* in millisecond : 0 ~ 1000*/
} DecInfo;

typedef struct
{
    EncOpenParam openParam;
    EncInitialInfo initialInfo;
    PhysicalAddress streamRdPtr;
    PhysicalAddress streamWrPtr;
    int streamEndflag;
    PhysicalAddress streamRdPtrRegAddr;
    PhysicalAddress streamWrPtrRegAddr;
    PhysicalAddress streamBufStartAddr;
    PhysicalAddress streamBufEndAddr;
    PhysicalAddress currentPC;
    PhysicalAddress busyFlagAddr;
    int streamBufSize;
    int linear2TiledEnable;
    int linear2TiledMode; // coda980 only
    int mapType;
    int userMapEnable;
    FrameBuffer frameBufPool[MAX_REG_FRAME];
    vpu_buffer_t vbFrame;
    vpu_buffer_t vbPPU;
    int frameAllocExt;
    int ppuAllocExt;
    vpu_buffer_t vbSubSampFrame;    /*!<< CODA960 only */
    vpu_buffer_t vbMvcSubSampFrame; /*!<< CODA960 only */
    int numFrameBuffers;
    int stride;
    int frameBufferHeight;
    int rotationEnable;
    int mirrorEnable;
    MirrorDirection mirrorDirection;
    int rotationAngle;
    int initialInfoObtained;
    int ringBufferEnable;
    SecAxiInfo secAxiInfo;
    MaverickCacheConfig cacheConfig;
    EncSubFrameSyncConfig subFrameSyncConfig; /*!<< CODA980 & WAVE320 */

#ifdef SUPPORT_MEM_PROTECT
    WriteMemProtectCfg writeMemProtectCfg;
#endif
    int ActivePPSIdx; /*!<< CODA980 */
    int frameIdx;     /*!<< CODA980 */
    int fieldDone;    /*!<< CODA980 */
    int lineBufIntEn;
    vpu_buffer_t vbWork;
    vpu_buffer_t vbScratch;
    TiledMapConfig mapCfg;
    DRAMConfig dramCfg; /*!<< CODA960 */
} EncInfo;

typedef struct CodecInst
{
    Int32 inUse;
    Int32 instIndex;
    Int32 coreIdx;
    Int32 productId;
    Int32 codecMode;
    Int32 codecModeAux;
    Int32 loggingEnable;
    Uint32 errorReasonCode;
    union
    {
	EncInfo encInfo;
	DecInfo decInfo;
    } CodecInfo;
} CodecInst;

#ifdef __cplusplus
extern "C" {
#endif

RetCode InitCodecInstancePool(Uint32 coreIdx);
RetCode GetCodecInstance(Uint32 coreIdx, CodecInst **ppInst);
void FreeCodecInstance(CodecInst *pCodecInst);

RetCode CheckDecOpenParam(DecOpenParam *pop);
int DecBitstreamBufEmpty(DecInfo *pDecInfo);
RetCode SetParaSet(DecHandle handle, int paraSetType, DecParamSet *para);
void DecSetHostParaAddr(Uint32 coreIdx, PhysicalAddress baseAddr, PhysicalAddress paraAddr);

#ifdef SUPPORT_MEM_PROTECT
int SetDecWriteProtectRegions(CodecInst *inst);
void ConfigDecWPROTRegion(int coreIdx, DecInfo *pDecInfo);
int SetEncWriteProtectRegions(CodecInst *inst);
void ConfigEncWPROTRegion(int coreIdx, EncInfo *pEncInfo, WriteMemProtectCfg *pCgf);
#endif

int ConfigSecAXICoda9(Uint32 coreIdx, CodStd codStd, SecAxiInfo *sa, int width, int height, int profile);
int ConfigSecAXIWave4(Uint32 coreIdx, CodStd codStd, SecAxiInfo *sa, int width, int height, int profile);
RetCode AllocateFrameBufferArrayV10(int coreIdx, FrameBuffer *frambufArray, vpu_buffer_t *pvbFrame, int mapType, int interleave, int framebufFormat, int num, int stride, int memHeight, int gdiIndex, int fbType, PhysicalAddress tiledBaseAddr, DRAMConfig *pDramCfg);

RetCode CheckEncInstanceValidity(EncHandle handle);
RetCode CheckEncOpenParam(EncOpenParam *pop);
RetCode CheckEncParam(EncHandle handle, EncParam *param);
RetCode GetEncHeader(EncHandle handle, EncHeaderParam *encHeaderParam);
RetCode EncParaSet(EncHandle handle, int paraSetType);
RetCode SetGopNumber(EncHandle handle, Uint32 *gopNumber);
RetCode SetIntraQp(EncHandle handle, Uint32 *intraQp);
RetCode SetBitrate(EncHandle handle, Uint32 *bitrate);
RetCode SetFramerate(EncHandle handle, Uint32 *frameRate);
RetCode SetIntraRefreshNum(EncHandle handle, Uint32 *pIntraRefreshNum);
RetCode SetSliceMode(EncHandle handle, EncSliceMode *pSliceMode);
RetCode SetHecMode(EncHandle handle, int mode);
void EncSetHostParaAddr(Uint32 coreIdx, PhysicalAddress baseAddr, PhysicalAddress paraAddr);

RetCode EnterLock(Uint32 coreIdx);
RetCode LeaveLock(Uint32 coreIdx);
RetCode EnterDispFlagLock(Uint32 coreIdx);
RetCode LeaveDispFlagLock(Uint32 coreIdx);
RetCode SetClockGate(Uint32 coreIdx, Uint32 on);

void SetPendingInst(Uint32 coreIdx, CodecInst *inst);
void ClearPendingInst(Uint32 coreIdx);
CodecInst *GetPendingInst(Uint32 coreIdx);
int GetPendingInstIdx(Uint32 coreIdx);

int SetTiledMapType(Uint32 coreIdx, TiledMapConfig *pMapCfg, int mapType, int stride, int interleave, DRAMConfig *dramCfg);
int GetXY2AXIAddr(TiledMapConfig *pMapCfg, int ycbcr, int posY, int posX, int stride, FrameBuffer *fb);
Uint32 FIORead(Uint32 coreIdx, Uint32 addr);
Int32 FIOWrite(Uint32 coreIdx, Uint32 addr, Uint32 data);
//for GDI 1.0
void SetTiledFrameBase(Uint32 coreIdx, PhysicalAddress baseAddr);
PhysicalAddress GetTiledFrameBase(Uint32 coreIdx, FrameBuffer *frame, int num);

int LevelCalculation(int MbNumX, int MbNumY, int frameRateInfo, int interlaceFlag, int BitRate, int SliceNum);
#ifdef __cplusplus
}
#endif

#endif // endif VPUAPI_UTIL_H_INCLUDED
