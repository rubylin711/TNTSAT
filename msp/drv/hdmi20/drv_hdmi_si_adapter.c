/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

static MT_U8 SI_Set_VSI_3D_FramePacking(void);
static MT_U8 SI_Set_VSI_3D_TopandBottom_Half(void);
static MT_U8 SI_Set_VSI_3D_SidebySide_Half(void);
static MT_U8 SI_Set_VSI_4K2K(MT_U32 u324KFormat);
MT_UNF_HDMI_DEEP_COLOR_E DRV_HDMI_Sidc2Mtdc(SiiDrvBitDepth_t dc);
MT_UNF_HDMI_VIDEO_MODE_E DRV_HDMI_Sicsc2Mtcsc(SiiDrvClrSpc_t csc);

#if defined (CEC_SUPPORT)
	u32 getCecDtsEnable(void);
	mt_void Si_CEC_Start(bool_t en);
	void SI_CEC_Enum( bool_t en, uint8_t hpd);
#endif

uint8_t  cecSourceLaList[] = { CEC_LOGADDR_TUNER1, CEC_LOGADDR_TUNER2, CEC_LOGADDR_TUNER3,
							   CEC_LOGADDR_TUNER4, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC,
							   CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC, CEC_LOGADDR_UNREGORBC
							 };

#if defined (CEC_SUPPORT)
mt_void Si_CEC_Start(bool_t en)
{
	sTxConfig.bCpiEn = en;
}
#endif

static void DRV_HDMI_Get_ParsedEDID(void)
{
	MT_UNF_EDID_BASE_INFO_S    *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	SiiLibEdidPar_t Edid = {0};
	SiiLibEdidPar_t *pEdid = &Edid;
	mt_u32 i, j;
	mt_u8 fs = 0;

	SiiDrvTxEdidParseGet(DRV_HDMI_Get_TxInst(), pEdid);

	pSinkCap->bSupportHdmi = (MT_BOOL)((pEdid->ieee_id == 0x000C03) || (pEdid->ieee_id == 0xC45DD8));
	//pSinkCap->enNativeFormat = (pEdid->ieee_id == 0x000C03) || (pEdid->ieee_id == 0xC45DD8);
	pSinkCap->st3DInfo.bSupport3D = (MT_BOOL)pEdid->d3Info.supported_3d;
	pSinkCap->st3DInfo.bSupport3DType[MT_UNF_EDID_3D_FRAME_PACKETING] = (MT_BOOL)pEdid->d3Info.supported_3d_frmpack;
	pSinkCap->st3DInfo.bSupport3DType[MT_UNF_EDID_3D_TOP_AND_BOTTOM] = (MT_BOOL)pEdid->d3Info.supported_3d_tb;
	pSinkCap->st3DInfo.bSupport3DType[MT_UNF_EDID_3D_SIDE_BY_SIDE_HALF] = (MT_BOOL)pEdid->d3Info.supported_3d_sbys;

	pSinkCap->stDeepColor.bDeepColor30Bit = (MT_BOOL)pEdid->b444DC10;
	pSinkCap->stDeepColor.bDeepColor36Bit = (MT_BOOL)pEdid->b444DC12;
	pSinkCap->stDeepColor.bDeepColor48Bit = (MT_BOOL)pEdid->b444DC16;
	pSinkCap->stDeepColor.bDeepColorY444 = (MT_BOOL)pEdid->bY444;
	pSinkCap->stDeepColor.bDeepColor30Bit_Y420 = (MT_BOOL)pEdid->scdc.bDc30bit420;
	pSinkCap->stDeepColor.bDeepColor36Bit_Y420 = (MT_BOOL)pEdid->scdc.bDc36bit420;
	pSinkCap->stDeepColor.bDeepColor48Bit_Y420 = (MT_BOOL)pEdid->scdc.bDc48bit420;

	pSinkCap->stColorMetry.bxvYCC601 = (MT_BOOL)pEdid->colorimetry.xvYCC601;
	pSinkCap->stColorMetry.bxvYCC709 = (MT_BOOL)pEdid->colorimetry.xvYCC709;
	pSinkCap->stColorMetry.bsYCC601 = (MT_BOOL)pEdid->colorimetry.sYCC601;
	pSinkCap->stColorMetry.bAdobleYCC601 = (MT_BOOL)pEdid->colorimetry.AdobeYCC601;
	pSinkCap->stColorMetry.bAdobleRGB = (MT_BOOL)pEdid->colorimetry.AdobeRGB;

	pSinkCap->stColorSpace.bRGB444 = (MT_BOOL)1;
	pSinkCap->stColorSpace.bYCbCr422 = (MT_BOOL)pEdid->Yuv422;
	pSinkCap->stColorSpace.bYCbCr444 = (MT_BOOL)pEdid->Yuv444;

	pSinkCap->u32AudioInfoNum = (mt_u32)(pEdid->audInfo.audformat_cnt);
	{
		mt_u32 speaker;
		speaker = (mt_u32)(pEdid->audInfo.speakerformat);
		memset(pSinkCap->bSupportAudioSpeaker, 0, MT_UNF_EDID_AUDIO_SPEAKER_BUTT * sizeof(MT_BOOL));
		for (i = 0; i < MT_UNF_EDID_AUDIO_SPEAKER_BUTT; i++) {
			pSinkCap->bSupportAudioSpeaker[i] = (speaker & 0x01);
			speaker >>= 1;
		}
	}
	for (i = 0; i < (pSinkCap->u32AudioInfoNum); i++) {
		pSinkCap->stAudioInfo[i].u8AudChannel = (mt_u8)pEdid->audInfo.audchannel[i];
		pSinkCap->stAudioInfo[i].enAudFmtCode = (MT_UNF_EDID_AUDIO_FORMAT_CODE_E)pEdid->audInfo.audformat[i];
		for ( j = 0; j < 8; j++) {
			fs = (pEdid->audInfo.audfs[i]) & ((uint8_t)(0x1 << j));
			pSinkCap->stAudioInfo[i].u32SupportSampleRateNum++;
			switch ( fs ) {
				case 0x1:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_32K;
					break;
				case 0x2:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_44K;
					break;
				case 0x4:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_48K;
					break;
				case 0x8:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_88K;
					break;
				case 0x10:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_96K;
					break;
				case 0x20:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_176K;
					break;
				case 0x40:
					pSinkCap->stAudioInfo[i].enSupportSampleRate[j] = MT_UNF_SAMPLE_RATE_192K;
					break;
				default:
					pSinkCap->stAudioInfo[i].u32SupportSampleRateNum--;
					break;
			}
		}
		if ( pEdid->audInfo.audformat[i] == 1 ) {
			j = 0;
			if ( pEdid->audInfo.audlen[i] & 0x1 ) {
				pSinkCap->stAudioInfo[i].bSupportBitDepth[j++] = MT_UNF_BIT_DEPTH_16;
			}
			if ( pEdid->audInfo.audlen[i] & 0x2 ) {
				pSinkCap->stAudioInfo[i].bSupportBitDepth[j++] = MT_UNF_BIT_DEPTH_20;
			}
			if ( pEdid->audInfo.audlen[i] & 0x4 ) {
				pSinkCap->stAudioInfo[i].bSupportBitDepth[j++] = MT_UNF_BIT_DEPTH_20;
			}
			pSinkCap->stAudioInfo[i].u32SupportBitDepthNum = (mt_u32)j;
		} else if ( pEdid->audInfo.audformat[i] > 1 && pEdid->audInfo.audformat[i] < 9 ) {
			pSinkCap->stAudioInfo[i].u32MaxBitRate = (mt_u32)(((mt_u32)pEdid->audInfo.audlen[i]) << 3);
		} else if ( pEdid->audInfo.audformat[i] == 12 ) {
			pSinkCap->stAudioInfo[i].u32MaxBitRate = (mt_u32)pEdid->audInfo.audlen[i];
		}
	}
	pSinkCap->u8Revision = (mt_u8)(pEdid->version & 0xff);
	pSinkCap->u8Version = (mt_u8)(((pEdid->version) >> 8) & 0xff);
	memcpy(pSinkCap->stMfrsInfo.u8MfrsName, pEdid->maninfo.man_name, 4);
	pSinkCap->stMfrsInfo.u32ProductCode = (mt_u32)pEdid->maninfo.product_code;
	pSinkCap->stMfrsInfo.u32SerialNumber = (mt_u32)pEdid->maninfo.serialnum;
	pSinkCap->stMfrsInfo.u32Week = (mt_u32)pEdid->maninfo.week;
	pSinkCap->stMfrsInfo.u32Year = (mt_u32)pEdid->maninfo.year;
	pSinkCap->stCECAddr.u8PhyAddrA = (mt_u8)(pEdid->cecAddr.sub[0]);
	pSinkCap->stCECAddr.u8PhyAddrB = (mt_u8)(pEdid->cecAddr.sub[1]);
	pSinkCap->stCECAddr.u8PhyAddrC = (mt_u8)(pEdid->cecAddr.sub[2]);
	pSinkCap->stCECAddr.u8PhyAddrD = (mt_u8)(pEdid->cecAddr.sub[3]);
	if ((pSinkCap->stCECAddr.u8PhyAddrA != 0xF ) && (pSinkCap->stCECAddr.u8PhyAddrB != 0xF ) &&
			(pSinkCap->stCECAddr.u8PhyAddrC != 0xF ) && (pSinkCap->stCECAddr.u8PhyAddrD != 0xF )) {
		pSinkCap->stCECAddr.bPhyAddrValid = TRUE;
	} else {
		pSinkCap->stCECAddr.bPhyAddrValid = FALSE;
	}
	for (i = 0; i < SII_LIB_EDID__DMDB_MAX; i++) {
		pSinkCap->u16SupportEMP |= (mt_u16)pEdid->DMDBInfo[i].DMType;
		if ( pEdid->DMDBInfo[i].DMType == 0 ) {
			break;
		}
	}
	pSinkCap->bSupportScdc = pEdid->scdc.bScdcPresent;
	pSinkCap->bSupportLTE = pEdid->scdc.bLTE340MscsScramble;
	pSinkCap->bSupportDdMat48k = pEdid->DdVsadb.SinkCap;
}

static void sUpdateEdid(void)
{
	SiiEdid_t   edid;

	SiiDrvTxEdidGet(DRV_HDMI_Get_TxInst(), &edid);
	DRV_HDMI_Get_ParsedEDID();

	//    SiiDrvRxEdidSet(sInstRx, &edid);
}

#if defined (CEC_SUPPORT)
void SI_CEC_Enum( bool_t en, uint8_t hpd)
{
	static uint8_t hpd_sts = 0;
	if ( en ) {
		SI_CEC_Open();
		SiiCecUpdatePhysicalAdress(SiiDrvTxCecPhysicalAddrGet(DRV_HDMI_Get_TxInst()));
		//mt_SiiCecSetSourceActive( true );
		SiiCecUpdatePowerState((SiiCecPowerstatus_t)CEC_POWERSTATUS_ON);
		if ( hpd_sts == 0 ) {
			SiiCecEnumerateDevices(0, en, cecSourceLaList);
			SiiCecEnumerateDeviceLa(cecSourceLaList );
			hpd_sts = 1;
		}
		SiiCecSetDevicePA( SiiDrvTxCecPhysicalAddrGet(DRV_HDMI_Get_TxInst()) );
		DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_TRUE);
	} else {
		//mt_SiiCecSetSourceActive( false );
		SiiCecUpdatePowerState((SiiCecPowerstatus_t)CEC_POWERSTATUS_STANDBY);
		if ( hpd == 0 ) {
			;
		}
		SiiCecEnumerateDevices(0, en, cecSourceLaList);
		hpd_sts = 0;
		SiiCecDisable();
		SiiCecReset();
	}
}
#endif

uint8_t GetSourceRealPowerSts(void)
{
	struct hdmiphy_param param;
	uint8_t outEn = 0;
	mt_analog_get_parameter(ANALOG_PARAM_INDEX_HDMIPHY,&param);
	outEn = (uint8_t)param.mute;
	HDMI20_DRV_HDMI_PRINTK("GetSourceRealPowerSts : %d-%d\n",outEn,SiiCecGetPowerState());
	return outEn;
}

static void sUpdateHotPlug(void)
{
	bool_t hotPlug = SiiDrvTxHotPlugStatusGet(DRV_HDMI_Get_TxInst());
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);

	if ( hotPlug ) {
		//set_tvsys_default();
		sUpdateEdid();
		#if defined (CEC_SUPPORT)
		//SI_CEC_Enum(hotPlug);
		if ( GetSourceRealPowerSts() == 0 ) {
			SiiCecUpdatePowerState((SiiCecPowerstatus_t)CEC_POWERSTATUS_ON);
		}
		#endif
		DRV_HDMI_NotifyEvent(MT_UNF_HDMI_EVENT_HOTPLUG);
	} else {
		#if defined (CEC_SUPPORT)
		SI_CEC_Enum(0, hotPlug);
		#endif
		memset(pSinkCap, 0, sizeof(MT_UNF_EDID_BASE_INFO_S));
		DRV_HDMI_NotifyEvent(MT_UNF_HDMI_EVENT_NO_PLUG);
	}
}

void SiiTxMtHpdPatchCallBack(uint8_t sts)
{
	if ( sts == 0 ) {
		HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
		//DRV_Set_CECEnable(MT_UNF_HDMI_ID_0, MT_FALSE);
		//DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
		pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
		memset(&(pstChnAttr[MT_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
		(MT_VOID)SI_CEC_Close();
	} else {
		if (MT_FALSE == DRV_Get_IsCECStart(MT_UNF_HDMI_ID_0)) 
		{//if ( DRV_Get_IsCECEnable(MT_UNF_HDMI_ID_0) )
			SI_CEC_SetUp();
			DRV_Set_CECEnable(MT_UNF_HDMI_ID_0, MT_TRUE);
			(MT_VOID)SI_CEC_Open();
#if defined (CEC_SUPPORT)
			SI_CEC_Enum(1, 1);
#endif
			DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_TRUE);
		}
	}
}

void SiiTxCallBack(SiiDrvTxEvent_t eventFlags)
{
	SiiDrvHdcpStatus_t 		hdcpStatus;
	SiiDrvHdcpKsvList_t 	bksvList;
	static SiiDrvHdcpStatus_t 		hdcpStatusLast = SII_DRV_HDCP_STATUS__OFF;
	//if( tpgEnable )
	//return;

	if (eventFlags & SII_DRV_TX_EVENT__HOT_PLUG_CHNG) {
		sUpdateHotPlug();
	}
	if (eventFlags & SII_DRV_TX_EVENT__RSEN_CHNG) {
		//SiiDrvRxRsenSet(sInstRx, SiiDrvTxRsenStatusGet(instTx));
	}
	if (eventFlags & SII_DRV_TX_EVENT__HDMI_STATE_CHNG) {
		;
	}
	if (eventFlags & SII_DRV_TX_EVENT__HDCP_STATE_CHNG) {
		SiiDrvTxHdcpStateStatusGet(DRV_HDMI_Get_TxInst(), &hdcpStatus);
		if ((hdcpStatus == SII_DRV_HDCP_STATUS__SUCCESS_1X) || (hdcpStatus == SII_DRV_HDCP_STATUS__SUCCESS_22)) {
			SiiDrvTxHdcpKsvListGet(DRV_HDMI_Get_TxInst(), &bksvList);
			// Compare with revocation list and approve/reject the KSV list.
			// Here approving by default
			SiiDrvTxHdcpKsvListApprovalSet(DRV_HDMI_Get_TxInst(), true);
			DRV_HDMI_NotifyEvent(MT_UNF_HDMI_EVENT_HDCP_SUCCESS);
			hdcpStatusLast = hdcpStatus;
		} else {
			SiiDrvTxHdcpKsvListApprovalSet(DRV_HDMI_Get_TxInst(), false);
			if (SII_DRV_HDCP_STATUS__FAILED == hdcpStatus && SII_DRV_HDCP_STATUS__FAILED != hdcpStatusLast) {
				DRV_HDMI_NotifyEvent(MT_UNF_HDMI_EVENT_HDCP_FAIL);
				hdcpStatusLast = hdcpStatus;
			}
		}
	}

	if (eventFlags & SII_DRV_TX_EVENT__CEC_CMD_RECEIVED) {
		SiiDrvCecInterruptHandler(SiiGetCecInst());
	}
}

static mt_u32 g_Event_Count[MAX_PROCESS_NUM];
#define MIX_EVENT_COUNT  1
void DRV_HDMI_CECNotifyEvent(SiiCecEvent_E event)
{
	MT_UNF_HDMI_EVENT_TYPE_E event_hdmi = MT_UNF_HDMI_EVENT_MAX;
	switch ( event ) {
		case SII_CEC_EVENT_TV_SEND_CEC_STANDBY:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_STANDBY;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_OK:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_OK;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_UP:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_UP;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_DOWN:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_DOWN;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_LEFT:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_LEFT;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_RIGHT:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_RIGHT;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_VOL_UP:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_VOL_UP;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_VOL_DOWN:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_VOL_DOWN;
			break;
		case SII_CEC_EVENT_TV_SEND_CEC_MUTE:
			event_hdmi = MT_UNF_HDMI_EVENT_TV_SEND_CEC_MUTE;
			break;
		case SII_CEC_EVENT_CEC_STATUS_INIT:
			event_hdmi = MT_UNF_HDMI_EVENT_CEC_STATUS_INIT;
			break;
		case SII_CEC_EVENT_CEC_STATUS_READY:
			event_hdmi = MT_UNF_HDMI_EVENT_CEC_STATUS_READY;
			break;
		case SII_CEC_EVENT_CEC_STATUS_POWER_ON:
			event_hdmi = MT_UNF_HDMI_EVENT_CEC_STATUS_POWER_ON;
			break;
		case SII_CEC_EVENT_CEC_STATUS_PORT_SWTICH_OUT:
			event_hdmi = MT_UNF_HDMI_EVENT_CEC_STATUS_PORT_SWTICH_OUT;
			break;
		default:
			event_hdmi = MT_UNF_HDMI_EVENT_MAX;
			break;
	}
	if ( MT_UNF_HDMI_EVENT_MAX != event_hdmi ) {
		DRV_HDMI_NotifyEvent(event_hdmi);
	}
}

mt_u32 SiiTxCreate(mt_void)
{
	SiiDrvHdcp2xCupdChkStat_t cupdChkStat;
	hdmi20_params_t *params = NULL;

	HDMI20_DRV_HDMI_PRINT_FUNC_ENTER();
	params = hdmi20_params_get();
	memset(&sTxConfig, 0, sizeof(SiiDrvTxConfig_t));
	sTxConfig.baseAddr	= 0;
	sTxConfig.instCra		= (SiiInst_t)NULL;
	sTxConfig.bHdcp2xEn 	= params->hdcp2x_en;   //HDCP2x enable?
	sTxConfig.bVidPathEn	= 1;
	#if defined(CEC_SUPPORT)
	sTxConfig.bCpiEn		= 1;     //0:cec disable, 1:cec enable
	#else
	sTxConfig.bCpiEn		= (bool_t)0;	  //0:cec disable, 1:cec enable
	#endif
	sTxConfig.bScdcEn		= params->scdc_en;
	sTxConfig.bHdcpEn		= params->hdcp_en;    //HDCP enable
	sInstTx_Drv = SiiDrvTxCreate("TX_IP", &sTxConfig);
	if ( params->hdcp_enhance_cfg ) {
		mt_init_keep_out_win(636, 240, 0, 1);
	}

	ASSERT_HDMI20(DRV_HDMI_Get_TxInst());

	SiiDrvTxRegisterCallBack(DRV_HDMI_Get_TxInst(), SiiTxCallBack);

	//Checking HDCP2x core update status
	if (sTxConfig.bHdcp2xEn) {
		SiiDrvTxHdcp2xCupdStatusGet(DRV_HDMI_Get_TxInst(), &cupdChkStat);
		switch (cupdChkStat) {
			case SII_DRV_HDCP2X_CUPD_CHK__ERROR:
				HDMI20_DRV_HDMI_PRINTK("HDCP2x Code Update Time Expired... Exit the aplication\n");
				HDMI20_DRV_HDMI_PRINTK("Press any Key to Exit\n");
				SiiDrvTxDelete(DRV_HDMI_Get_TxInst());
				return false;
			case SII_DRV_HDCP2X_CUPD_CHK__FAIL:
				HDMI20_DRV_HDMI_PRINTK("HDCP2x Code Update Failed... Exit the aplication\n");
				HDMI20_DRV_HDMI_PRINTK("Press any Key to Exit\n");
				SiiDrvTxDelete(DRV_HDMI_Get_TxInst());
				return false;
			case SII_DRV_HDCP2X_CUPD_CHK__DONE:
				HDMI20_DRV_HDMI_PRINTK("HDCP2x Code Update Successful\n");
				break;
		}
	}

	//Create CEC
	#if defined(CEC_SUPPORT)
	SiiCecCreate(sTxConfig.instCra);
	SiiCecRegisterEventNotifyCallBack(SiiGetCecInst(), DRV_HDMI_CECNotifyEvent);
	#endif
	HDMI20_DRV_HDMI_PRINT_FUNC_EXIT();
	if ( DRV_HDMI_Get_TxInst() ) {
		return MT_SUCCESS;
	} else {
		return MT_FAILURE;
	}
}

mt_u32 SiiTxCreate_uboot2app(mt_void)
{
	SiiDrvHdcp2xCupdChkStat_t cupdChkStat;
	hdmi20_params_t *params = NULL;
	HDMI_UBOOT_AVINFO_T avinfo = {0};

	HDMI20_DRV_HDMI_PRINT_FUNC_ENTER();
	params = hdmi20_params_get();
	SI_GetHdmiHalParams(&avinfo);
	memset(&sTxConfig, 0, sizeof(SiiDrvTxConfig_t));
	sTxConfig.baseAddr	= 0;
	sTxConfig.instCra		= (SiiInst_t)NULL;
	sTxConfig.bHdcp2xEn 	= params->hdcp2x_en;   //HDCP2x enable?
	sTxConfig.bVidPathEn	= 1;
	#if defined(CEC_SUPPORT)
	sTxConfig.bCpiEn		= 1;     //0:cec disable, 1:cec enable
	#else
	sTxConfig.bCpiEn		= (bool_t)0;	  //0:cec disable, 1:cec enable
	#endif
	sTxConfig.bScdcEn		= params->scdc_en;
	sTxConfig.bHdcpEn		= (avinfo.bHdcpEn || params->hdcp_en);    //HDCP enable
	sInstTx_Drv = SiiDrvTxCreate_uboot2main("TX_IP", &sTxConfig);
	if ( params->hdcp_enhance_cfg ) {
		mt_init_keep_out_win(636, 240, 0, 1);
	}

	ASSERT_HDMI20(DRV_HDMI_Get_TxInst());

	SiiDrvTxRegisterCallBack(DRV_HDMI_Get_TxInst(), SiiTxCallBack);

	//Checking HDCP2x core update status
	if (sTxConfig.bHdcp2xEn) {
		SiiDrvTxHdcp2xCupdStatusGet(DRV_HDMI_Get_TxInst(), &cupdChkStat);
		switch (cupdChkStat) {
			case SII_DRV_HDCP2X_CUPD_CHK__ERROR:
				HDMI20_DRV_HDMI_PRINTK("HDCP2x Code Update Time Expired... Exit the aplication\n");
				HDMI20_DRV_HDMI_PRINTK("Press any Key to Exit\n");
				SiiDrvTxDelete(DRV_HDMI_Get_TxInst());
				return false;
			case SII_DRV_HDCP2X_CUPD_CHK__FAIL:
				HDMI20_DRV_HDMI_PRINTK("HDCP2x Code Update Failed... Exit the aplication\n");
				HDMI20_DRV_HDMI_PRINTK("Press any Key to Exit\n");
				SiiDrvTxDelete(DRV_HDMI_Get_TxInst());
				return false;
			case SII_DRV_HDCP2X_CUPD_CHK__DONE:
				HDMI20_DRV_HDMI_PRINTK("HDCP2x Code Update Successful\n");
				break;
		}
	}
	//Create CEC
	#if defined(CEC_SUPPORT)
	SiiCecCreate(sTxConfig.instCra);
	SiiCecRegisterEventNotifyCallBack(SiiGetCecInst(), DRV_HDMI_CECNotifyEvent);
	#endif
	HDMI20_DRV_HDMI_PRINT_FUNC_EXIT();
	if ( DRV_HDMI_Get_TxInst() ) {
		return MT_SUCCESS;
	} else {
		return MT_FAILURE;
	}
}

void SiiTxDelete(void)
{
#if defined(CEC_SUPPORT)
	SiiCecDelete();
#endif
	SiiDrvTxDelete(DRV_HDMI_Get_TxInst());
}

MT_U8 SI_3D_Setting(MT_U32 u83DFormat)
{
	switch (u83DFormat) {
		case MT_UNF_EDID_3D_FRAME_PACKETING:
			SI_Set_VSI_3D_FramePacking();
			break;
			#if defined (SUPPORT_3D)
		case MT_UNF_EDID_3D_FIELD_ALTERNATIVE:
			SI_Set_VSI_3D_FieldAlt();
			break;
		case MT_UNF_EDID_3D_LINE_ALTERNATIVE:
			SI_Set_VSI_3D_LineAlt();
			break;
		case MT_UNF_EDID_3D_L_DEPTH:
			SI_Set_VSI_3D_L_Depth();
			break;
		case MT_UNF_EDID_3D_L_DEPTH_GRAPHICS_GRAPHICS_DEPTH:
			SI_Set_VSI_3D_L_Depth_2Graphic_Depth();
			break;
		case MT_UNF_EDID_3D_SIDE_BY_SIDE_FULL:
			SI_Set_VSI_3D_SidebySide_Full();
			break;
			#endif
		case MT_UNF_EDID_3D_TOP_AND_BOTTOM:
			SI_Set_VSI_3D_TopandBottom_Half();
			break;
		case MT_UNF_EDID_3D_SIDE_BY_SIDE_HALF:
			SI_Set_VSI_3D_SidebySide_Half();
			break;
		default:
			COM_ERR("Unknown 3D Mode \n");
			return MT_SUCCESS;
	}

	SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__VS, true);
	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3D_FramePacking(void)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x05; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;

	//3d_structure
	infoframe[7] = 0x40;
	infoframe[8] = 0x00;//[7:4]

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &infoframe_t);

	return MT_SUCCESS;
}

#if defined (SUPPORT_3D)
MT_U8 SI_Set_VSI_3D_FieldAlt(void)
{
	MT_U8 VendorBody[20], offset = 0;

	offset = 0;

	SI_TX_SendInfoFrame(VENDORSPEC_TYPE, VendorBody);

	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3D_LineAlt(void)
{
	MT_U8 VendorBody[20], offset = 0;

	offset = 0;

	SI_TX_SendInfoFrame(VENDORSPEC_TYPE, VendorBody);

	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3D_SidebySide_Full(void)
{
	MT_U8 VendorBody[20], offset = 0;

	offset = 0;

	SI_TX_SendInfoFrame(VENDORSPEC_TYPE, VendorBody);

	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3D_L_Depth(void)
{
	MT_U8 VendorBody[20], offset = 0;

	offset = 0;

	SI_TX_SendInfoFrame(VENDORSPEC_TYPE, VendorBody);

	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3D_L_Depth_2Graphic_Depth(void)
{
	MT_U8 VendorBody[20], offset = 0;

	offset = 0;

	SI_TX_SendInfoFrame(VENDORSPEC_TYPE, VendorBody);

	return MT_SUCCESS;
}
#endif

MT_U8 SI_Set_VSI_3D_TopandBottom_Half(void)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x05; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;

	//3d_structure
	infoframe[7] = 0x40;
	infoframe[8] = 0x60;

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &infoframe_t);

	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3D_SidebySide_Half(void)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x05; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;

	//3d_structure
	infoframe[2] = 0x07; //length
	infoframe[7] = 0x40;
	infoframe[8] = 0x80;

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &infoframe_t);

	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_3Dto2D(void)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x05; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;

	//3d_structure
	infoframe[2] = 0x07; //length
	infoframe[7] = 0x00;
	infoframe[8] = 0x00;

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &infoframe_t);

	return MT_SUCCESS;
}

MT_U8 SI_4K_Setting(MT_U8 u324KFormat)
{
	switch (u324KFormat) {
		case MT_DRV_DISP_FMT_3840X2160_30:

		case MT_DRV_DISP_FMT_3840X2160_25:
		case MT_DRV_DISP_FMT_3840X2160_24:
		case MT_DRV_DISP_FMT_4096X2160_24:
			// 4k setting
			SI_Set_VSI_4K2K(u324KFormat);
			break;
		default:
			COM_ERR("Unknown 4K Mode \n");
			return MT_SUCCESS;
	}
	SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__VS, true);
	return MT_SUCCESS;
}

MT_U8 SI_Set_VSI_4K2K(MT_U32 u324KFormat)
{
	uint8_t i = 0;
	uint8_t cksum = 0;
	SiiInfoFrame_t infoframe_t = {0};
	uint8_t *infoframe;

	infoframe = infoframe_t.b;

	//Followed from h1.4b table8-10 ~ 8-15
	infoframe[0] = 0x81; //packet type
	infoframe[1] = 0x01; //version
	infoframe[2] = 0x05; //length
	infoframe[3] = 0; //checksum

	infoframe[4] = 0x03;
	infoframe[5] = 0x0c;
	infoframe[6] = 0x00;

	if (u324KFormat == MT_DRV_DISP_FMT_3840X2160_24) {
		infoframe[7] = 0x20;
		infoframe[8] = 3;
	} else if (u324KFormat == MT_DRV_DISP_FMT_3840X2160_25) {
		infoframe[7] = 0x20;
		infoframe[8] = 2;
	} else if (u324KFormat == MT_DRV_DISP_FMT_3840X2160_30) {
		infoframe[7] = 0x20;
		infoframe[8] = 1;
	} else if (u324KFormat == MT_DRV_DISP_FMT_4096X2160_24) {
		infoframe[7] = 0x20;
		infoframe[8] = 4;
	}

	for (i = 0; i < SII_INFOFRAME_MAX_LEN; i++) {
		cksum += infoframe[i];
	}
	cksum = 256 - cksum;
	infoframe[3] = cksum;

	infoframe_t.ifId = SII_INFO_FRAME_ID__VS;
	SiiDrvTxInfoframeSet(DRV_HDMI_Get_TxInst(), &infoframe_t);

	return MT_SUCCESS;
}

//set vsdb mode,and base on vsdb mode, to setting 3d infoframe or 4k infoframe
MT_U8 SI_VSDB_Setting(VSDB_MODE_E mode, MT_U32 u32Fmt)
{
	VSDB_MODE_E forMode = DRV_Get_VSDBMode(MT_UNF_HDMI_ID_0);
	VSDB_MODE_E curMode = mode;

	COM_INFO("VSDB mode:%d , u8Fmt:%d \n", mode, u32Fmt);
	switch (curMode) {
		case VSDB_MODE_3D:
			SI_3D_Setting(u32Fmt);
			break;
		case VSDB_MODE_4K:
			SI_4K_Setting(u32Fmt);
			break;
		case VSDB_MODE_NONE:
			if ( forMode == VSDB_MODE_3D ) {
				SI_Set_VSI_3Dto2D();
				SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__VS, true);
			} else {
				SiiDrvTxInfoframeOnOffSet(DRV_HDMI_Get_TxInst(), SII_INFO_FRAME_ID__VS, false);
			}
			if (forMode != curMode) {
				//SI_DisableHdmiDevice();
				//SiiLibTimeMilliDelay(100);
				//SI_EnableHdmiDevice();
			}
			break;
		default:
			COM_ERR("Unknown VSDB Mode !\n");
			break;
	}

	DRV_Set_VSDBMode(MT_UNF_HDMI_ID_0, curMode);

	return MT_SUCCESS;
}

MT_UNF_HDMI_VIDEO_MODE_E DRV_HDMI_Sicsc2Mtcsc(SiiDrvClrSpc_t csc)
{
	MT_UNF_HDMI_VIDEO_MODE_E ret;
	switch (csc) {
		case SII_DRV_CLRSPC__YC444_601:
		case SII_DRV_CLRSPC__YC444_709:
		case SII_DRV_CLRSPC__XVYCC444_601:
		case SII_DRV_CLRSPC__XVYCC444_709:
		case SII_DRV_CLRSPC__YC444_2020:
			ret = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
			break;
		case SII_DRV_CLRSPC__YC422_601:
		case SII_DRV_CLRSPC__YC422_709:
		case SII_DRV_CLRSPC__XVYCC422_601:
		case SII_DRV_CLRSPC__XVYCC422_709:
		case SII_DRV_CLRSPC__YC422_2020:
			ret = MT_UNF_HDMI_VIDEO_MODE_YCBCR422;
			break;
		case SII_DRV_CLRSPC__YC420_601:
		case SII_DRV_CLRSPC__YC420_709:
		case SII_DRV_CLRSPC__XVYCC420_601:
		case SII_DRV_CLRSPC__XVYCC420_709:
		case SII_DRV_CLRSPC__YC420_2020:
			ret = MT_UNF_HDMI_VIDEO_MODE_YCBCR420;
			break;
		case SII_DRV_CLRSPC__RGB_FULL:
		case SII_DRV_CLRSPC__RGB_LIMITED:
			ret = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			break;
		default:
			ret = MT_UNF_HDMI_VIDEO_MODE_BUTT;
			break;
	}
	return ret;
}

MT_UNF_HDMI_DEEP_COLOR_E DRV_HDMI_Sidc2Mtdc(SiiDrvBitDepth_t dc)
{
	MT_UNF_HDMI_DEEP_COLOR_E ret;
	switch (dc) {
		case SII_DRV_BIT_DEPTH__8_BIT:
			ret = MT_UNF_HDMI_DEEP_COLOR_24BIT;
			break;
		case SII_DRV_BIT_DEPTH__10_BIT:
			ret = MT_UNF_HDMI_DEEP_COLOR_30BIT;
			break;
		case SII_DRV_BIT_DEPTH__12_BIT:
			ret = MT_UNF_HDMI_DEEP_COLOR_36BIT;
			break;
		case SII_DRV_BIT_DEPTH__16_BIT:
			ret = MT_UNF_HDMI_DEEP_COLOR_48BIT;
			break;
		case SII_DRV_BIT_DEPTH__PASSTHOUGH:
			ret = MT_UNF_HDMI_DEEP_COLOR_BUTT;
			break;
		default:
			ret = MT_UNF_HDMI_DEEP_COLOR_BUTT;
			break;
	}
	return ret;
}

