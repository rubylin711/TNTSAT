/*
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * This program is confidential and proprietary to Montage Technology Group Limited
 * and its affiliated companies(Montage), and may not be copied, reproduced, modified,
 * disclosed to others, published or used, in whole or in part, without the express
 * prior written permission of Montage.
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_dma.h"
#include "mt_spr_ext.h"
#include "mt_record.h"

#define DEMUX_ID 0

#define INVALID_PID 0x1FFF

typedef struct
{
	TTransportSessionId tsid;
	mt_u16 emi;
    mt_handle recHandle;
	FILE *recFD;
	/* Encrypted data physical address */
	mt_u8 *recEncBufAddrVir;
	mt_u8 *recEncBufAddrPhy;
	mt_u32 recEncBufSize;
} TsFileInfo;

static MT_BOOL gThreadRunFlag = MT_FALSE;

/* 1.5M */
#define REC_ENC_BUF_LEN		(0x180000)

static mt_u8 *mmz_malloc(mt_u32 size, mt_u32 alignByte)
{
    mt_u32 vir_addr;
    mt_u32 phy_addr;

    if (size == 0)
        return NULL;

    phy_addr = (mt_u32)mt_mmz_new(size, alignByte, "ddr", "rec_enc_buf");
    if (phy_addr == 0) {
        return NULL;
    }

    //map,but not cached
    vir_addr = (mt_u32)mt_mmz_map(phy_addr, 0);
    if (vir_addr == 0) {
        mt_mmz_delete(phy_addr);
        return NULL;
    }

    /* success,return the virtual address of the buffer */
    return (mt_u8 *)vir_addr;
}

static mt_u8 *vir2phy(const mt_u8 *vir)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 phy_addr;
    mt_u32 phy_size;

    ret = mt_mmz_get_phyaddr((const mt_void *)vir, &phy_addr, &phy_size);
    if (ret == MT_SUCCESS)
        return (mt_u8 *)phy_addr;
	else {
		printf("Get phyaddr by viraddr(%x) failed\n", (mt_u32)vir);
        return NULL;
	}
}

static mt_s32 mmz_free(mt_void *p_vir)
{
    mt_s32 ret;
    mt_u32 phy_addr;
    mt_u32 phy_size;

    if (p_vir == NULL) {
        printf("Can not free NULL pointer\n");
        return -1;
    }

    ret = mt_mmz_get_phyaddr(p_vir, &phy_addr, &phy_size);
    ret |= mt_mmz_unmap(phy_addr);
    ret |= mt_mmz_delete(phy_addr);

    return ret;
}

static mt_s32 mt_data_encrypt(mt_u16 emi, TTransportSessionId tsid, mt_u32 size, mt_u8 *inPhy, mt_u8 *outPhy)
{
	if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_IDSA
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_ECB_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CBC_TAIL_CLEAR
		|| emi == NOCS_EMI_MPEG_TS_DVB_AES128_CISSA_V1) {
		if (0 != mtSecRawStreamDataProcess(tsid, SESSION_OP_ENCRYPT, size, (const TUnsignedInt8 *)inPhy, (TUnsignedInt8 *)outPhy)) {
			printf("Raw stream data encryption process failed(Key may not set YET!!!tsid:%d)\n", tsid);
			return MT_FAILURE;
		} else {
			//hex_dump("188 src dump", srcVirAddr, 188);
			//hex_dump("188 dest dump after enc", destVirAddr, 188);
		}
	} else {
		printf("TODO:EMI:%x [To be implemented!]\n", emi);
	}

	return MT_SUCCESS;
}

static mt_s32 dmacpy(MT_EDMA_CH_E ch, mt_u8 *dst, const mt_u8 *src, mt_u32 size)
{
	mt_s32 ret = MT_FAILURE;

	while (MT_EDMA_STATUS_FREE != mt_unf_dma_check(ch)) {
		//10ms
		usleep(1000 * 10);
	}

	ret = mt_unf_dma_memcpy(ch, src, dst, size);
	if (ret != MT_SUCCESS) {
		printf("dma-%d copy from %p to %p with size(%x) failed", ch, src, dst, size);
		return MT_FAILURE;
	}

	return MT_SUCCESS;
}

static mt_void *SaveRecDataThread(mt_void *arg)
{
#if 1
    printf("[%s] gThreadRunFlag %d\n", __FUNCTION__, gThreadRunFlag);
    while (gThreadRunFlag) {
		usleep(1000 * 500);
		printf("Do nothing but loop for test\n");
		if (gThreadRunFlag == MT_FALSE)
			break;
	}
#else
    mt_s32 ret;
    TsFileInfo *pRecInfo = (TsFileInfo *)arg;
    FILE *rfd = pRecInfo->recFD;
	mt_handle recHandle = pRecInfo->recHandle;
	TTransportSessionId tsid = pRecInfo->tsid;
	mt_u16 emi = pRecInfo->emi;
	mt_u32 encBufSize = pRecInfo->recEncBufSize;
	mt_u8 *outPhy = pRecInfo->recEncBufAddrPhy;
	mt_u8 *outVir = pRecInfo->recEncBufAddrVir;
	mt_u32 size;
	mt_u32 remainSize = 0; //remain data size in outPhy that not processed
	mt_u32 sizeAlign16 = 0;

    printf("[%s] gThreadRunFlag %d\n", __FUNCTION__, gThreadRunFlag);
    while (gThreadRunFlag) {
        MT_UNF_DMX_REC_DATA_S recData = {0};
		printf("Acquiring the rec data\n");
        ret = MT_UNF_DMX_AcquireRecData(recHandle, &recData, 100);
        if (MT_SUCCESS != ret) {
            if (MT_ERR_DMX_TIMEOUT == ret) {
                continue;
            }
            if (MT_ERR_DMX_NOAVAILABLE_DATA == ret) {
                continue;
            }
            printf("[%s] MT_UNF_DMX_AcquireRecData failed 0x%x\n", __FUNCTION__, ret);
            break;
        }

		/* Encrypt the data and write to file */
		if (recData.u32Len > encBufSize) {
			printf("Acquired size(%d) > EncBuf size(%d)\n", recData.u32Len, encBufSize);
			size = encBufSize;
		} else {
			size = recData.u32Len;
		}
#if 0 //Clear stream test
		memcpy(outVir, recData.pDataAddr, size);
		ret = MT_SUCCESS;
#else
	#if 0
		//1.move data to output buffer directly
		dmacpy(MT_EDMA_CH_0, (mt_u8 *)(outPhy + remainSize), (const mt_u8 *)recData.u32DataPhyAddr, size);
		//2.Regulate size to 16-aligned, and always decrypt in-place
		if (emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			|| emi == NOCS_EMI_AES128_CBC_ZEROIV_TAIL_CLEAR
			|| emi == NOCS_EMI_AES128_CBC_PKCS7_PADDING) {
			sizeAlign16 = size / 16 * 16;
			remainSize = size % 16;
		} else {
			//ts-mode
			sizeAlign16 = size;
			remainSize = 0;
		}
		ret = mt_data_encrypt(emi, tsid, sizeAlign16, outPhy, outPhy);
	#else
		sizeAlign16 = size;
		remainSize = 0;
		printf("encrypt the data\n");
		ret = mt_data_encrypt(emi, tsid, sizeAlign16, (mt_u8 *)recData.u32DataPhyAddr, outPhy);
	#endif
#endif
		if (MT_SUCCESS == ret) {
			ret = (mt_s32)fwrite(outVir, 1, sizeAlign16, rfd);
			if (ret != sizeAlign16) {
				printf("[SaveRecDataThread] fwrite error, ret:%x\n", ret);
				break;
			}
			printf("<<<<<<<<write %d(remain:%d) bytes success>>>>>>>>\n", sizeAlign16, remainSize);
			//3.Move remainData to header of outPhy
			if (remainSize > 0) {
				dmacpy(MT_EDMA_CH_0, (mt_u8 *)(outPhy), (const mt_u8 *)(outPhy + sizeAlign16), remainSize);
			}

			recData.u32Len = size;
		} else {
			/* Maybe key is not ready yet, so we do nothing but release RecData if encryption failed */
			printf("<<<<<<<<NOTE:Key may not set yet!!!>>>>>>>>\n");
			printf("consume all data, size:%x\n", size);
			recData.u32Len = size;
		}

		printf("releasing the rec data\n");
        ret = MT_UNF_DMX_ReleaseRecData(recHandle, &recData);
        if (MT_SUCCESS != ret) {
            printf("[%s] MT_UNF_DMX_ReleaseRecData failed 0x%x\n", __FUNCTION__, ret);
            break;
        }
    }

	if (remainSize > 0) {
		printf("write the last block of data,size:%d\n", remainSize);
		ret = (mt_s32)fwrite(outVir, 1, remainSize, rfd);
		if (ret != remainSize) {
			printf("[SaveRecDataThread] fwrite error, ret:%x\n", ret);
		}
	}

#endif

    return MT_NULL;
}

mt_s32 mt_record_start(struct transportSession *ts, mt_char *filename, mt_u16 emi, mt_u32 vidPid, mt_u32 audPid)
{
    mt_s32 ret;
    MT_UNF_DMX_REC_ATTR_S RecAttr = {0};
    mt_handle recHandle = MT_INVALID_HANDLE;
    mt_handle vidChanHandle = MT_INVALID_HANDLE;
    mt_handle audChanHandle = MT_INVALID_HANDLE;
    pthread_t recTID;
    TsFileInfo tsRecInfo;
	mt_u8 isRecStarted = 0;

    memset(&RecAttr, 0 , sizeof(MT_UNF_DMX_REC_ATTR_S));
    RecAttr.u32DmxId = ts->recordDmxId;
    RecAttr.u32RecBufSize = 4 * 1024 * 1024; //TODO: Adjust this if needed
	RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_SELECT_PID;
	RecAttr.bDescramed = MT_TRUE;
	//RecAttr.type_mode = DMX_PARTIAL_TS_PACKET;
	RecAttr.type_mode = DMX_FULL_TS_WITHOUT_NULL_PACKET;
    ret = MT_UNF_DMX_CreateRecChn(&RecAttr, &recHandle);
	if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_CreateRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	printf("recHandle:%x, DmxId:%d\n", recHandle, RecAttr.u32DmxId);

	ret = MT_UNF_DMX_AddRecPid(recHandle, vidPid, &vidChanHandle);
	if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		goto rec_start_failed;
	}
	ts->vidChanHandle = vidChanHandle;

	ret = MT_UNF_DMX_AddRecPid(recHandle, audPid, &audChanHandle);
	if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_AddRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		goto rec_start_failed;
	}
	ts->audChanHandle = audChanHandle;

	ret = MT_UNF_DMX_StartRecChn(recHandle);
	if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_StartRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
		goto rec_start_failed;
	}
	isRecStarted = 1;

	ts->recHandle = recHandle;

    ts->recFd = fopen(filename, "wb");
    if (!ts->recFd) {
        printf("fopen error\n");
        goto rec_start_failed;
    }

	/* Pre-allocate encrypted data buffer */
	ts->recBufAddrVir = mmz_malloc(REC_ENC_BUF_LEN, 0x40000);
	if (NULL == ts->recBufAddrVir)
		goto rec_start_failed;

	ts->recBufAddrPhy = vir2phy(ts->recBufAddrVir);
	ts->recBufSize = REC_ENC_BUF_LEN;


	/* Mark recording thread as started */
    gThreadRunFlag = MT_TRUE;

    tsRecInfo.recHandle = recHandle;
	tsRecInfo.recFD = ts->recFd;
	tsRecInfo.recEncBufAddrVir = ts->recBufAddrVir;
	tsRecInfo.recEncBufAddrPhy = ts->recBufAddrPhy;
	tsRecInfo.recEncBufSize = ts->recBufSize;
	tsRecInfo.tsid = ts->tsid;
	tsRecInfo.emi = emi;
    ret = pthread_create(&recTID, MT_NULL, SaveRecDataThread, (mt_void *)&tsRecInfo);
    if (0 != ret) {
		printf("pthread_create error\n");
		goto rec_start_failed;
	}
	ts->recTID = recTID;

	return MT_SUCCESS;

rec_start_failed:
	if (isRecStarted)
		MT_UNF_DMX_StopRecChn(ts->recHandle);

	if (MT_INVALID_HANDLE != vidChanHandle)
		MT_UNF_DMX_DelRecPid(recHandle, vidChanHandle);

	if (MT_INVALID_HANDLE != audChanHandle)
		MT_UNF_DMX_DelRecPid(recHandle, audChanHandle);

	if (MT_INVALID_HANDLE != recHandle)
		MT_UNF_DMX_DestroyRecChn(ts->recHandle);

	if (ts->recFd)
		fclose(ts->recFd);

	if (ts->recBufAddrVir)
		mmz_free(ts->recBufAddrVir);

    return ret;
}

mt_s32 mt_record_stop(struct transportSession *ts)
{
	mt_s32 ret = MT_SUCCESS;

	/* Waiting */
	gThreadRunFlag = MT_FALSE;
	pthread_join(ts->recTID, MT_NULL);

	printf("rec thread joined\n");

	if (ts->recFd) {
		fclose(ts->recFd);
		ts->recFd = NULL;
	}
	printf("rec file closed\n");

	if (ts->recBufAddrVir) {
		mmz_free(ts->recBufAddrVir);
	}
	printf("rec buf freed\n");

	printf("recHandle:%x\n", ts->recHandle);
	ret = MT_UNF_DMX_StopRecChn(ts->recHandle);
	if (MT_SUCCESS != ret) {
	    printf("[%s - %u] MT_UNF_DMX_StopRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
	}
	printf("rec channel stopped\n");

	ret = MT_UNF_DMX_DelRecPid(ts->recHandle, ts->vidChanHandle);
	if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_DelRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
	}
	printf("vid channel deleted\n");

	ret = MT_UNF_DMX_DelRecPid(ts->recHandle, ts->audChanHandle);
	if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_DelRecPid failed 0x%x\n", __FUNCTION__, __LINE__, ret);
	}
	printf("audio channel deleted\n");

    ret = MT_UNF_DMX_DestroyRecChn(ts->recHandle);
    if (MT_SUCCESS != ret) {
		printf("[%s - %u] MT_UNF_DMX_DestroyRecChn failed 0x%x\n", __FUNCTION__, __LINE__, ret);
    }
	printf("rec channel destroyed, ret:%x\n", ret);
	return ret;
}
