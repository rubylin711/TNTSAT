#define CHIP_VER_ID 0x8000
#if 0
typedef enum
{
  /*!
    For chip Wizards
    */
  IC_WIZARDS = CHIP_VER_ID | 0x10,
  /*!
    For chip Magic
    */
  IC_MAGIC = CHIP_VER_ID | 0x20,
  /*!
    For chip CX2448X
    */
  IC_CX2448X = CHIP_VER_ID | 0x40,
  /*!
    For chip Warriors
    */
  IC_WARRIORS = CHIP_VER_ID | 0x80,
  /*!
    For chip Sonata
    */
  IC_SONATA = CHIP_VER_ID | 0x100,
  /*!
    For chip Jazz
    */
  IC_JAZZ = CHIP_VER_ID | 0x200,
  /*!
    For chip Concerto
    */
  IC_CONCERTO = CHIP_VER_ID | 0x400,
  /*!
    For chip ENSEMBLE
    */
  IC_ENSEMBLE = CHIP_VER_ID | 0x800,

  /*!
    For chip Trio
    */
  IC_TRIO = CHIP_VER_ID | 0x1000,

}chip_ic_t;


typedef enum {
    /*!
    normal play mode
    */
    TS_SEQ_NORMAL_PLAY,
    /*!
    step forward play mode
    */
    TS_SEQ_STEP_FW_PLAY,
    /*!
    step backward play mode
    */
    TS_SEQ_STEP_BW_PLAY,
    /*!
    fast play mode 2x
    */
    TS_SEQ_FAST_PLAY_2X,
    /*!
    fast play mode 4x
    */
    TS_SEQ_FAST_PLAY_4X,
    /*!
    fast play mode 8x
    */
    TS_SEQ_FAST_PLAY_8X,
    /*!
    fast play mode 16x
    */
    TS_SEQ_FAST_PLAY_16X,
    /*!
    fast play mode 32x
    */
    TS_SEQ_FAST_PLAY_32X,
    /*!
    slow play mode 2x
    */
    TS_SEQ_SLOW_PLAY_2X,
    /*!
    slow play mode 4x
    */
    TS_SEQ_SLOW_PLAY_4X,
    /*!
    revert fast play mode 2x
    */
    TS_SEQ_REV_FAST_PLAY_2X,
    /*!
    revert fast play mode 4x
    */
    TS_SEQ_REV_FAST_PLAY_4X,
    /*!
    revert fast play mode 8x
    */
    TS_SEQ_REV_FAST_PLAY_8X,
    /*!
    revert fast play mode 16x
    */
    TS_SEQ_REV_FAST_PLAY_16X,
    /*!
    revert fast play mode 32x
    */
    TS_SEQ_REV_FAST_PLAY_32X,
    /*!
    revert slow play mode 2x
    */
    TS_SEQ_REV_SLOW_PLAY_2X,
    /*!
    revert slow play mode 4x
    */
    TS_SEQ_REV_SLOW_PLAY_4X
} ts_seq_play_mode_t;

#endif

typedef enum
{
  /*!
    PCM
    */
  AUDIO_PCM = 0,
  /*!
    MPEG audio layer I
    */
  AUDIO_MP1 = 1,
  /*!
    MPEG audio layer II
    */
  AUDIO_MP2 = 2,
  /*!
    MPEG audio layer III
    */
  AUDIO_MP3 = 3,
  /*!
     AC3
    */
  AUDIO_AC3_VSB = 4,
  /*!
    EAC3
    */
  AUDIO_EAC3 = 5,
  /*!
    HE_AAC
    */
  AUDIO_AAC = 6,
  /*!
    AAC_V2
    */
  AUDIO_AAC_V2 = 7,
  /*!
    SPDIF
    */
  AUDIO_SPDIF = 107,
  /*!
    dolby convert
    */
  AUDIO_DOLBY_CONVERT = 108,
  /*!
   SPDIF_AC3
    */
  AUDIO_SPDIF_AC3 = 109,
  /*!
   SPDIF_EAC3
    */
  AUDIO_SPDIF_EAC3 = 110,
  /*!
    dts spdif pass through
    */
  AUDIO_SPDIF_DTS = 111,
  /*!
    Unknown audio format.
    */
  AUDIO_UNKNOWN
}adec_src_fmt_vsb_t;

