/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/version.h>
#include <linux/proc_fs.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/delay.h>

#include <linux/uaccess.h>
#include <asm/io.h>
#include <linux/seq_file.h>

#include "hi_drv_mmz.h"
#include "hi_drv_stat.h"
#include "hi_drv_sys.h"
#include "hi_drv_proc.h"

#include "hal_tianlai_adac.h"
#include "hi_drv_ao.h"

#include "hi_reg_common.h"
#include "drv_ao_private.h"

#define  DBG_ADAC_DISABLE_TIMER

/*----------------------------audio codec-----------------------------------*/

/*
0~0x7f
0:  +6dB
6:   0dB
7f: -121dB
 */
 static mt_void Digfi_DacSetVolume(mt_u32 left, mt_u32 right)
{
    SC_PERI_TIANLAI_ADAC0 Adac0;


    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    
    Adac0.bits.dacr_vol = right;
    Adac0.bits.dacl_vol = left;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;
}

#if 0
static mt_void Digfi_DacGetVolume(mt_u32 *left, mt_u32 *right)
{
    SC_PERI_TIANLAI_ADAC0 Adac0;

    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    *left = Adac0.bits.dacl_vol;
    *right = Adac0.bits.dacr_vol;
}

static mt_s32 Digfi_DacGetMute(mt_void)
{
    SC_PERI_TIANLAI_ADAC1 Adac1;

    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    return Adac1.bits.smutel;
}
#endif

mt_void Digfi_DacSetMute(mt_void)
{
    SC_PERI_TIANLAI_ADAC1 Adac1;

    // soft mute digi
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    
    Adac1.bits.smuter = 1;
    Adac1.bits.smutel = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32 = Adac1.u32;
}


mt_void Digfi_DacSetUnmute(mt_void)
{
    SC_PERI_TIANLAI_ADAC1 Adac1;

    // soft mute digi
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;

    Adac1.bits.smuter = 0;
    Adac1.bits.smutel = 0;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32 = Adac1.u32;
}

mt_void Digfi_DacSetSampleRate(MT_UNF_SAMPLE_RATE_E SR)
{
    SC_PERI_TIANLAI_ADAC1 Adac1;

    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    switch (SR)
    {
    case MT_UNF_SAMPLE_RATE_176K:
    case MT_UNF_SAMPLE_RATE_192K:
        Adac1.bits.sample_sel = 4;
        break;

    case MT_UNF_SAMPLE_RATE_88K:
    case MT_UNF_SAMPLE_RATE_96K:
        Adac1.bits.sample_sel = 3;
        break;

    case MT_UNF_SAMPLE_RATE_32K:
    case MT_UNF_SAMPLE_RATE_44K:
    case MT_UNF_SAMPLE_RATE_48K:
        Adac1.bits.sample_sel = 2;
        break;

    case MT_UNF_SAMPLE_RATE_16K:
    case MT_UNF_SAMPLE_RATE_22K:
    case MT_UNF_SAMPLE_RATE_24K:
        Adac1.bits.sample_sel = 1;
        break;

    case MT_UNF_SAMPLE_RATE_8K:
    case MT_UNF_SAMPLE_RATE_11K:
    case MT_UNF_SAMPLE_RATE_12K:
        Adac1.bits.sample_sel = 0;
        break;

    default:
        Adac1.bits.sample_sel = 2;
        break;
    }
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32 = Adac1.u32;

    return;
}

static mt_void Digfi_DacPoweup(MT_BOOL bResume)
{
    SC_PERI_TIANLAI_ADAC0 Adac0;
    SC_PERI_TIANLAI_ADAC1 Adac1; 
#ifndef MT_SND_MUTECTL_SUPPORT
    if(MT_TRUE == bResume)
    {
        //msleep(100); //add for resume popfree
    }
#endif
    /* open soft mute */
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.smuter = 1;
    Adac1.bits.smutel = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;

    /* step 1: open popfree */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.popfreel = 1;
    Adac0.bits.popfreer = 1;
#ifndef MT_SND_MUTECTL_SUPPORT    
    Adac0.bits.fs = 0; //add for popfree
#else
    if(MT_TRUE == bResume)
    {
        Adac0.bits.fs = 1; 
    }
    else
    {
        Adac0.bits.fs = 0; 
    }
#endif    
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;

    /*step 2: pd_vref power up	*/
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.pd_vref = 0;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;//0xf0 

    /*step 3: open DAC */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.pd_dacr = 0;
    Adac0.bits.pd_dacl = 0;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;//0x30

#ifdef MT_SND_MUTECTL_SUPPORT  
    if(MT_TRUE == bResume)
    {
        msleep(10);   
        Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
        Adac0.bits.fs = 0; //add for popfree
        g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;
    }
#endif  

    /*step 4: close profree */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.popfreel = 0;
    Adac0.bits.popfreer = 0;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;//0x00

    /*step 5: disable mute */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.mute_dacl = 0;
    Adac0.bits.mute_dacr = 0;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;

    /* soft unmute */
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.smutel = 0;      
    Adac1.bits.smuter = 0;
    Adac1.bits.sunmutel = 1;
    Adac1.bits.sunmuter = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;
     
    return;
}

static mt_void Digfi_DacPowedown(MT_BOOL bSuspend)
{
    SC_PERI_TIANLAI_ADAC0 Adac0;
    SC_PERI_TIANLAI_ADAC1 Adac1;

    // soft mute digi
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.smutel = 1;
    Adac1.bits.smuter = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;
    //udelay(1000); // request??

    /*step 1: enable mute */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.mute_dacl = 1;
    Adac0.bits.mute_dacr = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;

    /*step 2: open popfree */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.popfreel = 1;
    Adac0.bits.popfreer = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;

    /*step 3: close DAC */
    Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.pd_dacl = 1;
    Adac0.bits.pd_dacr = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;
#ifndef MT_SND_MUTECTL_SUPPORT
    if(MT_TRUE == bSuspend) //add for suspend popfree
    {
        Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
        Adac0.bits.fs = 0;
        Adac0.bits.popfreel = 1;
        Adac0.bits.popfreer = 1;
        Adac0.bits.pd_vref = 1;
        g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;
 
        msleep(2*1000);
        Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
        Adac0.bits.fs = 1;
        Adac0.bits.popfreel = 0;
        Adac0.bits.popfreer = 0;
        g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;
}
else
#endif
{
	/*step 4: pd_vref power down	*/
	Adac0.u32 = g_pstRegPeri->PERI_TIANLA_ADAC0.u32;
    Adac0.bits.pd_vref = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC0.u32 = Adac0.u32;
}

    return;
}


static mt_void Digfi_DacInit(MT_UNF_SAMPLE_RATE_E SR, MT_BOOL bResume)
{
    Digfi_DacPoweup(bResume);
    Digfi_DacSetSampleRate(SR);
    Digfi_DacSetVolume(0x06, 0x06);   /* 0dB */
}


/* same as old ADAC */
static mt_void Digfi_ADACEnable(mt_void)
{
    U_S40_TIANLAI_ADAC_CRG AdacCfg;
    SC_PERI_TIANLAI_ADAC1 Adac1;

    AdacCfg.u32 = g_pstRegCrg->PERI_CRG69.u32;
    AdacCfg.bits.adac_cken = 1;  /* ADAC  clk en */
    g_pstRegCrg->PERI_CRG69.u32 = AdacCfg.u32;
    udelay(10);

    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.rst = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.rst = 0;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;

    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.clksel2 = 0;			/* ADAC clksel2  2DIV */
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;

    AdacCfg.u32 = g_pstRegCrg->PERI_CRG69.u32;
    AdacCfg.bits.adac_srst_req = 1;  /* ADAC  soft reset */
    g_pstRegCrg->PERI_CRG69.u32 = AdacCfg.u32;
    AdacCfg.u32 = g_pstRegCrg->PERI_CRG69.u32;
    AdacCfg.bits.adac_srst_req = 0; /* ADAC  undo soft reset */
    g_pstRegCrg->PERI_CRG69.u32 = AdacCfg.u32;
    return;
}

/* same as old ADAC */
static mt_void Digfi_ADACDisable(mt_void)
{
    U_S40_TIANLAI_ADAC_CRG AdacCfg;
    SC_PERI_TIANLAI_ADAC1 Adac1;
    
    Adac1.u32 = g_pstRegPeri->PERI_TIANLA_ADAC1.u32;
    Adac1.bits.rst = 1;
    g_pstRegPeri->PERI_TIANLA_ADAC1.u32= Adac1.u32;

    AdacCfg.u32 = g_pstRegCrg->PERI_CRG69.u32;
    AdacCfg.bits.adac_srst_req = 1;  /* ADAC Datapath soft reset */
    g_pstRegCrg->PERI_CRG69.u32 = AdacCfg.u32;

    AdacCfg.u32 = g_pstRegCrg->PERI_CRG69.u32;
    AdacCfg.bits.adac_cken = 0;  /* ADAC  clk disable */
    g_pstRegCrg->PERI_CRG69.u32 = AdacCfg.u32;
    return;
}
/*
The start-up sequence consists on several steps in a pre-determined order as follows:
1. select the master clock mode (256 or 384 x Fs)
2. start the master clock
3. set pdz to high
4. select the sampling rate
5. reset the signal path (rstdpz to low and back to high after 100ns)
6. start the individual codec blocks
 */

mt_void ADAC_TIANLAI_Init(MT_UNF_SAMPLE_RATE_E enSR, MT_BOOL bResume)
{
    Digfi_ADACEnable();
    msleep(1);   //discharge
    Digfi_DacInit(enSR, bResume);
}

mt_void ADAC_TIANLAI_DeInit(MT_BOOL bSuspend)
{
    Digfi_DacPowedown(bSuspend);
    Digfi_ADACDisable();
    return;
}

