/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "vpss_osal.h"
#include <linux/wait.h>
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

/************************************************************************/
/* file operation                                                       */
/************************************************************************/

struct file *VPSS_OSAL_fopen(const char *filename, int flags, int mode)
{
        struct file *filp = filp_open(filename, flags, mode);
        return (IS_ERR(filp)) ? NULL : filp;
}

void VPSS_OSAL_fclose(struct file *filp)
{
        if (filp)
            filp_close(filp, NULL);
}

int VPSS_OSAL_fread(char *buf, unsigned int len, struct file *filp)
{
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
}

int VPSS_OSAL_fwrite(char *buf, int len, struct file *filp)
{
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
}


/************************************************************************/
/* event operation                                                      */
/************************************************************************/
mt_s32 VPSS_OSAL_InitEvent( OSAL_EVENT *pEvent, mt_s32 InitVal1, mt_s32 InitVal2)
{
    pEvent->flag_1 = InitVal1;
    pEvent->flag_2 = InitVal2;
    init_waitqueue_head( &(pEvent->queue_head) );
    return OSAL_OK;
}

mt_s32 VPSS_OSAL_GiveEvent( OSAL_EVENT *pEvent, mt_s32 InitVal1, mt_s32 InitVal2)
{
    pEvent->flag_1 = InitVal1;
    pEvent->flag_2 = InitVal2;

    wake_up(&(pEvent->queue_head));
	return OSAL_OK;
}

mt_s32 VPSS_OSAL_WaitEvent( OSAL_EVENT *pEvent, ktime_t s32WaitTime )
{
	int l_ret;
    long unsigned int time;
    time = jiffies;
    l_ret = wait_event_hrtimeout( pEvent->queue_head,
                                (pEvent->flag_1 != 0 || pEvent->flag_2 != 0),
                                s32WaitTime );
    if(l_ret == 0
       || pEvent->flag_2 == 1
       || l_ret < 0)
    {
        return OSAL_ERR;
    }
    else
    {
        return OSAL_OK;
    }
}

mt_s32 VPSS_OSAL_ResetEvent( OSAL_EVENT *pEvent, mt_s32 InitVal1, mt_s32 InitVal2)
{
    pEvent->flag_1 = InitVal1;
    pEvent->flag_2 = InitVal2;

    return OSAL_OK;
}


/************************************************************************/
/* mutux lock operation                                                 */
/************************************************************************/
mt_s32 VPSS_OSAL_InitLOCK(VPSS_OSAL_LOCK *pLock, mt_u32 u32InitVal)
{
    sema_init(pLock,u32InitVal);
    return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_DownLock(VPSS_OSAL_LOCK *pLock)
{
    mt_s32 s32Ret;
    s32Ret = down_interruptible(pLock);

    if (s32Ret < 0)
    {
		return MT_FAILURE;
	}
    else if (s32Ret == 0)
    {
        return MT_SUCCESS;
    }
    else
    {
        VPSS_FATAL("DownLock Error! ret = %d\n",s32Ret);
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_TryLock(VPSS_OSAL_LOCK *pLock)
{
    mt_s32 s32Ret;
    s32Ret = down_trylock(pLock);
    if (s32Ret == 0)
    {
        return MT_SUCCESS;
    }
    else
    {
        return MT_FAILURE;
    }
}

mt_s32 VPSS_OSAL_UpLock(VPSS_OSAL_LOCK *pLock)
{
    up(pLock);
    return MT_SUCCESS;
}



/************************************************************************/
/* spin lock operation                                                  */
/************************************************************************/
mt_s32 VPSS_OSAL_InitSpin(VPSS_OSAL_SPIN *pLock)
{
    spin_lock_init(pLock);
	return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_DownSpin(VPSS_OSAL_SPIN *pLock,unsigned long *flags)
{
    spin_lock_irqsave(pLock, *flags);

    return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_UpSpin(VPSS_OSAL_SPIN *pLock,unsigned long *flags)
{
    spin_unlock_irqrestore(pLock, *flags);

    return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_TryLockSpin(VPSS_OSAL_SPIN *pLock,unsigned long *flags)
{
   if(spin_trylock_irqsave(pLock, *flags))
   {
        return MT_SUCCESS;
   }
   else
   {
        return MT_FAILURE;
   }
}

/************************************************************************/
/* debug operation                                                      */
/************************************************************************/
mt_s32 VPSS_OSAL_GetProcArg(mt_char*  chCmd,mt_char*  chArg,mt_u32 u32ArgIdx)
{
    mt_u32 u32Count;
    mt_u32 u32CmdCount;
    mt_u32 u32LogCount;
    mt_u32 u32NewFlag;
    mt_char chArg1[DEF_FILE_NAMELENGTH] = {0};
    mt_char chArg2[DEF_FILE_NAMELENGTH] = {0};
    mt_char chArg3[DEF_FILE_NAMELENGTH] = {0};
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

    switch(u32ArgIdx)
    {
        case 1:
            memcpy(chArg,chArg1,sizeof(mt_char)*DEF_FILE_NAMELENGTH);
            break;
        case 2:
            memcpy(chArg,chArg2,sizeof(mt_char)*DEF_FILE_NAMELENGTH);
            break;
        case 3:
            memcpy(chArg,chArg3,sizeof(mt_char)*DEF_FILE_NAMELENGTH);
            break;
        default:
            break;
    }
    return MT_SUCCESS;
}


mt_s32 VPSS_OSAL_ParseCmd(mt_char*  chArg1,mt_char*  chArg2,mt_char*  chArg3,mt_void *pstCmd)
{
    return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_TransforUV10bitTobit(mt_u8 *pu10UVdata,mt_u8 *pu8Udata,mt_u8 *pu8Vdata,mt_u32 u32Stride,mt_u32 u32Width,MT_DRV_PIX_FORMAT_E eFormat)
{
    mt_u8 *pTmp;
    mt_u32 i,j,tmpU,tmpV;
    pTmp = VPSS_VMALLOC(u32Stride*8);
    if (pTmp == MT_NULL)
    {
        return MT_FAILURE;
    }
    for(i = 0; i < u32Stride; i++)
    {
        for(j=0; j < 8; j++)
        {
            pTmp[i*8 +j] = (pu10UVdata[i]>>j) & 0x1;
        }
    }
    for(i = 0; i < u32Width/2; i = i+2)
    {
        tmpU = 0;
        for(j=0; j < 8; j++)
        {
            tmpU |= pTmp[i*10+j+2]<<j;
        }
        tmpV = 0;
        for(j=0; j < 8; j++)
        {
            tmpV |= pTmp[(i+1)*10+j+2]<<j;
        }
        if((eFormat == MT_DRV_PIX_FMT_NV21) || (eFormat == MT_DRV_PIX_FMT_NV61_2X1) )
        {
            pu8Vdata[i] = tmpU;
            pu8Udata[i] = tmpV;
        }
        else
        {
            pu8Udata[i] = tmpU;
            pu8Vdata[i] = tmpV;
        }
    }
    VPSS_VFREE(pTmp);
    return MT_SUCCESS;
}
mt_s32 VPSS_OSAL_Transfor10bitTobit(mt_u8 *pu10Ydata,mt_u8 *pu8Ydata,mt_u32 u32Stride,mt_u32 u32Width)
{
    mt_u8 *pTmp;
    mt_u32 i,j,tmp;
    pTmp = VPSS_VMALLOC(u32Stride*8);
    if (pTmp == MT_NULL)
    {
        return MT_FAILURE;
    }
    for(i = 0; i < u32Stride; i++)
    {
        for(j=0; j < 8; j++)
        {
            pTmp[i*8 +j] = (pu10Ydata[i]>>j) & 0x1;
        }
    }
    for(i = 0; i < u32Width; i++)
    {
        tmp = 0;
        for(j=0; j < 8; j++)
        {
            tmp |= pTmp[i*10+j+2]<<j;
        }
        pu8Ydata[i] = tmp;
    }
    VPSS_VFREE(pTmp);
    return MT_SUCCESS;
}

mt_u8  temp[4096]={0};
static inline mt_void VPSS_OSAL_OneLine10To8Bit(int enType, mt_char *pInAddr, mt_u32 u32Width, mt_char *pOutAddr)
{
    mt_u32 i, j, u32Cnt;
    mt_char  *pTmpMem = NULL;

    pTmpMem = pInAddr;
    u32Cnt = HICEILING(u32Width, 4);//四个像素一循环，占5 byte,不足四个像素也占5byte处理
    for (i = 0; i < u32Cnt; i++)
    {
        for(j = 0; j < 5; j++)
        {
             temp[j] = *(pTmpMem + i*5 + j);
        }

        if (0 == enType)
        {
            *pOutAddr = ((temp[1] << 6) & 0xc0) | ((temp[0] >> 2)& 0x3f);
            pOutAddr++;
            *pOutAddr = ((temp[2] << 4) & 0xf0) | ((temp[1] >> 4)& 0x0f);
            pOutAddr++;
            *pOutAddr = ((temp[3] << 2) & 0xfc) | ((temp[2] >> 6)& 0x03);
            pOutAddr++;
            *pOutAddr = temp[4] & 0xff;
            pOutAddr++;

        }
        else if (1 == enType)
        {
            *pOutAddr = ((temp[2] << 4) & 0xf0) | ((temp[1] >> 4)& 0x0f);
			pOutAddr++;
			*pOutAddr = temp[4] & 0xff;
			pOutAddr++;
        }
        else
        {
            *pOutAddr = ((temp[1] << 6) & 0xc0) | ((temp[0] >> 2)& 0x3f);
			pOutAddr++;
			*pOutAddr = ((temp[3] << 2) & 0xfc) | ((temp[2] >> 6)& 0x03);
			pOutAddr++;
        }
    }

}

mt_s32 VPSS_OSAL_StrToNumb(mt_char*  chStr,mt_u32 *pu32Numb)
{
    mt_u32 u32Count = 0;
    mt_u32 u32RetNumb = 0;

    while(chStr[u32Count] != 0)
    {
        u32RetNumb = u32RetNumb*10 + chStr[u32Count] - '0';
        u32Count++;
    }

    *pu32Numb = u32RetNumb;

    return MT_SUCCESS;
}
mt_s32 VPSS_OSAL_WRITEYUV_10BIT(MT_DRV_VIDEO_FRAME_S *pstFrame,mt_char* pchFile)
{
    char str[50] = {0};
    unsigned char *ptr;
    FILE *fp;
    mt_u8 *pu8Udata;
    mt_u8 *pu8Vdata;
    mt_u8 *pu8Ydata;
    mt_u8 *pu10UVdata;
    mt_u8 *pu10Ydata;
    mt_s8  s_VpssSavePath[DEF_FILE_NAMELENGTH];
    mt_u32 i;
    mt_drv_log_get_storepath(s_VpssSavePath, DEF_FILE_NAMELENGTH);
    mt_osal_snprintf(str, 50, "%s/%s", s_VpssSavePath,pchFile);

    if (pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV21
        || pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV12)
    {
        pu8Udata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 /2);
        if (pu8Udata == MT_NULL)
        {
            return MT_FAILURE;
        }

        pu10UVdata = VPSS_VMALLOC(pstFrame->stBufAddr[0].u32Stride_C);
        if (pu10UVdata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            return MT_FAILURE;
        }

        pu8Vdata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 /2);
        if (pu8Vdata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu10UVdata);
            return MT_FAILURE;
        }
        pu8Ydata = VPSS_VMALLOC(pstFrame->u32Width);
        if (pu8Ydata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu8Vdata);
            VPSS_VFREE(pu10UVdata);
            return MT_FAILURE;
        }
        pu10Ydata = VPSS_VMALLOC(pstFrame->stBufAddr[0].u32Stride_Y);
        if (pu10Ydata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu8Vdata);
            VPSS_VFREE(pu10UVdata);
            VPSS_VFREE(pu8Ydata);
            return MT_FAILURE;
        }
        ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y);
        if (!ptr)
        {
            VPSS_FATAL("address is not valid!\n");
        }
        else
        {
            fp = VPSS_OSAL_fopen(str, O_RDWR | O_CREAT|O_APPEND, 0);
            if (fp == MT_NULL)
            {
                VPSS_FATAL("open file '%s' fail!\n", str);
                VPSS_VFREE(pu8Udata);
                VPSS_VFREE(pu8Vdata);
                VPSS_VFREE(pu8Ydata);
                VPSS_VFREE(pu10UVdata);
                VPSS_VFREE(pu10Ydata);
                return MT_FAILURE;
            }
            for (i=0; i<pstFrame->u32Height; i++)
            {
                memcpy(pu10Ydata,ptr,sizeof(mt_u8)*pstFrame->stBufAddr[0].u32Stride_Y);
                VPSS_OSAL_Transfor10bitTobit(pu10Ydata,pu8Ydata,pstFrame->stBufAddr[0].u32Stride_Y,pstFrame->u32Width);
                if(pstFrame->u32Width != VPSS_OSAL_fwrite(pu8Ydata,pstFrame->u32Width, fp))
                {
                    VPSS_FATAL("line %d: fwrite fail!\n",__LINE__);
                }
                ptr += pstFrame->stBufAddr[0].u32Stride_Y;
            }
            ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_C);
            for (i=0; i<pstFrame->u32Height/2; i++)
            {
                memcpy(pu10UVdata,ptr,pstFrame->stBufAddr[0].u32Stride_C);

                VPSS_OSAL_OneLine10To8Bit(1,pu10UVdata,pstFrame->u32Width, pu8Udata+i*pstFrame->u32Width/2);

                VPSS_OSAL_OneLine10To8Bit(2,pu10UVdata,pstFrame->u32Width, pu8Vdata+i*pstFrame->u32Width/2);

                //VPSS_OSAL_TransforUV10bitTobit(pu10UVdata,&pu8Udata[i*pstFrame->u32Width/2],&pu8Vdata[i*pstFrame->u32Width/2],
                //    pstFrame->stBufAddr[0].u32Stride_C,pstFrame->u32Width,pstFrame->ePixFormat);
                ptr += pstFrame->stBufAddr[0].u32Stride_C;
            }
            VPSS_OSAL_fwrite(pu8Udata, pstFrame->u32Width * pstFrame->u32Height / 2 /2, fp);
            VPSS_OSAL_fwrite(pu8Vdata, pstFrame->u32Width * pstFrame->u32Height / 2 /2, fp);
            VPSS_OSAL_fclose(fp);
            VPSS_FATAL("2d image has been saved to '%s' W=%d H=%d Format=%d \n",
                        str,pstFrame->u32Width,pstFrame->u32Height,pstFrame->ePixFormat);
        }
        VPSS_VFREE(pu8Udata);
        VPSS_VFREE(pu8Vdata);
        VPSS_VFREE(pu8Ydata);
        VPSS_VFREE(pu10UVdata);
        VPSS_VFREE(pu10Ydata);
    }
    else if (pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1
            || pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV16_2X1)
    {
        pu8Udata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 );
        if (pu8Udata == MT_NULL)
        {
            return MT_FAILURE;
        }

        pu10UVdata = VPSS_VMALLOC(pstFrame->stBufAddr[0].u32Stride_C);
        if (pu10UVdata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            return MT_FAILURE;
        }

        pu8Vdata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 );
        if (pu8Vdata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu10UVdata);
            return MT_FAILURE;
        }
        pu8Ydata = VPSS_VMALLOC(pstFrame->u32Width);
        if (pu8Ydata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu8Vdata);
            VPSS_VFREE(pu10UVdata);
            return MT_FAILURE;
        }
        pu10Ydata = VPSS_VMALLOC(pstFrame->stBufAddr[0].u32Stride_Y);
        if (pu10Ydata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu8Vdata);
            VPSS_VFREE(pu10UVdata);
            VPSS_VFREE(pu8Ydata);
            return MT_FAILURE;
        }
        ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y);
        if (!ptr)
        {
            VPSS_FATAL("address is not valid!\n");
        }
        else
        {
            fp = VPSS_OSAL_fopen(str, O_RDWR | O_CREAT|O_APPEND, 0);
            if (fp == MT_NULL)
            {
                VPSS_FATAL("open file '%s' fail!\n", str);
                VPSS_VFREE(pu8Udata);
                VPSS_VFREE(pu8Vdata);
                VPSS_VFREE(pu8Ydata);
                VPSS_VFREE(pu10UVdata);
                VPSS_VFREE(pu10Ydata);
                return MT_FAILURE;
            }
            for (i=0; i<pstFrame->u32Height; i++)
            {
                memcpy(pu10Ydata,ptr,sizeof(mt_u8)*pstFrame->stBufAddr[0].u32Stride_Y);
                VPSS_OSAL_Transfor10bitTobit(pu10Ydata,pu8Ydata,pstFrame->stBufAddr[0].u32Stride_Y,pstFrame->u32Width);
                if(pstFrame->u32Width != VPSS_OSAL_fwrite(pu8Ydata,pstFrame->u32Width, fp))
                {
                    VPSS_FATAL("line %d: fwrite fail!\n",__LINE__);
                }
                ptr += pstFrame->stBufAddr[0].u32Stride_Y;
            }

            ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_C);
            for (i=0; i<pstFrame->u32Height; i++)
            {
                memcpy(pu10UVdata,ptr,pstFrame->stBufAddr[0].u32Stride_C);

                VPSS_OSAL_OneLine10To8Bit(1,pu10UVdata,pstFrame->u32Width, pu8Udata+i*pstFrame->u32Width/2);

                VPSS_OSAL_OneLine10To8Bit(2,pu10UVdata,pstFrame->u32Width, pu8Vdata+i*pstFrame->u32Width/2);

//                VPSS_OSAL_TransforUV10bitTobit(pu10UVdata,&pu8Udata[i*pstFrame->u32Width/2],&pu8Vdata[i*pstFrame->u32Width/2],
   //                 pstFrame->stBufAddr[0].u32Stride_C,pstFrame->u32Width,pstFrame->ePixFormat);
                ptr += pstFrame->stBufAddr[0].u32Stride_C;
            }
            VPSS_OSAL_fwrite(pu8Udata, pstFrame->u32Width * pstFrame->u32Height  /2, fp);
            VPSS_OSAL_fwrite(pu8Vdata, pstFrame->u32Width * pstFrame->u32Height  /2, fp);
            VPSS_OSAL_fclose(fp);
            VPSS_FATAL("2d image has been saved to '%s' W=%d H=%d Format=%d \n",
                        str,pstFrame->u32Width,pstFrame->u32Height,pstFrame->ePixFormat);
        }
        VPSS_VFREE(pu8Udata);
        VPSS_VFREE(pu8Vdata);
        VPSS_VFREE(pu8Ydata);
        VPSS_VFREE(pu10UVdata);
        VPSS_VFREE(pu10Ydata);
    }
    else
    {
        VPSS_FATAL("PixFormat %d can't saveyuv\n",pstFrame->ePixFormat);
    }
    return MT_SUCCESS;
}
mt_s32 VPSS_OSAL_WRITEYUV_8BIT(MT_DRV_VIDEO_FRAME_S *pstFrame,mt_char* pchFile)
{
	char str[50] = {0};
	unsigned char *ptr;
	FILE *fp;
    mt_u8 *pu8Udata;
    mt_u8 *pu8Vdata;
    mt_u8 *pu8Ydata;
    mt_s8  s_VpssSavePath[DEF_FILE_NAMELENGTH];
    mt_u32 i,j;

    mt_drv_log_get_storepath(s_VpssSavePath, DEF_FILE_NAMELENGTH);
    mt_osal_snprintf(str, 50, "%s/%s", s_VpssSavePath,pchFile);

    if (pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV21
        || pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV12)
    {
        pu8Udata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 /2);
        if (pu8Udata == MT_NULL)
        {
            return MT_FAILURE;
        }
        pu8Vdata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 /2);
        if (pu8Vdata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            return MT_FAILURE;
        }
        pu8Ydata = VPSS_VMALLOC(pstFrame->stBufAddr[0].u32Stride_Y);
        if (pu8Ydata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu8Vdata);
            return MT_FAILURE;
        }

        ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y);

		if (!ptr)
		{
            VPSS_FATAL("address is not valid!\n");
		}
		else
		{
            fp = VPSS_OSAL_fopen(str, O_RDWR | O_CREAT|O_APPEND, 0644);

            if (fp == MT_NULL)
            {
                VPSS_FATAL("open file '%s' fail!\n", str);
                VPSS_VFREE(pu8Udata);
                VPSS_VFREE(pu8Vdata);
                VPSS_VFREE(pu8Ydata);
                return MT_FAILURE;
            }

            /*write Y data*/
            for (i=0; i<pstFrame->u32Height; i++)
            {
                memcpy(pu8Ydata,ptr,sizeof(mt_u8)*pstFrame->stBufAddr[0].u32Stride_Y);

                if(pstFrame->u32Width != VPSS_OSAL_fwrite(pu8Ydata,pstFrame->u32Width, fp))
				{
                    VPSS_FATAL("line %d: fwrite fail!\n",__LINE__);
                }
                ptr += pstFrame->stBufAddr[0].u32Stride_Y;
            }

            ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_C);
            /*write UV data */
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
            VPSS_OSAL_fwrite(pu8Udata, pstFrame->u32Width * pstFrame->u32Height / 2 /2, fp);

            VPSS_OSAL_fwrite(pu8Vdata, pstFrame->u32Width * pstFrame->u32Height / 2 /2, fp);


            VPSS_OSAL_fclose(fp);
            VPSS_FATAL("2d image has been saved to '%s' W=%d H=%d Format=%d \n",
                        str,pstFrame->u32Width,pstFrame->u32Height,pstFrame->ePixFormat);


		}
        VPSS_VFREE(pu8Udata);
        VPSS_VFREE(pu8Vdata);
        VPSS_VFREE(pu8Ydata);
    }
    else if (pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV21_TILE
            || pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV12_TILE)
    {
        mt_u8 *s8YuvArray;
        mt_u8 *s8UArray;
        mt_u8 *s8VArray;
        mt_u8 *dst;
        mt_u8 *src;
        mt_u8 *tmp;
        mt_u32 Stride;
        mt_u8 *Caddress ;

        s8YuvArray = VPSS_VMALLOC(((pstFrame->u32Width+16)&0xfffffff0) * pstFrame->u32Height);
        if (MT_NULL == s8YuvArray)
        {
			return MT_FAILURE;
        }

		s8UArray = VPSS_VMALLOC(((pstFrame->u32Width+16)&0xfffffff0) * pstFrame->u32Height / 2);
        if (MT_NULL == s8UArray)
        {
            VPSS_VFREE(s8YuvArray);
			return MT_FAILURE;
        }

		s8VArray = VPSS_VMALLOC(((pstFrame->u32Width+16)&0xfffffff0) * pstFrame->u32Height / 2);
        if (MT_NULL == s8VArray)
        {
            VPSS_VFREE(s8YuvArray);
            VPSS_VFREE(s8UArray);
			return MT_FAILURE;
        }

        fp = VPSS_OSAL_fopen(str, O_RDWR | O_CREAT|O_APPEND, 0644);
        if (fp == MT_NULL)
        {
            VPSS_FATAL("open file '%s' fail!\n", str);
            VPSS_VFREE(s8YuvArray);
            VPSS_VFREE(s8UArray);
            VPSS_VFREE(s8VArray);
            return MT_FAILURE;
        }

        Stride = ((pstFrame->u32Width + (64*4 -1))&(~(64*4 -1)))*16;
        for(i=0;i<pstFrame->u32Height;i++)
        {
            for(j=0;j<pstFrame->u32Width;j+=256)
            {
                dst  = (unsigned char*)(s8YuvArray+ pstFrame->u32Width*i + j);
                src =  (unsigned char*)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y) + Stride*(i/16)+(i%16)*256 + (j/256)*256*16;
				memcpy(dst,src,256);
            }
        }

        VPSS_OSAL_fwrite(s8YuvArray,pstFrame->u32Width*pstFrame->u32Height, fp);

        Caddress = phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y) + ((pstFrame->u32Width + (64*4 -1))&(~(64*4 -1)))*((pstFrame->u32Height + 31)/32)*32
                + ((pstFrame->u32Width  + 127) / 128 + 15) / 16 * 16*((pstFrame->u32Height + 31)/32)*32;

        for(i=0;i<pstFrame->u32Height/2;i++)
        {
            for(j=0;j<pstFrame->u32Width;j+=256)
            {
               dst  = (unsigned char*)(s8YuvArray + pstFrame->u32Width*i + j);
               src =  (unsigned char*)Caddress + (Stride/2)*(i/8)+(i%8)*256 +  (j/256)*256*8;
			   memcpy(dst,src,256);
            }
        }

        tmp = s8YuvArray;
        for (i=0;i<pstFrame->u32Height/2;i++)
        {
            for (j=0;j<pstFrame->u32Width/2;j++)
            {
                s8VArray[i*pstFrame->u32Width/2+j] = tmp[2*j];
                s8UArray[i*pstFrame->u32Width/2+j] = tmp[2*j+1];
            }
            tmp+= pstFrame->u32Width;
        }

        VPSS_OSAL_fwrite(s8UArray,pstFrame->u32Width*pstFrame->u32Height/4, fp);
        VPSS_OSAL_fwrite(s8VArray,pstFrame->u32Width*pstFrame->u32Height/4, fp);

		VPSS_OSAL_fclose(fp);
        VPSS_FATAL("Tile image has been saved to '%s' W=%d H=%d Format=%d \n",
                    str,pstFrame->u32Width,pstFrame->u32Height,pstFrame->ePixFormat);
        VPSS_VFREE(s8YuvArray);
        VPSS_VFREE(s8UArray);
        VPSS_VFREE(s8VArray);
	}
	else if (pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1
            || pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV16_2X1)
    {
        pu8Udata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 );
        if (pu8Udata == MT_NULL)
        {
            return MT_FAILURE;
        }
        pu8Vdata = VPSS_VMALLOC(pstFrame->u32Width * pstFrame->u32Height / 2 );
        if (pu8Vdata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            return MT_FAILURE;
        }
        pu8Ydata = VPSS_VMALLOC(pstFrame->stBufAddr[0].u32Stride_Y);
        if (pu8Ydata == MT_NULL)
        {
            VPSS_VFREE(pu8Udata);
            VPSS_VFREE(pu8Vdata);
            return MT_FAILURE;
        }

        ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_Y);

		if (!ptr)
		{
            VPSS_FATAL("address is not valid!\n");
		}
		else
		{

            fp = VPSS_OSAL_fopen(str, O_RDWR | O_CREAT|O_APPEND, 0644);

            if (fp == MT_NULL)
            {
                VPSS_FATAL("open file '%s' fail!\n", str);
                VPSS_VFREE(pu8Udata);
                VPSS_VFREE(pu8Vdata);
                VPSS_VFREE(pu8Ydata);
                return MT_FAILURE;
            }

            /*write Y data*/
            for (i=0; i<pstFrame->u32Height; i++)
            {
                memcpy(pu8Ydata,ptr,sizeof(mt_u8)*pstFrame->stBufAddr[0].u32Stride_Y);

                if(pstFrame->u32Width != VPSS_OSAL_fwrite(pu8Ydata,pstFrame->u32Width, fp))
				{
                    VPSS_FATAL("line %d: fwrite fail!\n",__LINE__);
                }
                ptr += pstFrame->stBufAddr[0].u32Stride_Y;
            }

            ptr = (unsigned char *)phys_to_virt(pstFrame->stBufAddr[0].u32PhyAddr_C);
            /*write UV data */
            for (i=0; i<pstFrame->u32Height; i++)
            {
                for (j=0; j<pstFrame->u32Width/2; j++)
                {
                    if(pstFrame->ePixFormat == MT_DRV_PIX_FMT_NV61_2X1)
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
            VPSS_OSAL_fwrite(pu8Udata, pstFrame->u32Width * pstFrame->u32Height  /2, fp);

            VPSS_OSAL_fwrite(pu8Vdata, pstFrame->u32Width * pstFrame->u32Height  /2, fp);


            VPSS_OSAL_fclose(fp);
            VPSS_FATAL("2d image has been saved to '%s' W=%d H=%d Format=%d \n",
                        str,pstFrame->u32Width,pstFrame->u32Height,pstFrame->ePixFormat);

		}
        VPSS_VFREE(pu8Udata);
        VPSS_VFREE(pu8Vdata);
        VPSS_VFREE(pu8Ydata);
    }
    else
    {
        VPSS_FATAL("PixFormat %d can't saveyuv\n",pstFrame->ePixFormat);
    }

    return MT_SUCCESS;
}
mt_s32 VPSS_OSAL_WRITEYUV(MT_DRV_VIDEO_FRAME_S *pstFrame,mt_char* pchFile)
{
    if(MT_DRV_PIXEL_BITWIDTH_8BIT == pstFrame->enBitWidth)
    {
        return VPSS_OSAL_WRITEYUV_8BIT(pstFrame,pchFile);
    }
    else
    if(MT_DRV_PIXEL_BITWIDTH_10BIT == pstFrame->enBitWidth)
    {
        return VPSS_OSAL_WRITEYUV_10BIT(pstFrame,pchFile);
    }
    else
    {
        return MT_FAILURE;
    }
}
mt_s32 VPSS_OSAL_CalBufSize(mt_u32 *pSize,mt_u32 *pStride,mt_u32 u32Height,mt_u32 u32Width,MT_DRV_PIX_FORMAT_E ePixFormat,MT_DRV_PIXEL_BITWIDTH_E  enOutBitWidth)
{
    mt_u32 u32RetSize = 0;
    mt_u32 u32RetStride = 0;

    //:TODO:默认按10bit紧凑分配
    switch (ePixFormat)
    {
        case MT_DRV_PIX_FMT_NV12:
        case MT_DRV_PIX_FMT_NV21:
            if(MT_DRV_PIXEL_BITWIDTH_8BIT == enOutBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
            }
            else
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
            }
            u32RetSize = u32Height * u32RetStride*3/2;
            break;
        case MT_DRV_PIX_FMT_NV16_2X1:
        case MT_DRV_PIX_FMT_NV61_2X1:
            if(MT_DRV_PIXEL_BITWIDTH_8BIT == enOutBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
            }
            else
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
            }
            u32RetSize = u32Height * u32RetStride*2;
            break;
        case MT_DRV_PIX_FMT_NV12_CMP:
        case MT_DRV_PIX_FMT_NV21_CMP:
            if(MT_DRV_PIXEL_BITWIDTH_8BIT == enOutBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
            }
            else
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
            }
            u32RetSize = u32Height * u32RetStride*3/2 + 16 * u32Height*3/2;
            break;
        case MT_DRV_PIX_FMT_NV16_CMP:
        case MT_DRV_PIX_FMT_NV61_CMP:
            if(MT_DRV_PIXEL_BITWIDTH_8BIT == enOutBitWidth)
            {
                u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width);
            }
            else
            {
                u32RetStride = MT_ALIGN_10BIT_COMP_YSTRIDE(u32Width);
            }
            u32RetSize = u32Height * u32RetStride*3/2 + 16 * u32Height*2;
            break;
        case MT_DRV_PIX_FMT_ARGB8888:
        case MT_DRV_PIX_FMT_ABGR8888:
            u32RetStride = MT_ALIGN_8BIT_YSTRIDE(u32Width*4);
            u32RetSize = u32Height * u32RetStride;
            break;
        default:
            VPSS_FATAL("Unsupport PixFormat %d.\n",ePixFormat);
            return MT_FAILURE;
    }

    //*pSize = u32RetSize;
    //*pStride = u32RetStride;
    *pSize = 1;//yihua and Rock Hu
    *pStride = 1;
    return MT_SUCCESS;
}


mt_s32 VPSS_OSAL_GetVpssVersion(VPSS_VERSION_E *penVersion)
{
    MT_CHIP_TYPE_E enChipType;
    MT_CHIP_VERSION_E enChipVersion;

    VPSS_CHECK_NULL(penVersion);

    mt_drv_sys_getchipversion(&enChipType, &enChipVersion);

    *penVersion = VPSS_VERSION_V1_0;//Rock_hu得到vpss版本

    return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_GetTileOffsetAddr(mt_u32 u32Xoffset,mt_u32 u32Yoffset,
                                   mt_u32 *pu32Yaddr,mt_u32 *pu32Caddr,
                                   MT_DRV_VID_FRAME_ADDR_S *pstOriAddr)
{
    mt_u32 tile_pix_pos,tile_y_stride,tile_c_stride;
    mt_u32 u32Y_Stride,u32Y_Addr;
    mt_u32 u32C_Stride,u32C_Addr;
    mt_u32 u32C_StartAddr;
    mt_u32 u32Y_StartAddr;

    u32Y_Addr = pstOriAddr->u32PhyAddr_Y;
    u32Y_Stride = pstOriAddr->u32Stride_Y;

    tile_pix_pos = (((u32Xoffset)/256)==(u32Y_Stride/256));
    tile_y_stride = (tile_pix_pos==1)?(((u32Y_Stride%256)==0)?256:(u32Y_Stride%256)):256;
    u32Y_StartAddr =u32Y_Addr+((u32Yoffset)/16)*u32Y_Stride*16+((u32Xoffset)/256)*256*16+((u32Yoffset)%16)*tile_y_stride+(u32Xoffset)%256;

    u32C_Addr = pstOriAddr->u32PhyAddr_C;
    u32C_Stride = pstOriAddr->u32Stride_C;

    tile_pix_pos = (((u32Xoffset)/256)==(u32C_Stride/256));
    tile_c_stride = (tile_pix_pos==1)?(((u32C_Stride%256)==0)?256:(u32C_Stride%256)):256;
    u32C_StartAddr =u32C_Addr+((u32Yoffset/2)/8)*u32C_Stride*8+((u32Xoffset)/256)*256*8+((u32Yoffset/2)%8)*tile_c_stride+(u32Xoffset)%256;

    *pu32Yaddr = u32Y_StartAddr;
    *pu32Caddr = u32C_StartAddr;

    return MT_SUCCESS;
}
mt_s32 VPSS_OSAL_GetCurTime(mt_u32 *pu32Hour,mt_u32 *pu32Minute,
                                   mt_u32 *pu32Second)
{
	struct timeval stCurrentTime;

	do_gettimeofday(&stCurrentTime);

	*pu32Hour = stCurrentTime.tv_sec /3600;
	*pu32Minute = (stCurrentTime.tv_sec - (*pu32Hour *3600))/60;
    *pu32Second = stCurrentTime.tv_sec % 60;

	return MT_SUCCESS;
}

mt_s32 VPSS_OSAL_GetSysMemSize(mt_u32 *pu32MemSize)
{
	mt_s32 s32Ret;
	mt_u32 u32Mem = 0;
    mt_sys_mem_config_s stConfig;

	s32Ret = mt_drv_sys_getmemconfig(&stConfig);

	if (MT_SUCCESS == s32Ret)
	{
		u32Mem = stConfig.u32TotalSize;
	}
	else
	{
		u32Mem = 0;
	}

	*pu32MemSize = u32Mem;

	return s32Ret;
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */
