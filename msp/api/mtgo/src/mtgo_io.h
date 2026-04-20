/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MTGO_IO_H__
#define __MTGO_IO_H__

/* add include here */
#include "mt_type.h"
#include "mt_go_comm.h"

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Macro Definition ******************************/



/*************************** Structure Definition ****************************/

typedef ulong IO_HANDLE;


/** IO device type*/
typedef enum
{
    IO_TYPE_FILE = 0, /**<  file*/
    IO_TYPE_MEM,      /**< memory*/
    IO_TYPE_STREAM,   /**< memory*/
    IO_TYPE_BUTT
} IO_TYPE_E;
/**IOdevice info*/
typedef union mtIO_INFO_U
{
    /** info in  case : input source is memory */
    struct
    {
        mt_char *pAddr; /**<memory address*/
        mt_u32 Length; /**<memory suze*/
    } MemInfo;
} IO_INFO_U;

typedef struct mtIO_DESC_S
{
    IO_INFO_U IoInfo;
} IO_DESC_S;

/*IO file position info*/
typedef enum
{
    IO_POS_SET = 0,
    IO_POS_CUR,
    IO_POS_END,
    IO_POS_BUTT
} IO_POS_E;



/********************** Global Variable declaration **************************/



/******************************* API declaration *****************************/
/**
 \brief create virtual file system instance 
 \param[out] IO_HANDLE *pIO pointer to VFS instance 
 \param[in] const DEC_IOINFO_S * pDecInfo  
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOCreate(IO_HANDLE *pIO, const IO_DESC_S * pDecInfo);

/**
 \brief  destroy virtual file system instance
 \param[in] IO_HANDLE IO pointer to VFS instance 
 \param[out] none
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IODestroy(IO_HANDLE IO);

/**
 \brief fs data read 
 \param[in] IO_HANDLE IO pointer to VFS instance 
 \param[in] mt_void *pBuf   input data BUFFER
 \param[in] mt_u32 BufLen   data BUFFER length 
 \param[out] mt_u32 *pCopyLen  data length has actual readed 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IORead(IO_HANDLE IO, mt_void *pBuf, mt_u32 BufLen, mt_u32 *pCopyLen, MT_BOOL *pEndFlag);

/**
 \brief  VFS seek
 \param[in] IO_HANDLE IO pointer to VFS instance 
 \param[in] IO_POS_E Position start position
 \param[in] mt_s32 Offset   offset 
 \param[out] none
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOSeek(IO_HANDLE IO, IO_POS_E Position, mt_s32 Offset);

/**
 \brief  get fs type 
 \param[in] IO_HANDLE IO
 \param[out] IO_TYPE_E *pIOType pointer to VFS instance 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOGetType(IO_HANDLE IO, IO_TYPE_E *pIOType);

/**
 \brief get fs file size 
 \param[in] IO_HANDLE IO
 \param[out]  mt_u32 *pLength pointer to file size 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOGetLength(IO_HANDLE IO, mt_u32 *pLength);

/**
 \brief get fs position 
 \param[in] IO_HANDLE IO
 \param[out]  mt_u32 *pPos pointer to VFS instance 
 \retval none
 \return mt_s32
 */
mt_s32 MTGO_ADP_IOGetPos(IO_HANDLE IO, mt_u32 *pPos);

/**
 \brief  get current postion addr
 \param[in] IO_HANDLE IO
 \param[out]  mt_u32 *pPos pointer for file postion 
 \retval none
 \return u8 *
 */
u8 * MTGO_ADP_IOGetCurrentPosAddr(IO_HANDLE IO);


#ifdef __cplusplus
}
#endif
#endif /* __MTGO_IO_H__ */

