/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/vmalloc.h>
#include <linux/interrupt.h>
#include <linux/seq_file.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <asm/io.h>
//#include <mach/hardware.h>

#include "mt_unf_avplay.h"
#include "mt_error_mpi.h"

#include "vconfig.h"
#include "vfmw.h"
#include "mt_drv_vdec_ioctl.h"
#include "mt_drv_vdec.h"
#include "drv_vdec_private.h"
#include "mt_drv_stat.h"
#include "mt_module_debug.h"
#include "drv_vdec_debug.h"
#include "vfmw_reg.h"

#define MAX_VID_PROTOCOL_NAME 20
#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */

static mt_s32   VDEC_DRV_CtrlWriteProc(struct file * file,
                                   const char __user * buf, size_t count, loff_t *ppos);
static mt_s32   VDEC_DRV_CtrlReadProc(struct seq_file *p, mt_void *v);

static mt_s32   VDEC_DRV_WriteProc(struct file * file,
                                   const char __user * buf, size_t count, loff_t *ppos);
static mt_s32   VDEC_DRV_ReadProc(struct seq_file *p, mt_void *v);

static mt_s32 VDEC_DRV_VPUReadProc(struct seq_file *p, mt_void *v);
static mt_s32 VDEC_DRV_VPUWriteProc(struct file * file, const char __user * buffer, size_t count, loff_t *ppos);
SINT32 KERN_VDEC_Control(SINT32 ChanID, VDEC_CID_E eCmdID, VOID *pArgs);

void VPU_DecodeStat2Str(mt_u32 eDecStat, char *strVidStd);
void VPU_InstMode2Str(mt_u32 eInstMode, char *strVidStd);
void VPU_DecodeMode2Str(mt_u32 eDecMode, char *strVidStd);
static long     VDEC_DRV_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

mt_u32 vdec_descriptor_get_write_ptr(mt_u32 ch);
mt_u32 vdec_descriptor_get_read_ptr(mt_u32 ch);

static mt_char *s_aszVdecType[MT_UNF_VCODEC_TYPE_BUTT + 1] =
{
    "MPEG2",
    "MPEG4",
    "AVS",
    "H263",
    "H264",
    "REAL8",
    "REAL9",
    "VC1",
    "VP6",
    "VP6F",
    "VP6A",
    "OTHER",	//MJPEG
    "SORENSON",
    "DIVX3",
    "RAW",
    "JPEG",
    "VP8",
    "OTHER",	//MSMPEG4V1
    "OTHER",	//MSMPEG4V2
    "OTHER",	//MSVIDEO1
    "OTHER",	//WMV1
    "OTHER",	//WMV2
    "OTHER",	//RV10
    "OTHER",	//RV20
    "OTHER",	//SVQ1
    "OTHER",	//SVQ3
    "OTHER",	//H261
    "OTHER",	//VP3
    "OTHER",	//VP5
    "OTHER",	//CINEPAK
    "OTHER",	//INDEO2
    "OTHER",	//INDEO3
    "OTHER",	//INDEO4
    "OTHER",	//INDEO5
    "OTHER",	//MJPEGB
    "OTHER",	//MVC
    "HEVC",
	"OTHER",	//DV
	"VP9",
    "UNKNOWN",
};

static struct file_operations VdecFileOpts = {
    .owner          = THIS_MODULE,
    .open           = VDEC_DRV_Open,
    .unlocked_ioctl = VDEC_DRV_Ioctl,
    .release        = VDEC_DRV_Release,
};

static baseops_s VdecOps = {
    .probe          = NULL,
    .remove         = NULL,
    .shutdown       = NULL,
    .prepare        = NULL,
    .complete       = NULL,
    .suspend        = VDEC_DRV_Suspend,
    .suspend_late   = NULL,
    .resume_early   = NULL,
    .resume         = VDEC_DRV_Resume,
};

static VDEC_REGISTER_PARAM_S s_stProcParam = {
    .pfnCtrlReadProc  = VDEC_DRV_CtrlReadProc,
    .pfnCtrlWriteProc = VDEC_DRV_CtrlWriteProc,
    .pfnReadProc      = VDEC_DRV_ReadProc,
    .pfnWriteProc     = VDEC_DRV_WriteProc,
    .pfnVpuReadProc   = VDEC_DRV_VPUReadProc,
    .pfnVpuWriteProc  = VDEC_DRV_VPUWriteProc,
};

/* save raw/yuv param */
static mt_s8  VdecSavePath[256] = {'/','m','n','t',0};
extern mt_u32 MaskCtrlWord;
extern mt_s32 VdecRawChanNum;
extern mt_s32 VdecYuvChanNum;
extern int g_vlog_level;

MT_BOOL bSaveoneyuv  = MT_FALSE;
mt_u32  u32SaveCnt = 0;
static long VDEC_DRV_Ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    int ret;

    ret = (long)mt_drv_usercopy(filp->f_path.dentry->d_inode, filp, cmd, arg, VDEC_Ioctl);
    return ret;
}

static __inline__ int str2val(char *str, ulong *data)
{
    ulong i, d, dat, weight;

    dat = 0;
    if(str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        i = 2;
        weight = 16;
    }
    else
    {
        i = 0;
        weight = 10;
    }

    for(; i < 10; i++)
    {
        if(str[i] < 0x20)break;
        else if (weight == 16 && str[i] >= 'a' && str[i] <= 'f')
        {
            d = str[i] - 'a' + 10;
        }
        else if (weight == 16 && str[i] >= 'A' && str[i] <= 'F')
        {
            d = str[i] - 'A' + 10;
        }
        else if (str[i] >= '0' && str[i] <= '9')
        {
            d = str[i] - '0';
        }
        else
        {
            return -1;
        }

        dat = dat * weight + d;
    }

    *data = dat;

    return 0;
}

static void Proc_Dump_Register(struct seq_file *p, char *module, ulong reg_base, unsigned int range)
{
	unsigned int reg_offset;

    PROC_PRINT(p, "--------------------------Dump %s Registers-------------------------", module);
	for (reg_offset=0; reg_offset<range; reg_offset+=4)
	{
		if ((reg_offset%16) == 0)
		{
//			PROC_PRINT(p, "\n0x%08x : ", reg_base+reg_offset);
		}
		//PROC_PRINT(p, "%08x ", VFMW_REG_READ(reg_base+reg_offset));
	}
    PROC_PRINT(p, "\n");
}

static mt_s32 VDEC_DRV_CtrlReadProc(struct seq_file *p, mt_void *v)
{
    PROC_PRINT(p, "\n");
    PROC_PRINT(p, "%-35s:%d\n", "VDEC VERSION",   VDEC_VERSION);
    //PROC_PRINT(p, "%-35s:%d\n", "MaskCtrlWord",   MaskCtrlWord);
    PROC_PRINT(p, "%-35s:%s\n", "VdecSavePath",   VdecSavePath);
    PROC_PRINT(p, "%-35s:%d\n", "VdecRawChanNum", VdecRawChanNum);
    PROC_PRINT(p, "%-35s:%d\n", "VdecYuvChanNum", VdecYuvChanNum);
    PROC_PRINT(p, "%-35s:%d\n", "VdecDumpESFlag", vdec_dump_es_get_enable());
    PROC_PRINT(p, "%-35s:%s\n", "VdecDumpESFileName", vdec_dump_es_get_file_name());
    PROC_PRINT(p, "\n");
    PROC_PRINT(p, "======================= param in () is not necessary ========================\n");
    PROC_PRINT(p, "echo  dat0        dat1      dat2   > /proc/msp/vdec_ctrl\n");
    PROC_PRINT(p, "echo  savestream  handle   (path)  -- turn on/off raw save\n");
    PROC_PRINT(p, "echo  saveyuv     handle   (path)  -- turn on/off yuv save\n");
    PROC_PRINT(p, "echo  saveoneyuv  handle   (path)  -- turn on/off oneyuv save\n");
    //PROC_PRINT(p, "echo  maskdfs     on/off           -- enable/disable mask dynamic fs\n");
	PROC_PRINT(p, "echo  dumpstat    on/off           -- turn on/off dump video es buffer state\n");
	PROC_PRINT(p, "echo  dumpes      on/off   (file)  -- turn on/off dump video es\n");
	PROC_PRINT(p, "echo  loglevel    [0-5]            -- set vdec drv log level\n");
    PROC_PRINT(p, "=============================================================================\n");

    return 0;
}

static mt_s32 VDEC_DRV_GetProcArg(mt_char*  chCmd,mt_char*  chArg,mt_u32 u32ArgIndex)
    {
        mt_u32 u32Count;
        mt_u32 u32CmdCount;
        mt_u32 u32LogCount;
        mt_u32 u32NewFlag;
        mt_char chArg1[VDEC_MAX_PROC_ARGS_SIZE] = {0};
        mt_char chArg2[VDEC_MAX_PROC_ARGS_SIZE] = {0};
        mt_char chArg3[VDEC_MAX_PROC_ARGS_SIZE*8] = {0};
        u32CmdCount = 0;

        /*clear empty space*/
        u32Count = 0;
        u32CmdCount = 0;
        u32LogCount = 1;
        u32NewFlag = 0;
        while(chCmd[u32Count] != 0 && chCmd[u32Count] != '\n' )
        {
            if (chCmd[u32Count] != ' ')
            {
                u32NewFlag = 1;
            }
            else
            {
                if(u32NewFlag == 1)
                {
                    u32LogCount++;
                    u32CmdCount= 0;
                    u32NewFlag = 0;
                }
            }

            if (u32NewFlag == 1)
            {
                switch(u32LogCount)
                {
                    case 1:
                        chArg1[u32CmdCount] = chCmd[u32Count];
                        u32CmdCount++;
                        break;
                    case 2:
                        chArg2[u32CmdCount] = chCmd[u32Count];
                        u32CmdCount++;
                        break;
                    case 3:
                        chArg3[u32CmdCount] = chCmd[u32Count];
                        u32CmdCount++;
                        break;
                    default:
                        break;
                }

            }
            u32Count++;
        }

        switch(u32ArgIndex)
        {
            case 1:
                memcpy(chArg,chArg1,sizeof(mt_char)*VDEC_MAX_PROC_ARGS_SIZE);
                break;
            case 2:
                memcpy(chArg,chArg2,sizeof(mt_char)*VDEC_MAX_PROC_ARGS_SIZE);
                break;
            case 3:
                memcpy(chArg,chArg3,sizeof(mt_char)*VDEC_MAX_PROC_ARGS_SIZE*8);
                break;
            default:
                break;
        }
        return MT_SUCCESS;
    }

static mt_s32 VDEC_DRV_CtrlWriteProc(struct file * file,
                                 const char __user * buffer, size_t count, loff_t *ppos)
{
    mt_char dat1[VDEC_MAX_PROC_ARGS_SIZE];
    mt_char dat2[VDEC_MAX_PROC_ARGS_SIZE];
    mt_char dat3[VDEC_MAX_PROC_ARGS_SIZE*8];
    static mt_char buf[256];
    mt_handle handle = 0;
    if(count >= sizeof(buf))
    {
        MT_FATAL_VDEC("FATAL: parameter string is too long!\n");
        return 0;
    }

    memset(buf, 0, sizeof(buf));
    if (copy_from_user(buf, buffer, count))
    {
        MT_FATAL_VDEC("FATAL: copy_from_user failed!\n");
        return 0;
    }
    VDEC_DRV_GetProcArg(buf,dat1,1);
    VDEC_DRV_GetProcArg(buf,dat2,2);
    VDEC_DRV_GetProcArg(buf,dat3,3);

    if(0==strncmp(dat1,"help",4))
    {
        mt_drv_proc_echohelp("================================== VDEC CTRL INFO ===============================\n");
        mt_drv_proc_echohelp("========================== param in () is not necessary =========================\n");
        mt_drv_proc_echohelp("echo  dat0           dat1      dat2   > /proc/msp/vdec_ctrl\n");
        mt_drv_proc_echohelp("echo  savestream     handle    (path)  -- turn on/off raw save\n");
        mt_drv_proc_echohelp("echo  saveyuv        handle    (path)  -- turn on/off yuv save\n");
        mt_drv_proc_echohelp("echo  saveoneyuv     handle    (path)  -- turn on/off oneyuv save\n");
        //mt_drv_proc_echohelp("echo  maskdfs        on/off            -- enable/disable mask dynamic fs\n");
        mt_drv_proc_echohelp("echo  dumpstat       on/off            -- turn on/off dump video es buffer state\n");
        mt_drv_proc_echohelp("echo  dumpes         on/off    (file)  -- turn on/off dump video es\n");
        mt_drv_proc_echohelp("echo  loglevel       [0-5]             -- set vdec drv log level\n");
        mt_drv_proc_echohelp("==================================================================================\n");
    }
    else if(0==strncmp(dat1,"savestream",10))
    {
        if(dat2[0]!=0)
        {
            str2val(dat2,&handle);
            if(handle >= MT_VDEC_MAX_INSTANCE_NEW)
            {
                MT_ERR_VDEC("ERROR:to large handle %x.\n",handle);
                return 0;
            }
        }

        if(dat3[0] !=0)
        {
            strncpy(VdecSavePath, dat3, sizeof(VdecSavePath));
            VdecSavePath[sizeof(VdecSavePath)-1]='\0';
        }

        if(MT_FALSE == BUFMNG_CheckFile(handle, 0))
        {
            mt_s8 str[80];

#if 0
            UINT32 Hour;
            UINT32 Minute;
            UINT32 Second;
            struct timeval CurrentTime;

            do_gettimeofday(&CurrentTime);
            Hour = CurrentTime.tv_sec /3600;
            Minute = (CurrentTime.tv_sec - (Hour *3600))/60;
            Second = CurrentTime.tv_sec % 60;

            snprintf(str,sizeof(str),"%s/vdec%02d_%02d_%02d_%02d.es",(char *)VdecSavePath, handle,Hour,Minute,Second);
#else
			time64_t kt;
			kt = ktime_get_seconds();
			snprintf(str,sizeof(str),"%s/vdec%02ld_%llx.es",(char *)VdecSavePath, handle, kt);

#endif

            if (BUFMNG_OpenFile(handle, str, 0) != MT_SUCCESS)
            {
                MT_FATAL_VDEC("FATAL: failed create file '%s' for raw stream save!\n",str);
            }
            else
            {
            }
        }
        else if (MT_TRUE == BUFMNG_CheckFile(handle, 0))
        {
            if (BUFMNG_CloseFile(handle, 0) != MT_SUCCESS)
            {
                MT_FATAL_VDEC("FATAL: failed close file for vdec%2d raw stream save!\n", handle);
            }
            else
            {
            }
        }
    }
    else if(0==strncmp(dat1,"saveyuv",7))
    {
        if(dat2[0]!=0)
        {
            str2val(dat2,&handle);
            if(handle >= MT_VDEC_MAX_INSTANCE_NEW)
            {
                MT_ERR_VDEC("ERROR:to large handle %x.\n",handle);
                return 0;
            }
        }

        if(dat3[0] !=0)
        {
            strncpy(VdecSavePath, dat3, sizeof(VdecSavePath));
            VdecSavePath[sizeof(VdecSavePath)-1]='\0';
        }

        bSaveoneyuv = MT_FALSE;
        if(MT_FALSE == BUFMNG_CheckFile(handle, 1))
        {
            mt_s8 str[80];

#if 0			
            UINT32 Hour;
            UINT32 Minute;
            UINT32 Second;
            struct timeval CurrentTime;

            do_gettimeofday(&CurrentTime);
            Hour = CurrentTime.tv_sec /3600;
            Minute = (CurrentTime.tv_sec - (Hour *3600))/60;
            Second = CurrentTime.tv_sec % 60;

            snprintf(str,sizeof(str),"%s/vdec%02d_%02d_%02d_%02d.yuv",(char *)VdecSavePath, handle,Hour,Minute,Second);
#else
	time64_t kt;
	kt = ktime_get_seconds();
	snprintf(str,sizeof(str),"%s/vdec%02ld_%llx.es",(char *)VdecSavePath, handle, kt);

#endif
            if (BUFMNG_OpenFile(handle, str, 1) != MT_SUCCESS)
            {
                MT_FATAL_VDEC("FATAL: failed create file '%s' for yuv save!\n",str);
            }
            else
            {
            }
        }
        else if (MT_TRUE == BUFMNG_CheckFile(handle, 1))
        {
            if (BUFMNG_CloseFile(handle, 1) != MT_SUCCESS)
            {
                MT_FATAL_VDEC("FATAL: failed close file for vdec%2d yuv save!\n", handle);
            }
            else
            {
            }
        }
    }
    else if(0==strncmp(dat1,"saveoneyuv",10))
    {
        if(dat2[0]!=0)
        {
            str2val(dat2,&handle);
            if(handle >= MT_VDEC_MAX_INSTANCE_NEW)
            {
                MT_ERR_VDEC("ERROR:to large handle %x.\n",handle);
                return 0;
            }
        }

        if(dat3[0] !=0)
        {
            strncpy(VdecSavePath, dat3, sizeof(VdecSavePath));
            VdecSavePath[sizeof(VdecSavePath)-1]='\0';
        }

        bSaveoneyuv = MT_TRUE;
        u32SaveCnt = 0;
        if(MT_FALSE == BUFMNG_CheckFile(handle, 1))
        {
            mt_s8 str[80];
			time64_t kt;
			kt = ktime_get_seconds();
			snprintf(str,sizeof(str),"%s/vdec%02ld_%llx.es",(char *)VdecSavePath, handle, kt);

            if (BUFMNG_OpenFile(handle, str, 1) != MT_SUCCESS)
            {
                MT_FATAL_VDEC("FATAL: failed create file '%s' for yuv save!\n",str);
            }
        }
    }
    else if(0==strncmp(dat1,"maskdfs",10))
    {
        if(0==strncmp(dat2,"on",10))
        {
            MaskCtrlWord = MaskCtrlWord|0x1;
            mt_drv_proc_echohelp("Mask dynamic fs.\n");
        }
        else if(0==strncmp(dat2,"off",10))
        {
            MaskCtrlWord = MaskCtrlWord&0xFFFFFFFE;
            mt_drv_proc_echohelp("Unmask dynamic fs.\n");
        }
        else
        {
        }
    }
    else if (0 == strncmp(dat1, "dumpstat", 11))
    {
        if (0 == strncmp(dat2, "on", 10))
        {
			g_VDebugEnable = 1;
        }
        else if (0 == strncmp(dat2, "off", 10))
        {
			g_VDebugEnable = 0;
        }
        else
        {
        }
	}
    else if (0 == strncmp(dat1, "dumpes", 10))
    {
        if (0 == strncmp(dat2, "on", 10))
        {
			vdec_dump_es_set_enable(dat3);
        }
        else if (0 == strncmp(dat2, "off", 10))
        {
			vdec_dump_es_set_disable();
        }
        else
        {
        }
	}
    else if (0 == strncmp(dat1, "loglevel", 8))
    {
        //g_vlog_level = (int)strtol(dat2);
        g_vlog_level = (int)(dat2[0] - '0');
	}
    else
    {
        MT_FATAL_VDEC("FATAL: unkown echo cmd '%d'!\n");
    }

    return count;
}

static mt_s32 VDEC_DRV_WriteProc(struct file * file,
                                 const char __user * buffer, size_t count, loff_t *ppos)
{
    struct seq_file *q = file->private_data;
    mt_proc_entry_t *pProcItem = q->private;
    mt_char dat1[VDEC_MAX_PROC_ARGS_SIZE];
    mt_char dat2[VDEC_MAX_PROC_ARGS_SIZE];
    mt_char dat3[VDEC_MAX_PROC_ARGS_SIZE*8];
    static mt_char buf[256];
    mt_handle hHandle = 0;
    mt_s32 s32Ret = MT_FAILURE;
    VDEC_CHANNEL_S *pstChan = MT_NULL;

    hHandle = (pProcItem->entry_name[4]-48)*10 + ((pProcItem->entry_name[5]) - 48);
    /**
    s32Ret = sscanf(pProcItem->entry_name, "vdec%02d", &hHandle);
    if(s32Ret <=0)
    {
       MT_ERR_VDEC("Invalid VDEC ID.\n");
       return 0;
    }**/
    pstChan = VDEC_DRV_GetChan(hHandle);
    if(MT_NULL == pstChan)
    {
        MT_ERR_VDEC("ERR: chan %d is not init!\n",hHandle);
        return 0;
    }

    hHandle = (MT_ID_VDEC << 16) | hHandle;
    if(count >= sizeof(buf))
    {
        MT_FATAL_VDEC("FATAL: parameter string is too long!\n");
        return 0;
    }

    memset(buf, 0, sizeof(buf));
    if (copy_from_user(buf, buffer, count))
    {
        MT_FATAL_VDEC("FATAL: copy_from_user failed!\n");
        return 0;
    }
    VDEC_DRV_GetProcArg(buf,dat1,1);
    VDEC_DRV_GetProcArg(buf,dat2,2);
    VDEC_DRV_GetProcArg(buf,dat3,3);

    if(0==strncmp(dat1,"help",4))
    {
        mt_drv_proc_echohelp("================== VDEC INFO ==================\n");
        mt_drv_proc_echohelp("=========== xx is chan num===========\n");
        mt_drv_proc_echohelp("echo  dat0             dat1    > /proc/msp/vdec[xx]\n");
        mt_drv_proc_echohelp("echo  low_delay_stat   start    -- start low_delay_stat\n");
        mt_drv_proc_echohelp("echo  low_delay_stat   stop     -- stop low_delay_stat\n");
        mt_drv_proc_echohelp("====================================================\n");
    }
    else if(0==strncmp(dat1,"low_delay_stat",14))	/* not support */
    {
         if(0==strncmp(dat2,"start",5))
         {
             //s32Ret = VDEC_IPC_Control(pstChan->hChan, VDEC_CID_START_LOWDLAY_CALC, &hHandle);//Rock_hu
             if (VDEC_OK != s32Ret)
             {
                 MT_ERR_VDEC("VFMW CFG_CHAN err!\n");
                 return MT_FAILURE;
             }
             mt_drv_ld_start_statistics(SCENES_VID_PLAY, (mt_void*)hHandle);
         }
         else if (0==strncmp(dat2,"stop",4))
         {
             //s32Ret = VDEC_IPC_Control(pstChan->hChan, VDEC_CID_STOP_LOWDLAY_CALC, &hHandle);//Rock_hu
             if (VDEC_OK != s32Ret)
             {
                 MT_ERR_VDEC("VFMW CFG_CHAN err!\n");
                 return MT_FAILURE;
             }
             mt_drv_ld_stop_statistics();
         }
    }
    else
    {
        MT_FATAL_VDEC("FATAL: unkown echo cmd '%d'!\n");
    }

    return count;
    //return MT_DRV_PROC_ModuleWrite(file, buffer, count, ppos, VDEC_DRV_DebugCtrl);
}

static mt_s32 VDEC_DRV_ReadProc(struct seq_file *p, mt_void *v)
{
    mt_s32 i;
	mt_u32 rd = 0;
	mt_u32 wt = 0;
	mt_u32 tmp = 0;
    mt_s32 s32Ret;
	VDEC_CHAN_STATE_S v_ch_st;
    VDEC_CHANNEL_S *pstChan;
    VDEC_CHAN_STATINFO_S *pstStatInfo;
    mt_proc_entry_t *pstProcItem;
    BUFMNG_STATUS_S stBMStatus = {0};
    mt_char aszDecMode[32];
    mt_char aszDisplayNorm[32];
    mt_char aszSampleType[32];
    mt_char aszYUVType[32];
    mt_char aszTopFirst[16];
    mt_char aszUserRatio[16];
    mt_char aszDecodeRatio[16];
    mt_char aszFrmPackingType[16];
    mt_char aszFieldMode[16];
    mt_char aszDecType[10];
    mt_char aszCapLevel[10];
    mt_char aszProtocolLevel[10];
    mt_handle hVpss = MT_INVALID_HANDLE;
    MT_DIS_FRAME_SLOT_INFO_T *pSlotInfo;

    pstProcItem = p->private;

    if (0 == strncmp(pstProcItem->entry_name, "vdec_ctrl",9))
    {
        return 0;
    }

    s32Ret = sscanf(pstProcItem->entry_name, "vdec%02d", &i);
    if(s32Ret <=0)
    {
       PROC_PRINT(p, "Invalid VDEC ID.\n");
       return 0;
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        PROC_PRINT(p, "Invalid VDEC ID:%d.\n", i);
        return 0;
    }

    pstChan = VDEC_DRV_GetChan(i);
    if (pstChan)
    {
        pstStatInfo = &(pstChan->stStatInfo);
        switch (pstChan->stCurCfg.enMode)
        {
        case MT_UNF_VCODEC_MODE_I:
            snprintf(aszDecMode, sizeof(aszDecMode), "I");
            break;
        case MT_UNF_VCODEC_MODE_IP:
            snprintf(aszDecMode, sizeof(aszDecMode), "IP");
            break;
        case MT_UNF_VCODEC_MODE_NORMAL:
            snprintf(aszDecMode, sizeof(aszDecMode), "NORMAL");
            break;
        default:
            snprintf(aszDecMode, sizeof(aszDecMode), "UNKNOWN(%d)", pstChan->stCurCfg.enMode);
            break;
        }

        switch (pstChan->enDisplayNorm)
        {
        case MT_UNF_ENC_FMT_PAL:
            snprintf(aszDisplayNorm, sizeof(aszDisplayNorm), "PAL");
            break;
        case MT_UNF_ENC_FMT_NTSC:
            snprintf(aszDisplayNorm, sizeof(aszDisplayNorm), "NTSC");
            break;
        default:
            snprintf(aszDisplayNorm, sizeof(aszDisplayNorm), "OTHER(%d)", pstChan->enDisplayNorm);
            break;
        }

        if (pstChan->stLastFrm.bProgressive)
        {
            snprintf(aszSampleType, sizeof(aszSampleType), "Progressive");
        }
        else
        {
            snprintf(aszSampleType, sizeof(aszSampleType), "Interlace");
        }

        snprintf(aszUserRatio, sizeof(aszUserRatio),  "%d:%d", pstChan->u32UserSetAspectWidth, pstChan->u32UserSetAspectHeight);
        snprintf(aszDecodeRatio, sizeof(aszDecodeRatio),  "%d:%d", pstChan->u32DecodeAspectWidth, pstChan->u32DecodeAspectHeight);
        switch(pstChan->eFramePackType)
        {
        case MT_UNF_FRAME_PACKING_TYPE_NONE:
            snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"2D");
            break;
        case MT_UNF_FRAME_PACKING_TYPE_SIDE_BY_SIDE:
            snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"SBS");
            break;
        case MT_UNF_FRAME_PACKING_TYPE_TOP_AND_BOTTOM:
            snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"TAB");
            break;
        case MT_UNF_FRAME_PACKING_TYPE_TIME_INTERLACED:
            snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"MVC");
            break;
		case MT_UNF_FRAME_PACKING_TYPE_FRAME_PACKING:
			snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"Frame Packing");
			break;
		case MT_UNF_FRAME_PACKING_TYPE_3D_TILE:
			snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"TILE 3D");
			break;
        case MT_UNF_FRAME_PACKING_TYPE_BUTT:
            default:
            snprintf(aszFrmPackingType,sizeof(aszFrmPackingType),"Not Set");
            break;
        }
        switch (pstChan->stLastFrm.enFieldMode)
        {
        case MT_DRV_FIELD_ALL:
            snprintf(aszFieldMode, sizeof(aszFieldMode), "Frame");
            break;
        case MT_DRV_FIELD_TOP:
            snprintf(aszFieldMode, sizeof(aszFieldMode), "Top");
            break;
        case MT_DRV_FIELD_BOTTOM:
            snprintf(aszFieldMode, sizeof(aszFieldMode), "Bottom");
            break;
        default:
            snprintf(aszFieldMode, sizeof(aszFieldMode), "UNKNOWN");
            break;
        }

        switch (pstChan->stLastFrm.ePixFormat)
        {
        case MT_DRV_PIX_FMT_NV08:
            snprintf(aszYUVType, sizeof(aszYUVType), "SP400");
            break;
        case MT_DRV_PIX_FMT_NV12_411:
            snprintf(aszYUVType, sizeof(aszYUVType), "SP411");
            break;
        case MT_DRV_PIX_FMT_NV21:
        case MT_DRV_PIX_FMT_NV21_TILE:
            snprintf(aszYUVType, sizeof(aszYUVType), "SP420");
            break;
        case MT_DRV_PIX_FMT_NV16:
            snprintf(aszYUVType, sizeof(aszYUVType), "SP422_1X2");
            break;
        case MT_DRV_PIX_FMT_NV16_2X1:
            snprintf(aszYUVType, sizeof(aszYUVType), "SP422_2X1");
            break;
        case MT_DRV_PIX_FMT_NV24:
            snprintf(aszYUVType, sizeof(aszYUVType), "SP444");
            break;
        /*case MT_UNF_FORMAT_YUV_PACKAGE_UYVY:
            sprintf(aszYUVType, "Package_UYVY");
            break;
        case MT_UNF_FORMAT_YUV_PACKAGE_YUYV:
            sprintf(aszYUVType, "Package_YUYV");
            break;
        case MT_UNF_FORMAT_YUV_PACKAGE_YVYU:
            sprintf(aszYUVType, "Package_YVYU");
            break;*/
        case MT_DRV_PIX_FMT_YUV400:
            snprintf(aszYUVType, sizeof(aszYUVType), "P400");
            break;
        case MT_DRV_PIX_FMT_YUV411:
            snprintf(aszYUVType, sizeof(aszYUVType), "P411");
            break;
        case MT_DRV_PIX_FMT_YUV420p:
            snprintf(aszYUVType, sizeof(aszYUVType), "P420");
            break;
        case MT_DRV_PIX_FMT_YUV422_1X2:
            snprintf(aszYUVType, sizeof(aszYUVType), "P422_1X2");
            break;
        case MT_DRV_PIX_FMT_YUV422_2X1:
            snprintf(aszYUVType, sizeof(aszYUVType), "P422_2X1");
            break;
        case MT_DRV_PIX_FMT_YUV_444:
            snprintf(aszYUVType, sizeof(aszYUVType), "P444");
            break;
        case MT_DRV_PIX_FMT_YUV410p:
            snprintf(aszYUVType, sizeof(aszYUVType), "P410");
            break;
        default:
            snprintf(aszYUVType, sizeof(aszYUVType), "UNKNOWN");
            break;
        }

        switch (pstChan->stUserCfgCap.enDecType)
        {
        case MT_UNF_VCODEC_DEC_TYPE_NORMAL:
            snprintf(aszDecType, sizeof(aszDecType), "NORMAL");
            break;
        case MT_UNF_VCODEC_DEC_TYPE_ISINGLE:
            snprintf(aszDecType, sizeof(aszDecType), "IFRAME");
            break;
        default:
            snprintf(aszDecType, sizeof(aszDecType), "UNKNOWN");
            break;
        }

        switch (pstChan->stUserCfgCap.enCapLevel)
        {
        case MT_UNF_VCODEC_CAP_LEVEL_QCIF:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "QCIF");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_CIF:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "CIF");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_D1:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "D1");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_720P:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "720P");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_FULLHD:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "FULLHD");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_2160x1280:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "1280P(1)");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_1280x2160:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "1280P(2)");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_2160x2160:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "2160P(1)");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_4096x2160:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "2160P(2)");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_2160x4096:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "2160P(3)");
            break;
        case MT_UNF_VCODEC_CAP_LEVEL_4096x4096:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "4096P");
            break;
        default:
            snprintf(aszCapLevel, sizeof(aszCapLevel), "UNKNOWN");
            break;
        }

        switch (pstChan->stUserCfgCap.enProtocolLevel)
        {
        case MT_UNF_VCODEC_PRTCL_LEVEL_MPEG:
            snprintf(aszProtocolLevel, sizeof(aszProtocolLevel), "OTHER");
            break;
        case MT_UNF_VCODEC_PRTCL_LEVEL_H264:
            snprintf(aszProtocolLevel, sizeof(aszProtocolLevel), "H264");
            break;
		case MT_UNF_VCODEC_PRTCL_LEVEL_MVC:
			snprintf(aszProtocolLevel, sizeof(aszProtocolLevel), "MVC");
			break;
        default:
            snprintf(aszProtocolLevel, sizeof(aszProtocolLevel), "UNKNOWN");
            break;
        }
        if(pstChan->stLastFrm.bTopFieldFirst)
        {
            snprintf(aszTopFirst, sizeof(aszTopFirst), "TRUE");
        }
        else
        {
            snprintf(aszTopFirst, sizeof(aszTopFirst), "FALSE");
        }
        s32Ret = VDEC_FindVpssHandleByVdecHandle(i,&hVpss);
        if(MT_SUCCESS !=s32Ret)
        {
            MT_ERR_VDEC("VDEC_FindVpssHandleByVdecHandle ERR\n");
        }
        PROC_PRINT(p, "============================== VDEC%d ================================\n", i);
        PROC_PRINT(p,
                        "Work State                          : %s\n",
                        (VDEC_CHAN_STATE_RUN == pstChan->enCurState) ? "RUN" : "STOP");
        PROC_PRINT(p,
                        "VpssID                              : vpss0%ld\n",
                         hVpss);
        PROC_PRINT(p,
                        "VfmwID                              : vfmw0%ld\n",
                        pstChan->hChan);
        PROC_PRINT(p,
                        "Codec ID                            : %s(0x%x)\n"
                        "Mode                                : %s\n"
                        "Priority                            : %u\n"
                        "ErrCover                            : %u\n"
                        "OrderOutput                         : %u\n"
                        "CtrlOption*                         : 0x%x\n"
                        "Capability                          : %s/%s/%s\n"
                        "Dynamic Frame Store                 : %s\n"
						"Use Desc Info Flag                  : %u\n"
						"Frame Rate                          : %u.%03u\n"
						"ForceFrameRateFlag                  : %u\n"
						"UnBlank                             : %d\n"
						"Dynamic Resolution Support          : %d\n"
						"Force Disable Timeout               : %d\n",
                        (pstChan->stCurCfg.enType
                         <= MT_UNF_VCODEC_TYPE_BUTT) ? (s_aszVdecType[pstChan->stCurCfg.enType]) : "UNKNOW",
                        pstChan->stCurCfg.enType,

                        aszDecMode,
                        pstChan->stCurCfg.u32Priority,
                        pstChan->stCurCfg.u32ErrCover,
                        pstChan->stCurCfg.bOrderOutput,
                        pstChan->stCurCfg.s32CtrlOptions,

                        aszDecType, aszCapLevel, aszProtocolLevel,

                        (1 == pstChan->stOption.u32DynamicFrameStoreAllocEn) ? "Enable" : "Disable",
                        pstChan->stCurCfg.u32UseDescInfoFlag,
						pstChan->stCurCfg.u32FrameRateInt, pstChan->stCurCfg.u32FrameRateDec,
						pstChan->stCurCfg.u32ForceFrameRateFlag,
						pstChan->stCurCfg.enUnBlank,
						VFMW_PARAMETER_DYNAMIC_RES,
						pstChan->stCurCfg.bForceDisableTimeout);
        if(1 == pstChan->stOption.u32DynamicFrameStoreAllocEn)
        {
            PROC_PRINT(p,"-------------------Dynamic Frame Store Information--------------------\n"
                         "Dynamic Frame Store Mode            : %s\n"
                         "DFS use MMZ                         : %d\n"
                         "DFS config Frame Number             : %d\n"
                         "DFS Extra  Frame Number             : %d\n"
                         "DFS Delay  Time(ms)                 : %d\n"
                         "DFS Max Mem Use(byte)               : %#x\n"
                         "DFS Memory PhyAddress               : %#llx\n"
                         "DFS Memory Length(byte)             : %ld\n",
                         (1 == pstChan->stOption.u32SelfAdaptionDFS) ? "Self" : "User",
                         pstChan->stOption.u32NeedMMZ,
                         pstChan->stOption.u32CfgFrameNum,
                         pstChan->stOption.s32ExtraFrameStoreNum,
                         pstChan->stOption.s32DelayTime,
                         pstChan->stOption.u32MaxMemUse,
                         pstChan->stVDHMMZBuf.startPhyAddr,
                         pstChan->stVDHMMZBuf.size
                         );//l00273086
        }

        PROC_PRINT(p,   "--------------------------Stream Information--------------------------\n"
                        "Source                              : %s%ld\n"
                        "StreamSize(Total/Current)           : 0x%x/0x%x\n"
                        "BitRate(bps)                        : %u\n",

                        (MT_INVALID_HANDLE == pstChan->hDmxVidChn) ? "User" : "DemuxChan",
                        (MT_INVALID_HANDLE == pstChan->hDmxVidChn) ? pstChan->hStrmBuf : (pstChan->hDmxVidChn&&0xff),

                        pstStatInfo->u32TotalVdecInByte,
                        pstStatInfo->u32TotalVdecHoldByte,

                        pstStatInfo->u32AvrgVdecInBps
                        );
        if (MT_UNF_VCODEC_TYPE_HEVC == pstChan->stCurCfg.enType)
        {
          #if (1 == MT_VDEC_VPU_SUPPORT)
            PROC_PRINT(p,
                            "LumaBitdepth                        : %d\n"
                            "ChromaBitdepth                      : %d\n",

                            pstChan->stVdecVpuStatus.u32LumaBitdepth,
                            pstChan->stVdecVpuStatus.u32ChromaBitdepth
                            );
          #else
            PROC_PRINT(p,
                            "LumaBitdepth                        : %d\n"
                            "ChromaBitdepth                      : %d\n",

                            pstChan->u32LastLumaBitdepth,
                            pstChan->u32LastChromaBitdepth
                            );
          #endif
        }
        else if (MT_UNF_VCODEC_TYPE_H264 == pstChan->stCurCfg.enType)
        {
            PROC_PRINT(p,
                            "LumaBitdepth                        : %d\n"
                            "ChromaBitdepth                      : %d\n",

                            pstChan->u32LastLumaBitdepth,
                            pstChan->u32LastChromaBitdepth
                            );
        }
        if (MT_INVALID_HANDLE == pstChan->hDmxVidChn)
        {
           s32Ret = BUFMNG_GetStatus(pstChan->hStrmBuf, &stBMStatus);

           if(MT_SUCCESS == s32Ret)
           {
                PROC_PRINT(p,
                        "StreamBufferSize(Total/Rd/Wt/Used/Percent)  : 0x%x/0x%x/0x%x/0x%x/%d%%\n",
                           (stBMStatus.u32Free+stBMStatus.u32Used),
                        	stBMStatus.u32RdPtr,
							stBMStatus.u32WrPtr,
                           stBMStatus.u32Used,
                           stBMStatus.u32Used*100/(stBMStatus.u32Free+stBMStatus.u32Used));
           }
		   
			rd = vdec_descriptor_get_read_ptr(pstChan->hStrmBuf);
			wt = vdec_descriptor_get_write_ptr(pstChan->hStrmBuf);
			tmp = wt >= rd ? (wt - rd) : (CFG_VDEC_VES_DESC_BUFF_SIZE - rd + wt);
			PROC_PRINT(p,
				"DescBuffer(Total/Rd/Wt/Percent)     : 0x%x/0x%x/0x%x/%d%%\n",
				CFG_VDEC_VES_DESC_BUFF_SIZE, rd, wt, tmp * 10 / CFG_VDEC_VES_DESC_BUFF_SIZE);
        }
		
		memset(&v_ch_st, 0, sizeof(VDEC_CHAN_STATE_S));
		
		if((pstChan->enCurState == VDEC_CHAN_STATE_RUN) && (pstChan->hChan >=0)) {
			s32Ret = KERN_VDEC_Control(pstChan->hChan, VDEC_CID_GET_CHAN_STATE, &v_ch_st);
		}
		MT_ERR_VDEC("stVdecChanCfg.s32FrameErrThr %d !\n", v_ch_st.dec_error_frame_num);
		
		if (VDEC_OK != s32Ret) {
			MT_ERR_VDEC("VFMW GET_CHAN_CFG err!\n");
		}
        PROC_PRINT(p,"--------------------------Picture Information-------------------------\n"
						"FrameNo                             : %u\n"
						"FrameIndex                          : %u\n"
                        "Width*Height                        : %d*%d\n"
						"PTS                                 : 0x%llx\n"
                        "Stride(Y/C)                         : %#x/%#x\n"
                        "FrameRate(fps)                      : Real(%u.%u) FrameInfo(%d)\n"
                        "PlayFormat                          : %s\n"
                        "FrmPackingType                      : %s\n"
                        "Aspect(User/Decode)                 : %s/%s\n"
                        "FieldMode                           : %s\n"
                        "Type                                : %s\n"
                        "YUVFormat                           : %s\n"
                        "TopFirst                            : %s\n"
                        "ErrFrame                            : %u\n"
                        "TypeNum(I/P)                        : %u/%u\n"
                        "FrameBuffer Range                   : [0x%llx,0x%llx]\n"
                        "VP6 Picture Reversed                : %s\n\n",

						pstChan->stLastFrm.u32FrameNo,
						pstChan->stLastFrm.u32FrameIndex,
                        pstChan->stLastFrm.u32Width, pstChan->stLastFrm.u32Height,
                        pstChan->stLastFrm.u64Pts,
                        pstChan->stLastFrm.stBufAddr[0].u32Stride_Y,
                        pstChan->stLastFrm.stBufAddr[0].u32Stride_C,
                        pstStatInfo->u32AvrgVdecFps, pstStatInfo->u32AvrgVdecFpsLittle,
                        pstChan->stLastFrm.u32FrameRate,
                        aszDisplayNorm, aszFrmPackingType, aszUserRatio, aszDecodeRatio, aszFieldMode,
                        aszSampleType, aszYUVType, aszTopFirst,

                        v_ch_st.dec_error_frame_num,//w00271806
                        pstStatInfo->u32FrameType[0],
                        pstStatInfo->u32FrameType[1],
                        pstChan->stVDHMMZBuf.startPhyAddr,(pstChan->stVDHMMZBuf.startPhyAddr+pstChan->stVDHMMZBuf.size),
                        (1 == (pstChan->stCurCfg.unExtAttr.stVP6Attr.bReversed && 0x1)) ? "YES" : "NO"

             );

		pSlotInfo = &pstChan->stLastFrm.slotInfo;
        PROC_PRINT(p,"--------------------------Slot Information-------------------------\n"
						"frm_cnt                             : %u\n"
						"pts                                 : 0x%llx\n"
						"pic(w/h)                            : %u*%u\n"
						"top field                           : (slot %u, pts %u, addrLuma 0x%x, addrChroma 0x%x, %x %x %x %x %u %u)\n"
						"bot field                           : (slot %u, pts %u, addrLuma 0x%x, addrChroma 0x%x, %x %x %x %x %u %u)\n"
						"top right field                     : (slot %u, pts %u, addrLuma 0x%x, addrChroma 0x%x, %x %x %x %x %u %u)\n"
						"bot right field                     : (slot %u, pts %u, addrLuma 0x%x, addrChroma 0x%x, %x %x %x %x %u %u)\n"
						"display_order_mode_valid            : %u\n"
						"display_order_mode                  : %d\n"
						"repeat_frm_num                      : %u\n"
						"frame_pic_flag                      : %u\n"
						"progressive_frame                   : %u\n"
						"top_field_first                     : %u\n"
						"picture_coding_type                 : %u\n"
						"aspect_ratio                        : %u\n"
						"field_storage_mode                  : %u\n"
						"isHalf                              : %u\n"
						"is_3D_flag                          : %u\n"
						"packing_type                        : %d\n"
						"input_rate                          : %u\n"
						"usrdat                              : %p %p %p %p\n"
						"sar(w/h)                            : %u*%u\n"
						"active_format_flag                  : %u\n"
						"active_format                       : %u\n"
						"end_of_stream_flag                  : %u\n"
						"vdec_mfbc_enable                    : %u\n"
						"dec_type                            : %d\n"
						"CurBank                             : %u\n"
						"crop_info                           : %u (%u, %u, %u, %u)\n\n",
						pSlotInfo->frm_cnt,
						pSlotInfo->pts,
						pSlotInfo->pic_width, pSlotInfo->pic_height,

						pSlotInfo->filedInfoTop.slot_idx, pSlotInfo->filedInfoTop.pts,
						pSlotInfo->filedInfoTop.addrLuma, pSlotInfo->filedInfoTop.addrChroma,
						pSlotInfo->filedInfoTop.addrLuma_lut, pSlotInfo->filedInfoTop.addrChroma_lut,
						pSlotInfo->filedInfoTop.stride_data_luma, pSlotInfo->filedInfoTop.stride_data_chroma,
						pSlotInfo->filedInfoTop.bit_depth_luma, pSlotInfo->filedInfoTop.bit_depth_chroma,

						pSlotInfo->filedInfoBot.slot_idx, pSlotInfo->filedInfoBot.pts,
						pSlotInfo->filedInfoBot.addrLuma, pSlotInfo->filedInfoBot.addrChroma,
						pSlotInfo->filedInfoBot.addrLuma_lut, pSlotInfo->filedInfoBot.addrChroma_lut,
						pSlotInfo->filedInfoBot.stride_data_luma, pSlotInfo->filedInfoBot.stride_data_chroma,
						pSlotInfo->filedInfoBot.bit_depth_luma, pSlotInfo->filedInfoBot.bit_depth_chroma,

						pSlotInfo->filedInfoTopRight.slot_idx, pSlotInfo->filedInfoTopRight.pts,
						pSlotInfo->filedInfoTopRight.addrLuma, pSlotInfo->filedInfoTopRight.addrChroma,
						pSlotInfo->filedInfoTopRight.addrLuma_lut, pSlotInfo->filedInfoTopRight.addrChroma_lut,
						pSlotInfo->filedInfoTopRight.stride_data_luma, pSlotInfo->filedInfoTopRight.stride_data_chroma,
						pSlotInfo->filedInfoTopRight.bit_depth_luma, pSlotInfo->filedInfoTopRight.bit_depth_chroma,

						pSlotInfo->filedInfoBotRight.slot_idx, pSlotInfo->filedInfoBotRight.pts,
						pSlotInfo->filedInfoBotRight.addrLuma, pSlotInfo->filedInfoBotRight.addrChroma,
						pSlotInfo->filedInfoBotRight.addrLuma_lut, pSlotInfo->filedInfoBotRight.addrChroma_lut,
						pSlotInfo->filedInfoBotRight.stride_data_luma, pSlotInfo->filedInfoBotRight.stride_data_chroma,
						pSlotInfo->filedInfoBotRight.bit_depth_luma, pSlotInfo->filedInfoBotRight.bit_depth_chroma,

						pSlotInfo->display_order_mode_valid,
						pSlotInfo->display_order_mode,
						pSlotInfo->repeat_frm_num,
						pSlotInfo->frame_pic_flag,
						pSlotInfo->progressive_frame,
						pSlotInfo->top_field_first,
						pSlotInfo->picture_coding_type,
						pSlotInfo->aspect_ratio,
						pSlotInfo->filed_storage_mode,
						pSlotInfo->isHalf,
						pSlotInfo->is_3D_flag,
						pSlotInfo->packing_type,
						pSlotInfo->input_rate,
						pSlotInfo->p_usrdat[0], pSlotInfo->p_usrdat[1], pSlotInfo->p_usrdat[2], pSlotInfo->p_usrdat[3],
						pSlotInfo->sar_width, pSlotInfo->sar_height,
						pSlotInfo->active_format_flag,
						pSlotInfo->active_format,
						pSlotInfo->end_of_stream_flag,
						pSlotInfo->vdec_mfbc_enable,
						pSlotInfo->dec_type,
						pSlotInfo->uCurBank,

						pSlotInfo->crop_info.bEnable,
						pSlotInfo->crop_info.left,
						pSlotInfo->crop_info.top,
						pSlotInfo->crop_info.width,
						pSlotInfo->crop_info.height);

        if (MT_INVALID_HANDLE == pstChan->hDmxVidChn)
        {
            //s32Ret = BUFMNG_GetStatus(pstChan->hStrmBuf, &stBMStatus);
            if(MT_SUCCESS == s32Ret)
            {

                PROC_PRINT(p,
                            "DMX/USER->VDEC\n"
                            "GetStreamBuffer(Try/OK)             : %d/%d\n"
                            "PutStreamBuffer(Try/OK)             : %d/%d\n",
                            stBMStatus.u32GetTry, stBMStatus.u32GetOK,
                            stBMStatus.u32PutTry, stBMStatus.u32PutOK);
            }
        }


        if (MT_INVALID_HANDLE == pstChan->hDmxVidChn)
        {
            PROC_PRINT(p,
                            "VDEC->VFMW\n"
                            "AcquireStream(Try/OK)               : %d/%d\n"
                            "ReleaseStream(Try/OK)               : %d/%d\n",
                            stBMStatus.u32RecvTry, stBMStatus.u32RecvOK,
                            stBMStatus.u32RlsTry, stBMStatus.u32RlsOK);
        }
        else
        {
            PROC_PRINT(p,
                            "VDEC->VFMW\n"
                            "AcquireStream(Try/OK)               : %d/%d\n"
                            "ReleaseStream(Try/OK)               : %d/%d\n",
                            pstStatInfo->u32VdecAcqBufTry, pstStatInfo->u32VdecAcqBufOK,
                            pstStatInfo->u32VdecRlsBufTry, pstStatInfo->u32VdecRlsBufOK);
        }

        PROC_PRINT(p,
                            "VFMW->VPSS\n"
                            "AcquireFrame(Try/OK)                : %d/%d\n"
                            "ReleaseFrame(Try/OK)                : %d/%d\n",
                        pstStatInfo->u32VdecRcvFrameTry, pstStatInfo->u32VdecRcvFrameOK,
                        pstStatInfo->u32VdecRlsFrameTry, pstStatInfo->u32VdecRlsFrameOK);
        #if 0
        PROC_PRINT(p,
                            "VDEC->VPSS\n"
                            "AcquireFrame(Try/OK)            : %d/%d\n"
                            "ReleaseFrame(Try/OK)            : %d/%d\n",
                        pstStatInfo->u32UserAcqFrameTry, pstStatInfo->u32UserAcqFrameOK,
                        pstStatInfo->u32UserRlsFrameTry, pstStatInfo->u32UserRlsFrameOK);
        #endif
        PROC_PRINT(p,
                            "VPSS->AVPLAY\n"
                            "AcquireFrame(Try/OK)                : %d/%d\n"
                            "ReleaseFrame(Try/OK)                : %d/%d\n\n",
                        pstStatInfo->u32AvplayRcvFrameTry, pstStatInfo->u32AvplayRcvFrameOK,
                        pstStatInfo->u32AvplayRlsFrameTry, pstStatInfo->u32AvplayRlsFrameOK);

		//dump vdec registers
		Proc_Dump_Register(p, "VDEC", REG_VDEC_BASE, REG_VDEC_RANGE);

        PROC_PRINT(p, "=======================================================================\n");

    }
    else
    {
        PROC_PRINT(p, "vdec not init!\n" );
    }

    return 0;
}

// add by w00278582
static mt_s32 VDEC_DRV_VPUReadProc(struct seq_file *p, mt_void *v)
{
    mt_s32 i, j;
    mt_s32 s32Ret;
    VDEC_CHANNEL_S *pstChan;
    mt_proc_entry_t *pstProcItem;
    char putArr[32];
    char *pStr = putArr;
    char strDecStat[MAX_VID_PROTOCOL_NAME];
    char strInstMode[MAX_VID_PROTOCOL_NAME];
    char strDecMode[MAX_VID_PROTOCOL_NAME];

    pstProcItem = p->private;

    if (0 == strncmp(pstProcItem->entry_name, "vdec_ctrl",9))
    {
        return 0;
    }

    s32Ret = sscanf(pstProcItem->entry_name, "vdec_vpu%02d", &i);
    if(s32Ret <=0)
    {
        PROC_PRINT(p, "Invalid VDEC ID.\n");
        return 0;
    }
    if (i >= MT_VDEC_MAX_INSTANCE_NEW)
    {
        PROC_PRINT(p, "Invalid VDEC ID:%d.\n", i);
        return 0;
    }

    pstChan = VDEC_DRV_GetChan(i);

    if(MT_NULL == pstChan)
    {
        MT_ERR_VDEC("ERR: chan %d is not init!\n",i);
        return 0;
    }

    for (j = 0; j < pstChan->stVdecVpuStatus.u32ActualFrmBufNum; j++)
    {
        snprintf(pStr, 3, "%2d",  pstChan->stVdecVpuStatus.stFrmStatus[j].u32IsPutVdecQueue);
        pStr++;
        pStr++;
    }
    VPU_DecodeStat2Str(pstChan->stVdecVpuStatus.u32DecodeStatus, strDecStat);
    VPU_InstMode2Str(pstChan->stVdecVpuStatus.u32InstanceMode, strInstMode);
    VPU_DecodeMode2Str(pstChan->stVdecVpuStatus.u32DecodeMode,strDecMode);

    if (pstChan)
    {
        PROC_PRINT(p, "|--------------------------------------------------------------------|\n");
        PROC_PRINT(p, "|            VPU%02d                 |VERSION         : %-10d     |\n", i, pstChan->stVdecVpuStatus.u32Version);
        PROC_PRINT(p, "|----------------------------------|---------------------------------|\n");
        PROC_PRINT(p, "|        VPU DECODER STATUS        |      BITSTREAM BUF INFO         |\n");
        PROC_PRINT(p, "|----------------------------------|---------------------------------|\n");
        PROC_PRINT(p, "|DecodeStatus    : %-7s         |PhyAddr         : 0x%-10x   |\n", strDecStat, pstChan->stVdecVpuStatus.u32PhyAddr);
        PROC_PRINT(p, "|DecodeMode      : %-7s         |BsBufSize       : %-9d      |\n", strDecMode, pstChan->stVdecVpuStatus.u32BsBufSize);
        PROC_PRINT(p, "|InstanceMode    : %-7s         |BsBufUsedSize   : %-9d      |\n", strInstMode, pstChan->stVdecVpuStatus.u32BsBuFUsedSize);
        PROC_PRINT(p, "|----------------------------------|BsBufPercent    : %-3d%%           |\n", pstChan->stVdecVpuStatus.u32BsBufPercent);
        PROC_PRINT(p, "|        BITSTREAM INFO            |BsBufReadPtr    : 0x%-10x   |\n", pstChan->stVdecVpuStatus.u32BsBufReadPtr);
        PROC_PRINT(p, "|----------------------------------|BsBufWritePtr   : 0x%-10x   |\n", pstChan->stVdecVpuStatus.u32BsBufWritePtr);
        PROC_PRINT(p, "|VedioStandard   : %-7s         |---------------------------------|\n", "HEVC");
        PROC_PRINT(p, "|DecWidth        : %-7d         |      VPU FRAME BUF INFO         |\n", pstChan->stVdecVpuStatus.u32DecWidth);
        PROC_PRINT(p, "|DecHeight       : %-7d         |---------------------------------|\n", pstChan->stVdecVpuStatus.u32DecHeight);
        PROC_PRINT(p, "|DispWidth       : %-7d         |ActualFrmBufNum : %-5d          |\n", pstChan->stVdecVpuStatus.u32DispWidth, pstChan->stVdecVpuStatus.u32ActualFrmBufNum);
        PROC_PRINT(p, "|DispHeight      : %-7d         |%-31s  |\n", pstChan->stVdecVpuStatus.u32DispHeight, putArr);
        for (j = 0; j < pstChan->stVdecVpuStatus.u32ActualFrmBufNum; j++)
        {
            PROC_PRINT(p, "%2d" ,pstChan->stVdecVpuStatus.stFrmStatus[j].u32IsPutVdecQueue);
        }
        PROC_PRINT(p, "\n");

        //PROC_PRINT(p, "|ErrRatio        : %-7d         |OldFrmBufNum    : %-5d          |\n", pstChan->stVdecVpuStatus.u32ErrRatio, pstChan->stVdecVpuStatus.u32OldFrmBufNum);
        PROC_PRINT(p, "|ErrRatio        : %-7d         |---------------------------------|\n", pstChan->stVdecVpuStatus.u32ErrRatio);
        PROC_PRINT(p, "|NumOfErrMBs     : %-7d         |\n", pstChan->stVdecVpuStatus.u32NumOfErrMBs);
        PROC_PRINT(p, "|SeqChangeCount  : %-7d         |\n", pstChan->stVdecVpuStatus.u32SeqChangeCount);
        PROC_PRINT(p, "|MainProfile     : %-7d         |\n", pstChan->stVdecVpuStatus.u32Profile);
        PROC_PRINT(p, "|LumaBitDepth    : %-7d         |\n", pstChan->stVdecVpuStatus.u32LumaBitdepth);
        PROC_PRINT(p, "|ChromaBitDepth  : %-7d         |\n", pstChan->stVdecVpuStatus.u32ChromaBitdepth);
        PROC_PRINT(p, "|DecodIndex      : %-7d         |\n", pstChan->stVdecVpuStatus.s32indexFrameDecoded);
        PROC_PRINT(p, "|DisplayIndex    : %-7d         |\n", pstChan->stVdecVpuStatus.s32indexFrameDisplay);
        PROC_PRINT(p, "|----------------------------------|\n");

    }
    else
    {
        PROC_PRINT(p, "vpu init!\n" );
    }

    return 0;
}

// add by w00278582
static mt_s32 VDEC_DRV_VPUWriteProc(struct file * file, const char __user * buffer, size_t count, loff_t *ppos)
{
    // do something
    return 0;
}

// w00278582
void VPU_DecodeStat2Str(mt_u32 eDecStat, char *strVidStd)
{
    char *pStrOpen         = "OPEN";
    char *pStrRun          = "OPEN";
    char *pStrStop         = "STOP";
    char *pStrClose        = "CLOSE";
    char *pStrButt         = "BUTT";

    switch (eDecStat)
    {
        case 0:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrOpen, strlen(pStrOpen));
            strVidStd[strlen(pStrOpen)] = '\0';
            break;
        case 1:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrRun, strlen(pStrRun));
            strVidStd[strlen(pStrRun)] = '\0';
            break;
        case 2:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrStop, strlen(pStrStop));
            strVidStd[strlen(pStrStop)] = '\0';
            break;
        case 3:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrClose, strlen(pStrClose));
            strVidStd[strlen(pStrClose)] = '\0';
            break;
        case 4:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrButt, strlen(pStrButt));
            strVidStd[strlen(pStrButt)] = '\0';
            break;

        default:
            *strVidStd = '\0';
            break;
    }
}

void VPU_InstMode2Str(mt_u32 eInstMode, char *strVidStd)
{
    char *pStrNormal       = "NORMAL";
    char *pStrISingle      = "ISINGLE";
    char *pStrButt         = "BUTT";

    switch (eInstMode)
    {
        case 0:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrNormal, strlen(pStrNormal));
            strVidStd[strlen(pStrNormal)] = '\0';
            break;
        case 1:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrISingle, strlen(pStrISingle));
            strVidStd[strlen(pStrISingle)] = '\0';
            break;
        case 2:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrButt, strlen(pStrButt));
            strVidStd[strlen(pStrButt)] = '\0';
            break;

        default:
            *strVidStd = '\0';
            break;
    }
}

void VPU_DecodeMode2Str(mt_u32 eDecMode, char *strVidStd)
{
    char *pStrIPB          = "IPB";
    char *pStrIP           = "IP";
    char *pStrI            = "I";
    char *pStrDISCARD      = "DISCARD";
    char *pStrBUTT         = "BUTT";

    switch (eDecMode)
    {
        case 0:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrIPB, strlen(pStrIPB));
            strVidStd[strlen(pStrIPB)] = '\0';
            break;
        case 1:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrIP, strlen(pStrIP));
            strVidStd[strlen(pStrIP)] = '\0';
            break;
        case 2:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrI, strlen(pStrI));
            strVidStd[strlen(pStrI)] = '\0';
            break;
        case 3:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrDISCARD, strlen(pStrDISCARD));
            strVidStd[strlen(pStrDISCARD)] = '\0';
            break;
        case 4:
        	//FIXME: bad usage
            strncpy(strVidStd, pStrBUTT, strlen(pStrBUTT));
            strVidStd[strlen(pStrBUTT)] = '\0';
            break;

        default:
            *strVidStd = '\0';
            break;
    }
}

static mt_device_s    VdecDev;
void  vdec_config_sram(void);

#include <linux/dma-direct.h>
phys_addr_t vdec_phy2dma_addr(phys_addr_t phy_addr)
{
	return phys_to_dma(VdecDev.dev, phy_addr);
}

mt_s32 __init VDEC_DRV_ModInit(mt_void)
{
    int ret;

    vdec_config_sram();

#ifndef MT_MCE_SUPPORT  //Rock_hu  暂时打开，以后靠宏控制
    ret = VDEC_DRV_Init();
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_VDEC("Init drv fail!\n");
        return MT_FAILURE;
    }
#endif
    MT_INFO_VDEC("VDEC_DRV_ModInit int \n");

    memset((void*)VdecDev.devfs_name, 0, sizeof(VdecDev.devfs_name));
    strncpy(VdecDev.devfs_name, UMAP_DEVNAME_VDEC, sizeof(VdecDev.devfs_name) - 1);
    VdecDev.fops   = &VdecFileOpts;
    VdecDev.minor  = UMAP_MIN_MINOR_VDEC;
    VdecDev.owner  = THIS_MODULE;
    VdecDev.drvops = &VdecOps;

    if (mt_drv_dev_register(&VdecDev) < 0)
    {
        MT_FATAL_VDEC("Reg dev failed\n");
        return MT_FAILURE;
    }

    ret = VDEC_DRV_RegisterProc(&s_stProcParam);
    if (MT_SUCCESS != ret)
    {
        MT_FATAL_VDEC("Reg proc fail!\n");
        return MT_FAILURE;
    }

    MT_INFO_VDEC("VDEC_DRV_ModInit Exit \n");

    return MT_SUCCESS;
}

mt_void __exit VDEC_DRV_ModExit(mt_void)
{
    VDEC_DRV_UnregisterProc();
    mt_drv_dev_unregister(&VdecDev);

#ifndef MT_MCE_SUPPORT
    VDEC_DRV_Exit();
#endif

    return;
}

MODULE_AUTHOR("MONTAGE-LZ");
MODULE_LICENSE("GPL");

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */
