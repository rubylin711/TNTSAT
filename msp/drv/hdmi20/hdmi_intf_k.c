/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/device.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/poll.h>
//#include <mach/hardware.h>
#include <linux/interrupt.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/delay.h>

//#include "common_dev.h"
#include "mt_drv_dev.h"
//#include "common_proc.h"
#include "mt_drv_proc.h"

//#include "mpi_priv_hdmi.h"
#include "mt_drv_hdmi.h"
//#include "mpi_priv_disp.h"
#include "mt_drv_disp.h"
#include "drv_disp_ioctl.h"
#include "drv_hdmi.h"
#include "mt_unf_hdmi.h"

#include "mt_unf_disp.h"
#include "mt_type.h"
#include "drv_disp_ext.h"
#include "mt_drv_module.h"
#include "mt_kernel_adapt.h"
#include "drv_hdmi_ext.h"
//#include "drv_cipher_ext.h"
//#include "drv_gpio_ext.h"
//#include "drv_hdmi20_ext.h"
#include "drv_global.h"
#include "drv_hdmi_ioctl.h"
#include "drv_compatibility.h"
#include "si_hdmitx.h"
#include "si_misc.h"

MT_DECLARE_MUTEX(g_hdmiMutex);

mt_s32 HDMI_DRV_Init(mt_void);
mt_void  HDMI_DRV_EXIT(mt_void);
mt_s32 MT_DRV_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi);
mt_s32 MT_DRV_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi);

#define MT_ERR_HDMI_COPY_DATA_ERR       -2
#define MT_ERR_HDMI_MALLOC_ERR          -3
#define MT_ERR_HDMI_FAILURE             -4

//#define TX_SLV1							(0x7A)
//#define TX_SLV0							(0x72)
//#define HDMI_DEV_CEC					(0xCC)

static mt_u32 g_u32CallbackAddr = 0;
static mt_u32 FromUserSpaceFlag = MT_TRUE;
#if 1
	static mt_u32  g_KernelProcID = MT_INVALID_HANDLE;
#endif

ulong g_ProcHandle = MT_INVALID_HANDLE;

mt_u32 HDMIStandbySetupFlag =  MT_FALSE;

//-------------------------------------------------------------------

/*****************************************************************************
 Prototype    : hdmi_Proc
 Description  : HDMI status in /proc/msp/hdmi
 Input        : None
 Output       : None
 Return Value :
 Calls        :
*****************************************************************************/

/*no use to use reference, we just get a func ptr.*/
mt_s32 hdmi_Open(struct inode *inode, struct file *filp)
{
	g_ProcHandle = (ulong)filp;

	return MT_SUCCESS;
}

mt_s32 hdmi_Close(struct inode *inode, struct file *filp)
{
	mt_u32 u32Index;
	HDMI_PROC_EVENT_S *pEventList =  DRV_Get_EventList(MT_UNF_HDMI_ID_0);

	COM_INFO("\n\ncome to hdmi_Close\n\n");

	#if 1
	for (u32Index = 0; u32Index < MAX_PROCESS_NUM; u32Index++) {
		if (pEventList[u32Index].u32ProcHandle == (ulong)filp) {
			DRV_HDMI_ReleaseProcID(MT_UNF_HDMI_ID_0, u32Index);
			break;
		}
	}

	if (DRV_Get_IsThreadStoped()) {
		//avoid ctrl+c in setFormatting / setAttring
		DRV_Set_ThreadStop(MT_FALSE);
	}

	DRV_HDMI_DeInit(MT_TRUE);

	if (DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) == 0) {
		HDMIStandbySetupFlag = MT_FALSE;
	}
	//disp_func_ops = NULL;
	#endif
	return 0;
}

unsigned int suspend_flag = 0;
unsigned int mt_suspend_flag = 0;
unsigned int start_flag = 0;
extern HDMI_CHN_ATTR_S  *DRV_Get_Glb_Param(void);

extern mt_void HDMI_PinConfig(mt_void);

mt_s32 hdmi_Suspend(basedev_s *pdev, pm_message_t state)
{
	//volatile mt_u32 *pulArgs = (mt_u32*)IO_ADDRESS(HDMI_HARDWARE_RESET_ADDR);
	//mt_u32 tmp = 0;
	//HDMI_CHN_ATTR_S *g_stHdmiChnParam_ptr = NULL;
	#if defined (CEC_SUPPORT)
	HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
	#endif

	mt_suspend_flag = 1;
	if (MT_FALSE == HDMIStandbySetupFlag) {
		COM_FATAL("HDMI Do not setup before\n");
		return 0;
	}

	//g_stHdmiChnParam_ptr = DRV_Get_ChnAttr();
	suspend_flag = 1;

	SI_SendCP_Packet(ON);
	SI_SetHdmiVideo(MT_FALSE);
	SI_SetHdmiAudio(MT_FALSE);
	start_flag = DRV_Get_IsChnStart(MT_UNF_HDMI_ID_0);
	DRV_Set_ChnStart(MT_UNF_HDMI_ID_0, MT_FALSE);

	#if defined (HDCP_SUPPORT)
	//SI_WriteByteEEPROM(EE_TX_HDCP, 0x00);
	SI_SetEncryption(OFF);
	DelayMS(1);
	#endif
	SI_DisableHdmiDevice();
	SI_CloseHdmiDevice();

	#if defined (CEC_SUPPORT)
	SI_CEC_Close();
	//DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
	pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
	memset(&(pstChnAttr[MT_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
	#endif

	SI_PowerDownHdmiTx();

	SI_PoweDownHdmiDevice();

	SI_HW_ResetCtrl(1);
	DelayMS(1);
	SI_HW_ResetPhy(1);
	DelayMS(1);

	MT_PRINT("HDMI suspend OK\n");
	return 0;
}

mt_s32 hdmi_Resume(basedev_s *pdev)
{
	#if 1//MT_DEVELOP_COMP_MASK
	HDMI_ATTR_S *pstHDMIAttr = DRV_Get_HDMIAttr(MT_UNF_HDMI_ID_0);
	//mt_suspend_flag = 0;
	if (0 == suspend_flag) {
		COM_FATAL("HDMI Do not hdmi_Suspend before\n");
		return 0;
	}

	#if defined (HDCP_SUPPORT)
	//SI_WriteByteEEPROM(EE_TX_HDCP, 0xFF);
	//set hdcp enable in setAttr In hotplug
	//SI_WriteByteEEPROM(EE_TX_HDCP, 0x00);
	#endif
	//init
	HDMI_PinConfig();
	SI_HW_ResetHDMITX();
	SI_SW_ResetHDMITX();
	//SI_DisableHdmiDevice();
	#if defined (CEC_SUPPORT)
	SI_CEC_SetUp();
	#endif

	DRV_Set_ForceUpdateFlag(MT_UNF_HDMI_ID_0, MT_TRUE);
	#if 1 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	//DRV_HDMI_GetAttr(enHDMI, &stHDMIAttr);
	DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_0, pstHDMIAttr);
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

	if (start_flag == MT_TRUE) {
		DRV_HDMI_Start(MT_UNF_HDMI_ID_0);
	}

	//g_stHdmiChnParam_ptr->bStart = start_flag;

	#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	SI_DisableInfoFrame(AVI_TYPE);
	SI_DisableInfoFrame(AUD_TYPE);
	if (start_flag == MT_TRUE) {
		#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
		#if defined (HDCP_SUPPORT)
		reset_hdcp_counter();
		#endif
		#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
		DRV_HDMI_Start(MT_UNF_HDMI_ID_0);
	}

	SI_EnableHdmiDevice();
	//udelay(200);
	SI_EnableInfoFrame(AVI_TYPE);
	//udelay(200);
	SI_EnableInfoFrame(AUD_TYPE);
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

	MT_PRINT("HDMI resume OK\n");
	suspend_flag = 0;
	//bStandby = MT_TRUE;
	#endif

	return 0;
}
unsigned int cec_enable_flag = 0;
static mt_s32 hdmi_ProcessCmd(unsigned int cmd, mt_void *arg, MT_BOOL bUser)
{
	mt_s32       u32Ret = MT_FAILURE;

	if (cmd != CMD_HDMI_POLL_EVENT && cmd != CMD_HDMI_GET_STATUS) {
		HDMI20_INTF_K__PRINT_FUNC_ENTER();
		HDMI20_INTF_K_PRINTK("\nhdmi_ProcessCmd 0x%x, %d\n", cmd, (mt_u32)bUser);
	}
	switch (cmd) {
		case CMD_HDMI_INIT: {
			HDMI_INIT_S *pHdmiInit;
			pHdmiInit = (HDMI_INIT_S*)arg;

			FromUserSpaceFlag = bUser;
			u32Ret = DRV_HDMI_Init(bUser);
			HDMIStandbySetupFlag = MT_TRUE;
			break;
		}

		case CMD_HDMI_DEINIT: {
			u32Ret = DRV_HDMI_DeInit(bUser);

			if (DRV_HDMI_GetInitNum(MT_UNF_HDMI_ID_0) == 0) {
				HDMIStandbySetupFlag = MT_FALSE;
			}

			break;
		}

		case CMD_HDMI_OPEN: {
			HDMI_OPEN_S *phdmiOpen;
			phdmiOpen = (HDMI_OPEN_S*)arg;
			u32Ret = DRV_HDMI_Open(phdmiOpen->enHdmi, phdmiOpen, bUser, phdmiOpen->u32ProcID);
			break;
		}

		case CMD_HDMI_CLOSE: {
			HDMI_CLOSE_S *phdmiClose;
			phdmiClose = (HDMI_CLOSE_S*)arg;
			u32Ret = DRV_HDMI_Close(phdmiClose->enHdmi);
			break;
		}

		case CMD_HDMI_START: {
			HDMI_START_S *phdmiStart;
			phdmiStart = (HDMI_START_S*)arg;
			u32Ret = DRV_HDMI_Start(phdmiStart->enHdmi);
			break;
		}

		case CMD_HDMI_STOP: {
			HDMI_STOP_S *phdmiStop;
			phdmiStop = (HDMI_STOP_S*)arg;
			u32Ret = DRV_HDMI_Stop(phdmiStop->enHdmi);
			break;
		}

		case CMD_HDMI_SINK_CAPABILITY: {
			HDMI_SINK_CAPABILITY_S *phdmisinkcap;
			phdmisinkcap = (HDMI_SINK_CAPABILITY_S*)arg;
			u32Ret = DRV_HDMI_GetSinkCapability(phdmisinkcap->enHdmi, &(phdmisinkcap->SinkCap));
			break;
		}

		case CMD_HDMI_POLL_EVENT: {
			HDMI_POLL_EVENT_S *pPollEvent;
			pPollEvent = (HDMI_POLL_EVENT_S*)arg;
			//compare callback addr in mpi
			//MT_ERR_HDMI("\n ---hdmi read event start--- \n");
			pPollEvent->Event = DRV_HDMI_ReadEvent(pPollEvent->enHdmi, pPollEvent->u32ProcID);
			//MT_ERR_HDMI("\n ---hdmi read event over--- \n");
			u32Ret = (pPollEvent->Event != 0) ? MT_SUCCESS : MT_ERR_HDMI_FAILURE;

			//����������
			//pPollEvent->u32CallbackAddr = g_u32CallbackAddr;
			break;
		}

		case CMD_HDMI_GET_ATTR: {
			HDMI_PORT_ATTR_S *phdmiattr;
			HDMI_ATTR_S      stHDMIAttr;
			phdmiattr = (HDMI_PORT_ATTR_S*)arg;

			memset((void*)&stHDMIAttr, 0, sizeof(HDMI_ATTR_S));
			u32Ret = DRV_HDMI_GetAttr(phdmiattr->enHdmi, &stHDMIAttr);
			memcpy(&phdmiattr->stHdmiAppAttr, &stHDMIAttr.stAppAttr, sizeof(HDMI_APP_ATTR_S));
			break;
		}

		case CMD_HDMI_SET_ATTR: {
			HDMI_PORT_ATTR_S *phdmiattr;
			HDMI_ATTR_S      stHDMIAttr;

			phdmiattr = (HDMI_PORT_ATTR_S*)arg;

			u32Ret = DRV_HDMI_GetAttr(phdmiattr->enHdmi, &stHDMIAttr);

			memcpy(&stHDMIAttr.stAppAttr, &phdmiattr->stHdmiAppAttr, sizeof(HDMI_APP_ATTR_S));

			//u32Ret = DRV_HDMI_SetAttr(phdmiattr->enHdmi, &phdmiattr->stHdmiAppAttr);
			u32Ret |= DRV_HDMI_SetAttr(phdmiattr->enHdmi, &stHDMIAttr);
			break;
		}

		case CMD_HDMI_GET_INFORFRAME: {
			HDMI_INFORFRAME_S *pInfoframe;

			pInfoframe = (HDMI_INFORFRAME_S*)arg;
			u32Ret = DRV_HDMI_GetInfoFrame(pInfoframe->enHdmi, pInfoframe->enInfoFrameType, &(pInfoframe->InfoFrame));
			break;
		}

		case CMD_HDMI_SET_INFORFRAME: {
			HDMI_INFORFRAME_S *pInfoframe;

			pInfoframe = (HDMI_INFORFRAME_S*)arg;
			u32Ret = DRV_HDMI_SetInfoFrame(pInfoframe->enHdmi, &(pInfoframe->InfoFrame));
			break;
		}

		case CMD_HDMI_SET_AVMUTE: {
			HDMI_AVMUTE_S *pAvmute;

			pAvmute = (HDMI_AVMUTE_S*)arg;
			u32Ret = DRV_HDMI_SetAVMute(pAvmute->enHdmi, pAvmute->AVMuteEnable);
			break;
		}
		case CMD_HDMI_VIDEO_TIMING: {
			HDMI_VIDEOTIMING_S *pTiming;

			pTiming = (HDMI_VIDEOTIMING_S*)arg;
			u32Ret = DRV_HDMI_SetFormat(pTiming->enHdmi, pTiming->VideoTiming, pTiming->enStereo);
			break;
		}
		case CMD_HDMI_PREVTIMING: {
			HDMI_PREVIDEOTIMING_S *pstPreVideoTiming;

			pstPreVideoTiming = (HDMI_PREVIDEOTIMING_S*)arg;
			u32Ret = DRV_HDMI_PreFormat(pstPreVideoTiming->enHdmi, pstPreVideoTiming->VideoTiming);
			break;
		}
		case CMD_HDMI_GET_DEEPCOLOR: {
			HDMI_DEEPCOLORC_S *pDeepcolormode;

			pDeepcolormode = (HDMI_DEEPCOLORC_S*)arg;
			u32Ret = DRV_HDMI_GetDeepColor(pDeepcolormode->enHdmi, &(pDeepcolormode->enDeepColor));
			break;
		}
		case CMD_HDMI_SET_DEEPCOLOR: {
			HDMI_DEEPCOLORC_S *pDeepcolormode;

			pDeepcolormode = (HDMI_DEEPCOLORC_S*)arg;
			u32Ret = DRV_HDMI_SetDeepColor(pDeepcolormode->enHdmi, pDeepcolormode->enDeepColor);
			break;
		}
		case CMD_HDMI_SET_XVYCC: {
			HDMI_SET_XVYCC_S *pxvYCCmode;

			pxvYCCmode = (HDMI_SET_XVYCC_S*)arg;
			u32Ret = DRV_HDMI_SetxvYCCMode(pxvYCCmode->enHdmi, pxvYCCmode->xvYCCEnable);
			break;
		}

		#if defined (CEC_SUPPORT)
		case CMD_HDMI_SET_CEC: {
			HDMI_CEC_S *pCECCmd;
			pCECCmd = (HDMI_CEC_S*)arg;
			if(CEC_OPCODE_IMAGE_VIEW_ON == pCECCmd->CECCmd.u8Opcode){
				SiiCecRemotePassPress(0x40);
				msleep(100);
				//SiiCecOneTouchPlay();
				//u32Ret = 0;
				//SiiCecSetSourceActive( true );
			}
			u32Ret = DRV_HDMI_SetCECCommand(pCECCmd->enHdmi, &(pCECCmd->CECCmd));
			break;
		}
		case CMD_HDMI_GET_CEC: {
			HDMI_CEC_S *pCECCmd;
			pCECCmd = (HDMI_CEC_S*)arg;
			u32Ret = DRV_HDMI_GetCECCommand(pCECCmd->enHdmi, &(pCECCmd->CECCmd), pCECCmd->timeout);
			//u32Ret = (u32Ret == 0)?MT_FAILURE:MT_SUCCESS;
			break;
		}
		case CMD_HDMI_CECSTATUS: {
			HDMI_CEC_STATUS *pCECStatus;
			pCECStatus = (HDMI_CEC_STATUS*)arg;
			u32Ret = DRV_HDMI_CECStatus(pCECStatus->enHdmi, &(pCECStatus->stStatus));
			break;
		}
		case CMD_HDMI_CEC_ENABLE: {
			u32Ret = MT_SUCCESS;
			if ( !DRV_Get_IsCECEnable(MT_UNF_HDMI_ID_0) ) {
				SI_CEC_SetUp();
				DRV_Set_CECEnable(MT_UNF_HDMI_ID_0, MT_TRUE);
				u32Ret = SI_CEC_Open();
			}
			//cec_enable_flag = 1;
			break;
		}
		case CMD_HDMI_CEC_DISABLE: {
			HDMI_CHN_ATTR_S *pstChnAttr = DRV_Get_ChnAttr();
			DRV_Set_CECEnable(MT_UNF_HDMI_ID_0, MT_FALSE);
			DRV_Set_CECStart(MT_UNF_HDMI_ID_0, MT_FALSE);
			pstChnAttr[MT_UNF_HDMI_ID_0].u8CECCheckCount = 0;
			memset(&(pstChnAttr[MT_UNF_HDMI_ID_0].stCECStatus), 0, sizeof(MT_UNF_HDMI_CEC_STATUS_S));
			u32Ret = SI_CEC_Close();
			//cec_enable_flag = 0;
			break;
		}
		#endif
		case CMD_HDMI_HDCP_ENABLE: {
			//DRV_HDMI_HDCP_Set(MT_UNF_HDMI_ID_0, MT_TRUE);
			u32Ret = MT_SUCCESS;
			break;
		}
		case CMD_HDMI_HDCP_DISABLE: {
			//DRV_HDMI_HDCP_Set(MT_UNF_HDMI_ID_0, MT_FALSE);
			u32Ret = MT_SUCCESS;
			break;
		}
		case CMD_HDMI_FORCE_GET_EDID: {
			HDMI_EDID_S *pEDID;

			pEDID = (HDMI_EDID_S*)arg;
			u32Ret = DRV_HDMI_Force_GetEDID(pEDID);
			break;
		}
		case CMD_HDMI_GET_RAW_EDID_INFO: {
			HDMI_RAW_EDID_INFO_S *raw_edid_info;
			raw_edid_info =  (HDMI_RAW_EDID_INFO_S*)arg;
			u32Ret = DRV_HDMI_GetRawEdidInfo(raw_edid_info->enHdmi, raw_edid_info->rawEdidInfo);
			break;
		}
		case CMD_HDMI_GET_HDMI_PLAYSTAUS: {
			HDMI_PLAYSTAUS_S *pPlayStaus;

			pPlayStaus = (HDMI_PLAYSTAUS_S*)arg;
			u32Ret = DRV_HDMI_GetPlayStatus(pPlayStaus->enHdmi, &(pPlayStaus->u32PlayStaus));
			break;
		}
		case CMD_HDMI_REG_CALLBACK_FUNC: {
			HDMI_REGCALLBACKFUNC_S *pstRegCallbackFunc;
			pstRegCallbackFunc = (HDMI_REGCALLBACKFUNC_S*)arg;
			g_u32CallbackAddr = pstRegCallbackFunc->u32CallBackAddr;
			u32Ret = MT_SUCCESS;
			break;
		}
		case CMD_HDMI_LOADKEY: {
			HDMI_LOADKEY_S *pstLoadKey;

			pstLoadKey = (HDMI_LOADKEY_S*)arg;
			u32Ret = DRV_HDMI_LoadKey(pstLoadKey->enHdmi, &pstLoadKey->stLoadKey);
			break;
		}
		case CMD_HDMI_GET_PROCID: {
			HDMI_GET_PROCID_S *pstProcID;
			pstProcID = (HDMI_GET_PROCID_S*)arg;
			u32Ret = DRV_HDMI_GetProcID(pstProcID->enHdmi, &pstProcID->u32ProcID);
			break;
		}
		case CMD_HDMI_RELEASE_PROCID: {
			HDMI_GET_PROCID_S *pstProcID;
			pstProcID = (HDMI_GET_PROCID_S*)arg;
			u32Ret = DRV_HDMI_ReleaseProcID(pstProcID->enHdmi, pstProcID->u32ProcID);
			break;
		}
		case CMD_HDMI_GET_AO_ATTR: {
			HDMI_GETAOATTR_S *pstGetAOAttr;
			pstGetAOAttr = (HDMI_GETAOATTR_S *)arg;
			u32Ret = DRV_HDMI_GetAOAttr(pstGetAOAttr->enHdmi, &pstGetAOAttr->stAudAttr);
			break;
		}
		case CMD_HDMI_AUDIO_CHANGE: {
			HDMI_GETAOATTR_S *pstGetAOAttr;
			pstGetAOAttr = (HDMI_GETAOATTR_S *)arg;
			u32Ret = DRV_HDMI_AudioChange(pstGetAOAttr->enHdmi, &pstGetAOAttr->stAudAttr);
			break;
		}
		case CMD_HDMI_GET_STATUS: {
			HDMI_STATUS_S *pstStatus;
			pstStatus = (HDMI_STATUS_S *)arg;
			u32Ret = DRV_HDMI_GetStatus(pstStatus->enHdmi, &pstStatus->stStatus);
			break;
		}
		case CMD_HDMI_GET_DELAY: {
			HDMI_DELAY_S *pstDelay;
			pstDelay = (HDMI_DELAY_S *)arg;
			DRV_HDMI_GetDelay(pstDelay->enHdmi, &pstDelay->stDelay);
			u32Ret = MT_SUCCESS;
			break;
		}
		case CMD_HDMI_SET_DELAY: {
			HDMI_DELAY_S *pstDelay;
			pstDelay = (HDMI_DELAY_S *)arg;
			DRV_HDMI_SetDelay(pstDelay->enHdmi, &pstDelay->stDelay);
			u32Ret = MT_SUCCESS;
			break;
		}
		case CMD_HDMI_SET_NOTIFY: {
			u32Ret = DRV_HDMI_Set_Notify(arg);
			break;
		}
		case CMD_HDMI_OUTPUT_SET: {
			HDMI_OUTPUT_S *phdmiOutput;
			phdmiOutput = (HDMI_OUTPUT_S *)arg;
			u32Ret = DRV_HDMI_Output_Set(phdmiOutput->outEnable);
			break;
		}
		case CMD_HDMI_REGISTERS_DUMP: {
			HDMI_REG_DUMP_S *phdmiDump;
			phdmiDump = (HDMI_REG_DUMP_S *)arg;
			u32Ret = DRV_HDMI_Registers_Dump(phdmiDump->enHdmi);
			break;
		}
		case CMD_HDMI_REGISTER_WRITE: {
			HDMI_REG_WRITE_S *phdmiWrite;
			phdmiWrite = (HDMI_REG_WRITE_S *)arg;
			u32Ret = DRV_HDMI_Register_Write(phdmiWrite->enHdmi, phdmiWrite->addr, phdmiWrite->data);
			break;
		}
		case CMD_HDMI_REGISTER_READ: {
			HDMI_REG_READ_S *phdmiRead;
			mt_u8 data = 0;
			phdmiRead = (HDMI_REG_READ_S *)arg;

			u32Ret = DRV_HDMI_Register_Read(phdmiRead->enHdmi, phdmiRead->addr, &data);
			put_user((mt_u32)data, phdmiRead->data);  // fixed sym4 crash issue,  copy to user space
			break;
		}
		case CMD_HDMI_SET_OSDNAME: {
			HDMI_OSD_NAME_S *osd_name;
			osd_name =	(HDMI_OSD_NAME_S*)arg;
			u32Ret = DRV_HDMI_SetOsdName(osd_name->enHdmi, osd_name->osdName);
			break;
		}
		default: {
			COM_ERR("unkonw cmd:0x%x\n", cmd);
			return -ENOIOCTLCMD;
		}
	}

	return u32Ret;

}
//unsigned int cec_enable_flag = 0;
mt_s32 hdmi_Ioctl(struct inode *inode, struct file *file,
				  unsigned int cmd, mt_void *arg)
{
	mt_s32   s32Ret = MT_FAILURE;
	s32Ret = down_interruptible(&g_hdmiMutex);
	s32Ret = hdmi_ProcessCmd(cmd, arg, MT_TRUE);
	up(&g_hdmiMutex);
	return s32Ret;
}

//�޸���������
//mt_s32  HDMI_ModeInit_0(mt_void)
mt_s32 HDMI_DRV_Init(mt_void)
{
	#ifdef HDMI_DEBUG
	if (MT_FAILURE == HDMI_DbgInit()) {
		MT_PRINT("HDMI debug Init failed!\n");
	}
	#endif
	#if 1
	return DRV_HDMI_Register();
	#else
	return MT_SUCCESS;
	#endif
}

//�޸���������
//mt_void  HDMI_ModeExit_0(mt_void)
mt_void  HDMI_DRV_EXIT(mt_void)
{
	#ifdef HDMI_DEBUG
	HDMI_DbgDeInit();
	#endif
	#if 1

	DRV_HDMI_UnRegister();
	#endif
	return;
}

/****************************************************************/
extern mt_u32 DRV_HDMI_Get_Update_Tvsys(mt_void);
mt_s32 hdmi_ExtIoctl(unsigned int cmd, void *argp)
{

	mt_s32 s32Ret = MT_FAILURE;
	mt_u32 force_no_sem = 0;

	force_no_sem = DRV_HDMI_Get_Update_Tvsys() && (cmd == CMD_HDMI_VIDEO_TIMING);
	if (!in_atomic() && force_no_sem == 0) {
		s32Ret = down_interruptible(&g_hdmiMutex);
	}
	s32Ret = hdmi_ProcessCmd(cmd, argp, MT_FALSE);
	if (!in_atomic() && force_no_sem == 0) {
		up(&g_hdmiMutex);
	}
	return s32Ret;
}
void hdmi_MCE_ProcHotPlug(MT_UNF_HDMI_ID_E hHdmi)
{
	mt_s32          ret = MT_SUCCESS;
	//MT_UNF_HDMI_ATTR_S             stHdmiAttr;
	//HDMI_SINK_CAPABILITY_S stSinkCap;
	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	HDMI_PORT_ATTR_S stHDMIPortAttr;
	HDMI_APP_ATTR_S *pstAppAttr;
	HDMI_START_S stHDMIStart;
	HDMI_STATUS_S stHdmiStatus;

	MT_PRINT("\n---HDMI kernel event(no UserCallBack): HOTPLUG. --- \n");
	memset(&stHdmiStatus, 0, sizeof(HDMI_STATUS_S));
	stHdmiStatus.enHdmi = hHdmi;
	ret = hdmi_ExtIoctl(CMD_HDMI_GET_STATUS, &stHdmiStatus);
	if (ret != MT_SUCCESS) {
		COM_ERR("Get HDMI Status err!\n");
		return ;
	}

	if (MT_FALSE == stHdmiStatus.stStatus.bConnected) {
		COM_ERR("No Connect!\n");
		return;
	}

	COM_INFO("Connect !\n");

	#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	memset(&stSinkCap, 0, sizeof(HDMI_SINK_CAPABILITY_S));
	stSinkCap.enHdmi = hHdmi;

	ret = hdmi_ExtIoctl(CMD_HDMI_SINK_CAPABILITY, &stSinkCap);
	if (ret != MT_SUCCESS) {
		COM_ERR("Get SINK_CAPABILITY err!\n");
		bIsRealEDID = MT_FALSE;

		if (ret != MT_ERR_HDMI_NOT_REAL_EDID) {
			return ;
		}
	}

	COM_INFO("hdmi_ExtIoctl CMD_HDMI_SINK_CAPABILITY ok! \n");
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

	memset(&stHDMIPortAttr, 0, sizeof(HDMI_PORT_ATTR_S));
	stHDMIPortAttr.enHdmi = hHdmi;
	ret = hdmi_ExtIoctl(CMD_HDMI_GET_ATTR, &stHDMIPortAttr);
	if (ret != MT_SUCCESS) {
		COM_ERR("Get hdmi attr err!\n");
		return ;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_GET_ATTR ok! \n");

	pstAppAttr = &stHDMIPortAttr.stHdmiAppAttr;

	if (DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		//stHdmiAttr.enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_YCBCR444;
		if (MT_TRUE == pSinkCap->bSupportHdmi) {
			pstAppAttr->bEnableHdmi = MT_TRUE;
			if (MT_TRUE != pSinkCap->stColorSpace.bYCbCr444) {
				pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			}
		} else {
			pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
			//��ȡ����edid�����Ҳ�֧��hdmi�����?viģʽ
			//read real edid ok && sink not support hdmi,then we run in dvi mode
			pstAppAttr->bEnableHdmi = MT_FALSE;
		}
	} else {
		if (DRV_Get_DefaultOutputMode(MT_UNF_HDMI_ID_0) != MT_UNF_HDMI_DEFAULT_ACTION_DVI) {
			pstAppAttr->bEnableHdmi = MT_TRUE;
		} else {
			pstAppAttr->bEnableHdmi = MT_FALSE;
		}
	}

	if (MT_TRUE == pstAppAttr->bEnableHdmi) {
		pstAppAttr->bEnableAudio = MT_TRUE;
		pstAppAttr->bEnableVideo = MT_TRUE;
		pstAppAttr->bEnableAudInfoFrame = MT_TRUE;
		pstAppAttr->bEnableAviInfoFrame = MT_TRUE;
	} else {
		pstAppAttr->bEnableAudio = MT_FALSE;
		pstAppAttr->bEnableVideo = MT_TRUE;
		pstAppAttr->bEnableAudInfoFrame = MT_FALSE;
		pstAppAttr->bEnableAviInfoFrame = MT_FALSE;
		pstAppAttr->enVidOutMode = MT_UNF_HDMI_VIDEO_MODE_RGB444;
	}

	ret = hdmi_ExtIoctl(CMD_HDMI_SET_ATTR, &stHDMIPortAttr);
	if (ret != MT_SUCCESS) {
		COM_ERR("set attr err!:0x%x\n", ret);
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_SET_ATTR ok! \n");

	// 9
	memset(&stHDMIStart, 0, sizeof(HDMI_START_S));
	stHDMIStart.enHdmi = hHdmi;
	ret = hdmi_ExtIoctl(CMD_HDMI_START, &stHDMIStart);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi startup  err!:0x%x\n", ret);
		return ;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_START ok! \n");
	return;
}

#if 1
//for O5
mt_s32 MT_DRV_HDMI_ExtIoctl(unsigned int cmd, void *argp)
{
	mt_s32 s32Ret = MT_SUCCESS;
	s32Ret = hdmi_ExtIoctl(cmd, argp);
	return s32Ret;
}
//for 05
mt_s32  MT_DRV_HDMI_GetCECAddr(mt_u32 *pcec_addr)
{
	MT_UNF_HDMI_CEC_STATUS_S     *pstCECStatus;
	HDMI_CHN_ATTR_S *pstHdmiChnParam;
	pstHdmiChnParam = DRV_Get_ChnAttr();
	if (pstHdmiChnParam[0].bCECStart == MT_FALSE) {
		return MT_FAILURE;
	}

	pstCECStatus = &(pstHdmiChnParam[0].stCECStatus);
	*pcec_addr = pstCECStatus->u8LogicalAddr;

	return MT_SUCCESS;
}
//for 05
mt_void MT_DRV_HDMI_ProcHotPlug(MT_HANDLE hHdmi)
{
	hdmi_MCE_ProcHotPlug(hHdmi);
}
//for 05
mt_s32 MT_DRV_HDMI_GetBinInfoFrame(MT_UNF_HDMI_INFOFRAME_TYPE_E enInfoFrameType, void *infor_ptr)
{
	DRV_O5_HDMI_GetBinInfoFrame(enInfoFrameType, infor_ptr);
	return 0;
}

mt_s32 MT_DRV_HDMI_Open(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 ret = 0;
	HDMI_INIT_S stHDMIInit;
	HDMI_OPEN_S stHDMIOpen;
	HDMI_GET_PROCID_S stHDMIProcIDOpen;

	memset(&stHDMIProcIDOpen, 0, sizeof(HDMI_GET_PROCID_S));
	//reg disp func
	hdmi_Open(MT_NULL, MT_NULL);

	ret = hdmi_ExtIoctl(CMD_HDMI_INIT, &stHDMIInit);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi init err!:0x%x\n", ret);
		return MT_FAILURE;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_INIT ok! \n");

	if (g_KernelProcID == MT_INVALID_HANDLE) {
		ret = hdmi_ExtIoctl(CMD_HDMI_GET_PROCID, &stHDMIProcIDOpen);
		if (ret != MT_SUCCESS) {
			COM_ERR("Error:HDMI Process is full,can't get process ID:0x%x\n", ret);
			return MT_FAILURE;
		}
		g_KernelProcID = stHDMIProcIDOpen.u32ProcID;
	}

	// 2
	stHDMIOpen.enHdmi = enHdmi;
	stHDMIOpen.enDefaultMode = MT_UNF_HDMI_DEFAULT_ACTION_HDMI;
	stHDMIOpen.u32ProcID = g_KernelProcID;
	//stHDMIOpen.g_U32CallbackAddr = (mt_u32)hdmi_MCE_ProcHotPlug;
	/*set 322 encryted key*/
	ret = hdmi_ExtIoctl(CMD_HDMI_OPEN, &stHDMIOpen);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi open err!:0x%x\n", ret);
		return MT_FAILURE;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_OPEN ok! \n");

	return MT_SUCCESS;
}

mt_s32 MT_DRV_HDMI_Close(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 ret = 0;
	HDMI_STOP_S stHDMIStop;
	HDMI_CLOSE_S stHDMIClose;
	HDMI_DEINIT_S stHDMIDeinit;
	HDMI_GET_PROCID_S stHDMIProcIDRelease;

	// 0
	memset(&stHDMIStop, 0, sizeof(HDMI_STOP_S));
	stHDMIStop.enHdmi = enHdmi;
	ret = hdmi_ExtIoctl(CMD_HDMI_STOP, &stHDMIStop);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi stop err!:0x%x\n", ret);
		return MT_FAILURE;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_STOP ok! \n");

	// 1
	memset(&stHDMIClose, 0, sizeof(HDMI_CLOSE_S));
	stHDMIClose.enHdmi = enHdmi;
	ret = hdmi_ExtIoctl(CMD_HDMI_CLOSE, &stHDMIClose);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi close err!:0x%x\n", ret);
		return MT_FAILURE;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_CLOSE ok! \n");

	// 2
	memset(&stHDMIDeinit, 0, sizeof(HDMI_DEINIT_S));
	ret = hdmi_ExtIoctl(CMD_HDMI_DEINIT, &stHDMIDeinit);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi deinit err!:0x%x\n", ret);
		return MT_FAILURE;
	}
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_DEINIT ok! \n");

	// 3
	memset(&stHDMIProcIDRelease, 0, sizeof(HDMI_GET_PROCID_S));
	stHDMIProcIDRelease.u32ProcID = g_KernelProcID;
	ret = hdmi_ExtIoctl(CMD_HDMI_RELEASE_PROCID, &stHDMIProcIDRelease);
	if (ret != MT_SUCCESS) {
		COM_ERR("hdmi release proc id err!:0x%x\n", ret);
		return MT_FAILURE;
	}
	g_KernelProcID = MT_INVALID_HANDLE;
	COM_INFO("hdmi_ExtIoctl CMD_HDMI_RELEASE_PROCID ok! \n");

	return MT_SUCCESS;
}

mt_s32  MT_DRV_HDMI_Init(mt_void)
{

	return HDMI_DRV_Init();
}

mt_void  MT_DRV_HDMI_Deinit(mt_void)
{
	HDMI_DRV_EXIT();
}

mt_s32 MT_DRV_HDMI_PlayStus(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *pu32Stutus)
{
	mt_s32   s32Ret = MT_SUCCESS;
	HDMI_PLAYSTAUS_S stPlayStaus;
	stPlayStaus.enHdmi = enHdmi;
	stPlayStaus.u32PlayStaus = MT_FALSE;

	s32Ret = hdmi_ExtIoctl(CMD_HDMI_GET_HDMI_PLAYSTAUS, &stPlayStaus);

	if ( MT_SUCCESS  != s32Ret) {
		return s32Ret;
	}

	*pu32Stutus = stPlayStaus.u32PlayStaus;
	return s32Ret;
}

mt_s32 MT_DRV_AO_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr)
{
	mt_s32   s32Ret = MT_SUCCESS;
	HDMI_GETAOATTR_S stGetAOAttr;
	stGetAOAttr.enHdmi = enHdmi;

	s32Ret = hdmi_ExtIoctl(CMD_HDMI_GET_AO_ATTR, &stGetAOAttr);
	if ( MT_SUCCESS  != s32Ret) {
		return s32Ret;
	}
	*pstHDMIAOAttr = stGetAOAttr.stAudAttr;
	return s32Ret;
}

mt_s32 MT_DRV_HDMI_GetSinkCapability(MT_UNF_HDMI_ID_E enHdmi, MT_UNF_EDID_BASE_INFO_S *pstSinkCap)
{
	#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	mt_s32   s32Ret = MT_SUCCESS;
	HDMI_SINK_CAPABILITY_S stSinkcap;
	memset(&stSinkcap, 0, sizeof(HDMI_SINK_CAPABILITY_S));
	stSinkcap.enHdmi = enHdmi;
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_SINK_CAPABILITY, &stSinkcap);
	*pstSinkCap = stSinkcap.SinkCap;
	return s32Ret;
	#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/

	MT_UNF_EDID_BASE_INFO_S *pSinkCap = DRV_Get_SinkCap(MT_UNF_HDMI_ID_0);
	memcpy(pstSinkCap, pSinkCap, sizeof(MT_UNF_EDID_BASE_INFO_S));

	if (!DRV_Get_IsValidSinkCap(MT_UNF_HDMI_ID_0)) {
		COM_INFO("Invalid Sink capability \n");
		return MT_FAILURE;
	} else {
		return MT_SUCCESS;
	}
}

mt_s32 MT_DRV_HDMI_GetAudioCapability(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_HDMI_AUDIO_CAPABILITY_S *pstAudCap)
{
	MT_DRV_HDMI_AUDIO_CAPABILITY_S *pOldAudioCap = DRV_Get_OldAudioCap();

	if (DRV_Get_IsValidSinkCap(enHdmi)) {
		memcpy(pstAudCap, pOldAudioCap, sizeof(MT_DRV_HDMI_AUDIO_CAPABILITY_S));
		return MT_SUCCESS;
	} else {
		return MT_FAILURE;
	}
}

mt_s32 MT_DRV_HDMI_SetAudioMute(MT_UNF_HDMI_ID_E enHdmi)
{
	SI_SetHdmiAudio(MT_FALSE);
	return MT_SUCCESS;
}

mt_s32 MT_DRV_HDMI_SetAudioUnMute(MT_UNF_HDMI_ID_E enHdmi)
{
	//SI_SetHdmiAudio(MT_TRUE);
	HDMI_APP_ATTR_S *pstAppAttr = DRV_Get_AppAttr(enHdmi);

	SI_SetHdmiAudio(pstAppAttr->bEnableAudio);
	return MT_SUCCESS;
}

mt_s32 MT_DRV_HDMI_AudioChange(MT_UNF_HDMI_ID_E enHdmi, HDMI_AUDIO_ATTR_S *pstHDMIAOAttr)
{
	mt_s32 s32Ret = MT_SUCCESS;
	HDMI_GETAOATTR_S stAoAttr;

	stAoAttr.enHdmi = enHdmi;
	memcpy(&stAoAttr.stAudAttr, pstHDMIAOAttr, sizeof(HDMI_AUDIO_ATTR_S));
	//stAudAttr.stAudAttr = *pstHDMIAOAttr;
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_AUDIO_CHANGE, &stAoAttr);

	if (HDMI_AUDIO_INTERFACE_SPDIF == pstHDMIAOAttr->enSoundIntf) { //fixbug32218, rootcause is hide
		msleep(50);
	}

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_PreFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enEncodingFormat)
{
	mt_s32 s32Ret = MT_SUCCESS;
	HDMI_PREVIDEOTIMING_S stPreVidTiming;

	stPreVidTiming.enHdmi = enHdmi;
	stPreVidTiming.VideoTiming = enEncodingFormat;
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_PREVTIMING, &stPreVidTiming);

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_SetFormat(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo)
{
	mt_s32 s32Ret = MT_SUCCESS;
	HDMI_VIDEOTIMING_S stVideoTiming;

	stVideoTiming.enHdmi = enHdmi;
	stVideoTiming.VideoTiming = enFmt;
	stVideoTiming.enStereo = enStereo;
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_VIDEO_TIMING, &stVideoTiming);

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_Detach(MT_UNF_HDMI_ID_E enHdmi)
{
	mt_s32 s32Ret = MT_SUCCESS;

	DRV_Set_ThreadStop(MT_TRUE);
	s32Ret = MT_DRV_HDMI_PreFormat(enHdmi, MT_DRV_DISP_FMT_BUTT);

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_Attach(MT_UNF_HDMI_ID_E enHdmi, MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo)
{
	mt_s32 s32Ret = MT_SUCCESS;

	s32Ret = MT_DRV_HDMI_SetFormat(enHdmi, enFmt, enStereo);
	DRV_Set_ThreadStop(MT_FALSE);

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_GetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_APP_ATTR_S *pstAttr)
{
	mt_s32 s32Ret = MT_SUCCESS;
	HDMI_PORT_ATTR_S stPortAttr;
	memset(&stPortAttr, 0, sizeof(HDMI_PORT_ATTR_S));

	stPortAttr.enHdmi = enHdmi;
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_GET_ATTR, &stPortAttr);
	*pstAttr = stPortAttr.stHdmiAppAttr;
	return s32Ret;
}

mt_s32 MT_DRV_HDMI_SetAttr(MT_UNF_HDMI_ID_E enHdmi, HDMI_APP_ATTR_S *pstAttr)
{
	mt_s32 s32Ret = MT_SUCCESS;
	HDMI_PORT_ATTR_S stPortAttr;

	stPortAttr.enHdmi = enHdmi;
	stPortAttr.stHdmiAppAttr = *pstAttr;
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_SET_ATTR, &stPortAttr);
	return s32Ret;
}

//#ifndef MODULE

mt_s32 hdmi_SoftResume(MT_DRV_DISP_FMT_E enFmt, MT_DRV_DISP_STEREO_E enStereo)
{
	HDMI_VIDEO_ATTR_S   *pstVidAttr = DRV_Get_VideoAttr(MT_UNF_HDMI_ID_0);
	MT_DRV_DISP_FMT_E   enEncodingFormat = enFmt;

	if ((enEncodingFormat == MT_DRV_DISP_FMT_PAL) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_B) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_B1) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_D) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_D1) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_G) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_H) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_K) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_I) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_M) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_N) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_Nc) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_PAL_60) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_1440x576i_50) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_SIN) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_COS) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_L) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_B) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_G) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_D) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_K) ||
			(enEncodingFormat == MT_DRV_DISP_FMT_SECAM_H)) {
		enEncodingFormat = MT_DRV_DISP_FMT_PAL;
	} else if ((enEncodingFormat == MT_DRV_DISP_FMT_NTSC) ||
			   (enEncodingFormat == MT_DRV_DISP_FMT_NTSC_J) ||
			   (enEncodingFormat == MT_DRV_DISP_FMT_1440x480i_60) ||
			   (enEncodingFormat == MT_DRV_DISP_FMT_NTSC_443)) {
		enEncodingFormat = MT_DRV_DISP_FMT_NTSC;
	} else if (enEncodingFormat == MT_DRV_DISP_FMT_1080P_24_FP) {
		enEncodingFormat = MT_DRV_DISP_FMT_1080P_24;
	} else if (enEncodingFormat == MT_DRV_DISP_FMT_720P_60_FP) {
		enEncodingFormat = MT_DRV_DISP_FMT_720P_60;
	} else if (enEncodingFormat == MT_DRV_DISP_FMT_720P_50_FP) {
		enEncodingFormat = MT_DRV_DISP_FMT_720P_50;
	}

	pstVidAttr->b3DEnable = MT_TRUE;
	if (DISP_STEREO_FPK == enStereo) {
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_FRAME_PACKETING;
	} else if (DISP_STEREO_SBS_HALF == enStereo) {
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_SIDE_BY_SIDE_HALF;
	} else if (DISP_STEREO_TAB == enStereo) {
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_TOP_AND_BOTTOM;
	} else {
		pstVidAttr->b3DEnable = MT_FALSE;
		pstVidAttr->u83DParam = MT_UNF_EDID_3D_BUTT;
	}

	COM_INFO("FMT:%d,3DFlag:%d, 3dParm:%d\n", enEncodingFormat, pstVidAttr->b3DEnable, pstVidAttr->u83DParam);
	pstVidAttr->enVideoFmt = enEncodingFormat;

	suspend_flag = 0;

	return MT_SUCCESS;
}

mt_s32 MT_DRV_HDMI_Video_Cofig(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param)
{
	mt_s32 s32Ret = MT_SUCCESS;
	s32Ret = DRV_HDMI_Set_Video(p_param, enHdmi);
	return s32Ret;
}

mt_s32 MT_DRV_HDMI_Notify_register(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param)
{
	mt_s32 s32Ret = MT_SUCCESS;

	s32Ret = hdmi_ExtIoctl(CMD_HDMI_SET_NOTIFY, p_param);
	MT_INFO_HDMI("0x%x\n", s32Ret);

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_Av_Mute(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param)
{
	mt_s32 s32Ret = MT_SUCCESS;

	//MT_INFO_HDMI("MT_DRV_HDMI_Av_Mute, line[%d]\n", __LINE__);
	s32Ret = hdmi_ExtIoctl(CMD_HDMI_SET_AVMUTE, p_param);
	//MT_INFO_HDMI("-----------------------\n");

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_Clk_Cfg(MT_UNF_HDMI_ID_E enHdmi, mt_u32 *p_param)
{
	mt_s32 s32Ret = MT_SUCCESS;
	HDMI_CLK_CFG_S *clk_cfg_info;

	//MT_INFO_HDMI("MT_DRV_HDMI_Clk_Cfg, line[%d]\n", __LINE__);
	clk_cfg_info =  (HDMI_CLK_CFG_S*)p_param;
	s32Ret = DRV_HDMI_Clk_Cfg(clk_cfg_info->enHdmi, clk_cfg_info);
	//MT_INFO_HDMI("-----------------------\n");

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_EMP_Cfg(MT_UNF_HDMI_ID_E enHdmi, void *p_param, mt_u32 len)
{
	mt_s32 s32Ret = MT_SUCCESS;

	//MT_INFO_HDMI("MT_DRV_HDMI_EMP_Cfg, line[%d]\n", __LINE__);
	s32Ret = DRV_HDMI_Emp_Cfg(enHdmi, (void *)p_param, len);
	//MT_INFO_HDMI("-----------------------\n");

	return s32Ret;
}

mt_s32 MT_DRV_HDMI_Vcfg_Check(MT_UNF_HDMI_ID_E enHdmi, void *param_in, void *param_out)
{
	mt_s32 s32Ret = 0;
	s32Ret = DRV_HDMI_Vcfg_Check(enHdmi, param_in, param_out);

	return s32Ret;
}


EXPORT_SYMBOL(MT_DRV_HDMI_EMP_Cfg);
EXPORT_SYMBOL(HDMI_DRV_Init);
EXPORT_SYMBOL(HDMI_DRV_EXIT);
EXPORT_SYMBOL(hdmi_Open);
EXPORT_SYMBOL(hdmi_Close);
EXPORT_SYMBOL(hdmi_Suspend);
EXPORT_SYMBOL(hdmi_Resume);
EXPORT_SYMBOL(hdmi_SoftResume);
EXPORT_SYMBOL(hdmi_Ioctl);
EXPORT_SYMBOL(DRV_HDMI_GetAttr);
//EXPORT_SYMBOL(DRV_ReadByte_8BA);
EXPORT_SYMBOL(DRV_HDMI_GetSinkCapability);
//EXPORT_SYMBOL(SI_Proc_ReadEDIDBlock);
EXPORT_SYMBOL(MT_DRV_HDMI_ExtIoctl);
EXPORT_SYMBOL(MT_DRV_HDMI_GetCECAddr);
EXPORT_SYMBOL(MT_DRV_HDMI_ProcHotPlug);
EXPORT_SYMBOL(MT_DRV_HDMI_GetBinInfoFrame);
#if defined (CEC_SUPPORT)
	EXPORT_SYMBOL(DRV_HDMI_CECStatus);
	EXPORT_SYMBOL(MT_DRV_HDMI_Open);
	EXPORT_SYMBOL(MT_DRV_HDMI_Close);

#endif
//#endif

#if 0 /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
	//������Ҫ��ģ��һ��ע��
	EXPORT_SYMBOL(MT_DRV_HDMI_Init);
	EXPORT_SYMBOL(MT_DRV_HDMI_Deinit);
	EXPORT_SYMBOL(MT_DRV_HDMI_Open);
	EXPORT_SYMBOL(MT_DRV_HDMI_Close);

	EXPORT_SYMBOL(MT_DRV_HDMI_PlayStus);
	EXPORT_SYMBOL(MT_DRV_AO_HDMI_GetAttr);
	EXPORT_SYMBOL(MT_DRV_HDMI_GetSinkCapability);
	EXPORT_SYMBOL(MT_DRV_HDMI_AudioChange);

	EXPORT_SYMBOL(MT_DRV_HDMI_SetFormat);
	EXPORT_SYMBOL(MT_DRV_HDMI_Set3DMode);
#endif /*--NO MODIFY : COMMENT BY CODINGPARTNER--*/
#endif

