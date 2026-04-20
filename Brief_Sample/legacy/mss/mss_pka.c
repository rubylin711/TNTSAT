/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include "mt_type.h"
#include "mt_common.h"
#include "mt_unf_cipher_v2.h"
#include "mss_cmd_utils.h"
#include "mss_tee_client.h"
#include "mss_pka.h"

struct pka_mod_data
{
	unsigned int a_bits;
	unsigned char a[256];
	unsigned int b_bits;
	unsigned char b[256];
	unsigned int m_bits;
	unsigned char m[256];

	unsigned char mod[256]; // mod =a mod m
	unsigned char modinverse[256]; // modinverse =b^-1 mod m
	unsigned char modadd[256]; // modadd = (a+b) mod m
	unsigned char modsub[256]; // modsub = (a-b) mod m
	unsigned char modmul[256]; // modmul = (a*b) mod m
}__attribute__((aligned(4)));
#include "pka_mod_testcase.h"

static int mss_pka_mod(struct pka_mod_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_handle;
    mt_u8 output[256];

    mss_dump("Op1", testcase->a, testcase->a_bits>>3);
    mss_dump("Op2", testcase->b, testcase->b_bits>>3);
    mss_dump("M", testcase->m, testcase->m_bits>>3);

    ret = mt_unf_cipher_bn_create(&p_handle);

    memset(output, 0, 256);
    ret |= mt_unf_cipher_bn_mod_mod(p_handle, output, testcase->a, testcase->m,
                    testcase->a_bits>>3, testcase->m_bits>>3);

    mss_dump("Mod Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->mod, testcase->m_bits>>3) == 0) {
      printf("PKA MOD Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MOD Failed!\n");
      ret |= MT_FAILURE;
    }

    memset(output, 0, 256);
    ret |= mt_unf_cipher_bn_mod_mul(p_handle, output, testcase->a, testcase->b, testcase->m,
                    testcase->a_bits>>3, testcase->b_bits>>3, testcase->m_bits>>3);

    mss_dump("Mod Mul Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->modmul, testcase->m_bits>>3) == 0) {
      printf("PKA MODMUL Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MODMUL Failed!\n");
      ret |= MT_FAILURE;
    }

    memset(output, 0, 256);
    ret |= mt_unf_cipher_bn_mod_add(p_handle, output, testcase->a, testcase->b, testcase->m,
                    testcase->a_bits>>3, testcase->b_bits>>3, testcase->m_bits>>3);

    mss_dump("Mod Add Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->modadd, testcase->m_bits>>3) == 0) {
      printf("PKA MODADD Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MODADD Failed!\n");
      ret |= MT_FAILURE;
    }

    memset(output, 0, 256);
    ret |= mt_unf_cipher_bn_mod_sub(p_handle, output, testcase->a, testcase->b, testcase->m,
                    testcase->a_bits>>3, testcase->b_bits>>3, testcase->m_bits>>3);

    mss_dump("Mod Sub Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->modsub, testcase->m_bits>>3) == 0) {
      printf("PKA MODSUB Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MODSUB Failed!\n");
      ret |= MT_FAILURE;
    }

    if (testcase->m_bits < 2048) {
        memset(output, 0, 256);
        ret |= mt_unf_cipher_bn_mod_inv(p_handle, output, testcase->b, testcase->m,
                        testcase->b_bits>>3, testcase->m_bits>>3);

        mss_dump("Mod Inv Output", output, testcase->m_bits>>3);
        if(memcmp(output, testcase->modinverse, testcase->m_bits>>3) == 0) {
          printf("PKA MODINV Success!\n");
          ret |= MT_SUCCESS;
        }
        else {
          printf("PKA MODINV Failed!\n");
          ret |= MT_FAILURE;
        }
    }

    ret |= mt_unf_cipher_bn_destroy(p_handle);

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_mod_tee(struct pka_mod_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 output[256];

    mss_dump("Op1", testcase->a, testcase->a_bits>>3);
    mss_dump("Op1", testcase->b, testcase->b_bits>>3);
    mss_dump("M", testcase->m, testcase->m_bits>>3);

    memset(output, 0, 256);
    ret |= tee_mod_mod(testcase->a, testcase->a_bits>>3, 
                testcase->m, testcase->m_bits>>3, output, testcase->m_bits>>3);

    mss_dump("Mod Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->mod, testcase->m_bits>>3) == 0) {
      printf("PKA MOD Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MOD Failed!\n");
      ret |= MT_FAILURE;
    }

    memset(output, 0, 256);
    ret |= tee_mod_mul(testcase->a, testcase->a_bits>>3, testcase->b, testcase->b_bits>>3, 
                    testcase->m, testcase->m_bits>>3, output, testcase->m_bits>>3);

    mss_dump("Mod Mul Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->modmul, testcase->m_bits>>3) == 0) {
      printf("PKA MODMUL Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MODMUL Failed!\n");
      ret |= MT_FAILURE;
    }

    memset(output, 0, 256);
    ret |= tee_mod_add(testcase->a, testcase->a_bits>>3, testcase->b, testcase->b_bits>>3, 
                    testcase->m, testcase->m_bits>>3, output, testcase->m_bits>>3);

    mss_dump("Mod Add Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->modadd, testcase->m_bits>>3) == 0) {
      printf("PKA MODADD Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MODADD Failed!\n");
      ret |= MT_FAILURE;
    }

    memset(output, 0, 256);
    ret |= tee_mod_sub(testcase->a, testcase->a_bits>>3, testcase->b, testcase->b_bits>>3, 
                    testcase->m, testcase->m_bits>>3, output, testcase->m_bits>>3);

    mss_dump("Mod Sub Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->modsub, testcase->m_bits>>3) == 0) {
      printf("PKA MODSUB Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA MODSUB Failed!\n");
      ret |= MT_FAILURE;
    }

    if (testcase->m_bits < 2048) {
        memset(output, 0, 256);
        ret |= tee_mod_inv(testcase->b, testcase->b_bits>>3,
                         testcase->m, testcase->m_bits>>3, output, testcase->m_bits>>3);

        mss_dump("Mod Inv Output", output, testcase->m_bits>>3);
        if(memcmp(output, testcase->modinverse, testcase->m_bits>>3) == 0) {
          printf("PKA MODINV Success!\n");
          ret |= MT_SUCCESS;
        }
        else {
          printf("PKA MODINV Failed!\n");
          ret |= MT_FAILURE;
        }
    }

    return ret;
}
#else
static int mss_pka_mod_tee(struct pka_mod_data *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static mt_s32 mss_pka_mod_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_mod_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
    }

    for (i = 0; i < ARRAY_SIZE(pka_mod_testcase); i++) {
        testcase =  &pka_mod_testcase[i];
        if (cpu_mode == MSS_SECCPU)
            ret |= mss_pka_mod_tee(testcase);
        else
            ret |= mss_pka_mod(testcase);
        if (ret != MT_SUCCESS)
            break;
    }

    return ret;
}

struct pka_mod_exp_data
{
	unsigned int a_bits;
	unsigned char a[256];
	unsigned int e_bits;
	unsigned char e[256];
	unsigned int m_bits;
	unsigned char m[256];

	unsigned char exp[256]; // exp = (a^e) mod m
}__attribute__((aligned(4)));
#include "pka_mod_exp_testcase.h"

static int mss_pka_mod_exp(struct pka_mod_exp_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_handle;
    mt_u8 output[256];

    mss_dump("a:", testcase->a, testcase->a_bits>>3);
    mss_dump("e:", testcase->e, testcase->e_bits>>3);
    mss_dump("m:", testcase->m, testcase->m_bits>>3);

    ret = mt_unf_cipher_bn_create(&p_handle);

    memset(output, 0, 256);
    ret |= mt_unf_cipher_bn_mod_exp(p_handle, output, testcase->a, testcase->e, testcase->m,
                    testcase->a_bits>>3, testcase->e_bits>>3, testcase->m_bits>>3);

    mss_dump("Mod Exp Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->exp, testcase->m_bits>>3) == 0) {
      printf("PKA EXP Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA EXP Failed!\n");
      ret |= MT_FAILURE;
    }

    ret |= mt_unf_cipher_bn_destroy(p_handle);

    return ret;
}

#ifdef CONFIG_MT_TEE_SUPPORT
static int mss_pka_mod_exp_tee(struct pka_mod_exp_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u8 output[256];

    mss_dump("a:", testcase->a, testcase->a_bits>>3);
    mss_dump("e:", testcase->e, testcase->e_bits>>3);
    mss_dump("m:", testcase->m, testcase->m_bits>>3);

    memset(output, 0, 256);
    ret |= tee_mod_exp(testcase->a, testcase->a_bits>>3, testcase->e, testcase->e_bits>>3,
                    testcase->m, testcase->m_bits>>3, output, testcase->m_bits>>3);

    mss_dump("Mod Exp Output", output, testcase->m_bits>>3);
    if(memcmp(output, testcase->exp, testcase->m_bits>>3) == 0) {
      printf("PKA EXP Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA EXP Failed!\n");
      ret |= MT_FAILURE;
    }

    return ret;
}
#else
static int mss_pka_mod_exp_tee(struct pka_mod_exp_data *testcase)
{
    printf("not support tee\n");
    return MT_FAILURE;
}
#endif

static int mss_pka_mod_exp_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_mod_exp_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
    }

    for (i = 0; i < ARRAY_SIZE(pka_mod_exp_testcase); i++) {
        testcase =  &pka_mod_exp_testcase[i];
        if (cpu_mode == MSS_SECCPU)
            ret |= mss_pka_mod_exp_tee(testcase);
        else
            ret |= mss_pka_mod_exp(testcase);
        if (ret != MT_SUCCESS)
            break;
    }

    return ret;
}

struct pka_point_mul_data
{
	char curve_name[32];
	unsigned int bits;
	unsigned char p[64];
	unsigned char a[64];
	unsigned char b[64];
	unsigned char gx[64];
	unsigned char gy[64];
	unsigned char n[64];
	unsigned char k1[64];
	unsigned char x1[64];  // (x1, y1) = k1 * (gx, gy)
	unsigned char y1[64];
	unsigned char k2[64];
	unsigned char x2[64];  // (x2, y2) = k2 * (x1, y1)
	unsigned char y2[64];
}__attribute__((aligned(4)));
#include "pka_point_mul_testcase.h"

static int mss_pka_ecp_mul(struct pka_point_mul_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_handle;
    MT_CIPHER_EC_PARAMS_S xParams;
    MT_CIPHER_EC_POINT_S point;
    const mt_u8 *g_scalar = NULL;
    const mt_u8 *p_scalar = NULL;
    mt_u8 rx[64];
    mt_u8 ry[64];
    MT_CIPHER_EC_POINT_S r = {rx, ry};

    mss_dump("p:", testcase->p, testcase->bits>>3);
    mss_dump("a:", testcase->a, testcase->bits>>3);

    xParams.q = testcase->p;
    xParams.a = testcase->a;
    xParams.b = testcase->b;
    xParams.GX = testcase->gx;
    xParams.GY = testcase->gy;
    xParams.keySize = testcase->bits>>3;

    g_scalar = testcase->k1;
    p_scalar = testcase->k2;

    point.X = testcase->x1;
    point.Y = testcase->y1;

    ret = mt_unf_cipher_ecp_create(&p_handle);

    printf("Ecp Mul g_scalar*(GX,GY)\n");
    ret |= mt_unf_cipher_ecp_mul(p_handle, xParams, &r, g_scalar, NULL, NULL);

    mss_dump("Output X:", r.X, testcase->bits>>3);
    mss_dump("Output Y:", r.Y, testcase->bits>>3);

    if(memcmp(r.X, testcase->x1, testcase->bits>>3) == 0
        && memcmp(r.Y, testcase->y1, testcase->bits>>3) == 0) {
      printf("PKA ECP GPOINT MUL Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA ECP GPOINT MUL Failed!\n");
      ret |= MT_FAILURE;
    }

    printf("Ecp Mul p_scalar*(X,Y)\n");
    ret |= mt_unf_cipher_ecp_mul(p_handle, xParams, &r, NULL, &point, p_scalar);

    mss_dump("Output X:", r.X, testcase->bits>>3);
    mss_dump("Output Y:", r.Y, testcase->bits>>3);

    if(memcmp(r.X, testcase->x2, testcase->bits>>3) == 0
        && memcmp(r.Y, testcase->y2, testcase->bits>>3) == 0) {
      printf("PKA ECP POINT MUL Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA ECP POINT MUL Failed!\n");
      ret |= MT_FAILURE;
    }

    ret |= mt_unf_cipher_ecp_destroy(p_handle);

    return ret;
}

static int mss_pka_ecp_mul_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_point_mul_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode __attribute__((unused)) = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
    }

    for (i = 0; i < ARRAY_SIZE(pka_point_mul_testcase); i++) {
        testcase =  &pka_point_mul_testcase[i];
        ret = mss_pka_ecp_mul(testcase);
        if (ret != MT_SUCCESS)
            break;
    }

    return ret;
}

struct pka_point_add_data
{
	char curve_name[32];
	unsigned int bits;
	unsigned char p[64];
	unsigned char a[64];
	unsigned char b[64];
	unsigned char x1[64];
	unsigned char y1[64];
	unsigned char x2[64];
	unsigned char y2[64];
	unsigned char addx[64]; // (addx, addy) = (x1, y1) + (x2, y2)
	unsigned char addy[64];
}__attribute__((aligned(4)));
#include "pka_point_add_testcase.h"

static int mss_pka_ecp_add(struct pka_point_add_data *testcase)
{
    mt_s32 ret = MT_SUCCESS;
    mt_handle p_handle;
    MT_CIPHER_EC_PARAMS_S xParams;
    MT_CIPHER_EC_POINT_S a;
    MT_CIPHER_EC_POINT_S b;
    mt_u8 rx[64];
    mt_u8 ry[64];
    MT_CIPHER_EC_POINT_S r = {rx, ry};

    printf("Ecp Add curve:%s\n", testcase->curve_name);

    mss_dump("p:", testcase->p, testcase->bits>>3);
    mss_dump("a:", testcase->a, testcase->bits>>3);

    xParams.q = testcase->p;
    xParams.a = testcase->a;
    xParams.b = testcase->b;
    xParams.keySize = testcase->bits>>3;

    a.X = testcase->x1;
    a.Y = testcase->y1;

    b.X = testcase->x2;
    b.Y = testcase->y2;

    ret = mt_unf_cipher_ecp_create(&p_handle);

    ret |= mt_unf_cipher_ecp_add(p_handle, xParams, &r, &a, &b);

    mss_dump("Output X:", r.X, testcase->bits>>3);
    mss_dump("Output Y:", r.Y, testcase->bits>>3);

    if(memcmp(r.X, testcase->addx, testcase->bits>>3) == 0
        && memcmp(r.Y, testcase->addy, testcase->bits>>3) == 0) {
      printf("PKA ECP ADD Success!\n");
      ret |= MT_SUCCESS;
    }
    else {
      printf("PKA ECP ADD Failed!\n");
      ret |= MT_FAILURE;
    }

    ret |= mt_unf_cipher_ecp_destroy(p_handle);

    return ret;
}

static int mss_pka_ecp_add_test(int argc, char *const argv[])
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    struct pka_point_add_data *testcase = NULL;
    MSS_CPU_MODE_E cpu_mode __attribute__((unused)) = MSS_APCPU;

    if (argc > 0 && strcmp(argv[0], "-t") == 0) {
        cpu_mode = MSS_SECCPU;
    }

    for (i = 0; i < ARRAY_SIZE(pka_point_add_testcase); i++) {
        testcase =  &pka_point_add_testcase[i];
        ret = mss_pka_ecp_add(testcase);
        if (ret != MT_SUCCESS)
            break;
    }

    return ret;
}

static void mss_pka_cmd_help(void)
{
    mss_print("Usage:\n");

    mss_print("\npka mod [-t]\n");
    mss_print("\t pka mod test: mod, modmul, modadd, modsub, modinv\n");

    mss_print("\npka exp [-t]\n");
    mss_print("\t pka exponent test\n");

    mss_print("\npka ecpmul [-t]\n");
    mss_print("\t pka ecp mul test\n");

    mss_print("\npka ecpadd [-t]\n");
    mss_print("\t pka ecp add test\n");

    mss_print("\npka rsa [-t]\n");
    mss_print("\t pka rsa public/private key encrypt/decrypt & signature/verify test\n");
    mss_print("pka rsa encdec [-t] [oaep_sha256|pkcs1v15]\n");
    mss_print("\t pka rsa public/private key encrypt/decrypt test\n");
    mss_print("pka rsa sign [-t] [pkcs1v15_sha1|pss_sha256]\n");
    mss_print("\t pka rsa signature/verify test\n");

    mss_print("\npka ecdh [-t]\n");
    mss_print("\t pka ecdh gen key test\n");

    mss_print("\npka ecdsa [-t]\n");
    mss_print("\t pka ecdsa signature/verify test\n");

    mss_print("\npka sm2 sign [-t]\n");
    mss_print("\t pka sm2 signature/verify test\n");
    mss_print("\npka sm2 encdec [-t]\n");
    mss_print("\t pka sm2 encrypt/decrypt test\n");
}

mt_s32 mss_pka_test(int argc, char *const argv[])
{
    int ret = -1;
    mt_u32 i;
    char *cmd;

    if (0 == strncmp("-h", argv[0], 2)){
        mss_pka_cmd_help();
        return 0;
    }

    for (i = 0; i < argc; i++)
        printf("%s [%d]:%s\n", __FUNCTION__, i, argv[i]);

    cmd = argv[0];
    if (0 == strncmp("mod", cmd, 3)){
        ret = mss_pka_mod_test(argc - 1, argv + 1);
    } else if (0 == strncmp("exp", cmd, 3)){
        ret = mss_pka_mod_exp_test(argc - 1, argv + 1);
    } else if (0 == strncmp("ecpmul", cmd, 6)){
        ret = mss_pka_ecp_mul_test(argc - 1, argv + 1);
    } else if (0 == strncmp("ecpadd", cmd, 6)){
        ret = mss_pka_ecp_add_test(argc - 1, argv + 1);
    } else if (0 == strncmp("rsa", cmd, 3)){
        ret = mss_pka_rsa_test(argc - 1, argv + 1);
    } else if (0 == strncmp("sm2", cmd, 3)){
        ret = mss_pka_sm2_test(argc - 1, argv + 1);
    } else if (0 == strncmp("ecdh", cmd, 4)){
        ret = mss_pka_ecdh_test(argc - 1, argv + 1);
    } else if (0 == strncmp("ecdsa", cmd, 5)){
        ret = mss_pka_ecdsa_test(argc - 1, argv + 1);
    }

    return ret;
}

