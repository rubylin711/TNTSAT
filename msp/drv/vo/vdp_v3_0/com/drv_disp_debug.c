
/******************************************************************************
  Copyright (C), 2017, Montage Tech. Co., Ltd.
******************************************************************************
File Name     : drv_disp_debug.c
Version       : Initial Draft
Author        : Montage multimedia software group
Created       : 2017/01/30
Last Modified :
Description   :
Function List :
History       :
******************************************************************************/

#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <linux/fs.h>
#include <linux/mm.h>

#include <linux/uaccess.h>

#include "drv_disp_debug.h"
#include "drv_disp_com.h"
#include "mt_drv_win.h"
#include "drv_window.h"
#include "mt_osal.h"
#include "mt_module_debug.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

int vdp_str2val(char *str, unsigned int *data)
{
    unsigned int i, d, dat, weight;

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
        if(str[i] < 0x20)
        {
            break;
        }
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

/************************************************************************/
/* file tell position                                                    */
/************************************************************************/
struct file *vdp_k_fopen(const char *filename, int flags, int mode)
{
    struct file *filp = filp_open(filename, flags, mode);
    return (IS_ERR(filp)) ? NULL : filp;
}

void vdp_k_fclose(struct file *filp)
{
    if (filp)
    {
        filp_close(filp, NULL);
    }
}

int vdp_k_fread(char *buf, unsigned int len, struct file *filp)
{
#if 0
        int readlen;
        mm_segment_t oldfs;

        if (filp == NULL)
                return -ENOENT;
        if (filp->f_op->read == NULL)
                return -ENOSYS;
        if (((filp->f_flags & O_ACCMODE) & (O_RDONLY | O_RDWR)) == 0)
                return -EACCES;

        oldfs = get_fs();

        set_fs(KERNEL_DS);

        readlen = filp->f_op->read(filp, buf, len, &filp->f_pos);

        set_fs(oldfs);

        return readlen;
#endif
    return 0;	
}

int vdp_k_fwrite(char *buf, int len, struct file *filp)
{
#if 0
        int writelen;
        mm_segment_t oldfs;

        if (filp == NULL)
                return -ENOENT;
        if (filp->f_op->write == NULL)
                return -ENOSYS;
        if (((filp->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
                return -EACCES;
        oldfs = get_fs();
        set_fs(KERNEL_DS);
        writelen = filp->f_op->write(filp, buf, len, &filp->f_pos);
        set_fs(oldfs);

        return writelen;
#endif
    return 0;

}

//#define VDP_DEBUG_USE_IOREMAP 1

mt_s32 vdp_k_SaveYUVImg(struct file *pfYUV, MT_DRV_VIDEO_FRAME_S *pstFrame, mt_s32 num)
{
    mmz_buffer_s stMBuf;
    mt_u8 *ptr;
    mt_u8 *pu8Udata;
    mt_u8 *pu8Vdata;
    mt_u8 *pu8Ydata;
    mt_u32 i,j;
    mt_s32 nRet = MT_SUCCESS;

    if ((!pstFrame) || (!pfYUV) )
    {
        MT_ERR_WIN("pstFrame is null!\n");
        return MT_FAILURE;
    }

    memset((void*)&stMBuf, 0, sizeof(mmz_buffer_s));

    stMBuf.startPhyAddr = pstFrame->stBufAddr[0].u32PhyAddr_Y;
    if (!stMBuf.startPhyAddr)
    {
        MT_ERR_WIN("address '0x%x' is null!\n", pstFrame->stBufAddr[0].u32PhyAddr_Y);
        return MT_FAILURE;
    }

#ifdef VDP_DEBUG_USE_IOREMAP
#else
    MT_INFO_VO("========================map phy addr =0x%x\n", stMBuf.startPhyAddr);

    //nRet = MT_DRV_MMZ_Map(&stMBuf);
    //ptr = (mt_u8 *)stMBuf.u32StartVirAddr);
    ptr = (mt_u8 *)phys_to_virt(stMBuf.startPhyAddr);
    nRet = ptr ? MT_SUCCESS : MT_FAILURE;
#endif
    if (nRet)
    {
        MT_ERR_WIN("address '0x%x' is not valid!\n", pstFrame->stBufAddr[0].u32PhyAddr_Y);
        return MT_FAILURE;
    }

    pu8Udata = (mt_u8 *)DISP_MALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 /2);
    if (!pu8Udata)
        goto EXIT3;
    pu8Vdata = (mt_u8 *)DISP_MALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 /2);
    if ( !pu8Vdata)
        goto EXIT2;
    pu8Ydata = (mt_u8 *)DISP_MALLOC(pstFrame->stBufAddr[0].u32Stride_Y);
    if (!pu8Ydata)
       goto EXIT1;


    /*write Y*/
    for (i=0; i<pstFrame->u32Height; i++)
    {
        memcpy(pu8Ydata, ptr, sizeof(mt_u8)*pstFrame->stBufAddr[0].u32Stride_Y);

        if(pstFrame->u32Width != vdp_k_fwrite(pu8Ydata, pstFrame->u32Width, pfYUV))
        {
            MT_ERR_WIN("line %d: fwrite fail!\n",__LINE__);
        }
        ptr += pstFrame->stBufAddr[0].u32Stride_Y;
    }

    /* U V transfer and save */
    for (i=0; i<pstFrame->u32Height/2; i++)
    {
        for (j=0; j<pstFrame->u32Width/2; j++)
        {
            if(pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV21)
            {
                pu8Vdata[i*pstFrame->u32Width/2+j] = ptr[2*j];
                pu8Udata[i*pstFrame->u32Width/2+j] = ptr[2*j+1];
            }
            else
            {
                pu8Udata[i*pstFrame->u32Width/2+j] = ptr[2*j];
                pu8Vdata[i*pstFrame->u32Width/2+j] = ptr[2*j+1];
            }
        }
        ptr += pstFrame->stBufAddr[0].u32Stride_C;
    }

    /*write U */
    vdp_k_fwrite(pu8Udata, pstFrame->u32Width * pstFrame->u32Height / 2 /2, pfYUV);

    /*write V */
    vdp_k_fwrite(pu8Vdata, pstFrame->u32Width * pstFrame->u32Height / 2 /2, pfYUV);

EXIT1:
   DISP_FREE(pu8Ydata);
EXIT2:
    DISP_FREE(pu8Vdata);
EXIT3:
    DISP_FREE(pu8Udata);

#ifdef VDP_DEBUG_USE_IOREMAP
    iounmap(ptr);
#else
    mt_drv_mmz_unmap(&stMBuf);
#endif

    return MT_SUCCESS;
}

static mt_char u8VdpDebugStr[256];
static mt_char s_VdpSavePath[64] = {'/','m','n','t',0};

mt_s32 vdp_DebugSaveYUVImg(MT_DRV_VIDEO_FRAME_S *pstCurFrame, mt_char *buffer, mt_u32 count)
{
    struct file *pfYUV;
    int i,j;
    mt_s32 nRet = MT_SUCCESS;
    mt_u32 u32TimeMs = 0;

   // (mt_void)MT_DRV_SYS_GetTimeStampMs(&u32TimeMs);

    i = 0;
    j = 0;

    for(; i < count; i++)
    {
        if(j==0 && buffer[i]==' ')continue;
        if(buffer[i] > ' ')u8VdpDebugStr[j++] = buffer[i];
        if(j>0 && buffer[i]<=' ')break;
    }

    u8VdpDebugStr[j] = 0;

    if (u8VdpDebugStr[0] == '/')
    {
        if(u8VdpDebugStr[j-1] == '/')
        {
            u8VdpDebugStr[j-1] = 0;
        }
        MT_INFO_VO("******* VDP save path: %s ********\n", u8VdpDebugStr);
        mt_osal_strncpy(s_VdpSavePath, u8VdpDebugStr, j);
    }

    // open file in kernel
    mt_osal_snprintf(u8VdpDebugStr, 256, "%s/vdp_0x%x_%d_%d_time_%d.yuv", s_VdpSavePath,
                                          pstCurFrame->stBufAddr[0].u32PhyAddr_Y,
                                          pstCurFrame->u32Width,
                                          pstCurFrame->u32Height,
                                          u32TimeMs);

    pfYUV = vdp_k_fopen(u8VdpDebugStr, O_RDWR|O_CREAT|O_APPEND, 0);

    if (pfYUV)
    {
        // save yuv data
        nRet = vdp_k_SaveYUVImg(pfYUV, pstCurFrame, 1);
        vdp_k_fclose(pfYUV);
        MT_PRINT("save image '%s' = 0x%x\n", u8VdpDebugStr, nRet);
    }
    else
    {
        MT_ERR_WIN("open file '%s' fail!\n", u8VdpDebugStr);
        nRet = MT_FAILURE;
    }

    return nRet;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */


