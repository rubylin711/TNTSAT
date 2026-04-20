/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <unistd.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <mt_type.h>
#include <assert.h>

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

#include <errno.h>
#include <fcntl.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include <mt_frame_work.h>

#define MT_ASSERT(x)    assert(x)
#if 0
static pthread_mutex_t g_testfm_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_TESTFM_LOCK() (void)pthread_mutex_lock(&g_testfm_mutex);
#define MT_TESTFM_UNLOCK() (void)pthread_mutex_unlock(&g_testfm_mutex);
#endif



testfm_cmd_information_t *testfm_infor_shmptr;


void private_setresult(testfm_result_t result, mt_u32 errorcode)
{
  if(result == (testfm_result_t)RET_SUCCESS)
  {
    printf("RET: success\n");
  }
  else
  {
    printf("RET: fail\n");
    printf("ENO: %x\n", errorcode);
  }
}
#if 0
static char mt_dec2char(mt_u8 num)
{
  if(/*(num >= 0) && */(num <= 9))
    return ('0' + num);

  if((num >= 0xa) && (num <= 0xf))
    return ('a' + (num-10));

  return ' ';
}
#endif
static mt_u8 mt_char2dec(char c)
{
  if((c >= '0') && (c <= '9'))
    return (c-'0');

  if((c >= 'a') && (c <= 'f'))
    return (10 + (c-'a'));

  if((c >= 'A') && (c <= 'F'))
    return (10 + (c-'A'));

  return 0xff;
}

mt_u32 mt_test_get_num(mt_char *str)
{
  int len = 0;
  int i = 0;
  mt_u32 num = 0;
  int indicate = 0;
  mt_u8 dec = 0;
  char *p_str = str;

  if(str == NULL)
  {
     return 0;
  }

  if((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X')))
  {
    indicate = 1;
    p_str = &str[2];
  }

  len = strlen(p_str);

  if(len == 0)
    return -1;

  for(i = 0; i < len; i++)
  {
    dec = mt_char2dec(p_str[i]);
    if(dec == 0xff)
      continue;

    if(indicate == 1)
      num = num<<4;
    else
      num = num *10;
    num += dec;
  }

  return num;
}

/*
说明：函数中的argc和argv通常直接从main()到两个参数传递而来。
optsting是选项参数组成的字符串，如果该字符串里任一字母后有冒号，那么这个选项就要求有参数，
optarg就是选项参数，如果要带多个参数，需要将整个参数通过引号作用。如: -c "0x123 123 abc ddd"
optind是当前索引，
optopt用于当发现无效选项字符的时候，getopt函数或者返回 “？”或者返回“：”字符，
并且optopt包含了所发现的无效选项字符。
如果optstring参数的第一个字符是冒号，那么getopt会根据错误情况返回不同的字符，
当错误是无效选项，getopt返回“？”，当错误是缺少选项参数，getopt返回“：”。
注：GNU getopt()第三个特点是optstring中的选项字符后面接两个冒号，就允许该选项有可选的选项参数。
在选项参数不存在的情况下，GNU getopt()返回选项字符并将optarg设置为NULL。
*/
static int getopt_val(int* i,int argc,char*const* argv,const char *shortopts,
        const struct option *longopts){
    char *arg = argv[*i];
    char *ptr;
    const struct option *opt_ptr = longopts;
    if(arg[0] == '-'){
        if(strlen(arg) == 2){
            //短参数项
            if((ptr=strchr(shortopts,arg[1])) != NULL){
                if(ptr < &shortopts[strlen(shortopts)-1]){
                    if(*(ptr+1) == ':'){
                        //必须有参数
                        if(*i == argc -1){
                            testfm_infor_shmptr->myoptarg = NULL;
                            return '?';
                        }else{
                            if(argv[*i+1][0] == '-'){
                                testfm_infor_shmptr->myoptarg = NULL;
                                return '?';
                            }else{

                                testfm_infor_shmptr->myoptarg = argv[++(*i)];
                                return *ptr;
                            }
                        }
                    }else{
                      //无参数
                      testfm_infor_shmptr->myoptarg = NULL;
                      return *ptr;
                    }

                }else{
                    //无参数
                    testfm_infor_shmptr->myoptarg = NULL;
                    return *ptr;
                }
            }else{
                testfm_infor_shmptr->myoptarg = NULL;
                return '?';
            }
        }else if(strlen(arg) > 2){
            if(arg[1] == '-'){
                //长参数项
                int flag = 0;
                while(opt_ptr->name != NULL){
                    //printf("==i=%d=ddd=name=%s====arg=%s==\n", *i,opt_ptr->name, argv[*i]+2);
                    if(strncmp(opt_ptr->name,argv[*i]+2,strlen(argv[*i])-2) == 0){
                        flag = 1;
                        switch(opt_ptr->has_arg){
                        case required_argument:
                            if(*i == argc - 1){
                                testfm_infor_shmptr->myoptarg = NULL;
                                return '?';
                            }else{
                                if(argv[*i+1][0] == '-'){
                                    testfm_infor_shmptr->myoptarg = NULL;
                                    return '?';
                                }else{
                                    testfm_infor_shmptr->myoptarg = argv[++(*i)];
                                    return opt_ptr->val;
                                }
                            }
                            break;
                        case optional_argument:
                            if(*i == argc - 1){
                                testfm_infor_shmptr->myoptarg = NULL;
                                return opt_ptr->val;
                            }else{
                                if(argv[*i+1][0] == '-'){
                                    testfm_infor_shmptr->myoptarg = NULL;
                                    return opt_ptr->val;
                                }else{
                                    testfm_infor_shmptr->myoptarg = argv[*i+1];
                                    return opt_ptr->val;
                                }
                            }
                            break;
                        case no_argument:
                            testfm_infor_shmptr->myoptarg = NULL;
                            return opt_ptr->val;
                        default:
                            testfm_infor_shmptr->myoptarg = NULL;
                            return '?';
                        }
                    }else{
                        opt_ptr++;
                        continue;
                    }
                }
                if(flag == 0){
                    //不合法参数
                    testfm_infor_shmptr->myoptarg = NULL;
                    return '?';
                }
            }else{
                testfm_infor_shmptr->myoptarg = NULL;
                //-开头的长参数项，暂时视为错误
                return '?';
            }
        }else{
            // -
            testfm_infor_shmptr->myoptarg = NULL;
            return '?';
        }

    }else{
        //传进来的必须是参数项
        testfm_infor_shmptr->myoptarg = NULL;
        return '?';
    }

    testfm_infor_shmptr->myoptarg = NULL;
    return '?';
}

static int mygetopt_long(int argc, char *const *argv,
        const char *shortopts,
            const struct option *longopts,int *longind){

//printf("\n #%s###%s###%s#\n", argv[1], argv[2], argv[3]);
    //1.参数检查
    //2.函数环境初始化
    static int i = 0;
    i++;
    if(i >= argc){
        return -1;
    }
    //起始的时候item为?
    static int item = '?';
    testfm_infor_shmptr->myoptarg = NULL;

    //3.函数业务逻辑开始
    if(item != '?' && argv[i][0] != '-'){
        //当前是多值读取状态
        //读取下一个值
        testfm_infor_shmptr->myoptarg = argv[i];
        return item;
    }else if(item == '?' && argv[i][0] != '-'){
        //当前是程序后面直接跟参数的情况
        testfm_infor_shmptr->myoptarg = NULL;
        return '?';

    }else if(item != '?' && argv[i][0] == '-'){
        //多值读取状态，但是当前值是一个参数项，结束多值读取状态,开始检查参数
        item = getopt_val(&i,argc,argv,shortopts,longopts);
        return item;

    }else{
        // item == '?' && argv[i][0] == '-'
        item = getopt_val(&i,argc,argv,shortopts,longopts);

        return item;
    }

    //4.函数处理完毕返回
    return '?';
}


int mt_add_opts_to_testfm(mt_testfm_cmdoption_t testfm_cmdopt[], int cmd_num)
{
   int i;
   //const struct option *opt_temp = NULL;
   if((cmd_num >= MAIN_COMMAOND_COUNT) || (!testfm_infor_shmptr->shm_status))
    return -1;
   testfm_infor_shmptr->shm_status = MT_FALSE;
   memset(testfm_infor_shmptr->mt_test_cmdopt, 0x0, MAIN_COMMAOND_COUNT * sizeof(mt_testfm_cmdoption_t));
   for(i = 0; i < cmd_num; i++)
   {

      //opt_temp = testfm_cmdopt[i].opts;
      testfm_infor_shmptr->mt_test_cmdopt[i].opts = testfm_cmdopt[i].opts;
      testfm_infor_shmptr->mt_test_cmdopt[i].shortopts = testfm_cmdopt[i].shortopts;
      strcpy(testfm_infor_shmptr->mt_test_cmdopt[i].main_cmd, testfm_cmdopt[i].main_cmd);

      //printf("i=%d,  main_cmd= %s\n",i,testfm_cmdopt[i].main_cmd);
      //printf("     addr=%p  \n", testfm_infor_shmptr->mt_test_cmdopt[i].opts);
      //printf("    name= %s   \n",testfm_infor_shmptr->mt_test_cmdopt[i].opts->name);
   }
  testfm_infor_shmptr->shm_status = MT_TRUE;
   return 0;

}

int mt_reg_opt_to_testfm(char *main_cmd, const struct option *opts, char *shortopts)
{
  int i;
   if(!testfm_infor_shmptr->shm_status || (!main_cmd) || (!opts) || (!shortopts))
    return -1;

   testfm_infor_shmptr->shm_status = MT_FALSE;
  for(i = 0; i < MAIN_COMMAOND_COUNT; i++)
  {
     if(NULL == testfm_infor_shmptr->mt_test_cmdopt[i].opts)
     {
        memset(&(testfm_infor_shmptr->mt_test_cmdopt[i]), 0x0, sizeof(mt_testfm_cmdoption_t));
        testfm_infor_shmptr->mt_test_cmdopt[i].opts = opts;
        testfm_infor_shmptr->mt_test_cmdopt[i].shortopts = shortopts;
        strcpy(testfm_infor_shmptr->mt_test_cmdopt[i].main_cmd, main_cmd);
    break;
     }
  }
  testfm_infor_shmptr->shm_status = MT_TRUE;
   if(i >= MAIN_COMMAOND_COUNT)
    return -1;

   return 0;
}

void mt_testfm_int(char *mod_name)
{
   int i = 0;
   int shmid;
    if((NULL == mod_name) ||(strlen(mod_name) >= TESTFM_MODULE_NAME_MAX))
    {
      printf("test module name error!!!\n");
      exit(1);
    }
    if((shmid = shmget(TESTFM_TESTFM_SHM_KEY,sizeof(testfm_cmd_information_t),IPC_CREAT)) ==-1)
    {
      printf("testfm shmget error =0x%x \n", errno);
      exit(1);
    }
    printf("shmid is %d\n",shmid);
    if((testfm_infor_shmptr =(testfm_cmd_information_t *)shmat(shmid,0,0))==(void *)-1)
    {
        printf("testfm shmptr error!\n");
        exit(1);
    }


    //testfm_infor_shmptr->commond_arg = NULL;
    memset(testfm_infor_shmptr->commond_arg, 0x0, COMMAOND_OPTIONS_MAX);
    for(i = 0; i < MAIN_COMMAOND_COUNT; i++)
        {
          memset(testfm_infor_shmptr->mt_test_cmdopt[i].main_cmd, 0x0,MAIN_COMMAOND_LEN_MAX);
       testfm_infor_shmptr->mt_test_cmdopt[i].opts = NULL;
       testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len = 0;
      testfm_infor_shmptr->mt_test_cmdopt[i].shortopts = NULL;
          memset(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr, 0x0, OPTIONS_PARAM_NUM_MAX * sizeof(mt_testfm_option_param_t));

        }
     memset(testfm_infor_shmptr->testfm_module_name, 0x0, TESTFM_MODULE_NAME_MAX);
     memcpy(testfm_infor_shmptr->testfm_module_name, mod_name, strlen(mod_name));
     testfm_infor_shmptr->shm_status = MT_TRUE;
     //printf("$$$$testfm_infor_shmptr->testfm_module_name= %s\n", testfm_infor_shmptr->testfm_module_name);
}
void mt_testfm_exit(void)
{
     mt_s32 s32Ret = 0;

     testfm_infor_shmptr->shm_status = MT_FALSE;
    //脱离共享内存区段
    s32Ret=shmdt(testfm_infor_shmptr);
    if( s32Ret )
        printf( "Memory detached failed %d /n", s32Ret);

}

MT_BOOL mt_testfm_check_module(char *mod_name)
{

//printf("--mod_name-0--%s----1--%s-\n",testfm_infor_shmptr->testfm_module_name, mod_name);
   if((NULL == mod_name) ||(strlen(mod_name) >= TESTFM_MODULE_NAME_MAX))
    {
        return MT_FALSE;
    }

   if(0 == strcmp(testfm_infor_shmptr->testfm_module_name, mod_name))
   {
        return MT_TRUE;
   }
   return MT_FALSE;
}
testfm_result_t mt_testfm_cmd_process(int32_t argc, char **argv)
{
   int32_t opt = 0, i = 0, k = 0;
   testfm_result_t ret = RET_SUCCESS;
   const struct option *temp_opts = NULL;

    if(!testfm_infor_shmptr->shm_status)
   {
     return RET_FAIL;
   }
    testfm_infor_shmptr->shm_status = MT_FALSE;

    if(argc >= 2)
        {
            if((NULL ==argv[1]) || strlen(argv[1]) >= MAIN_COMMAOND_LEN_MAX)
        {
          ret = RET_TTESTFM_MAINCMD_EER;
          goto testfm_err_0;
        }

        memset(testfm_infor_shmptr->commond_arg, 0x0, COMMAOND_OPTIONS_MAX);
        memcpy(testfm_infor_shmptr->commond_arg, argv[1], strlen(argv[1]));
        printf("\n testfm_infor_shmptr->commond_arg=%s,   %p\n", testfm_infor_shmptr->commond_arg, testfm_infor_shmptr->commond_arg);

        if(0 == testfm_infor_shmptr->commond_arg[0])
            {
            ret = RET_TTESTFM_MAINCMD_EER;
            goto testfm_err_0;
            }


           for(i = 0; i < MAIN_COMMAOND_COUNT; i++)
           {

              if(0 == strcmp(testfm_infor_shmptr->commond_arg, testfm_infor_shmptr->mt_test_cmdopt[i].main_cmd))
                {
                        break;
                }
           }

           if(i >= MAIN_COMMAOND_COUNT)
            {
                           ret = RET_TTESTFM_MAINCMD_EER;
                  goto testfm_err_0;
            }

          testfm_infor_shmptr->main_cmd_index = i;
       while ((opt = mygetopt_long((argc - 1), &argv[1],  testfm_infor_shmptr->mt_test_cmdopt[i].shortopts, testfm_infor_shmptr->mt_test_cmdopt[i].opts, NULL)) != -1)
       {
             if('?' == opt)
                {
           ret = RET_TTESTFM_PARAM_ERR;
           goto testfm_err_0;
                }
              temp_opts = testfm_infor_shmptr->mt_test_cmdopt[i].opts;
        while( temp_opts->val)
        {
           if(opt == temp_opts->val)
           {

              if(strlen(temp_opts->name) >= MAIN_COMMAOND_LEN_MAX)
                {
                ret = RET_TTESTFM_OPTS_ERR;
                goto testfm_err_0;
                }

              if(NULL != testfm_infor_shmptr->myoptarg && strlen(testfm_infor_shmptr->myoptarg) >= OPTIONS_DATA_LEN_MAX)
             {
                ret = RET_TTESTFM_PARAM_OVER;
                goto testfm_err_0;
                }
              if(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len >= COMMAOND_OPTIONS_MAX)
                {
                ret = RET_TTESTFM_OPTS_OVER;
                goto testfm_err_0;
                }

//check opt name wether exist
                     for(k = 0; k <testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len; k++)
                        {
                         if(0 == strcmp(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[k].opts_param, temp_opts->name))
                     {
                           break;
                      }
                        }
            if(k < testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len)
            {
                             if(testfm_infor_shmptr->myoptarg)
                {
                      if(strlen(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[k].opts_data) + 1+strlen(testfm_infor_shmptr->myoptarg) >= OPTIONS_DATA_LEN_MAX)
                     {
                        ret = RET_TTESTFM_PARAM_OVER;
                        goto testfm_err_0;
                        }

                      strcat(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[k].opts_data, " ");//多参数用空格隔开
                               strcat(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[k].opts_data, testfm_infor_shmptr->myoptarg);
                               testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[k].opts_flag = MT_TRUE;
                }
                            break;
            }


            if(NULL == testfm_infor_shmptr->myoptarg)
                        {
                        testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len].opts_flag = MT_FALSE;

                        }
            else
            {
                 if(strlen(testfm_infor_shmptr->myoptarg) >= OPTIONS_DATA_LEN_MAX)
                    {
                    ret = RET_TTESTFM_PARAM_OVER;
                    goto testfm_err_0;
                    }
                          memcpy(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len].opts_data, testfm_infor_shmptr->myoptarg, strlen(testfm_infor_shmptr->myoptarg));
                          testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len].opts_flag = MT_TRUE;
            }
            memcpy(testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_curr[testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len].opts_param, temp_opts->name, strlen( temp_opts->name));
            testfm_infor_shmptr->mt_test_cmdopt[i].opts_list_len++;
                     break;
           }

                 temp_opts++;
        }
      }
        }
    else
    {
        ret =  RET_FAIL;
    }

testfm_err_0:
    testfm_infor_shmptr->shm_status = MT_TRUE;
    return ret;
}



MT_BOOL mt_get_param_testfm(char* param_name, char param_data[])
{
   //testfm_infor_shmptr->main_cmd_index is current main coommand
   int i =0, len = 0;

   len = testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_len;
   for(i = 0; i < len; i++)
    {
          if(0 == strcmp(param_name, testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param))
        {
                break;
        }
    }
     if((i >= len) || (testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_flag == MT_FALSE))
        return MT_FALSE;

     memset(param_data, 0x0, strlen(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_data));
     memcpy(param_data, testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_data, strlen(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_data));
     return MT_TRUE;
}


MT_BOOL mt_get_params_count_testfm(mt_option_param_t  p_params[], int *p_count)
{
   //testfm_infor_shmptr->main_cmd_index is current main coommand
   int i =0, len = 0;

   len = testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_len;
   *p_count = 0;
   for(i = 0; i < len; i++)
    {
          if(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param[0] && (testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_flag != MT_FALSE))
         {
                memset(p_params[i].opts_param, 0x0, sizeof(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param) + 1);
                memset(p_params[i].opts_data, 0x0, sizeof(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_data) + 1);
          memcpy(p_params[i].opts_param, testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param, strlen(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param));
          memcpy(p_params[i].opts_data, testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_data, strlen(testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_data));
                (*p_count)++;
       }
    }
   if(*p_count)
      return MT_TRUE;

   return MT_FALSE;
}

MT_BOOL mt_get_param_opts(char* p_name, mt_option_param_t  p_params[], int count, char param_data[])
{
   //testfm_infor_shmptr->main_cmd_index is current main coommand
   int i =0;

   for(i = 0; i < count; i++)
    {
        if (0 == strcmp(p_name, p_params[i].opts_param))
        {
            memset(param_data, 0x0, strlen(p_params[i].opts_data));
            memcpy(param_data, p_params[i].opts_data, strlen(p_params[i].opts_data));
            break;
        }
    }
     if(i >= count)
        return MT_FALSE;

     return MT_TRUE;
}


MT_BOOL  mt_get_curr_opts_cmd(char curr_opts[])
{

   memset(curr_opts, 0x0, COMMAOND_OPTIONS_MAX);
   memcpy(curr_opts, testfm_infor_shmptr->commond_arg, strlen(testfm_infor_shmptr->commond_arg));

   return MT_TRUE;
}

MT_BOOL  mt_check_fixed_otp(char *fixed_otp)
{
  int i =0, len = 0;

   len = testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_len;
//printf("len=%d====\n",len);
   for(i = 0; i < len; i++)
   {
   //printf("====i=%d===param=%s=\n",i, testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param);
     if(0 == strcmp(fixed_otp, testfm_infor_shmptr->mt_test_cmdopt[testfm_infor_shmptr->main_cmd_index].opts_list_curr[i].opts_param))
    {
           return MT_TRUE;;
    }
   }

   return MT_FALSE;
}
#if 0
 void debug_dump_cmd()
{
  MT_BOOL ret = 0;
  char param_data[256] = {0,};
  printf("dump string\n");

  ret = mt_get_param_testfm("config0",param_data);
  if(ret)
  printf("config0:ret = %d, %s\n", ret,param_data);


  ret = mt_get_param_testfm("config1",param_data);

    if(ret)
  printf("config1:ret = %d, %s\n", ret,param_data);


ret = mt_get_param_testfm("help1",param_data);
  if(ret)
  printf("help1:ret = %d, %s\n", ret,param_data);


}

#endif

