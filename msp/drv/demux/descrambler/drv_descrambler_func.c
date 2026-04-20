/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>

#include "mt_type.h"
#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_module_debug.h"
#include "mt_kernel_adapt.h"
#include "mt_unf_descrambler.h"
#include "drv_demux_config.h"
#include "drv_demux_define.h"
#include "drv_descrambler.h"
#include "drv_descrambler_func.h"
#include "drv_descrambler_reg.h"
#include "hal_descrambler.h"
#include "hal_demux_regs.h"

//#include "drv_advca_ext.h"    //modify yuwu
#include "drv_kt_if.h"
#include "mt_mach/chipinfo.h"

extern DMX_DEV_OSI_S *g_pDmxDevOsi;

//static ADVCA_EXPORT_FUNC_S *g_pAdvcaFunc = MT_NULL;   //by yuwu

/*
** des/aes : big-endian     csa : little-endian
*/

#ifndef CONFIG_MT_KEYTABLE
static mt_u32 DMX_OsiDescramblerSetOddKeys(mt_u8 KeyId, mt_u8 *key, mt_s32 key_length)
{
    mt_u32 key0 = 0;
    mt_u32 key1 = 0;
    mt_u32 key2 = 0;
    mt_u32 key3 = 0;
    mt_u32 key4 = 0;
    mt_u32 key5 = 0;
    mt_u8 p_key[24];

    MT_INFO_DEMUX("DMX_OsiDescramblerSetOddKeys >>> key_length=%d\n",key_length);
    memset(p_key, 0, 24);
    if (key != NULL) {
        memcpy(p_key, key, key_length);
    } else {
        return MT_FAILURE;
    }

    if (key_length >= 4) {
        key0 = ((p_key[0] << 24) & (0xff << 24)) | ((p_key[1] << 16) & (0xff << 16)) | ((p_key[2] << 8) & (0xff << 8)) | ((p_key[3] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_odd0(KeyId, key0);
    }
    if (key_length >= 8) {
        key1 = ((p_key[4] << 24) & (0xff << 24)) | ((p_key[5] << 16) & (0xff << 16)) | ((p_key[6] << 8) & (0xff << 8)) | ((p_key[7] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_odd1(KeyId, key1);
    }
    if (key_length >= 12) {
        key2 = ((p_key[8] << 24) & (0xff << 24)) | ((p_key[9] << 16) & (0xff << 16)) | ((p_key[10] << 8) & (0xff << 8)) | ((p_key[11] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_odd2(KeyId, key2);
    }
    if (key_length >= 16) {
        key3 = ((p_key[12] << 24) & (0xff << 24)) | ((p_key[13] << 16) & (0xff << 16)) | ((p_key[14] << 8) & (0xff << 8)) | ((p_key[15] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_odd3(KeyId, key3);
    }
    if (key_length >= 20) {
        key4 = ((p_key[16] << 24) & (0xff << 24)) | ((p_key[17] << 16) & (0xff << 16)) | ((p_key[18] << 8) & (0xff << 8)) | ((p_key[19] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_odd4(KeyId, key4);
    }
    if (key_length >= 24) {
        key5 = ((p_key[20] << 24) & (0xff << 24)) | ((p_key[21] << 16) & (0xff << 16)) | ((p_key[22] << 8) & (0xff << 8)) | ((p_key[23] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_odd5(KeyId, key5);
    }
    //reg_set_tsi_ds_cw_op_odd_push_en(1);

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
    reg_set_tsi_ds_cw_op((KeyId << 4) | 0x2);
#endif

    //MT_INFO_DEMUX("\r\n dmx_descrambler_set_odd_keys -------- >(ch=%d) %x,%x,%x,%x,%x,%x\n",
    //                                       KeyId,key0,key1,key2,key3,key4,key5);

    return MT_SUCCESS;
}

/*!
  set the even keys

  \param[in] p_dmx The pointer to the demux device.
  \param[in] channel   The channel ID handle.
  \param[in] p_key    The pointer to key buffer.
  \param[in] key_length    The key length.
  */

static mt_u32 DMX_OsiDescramblerSetEvenKeys(mt_u8 KeyId, mt_u8 *key, mt_s32 key_length)
{
    mt_u32 key0 = 0;
    mt_u32 key1 = 0;
    mt_u32 key2 = 0;
    mt_u32 key3 = 0;
    mt_u32 key4 = 0;
    mt_u32 key5 = 0;
    mt_u8 p_key[24];

    memset(p_key, 0, 24);
    if (key != NULL)
        memcpy(p_key, key, key_length);
    else
        return MT_FAILURE;
    MT_INFO_DEMUX("DMX_OsiDescramblerSetEvenKeys >>> key_length=%d\n",key_length);

    if (key_length >= 4) {
        key0 = ((p_key[0] << 24) & (0xff << 24)) | ((p_key[1] << 16) & (0xff << 16)) | ((p_key[2] << 8) & (0xff << 8)) | ((p_key[3] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_even0(KeyId, key0);
    }
    if (key_length >= 8) {
        key1 = ((p_key[4] << 24) & (0xff << 24)) | ((p_key[5] << 16) & (0xff << 16)) | ((p_key[6] << 8) & (0xff << 8)) | ((p_key[7] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_even1(KeyId, key1);
    }
    if (key_length >= 12) {
        key2 = ((p_key[8] << 24) & (0xff << 24)) | ((p_key[9] << 16) & (0xff << 16)) | ((p_key[10] << 8) & (0xff << 8)) | ((p_key[11] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_even2(KeyId, key2);
    }
    if (key_length >= 16) {
        key3 = ((p_key[12] << 24) & (0xff << 24)) | ((p_key[13] << 16) & (0xff << 16)) | ((p_key[14] << 8) & (0xff << 8)) | ((p_key[15] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_even3(KeyId, key3);
    }
    if (key_length >= 20) {
        key4 = ((p_key[16] << 24) & (0xff << 24)) | ((p_key[17] << 16) & (0xff << 16)) | ((p_key[18] << 8) & (0xff << 8)) | ((p_key[19] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_even4(KeyId, key4);
    }
    if (key_length >= 24) {
        key5 = ((p_key[20] << 24) & (0xff << 24)) | ((p_key[21] << 16) & (0xff << 16)) | ((p_key[22] << 8) & (0xff << 8)) | ((p_key[23] << 0) & (0xff << 0));
        reg_set_tsi_ds_chn_even5(KeyId, key5);
    }
    //reg_set_tsi_ds_cw_op_even_push_en(1);

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
    reg_set_tsi_ds_cw_op((KeyId << 4) | 0x4);
#endif

    //MT_INFO_DEMUX("\r\n dmx_descrambler_set_even_keys --------- >(ch=%d)%x,%x,%x,%x,%x,%x\n",
    //           KeyId,key0,key1,key2,key3,key4,key5);

    return MT_SUCCESS;
}
#endif

#ifdef DMX_DESCRAMBLER_VERSION_1
mt_void DescInitHardFlag(mt_void)
{
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    U_CA_INFO0 CaInfo;

    //DmxHalInitSpeCWOrder();
    //DmxHalInitTdesCWOrder();

    DmxDevOsi->KeyCsa2HardFlag = 0;
    DmxDevOsi->KeyCsa3HardFlag = 0;
    DmxDevOsi->KeySpeHardFlag = 0;
    DmxDevOsi->KeyOtherHardFlag = 0;

    CaInfo.value = DmxHalGetOptCAType();
    if (CaInfo.bits.hardonly_csa2) {
	DmxDevOsi->KeyCsa2HardFlag = DMX_KEY_HARDONLY_FLAG;
    }

    if (CaInfo.bits.hardonly_spe) {
	DmxDevOsi->KeySpeHardFlag = DMX_KEY_HARDONLY_FLAG;
    }

    if (CaInfo.bits.hardonly_csa3) {
	DmxDevOsi->KeyCsa3HardFlag = DMX_KEY_HARDONLY_FLAG;
    }

    if (0 == CaInfo.value) // not handonly, open iv enable
    {
	mt_u32 DmxId;

	for (DmxId = 0; DmxId < DMX_CNT; DmxId++) {
	    DmxHalSetDmxIvEnable(DmxId, MT_TRUE);
	}
    }
}
#endif

mt_void DescramblerReset(mt_u32 KeyId, DMX_KeyInfo_S *KeyInfo)
{

#ifdef MT_DEMUX_PROC_SUPPORT
    KeyInfo->ChanCount = 0;

    memset(KeyInfo->EvenKey, 0, sizeof(KeyInfo->EvenKey));
    memset(KeyInfo->OddKey, 0, sizeof(KeyInfo->OddKey));
#endif

    if (MT_UNF_DMX_CA_NORMAL == KeyInfo->CaType)
    {
#ifdef CONFIG_MT_KEYTABLE
        void *p_priv = drv_kt_get_handle();
        MT_INFO_DEMUX("[%s]   KeyInfo->KeyLen = %d, odd_key_slot_index= %d,  even_key_slot_index= %d\n", __FUNCTION__,  KeyInfo->KeyLen, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index, KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index );
        if (KeyInfo->KeyLen == 8)   //8*4 byte key
        {
            if (KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index != MT_KT_SLOT_ID_INVALID)
            {
                drv_kt_slot_release(p_priv, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index);
                drv_kt_slot_release(p_priv, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index+1);
                KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index = MT_KT_SLOT_ID_INVALID;

            }
            if (KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index != MT_KT_SLOT_ID_INVALID)
            {
                drv_kt_slot_release(p_priv, KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index);
                drv_kt_slot_release(p_priv, KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index+1);
                KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index = MT_KT_SLOT_ID_INVALID;
            }
        }
        else
        {
            if (KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index != MT_KT_SLOT_ID_INVALID)
            {
                drv_kt_slot_release(p_priv, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index);
                KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index = MT_KT_SLOT_ID_INVALID;
            }
            if (KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index != MT_KT_SLOT_ID_INVALID)
            {
                drv_kt_slot_release(p_priv, KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index);
                KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index = MT_KT_SLOT_ID_INVALID;
            }
        }
#endif
    }

    KeyInfo->CaType = MT_UNF_DMX_CA_BUTT;
}

mt_s32 DescCreate(mt_u32 KeyId, mt_u32 CaType, mt_u32 DescType, mt_u32 CaEntropy)
{
    mt_u8 key[DMX_KEY_MAX_LEN] = { 0 };

    //if (MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2 == DescType) {
	//DmxHalSetEntropyReduction(KeyId, CaEntropy);
    //}
    reg_set_tsi_ds_chn_tscfg(KeyId, 0);
    reg_set_tsi_ades_pktmode(KeyId, 0);
    reg_set_tsi_ades_disc_mode(KeyId, 0);
    reg_set_tsi_ds_core(KeyId, 0);
    reg_set_tsi_aes_ive_ivecal_en(KeyId, 0);
    DMX_OsiDescramblerSetIVKey(KeyId, DMX_KEY_TYPE_EVEN, key);
    DMX_OsiDescramblerSetIVKey(KeyId, DMX_KEY_TYPE_ODD, key);

    return MT_SUCCESS;
}

static mt_s32 DMXDescramblerConfigMode(MT_UNF_DMX_DESCRAMBLER_TYPE_E enDescramblerType,
											MT_DMX_OsiDescrambler_ModeConfig * modeConfig)
{
    mt_u32 KeyLen = DMX_KEY_MAX_LEN / sizeof(mt_u32);
    mt_u8 ivecal_mode = modeConfig->ivecal_mode;
    mt_u8 pkt_mode = 0x0;
    mt_u8 ds_mode = 2;  //tscfg.bitc.ds_mode
    mt_u8 ds_core = 0;
    mt_u32 disc_mode = 0x0;
    mt_u8 disc_mode_ctr = 0x0;

    switch (enDescramblerType) {
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2:
    	reg_set_dmx_ds_big_little_endian(1);
	ds_mode = 2;
	disc_mode = 0x0;
	KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE:
#ifndef CONFIG_MT_CHIP_SYMPHONY6
	reg_set_dmx_ds_big_little_endian(0);
#endif
	ds_mode = 2;
	disc_mode = 0x0;
	KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_TS:
    	reg_set_dmx_ds_big_little_endian(1);
	ds_mode = 0;
	disc_mode = 0x0;
	KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_PES:
    	reg_set_dmx_ds_big_little_endian(1);
	ds_mode = 1;
	disc_mode = 0x0;
	KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_CONFORMANCE:
    	reg_set_dmx_ds_big_little_endian(1);
        ds_mode = 1;
	disc_mode = 0x0;
        ds_core = 4;
	KeyLen = 2;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3:
	ds_mode = 2;
	disc_mode = 0x0;
	ds_core = 2;
	KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3_TS:
	ds_mode = 0;
	disc_mode = 0x0;
	ds_core = 2;
	KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3_PES:
	ds_mode = 1;
	disc_mode = 0x0;
	ds_core = 2;
	KeyLen = 4;
	break;

    /*AES*/
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_HEAD_CLEAR:
	pkt_mode = 0x0;
	disc_mode = 0x62;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_TAIL_CLEAR:
	pkt_mode = 0x1;
	disc_mode = 0x62;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_HEAD_CLEAR:
	pkt_mode = 0x0;
	disc_mode = 0x6a;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR:
	pkt_mode = 0x1;
	disc_mode = 0x6a;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS:
	pkt_mode = 0x22;
	ivecal_mode = 0x10;
	disc_mode = 0x6e;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS_XOR:
	pkt_mode = 0x22;
	disc_mode = 0x6e;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042_TAIL_CLEAR:
	disc_mode = 0x12;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042:
	pkt_mode = 0x12;
	disc_mode = 0x12;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_RCBC_CTS:
	pkt_mode = 0x21;
	ivecal_mode = 0x13;
	disc_mode = 0x76;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS1:
	pkt_mode = 0x02;
	disc_mode = 0x8e;   //1011(b7 b4 b3 b2)(cbc cts1)  010(b30 b1 b0)(aes)
	ds_core = 1;
       KeyLen = 4;
	break;

    /*TDES*/
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_HEAD_CLEAR:
       pkt_mode = 0x0;
	disc_mode = 0x61;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_TAIL_CLEAR:
	pkt_mode = 0x1;
	disc_mode = 0x61;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_CTS:
	disc_mode = 0x65;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_HEAD_CLEAR:
	pkt_mode = 0x0;
	disc_mode = 0x69;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_TAIL_CLEAR:
	pkt_mode = 0x1;
	disc_mode = 0x69;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_DVS042:
	disc_mode = 0x71;
	ds_core = 1;
       pkt_mode = 0x12;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_CTS1:
	pkt_mode = 0x02;
	disc_mode = 0x8d;   //1011(b7 b4 b3 b2)(cbc cts1)  001(b30 b1 b0)(tdes)
	ds_core = 1;
       KeyLen = 4;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_HEAD_CLEAR:
	pkt_mode = 0x0;
	disc_mode = 0x68;
	ds_core = 1;
       KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_TAIL_CLEAR:
	pkt_mode = 0x1;
	disc_mode = 0x68;
	ds_core = 1;
       KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_HEAD_CLEAR:
	pkt_mode = 0x0;
	disc_mode = 0x60;
	ds_core = 1;
       KeyLen = 2;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_TAIL_CLEAR:
	pkt_mode = 0x1;
	disc_mode = 0x60;
	ds_core = 1;
       KeyLen = 2;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_CLR:
	ds_core = 1;
	disc_mode = 0x04;   //0001(b7b4b3b2) 000(b30 b1 b0)   (ecb-cts des)
	disc_mode_ctr = 0;
	ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
	pkt_mode = 0x02;	   //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	KeyLen = 2;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_XOR:
	ds_core = 1;
	disc_mode = 0x04;   //0001(b7b4b3b2) 000(b30 b1 b0)   (ecb-cts des)
	disc_mode_ctr = 0;
	ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
	pkt_mode = 0x22;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	KeyLen = 2;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_SCTE52:
	ds_core = 1;
	disc_mode = 0x1C;   //0111(b7b4b3b2) 000(b30 b1 b0)   (scte52 des)
	disc_mode_ctr = 0;
	ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
	pkt_mode = 0X12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	KeyLen = 2;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_CTS1:
	pkt_mode = 0x02;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	disc_mode = 0x8c;   //1011(b7 b4 b3 b2)(cbc cts1)  000(b30 b1 b0)(des)
	ds_core = 1;
       KeyLen = 2;
	break;

    //add disc mod ctr  DES
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x78;
	disc_mode_ctr = 0x0;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_H:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x78;
	disc_mode_ctr = 0x1;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_L:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x78;
	disc_mode_ctr = 0x2;
	ds_core = 1;
       KeyLen = 4;
	break;


      // 3DES
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x79;
	disc_mode_ctr = 0x0;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_H:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x79;
	disc_mode_ctr = 0x1;
	ds_core = 1;
       KeyLen = 4;
	break;
     case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_L:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x79;
	disc_mode_ctr = 0x2;
	ds_core = 1;
       KeyLen = 4;
	break;

	//AES
    case  MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR:                     /*add disc_mod_ctr*/
	pkt_mode = 0x12;
	disc_mode = 0x7a;
	disc_mode_ctr = 0x0;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR_H:   //01 110 010    = 0x11a
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x7a;
	disc_mode_ctr = 0x1;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR_L:
	pkt_mode = 0x12;	    //bit0-1: 00 head_clear, 01 tail_clear , 10 cal    bit4-5:  00 clear, 10 or, 01 cal
	disc_mode = 0x7a;
	disc_mode_ctr = 0x2;
	ds_core = 1;
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLER_TYPE_ASA:
	ds_mode = 2;
	disc_mode = 0x0;
	ds_core = 3;
	KeyLen = 4;
	break;
	/*  ds_mode config :
       {b8 b1 b0 } 000 des, 001 tdes(aba),010 aes, 011 sm4, 100 multi2,
                         101 gost28147, 110 gost3412 kuznyechik, 111 gost3412 magma

	{b7 b4 b3 b2} 000 ecb, 001 ecb-cts, 010 cbc, 011 cbc-cts, 100 cbc-dvs042(cbc-ofb)
	                     101 rcbc, 110 ctr, 111 scte52, 1000 ecb cts2, 1010 cfb
	*/
    case MT_UNF_DMX_DESCRAMBLE_SM4_ECB_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 3;   //000 011  (sm4_ecb)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_ECB_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 3;   //000 011  (sm4_ecb)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x0b;   //0010(b7b4b3b2) 011(b30 b1 b0)   (sm4_cbc)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x0b;  //0010(b7b4b3b2) 011(b30 b1 b0)   (sm4_cbc)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_OFB:
	ds_core = 1;
	disc_mode = 0x13;   //0100(b7b4b3b2) 011(b30 b1 b0)   (sm4_ofb)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0X10;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS:
	ds_core = 1;
	disc_mode = 0x0f;    //0011(b7b4b3b2) 011(b30 b1 b0)   (sm4_cbc_cts)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x02;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS_XOR:
	ds_core = 1;
	disc_mode = 0x0f;    //0011(b7b4b3b2) 011(b30 b1 b0)   (sm4_cbc_cts)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x22;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
       KeyLen = 4;
	break;
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS1:
	pkt_mode = 0x02;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	disc_mode = 0x8f;   //1011(b7 b4 b3 b2)(cbc cts1)  011(b30 b1 b0)(sm4)
	ds_core = 1;
       KeyLen = 4;
	break;

    /*gost 3412k*/
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_ECB_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000002;   //0000(b7b4b3b2) 110(b30 b1 b0)   (gost3412k ecb)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_ECB_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000002;   //000  110(b30 b1 b0)  (gost3412k ecb)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x4000000a;   //010(b4b3b2)  110(b30 b1 b0)  (cbc gost3412k )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x4000000a;   //010(b4b3b2)  110(b30 b1 b0)  (cbc gost3412k )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_OFB:
	ds_core = 1;
	disc_mode = 0x40000012;   //0100(b7b4b3b2) 110(b30 b1 b0)   (cbc-ofb gost3412k )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CTR:
	ds_core = 1;
	disc_mode = 0x4000001a;   //0110(b7b4b3b2) 110(b30 b1 b0)   (cbc-ctr gost3412k )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CFB:
	ds_core = 1;
	disc_mode = 0x4000008a;   //1010(b7b4b3b2) 110(b30 b1 b0)   (ofb gost3412k )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_CTS1:
	pkt_mode = 0x02;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	disc_mode = 0x4000008e;   //1011(b7 b4 b3 b2)(cbc cts1)  110(b30 b1 b0)(gost3412k)
	ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
	ds_core = 1;
       KeyLen = 8;
	break;

    /*gost 28147*/
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_ECB_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000001;   //000  101(b30 b1 b0)  (ecb gost28147)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_ECB_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000001;   //000  101(b30 b1 b0)  (ecb gost28147)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000009;   //010(b4b3b2)  101(b30 b1 b0)  (cbc gost28147 )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000009;   //010(b4b3b2)  101(b30 b1 b0)  (cbc gost28147 )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_OFB:
	ds_core = 1;
	disc_mode = 0x40000011;   //0100(b7b4b3b2) 101(b30 b1 b0)   (ofb gost28147 )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0X12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CTR:
	ds_core = 1;
	disc_mode = 0x40000019;   //0110(b7b4b3b2)  101(b30 b1 b0)   (ctr gost28147 )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CFB:
	ds_core = 1;
	disc_mode = 0x40000089;   //1010(b7b4b3b2) 101(b30 b1 b0)   (cfb gost28147 )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_CTS1:
	pkt_mode = 0x02;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	disc_mode = 0x4000008d;   //1011(b7 b4 b3 b2)(cbc cts1)  101(b30 b1 b0)(gost28147)
	ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
	ds_core = 1;
       KeyLen = 8;
	break;


    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_ECB_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000003;   //000  111(b30 b1 b0)  (ecb gost3412m)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_ECB_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x40000003;   //000  111(b30 b1 b0)  (ecb gost3412m)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_HEAD_CLEAR:
	ds_core = 1;
	disc_mode = 0x4000000b;   //010(b4b3b2)  111(b30 b1 b0)  (cbc gost3412m )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_TAIL_CLEAR:
	ds_core = 1;
	disc_mode = 0x4000000b;   //010(b4b3b2)  111(b30 b1 b0)  (cbc gost3412m )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 1;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_OFB:
	ds_core = 1;
	disc_mode = 0x40000013;   //0100(b7b4b3b2) 111(b30 b1 b0)   (ofb gost3412m )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0X12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CTR:
	ds_core = 1;
	disc_mode = 0x4000001b;   //0110(b7b4b3b2)  111(b30 b1 b0)   (ctr gost3412m )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CFB:
	ds_core = 1;
	disc_mode = 0x4000008b;   //1010(b7b4b3b2) 111(b30 b1 b0)   (cfb gost3412m )
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 8;
	break;
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_CTS1:
	pkt_mode = 0x02;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt 0 clear   2 xor  1 cal
	disc_mode = 0x4000008f;   //1011(b7 b4 b3 b2)(cbc cts1)  111(b30 b1 b0)(gost3412m)
	ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
	ds_core = 1;
       KeyLen = 8;
	break;

    case MT_UNF_DMX_DESCRAMBLE_MULTI2_CBC_OFB:
	ds_core = 1;
	disc_mode = 0x40000010;   //0100(b7b4b3b2) 100(b30 b1 b0)   (ofb multi2)
       disc_mode_ctr = 0;
       ds_mode = 2;            //0:ts  1:pes 2:auto 3:whole
      	pkt_mode = 0x12;	    //bit0-1: 0 head_clear, 1 tail_clear , 2 cal   bit4-5: small_pkt
       KeyLen = 2;
	break;



    /*montage ca type*/
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_IPTV:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CI:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CI:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_IPTV:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_NS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_NS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_IPTV:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_ECB:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_SMS4_CBC:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_IPTV:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC:
	MT_INFO_DEMUX("MT_ERR_DMX_NOT_SUPPORT   Invalid DescramblerType %d\n", enDescramblerType);
	return MT_ERR_DMX_NOT_SUPPORT;

    default:
	MT_INFO_DEMUX("Invalid DescramblerType %d\n", enDescramblerType);
	return MT_ERR_DMX_INVALID_PARA;
    }
    modeConfig->disc_mode = disc_mode;
    modeConfig->disc_mode_ctr = disc_mode_ctr;
    modeConfig->ds_core = ds_core;
    modeConfig->ds_mode = ds_mode;
    modeConfig->pkt_mode = pkt_mode;
    modeConfig->KeyLen= KeyLen;
    modeConfig->ivecal_mode = ivecal_mode;
    MT_INFO_DEMUX(" modeConfig->pkt_mode %d\n", pkt_mode);
    return MT_SUCCESS;
}

#ifdef CONFIG_MT_KEYTABLE

static mt_s32 DMXDescramblerConfigKeyAttribute(mt_u32  DescType, MT_KT_KEY_ATTR_S *p_ka)
{
    if (p_ka == NULL)
	return MT_FAILURE;

    memset(p_ka, 0, sizeof(MT_KT_KEY_ATTR_S));
    p_ka->DEC_ONOFF = MT_KT_ATTR_ON;
    p_ka->ENC_ONOFF = MT_KT_ATTR_OFF;

    p_ka->REE_ONOFF = MT_KT_ATTR_ON;
    p_ka->TS_ONOFF = MT_KT_ATTR_ON;

    switch (DescType) {
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_TS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_PES:
        p_ka->CSAv2_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_8B_SIZE;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_CONFORMANCE:
        p_ka->CSAv2_CONFORMANCE_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_8B_SIZE;
    break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE:
        p_ka->CSAv2_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_8B_SIZE;
#ifndef CONFIG_MT_CHIP_SYMPHONY6
        reg_set_dmx_ds_big_little_endian(0);
#endif
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3_TS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3_PES:
         reg_set_dmx_ds_big_little_endian(1);
	 p_ka->CSAv3_ONOFF = MT_KT_ATTR_ON;
         p_ka->KEY_SIZE = MT_KT_SLOT_16B_SIZE;
	 break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_CLR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_XOR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_SCTE52:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_CTS1:
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR:
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_H:
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_L:
	 p_ka->DES_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_8B_SIZE;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_CTS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_DVS042:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_CTS1:
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR:
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_H:
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_L:
        p_ka->TDES_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_16B_SIZE;
	break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS_XOR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_RCBC_CTS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_CTS1:
    case  MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR:                     /*add disc_mod_ctr*/
    case MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR_H:
    case MT_UNF_DMX_DESCRAMBLE_DESC_AES_CTR_L:
        p_ka->AES_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_16B_SIZE;
	break;

    case MT_UNF_DMX_DESCRAMBLE_SM4_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_SM4_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_OFB:
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS:
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS_XOR:
    case MT_UNF_DMX_DESCRAMBLE_SM4_CBC_CTS1:
        p_ka->SM2_3_4_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_16B_SIZE;
	break;

    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_OFB:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CTR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CFB:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412K_CBC_CTS1:
        p_ka->GOST_R34_12_KUZNYECHIK_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_32B_SIZE;
        break;

    case MT_UNF_DMX_DESCRAMBLE_GOST28147_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_OFB:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CTR:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CFB:
    case MT_UNF_DMX_DESCRAMBLE_GOST28147_CBC_CTS1:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_OFB:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CTR:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CFB:
    case MT_UNF_DMX_DESCRAMBLE_GOST3412M_CBC_CTS1:
        p_ka->GOST_28147_89_OR_R34_12_MAGMA_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_32B_SIZE;
        break;

    case MT_UNF_DMX_DESCRAMBLE_MULTI2_CBC_OFB:
        p_ka->Multi2_ONOFF = MT_KT_ATTR_ON;
        p_ka->KEY_SIZE = MT_KT_SLOT_8B_SIZE;
        break;

    case MT_UNF_DMX_DESCRAMBLER_TYPE_BUTT:

	break;
    default:
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}
#endif

static mt_s32 DMXDescramblerSetIvMode(mt_u8 descramblerId, MT_UNF_DMX_IVE_calc_mode_t ivMode)
{
    mt_u8 ivecal_mode = 0;

    MT_INFO_DEMUX("symphony_get_chip_rev() = 0x%x, ivMode=0x%x\n", symphony_get_chip_rev(), ivMode);
    switch (ivMode) {
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_DES: //00 00 0
	ivecal_mode = 0;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_TDES: //01 00 0
	ivecal_mode = 0x8;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_AES: //10 00 0
	ivecal_mode = 0x10;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDI_DISABLE: //11 00 0
	ivecal_mode = 0x18;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_DES: //00 01 0
	ivecal_mode = 0x2;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_TDES: //01 01 0
	ivecal_mode = 0xa;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_AES: //10 01 0
	ivecal_mode = 0x12;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MDD_DISABLE: //11 01 0
	ivecal_mode = 0x1a;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_DES: //00 10 0
	ivecal_mode = 0x4;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_TDES: //01 10 0
	ivecal_mode = 0xc;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_AES: //10 10 0
	ivecal_mode = 0x14;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_CBC_MSC_DISABLE: //11 10 0
	ivecal_mode = 0x1c;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_DES: //00 00 1
	ivecal_mode = 0x1;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_TDES: //01 00 1
	ivecal_mode = 0x9;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_AES: //10 00 1
	ivecal_mode = 0x11;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDI_DISABLE: //11 00 1
	ivecal_mode = 0x19;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_DES: //00 01 1
	ivecal_mode = 0x3;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_TDES: //01 01 1
	ivecal_mode = 0xb;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_AES: //10 01 1
	ivecal_mode = 0x13;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MDD_DISABLE: //11 01 1
	ivecal_mode = 0x1b;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_DES: //00 10 1
	ivecal_mode = 0x5;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_TDES: //01 10 1
	ivecal_mode = 0xd;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_AES: //10 10 1
	ivecal_mode = 0x15;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_RCBC_MSC_DISABLE: //11 10 1
	ivecal_mode = 0x1d;
	break;
    case MT_UNF_DMX_IVE_CALC_MODE_DISABLE:
	ivecal_mode = 0x0;
	break;
    default:
	ivecal_mode = 0x0;
	break;
    }
    MT_INFO_DEMUX("symphony_get_chip_rev() = 0x%x, ivecal_mode=0x%x\n", symphony_get_chip_rev(), ivecal_mode);
    if (ivecal_mode != 0) {
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	if (symphony_get_chip_rev() >= CHIP_SYMPHONY4_A0) //SYMPHONY_4
	{
	    reg_tsi_aes_ive_t_sym6 aes_ive;
	    aes_ive.bitc.ivecal_en = 1;
	    aes_ive.bitc.ivecal_mode = ivecal_mode;
	    reg_set_tsi_aes_ive(descramblerId, aes_ive.all);
	    MT_INFO_DEMUX("aes_ive.all = 0x%x\n", aes_ive.all);
	}
#else
	if (symphony_get_chip_rev() >= CHIP_SYMPHONY4_A0) //SYMPHONY_4
	{
	    reg_tsi_aes_ive_t_sym4 aes_ive;
	    aes_ive.bitc.ivecal_en = 1;
	    aes_ive.bitc.ivecal_mode = ivecal_mode;
	    reg_set_tsi_aes_ive(descramblerId, aes_ive.all);
	    MT_INFO_DEMUX("aes_ive.all = 0x%x\n", aes_ive.all);
	}
	else if (symphony_get_chip_rev() >= CHIP_SYMPHONY2_A0) //SYMPHONY_2
	{
	    reg_tsi_aes_ive_t_sym2 aes_ive;
	    aes_ive.bitc.ivecal_en = 1;
	    aes_ive.bitc.iveinit_reg_sel = 0x2;
	    aes_ive.bitc.ivecal_mode = ivecal_mode;
	    reg_set_tsi_aes_ive(descramblerId, aes_ive.all);
	}
	else
	{
	    reg_tsi_aes_ive_t aes_ive;
	    aes_ive.bitc.ivecal_en = 1;
	    aes_ive.bitc.iveinit_reg_sel = 0x1;
	    aes_ive.bitc.ivecal_mode = ivecal_mode;
	    reg_set_tsi_aes_ive(descramblerId, aes_ive.all);
	}
#endif
    }

    return MT_SUCCESS;
}

mt_void DmxDescramblerResume(mt_void)
{
    mt_u32 i;

    for (i = 0; i < DMX_KEY_CNT; i++) {
	DMX_KeyInfo_S *KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[i];

	if (MT_UNF_DMX_CA_BUTT != KeyInfo->CaType) {

#ifdef DMX_DESCRAMBLER_VERSION_1
	    DescCreate(i, KeyInfo->CaType, KeyInfo->DescType, KeyInfo->CaEntropy);
#endif

#ifdef MT_DEMUX_PROC_SUPPORT
	    KeyInfo->ChanCount = 0;

	    DMX_OsiDescramblerSetKey(i, DMX_KEY_TYPE_EVEN, (mt_u8 *)KeyInfo->EvenKey);
	    DMX_OsiDescramblerSetKey(i, DMX_KEY_TYPE_ODD, (mt_u8 *)KeyInfo->OddKey);

	    KeyInfo->DescramblerMode.ds_even_push_en = 0;
	    KeyInfo->DescramblerMode.ds_odd_push_en = 0;
	    KeyInfo->DescramblerMode.ds_clr_en = 1;
	    KeyInfo->DescramblerMode.ds_srctag_clr = 0;
	    KeyInfo->DescramblerMode.ds_cw_ch = i;
	    KeyInfo->DescramblerMode.ds_mode = 0;
	    KeyInfo->DescramblerMode.ds_core = 0;
	    KeyInfo->DescramblerMode.ivecal_mode = 0;
	    KeyInfo->DescramblerMode.disc_mode = 0;

	    KeyInfo->DescramblerMode.pkt_mode = 0; //~ive

	    DmxHalDescramblerSetMode(&(KeyInfo->DescramblerMode));
#endif
	}
    }
}

/***********************************************************************************
* Function      : DMXDescramblerCreate1
* Description   : create a new descrambler
* Input         :
* Output        : KeyId
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others:
***********************************************************************************/
static mt_s32 DMXDescramblerCreate1(mt_u32 *KeyId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *Attr, mt_u32 evenKeySlot, mt_u32 oddKeySlot)
{
    mt_s32 ret = MT_ERR_DMX_NOFREE_KEY;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_KeyInfo_S *KeyInfo = DmxDevOsi->DmxKeyInfo;
    U_CA_INFO0 CaInfo;
    mt_u32 i;
    MT_DMX_OsiDescrambler_ModeConfig  modeConfig={0,};

    CaInfo.value = DmxHalGetOptCAType();
    MT_INFO_DEMUX("Attr->enDescramblerType = %d\n", Attr->enDescramblerType);
    modeConfig.ivecal_mode = Attr->ivMode;
    DMXDescramblerConfigMode(Attr->enDescramblerType,  &modeConfig);


    MT_INFO_DEMUX("%s >>> line=%d\n",__FUNCTION__,__LINE__);

    if(Attr->enDecOrEncMode == MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN)
    {
        modeConfig.disc_mode = modeConfig.disc_mode & 0x9f;   //  enable enc bit5 = 0, bit6=0;
    }
    if (0 == down_interruptible(&DmxDevOsi->lock_Key)) {
	for (i = 0; i < DMX_KEY_CNT; i++)  {
	    if (MT_UNF_DMX_CA_BUTT != KeyInfo[i].CaType) {
		continue;
	    }

	    if (MT_SUCCESS == DescCreate(i, Attr->enCaType, Attr->enDescramblerType, Attr->enEntropyReduction)) {
		KeyInfo[i].CaType = Attr->enCaType;
		KeyInfo[i].CaEntropy = Attr->enEntropyReduction;
		KeyInfo[i].DescType = Attr->enDescramblerType;
		KeyInfo[i].KeyLen = modeConfig.KeyLen;

		KeyInfo[i].DescramblerMode.ds_even_push_en = 1;
		KeyInfo[i].DescramblerMode.ds_odd_push_en = 1;
		KeyInfo[i].DescramblerMode.ds_clr_en = 0;
		
		if(Attr->enTsDescFlag == MT_UNF_DMX_DESCRAMBLER_DONOT_DELETE_DESC_TAG)
			KeyInfo[i].DescramblerMode.ds_srctag_clr = 0;
		else
			KeyInfo[i].DescramblerMode.ds_srctag_clr = 1;
		
		KeyInfo[i].DescramblerMode.ds_cw_ch = i;
		KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = 0;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = 1;
		MT_INFO_DEMUX("KeyInfo[i].DescramblerMode.ds_chn_tscfg.all = 0x%x\n",KeyInfo[i].DescramblerMode.ds_chn_tscfg.all);	  
#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	       MT_INFO_DEMUX("symphony1 do not support enc !\n");
#else
		switch(Attr->enDecOrEncMode)
		{
			case MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN:
			case MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_ODD:
			{
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 1;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = 1; //odd
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = 0;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = 1;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = 1;
				break;
			}
			case MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_EVEN:
			{
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 1;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = 0;  //even
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = 0;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = 1;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = 1;
				break;
			}
			default:
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 0;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = 0;  //even
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = 0;
				//KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = 1;
				//KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = 1;
				break;
		}
#endif
		KeyInfo[i].DescramblerMode.ds_mode = modeConfig.ds_mode;
		KeyInfo[i].DescramblerMode.ds_core = modeConfig.ds_core;
		KeyInfo[i].DescramblerMode.ivecal_mode = modeConfig.ivecal_mode;
		KeyInfo[i].DescramblerMode.disc_mode = modeConfig.disc_mode;
		KeyInfo[i].DescramblerMode.ades_disc_mode.bitc.disc_mode_ctr = modeConfig.disc_mode_ctr;
		MT_INFO_DEMUX("KeyInfo[i].DescramblerMode.ds_chn_tscfg.all = 0x%x\n",KeyInfo[i].DescramblerMode.ds_chn_tscfg.all);
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
              switch (Attr->tsCfg.tscfgCwopt1Mode)
              {
                    case MT_UNF_DMX_ACTIVE_PES_LOWBIT_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 1;
                        break;
                    case MT_UNF_DMX_ACTIVE_PES_NO_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
                        break;
                    case MT_UNF_DMX_INACTIVE_PES_SCB_NO_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 1;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
                        break;
                   case MT_UNF_DMX_INACTIVE_PES_LOWBIT_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 1;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 1;
                        break;
                    default:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
                        break;
              }
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.gost_sbox_sel = Attr->tsCfg.gost_sbox_sel;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.gost_bit_inv = Attr->tsCfg.gost_bit_inv;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.multi2_key_msb64 = Attr->tsCfg.multi2_key_msb64;
              if(modeConfig.disc_mode & 0x40000000){
                   KeyInfo[i].DescramblerMode.ades_disc_mode.bitc.dis_mode_b8 = 1;
              }else{
                   KeyInfo[i].DescramblerMode.ades_disc_mode.bitc.dis_mode_b8 = 0;
              }
#endif
		MT_INFO_DEMUX("ds_cw_ch = %d, ds_mode=%x, ds_core=%x,ivecal_mode=%x, disc_mode=%x, pkt_mode=0x%x, tscfg=0x%x\n",
		       i, modeConfig.ds_mode, modeConfig.ds_core, modeConfig.ivecal_mode, modeConfig.disc_mode,modeConfig.pkt_mode,
		       KeyInfo[i].DescramblerMode.ds_chn_tscfg.all);
 	       KeyInfo[i].DescramblerMode.pkt_mode = modeConfig.pkt_mode;
		if (modeConfig.ivecal_mode != 0) {
		    DMXDescramblerSetIvMode(i, Attr->ivMode);
		}
		

              KeyInfo[i].ChanCount = 0;

		if(MT_UNF_DMX_CA_NORMAL == Attr->enCaType)
		{
#ifdef CONFIG_MT_KEYTABLE
			KeyInfo[i].DescramblerMode.keyslot_tab.even_key_slot_index = evenKeySlot;
			KeyInfo[i].DescramblerMode.keyslot_tab.odd_key_slot_index = oddKeySlot;
#endif
		}

		DmxHalDescramblerSetMode(&(KeyInfo[i].DescramblerMode));
		*KeyId = i;
		ret = i;
		break;
	    }
	}
	up(&DmxDevOsi->lock_Key);
    } else {
	ret = MT_ERR_DMX_BUSY;
	MT_INFO_DEMUX("MT_ERR_DMX_BUSY !! \n");
    }

    return ret;
}


/***********************************************************************************
* Function      :   DMX_OsiNewDescrambler
* Description   :  apply a new descrambler
* Input         :
* Output        :  pKeyId
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiDescramblerCreate(mt_u32 *KeyId, const MT_UNF_DMX_DESCRAMBLER_ATTR_S *DescAttr)
{
    MT_UNF_DMX_DESCRAMBLER_ATTR_S Attr;
    mt_u32 evenKeySlotId = MT_KT_SLOT_ID_INVALID;
    mt_u32 oddKeySlotId = MT_KT_SLOT_ID_INVALID;
#ifdef CONFIG_MT_KEYTABLE
    mt_u32 tempKeySlotId = MT_KT_SLOT_ID_INVALID;
    MT_KT_KEY_ATTR_S key_attr = { 0 };
#endif
    mt_s32 ch=-1;

    MT_INFO_DEMUX("%s============start\n",__FUNCTION__);

#ifdef CONFIG_MT_KEYTABLE
    DMXDescramblerConfigKeyAttribute(DescAttr->enDescramblerType, &key_attr);
#endif

    if ((MT_UNF_DMX_CA_NORMAL != DescAttr->enCaType) && (MT_UNF_DMX_CA_ADVANCE != DescAttr->enCaType)) {
	MT_WARN_DEMUX("CaType=%d\n", DescAttr->enCaType);

	return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CA_NORMAL == DescAttr->enCaType)
    {
#ifdef CONFIG_MT_KEYTABLE
        void *p_priv = drv_kt_get_handle();
        if (key_attr.KEY_SIZE == MT_KT_SLOT_32B_SIZE)
        {
            MT_INFO_DEMUX("%s  ------------- keyslot  need even slot\n", __FUNCTION__);
            drv_kt_slot_request(p_priv,  &tempKeySlotId);
            if (tempKeySlotId%2 == 0)
            {
                evenKeySlotId = tempKeySlotId;
                tempKeySlotId = MT_KT_SLOT_ID_INVALID;
                drv_kt_slot_request(p_priv,  &tempKeySlotId);
                drv_kt_slot_request_multi(p_priv, 2, &oddKeySlotId);
            }
            else
            {
                drv_kt_slot_request_multi(p_priv, 2, &evenKeySlotId);
                drv_kt_slot_request_multi(p_priv, 2, &oddKeySlotId);
                drv_kt_slot_release(p_priv,tempKeySlotId);
            }
        }
        else
        {
            drv_kt_slot_request(p_priv, &evenKeySlotId);
            drv_kt_slot_request(p_priv, &oddKeySlotId);
        }


        if (evenKeySlotId == MT_KT_SLOT_ID_INVALID || oddKeySlotId == MT_KT_SLOT_ID_INVALID) {
            return MT_ERR_DMX_NOFREE_KEY;
        }
        MT_INFO_DEMUX("DMX_OsiDescramblerCreate    evenKeySlotId=%d,oddKeySlotId=%d\n", evenKeySlotId, oddKeySlotId);
#endif
    }
  
    Attr.enCaType = DescAttr->enCaType;
    Attr.enDescramblerType = DescAttr->enDescramblerType;
    Attr.enEntropyReduction = MT_UNF_DMX_CA_ENTROPY_REDUCTION_CLOSE;
    Attr.ivMode = DescAttr->ivMode;
    Attr.enDecOrEncMode = DescAttr->enDecOrEncMode;
    memcpy(&Attr.tsCfg, &DescAttr->tsCfg, sizeof(MT_UNF_DMX_DESCRAMBLER_TSCFG_S));
    
    if (MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2 == DescAttr->enDescramblerType || MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE == DescAttr->enDescramblerType) {
	if ((MT_UNF_DMX_CA_ENTROPY_REDUCTION_CLOSE != DescAttr->enEntropyReduction)
		&& (MT_UNF_DMX_CA_ENTROPY_REDUCTION_OPEN != DescAttr->enEntropyReduction)) {
	    MT_WARN_DEMUX("EntropyReduction=%d\n", DescAttr->enEntropyReduction);

	    //return MT_ERR_DMX_INVALID_PARA;
	}

	Attr.enEntropyReduction = DescAttr->enEntropyReduction;
    }
#ifdef DMX_DESCRAMBLER_VERSION_1
    MT_INFO_DEMUX("evenKeySlotId = %d, oddKeySlotId = %d\n",evenKeySlotId, oddKeySlotId);
    ch = DMXDescramblerCreate1(KeyId, &Attr, evenKeySlotId, oddKeySlotId);
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_INFO_DEMUX("DescAttr = %d, ch = %d, validate_tag = %d\n",DescAttr, ch, DescAttr->tsCfg.validate_tag);
   if(MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE == DescAttr->enDescramblerType)
   {
   	 if((ch >=0) && (ch < DMX_KEY_CNT))   reg_set_tsi_ds_chn_tscfg_pes_csa2_key_msb64(ch, MT_TRUE);
   }
#endif

   return  MT_SUCCESS ;
}

/***********************************************************************************
* Function      : DMXDescramblerCreate1
* Description   : create a new descrambler
* Input         :
* Output        : KeyId
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others:
***********************************************************************************/
static mt_s32 DMXDescramblerProCreate1(mt_u32 *KeyId, MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *Attr, mt_u32 evenKeySlot, mt_u32 oddKeySlot)
{
    mt_s32 ret = MT_ERR_DMX_NOFREE_KEY;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_KeyInfo_S *KeyInfo = DmxDevOsi->DmxKeyInfo;
    U_CA_INFO0 CaInfo;
    mt_u32 i;
    MT_DMX_OsiDescrambler_ModeConfig  modeConfig={0,};

    CaInfo.value = DmxHalGetOptCAType();
    MT_INFO_DEMUX("Attr->enDescramblerType = %d\n", Attr->enDescramblerType);
    modeConfig.ivecal_mode = Attr->ivMode;
    DMXDescramblerConfigMode(Attr->enDescramblerType,  &modeConfig);
    MT_INFO_DEMUX("%s >>> line=%d\n",__FUNCTION__,__LINE__);
#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    if(Attr->enDecOrEncMode == MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN)
    {
        modeConfig.disc_mode = modeConfig.disc_mode & 0x9f;   //  enable enc bit5 = 0, bit6=0;
    }
#endif //defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
    if (0 == down_interruptible(&DmxDevOsi->lock_Key)) {
	for (i = 0; i < DMX_KEY_CNT; i++)  {
	    if (MT_UNF_DMX_CA_BUTT != KeyInfo[i].CaType) {
		continue;
	    }

	    if (MT_SUCCESS == DescCreate(i, Attr->enCaType, Attr->enDescramblerType, Attr->enEntropyReduction)) {
		KeyInfo[i].CaType = Attr->enCaType;
		KeyInfo[i].CaEntropy = Attr->enEntropyReduction;
		KeyInfo[i].DescType = Attr->enDescramblerType;
		KeyInfo[i].KeyLen = modeConfig.KeyLen;

		KeyInfo[i].DescramblerMode.ds_even_push_en = 1;
		KeyInfo[i].DescramblerMode.ds_odd_push_en = 1;
		KeyInfo[i].DescramblerMode.ds_clr_en = 0;

		if(Attr->enTsDescFlag == MT_UNF_DMX_DESCRAMBLER_DONOT_DELETE_DESC_TAG)
			KeyInfo[i].DescramblerMode.ds_srctag_clr = 0;
		else
			KeyInfo[i].DescramblerMode.ds_srctag_clr = 1;
		
		KeyInfo[i].DescramblerMode.ds_cw_ch = i;
		KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = 0;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = 1;
#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	       MT_INFO_DEMUX("symphony1 do not support enc !\n");
#else
		switch(Attr->enDecOrEncMode)
		{
			case MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN:
			{
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 1;  //enable enc
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = Attr->encMode.encOddOrEven;  //enable odd key
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = Attr->encMode.encForce;   //onoff ebcForce
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = Attr->encMode.encTsScrClrRange;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = Attr->encMode.encScrTagClr;  //onoff modify tag
				break;
			}
			case MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_ODD:
			{
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 1;  //enable enc
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = 1;  //enable odd key
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = Attr->encMode.encForce;   //onoff ebcForce
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = Attr->encMode.encTsScrClrRange;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = Attr->encMode.encScrTagClr;  //onoff modify tag
				break;
			}
			case MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN_EVEN:
			{
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 1;  //enable enc
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = 0;  //enable even key
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = Attr->encMode.encForce;   //onoff ebcForce
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = Attr->encMode.encTsScrClrRange;  //add modify range
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = Attr->encMode.encScrTagClr;  //onoff modify tag
				break;
			}
			default:
			{
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco = 0;
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.enc_odd_even_eco = 0;  //even
				KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.scr_enc_force = 0;
				break;
			}
		}
#endif
              switch (Attr->tsCfg.tscfgDsMode)
              {
                    case MT_UNF_DMX_CA_AUTO_DS_MODE:
                        modeConfig.ds_mode = 2;
                        break;
                    case MT_UNF_DMX_CA_FORCE_TS_LAYER:
                        modeConfig.ds_mode = 0;
                        break;
                    case MT_UNF_DMX_CA_FORCE_PES_LAYER:
                        modeConfig.ds_mode = 1;
                        break;
                   case MT_UNF_DMX_CA_BUTT_MODE:
                        modeConfig.ds_mode = 3;
                        break;
                    default:
                        modeConfig.ds_mode = 2;
                        break;
              }

              MT_INFO_DEMUX("ds_cw_ch = %d, ds_mode=%#x, ds_chn_tscfg=%#x, Attr->tsCfg.tscfgCwopt1Mode =%#x, Attr->tsCfg.tscfgDsMode =%#x  Attr->tsCfg.gost_sbox_sel = %#x\n",
		       i, KeyInfo[i].DescramblerMode.ds_mode, KeyInfo[i].DescramblerMode.ds_chn_tscfg, Attr->tsCfg.tscfgCwopt1Mode, Attr->tsCfg.tscfgDsMode, Attr->tsCfg.gost_sbox_sel);
		KeyInfo[i].DescramblerMode.ds_mode = modeConfig.ds_mode;
		KeyInfo[i].DescramblerMode.ds_core = modeConfig.ds_core;
		KeyInfo[i].DescramblerMode.ivecal_mode = modeConfig.ivecal_mode;
		KeyInfo[i].DescramblerMode.disc_mode = modeConfig.disc_mode;
		KeyInfo[i].DescramblerMode.ades_disc_mode.bitc.disc_mode_ctr = modeConfig.disc_mode_ctr;
#if defined(CONFIG_MT_CHIP_SYMPHONY6)     
              MT_INFO_DEMUX("Attr->tsCfg.tscfgCwopt1Mode=%d\n",Attr->tsCfg.tscfgCwopt1Mode);
              switch (Attr->tsCfg.tscfgCwopt1Mode)
              {
                    case MT_UNF_DMX_ACTIVE_PES_LOWBIT_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 1;
                        break;
                    case MT_UNF_DMX_ACTIVE_PES_NO_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
                        break;
                    case MT_UNF_DMX_INACTIVE_PES_SCB_NO_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 1;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
                        break;
                   case MT_UNF_DMX_INACTIVE_PES_LOWBIT_DSC:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 1;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 1;
                        break;
                    default:
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
                        KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
                        break;
              }
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.gost_sbox_sel = Attr->tsCfg.gost_sbox_sel;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.gost_bit_inv = Attr->tsCfg.gost_bit_inv;
              KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.multi2_key_msb64 = Attr->tsCfg.multi2_key_msb64;
		MT_INFO_DEMUX("KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode=%d\n",KeyInfo[i].DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode);	  
              if(modeConfig.disc_mode & 0x40000000){
                   KeyInfo[i].DescramblerMode.ades_disc_mode.bitc.dis_mode_b8 = 1;
              }else{
                   KeyInfo[i].DescramblerMode.ades_disc_mode.bitc.dis_mode_b8 = 0;
              }
#endif              
		MT_INFO_DEMUX("ds_cw_ch = %d, ds_mode=%x, ds_core=%x,ivecal_mode=%x, disc_mode=%x, pkt_mode=0x%x\n",
		       i, modeConfig.ds_mode, modeConfig.ds_core, modeConfig.ivecal_mode, modeConfig.disc_mode,modeConfig.pkt_mode);
 	       KeyInfo[i].DescramblerMode.pkt_mode = modeConfig.pkt_mode;
		if (modeConfig.ivecal_mode != 0) {
		    DMXDescramblerSetIvMode(i, Attr->ivMode);
		}

              KeyInfo[i].ChanCount = 0;

		if(MT_UNF_DMX_CA_NORMAL == Attr->enCaType)
		{
#ifdef CONFIG_MT_KEYTABLE
			KeyInfo[i].DescramblerMode.keyslot_tab.even_key_slot_index = evenKeySlot;
			KeyInfo[i].DescramblerMode.keyslot_tab.odd_key_slot_index = oddKeySlot;
#endif
		}
              MT_INFO_DEMUX("ds_cw_ch = %d, ds_mode=%x, ds_chn_tscfg=%xs\n",
		       i, KeyInfo[i].DescramblerMode.ds_mode, KeyInfo[i].DescramblerMode.ds_chn_tscfg);
		DmxHalDescramblerSetMode(&(KeyInfo[i].DescramblerMode));
		*KeyId = i;
		ret = i;

		break;
	    }
	}
	up(&DmxDevOsi->lock_Key);
    } else {
	ret = MT_ERR_DMX_BUSY;
	MT_INFO_DEMUX("MT_ERR_DMX_BUSY !! \n");
    }

    return ret;
}

/***********************************************************************************
* Function      :   DMX_OsiNewDescrambler
* Description   :  apply a new descrambler
* Input         :
* Output        :  pKeyId
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiDescramblerProCreate(mt_u32 *KeyId, const MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S *DescAttr)
{
    MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S Attr={0,};
    mt_u32 evenKeySlotId = MT_KT_SLOT_ID_INVALID;
    mt_u32 oddKeySlotId = MT_KT_SLOT_ID_INVALID;
    mt_s32 ch=-1;
    MT_DMX_OsiDescrambler_ModeConfig  modeConfig={0,};
#ifdef CONFIG_MT_KEYTABLE
    mt_u32 tempKeySlotId = MT_KT_SLOT_ID_INVALID;
    MT_KT_KEY_ATTR_S key_attr = { 0 };
#endif
    memset(&Attr, 0, sizeof(MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S));
    DMXDescramblerConfigMode(DescAttr->enDescramblerType,  &modeConfig);
#ifdef CONFIG_MT_KEYTABLE
    DMXDescramblerConfigKeyAttribute(DescAttr->enDescramblerType, &key_attr);
#endif

    MT_INFO_DEMUX("%s============start\n",__FUNCTION__);
    if ((MT_UNF_DMX_CA_NORMAL != DescAttr->enCaType) && (MT_UNF_DMX_CA_ADVANCE != DescAttr->enCaType)) {
	MT_WARN_DEMUX("CaType=%d\n", DescAttr->enCaType);

	return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CA_NORMAL == DescAttr->enCaType)
    {
#ifdef CONFIG_MT_KEYTABLE
        void *p_priv = drv_kt_get_handle();
        MT_INFO_DEMUX("%s   DescAttr->enDescramblerType = %d\n", __FUNCTION__,   DescAttr->enDescramblerType);
        if (key_attr.KEY_SIZE == MT_KT_SLOT_32B_SIZE)
        {
            MT_INFO_DEMUX("%s  ------------- keyslot  need even slot\n", __FUNCTION__);
            drv_kt_slot_request(p_priv,  &tempKeySlotId);
            if (tempKeySlotId%2 == 0)
            {
                evenKeySlotId = tempKeySlotId;
                tempKeySlotId = MT_KT_SLOT_ID_INVALID;
                drv_kt_slot_request(p_priv,  &tempKeySlotId);
                drv_kt_slot_request_multi(p_priv, 2, &oddKeySlotId);
            }
            else
            {
                drv_kt_slot_request_multi(p_priv, 2, &evenKeySlotId);
                drv_kt_slot_request_multi(p_priv, 2, &oddKeySlotId);
                drv_kt_slot_release(p_priv,tempKeySlotId);
            }
        }
        else
        {
            MT_INFO_DEMUX("%s  ------------- normal\n", __FUNCTION__);
            drv_kt_slot_request(p_priv, &evenKeySlotId);
            drv_kt_slot_request(p_priv, &oddKeySlotId);
        }

        if (evenKeySlotId == MT_KT_SLOT_ID_INVALID || oddKeySlotId == MT_KT_SLOT_ID_INVALID) {
            return MT_ERR_DMX_NOFREE_KEY;
        }
        MT_INFO_DEMUX("%s     evenKeySlotId=%d,oddKeySlotId=%d\n", __FUNCTION__, evenKeySlotId, oddKeySlotId);
#endif
    }

    Attr.enCaType = DescAttr->enCaType;
    Attr.enDescramblerType = DescAttr->enDescramblerType;
    Attr.enEntropyReduction = MT_UNF_DMX_CA_ENTROPY_REDUCTION_CLOSE;
    Attr.ivMode = DescAttr->ivMode;
    Attr.enDecOrEncMode = DescAttr->enDecOrEncMode;
    Attr.tsCfg.tscfgDsMode = DescAttr->tsCfg.tscfgDsMode;
    Attr.tsCfg.tscfgCwopt1Mode = DescAttr->tsCfg.tscfgCwopt1Mode;
    memcpy(&Attr.tsCfg, &DescAttr->tsCfg, sizeof(MT_UNF_DMX_DESCRAMBLER_TSCFG_S));
    memcpy(&Attr.encMode, &DescAttr->encMode, sizeof(MT_UNF_DMX_DESCRAMBLER_ENC_ATTR_S));

    if (MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2 == DescAttr->enDescramblerType || MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE == DescAttr->enDescramblerType) {
	if ((MT_UNF_DMX_CA_ENTROPY_REDUCTION_CLOSE != DescAttr->enEntropyReduction)
		&& (MT_UNF_DMX_CA_ENTROPY_REDUCTION_OPEN != DescAttr->enEntropyReduction)) {
	    MT_WARN_DEMUX("EntropyReduction=%d\n", DescAttr->enEntropyReduction);

	    //return MT_ERR_DMX_INVALID_PARA;
	}

	Attr.enEntropyReduction = DescAttr->enEntropyReduction;
    }
    /**/

#ifdef DMX_DESCRAMBLER_VERSION_1
    MT_INFO_DEMUX("%s  evenKeySlotId = %d, oddKeySlotId = %d\n", __FUNCTION__, evenKeySlotId, oddKeySlotId);
    ch = DMXDescramblerProCreate1(KeyId, &Attr, evenKeySlotId, oddKeySlotId);
#endif
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_INFO_DEMUX("DescAttr = %d, ch = %d, validate_tag = %d\n",DescAttr, ch, DescAttr->tsCfg.validate_tag);
   if(MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE == DescAttr->enDescramblerType)
   {
	 Attr.tsCfg.csa2_key_msb64 = MT_TRUE;
   }
   if(DescAttr && ((ch >=0) && (ch < DMX_KEY_CNT)))
   {
    	memcpy(&Attr, DescAttr, sizeof(MT_UNF_DMX_DESCRAMBLER_PRO_ATTR_S));
       DmxHalDescramblerSetModeForSym6(ch, &Attr);
   }
#endif
    return  MT_SUCCESS ;
}


/***********************************************************************************
* Function      : DMX_OsiDescramblerDestroy
* Description   : destroy a Descrambler
* Input         : KeyId
* Output        :
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others:
***********************************************************************************/
mt_s32 DMX_OsiDescramblerDestroy(mt_u32 KeyId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_KeyInfo_S *KeyInfo = &DmxDevOsi->DmxKeyInfo[KeyId];
    DMX_ChanInfo_S *ChanInfo = DmxDevOsi->DmxChanInfo;
    mt_u32 i;

    if (MT_UNF_DMX_CA_BUTT != KeyInfo->CaType) {
	for (i = 0; i < DMX_CHANNEL_CNT; i++) {
	    if (ChanInfo[i].KeyId == KeyId) {
		DmxHalSetChannelCWIndex(ChanInfo[i].ChanId, 0);

		DmxHalSetChannelDsc(ChanInfo[i].ChanId, MT_FALSE);
	    }
	}

	ret = down_interruptible(&DmxDevOsi->lock_Key);

	DescramblerReset(KeyId, KeyInfo);

	up(&DmxDevOsi->lock_Key);

	ret = MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiDescramblerGetAttr(mt_u32 KeyId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDescramblerAttr)
{
    DMX_KeyInfo_S *KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[KeyId];

    pstDescramblerAttr->enCaType = KeyInfo->CaType;
    pstDescramblerAttr->enDescramblerType = KeyInfo->DescType;
    pstDescramblerAttr->enEntropyReduction = KeyInfo->CaEntropy;
    pstDescramblerAttr->ivMode = KeyInfo->DescramblerMode.ivecal_mode;

    return MT_SUCCESS;
}

mt_s32 DMX_OsiDescramblerSetAttr(mt_u32 KeyId, MT_UNF_DMX_DESCRAMBLER_ATTR_S *pstDescramblerAttr)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_KeyInfo_S *KeyInfo = NULL;
    MT_DMX_OsiDescrambler_ModeConfig  modeConfig={0,};

    if((DMX_KEY_CNT <= KeyId) ||( KeyId < 0))  return  MT_ERR_DMX_INVALID_PARA;
    if(pstDescramblerAttr == NULL)    return  MT_ERR_DMX_INVALID_PARA;
    KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[KeyId];

    if (KeyInfo->DescType != pstDescramblerAttr->enDescramblerType)
    {
        modeConfig.ivecal_mode = pstDescramblerAttr->ivMode;
	DMXDescramblerConfigMode(pstDescramblerAttr->enDescramblerType,  &modeConfig);
#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	if(pstDescramblerAttr->enDecOrEncMode == MT_UNF_DMX_CA_KEY_ATTR_ENC_OPEN)
	{
		modeConfig.disc_mode = modeConfig.disc_mode & 0x9f;   //  enable enc bit5 = 0, bit6=0;
	}
#endif //defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4)
	KeyInfo->KeyLen = modeConfig.KeyLen;

        KeyInfo->DescramblerMode.ds_even_push_en = 1;
        KeyInfo->DescramblerMode.ds_odd_push_en = 1;
        KeyInfo->DescramblerMode.ds_clr_en = 0;
	 if(pstDescramblerAttr->enTsDescFlag == MT_UNF_DMX_DESCRAMBLER_DONOT_DELETE_DESC_TAG)
		KeyInfo->DescramblerMode.ds_srctag_clr = 0;
	 else
		KeyInfo->DescramblerMode.ds_srctag_clr = 1;
        KeyInfo->DescramblerMode.ds_mode = modeConfig.ds_mode; //tscfg.bitc.ds_mode
        KeyInfo->DescramblerMode.ds_core = modeConfig.ds_core;
        KeyInfo->DescramblerMode.ivecal_mode = modeConfig.ivecal_mode;
        KeyInfo->DescramblerMode.disc_mode = modeConfig.disc_mode;
 	KeyInfo->DescramblerMode.ades_disc_mode.bitc.disc_mode_ctr = modeConfig.disc_mode_ctr;

        MT_INFO_DEMUX("ds_cw_ch = %d, ds_mode=%x, ds_core=%x,ivecal_mode=%x, disc_mode=%x\n",
                KeyInfo->DescramblerMode.ds_cw_ch, modeConfig.ds_mode, modeConfig.ds_core, modeConfig.ivecal_mode, modeConfig.disc_mode);
        KeyInfo->DescramblerMode.pkt_mode = modeConfig.pkt_mode;
        if (modeConfig.ivecal_mode != 0) {
            //KeyInfo->DescramblerMode.pkt_mode = 0x20; //~ive
            DMXDescramblerSetIvMode(KeyInfo->DescramblerMode.ds_cw_ch, pstDescramblerAttr->ivMode);
        }
	 if (MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042 == pstDescramblerAttr->enDescramblerType
                || MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_DVS042 == pstDescramblerAttr->enDescramblerType)
        {
            KeyInfo->DescramblerMode.pkt_mode = 0x10; //dvs042 mode
        } else {
            KeyInfo->DescramblerMode.pkt_mode = modeConfig.pkt_mode;
        }

	 KeyInfo->DescramblerMode.ds_chn_tscfg.bitc.pes_cwopt1_mode = 0;
        KeyInfo->DescramblerMode.ds_chn_tscfg.bitc.ts_cwopt1_mode = 0;
        KeyInfo->DescramblerMode.ds_chn_tscfg.bitc.tsscr_clr_range = 0;
        KeyInfo->DescramblerMode.ds_chn_tscfg.bitc.scrtag_clr = 1;
	 MT_INFO_DEMUX("%s : %d >>>>>KeyInfo->DescramblerMode.pkt_mode=0x%x\n",__FUNCTION__,__LINE__,KeyInfo->DescramblerMode.pkt_mode);
        DmxHalDescramblerSetMode(&(KeyInfo->DescramblerMode));
   }
    /* New CA type become effective after set new CW key  */
    KeyInfo->CaType = pstDescramblerAttr->enCaType;
    KeyInfo->DescramblerMode.ivecal_mode = pstDescramblerAttr->ivMode;

    ret = MT_SUCCESS;

    return ret;
}

/**
*  for  CA_ADVANCE   only
*/
mt_s32 DMX_OsiDescramblerSetKeySlot(mt_u32 KeyId, DMX_KEY_TYPE_E KeyType, mt_u8 KeySlot)
{
#ifdef CONFIG_MT_KEYTABLE
	reg_tsi_tsi_keyslot_tab_t reg_keyslot_tab;
	DMX_KeyInfo_S *KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[KeyId];

	if (KeySlot == MT_KT_SLOT_ID_INVALID) {
		return MT_ERR_DMX_INVALID_PARA;
	}

	if (MT_UNF_DMX_CA_ADVANCE != KeyInfo->CaType) {
	    return MT_ERR_DMX_INVALID_PARA;
	}
	if (DMX_KEY_TYPE_EVEN == KeyType) {
	    if (KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index != (mt_u32)KeySlot) {
		KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index = (mt_u32)KeySlot;
	    }
	}

	if (DMX_KEY_TYPE_ODD == KeyType) {
	    if (KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index != (mt_u32)KeySlot) {
		KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index = (mt_u32)KeySlot;
	    }
	}
	KeyInfo->DescramblerMode.keyslot_tab.entry_valid = 1;
	MT_INFO_DEMUX("evenkey_slotid=%d, oddkey_slot_id=%d\n",
	       KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index,
	       KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index);

	memset(&reg_keyslot_tab, 0, sizeof(reg_tsi_tsi_keyslot_tab_t));
	reg_keyslot_tab.bitc.entry_valid = KeyInfo->DescramblerMode.keyslot_tab.entry_valid;
	reg_keyslot_tab.bitc.even_key_slot_index = KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index;
	reg_keyslot_tab.bitc.odd_key_slot_index = KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index;

	reg_set_tsi_keyslot_tab(KeyId, reg_keyslot_tab.all);
#endif

    return MT_SUCCESS;
}

mt_s32 DMX_OsiDescramblerSetKey(mt_u32 KeyId, DMX_KEY_TYPE_E KeyType, mt_u8 *Key)
{
    DMX_KeyInfo_S *KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[KeyId];
    mt_u32 i;
    //DRV_ADVCA_EXTFUNC_PARAM_S stAdvcaFuncParam = {0};
    MT_INFO_DEMUX("%s ======KeyType=%d,  KeyLen = %u\n", __FUNCTION__, KeyType, KeyInfo->KeyLen);
    if (MT_UNF_DMX_CA_BUTT == KeyInfo->CaType) {
	return MT_ERR_DMX_INVALID_PARA;
    }
    if (MT_UNF_DMX_CA_ADVANCE == KeyInfo->CaType) {
	return MT_ERR_DMX_INVALID_PARA;
    }
#ifdef MT_DEMUX_PROC_SUPPORT
    for (i = 0; i < KeyInfo->KeyLen; i++) {
	if (DMX_KEY_TYPE_EVEN == KeyType) {
	    KeyInfo->EvenKey[i] = ((mt_u32 *)Key)[i];
	} else {
	    KeyInfo->OddKey[i] = ((mt_u32 *)Key)[i];
	}
    }
#endif

    if (MT_UNF_DMX_CA_NORMAL == KeyInfo->CaType) {

#ifdef CONFIG_MT_KEYTABLE
    void *p_priv = drv_kt_get_handle();
    mt_u32 key[8] = { 0 };
    mt_u32 keySlotId = MT_KT_SLOT_ID_INVALID;
    MT_KT_KEY_ATTR_S attr = { 0 };
    MT_KT_KEY_ATTR_S attr1 = { 0 };

    if (DMXDescramblerConfigKeyAttribute(KeyInfo->DescType, &attr) != MT_SUCCESS) {
        MT_INFO_DEMUX("DescType error !!  DescType=%d\n", KeyInfo->DescType);
        return MT_ERR_DMX_INVALID_PARA;
    }
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    if((KeyInfo->DescramblerMode.ds_chn_tscfg.bitc.enc_mode_eco == 0x01)  && (KeyInfo->DescramblerMode.ds_core == 1))
    {
		attr.ENC_ONOFF  = MT_KT_ATTR_ON;
    		attr.DEC_ONOFF = MT_KT_ATTR_OFF;
    }
#else //CONFIG_MT_CHIP_SYMPHONY6
    /*  enable enc bit5 = 0, bit6=0;  aes des tdes  */
    if(((KeyInfo->DescramblerMode.disc_mode & 0x60) == 0x00)  && (KeyInfo->DescramblerMode.ds_core == 1))
    {
		attr.ENC_ONOFF  = MT_KT_ATTR_ON;
    		attr.DEC_ONOFF = MT_KT_ATTR_OFF;
    }
#endif  //CONFIG_MT_CHIP_SYMPHONY6
    memcpy(key, Key, attr.KEY_SIZE);

    if (KeyType == DMX_KEY_TYPE_EVEN) {
        keySlotId = KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index;
    } else if (KeyType == DMX_KEY_TYPE_ODD) {
        keySlotId = KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index;
    } else {
        MT_INFO_DEMUX("KeyType error !!  KeyType=%d\n", KeyType);
        return MT_ERR_DMX_INVALID_PARA;
    }

    drv_kt_slot_active(p_priv, keySlotId, MT_KT_SLOT_DO_INACTIVE);
    drv_kt_write_attribute(p_priv, keySlotId, attr);
    drv_kt_read_attribute(p_priv, keySlotId, &attr1);
    drv_kt_write_key(p_priv, keySlotId, (mt_u8 *)key, attr.KEY_SIZE);
    drv_kt_slot_active(p_priv, keySlotId, MT_KT_SLOT_DO_ACTIVE);
    if(1)
    {
    	    /*
    	    mt_u8 key_out[16];   //dump key
           drv_kt_read_key(p_priv, keySlotId, (mt_u8 *)key_out, 16);
           MT_INFO_DEMUX("KeyId=%x, size = %d, Key(%x %x %x %x %x %x %x %x)(%x %x %x %x %x %x %x %x)\n",
	           keySlotId, attr.KEY_SIZE,
	           key_out[0], key_out[1], key_out[2], key_out[3], key_out[4], key_out[5], key_out[6], key_out[7]
		   ,key_out[8], key_out[9], key_out[10], key_out[11], key_out[12], key_out[13], key_out[14], key_out[15]);
	    */
	    MT_INFO_DEMUX("kt attr AES_ONOFF:0x%x,DES_ONOFF:0x%x,TDES_ONOFF:0x%x,CSAv2_ONOFF:0x%x,CSAv3_ONOFF:0x%x,SM2_3_4_ONOFF:0x%x,ASA_ONOFF:0x%x,TS_ONOFF:0x%x,M2M_ONOFF:0x%x\n",
		 attr1.AES_ONOFF,attr1.DES_ONOFF,attr1.TDES_ONOFF,attr1.CSAv2_ONOFF,attr1.CSAv3_ONOFF,attr1.SM2_3_4_ONOFF,attr1.ASA_ONOFF,attr1.TS_ONOFF,attr1.M2M_ONOFF);
           MT_INFO_DEMUX("kt attr1 GOST_28147_89_OR_R34_12_MAGMA_ONOFF:0x%x, GOST_R34_12_KUZNYECHIK_ONOFF:0x%x,\n",
		     attr1.GOST_28147_89_OR_R34_12_MAGMA_ONOFF, attr1.GOST_R34_12_KUZNYECHIK_ONOFF);
	    MT_INFO_DEMUX("kt attr1 MAC_ONOFF:0x%x,Multi2_ONOFF:0x%x,REE_ONOFF:0x%x,DEC_ONOFF:0x%x,ENC_ONOFF:0x%x,KEY_SIZE:0x%x,IV_SIZE:0x%x,KEY_SOURCE:0x%x,TDES_KEYCHK:0x%x\n",
		 attr1.MAC_ONOFF,attr1.Multi2_ONOFF,attr1.REE_ONOFF,attr1.DEC_ONOFF,attr1.ENC_ONOFF,attr1.KEY_SIZE,attr1.IV_SIZE,attr1.KEY_SOURCE,attr1.TDES_KEYCHK);
    }
    MT_INFO_DEMUX("KeyId=%x, size = %d, Key(%x %x %x %x %x %x %x %x)\n",
           keySlotId, attr.KEY_SIZE,
           Key[0], Key[1], Key[2], Key[3], Key[4], Key[5], Key[6], Key[7]);
#else
    if (strlen(Key) <= 8)
        KeyInfo->KeyLen = 8 /4;
    else
        KeyInfo->KeyLen = strlen(Key) / 4;

    MT_INFO_DEMUX("KeyId=%x, KeyInfo->KeyLen = %d, strlen(Key)=%d\n", KeyId, KeyInfo->KeyLen, strlen(Key));

    if (KeyType == DMX_KEY_TYPE_EVEN) {
	DMX_OsiDescramblerSetEvenKeys(KeyId, Key, KeyInfo->KeyLen * sizeof(mt_u32));
    } else if (KeyType == DMX_KEY_TYPE_ODD) {
	DMX_OsiDescramblerSetOddKeys(KeyId, Key, KeyInfo->KeyLen * sizeof(mt_u32));
    } else {
	MT_INFO_DEMUX("KeyType error !!  KeyType=%d\n", KeyType);
    }
#endif

    } else {
	return MT_ERR_DMX_INVALID_PARA;
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiDescramblerSetIVKey(mt_u32 KeyId, DMX_KEY_TYPE_E KeyType, mt_u8 *Key)
{
    DMX_KeyInfo_S *KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[KeyId];
#ifdef CONFIG_MT_KEYTABLE
    void *p_priv = drv_kt_get_handle();
    MT_KT_SLOT_SIZE_E size;
#else
    mt_u32 i;
#endif

    if (MT_UNF_DMX_CA_BUTT == KeyInfo->CaType) {
	return MT_ERR_DMX_INVALID_PARA;
    }
    if (MT_UNF_DMX_CA_ADVANCE == KeyInfo->CaType) {
	return MT_ERR_DMX_INVALID_PARA;
    }

#ifdef CONFIG_MT_KEYTABLE
    switch (KeyInfo->DescType) {
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_TS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_PES:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2_HIGH8BYTE:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_CLR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_XOR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_DES_SCTE52:
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR:
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_H:
    case MT_UNF_DMX_DESCRAMBLE_DESC_DES_CTR_L:
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR:
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_H:
    case MT_UNF_DMX_DESCRAMBLE_DESC_3DES_CTR_L:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_CTS:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_HEAD_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_TAIL_CLEAR:
    case MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_DVS042:
	 size = MT_KT_SLOT_8B_SIZE;
        break;

    default:
        size = MT_KT_SLOT_16B_SIZE;
        break;
    }
    MT_INFO_DEMUX("KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index=%d, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index=%d\n",
		KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index,KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index);
    MT_INFO_DEMUX("vikey : 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",Key[0],Key[1],Key[2],Key[3],Key[4],Key[5],Key[6],Key[7]);
    if (KeyType == DMX_KEY_TYPE_EVEN) {
        drv_kt_write_iv(p_priv, KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index, Key, size);
    }
    if (KeyType == DMX_KEY_TYPE_ODD) {
        drv_kt_write_iv(p_priv, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index, Key, size);
    }
#else
    if (MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CI == KeyInfo->DescType) //change iv cw order, AES CI+ request
    {
        mt_u32 len = KeyInfo->KeyLen * sizeof(mt_u32);
        mt_u8 tmp[DMX_KEY_MAX_LEN];

        memcpy(tmp, Key, len);

        for (i = 0; i < len; i++) {
            Key[i] = tmp[len - i - 1];
        }
    }

    if (KeyType == DMX_KEY_TYPE_EVEN) {
        DMX_OsiDescramblerSetEvenKeys(KeyId, Key, KeyInfo->KeyLen * sizeof(mt_u32));
    } else if (KeyType == DMX_KEY_TYPE_ODD) {
        DMX_OsiDescramblerSetOddKeys(KeyId, Key, KeyInfo->KeyLen * sizeof(mt_u32));
    } else {
        MT_INFO_DEMUX("KeyType error !!  KeyType=%d\n", KeyType);
    }
#endif

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiDescramblerAttach
* Description   : attcg  Descrambler to channel
* Input         : KeyId, ChanId
* Output        :
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others        :
***********************************************************************************/
mt_s32 DMX_OsiDescramblerAttach(mt_u32 KeyId, mt_u32 ChanId)
{
    mt_s32 ret;
    int i = 0;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxDevOsi->DmxChanInfo[ChanId];
    DMX_KeyInfo_S *KeyInfo = &DmxDevOsi->DmxKeyInfo[KeyId];
#ifdef CONFIG_MT_KEYTABLE
    reg_tsi_tsi_keyslot_tab_t reg_keyslot_tab;
#endif

    if ((MT_UNF_DMX_CA_BUTT == KeyInfo->CaType) || (DMX_INVALID_CHAN_ID == ChanInfo->ChanId)) {
	return MT_ERR_DMX_INVALID_PARA;
    }

    if (DMX_INVALID_KEY_ID != ChanInfo->KeyId) {
	return MT_ERR_DMX_ATTACHED_KEY;
    }

    ret = down_interruptible(&DmxDevOsi->lock_Key);
    ChanInfo->KeyId = KeyId;
#ifdef MT_DEMUX_PROC_SUPPORT
   ++ KeyInfo->ChanCount;
#endif
    up(&DmxDevOsi->lock_Key);

    MT_INFO_DEMUX("ChanInfo->ChanId = 0x%x, keyId=0x%x\n", ChanInfo->ChanId, KeyId);

    for(i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if (DmxDevOsi->DmxChanInfo[i].DmxId < DMX_CNT
			&& ChanId != i
			&& ChanInfo->ChanPid == DmxDevOsi->DmxChanInfo[i].ChanPid
			&& ChanInfo->slot_reg0.bitc.src == DmxDevOsi->DmxChanInfo[i].slot_reg0.bitc.src
			&& DMX_INVALID_KEY_ID != DmxDevOsi->DmxChanInfo[i].KeyId)
            break;
    }
    if (i >= DMX_CHANNEL_CNT)
    {
    	ChanInfo->slot_reg1.bitc.descrambler_en = 1;
    }
    else
    {
        ChanInfo->slot_reg1.bitc.descrambler_en = 0;
        MT_INFO_DEMUX("[%s] =========>same pid \n", __FUNCTION__);
        
    }
    
    ChanInfo->slot_reg1.bitc.cw_ch_0_3 = KeyId & 0xf;
    ChanInfo->slot_reg1.bitc.cw_ch_4 = (KeyId >> 4) & 0x1;
    reg_set_demux_slotn_cfg1(ChanInfo->ChanId, ChanInfo->slot_reg1.all);
    /*
    reg_set_demux_slotn_cfg1_cw_ch(ChanInfo->ChanId,  KeyId);
    ChanInfo->slot_reg1.bitc.descrambler_en = 1;
    reg_set_demux_slotn_cfg1_descrambler_en(ChanInfo->ChanId,  1);
    */
    ChanInfo->KeyId = KeyId;

    if (KeyInfo->ChanCount > 1)
		return MT_SUCCESS;

#ifdef CONFIG_MT_KEYTABLE
    KeyInfo->DescramblerMode.keyslot_tab.entry_valid = 1;
    MT_INFO_DEMUX("evenkey_slotid=%d, oddkey_slot_id=%d\n",
       KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index,
       KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index);

    memset(&reg_keyslot_tab, 0, sizeof(reg_tsi_tsi_keyslot_tab_t));
    reg_keyslot_tab.bitc.entry_valid = KeyInfo->DescramblerMode.keyslot_tab.entry_valid;
    reg_keyslot_tab.bitc.even_key_slot_index = KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index;
    reg_keyslot_tab.bitc.odd_key_slot_index = KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index;

    reg_set_tsi_keyslot_tab(KeyId, reg_keyslot_tab.all);
    MT_INFO_DEMUX("DMX_OsiDescramblerAttach  >>>  ChanInfo->ChanId = 0x%x,  KeyId=0x%x\n",
        ChanInfo->ChanId, KeyId);
#endif

    //if (KeyInfo->DescramblerMode.ivecal_mode > 0) {
	//DMXDescramblerSetIvMode(KeyId, KeyInfo->DescramblerMode.ivecal_mode);
    //}

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiDescramblerDetach
* Description   : dettach Descrambler from a channel
* Input         : KeyId, ChanId
* Output        :
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others        :
***********************************************************************************/
mt_s32 DMX_OsiDescramblerDetach(mt_u32 KeyId, mt_u32 ChanId)
{
    mt_s32 ret;
    int i = 0;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxDevOsi->DmxChanInfo[ChanId];
    DMX_KeyInfo_S *KeyInfo = &DmxDevOsi->DmxKeyInfo[KeyId];

    if (MT_UNF_DMX_CA_BUTT == KeyInfo->CaType) {
	MT_WARN_DEMUX("key %d has not been create yet\n", KeyId);

	return MT_ERR_DMX_INVALID_PARA;
    }

    if (DMX_INVALID_KEY_ID == ChanInfo->KeyId) {
	MT_WARN_DEMUX("Channel %d has not attached any descrambler\n", ChanId);

	return MT_ERR_DMX_NOATTACH_KEY;
    }

    if (ChanInfo->KeyId != KeyId) {
	MT_WARN_DEMUX("Detach Wrong Key from channel %d\n", ChanId);

	return MT_ERR_DMX_UNMATCH_KEY;
    }

    ret = down_interruptible(&DmxDevOsi->lock_Key);
    ChanInfo->KeyId = DMX_INVALID_KEY_ID;

#ifdef MT_DEMUX_PROC_SUPPORT
    --KeyInfo->ChanCount;
#endif

    up(&DmxDevOsi->lock_Key);
#if 0
    DmxHalSetChannelCWIndex(ChanId, 0);

#ifdef DMX_DESCRAMBLER_VERSION_1
    DmxHalSetChanCwTabId(ChanId, 0);
#endif

    DmxHalSetChannelDsc(ChanId, MT_FALSE);
#else
    ChanInfo->slot_reg1.bitc.cw_ch_0_3 = 0;
    ChanInfo->slot_reg1.bitc.cw_ch_4 = 0;
    reg_set_demux_slotn_cfg1_cw_ch(ChanInfo->ChanId, 0);

    for(i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if (DmxDevOsi->DmxChanInfo[i].DmxId < DMX_CNT  
			&& ChanId != i
			&& ChanInfo->ChanPid == DmxDevOsi->DmxChanInfo[i].ChanPid
			&& ChanInfo->slot_reg0.bitc.src == DmxDevOsi->DmxChanInfo[i].slot_reg0.bitc.src
			&& DMX_INVALID_KEY_ID != DmxDevOsi->DmxChanInfo[i].KeyId)
            break;
    }
    if (i >= DMX_CHANNEL_CNT)
    {
        ChanInfo->slot_reg1.bitc.descrambler_en = 0;
        reg_set_demux_slotn_cfg1_descrambler_en(ChanInfo->ChanId, 0);
        MT_INFO_DEMUX("[%s]  ==========>descrambler_en = %d\n", __FUNCTION__, ChanInfo->slot_reg1.bitc.descrambler_en);
    }
    else
    {
        MT_INFO_DEMUX("[%s] =========>same pid \n", __FUNCTION__);
        DmxDevOsi->DmxChanInfo[i].slot_reg1.bitc.descrambler_en = 1;
        reg_set_demux_slotn_cfg1_descrambler_en(DmxDevOsi->DmxChanInfo[i].ChanId, 1);
        MT_INFO_DEMUX("[%s]  ==========>ChanId = %d, PID = %d, descrambler_en = %d\n", __FUNCTION__,
                DmxDevOsi->DmxChanInfo[i].ChanId,
                DmxDevOsi->DmxChanInfo[i].ChanPid, 
                DmxDevOsi->DmxChanInfo[i].slot_reg1.bitc.descrambler_en);
        
        ChanInfo->slot_reg1.bitc.descrambler_en = 0;
        reg_set_demux_slotn_cfg1_descrambler_en(ChanInfo->ChanId, 0);
        MT_INFO_DEMUX("[%s]  ==========>ChanId = %d, PID = %d, descrambler_en = %d\n", __FUNCTION__,
                ChanInfo->ChanId,
                ChanInfo->ChanPid, 
                ChanInfo->slot_reg1.bitc.descrambler_en);
    }

    
	MT_INFO_DEMUX("[%s]  >>>  even_key_slot_index = %d,  odd_key_slot_index=%d\n",
        __FUNCTION__, KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index, KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index);
    //KeyInfo->DescramblerMode.keyslot_tab.even_key_slot_index = 0;
    //KeyInfo->DescramblerMode.keyslot_tab.odd_key_slot_index = 0;
#ifdef CONFIG_MT_KEYTABLE
    if(KeyInfo->ChanCount == 0)
    {
    	 reg_set_tsi_keyslot_tab(KeyId, 0);
    }
    MT_INFO_DEMUX("DMX_OsiDescramblerDetach  >>>  ChanInfo->ChanId = 0x%x,  KeyId=0x%x\n",
        ChanInfo->ChanId, KeyId);
#endif

#endif
    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiDescramblerGetFreeKeyNum
* Description   : get free Descramber Num
* Input         :
* Output        : FreeCount
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others        :
***********************************************************************************/
mt_s32 DMX_OsiDescramblerGetFreeKeyNum(mt_u32 *FreeCount)
{
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_KeyInfo_S *KeyInfo = DmxDevOsi->DmxKeyInfo;
    mt_u32 i;

    *FreeCount = 0;

    if (0 == down_interruptible(&DmxDevOsi->lock_Key)) {
	for (i = 0; i < DMX_KEY_CNT; i++) {
	    if (MT_UNF_DMX_CA_BUTT == KeyInfo[i].CaType) {
		++(*FreeCount);
	    }
	}
	up(&DmxDevOsi->lock_Key);

	return MT_SUCCESS;
    }

    return MT_ERR_DMX_BUSY;
}

/***********************************************************************************
* Function      : DMX_OsiDescramblerGetKeyId
* Description   : get key
* Input         : ChanId
* Output        : KeyId
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others        :
***********************************************************************************/
mt_s32 DMX_OsiDescramblerGetKeyId(mt_u32 ChanId, mt_u32 *KeyId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxDevOsi->DmxChanInfo[ChanId];

    if (DMX_INVALID_CHAN_ID == ChanInfo->ChanId) {
	return ret;
    }

    ret = MT_ERR_DMX_NOATTACH_KEY;

    if (0 == down_interruptible(&DmxDevOsi->lock_Key)) {
	if (ChanInfo->KeyId < DMX_KEY_CNT) {
	    *KeyId = ChanInfo->KeyId;

	    ret = MT_SUCCESS;
	}

	up(&DmxDevOsi->lock_Key);
    } else {
	ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}



#ifdef MT_DEMUX_PROC_SUPPORT
DMX_KeyInfo_S *DMX_OsiGetKeyProc(mt_u32 KeyId)
{
    DMX_KeyInfo_S *KeyInfo;

    KeyInfo = &g_pDmxDevOsi->DmxKeyInfo[KeyId];
    if (MT_UNF_DMX_CA_BUTT == KeyInfo->CaType) {
	KeyInfo = MT_NULL;
    }

    return KeyInfo;
}
#endif
