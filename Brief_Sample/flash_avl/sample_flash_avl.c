/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mtd/mtd-abi.h>
#include "mt_type.h"
#include "mt_unf_flash.h"
#include "mt_unf_wdg.h"
#include "mt_unf_common.h"
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_FLASHAVL_DEBUG

#define MT_FLASHAVL_PRINT   printf
#else

#define MT_FLASHAVL_PRINT

#endif

#define SAMPLE_FLASHAVL_FUNCTION_ENTER()    MT_FLASHAVL_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_FLASHAVL_FUNCTION_EXIT()     MT_FLASHAVL_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_FLASHAVL_FATAL_PRINT(fmt...)         MT_FLASHAVL_PRINT(" [FATAL] " fmt)
#define SAMPLE_FLASHAVL_ERR_PRINT(fmt...)           MT_FLASHAVL_PRINT(" [ERROR] " fmt)
#define SAMPLE_FLASHAVL_WARN_PRINT(fmt...)          MT_FLASHAVL_PRINT(" [WARN] "  fmt)
#define SAMPLE_FLASHAVL_INFO_PRINT(fmt...)          MT_FLASHAVL_PRINT(" [INFO] "  fmt)
#define SAMPLE_FLASHAVL_DBG_PRINT(fmt...)           MT_FLASHAVL_PRINT(" [DEBUG] " fmt)

#define SAMPLE_FLASHAVL_PRINT  printf
#define FIRST_ERROR             1


#define SAMPLE_FLASHAVL_OPERATE_SIZE    1024 * 1024 * 20
/*************************** Structure Definition ****************************/
typedef enum
{
    MT_FLASHAVL_OPERATE_IMG   = 0x1,
    MT_FLASHAVL_OPERATE_DATA  = 0x2,
    MT_FLASHAVL_OPERATE_BLOCK = 0x4,
    MT_FLASHAVL_OPERATE_INTERRUPT = 0x8,
}mt_flashavl_operate_t;

typedef struct
{
    MT_S32    count;
    MT_U32    size;
    MT_BOOL   WDGOpen;
    MT_CHAR   file_name[128];
    MT_CHAR   mtddev_name[32];
    mt_flashavl_operate_t   runImg;
} mt_flashavl_para_t;
/********************** Global Variable declaration **************************/
static MT_U32 crc_table[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
static MT_BOOL g_bStopThread = MT_TRUE;
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_FlashAvlMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_U32 CRC32(const MT_U8 *data, MT_U32 length)
{
    MT_U32 crc = 0xFFFFFFFF;
    MT_U8 index = 0;

    for (MT_U32 i = 0; i < length; i++)
    {
        index = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc_table[index];
    }

    return crc ^ 0xFFFFFFFF;
}

//not used
#if 0
static MT_VOID GetTime(mt_float *ustime)
{
    struct timeval tv;
    MT_CHAR str[32] = { 0 };

    if (gettimeofday(&tv, NULL) == 0)
    {
        sprintf(str, "%ld.%ld", tv.tv_sec, tv.tv_usec);
        *ustime = atof(str);
    }
    else
    {
        SAMPLE_FLASHAVL_ERR_PRINT("Failed to get time\n");
    }
}
#endif

static MT_S32 GetFilePath(const MT_CHAR *url, MT_CHAR *path)
{
    MT_S32 directoryIndex = 0;
    MT_S32 pathLength = strlen(url);
    MT_S32 lenth = 0;
    MT_CHAR directory[256] = {0};

    if(url[0] == '.')
    {
        strcpy(path, "./");
        return MT_SUCCESS;
    }
    for(int i = 0; i < pathLength; i++)
    {
        if(url[i] == '/')
        {
            strcat(path, directory);
            lenth = strlen(path);
            path[lenth] = '/';
            memset(directory, 0, sizeof(directory));
            directoryIndex = 0;
        }
        else
        {
            directory[directoryIndex++] = url[i];
        }
    }
    return MT_SUCCESS;
}

static MT_S32 *MT_WdgFeedTask(MT_VOID *param)
{
    MT_S32 s32Ret = 0;

    while(g_bStopThread == MT_FALSE)
    {
        /* Clear WDG during timeout, can not reset system */
        s32Ret = mt_unf_wdg_clear(WDG_NO);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_wdg_clear err! s32Ret = 0x%x\n",s32Ret);
        }
    }
    return 0;
}

static MT_S32 MT_FlashAvlWDGOpen(pthread_t *stInjectTSThread)
{
    MT_S32  s32Ret = 0;
     /* Open WDG*/
    s32Ret = mt_unf_wdg_init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_wdg_init err! s32Ret = 0x%x\n",s32Ret);
        return s32Ret;
    }

    /* Set WDG TimeOut */
    s32Ret = mt_unf_wdg_set_timeout(WDG_NO, 2000);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_wdg_set_timeout err! s32Ret = 0x%x\n",s32Ret);
        (MT_VOID)mt_unf_wdg_deinit();
        return s32Ret;
    }

    /* Enable WDG */
    s32Ret = mt_unf_wdg_enable(WDG_NO);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_wdg_enable err! s32Ret = 0x%x\n",s32Ret);
        (MT_VOID)mt_unf_wdg_deinit();
        return s32Ret;
    }

    g_bStopThread = MT_FALSE;

    s32Ret = pthread_create(stInjectTSThread, NULL, (void * (*)(void *))MT_WdgFeedTask, NULL);
    if (MT_SUCCESS != s32Ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("pthread_create ret = %d\n", s32Ret);
        (MT_VOID)mt_unf_wdg_disable(WDG_NO);
        (MT_VOID)mt_unf_wdg_deinit();
        return s32Ret;
    }

    return MT_SUCCESS;
}

static MT_VOID MT_FlashAvlWDGClose(pthread_t stInjectTSThread)
{
    if(g_bStopThread == MT_FALSE)
    {
        g_bStopThread = MT_TRUE;
        (MT_VOID)pthread_join(stInjectTSThread, NULL);
    }
    (MT_VOID)mt_unf_wdg_disable(WDG_NO);
    (MT_VOID)mt_unf_wdg_deinit();
}

static MT_S32 MT_FlashAvlEraseMtd(MT_HANDLE hmtd)
{
    MT_S32  ret = 0;
    struct mtd_info_user mtd_info = { 0 };

    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_erase(hmtd, 0, mtd_info.erasesize);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_erase failed, ret = 0x%x \n", ret);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}
static MT_S32 MT_FlashAvlReadDate(MT_HANDLE hmtd, MT_U32 offset, MT_S32 size, MT_U32 *crc_readValue)
{
    MT_S32  ret = 0;
    MT_U8   *readBuffer = NULL;
    MT_U32 crc_value = 0xFFFFFFFF;

    readBuffer = (MT_U8*)malloc(size);

    ret = mt_unf_flash_read(hmtd, offset, readBuffer, size);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_read failed. ret = %#x\n", ret);
        free(readBuffer);
        return MT_FAILURE;
    }
    crc_value = CRC32(readBuffer, size);


    MT_FLASHAVL_PRINT("Read CRC32 checksum:  <0x%x>.\n", crc_value);

    *crc_readValue = crc_value;

    free(readBuffer);

    return MT_SUCCESS;

}

static MT_S32 MT_FlashAvlWriteInterrupt(MT_HANDLE hmtd, MT_U8 date, MT_S32 isize, MT_S32 *psize)
{
    MT_S32  ret = 0;
    MT_S32  offset = 0;
    MT_S32  maxWrite = 0;
    MT_S32  size = 0;
    MT_U32  flashLength = 0;
    MT_U8   *writeBuffer = MT_NULL;
    MT_U8   *buffer = MT_NULL;
    struct timespec start, end;
    double elapsed_seconds;
    struct mtd_info_user mtd_info = { 0 };

    maxWrite = SAMPLE_FLASHAVL_OPERATE_SIZE;

    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_get_avalen(hmtd, &flashLength);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_get_avalen failed.ret = %#x\n", ret);
        return MT_FAILURE;
    }

    if(flashLength != mtd_info.size)
    {
        MT_FLASHAVL_PRINT("The partition size is %d, the effective length of the partition is %d\n", mtd_info.size, flashLength);
        MT_FLASHAVL_PRINT("There are bad blocks and the bad block size is %d\n", mtd_info.size - flashLength);
    }

    if(0 != isize)
    {
        if(flashLength > isize)
        {
            *psize = isize;
        }
        else
        {
            *psize = flashLength;
        }
    }
    else
    {
        if(flashLength != mtd_info.size)
        {
            *psize = flashLength;
        }
        else
        {
            *psize = mtd_info.size;
        }
    }

    size = *psize / 2;

    if(size <= maxWrite)
    {
        writeBuffer = (MT_U8*)malloc(size);
        memset(writeBuffer, date, size);
    }
    else
    {
        writeBuffer = (MT_U8*)malloc(SAMPLE_FLASHAVL_OPERATE_SIZE);
        memset(writeBuffer, date, SAMPLE_FLASHAVL_OPERATE_SIZE / 2);
    }

    buffer = writeBuffer;

    clock_gettime(CLOCK_MONOTONIC, &start);
    MT_FLASHAVL_PRINT("Start time: %fs\n", start.tv_sec + (start.tv_nsec) / 1e9);
    if(size <= maxWrite)
    {
        ret = mt_unf_flash_write(hmtd, offset, buffer, size);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            free(writeBuffer);
            return MT_FAILURE;
        }
    }
    else
    {
        for(MT_S32 i = 0; i < size / maxWrite; i++)
        {
            ret = mt_unf_flash_write(hmtd, offset, buffer, maxWrite);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
                free(writeBuffer);
                return MT_FAILURE;
            }
            offset += maxWrite;
        }
        ret = mt_unf_flash_write(hmtd, offset, buffer, size % maxWrite);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            free(writeBuffer);
            return MT_FAILURE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    MT_FLASHAVL_PRINT("End time:   %fs\n", end.tv_sec + (end.tv_nsec) / 1e9);
    elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    MT_FLASHAVL_PRINT("Write time: %lf s\n", elapsed_seconds);

    free(writeBuffer);

    return MT_SUCCESS;
}

static MT_S32 MT_FlashAvlWriteBlock(MT_HANDLE hmtd, MT_U8 date, MT_S32 isize, MT_S32 *psize)
{
    MT_S32  ret = 0;
    MT_S32  offset = 0;
    MT_S32  maxWrite = 0;
    MT_S32  size = 0;
    MT_U32  flashLength = 0;
    MT_U8   *writeBuffer = MT_NULL;
    MT_U8   *buffer = MT_NULL;
    struct timespec start, end;
    double elapsed_seconds;
    struct mtd_info_user mtd_info = { 0 };

    maxWrite = SAMPLE_FLASHAVL_OPERATE_SIZE;

    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_get_avalen(hmtd, &flashLength);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_get_avalen failed.ret = %#x\n", ret);
        return MT_FAILURE;
    }

    if(flashLength != mtd_info.size)
    {
        MT_FLASHAVL_PRINT("The partition size is %d, the effective length of the partition is %d\n", mtd_info.size, flashLength);
        MT_FLASHAVL_PRINT("There are bad blocks and the bad block size is %d\n", mtd_info.size - flashLength);
    }

    if(0 != isize)
    {
        if(flashLength > isize)
        {
            *psize = isize;
        }
        else
        {
            *psize = flashLength;
        }
    }
    else
    {
        if(flashLength != mtd_info.size)
        {
            *psize = flashLength;
        }
        else
        {
            *psize = mtd_info.size;
        }
    }

    size = *psize / 2;

    if(size <= maxWrite)
    {
        writeBuffer = (MT_U8*)malloc(size);
        memset(writeBuffer, date, size);
    }
    else
    {
        writeBuffer = (MT_U8*)malloc(SAMPLE_FLASHAVL_OPERATE_SIZE);
        memset(writeBuffer, date, SAMPLE_FLASHAVL_OPERATE_SIZE / 2);
    }

    buffer = writeBuffer;

    clock_gettime(CLOCK_MONOTONIC, &start);
    MT_FLASHAVL_PRINT("Start time: %fs\n", start.tv_sec + (start.tv_nsec) / 1e9);
    if(size <= maxWrite)
    {
        ret = mt_unf_flash_write(hmtd, 0, buffer, size);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            free(writeBuffer);
            return MT_FAILURE;
        }
    }
    else
    {
        for(MT_S32 i = 0; i < size / maxWrite; i++)
        {
            ret = mt_unf_flash_write(hmtd, offset, buffer, maxWrite);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
                free(writeBuffer);
                return MT_FAILURE;
            }
            offset += maxWrite;
        }
        ret = mt_unf_flash_write(hmtd, offset, buffer, size % maxWrite);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            free(writeBuffer);
            return MT_FAILURE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    MT_FLASHAVL_PRINT("End time:   %fs\n", end.tv_sec + (end.tv_nsec) / 1e9);
    elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    MT_FLASHAVL_PRINT("Write time: %lf s\n", elapsed_seconds);

    free(writeBuffer);

    return MT_SUCCESS;
}


static MT_S32 MT_FlashAvlWriteDate(MT_HANDLE hmtd, MT_U8 date, MT_S32 issize, MT_S32 *psize, MT_U32 *crc_writeValue)
{
    MT_S32  ret = 0;
    MT_S32  offset = 0;
    MT_S32  maxWrite = 0;
    MT_U32  crc_value = 0xFFFFFFFF;
    MT_U32  flashLength = 0;
    struct timespec start, end;
    double elapsed_seconds;
    MT_U8   *writeBuffer = MT_NULL;
    MT_U8   *buffer = MT_NULL;
    struct mtd_info_user mtd_info = { 0 };

    maxWrite = SAMPLE_FLASHAVL_OPERATE_SIZE;

    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_get_avalen(hmtd, &flashLength);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_get_avalen failed.ret = %#x\n", ret);
        return MT_FAILURE;
    }

    if(flashLength != mtd_info.size)
    {
        MT_FLASHAVL_PRINT("The partition size is %d, the effective length of the partition is %d\n", mtd_info.size, flashLength);
        MT_FLASHAVL_PRINT("There are bad blocks and the bad block size is %d\n", mtd_info.size - flashLength);
    }

    if(0 != issize)
    {
        if(flashLength > issize)
        {
            *psize = issize;
        }
        else
        {
            *psize = flashLength;
        }
    }
    else
    {
        if(flashLength != mtd_info.size)
        {
            *psize = flashLength;
        }
        else
        {
            *psize = mtd_info.size;
        }
    }

    writeBuffer = (MT_U8*)malloc(*psize);
    memset(writeBuffer, date, *psize);

    buffer = writeBuffer;

    crc_value = CRC32(buffer, *psize);
    *crc_writeValue = crc_value;

    clock_gettime(CLOCK_MONOTONIC, &start);
    MT_FLASHAVL_PRINT("Start time: %fs\n", start.tv_sec + (start.tv_nsec) / 1e9);
    if(*psize <= maxWrite)
    {
        ret = mt_unf_flash_write(hmtd, 0, buffer, *psize);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            free(writeBuffer);
            return MT_FAILURE;
        }
    }
    else
    {
        for(MT_S32 i = 0; i < *psize / maxWrite; i++)
        {
            ret = mt_unf_flash_write(hmtd, offset, buffer, maxWrite);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
                free(writeBuffer);
                return MT_FAILURE;
            }
            buffer += maxWrite;
            offset += maxWrite;
        }
        ret = mt_unf_flash_write(hmtd, offset, buffer, *psize % maxWrite);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            free(writeBuffer);
            return MT_FAILURE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    MT_FLASHAVL_PRINT("End time:   %fs\n", end.tv_sec + (end.tv_nsec) / 1e9);
    elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    MT_FLASHAVL_PRINT("Write time: %lf s\n", elapsed_seconds);
    MT_FLASHAVL_PRINT("Write CRC32 checksum: 0x%x\n", crc_value);

    free(writeBuffer);

    return MT_SUCCESS;
}

static MT_S32 MT_FlashAvlWriteImg(MT_HANDLE hmtd, MT_CHAR *fileName, MT_S32 *psize, MT_U32 *crc_writeValue)
{
    MT_S32   ret = 0;
    MT_U32   Readlen = 0;
    MT_U32   flashLength = 0;
    MT_S32   offset = 0;
    MT_CHAR  path[64];
    MT_CHAR  cmdBuffer[64];
    struct timespec start, end;
    double elapsed_seconds;
    FILE     *pTsFile = NULL;
    FILE     *pFile = NULL;
    MT_U8    *head_buffer = NULL;
    MT_U8    *write_buffer = NULL;
    MT_U8    *src = NULL;
    MT_U32   size = 0;
    MT_U32   crc_value = 0xFFFFFFFF;
    MT_S32   maxWrite = 1024 * 1024 * 10;
    struct mtd_info_user mtd_info = { 0 };

    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n", ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_get_avalen(hmtd, &flashLength);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_get_avalen failed.ret = %#x\n", ret);
        return MT_FAILURE;
    }

    if(flashLength != mtd_info.size)
    {
        MT_FLASHAVL_PRINT("The partition size is %d, the effective length of the partition is %d\n", mtd_info.size, flashLength);
        MT_FLASHAVL_PRINT("There are bad blocks and the bad block size is %d\n", mtd_info.size - flashLength);
    }

    pTsFile = fopen(fileName, "rb");
    if(NULL == pTsFile)
    {
        SAMPLE_FLASHAVL_ERR_PRINT( "file open error!!\n");
        return MT_FAILURE;
    }

    if(-1 == ftell(pTsFile))
    {
        SAMPLE_FLASHAVL_ERR_PRINT("ftell error\n");
        fclose(pTsFile);
        pTsFile = NULL;
        return MT_FAILURE;
    }
    if(fseek(pTsFile, 0, SEEK_END) != 0)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("fseek failed\n");
        fclose(pTsFile);
        pTsFile = NULL;
        return MT_FAILURE;
    }


    size = ftell(pTsFile);
    if(size > flashLength)
    {
        MT_FLASHAVL_PRINT("file size: %d, flash effective size: %d\n", size, flashLength);
        SAMPLE_FLASHAVL_ERR_PRINT("The write file is too large\n");
        fclose(pTsFile);
        pTsFile = NULL;
        return MT_FAILURE;
    }

    *psize = size;

    head_buffer = (MT_U8*)malloc(size);
    src = head_buffer;
    write_buffer = head_buffer;

    if(fseek(pTsFile, 0, SEEK_SET) != 0)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("fseek failed\n");
        fclose(pTsFile);
        pTsFile = NULL;
        free(head_buffer);
        return MT_FAILURE;
    }

    if(size <= 10240)
    {
        Readlen = fread(src, sizeof(MT_U8), size, pTsFile);
    }
    else
    {
        for(MT_S32 i = 0; i < size / 10240; i++)
        {
            Readlen = fread(src, sizeof(MT_U8), 10240, pTsFile);
            if(Readlen <= 0)
            {
                break;
            }
            src += 10240;
        }
        fread(src, sizeof(MT_U8), size % 10240, pTsFile);
    }

    crc_value = CRC32(head_buffer, size);
    *crc_writeValue = crc_value;


    clock_gettime(CLOCK_MONOTONIC, &start);
    MT_FLASHAVL_PRINT("Start time: %fs\n", start.tv_sec + (start.tv_nsec) / 1e9);
    if(size <= maxWrite)
    {
        ret = mt_unf_flash_write(hmtd, 0, write_buffer, size);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            return MT_FAILURE;
        }
    }
    else
    {
        for(MT_S32 i = 0; i < size / maxWrite; i++)
        {
            ret = mt_unf_flash_write(hmtd, offset, write_buffer, maxWrite);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
                return MT_FAILURE;
            }
            write_buffer += maxWrite;
            offset += maxWrite;
        }
        ret = mt_unf_flash_write(hmtd, offset, write_buffer, size % maxWrite);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
            return MT_FAILURE;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    MT_FLASHAVL_PRINT("End time:   %fs\n", end.tv_sec + (end.tv_nsec) / 1e9);
    elapsed_seconds = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    MT_FLASHAVL_PRINT("Write time: %lf s\n", elapsed_seconds);

    MT_FLASHAVL_PRINT("Write CRC32 checksum: 0x%x\n", crc_value);

    memset(path, 0, sizeof(path));
    GetFilePath(fileName, path);
    strcat(path, "record.txt");

    sprintf(cmdBuffer, "touch %s", path);
    system(cmdBuffer);

    pFile = fopen(path, "a+");
    if(pFile == NULL)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("Unable to log, please confirm that the path %s exists\n", path);
    }
    else
    {
        fprintf(pFile, "Start time: %fs\n", start.tv_sec + (start.tv_nsec) / 1e9);
        fprintf(pFile, "End time:   %fs\n", end.tv_sec + (end.tv_nsec) / 1e9);
        fprintf(pFile, "Write time: %fs\n\n", elapsed_seconds);

        fclose(pFile);
        pFile = NULL;
    }

    fclose(pTsFile);
    pTsFile = NULL;
    free(head_buffer);

    return MT_SUCCESS;
}

static MT_S32 MT_FlashAvlRunInterrupt(mt_flashavl_para_t flashInfo)
{
    MT_S32    ret = 0;
    MT_U32    CrcReadValue = 0;
    MT_U32    CRCInit = 0;
    MT_S32    size = 0;
    MT_U32    flashLength = 0;
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;
    struct mtd_info_user mtd_info = { 0 };

    ret = mt_unf_flash_open(flashInfo.mtddev_name, &hmtdblock);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n", ret);
        SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
        return ret;
    }

    ret = mt_unf_flash_info(hmtdblock, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_get_avalen(hmtdblock, &flashLength);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_get_avalen failed.ret = %#x\n", ret);
        return MT_FAILURE;
    }

    if(0 == flashInfo.size)
    {
        if(flashLength != mtd_info.size)
        {
            MT_FLASHAVL_PRINT("The partition size is %d, the effective length of the partition is %d\n", mtd_info.size, flashLength);
            MT_FLASHAVL_PRINT("There are bad blocks and the bad block size is %d\n", mtd_info.size - flashLength);

            ret = MT_FlashAvlReadDate(hmtdblock, flashLength / 2, flashLength / 2, &CRCInit);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
                SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
                (MT_VOID)mt_unf_flash_close(hmtdblock);
                return MT_FAILURE;
            }
        }
        else
        {
            ret = MT_FlashAvlReadDate(hmtdblock, mtd_info.size / 2, mtd_info.size / 2, &CRCInit);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
                SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
                (MT_VOID)mt_unf_flash_close(hmtdblock);
                return MT_FAILURE;
            }
        }
    }
    else
    {
        if(flashLength > flashInfo.size)
        {
            ret = MT_FlashAvlReadDate(hmtdblock, flashInfo.size / 2, flashInfo.size / 2, &CRCInit);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
                SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
                (MT_VOID)mt_unf_flash_close(hmtdblock);
                return MT_FAILURE;
            }
        }
        else
        {
            MT_FLASHAVL_PRINT("Maximum read partition is %d\n", flashLength);
            ret = MT_FlashAvlReadDate(hmtdblock, flashLength / 2, flashLength / 2, &CRCInit);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
                SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
                (MT_VOID)mt_unf_flash_close(hmtdblock);
                return MT_FAILURE;
            }
        }
    }

    for(MT_S32 i = 0; i < flashInfo.count; i++)
    {
        MT_FLASHAVL_PRINT("\nThe current number of flashes is %d\n", i + 1);

        g_bStopThread = MT_TRUE;
        ret = MT_FlashAvlWriteInterrupt(hmtdblock, 0x5A, flashInfo.size, &size);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlWriteInterrupt failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlReadDate(hmtdblock, size / 2, size / 2, &CrcReadValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        if(CrcReadValue != CRCInit)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("The value of the area next to block is modified!\n");
            SAMPLE_FLASHAVL_ERR_PRINT("case interrupt failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }
    }

    sleep(1);
    MT_FLASHAVL_PRINT("case interrupt succeeded\n");
    (MT_VOID)mt_unf_flash_close(hmtdblock);

    return MT_SUCCESS;
}

static MT_S32 MT_FlashAvlRunBlock(mt_flashavl_para_t flashInfo)
{
    MT_S32    ret = 0;
    MT_S32    crcErr = 0;
    MT_S32    firErr = 0;
    MT_U32    CrcReadValue = 0;
    MT_U32    CRCInit = 0;
    MT_S32    size = 0;
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;

    ret = mt_unf_flash_open(flashInfo.mtddev_name, &hmtdblock);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n", ret);
        SAMPLE_FLASHAVL_ERR_PRINT("case block failed\n");
        return ret;
    }

    ret = MT_FlashAvlWriteBlock(hmtdblock, 0x5A, flashInfo.size, &size);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlWriteBlock failed.ret = %#x\n", ret);
        SAMPLE_FLASHAVL_ERR_PRINT("case block failed\n");
        (MT_VOID)mt_unf_flash_close(hmtdblock);
        return MT_FAILURE;
    }

    ret = MT_FlashAvlReadDate(hmtdblock, 0, size, &CRCInit);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
        SAMPLE_FLASHAVL_ERR_PRINT("case block failed\n");
        (MT_VOID)mt_unf_flash_close(hmtdblock);
        return MT_FAILURE;
    }

    for(MT_S32 i = 0; i < flashInfo.count; i++)
    {
        MT_FLASHAVL_PRINT("\nThe current number of flashes is %d\n", i + 1);

        ret = MT_FlashAvlWriteBlock(hmtdblock, 0x5A, flashInfo.size, &size);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlWriteBlock failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case block failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlReadDate(hmtdblock, 0, size, &CrcReadValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case block failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        MT_FLASHAVL_PRINT("Init CRC32 checksum:  0x%x\n", CRCInit);
        if(CrcReadValue != CRCInit)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("The value of the area next to block is modified! The current number of cycles %d\n", i + 1);
            crcErr += 1;
            if(FIRST_ERROR == crcErr)
            {
                firErr = i + 1;
            }
        }
    }

    MT_FLASHAVL_PRINT("Test results:\n");
    MT_FLASHAVL_PRINT("%d cycle, block data first error\n", firErr);
    MT_FLASHAVL_PRINT("%d block data errors\n", crcErr);
    MT_FLASHAVL_PRINT("case block succeeded\n");
    (MT_VOID)mt_unf_flash_close(hmtdblock);

    return MT_SUCCESS;
}

static MT_S32 MT_FlashAvlRunImg(mt_flashavl_para_t flashInfo)
{
    MT_S32    ret = 0;
    MT_S32    crcErr = 0;
    MT_S32    firErr = 0;
    MT_U32    CrcWriteValue = 0;
    MT_U32    CrcReadValue = 0;
    MT_S32    size = 0;
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;

    ret = mt_unf_flash_open(flashInfo.mtddev_name, &hmtdblock);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n", ret);
        SAMPLE_FLASHAVL_ERR_PRINT("case img failed\n");
        return MT_FAILURE;
    }

    for(MT_S32 i = 0; i < flashInfo.count; i++)
    {
        MT_FLASHAVL_PRINT("\nThe current number of flashes is %d\n", i + 1);
        ret = MT_FlashAvlEraseMtd(hmtdblock);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("case img failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlWriteImg(hmtdblock, flashInfo.file_name, &size, &CrcWriteValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlWriteImg failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case img failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlReadDate(hmtdblock, 0, size, &CrcReadValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case img failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        if(CrcWriteValue != CrcReadValue)
        {
            crcErr += 1;
            SAMPLE_FLASHAVL_ERR_PRINT("CRC validation failed! The current number of cycles %d\n", i + 1);
            if(FIRST_ERROR == crcErr)
            {
                firErr = i + 1;
            }
        }
    }

    MT_FLASHAVL_PRINT("Test results:\n");
    MT_FLASHAVL_PRINT("%d cycle, img data first error\n", firErr);
    MT_FLASHAVL_PRINT("%d img data errors\n", crcErr);
    MT_FLASHAVL_PRINT("case img succeeded\n");
    (MT_VOID)mt_unf_flash_close(hmtdblock);

    return MT_SUCCESS;
}


static MT_S32 MT_FlashAvlRunData(mt_flashavl_para_t flashInfo)
{
    MT_S32    ret = 0;
    MT_U32    CrcWriteValue = 0;
    MT_U32    CrcReadValue = 0;
    MT_S32    size = 0;
    MT_S32    crc5AErr = 0;
    MT_S32    crcA5Err = 0;
    MT_S32    firErr5A = 0;
    MT_S32    firErrA5 = 0;
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;

    ret = mt_unf_flash_open(flashInfo.mtddev_name, &hmtdblock);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n", ret);
        SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
        return ret;
    }

    for(MT_S32 i = 0; i < flashInfo.count; i++)
    {
        MT_FLASHAVL_PRINT("\nThe current number of times 5A data is written is %d\n", i + 1);
        ret = MT_FlashAvlEraseMtd(hmtdblock);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlWriteDate(hmtdblock, 0x5A, flashInfo.size, &size, &CrcWriteValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlWriteDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlReadDate(hmtdblock, 0, size, &CrcReadValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        if(CrcWriteValue != CrcReadValue)
        {
            crc5AErr += 1;
            SAMPLE_FLASHAVL_ERR_PRINT("5A data CRC validation failed! The current number of cycles %d\n", i + 1);
            if(FIRST_ERROR == crc5AErr)
            {
                firErr5A = i + 1;
            }
        }

        MT_FLASHAVL_PRINT("\nThe current number of times A5 data is written is %d\n", i + 1);
        ret = MT_FlashAvlEraseMtd(hmtdblock);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlWriteDate(hmtdblock, 0xA5, flashInfo.size, &size, &CrcWriteValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlWriteDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        ret = MT_FlashAvlReadDate(hmtdblock, 0, size, &CrcReadValue);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlReadDate failed.ret = %#x\n", ret);
            SAMPLE_FLASHAVL_ERR_PRINT("case A5 data failed\n");
            (MT_VOID)mt_unf_flash_close(hmtdblock);
            return MT_FAILURE;
        }

        if(CrcWriteValue != CrcReadValue)
        {
            crcA5Err += 1;
            SAMPLE_FLASHAVL_ERR_PRINT("A5 data CRC validation failed! The current number of cycles %d\n", i + 1);
            if(FIRST_ERROR == crcA5Err)
            {
                firErrA5 = i + 1;
            }
        }
    }

    MT_FLASHAVL_PRINT("Test results:\n");
    MT_FLASHAVL_PRINT("%d cycle, 5A data first error\n", firErr5A);
    MT_FLASHAVL_PRINT("%d cycle, A5 data first error\n", firErrA5);
    MT_FLASHAVL_PRINT("%d 5A data errors\n", crc5AErr);
    MT_FLASHAVL_PRINT("%d A5 data errors\n", crcA5Err);

    MT_FLASHAVL_PRINT("case A5 data succeeded\n");
    (MT_VOID)mt_unf_flash_close(hmtdblock);
    return MT_SUCCESS;
}


static MT_VOID MT_FlashAvlPrint_Help(MT_CHAR *name)
{
    MT_FLASHAVL_PRINT("Lack of parameters\n");
    MT_FLASHAVL_PRINT("\nUsage:\n");
    MT_FLASHAVL_PRINT("%s\n", name);
    MT_FLASHAVL_PRINT("    -c: Write the image file to the specified range\n");
    MT_FLASHAVL_PRINT("    -n: The specified region writes 5A and A5 data\n");
    MT_FLASHAVL_PRINT("    -f: Check if the value next to the block has been modified\n");
    MT_FLASHAVL_PRINT("    -i: Check if the value next to the block has been modified(Read and write half)\n");
    MT_FLASHAVL_PRINT("example:\n");
    MT_FLASHAVL_PRINT("    %s -c /dev/mtd7 ./usrfs.img 1\n", name);
    MT_FLASHAVL_PRINT("    %s -n /dev/mtd8 1024000 1\n", name);
    MT_FLASHAVL_PRINT("    %s -f /dev/mtd8 1024000 1\n", name);
    MT_FLASHAVL_PRINT("    %s -i /dev/mtd8 1 1024000 0\n", name);
}

static MT_S32 MT_FlashParase_args(MT_S32 argc, MT_CHAR *argv[], mt_flashavl_para_t *flashInfo)
{
    MT_S32 opt = 0;

    if(argc < 2)
    {
        (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHc:n:i:f:s")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'c':
                if(argc != 5)
                {
                    (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                MTADP_Strncpy(flashInfo->mtddev_name, argv[2], sizeof(flashInfo->mtddev_name));
                MTADP_Strncpy(flashInfo->file_name, argv[3], sizeof(flashInfo->file_name));
                flashInfo->count = strtol(argv[4], 0, 0);
                flashInfo->runImg = MT_FLASHAVL_OPERATE_IMG;
                return MT_SUCCESS;
            case 'n':
                if(argc != 5)
                {
                    (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                MTADP_Strncpy(flashInfo->mtddev_name, argv[2], sizeof(flashInfo->mtddev_name));
                flashInfo->size = strtol(argv[3], 0, 0);
                flashInfo->count = strtol(argv[4], 0, 0);
                flashInfo->runImg = MT_FLASHAVL_OPERATE_DATA;
                return MT_SUCCESS;
            case 'i':
                if(argc != 6)
                {
                    (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                MTADP_Strncpy(flashInfo->mtddev_name, argv[2], sizeof(flashInfo->mtddev_name));
                flashInfo->WDGOpen = strtol(argv[3], 0, 0);
                flashInfo->size = strtol(argv[4], 0, 0);
                flashInfo->count = strtol(argv[5], 0, 0);
                flashInfo->runImg = MT_FLASHAVL_OPERATE_INTERRUPT;
                return MT_SUCCESS;
            case 'f':
                if(argc != 5)
                {
                    (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
                    return MT_FAILURE;
                }

                MTADP_Strncpy(flashInfo->mtddev_name, argv[2], sizeof(flashInfo->mtddev_name));
                flashInfo->size = strtol(argv[3], 0, 0);
                flashInfo->count = strtol(argv[4], 0, 0);
                flashInfo->runImg = MT_FLASHAVL_OPERATE_BLOCK;
                return MT_SUCCESS;
            default:
                (MT_VOID)MT_FlashAvlPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }


    return MT_SUCCESS;

}


#ifdef MT_SAMPLE_APP
MT_S32 MT_FlashAvlMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32    ret = 0;
    mt_flashavl_para_t flashInfo = { 0 };
    pthread_t stInjectTSThread;

    ret = MT_FlashParase_args(argc, argv, &flashInfo);
    if (MT_FAILURE == ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("failed to mt_sys_init\n");
        return ret;
    }
#endif

    if(flashInfo.WDGOpen == MT_TRUE)
    {
        ret = MT_FlashAvlWDGOpen(&stInjectTSThread);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlRunImg failed.ret = %#x\n",ret);
            goto ERR0;
        }
    }

    ret = mt_unf_flash_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASHAVL_ERR_PRINT("mt_unf_flash_init failed.ret = %#x\n",ret);
        goto ERR1;
    }

    if(flashInfo.runImg == MT_FLASHAVL_OPERATE_IMG)
    {
        ret = MT_FlashAvlRunImg(flashInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlRunImg failed.ret = %#x\n",ret);
            goto ERR2;
        }
    }
    else if(flashInfo.runImg == MT_FLASHAVL_OPERATE_DATA)
    {
       ret = MT_FlashAvlRunData(flashInfo);
       if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlRunData failed.ret = %#x\n",ret);
            goto ERR2;
        }
    }
    else if(flashInfo.runImg == MT_FLASHAVL_OPERATE_BLOCK)
    {
       ret = MT_FlashAvlRunBlock(flashInfo);
       if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlRunBlock failed.ret = %#x\n",ret);
            goto ERR2;
        }
    }
    else if(flashInfo.runImg == MT_FLASHAVL_OPERATE_INTERRUPT)
    {
        ret = MT_FlashAvlRunInterrupt(flashInfo);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_FLASHAVL_ERR_PRINT("MT_FlashAvlRunInterrupt failed.ret = %#x\n",ret);
            goto ERR2;
        }
    }
ERR2:
    (MT_VOID)mt_unf_flash_deinit();
ERR1:
    if(flashInfo.WDGOpen == MT_TRUE)
    {
        (MT_VOID)MT_FlashAvlWDGClose(stInjectTSThread);
    }
ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return MT_SUCCESS;
}

