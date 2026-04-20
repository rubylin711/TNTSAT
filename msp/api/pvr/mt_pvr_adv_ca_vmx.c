/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <signal.h>
#include "mt_type.h"
#include "mt_debug.h"
#ifdef CONFIG_MT_CHIP_SYMPHONY1
#include "mt_unf_cipher.h"
#else
#include "mt_unf_cipher_v2.h"
#endif




#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef VMX_ADVCA_PVR


static void utc_time_to_code(utc_time_t *p_time, u8 *p_code)
{
	u16 mjd = 0;
	u8  L=0;
	u16 year = p_time->year - 1900;

	if(p_time->month == 1 || p_time->month == 2)
	{
		L = 1;
	}
	mjd = 14956 + p_time->day + (int)((year - L)*365.25) + (int)((p_time->month + 1 + L*12)*30.6001);
	p_code[0] = (u8)((mjd & 0xff00) >> 8);
	p_code[1] = (u8)(mjd & 0x00ff);
	p_code[2] = ((p_time->hour / 10) << 4) | ( p_time->hour % 10);
	p_code[3] = ((p_time->minute/ 10) << 4) | ( p_time->minute % 10);
	p_code[4] = ((p_time->second / 10) << 4) | ( p_time->second % 10);
}


MT_S32 MT_UNF_PVR_AdvCaInit(void)
{
	vmxAdvCaData_S *gbVmxAdvCaData = NULL;
       if(g_pAdvCaPvrDev == NULL)
       {
		g_pAdvCaPvrDev = (vmxAdvCaData_S *)malloc(sizeof(vmxAdvCaData_S));	
		memset(g_pAdvCaPvrDev, 0, sizeof(vmxAdvCaData_S));
       }
	   
	gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
	gbVmxAdvCaData->vmxAdvCaStartDescramblingCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVRRecordCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaDVRStopCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVREncryptCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaDVRDecryptCallBack = NULL;	 

	gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[1]);
	gbVmxAdvCaData->vmxAdvCaStartDescramblingCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVRRecordCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaDVRStopCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVREncryptCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaDVRDecryptCallBack = NULL;		   
	return MT_SUCCESS;
}


/*  data encrypt
MT_BC_DVREncrypt( uint8_t bChannelId,
                       uint8_t* pabDest,
                       uint8_t* pabSource,
                       uint32_t lSize,
                       uint8_t* pabStoreInfo,
                       uint16_t *pwStoreInfoLen);
int16_t MT_BC_DVRDecrypt( uint8_t bChannelId,
                       uint8_t* pabDest,
                       uint8_t* pabSource,
                       uint32_t lSize )
*/

RET_CODE advCaVMX_WriteCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
										u8 *pu8DestVirAddr,  u32 u32DestPhyAddr, 
										u8 *pu8SrcDataVirAddr, u32 u32SrcDataPhyAddr, 
										u32 u32Offset, 
										u32 u32DataSize)

{
	s32 ret = 0;
       u8 pabStoreInfo[ADVCA_PVR_INFO_LEN] = {0};
	u16 pwStoreInfoLen = 0;
	
	vmxAdvCaData_S *gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
	if(gbVmxAdvCaData->vmxAdvCaDVREncryptCallBack == NULL)   
	{
		return MT_FAILURE;
	}
	PVR_PRINTF("%s  pstDataAttr->u32ChnID=%d pabDesVirtAddr=0x%x,u32DestPhyAddr=0x%x, pu8SrcDataVirAddr=0x%x, u32SrcDataPhyAddr=0x%x, u32DataSize=0x%x\n", 
		__FUNCTION__,pstDataAttr->u32ChnID, pu8DestVirAddr, u32DestPhyAddr, pu8SrcDataVirAddr,u32SrcDataPhyAddr,u32DataSize);
	if((u32DataSize + u32Offset) % 16 != 0 )
	{
		PVR_PRINTF("Error >> %s  %d \n", __FUNCTION__,__LINE__);
		return  MT_FAILURE;
	}
	ret =mt_vmx_pvr_set_address(pu8DestVirAddr,  u32DestPhyAddr, 0);
	
	ret |=mt_vmx_pvr_set_address(pu8SrcDataVirAddr,  u32SrcDataPhyAddr, 1);
	
	ret |= gbVmxAdvCaData->vmxAdvCaDVREncryptCallBack(gbVmxAdvCaData->caChannelId, pu8DestVirAddr, pu8SrcDataVirAddr,  u32DataSize,  pabStoreInfo,  &pwStoreInfoLen);
	if(ret == MT_SUCCESS)
	{
		if(pwStoreInfoLen != 0 )
		{
			int i=0;
			PVRAddrU64 pvrAddr;
			u8 *caData = malloc(pwStoreInfoLen + 16);
			if(caData == NULL) 
			{
				PVR_PRINTF("Error >> %s  %d \n", __FUNCTION__,__LINE__);
				return  MT_FAILURE;
			}
			memset(caData, 0, (pwStoreInfoLen + 16));
			PVR_PRINTF("gbVmxAdvCaData->caDatafilepath = %s  pabInfo_len = %d ---------save ca data \n", gbVmxAdvCaData->caDatafilepath,pwStoreInfoLen);
			for(i=0; i<pwStoreInfoLen;i++)  PVR_PRINTF("%02x ",pabStoreInfo[i]);
			PVR_PRINTF("\n%s  %d ---------save ca data \n", __FUNCTION__,__LINE__);

			pvrAddr.u64Addr = pstDataAttr->u64FileReadOffset;
			memcpy(caData, pvrAddr.u8Addr, 8);
			caData[8] = (pwStoreInfoLen << 8 ) & 0xff;
			caData[9] = pwStoreInfoLen  & 0xff;
			memcpy(&caData[10], pabStoreInfo, pwStoreInfoLen);
			PVR_PRINTF("pvrAddr.u64Addr=%llx, %llx, len=%x,%x\n",pvrAddr.u64Addr,pstDataAttr->u64FileReadOffset,caData[8],caData[9]);
			ret = MT_UNF_PVR_SetCAData(gbVmxAdvCaData->caDatafilepath, caData, (pwStoreInfoLen + 10), 1);
			if(ret != MT_SUCCESS)
			{
				PVR_PRINTF("Error >> %s  %d \n", __FUNCTION__,__LINE__);
				return  MT_FAILURE;
			}
			else
			{
				PVR_PRINTF("Success ---------save ca data\n");
			}
		}
	}
	else
	{
		PVR_PRINTF("%s  %d \n", __FUNCTION__,__LINE__);
	}
	
	PVR_PRINTF("%s  %d , ret=0x%x\n", __FUNCTION__,__LINE__,ret);

	return MT_SUCCESS;
}

RET_CODE advCaVMX_ReadCallback(MT_UNF_PVR_DATA_ATTR_S *pstDataAttr, 
										u8 *pu8DestVirAddr,  u32 u32DestPhyAddr, 
										u8 *pu8SrcDataVirAddr, u32 u32SrcDataPhyAddr, 
										u32 u32Offset, 
										u32 u32DataSize)
{
	s32 ret = 0;
	u16 pwStoreInfoLen = 0;
	u8 caData[ADVCA_PVR_INFO_LEN] = {0};
	int index=0;
	vmxAdvCaData_S *gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);

	if(gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack == NULL)   
	{
		return MT_FAILURE;
	}
	if(gbVmxAdvCaData->vmxAdvCaDVRDecryptCallBack== NULL)   
	{
		return MT_FAILURE;
	}
	PVR_PRINTF("%s  gbVmxAdvCaData->infoLen=%d pabDesVirtAddr=0x%x,u32DestPhyAddr=0x%x, pu8SrcDataVirAddr=0x%x, u32SrcDataPhyAddr=0x%x, u32DataSize=0x%x\n", 
		__FUNCTION__,gbVmxAdvCaData->infoLen, pu8DestVirAddr, u32DestPhyAddr, pu8SrcDataVirAddr,u32SrcDataPhyAddr,u32DataSize);
	if((u32DataSize + u32Offset) % 16 != 0 )
	{
		PVR_PRINTF("Error >> %s  %d \n", __FUNCTION__,__LINE__);
		return  MT_FAILURE;
	}
	if(gbVmxAdvCaData->infoLen == 0)
	{
		
		ret = MT_UNF_PVR_GetCAData(gbVmxAdvCaData->caDatafilepath, caData, ADVCA_PVR_INFO_LEN, &pwStoreInfoLen);
		if(MT_SUCCESS != ret)
		{
			PVR_PRINTF("MT_UNF_PVR_GetCAData >> %s  %d \n", __FUNCTION__,__LINE__);
		}
		else
		{
			int offset=0;
			{
				int x=0;
				PVR_PRINTF("\n==================================\n");
				for(x=0; x<pwStoreInfoLen;x++)   PVR_PRINTF("%02x ",caData[x]);
				PVR_PRINTF("\n==================================\n");
			}
			gbVmxAdvCaData->storeInfoCount = 0;
			gbVmxAdvCaData->blocksize = 0;    //4 //4byte
			gbVmxAdvCaData->infoLen = (caData[4] << 8) + caData[5];
			gbVmxAdvCaData->info = (u8 *)malloc(gbVmxAdvCaData->infoLen);
			memcpy(gbVmxAdvCaData->info,  &caData[6], gbVmxAdvCaData->infoLen);
			PVR_PRINTF("gbVmxAdvCaData->infoLen=%x, %x %x\n",gbVmxAdvCaData->infoLen,caData[4],caData[5]);

			offset = (6 + gbVmxAdvCaData->infoLen);
			do
			{
				index = gbVmxAdvCaData->storeInfoCount;
				gbVmxAdvCaData->storeInfo[index].streamPos = *(u64 *)&caData[offset];  //8byte
				gbVmxAdvCaData->storeInfo[index].storeInfoLen = (caData[offset+8] << 8) + caData[offset+9];  //2byte
				PVR_PRINTF("gbVmxAdvCaData->storeInfo[j].storeInfoLen=%x, por=0x%llx\n",gbVmxAdvCaData->storeInfo[index].storeInfoLen,gbVmxAdvCaData->storeInfo[index].streamPos);
				if(gbVmxAdvCaData->storeInfo[index].storeInfoLen < 4096)
				{
					gbVmxAdvCaData->storeInfo[index].storeInfoData  = (u8 *)malloc(gbVmxAdvCaData->storeInfo[index].storeInfoLen);
					memcpy(gbVmxAdvCaData->storeInfo[index].storeInfoData, &caData[offset+10], gbVmxAdvCaData->storeInfo[index].storeInfoLen);
					offset = offset + 10 + gbVmxAdvCaData->storeInfo[index].storeInfoLen;
					gbVmxAdvCaData->storeInfoCount ++;
				}
				else
				{
					printf("CA data error  pwStoreInfoLen=%d\n",pwStoreInfoLen);
				}
			}while( offset < pwStoreInfoLen);
			gbVmxAdvCaData->storteInfoCurrenId = 0;
			
		}
	}
	index = gbVmxAdvCaData->storteInfoCurrenId;
	PVR_PRINTF("gbVmxAdvCaData->caChannelId = 0x%x, fileOffset=%llx, streamPos[%d/%d]=%llx\n",
			gbVmxAdvCaData->caChannelId, pstDataAttr->u64FileReadOffset, index,gbVmxAdvCaData->storeInfoCount, gbVmxAdvCaData->storeInfo[index].streamPos);
	if((index < gbVmxAdvCaData->storeInfoCount)  &&  (pstDataAttr->u64FileReadOffset >= gbVmxAdvCaData->storeInfo[index].streamPos))
	{
		u8 utc_code[5] = {0};
		
		utc_time_t new_time = {0};
		MT_UNF_PVR_PLAY_STATUS_S pstStatus;
  		time_get(&new_time, TRUE);
		utc_time_to_code(&new_time,  utc_code);
		
		gbVmxAdvCaData->storteInfoCurrenId ++;

		
		//ret =  MT_UNF_PVR_PlayGetStatus(gbVmxAdvCaData->caChannelId, &pstStatus);
		if(MT_SUCCESS != ret)
		{
		//	PVR_PRINTF("Error >> %s  %d\n", __FUNCTION__,__LINE__);
		//	return ret;
		}
		//PVR_PRINTF("pstStatus.u64CurPlayPos = 0x%llx\n",pstStatus.u64CurPlayPos);
		/*
		{
			int x=0;
			PVR_PRINTF("\n==================================\n");
			for(x=0; x<gbVmxAdvCaData->infoLen;x++)   PVR_PRINTF("%02x ",gbVmxAdvCaData->info[x]);
			PVR_PRINTF("\n==================================\n");
		}
		{
			int x=0;
			PVR_PRINTF("\n==================================\n");
			for(x=0; x<gbVmxAdvCaData->storeInfo[0].storeInfoLen;x++)   PVR_PRINTF("%02x ",gbVmxAdvCaData->storeInfo[0].storeInfo[x]);
			PVR_PRINTF("\n==================================\n");
		}
		*/
		//memcpy(utc_code, tmp->utc_code, 5);
		PVR_PRINTF("MT_UNF_PVR_GetCAData >> OK  utc_code=%x,%x,%x,%x,%x\n", 	utc_code[0],utc_code[1],utc_code[2],utc_code[3],utc_code[4]);	
		if(index == 0)
		{
			ret = gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack(gbVmxAdvCaData->caChannelId,  
								gbVmxAdvCaData->info, gbVmxAdvCaData->infoLen, 
								gbVmxAdvCaData->storeInfo[index].storeInfoData,  gbVmxAdvCaData->storeInfo[index].storeInfoLen, 
								utc_code); 
		}
		else
		{
			ret = gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack(gbVmxAdvCaData->caChannelId,  
								NULL, 0, 
								gbVmxAdvCaData->storeInfo[index].storeInfoData,  gbVmxAdvCaData->storeInfo[index].storeInfoLen, 
								utc_code); 
		}
		if(MT_SUCCESS != ret)
		{
			PVR_PRINTF("Error >> %s  %d\n", __FUNCTION__,__LINE__);
			return  MT_FAILURE;
		}
		else
		{
			PVR_PRINTF("Success >> %s  %d\n", __FUNCTION__,__LINE__);
		}
	}	
	ret = mt_vmx_pvr_set_address(pu8DestVirAddr,  u32DestPhyAddr, 0);
	ret |= mt_vmx_pvr_set_address(pu8SrcDataVirAddr,  u32SrcDataPhyAddr, 1);
	PVR_PRINTF("MT_BC_DVRDecrypt  >>> destVirAddr=0x%x, SrcVirAddr=0x%x, DataSize=0x%x\n",  pu8DestVirAddr, pu8SrcDataVirAddr,  u32DataSize);
	ret |= gbVmxAdvCaData->vmxAdvCaDVRDecryptCallBack(gbVmxAdvCaData->caChannelId, pu8DestVirAddr, pu8SrcDataVirAddr,  u32DataSize);
	if(MT_SUCCESS == ret)
	{
		PVR_PRINTF("Success >> %s  %d \n", __FUNCTION__,__LINE__);
	}
	else
	{
		PVR_PRINTF("Failure >> %s  %d, ret=%d\n", __FUNCTION__,__LINE__,ret);
	}
		
	PVR_PRINTF("%s  %d , ret=0x%x\n", __FUNCTION__,__LINE__,ret);

	return MT_SUCCESS;
}




static MT_S32 MT_UNF_PVR_AdvCaStartDescrambler(mt_u32 u32DmxId, mt_u16 v_pid, mt_u16 a_pid, u8 dectype)
{
    mt_u32 Ret = 0;
    MT_UNF_DMX_DESCRAMBLER_ATTR_S stcaDescAttr;
    mt_handle hDescrambler;
    MT_HANDLE hAvPlay;
    mt_handle hcaVidDesc, hcaAudDesc;
    mt_handle hDmxVidChn, hDmxAudChn;	
    u8 VID_ODD_KEYSLOTID = 0;
    u8 VID_EVEN_KEYSLOTID = 0;
    u8 AUD_ODD_KEYSLOTID = 0;
    u8 AUD_EVEN_KEYSLOTID = 0;
    vmxAdvCaData_S *gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
	
    memset(&stcaDescAttr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_ATTR_S));

    PVR_PRINTF("%s  dectype = %d\n", __FUNCTION__,dectype);	
    if(dectype >> 5)
	{
        if(dectype & 0x4)
		{
            stcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_HEAD_CLEAR;
		}
		else
		{
            stcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_TAIL_CLEAR;
		}
		
	}
       else
       {
        	stcaDescAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2;
	}
	printf("MT_UNF_PVR_AdvCaStartDescrambler >>> stcaDescAttr.enDescramblerType = %x\n",stcaDescAttr.enDescramblerType);
	stcaDescAttr.enCaType = MT_UNF_DMX_CA_ADVANCE;
	Ret = MT_UNF_DMX_CreateDescramblerExt(u32DmxId, &stcaDescAttr, &hcaVidDesc);
	if(MT_SUCCESS != Ret)
	{
	PVR_PRINTF("%s  %d\n", __FUNCTION__,__LINE__);
		return 0;
	}

	Ret = MT_UNF_DMX_CreateDescramblerExt(u32DmxId, &stcaDescAttr, &hcaAudDesc);
	if(MT_SUCCESS != Ret)
	{
	PVR_PRINTF("%s  %d\n", __FUNCTION__,__LINE__);
		return  0;
	}
	PVR_PRINTF("%s  %d\n", __FUNCTION__,__LINE__);

	Ret = MT_UNF_DMX_GetChannelHandleByPidType(u32DmxId, v_pid,  MT_UNF_DMX_CHAN_TYPE_REC, &hDmxVidChn);
	Ret |= MT_UNF_DMX_GetChannelHandleByPidType(u32DmxId, a_pid,  MT_UNF_DMX_CHAN_TYPE_REC, &hDmxAudChn); 
	PVR_PRINTF("%s  %d , hDmxVidChn=0x%x, hDmxAudChn=0x%x\n", __FUNCTION__,__LINE__,hDmxVidChn, hDmxAudChn);
	Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxVidChn, &hDescrambler);
	if (MT_SUCCESS != Ret) 
	{
		if (MT_ERR_DMX_NOATTACH_KEY == Ret) 	
		{
			(mt_void) MT_UNF_DMX_AttachDescrambler(hcaVidDesc, hDmxVidChn);
		}
		else
		{
			printf("MT_UNF_DMX_GetDescramblerKeyHandle failed:%x\n", Ret);
			return  0;
		}

	}
	else
	{
		if ((hDescrambler != hcaVidDesc)) 
		{
			(mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxVidChn);
			(mt_void) MT_UNF_DMX_AttachDescrambler(hcaVidDesc, hDmxVidChn);
		}
	}
	
	Ret = MT_UNF_DMX_GetDescramblerKeyHandle(hDmxAudChn, &hDescrambler);
	if (MT_SUCCESS != Ret) 
	{
		if (MT_ERR_DMX_NOATTACH_KEY == Ret) 
		{
			(mt_void) MT_UNF_DMX_AttachDescrambler(hcaAudDesc, hDmxAudChn);
		}
		else
		{
			printf("MT_UNF_DMX_GetDescramblerKeyHandle failed:%x\n", Ret);
			return  0;
		}
	} 
	else 
	{
		if ((hDescrambler != hcaAudDesc)) 
		{
			(mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, hDmxAudChn);
			(mt_void) MT_UNF_DMX_AttachDescrambler(hcaAudDesc, hDmxAudChn);
		}
	}

	Ret =  mt_vmx_keyslot_request(&VID_ODD_KEYSLOTID);
	Ret |= mt_vmx_keyslot_request(&VID_EVEN_KEYSLOTID);
	Ret |= mt_vmx_keyslot_request(&AUD_ODD_KEYSLOTID);
	Ret |= mt_vmx_keyslot_request(&AUD_EVEN_KEYSLOTID);
	if (MT_SUCCESS != Ret) 
	{
		return 0;
	}
	MT_UNF_DMX_SetDescramblerOddKeySlot(hcaVidDesc, VID_ODD_KEYSLOTID);
	MT_UNF_DMX_SetDescramblerEvenKeySlot(hcaVidDesc, VID_EVEN_KEYSLOTID);

	MT_UNF_DMX_SetDescramblerOddKeySlot(hcaAudDesc, AUD_ODD_KEYSLOTID);
	MT_UNF_DMX_SetDescramblerEvenKeySlot(hcaAudDesc, AUD_EVEN_KEYSLOTID);
	gbVmxAdvCaData->keyslot[0] = VID_ODD_KEYSLOTID;
	gbVmxAdvCaData->keyslot[1] = VID_EVEN_KEYSLOTID;
	gbVmxAdvCaData->keyslot[2] = AUD_ODD_KEYSLOTID;
	gbVmxAdvCaData->keyslot[3] = AUD_EVEN_KEYSLOTID;
	gbVmxAdvCaData->hcaAudDesc = hcaAudDesc;
	gbVmxAdvCaData->hcaVidDesc =  hcaVidDesc;
	gbVmxAdvCaData->hDmxAudChn = hDmxAudChn;
	gbVmxAdvCaData->hDmxVidChn = hDmxVidChn;
	
	Ret = (VID_ODD_KEYSLOTID << 24 ) | (VID_EVEN_KEYSLOTID << 16) | (AUD_ODD_KEYSLOTID << 8 ) | AUD_EVEN_KEYSLOTID;
	PVR_PRINTF("ret = 0x%x, %x %x , %x, %x\n", Ret, VID_ODD_KEYSLOTID,VID_EVEN_KEYSLOTID,AUD_ODD_KEYSLOTID, AUD_EVEN_KEYSLOTID);

	return  Ret;
}


//static RET_CODE mpvr_rec_create_chn_cancel_descrambler(int id)
static MT_S32 MT_UNF_PVR_AdvCaStopDescrambler(int id)
{
    mt_s32 Ret;
    mt_handle hDescrambler;
    vmxAdvCaData_S *gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[id]);

     printf("%s=====%d\n",__FUNCTION__,__LINE__);
	
    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(gbVmxAdvCaData->hDmxVidChn, &hDescrambler);
    if (MT_SUCCESS == Ret)          (mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, gbVmxAdvCaData->hDmxVidChn);
    Ret = MT_UNF_DMX_GetDescramblerKeyHandle(gbVmxAdvCaData->hDmxAudChn, &hDescrambler);
    if (MT_SUCCESS == Ret)            (mt_void) MT_UNF_DMX_DetachDescrambler(hDescrambler, gbVmxAdvCaData->hDmxAudChn);
	
    (mt_void) MT_UNF_DMX_DestroyDescrambler(gbVmxAdvCaData->hcaVidDesc);
    (mt_void) MT_UNF_DMX_DestroyDescrambler(gbVmxAdvCaData->hcaAudDesc);
    Ret |= mt_vmx_keyslot_release(gbVmxAdvCaData->keyslot[0]);
    Ret |= mt_vmx_keyslot_release(gbVmxAdvCaData->keyslot[1]);
    Ret |= mt_vmx_keyslot_release(gbVmxAdvCaData->keyslot[2]);
    Ret |= mt_vmx_keyslot_release(gbVmxAdvCaData->keyslot[3]);

    return Ret;	
}

MT_S32 MT_UNF_PVR_AdvCaStartRec(MT_U32 u32ChnID)
{

	if((attr.bSupportAdvCa == MT_UNF_PVR_REC_VMX)) 
		//&&  (p_rec_attr->advca_pvr_attr.audio_ecm_pid > 0 &&  p_rec_attr->advca_pvr_attr.audio_ecm_pid < 0x1fff)
		//&&  (p_rec_attr->advca_pvr_attr.video_ecm_pid > 0 &&  p_rec_attr->advca_pvr_attr.video_ecm_pid < 0x1fff))
	{
              //demo code
		u16 StreamPid[2];
          	u16 EcmPid[2];
		u32 key_slot = 0;	
		vmxAdvCaData_S *gbVmxAdvCaData;
		if(p_rec_attr->rec_in == 0)
		{
			g_pAdvCaPvrDev->AdvCaType = VMX_ADV_CA;
			gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
			memset(gbVmxAdvCaData, 0, sizeof(vmxAdvCaData_S));
			gbVmxAdvCaData->pvrChannelId = pChan->chnid;
		}
		else
		{
			g_pAdvCaPvrDev->AdvCaType = VMX_ADV_CA;
			gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[1]);
			memset(gbVmxAdvCaData, 0, sizeof(vmxAdvCaData_S));
			gbVmxAdvCaData->pvrChannelId = pChan->chnid;
		}
		if(gbVmxAdvCaData->vmxAdvCaDVRRecordCallBack == NULL)   
		{
			return MT_FAILURE;
		}
		if(gbVmxAdvCaData->vmxAdvCaStartDescramblingCallBack == NULL)   
		{
			return MT_FAILURE;
		}
		gbVmxAdvCaData->pmtServiceId = p_rec_attr->media_info.extern_pid[2].pid;
		printf("start BC_StartDescrambling  gbVmxAdvCaData->pmtServiceId=0x%x\n",gbVmxAdvCaData->pmtServiceId);
		if(gbVmxAdvCaData->pmtServiceId > 0)
		{
			key_slot = MT_UNF_PVR_AdvCaStartDescrambler(attr.u32DemuxID, 
						p_rec_attr->media_info.v_pid, 
						p_rec_attr->media_info.a_pid,
						p_rec_attr->media_info.extern_pid[0].fmt);
			if(key_slot == 0) 
			{
				PVR_PRINTF("Error :  MT_UNF_PVR_AdvCaStartDescrambler return error !\n");
				return  MT_FAILURE;
			}
			
			printf("ecm_pid =  %x  %x\n",p_rec_attr->media_info.extern_pid[0].pid, p_rec_attr->media_info.extern_pid[1].pid);
			EcmPid[0] = p_rec_attr->media_info.extern_pid[0].pid;
			EcmPid[1] = p_rec_attr->media_info.extern_pid[1].pid;
			StreamPid[0] = (u16)(key_slot >> 16);
			StreamPid[1] = (u16)(key_slot & 0xffff);
			
			gbVmxAdvCaData->caServiceIdx = 1;
			gbVmxAdvCaData->caChannelId = 0;		
		       //last_svc_id;//p_rec_attr->media_info.service_idx;

			printf("start BC_StartDescrambling [%08x %08x] [%08x %08x]  service_idx=%x\n",
				EcmPid[0], EcmPid[1], StreamPid[0],StreamPid[1],  p_rec_attr->media_info.extern_pid[2].pid);

			ret = gbVmxAdvCaData->vmxAdvCaStartDescramblingCallBack(gbVmxAdvCaData->pmtServiceId, 2, EcmPid, StreamPid, gbVmxAdvCaData->caServiceIdx);
				
			mtos_task_sleep(100);   //Wait for descrambler ok
			mtos_printk("BC_StartDescrambling ret %d\n",ret);
		}	
		memset(gbVmxAdvCaData->caDatafilepath, 0, MPVR_MAX_FILENAME_LEN);
		sprintf(gbVmxAdvCaData->caDatafilepath,"%s.ca",name_tmp);
		/*
		MT_BC_DVRRecord( uint8_t bServiceIdx,
	                      uint8_t bChannelId,
	                      uint8_t* pabInfo,
	                      uint16_t wInfoLen,
	                      uint8_t * pabActualTime );
	                      */
	       if(p_rec_attr->stream_type != MPVR_STREAM_TYPE_ALL_TS)
	       {
			u8 bServiceIdx = p_rec_attr->media_info.service_idx & 0xff;
			u8 *pabInfo= NULL;

			pabInfo = malloc(sizeof(mpvr_rec_media_t) + 16);
			if(pabInfo == NULL)
			{
				return  MT_FAILURE;
			}
			memset(pabInfo, 0, (sizeof(mpvr_rec_media_t) + 16));
			pabInfo[4] = (sizeof(mpvr_rec_media_t) >> 8) & 0xff;
			pabInfo[5] = sizeof(mpvr_rec_media_t)  & 0xff;
			memcpy(&pabInfo[6], &(p_rec_attr->media_info), sizeof(mpvr_rec_media_t));
			//if(NULL == currentPvrPabInfo)   currentPvrPabInfo = malloc(ADVCA_PVR_INFO_LEN);
			//memcpy(currentPvrPabInfo, &(p_rec_attr->media_info), sizeof(mpvr_rec_media_t));
			
	              // wait descrambler running
			//while(1)
		       //{
			//	MMI_SetDescrambling_State();
		       //}
			printf("MT_BC_DVRRecord start  utc_code=%x,%x,%x,%x,%x\n",utc_code[0],utc_code[1],utc_code[2],utc_code[3],utc_code[4]);
			ret = gbVmxAdvCaData->vmxAdvCaDVRRecordCallBack(gbVmxAdvCaData->caServiceIdx, 0,  &pabInfo[6],   sizeof(mpvr_rec_media_t),  utc_code);       
			if(ret == 0) 
			{
				ret = MT_UNF_PVR_SetCAData(gbVmxAdvCaData->caDatafilepath, pabInfo, (sizeof(mpvr_rec_media_t) + 6),  0);
				if(ret != MT_SUCCESS)
				{
					PVR_PRINTF("Error >> %s  %d \n", __FUNCTION__,__LINE__);
					return  MT_FAILURE;
				}
				printf("MT_BC_DVRRecord Success !\n");
				
			}
			else
			{
				printf("MT_BC_DVRRecord Failure !\n");
				return  MT_FAILURE;
				
			}
			if(pabInfo)  free(pabInfo);
			MT_UNF_PVR_RegisterExtraCallback(pChan->chnid, MT_UNF_PVR_EXTRA_WRITE_CALLBACK, advCaVMX_WriteCallback, NULL);
	       }
	}
}

MT_S32 MT_UNF_PVR_AdvCaStopRec(MT_U32 u32ChnID)
{
	vmxAdvCaData_S *gbVmxAdvCaData;

	if(g_pAdvCaPvrDev)
	{
		int id = 0;
		if(u32ChnID == g_pAdvCaPvrDev->advPvrInfo[0].pvrChannelId)
		{
			gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
			if(gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack == NULL)   
			{
				return MT_FAILURE;
			}			
			ret = gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack(gbVmxAdvCaData->pmtServiceId, 
																	gbVmxAdvCaData->caServiceIdx);
			if(ret == 0)
			{
				PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
			}
			else
			{
				PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
			}

		}
		else
		{
			id = 1;
			gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[1]);
			if(gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack == NULL)   
			{
				return MT_FAILURE;
			}
			ret = gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack(gbVmxAdvCaData->pmtServiceId, 
																	gbVmxAdvCaData->caServiceIdx);
			if(ret == 0)
			{
				PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
			}
			else
			{
				PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
			}
		}

		if(gbVmxAdvCaData->vmxAdvCaDVRStopCallBack == NULL)   
		{
			return MT_FAILURE;
		}
	       ret = MT_UNF_PVR_UnRegisterExtraCallBack(u32ChnID, MT_UNF_PVR_EXTRA_WRITE_CALLBACK);
		if(ret == 0)
		{
			 PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
		}
		else
		{
			 PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
		}
	
		MT_UNF_PVR_AdvCaStopDescrambler(id);
			
		ret = gbVmxAdvCaData->vmxAdvCaDVRStopCallBack(0);
		if(ret == 0)
		{
			 PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
		}
		else
		{
			 PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
		}
	}
	return MT_SUCCESS;
}

MT_S32 MT_UNF_PVR_AdvCaStartPlay(MT_U32 u32ChnID)
{
	if(1)
	{
		char attrfilepath[64];
		mpvr_rec_attr_t rec_attr = {0};
		int dataRead=0;
		vmxAdvCaData_S *gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
		
		memset(gbVmxAdvCaData->caDatafilepath, 0, MPVR_MAX_FILENAME_LEN);
		sprintf(gbVmxAdvCaData->caDatafilepath,"%s.ca",name_tmp);

  	 	PVR_PRINTF("%s  %d  gbVmxAdvCaData->caChannelId = %d\n", __FUNCTION__,__LINE__,gbVmxAdvCaData->caChannelId);
		MT_UNF_PVR_RegisterExtraCallback(pChan->chnid, MT_UNF_PVR_EXTRA_READ_CALLBACK, advCaVMX_ReadCallback, NULL);
	}
	return MT_SUCCESS;
}


MT_S32 MT_UNF_PVR_AdvCaStopPlay(MT_U32 u32ChnID)
{
	if(1)
	{
		vmxAdvCaData_S *gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[0]);
		if(gbVmxAdvCaData->vmxAdvCaDVRStopCallBack == NULL)   
		{
			return MT_FAILURE;
		}
		if(gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack == NULL)   
		{
			return MT_FAILURE;
		}
		ret = MT_UNF_PVR_UnRegisterExtraCallBack(pChan->chnid, MT_UNF_PVR_EXTRA_READ_CALLBACK);
		if(ret == 0)
		{
			 PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
		}
		else
		{
			 PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
		}
		
		ret = gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack(gbVmxAdvCaData->pmtServiceId, gbVmxAdvCaData->caServiceIdx);
		if(ret == 0)
		{
			 PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
		}
		else
		{
			 PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
		}
		ret = gbVmxAdvCaData->vmxAdvCaDVRStopCallBack(1);
		if(ret == 0)
		{
			 PVR_PRINTF("Success : %s  %d \n", __FUNCTION__,__LINE__);
		}
		else
		{
			 PVR_PRINTF("Failure : %s  %d \n", __FUNCTION__,__LINE__);
		}
		//if(NULL != currentPvrPabInfo) 
		//{
		//	free(currentPvrPabInfo);
		//	currentPvrPabInfo = NULL;
		//}
		if(gbVmxAdvCaData->infoLen > 0)
		{
			free(gbVmxAdvCaData->info);
			
			memset(gbVmxAdvCaData, 0, sizeof(vmxAdvCaData_S));
		}
	}
	return MT_SUCCESS;
}


MT_S32 MT_UNF_PVR_RegisterAdvCaFuncCallBack(int id,
														caStartDescrambleCallBack           vmxAdvCaStartDescramblingCallBack,
														caStopDescrambleCallBack            vmxAdvCaStopDescramblingCallBack,
														caDVRRecordCallBack           vmxAdvCaDVRRecordCallBack,
														caDVRReplayCallBack           vmxAdvCaDVRReplayCallBack,
														caDVRStopCallBack           vmxAdvCaDVRStopCallBack,
														caDVREncryptCallBack           vmxAdvCaDVREncryptCallBack,
														caDVRDecryptCallBack           vmxAdvCaDVRDecryptCallBack)
{
	vmxAdvCaData_S *gbVmxAdvCaData = NULL;
	if(g_pAdvCaPvrDev == NULL)  
	{
		return MT_FAILURE;
	}
	gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[id]);
	if(vmxAdvCaStartDescramblingCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaStartDescramblingCallBack = vmxAdvCaStartDescramblingCallBack;
	}
	else
	{
		return MT_FAILURE;
	}
	if(vmxAdvCaStopDescramblingCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack = vmxAdvCaStopDescramblingCallBack;
	}
	else
	{
		return MT_FAILURE;
	}	
	if(vmxAdvCaDVRRecordCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaDVRRecordCallBack = vmxAdvCaDVRRecordCallBack;
	}
	else
	{
		return MT_FAILURE;
	}	
	if(vmxAdvCaDVRReplayCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack = vmxAdvCaDVRReplayCallBack;
	}
	else
	{
		return MT_FAILURE;
	}	
	if(vmxAdvCaDVRStopCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaDVRStopCallBack = vmxAdvCaDVRStopCallBack;
	}
	else
	{
		return MT_FAILURE;
	}
	if(vmxAdvCaDVREncryptCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaDVREncryptCallBack = vmxAdvCaDVREncryptCallBack;
	}
	else
	{
		return MT_FAILURE;
	}	
	if(vmxAdvCaDVRDecryptCallBack)
	{
		gbVmxAdvCaData->vmxAdvCaDVRDecryptCallBack = vmxAdvCaDVRDecryptCallBack;
	}
	else
	{
		return MT_FAILURE;
	}
	
	return MT_SUCCESS;
}


MT_S32 MT_UNF_PVR_UnRegisterAdvCaFuncCallBack(int id	)
{
	vmxAdvCaData_S *gbVmxAdvCaData = NULL;
	if(g_pAdvCaPvrDev == NULL)  
	{
		return MT_FAILURE;
	}
	gbVmxAdvCaData = &(g_pAdvCaPvrDev->advPvrInfo[id]);

	gbVmxAdvCaData->vmxAdvCaStartDescramblingCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaStopDescramblingCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVRRecordCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVRReplayCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaDVRStopCallBack = NULL;
	gbVmxAdvCaData->vmxAdvCaDVREncryptCallBack = NULL;
       gbVmxAdvCaData->vmxAdvCaDVRDecryptCallBack = NULL;	   
	
	
	return MT_SUCCESS;
}



#endif //VMX_ADVCA_PVR

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

