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
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_FLASH_DEBUG

#define MT_FLASH_PRINT   printf
#else

#define MT_FLASH_PRINT

#endif

#define SAMPLE_FLASH_FUNCTION_ENTER()   MT_FLASH_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_FLASH_FUNCTION_EXIT()    MT_FLASH_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_FLASH_FATAL_PRINT(fmt...)        MT_FLASH_PRINT(" [FATAL] " fmt)
#define SAMPLE_FLASH_ERR_PRINT(fmt...)          MT_FLASH_PRINT(" [ERROR] " fmt)
#define SAMPLE_FLASH_WARN_PRINT(fmt...)         MT_FLASH_PRINT(" [WARN] "  fmt)
#define SAMPLE_FLASH_INFO_PRINT(fmt...)         MT_FLASH_PRINT(" [INFO] "  fmt)
#define SAMPLE_FLASH_DBG_PRINT(fmt...)          MT_FLASH_PRINT(" [DEBUG] " fmt)

#define SAMPLE_FLASH_PRINT  printf

#define SAMPLE_FLASH_MAX_OP_SIZE 0x20000

#define SAMPLE_FLASH_OPERATE_SIZE    0x100
#define SAMPLE_FLASH_OP_EARSE_SIZE   0x20000


/*************************** Structure Definition ****************************/
typedef struct
{
    MT_CHAR mtddev_name[32];
} mt_flashavl_para_t;
/********************** Global Variable declaration **************************/
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_FlashMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif


static MT_VOID MT_FlashPrintData(MT_U8 *title, MT_U8 *data, MT_U32 length)
{
#ifdef MT_SAMPLE_FLASH_DEBUG
    SAMPLE_FLASH_PRINT("Print: %s \n", title);
    for(int i = 0; i < length; i++)
    {
        if(i % 16 == 0)
        {
            SAMPLE_FLASH_PRINT("\n\t");
        }
        SAMPLE_FLASH_PRINT("%02x ", data[i]);
    }
    SAMPLE_FLASH_PRINT("\n");
#endif
}


static MT_VOID MT_FlashPrintMtdInfo(MT_HANDLE hmtd)
{
    MT_S32  ret = 0;
    MT_U32  flash_id = 0;
    MT_U8   id_uni[32] = { 0 };
    MT_U32  uid_len = 16;
    struct mtd_info_user mtd_info = { 0 };

    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return;
    }

    if(MTD_NANDFLASH == mtd_info.type)
    {
        uid_len = 32;
    }

    ret = mt_unf_flash_id_uni(hmtd, id_uni, uid_len);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_id_uni failed.ret = %#x\n",ret);
        return;
    }

    ret = mt_unf_flash_id(hmtd, &flash_id);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_id failed.ret = %#x\n",ret);
        return;
    }

    SAMPLE_FLASH_PRINT("\tThe basic information of partion:\n");
    SAMPLE_FLASH_PRINT("\tid:        0x%x\n", flash_id);
    SAMPLE_FLASH_PRINT("\ttype:      0x%x\n", mtd_info.type);
    SAMPLE_FLASH_PRINT("\tsize:      0x%x\n", mtd_info.size);
    SAMPLE_FLASH_PRINT("\terasesize: 0x%x\n", mtd_info.erasesize);
    SAMPLE_FLASH_PRINT("\twritesize: 0x%x\n", mtd_info.writesize);
    SAMPLE_FLASH_PRINT("\toobSize:   0x%x\n", mtd_info.oobsize);
    SAMPLE_FLASH_PRINT("\tunique id:\n\t  ");
    for(MT_S32 i = 0; i < 16; i++)
    {
        SAMPLE_FLASH_PRINT("0x%02x ", id_uni[i]);
    }

    if(MTD_NANDFLASH == mtd_info.type)
    {
        SAMPLE_FLASH_PRINT("\n\tcomplement:\n\t  ");
        for(MT_S32 i = 16; i < 32; i++)
        {
            SAMPLE_FLASH_PRINT("0x%02x ", id_uni[i]);
        }
        SAMPLE_FLASH_PRINT("\n\tXOR:\n\t  ");
        for(MT_S32 i = 0; i < 16; i++)
        {
            SAMPLE_FLASH_PRINT("0x%02x ", (id_uni[i] ^ id_uni[16+i]));
        }
    }
    SAMPLE_FLASH_PRINT("\n");

}

static MT_S32 MT_FlashEraseMtd(MT_HANDLE hmtd, MT_U32 offset)
{
    MT_S32  ret = 0;
    struct mtd_info_user mtd_info = { 0 };

    SAMPLE_FLASH_FUNCTION_ENTER();
    ret = mt_unf_flash_info(hmtd, (void*)&mtd_info);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_info failed.ret = %#x\n",ret);
        return MT_FAILURE;
    }

    ret = mt_unf_flash_erase(hmtd, offset, mtd_info.erasesize);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("fail to erase %x, ret = 0x%x \n", offset, ret);
        return MT_FAILURE;
    }


    SAMPLE_FLASH_INFO_PRINT("Erase successful!\n");

    SAMPLE_FLASH_FUNCTION_EXIT();

    return MT_SUCCESS;
}
static MT_S32 MT_FlashReadDate(MT_HANDLE hmtd, MT_U32 offset)
{
    MT_S32  ret = 0;
    MT_U8   readBuffer[SAMPLE_FLASH_OPERATE_SIZE] = {0};

    SAMPLE_FLASH_FUNCTION_ENTER();
    ret = mt_unf_flash_read(hmtd, offset, readBuffer, (MT_U32)SAMPLE_FLASH_OPERATE_SIZE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_read failed. offset: 0x%x ret = %#x\n", offset, ret);
        return MT_FAILURE;
    }

    SAMPLE_FLASH_INFO_PRINT("offset: 0x%x\n", offset);

    MT_FlashPrintData("Read data: ", readBuffer, SAMPLE_FLASH_OPERATE_SIZE);


    SAMPLE_FLASH_INFO_PRINT("Read successful!\n");
    SAMPLE_FLASH_FUNCTION_EXIT();
    return MT_SUCCESS;

}

static MT_S32 MT_FlashWriteDate(MT_HANDLE hmtd, MT_U32 offset)
{
    MT_S32  ret = 0;
    MT_U8   writeBuffer[SAMPLE_FLASH_OPERATE_SIZE] = {0};

    SAMPLE_FLASH_FUNCTION_ENTER();

    for(int i = 0; i < SAMPLE_FLASH_OPERATE_SIZE; i++)
    {
        writeBuffer[i] = i;
    }

    MT_FlashPrintData("Write data: ", writeBuffer, SAMPLE_FLASH_OPERATE_SIZE);

    ret = mt_unf_flash_write(hmtd, offset, writeBuffer, (MT_U32)SAMPLE_FLASH_OPERATE_SIZE);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_write failed. offset: 0x%x ret = %#x\n", offset, ret);
        return MT_FAILURE;
    }

    SAMPLE_FLASH_INFO_PRINT("Write succeeded!\n");
    SAMPLE_FLASH_FUNCTION_EXIT();
    return MT_SUCCESS;
}


/*!
@brief help
@param[in]     MT_VOID
@return MT_VOID
*/
static MT_VOID MT_FlashPrintMenu(MT_VOID)
{
    MT_FLASH_PRINT("\ncommond: \n");
    MT_FLASH_PRINT("     i : Basic information about the partition\n");
    MT_FLASH_PRINT("     w : Write data to the partition \n");
    MT_FLASH_PRINT("     r : Read data from the partition \n");
    MT_FLASH_PRINT("     e : Erase partition data\n");
    MT_FLASH_PRINT("     h : help \n");
    MT_FLASH_PRINT("     q : quit \n");
    MT_FLASH_PRINT("FLASH>> ");
}


/*!
@brief flash to achieve write, read, erase
@param[in] gmtd_handle  mtd handle
@return::MT_VOID
*/
static MT_VOID MT_FlashCmdTask(MT_HANDLE gmtd_handle)
{
    MT_U32  mtdOffset = 0;
    MT_S32  ret = 0;
    MT_CHAR inputCmd[128];



    while (1)
    {
        (MT_VOID)MT_FlashPrintMenu();

        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        if('q' == inputCmd[0])
        {
           SAMPLE_FLASH_INFO_PRINT("<Exit>!\n");
           break;

        }
        else if('i' == inputCmd[0])
        {
            (MT_VOID)MT_FlashPrintMtdInfo(gmtd_handle);

        }
        else if('e' == inputCmd[0])
        {
            ret = MT_FlashEraseMtd(gmtd_handle, mtdOffset);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_write failed.ret = %#x\n",ret);
            }
        }
        else if('r' == inputCmd[0])
        {
            ret = MT_FlashReadDate(gmtd_handle, mtdOffset);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_write failed.ret = %#x\n",ret);
            }
        }
        else if('w' == inputCmd[0])
        {
            ret = MT_FlashWriteDate(gmtd_handle, mtdOffset);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_write failed.ret = %#x\n",ret);
            }
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_FLASH_INFO_PRINT("Print help info \n");
            continue;
        }


    }
}

static MT_VOID MT_FlashPrint_Help(MT_CHAR *name)
{
    MT_FLASH_PRINT("Lack of parameters\n");
    MT_FLASH_PRINT("\nUsage:\n");
    MT_FLASH_PRINT("%s\n", name);
    MT_FLASH_PRINT("    -f: flash block name\n");
    MT_FLASH_PRINT("example:\n");
    MT_FLASH_PRINT("    %s -f /dev/mtd13\n", name);
}

static MT_S32 MT_FlashParase_args(MT_S32 argc, MT_CHAR *argv[], mt_flashavl_para_t *flashInfo)
{
    MT_S32 opt = 0;

    if(argc != 3)
    {
        (MT_VOID)MT_FlashPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hH:f")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_FlashPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                MTADP_Strncpy(flashInfo->mtddev_name, argv[2], sizeof(flashInfo->mtddev_name));
                return MT_SUCCESS;
            default:
                (MT_VOID)MT_FlashPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;

}



#ifdef MT_SAMPLE_APP
MT_S32 MT_FlashMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32    ret = 0;

    mt_flashavl_para_t flashInfo = { 0 };
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;

    ret = MT_FlashParase_args(argc, argv, &flashInfo);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("failed to mt_sys_init\n");
        return ret;
    }
#endif

    ret = mt_unf_flash_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_init failed.ret = %#x\n",ret);
        goto ERR0;
    }

    MT_FLASH_PRINT("====Opening flash.....\n");

    ret = mt_unf_flash_open(flashInfo.mtddev_name, &hmtdblock);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_FLASH_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n",ret);
        goto ERR1;
    }

    (MT_VOID)MT_FlashCmdTask(hmtdblock);

    (MT_VOID)mt_unf_flash_close(hmtdblock);
ERR1:
    (MT_VOID)mt_unf_flash_deinit();
ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return MT_SUCCESS;
}

