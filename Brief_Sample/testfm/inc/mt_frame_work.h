/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef    __MT_FRAME_WORK_H__
#define     __MT_FRAME_WORK_H__


#include "mt_type.h"


#define MAIN_COMMAOND_LEN_MAX      (20)
#define MAIN_COMMAOND_COUNT		(30)
#define COMMAOND_OPTIONS_MAX		(20)
#define OPTIONS_PARAM_NUM_MAX	(20)
#define OPTIONS_DATA_LEN_MAX       	(256)
#define TESTFM_MODULE_NAME_MAX  	(30)




typedef enum
{
  TESTFM_TESTFM_SHM_KEY = 25,
  TESTFM_SCI_SHM_KEY,
  TESTFM_IR_SHM_KEY,
  TESTFM_SAMPLE_TEMPLATE_CMD_SHM_KEY,
  TESTFM_MTGO_CMD_SHM_KEY,   
  
  TESTFM_SHM_KEY_MAX
}testfm_shm_key_t;

typedef enum
{
  RET_SUCCESS = 0,
  RET_TTESTFM_MAINCMD_EER ,
  RET_TTESTFM_PARAM_OVER ,
  RET_TTESTFM_PARAM_ERR,
  RET_TTESTFM_OPTS_OVER,
  RET_TTESTFM_OPTS_ERR,
  RET_FAIL,
}testfm_result_t;

typedef struct mt_option_param
{
   char opts_param[MAIN_COMMAOND_LEN_MAX];
   char opts_data[OPTIONS_DATA_LEN_MAX];
}mt_option_param_t;

typedef struct mt_testfm_option_param
{
   char opts_param[MAIN_COMMAOND_LEN_MAX];
   char opts_data[OPTIONS_DATA_LEN_MAX];
   MT_BOOL opts_flag;
}mt_testfm_option_param_t;

typedef struct mt_testfm_cmdoption
{
   char main_cmd[MAIN_COMMAOND_LEN_MAX];
   char *shortopts;
   const struct option *opts;
   int opts_list_len;
   mt_testfm_option_param_t   opts_list_curr[OPTIONS_PARAM_NUM_MAX];
}mt_testfm_cmdoption_t;

typedef struct testfm_cmd_information
{
	char* myoptarg;
	char commond_arg[OPTIONS_PARAM_NUM_MAX];
	int main_cmd_index;
	int test_count;  
	MT_BOOL shm_status;
	mt_testfm_cmdoption_t  mt_test_cmdopt[MAIN_COMMAOND_COUNT];
	char testfm_module_name[TESTFM_MODULE_NAME_MAX]; 
}testfm_cmd_information_t;

extern testfm_cmd_information_t *testfm_infor_shmptr; 

extern ulong testfm_get_num(char *str);

//extern int mt_get_mt_opts_list(mt_testfm_option_param_t   **opt_list);
//获得当前输入的 主命令名称，如果有返回true，否则返回false
extern MT_BOOL  mt_get_curr_opts_cmd(char curr_opts[]);

//检测某个指定参数是否有在testfm中，如果有返回true，否则返回false
extern MT_BOOL  mt_check_fixed_otp(char *fixed_otp);

//传入指定参数param_name，得到该参数对应的值，有参数返回true，没有返回false
extern MT_BOOL mt_get_param_testfm(char* param_name, char param_data[]);
//传入mt_option_param_t存放参数名字和参数值，指针p_count存放参数数目
extern MT_BOOL mt_get_params_count_testfm(mt_option_param_t  p_params[], int *p_count);
//根据传入的mt_option_param_t，找到指定的参数名对应的参数值
extern MT_BOOL mt_get_param_opts(char* p_name, mt_option_param_t  p_params[], int count, char param_data[]);

//添加指定命令到测试程序中，添加的是一个命令数组，其将之前的所有注册无效
extern int mt_add_opts_to_testfm(mt_testfm_cmdoption_t testfm_cmdopt[], int cmd_num);
//添加指定的单个命令到测试程序中，不改变之前注册的命令
int mt_reg_opt_to_testfm(char *main_cmd, const struct option *opts, char *shortopts);

//传入main函数参数进行处理 
extern testfm_result_t mt_testfm_cmd_process(int32_t argc, char **argv);
//初始化testfm
extern void mt_testfm_int(char *mod_name);
//退出testfm
extern void mt_testfm_exit(void);
//检测当前是否是在测试指定模块
extern MT_BOOL mt_testfm_check_module(char *mod_name);

 void private_setresult(testfm_result_t result, mt_u32 errorcode);
#define TESTFM_SETRESULT(result, errorcode, is_assert) \
  do \
  {  \
    private_setresult(result, errorcode);\
  }\
  while (0)


  #define TESTFM_LOG(...) \
    do\
    { \
      printf("#LOG: ");\
      printf(__VA_ARGS__);\
      printf(" #END\n");\
    }\
    while(0)



mt_u32 mt_test_get_num(mt_char *str);



#endif
