/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/***************************** included files ******************************/
#include "mt_type.h"
#include "mt_drv_struct.h"
#include "mt_drv_dev.h"
#include "mt_drv_proc.h"
#include "mt_drv_stat.h"
#include "mt_drv_mem.h"
#include "mt_drv_dma.h"
#include "mt_drv_ampshm.h"
#include "mt_kernel_adapt.h"
#include "mt_module_debug.h"
#include "mt_mpi_demux.h"
#include "mt_drv_demux.h"
#include "mt_drv_adec.h"
#include "drv_demux.h"
#include "drv_demux_ioctl.h"
#include "drv_demux_ext.h"
#include "drv_demux_func.h"
#include "drv_sync_ioctl.h"

#include "../../../sync/drv_sync.h"

#ifdef DMX_DESCRAMBLER_SUPPORT
#include "mt_drv_descrambler.h"
#include "drv_descrambler_func.h"
#include "descrambler/drv_descrambler.h"
#endif

#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/kthread.h>
#include <uapi/linux/sched/types.h>


//#define MT_MCE_SUPPORT
/**************************** global variables ****************************/
static demux_priv_data demux_priv_data_info;
static mt_device_s g_stDemuxDev;
static struct task_struct *demux_av_thread = NULL;
struct task_struct *demux_pes_thread = NULL;
static mt_s32 g_channel_reset = 0;
static mt_u64 g_continue_pts = 0;

#define INVALID_OQ_ID 256
#ifdef MT_DEMUX_PROC_SUPPORT
/****************************** internal function *****************************/
char* SAVE_VES_STR = "save_ves";
char* SAVE_AES_STR = "save_aes";
char* SAVE_REC_STR = "save_rec";

char* SAVE_ALLTS_STR = "save allts";
char* SAVE_IPTS_STR = "save ipts";
char* SAVE_DMXTS_STR = "save dmxts";
char* HELP_STR = "help";
char* START_STR = "start";
char* STOP_STR = "stop";
char* DEBUG_OPEN_STR = "debug open";
char* DEBUG_CLOSE_STR = "debug close";

static MT_BOOL      DebugEn           = MT_FALSE;
extern DMX_Proc_Global_Info_S GlobalProcInfo;
OQ_HeaderInfor_t            OQInfo[DMX_OQ_CNT];
FQ_HeaderInfor_t            FQInfo[DMX_OQ_CNT];
ChannelDataFlow_info_t      ChannelDF[DMX_OQ_CNT];


mt_u8 DescProcList[10][40]  = {   "DescPhyAddr (0xc800+0x100*t)",
                                  "DescDepth   (0xc804+0x100*t)",
                                  "DescWPtr    (0xc808+0x100*t)",
                                  "DescRPtr    (0xc808+0x100*t)",
                                  "ValidDescNum(0xc80c+0x100*t)",
                                  "AddDescNum  (0xc810+0x100*t)",
                                  "DescWAddr   (     *DescWPtr)",
                                  "DescRAddr   (     *DescRPtr)",
                                  "DescSize    ( *DescWPtr + 1)",
                                  };

mt_u8 BPProcList[10][40]    = {   "FQLow32BPEn       (0x0xC220)",
                                  "FQHigh8BPEn       (0x0xC224)",
                                  "FQBP          (0xc340+t*0x8)",
                                  "SwitchBufferBp      (0xc204)",
                                  "OverflowBp          (0xc204)",
                                  "Rate        (0xc830+0x100*t)",
                                  "DebugEn     (0xC890+0x100*t)",
                                  "OutByte     (0xC894+0x100*t)",
                                  "OutTSNum    (0xC898+0x100*t)",
                                  "BPCount     (0xC89c+0x100*t)",

                                  };

mt_u8 OQProcInfo[13][40]  = {   "OQID                    :",
                                "BBWAddr    (0xe008[15:0])",
                                "BBEopAddr (0xe008[31:16])",
                                "OQUse      (0xe00c[13:0])",
                                "BBSize    (0xe00c[31:16])",
                                "BBSAddr          (0xe800)",
                                "OQWPtr      (0xe804[9:0])",
                                "OQRPtr    (0xe804[25:16])",
                                "OQWAddr         (*OQWPtr)",
                                "OQRAddr         (*OQRPtr)",
                                "FQCfg       (0xe808[9:0])",
                                "OQSize    (0xe808[25:16])",
                                "OQSAddr          (0xe80c)"

                            };

mt_u8 OQComments[13][40]  = {   "OQ ID",
                                "Cur BB Writing offset addr",
                                "Cur BB EOP offset addr",
                                "Out OQ Desc total num",
                                "BB(BufferBlock) size",
                                "Cur (Writing)BB start addr",
                                "OQ Writing ptr",
                                "OQ Reading ptr",
                                "OQ Writing ptr DDR Addr",
                                "OQ Reading ptr DDR Addr",
                                "FQ ID",
                                "OQ depth",
                                "OQ Start addr",

                            };

mt_u8 FQProcInfo[6][40]  = {    "FQUse      (0xd000[15:0])",
                                "FQRPtr     (0xd004[15:0])",
                                "FQVal      (0xd004[31:16)",
                                "FQWPtr     (0xd008[15:0])",
                                "FQSize    (0xd008[31:16])",
                                "FQStartAddr      (0xd00c)"
                            };
mt_u8 FQComments[6][40]  = {    "Total FQ Desc Num used",
                                "FQ Reading ptr",
                                "Valid FQ Desc (logic)",
                                "FQ Writing ptr",
                                "FQ depth",
                                "FQ Start addr",
                            };

mt_u8 ChanDataFlow[4][40]  = {  "PIDTsPacket  (0xda00+4*n)",
                                "BufferTsNum  (0xd800+4*n)",
                                "OQEn         (0xc130+4*n)",
                                "FQEn         (0xc140+4*n)",
                            };

mt_u8 ChanDFComments[4][40] = { "Ts  Num passed chan",
                                "Ts  Num entered Chan buf",
                                "OQ Enable",
                                "OFQ Enable",
                            };


static mt_char ChanType[][4]   = {"SEC", "PES", "AUD", "VID", "PST", "ECM", "HPE", "REC", "AAD"};
static mt_char OutMode[][8]    = {"Reserve","PLY", "REC", "P&R"};

extern DMX_DEV_OSI_S *g_pDmxDevOsi;
extern struct semaphore  g_lock_av_sema;
extern struct semaphore  g_lock_pes_sema;
extern struct mutex g_demux_ch_mutex;

static mt_void DMXDebugShowHelp(mt_void)
{
    mt_drv_proc_echohelp("cat  /proc/msp/demux_main                   -- display all demux info\n");
    mt_drv_proc_echohelp("cat  /proc/msp/demux_chan                   -- display running channel info\n");
    mt_drv_proc_echohelp("cat  /proc/msp/demux_filter                 -- display running filter info\n");
    mt_drv_proc_echohelp("cat  /proc/msp/demux_rec                    -- display record info\n");
	mt_drv_proc_echohelp("echo save_ves /media/sda1 >  /proc/msp/demux_main  -- store ves\n");
	mt_drv_proc_echohelp("echo save_aes /media/sda1 >  /proc/msp/demux_main  -- store aes\n");
	mt_drv_proc_echohelp("echo save_rec /media/sda1 >  /proc/msp/demux_main  -- store aes\n");
    mt_drv_proc_echohelp("echo LogLevel = 0x8 > /proc/msp/demux_main  -- display dmx apts info\n");
}

#define STRSKIPBLANK(str)      \
        while ((str[0] == ' ') || (str[0] == '='))\
        {\
        (str)++;\
        }\

#define DRV_DMX_PROC_LEN	512
static mt_s32 DMXProcWrite(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    mt_u32 cmd = 0xff;
    mt_u32 param = 0;
    mt_u32 needparam = 0;
    DMX_DEBUG_CMD_CTRl cmdctrl;
	char szBuf[DRV_DMX_PROC_LEN] = {0};
	mt_u32 buf_len = 0;
	
    char *p1 = szBuf;
    mt_s32 ret;
	
	mt_u8  len = 0;
	char es_save_path[DRV_DMX_PROC_LEN] = {0};

	memset(szBuf, 0, sizeof(szBuf)); 
	if (copy_from_user(p1, buf, count < DRV_DMX_PROC_LEN ? count : DRV_DMX_PROC_LEN))
    {
        MT_ERR_DEMUX("copy from user failed\n");
        return -1;
    }

    szBuf[strlen(szBuf) - 1] = '\0';//use '\0' to instead of the last character '\n' 
	buf_len = strlen(szBuf);
	
    STRSKIPBLANK(p1);
	
    if (strstr(p1, SAVE_VES_STR) && p1[0] == 's')
    {
        cmd = DMX_DEBUG_CMD_SAVE_VES;

		len = strlen(SAVE_VES_STR);
        p1 += len;
		while (len < buf_len)
		{
			if ('/' == *p1)//find the start of a path
			{
				break;
			}

			p1++;
			len++;
		}
        
		if (buf_len == len)//can't not find '/'
		{
			printk(KERN_ERR "[%s %d]don't set path, /media/sda1 will be used\n", __FUNCTION__, __LINE__);
			memset(es_save_path, 0, sizeof(es_save_path));
			snprintf(es_save_path, sizeof(es_save_path), "%s", "/media/sda1");
		}
		else
		{			
	        memset(es_save_path, 0, sizeof(es_save_path));
			snprintf(es_save_path, sizeof(es_save_path), "%s", p1);
			if (len < buf_len - 1)			
			{
				if ('/' == es_save_path[buf_len - 1 - len])//if the last character is '/', delete it.
				{
					es_save_path[buf_len - 1 - len] = '\0';
				}
	        }
		}
		printk(KERN_ERR "[%s %d]video es_save_path=%s\n", __FUNCTION__, __LINE__, es_save_path);
    }
    else if (strstr(p1, SAVE_AES_STR) && p1[0] == 's')
    {
        cmd = DMX_DEBUG_CMD_SAVE_AES;
        len = strlen(SAVE_VES_STR);
        p1 += len;
		while (len < buf_len)
		{
			if ('/' == *p1)//find the start of a path
			{
				break;
			}

			p1++;
			len++;
		}
        
		if (buf_len == len)//can't not find '/'
		{
			printk(KERN_ERR "[%s %d]don't set path, /media/sda1 will be used\n", __FUNCTION__, __LINE__);
			memset(es_save_path, 0, sizeof(es_save_path));
			snprintf(es_save_path, sizeof(es_save_path), "%s", "/media/sda1");
		}
		else
		{			
	        memset(es_save_path, 0, sizeof(es_save_path));
			snprintf(es_save_path, sizeof(es_save_path), "%s", p1);
			if (len < buf_len - 1)			
			{
				if ('/' == es_save_path[buf_len - 1 - len])//if the last character is '/', delete it.
				{
					es_save_path[buf_len - 1 - len] = '\0';
				}
	        }
		}
		printk(KERN_ERR "[%s %d]audio es_save_path=%s\n", __FUNCTION__, __LINE__, es_save_path);
    }
	else if (strstr(p1, SAVE_REC_STR) && p1[0] == 's')
    {
        cmd = DMX_DEBUG_CMD_SAVE_REC;
        len = strlen(SAVE_REC_STR);
        p1 += len;
		while (len < buf_len)
		{
			if ('/' == *p1)//find the start of a path
			{
				break;
			}

			p1++;
			len++;
		}
        
		if (buf_len == len)//can't not find '/'
		{
			printk(KERN_ERR "[%s %d]don't set path, /media/sda1 will be used\n", __FUNCTION__, __LINE__);
			memset(es_save_path, 0, sizeof(es_save_path));
			snprintf(es_save_path, sizeof(es_save_path), "%s", "/media/sda1");
		}
		else
		{			
	        memset(es_save_path, 0, sizeof(es_save_path));
			snprintf(es_save_path, sizeof(es_save_path), "%s", p1);
			if (len < buf_len - 1)			
			{
				if ('/' == es_save_path[buf_len - 1 - len])//if the last character is '/', delete it.
				{
					es_save_path[buf_len - 1 - len] = '\0';
				}
	        }
		}
		printk(KERN_ERR "[%s %d]record es_save_path=%s\n", __FUNCTION__, __LINE__, es_save_path);
    }
    else if (strstr(p1, SAVE_IPTS_STR) && p1[0] == 's')
    {
        cmd = DMX_DEBUG_CMD_SAVE_IPTS;
        p1 += strlen(SAVE_IPTS_STR);
        needparam = 1;
    }
    else if (strstr(p1, SAVE_DMXTS_STR) && p1[0] == 's')
    {
        cmd = DMX_DEBUG_CMD_SAVE_DMXTS;
        p1 += strlen(SAVE_DMXTS_STR);
        needparam = 1;
    }
    else if (strstr(p1, HELP_STR) && p1[0] == 'h')
    {
        DMXDebugShowHelp();
        return count;
    }
    else if (strstr(p1, DEBUG_OPEN_STR) && p1[0] == 'd')
    {
        DebugEn = MT_TRUE;
        return count;
    }
    else if (strstr(p1, DEBUG_CLOSE_STR) && p1[0] == 'd')
    {
        DebugEn = MT_FALSE;
        return count;
    }
    else if (strstr(p1, "LogLevel"))
    {
        cmd = DMX_DEBUG_CMD_PRINT_INFO;
        p1 += strlen("LogLevel");
        needparam = 1;
    }
    else
    {
        MT_PRINT("unknow command:%s\n",p1);
        DMXDebugShowHelp();
        return (-1);
    }

    /*
    do not delete the next useless code
    */
#if 0
    STRSKIPBLANK(p1);

    if (strstr(p1, START_STR))
    {
        cmdctrl = DMX_DEBUG_CMD_START;
        p1 += strlen(START_STR);
    }
    else if (strstr(p1, STOP_STR))
    {
        cmdctrl = DMX_DEBUG_CMD_STOP;
        p1 += strlen(STOP_STR);
        needparam = 0;
    }
    else
    {
        MT_PRINT("command is not correct:%s\n",p2);
        DMXDebugShowHelp();
        return (-1);
    }
#endif

    if (needparam)
    {
        STRSKIPBLANK(p1);

        if (p1[0] == '\n')//do not have param
        {
            MT_PRINT("command is not correct\n");
            DMXDebugShowHelp();
            return (-1);
        }

        param = (mt_u32)simple_strtoul(p1, &p1, 0);
    }

    ret = DMX_OsrDebugCtrl(cmd, cmdctrl, param, es_save_path);

    if (ret != MT_SUCCESS)
    {
        MT_PRINT("command is not correct\n");
        DMXDebugShowHelp();
        return ret;
    }

    return count;
}

static mt_s32 DMXProcRead(struct seq_file *p, mt_void *v)
{
    mt_u32 i = 0;
    DMX_ChanInfo_S *ChanInfo = NULL;
    mt_u32 samp_ctrl = 0;
    mt_u32 samp_sta = 0;

    PROC_PRINT(p, "======================================================\n");
    PROC_PRINT(p, "1. ChId  PID     Type SlotId  CFG0        CFG1  \n");

    for(i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if((reg_get_demux_slotn_cfg0(i) != 0)  &&  (reg_get_demux_slotn_cfg1(i) != 0))
        {
            ChanInfo = DMX_OsiGetChannelProc(i);

            if (!ChanInfo)
            {
                continue;
            }

            PROC_PRINT(p, "   %3d   0x%-4x  %s   %3d    0x%08x  0x%08x\n",
                  i,
                  ChanInfo->ChanPid,
                  ChanType[ChanInfo->ChanType],
                  i,
                  reg_get_demux_slotn_cfg0(i),
                  reg_get_demux_slotn_cfg1(i));
        }
    }

    PROC_PRINT(p, "\n2. REC INFO\n");
    PROC_PRINT(p, "   RECORD_EN : 0x%08x\n", reg_get_trpp_channel_record_en());
    PROC_PRINT(p, "   RecId IdxId  RecSet       BufStart     BufEnd       BufRead      BufWrite     IdxRead    IdxWrite\n");

    for (i = 0; i < DMX_REC_CNT; i++)
    {
        if((reg_get_trpp_ch_rec_start_addr(i) != 0)  &&  (reg_get_trpp_ch_rec_end_addr(i) != 0))
        {
            PROC_PRINT(p, "     %d     %d    0x%08x   0x%08x   0x%08x   0x%08x   0x%08x   0x%08x   0x%08x\n", i, i,
                  reg_get_trpp_ch_rec_set(i),
                  reg_get_trpp_ch_rec_start_addr(i),
                  reg_get_trpp_ch_rec_end_addr(i),
                  reg_get_trpp_ch_rec_rd_addr(i),
                  reg_get_trpp_ch_rec_wr_addr(i),
                  reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(i),
                  reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(i));
          }
    }

    PROC_PRINT(p, "\n3. SWTSI INFO\n");
    PROC_PRINT(p, "   SWTSIId  SWITSIStart   SWTSICtrl   SWTSISta    SWSTIDma\n");

    for (i = 0; i < DMX_RAMPORT_CNT; i++)
    {
        DMX_RamPort_Info_S *RamPortInfo = NULL;

        RamPortInfo = DMX_OsiGetSwTsiProc(i);

        if (!RamPortInfo)
        {
            continue;
        }

        if ((RamPortInfo->PhyAddr != 0) &&
            (RamPortInfo->KerAddr != 0) &&
            (RamPortInfo->BufSize != 0))
        {
            PROC_PRINT(p, "\033[31m      %d     0x%08x    0x%08x  0x%08x    %s\033[0m\n", i,
                      reg_get_swtsi_chn_lln_addr_ch_lln_addr(i),
                      reg_get_swtsi_chn_control(i),
                      reg_get_swtsi_chn_state(i),
                      ((reg_get_swtsi_chn_state(i) & 0x1) ? "BUSY" : "IDLE"));
        }
        else
        {
            PROC_PRINT(p, "      %d     0x%08x    0x%08x  0x%08x    %s\n", i,
                      reg_get_swtsi_chn_lln_addr_ch_lln_addr(i),
                      reg_get_swtsi_chn_control(i),
                      reg_get_swtsi_chn_state(i),
                      ((reg_get_swtsi_chn_state(i) & 0x1) ? "BUSY" : "IDLE"));
        }
    }


    for (i = 0; i < DMX_RAMPORT_CNT; i++)
    {
        DMX_RamPort_Info_S *RamPortInfo = NULL;

        RamPortInfo = DMX_OsiGetSwTsiProc(i);

        if (!RamPortInfo)
        {
            continue;
        }

        if ((RamPortInfo->PhyAddr != 0) &&
            (RamPortInfo->KerAddr != 0) &&
            (RamPortInfo->BufSize != 0))
        {
            PROC_PRINT(p, "\n\033[31m   SWTSI%d BUF FULL CARE INFO:\n", i);
            PROC_PRINT(p, "   SWTSI_AF_CFG0  SF( 31: 00): 0x%08x\n", reg_get_swtsi_chn_af_cfg0(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG1  SF( 63: 32): 0x%08x\n", reg_get_swtsi_chn_af_cfg1(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG2  SF( 95: 64): 0x%08x\n", reg_get_swtsi_chn_af_cfg2(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG3  SF(127: 96): 0x%08x\n", reg_get_swtsi_chn_af_cfg3(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG4  AV( 31: 00): 0x%08x\n", reg_get_swtsi_chn_af_cfg4(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG5 REC( 11: 00): 0x%08x\033[0m\n", reg_get_swtsi_chn_af_cfg5(i));
        }
        else
        {
            PROC_PRINT(p, "\n   SWTSI%d BUF FULL CARE INFO:\n", i);
            PROC_PRINT(p, "   SWTSI_AF_CFG0  SF( 31: 00): 0x%08x\n", reg_get_swtsi_chn_af_cfg0(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG1  SF( 63: 32): 0x%08x\n", reg_get_swtsi_chn_af_cfg1(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG2  SF( 95: 64): 0x%08x\n", reg_get_swtsi_chn_af_cfg2(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG3  SF(127: 96): 0x%08x\n", reg_get_swtsi_chn_af_cfg3(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG4  AV( 31: 00): 0x%08x\n", reg_get_swtsi_chn_af_cfg4(i));
            PROC_PRINT(p, "   SWTSI_AF_CFG5 REC( 11: 00): 0x%08x\n", reg_get_swtsi_chn_af_cfg5(i));

        }
    }

    PROC_PRINT(p, "\n4. TSI PORT INFO:\n");
    PROC_PRINT(p, "   TSIId  TsiSampleCtrl  TsiSampleSta  TSLock  SYNCLock  FIFOverFlow  TSCnt\n");

    samp_ctrl = reg_get_ts0_sample_ctrl();
    samp_sta = reg_get_ts0_sample_sta();

    if ((samp_sta >> 31) && ((samp_sta >> 30)&0x1))
    {

        PROC_PRINT(p, "\033[31m     0    0x%08x     0x%08x      %d        %d           %d         %d\033[0m\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }
    else
    {
        PROC_PRINT(p, "     0    0x%08x     0x%08x      %d        %d           %d         %d\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }

    samp_ctrl = reg_get_ts1_sample_ctrl();
    samp_sta = reg_get_ts1_sample_sta();

    if ((samp_sta >> 31) && ((samp_sta >> 30)&0x1))
    {

        PROC_PRINT(p, "\033[31m     1    0x%08x     0x%08x      %d        %d           %d         %d\033[0m\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }
    else
    {
        PROC_PRINT(p, "     1    0x%08x     0x%08x      %d        %d           %d         %d\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }

    samp_ctrl = reg_get_ts2_sample_ctrl();
    samp_sta = reg_get_ts2_sample_sta();

    if ((samp_sta >> 31) && ((samp_sta >> 30)&0x1))
    {

        PROC_PRINT(p, "\033[31m     2    0x%08x     0x%08x      %d        %d           %d         %d\033[0m\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }
    else
    {
        PROC_PRINT(p, "     2    0x%08x     0x%08x      %d        %d           %d         %d\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }

    samp_ctrl = reg_get_ts3_sample_ctrl();
    samp_sta = reg_get_ts3_sample_sta();

    if ((samp_sta >> 31) && ((samp_sta >> 30)&0x1))
    {

        PROC_PRINT(p, "\033[31m     3    0x%08x     0x%08x      %d        %d           %d         %d\033[0m\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }
    else
    {
        PROC_PRINT(p, "     3    0x%08x     0x%08x      %d        %d           %d         %d\n\n",
            samp_ctrl,  samp_sta, (samp_sta >> 31), ((samp_sta >> 30)&0x1),
            ((samp_sta >> 24)&0x1), ((samp_sta >> 8)&0xf));
    }

    PROC_PRINT(p, "5. TS SOURCE INFO:\n");
    PROC_PRINT(p, "   DmxId   TsiId   SOURCE  MODE\n");

    for (i = 0; i < DMX_CNT; i++)
    {
        DMX_Sub_DevInfo_S *TsiPortInfo = NULL;
        mt_u32 j = 0;
        mt_u32 play_mode = 0;
        mt_char str[][16] = {"PLAY", "REC", "PLAY+REC"};
        mt_char *pstr = NULL;

        TsiPortInfo = DMX_OsiGetTsiPortProc(i);

        switch (TsiPortInfo->PortMode)
        {
            case DMX_PORT_MODE_TUNER:
            {
                for (j = 0; j < DMX_CHANNEL_CNT; j++)
                {
                    if ((reg_get_demux_slotn_cfg0(j) != 0) &&
                        (reg_get_demux_slotn_cfg1(j) != 0) &&
                        (reg_get_demux_slotn_cfg0_src(j) == TsiPortInfo->PortId))
                    {
                        DMX_ChanInfo_S *ChanInfo = NULL;

                        ChanInfo = DMX_OsiGetChannelProc(j);

                        if (ChanInfo->DmxId  != i)
                        {
                            continue;
                        }

                        if (DRV_DMX_SLOT_PROCESS_RECORD & reg_get_demux_slotn_cfg1_process_type(j))
                        {
                            play_mode |= MT_UNF_DMX_CHAN_OUTPUT_MODE_REC;
                        }
                        else
                        {
                            play_mode |= MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
                        }

                        if (play_mode == MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY_REC)
                        {
                            break;
                        }
                    }
                }

                break;
            }
            

            case DMX_PORT_MODE_RAM:
            {
                for (j = 0; j < DMX_CHANNEL_CNT; j++)
                {
                    if ((reg_get_demux_slotn_cfg0(j) != 0) &&
                        (reg_get_demux_slotn_cfg1(j) != 0) &&
                        (reg_get_demux_slotn_cfg0_src(j) == (TsiPortInfo->PortId + 4)))
                    {
                        if (DRV_DMX_SLOT_PROCESS_RECORD & reg_get_demux_slotn_cfg1_process_type(j))
                        {
                            play_mode |= MT_UNF_DMX_CHAN_OUTPUT_MODE_REC;
                        }
                        else
                        {
                            play_mode |= MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
                        }

                        if (play_mode == MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY_REC)
                        {
                            break;
                        }
                    }
                }

                break;
            }

            default:
                break;
        }

        if (!play_mode)
        {
            continue;
        }

        if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == play_mode)
        {
            pstr = str[0];
        }
        else if (MT_UNF_DMX_CHAN_OUTPUT_MODE_REC == play_mode)
        {
            pstr = str[1];
        }
        else
        {
            pstr = str[2];
        }

        PROC_PRINT(p, "     %d       %d    %6s   %-12s\n", i, TsiPortInfo->PortId,
            (DMX_PORT_MODE_RAM == TsiPortInfo->PortMode) ? "RAM" : "TUNER" , pstr);
    }
 #if defined(CONFIG_MT_CHIP_SYMPHONY6)

#else
    PROC_PRINT(p, "\n6. DS INFO:\n");
    PROC_PRINT(p,  "   Ds debug:    %08x\n", (*(volatile mt_u32 *)reg_dmx_tsi_ds_dsch_for_symphony2));
    PROC_PRINT(p,  "   Ds killCount:%08x\n", (*(volatile mt_u32 *)reg_dmx_tsi_ds_kill_cnt_for_symphony2));
    PROC_PRINT(p,  "   BitLittle:   %08x\n", (*(volatile mt_u32 *)reg_dmx_ds_big_little_endian));

    for (i = 0; i < 16; i++)
    {
        PROC_PRINT(p,  "   Ds core:     %08x   %08x   %08x   %08x   %08x   %08x\n",
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_ds_chn_tscfg_sym2 + 0x40*i)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_ds_core_sym2 + 0x40*i)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_aes_ive_sym2 + 0x40*i)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_ades_disc_mode_sym2 + 0x40*i)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_ades_pktmode_sym2 + 0x40*i)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_algo_cw_ive_port + 0x40*i)));
   }

   PROC_PRINT( p, "   Ds keyslot:\n");

   for (i = 0; i < 16; i++)
   {
        PROC_PRINT( p, "    %08x",(*(volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab + i*4)));
        if(i == 8)   PROC_PRINT( p, "\n");
   }
#endif
   PROC_PRINT(p, "\n======================================================\n");

   return MT_SUCCESS;
}

static mt_void PortTypeToString(mt_char *str, mt_u32 len, mt_u32 Type)

{
    switch (Type)
    {
        case MT_UNF_DMX_PORT_TYPE_PARALLEL_BURST :
            strncpy(str, "PARALLEL_BURST",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID :
            strncpy(str, "PARALLEL_VALID",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_204 :
            strncpy(str, "PARALLEL_NOSYNC_204",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204 :
            strncpy(str, "PARALLEL_NOSYNC_188_204",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_SERIAL :
            strncpy(str, "SERIAL",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_SERIAL2BIT :
            strncpy(str, "SERIAL2BIT",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_SERIAL_NOSYNC :
            strncpy(str, "SERIAL_NOSYNC",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_SERIAL2BIT_NOSYNC :
            strncpy(str, "SERIAL2BIT_NOSYNC",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_USER_DEFINED :
            strncpy(str, "USER_DEFINED",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_AUTO :
            strncpy(str, "AUTO",len);
            break;

        case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188 :
        default :
            strncpy(str, "PARALLEL_NOSYNC_188",len);
    }
}

static mt_s32 DMXPortProcRead_debug(struct seq_file *p, mt_void *v)
{
    mt_u32                      i;
    mt_u32                      j;
    DMX_Proc_RamPort_DescInfo_S DescInfo[DMX_RAMPORT_CNT];
    DMX_Proc_RamPort_BPStatus_S BPStatus[DMX_RAMPORT_CNT];
    mt_u32 * pFiled = NULL;



    memset(&DescInfo[0],0x0,sizeof(DescInfo));
    memset(&BPStatus[0],0x0,sizeof(BPStatus));
    for (i = 0; i < 1; i++)
    {
        DMX_OsiRamPortGetDescInfo(i,&DescInfo[i]);
        DMX_OsiRamPortGetBPStatus(i,&BPStatus[i]);
    }
    PROC_PRINT(p,"\n-----------------------RAM Port inner debug information----------------------------\n");
    PROC_PRINT(p, "    information\\Port ID          128         129         130         131         132\n");
    PROC_PRINT(p,"Desc information:\n");
    for ( i = 0 ; i < 9 ; i++ )
    {

        PROC_PRINT(p,"%s     ",DescProcList[i]);
        for (j = 0; j < DMX_RAMPORT_CNT; j++)
        {
            pFiled = (mt_u32 *)&(DescInfo[j].DescPhyAddr);
            PROC_PRINT(p,"0x%-8x  ", *(pFiled + i) );
        }
        PROC_PRINT(p,"\n");
    }

    PROC_PRINT(p,"\nBack Pressure information:\n");
    for ( i = 0 ; i < 2 ; i++ )
    {
        PROC_PRINT(p,"%s     ",BPProcList[i]);
        for (j = 0; j < DMX_RAMPORT_CNT; j++)
        {
            pFiled = (mt_u32 *)&(BPStatus[j].FQLow32BPEn);
            PROC_PRINT(p,"0x%-8x  ", *(pFiled + i) );
        }
        PROC_PRINT(p,"\n");
    }

    for ( i = 2 ; i < 10 ; i++ )
    {
        PROC_PRINT(p,"%s     ",BPProcList[i]);
        for (j = 0; j < DMX_RAMPORT_CNT; j++)
        {
            pFiled = (mt_u32 *)&(BPStatus[j].FQLow32BPEn);
            PROC_PRINT(p," %-9d  ", *(pFiled + i) );
        }
        PROC_PRINT(p,"\n");
    }

    return MT_SUCCESS;
}

static mt_s32 DMXPortProcRead(struct seq_file *p, mt_void *v)
{
    mt_u32                      i;
    mt_u32                      TsPacks;
    mt_u32                      ErrTsPacks;
    MT_UNF_DMX_PORT_ATTR_S      PortAttr;
    mt_u32                      PortID;
    MT_UNF_DMX_TSO_PORT_E       TSIAttachTSO;

    PROC_PRINT(p, "\n-------------------------------TSI port-----------------------------------\n");
    PROC_PRINT(p, " Id   AllTsCnt   ErrTsCnt  Lock/lost  ClkReverse   BitSel      PortName     Type\n");

    for (i = 0; i < DMX_TUNERPORT_CNT; i++)
    {
        mt_char str[32] = "";
        mt_char BitSel[16]  = "";
        mt_char AttachTSO[16]  = "";
        if ( DMX_IFPORT_CNT != 0 )
        {
            PortID = i + MT_UNF_DMX_PORT_TSI_0 - DMX_IFPORT_CNT;
        }
        else
        {
            PortID = i + MT_UNF_DMX_PORT_TSI_0;
        }
        MT_DRV_DMX_TunerPortGetAttr(i, &PortAttr);

        MT_DRV_DMX_TunerPortGetPacketNum(i+MT_UNF_DMX_PORT_TSI_0, &TsPacks, &ErrTsPacks);

        PortTypeToString(str, sizeof(str),PortAttr.enPortType);
        if ( PortAttr.u32SerialBitSelector == 0x0 )
        {
            strncpy(BitSel, "D7",sizeof(BitSel));
        }
        else
        {
            strncpy(BitSel, "D0",sizeof(BitSel));
        }

        if ( MT_DRV_DMX_IsTSIAttachTSO(i, &TSIAttachTSO))
        {
            snprintf(AttachTSO, sizeof(AttachTSO),"%d",TSIAttachTSO);
        }
        else
        {
            strncpy(AttachTSO, "-",sizeof(AttachTSO));
        }

        PROC_PRINT(p, "%3u    0x%-8x 0x%-4x     %u/%u        %d          %s           TSI%d       %s\n",
            PortID, TsPacks, ErrTsPacks, PortAttr.u32SyncLockTh, PortAttr.u32SyncLostTh, PortAttr.u32TunerInClk,BitSel, i, str);

    }

    if ( DebugEn )
    {
        DMXPortProcRead_debug(p,v);
    }

#ifdef DMX_TAG_DEAL_SUPPORT
    {
        PROC_PRINT(p, "\n----------------------TagDeal port-------------------------------\n");
        PROC_PRINT(p, " Id  Enable  SyncMode TagLen              Tag               \n");

        for (i = 0; i < DMX_TAG_MAX_TS_WAY; i ++)
        {
            MT_UNF_DMX_TAG_ATTR_S TagPortAttrs;
            mt_u32 * pu32Low = (mt_u32 *)&(TagPortAttrs.au8Tag[0]);                   /* 0~4 bytes */
            mt_u32 * pu32Mid = (mt_u32 *)&(TagPortAttrs.au8Tag[4]);                /* 4~8 bytes */
            mt_u32 * pu32High = (mt_u32 *)&(TagPortAttrs.au8Tag[4 + 4]);         /* 8~12 bytes*/

            DMX_OsiGetTagPortAttr(i, &TagPortAttrs);

            PROC_PRINT(p, "  %u     %u       %u       %u     0x%08x%08x%08x\n", i, TagPortAttrs.bEnabled, TagPortAttrs.enSyncMod, TagPortAttrs.u32TagLen,
                *pu32Low, *pu32Mid, *pu32High);
        }
    }
#endif
    return MT_SUCCESS;
}


extern ulong mt_get_vdec_base(void);
static __inline mt_u32 dmx_inl(ulong port)
{
    return *((volatile mt_u32 *)(port));
}

static mt_s32 DMXChanProcRead(struct seq_file *p, mt_void *v)
{
    mt_u32  ChanId;

    PROC_PRINT(p, "ChId  DmxId  PID      Type  Mod  Stat\t KeyId    Acquire(Try/Ok)    Release   BufSize   DescSize   BufRead     BufWrite   DescRead   DescWrite   BufUsed DescUsed Overflow\n");

    for (ChanId = 0; ChanId < DMX_CHANNEL_CNT; ChanId++)
    {
        DMX_ChanInfo_S *ChanInfo;
        mt_char         KeyStr[8]       = "--";
        DMX_Proc_ChanBuf_S  BufInfo;
        mt_u32              UsedPercent = 0;
        mt_u32              DescUsedPercent = 0;
        mt_u32              BufSize;
        mt_u32              DescSize;

        ChanInfo = DMX_OsiGetChannelProc(ChanId);

        if (!ChanInfo)
        {
            continue;
        }

        if (ChanInfo->KeyId < DMX_KEY_CNT)
        {
            snprintf(KeyStr, 8,"%-2u", ChanInfo->KeyId);
        }

        if (MT_SUCCESS != DMX_OsiGetChanBufProc(ChanId, &BufInfo))
        {
            continue;
        }

        if( (BufInfo.DataRead == 0) && (ChanId == 0))
        {
            BufInfo.DataRead =  dmx_inl(mt_get_vdec_base() + 0xb0) * 8; //vdec_rp
        }


        BufSize = BufInfo.DescDepth * BufInfo.BlockSize / 1024;
        DescSize = BufInfo.DescSize / 1024;


        switch (ChanInfo->ChanType)
        {
            case MT_UNF_DMX_CHAN_TYPE_AUD:
            case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
            case MT_UNF_DMX_CHAN_TYPE_VID:
            case MT_UNF_DMX_CHAN_TYPE_HW_PES:
            {
                if (BufInfo.DataRead <= BufInfo.DataWrite)
                {
                    UsedPercent = (BufInfo.DataWrite - BufInfo.DataRead) * 100 / BufInfo.BlockSize;
                }
                else
                {
                    UsedPercent = (BufInfo.BlockSize + BufInfo.DataWrite - BufInfo.DataRead) * 100 / BufInfo.BlockSize;
                }

                if (BufInfo.DescRead <= BufInfo.DescWrite)
                {
                    DescUsedPercent = (BufInfo.DescWrite - BufInfo.DescRead) * 100 / BufInfo.DescSize;
                }
                else
                {
                    DescUsedPercent = (BufInfo.DescSize + BufInfo.DescWrite - BufInfo.DescRead) * 100 / BufInfo.DescSize;
                }

                PROC_PRINT(p, "%-2u    %u      0x%-4x   %s   %s  %s\t %s   %10u/%-10u  %-8u  %6uK  %6uK     0x%-8x  0x%-8x 0x%-8x 0x%-8x   %3u%%    %3u%%      %2u\n",
                                ChanId,
                                ChanInfo->DmxId,
                                ChanInfo->ChanPid,
                                ChanType[ChanInfo->ChanType],
                                (ChanInfo->ChanOutMode == MT_UNF_DMX_CHAN_OUTPUT_MODE_BUTT)?"UNO":OutMode[ChanInfo->ChanOutMode],
                                (ChanInfo->ChanStatus == MT_UNF_DMX_CHAN_CLOSE)  ? "CLOSE" : "OPEN",
                                KeyStr,
                                ChanInfo->u32TotolAcq,
                                ChanInfo->u32HitAcq,
                                ChanInfo->u32Release,
                                BufSize,
                                DescSize,
                                BufInfo.DataRead,
                                BufInfo.DataWrite,
                                BufInfo.DescRead,
                                BufInfo.DescWrite,
                                UsedPercent,
                                DescUsedPercent,
                                BufInfo.Overflow
                     );
            }
            break;

            case MT_UNF_DMX_CHAN_TYPE_SEC:
            case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
            case MT_UNF_DMX_CHAN_TYPE_POST:
            case MT_UNF_DMX_CHAN_TYPE_PES:
            {
                if (BufInfo.SoftDataRead <= BufInfo.DataWrite)
                {
                    UsedPercent = (BufInfo.DataWrite - BufInfo.SoftDataRead) * 100 / BufInfo.BlockSize;
                }
                else
                {
                    UsedPercent = (BufInfo.BlockSize + BufInfo.DataWrite - BufInfo.SoftDataRead) * 100 / BufInfo.BlockSize;
                }

                if (BufInfo.SoftDescRead <= BufInfo.DescWrite)
                {
                    DescUsedPercent = (BufInfo.DescWrite - BufInfo.SoftDescRead) * 100 / BufInfo.DescSize;
                }
                else
                {
                    DescUsedPercent = (BufInfo.DescSize + BufInfo.DescWrite - BufInfo.SoftDescRead) * 100 / BufInfo.DescSize;
                }

                PROC_PRINT(p, "%-2u    %u      0x%-4x   %s   %s  %s\t %s   %10u/%-10u  %-8u  %6uK  %6uK     0x%-8x  0x%-8x 0x%-8x 0x%-8x   %3u%%    %3u%%      %2u\n",
                                ChanId,
                                ChanInfo->DmxId,
                                ChanInfo->ChanPid,
                                ChanType[ChanInfo->ChanType],
                                (ChanInfo->ChanOutMode == MT_UNF_DMX_CHAN_OUTPUT_MODE_BUTT)?"UNO":OutMode[ChanInfo->ChanOutMode],
                                (ChanInfo->ChanStatus == MT_UNF_DMX_CHAN_CLOSE)  ? "CLOSE" : "OPEN",
                                KeyStr,
                                ChanInfo->u32TotolAcq,
                                ChanInfo->u32HitAcq,
                                ChanInfo->u32Release,
                                BufSize,
                                DescSize,
                                BufInfo.SoftDataRead,
                                BufInfo.DataWrite,
                                BufInfo.SoftDescRead,
                                BufInfo.DescWrite,
                                UsedPercent,
                                DescUsedPercent,
                                BufInfo.Overflow
                     );
            }
            break;

            case MT_UNF_DMX_CHAN_TYPE_REC:
            {
                PROC_PRINT(p, "%-2u    %u      0x%-4x   %s   %s  %s\t %s   %10s/%-10s  %-8s  %6s   %6s        %-8s    %-8s   %-8s    %-8s  %4s      %2s       %2s\n",
                                ChanId,
                                ChanInfo->DmxId,
                                ChanInfo->ChanPid,
                                ChanType[ChanInfo->ChanType],
                                (ChanInfo->ChanOutMode == MT_UNF_DMX_CHAN_OUTPUT_MODE_BUTT)?"UNO":OutMode[ChanInfo->ChanOutMode],
                                (ChanInfo->ChanStatus == MT_UNF_DMX_CHAN_CLOSE)  ? "CLOSE" : "OPEN",
                                KeyStr,
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--",
                                "--"
                     );
            }
            break;

            default:
                break;
        }
    }

    return MT_SUCCESS;
}

mt_s32 DMXChanBufProcRead_debug(struct seq_file *p, mt_void *v)
{
    mt_u32                      i;
    mt_u32                      j;
    mt_u32                      FiledCount;
    mt_u32                      PrintCount;
    mt_u32                      ChanId;
    mt_u32                      FQID;
    mt_u32                      OQID;
    mt_u32                      * pFiled = NULL;
    DMX_ChanInfo_S              *ChanInfo;
    mt_u32                      ChannelOQID[DMX_CHANNEL_CNT]  ;



    memset(&OQInfo[0],0x0,sizeof(OQInfo));
    memset(&FQInfo[0],0x0,sizeof(FQInfo));
    memset(&ChannelDF[0],0x0,sizeof(ChannelDF));

    for ( ChanId = 0 ; ChanId < DMX_CHANNEL_CNT ; ChanId++ )
    {
        ChannelOQID[ChanId] = INVALID_OQ_ID;
    }

    for (ChanId = 0; ChanId < DMX_CHANNEL_CNT; ChanId++)
    {
        ChanInfo = DMX_OsiGetChannelProc(ChanId);
        if (!ChanInfo)
        {
            break;
        }

        if ( ChanInfo->ChanOutMode == MT_UNF_DMX_CHAN_OUTPUT_MODE_REC)
        {
            OQID = DMX_CHANNEL_CNT + ChanInfo->DmxId*2;
        }
        else
        {
            OQID = ChanId;
        }
        ChannelOQID[ChanId] = OQID;

        DMX_OsiGetOQInfo(OQID, &OQInfo[OQID]);
        FQID = OQInfo[OQID].FQCfg;
        DMX_OsiGetFQInfo(FQID, &FQInfo[FQID]);
        DMX_OsiGetChannelDataFlow(ChanId, &ChannelDF[ChanId]);
    }


    PROC_PRINT(p,"\n-----------Channel buf  inner debug information--------------\n");

    ChanId = 0;
    while ( ChanId <  DMX_CHANNEL_CNT)
    {
        if ( ChannelOQID[ChanId] == INVALID_OQ_ID )
        {

            ChanId++;
            continue;
        }
        //=============================print the title=========================
        PROC_PRINT(p, " information\\Channel ID      ");
        PrintCount = 0;
        for (i = ChanId; i< DMX_CHANNEL_CNT; i++)
        {
            if ( ChannelOQID[i] == INVALID_OQ_ID  )
            {
                continue;
            }
            PROC_PRINT(p," %-9d  ", i );
            PrintCount++;
            if ( PrintCount >= 7 )
            {
                break;
            }
        }
        PROC_PRINT(p," Comments\n");
        //=============================print OQ information=========================
        PROC_PRINT(p,"OQ information:\n");
        for ( FiledCount = 0 ; FiledCount < 13 ; FiledCount++ )
        {
            PROC_PRINT(p,"%s     ",OQProcInfo[FiledCount]);
            PrintCount = 0;
            for (j = ChanId; j <DMX_CHANNEL_CNT; j++)
            {
                OQID = ChannelOQID[j];
                if ( OQID == INVALID_OQ_ID  )
                {
                    continue;
                }

                pFiled = (mt_u32 *)&(OQInfo[OQID].OQID);
                PROC_PRINT(p,"0x%-8x  ", *(pFiled + FiledCount) );
                PrintCount++;
                if ( PrintCount >= 7 )
                {
                    break;
                }

            }
            PROC_PRINT(p,"%s\n",OQComments[FiledCount]);
        }
        //=============================print FQ information=========================
        PROC_PRINT(p,"\nFQ information:\n");
        for ( FiledCount = 0 ; FiledCount < 6 ; FiledCount++ )
        {
            PROC_PRINT(p,"%s     ",FQProcInfo[FiledCount]);
            PrintCount = 0;
            for (j = ChanId; j <DMX_CHANNEL_CNT; j++)
            {
                OQID = ChannelOQID[j];
                if ( OQID == INVALID_OQ_ID  )
                {
                    continue;
                }

                FQID = OQInfo[OQID].FQCfg;
                pFiled = (mt_u32 *)&(FQInfo[FQID].FQUse);
                PROC_PRINT(p,"0x%-8x  ", *(pFiled + FiledCount) );
                PrintCount++;
                if ( PrintCount >= 7 )
                {
                    break;
                }


            }
            PROC_PRINT(p,"%s\n",FQComments[FiledCount]);
        }

        //=============================print Data flow information=========================
        PROC_PRINT(p,"\nChannel Data flow:\n");

        for ( FiledCount = 0 ; FiledCount < 4 ; FiledCount++ )
        {
            PROC_PRINT(p,"%s     ",ChanDataFlow[FiledCount]);
            PrintCount = 0;
            for (j = ChanId; j <DMX_CHANNEL_CNT; j++)
            {
                if ( ChannelOQID[j] == INVALID_OQ_ID  )
                {
                    continue;
                }

                pFiled = (mt_u32 *)&(ChannelDF[j].PIDTsPacket);
                PROC_PRINT(p,"0x%-8x  ", *(pFiled + FiledCount) );
                PrintCount++;
                if ( PrintCount >= 7 )
                {
                    break;
                }
            }
            PROC_PRINT(p,"%s\n",ChanDFComments[FiledCount]);
        }

        ChanId = j;
        PROC_PRINT(p,"\n-------------------------------------------------------------\n");

    }
    return 0;

}

static mt_s32 DMXFilterProcRead(struct seq_file *p, mt_void *v)
{
    mt_u32 FilterId;
    mt_u32 i;
    DMX_ChanInfo_S *ChanInfo = NULL;

    PROC_PRINT(p, "FltId  ChId    PID    BufId  Depth   FltCfg      FUNITId      Value       Mask        Mode\n");

    for (FilterId = 0; FilterId < DMX_FILTER_CNT; FilterId++)
    {
        DMX_FilterInfo_S   *FilterInfo;

        FilterInfo = DMX_OsiGetFilterProc(FilterId);

        if (!FilterInfo)
        {
            continue;
        }

        ChanInfo = DMX_OsiGetChannelProc(FilterInfo->ChanId);

        if (!ChanInfo)
        {
            continue;
        }

        PROC_PRINT(p, "%2u   %4u      0x%-4x  %3u    %2u     0x%08x", FilterId, FilterInfo->ChanId, 
            reg_get_demux_slotn_cfg0_pid(FilterInfo->ChanId), ChanInfo->secBuffId, FilterInfo->Depth, reg_get_filtern_config(FilterId));

        for (i = 0; i < 4; i++)
        {
            if (DMX_INVALID_CHAN_ID != FilterInfo->FilterBuffId[i])
            {
                PROC_PRINT(p, "   %3d         0x%08x  0x%08x  0x%08x\n",
                    FilterInfo->FilterBuffId[i],
                    reg_get_funit_filter_data(FilterInfo->FilterBuffId[i]),
                    reg_get_funit_filter_mask(FilterInfo->FilterBuffId[i]),
                    reg_get_funit_filter_mode(FilterInfo->FilterBuffId[i]));
                PROC_PRINT(p, "\t\t\t\t\t       ");                
            }
        }

        PROC_PRINT(p, "\n");
    }

    return 0;
}


static mt_s32 DMXRecProcRead(struct seq_file *p, mt_void *v)
{
    mt_u32 RecId;

    PROC_PRINT(p, "RecId  Type      Descramed  Status  BufSize  IdxSize   BufRead    BufWrite    IdxRead    IdxWrite   BufUsed  IdxUsed  Overflow\n");

    for (RecId = 0; RecId < DMX_REC_CNT; RecId++)
    {
        DMX_Proc_Rec_BufInfo_S  RecInfo;
        mt_char                *Type;
        mt_char                *Status;
        mt_u32                  BufSize;
        mt_u32                  IdxSize;
        mt_u32                  UsedPercent;
        mt_u32                  IdxUsedPercent;

        if (MT_SUCCESS != DMX_OsiGetDmxRecProc(RecId, &RecInfo))
        {
            continue;
        }

        switch (RecInfo.RecType)
        {
            case MT_UNF_DMX_REC_TYPE_SELECT_PID :
                Type = "singlePid";
                break;

            case MT_UNF_DMX_REC_TYPE_ALL_PID :
                Type = "allPid";
                break;

            default :
                continue;
        }

        BufSize = RecInfo.BufSize / 1024;
        IdxSize = RecInfo.IndexSize / 1024;
        Status  = RecInfo.RecStatus ? "start" : "stop ";

        if (RecInfo.BufRead <= RecInfo.BufWrite)
        {
            UsedPercent = (RecInfo.BufWrite - RecInfo.BufRead) * 100 / RecInfo.BufSize;
        }
        else
        {
            UsedPercent = (RecInfo.BufSize + RecInfo.BufWrite - RecInfo.BufRead) * 100 / RecInfo.BufSize;
        }


        if (RecInfo.IndexRead <= RecInfo.IndexWrite)
        {
            IdxUsedPercent = (RecInfo.IndexWrite - RecInfo.IndexRead) * 100 / RecInfo.IndexSize;
        }
        else
        {
            IdxUsedPercent = (RecInfo.IndexSize + RecInfo.IndexWrite - RecInfo.IndexRead) * 100 / RecInfo.IndexSize;
        }

        PROC_PRINT(p, "%3u    %s      %u     %s  %6uK %6uK     0x%-8x 0x%-8x  0x%-6x   0x%-6x    %2u%%      %2u%%        %u\n",
            RecId, Type, RecInfo.Descramed, Status, BufSize, IdxSize, RecInfo.BufRead, RecInfo.BufWrite,
            RecInfo.IndexRead, RecInfo.IndexWrite, UsedPercent, IdxUsedPercent, RecInfo.Overflow);
    }

    return MT_SUCCESS;
}
#endif

static mt_u32 DMXDumpAllRegister(mt_u32 tag)
{
    mt_u32 i = 0;

    printk("Demux all register:  \n");
    printk( "======================================================\n");
    printk( "1.  SLOT NUM:| CFG0  |  CFG1  |\n");
    for(i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if((reg_get_demux_slotn_cfg0(i) != 0)  &&  (reg_get_demux_slotn_cfg1(i) != 0))
        {
            printk( "    SLOT[%02d]:%08x %08x\n", i,
                  reg_get_demux_slotn_cfg0(i),
                  reg_get_demux_slotn_cfg1(i));
        }
    }

    printk( "2.  BUF NUM:| STADDR|  SIZE  |DISCWPTR| INT CFG|DATAWPTR| INT STA|CSEC LEN|   CRC  |\n");
    for(i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if((reg_get_bufn_staddr(i) != 0)  &&  (reg_get_bufn_size(i) != 0))
        {
            printk( "    BUF[%02d]:%08x %08x %08x %08x %08x %08x %08x %08x\n", i,
                  reg_get_bufn_staddr(i),
                  reg_get_bufn_size(i),
                  reg_get_bufn_disc_wptr(i),
                  reg_get_bufn_ts_int_cfg(i),
                  reg_get_bufn_data_wptr(i),
                  reg_get_bufn_int_sta(i),
                  reg_get_bufn_cursec_len(i),
                  reg_get_bufn_crc_value(i));
        }
    }

    for(i = 0; i < DMX_CHANNEL_CNT - 7; i = i + 8)
    {
        if((reg_get_filtern_config(i) != 0) ||(reg_get_filtern_config(i+1) != 0))
        {
            printk( "3.  FITLER %02d-%02d:%08x %08x %08x %08x %08x %08x %08x %08x\n", i, i + 7,
                  reg_get_filtern_config(i),
                  reg_get_filtern_config(i + 1),
                  reg_get_filtern_config(i + 2),
                  reg_get_filtern_config(i + 3),
                  reg_get_filtern_config(i + 4),
                  reg_get_filtern_config(i + 5),
                  reg_get_filtern_config(i + 6),
                  reg_get_filtern_config(i + 7));
        }
    }

    printk( "4.  FUNIT NUM:|  DATA  |  MASK  |  MODE  |\n");
    for(i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        if(reg_get_funit_filter_mask(i) != 0)
        {
            printk( "    FUNIT[%03d]:%08x %08x %08x\n", i,
                  reg_get_funit_filter_data(i),
                  reg_get_funit_filter_mask(i),
                  reg_get_funit_filter_mode(i));
            i++;
            printk( "    FUNIT[%03d]:%08x %08x %08x\n", i,
                  reg_get_funit_filter_data(i),
                  reg_get_funit_filter_mask(i),
                  reg_get_funit_filter_mode(i));
            i++;
            printk( "    FUNIT[%03d]:%08x %08x %08x\n", i,
                  reg_get_funit_filter_data(i),
                  reg_get_funit_filter_mask(i),
                  reg_get_funit_filter_mode(i));
            i++;
            printk( "    FUNIT[%03d]:%08x %08x %08x\n", i,
                  reg_get_funit_filter_data(i),
                  reg_get_funit_filter_mask(i),
                  reg_get_funit_filter_mode(i));
        }
    }
    printk( "=====================TRPP Regs=========================\n");
    printk( "TRPP_CHANNEL_PARSE_EN:0x%08x\n", reg_get_trpp_channel_parse_en());
    printk( "TRPP_CHANNEL_RECORD_EN:0x%08x\n", reg_get_trpp_channel_record_en());
    printk( "TRPP_ESBUF_CH : 0x%08x\n",reg_get_trpp_esbuf_ch());

    printk( "5.  TRPP CH:|PROPERTY|  PARSE |  CODE1 | CODEM1 |  CODE2 |\
    CODEM2 |DSCSTART|DATSTART| DSCREND| DATAEND|DSCRP_RD| DATA RD|DSCRP_WR| DATA_WR|\n");
    for(i = 0; i < DMX_AV_CHANNEL_CNT; i++)
    {
        if((reg_get_trpp_ch_property(i) != 0)  &&  (reg_get_trpp_ch_parse_set(i) != 0))
        {
            printk( "    TRPP[%02d]:%08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x %08x \n", i,
                  reg_get_trpp_ch_property(i),
                  reg_get_trpp_ch_parse_set(i),
                  reg_get_trpp_ch_start_code1(i),
                  reg_get_trpp_ch_frm_start_code_m1(i),
                  reg_get_trpp_ch_start_code2(i),
                  reg_get_trpp_ch_frm_start_code_m2(i),
                  reg_get_trpp_ch_dscrpt_start_addr(i),
                  reg_get_trpp_ch_data_start_addr(i),
                  reg_get_trpp_ch_dscrpt_end_addr(i),
                  reg_get_trpp_ch_data_end_addr(i),
                  reg_get_trpp_ch_dscrpt_rd_addr(i),
                  reg_get_trpp_ch_data_rd_addr(i),
                  reg_get_trpp_ch_dscrpt_wr_addr(i),
                  reg_get_trpp_ch_data_wr_addr(i));
        }
    }
    printk( "6.  TRPP_REC CH:|REC_SET| DATSTART| DATAEND| DATA RD| DATA WR|\n");
    for(i = 0; i < 4; i++)
    {
        if((reg_get_trpp_ch_rec_start_addr(i) != 0)  &&  (reg_get_trpp_ch_rec_end_addr(i) != 0))
        {
            printk( "    TRPP_REC[%02d]:%08x %08x %08x %08x %08x %08x %08x\n", i,
                  reg_get_trpp_ch_rec_set(i),
                  reg_get_trpp_ch_rec_start_addr(i),
                  reg_get_trpp_ch_rec_end_addr(i),
                  reg_get_trpp_ch_rec_rd_addr(i),
                  reg_get_trpp_ch_rec_wr_addr(i),
                  reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(i),
                  reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(i));
          }
    }
	
    printk( "TS0:  %08x   %08x\n", reg_get_ts0_sample_ctrl(),  reg_get_ts0_sample_sta());
    printk( "TS1:  %08x   %08x\n", reg_get_ts1_sample_ctrl(),  reg_get_ts1_sample_sta());
    printk( "TS2:  %08x   %08x\n", reg_get_ts2_sample_ctrl(),  reg_get_ts2_sample_sta());
    printk( "TS3:  %08x   %08x\n", reg_get_ts3_sample_ctrl(),  reg_get_ts3_sample_sta());
#if defined(CONFIG_MT_CHIP_SYMPHONY6) 

#else
    printk( "Ds debug:    %08x\n", (*(volatile mt_u32 *)reg_dmx_tsi_ds_kill_cnt_for_symphony2));
    printk( "BitLittle:      %08x\n", (*(volatile mt_u32 *)reg_dmx_ds_big_little_endian));
    printk( "Ds core       %08x   %08x   %08x   %08x   %08x   %08x\n",
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_ds_chn_tscfg_sym2),
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_ds_core_sym2),
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_aes_ive_sym2),
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_ades_disc_mode_sym2),
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_ades_pktmode_sym2),
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_algo_cw_ive_port));
   printk( "Ds keyslot    %08x   %08x   %08x   %08x   %08x   %08x\n",
                                                              (*(volatile mt_u32 *)reg_dmx_tsi_keyslot_tab),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab+4)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab+8)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab+12)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab+16)),
                                                              (*(volatile mt_u32 *)(reg_dmx_tsi_keyslot_tab+20)));
    printk( "======================================================\n");
#endif
    return MT_SUCCESS;
}

static void DMXDataProcListInit(demux_aud_ch_list *p)
{
    int i = 0;
    
    g_channel_reset = 1;

    p->main_chan_num = 0;
    p->main_select_index = -1;
    p->main_pre_index = -1;
    p->trick_seek_to_play = 0;
    p->desc_chan_id = DMX_INVALID_CHAN_ID;
    for(i = 0; i < DMX_PROC_CH_MAX; i++) {
        p->main_chan_id[i] = DMX_INVALID_CHAN_ID;
    }

    sema_init(&p->aud_track_sem, 0);
    sema_init(&p->trick_seek_sem, 0);
}

/**************************** external functions ******************************/

static mt_s32 DMXGlobalIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_GET_POOLBUF_ADDR:
        {
            DMX_MMZ_BUF_S *Param = (DMX_MMZ_BUF_S*)arg;

            ret = MT_DRV_DMX_GetPoolBufAddr(Param);
            break;
        }

        case CMD_DEMUX_GET_CAPABILITY:
        {
            MT_UNF_DMX_CAPABILITY_S *Param = (MT_UNF_DMX_CAPABILITY_S*)arg;

            ret = MT_DRV_DMX_GetCapability(Param);

            break;
        }
        case CMD_DEMUX_SET_PUSI:
        {
            MT_UNF_DMX_PUSI_SET_S *pPara = (MT_UNF_DMX_PUSI_SET_S *)arg;
            ret = MT_DRV_DMX_SetPusi(pPara->bPusi);
            break;
        }
        case CMD_DEMUX_SET_TEI:
        {
            MT_UNF_DMX_TEI_SET_S *pPara = (MT_UNF_DMX_TEI_SET_S *)arg;
            ret = MT_DRV_DMX_SetTei(pPara);
            break;
        }
        case CMD_DEMUX_TSI_ATTACH_TSO:
        {
            MT_UNF_DMX_TSI_ATTACH_TSO_S *pPara = (MT_UNF_DMX_TSI_ATTACH_TSO_S *)arg;
            ret = MT_DRV_DMX_TSIAttachTSO(pPara);
            break;
        }

        case CMD_DEMUX_GET_REGISTER:
        {
            ret = MT_DRV_DMX_ReadRegister(*(mt_u32*)arg);
            break;
        }

        case CMD_DEMUX_SET_REGISTER:
        {
            DMX_SetRegister_S *pPara = (DMX_SetRegister_S *)arg;
            MT_DRV_DMX_WriteRegister(pPara->u32RegisterOffset, pPara->u32RegisterValue);
            break;
        }
        case CMD_DEMUX_DUMP_ALL_REGISTER:
        {
            ret = DMXDumpAllRegister(*(mt_u32*)arg);
            break;
        }

		case CMD_DEMUX_SOFT_RESET:
		{
			DMX_OsiSoftReset();
			break;
		}

		case CMD_DEMUX_HAEDWARE_INIT:
		{
			DMX_OsiDeviceHardInit();
			break;
		}
		
		case CMD_DEMUX_DMX_SET_PORTID:
		{
			DMX_Set_PortId_S *pPara = (DMX_Set_PortId_S *)arg;
			ret = MT_DRV_DMX_SetPortId(pPara->hChannel, pPara->PortMode, pPara->PortId);
			break;
		}

		case CMD_DEMUX_GET_CLK:
		{
			MT_UNF_DMX_TSI_CLK_E *pPara = (MT_UNF_DMX_TSI_CLK_E *)arg;
			ret = DMX_OsiGetTsiClk(pPara);

			break;
		}
		
		case CMD_DEMUX_SET_CLK:
		{
			MT_UNF_DMX_TSI_CLK_E *pPara = (MT_UNF_DMX_TSI_CLK_E *)arg;
			ret = DMX_OsiSetTsiClk(*pPara);

			break;
		}
		
        default:
        {
            //MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
            //PROC_PRINT(p, "unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMXPortIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_PORT_GET_ATTR:
        {
            DMX_Port_GetAttr_S *Param = (DMX_Port_GetAttr_S*)arg;

            if (DMX_PORT_MODE_RAM == Param->PortMode)
            {
                ret = MT_DRV_DMX_RamPortGetAttr(Param->PortId, &Param->PortAttr);
            }
            else
            {
                ret = MT_DRV_DMX_TunerPortGetAttr(Param->PortId, &Param->PortAttr);
            }

            break;
        }

        case CMD_DEMUX_PORT_SET_ATTR:
        {
            DMX_Port_SetAttr_S *Param = (DMX_Port_SetAttr_S*)arg;

            if (DMX_PORT_MODE_RAM == Param->PortMode)
            {
                ret = MT_DRV_DMX_RamPortSetAttr(Param->PortId, &Param->PortAttr);
            }
            else
            {
                ret = MT_DRV_DMX_TunerPortSetAttr(Param->PortId, &Param->PortAttr);
            }

            break;
        }

        case CMD_DEMUX_TSO_PORT_GET_ATTR:
        {
            DMX_TSO_Port_Attr_S *Param = (DMX_TSO_Port_Attr_S*)arg;
            ret = MT_DRV_DMX_TSOPortGetAttr(Param->PortId, &Param->PortAttr);
            break;
        }

        case CMD_DEMUX_TSO_PORT_SET_ATTR:
        {
            DMX_TSO_Port_Attr_S *Param = (DMX_TSO_Port_Attr_S*)arg;
            ret = MT_DRV_DMX_TSOPortSetAttr(Param->PortId, &Param->PortAttr);
            break;
        }

        case CMD_DEMUX_DMX_GET_TAG_ATTR:
        {
            #ifdef DMX_TAG_DEAL_SUPPORT
            DMX_Tag_GetAttr_S *Param = (DMX_Tag_GetAttr_S*)arg;

            ret = MT_DRV_DMX_GetTagAttr(Param->DmxId, &Param->TagAttr);
            #else
            ret = MT_ERR_NOT_SUPPORT_TAGDEAL;
            #endif
            break;
        }

        case CMD_DEMUX_DMX_SET_TAG_ATTR:
        {
            #ifdef DMX_TAG_DEAL_SUPPORT
            DMX_Tag_SetAttr_S *Param = (DMX_Tag_GetAttr_S*)arg;

            ret = MT_DRV_DMX_SetTagAttr(Param->DmxId, &Param->TagAttr);
            #else
            ret = MT_ERR_NOT_SUPPORT_TAGDEAL;
            #endif
            break;
        }

        case CMD_DEMUX_PORT_ATTACH:
        {
            DMX_Port_Attach_S *Param = (DMX_Port_Attach_S*)arg;

            if (DMX_PORT_MODE_RAM == Param->PortMode)
            {
                ret = MT_DRV_DMX_AttachRamPort(Param->DmxId, Param->PortId);
            }
            else
            {
                ret = MT_DRV_DMX_AttachTunerPort(Param->DmxId, Param->PortId);
            }

            break;
        }

        case CMD_DEMUX_PORT_DETACH:
        {
            ret = MT_DRV_DMX_DetachPort(*(mt_u32*)arg);

            break;
        }

        case CMD_DEMUX_PORT_GETID:
        {
            DMX_Port_GetId_S *Param = (DMX_Port_GetId_S*)arg;

            ret = MT_DRV_DMX_GetPortId(Param->DmxId, &Param->PortMode, &Param->PortId);

            break;
        }

        case CMD_DEMUX_PORT_GETPACKETNUM:
        {
            DMX_PortPacketNum_S *Param = (DMX_PortPacketNum_S*)arg;

            if (DMX_PORT_MODE_RAM == Param->PortMode)
            {
                Param->ErrTsPackCnt = 0;

                ret = MT_DRV_DMX_RamPortGetPacketNum(Param->PortId, &Param->TsPackCnt);
            }
            else
            {
                ret = MT_DRV_DMX_TunerPortGetPacketNum(Param->PortId, &Param->TsPackCnt, &Param->ErrTsPackCnt);
            }

            break;
        }

		case CMD_DEMUX_GET_PORT_CTRL:
		{
			DMX_Port_Ctrl_S *Param = (DMX_Port_Ctrl_S *)arg;
			if (DMX_PORT_MODE_TUNER == Param->PortMode)
			{
				ret = DMX_OsiGetPortCtrl(Param->PortId, &Param->PortValue.all);
			}
			
			break;
		}
		
		case CMD_DEMUX_SET_PORT_CTRL:
		{
			DMX_Port_Ctrl_S *Param = (DMX_Port_Ctrl_S *)arg;
			if (DMX_PORT_MODE_TUNER == Param->PortMode)
			{				
				ret = DMX_OsiSetPortCtrl(Param->PortId, Param->PortValue.all);
			}

			break;
		}
		
		case CMD_DEMUX_GET_PORT_CLK:
		{
			DMX_Port_Clk_S *Param = (DMX_Port_Clk_S *)arg;
			if (DMX_PORT_MODE_TUNER == Param->PortMode)
			{
				ret = DMX_OsiGetPortClk(Param->PortId, &Param->PortClk);
			}
			
			break;
		}
		
		case CMD_DEMUX_SET_PORT_CLK:
		{
			DMX_Port_Clk_S *Param = (DMX_Port_Clk_S *)arg;
			if (DMX_PORT_MODE_TUNER == Param->PortMode)
			{				
				ret = DMX_OsiSetPortClk(Param->PortId, Param->PortClk);
			}

			break;
		}
				
		case CMD_DEMUX_GET_PORT_SRC:
		{
			DMX_Port_Src_S *Param = (DMX_Port_Src_S *)arg;
			if (DMX_PORT_MODE_TUNER == Param->PortMode)
			{
				ret = DMX_OsiGetPortSrc(Param->PortId, &Param->PortSrc);
			}
			
			break;
		}
		
		case CMD_DEMUX_SET_PORT_SRC:
		{
			DMX_Port_Src_S *Param = (DMX_Port_Src_S *)arg;
			if (DMX_PORT_MODE_TUNER == Param->PortMode)
			{				
				ret = DMX_OsiSetPortSrc(Param->PortId, Param->PortSrc);
			}

			break;
		}

        default:
        {
            //MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
           // PROC_PRINT(p, "unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMXTsBufferIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_TS_BUFFER_INIT:
        {
            DMX_TsBufInit_S *Param = (DMX_TsBufInit_S*)arg;

            ret = MT_DRV_DMX_CreateTSBuffer(Param->PortId, Param->BufSize, &Param->TsBuf, (ulong)file);

            break;
        }

        case CMD_DEMUX_TS_BUFFER_DEINIT:
        {
            ret = MT_DRV_DMX_DestroyTSBuffer(*(mt_u32*)arg);
            break;
        }

        case CMD_DEMUX_TS_BUFFER_GET:
        {
            DMX_TsBufGet_S *Param = (DMX_TsBufGet_S*)arg;
            ret = MT_DRV_DMX_GetTSBuffer(Param->PortId, Param->ReqLen, &Param->Data, Param->TimeoutMs); 
            break;
        }

        case CMD_DEMUX_TS_BUFFER_PUT:
        {
            DMX_TsBufPut_S *Param = (DMX_TsBufPut_S*)arg;

            ret = MT_DRV_DMX_PutTSBuffer(Param->PortId, Param->ValidDataLen, Param->StartPos, Param->Pid, Param->DateType, Param->Pts, Param->SpecifiedDataAddr);

            break;
        }

        case CMD_DEMUX_TS_BUFFER_RESET:
        {
            ret = MT_DRV_DMX_ResetTSBuffer(*(mt_u32*)arg);

            break;
        }

        case CMD_DEMUX_TS_BUFFER_GET_STATUS:
        {
            DMX_TsBufStaGet_S *Param = (DMX_TsBufStaGet_S*)arg;

            ret = MT_DRV_DMX_GetTSBufferStatus(Param->PortId, &Param->Status);

            break;
        }
		
        case CMD_DEMUX_CHAN_BUF_FULL_CARE_SET:
        {
            DMX_BufFullCare_S *pPara = (DMX_BufFullCare_S *)arg;
            ret = MT_DRV_DMX_SetChannelBufFullCare(pPara->hChannel, &pPara->stBufFullCare);
            break;
        }

		case CMD_DEMUX_TS_BUFFER_GET_FULL_CARE:
		{
			DMX_TsBufFullCare_S *pPara = (DMX_TsBufFullCare_S *)arg;
            ret = DMX_OsiGetTsBufferFullCare(pPara->PortId, &pPara->AvFullCare, &pPara->RecFullCare);
			pr_err("[%s %d]port_id=%d, AvFullCare.all=0x%x, RecFullCare.all=0x%x\n", __FUNCTION__, __LINE__, pPara->PortId, pPara->AvFullCare.all, pPara->RecFullCare.all);
			break;
		}

		case CMD_DEMUX_TS_BUFFER_SET_FULL_CARE:
		{
			DMX_TsBufFullCare_S *pPara = (DMX_TsBufFullCare_S *)arg;
            ret = DMX_OsiSetTsBufferFullCare(pPara->PortId, pPara->AvFullCare, pPara->RecFullCare);
			pr_err("[%s %d]port_id=%d, AvFullCare.all=0x%x, RecFullCare.all=0x%x\n", __FUNCTION__, __LINE__, pPara->PortId, pPara->AvFullCare.all, pPara->RecFullCare.all);
			break;
		}
		
        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMXChanIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_CHAN_NEW:
        {
            DMX_ChanNew_S *pPara = (DMX_ChanNew_S *)arg;
            ret = MT_DRV_DMX_CreateChannel(pPara->u32DemuxId, &pPara->stChAttr,
                                    &pPara->hChannel, &pPara->stChBuf, &pPara->stChDescBuf, (ulong)file);
            break;
        }

        case CMD_DEMUX_CHAN_DEL:
        {
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
            demux_aud_ch_list *chlist = &demux_priv_data_info.aud_proc_list;
            mt_handle h_channel = *(mt_handle*)arg;
            MT_UNF_DMX_CHAN_ATTR_S chAttr;
            mt_u32 chid = 0;

            memset(&chAttr, 0, sizeof(chAttr));
            ret = MT_DRV_DMX_GetChannelAttr(*(mt_handle*)arg, &chAttr);
            ret |= MT_DRV_DMX_GetChannelId(h_channel, &chid);
			
            if (MT_SUCCESS == ret)
            {
                if (MT_UNF_DMX_CHAN_TYPE_AUD == chAttr.enChannelType)
                {
                    mt_u32 id = 0;
                    mt_u32 chExist = MT_FALSE;

                    for (id = 0; id < DMX_PROC_CH_MAX; id++)
                    {
                        if (chlist->main_chan_id[id] == chid)
                        {                                
                            chExist = MT_TRUE;
                            break;
                        }
                    }
                    /* close it */
                    if ((MT_TRUE == chExist) && (id < DMX_PROC_CH_MAX))
                    {
                        MT_ERR_DEMUX("auto close ch %d before destroy it\n", chid);
                        if(mutex_lock_killable(&g_demux_ch_mutex))
                        {
													 MT_ERR_DEMUX("<%s> : <%d> : mutex_lock_killable fail\n", __FUNCTION__, __LINE__);
												}
                        chlist->main_chan_id[id] = DMX_INVALID_CHAN_ID;
                        mutex_unlock(&g_demux_ch_mutex);
                    }
                }
#ifdef CONFIG_MT_AUDIO_AD
                else if (MT_UNF_DMX_CHAN_TYPE_AUD_AD == chAttr.enChannelType)
                {
                    mutex_lock_killable(&g_demux_ch_mutex);
                    chlist->desc_chan_id = DMX_INVALID_CHAN_ID;
                    mutex_unlock(&g_demux_ch_mutex);
                    MT_DBG_DEMUX("del AD ch %d\n", chid);
                }
#endif
            }
#endif
            ret = MT_DRV_DMX_DestroyChannel(*(mt_handle*)arg);
            break;
        }

        case CMD_DEMUX_CHAN_OPEN:
        {
            mt_handle h_channel = *(mt_handle*)arg;

            ret = MT_DRV_DMX_OpenChannel(h_channel);

#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
            if (MT_SUCCESS == ret) 
            {
                demux_aud_ch_list *chlist = &demux_priv_data_info.aud_proc_list;
                MT_UNF_DMX_CHAN_ATTR_S chAttr;
                mt_u32 chid = 0;
                    
				memset(&chAttr, 0, sizeof(chAttr));
                ret = MT_DRV_DMX_GetChannelAttr(*(mt_handle*)arg, &chAttr);
                ret |= MT_DRV_DMX_GetChannelId(h_channel, &chid);
				
                if (MT_SUCCESS == ret) 
                {
                    if (MT_UNF_DMX_CHAN_TYPE_AUD == chAttr.enChannelType)
                    {
                        mt_u32 id = 0;
                        mt_u32 chExist = MT_FALSE;

                        for (id = 0; id < DMX_PROC_CH_MAX; id++)
                        {
                            if (chlist->main_chan_id[id] == chid)
                            {
                                MT_ERR_DEMUX("channel id %d already exist, please close it first\n", chid);
                                chExist = MT_TRUE;
                                break;
                            }
                        }
                        
                        if (MT_FALSE == chExist)
                        {
                            id = 0;
                            while ((id < DMX_PROC_CH_MAX) && (chlist->main_chan_id[id] != DMX_INVALID_CHAN_ID))
                            {
                                id++;
                            }
							
                            MT_DBG_DEMUX("ch[%d] add to proc list index[%d], %#x\n", chid, id, chlist->main_chan_id[id]);
							
                            if (id < DMX_PROC_CH_MAX)
                            {
                                chlist->main_chan_id[id] = chid;
                                /* set default active channel */
                                if (-1 == chlist->main_select_index)
                                {
                                    chlist->main_select_index = 0;
                                    chlist->main_pre_index = -1;
                                }
								
                                chlist->main_chan_num++;
                            }
                            else
                            {
                                MT_ERR_DEMUX("TODO: max num of audio channel %d\n", DMX_PROC_CH_MAX);
                            }
                        }
                    }
#ifdef CONFIG_MT_AUDIO_AD 
                    else if (MT_UNF_DMX_CHAN_TYPE_AUD_AD == chAttr.enChannelType)
                    {
                        chlist->desc_chan_id = chid;
                        MT_DBG_DEMUX("ch[%d] add to ad list\n", chid);
                    }
#endif
                }
                else
                {
                    MT_ERR_DEMUX("get channel attr or id failed\n");
                }
            }
            else
            {
                MT_ERR_DEMUX("open channel failed\n");
            }
#endif
            break;
        }

        case CMD_DEMUX_CHAN_CLOSE:
        {
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
            demux_aud_ch_list *chlist = &demux_priv_data_info.aud_proc_list;
            mt_handle h_channel = *(mt_handle*)arg;
            MT_UNF_DMX_CHAN_ATTR_S chAttr;
            mt_u32 chid = 0;

            memset(&chAttr, 0, sizeof(chAttr));
            ret = MT_DRV_DMX_GetChannelAttr(*(mt_handle*)arg, &chAttr);
            ret |= MT_DRV_DMX_GetChannelId(h_channel, &chid);
			
            if (MT_SUCCESS == ret)
            {
                if (MT_UNF_DMX_CHAN_TYPE_AUD == chAttr.enChannelType)
                {
                    mt_s32 id = 0;
					
                    while ((id < DMX_PROC_CH_MAX) && (chlist->main_chan_id[id] != chid))
                    {
                        id++;
                    }
					
                    MT_DBG_DEMUX("ch[%d] remove proc to list result %d, %#x\n", chid, id, chlist->main_chan_id[id]);
					
                    if (id < DMX_PROC_CH_MAX)
                    {
                        if(mutex_lock_killable(&g_demux_ch_mutex))
												{
													 MT_ERR_DEMUX("<%s> : <%d> : mutex_lock_killable fail\n", __FUNCTION__, __LINE__);
												}
                        chlist->main_chan_id[id] = DMX_INVALID_CHAN_ID;
						
                        if (id == chlist->main_select_index)
                        {
                            /* find a valid channel as default */							

                            for (id = 0; id < DMX_PROC_CH_MAX; id++)
                            {
                                if (chlist->main_chan_id[id] != DMX_INVALID_CHAN_ID)
                                {
                                    break;
                                }
                            }

                            if (id < DMX_PROC_CH_MAX)
                            {
                                chlist->main_select_index = id;
                            }
                            else
                            {
                                chlist->main_select_index = -1;
                            }

                            MT_DBG_DEMUX("%s, %d, select_index = %d\n", __func__, __LINE__, chlist->main_select_index);
                        }

                        if (chlist->main_chan_num > 0)
                        {
                            chlist->main_chan_num--;
							
                            if (0 == chlist->main_chan_num)
                            {
                                g_channel_reset = 1;
                                chlist->main_pre_index = -1;
                            }
                        }
                        else
                        {
                            MT_ERR_DEMUX("%s, %d, main_chan_num error\n", __func__, __LINE__);
                        }
						
                        mutex_unlock(&g_demux_ch_mutex);
                    }
                    else 
                    {
                        MT_ERR_DEMUX("target channel %u not exist, please open it first\n", chid);
                    }                    
                }
#ifdef CONFIG_MT_AUDIO_AD 
                else if (MT_UNF_DMX_CHAN_TYPE_AUD_AD == chAttr.enChannelType)
                {
                    mutex_lock_killable(&g_demux_ch_mutex);
                    chlist->desc_chan_id = DMX_INVALID_CHAN_ID;
                    mutex_unlock(&g_demux_ch_mutex);
                    MT_INFO_DEMUX("remove %d from ad list\n", chid);
                }                
#endif
            }
            else 
            {
                MT_ERR_DEMUX("get channel attr or id failed\n");
            }
#endif            
            ret = MT_DRV_DMX_CloseChannel(*(mt_handle*)arg);            
            break;
        }

        case CMD_DEMUX_CHAN_ATTR_GET:
        {
            DMX_GetChan_Attr_S *pPara = (DMX_GetChan_Attr_S *)arg;
            ret = MT_DRV_DMX_GetChannelAttr(pPara->hChannel, &pPara->stChAttr);
            break;
        }

        case CMD_DEMUX_CHAN_ATTR_SET:
        {
            DMX_SetChan_Attr_S *pPara = (DMX_SetChan_Attr_S *)arg;
            ret = MT_DRV_DMX_SetChannelAttr(pPara->hChannel, &pPara->stChAttr);
            break;
        }

        case CMD_DEMUX_GET_CHAN_STATUS:
        {
            DMX_ChanStatusGet_S *pPara = (DMX_ChanStatusGet_S *)arg;
            ret = MT_DRV_DMX_GetChannelStatus(pPara->hChannel, &pPara->stStatus);
            break;
        }

        case CMD_DEMUX_PID_SET:
        {
            DMX_ChanPIDSet_S *pPara = (DMX_ChanPIDSet_S *)arg;
            ret = MT_DRV_DMX_SetChannelPID(pPara->hChannel, pPara->u32Pid);
            break;
        }

        case CMD_DEMUX_PID_GET:
        {
            DMX_ChanPIDGet_S *pPara = (DMX_ChanPIDGet_S *)arg;
            ret = MT_DRV_DMX_GetChannelPID(pPara->hChannel, &pPara->u32Pid);
            break;
        }

        case CMD_DEMUX_CHANID_GET:
        {
            DMX_ChannelIdGet_S *pPara = (DMX_ChannelIdGet_S *)arg;
            ret = MT_DRV_DMX_GetChannelHandle(pPara->u32DmxId, pPara->u32Pid, pPara->enChannelType, &pPara->hChannel);
            break;
        }

        /*
        ** add for advca
        */
        case CMD_DEMUX_AVCHANID_GET:
        {
            DMX_AVChannelIdGet_S *pPara = (DMX_AVChannelIdGet_S *)arg;
            ret = MT_DRV_DMX_GetAVChannelHandle(pPara->u32DmxId, &pPara->vidChannel, &pPara->audChannel);
            break;
        }
        case CMD_DEMUX_FREECHAN_GET:
        {
            DMX_FreeChanGet_S *pPara = (DMX_FreeChanGet_S *)arg;
            ret = MT_DRV_DMX_GetFreeChannelCount(pPara->u32DmxId, &pPara->u32FreeCount);
            break;
        }

        case CMD_DEMUX_SCRAMBLEFLAG_GET:
        {
            DMX_ScrambledFlagGet_S *pPara = (DMX_ScrambledFlagGet_S *)arg;
            ret = MT_DRV_DMX_GetScrambledFlag(pPara->hChannel, &pPara->enScrambleFlag);
            break;
        }

        case CMD_DEMUX_CHAN_SET_EOS_FLAG:
        {
            ret = MT_DRV_DMX_SetChannelEosFlag(*(mt_handle*)arg);
            break;
        }

#ifdef DMX_USE_ECM
        case CMD_DEMUX_GET_CHAN_SWFLAG:
        {
            DMX_ChanSwGet_S *pPara = (DMX_ChanSwGet_S *)arg;
            ret = DMX_OsrGetChannelSwFlag(pPara->hChannel, &pPara->u32SwFlag);
            break;
        }

        case CMD_DEMUX_GET_CHAN_SWBUF_ADDR:
        {
            DMX_ChanSwBufGet_S *pPara = (DMX_ChanSwBufGet_S *)arg;
            ret = DMX_OsrGetChannelSwBufAddr(pPara->hChannel, &(pPara->stChnBuf));
            break;
        }
#endif

        case CMD_DEMUX_GET_CHAN_TSCNT:
        {
            DMX_ChanChanTsCnt_S *pPara = (DMX_ChanChanTsCnt_S *)arg;
            ret = MT_DRV_DMX_GetChannelTsCount(pPara->hChannel, &(pPara->u32ChanTsCnt));
            break;
        }
        case CMD_DEMUX_CHAN_CC_REPEAT_SET:
        {
            DMX_SetChan_CC_REPEAT_S *pPara = (DMX_SetChan_CC_REPEAT_S *)arg;
            ret = MT_DRV_DMX_SetChannelCCRepeat(pPara->stChCCRepeatSet.hChannel, &pPara->stChCCRepeatSet);
            break;
        }
		
        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x, 0x%x\n", cmd, CMD_DEMUX_CHANID_GET);
        }
    }

    return ret;
}

static mt_s32 DMXFiltIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_FLT_NEW:
        {
            DMX_NewFilter_S *Param = (DMX_NewFilter_S*)arg;

            ret = MT_DRV_DMX_CreateFilter(Param->DmxId, &Param->FilterAttr, &Param->Filter, (ulong)file);

            break;
        }

        case CMD_DEMUX_FLT_DEL:
        {
            ret = MT_DRV_DMX_DestroyFilter(*(mt_handle*)arg);

            break;
        }

        case CMD_DEMUX_FLT_SET:
        {
            DMX_FilterSet_S *Param = (DMX_FilterSet_S*)arg;

            ret = MT_DRV_DMX_SetFilterAttr(Param->Filter, &Param->FilterAttr);

            break;
        }

        case CMD_DEMUX_FLT_GET:
        {
            DMX_FilterGet_S *Param = (DMX_FilterGet_S*)arg;

            ret = MT_DRV_DMX_GetFilterAttr(Param->Filter, &Param->FilterAttr);

            break;
        }

        case CMD_DEMUX_FLT_ATTACH:
        {
            DMX_FilterAttach_S *Param = (DMX_FilterAttach_S*)arg;

            ret = MT_DRV_DMX_AttachFilter(Param->Filter, Param->Channel);

            break;
        }

        case CMD_DEMUX_FLT_DETACH:
        {
            DMX_FilterDetach_S *Param = (DMX_FilterDetach_S*)arg;

            ret = MT_DRV_DMX_DetachFilter(Param->Filter, Param->Channel);

            break;
        }

        case CMD_DEMUX_FREEFLT_GET:
        {
            DMX_FreeFilterGet_S *Param = (DMX_FreeFilterGet_S*)arg;

            ret = MT_DRV_DMX_GetFreeFilterCount(Param->DmxId, &Param->FreeCount);

            break;
        }

        case CMD_DEMUX_FLT_DELALL:
        {
            ret = MT_DRV_DMX_DestroyAllFilter(*(mt_handle*)arg);

            break;
        }

        case CMD_DEMUX_FLT_CHANID_GET:
        {
            DMX_FilterChannelIDGet_S *Param = (DMX_FilterChannelIDGet_S*)arg;

            ret = MT_DRV_DMX_GetFilterChannelHandle(Param->Filter, &Param->Channel);

            break;
        }

		//move to here:
        case CMD_DEMUX_AVCHANID_GET:
        {
            DMX_AVChannelIdGet_S *pPara = (DMX_AVChannelIdGet_S *)arg;
            ret = MT_DRV_DMX_GetAVChannelHandle(pPara->u32DmxId, &pPara->vidChannel, &pPara->audChannel);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}



static mt_s32 DMXRecvIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_GET_DATA_FLAG:
        {
            DMX_GetDataFlag_S *pPara = (DMX_GetDataFlag_S *)arg;
            ret = MT_DRV_DMX_GetDataHandle(pPara->u32Flag, pPara->u32TimeOutMs);
            break;
        }

        case CMD_DEMUX_SELECT_DATA_FLAG:
        {
            DMX_SelectDataFlag_S *pPara = (DMX_SelectDataFlag_S*)arg;
            mt_handle ch_handle[128] = {0};

            /*
            patch for bug130066, pPara->channel is a user space pointer, can not uses it in kernel space
            */
            memset(ch_handle, 0, sizeof(ch_handle));

            if (copy_from_user(ch_handle, (void __user *)pPara->channel, (sizeof(mt_handle))*pPara->channelnum))
            {
                ret = -EFAULT;
                break;
            }

            ret = MT_DRV_DMX_SelectDataHandle(ch_handle, pPara->channelnum, pPara->u32Flag, pPara->u32TimeOutMs);
            break;
        }

        case CMD_DEMUX_CHECK_DATA_FLAG:
        {
            DMX_CheckDataFlag_S *pPara = (DMX_CheckDataFlag_S *)arg;

            ret = MT_DRV_DMX_CheckDataHandle(pPara->hChannel, pPara->u32TimeOutMs);
            break;
        }

        case CMD_DEMUX_ACQUIRE_MSG:
        {
            DMX_AcqMsg_S *pPara = (DMX_AcqMsg_S *)arg;
            DMX_UserMsg_S stReqBufTmp[DMX_DEFAULT_BUF_NUM];
            DMX_UserMsg_S* pstBufTmp;
            mt_u32 u32AcquireNum = pPara->u32AcquireNum;
            MT_UNF_DMX_DATA_S *pstBuf;
            if (u32AcquireNum > DMX_DEFAULT_BUF_NUM)
            {
                pstBufTmp = MT_VMALLOC(MT_ID_DEMUX, sizeof(DMX_UserMsg_S) * u32AcquireNum);
                if (NULL == pstBufTmp)
                {
                    MT_FATAL_DEMUX("malloc failed.\n");
                    ret = MT_ERR_DMX_ALLOC_MEM_FAILED;
                    break;
                }
            }
            else
            {
                pstBufTmp = stReqBufTmp;
            }

            ret = MT_DRV_DMX_AcquireBuf(pPara->hChannel, pPara->u32AcquireNum,
                                    &pPara->u32AcquiredNum, pstBufTmp, pPara->u32TimeOutMs);

            if (ret == MT_SUCCESS && pPara->u32AcquiredNum)
            {
                MT_UNF_DMX_DATA_S stUsrBuf;
                mt_u32 i;

                pstBuf = pPara->pstBuf;

                for (i = 0; i < pPara->u32AcquiredNum; i++)
                {
                    stUsrBuf.data_phy_addr = pstBufTmp[i].u32BufStartAddr;
                    stUsrBuf.u32Size = pstBufTmp[i].u32MsgLen;
                    stUsrBuf.enDataType = pstBufTmp[i].enDataType;
                    stUsrBuf.filthandle = DMX_FLTHANDLE(pstBufTmp[i].filterid);

                    if (copy_to_user(&pstBuf[i], &stUsrBuf, sizeof(MT_UNF_DMX_DATA_S)))
                    {
                        MT_ERR_DEMUX("copy data buf to usr failed\n");
                        ret = MT_FAILURE;
                        break;
                    }
                }
            }
            if (u32AcquireNum > DMX_DEFAULT_BUF_NUM)
            {
                MT_VFREE(MT_ID_DEMUX, pstBufTmp);
            }

            break;
        }

        case CMD_DEMUX_RELEASE_MSG:
        {
            DMX_RelMsg_S *pPara = (DMX_RelMsg_S *)arg;
            DMX_UserMsg_S *pstBufTmp;
            DMX_UserMsg_S stReqBufTmp[DMX_DEFAULT_BUF_NUM];
            mt_u32 u32ReleaseNum;
            MT_UNF_DMX_DATA_S* pstBuf;
            MT_UNF_DMX_DATA_S stUsrBuf;
            mt_u32 i;

            u32ReleaseNum = pPara->u32ReleaseNum;
            if (u32ReleaseNum > DMX_DEFAULT_BUF_NUM)
            {
                pstBufTmp = MT_VMALLOC(MT_ID_DEMUX, sizeof(DMX_UserMsg_S) * u32ReleaseNum);
            }
            else
            {
                pstBufTmp = stReqBufTmp;
            }

            if (NULL == pstBufTmp)
            {
                MT_FATAL_DEMUX("malloc failed u32ReleaseNum:%x.\n", u32ReleaseNum);
                ret =  MT_ERR_DMX_ALLOC_MEM_FAILED;
                break;
            }
            pstBuf = pPara->pstBuf;
            for (i = 0; i < u32ReleaseNum; i++)
            {
                if (copy_from_user(&stUsrBuf, &pstBuf[i], sizeof(MT_UNF_DMX_DATA_S)))
                {
                    MT_ERR_DEMUX("copy data buf to usr failed\n");
                    ret = MT_FAILURE;
                    break;
                }

                pstBufTmp[i].u32BufStartAddr = stUsrBuf.data_phy_addr;
                pstBufTmp[i].u32MsgLen = stUsrBuf.u32Size;
            }
            ret = MT_DRV_DMX_ReleaseBuf(pPara->hChannel, u32ReleaseNum, pstBufTmp);
            if (u32ReleaseNum > DMX_DEFAULT_BUF_NUM)
            {
                MT_VFREE(MT_ID_DEMUX, pstBufTmp);
            }


            break;
        }

        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMXPcrIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_PCR_NEW:
        {
            DMX_NewPcr_S *pPara = (DMX_NewPcr_S *)arg;
            ret = MT_DRV_DMX_CreatePcrChannel(pPara->u32DmxId, &pPara->u32PcrId, (ulong)file);
            break;
        }

        case CMD_DEMUX_PCR_DEL:
        {
            ret = MT_DRV_DMX_DestroyPcrChannel(*(mt_u32*)arg);
            break;
        }

        case CMD_DEMUX_PCRPID_SET:
        {
            DMX_PcrPidSet_S *pPara = (DMX_PcrPidSet_S *)arg;
            ret = MT_DRV_DMX_PcrPidSet(pPara->pu32PcrChId, pPara->u32Pid);
            break;
        }

        case CMD_DEMUX_PCRPID_GET:
        {
            DMX_PcrPidGet_S *pPara = (DMX_PcrPidGet_S *)arg;
            ret = MT_DRV_DMX_PcrPidGet(pPara->pu32PcrChId, &pPara->u32Pid);
            break;
        }

        /* not support PCR user mode get, presently */
        case CMD_DEMUX_CURPCR_GET:
        {
            DMX_PcrScrGet_S *pPara = (DMX_PcrScrGet_S *)arg;
            ret = MT_DRV_DMX_PcrScrGet(pPara->pu32PcrChId, &pPara->u64PcrValue, &pPara->u64ScrValue);
            break;
        }

        case CMD_DEMUX_PCRSYN_ATTACH:
        {
            DMX_PCRSYNC_S *pPara = (DMX_PCRSYNC_S *)arg;
            ret = MT_DRV_DMX_PcrSyncAttach(pPara->u32PcrChId, pPara->u32SyncHandle);
            break;
        }

        case CMD_DEMUX_PCRSYN_DETACH:
        {
            DMX_PCRSYNC_S *pPara = (DMX_PCRSYNC_S *)arg;
            ret = MT_DRV_DMX_PcrSyncDetach(pPara->u32PcrChId);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMXAvIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

    switch (cmd)
    {
        case CMD_DEMUX_PES_BUFFER_GETSTAT:
        {
            DMX_PesBufStaGet_S *pPara = (DMX_PesBufStaGet_S *)arg;
            ret = MT_DRV_DMX_GetPESBufferStatus(pPara->hChannel, &pPara->stBufStat);
            break;
        }

        case CMD_DEMUX_ES_BUFFER_GET:
        {
            DMX_PesBufGet_S *pPara = (DMX_PesBufGet_S *)arg;
            DMX_Stream_S stEsBuf;
            stEsBuf.u32BufLen = 0;
            stEsBuf.u32PtsMs = (mt_u32)MT_INVALID_PTS_U64;
            ret = MT_DRV_DMX_AcquireEs(pPara->hChannel, &stEsBuf);

            if (MT_SUCCESS == ret)
            {
                pPara->stEsBuf.es_data_phy_addr = stEsBuf.u32BufPhyAddr;
                pPara->stEsBuf.u32BufLen = stEsBuf.u32BufLen;
                pPara->stEsBuf.u64PtsMs = stEsBuf.u32PtsMs;
            }

            break;
        }

        case CMD_DEMUX_ES_BUFFER_PUT:
        {
            DMX_PesBufGet_S *pPara = (DMX_PesBufGet_S *)arg;
            DMX_Stream_S stEsBuf;

            stEsBuf.u32BufPhyAddr = pPara->stEsBuf.es_data_phy_addr;
            stEsBuf.u32BufLen = pPara->stEsBuf.u32BufLen;
            stEsBuf.u32PtsMs = pPara->stEsBuf.u64PtsMs;

            ret = MT_DRV_DMX_ReleaseEs(pPara->hChannel, &stEsBuf);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMXRecIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;
    MT_DBG_DEMUX("DMXRecIoctl======cmd=0x%x, 0x%x\n",cmd,CMD_DEMUX_REC_CHAN_CREATE);
    switch (cmd)
    {
        case CMD_DEMUX_REC_CHAN_CREATE :
        {
            DMX_Rec_CreateChan_S *Param = (DMX_Rec_CreateChan_S*)arg;
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_CREATE\n");
            ret = MT_DRV_DMX_CreateRecChn(
                    &Param->RecAttr,
                    &Param->RecHandle,
                    &Param->RecBufPhyAddr,
                    &Param->RecBufSize,
                    &Param->RecIdxBufPhyAddr,
                    &Param->RecIdxBufSize,
                    (ulong)file
                );

            break;
        }

        case CMD_DEMUX_REC_CHAN_DESTROY :
        {
            ret = MT_DRV_DMX_DestroyRecChn(*(mt_handle*)arg);
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_DESTROY\n");
            break;
        }

        case CMD_DEMUX_REC_CHAN_ADD_PID :
        {
            DMX_Rec_AddPid_S *Param = (DMX_Rec_AddPid_S*)arg;
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_ADD_PID\n");
            ret = MT_DRV_DMX_AddRecPid(Param->RecHandle, Param->Pid, &Param->ChanHandle, (ulong)file);

            break;
        }

        case CMD_DEMUX_REC_CHAN_DEL_PID :
        {
            DMX_Rec_DelPid_S *Param = (DMX_Rec_DelPid_S*)arg;
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_DEL_PID\n");
            ret = MT_DRV_DMX_DelRecPid(Param->RecHandle, Param->ChanHandle);

            break;
        }

        case CMD_DEMUX_REC_CHAN_DEL_ALL_PID :
        {
            ret = MT_DRV_DMX_DelAllRecPid(*(mt_handle*)arg);
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_DEL_ALL_PID\n");
            break;
        }

        case CMD_DEMUX_REC_CHAN_ADD_EXCLUDE_PID :
        {
            DMX_Rec_ExcludePid_S *Param = (DMX_Rec_ExcludePid_S*)arg;
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_ADD_EXCLUDE_PID\n");
            ret = MT_DRV_DMX_AddExcludeRecPid(Param->RecHandle, Param->Pid);

            break;
        }

        case CMD_DEMUX_REC_CHAN_DEL_EXCLUDE_PID :
        {
            DMX_Rec_ExcludePid_S *Param = (DMX_Rec_ExcludePid_S*)arg;

            ret = MT_DRV_DMX_DelExcludeRecPid(Param->RecHandle, Param->Pid);

            break;
        }

        case CMD_DEMUX_REC_CHAN_CANCEL_EXCLUDE :
        {
            ret = MT_DRV_DMX_DelAllExcludeRecPid(*(mt_handle*)arg);

            break;
        }

        case CMD_DEMUX_REC_CHAN_START :
        {
            ret = MT_DRV_DMX_StartRecChn(*(mt_handle*)arg);
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_START\n");
            break;
        }

        case CMD_DEMUX_REC_CHAN_STOP :
        {
            ret = MT_DRV_DMX_StopRecChn(*(mt_handle*)arg);

            break;
        }

        case CMD_DEMUX_REC_CHAN_ACQUIRE_DATA :
        {
            DMX_Rec_AcquireData_S *Param = (DMX_Rec_AcquireData_S*)arg;

            ret = MT_DRV_DMX_AcquireRecData(Param->RecHandle, &Param->RecData, Param->TimeoutMs);

            break;
        }

        case CMD_DEMUX_REC_CHAN_RELEASE_DATA :
        {
            DMX_Rec_ReleaseData_S *Param = (DMX_Rec_ReleaseData_S*)arg;

            ret = MT_DRV_DMX_ReleaseRecData(Param->RecHandle, &Param->RecData);

            break;
        }

        case CMD_DEMUX_REC_CHAN_ACQUIRE_INDEX :
        {
            DMX_Rec_AcquireIndex_S *Param = (DMX_Rec_AcquireIndex_S*)arg;

            ret = MT_DRV_DMX_AcquireRecIndex(Param->RecHandle, &Param->IndexData, Param->TimeoutMs);

            break;
        }

        case CMD_DEMUX_REC_CHAN_GET_BUF_STATUS :
        {
            DMX_Rec_BufStatus_S *Param = (DMX_Rec_BufStatus_S*)arg;

            ret = MT_DRV_DMX_GetRecBufferStatus(Param->RecHandle, &Param->BufStatus);

            break;
        }
		
        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

static mt_s32 DMX_LinkRec_FastPlay_Ioctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;

//	printk("[%s %d]CMD_DEMUX_LINKREC_CHAN_CREATE=0x%lx\n", __FUNCTION__, __LINE__, CMD_DEMUX_LINKREC_CHAN_CREATE);
    switch (cmd)
    {        
		case CMD_DEMUX_LINKREC_CHAN_CREATE:
        {
            DMX_LinkRec_CreateChan_S *Param = (DMX_LinkRec_CreateChan_S*)arg;
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_CREATE\n");
            ret = MT_DRV_DMX_CreateLinkRecChn(Param, (ulong)file);

            break;
        }
		
		case CMD_DEMUX_LINKREC_CHAN_ACQUIRE_DATA:
        {
            DMX_LinkRec_AcquireData_S *Param = (DMX_LinkRec_AcquireData_S*)arg;

            ret = MT_DRV_DMX_AcquireLinkRecData(Param->RecHandle, &Param->RecData, Param->TimeoutMs, &Param->node_index);

            break;
        }

		case CMD_DEMUX_LINKREC_CHAN_RELEASE_DATA:
        {
            DMX_Rec_ReleaseData_S *Param = (DMX_Rec_ReleaseData_S*)arg;

            ret = MT_DRV_DMX_ReleaseLinkRecData(Param->RecHandle, &Param->RecData);

            break;
        }

		case CMD_DEMUX_LINKREC_CHAN_DESTROY:
        {
            ret = MT_DRV_DMX_DestroyLinkRecChn(*(mt_handle*)arg);
            MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_DESTROY\n");
            break;
        }

		case CMD_DEMUX_LINKREC_CHAN_ACQUIRE_INDEX :
        {
            DMX_Rec_AcquireIndex_S *Param = (DMX_Rec_AcquireIndex_S*)arg;

            ret = MT_DRV_DMX_AcquireLinkRecIndex(Param->RecHandle, &Param->IndexData, Param->TimeoutMs);

            break;
        }
		
		case CMD_DEMUX_FASTPLAY_START:
		{
			DMX_FastPlay_Param_S *Param = (DMX_FastPlay_Param_S*)arg;
			ret = DMX_DRV_FastPlay_Start(Param->swtsi_num, Param->rec_id, Param->node_num);
			MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_DESTROY\n");
			break;
		}

		case CMD_DEMUX_FASTPLAY_STOP:
		{
			ret = DMX_DRV_FastPlay_Stop();
			MT_DBG_DEMUX("CMD_DEMUX_REC_CHAN_DESTROY\n");
			break;
		}		
        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}


#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
static mt_s32 DMXDataIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    demux_aud_ch_list *chan = &demux_priv_data_info.aud_proc_list;
    mt_s32 ret = MT_SUCCESS;
    
    switch (cmd)
    {
        case CMD_DEMUX_DATA_PUSH_START:
        {
            mt_handle h_channel = *(mt_handle*)arg;
            MT_UNF_DMX_CHAN_ATTR_S chAttr;
            mt_u32 chid = 0;
			
			memset(&chAttr, 0, sizeof(chAttr));
            ret = MT_DRV_DMX_GetChannelAttr(*(mt_handle*)arg, &chAttr);
            ret |= MT_DRV_DMX_GetChannelId(h_channel, &chid);
			
            if (MT_SUCCESS == ret)
            {
                mt_u32 i = 0;
                /* Find the target channel and active it */
                do {
                    if (chan->main_chan_id[i] == chid)
                    {
                        mutex_lock(&g_demux_ch_mutex);
                        chan->main_select_index = i;
                        MT_DBG_DEMUX("%s, %d, select_index = %d, pre = %d\n", __func__, __LINE__, i, chan->main_pre_index);
                        mutex_unlock(&g_demux_ch_mutex);
						
                        //if (down_interruptible(&chan->aud_track_sem))
                        if (down_timeout(&chan->aud_track_sem, 2000))
                        {
                            ret = -ETIME;
                            MT_ERR_DEMUX("%s, %d, wait sem failed\n", __func__, __LINE__);
                        }

                        break;
                    }
					
                    i++;
                } while (i < DMX_PROC_CH_MAX);

                /*
                patch for bug126653, after user space sets mutil aud, it has done
                audio track once, but all the aud channel has not opened, so drv
                should store the chid which point to pAvplay->CurDmxAudChn
                */
                if (i >= DMX_PROC_CH_MAX)
                {
                    mutex_lock(&g_demux_ch_mutex);
                    chan->main_chan_id[0] = chid;
                    chan->main_select_index = 0;
                    chan->main_pre_index = -1;
                    chan->main_chan_num++;
                    mutex_unlock(&g_demux_ch_mutex);
                }
            }
			
            break;
        }
        case CMD_DEMUX_DATA_PUSH_STOP:
        {
			int i = 0;
            SYNC_GET_AVSYNC_INFO_S sync_info;

			mutex_lock(&g_demux_ch_mutex);
            g_channel_reset = 1;
            chan->main_select_index = -1;
            /*
            patch for 128879, some cases will cause chan->aud_track_sem to valid
            so drv should init it in each audio track, if not, audio track thread
            and DMX_AV_Proc_Thread will not sync
            */
            sema_init(&chan->aud_track_sem, 0);
            ret = avsync_get_avsync_info((SYNC_GET_AVSYNC_INFO_S *)&sync_info);
			
            if (MT_SUCCESS == ret)
            {
                g_continue_pts = (mt_u64)sync_info.cur_apts * 1000;
                do_div(g_continue_pts, 45);
                chan->cur_play_vpts = sync_info.cur_vpts;
                MT_DBG_DEMUX("Stop data push, pts = %#x, %llu ms\n", sync_info.cur_vpts, g_continue_pts);
            }

			mutex_unlock(&g_demux_ch_mutex);
			
			for (i = 0; i < 5; i++)
			{
				msleep_interruptible(1);
				
				if (0 == g_channel_reset)
				{					
					break;
				}
			}
            
            break;
        }

        case CMD_DEMUX_CHAN_INDEX_RESUME:
        {
            mt_handle h_channel = *(mt_handle *)arg;
            mt_u32 i = 0;
            MT_UNF_DMX_CHAN_ATTR_S chAttr;
            mt_u32 chid = DMX_INVALID_CHAN_ID;

            memset(&chAttr, 0, sizeof(chAttr));
            ret = MT_DRV_DMX_GetChannelAttr(*(mt_handle*)arg, &chAttr);
            ret |= MT_DRV_DMX_GetChannelId(h_channel, &chid);

            if (MT_SUCCESS == ret)
            {
                if (MT_UNF_DMX_CHAN_TYPE_AUD == chAttr.enChannelType)
                {

                    for (i = 0; i < DMX_PROC_CH_MAX; i++)
                    {
                        if (chid == chan->main_chan_id[i])
                        {
                            mutex_lock(&g_demux_ch_mutex);			
                            chan->main_select_index = i;
                            chan->main_pre_index = i;
                            MT_DBG_DEMUX("%s, %d,  resume chan[%d], select_index = %d\n", __func__, __LINE__, chid , i);
                            mutex_unlock(&g_demux_ch_mutex);
                            break;
                        }
                        else
                        {
                            //do nothing
                        }
                    }

                    if (i >= DMX_PROC_CH_MAX)
                    {
                        MT_ERR_DEMUX("%s, %d, chid = %d not find\n", __func__, __LINE__, chid);
                    }
                }
                else
                {
                    MT_ERR_DEMUX("%s, %d, ChannelType = %d\n", __func__, __LINE__, chAttr.enChannelType);
                }
            }
            else
            {
                MT_ERR_DEMUX("%s, %d, ret = %#x\n", __func__, __LINE__, ret);
            }

            break;
        }

        case CMD_DEMUX_TRICK_SEEK_IN:
        {
            DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
            DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[0];

            /*
             * fix bug24399, if no video, do not pick aes
             */
            if ((DMX_INVALID_CHAN_ID == ChanInfo->ChanId) ||
                (DMX_INVALID_PID == ChanInfo->ChanPid))
            {
                ret = MT_SUCCESS;
                break;
            }

            mutex_lock(&g_demux_ch_mutex);
            chan->trick_seek_to_play = 1;
            sema_init(&chan->trick_seek_sem, 0);
            MT_DBG_DEMUX("%s, %d, start trick or seek\n", __func__, __LINE__);
            mutex_unlock(&g_demux_ch_mutex);
            ret = MT_SUCCESS;
            break;
        }

        case CMD_DEMUX_TRICK_SEEK_OUT:
        {
            DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
            DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[0];

            /*
             * fix bug24399, if no video, do not pick aes
             */
            if ((DMX_INVALID_CHAN_ID == ChanInfo->ChanId) ||
                (DMX_INVALID_PID == ChanInfo->ChanPid))
            {
                ret = MT_SUCCESS;
                break;
            }

            up(&chan->trick_seek_sem);
            MT_DBG_DEMUX("%s, %d, end trick or seek\n", __func__, __LINE__);
            ret = MT_SUCCESS;
            break;
        }

        default:
        {
            MT_ERR_DEMUX("unknown cmd: %#x\n", cmd);
        }
    }

    return ret;

}
#endif

static mt_s32 DMXT2miIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = MT_FAILURE;
	mt_u32 value = *(mt_u32 *)arg; 
	static mt_u32 input_ch = 0;//it will use by other function, so define as static
	mt_u32 output_ch = 0;
	mt_u32 Id = 0;
	DMX_T2MI_Para_S t2mi_para = {0};
	mt_u32 reg_value = 0;
	
    switch (cmd)
    {
		case CMD_DEMUX_T2MI_CONFIG:
		{
			t2mi_para = *(DMX_T2MI_Para_S*)arg;

			if (t2mi_para.enable)
			{
				reg_set_dmx_t2mi_en(1);
				if (MT_UNF_DMX_PORT_RAM_1 == t2mi_para.input)//sym4 only ram1 can inject into t2mi
				{
					reg_value = reg_get_dmx_t2mi_set2();
					reg_value |= (1<<20);//swtsi input enable
					reg_set_dmx_t2mi_set2(reg_value);
				}						

				input_ch = t2mi_para.input;//store the input channel
				
				reg_value = reg_get_dmx_t2mi_set1();
				reg_value &= ~((3<<28) | (3<<24) | (0xFF<<16) | 0x1FF);
				reg_value |= ((t2mi_para.input<<28) | (t2mi_para.output<<24) | (t2mi_para.plpid<<16) | t2mi_para.pid);
	    		reg_set_dmx_t2mi_set1(reg_value);				
			}
			else
			{
				reg_set_dmx_t2mi_en(0);
			}
			
			break;
		}
		
        case CMD_DEMUX_T2MI_ENABLE:
        {   
        	reg_set_dmx_t2mi_en(value);
            break;
        }
		
        case CMD_DEMUX_T2MI_SET_IN_CH:
        {	
        	printk(KERN_ERR "[%s %d]value=0x%x\n", __FUNCTION__, __LINE__, value);
		#ifdef CONFIG_MT_CHIP_SYMPHONY6
        	if ((value >= MT_UNF_DMX_PORT_TSI_0) && (value <= MT_UNF_DMX_PORT_TSI_3))
        	{
        		//input_ch = value - MT_UNF_DMX_PORT_TSI_0;

				//this refer 'DMX_MPI_PortGetTypeAndID'
        		Id = value - MT_UNF_DMX_PORT_TSI_0;

	            if (Id < DMX_TSIPORT_CNT)
	            {
	                input_ch = Id + DMX_IFPORT_CNT;
	            }
	            else if (Id == DMX_TSIPORT_CNT)
	            {
	                input_ch = Id - DMX_TSIPORT_CNT;
	            }
        	}
			else if ((value >= MT_UNF_DMX_PORT_RAM_0) && (value <= MT_UNF_DMX_PORT_RAM_3))
			{
				input_ch = value - MT_UNF_DMX_PORT_RAM_0 + 4;
			}
		#else
        	if ((value >= MT_UNF_DMX_PORT_TSI_0) && (value <= MT_UNF_DMX_PORT_TSI_3))
        	{
        		input_ch = value - MT_UNF_DMX_PORT_TSI_0;
        	}
			else if (MT_UNF_DMX_PORT_RAM_1 == value)//sym4 only ram1 can inject into t2mi
			{
				reg_set_dmx_t2mi_set2_t2mi_swtsi_en(1);
			}
		#endif
			else
			{
				MT_ERR_DEMUX("[%s %d]set in ch value(0x%x) is error\n", __FUNCTION__, __LINE__, value);
			}

			printk("[%s %d]input channel is: %d\n", __FUNCTION__, __LINE__, input_ch);
			reg_set_dmx_t2mi_set1_t2mi_input_ch(input_ch);
            break;
        }

        case CMD_DEMUX_T2MI_SET_OUT_CH:
        {  
        	if ((value >= MT_UNF_DMX_PORT_TSI_0) && (value <= MT_UNF_DMX_PORT_TSI_3))
        	{
        		//this refer 'DMX_MPI_PortGetTypeAndID'
        		Id = value - MT_UNF_DMX_PORT_TSI_0;

	            if (Id < DMX_TSIPORT_CNT)
	            {
	                output_ch = Id + DMX_IFPORT_CNT;
	            }
	            else if (Id == DMX_TSIPORT_CNT)
	            {
	                output_ch = Id - DMX_TSIPORT_CNT;
	            }
        		reg_set_dmx_t2mi_set1_t2mi_output_ch(output_ch);

				//because the input channel is write only
				//we must set it again after set output channel, pid or plpid
			#ifdef CONFIG_MT_CHIP_SYMPHONY4
				reg_set_dmx_t2mi_set1_t2mi_input_ch(input_ch);
			#endif
        	}			
			else
			{
				MT_ERR_DEMUX("[%s %d]set out ch value(0x%x) is error\n", __FUNCTION__, __LINE__, value);
			}
            break;
        }

		case CMD_DEMUX_T2MI_SET_PID:
        {   
        	reg_set_dmx_t2mi_set1_t2mi_pid(value);

			//because the input channel is write only
			//we must set it again after set output channel, pid or plpid
		#ifdef CONFIG_MT_CHIP_SYMPHONY4
			reg_set_dmx_t2mi_set1_t2mi_input_ch(input_ch);
		#endif
            break;
        }

		case CMD_DEMUX_T2MI_SET_PLPID:
        {
        	reg_set_dmx_t2mi_set1_t2mi_plp_id(value);

			//because the input channel is write only
			//we must set it again after set output channel, pid or plpid
		#ifdef CONFIG_MT_CHIP_SYMPHONY4
			reg_set_dmx_t2mi_set1_t2mi_input_ch(input_ch);
		#endif
		
            break;
        }

		case CMD_DEMUX_T2MI_SOFTRESET:
        {
        	DMX_OsiT2miSoftReset();
            break;
        }
		
        default:
        {
            MT_ERR_DEMUX("unknown cmd: %#x\n", cmd);
        }
    }

    return ret;

}

static mt_s32 DMXOtherIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
	mt_s32 ret = MT_FAILURE;
    mt_handle h_channel = 0;	
	
    switch (cmd)
    {
		case CMD_DEMUX_GET_CHAN_REF:
		{
			h_channel = *(mt_handle*)arg;
			ret = DMX_GetChanRef(DMX_CHANID(h_channel));
			break;
		}
		case CMD_DEMUX_GET_CHAN_AVID:
		{
			h_channel = *(mt_handle*)arg;
			ret = DMX_GetChanAVId(DMX_CHANID(h_channel));
			break;
		}
		
		default:
			break;
    }

	return ret;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
static mt_s32 DMX_CiplusIoctl(struct file *file, mt_u32 cmd, mt_void *arg)
{
	mt_s32 ret = MT_FAILURE;
	switch(cmd)
	{
		case CMD_TSI_CI_CHAN_EN: 
			{
				TSI_CIPlus_Enable_S * ci_para = (TSI_CIPlus_Enable_S*)arg;
				ret = MT_DRV_DMX_Channel_Ci_Enable(ci_para->hChannel,ci_para->isenable);
				break;
			}
		case CMD_TSI_CI_REC_CH_SET:
			{
				TSI_CIPlus_ChCfg_S *chcfg = (TSI_CIPlus_ChCfg_S*)arg;
				ret = MT_DRV_TSI_CI_Rec_Channel_Cfg(chcfg->chan);
				break;
			}
		case CMD_TSI_CI_SWTSI_CH_SET:
			{
				TSI_CIPlus_ChCfg_S *chcfg = (TSI_CIPlus_ChCfg_S*)arg;
				ret = MT_DRV_TSI_CI_Swtsi_Channel_Cfg(chcfg->chan);
				break;
			}
		case CMD_TSI_CI_BUF_CFG:
			{
				TSI_CIPlus_BufCfg_S *bufcfg = (TSI_CIPlus_BufCfg_S*)arg;
				ret = MT_DRV_TSI_CI_Buf_Cfg(bufcfg->use_cache);
				break;
			}
		case CMD_TSI_CI_CAMCLK_CFG:
			{
				TSI_CIPlus_CamClkCfg_S *clkcfg = (TSI_CIPlus_CamClkCfg_S*)arg;
				ret = MT_DRV_TSI_CI_Cicamclk_Cfg(clkcfg->div);
				break;
			}
		case CMD_TSI_CI_TSINTERVAL_CFG:
			{
				TSI_CIPlus_TsPktIntervalCfg_S *cfg = (TSI_CIPlus_TsPktIntervalCfg_S*)arg;
				ret = MT_DRV_TSI_CI_TsInterval_Cfg(cfg->interval);
				break;
			}
		case CMD_TSI_TSI2_SRC_CFG:
			{
				TSI_TSI2_SourceCfg_S *cfg = (TSI_TSI2_SourceCfg_S*)arg;
				ret = MT_DRV_TSI_Tsi2_Source_Cfg(cfg->from_cam,cfg->serial);
				break;
			}
		case CMD_TSI_CI_LLN_NUM_START_CFG:
			{
				TSI_CIPlus_LlnNumStart_S *cfg = (TSI_CIPlus_LlnNumStart_S*)arg;
				ret = MT_DRV_TSI_CI_LlnStartNum_Cfg(cfg->lln_num_start);
				break;
			}
		case CMD_TSI_CI_SWTSI_BYTEORDER_CFG:
			{
				TSI_CIPlus_SwtsiByteorder_S *cfg = (TSI_CIPlus_SwtsiByteorder_S*)arg;
				ret = MT_DRV_TSI_CI_SwtsiByteorder_Cfg(cfg->byteorder);
				break;
			}
		case CMD_TSI_CI_SWTSI_FULL_CFG:
			{
				TSI_CIPlus_SwtsiFullCfg_S *cfg = (TSI_CIPlus_SwtsiFullCfg_S*)arg;
				ret = MT_DRV_DMX_Channel_Swtsi_Full_Cfg(cfg->hChannel,cfg->care_full);
				break;
			}
		case CMD_TSI_CI_AHBRD_DELAY_CFG:
			{
				TSI_CIPlus_AhbRdDelay_S *cfg = (TSI_CIPlus_AhbRdDelay_S*)arg;
				ret = MT_DRV_TSI_CI_AhbRdDelay_Cfg(cfg->ahb_rd_delay);
				break;
			}
		case CMD_TSI_CI_GETSTATUS:
			{
				TSI_CIPlus_Status_S *st = (TSI_CIPlus_Status_S*)arg;
				ret = MT_DRV_TSI_CI_GetStatus(&st->bufstatus,&st->cistatus);
				break;
			}
		case CMD_TSI_CI_ENABLE_CFG:
			{
				TSI_CIPlus_EnableCfg_S *bufcfg = (TSI_CIPlus_EnableCfg_S*)arg;
				ret = MT_DRV_TSI_CI_Enable_Cfg(bufcfg->enable);
				break;
			}
		default:
			break;
	}
	return ret;
}
#endif
mt_s32 DMX_OsrIoctl(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32  ret     = MT_FAILURE;
    mt_u32  CmdType = cmd & DEMUX_CMD_MASK;

    //printk("DMX_OsrIoctl    cmd = 0x%x,  CmdType=%x\n",cmd,CmdType);

    switch (CmdType)
    {
        case DEMUX_GLOBAL_CMD:
        {
            ret = DMXGlobalIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_PORT_CMD:
        {
            ret = DMXPortIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_TSBUFFER_CMD:
        {
            ret = DMXTsBufferIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_CHAN_CMD:
        {
            ret = DMXChanIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_FILT_CMD:
        {
            ret = DMXFiltIoctl(file, cmd, arg);

            break;
        }

    #ifdef DMX_DESCRAMBLER_SUPPORT
        case DEMUX_KSET_CMD:
        {
            ret = DMXKeyIoctl(file, cmd, arg);

            break;
        }
    #endif

        case DEMUX_RECV_CMD:
        {
            ret = DMXRecvIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_PCR_CMD:
        {
            ret = DMXPcrIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_AV_CMD:
        {
            ret = DMXAvIoctl(file, cmd, arg);

            break;
        }

        case DEMUX_REC_CMD:
        {
            ret = DMXRecIoctl(file, cmd, arg);

            break;
        }
		
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
        case DEMUX_DATA_CMD:
        {
            ret = DMXDataIoctl(file, cmd, arg);
            break;
        }
#endif

		case DEMUX_T2MI_CMD:
        {
            ret = DMXT2miIoctl(file, cmd, arg);
            break;
        }

		case DEMUX_LINKREC_FASTPLAY_CMD:
        {
            ret = DMX_LinkRec_FastPlay_Ioctl(file, cmd, arg);

            break;
        }
		
		case DEMUX_OTHER1_CMD:
		case DEMUX_OTHER2_CMD:	
		{
			ret = DMXOtherIoctl(file, cmd, arg);
            break;
		}
		
#if defined(CONFIG_MT_CHIP_SYMPHONY6)		
		case DEMUX_CIPLUS_CMD: {
			ret = DMX_CiplusIoctl(file,cmd,arg);
			break;
		}
#endif		
        default:
        {
            MT_ERR_DEMUX("unknown cmd: 0x%x\n", cmd);
        }
    }

    return ret;
}

/*****************************************************************************
 Prototype    : DMX_DRV_Open
 Description  : open function in DEMUX module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
static mt_s32 DMX_DRV_Open(struct inode * inode, struct file * file)
{
    MT_S32 mt_idx = iminor(inode);
    demux_priv_data *demux_priv_data = get_mt_priv(mt_idx);

    if (!IS_ERR_OR_NULL(demux_priv_data->tsiclk))
        clk_prepare_enable(demux_priv_data->tsiclk);
	
    if (atomic_inc_return(&demux_priv_data->atmOpenCnt) == 1) {
		MT_DRV_DMX_Open();
    }
    return 0;
}

/*****************************************************************************
 Prototype    : DMX_DRV_Release
 Description  : release function in DEMUX module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
static mt_s32 DMX_DRV_Release(struct inode * inode, struct file * file)
{
    MT_S32 mt_idx = iminor(inode);
    demux_priv_data *demux_priv_data = get_mt_priv(mt_idx);
    if (atomic_dec_return(&demux_priv_data->atmOpenCnt) != 0) 
	{
		MT_ALWAYS_PRINT("%s  atmOpenCnt  is not zero !\n", __func__);
    }
	else
	{
	    DMXDataProcListInit(&demux_priv_data_info.aud_proc_list);

	    mutex_lock(&g_demux_ch_mutex);
	    MT_DRV_DMX_Close((ulong)file);
	    mutex_unlock(&g_demux_ch_mutex);

	    if (!IS_ERR_OR_NULL(demux_priv_data->tsiclk))
	        clk_disable_unprepare(demux_priv_data->tsiclk);
	}
    return 0;
}

/*****************************************************************************
 Prototype    : DMX_DRV_Ioctl
 Description  : Ioctl function in DEMUX module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
static long DMX_DRV_Ioctl(struct file *file, mt_u32 cmd, unsigned long arg)
{
    return (long) mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, DMX_OsrIoctl);
}

static struct file_operations DMX_DRV_Fops =
{
    .owner          = THIS_MODULE,
    .open           = DMX_DRV_Open,
    .unlocked_ioctl = DMX_DRV_Ioctl,
    .release        = DMX_DRV_Release,
};

static baseops_s dmx_drvops =
{
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = DMX_OsiSuspend,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = DMX_OsiResume,
};

#ifndef CONFIG_MT_USE_DATAPIPE_FLOW

extern mt_s32 __weak MT_DRV_ADEC_Reset_PTS_Queue(void);
extern mt_s32 __weak MT_DRV_ADEC_Write(mt_u32 index, phys_addr_t addr, mt_u32 len, mt_u64 pts, mt_u32 pts_valid);
extern mt_s32 __weak MT_DRV_ADEC_Write_AD(mt_u32 index, phys_addr_t addr, mt_u32 len, mt_u64 pts, mt_u32 pts_valid);
extern mt_u8 g_audioEsBufCleared;

/*
-------------------------------------------------------------------------
|                     the status definiton table                        |
-------------------------------------------------------------------------
| main_select_index  |  main_pre_index  |  equal  |  status             |
-------------------------------------------------------------------------
|       -1           |       -1         |   yes   |  not start          |
-------------------------------------------------------------------------
|       -1           |       -1         |   no    |  error              |
-------------------------------------------------------------------------
|       -1           |       !-1        |   yes   |  error              |
-------------------------------------------------------------------------
|       -1           |       !-1        |   no    |  will audio track   |
-------------------------------------------------------------------------
|       !-1          |       -1         |   yes   |  error              |
-------------------------------------------------------------------------
|       !-1          |       -1         |   no    |  start              |
-------------------------------------------------------------------------
|       !-1          |       !-1        |   yes   |  normal play        |
-------------------------------------------------------------------------
|       !-1          |       !-1        |   no    |  audio track        |
-------------------------------------------------------------------------
*/
static mt_s32 DMX_AV_Proc_Thread(void *arg)
{
    demux_priv_data *dmx = NULL;
    DMX_Stream_S stEsBuf;
    mt_u8 continue_data = 0;
    mt_u32 chid = DMX_INVALID_CHAN_ID;
    mt_u32 pid  = DMX_INVALID_PID;
    mt_s32 index = 0;
    mt_s32 ret = 0;
    demux_aud_ch_list *aud_proc_chan = NULL;

    dmx = (demux_priv_data *)arg;
    aud_proc_chan = &dmx->aud_proc_list;

    while (!kthread_should_stop())
    {
        if (0 == down_interruptible(&g_lock_av_sema))
        {
	    	if (!g_pDmxDevOsi)
            {
                up(&g_lock_av_sema);
                msleep_interruptible(5);
                continue;
            }

            if (0 == g_pDmxDevOsi->Reference)
            {
                up(&g_lock_av_sema);
                msleep_interruptible(5);
                continue;
            }

            mutex_lock(&g_demux_ch_mutex);

            if (g_audioEsBufCleared || g_channel_reset)
            {
                if ((chid != DMX_INVALID_CHAN_ID) &&
                    /* check whether the target channel closed or not */
                    ((chid == aud_proc_chan->main_chan_id[index]) ||
                     (chid == aud_proc_chan->desc_chan_id)))
                {
                    if (!g_audioEsBufCleared && (stEsBuf.u32BufVirAddr != 0) &&
                        (stEsBuf.u32BufLen > 0))
                    {
                        ret = DMX_OsiReleaseReadEs(chid, &stEsBuf);
                    }
                }

                memset(&stEsBuf, 0, sizeof(DMX_Stream_S));
                index = 0;

			
#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)
            MT_DRV_ADEC_Reset_PTS_Queue();
#endif
                g_channel_reset = 0;
            }

            continue_data = 0;
            g_audioEsBufCleared = 0;

            if (index < DMX_PROC_CH_MAX)
            {
                chid = aud_proc_chan->main_chan_id[index];

                while ((chid != DMX_INVALID_CHAN_ID) &&
                    (0 == g_channel_reset))
                {
                    memset(&stEsBuf, 0, sizeof(DMX_Stream_S));
                    stEsBuf.u64PtsMs = MT_INVALID_PTS_U64;
                    ret = DMX_OsiReadEsRequest(chid, &stEsBuf);
                    ret |= DMX_OsiGetChannelPid(chid, &pid);

                    /*
                    patch for bug126653, for multi audio, if the ret is not success
                    and the request channel is not the selected channel, do not goto
                    the_end, if not, the selected channel data will not send to adec
                    on time, and audio will under flow.
                    */
                    if ((MT_SUCCESS != ret) && (index == aud_proc_chan->main_select_index))
                    {
                        /*
                        for normal play, if get data failed, goto the_end, then read again,
                        if in audio track status, do not goto the_end, because read es would
                        failed, if goto the_end, aud_track_sem will never be valid, so app
                        will be blocked.
                        */
                        if ((aud_proc_chan->main_select_index == aud_proc_chan->main_pre_index) &&
                            (-1 != aud_proc_chan->main_pre_index) &&
                            (-1 != aud_proc_chan->main_select_index))
                        {
                            goto the_aud_ad;
                        }
                    }

                    /*
                    for normal play
                    */
                    if ((aud_proc_chan->main_select_index == aud_proc_chan->main_pre_index) &&
                        (-1 != aud_proc_chan->main_pre_index) &&
                        (-1 != aud_proc_chan->main_select_index))
                    {
                        if ((index == aud_proc_chan->main_select_index) &&
                            (stEsBuf.u32BufVirAddr != 0) &&
                            (stEsBuf.u32BufLen > 0))
                        {
                            if (!aud_proc_chan->trick_seek_to_play)
                            {
                        		ret = MT_DRV_ADEC_Write(pid, stEsBuf.u32BufPhyAddr, stEsBuf.u32BufLen, stEsBuf.u32PtsMs, stEsBuf.pts_valid);
    					
                                if (ret != MT_SUCCESS)
                                {
                                    continue_data = 0;
                                    goto the_aud_ad;
                                }
                                else
                                {
                                    continue_data = 1;
                                }
                            }
                            else
                            {
                                SYNC_GET_AVSYNC_INFO_S sync_info;

                                if (down_trylock(&aud_proc_chan->trick_seek_sem))
                                {
                                    /*
                                     * if try to get trick_seek_sem failed, unlock
                                     * the mutex, becase the trick_seek_sem set to
                                     * valid during av reset, and av reset need get
                                     * the mutex first, if not try but call
                                     * down_timeout directly, the sys will dead lock
                                     */
                                    mutex_unlock(&g_demux_ch_mutex);

                                    if (down_timeout(&aud_proc_chan->trick_seek_sem, 2000))
                                    {
                                        continue_data = 0;
                                        MT_WARN_DEMUX("get trick_seek_sem timeout\n");
                                        mutex_lock(&g_demux_ch_mutex);
                                        goto the_aud_ad;
                                    }

                                    mutex_lock(&g_demux_ch_mutex);
                                }

                                memset(&sync_info, 0, sizeof(SYNC_GET_AVSYNC_INFO_S));
                                ret = avsync_get_avsync_info((SYNC_GET_AVSYNC_INFO_S *)&sync_info);

                                if (MT_SUCCESS != ret)
                                {
                                    continue_data = 0;
                                    up(&aud_proc_chan->trick_seek_sem);
                                    goto the_aud_ad;
                                }

                                if (0 == sync_info.cur_vpts)
                                {
                                    continue_data = 0;
                                    up(&aud_proc_chan->trick_seek_sem);
                                    goto the_aud_ad;
                                }

                                MT_DBG_DEMUX("trickseek to play, vpts = %#x\n", sync_info.cur_vpts);
                                aud_proc_chan->cur_play_vpts = sync_info.cur_vpts;
                                memset(&stEsBuf, 0, sizeof(DMX_Stream_S));
                                stEsBuf.u64PtsMs = MT_INVALID_PTS_U64;
                                ret = DMX_OsiReadEsRequestByPtsForTrickSeek(chid, &stEsBuf, aud_proc_chan->cur_play_vpts);

                                if (MT_SUCCESS != ret)
                                {
                                    continue_data = 0;
                                    up(&aud_proc_chan->trick_seek_sem);
                                    goto the_aud_ad;
                                }

                                if (stEsBuf.u32PtsMs > aud_proc_chan->cur_play_vpts)
                                {
                                    avsync_trick_seek_to_normal();
                                }

                                DMX_OsiGetChannelPid(chid, &pid);
                                ret = MT_DRV_ADEC_Write(pid, stEsBuf.u32BufPhyAddr, stEsBuf.u32BufLen, stEsBuf.u32PtsMs, stEsBuf.pts_valid);

                                if (MT_SUCCESS != ret)
                                {
                                    continue_data = 0;
                                    up(&aud_proc_chan->trick_seek_sem);
                                    goto the_aud_ad;
                                }
                                else
                                {
                                    continue_data = 1;
                                }

                                aud_proc_chan->trick_seek_to_play = 0;
                            }
                        }
                        else
                        {
                            /*
                             * if the channel is not the selected, do not
                             * sleep regardless of whether the data is
                             * got or not.
                             */
                            continue_data = 1;
                        }
                    }
                    /*
                    for audio track
                    */
                    else if ((aud_proc_chan->main_select_index != aud_proc_chan->main_pre_index) &&
                        (-1 != aud_proc_chan->main_pre_index) &&
                        (-1 != aud_proc_chan->main_select_index))
                    {
                        chid = aud_proc_chan->main_chan_id[aud_proc_chan->main_select_index];
                        memset(&stEsBuf, 0, sizeof(DMX_Stream_S));
                        stEsBuf.u64PtsMs = MT_INVALID_PTS_U64;
                        ret = DMX_OsiReadEsRequestByPtsForAudTrack(chid, &stEsBuf, aud_proc_chan->cur_play_vpts);
                        ret |= DMX_OsiGetChannelPid(chid, &pid);

                        if (MT_SUCCESS != ret)
                        {
                            continue_data = 0;
                            goto the_aud_ad;
                        }

                        aud_proc_chan->main_pre_index = aud_proc_chan->main_select_index;
                        MT_DBG_DEMUX("%s, %d, audio track finished, sel = %d\n", __func__, __LINE__, aud_proc_chan->main_select_index);
                        up(&aud_proc_chan->aud_track_sem);
                        ret = MT_DRV_ADEC_Write(pid, stEsBuf.u32BufPhyAddr, stEsBuf.u32BufLen, stEsBuf.u32PtsMs, stEsBuf.pts_valid);

                        if (MT_SUCCESS != ret)
                        {
                            continue_data = 0;
                            goto the_aud_ad;
                        }

                        continue_data = 1;
                    }

                    /*
                    for will audio track, just release the read data
                    */
                    else if ((-1 != aud_proc_chan->main_pre_index) &&
                        (-1 == aud_proc_chan->main_select_index))
                    {
                        //do noting
                    }
                    /*
                    for normal play, if the pre index is -1, means the first time get data, write data to adec.
                    */
                    else if ((-1 == aud_proc_chan->main_pre_index) &&
                        (-1 != aud_proc_chan->main_select_index))
                    {
                        if ((index == aud_proc_chan->main_select_index) &&
                            (stEsBuf.u32BufVirAddr != 0) &&
                            (stEsBuf.u32BufLen > 0))
                        {
                            if (!aud_proc_chan->trick_seek_to_play)
                            {
                                ret = MT_DRV_ADEC_Write(pid, stEsBuf.u32BufPhyAddr, stEsBuf.u32BufLen, stEsBuf.u32PtsMs, stEsBuf.pts_valid);

                                if (ret != MT_SUCCESS)
                                {
                                    continue_data = 0;
                                    goto the_aud_ad;
                                }
                                else
                                {
                                    continue_data = 1;
                                }
                            }
                            else
                            {
                                /*
                                 * for trick mode, after pvr resume, if get aes
                                 * data successed, do not update to adec, the
                                 * purpose is to avoid redundant code for
                                 * checking apts twice. And switch the status
                                 * to normal play, goto the_end, not the_aud_ad
                                 */
                                aud_proc_chan->main_pre_index = aud_proc_chan->main_select_index;
                                continue_data = 1;
                                goto the_end;
                            }
                        }
                        else
                        {
                            /*
                             * if the channel is not the selected, do not
                             * sleep regardless of whether the data is
                             * got or not.
                             */
                            continue_data = 1;
                        }

                        aud_proc_chan->main_pre_index = aud_proc_chan->main_select_index;
                        MT_DBG_DEMUX("%s, %d, first get data, chid = %d\n", __func__, __LINE__,
                            aud_proc_chan->main_pre_index);
                    }
                    else
                    {
                        MT_ERR_DEMUX("%s, %d, error aud case!!!\n", __func__, __LINE__);
                    }

                    /* release data */
                    if ((stEsBuf.u32BufVirAddr != 0) &&
                        (stEsBuf.u32BufLen > 0))
                    {
                        ret = DMX_OsiReleaseReadEs(chid, &stEsBuf);
                        memset(&stEsBuf, 0, sizeof(DMX_Stream_S));
                    }

                    index++;

                    if (index >= DMX_PROC_CH_MAX)
                    {
                        break;
                    }

                    chid = aud_proc_chan->main_chan_id[index];
                };
            }

            if (1 == g_channel_reset)
            {
                goto the_end;
            }

            /* reset loop index */
            index = 0;
            pid = DMX_INVALID_PID;

    the_aud_ad:
            /* process ad */
#ifdef CONFIG_MT_AUDIO_AD
            chid = aud_proc_chan->desc_chan_id;

            if (chid != DMX_INVALID_CHAN_ID)
            {
                memset(&stEsBuf, 0, sizeof(DMX_Stream_S));
                ret = DMX_OsiReadEsRequest(chid, &stEsBuf);
                ret |= DMX_OsiGetChannelPid(chid, &pid);

                if (MT_SUCCESS != ret)
                {
                    continue_data = 0;
                    goto the_end;
                }

                if ((stEsBuf.u32BufVirAddr != 0) &&
                    (stEsBuf.u32BufLen > 0))
                {
                    ret = MT_DRV_ADEC_Write_AD(pid, stEsBuf.u32BufPhyAddr, stEsBuf.u32BufLen, stEsBuf.u32PtsMs, stEsBuf.pts_valid);

                    if (ret != MT_SUCCESS)
                    {
                        continue_data = 0;
                        goto the_end;
                    }

                    ret = DMX_OsiReleaseReadEs(chid, &stEsBuf);
                    continue_data = 1;
                }
            }
#endif

    the_end:
            mutex_unlock(&g_demux_ch_mutex);
            up(&g_lock_av_sema);

            if (0 == continue_data)
            {
                msleep_interruptible(2);
            }
        }
    }

    return 0;
}

#endif

static mt_s32 DMX_PES_Proc_Thread(void *arg)
{
    DMX_ChanInfo_S *chan_info = NULL;
    DMX_ChanInfo_S *chan_next = NULL;
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;
    struct list_head *pes_chan_list_head = NULL;
    mt_u32 ret = 0;

    while (!kthread_should_stop())
	{
	    if (0 == down_interruptible(&g_lock_pes_sema)) 
	    {
	        if (!g_pDmxDevOsi)
	        {
	        	up(&g_lock_pes_sema);
	            msleep_interruptible(5);
	            continue;
	        }

	        if (0 == g_pDmxDevOsi->Reference)
	        {
	        	up(&g_lock_pes_sema);
	            msleep_interruptible(5);
	            continue;
	        }

	        pes_chan_list_head = &g_pDmxDevOsi->pes_chan_list;
	        mutex_lock(&g_pDmxDevOsi->pes_chan_list_mutex);

	        if (list_empty_careful(pes_chan_list_head))
	        {
	        	up(&g_lock_pes_sema);
	            mutex_unlock(&g_pDmxDevOsi->pes_chan_list_mutex);
	            msleep_interruptible(10);
	            continue;
	        }

	        list_for_each_entry_safe(chan_info, chan_next, pes_chan_list_head, chan_node)
	        {
	            mutex_lock(&chan_info->chan_mutex);
	            peshead = chan_info->softpeshead;

	            if (peshead)
	            {
	                ret = DMX_OsiParsePes(chan_info->ChanId);

	                if (!list_empty_careful(&peshead->pes_used_node_list))
	                {
	                    p_pes_node = list_entry(peshead->pes_used_node_list.next, DMX_SOFT_PES_NODE_S, pes_node);

	                    if ((p_pes_node->pes_node_len) && (p_pes_node->pes_node_addr) && (p_pes_node->pes_node_merged))
	                    {
	                        wake_up(&chan_info->pes_wait);
	                    }
	                }

	            }
	            else
	            {
	                MT_ERR_DEMUX("peshead is null\n");
	            }

	            mutex_unlock(&chan_info->chan_mutex);
	        }

	        mutex_unlock(&g_pDmxDevOsi->pes_chan_list_mutex);
			up(&g_lock_pes_sema);
	        msleep_interruptible(4);
	    }
    }
	return 0;
}

mt_s32 dmx_get_audio_es_buf_id(mt_u32 *buf_id)
{
    mt_u32 select_id = 0;
    mt_u32 ch_id = 0;

    if (NULL == buf_id)
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    select_id = demux_priv_data_info.aud_proc_list.main_select_index;
    ch_id = demux_priv_data_info.aud_proc_list.main_chan_id[select_id];

    if (g_pDmxDevOsi)
    {
        *buf_id = g_pDmxDevOsi->DmxChanInfo[ch_id].avChanId;
    }
    else
    {
        return MT_ERR_DMX_NULL_PTR;
    }

    return MT_SUCCESS;;
}
EXPORT_SYMBOL(dmx_get_audio_es_buf_id);

/*****************************************************************************
 Prototype    : DMX_DRV_ModInit
 Description  : initialize function in DEMUX module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
static mt_void do_dmx_drv_modexit(mt_void);

mt_s32 __init DMX_DRV_ModInit(mt_void)
{
#ifdef MT_DEMUX_PROC_SUPPORT
    mt_proc_entry_t *item;
    mt_drv_proc_t stFnOpt = {0};
#endif
    struct sched_param param;

    demux_priv_data_info.tsiclk = clk_get(NULL, "tsiclk");
    if (!IS_ERR_OR_NULL(demux_priv_data_info.tsiclk))
        clk_prepare_enable(demux_priv_data_info.tsiclk);
    
    DMXDataProcListInit(&demux_priv_data_info.aud_proc_list);
	atomic_set(&demux_priv_data_info.atmOpenCnt, 0);

#ifndef MT_MCE_SUPPORT
    if (MT_SUCCESS != MT_DRV_DMX_Init())
    {
        goto out;
    }
#endif

    strncpy(g_stDemuxDev.devfs_name, UMAP_DEVNAME_DEMUX, strlen(UMAP_DEVNAME_DEMUX));
    g_stDemuxDev.devfs_name[strlen(UMAP_DEVNAME_DEMUX)] = '\0';
    g_stDemuxDev.fops   = &DMX_DRV_Fops;
    g_stDemuxDev.minor  = UMAP_MIN_MINOR_DEMUX;
    g_stDemuxDev.owner  = THIS_MODULE;
    g_stDemuxDev.drvops = &dmx_drvops;
    g_stDemuxDev.priv = &demux_priv_data_info;

    if (mt_drv_dev_register(&g_stDemuxDev) < 0)
    {
        MT_FATAL_DEMUX("Unable to register demux dev\n");
        goto out;
    }

	MT_INIT_MUTEX(&g_lock_av_sema);
	MT_INIT_MUTEX(&g_lock_pes_sema);

	mutex_init(&g_demux_ch_mutex);
	
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
    demux_av_thread = kthread_create(DMX_AV_Proc_Thread, &demux_priv_data_info, "av_proc");

    if (NULL == demux_av_thread)
    {
        MT_FATAL_DEMUX("Create demux kthread failed\n");
        goto out;
    }
	
	param.sched_priority = 99;
	sched_setscheduler(demux_av_thread, SCHED_RR, &param);

    if(!IS_ERR(demux_av_thread))
        wake_up_process(demux_av_thread);
#endif

    demux_pes_thread = kthread_create(DMX_PES_Proc_Thread, NULL, "pes_proc");

    if (NULL == demux_pes_thread)
    {
        MT_FATAL_DEMUX("Create demux kthread failed\n");
        goto out;
    }
	
	param.sched_priority = 99;
	sched_setscheduler(demux_pes_thread, SCHED_RR, &param);

    if (!IS_ERR(demux_pes_thread))
        wake_up_process(demux_pes_thread);

#ifdef MT_DEMUX_PROC_SUPPORT
    item = mt_drv_proc_add_module("demux_main", NULL, NULL);
    if (!item)
    {
        MT_ERR_DEMUX("add proc demux_main failed\n");
    }
    else
    {
        item->read  = DMXProcRead;
        item->write = DMXProcWrite;
    }

    stFnOpt.fnRead = DMXPortProcRead;
    item = mt_drv_proc_add_module("demux_port", &stFnOpt, NULL);
    if (!item)
    {
        MT_ERR_DEMUX("add proc demux_port failed\n");
    }

    stFnOpt.fnRead = DMXChanProcRead;
    item = mt_drv_proc_add_module("demux_chan", &stFnOpt, NULL);
    if (!item)
    {
        MT_ERR_DEMUX("add proc demux_chan failed\n");
    }

    stFnOpt.fnRead = DMXFilterProcRead;
    item = mt_drv_proc_add_module("demux_filter", &stFnOpt, NULL);
    if (!item)
    {
        MT_ERR_DEMUX("add proc demux_filter failed\n");
    }

#ifdef DMX_DESCRAMBLER_SUPPORT
    stFnOpt.fnRead = DMXKeyProcRead;
    item = mt_drv_proc_add_module("demux_key", &stFnOpt, NULL);
    if (!item)
    {
        MT_ERR_DEMUX("add proc demux_key failed\n");
    }
#endif

    stFnOpt.fnRead = DMXRecProcRead;
    item = mt_drv_proc_add_module("demux_rec", &stFnOpt, NULL);
    if (!item)
    {
        MT_ERR_DEMUX("add proc demux_rec failed\n");
    }

#endif	

    MT_ALWAYS_PRINT("MT_SUCCESS\n");

    if (!IS_ERR_OR_NULL(demux_priv_data_info.tsiclk))
        clk_disable_unprepare(demux_priv_data_info.tsiclk);

    return MT_SUCCESS;

out :
    do_dmx_drv_modexit();
    MT_ALWAYS_PRINT("MT_FAILURE\n");
    return MT_FAILURE;
}

/*****************************************************************************
 Prototype    : DMX_DRV_ModExit
 Description  : exit function in DEMUX module
 Input        : None
 Output       : None
 Return Value :
*****************************************************************************/
static mt_void do_dmx_drv_modexit(mt_void)
{
    if (!IS_ERR_OR_NULL(demux_priv_data_info.tsiclk))
        clk_prepare_enable(demux_priv_data_info.tsiclk);

#ifdef MT_DEMUX_PROC_SUPPORT
    mt_drv_proc_rm_module("demux_rec_index");
    mt_drv_proc_rm_module("demux_rec");
    mt_drv_proc_rm_module("demux_pcr");
#ifdef DMX_DESCRAMBLER_SUPPORT
    mt_drv_proc_rm_module("demux_key");
#endif
    mt_drv_proc_rm_module("demux_filter");
    mt_drv_proc_rm_module("demux_chanbuf");
    mt_drv_proc_rm_module("demux_chan");
    mt_drv_proc_rm_module("demux_tsbuf");
    mt_drv_proc_rm_module("demux_port");
    mt_drv_proc_rm_module("demux_main");
#endif

#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
    if(demux_av_thread) {
        kthread_stop(demux_av_thread);
        demux_av_thread = NULL;
    }
#endif

    mt_drv_dev_unregister(&g_stDemuxDev);

#ifndef MT_MCE_SUPPORT
    MT_DRV_DMX_DeInit();
#endif

    if (!IS_ERR_OR_NULL(demux_priv_data_info.tsiclk)) {
        clk_disable_unprepare(demux_priv_data_info.tsiclk);
        clk_put(demux_priv_data_info.tsiclk);
    }
}

mt_void __exit DMX_DRV_ModExit(mt_void)
{
	do_dmx_drv_modexit();
}
