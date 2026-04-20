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

#include "ngwm_ree.h"


#define   CMD_NUM      120
#define   STRNUB       4096
#define   MAX_CMD_LEN  100
#define   MAX_WAIT_SEC 5
#define   MAX_RUN_THREAD 10



static MT_BOOL  MTCommandtaskrun = MT_FALSE;

typedef  mt_s32 (*MTSampleCommandFunc)(mt_s32 argc, char *argv[]);

typedef struct tag_command_INFO
{
   char name[20];
   char help[MAX_CMD_LEN];
   mt_u8 resource;
   MTSampleCommandFunc func;
}command_INFO;

typedef struct tag_run_INFO
{
    char name[20];
    char status;
    mt_u8 resource;
}MT_RUN_INFO;

#define MTADP_MPI_DEBUG

#ifdef MTADP_MPI_DEBUG

#define MTADP_MPI_PRINT   printf

#else

#define MTADP_MPI_PRINT

#endif

#define MTADP_MPI_FUNCTION_ENTER()  MTADP_MPI_PRINT("[MTADP_MPI][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_MPI_FUNCTION_EXIT()   MTADP_MPI_PRINT("[MTADP_MPI][%s]: Exit ==<< \n", __FUNCTION__)

int mt_optind = 1;
static int mt_optopt;
char *mt_optarg;


static command_INFO g_cmd_str[CMD_NUM];

extern MT_S32 MTSampleCommandRegister(const char *name, mt_s32 ( * func)(mt_s32 argc,  char *argv[]), const char * help, u32 useres);
extern void *MTSampleCommadTask(void *pParam);
extern MT_S32 MTSampleCommandRun(MT_CHAR* cmdStr, MT_RUN_INFO *pRuninfo);
extern MT_S32 MTSampleCommandInit(void);

typedef struct
{
    mt_u32 test_id;
} mt_input_wm_para_t;

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

    while(MTCommandtaskrun)
    {
        //MTSampleCommand_Help_Func(0, NULL);
        printf("MTCMD>> ");
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);//SAMPLE_GET_INPUTCMD(InputCmd);
        fgetret=fgetret;
 if (fgetret)
    printf("fgetret, 0x%x, 0x%x, 0x%x \n", fgetret[0], fgetret[1], fgetret[2]);
 else
    printf("It is NULL \n");
        MTSampleCommandRun(inputCmd, run_info);
        MT_USLEEP(10000);
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
    printf("cmdStr = %s \n", cmdStr);
    mt_optind = 1; // 1  command parameters ?
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



mt_s32 MTADP_Getopt(int argc, char *argv[], char *opts)
{
    static int sp = 1;
    int c;
    char *cp;

    MTADP_MPI_FUNCTION_ENTER();

    if (sp == 1) {
        if (mt_optind >= argc)
            return EOF;
        else if (!strcmp(argv[mt_optind], "--")) {
            mt_optind++;
            return EOF;
        }
        else if(argv[mt_optind][0] != '-' || argv[mt_optind][1] == '\0')
        {
            return '?';
        }
    }
    mt_optopt = c = argv[mt_optind][sp];

    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
        //fprintf(stderr, ": illegal option -- %c\n", c);
        if (argv[mt_optind][++sp] == '\0') {
            mt_optind++;
        }
        sp = 1;
        return '?';
    }

    if (*++cp == ':') {
        if (argv[mt_optind][sp+1] != '\0')
            mt_optarg = &argv[mt_optind++][sp+1];
        else if(++mt_optind >= argc) {
            //fprintf(stderr, ": option requires an argument -- %c\n", c);
            sp = 1;
            return '?';
        } else
            mt_optarg = argv[mt_optind++];
        sp = 1;
    } else {
        if (argv[mt_optind][++sp] == '\0') {
            sp = 1;
            mt_optind++;
        }
        mt_optarg = NULL;
    }

    MTADP_MPI_FUNCTION_EXIT();

    return c;
}

/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static MT_VOID MT_wmPrint_help(MT_CHAR *name)
{
    printf("Please input parameters: 0, 1, 2 ...\n");

}


/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@param[out] pInutParam      Analyze input parameter values
@return::void
@*/
static mt_s32 MT_WMParase_args(int argc, char *argv[], mt_input_wm_para_t *pOutParam)
{
    int opt = 0;



    if(argc < 1)
    {
        (MT_VOID)MT_wmPrint_help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:0:1:2")) != -1)
    {
        printf("%s, %d, 0x%x == \n", __FUNCTION__, __LINE__, opt);

        switch(opt)
        {
            case 'h':
            case 'H':
                (MT_VOID)MT_wmPrint_help(argv[0]);
                return MT_FAILURE;
            case '0':
                pOutParam->test_id = 0;
                goto end;
            case '1':
                //pOutParam->sym_rate = strtol(mt_optarg, 0, 0);
                pOutParam->test_id = 1;
                goto end;
            case '2':
                //pOutParam->mod_type = strtol(mt_optarg, 0, 0);
                pOutParam->test_id = 2;
                goto end;

                return MT_FAILURE;
            default:
                (MT_VOID)MT_wmPrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
end:
    return MT_SUCCESS;
}

static MT_S32 MTSampleCommand_WM_Func(mt_s32 argc, char *argv[])
{
    int i  = 0;
    //char shellTemp[256] = { 0 };
    char testVector_en[9] = {0xF0, 0x04, 0x00, 0x00, 0x00, 0x02, 0xD0, 0x01, 0x80};
    char testVector_dis[9] = {0xF0, 0x04, 0x00, 0x00, 0x00, 0x02, 0xD0, 0x01, 0x00};//{0xF0, 0x04, 0x00, 0x00, 0x00, 0x01};

    unsigned char _19_0x12345678_0xc8_1530532846_settingD_stubon_on_bin[] = {
      0xf0, 0x04, 0x00, 0x00, 0x00, 0x01, 0x40, 0x04, 0x12, 0x34, 0x56, 0x78,
      0x41, 0x04, 0x5b, 0x3a, 0x13, 0xee, 0x43, 0x08, 0x01, 0x00, 0x05, 0xc8,
      0x01, 0xac, 0x7f, 0x33, 0x44, 0x88, 0x01, 0x00, 0x05, 0x06, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xff, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xd0, 0x01,
      0x80
};


    mt_input_wm_para_t  mt_input_wm_para = {0,};
    char *p_testvec = NULL;
    unsigned char input_len = 0;
    mt_s32 ret = -1;
    char input_para[128] = {0,};
    char opts = 0;

    mt_input_wm_para.test_id = 1;


    printf("ngwmGetReeInterface()->configure: %d, %s, %s\n", argc, argv[0], argv[1]);

    input_len = strlen(argv[1]);
    if (input_len > 128)
        input_len = 128;

    memcpy(input_para, argv[1], input_len);


    for (i = 0; i < input_len; i++) {
        opts = input_para[i];
        printf("i= %d, opts = 0x%x\n", i, opts);
        if ((opts >= '0' ) && (opts >= '9' ))
            break;
    }

    printf("%s, %d opts = 0x%x\n", __FUNCTION__, __LINE__, opts);


/*
    ret = MT_WMParase_args(argc, argv, &mt_input_wm_para);
    if (ret == MT_SUCCESS)
        printf(" Execute warter mark commands: 0x%x, opts=%c\n", mt_input_wm_para.test_id, *opts);
    else {
        printf("input error: \n");
        return -1;
    }
*/
    if (opts == '0') {
        mt_input_wm_para.test_id = 0;
    } else if (opts == '1') {
        mt_input_wm_para.test_id = 1;
    } else {
        mt_input_wm_para.test_id = 2;
    }

    input_len = 9;
    if (mt_input_wm_para.test_id == 0)
    {
        p_testvec = testVector_dis;
        printf("%s, %d \n", __FUNCTION__, __LINE__);
    } else if (mt_input_wm_para.test_id == 1)
    {
        p_testvec = testVector_en;
        printf("%s, %d \n", __FUNCTION__, __LINE__);
    } else if (mt_input_wm_para.test_id == 2)
    {
        p_testvec = _19_0x12345678_0xc8_1530532846_settingD_stubon_on_bin;
        input_len = sizeof(_19_0x12345678_0xc8_1530532846_settingD_stubon_on_bin);
    } else {
        MT_wmPrint_help(&opts);
        return MT_SUCCESS;
    }
    //printf("%s, %d \n", __FUNCTION__, __LINE__, p_testvec, input_len);

    // don't use pipe interface.
    TNgwmReeResult res = ngwmGetReeInterface()->configure(p_testvec, input_len);
    //use pipe interface
    //TNgwmReeResult res = ngwmGetReeInterface()->configureByPipe(1, p_testvec, input_len);
    if (res != NGWM_REE_SUCCESS)
    {
        printf("Error! configuration (%d)\n", res);
        //exit(1);
    }


    return MT_SUCCESS;
}
MT_S32 MTSampleCommandInit(void)
{
    memset(g_cmd_str, 0, sizeof(g_cmd_str));

    MTSampleCommandRegister("help",MTSampleCommand_Help_Func,"Command_Line help command.", 0);
    MTSampleCommandRegister("exit",MTSampleCommand_Quit_Func,"Exit App command.", 0);
    MTSampleCommandRegister("sh",MTSampleCommand_Shell_Func,"This linux shell command.", 0);
    MTSampleCommandRegister("wm",MTSampleCommand_WM_Func,"Test REE watermark.", 0);

    MTCommandtaskrun = MT_TRUE;

    return 0;
}


