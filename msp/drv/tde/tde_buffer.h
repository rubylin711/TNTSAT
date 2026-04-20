/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef  __MT_TDE_BUFFER_H__
#define  __MT_TDE_BUFFER_H__

#include "tde_define.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* End of #ifdef __cplusplus */


STATIC phys_addr_t s_u32TDEPhyBuff = 0;
STATIC mt_u32 s_u32TDEBuffRef = 0;
#ifndef TDE_BOOT
static spinlock_t s_TDEBuffLock;
static mt_size_t s_TDEBuffLockFlags;
#endif
STATIC mt_u32  TDE_AllocPhysicBuff(mt_u32 u32CbCrOffset)
{
    phys_addr_t u32PhyAddr;
    mt_u32 u32CscBufferSize;

    #ifdef CFG_MT_TDE_CSCTMPBUFFER_SIZE
    u32CscBufferSize = CFG_MT_TDE_CSCTMPBUFFER_SIZE;
    #else
    u32CscBufferSize = g_u32TdeTmpBuf;
    #endif
    TDE_LOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
    if (0 == s_u32TDEPhyBuff)
    {
        if((u32CbCrOffset*3) > u32CscBufferSize)
        {
            TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
            return 0;
        }

        TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
        
        u32PhyAddr = MT_GFX_AllocMem("TDE_TEMP_BUFFER",NULL,u32CscBufferSize);
        if(0 == u32PhyAddr)
        {
            return 0;
        }
        
        TDE_LOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
        if (0 == s_u32TDEPhyBuff)
        {
            s_u32TDEPhyBuff = u32PhyAddr;
            s_u32TDEBuffRef = 0;
        }
        else
        {
            s_u32TDEBuffRef++;
            TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
            MT_GFX_FreeMem(u32PhyAddr);
            return s_u32TDEPhyBuff + u32CbCrOffset;
        }
        
    }

    s_u32TDEBuffRef++;
    TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);

    return s_u32TDEPhyBuff + u32CbCrOffset;
}

STATIC mt_void TDE_FreePhysicBuff(mt_void)
{
    TDE_LOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
    if (0 == s_u32TDEBuffRef)
    {
        TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
        return;
    }

    s_u32TDEBuffRef--;
    if (0 == s_u32TDEBuffRef)
    {
        mt_u32 u32PhyBuff = s_u32TDEPhyBuff;
        s_u32TDEPhyBuff = 0;
        TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
        MT_GFX_FreeMem(u32PhyBuff);
        return;
    }
    TDE_UNLOCK(&s_TDEBuffLock, s_TDEBuffLockFlags);
}

#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* End of #ifdef __cplusplus */

#endif /* __MT_TDE_BUFFER_H__ */


