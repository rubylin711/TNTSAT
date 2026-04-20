/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

SiiInst_t SiiModTxHdcpCreate_uboot2main(char *pNameStr, SiiModTxHdcpConfig_t *pConfig)
{
	HdcpObj_t*	hdcpObj = NULL;
	SiiDrvCraAddr_t	baseAddr;
	SiiInst_t		craInst;
	uint8_t hdcp2x_loaded = 0;

	/* Allocate memory for object */
	hdcpObj = (HdcpObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(HdcpObj_t));
	SII_PLATFORM_DEBUG_ASSERT(hdcpObj);

	hdcpObj->pConfig = (SiiModTxHdcpConfig_t*)SiiLibMallocCreate(sizeof(SiiModTxHdcpConfig_t));
	if ( hdcpObj->pConfig ) {
		SII_MEMCPY(hdcpObj->pConfig, pConfig, sizeof(SiiModTxHdcpConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiModTxHdcpCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	//Register Callback Function
	hdcpObj->cbFunc = hdcpObj->pConfig->cbFunc;

	baseAddr = sbaseAddrGet(hdcpObj);
	craInst	 = sCraInstGet(hdcpObj);

	/*--------------------------------*/
	/* Initialize user request states */
	/*--------------------------------*/
	hdcpObj->isAuthRequested	= false;
	hdcpObj->bBksvListApproved	= false;
	hdcpObj->isTxHpdAsserted	= false;
	hdcpObj->bAvMute			= false;
	hdcpObj->isDsHdcp_2_2_Cap_Read = false;
	hdcpObj->hdcp2x_seq_num_m = 0;
	hdcpObj->hdcpContentType = SII_DRV_HDCP_CONTENT_TYPE__0;
	hdcpObj->hdcp2x_repeater_ready = false;
	hdcpObj->hdcpStatus = SII_DRV_HDCP_STATUS__OFF;
	if ( pConfig->maxDsDev ) {
		hdcpObj->ksvList.pListStart = hdcpObj->ksvList.pList = (uint8_t*)SiiLibMallocCreate(pConfig->maxDsDev * sizeof(SiiDrvHdcpKsv_t));
		hdcpObj->ksvList.length  = 0;
		SII_PLATFORM_DEBUG_ASSERT(hdcpObj->ksvList.pList);
	}

	/*--------------------------------*/
	/* Initialize user status         */
	/*--------------------------------*/
	hdcpObj->authState				= SII_MOD_TX_HDCP_EVENT__OFF;
	hdcpObj->dsHdcp_2_2_Supported	= 0;
	hdcpObj->dsHdcp_1_x_Supported	= 0;
	hdcpObj->hdcp2xIntStat.reg0		= 0;
	hdcpObj->hdcp2xIntStat.reg1		= 0;
	hdcpObj->hdcp1xIntStat.reg0		= 0;
	hdcpObj->hdcp1xIntStat.reg1		= 0;
	hdcpObj->EcmIntStat.reg0		= 0;
	hdcpObj->EcmIntStat.reg1		= 0;
	hdcpObj->authFailCounter		= 0;

	/*-------Set HDCP Topology to default----------*/
	hdcpObj->hdcpTopology.depth				= 0;
	hdcpObj->hdcpTopology.deviceCount		= 0;
	hdcpObj->hdcpTopology.hdcp1xRepeaterDs	= 0;
	hdcpObj->hdcpTopology.hdcp20RepeaterDs	= 0;
	hdcpObj->hdcpTopology.maxCascadeExceeded = 0;
	hdcpObj->hdcpTopology.maxDevsExceeded	= 0;

	hdcp2x_loaded = SiiDrvCraRdReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_1) & BIT_MSK__HDCP2X_CTL_1__REG_HDCP2X_REAUTH_SW;

	if (hdcpObj->pConfig->bHdcp2xEn && hdcp2x_loaded == 0) {

		sHdcp2xCodeUpdatePatch(hdcpObj);//Add this according to IC's suggestion, or HDCP2x will NOT send AKE_init

		//HDCP2x Patch Update
		sHdcp2xCodeUpdate(hdcpObj);

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_CTL_1		, BIT_MSK__HDCP2X_CTL_1__REG_HDCP2X_REAUTH_SW);

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP0		, 0x02);  // HDCP2X TP0=2
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP1		, 0x8C);  // HDCP2X TP1=24, 20MHz->98, 24MHz->117 27MHz->140
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP2		, 0x01);  // HDCP2X TP2=1
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP3		, 0x32);  // HDCP2X TP3=50
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP4		, 0x1E);  // HDCP2X TP4=30
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP5		, 0x78);  // HDCP2X TP5=120
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP6		, 0x02);  // HDCP2X TP6=2
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP7		, 0x0A);  // HDCP2X TP7=10
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP8		, 0x0A);  // HDCP2X TP8=10 Cert read timeout
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP9		, 0x14);  // HDCP2X TP9=20
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP10		, 0x16);  // HDCP2X TP10=22
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP11		, 0xC8);  // HDCP2X TP11=200
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP12		, 0x96);  // HDCP2X TP12=150
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP13		, 0x10);  // HDCP2X TP13=16
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP14		, 0xC8);  // HDCP2X TP14=200
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_TP15		, 0x00);  // HDCP2X TP15=0

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN0		, 0x00);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN1		, 0x22);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN2		, 0x80);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_GP_IN3		, 0x00);  //
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_POLL_VAL0	, 0x05); // DDC polling interval, default 0x05
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_POLL_VAL1	, 0xe4); // DDC polling interval, default 0x32

		/* Apply AES reset when authdone=0 */
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__HDCP2X_AESCTL, BIT_MSK__HDCP2X_AESCTL__RI_AES_RST_AUTHDONE);

		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR0		, 0xFF);	//Clearing Intr0
		SiiDrvCraWrReg8(craInst, baseAddr | REG_ADDR__HDCP2X_INTR1		, 0xFF);	// clearing Intr1

		//Disabling HDCP Encryption
		SiiDrvCraSetBit8(craInst, baseAddr | REG_ADDR__TPI_COPP_DATA2, BIT_MSK__TPI_COPP_DATA2__REG_INTR_ENCRYPTION);
	} else {
		// HDCP2x inited in uboot
		hdcpObj->hdcp2xCupdStat = SII_DRV_HDCP2X_CUPD_CHK__DONE;
		hdcpObj->authState	= SII_MOD_TX_HDCP_EVENT__AUTHENTICATED;
	}

	// Create a timer to update hdcp states
	hdcpObj->instTimerStatePoll = SII_LIB_SEQ_TIMER_CREATE("HDCP_State_Machine_Handler", sHdcpStateMachineHandler, SII_LIB_OBJ_INST(hdcpObj), 252);
	SII_PLATFORM_DEBUG_ASSERT(hdcpObj->instTimerStatePoll);

	return SII_LIB_OBJ_INST(hdcpObj);
}
