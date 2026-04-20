/******************************************************************************************
  File Name     : avplay_intf.c
  Version       : Initial Draft
  Author        : Montage tech multimedia software group
  Created       : 2016/01/06
  Description   :
  History       :
  1.Date        : 2016/01/06
    Author      :
    Modification: Created file

*******************************************************************************/
#include <linux/vmalloc.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/cpufreq.h>

#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_mmz.h"
#include "mt_drv_avplay.h"
#include "mt_error_mpi.h"
#include "mt_drv_module.h"
#include "mt_module.h"
#include "drv_avplay_ext.h"
#include "mt_kernel_adapt.h"
#include "drv_avplay_ioctl.h"
#include "mt_osal.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

#define AVPLAY_NAME         "MT_AVPLAY"

static mt_device_s          g_AvplayRegisterData;
static atomic_t             g_AvplayCount = ATOMIC_INIT(0);
AVPLAY_GLOBAL_STATE_S       g_AvplayGlobalState;

mt_u8 *g_pAvplayStreamTypeString[3] = {"TS", "ES", "VP"};

mt_u8 *g_pAvplayStatusString[7] = {
    "STOP",
    "PREPLAY",
    "PLAY",
    "TPLAY",
    "PAUSE",
    "EOS",
    "SEEK"
};

mt_u8 *g_pAvplayOverflowString[2] = {
    "RESET",
    "DISCARD",
};

mt_char *g_pAvplayVdecType[MT_UNF_VCODEC_TYPE_BUTT+1] = {
    "MPEG2",
    "MPEG4",
    "AVS",
    "H263",
    "H264",
    "REAL8",
    "REAL9",
    "VC1",
    "VP6",
    "VP6F",
    "VP6A",
    "MJPEG",
    "SORENSON",
    "DIVX3",
    "RAW",
    "JPEG",
    "VP8",
    "MSMPEG4V1",
    "MSMPEG4V2",
    "MSVIDEO1",
    "WMV1",
    "WMV2",
    "RV10",
    "RV20",
    "SVQ1",
    "SVQ3",
    "H261",
    "VP3",
    "VP5",
    "CINEPAK",
    "INDEO2",
    "INDEO3",
    "INDEO4",
    "INDEO5",
    "MJPEGB",
    "MVC",
    "HEVC",
	"DV",
    "BUTT"
};

mt_u8 *g_pAvplayVdecMode[4] = {
    "NORMAL",
    "IP",
    "I",
	"DROP_B",
};

MT_DECLARE_MUTEX(g_AvplayMutex);

static mt_s32 AVPLAY_ProcParsePara(mt_char *pProcPara,mt_char **ppItem,mt_char **ppValue)
{
    mt_char *pChar = MT_NULL;
    mt_char *pItem,*pValue;

    pChar = strchr(pProcPara,'=');
    if (MT_NULL == pChar)
    {
        return MT_FAILURE; /* Not Found '=' */
    }

    pItem = pProcPara;
    pValue = pChar + 1;
    *pChar = '\0';

    /* remove blank bytes from item tail */
    pChar = pItem;
    while(*pChar != ' ' && *pChar != '\0')
    {
        pChar++;
    }
    *pChar = '\0';

    /* remove blank bytes from value head */
    while(*pValue == ' ')
    {
        pValue++;
    }

    *ppItem = pItem;
    *ppValue = pValue;
    return MT_SUCCESS;
}

static mt_s32 AVPLAY_ProcRead(struct seq_file *p, mt_void *v)
{
    mt_proc_entry_t  *pProcItem;
    mt_u32            AvplayId;
    AVPLAY_S          *pAvplay;
    mt_u32            i;
    mt_char           szFrcInRate[16]   = {0};
    mt_char           szFrcOutRate[16]  = {0};
    mt_char           szTplaySpeed[16]  = {0};
    mt_char           szSyncID[16]      = {0};
    mt_char           szDemuxID[16]     = {0};

    pProcItem = p->private;

    AvplayId = (pProcItem->entry_name[6] - '0')*10 + (pProcItem->entry_name[7] - '0');

    pAvplay = g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay;

	if(pAvplay->FrcParamCfg.u32InRate > 100000)
	{
	    mt_osal_snprintf(szFrcInRate, sizeof(szFrcInRate), "%d.%d",
	        pAvplay->FrcParamCfg.u32InRate/10000, (pAvplay->FrcParamCfg.u32InRate/100)%100);
	}
	else
	{
	    mt_osal_snprintf(szFrcInRate, sizeof(szFrcInRate), "%d.%d",
	        pAvplay->FrcParamCfg.u32InRate/100, pAvplay->FrcParamCfg.u32InRate%100);
	}
    mt_osal_snprintf(szFrcOutRate, sizeof(szFrcOutRate), "%d.%d",
        pAvplay->FrcParamCfg.u32OutRate/100, pAvplay->FrcParamCfg.u32OutRate%100);

    mt_osal_snprintf(szTplaySpeed, sizeof(szTplaySpeed), "%d.%d",
        pAvplay->FrcParamCfg.u32PlayRate/256, pAvplay->FrcParamCfg.u32PlayRate % 256 * 100 / 256);

    mt_osal_snprintf(szSyncID, sizeof(szSyncID), "sync%02d", pAvplay->hSync & 0xff);

    if (MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
    {
        mt_osal_snprintf(szDemuxID, sizeof(szDemuxID), "INVALID");
    }
    else
    {
        mt_osal_snprintf(szDemuxID, sizeof(szDemuxID), "%d", pAvplay->AvplayAttr.u32DemuxId);
    }

    PROC_PRINT(p,"----------------------Montage AVPLAY%d Out Info-------------------\n", AvplayId);

    PROC_PRINT(p,
                    "Stream Type           :%-10s   |DmxId                 :%s\n"
                    "CurStatus             :%-10s   |OverflowProc          :%s\n"
                    "Sync ID               :%-10s   |ThreadID              :%d\n"
                    "ThreadScheTimeOutCnt  :%-10u   |ThreadExeTimeOutCnt   :%u\n"
                    "CpuFreqScheTimeCnt    :%-10u\n",
                    g_pAvplayStreamTypeString[pAvplay->AvplayAttr.stStreamAttr.enStreamType],
                    szDemuxID,
                    g_pAvplayStatusString[pAvplay->CurStatus],
                    g_pAvplayOverflowString[pAvplay->OverflowProc],
                    szSyncID,
                    pAvplay->ThreadID,
                    pAvplay->DebugInfo.ThreadScheTimeOutCnt,
                    pAvplay->DebugInfo.ThreadExeTimeOutCnt,
                    pAvplay->DebugInfo.CpuFreqScheTimeCnt
                    );

    PROC_PRINT(p,
                    "------------------------------VID CHANNEL--------------------------\n"
                    "Vid Enable            :%-10s   |Vdec Type             :%s\n"
                    "VidOverflowNum        :%-10u   |Vdec Mode             :%s\n"
                    "VidUnderflowNum       :%-10u   |FrcEnable             :%s\n"
                    "VidPid                :0x%-10x |FrcOutRate            :%s\n"
                    "FrcInRate             :%-10s   |LowDelayEnable        :%s\n"
                    "TplaySpeed            :%-10s   |Vdec ID               :vdec%02lu\n",
                    (pAvplay->VidEnable) ? "TRUE" : "FALSE",
                    g_pAvplayVdecType[pAvplay->VdecAttr.enType],
                    pAvplay->DebugInfo.VidOverflowNum,
                    g_pAvplayVdecMode[pAvplay->VdecAttr.enMode],
                    pAvplay->DebugInfo.VidUnderflowNum,
                    (pAvplay->bFrcEnable) ? "TRUE" : "FALSE",
                    pAvplay->DmxVidPid,                    
                    szFrcOutRate,
                    szFrcInRate,
                    (pAvplay->LowDelayAttr.bEnable) ? "TRUE" : "FALSE",
                    szTplaySpeed,                    
                    pAvplay->hVdec & 0xff
                    );

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        PROC_PRINT(p, "FrameChanID           :vpss_port%04lx->win%04lx(master)\n",
                        pAvplay->MasterFrmChn.hPort & 0xffff,
                        pAvplay->MasterFrmChn.hWindow & 0xffff);

    }

    for (i = 0; i < pAvplay->SlaveChnNum; i++)
    {
        PROC_PRINT(p, "FrameChanID           :vpss_port%04lx->win%04lx(slave%02d)\n",
                        pAvplay->SlaveFrmChn[i].hPort & 0xffff,
                        pAvplay->SlaveFrmChn[i].hWindow & 0xffff,
                        i);
    }

    for (i = 0; i < pAvplay->VirChnNum; i++)
    {
        PROC_PRINT(p, "FrameChanID           :vpss_port%04lx->win%04lx(virtual%02d)\n",
                        pAvplay->VirFrmChn[i].hPort & 0xffff,
                        pAvplay->VirFrmChn[i].hWindow & 0xffff,
                        i);
    }

    PROC_PRINT(p,
                    "AcquireFrame(Try/OK)  :%u/%u\n",
                    pAvplay->DebugInfo.AcquireVidFrameNum,
                    pAvplay->DebugInfo.AcquiredVidFrameNum
                    );

    if (MT_INVALID_HANDLE != pAvplay->MasterFrmChn.hWindow)
    {
        PROC_PRINT(p,
                        "SendFrame(Try/OK)     :%u/%u(master)\n",
                        pAvplay->DebugInfo.MasterVidStat.SendNum,
                        pAvplay->DebugInfo.MasterVidStat.PlayNum +
                        pAvplay->DebugInfo.MasterVidStat.RepeatNum +
                        pAvplay->DebugInfo.MasterVidStat.DiscardNum
                        );
    }

    for (i = 0; i < pAvplay->SlaveChnNum; i++)
    {
        PROC_PRINT(p,
                        "SendFrame(Try/OK)     :%u/%u(slave%02d)\n",
                        pAvplay->DebugInfo.SlaveVidStat[i].SendNum,
                        pAvplay->DebugInfo.SlaveVidStat[i].PlayNum +
                        pAvplay->DebugInfo.SlaveVidStat[i].RepeatNum +
                        pAvplay->DebugInfo.SlaveVidStat[i].DiscardNum,
                        i
                        );
    }

    for (i = 0; i < pAvplay->VirChnNum; i++)
    {
        PROC_PRINT(p,
                        "SendFrame(Try/OK)     :%u/%u(virtual%02d)\n",
                        pAvplay->DebugInfo.VirVidStat[i].SendNum,
                        pAvplay->DebugInfo.VirVidStat[i].PlayNum +
                        pAvplay->DebugInfo.VirVidStat[i].RepeatNum +
                        pAvplay->DebugInfo.VirVidStat[i].DiscardNum,
                        i
                        );
    }

    //PROC_PRINT(p, "\n");

    PROC_PRINT(p,
                  "------------------------------AUD CHANNEL--------------------------\n"
                  "Aud Enable            :%-10s   |Adec Type             :%s\n"
                  "AudOverflowNum        :%-10u   |AdecDelayMs           :%llu\n"
                  "AudUnderflowNum       :%-10u   |DmxAudChnNum          :%u\n",
                  (pAvplay->AudEnable) ? "TRUE" : "FALSE",
                  pAvplay->AdecNameInfo.szHaCodecName,
                  pAvplay->DebugInfo.AudOverflowNum,
                  pAvplay->AdecDelayMs,
                  pAvplay->DebugInfo.AudUnderflowNum,
                  pAvplay->DmxAudChnNum
                  );
#if 0
//mzhu_fpga
    PROC_PRINT(p, "DmxAudPid             :");

    for (i = 0; i < pAvplay->DmxAudChnNum; i++)
    {
        PROC_PRINT(p, "0x%x", pAvplay->DmxAudPid[i]);

        if ((pAvplay->DmxAudChnNum > 1) && (i == pAvplay->CurDmxAudChn))
        {
            PROC_PRINT(p, "(play)");
        }

        if (i < pAvplay->DmxAudChnNum - 1)
        {
            PROC_PRINT(p, ",");
        }
    }

    PROC_PRINT(p, "\n");

    PROC_PRINT(p, "Adec ID               :adec%02ld\n", pAvplay->hAdec & 0xff);

    for (i = 0; i < pAvplay->TrackNum; i++)
    {
        PROC_PRINT(p, "Track ID              :track%02ld", pAvplay->hTrack[i] & 0xff);

        if (pAvplay->hSyncTrack == pAvplay->hTrack[i])
        {
            PROC_PRINT(p, "(master)");
        }

        PROC_PRINT(p, "\n");
    }

    PROC_PRINT(p,
                    "AcquireStream(Try/OK) :%u/%u\n"
                    "SendStream(Try/OK)    :%u/%u\n"
                    "AcquireFrame(Try/OK)  :%u/%u\n",
                    pAvplay->DebugInfo.AcquireAudEsNum,
                    pAvplay->DebugInfo.AcquiredAudEsNum,
                    pAvplay->DebugInfo.SendAudEsNum,
                    pAvplay->DebugInfo.SendedAudEsNum,
                    pAvplay->DebugInfo.AcquireAudFrameNum,
                    pAvplay->DebugInfo.AcquiredAudFrameNum
                    );

    for (i = 0; i < pAvplay->TrackNum; i++)
    {
        PROC_PRINT(p,
                        "SendFrame(Try/OK)     :%u/%u",
                        pAvplay->DebugInfo.SendAudFrameNum,
                        pAvplay->DebugInfo.SendedAudFrameNum
                        );

        if (pAvplay->hSyncTrack == pAvplay->hTrack[i])
        {
            PROC_PRINT(p, "(master)");
        }

        PROC_PRINT(p, "\n");
    }
#endif
    PROC_PRINT(p, "\n");

    return MT_SUCCESS;
}

static mt_void AVPLAY_ProcPrintHelp(mt_void)
{
    mt_drv_proc_echohelp("echo FrcEnable=true|false > /proc/msp/avplayxx, enable or disable frc\n"
          );

    return;
}

static mt_s32 AVPLAY_ProcWrite(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    struct seq_file   *s = file->private_data;
    mt_proc_entry_t  *pProcItem = s->private;
    mt_u32            AvplayId;
    mt_char           ProcPara[64]={0};
    mt_s32            Ret;
    mt_char           *pItemName = MT_NULL;
    mt_char           *pItemValue = MT_NULL;
    AVPLAY_S          *pAvplay = MT_NULL;

    if (copy_from_user(ProcPara, buf, count))
    {
        return -EFAULT;
    }

    ProcPara[sizeof(ProcPara) - 1] = 0;
    Ret = AVPLAY_ProcParsePara(ProcPara, &pItemName, &pItemValue);
    if (MT_SUCCESS != Ret)
    {
        AVPLAY_ProcPrintHelp();
        return -EFAULT;
    }

    AvplayId = (pProcItem->entry_name[6] - '0')*10 + (pProcItem->entry_name[7] - '0');

    if (AvplayId >= AVPLAY_MAX_NUM)
    {
        return -EFAULT;
    }

    pAvplay = g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay;
    if (MT_NULL == pAvplay)
    {
        return -EFAULT;
    }

    /*if (0 == MT_OSAL_Strncmp(pItemName, "FrcEnable", strlen("FrcEnable")))
    {
        if (0 == MT_OSAL_Strncmp(pItemValue, "true", strlen("true")))
        {
            pAvplay->bFrcEnable = MT_TRUE;
        }
        else if (0 == MT_OSAL_Strncmp(pItemValue, "false", strlen("false")))
        {
            pAvplay->bFrcEnable = MT_FALSE;
        }
        else
        {
            AVPLAY_ProcPrintHelp();
        }
    }
    else
    {
        AVPLAY_ProcPrintHelp();
    }*/

    if (0 == strncmp(pItemName, "FrcEnable", strlen("FrcEnable")))
    {
        if (0 == strncmp(pItemValue, "true", strlen("true")))
        {
            pAvplay->bFrcEnable = MT_TRUE;
        }
        else if (0 == strncmp(pItemValue, "false", strlen("false")))
        {
            pAvplay->bFrcEnable = MT_FALSE;
        }
        else
        {
            AVPLAY_ProcPrintHelp();
        }
    }
    else
    {
        AVPLAY_ProcPrintHelp();
    }



    return count;
}

mt_s32 AVPLAY_Create(AVPLAY_CREATE_S *pAvplayCreate, struct file *file)
{
    mt_proc_entry_t  *pProcItem;
    mt_char           ProcName[12];
    mmz_buffer_s      MemBuf;
    mt_s32            Ret;
    mt_u32            i;
    mt_char           BufName[32];

    if (AVPLAY_MAX_NUM == g_AvplayGlobalState.AvplayCount)
    {
        MT_ERR_AVPLAY("the avplay num is max.\n");
        return MT_ERR_AVPLAY_CREATE_ERR;
    }

    for (i=0; i<AVPLAY_MAX_NUM; i++)
    {
        if (MT_NULL == g_AvplayGlobalState.AvplayInfo[i].pAvplay)
        {
            break;
        }
    }

    if (i == AVPLAY_MAX_NUM)
    {
        MT_ERR_AVPLAY("the avplay num is max.\n");
        return MT_ERR_AVPLAY_CREATE_ERR;
    }

    mt_osal_snprintf(BufName, sizeof(BufName), "AVPLAY_Inst%02d", i);

	//Bug: 0x2000(8K) < sizeof(AVPLAY_S)
    //Ret = mt_drv_mmz_alloc_and_map(BufName, MMZ_OTHERS, 0x2000, 0, &MemBuf);
    Ret = mt_drv_mmz_alloc_and_map(BufName, MMZ_OTHERS, sizeof(AVPLAY_S), 0, &MemBuf);
    if (Ret != MT_SUCCESS)
    {
        MT_FATAL_AVPLAY("malloc %s mmz failed.\n", BufName);

        return Ret;
    }
    memset((void*)MemBuf.startVirAddr, 0, sizeof(AVPLAY_S));

    mt_osal_snprintf(ProcName, sizeof(ProcName), "%s%02d", MT_MOD_AVPLAY, i);

    pProcItem = mt_drv_proc_add_module(ProcName, MT_NULL, MT_NULL);
    if (!pProcItem)
    {
        MT_FATAL_AVPLAY("add %s proc failed.\n", ProcName);

        mt_drv_mmz_unmap_and_release(&MemBuf);

        return MT_FAILURE;
    }
    pProcItem->read = AVPLAY_ProcRead;
    pProcItem->write = AVPLAY_ProcWrite;

    g_AvplayGlobalState.AvplayInfo[i].pAvplay = (AVPLAY_S *)MemBuf.startVirAddr;
    g_AvplayGlobalState.AvplayInfo[i].pAvplay->hVdec = -1;
    g_AvplayGlobalState.AvplayInfo[i].AvplayPhyAddr = MemBuf.startPhyAddr;
    g_AvplayGlobalState.AvplayInfo[i].File = (ulong)file;
    g_AvplayGlobalState.AvplayInfo[i].AvplayStreamtype = pAvplayCreate->AvplayStreamtype;

    pAvplayCreate->AvplayId = i;
    pAvplayCreate->AvplayPhyAddr = MemBuf.startPhyAddr;
    g_AvplayGlobalState.AvplayCount++;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_AcquireAudFrameNum(mt_u32 AvplayId)
{
    if (MT_NULL == g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay)
    {
        MT_ERR_AVPLAY("this is invalid handle.\n");
        return -1;
    }
	
	g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay->DebugInfo.AcquireAudFrameNum++;

	return 0;
}

mt_s32 AVPLAY_AcquiredAudFrameNum(mt_u32 AvplayId, MT_UNF_AO_FRAMEINFO_S *pstAOFrame)
{
    if (MT_NULL == g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay)
    {
        MT_ERR_AVPLAY("this is invalid handle.\n");
        return -1;
    }

	memcpy(&g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay->AvplayAudFrm, pstAOFrame, sizeof(MT_UNF_AO_FRAMEINFO_S));
	g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay->DebugInfo.AcquiredAudFrameNum++;
	return 0;

}

mt_s32 AVPLAY_Destroy(mt_u32 AvplayId)
{
    mt_char           ProcName[17];
    mmz_buffer_s      MemBuf;

    if (MT_NULL == g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay)
    {
        MT_ERR_AVPLAY("this is invalid handle.\n");
        return MT_ERR_AVPLAY_DESTROY_ERR;
    }

    memset(ProcName, 0, sizeof(ProcName));

    mt_osal_snprintf(ProcName, sizeof(ProcName), "%s%02d", MT_MOD_AVPLAY, AvplayId);
    mt_drv_proc_rm_module(ProcName);//MT_DRV_PROC_RemoveModule

    MemBuf.startVirAddr = (void*)g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay;
    MemBuf.startPhyAddr = g_AvplayGlobalState.AvplayInfo[AvplayId].AvplayPhyAddr;
    //Bug: 0x2000 < sizeof(AVPLAY_S)
    //MemBuf.u32Size = 0x2000;
    MemBuf.size = sizeof(AVPLAY_S);

    mt_drv_mmz_unmap_and_release(&MemBuf);

    g_AvplayGlobalState.AvplayInfo[AvplayId].pAvplay = MT_NULL;
    g_AvplayGlobalState.AvplayInfo[AvplayId].AvplayPhyAddr = MT_NULL;
    g_AvplayGlobalState.AvplayInfo[AvplayId].File = MT_NULL;
    g_AvplayGlobalState.AvplayInfo[AvplayId].AvplayUsrAddr = MT_NULL;

    g_AvplayGlobalState.AvplayCount--;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_SetUsrAddr(AVPLAY_USR_ADDR_S *pAvplayUsrAddr)
{
    g_AvplayGlobalState.AvplayInfo[pAvplayUsrAddr->AvplayId].AvplayUsrAddr = (ulong)(pAvplayUsrAddr->AvplayUsrAddr);
    return MT_SUCCESS;
}

mt_s32 AVPLAY_CheckId(AVPLAY_USR_ADDR_S *pAvplayUsrAddr, struct file *file)
{
    if (g_AvplayGlobalState.AvplayInfo[pAvplayUsrAddr->AvplayId].File != ((ulong)file))
    {
        MT_ERR_AVPLAY("this is invalid handle.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    if (MT_NULL == g_AvplayGlobalState.AvplayInfo[pAvplayUsrAddr->AvplayId].pAvplay)
    {
        MT_ERR_AVPLAY("this is invalid handle.\n");
        return MT_ERR_AVPLAY_INVALID_PARA;
    }

    pAvplayUsrAddr->AvplayUsrAddr = g_AvplayGlobalState.AvplayInfo[pAvplayUsrAddr->AvplayId].AvplayUsrAddr;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_GetPlayInfo(MT_UNF_AVPLAY_PLAYERINFO_S * info)
{
    AVPLAY_S          *pAvplay = MT_NULL;
    if (MT_NULL == info)
    {
        return -EFAULT;
    }
	
	if(info->index >= AVPLAY_MAX_NUM)
	{
        return -EFAULT;
	}
	
    pAvplay = g_AvplayGlobalState.AvplayInfo[info->index].pAvplay;
    if (MT_NULL == pAvplay)
    {
        return -EFAULT;
    }
	info->CurStatus = pAvplay->CurStatus;
	info->VidEnable = pAvplay->VidEnable;
	info->AudEnable = pAvplay->AudEnable;

	if(MT_UNF_AVPLAY_STREAM_TYPE_ES == pAvplay->AvplayAttr.stStreamAttr.enStreamType)
	{
		info->u32DemuxId = 0xff;
	}
	else
	{
		info->u32DemuxId = pAvplay->AvplayAttr.u32DemuxId;
	}
	info->enStreamType = pAvplay->AvplayAttr.stStreamAttr.enStreamType;
	info->AvplayCount = g_AvplayGlobalState.AvplayCount;

    return MT_SUCCESS;
}

mt_s32 AVPLAY_CheckNum(mt_u32 *pAvplayNum, struct file *file)
{
    mt_u32   i;

    *pAvplayNum = 0;

    for (i=0; i<AVPLAY_MAX_NUM; i++)
    {
        if (g_AvplayGlobalState.AvplayInfo[i].File == ((ulong)file))
        {
            (*pAvplayNum)++;
        }
    }

    return MT_SUCCESS;
}

extern void cpufreq_interactive_boost(void);
mt_s32 AVPLAY_SetCpuFreq(struct file *file)
{
    mt_u32  i;
    //mt_s32  Ret;
    struct  cpufreq_policy cur_policy;

    for (i = 0; i < AVPLAY_MAX_NUM; i++)
    {
        if (g_AvplayGlobalState.AvplayInfo[i].File == ((ulong)file))
        {
            break;
        }
    }

    if (i == AVPLAY_MAX_NUM)
    {
        MT_ERR_AVPLAY("call AVPLAY_SetCpuFreq fail.\n");
        return MT_FAILURE;
    }

		//Rock_hu
    /*Ret = cpufreq_get_policy(&cur_policy, 0);
    if (Ret)
    {
        return MT_FAILURE;
    }*/

    if (!strncasecmp(cur_policy.governor->name, "interactive", CPUFREQ_NAME_LEN))
    {
        MT_INFO_AVPLAY("------------boost--------------\n");
        //cpufreq_interactive_boost();//Rock_hu
    }

    return MT_SUCCESS;
}

mt_s32 AVPLAY_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg)
{
    mt_s32           Ret;

    Ret = down_interruptible(&g_AvplayMutex);

    switch (cmd)
    {
        case CMD_AVPLAY_CREATE:
        {
            AVPLAY_CREATE_S  *pAvplayCreate;

            pAvplayCreate = (AVPLAY_CREATE_S *)arg;

            Ret = AVPLAY_Create(pAvplayCreate, file);

            break;
        }

        case CMD_AVPLAY_DESTROY:
        {
            Ret = AVPLAY_Destroy(*((mt_u32 *)arg));

            break;
        }

        case CMD_AVPLAY_SET_USRADDR:
        {
            AVPLAY_USR_ADDR_S *pAvplayUsrAddr;

            pAvplayUsrAddr = (AVPLAY_USR_ADDR_S *)arg;

            Ret = AVPLAY_SetUsrAddr(pAvplayUsrAddr);

            break;
        }

        case CMD_AVPLAY_CHECK_ID:
        {
            AVPLAY_USR_ADDR_S *pAvplayUsrAddr;

            pAvplayUsrAddr = (AVPLAY_USR_ADDR_S *)arg;

            Ret = AVPLAY_CheckId(pAvplayUsrAddr, file);

            break;
        }

        case CMD_AVPLAY_CHECK_NUM:
        {
            Ret = AVPLAY_CheckNum((mt_u32 *)arg, file);

            break;
        }

        case CMD_AVPLAY_SET_CPUFREQ:
        {
            Ret = AVPLAY_SetCpuFreq(file);

			break;
        }
		case CMD_AVPLAY_GET_PLAYERINFO:
		{
            MT_UNF_AVPLAY_PLAYERINFO_S *info;
            info = (MT_UNF_AVPLAY_PLAYERINFO_S *)arg;
            Ret = AVPLAY_GetPlayInfo(info);
			break;
		}
        default:
            up(&g_AvplayMutex);
            return -ENOIOCTLCMD;
    }

    up(&g_AvplayMutex);
    return Ret;
}

static mt_s32 AVPLAY_DRV_Open(struct inode *finode, struct file  *ffile)
{
/*
    mt_s32            Ret;

    Ret = down_interruptible(&g_AvplayMutex);

    if (1 == atomic_inc_return(&g_AvplayCount))
    {
    }

    up(&g_AvplayMutex);
*/
    return 0;
}

static mt_s32 AVPLAY_DRV_Close(struct inode *finode, struct file  *ffile)
{
    mt_s32           i;
    mt_s32           Ret;

    Ret = down_interruptible(&g_AvplayMutex);

    for (i=0; i<AVPLAY_MAX_NUM; i++)
    {
        if (g_AvplayGlobalState.AvplayInfo[i].File == ((ulong)ffile))
        {
            Ret = AVPLAY_Destroy(i);
            if (Ret != MT_SUCCESS)
            {
                up(&g_AvplayMutex);
                return -1;
            }
        }
    }

    if (atomic_dec_and_test(&g_AvplayCount))
    {
    }

    up(&g_AvplayMutex);

    return 0;
}

static long AVPLAY_DRV_Ioctl(struct file *ffile, unsigned int cmd, unsigned long arg)
{
    mt_s32 Ret;

    Ret = mt_drv_usercopy(ffile->f_path.dentry->d_inode, ffile, cmd, arg, AVPLAY_Ioctl);

    return Ret;
}

static struct file_operations AVPLAY_FOPS =
{
    .owner          =  THIS_MODULE,
    .open           =  AVPLAY_DRV_Open,
    .unlocked_ioctl =  AVPLAY_DRV_Ioctl,
    .release        =  AVPLAY_DRV_Close,
};

/*****************************************************************************
 Prototype    : AVPLAY_Suspend
 Description  :
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :
  History        :
  1.Date         : 2010/5/15
    Author       : weideng
    Modification : Created function
*****************************************************************************/
static mt_s32 AVPLAY_Suspend(basedev_s *pdev, pm_message_t state)
{
    mt_u32      i;

    for (i = 0; i < AVPLAY_MAX_NUM; i++)
    {
        if (MT_NULL != g_AvplayGlobalState.AvplayInfo[i].pAvplay)
        {
            g_AvplayGlobalState.AvplayInfo[i].pAvplay->bStandBy = MT_TRUE;
        }
    }

    MT_PRINT("AVPLAY suspend OK\n");
    return 0;
}

/*****************************************************************************
 Prototype    : AVPLAY_Resume
 Description  :
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :
  History        :
  1.Date         : 2010/5/15
    Author       : wei deng
    Modification : Created function
*****************************************************************************/
static mt_s32 AVPLAY_Resume(basedev_s *pdev)
{
    MT_PRINT("AVPLAY resume OK\n");

    return 0;
}

static baseops_s AVPLAY_DRVOPS = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = AVPLAY_Suspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = AVPLAY_Resume,
};

mt_s32 __init AVPLAY_DRV_ModInit(mt_void)
{
    mt_u32      i;
    mt_s32      Ret;

    MT_INFO_AVPLAY("AVPLAY_DRV_ModInit int\n");

    Ret = mt_drv_module_register(MT_ID_AVPLAY, AVPLAY_NAME, MT_NULL);
    if(MT_SUCCESS != Ret)
    {
        MT_ERR_AVPLAY("ERR: MT_DRV_MODULE_Register, Ret = %#x!\n", Ret);
    }

    g_AvplayGlobalState.AvplayCount = 0;
    for (i=0; i<AVPLAY_MAX_NUM; i++)
    {
        g_AvplayGlobalState.AvplayInfo[i].pAvplay = MT_NULL;
        g_AvplayGlobalState.AvplayInfo[i].AvplayPhyAddr = MT_NULL;
        g_AvplayGlobalState.AvplayInfo[i].File = MT_NULL;
        g_AvplayGlobalState.AvplayInfo[i].AvplayUsrAddr = MT_NULL;
    }

    mt_osal_snprintf(g_AvplayRegisterData.devfs_name, sizeof(g_AvplayRegisterData.devfs_name), UMAP_DEVNAME_AVPLAY);
    g_AvplayRegisterData.fops = &AVPLAY_FOPS;
    g_AvplayRegisterData.minor = UMAP_MIN_MINOR_AVPLAY;
    g_AvplayRegisterData.owner  = THIS_MODULE;
    g_AvplayRegisterData.drvops = &AVPLAY_DRVOPS;

    if (mt_drv_dev_register(&g_AvplayRegisterData) < 0)
    {
        MT_ERR_AVPLAY("register AVPLAY failed.\n");
        return MT_FAILURE;
    }

    MT_INFO_AVPLAY("AVPLAY_DRV_ModInit exit \n");

    return  0;
}

mt_void __exit AVPLAY_DRV_ModExit(mt_void)
{
    mt_drv_dev_unregister(&g_AvplayRegisterData);

    mt_drv_module_unregister(MT_ID_AVPLAY);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
