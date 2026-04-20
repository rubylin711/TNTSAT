/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

//lint -wlib(0)
#include "string.h"

#include "mt_go_comm.h"
#include "mt_go_decoder.h"
#include "mtgo_adp_sys.h"
#include "mtgo_common.h"
#include "mtgo_io.h"

/***************************** Macro Definition ******************************/

/*************************** Structure Definition ****************************/

/** IO device info*/
typedef union mtIO_DEVINFO_U
{
    /**info in case : input source is memory */
    struct
    {
        mt_u32 Position; /**< IO position , offset from file start*/
        mt_char *pAddr;   /**< pointer to memory */
        mt_u32 Length;   /**< length */
    } MemInfo;
} IO_DEVINFO_U;
typedef struct mtIO_INSTANCE_S IO_INSTANCE_S;

struct mtIO_INSTANCE_S
{
    IO_TYPE_E    Type;     /** fs type*/
    IO_DEVINFO_U IoInfo;   /**< IO info */
    mt_s32       (* IoCreate)(IO_INSTANCE_S *pIO, const IO_DESC_S *pDecInfo);
    mt_void      (* IoDestroy)(IO_INSTANCE_S *pIO);
    mt_s32       (* IoRead)(IO_INSTANCE_S *pIO, mt_void *pBuf, mt_u32 BufLen, mt_u32 *pCopyLen);
    mt_s32       (* IoSeek)(IO_INSTANCE_S *pIO, IO_POS_E Position, mt_s32 Offset);
    mt_s32       (* IoGetLength)(const IO_INSTANCE_S *pIO, mt_u32 *pLength);
    mt_s32       (* IoGetPos)(const IO_INSTANCE_S *pIO, mt_u32 *pPos);
    u8 *           (* IoGetPosAddr)(const IO_INSTANCE_S *pIO);
};

/********************** Global Variable declaration **************************/

/******************************* API declaration *****************************/
/**
 \brief create virtual file system in case of memory type
 \param[in] IO_INSTANCE_S *pIO
 \param[in] const IO_DESC_S *pDecInfo
 \param[out] none
 \retval none
 \return none
 */
static mt_s32 Mem_Create(IO_INSTANCE_S *pIO, const IO_DESC_S *pDecInfo)
{
#if 0
    MTGO_ADP_ASSERT(MT_NULL_PTR != pDecInfo->IoInfo.MemInfo.pAddr);
    MTGO_ADP_ASSERT(pDecInfo->IoInfo.MemInfo.Length > 0);
#endif
    if(MT_NULL_PTR == pDecInfo->IoInfo.MemInfo.pAddr || pDecInfo->IoInfo.MemInfo.Length <= 0)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
    pIO->IoInfo.MemInfo.pAddr    = pDecInfo->IoInfo.MemInfo.pAddr;
    pIO->IoInfo.MemInfo.Length   = pDecInfo->IoInfo.MemInfo.Length;
    pIO->IoInfo.MemInfo.Position = 0;
    return MT_SUCCESS;
}

/**
 \brief read data in case of memory type file 
 \param[in] IO_INSTANCE_S *pIO file instance pointer 
 \param[in] mt_void *pBuf  input BUF address 
 \param[in] mt_u32 BufLen  input BUF lengteh 
 \param[in] mt_u32 *pCopyLen read data size 
 \param[out] none
 \retval none
 \return none
 */
static mt_s32 Mem_Read(IO_INSTANCE_S *pIO, mt_void *pBuf, mt_u32 BufLen, mt_u32 *pCopyLen)
{
    mt_u32 Length;
#if 0
    MTGO_ADP_ASSERT(pIO->IoInfo.MemInfo.Length >= pIO->IoInfo.MemInfo.Position);
#endif
    if(pIO->IoInfo.MemInfo.Length < pIO->IoInfo.MemInfo.Position)
    {
        MTGO_ERROR(MT_FAILURE);
        return MT_FAILURE;
    }
    /** calculate remainder data size */
    Length = (mt_u32)(pIO->IoInfo.MemInfo.Length - pIO->IoInfo.MemInfo.Position);
    if (BufLen < Length)
    {
        *pCopyLen = BufLen;
    }
    else
    {
        *pCopyLen = Length;
    }
    
#ifdef MT_MINIBOOT_SUPPORT
    //mmu_cache_enable();
#else
    //mm_dcache_enable(0);
#endif

    memcpy(pBuf, \
           (const mt_void*)(pIO->IoInfo.MemInfo.pAddr + pIO->IoInfo.MemInfo.Position), *pCopyLen);

#ifdef MT_MINIBOOT_SUPPORT
    //mmu_cache_disable();
#else
    //dcache_disable();
#endif

    pIO->IoInfo.MemInfo.Position += *pCopyLen;
    return MT_SUCCESS;
}

/**
 \brief  file size 
 \param[in] IO_INSTANCE_S *pIO file instance pointer
 \param[in] IO_POS_E Position  
 \param[in] mt_s32 Offset     seek offset 
 \param[out] none
 \retval none
 \return none
 */
static mt_s32 Mem_Seek(IO_INSTANCE_S *pIO, IO_POS_E Position, mt_s32 Offset)
{
    mt_u32 StartPos;
    StartPos = pIO->IoInfo.MemInfo.Position;
    
    switch (Position)
    {
    case IO_POS_CUR:
    {
        pIO->IoInfo.MemInfo.Position = (mt_u32)((mt_s32)pIO->IoInfo.MemInfo.Position + Offset);
        break;
    }
    case IO_POS_END:
    {
        pIO->IoInfo.MemInfo.Position = pIO->IoInfo.MemInfo.Length;
        pIO->IoInfo.MemInfo.Position = (mt_u32)((mt_s32)pIO->IoInfo.MemInfo.Position + Offset);
        break;
    }
    case IO_POS_SET:
    {
        pIO->IoInfo.MemInfo.Position = 0;
        pIO->IoInfo.MemInfo.Position = (mt_u32)((mt_s32)pIO->IoInfo.MemInfo.Position + Offset);
        break;
    }
    default:
    {
        MTGO_ERROR(MT_FAILURE);
        break;
    }
    }

    if ((mt_s32)pIO->IoInfo.MemInfo.Position > (mt_s32)pIO->IoInfo.MemInfo.Length)
    {
        MTGO_ERROR(MT_FAILURE);
        pIO->IoInfo.MemInfo.Position = StartPos;
        return MT_FAILURE;
    }

    if ((mt_s32)pIO->IoInfo.MemInfo.Position < 0)
    {
        MTGO_ERROR(MT_FAILURE);
        pIO->IoInfo.MemInfo.Position = StartPos;
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

/**
 \brief get file lengthe
 \param[in] const IO_INSTANCE_S *pIO  file instance pointer 
 \param[in] mt_u32 *pLength   file length 
 \param[out] none
 \retval none
 \return none
 */
static mt_s32 Mem_GetLength(const IO_INSTANCE_S *pIO, mt_u32 *pLength)
{
    *pLength = pIO->IoInfo.MemInfo.Length;
    return MT_SUCCESS;
}

/**
 \brief get current position 
 \param[in] const IO_INSTANCE_S *pIO file instance pointer
 \param[in] mt_u32 *pPos    pointer for file current position
 \param[out] none
 \retval none
 \return none
 */
static mt_s32 Mem_GetPos(const IO_INSTANCE_S *pIO, mt_u32 *pPos)
{
    *pPos = pIO->IoInfo.MemInfo.Position;
    return MT_SUCCESS;
}

/**
 \brief get current position address
 \param[in] const IO_INSTANCE_S *pIO file instance pointer
 \param[out] none
 \retval none
 \return pos address
 */
static u8 * Mem_GetPosAddr(const IO_INSTANCE_S *pIO)
{
    return  (u8 *)pIO->IoInfo.MemInfo.pAddr + pIO->IoInfo.MemInfo.Position;
}


/**
 \brief  destroi file instance 
 \param[in] IO_INSTANCE_S *pIO file instance pointer
 \param[out] none
 \retval none
 \return none
 */
static mt_void Mem_destroy(IO_INSTANCE_S *pIO)
{
    pIO->IoInfo.MemInfo.pAddr    = MT_NULL_PTR;
    pIO->IoInfo.MemInfo.Length   = 0;
    pIO->IoInfo.MemInfo.Position = 0;
    return;
}


/**
 \brief create virtual file system instance 
 \param[out] IO_HANDLE *pIO virtual file system instance 
 \param[in] const DEC_IOINFO_S * pDecInfo  pointer of input file information
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOCreate(IO_HANDLE *pIO, const IO_DESC_S *pDecInfo)
{
    IO_INSTANCE_S *pIoInstance = MT_NULL_PTR;
    mt_s32 Ret;

    /**check input parameter */
    if (MT_NULL_PTR == pDecInfo)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    if (MT_NULL_PTR == pIO)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    /** create instance */
    pIoInstance = (IO_INSTANCE_S *)MTGO_Malloc(sizeof(IO_INSTANCE_S));
    if (MT_NULL_PTR == pIoInstance)
    {
        MTGO_ERROR(MTGO_ERR_NOMEM);
        return MTGO_ERR_NOMEM;
    }

    /** hook the file instance*/
        if (MT_NULL_PTR == pDecInfo->IoInfo.MemInfo.pAddr)
        {
            MTGO_ERROR(MTGO_ERR_NULLPTR);
            Ret = MTGO_ERR_NULLPTR;
            goto freemem;
        }

        pIoInstance->Type = IO_TYPE_MEM;
        pIoInstance->IoCreate  = Mem_Create;
        pIoInstance->IoDestroy = Mem_destroy;
        pIoInstance->IoRead   = Mem_Read;
        pIoInstance->IoSeek   = Mem_Seek;
        pIoInstance->IoGetPos = Mem_GetPos;
        pIoInstance->IoGetLength = Mem_GetLength;
        pIoInstance->IoGetPosAddr = Mem_GetPosAddr;

    /** initial instance */
    Ret = pIoInstance->IoCreate(pIoInstance, pDecInfo);
    if (MT_SUCCESS != Ret)
    {
        MTGO_ERROR(Ret);
        goto freemem;
    }

    /** get hanle */
    *pIO = (IO_HANDLE)pIoInstance;
    return MT_SUCCESS;

freemem:

    /** free memory */
    MTGO_Free(pIoInstance);
    return Ret;
}

/**
 \brief destroy virtual file system 
 \param[in] IO_HANDLE IO VFS instance 
 \param[out] none
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IODestroy(IO_HANDLE IO)
{
    IO_INSTANCE_S *pIoInstance;

    /** get instance */
    pIoInstance = (IO_INSTANCE_S *)IO;
    /** close instance*/
    pIoInstance->IoDestroy(pIoInstance);
    /** destroy instance*/
    MTGO_Free(pIoInstance);
    return MT_SUCCESS;
}

/**
 \brief read data 
 \param[in] IO_HANDLE IO VFS instance 
 \param[in] mt_void *pBuf   input data BUFFER
 \param[in] mt_u32 BufLen   input BUFFER length 
 \param[out] mt_u32 *pCopyLen lenth has read 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IORead(IO_HANDLE IO, mt_void *pBuf, mt_u32 BufLen, mt_u32 *pCopyLen, MT_BOOL *pEndFlag)
{
    IO_INSTANCE_S *pIoInstance;
    mt_s32 Ret;

    pIoInstance = (IO_INSTANCE_S *)IO;

    if ((MT_NULL_PTR == pCopyLen) || (MT_NULL_PTR == pBuf) || (MT_NULL_PTR == pEndFlag))
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    /** use function pointer */
    Ret = pIoInstance->IoRead(pIoInstance, pBuf, BufLen, pCopyLen);
    if (MT_SUCCESS != Ret)
    {
        MTGO_ERROR(Ret);
        return Ret;
    }

    /** check is  file end */
    if ((0 == *pCopyLen) || (*pCopyLen < BufLen))
    {
        *pEndFlag = MT_TRUE;
    }
    else
    {
        *pEndFlag = MT_FALSE;
    }

    return MT_SUCCESS;
}

/**
 \brief VFS file seek 
 \param[in] IO_HANDLE IO VFS instance 
 \param[in] IO_POS_E Position  
 \param[in] mt_s32 Offset   offset 
 \param[out] none
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOSeek(IO_HANDLE IO, IO_POS_E Position, mt_s32 Offset)
{
    IO_INSTANCE_S *pIoInstance;

    pIoInstance = (IO_INSTANCE_S *)IO;

    if (Position >= IO_POS_BUTT)
    {
        MTGO_ERROR(MTGO_ERR_INVPARAM);
        return MTGO_ERR_INVPARAM;
    }
    /**use function pointer*/
    return pIoInstance->IoSeek(pIoInstance, Position, Offset);
}

/**
 \brief get the type of file system 
 \param[in] IO_HANDLE IO
 \param[out] IO_TYPE_E *pIOType pointer for file type
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOGetType(IO_HANDLE IO, IO_TYPE_E *pIOType)
{
    IO_INSTANCE_S *pIoInstance;

    pIoInstance = (IO_INSTANCE_S *)IO;

    if (MT_NULL_PTR == pIOType)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    *pIOType = pIoInstance->Type;
    return MT_SUCCESS;
}

/**
 \brief get file length 
 \param[in] IO_HANDLE IO
 \param[out]  mt_u32 *pLength pointer of file length 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOGetLength(IO_HANDLE IO, mt_u32 *pLength)
{
    IO_INSTANCE_S *pIoInstance;

    pIoInstance = (IO_INSTANCE_S *)IO;

    if (MT_NULL_PTR == pLength)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    return pIoInstance->IoGetLength(pIoInstance, pLength);
}

/**
 \brief  get file postion 
 \param[in] IO_HANDLE IO
 \param[out]  mt_u32 *pPos pointer for file postion 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOGetPos(IO_HANDLE IO, mt_u32 *pPos)
{
    IO_INSTANCE_S *pIoInstance;

    pIoInstance = (IO_INSTANCE_S *)IO;

    if (MT_NULL_PTR == pPos)
    {
        MTGO_ERROR(MTGO_ERR_NULLPTR);
        return MTGO_ERR_NULLPTR;
    }

    return pIoInstance->IoGetPos(pIoInstance, pPos);
}


/**
 \brief  get current postion addr
 \param[in] IO_HANDLE IO
 \param[out]  mt_u32 *pPos pointer for file postion 
 \retval none
 \return u8 *
 */
u8 * MTGO_ADP_IOGetCurrentPosAddr(IO_HANDLE IO)
{
    IO_INSTANCE_S *pIoInstance;

    pIoInstance = (IO_INSTANCE_S *)IO;

    if(pIoInstance->IoInfo.MemInfo.Length < pIoInstance->IoInfo.MemInfo.Position)
    {
        MTGO_ERROR(MT_FAILURE);
        return NULL;
    }

    return pIoInstance->IoGetPosAddr(pIoInstance);
}



