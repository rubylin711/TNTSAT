/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __TTX_DEC_VSB_H__
#define __TTX_DEC_VSB_H__

/*!
  ABC
  */
#define TTX_SUPPORT_DRCS        0

/*!
  ABC
  */
#define TTX_PACKET_LENGTH       45

/*!
  ABC
  */
#define TTX_ROWS                25
/*!
  ABC
  */
#define TTX_COLUMNS             40
/*!
  ABC
  */
#define TTX_LAST_ROW            (((TTX_ROWS) - 1) * (TTX_COLUMNS))

/*!
  ABC
  */
#define TTX_FIRST_SUBPAGE       0x6FFF
/*!
  ABC
  */
#define TTX_ANY_SUBPAGE         0x7FFF
/*!
  ABC
  */
#define TTX_NULL_SUBPAGE        0x3F7F
/*!
  ABC
  */
#define TTX_NULL_PAGE_NO        0x8FF

/*!
  ABC
  */
#define TTX_NO_PAGE(page_no)    (((page_no) & 0xFF) == 0xFF)

/*!
  ABC
  */
#define TTX_DRCS_PTUS_PER_PAGE    48

/*!
  ABC
  */
#define C4_ERASE_PAGE           (1 << 3)
/*!
  ABC
  */
#define C5_NEWSFLASH            (1 << 6)
/*!
  ABC
  */
#define C6_SUBTITLE             (1 << 7)
/*!
  ABC
  */
#define C7_SUPPRESS_HEADER      (1 << 8)
/*!
  ABC
  */
#define C8_UPDATE               (1 << 9)
/*!
  ABC
  */
#define C9_INTERRUPTED          (1 << 10)
/*!
  ABC
  */
#define C10_INHIBIT_DISPLAY     (1 << 11)
/*!
  ABC
  */
#define C11_MAGAZINE_SERIAL     (1 << 12)
/*!
  ABC
  */
#define C12_FRAGMENT            (1 << 13)
/*!
  ABC
  */
#define C13_PARTIAL_PAGE        (1 << 14)
/*!
  ABC
  */
#define C14_RESERVED            (1 << 15)

/*!
  Index of the "transparent" color in the Level 2.5/3.5 color_map[].
  */
#define TTX_TRANSPARENT_BLACK   8

/*!
  return code
  */
typedef enum
{
    /*!
      ABC
      */
    VBI_RC_SUCCESS,
    /*!
      ABC
      */
    VBI_RC_FAILED,
    /*!
      ABC
      */
    VBI_RC_BAD_BAOUNDARY,
    /*!
      ABC
      */
    VBI_RC_INVALID_DATA
} vbi_rc_t;

/*!
  level of decoder
  */
typedef enum
{
    /*!
      ABC
      */
    TTX_DEC_LEVEL_1,
    /*!
      ABC
      */
    TTX_DEC_LEVEL_1P5,
    /*!
      ABC
      */
    TTX_DEC_LEVEL_2P5,
    /*!
      ABC
      */
    TTX_DEC_LEVEL_3P5
} ttx_dec_level_t;

/*!
  priority of page
  */
typedef enum
{
    /*!
      undefine
      */
    TTX_PAGE_PRI_UNDEF = 0,
    /*!
      other
      */
    TTX_PAGE_PRI_LOWEST,
    /*!
      Page whitch sub-page number is 0 or 1
      */
    TTX_PAGE_PRI_3,
    /*!
      Page such as 111, 222, 333 ...
      and sub-page number is 0 or 1
      */
    TTX_PAGE_PRI_2,
    /*!
      Page number is 100, 200, 300 ...
      and sub-page number is 0 or 1
      */
    TTX_PAGE_PRI_1,
    /*!
      link page, with sub-page no is 0 or 1
      */
    TTX_PAGE_PRI_LINK_PAGE,
    /*!
      currently waiting page
      */
    TTX_PAGE_PRI_WAITING_PAGE,
    /*!
      resource
      */
    TTX_PAGE_PRI_RESOURCE,
    /*!
      max
      */
    TTX_PAGE_PRI_MAX
} ttx_page_priority_t;

/*!
  page type
  */
typedef enum
{
    /*!
      ABC
      */
    TTX_PAGE_TYPE_UNKNOWN           = 0x00,

    /*!
      ABC
      */
    TTX_PAGE_TYPE_INVALID           = 0x00,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_NORMAL            = 0x01,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_SUBTITLE          = 0x70,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_SUBT_INDEX        = 0x78,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_NO_STD_SUBPAGE    = 0x79,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_PROG_SCHEDULE        = 0x81,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_NOT_PUBLIC           = 0x80,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_CA_DATA_BRODCAST     = 0xE0,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_EPG_DATA             = 0xE3,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_SYS_PAGE             = 0xE7,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_DISP_SYS_PAGE        = 0xF7,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_KEY_WORD_SEARCH_LIST = 0xF9,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_TOP_BLOCK            = 0xFA,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_TOP_GROUP            = 0xFB,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_TRIGGER_DATA         = 0xFC,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_ACI                  = 0xFD,
    /*!
      ABC
      */
    TTX_PAGE_TYPE_TOP_PAGE             = 0xFE
} ttx_page_type_t;

/*!
  page function
  Depending on context.
  */
typedef enum
{
    /*!
      ABC
      */
    TTX_PAGE_FUNC_SUBTITLE      = -6,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_ACI           = -5,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_EPG           = -4,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_EACEMT_TRIGGER  = -3,

    /*!
        Invalid data
      */
    /*!
      ABC
      */
    TTX_PAGE_FUNC_DISCARD       = -2,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_UNKOWN        = -1,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_LOP           = 0,

    /*!
        Data broadcasting page coded according
        to EN 300 708 Section 4 (Page Format - Clear).
     */
    /*!
      ABC
      */
    TTX_PAGE_FUNC_DATA,

    /*!
      ABC
      */
    TTX_PAGE_FUNC_GPOP,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_POP,
    /*!
      ABC
      */
    /*!
      ABC
      */
    TTX_PAGE_FUNC_GDRCS,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_DRCS,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_MOT,
    /*!
      ABC
      */
    /*!
      ABC
      */
    TTX_PAGE_FUNC_MIP,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_BTT,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_AIT,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_MPT,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_MPT_EX,
    /*!
      ABC
      */
    TTX_PAGE_FUNC_IEC_TRIGGER
} ttx_page_function_t;

/*!
  ABC
  */
typedef struct
{
    /*!
      ABC
      */
    ttx_page_function_t function;
    /*!
      ABC
      */
    s16                 page_no;
    /*!
      ABC
      */
    s16                 sub_no;
    u8                  manual_flag;
    u16                 control_bit;  //add for fixing bug 18731    
} ttx_page_link_t;

/*!
  ABC
  */
typedef struct ttx_ext_fallback
{
    /*!
      ABC
      */
    MT_BOOL    black_bg_substitution;
    /*!
      ABC
      */
    u8      left_panel_columns;
    /*!
      ABC
      */
    u8      right_panel_columns;
} ttx_ext_fallback_t;

/*!
  object type
  Depending on context.
  */
typedef enum
{
    /*!
      ABC
      */
    TTX_OBJ_LOCAL_ENH_DATA = 0,
    /*!
      ABC
      */
    TTX_OBJ_NONE           = 0,
    /*!
      ABC
      */
    TTX_OBJ_ACTIVE,
    /*!
      ABC
      */
    TTX_OBJ_ADAPTIVE,
    /*!
      ABC
      */
    TTX_OBJ_PASSIVE
} ttx_object_type_t;

/*!
  DRCS mode
  */
typedef enum
{
    /*!
      ABC
      */
    DRCS_MODE_12B10D1 = 0,
    /*!
      ABC
      */
    DRCS_MODE_12B10D2,
    /*!
      ABC
      */
    DRCS_MODE_12B10D4,
    /*!
      ABC
      */
    DRCS_MODE_6B5D4,
    /*!
      ABC
      */
    DRCS_MODE_SUBSEQUENT_PTU = 14,
    /*!
      ABC
      */
    DRCS_MODE_NO_DATA
} ttx_drcs_mode_t;

/*!
  POP link
  */
typedef struct
{
    /*!
      ABC
      */
    s16                 page_no;
    /*!
      ABC
      */
    ttx_ext_fallback_t  fallback;

    /*!
      ABC
      */
    struct
    {
        /*!
          ABC
          */
        ttx_object_type_t   type;
        /*!
          ABC
          */
        u8                  address;
    } default_obj[2];

} ttx_pop_link_t;

/*!
  ABC
  */
typedef struct
{
    /*!
      ABC
      */
    u8    address;
    /*!
      ABC
      */
    u8    mode;
    /*!
      ABC
      */
    u8    data;
} ttx_triplet_t;

/*!
  Level one page
  */
typedef struct
{
    /*!
      ABC
      */
    u8              raw[26][40];
    /*!
      Packet X/27/0-5 links.
      */
    ttx_page_link_t link[6 * 6];
    /*!
      ABC
      */
    MT_BOOL            have_flof;
} ttx_lop_t;

/*!
  extension of ttx
  */
typedef struct
{
    /*!
        We have data from packets X/28 (in LOP) or M/29 (in magazine)
        with this set of designations. LOP pages without X/28 packets
        should fall back to the magazine defaults unless these bits
        are set. The extension data in struct ttx_magazine is always
        valid, contains global defaults as specified in EN 300 706
        unless M/29 packets have been received.

        - 1 << 4: color_map[0 ... 15] is valid
        - 1 << 1: drcs_clut[] is valid
        - 1 << 0 or 1 << 4: the remaining fields are valid.

        color_map[32 .. 39] is always valid.
      */
    u32                 designations;

    /*!
        Primary and secondary character set.
      */
    u8                  charset_code[2];

    u32                 def_screen_color;
    u32                 def_row_color;

    /*!
        Adding these values (0, 8, 16, 24) to character color
        0 ... 7 gives an index into color_map[] below.
      */
    u8                  foreground_clut;
    /*!
      ABC
      */
    u8                  background_clut;


    /*!
      ABC
      */
    ttx_ext_fallback_t  fallback;

    /*!
      For compatibility with the drcs_clut pointer in ttx_osd_page.
      */
    u8                  drcs_clut[2 + 2 * 4 + 2 * 16];

    /*!
        Five palettes of 8 each colors each.

        Level 1.0 and 1.5 pages use only palette #0, Level 2.5 and
        3.5 pages use palette #0 ... #3.

        At Level 2.5 palette #2 and #3 are redefinable, except
        color_map[8] which is "transparent" color
        (TTX_TRANSPARENT_BLACK). At Level 3.5 palette #0 and #1
        are also redefinable.

        Palette #4 never changes and contains internal colors for
        navigation bars, search text highlighting etc.
        These colors are roughly the same as the default palette #0,
        see ttx_color.
      */
    u32            color_map[40];
} ttx_extension_t;

/*!
  RAW page
  */
typedef struct _ttx_raw
{
    ttx_page_priority_t priority;
    u32                 error;

    ttx_page_function_t function;

    s16                 page_no;
    s16                 sub_no;
    u8                  national;
    u16                 control_bits;
    MT_BOOL                 charset_code_set;
    MT_BOOL                 national_set;
    MT_BOOL                 continued;
    s32                 lop_packets;
    s32                 x26_designations;
    s32                 x27_designations;
    s32                 x28_designations;

    struct _ttx_raw     *p_next;
    struct _ttx_raw     *p_next_buf;
    /*!
        ttx_lop_t                       1329

        enh_lop                         2169
            ttx_lop_t           1329
            ttx_triplet_t       836
            s32                 4

        ext_lop                         2217
            ttx_lop_t           1329
            ttx_triplet_t       836
            s32                 4
            ttx_extension_t     48

        gpop                            2224

        gdrcs                           2936
    */
    union
    {
        ttx_lop_t           unknown;

        ttx_lop_t           lop;

        /*!
          Level one page with X/26 page enhancements.
          */
        struct
        {
            ttx_lop_t       lop;
            ttx_triplet_t   trip[16 * 13 + 1];
            s32             trip_no;
        } enh_lop;

        /*!
            Level one page with X/26 page enhancements
            and X/28 extensions for Level 2.5 / 3.5.
          */
        struct
        {
            ttx_lop_t       lop;
            ttx_triplet_t   trip[16 * 13 + 1];
            s32             trip_no;
            ttx_extension_t ext;
        } ext_lop;

        /*!
          (Global) public object page.
          */
        struct
        {
            /*!
                12 * 2 triplet pointers from packet 1 ... 4.
                Valid range 0 ... 506 (39 packets * 13 triplets),
                unused pointers 511 (10.5.1.2), broken -1.
              */
            u16             pointer[4 * 12 * 2];

            /*!
                13 triplets from each of packet 3 ... 25 and
                26/0 ... 26/15.

                Valid range of mode 0x00 ... 0x1F, broken -1.
              */
            ttx_triplet_t triplet[39 * 13 + 1];
        } gpop, pop;

        /*!
            (Global) dynamically redefinable characters
            download page.
          */
#if TTX_SUPPORT_DRCS
        struct
        {
            u8              chars[TTX_DRCS_PTUS_PER_PAGE][12 * 10 / 2];
            u8              mode[TTX_DRCS_PTUS_PER_PAGE];

            /*!
                1 << (0 ... (DRCS_PTUS_PER_PAGE - 1)).

                Note characters can span multiple successive PTUs,
                see get_drcs_data().
              */
            long long       invalid;
        } gdrcs, drcs;
#endif
    } data;
} ttx_raw_t;

/*!
  magazine
  */
typedef struct
{
    /*!
      Default extension.
      */
    ttx_extension_t extension;

    /*!
       Converts page number to index into pop_link[] for default
       object invocation. Valid range 0 ... 7, -1 if broken.
      */
    s8              pop_lut[0x100];

    /*!
       Converts page number to index into drcs_link[] for default
       object invocation. Valid range 0 ... 7, -1 if broken.
      */
    s8              drcs_lut[0x100];

    /*!
       Level 2.5 [0] or 3.5 [1], one global [0] and seven local links
       to POP page. NO_PAGE(pop_link[][].pgno) == TRUE if the link
       is unused or broken.
      */
    ttx_pop_link_t  pop_link[2][8];

    /*!
       Level 2.5 [0] or 3.5 [1], one global [0] and seven local links
       to DRCS page. NO_PAGE(drcs_link[][]) == TRUE if the link
       is unused or broken.
      */
    s16             drcs_link[2][8];

    ttx_raw_t       *p_raw;
    ttx_raw_t       *p_cache;
} ttx_magazine_t;

/*!
  RAW inventory
  */
typedef struct
{
    u8  cached;
    u8  priority;

    /*!
      Actually ttx_page_type.
      */
    u8  page_type;

    /*!
      Actually ttx_ttx_charset_code, 0xFF if unknown.
      */
    u8  charset_code;

} ttx_raw_inventory_t;

/*!
  message
  */
typedef enum
{
    TTX_NOTIFY_RECEIVED_PAGE,
    TTX_NOTIFY_TIME_UPDATE,
    TTX_NOTIFY_MAX
} ttx_notify_msg_t;

/*!
  ABC
  */
typedef u32
(*ttx_notify_t)(ttx_notify_msg_t msg, ulong p1, ulong p2, void *p_context);

/*!
  ABC
  */
typedef u8  (*ttx_national_subset_cb_fun_t)(u8 , u8, MT_BOOL, MT_BOOL);

/*!
  subtitle decoder
  */
typedef struct
{
    /*!
      ABC
      */
    ttx_dec_level_t     max_level;
    /*!
      ABC
      */
    u32                 max_page_num;
    /*!
      ABC
      */
    u32                 max_sub_page_num;
    /*!
      ABC
      */
    u32                 mag_max_page_no;
    /*!
      ABC
      */
    u16                 mag_max_page_section;
    /*!
      ABC
      */
    MT_BOOL                is_running;
    /*!
      ABC
      */
    MT_BOOL                is_display;
    /*!
      ABC
      */
    MT_BOOL                is_buf_update;
    /*!
      ABC
      */
    MT_BOOL                is_subtitle_mode;       ///TODO:
    /*!
      ABC
      */
    MT_BOOL                is_display_subtitle;    ///TODO:
    /*!
      ABC
      */
    ttx_raw_t           *p_cache;
    /*!
      ABC
      */
    ttx_page_link_t     initial_page;
    /*!
      ABC
      */
    ttx_page_link_t     display_page;
    /*!
      ABC
      */
    ttx_page_link_t     waiting_page;
    /*!
      for selete page
      */
    u16                 input_page_no;
    /*!
      incoming page from the ts
      */
    u16                 incoming_page_no;
    /*!
      ABC
      */
    ttx_magazine_t      def_mag;
    /*!
      ABC
      */
    ttx_magazine_t      mag[8];
    /*!
      ABC
      */
    ttx_raw_t           *p_raw_page_buf;

    /*!
      ABC
      */
    ttx_raw_t           *p_free_raw_buf;
    /*!
      ABC
      */
    ttx_raw_t           *p_raw_sub;
    /*!
      ABC
      */
    ttx_raw_t           *p_free_raw_sub_buf;
    /*!
      ABC
      */
    ttx_raw_inventory_t inventory[800];
    /*!
      ABC
      */
    u32                 raw_cnt;
    /*!
      ABC
      */
    u16                 recycle;

    /*!
      ABC
      */
    ttx_notify_t        notify_func;
    /*!
      ABC
      */
    void                *p_context;

    /*!
      ABC
      */
    const ttx_font_descr_t *p_font_descr;
    /*!
      the last showed page
      */
    u16 last_page;
    /*!
      the last showed sub page
      */
    u16 last_sub;

    /*!
     To check waiting page is coming or not
    */
    MT_BOOL is_waiting_page_coming;
    /*!
      ABC
      */
    ttx_national_subset_cb_fun_t set_national_char_cb;
} ttx_decoder_t;

/*!
  reset decoder

  \param[in] p_dec
  */
vbi_rc_t ttx_dec_reset_vsb(ttx_decoder_t *p_dec);

/*!
  start decoder

  \param[in] func
  \param[in] p_context
  */
vbi_rc_t ttx_dec_start_vsb(ttx_decoder_t *p_dec
                           , ttx_notify_t p_func, ulong cb_addr, void *p_context);

/*!
  stop decoder

  \param[in] p_dec
  */
vbi_rc_t ttx_dec_stop_vsb(ttx_decoder_t *p_dec);

/*!
  convert page from one function to other function

  \param[in] p_page
  \param[in] new_function
  */
vbi_rc_t ttx_dec_convert_page_vsb(ttx_decoder_t *p_dec
                                  , ttx_raw_t *p_page, ttx_page_function_t new_function);

/*!
  decoder teletext data

  \param[in] p
  \param[in] type
  */
vbi_rc_t ttx_dec_data_process_vsb(ttx_decoder_t *p_dec
                                  , u8 *p_data, u8 type);

#endif
