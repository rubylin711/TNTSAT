/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include "signal.h"
#include <sys/stat.h>
#include<fcntl.h>
#include<sys/stat.h>
#include <unistd.h>

#include "mt_type.h"
#include "mt_cmdline.h"
#include "mt_common.h"
#include "mt_adp_mpi.h"


#define   CMD_NUM      120
#define   STRNUB       4096
#define   MAX_CMD_LEN  100
#define   MAX_WAIT_SEC 5
#define   MAX_RUN_THREAD 10



static MT_BOOL  MTCommandtaskrun = MT_FALSE;
extern int mt_optind;

typedef  mt_s32 (*MTSampleCommandFunc)(mt_s32 argc, char *argv[]);

typedef struct tag_command_INFO
{
   char name[20];
   char help[100];
   mt_u8 resource;
   MTSampleCommandFunc func;
}command_INFO;

typedef struct tag_run_INFO
{
    char name[20];
    char status;
    mt_u8 resource;
}MT_RUN_INFO;



static command_INFO g_cmd_str[CMD_NUM];

extern MT_S32 MTSampleCommandRegister(const char *name, mt_s32 ( * func)(mt_s32 argc,  char *argv[]), const char * help, u32 useres);
extern void *MTSampleCommadTask(void *pParam);
extern MT_S32 MTSampleCommandRun(MT_CHAR* cmdStr, MT_RUN_INFO *pRuninfo);
extern MT_S32 MTSampleCommandInit(void);


/***********************************
**
**SysLogInit函数    syslog变量初使化
**成功返回FY_CUCCESS
**失败返回FY_FAILSE
**
**
**************************************/


static MT_S32 MTSampleCommand_Help_Func(mt_s32 argc, char *argv[])
{
    char nameTemp[20];
    char helpTemp[100];
    MT_S32 i=0;

    while(g_cmd_str[i].name[0] != '\0')
    {
        strcpy(nameTemp,g_cmd_str[i].name);
        strcpy(helpTemp,g_cmd_str[i].help);
        printf(" %12s : %s\n",nameTemp,helpTemp);
        i++;
        if(i>CMD_NUM)
        {
            break ;
        }
    }

    return MT_SUCCESS;
}

static MT_S32 MTSampleCommand_Quit_Func(mt_s32 argc, char *argv[])
{
    printf("Eixt Current Application\n");
    MTCommandtaskrun = MT_FALSE;

    return MT_SUCCESS;
}

static MT_S32 MTSampleCommand_Shell_Func(mt_s32 argc, char *argv[])
{
    int i  = 0;
    char shellTemp[256] = { 0 };

    printf(" Execute shell commands\n");
    for(i = 1; i < argc; i++)
    {
        strcat(shellTemp, argv[i]);
        strcat(shellTemp, " ");
    }
    system(shellTemp);

    return MT_SUCCESS;
}

static MT_S32  MTSampleCommandFind(const char *name ,command_INFO **cmdfc)
{
    MT_S32  i;
    if((name == NULL) || (*name == 0))
        return MT_FAILURE;
    for(i=0;i<CMD_NUM;i++)
    {
        if(strcmp(g_cmd_str[i].name,name)==0)
        {
            *cmdfc=&(g_cmd_str[i]);
            return MT_SUCCESS;
        }
    }
    return MT_FAILURE;
}

static MT_S32 MTSampleCommandCheckRes(command_INFO *pcmd, MT_RUN_INFO *pRuninfo)
{
    MT_S32  i;

    for(i=0; i<MAX_RUN_THREAD; i++)
    {
        if(pRuninfo[i].status == MT_TRUE)
        {
            if(pcmd->resource & pRuninfo[i].resource)
            {
                if(strcmp(pRuninfo[i].name, pcmd->name)==0)
                {
                    printf("%s is already running... \n", pRuninfo[i].name);
                    return 0;
                }
                else
                {
                    printf("Resource is used by %s... \n", pRuninfo[i].name);
                    return 1;
                }
            }
        }

    }

    return 0;
}


static MT_S32 MTSampleCommandAddRunInfo(command_INFO *pcmd, MT_RUN_INFO *pRuninfo)
{
    MT_S32  i;

    if(*(pcmd->name) == 0)
        return MT_FAILURE;

    for(i=0; i<MAX_RUN_THREAD; i++)
    {
        if(pRuninfo[i].status == MT_TRUE)
        {
            if(strcmp(pRuninfo[i].name, pcmd->name)==0)
            {
                printf("%s is already running... \n", pRuninfo[i].name);
                break;
            }
            continue;
        }

        memcpy(pRuninfo[i].name, pcmd->name, sizeof(pRuninfo[i].name));
        pRuninfo[i].status = MT_TRUE;
        pRuninfo[i].resource = pcmd->resource;

        printf("Add app.[%s]..index[%d].\n", pcmd->name, i);
        break;
    }

    if( i == MAX_RUN_THREAD)
    {
        printf("Max run app. please stop some .\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


static MT_S32 MTSampleCommandDelRunInfo(const char * name, MT_RUN_INFO *pRuninfo)
{
    MT_S32  i;
    if((name == NULL) || (*name == 0))
        return MT_FAILURE;

    for(i=0; i<MAX_RUN_THREAD; i++)
    {
        if((pRuninfo[i].status == MT_TRUE) && (strcmp(pRuninfo[i].name, name)==0))
        {
            printf("Del app.[%s]...\n", name);
            memset(&pRuninfo[i], 0, sizeof(MT_RUN_INFO));
        }
    }

    return MT_SUCCESS;
}


static MT_S32 MTSampleCommandGetArgs(const char* pInput, char *pOutput[], int *argcNum)
{
    char* ptr = (char*)pInput;
    int  curNum=0;
    char  buffer[256];
    char* tp=NULL;
    char *parg = NULL;

    //printf("parse str: %s \n", ptr);

    while(*ptr)
    {
        while(*(ptr) == 0x20)  //delete all space char
        {
            ptr++;
        }

        if(*(ptr) == 10)  //is enter break parse
        {
            //printf("parse str end. \n");
            break;
        }

        tp = buffer;
        while((*ptr!=0) && (*ptr!=0x20) && (*ptr!=10))  //null or space char
        {
            *(tp++) = *(ptr++);
        }

        *tp = '\0'; //add null at the end

        if(*buffer == 0)
            return MT_FAILURE;

        parg = (char*)malloc(strlen(buffer) + 1);

        memset(parg, 0 , strlen(buffer) + 1);
        printf("argc[%d], argv[%s] pOutput: 0x%p \n", curNum, buffer, parg);
        memcpy(parg, buffer, strlen(buffer));

        pOutput[curNum] = parg ;

        curNum++;

    }

    *argcNum = curNum;
    return MT_SUCCESS;
}

static MT_S32 MTSampleCommandFreeArgs(int argc, char *argv[])
{
    int i = 0;

    printf("free args. count [%d].\n", argc);
    for(i=0; i<argc; i++)
    {
        free(argv[i]);
    }

    return MT_SUCCESS;
}


MT_S32 MTSampleCommandRegister(const char *name, mt_s32 ( * func)(mt_s32 argc,  char *argv[]), const char * help, u32 useres)
{
    MT_S32 i,k;

    if(strlen((char*)name)>20)
    {
        return MT_FAILURE;
    }
    if(strlen((char*)help)>100)
    {
        return MT_FAILURE;
    }

    for(i=0;i<CMD_NUM;i++)
    {
        if(strstr(g_cmd_str[i].name,(char*)name)!=NULL)
        {
            return MT_FAILURE;
        }
        if(g_cmd_str[i].name[0]=='\0')
        {
            strcpy((g_cmd_str[i].name),(char*)name);
            strcpy((g_cmd_str[i].help),(char*)help);
            g_cmd_str[i].resource = useres;
            g_cmd_str[i].func=func;
            k=i+1;
            if(k<CMD_NUM)
            {
                g_cmd_str[k].name[0]='\0';
            }
            //printf(" Register is printf  %s\n\r",g_cmd_str[i].name);
            return MT_SUCCESS;
        }
    }
    return MT_FAILURE;
}



void *MTSampleCommadTask(void *pParam)
{
    char *fgetret=NULL;
    MT_CHAR inputCmd[512];
    MT_RUN_INFO run_info[MAX_RUN_THREAD];
    mt_u8 i = 0;

	memset(&run_info, 0, sizeof(run_info));

    while(MTCommandtaskrun)
    {
        //MTSampleCommand_Help_Func(0, NULL);
        printf("MTCMD>> ");
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);//SAMPLE_GET_INPUTCMD(InputCmd);
        fgetret=fgetret;

        MTSampleCommandRun(inputCmd, run_info);
    }

    for(i=0; i<MAX_RUN_THREAD; i++)
    {
        if(run_info[i].status == MT_TRUE)
        {
            snprintf(inputCmd, 256, "%s -q", run_info[i].name);
            MTSampleCommandRun(inputCmd, run_info);
        }
    }

    return NULL;
}


MT_S32 MTSampleCommandRun(MT_CHAR* cmdStr, MT_RUN_INFO *pRuninfo)
{
    char   *argv[32] = {0};
    MT_CHAR  runStr[128];
    command_INFO *pcmd;
    int argc = 0;
    int ret = 0;

    if(cmdStr==NULL)
    {
        printf("parameter error !!\n");
        return -1;
    }

    mt_optind = 1;
    if(MTSampleCommandGetArgs(cmdStr, argv, &argc) != MT_SUCCESS)
    {
        printf("parameter error !!\n");
        return -1;
    }

    //printf("exit  MTSampleCommandGetArgs  name [%s] argc: [%d]!!\n", argv[0], argc);

    if(MTSampleCommandFind(argv[0], &pcmd) != MT_SUCCESS)
    {
        printf(" command not support,please input help to get command list!!\n");
        return -1;
    }

    if(MTSampleCommandCheckRes(pcmd, pRuninfo))
    {
        printf("Resource is use, please stop ,then run %s \n", pcmd->name);
        MTSampleCommandFreeArgs(argc, argv);
        return -1;
    }

    ret = pcmd->func(argc, argv);
    if(1 == ret)
    {
        ret = MTSampleCommandAddRunInfo(pcmd, pRuninfo);
        if(MT_SUCCESS != ret)
        {
            snprintf(runStr, 128, "%s -q", argv[0]);
            MTSampleCommandRun(runStr, pRuninfo);
        }

    }
    else if(MT_SUCCESS == ret)
    {
        MTSampleCommandDelRunInfo(argv[0], pRuninfo);
    }

    MTSampleCommandFreeArgs(argc, argv);
    return 0;
}

MT_S32 MTSampleCommandInit(void)
{
    memset(g_cmd_str, 0, sizeof(g_cmd_str));

    MTSampleCommandRegister("help",MTSampleCommand_Help_Func,"Command_Line help command.", 0);
    MTSampleCommandRegister("exit",MTSampleCommand_Quit_Func,"Exit App command.", 0);
    MTSampleCommandRegister("sh",MTSampleCommand_Shell_Func,"This linux shell command.", 0);

    MTCommandtaskrun = MT_TRUE;

    return 0;
}


