/******************************************************************************

  Copyright (C), 2001-2011, Montage Tech. Co., Ltd.

 ******************************************************************************
  File Name     : mplayer.c
  Version       : Initial Draft
  Author        : Montage multimedia software group
  Created       : 2010/01/26
  Description   :
  History       :
  1.Date        : 2010/01/26
    Author      : w58735
    Modification: Created file

******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>

#include <assert.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "../common/mt_adp_mpi.h"
#include "../common/mt_adp_demux.h"

#define TUNER_PORT      0
#define DMX_ID          0

static MT_BOOL g_bStopThread = MT_FALSE;  

static mt_handle dmxhandle;

static void dumpdata(mt_u8 *head,mt_u32 lens)
{
    mt_u32 i;
    printf("data:\n");
    for(i=0;i<lens;i++){
        printf("%02x ",head[i]);
        if(7==(i&7)){
            printf("\n");
        }
    }
    printf("\n");
}

static mt_s32 CatchTs(void)
{
    mt_u32 dumpsz=0;
    mt_u32 i;
    mt_u32 u32AcquireNum = 10;
    mt_u32 pu32AcquiredNum;
    MT_UNF_DMX_DATA_S pstBuf[10];
    mt_u32 u32HandleNum = 32;
    mt_s32 ret;

    while(!g_bStopThread)
    {
        ret = MT_UNF_DMX_AcquireBuf(dmxhandle,u32AcquireNum,(mt_u32 *)(&pu32AcquiredNum),pstBuf,(mt_u32)5000);
        if(MT_SUCCESS != ret) {
            printf("MT_UNF_DMX_AcquireBuf failed!\n");
            MT_USLEEP(10 * 1000);
            continue;
        }
        for(i=0;i<pu32AcquiredNum;i++){
            if(pstBuf[i].u32Size>16){
                dumpsz=16;
            }else{
                dumpsz=pstBuf[i].u32Size;
            }
            printf("handle:%x,filtid:%x,size=%x,data16:\n",dmxhandle,pstBuf[i].filthandle,pstBuf[i].u32Size);
            dumpdata(pstBuf[i].pu8Data,dumpsz);
        }
        ret = MT_UNF_DMX_ReleaseBuf(dmxhandle,pu32AcquiredNum,pstBuf);
        if (MT_SUCCESS != ret){
            printf("call MT_UNF_DMX_ReleaseBuf failed!\n");
        }
    }
    printf("receive over!\n");
    return MT_SUCCESS;
}

static mt_s32 CatchPes(void)
{
    mt_u32 i;
    mt_handle dathandle[128]={0};
    mt_u32 dat_cnt=128;
    MT_UNF_ES_BUF_S esinfo={0};

    while(!g_bStopThread){
        dat_cnt=128;
        if(MT_SUCCESS!=MT_UNF_DMX_GetDataHandle(dathandle,&dat_cnt,1000)){
            printf("call MT_UNF_DMX_SelectDataHandle failed!\n");
            MT_USLEEP(10 * 1000);
            continue;
        }
        for(i=0;i<dat_cnt;i++){
            if(MT_SUCCESS==MT_UNF_DMX_AcquireEs(dathandle[i],&esinfo)){
                dumpdata(esinfo.pu8Buf,esinfo.u32BufLen);
                MT_UNF_DMX_ReleaseEs(dathandle[i],&esinfo);
            }else{
                printf("+++AcquireEs err\n");
            }
        }
    }
    printf("receive over!\n");
    return MT_SUCCESS;
}

/*
func:this sample show us how to catch ts or pes
s:
mt_tuner -o .open 0
mt_tuner -o .setpara 0 2 34 88 280 24 1 0
mt_tuner -o .slock 0 3825 6780 0 1 0 0
*/
mt_s32 main(mt_s32 argc,mt_char *argv[])
{//./filter_sample 538 6875 64 pid pes/ts
    mt_s32                      Ret;
    MT_UNF_DMX_CHAN_ATTR_S      stChnAttr;
    mt_u32 freq, symbol_rate, qam_mode;
    mt_sys_version_s stSysChipInfo;
    mt_u32 tspes_pid = 0;
    mt_u32 is_tstype = 0;
    mt_char                     InputCmd[128]={0};
    pthread_t g_TsThd1;

    if(6 != argc){
        printf( "usage:./pg fre sysm qam pid type\n"
                "./filter_sample 378 6875 64 0 ts\n"
                "./filter_sample 306 6875 64 2313 pes\n"
                "./filter_sample 4150 27500 0 0 ts\n"
                "./filter_sample 506 0 8 0 ts\n");
        return -1;
    }
    mt_sys_init();
    Ret = MT_UNF_DMX_Init();
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_Init failed.\n");
        goto SYS_DEINIT;
    }

    freq = (mt_u32)strtol(argv[1], NULL, 0);
    symbol_rate = (mt_u32)strtol(argv[2], NULL, 0);
    qam_mode = (mt_u32)strtol(argv[3], NULL, 0);
    tspes_pid = (mt_u32)strtol(argv[4], NULL, 0);
    if(!strcasecmp("ts", argv[5])){
        is_tstype=1;
    }
    printf("argv=%d,%d,%d,%d,%d\n",freq,symbol_rate,qam_mode,tspes_pid,is_tstype);
    mtadp_fe_init();
    if ((16<=qam_mode) && (256>=qam_mode)){     //dvbc
        mtadp_fe_connect(0, freq, symbol_rate, qam_mode);
    }else if(0<qam_mode){                       //dvbt.t2
        mtadp_fe_connect_dvbtauto(0,freq,qam_mode);
    }else{
        mtadp_fe_connect_dvbs(0,freq, symbol_rate,0, 0, 0);
    }
#ifdef CONFIG_MT_CHIP_ARIA
    Ret |= MT_UNF_DMX_AttachTSPort(0, MT_UNF_DMX_PORT_TSI_2);
#else
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);
    if (0<qam_mode){                                                //dvbc.T.T2
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.C.T.T2
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_1);
        }else{                                                      //sym1.C.T.T2
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);
        }
    }else{
        if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion){   //sym2.S
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_0);
        }else{                                                      //sym1.S
            Ret |= MT_UNF_DMX_AttachTSPort(DMX_ID, MT_UNF_DMX_PORT_TSI_1);
        }
    }
#endif  
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        goto DMX_DEINIT;
    }

    Ret = MT_UNF_DMX_GetChannelDefaultAttr(&stChnAttr);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_GetChannelDefaultAttr failed!\n");
        goto DMX_DEINIT;
    }
    if(0==is_tstype){
        stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_PES;
    }else{
        stChnAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_POST;
    }
    stChnAttr.u32BufSize = (188*1024);
    stChnAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    Ret = MT_UNF_DMX_CreateChannel(DMX_ID,&stChnAttr,&dmxhandle);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_CreateChannel failed!\n");
        goto DMX_DEINIT;
    }
    Ret = MT_UNF_DMX_SetChannelPID(dmxhandle,tspes_pid);
    Ret |= MT_UNF_DMX_OpenChannel(dmxhandle);
    if (MT_SUCCESS != Ret){
        printf("call MT_UNF_DMX_OpenChannel failed!\n");
        goto CHN_DESTROY;
    }    

    if(0==is_tstype){
        pthread_create(&g_TsThd1, MT_NULL, (mt_void *)CatchPes, MT_NULL);
    }else{
        pthread_create(&g_TsThd1, MT_NULL, (mt_void *)CatchTs, MT_NULL);
    }

    while(1){
        printf("please input 'q' to quit!\n");
        memset(InputCmd, 0, 128);
        SAMPLE_GET_INPUTCMD(InputCmd);
        if ('q' == InputCmd[0]){
            printf("prepare to quit!\n");
            break;
        }
    }
    g_bStopThread = MT_TRUE;

    pthread_join(g_TsThd1, MT_NULL);
    
    CHN_DESTROY:
        MT_UNF_DMX_CloseChannel(dmxhandle);
        MT_UNF_DMX_DestroyChannel(dmxhandle);
    DMX_DEINIT:
        MT_UNF_DMX_DetachTSPort(DMX_ID);
        MT_UNF_DMX_DeInit();
    SYS_DEINIT:
    mtadp_fe_deinit();
    mt_sys_deinit();
    return Ret;
}

