/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MT_SVR_ASINK_H__
#define __MT_SVR_ASINK_H__

typedef struct mtSVR_ASINK_OPEN_PARAM
{
    MT_U32 u32SampleRate; /**<Sampling rate*/    /**<CNcomment: 采样率*/
    MT_U16 u16Channels; /**<Number of channels*/ /**<CNcomment: 通道数量*/
    /** u16BitPerSample: (PCM) Data depth, and format of storing the output data
          If the data depth is 16 bits, 16-bit word memory is used.
          If the data depth is greater than 16 bits, 32-bit word memory is used, and data is stored as left-aligned data. That is, the valid data is at upper bits.
     */
    /**CNcomment: u16BitPerSample: (PCM) 数据位宽设置. 输出存放格式
          等于16bit:   占用16bit word内存
          大于16bit:   占用32bit word内存, 数据左对齐方式存放(有效数据在高位)
     */
    MT_U16 u16BitPerSample; /**<Data depth*/ /**<CNcomment: 数据位宽*/
} MT_SVR_ASINK_OPEN_PARAM;

typedef enum mtSVR_ASINK_EVENT_E {
    MT_SVR_ASINK_EVENT_STREAM_END = 0,
    /**<Sent after all the buffers queued in AF and HW are played, refer to Android::MediaPlayerBase::AudioSink::cb_event_t*/ /**<CNcomment: 在AF和硬件中的数据播放完成后发送，请参考:Android::MediaPlayerBase::AudioSink::cb_event_t*/
    MT_SVR_ASINK_EVENT_FILL_BUFFER,
    /**<Request to write more data to buffer.*/ /**<CNcomment: 请求向buffer中写入更多数据*/
} MT_SVR_ASINK_EVENT_E;

typedef struct mtSVR_ASINK_S MT_SVR_ASINK_S;

/** Callback returns the number of bytes actually written to the buffer.. */
/** CNcomment:回调函数返回向buffer中写入的字节数*/
typedef MT_SIZE_T (*MT_SVR_ASINK_Callback)(
    MT_SVR_ASINK_S *asink, MT_VOID *buffer, MT_SIZE_T size, MT_VOID *cookie,
    MT_SVR_ASINK_EVENT_E event);

struct mtSVR_ASINK_S
{
    mt_s32 (*open)(MT_SVR_ASINK_S *asink, MT_SVR_ASINK_OPEN_PARAM *param, MT_SVR_ASINK_Callback cb, MT_VOID *cookie);
    mt_s32 (*close)(MT_SVR_ASINK_S *asink);
    mt_s32 (*start)(MT_SVR_ASINK_S *asink);
    mt_s32 (*stop)(MT_SVR_ASINK_S *asink);
    mt_s32 (*pause)(MT_SVR_ASINK_S *asink);
    mt_s32 (*flush)(MT_SVR_ASINK_S *asink);
    mt_s32 (*control)(MT_SVR_ASINK_S *asink, MT_U32 cmd, ...);
    MT_VOID *opaque; /**<private data pointer*/ /**<CNcomment: 私有数据*/
};

#endif
