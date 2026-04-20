/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kthread.h>
#include <linux/timekeeping.h>
#include <linux/time.h>

#include "mt_type.h"
#include "mt_module.h"
#include "mt_drv_mmz.h"
#include "mt_drv_mem.h"
#include "mt_drv_module.h"
#include "mt_drv_proc.h"
#include "mt_drv_file.h"
#include "mt_kernel_adapt.h"
#include "mt_module_debug.h"
#include "mt_mpi_demux.h"

#include "mt_drv_demux.h"
#include "drv_demux_ioctl.h"
#include "hal_demux.h"
#include "drv_demux_func.h"
#include "drv_demux.h"
#include "drv_demux_reg.h"
#include "drv_demux_sw.h"
#include "drv_demux_osal.h"
#include "drv_demux_ext.h"

#ifdef DMX_DESCRAMBLER_SUPPORT
#include "mt_drv_descrambler.h"
#include "descrambler/drv_descrambler.h"
#endif

//#include "drv_sync_ext.h"

/***************************** Macro Definition ******************************/
#define DEMUX_NAME                      "MT_DEMUX"

#define DMX_MIN_POOLBUFFER_SIZE         0x80000
#define DMX_MAX_POOLBUFFER_SIZE         0x400000
#define DMX_DEFAULT_POOLBUFFER_SIZE     CFG_MT_DEMUX_POOLBUF_SIZE

#define DMX_FLTID(FilterHandle)     ((FilterHandle) & 0xff)


#define DMX_CHECK_FLTHANDLE(FilterHandle)                               \
    do                                                                  \
    {                                                                   \
        if (   (DMX_FLTID(FilterHandle) >= DMX_FILTER_CNT)              \
            || (((FilterHandle) & 0xffffff00) != DMX_FLTHANDLE(0)) )    \
        {                                                               \
            MT_ERR_DEMUX("Invalid FilterHandle 0x%x\n", FilterHandle); \
            return MT_ERR_DMX_INVALID_PARA;                             \
        }                                                               \
    } while (0)

#define DMX_CHECK_PCRHANDLE(PcrHandle)                                  \
    do                                                                  \
    {                                                                   \
        if ((PcrHandle) >= DMX_PCR_CHANNEL_CNT)                         \
        {                                                               \
            MT_ERR_DEMUX("Invalid PcrHandle 0x%x\n", PcrHandle);       \
            return MT_ERR_DMX_INVALID_PARA;                             \
        }                                                               \
    } while (0)

#define DMX_DEFAULT_BUF_NUM     16

/**************************** global variables ****************************/

DMX_DEV_OSR_S g_stDmxOsr;

static DEMUX_EXPORT_FUNC_S s_DmxExportFuncs =
{
    .pfnDmxAcquireEs    = MT_DRV_DMX_AcquireEs,
    .pfnDmxReleaseEs    = MT_DRV_DMX_ReleaseEs,
    .pfnDmxSuspend      = DMX_OsiSuspend,
    .pfnDmxResume       = DMX_OsiResume,
};

static uint DmxPoolBufSize  = DMX_DEFAULT_POOLBUFFER_SIZE;
static uint DmxBlockSize    = DMX_FQ_COM_BLKSIZE;

module_param(DmxPoolBufSize, uint, S_IRUGO);
module_param(DmxBlockSize, uint, S_IRUGO);

//extern SYNC_EXPORT_FUNC_S   *g_pSyncFunc;
extern DMX_DEV_OSI_S *g_pDmxDevOsi;

/****************************** internal function *****************************/
#ifdef MT_DEMUX_PROC_SUPPORT
static mt_void DMX_OsrStopSaveEs(mt_void);
static mt_void DMX_OsrSaveIPTsStop(mt_void);
static mt_void DMX_OsrSaveALLTs_Stop(mt_void);
static mt_void DMX_OsrSaveDmxTs_Stop(mt_void);
#endif

static mt_u32 DMXParseBootargs(const mt_char *buf, const mt_char *str)
{
    mt_u32      ret = 0;
    mt_char    *p;

    p = strstr(buf, str);
    if (p)
    {
        p += strlen(str);

        ret = simple_strtol(p, NULL, 0);
    }

    return ret;
}

static mt_void DMXGetExternalParameter(mt_u32 *BufSize)
{
    mt_u32 ret;

    ret = DMXParseBootargs(saved_command_line, "DmxPoolBufSize=");
    if (0 != ret)
    {
        if (ret < DMX_MIN_POOLBUFFER_SIZE)
        {
            *BufSize = DMX_MIN_POOLBUFFER_SIZE;
        }
        else if (ret > DMX_MAX_POOLBUFFER_SIZE)
        {
            *BufSize = DMX_MAX_POOLBUFFER_SIZE;
        }
        else
        {
            *BufSize = ret;
        }
    }
	//add by yuwu
	//printk("DMXGetExternalParameter===========================DMX_MAX_POOLBUFFER_SIZE=%x\n",DMX_MAX_POOLBUFFER_SIZE);
    *BufSize = DMX_MAX_POOLBUFFER_SIZE;
}

/**************************** external functions ******************************/

mt_u32 MT_DRV_DMX_ReadRegister(mt_u32 dmxRegisterOffset)
{
    return DMX_OsiReadRegister(dmxRegisterOffset);    
}

mt_void MT_DRV_DMX_WriteRegister(mt_u32 dmxRegisterOffset, mt_u32 value)
{
    DMX_OsiWriteRegister(dmxRegisterOffset, value);
}

/*****************************************************************************
 Prototype    : MT_DRV_DMX_Init
 Description  : DEMUX module initialize function
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
mt_s32 MT_DRV_DMX_Init(mt_void)
{
    mt_s32 ret;
   
    ret = mt_drv_module_register(MT_ID_DEMUX, DEMUX_NAME, (mt_void*)&s_DmxExportFuncs);
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_DEMUX("mt_drv_module_register failed\n");

        return ret;
    }
    
    DMXGetExternalParameter(&DmxPoolBufSize);

    ret = DMX_OsiDeviceInit(DmxPoolBufSize, DmxBlockSize);
    if (MT_SUCCESS != ret)
    {
        mt_drv_module_unregister(MT_ID_DEMUX);
        MT_ERR_DEMUX("DMX_OsiDeviceInit failed 0x%x\n", ret);

        return ret;
    }
    
    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : MT_DRV_DMX_DeInit
 Description  : DEMUX module exit function
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
mt_void MT_DRV_DMX_DeInit(mt_void)
{
#ifdef MT_DEMUX_PROC_SUPPORT
    DMX_OsrStopSaveEs();
    DMX_OsrSaveIPTsStop();
    DMX_OsrSaveALLTs_Stop();
    DMX_OsrSaveDmxTs_Stop();
#endif

    DMX_OsiDeviceDeInit();

    mt_drv_module_unregister(MT_ID_DEMUX);
}

/*****************************************************************************
 Prototype    : MT_DRV_DMX_Open
 Description  : demux open, called when open device node
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
mt_s32 MT_DRV_DMX_Open(mt_void)
{
    //return mt_drv_module_getfunction(MT_ID_SYNC, (mt_void**)&g_pSyncFunc);
    return 0;

}

/*****************************************************************************
 Prototype    : MT_DRV_DMX_Close
 Description  : demux close function, called when close device node
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
mt_s32 MT_DRV_DMX_Close(ulong file)
{
    mt_s32  ret;
    mt_u32  i;

    /* stop record and release the record buffer */
    for (i = 0; i < DMX_CNT; i++)
    {
        if (g_stDmxOsr.RecFile[i] == (mt_u32)file)
        {
            ret = DMX_DRV_REC_StopRecChn(i);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_DEMUX("rec %u stop failed 0x%x\n", i, ret);
            }

            ret = DMX_DRV_REC_DestroyChannel(i);
            if (MT_SUCCESS != ret)
            {
                MT_ERR_DEMUX("rec %u destroy failed 0x%x\n", i, ret);
            }
        }
    }
	
    for (i = 0; i < DMX_REC_INDEX_CNT; i++)
    {
        DMX_OsiClearIndexBuf(i, 0);
    }
    /*TS Buffer Deinit, the most important to delete the mmz memory */
    for (i = 0; i < DMX_RAMPORT_CNT; i++)
    {
        if (g_stDmxOsr.u32TsBufProcessHandle[i] == (mt_u32)file)
        {
            MT_DRV_DMX_DestroyTSBuffer(i);
        }
    }

    /*destroy the filters used in this process */
    for (i = 0; i < DMX_FILTER_CNT; i++)
    {
        if (g_stDmxOsr.u32FilterProcessHandle[i] == (mt_u32)file)
        {
            MT_DRV_DMX_DestroyFilter(DMX_FLTHANDLE(i));
        }
    }

#ifdef DMX_DESCRAMBLER_SUPPORT
    DmxDestroyAllDescrambler((ulong)file);
#endif

    /*destroy the channels used in this process */
    for (i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if (g_stDmxOsr.ChanFile[i] == (mt_u32)file)
        {
            ret = MT_DRV_DMX_DestroyChannel(DMX_CHANHANDLE(i));
            if (MT_SUCCESS != ret)
            {
                MT_ERR_DEMUX("destroy chan failed 0x%x\n", ret);
            }
        }
    }

    /*destroy the PCR channels used in this process */
    for (i = 0; i < DMX_PCR_CHANNEL_CNT; i++)
    {
        if (g_stDmxOsr.u32PcrChProcessHandle[i] == (mt_u32)file)
        {
            MT_DRV_DMX_DestroyPcrChannel(i);
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_GetPoolBufAddr(DMX_MMZ_BUF_S *PoolBuf)
{
    mt_u32 VirAddr;

    CHECKPOINTER(PoolBuf);
    MT_INFO_DEMUX("--------MT_DRV_DMX_GetPoolBufAddr-------\n");
    return DMX_OsiGetPoolBufAddr(&VirAddr, &PoolBuf->u32BufPhyAddr, (mt_u32 *)&PoolBuf->u32BufSize);
}


mt_s32 MT_DRV_DMX_GetCapability(MT_UNF_DMX_CAPABILITY_S *Cap)
{
    CHECKPOINTER(Cap);

    Cap->u32IFPortNum       = DMX_IFPORT_CNT;
    Cap->u32TSIPortNum      = DMX_TSIPORT_CNT;
    Cap->u32TSOPortNum      = DMX_TSOPORT_CNT;
    Cap->u32RamPortNum      = DMX_RAMPORT_CNT;
    Cap->u32DmxNum          = DMX_CNT;
    Cap->u32ChannelNum      = DMX_CHANNEL_CNT;
    Cap->u32AVChannelNum    = DMX_AV_CHANNEL_CNT;
    Cap->u32FilterNum       = DMX_FILTER_CNT;
    Cap->u32KeyNum          = DMX_KEY_CNT;
    Cap->u32RecChnNum       = DMX_REC_CNT;

    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_SetPusi(MT_BOOL bCheckPusi)
{
    MT_BOOL bNoPusiEn = MT_TRUE;
    bNoPusiEn = (bCheckPusi == MT_TRUE)?MT_FALSE:MT_TRUE;
    DMX_OsiSetNoPusiEn(bNoPusiEn);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_SetTei(MT_UNF_DMX_TEI_SET_S *pstTei)
{
    CHECKPOINTER(pstTei);
    CHECKDMXID(pstTei->u32DemuxID);
    DMX_OsiSetTei(pstTei->u32DemuxID,pstTei->bTei);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_TSIAttachTSO(MT_UNF_DMX_TSI_ATTACH_TSO_S *pstTSIAttachTSO)
{
    mt_u32 PortId = 0;
    mt_u32 Id = 0;
    CHECKPOINTER(pstTSIAttachTSO);

    Id = (mt_u32)pstTSIAttachTSO->enTSI - (mt_u32)MT_UNF_DMX_PORT_TSI_0;
    if (Id < DMX_TSIPORT_CNT)
    {                
        PortId     = Id + DMX_IFPORT_CNT;   
        DMX_OsiTSIAttashTSO(PortId,pstTSIAttachTSO->enTSO);   
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }          
}

MT_BOOL MT_DRV_DMX_IsTSIAttachTSO(mt_u32 PortId, MT_UNF_DMX_TSO_PORT_E* TSO)
{
    if ( (PortId < DMX_IFPORT_CNT) || (PortId > DMX_TUNERPORT_CNT) )
    {
        MT_ERR_DEMUX("invalid port id : %d\n",PortId);
        return MT_FALSE;   
    }
    return DMX_OsiIsTSIAttachTSO(PortId,TSO); 
}


mt_s32 MT_DRV_DMX_TSOPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr)
{
    CHECKTSOPORTID(PortId);
    CHECKPOINTER(PortAttr);

    return DMX_OsiTSOPortGetAttr(PortId, PortAttr);
}

mt_s32 MT_DRV_DMX_TSOPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr)
{
    CHECKTSOPORTID(PortId);
    CHECKPOINTER(PortAttr);

    return DMX_OsiTSOPortSetAttr(PortId, PortAttr);
}

mt_s32 MT_DRV_DMX_TunerPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    CHECKTUNERPORTID(PortId);
    CHECKPOINTER(PortAttr);

    return DMX_OsiTunerPortGetAttr(PortId, PortAttr);
}

mt_s32 MT_DRV_DMX_TunerPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    CHECKTUNERPORTID(PortId);
    CHECKPOINTER(PortAttr);

    return DMX_OsiTunerPortSetAttr(PortId, PortAttr);
}

mt_s32 MT_DRV_DMX_RamPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    CHECKRAMPORTID(PortId);
    CHECKPOINTER(PortAttr);

    return DMX_OsiRamPortGetAttr(PortId, PortAttr);
}

mt_s32 MT_DRV_DMX_RamPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    CHECKRAMPORTID(PortId);
    CHECKPOINTER(PortAttr);

    return DMX_OsiRamPortSetAttr(PortId, PortAttr);
}
#ifdef DMX_TAG_DEAL_SUPPORT
mt_s32  MT_DRV_DMX_GetTagAttr(const mt_u32 DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr)
{
    CHECKTUNERPORTID(DmxId);
    CHECKPOINTER(pstAttr);

    return DMX_OsiGetTagAttr(DmxId, pstAttr);
}

mt_s32  MT_DRV_DMX_SetTagAttr(const mt_u32 DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr)
{
    CHECKTUNERPORTID(DmxId);
    CHECKPOINTER(pstAttr);

    return DMX_OsiSetTagAttr(DmxId, pstAttr);
}

mt_s32 MT_DRV_DMX_GetTagPortId(mt_u32 DmxId, mt_u32 *TagPortId)
{
    CHECKDMXID(DmxId);
    CHECKPOINTER(TagPortId);

    return DMX_OsiGetTagPortId(DmxId, TagPortId);
}
#endif

mt_s32 MT_DRV_DMX_AttachTunerPort(mt_u32 DmxId, mt_u32 PortId)
{
    CHECKDMXID(DmxId);
    CHECKTUNERPORTID(PortId);

    return DMX_OsiAttachPort(DmxId, DMX_PORT_MODE_TUNER, PortId);
}

mt_s32 MT_DRV_DMX_AttachRamPort(mt_u32 DmxId, mt_u32 PortId)
{
    CHECKDMXID(DmxId);
    CHECKRAMPORTID(PortId);

    return DMX_OsiAttachPort(DmxId, DMX_PORT_MODE_RAM, PortId);
}

mt_s32 MT_DRV_DMX_DetachPort(mt_u32 DmxId)
{
    CHECKDMXID(DmxId);

    return DMX_OsiDetachPort(DmxId);
}

mt_s32 MT_DRV_DMX_GetPortId(mt_u32 DmxId, DMX_PORT_MODE_E *PortMode, mt_u32 *PortId)
{
    CHECKDMXID(DmxId);
    CHECKPOINTER(PortMode);
    CHECKPOINTER(PortId);

    return DMX_OsiGetPortId(DmxId, PortMode, PortId);
}



mt_s32 MT_DRV_DMX_TunerPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt, mt_u32 *ErrTsPackCnt)
{
    CHECKTUNERPORTID(PortId);
    CHECKPOINTER(TsPackCnt);
    CHECKPOINTER(ErrTsPackCnt);

    return DMX_OsiTunerPortGetPacketNum(PortId, TsPackCnt, ErrTsPackCnt);
}

mt_s32 MT_DRV_DMX_RamPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt)
{
    CHECKRAMPORTID(PortId);
    CHECKPOINTER(TsPackCnt);

    return DMX_OsiRamPortGetPacketNum(PortId, TsPackCnt);
}

/*
    application ts buffer, initialized repeatly will return failure.
    repeatly initialize in the same thread, will return success in user mode.
    In this repeatly call it, imply called from different process.
    not supported used in more than one process for one port TS buffer
 */
mt_s32 MT_DRV_DMX_CreateTSBuffer(const mt_u32 PortId, const mt_u32 Size, DMX_MMZ_BUF_S *TsBuf, const ulong file)
{
    mt_s32 ret;

    CHECKRAMPORTID(PortId);
    CHECKPOINTER(TsBuf);

    ret = DMX_OsiTsBufferCreate(PortId, Size, TsBuf);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.u32TsBufProcessHandle[PortId] = file;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DestroyTSBuffer(const mt_u32 PortId)
{
    mt_s32 ret;

    CHECKRAMPORTID(PortId);

    ret = DMX_OsiTsBufferDestroy(PortId);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.u32TsBufProcessHandle[PortId] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_GetTSBuffer(const mt_u32 PortId, const mt_u32 ReqLen, DMX_DATA_BUF_S *Data, const mt_u32 TimeoutMs)
{
    CHECKRAMPORTID(PortId);
    CHECKPOINTER(Data);

    return DMX_OsiTsBufferGet(PortId, ReqLen, Data, TimeoutMs);
}

mt_s32 MT_DRV_DMX_PutTSBuffer(const mt_u32 PortId, const mt_u32 DataLen, const mt_u32 StartPos, const mt_u16 Pid, dmx_ts_data_t DateType, mt_u32  Pts, mt_u32 SpecifiedDataAddr)
{
    CHECKRAMPORTID(PortId);

    return DMX_OsiTsBufferPut(PortId, DataLen, StartPos, Pid, DateType, Pts, SpecifiedDataAddr);
}

mt_s32 MT_DRV_DMX_ResetTSBuffer(const mt_u32 PortId)
{
    CHECKRAMPORTID(PortId);

    return DMX_OsiTsBufferReset(PortId);
}

mt_s32 MT_DRV_DMX_GetTSBufferStatus(const mt_u32 PortId, MT_UNF_DMX_TSBUF_STATUS_S *Status)
{
    CHECKRAMPORTID(PortId);
    CHECKPOINTER(Status);

    return DMX_OsiTsBufferGetStatus(PortId, Status);
}

mt_s32 MT_DRV_DMX_CreateChannel(
        mt_u32                  u32DmxId,
        MT_UNF_DMX_CHAN_ATTR_S *pstChAttr,
        mt_handle              *phChannel,
        DMX_MMZ_BUF_S          *pstChBuf,
        DMX_MMZ_BUF_S          *pstDescChBuf,
        ulong                  file
    )
{
    mt_s32  ret;
    mt_u32  ChanId;

    CHECKDMXID(u32DmxId);
    CHECKPOINTER(pstChAttr);
    CHECKPOINTER(phChannel);
    CHECKPOINTER(pstChBuf);

    ret = DMX_OsiCreateChannel(u32DmxId, pstChAttr, pstChBuf, pstDescChBuf, &ChanId);

    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.ChanFile[ChanId] = (mt_u32)file;
        *phChannel = DMX_CHANHANDLE(ChanId);
        MT_INFO_DEMUX("ChanId = %d, *phChannel = %#x\n", ChanId, *phChannel);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DestroyChannel(mt_handle hChannel)
{
    mt_s32  ret;
    mt_u32  ChanId;

    DMX_CHECK_CHANHANDLE(hChannel);

    ChanId = DMX_CHANID(hChannel);

    ret = DMX_OsiDestroyChannel(ChanId);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.ChanFile[ChanId] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiGetChannelAttr(DMX_CHANID(hChannel), pstChAttr);
}

mt_s32 MT_DRV_DMX_SetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiSetChannelAttr(DMX_CHANID(hChannel), pstChAttr);
}

mt_s32 MT_DRV_DMX_SetChannelPID(mt_handle hChannel, mt_u32 u32Pid)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiSetChannelPid(DMX_CHANID(hChannel), u32Pid);
}

mt_s32 MT_DRV_DMX_GetChannelPID(mt_handle hChannel, mt_u32 *pu32Pid)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pu32Pid);

    return DMX_OsiGetChannelPid(DMX_CHANID(hChannel), pu32Pid);
}

mt_s32 MT_DRV_DMX_OpenChannel(mt_handle hChannel)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiOpenChannel(DMX_CHANID(hChannel));
}

mt_s32 MT_DRV_DMX_CloseChannel(mt_handle hChannel)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiCloseChannel(DMX_CHANID(hChannel));
}

mt_s32 MT_DRV_DMX_GetChannelStatus(mt_handle hChannel, MT_UNF_DMX_CHAN_STATUS_S *pstStatus)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiGetChannelStatus(DMX_CHANID(hChannel), &pstStatus->enChanStatus);
}

mt_s32 MT_DRV_DMX_GetChannelId(mt_handle hChannel, mt_u32 *channelId)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    *channelId = DMX_CHANID(hChannel);
    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_GetChannelHandle(mt_u32 DmxId, mt_u32 Pid, MT_UNF_DMX_CHAN_TYPE_E  ChanType, mt_handle *ChanHandle)
{
    mt_s32  ret;
    mt_u32  ChanId;

    CHECKDMXID(DmxId);
    CHECKPOINTER(ChanHandle);

    ret = DMX_OsiGetChannelId(DmxId, Pid,  ChanType, &ChanId);
    if (MT_SUCCESS == ret)
    {
        *ChanHandle = DMX_CHANHANDLE(ChanId);
    }

    return ret;
}
  /*
    ** add for advca
    */
mt_s32 MT_DRV_DMX_GetAVChannelHandle(mt_u32 DmxId, mt_handle *vidHandle, mt_handle *auddHandle)
{
    mt_s32  ret;
    mt_u32  vidChanId;
    mt_u32  audChanId;
    
    CHECKDMXID(DmxId);
    CHECKPOINTER(vidHandle);
    CHECKPOINTER(auddHandle);

    ret = DMX_OsiGetAVChannelId(DmxId,  &vidChanId, &audChanId);
    if (MT_SUCCESS == ret)
    {
        *vidHandle = DMX_CHANHANDLE(vidChanId);
        *auddHandle = DMX_CHANHANDLE(audChanId);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_GetFreeChannelCount (mt_u32 DmxId, mt_u32 *FreeCount)
{
    CHECKDMXID(DmxId);
    CHECKPOINTER(FreeCount);

    return DMX_OsiGetFreeChannelNum(DmxId, FreeCount);
}

mt_s32 MT_DRV_DMX_GetScrambledFlag(mt_handle hChannel, MT_UNF_DMX_SCRAMBLED_FLAG_E *ScrambleFlag)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(ScrambleFlag);

    return DMX_OsiGetChannelScrambleFlag(DMX_CHANID(hChannel), ScrambleFlag);
}

mt_s32 MT_DRV_DMX_SetChannelEosFlag(mt_handle hChannel)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiSetChannelEosFlag(DMX_CHANID(hChannel));
}

#ifdef DMX_USE_ECM
mt_s32 DMX_OsrGetChannelSwFlag(mt_handle hChannel, mt_u32 *pu32SwFlag)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pu32SwFlag);

    return DMX_OsiGetChannelSwFlag(DMX_CHANID(hChannel), pu32SwFlag);
}

mt_s32 DMX_OsrGetChannelSwBufAddr(mt_handle hChannel, DMX_MMZ_BUF_S* pstSwBuf)
{
    mt_s32 ret;
    mmz_buffer_s stChnSwBuf;

    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pstSwBuf);

    ret = DMX_OsiGetChannelSwBufAddr(DMX_CHANID(hChannel), &stChnSwBuf);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("DMX_OsiGetChannelSwBufAddr failed:ChId=%d\n", DMX_CHANID(hChannel));
        return ret;
    }
    pstSwBuf->u32BufPhyAddr = stChnSwBuf.u32StartPhyAddr;
    pstSwBuf->u32BufKerVirAddr = stChnSwBuf.u32StartVirAddr;
    pstSwBuf->u32BufSize = stChnSwBuf.u32Size;

    return MT_SUCCESS;

}
#endif
mt_s32 MT_DRV_DMX_GetChannelTsCount(mt_handle hChannel, mt_u32 *pu32TsCnt)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pu32TsCnt);

    return DMX_OsiGetChannelTsCnt(DMX_CHANID(hChannel), pu32TsCnt);
}

mt_s32 MT_DRV_DMX_SetChannelCCRepeat(mt_handle hChannel, MT_UNF_DMX_CHAN_CC_REPEAT_SET_S *pstChCCReaptSet)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    return DMX_OsiSetChannelCCRepeat(DMX_CHANID(hChannel),pstChCCReaptSet);
}


mt_s32 MT_DRV_DMX_CreateFilter(mt_u32 DmxId, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr, mt_handle *Filter, ulong file)
{
    mt_s32  ret;
    mt_u32  FilterId;

    CHECKDMXID(DmxId);
    CHECKPOINTER(FilterAttr);
    CHECKPOINTER(Filter);

    ret = DMX_OsiNewFilter(DmxId, &FilterId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    ret = DMX_OsiSetFilterAttr(FilterId, FilterAttr);
    if (MT_SUCCESS != ret)
    {
        DMX_OsiDeleteFilter(FilterId);

        return ret;
    }

    *Filter = DMX_FLTHANDLE(FilterId);

    g_stDmxOsr.u32FilterProcessHandle[FilterId] = file;

    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_DestroyFilter(mt_handle Filter)
{
    mt_s32 ret;

    DMX_CHECK_FLTHANDLE(Filter);

    ret = DMX_OsiDeleteFilter(DMX_FLTID(Filter));
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.u32FilterProcessHandle[DMX_FLTID(Filter)] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DestroyAllFilter(mt_handle Channel)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 FilterId;

    DMX_CHECK_CHANHANDLE(Channel);

    for (FilterId = 0; FilterId < DMX_FILTER_CNT; FilterId++)
    {
        mt_u32 ChanId;

        ret = DMX_OsiGetFilterChannel(FilterId, &ChanId);
        if (MT_SUCCESS == ret)
        {
            if (ChanId == DMX_CHANID(Channel))
            {
                ret = DMX_OsiDeleteFilter(FilterId);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_DEMUX("DMX_OsiDeleteFilter failed:0x%x\n", ret);
                    return ret;
                }

                g_stDmxOsr.u32FilterProcessHandle[FilterId] = 0;
            }
        }
    }

    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_SetFilterAttr(mt_handle Filter, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr)
{
    DMX_CHECK_FLTHANDLE(Filter);
    CHECKPOINTER(FilterAttr);

    return DMX_OsiSetFilterAttr(DMX_FLTID(Filter), FilterAttr);
}

mt_s32 MT_DRV_DMX_GetFilterAttr(mt_handle Filter, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr)
{
    DMX_CHECK_FLTHANDLE(Filter);
    CHECKPOINTER(FilterAttr);

    return DMX_OsiGetFilterAttr(DMX_FLTID(Filter), FilterAttr);
}

mt_s32 MT_DRV_DMX_AttachFilter(mt_handle Filter, mt_handle Channel)
{
    DMX_CHECK_FLTHANDLE(Filter);
    DMX_CHECK_CHANHANDLE(Channel);

    return DMX_OsiAttachFilter(DMX_FLTID(Filter), DMX_CHANID(Channel));
}

mt_s32 MT_DRV_DMX_DetachFilter(mt_handle Filter, mt_handle Channel)
{
    DMX_CHECK_FLTHANDLE(Filter);
    DMX_CHECK_CHANHANDLE(Channel);

    return DMX_OsiDetachFilter(DMX_FLTID(Filter), DMX_CHANID(Channel));
}

mt_s32 MT_DRV_DMX_GetFilterChannelHandle(mt_handle Filter, mt_handle *Channel)
{
    mt_s32 ret;
    mt_u32 ChanId;

    DMX_CHECK_FLTHANDLE(Filter);
    CHECKPOINTER(Channel);

    ret = DMX_OsiGetFilterChannel(DMX_FLTID(Filter), &ChanId);
    if (MT_SUCCESS == ret)
    {
        *Channel = DMX_CHANHANDLE(ChanId);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_GetFreeFilterCount(mt_u32 DmxId, mt_u32 *FreeCount)
{
    CHECKDMXID(DmxId);
    CHECKPOINTER(FreeCount);

    return DMX_OsiGetFreeFilterNum(DmxId, FreeCount);
}

mt_s32 MT_DRV_DMX_CheckDataHandle(mt_handle hChannel, mt_u32 u32TimeOutMs)
{
    mt_u32  ChanId;

    DMX_CHECK_CHANHANDLE(hChannel);
    ChanId = DMX_CHANID(hChannel);

    return DMX_OsiCheckDataFlag(ChanId, u32TimeOutMs);
}

mt_s32 MT_DRV_DMX_GetDataHandle(mt_u32 *pu32Flag, mt_u32 u32TimeOutMs)
{
    CHECKPOINTER(pu32Flag);

    return DMX_OsiSelectDataFlag(MT_NULL, DMX_CHANNEL_CNT, pu32Flag, u32TimeOutMs);
}

mt_s32 MT_DRV_DMX_SelectDataHandle(mt_handle *pu32WatchChannel, mt_u32 u32WatchNum, mt_u32 *pu32Flag, mt_u32 u32TimeOutMs)
{
    mt_u32  i;
    mt_u32  Chan[DMX_CHANNEL_CNT];

    CHECKPOINTER(pu32WatchChannel);
    CHECKPOINTER(pu32Flag);

    if (0 == u32WatchNum)
    {
        MT_ERR_DEMUX("u32WatchNum == 0!\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    for (i = 0; i < u32WatchNum; i++)
    {
        DMX_CHECK_CHANHANDLE(pu32WatchChannel[i]);

        Chan[i] = DMX_CHANID(pu32WatchChannel[i]);
    }

    return DMX_OsiSelectDataFlag(Chan, u32WatchNum, pu32Flag, u32TimeOutMs);
}

/*pstBuf is the address of user mode, need to copy out buf info
   pu8Buf in pstBuf need to return physical address to user mode
 */
mt_s32 MT_DRV_DMX_AcquireBuf(mt_handle hChannel, mt_u32 u32AcquireNum,
                          mt_u32 *pu32AcquiredNum, DMX_UserMsg_S *pstBuf,
                          mt_u32 u32TimeOutMs)
{
    mt_s32  ret;
    mt_u32  ChanId;
    mt_u32  i = 0;

    DMX_CHECK_CHANHANDLE(hChannel);
    ChanId = DMX_CHANID(hChannel);
    *pu32AcquiredNum = 0;

    ret = DMX_OsiReadDataRequest(ChanId, u32AcquireNum, pu32AcquiredNum, pstBuf, u32TimeOutMs);

    if (MT_SUCCESS != ret)
    {
        MT_WARN_DEMUX("DMX_OsiReadDataRequest failed:%x.\n",ret);
        return ret;
    }

    for (i = 0; i < *pu32AcquiredNum; i++)
    {
        pstBuf[i].enDataType = MT_UNF_DMX_DATA_TYPE_WHOLE;
    }

    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_ReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum, DMX_UserMsg_S *pstBuf)
{
    mt_s32 ret;
    mt_u32              ChanId;
    #ifdef DMX_USE_ECM
    mt_u32 u32SwFlag;
    #endif

    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pstBuf);

    if (0 == u32ReleaseNum)
    {
        MT_ERR_DEMUX("u32ReleaseNum == 0!\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    ChanId = DMX_CHANID(hChannel);

#ifdef DMX_USE_ECM
    DMX_OsiGetChannelSwFlag(ChanId, &u32SwFlag);
    if (u32SwFlag)
    {
        mt_s32 s32Ret;
        s32Ret  = MT_DMX_SwReleaseReadData(ChanId, u32ReleaseNum, pstBuf);
        if (MT_SUCCESS != s32Ret)
        {
            MT_WARN_DEMUX(" sw channel %d release error:%x!\n", ChanId, s32Ret);
            return s32Ret;
        }

    }
    else
    {
#endif
        ret = DMX_OsiReleaseReadData(ChanId, u32ReleaseNum, pstBuf);

        if (MT_SUCCESS != ret)
        {
            MT_WARN_DEMUX("Rel Data failed: ChId = %d, ret = %#x\n", ChanId, ret);
            return ret;
        }
 #ifdef DMX_USE_ECM
     }
 #endif
    return MT_SUCCESS;
}

mt_s32 MT_DRV_DMX_PeekBuf(mt_handle hChannel, mt_u32 u32PeekLen,DMX_UserMsg_S *pstBuf)
{
    mt_s32  ret;
    mt_u32  ChanId;
    DMX_UserMsg_S stReqBufTmp[16];
    #ifdef DMX_USE_ECM
    mt_u32 u32SwFlag;
    #endif

    DMX_CHECK_CHANHANDLE(hChannel);
    ChanId = DMX_CHANID(hChannel);
    memset(stReqBufTmp,0x0,sizeof(stReqBufTmp));


    #ifdef DMX_USE_ECM
    DMX_OsiGetChannelSwFlag(ChanId, &u32SwFlag);
    if (u32SwFlag)
    {
        ret  = MT_DMX_SwPeekDataRequest(ChanId, u32PeekLen, stReqBufTmp);
        if (MT_SUCCESS != ret)
        {
            return ret;
        }
    }
    else
    {
    #endif
        ret = DMX_OsiPeekDataRequest(ChanId, u32PeekLen, stReqBufTmp);
        if (MT_SUCCESS != ret)
        {
            return ret;
        }
    #ifdef DMX_USE_ECM
    }
    #endif
    memcpy(pstBuf,stReqBufTmp,sizeof(DMX_UserMsg_S));
    if (pstBuf->u32MsgLen > u32PeekLen)
    {
        pstBuf->u32MsgLen = u32PeekLen;
    }
    return MT_SUCCESS;

}

mt_s32 MT_DRV_DMX_CreatePcrChannel(const mt_u32 DmxId, mt_u32 *PcrHandle, const ulong file)
{
    mt_s32 ret;

    CHECKDMXID(DmxId);
    CHECKPOINTER(PcrHandle);

    ret = DMX_OsiPcrChannelCreate(DmxId, PcrHandle);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.u32PcrChProcessHandle[*PcrHandle] = (mt_u32)file;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DestroyPcrChannel(const mt_u32 PcrHandle)
{
    mt_s32 ret;

    DMX_CHECK_PCRHANDLE(PcrHandle);

    ret = DMX_OsiPcrChannelDestroy(PcrHandle);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.u32PcrChProcessHandle[PcrHandle] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_PcrPidSet(const mt_u32 PcrHandle, const mt_u32 PcrPid)
{
    DMX_CHECK_PCRHANDLE(PcrHandle);

    return DMX_OsiPcrChannelSetPid(PcrHandle, PcrPid);
}

mt_s32 MT_DRV_DMX_PcrPidGet(const mt_u32 PcrHandle, mt_u32 *PcrPid)
{
    DMX_CHECK_PCRHANDLE(PcrHandle);
    CHECKPOINTER(PcrPid);

    return DMX_OsiPcrChannelGetPid(PcrHandle, PcrPid);
}

mt_s32 MT_DRV_DMX_PcrScrGet(const mt_u32 PcrHandle, mt_u64 *PcrValue, mt_u64 *ScrValue)
{
    DMX_CHECK_PCRHANDLE(PcrHandle);
    CHECKPOINTER(PcrValue);
    CHECKPOINTER(ScrValue);

    return DMX_OsiPcrChannelGetClock(PcrHandle, PcrValue, ScrValue);
}

mt_s32 MT_DRV_DMX_PcrSyncAttach(const mt_u32 PcrHandle, const mt_u32 SyncHandle)
{
    DMX_CHECK_PCRHANDLE(PcrHandle);

    return DMX_OsiPcrChannelAttachSync(PcrHandle, SyncHandle);
}

mt_s32 MT_DRV_DMX_PcrSyncDetach(const mt_u32 PcrHandle)
{
    DMX_CHECK_PCRHANDLE(PcrHandle);

    return DMX_OsiPcrChannelDetachSync(PcrHandle);
}

mt_s32 MT_DRV_DMX_GetPESBufferStatus(mt_handle ChanHandle, MT_MPI_DMX_BUF_STATUS_S *BufStatus)
{
    DMX_CHECK_CHANHANDLE(ChanHandle);
    CHECKPOINTER(BufStatus);

    return DMX_OsiGetChanBufStatus(DMX_CHANID(ChanHandle), BufStatus);
}

mt_s32 MT_DRV_DMX_AcquireEs(mt_handle hChannel, DMX_Stream_S *pEsBuf)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pEsBuf);

    return DMX_OsiReadEsRequest(DMX_CHANID(hChannel), pEsBuf);
}

mt_s32 MT_DRV_DMX_ReleaseEs(mt_handle hChannel, DMX_Stream_S *pEsBuf)
{
    DMX_CHECK_CHANHANDLE(hChannel);
    CHECKPOINTER(pEsBuf);

    return DMX_OsiReleaseReadEs(DMX_CHANID(hChannel), pEsBuf);
}

mt_s32 MT_DRV_DMX_CreateRecChn(
        MT_UNF_DMX_REC_ATTR_S  *RecAttr,
        mt_handle              *RecHandle,
        phys_addr_t            *RecBufPhyAddr,
        mt_u32                 *RecBufSize,
        phys_addr_t            *RecIdxBufPhyAddr,
        mt_u32                 *RecIdxBufSize,
        ulong                  file
    )
{
    mt_s32  ret;
    mt_u32  RecId;
    DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp = (DMX_REC_TIMESTAMP_MODE_E)RecAttr->rec_mode; //DMX_REC_TIMESTAMP_NONE;
    MT_INFO_DEMUX("==============\n");
    CHECKPOINTER(RecAttr);
    CHECKPOINTER(RecHandle);
    CHECKPOINTER(RecBufPhyAddr);
    CHECKPOINTER(RecBufSize);
    CHECKPOINTER(RecIdxBufPhyAddr);
    CHECKPOINTER(RecIdxBufSize);
    MT_INFO_DEMUX("==============\n");
    ret = DMX_DRV_REC_CreateChannel(RecAttr, enRecTimeStamp,&RecId, RecBufPhyAddr, RecBufSize, RecIdxBufPhyAddr, RecIdxBufSize);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.RecFile[RecId] = file;

        *RecHandle = DMX_RECHANDLE(RecId);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_CreateLinkRecChn(DMX_LinkRec_CreateChan_S *link_rec_param,	ulong		file)
{
	mt_s32	ret;
	mt_u32	RecId;
	DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp = (DMX_REC_TIMESTAMP_MODE_E)(link_rec_param->RecAttr.rec_mode); //DMX_REC_TIMESTAMP_NONE;
	MT_INFO_DEMUX("==============\n");
	CHECKPOINTER(link_rec_param);
	MT_INFO_DEMUX("==============\n");
	ret = DMX_DRV_LinkREC_CreateChannel(&link_rec_param->RecAttr, enRecTimeStamp, &RecId, link_rec_param);
	if (MT_SUCCESS == ret)
	{
		g_stDmxOsr.RecFile[RecId] = file;
		
		link_rec_param->RecHandle = DMX_RECHANDLE(RecId);
	}

	return ret;
}

mt_s32 MT_DRV_DMX_DestroyRecChn(mt_handle RecHandle)
{
    mt_s32  ret;
    mt_u32  RecId;

    DMX_CHECK_RECHANDLE(RecHandle);

    RecId = DMX_RECID(RecHandle);

    ret = DMX_DRV_REC_DestroyChannel(RecId);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.RecFile[RecId] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DestroyLinkRecChn(mt_handle RecHandle)
{
    mt_s32  ret;
    mt_u32  RecId;

    DMX_CHECK_RECHANDLE(RecHandle);

    RecId = DMX_RECID(RecHandle);

    ret = DMX_DRV_LinkREC_DestroyChannel(RecId);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.RecFile[RecId] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_AddRecPid(mt_handle RecHandle, mt_u32 Pid, mt_handle *ChanHandle, ulong file)
{
    mt_s32  ret;
    mt_u32  ChanId;
    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(ChanHandle);
    ret = DMX_DRV_REC_AddRecPid(DMX_RECID(RecHandle), Pid, &ChanId);
    if (MT_SUCCESS == ret)
    {
        *ChanHandle = DMX_CHANHANDLE(ChanId);

        g_stDmxOsr.ChanFile[ChanId] = (mt_u32)file;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DelRecPid(mt_handle RecHandle, mt_handle ChanHandle)
{
    mt_s32  ret;
    mt_u32  ChanId;

    DMX_CHECK_RECHANDLE(RecHandle);
    DMX_CHECK_CHANHANDLE(ChanHandle);

    ChanId = DMX_CHANID(ChanHandle);

    ret = DMX_DRV_REC_DelRecPid(DMX_RECID(RecHandle), ChanId);
    if (MT_SUCCESS == ret)
    {
        g_stDmxOsr.ChanFile[ChanId] = 0;
    }

    return ret;
}

mt_s32 MT_DRV_DMX_DelAllRecPid(mt_handle RecHandle)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_DelAllRecPid(DMX_RECID(RecHandle));
}

//use when can not get scd,opentv 5 add this function
mt_s32 MT_DRV_DMX_GetRecTsCnt(mt_handle RecHandle,mt_u32* TSCnt)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_GetTsCnt(DMX_RECID(RecHandle),TSCnt);
}


mt_s32 MT_DRV_DMX_AddExcludeRecPid(mt_handle RecHandle, mt_u32 Pid)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_AddExcludeRecPid(DMX_RECID(RecHandle), Pid);
}

mt_s32 MT_DRV_DMX_DelExcludeRecPid(mt_handle RecHandle, mt_u32 Pid)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_DelExcludeRecPid(DMX_RECID(RecHandle), Pid);
}

mt_s32 MT_DRV_DMX_DelAllExcludeRecPid(mt_handle RecHandle)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_DelAllExcludeRecPid(DMX_RECID(RecHandle));
}

mt_s32 MT_DRV_DMX_StartRecChn(mt_handle RecHandle)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_StartRecChn(DMX_RECID(RecHandle));
}

mt_s32 MT_DRV_DMX_StopRecChn(mt_handle RecHandle)
{
    DMX_CHECK_RECHANDLE(RecHandle);

    return DMX_DRV_REC_StopRecChn(DMX_RECID(RecHandle));
}

mt_s32 MT_DRV_DMX_AcquireRecData(mt_handle RecHandle, MT_UNF_DMX_REC_DATA_S *RecData, mt_u32 Timeout)
{
    mt_s32  ret;
    phys_addr_t  PhyAddr;
    ulong  KerAddr;
    mt_u32  Len;

    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(RecData);

    ret = DMX_DRV_REC_AcquireRecData(DMX_RECID(RecHandle), &PhyAddr, &KerAddr, &Len, Timeout);
    if (MT_SUCCESS == ret)
    {
        RecData->u32DataPhyAddr = PhyAddr;
        RecData->pDataAddr      = (mt_u8  *)KerAddr;
        RecData->u32Len         = Len;
	 //printk("RecData->pDataAddr=0x%x, RecData->u32Len=%d\n",RecData->pDataAddr,RecData->u32Len);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_AcquireLinkRecData(mt_handle RecHandle, MT_UNF_DMX_REC_DATA_S *RecData, mt_u32 Timeout, mt_u8 *p_node_index)
{
    mt_s32  ret;
    phys_addr_t  PhyAddr;
    ulong  KerAddr;
    mt_u32  Len;
	mt_u8 node_index;

    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(RecData);
	CHECKPOINTER(p_node_index);

	*p_node_index = 0;
    ret = DMX_DRV_LinkREC_AcquireRecData(DMX_RECID(RecHandle), &PhyAddr, &KerAddr, &Len, Timeout, &node_index);
    if (MT_SUCCESS == ret)
    {
        RecData->u32DataPhyAddr = PhyAddr;
        RecData->pDataAddr      = (mt_u8  *)KerAddr;
        RecData->u32Len         = Len;
		*p_node_index 			= node_index;
	 //printk("RecData->pDataAddr=0x%x, RecData->u32Len=%d\n",RecData->pDataAddr,RecData->u32Len);
    }

    return ret;
}

mt_s32 MT_DRV_DMX_ReleaseRecData(mt_handle RecHandle, const MT_UNF_DMX_REC_DATA_S *RecData)
{
    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(RecData);

    return DMX_DRV_REC_ReleaseRecData(DMX_RECID(RecHandle), RecData->u32DataPhyAddr, RecData->u32Len);
}

mt_s32 MT_DRV_DMX_ReleaseLinkRecData(mt_handle RecHandle, const MT_UNF_DMX_REC_DATA_S *RecData)
{
    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(RecData);

    return DMX_DRV_LinkREC_ReleaseRecData(DMX_RECID(RecHandle), RecData->u32DataPhyAddr, RecData->u32Len);
}

mt_s32 MT_DRV_DMX_AcquireRecIndex(mt_handle RecHandle, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout)
{
    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(RecIndex);
    return DMX_DRV_REC_AcquireRecIndex(DMX_RECID(RecHandle), RecIndex, Timeout);
}

mt_s32 MT_DRV_DMX_AcquireLinkRecIndex(mt_handle RecHandle, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout)
{
    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(RecIndex);
    return DMX_DRV_LinkREC_AcquireRecIndex(DMX_RECID(RecHandle), RecIndex, Timeout);
}

mt_s32 MT_DRV_DMX_GetRecBufferStatus(mt_handle RecHandle, MT_UNF_DMX_RECBUF_STATUS_S *BufStatus)
{
    DMX_CHECK_RECHANDLE(RecHandle);
    CHECKPOINTER(BufStatus);

    return DMX_DRV_REC_GetRecBufferStatus(DMX_RECID(RecHandle), BufStatus);
}


mt_s32 MT_DRV_DMX_SetChannelBufFullCare(mt_handle hChannel, MT_UNF_DMX_BUF_FULL_CARE_S *pstChBufFull)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiSetChannelBufFullCare(DMX_CHANID(hChannel), pstChBufFull);
}

mt_s32 MT_DRV_DMX_SetPortId(mt_handle hChannel, DMX_PORT_MODE_E PortMode, mt_u32 PortId)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiSetPortId(DMX_CHANID(hChannel), PortMode, PortId);
}


#ifdef MT_DEMUX_PROC_SUPPORT
#define DMX_FILE_NAME_LEN   (256)

MT_DECLARE_MUTEX(SaveEsMutex);
static struct file *DmxEsHandle[DMX_AV_CHANNEL_CNT];
static mt_u32       SaveEsFlag         = 0;


mt_void DMX_OsrSaveEs(mt_u32 type, mt_u8 *buf, mt_u32 len, mt_u32 chnid, char *p_path)
{
    struct tm now;

    if (len)
    {
        if (0 == down_interruptible(&SaveEsMutex))
        {
                if (SaveEsFlag && ((MT_UNF_DMX_CHAN_TYPE_VID == type) 
                    				|| (MT_UNF_DMX_CHAN_TYPE_AUD == type) 
                    				|| (MT_UNF_DMX_CHAN_TYPE_AUD_AD == type)
                    				|| (MT_UNF_DMX_CHAN_TYPE_REC == type)))
                {
                    if (MT_NULL == DmxEsHandle[chnid])
                    {
                        mt_char str[DMX_FILE_NAME_LEN]  = {0};
						time64_to_tm(ktime_get_seconds(), 0, &now);

						printk(KERN_ERR "save_path: %s\n", p_path);
                        if (MT_UNF_DMX_CHAN_TYPE_VID == type)
                        {
                            snprintf(str, sizeof(str), "%s/dmx_vid_%u-%02u_%02u_%02u.es", p_path, chnid, now.tm_hour, now.tm_min, now.tm_sec);
                        }
                        else if (MT_UNF_DMX_CHAN_TYPE_AUD == type)
                        {
                            snprintf(str, sizeof(str), "%s/dmx_aud_%u-%02u_%02u_%02u.es", p_path, chnid, now.tm_hour, now.tm_min, now.tm_sec);
                        }
						else if (MT_UNF_DMX_CHAN_TYPE_AUD_AD == type)
                        {
                            snprintf(str, sizeof(str), "%s/dmx_aud_ad_%u-%02u_%02u_%02u.es", p_path, chnid, now.tm_hour, now.tm_min, now.tm_sec);
                        }
						else
                        {
                            snprintf(str, sizeof(str), "%s/dmx_rec_%u-%02u_%02u_%02u.ts", p_path, chnid, now.tm_hour, now.tm_min, now.tm_sec);
                        }
						printk(KERN_ERR "save file: %s\n", str);

                        DmxEsHandle[chnid] = mt_drv_file_open(str, 1);
                        if (!DmxEsHandle[chnid])
                        {
                            MT_ERR_DEMUX("open %s error\n", str);

                            return;
                        }
                    }
                    mt_drv_file_write(DmxEsHandle[chnid], buf, len);
                    if (DmxEsHandle[chnid])
                    {
                        mt_drv_file_close(DmxEsHandle[chnid]);
                        DmxEsHandle[chnid] = MT_NULL;
                    }
                }

                up(&SaveEsMutex);
        }
    }
}

static mt_u32  DMX_OsrSaveEsDataToFile(DMX_DEBUG_CMD type, char *p_path)
{
    DMX_ChanInfo_S *ChanInfo;
    mt_u32 ChanId=0;
	mt_u32 rec_id;
	mt_u8 i = 0;
	
	if ((type == DMX_DEBUG_CMD_SAVE_VES) || (type == DMX_DEBUG_CMD_SAVE_AES))
	{
	    for(ChanId=0; ChanId<DMX_CHANNEL_CNT; ChanId++)
	    {
	        ChanInfo = DMX_OsiGetChannelProc(ChanId);
	        if(ChanInfo == NULL)
	        {
	            continue;
	        }
			
	       	if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode)) 
			{
	           if(type == DMX_DEBUG_CMD_SAVE_VES)
	           {
	           		printk(KERN_ERR "[%s %d]ChanInfo->ChanType=%d\n", __FUNCTION__, __LINE__, ChanInfo->ChanType);
	                if(MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType)
	                {
	                    DMX_ChanEsBuff_S *EsBuffer = NULL;
	                    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & ChanInfo->ChanStatus)) 
						{
							printk(KERN_ERR "[%s %d]video chan not open\n", __FUNCTION__, __LINE__);
							return MT_ERR_DMX_NOT_OPEN_CHAN;
	                    }

	                    EsBuffer = DMX_OsiGetChannelEsBufProc(ChanInfo->avChanId);
	                    if(EsBuffer)
	                    {
	                        printk(KERN_ERR "[%s %d]video, addr=0x%lx, size=0x%lx\n", __FUNCTION__, __LINE__, (ulong)(EsBuffer->data_buff.startVirAddr), EsBuffer->data_buff.size);
	                        DMX_OsrSaveEs(MT_UNF_DMX_CHAN_TYPE_VID, (mt_u8 *)(EsBuffer->data_buff.startVirAddr), EsBuffer->data_buff.size, 0, p_path);
	                    }
	                }				
	           }
			   
	           if(type == DMX_DEBUG_CMD_SAVE_AES)
	           {
	           		printk(KERN_ERR "[%s %d]ChanInfo->ChanType=%d\n", __FUNCTION__, __LINE__, ChanInfo->ChanType);
	                DMX_ChanEsBuff_S *EsBuffer =NULL;
	                if(MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType || MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType)
	                {
	                    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & ChanInfo->ChanStatus)) 
						{
							printk(KERN_ERR "[%s %d]audio chan not open\n", __FUNCTION__, __LINE__);
	                    	return MT_ERR_DMX_NOT_OPEN_CHAN;
	                    }
						
	                    EsBuffer = DMX_OsiGetChannelEsBufProc(ChanInfo->avChanId);
	                    if(EsBuffer)
	                    {
	                        printk(KERN_ERR "[%s %d]audio, addr=0x%lx, size=0x%lx\n", __FUNCTION__, __LINE__, (ulong)(EsBuffer->data_buff.startVirAddr), EsBuffer->data_buff.size);
	                        DMX_OsrSaveEs(MT_UNF_DMX_CHAN_TYPE_AUD, (mt_u8 *)(EsBuffer->data_buff.startVirAddr), EsBuffer->data_buff.size, 1, p_path);
	                    }
	                }				
	           }			   
	        }
	    }
	}
	
	if (type == DMX_DEBUG_CMD_SAVE_REC)
	{
		for (rec_id=0; rec_id<DMX_REC_CNT; rec_id++)
		{
			DMX_RecInfo_S *RecInfo = &g_pDmxDevOsi->DmxRecInfo[rec_id];
			if (DMX_REC_STATUS_START == RecInfo->RecStatus)
			{
				if (RecInfo->link_mode)
				{	
					struct file *file_handle = MT_NULL;
					struct tm now;
					
					if (MT_NULL == file_handle)
                    {
                        mt_char str[DMX_FILE_NAME_LEN]  = {0};
						time64_to_tm(ktime_get_seconds(), 0, &now);						
                        snprintf(str, sizeof(str), "%s/dmx_link_rec_%u-%02u_%02u_%02u.ts", p_path, rec_id, now.tm_hour, now.tm_min, now.tm_sec);
                        
						printk(KERN_ERR "save file: %s\n", str);

                        file_handle = mt_drv_file_open(str, 1);
                        if (!file_handle)
                        {
                            MT_ERR_DEMUX("open %s error\n", str);
                            return MT_FAILURE;
                        }
                    }
					
					for (i=0; i<RecInfo->link_node_num; i++)
					{
						printk(KERN_ERR "[%s %d]link_rec, i=%d, rec_id=%d, addr=0x%lx, size=0x%lx\n", __FUNCTION__, __LINE__, i, rec_id, (ulong)(RecInfo->link_rec_buf[i].startVirAddr), RecInfo->link_rec_buf[i].size);			            
						mt_drv_file_write(file_handle, (mt_u8 *)(RecInfo->link_rec_buf[i].startVirAddr), RecInfo->link_rec_buf[i].size);
					}
				
                    if (file_handle)
                    {
                        mt_drv_file_close(file_handle);
                        file_handle = MT_NULL;
                    }
				}
				else
				{
					printk(KERN_ERR "[%s %d]rec, rec_id=%d, addr=0x%lx, size=0x%lx\n", __FUNCTION__, __LINE__, rec_id, (ulong)(RecInfo->RecBuffer.startVirAddr), RecInfo->RecBuffer.size);
		            DMX_OsrSaveEs(MT_UNF_DMX_CHAN_TYPE_REC, (mt_u8 *)(RecInfo->RecBuffer.startVirAddr), RecInfo->RecBuffer.size, rec_id, p_path);
				}
			}
		}					
	}	
    return 0;
}

static mt_s32 DMX_OsrStartSaveEs(mt_void)
{
    mt_char path[DMX_FILE_NAME_LEN] = {0};

    if (SaveEsFlag)
    {
        return MT_SUCCESS;
    }


    if (MT_SUCCESS != mt_drv_file_get_storepath(path, DMX_FILE_NAME_LEN))
    {
        MT_ERR_DEMUX("get path failed\n");

        return MT_FAILURE;
    }
    if (0 == down_interruptible(&SaveEsMutex))
    {
        memset(DmxEsHandle,0,DMX_AV_CHANNEL_CNT*sizeof(struct file *));
        SaveEsFlag = 1;
        up(&SaveEsMutex);
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}

static mt_void DMX_OsrStopSaveEs(mt_void)
{
    mt_s32 i;
    if (0 == down_interruptible(&SaveEsMutex))
    {
        for (i = 0; i < DMX_AV_CHANNEL_CNT; i++)
        {
            if (DmxEsHandle[i])
            {
                mt_drv_file_close(DmxEsHandle[i]);
                DmxEsHandle[i] = MT_NULL;
            }
        }

        SaveEsFlag = 0;
        up(&SaveEsMutex);
    }

}

MT_DECLARE_MUTEX(DmxRamPortTsMutex);

static struct file *DmxRamPortTsHandle  = MT_NULL;
static mt_u32       DmxRamPortID         = 0;

mt_void DMX_OsrSaveIPTs(mt_u8 *buf, mt_u32 len,mt_u32 u32PortID)
{
    if (0 == down_interruptible(&DmxRamPortTsMutex))
    {
        if (DmxRamPortTsHandle && (DmxRamPortID - MT_UNF_DMX_PORT_RAM_0) == u32PortID)
        {
            mt_drv_file_write(DmxRamPortTsHandle, buf, len);
        }
        up(&DmxRamPortTsMutex);
    }
}

static mt_s32 DMX_OsrSaveIPTsStart(mt_u32 u32PortID)
{
    char str[DMX_FILE_NAME_LEN];
    struct tm now;

    if ((u32PortID < MT_UNF_DMX_PORT_RAM_0) || (u32PortID  > (MT_UNF_DMX_PORT_RAM_0 + DMX_RAMPORT_CNT)))
    {
        MT_ERR_DEMUX("invalid port id:%d\n",u32PortID);
        return MT_FAILURE;
    }

    if (MT_SUCCESS != mt_drv_file_get_storepath(str, DMX_FILE_NAME_LEN))
    {
        MT_ERR_DEMUX("get path failed\n");

        return MT_FAILURE;
    }
    DmxRamPortID = u32PortID;

 if (0 == down_interruptible(&DmxRamPortTsMutex))
    {
        if (DmxRamPortTsHandle == MT_NULL)
        {
            //time_to_tm(get_seconds(), 0, &now);
						time64_to_tm(ktime_get_seconds(), 0, &now);
            snprintf(str, sizeof(str),"%s/dmx_ram_%u-%02u_%02u_%02u.ts", str, u32PortID, now.tm_hour, now.tm_min, now.tm_sec);

            DmxRamPortTsHandle = mt_drv_file_open(str, 1);
            if (!DmxRamPortTsHandle)
            {
                MT_ERR_DEMUX("open %s error\n", str);
                up(&DmxRamPortTsMutex);
                return MT_FAILURE;
            }
        }
        up(&DmxRamPortTsMutex);
    }

    return MT_SUCCESS;
}

static mt_void DMX_OsrSaveIPTsStop(mt_void)
{
if (0 == down_interruptible(&DmxRamPortTsMutex))
    {
    if (DmxRamPortTsHandle)
    {
        mt_drv_file_close(DmxRamPortTsHandle);
        DmxRamPortTsHandle = MT_NULL;
        DmxRamPortID = 0;
    }
     up(&DmxRamPortTsMutex);
    }
}

#define DMX_ALLTS_DMXID (4)
#define DMX_DMXTS_DMXID (4)

#define DMX_INVALID_REC_ID  0xFFFF

static struct file         *DmxAllTsHandle  = MT_NULL;
static struct task_struct  *DmxAllTsThread  = MT_NULL;
static mt_u32               DmxAllTsRecId   = DMX_INVALID_REC_ID;

static mt_s32 MT_DMX_SaveAllTS_Routine(mt_void *arg)
{
    while (1)
    {
        mt_u32  RecId   = DmxAllTsRecId;
        phys_addr_t  PhyAddr;
        ulong  KerAddr;
        mt_u32  Len;

        if (kthread_should_stop())
        {
            break;
        }

        if (DMX_INVALID_REC_ID == RecId)
        {
            continue;
        }

        if (MT_SUCCESS == DMX_DRV_REC_AcquireRecData(RecId, &PhyAddr, &KerAddr, &Len, 500))
        {
            if (DmxAllTsHandle)
            {
                mt_drv_file_write(DmxAllTsHandle, (mt_s8*)KerAddr, Len);
            }

            DMX_DRV_REC_ReleaseRecData(RecId, PhyAddr, Len);

        }

        yield();
    }

    return 0;
}

static mt_s32 DMX_OsrSaveALLTs_Start(mt_u32 PortId)
{
    mt_s32                  ret;
    DMX_PORT_MODE_E         PortMode;
    MT_UNF_DMX_REC_ATTR_S   RecAttr;
    phys_addr_t             PhyAddr;
    mt_u32                  BufSize;
    phys_addr_t             IdxPhyAddr;
    mt_u32                  IdxBufSize;
    mt_char                 FileName[DMX_FILE_NAME_LEN] = {0};
    DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp = DMX_REC_TIMESTAMP_NONE;
    struct tm now;

    ret = mt_drv_file_get_storepath(FileName, DMX_FILE_NAME_LEN);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("get path failed\n");

        return ret;
    }


    if (MT_NULL != DmxAllTsHandle)
    {
        MT_ERR_DEMUX("already started\n");

        return MT_FAILURE;
    }

    //time_to_tm(get_seconds(), 0, &now);
    time64_to_tm(ktime_get_seconds(), 0, &now);
    snprintf(FileName, sizeof(FileName),"%s/dmx_allts_%u-%02u_%02u_%02u.ts", FileName, PortId, now.tm_hour, now.tm_min, now.tm_sec);

    DmxAllTsHandle = mt_drv_file_open(FileName, 1);
    if (!DmxAllTsHandle)
    {
        MT_ERR_DEMUX("open %s error\n", FileName);

        goto exit;
    }

    if (PortId >= MT_UNF_DMX_PORT_RAM_0)
    {
        PortMode = DMX_PORT_MODE_RAM;

        PortId -= MT_UNF_DMX_PORT_RAM_0;
    }
    else
    {
        PortMode = DMX_PORT_MODE_TUNER;
        if(PortId >= MT_UNF_DMX_PORT_TSI_0)
        {
            PortId = PortId - MT_UNF_DMX_PORT_TSI_0 + DMX_IFPORT_CNT;
        }
    }

    ret = DMX_OsiAttachPort(DMX_ALLTS_DMXID, PortMode, PortId);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("AttachPort failed 0x%x\n", ret);

        goto exit;
    }

    RecAttr.u32DmxId        = DMX_ALLTS_DMXID;
    RecAttr.u32RecBufSize   = 0x400000;
    RecAttr.enRecType       = MT_UNF_DMX_REC_TYPE_ALL_PID;

    ret = DMX_DRV_REC_CreateChannel(&RecAttr, enRecTimeStamp,&DmxAllTsRecId, &PhyAddr, &BufSize, &IdxPhyAddr, &IdxBufSize);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("open rec failed 0x%x\n", ret);

        goto exit;
    }

    ret = DMX_DRV_REC_StartRecChn(DmxAllTsRecId);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("start rec failed 0x%x\n", ret);

        goto exit;
    }

    DmxAllTsThread = kthread_create(MT_DMX_SaveAllTS_Routine, MT_NULL, "SaveAllTs");
    if (IS_ERR(DmxAllTsThread))
    {
        MT_ERR_DEMUX("create kthread failed\n");

        goto exit;
    }

    wake_up_process(DmxAllTsThread);


    return MT_SUCCESS;

exit :
    if (DMX_INVALID_REC_ID != DmxAllTsRecId)
    {
        DMX_DRV_REC_StopRecChn(DmxAllTsRecId);

        DMX_DRV_REC_DestroyChannel(DmxAllTsRecId);

        DmxAllTsRecId = DMX_INVALID_REC_ID;
    }

    DMX_OsiDetachPort(DMX_ALLTS_DMXID);

    if (DmxAllTsHandle)
    {
        mt_drv_file_close(DmxAllTsHandle);
        DmxAllTsHandle = MT_NULL;
    }

    return MT_FAILURE;
}

static mt_void DMX_OsrSaveALLTs_Stop(mt_void)
{
    if (DmxAllTsHandle)
    {
        kthread_stop(DmxAllTsThread);
        DmxAllTsThread = MT_NULL;

        DMX_DRV_REC_StopRecChn(DmxAllTsRecId);

        DMX_DRV_REC_DestroyChannel(DmxAllTsRecId);

        DmxAllTsRecId = DMX_INVALID_REC_ID;

        DMX_OsiDetachPort(DMX_ALLTS_DMXID);

        mt_drv_file_close(DmxAllTsHandle);
        DmxAllTsHandle = MT_NULL;

    }
}

static struct task_struct  *DmxRecTsThread  = MT_NULL;
static struct file         *DmxRecTsHandle  = MT_NULL;
static mt_u32               DmxTsRecId      = DMX_INVALID_REC_ID;

static mt_s32 MT_DMX_SaveDmxTS_Routine(mt_void *arg)
{
    while (1)
    {
        mt_u32  RecId   = DmxTsRecId;
        phys_addr_t  PhyAddr;
        ulong  KerAddr;
        mt_u32  Len;

        if (kthread_should_stop())
        {
            break;
        }

        if (DMX_INVALID_REC_ID == RecId)
        {
            continue;
        }

        if (MT_SUCCESS == DMX_DRV_REC_AcquireRecData(RecId, &PhyAddr, &KerAddr, &Len, 500))
        {
            if (DmxRecTsHandle)
            {
                mt_drv_file_write(DmxRecTsHandle, (mt_s8*)KerAddr, Len);
            }

            DMX_DRV_REC_ReleaseRecData(RecId, PhyAddr, Len);
        }

        yield();
    }

    return 0;
}

static mt_s32 DMX_OsrSaveDmxTs_Start(mt_u32 DmxId)
{
    mt_s32                  ret;
    MT_UNF_DMX_REC_ATTR_S   RecAttr;
    phys_addr_t             PhyAddr;
    mt_u32                  BufSize;
    phys_addr_t             IdxPhyAddr;
    mt_u32                  IdxBufSize;
    mt_char                 FileName[DMX_FILE_NAME_LEN] = {0};
    DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp = DMX_REC_TIMESTAMP_NONE;
    struct tm now;

    ret = mt_drv_file_get_storepath(FileName, DMX_FILE_NAME_LEN);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("get path failed\n");

        return MT_FAILURE;
    }

    if (MT_NULL != DmxRecTsHandle)
    {
        MT_ERR_DEMUX("already started\n");

        return MT_FAILURE;
    }

    ret = DMX_OsiSaveDmxTs_Start(DmxId, DMX_DMXTS_DMXID);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("ts start failed\n");

        return MT_FAILURE;
    }

    //time_to_tm(get_seconds(), 0, &now);
		time64_to_tm(ktime_get_seconds(), 0, &now);
    snprintf(FileName,sizeof(FileName), "%s/dmx_rects_%u-%02u_%02u_%02u.ts", FileName, DmxId, now.tm_hour, now.tm_min, now.tm_sec);

    DmxRecTsHandle = mt_drv_file_open(FileName, 1);
    if (!DmxRecTsHandle)
    {
        MT_ERR_DEMUX("open %s error\n", FileName);

        goto exit;
    }

    RecAttr.u32DmxId        = DMX_DMXTS_DMXID;
    RecAttr.enIndexType     = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
    RecAttr.u32IndexSrcPid  = DMX_INVALID_PID;
    RecAttr.enVCodecType    = MT_UNF_VCODEC_TYPE_MPEG2;
    RecAttr.enRecType       = MT_UNF_DMX_REC_TYPE_SELECT_PID;
    RecAttr.u32RecBufSize   = 0x400000;

    ret = DMX_DRV_REC_CreateChannel(&RecAttr, enRecTimeStamp,&DmxTsRecId, &PhyAddr, &BufSize, &IdxPhyAddr, &IdxBufSize);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("open rec failed 0x%x\n", ret);

        goto exit;
    }

    ret = DMX_DRV_REC_StartRecChn(DmxTsRecId);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("start rec failed 0x%x\n", ret);

        goto exit;
    }

    DmxRecTsThread = kthread_create(MT_DMX_SaveDmxTS_Routine, MT_NULL, "SaveDmxTs");
    if (IS_ERR(DmxRecTsThread))
    {
        MT_ERR_DEMUX("create kthread failed\n");

        goto exit;
    }

    wake_up_process(DmxRecTsThread);

    return MT_SUCCESS;

exit :
    if (DMX_INVALID_REC_ID != DmxTsRecId)
    {
        DMX_DRV_REC_StopRecChn(DmxTsRecId);

        DMX_DRV_REC_DestroyChannel(DmxTsRecId);

        DmxTsRecId = DMX_INVALID_REC_ID;
    }

    if (DmxRecTsHandle)
    {
        mt_drv_file_close(DmxRecTsHandle);
        DmxRecTsHandle = MT_NULL;
    }

    DMX_OsiSaveDmxTs_Stop(DMX_DMXTS_DMXID);

    return MT_FAILURE;
}

static mt_void DMX_OsrSaveDmxTs_Stop(mt_void)
{
    if (DmxRecTsHandle)
    {
        kthread_stop(DmxRecTsThread);
        DmxRecTsThread = MT_NULL;

        DMX_DRV_REC_StopRecChn(DmxTsRecId);

        DMX_DRV_REC_DestroyChannel(DmxTsRecId);

        DmxTsRecId = DMX_INVALID_REC_ID;

        DMX_OsiSaveDmxTs_Stop(DMX_DMXTS_DMXID);

        mt_drv_file_close(DmxRecTsHandle);
        DmxRecTsHandle = MT_NULL;
    }

}

/* control function entry by proc file system */
mt_s32 DMX_OsrDebugCtrl(mt_u32 cmd, DMX_DEBUG_CMD_CTRl cmdctrl, mt_u32 param, char *p_path)
{
    mt_s32 ret;
    switch (cmd)
    {
        case DMX_DEBUG_CMD_SAVE_VES:
        {
            ret = DMX_OsrStartSaveEs();
            if (MT_SUCCESS == ret)
            {
                printk(KERN_ERR "begin save ves\n");
            }
            DMX_OsrSaveEsDataToFile(DMX_DEBUG_CMD_SAVE_VES, p_path);
            
            DMX_OsrStopSaveEs();
			printk(KERN_ERR "save ves finished\n");
			
            break;
        }
        
        case DMX_DEBUG_CMD_SAVE_AES:
        {            
            ret = DMX_OsrStartSaveEs();
            if (MT_SUCCESS == ret)
            {
                printk(KERN_ERR "begin save ves\n");
            }
            DMX_OsrSaveEsDataToFile(DMX_DEBUG_CMD_SAVE_AES, p_path);
       
            DMX_OsrStopSaveEs();
			printk(KERN_ERR "save aes finished\n");
			
            break;
        }

		case DMX_DEBUG_CMD_SAVE_REC:
        {            
            ret = DMX_OsrStartSaveEs();
            if (MT_SUCCESS == ret)
            {
                printk(KERN_ERR "begin save rec\n");
            }
            DMX_OsrSaveEsDataToFile(DMX_DEBUG_CMD_SAVE_REC, p_path);
       
            DMX_OsrStopSaveEs();
			printk(KERN_ERR "save rec finished\n");
			
            break;
        }
        
        case DMX_DEBUG_CMD_SAVE_ALLTS:
        {
            if (DMX_DEBUG_CMD_STOP == cmdctrl)
            {
                MT_PRINT("stop allts save\n");
                DMX_OsrSaveALLTs_Stop();
            }
            else
            {
                ret = DMX_OsrSaveALLTs_Start(param);
                if (MT_SUCCESS == ret)
                {
                    MT_PRINT("begin save allts of port:%d\n",param);
                }
            }

            break;
        }

        case DMX_DEBUG_CMD_SAVE_IPTS:
        {
            if (DMX_DEBUG_CMD_START == cmdctrl)
            {
                ret = DMX_OsrSaveIPTsStart(param);
                if (MT_SUCCESS == ret)
                {
                    MT_PRINT("begin save ram port:%d ts\n",param);
                }
            }
            else
            {
                MT_PRINT("stop save ip port ts\n");
                DMX_OsrSaveIPTsStop();
            }

            break;
        }

        case DMX_DEBUG_CMD_SAVE_DMXTS:
        {
            if (DMX_DEBUG_CMD_STOP == cmdctrl)
            {
                MT_PRINT("stop save dmx ts\n");
                DMX_OsrSaveDmxTs_Stop();
            }
            else
            {
                ret = DMX_OsrSaveDmxTs_Start(param);
                if (MT_SUCCESS == ret)
                {
                    MT_PRINT("begine save dmx:%d ts\n",param);
                }
            }

            break;
        }

        case DMX_DEBUG_CMD_PRINT_INFO:
        {
            g_dmx_loglevel = param;
            MT_PRINT("print dmx info: %#x\n", g_dmx_loglevel);
            break;
        }

        default:
            return MT_FAILURE;
    }

    return MT_SUCCESS;
}

#endif

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 MT_DRV_DMX_Channel_Ci_Enable(mt_handle hChannel, mt_u32 enable)
{
    DMX_CHECK_CHANHANDLE(hChannel);

    return DMX_OsiChannelCiEnable(DMX_CHANID(hChannel), enable);
}

mt_s32 MT_DRV_TSI_CI_Rec_Channel_Cfg(mt_handle hChannel)
{	
    DMX_CHECK_RECHANDLE(hChannel);
	return DMX_OsiCiRecChanCfg(DMX_RECID(hChannel));
}

mt_s32 MT_DRV_TSI_CI_Swtsi_Channel_Cfg(mt_handle hChannel)
{	
    CHECKRAMPORTID(hChannel);
	return DMX_OsiCiSwtsiChanCfg(hChannel);
}

mt_s32 MT_DRV_TSI_CI_Buf_Cfg(mt_u8 usecache)
{
	return DMX_OsiCiBufCfg(usecache);
}

mt_s32 MT_DRV_TSI_CI_Enable_Cfg(mt_u8 enable)
{
	return DMX_OsiCiEnableCfg(enable);
}

mt_s32 MT_DRV_TSI_CI_Cicamclk_Cfg(mt_u32 clkdiv)
{
	return DMX_OsiCiCamClkCfg(clkdiv);
}

mt_s32 MT_DRV_TSI_CI_TsInterval_Cfg(mt_u32 tsinterval)
{
	return DMX_OsiCiTsIntervalCfg(tsinterval);
}

mt_s32 MT_DRV_TSI_Tsi2_Source_Cfg(mt_u8 from_cam,mt_u8 serial)
{
	return DMX_OsiTsi2SourceCfg(from_cam,serial);
}

mt_s32 MT_DRV_TSI_CI_LlnStartNum_Cfg(mt_u32 lln_start_num)
{
	return DMX_OsiCiLlnNumStartCfg(lln_start_num);
}

mt_s32 MT_DRV_TSI_CI_SwtsiByteorder_Cfg(mt_u32 byteorder)
{
	return DMX_OsiSwtsiByteorderCfg(byteorder);
}

mt_s32 MT_DRV_DMX_Channel_Swtsi_Full_Cfg(mt_handle hChannel, mt_u32 full)
{
    CHECKRAMPORTID(hChannel);
    return DMX_OsiSwtsiFullcareCfg(hChannel, full);
}

mt_s32 MT_DRV_TSI_CI_AhbRdDelay_Cfg(mt_u32 rddelay)
{	
	return DMX_OsiCiAhbDelayCfg(rddelay);
}

mt_s32 MT_DRV_TSI_CI_GetStatus(mt_u8 *bufstatus,mt_u8 *cistaus)
{
	return DMX_OsiGetCiStatus(bufstatus,cistaus);
}
#endif

/*add end*/

/*opentv5 export these symbol*/
EXPORT_SYMBOL(MT_DRV_DMX_ReadRegister);
EXPORT_SYMBOL(MT_DRV_DMX_WriteRegister);
EXPORT_SYMBOL(MT_DRV_DMX_Init);
EXPORT_SYMBOL(MT_DRV_DMX_DeInit);
EXPORT_SYMBOL(DMX_OsiSuspend);
EXPORT_SYMBOL(DMX_OsiResume);
EXPORT_SYMBOL(MT_DRV_DMX_Open);
EXPORT_SYMBOL(MT_DRV_DMX_Close);
EXPORT_SYMBOL(MT_DRV_DMX_GetPoolBufAddr);
EXPORT_SYMBOL(MT_DRV_DMX_GetCapability);
EXPORT_SYMBOL(MT_DRV_DMX_TunerPortGetAttr);
EXPORT_SYMBOL(MT_DRV_DMX_TunerPortSetAttr);
EXPORT_SYMBOL(MT_DRV_DMX_RamPortGetAttr);
EXPORT_SYMBOL(MT_DRV_DMX_RamPortSetAttr);
EXPORT_SYMBOL(MT_DRV_DMX_AttachTunerPort);
EXPORT_SYMBOL(MT_DRV_DMX_AttachRamPort);
EXPORT_SYMBOL(MT_DRV_DMX_DetachPort);
EXPORT_SYMBOL(MT_DRV_DMX_GetPortId);
EXPORT_SYMBOL(MT_DRV_DMX_TunerPortGetPacketNum);
EXPORT_SYMBOL(MT_DRV_DMX_RamPortGetPacketNum);
EXPORT_SYMBOL(MT_DRV_DMX_CreateTSBuffer);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyTSBuffer);
EXPORT_SYMBOL(MT_DRV_DMX_GetTSBuffer);
EXPORT_SYMBOL(MT_DRV_DMX_PutTSBuffer);
EXPORT_SYMBOL(MT_DRV_DMX_ResetTSBuffer);
EXPORT_SYMBOL(MT_DRV_DMX_GetTSBufferStatus);
EXPORT_SYMBOL(MT_DRV_DMX_CreateChannel);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyChannel);
EXPORT_SYMBOL(MT_DRV_DMX_GetChannelAttr);
EXPORT_SYMBOL(MT_DRV_DMX_SetChannelAttr);
EXPORT_SYMBOL(MT_DRV_DMX_SetChannelPID);
EXPORT_SYMBOL(MT_DRV_DMX_GetChannelPID);
EXPORT_SYMBOL(MT_DRV_DMX_OpenChannel);
EXPORT_SYMBOL(MT_DRV_DMX_CloseChannel);
EXPORT_SYMBOL(MT_DRV_DMX_GetChannelId);
EXPORT_SYMBOL(MT_DRV_DMX_GetChannelStatus);
EXPORT_SYMBOL(MT_DRV_DMX_GetChannelHandle);
EXPORT_SYMBOL(MT_DRV_DMX_GetFreeChannelCount);
EXPORT_SYMBOL(MT_DRV_DMX_GetScrambledFlag);
EXPORT_SYMBOL(MT_DRV_DMX_GetChannelTsCount);
EXPORT_SYMBOL(MT_DRV_DMX_SetChannelEosFlag);
EXPORT_SYMBOL(MT_DRV_DMX_CreateFilter);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyFilter);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyAllFilter);
EXPORT_SYMBOL(MT_DRV_DMX_SetFilterAttr);
EXPORT_SYMBOL(MT_DRV_DMX_GetFilterAttr);
EXPORT_SYMBOL(MT_DRV_DMX_AttachFilter);
EXPORT_SYMBOL(MT_DRV_DMX_DetachFilter);
EXPORT_SYMBOL(MT_DRV_DMX_GetFilterChannelHandle);
EXPORT_SYMBOL(MT_DRV_DMX_GetFreeFilterCount);
EXPORT_SYMBOL(MT_DRV_DMX_GetDataHandle);
EXPORT_SYMBOL(MT_DRV_DMX_SelectDataHandle);
EXPORT_SYMBOL(MT_DRV_DMX_AcquireBuf);
EXPORT_SYMBOL(MT_DRV_DMX_ReleaseBuf);
EXPORT_SYMBOL(MT_DRV_DMX_PeekBuf);
EXPORT_SYMBOL(MT_DRV_DMX_CreatePcrChannel);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyPcrChannel);
EXPORT_SYMBOL(MT_DRV_DMX_PcrPidSet);
EXPORT_SYMBOL(MT_DRV_DMX_PcrPidGet);
EXPORT_SYMBOL(MT_DRV_DMX_PcrScrGet);
EXPORT_SYMBOL(MT_DRV_DMX_PcrSyncAttach);
EXPORT_SYMBOL(MT_DRV_DMX_PcrSyncDetach);
EXPORT_SYMBOL(MT_DRV_DMX_AcquireEs);
EXPORT_SYMBOL(MT_DRV_DMX_ReleaseEs);
EXPORT_SYMBOL(MT_DRV_DMX_GetPESBufferStatus);
EXPORT_SYMBOL(MT_DRV_DMX_CreateRecChn);
EXPORT_SYMBOL(MT_DRV_DMX_DestroyRecChn);
EXPORT_SYMBOL(MT_DRV_DMX_AddRecPid);
EXPORT_SYMBOL(MT_DRV_DMX_DelRecPid);
EXPORT_SYMBOL(MT_DRV_DMX_DelAllRecPid);
EXPORT_SYMBOL(MT_DRV_DMX_AddExcludeRecPid);
EXPORT_SYMBOL(MT_DRV_DMX_DelExcludeRecPid);
EXPORT_SYMBOL(MT_DRV_DMX_DelAllExcludeRecPid);
EXPORT_SYMBOL(MT_DRV_DMX_StartRecChn);
EXPORT_SYMBOL(MT_DRV_DMX_StopRecChn);
EXPORT_SYMBOL(MT_DRV_DMX_AcquireRecData);
EXPORT_SYMBOL(MT_DRV_DMX_ReleaseRecData);
EXPORT_SYMBOL(MT_DRV_DMX_AcquireRecIndex);
EXPORT_SYMBOL(MT_DRV_DMX_GetRecBufferStatus);
EXPORT_SYMBOL(MT_DRV_DMX_GetRecTsCnt);
EXPORT_SYMBOL(MT_DRV_DMX_CreateLinkRecChn);
EXPORT_SYMBOL(MT_DRV_DMX_AcquireLinkRecData);
EXPORT_SYMBOL(MT_DRV_DMX_SetPortId);

/*add for sym6 verify, please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
EXPORT_SYMBOL(MT_DRV_DMX_Channel_Ci_Enable);
EXPORT_SYMBOL(MT_DRV_TSI_CI_Rec_Channel_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_Swtsi_Channel_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_Buf_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_Cicamclk_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_TsInterval_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_Tsi2_Source_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_LlnStartNum_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_SwtsiByteorder_Cfg);
EXPORT_SYMBOL(MT_DRV_DMX_Channel_Swtsi_Full_Cfg);
EXPORT_SYMBOL(MT_DRV_TSI_CI_GetStatus);
EXPORT_SYMBOL(MT_DRV_TSI_CI_Enable_Cfg);

#endif

/*end*/
