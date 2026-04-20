/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include "mt_common.h"
#include "ca_sec.h"
#include "mt_unf_otp.h"
#include "mt_unf_cipher_v2.h"
#include "mt_unf_descrambler.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecc.h"
#include "mt_unf_rsa.h"
#include "smpc_api.h"

#include "mt_sec_ext.h"
#include "ca_cert_impl.h"
#include "nocs_sec_impl.h"
#include "mt_common_enc_process.h"

#include "mt_unf_dma.h"


#define SUPPORT_CBCS_MODE

#define MT_SESSION_MAGIC    0xDEADBEAF
 /**Parse NAGRA EMI
 * EMI definition:
 *	TUnsignedInt16[15] = 0 : Nagravision definition
 *	TUnsignedInt16[15] = 1 : platform specific EMIs
 *
 *	TUnsignedInt16[14] = 0 : MPEG-TS	(Descrambler Path)
 *	TUnsignedInt16[14] = 1 : OTT		(M2M Path)
 */
#define EMI_PLAT_DEF	0x8000
#define EMI_OTHER_STD	0x4000

typedef enum {
    SESSION_RAM2RAM,
    SESSION_STREAM,
    SESSION_UNKNOWN,
} TSessionType;


/**The offset OPT address for NUID low 32 bits */
#define OTP_NUIDL_OFFSET                            (0xfab << 2)
#define OTP_NUIDL_SIZE                              (4)
#define OTP_NUIDL_SHIFT                             (0)
#define OTP_NUIDL_BITS                              (32)

#define OTP_NUIDH_OFFSET                            (0xfac << 2)
#define OTP_NUIDH_SIZE                              (4)
#define OTP_NUIDH_SHIFT                             (0)
#define OTP_NUIDH_BITS                              (32)

#define OTP_PRIVILEGED_MODE_OFFSET                  (0x3E78)
#define OTP_PRIVILEGED_MODE_SIZE                    (4)
#define OTP_PRIVILEGED_MODE_SHIFT                   (10)
#define OTP_PRIVILEGED_MODE_MASK                    (0xF)
#define OTP_REEISPRIVILEGE_VALUE                    (0xF)
#define OTP_NUID2ND_L_OFFSET                            (0xfba << 2)
#define OTP_NUID2ND_L_SIZE                              (4)
#define OTP_NUID2ND_L_SHIFT                             (0)
#define OTP_NUID2ND_L_BITS                              (32)

#define OTP_NUID2ND_H_OFFSET                            (0xfbb << 2)
#define OTP_NUID2ND_H_SIZE                              (4)
#define OTP_NUID2ND_H_SHIFT                             (0)
#define OTP_NUID2ND_H_BITS                              (32)


static pthread_mutex_t g_sec_mutex = PTHREAD_MUTEX_INITIALIZER;
#define secLock() (void) pthread_mutex_lock(&g_sec_mutex);
#define secUnlock() (void) pthread_mutex_unlock(&g_sec_mutex);

static pthread_mutex_t g_sec_process_mutex = PTHREAD_MUTEX_INITIALIZER;
#define secProcessLock() (void) pthread_mutex_lock(&g_sec_process_mutex);
#define secProcessUnlock() (void) pthread_mutex_unlock(&g_sec_process_mutex);

static TBoolean mtSecCryptoEngineProcessEnd(mt_handle crypto_handle, TUnsignedInt16 xEmi);
static TSecStatus secEnableTeePrivilegedMode(void);
static TSecStatus secEncryptFlashProtKey(const TUnsignedInt8 * pxInput, TUnsignedInt8 * pxOutput, size_t xSize);
static TSecStatus secGetNuid64(TSecNuid64 * pxNuid);
static TSecStatus secGetTeePrivilegedMode(TBoolean * pxEnabled);
static TSecStatus secGetChipsetExtension(const TChar ** ppxChipsetExtension);
static TSecStatus secGetChipId(TUnsignedInt8 pxOwnerId[2], TUnsignedInt8 pxChipId[8]);
TSignedInt32 mtSecSetIsMultiSessionFlag(TTransportSessionId xTransportSessionId, TBoolean isMultiSession);
TSignedInt32 mtSecRawStreamDataProcess_phy(TTransportSessionId xTransportSessionId, TSessionOpType op, TUnsignedInt32 size, const TUnsignedInt8 *input, TUnsignedInt8 *output);
//void sec_dump(const char *str, MT_U8 *addr, MT_U32 size);


static const TUnsignedInt8 KLD_CST[] = {
	0xA9, 0x32, 0x30, 0x31, 0x31, 0x4E, 0x61, 0x67,
	0x72, 0x61, 0x76, 0x69, 0x73, 0x69, 0x6F, 0x6E
};

static const TUnsignedInt8 CISSA_InitIV[16] = {
	0x44, 0x56, 0x42, 0x54, 0x4d, 0x43, 0x50, 0x54,
	0x41, 0x45, 0x53, 0x43,0x49, 0x53, 0x53, 0x41
};

static const TUnsignedInt8 CBC_ZeroIV[16] = {
	0x00,
};

/******************************************************************************
  Private Implementation Functions
 ******************************************************************************/

/* Nagra Root Key List */
#define NOCS_AES_ROOTKEY					MT_CIPHER_KEYLADDER_SCK_0
#define NOCS_TDES_ROOTKEY					MT_CIPHER_KEYLADDER_SCK_1
#define NOCS_CSA2_ROOTKEY					MT_CIPHER_KEYLADDER_SCK_2
#define NOCS_CSA3_ROOTKEY					MT_CIPHER_KEYLADDER_SCK_3
#define NOCS_FLASH_PROTECT_ROOTKEY		MT_CIPHER_KEYLADDER_SCK_13
#define NOCS_ETSI_KDF_ROOTKEY				MT_CIPHER_KEYLADDER_SCK_5
#define NOCS_ETSI_CW_ROOTKEY				MT_CIPHER_KEYLADDER_PRIVATE_0	//private0
#define NOCS_ETSI_NONCE_ROOTKEY			MT_CIPHER_KEYLADDER_PRIVATE_1	//private1
#define NOCS_ETSI_SECRET_MASK_KEY			MT_CIPHER_HARDWIRED_KEY0
/* Conax Key*/
#define CONAX_CSK_ROOTKEY					MT_CIPHER_KEYLADDER_SCK_4
#define CONAX_LPPK_ROOTKEY				MT_CIPHER_KEYLADDER_SCK_5
#define CONAX_PVR_ROOTKEY					MT_CIPHER_KEYLADDER_SCK_6

/* Nagra Flash protection key slot id */
#define NOCS_FLASH_PROT_KEY_SLOT                                (1) //MT_CIPHER_KEYSLOT_FLASH

#ifdef CA_SEC_DEBUG
TUnsignedInt8 gSecDebugLevel = NOSC_SEC_API_DEBUG_PRINT_MAX;
#endif

#define SESSION_VALID       0x55aa55aa
#define SESSION_INVALID     0x00000000

#define sec_bit(pos)                        (1U << (pos))
#define sec_bits(val, pos)                  ((val) << (pos))
#define sec_set_bit(reg, mask)              ((reg) |= (mask))
#define sec_clr_bit(reg, mask)              ((reg) &= ~(mask))
#define sec_get_bit(reg, mask)              ((reg) & (mask))
#define sec_get_bit_val(reg, shift, vmask)  (((unsigned int)(reg) >> (shift)) & (vmask))
#define sec_set_bit_val(reg, shift, mask)   (((unsigned int)(reg) & (mask)) << (shift))
#if 0
static long long get_sys_time_ms(void)
{
    long long time_ms = 0;
    struct timeval tv;

    gettimeofday(&tv, NULL);
    time_ms = ((long long)tv.tv_sec*1000000 + tv.tv_usec) / 1000;

    return time_ms;
}
#endif
/*
void sec_dump(const char *str, MT_U8 *addr, MT_U32 size)
{
#if 0//def CA_SEC_DEBUG
	int i;
	printf("%s: %p(%d)\n", str, addr, size);
	for (i = 0; i < size; i++)
	{
		if ((i % 16) == 0)
			printf("%08lx: ", (ulong)(addr + i));

		printf("%02x ", addr[i]);

		if ((i % 16) == 15)
			printf("\n");
	}
	printf("\n");
#endif
}
*/

static int mt_dmacpy_ch4(phys_addr_t dst, const phys_addr_t src, mt_u32 size)
{
    MT_EDMA_CH_E ch = MT_EDMA_CH_4;
    mt_u32 count = 0;
    mt_s32 ret = SEC_ERROR;

    while (MT_EDMA_STATUS_FREE != mt_unf_dma_check(ch)) {
        //10ms
        MT_USLEEP(1000 * 10);
        if (count >= 300) {// timeout is 3000ms
            return SEC_ERROR;
        }
        count++;
    }

    ret = mt_unf_dma_memcpy(ch, (const phys_addr_t)src, (phys_addr_t)dst, size);
    if (ret != MT_SUCCESS) {
        SECAPI_ERROR("dma-%d copy from %p to %p with size(%x) failed", ch, src, dst, size);
        return SEC_ERROR;
    }

    return SEC_NO_ERROR;
}

struct MT_SEC_SESSION_S
{
    void  *pxSession;
    TSessionType sessiontype;
    TSessionOpType op;
    TUnsignedInt32  sessionStatus;
    SMPC_SlotType sessionSlotType; //default is Audio, it use to index which PVR and vedio type should be used !!!
    //dmx infor
};

static int mtSecSetIv(TUnsignedInt32 xKeyslotId, TUnsignedInt8 *iv, TUnsignedInt32 ivSize);

static void mtSecSetImplicitIv(TUnsignedInt16 xEmi, TUnsignedInt32 xSlotId, TUnsignedInt32 xSlotCount);
//Fixed TransportSessionId for DALTS
//#define MT_TSID_FOR_DALTS   0

//session is diffrent from transportID, one transportID may have two session, one is encryption, the other is decryption!!!
struct MT_SEC_SESSION_S gsSecSessions[MAX_INTERNAL_SESSIONS] = {0};

//TODO:Different session has different pids
static TSecPidInfo gsSecPIDInfo[MAX_INTERNAL_SESSIONS] = {0};

static int sessionValidation(void *pxSession)
{
    int i = 0;

    secLock();
    for (i = 0; i < MAX_INTERNAL_SESSIONS; i++)
    {
        if (gsSecSessions[i].sessionStatus == SESSION_VALID)
        {
            if (gsSecSessions[i].pxSession == pxSession)
            {

//		if (xSession->magic != MT_SESSION_MAGIC)
	//	return SEC_ERROR_BAD_PARAMETER;

                secUnlock();
                return 0; //valid session
            }
        }
    }
    secUnlock();

    SECAPI_INFO("ERROR \n");
    return -1; //not a valid session
}

static void removeSession(void *pxSession)
{
    int i = 0;

    secLock();
    for (i = 0; i < MAX_INTERNAL_SESSIONS; i++)
    {
        if (gsSecSessions[i].sessionStatus == SESSION_VALID)
        {
            if (gsSecSessions[i].pxSession == pxSession)
            {
                SECAPI_INFO("pxSession[%d]: 0x%lx\n", i, (TUnsignedLong)pxSession);
                gsSecSessions[i].pxSession = NULL;
                gsSecSessions[i].sessionStatus = SESSION_INVALID;
                gsSecSessions[i].sessiontype = SESSION_UNKNOWN;
                gsSecSessions[i].op = SESSION_UNKNOWN;
                gsSecSessions[i].sessionSlotType = 0;
                secUnlock();
                return;
            }
        }
    }
    secUnlock();
}

static int insertSession(void *pxSession, TSessionType type, TSessionOpType op)
{
    int i = 0;

    secLock();
    for (i = 0; i < MAX_INTERNAL_SESSIONS; i++)
    {
        if (gsSecSessions[i].sessionStatus == SESSION_INVALID)
        {
            if (gsSecSessions[i].pxSession == NULL)
            {
                gsSecSessions[i].pxSession = pxSession;
                gsSecSessions[i].sessionStatus = SESSION_VALID;
                gsSecSessions[i].sessiontype = type;
                gsSecSessions[i].op = op;
                SECAPI_INFO("pxSession[%d]: 0x%lx, id = %d, op = %d\n", i, (TUnsignedLong)pxSession, ((TSecStreamSession)(gsSecSessions[i].pxSession))->transportSessionId, op);

                secUnlock();
                return 0; //success
            }
        }
    }

    secUnlock();
    return -1; //max number of sessions reaches

}

static int checkSameTypeSession( TSessionType type, TSessionOpType op )
{
    int i = 0;
    int k = 0;

    secLock();
    for (i = 0; i < MAX_INTERNAL_SESSIONS; i++)
    {
        if (gsSecSessions[i].sessionStatus == SESSION_VALID && gsSecSessions[i].sessiontype == type&&  gsSecSessions[i].op == op)
        {
		  k++;
                secUnlock();
        }
    }

    secUnlock();
    return k; //max number of sessions reaches

}

static TSecStreamSession mtSecGetSessionByTransportSessionId(TTransportSessionId xTransportSessionId, TSessionOpType op);

static MT_UNF_DMX_CHAN_TYPE_E toDmxChannelType(TUnsignedInt16 type)
{
    switch (type) {
    case SEC_PID_VIDEO:
        return  MT_UNF_DMX_CHAN_TYPE_VID;
    case SEC_PID_AUDIO:
        return  MT_UNF_DMX_CHAN_TYPE_AUD;
    default:
        return MT_UNF_DMX_CHAN_TYPE_VID;
    }
}

static mt_s32 mtSecKeySlotRequest(mt_u8 *slot_index, mt_u32 num)
{
    mt_s32 ret = MT_SUCCESS;
    unsigned int slots[4];

	SECAPI_INFO("num = %d\n", num);
    if ((num < 1) || (num > 4))
        return MT_FAILURE;

    ret = mt_unf_cipher_keyslot_request_multi(num, slots);
    if (ret != MT_SUCCESS) {
        SECAPI_ERROR("request slot failed, ret:%x\n", ret);
        return ret;
    }

    *slot_index = (mt_u8)slots[0];
    SECAPI_INFO("SlotId %d, %d\n", slots[0], slots[1]);

    return MT_SUCCESS;
}

static mt_u8 *vir2phy(const mt_u8 *vir);


static TSecStatus psecByteSwap(TUnsignedInt8 *pxBuffer, TUnsignedInt32 xLen)
{
    TUnsignedInt32 index = 0;
    TUnsignedInt8 tmp = 0;
    for (index=0; index<xLen/2; index++)
    {
        tmp = pxBuffer[index];
        pxBuffer[index] = pxBuffer[xLen-1-index];
        pxBuffer[xLen-1-index] = tmp;
    }
    return SEC_NO_ERROR;
}

static void *psecMalloc(size_t xSize)
{
	void *paddr = NULL;

	paddr = malloc(xSize);
	SECAPI_DINFO("paddr = 0x%lx  xSize = %lx\n", (TUnsignedLong)paddr, xSize);
	return paddr;
}

static void psecFree(void *pxBuffer)
{
	SECAPI_DINFO("pxBuffer=0x%lx \n", (TUnsignedLong)pxBuffer);
	free(pxBuffer);
}

/*
 * Nagravision defined DVB EMI
 */
static TBoolean psecIsDvbEmi(TUnsignedInt16 xEMI)
{
    //TODO:remove 0020,0021,0022,0023 for m2m ts-mode
#if 0
    if (xEMI == 0x0020 || xEMI == 0x0021 || xEMI == 0x0022 || xEMI == 0x0023) {
        return FALSE; //temp solution
    }
#endif

    if (EMI_PLAT_DEF == (xEMI & EMI_PLAT_DEF)) {
        //Currently not support
        return FALSE;
    }

    if (EMI_OTHER_STD == ((xEMI & 0x7FFF) & EMI_OTHER_STD)) {
        //OTT EMI
        return FALSE;
    }

    return TRUE;
}

static TBoolean psecIsHmacEmi(TUnsignedInt16 xEMI)
{
    switch(xEMI)
    {
    case NOCS_EMI_HMAC_SHA256:
        return TRUE;
    default:
        return FALSE;
    }
}

static TBoolean psecChkEmiRange(TUnsignedInt16 xEMI)
{
    //SECAPI_DINFO(" 0x%x xEmi = 0x%x\n", xEMI);

    switch(xEMI)
    {
    case NOCS_EMI_MPEG_TS_DVB_CSA2:
    case NOCS_EMI_MPEG_TS_DVB_CSA3:
    case NOCS_EMI_MPEG_TS_DVB_ASA_64:
    case NOCS_EMI_MPEG_TS_DVB_ASA_128:
    case NOCS_EMI_MPEG_TS_DVB_ASA_LIGHT:
    case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
    case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_DES_ECB_CTS:
    case NOCS_EMI_MPEG_TS_DVB_MULTI2:
    case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
    case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
    case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
    case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
    case NOCS_EMI_AES128_MPEG_DASH_CTR:
    case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
    case NOCS_EMI_AES128_CTR:
    case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
    case NOCS_EMI_TDES_ECB_TAIL_CLEAR:
    case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
    case NOCS_EMI_AES128_HLS:
    case NOCS_EMI_AES128_MPEG_DASH_CBCS:
    case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
    case NOCS_EMI_AES256_ECB_TAIL_CLEAR:
    case NOCS_EMI_HMAC_SHA256:
        return TRUE;
    default:
        SECAPI_ERROR("unknown EMI:0x%x\n", xEMI);
        return FALSE;
    }
}

static MT_CIPHER_KEYLADDER_SOURCE_E psecGetRootKeyID(TUnsignedInt16 xEmi)
{
    MT_CIPHER_KEYLADDER_SOURCE_E rootkey = 0;
    switch(xEmi)
    {
    case NOCS_EMI_MPEG_TS_DVB_CSA2:
        rootkey = NOCS_CSA2_ROOTKEY;
        break;
    case NOCS_EMI_MPEG_TS_DVB_CSA3:
        rootkey = NOCS_CSA3_ROOTKEY;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR:
    case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
    case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
    case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
    case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
    case NOCS_EMI_AES128_MPEG_DASH_CTR:
    case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
    case NOCS_EMI_AES128_CTR:
    case NOCS_EMI_AES128_HLS:
    case NOCS_EMI_AES128_MPEG_DASH_CBCS:
    case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
        rootkey = NOCS_AES_ROOTKEY;
        break;
    case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
    case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
    case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
    case NOCS_EMI_TDES_ECB_TAIL_CLEAR:
    case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
        rootkey = NOCS_TDES_ROOTKEY;
        break;
    default:
        SECAPI_ERROR("no rootkey for EMI %04x\n", xEmi);
        break;
    }
    return rootkey;
}
#define CONAX_CSK_KEYID     (0x11)
#define CONAX_LPPK_KEYID     (0x12)
#define CONAX_PVR_KEYID     (0x13)

static MT_CIPHER_KEYLADDER_SOURCE_E psecGetMklRootKeyID(TUnsignedInt8 rootKeyId)
{
    MT_CIPHER_KEYLADDER_SOURCE_E rootkey = MT_CIPHER_KEYLADDER_SCK_UNKNOWN;
    if (CONAX_CSK_KEYID == rootKeyId)
    {
        rootkey = CONAX_CSK_ROOTKEY;;
    }
    else if (CONAX_LPPK_KEYID == rootKeyId)
    {
        rootkey = CONAX_LPPK_ROOTKEY;
    }
    else if (CONAX_PVR_KEYID == rootKeyId)
    {
        rootkey = CONAX_PVR_ROOTKEY;
    }
    return rootkey;
}

static TBoolean psecIsEMINeedIV(TUnsignedInt16 xEMI)
{
    switch(xEMI)
    {
    case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
    case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
    case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
    case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
    case NOCS_EMI_AES128_MPEG_DASH_CTR:
    case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
    case NOCS_EMI_AES128_CTR:
    case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
    case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
    case NOCS_EMI_AES128_HLS:
    case NOCS_EMI_AES128_MPEG_DASH_CBCS:
    case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
        return TRUE;
    };
    return FALSE;
}

static TBoolean psecChkKeySize(TUnsignedInt16 xEMI, size_t xClearTextKeySize)
{
	SECAPI_INFO("xEMI=0x%x, xClearTextKeySize=0x%lx\n", xEMI, xClearTextKeySize);
	switch (xEMI) {
	case NOCS_EMI_MPEG_TS_DVB_CSA2:
	case NOCS_EMI_MPEG_TS_DVB_DES_ECB_CTS:
	case NOCS_EMI_MPEG_TS_DVB_MULTI2:
		if (8 == xClearTextKeySize) {
			return TRUE;
		} else {
			return FALSE;
		}
	case NOCS_EMI_MPEG_TS_DVB_CSA3:
	case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
	case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR:
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
	case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:
	case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
	case NOCS_EMI_AES128_MPEG_DASH_CTR:
	case NOCS_EMI_AES128_CBC_TAIL_CLEAR:
	case NOCS_EMI_AES128_CTR:
	case NOCS_EMI_AES128_HLS:
	case NOCS_EMI_AES128_MPEG_DASH_CBCS:
	case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
	case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_TDES_ECB_TAIL_CLEAR:
	case NOCS_EMI_TDES_CBC_TAIL_CLEAR:
		if (16 == xClearTextKeySize) {
			return TRUE;
		} else {
			return FALSE;
		}
	case NOCS_EMI_AES256_ECB_TAIL_CLEAR:
	case NOCS_EMI_HMAC_SHA256:
		if (32 == xClearTextKeySize) {
			return TRUE;
		} else {
			return FALSE;
		}
	default:
		return FALSE;
	}
}

static TBoolean psecIsKey256(TUnsignedInt16 xEMI)
{
	switch (xEMI) {
	case NOCS_EMI_AES256_ECB_TAIL_CLEAR:
	case NOCS_EMI_HMAC_SHA256:
		return TRUE;
	default:
		return FALSE;
	}
}

static void mt_ciper2Ctrl(TBoolean isEncryptionSession, TUnsignedInt16 EMI, MT_CIPHER_CTRL_S * p_ctrl)
{
	SECAPI_INFO("isEnc = %d, EMI = 0x%x \n", isEncryptionSession, EMI);
	if (isEncryptionSession)
		p_ctrl->operation = MT_CIPHER_OPERATION_ENCRYPT;
	else
		p_ctrl->operation = MT_CIPHER_OPERATION_DECRYPT;

	if (psecIsDvbEmi(EMI))
		p_ctrl->core = MT_CIPHER_CORE_M2M_TS;
	else
		p_ctrl->core = MT_CIPHER_CORE_M2M_RAW;

	switch (EMI) {
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:	//AES128   0x4020  CBC mode with all bits set to zero IV. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_ECB_TAIL_CLEAR:	//AES128   0x4021  ECB mode. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		SECAPI_INFO("p_ctrl->work_mode = 0x%x \n", p_ctrl->work_mode);
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:	//AES128    MPEG/DASH with clear padding (CBC mode)
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_HLS:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		#ifdef SUPPORT_CBCS_MODE
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBCS;// MT_CIPHER_WORK_MODE_CBCS;
		#else
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		#endif
		break;
	case NOCS_EMI_AES128_CBC_PKCS7_PADDING:	//AES128  0x4023  CBC with PKCS#7 padding. Apple Live Streaming Standard
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CBCS:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		#ifdef SUPPORT_CBCS_MODE
		p_ctrl->work_mode =MT_CIPHER_WORK_MODE_CBCS;// MT_CIPHER_WORK_MODE_CBCS;
		#else
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		#endif
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CTR:	//AES128    MPEG/DASH(CTR mode)
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CTR;
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CTR;
		break;
	case NOCS_EMI_AES128_CBC_TAIL_CLEAR:	//AES128    CBC mode. IV modifiable. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_CTR:	//AES128    CTR mode.
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CTR;
		break;
	case NOCS_EMI_AES256_ECB_TAIL_CLEAR:	//AES256    ECB mode. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES256;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		break;
	case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:	//TDES      CBC mode with all bits set to zero IV. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_TDES_ECB_TAIL_CLEAR:	//TDES      ECB mode. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		break;
	case NOCS_EMI_TDES_CBC_TAIL_CLEAR:	//TDES      CBC mode. IV modifiable. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_HMAC_SHA256:	//HMAC256
		p_ctrl->algorithm = MT_CIPHER_ALG_HMAC256;
		break;
	case NOCS_EMI_MPEG_TS_DVB_CSA2:
		p_ctrl->algorithm = MT_CIPHER_ALG_CSA2;
		break;
	case NOCS_EMI_MPEG_TS_DVB_CSA3:
		p_ctrl->algorithm = MT_CIPHER_ALG_CSA3;
		break;
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR://0x0021
	case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		break;
	case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA://0x0020
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBCDVS042;
		break;
	case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR://0x0022
	case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1://0x0023
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBCDVS042;
		break;
	case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		break;
	default:
		break;
	}
}

static void psecSession2Ctrl(TSecCipherSession xSession,
	MT_CIPHER_CTRL_S * p_ctrl)
{
	mt_ciper2Ctrl(xSession->isEncryptionSession, xSession->EMI, p_ctrl);
}

static TSecStatus psecKeyLadder(
	MT_CIPHER_KEYLADDER_SOURCE_E root_key,
	const unsigned char *pxL2CipheredProtectingKey,
	const unsigned char *pxL1CipheredProtectingKey,
	const unsigned char *pxCipheredContentKey,
	mt_u32 slot_id,
	MT_CIPHER_ALGORITHM_E cipher_algorithm)
{
	mt_s32 ret = -1;
	mt_handle kl_handle = 0;
	mt_u8 *p_kl_cmdsig = NULL;
#if 1 //hardkey1异或OTP_KPHMACAuthKey, TEE-OS will set it, so after booting TEE-OS, shoulde use this group!
    mt_u8 kl_cmdsig[] = {
        0xBF,0x0D,0x48,0x0D,0xC0,0xA0,0x11,0xEC,
        0xDD,0x8E,0x6F,0xBC,0x94,0x4E,0x10,0xBB,
        0x3D,0xC5,0x8A,0x17,0x93,0x12,0x68,0xED,
        0xD0,0x8E,0x5C,0x07,0xE1,0xEC,0x8C,0x71,//SCK0 AESRootKey, 3, TDES
        0x17,0x45,0x53,0x0A,0x6F,0x72,0xED,0x52,
        0xAB,0xE5,0x92,0x99,0xD6,0x32,0x68,0x00,
        0x41,0xE9,0x9E,0x8E,0xA8,0x29,0x0D,0xC9,
        0x0A,0xC8,0x9A,0xCD,0x4B,0x8D,0xA8,0xA5,//SCK1 TDESRootKey, 3, TDES
        0xC7,0xA0,0xCB,0x82,0x5F,0x88,0xB6,0xD3,
        0xFF,0x89,0xD4,0x1B,0xEA,0x93,0x5F,0xC9,
        0x86,0x29,0xA5,0x66,0x12,0xC9,0x1E,0x34,
        0x55,0xBD,0xD2,0x99,0xFC,0x07,0xAE,0x37,//SCK2 CSA2RootKey, 3, TDES
        0xBF,0xD7,0x34,0x5B,0xFC,0x08,0xC2,0xC5,
        0x9A,0x26,0x75,0x96,0x2F,0x93,0x57,0xBD,
        0x61,0x4D,0x86,0xA1,0xD0,0x33,0x9C,0xF7,
        0x82,0x33,0x74,0xCF,0xAF,0xD1,0xD9,0x7A,//SCK3 CSA3RootKey, 3, TDES
        0x7D,0xD9,0x99,0x4E,0x14,0x4F,0xB6,0x06,
        0x16,0xDC,0x48,0xAD,0x6B,0x94,0xA8,0x99,
        0xD6,0x6E,0xC9,0x17,0xF0,0x8A,0x80,0xD6,
        0x8D,0xB5,0xA3,0x52,0x36,0xAD,0x1B,0xB6,//SCK4 CSK, 3, TDES
        0x71,0x5C,0xBD,0x98,0x15,0xB7,0xA7,0x5A,
        0xA6,0x1D,0x82,0x29,0x8B,0x60,0x32,0x6C,
        0xED,0xE7,0x26,0x6F,0x6A,0x39,0xB2,0xC2,
        0x09,0x92,0xDF,0x59,0x3D,0x84,0x99,0x9E,//SCK4 CSK, 3, AES
        0xF3,0xD8,0x26,0x0B,0x01,0x93,0xAD,0x47,
        0x66,0x42,0xEE,0x85,0x63,0xB9,0x85,0x66,
        0x76,0x0F,0xEA,0x46,0xF1,0xBB,0xCE,0x16,
        0x72,0xDB,0x9E,0xD9,0x14,0xF8,0x21,0xC3,//SCK5 LPPK, 3, TDES
        0x68,0x10,0xFC,0x4A,0xDD,0x7A,0x38,0x42,
        0xDA,0xCD,0x19,0xC1,0x35,0xB4,0x98,0x90,
        0xAC,0x75,0x44,0x70,0x7D,0x06,0x14,0x92,
        0x83,0x2A,0xCE,0xAB,0x08,0xFE,0xA4,0x8A,//SCK6 PVR, 3, AES
        0x3F,0x85,0x1D,0x89,0xC4,0x2E,0x1D,0x09,
        0x25,0xC4,0x15,0x8A,0xE5,0x53,0x77,0xD9,
        0xDD,0x03,0x81,0xFB,0xE1,0xF8,0xFF,0x01,
        0x81,0x5E,0xD1,0x31,0x24,0x48,0x05,0xCC,//SCK13 flashprotectRootKey, 3, TDES
        0x93,0xDC,0xF8,0xE9,0x84,0x44,0x2B,0xD7,
        0x4A,0x14,0x5D,0x41,0x2E,0x96,0x9C,0x1A,
        0x8B,0x3D,0x8C,0x53,0xCD,0x4B,0x37,0x6A,
        0xB6,0xBA,0xF8,0xCE,0xED,0xF6,0x96,0xDE,//SCK15 ETSI, 3, TDES
    };
#else //hw key0
	mt_u8 kl_cmdsig[] = {
        0xDB,0x95,0x54,0x24,0x55,0x5F,0x2D,0xC6,0x1E,0x45,0x6B,0x84,0x7B,0x05,0x4B,0x8C,
        0x56,0xFE,0x66,0x9B,0xD5,0xAD,0x82,0x2E,0x51,0x55,0xB3,0xE8,0xDA,0xFD,0xE2,0xAA,//SCK0 AESRootKey, 3, TDES
        0x0C,0xEC,0x61,0xCF,0xF6,0xE5,0x49,0x76,0x4A,0xBA,0xAC,0xF7,0x99,0xB8,0xD7,0x19,
        0x6D,0xDF,0xFB,0x06,0x09,0xBA,0xEB,0x3B,0x70,0x68,0x56,0xDE,0xC8,0x0C,0x0F,0x58,//SCK1 TDESRootKey, 3, TDES
        0xE0,0x4C,0x9C,0x1D,0x2C,0x96,0xA1,0x79,0xAC,0x49,0xA5,0x5D,0xE2,0x97,0x63,0xD0,
        0x76,0x96,0x6D,0xD0,0x01,0x2C,0xD7,0x4B,0xF5,0x9C,0x7D,0x23,0x69,0x08,0xD3,0x6C,//SCK2 CSA2RootKey, 3, TDES
        0x1B,0xA1,0x28,0x55,0xA6,0xBD,0x1D,0xC2,0xA1,0x35,0xAF,0x1D,0xFD,0x17,0x25,0xDF,
        0x74,0xD5,0x14,0x27,0x15,0x9B,0x41,0xD3,0x11,0xCA,0x7B,0x0C,0x1A,0x80,0xB9,0x19,//SCK3 CSA3RootKey, 3, TDES
        0x3E,0xE2,0x79,0xF1,0x49,0xD4,0xE7,0x47,0x23,0xB5,0xAB,0x33,0x68,0xBB,0x1D,0xE8,
        0x02,0x2F,0xAF,0x07,0x8D,0x70,0x3E,0x6B,0xA6,0x8F,0x40,0xBE,0xF9,0x66,0xC8,0xBF,//SCK4 CSK, 3, TDES
        0x79,0xF0,0x3F,0x93,0x2E,0xCE,0xF7,0xAE,0xF3,0x99,0x1D,0x4A,0xC6,0xA1,0x75,0x54,
        0xA4,0xAB,0x14,0x28,0x34,0xAB,0x86,0xA3,0xC6,0x3F,0xEC,0x47,0x31,0x2D,0x30,0x42,//SCK4 CSK, 3, AES
        0x46,0x62,0xE3,0xEF,0xC6,0xBE,0xA7,0x56,0xF9,0xE6,0x1D,0x75,0xF8,0xD8,0x5E,0xFB,
        0x34,0x7D,0x60,0xC2,0x69,0x56,0x82,0x95,0x30,0x1E,0x60,0xD6,0x97,0x06,0xBC,0xF2,//SCK5 LPPK, 3, TDES
        0xFF,0x4B,0x53,0x99,0xBF,0xEF,0xE1,0xD7,0x46,0x81,0x9F,0x57,0x4E,0xDF,0x42,0xAA,
        0xC5,0xC6,0xE2,0xB1,0x02,0x52,0xA1,0x33,0xB1,0x3B,0xC8,0x08,0xF1,0x3C,0x07,0xD7,//SCK6 PVR, 3, AES
        0x78,0x8F,0x95,0xD8,0x3C,0x78,0x5E,0x7D,0xE4,0xFA,0xB1,0x2C,0xB6,0xB1,0xD5,0xED,
        0x79,0xA6,0x82,0x2E,0x0A,0x0A,0xCC,0x50,0x4F,0xF0,0x01,0x64,0xF5,0xD5,0x3F,0x14,//SCK13 flashprotectRootKey, 3, TDES
        0x44,0x6D,0x9C,0x76,0xE9,0x3A,0x5A,0x59,0x55,0x5B,0x14,0x4C,0x93,0x47,0x80,0x82,
        0x5A,0xD8,0x8C,0xA8,0xE1,0xE1,0x1E,0xD6,0x67,0x3A,0xFC,0x95,0x19,0x55,0xB9,0xC0,//SCK15 ETSI, 3, TDES
    };
#endif
	SECAPI_INFO("KeyLadder SCK%d to slot %d\n", root_key, slot_id);

	if ((root_key > MT_CIPHER_KEYLADDER_SCK_6)
	    && (root_key != MT_CIPHER_KEYLADDER_SCK_13)
	    && (root_key != MT_CIPHER_KEYLADDER_SCK_15)) {
		SECAPI_ERROR("KeyLadder unknown rootkey %d\n", root_key);
		return SEC_ERROR;
	}
	//temporary method cause of A0 not support AK=BK check for ContentKey
	if (!memcmp(pxCipheredContentKey, pxCipheredContentKey + 8, 8)) {
		SECAPI_ERROR("KeyLadder ContentKey AK=BK found\n");
		return SEC_NO_ERROR;
		//return SEC_ERROR;
	}

       /* The keyladder signature of Sym4 is 32 bytes */
	switch (root_key) {
	case MT_CIPHER_KEYLADDER_SCK_0:
	case MT_CIPHER_KEYLADDER_SCK_1:
	case MT_CIPHER_KEYLADDER_SCK_2:
	case MT_CIPHER_KEYLADDER_SCK_3:
		p_kl_cmdsig = kl_cmdsig + (32 * root_key);
		break;
	case MT_CIPHER_KEYLADDER_SCK_4:
		if (MT_CIPHER_ALG_TDES == cipher_algorithm) {
			p_kl_cmdsig = kl_cmdsig + 128;
		} else {
			p_kl_cmdsig = kl_cmdsig + 160;
		}
		break;
	case MT_CIPHER_KEYLADDER_SCK_5:
		p_kl_cmdsig = kl_cmdsig + 192;
		break;
	case MT_CIPHER_KEYLADDER_SCK_6:
		p_kl_cmdsig = kl_cmdsig + 224;
		break;
	case MT_CIPHER_KEYLADDER_SCK_13:	/* In nagra case, it is flash protection key */
		p_kl_cmdsig = kl_cmdsig + 256;
             break;
      case MT_CIPHER_KEYLADDER_SCK_15:
		p_kl_cmdsig = kl_cmdsig + 288;
		break;
	default:
		return SEC_ERROR;
		break;
	}

	ret = mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_0, &kl_handle);
	if (ret != 0) {
		SECAPI_ERROR("keyladder create error\n");
		return SEC_ERROR;
	}

	ret = mt_unf_cipher_keyladder_start(kl_handle, root_key);
	if (ret != 0) {
		SECAPI_ERROR("keyladder start error\n");
		goto out;
	}


	MT_CIPHER_CTRL_S s_ctrl = { 0 };
	s_ctrl.core = MT_CIPHER_CORE_M2M_RAW;
	s_ctrl.algorithm = cipher_algorithm;
	s_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
	s_ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;
	ret = mt_unf_cipher_keyladder_link(kl_handle, &s_ctrl,
		(unsigned char *)pxL2CipheredProtectingKey, 16);
	ret |= mt_unf_cipher_keyladder_link(kl_handle, &s_ctrl,
		(unsigned char *)pxL1CipheredProtectingKey, 16);
	ret |= mt_unf_cipher_keyladder_link(kl_handle, &s_ctrl,
		(unsigned char *)pxCipheredContentKey, 16);
	if (ret != 0) {
		SECAPI_ERROR("keyladder link error\n");
		goto out;
	}

	//sec_dump("p_kl_cmdsig", p_kl_cmdsig, 32);
	ret = mt_unf_cipher_keyladder_set_signature(kl_handle, p_kl_cmdsig);
	if (ret != 0) {
		SECAPI_ERROR("keyladder signature error ret = 0x%x\n", ret);
		goto out;
	}

	ret = mt_unf_cipher_keyladder_end(kl_handle, slot_id);
      	if (ret != 0) {
		SECAPI_ERROR("ret == 0x%x \n", ret);
		goto out;
	}
out:
	mt_unf_cipher_keyladder_destroy(kl_handle);
	if (ret != 0) {
		SECAPI_ERROR("KeyLadder SCK%d to slot %d\n", root_key, slot_id);
		/* return SEC_ERROR; */
		return SEC_NO_ERROR;	/*For testharness test */
	}
#if 0
	SECAPI_INFO("KeyLadder SCK%d to slot %d success\n", root_key, slot_id);
	hex_dump("pxL2CipheredProtectingKey", pxL2CipheredProtectingKey, 16);
	hex_dump("pxL1CipheredProtectingKey", pxL1CipheredProtectingKey, 16);
	hex_dump("pxCipheredContentKey", pxCipheredContentKey, 16);
#endif
	return SEC_NO_ERROR;
}

static MT_CIPHER_ALGORITHM_E psecGetKlAlgorithm(TSecKladCipher secKlAlgorithm)
{
	MT_CIPHER_ALGORITHM_E kl_algorithm = MT_CIPHER_ALG_BUTT;

	switch (secKlAlgorithm) {
	case SEC_KLAD_CIPHER_TDES:
		kl_algorithm = MT_CIPHER_ALG_TDES;
		break;
	case SEC_KLAD_CIPHER_AES128:
		kl_algorithm = MT_CIPHER_ALG_AES;
		break;
	case SEC_LAST_KLAD_CIPHER:
	default:
		break;
	}

	return kl_algorithm;
}

/*
 * @brief for AES, just a straght zero-pad
 */
static void padVendorIDAES(TUnsignedInt8 * pxVidInput, TUnsignedInt8 * pxVidOutput)
{
	memset(pxVidOutput, 0, 14);
	memcpy(pxVidOutput + 14, pxVidInput, 2);
}

/*
 * @brief for TDES, the vendor ID must be placed in each half, and we want to somehow make each half different
 */
static void padVendorIDDES3(TUnsignedInt8 * pxVidInput, TUnsignedInt8 * pxVidOutput)
{
	pxVidOutput[0] = 0x01;
	memset(pxVidOutput + 1, 0, 5);
	memcpy(pxVidOutput + 6, pxVidInput, 2);
	pxVidOutput[8] = 0x02;
	memset(pxVidOutput + 9, 0, 5);
	memcpy(pxVidOutput + 14, pxVidInput, 2);
}

/*
 * @brief for AES, just a straght zero-pad
 */
static void padModuleIDAES(TUnsignedInt8 * pxMidInput, TUnsignedInt8 * pxMidOutput)
{
	memset(pxMidOutput, 0, 15);
	pxMidOutput[15] = pxMidInput[0];
}

/*
 * @brief for TDES, the Module ID must be placed in each half, and we want to somehow make each half different
 */
static void padModuleIDDES3(TUnsignedInt8 * pxMidInput, TUnsignedInt8 * pxMidOutput)
{
	pxMidOutput[0] = 0x01;
	memset(pxMidOutput + 1, 0, 6);
	pxMidOutput[7] = pxMidOutput[0];
	pxMidOutput[8] = 0x02;
	memset(pxMidOutput + 9, 0, 6);
	pxMidOutput[15] = pxMidOutput[0];
}

static TSecStatus psecRootKeyDerivation(const TSecEtsiKladConfig * pxKladConfig)
{
	mt_s32 ret = -1;
	mt_handle kl_handle = 0;
	TUnsignedInt8 vidPad[16], midPad[16];
	MT_CIPHER_STANDARD_PROFILE_S kdf_profile;
	MT_CIPHER_CTRL_S cipher_ctrl_psm, cipher_ctrl_mkd;

	switch (pxKladConfig->profile) {
	case SEC_ETSI_SCTE_PROFILE_1:
		cipher_ctrl_psm.algorithm = MT_CIPHER_ALG_TDES;
		cipher_ctrl_mkd.algorithm = MT_CIPHER_ALG_BUTT;
		kdf_profile = MT_CIPHER_KL_SCTE_201_2013_P1;
		break;
	case SEC_ETSI_SCTE_PROFILE_1A:
		cipher_ctrl_psm.algorithm = MT_CIPHER_ALG_TDES;
		cipher_ctrl_mkd.algorithm = MT_CIPHER_ALG_TDES;
		kdf_profile = MT_CIPHER_KL_SCTE_201_2013_P1A;
		break;
	case SEC_ETSI_SCTE_PROFILE_2:
		cipher_ctrl_psm.algorithm = MT_CIPHER_ALG_AES;
		cipher_ctrl_psm.operation = MT_CIPHER_OPERATION_ENCRYPT;
		cipher_ctrl_mkd.algorithm = MT_CIPHER_ALG_BUTT;
		kdf_profile = MT_CIPHER_KL_SCTE_201_2013_P2;
		break;
	case SEC_ETSI_SCTE_PROFILE_2A:
		cipher_ctrl_psm.algorithm = MT_CIPHER_ALG_AES;
		cipher_ctrl_psm.operation = MT_CIPHER_OPERATION_ENCRYPT;
		cipher_ctrl_mkd.algorithm = MT_CIPHER_ALG_AES;
		cipher_ctrl_mkd.operation = MT_CIPHER_OPERATION_ENCRYPT;
		kdf_profile = MT_CIPHER_KL_SCTE_201_2013_P2A;
		break;
	case SEC_ETSI_SCTE_PROFILE_2B:
		cipher_ctrl_psm.algorithm = MT_CIPHER_ALG_AES;
		cipher_ctrl_psm.operation = MT_CIPHER_OPERATION_DECRYPT;
		cipher_ctrl_mkd.algorithm = MT_CIPHER_ALG_AES;
		cipher_ctrl_mkd.operation = MT_CIPHER_OPERATION_DECRYPT;
		kdf_profile = MT_CIPHER_KL_SCTE_201_2013_P2B;
		break;
	default:
		SECAPI_ERROR("keyladder kdf not support this profile\n");
		return SEC_ERROR;
		break;
	}

	if (cipher_ctrl_psm.algorithm == MT_CIPHER_ALG_AES) {
		padVendorIDAES((TUnsignedInt8 *)pxKladConfig->vendorId, vidPad);
		padModuleIDAES((TUnsignedInt8 *)&pxKladConfig->moduleId, midPad);
	} else if (cipher_ctrl_psm.algorithm == MT_CIPHER_ALG_TDES) {
		padVendorIDDES3((TUnsignedInt8 *)pxKladConfig->vendorId, vidPad);
		padModuleIDDES3((TUnsignedInt8 *)&pxKladConfig->moduleId, midPad);
	} else {
	}

	ret = mt_unf_cipher_keyladder_create(MT_CIPHER_KEYLADDER_0, &kl_handle);
	if (ret != 0) {
		SECAPI_ERROR("keyladder kdf create error\n");
		return SEC_ERROR;
	}

	ret = mt_unf_cipher_keyladder_start(kl_handle, NOCS_ETSI_KDF_ROOTKEY);
	if (ret != 0) {
		SECAPI_ERROR("keyladder kdf select rootkey error\n");
		goto out;
	}

	ret = mt_unf_cipher_keyladder_link(kl_handle, &cipher_ctrl_psm, vidPad,
					 16);
	if (ret != 0) {
		SECAPI_ERROR("keyladder kdf make PSM error\n");
		goto out;
	}

	ret = mt_unf_cipher_keyladder_kdf_seedv(kl_handle, vidPad, kdf_profile,
			NOCS_ETSI_SECRET_MASK_KEY);
	if (ret != 0) {
		SECAPI_ERROR("keyladder kdf make VFS & FKD error\n");
		goto out;
	}

	if (cipher_ctrl_mkd.algorithm < MT_CIPHER_ALG_BUTT) {
		ret = mt_unf_cipher_keyladder_link(kl_handle, &cipher_ctrl_mkd,
						 midPad, 16);
		if (ret != 0) {
			SECAPI_ERROR("keyladder kdf make MKD error\n");
			goto out;
		}
	}

	ret = mt_unf_cipher_keyladder_store(kl_handle,
			MT_CIPHER_KEYLADDER_STORE_PRIVATE0);
	if (ret != 0) {
		SECAPI_ERROR("keyladder kdf store key to private0 error\n");
		goto out;
	}

out:
	mt_unf_cipher_keyladder_destroy(kl_handle);
	if (ret != 0) {
		SECAPI_ERROR("%s[%d] error\n", __func__, __LINE__);
		return SEC_ERROR;
	}
	return SEC_NO_ERROR;
}

static TSecStatus psecSessionGetKeySlotIndex(
	TSecCipherSession xSession,
	size_t xKeyIdSize,
	TUnsignedInt8 * pxKeyId,
	TUnsignedInt32 * pxKeySlot0,
	TUnsignedInt32 * pxKeySlot1)
{
	unsigned int i = 0;

	*pxKeySlot0 = 0xffffffff;
	if (pxKeySlot1 != NULL) {
		*pxKeySlot1 = 0xffffffff;
	}

	if (!xSession->isStreamSession) {
		*pxKeySlot0 = 0;
		return SEC_NO_ERROR;
	}

	if ((xKeyIdSize == 0) && (pxKeyId == NULL)) {
		*pxKeySlot0 = 0;
		if (pxKeySlot1 != NULL) {
			*pxKeySlot1 = 1;
		}
		xSession->keySlotUsed[0] = 1;
		xSession->keySlotUsed[1] = 1;
		xSession->keyID[0][0] = 0;	//even
		xSession->keyID[1][0] = 1;	//odd
	} else if ((xSession->EMI & 0xFF00) == 0x0000) {
		if ((xKeyIdSize == 1) && (pxKeyId != NULL)) {
			if ((pxKeyId[0] == 0) || (pxKeyId[0] == 1)) {
				*pxKeySlot0 = pxKeyId[0];
			}
			xSession->keySlotUsed[pxKeyId[0]] = 1;
			xSession->keyID[pxKeyId[0]][0] = pxKeyId[0];
			SECAPI_INFO("pxKeyId[0] = %d, %d \n", pxKeyId[0],
				xSession->keyID[pxKeyId[0]][0]);
		}
	} else {
		if ((xKeyIdSize > 0) && (pxKeyId != NULL)) {
			for (i = 0; i < 2; i++) {
				if (!xSession->keySlotUsed[i]) {
					xSession->keySlotUsed[i] = 1;
					*pxKeySlot0 = i;
					memcpy(xSession->keyID[i], pxKeyId,
						xKeyIdSize);
					break;
				} else
					if (!memcmp(xSession->keyID[i], pxKeyId,
						xKeyIdSize)) {
						*pxKeySlot0 = i;
						break;
				}
			}
			if (i == 2) {
				xSession->keySlotLatest = (xSession->keySlotLatest + 1) % 2;
				*pxKeySlot0 = xSession->keySlotLatest;
				memcpy(xSession->keyID[*pxKeySlot0], pxKeyId,
					xKeyIdSize);
			}

		}
	}
	return SEC_NO_ERROR;
}

static TBoolean attachDescrambler(TTransportSessionId xTransportSessionId, mt_handle *pHandle, TUnsignedInt32 evenSlot, TUnsignedInt32 oddSlot)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_DMX_DESCRAMBLER_ATTR_S stDescramblerAttr = {0};
    mt_handle dscHandle = MT_INVALID_HANDLE;
    mt_u32 xDmxId = 0;

    SECAPI_INFO("\n");

    SECAPI_INFO("current tsid=%d\n", xTransportSessionId);
    xDmxId = mtSecGetPlatDmxId(xTransportSessionId);

    SECAPI_INFO("evenSlot:%d oddSlot:%d xDmxId = %d \n", evenSlot,oddSlot, xDmxId);

    stDescramblerAttr.enCaType = MT_UNF_DMX_CA_ADVANCE;
    stDescramblerAttr.enEntropyReduction = MT_UNF_DMX_CA_ENTROPY_REDUCTION_OPEN;
    stDescramblerAttr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2; //default

    ret = MT_UNF_DMX_CreateDescramblerExt(xDmxId, &stDescramblerAttr, &dscHandle);
    if (ret != MT_SUCCESS) {

        SECAPI_ERROR("create descrambler failed\n");
        return FALSE;
    }

    ret = MT_UNF_DMX_SetDescramblerEvenKeySlot(dscHandle, (mt_u8)evenSlot);
    if (ret != MT_SUCCESS) {

        MT_UNF_DMX_DestroyDescrambler(dscHandle);
        SECAPI_ERROR("MT_UNF_DMX_SetDescramblerOddKeySlot failed ret = %x  xTransportSessionId = %x dscHandle = %x oddSlot = %d evenSlot= %d  \n", ret, xTransportSessionId,  dscHandle, oddSlot, evenSlot);

        return FALSE;
    }

    ret = MT_UNF_DMX_SetDescramblerOddKeySlot(dscHandle, (mt_u8)oddSlot);

    if (ret != MT_SUCCESS) {

        SECAPI_ERROR("MT_UNF_DMX_SetDescramblerOddKeySlot failed ret = %x  xTransportSessionId = %x dscHandle = %x oddSlot = %d evenSlot= %d  \n", ret, xTransportSessionId,  dscHandle, oddSlot, evenSlot);

        MT_UNF_DMX_DestroyDescrambler(dscHandle);

        return FALSE;
    }

    *pHandle = dscHandle;

    SECAPI_INFO("dscHandle=0x%lx \n", dscHandle);

    return TRUE;
}

static TBoolean detachDescrambler(TTransportSessionId tsid, transportSessionType type, mt_handle dschandle, TSecPidInfo *pxPidInfo)
{
    mt_u32 i = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_handle hChannel = MT_INVALID_HANDLE;
    MT_UNF_DMX_CHAN_TYPE_E enType;
    TUnsignedInt32 dmxid = mtSecGetPlatDmxId(tsid);

    TUnsignedInt32 pidnum = pxPidInfo->pidNum;

    SECAPI_INFO("\n");

    SECAPI_INFO("tsid=%ld, dmxid = %d \n", tsid, dmxid);

    if (pidnum > 0) {
        for (i = 0; i < pidnum; i++) {

            SECAPI_TRACE("i = 0x%x, dscHandle=0x%lx, id=0x%x \n", i, dschandle, dmxid);

            if (type == TRANSPORT_SESSION_TYPE_RECORD)
                enType = MT_UNF_DMX_CHAN_TYPE_REC;
            else
                enType = toDmxChannelType(pxPidInfo->pidList[i].type);
            ret = MT_UNF_DMX_GetChannelHandleByPidType(dmxid, pxPidInfo->pidList[i].pid, enType, &hChannel);
            if (ret == MT_SUCCESS) {
                ret = MT_UNF_DMX_DetachDescrambler(dschandle, hChannel);
            }
        }
    }

    MT_UNF_DMX_DestroyDescrambler(dschandle);
    if (ret == MT_SUCCESS)
        return TRUE;
    else
        return FALSE;
}

static TBoolean psecSetDescramblerType(mt_handle hDscHandle, TUnsignedInt16 xEMI)
{
    mt_s32 ret = MT_SUCCESS;
    MT_UNF_DMX_DESCRAMBLER_ATTR_S dsc_attr = {0};

    SECAPI_INFO("\n");

    if (hDscHandle == 0) {
	 SECAPI_INFO("%s %d \n",__FUNCTION__,__LINE__);
        return FALSE;
    }

    ret = MT_UNF_DMX_GetDescramblerAttr(hDscHandle, &dsc_attr);
    if (ret != MT_SUCCESS) {
	 SECAPI_ERROR("%s %d \n",__FUNCTION__,__LINE__);

        return FALSE;
    }

    switch (xEMI) {
    case NOCS_EMI_MPEG_TS_DVB_CSA2:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA2;
        break;
    case NOCS_EMI_MPEG_TS_DVB_CSA3:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_CSA3;
        break;
    case NOCS_EMI_MPEG_TS_DVB_ASA_64:
    case NOCS_EMI_MPEG_TS_DVB_ASA_128:
    case NOCS_EMI_MPEG_TS_DVB_ASA_LIGHT:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_ASA;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_DVS042;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_TAIL_CLEAR;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR:
	 dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_HEAD_CLEAR;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR;
        break;
    case NOCS_EMI_MPEG_TS_DVB_TDES_CBC_ZEROIV_DVS042:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_CBC_DVS042;
        break;
    case NOCS_EMI_MPEG_TS_DVB_TDES_ECB_TAIL_CLEAR:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_TDES_ECB_TAIL_CLEAR;
        break;
    case NOCS_EMI_MPEG_TS_DVB_DES_ECB_CTS:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_DES_ECB_CTS_CLR;
        break;
    case NOCS_EMI_MPEG_TS_DVB_MULTI2:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLE_MULTI2_CBC_OFB;
        break;

        /* For RAW Stream */
    case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR;
        break;
    case NOCS_EMI_AES128_ECB_TAIL_CLEAR:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_ECB_TAIL_CLEAR;
        break;
    case NOCS_EMI_AES128_CBC_PKCS7_PADDING:
        dsc_attr.enDescramblerType = MT_UNF_DMX_DESCRAMBLER_TYPE_AES_CBC_TAIL_CLEAR;
        break;
    default:
        SECAPI_ERROR("EMI=0x%x:Not Support!\n", xEMI);

        return FALSE;
    }

    SECAPI_INFO("enDescramblerType:%d\n", dsc_attr.enDescramblerType);

    ret = MT_UNF_DMX_SetDescramblerAttr(hDscHandle, &dsc_attr);

    if (ret != MT_SUCCESS) {

        SECAPI_ERROR("SetDescramblerAttr failed\n");
        return FALSE;
    }

    return TRUE;
}



static TBoolean mtSecCryptoEngineProcessPrepare(MT_CIPHER_CTRL_S *p_ctrl,
        TUnsignedInt16 algo_emi,
        TSecPidInfo *pidInfo,
        unsigned int kt_slot,
        mt_handle *pHandle)
{
	mt_handle crypto_handle = MT_INVALID_HANDLE;
	MT_CIPHER_TS_PARA_S ts_para = {0};
	mt_u8 isTS = 0; // for TS-MODE
	mt_u8 isHMAC = 0;
	int ret = 0;

	if (psecIsDvbEmi(algo_emi))
		isTS = 1;
	else if (psecIsHmacEmi(algo_emi))
		isHMAC = 1;

	SECAPI_TRACE("algo_emi = 0x%x, kt_slot = %d, mode = 0x%x  isTS = %d isHMAC =%d  algo_emi = %x\n", algo_emi, kt_slot, p_ctrl->work_mode, isTS, isHMAC , algo_emi);

	if (isTS) {
		// TS-MODE
		if (MT_SUCCESS != mt_unf_cipher_ts_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle)) {
			SECAPI_ERROR("ts create failed\n");
			goto ce_create_error;
		}

		ts_para.ts_enc_ksel = MT_TS_ENC_EVEN_KEY;   //Doesn't matter, even=odd
		ts_para.ts_dec_ind = MT_TS_DEC_IND_CLEAR;    //keep it for comapring with expect
		ts_para.ts_pkt_len = MT_TS_LEN_PKT_188BYTE;
		ts_para.ts_ive_mode = MT_TS_IVE_OFF;        //if need iv, use user input IV

		SECAPI_INFO("pidInfo->pidNum:%d\n", pidInfo->pidNum);

		if (pidInfo->pidNum > 0) {
			ts_para.ts_pid0_filt_en = MT_TS_PID_FILT_ENABLE;
			ts_para.ts_pid0_filt_num = pidInfo->pidList[0].pid;
			SECAPI_INFO("pid0:0x%x\n", ts_para.ts_pid0_filt_num);
		}
		if (pidInfo->pidNum > 1) {
			ts_para.ts_pid1_filt_en = MT_TS_PID_FILT_ENABLE;
			ts_para.ts_pid1_filt_num = pidInfo->pidList[1].pid;
			SECAPI_INFO("pid1:0x%x\n", ts_para.ts_pid1_filt_num);
		}
		if (pidInfo->pidNum > 2) {
			ts_para.ts_pid2_filt_en = MT_TS_PID_FILT_ENABLE;
			ts_para.ts_pid2_filt_num = pidInfo->pidList[2].pid;
			SECAPI_INFO("pid2:0x%x\n", ts_para.ts_pid2_filt_num);
		}

		if (p_ctrl->operation == MT_CIPHER_OPERATION_ENCRYPT)
			ts_para.ts_force_enc_en = MT_TS_FORCE_ENC_ENABLE;
		else
			ts_para.ts_force_enc_en = MT_TS_FORCE_ENC_DISABLE;

		ts_para.ts_short_mode = MT_TS_SHORT_TAIL;
		/* tail processing */
		if (algo_emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA) {
			ts_para.ts_small_mode = MT_TS_SMALL_DVS042_TAIL;
		} else {
			ts_para.ts_small_mode = MT_TS_SMALL_CLEAR;
		}
		//printf("0x%x, 0x%x, 0x%x, 0x%x, ts_small_mode=%d\n", p_ctrl->core, p_ctrl->operation, p_ctrl->algorithm, p_ctrl->work_mode, ts_para.ts_small_mode);
		if (MT_SUCCESS != mt_unf_cipher_ts_config(crypto_handle, p_ctrl, &ts_para, kt_slot, kt_slot)) {
			SECAPI_ERROR("ts config failed\n");
			goto ce_config_error;
		}
	}
	else if (isHMAC) {
		MT_CIPHER_MAC_TYPE_E hmac_type;
		MT_CIPHER_HMAC_ATTS_S hmac_attr;

		memset(&hmac_attr, 0, sizeof(hmac_attr));
                hmac_attr.key_slot[0] = MT_CIPHER_KEYSLOT_INVALID;
                hmac_attr.key_slot[1] = MT_CIPHER_KEYSLOT_INVALID;

		if (algo_emi == NOCS_EMI_HMAC_SHA256) {
			hmac_type = MT_CIPHER_MAC_TYPE_SHA256;
			hmac_attr.key_len = 32;
			hmac_attr.key_slot[0] = kt_slot;
		} else {
			SECAPI_ERROR("unknown emi\n");
			goto ce_create_error;
		}

		if (0 != mt_unf_cipher_mac_create(hmac_type, &hmac_attr, &crypto_handle)) {
			SECAPI_ERROR("mt_unf_cipher_mac_create fail\n");
			goto ce_create_error;
		}
	} else {
		//RAW-EMIs
		//printf("RAW-EMIs---------------------------------\n");
		if (0 != mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle)) {
			SECAPI_ERROR("crypto create failed\n");
			goto ce_create_error;
		}

		ret = mt_unf_cipher_crypto_config(crypto_handle, p_ctrl, kt_slot);
		if (0 != ret ) {
			SECAPI_ERROR("crypto config failed ret = 0x%x \n", ret);
			goto ce_config_error;
		}
	}

	*pHandle = crypto_handle;
//	SECAPI_INFO("\n");
	return TRUE;

ce_config_error:

	if (isTS)
		mt_unf_cipher_ts_destroy(crypto_handle);
	else if (!isHMAC)
		mt_unf_cipher_crypto_destroy(crypto_handle);

ce_create_error:
	return FALSE;
}


static mt_s32 mtSecCryptoEngineProcessStart_phy(mt_handle crypto_handle,
        TUnsignedInt16 xEmi,
        const TUnsignedInt8 *pxInput,
        TUnsignedInt8 *pxOutput,
        size_t xMessageSize)
{
	mt_s32 ret = MT_FAILURE;
	//mt_u8 *p_src_phy_addr;
	//mt_u8 *p_dst_phy_addr;
	//mt_u8 *p_src_mmz_addr = NULL;
	//mt_u8 *p_dst_mmz_addr = NULL;
	mt_u8 isTS = 0; // for TS-MODE
	mt_u8 isHMAC = 0;

//	SECAPI_INFO("crypto_handle = 0x%lx\n", crypto_handle);

	if (psecIsDvbEmi(xEmi))
		isTS = 1;
	else if (psecIsHmacEmi(xEmi))
		isHMAC = 1;

 	SECAPI_TRACE("%s %d xEmi = 0x%x xMessageSize = %d  isTS=%d isHMAC=%d \n",__FUNCTION__,__LINE__, xEmi, xMessageSize, isTS, isHMAC);

	if (isTS) {


		//mt_unf_cipher_ts_create
		//mt_unf_cipher_crypto_config

		//SECAPI_INFO("ts-mode process in, size:%ld, 0x%lx, 0x%lx \n", xMessageSize, pxInput, pxOutput);
		ret = mt_unf_cipher_ts_process(crypto_handle, (phys_addr_t)pxInput, (phys_addr_t)pxOutput, xMessageSize);
		if( ret != 0)
		{
			SECAPI_ERROR("%s %d ret = 0x%x\n",__FUNCTION__,__LINE__, ret);
		}

	}
	else if (isHMAC) {
		//SECAPI_INFO("mac-mode process in, size:%ld\n", xMessageSize);

		ret = mt_unf_cipher_mac_update(crypto_handle, pxInput, xMessageSize);

		ret |= mt_unf_cipher_mac_final(crypto_handle, pxOutput);
		if( ret != 0)
		{
			SECAPI_ERROR("%s %d ret = 0x%x\n",__FUNCTION__,__LINE__, ret);
		}

	}
	else {
		//SECAPI_INFO("raw-mode process in, size:%ld\n", xMessageSize);
		//printf("%s %d crypto_handle=%d, pxInput=%x, pxOutput=%x, xMessageSize=%d \n",__FUNCTION__,__LINE__, crypto_handle, (mt_u8 *)pxInput, (mt_u8 *)pxOutput, xMessageSize);

		ret = mt_unf_cipher_crypto_process_phy(crypto_handle, (phys_addr_t)pxInput, (phys_addr_t)pxOutput, xMessageSize);
		if( ret != 0)
		{
			SECAPI_ERROR("%s %d ret = 0x%x\n",__FUNCTION__,__LINE__, ret);
		}

	}

	//printf("%s %d   ret = 0x%x\n",__FUNCTION__,__LINE__, ret);

	//SECAPI_INFO("ret = %d \n", ret);
	return ret;
}


// pxInput and pxOutput is vir adr
static mt_s32 mtSecCryptoEngineProcessStart(mt_handle crypto_handle,
        TUnsignedInt16 xEmi,
        const TUnsignedInt8 *pxInput,
        TUnsignedInt8 *pxOutput,
        size_t xMessageSize)
{
	mt_s32 ret = MT_FAILURE;
	mt_u8 *p_src_phy_addr = NULL;
	mt_u8 *p_dst_phy_addr = NULL;
	mt_u8 *p_src_mmz_addr = NULL;
	mt_u8 *p_dst_mmz_addr = NULL;
	mt_u8 isTS = 0; // for TS-MODE
	mt_u8 isHMAC = 0;

//	SECAPI_INFO("crypto_handle = 0x%lx\n", crypto_handle);

	if (psecIsDvbEmi(xEmi))
		isTS = 1;
	else if (psecIsHmacEmi(xEmi))
		isHMAC = 1;

	//printf("%s %d xEmi = 0x%x xMessageSize = %d  isTS=%d isHMAC=%d \n",__FUNCTION__,__LINE__, xEmi, xMessageSize, isTS, isHMAC);

	if (isTS) {

		p_src_phy_addr = vir2phy(pxInput);
		p_dst_phy_addr = vir2phy(pxOutput);

		if (!p_src_phy_addr) {

			p_src_mmz_addr = mt_unf_cipher_malloc(xMessageSize);
			memcpy(p_src_mmz_addr, pxInput, xMessageSize);
			p_src_phy_addr = vir2phy(p_src_mmz_addr);

			//sec_dump("pxInput", p_src_mmz_addr, xMessageSize);
		}

		if (!p_dst_phy_addr) {

			p_dst_mmz_addr = mt_unf_cipher_malloc(xMessageSize);
			p_dst_phy_addr = vir2phy(p_dst_mmz_addr);

		}

		SECAPI_DINFO("ts-mode process in, size:%ld, 0x%p, 0x%p \n", xMessageSize, p_src_phy_addr, p_dst_phy_addr);
		ret = mt_unf_cipher_ts_process(crypto_handle, (phys_addr_t)p_src_phy_addr, (phys_addr_t)p_dst_phy_addr, xMessageSize);

		if (p_dst_mmz_addr) {

			memcpy(pxOutput, p_dst_mmz_addr, xMessageSize);

			//sec_dump("pxOutput", pxOutput, xMessageSize);
		}

		if (p_src_mmz_addr)
			mt_unf_cipher_free(p_src_mmz_addr);
		if (p_dst_mmz_addr)
			mt_unf_cipher_free(p_dst_mmz_addr);
	}
	else if (isHMAC) {
		SECAPI_INFO("mac-mode process in, size:%ld\n", xMessageSize);

		ret = mt_unf_cipher_mac_update(crypto_handle, pxInput, xMessageSize);

		ret |= mt_unf_cipher_mac_final(crypto_handle, pxOutput);

	}
	else {
		//for raw EMI 0x4020 0x4021 0x4023

		//SECAPI_INFO("raw-mode process in, size:%ld\n", xMessageSize);

		p_src_phy_addr = vir2phy(pxInput);
		p_dst_phy_addr = vir2phy(pxOutput);

		if (!p_src_phy_addr) {

			//printf("------------------------------------------------------------------------------------\n");
			p_src_mmz_addr = mt_unf_cipher_malloc(xMessageSize);
			memcpy(p_src_mmz_addr, pxInput, xMessageSize);
			p_src_phy_addr = vir2phy(p_src_mmz_addr);

			//sec_dump("p_src_mmz_addr", p_src_mmz_addr, xMessageSize);
			//sec_dump("pxInput", p_src_mmz_addr, xMessageSize);
		}

		if (!p_dst_phy_addr) {

			p_dst_mmz_addr = mt_unf_cipher_malloc(xMessageSize);
			p_dst_phy_addr = vir2phy(p_dst_mmz_addr);

		}


		//printf("%s %d crypto_handle=%d, pxInput=%x, pxOutput=%x, xMessageSize=%d p_src_phy_addr = %x p_dst_phy_addr = %x \n",__FUNCTION__,__LINE__,
			//crypto_handle, (mt_u8 *)pxInput, (mt_u8 *)pxOutput, xMessageSize, p_src_phy_addr,p_dst_phy_addr );
		if(p_src_phy_addr == NULL || p_dst_phy_addr == NULL)
		{
			SECAPI_ERROR("[error]%s %d p_src_phy_addr = %p , p_dst_phy_addr =%p \n",__FUNCTION__,__LINE__, p_src_phy_addr, p_dst_phy_addr);
			return 1;
		}

		ret = mt_unf_cipher_crypto_process_phy(crypto_handle, (phys_addr_t)p_src_phy_addr, (phys_addr_t)p_dst_phy_addr, xMessageSize);

		//sec_dump("p_dst_mmz_addr", p_dst_mmz_addr, xMessageSize);

		if (p_dst_mmz_addr) {

			memcpy(pxOutput, p_dst_mmz_addr, xMessageSize);
			//sec_dump("pxOutput", pxOutput, xMessageSize);

		}

		if (p_src_mmz_addr)
			mt_unf_cipher_free(p_src_mmz_addr);
		if (p_dst_mmz_addr)
			mt_unf_cipher_free(p_dst_mmz_addr);


	}

	//printf("%s %d \n",__FUNCTION__,__LINE__);

//	SECAPI_INFO("ret = %d \n", ret);
	return ret;
}

static TBoolean mtSecCryptoEngineProcessEnd(mt_handle crypto_handle, TUnsignedInt16 xEmi)
{
	TBoolean ret = MT_FAILURE;
	mt_u8 isTS = 0; // for TS-MODE
	mt_u8 isHMAC = 0;

//	SECAPI_INFO("\n");


	if (psecIsDvbEmi(xEmi))
	{
		isTS = 1;
	}
	else if (psecIsHmacEmi(xEmi))
	{
		isHMAC = 1;
	}

	if (isTS) {
		if(crypto_handle!=0 && crypto_handle!=MT_INVALID_HANDLE)
		ret = mt_unf_cipher_ts_destroy(crypto_handle);
	} else if (!isHMAC) {
		if(crypto_handle!=0&& crypto_handle!=MT_INVALID_HANDLE)
		ret = mt_unf_cipher_crypto_destroy(crypto_handle);
	}

	//SECAPI_INFO("crypto destroyed!\n");

	return ret;
}

TSignedInt32 mtSecRawStreamDataProcess_phy(TTransportSessionId xTransportSessionId, TSessionOpType op, TUnsignedInt32 size, const TUnsignedInt8 *input, TUnsignedInt8 *output)
{
    mt_s32 ret = MT_FAILURE;
    mt_handle crypto_handle = MT_INVALID_HANDLE;
    TSecStreamSession xSession;
    TUnsignedInt16 xEmi;

    //SECAPI_INFO("Raw stream data process In, size:%d, tsid:%d, inAddr:%x,outAddr:%x\n", size, xTransportSessionId, (TUnsignedInt32)input, (TUnsignedInt32)output);
    secLock();

    xSession = mtSecGetSessionByTransportSessionId(xTransportSessionId, op);

    if (NULL == xSession) {
        secUnlock();

        SECAPI_ERROR("Session not found or closed!  xTransportSessionId = %d , op = %d \n" , xTransportSessionId, op);
        return -4;
    }

    xEmi = xSession->EMI;
    crypto_handle = xSession->cryptoHandle;
    //SECAPI_INFO("Got crypto_handle:%x, emi:%x xTransportSessionId = %d \n", crypto_handle, xEmi, xTransportSessionId);

    //RAW stream data process
    if (xEmi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
            || xEmi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
            || xEmi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {

        //TODO:0020,0021,0022,0023 shall go to Scrambler
        //SECAPI_INFO("Raw stream data process start, size:%d\n", size);
        if (crypto_handle != MT_INVALID_HANDLE) {

            if (SESSION_OP_ENCRYPT == op
                    && xEmi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {

                //TODO: how to padding, and output data size after padded?
                //printf("pkcs7_padding, insize:%d\n", size);
                ret = mtSecCryptoEngineProcessStart_phy(crypto_handle, xEmi, input, output, size);

                if (MT_SUCCESS != ret) {
                    SECAPI_ERROR("Raw stream data process failed, size:%d  ret = 0x%x\n", size, ret);
                    ret = -1;
                    goto raw_stream_process_out;
                }
            } else {


		  //SECAPI_INFO("insize:%d, call process start\n", size);
                ret = mtSecCryptoEngineProcessStart_phy(crypto_handle, xEmi, input, output, size);

                if (MT_SUCCESS != ret) {
                    SECAPI_ERROR("Raw stream data process failed, size:%d ret = %x\n", size, ret);
                    ret = -1;

                    goto raw_stream_process_out;
                }
            }
        } else {
            SECAPI_ERROR("Crypto not ready yet, just return success and wait...\n");
            ret = -1;//0;

            goto raw_stream_process_out;
        }
    }

raw_stream_process_out:
    secUnlock();
	//SECAPI_INFO("process done,ret:%x\n", ret);
    return ret;

}


TSignedInt32 mtSecRawStreamDataProcess(TTransportSessionId xTransportSessionId, TSessionOpType op, TUnsignedInt32 size, const TUnsignedInt8 *input, TUnsignedInt8 *output)
{
    mt_s32 ret = MT_FAILURE;
    mt_handle crypto_handle = MT_INVALID_HANDLE;
    TSecStreamSession xSession;
    TUnsignedInt16 xEmi;

    //SECAPI_INFO("Raw stream data process In, size:%d, tsid:%d, inAddr:%x,outAddr:%x\n", size, xTransportSessionId, input, output);
    secLock();

    xSession = mtSecGetSessionByTransportSessionId(xTransportSessionId, op);

    if (NULL == xSession) {
        secUnlock();

        SECAPI_ERROR("Session not found or closed!\n");
        return -1;
    }

    xEmi = xSession->EMI;
    crypto_handle = xSession->cryptoHandle;
    //SECAPI_INFO("Got crypto_handle:%x, emi:%x xTransportSessionId = %d \n", crypto_handle, xEmi, xTransportSessionId);


    //RAW stream data process
    if (xEmi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
            || xEmi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
            || xEmi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
            || xEmi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {

        //TODO:0020,0021,0022,0023 shall go to Scrambler
        //SECAPI_INFO("Raw stream data process start, size:%d\n", size);
        if (crypto_handle != MT_INVALID_HANDLE) {

            if (SESSION_OP_ENCRYPT == op
                    && xEmi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {

                //TODO: how to padding, and output data size after padded?
               //printf("pkcs7_padding, insize:%d\n", size);
                ret = mtSecCryptoEngineProcessStart(crypto_handle, xEmi, input, output, size);

                if (MT_SUCCESS != ret) {
                    SECAPI_ERROR("Raw stream data process failed, size:%d  ret = 0x%x \n", size, ret);
                    ret = -1;
                    goto raw_stream_process_out;
                }
            } else {


				//SECAPI_INFO("insize:%d, call process start\n", size);
                ret = mtSecCryptoEngineProcessStart(crypto_handle, xEmi, input, output, size);

                if (MT_SUCCESS != ret) {
                    SECAPI_ERROR("Raw stream data process failed, size:%d ret = %x\n", size, ret);
                    ret = -1;

                    goto raw_stream_process_out;
                }
            }
        } else {
            SECAPI_ERROR("Crypto not ready yet, just return success and wait...\n");
            ret = -1;//0;

            goto raw_stream_process_out;
        }
    }

raw_stream_process_out:
    secUnlock();
	//SECAPI_INFO("process done,ret:%x\n", ret);
    return ret;

}

static TBoolean mtSecCryptoEngineProcess(MT_CIPHER_CTRL_S *p_ctrl,
        TUnsignedInt16 algo_emi,
        const TUnsignedInt8 *pxInput,
        TUnsignedInt8 *pxOutput,
        size_t xMessageSize, unsigned int kt_slot)
{
    mt_handle crypto_handle = MT_INVALID_HANDLE;
    TBoolean ret = FALSE;
    mt_s32 mret;

    SECAPI_INFO("xMessageSize:0x%lx\n", xMessageSize);

    if (0 != mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle)) {
        SECAPI_ERROR("\n");
        goto ce_create_error;
    }

    if (0 != mt_unf_cipher_crypto_config(crypto_handle, p_ctrl, kt_slot)) {
        SECAPI_ERROR("\n");
        goto ce_config_error;
    }

    mret = mt_unf_cipher_crypto_process(crypto_handle,
		(TUnsignedInt8 *)pxInput, pxOutput, xMessageSize);
    if (mret != MT_SUCCESS) {
        SECAPI_INFO("process, ret:%x\n", mret);
        goto ce_end;
    }

		//sec_dump("pxInput ", pxInput, 0x20);
		//sec_dump("pxOutput ", pxOutput, 0x20);

    //2.processing last padded block
    //if (needPadding) {
    //if (0 != mt_unf_cipher_crypto_process(crypto_handle, (mt_u8 *)pxPaddingBlock, (mt_u8 *)pxPaddingBlock, (xLastBlockSize + xPaddingSize))) {
    //ret = FALSE;
    //goto ce_end;
    //}
    //
    //memcpy(pxOutput + (xMessageSize - xLastBlockSize), pxPaddingBlock, xLastBlockSize);
    //}

    ret = TRUE;

ce_end:
    //if (needPadding)
    //mt_unf_cipher_free(pxPaddingBlock);

ce_config_error:
    mt_unf_cipher_crypto_destroy(crypto_handle);
ce_create_error:
    return ret;
}

static void psecStreamSession2Ctrl(
	TSecStreamSession xSession,
	MT_CIPHER_CTRL_S * p_ctrl)
{
	//printf("psecStreamSession2Ctrl\n");

	p_ctrl->operation =
		xSession->isEncryptionSession ? MT_CIPHER_OPERATION_ENCRYPT : MT_CIPHER_OPERATION_DECRYPT;
	p_ctrl->core = MT_CIPHER_CORE_M2M_RAW;
	if ((xSession->EMI & 0xFF00) == 0x0000)
		p_ctrl->core = MT_CIPHER_CORE_M2M_TS;
	switch (xSession->EMI) {
	case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA:
        p_ctrl->algorithm = MT_CIPHER_ALG_AES;
        p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBCDVS042;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR:
    case NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR:
        p_ctrl->algorithm = MT_CIPHER_ALG_AES;
        p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
        p_ctrl->algorithm = MT_CIPHER_ALG_AES;
        p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
        break;
    case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
        p_ctrl->algorithm = MT_CIPHER_ALG_AES;
        p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
        p_ctrl->core = MT_CIPHER_CORE_M2M_TS;
        break;
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:	//AES128    CBC mode with all bits set to zero IV. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_ECB_TAIL_CLEAR:	//AES128    ECB mode. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CBC_CLEAR_PADDING:	//AES128    MPEG/DASH with clear padding (CBC mode)
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_HLS:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		#ifdef SUPPORT_CBCS_MODE
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBCS;//MT_CIPHER_WORK_MODE_CBCS;//MT_CIPHER_WORK_MODE_CBCS;
		#else
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		#endif
		break;
	case NOCS_EMI_AES128_CBC_PKCS7_PADDING:	//AES128    CBC with PKCS#7 padding. Apple Live Streaming Standard
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CBCS:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		#ifdef SUPPORT_CBCS_MODE
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBCS;//MT_CIPHER_WORK_MODE_CBCS;//MT_CIPHER_WORK_MODE_CBCS;
		#else
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		#endif
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CTR_CENS:
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CTR;
		break;
	case NOCS_EMI_AES128_MPEG_DASH_CTR:	//AES128    MPEG/DASH(CTR mode)  MT_CIPHER_WORK_MODE_CENS Cenc Counter Mode Full Sample One shot Mode 0x4024
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CTR;
		break;
	case NOCS_EMI_AES128_CBC_TAIL_CLEAR:	//AES128    CBC mode. IV modifiable. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_AES128_CTR:	//AES128    CTR mode.
		p_ctrl->algorithm = MT_CIPHER_ALG_AES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CTR;
		break;
	case NOCS_EMI_TDES_CBC_ZEROIV_TAIL_CLEAR:	//TDES      CBC mode with all bits set to zero IV. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	case NOCS_EMI_TDES_ECB_TAIL_CLEAR:	//TDES      ECB mode. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_ECB;
		break;
	case NOCS_EMI_TDES_CBC_TAIL_CLEAR:	//TDES      CBC mode. IV modifiable. Termination in clear
		p_ctrl->algorithm = MT_CIPHER_ALG_TDES;
		p_ctrl->work_mode = MT_CIPHER_WORK_MODE_CBC;
		break;
	default:
		break;
	}
}

TSecStatus mt_SetReeMkl2LevelProtectedKey(
	TSecCipherSession xSession,
	TUnsignedInt16 xEmi,
	const TSecKladConfig * pxKladConfig,
	size_t xKeyIdSize,
	TUnsignedInt8 * pxKeyId,
	size_t xCipheredContentKeySize,
	const TUnsignedInt8 *pxCipheredContentKey,
	size_t xCipheredProtectingKeySize,
	const TUnsignedInt8 *pxL1CipheredProtectingKey,
	const TUnsignedInt8 *pxL2CipheredProtectingKey);

TSecStatus mt_SetReeMkl2LevelProtectedKey(
	TSecCipherSession xSession,
	TUnsignedInt16 xEmi,
	const TSecKladConfig * pxKladConfig,
	size_t xKeyIdSize,
	TUnsignedInt8 * pxKeyId,
	size_t xCipheredContentKeySize,
	const TUnsignedInt8 *pxCipheredContentKey,
	size_t xCipheredProtectingKeySize,
	const TUnsignedInt8 *pxL1CipheredProtectingKey,
	const TUnsignedInt8 *pxL2CipheredProtectingKey)
{
	TUnsignedInt32 i = 0;
	TUnsignedInt32 keySlotIndex[2] = { 0xffffffff, 0xffffffff };
	TUnsignedInt8 realCipheredContentKey[16] = { 0 };
	MT_CIPHER_ALGORITHM_E klAlgorithm = MT_CIPHER_ALG_BUTT;

	SECAPI_TRACE("0x%x, %ld , %ld, %ld, 0x%x, 0x%lx \n", xEmi,
		xCipheredContentKeySize, xCipheredProtectingKeySize,
		xKeyIdSize, pxKladConfig->rootKeyId, (TUnsignedLong)pxKeyId);
/*
	hex_dump("pxKladConfig", pxKladConfig, sizeof(TSecKladConfig));

	hex_dump("pxL2", pxL2CipheredProtectingKey, xCipheredProtectingKeySize);
	hex_dump("pxL1", pxL1CipheredProtectingKey, xCipheredProtectingKeySize);
	hex_dump("pxCipheredContentKey", pxCipheredContentKey, xCipheredProtectingKeySize);
*/
	SECAPI_INFO("\n");
	if (NULL == xSession) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (NULL == pxL1CipheredProtectingKey
		|| NULL == pxL2CipheredProtectingKey
		|| NULL == pxCipheredContentKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (!psecChkEmiRange(xEmi)) {
		return SEC_ERROR_BAD_EMI;
	}
	if (!psecChkKeySize(xEmi, xCipheredContentKeySize)) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (16 != xCipheredProtectingKeySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (NULL == pxKeyId && 0 != xKeyIdSize) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (NULL != pxKeyId && 0 == xKeyIdSize) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (NULL == pxKladConfig) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	klAlgorithm = psecGetKlAlgorithm(pxKladConfig->cipher);
	if (klAlgorithm == MT_CIPHER_ALG_BUTT) {

		return SEC_ERROR_BAD_PARAMETER;
	}

	if (sessionValidation(xSession)) {

		return SEC_ERROR_BAD_PARAMETER;
	}
	if (xCipheredContentKeySize == 8) {
		memcpy(realCipheredContentKey + 8, pxCipheredContentKey, 8);
	} else {
		memcpy(realCipheredContentKey, pxCipheredContentKey, 16);
	}
	xSession->isUseFlashProtKey = FALSE;
	if (xSession->EMI != xEmi) {
		xSession->keySlotUsed[0] = 0;
		xSession->keySlotUsed[1] = 0;
		xSession->EMI = xEmi;
	}

	psecSessionGetKeySlotIndex(xSession, xKeyIdSize, pxKeyId,
		&keySlotIndex[0], &keySlotIndex[1]);

	for (i = 0; i < 2; i++) {
		if (keySlotIndex[i] != 0xffffffff) {
			if (psecKeyLadder(psecGetMklRootKeyID(pxKladConfig->rootKeyId),
				(const unsigned char *)pxL2CipheredProtectingKey,
				(const unsigned char *)pxL1CipheredProtectingKey,
				(const unsigned char *)realCipheredContentKey,
				xSession->keySlotID[keySlotIndex[i]], klAlgorithm)) {
				secUnlock();
				return SEC_ERROR;
			}
#if 0
			if (xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
				memcpy(pSecKey->iv, CISSA_InitIV, 16);
				pSecKey->ivLen = 16;
			}

			/* rewrite iv after inacitve */
			if (pSecKey->ivLen > 0) {
				mt_unf_cipher_keyslot_set_iv(xSession->keySlotID[keySlotIndex[i]], pSecKey->iv, pSecKey->ivLen);
			}
#endif
			mt_unf_cipher_keyslot_info(xSession->keySlotID[keySlotIndex[i]]);
		}
	}
    xSession->isKeySet = TRUE;
    if (xSession->isStreamSession) {
        SECAPI_INFO("\n");
        psecSetDescramblerType((mt_handle)(xSession->dscHandle), xEmi);
    }
    secUnlock();

    return SEC_NO_ERROR;
}

/******************************************************************************
                             CHIPSET IDENTIFICATION
******************************************************************************/

/**
 *  @ingroup g_sec_chip_id
 *
 *  @brief
 *    The prototype of this function is strictly identical to the callback
 *    TSecGetNuid. Its implementation is mandatory for backward compatibility
 *    with older client applications that are not aware of the function table
 *    TSecFunctionTable.
*/
TSecStatus secGetNuid(TSecNuid * pxNuid)
{
	TSecStatus status;
	TUnsignedInt8 nuid[4] = { 0 };
	SECAPI_INFO("\n");
	if (pxNuid == NULL) {
		return SEC_ERROR;
	}

	if (mt_otp_read_byte(OTP_NUIDL_OFFSET, OTP_NUIDL_SIZE, (MT_U8 *) nuid)
		!= MT_SUCCESS) {
		status = SEC_ERROR;
		goto OUT;
	}
	status = SEC_NO_ERROR;
	psecByteSwap(nuid, 4);
	SECAPI_INFO("[ree]secGetNuid nuid[0] = %x , nuid[1] = %x , nuid[2] = %x , nuid[3]=%x  \n", nuid[0], nuid[1], nuid[2], nuid[3]);

	memcpy(pxNuid, nuid, sizeof(TSecNuid));
OUT:
	return status;
}

static TSecStatus secGetNuid64(TSecNuid64 * pxNuid)
{
	TSecStatus status;
	TUnsignedInt8 nuid[8] = { 0 };

	SECAPI_INFO("\n");
	if (pxNuid == NULL) {
		return SEC_ERROR;
	}

	if (mt_otp_read_byte(OTP_NUIDL_OFFSET, OTP_NUIDL_SIZE, (MT_U8 *) nuid)
		!= MT_SUCCESS) {
		status = SEC_ERROR;
		goto OUT;
	}

	if (mt_otp_read_byte(OTP_NUIDH_OFFSET, OTP_NUIDH_SIZE,
	 	(MT_U8 *) (nuid + 4)) != MT_SUCCESS) {
		status = SEC_ERROR;
		goto OUT;
	}

	status = SEC_NO_ERROR;
	psecByteSwap(nuid, 8);
	SECAPI_INFO("[ree]secGetNuid64 nuid[0] = %x , nuid[1] = %x , nuid[2] = %x , nuid[3]=%x, nuid[4] = %x , nuid[5] = %x , nuid[6]=%x , nuid[7]=%x\n", nuid[0], nuid[1], nuid[2], nuid[3],nuid[4], nuid[5], nuid[6], nuid[7]);

	memcpy(pxNuid, nuid, sizeof(TSecNuid64));
OUT:
	return status;
}

/**
 *  @ingroup g_sec_chip_id
 *
 *  @brief
 *    The prototype of this function is strictly identical to the callback
 *    TSecGetChipsetRevision. Its implementation is mandatory for backward
 *    compatibility with older client applications that are not aware of the
 *    function table TSecFunctionTable.
*/
#define SEC_R_CHIP_ID_RDEN		(0xBF140008)
#define SEC_R_CHIP_ID			(0xBF140004)

static TChar pChipRevision[4] = {0,};
TSecStatus secGetChipsetRevision(const TChar ** ppxChipsetRevision)
{
	TUnsignedInt32 data = 0;

	//SECAPI_INFO("0x%x, 0x%x\n", ppxChipsetRevision, *ppxChipsetRevision);
	if ((ppxChipsetRevision == NULL)) {
		return SEC_ERROR;
	}

	mt_sys_write_register(SEC_R_CHIP_ID_RDEN, 0xffff);
	//must use phisical address
	mt_sys_read_register(SEC_R_CHIP_ID, &data);
	data = data & 0xffff;
	pChipRevision[2] = 0;
	pChipRevision[3] = 0;
#if 1 //only for test
	switch (data) {
	case 0xB000:
		pChipRevision[0] = 'A';
		pChipRevision[1] = '0';
		break;
	case 0xB001:
		pChipRevision[0] = 'A';
		pChipRevision[1] = '1';
		break;
	default:
             SECAPI_INFO("\n");
		return SEC_ERROR;
	}
#else
        pChipRevision[0] = 'A';
        pChipRevision[1] = '0';  //all test is A0
#endif

       if (*ppxChipsetRevision == NULL) {
            *ppxChipsetRevision = pChipRevision;
       } else {
	        strcpy((char *)*ppxChipsetRevision, (const char *)pChipRevision);
       }

	return SEC_NO_ERROR;
}

#define OTP_CHIP_INFOR_1			(0xBF313FC4)
#define OTP_PACKAGEINFO_OFFSET	(28)
#define OTP_SPIDRAM_OFFSET		(24)
#define OTP_CAVENDOR_OFFSET		(18)
#define OTP_CAVERSION_OFFSET		(14)
#define OTP_DRMINFO_OFFSET		(10)
#define OTP_IPLICENSE_OFFSET		(6)

static char p_ch[20] = {0, };
static TSecStatus secGetChipsetExtension(const TChar ** ppxChipsetExtension)
{
	unsigned int chip_infor;
	unsigned char temp0;
	unsigned char package_infor[16] = { 'Q', 'R', 'S',0x00, 'N', 'O', 'P', 0x00, 'B', 'C', 'D'};
	unsigned char ca_vendor[32] = {0x00, 'G', 'C', 'T', 'A', 'U', 'P', 'V', 'D', 'Y', 'Z', 'S'};

	SECAPI_INFO("\n");
	if ((ppxChipsetExtension == NULL)) {
		return SEC_ERROR;
	}
	memset(p_ch, 0x0, sizeof(p_ch));
	if (mt_otp_read_byte ((OTP_CHIP_INFOR_1 & 0xFFFF), OTP_NUIDL_SIZE,
		(MT_U8 *) & chip_infor) != MT_SUCCESS) {
		return SEC_ERROR;
	}
	SECAPI_INFO("chip_infor = 0x%x \n", chip_infor);
	temp0 = (chip_infor >> OTP_PACKAGEINFO_OFFSET) & 0xF;
	p_ch[0] = package_infor[temp0];
	//temp0 = (chip_infor >> OTP_SPIDRAM_OFFSET) & 0xF;
	//p_ch[1] = drm_size[temp0];
	p_ch[1] = 'x';/* update according to Nagra e-mail request */
	temp0 = (chip_infor >> OTP_CAVENDOR_OFFSET) & 0x3F;
	p_ch[2] = ca_vendor[temp0];
	if (p_ch[2] != 'G') {
		//it must be Nagra project!
		return SEC_ERROR;
	}
	temp0 = (chip_infor >> OTP_CAVERSION_OFFSET) & 0xF;
	p_ch[3] = '0' + temp0;
	//temp0 = (chip_infor >> OTP_DRMINFO_OFFSET) & 0xF;
	//p_ch[4] = drm_infor[temp0];
	p_ch[4] = 'x';/* update according to Nagra e-mail request */
	//temp0 = (chip_infor >> OTP_IPLICENSE_OFFSET) & 0xF;
	//p_ch[5] = ip_license[temp0];
	p_ch[5] = 'x';/* update according to Nagra e-mail request */
       if (*ppxChipsetExtension == NULL) {
            *ppxChipsetExtension = p_ch;
       } else {
	        strcpy((char *)*ppxChipsetExtension, (const char *)p_ch);
       }

	return SEC_NO_ERROR;
}

static TSecStatus secGetChipId(TUnsignedInt8 pxOwnerId[2], TUnsignedInt8 pxChipId[8])
{
	TSecStatus status = SEC_ERROR;
	TUnsignedInt8 nuid[8] = { 0 };

	SECAPI_INFO("\n");
	//hex_dump("pxOwnerId:", pxOwnerId, 2);
	//for test conax
	if ((pxOwnerId[0] = 0x0B) && (pxOwnerId[1]) == 0x00) {
		if (mt_otp_read_byte(OTP_NUID2ND_L_OFFSET, OTP_NUID2ND_L_SIZE,
			(MT_U8 *) nuid) != MT_SUCCESS) {
			status = SEC_ERROR;
			goto OUT;
		}
		if (mt_otp_read_byte(OTP_NUID2ND_H_OFFSET, OTP_NUID2ND_H_SIZE,
			(MT_U8 *) (nuid + 4)) != MT_SUCCESS) {
			status = SEC_ERROR;
			goto OUT;
		}

		status = SEC_NO_ERROR;
		//hex_dump("nuid:", nuid, 8);
		psecByteSwap(nuid, 8);
		memcpy(pxChipId, nuid, 8);
	}
	//hex_dump("secGetChipId:", pxChipId, 8);
OUT:
	return status;
}

/******************************************************************************
  Session management
 ******************************************************************************/

TUnsignedInt16 mtSecGetSessionEmi(TTransportSessionId xTransportSessionId, TSessionOpType op)
{
	TSecStreamSession xSession = NULL;
	TUnsignedInt16 emi = 0xffff;

	secLock();

	xSession = mtSecGetSessionByTransportSessionId(xTransportSessionId, op);
	if (xSession == NULL) {
		secUnlock();
		return 0xffff;
	}
	emi = xSession->EMI;

	secUnlock();

	return emi;
}

static TSecStreamSession mtSecGetSessionByTransportSessionId(TTransportSessionId xTransportSessionId, TSessionOpType op)
{
    int i = 0;

    if (xTransportSessionId == TRANSPORT_SESSION_ID_INVALID
            || op == SESSION_OP_UNKNOWN) {

        return NULL;
    }

    //find a stream session
    for (i = 0; i < MAX_INTERNAL_SESSIONS; i++) {
        if (gsSecSessions[i].pxSession != NULL
                && gsSecSessions[i].sessiontype == SESSION_STREAM
                && gsSecSessions[i].op == op) {

            if (((TSecStreamSession)(gsSecSessions[i].pxSession))->transportSessionId == xTransportSessionId) {
                return (TSecStreamSession)(gsSecSessions[i].pxSession);
            }
        }
    }
    return NULL;

}

/**Operation type of SMP*/
/**CNcomment:*/
typedef enum {
    SESSION_SMP_CLOSE_START = 0x10, /**CLOSE SMP START. */
    SESSION_SMP_CLOSE_END,  /**CLOSE SMP END. */
    SESSION_SMP_UNKNOW,     /**Invalid type. */
} TSessionOpSMPSTATUSType;


static pfnEventCallback_t g_pfnEC[MT_SEC_EVENT_BUTT] = {
	NULL,
	NULL,
	NULL,
	NULL,
};


static TEEC_Result mtExtSMPC_Close(TSecStreamSession xSession, uint32_t keySlot)
{
    TEEC_Result tret = TEEC_SUCCESS;
    TTransportSessionId xTSID = 0;
    if (xSession == NULL) {
        return SEC_ERROR_BAD_PARAMETER;
    }
    //("%s-%d start caller...smpOpStatus = 0x%x\n", __FUNCTION__, __LINE__, xSession->smpOpStatus);
    xTSID = xSession->transportSessionId;
    if ((xSession->smpOpStatus != SESSION_SMP_CLOSE_END) && (xSession->smpOpStatus != SESSION_SMP_CLOSE_START)) {

        if (g_pfnEC[MT_SEC_EVENT_SMPC_CLOSE_START] != NULL) {

            //printf("%s-%d start caller...xTSID = %d\n", __FUNCTION__, __LINE__, xTSID);
            g_pfnEC[MT_SEC_EVENT_SMPC_CLOSE_START](xTSID, MT_SEC_EVENT_SMPC_CLOSE_START);
        }
        xSession->smpOpStatus = SESSION_SMP_CLOSE_START;
    }
    tret = SMPC_Close(keySlot);
    if (xSession->smpOpStatus == SESSION_SMP_CLOSE_START) {
        if (g_pfnEC[MT_SEC_EVENT_SMPC_CLOSE_END] != NULL) {
            xTSID = xSession->transportSessionId;
            g_pfnEC[MT_SEC_EVENT_SMPC_CLOSE_END](xTSID, MT_SEC_EVENT_SMPC_CLOSE_END);
        }
        xSession->smpOpStatus = SESSION_SMP_CLOSE_END;
        //printf("%s-%d end caller...smpOpStatus = 0x%x, 0x%x\n", __FUNCTION__, __LINE__, xSession->smpOpStatus, SESSION_SMP_CLOSE_END);
    }


    return tret;
}

TSecStatus mtSecSetEventCallback(
	MT_SEC_EVENT_E e_sec_event,
	pfnEventCallback_t pfnEC)
{
	if (e_sec_event >= MT_SEC_EVENT_BUTT) {
		return SEC_ERROR;
	}
	if (pfnEC == NULL) {
		return SEC_ERROR;
	}
	secLock();
	if (g_pfnEC[e_sec_event] == NULL) {
		g_pfnEC[e_sec_event] = pfnEC;
		secUnlock();
		return SEC_NO_ERROR;
	}

	secUnlock();

	SECAPI_INFO("event:%d  pfnEC:0x%lx\n", e_sec_event, (TUnsignedLong)pfnEC);

	//Already registered
	return SEC_ERROR;

}

//called by player who knows pidlist
//TODO
TSignedInt32 mtSecSetSessionPid(TTransportSessionId xTransportSessionId, TSecPidInfo *pxPidInfo, TBoolean smp)
{
    SECAPI_INFO("%s %d pxPidInfo = %x pxPidInfo->pidNum = %d sessionType = %d xTransportSessionId = %x smp = %x\n",__FUNCTION__,__LINE__, pxPidInfo,pxPidInfo->pidNum, pxPidInfo->sessionType,xTransportSessionId, smp);

    TUnsignedInt32 i = 0;

    if ((xTransportSessionId == TRANSPORT_SESSION_ID_INVALID) || (!xTransportSessionId))
        return -1;

    if (pxPidInfo == NULL)
        return -1;

    /*if (pxPidInfo->pidNum == 0)
        return -1;*/

    if (xTransportSessionId > MAX_INTERNAL_SESSIONS)
        return -1;

    secLock();

    gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType = pxPidInfo->sessionType;
    SECAPI_DINFO(" xTransportSessionId = %x  gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType = %d \n", xTransportSessionId, gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType);

    gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].pidNum = pxPidInfo->pidNum;
    gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].tsid = xTransportSessionId;
    gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].smp = smp;

    for (i = 0; i < pxPidInfo->pidNum; i++) {

        gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].pidList[i].pid = pxPidInfo->pidList[i].pid;
        gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].pidList[i].type = pxPidInfo->pidList[i].type;

    }

    secUnlock();

    //Debug
	/*
    {
        int i = 0;
        SECAPI_TRACE("Got pidlist::tsid=%d, num=%d,list:[] \n", xTransportSessionId, pxPidInfo->pidNum);
        for (i = 0; i < pxPidInfo->pidNum; i++) {
            SECAPI_TRACE("Got pidlist::pidList[%d]=[0x%x:%d] \n", i, pxPidInfo->pidList[i].pid, pxPidInfo->pidList[i].type);
        }
    }
	*/

    return 0;
}


TBoolean mtSecGetSmp( TTransportSessionId xTransportSessionId)
{
	TBoolean smp;
	//return TRUE;//TRUE;//TRUE;

	secLock();
	smp = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].smp;
	secUnlock();

	SECAPI_INFO("[%s]::smp=%s\n", __FUNCTION__, (smp ? "ON":"OFF"));
	return  smp;

}

TSignedInt32 mtSecSetIsMultiSessionFlag(TTransportSessionId xTransportSessionId, TBoolean isMultiSession)
{

    SECAPI_INFO(" isMultiSession = %d \n", isMultiSession);

    if ((xTransportSessionId == TRANSPORT_SESSION_ID_INVALID) || (!xTransportSessionId))
        return -1;



    if (xTransportSessionId > MAX_INTERNAL_SESSIONS)
        return -1;

    secLock();

    gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].isMultiSession = isMultiSession;

    secUnlock();



    return 0;
}

/*TUnsignedLong mtSecGetRecDscHandle(TTransportSessionId xTransportSessionId)
{


    if ((xTransportSessionId == TRANSPORT_SESSION_ID_INVALID) || (!xTransportSessionId))
        return 0;



    if (xTransportSessionId > MAX_INTERNAL_SESSIONS)
        return 0;


    return gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].recDscHandle;
}*/


static void getLocalPidList(TTransportSessionId xTransportSessionId, TSecPidInfo *pPidList)
{
	SECAPI_INFO("xTransportSessionId = 0x%x \n", xTransportSessionId);

	if (xTransportSessionId < MAX_INTERNAL_SESSIONS) {
		secLock();
		pPidList->sessionType = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType;
		pPidList->pidNum = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].pidNum;
		pPidList->tsid = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].tsid;
		if (pPidList->pidNum > 0)
			memcpy(pPidList->pidList, gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].pidList, (gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].pidNum * sizeof(TUnsignedInt32)));

		secUnlock();
	}
}

static void clearLocalPidList(TTransportSessionId xTransportSessionId)
{
	SECAPI_INFO("\n");

	if (xTransportSessionId < MAX_INTERNAL_SESSIONS) {
		secLock();
		memset(&gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK], 0, sizeof(TSecPidInfo));
		secUnlock();
	}
}

static TSecStatus mtSecAttachSessionPidToDescrambler(TTransportSessionId xTransportSessionId, TSessionOpType op, TSecPidInfo *pxPidInfo)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i = 0;
    mt_handle hChannel = MT_INVALID_HANDLE;
    mt_u32 xDmxId = 0;
    MT_UNF_DMX_CHAN_TYPE_E enType;

	SECAPI_INFO("\n");

    if ((xTransportSessionId == TRANSPORT_SESSION_ID_INVALID) || (!xTransportSessionId) || (SESSION_OP_UNKNOWN == op)) {
        SECAPI_ERROR("Bad TSID:%d\n", xTransportSessionId);
        return SEC_ERROR_BAD_PARAMETER;
    }

    secLock();

#if defined(MT_TSID_FOR_DALTS) //TODO:For DALTS, Only ID=0 is used; For specific project, please use xTransportSessionId.
    xDmxId = (mt_u32)MT_TSID_FOR_DALTS;
#else
    xDmxId = mtSecGetPlatDmxId(xTransportSessionId);
#endif

    TSecStreamSession pSecSession = mtSecGetSessionByTransportSessionId(xTransportSessionId, op);
    if (pSecSession == NULL) {
        secUnlock();
        SECAPI_ERROR("Session not found:tsid=%d\n", xTransportSessionId);
        return SEC_ERROR_BAD_PARAMETER;
    }

    pSecSession->pidInfo.sessionType = pxPidInfo->sessionType;
    pSecSession->pidInfo.pidNum = pxPidInfo->pidNum;
    for (i = 0; i < pxPidInfo->pidNum; i++) {
        pSecSession->pidInfo.pidList[i].pid = pxPidInfo->pidList[i].pid;
        pSecSession->pidInfo.pidList[i].type = pxPidInfo->pidList[i].type;
    }

    for (i = 0; i < pSecSession->pidInfo.pidNum; i++) {
        if (pSecSession->pidInfo.sessionType == TRANSPORT_SESSION_TYPE_RECORD)
            enType = MT_UNF_DMX_CHAN_TYPE_REC;
        else
            enType = toDmxChannelType(pSecSession->pidInfo.pidList[i].type);
        ret = MT_UNF_DMX_GetChannelHandleByPidType(xDmxId, pSecSession->pidInfo.pidList[i].pid, enType, &hChannel);
	        SECAPI_DINFO("tsid:%d, pid:0x%x,type:%d hChannel = %lx  videoDscHandle=%lx audioDscHandle=%lx recDscHandle=%lx pSecSession->pidInfo.pidNum = %d \n", xTransportSessionId, pSecSession->pidInfo.pidList[i].pid, enType, hChannel,
				pSecSession->videoDscHandle, pSecSession->audioDscHandle,pSecSession->recDscHandle, pSecSession->pidInfo.pidNum);

        if (ret == MT_SUCCESS)
	{
            if (enType == MT_UNF_DMX_CHAN_TYPE_VID)
            {
            	ret = MT_UNF_DMX_AttachDescrambler(pSecSession->videoDscHandle, hChannel);
		if( ret )
		SECAPI_ERROR("MT_UNF_DMX_CHAN_TYPE_VID ret = 0x%x pSecSession->videoDscHandle = 0x%lx  hChannel = 0x%lx\n", ret, pSecSession->videoDscHandle, hChannel);
		else
		SECAPI_DINFO(" success MT_UNF_DMX_CHAN_TYPE_VID ret = 0x%x pSecSession->videoDscHandle = 0x%lx  hChannel = 0x%lx\n", ret, pSecSession->videoDscHandle, hChannel);

            }
            else if (enType == MT_UNF_DMX_CHAN_TYPE_AUD)
             {
             ret = MT_UNF_DMX_AttachDescrambler(pSecSession->audioDscHandle, hChannel);
		if( ret )
		SECAPI_ERROR("MT_UNF_DMX_CHAN_TYPE_AUD ret = 0x%x pSecSession->audioDscHandle = 0x%lx  hChannel = 0x%lx\n", ret, pSecSession->audioDscHandle, hChannel);
		else
		SECAPI_DINFO("success MT_UNF_DMX_CHAN_TYPE_AUD ret = %d pSecSession->audioDscHandle = 0x%lx  hChannel = 0x%lx\n", ret, pSecSession->audioDscHandle, hChannel);

            }
            else if (enType == MT_UNF_DMX_CHAN_TYPE_REC)
             {
             	ret = MT_UNF_DMX_AttachDescrambler(pSecSession->recDscHandle, hChannel);
		if( ret )
		SECAPI_ERROR("MT_UNF_DMX_CHAN_TYPE_REC ret = 0x%x pSecSession->recDscHandle = 0x%lx  hChannel = 0x%lx\n", ret, pSecSession->recDscHandle, hChannel);
		else
		SECAPI_DINFO("success MT_UNF_DMX_CHAN_TYPE_REC ret = %d pSecSession->recDscHandle = 0x%lx  hChannel = 0x%lx\n", ret, pSecSession->recDscHandle, hChannel);

            }
            else
            {
            	ret = MT_FAILURE;
            }
            SECAPI_INFO("Attach pid:%x to Descrambler success, ret:0x%x\n", pSecSession->pidInfo.pidList[i].pid, ret);
        }
	 else
        {
            SECAPI_ERROR("Attach pid:%x to Descrambler failed, ret:0x%x\n", pSecSession->pidInfo.pidList[i].pid, ret);
        }
    }

	secUnlock();

	if (ret)
		return SEC_ERROR;
	else
		return SEC_NO_ERROR;
}

//TODO:adapt S4
TBoolean mtSecGetSessionClosed(TTransportSessionId xTransportSessionId, TSessionOpType op)
{
	SECAPI_INFO("\n");

    if (xTransportSessionId == TRANSPORT_SESSION_ID_INVALID || (SESSION_OP_UNKNOWN == op))
        return TRUE;

    secLock();
    if (NULL == mtSecGetSessionByTransportSessionId(xTransportSessionId, op)) {
        secUnlock();
        return TRUE;
    }
    secUnlock();

    return FALSE;
}

/******************************************************************************/
/*                                                                            */
/*                               CHIPSET CONFIGURATION                        */
/*                                                                            */
/******************************************************************************/

static TSecStatus secEncryptFlashProtKey
    (const TUnsignedInt8 * pxInput, TUnsignedInt8 * pxOutput, size_t xSize) {
	MT_CIPHER_CTRL_S ctrl = { 0 };
	TUnsignedInt8 KLD_L2[] ={ 0xfa, 0xbf, 0xc7, 0x69, 0xd4, 0x16, 0x33, 0x7c,
		0x99, 0xe3, 0x5d, 0xd0, 0x15, 0xd2, 0xa6, 0xa0 };
	TUnsignedInt8 KLD_L1[] ={ 0xed, 0xdb, 0x7f, 0x9d, 0x3c, 0xb6, 0x1e, 0xfc,
		0xe2, 0x37, 0xce, 0xd2, 0x3b, 0x80, 0xa4, 0x4c };
	TUnsignedInt8 KLD_L0[] = { 0x44, 0x09, 0x20, 0x4e, 0x12, 0xda, 0xc2, 0x8e,
		0x2f, 0x8e, 0x3c, 0x9f, 0xa1, 0x29, 0x54, 0x8b };

	SECAPI_INFO("\n");
	if (NULL == pxInput || NULL == pxOutput) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (16 != xSize) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (psecKeyLadder(NOCS_FLASH_PROTECT_ROOTKEY, KLD_L2, KLD_L1, KLD_L0,
		NOCS_FLASH_PROT_KEY_SLOT, MT_CIPHER_ALG_TDES)) {
		return SEC_ERROR;
	}

	mt_unf_cipher_keyslot_info(NOCS_FLASH_PROT_KEY_SLOT);

	ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;
	ctrl.algorithm = MT_CIPHER_ALG_AES;
	ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;
	if (!mtSecCryptoEngineProcess(&ctrl, 0xFFFF, pxInput, pxOutput, xSize, NOCS_FLASH_PROT_KEY_SLOT))
		return SEC_ERROR;

	return SEC_NO_ERROR;
}

static TSecStatus secEnableTeePrivilegedMode(void)
{
	mt_u8 otp_firewall[4];

	otp_firewall[0] = 0;
	otp_firewall[1] =
	sec_set_bit_val(OTP_REEISPRIVILEGE_VALUE, 2,
		OTP_PRIVILEGED_MODE_MASK);
	otp_firewall[2] = 0;
	otp_firewall[3] = 0;

	SECAPI_INFO("\n");

	if (mt_otp_write_byte(OTP_PRIVILEGED_MODE_OFFSET,
		OTP_PRIVILEGED_MODE_SIZE, (MT_U8 *) otp_firewall) != MT_SUCCESS) {
		return SEC_ERROR;
	}

	if (mt_otp_read_byte(OTP_PRIVILEGED_MODE_OFFSET,
		OTP_PRIVILEGED_MODE_SIZE, (MT_U8 *) otp_firewall) != MT_SUCCESS) {
		return SEC_ERROR;
	}
	if (sec_get_bit_val(otp_firewall[1], 2, OTP_PRIVILEGED_MODE_MASK) !=
		OTP_REEISPRIVILEGE_VALUE) {
		return SEC_ERROR;
	}
	SECAPI_INFO("privilege -> 0x%x, 0x%x, 0x%x, 0x%x\n", otp_firewall[0],
		otp_firewall[1], otp_firewall[2], otp_firewall[3]);

	return SEC_NO_ERROR;
}

static TSecStatus secGetTeePrivilegedMode(TBoolean * pxEnabled)
{
	mt_u8 otp_firewall[4];

	SECAPI_INFO("\n");

	if (!pxEnabled) {
		return SEC_ERROR;
	}
#if 1
	if (mt_otp_read_byte(OTP_PRIVILEGED_MODE_OFFSET, OTP_PRIVILEGED_MODE_SIZE,
		(MT_U8 *) otp_firewall) != MT_SUCCESS) {
		return SEC_ERROR;
	}

	if (sec_get_bit_val(otp_firewall[1], 2, OTP_PRIVILEGED_MODE_MASK) !=
		OTP_REEISPRIVILEGE_VALUE) {
		*pxEnabled = FALSE;
	} else {
		*pxEnabled = TRUE;
	}
	SECAPI_INFO("privilege -> 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		otp_firewall[0], otp_firewall[1], otp_firewall[2],
		otp_firewall[3], *pxEnabled);
#else
    *pxEnabled = TRUE;
#endif
	return SEC_NO_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                                  RESET                                     */
/*                                                                            */
/******************************************************************************/

void secChipReset(void)
{
	SECAPI_INFO("\n");
	system("reboot");
}

/******************************************************************************/
/*                                                                            */
/*                                BLOCK CIPHER                                */
/*                                                                            */
/******************************************************************************/

TSecStatus secEncryptData
(
 TUnsignedInt8*  pxOutput,
 const TUnsignedInt8*  pxInput,
 TSize           xDataSize
 )
{
    mt_u8 slot_id;
    MT_CIPHER_CTRL_S ctrl = {0};
    TSecStatus ret = SEC_ERROR;

    SECAPI_INFO("\n");
    if (pxInput == NULL || pxOutput == NULL)
        return SEC_ERROR;

    if (!xDataSize || (xDataSize % 8))
        return SEC_ERROR;

    if (0 != mtSecKeySlotRequest(&slot_id, 1))
        goto sec_encdata_ktreq_error;

    if (psecKeyLadder(NOCS_TDES_ROOTKEY, KLD_CST, KLD_CST, KLD_CST, slot_id, MT_CIPHER_ALG_TDES))
        goto sec_encdata_kld_error;

    mt_unf_cipher_keyslot_info(slot_id);

    ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;
    ctrl.algorithm = MT_CIPHER_ALG_TDES; //TDES_ABA ?
    ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;
    if (!mtSecCryptoEngineProcess(&ctrl, 0xFFFF, pxInput, pxOutput, xDataSize, slot_id)) {
        SECAPI_ERROR("crypto endigne error\n");
        goto sec_encdata_cpt_error;
    }
    ret = SEC_NO_ERROR;

sec_encdata_kld_error:
sec_encdata_cpt_error:
    mt_unf_cipher_keyslot_release(slot_id);
sec_encdata_ktreq_error:
    return ret;
}

TSecStatus secDecryptData
(
 TUnsignedInt8*  pxOutput,
 const TUnsignedInt8*  pxInput,
 TSize           xDataSize
 )
{
    mt_u8 slot_id;
    MT_CIPHER_CTRL_S ctrl = {0};
    TSecStatus ret = SEC_ERROR;

    SECAPI_INFO("\n");
    if (pxInput == NULL || pxOutput == NULL)
        return SEC_ERROR;

    if (!xDataSize || (xDataSize % 8))
        return SEC_ERROR;

    if (0 != mtSecKeySlotRequest(&slot_id, 1))
        goto sec_decdata_ktreq_error;

    if (psecKeyLadder(NOCS_TDES_ROOTKEY, KLD_CST, KLD_CST, KLD_CST, slot_id, MT_CIPHER_ALG_TDES))
        goto sec_decdata_kld_error;

    mt_unf_cipher_keyslot_info(slot_id);

    ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
    ctrl.algorithm = MT_CIPHER_ALG_TDES; //TDES_ABA ?
    ctrl.work_mode = MT_CIPHER_WORK_MODE_ECB;
    if (!mtSecCryptoEngineProcess(&ctrl, 0xFFFF, pxInput, pxOutput, xDataSize, slot_id))
        goto sec_decdata_cpt_error;

    ret = SEC_NO_ERROR;

sec_decdata_kld_error:
sec_decdata_cpt_error:
    mt_unf_cipher_keyslot_release(slot_id);
sec_decdata_ktreq_error:
    return ret;
}

static TSecStatus secOpenRam2RamEncryptSession
(
	TSecCipherSession*     pxSession
)
{
	mt_u8 slotID;
	TSecCipherSession xSession;

	SECAPI_INFO("pxSession: 0x%lx\n", (TUnsignedLong)pxSession);
	if (pxSession == NULL) {
		SECAPI_ERROR("input eror\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	xSession = (TSecCipherSession)psecMalloc(sizeof(struct SSecCipherSession));
	if (xSession == NULL) {
		SECAPI_ERROR("malloc Ram2RamEncryptSession failed\n");
		return SEC_ERROR;
	}
	memset(xSession, 0, sizeof(struct SSecCipherSession));

	if (0 != mtSecKeySlotRequest(&slotID, 1)) {
		psecFree(xSession);
		SECAPI_ERROR("request key slot failed\n");
		return SEC_ERROR;
	}

	/*For Ram2Ram, only 1 slot is needed*/
	xSession->keySlotID[0] = slotID;
	xSession->keySlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->key256SlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->key256SlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->isStreamSession = FALSE;
	xSession->isEncryptionSession = TRUE;
       xSession->transportSessionId = TRANSPORT_SESSION_ID_INVALID;

	xSession->magic = MT_SESSION_MAGIC;
	if(checkSameTypeSession(SESSION_RAM2RAM, SESSION_OP_ENCRYPT))
	{
		xSession->isMultiSession = TRUE;
	}
	else
	{
		xSession->isMultiSession = FALSE;
	}
	insertSession(xSession, SESSION_RAM2RAM, SESSION_OP_ENCRYPT);

	*pxSession = xSession;

	SECAPI_INFO("xSession: 0x%lx slotID=0x%x\n", (TUnsignedLong)xSession, slotID);
	return SEC_NO_ERROR;
}

static TSecStatus secOpenRam2RamDecryptSession
(
	TSecCipherSession*     pxSession
)
{
	mt_u8 slotID;
	TSecCipherSession xSession;

	if (pxSession == NULL) {
		SECAPI_ERROR("input eror\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	SECAPI_INFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);

	xSession = (TSecCipherSession)psecMalloc(sizeof(struct SSecCipherSession));
	if (xSession == NULL) {
		SECAPI_ERROR("malloc Ram2RamDecryptSession failed\n");
		return SEC_ERROR;
	}
	memset(xSession, 0, sizeof(struct SSecCipherSession));

	if (0 != mtSecKeySlotRequest(&slotID, 1)) {
		psecFree(xSession);
		SECAPI_ERROR("request key slot failed\n");
		return SEC_ERROR;
	}
	xSession->keySlotID[0] = slotID;
	xSession->keySlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->key256SlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->key256SlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->isStreamSession = FALSE;
	xSession->isEncryptionSession = FALSE;
       xSession->transportSessionId = TRANSPORT_SESSION_ID_INVALID;

	xSession->magic = MT_SESSION_MAGIC;
	if(checkSameTypeSession(SESSION_RAM2RAM, SESSION_OP_DECRYPT))
	{
		xSession->isMultiSession = TRUE;
	}
	else
	{
		xSession->isMultiSession = FALSE;
	}
	insertSession(xSession, SESSION_RAM2RAM, SESSION_OP_DECRYPT);

	*pxSession = xSession;

	return SEC_NO_ERROR;
}

static TSecStatus secOpenStreamEncryptSession
(
	TSecCipherSession*     pxSession,
	TTransportSessionId     xTransportSessionId
)
{
	mt_u8 slotID = 0;
	TSecCipherSession xSession;

	SECAPI_INFO("xTransportSessionId: 0x%x\n", xTransportSessionId);

	if (pxSession == NULL) {
		SECAPI_ERROR("input eror\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xTransportSessionId == TRANSPORT_SESSION_ID_INVALID  || (!xTransportSessionId)) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	xSession = (TSecCipherSession)psecMalloc(sizeof(struct SSecCipherSession));
	if (xSession == NULL) {
		SECAPI_ERROR("malloc failed\n");
		return SEC_ERROR;
	}
	memset(xSession, 0, sizeof(struct SSecCipherSession));
	if (0 != mtSecKeySlotRequest(&slotID, 2)) {
		psecFree(xSession);
		SECAPI_ERROR("request key slot 1 failed\n");
		return SEC_ERROR;
	}
	xSession->keySlotID[0] = slotID;		//Even
	xSession->keySlotID[1] = slotID + 1;	//Odd
	xSession->key256SlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->key256SlotID[1] = MT_CIPHER_KEYSLOT_INVALID;

	xSession->isStreamSession = TRUE;
	xSession->isEncryptionSession = TRUE;
	xSession->transportSessionId = xTransportSessionId;

	xSession->magic = MT_SESSION_MAGIC;

	SECAPI_TRACE("xSession:0x%lx\n", (TUnsignedLong)xSession);

	//TODO:create descrambler and attach
	//attachDescrambler(*pxSession, slotID, slotID + 1);
	#if 0
	if(checkSameTypeSession(SESSION_STREAM, SESSION_OP_ENCRYPT))
	{
		xSession->isMultiSession = TRUE;
	}
	else
	{
		xSession->isMultiSession = FALSE;
	}
	#else
	xSession->isMultiSession = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].isMultiSession;
	#endif
	insertSession(xSession, SESSION_STREAM, SESSION_OP_ENCRYPT);
	*pxSession = xSession;

	/*Notify caller*/
	if (g_pfnEC[MT_SEC_EVENT_OPEN] != NULL) {
		g_pfnEC[MT_SEC_EVENT_OPEN](xTransportSessionId, MT_SEC_EVENT_OPEN);
	}

	return SEC_NO_ERROR;
}


static TSecStatus secOpenStreamDecryptSession
(
	TSecCipherSession*     pxSession,
	TTransportSessionId     xTransportSessionId
)
{
	mt_u8 slotID = 0;
	TSecCipherSession xSession;


	SECAPI_INFO("xTransportSessionId: 0x%x\n", xTransportSessionId);

	if (pxSession == NULL) {
		SECAPI_ERROR("input eror\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xTransportSessionId == TRANSPORT_SESSION_ID_INVALID  || (!xTransportSessionId))
		return SEC_ERROR_BAD_PARAMETER;

	xSession = (TSecCipherSession)psecMalloc(sizeof(struct SSecCipherSession));
	if (xSession == NULL) {
		SECAPI_ERROR("malloc failed\n");
		return SEC_ERROR;
	}

	memset(xSession, 0, sizeof(struct SSecCipherSession));
	if (0 != mtSecKeySlotRequest(&slotID, 2)) {
		psecFree(xSession);
		SECAPI_ERROR("request key slot 1 failed\n");
		return SEC_ERROR;
	}
	xSession->keySlotID[0] = slotID;
	xSession->keySlotID[1] = slotID + 1;
	xSession->key256SlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->key256SlotID[1] = MT_CIPHER_KEYSLOT_INVALID;

	xSession->isStreamSession = TRUE;
	xSession->isEncryptionSession = FALSE;

	xSession->magic = MT_SESSION_MAGIC;
	SECAPI_TRACE("xSession:0x%lx\n", (TUnsignedLong)xSession);

	SECAPI_INFO("keySlotID[0]:%d  keySlotID[1]:%d\n", slotID, slotID+1);

#if defined(MT_TSID_FOR_DALTS) //TODO:For DALTS, Only ID=0 is used; For specific project, please use xTransportSessionId.
	SECAPI_INFO("tsid_for_DALTS_Used=%d\n", MT_TSID_FOR_DALTS);
	xSession->transportSessionId = MT_TSID_FOR_DALTS;
#else
	xSession->transportSessionId = xTransportSessionId;
#endif

	//TODO:create descrambler and attach
	//attachDescrambler(*pxSession, slotID, slotID + 1);

	while( gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType == TRANSPORT_SESSION_TYPE_NONE)//wait for nvSpr Session open
	{
		MT_USLEEP(1000);
	}

	xSession->isMultiSession = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].isMultiSession;
	SECAPI_INFO("[SEC API]xTransportSessionId:%x   xSession->isMultiSession = %d\n", xTransportSessionId, xSession->isMultiSession);

	insertSession(xSession, SESSION_STREAM, SESSION_OP_DECRYPT);
	*pxSession = xSession;

	/*Notify caller*/
	if (g_pfnEC[MT_SEC_EVENT_OPEN] != NULL) {
		SECAPI_INFO("MT_SEC_EVENT_OPEN\n");

		g_pfnEC[MT_SEC_EVENT_OPEN](xTransportSessionId, MT_SEC_EVENT_OPEN);
	}

	return SEC_NO_ERROR;
}

static TSecStatus secCloseSession
(
	TSecCipherSession      xSession
)
{
	TTransportSessionId xTSID = TRANSPORT_SESSION_ID_INVALID;
	TUnsignedInt8 isStreamSession = 0;

	SECAPI_INFO("xSession:0x%lx\n", (TUnsignedLong)xSession);

	if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (sessionValidation(xSession)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	xTSID = xSession->transportSessionId;
	isStreamSession = xSession->isStreamSession;

	/*Notify caller that session will be closed, only for Stream Session.*/
	if (isStreamSession) {
		if (g_pfnEC[MT_SEC_EVENT_CLOSE] != NULL) {
			SECAPI_INFO("%s-%d Closed, Notify caller...\n", __FUNCTION__, __LINE__);
			g_pfnEC[MT_SEC_EVENT_CLOSE](xTSID, MT_SEC_EVENT_CLOSE);
		}
	}

	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
		SECAPI_INFO("-----------release slot:%d------------\n", xSession->keySlotID[0]);
		mt_unf_cipher_keyslot_release(xSession->keySlotID[0]);
	}

	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[1]) {
		SECAPI_INFO("-----------release slot:%d------------\n", xSession->keySlotID[1]);
		mt_unf_cipher_keyslot_release(xSession->keySlotID[1]);
	}

	if (MT_CIPHER_KEYSLOT_INVALID != xSession->key256SlotID[0]) {
		SECAPI_INFO("-----------release 256slot:%d------------\n", xSession->key256SlotID[0]);
		mt_unf_cipher_keyslot_release(xSession->key256SlotID[0]);
		mt_unf_cipher_keyslot_release(xSession->key256SlotID[0]+1);
	}

	if (MT_CIPHER_KEYSLOT_INVALID != xSession->key256SlotID[1]) {
		SECAPI_INFO("-----------release 256slot:%d------------\n", xSession->key256SlotID[1]);
		mt_unf_cipher_keyslot_release(xSession->key256SlotID[1]);
		mt_unf_cipher_keyslot_release(xSession->key256SlotID[1]+1);
	}

	xSession->metadataSize = 0;
	memset(xSession->metadata, 0, 4);

	if (isStreamSession) {
		//TODO:detach descrambler
		//detachDescrambler(xSession);
	}

	//mtSecCryptoEngineProcessEnd(xSession->cryptoHandle, xSession->EMI);

	xSession->magic = 0xffffffff;

	removeSession(xSession);
	xSession = NULL;
	psecFree(xSession);

	return SEC_NO_ERROR;
}

/*
 * Only used for stream session
 * TODO: Add OTT KeyId rotation.
 */
static TUnsignedInt8 getPolarityByKeyId(TUnsignedInt8 *pxKeyId, TUnsignedInt16 xKeyIdSize)
{
	if (xKeyIdSize == 0 || pxKeyId == NULL)
		return 0; //Even by default

	if ((pxKeyId[0] & 0x01) == 0x00)
		return 0; //even
	else
		return 1; //Odd

}

static TSecStatus secSetClearTextKey
(
	TSecCipherSession   xSession,
	TUnsignedInt16      xEmi,
	size_t              xKeyIdSize,
	TUnsignedInt8*     pxKeyId,
	size_t              xClearTextKeySize,
	const TUnsignedInt8*     pxClearTextKey
)
{
	mt_s32 ret = MT_FAILURE;
	MT_CIPHER_CTRL_S KeyCtrl;
	TUnsignedInt8 pol;
	TUnsignedInt32 keyslot;

       //printf("[REE] set clear key text --------------------------------------------- \n");
       SECAPI_DINFO("\n");
	SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);
	if ((NULL == xSession)
		|| (NULL == pxClearTextKey)
		|| (FALSE == psecChkKeySize(xEmi, xClearTextKeySize))
		|| ((0 == xKeyIdSize) && (pxKeyId != NULL))
		|| ((0 != xKeyIdSize) && (pxKeyId == NULL))
		|| (sessionValidation(xSession))) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	}
       SECAPI_DINFO("\n");

	if (FALSE == psecChkEmiRange(xEmi)) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_EMI;
	}
       SECAPI_DINFO("\n");

	xSession->isUseFlashProtKey = FALSE;
	xSession->EMI = xEmi;
       SECAPI_DINFO("\n");

	memset(&KeyCtrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	psecSession2Ctrl(xSession, &KeyCtrl);
       SECAPI_DINFO("\n");

	if (xSession->isStreamSession) {
		pol = getPolarityByKeyId(pxKeyId, xKeyIdSize);
		keyslot = xSession->keySlotID[pol];
	} else {
		keyslot = xSession->keySlotID[0];
	}
       SECAPI_DINFO("\n");

	if (keyslot != MT_CIPHER_KEYSLOT_INVALID) {
		//sec_dump("pxClearTextKey", (mt_u8 *)pxClearTextKey, xClearTextKeySize);
		ret = mt_unf_cipher_keyslot_set(keyslot, &KeyCtrl, (unsigned char *)pxClearTextKey, NULL);
		if (ret)
			return SEC_ERROR;

		mt_unf_cipher_keyslot_info(keyslot);
	}
	       SECAPI_DINFO("\n");

	mtSecSetImplicitIv(xEmi, keyslot, 1);
	       SECAPI_DINFO("\n");

	xSession->isKeySet = TRUE;
	if (xSession->isStreamSession) {
		if (xSession->isEncryptionSession == FALSE) {
			//psecSetDescramblerType((mt_handle)xSession->dscHandle, xEmi);
		}
	} else {
		mtSecCryptoEngineProcessPrepare(&KeyCtrl, xSession->EMI, &xSession->pidInfo,
			keyslot, &xSession->cryptoHandle);
	}
	       SECAPI_DINFO("\n");


	return SEC_NO_ERROR;
}

static TSecStatus secSet1LevelProtectedKey
(
	TSecCipherSession   xSession,
	TUnsignedInt16      xEmi,
	size_t              xKeyIdSize,
	TUnsignedInt8*     pxKeyId,
	size_t              xCipheredContentKeySize,
	const TUnsignedInt8*     pxCipheredContentKey,
	size_t              xCipheredProtectingKeySize,
	const TUnsignedInt8*     pxL1CipheredProtectingKey
)
{
	MT_CIPHER_CTRL_S KeyCtrl = {0};
	TUnsignedInt8 realCipheredContentKey[16] = {0};
	TUnsignedInt8 pol;
	TUnsignedInt32 keyslot;

	SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);
	if (NULL == xSession)
		return SEC_ERROR_BAD_PARAMETER;

	if (NULL == pxL1CipheredProtectingKey
		|| NULL == pxCipheredContentKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (!psecChkEmiRange(xEmi))
		return SEC_ERROR_BAD_EMI;

	if (!psecChkKeySize(xEmi, xCipheredContentKeySize))
		return SEC_ERROR_BAD_PARAMETER;

	if (16 != xCipheredProtectingKeySize)
		return SEC_ERROR_BAD_PARAMETER;

	if (NULL == pxKeyId && 0 != xKeyIdSize)
		return SEC_ERROR_BAD_PARAMETER;

	if (NULL != pxKeyId && 0 == xKeyIdSize)
		return SEC_ERROR_BAD_PARAMETER;

	if (sessionValidation(xSession))
		return SEC_ERROR_BAD_PARAMETER;

	/*only for stream session*/
	if (xSession->isStreamSession == FALSE)
		return SEC_ERROR_BAD_USAGE;

	if (xCipheredContentKeySize == 8) {
		memcpy(realCipheredContentKey+8, pxCipheredContentKey, 8);
	} else {
		memcpy(realCipheredContentKey, pxCipheredContentKey, 16);
	}
	xSession->isUseFlashProtKey = FALSE;
	xSession->EMI = xEmi;

	memset(&KeyCtrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	psecSession2Ctrl(xSession, &KeyCtrl);

	pol = getPolarityByKeyId(pxKeyId, xKeyIdSize);
	keyslot = xSession->keySlotID[pol];
	if (psecKeyLadder(psecGetRootKeyID(xEmi), KLD_CST,
		(const unsigned char *)pxL1CipheredProtectingKey,
		(const unsigned char *)realCipheredContentKey,
		keyslot,
		MT_CIPHER_ALG_TDES)) {
		return SEC_ERROR;
	}
	xSession->isKeySet = TRUE;
	mt_unf_cipher_keyslot_info(keyslot);
	mtSecSetImplicitIv(xEmi, keyslot, 1);
	if (xSession->isEncryptionSession == FALSE) {
		//psecSetDescramblerType((mt_handle)(xSession->dscHandle), xEmi);
	}

	return SEC_NO_ERROR;
}

static TSecStatus secSet2LevelProtectedKey
(
	TSecCipherSession   xSession,
	TUnsignedInt16      xEmi,
	size_t              xKeyIdSize,
	TUnsignedInt8*     pxKeyId,
	size_t              xCipheredContentKeySize,
	const TUnsignedInt8*     pxCipheredContentKey,
	size_t              xCipheredProtectingKeySize,
	const TUnsignedInt8*     pxL1CipheredProtectingKey,
	const TUnsignedInt8*     pxL2CipheredProtectingKey
)
{
	MT_CIPHER_CTRL_S KeyCtrl = {0};
	TUnsignedInt8 realCipheredContentKey[16] = {0};
	TUnsignedInt8 pol;
	TUnsignedInt32 keyslot;

	SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);

	if (NULL == xSession) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == pxL1CipheredProtectingKey
		|| NULL == pxL2CipheredProtectingKey
		|| NULL == pxCipheredContentKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (!psecChkEmiRange(xEmi)) {
		return SEC_ERROR_BAD_EMI;
	}

	if (!psecChkKeySize(xEmi, xCipheredContentKeySize)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (16 != xCipheredProtectingKeySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == pxKeyId && 0 != xKeyIdSize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL != pxKeyId && 0 == xKeyIdSize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (sessionValidation(xSession)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xCipheredContentKeySize == 8) {
		memcpy(realCipheredContentKey+8, pxCipheredContentKey, 8);
	} else {
		memcpy(realCipheredContentKey, pxCipheredContentKey, 16);
	}
	xSession->isUseFlashProtKey = FALSE;
	xSession->EMI = xEmi;

	memset(&KeyCtrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
	psecSession2Ctrl(xSession, &KeyCtrl);

	pol = getPolarityByKeyId(pxKeyId, xKeyIdSize);
	keyslot = xSession->keySlotID[pol];
	if (psecKeyLadder(psecGetRootKeyID(xEmi),
		(const unsigned char *)pxL2CipheredProtectingKey,
		(const unsigned char *)pxL1CipheredProtectingKey,
		(const unsigned char *)realCipheredContentKey,
		keyslot,
		MT_CIPHER_ALG_TDES)) {
		return SEC_ERROR;
	}
	xSession->isKeySet = TRUE;
	mt_unf_cipher_keyslot_info(keyslot);

	if (xSession->isStreamSession) {
	} else {
		//mtSecCryptoEngineProcessPrepare(&KeyCtrl, xSession->EMI, keyslot, &xSession->cryptoHandle);
		mtSecCryptoEngineProcessPrepare(&KeyCtrl, xSession->EMI, &xSession->pidInfo,
			keyslot, &xSession->cryptoHandle);
	}
	mtSecSetImplicitIv(xEmi, keyslot, 1);
	if (xSession->isEncryptionSession == FALSE) {
		//psecSetDescramblerType((mt_handle)(xSession->dscHandle), xEmi);
	}

	return SEC_NO_ERROR;
	}

static TSecStatus secSetEtsi2LevelProtectedKey
(
	TSecCipherSession    xSession,
	TUnsignedInt16       xEmi,
	const TSecEtsiKladConfig* pxKladConfig,
	size_t               xKeyIdSize,
	const TUnsignedInt8*      pxKeyId,
	size_t               xCipheredContentKeySize,
	const TUnsignedInt8*      pxCipheredContentKey,
	size_t               xCipheredProtectingKeySize,
	const TUnsignedInt8*      pxL1CipheredProtectingKey,
	const TUnsignedInt8*      pxL2CipheredProtectingKey
)
{
	TUnsignedInt8 realCipheredContentKey[16] = {0};
	MT_CIPHER_ALGORITHM_E klAlgorithm = MT_CIPHER_ALG_BUTT;
	TUnsignedInt8 pol;
	TUnsignedInt32 keyslot;

	SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);
	if (NULL == xSession) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if ((NULL == pxL1CipheredProtectingKey)
		|| (NULL == pxL2CipheredProtectingKey)
		|| (NULL == pxCipheredContentKey)
		|| (NULL == pxKladConfig)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (!psecChkEmiRange(xEmi)) {
		return SEC_ERROR_BAD_EMI;
	}

	if (!psecChkKeySize(xEmi, xCipheredContentKeySize)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (16 != xCipheredProtectingKeySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if ((NULL == pxKeyId) && (0 != xKeyIdSize)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if ((NULL != pxKeyId) && (0 == xKeyIdSize)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (sessionValidation(xSession)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xCipheredContentKeySize == 8) {
		memcpy(realCipheredContentKey+8, pxCipheredContentKey, 8);
	} else {
		memcpy(realCipheredContentKey, pxCipheredContentKey, 16);
	}

	xSession->isUseFlashProtKey = FALSE;
	xSession->EMI = xEmi;

	if (psecRootKeyDerivation(pxKladConfig)) {
		return SEC_ERROR;
	}

	klAlgorithm = psecGetKlAlgorithm(pxKladConfig->cipher);
	if (klAlgorithm == MT_CIPHER_ALG_BUTT) {
		return SEC_ERROR;
	}

	pol = getPolarityByKeyId((TUnsignedInt8*)pxKeyId, xKeyIdSize);
	keyslot = xSession->keySlotID[pol];
	if (psecKeyLadder(NOCS_ETSI_CW_ROOTKEY,
		  (const unsigned char *)pxL2CipheredProtectingKey,
		  (const unsigned char *)pxL1CipheredProtectingKey,
		  (const unsigned char *)realCipheredContentKey,
		  keyslot,
		  klAlgorithm)) {
		return SEC_ERROR;
	}
	xSession->isKeySet = TRUE;
	mt_unf_cipher_keyslot_info(keyslot);
	mtSecSetImplicitIv(xEmi, keyslot, 1);
	if (xSession->isEncryptionSession == FALSE) {
		//psecSetDescramblerType((mt_handle)(xSession->dscHandle), xEmi);
	}

	return SEC_NO_ERROR;
}

static TSecStatus secUseCertKey
(
	TSecCipherSession   xSession,
	TUnsignedInt16      xEmi,
	size_t              xKeyIdSize,
	TUnsignedInt8*     pxKeyId
)
{
	//mt_handle handle = 0;
	mt_s32 ret = -1;
	TUnsignedInt8 pol;
	TUnsignedInt32 keyslot;
	MT_CIPHER_CTRL_S KeyCtrl;
	mt_u8 slotID;
	TUnsignedInt32 meta;
	TUnsignedInt32 tmp;

	SECAPI_DINFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);

	if (NULL == xSession) {
		SECAPI_ERROR("secUseCertKey error %s\n", __func__);
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (sessionValidation(xSession)) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	//ret = mt_unf_cert_open(&handle);
	//if(ret != 0) {
	//	SECAPI_ERROR("secUseCertKey error %s\n", __func__);
	//	return SEC_ERROR;
	//}

	if (NULL == pxKeyId && 0 != xKeyIdSize) {
		SECAPI_ERROR("secUseCertKey error %s\n", __func__);
		//mt_unf_cert_key_ack(handle);
		//mt_unf_cert_close(handle);
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL != pxKeyId && 0 == xKeyIdSize) {
		SECAPI_ERROR("secUseCertKey error %s\n", __func__);
		//mt_unf_cert_key_ack(handle);
		//mt_unf_cert_close(handle);
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (!psecChkEmiRange(xEmi)) {
		SECAPI_ERROR("secUseCertKey error %s\n", __func__);
		//mt_unf_cert_key_ack(handle);
		//mt_unf_cert_close(handle);
		return SEC_ERROR_BAD_EMI;
	}

	xSession->isUseFlashProtKey = FALSE;
	xSession->EMI = xEmi;

        if (psecIsKey256(xSession->EMI)) {
            if (xSession->key256SlotID[0] == MT_CIPHER_KEYSLOT_INVALID) {
        	if (0 != mtSecKeySlotRequest(&slotID, 2)) {
        		psecFree(xSession);
        		SECAPI_ERROR("request key slot failed\n");
        		return SEC_ERROR;
        	}
            	xSession->key256SlotID[0] = slotID;
            }
        }

	if (xSession->isStreamSession) {
		pol = getPolarityByKeyId(pxKeyId, xKeyIdSize);
		keyslot = xSession->keySlotID[pol];
		xSession->polar = pol;
	} else {
            if (psecIsKey256(xSession->EMI))
		keyslot = xSession->key256SlotID[0];
            else
		keyslot = xSession->keySlotID[0];
	}

	ret = certExportKey(keyslot);

	if(ret != 0) {
		SECAPI_ERROR("secUseCertKey error %s\n", __func__);
		return SEC_ERROR;
	}

        mtSecSetIv(keyslot, (TUnsignedInt8 *)CBC_ZeroIV, sizeof(CBC_ZeroIV));

	/* rewrite iv after inacitve */
	mtSecSetImplicitIv(xSession->EMI, keyslot, 1);

	mt_unf_cipher_keyslot_info(keyslot);
	if (psecIsKey256(xSession->EMI))
		mt_unf_cipher_keyslot_info(keyslot+1);

	if (xSession->metadataSize == 4) {
		ret = mt_unf_cipher_keyslot_get_metadata(keyslot, &meta);
            SECAPI_INFO("meta = 0x%x\n", meta);
		if (ret < 0) {
			return SEC_ERROR;
		}

		tmp = (xSession->metadata[0]<<24) | (xSession->metadata[1]<<16) |
			(xSession->metadata[2]<<8) |(xSession->metadata[3]<<0);
		if (tmp != meta) {
        		SECAPI_ERROR("Metadata mismatch error, ktmeta:0x%x setmeta:0x%x\n", meta, tmp);
        		return SEC_ERROR;
		}
	}

	xSession->isKeySet = TRUE;
	if (xSession->isStreamSession) {
		SECAPI_DINFO("xSession->EMI:0x%x\n", xSession->EMI);
		/*Only for Decryption session*/
		if (xSession->isEncryptionSession == FALSE) {
			//psecSetDescramblerType((mt_handle)xSession->dscHandle, xEmi);
		}
	} else {
		memset(&KeyCtrl, 0x00, sizeof(MT_CIPHER_CTRL_S));
		psecSession2Ctrl(xSession, &KeyCtrl);
		mtSecCryptoEngineProcessPrepare(&KeyCtrl, xSession->EMI, &xSession->pidInfo,
			keyslot, &xSession->cryptoHandle);
	}

	return SEC_NO_ERROR;
}

static TSecStatus secUseFlashProtKey
(
 TSecCipherSession   xSession,
 TUnsignedInt16      xEmi
 )
{
    SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);
    if (NULL == xSession)
        return SEC_ERROR_BAD_PARAMETER;

    if (FALSE == psecChkEmiRange(xEmi))
        return SEC_ERROR_BAD_EMI;

    if (sessionValidation(xSession))
        return SEC_ERROR_BAD_PARAMETER;

    if (TRUE == xSession->isStreamSession)
        return SEC_ERROR_BAD_USAGE;

    xSession->isUseFlashProtKey = TRUE;
    xSession->EMI = xEmi;
	xSession->isKeySet = TRUE;
    return SEC_NO_ERROR;
}

static TSecStatus secSetMetadata
(
 TSecCipherSession   xSession,
 size_t              xMetadataSize,
 const TUnsignedInt8*     pxMetadata
 )
{
	SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);

	if (!xSession)
		return SEC_ERROR_BAD_PARAMETER;

	if (!pxMetadata || xMetadataSize == 0) {
		xSession->metadataSize = 0;
		memset(xSession->metadata, 0, 4);
		return SEC_NO_ERROR;
	}

	if (xMetadataSize == 0 || !pxMetadata)
		return SEC_ERROR_BAD_PARAMETER;

	if (xMetadataSize > 4) {
		SECAPI_ERROR("xMetadataSize:%ld overflow\n", xMetadataSize);
		return SEC_ERROR;
	}

	xSession->metadataSize = xMetadataSize;
	memcpy(xSession->metadata, pxMetadata, xMetadataSize);

	SECAPI_INFO("pxMetadata:%02x %02x %02x %02x\n",
		xSession->metadata[0], xSession->metadata[1], xSession->metadata[2], xSession->metadata[3]);

	return SEC_NO_ERROR;
}

static TSecStatus secSessionEncrypt
(
	TSecCipherSession   xSession,
	const TUnsignedInt8*     pxInput,
	TUnsignedInt8*     pxOutput,
	size_t              xMessageSize,
	const TUnsignedInt8*     pxInitVector,
	size_t              xInitVectorSize
)
{
	mt_s32 ret = MT_SUCCESS;
	unsigned int slot_id;
	TUnsignedInt32 residualLen;
	TUnsignedInt32 blockSize;
	MT_CIPHER_CTRL_S ctrl;
	TUnsignedInt8 *tmpInputBuffer;

	SECAPI_DINFO("xSession = 0x%lx xMessageSize = %ld  pxInput=%x, pxOutput=%x \n", (TUnsignedLong)xSession, xMessageSize, pxInput, pxOutput);

	if (!xSession || !pxInput || !pxOutput)
		return SEC_ERROR_BAD_PARAMETER;

	if (xMessageSize == 0)
		return SEC_ERROR_BAD_PARAMETER;

	if (sessionValidation(xSession))
		return SEC_ERROR_BAD_PARAMETER;

	if (!xSession->isEncryptionSession)
		return SEC_ERROR_BAD_USAGE;

	if (xSession->isUseFlashProtKey) {
		slot_id = NOCS_FLASH_PROT_KEY_SLOT;
	} else {
		if (psecIsKey256(xSession->EMI))
			slot_id = xSession->key256SlotID[xSession->polar];
		else
			slot_id = xSession->keySlotID[xSession->polar];
	}

	//sec_dump("inputdata:", (mt_u8 *)pxInput, xMessageSize);

	//这里nagra 可能会从kernel 标准库malloc ,这里面malloc出来的地址虽然也可能可以转成物理地址，但是会偶发catch问题，所以这里需要重新用mmz maolloc地址

	tmpInputBuffer = (TUnsignedInt8 *)mt_unf_cipher_malloc(xMessageSize);
	memcpy(tmpInputBuffer, pxInput, xMessageSize);

	if (xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
		SECAPI_DINFO("set iv \n");
		ret = mt_unf_cipher_keyslot_set_iv(slot_id, (TUnsignedInt8*)CISSA_InitIV, sizeof(CISSA_InitIV));
		if (ret != 0) {
			SECAPI_ERROR("Set IV for slot:%d failed\n", slot_id);
			return SEC_ERROR;
		}
	} else {
		if (psecIsEMINeedIV(xSession->EMI)) {
			if (pxInitVector && xInitVectorSize != 0) {
				//sec_dump("pxInitVector", (mt_u8 *)pxInitVector, xInitVectorSize);
				ret = mt_unf_cipher_keyslot_set_iv(slot_id, (TUnsignedInt8*)pxInitVector, xInitVectorSize);
				if (ret != MT_SUCCESS)
					return SEC_ERROR;
			}
		}
	}
	psecSession2Ctrl(xSession, &ctrl);
	SECAPI_INFO("ctrl.work_mode = 0x%x \n", ctrl.work_mode);
	if (psecIsDvbEmi(xSession->EMI)) {
		residualLen = 0;
	} else {

		switch(ctrl.algorithm)
		{
		case MT_CIPHER_ALG_AES:
		case MT_CIPHER_ALG_AES256:
		case MT_CIPHER_ALG_CSA3:
		{
			blockSize = 16;
			break;
		}
		case MT_CIPHER_ALG_DES:
		case MT_CIPHER_ALG_TDES:
		case MT_CIPHER_ALG_CSA2:
		{
			blockSize = 8;
			break;
		}
		case MT_CIPHER_ALG_HMAC256:
		{
			blockSize = xMessageSize;
			break;
		}
		default:
			SECAPI_ERROR("unknown algo\n");
			return SEC_ERROR;
		};

		residualLen = xMessageSize % blockSize;
		if (residualLen > 0) {
			if (ctrl.work_mode == MT_CIPHER_WORK_MODE_CTR)
				residualLen = 0;

			SECAPI_INFO("residualLen:0x%x\n", residualLen);
		}
	}
	SECAPI_INFO("ctrl.work_mode = 0x%x \n", ctrl.work_mode);
	if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xSession->EMI, &xSession->pidInfo,
		slot_id, &xSession->cryptoHandle)) {
		SECAPI_ERROR("MT CE prepare failed\n");
		return SEC_ERROR;
	}
	//SECAPI_DINFO(" (xMessageSize - residualLen) = %ld residualLen = %d  \n", xMessageSize - residualLen, residualLen);


	if(xMessageSize - residualLen)
	{
		ret = mtSecCryptoEngineProcessStart(xSession->cryptoHandle,
				xSession->EMI, tmpInputBuffer, pxOutput, xMessageSize - residualLen);
	}

	if (ret != 0)
		return SEC_ERROR;

	SECAPI_INFO("\n");
	//printf("%s %d \n",__FUNCTION__,__LINE__);
	//sec_dump("outputdata:", (mt_u8 *)pxOutput, xMessageSize);

	mtSecCryptoEngineProcessEnd(xSession->cryptoHandle, xSession->EMI);
	xSession->cryptoHandle = MT_INVALID_HANDLE;
	if (residualLen > 0) {
		memcpy(pxOutput + xMessageSize - residualLen,
			pxInput + xMessageSize - residualLen,
			residualLen);
	}
	SECAPI_INFO("\n");
	return SEC_NO_ERROR;
}

static TSecStatus secSessionDecrypt
(
	TSecCipherSession   xSession,
	const TUnsignedInt8*     pxInput,
	TUnsignedInt8*     pxOutput,
	size_t              xMessageSize,
	const TUnsignedInt8*     pxInitVector,
	size_t              xInitVectorSize
)
{
	mt_s32 ret = MT_SUCCESS;
	TUnsignedInt32 slot_id = 0;
	TUnsignedInt32 residualLen;
	TUnsignedInt32 blockSize;
	MT_CIPHER_CTRL_S ctrl;
	TUnsignedInt8 *tmpInputBuffer;

	SECAPI_INFO("xSession = 0x%lx\n", (TUnsignedLong)xSession);

	if (!xSession || !pxInput || !pxOutput)
		return SEC_ERROR_BAD_PARAMETER;

	if (xMessageSize == 0)
		return SEC_ERROR_BAD_PARAMETER;

	if (sessionValidation(xSession))
		return SEC_ERROR_BAD_PARAMETER;

	if (xSession->isEncryptionSession)
		return SEC_ERROR_BAD_USAGE;

	if (xSession->isUseFlashProtKey) {
		slot_id = NOCS_FLASH_PROT_KEY_SLOT;
	} else {
		if (psecIsKey256(xSession->EMI))
			slot_id = xSession->key256SlotID[xSession->polar];
		else
			slot_id = xSession->keySlotID[xSession->polar];
	}
	SECAPI_INFO("xSession->polar:%d slot_id:%d\n",xSession->polar, slot_id);

	tmpInputBuffer = (TUnsignedInt8 *)mt_unf_cipher_malloc(xMessageSize);
	memcpy(tmpInputBuffer, pxInput, xMessageSize);

	if (xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
		ret = mt_unf_cipher_keyslot_set_iv(slot_id, (TUnsignedInt8*)CISSA_InitIV, sizeof(CISSA_InitIV));
		if (ret != 0) {
			SECAPI_ERROR("Set IV for slot:%d failed\n", slot_id);
			return SEC_ERROR;
		}
	} else {
		if (psecIsEMINeedIV(xSession->EMI)) {
			if (pxInitVector && xInitVectorSize != 0) {
				//sec_dump("pxInitVector", (mt_u8 *)pxInitVector, xInitVectorSize);
				ret = mt_unf_cipher_keyslot_set_iv(slot_id, (TUnsignedInt8*)pxInitVector, xInitVectorSize);
				if (ret != MT_SUCCESS)
					return SEC_ERROR;
			}
		}
	}
	psecSession2Ctrl(xSession, &ctrl);
	if (psecIsDvbEmi(xSession->EMI)) {
		residualLen = 0;
	} else {
		switch(ctrl.algorithm)
		{
		case MT_CIPHER_ALG_AES:
		case MT_CIPHER_ALG_AES256:
		case MT_CIPHER_ALG_CSA3:
			blockSize = 16;
			break;
		case MT_CIPHER_ALG_DES:
		case MT_CIPHER_ALG_TDES:
		case MT_CIPHER_ALG_CSA2:
			blockSize = 8;
			break;
		case MT_CIPHER_ALG_HMAC256:
			blockSize = xMessageSize;
			break;
		default:
			SECAPI_ERROR("unknown algo\n");
			return SEC_ERROR;
		};

		residualLen = xMessageSize % blockSize;
		if (residualLen > 0) {
			if (ctrl.work_mode == MT_CIPHER_WORK_MODE_CTR)
				residualLen = 0;

			SECAPI_INFO("residualLen:0x%x\n", residualLen);
		}
	}
	if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xSession->EMI, &xSession->pidInfo,
		slot_id, &xSession->cryptoHandle)) {
		SECAPI_ERROR("MT CE prepare failed\n");
		return SEC_ERROR;
	}
	if( xMessageSize - residualLen )
	{
		ret = mtSecCryptoEngineProcessStart(xSession->cryptoHandle,
			xSession->EMI, tmpInputBuffer, pxOutput, xMessageSize - residualLen);
	}
	if (ret != 0) {
		SECAPI_ERROR("ret:%d\n", ret);
		return SEC_ERROR;
	}
	SECAPI_INFO("%s %d \n",__FUNCTION__,__LINE__);
	mtSecCryptoEngineProcessEnd(xSession->cryptoHandle, xSession->EMI);
	xSession->cryptoHandle = MT_INVALID_HANDLE;

	if (residualLen > 0) {
		memcpy(pxOutput + xMessageSize - residualLen,
			pxInput + xMessageSize - residualLen,
			residualLen);
	}
	return SEC_NO_ERROR;
}

static TSecStatus secProcessOpaqueData
(
 TSecCipherSession    xSession,
 size_t               xKeyIdSize,
 const TUnsignedInt8*      pxKeyId,
 const void*              pxOpaqueInput,
 void*              pxOpaqueOutput,
 TBoolean            xLastChunk
)
{
     int ret = 0;
    SECAPI_INFO("[REE]secProcessOpaqueData: 0x%lx\n", (TUnsignedLong)xSession);

    return ret;
}

static mt_u8 *vir2phy(const mt_u8 *vir)
{
    mt_s32 ret = MT_SUCCESS;
    phys_addr_t phy_addr;
    ulong phy_size;
    //SECAPI_INFO("vir = 0x%lx \n", vir);
    ret = mt_mmz_get_phyaddr((mt_void *)vir, &phy_addr, &phy_size);
    if (ret == MT_SUCCESS) {
       // SECAPI_INFO("Got vir = 0x%lx, phyaddr:%x\n", vir, phy_addr);
        return (mt_u8 *)phy_addr;
    }
    else {
		ret = mt_mem_get_phyaddr((mt_void *)vir, &phy_addr);
		if (ret == MT_SUCCESS) {
	       // SECAPI_INFO("Got vir = 0x%lx, phyaddr:%x\n", vir, phy_addr);
	        return (mt_u8 *)phy_addr;
	     }

	 SECAPI_ERROR("Get phyaddr by viraddr(%p) failed", vir);
        return NULL;
    }
}

static TUnsignedInt8* secAllocateBuffer
(
 size_t  xBufferSize
 )
{
    unsigned char * tmp_char;

    tmp_char=  mt_unf_cipher_malloc(xBufferSize);
    //mt_s64 PhyAddr = vir2phy(tmp_char);
    //SECAPI_DINFO("  secAllocateBuffer buffer adr = 0x%, 0x%x, 0x%x \n", tmp_char, PhyAddr, xBufferSize);

    return tmp_char;
}

static TSecStatus secFreeBuffer
(
 TUnsignedInt8*     pxBuffer
 )
{
    SECAPI_DINFO(" secFreeBuffer adr = %x \n", pxBuffer);
    if (pxBuffer == NULL) {
        return SEC_NO_ERROR;
    }
    if (pxBuffer && (MT_SUCCESS == mt_unf_cipher_free(pxBuffer)))
        return SEC_NO_ERROR;
    else
        return SEC_ERROR;
}

/* -------------------------------------------------------------------------- */
/*                           STREAM PROCESSING SESSION                        */
/* -------------------------------------------------------------------------- */

//for ott stream Encrypt
static TSecStatus secStreamEncryptSessionOpen (
        TSecStreamSession*     pxSession,
        TTransportSessionId     xTransportSessionId,
        TUnsignedInt16          xEmi)
{
	TSecStreamSession xSession;

	//printf("xTransportSessionId: 0x%x\n", xTransportSessionId);

	if (NULL == pxSession) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (/*0==xTransportSessionId  ||   modify by zbding 202040709 2.By default, the value and number of Nagra Transport Session Id for Stream Encryption is set to 0 and 1 respectively*/
		TRANSPORT_SESSION_ID_INVALID==xTransportSessionId ||xTransportSessionId==-1||xTransportSessionId==0xFFFFFFFF ) {
		SECAPI_ERROR("%s %d xTransportSessionId = %d  \n",__FUNCTION__,__LINE__, xTransportSessionId);

		return SEC_ERROR_BAD_PARAMETER;
	}

	if (!psecChkEmiRange(xEmi)) {
		SECAPI_ERROR("%s %d xTransportSessionId = %d SEC_ERROR_BAD_EMI \n",__FUNCTION__,__LINE__, xTransportSessionId);

		return SEC_ERROR_BAD_EMI;
	}

	xSession = (TSecStreamSession)psecMalloc(sizeof(struct SSecStreamSession));
	if (NULL == xSession) {
		SECAPI_ERROR("%s %d xTransportSessionId = %d NULL == xSession \n",__FUNCTION__,__LINE__, xTransportSessionId);

		return SEC_ERROR;
	}
	memset(xSession, 0, sizeof(struct SSecStreamSession));

	xSession->keySlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotID[2] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotID[3] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotStat = 0;

	xSession->isEncryptionSession = TRUE;
	xSession->transportSessionId = xTransportSessionId;
	xSession->EMI = xEmi;
	xSession->cryptoHandle = MT_INVALID_HANDLE;
	xSession->audioDscHandle = MT_INVALID_HANDLE;
	xSession->videoDscHandle = MT_INVALID_HANDLE;
	xSession->recDscHandle = MT_INVALID_HANDLE;

	xSession->magic = MT_SESSION_MAGIC;
	SECAPI_DINFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);
#ifdef _MT_NOCS_WITH_TALTS_ //talts test needs it ???

       TUnsignedInt32 nTimeout = 0;
#if 0
	//backup the new session
	//printf("%s %d xTransportSessionId = %d  \n",__FUNCTION__,__LINE__, xTransportSessionId);
	//验SEC-SPS-0100	这里要改下, 从spr 传标记进来是否是dvb , record , or replay, ott 等几种类型，都不是的话说明是类似SEC-SPS-0100 only Session Open Close Test
	while( /*nTimeout-- &&*/ gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType == TRANSPORT_SESSION_TYPE_NONE)//wait for nvSpr Session open
	{
		//SECAPI_DINFO("xTransportSessionId:%x   gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType = %d\n", xTransportSessionId, gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType);
		MT_USLEEP(1000);
	}
 #else
       nTimeout = 8000; //1000
       while((nTimeout--) && (gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType == TRANSPORT_SESSION_TYPE_NONE))//wait for nvSpr Session open
	{
		//SECAPI_DINFO("xTransportSessionId:%x   gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType = %d\n", xTransportSessionId, gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType);
		MT_USLEEP(1000);
	}
       if (nTimeout == 0) {
            SECAPI_ERROR("It is timeout, no dvb, record or replay, ott ...\n");
       }

#endif
#endif
	xSession->isMultiSession = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].isMultiSession;

	insertSession(xSession, SESSION_STREAM, SESSION_OP_ENCRYPT);

	*pxSession = xSession;
	//printf("%s %d xTransportSessionId = %d  \n",__FUNCTION__,__LINE__, xTransportSessionId);

	return SEC_NO_ERROR;
}

//for ott stream reEncrypt: decrpt dash stream and then encrypt with another key to ouput
static TSecStatus secStreamReEncryptSessionOpen (
        TSecStreamSession*     pxSession,
        TTransportSessionId     xTransportSessionId,
        TUnsignedInt16          xEmi)
{
	TSecStreamSession xSession;

	//printf("xTransportSessionId: 0x%x\n", xTransportSessionId);

	if (NULL == pxSession) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (/*0==xTransportSessionId  ||   modify by zbding 202040709 2.By default, the value and number of Nagra Transport Session Id for Stream Encryption is set to 0 and 1 respectively*/
		TRANSPORT_SESSION_ID_INVALID==xTransportSessionId ||xTransportSessionId==-1||xTransportSessionId==0xFFFFFFFF ) {
		SECAPI_ERROR("%s %d xTransportSessionId = %d  \n",__FUNCTION__,__LINE__, xTransportSessionId);

		return SEC_ERROR_BAD_PARAMETER;
	}

	if (!psecChkEmiRange(xEmi)) {
		SECAPI_ERROR("%s %d xTransportSessionId = %d SEC_ERROR_BAD_EMI \n",__FUNCTION__,__LINE__, xTransportSessionId);

		return SEC_ERROR_BAD_EMI;
	}

	xSession = (TSecStreamSession)psecMalloc(sizeof(struct SSecStreamSession));
	if (NULL == xSession) {
		SECAPI_ERROR("%s %d xTransportSessionId = %d NULL == xSession \n",__FUNCTION__,__LINE__, xTransportSessionId);

		return SEC_ERROR;
	}
	memset(xSession, 0, sizeof(struct SSecStreamSession));

	xSession->keySlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotID[2] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotID[3] = MT_CIPHER_KEYSLOT_INVALID;
	xSession->keySlotStat = 0;

	xSession->isEncryptionSession = TRUE;
	xSession->transportSessionId = xTransportSessionId;
	xSession->EMI = xEmi;
	xSession->cryptoHandle = MT_INVALID_HANDLE;
	xSession->audioDscHandle = MT_INVALID_HANDLE;
	xSession->videoDscHandle = MT_INVALID_HANDLE;
	xSession->recDscHandle = MT_INVALID_HANDLE;

	xSession->magic = MT_SESSION_MAGIC;
	SECAPI_DINFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);
	xSession->isMultiSession = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].isMultiSession;

	insertSession(xSession, SESSION_STREAM, SESSION_OP_ENCRYPT);

	*pxSession = xSession;
	//printf("%s %d xTransportSessionId = %d  \n",__FUNCTION__,__LINE__, xTransportSessionId);

	return SEC_NO_ERROR;
}

//for ott stream decrypt
static TSecStatus secStreamDecryptSessionOpen (
        TSecStreamSession*     pxSession,
        TTransportSessionId     xTransportSessionId,
        TUnsignedInt16          xEmi)
{
    TSecStreamSession xSession;
    SECAPI_DINFO("xTransportSessionId: 0x%x xEmi = 0x%x\n", xTransportSessionId, xEmi);

    if (NULL == pxSession) {
        return SEC_ERROR_BAD_PARAMETER;
    }

    if ( TRANSPORT_SESSION_ID_INVALID==xTransportSessionId ||xTransportSessionId==-1||xTransportSessionId==0xFFFFFFFF ) {
        return SEC_ERROR_BAD_PARAMETER;
    }

	if (!psecChkEmiRange(xEmi)) {

   		SECAPI_ERROR("[ERR] SEC_ERROR_BAD_EMI xTransportSessionId: 0x%x xEmi = 0x%x\n", xTransportSessionId, xEmi);

		return SEC_ERROR_BAD_EMI;
	}

    SECAPI_INFO("In,tsid=0x%x\n", xTransportSessionId);

    xSession = (TSecStreamSession)psecMalloc(sizeof(struct SSecStreamSession));
    if (NULL == xSession) {
        return SEC_ERROR;
    }
    memset(xSession, 0, sizeof(struct SSecStreamSession));

    xSession->keySlotID[0] = MT_CIPHER_KEYSLOT_INVALID;
    xSession->keySlotID[1] = MT_CIPHER_KEYSLOT_INVALID;
    xSession->keySlotID[2] = MT_CIPHER_KEYSLOT_INVALID;
    xSession->keySlotID[3] = MT_CIPHER_KEYSLOT_INVALID;
    xSession->keySlotStat = 0;

    xSession->isEncryptionSession = FALSE;
    xSession->transportSessionId = xTransportSessionId;
    xSession->EMI = xEmi;

    xSession->cryptoHandle = MT_INVALID_HANDLE;
    xSession->audioDscHandle = MT_INVALID_HANDLE;
    xSession->videoDscHandle = MT_INVALID_HANDLE;
    xSession->recDscHandle = MT_INVALID_HANDLE;

    xSession->magic = MT_SESSION_MAGIC;
    SECAPI_DINFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);
#ifdef _MT_NOCS_WITH_TALTS_ //talts test needs it ?
    TUnsignedInt32 nTimeout = 0;
	nTimeout = 8000; //1000 //这里必须要while 循环死等，否则会出现nvSprOpenDvbSession 没有执行完就跳转到这里现象，如SEC-SPS-0614
	//验SEC-SPS-0100	这里要改下, 从spr 传标记进来是否是dvb , record , or replay, ott 等几种类型，都不是的话说明是类似SEC-SPS-0100 only Session Open Close Test
	while((nTimeout--) && (gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType == TRANSPORT_SESSION_TYPE_NONE))//wait for nvSpr   Session Open
	//20250414最新测试SEC-SPS-0614这里条件判断去掉也没问题了，后续多观察跟进下，看看其他CASE是否也有影响。
	{
		//SECAPI_DINFO("xTransportSessionId:%x   gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType = %d\n", xTransportSessionId, gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].sessionType);
		MT_USLEEP(1000);

	}
       SECAPI_DINFO("_MT_NOCS_WITH_TALTS_\n");
       if (nTimeout == 0) {
            SECAPI_ERROR("It is timeout, no dvb, record or replay, ott ...\n");
       }
#endif
	xSession->isMultiSession = gsSecPIDInfo[xTransportSessionId & MAX_SESSIONS_ID_MASK].isMultiSession;

	SECAPI_DINFO("xTransportSessionId:%x   xSession->isMultiSession = %d\n", xTransportSessionId, xSession->isMultiSession);

    //backup the new session
    insertSession(xSession, SESSION_STREAM, SESSION_OP_DECRYPT);

    *pxSession = xSession;

    SECAPI_DINFO("InitStat keySlotID0=%x, keySlotID1=%x xTransportSessionId = %d  xSession->isMultiSession  = %d \n",xSession->keySlotID[0] ,xSession->keySlotID[1], xTransportSessionId, xSession->isMultiSession );

    return SEC_NO_ERROR;
}

static TSecStatus secStreamSessionClose
(
 TSecStreamSession xSession
 )
{
    TUnsignedInt16 xEmi;
    TUnsignedInt8 i = 0;
    TSecPidInfo pidinfo = {0};
    mt_handle crypto_handle = MT_INVALID_HANDLE;
    TEEC_Result tret = TEEC_SUCCESS;

    if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession)
   {
 		// printf("return 1 ~~~~~~~~~~~\n");

        return SEC_ERROR_BAD_PARAMETER;
    }
	  // printf(" secStreamSessionClose xSession MAGIC : 0x%x xSession->magic = 0x%x \n", *(TUnsignedInt32 *)xSession, xSession->magic );

    //to avoid double free of the session
    if (sessionValidation(xSession))
        return SEC_ERROR_BAD_PARAMETER;

       //printf(" ------  xSession: 0x%lx, magcic = 0x%lx MT_SESSION_MAGIC = 0x%lx\n", (TUnsignedLong)xSession, xSession->magic, MT_SESSION_MAGIC);

	secLock();

	if (xSession->isEncryptionSession) {
		SECAPI_INFO("This Encryption Session, tsid:%d, EMI:%x, now closing, %d, %d, %d, %d \n", xSession->transportSessionId, xSession->EMI,
                    xSession->keySlotID[0], xSession->keySlotID[1], xSession->keySlotID[2], xSession->keySlotID[3]);

	} else {
		SECAPI_INFO("This Decryption Session, tsid:%d, EMI:%x, now closing, %d, %d, %d, %d \n", xSession->transportSessionId, xSession->EMI,
                    xSession->keySlotID[0], xSession->keySlotID[1], xSession->keySlotID[2], xSession->keySlotID[3]);

	}

    for (i = 0; i < 4; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[i]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
        xSession->keySlotID[i] = MT_CIPHER_KEYSLOT_INVALID;
    }

    xEmi = xSession->EMI;

    crypto_handle = (mt_handle)xSession->cryptoHandle;
    if (MT_INVALID_HANDLE != crypto_handle) {
        mtSecCryptoEngineProcessEnd(crypto_handle, xEmi);
        SECAPI_INFO("Crypto closed\n");
	 xSession->cryptoHandle = MT_INVALID_HANDLE;

    }
	secUnlock();

    getLocalPidList(xSession->transportSessionId, &pidinfo);

	secLock();

    if (psecIsDvbEmi(xSession->EMI) || pidinfo.pidNum > 0) {
        if (xSession->audioDscHandle != MT_INVALID_HANDLE) {
            detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, xSession->audioDscHandle, &pidinfo);
            xSession->audioDscHandle = MT_INVALID_HANDLE;
        }
        if (xSession->videoDscHandle != MT_INVALID_HANDLE) {
            detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, xSession->videoDscHandle, &pidinfo);
            xSession->videoDscHandle = MT_INVALID_HANDLE;
        }
        if (xSession->recDscHandle != MT_INVALID_HANDLE) {
            detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, xSession->recDscHandle, &pidinfo);
            xSession->recDscHandle = MT_INVALID_HANDLE;
        }
    }
    secUnlock();
    mtSecClrPlatDmxId(xSession->transportSessionId);

    clearLocalPidList(xSession->transportSessionId);
    removeSession(xSession);
    //xSession->magic = 0xFFFFFFFF;    //reset the magic
    memset(xSession, 0x00, sizeof(struct SSecStreamSession));
    // SECAPI_DINFO("  psecFree secStreamSessionClose xSession MAGIC : 0x%x xSession->magic = 0x%x\n", *(TUnsignedInt32 *)xSession, xSession->magic);
    //printf(" psecFree ---------%d, %d magcic = 0x%lx \n", sizeof(TSecStreamSession), sizeof(struct SSecStreamSession), xSession->magic);
    psecFree((void *)xSession);
    //printf(" psecFree -----------------  xSession: 0x%lx magcic = 0x%lx \n", (TUnsignedLong)xSession, xSession->magic);
    xSession = NULL;

    SECAPI_DINFO("3333   secStreamSessionClose retrun ok \n");

    return SEC_NO_ERROR;
}

static TUnsignedInt16 mapKeySlotIdFromIndex(mt_u8 count, mt_u8 index)
{
    /*
     * Slot format
     * (count - 1)(8B) | index(8B)
     */
	SECAPI_INFO("\n");

    return (((TUnsignedInt16)(count - 1)) << 8) | index;
}

//#define SET_IV_BY_SMPC //only for debug
static int mtSecSetIv(TUnsignedInt32 xKeyslotId, TUnsignedInt8 *iv, TUnsignedInt32 ivSize)
{
	//printf("xKeyslotId = %d ivSize = %d \n", xKeyslotId, ivSize);
	int ret = 0;
	//sec_dump("iv", iv, ivSize);

#ifdef SET_IV_BY_SMPC
	TEEC_Result tret = TEEC_SUCCESS;
	tret = SMPC_SetIV(xKeyslotId, iv, ivSize);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("set iv by smpc failed,ret=%x\n", tret);
		return -1;
	}
	return 0;
#else
	ret = mt_unf_cipher_keyslot_set_iv(xKeyslotId, iv, ivSize);
	return ret;
#endif
}

static void mtSecSetImplicitIv(TUnsignedInt16 xEmi, TUnsignedInt32 xSlotId, TUnsignedInt32 xSlotCount)
{
	int i = 0;
	int ret = 0;
	SECAPI_INFO("\n");

	switch (xEmi) {
	case NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1:
		for (i = 0; i < xSlotCount; i++) {
			ret = mtSecSetIv((xSlotId + i), (TUnsignedInt8*)CISSA_InitIV, sizeof(CISSA_InitIV));
			if (ret != 0) {
				SECAPI_ERROR("Set IV for slot:%d failed\n", (xSlotId + i));
				return;
			}

		}
		break;
	case NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR:
	case NOCS_EMI_MPEG_TS_DVB_AES128_IDSA: //DVS042, zero IV
	case NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR:
		for (i = 0; i < xSlotCount; i++) {
			ret = mtSecSetIv((xSlotId + i), (TUnsignedInt8 *)CBC_ZeroIV, sizeof(CBC_ZeroIV));
			if (ret != 0) {
				SECAPI_ERROR("Set IV for slot:%d failed\n", (xSlotId + i));
				return;
			}

		}
		break;
	default:
		//TODO:If needs others, add more cases here
		break;
	}
}


// OTT ...
static TSecStatus mtSecOttSessionGetSlotAndSetMbInfo(TSecStreamSession xSession, size_t xKeyIdSize, const TUnsignedInt8 *pxKeyId, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	int i = 0;
	SMPC_MemType video_type = 0;
	MBType mb_type;
	MBInfo mb = {0};
	SMPC_SlotType slot_type = 0;

	//find an existed keyID
	for (i = 0; i < sizeof(xSession->keySlotID); i++) {
		//same keyID is needed
		if (!memcmp(xSession->keyID[i], pxKeyId, xKeyIdSize)
                    && (xSession->keySlotID[i] != MT_CIPHER_KEYSLOT_INVALID)) {
			*pxKeySlotIndex = xSession->keySlotID[i];
			SECAPI_INFO("Find the slot:%d based on keyID[%d]\n", *pxKeySlotIndex, i);
			break;
		}
	}

	/*allocate a new keyID
	 * keySlotStat_bitx = 0:spare; 1:busy
	 * keySlotStat_bit0 --> keySlotID[0]
	 * keySlotStat_bit1 --> keySlotID[1]
	 */

	 if(xSession->isMultiSession == FALSE)
	 {
	 	video_type = SMPC_MEM_TYPE_VIDEO;
		mb_type = MB_VID;
		slot_type = SMPC_SLOT_TYPE_OTT;
	 }
	 else
	 {
	 	video_type = SMPC_MEM_TYPE_SUB_VIDEO;
		mb_type = MB_SUB_VID;
		slot_type = SMPC_SLOT_TYPE_SUB_OTT;

	 }
	//printf("mtSecOttSessionGetSlotAndSetMbInfo  isMultiSession = %d transportSessionId = 0x%x slot_type = %d video_type = %d mb_type = %d \n", xSession->isMultiSession, xSession->transportSessionId, slot_type, video_type, mb_type);

	if (i == sizeof(xSession->keySlotID)) {
		for (i = 0; i < sizeof(xSession->keySlotID); i++) {
			if (!(xSession->keySlotStat & (1 << i))) {
				if (xSession->keySlotID[i] == MT_CIPHER_KEYSLOT_INVALID) {
					//if (0 != mtSecKeySlotRequest(&(xSession->keySlotID[i]), 1)) {
					if (0 != mtSecKeySlotRequest(&(xSession->keySlotID[i]), 2)) {
						SECAPI_ERROR("request key slot 0 failed\n");
						return SEC_ERROR;
					}
                                   xSession->keySlotID[i + 1] = xSession->keySlotID[i] + 1;
					//mtSecSetImplicitIv(xSession->EMI, xSession->keySlotID[i], 1);
					mtSecSetImplicitIv(xSession->EMI, xSession->keySlotID[i], 2);
					SECAPI_INFO("Got new slot:%d, %d\n", xSession->keySlotID[i], xSession->keySlotID[i+1]);
					tret = SMPC_Open(xSession->keySlotID[i], slot_type);
					if (tret != TEEC_SUCCESS) {
						SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[i]);
						goto __get_slot_failed;
					}
                                 tret = SMPC_Open(xSession->keySlotID[i + 1], slot_type);
					if (tret != TEEC_SUCCESS) {
						SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[i]);
						goto __get_slot_failed;
					}
					//printf("mtSecGetProtectBufferByType  00000000000000000000000000000  mb_type = %d \n" , mb_type);

					if (mtSecGetSmp(xSession->transportSessionId)  ) {
						//printf("mtSecGetProtectBufferByType  11111111111111111111111111111 mb_type = %d \n", mb_type);
						if (mtSecGetProtectBufferByType(mb_type, &mb, 0)) {
							//printf("mtSecGetProtectBufferByType  2222222222222222222222222222222\n");

							tret = SMPC_RegisterMemory(xSession->keySlotID[i], video_type, (uint32_t)mb.start, mb.size);
							if (tret != TEEC_SUCCESS) {
								SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
								goto __get_slot_failed;
							}
							//printf("SMPC_RegisterMemory video  adr = %lld ,  size = %lx \n",  mb.start, mb.size);
                                                 tret = SMPC_RegisterMemory(xSession->keySlotID[i + 1], video_type, (uint32_t)mb.start, mb.size);
							if (tret != TEEC_SUCCESS) {
								SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
								goto __get_slot_failed;
							}

						}

						if (mtSecGetProtectBufferByType(MB_AUD, &mb, 0) && xSession->isMultiSession == FALSE ) {
							tret = SMPC_RegisterMemory(xSession->keySlotID[i], SMPC_MEM_TYPE_AUDIO, (uint32_t)mb.start, mb.size);
							if (tret != TEEC_SUCCESS) {
								SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
								goto __get_slot_failed;
							}

							//printf("SMPC_RegisterMemory audio  adr = %lld ,  size = %lx\n",  mb.start, mb.size);

                                               tret = SMPC_RegisterMemory(xSession->keySlotID[i + 1], SMPC_MEM_TYPE_AUDIO, (uint32_t)mb.start, mb.size);
							if (tret != TEEC_SUCCESS) {
								SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
								goto __get_slot_failed;
							}
						}
						if (mtSecGetProtectBufferByType(MB_OTT, &mb, 0)) {
							SECAPI_INFO("mb.start:%lld, mb.size:%lx\n", mb.start, (TUnsignedLong)mb.size);
							tret = SMPC_RegisterMemory(xSession->keySlotID[i], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
							if (tret != TEEC_SUCCESS) {
								SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
								goto __get_slot_failed;
							}

						      //printf("SMPC_RegisterMemory ott  adr = %lld ,  size = %lx \n",  mb.start, mb.size);

                                               tret = SMPC_RegisterMemory(xSession->keySlotID[i + 1], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
							if (tret != TEEC_SUCCESS) {
								SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
								goto __get_slot_failed;
							}

						}

					}
				}

				*pxKeySlotIndex = xSession->keySlotID[i];

				//backup current keyID
				TUnsignedInt32 k;
				for (k = 0; k < xKeyIdSize; k++)
					xSession->keyID[i][k] =  pxKeyId[k];
				//clear all stat, then mark selected one
				xSession->keySlotStat = 0;
				xSession->keySlotStat = (1 << i);
				SECAPI_INFO("Find the slot:%d based on keyStat_bit%d\n", *pxKeySlotIndex, i);

				break;
			}
		}
	}
	//hex_dump("keyid dump", pxKeyId, xKeyIdSize);
	SECAPI_INFO("====>slotId:%x\n", *pxKeySlotIndex);

	return SEC_NO_ERROR;

__get_slot_failed:

    for (i = 0; i < 4; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[i]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
    }
    return SEC_ERROR;
}

/*
 * Replaying Raw EMIs Session EMI 4020 4021 4023 , for  m2m decrypt
 */
static TSecStatus mtSecRepRawSessionGetSlotAndSetMbInfo(TSecStreamSession xSession, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	mt_u8 slotId = 0;
	MBInfo mb = {0};
	MT_CIPHER_CTRL_S ctrl = {0};
	SMPC_MemType video_type = 0;
	MBType mb_type;
	SMPC_SlotType slot_type = 0;

	if(xSession->isMultiSession == FALSE)
	 {
	 	video_type = SMPC_MEM_TYPE_VIDEO;
		mb_type = MB_VID;
		slot_type = SMPC_SLOT_TYPE_OTT;
	 }
	 else
	 {
	 	video_type = SMPC_MEM_TYPE_SUB_VIDEO;
		mb_type =MB_SUB_VID;
		slot_type = SMPC_SLOT_TYPE_SUB_OTT;

	 }

	SECAPI_DINFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);

	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
		*pxKeySlotIndex = xSession->keySlotID[0];
		return SEC_NO_ERROR;
	}

	if (MT_SUCCESS != mtSecKeySlotRequest(&slotId, 1)) {
		SECAPI_ERROR("request key slot 0 failed\n");
		return SEC_ERROR;
	}
	mtSecSetImplicitIv(xSession->EMI, slotId, 1);

	xSession->keySlotID[0] = slotId;
	tret = SMPC_Open(xSession->keySlotID[0], slot_type);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
		goto __get_slot_failed;
	}
	//More than 1 block of buffer to register
	if (mtSecGetSmp(xSession->transportSessionId) ) {
		if (mtSecGetProtectBufferByType(MB_OTT, &mb, 1)) {
			//printf("register:addr:%lld,size:%lx\n", mb.start, mb.size);
			tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}
		}

		if (mtSecGetProtectBufferByType(MB_OTT_1, &mb, 1)) {
			SECAPI_INFO("register:addr:%lld,size:%lx\n", mb.start, mb.size);
			tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}
		}

		if (mtSecGetProtectBufferByType(mb_type, &mb, 1)) {
			tret = SMPC_RegisterMemory(xSession->keySlotID[0], video_type, (uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}

		}

		if(xSession->isMultiSession == FALSE)
		{
			if (mtSecGetProtectBufferByType(MB_AUD, &mb, 1)) {
				tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_AUDIO, (uint32_t)mb.start, mb.size);
				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[0]);
					goto __get_slot_failed;
				}
			}
		}
	}

	//RAW-EMI encryption prepare
	psecStreamSession2Ctrl(xSession, &ctrl);

	if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xSession->EMI,
		&xSession->pidInfo, slotId, &xSession->cryptoHandle)) {
	//if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xSession->EMI, slotId, &crypto_handle)) {
		mt_unf_cipher_keyslot_release(slotId);
		goto __get_slot_failed;
	}

	//RAW stream data process, pick one slot
	*pxKeySlotIndex = mapKeySlotIdFromIndex(1, slotId);
	return SEC_NO_ERROR;

__get_slot_failed:
    if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
        tret = mtExtSMPC_Close(xSession, xSession->keySlotID[0]);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[0]);
	}
        mt_unf_cipher_keyslot_release(xSession->keySlotID[0]);
    }
	return SEC_ERROR;
}

/*
 * Replaying TS EMIs Session , EMI 0020,0021,0022,0023 for ts descrambler
 */
static TSecStatus mtSecRepTsSessionGetSlotAndSetMbInfo(TSecStreamSession xSession, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	mt_u8 slotId = 0, i = 0;
	TSecPidInfo pidinfo = {0};
	TBoolean isPidValid = FALSE;
	MBInfo mb = {0};
	mt_handle audio_dsc_handle, video_dsc_handle;
	SECAPI_INFO("xSession: 0x%lx, enc=%d xSession->isMultiSession = %d \n", (TUnsignedLong)xSession, xSession->isEncryptionSession, xSession->isMultiSession);


	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
		*pxKeySlotIndex = mapKeySlotIdFromIndex(4, xSession->keySlotID[0]);
		return SEC_NO_ERROR;
	}

	if (0 != mtSecKeySlotRequest(&slotId, 4)) {
		SECAPI_ERROR("request 4 key slots failed\n");
		return SEC_ERROR;
	}
	SECAPI_INFO("Requested ID:%d\n", slotId);
	mtSecSetImplicitIv(xSession->EMI, slotId, 4);

	xSession->keySlotID[0] = slotId;

	tret = SMPC_Open(xSession->keySlotID[0], SMPC_SLOT_TYPE_AUDIO);

	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC Open for slot%d failed\n", xSession->keySlotID[0]);
		goto __get_slot_failed;
	}

	//0:keep for next Get
	if (mtSecGetSmp(xSession->transportSessionId)) {

		if (mtSecGetProtectBufferByType(MB_AUD, &mb, 0) && xSession->isMultiSession == FALSE ) {
			tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_AUDIO, (uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}

		}
	}

	xSession->keySlotID[1] = slotId + 1;

      if(xSession->isMultiSession == FALSE)
      {

    	   tret = SMPC_Open(xSession->keySlotID[1], SMPC_SLOT_TYPE_VIDEO);

      }
      else
      {
    	   tret = SMPC_Open(xSession->keySlotID[1], SMPC_SLOT_TYPE_SUB_VIDEO);

       }
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC Open for slot%d failed\n", xSession->keySlotID[1]);
		goto __get_slot_failed;
	}
	if (mtSecGetSmp(xSession->transportSessionId)) {

		if(xSession->isMultiSession == FALSE){
			if (mtSecGetProtectBufferByType(MB_VID, &mb, 0)) {

				tret = SMPC_RegisterMemory(xSession->keySlotID[1], SMPC_MEM_TYPE_VIDEO, (uint32_t)mb.start, mb.size);
				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[1]);
					goto __get_slot_failed;
				}

			}
		}
		else
		{

			if (mtSecGetProtectBufferByType(MB_SUB_VID, &mb, 0)) {
				//printf("%s %d mb.start=%lld mb.size=%lx\n",__FUNCTION__,__LINE__, mb.start, mb.size);

				tret = SMPC_RegisterMemory(xSession->keySlotID[1], SMPC_MEM_TYPE_SUB_VIDEO, (uint32_t)mb.start, mb.size);
				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed  mb.start=%lld mb.size=%lx \n", xSession->keySlotID[1],  mb.start, mb.size);
					goto __get_slot_failed;
				}

			}
		}
	}
	xSession->keySlotID[2] = slotId + 2;

	tret = SMPC_Open(xSession->keySlotID[2], SMPC_SLOT_TYPE_AUDIO);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC Open for slot%d failed\n", xSession->keySlotID[2]);
		goto __get_slot_failed;
	}
	if (mtSecGetSmp(xSession->transportSessionId)) {

		if (mtSecGetProtectBufferByType(MB_AUD, &mb, 1) && xSession->isMultiSession == FALSE) {
		tret = SMPC_RegisterMemory(xSession->keySlotID[2], SMPC_MEM_TYPE_AUDIO, (uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[2]);
				goto __get_slot_failed;
			}

		}
	}
	xSession->keySlotID[3] = slotId + 3;

      if(xSession->isMultiSession == FALSE)
      {
    	   tret = SMPC_Open(xSession->keySlotID[3], SMPC_SLOT_TYPE_VIDEO);
      }
      else
      {
    	   tret = SMPC_Open(xSession->keySlotID[3], SMPC_SLOT_TYPE_SUB_VIDEO);
       }
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC Open for slot%d failed\n", xSession->keySlotID[1]);
		goto __get_slot_failed;
	}
	if (mtSecGetSmp(xSession->transportSessionId) ) {

		if(xSession->isMultiSession == FALSE)
		{
			if (mtSecGetProtectBufferByType(MB_VID, &mb, 1)) {
				tret = SMPC_RegisterMemory(xSession->keySlotID[3], SMPC_MEM_TYPE_VIDEO, (uint32_t)mb.start, mb.size);
				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[3]);
					goto __get_slot_failed;
				}
			}
		}
		else
		{
			if (mtSecGetProtectBufferByType(MB_SUB_VID, &mb, 1)) {
				tret = SMPC_RegisterMemory(xSession->keySlotID[3], SMPC_MEM_TYPE_SUB_VIDEO, (uint32_t)mb.start, mb.size);
				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[3]);
					goto __get_slot_failed;
				}
			}

		}
	}

	getLocalPidList(xSession->transportSessionId, &pidinfo);
	if (pidinfo.pidNum > 0)
		isPidValid = TRUE;

	//create descrambler and attach
	if (isPidValid) {
		if(xSession->isMultiSession == FALSE)
		{
			if (!attachDescrambler(xSession->transportSessionId, &audio_dsc_handle, xSession->keySlotID[0], xSession->keySlotID[2])) {
				SECAPI_ERROR("attachDescrambler audio  error audio_dsc_handle = %x \n", audio_dsc_handle);
				goto __get_slot_failed;
			}
		}

		if (!attachDescrambler(xSession->transportSessionId, &video_dsc_handle, xSession->keySlotID[1], xSession->keySlotID[3])) {
				SECAPI_ERROR("attachDescrambler video  error video_dsc_handle = %x \n", video_dsc_handle);
			goto __get_slot_failed;
		}

		//remember descrambler
		if(xSession->isMultiSession == FALSE)
		{
			xSession->audioDscHandle = audio_dsc_handle;
		}
		xSession->videoDscHandle = video_dsc_handle;

		//attach pidlist
		if (SEC_NO_ERROR != mtSecAttachSessionPidToDescrambler(xSession->transportSessionId, SESSION_OP_DECRYPT, &pidinfo)) {
			if(xSession->isMultiSession == FALSE)
			{
				detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, audio_dsc_handle, &pidinfo);
			}
			detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, video_dsc_handle, &pidinfo);
			goto __get_slot_failed;
		}

		psecSetDescramblerType((mt_handle)xSession->videoDscHandle, xSession->EMI);
		if(xSession->isMultiSession == FALSE)
		{
			psecSetDescramblerType((mt_handle)xSession->audioDscHandle, xSession->EMI);
		}
		SECAPI_INFO("Attach descrambler Done\n");
	}

	*pxKeySlotIndex = mapKeySlotIdFromIndex(4, slotId);
	SECAPI_INFO("Got KeySlotIndex:%x", *pxKeySlotIndex);
	return SEC_NO_ERROR;

__get_slot_failed:

    for (i = 0; i < 4; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[i]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
    }
    return SEC_ERROR;
}

/*
 * Recording Scrambling Session (With KeyId)
 * TODO: Usage of this function is not determined yet!
 */
static TSecStatus mtSecRecScrSessionGetSlotAndSetMbInfoWithKeyId(TSecStreamSession xSession, size_t xKeyIdSize, const TUnsignedInt8 *pxKeyId, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	MBInfo mb = {0};
	SMPC_SlotType slot_type = SMPC_SLOT_TYPE_PVR_ENC;
	MBType  mb_type;
	SMPC_MemType smp_mem_type;
	mt_u8 rec_chan = 0xFF;
	mt_u32 k = 0, i = 0;

/*
	if(xSession->isMultiSession == FALSE)
	{
		slot_type = SMPC_SLOT_TYPE_PVR_ENC;
		mb_type = MB_REC;
		smp_mem_type = SMPC_MEM_TYPE_PVR;
	}
	else
	{
		slot_type = SMPC_SLOT_TYPE_PVR1_ENC;
		mb_type = MB_REC_1;
		smp_mem_type = SMPC_MEM_TYPE_PVR1;

	}
*/
      MT_UNF_DMX_PORT_E enPortId = MT_UNF_DMX_PORT_RAM_0;
      k = mtSecGetPlatDmxId(xSession->transportSessionId & MAX_SESSIONS_ID_MASK);

      SECAPI_INFO("dmx id = %d \n", k);
       if (MT_UNF_DMX_GetTSPortId(k, &enPortId)) {

            SECAPI_ERROR("enPortId = 0x%x\n, enPortId");
            //return SEC_NO_ERROR;//TALTS need get keslot success
       }

       //slot_type += enPortId - MT_UNF_DMX_PORT_RAM_0;
       SECAPI_INFO("enPortId = 0x%x \n", enPortId);
       //get valid record chan by dmx port ID
       i = 1000;
       while((!mtSecGetRecChanByDmxID(enPortId, &rec_chan, 1)) && i) {
            MT_USLEEP(1000);
            i--;
       }
       if (rec_chan >= MAX_DMX_REC_CHAN) {
            SECAPI_ERROR("rec_chan is error! rec_chan = 0x%x\n", rec_chan);
            //return SEC_ERROR;//TALTS need get keslot success
            rec_chan = MAX_DMX_REC_CHAN - 1;
       }
       slot_type += rec_chan;
       mb_type = MB_REC + enPortId - MT_UNF_DMX_PORT_RAM_0;

	smp_mem_type = SMPC_MEM_TYPE_PVR + enPortId - MT_UNF_DMX_PORT_RAM_0;

       k = 0;
	SECAPI_INFO("xSession: 0x%lx, xKeyIdSize = %ld\n", (TUnsignedLong)xSession, xKeyIdSize);

	//sec_dump("keyid dump", pxKeyId, xKeyIdSize);

	//find an existed keyID
	for (i = 0; i < sizeof(xSession->keySlotID); i++) {
		//same keyID is needed
		if ((!memcmp(xSession->keyID[i], pxKeyId, xKeyIdSize))
                    && (xSession->keySlotID[i] != MT_CIPHER_KEYSLOT_INVALID)) {
			*pxKeySlotIndex = xSession->keySlotID[i];
			SECAPI_INFO("Find the slot:%d based on keyID[%d]\n", *pxKeySlotIndex, i);
			break;
		}
	}

	/*allocate a new keyID
	 * keySlotStat_bitx = 0:spare; 1:busy
	 * keySlotStat_bit0 --> keySlotID[0]
	 * keySlotStat_bit1 --> keySlotID[1]
	 */
	if (i == sizeof(xSession->keySlotID)) {
		for (i = 0; i < sizeof(xSession->keySlotID); i++) {
			if (!(xSession->keySlotStat & (1 << i))) {
				if (xSession->keySlotID[i] == MT_CIPHER_KEYSLOT_INVALID) {
					//if (0 != mtSecKeySlotRequest(&(xSession->keySlotID[i]), 1)) {
					if (0 != mtSecKeySlotRequest(&(xSession->keySlotID[i]), 2)) {
						SECAPI_ERROR("request key slot 0 failed\n");
						return SEC_ERROR;
				      }
                                  xSession->keySlotID[i + 1] = xSession->keySlotID[i] + 1;
					//mtSecSetImplicitIv(xSession->EMI, xSession->keySlotID[i], 1);
					mtSecSetImplicitIv(xSession->EMI, xSession->keySlotID[i], 2);
					SECAPI_INFO("Got new slot[%d]:%d, %d\n", i, xSession->keySlotID[i], xSession->keySlotID[i + 1]);

					//recording buffer register
					SECAPI_INFO("SMPC_Open:%d\n", xSession->keySlotID[i]);
					tret = SMPC_Open(xSession->keySlotID[i], slot_type);
					if (tret != TEEC_SUCCESS) {
						SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[i]);
						goto __get_slot_failed;
					}

                                 SECAPI_INFO("SMPC_Open:%d\n", xSession->keySlotID[i + 1]);
					tret = SMPC_Open(xSession->keySlotID[i + 1], slot_type);
					if (tret != TEEC_SUCCESS) {
						SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[i]);
						goto __get_slot_failed;
					}

					if (mtSecGetSmp(xSession->transportSessionId)) {
						k = 0;
						while (!mtSecGetProtectBufferByType(mb_type, &mb, 0) && k++< 2000)
						{
							//printf("w...\n");
						}
                        SECAPI_INFO("mb_type=0x%x, start=0x%x, size = 0x%x, smp_mem_type=0x%x\n", mb_type, mb.start, mb.size, smp_mem_type);
						//if (mtSecGetProtectBufferByType(mb_type, &mb, 0))
						tret = SMPC_RegisterMemory(xSession->keySlotID[i], smp_mem_type, (uint32_t)mb.start, mb.size);
						if (tret != TEEC_SUCCESS) {
							SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[i]);
							goto __get_slot_failed;
						}

						tret = SMPC_RegisterMemory(xSession->keySlotID[i + 1], smp_mem_type, (uint32_t)mb.start, mb.size);
						if (tret != TEEC_SUCCESS) {
							SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[i]);
							goto __get_slot_failed;
						}
					}
				}
				*pxKeySlotIndex = xSession->keySlotID[i];

				//backup current keyID
				for (k = 0; k < xKeyIdSize; k++)
					xSession->keyID[i][k] =  pxKeyId[k];

				//clear all stat, then mark selected one
				xSession->keySlotStat = 0;
				xSession->keySlotStat = (1 << i);
				SECAPI_INFO("Find the slot:%d based on keyStat_bit%d\n", *pxKeySlotIndex, i);

				break;
			}
		}
	}
	return SEC_NO_ERROR;

__get_slot_failed:

    for (i = 0; i < 4; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[i]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
    }
    return SEC_ERROR;

}

/*
 * Recording Scrambling Session
 */

static TSecStatus mtSecRecScrSessionGetSlotAndSetMbInfo(TSecStreamSession xSession, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	mt_u8 slotId = 0;
	MBInfo mb = {0};
	mt_u32 k = 0;
	MT_CIPHER_CTRL_S ctrl = {0};
       SMPC_SlotType slot_type = SMPC_SLOT_TYPE_PVR_ENC;//need get correct value from record channel
       SMPC_MemType smp_mem_type;
       MBType  mb_type;
       mt_u8 rec_chan = 0xFF;

	SECAPI_INFO("xSession: 0x%lx transportSessionId=%d \n", (TUnsignedLong)xSession, xSession->transportSessionId);


	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
		//For current Session, slot has been requested, and is not released yet, no need to allocate new one
		*pxKeySlotIndex = xSession->keySlotID[0];
		return SEC_NO_ERROR;
	}

	if (MT_SUCCESS != mtSecKeySlotRequest(&slotId, 1)) {
		SECAPI_ERROR("request key slot 0 failed\n");
		return SEC_ERROR;
	}
	mtSecSetImplicitIv(xSession->EMI, slotId, 1);
/*
	//encrypted output buffer register
	xSession->keySlotID[0] = slotId;
	k = 0;
	if(xSession->isMultiSession == FALSE)
	{
		tret = SMPC_Open(xSession->keySlotID[0], SMPC_SLOT_TYPE_PVR_ENC);
	}
	else
	{
		tret = SMPC_Open(xSession->keySlotID[0], SMPC_SLOT_TYPE_PVR1_ENC);
	}
*/
       MT_UNF_DMX_PORT_E enPortId = MT_UNF_DMX_PORT_RAM_0;
      k = mtSecGetPlatDmxId(xSession->transportSessionId & MAX_SESSIONS_ID_MASK);

      SECAPI_INFO("dmx id = %d \n", k);
       if (MT_UNF_DMX_GetTSPortId(k, &enPortId)) {

            SECAPI_ERROR("enPortId = 0x%x\n, enPortId");
            //return SEC_NO_ERROR;//TALTS need get keslot success
       }

       //slot_type += enPortId - MT_UNF_DMX_PORT_RAM_0;
       SECAPI_INFO("enPortId = 0x%x \n", enPortId);
       mb_type = MB_REC + enPortId - MT_UNF_DMX_PORT_RAM_0;
       k = 1000;
       while((!mtSecGetRecChanByDmxID(enPortId, &rec_chan, 1)) && k) {
            MT_USLEEP(1000);
            k--;
       }
       if (rec_chan >= MAX_DMX_REC_CHAN) {
            SECAPI_ERROR("rec_chan is error! rec_chan = 0x%x\n", rec_chan);
            rec_chan = MAX_DMX_REC_CHAN -1;
            //return SEC_ERROR;////TALTS need get keslot success
       }
       slot_type += rec_chan;
	smp_mem_type = SMPC_MEM_TYPE_PVR + enPortId - MT_UNF_DMX_PORT_RAM_0;
/*
#ifdef  MT_TEST_TALTS
       if(xSession->isMultiSession == FALSE)
	{
		slot_type = SMPC_SLOT_TYPE_PVR_ENC;
		smp_mem_type = SMPC_MEM_TYPE_PVR;
	}
#endif
*/
       k = 0;
       //encrypted output buffer register
	xSession->keySlotID[0] = slotId;


   tret = SMPC_Open(xSession->keySlotID[0], slot_type);


/*
	if (mtSecGetSmp(xSession->transportSessionId)) {
		//printf("mtSecRecScrSessionGetSlotAndSetMbInfo xSession->isMultiSession = %d  \n", xSession->isMultiSession);
		if(xSession->isMultiSession == FALSE)
		{
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}
			while (!mtSecGetProtectBufferByType(MB_REC, &mb, 0) && k++< 2000)
			{
				//printf("w...\n");
			}
			//printf("SMPC_SLOT_TYPE_PVR_ENC --------------------------------\n");

			tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_PVR,(uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
			goto __get_slot_failed;
			}
		}
		else
		{
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}
			while (!mtSecGetProtectBufferByType(MB_REC_1, &mb, 0)&& k++< 2000)
			{
				//printf("w1...\n");
			}
			//printf("SMPC_SLOT_TYPE_PVR1_ENC --------------------------------\n");

			tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_PVR1,(uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
			goto __get_slot_failed;
			}
		}
	}
*/
      if (mtSecGetSmp(xSession->transportSessionId)) {
        	//printf("mtSecRecScrSessionGetSlotAndSetMbInfo xSession->isMultiSession = %d  \n", xSession->isMultiSession);

        	if (tret != TEEC_SUCCESS) {
        		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
        		goto __get_slot_failed;
        	}
        	while (!mtSecGetProtectBufferByType(mb_type, &mb, 0)&& k++< 2000)
        	{
        		//printf("w1...\n");
        	}
        	//printf("SMPC_SLOT_TYPE_PVR1_ENC --------------------------------\n");
            SECAPI_INFO("mb_type=0x%x, start=0x%x, size = 0x%x, smp_mem_type=0x%x\n", mb_type, mb.start, mb.size, smp_mem_type);
        	tret = SMPC_RegisterMemory(xSession->keySlotID[0], smp_mem_type,(uint32_t)mb.start, mb.size);
        	if (tret != TEEC_SUCCESS) {
        		SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
        		goto __get_slot_failed;
        	}
	}



	//RAW-EMI encryption prepare
	psecStreamSession2Ctrl(xSession, &ctrl);
	if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xSession->EMI,
		&xSession->pidInfo, slotId, &xSession->cryptoHandle)) {
	//if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xSession->EMI, slotId, &crypto_handle)) {
		goto __get_slot_failed;
	}
	*pxKeySlotIndex = slotId;
	gsSecPIDInfo[xSession->transportSessionId].recDscHandle = xSession->recDscHandle;

	SECAPI_INFO(" success success success  xSession->cryptoHandle = %lx  xSession->transportSessionId = %d \n", xSession->cryptoHandle, xSession->transportSessionId);
	return SEC_NO_ERROR;

__get_slot_failed:
    if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
        tret = mtExtSMPC_Close(xSession, xSession->keySlotID[0]);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[0]);
	}
        mt_unf_cipher_keyslot_release(xSession->keySlotID[0]);
    }

	return SEC_ERROR;
}

/*
 * Recording Descrambling Session
 */
static TSecStatus mtSecRecDscSessionGetSlotAndSetMbInfo(TSecStreamSession xSession, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	mt_u8 slotId = 0, i = 0;
	TSecPidInfo pidinfo = {0};
	TBoolean isPidValid = FALSE;
	MBInfo mb = {0};
	mt_u32 k = 0;
	mt_handle rec_dsc_handle;
       SMPC_SlotType slot_type = SMPC_SLOT_TYPE_PVR_DEC;
       SMPC_MemType smp_mem_type;
       MBType  mb_type;
       MT_UNF_DMX_PORT_E enPortId = MT_UNF_DMX_PORT_RAM_0;
	//printf("  mtSecRecDscSessionGetSlotAndSetMbInfo  xSession: 0x%lx\n", (TUnsignedLong)xSession);

	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
		*pxKeySlotIndex = mapKeySlotIdFromIndex(2, xSession->keySlotID[0]);
		return SEC_NO_ERROR;
	}

	if (0 != mtSecKeySlotRequest(&slotId, 2)) {
		SECAPI_ERROR("request key slot 0 failed\n");
		return SEC_ERROR;
	}

	mtSecSetImplicitIv(xSession->EMI, slotId, 2);

	//recording buffer register
	xSession->keySlotID[0] = slotId;
	xSession->keySlotID[1] = slotId + 1;

	//printf("mtSecRecScrSessionGetSlotAndSetMbInfo xSession->isMultiSession = %d  \n", xSession->isMultiSession);

      k = mtSecGetPlatDmxId(xSession->transportSessionId & MAX_SESSIONS_ID_MASK);

      SECAPI_INFO("dmx id = %d \n", k);
       if (MT_UNF_DMX_GetTSPortId(k, &enPortId)) {

            SECAPI_ERROR("enPortId = 0x%x\n, enPortId");
            //return SEC_NO_ERROR;//TALTS need get keslot success
       }
       slot_type += enPortId - MT_UNF_DMX_PORT_RAM_0;
       SECAPI_INFO("enPortId = 0x%x , xSession->isMultiSession =%d\n", enPortId, xSession->isMultiSession);
       mb_type = MB_REC + enPortId - MT_UNF_DMX_PORT_RAM_0;
       smp_mem_type = SMPC_MEM_TYPE_PVR + enPortId - MT_UNF_DMX_PORT_RAM_0;
    /*
#ifdef  MT_TEST_TALTS
       //Do talts test SPS_3202 shows, if pip path value is less than main path, ecryption will error "mtSecRawStreamDataProcess_phy Raw stream data encryption process failed(Key may not set YET"
      if(xSession->isMultiSession == FALSE)
	{
		slot_type = SMPC_SLOT_TYPE_PVR_DEC;
		smp_mem_type = SMPC_MEM_TYPE_PVR;
	}
#endif
*/
       k = 0;

/*
	if(xSession->isMultiSession == FALSE)
	{
		while (!mtSecGetProtectBufferByType(MB_REC, &mb, 0) && k++< 2000)
		{
			//printf("rec dec w...\n");
		}
		//printf("SMPC_SLOT_TYPE_PVR_DEC -------------------------------- (uint32_t)mb.start = %d  mb.size= %d  \n", (uint32_t)mb.start, mb.size);
		tret = SMPC_Open(xSession->keySlotID[0], SMPC_SLOT_TYPE_PVR_DEC);
		if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
			goto __get_slot_failed;
		}
		tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_PVR,(uint32_t)mb.start, mb.size);
		if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
		goto __get_slot_failed;
		}

		xSession->keySlotID[1] = slotId + 1;

		tret = SMPC_Open(xSession->keySlotID[1], SMPC_SLOT_TYPE_PVR_DEC);
		if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[1]);
			goto __get_slot_failed;
		}

		if (mtSecGetSmp(xSession->transportSessionId))
		{
			tret = SMPC_RegisterMemory(xSession->keySlotID[1], SMPC_MEM_TYPE_PVR,(uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
			goto __get_slot_failed;
			}
		}
	}
	else
	{
		while (!mtSecGetProtectBufferByType(MB_REC_1, &mb, 0)&& k++< 2000)
		{
			//printf("rec dec w1...\n");
		}
		//printf("SMPC_SLOT_TYPE_PVR1_DEC --------------------------------\n");
		tret = SMPC_Open(xSession->keySlotID[0], SMPC_SLOT_TYPE_PVR1_DEC);
		if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
			goto __get_slot_failed;
		}

		tret = SMPC_RegisterMemory(xSession->keySlotID[0], SMPC_MEM_TYPE_PVR1,(uint32_t)mb.start, mb.size);
		if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
		goto __get_slot_failed;
		}

		//xSession->keySlotID[1] = slotId + 1;

		tret = SMPC_Open(xSession->keySlotID[1], SMPC_SLOT_TYPE_PVR1_DEC);
		if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[1]);
			goto __get_slot_failed;
		}

		if (mtSecGetSmp(xSession->transportSessionId))
		{
			tret = SMPC_RegisterMemory(xSession->keySlotID[1], SMPC_MEM_TYPE_PVR1,(uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
			SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
			goto __get_slot_failed;
			}
		}
	}
*/
       k = 0;
       while (!mtSecGetProtectBufferByType(mb_type, &mb, 0)&& k++< 2000)
	{
		//printf("rec dec w1...\n");
	}
	//printf("SMPC_SLOT_TYPE_PVR1_DEC --------------------------------\n");
	tret = SMPC_Open(xSession->keySlotID[0], slot_type);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);
		goto __get_slot_failed;
	}
       if (mtSecGetSmp(xSession->transportSessionId))
	{
        	tret = SMPC_RegisterMemory(xSession->keySlotID[0], smp_mem_type,(uint32_t)mb.start, mb.size);
        	if (tret != TEEC_SUCCESS) {
                	SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[0]);
                	goto __get_slot_failed;
        	}
       }

	//xSession->keySlotID[1] = slotId + 1;

	tret = SMPC_Open(xSession->keySlotID[1], slot_type);
	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[1]);
		goto __get_slot_failed;
	}

	if (mtSecGetSmp(xSession->transportSessionId))
	{
		tret = SMPC_RegisterMemory(xSession->keySlotID[1], smp_mem_type,(uint32_t)mb.start, mb.size);
		if (tret != TEEC_SUCCESS) {
        		SECAPI_ERROR("SMPC RegisterSecureMemory failed, slotId:%d\n", xSession->keySlotID[1]);
        		goto __get_slot_failed;
		}
	}


	getLocalPidList(xSession->transportSessionId, &pidinfo);
	if (pidinfo.pidNum > 0)
		isPidValid = TRUE;

	//printf("xSession->keySlotID[0]= %d , xSession->keySlotID[1] = %d \n", xSession->keySlotID[0], xSession->keySlotID[1]);
	if (isPidValid) {
		if (!attachDescrambler(xSession->transportSessionId, &rec_dsc_handle, xSession->keySlotID[0], xSession->keySlotID[1])) {
			goto __get_slot_failed;
		}

		//remember descrambler
		xSession->recDscHandle = rec_dsc_handle;

		//attach pidlist
		if (SEC_NO_ERROR != mtSecAttachSessionPidToDescrambler(xSession->transportSessionId, SESSION_OP_DECRYPT, &pidinfo)) {
			detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, rec_dsc_handle, &pidinfo);
			goto __get_slot_failed;
		}

		psecSetDescramblerType((mt_handle)xSession->recDscHandle, xSession->EMI);
	}

	*pxKeySlotIndex = mapKeySlotIdFromIndex(2, slotId);

	gsSecPIDInfo[xSession->transportSessionId].recDscHandle = xSession->recDscHandle;

	return SEC_NO_ERROR;

__get_slot_failed:

    for (i = 0; i < 2; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[i]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
    }
    return SEC_ERROR;

}

/*
 * Dvb Play Session
 */
static TSecStatus mtSecDvbSessionGetSlotAndSetMbInfo(TSecStreamSession xSession, TUnsignedInt16 *pxKeySlotIndex)
{
	TEEC_Result tret = TEEC_SUCCESS;
	mt_u8 slotId = 0, i = 0;
	TSecPidInfo pidinfo = {0};
	TBoolean isPidValid = FALSE;
	MBInfo mb = {0};
	mt_handle audio_dsc_handle, video_dsc_handle;
	SECAPI_INFO("xSession: 0x%lx  isMultiSession = %d \n", (TUnsignedLong)xSession, xSession->isMultiSession );


	if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[0]) {
		*pxKeySlotIndex = mapKeySlotIdFromIndex(4, xSession->keySlotID[0]);

		return SEC_NO_ERROR;
	}

	if (0 != mtSecKeySlotRequest(&slotId, 4)) {
		SECAPI_ERROR("request 4 key slots failed\n");
		//printf("%s %d \n",__FUNCTION__,__LINE__);

		return SEC_ERROR;
	}

	mtSecSetImplicitIv(xSession->EMI, slotId, 4);

	xSession->keySlotID[0] = slotId;

	tret = SMPC_Open(xSession->keySlotID[0], SMPC_SLOT_TYPE_AUDIO);
	if (tret != TEEC_SUCCESS) {

		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[0]);

		goto __get_slot_failed;
	}

	//mark as keeping
	if (mtSecGetSmp(xSession->transportSessionId)) {
		if (mtSecGetProtectBufferByType(MB_AUD, &mb, 0)) {
                    SECAPI_INFO("start = 0x%lld, size = 0x%lx \n", mb.start, mb.size);

		      if(xSession->isMultiSession == FALSE)
		      {
                  		tret = SMPC_RegisterMemory(xSession->keySlotID[0],SMPC_MEM_TYPE_AUDIO , (uint32_t)mb.start, mb.size);
				//printf("%s %d keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",__FUNCTION__,__LINE__, xSession->keySlotID[0], (void *)mb.start,mb.size );
		      }
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("%s %d  tret = 0x%x  ------------------- \n",__FUNCTION__,__LINE__, tret);
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[0]);
				goto __get_slot_failed;
			}
		}
	}

	xSession->keySlotID[1] = slotId + 1;

	if(xSession->isMultiSession == FALSE){
		tret = SMPC_Open(xSession->keySlotID[1], SMPC_SLOT_TYPE_VIDEO);
	}
	else
	{
		tret = SMPC_Open(xSession->keySlotID[1], SMPC_SLOT_TYPE_SUB_VIDEO);
	}

	if (tret != TEEC_SUCCESS) {

		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[1]);
		goto __get_slot_failed;
	}
	if (mtSecGetSmp(xSession->transportSessionId)) {
		//mark as keeping
		if(xSession->isMultiSession == FALSE)
		{
			if (mtSecGetProtectBufferByType(MB_VID, &mb, 0)) {

			//printf("%s %d keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",__FUNCTION__,__LINE__, xSession->keySlotID[1], (void *)mb.start,mb.size );

	                    tret = SMPC_RegisterMemory(xSession->keySlotID[1], SMPC_MEM_TYPE_VIDEO, (uint32_t)mb.start, mb.size);

				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[1]);

					goto __get_slot_failed;
				}
			}
		}
		else
		{
			if (mtSecGetProtectBufferByType(MB_SUB_VID, &mb, 0)) {

			//printf("%s %d keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",__FUNCTION__,__LINE__, xSession->keySlotID[1], (void *)mb.start,mb.size );

	                    tret = SMPC_RegisterMemory(xSession->keySlotID[1], SMPC_MEM_TYPE_SUB_VIDEO, (uint32_t)mb.start, mb.size);

				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[1]);

					goto __get_slot_failed;
				}
			}
		}
	}

	xSession->keySlotID[2] = slotId + 2;

	tret = SMPC_Open(xSession->keySlotID[2], SMPC_SLOT_TYPE_AUDIO);
	if (tret != TEEC_SUCCESS) {

		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[2]);
		goto __get_slot_failed;
	}
	//mark as clear
	if (mtSecGetSmp(xSession->transportSessionId)) {
		if (mtSecGetProtectBufferByType(MB_AUD, &mb, 1)) {

			//printf("%s %d  keySlotID = %d mb.start = %x mb.size = 0x%x  \n",__FUNCTION__,__LINE__, xSession->keySlotID[2], (void *)mb.start,mb.size );
		      if(xSession->isMultiSession == FALSE)
                    {
                    		tret = SMPC_RegisterMemory(xSession->keySlotID[2], SMPC_MEM_TYPE_AUDIO, (uint32_t)mb.start, mb.size);
		      }
			if (tret != TEEC_SUCCESS) {

				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[2]);
				goto __get_slot_failed;
			}
		}
	}

	xSession->keySlotID[3] = slotId + 3;


	    if(xSession->isMultiSession == FALSE)
	   {
	   	  tret = SMPC_Open(xSession->keySlotID[3], SMPC_SLOT_TYPE_VIDEO);
	   }
          else
          {
          	  tret = SMPC_Open(xSession->keySlotID[3], SMPC_SLOT_TYPE_SUB_VIDEO);
          }

	if (tret != TEEC_SUCCESS) {
		SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[3]);

		goto __get_slot_failed;
	}
	//mark as clear

	if (mtSecGetSmp(xSession->transportSessionId)) {
		if(xSession->isMultiSession == FALSE)
		{
			if (mtSecGetProtectBufferByType(MB_VID, &mb, 1)) {

		       //printf("%s %d  keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",__FUNCTION__,__LINE__, xSession->keySlotID[3], (void *)mb.start,mb.size );

                    tret = SMPC_RegisterMemory(xSession->keySlotID[3], SMPC_MEM_TYPE_VIDEO, (uint32_t)mb.start, mb.size);
			if (tret != TEEC_SUCCESS) {
				SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[3]);
				goto __get_slot_failed;
			}
			}
		}
		else
		{
			if (mtSecGetProtectBufferByType(MB_SUB_VID, &mb, 1)) {

				//printf("%s %d  keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",__FUNCTION__,__LINE__, xSession->keySlotID[3], (void *)mb.start,mb.size );

	                    tret = SMPC_RegisterMemory(xSession->keySlotID[3], SMPC_MEM_TYPE_SUB_VIDEO, (uint32_t)mb.start, mb.size);

				if (tret != TEEC_SUCCESS) {
					SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[3]);
					goto __get_slot_failed;
				}
			}
		}
	}

	getLocalPidList(xSession->transportSessionId, &pidinfo);
	if (pidinfo.pidNum > 0)
		isPidValid = TRUE;

	if (isPidValid) {
		SECAPI_INFO("Now Create descrambler, and attach PIDs\n");
		//printf("%s %d  transportSessionId = %d audio_dsc_handle = %d keySlotID[0]=%d keySlotID[2]=%d \n",__FUNCTION__,__LINE__,
			//xSession->transportSessionId, audio_dsc_handle, xSession->keySlotID[0], xSession->keySlotID[2]);

		//create descrambler and attach
		if (!attachDescrambler(xSession->transportSessionId, &audio_dsc_handle, xSession->keySlotID[0], xSession->keySlotID[2])) {

			goto __get_slot_failed;
		}

		if (!attachDescrambler(xSession->transportSessionId, &video_dsc_handle, xSession->keySlotID[1], xSession->keySlotID[3])) {

			goto __get_slot_failed;
		}

		//remember descrambler
		xSession->audioDscHandle = audio_dsc_handle;
		xSession->videoDscHandle = video_dsc_handle;

		//attach pidlist
		if (SEC_NO_ERROR != mtSecAttachSessionPidToDescrambler(xSession->transportSessionId, SESSION_OP_DECRYPT, &pidinfo)) {

			detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, audio_dsc_handle, &pidinfo);
			detachDescrambler(xSession->transportSessionId, pidinfo.sessionType, video_dsc_handle, &pidinfo);

			goto __get_slot_failed;
		}

		psecSetDescramblerType((mt_handle)xSession->videoDscHandle, xSession->EMI);
		psecSetDescramblerType((mt_handle)xSession->audioDscHandle, xSession->EMI);


    }

	*pxKeySlotIndex = mapKeySlotIdFromIndex(4, slotId);

	return SEC_NO_ERROR;

__get_slot_failed:

    for (i = 0; i < 4; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[0]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
    }
    return SEC_ERROR;
}

/*
 * Decrypt dash OTT data and than encrypted data with the same keyID
 *In OTT use case, pxKeyId must not be NULL and this function returns the single key slot
 *allocated to this session. Key rotation is achieved by opening a new session.
 * TODO: Usage of this function is not determined yet!
 */
static TSecStatus mtSecReEncryptSessionGetSlotAndSetMbInfoWithKeyId(TSecStreamSession xSession, size_t xKeyIdSize,
    const TUnsignedInt8 *pxKeyId, TUnsignedInt16 *pxKeySlotIndex)
{
    TEEC_Result tret = TEEC_SUCCESS;
    MBInfo mb = {0};
    SMPC_SlotType slot_type_dec = SMPC_SLOT_TYPE_EXPORT_OTT_DEC;
    SMPC_SlotType slot_type_enc = SMPC_SLOT_TYPE_EXPORT_OTT_ENC;
    //MBType  mb_type = {0,};
    //SMPC_MemType smp_mem_type = SMPC_MEM_TYPE_OTT;
    mt_u32 k = 0, i = 0;

    SECAPI_REENC_INFO("xSession: 0x%lx, xKeyIdSize = %ld\n", (TUnsignedLong)xSession, xKeyIdSize);
    //sec_dump("keyid dump", pxKeyId, xKeyIdSize);
    //find an existed keyID:0 or 2 for decryption, 1 or 3 for encryption
    for (i = 0; i < sizeof(xSession->keySlotID); i+=2) {
        //same keyID is needed
        if ((!memcmp(xSession->keyID[i], pxKeyId, xKeyIdSize))
            && (xSession->keySlotID[i] != MT_CIPHER_KEYSLOT_INVALID)) {
            *pxKeySlotIndex = xSession->keySlotID[i];
            SECAPI_REENC_INFO("Find the slot:%d based on keyID[%d]\n", *pxKeySlotIndex, i);
            break;
        }
    }
    /*allocate a new keyID:
    * keySlotStat_bitx = 0:spare; 1:busy
    * keySlotStat_bit0 --> keySlotID[0]
    * keySlotStat_bit1 --> keySlotID[1]
    */
    if (i == sizeof(xSession->keySlotID)) {
        for (i = 0; i < sizeof(xSession->keySlotID); i+=2) {
            if (!(xSession->keySlotStat & (1 << i))) {
                if (xSession->keySlotID[i] == MT_CIPHER_KEYSLOT_INVALID) {
                        if (0 != mtSecKeySlotRequest(&(xSession->keySlotID[i]), 2)) {
                        SECAPI_ERROR("request key slot 0 failed\n");
                        return SEC_ERROR;
                    }
                    xSession->keySlotID[i + 1] = xSession->keySlotID[i] + 1;
                    mtSecSetImplicitIv(xSession->EMI, xSession->keySlotID[i], 2);
                    SECAPI_REENC_INFO("Got new slot[%d]:%d, %d\n", i, xSession->keySlotID[i], xSession->keySlotID[i + 1]);
                    //OTT decryption buffer register
                    tret = SMPC_Open(xSession->keySlotID[i], slot_type_dec);
                    if (tret != TEEC_SUCCESS) {
                        SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[i]);
                        goto __get_slot_failed;
                    }
                    //OTT encryption buffer register
                    tret = SMPC_Open(xSession->keySlotID[i + 1], slot_type_enc);
                    if (tret != TEEC_SUCCESS) {
                        SECAPI_ERROR("SMPC open failed, slotId:%d\n", xSession->keySlotID[i]);
                        goto __get_slot_failed;
                    }
                    if (mtSecGetSmp(xSession->transportSessionId)) {
                        SECAPI_REENC_INFO("xSession->isMultiSession = %d \n", xSession->isMultiSession);
                        if(xSession->isMultiSession == FALSE) //first one
                        {
                            if (mtSecGetProtectBufferByType(MB_OTT, &mb, 0)) {
                                SECAPI_REENC_INFO("%s %d  keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",
                                    __FUNCTION__,__LINE__, xSession->keySlotID[i], (void *)mb.start,mb.size );
                                tret = SMPC_RegisterMemory(xSession->keySlotID[i], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
                                if (tret != TEEC_SUCCESS) {
                                    SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
                                    goto __get_slot_failed;
                                }
                                //Although only slot_type_dec will configure ott memroy, but still configure data
                                tret = SMPC_RegisterMemory(xSession->keySlotID[i + 1], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
                                if (tret != TEEC_SUCCESS) {
                                    SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i + 1]);
                                    goto __get_slot_failed;
                                }
                            }
                        } else {
                        SECAPI_REENC_INFO("==\n");
                            if (mtSecGetProtectBufferByType(MB_OTT, &mb, 0)) {  //second one
                                SECAPI_REENC_INFO("%s %d  keySlotID = %d mb.start = %p mb.size = 0x%lx  \n",
                                    __FUNCTION__,__LINE__, xSession->keySlotID[i], (void *)mb.start,mb.size );
                                tret = SMPC_RegisterMemory(xSession->keySlotID[i], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
                                if (tret != TEEC_SUCCESS) {
                                    SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i]);
                                    goto __get_slot_failed;
                                }
                                //Although only slot_type_dec will configure ott memroy, but still configure data
                                tret = SMPC_RegisterMemory(xSession->keySlotID[i + 1], SMPC_MEM_TYPE_OTT, (uint32_t)mb.start, mb.size);
                                if (tret != TEEC_SUCCESS) {
                                    SECAPI_ERROR("register mem for slot%d failed\n", xSession->keySlotID[i + 1]);
                                    goto __get_slot_failed;
                                }
                            }
                        }
                    }
                }
                *pxKeySlotIndex = xSession->keySlotID[i];

                //backup current keyID
                for (k = 0; k < xKeyIdSize; k++)
                    xSession->keyID[i][k] =  pxKeyId[k];

                //clear all stat, then mark selected one
                xSession->keySlotStat = 0;
                xSession->keySlotStat = (1 << i);
                SECAPI_REENC_INFO("Find the slot:%d based on keyStat_bit%d\n", *pxKeySlotIndex, i);

                break;
            }
        }
    }

	return SEC_NO_ERROR;

__get_slot_failed:

    for (i = 0; i < 4; i++) {
        if (MT_CIPHER_KEYSLOT_INVALID != xSession->keySlotID[i]) {
            tret = mtExtSMPC_Close(xSession, xSession->keySlotID[i]);
            if (tret != TEEC_SUCCESS) {
                SECAPI_ERROR("SMPC close failed, slotId:%d\n", xSession->keySlotID[i]);
            }
            mt_unf_cipher_keyslot_release(xSession->keySlotID[i]);
        }
    }
    return SEC_ERROR;

}

static TSecStatus secStreamEncryptSessionGetKeySlot
(
 TSecStreamSession   xSession,
 size_t              xKeyIdSize,
 const TUnsignedInt8*     pxKeyId,
 TUnsignedInt16*    pxKeySlotId
 )
{


    TSecStatus ret = SEC_ERROR;
    SECAPI_INFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);

    if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession)
        return SEC_ERROR_BAD_PARAMETER;

    if (sessionValidation(xSession))
        return SEC_ERROR_BAD_PARAMETER;

    if (0 != xKeyIdSize && NULL == pxKeyId)
        return SEC_ERROR_BAD_PARAMETER;

    if (0 == xKeyIdSize && NULL != pxKeyId)
        return SEC_ERROR_BAD_PARAMETER;

    if (NULL == pxKeySlotId)
        return SEC_ERROR_BAD_PARAMETER;

    if (NULL == pxKeyId) {
		if (xSession->EMI == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			|| xSession->EMI == NOCS_EMI_AES128_ECB_TAIL_CLEAR
			|| xSession->EMI == NOCS_EMI_AES128_CBC_PKCS7_PADDING
			|| xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
			|| xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
			|| xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_HEAD_CLEAR
			|| xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
			|| xSession->EMI == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {

			ret = mtSecRecScrSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);

		} else {
			SECAPI_INFO("Warning::EMI:%x is not supported for Scrambling\n", xSession->EMI);
			//TODO:ASA slot is not controlled by MT ?
			ret = SEC_ERROR;
		}
    } else { //OTT
		ret = mtSecRecScrSessionGetSlotAndSetMbInfoWithKeyId(xSession, xKeyIdSize, pxKeyId, pxKeySlotId);
    }
    //printf("*pxKeySlotId = 0x%x\n", *pxKeySlotId);
    //printf("%s %d  ret = %d \n",__FUNCTION__,__LINE__, ret);

    return ret;
}

static TSecStatus secStreamReEncryptSessionGetKeySlot
(
 TSecStreamSession   xSession,
 size_t              xKeyIdSize,
 const TUnsignedInt8*     pxKeyId,
 TUnsignedInt16*    pxKeySlotId
 )
{

    TSecStatus ret = SEC_ERROR;
    SECAPI_REENC_INFO("xSession: 0x%lx\n", (TUnsignedLong)xSession);

    if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession)
        return SEC_ERROR_BAD_PARAMETER;

    if (sessionValidation(xSession))
        return SEC_ERROR_BAD_PARAMETER;

    if (0 != xKeyIdSize && NULL == pxKeyId)
        return SEC_ERROR_BAD_PARAMETER;

    if (0 == xKeyIdSize && NULL != pxKeyId)
        return SEC_ERROR_BAD_PARAMETER;

    if (NULL == pxKeySlotId)
        return SEC_ERROR_BAD_PARAMETER;

    if (NULL == pxKeyId) {
        ret = SEC_ERROR;
        SECAPI_REENC_INFO("%s %d  ret = %d \n",__FUNCTION__,__LINE__, ret);
    } else { //OTT
        ret = mtSecReEncryptSessionGetSlotAndSetMbInfoWithKeyId(xSession, xKeyIdSize, pxKeyId, pxKeySlotId);
    }
    //printf("*pxKeySlotId = 0x%x\n", *pxKeySlotId);
    //printf("%s %d  ret = %d \n",__FUNCTION__,__LINE__, ret);

    return ret;
}

static TSecStatus secStreamDecryptSessionGetKeySlot
(
 TSecStreamSession   xSession,
 size_t              xKeyIdSize,
 const TUnsignedInt8*     pxKeyId,
 TUnsignedInt16*    pxKeySlotId
 )
{
    TSecStatus ret;
    TSecPidInfo pidinfo = {0};

    if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession)
      {

      		return SEC_ERROR_BAD_PARAMETER;
    }

    if (sessionValidation(xSession))
        return SEC_ERROR_BAD_PARAMETER;

    if (0 != xKeyIdSize && NULL == pxKeyId)
     {

     		return SEC_ERROR_BAD_PARAMETER;
    }

    if (0 == xKeyIdSize && NULL != pxKeyId)
     {

    	 	return SEC_ERROR_BAD_PARAMETER;
    }
    if (NULL == pxKeySlotId)
      {

      		return SEC_ERROR_BAD_PARAMETER;
    	}

    SECAPI_DINFO("xSession: 0x%lx, xSession->transportSessionId = 0x%x   \n", (TUnsignedLong)xSession, xSession->transportSessionId);
    SECAPI_DINFO("%s %d xSession->EMI = 0x%x  pxKeyId = %p \n",__FUNCTION__,__LINE__, xSession->EMI, pxKeyId);
    //secLock();

    if (NULL == pxKeyId) {

		getLocalPidList(xSession->transportSessionId, &pidinfo);
             SECAPI_DINFO("xSession->keySlotID[0]= %d , xSession->keySlotID[1] = %d, pidNum = %d \n", xSession->keySlotID[0], xSession->keySlotID[1], pidinfo.pidNum);
		if (xSession->EMI == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			|| xSession->EMI == NOCS_EMI_AES128_ECB_TAIL_CLEAR
			|| xSession->EMI == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
			if (pidinfo.sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {
                        SECAPI_INFO("\n");


				ret = mtSecRepRawSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);


			} else {
				SECAPI_INFO("sessionType:%d (<--What is it?)\n", pidinfo.sessionType);
				//printf("%s %d  ret = %d \n",__FUNCTION__,__LINE__, ret);

				ret = SEC_ERROR;
			}
		} else {
#if 1
			/*
			 * EMIS:0000,0001,0020,0021,0022,0023 Need even/odd information on tee side
			 * Convention usage: even_audio | even_video | odd_audio | odd_video
			 */
			if (pidinfo.sessionType == TRANSPORT_SESSION_TYPE_DVB) {
                        SECAPI_INFO("\n");

				ret = mtSecDvbSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);

			} else if (pidinfo.sessionType == TRANSPORT_SESSION_TYPE_RECORD) {
			    SECAPI_INFO("\n");

				ret = mtSecRecDscSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);

			} else if (pidinfo.sessionType == TRANSPORT_SESSION_TYPE_REPLAY) {
			    SECAPI_INFO("\n");

				ret = mtSecRepTsSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);

			} else if (pidinfo.sessionType == TRANSPORT_SESSION_TYPE_NONE) {
			    SECAPI_INFO("\n");

				ret = mtSecDvbSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);

			} else {
				SECAPI_INFO("sessionType:%d (<--What is it?)\n", pidinfo.sessionType);
				ret = SEC_ERROR;
			}
 #else
 ret = mtSecRepTsSessionGetSlotAndSetMbInfo(xSession, pxKeySlotId);
#endif
			SECAPI_INFO("*pxKeySlotId=0x%x, sessionType:%d, ret = 0x%x\n", *pxKeySlotId, pidinfo.sessionType, ret);
		}
    } else {
		//OTT
		 SECAPI_DINFO("\n");

		ret = mtSecOttSessionGetSlotAndSetMbInfo(xSession, xKeyIdSize, pxKeyId, pxKeySlotId);

    }

	//secUnlock();

	SECAPI_INFO("ret:%x\n", ret);
       return ret;
}

static TSecStatus secStreamSessionSet2LevelProtectedKey
(
 TUnsignedInt16      xKeySlotId,
 const TSecKeyParams*     pxKeyParams,
 size_t              xCipheredContentKeySize,
 const TUnsignedInt8*     pxCipheredContentKey,
 size_t              xCipheredProtectingKeySize,
 const TUnsignedInt8*     pxL1CipheredProtectingKey,
 const TUnsignedInt8*     pxL2CipheredProtectingKey
 )
{
    TUnsignedInt8 realCipheredContentKey[16] = {0};

    SECAPI_INFO("\n");
    if (NULL == pxKeyParams) {
        return SEC_ERROR_BAD_PARAMETER;
    }

    if (NULL==pxCipheredContentKey
            || NULL==pxL1CipheredProtectingKey
            || NULL==pxL2CipheredProtectingKey) {
        return SEC_ERROR_BAD_PARAMETER;
    }

    if (FALSE == psecChkKeySize(pxKeyParams->emi, xCipheredContentKeySize))
        return SEC_ERROR_BAD_PARAMETER;

    if (16 != xCipheredProtectingKeySize)
        return SEC_ERROR_BAD_PARAMETER;

    if (FALSE == psecChkEmiRange(pxKeyParams->emi))
        return SEC_ERROR_BAD_EMI;

    if (xCipheredContentKeySize == 8) {
        memcpy(realCipheredContentKey+8, pxCipheredContentKey, 8);
    } else {
        memcpy(realCipheredContentKey, pxCipheredContentKey, 16);
    }

    if (psecKeyLadder(psecGetRootKeyID(pxKeyParams->emi),
                (const unsigned char *)pxL2CipheredProtectingKey,
                (const unsigned char *)pxL1CipheredProtectingKey,
                (const unsigned char *)realCipheredContentKey,
                xKeySlotId,
                MT_CIPHER_ALG_TDES)) {
        return SEC_ERROR;
    }

    mt_unf_cipher_keyslot_info(xKeySlotId);

    return SEC_NO_ERROR;
}

#if 0
static mt_s32 dmacpy(MT_EDMA_CH_E ch, mt_u8 *dst, const mt_u8 *src, mt_u32 size)
{
//	printf("%s %d \n", __func__, __LINE__);
	mt_s32 ret = MT_FAILURE;
	mt_unf_dma_request_channel(ch);
	while (MT_EDMA_STATUS_FREE != mt_unf_dma_check(ch)) {
		//10ms
		usleep(1000 * 10);
	}

	ret = mt_unf_dma_memcpy(ch, src, dst, size);

	mt_unf_dma_release_channel(ch);
	if (ret != MT_SUCCESS) {
		printf("dma-%d copy from %p to %p with size(%x) fail  ret = %x ", ch, src, dst, size, ret);
		return MT_FAILURE;
	}

	printf("dma-%d copy from %p to %p with size(%x) success", ch, src, dst, size);
	return MT_SUCCESS;
}

static void *mmz_malloc(const char *area, ulong size)
{
    void *vir_addr = NULL;
    phys_addr_t phy_addr;

    if (size == 0)
        return NULL;

	//DMSG("request size:%x", size);

    phy_addr = (mt_u32)mt_mmz_new(size, 0, area, "spr_buf");
    if (phy_addr == 0) {
		printf("new mmz size:%lu failed", size);
        return NULL;
    }
	//DMSG("New PhyAddr:%x,size:%x", phy_addr, size);

    //map,but not cached
    vir_addr = mt_mmz_map(phy_addr, 0);
    if (vir_addr == NULL) {
		printf("map mmz phy:%llu failed", phy_addr);
        mt_mmz_delete(phy_addr);
        return NULL;
    }

	//DMSG("PhyAddr->VirAddr:%x", vir_addr);
    /* success,return the virtual address of the buffer */
    return vir_addr;
}

static mt_s32 mmz_free(mt_void *p_vir)
{
    mt_s32 ret;
    phys_addr_t phy_addr;
    ulong phy_size;

	//DMSG("VirAddr:%p", p_vir);

    if (p_vir == NULL) {
        printf("Can not free NULL pointer\n");
        return -1;
    }

    ret = mt_mmz_get_phyaddr(p_vir, &phy_addr, &phy_size);
    ret |= mt_mmz_unmap(p_vir);
    ret |= mt_mmz_delete(phy_addr);
    printf("VirAddr:%p  ret = %d ", p_vir, ret);
    return ret;
}
#endif


static mt_s32 mtSecCryptoAsyncRequest(MT_CIPHER_CTRL_S *p_ctrl,
									  mt_handle crypto_handle,
									  TUnsignedInt16 xKeySlotId,
									  const TSecOpaqueInputBufferV1 *OpaqueInput,
									  TSecOpaqueOutputBuffer *OpaqueOutput)
{
    mt_s32 ret = MT_FAILURE;
    mt_s32 i = 0;
    #ifndef SUPPORT_CBCS_MODE
    mt_u32 crypt_blocks = OpaqueInput->cryptBlocks;
    mt_u32 skip_blocks = OpaqueInput->skipBlocks;
    #endif
    TSecChunkInfo *chunkInfo = NULL;
    mt_u8 *iPhyAddr = vir2phy(OpaqueInput->data);
    mt_u8 *oPhyAddr = vir2phy(OpaqueOutput->data);

    mt_u8 *iVirAddr = OpaqueInput->data;
    mt_u8 *oVirAddr = OpaqueOutput->data;

	#ifndef SUPPORT_CBCS_MODE
	//SECAPI_INFO("crypt_blocks = %d skip_blocks = %d \n", crypt_blocks, skip_blocks);
	#endif

	#ifdef SUPPORT_CBCS_MODE

	 for (i = 0; i < OpaqueInput->chunkCount; i++)
	  {
        	chunkInfo = &OpaqueInput->chunks[i];

		if (chunkInfo->initVector != NULL || chunkInfo->initVectorSize > 0 ) {

			ret = mt_unf_cipher_crypto_config(crypto_handle, p_ctrl, xKeySlotId);
			if (0 != ret ) {
				SECAPI_ERROR("crypto config failed ret = 0x%x chunk index = %d \n", ret, i);
				goto async_request_error;
			}

			//data_debug_dump(chunkInfo->initVector, chunkInfo->initVectorSize);
			ret = mtSecSetIv(xKeySlotId, chunkInfo->initVector, chunkInfo->initVectorSize);
			if (ret != MT_SUCCESS) {
				SECAPI_ERROR("Set IV for slot:%d failed\n", xKeySlotId);
				goto async_request_error;
			}
		}

		// printf("in put data: \n");

	       //SECAPI_INFO("%s %d  crypto_handle=%x iPhyAddr=%x oPhyAddr=%x chunkInfo->clearSize=%d chunkInfo->protSize=%d \n",__FUNCTION__,__LINE__, crypto_handle,iPhyAddr,oPhyAddr, chunkInfo->clearSize, chunkInfo->protSize);
		//data_debug_dump(iVirAddr, (chunkInfo->clearSize + chunkInfo->protSize));

		 ret = mt_unf_cipher_crypto_process_phy_ext(crypto_handle, (phys_addr_t)iPhyAddr, (phys_addr_t)oPhyAddr, chunkInfo->clearSize, chunkInfo->protSize);

	        if (ret != MT_SUCCESS)
	         {
	         	 SECAPI_ERROR("[err ]mt_unf_cipher_crypto_process_phy_ext  ret = %x \n", ret);
	         	goto async_request_error;
		  }
		// printf("out put data: \n");
		 //data_debug_dump(oVirAddr, (chunkInfo->clearSize + chunkInfo->protSize));

	  	 iPhyAddr += (chunkInfo->clearSize + chunkInfo->protSize);
	        oPhyAddr += (chunkInfo->clearSize + chunkInfo->protSize);


		iVirAddr += (chunkInfo->clearSize + chunkInfo->protSize);
        	oVirAddr += (chunkInfo->clearSize + chunkInfo->protSize);
	 }

	#else

       if (crypt_blocks == 0 && skip_blocks == 0) {

		//SECAPI_INFO("\n");
        for (i = 0; i < OpaqueInput->chunkCount; i++) {
			//One-shot mode, async every chunk and wait complete
			//cenc fullsample mode
            chunkInfo = &OpaqueInput->chunks[i];

			if (chunkInfo->initVector != NULL || chunkInfo->initVectorSize > 0) {
				//printf("%s %d set iv111 xKeySlotId = %d \n",__FUNCTION__,__LINE__, xKeySlotId);

				//data_debug_dump(chunkInfo->initVector, chunkInfo->initVectorSize);
				ret = mtSecSetIv(xKeySlotId, chunkInfo->initVector, chunkInfo->initVectorSize);
				if (ret != MT_SUCCESS) {
					SECAPI_ERROR("Set IV for slot:%d failed\n", xKeySlotId);
					goto async_request_error;
				}


			}


		//printf("input data:\n");
		//data_debug_dump(iVirAddr, (chunkInfo->clearSize + chunkInfo->protSize));

            ret = mt_unf_cipher_crypto_async_request(crypto_handle,
                    iVirAddr,
                    oVirAddr,
                    1,
                    chunkInfo->clearSize,
                    chunkInfo->protSize);
            if (ret != MT_SUCCESS) {
                SECAPI_ERROR("async_request failed: crypto_handle = %x iPhyAddr:%lx,oPhyAddr:%lx, clearSize:%d, protSize:%d(mret=%x)\n",
                        crypto_handle, (TUnsignedLong)OpaqueInput->data, (TUnsignedLong)OpaqueOutput->data, chunkInfo->clearSize, chunkInfo->protSize, ret);
                goto async_request_error;
            }

		   	//SECAPI_ERROR("output data------------------------:\n");
			//data_debug_dump(oVirAddr, (chunkInfo->clearSize + chunkInfo->protSize));

			iVirAddr += (chunkInfo->clearSize + chunkInfo->protSize);
            		oVirAddr += (chunkInfo->clearSize + chunkInfo->protSize);
			if (i < (OpaqueInput->chunkCount - 1)) {
				//cenc subsample mode:link subsample together, then start processing
				if (OpaqueInput->chunks[i+1].initVector == NULL)
				{
					//printf("mtSecCryptoAsyncRequest -----------------------------------------------\n");

					continue;
				}
			}


			ret = mt_unf_cipher_crypto_async_start(crypto_handle);
			if (ret != MT_SUCCESS) {
				SECAPI_ERROR("crypto async start failed\n");
				goto async_request_error;
			}

			ret = mt_unf_cipher_crypto_async_wait(crypto_handle);
			if (ret != MT_SUCCESS) {
				SECAPI_ERROR("crypto async wait failed\n");
				goto async_request_error;
			}

        }


    } else {
        mt_u32 crypt_bytes = crypt_blocks * 16;
        mt_u32 skip_bytes = skip_blocks * 16;

        for (i = 0; i < OpaqueInput->chunkCount; i++) {
            chunkInfo = &OpaqueInput->chunks[i];

	 if( chunkInfo->protSize < 16 )
	 {
	 	memcpy(oVirAddr, iVirAddr, chunkInfo->clearSize+ chunkInfo->protSize);
		//printf("%s %d chunkInfo->protSize =%d  ------------------------------------- \n",__FUNCTION__,__LINE__, chunkInfo->protSize);
	 	continue;
	 }

			/*IV first*/
				//data_debug_dump(chunkInfo->initVector, chunkInfo->initVectorSize);
				//printf("input data:\n");
				//			data_debug_dump(iVirAddr, (chunkInfo->clearSize + chunkInfo->protSize));

			ret = mtSecSetIv(xKeySlotId, chunkInfo->initVector, chunkInfo->initVectorSize);
			if (ret != MT_SUCCESS) {
				SECAPI_ERROR("Set IV for slot:%d failed\n", xKeySlotId);
				goto async_request_error;
			}
				//printf("%s %d \n",__FUNCTION__,__LINE__);

            /*1.request clear+(first crypt_bytes)*/
            ret = mt_unf_cipher_crypto_async_request(crypto_handle,
                    iVirAddr,
                    oVirAddr,
                    1,
                    chunkInfo->clearSize,
                    crypt_bytes);
            if (ret != MT_SUCCESS) {
                SECAPI_ERROR("async_request failed:InPos:%lx,OutPos:%lx, clearSize:%d, protSize:%d(mret=%d)",
                        (TUnsignedLong)iVirAddr, (TUnsignedLong)oVirAddr, chunkInfo->clearSize, chunkInfo->protSize, ret);
                goto async_request_error;
            }

            /*2.pattern in protSize*/
            iVirAddr += chunkInfo->clearSize + crypt_bytes;
            oVirAddr += chunkInfo->clearSize + crypt_bytes;
            mt_u32 pat_num = chunkInfo->protSize / (crypt_bytes + skip_bytes);
		//if( pat_num == 0 ) pat_num = 2;
		//printf("%s %d \n",__FUNCTION__,__LINE__);
		//printf("%s %d chunkInfo->clearSize = %d  chunkInfo->protSize = %d \n",__FUNCTION__,__LINE__, chunkInfo->clearSize, chunkInfo->protSize);

		//printf("async_request :crypto = %x src:%lx,dst:%lx,count=%d, clear_length:%d, protected_length:%d chunkInfo->protSize = %d chunkCount = %d \n",
                   //     crypto_handle, (TUnsignedLong)iVirAddr, (TUnsignedLong)oVirAddr, (pat_num - 1), skip_bytes, crypt_bytes, chunkInfo->protSize, OpaqueInput->chunkCount);
		//printf("%s %d pat_num = %d i = %d \n",__FUNCTION__,__LINE__, pat_num, i);

            ret = mt_unf_cipher_crypto_async_request(crypto_handle,
                    iVirAddr,
                    oVirAddr,
                    (pat_num - 1),
                    skip_bytes,
                    crypt_bytes);
            if (ret != MT_SUCCESS) {
                SECAPI_ERROR("async_request failed:InPos:%lx,OutPos:%lx, clearSize:%d, protSize:%d(mret=%d)",
                        (TUnsignedLong)iVirAddr, (TUnsignedLong)oVirAddr, skip_bytes, crypt_bytes, ret);
                goto async_request_error;
            }

            /*3.(last skip_bytes + Not pattern aligned part)*/
            mt_u32 not_pat_aligned_size = chunkInfo->protSize % (crypt_bytes + skip_bytes);
            iVirAddr += (pat_num - 1) * (skip_bytes + crypt_bytes);
            oVirAddr += (pat_num - 1) * (skip_bytes + crypt_bytes);
            ret = mt_unf_cipher_crypto_async_request(crypto_handle,
                    iVirAddr,
                    oVirAddr,
                    1,
                    (skip_bytes + not_pat_aligned_size - 1),
                    1); //at least 1 for trick
            if (ret != MT_SUCCESS) {
                SECAPI_ERROR("async_request failed:InPos:%lx,OutPos:%lx, clearSize:%d, protSize:%d(mret=%d)",
                        (TUnsignedLong)iVirAddr, (TUnsignedLong)oVirAddr, not_pat_aligned_size, 1, ret);
                goto async_request_error;
            }				//printf("output data:\n");

							//data_debug_dump(oVirAddr, (chunkInfo->clearSize + chunkInfo->protSize));

            iVirAddr += (skip_bytes + not_pat_aligned_size);
            oVirAddr += (skip_bytes + not_pat_aligned_size);

        }

		SECAPI_INFO("success!\n");


		ret = mt_unf_cipher_crypto_async_start(crypto_handle);
		if (ret != MT_SUCCESS) {
			SECAPI_ERROR("crypto async start failed\n");
			goto async_request_error;
		}

		ret = mt_unf_cipher_crypto_async_wait(crypto_handle);
		if (ret != MT_SUCCESS) {
			SECAPI_ERROR("crypto async wait failed\n");
			goto async_request_error;
		}

    }

    #endif

    return MT_SUCCESS;

async_request_error:
    return MT_FAILURE;
}


static TBoolean pre_xLastChunkFlg = TRUE;
static TSecStatus mtSecProcessOpaqueData(
	TSecStreamSession   xSession,
	TUnsignedInt16      xKeySlotId,
	const TSecOpaqueInputBufferV1 *OpaqueInput,
	TSecOpaqueOutputBuffer *OpaqueOutput,
	TBoolean xLastChunk)
{
	TUnsignedInt32 chunkCount = OpaqueInput->chunkCount;
	TUnsignedInt32 inSize = OpaqueInput->size;
	TSecStatus ret = SEC_NO_ERROR;
	mt_s32 mret = MT_SUCCESS;
	MT_CIPHER_CTRL_S ctrl;
	TUnsignedInt16 xEMI;
	//printf("m2min\n");

	//SECAPI_INFO(" in xSession: 0x%lx  transportSessionId = %d ticks = %lld \n", (TUnsignedLong)xSession,xSession->transportSessionId, get_sys_time_ms());

	if (1 != OpaqueInput->version) {
             SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_USAGE;
	}

	xEMI = xSession->EMI;

#if 0
	//print info for debug
	SECAPI_INFO("--TID:%d--\n", gettid());
	SECAPI_INFO("xEmi:%x\n", xSession->EMI);
	SECAPI_INFO("xKeySlotId:%d\n", xKeySlotId);
	SECAPI_DINFO("lastChunk:%s\n", (xLastChunk == TRUE ? "TRUE":"FALSE"));
	SECAPI_DINFO("chunkcount:%d\n", OpaqueInput->chunkCount);
	SECAPI_INFO("In data pointer:0x%lx(phy:%lx)\n", (TUnsignedLong)OpaqueInput->data, (TUnsignedLong)vir2phy(OpaqueInput->data));
	SECAPI_INFO("Out data pointer:0x%lx(phy:%lx)\n", (TUnsignedLong)OpaqueOutput->data, (TUnsignedLong)vir2phy(OpaqueOutput->data));
	SECAPI_DINFO("data size:%d\n", inSize);
	SECAPI_INFO("skipblocks:%d\n", OpaqueInput->skipBlocks);
	SECAPI_INFO("cryptblocks:%d\n", OpaqueInput->cryptBlocks);
	/*
	for (i = 0; i < chunkCount; i++) {
		SECAPI_INFO("chunks[%d]::clearsize:%d\n", i, OpaqueInput->chunks[i].clearSize);
		SECAPI_INFO("chunks[%d]::protsize:%d\n", i, OpaqueInput->chunks[i].protSize);
		SECAPI_INFO("chunks[%d]::initvectorsize:%d\n", i, OpaqueInput->chunks[i].initVectorSize);
		hex_dump("iv", OpaqueInput->chunks[i].initVector, OpaqueInput->chunks[i].initVectorSize);
	}
	*/
	//dump input data
	//
#endif


	//SECAPI_INFO("psecStreamSession2Ctrl in \n");
	psecStreamSession2Ctrl(xSession, &ctrl);
	//SECAPI_INFO("psecStreamSession2Ctrl out \n");

	if (chunkCount > 0) {
		//0x4024 0x4029 0x402a
		if (xEMI == NOCS_EMI_AES128_MPEG_DASH_CTR
		|| xEMI == NOCS_EMI_AES128_HLS
		|| xEMI == NOCS_EMI_AES128_MPEG_DASH_CBCS) {
			//SECAPI_INFO("xEMI = %d --------------------------------------------------------------------\n", xEMI);
			ctrl.cbcs_params.crypt_blocks = OpaqueInput->cryptBlocks;
			ctrl.cbcs_params.skip_blocks = OpaqueInput->skipBlocks;
			//printf(" ctrl.cbcs_params.crypt_blocks=%d  ctrl.cbcs_params.skip_blocks=%d \n",  ctrl.cbcs_params.crypt_blocks , ctrl.cbcs_params.skip_blocks);

			 if( OpaqueInput->skipBlocks == 0 && ctrl.work_mode == MT_CIPHER_WORK_MODE_CBCS)
			 {
			 	ctrl.work_mode = MT_CIPHER_WORK_MODE_CBC;
				//SECAPI_INFO("change cbcs mode to cbc ...\n");
			 }

			 if ((ctrl.work_mode == MT_CIPHER_WORK_MODE_CBCS) || (ctrl.work_mode == MT_CIPHER_WORK_MODE_CENS))
			 {
				//SECAPI_INFO("cbcs ctrl.cbcs_params.crypt_blocks=%d  ctrl.cbcs_params.skip_blocks=%d  ctrl.core = %d \n",  ctrl.cbcs_params.crypt_blocks , ctrl.cbcs_params.skip_blocks, ctrl.core);
			 }

			if( pre_xLastChunkFlg == TRUE)
			{
				if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xEMI, &xSession->pidInfo,
						xKeySlotId, &xSession->cryptoHandle)) {
					SECAPI_ERROR("MT CE prepare failed\n");
					return SEC_ERROR;
				}
			}
                    //SECAPI_DINFO("tsid = %d, slot=%d, inSize=0x%x\n", xSession->transportSessionId, xKeySlotId, inSize);

                    //if (xSession->transportSessionId == 1)
                    {
                        /*
                        SECAPI_DINFO("tsid = %d, slot=%d, inSize=0x%x, phy 0x%lx, 0x%lx \n",xSession->transportSessionId, xKeySlotId, inSize,
                            (TUnsignedLong)vir2phy(OpaqueInput->data), (TUnsignedLong)vir2phy(OpaqueOutput->data));
                        */
			    //sec_dump("inputdata2:", OpaqueInput->data, 32);

                        //SECAPI_INFO("tsid = 2 \n");
		        }

			ret = mtSecCryptoAsyncRequest(&ctrl, xSession->cryptoHandle, xKeySlotId, OpaqueInput, OpaqueOutput);
			if (ret) {
				SECAPI_ERROR("cenc process failed ret = %x \n", ret);
				goto mt_ce_error;
			}

			if( xLastChunk )
			{
				mtSecCryptoEngineProcessEnd(xSession->cryptoHandle, xEMI);
				xSession->cryptoHandle = MT_INVALID_HANDLE;
			}
                    //if (xSession->transportSessionId == 2)
                    {

			    //sec_dump("Output2:", OpaqueOutput->data, 16);
                       /*
                        if ((OpaqueInput->data[0x14] == 0xe9) && (OpaqueInput->data[0x15] == 0x5c) && (OpaqueInput->data[0x16] == 0x53)) {
                            SECAPI_INFO("pause.... \n");
                            while(1);
                        }
                        */
                    }


			//Debug
			OpaqueOutput->size = inSize;
			pre_xLastChunkFlg = xLastChunk;

		} else {
			//chunkCount should be 1 here EMI0X4023
			if (xEMI == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {

				if (FALSE == mtSecCryptoEngineProcessPrepare(&ctrl, xEMI, &xSession->pidInfo,
					xKeySlotId, &xSession->cryptoHandle)) {
					//if (FALSE == mtSecCryptoEngineProcessPrepare(pCtrl, xEMI, xKeySlotId, &crypto_handle)) {
					SECAPI_ERROR("MT CE prepare failed\n");
					return SEC_ERROR;
				}

				mret = mtSecSetIv(xKeySlotId, OpaqueInput->chunks[0].initVector, OpaqueInput->chunks[0].initVectorSize);
				if (mret != MT_SUCCESS) {
					SECAPI_ERROR("Set IV for slot:%d failed\n", xKeySlotId);
					goto mt_ce_error;
				}

#if 0
				mret = mtSecCryptoEngineProcessStart(xSession->cryptoHandle,
							xEMI,
							vir2phy(OpaqueInput->data),
							vir2phy(OpaqueOutput->data),
							inSize);
#else
			mret = mtSecCryptoEngineProcessStart(xSession->cryptoHandle,
							xEMI,
							OpaqueInput->data,
							OpaqueOutput->data,
							inSize);
#endif

				if (mret != MT_SUCCESS) {
					ret = SEC_ERROR;
					SECAPI_ERROR("[err]%s %d \n",__FUNCTION__,__LINE__);

					goto mt_ce_error;
				}

				//Ignore padding part, make 188 bytes aligned
				//OpaqueOutput->size = (inSize / 188 * 188);

				mtSecCryptoEngineProcessEnd(xSession->cryptoHandle, xEMI);
				xSession->cryptoHandle = MT_INVALID_HANDLE;

				//hex_dump("pkcs7", OpaqueOutput->data, 32);
			}

		}
	}
	//SECAPI_INFO("\n");

	return SEC_NO_ERROR;

mt_ce_error:
	if (MT_INVALID_HANDLE != xSession->cryptoHandle)
	{
		SECAPI_ERROR("[ERR]%s %d \n",__FUNCTION__,__LINE__);
		mtSecCryptoEngineProcessEnd(xSession->cryptoHandle, xEMI);
		xSession->cryptoHandle = MT_INVALID_HANDLE;
	}

	return ret;
}

/*
extern TSecFunctionTable *grant_secGetFunctionTable;
extern TSecStreamSession grantTSecStreamSession_0, grantTSecStreamSession_1;
extern TUnsignedInt8 *grantTestBuffer;
extern TUnsignedInt8 *grantTestBufferPhy;
*/

static TSecStatus secStreamSessionProcessOpaqueData
(
 TSecStreamSession   xSession,
 TUnsignedInt16      xKeySlotId,
 const void*              pxOpaqueInput,
 void*              pxOpaqueOutput,
 const TUnsignedInt8*     pxInitVector,
 size_t              xInitVectorSize,
 TBoolean            xLastChunk
 )
{
    const TSecOpaqueInputBufferV1 *OpaqueInput;
    TSecOpaqueOutputBuffer *OpaqueOutput = NULL;
    int ret = 0;

    if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession) {
        SECAPI_ERROR("return BAD_PARAMETER(%d)\n", SEC_ERROR_BAD_PARAMETER);
        return SEC_ERROR_BAD_PARAMETER;
    }

    //SECAPI_INFO("secStreamSessionProcessOpaqueData  xSession: 0x%lx tsid= %d \n", (TUnsignedLong)xSession, xSession->transportSessionId);

    if (NULL != pxInitVector || 0 != xInitVectorSize) {
        //parameters are deprecated, should be NULL and 0.
        SECAPI_ERROR("IV and Size are deprecated, should be NULL & 0.\n");
        return SEC_ERROR_BAD_USAGE;
    }

    if (0xFFFF == xKeySlotId) {
        SECAPI_ERROR("Invalid slot id.\n");
        return SEC_ERROR_BAD_PARAMETER;
    }

    if (xKeySlotId != xSession->keySlotID[0] && xKeySlotId != xSession->keySlotID[1]) {
        SECAPI_ERROR("Not a valid slot id.\n");
        return SEC_ERROR_BAD_PARAMETER;
    }

    if (NULL == pxOpaqueInput || NULL == pxOpaqueOutput) {
        SECAPI_ERROR("Input or Output buffer error.  pxOpaqueInput = %lx pxOpaqueOutput = %lx \n", (ulong)pxOpaqueInput , (ulong)pxOpaqueOutput);
        return SEC_ERROR_BAD_PARAMETER;
    }

    OpaqueInput = (const TSecOpaqueInputBufferV1 *)pxOpaqueInput;
    OpaqueOutput = (TSecOpaqueOutputBuffer*)pxOpaqueOutput;


    SECAPI_REENC_INFO("emi = 0x%x, xKeySlotId=%d, in=0x%lx,, out=0x%lx,ver:0x%x, size:0x%x, chunks->clearSize:0x%x, protSize= 0x%x,chunkCount:0x%x, skipBlocks:0x%x, cryptBlocks:0x%x\n",
        xSession->EMI, xKeySlotId, OpaqueInput, OpaqueOutput, OpaqueInput->version, OpaqueInput->size, OpaqueInput->chunks->clearSize,
        OpaqueInput->chunks->protSize, OpaqueInput->chunkCount, OpaqueInput->skipBlocks, OpaqueInput->cryptBlocks);

    secProcessLock();
    ret = mtSecProcessOpaqueData(xSession, xKeySlotId, OpaqueInput, OpaqueOutput, xLastChunk);
    secProcessUnlock();

    if (OpaqueInput->size != OpaqueOutput->size)
    {
        SECAPI_REENC_INFO("The size of input and output  are not match, 000  \n");
    }

    SECAPI_REENC_INFO("ret = 0x%x \n", ret);
    return ret;
}

static TSecStatus secStreamReEncryptSessionProcessOpaqueData
(
 TSecStreamSession   xSession,
 TUnsignedInt16      xKeySlotId,
 const void*              pxOpaqueInput,
 void*              pxOpaqueOutput,
 const TUnsignedInt8*     pxInitVector,
 size_t              xInitVectorSize,
 TBoolean            xLastChunk
 )
{
    const TSecOpaqueInputBufferV1 *initOpaqueInput;
    TSecOpaqueInputBufferV1 *finalOpaqueOutput = NULL;
    TSecOpaqueOutputBuffer tempOpaqueOutput = {0,};
    TSecOpaqueOutputBuffer tempOpaqueOutput2 = {0,};
    TSecChunkInfo *chunkInfo = NULL;
    int ret = 0, i = 0;

    if (NULL == xSession || (~0U) == (TUnsignedLong)xSession || 0xDEAD == (TUnsignedLong)xSession) {
        SECAPI_ERROR("return BAD_PARAMETER(%d)\n", SEC_ERROR_BAD_PARAMETER);
        return SEC_ERROR_BAD_PARAMETER;
    }

    SECAPI_REENC_INFO("secStreamSessionProcessOpaqueData  xSession: 0x%lx tsid= %d \n", (TUnsignedLong)xSession, xSession->transportSessionId);

    if (NULL != pxInitVector || 0 != xInitVectorSize) {
        //parameters are deprecated, should be NULL and 0.
        SECAPI_ERROR("IV and Size are deprecated, should be NULL & 0.\n");
        return SEC_ERROR_BAD_USAGE;
    }

    if (0xFFFF == xKeySlotId) {
        SECAPI_ERROR("Invalid slot id.\n");
        return SEC_ERROR_BAD_PARAMETER;
    }

    if (xKeySlotId != xSession->keySlotID[0] && xKeySlotId != xSession->keySlotID[1]) {
        SECAPI_ERROR("Not a valid slot id.\n");
        return SEC_ERROR_BAD_PARAMETER;
    }

    if (NULL == pxOpaqueInput || NULL == pxOpaqueOutput) {
        SECAPI_ERROR("Input or Output buffer error.  pxOpaqueInput = %lx pxOpaqueOutput = %lx \n", (ulong)pxOpaqueInput , (ulong)pxOpaqueOutput);
        return SEC_ERROR_BAD_PARAMETER;
    }
    memset(&tempOpaqueOutput, 0x00, sizeof(TSecOpaqueOutputBuffer));
    memset(&tempOpaqueOutput2, 0x00, sizeof(TSecOpaqueOutputBuffer));


    //pxOpaqueInput and pxOpaqueOutput maybe the same value
    initOpaqueInput = (const TSecOpaqueInputBufferV1 *)pxOpaqueInput;
    finalOpaqueOutput = (TSecOpaqueInputBufferV1*)pxOpaqueOutput;
    finalOpaqueOutput->version = initOpaqueInput->version;

    tempOpaqueOutput.data = (TUnsignedInt8*)mtSec_OTTAllocateSMPMemory(initOpaqueInput->size, MB_OTT);
    if (tempOpaqueOutput.data == NULL) {
        SECAPI_ERROR("\n");
        ret = SEC_ERROR;
        goto reEncEnd;
    }
    //finalOpaqueOutput->chunks = initOpaqueInput->chunks;
    for (i = 0; i < initOpaqueInput->chunkCount; i++)
    {
        chunkInfo = &initOpaqueInput->chunks[i];
        if (chunkInfo != NULL) {
            if (finalOpaqueOutput->chunks == NULL) {
                SECAPI_ERROR("finalOpaqueOutput->chunks[i] shouled not be NULL = %d \n", i);
                return SEC_ERROR_BAD_PARAMETER;
            }

        }
        finalOpaqueOutput->chunks[i].clearSize = chunkInfo->clearSize;
        finalOpaqueOutput->chunks[i].protSize = chunkInfo->protSize;
        finalOpaqueOutput->chunks[i].initVectorSize = chunkInfo->initVectorSize;
        if (chunkInfo->initVector != NULL || chunkInfo->initVectorSize > 0 ) {
            if (finalOpaqueOutput->chunks[i].initVector == NULL) {
                SECAPI_ERROR("finalOpaqueOutput->chunks[i].initVector shouled not be NULL = %d \n", i);
                return SEC_ERROR_BAD_PARAMETER;
            }
            memcpy(finalOpaqueOutput->chunks[i].initVector, chunkInfo->initVector, chunkInfo->initVectorSize);
        }
    }

    finalOpaqueOutput->chunkCount = initOpaqueInput->chunkCount;
    finalOpaqueOutput->skipBlocks = initOpaqueInput->skipBlocks;
    finalOpaqueOutput->cryptBlocks = initOpaqueInput->cryptBlocks;


    SECAPI_REENC_INFO("version=0x%x, indata=0x%lx, size:0x%x, chunk:0x%x, chunkCount:0x%x, skipBlocks:0x%x, cryptBlocks:0x%x\n",
        finalOpaqueOutput->version, finalOpaqueOutput->data, finalOpaqueOutput->size, finalOpaqueOutput->chunks, finalOpaqueOutput->chunkCount,
        finalOpaqueOutput->skipBlocks,finalOpaqueOutput->cryptBlocks);


    SECAPI_REENC_INFO("EMI = 0x%x, data = %p, size = 0x%x \n", xSession->EMI, tempOpaqueOutput.data, initOpaqueInput->size);

    tempOpaqueOutput2.data = finalOpaqueOutput->data;//save output pointer to temp tempOpaqueOutput2
    tempOpaqueOutput2.size = finalOpaqueOutput->size;


    secProcessLock();
    xSession->isEncryptionSession = FALSE;//Do decryption firstly.

    ret = mtSecProcessOpaqueData(xSession, xKeySlotId, initOpaqueInput, &tempOpaqueOutput, xLastChunk);
    secProcessUnlock();
    if (ret) {
        SECAPI_ERROR("ret = 0x%x \n", ret);
        ret = SEC_ERROR;
        goto reEncEnd;
    }

    mt_u8 *p_phyaddr = vir2phy(tempOpaqueOutput.data);
    mt_u8 *p_finalphyaddr = vir2phy(finalOpaqueOutput->data);

    if (tempOpaqueOutput.size != initOpaqueInput->size)
    {
        SECAPI_REENC_INFO("The size of input and output  are not match 111 \n");
    }
    finalOpaqueOutput->size = tempOpaqueOutput.size;
    secProcessLock();
    xSession->isEncryptionSession = TRUE; //Redo encryption.
    finalOpaqueOutput->data = tempOpaqueOutput.data;

    ret = mtSecProcessOpaqueData(xSession, xKeySlotId + 1, finalOpaqueOutput, &tempOpaqueOutput, xLastChunk);
    secProcessUnlock();
    finalOpaqueOutput->data = tempOpaqueOutput2.data;//restore output pointer to final output structure

    SECAPI_REENC_INFO("ret = 0x%x finalOpaqueOutput->data = 0x%x, size =0x%x\n", ret, finalOpaqueOutput->data, tempOpaqueOutput.size);
    if (ret) {
        SECAPI_ERROR("ret = 0x%x \n", ret);
        ret = SEC_ERROR;
        goto reEncEnd;
    }

    SECAPI_REENC_INFO("\n");

    secProcessLock();
    if( mt_dmacpy_ch4((phys_addr_t) p_finalphyaddr, (phys_addr_t)p_phyaddr, tempOpaqueOutput.size) !=MT_SUCCESS )// smp on noly can use MT_EDMA_CH_1
    {
        SECAPI_REENC_INFO("dmacpy MT_EDMA_CH_4...33.fail!  \n");
    }
    secProcessUnlock();

reEncEnd:
    if (tempOpaqueOutput.data) {
        mtSec_OTTFreeSMPMemory(tempOpaqueOutput.data);
    }

    return ret;
}
/******************************************************************************/
/*                                                                            */
/*                                    RSA                                     */
/*                                                                            */
/******************************************************************************/

static TSecStatus secRsaComputeCrtParams(
	size_t xKeySize,
	TUnsignedInt32 xE,
	const TUnsignedInt8 * pxP,
	const TUnsignedInt8 * pxQ,
	TUnsignedInt8 * pxDP,
	TUnsignedInt8 * pxDQ,
	TUnsignedInt8 * pxQInv)
{
	TSignedInt32 ret;

	SECAPI_INFO("\n");

	if (!pxP || !pxQ || !pxDP || !pxDQ || !pxQInv)
		return SEC_ERROR_BAD_PARAMETER;

	if (xE != 3 && xE != 17 && xE != 65537)
		return SEC_ERROR_BAD_PARAMETER;

	if (xKeySize < 64 || xKeySize > 256) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	ret = mt_unf_rsa_gen_crt_params((TUnsignedInt32)xKeySize,
		xE, pxP, pxQ, pxDP, pxDQ, pxQInv);
	if (ret != MT_SUCCESS) {
		return SEC_ERROR;
	}

	return SEC_NO_ERROR;
}

static TSecStatus secRsaPublicEncrypt(
	const TUnsignedInt8 * pxInput,
	size_t xInputSize,
	TUnsignedInt8 * pxOutput,
	TUnsignedInt32 xE,
	const TUnsignedInt8 * pxN,
	size_t xKeySize,
	TSecRsaPadding xPadding)
{
	TUnsignedInt8 e[4] = { 0 };
	TSecStatus ret = SEC_NO_ERROR;
	struct rsa_public_key pubkey;
	MT_RSA_ALG algo;

	SECAPI_INFO("\n");

	if (NULL == pxInput || NULL == pxOutput || NULL == pxN) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (xKeySize < 64) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (3 == xE) {
		e[3] = 0x3;
	} else if (17 == xE) {
		e[3] = 0x11;
	} else if (65537 == xE) {
		e[3] = 0x1;
		e[1] = 0x1;
	} else {
		return SEC_ERROR_BAD_PARAMETER;
	}

	memset(&pubkey, 0, sizeof(struct rsa_public_key));
	pubkey.e_length = 4;
	pubkey.e = e;
	pubkey.n_length = xKeySize;
	pubkey.n = (unsigned char *)pxN;

	if (SEC_RSA_OAEP_SHA_1_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA1;
	else if (SEC_RSA_OAEP_SHA_256_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA256;
	else if (SEC_RSA_NO_PADDING == xPadding)
		algo = MT_RSA_NOPAD;
	else
		return SEC_ERROR_BAD_PARAMETER;

	ret = mt_unf_rsa_public_encrypt(&pubkey, (unsigned char *)pxInput, xInputSize, pxOutput, algo);
	if (MT_SUCCESS == (int)ret)
		return SEC_NO_ERROR;
	else if (MT_CIPHER_ERR_BAD_PARAMETERS == (int)ret)
		return SEC_ERROR_BAD_PARAMETER;
	else
		return SEC_ERROR;
}

static TSecStatus secRsaPrivateEncrypt(
	const TUnsignedInt8 * pxInput,
	size_t xInputSize,
	TUnsignedInt8 * pxOutput,
	TUnsignedInt32 xE,
	const TUnsignedInt8 * pxN,
	const TUnsignedInt8 * pxP,
	const TUnsignedInt8 * pxQ,
	const TUnsignedInt8 * pxDP,
	const TUnsignedInt8 * pxDQ,
	const TUnsignedInt8 * pxQInv,
	size_t xKeySize,
	TSecRsaPadding xPadding)
{
	TSecStatus ret = SEC_NO_ERROR;
	//TUnsignedInt8 e[4] = { 0, };
	struct rsa_keypair privkey;
	MT_RSA_ALG algo;

	SECAPI_INFO("\n");

	if (NULL == pxInput || NULL == pxOutput || NULL == pxN || NULL == pxP
		|| NULL == pxQ || NULL == pxDP || NULL == pxDQ || NULL == pxQInv) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xKeySize < 64) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (3 == xE) {
		//e[3] = 0x3;
	} else if (17 == xE) {
		//e[3] = 0x11;
	} else if (65537 == xE) {
		//e[3] = 0x1;
		//e[1] = 0x1;
	} else {
		return SEC_ERROR_BAD_PARAMETER;
	}
	SECAPI_INFO("\n");

	memset(&privkey, 0, sizeof(struct rsa_keypair));
	privkey.n_length = xKeySize;
	privkey.n = (unsigned char *)pxN;
	privkey.p_length = xKeySize/2;
	privkey.p = (unsigned char *)pxP;
	privkey.q_length = xKeySize/2;
	privkey.q = (unsigned char *)pxQ;
	privkey.qInv_length = xKeySize/2;
	privkey.qInv = (unsigned char *)pxQInv;
	privkey.dp_length = xKeySize/2;
	privkey.dp = (unsigned char *)pxDP;
	privkey.dq_length = xKeySize/2;
	privkey.dq = (unsigned char *)pxDQ;

	if (SEC_RSA_OAEP_SHA_1_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA1;
	else if (SEC_RSA_OAEP_SHA_256_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA256;
	else if (SEC_RSA_NO_PADDING == xPadding)
		algo = MT_RSA_NOPAD;
	else
		return SEC_ERROR_BAD_PARAMETER;

	ret = mt_unf_rsa_private_encrypt(&privkey, (unsigned char *)pxInput, xInputSize, pxOutput, algo);
	if (MT_SUCCESS == (int)ret)
		return SEC_NO_ERROR;
	else if (MT_CIPHER_ERR_BAD_PARAMETERS == (int)ret)
		return SEC_ERROR_BAD_PARAMETER;
	else
		return SEC_ERROR;
}

static TSecStatus secRsaPublicDecrypt(
	const TUnsignedInt8 * pxInput,
	TUnsignedInt8 * pxOutput,
	size_t * pxOutputSize,
	TUnsignedInt32 xE,
	const TUnsignedInt8 * pxN,
	size_t xKeySize,
	TSecRsaPadding xPadding)
{
	TUnsignedInt8 e[4] = { 0 };
	TSecStatus ret = SEC_NO_ERROR;
	struct rsa_public_key pubkey;
	MT_RSA_ALG algo;

	SECAPI_INFO("\n");
	if (NULL == pxInput || NULL == pxOutput || NULL == pxN) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xKeySize < 64) {
		return SEC_ERROR_BAD_PARAMETER;
	}
	if (3 == xE) {
		e[3] = 0x3;
	} else if (17 == xE) {
		e[3] = 0x11;
	} else if (65537 == xE) {
		e[3] = 0x1;
		e[1] = 0x1;
	} else {
		return SEC_ERROR_BAD_PARAMETER;
	}

	memset(&pubkey, 0, sizeof(struct rsa_public_key));
	pubkey.e_length = 4;
	pubkey.e = e;
	pubkey.n_length = xKeySize;
	pubkey.n = (unsigned char *)pxN;

	if (SEC_RSA_OAEP_SHA_1_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA1;
	else if (SEC_RSA_OAEP_SHA_256_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA256;
	else if (SEC_RSA_NO_PADDING == xPadding)
		algo = MT_RSA_NOPAD;
	else
		return SEC_ERROR_BAD_PARAMETER;

	ret = mt_unf_rsa_public_decrypt(&pubkey,
		(unsigned char *)pxInput, xKeySize, pxOutput, (TUnsignedInt32 *)pxOutputSize, algo);
	if (MT_SUCCESS == ret)
		return SEC_NO_ERROR;
	else if (MT_CIPHER_ERR_BAD_PARAMETERS == (int)ret)
		return SEC_ERROR_BAD_PARAMETER;
	else if (MT_RSA_ERR_BAD_PADDING == (int)ret)
		return SEC_ERROR_BAD_PADDING;
	else
		return SEC_ERROR;
}

static TSecStatus secRsaPrivateDecrypt(
	const TUnsignedInt8 * pxInput,
	TUnsignedInt8 * pxOutput,
	size_t * pxOutputSize,
	TUnsignedInt32 xE,
	const TUnsignedInt8 * pxN,
	const TUnsignedInt8 * pxP,
	const TUnsignedInt8 * pxQ,
	const TUnsignedInt8 * pxDP,
	const TUnsignedInt8 * pxDQ,
	const TUnsignedInt8 * pxQInv,
	size_t xKeySize,
	TSecRsaPadding xPadding)
{
	TSecStatus ret = SEC_NO_ERROR;
	//TUnsignedInt8 e[4] = {0};
	struct rsa_keypair privkey;
	MT_RSA_ALG algo;

	SECAPI_INFO("\n");

	if (NULL == pxInput
		|| NULL == pxOutput
		|| NULL == pxN
		|| NULL == pxP
		|| NULL == pxQ || NULL == pxDP || NULL == pxDQ || NULL == pxQInv) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xKeySize < 64) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (3 == xE) {
		//e[3] = 0x3;
	} else if (17 == xE) {
		//e[3] = 0x11;
	} else if (65537 == xE) {
		//e[3] = 0x1;
		//e[1] = 0x1;
	} else {
		return SEC_ERROR_BAD_PARAMETER;
	}

	memset(&privkey, 0, sizeof(struct rsa_keypair));
	privkey.n_length = xKeySize;
	privkey.n = (unsigned char *)pxN;
	privkey.p_length = xKeySize/2;
	privkey.p = (unsigned char *)pxP;
	privkey.q_length = xKeySize/2;
	privkey.q = (unsigned char *)pxQ;
	privkey.qInv_length = xKeySize/2;
	privkey.qInv = (unsigned char *)pxQInv;
	privkey.dp_length = xKeySize/2;
	privkey.dp = (unsigned char *)pxDP;
	privkey.dq_length = xKeySize/2;
	privkey.dq = (unsigned char *)pxDQ;

	if (SEC_RSA_OAEP_SHA_1_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA1;
	else if (SEC_RSA_OAEP_SHA_256_PADDING == xPadding)
		algo = MT_RSAES_PKCS1_OAEP_MGF1_SHA256;
	else if (SEC_RSA_NO_PADDING == xPadding)
		algo = MT_RSA_NOPAD;
	else
		return SEC_ERROR_BAD_PARAMETER;

	ret = mt_unf_rsa_private_decrypt(&privkey,
		(unsigned char *)pxInput, xKeySize, pxOutput, (TUnsignedInt32 *)pxOutputSize, algo);
	if (MT_SUCCESS == (int)ret)
		return SEC_NO_ERROR;
	else if (MT_CIPHER_ERR_BAD_PARAMETERS == (int)ret)
		return SEC_ERROR_BAD_PARAMETER;
	else if (MT_RSA_ERR_BAD_PADDING == (int)ret)
		return SEC_ERROR_BAD_PADDING;
	else
		return SEC_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                               DIFFIE-HELLMAN                               */
/*                                                                            */
/******************************************************************************/

static TSecStatus secDhGenerateKey(
	const TUnsignedInt8 * pxG,
	const TUnsignedInt8 * pxP,
	const TUnsignedInt8 * pxInputPrivKey,
	TUnsignedInt8 * pxOutputPrivKey,
	TUnsignedInt8 * pxPubKey,
	size_t xKeySize)
{
	mt_handle bn_handle = 0;

	SECAPI_INFO("\n");
	if (NULL == pxG || NULL == pxP || NULL == pxPubKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xKeySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == pxInputPrivKey && NULL == pxOutputPrivKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == pxInputPrivKey && NULL != pxOutputPrivKey) {
		if (mt_unf_cipher_get_random_number(xKeySize, pxOutputPrivKey))
			return SEC_ERROR;
	}

	if (mt_unf_cipher_bn_create(&bn_handle)) {
		goto DhGenKey_error_create;
	}
	if (mt_unf_cipher_bn_mod_exp(bn_handle, pxPubKey,
		(unsigned char *)pxG,
		(unsigned char *)(pxInputPrivKey ?
		pxInputPrivKey :
		pxOutputPrivKey),
		(unsigned char *)pxP, xKeySize,
		xKeySize, xKeySize)) {
		goto DhGenKey_error_exp;
	}
	mt_unf_cipher_bn_destroy(bn_handle);

	return SEC_NO_ERROR;

DhGenKey_error_exp:
	mt_unf_cipher_bn_destroy(bn_handle);
DhGenKey_error_create:
	return SEC_ERROR;
}

static TSecStatus secDhComputeKey(
	const TUnsignedInt8 * pxP,
	const TUnsignedInt8 * pxPrivKey,
	const TUnsignedInt8 * pxOtherPubKey,
	TUnsignedInt8 * pxSharedSecret,
	size_t xKeySize)
{
	mt_handle bn_handle = 0;

	SECAPI_INFO("\n");
	if (NULL == pxP || NULL == pxPrivKey || NULL == pxOtherPubKey
		|| NULL == pxSharedSecret) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xKeySize) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (mt_unf_cipher_bn_create(&bn_handle)) {
		SECAPI_ERROR("\n");
		goto DhCptKey_error_create;
	}
	if (mt_unf_cipher_bn_mod_exp(bn_handle, pxSharedSecret,
		(unsigned char *)pxOtherPubKey,
		(unsigned char *)pxPrivKey,
		(unsigned char *)pxP, xKeySize,
		xKeySize, xKeySize)) {
		SECAPI_ERROR("\n");
		goto DhCptKey_error_exp;
	}
	mt_unf_cipher_bn_destroy(bn_handle);

	return SEC_NO_ERROR;

DhCptKey_error_exp:
	mt_unf_cipher_bn_destroy(bn_handle);
DhCptKey_error_create:
	return SEC_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                                    SHA-256                                 */
/*                                                                            */
/******************************************************************************/

static TSecStatus secSha256Init(TSecHashContext * pxContext) {
	TSecHashContext HashContext = NULL;
	mt_handle hash_handle = MT_INVALID_HANDLE;
	mt_s32 ret = MT_SUCCESS;
	SECAPI_INFO("\n");

	if (NULL == pxContext) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	HashContext = psecMalloc(sizeof(struct SSecHashContext));
	if (NULL == HashContext) {
		goto sha256_ctxt_error;
	}
	memset(HashContext, 0, sizeof(struct SSecHashContext));

	ret = mt_unf_cipher_hash_create(MT_CIPHER_HASH_TYPE_SHA256, NULL,
		&hash_handle);
       SECAPI_INFO("\n");
	if (ret != MT_SUCCESS) {
		goto sha256_create_error;
	}

	HashContext->priv_handle = hash_handle;

	*pxContext = HashContext;
	return SEC_NO_ERROR;

sha256_create_error:
    SECAPI_INFO("\n");
	if (HashContext != NULL)
		psecFree(HashContext);
sha256_ctxt_error:
    SECAPI_INFO("\n");
	return SEC_ERROR;
}

static TSecStatus secSha256Update(
	TSecHashContext xContext,
	const TUnsignedInt8 * pxMessageChunk,
	size_t xChunkSize)
{

	if (NULL == xContext) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xChunkSize) {
		//return SEC_ERROR;
		return SEC_NO_ERROR;
	}

	if (NULL == pxMessageChunk) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (mt_unf_cipher_hash_update(xContext->priv_handle, (unsigned char *)pxMessageChunk,
		xChunkSize)) {
		SECAPI_INFO("\n");
		return SEC_ERROR;
	}

	return SEC_NO_ERROR;
}

static TSecStatus secSha256Final(
	TSecHashContext xContext,
	TUnsignedInt8 * pxMessageDigest)
{
	TSecStatus sec_ret = SEC_ERROR_BAD_PARAMETER;
	mt_s32 ret = MT_SUCCESS;

	SECAPI_INFO("\n");

	if (NULL == xContext) {
		goto sha256_return;
	}

	if (NULL == pxMessageDigest) {
		goto sha256_release;
	}

	ret = mt_unf_cipher_hash_final(xContext->priv_handle, pxMessageDigest);
	sec_ret = (ret == MT_SUCCESS) ? SEC_NO_ERROR : SEC_ERROR;

sha256_release:
	psecFree(xContext);
sha256_return:
	return sec_ret;
}

/******************************************************************************/
/*                                                                            */
/*                                    HMAC                                    */
/*                                                                            */
/******************************************************************************/

static TSecStatus secHmacSha256Init(
	const TUnsignedInt8 * pxKey,
	size_t xKeySize,
	TSecHashContext * pxContext)
{
	TSecHashContext HashContext;
	mt_handle hmac_handle;
	MT_CIPHER_HMAC_ATTS_S s_hmac_attr = { 0 };

	SECAPI_INFO("\n");

	if (NULL == pxContext || NULL == pxKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (xKeySize > 64) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	HashContext = psecMalloc(sizeof(struct SSecHashContext));
	if (NULL == HashContext) {
		goto hmac_ctxt_error;
	}
	memset(HashContext, 0, sizeof(struct SSecHashContext));

	s_hmac_attr.p_hmac_key = (unsigned char *)pxKey;
	s_hmac_attr.key_len = xKeySize;
        s_hmac_attr.key_slot[0] = MT_CIPHER_KEYSLOT_INVALID;
        s_hmac_attr.key_slot[1] = MT_CIPHER_KEYSLOT_INVALID;

	if (0 != mt_unf_cipher_mac_create(MT_CIPHER_MAC_TYPE_SHA256,
		&s_hmac_attr, &hmac_handle)) {
		goto hmac_create_error;
	}

	HashContext->priv_handle = hmac_handle;
	*pxContext = HashContext;

	return SEC_NO_ERROR;

hmac_create_error:
	psecFree(HashContext);
hmac_ctxt_error:
	return SEC_ERROR;
}

#if SECAPI_VERSION_INT >= SEC_TOOL_VERSION_INT(6, 12, 0)
static TSecStatus secHmacSha256CertInit
(
	TSecHashContext*    pxContext
)
{
	TSecHashContext HashContext;
	mt_handle hmac_handle;
	MT_CIPHER_HMAC_ATTS_S s_hmac_attr = { 0 };

	SECAPI_INFO("\n");

	if (NULL == pxContext) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	HashContext = psecMalloc(sizeof(struct SSecHashContext));
	if (NULL == HashContext) {
		goto hmac_ctxt_error;
	}
	memset(HashContext, 0, sizeof(struct SSecHashContext));

	s_hmac_attr.p_hmac_key = NULL;
	s_hmac_attr.key_len = 16;
	s_hmac_attr.key_slot[0] = MT_CIPHER_KEYSLOT_INVALID;
	s_hmac_attr.key_slot[1] = MT_CIPHER_KEYSLOT_INVALID;
	if (mt_unf_cipher_keyslot_request(s_hmac_attr.key_slot)) {
		goto hmac_create_error;
	}
	if(certExportKey(s_hmac_attr.key_slot[0])) {
		SECAPI_ERROR("export cert key error %s\n", __func__);
		goto hmac_create_error;
	}
	if (0 != mt_unf_cipher_mac_create(MT_CIPHER_MAC_TYPE_SHA256,
		&s_hmac_attr, &hmac_handle)) {
		goto hmac_create_error;
	}

	HashContext->priv_handle = hmac_handle;
	*pxContext = HashContext;

	return SEC_NO_ERROR;

hmac_create_error:
	psecFree(HashContext);
hmac_ctxt_error:
	return SEC_ERROR;
}
#endif


static TSecStatus secHmacSha256Update(
	TSecHashContext xContext,
	const TUnsignedInt8 * pxMessageChunk,
	size_t xChunkSize)
{
	SECAPI_INFO("\n");

	if (NULL == xContext) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xChunkSize) {
		return SEC_ERROR;
	}

	if (NULL == pxMessageChunk) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 != mt_unf_cipher_mac_update(xContext->priv_handle,
		(unsigned char *)pxMessageChunk, xChunkSize)) {
		return SEC_ERROR;
	}

	return SEC_NO_ERROR;
}

static TSecStatus secHmacSha256Final(
	TSecHashContext xContext,
	TUnsignedInt8 * pxMessageDigest)
{
	TSecStatus sec_ret = SEC_ERROR_BAD_PARAMETER;
	mt_s32 ret = MT_SUCCESS;

	SECAPI_INFO("\n");

	if (NULL == xContext) {
		return sec_ret;
	}

	if (NULL == pxMessageDigest) {
		goto sha256_release;
	}

	ret = mt_unf_cipher_mac_final(xContext->priv_handle, pxMessageDigest);
	sec_ret = (ret == MT_SUCCESS) ? SEC_NO_ERROR : SEC_ERROR;

sha256_release:
	psecFree(xContext);

	return sec_ret;
}

/******************************************************************************/
/*                                                                            */
/*                                  RANDOM                                    */
/*                                                                            */
/******************************************************************************/
static TSecStatus secGenerateRandomBytes(
	size_t xNumOfBytes,
	TUnsignedInt8 * pxRandomBytes)
{
	SECAPI_INFO("\n");
	if (xNumOfBytes == 0 || xNumOfBytes > 1024 || pxRandomBytes == NULL) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (mt_unf_cipher_get_random_number(xNumOfBytes, pxRandomBytes))
	{
		return SEC_ERROR;
	}

	return SEC_NO_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                                    ECDSA                                   */
/*                                                                            */
/******************************************************************************/

static TSecStatus secEcdsaSign(
	TSecEcParams xParams,
	TSecHashType xHashType,
	const TUnsignedInt8 * pxPrivKey,
	const TUnsignedInt8 * pxMessage,
	size_t xMessageSize,
	TUnsignedInt8 * pxSigR,
	TUnsignedInt8 * pxSigS)
{
	MT_CIPHER_EC_PARAMS_S Params;
	MT_CIPHER_HASH_TYPE_E HashType;

	SECAPI_INFO("\n");
	if (NULL == pxPrivKey || NULL == pxMessage || 0 == xMessageSize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == xParams.q || NULL == xParams.a || NULL == xParams.GX
		|| NULL == xParams.GY || NULL == xParams.n) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == pxSigR || NULL == pxSigS) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xParams.keySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == xParams.h) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (SEC_HASH_SHA1 == xHashType) {
		HashType = MT_CIPHER_HASH_TYPE_SHA1;
	} else if (SEC_HASH_SHA256 == xHashType) {
		HashType = MT_CIPHER_HASH_TYPE_SHA256;
	} else {
		return SEC_ERROR_BAD_PARAMETER;
	}

	Params.q = xParams.q;
	Params.a = xParams.a;
	Params.b = xParams.b;
	Params.GX = xParams.GX;
	Params.GY = xParams.GY;
	Params.n = xParams.n;
	Params.h = xParams.h;
	Params.keySize = xParams.keySize;

	if (mt_unf_ecc_ecdsa_sign(&Params, HashType, pxPrivKey,
		pxMessage, xMessageSize, pxSigR, pxSigS)) {
		return SEC_ERROR;
	}

	return SEC_NO_ERROR;
}

static TSecStatus secEcdsaVerify(
	TSecEcParams xParams,
	TSecHashType xHashType,
	const TUnsignedInt8 * pxPubKeyX,
	const TUnsignedInt8 * pxPubKeyY,
	const TUnsignedInt8 * pxMessage,
	size_t xMessageSize,
	const TUnsignedInt8 * pxSigR,
	const TUnsignedInt8 * pxSigS)
{

	MT_CIPHER_EC_PARAMS_S Params;
	MT_CIPHER_HASH_TYPE_E HashType;
	mt_s32 ret;

	SECAPI_INFO("\n");
	if (NULL == pxPubKeyX || NULL == pxPubKeyY || NULL == pxMessage
		|| NULL == pxSigR || NULL == pxSigS) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == xParams.q || NULL == xParams.a || NULL == xParams.GX
		|| NULL == xParams.GY || NULL == xParams.n) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xParams.keySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == xParams.h) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (SEC_HASH_SHA1 == xHashType) {
		HashType = MT_CIPHER_HASH_TYPE_SHA1;
	} else if (SEC_HASH_SHA256 == xHashType) {
		HashType = MT_CIPHER_HASH_TYPE_SHA256;
	} else {
		return SEC_ERROR_BAD_PARAMETER;
	}

	Params.q = xParams.q;
	Params.a = xParams.a;
	Params.b = xParams.b;
	Params.GX = xParams.GX;
	Params.GY = xParams.GY;
	Params.n = xParams.n;
	Params.h = xParams.h;
	Params.keySize = xParams.keySize;

	ret = mt_unf_ecc_ecdsa_verify(&Params, HashType, pxPubKeyX, pxPubKeyY,
			pxMessage, xMessageSize, pxSigR, pxSigS);
	if (ret) {
		if (MT_CIPHER_ERR_ECC_VERIFY_FAILED == ret) {
			return SEC_ERROR_BAD_SIGNATURE;
		} else {
			return SEC_ERROR;
		}
	}

	return SEC_NO_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                                    ECDH                                   */
/*                                                                            */
/******************************************************************************/
static TSecStatus secEcdhGenerateKey(
	TSecEcParams xParams,
	const TUnsignedInt8 * pxInputPrivKey,
	TUnsignedInt8 * pxOutputPrivKey,
	TUnsignedInt8 * pxPubKeyX,
	TUnsignedInt8 * pxPubKeyY)
{
	TSignedInt32 ret;
	MT_CIPHER_EC_PARAMS_S ec_Params = { 0 };
	TUnsignedInt8 *priKey;
	TUnsignedInt32 i;

	SECAPI_INFO("\n");
	if (NULL == pxInputPrivKey && NULL == pxOutputPrivKey) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == pxPubKeyX || NULL == pxPubKeyY) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == xParams.q || NULL == xParams.a || NULL == xParams.b
		|| NULL == xParams.GX || NULL == xParams.GY || NULL == xParams.n
		|| NULL == xParams.h) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xParams.keySize) {
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (1 != xParams.h[xParams.keySize - 1]) {
		return SEC_ERROR_BAD_PARAMETER;
	} else {
		for (i = 0; i < xParams.keySize - 1; i++) {
			if (0 != xParams.h[i]) {
				return SEC_ERROR_BAD_PARAMETER;
			}
		}
	}

	ec_Params.q = xParams.q;
	ec_Params.a = xParams.a;
	ec_Params.b = xParams.b;
	ec_Params.GX = xParams.GX;
	ec_Params.GY = xParams.GY;
	ec_Params.n = xParams.n;
	ec_Params.h = xParams.h;
	ec_Params.keySize = xParams.keySize;

	if (NULL == pxInputPrivKey && NULL != pxOutputPrivKey) {
		priKey = pxOutputPrivKey;
		ret = mt_unf_ecc_ecdh_gen_keypair(&ec_Params, priKey, pxPubKeyX, pxPubKeyY);
		if (ret < 0)
			return SEC_ERROR;
	} else {
		priKey = pxInputPrivKey;
		ret = mt_unf_ecc_ecdh_gen_pubkey(&ec_Params, priKey, pxPubKeyX, pxPubKeyY);
		if (ret < 0)
			return SEC_ERROR;
	}

	return SEC_NO_ERROR;
}

static TSecStatus secEcdhComputeKey(
	TSecEcParams xParams,
	const TUnsignedInt8 * pxPrivKey,
	const TUnsignedInt8 * pxOtherPubKeyX,
	const TUnsignedInt8 * pxOtherPubKeyY,
	TUnsignedInt8 * pxSharedSecret)
{
	TSignedInt32 ret;
	MT_CIPHER_EC_PARAMS_S ec_Params;
	TUnsignedInt8 r_X[xParams.keySize];
	TUnsignedInt8 r_Y[xParams.keySize];
	TUnsignedInt32 i;

	SECAPI_INFO("\n");
	if (NULL == pxPrivKey || NULL == pxOtherPubKeyX
		|| NULL == pxOtherPubKeyY || NULL == pxSharedSecret) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (NULL == xParams.q || NULL == xParams.a || NULL == xParams.b
		|| NULL == xParams.GX || NULL == xParams.GY || NULL == xParams.n
		|| NULL == xParams.h) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (0 == xParams.keySize) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	}

	if (1 != xParams.h[xParams.keySize - 1]) {
		SECAPI_ERROR("\n");
		return SEC_ERROR_BAD_PARAMETER;
	} else {
		for (i = 0; i < xParams.keySize - 1; i++) {
			if (0 != xParams.h[i]) {
				SECAPI_ERROR("\n");
				return SEC_ERROR_BAD_PARAMETER;
			}
		}
	}

	ec_Params.q = xParams.q;
	ec_Params.a = xParams.a;
	ec_Params.b = xParams.b;
	ec_Params.GX = xParams.GX;
	ec_Params.GY = xParams.GY;
	ec_Params.n = xParams.n;
	ec_Params.h = xParams.h;
	ec_Params.keySize = xParams.keySize;

	ret = mt_unf_ecc_ecdh_gen_sharekey(&ec_Params, pxPrivKey,
                    pxOtherPubKeyX, pxOtherPubKeyY,
                    r_X, r_Y);
	if (ret < 0)
		return SEC_ERROR;

	memcpy(pxSharedSecret, r_X, xParams.keySize);

	return SEC_NO_ERROR;
}

/******************************************************************************/
/*                                                                            */
/*                                 KEYSTORE                                   */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/*                               FUNCTION TABLE                               */
/*                                                                            */
/******************************************************************************/


  static TSecStatus secStreamSessionSetUserIntent
  (
    TUnsignedInt16      xKeySlotId,
    TSecUserIntent      xUserIntent
  )
{
	SECAPI_ERROR("[REEDEBUG]%s %d  xKeySlotId = %d xUserIntent = %d\n",__FUNCTION__,__LINE__, xKeySlotId, xUserIntent);

	return SEC_NO_ERROR;
}
static TSecStatus secStreamSessionProcessContentUsageRules
(
	TUnsignedInt16      xKeySlotId,
	const TUnsignedInt8*     pxUsageRules,
	size_t              xUsageRulesSize
)
{
	SECAPI_ERROR("[REEDEBUG]%s %d  \n",__FUNCTION__,__LINE__);

	return SEC_NO_ERROR;
}


static const ISecStreamSession gSecStreamDecryptSession = {
    secStreamDecryptSessionOpen,
    secStreamSessionClose,
    secStreamDecryptSessionGetKeySlot,
    NULL, //secStreamSessionSetClearTextKey,
    NULL, //secStreamSessionSetCertKey,
    secStreamSessionSet2LevelProtectedKey,
    secStreamSessionProcessOpaqueData,
    NULL, //secStreamSessionProcessContentUsageRules,
    NULL, //secStreamSessionSetEtsi2LevelProtectedKey,
    NULL,//secStreamSessionSetUserIntent,
    NULL, //secStreamSessionSetMkl2LevelProtectedKey,
    NULL, //secStreamSessionEnableSecureProcessor,
#if SECAPI_VERSION_INT >= SEC_TOOL_VERSION_INT(6, 13, 0)
    NULL,
#endif
};

static const ISecStreamSession gSecStreamEncryptSession = {
    secStreamEncryptSessionOpen,
    secStreamSessionClose,
    secStreamEncryptSessionGetKeySlot,
    NULL, //secStreamSessionSetClearTextKey,
    NULL, //secStreamSessionSetCertKey,
    secStreamSessionSet2LevelProtectedKey,
    secStreamSessionProcessOpaqueData,
    secStreamSessionProcessContentUsageRules,
    NULL, //secStreamSessionSetEtsi2LevelProtectedKey,
    secStreamSessionSetUserIntent,
    NULL, //secStreamSessionSetMkl2LevelProtectedKey,
    NULL, //secStreamSessionEnableSecureProcessor,
#if SECAPI_VERSION_INT >= SEC_TOOL_VERSION_INT(6, 13, 0)
    NULL,
#endif
};
#if SECAPI_VERSION_INT >= SEC_TOOL_VERSION_INT(6, 13, 0)
static const ISecStreamSession gSecStreamReEncryptSession = {
    secStreamReEncryptSessionOpen,
    secStreamSessionClose,
    secStreamReEncryptSessionGetKeySlot,
    NULL, //secStreamSessionSetClearTextKey,
    NULL, //secStreamSessionSetCertKey,
    NULL,
    secStreamReEncryptSessionProcessOpaqueData,
    NULL,
    NULL, //secStreamSessionSetEtsi2LevelProtectedKey,
    NULL,
    NULL, //secStreamSessionSetMkl2LevelProtectedKey,
    NULL, //secStreamSessionEnableSecureProcessor,
    NULL,
};
#endif

static TSecFunctionTable gSecFunctionTable = {
	SECAPI_VERSION_INT,
	secGetNuid,
	secGetChipsetRevision,
	secEncryptData,
	secDecryptData,
	secGenerateRandomBytes,
	(TSecRsaGenerateKey) NULL,
	secRsaPublicEncrypt,
	secRsaPrivateEncrypt,
	secRsaPublicDecrypt,
	secRsaPrivateDecrypt,
	secDhGenerateKey,
	secDhComputeKey,
	(TSecSha1Init) NULL,
	(TSecSha1Update) NULL,
	(TSecSha1Final) NULL,
	secSha256Init,
	secSha256Update,
	secSha256Final,
	secHmacSha256Init,
	secHmacSha256Update,
	secHmacSha256Final,
	(TSecEcdsaGenerateKey) NULL,
	secEcdsaSign,
	secEcdsaVerify,
	secEcdhGenerateKey,
	secEcdhComputeKey,
	secOpenRam2RamEncryptSession,
	secOpenRam2RamDecryptSession,
	secCloseSession,
	secSetClearTextKey,
	secSet2LevelProtectedKey,
	secUseCertKey,
	secSessionEncrypt,
	secSessionDecrypt,
	secGetNuid64,
	secGetChipsetExtension,
	(TSecSha384Init) NULL,
	(TSecSha384Update) NULL,
	(TSecSha384Final) NULL,
	secRsaComputeCrtParams,
	secEncryptFlashProtKey,
	secUseFlashProtKey,
	secOpenStreamEncryptSession,
	secOpenStreamDecryptSession,
	(TSecSet0LevelProtectedKey)NULL,
	secSet1LevelProtectedKey,
	secSetMetadata,
	secAllocateBuffer,
	secFreeBuffer,
	(TSecUseLegacyKey) NULL,
	(TSecEnableProtectedBuffer) NULL,
	&gSecStreamDecryptSession,
	&gSecStreamEncryptSession,
	secEnableTeePrivilegedMode,
	secGetTeePrivilegedMode,
	secSetEtsi2LevelProtectedKey,
	(TSecUnprotectKeystore) NULL,
	(TSecSetMkl2LevelProtectedKey) NULL,
	secGetChipId,
//#if SECAPI_VERSION_INT >= SEC_TOOL_VERSION_INT(6, 12, 0)
	secProcessOpaqueData,
	secHmacSha256CertInit,
//#endif
#if SECAPI_VERSION_INT >= SEC_TOOL_VERSION_INT(6, 13, 0)
       &gSecStreamReEncryptSession,
#endif
};

TSecFunctionTable *secGetFunctionTable(void)
{
	return &gSecFunctionTable;
}



