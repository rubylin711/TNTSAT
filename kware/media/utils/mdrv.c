/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2025 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#define MODULE_TAG "MDRV"
#include "mutil.h"
#include "mlog.h"
#include "mdrv.h"
#include "mt_unf_video.h"
#include "mt_audio_codec.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#if MT_DES("Internal Data and Structure", 1)
typedef struct {
    unsigned int type;
    const char *str;
} mdrv_type2str_t;

#endif

#if MT_DES("Internal", 1)
const static mdrv_type2str_t audio_type2str[] = {
    {HA_AUDIO_ID_PCM            , "PCM"            },
    {HA_AUDIO_ID_MP2            , "MP2"            },
    {HA_AUDIO_ID_MP3            , "MP3"            },
    {HA_AUDIO_ID_AAC            , "AAC"            },
    {HA_AUDIO_ID_BLYRAYLPCM     , "BLYRAYLPCM"     },
    {HA_AUDIO_ID_COOK           , "COOK"           },
    {HA_AUDIO_ID_DRA            , "DRA"            },
    {HA_AUDIO_ID_WMA9STD        , "WMA9STD"        },
    {HA_AUDIO_ID_VORBIS         , "VORBIS"         },
    {HA_AUDIO_ID_OGG            , "OGG"            },
    {HA_AUDIO_ID_FLAC           , "FLAC"           },
    {HA_AUDIO_ID_APE            , "APE"            },
    {HA_AUDIO_ID_OPUS           , "OPUS"           },
    {HA_AUDIO_ID_AMRNB          , "AMRNB"          },
    {HA_AUDIO_ID_AMRWB          , "AMRWB"          },
    {HA_AUDIO_ID_G711           , "G711"           },
    {HA_AUDIO_ID_G722           , "G722"           },
    {HA_AUDIO_ID_TRUEHD         , "TRUEHD"         },
    {HA_AUDIO_ID_AC3PASSTHROUGH , "AC3PASSTHROUGH" },
    {HA_AUDIO_ID_EAC3PASSTHROUGH, "EAC3PASSTHROUGH"},
    {HA_AUDIO_ID_DTSPASSTHROUGH , "DTSPASSTHROUGH" },
    {HA_AUDIO_ID_DOLBY_PLUS     , "DOLBY PLUS"     },
    {HA_AUDIO_ID_DOLBY_TRUEHD   , "DOLBY TRUEHD"   },
    {HA_AUDIO_ID_DOLBY_CONVERT  , "DOLBY CONVERT"  },
    {HA_AUDIO_ID_DOLBY_AC4      , "DOLBY AC4"      },
    {HA_AUDIO_ID_VVID           , "VVID(av3a)"     },
    {HA_AUDIO_ID_DTSHD          , "DTSHD"          },
    {HA_AUDIO_ID_DTSM6          , "DTSM6"          },
};

const static mdrv_type2str_t video_type2str[] = {
    {MT_UNF_VCODEC_TYPE_MPEG2,           "MPEG2"    },
    {MT_UNF_VCODEC_TYPE_MPEG4,           "MPEG4"    },
    {MT_UNF_VCODEC_TYPE_AVS,             "AVS"      },
    {MT_UNF_VCODEC_TYPE_H263,            "H263"     },
    {MT_UNF_VCODEC_TYPE_H264,            "H264"     },
    {MT_UNF_VCODEC_TYPE_REAL8,           "REAL8"    },
    {MT_UNF_VCODEC_TYPE_REAL9,           "REAL9"    },
    {MT_UNF_VCODEC_TYPE_VC1,             "VC1"      },
    {MT_UNF_VCODEC_TYPE_VP6,             "VP6"      },
    {MT_UNF_VCODEC_TYPE_VP6F,            "VP6F"     },
    {MT_UNF_VCODEC_TYPE_VP6A,            "VP6A"     },
    {MT_UNF_VCODEC_TYPE_MJPEG,           "MJPEG"    },
    {MT_UNF_VCODEC_TYPE_SORENSON,        "SORENSON" },
    {MT_UNF_VCODEC_TYPE_DIVX3,           "DIVX3"    },
    {MT_UNF_VCODEC_TYPE_RAW,             "RAW"      },
    {MT_UNF_VCODEC_TYPE_JPEG,            "JPEG"     },
    {MT_UNF_VCODEC_TYPE_VP8,             "VP8"      },
    {MT_UNF_VCODEC_TYPE_MSMPEG4V1,       "MSMPEG4V1"},
    {MT_UNF_VCODEC_TYPE_MSMPEG4V2,       "MSMPEG4V2"},
    {MT_UNF_VCODEC_TYPE_MSVIDEO1,        "MSVIDEO1" },
    {MT_UNF_VCODEC_TYPE_WMV1,            "WMV1"     },
    {MT_UNF_VCODEC_TYPE_WMV2,            "WMV2"     },
    {MT_UNF_VCODEC_TYPE_RV10,            "RV10"     },
    {MT_UNF_VCODEC_TYPE_RV20,            "RV20"     },
    {MT_UNF_VCODEC_TYPE_SVQ1,            "SVQ1"     },
    {MT_UNF_VCODEC_TYPE_SVQ3,            "SVQ3"     },
    {MT_UNF_VCODEC_TYPE_H261,            "H261"     },
    {MT_UNF_VCODEC_TYPE_VP3,             "VP3"      },
    {MT_UNF_VCODEC_TYPE_VP5,             "VP5"      },
    {MT_UNF_VCODEC_TYPE_CINEPAK,         "CINEPAK"  },
    {MT_UNF_VCODEC_TYPE_INDEO2,          "INDEO2"   },
    {MT_UNF_VCODEC_TYPE_INDEO3,          "INDEO3"   },
    {MT_UNF_VCODEC_TYPE_INDEO4,          "INDEO4"   },
    {MT_UNF_VCODEC_TYPE_INDEO5,          "INDEO5"   },
    {MT_UNF_VCODEC_TYPE_MJPEGB,          "MJPEGB"   },
    {MT_UNF_VCODEC_TYPE_MVC,             "MVC"      },
    {MT_UNF_VCODEC_TYPE_HEVC,            "HEVC"     },
    {MT_UNF_VCODEC_TYPE_DV,              "DV"       },
    {MT_UNF_VCODEC_TYPE_VP9,             "VP9"      },
    {MT_UNF_VCODEC_TYPE_AVS2,            "AVS2"     },
};

const static char *avdec_type_to_str(
    mdrv_type2str_t *array,
    unsigned int array_cnt, unsigned int type)
{
    for (int i = 0; i < array_cnt; i++) {
        if (type == array[i].type) {
            return array[i].str;
        }
    }
    return "";
}

#endif

#if MT_DES("External API Definition", 1)

const char *mdrv_adec_type_to_str(const unsigned int type)
{
    return avdec_type_to_str(
        (mdrv_type2str_t *) &audio_type2str[0],
        ARRAY_CNT(audio_type2str), type);
}

const char *mdrv_vdec_type_to_str(const unsigned int type)
{
    return avdec_type_to_str(
        (mdrv_type2str_t *) &video_type2str[0],
        ARRAY_CNT(video_type2str), type);
}

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */
