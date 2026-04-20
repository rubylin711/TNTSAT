/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#include <stdio.h>
#include <pthread.h>
#include "mt_common.h"
#include "mt_sec_ext.h"

#include "mt_unf_cipher_v2.h"
#include "mt_unf_demux.h"
#include "mt_unf_misc.h"
#if 0
#define MT_SEC_EMSG(fmt, args...)   printf("MT_SEC err [%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)
#define MT_SEC_DMSG(fmt, args...)   printf("MT_SEC infor[%s:%u]"fmt, __FUNCTION__, __LINE__, ##args)
#else
#define MT_SEC_EMSG(fmt, args...)
#define MT_SEC_DMSG(fmt, args...)
#endif

#ifndef MT_SEC_OTT_SMP_PART
#define MT_SEC_OTT_SMP_PART "ott0"
#endif

static pthread_mutex_t mt_sec_mutex = PTHREAD_MUTEX_INITIALIZER;
#define mtSecLock() (void) pthread_mutex_lock(&mt_sec_mutex);
#define mtSecUnlock() (void) pthread_mutex_unlock(&mt_sec_mutex);

/**For the limitation of DMX_REC_CNT, there are not really DMX ID, but a map ID */
/**CNcomment:限定录制使用的DMX映射号码*/

/*
 * Convert tsid into dmxid: 1,2,3,4 --> 0,1,2,3
 *   NOTE:This is device dependant. If 'tsid' is defined by Application, please modify it accordingly.
 */
/*#define TSID_TO_DMXID(x)	(x % MAX_DMX_ID)*/

enum PlatDmxId {
	PLAT_DMX_ID_0,
	PLAT_DMX_ID_1,
	PLAT_DMX_ID_2,
	PLAT_DMX_ID_3,
	PLAT_DMX_ID_4,
	PLAT_DMX_ID_5,
	PLAT_DMX_ID_MAX,
};

#define MAX_DMX_ID	(PLAT_DMX_ID_MAX)//(4) 最大可以8


/*
 * Convert TSID to platform supported DemuxId
 */
struct TsidPair {
	mt_u32 id_plat;
	mt_u32 id_nv;
	mt_u32 paired;
};

#define TSID_INVALID	(0xFF)

static struct TsidPair gTsidPair[MAX_DMX_ID] = {
	{PLAT_DMX_ID_0, TSID_INVALID, 0},
	{PLAT_DMX_ID_1, TSID_INVALID, 0},
	{PLAT_DMX_ID_2, TSID_INVALID, 0},
	{PLAT_DMX_ID_3, TSID_INVALID, 0},
       {PLAT_DMX_ID_4, TSID_INVALID, 0},
       {PLAT_DMX_ID_5, TSID_INVALID, 0},
	/*Add here if has more IDs*/
};




#if 0
mt_u32 mtSecGetPlatDmxId(mt_u32 tsid)
{
	mt_u32 id_plat = MAX_DMX_ID;
	mt_u32 i = 0;

	mtSecLock();

	for (i = 0; i < MAX_DMX_ID; i++) {
		if ((tsid == gTsidPair[i].id_nv) && (1 == gTsidPair[i].paired) ) {
			/*get the already paired id*/
			id_plat = gTsidPair[i].id_plat;
			break;
		}
	}

	/*not paired yet, pair it with a new id*/
	if (i == MAX_DMX_ID) {
		if (tsid < MAX_DMX_ID) {
			gTsidPair[tsid].id_nv = tsid;
			gTsidPair[tsid].paired = 1;
			id_plat = gTsidPair[tsid].id_plat;
		} else {
			for (i = 0; i < MAX_DMX_ID; i++) {
				if (gTsidPair[i].paired == 0) {
					gTsidPair[i].id_nv = tsid;
					gTsidPair[i].paired = 1;
					id_plat = gTsidPair[i].id_plat;
					break;
				}
			}
		}
	}
	mtSecUnlock();

	if (id_plat == MAX_DMX_ID) {
		MT_SEC_DMSG("FATAL:get plat dmx id failed!");
	}

	MT_SEC_DMSG("<--tsid(%d) paired with platDmxId(%d)-->\n", tsid, id_plat);

	return id_plat;
}
#else
mt_u32 mtSecGetPlatDmxId(mt_u32 tsid)
{
	mt_u32 id_plat = MAX_DMX_ID;
	mt_u32 i = 0;

	MT_UNF_DMX_PORT_E PortId;
	mtSecLock();

	for (i = 0; i < MAX_DMX_ID; i++) {
		if ((tsid == gTsidPair[i].id_nv) && (1 == gTsidPair[i].paired) ) {
			/*get the already paired id*/
			id_plat = gTsidPair[i].id_plat;
			break;
		}
	}

	/*not paired yet, pair it with a new id*/
	if (i == MAX_DMX_ID) {
		for (i = 0; i < MAX_DMX_ID; i++) {
			if (gTsidPair[i].paired == 0 && MT_UNF_DMX_GetTSPortId(i, &PortId ) == MT_ERR_DMX_NOATTACH_PORT) {
				gTsidPair[i].id_nv = tsid;
				gTsidPair[i].paired = 1;
				id_plat = gTsidPair[i].id_plat;
				break;
			}
		}
	}
	mtSecUnlock();

	if (id_plat == MAX_DMX_ID) {
		//MT_SEC_DMSG("FATAL:get plat dmx id failed!");
	}

	//MT_SEC_DMSG("<--tsid(%d) paired with platDmxId(%d)-->\n", tsid, id_plat);

	return id_plat;
}
#if 1
mt_u32 mtSecSetPlatDmxId(mt_u32 tsid, mt_u8 platDmxId)
{
	mt_u32 id_plat = MAX_DMX_ID;

        mtSecLock();
	/*pair it with a new id*/
	if (platDmxId < MAX_DMX_ID) {
		gTsidPair[platDmxId].id_nv = tsid;
		gTsidPair[platDmxId].paired = 1;
		id_plat = gTsidPair[platDmxId].id_plat;
	}
	mtSecUnlock();

	return id_plat;
}
#endif
#endif

mt_u32 mtSecGetTsIdByDmxId(mt_u32 dmxid)
{
	mt_u32 tsid = TSID_INVALID;
	mt_u32 i = 0;

	mtSecLock();
	for (i = 0; i < MAX_DMX_ID; i++) {
		if ((dmxid == gTsidPair[i].id_plat) && (1 == gTsidPair[i].paired)) {
			/*get the already paired id*/
			tsid = gTsidPair[i].id_nv;
			break;
		}
	}
	mtSecUnlock();

	return tsid;
}

void mtSecClrPlatDmxId(mt_u32 tsid)
{
	mt_u32 i = 0;

	mtSecLock();

	for (i = 0; i < MAX_DMX_ID; i++) {
		if (gTsidPair[i].paired == 1) {
			if (tsid == gTsidPair[i].id_nv) {
				gTsidPair[i].id_nv = TSID_INVALID;
				gTsidPair[i].paired = 0;
				break;
			}
		}
	}

	mtSecUnlock();
}

/*
 * Media Buffer Info for storing buffers of media path.
 */
static MBInfo gMBInfo[MAX_MB_COUNT] = {0};
static RecChanWithDmxIDInfo gRecChanDmxID[MAX_DMX_REC_CHAN] = {0};

TBoolean mtSecSetProtectBuffer(phys_addr_t addrPhy, size_t size, MBType type)
{
	int i = 0;
	//MT_SEC_DMSG("mtSecSetProtectBuffer  addrPhy = %x size = %x type = %d \n",  addrPhy, size, type);
       mtSecLock();
	//check if addr already registered
	for (i = 0; i < MAX_MB_COUNT; i++) {
		if (gMBInfo[i].start == addrPhy) {
			mtSecUnlock();
			//MT_SEC_DMSG("check if addr already registered i =%d  \n",  i);

			return TRUE;
		}
	}

	//find a free slot to add
	for (i = 0; i < MAX_MB_COUNT; i++) {
		if (gMBInfo[i].start == 0) {

			gMBInfo[i].start = addrPhy;
			gMBInfo[i].size = size;
			gMBInfo[i].type = type;
			//MT_SEC_DMSG("find a free slot to add i =%d  \n",  i);

			mtSecUnlock();
			return TRUE;
		}
	}

	mtSecUnlock();
	return FALSE;
}


TBoolean mtSecGetProtectBufferByType(MBType type, MBInfo *mb, mt_u8 clr)
{
	int i = 0;

    mtSecLock();
	//check if already registered
	for (i = 0; i < MAX_MB_COUNT; i++) {
		//MT_SEC_DMSG("gMBInfo[%d].type =%d   gMBInfo[i].start = %d  \n", i, gMBInfo[i].type,  gMBInfo[i].start );
		if (gMBInfo[i].type == type && gMBInfo[i].start != 0) {
			mb->start = gMBInfo[i].start;
			mb->size = gMBInfo[i].size;
			mb->type = gMBInfo[i].type;
			if (clr) {
				//mark as retrieved
				gMBInfo[i].start = 0;
				gMBInfo[i].type = 0;
				gMBInfo[i].size = 0;
			}
			mtSecUnlock();
			return TRUE;
		}
	}

	mtSecUnlock();

	//MT_SEC_DMSG("mtSecGetProtectBufferByType return false \n");
	return FALSE;
}

TBoolean mtSecResetProtectBufferList(void)
{
	int i = 0;

    mtSecLock();

	//MT_SEC_DMSG("mtSecResetProtectBufferList  \n");

	for (i = 0; i < MAX_MB_COUNT; i++) {
		gMBInfo[i].start = 0;
		gMBInfo[i].size = 0;
		gMBInfo[i].type = 0;
	}

	mtSecUnlock();
	return TRUE;
}


TBoolean mtSecGetRecChanByDmxID(mt_u32 dmxPort, mt_u8 *p_ch, mt_u8 clr)
{
	int i = 0;

    mtSecLock();
	//check if already registered
	for (i = 0; i < MAX_DMX_REC_CHAN; i++) {

		if (gRecChanDmxID[i].dmxPortID == dmxPort && gRecChanDmxID[i].valid) {
			*p_ch = gRecChanDmxID[i].recChan;
                    MT_SEC_DMSG("%d %d  %d \n", i, gRecChanDmxID[i].dmxPortID,  gRecChanDmxID[i].recChan, gRecChanDmxID[i].valid);
			if (clr) {
				//mark as retrieved
				gRecChanDmxID[i].dmxPortID = 0;
				gRecChanDmxID[i].recChan = 0;
				gRecChanDmxID[i].valid = FALSE;
			}
			mtSecUnlock();
			return TRUE;
		}
	}
	mtSecUnlock();
	//MT_SEC_DMSG(" return false \n");
	return FALSE;
}

TBoolean mtSecSetRecChanByDmxID(mt_u32 dmxPort, mt_u8 ch)
{
	int i = 0;
	TBoolean ret = FALSE;
	MT_SEC_DMSG(" dmxPort =%x, ch = %d \n",  dmxPort, ch);
       mtSecLock();
	//check if addr already registered
	for (i = 0; i < MAX_DMX_REC_CHAN; i++) {
        MT_SEC_DMSG("%d %d  %d  %d \n", i, gRecChanDmxID[i].dmxPortID,  gRecChanDmxID[i].recChan, gRecChanDmxID[i].valid);
		if (gRecChanDmxID[i].dmxPortID == dmxPort) {
			gRecChanDmxID[i].recChan = ch;
			gRecChanDmxID[i].valid = TRUE;
                    MT_SEC_DMSG("%d %d  %d  %d \n", i, gRecChanDmxID[i].dmxPortID,  gRecChanDmxID[i].recChan, gRecChanDmxID[i].valid);
			ret = TRUE;
			goto opend;
		}
	}

	//find a free slot to add
	for (i = 0; i < MAX_DMX_REC_CHAN; i++) {
        MT_SEC_DMSG("%d %d  %d  %d \n", i, gRecChanDmxID[i].dmxPortID,  gRecChanDmxID[i].recChan, gRecChanDmxID[i].valid);
		if (gRecChanDmxID[i].valid == FALSE) {
			gRecChanDmxID[i].dmxPortID = dmxPort;
			gRecChanDmxID[i].recChan = ch;
			gRecChanDmxID[i].valid = TRUE;
			MT_SEC_DMSG("%d %d  %d  %d \n", i, gRecChanDmxID[i].dmxPortID,  gRecChanDmxID[i].recChan, gRecChanDmxID[i].valid);
			ret = TRUE;
			goto opend;
		}
	}
opend:
	mtSecUnlock();
	return ret;
}
#define MT_OP_REE_FLAG_LO   (0xbf448090)
#define MT_OP_REE_FLAG_HI   (0xbf448094)

TBoolean mtSecGetOpREEFlag(MTSecOpREEFlag *p_opcStatus)
{
    mt_u32 data = 0;
    uint64_t opc_status = 0;

    if (p_opcStatus) {
        memset(p_opcStatus, 0x00, sizeof(MTSecOpREEFlag));
    } else {
        return FALSE;
    }
    mt_sys_read_register(MT_OP_REE_FLAG_LO, &data);
    opc_status = data;
    mt_sys_read_register(MT_OP_REE_FLAG_HI, &data);
    opc_status = opc_status | ((uint64_t)data << 32);
    if (opc_status) {
        //printf("MT_SEC infor[%s:%d] opc_status = 0x%p", __FUNCTION__, __LINE__, opc_status);
        if ((opc_status & (1 << 6)))
        {
            p_opcStatus->violateHDCP1 = 1;
        }
        if ((opc_status & (1 << 7)))
        {
            p_opcStatus->violateHDCP2 = 1;
        }
        if ((opc_status & (1 << 8)))
        {
            p_opcStatus->violate4K = 1;
        }
        if ((opc_status & (1 << 9)))
        {
            p_opcStatus->violate2K = 1;
        }
        if ((opc_status & (1 << 10)))
        {
            p_opcStatus->violateHD = 1;
        }
        if ((opc_status & (1 << 11)))
        {
            p_opcStatus->violateSD= 1;
        }
        p_opcStatus->isViolation = TRUE;
        return TRUE;
    } else {
        return FALSE;
    }
}

#define MT_SEC_CKL_REG  (0xbf50c000)
void mtSecRelatedModuleClkEnable(void)
{
    mt_u32 ret = 0;
    mt_u32 data = 0;
    //ret = mt_sys_init();
    ret |= mt_unf_misc_init();
    ret |= mt_unf_cipher_init();
    ret |= mt_unf_misc_module_set(HAL_KT, 1);
    ret |= mt_unf_misc_module_set(HAL_CRYPTO, 1);
    //ret |= mt_unf_misc_module_set(HAL_CRYPTO_DES, 1);
    ret |= mt_unf_misc_module_set(HAL_CRYPTO_TDES, 1);
    ret |= mt_unf_misc_module_set(HAL_CRYPTO_AES, 1);
    ret |= mt_unf_misc_module_set(HAL_CRYPTO_SHA, 1);
    ret |= mt_unf_misc_module_set(HAL_CRYPTO_RSA, 1);
    ret |= mt_unf_misc_module_set(HAL_KL_CW, 1);
    ret |= mt_unf_misc_module_set(HAL_SECHD0, 1); // enable cert clock!
    //printf("mt_cert_init ret 0x%x \n", ret);

    mt_sys_read_register(MT_SEC_CKL_REG, &data);
    //printf("value 0x%x \n", data);
    data |= (1 << 3); //enable cert clock!
    mt_sys_write_register(MT_SEC_CKL_REG, data);
    mt_sys_read_register(MT_SEC_CKL_REG, &data);
	//printf("value == 0x%x \n", data);

}

TBoolean mtSecResetRecChanDmxIDList(void)
{
    int i = 0;

    mtSecLock();
    for (i = 0; i < MAX_DMX_REC_CHAN; i++) {
        gRecChanDmxID[i].dmxPortID = 0;
        gRecChanDmxID[i].recChan = 0;
        gRecChanDmxID[i].valid = FALSE;
    }
    mtSecUnlock();
    return TRUE;
}

void *mtSec_OTTAllocateSMPMemory(TUnsignedInt32 size, MBType mbType)
{
    void *vir_addr = NULL;
    phys_addr_t phy_addr;

    if (size == 0)
        return NULL;
    if (mbType == MB_OTT) {
        phy_addr = (mt_u32)mt_mmz_new(size, SMP_DDR_ALIGN_UNIT, MT_SEC_OTT_SMP_PART, "ott_smp");
    } else
    {
        MT_SEC_EMSG("mb type not support 0x%x", mbType);
        return NULL;
    }
    if (phy_addr == 0) {
		MT_SEC_EMSG("new mmz size:%lu failed", size);
        return NULL;
    }
	//DMSG("New PhyAddr:%x,size:%x", phy_addr, size);

    //map,but not cached
    vir_addr = mt_mmz_map(phy_addr, 0);
    if (vir_addr == NULL) {
		MT_SEC_EMSG("map mmz phy:%llu failed", phy_addr);
        mt_mmz_delete(phy_addr);
        return NULL;
    }

    //MT_SEC_DMSG("PhyAddr->VirAddr:%p, phy_addr = 0x%p", vir_addr, phy_addr);
    /* success,return the virtual address of the buffer */
    return vir_addr;
}

mt_s32 mtSec_OTTFreeSMPMemory(void *p_ottSmpAddr)
{
    mt_s32 ret;
    phys_addr_t phy_addr;
    ulong phy_size;

    if (p_ottSmpAddr == NULL) {
        MT_SEC_EMSG("Can not free NULL pointer\n");
        return -1;
    }

    ret = mt_mmz_get_phyaddr(p_ottSmpAddr, &phy_addr, &phy_size);
    ret |= mt_mmz_unmap((void *)phy_addr);
    ret |= mt_mmz_delete(phy_addr);

    return ret;
}




//#####################################
#if 1

#include <tee_client_api.h>

#include "smpc_api.h"
#include "mt_unf_cipher_v2.h"

static TEEC_Context ext_mtlz_teec_ctx;
static TEEC_Session ext_mtlz_sess = {0,};

#define TA_EXT_MTLZ_UUID \
	{	0x64eed1a2, 0xde8d, 0xff79, \
		{0x0e, 0xc7, 0xdb, 0x50, 0xce, 0x36, 0x32, 0xbf} }


/* The function IDs implemented in this TA */
enum {
    TA_EXT_MTLZ_CMD_TEST = 0,
    TA_EXT_MTLZ_CMD_SMP_PRE_ENABLE,
    TA_EXT_MTLZ_CMD_MAX,
};


#define MT_SEC_EXT_SMP_ON


#define EXTMTLZ_KEYSLOT_NUM     (4)

#if 0
#define EXT_MTLZ_MSG(fmt, ...)  printf("[mtlz]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define EXT_MTLZ_MSG(fmt, ...)
#endif

//0 is main Vdec, 1 is PIP vdec
mt_u32 ext_mtlz_smp_keyslot[EXTMTLZ_KEYSLOT_NUM] = {MT_CIPHER_KEYSLOT_INVALID, MT_CIPHER_KEYSLOT_INVALID, MT_CIPHER_KEYSLOT_INVALID, MT_CIPHER_KEYSLOT_INVALID};

mt_u32 mtExtMTLZ_TEECInit(void)
{
    TEEC_Result res = TEEC_SUCCESS;
    uint32_t err_origin;
    TEEC_UUID uuid = TA_EXT_MTLZ_UUID;
    TEEC_Operation op;
    //mt_u8 i = 0;

#ifdef MT_SEC_EXT_SMP_ON
    res = TEEC_InitializeContext(NULL, &ext_mtlz_teec_ctx);
    if (res != TEEC_SUCCESS) {
        EXT_MTLZ_MSG("Init tee context failed\n");
        goto end;
    }
    EXT_MTLZ_MSG("%s, %d, ctx = 0x%x\n", __FUNCTION__, __LINE__, ext_mtlz_sess.ctx);
    res = TEEC_OpenSession(&ext_mtlz_teec_ctx, &ext_mtlz_sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS) {
        EXT_MTLZ_MSG("TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
        goto end;
    }
    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /*
    * Prepare the argument. Pass a value in the first parameter,
    * the remaining three parameters are unused.
    */
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
    	 TEEC_NONE, TEEC_NONE);
    op.params[0].value.a = 42;


    EXT_MTLZ_MSG("Invoking TA to increment %d\n", op.params[0].value.a);
    res = TEEC_InvokeCommand(&ext_mtlz_sess, TA_EXT_MTLZ_CMD_TEST, &op, &err_origin);
    if (res != TEEC_SUCCESS) {
        EXT_MTLZ_MSG("TEEC_InvokeCommand failed with code 0x%x origin 0x%x", res, err_origin);
        goto end;
    }

    EXT_MTLZ_MSG("TA incremented value to %d, ctx = 0x%x\n", op.params[0].value.a, ext_mtlz_sess.ctx);


    if (ext_mtlz_smp_keyslot[0] >= MT_CIPHER_KEYSLOT_INVALID)
        res = mt_unf_cipher_keyslot_request_multi(EXTMTLZ_KEYSLOT_NUM, ext_mtlz_smp_keyslot);

    EXT_MTLZ_MSG("keyslot %d, %d, %d, %d \n", ext_mtlz_smp_keyslot[0], ext_mtlz_smp_keyslot[1], ext_mtlz_smp_keyslot[2], ext_mtlz_smp_keyslot[3]);
end:
#endif
    return res;
}

mt_u32 mtExtMTLZ_TEECDeinit(void)
{
    mt_u8 i = 0;
    TEEC_Result res = TEEC_SUCCESS;

#ifdef MT_SEC_EXT_SMP_ON
    TEEC_CloseSession(&ext_mtlz_sess);
    TEEC_FinalizeContext(&ext_mtlz_teec_ctx);
    ext_mtlz_smp_keyslot[0] = MT_CIPHER_KEYSLOT_INVALID;
    ext_mtlz_smp_keyslot[1] = MT_CIPHER_KEYSLOT_INVALID;
    for (i = 0; i < EXTMTLZ_KEYSLOT_NUM; i ++) {
        if (ext_mtlz_smp_keyslot[i] < MT_CIPHER_KEYSLOT_INVALID)
                mt_unf_cipher_keyslot_release(ext_mtlz_smp_keyslot[i]);
    }
#endif
    return res;
}


mt_u32 mtExtMTLZ_TEE_SMP_En(bool isMianPlay, MTPlayType mt_play)
{
    TEEC_Result res = TEEC_SUCCESS;
    uint32_t err_origin;
    TEEC_Operation op;
    SMPC_SlotType soltType;
    SMPC_MemType memTypeVedio = -1, memTypeAudio = -1, memTypeOtt = -1;
    mt_u32 keyslot = MT_CIPHER_KEYSLOT_INVALID, keyslotAud = MT_CIPHER_KEYSLOT_INVALID;
    MBInfo mb = {0};
    //MBType mbType = 0;
#ifdef MT_SEC_EXT_SMP_ON
    if (isMianPlay) {
        keyslot = ext_mtlz_smp_keyslot[0];
        keyslotAud = ext_mtlz_smp_keyslot[1];
        memTypeVedio = SMPC_MEM_TYPE_VIDEO;
        EXT_MTLZ_MSG("\n");
        if (mt_play == MT_DVB_PLAY) {
            soltType = SMPC_SLOT_TYPE_VIDEO;
            memTypeAudio = SMPC_MEM_TYPE_AUDIO;
            EXT_MTLZ_MSG("\n");
         } else if (mt_play == MT_OTT_PLAY) {
            soltType = SMPC_SLOT_TYPE_OTT;
            memTypeAudio = SMPC_MEM_TYPE_AUDIO;
            memTypeOtt = SMPC_MEM_TYPE_OTT;
            EXT_MTLZ_MSG("\n");
         } else {
            res = TEEC_ERROR_BAD_PARAMETERS;
            EXT_MTLZ_MSG("\n");
            goto end;
        }
    } else {
        keyslot = ext_mtlz_smp_keyslot[2];
        memTypeVedio = SMPC_MEM_TYPE_SUB_VIDEO;
        EXT_MTLZ_MSG("\n");
        if (mt_play == MT_DVB_PLAY) {
            soltType = SMPC_SLOT_TYPE_SUB_VIDEO;
            EXT_MTLZ_MSG("\n");
        } else if (mt_play == MT_OTT_PLAY) {
            soltType = SMPC_SLOT_TYPE_SUB_OTT;
            memTypeOtt = SMPC_MEM_TYPE_OTT;
            EXT_MTLZ_MSG("\n");
        } else {
            res = TEEC_ERROR_BAD_PARAMETERS;
            EXT_MTLZ_MSG("\n");
            goto end;
        }
    }

    if (keyslot >= MT_CIPHER_KEYSLOT_INVALID) {
        res = TEEC_ERROR_BAD_PARAMETERS;
        goto end;
    }

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /*
     * Prepare the argument. Pass a value in the first parameter,
     * the remaining three parameters are unused.
     */
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_NONE,
    				 TEEC_NONE, TEEC_NONE);

    res = SMPC_Open(keyslot, soltType);
    EXT_MTLZ_MSG("res = 0x%x memTypeVedio = 0x%x \n", res, memTypeVedio);

    if ((memTypeVedio == SMPC_MEM_TYPE_VIDEO) || (memTypeVedio == SMPC_MEM_TYPE_SUB_VIDEO)) {
        if (memTypeVedio == SMPC_MEM_TYPE_VIDEO) {
            if (!mtSecGetProtectBufferByType(MB_VID, &mb, 0)) {
                res = TEEC_ERROR_NOT_SUPPORTED;
                goto end;
            }
        } else {
            if (!mtSecGetProtectBufferByType(MB_SUB_VID, &mb, 0)) {
                res = TEEC_ERROR_NOT_SUPPORTED;
                goto end;
            }
        }
        res = SMPC_RegisterMemory(keyslot, memTypeVedio, mb.start, mb.size);
        EXT_MTLZ_MSG("res = 0x%x, kl = %d, 0x%x, 0x%x \n", res, keyslot, mb.start, mb.size);
    }
    if (memTypeAudio == SMPC_MEM_TYPE_AUDIO) {
        res = SMPC_Open(keyslotAud, SMPC_SLOT_TYPE_AUDIO);
        EXT_MTLZ_MSG("res = 0x%x memTypeAudio = 0x%x \n", res, memTypeAudio);
        if (!mtSecGetProtectBufferByType(MB_AUD, &mb, 0)) {
            res = TEEC_ERROR_NOT_SUPPORTED;
            goto end;
        }
        res = SMPC_RegisterMemory(keyslotAud, memTypeAudio, mb.start, mb.size);
        EXT_MTLZ_MSG("res = 0x%x, keyslotAud = %d, 0x%x, 0x%x \n", res, keyslotAud, mb.start, mb.size);
    }
    if (memTypeOtt == SMPC_MEM_TYPE_OTT) {
        if (!mtSecGetProtectBufferByType(MB_OTT, &mb, 0)) {
            res = TEEC_ERROR_NOT_SUPPORTED;
            goto end;
        }
        res = SMPC_RegisterMemory(keyslot, memTypeOtt, mb.start, mb.size);
        EXT_MTLZ_MSG("res = 0x%x, kl = %d, 0x%x, 0x%x \n", res, keyslot, mb.start, mb.size);
    }

    op.params[0].value.a = keyslot;

    EXT_MTLZ_MSG("Invoking TA to  %d, kl = 0x%x, 0x%x, 0x%x \n", op.params[0].value.a, keyslot, mb.start, mb.size);
    res = TEEC_InvokeCommand(&ext_mtlz_sess, TA_EXT_MTLZ_CMD_SMP_PRE_ENABLE, &op,
    			 &err_origin);
    if (res != TEEC_SUCCESS) {
    	EXT_MTLZ_MSG("TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
    		res, err_origin);
            goto end;
    }
    EXT_MTLZ_MSG("TA incremented value to %d, ctx = 0x%x\n", op.params[0].value.a, ext_mtlz_sess.ctx);

    if (memTypeAudio == SMPC_MEM_TYPE_AUDIO) {
        op.params[0].value.a = keyslotAud;

        EXT_MTLZ_MSG("Invoking TA to  %d, kl = 0x%x, 0x%x, 0x%x \n", op.params[0].value.a, keyslot, mb.start, mb.size);
        res = TEEC_InvokeCommand(&ext_mtlz_sess, TA_EXT_MTLZ_CMD_SMP_PRE_ENABLE, &op,
        			 &err_origin);
        if (res != TEEC_SUCCESS) {
        	EXT_MTLZ_MSG("TEEC_InvokeCommand failed with code 0x%x origin 0x%x",
        		res, err_origin);
                goto end;
        }
        EXT_MTLZ_MSG("TA incremented value to %d, ctx = 0x%x\n", op.params[0].value.a, ext_mtlz_sess.ctx);
    }
    return TEEC_SUCCESS;
end:
    EXT_MTLZ_MSG("res = 0x%x error  0x%x,  0x%x,  0x%x \n", res, memTypeVedio, memTypeAudio, memTypeOtt);
#endif
    return res;
}

mt_u32 mtExtMTLZ_TEE_SMP_Dis(bool isMianPlay)
{
    TEEC_Result res = TEEC_SUCCESS;

#ifdef MT_SEC_EXT_SMP_ON
    mt_u32 keyslot = MT_CIPHER_KEYSLOT_INVALID;
    mt_u32 aud_keyslot = MT_CIPHER_KEYSLOT_INVALID;

    if (isMianPlay) {
        keyslot = ext_mtlz_smp_keyslot[0];
        aud_keyslot = ext_mtlz_smp_keyslot[1];

        if (aud_keyslot < MT_CIPHER_KEYSLOT_INVALID) {
            res = SMPC_Close(aud_keyslot);
            EXT_MTLZ_MSG("%s, %d res = 0x%x\n", __FUNCTION__, __LINE__, res);
            if (res != TEEC_SUCCESS) {
                EXT_MTLZ_MSG("%s, %d res = 0x%x\n", __FUNCTION__, __LINE__, res);
                goto end;
            }
        }
    } else {
        keyslot = ext_mtlz_smp_keyslot[2];
    }

    if (keyslot >= MT_CIPHER_KEYSLOT_INVALID) {
        res = TEEC_ERROR_ACCESS_DENIED;
        goto end;
    }

    res = SMPC_Close(keyslot);
    EXT_MTLZ_MSG("%s, %d res = 0x%x\n", __FUNCTION__, __LINE__, res);
    if (res != TEEC_SUCCESS) {
        EXT_MTLZ_MSG("%s, %d res = 0x%x\n", __FUNCTION__, __LINE__, res);
        goto end;
    }
end:
#endif
    return res;
}

#endif



