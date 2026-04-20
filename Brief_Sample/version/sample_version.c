/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_VERSION_DEBUG

#define MT_VERSION_PRINT   printf
#else

#define MT_VERSION_PRINT

#endif

#define SAMPLE_VERSION_FUNCTION_ENTER()     MT_VERSION_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_VERSION_FUNCTION_EXIT()      MT_VERSION_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_VERSION_FATAL_PRINT(fmt...)          MT_VERSION_PRINT(" [FATAL] " fmt)
#define SAMPLE_VERSION_ERR_PRINT(fmt...)            MT_VERSION_PRINT(" [ERROR] " fmt)
#define SAMPLE_VERSION_WARN_PRINT(fmt...)           MT_VERSION_PRINT(" [WARN] "  fmt)
#define SAMPLE_VERSION_INFO_PRINT(fmt...)           MT_VERSION_PRINT(" [INFO] "  fmt)
#define SAMPLE_VERSION_DBG_PRINT(fmt...)            MT_VERSION_PRINT(" [DEBUG] " fmt)


#define  SAMPLE_VERSION_PRINT  printf
/*************************** Structure Definition ****************************/

/********************** Global Variable declaration **************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_VersionMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_S32 MT_VersionRead(MT_CHAR fileProlist[256], MT_CHAR fileCommit[256])
{
    FILE *fp1;
    FILE *fp2;
    MT_S32 len = 0;
    MT_CHAR prolist[256];
    MT_CHAR commitId[256];
    MT_CHAR output[512];

    if((fp1 = fopen(fileProlist, "r")) == NULL)
    {
        SAMPLE_VERSION_ERR_PRINT("Can not open file %s!\n", fileProlist);
        return MT_FAILURE;
    }
    if((fp2 = fopen(fileCommit, "r")) == NULL)
    {
        SAMPLE_VERSION_ERR_PRINT("Can not open file %s!\n", fileCommit);
        fclose(fp1);
        return MT_FAILURE;
    }

    while(fgets(prolist, sizeof(prolist), fp1) != NULL && fgets(commitId, sizeof(commitId), fp2) != NULL)
    {
        len = strlen(prolist);
        if(len > 0 && prolist[len - 1] == '\n')
        {
            prolist[len - 1] = '\0';
        }

        len = strlen(commitId);
        if(len > 0 && commitId[len - 1] == '\n')
        {
            commitId[len - 1] = '\0';
        }

        sprintf(output, "%-25s%s", prolist, commitId);
        printf("%s\n", output);
    }

    fclose(fp1);
    fclose(fp2);
    fp1 = NULL;
    fp2 = NULL;

    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_VersionMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = 0;

#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if (MT_SUCCESS != ret)
    {
        SAMPLE_VERSION_ERR_PRINT("MT_UNF_IR_Init ret = %d\n", ret);
        return ret;
    }
#endif

    MT_VersionRead("/usr/local/stb/res/prolist.info", "/usr/local/stb/res/commit.info");

#ifndef MT_SAMPLE_APP
    /** system deinitialized */
    (MT_VOID)mt_sys_deinit();
#endif

    return ret;


}

