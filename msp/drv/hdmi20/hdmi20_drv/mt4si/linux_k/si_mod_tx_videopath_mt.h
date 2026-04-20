/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

SiiInst_t SiiModTxVideoPathCreate_uboot2main(char *pNameStr, SiiModTxVideoPathCfg_t *pConfig)
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
