/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

/**
 \file
 \brief Server (SVR) player module. CNcomment:svr playerÄ£¿éCNend
 \author Montage Technologies Co., Ltd.
 \date 2008-2018
 \version 1.0
 \author
 \date 2017-11-10
 */
#ifndef __MT_SVR_VSINK_H__
#define __MT_SVR_VSINK_H__

#include <mt_type.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef enum mtSVR_VSINK_CMD_E {
    /**
     * Set picture geometry
     * parameter list:
     *  MT_U32 width, MT_U32 height, MT_U32 format
     */
    MT_SVR_VSINK_SET_DIMENSIONS,
    MT_SVR_VSINK_SET_FORMAT,
    /**
     * set picture number
     * parameter list:
     *  MT_U32 count,
     */
    MT_SVR_VSINK_SET_PICNB,
    /**
     * write data to the picture according to format
     * parameter list:
     *  MT_SVR_PICTURE_S* pic, MT_U8* data
     */
    MT_SVR_VSINK_WRITE_PIC,
    /**
     * set picture crop rect
     */
    MT_SVR_VSINK_SET_CROP,
    /**
     * get mini-undequeue buffer number
     * parameter list:
     *  MT_U32*
     */
    MT_SVR_VSINK_GET_MINBUFNB,

    /**
     * check sync fence
     * parameter list:
     *  MT_SVR_PICTURE_S* pic
     */
    MT_SVR_VSINK_CHECK_FENCE,
} MT_SVR_VSINK_CMD_E;

typedef struct mtSVR_PICTURE_S
{
    //property for frame buffer
    MT_U32 u32Width;
    MT_U32 u32Height;
    MT_U32 u32Stride;
    mt_s32 s32FrameBufFd;
    MT_HANDLE hBuffer;
    //property for meta data buffer
    mt_s32 s32MetadataBufFd;
    MT_HANDLE hMetadataBuf;
    MT_U32 u32MetadataBufSize;
    MT_U32 u32RepeatCnt;

    MT_S64 s64Pts;
    MT_VOID *priv;
    MT_U32 u32PrivSize;
} MT_SVR_PICTURE_S;

typedef struct mtSVR_VFORMAT_S
{
    MT_U32 u32Fmt;
} MT_SVR_VFORMAT_S;

typedef struct mtSVR_VSINK_S MT_SVR_VSINK_S;
struct mtSVR_VSINK_S
{
    /**
     * Return buffers to original pool
     */
    mt_s32 (*cancel)(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *pics, MT_U32 cnt);

    /**
     * Return a pointer of internal picture pool.
     * Count just indicate user expected buffer number,
     * but it is depend on implementation.
     * caller does not need to free MT_SVR_PIC_POOL_S pointer.
     */
    mt_s32 (*dequeue)(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *pics, MT_U32 cnt);

    /**
     * Prepare a picture for display
     */
    mt_s32 (*prepare)(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *picIn);

    /**
     * Display a picture
     */
    mt_s32 (*queue)(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *picIn);

    /**
     * Control on the module
     */
    mt_s32 (*control)(MT_SVR_VSINK_S *vsink, MT_U32 cmd, ...);

    /**
     * private data pointer
     */
    MT_VOID *opaque;
};

static inline mt_s32 MT_SVR_VSINK_Cancel(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *pics, MT_U32 cnt)
{
    if (vsink && vsink->cancel) {
	return vsink->cancel(vsink, pics, cnt);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_Dequeue(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *pics, MT_U32 count)
{
    if (vsink && vsink->dequeue) {
	return vsink->dequeue(vsink, pics, count);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_Prepare(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *picIn)
{
    if (vsink && vsink->prepare) {
	return vsink->prepare(vsink, picIn);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_Queue(MT_SVR_VSINK_S *vsink, MT_SVR_PICTURE_S *picIn)
{
    if (vsink && vsink->queue) {
	return vsink->queue(vsink, picIn);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_SetDimensions(MT_SVR_VSINK_S *vsink,
                                                MT_U32 width, MT_U32 height)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_SET_DIMENSIONS, width, height);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_SetFormat(MT_SVR_VSINK_S *vsink, MT_U32 format)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_SET_FORMAT, format);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_SetPictCnt(MT_SVR_VSINK_S *vsink,
                                             MT_U32 count)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_SET_PICNB, count);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_WritePicture(MT_SVR_VSINK_S *vsink,
                                               MT_SVR_PICTURE_S *pic, MT_U8 *data, MT_U32 size)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_WRITE_PIC, pic, data, size);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_SetCrop(MT_SVR_VSINK_S *vsink,
                                          mt_s32 left, mt_s32 top, mt_s32 right, mt_s32 bottom)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_SET_CROP, left, top, right, bottom);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_GetMinBufferNb(MT_SVR_VSINK_S *vsink,
                                                 MT_U32 *pu32MinBufferNb)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_GET_MINBUFNB, pu32MinBufferNb);
    }
    return MT_FAILURE;
}

static inline mt_s32 MT_SVR_VSINK_CheckFence(MT_SVR_VSINK_S *vsink,
                                             MT_SVR_PICTURE_S *pic)
{
    if (vsink && vsink->control) {
	return vsink->control(vsink, MT_SVR_VSINK_CHECK_FENCE, pic);
    }
    return MT_FAILURE;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
#endif /*__MT_SVR_VSINK_H__*/
