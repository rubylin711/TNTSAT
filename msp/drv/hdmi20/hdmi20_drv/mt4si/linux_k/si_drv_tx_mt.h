/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

SiiInst_t SiiDrvTxCreate_uboot2main(char *pNameStr, SiiDrvTxConfig_t *pConfig)
{
	TxObj_t *pObj = NULL;
	SiiDrvCraAddr_t         baseAddr = 0;
	SiiInst_t				instCra;

	/* Allocate memory for object */
	pObj = (TxObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(TxObj_t));
	SII_PLATFORM_DEBUG_ASSERT(pObj);

	pObj->pConfig = (SiiDrvTxConfig_t*)SiiLibMallocCreate(sizeof(SiiDrvTxConfig_t));
	if ( pObj->pConfig ) {
		memcpy(pObj->pConfig, pConfig, sizeof(SiiDrvTxConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiDrvTxCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	baseAddr = sBaseAddrGet(pObj);
	instCra  = sCraInstGet(pObj);

	//Initialize
	pObj->hdcpStatus		= SII_DRV_HDCP_STATUS__OFF;
	pObj->isHdmiConnected	= false;

	// Create a timer to capture group interrupts
	pObj->instIntrHandler = SII_LIB_SEQ_TIMER_CREATE("Tx Interrupt Handler", sTxGroupInterruptHandler, SII_LIB_OBJ_INST(pObj), TIMER_START__TX_INTR_PRI);
	SII_PLATFORM_DEBUG_ASSERT(pObj->instIntrHandler);
	SiiLibSeqTimerStart(pObj->instIntrHandler, TIMER_START__TX_INTR, TIMER_START__TX_INTR_REPEAT);

	//Tx Soft Reset
	//SiiDrvCraSetBit8(instCra, baseAddr | REG_ADDR__PWD_SRST, BIT_MSK__PWD_SRST__REG_SW_RST);

	//Creating HDMI Tx
	pObj->hdmiConfig.baseAddrTx			= baseAddr;
	pObj->hdmiConfig.instTxCra				= instCra;
	pObj->hdmiConfig.instTx				= SII_LIB_OBJ_INST(pObj);
	pObj->hdmiConfig.cbFunc	= SiiDrvTxHdmiModCallBack;
	pObj->hdmiConfig.bScdcEn = pObj->pConfig->bScdcEn;
	pObj->hdmiConfig.bHdcpEn = pObj->pConfig->bHdcpEn;
	pObj->isHpdForced = false;
	pObj->instHdmiTx = SiiModTxHdmiCreate_uboot2main("tx_hdmi", &pObj->hdmiConfig);
	SII_PLATFORM_DEBUG_ASSERT(pObj->instHdmiTx);
	pConfig->scdcInst = pObj->hdmiConfig.scdcInst;

	// create HDCP object
	pObj->hdcpConfig.baseAddrTx	= baseAddr;
	pObj->hdcpConfig.bHdcpRepeat	= false;
	pObj->hdcpConfig.bHdcp2xEn		= pObj->pConfig->bHdcp2xEn;
	pObj->hdcpConfig.maxDsDev		= 127;
	pObj->hdcpConfig.instTxCra		= instCra;
	pObj->hdcpConfig.instTx		= SII_LIB_OBJ_INST(pObj);
	pObj->hdcpConfig.cbFunc		= SiiDrvTxHdcpModCallBack;
	pObj->instHdcp = SiiModTxHdcpCreate_uboot2main("tx_hdcp", &pObj->hdcpConfig);
	SII_PLATFORM_DEBUG_ASSERT(pObj->instHdcp);

	if (pObj->pConfig->bVidPathEn) {
		//Creating VideoPath
		pObj->vidPathConfig.instTxCra	= instCra;
		pObj->vidPathConfig.instTx		= SII_LIB_OBJ_INST(pObj);
		pObj->instVideoPath	= SiiModTxVideoPathCreate_uboot2main("VideoPath", &pObj->vidPathConfig);
		SII_PLATFORM_DEBUG_ASSERT(pObj->instVideoPath);
	}

	//clear reset
	//SiiDrvCraClrBit8(instCra, baseAddr | REG_ADDR__PWD_SRST, BIT_MSK__PWD_SRST__REG_SW_RST);

	return SII_LIB_OBJ_INST(pObj);
}
/***** end of file ***********************************************************/
