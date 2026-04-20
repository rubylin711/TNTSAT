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
#include "mt_unf_misc.h"
#include "mt_unf_cipher_v2.h"
#include "mss_cmd_utils.h"
#include "mss_pka.h"

#if 0
void print_time(const char *func_name, const char *tag, mss_clock_t time)
{
    mss_print("-------------------\n");
    mss_print("%s::%s --> ", func_name, tag);
    mss_print("[%f]s\n", ((double)time / CLOCKS_PER_SEC));
    mss_print("-------------------\n");
}
#endif

static void cmd_mss_help(void)
{
    mss_print("Usage:\n");
    mss_print("kt -h\n");
    mss_print("hash -h\n");
    mss_print("hmac -h\n");
    mss_print("crypto -h\n");
    mss_print("kl -h\n");
    mss_print("pka -h\n");
}

#if 0
/* virutal address to physical address, no check */
static mt_u8 *vir2phy(mt_u8 *vir)
{
    phys_addr_t phy_addr;
    ulong phy_size;

    mt_mmz_get_phyaddr((mt_void *)vir, &phy_addr, &phy_size);

    return (mt_u8 *)phy_addr;
}
#endif

mt_s32 mss_kt_test(int argc, char *const argv[]);
mt_s32 mss_hash_test(int argc, char *const argv[]);
mt_s32 mss_hmac_test(int argc, char *const argv[]);
mt_s32 mss_crypto_test(int argc, char *const argv[]);
mt_s32 mss_kl_test(int argc, char *const argv[]);

int tee_my_test(void);

static int mss_test_start(char *cmd_name, int argc, char *const argv[])
{
    int ret = 0;
    int i = 0;

    mss_print("\n[CMD]: %s ", cmd_name);
    for (i = 0; i < argc; i++)
        mss_print("%s ", argv[i]);
    mss_print("\n");

    if (0 == strncmp("kt", cmd_name, 2)) {
        ret = mss_kt_test(argc, argv);
    } else if (0 == strncmp("hash", cmd_name, 4)) {
        ret = mss_hash_test(argc, argv);
    } else if (0 == strncmp("hmac", cmd_name, 4)) {
        ret = mss_hmac_test(argc, argv);
    } else if (0 == strncmp("crypto", cmd_name, 6)) {
        ret = mss_crypto_test(argc, argv);
    } else if (0 == strncmp("kl", cmd_name, 2)) {
        ret = mss_kl_test(argc, argv);
    } else if (0 == strncmp("pka", cmd_name, 3)) {
        ret = mss_pka_test(argc, argv);
    }

    return ret;
}

static int cmd_mss(char *subcmd)
{
    int ret = 0;
    char *token;
    char delimiter[] = " \t\n";   /* space or tab or \n */
    char cmd_name[10] = {0};
    char *cmd_args[10] = {NULL};
    int argc;
    char ** argv;
    int i = 0;

    token = strtok(subcmd, delimiter);
    //mss_print("Got cmd_name:%s, strlen=%d\n", token, strlen(token));
    if (token != NULL) {
        strcpy(cmd_name, token);
    } else {
        mss_print("Error:Unknown sub-command\n");
        return -1;
    }

    argc = 0;
    while (token != NULL) {
        token = strtok(NULL, delimiter);
        if (token != NULL) {
            if (argc >= 10) {
                mss_print("mss cmd too long\n");
                goto EXIT;
            }
            cmd_args[argc] = (char *)malloc(strlen(token)+1);
            strcpy(cmd_args[argc], token);
            argc++;
        } else {
            //mss_print("null token, no more parsing, i=%d\n", i);
        }
    }

    argv = cmd_args;

    ret = mss_test_start(cmd_name, argc, argv);

EXIT:
    for (i = 0; i < 10; i++) {
        if (cmd_args[i] != NULL) {
            free(cmd_args[i]);
            cmd_args[i] = NULL;
        }
    }

    return ret;
}
    
int main(int argc,char ** argv)
{
    char *in_str;
    size_t nbytes = 50;
    ssize_t ret = -1;

    //sys init first
    mt_sys_init();
    ret = mt_unf_misc_init();
    ret = mt_unf_misc_module_set(HAL_KT, 1);
    ret = mt_unf_misc_module_set(HAL_CRYPTO, 1);
    ret = mt_unf_misc_module_set(HAL_CRYPTO_DES, 1);
    ret = mt_unf_misc_module_set(HAL_CRYPTO_TDES, 1);
    ret = mt_unf_misc_module_set(HAL_CRYPTO_AES, 1);
    ret = mt_unf_misc_module_set(HAL_CRYPTO_SHA, 1);
    ret = mt_unf_misc_module_set(HAL_CRYPTO_RSA, 1);
    ret = mt_unf_misc_module_set(HAL_KL_CW, 1);
    ret = mt_unf_misc_module_set(HAL_RNG, 1);
    ret = mt_unf_misc_module_set(HAL_RNG2, 1);

    ret = mt_unf_cipher_init();

    cmd_mss_help();

    in_str = (char *)malloc(nbytes + 1);

    mss_print("Please input sub-command...\n");
    mss_print(">>> ");
    while (1) {
        ret = getline(&in_str, &nbytes, stdin);
        if (ret > 1) {
            if (strncmp(in_str, "q", ret - 1) == 0) {
                mss_print("Bye-bye!\n");
                break;
            } else if (strncmp(in_str, "h", ret - 1) == 0) {
                cmd_mss_help();
            } else {
                if (cmd_mss(in_str)) {
                    mss_print("Mss command failed!\n");
                    cmd_mss_help();
                }
            }
        } else if (ret == -1) {
            mss_print("Term\n");
            break;
        }

        mss_print(">>> ");
    }

    mss_print("cmd_mss exit.\n");

    free(in_str);

    mt_unf_cipher_deinit();
    mt_sys_deinit();

    return 0;
}
