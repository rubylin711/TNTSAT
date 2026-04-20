#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <mt_unf_ir.h>
#include <mt_unf_keyled.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "mt_unf_pm.h"
#include "mt_unf_cipher_v2.h"
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_CIPHER_R2R_DEBUG
#define MT_CIPHER_R2R_PRINT           printf
#else
#define MT_CIPHER_R2R_PRINT
#endif

#define SAMPLE_CIPHER_R2R_FATAL_PRINT(fmt...)      MT_CIPHER_R2R_PRINT(" [FATAL] " fmt)
#define SAMPLE_CIPHER_R2R_ERR_PRINT(fmt...)        MT_CIPHER_R2R_PRINT(" [ERROR] " fmt)
#define SAMPLE_CIPHER_R2R_WARN_PRINT(fmt...)       MT_CIPHER_R2R_PRINT(" [WARN] "  fmt)
#define SAMPLE_CIPHER_R2R_INFO_PRINT(fmt...)       MT_CIPHER_R2R_PRINT(" [INFO] "  fmt)
#define SAMPLE_CIPHER_R2R_DBG_PRINT(fmt...)        MT_CIPHER_R2R_PRINT(" [DEBUG] " fmt)

#define SAMPLE_CIPHER_R2R_PRINT      printf

#define MAX_KEY_SIZE     256
#define DATA_MEM_SIZE    128

#define MT_R2R_READ_LEN  128
#define MT_R2R_ALG_DES   0
#define MT_R2R_ALG_TDES  1
#define MT_R2R_ALG_AES   2
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
static const u8 g_AESkey[] = {
    0x1F,0xDF,0xFA,0x9D,0xAE,0x67,0xFF,0x34,
    0xEC,0x5A,0xAA,0x1C,0xF1,0x2C,0x1D,0x38
};

static const u8 g_DESkey[] = {
    0x2F,0xDF,0xFA,0x9D,0xAE,0x67,0xFF,0x34
};

static const u8 g_TDESkey[] = {
    0x3F,0xDF,0xFA,0x9D,0xAE,0x67,0xFF,0x34,
    0x77,0x91,0xD3,0x69,0xAE,0xCB,0x56,0xE3
};

static const u8 g_IV[] = {
    0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
    0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10
};
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_R2RMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static void dump_data(const MT_CHAR *title, MT_U8 *buf, MT_U32 size)
{
    u32 i;

    if(buf == NULL || size == 0)
    {
        return;
    }

    if(title != NULL)
    {
        SAMPLE_CIPHER_R2R_PRINT("%s: start", title);
    }
    for(i = 0; i < size; i++)
    {
        if(i % 16 == 0)
        {
        SAMPLE_CIPHER_R2R_PRINT("\n 0x%p: ", buf + i);
        }
        SAMPLE_CIPHER_R2R_PRINT("%02x ", buf[i]);
    }

    SAMPLE_CIPHER_R2R_PRINT("\n%s: end\n\n", title);

}


static MT_S32 MT_R2RFileRead(MT_CHAR fileName[256], MT_U8 **data, MT_S32 *file_size)
{
    FILE *fp;
    MT_S32 i = 0;
    MT_S32 size = 0;
    MT_S32 readlen = 0;
    MT_U8  *src = NULL;


    if((fp = fopen(fileName, "r")) == NULL)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("Can not open file!\n");
        return MT_FAILURE;
    }

    if(-1 == ftell(fp))
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("ftell error\n");
        return MT_FAILURE;
    }
    if(fseek(fp, 0, SEEK_END) != 0)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("fseek failed\n");
        return MT_FAILURE;
    }

    size = ftell(fp);

    *data = (MT_U8*)malloc(size);
    src = *data;
    if(fseek(fp, 0, SEEK_SET) != 0)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("fseek failed\n");
        return MT_FAILURE;
    }

    if(size <= 1024)
    {
        readlen = fread(src, sizeof(MT_U8), size, fp);
    }
    else
    {
        for(i = 0; i < size / 1024; i++)
        {
            readlen = fread(src, sizeof(MT_U8), 1024, fp);
            if(readlen <= 0)
            {
                break;
            }
            src += 1024;
        }
        fread(src, sizeof(MT_U8), size % 1024, fp);
    }

    src = NULL;
    *file_size = size;
    fclose(fp);
    return 0;
}


static MT_VOID MT_R2RGetKey(MT_CIPHER_ALGORITHM_E algorithm, MT_U8 *key)
{
    if(MT_CIPHER_ALG_DES == algorithm)
    {
        memcpy(key, g_DESkey, 8);
    }
    else if(MT_CIPHER_ALG_TDES == algorithm)
    {
        memcpy(key, g_TDESkey, 16);
    }
    else if(MT_CIPHER_ALG_AES == algorithm)
    {
        memcpy(key, g_AESkey, 16);
    }
}



static MT_S32 MT_R2REncrypt(MT_CIPHER_WORK_MODE_E work_mode, MT_CIPHER_ALGORITHM_E algorithm, MT_S32 file_size, MT_U8 *in_data, MT_U8 *out_data)
{
    MT_U8            *r2r_out = NULL;
    MT_U8            *key = NULL;
    MT_U8            *r2r_in = NULL;
    MT_S32           ret = MT_FAILURE;
    MT_U32           request_slot = 0;
    MT_CIPHER_CTRL_S crypto_ctrl = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_u32           crypto_handle = 0;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    ulong           crypto_handle = 0;
#endif

    key = mt_unf_cipher_malloc(MAX_KEY_SIZE);
    memset(key, 0, MAX_KEY_SIZE);
    MT_R2RGetKey(algorithm, key);

    r2r_out = mt_unf_cipher_malloc(file_size);
    memset(r2r_out, 0, file_size);

    r2r_in = mt_unf_cipher_malloc(file_size);
    memset(r2r_in, 0, file_size);
    memcpy(r2r_in, in_data, file_size);

    ret = mt_unf_cipher_keyslot_request((unsigned int *)&request_slot);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_keyslot_request] error ret [%#x]\n", ret);
        goto ERR0;
    }

    crypto_ctrl.operation = MT_CIPHER_OPERATION_ENCRYPT;
    crypto_ctrl.algorithm = algorithm;
    crypto_ctrl.work_mode = work_mode;
    crypto_ctrl.core = MT_CIPHER_CORE_M2M_RAW;

    if(MT_CIPHER_WORK_MODE_ECB == work_mode)
    {
        ret = mt_unf_cipher_keyslot_set(request_slot, &crypto_ctrl, key, NULL);
        if(SUCCESS != ret)
        {
            SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_keyslot_set] error ret [%#x]\n", ret);
            goto ERR1;
        }
    }
    else
    {
        ret = mt_unf_cipher_keyslot_set(request_slot, &crypto_ctrl, key, g_IV);
        if(SUCCESS != ret)
        {
            SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_keyslot_set] error ret [%#x]\n", ret);
            goto ERR1;
        }
    }


    ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_1, &crypto_handle);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_crypto_create] error ret [%#x]\n", ret);
        goto ERR1;
    }

    ret = mt_unf_cipher_crypto_config(crypto_handle, &crypto_ctrl, request_slot);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_crypto_config] error ret [%#x]\n", ret);
        goto ERR2;
    }

    ret = mt_unf_cipher_crypto_process(crypto_handle, r2r_in, r2r_out, file_size);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_crypto_process] error ret [%#x]\n", ret);
        goto ERR2;
    }

    memcpy(out_data, r2r_out, file_size);

    if(file_size > DATA_MEM_SIZE)
    {
        dump_data("r2r_in:", r2r_in, DATA_MEM_SIZE);
        dump_data("r2r_encrypt:", r2r_out, DATA_MEM_SIZE);
    }
    else
    {
        dump_data("r2r_in:", r2r_in, file_size);
        dump_data("r2r_encrypt:", r2r_out, file_size);
    }

ERR2:
    (MT_VOID)mt_unf_cipher_crypto_destroy(crypto_handle);
ERR1:
    (MT_VOID)mt_unf_cipher_keyslot_release(request_slot);
ERR0:
    mt_unf_cipher_free(r2r_in);
    mt_unf_cipher_free(key);
    mt_unf_cipher_free(r2r_out);

    return MT_SUCCESS;
}


static MT_S32 MT_R2RDecrypt(MT_CIPHER_WORK_MODE_E work_mode, MT_CIPHER_ALGORITHM_E algorithm, MT_S32 file_size, MT_U8 *in_data)
{
    MT_U8            *r2r_out = NULL;
    MT_U8            *key = NULL;
    MT_U8            *r2r_in = NULL;
    MT_S32           ret = MT_FAILURE;
    MT_U32           request_slot = 0;
    MT_CIPHER_CTRL_S crypto_ctrl = { 0 };
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    mt_u32           crypto_handle = 0;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    ulong           crypto_handle = 0;
#endif

    key = mt_unf_cipher_malloc(MAX_KEY_SIZE);
    memset(key, 0, MAX_KEY_SIZE);
    MT_R2RGetKey(algorithm, key);

    r2r_out = mt_unf_cipher_malloc(file_size);
    memset(r2r_out, 0, file_size);

    r2r_in = mt_unf_cipher_malloc(file_size);
    memset(r2r_in, 0, file_size);
    memcpy(r2r_in, in_data, file_size);

    ret = mt_unf_cipher_keyslot_request((unsigned int *)&request_slot);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_keyslot_request] error ret [%#x]\n", ret);
        goto ERR0;
    }

    crypto_ctrl.operation = MT_CIPHER_OPERATION_DECRYPT;
    crypto_ctrl.algorithm = algorithm;
    crypto_ctrl.work_mode = work_mode;
    crypto_ctrl.core = MT_CIPHER_CORE_M2M_RAW;

    if(MT_CIPHER_WORK_MODE_ECB == work_mode)
    {
        ret = mt_unf_cipher_keyslot_set(request_slot, &crypto_ctrl, key, NULL);
        if(SUCCESS != ret)
        {
            SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_keyslot_set] error ret [%#x]\n", ret);
            goto ERR1;
        }
    }
    else
    {
        ret = mt_unf_cipher_keyslot_set(request_slot, &crypto_ctrl, key, g_IV);
        if(SUCCESS != ret)
        {
            SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_keyslot_set] error ret [%#x]\n", ret);
            goto ERR1;
        }
    }


    ret = mt_unf_cipher_crypto_create(MT_CIPHER_CRYPTO_CH_0, &crypto_handle);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_crypto_create] error ret [%#x]\n", ret);
        goto ERR1;
    }

    ret = mt_unf_cipher_crypto_config(crypto_handle, &crypto_ctrl, request_slot);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_crypto_config] error ret [%#x]\n", ret);
        goto ERR2;
    }

    ret = mt_unf_cipher_crypto_process(crypto_handle, r2r_in, r2r_out, file_size);
    if(SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("[mt_unf_cipher_crypto_process] error ret [%#x]\n", ret);
        goto ERR2;
    }

    if(file_size > DATA_MEM_SIZE)
    {
        dump_data("r2r_decrypt:", r2r_out, DATA_MEM_SIZE);
    }
    else
    {
        dump_data("r2r_decrypt:", r2r_out, file_size);
    }

ERR2:
    (MT_VOID)mt_unf_cipher_crypto_destroy(crypto_handle);
ERR1:
    (MT_VOID)mt_unf_cipher_keyslot_release(request_slot);
ERR0:
    mt_unf_cipher_free(r2r_in);
    mt_unf_cipher_free(key);
    mt_unf_cipher_free(r2r_out);

    return MT_SUCCESS;
}


static MT_VOID MT_R2RPrintMenu(MT_VOID)
{
    SAMPLE_CIPHER_R2R_PRINT("\ncommond: \n");
    SAMPLE_CIPHER_R2R_PRINT("     w : DES-ECB\n");
    SAMPLE_CIPHER_R2R_PRINT("     e : DES-CBC\n");
    SAMPLE_CIPHER_R2R_PRINT("     a : TDES-ECB\n");
    SAMPLE_CIPHER_R2R_PRINT("     d : TDES-CBC\n");
    SAMPLE_CIPHER_R2R_PRINT("     t : AES-ECB\n");
    SAMPLE_CIPHER_R2R_PRINT("     y : AES-CBC\n");
    SAMPLE_CIPHER_R2R_PRINT("     u : AES-CTR\n");
    SAMPLE_CIPHER_R2R_PRINT("     h : help \n");
    SAMPLE_CIPHER_R2R_PRINT("     q : quit \n");
    SAMPLE_CIPHER_R2R_PRINT("R2R>> ");
}


static MT_VOID MT_R2RCmdTask(MT_CHAR filename[256], MT_U8 *data_in, MT_S32 file_size)
{
    MT_CHAR inputCmd[32] = { 0 };
    MT_U8   *r2r_decrypt = MT_NULL;

    r2r_decrypt = (MT_U8*)malloc(file_size);
    memset(r2r_decrypt, 0, file_size);

    while(1)
    {
        (MT_VOID)MT_R2RPrintMenu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            break;
        }
        else if('w' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("DES-ECB mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_ECB, MT_CIPHER_ALG_DES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_ECB, MT_CIPHER_ALG_DES, file_size, r2r_decrypt);
        }
        else if('e' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("DES-CBC mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_CBC, MT_CIPHER_ALG_DES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_CBC, MT_CIPHER_ALG_DES, file_size, r2r_decrypt);
        }
        else if('a' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("TDES-ECB mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_ECB, MT_CIPHER_ALG_TDES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_ECB, MT_CIPHER_ALG_TDES, file_size, r2r_decrypt);
        }
        else if('d' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("TDES-CBC mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_CBC, MT_CIPHER_ALG_TDES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_CBC, MT_CIPHER_ALG_TDES, file_size, r2r_decrypt);
        }
        else if('t' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("AES-ECB mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_ECB, MT_CIPHER_ALG_AES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_ECB, MT_CIPHER_ALG_AES, file_size, r2r_decrypt);
        }
        else if('y' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("AES-CBC mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_CBC, MT_CIPHER_ALG_AES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_CBC, MT_CIPHER_ALG_AES, file_size, r2r_decrypt);
        }
        else if('u' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("AES-CTR mode\n");
            (MT_VOID)MT_R2REncrypt(MT_CIPHER_WORK_MODE_CTR, MT_CIPHER_ALG_AES, file_size, data_in, r2r_decrypt);
            (MT_VOID)MT_R2RDecrypt(MT_CIPHER_WORK_MODE_CTR, MT_CIPHER_ALG_AES, file_size, r2r_decrypt);
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_CIPHER_R2R_INFO_PRINT("Print help info\n");
        }
    }

    free(r2r_decrypt);
}

static MT_VOID MT_R2RPrint_Help(MT_CHAR *name)
{
    SAMPLE_CIPHER_R2R_PRINT("Lack of parameters\n");
    SAMPLE_CIPHER_R2R_PRINT("\nUsage:\n");
    SAMPLE_CIPHER_R2R_PRINT("%s\n", name);
    SAMPLE_CIPHER_R2R_PRINT("    -f: File path\n");
    SAMPLE_CIPHER_R2R_PRINT("example:\n");
    SAMPLE_CIPHER_R2R_PRINT("    %s -f ./encrypt.txt\n", name);
}

static MT_S32 MT_R2RParase_args(MT_S32 argc, MT_CHAR *argv[], MT_CHAR *filename)
{
    MT_S32 opt = 0;

    if(argc != 3)
    {
        (MT_VOID)MT_R2RPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_R2RPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'f':
                strcpy(filename, argv[2]);
                break;
            default:
                (MT_VOID)MT_R2RPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_R2RMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32  ret = MT_FAILURE;
    MT_S32  file_size = 0;
    MT_CHAR filename[256] = { 0 };
    MT_U8   *r2r_in = NULL;

    ret = MT_R2RParase_args(argc, argv, filename);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }
#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_CIPHER_R2R_ERR_PRINT("failed to mt_sys_init\n");
        return ret;
    }
#endif
    ret = MT_R2RFileRead(filename, &r2r_in, &file_size);
    if(MT_SUCCESS != ret)
    {
        goto ERR0;
    }

    (MT_VOID)MT_R2RCmdTask(filename, r2r_in, file_size);
    free(r2r_in);

ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return ret;
}

