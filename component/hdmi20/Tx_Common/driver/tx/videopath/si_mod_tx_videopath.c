/*************************************************************************
* si_drv_videopath.c
*************************************************************************/

#include "mt_hdmi20_cfg.h"
//#include "si_drv_videopath_cra.h"
#include "si_vidpath_regs.h"
#include "si_drv_tx_regs.h"
#include "si_datatypes.h"
#include "si_lib_log_api.h"
#include "si_lib_obj_api.h"
#include "si_lib_seq_api.h"
#include "si_mod_tx_videopath_api.h"
#include "si_drv_tx_api.h"

/***** Register Module name **************************************************/
SII_LIB_OBJ_MODULE_DEF(videopath);

#define BIT_MASK__VP__INPUT_SYNC_ADJUST_CONFIG__SIZE		1
#define BIT_MASK__VP__DEGEN_CONFIG__SIZE					1
#define BIT_MASK__VP__DEGEN_PIXEL_DELAY__SIZE				2
#define BIT_MASK__VP__DEGEN_LINE_DELAY__SIZE				2
#define BIT_MASK__VP__OUTPUT_MUTE__SIZE						1
#define BIT_MASK__VP__CMS_CSC1_444_422_CONFIG__SIZE  1
#define BIT_MASK__VP__CMS_CSC1_422_420_CONFIG__SIZE 1
#define BIT_MASK__VP__CMS_CSC0_420_422_CONFIG__SIZE 1
#define BIT_MASK__VP__CMS_CSC0_422_444_CONFIG__SIZE 1

#define BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE 2
#define BIT_MASK__VP__CMS_CSC0_444_422_CONFIG__SIZE 1
#define BIT_MASK__VP__CMS_CSC0_422_420_CONFIG__SIZE 1

#define BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE 2
#define BIT_MASK__VP__CMS_CSC1_420_422_CONFIG__SIZE 1
#define BIT_MASK__VP__CMS_CSC1_422_444_CONFIG__SIZE 1

#define BIT_MASK__VP__INPUT_FORMAT__SIZE 2

#define BIT_MASK__VP__INPUT_SYNC_ADJUST_CONFIG__SIZE 1

#define BIT_MASK__VP__DEGEN_CONFIG__SIZE 1
#define BIT_MASK__VP__DEGEN_PIXEL_DELAY__SIZE 2
#define BIT_MASK__VP__DEGEN_LINE_DELAY__SIZE 2

#define BIT_MASK__VP__OUTPUT_FORMAT__SIZE 2

#define BIT_MASK__VP__OUTPUT_MUTE__SIZE 1

#define BIT_MASK__VP__CMS__CSC1__DITHERING_CONFIG__SIZE 1

#define BIT_MASK__VP__FDET_STATUS__SIZE 1
#define BIT_MASK__VP__FDET_IRQ_STAT__SIZE 3
#define BIT_MASK__VP__OUTPUT_SYNC_CONFIG__SIZE				1
#define BIT_MASK__VP__FDET_IRQ_MASK__SIZE					3

#if SI_DRV_DINO_CFG
	extern bool_t isColorConv_en;
	extern bool_t isOutBItDepthChng;
	extern uint8_t outClrSpc;
	extern uint8_t outClrConvStd;
	extern uint8_t outClrDc;
#endif

typedef enum {
	RGB = 0,
	YCbCr422,
	YCbCr444,
	YCbCr420
} ColorSpace_t;

typedef enum {
	QUANTIZATION_VIDEO_LEVELS, // 16 - 235
	QUANTIZATION_PC_LEVELS // 0 - 255
} QuantizationLevel_t;

typedef struct RateConvertCfg {
	bit_fld_t     mux_420_enable;
} RateConvertCfg_t;

typedef struct EmbeddeSyncCfg {
	uint8_t tmp;
} EmbeddeSyncCfg_t;

typedef struct DeGenCfg {
	bit_fld_t enable;
	bit_fld16_t pixel_delay;
	bit_fld16_t line_delay;
} DeGenCfg_t;

typedef struct VideoBalnkCfg {
	uint8_t tmp;
} VideoBalnkCfg_t;

typedef struct SamplerCfg {
	ColorSpace_t inputClrSpc;
	ColorSpace_t outputClrSpc;
} SamplerCfg_t;

typedef struct CscConfig {
	ColorSpace_t inputClrSpc;
	ColorSpace_t outputClrSpc;
	QuantizationLevel_t inQuantization;
	QuantizationLevel_t outQuantization;
	SiiDrvConvStd_t inConvStd;
	SiiDrvConvStd_t outConvStd;
} CscConfig_t;

typedef struct GammaCorrectionConfig {
	uint8_t tmp;
} GammaCorrectionConfig_t;

typedef struct DataPathMuxCfg {
	uint8_t tmp;
} DataPathMuxCfg_t;

typedef enum {
	SYNCPOL_NEGATIVE,
	SYNCPOL_POSITIVE
} SyncPol_t;

typedef struct SyncPolAdjustCfg {
	bit_fld_t             bAutoAdjust;
	SyncPol_t             HSYNC_Pol;
	SyncPol_t             VSYNC_Pol;
} SyncPolAdjustCfg_t;

typedef struct OuputClorInfo {
	SiiDrvConvStd_t convStd;
	ColorSpace_t         clrSpc;
	SiiDrvBitDepth_t vidDcDepth;
	QuantizationLevel_t quntization;
	SiiQuantLevel_t quntization_mt;
} VideoColorInfo_t;

typedef enum {
	DITHER__12_TO_10,
	DITHER__12_TO_8,
	DITHER__10_TO_8,
	DITHER__NOCHNG,
} DitherCfg_t;

typedef struct CmsCfg {
	bit_fld_t               bEnable_420422;
	bit_fld_t               bEnable_422444;
	bit_fld_t               bEnable_csc0;
	bit_fld_t               bIsGamaCorrEnable;
	bit_fld_t               bEnable_csc1;
	bit_fld_t               bEnable_444422;
	bit_fld_t               bEnable_422420;
	bit_fld_t               bEnable_Dithering;
	VideoColorInfo_t        inClrInfo;
	VideoColorInfo_t        outClrInfo;
} CmsCfg_t;

typedef struct VideoPathConfig {
	bit_fld_t               bEnable_IDPMux;
	bit_fld_t               bEnable_IRC;
	bit_fld_t               bEnable_InSPA;
	bit_fld_t               bEnable_656D;
	bit_fld_t               bEnable_DEG;
	bit_fld_t               bEnable_CMS;
	bit_fld_t               bEnable_VB;
	bit_fld_t               bEnable_ORC;
	bit_fld_t               bEnable_ESE;
	bit_fld_t               bEnable_OutSPA;
	bit_fld_t               bEnable_ODPMux;
	bit_fld_t               bEnable_Dithering;

	VideoColorInfo_t        outClrInfo;
	VideoColorInfo_t        inClrInfo;
	SiiHvSyncPol_t			outputHVSyncPol;
	SiiInst_t				instTxCra;
} VideoPathConfig_t;

typedef struct videopathparam {
	SiiHvSyncPol_t         hvSyncPol;
	bit_fld_t              bColorConv_en;
	bit_fld_t              bDithering_en;
	VideoColorInfo_t       inClrInfo;
	VideoColorInfo_t       outClrInfo;
	uint16_t				cscMatrix[4][15];
	SiiInst_t			   instTxCra;

} VideoPathParam_t;

static void sVidPathOutputColorSpaceSet(VideoPathParam_t *pVidPathParam, SiiDrvClrSpc_t clrSpc);
static void sVidPathOutputBitDepthSet(VideoPathParam_t *pVidPathParam, SiiDrvBitDepth_t bitDepth);
static void sVideoPathColorInfoConfig(VideoPathParam_t *pVidPathParam, SiiDrvTxColorInfoCfg_t *pClrInfo);
static void sVidPathHvSyncPolaritySet(VideoPathParam_t *pVidPathParam, SiiHvSyncPol_t hvSyncPol);

static void sGetInputVideoColorInfo(VideoPathParam_t *pVidPathParam, VideoColorInfo_t* pVidClrInfo);
static void sSetVideoPathdefaults(VideoPathParam_t *pVidPathParam);

static void sVideoPathCoreConfig(VideoPathParam_t *pVidPathParam, VideoPathConfig_t* pConfig);
static void sInputDataPathMuxConfig(VideoPathParam_t *pVidPathParam, DataPathMuxCfg_t* pinMap);
static void sInputRateConverterConfig(VideoPathParam_t *pVidPathParam, RateConvertCfg_t* pCfg);
static void sInSyncPolarityAdjustmentConfig(VideoPathParam_t *pVidPathParam, SyncPolAdjustCfg_t *pCfg);
static void s656DecderConfig(VideoPathParam_t *pVidPathParam, EmbeddeSyncCfg_t* pCfg);
static void sDEGeneratorConfig(VideoPathParam_t *pVidPathParam, DeGenCfg_t* pCfg);
static void sVideoBlankingConfig(VideoPathParam_t *pVidPathParam, VideoBalnkCfg_t* pCfg);
static void sOutputRateConverterConfig(VideoPathParam_t *pVidPathParam, RateConvertCfg_t* pCfg);
static void sEmbeddedSyncEncoderConfig(VideoPathParam_t *pVidPathParam, EmbeddeSyncCfg_t* pCfg);
static void sOutSyncPolarityAdjustmentConfig(VideoPathParam_t *pVidPathParam, SyncPolAdjustCfg_t *pCfg);
static void sOutnputDataPathMuxConfig(VideoPathParam_t *pVidPathParam, DataPathMuxCfg_t* pinMap);

static void sVideoPathCmsConfig(VideoPathParam_t *pVidPathParam, CmsCfg_t* pCfg);
static void sUpSamplerConfig(VideoPathParam_t *pVidPathParam, SamplerCfg_t* pCfg);
static void sVideoPathCsc0Config(VideoPathParam_t *pVidPathParam, CscConfig_t* pCfg);
static void sGammaCorrectionConfig(VideoPathParam_t *pVidPathParam, GammaCorrectionConfig_t* pCfg);
static void sVideoPathCsc1Config(VideoPathParam_t *pVidPathParam, CscConfig_t* pCfg);
static void sDownSamplerConfig(VideoPathParam_t *pVidPathParam, SamplerCfg_t* pCfg);
static void sDitheringConfig(VideoPathParam_t *pVidPathParam, DitherCfg_t* dithConf);

static void sOutputMute(VideoPathParam_t *pVidPathParam, bool_t enMute);
static void sDecodeColorInfo(VideoPathParam_t *pVidPathParam, SiiDrvClrSpc_t clrSpc, VideoColorInfo_t* pClrInfo);
void sVidPathInputQuantSet(VideoPathParam_t *pVidPathParam, SiiQuantLevel_t quant);

//static void sVideoPathIntrHandler(SiiInst_t);
#if (__HDMI_OS_KERNEL__ || __HDMI_OS_RTOS__)
	#include "mt4si/linux_k/si_mod_tx_videopath_mt.h"
#endif
//----------------------------------------------------------------------------
SiiInst_t SiiModTxVideoPathCreate(char *pNameStr, SiiModTxVideoPathCfg_t *pConfig)
{
	VideoPathParam_t*		pvidPathParam;
	VideoPathConfig_t		vidPathCfg;
	uint32_t				vp_fdet_irq_mask;
	pvidPathParam = (VideoPathParam_t *) SII_LIB_OBJ_CREATE(pNameStr, sizeof(VideoPathParam_t));
	SII_PLATFORM_DEBUG_ASSERT(pvidPathParam);

	pvidPathParam->inClrInfo.clrSpc = YCbCr444;
	pvidPathParam->inClrInfo.convStd = SII_DRV_CONV_STD__BT_709;
	pvidPathParam->inClrInfo.vidDcDepth = SII_DRV_BIT_DEPTH__12_BIT;
	pvidPathParam->inClrInfo.quntization = QUANTIZATION_VIDEO_LEVELS;
	pvidPathParam->inClrInfo.quntization_mt = QUANTIZATION_BUTT;

	pvidPathParam->bColorConv_en = 0;
	pvidPathParam->instTxCra = pConfig->instTxCra;

	//Disabling Input datapath mux as it is required only if input tmsd lines are swapped
	vidPathCfg.bEnable_IDPMux = 0;
	vidPathCfg.bEnable_ODPMux = 0;

	//presently not using 656 decoder and embedded sync encoder
	vidPathCfg.bEnable_656D = 0;
	vidPathCfg.bEnable_ESE = 0;

	//automatic sync polairty adjustment
	vidPathCfg.bEnable_InSPA = 1;
	vidPathCfg.bEnable_OutSPA = 0;

	vidPathCfg.bEnable_DEG = 0;

	//video blanking shall be used when required
	vidPathCfg.bEnable_VB = 0;

	//temoprarily dsiabling below modules
	vidPathCfg.bEnable_ORC = 0;
	vidPathCfg.bEnable_IRC = 0;
	vidPathCfg.bEnable_CMS = 0;
	vidPathCfg.bEnable_Dithering = 0;

	vidPathCfg.instTxCra = pConfig->instTxCra;

	sVideoPathCoreConfig(pvidPathParam, &vidPathCfg);

	//Unmask VideoPath Interrupt
	//vp_fdet_irq_mask = 0x07FFFF;
	//VideoPath Intr mask in enabled by default. Clear it to avoid group Intr.
	vp_fdet_irq_mask = 0x0;
	SiiModTxVideoPathRegWrite(pvidPathParam->instTxCra, REG_ADDR__VP__FDET_IRQ_MASK, (uint8_t *) &vp_fdet_irq_mask, BIT_MASK__VP__FDET_IRQ_MASK__SIZE);

	return (SiiInst_t)SII_LIB_OBJ_INST(pvidPathParam);
}

//----------------------------------------------------------------------------
void SiiModTxVideoPathDelete( SiiInst_t inst)
{
	VideoPathParam_t *pVidPathParam = (VideoPathParam_t *)SII_LIB_OBJ_PNTR(inst);

	SII_LIB_OBJ_DELETE(pVidPathParam);
}

void sVidPathCsc0MatrixCfgRGBOutput(VideoPathParam_t *pVidPathParam)
{
	uint8_t i;
	uint8_t nzero = 0;
	uint16_t  vp__cms__csc0__multi_csc_config_mtx[15] = {	0x1000, 0xFD5E,0xF6DC,\
															0x1000, 0x1E1A, 0x0000,\
															0x1000, 0x0000, 0x1798,\
															0x1F00, 0x1800, 0x1800,\
															0x0100, 0x0100, 0x0100
															};
	for (i=0;i<15;i++) {
		if ( pVidPathParam->cscMatrix[0][i] !=0 ) {
			nzero = 1;
			break;
		}
	}
	if ( nzero ) {
		for (i=0;i<15;i++) {
			vp__cms__csc0__multi_csc_config_mtx[i] = pVidPathParam->cscMatrix[0][i];
		}
	}

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_MULTCOEFFR1C1, (uint8_t *)&vp__cms__csc0__multi_csc_config_mtx[0], sizeof(vp__cms__csc0__multi_csc_config_mtx));
}

void sVidPathCscEnCfg(VideoPathParam_t *pVidPathParam, VideoPathConfig_t* vidPathCfg)
{
	uint16_t  vp__cms__csc0__multi_csc_config = 0;
	//uint16_t  vp__cms__csc1__multi_csc_config = 0;
	if ( vidPathCfg->inClrInfo.clrSpc != RGB && vidPathCfg->outClrInfo.clrSpc != RGB ) {
		SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
		vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE;
		SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
		//SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc1__multi_csc_config, BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE);
		//vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__ENABLE;
		//SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc1__multi_csc_config, BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE);
	} else if ( vidPathCfg->inClrInfo.clrSpc != RGB && vidPathCfg->outClrInfo.clrSpc == RGB ) {
		//SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
		//vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE;
		//vp__cms__csc0__multi_csc_config |= (BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE & 0x02);
		//SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
		sVidPathCsc0MatrixCfgRGBOutput(pVidPathParam);
	}
}

//Static functions
//********************************************************************************
void sVidPathOutputColorSpaceSet(VideoPathParam_t *pVidPathParam, SiiDrvClrSpc_t clrSpc)
{
	VideoPathConfig_t      vidPathCfg;

	SII_MEMSET(&vidPathCfg, 0, sizeof(VideoPathConfig_t));

	pVidPathParam->bColorConv_en = 1;
	sDecodeColorInfo(pVidPathParam, clrSpc, & (vidPathCfg.outClrInfo));
	vidPathCfg.outClrInfo.vidDcDepth = pVidPathParam->outClrInfo.vidDcDepth;
	//vidPathCfg.outClrInfo.clrSpc = pVidPathParam->outClrInfo.clrSpc;
	vidPathCfg.outClrInfo.convStd = pVidPathParam->outClrInfo.convStd;

	memcpy(&pVidPathParam->outClrInfo, &vidPathCfg.outClrInfo, sizeof(VideoColorInfo_t));
	//pVidPathParam->outClrInfo = vidPathCfg.outClrInfo;

	SI_HDMI20_PRINT("[%s_%d]%d,%d,%d,%d", __func__, __LINE__, (uint32_t)vidPathCfg.outClrInfo.convStd, (uint32_t)vidPathCfg.outClrInfo.clrSpc, \
					(uint32_t)vidPathCfg.outClrInfo.quntization, (uint32_t)vidPathCfg.outClrInfo.vidDcDepth);
	sGetInputVideoColorInfo(pVidPathParam, &vidPathCfg.inClrInfo);

	//set output rate converter block if required output clr space is YCBCR420
	if (vidPathCfg.outClrInfo.clrSpc == YCbCr420) {
		vidPathCfg.bEnable_ORC = 1;
	} else {
		vidPathCfg.bEnable_ORC = 0;
	}

	if (vidPathCfg.inClrInfo.clrSpc == YCbCr420) {
		vidPathCfg.bEnable_IRC = 1;
	} else {
		vidPathCfg.bEnable_IRC = 0;
	}

	sOutputMute(pVidPathParam, true);
	if ( vidPathCfg.inClrInfo.clrSpc != RGB && vidPathCfg.outClrInfo.clrSpc != RGB ) {
		vidPathCfg.bEnable_CMS = 0;
		//sOutputMute(pVidPathParam, false);
	} else {
		vidPathCfg.bEnable_CMS = 1;
	}
	sVidPathCscEnCfg(pVidPathParam, &vidPathCfg);
	vidPathCfg.instTxCra = pVidPathParam->instTxCra;

	sOutputMute(pVidPathParam, true);
	sSetVideoPathdefaults(pVidPathParam);
	sVideoPathCoreConfig(pVidPathParam, &vidPathCfg);
	sOutputMute(pVidPathParam, false);

	#if SI_DRV_DINO_CFG
	outClrSpc = (uint8_t)vidPathCfg.outClrInfo.clrSpc;
	outClrConvStd = (uint8_t)vidPathCfg.outClrInfo.convStd;
	outClrDc = (uint8_t)vidPathCfg.inClrInfo.vidDcDepth;
	isColorConv_en = true;
	DrvDinoUpdateInfoFrame(0);
	#endif
}

//-----------------------------------------------------------------------------
void sVidPathOutputBitDepthSet(VideoPathParam_t *pVidPathParam, SiiDrvBitDepth_t bitDepth)
{
	VideoPathConfig_t      vidPathCfg;

	SII_MEMSET(&vidPathCfg, 0, sizeof(VideoPathConfig_t));
	SI_HDMI20_PRINT("[%s_%d]", __func__, __LINE__);
	sGetInputVideoColorInfo(pVidPathParam, & vidPathCfg.inClrInfo );

	if ((bitDepth != SII_DRV_BIT_DEPTH__PASSTHOUGH) && (bitDepth <= pVidPathParam->inClrInfo.vidDcDepth)) {
		vidPathCfg.bEnable_Dithering = 1;
		if (vidPathCfg.inClrInfo.clrSpc == YCbCr420) {
			vidPathCfg.bEnable_IRC = 1;
		} else {
			vidPathCfg.bEnable_IRC = 0;
		}

		pVidPathParam->outClrInfo.vidDcDepth = bitDepth;
		vidPathCfg.outClrInfo = pVidPathParam->outClrInfo;

		if (vidPathCfg.outClrInfo.clrSpc == YCbCr420) {
			vidPathCfg.bEnable_ORC = 1;
		} else {
			vidPathCfg.bEnable_ORC = 0;
		}

		pVidPathParam->bDithering_en = true;
		vidPathCfg.bEnable_CMS = 1;

		vidPathCfg.instTxCra = pVidPathParam->instTxCra;

		sOutputMute(pVidPathParam, true);
		sSetVideoPathdefaults(pVidPathParam);
		sVideoPathCoreConfig(pVidPathParam, &vidPathCfg);
		sOutputMute(pVidPathParam, false);
		#if SI_DRV_DINO_CFG
		outClrConvStd = (uint8_t)vidPathCfg.outClrInfo.convStd;
		outClrDc = (uint8_t)vidPathCfg.outClrInfo.vidDcDepth;
		outClrSpc = (uint8_t)vidPathCfg.outClrInfo.clrSpc;
		isColorConv_en = true;
		isOutBItDepthChng = true;
		#endif
	} else {
		//need to recover when higher bit depth is set.
		pVidPathParam->outClrInfo.vidDcDepth = bitDepth;
		pVidPathParam->bDithering_en = true;
	}

}

void sVideoPathColorInfoConfig(VideoPathParam_t *pVidPathParam, SiiDrvTxColorInfoCfg_t *pClrInfo)
{
	VideoPathConfig_t      vidPathCfg;

	SII_MEMSET(&vidPathCfg, 0, sizeof(VideoPathConfig_t));

	sDecodeColorInfo(pVidPathParam, pClrInfo->inputClrSpc, & pVidPathParam->inClrInfo);
	pVidPathParam->inClrInfo.vidDcDepth = pClrInfo->inputVidDcDepth;
	//pVidPathParam->inClrInfo.clrSpc = pClrInfo->inputClrSpc;
	pVidPathParam->inClrInfo.convStd = pClrInfo->inputClrConvStd;

	SI_HDMI20_PRINT("[%s_%d]", __func__, __LINE__);
	sGetInputVideoColorInfo(pVidPathParam, &vidPathCfg.inClrInfo);
	if ( vidPathCfg.inClrInfo.quntization_mt != QUANTIZATION_BUTT) {
		vidPathCfg.inClrInfo.quntization = (int)vidPathCfg.inClrInfo.quntization_mt;
	}

	if (vidPathCfg.inClrInfo.clrSpc == YCbCr420) {
		vidPathCfg.bEnable_IRC = 1;
	} else {
		vidPathCfg.bEnable_IRC = 0;
	}

	if (pVidPathParam->bColorConv_en || pVidPathParam->bDithering_en) {
		vidPathCfg.bEnable_CMS = 1;
		vidPathCfg.outClrInfo = pVidPathParam->outClrInfo;
	} else {
		vidPathCfg.outClrInfo = vidPathCfg.inClrInfo;
		pVidPathParam->outClrInfo = vidPathCfg.outClrInfo;
	}

	vidPathCfg.bEnable_Dithering = pVidPathParam->bDithering_en;
	// below change is done considering the fact that for input colour depth has to be followed in following cases:
	// 1. when dithering is disabled
	// 2. Disthering is enabled and input bit depth is less than required dithering output.
	// In other cases colour depth has to be recovered to the desired colour depth
	/*if(pVidPathParam->bDithering_en && (vidPathCfg.inClrInfo.vidDcDepth >= pVidPathParam->outClrInfo.vidDcDepth))
	{
	vidPathCfg.bEnable_Dithering = 1;
	}
	else if(pVidPathParam->bDithering_en && (vidPathCfg.inClrInfo.vidDcDepth < pVidPathParam->outClrInfo.vidDcDepth))
	{
	vidPathCfg.outClrInfo.vidDcDepth = vidPathCfg.inClrInfo.vidDcDepth;
	}*/
	// else
	if (!(pVidPathParam->bDithering_en && (vidPathCfg.inClrInfo.vidDcDepth >= pVidPathParam->outClrInfo.vidDcDepth))) {
		vidPathCfg.outClrInfo.vidDcDepth = vidPathCfg.inClrInfo.vidDcDepth;
	}

	if (vidPathCfg.outClrInfo.clrSpc == YCbCr420) {
		vidPathCfg.bEnable_ORC = 1;
	} else {
		vidPathCfg.bEnable_ORC = 0;
	}
	// CMS is always enabled to make any incoming input videodepth to 12 bit.
	// For this if we wan to let the video in pas trough mode, dithering has to be enabled to match the input color depth.
	vidPathCfg.bEnable_CMS = 1;

	vidPathCfg.instTxCra = pVidPathParam->instTxCra;

	sOutputMute(pVidPathParam, true);
	sSetVideoPathdefaults(pVidPathParam );
	sVideoPathCoreConfig( pVidPathParam, &vidPathCfg);
	sOutputMute(pVidPathParam, false);

	#if SI_DRV_DINO_CFG
	outClrConvStd = (uint8_t)vidPathCfg.outClrInfo.convStd;
	#if SI_DRV_DINO_CFG
	if (pVidPathParam->bColorConv_en || (outClrSpc != vidPathCfg.outClrInfo.clrSpc)) {
		isColorConv_en = true;
	}
	#endif
	outClrSpc = (uint8_t)vidPathCfg.outClrInfo.clrSpc;

	if (outClrDc != vidPathCfg.outClrInfo.vidDcDepth) {
		outClrDc = (uint8_t)vidPathCfg.outClrInfo.vidDcDepth;
		isOutBItDepthChng = true;
	}
	#endif
}

//-----------------------------------------------------------------------------
void sVidPathHvSyncPolaritySet(VideoPathParam_t *pVidPathParam, SiiHvSyncPol_t hvSyncPol)
{
	VideoPathConfig_t      vidPathCfg;

	SII_MEMSET(&vidPathCfg, 0, sizeof(VideoPathConfig_t));
	SII_LIB_LOG_PRINT1(pVidPathParam, ("Setting HV_SYNC Polarity to: %02X\n", hvSyncPol));
	vidPathCfg.outputHVSyncPol = hvSyncPol;
	vidPathCfg.bEnable_OutSPA = 1;
	vidPathCfg.instTxCra = pVidPathParam->instTxCra;
	sVideoPathCoreConfig(pVidPathParam, &vidPathCfg);
}

//-----------------------------------------------------------------------------
void sVidPathInputQuantSet(VideoPathParam_t *pVidPathParam, SiiQuantLevel_t quant)
{
	VideoPathConfig_t      vidPathCfg;

	SII_MEMSET(&vidPathCfg, 0, sizeof(VideoPathConfig_t));
	SII_LIB_LOG_PRINT1(pVidPathParam, ("Setting Input_Quant to: %d\n", quant));
	vidPathCfg.inClrInfo.quntization_mt = quant;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
static void sGetInputVideoColorInfo(VideoPathParam_t *pVidPathParam, VideoColorInfo_t* pVidClrInfo)
{
	//VideoPathParam_t *pVidPathParam = (VideoPathParam_t *) SII_LIB_OBJ_PNTR(inst);

	#if 0
	SII_LIB_LOG_DEBUG1(pVidPathParam, ("cs=%d,std=%d,dc=%d,q=%d\n", \
									   (uint32_t)pVidPathParam->inClrInfo.clrSpc, \
									   (uint32_t)pVidPathParam->inClrInfo.convStd, \

									   (uint32_t)pVidPathParam->inClrInfo.vidDcDepth, \
									   (uint32_t)pVidPathParam->inClrInfo.quntization));
	#endif
	pVidClrInfo->clrSpc = pVidPathParam->inClrInfo.clrSpc;
	pVidClrInfo->convStd = pVidPathParam->inClrInfo.convStd;
	pVidClrInfo->vidDcDepth = pVidPathParam->inClrInfo.vidDcDepth;
	pVidClrInfo->quntization = pVidPathParam->inClrInfo.quntization;
}

//-----------------------------------------------------------------------------
static void sSetVideoPathdefaults(VideoPathParam_t *pVidPathParam)
{
	uint16_t vp__input_format;
	uint16_t vp__output_format;
	uint8_t vp__cms__csc0__c420_c422_config;
	uint8_t vp__cms__csc0__c422_c444_config;
	uint8_t vp__cms__csc1__c444_c422_config;
	uint8_t vp__cms__csc1__c422_c420_config;
	uint16_t vp__cms__csc0__multi_csc_config;
	uint16_t vp__cms__csc1__multi_csc_config;
	uint8_t vp__cms__csc1__dither_config;

	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__INPUT_FORMAT, (uint8_t *) &vp__input_format, BIT_MASK__VP__INPUT_FORMAT__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_FORMAT, (uint8_t *) &vp__output_format, BIT_MASK__VP__OUTPUT_FORMAT__SIZE);

	vp__input_format &= ~BIT_MSK__VP__INPUT_FORMAT__MUX_420_ENABLE;
	vp__output_format &= ~BIT_MSK__VP__OUTPUT_FORMAT__DEMUX_420_ENABLE;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__INPUT_FORMAT, (uint8_t *) &vp__input_format, BIT_MASK__VP__INPUT_FORMAT__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_FORMAT, (uint8_t *) &vp__output_format, BIT_MASK__VP__OUTPUT_FORMAT__SIZE);

	//SELECT_VIDEO_PATH_PAGE(PAGE_CSC);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C420_C422_CONFIG, (uint8_t *) &vp__cms__csc0__c420_c422_config, BIT_MASK__VP__CMS_CSC0_420_422_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C422_C444_CONFIG, (uint8_t *) &vp__cms__csc0__c422_c444_config, BIT_MASK__VP__CMS_CSC0_422_444_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, (uint8_t *) &vp__cms__csc1__c444_c422_config, BIT_MASK__VP__CMS_CSC1_444_422_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, (uint8_t *) &vp__cms__csc1__c422_c420_config, BIT_MASK__VP__CMS_CSC1_422_420_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *) &vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, (uint8_t *) &vp__cms__csc1__multi_csc_config, BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__DITHER_CONFIG, (uint8_t *)&vp__cms__csc1__dither_config, BIT_MASK__VP__CMS__CSC1__DITHERING_CONFIG__SIZE);

	vp__cms__csc0__c420_c422_config |= BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__BYPASS;
	vp__cms__csc0__c420_c422_config &= ~BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__ENABLE;

	vp__cms__csc0__c422_c444_config &= ~BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__ENABLE;

	vp__cms__csc1__c444_c422_config &= ~BIT_MSK__VP__CMS__CSC1__C444_C422_CONFIG__ENABLE;

	vp__cms__csc1__c422_c420_config |= BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__BYPASS;
	vp__cms__csc1__c422_c420_config &= ~BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__ENABLE;

	//vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE;
	vp__cms__csc0__multi_csc_config |= BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__DISABLE_SATURATION;

	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__ENABLE;
	vp__cms__csc1__multi_csc_config |= BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__DISABLE_SATURATION;

	vp__cms__csc1__dither_config |= 0x03;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C420_C422_CONFIG, (uint8_t *) &vp__cms__csc0__c420_c422_config, BIT_MASK__VP__CMS_CSC0_420_422_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C422_C444_CONFIG, (uint8_t *) &vp__cms__csc0__c422_c444_config, BIT_MASK__VP__CMS_CSC0_422_444_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, (uint8_t *) &vp__cms__csc1__c444_c422_config, BIT_MASK__VP__CMS_CSC1_444_422_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, (uint8_t *) &vp__cms__csc1__c422_c420_config, BIT_MASK__VP__CMS_CSC1_422_420_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *) &vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, (uint8_t *) &vp__cms__csc1__multi_csc_config, BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__DITHER_CONFIG, (uint8_t *)&vp__cms__csc1__dither_config, BIT_MASK__VP__CMS__CSC1__DITHERING_CONFIG__SIZE);
}

static void sDecodeColorInfo(VideoPathParam_t *pVidPathParam, SiiDrvClrSpc_t clrSpc, VideoColorInfo_t* pClrInfo)
{
	VideoColorInfo_t inClrInfo;

	SI_HDMI20_PRINT("[%s_%d]", __func__, __LINE__);
	sGetInputVideoColorInfo(pVidPathParam, &inClrInfo);

	switch (clrSpc) {
		case SII_DRV_CLRSPC__PASSTHRU:
			pClrInfo->convStd = inClrInfo.convStd;
			pClrInfo->clrSpc = inClrInfo.clrSpc;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__RGB_FULL:
			pClrInfo->convStd = inClrInfo.convStd;
			pClrInfo->clrSpc = RGB;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__RGB_LIMITED:
			pClrInfo->convStd = inClrInfo.convStd;
			pClrInfo->clrSpc = RGB;
			pClrInfo->quntization = QUANTIZATION_PC_LEVELS;
			break;
		case SII_DRV_CLRSPC__XVYCC420_601:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_601;
			pClrInfo->clrSpc = YCbCr420;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__XVYCC420_709:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_709;
			pClrInfo->clrSpc = YCbCr420;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__XVYCC422_601:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_601;
			pClrInfo->clrSpc = YCbCr422;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__XVYCC422_709:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_709;
			pClrInfo->clrSpc = YCbCr422;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__XVYCC444_601:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_601;
			pClrInfo->clrSpc = YCbCr444;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__XVYCC444_709:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_709;
			pClrInfo->clrSpc = YCbCr444;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC420_2020:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
			pClrInfo->clrSpc = YCbCr420;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC420_601:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_601;
			pClrInfo->clrSpc = YCbCr420;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC420_709:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_709;
			pClrInfo->clrSpc = YCbCr420;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC422_2020:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
			pClrInfo->clrSpc = YCbCr422;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC422_601:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_601;
			pClrInfo->clrSpc = YCbCr422;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC422_709:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_709;
			pClrInfo->clrSpc = YCbCr422;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC444_2020:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_2020_CONST_LUMINOUS;
			pClrInfo->clrSpc = YCbCr444;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC444_601:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_601;
			pClrInfo->clrSpc = YCbCr444;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
		case SII_DRV_CLRSPC__YC444_709:
			pClrInfo->convStd = SII_DRV_CONV_STD__BT_709;
			pClrInfo->clrSpc = YCbCr444;
			pClrInfo->quntization = QUANTIZATION_VIDEO_LEVELS;
			break;
	}
}

//-----------------------------------------------------------------------------
static void sVideoPathCoreConfig(VideoPathParam_t *pVidPathParam, VideoPathConfig_t* pCfg)
{
	#if (MT_SDK_COMPILE_HDMI20 == 0)
	DataPathMuxCfg_t       dpmCfg;
	RateConvertCfg_t       rcCfg;
	SyncPolAdjustCfg_t     spaCfg;
	EmbeddeSyncCfg_t       esdCfg;
	DeGenCfg_t             degCfg;
	CmsCfg_t               cmsCfg;
	VideoBalnkCfg_t        vidBalnkCfg;
	EmbeddeSyncCfg_t       eseCfg;
	#else
	DataPathMuxCfg_t       dpmCfg = {0};
	RateConvertCfg_t       rcCfg = {0};
	SyncPolAdjustCfg_t     spaCfg = {0};
	EmbeddeSyncCfg_t       esdCfg = {0};
	DeGenCfg_t             degCfg = {0};
	CmsCfg_t               cmsCfg = {0};
	VideoBalnkCfg_t        vidBalnkCfg = {0};
	EmbeddeSyncCfg_t       eseCfg = {0};
	#endif

	if (pCfg->bEnable_IDPMux) {
		//set configuration params
		sInputDataPathMuxConfig(pVidPathParam, &dpmCfg);
	}
	if (pCfg->bEnable_IRC) {
		//set configuration params
		rcCfg.mux_420_enable = 1;
		sInputRateConverterConfig(pVidPathParam, &rcCfg);
	}
	if (pCfg->bEnable_InSPA) {
		spaCfg.bAutoAdjust = 0;
		sInSyncPolarityAdjustmentConfig(pVidPathParam, &spaCfg);
	}
	if (pCfg->bEnable_656D) {
		//set configuration params
		s656DecderConfig(pVidPathParam, &esdCfg);
	}
	if (pCfg->bEnable_DEG) {
		//set configuration params
		sDEGeneratorConfig(pVidPathParam, &degCfg);
	}
	if (pCfg->bEnable_CMS) {
		if (pCfg->inClrInfo.clrSpc == YCbCr420) {
			cmsCfg.bEnable_420422 = 1;
			cmsCfg.bEnable_422444 = 1;
		} else if (pCfg->inClrInfo.clrSpc == YCbCr422) {
			cmsCfg.bEnable_420422 = 0;
			cmsCfg.bEnable_422444 = 1;
		} else {
			cmsCfg.bEnable_420422 = 0;
			cmsCfg.bEnable_422444 = 0;
		}
		if ( pCfg->outClrInfo.clrSpc == RGB ) {
			cmsCfg.bEnable_csc0 = 1;
			cmsCfg.bIsGamaCorrEnable = 0;
			cmsCfg.bEnable_csc1 = 0;
		} else {
			cmsCfg.bEnable_csc0 = 0;
			cmsCfg.bIsGamaCorrEnable = 0;
			cmsCfg.bEnable_csc1 = 0;
		}
		if (pCfg->outClrInfo.clrSpc == YCbCr420) {
			cmsCfg.bEnable_444422 = 1;
			cmsCfg.bEnable_422420 = 1;
		} else if (pCfg->outClrInfo.clrSpc == YCbCr422) {
			cmsCfg.bEnable_444422 = 1;
			cmsCfg.bEnable_422420 = 0;
		} else {
			cmsCfg.bEnable_444422 = 0;
			cmsCfg.bEnable_422420 = 0;
		}

		cmsCfg.outClrInfo = pCfg->outClrInfo;
		cmsCfg.inClrInfo =  pCfg->inClrInfo;

		if ((pCfg->bEnable_Dithering) || (pCfg->outClrInfo.vidDcDepth != pCfg->inClrInfo.vidDcDepth)) {
			cmsCfg.bEnable_Dithering = pCfg->bEnable_Dithering;
		} else {
			cmsCfg.bEnable_Dithering = 0;
		}

		sVideoPathCmsConfig(pVidPathParam, &cmsCfg);
	}
	if (pCfg->bEnable_VB) {
		//set configuration params
		sVideoBlankingConfig(pVidPathParam, &vidBalnkCfg);
	}
	if (pCfg->bEnable_ORC) {
		//set configuration params
		rcCfg.mux_420_enable = 1;
		sOutputRateConverterConfig(pVidPathParam, &rcCfg);
	}
	if (pCfg->bEnable_ESE) {
		//set configuration params
		sEmbeddedSyncEncoderConfig(pVidPathParam, &eseCfg);
	}
	if (pCfg->bEnable_OutSPA) {
		spaCfg.bAutoAdjust = 0;
		switch (pCfg->outputHVSyncPol) {
			case SII_HV_SYNC_POL__HPVP:
				spaCfg.HSYNC_Pol = SYNCPOL_POSITIVE;
				spaCfg.VSYNC_Pol = SYNCPOL_POSITIVE;
				break;
			case SII_HV_SYNC_POL__HPVN:
				spaCfg.HSYNC_Pol = SYNCPOL_POSITIVE;
				spaCfg.VSYNC_Pol = SYNCPOL_NEGATIVE;
				break;
			case SII_HV_SYNC_POL__HNVP:
				spaCfg.HSYNC_Pol = SYNCPOL_NEGATIVE;
				spaCfg.VSYNC_Pol = SYNCPOL_POSITIVE;
				break;
			case SII_HV_SYNC_POL__HNVN:
				spaCfg.HSYNC_Pol = SYNCPOL_NEGATIVE;
				spaCfg.VSYNC_Pol = SYNCPOL_NEGATIVE;
				break;
		}
		sOutSyncPolarityAdjustmentConfig(pVidPathParam, &spaCfg);
	}
	if (pCfg->bEnable_ODPMux) {
		//set configuration params
		sOutnputDataPathMuxConfig(pVidPathParam, &dpmCfg);
	}
}

//-----------------------------------------------------------------------------
static void sVideoPathCmsConfig(VideoPathParam_t *pVidPathParam, CmsCfg_t* pCfg)
{
	#if (MT_SDK_COMPILE_HDMI20 == 0)
	SamplerCfg_t upSamplerCfg;
	SamplerCfg_t downSamplerCfg;
	GammaCorrectionConfig_t rCfg;
	CscConfig_t csc0Cfg;
	CscConfig_t csc1Cfg;
	DitherCfg_t dithCfg;
	#else
	SamplerCfg_t upSamplerCfg = {0};
	SamplerCfg_t downSamplerCfg = {0};
	GammaCorrectionConfig_t rCfg = {0};
	CscConfig_t csc0Cfg = {0};
	CscConfig_t csc1Cfg = {0};
	DitherCfg_t dithCfg = {0};
	#endif

	//SELECT_VIDEO_PATH_PAGE(PAGE_CMS);

	if (pCfg->bEnable_420422 || pCfg->bEnable_422444) {
		/**
		* Conversion from 420->422/422->444
		*/
		upSamplerCfg.inputClrSpc = pCfg->inClrInfo.clrSpc;
		upSamplerCfg.outputClrSpc = YCbCr444;
		sUpSamplerConfig(pVidPathParam, &upSamplerCfg);
	}

	if (pCfg->bEnable_csc0 && (pCfg->inClrInfo.clrSpc != RGB)) {
		if (!pCfg->bIsGamaCorrEnable || (pCfg->inClrInfo.clrSpc != RGB)) {
			/**
			* Conversion from YCbCr444->RGB
			*/
			csc0Cfg.inConvStd = pCfg->inClrInfo.convStd;
			csc0Cfg.inputClrSpc = pCfg->inClrInfo.clrSpc;
			csc0Cfg.outputClrSpc = RGB;
			csc0Cfg.inQuantization = pCfg->inClrInfo.quntization;

			if (pCfg->outClrInfo.clrSpc != RGB) {
				pCfg->bEnable_csc1 = 1;
			} else {
				pCfg->bEnable_csc1 = 0;
			}

			if (pCfg->bEnable_csc1) {
				csc0Cfg.outQuantization = pCfg->inClrInfo.quntization;
			}

			csc0Cfg.outConvStd = pCfg->bEnable_csc1 ? pCfg->inClrInfo.convStd : pCfg->outClrInfo.convStd;

			sVideoPathCsc0Config(pVidPathParam, &csc0Cfg);
		}
	}

	if (pCfg->bIsGamaCorrEnable) {
		/**
		* Gamma Correction
		*/
		sGammaCorrectionConfig(pVidPathParam, &rCfg);
	}

	if (pCfg->bEnable_csc1) {
		/**
		* Conversion from RGB->YCBCR444
		*/
		csc1Cfg.inConvStd = pCfg->inClrInfo.convStd;
		csc1Cfg.outConvStd = pCfg->outClrInfo.convStd;
		csc1Cfg.inputClrSpc = (pCfg->bEnable_csc0) ? RGB : pCfg->inClrInfo.clrSpc;
		csc1Cfg.outputClrSpc = (pCfg->outClrInfo.clrSpc == RGB) ? RGB : YCbCr444;
		csc1Cfg.inQuantization = pCfg->inClrInfo.quntization;
		csc1Cfg.outQuantization = pCfg->outClrInfo.quntization;

		sVideoPathCsc1Config(pVidPathParam, &csc1Cfg);
	}

	if (pCfg->bEnable_444422 || pCfg->bEnable_422420) {
		/**
		* Conversion from 444->422/422->420
		*/
		downSamplerCfg.inputClrSpc = YCbCr444;
		downSamplerCfg.outputClrSpc = pCfg->outClrInfo.clrSpc;
		sDownSamplerConfig(pVidPathParam, &downSamplerCfg);
	}

	if (pCfg->bEnable_Dithering) {
		/**
		*  Conversion from 12->10, 12->8, 10->8 bit
		*  Only the registers is CSC1 have effect in dithering
		*/
		switch (pCfg->inClrInfo.vidDcDepth) {
			case SII_DRV_BIT_DEPTH__12_BIT:
				switch (pCfg->outClrInfo.vidDcDepth) {
					case SII_DRV_BIT_DEPTH__10_BIT:
						dithCfg = DITHER__12_TO_10;
						break;
					case SII_DRV_BIT_DEPTH__8_BIT:
						dithCfg = DITHER__12_TO_8;
						break;
					default:
						dithCfg = DITHER__NOCHNG; // video path core does not support any other scenarios
						break;
				}
				break;
			case SII_DRV_BIT_DEPTH__10_BIT:
				//here only 8 bit conversion is allowed
				dithCfg = DITHER__10_TO_8;
				break;
			default:
				dithCfg = DITHER__NOCHNG; // upscaling is not available
				break;
		}
	} else {
		switch (pCfg->inClrInfo.vidDcDepth) {
			case SII_DRV_BIT_DEPTH__12_BIT:
				dithCfg = DITHER__NOCHNG; // video path core does not support any other scenarios
				break;
			case SII_DRV_BIT_DEPTH__10_BIT:
				dithCfg = DITHER__12_TO_10;
				break;
			case SII_DRV_BIT_DEPTH__8_BIT:
				dithCfg = DITHER__12_TO_8;
				break;
			default:
				dithCfg = DITHER__NOCHNG; // upscaling is not available
				break;
		}
	}
	sDitheringConfig(pVidPathParam, &dithCfg);
}

static void sUpSamplerConfig(VideoPathParam_t *pVidPathParam, SamplerCfg_t* pCfg)
{
	uint8_t   vp__cms__csc0__c420_c422_config;
	uint8_t   vp__cms__csc0__c422_c444_config;

	//SELECT_VIDEO_PATH_PAGE(PAGE_CSC);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C420_C422_CONFIG, (uint8_t *) &vp__cms__csc0__c420_c422_config, BIT_MASK__VP__CMS_CSC0_420_422_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C422_C444_CONFIG, (uint8_t *) &vp__cms__csc0__c422_c444_config, BIT_MASK__VP__CMS_CSC0_422_444_CONFIG__SIZE);

	if (pCfg->inputClrSpc == YCbCr420) {
		switch (pCfg->outputClrSpc) {
			case YCbCr444:
				vp__cms__csc0__c420_c422_config &= ~BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__BYPASS;
				vp__cms__csc0__c420_c422_config |= BIT_MSK__VP__CMS__CSC0__C420_C422_CONFIG__ENABLE;
				vp__cms__csc0__c422_c444_config |= BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__ENABLE;
				vp__cms__csc0__c422_c444_config |= BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__USE_CB_OR_CR;
				break;
			//fall through
			case YCbCr422:
				vp__cms__csc0__c422_c444_config |= BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__ENABLE;
				vp__cms__csc0__c422_c444_config |= BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__USE_CB_OR_CR;
				break;
			default:
				break;
		}
	}
	if (pCfg->inputClrSpc == YCbCr422) {
		switch (pCfg->outputClrSpc) {
			case YCbCr444:
				vp__cms__csc0__c422_c444_config |= BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__ENABLE;
				vp__cms__csc0__c422_c444_config |= BIT_MSK__VP__CMS__CSC0__C422_C444_CONFIG__USE_CB_OR_CR;
				break;
			default:
				break;
		}
	}

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C420_C422_CONFIG, (uint8_t *) &vp__cms__csc0__c420_c422_config, BIT_MASK__VP__CMS_CSC0_420_422_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__C422_C444_CONFIG, (uint8_t *) &vp__cms__csc0__c422_c444_config, BIT_MASK__VP__CMS_CSC0_422_444_CONFIG__SIZE);
}

static void sDownSamplerConfig(VideoPathParam_t *pVidPathParam, SamplerCfg_t* pCfg)
{
	uint8_t   vp__cms__csc1__c444_c422_config;
	uint8_t   vp__cms__csc1__c422_c420_config;
	//SELECT_VIDEO_PATH_PAGE(PAGE_CSC);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, (uint8_t *) &vp__cms__csc1__c444_c422_config, BIT_MASK__VP__CMS_CSC1_444_422_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, (uint8_t *) &vp__cms__csc1__c422_c420_config, BIT_MASK__VP__CMS_CSC1_422_420_CONFIG__SIZE);

	if (pCfg->inputClrSpc == YCbCr444) {
		switch (pCfg->outputClrSpc) {
			case YCbCr420:
				vp__cms__csc1__c422_c420_config |= BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__ENABLE;
				vp__cms__csc1__c422_c420_config &= ~BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__BYPASS;
				vp__cms__csc1__c444_c422_config |= BIT_MSK__VP__CMS__CSC1__C444_C422_CONFIG__ENABLE;
				//fall through
				break;
			case YCbCr422:
				vp__cms__csc1__c444_c422_config |= BIT_MSK__VP__CMS__CSC1__C444_C422_CONFIG__ENABLE;
				break;
			default:
				break;
		}
	}
	if (pCfg->inputClrSpc == YCbCr422) {
		switch (pCfg->outputClrSpc) {
			case YCbCr444:
				vp__cms__csc1__c422_c420_config |= BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__ENABLE;
				vp__cms__csc1__c422_c420_config &= ~BIT_MSK__VP__CMS__CSC1__C422_C420_CONFIG__BYPASS;
				break;
			default:
				break;
		}
	}

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C444_C422_CONFIG, (uint8_t *) &vp__cms__csc1__c444_c422_config, BIT_MASK__VP__CMS_CSC1_444_422_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__C422_C420_CONFIG, (uint8_t *) &vp__cms__csc1__c422_c420_config, BIT_MASK__VP__CMS_CSC1_422_420_CONFIG__SIZE);

}

//-----------------------------------------------------------------------------
static void sVideoPathCsc0Config(VideoPathParam_t *pVidPathParam, CscConfig_t* pCfg)
{
	uint16_t  vp__cms__csc0__multi_csc_config;

	//SELECT_VIDEO_PATH_PAGE(PAGE_CSC);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);

	SI_HDMI20_PRINT("\n[ztq][%s_%d]in[%d,%d,%d], out[%d,%d,%d] before set csc0=0x%x\n", __func__, __LINE__, (uint32_t)pCfg->inConvStd, (uint32_t)pCfg->inputClrSpc, (uint32_t)pCfg->inQuantization, \
					(uint32_t)pCfg->outConvStd, (uint32_t)pCfg->outputClrSpc, (uint32_t)pCfg->outQuantization, \
					(uint32_t)vp__cms__csc0__multi_csc_config);
	//-----------configure csc0----------------
	/**
	* In CSC0 we convert to RGB irrespective of input as gamma correction requires RGB input
	*/
	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_STD;
	vp__cms__csc0__multi_csc_config |= (pCfg->inConvStd << 6);
	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_RGB;
	vp__cms__csc0__multi_csc_config |= (pCfg->inputClrSpc == RGB) ? BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_RGB : 0;
	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_PC;
	if (pCfg->inQuantization == QUANTIZATION_PC_LEVELS) {
		vp__cms__csc0__multi_csc_config |= BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__IN_PC;
	}

	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_STD;
	vp__cms__csc0__multi_csc_config |= (pCfg->outConvStd << 2);
	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_RGB;
	vp__cms__csc0__multi_csc_config |= (pCfg->outputClrSpc == RGB) ? BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_RGB : 0;
	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_PC;
	if (pCfg->outQuantization == QUANTIZATION_PC_LEVELS) {
		vp__cms__csc0__multi_csc_config |= BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__OUT_PC;
	}

	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__DITHER_ENABLE;

	vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE;
	if ( pCfg->outputClrSpc == RGB ) {
		switch ( pCfg->outConvStd ) {
			case SII_DRV_CONV_STD__BT_2020_NON_CONST_LUMINOUS:
				vp__cms__csc0__multi_csc_config |= (BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE & 0x0002);
				break;
			case SII_DRV_CONV_STD__BT_709:
			case SII_DRV_CONV_STD__BT_601:
				vp__cms__csc0__multi_csc_config |= (BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE & 0x0001);
				break;
			default:
				vp__cms__csc0__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE;
				break;
		}
	}
	//vp__cms__csc0__multi_csc_config |= (BIT_MSK__VP__CMS__CSC0__MULTI_CSC_CONFIG__ENABLE & 0x0001);
	SI_HDMI20_PRINT("\n[ztq][%s_%d]in[%d,%d,%d], out[%d,%d,%d] after set csc0=0x%x\n", __func__, __LINE__, (uint32_t)pCfg->inConvStd, (uint32_t)pCfg->inputClrSpc, (uint32_t)pCfg->inQuantization, \
					(uint32_t)pCfg->outConvStd, (uint32_t)pCfg->outputClrSpc, (uint32_t)pCfg->outQuantization, \
					(uint32_t)vp__cms__csc0__multi_csc_config);
	//--------------------------------------------------------
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC0__MULTI_CSC_CONFIG, (uint8_t *)&vp__cms__csc0__multi_csc_config, BIT_MASK__VP__CMS_CSC0_MULTI_CSC_CONFIG__SIZE);
}

//-----------------------------------------------------------------------------
static void sVideoPathCsc1Config(VideoPathParam_t *pVidPathParam, CscConfig_t* pCfg)
{
	uint16_t   vp__cms__csc1__multi_csc_config;
	//SELECT_VIDEO_PATH_PAGE(PAGE_CSC);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, (uint8_t *) &vp__cms__csc1__multi_csc_config, BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE);
	SI_HDMI20_PRINT("\n[ztq][%s_%d]in[%d,%d,%d], out[%d,%d,%d] before set csc0=0x%x\n", __func__, __LINE__, (uint32_t)pCfg->inConvStd, (uint32_t)pCfg->inputClrSpc, (uint32_t)pCfg->inQuantization, \
					(uint32_t)pCfg->outConvStd, (uint32_t)pCfg->outputClrSpc, (uint32_t)pCfg->outQuantization, \
					(uint32_t)vp__cms__csc1__multi_csc_config);
	//-----------configure cs1----------------
	/**
	* Input to this block is RGB
	*/
	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_STD;
	vp__cms__csc1__multi_csc_config |= (pCfg->inConvStd << 6);
	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_RGB;
	vp__cms__csc1__multi_csc_config |= (pCfg->inputClrSpc == RGB) ? BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_RGB : 0;
	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_PC;
	if (pCfg->inQuantization == QUANTIZATION_PC_LEVELS) {
		vp__cms__csc1__multi_csc_config |= BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__IN_PC;
	}

	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_STD;
	vp__cms__csc1__multi_csc_config |= (pCfg->outConvStd << 2);
	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_RGB;
	vp__cms__csc1__multi_csc_config |= (pCfg->outputClrSpc == RGB) ? BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_RGB : 0;
	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_PC;
	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_PC;
	if (pCfg->outQuantization == QUANTIZATION_PC_LEVELS) {
		vp__cms__csc1__multi_csc_config |= BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__OUT_PC;
	}

	vp__cms__csc1__multi_csc_config &= ~BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__DITHER_ENABLE;

	//vp__cms__csc1__multi_csc_config |= (BIT_MSK__VP__CMS__CSC1__MULTI_CSC_CONFIG__ENABLE & 0x0001);

	SI_HDMI20_PRINT("\n[ztq][%s_%d]in[%d,%d,%d], out[%d,%d,%d] after set csc0=0x%x\n", __func__, __LINE__, (uint32_t)pCfg->inConvStd, (uint32_t)pCfg->inputClrSpc, (uint32_t)pCfg->inQuantization, \
					(uint32_t)pCfg->outConvStd, (uint32_t)pCfg->outputClrSpc, (uint32_t)pCfg->outQuantization, \
					(uint32_t)vp__cms__csc1__multi_csc_config);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__MULTI_CSC_CONFIG, (uint8_t *) &vp__cms__csc1__multi_csc_config, BIT_MASK__VP__CMS_CSC1_MULTI_CSC_CONFIG__SIZE);
}

//-----------------------------------------------------------------------------
static void sInputDataPathMuxConfig(VideoPathParam_t *pVidPathParam, DataPathMuxCfg_t* pinMap)
{
	//pVidPathParam = pVidPathParam;
	//pinMap = pinMap;
}

//-----------------------------------------------------------------------------
static void sInputRateConverterConfig(VideoPathParam_t *pVidPathParam, RateConvertCfg_t* pCfg)
{
	uint16_t vp_input_format;

	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__INPUT_FORMAT, (uint8_t *)&vp_input_format, BIT_MASK__VP__INPUT_FORMAT__SIZE);

	vp_input_format &= ~BIT_MSK__VP__INPUT_FORMAT__MUX_420_ENABLE;
	vp_input_format |= pCfg->mux_420_enable ? BIT_MSK__VP__INPUT_FORMAT__MUX_420_ENABLE : 0;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__INPUT_FORMAT, (uint8_t *) &vp_input_format, BIT_MASK__VP__INPUT_FORMAT__SIZE);
}

//-----------------------------------------------------------------------------
static void sInSyncPolarityAdjustmentConfig(VideoPathParam_t *pVidPathParam, SyncPolAdjustCfg_t *pCfg)
{
	uint8_t vp__input_sync_adjust_config;

	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__INPUT_SYNC_ADJUST_CONFIG, (uint8_t *)&vp__input_sync_adjust_config, BIT_MASK__VP__INPUT_SYNC_ADJUST_CONFIG__SIZE);

	vp__input_sync_adjust_config &= ~BIT_MSK__VP__INPUT_SYNC_ADJUST_CONFIG__AUTO_DISABLE;
	vp__input_sync_adjust_config |= pCfg->bAutoAdjust ? 0 : BIT_MSK__VP__INPUT_SYNC_ADJUST_CONFIG__AUTO_DISABLE;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__INPUT_SYNC_ADJUST_CONFIG, (uint8_t *)&vp__input_sync_adjust_config, BIT_MASK__VP__INPUT_SYNC_ADJUST_CONFIG__SIZE);
}

static void s656DecderConfig(VideoPathParam_t *pVidPathParam, EmbeddeSyncCfg_t* pCfg)
{
	//pVidPathParam = pVidPathParam;
	//pCfg = pCfg;
}

static void sDEGeneratorConfig(VideoPathParam_t *pVidPathParam, DeGenCfg_t* pCfg)
{
	#if (MT_SDK_COMPILE_HDMI20 == 0)
	uint8_t       vp__degen_config;
	uint16_t      vp__degen_pixel_delay;
	uint16_t      vp__degen_line_delay;
	#else
	uint8_t       vp__degen_config = 0;
	uint16_t      vp__degen_pixel_delay = 0;
	uint16_t      vp__degen_line_delay = 0;
	#endif

	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__DEGEN_CONFIG, (uint8_t *)&vp__degen_config, BIT_MASK__VP__DEGEN_CONFIG__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__DEGEN_PIXEL_DELAY, (uint8_t *)&vp__degen_pixel_delay, BIT_MASK__VP__DEGEN_PIXEL_DELAY__SIZE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__DEGEN_LINE_DELAY, (uint8_t *)&vp__degen_line_delay, BIT_MASK__VP__DEGEN_LINE_DELAY__SIZE);

	vp__degen_config &= ~BIT_MSK__VP__DEGEN_CONFIG__ENABLE;
	vp__degen_config |= pCfg->enable ? BIT_MSK__VP__DEGEN_CONFIG__ENABLE : 0;

	vp__degen_pixel_delay = pCfg->pixel_delay;
	vp__degen_line_delay = pCfg->line_delay;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__DEGEN_CONFIG, (uint8_t *)&vp__degen_config, BIT_MASK__VP__DEGEN_CONFIG__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__DEGEN_PIXEL_DELAY, (uint8_t *)&vp__degen_pixel_delay, BIT_MASK__VP__DEGEN_PIXEL_DELAY__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__DEGEN_LINE_DELAY, (uint8_t *)&vp__degen_line_delay, BIT_MASK__VP__DEGEN_LINE_DELAY__SIZE);
}

static void sGammaCorrectionConfig(VideoPathParam_t *pVidPathParam, GammaCorrectionConfig_t* pCfg)
{
	//pVidPathParam = pVidPathParam;
	//pCfg = pCfg;
}

static void sVideoBlankingConfig(VideoPathParam_t *pVidPathParam, VideoBalnkCfg_t* pCfg)
{
	//pVidPathParam = pVidPathParam;
	//pCfg = pCfg;
}

static void sOutputRateConverterConfig(VideoPathParam_t *pVidPathParam, RateConvertCfg_t* pCfg)
{
	uint16_t vp__output_format;

	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_FORMAT, (uint8_t *)&vp__output_format, BIT_MASK__VP__OUTPUT_FORMAT__SIZE);

	vp__output_format &= ~BIT_MSK__VP__OUTPUT_FORMAT__DEMUX_420_ENABLE;
	vp__output_format |= pCfg->mux_420_enable ? BIT_MSK__VP__OUTPUT_FORMAT__DEMUX_420_ENABLE : 0;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_FORMAT, (uint8_t *) &vp__output_format, BIT_MASK__VP__OUTPUT_FORMAT__SIZE);
}

static void sEmbeddedSyncEncoderConfig(VideoPathParam_t *pVidPathParam, EmbeddeSyncCfg_t* pCfg)
{
	//pVidPathParam = pVidPathParam;
	//pCfg = pCfg;
}

static void sOutSyncPolarityAdjustmentConfig(VideoPathParam_t *pVidPathParam, SyncPolAdjustCfg_t *pCfg)
{
	uint8_t vp__output_sync_config;
	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);

	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_SYNC_CONFIG, (uint8_t *)&vp__output_sync_config, BIT_MASK__VP__OUTPUT_SYNC_CONFIG__SIZE);
	vp__output_sync_config &= ~(BIT_MSK__VP__OUTPUT_SYNC_CONFIG__VSYNC_POLARITY | BIT_MSK__VP__OUTPUT_SYNC_CONFIG__HSYNC_POLARITY);
	vp__output_sync_config |= pCfg->VSYNC_Pol ? 0 : BIT_MSK__VP__OUTPUT_SYNC_CONFIG__VSYNC_POLARITY;
	vp__output_sync_config |= pCfg->HSYNC_Pol ? 0 : BIT_MSK__VP__OUTPUT_SYNC_CONFIG__HSYNC_POLARITY;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_SYNC_CONFIG, &vp__output_sync_config, BIT_MASK__VP__OUTPUT_SYNC_CONFIG__SIZE);
}

static void sOutnputDataPathMuxConfig(VideoPathParam_t *pVidPathParam, DataPathMuxCfg_t* pinMap)
{
	//pVidPathParam = pVidPathParam;
	//pinMap = pinMap;
}

static void sOutputMute(VideoPathParam_t *pVidPathParam, bool_t enMute)
{
	uint8_t vp__output_mute;

	return ;
	//SELECT_VIDEO_PATH_PAGE(PAGE_CORE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_MUTE, (uint8_t *)&vp__output_mute, BIT_MASK__VP__OUTPUT_MUTE__SIZE);

	//vp__output_mute = enMute? 0xFF:0x00;
	vp__output_mute = enMute ? 0xF0 : 0x00;

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__OUTPUT_MUTE, (uint8_t *)&vp__output_mute, BIT_MASK__VP__OUTPUT_MUTE__SIZE);
}

static void sDitheringConfig(VideoPathParam_t *pVidPathParam, DitherCfg_t* dithConf)
{
	uint8_t vp__cms__csc1__dither_config;

	return ; //Disable dither for redmine 23808/23809
	//SELECT_VIDEO_PATH_PAGE(PAGE_CSC);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__DITHER_CONFIG, (uint8_t *)&vp__cms__csc1__dither_config, BIT_MASK__VP__CMS__CSC1__DITHERING_CONFIG__SIZE);

	vp__cms__csc1__dither_config &= ~BIT_MSK__VP__CMS__CSC1__DITHER_CONFIG__MODE;

	switch (*dithConf) {
		case DITHER__12_TO_10:
			vp__cms__csc1__dither_config |= 0x00;
			break;
		case DITHER__12_TO_8:
			vp__cms__csc1__dither_config |= 0x01;
			break;
		case DITHER__10_TO_8:
			vp__cms__csc1__dither_config |= 0x02;
			break;
		case DITHER__NOCHNG:
			vp__cms__csc1__dither_config |= 0x03;
			break;
	}

	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__CMS__CSC1__DITHER_CONFIG, (uint8_t *)&vp__cms__csc1__dither_config, BIT_MASK__VP__CMS__CSC1__DITHERING_CONFIG__SIZE);
}

static void sUpdateHVSyncPol(VideoPathParam_t *pVidPathParam)
{
	uint8_t vp__fdet_status;

	//SELECT_VIDEO_PATH_PAGE(pVidPathParam->pConfig->instCra, PAGE_CORE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__FDET_STATUS, (uint8_t *)&vp__fdet_status, BIT_MASK__VP__FDET_STATUS__SIZE);

	if (vp__fdet_status & BIT_MSK__VP__FDET_STATUS__HSYNC_POLARITY) {
		pVidPathParam->hvSyncPol = (vp__fdet_status & BIT_MSK__VP__FDET_STATUS__VSYNC_POLARITY) ? SII_HV_SYNC_POL__HPVP : SII_HV_SYNC_POL__HPVN;
	} else {
		pVidPathParam->hvSyncPol = (vp__fdet_status & BIT_MSK__VP__FDET_STATUS__VSYNC_POLARITY) ? SII_HV_SYNC_POL__HNVP : SII_HV_SYNC_POL__HNVN;
	}
}

static void SiiModVidpathSet_0(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *inData)
{
	VideoPathParam_t*      pVidPathParam = (VideoPathParam_t*)SII_LIB_OBJ_PNTR(inst);
	switch (opcode) {
		case SII_MOD_TX_VIDPATH_OPCODE__OUTPUT_COLORSPACE: {
			SiiDrvClrSpc_t clrSpc = *(SiiDrvClrSpc_t *)inData;
			sVidPathOutputColorSpaceSet(pVidPathParam, clrSpc);
			break;
		}
		case SII_MOD_TX_VIDPATH_OPCODE__COLOR_INFO_CONFIG: {
			SiiDrvTxColorInfoCfg_t *infoCfg = (SiiDrvTxColorInfoCfg_t *)inData;
			sVideoPathColorInfoConfig(pVidPathParam, infoCfg);
			break;
		}
		default: {
			break;
		}
	}
}

static void SiiModVidpathSet_1(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *inData)
{
	VideoPathParam_t*      pVidPathParam = (VideoPathParam_t*)SII_LIB_OBJ_PNTR(inst);
	switch (opcode) {
		case SII_MOD_TX_VIDPATH_OPCODE__BIT_DEPTH: {
			SiiDrvBitDepth_t bitDepth = *(SiiDrvBitDepth_t *)inData;
			sVidPathOutputBitDepthSet(pVidPathParam, bitDepth);
			break;
		}
		case SII_MOD_TX_VIDPATH_OPCODE__HV_SYNC_POLARITY: {
			SiiHvSyncPol_t hvSyncPol = *(SiiHvSyncPol_t *)inData;
			sVidPathHvSyncPolaritySet(pVidPathParam, hvSyncPol);
			break;
		}
		case SII_MOD_TX_VIDPATH_OPCODE__QUANTIZATION: {
			SiiQuantLevel_t quant = *(SiiQuantLevel_t *)inData;
			sVidPathInputQuantSet(pVidPathParam, quant);
			break;
		}
		default: {
			break;
		}
	}
}

static void SiiModVidpathSet_2(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *inData)
{
	VideoPathParam_t*      pVidPathParam = (VideoPathParam_t*)SII_LIB_OBJ_PNTR(inst);
	switch (opcode) {
		case SII_MOD_TX_VIDPATH_OPCODE__COLORIMETRY: {
			pVidPathParam->outClrInfo.convStd = *((SiiDrvConvStd_t *)inData);
			SII_LIB_LOG_DEBUG1(pVidPathParam, ("outstd:%d\n",pVidPathParam->outClrInfo.convStd));
			break;
		}
		case SII_MOD_TX_VIDPATH_OPCODE__CSC0_MATRIX: {
			memcpy(pVidPathParam->cscMatrix[0],inData,15*sizeof(uint16_t));
			{
				uint8_t i;
				for (i=0;i<15;i++) {
					SII_LIB_LOG_DEBUG1(pVidPathParam, ("mtx[0][%d]:%4x\n",pVidPathParam->cscMatrix[0][i]));
				}
			}
			break;
		}
		default: {
			break;
		}
	}
}

bool_t SiiModVidpathSet(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *inData)
{
	SiiModVidpathSet_0(inst, opcode, inData);
	SiiModVidpathSet_1(inst, opcode, inData);
	SiiModVidpathSet_2(inst, opcode, inData);
	return true;
}

bool_t SiiModVidpathGet(SiiInst_t inst, SiiModTxVidPathOpcode_t opcode, void *outData)
{
	//inst = inst;
	//outData = outData;
	//opcode = opcode;
	return true;
}

void SiiModVidPathInterruptHandler(SiiInst_t inst)
{
	uint32_t vp__fdet_irq_stat;
	VideoPathParam_t *pVidPathParam = (VideoPathParam_t *)SII_LIB_OBJ_PNTR(inst);

	//pVidPathParam->eventsFalg = 0;

	//SELECT_VIDEO_PATH_PAGE(pVidPathParam->pConfig->instCra, PAGE_CORE);
	SiiModTxVideoPathRegRead(pVidPathParam->instTxCra, REG_ADDR__VP__FDET_IRQ_STATUS, (uint8_t *)&vp__fdet_irq_stat, BIT_MASK__VP__FDET_IRQ_STAT__SIZE);
	SiiModTxVideoPathRegWrite(pVidPathParam->instTxCra, REG_ADDR__VP__FDET_IRQ_STATUS, (uint8_t *)&vp__fdet_irq_stat, BIT_MASK__VP__FDET_IRQ_STAT__SIZE);

	if (vp__fdet_irq_stat & (BIT_MSK__VP__FDET_IRQ_STATUS__HSYNC_POLARITY | BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_POLARITY)) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("HSYNC/VSYNC changed"));
		sUpdateHVSyncPol(pVidPathParam);
		//pVidPathParam->eventsFlag |= SII_DRV_VIDPATH_EVENT__SYNC_CHANGE;
	}
	if (vp__fdet_irq_stat  & BIT_MSK__VP__FDET_IRQ_STATUS__FRAME_RATE) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("Frame Rate changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__HBACK_COUNT) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("HBACK Count changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__HFRONT_COUNT) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("HFRONT Count changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__HSYNC_HIGH_COUNT) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("HSYNC HIGH COUNT changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__HSYNC_LOW_COUNT) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("HSYNC LOW Count changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__INTERLACED) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("Interlaced changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__LINE_COUNT) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("LINE Count changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__PIXEL_COUNT) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("PIXEL Count changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VBACK_COUNT_EVEN) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VBACK Count EVEN changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VBACK_COUNT_ODD) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VFRONT Count ODD changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VFRONT_COUNT_EVEN) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VFRONT Count EVEN changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VFRONT_COUNT_ODD) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VFRONT Count ODD changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VIDEO656) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VIDEO656 changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_HIGH_COUNT_EVEN) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VSYNC HIGH Count EVEN changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_HIGH_COUNT_ODD) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VSYNC HIGH Count ODD changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_LOW_COUNT_EVEN) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VSYNC LOW Count EVEN changed"));
	}
	if (vp__fdet_irq_stat & BIT_MSK__VP__FDET_IRQ_STATUS__VSYNC_LOW_COUNT_ODD) {
		SII_LIB_LOG_DEBUG1(pVidPathParam, ("VSYNC LOW Count ODD changed"));
	}

	//if( pVidPathParam->appNotifycbFunc && pVidPathParam->eventsFalg)
	{
		//pVidPathParam->appNotifycbFunc(pVidPathParam->eventsFalg);
	}
}
