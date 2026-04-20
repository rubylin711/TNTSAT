/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __DRV_AI_COMMON_H__
#define __DRV_AI_COMMON_H__

#include "circ_buf.h"
#include "hal_aiao_common.h"
#include <sound/pcm.h>
#include "drv_ai_ext.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define AI_NAME "MT_AI"

#define AI_LATENCYMS_PERFRAME_DF (20)
#define AI_SAMPLE_PERFRAME_DF (1024) //(48000/1000*AI_LATENCYMS_PERFRAME_DF)
#define AI_BUFF_FRAME_NUM_DF (6 *2)

#define AI_I2S0_MSK (0)
#define AI_I2S1_MSK (1)
#define AI_ADAC_MSK (2)
#define AI_HDMI_MSK (3)

#define AI_OPEN_CNT_MAX (2)

#define AI_QUERY_BUF_CNT_MAX (AI_LATENCYMS_PERFRAME_DF * 6) //48k/8k = 6

//AI BUF ATTR
#if 0
typedef struct mtAI_BUF_ATTR_S
{
    mt_u32      u32Start;
    mt_u32      u32Read;
    mt_u32      u32Write;
    mt_u32      u32End;
    /* user space virtual address */
    mt_u32 u32UserVirBaseAddr;
    /* kernel space virtual address */
    mt_u32 u32KernelVirBaseAddr;
    //TO DO
    //MMZ Handle
    
} AI_BUF_ATTR_S;
#endif

#define AI_PATH_NAME_MAXLEN 256 //for proc
#define AI_FILE_NAME_MAXLEN 256

#define AI_STRING_SKIP_BLANK(str) \
    while (str[0] == ' ') {       \
	(str)++;                  \
    }

#define AI_PROC_SHOW_HELP(u32Ai)                                                          \
    do {                                                                                  \
	mt_drv_proc_echohelp("\nfunction: record pcm data from ai\n");                    \
	mt_drv_proc_echohelp("commad:   echo save start|stop > /proc/msp/ai%d\n", u32Ai); \
	mt_drv_proc_echohelp("example:  echo save start > /proc/msp/ai%d\n", u32Ai);      \
    } while (0)

typedef enum {
    AI_CMD_CTRL_STOP = 0,
    AI_CMD_CTRL_START,
    AI_CMD_CTRL_BUTT
} AI_CMD_CTRL_E;

typedef enum {
    AI_CMD_PROC_SAVE_AI = 0,
    AI_CMD_PROC_BUTT
} AI_CMD_PROC_E;

typedef struct
{
    mt_u32 u32BufPhyAddr;
    mt_u32 u32BufVirAddr;
    mt_u32 u32BufSize;
    mt_u32 u32PeriodByteSize;
    mt_u32 u32Periods;
} AI_ALSA_BUF_ATTR_S;

typedef struct mtAI_ALSA_Param_S
{
    AI_ALSA_BUF_ATTR_S stBuf; //for  alsa  mmap dma buffer
    AIAO_IsrFunc *IsrFunc;
    void *substream; //for alsa ISR func params
} AI_ALSA_Param_S;

typedef enum {
    AI_CHANNEL_STATUS_STOP = 0,
    AI_CHANNEL_STATUS_START,
    AI_CHANNEL_STATUS_CAST_BUTT,
} AI_CHANNEL_STATUS_E;

typedef struct
{
    mt_u32 u32AqcTryCnt;
    mt_u32 u32AqcCnt;
    mt_u32 u32RelTryCnt;
    mt_u32 u32RelCnt;
} AI_PROC_INFO_S;

typedef struct
{
    MT_UNF_AI_ATTR_S stSndPortAttr;
    MT_UNF_AI_E enAiPort;
    AIAO_PORT_ID_E enPort;
    AI_CHANNEL_STATUS_E enCurnStatus;
    mmz_buffer_s stRbfMmz;   //port mmz buf: 6 frames buf
    mmz_buffer_s stAiRbfMmz; //ai mmz buf:one frame buf
    AI_BUF_ATTR_S stAiBuf;   //the same as stAiRbfMmz In physics
    mt_u32 u32File;
    AI_PROC_INFO_S stAiProc;
    MT_BOOL bAttach;
    mt_handle hTrack;
    mt_u32 u32Rptr;
    mt_u32 u32Wptr;
    MT_BOOL bAlsa;
    mt_void *pAlsaPara;
    MT_UNF_AI_DELAY_S stDelayComps;
    /*save pcm*/
    AI_CMD_CTRL_E enSaveState;
    mt_u32 u32SaveCnt;
    struct file *fileHandle;
} AI_CHANNEL_STATE_S;

//AI
typedef struct mtAI_RESOURCE_S
{
    MT_UNF_AI_E enAIPortID; //AI Port ID
    //MT_UNF_AI_INPUTTYPE_E  enAIType;                //AI Type
    //CIRC_BUF_S               stCB;
    mmz_buffer_s stRbfMmz;

} AI_RESOURCE_S;

typedef struct tagAI_REGISTER_PARAM_S
{
    mt_proc_read_func pfnReadProc;
    mt_drv_proc_write_func pfnWriteProc;
} AI_REGISTER_PARAM_S;

//AI GLOABL RESOURCE
typedef struct mtAI_GLOBAL_RESOURCE_S
{
    mt_u32 u32BitFlag_AI; //resource usage such as  (1 << I2S | 1  << HDMI RX | 1 <<  ...)
    AI_CHANNEL_STATE_S *pstAI_ATTR_S[AI_MAX_TOTAL_NUM];
    AI_REGISTER_PARAM_S *pstProcParam; /* AI Proc functions */
    //to do
    AI_EXPORT_FUNC_S stExtFunc; /* AI provide extenal functions */
} AI_GLOBAL_RESOURCE_S;

/* private dev state Save AI Resource opened */
typedef struct mtAI_AOESTATE_S
{
    //ai
    mt_u32 *RecordId[AI_MAX_TOTAL_NUM];
    //todo

} AI_STATE_S;

typedef struct mtAI_PRIV_DATA_S
{
    struct clk *audinclk;
    struct clk *audinaxiclk;
} AI_PRIV_DATA_S;

mt_s32 AI_DRV_Open(struct inode *finode, struct file *ffile);
long AI_DRV_Ioctl(struct file *file, mt_u32 cmd, unsigned long arg);
mt_s32 AI_DRV_Release(struct inode *finode, struct file *ffile);
mt_s32 AI_DRV_Init(mt_void);
mt_void AI_DRV_Exit(mt_void);
mt_s32 AI_DRV_ReadProc(struct seq_file *p, mt_void *v);
mt_s32 AI_DRV_WriteProc(struct file *file, const char __user *buf, size_t count, loff_t *ppos);
mt_s32 AI_DRV_RegisterProc(AI_REGISTER_PARAM_S *pstParam);
mt_void AI_DRV_UnregisterProc(mt_void);
mt_s32 AI_DRV_Suspend(basedev_s *pdev, pm_message_t state);
mt_s32 AI_DRV_Resume(basedev_s *pdev);
mt_s32 AI_GetPortBuf(mt_handle hAi, AIAO_RBUF_ATTR_S *pstAiaoBuf);
mt_s32 AI_GetPortAttr(mt_handle hAi, AIAO_PORT_ATTR_S *pstPortAttr);
mt_s32 AI_SetAttachFlag(mt_handle hAi, mt_handle hTrack, MT_BOOL bAttachFlag);
mt_s32 AI_GetEnable(mt_handle hAi, MT_BOOL *pbEnable);
mt_s32 AI_SetEnable(mt_handle hAi, MT_BOOL bEnable, MT_BOOL bTrackResume);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif
