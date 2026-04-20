/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_common.h"
#include "mt_flash.h"
#include "mt_unf_cipher_v2.h"
#include "mt_adp_mpi.h"

#ifdef MT_SAMPLE_HMAC_DEBUG
#define MT_HMAC_PRINT   printf
#else
#define MT_HMAC_PRINT
#endif

#define SAMPLE_HMAC_FUNCTION_ENTER()    MT_HMAC_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_HMAC_FUNCTION_EXIT()     MT_HMAC_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_HMAC_FATAL_PRINT(fmt...)     MT_HMAC_PRINT(" [FATAL] " fmt)
#define SAMPLE_HMAC_ERR_PRINT(fmt...)       MT_HMAC_PRINT(" [ERROR] " fmt)
#define SAMPLE_HMAC_WARN_PRINT(fmt...)      MT_HMAC_PRINT(" [WARN] "  fmt)
#define SAMPLE_HMAC_INFO_PRINT(fmt...)      MT_HMAC_PRINT(" [INFO] "  fmt)
#define SAMPLE_HMAC_DBG_PRINT(fmt...)       MT_HMAC_PRINT(" [DEBUG] " fmt)

#define SAMPLE_HMAC_PRINT   printf
#define MT_SAMPLE_HMAC_KEY_SIZE     64
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

typedef struct tagSource_Param_T
{
    mt_char datafile[256];
    mt_char hmackey[256];
}source_param_t;

static mt_u8 hmac_key[MT_SAMPLE_HMAC_KEY_SIZE] = {
      0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31,
      0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,
      0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
      0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
      0x31,0x31,0x31,0x31,0x31,0x31,0x31,0x31,
      0x32,0x32,0x32,0x32,0x32,0x32,0x32,0x32,
      0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33,
      0x34,0x34,0x34,0x34,0x34,0x34,0x34,0x34,
  };

static MT_BOOL    g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
MT_S32 MT_HmacSha1Main(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif



static MT_S32 MT_HmacRead_file(char *file_name, size_t *out_file_size, mt_u8 **buf)
{
    size_t file_size = 0;
    size_t read_num = 0;
    mt_u8 *temp_buf = NULL;
    FILE *fp = NULL;

    if (NULL == out_file_size)
    {
        SAMPLE_HMAC_ERR_PRINT("out_file_size is null \n");
        return MT_FAILURE;
    }

    fp = fopen(file_name, "rb");
    if(NULL == fp)
    {
        SAMPLE_HMAC_ERR_PRINT("fopen %s failed\n", file_name);
        return MT_FAILURE;
    }

    //get file size
    fseek(fp, 0, SEEK_END);
    file_size = (size_t)ftell(fp);
    if(file_size == 0)
    {
        SAMPLE_HMAC_ERR_PRINT("file_size err \n");
        fclose(fp);
        return MT_FAILURE;
    }
    SAMPLE_HMAC_INFO_PRINT("sizeof %s is:%lu\n", file_name, file_size);
    fseek(fp, 0, SEEK_SET);


    temp_buf = malloc(file_size);



    //read data
    read_num = fread(temp_buf, 1, file_size, fp);
    if (read_num < file_size)
    {

        free(temp_buf);

        fclose(fp);
        SAMPLE_HMAC_ERR_PRINT("read file failed\n");
        return MT_FAILURE;
    }

    fclose(fp);


    *out_file_size = read_num;
    *buf = temp_buf;

    return MT_SUCCESS;
}


static void MT_HmacDump(const char *tag, mt_u8 *buffer, mt_u32 len)
{
    mt_u32 i = 0;
    SAMPLE_HMAC_PRINT("\n%s:\n", tag);
    SAMPLE_HMAC_PRINT("==================================\n");
    for(i = 0; i < len; i++)
    {
        if((i%16) == 0 && i != 0)
            SAMPLE_HMAC_PRINT("\n");
        SAMPLE_HMAC_PRINT("%02X ", buffer[i]);
    }
    SAMPLE_HMAC_PRINT("\n==================================\n");
}

static MT_S32 MT_HmacSha(MT_CIPHER_HMAC_ATTS_S *hmac_attr, MT_CIPHER_HASH_TYPE_E hash_type, mt_u8 *temp_buffer, mt_u32 length)
{
    mt_s32 ret = 0;
    mt_u8 *hash_value = NULL;
    mt_u32 hash_value_len = 20;
    mt_u8 *buffer = NULL;
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_u32 p_cipher = 0;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    ulong p_cipher = 0;
#endif

    if(hash_type == MT_CIPHER_HASH_TYPE_SHA1)
    {
        hash_value_len = 20;
    }
    else
    {
        hash_value_len = 32;
    }
    hash_value = mt_unf_cipher_malloc(hash_value_len);
    if(hash_value == NULL)
    {
        SAMPLE_HMAC_ERR_PRINT("malloc hash_value err \n");
        return  MT_FAILURE;
    }
    memset(hash_value, 0, hash_value_len);
    //ret = mt_unf_cipher_hash_create(hash_type, hmac_attr,&p_cipher);
    ret = mt_unf_cipher_mac_create(MT_CIPHER_MAC_TYPE_SHA1, hmac_attr,&p_cipher);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HMAC_ERR_PRINT("mt_unf_cipher_hash_create err%d \n", ret);
        (MT_VOID)mt_unf_cipher_free(hash_value);
        return  MT_FAILURE;
    }
    buffer = mt_unf_cipher_malloc(length);
    if(NULL == buffer)
    {
        SAMPLE_HMAC_ERR_PRINT("malloc buffer err \n");
        return  MT_FAILURE;
    }
    memcpy(buffer, temp_buffer, length);
   // ret = mt_unf_cipher_hash_update(p_cipher,  buffer, length);
    ret = mt_unf_cipher_mac_update(p_cipher,  buffer, length);

    if (MT_SUCCESS != ret)
    {
        SAMPLE_HMAC_ERR_PRINT("mt_unf_cipher_hash_update err%d \n",ret);
        (MT_VOID)mt_unf_cipher_free(buffer);
        (MT_VOID)mt_unf_cipher_free(hash_value);
        return  MT_FAILURE;
    }
    ret = mt_unf_cipher_mac_final(p_cipher, hash_value);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_HMAC_ERR_PRINT("mt_unf_cipher_hash_final err \n");
        (MT_VOID)mt_unf_cipher_free(buffer);
        (MT_VOID)mt_unf_cipher_free(hash_value);
        return  MT_FAILURE;
    }
    (MT_VOID)MT_HmacDump("Output", hash_value, hash_value_len);
    (MT_VOID)mt_unf_cipher_free(buffer);
    (MT_VOID)mt_unf_cipher_free(hash_value);
    return MT_SUCCESS;
}

static MT_VOID MT_HmacPrintMenu(MT_VOID)
{
    SAMPLE_HMAC_PRINT("\n");
    SAMPLE_HMAC_PRINT("     a : hmac to sha1 \n");
    SAMPLE_HMAC_PRINT("     d : hmac to sha256 \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_HMAC_PRINT("     b : background run \n");
#endif
    SAMPLE_HMAC_PRINT("     h : help \n");
    SAMPLE_HMAC_PRINT("     q : quit \n");
    SAMPLE_HMAC_PRINT("Hmac>> ");

}

static void MT_HmacPrint_help(char *name)
{
    SAMPLE_HMAC_ERR_PRINT("Lack of parameters\n");
    SAMPLE_HMAC_ERR_PRINT("such as: %s -f datafile \n", name);
    SAMPLE_HMAC_ERR_PRINT(" %s -q  <exit> \n", name);
}


static mt_s32 MT_HmacCmdTask(mt_u8 *buffer, mt_u32 length, MT_CIPHER_HMAC_ATTS_S *hmac_attr)
{
    MT_S32     ret = MT_FAILURE;
    MT_CHAR    inputCmd[32] = { 0 };
    MT_CHAR    *pfgetret = NULL;

    while(1)
    {
        (MT_VOID)MT_HmacPrintMenu();
        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_HMAC_INFO_PRINT("<Exit!>\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        #ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_HMAC_INFO_PRINT("Dvbs play in back!\n");
            break;
        }
        #endif
        else if('a' == inputCmd[0])
        {
            ret = MT_HmacSha(hmac_attr, MT_CIPHER_MAC_TYPE_SHA1, buffer, length);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_HMAC_ERR_PRINT("failed to MT_HashSha1 \n");
                return MT_FAILURE;
            }
            continue;
        }
        else if('d' == inputCmd[0])
        {
            ret = MT_HmacSha(hmac_attr, MT_CIPHER_HASH_TYPE_SHA256, buffer, length);
             if(MT_SUCCESS != ret)
            {
                SAMPLE_HMAC_ERR_PRINT("failed to MT_HashSha256 \n");
                return MT_FAILURE;
            }
            continue;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_HMAC_INFO_PRINT("Print help info \n");
            continue;
        }
    }
    return MT_SUCCESS;
}

static void MT_HmacExit(void)
{
    (MT_VOID)mt_unf_cipher_deinit();
    g_bTaskQuit = MT_TRUE;
}

static mt_s32 MT_HmacParase_args(int argc, char *argv[], source_param_t *pInparam)
{
    int opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_HmacPrint_help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_HmacExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                MTADP_Strncpy(pInparam->datafile, mt_optarg, sizeof(pInparam->datafile));
                break;

            default:
                (void)MT_HmacPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }

    return MT_SUCCESS;
}



#ifdef MT_SAMPLE_APP
MT_S32 MT_HmacSha1Main(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_u8 *buffer = NULL;
    mt_u32 ret = MT_FAILURE;
    MT_CIPHER_HMAC_ATTS_S hmac_attr = { 0 };
    source_param_t stParam = { 0 };
    size_t buffer_size = 0;


    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_HmacPrint_help(argv[0]);
        return MT_SUCCESS;
    }
    ret = MT_HmacParase_args(argc, argv, &stParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_HMAC_ERR_PRINT("Parase args err. stop window.\n");
        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_HMAC_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
   }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = mt_sys_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_HMAC_ERR_PRINT("failed to mt_sys_init \n");
            return MT_FAILURE;
        }
#endif
        ret = mt_unf_cipher_init();
        if(MT_SUCCESS != ret)
        {
            SAMPLE_HMAC_ERR_PRINT("failed to mt_unf_cipher_init \n");
            goto ERR1;
        }

        SAMPLE_HMAC_INFO_PRINT("Calc hmac of user input data:\n");


        ret = MT_HmacRead_file(stParam.datafile, &buffer_size, &buffer);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_HMAC_ERR_PRINT("failed to MT_HmacRead_file \n");
            goto ERR2;
        }

        hmac_attr.p_hmac_key = hmac_key;
        hmac_attr.key_len = MT_SAMPLE_HMAC_KEY_SIZE;
        g_bTaskQuit = MT_FALSE;
    }
    (MT_VOID)MT_HmacCmdTask(buffer, buffer_size, &hmac_attr);
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR2:
     (MT_VOID)MT_HmacExit();
ERR1:
     #ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
    #endif

    return MT_SUCCESS;
}


