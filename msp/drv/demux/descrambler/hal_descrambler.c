/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"

#include "mt_unf_descrambler.h"

#include "drv_demux_config.h"
#include "../drv_demux_reg.h"
#include "drv_descrambler_reg.h"
#include "hal_descrambler.h"
#include "mt_module_debug.h"
#include "mt_mach/chipinfo.h"


mt_void DmxHalDescramblerSetMode(DescModeSetting_t  *DescramblerMode)
{
    mt_u8 ch = DescramblerMode->ds_cw_ch;
    MT_INFO_DEMUX("even_push_en=%d, odd_push_en=%d, clr_en=%d,  ds_mode=0x%x, ds_core=0x%x,pkt_mode=0x%x, tscfg = 0x%x\n",
                DescramblerMode->ds_even_push_en, DescramblerMode->ds_odd_push_en,
                DescramblerMode->ds_clr_en,  DescramblerMode->ds_mode,
                DescramblerMode->ds_core, DescramblerMode->pkt_mode,
                DescramblerMode->ds_chn_tscfg.all);
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0)
     {
        //reg_set_tsi_keyslot_tab(ch, DescramblerMode->keyslot_tab.all);

          DescramblerMode->ds_chn_tscfg.bitc.ds_mode = DescramblerMode->ds_mode;
	   DescramblerMode->ds_chn_tscfg.bitc.scrtag_clr = DescramblerMode->ds_srctag_clr;
          reg_set_tsi_ds_chn_tscfg(ch, DescramblerMode->ds_chn_tscfg.all);

          DescramblerMode->ades_pktmode.all  = DescramblerMode->pkt_mode;
          //DescramblerMode->ades_pktmode.bitc.small_pkt_mode  = DescramblerMode->pkt_mode;
          reg_set_tsi_ades_pktmode(ch, DescramblerMode->ades_pktmode.all);

          DescramblerMode->ades_disc_mode.bitc.disc_mode = DescramblerMode->disc_mode;
          reg_set_tsi_ades_disc_mode(ch, DescramblerMode->ades_disc_mode.all);
          //reg_set_tsi_aes_ive(ch, DescramblerMode->aes_ive.all);
          reg_set_tsi_ds_core(ch, DescramblerMode->ds_core);
    }
    else      //SYMPHONY_2
    {
    #if defined(CONFIG_MT_CHIP_SYMPHONY1) 
        if(DescramblerMode->ds_even_push_en == 1)
        {
            reg_set_tsi_ds_cw_op((ch << 4) | 0x4);
        }
        if(DescramblerMode->ds_odd_push_en == 1)
        {
            reg_set_tsi_ds_cw_op((ch << 4) | 0x2);
        }
        if(DescramblerMode->ds_clr_en == 1)
        {
            reg_set_tsi_ds_cw_op((ch << 4) | 0x1);
        }
	#endif
	
        reg_set_tsi_ds_chn_tscfg_ds_mode(ch, DescramblerMode->ds_mode);
        reg_set_tsi_ds_chn_tscfg_scrtag_clr(ch, DescramblerMode->ds_srctag_clr);
        reg_set_tsi_ds_core(ch, DescramblerMode->ds_core);
        MT_INFO_DEMUX("\r\n ch=%x, disc_mode=%x, pkt=0x%x\n",ch,
                  DescramblerMode->disc_mode,DescramblerMode->pkt_mode);

        reg_set_tsi_ades_disc_mode(ch, DescramblerMode->disc_mode);
        reg_set_tsi_ades_pktmode(ch, DescramblerMode->pkt_mode);
    }//SYMPHONY_2
}


#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_void DmxHalDescramblerSetModeForSym6(mt_s32 ch, MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S  *attr)
{
	if(attr == NULL)   return;
       if((ch >= DMX_KEY_CNT) ||(ch < 0))  return;
       MT_INFO_DEMUX("%s : %d  >>> attr->tsCfg.validate_tag=%d, %d, %d\n",__FUNCTION__,__LINE__, 
	   		attr->tsCfg.validate_tag,attr->dsCore.validate_tag,attr->pdkMode.validate_tag);
	if(attr->tsCfg.validate_tag)
	{
		reg_set_tsi_ds_chn_tscfg_pes_gost_bit_inv(ch, attr->tsCfg.gost_bit_inv);
		reg_set_tsi_ds_chn_tscfg_pes_gost_sbox_sel(ch, attr->tsCfg.gost_sbox_sel);
		reg_set_tsi_ds_chn_tscfg_pes_multi2_key_msb64(ch, attr->tsCfg.multi2_key_msb64);
		reg_set_tsi_ds_chn_tscfg_pes_csa2_key_msb64(ch, attr->tsCfg.csa2_key_msb64);
		reg_set_tsi_ds_chn_tscfg_pes_des_key_msb64(ch, attr->tsCfg.des_key_msb64);
		MT_INFO_DEMUX("%s : %d  >>> attr->tsCfg.gost_bit_inv=%d, %d\n",__FUNCTION__,__LINE__,
			attr->tsCfg.gost_bit_inv,attr->tsCfg.gost_sbox_sel);
	}
	if(attr->dsCore.validate_tag)
       {
       	reg_set_tsi_ds_core_csa3_opti(ch, attr->dsCore.csa3_opti);
		MT_INFO_DEMUX("%s : %d  >>> attr->dsCore.csa3_opti=%d\n",__FUNCTION__,__LINE__,attr->dsCore.csa3_opti);
       }
	
	if(attr->pdkMode.validate_tag)
	{
		reg_set_tsi_ades_pktmode_short_pkt_mode(ch, attr->pdkMode.short_pkt_mode);
		reg_set_tsi_ades_pktmode_small_pkt_mode(ch, attr->pdkMode.small_pkt_mode);
		MT_INFO_DEMUX("%s : %d  >>> attr->pdkMode.short_pkt_mode=%d, %d\n",__FUNCTION__,__LINE__,
			attr->pdkMode.short_pkt_mode,attr->pdkMode.small_pkt_mode);
	}
       return;
}
#endif /*CONFIG_MT_CHIP_SYMPHONY6*/

/***********************************************************************************
* Function      : DmxHalSetChannelCWIndex
* Description   : Set Channel CW Index
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelCWIndex(mt_u32 ChanId, mt_u32 cwIndex)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.cw_index = cwIndex;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);
}

/***********************************************************************************
* Function      : DmxHalSetChannelDsc
* Description   : Enable Channel Dsc
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetChannelDsc(mt_u32 ChanId, MT_BOOL Enable)
{
    U_DMX_PID_CTRL dmx_pidctrl;

    dmx_pidctrl.all = DMX_READ_REG(DMX_PID_CTRL(ChanId));
    dmx_pidctrl.bits.do_scram = Enable;
    DMX_WRITE_REG(DMX_PID_CTRL(ChanId), dmx_pidctrl.all);
}

mt_u32 DmxHalGetOptCAType(mt_void)
{
    return DMX_READ_REG(CA_INFO0);
}

#ifdef DMX_DESCRAMBLER_VERSION_0

mt_u32 DmxHalGetOptDescramblerType(mt_void)
{
    return DMX_READ_REG(DMX_CA_DBG_0);
}

/***********************************************************************************
* Function      : DmxHalSetCAType
* Description   :
* Input         :
* Output        :
* Return        : 1: Opt set Hard ca; 0: Soft ca
* Others        :
***********************************************************************************/
mt_void DmxHalSetCAType(mt_u32 KeyId, MT_BOOL bAdvance)
{
    mt_u32 value;

    value = DMX_READ_REG(CW_SEL);
    if (bAdvance)
    {
        value |= (1 << KeyId);
    }
    else
    {
        value &= ~(1 << KeyId);
    }

    DMX_WRITE_REG(CW_SEL, value);
}

/***********************************************************************************
* Function      : DmxHalSetDescramblerType
* Description   :
* Input         :
* Output        :
* Return        : 1: Opt set 3.0 desc, 0: 2.0
* Others:
***********************************************************************************/
mt_void DmxHalSetDescramblerType(mt_u32 KeyId, MT_BOOL bHigh)
{
    mt_u32 value;

    value = DMX_READ_REG(CW_CSA3);
    if (bHigh)
    {
        value |= (1 << KeyId);
    }
    else
    {
        value &= ~(1 << KeyId);
    }

    DMX_WRITE_REG(CW_CSA3, value);
}

mt_void DmxHalSetCWWord(mt_u32 u32CWId, mt_u32 u32WordId, mt_u32 u32Data, mt_u32 u32EvenOdd)
{
    U_CW_SET unCwCfg;

    //firstly, config the control register, and then config cw data register, logic map
    u32CWId     = u32CWId & 0x1f;
    u32WordId   = u32WordId & 0x3;
    u32EvenOdd  = u32EvenOdd & 0x1;
    unCwCfg.all = DMX_READ_REG(CW_SET);
    unCwCfg.bits.cw_group_id = u32CWId;
    unCwCfg.bits.cw_word_id  = u32WordId;
    unCwCfg.bits.cw_odd_even = u32EvenOdd;
    DMX_WRITE_REG(CW_SET, unCwCfg.all);

    DMX_WRITE_REG(DMX_CW_DATA, u32Data);
}

#endif

/***********************************************************************************
* Function      : DmxHalSetEntropyReduction
* Description   : Set the valid bits of cw
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetEntropyReduction(mt_u32 KeyId, MT_UNF_DMX_CA_ENTROPY_E EntropyReduction)
{
    mt_u32 Value;

    Value = DMX_READ_REG(CA_ENTROPY);
    if (MT_UNF_DMX_CA_ENTROPY_REDUCTION_CLOSE == EntropyReduction)
    {
        Value |= (1 << KeyId);
    }
    else
    {
        Value &= ~(1 << KeyId);
    }

    DMX_WRITE_REG(CA_ENTROPY, Value);
}

#ifdef DMX_DESCRAMBLER_VERSION_1
/***********************************************************************************
* Function      : DmxHalInitSpeCWOrder
* Description   : set ca type spe cw order as byte and word change
* Input         :
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalInitSpeCWOrder(mt_void)
{
    mt_u32 Value;

    Value = DMX_READ_REG(DMX_CW_CTRL0);

    Value |= 0x33300;   //for spe type:byte and word change
    DMX_WRITE_REG(DMX_CW_CTRL0, Value);
}

/***********************************************************************************
* Function      : DmxHalInitTdesCWOrder
* Description   : set ca type tdes cw order as byte and word change
* Input         :
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalInitTdesCWOrder(mt_void)
{
    mt_u32 Value;

    Value = DMX_READ_REG(DMX_CW_CTRL2);

    Value |= 0x330;   //for tdes ecb and tdes cbc:byte and word change
    DMX_WRITE_REG(DMX_CW_CTRL2, Value);
}

/***********************************************************************************
* Function      : DmxHalSetCWWord1
* Description   : Set CW Word
* Input         :
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalSetCWWord1(
        mt_u32                          GroupId,
        mt_u32                          WordId,
        mt_u32                          Key,
        DMX_KEY_TYPE_E                  KeyType,
        DMX_CW_TYPE                     CWType,
        MT_UNF_DMX_DESCRAMBLER_TYPE_E   DescType
    )
{
    U_CW_SET1   reg;
    mt_u32      type;

    switch (DescType)
    {
        case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3 :
            type = 0x10;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_IPTV :
            type = 0x20;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB :
            type = 0x21;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CI :
            type = 0x22;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_IPTV :
            type = 0x30;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CI :
            type = 0x32;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC :
            type = 0x33;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_NS :
            type = 0x40;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_NS :
            type = 0x41;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_IPTV :
            type = 0x50;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_ECB :
            type = 0x51;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_CBC :
            type = 0x53;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC :
            type = 0x63;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_IPTV :
            type = 0x70;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB :
            type = 0x71;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC :
            type = 0x73;
            break;

        case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2 :
        default :
            type = 0;
    }

    //firstly, config the control register, and then config cw data register, logic map

    reg.value = DMX_READ_REG(CW_SET);

    reg.bits.cw_word_id     = WordId;
    reg.bits.cw_odd_even    = (DMX_KEY_TYPE_ODD == KeyType) ? 1 : 0;
    reg.bits.cw_group_id    = GroupId;
    reg.bits.cw_type        = type;
    reg.bits.cw_iv_sel      = (DMX_KEY_IV == CWType) ? 1 : 0;

    DMX_WRITE_REG(CW_SET, reg.value);

    DMX_WRITE_REG(DMX_CW_DATA, Key);
}

/***********************************************************************************
* Function      :  DmxHalSetChanCwTabId
* Description   :
* Input         :
* Output        :
* Return        :
* Others        :
***********************************************************************************/
mt_void DmxHalSetChanCwTabId(mt_u32 ChanId, mt_u32 TabId)
{
    mt_u32 value;
    mt_u32 offset = ChanId & 0xF;

    value = DMX_READ_REG(CHAN_CW_TAB_ID(ChanId));

    value &= ~(0x3 << (offset * 2));
    value |= TabId << (offset * 2);

    DMX_WRITE_REG(CHAN_CW_TAB_ID(ChanId), value);
}

/***********************************************************************************
* Function      : DmxHalSetDmxIvEnable
* Description   :
* Input         :
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetDmxIvEnable(mt_u32 DmxId, MT_BOOL Enable)
{
    U_DMX_CW_SET stDmxCWSet;

    stDmxCWSet.all = DMX_READ_REG(DMX_CW_SET(DmxId));
    if (Enable != 0)
    {
        stDmxCWSet.bits.cw_iv_en = 1;
    }
    else
    {
        stDmxCWSet.bits.cw_iv_en = 0;
    }
    DMX_WRITE_REG(DMX_CW_SET(DmxId), stDmxCWSet.all);
}

/***********************************************************************************
* Function      :  DmxHalSetCSA3Reset
* Description   :
* Input         :  Enable 1 : set pvr csa3.0 soft reset
*                         0 : clear pvr csa3.0 soft reset
* Output        :
* Return        :
* Others:
***********************************************************************************/
mt_void DmxHalSetCSA3Reset(MT_BOOL Enable)
{
    U_PVR_CSA3_RST reg;

    reg.value = DMX_READ_REG(PVR_CSA3_RST);

    reg.bits.pvr_csa3_soft_rst = Enable;
    DMX_WRITE_REG(PVR_CSA3_RST, reg.value);
}
#endif

