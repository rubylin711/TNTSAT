/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "mt_unf_timer.h"

//#include "mt_frame_work.h"

static mt_s32 timer_cnt = 0xff;
static mt_u32 timer_id = -1;

static void timer_default_callback(int sig)
{
  if(sig == SIGIO)
  {
      printf("count = %d\n", timer_cnt);
      timer_cnt--;

      if(timer_cnt <= 0)
      {
        mt_unf_timer_release(timer_id);
      }
  }
}

static mt_s32 timer_cmd_run_help(void)
{
  printf("\n-h, --help		get run command information\n");
  printf("timer -c(reate) -i(nterval) (time) -l(oop) (0/1)  -r(ead) (timer id)\n");
  printf("      -c(reate)           create a timer\n");
  printf("      -i(nterval)         set the interval time, in ms\n");
  printf("      -l(oop)              set the count of loop time, if not set or 0, it will loop forever\n");
  printf("      -r(ead count)          read the timer count\n");
  printf("      \n");
  printf("Examples:\n");
  printf("1. timer -c -i 5000 -l 5 \n");
  printf("  describtion:create a 5000ms timer and loop 5 times\n");
  //printf("                   current max timer is 32s. it depends on the cpu frequency\n");
  return MT_SUCCESS;
}

int main(int argc, char *argv[])
{
    char oc;
    mt_s32 s32Ret;
    mt_u32 is_need_create = 0;
    mt_u32 interval = 0;
    mt_u32 readcount_flag = 0;
    mt_u32 loopcount = 1;
    mt_u32 uloop = 0;
    mt_u32 pu32Value = 0;
    mt_u32 loopout = 0;
    mt_s32 ps32TimerDevFd = 0;

	while ((oc = getopt(argc, argv, "ci:l:r")) != -1)
	{
		switch(oc) {
		case 'c':
			is_need_create = 1;
			break;
		case 'i':
			interval = atoi(optarg);
			break;
		case 'r':
			readcount_flag = 1;
			break;
		case 'l':
			loopcount =  atoi(optarg);
			break;
		case 'h':
		case '?':
			timer_cmd_run_help();
			return MT_SUCCESS;
		default:
			loopout = 1;
			break;
		}

		if (loopout == 1) {
			break;
		}
	}

   printf("%s %d \n",  __FUNCTION__, __LINE__);
    /* Open TIMER*/
    s32Ret = mt_unf_timer_init(&ps32TimerDevFd);
    if (MT_SUCCESS != s32Ret)
    {
        printf("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
		//TESTFM_SETRESULT(RET_FAIL, __LINE__, FALSE);
        return s32Ret;
    }

    /*creat a timer*/
    if(is_need_create)
    {
        timer_cnt = loopcount;
        printf("%s %d interval=%dms \n",	__FUNCTION__, __LINE__, interval);
        printf("%s %d loopcount =%d \n",	__FUNCTION__, __LINE__, loopcount);

        s32Ret = mt_unf_timer_request(interval, 1,  &timer_id);
        if (MT_SUCCESS != s32Ret)
        {
            printf("%s: %d ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
            goto ERR1;
        }
        else
        {
            printf("%s %d creat timer success, ID=%d \n", __FUNCTION__, __LINE__, timer_id);
         }

        //启动信号驱动机制
        signal(SIGIO, timer_default_callback);

         //F_SETOWN：设置将要在文件描述词fd上接收SIGIO
        //F_SETFL ：设置文件状态标志
        // F_GETFL ：读取文件状态标志。
        //FASYNC:   启用异步通知机制
        fcntl(ps32TimerDevFd, F_SETOWN, getpid());
        fcntl(ps32TimerDevFd, F_SETFL, fcntl(ps32TimerDevFd, F_GETFL) | FASYNC);
     }

    if(readcount_flag && (timer_id !=-1))
    {
         for (uloop = 0; uloop < 5; uloop++)
        {
            s32Ret = mt_unf_timer_read_cnt(timer_id, &pu32Value);
            if (MT_SUCCESS != s32Ret)
            {
                printf("%s: %d read cnt ErrorCode=0x%x\n", __FILE__, __LINE__, s32Ret);
                goto ERR1;
            }
            printf("Read timer[%d] cuont = %d\n", timer_id, pu32Value);
            sleep(1);
        }
    }

    while(1)
    {
         if(timer_cnt  <= 0)
         {
             //mt_unf_wdg_deinit();
             goto RESULT;
         }
         sleep(100);
    }

ERR1:
    //mt_unf_wdg_deinit();
RESULT:
	 if(s32Ret != MT_SUCCESS)
	 {
      //TESTFM_SETRESULT(RET_FAIL, err_code, FALSE);
	 }
    else
    {
      //TESTFM_SETRESULT(RET_SUCCESS, 0, FALSE);
    }
	return s32Ret;
}
