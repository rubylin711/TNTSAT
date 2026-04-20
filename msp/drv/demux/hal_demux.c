/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "mt_module.h"
#include "mt_reg_common.h"

#include "mt_module_debug.h"

//#include "demux_debug.h"
#include "drv_demux_reg.h"
#include "drv_demux_define.h"
#include "drv_demux_osal.h"
#include "hal_demux.h"

//#include "basedef.h"
#include "mt_drv_struct.h"
//#include "mt_drv_sys.h"
#include <asm/barrier.h>    /*wmb() */ //we added this for DTS2013091204261
#include <linux/highmem.h>

#include "hal_demux_regs.h"
#if 1
#define DMX_COM_EQUAL(exp, act)
#else
#define DMX_COM_EQUAL(exp, act)                                                         \
    do                                                                                  \
    {                                                                                   \
        if (exp != act)                                                                 \
        {                                                                               \
            MT_ERR_DEMUX("Write register error, exp=0x%x, act=0x%x\n", exp, act);       \
        }                                                                               \
    } while (0)
#endif

/*demux 中对于寄存器操作存在竞争，可能会引起莫名奇妙的错误，review 所有寄存器，统一修改
问题单号: DTS2013082001104 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36))
spinlock_t DmxHalLcok = SPIN_LOCK_UNLOCKED;
#else
spinlock_t DmxHalLcok = __SPIN_LOCK_UNLOCKED(DmxHalLcok);
#endif
/*****************************************add by hwu*********************************/
//#define phys_to_page(x) x
/**************************************************************************/

#define DEMUX_MAP_DDR_PHYADDRESS(phys)      kmap((phys_to_page(phys)))
#define DEMUX_UMMAP_DDR_PHYADDRESS(phys)    kunmap((phys_to_page(phys)))


/*注意,此函数暂时没有考虑CV200扣脉冲计算,因为目前没有使能此功能*/
mt_u32 DmxHalGetClk(mt_void)
{
    mt_u32 Clk = 250;

    return Clk;
}


/***********************************************************************************
* Function      : DmxHalTSOPortSetAttr
* Description   : Set TSO Port
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalTSOPortSetAttr(mt_u32 PortId,MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr)
{
    U_TSOUT_CFG tsout_cfg;
    mt_u32 u32TSOSourcePortID       = 0;

    /*
    0x01～0x07：分别选择DVB下的TS1～TS7端口码流；
    0x10～0x15：分别选择IP下的IP0～IP5内部端口码流。
    */
    if ( PortAttr->enTSSource >= MT_UNF_DMX_PORT_RAM_0 )
    {
        u32TSOSourcePortID = PortAttr->enTSSource - MT_UNF_DMX_PORT_RAM_0 + 0x10;
    }
    else if(PortAttr->enTSSource >= MT_UNF_DMX_PORT_TSI_0)
    {
        u32TSOSourcePortID = PortAttr->enTSSource - MT_UNF_DMX_PORT_TSI_0 + DMX_IFPORT_CNT + 0x1;
    }
    else
    {
        u32TSOSourcePortID = PortAttr->enTSSource + 0x1;
    }

    tsout_cfg.all                   = DMX_READ_REG(TSOUT_CFG(PortId));
    tsout_cfg.bits.dis              = PortAttr->bEnable? 0:1;
    tsout_cfg.bits.tsout_source_sel = u32TSOSourcePortID;
    tsout_cfg.bits.clkgt_mode       = (PortAttr->enClkMode == MT_UNF_DMX_TSO_CLK_MODE_JITTER)?1:0;
    tsout_cfg.bits.vld_mode         = (PortAttr->enValidMode == MT_UNF_DMX_TSO_VALID_ACTIVE_HIGH)?1:0;
    tsout_cfg.bits.sync_pos         = (!PortAttr->bBitSync)?1:0;
    tsout_cfg.bits.spi_mode         = (!PortAttr->bSerial)?1:0;
    tsout_cfg.bits.serial_bit_sel   = (PortAttr->enBitSelector == MT_UNF_DMX_TSO_SERIAL_BIT_7)?1:0;
    if ( tsout_cfg.bits.serial_bit_sel == 0x1 )
    {
        tsout_cfg.bits.byte_endian  = (PortAttr->bLSB )?1:0;
    }
    else
    {
        tsout_cfg.bits.byte_endian  = (PortAttr->bLSB )?0:1;
    }


    DMX_WRITE_REG(TSOUT_CFG(PortId), tsout_cfg.all);
    DMX_COM_EQUAL(tsout_cfg.all, DMX_READ_REG(TSOUT_CFG(PortId)));
}

/***********************************************************************************
* Function      : DmxHalDvbPortSetAttr
* Description   : Set Tuner Port
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDvbPortSetAttr(
        mt_u32                  PortId,
        MT_UNF_DMX_PORT_TYPE_E  PortType,
        mt_u32                  SyncOn,
        mt_u32                  SyncOff,
        mt_u32                  TunerInClk,
        mt_u32                  BitSelector
    )
{
    U_TS_INTERFACE  ts_interface;

    ts_interface.all = DMX_READ_REG(TS_INTERFACE(PortId));

    ts_interface.bits.port_sel      = 0;
    ts_interface.bits.sync_clear    = 1;
    DMX_WRITE_REG(TS_INTERFACE(PortId), ts_interface.all);

    DMX_COM_EQUAL(ts_interface.all, DMX_READ_REG(TS_INTERFACE(PortId)));

    msleep(2);

    switch (PortType)
    {
        case MT_UNF_DMX_PORT_TYPE_PARALLEL_BURST :
        {
            ts_interface.bits.serial_sel    = 0;
            ts_interface.bits.sync_mode     = 0;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188 :
        {
            ts_interface.bits.serial_sel        = 0;
            ts_interface.bits.sync_mode         = 2;
            ts_interface.bits.nosync_fixed_204  = 0;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_204 :
        {
            ts_interface.bits.serial_sel        = 0;
            ts_interface.bits.sync_mode         = 2;
            ts_interface.bits.nosync_fixed_204  = 1;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204 :
        {
            ts_interface.bits.serial_sel    = 0;
            ts_interface.bits.sync_mode     = 3;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_SERIAL :
        {
            ts_interface.bits.serial_sel    = 1;
            ts_interface.bits.sync_mode     = 0;
            ts_interface.bits.ser_2bit_mode = 0;
            ts_interface.bits.ser_2bit_rev  = 0;
            ts_interface.bits.ser_nosync    = 0;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_SERIAL2BIT :
        {
            ts_interface.bits.serial_sel    = 1;
            ts_interface.bits.sync_mode     = 0;
            ts_interface.bits.ser_2bit_mode = 1;
            ts_interface.bits.ser_2bit_rev  = 0;
            ts_interface.bits.ser_nosync    = 0;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_SERIAL_NOSYNC :
        {
            ts_interface.bits.serial_sel    = 1;
            ts_interface.bits.sync_mode     = 0;
            ts_interface.bits.ser_2bit_mode = 0;
            ts_interface.bits.ser_2bit_rev  = 0;
            ts_interface.bits.ser_nosync    = 1;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_SERIAL2BIT_NOSYNC :
        {
            ts_interface.bits.serial_sel    = 1;
            ts_interface.bits.sync_mode     = 0;
            ts_interface.bits.ser_2bit_mode = 1;
            ts_interface.bits.ser_2bit_rev  = 0;
            ts_interface.bits.ser_nosync    = 1;

            break;
        }

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID :
        default :
        {
            ts_interface.bits.serial_sel    = 0;
            ts_interface.bits.sync_mode     = 1;
        }
    }

    ts_interface.bits.bit_sel = BitSelector;

    ts_interface.bits.syncon_th     = SyncOn;
    ts_interface.bits.syncoff_th    = SyncOff;
    ts_interface.bits.sync_clear    = 0;

    DMX_WRITE_REG(TS_INTERFACE(PortId), ts_interface.all);

    DMX_COM_EQUAL(ts_interface.all, DMX_READ_REG(TS_INTERFACE(PortId)));

    ts_interface.bits.port_sel = 1;
    DMX_WRITE_REG(TS_INTERFACE(PortId), ts_interface.all);

    DMX_COM_EQUAL(ts_interface.all, DMX_READ_REG(TS_INTERFACE(PortId)));
}


mt_void DmxHalDvbPortSetClkInPol(mt_u32 PortId, MT_BOOL Pol)
{
}

/***********************************************************************************
* Function      : DmxHalDvbPortSetTsCountCtrl
* Description   : Set TS Count Ctrl
* Input         : PortId, option
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDvbPortSetTsCountCtrl(const mt_u32 PortId, const mt_u32 option)
{
    U_TS_COUNT_CTRL ts_count_ctrl;

    ts_count_ctrl.all = DMX_READ_REG(TS_COUNT_CTRL(PortId));

    ts_count_ctrl.bits.ts_count_ctrl = option;
    DMX_WRITE_REG(TS_COUNT_CTRL(PortId), ts_count_ctrl.all);

    DMX_COM_EQUAL(ts_count_ctrl.all, DMX_READ_REG(TS_COUNT_CTRL(PortId)));
}

/***********************************************************************************
* Function      : DmxHalDvbPortGetTsPackCount
* Description   : Get TS Pack Counter
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalDvbPortGetTsPackCount(mt_u32 PortId)
{
    if(PortId == MT_UNF_DMX_PORT_TSI_0)   return  reg_get_ts0_sample_sta_ts_cnt();
    else if(PortId == MT_UNF_DMX_PORT_TSI_1)   return  reg_get_ts1_sample_sta_ts_cnt();
    else if(PortId == MT_UNF_DMX_PORT_TSI_2)   return  reg_get_ts2_sample_sta_ts_cnt();
    else if(PortId == MT_UNF_DMX_PORT_TSI_3)   return  reg_get_ts3_sample_sta_ts_cnt();
    else return 0;
}

/***********************************************************************************
* Function      : DmxHalDvbPortSetErrTsCountCtrl
* Description   : Set ETS Count Ctrl
* Input         : PortId, option
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDvbPortSetErrTsCountCtrl(const mt_u32 PortId, const mt_u32 option)
{
    U_ETS_COUNT_CTRL ets_count_ctrl;

    ets_count_ctrl.all = DMX_READ_REG(ETS_COUNT_CTRL(PortId));

    ets_count_ctrl.bits.ets_count_ctrl = option;
    DMX_WRITE_REG(ETS_COUNT_CTRL(PortId), ets_count_ctrl.all);

    DMX_COM_EQUAL(ets_count_ctrl.all, DMX_READ_REG(ETS_COUNT_CTRL(PortId)));
}

/***********************************************************************************
* Function      : DmxHalDvbPortGetErrTsPackCount
* Description   : Get Error TS Pack Count
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalDvbPortGetErrTsPackCount(mt_u32 PortId)
{
    if(PortId == MT_UNF_DMX_PORT_TSI_0)   return  reg_get_ts0_sample_sta_err_cnt();
    else if(PortId == MT_UNF_DMX_PORT_TSI_1)   return  reg_get_ts1_sample_sta_err_cnt();
    else if(PortId == MT_UNF_DMX_PORT_TSI_2)   return  reg_get_ts2_sample_sta_err_cnt();
    else if(PortId == MT_UNF_DMX_PORT_TSI_3)   return  reg_get_ts3_sample_sta_err_cnt();
    else return 0;
}

mt_void DmxHalIPPortSetAttr(mt_u32 PortId, MT_UNF_DMX_PORT_TYPE_E PortType, mt_u32 SyncOn, mt_u32 SyncOff)
{
    U_IP_SYNC_TH_CFG ipsync_cfg;

    ipsync_cfg.all = DMX_READ_REG(IP_SYNC_TH_CFG(PortId));

    switch (PortType)
    {
        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188 :
            ipsync_cfg.bits.ip_sync_type = 0;
            break;

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_204 :
            ipsync_cfg.bits.ip_sync_type = 1;
            break;

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204 :
        case MT_UNF_DMX_PORT_TYPE_USER_DEFINED :
            ipsync_cfg.bits.ip_sync_type = 2;
            break;

    #ifdef DMX_RAM_PORT_AUTO_SCAN_SUPPORT
        case MT_UNF_DMX_PORT_TYPE_AUTO :
            ipsync_cfg.bits.ip_sync_type = 3;
            break;
    #endif

        default :
            ipsync_cfg.bits.ip_sync_type = 2;
    }

    ipsync_cfg.bits.ip_sync_th = SyncOn;
    ipsync_cfg.bits.ip_loss_th = SyncOff;

    DMX_WRITE_REG(IP_SYNC_TH_CFG(PortId), ipsync_cfg.all);

    DMX_COM_EQUAL(ipsync_cfg.all, DMX_READ_REG(IP_SYNC_TH_CFG(PortId)));
}

mt_void DmxHalIPPortSetSyncLen(mt_u32 PortId, mt_u32 SyncLen1, mt_u32 SyncLen2)
{
    U_DMX_SYNC_LEN_SET SyncSet;

    SyncSet.value = DMX_READ_REG(IP_SYNC_LEN(PortId));
    SyncSet.bits.ip_nosync_len1 = SyncLen1;
    SyncSet.bits.ip_nosync_len2 = SyncLen2;

    DMX_WRITE_REG(IP_SYNC_LEN(PortId), SyncSet.value);

    DMX_COM_EQUAL(SyncSet.value, DMX_READ_REG(IP_SYNC_LEN(PortId)));
}

#ifdef DMX_RAM_PORT_AUTO_SCAN_SUPPORT
mt_void DmxHalIPPortSetAutoScanRegion(mt_u32 PortId, mt_u32 len, mt_u32 step)
{
    U_DMX_SYNC_LEN_SET SyncSet;

    SyncSet.value = DMX_READ_REG(IP_SYNC_LEN(PortId));
    SyncSet.bits.ip_nosync_region   = len;
    SyncSet.bits.ip_nosync_step     = step;

    DMX_WRITE_REG(IP_SYNC_LEN(PortId), SyncSet.value);

    DMX_COM_EQUAL(SyncSet.value, DMX_READ_REG(IP_SYNC_LEN(PortId)));
}
#endif

/***********************************************************************************
* Function      : DmxHalIPPortSetTsCountCtrl
* Description   : Set TS Pack Count
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortSetTsCountCtrl(const mt_u32 PortId, const MT_BOOL enable)
{
    U_IP_DBG_CNT_EN ip_count_en;

    ip_count_en.all = DMX_READ_REG(IP_DBG_CNT_EN(PortId));

    ip_count_en.bits.ip_dbg_cnt_en = enable;
    DMX_WRITE_REG(IP_DBG_CNT_EN(PortId), ip_count_en.all);

    DMX_COM_EQUAL(ip_count_en.all, DMX_READ_REG(IP_DBG_CNT_EN(PortId)));
}

/***********************************************************************************
* Function      : DmxHalIPPortGetTsPackCount
* Description   : Get TS Pack Counter Status
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalIPPortGetTsPackCount(mt_u32 PortId)
{
    return DMX_READ_REG(IP_DBG_OUT1(PortId));
}

/***********************************************************************************
* Function      : DmxHalIPPortStartStream
* Description   : Start pushing stream
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortStartStream(const mt_u32 PortId, const MT_BOOL Enable)
{
    U_IP_TS_OUT_STOP ipout_en;

    ipout_en.all = DMX_READ_REG(IP_TS_OUT_STOP(PortId));

    ipout_en.bits.ip_stop_en = Enable ? 0 : 1;
    DMX_WRITE_REG(IP_TS_OUT_STOP(PortId), ipout_en.all);

    DMX_COM_EQUAL(ipout_en.all, DMX_READ_REG(IP_TS_OUT_STOP(PortId)));
}

mt_void DmxHalIPPortRateSet(mt_u32 PortId, mt_u32 Rate)
{
    DMX_WRITE_REG(IP_TS_RATE_CFG(PortId), Rate);
    DMX_COM_EQUAL(Rate, DMX_READ_REG(IP_TS_RATE_CFG(PortId)));
}

/***********************************************************************************
* Function      : DmxHalIPPortDescSet
* Description   : Set IP descriptors
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortDescSet(mt_u32 PortId, mt_u32 StartAddr, mt_u32 Depth)
{
    U_IP_DESC_SIZE ipsize;

    ipsize.all = DMX_READ_REG(IP_DESC_SIZE(PortId));

    ipsize.bits.ip_desc_size = Depth;
    DMX_WRITE_REG(IP_DESC_SIZE(PortId), ipsize.all);

    DMX_COM_EQUAL(ipsize.all, DMX_READ_REG(IP_DESC_SIZE(PortId)));

    DMX_WRITE_REG(IP_DESC_SADDR(PortId), StartAddr);

    DMX_COM_EQUAL(StartAddr, DMX_READ_REG(IP_DESC_SADDR(PortId)));
}

/***********************************************************************************
* Function      : DmxHalIPPortDescAdd
* Description   : add descriptors
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortDescAdd(const mt_u32 PortId, const mt_u32 DescNum)
{
    U_IP_DESC_ADD DescAdd;

    wmb();/*sync the DDR*/
    DescAdd.all = DMX_READ_REG(IP_DESC_ADD(PortId));

    DescAdd.bits.ip_desc_add = DescNum;
    DMX_WRITE_REG(IP_DESC_ADD(PortId), DescAdd.all);

    DMX_COM_EQUAL(DescAdd.all, DMX_READ_REG(IP_DESC_ADD(PortId)));
}

/***********************************************************************************
* Function      : DmxHalIPPortSetIntCnt
* Description   : Set ip out int level
* Input         : PortId, PackeNum
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortSetIntCnt(mt_u32 PortId, mt_u32 DescNum)
{
    U_IP_INT_CNT_CFG unIpIntCfg;

    unIpIntCfg.all = DMX_READ_REG(IP_INT_CNT_CFG(PortId));

    unIpIntCfg.bits.ip_int_cfg = DescNum;
    DMX_WRITE_REG(IP_INT_CNT_CFG(PortId), unIpIntCfg.all);

    DMX_COM_EQUAL((unIpIntCfg.all & 0xf), (DMX_READ_REG(IP_INT_CNT_CFG(PortId)) & 0xf));
}

/***********************************************************************************
* Function      : DmxHalIPPortGetOutIntStatus
* Description   : Get desc_out int status
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalIPPortGetOutIntStatus(mt_u32 PortId)
{
    U_IP_IINT_INT ip_int;

    ip_int.all = DMX_READ_REG(IP_IINT_INT(PortId));

    return ip_int.bits.ip_iint_desc_out;
}

/***********************************************************************************
* Function      : DmxHalIPPortClearOutIntStatus
* Description   : clear desc_out int status
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortClearOutIntStatus(mt_u32 PortId)
{
    U_IP_IRAW_INT ip_raw_int;

    ip_raw_int.all = DMX_READ_REG(IP_IRAW_INT(PortId));

    ip_raw_int.bits.ip_iraw_desc_out = 1;
    DMX_WRITE_REG(IP_IRAW_INT(PortId), ip_raw_int.all);
}

/***********************************************************************************
* Function      : DmxHalIPPortSetOutInt
* Description   : Set desc_out int enale
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortSetOutInt(const mt_u32 PortId, const MT_BOOL Enable)
{
    U_IP_IENA_INT ip_int_ena;

    ip_int_ena.all = DMX_READ_REG(IP_IENA_INT(PortId));

    ip_int_ena.bits.ip_iena_desc_out = Enable;
    DMX_WRITE_REG(IP_IENA_INT(PortId), ip_int_ena.all);

    DMX_COM_EQUAL(ip_int_ena.all, DMX_READ_REG(IP_IENA_INT(PortId)));
}

/***********************************************************************************
* Function      : DmxHalIPPortDescGetRead
* Description   : Get desc read offset
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalIPPortDescGetRead(mt_u32 PortId)
{
    U_IP_DESC_PTR ip_ptr;

    ip_ptr.all = DMX_READ_REG(IP_DESC_PTR(PortId));

    return ip_ptr.bits.ip_desc_rptr;
}

/***********************************************************************************
* Function      : DmxHalIPPortEnableInt
* Description   : Set IP int enable
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortEnableInt(mt_u32 PortId)
{
    U_ENA_INT_TYPE  unEnaErr;
    U_IP_IENA_INT   unEnaIp;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaIp.all = DMX_READ_REG(IP_IENA_INT(PortId));

    switch (PortId)
    {
        case 0:
            unEnaErr.bits.iena_ip0_all = 1;
            break;

        case 1:
            unEnaErr.bits.iena_ip1_all = 1;
            break;

        case 2:
            unEnaErr.bits.iena_ip2_all = 1;
            break;

        case 3:
            unEnaErr.bits.iena_ip3_all = 1;
            break;

        case 4:
            unEnaErr.bits.iena_ip4_all = 1;
            break;

        case 5:
            unEnaErr.bits.iena_ip5_all = 1;
            break;

        default:
            return;
    }

    unEnaIp.bits.ip_iena_all = 1;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
    DMX_WRITE_REG(IP_IENA_INT(PortId), unEnaIp.all);
}

/***********************************************************************************
* Function      : DmxHalIPPortDisableInt
* Description   : Set IP int disable
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortDisableInt(mt_u32 PortId)
{
    U_ENA_INT_TYPE unEnaErr;
    U_IP_IENA_INT unEnaIp;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaIp.all = DMX_READ_REG(IP_IENA_INT(PortId));
    switch (PortId)
    {
    case 0:
        unEnaErr.bits.iena_ip0_all = 0;
        break;
    case 1:
        unEnaErr.bits.iena_ip1_all = 0;
        break;
    case 2:
        unEnaErr.bits.iena_ip2_all = 0;
        break;
    case 3:
        unEnaErr.bits.iena_ip3_all = 0;
        break;
    default:
        return;
    }

    unEnaIp.bits.ip_iena_all = 0;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
    DMX_WRITE_REG(IP_IENA_INT(PortId), unEnaIp.all);
}


mt_void DmxHalIPPortGetBPStatus(mt_u32 PortId, DMX_Proc_RamPort_BPStatus_S *BPStatus)
{

    mt_u32  offset;
    mt_u32 status32;
    mt_u32 status40;
    mt_u32 regaddr;
    mt_u32 OverflowBp;
    mt_u32 SwitchBufferBp;
   
    offset = 0;
    BPStatus->FQLow32BPEn = DMX_READ_REG(IP_BP_FQ_CFG(PortId, offset));/*0xC220*/
    offset = 1;
    BPStatus->FQHigh8BPEn = DMX_READ_REG(IP_BP_FQ_CFG(PortId, offset));/*0xC224*/

    IP_BP_FQ_STA(PortId, 0,regaddr);
    status32 = DMX_READ_REG(regaddr);
    IP_BP_FQ_STA(PortId, 1,regaddr);
    status40 = DMX_READ_REG(regaddr) & 0xff;

    BPStatus->FQBP =  (MT_BOOL)(status32 || status40);

    OverflowBp                  =  DMX_READ_REG(IP_CHN_BP_STA) & 0x7;    
    BPStatus->OverflowBp        =  (PortId == OverflowBp)? MT_TRUE:MT_FALSE;
    SwitchBufferBp              =   (DMX_READ_REG(IP_CHN_BP_STA) >> 8 ) & 0x7;
    BPStatus->SwitchBufferBp    =  (PortId == SwitchBufferBp)? MT_TRUE:MT_FALSE;
    BPStatus->Rate              =   DMX_READ_REG(IP_TS_RATE_CFG(PortId)) ;
    BPStatus->DebugEn           =   DMX_READ_REG(IP_DBG_CNT_EN(PortId)) ;
    BPStatus->OutByte           =   DMX_READ_REG(IP_DBG_OUT0(PortId));
    BPStatus->OutTSNum          =   DMX_READ_REG(IP_DBG_OUT1(PortId));
    BPStatus->BPCount           =   DMX_READ_REG(IP_DBG_OUT2(PortId) >> 24) & 0xff;

    return ;
}

mt_void DmxHalIPPortGetDescInfo(mt_u32 PortId, DMX_Proc_RamPort_DescInfo_S *DescInfo)
{
    mt_u32 DescWNodeAddr;
    mt_u32 DescRNodeAddr;
    mt_u32* DescWNodeVirAddr = NULL;
    mt_u32* DescRNodeVirAddr = NULL;
    mt_u32* DescSizeVirAddr = NULL;

    DescInfo->DescPhyAddr   = DMX_READ_REG(IP_DESC_SADDR(PortId));
    DescInfo->DescDepth     = DMX_READ_REG(IP_DESC_SIZE(PortId));
    DescInfo->DescWPtr      = (DMX_READ_REG(IP_DESC_PTR(PortId)) >> 16 ) & 0xffff ;
    DescInfo->DescRPtr      = DMX_READ_REG(IP_DESC_SIZE(PortId)) & 0xffff;
    DescInfo->ValidDescNum  = DMX_READ_REG(IP_VLDDESC_CNT(PortId));
    DescInfo->AddDescNum    = DMX_READ_REG(IP_DESC_ADD(PortId));

    DescWNodeAddr            = DescInfo->DescPhyAddr + DescInfo->DescWPtr * 16 ; /*每个描述子节点size 是 16BYTE*/
    DescRNodeAddr            = DescInfo->DescPhyAddr + DescInfo->DescRPtr * 16 ; /*每个描述子节点size 是 16BYTE*/

    if ( (DescWNodeAddr != 0) && (DescRNodeAddr != 0) )
    {
        //DescWNodeVirAddr        =  ( mt_u32*)DEMUX_MAP_DDR_PHYADDRESS(DescWNodeAddr);
        //DescRNodeVirAddr        =  ( mt_u32*)(DEMUX_MAP_DDR_PHYADDRESS(DescRNodeAddr));
        //DescSizeVirAddr         =  ( mt_u32*)(DEMUX_MAP_DDR_PHYADDRESS(DescWNodeAddr) +  4);

        if ( DescWNodeVirAddr != NULL  )
        {
            DescInfo->DescWAddr  = *DescWNodeVirAddr;
        }

        if ( DescRNodeVirAddr != NULL )
        {
            DescInfo->DescRAddr  =  *DescRNodeVirAddr;
        }
        if ( DescSizeVirAddr != NULL )
        {
            DescInfo->DescSize = *DescSizeVirAddr & 0xffff;  
        }

        //DEMUX_UMMAP_DDR_PHYADDRESS(DescWNodeAddr);
        //DEMUX_UMMAP_DDR_PHYADDRESS(DescRNodeAddr);
    }

    return ;
}

mt_void DMXHalGetChannelDataFlow(mt_u32 ChannelId, ChannelDataFlow_info_t *ChannelDF)
{
    mt_u32  value;
    mt_u32  offset  = ChannelId >> 5;
    mt_u32  bit     = ChannelId & 0x1f;
    
    ChannelDF->PIDTsPacket      =  DMX_READ_REG(CHANNEL_TS_COUNT(ChannelId));
    ChannelDF->BufferTsPacket   =  DMX_READ_REG(ADDR_INT_CNT(ChannelId));
    value                       = DMX_READ_REG(OQ_ENA_0(offset));
    ChannelDF->OQEn             = (value >> bit) & 0x1;
    value                       = DMX_READ_REG(FQ_ENA_0(offset));
    ChannelDF->FQEn             = (value >> bit) & 0x1;
    
    return;
}

mt_void DmxHalGetChannelTSCount(mt_u32 ChanId, mt_u32 *ChanTsCount, mt_u32* OQTsCount)
{
    *ChanTsCount = DMX_READ_REG(CHANNEL_TS_COUNT(ChanId));
    *OQTsCount = DMX_READ_REG(ADDR_INT_CNT(ChanId));
}

/***********************************************************************************
* Function      : DmxHalSetChannelDataType
* Description   : Set Channel Data Type
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelDataType(mt_u32 ChanId, DMX_CHAN_DATA_TYPE_E DataType)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));

    dmx_pidctrl.bits.data_type = DataType;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelAFMode
* Description   :  Set Channel AF Mode
* Input         : Portid (= 0,1,2)
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelAFMode(mt_u32 ChanId, DMX_Ch_AFMode_E eAfMode)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.af_mode = eAfMode & 0x3;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      : DmxHalSetChannelCRCMode
* Description   : Set Channel CRC Mode
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelCRCMode(mt_u32 ChanId, MT_UNF_DMX_CHAN_CRC_MODE_E CrcMode)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));

    switch (CrcMode)
    {
        case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD :
            dmx_pidctrl.bits.crc_mode = 1;
            break;

        case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_SEND :
            dmx_pidctrl.bits.crc_mode = 2;
            break;

        case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD :
            dmx_pidctrl.bits.crc_mode = 3;
            break;

        case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_SEND :
            dmx_pidctrl.bits.crc_mode = 4;
            break;

        case MT_UNF_DMX_CHAN_CRC_MODE_FORBID :
        default :
            dmx_pidctrl.bits.crc_mode = 0;
    }

    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelCCDiscon
* Description   :  Set Channel CC discontiune Mode
* Input         : Portid (= 0,1,2)  DiscardFlag =0:not discard the discontinuous CC TS packet;
1 discard the discontinuous CC TS packet
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelCCDiscon(mt_u32 ChanId, mt_u32 DiscardFlag)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.cc_discon_ctrl = DiscardFlag & 0x1;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelPusiCtrl
* Description   :  Set Channel pusi control
* Input         : Portid (= 0,1,2)  PusiCtrl =0 : PUSI valid, check PUSI, start output stream when PUSI incoming
1: PUSI invalid, not check PUSI, output stream directly
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelPusiCtrl(mt_u32 ChanId, mt_u32 PusiCtrl)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.pusi_disable = PusiCtrl & 0x1;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelCCRepeatCtrl
* Description   :  Set Channel CC Repeat Ctrl
* Input         : Portid (= 0,1,2)  CCRepeatCtrl =0:discard this packet; 1: reserve this packet
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelCCRepeatCtrl(mt_u32 ChanId, mt_u32 CCRepeatCtrl)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.cc_equ_rve = CCRepeatCtrl & 0x1;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelTsPostMode
* Description   :  Set Channel Ts Post Mode
* Input         : Portid (= 0,1,2)  TSPost =0:not select TS_POST mode; 1: select TS_POST mode
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelTsPostMode(mt_u32 ChanId, mt_u32 TsPost)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.ts_post_mode = TsPost & 0x1;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelTsPostThresh
* Description   :  Set Channel Ts Post Thresh
* Input         : Portid (= 0,1,2) Threshold
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelTsPostThresh(mt_u32 ChanId, mt_u32 Threshold)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.ts_post_threshold = Threshold & 0x3f;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelAttr
* Description   :  Set Channel attr
* Input         : Portid (= 0,1,2) Threshold
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelAttr(mt_u32 ChanId, DMX_Ch_ATTR_E echattr)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    if ((echattr == DMX_CH_AUDIO) || (echattr == DMX_CH_VIDEO))
    {
        dmx_pidctrl.bits.pusi_disable = 1;
    }
    else
    {
        dmx_pidctrl.bits.pusi_disable = 0;
    }

    dmx_pidctrl.bits.ch_attri = echattr & 0x7;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);

    DMX_COM_EQUAL(dmx_pidctrl.all, DMX_READ_REG(DMX_PID_CTRL(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelFltMode
* Description   :  Set Channel Filter hit Mode
* Input         : u32Chid  bEnable
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelFltMode(mt_u32 u32Chid, MT_BOOL bEnable)
{
    U_DMX_PID_REC_BUF unPidRecTrl;

    unPidRecTrl.all = DMX_READ_REG(DMX_PID_REC_BUF(u32Chid));
    unPidRecTrl.bits.flt_hit_mode = bEnable;
    DMX_WRITE_REG(DMX_PID_REC_BUF(u32Chid), unPidRecTrl.all);

    DMX_COM_EQUAL(unPidRecTrl.all, DMX_READ_REG(DMX_PID_REC_BUF(u32Chid)));
}

/***********************************************************************************
* Function      :  DmxHalGetChannelPlayDmxid
* Description   :   Get Channel PlayDmxid
* Input         : chid (= 0,1,2)
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetChannelPlayDmxid(mt_u32 ChanId, mt_u32 *dmxid)
{
    U_DMX_PID_EN dmx_pid_en;

    dmx_pid_en.all = DMX_READ_REG(DMX_PID_EN(ChanId));
    *dmxid = (dmx_pid_en.bits.pid_play_dmx_id & 0x7) - 1;
}

/***********************************************************************************
* Function      :  DmxHalSetChannelPlayDmxid
* Description   :  Set Channel PlayDmxid
* Input         : Portid (= 0,1,2) Threshold
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelPlayDmxid(mt_u32 ChanId, mt_u32 dmxid)
{
    U_DMX_PID_EN dmx_pid_en;

    dmx_pid_en.all = DMX_READ_REG(DMX_PID_EN(ChanId));
    dmx_pid_en.bits.pid_play_dmx_id = (dmxid + 1) & 0x7;
    DMX_WRITE_REG(DMX_PID_EN(ChanId), dmx_pid_en.all);

    DMX_COM_EQUAL(dmx_pid_en.all, DMX_READ_REG(DMX_PID_EN(ChanId)));
}

/***********************************************************************************
* Function      :  DmxHalSetChannelRecDmxid
* Description   :  Set Channel record Dmxid
* Input         : Portid (= 0,1,2) Threshold
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelRecDmxid(mt_u32 ChanId, mt_u32 dmxid)
{
    U_DMX_PID_EN dmx_pid_en;

    dmx_pid_en.all = DMX_READ_REG(DMX_PID_EN(ChanId));
    dmx_pid_en.bits.pid_rec_dmx_id = (dmxid + 1) & 0x7;
    DMX_WRITE_REG(DMX_PID_EN(ChanId), dmx_pid_en.all);

    DMX_COM_EQUAL(dmx_pid_en.all, DMX_READ_REG(DMX_PID_EN(ChanId)));
}

/***********************************************************************************
* Function      : DmxHalSetChannelPid
* Description   : set Channel pid
* Input         : ChanId, pid
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelPid(mt_u32 ChanId, mt_u32 pid)
{
    U_DMX_PID_VALUE dmx_pid;

    dmx_pid.all = DMX_READ_REG(DMX_PID_VALUE(ChanId));
    dmx_pid.bits.pid_value = pid & 0x1fff;
    DMX_WRITE_REG(DMX_PID_VALUE(ChanId), dmx_pid.all);

    DMX_COM_EQUAL(dmx_pid.all, DMX_READ_REG(DMX_PID_VALUE(ChanId)));
}

/***********************************************************************************
* Function      : DmxHalSetChannelRecBufId
* Description   : Set Channel Rec BufId
* Input         : ChanId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelRecBufId(mt_u32 ChanId, mt_u32 OQId)
{
    U_DMX_PID_REC_BUF pid_rec_buf;

    pid_rec_buf.all = DMX_READ_REG(DMX_PID_REC_BUF(ChanId));

    pid_rec_buf.bits.rec_buf = OQId;
    DMX_WRITE_REG(DMX_PID_REC_BUF(ChanId), pid_rec_buf.all);

    DMX_COM_EQUAL(pid_rec_buf.all, DMX_READ_REG(DMX_PID_REC_BUF(ChanId)));
}

/***********************************************************************************
* Function      : DmxHalSetChannelPlayBufId
* Description   : Set Channel Play BufId
* Input         : ChanId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelPlayBufId(mt_u32 ChanId, mt_u32 OQId)
{
    U_DMX_PID_PLAY_BUF pid_play_buf;

    pid_play_buf.all = DMX_READ_REG(DMX_PID_PLAY_BUF(ChanId));

    pid_play_buf.bits.play_buf = OQId;
    DMX_WRITE_REG(DMX_PID_PLAY_BUF(ChanId), pid_play_buf.all);

    DMX_COM_EQUAL(pid_play_buf.all, DMX_READ_REG(DMX_PID_PLAY_BUF(ChanId)));
}

/***********************************************************************************
* Function      : DmxHalSetDemuxPortId
* Description   : Set Dmx PortId
* Input         : DmxId, PortMode, PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetDemuxPortId(mt_u32 DmxId, DMX_PORT_MODE_E PortMode, mt_u32 PortId)
{
    
}


/***********************************************************************************
* Function      :  DmxHalSetDataFakeMod
* Description   :  Set Data Fake Mode
* Input         : bFakeEn
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetDataFakeMod(MT_BOOL bFakeEn)
{

}

/***********************************************************************************
* Function      : DmxHalSetRecType
* Description   : Set Rec Type
* Input         : DmxId, RecType
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetRecType(mt_u32 DmxId, DMX_REC_TYPE_E RecType)
{
    U_DMX_CTRL_FUNC dmx_ctrl_func;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    DmxId += DMX_SW_HW_OFFSET;

    dmx_ctrl_func.all = DMX_READ_REG(DMX_CTRL_FUNC);
    switch (DmxId)
    {
        case 1:
            dmx_ctrl_func.bits.dmx1_rec_ctrl = RecType;
            break;

        case 2:
            dmx_ctrl_func.bits.dmx2_rec_ctrl = RecType;
            break;

        case 3:
            dmx_ctrl_func.bits.dmx3_rec_ctrl = RecType;
            break;

        case 4:
            dmx_ctrl_func.bits.dmx4_rec_ctrl = RecType;
            break;

        case 5:
            dmx_ctrl_func.bits.dmx5_rec_ctrl = RecType;
            break;
            
        case 6:
            dmx_ctrl_func.bits.dmx6_rec_ctrl = RecType;
            break;

        case 7:
            dmx_ctrl_func.bits.dmx7_rec_ctrl = RecType;
            break;

        default:
            dmx_ctrl_func.bits.dmx1_rec_ctrl = RecType;
    }

    DMX_WRITE_REG(DMX_CTRL_FUNC, dmx_ctrl_func.all);

    DMX_COM_EQUAL(dmx_ctrl_func.all, DMX_READ_REG(DMX_CTRL_FUNC));
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      : DmxHalFlushChannel
* Description   : Flush Channel
* Input         : ChanId, FlushType
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalFlushChannel(mt_u32 ChanId, DMX_FLUSH_TYPE_E FlushType)
{
    U_DMX_GLB_FLUSH glb_flush;

    glb_flush.all = DMX_READ_REG(DMX_GLB_FLUSH);

    glb_flush.bits.flush_ch     = ChanId;
    glb_flush.bits.flush_type   = FlushType;
    glb_flush.bits.flush_cmd    = 1;
    DMX_WRITE_REG(DMX_GLB_FLUSH, glb_flush.all);
}

/***********************************************************************************
* Function      :  DmxHalIsFlushChannelDone
* Description   :  Is flush channel done
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
MT_BOOL DmxHalIsFlushChannelDone(mt_void)
{
    U_DMX_GLB_FLUSH glb_flush;

    glb_flush.all = DMX_READ_REG(DMX_GLB_FLUSH);

    return glb_flush.bits.flush_done ? MT_TRUE : MT_FALSE;
}

/***********************************************************************************
* Function      : DmxHalSetTsRecBufId
* Description   : Set Ts Record Buf Id
* Input         : DmxId, OqId
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalSetTsRecBufId(mt_u32 DmxId, mt_u32 OqId)
{
    U_DMX_GLB_CTRL2 glb_ctrl2;

    glb_ctrl2.all = DMX_READ_REG(DMX_GLB_CTRL2(DmxId));

    glb_ctrl2.bits.dmx_tsrec_buf = OqId;
    DMX_WRITE_REG(DMX_GLB_CTRL2(DmxId), glb_ctrl2.all);

    DMX_COM_EQUAL(glb_ctrl2.all, DMX_READ_REG(DMX_GLB_CTRL2(DmxId)));
}

/***********************************************************************************
* Function      : DmxHalSetSpsRefRecCh
* Description   : Set Sps Reference Channel
* Input         : DmxId, ChanId
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalSetSpsRefRecCh(mt_u32 DmxId, mt_u32 ChanId)
{
    U_DMX_GLB_CTRL3 glb_ctrl3;

    glb_ctrl3.all = DMX_READ_REG(DMX_GLB_CTRL3(DmxId));
    glb_ctrl3.bits.dmx_spsrec_pusi_ch = ChanId;
    DMX_WRITE_REG(DMX_GLB_CTRL3(DmxId), glb_ctrl3.all);

    DMX_COM_EQUAL(glb_ctrl3.all, DMX_READ_REG(DMX_GLB_CTRL3(DmxId)));
}

/***********************************************************************************
* Function      :  DmxHalSetSpsPauseType
* Description   : Set Sps Reference Channel
* Input         : dmxid(= 1,2,3,4,5)type
* Output        :
* Return        :
* Others:       :type  0: after sps_ctrl valid, when find out the spx_num pes header in base channel, pause the play
1:after sps_ctrl valid, when find out ts tail in base channel, puase the timeshift
***********************************************************************************/
mt_void DmxHalSetSpsPauseType(mt_u32 DmxId, mt_u32 type)
{
    U_DMX_GLB_CTRL3 glb_ctrl3;

    glb_ctrl3.all = DMX_READ_REG(DMX_GLB_CTRL3(DmxId));
    glb_ctrl3.bits.dmx_sps_type = type & 0x1;
    DMX_WRITE_REG(DMX_GLB_CTRL3(DmxId), glb_ctrl3.all);

    DMX_COM_EQUAL(glb_ctrl3.all, DMX_READ_REG(DMX_GLB_CTRL3(DmxId)));
}

/***********************************************************************************
* Function      : DmxHalSetFilter
* Description   : Set filter attr
* Input         : FilterNum, Depth, Content, Mask
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalSetFilter(mt_u32 FilterId, mt_u32 Depth, mt_u8 Content, MT_BOOL bReverse, mt_u8 Mask)
{
    U_DMX_FILTER dmx_filter_crtl;

    dmx_filter_crtl.all = 0;

    dmx_filter_crtl.bits.wdata_mask     = Mask;
    dmx_filter_crtl.bits.wdata_content  = Content;
    dmx_filter_crtl.bits.wdata_mode     = bReverse;
    DMX_WRITE_REG(DMX_FILTERxy(FilterId, Depth), dmx_filter_crtl.all);
}

/***********************************************************************************
* Function      : DmxHalClearOq
* Description   : Clear Oq
* Input         : OQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClearOq(mt_u32 OqId, DMX_OQ_CLEAR_TYPE_E ClearType)
{
    U_CLEAR_CHANNEL clear_ch;

    clear_ch.all = DMX_READ_REG(CLR_CHN_CMD);

    clear_ch.bits.clear_chn   = OqId;
    clear_ch.bits.clear_type  = ClearType;
    clear_ch.bits.clear_start = 1;
    DMX_WRITE_REG(CLR_CHN_CMD, clear_ch.all);
}

/***********************************************************************************
* Function      : DmxHalIsClearOqDone
* Description   :
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
MT_BOOL DmxHalIsClearOqDone(mt_void)
{
    MT_BOOL         ret = MT_FALSE;
    U_IRAW_CLR_CHN  clear_ch;

    clear_ch.all = DMX_READ_REG(RAW_CLR_CHN);
    if (clear_ch.bits.iraw_clr_chn)
    {
        DMX_WRITE_REG(RAW_CLR_CHN, clear_ch.all);

        ret = MT_TRUE;
    }

    return ret;
}

/***********************************************************************************
* Function      :  DmxHalEnableAllPVRInt
* Description   :    Get eop int status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableAllPVRInt(mt_void)
{
    DMX_WRITE_REG(ENA_PVR_INT, 1);
}

/***********************************************************************************
* Function      :  DmxHalDisableAllPVRInt
* Description   :    Get eop int status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableAllPVRInt(mt_void)
{
    DMX_WRITE_REG(ENA_PVR_INT, 0);

    DMX_COM_EQUAL(0, DMX_READ_REG(ENA_PVR_INT));
}

#ifdef MT_DEMUX_PROC_SUPPORT
/***********************************************************************************
* Function      : DmxHalFQEnableAllOverflowInt
* Description   : enable fq overflow int
* Input         :
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalFQEnableAllOverflowInt(mt_void)
{
    U_ENA_INT_TYPE unEnaErr;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);

    unEnaErr.bits.iena_chn_i = 0x3;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
}

/***********************************************************************************
* Function      : DmxHalFQGetAllOverflowIntStatus
* Description   : Get fq overflow int status
* Input         :
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_u32 DmxHalFQGetAllOverflowIntStatus(mt_void)
{
    U_INT_STA_TYPE unIntSta;

    unIntSta.all = DMX_READ_REG(INT_STA_TYPE);

    return unIntSta.bits.iint_chn_i;
}

mt_u32 DmxHalFQGetOverflowIntStatus(mt_u32 offset)
{
    return DMX_READ_REG(INT_FQ_CHN_0(offset));
}

mt_u32 DmxHalFQGetOverflowIntType(mt_u32 offset)
{
    return DMX_READ_REG(TYPE_FQ_CHN_0(offset));
}

mt_void DmxHalFQClearOverflowInt(mt_u32 FqId)
{
    mt_u32  bit     = FqId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value = 1 << bit;
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

mt_void DmxHalFQSetOverflowInt(mt_u32 FqId, MT_BOOL Enable)
{
    mt_u32  offset  = FqId >> 5;
    mt_u32  bit     = FqId & 0x1F;
    mt_u32  value;

    value = DMX_READ_REG(ENA_FQ_CHN_0(offset));

    if (Enable)
    {
        value |= 1 << bit;
    }
    else
    {
        value &= ~(1 << bit);
    }
    DMX_WRITE_REG(ENA_FQ_CHN_0(offset), value);
}

MT_BOOL DmxHalFQIsEnableOverflowInt(mt_u32 FqId)
{
    mt_u32  offset  = FqId >> 5;
    mt_u32  bit     = FqId & 0x1F;
    mt_u32  value;

    value = DMX_READ_REG(ENA_FQ_CHN_0(offset));

    return (value & (1 << bit)) ? MT_TRUE : MT_FALSE;
}
#endif

/***********************************************************************************
* Function      : DmxHalOQGetAllOverflowIntStatus
* Description   : Get oq overflow int status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalOQGetAllOverflowIntStatus(mt_void)
{
    U_INT_STA_TYPE unIntSta;

    unIntSta.all = DMX_READ_REG(INT_STA_TYPE);

    return unIntSta.bits.iint_chn_o;
}

/***********************************************************************************
* Function      : DmxHalOQGetAllEopIntStatus
* Description   :
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalOQGetAllEopIntStatus(mt_void)
{
    U_INT_STA_TYPE unIntSta;

    unIntSta.all = DMX_READ_REG(INT_STA_TYPE);

    return unIntSta.bits.iint_eop_o;
}

/***********************************************************************************
* Function      :  DmxHalEnableAllChEopInt
* Description   :    Set channel overflow  int enable
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableAllChEopInt(mt_void)
{
    U_ENA_INT_TYPE unEnaErr;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaErr.bits.iena_eop_o = 0xf;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
}

/***********************************************************************************
* Function      :  DmxHalEnableAllChEnqueInt
* Description   :    Set channel overflow  int enable
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableAllChEnqueInt(mt_void)
{
    U_ENA_INT_TYPE unEnaErr;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaErr.bits.iena_desc_o = 0xf;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
}

/***********************************************************************************
* Function      :  DmxHalEnableFQOutqueInt
* Description   :    Set channel overflow  int enable
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableFQOutqueInt(mt_void)
{
    U_ENA_INT_TYPE unEnaErr;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaErr.bits.iena_desc_i = 0x3;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
}

/***********************************************************************************
* Function      :  DmxHalDisableFQOutqueInt
* Description   :    Set IP  int disable
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableFQOutqueInt(mt_void)
{
    U_ENA_INT_TYPE unEnaErr;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaErr.bits.iena_desc_i = 0;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
}

/***********************************************************************************
* Function      :  DmxHalSetFlushMaxWaitTime
* Description   :    Set play dmx2buf bp mode
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetFlushMaxWaitTime(mt_u32 u32MaxTime)
{
    U_CLR_WAIT_TIME unWaitTime;

    unWaitTime.all = DMX_READ_REG(CLR_WAIT_TIME);
    unWaitTime.bits.clr_wait_time = u32MaxTime & 0xffff;
    DMX_WRITE_REG(CLR_WAIT_TIME, unWaitTime.all);
}

/***********************************************************************************
* Function      :  DmxHalSetScdFilter
* Description   :    set SCD filter content
* Input         : u32FltId u8Content
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_s32 DmxHalSetScdFilter(mt_u32 u32FltId, mt_u8 u8Content)
{
    U_SCD_FLT filter;
    mt_u32 offset = 0;

    if (u32FltId >= 10)
    {
        return MT_FAILURE;
    }

    offset = u32FltId >> 2;

    offset <<= 2;

    u32FltId -= offset;

    filter.all = DMX_READ_REG(SCD_FLT0_3 + offset);

    switch (u32FltId)
    {
    case 0:
        filter.bits.flt0 = u8Content;
        break;
    case 1:
        filter.bits.flt1 = u8Content;
        break;
    case 2:
        filter.bits.flt2 = u8Content;
        break;
    case 3:
        filter.bits.flt3 = u8Content;
        break;
    default:
        return MT_FAILURE;
    }

    DMX_WRITE_REG(SCD_FLT0_3 + offset, filter.all);

    DMX_COM_EQUAL(filter.all, DMX_READ_REG(SCD_FLT0_3 + offset));

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      :  DmxHalSetScdRangeFilter
* Description   :    set SCD filter range
* Input         : u32FltId u8High u8Low
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_s32 DmxHalSetScdRangeFilter(mt_u32 u32FltId, mt_u8 u8High, mt_u8 u8Low)
{
    U_SCD_FLT filter;
    mt_u32 offset = 0;

    BUG_ON(u32FltId >= DMX_TOTAL_RANGE_FLTNUM);

    /*calculate the offset of each scd filter*/
    switch (u32FltId)
    {
    case 1:
    case 2:
        offset = 4;
        break;
    case 3:
    case 4:
        offset = 8;
        break;
    case 5:
    case 6:
        offset = 12;
        break;
    case 0:
    default:
        offset = 0;
        break;
    }

    filter.all = DMX_READ_REG(SCD_FLT8_11 + offset);

    switch ((u32FltId + 1) % 2)
    {
    case 0:
        filter.bits.flt0 = u8Low;
        filter.bits.flt1 = u8High;
        break;
    case 1:
        filter.bits.flt2 = u8Low;
        filter.bits.flt3 = u8High;
        break;
    }

    DMX_WRITE_REG(SCD_FLT8_11 + offset, filter.all);

    DMX_COM_EQUAL(filter.all, DMX_READ_REG(SCD_FLT8_11 + offset));

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DmxHalSetScdNewRangeFilter
* Description   : set SCD new filter range
* Input         : FilterId, High, Low, Mask, Negate
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetScdNewRangeFilter(mt_u32 FilterId, mt_u8 High, mt_u8 Low, mt_u8 Mask, MT_BOOL Negate)
{
    U_SCD_FLT_V200  filter;
    mt_u32          new_flt_neg;

    filter.all = DMX_READ_REG(SCD_NEW_FLTSET(FilterId));
    filter.bits.byte_h  = High;
    filter.bits.byte_l  = Low;
    filter.bits.mask    = Mask;

    DMX_WRITE_REG(SCD_NEW_FLTSET(FilterId), filter.all);

    DMX_COM_EQUAL(filter.all, DMX_READ_REG(SCD_NEW_FLTSET(FilterId)));

    new_flt_neg = DMX_READ_REG(SCD_NEW_FLT_NEG);
    if (Negate)
    {
        new_flt_neg |= (1 << FilterId);
    }
    else
    {
        new_flt_neg &= ~(1 << FilterId);
    }

    DMX_WRITE_REG(SCD_NEW_FLT_NEG, new_flt_neg);

    DMX_COM_EQUAL(new_flt_neg, DMX_READ_REG(SCD_NEW_FLT_NEG));
}

/***********************************************************************************
* Function      : DmxHalChooseScdFilter
* Description   : Choose Scd Filter
* Input         : ScdId, FilterId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalChooseScdFilter(mt_u32 ScdId, mt_u32 FilterId)
{

}

/***********************************************************************************
* Function      : DmxHalScdFilterClear
* Description   : Clear Scd Filter
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalScdFilterClear(mt_u32 ScdId)
{

}

/***********************************************************************************
* Function      : DmxHalChooseScdRangeFilter
* Description   : Choose Scd Range Filter
* Input         : ScdId, FilterId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalChooseScdRangeFilter(mt_u32 ScdId, mt_u32 FilterId)
{

}

/***********************************************************************************
* Function      : DmxHalScdRangeFilterClear
* Description   : Clear Scd Range Filter
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalScdRangeFilterClear(mt_u32 ScdId)
{

}

/***********************************************************************************
* Function      : DmxHalChooseScdNewRangeFilter
* Description   : Choose Scd New Range Filter
* Input         : ScdId, FilterId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalChooseScdNewRangeFilter(mt_u32 ScdId, mt_u32 FilterId)
{

}

/***********************************************************************************
* Function      : DmxHalScdNewRangeFilterClear
* Description   : Clear Scd New Range Filter
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalScdNewRangeFilterClear(mt_u32 ScdId)
{
    mt_u32 flt_en;

    flt_en = DMX_READ_REG(SCD_NEW_FLTEN(ScdId));

    flt_en &= 0xFFFF0000;
    DMX_WRITE_REG(SCD_NEW_FLTEN(ScdId), flt_en);

    DMX_COM_EQUAL(flt_en, DMX_READ_REG(SCD_NEW_FLTEN(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalEnablePesSCD
* Description   : enbale pes
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalEnablePesSCD(mt_u32 ScdId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.pes_en = 1;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalDisablePesSCD
* Description   : disbale pes
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalDisablePesSCD(mt_u32 ScdId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.pes_en = 0;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalEnableEsSCD
* Description   : enbale es
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalEnableEsSCD(mt_u32 ScdId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.esscd_en = 1;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalDisableEsSCD
* Description   : disbale es
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalDisableEsSCD(mt_u32 ScdId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.esscd_en = 0;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalEnableMp4SCD
* Description   : enbale m4_short_en
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalEnableMp4SCD(mt_u32 ScdId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.m4_short_en = 1;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalDisableMp4SCD
* Description   : disbale m4_short_en
* Input         : ScdId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalDisableMp4SCD(mt_u32 ScdId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.m4_short_en = 0;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalSetSCDAttachChannel
* Description   : set SCD channel
* Input         : ScdId ChanId
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalSetSCDAttachChannel(mt_u32 ScdId, mt_u32 ChanId)
{
    U_SCD_SET scd_set;

    scd_set.all = DMX_READ_REG(SCD_SETa(ScdId));

    scd_set.bits.scd_ch = ChanId;

    DMX_WRITE_REG(SCD_SETa(ScdId), scd_set.all);

    DMX_COM_EQUAL(scd_set.all, DMX_READ_REG(SCD_SETa(ScdId)));
}

/***********************************************************************************
* Function      : DmxHalAllocSCDBufferId
* Description   : set SCD buffer id
* Input         : ScdId, OqId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalAllocSCDBufferId(mt_u32 ScdId, mt_u32 OqId)
{
    U_SCD_BUF scd_buf;

    scd_buf.all = DMX_READ_REG(SCD_BUF0(ScdId));

    scd_buf.bits.buf_num = OqId & 0x7f;

    DMX_WRITE_REG(SCD_BUF0(ScdId), scd_buf.all);

    DMX_COM_EQUAL(scd_buf.all, DMX_READ_REG(SCD_BUF0(ScdId)));
}

/***********************************************************************************
* Function      :  DmxHalSetFlushIPPort
* Description   :  Set Flush IP Port
* Input         : u32PortId
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalSetFlushIPPort(mt_u32 PortId)
{
    U_IP_CLRCHN_REQ ip_flush;

    ip_flush.all = DMX_READ_REG(IP_CLRCHN_REQ(PortId));

    ip_flush.bits.ip_clrchn_req = 1;
    DMX_WRITE_REG(IP_CLRCHN_REQ(PortId), ip_flush.all);
}

/***********************************************************************************
* Function      :  DmxHalGetChannelTSScrambleFlag
* Description   :   Get Channel CRC Mode
* Input         : chid (= 0,1,2)
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetChannelTSScrambleFlag(mt_u32 ChanId, MT_BOOL  *pEnable)
{
    U_CH_HIS ch_sramble;

    ch_sramble.all = DMX_READ_REG(CH_HIS(ChanId));
    *pEnable = (ch_sramble.bits.ts_scr_flag & 0x1);
}

/***********************************************************************************
* Function      :  DmxHalGetChannelPesScrambleFlag
* Description   :   Get Channel CRC Mode
* Input         : chid (= 0,1,2)
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetChannelPesScrambleFlag(mt_u32 ChanId, MT_BOOL  *pEnable)
{
    U_CH_HIS ch_sramble;

    ch_sramble.all = DMX_READ_REG(CH_HIS(ChanId));
    *pEnable = (ch_sramble.bits.pes_scr_flag & 0x1);
}

/***********************************************************************************
* Function      : DmxHalClrAutoIPBP
* Description   : IP Auto Clear BP
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalIPPortAutoClearBP(mt_void)
{
    U_IP_BP_CLR_CFG reg;

    reg.value = DMX_READ_REG(IP_BP_CLR_CFG);

    reg.bits.ipaful_clr_ena = 0;
    DMX_WRITE_REG(IP_BP_CLR_CFG, reg.value);

    DMX_COM_EQUAL(reg.value, DMX_READ_REG(IP_BP_CLR_CFG));
}

/***********************************************************************************
* Function      : DmxHalGetIPBPStatus
* Description   : Get IP BP Status
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
MT_BOOL DmxHalGetIPBPStatus(mt_u32 PortId)
{
    mt_size_t u32LockFlag;
    mt_u32 status32;
    mt_u32 status40;
    mt_u32 regaddr;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);


    IP_BP_FQ_STA(PortId, 0,regaddr);
    status32 = DMX_READ_REG(regaddr);
    IP_BP_FQ_STA(PortId, 1,regaddr);
    status40 = DMX_READ_REG(regaddr) & 0xff;

    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);

    return (status32 || status40);
}

/***********************************************************************************
* Function      : DmxHalClrIPBPStatus
* Description   : clear IP BP Status
* Input         : PortId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClrIPBPStatus(mt_u32 PortId)
{
    mt_u32 regaddr;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);
    IP_BP_FQ_STA(PortId, 0,regaddr);
    DMX_WRITE_REG(regaddr, 0);
    IP_BP_FQ_STA(PortId, 1,regaddr);
    DMX_WRITE_REG(regaddr, 0);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      : DmxHalClrIPFqBPStatus
* Description   : clear IP BP Status for one fq
* Input         : PortId, FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClrIPFqBPStatus(mt_u32 PortId, mt_u32 FQId)
{
    mt_u32  offset  = FQId >> 5;
    mt_u32  bit     = FQId & 0x1F;
    mt_u32  value;
    mt_u32 regaddr;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    IP_BP_FQ_STA(PortId, offset,regaddr);
    value = DMX_READ_REG(regaddr);

    value &= ~(1 << bit);
    DMX_WRITE_REG(regaddr, value);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      : DmxHalSetPcrDmxId
* Description   : Set PCR DmxId
* Input         : PcrId, DmxId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetPcrDmxId(const mt_u32 PcrId, const mt_u32 DmxId)
{
    reg_set_dmx_tsp_pcrsetn_pcr_ch(PcrId, DmxId);

}

/***********************************************************************************
* Function      : DmxHalSetPcrPid
* Description   : Set PCR Pid
* Input         : PcrId, PcrPid
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetPcrPid(const mt_u32 PcrId, const mt_u32 PcrPid)
{
    reg_set_dmx_tsp_pcrsetn_pcr_id(PcrId, PcrPid);
}

/***********************************************************************************
* Function      : DmxHalGetPcrValue
* Description   : Get PCR value
* Input         : PcrId, PcrVal
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetPcrValue(const mt_u32 PcrId, mt_u64 *PcrVal)
{
    mt_u64 value0 = 0, value1 = 0;

    //PCR value(extend bit and high bit)register 0,0x3F00+0x4*j
    //pcr_extra_8_0 16:8    RO  0x000   PCR extend bit
    //reserved       7:1    RO  0x00    reserved
    //pcr_base_32      0    RO  0x0     PCR_base the highest bit
    //PCR value(low 32 bits) register 1 0x3F04+0x4*j
    //pcr_base_31_0 31:0    RO  0x00000000  PCR_base low 32 bits

    value0 = DMX_READ_REG(DMX_CH_PCR_VALUE0(PcrId));
    value1 = DMX_READ_REG(DMX_CH_PCR_VALUE1(PcrId));

    //*PcrVal = ((((value0&0xff00)>>7) +(value0&0x1))<<32) + value1;
    *PcrVal = ((value0 & 0x1) << 32) + value1;
}

/***********************************************************************************
* Function      : DmxHalGetScrValue
* Description   : Get SCR value
* Input         : PcrId, ScrVal
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetScrValue(const mt_u32 PcrId, mt_u64 *ScrVal)
{
    mt_u64 value0 = 0, value1 = 0;

    //when PCR incoming SCRvalue(extend bit and high bit) register 0    0x3F08+0x4*j
    //scr_extra_8_0 16:8    RO  0x000   SCR extend bit
    //reserved  7:1 RO  0x00    reserved
    //scr_base_32   0   RO  0x0 SCR_base the highest bit

    //when PCR incoming SCR value(low 32 bits) register 1   0x3F0C+0x4*j
    //scr_base_31_0 31:0    RO  0x00000000  SCR_base low 32 bits

    value0 = DMX_READ_REG(DMX_CH_SCR_VALUE0(PcrId));
    value1 = DMX_READ_REG(DMX_CH_SCR_VALUE1(PcrId));

    //*ScrVal = ((((value0&0xff00)>>7) +(value0&0x1))<<32) + value1;
    *ScrVal = ((value0 & 0x1) << 32) + value1;
}

/***********************************************************************************
* Function      :  DmxHalGetTotalTeiIntStatus
* Description   :    Get Total Sync Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetTotalTeiIntStatus(mt_void)
{
    U_PVR_INT_SCAN unPvrIntSta;

    unPvrIntSta.all = DMX_READ_REG(PVR_INT_SCAN);
    return unPvrIntSta.bits.total_int_err;
}

/***********************************************************************************
* Function      :  DmxHalGetTotalPcrIntStatus
* Description   :    Get Total pcr int  Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetTotalPcrIntStatus(mt_void)
{
    U_PVR_INT_SCAN unPvrIntSta;

    unPvrIntSta.all = DMX_READ_REG(PVR_INT_SCAN);
    return unPvrIntSta.bits.total_int_pcr;
}

/***********************************************************************************
* Function      :  DmxHalGetTotalDiscIntStatus
* Description   :    Get Total Disc int Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetTotalDiscIntStatus(mt_void)
{
    U_PVR_INT_SCAN unPvrIntSta;

    unPvrIntSta.all = DMX_READ_REG(PVR_INT_SCAN);
    return unPvrIntSta.bits.total_int_disc;
}

/***********************************************************************************
* Function      :  DmxHalGetTotalCrcIntStatus
* Description   :    Get Total crc int Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetTotalCrcIntStatus(mt_void)
{
    U_PVR_INT_SCAN unPvrIntSta;

    unPvrIntSta.all = DMX_READ_REG(PVR_INT_SCAN);
    return unPvrIntSta.bits.total_int_fltcrc;
}

/***********************************************************************************
* Function      :  DmxHalGetTotalPenLenIntStatus
* Description   :    Get Total pes len int Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetTotalPenLenIntStatus(mt_void)
{
    U_PVR_INT_SCAN unPvrIntSta;

    unPvrIntSta.all = DMX_READ_REG(PVR_INT_SCAN);
    return unPvrIntSta.bits.total_int_peslen;
}

/***********************************************************************************
* Function      : DmxHalGetPcrIntStatus
* Description   : Get Pcr Int Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetPcrIntStatus(mt_void)
{
    U_STA_PCR_ARRI PcrIntSta;

    PcrIntSta.all = DMX_READ_REG(STA_PCR_ARRI);

    return PcrIntSta.bits.sta_pcr;
}

/***********************************************************************************
* Function      : DmxHalClrPcrIntStatus
* Description   : Clean Pcr Int
* Input         : PcrId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClrPcrIntStatus(const mt_u32 PcrId)
{
    DMX_WRITE_REG(RAW_PCR_ARRI, (1 << PcrId));
}

/***********************************************************************************
* Function      : DmxHalSetPcrIntEnable
* Description   : Set Pcr Int Enable
* Input         : PcrId, Enable
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetPcrIntEnable(const mt_u32 PcrId, const MT_BOOL Enable)
{
   
    if (Enable)
    {
        reg_set_dmx_tsp_pcrsetn_pcr_ena(PcrId, 1);
    }
    else
    {
        reg_set_dmx_tsp_pcrsetn_pcr_ena(PcrId, 0);
    }

}

/***********************************************************************************
* Function      :  DmxHalGetTeiIntInfo
* Description   :    Get Tei Int Info
* Input         :
* Output        :   pu32DmxId , pu32ChanId
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetTeiIntInfo(mt_u32* pu32DmxId, mt_u32* pu32ChanId)
{
    if (pu32DmxId && pu32ChanId)
    {
        U_STA_TEI unTeiIntSta;

        unTeiIntSta.all = DMX_READ_REG(STA_TEI);

        *pu32DmxId  = unTeiIntSta.bits.tei_dmx;
        *pu32ChanId = unTeiIntSta.bits.tei_ch;
    }
}

/***********************************************************************************
* Function      :  DmxHalClrTeiIntStatus
* Description   :    Clr Tei Int Status
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClrTeiIntStatus(mt_void)
{
    DMX_WRITE_REG(RAW_TEI, 1);
}

/***********************************************************************************
* Function      : DmxHalGetDiscIntStatus
* Description   : Get disc int status
* Input         : RegionNum
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetDiscIntStatus(mt_u32 RegionNum)
{
    return DMX_READ_REG(STA_DISC0(RegionNum));
}

/***********************************************************************************
* Function      : DmxHalClearDiscIntStatus
* Description   : Clr Disc int status
* Input         : ChanId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClearDiscIntStatus(mt_u32 ChanId)
{

}

/***********************************************************************************
* Function      : DmxHalGetCrcIntStatus
* Description   : Get Crc int status
* Input         : RegionNum
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetCrcIntStatus(mt_u32 RegionNum)
{
    return DMX_READ_REG(STA_FLTCRC0(RegionNum));
}

/***********************************************************************************
* Function      : DmxHalClearCrcIntStatus
* Description   : Clr Crc int status
* Input         : ChanId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClearCrcIntStatus(mt_u32 ChanId)
{

}

/***********************************************************************************
* Function      : DmxHalGetPesLenIntStatus
* Description   : Get Pes Len int status
* Input         : RegionNum
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetPesLenIntStatus(mt_u32 RegionNum)
{
    return DMX_READ_REG(STA_PES_LEN0(RegionNum));
}

/***********************************************************************************
* Function      : DmxHalClearPesLenIntStatus
* Description   : Clr Pes Len int status
* Input         : ChanId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClearPesLenIntStatus(mt_u32 ChanId)
{

}

/***********************************************************************************
* Function      :  DmxHalFlushScdBuf
* Description   :    Flush Scd Buf
* Input         : u32ScdId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalFlushScdBuf(mt_u32 u32ScdId)
{

}

/***********************************************************************************
* Function      : DmxHalClrScdCnt
* Description   : clear Scd cnt
* Input         : ScdId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalClrScdCnt(mt_u32 ScdId)
{

}

mt_void DmxHalGetRecTsCnt(mt_u32 ScdId, mt_u64 *TsCnt)
{

}

mt_void DmxHalGetCurrentSCR(mt_u32 *ScrClk)
{

}



mt_void DmxHalConfigHardware(mt_void)
{
    reg_dmx_init();
}
/***********************************************************************************
* Function      : DmxHalCfgTSOClk
* Description   : Config TSO clock pahse
* Input         :
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalCfgTSOClk(mt_u32 PortId,MT_BOOL ClkReverse,mt_u32 enClk,mt_u32 ClkDiv)
{
}

/***********************************************************************************
* Function      : DmxHalGetTSOClkCfg
* Description   : Config TSO clock pahse
* Input         :
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalGetTSOClkCfg(mt_u32 PortId,MT_BOOL *ClkReverse,mt_u32 *enClk,mt_u32 *ClkDiv)
{

}

/***********************************************************************************
* Function      : DmxHalAttachFilter
* Description   : Attach Filter to channel
* Input         : FilterId, ChanId
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalAttachFilter(mt_u32 FilterId, mt_u32 ChanId)
{
    mt_u32  filter_en = 1 << (FilterId & 0x1F);
#ifndef DMX_REGION_SUPPORT
    mt_u32  value;
    mt_u32  offset;
    mt_u32  i;

    value = DMX_READ_REG(DMX_FILTER_EN(ChanId));
    for (i = 0; i < 32; i++)
    {
        if (0 == (value & (1 << i)))
        {
            break;
        }
    }

    value = DMX_READ_REG(DMX_FILTER_ID(ChanId, i));

    offset = (i & 0x3) * 8;

    value &= ~(0xFF << offset);
    value |= FilterId << offset;

    DMX_WRITE_REG(DMX_FILTER_ID(ChanId, i), value);

    filter_en = (1 << i);
#endif
    filter_en |= DMX_READ_REG(DMX_FILTER_EN(ChanId));

    DMX_WRITE_REG(DMX_FILTER_EN(ChanId), filter_en);

    DMX_COM_EQUAL(filter_en, DMX_READ_REG(DMX_FILTER_EN(ChanId)));
}

/***********************************************************************************
* Function      : DmxHalDetachFilter
* Description   : Detach Filter from channel
* Input         : FilterId, ChanId
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalDetachFilter(mt_u32 FilterId, mt_u32 ChanId)
{
    int i=0;
    
    //for(i=0;i<4;i++)
    {
	    reg_set_funit_filter_data(FilterId + i, 0);
	    reg_set_funit_filter_mask(FilterId + i, 0);
	    reg_set_funit_filter_mode(FilterId + i, 0);
    }
   
}

/***********************************************************************************
* Function      :  DmxHalEnableOQOutDInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableOQOutDInt(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1F;
    mt_u32  value;

    value  = DMX_READ_REG(ENA_OQ_DESC_0(offset));

    value |= 1 << bit;
    DMX_WRITE_REG(ENA_OQ_DESC_0(offset), value);
}

/***********************************************************************************
* Function      :  DmxHalDisableOQOutDInt
* Description   :    Set fix int enale
* Input         : u32OQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableOQOutDInt(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1F;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value  = DMX_READ_REG(ENA_OQ_DESC_0(offset));

    value &= ~(1 << bit);
    DMX_WRITE_REG(ENA_OQ_DESC_0(offset), value);
	
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      :  DmxHalGetOQEopIntStatus
* Description   :    Get fix int status
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
MT_BOOL DmxHalGetOQEopIntStatus(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;

    value = DMX_READ_REG(INT_OQ_EOP_0(offset));

    return (value & (1 << bit)) ? MT_TRUE : MT_FALSE;
}

/*
 * Clear EOP raw interrupt state which always set when OQ received data. 
 */
static inline mt_void __DmxHalClearOQEopIntStatus(mt_u32 OQId)
{
    //mt_u32  offset  = OQId >> 5;
    //mt_u32  bit     = OQId & 0x1f;
    //mt_u32  value = 1 << bit;

    //DMX_WRITE_REG(RAW_OQ_EOP_0(offset), value);
}

mt_void DmxHalClearOQEopIntStatus(mt_u32 OQId)
{
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);
    __DmxHalClearOQEopIntStatus(OQId);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/*
 * eop interrupt active usage count.
 */
//static atomic_t OQEopActiveUsageCount[DMX_OQ_CNT] =  { [0 ... (DMX_OQ_CNT -1)] = ATOMIC_INIT(0) };
static atomic_t OQEopActiveUsageCount[128] ;//=  { [0 ... (DMX_OQ_CNT -1)] = ATOMIC_INIT(0) };
/*
 * enable EOP interrupt. 
 */
static inline mt_void __DmxHalEnableOQEopInt(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;

    value  = DMX_READ_REG(ENA_OQ_EOP_0(offset));
    value |= (1 << bit);   
    DMX_WRITE_REG(ENA_OQ_EOP_0(offset), value);
}

mt_void DmxHalEnableOQEopInt(mt_u32 OQId)
{
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    if (0 == atomic_read(&OQEopActiveUsageCount[OQId]))
    {
        /* clear raw eop interrupt */
        __DmxHalClearOQEopIntStatus(OQId);

        /* enable OQ Eop interrupt */
        __DmxHalEnableOQEopInt(OQId);
    }

    atomic_inc(&OQEopActiveUsageCount[OQId]);   

    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);

    //MT_DBG_DEMUX("OQ channel(id:%d) EOP interrupt active usage count is [%d].\n", OQId, atomic_read(&OQEopActiveUsageCount[OQId]));
}

/*
 * disable EOP interrupt. 
 */
static inline mt_void __DmxHalDisableOQEopInt(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;
    
    value  = DMX_READ_REG(ENA_OQ_EOP_0(offset));
    value &= ~(1 << bit);
    DMX_WRITE_REG(ENA_OQ_EOP_0(offset), value);
}

mt_void DmxHalDisableOQEopInt(mt_u32 OQId)
{ 
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);
    
    if (unlikely(0 == atomic_read(&OQEopActiveUsageCount[OQId]))) 
    {
        goto out; /* Eop interrupt has not enabled */
    }
    else
    {
        atomic_dec(&OQEopActiveUsageCount[OQId]);

        if (0 == atomic_read(&OQEopActiveUsageCount[OQId]))
        {
            /* disable OQ Eop int */
            __DmxHalDisableOQEopInt(OQId);
        }
    }
    
out:
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
    
    //MT_DBG_DEMUX("OQ channel(id:%d) EOP interrupt active usage count is [%d].\n", OQId, atomic_read(&OQEopActiveUsageCount[OQId]));
}

MT_BOOL DmxHalOQGetOverflowIntStatus(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value = DMX_READ_REG(INT_OQ_CHN_0(offset));
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);

    return (value & (1 << bit)) ? MT_TRUE : MT_FALSE;
}

mt_void DmxHalOQClearOverflowInt(mt_u32 OQId)
{

}

/***********************************************************************************
* Function      :  DmxHalOQEnableOverflowInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalOQEnableOverflowInt(mt_u32 OQId)
{

}

/***********************************************************************************
* Function      :  DmxHalOQDisableOverflowInt
* Description   :    Set fix int enale
* Input         : u32OQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalOQDisableOverflowInt(mt_u32 OQId)
{

}

mt_u32 DmxHalOQGetAllOutputIntStatus(mt_void)
{
    U_INT_STA_TYPE unIntSta;

    unIntSta.all = DMX_READ_REG(INT_STA_TYPE);

    return unIntSta.bits.iint_desc_o;
}

mt_u32 DmxHalOQGetOutputIntStatus(mt_u32 OqRegionId)
{
    return DMX_READ_REG(INT_OQ_DESC_0(OqRegionId));
}

mt_void DmxHalOQEnableOutputInt(mt_u32 OQId, MT_BOOL Enable)
{

}

/***********************************************************************************
* Function      :  DmxHalEnableFQOvflErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableFQOvflErrInt(mt_void)
{
    U_ENA_INT_TYPE unEnaErr;

    unEnaErr.all = DMX_READ_REG(ENA_INT_TYPE);
    unEnaErr.bits.iena_fq_ovfl_err = 1;
    DMX_WRITE_REG(ENA_INT_TYPE, unEnaErr.all);
}

/***********************************************************************************
* Function      :  DmxHalDisableFQOvflErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableFQOvflErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalEnableFQOvflErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableOQOvflErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalDisableOQOvflErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableOQOvflErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalEnableFQCfgErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableFQCfgErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalDisableFQCfgErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableFQCfgErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalEnableFQDescErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableFQDescErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalDisableFQDescErrInt
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableFQDescErrInt(mt_void)
{

}

/***********************************************************************************
* Function      :  DmxHalEnableAllDavInt
* Description   :    Enable All  Int
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableAllDavInt(mt_void)
{
    U_ENA_INT_ALL ena_all_int;

    ena_all_int.all = DMX_READ_REG(ENA_INT_ALL);
    ena_all_int.bits.iena_all = 1;
    DMX_WRITE_REG(ENA_INT_ALL, ena_all_int.all);
}

/***********************************************************************************
* Function      :  DmxHalEnableOQRecive
* Description   :    Set fix int enale
* Input         : u32OQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalEnableOQRecive(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value  = DMX_READ_REG(OQ_ENA_0(offset));
    value |= 1 << bit;
    DMX_WRITE_REG(OQ_ENA_0(offset), value);

    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);

}

/***********************************************************************************
* Function      :  DmxHalDisableOQRecive
* Description   :    Set fix int enale
* Input         : u32OQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDisableOQRecive(mt_u32 OQId)
{
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value  = DMX_READ_REG(OQ_ENA_0(offset));
    value &= ~(1 << bit);
    DMX_WRITE_REG(OQ_ENA_0(offset), value);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);

}

MT_BOOL DmxHalGetOQEnableStatus(mt_u32 OQId)
{
    MT_BOOL ret;
    mt_u32  offset  = OQId >> 5;
    mt_u32  bit     = OQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value  = DMX_READ_REG(OQ_ENA_0(offset));
    value &= 1 << bit;
    if (value)
    {
        ret =  MT_TRUE;
    }
    else
    {
        ret =  MT_FALSE;
    }
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
    return ret;
}

/***********************************************************************************
* Function      :  DmxHalFQEnableRecive
* Description   :    Set fix int enale
* Input         : u32FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalFQEnableRecive(mt_u32 FQId, MT_BOOL Enable)
{
    mt_u32  offset  = FQId >> 5;
    mt_u32  bit     = FQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);


    value = DMX_READ_REG(FQ_ENA_0(offset));

    if (Enable)
    {
        value |= 1 << bit;
    }
    else
    {
        value &= ~(1 << bit);
    }
    DMX_WRITE_REG(FQ_ENA_0(offset), value);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

mt_s32 DmxHalGetInitStatus(mt_void)
{
    mt_s32 ret      = MT_FAILURE;
    mt_u32 FqStatus = DMX_READ_REG(FQ_INIT_DONE) & DMX_MASK_BIT_0;
    mt_u32 OqStatus = DMX_READ_REG(OQ_INIT_DONE) & DMX_MASK_BIT_0;

    if (!FqStatus && !OqStatus)
    {
        ret = MT_SUCCESS;
    }

    return ret;
}

/***********************************************************************************
* Function      :  DmxHalSetFQWORDx
* Description   :    Set fix int enale
* Input         : u32FQId,u32Data
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetFQWORDx(mt_u32 FQId, mt_u32 Offset, mt_u32 Value)
{
    wmb();/*sync the DDR*/
    switch (Offset)
    {
        case DMX_FQ_CTRL_OFFSET:
        {
            DMX_WRITE_REG(ADDR_FQ_WORD0(FQId), Value);
            break;
        }

        case DMX_FQ_RDVD_OFFSET:
        {
            DMX_WRITE_REG(ADDR_FQ_WORD1(FQId), Value);
            break;
        }

        case DMX_FQ_SZWR_OFFSET:
        {
            DMX_WRITE_REG(ADDR_FQ_WORD2(FQId), Value);
            break;
        }

        case DMX_FQ_START_OFFSET:
        {
            DMX_WRITE_REG(ADDR_FQ_WORD3(FQId), Value);
            break;
        }

        default:
        {
            break;
        }
    }
}

/***********************************************************************************
* Function      :  DmxHalGetFQWORDx
* Description   :    Set fix int enale
* Input         : u32FQId,u32Data
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetFQWORDx(mt_u32 FQId, mt_u32 Offset, mt_u32 *pu32Data)
{
    switch (Offset)
    {
        case DMX_FQ_CTRL_OFFSET:
            *pu32Data = DMX_READ_REG(ADDR_FQ_WORD0(FQId));
            break;

        case DMX_FQ_RDVD_OFFSET:
            *pu32Data = DMX_READ_REG(ADDR_FQ_WORD1(FQId));
            break;

        case DMX_FQ_SZWR_OFFSET:
            *pu32Data = DMX_READ_REG(ADDR_FQ_WORD2(FQId));
            break;

        case DMX_FQ_START_OFFSET:
            *pu32Data = DMX_READ_REG(ADDR_FQ_WORD3(FQId));
            break;

        default:
        {
            break;
        }
    }
}

/***********************************************************************************
* Function      :  DmxHalSetFQWritePtr
* Description   :    Set fix int enale
* Input         : u32FQId,u32Data
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetFQWritePtr(mt_u32 FQId, mt_u32 WritePtr)
{
    wmb();/*sync the DDR*/
    DMX_WRITE_REG(ADDR_FQ_WORD2(FQId), WritePtr);
}

/***********************************************************************************
* Function      : DmxHalGetFQWritePtr
* Description   : Set fix int enale
* Input         : FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetFQWritePtr(mt_u32 FQId)
{
    return DMX_READ_REG(ADDR_FQ_WORD2(FQId)) & 0xffff;
}

/***********************************************************************************
* Function      : DmxHalGetFQReadPtr
* Description   : Set fix int enale
* Input         : FQId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetFQReadPtr(mt_u32 FQId)
{
    return DMX_READ_REG(ADDR_FQ_WORD1(FQId)) & 0xffff;
}

/***********************************************************************************
* Function      :  DmxHalSetOQWORDx
* Description   :    Set fix int enale
* Input         : u32FQId,u32Data
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetOQWORDx(mt_u32 OQId, mt_u32 Offset, mt_u32 Value)
{
    wmb();/*sync the DDR*/
    switch (Offset)
    {
        case DMX_OQ_RSV_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD0(OQId), Value);
            break;

        case DMX_OQ_CTRL_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD1(OQId), Value);
            break;

        case DMX_OQ_EOPWR_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD2(OQId), Value);
            break;

        case DMX_OQ_SZUS_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD3(OQId), Value);
            break;

        case DMX_OQ_SADDR_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD4(OQId), Value);
            break;

        case DMX_OQ_RDWR_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD5(OQId), Value);
            break;

        case DMX_OQ_CFG_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD6(OQId), Value);
            break;

        case DMX_OQ_START_OFFSET:
            DMX_WRITE_REG(ADDR_OQ_WORD7(OQId), Value);
            break;

        default:
            break;
    }
}

/***********************************************************************************
* Function      :  DmxHalGetOQWORDx
* Description   :    Set fix int enale
* Input         : OQId,u32Data
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalGetOQWORDx(mt_u32 OQId, mt_u32 Offset, mt_u32 *Value)
{
    switch (Offset)
    {
        case DMX_OQ_RSV_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD0(OQId));
            break;

        case DMX_OQ_CTRL_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD1(OQId));
            break;

        case DMX_OQ_EOPWR_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD2(OQId));
            break;

        case DMX_OQ_SZUS_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD3(OQId));
            break;

        case DMX_OQ_SADDR_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD4(OQId));
            break;

        case DMX_OQ_RDWR_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD5(OQId));
            break;

        case DMX_OQ_CFG_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD6(OQId));
            break;

        case DMX_OQ_START_OFFSET:
            *Value = DMX_READ_REG(ADDR_OQ_WORD7(OQId));
            break;

        default:
            break;
    }
}

//set oq description word mask bit, set bit to 1 for the needless bit
mt_void DxmHalSetOQRegMask(mt_u32 u32MaskValue)
{
    DMX_WRITE_REG(OQ_WR_MASK, u32MaskValue & 0xf);
}

mt_u32 DxmHalGetOQRegMask(mt_void)
{
    return DMX_READ_REG(OQ_WR_MASK);
}

/***********************************************************************************
* Function      :  DmxHalSetOQReadPtr
* Description   :    Set fix int enale
* Input         : u32OQId,u32ReadPtr
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetOQReadPtr(mt_u32 OQId, mt_u32 ReadPtr)
{
    mt_u32 u32MaskValue;

    //wmb();/*sync the DDR*/

    ReadPtr = (ReadPtr & DMX_OQ_DEPTH) << 16;

    if (DmxHalGetOQEnableStatus(OQId) == MT_FALSE)
    {
        u32MaskValue = DxmHalGetOQRegMask();
        DxmHalSetOQRegMask(0x3); //shield the needless bits, prevent from effecting other bits
        DMX_WRITE_REG(ADDR_OQ_WORD5(OQId), ReadPtr);
        DxmHalSetOQRegMask(u32MaskValue);
    }
    else
    {
        DMX_WRITE_REG(ADDR_OQ_WORD5(OQId), ReadPtr);
    }
}

/***********************************************************************************
* Function      :  DmxHalAttachIPBPFQ
* Description   :  Set Ip Back Push OQ and enable
* Input         : u32IPNum,BufferId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalAttachIPBPFQ(mt_u32 PortId, mt_u32 FQId)
{
    mt_u32  offset  = FQId >> 5;
    mt_u32  bit     = FQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value = DMX_READ_REG(IP_BP_FQ_CFG(PortId, offset));

    value |= 1 << bit;
    DMX_WRITE_REG(IP_BP_FQ_CFG(PortId, offset), value);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      :  DmxHalDetachIPBPFQ
* Description   :  Set Ip Back Push OQ and enable
* Input         : u32IPNum,BufferId
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalDetachIPBPFQ(mt_u32 PortId, mt_u32 FQId)
{
    mt_u32  offset  = FQId >> 5;
    mt_u32  bit     = FQId & 0x1f;
    mt_u32  value;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    value = DMX_READ_REG(IP_BP_FQ_CFG(PortId, offset));

    value &= ~(1 << bit);
    DMX_WRITE_REG(IP_BP_FQ_CFG(PortId, offset), value);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      :  DmxHalSetRecTsCounter
* Description   :  Set Record Ts Counter and enable
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetRecTsCounter(mt_u32 u32DmxId, mt_u32 u32OqId)
{
    U_REC_TSCNT_CFG_0 unTsCntCfg0;
    U_REC_TSCNT_CFG_1 unTsCntCfg1;
    mt_u32 u32TsCounter;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    u32TsCounter = u32DmxId;

    unTsCntCfg0.all = DMX_READ_REG(REC_TSCNT_CFG_0);
    unTsCntCfg1.all = DMX_READ_REG(REC_TSCNT_CFG_1);
    switch (u32TsCounter)
    {
    case 0:
        unTsCntCfg0.bits.tscnt0_oqid = u32OqId & 0x7f;
        unTsCntCfg0.bits.tscnt0_ena = 1;
        break;
    case 1:
        unTsCntCfg0.bits.tscnt1_oqid = u32OqId & 0x7f;
        unTsCntCfg0.bits.tscnt1_ena = 1;
        break;
    case 2:
        unTsCntCfg0.bits.tscnt2_oqid = u32OqId & 0x7f;
        unTsCntCfg0.bits.tscnt2_ena = 1;
        break;
    case 3:
        unTsCntCfg0.bits.tscnt3_oqid = u32OqId & 0x7f;
        unTsCntCfg0.bits.tscnt3_ena = 1;
        break;
    case 4:
        unTsCntCfg1.bits.tscnt4_oqid = u32OqId & 0x7f;
        unTsCntCfg1.bits.tscnt4_ena = 1;
        break;
    case 5:
        unTsCntCfg1.bits.tscnt5_oqid = u32OqId & 0x7f;
        unTsCntCfg1.bits.tscnt5_ena = 1;
        break;
    default:
        spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
        return;
    }

    DMX_WRITE_REG(REC_TSCNT_CFG_0, unTsCntCfg0.all);
    DMX_WRITE_REG(REC_TSCNT_CFG_1, unTsCntCfg1.all);
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      :  DmxHalSetRecTsCntReplace
* Description   :  Set Record Ts Counter replace
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetRecTsCntReplace(mt_u32 u32DmxId)
{
    U_SCD_TSCNT_ENA unTsCntRpl;
    mt_u32 u32TsCounter;

    u32TsCounter = u32DmxId;

    unTsCntRpl.all = DMX_READ_REG(SCD_TSCNT_ENA);
    switch (u32TsCounter)
    {
    case 0:
        unTsCntRpl.bits.tscnt0_rep_ena = 1;
        break;
    case 1:
        unTsCntRpl.bits.tscnt1_rep_ena = 1;
        break;
    case 2:
        unTsCntRpl.bits.tscnt2_rep_ena = 1;
        break;
    case 3:
        unTsCntRpl.bits.tscnt3_rep_ena = 1;
        break;
    case 4:
        unTsCntRpl.bits.tscnt4_rep_ena = 1;
        break;
    case 5:
        unTsCntRpl.bits.tscnt5_rep_ena = 1;
        break;
    default:
        return;
    }

    DMX_WRITE_REG(SCD_TSCNT_ENA, unTsCntRpl.all);
}

/***********************************************************************************
* Function      :  DmxHalGetOqCounter
* Description   :  Get Oq Counter
* Data Accessed :  (Optional)
* Data Updated  :  (Optional)
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetOqCounter(mt_u32 OQId)
{
    return DMX_READ_REG(ADDR_INT_CNT(OQId));
}

/***********************************************************************************
* Function      :  DmxHalResetOqCounter
* Description   :  Get Oq Counter
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalResetOqCounter(mt_u32 OQId)
{
    DMX_WRITE_REG(ADDR_INT_CNT(OQId), 0);
}

/***********************************************************************************
* Function      :  DmxHalGetChannelCounter
* Description   :  Get Oq Counter
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_u32 DmxHalGetChannelCounter(mt_u32 ChanId)
{
    return DMX_READ_REG(CHANNEL_TS_COUNT(ChanId));
}

/***********************************************************************************
* Function      :  DmxHalResetOqCounter
* Description   :  Get Oq Counter
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalResetChannelCounter(mt_u32 ChanId)
{
    DMX_WRITE_REG(CHANNEL_TS_COUNT(ChanId), 0);
}

/***********************************************************************************
* Function      :  DmxHalFilterSetSecStuffCtrl
* Description   :
* Input         :  Enable: 1 - receive the stuff not 0xff
*                          0 - do not receive the stuff not 0xff
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalFilterSetSecStuffCtrl(MT_BOOL Enable)
{
    U_DMX_FILTER_CTRL flt_ctrl;

    flt_ctrl.value = DMX_READ_REG(DMX_FILTER_CTRL);
    if (Enable)
    {
        flt_ctrl.bits.sec_stuff_nopusi_e = 1;
    }
    else
    {
        flt_ctrl.bits.sec_stuff_nopusi_e = 0;
    }

    DMX_WRITE_REG(DMX_FILTER_CTRL, flt_ctrl.value);
    DMX_COM_EQUAL(flt_ctrl.value, DMX_READ_REG(DMX_FILTER_CTRL));
}

/***********************************************************************************
* Function      :  DmxHalSetTei
* Description   :
* Input         :
*
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetTei(mt_u32   u32DemuxID,MT_BOOL bCheckTei)
{
    U_DMX_GLB_CTRL1 glb_cfg1;

    glb_cfg1.value = DMX_READ_REG(DMX_GLB_CTRL1(u32DemuxID));/*(0x3B00 + ((DmxId) << 4))*/
    if (bCheckTei)
    {
        glb_cfg1.bits.dmx_tei_ctrl = 1;
    }
    else
    {
        glb_cfg1.bits.dmx_tei_ctrl = 0;
    }
    DMX_WRITE_REG(DMX_GLB_CTRL1(u32DemuxID), glb_cfg1.value);
    DMX_COM_EQUAL(glb_cfg1.value, DMX_READ_REG(DMX_GLB_CTRL1(u32DemuxID)));
}

#ifdef DMX_FILTER_DEPTH_SUPPORT
/***********************************************************************************
* Function      : DmxHalFilterEnableDepth
* Description   : enable filter depth
* Input         :
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalFilterEnableDepth(mt_void)
{
    U_DMX_FILTER_CTRL flt_ctrl;

    flt_ctrl.value = DMX_READ_REG(DMX_FILTER_CTRL);

    flt_ctrl.bits.minlen_discard_by_flt = 1;

    DMX_WRITE_REG(DMX_FILTER_CTRL, flt_ctrl.value);

    DMX_COM_EQUAL(flt_ctrl.value, DMX_READ_REG(DMX_FILTER_CTRL));
}

/***********************************************************************************
* Function      : DmxHalFilterSetDepth
* Description   : Set filter depth
* Input         : FilterId, Depth
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalFilterSetDepth(mt_u32 FilterId, mt_u32 Depth)
{
    U_DMX_FILTER_NUM flt_num;

    flt_num.value = DMX_READ_REG(DMX_FLT_NUM(FilterId));

    flt_num.bits.flt_depth = Depth;

    DMX_WRITE_REG(DMX_FLT_NUM(FilterId), flt_num.value);

    DMX_COM_EQUAL(flt_num.value, DMX_READ_REG(DMX_FLT_NUM(FilterId)));
}
#endif

#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
/***********************************************************************************
* Function      : DmxHalEnableAllRecExcludePid
* Description   : Enable all rec exclude pid function
* Input         :
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalEnableAllRecExcludePid(mt_u32 DmxID)
{
    U_DMX_GLB_CTRL1 glb_cfg1;

    glb_cfg1.value = DMX_READ_REG(DMX_GLB_CTRL1(DmxID));
    glb_cfg1.bits.dmx_allrec_neg_en = 1;
    DMX_WRITE_REG(DMX_GLB_CTRL1(DmxID), glb_cfg1.value);
    DMX_COM_EQUAL(glb_cfg1.value, DMX_READ_REG(DMX_GLB_CTRL1(DmxID)));
}

/***********************************************************************************
* Function      : DmxHalDisableAllRecExcludePid
* Description   : Disable all rec exclude pid function
* Input         :
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalDisableAllRecExcludePid(mt_u32 DmxID)
{
    U_DMX_GLB_CTRL1 glb_cfg1;

    glb_cfg1.value = DMX_READ_REG(DMX_GLB_CTRL1(DmxID));
    glb_cfg1.bits.dmx_allrec_neg_en = 0;
    DMX_WRITE_REG(DMX_GLB_CTRL1(DmxID), glb_cfg1.value);
    DMX_COM_EQUAL(glb_cfg1.value, DMX_READ_REG(DMX_GLB_CTRL1(DmxID)));
}

/***********************************************************************************
* Function      : DmxHalGetAllRecExcludePid
* Description   : Get all rec exclude pid
* Input         :
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalGetAllRecExcludePid(mt_u32 RecCfgID, mt_u32* DmxID, mt_u32* PID)
{
    U_STA_ALLREC_CFG allrec_cfg;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    allrec_cfg.value = DMX_READ_REG(ALLREC_CFG0_29(RecCfgID));

    *DmxID = allrec_cfg.bits.recdel_pid_dmxid;
    *PID   = allrec_cfg.bits.recdel_pid_value;
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

/***********************************************************************************
* Function      : DmxHalSetAllRecExcludePid
* Description   : Set filter depth
* Input         : FilterId, Depth
* Output        :
* Return        :
* Others:       :
***********************************************************************************/
mt_void DmxHalSetAllRecExcludePid(mt_u32 RecCfgID, mt_u32 DmxID, mt_u32 PID)
{
    U_STA_ALLREC_CFG allrec_cfg;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    allrec_cfg.value = DMX_READ_REG(ALLREC_CFG0_29(RecCfgID));

    allrec_cfg.bits.recdel_pid_dmxid = DmxID;
    allrec_cfg.bits.recdel_pid_value = PID;

    DMX_WRITE_REG(ALLREC_CFG0_29(RecCfgID), allrec_cfg.value);

    DMX_COM_EQUAL(allrec_cfg.value, DMX_READ_REG(ALLREC_CFG0_29(RecCfgID)));
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}

#endif

#ifdef DMX_REC_TIME_STAMP_SUPPORT    

/***********************************************************************************
* Function      : DmxHalConfigRecTsTimeStamp
* Description   : Config Record Ts time stamp
* Input         : DmxID,enRecTimeStamp
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalConfigRecTsTimeStamp(mt_u32 DmxID, DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp)
{
    U_TIMESTAMP_CTRL timestamp_ctrl;
    timestamp_ctrl.value = DMX_READ_REG(TIMESTAMP_CTRL);
    timestamp_ctrl.value |= ((enRecTimeStamp << DmxID) && (0x3fff));
    DMX_WRITE_REG(TIMESTAMP_CTRL, timestamp_ctrl.value);
}
#endif

#ifdef DMX_TAG_DEAL_SUPPORT
/*
 * enable or disable TS port tag deal mode
 */
mt_s32 DmxHalSetTagDealCtl(mt_u32 PortId, MT_BOOL bEnable, mt_u32 u32SyncMode, mt_u32 u32TagLen)
{
    U_MUX_SOURCE mux_source;
    U_MUX_CONFIG mux_config;
    U_DBG_DVBT dbg_config;
    mt_u32 TSPortId = 0; /* no mux source */
    
    if (MT_TRUE == bEnable)
    {
        if (DMX_INVALID_PORT_ID == PortId)
        {
            MT_ERR_DEMUX("invalid TS port(%d).\n", PortId);
            return MT_ERR_DMX_INVALID_PARA;
        }

        TSPortId = PortId + DMX_PORT_OFFSET;
        
        mux_source.all = DMX_READ_REG(TS_TAG_MUX_SOURCE); 
        mux_source.bits.mux_dvb_inf_sel = TSPortId;
        DMX_WRITE_REG(TS_TAG_MUX_SOURCE, mux_source.all);

        dbg_config.all = DMX_READ_REG(TS_DBG_DVBT_INTERFACE(PortId));
        dbg_config.bits.dbg_ser_len_bypass = 1;
        DMX_WRITE_REG(TS_DBG_DVBT_INTERFACE(PortId), dbg_config.all);
    }
    else
    {
        if (DMX_INVALID_PORT_ID == PortId )
        {
            TSPortId = 0;
        }
        
        mux_source.all = DMX_READ_REG(TS_TAG_MUX_SOURCE); 
        mux_source.bits.mux_dvb_inf_sel = TSPortId;
        DMX_WRITE_REG(TS_TAG_MUX_SOURCE, mux_source.all);

        dbg_config.all = DMX_READ_REG(TS_DBG_DVBT_INTERFACE(PortId));
        dbg_config.bits.dbg_ser_len_bypass = 0;
        DMX_WRITE_REG(TS_DBG_DVBT_INTERFACE(PortId), dbg_config.all);
    }

    mux_config.all = DMX_READ_REG(TS_TAG_MUX_CONFIG);
    mux_config.bits.mux_work_en = bEnable;
    mux_config.bits.sync_mode_sel = u32SyncMode;
    mux_config.bits.tag_length = u32TagLen;
    DMX_WRITE_REG(TS_TAG_MUX_CONFIG, mux_config.all);

    return MT_SUCCESS;
}

/*
 * set tag values 
 */
mt_void DmxHalSetTagDealAttr(mt_u32 u32TagPortId, mt_u32 u32Low, mt_u32 u32Mid, mt_u32 u32High)
{
    DMX_WRITE_REG(TS_TAG_LOW_REG(u32TagPortId), u32Low);
    DMX_WRITE_REG(TS_TAG_LOW_REG(u32TagPortId), u32Mid);
    DMX_WRITE_REG(TS_TAG_LOW_REG(u32TagPortId), u32High);
}

/*
 * set demux use which tag deal port.
 */
mt_void DmxHalSetDemuxTagPortId(mt_u32 DmxId, mt_u32 u32TagPortId)
{
    mt_u32          Id = 0;
    mt_size_t u32LockFlag;

    spin_lock_irqsave(&DmxHalLcok, u32LockFlag);

    if (u32TagPortId == DMX_INVALID_PORT_ID)
    {
        Id = 0;
    }
    else
    {
        Id = u32TagPortId + DMX_TAG_PORT_OFFSET;
    }
    
    if (DmxId < 4)
    {
        U_SWITCH_CFG0   SwitchCfg0;

        SwitchCfg0.all = DMX_READ_REG(SWITCH_CFG0);

        switch (DmxId)
        {
            case 0:
                SwitchCfg0.bits.switch_cfg1 = Id;
                break;

            case 1:
                SwitchCfg0.bits.switch_cfg2 = Id;
                break;

            case 2:
                SwitchCfg0.bits.switch_cfg3 = Id;
                break;

            case 3:
            default :
                SwitchCfg0.bits.switch_cfg4 = Id;
                break;
        }

        DMX_WRITE_REG(SWITCH_CFG0, SwitchCfg0.all);
    }
    else
    {
        U_SWITCH_CFG1   SwitchCfg1;

        SwitchCfg1.all = DMX_READ_REG(SWITCH_CFG1);

        switch (DmxId)
        {
            case 4:
                SwitchCfg1.bits.switch_cfg5 = Id;
                break;

            case 5:
                SwitchCfg1.bits.switch_cfg6 = Id;
                break;

            case 6:
                SwitchCfg1.bits.switch_cfg7 = Id;
                break;

            default:
                break;
        }

        DMX_WRITE_REG(SWITCH_CFG1, SwitchCfg1.all);
    }
    spin_unlock_irqrestore(&DmxHalLcok, u32LockFlag);
}
#endif

#ifdef DMX_SUPPORT_RAM_CLK_AUTO_CTL
/*
 * ram clk gate control is an built_in mechanism of chip logical for reducing power consumption.
 * here we just enabled it.
 */
mt_void DmxHalEnableRamClkAutoCtl(mt_void)
{
    mt_u32 value;
            
#define ENABLE_REG_OFFSET_1 (0x0a38) 
#define ENABLE_BIT_OFFSET_1 (0)
#define ENABLE_REG_OFFSET_2 (0xc158)  
#define ENABLE_BIT_OFFSET_2 (15)

    value = DMX_READ_REG(ENABLE_REG_OFFSET_1);
    value |= 1 << ENABLE_BIT_OFFSET_1;
    DMX_WRITE_REG(ENABLE_REG_OFFSET_1, value);

    BUG_ON(0 == (DMX_READ_REG(ENABLE_REG_OFFSET_1) & (1 << ENABLE_BIT_OFFSET_1)) );

    value = DMX_READ_REG(ENABLE_REG_OFFSET_2);
    value |= 1 << ENABLE_BIT_OFFSET_2;    
    DMX_WRITE_REG(ENABLE_REG_OFFSET_2, value);

    BUG_ON(0 == (DMX_READ_REG(ENABLE_REG_OFFSET_2) & (1 << ENABLE_BIT_OFFSET_2)) );

}
#endif
