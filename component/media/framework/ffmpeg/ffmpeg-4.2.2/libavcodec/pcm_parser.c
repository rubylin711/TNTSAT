/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2024 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "parser.h"


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define MAX_PCM_DVD_CODEC_DATA_LEN            3

static int pcm_dvd_parse(
    AVCodecContext *avctx, const uint8_t *header)
{
    /* no traces of 44100 and 32000Hz in any commercial software or player */
    static const uint32_t frequencies[4] = { 48000, 96000, 44100, 32000 };

    av_log(avctx, AV_LOG_DEBUG, "pcm_dvd_parse_header: header = %02x%02x%02x\n",
            header[0], header[1], header[2]);
    /*
     * header[0] emphasis (1), muse(1), reserved(1), frame number(5)
     * header[1] quant (2), freq(2), reserved(1), channels(3)
     * header[2] dynamic range control (0x80 = off)
     */

    /* get the sample depth and derive the sample format from it */
    avctx->bits_per_coded_sample = 16 + (header[1] >> 6 & 3) * 4;
    if (avctx->bits_per_coded_sample == 28) {
        av_log(avctx, AV_LOG_ERROR,
               "PCM DVD unsupported sample depth %i\n",
               avctx->bits_per_coded_sample);
        return AVERROR_INVALIDDATA;
    }
    avctx->sample_fmt = avctx->bits_per_coded_sample == 16 ? AV_SAMPLE_FMT_S16
                                                           : AV_SAMPLE_FMT_S32;
    avctx->bits_per_raw_sample = avctx->bits_per_coded_sample;

    /* get the sample rate */
    avctx->sample_rate = frequencies[header[1] >> 4 & 3];
    /* get the number of channels */
    avctx->channels = 1 + (header[1] & 7);
    /* calculate the bitrate */
    avctx->bit_rate = avctx->channels *
                      avctx->sample_rate *
                      avctx->bits_per_coded_sample;
    return 0;
}

static int pcm_dvd_audio_parse(AVCodecParserContext *s1, AVCodecContext *avctx,
                        const uint8_t **poutbuf, int *poutbuf_size,
                        const uint8_t *buf, int buf_size)
{
    if (!buf || buf_size < MAX_PCM_DVD_CODEC_DATA_LEN) {
        *poutbuf      = buf;
        *poutbuf_size = buf_size;
        return buf_size;
    }

    if (!avctx->sample_rate || !avctx->channels) {
        (void) pcm_dvd_parse(avctx, buf);
    }
    /* always return the full packet. this parser isn't doing any splitting or
       combining, only packet analysis */
    *poutbuf      = buf + MAX_PCM_DVD_CODEC_DATA_LEN;
    *poutbuf_size = buf_size - MAX_PCM_DVD_CODEC_DATA_LEN;
    return buf_size;
}

const AVCodecParser ff_pcm_dvd_parser = {
    .codec_ids      = { AV_CODEC_ID_PCM_DVD},
    .parser_parse   = pcm_dvd_audio_parse,
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
