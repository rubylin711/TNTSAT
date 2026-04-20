/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

SiiInst_t SiiModTxHdmiCreate_uboot2main(char *pNameStr, SiiModTxHdmiConfig_t *pConfig)
{
	HdmiTxObj_t*	hdmiObj = NULL;

	/* Allocate memory for hdmi_tx object */
	hdmiObj = (HdmiTxObj_t*)SII_LIB_OBJ_CREATE(pNameStr, sizeof(HdmiTxObj_t));
	SII_PLATFORM_DEBUG_ASSERT(hdmiObj);

	hdmiObj->pConfig = (SiiModTxHdmiConfig_t*)SiiLibMallocCreate(sizeof(SiiModTxHdmiConfig_t));
	if ( hdmiObj->pConfig ) {
		memcpy(hdmiObj->pConfig, pConfig, sizeof(SiiModTxHdmiConfig_t));
	} else {
		SII_LIB_LOG_DEBUG2(("SiiModTxHdmiCreate Malloc cfg fail\n"));
		return (SiiInst_t)NULL;
	}

	hdmiObj->cbFunc = hdmiObj->pConfig->cbFunc;

	/*--------------------------------*/
	/* Initialize user request states */
	/*--------------------------------*/
	//hdmiObj->primLink        = pConfig->primLink;
	//hdmiObj->bDualLink       = false;
	hdmiObj->tmdsMode			= SII_TMDS_MODE__NONE;
	hdmiObj->prevTmdsMode		= SII_TMDS_MODE__NONE;
	hdmiObj->hvSyncPol			= SII_HV_SYNC_POL__HPVP;
	hdmiObj->eState				= SII_MOD_TX_HDMI_STATUS__TMDS_OFF;

	hdmiObj->bInitHotPlug		= false;  // set this to false, so it will check the h/w status

	hdmiObj->bIsHdcpOn			= hdmiObj->pConfig->bHdcpEn;//true;
	hdmiObj->bWasHdcpOn			= false;
	hdmiObj->bAvMute			= false;
	hdmiObj->bIfOnAvi			= true;
	hdmiObj->bIfOnAudio			= true;
	hdmiObj->bIfOnVs			= true;
	hdmiObj->bIfOnSpd			= false;
	hdmiObj->bIfOnGbd			= false;
	hdmiObj->bIfOnMpeg			= false;
	hdmiObj->bIfOnIsrc			= false;
	hdmiObj->bIfOnIsrc2			= false;
	hdmiObj->bIfOnAcp			= false;

	/*--------------------------------*/
	/* Initialize user status         */
	/*--------------------------------*/
	hdmiObj->bHotPlug	= false;
	hdmiObj->bRsen		= false;

	/*--------------------------------*/
	/* Initialize interrupts Status    */
	/*--------------------------------*/
	hdmiObj->intStat.reg0 = 0x00;
	hdmiObj->intStat.reg1 = 0x00;

	//Clear Infoframes
	sClearInfoFrame(SII_INFO_FRAME_ID__AVI,    &hdmiObj->ifAvi);
	sClearInfoFrame(SII_INFO_FRAME_ID__AUDIO,  &hdmiObj->ifAudio);
	sClearInfoFrame(SII_INFO_FRAME_ID__VS,     &hdmiObj->ifVs);
	sClearInfoFrame(SII_INFO_FRAME_ID__SPD,    &hdmiObj->ifSpd);
	sClearInfoFrame(SII_INFO_FRAME_ID__GBD,    &hdmiObj->ifGbd);
	sClearInfoFrame(SII_INFO_FRAME_ID__MPEG,   &hdmiObj->ifMpeg);
	sClearInfoFrame(SII_INFO_FRAME_ID__ISRC,   &hdmiObj->ifIsrc);
	sClearInfoFrame(SII_INFO_FRAME_ID__ISRC2,  &hdmiObj->ifIsrc2);
	sClearInfoFrame(SII_INFO_FRAME_ID__ACP,    &hdmiObj->ifAcp);
	sClearInfoFrame(SII_INFO_FRAME_ID__HDR,     &hdmiObj->ifHdr);

	//Set Audio Infoframes
	hdmiObj->ifAudio.b[0] = 0x84;
	hdmiObj->ifAudio.b[1] = 0x01;
	hdmiObj->ifAudio.b[2] = 0x0a;
	hdmiObj->ifAudio.b[3] = 0x70;
	hdmiObj->ifAudio.b[4] = 0x01;	// 2-ch
	hdmiObj->ifAudio.b[5] = 0x00;
	hdmiObj->ifAudio.b[6] = 0x00;
	hdmiObj->ifAudio.b[7] = 0x00;
	hdmiObj->ifAudio.b[8] = 0x00;
	hdmiObj->ifAudio.b[9] = 0x00;
	hdmiObj->ifAudio.b[10] = 0x00;
	hdmiObj->ifAudio.b[11] = 0x00;
	hdmiObj->ifAudio.b[12] = 0x00;
	hdmiObj->ifAudio.b[13] = 0x00;

	//Set Audio Channel Status
	hdmiObj->channelStatus.i2s_chst0 = 0x00;
	hdmiObj->channelStatus.i2s_chst1 = 0x00;
	hdmiObj->channelStatus.i2s_chst2 = 0x00;
	hdmiObj->channelStatus.i2s_chst3 = 0x02;
	hdmiObj->channelStatus.i2s_chst4 = 0x0b;
	hdmiObj->channelStatus.i2s_chst5 = 0x00;
	hdmiObj->channelStatus.i2s_chst6 = 0x00;

	//Set Audio Format
	//hdmiObj->audioFormat.spdif = true;
	hdmiObj->audioFormat.i2s = true;
	hdmiObj->audioFormat.layout1 = AUDIO_FORMAT__2CH;
	hdmiObj->audioFormat.audioFs = SII_AUDIO_FS__48KHZ;

	//Set TDM Audio
	hdmiObj->audioOverTdm = false;

	// Create a timer to update Tx states
	hdmiObj->timerHwUpdate = SII_LIB_SEQ_TIMER_CREATE("Hardware_Update", sTxHwUpdateHandler, SII_LIB_OBJ_INST(hdmiObj), TIMER_START__HW_UPDATE_PRI);
	SII_PLATFORM_DEBUG_ASSERT(hdmiObj->timerHwUpdate);

	//Creating SCDC
	if (pConfig->bScdcEn) {
		sScdcConfig.instTxCra = pConfig->instTxCra;
		sScdcConfig.baseAddr = pConfig->baseAddrTx;
		sScdcConfig.cbFunc = sTxHdmiScdcCallBack;
		hdmiObj->scdcInst = SiiModTxScdcCreate("tx_scdc", SII_LIB_OBJ_INST(hdmiObj), &sScdcConfig);
		pConfig->scdcInst = hdmiObj->scdcInst;
		SII_PLATFORM_DEBUG_ASSERT(hdmiObj->scdcInst);

		hdmiObj->pConfig->scdcInst = hdmiObj->scdcInst;
		pConfig->scdcInst = hdmiObj->pConfig->scdcInst;
	}
	/*--------------------------------*/
	/* Static hardware configuration  */
	/*--------------------------------*/
	//Tx IP initialization .. Inited in uboot
	// reset AIP and AFIFO
	SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__AIP_RST, 0x00);

	//Enable Intr1 Interrupts.
	SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__INTR1_MASK, BIT_MSK__INTR1_MASK__REG_INTR1_MASK5 | BIT_MSK__INTR1_MASK__REG_INTR1_MASK6);
	//SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__INTR1_MASK, BIT_MSK__INTR1_MASK__REG_INTR1_MASK6);
	#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	//Enable EMP intr
	SiiDrvCraWrReg8((SiiInst_t)NULL, REG_ADDR__INTR_MASK, BIT_MSK_REG__EMP_SUCCESS_HDMI_MASK | \
					BIT_MSK_REG__EMP_ERR_HDMI_MASK | BIT_MSK_REG__EMP_ERR_CPU_MASK | \
					BIT_MSK_REG__DMA_DONE_MASK);
	#endif

	//Set Internal State to TMDS_OFF
	hdmiObj->iState = SII_MOD_TX_HDMI_EVENT__TMDS_OFF;

	return SII_LIB_OBJ_INST(hdmiObj);
}

extern MT_UNF_EDID_BASE_INFO_S *DRV_Get_SinkCap(MT_UNF_HDMI_ID_E enHdmi);
static void EdidCapInit(void)
{
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	//HDMI_PRIVATE_EDID_S *pPriSinkCap = DRV_Get_PriSinkCap(MT_UNF_HDMI_ID_0);

	{
	extern void DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_E enHdmi, MT_BOOL bSinkValid);
	DRV_Set_SinkCapValid(MT_UNF_HDMI_ID_0, MT_FALSE);
	}
	memset(pSinkCap, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
	//memset(pPriSinkCap, 0, sizeof(HDMI_PRIVATE_EDID_S));
	//init native fmt
	pSinkCap->enNativeFormat = MT_UNF_ENC_FMT_BUTT;
}

extern MT_UNF_EDID_BASE_INFO_S *DRV_Get_SinkCap(MT_UNF_HDMI_ID_E enHdmi);
static void EdidSetExtBlkNum(uint8_t extensions)
{
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	pSinkCap->u8ExtBlockNum = extensions;
}
