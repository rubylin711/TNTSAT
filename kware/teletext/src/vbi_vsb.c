/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "string.h"
#include "mt_type.h"
#include "sys_define.h"
#include "mt_common.h"
//#include "hal_base.h"
//#include "hal_dma.h"
//#include "hal_misc.h"
//#include "mtos_sem.h"
//#include "mtos_mem.h"
//#include "mtos_printk.h"
//#include "mtos_misc.h"
//#include "mtos_task.h"
//#include "mtos_msg.h"
//#include "mem_manager.h"
//#include "class_factory.h"
//#include "drv_dev.h"
//#include "drv_misc.h"
//#include "common.h"
//#include "lib_rect.h"
//#include "dmx.h"
//#include "vbi_inserter.h"
//#include "display.h"
//#include "mdl.h"
#include "vbi_api.h"

#include "ttx_lang_vsb.h"
#include "ttx_bcd_vsb.h"
#include "ttx_hamm_vsb.h"
#include "ttx_dec_vsb.h"
#include "ttx_format_vsb.h"
#include "ttx_db_vsb.h"
#include "mt_unf_ttx.h"
#include "pthread.h"
#include "ttx_render_vsb.h"
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>

#define VBI_MSG_DEPTH               128
#define VBI_PES_BUF_SIZE            (64/2 * (KBYTES))
#define VBI_FILTER_BUF_SIZE         (512 * 188)
#define VBI_TASK_TIMEOUT            20

#define VBI_CLASS_ID  23

//#define DUMP_TTX_DATA_FLAG
#ifdef DUMP_TTX_DATA_FLAG
#include "ff.h"
#include "fsioctl.h"
#include "ufs.h"
#include "mt_debug.h"
#endif

//#define DMX_INTERFACE
#ifdef __LINUX__
#define DMX_INTERFACE
#endif


#define VBI_MAKE_U16(high, low)     ((((u16)(high) << 8) & 0xff00) + (low))

//#define DRV_VBI_DEBUG
#ifdef DRV_VBI_DEBUG
#define VBI_PRINTF    OS_PRINTF
#else
#ifdef WIN32
#define VBI_PRINTF
#else
#define VBI_PRINTF  DUMMY_PRINTF
#endif
#endif

/*!
    the status of PES comprise
  */
typedef enum
{
    VBI_PES_PKT_INIT,
    VBI_PES_PKT_START,
    VBI_PES_PKT_MID,
    VBI_PES_PKT_ERR,
} vbi_pes_status_t;



extern const u8 _vbi_bit_reverse_vsb[256];
extern const u8 _vbi_hamm8_fwd_vsb[16];
extern const s8 _vbi_hamm8_inv_vsb[256];
extern const u8 _vbi_hamm24_fwd_0_vsb[256];
extern const u8 _vbi_hamm24_fwd_1_vsb[256];
extern const u8 _vbi_hamm24_fwd_2_vsb[4];
extern const s8 _vbi_hamm24_inv_par_vsb[3][256];
extern const u8 _vbi_hamm24_inv_d1_d4_vsb[64];
extern const s32 _vbi_hamm24_inv_err_vsb[64];

extern u32 g_inital_page;

static ttx_font_size_t pre_font_size = TTX_FONT_NORMAL;
#ifdef DUMP_TTX_DATA_FLAG
ufs_file_t  * p_dump_ttx_fp = NULL;
static u8 *  ttx_pes_buf_tmp = NULL;
u8 * p_ttx_pes_buf_tmp = NULL;
u32 ttx_write_cnt = 0;
u8 ttx_write_flag = 0;
extern unsigned short   *  Convert_Utf8_To_Unicode(unsigned char * putf8, unsigned short * out);
#endif
static pthread_mutex_t render_mutex = PTHREAD_MUTEX_INITIALIZER;
/*!
   TODO: this function is odd parity decode

   \param[in] c
  */
static inline s32 vbi_unpar8_vsb(u32 c)
{
    if((_vbi_hamm24_inv_par_vsb[0][(u8)c] & 32) != 0)
    {
        return c & 127;
    }
    else
    {
        /*!
           The idea is to OR results together to find a parity
           error in a sequence, rather than a test and branch on
           each byte.
          */
        return -1;
    }
}

typedef struct dvb_vbi
{
    s32             callback_msg_id;
    s16             msg_cnt;

    ttx_decoder_t           *p_ttx_dec;

    ttx_osd_page_t          *p_osd_page;
    MT_BOOL                    is_ttx_pause;

    video_std_t         vid_std;
    u16                 pid;

    u8                  *p_pes_buf;
    u8                  *p_dmx_buf;
    void                *p_rgn;//ui create the osd region handle
	MT_UNF_TTX_CB_FN pfnCB;

    u8 *p_pal_font;
    u8 *p_ntsl_font;
    u8 *p_small_font;
    u8 *p_hd_font;
    MT_BOOL  is_big_mem;
    ttx_region_pos_t region_pos;
    u8 ts_in;
    u32 insertion;
    ttx_render_vsb_t            *p_ttx_render;
    u8 page_draw;
} dvb_vbi_vsb_t;

/*!
  command for VBI
  */
typedef enum
{
    VBI_CMD_START_TTX,
    VBI_CMD_STOP_TTX,
    VBI_CMD_TTX_SHOW,
    VBI_CMD_TTX_HIDE,
    VBI_CMD_TTX_PAUSE,
    VBI_CMD_TTX_RESUME,
    VBI_CMD_TTX_KEY,

    VBI_CMD_START_INSER,
    VBI_CMD_STOP_INSER,

    VBI_CMD_SET_PID,
    VBI_CMD_SET_VIDEO_STD,
    VBI_CMD_SET_FONT_SIZE,
    VBI_CMD_SET_LANGUAGE_CODE,
} vbi_cmd_t;

static dvb_vbi_vsb_t *p_dvb_vbi_handle = NULL;

RET_CODE vbi_set_callback(MT_UNF_TTX_CB_FN pfnCB);


static dvb_vbi_vsb_t *dvb_vbi_get_handle(void)
{
    return p_dvb_vbi_handle;
}
static void dvb_vbi_set_handle(dvb_vbi_vsb_t *handle)
{
    p_dvb_vbi_handle = handle;
}



static const u8 bit_swap_vsb[256] =
{
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

static MT_BOOL ttx_is_same_clock_vsb(u8 *p_new_header, u8 *p_old_header)
{
    s32 i = 0;

    for(i = 0; i < 8; p_new_header ++, p_old_header ++, i ++)
    {
        if(*p_new_header != *p_old_header
                && (vbi_unpar8_vsb(*p_new_header) | vbi_unpar8_vsb(*p_old_header)) >= 0)
            return FALSE;
    }

    return TRUE;
}

static void ttx_set_display_page_vsb(dvb_vbi_vsb_t *p_dvb_vbi
                                     , ttx_raw_t *p_in_page, u16 page_no, u16 sub_no)
{
    vbi_rc_t      rc;
    ttx_decoder_t *p_ttx_dec = p_dvb_vbi->p_ttx_dec;
    ttx_raw_t *p_page = NULL;


    //OS_PRINTF("ttx_set_display_page_vsb 0x%x  0x%x \n",page_no, sub_no);
    if(p_ttx_dec == NULL || !p_ttx_dec->is_display || !p_dvb_vbi->p_osd_page || !p_dvb_vbi->p_ttx_render)
        return;

    p_ttx_dec->waiting_page.page_no = page_no;
    p_ttx_dec->waiting_page.sub_no  = sub_no;

    if(p_in_page == NULL)
    {
        rc = ttx_get_raw_page_vsb(p_ttx_dec, page_no, sub_no, 0xffff, &p_page);

        if(rc != VBI_RC_SUCCESS)
        {
            if(p_ttx_dec->is_subtitle_mode != TRUE)
            {
                p_ttx_dec->is_display_subtitle = FALSE;
                ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                            , page_no, sub_no, FALSE);
                ttx_add_navigation_bar_vsb(p_dvb_vbi->p_osd_page, page_no);
                if((p_dvb_vbi->p_osd_page->rows > 0) && (p_dvb_vbi->p_osd_page->columns > 0))
                {
                    ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render,
                                             p_dvb_vbi->p_ttx_dec->is_subtitle_mode
                                             , p_dvb_vbi->p_osd_page
                                             , TTX_ROWS - 1, TTX_ROWS
                                             , 0, TTX_COLUMNS
                                             , FALSE, FALSE);
                }
            }
            return;
        }
    }
    else
    {
        p_page = p_in_page;
    }

    if((p_page->control_bits & C6_SUBTITLE ||p_page->control_bits & C5_NEWSFLASH) != 0)
    {
        if(p_ttx_dec->is_display_subtitle != TRUE)
            ttx_render_clear_page_vsb(p_dvb_vbi->p_ttx_render, SCREEM_COLOR_INDEX);
        else if(p_ttx_dec->input_page_no == 0xffff)
            ttx_render_clear_header_vsb(p_dvb_vbi->p_ttx_render
                                        , SCREEM_COLOR_INDEX);
        else if((p_page->control_bits & C6_SUBTITLE) && (p_page->control_bits & C4_ERASE_PAGE)) //for bug8591(redmine)
        {
            ttx_render_clear_page_vsb(p_dvb_vbi->p_ttx_render, SCREEM_COLOR_INDEX);            
        }

        ttx_osd_page_format_vsb(p_ttx_dec
                                , p_dvb_vbi->p_osd_page, p_page, FALSE, TRUE);

        ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render, p_dvb_vbi->p_ttx_dec->is_subtitle_mode
                                 , p_dvb_vbi->p_osd_page
                                 , 1, p_dvb_vbi->p_osd_page->rows
                                 , 0, p_dvb_vbi->p_osd_page->columns
                                 , FALSE, FALSE);
        p_ttx_dec->is_display_subtitle = TRUE;
    }
    else
    {
        if(p_ttx_dec->is_display_subtitle == TRUE)
            ttx_render_clear_page_vsb(p_dvb_vbi->p_ttx_render, SCREEM_COLOR_INDEX);

        ttx_osd_page_format_vsb(p_ttx_dec
                                , p_dvb_vbi->p_osd_page, p_page, FALSE, TRUE);

        if(p_ttx_dec->is_subtitle_mode == TRUE)
            return;

        if(p_ttx_dec->input_page_no == 0xffff
                || (p_ttx_dec->input_page_no & 0xfff) == p_page->page_no)
        {
            ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render, FALSE
                                     , p_dvb_vbi->p_osd_page
                                     , 0, p_dvb_vbi->p_osd_page->rows
                                     , 0, p_dvb_vbi->p_osd_page->columns
                                     , FALSE, FALSE);
        }
        else
        {
            ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render, FALSE
                                     , p_dvb_vbi->p_osd_page
                                     , 0, 1, 8, p_dvb_vbi->p_osd_page->columns
                                     , FALSE, FALSE);

            ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render, FALSE
                                     , p_dvb_vbi->p_osd_page
                                     , 1, p_dvb_vbi->p_osd_page->rows
                                     , 0, p_dvb_vbi->p_osd_page->columns
                                     , FALSE, FALSE);
        }

        p_ttx_dec->is_display_subtitle = FALSE;
    }

    p_ttx_dec->display_page.page_no = p_page->page_no;
    p_ttx_dec->display_page.sub_no  = p_page->sub_no;
    p_ttx_dec->waiting_page.page_no = p_page->page_no;
    p_ttx_dec->waiting_page.sub_no  = p_page->sub_no;
    //p_page->priority = TTX_PAGE_PRI_1;
    return;
}

#if 0
void ttx_get_window_size(unsigned short *pWidth, unsigned short *pHeight)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;
    ttx_render_vsb_t *p_ttx_render = NULL;
    ttx_font_size_t font_size;

    p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi == NULL)
        return;

    p_ttx_render = p_dvb_vbi->p_ttx_render;
    font_size = p_ttx_render->font_size;

    if(font_size == TTX_FONT_HD)
    {
        *pWidth = TTX_PAGE_W_HD;
        *pHeight = TTX_PAGE_H_HD;
    }
    else if(font_size == TTX_FONT_SMALL)
    {
        *pWidth = TTX_PAGE_W_SMALL;
        *pHeight = TTX_PAGE_H_SMALL;
    }
    else if(font_size == TTX_FONT_NORMAL)
    {
        if(p_ttx_render->video_std == VID_STD_PAL)
        {
            *pWidth = TTX_PAGE_W_PAL;
            *pHeight = TTX_PAGE_H_PAL;
        }
        else
        {
            *pWidth = TTX_PAGE_W_NTSC;
            *pHeight = TTX_PAGE_H_NTSC;
        }
    }

    return;

}
#endif

static unsigned long long mt_get_time_ms( void )
{
    struct timeval tv;
    unsigned long long time_ms = 0ULL;

    gettimeofday(&tv, NULL);

    time_ms  = (unsigned long long)tv.tv_sec * 1000;
    time_ms += (unsigned long long)tv.tv_usec / 1000;
    return (time_ms);
}

#if 0
static u64 mt_get_time_interval_ms(u64 after, u64 before)
{
   u64 diff = 0;
	 if (after >= before)
   {
     diff = ((after - before));
	 }
	 else
   {
     diff = ((UINT64_MAX - (before - after) + 1));
   }	
   return diff;
}
#endif

static u32 ttx_dec_notify_vsb(ttx_notify_msg_t msg
                              , ulong p1, ulong p2, void *p_context)
{
    static unsigned long long old_ticks = 0;
    //static unsigned long long old_ticks_draw = 0;
    //unsigned long long        ticks_delta     = 0;
    //unsigned long long        ticks_diff      = 0;
    ttx_raw_t   *p_current_page = NULL;
    s32         i   = 0;
    s32         raw = 0;
    vbi_rc_t    rc;
    
    static u32 flash_cnt = 0;
    
    u32 cur_tick = 0;    
    u8  flash_line_begin = 0;
    u8  flash_line_end = 0;
    
    if(p_context == NULL)
    {
        return 0;
    }
    
    dvb_vbi_vsb_t       *p_dvb_vbi      = (dvb_vbi_vsb_t *)p_context;
    ttx_decoder_t   *p_ttx_dec      = p_dvb_vbi->p_ttx_dec;

    if(p_ttx_dec == NULL)
    {
        return 0;
    }
    
    ttx_page_link_t *p_waiting_page = &p_ttx_dec->waiting_page;
    ttx_page_link_t *p_display_page = &p_ttx_dec->display_page;

    if(msg == TTX_NOTIFY_TIME_UPDATE)
    {
        ttx_page_link_t *p_link = (ttx_page_link_t *)p1;
        u8              *p_data = (u8 *)p2;

        pthread_mutex_lock(&render_mutex);

        if(p_ttx_dec->is_display != TRUE)
        {
        	pthread_mutex_unlock(&render_mutex);
            return 0;
        }
		if(p_ttx_dec->is_display_subtitle)//fix bug 109874
		{
            pthread_mutex_unlock(&render_mutex);
			return 0;        
		}
        if(p_link) //add for fixing bug 18731  
        {
            if((p_link->control_bit & C7_SUPPRESS_HEADER) || (p_link->control_bit & C9_INTERRUPTED)
                ||((p_link->page_no &0x00ff) == 0x00ff)||((p_link->sub_no &0x3f7f) == 0x3f7f))
            {
                pthread_mutex_unlock(&render_mutex);
                return 0;
            }
        }        
        if(p_display_page->page_no == p_waiting_page->page_no
                && p_display_page->sub_no == p_waiting_page->sub_no
                /*&& (p_link->page_no & 0xf00) == (p_display_page->page_no & 0xf00)*//*remove for bug 18731*/)
        {
#if 0  //remove code for fixing bug 18731
            ticks_delta = mt_get_time_ms();
            ticks_diff = mt_get_time_interval_ms(ticks_delta, old_ticks);
            if(ticks_diff < 500ULL)
            {
                pthread_mutex_unlock(&render_mutex);
                return 0;
            }
#endif
            rc = ttx_get_raw_page_vsb(p_ttx_dec
                                      , p_ttx_dec->display_page.page_no
                                      , p_ttx_dec->display_page.sub_no
                                      , 0xffff
                                      , &p_current_page);

            //fix bug 54694&54570
            if(rc != VBI_RC_SUCCESS)
            {
                pthread_mutex_unlock(&render_mutex);
                return 0;
            }
            if(p_current_page == NULL)
            {
                MT_ASSERT(0);
            }
            //Ryan add , do nothing when news flash come
            if((p_current_page->control_bits & C5_NEWSFLASH) != 0)
            {
                pthread_mutex_unlock(&render_mutex);
                return 0;
            }

            //old_ticks = ticks_delta;
            if(rc == VBI_RC_SUCCESS
                  && (ttx_is_same_clock_vsb(&p_data[32]
                                              , &p_current_page->data.lop.raw[0][32])) != TRUE) 
            {
                for(i = 32; i < p_dvb_vbi->p_osd_page->columns; i ++)
                {
                    p_current_page->data.lop.raw[0][i]
                        = p_data[i];

                    if((raw = vbi_unpar8_vsb(p_data[i])) > 0x1F)
                        p_dvb_vbi->p_osd_page->text[i].unicode = raw;
                }

                ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render,
                                         p_dvb_vbi->p_ttx_dec->is_subtitle_mode
                                         , p_dvb_vbi->p_osd_page
                                         , 0, 1, 32, p_dvb_vbi->p_osd_page->columns
                                         , FALSE, FALSE);
            }
            cur_tick = mt_get_time_ms();
            if(cur_tick - old_ticks > 500) //add for bug 18543
            {
                flash_line_begin = flash_line_end = 0;
                ttx_get_flash_line_pos(p_dvb_vbi->p_osd_page,&flash_line_begin,&flash_line_end);
                if((flash_line_begin != 0) && (flash_line_end != 0))
                {
                    ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render,0,p_dvb_vbi->p_osd_page,
                        flash_line_begin,flash_line_end, 0, p_dvb_vbi->p_osd_page->columns
                                 , FALSE, (((flash_cnt & 0x00000001)== 0)?TRUE:FALSE));                           

                }
                flash_cnt++;
                old_ticks = cur_tick;
            }            
        }
        pthread_mutex_unlock(&render_mutex);
        return 0;
    }

    if(msg == TTX_NOTIFY_RECEIVED_PAGE)
    {
        ttx_raw_t *p_page = (ttx_raw_t *)p1;
        if(NULL == p_page)
        {
            return 0;
        }
				pthread_mutex_lock(&render_mutex);
        if(p_ttx_dec->is_display != TRUE)
        {
        	pthread_mutex_unlock(&render_mutex);
            return 0;
        }
        if(p_waiting_page->page_no == p_page->page_no
                && (p_waiting_page->sub_no == p_page->sub_no
                    || p_waiting_page->sub_no == TTX_NULL_SUBPAGE
                    || p_waiting_page->sub_no == TTX_ANY_SUBPAGE
                    || p_waiting_page->sub_no == TTX_FIRST_SUBPAGE))
        {
            //fix bug 6242, the first display subpage should be index1.
            //Index0 stands for no subpage
            //use ttx_get_buf to store the subpage in order
            if(((p_page->control_bits & (C8_UPDATE | C4_ERASE_PAGE)) != 0)
                    || ((p_display_page->page_no != p_waiting_page->page_no))
                    || ((p_display_page->page_no == p_waiting_page->page_no)
                        && (p_display_page->sub_no != p_waiting_page->sub_no)))
            {
                //   OS_PRINTF("p_page->control_bits 0x%x, pg no %x %x  sub no %x %x\n",
                //     p_page->control_bits, p_display_page->page_no, p_waiting_page->page_no,
                //    p_display_page->sub_no , p_waiting_page->sub_no);
                ttx_set_display_page_vsb(p_dvb_vbi, p_page,
                                         p_waiting_page->page_no, p_waiting_page->sub_no);
            }
						pthread_mutex_unlock(&render_mutex);
            return 0;
        }
      	else if((p_page->page_no == p_waiting_page->page_no) && (p_page->sub_no != p_waiting_page->sub_no) && (p_display_page->manual_flag == 0)
          && !p_ttx_dec->is_display_subtitle)//fix bug112826, bug110791
    	  {
                ttx_set_display_page_vsb(p_dvb_vbi, p_page,
                                         p_page->page_no, p_page->sub_no);
								pthread_mutex_unlock(&render_mutex);

		        return 0;
        }
        else if(((p_page->control_bits & (C5_NEWSFLASH | C6_SUBTITLE
                                          | C7_SUPPRESS_HEADER | C9_INTERRUPTED | C10_INHIBIT_DISPLAY))
                 == 0)
                && (!p_ttx_dec->is_display_subtitle))
        {
            if(p_display_page->page_no != p_waiting_page->page_no
                    || p_display_page->sub_no != p_waiting_page->sub_no)
            {
                /*    roll header    */
                ttx_osd_page_format_vsb(p_ttx_dec
                                        , p_dvb_vbi->p_osd_page, p_page, TRUE, FALSE);
                ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render, FALSE
                                         , p_dvb_vbi->p_osd_page
                                         , 0, 1, 8, p_dvb_vbi->p_osd_page->columns
                                         , FALSE, FALSE);
            }
            else if(1 &&((((p_page->control_bits & C13_PARTIAL_PAGE) == 0)
                         || ((p_ttx_dec->display_page.page_no & 0xf00)
                             == (p_page->page_no & 0xf00))) || ((p_page->page_no & 0xF) == 0)))
            {
#if 0                
                /*    update time    */
                ticks_delta = mt_get_time_ms();
                ticks_diff = mt_get_time_interval_ms(ticks_delta, old_ticks_draw);
                if(ticks_diff < 500ULL)
                {
                	pthread_mutex_unlock(&render_mutex);
                    return 0;
                }
                else
                {
                    old_ticks_draw = ticks_delta;
                }

                rc = ttx_get_raw_page_vsb(p_ttx_dec
                                          , p_ttx_dec->display_page.page_no
                                          , p_ttx_dec->display_page.sub_no
                                          , 0xffff
                                          , &p_current_page);
                if(rc == VBI_RC_SUCCESS
                        && (ttx_is_same_clock_vsb(&p_page->data.lop.raw[0][32]
                                                  , &p_current_page->data.lop.raw[0][32])) != TRUE)
                {
                    for(i = 32; i < p_dvb_vbi->p_osd_page->columns; i ++)
                    {
                        p_current_page->data.lop.raw[0][i]
                            = p_page->data.lop.raw[0][i];

                        if((raw = vbi_unpar8_vsb(p_page->data.lop.raw[0][i]))
                                > 0x1F)
                            p_dvb_vbi->p_osd_page->text[i].unicode = raw;
                    }

                    ttx_render_draw_page_vsb(p_dvb_vbi->p_ttx_render, FALSE
                                             , p_dvb_vbi->p_osd_page
                                             , 0, 1, 32, p_dvb_vbi->p_osd_page->columns
                                             , FALSE, FALSE);

                }
#endif                
            }
  
		    pthread_mutex_unlock(&render_mutex);

            return 0;
        }
        else if((p_display_page->page_no == TTX_NULL_PAGE_NO) //fix bug 109874,
          && (!p_ttx_dec->is_display_subtitle)
          && (!(p_page->control_bits & C6_SUBTITLE 
          || p_page->control_bits & C5_NEWSFLASH))) //fix bug 116245, 116626, 123136          
				{
						if(g_inital_page)
							ttx_set_display_page_vsb(p_dvb_vbi, p_page,
	                                         p_ttx_dec->incoming_page_no, TTX_FIRST_SUBPAGE);

				}		
				pthread_mutex_unlock(&render_mutex);
    }

    return 0;
}

static void ttx_key_proc_vsb(dvb_vbi_vsb_t *p_dvb_vbi, u32 key)
{
    vbi_rc_t  rc = VBI_RC_SUCCESS;
    MT_BOOL is_big_mem = vbi_is_big_mem();
    ttx_decoder_t *p_ttx_dec = p_dvb_vbi->p_ttx_dec;

    switch(key)
    {
    case TTX_KEY_PAGE_UP:
    case TTX_KEY_PAGE_DOWN:
    {
        u16         new_page_no = 0;
        u16         cur_page_no = 0;
        MT_BOOL        b_reset = FALSE;
				//fix bug 108790
				if(g_inital_page == 0)
					break;

        if(p_ttx_dec == NULL)
        {
            break;
        }
        cur_page_no = p_ttx_dec->waiting_page.page_no;

        if(key == TTX_KEY_PAGE_UP)
        {
            ttx_get_next_page_up_vsb(p_ttx_dec, cur_page_no, &(p_ttx_dec->input_page_no));
            new_page_no = p_ttx_dec->input_page_no;
        }
        else
        {
            ttx_get_prev_page_down_vsb(p_ttx_dec, cur_page_no, &(p_ttx_dec->input_page_no));
            new_page_no = p_ttx_dec->input_page_no;
        }

        b_reset = ttx_buf_flush_vsb(p_ttx_dec, cur_page_no, new_page_no);
        if(TRUE == b_reset)
        {
            //different page section
            //ttx_dec_reset_vsb(p_ttx_dec);
        }
        p_ttx_dec->is_buf_update = TRUE;
        p_ttx_dec->input_page_no = new_page_no;
	p_ttx_dec->display_page.manual_flag = 0;

	ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                    , new_page_no, 0, FALSE);

        ttx_set_display_page_vsb(p_dvb_vbi
                                 , NULL, new_page_no, TTX_FIRST_SUBPAGE);
        break;
    }
    case TTX_KEY_UP:
    case TTX_KEY_DOWN:
    {
        u16         new_page_no = 0;
        u16         cur_page_no = 0;
        u16         cur_page_section = 0;
        MT_BOOL        b_reset = FALSE;
				//fix bug 108790
				if(g_inital_page == 0)
					break;

        if(p_ttx_dec == NULL)
        {
            break;
        }

        cur_page_no = p_ttx_dec->waiting_page.page_no;
        rc = ttx_get_page_section_vsb(p_ttx_dec, cur_page_no, &cur_page_section);

        if(key == TTX_KEY_UP)
        {
            if(is_big_mem)
                ttx_get_next_raw_page_no_vsb(p_ttx_dec, cur_page_no, &(p_ttx_dec->input_page_no));
            else
                ttx_get_next_page_no_vsb(p_ttx_dec, cur_page_no, &(p_ttx_dec->input_page_no));
            new_page_no = p_ttx_dec->input_page_no;
        }
        else
        {
            if(is_big_mem)
                ttx_get_prev_raw_page_no_vsb(p_ttx_dec, cur_page_no, &(p_ttx_dec->input_page_no));
            else
                ttx_get_prev_page_no_vsb(p_ttx_dec, cur_page_no, &(p_ttx_dec->input_page_no));
            new_page_no = p_ttx_dec->input_page_no;
        }

	 if(new_page_no != cur_page_no)
	 {
        b_reset = ttx_buf_flush_vsb(p_ttx_dec, cur_page_no, new_page_no);
        if(TRUE == b_reset)
        {
            //different page section
            //ttx_dec_reset_vsb(p_ttx_dec);
        }
        p_ttx_dec->is_buf_update = TRUE;
        p_ttx_dec->input_page_no = new_page_no;
	 p_ttx_dec->display_page.manual_flag = 0;

	ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                    , new_page_no, 0, FALSE);

        ttx_set_display_page_vsb(p_dvb_vbi
                                 , NULL, new_page_no, TTX_FIRST_SUBPAGE);
	 }
        break;
    }
    case TTX_KEY_RIGHT:
    case TTX_KEY_LEFT:
    {
        ttx_raw_t *p_page = NULL;
        //fix bug6242 subpage increase in order
//        u8  cur_in_decimal = 0;
        //      u8  next_in_decimal = 0;
        //      u8  prev_in_decimal = 0;
//       s16 sub_no = 0;

        //     sub_no = p_ttx_dec->waiting_page.sub_no;
        //      cur_in_decimal = ((sub_no & 0xF0) >> 4) * 10
        //                      + ((sub_no & 0xF));
				//fix bug 108790
				if(g_inital_page == 0)
					break;

        if(p_ttx_dec == NULL)
        {
            break;
        }

        if(key == TTX_KEY_LEFT)
        {
            p_ttx_dec->is_buf_update = TRUE;
            p_ttx_dec->input_page_no = p_ttx_dec->waiting_page.page_no;
            rc = ttx_get_prev_raw_subpage_vsb(p_ttx_dec
                                              , p_ttx_dec->waiting_page.page_no
                                              , p_ttx_dec->waiting_page.sub_no
                                              , 0xffff
                                              , &p_page);
            if(rc != VBI_RC_SUCCESS)
            {
                return;
            }
            if(NULL == p_page)
            {
                if(p_ttx_dec->display_page.manual_flag == 0)
                {
                    ttx_set_display_page_vsb(p_dvb_vbi
                                             , NULL, p_ttx_dec->display_page.page_no, p_ttx_dec->display_page.sub_no);
                }
                return;
            }
            //         sub_no = p_page->sub_no;
            //         prev_in_decimal = ((sub_no & 0xF0) >> 4) * 10
            //                           + ((sub_no & 0xF));

            //not support over roll now
            //            if(sub_no != p_ttx_dec->waiting_page.sub_no)
            ttx_set_display_page_vsb(p_dvb_vbi, NULL, p_ttx_dec->waiting_page.page_no, p_page->sub_no);
        }
        else
        {
            p_ttx_dec->is_buf_update = TRUE;
            p_ttx_dec->input_page_no = p_ttx_dec->waiting_page.page_no;
            rc = ttx_get_next_raw_subpage_vsb(p_ttx_dec
                                              , p_ttx_dec->waiting_page.page_no
                                              , p_ttx_dec->waiting_page.sub_no
                                              , 0xffff
                                              , &p_page);
            if(rc != VBI_RC_SUCCESS)
            {
                return;
            }
            if(NULL == p_page)
            {
                if(p_ttx_dec->display_page.manual_flag == 0)
                {
                    ttx_set_display_page_vsb(p_dvb_vbi
                                             , NULL, p_ttx_dec->display_page.page_no, p_ttx_dec->display_page.sub_no);
                }
                return;
            }
            //         sub_no = p_page->sub_no;
            //          next_in_decimal = ((sub_no & 0xF0) >> 4) * 10
            //                           + ((sub_no & 0xF));

            //we do not know which is the last subpage,set the first subpage if reaches last
            //this may occurs when last subpage reaches.
            //Or the next subpage packet not decoded
            //          if(sub_no != p_ttx_dec->waiting_page.sub_no)
            ttx_set_display_page_vsb(p_dvb_vbi, NULL, p_ttx_dec->waiting_page.page_no, p_page->sub_no);
        }
        break;
    }
    case TTX_KEY_RED:
    case TTX_KEY_GREEN:
    case TTX_KEY_YELLOW:
    case TTX_KEY_CYAN:
    case TTX_KEY_INDEX:
    {
        u8              index = 0;
        ttx_osd_page_t  *p_osd_page = p_dvb_vbi->p_osd_page;
				//fix bug 108790
				if(g_inital_page == 0)
					break;

        if(p_ttx_dec == NULL)
        {
            break;
        }

        index = (key == TTX_KEY_RED)
                ? 0 : (key == TTX_KEY_GREEN)
                ? 1 : (key == TTX_KEY_YELLOW)
                ? 2 : (key == TTX_KEY_CYAN)
                ? 3 : 5;

        p_ttx_dec->is_buf_update = TRUE;
        p_ttx_dec->input_page_no = p_osd_page->nav_link[index].page_no;

        if(p_osd_page->nav_link[index].page_no > 0
                && p_osd_page->nav_link[index].page_no < TTX_NULL_PAGE_NO)//fix bug5978
        {
            //ttx_dec_reset_vsb(p_ttx_dec);
            p_ttx_dec->input_page_no = p_osd_page->nav_link[index].page_no;
            p_ttx_dec->is_buf_update = TRUE;
            p_ttx_dec->display_page.manual_flag = 0;
#if 0
            ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                        , p_osd_page->nav_link[index].page_no, 0, FALSE);
#endif
            ttx_set_display_page_vsb(p_dvb_vbi
                                     , NULL
                                     , p_osd_page->nav_link[index].page_no
                                     , p_osd_page->nav_link[index].sub_no);
            //p_ttx_dec->input_page_no = 0xffff;
        }
        break;
    }
    case TTX_KEY_TRANSPARENT:
    {
        p_dvb_vbi->p_osd_page->user_screen_opacity -= 25;
        if(p_dvb_vbi->p_osd_page->user_screen_opacity < 0)
            p_dvb_vbi->p_osd_page->user_screen_opacity = 100;
        ttx_render_set_bg_transparent_vsb(p_dvb_vbi->p_ttx_render
                                          , p_dvb_vbi->p_osd_page
                                          , p_dvb_vbi->p_osd_page->user_screen_opacity);
    }
    default:
        if(key >= TTX_KEY_0 && key <= TTX_KEY_9)
        {
            u16     page_no = 0;
            u32     cnt     = 0;
            MT_BOOL    b_flush = FALSE;
						//fix bug 108790
						if(g_inital_page == 0)
							break;
            
            if(p_ttx_dec == NULL)
            {
                break;
            }

            if(0 == (p_ttx_dec->input_page_no & 0xf000))
                p_ttx_dec->input_page_no = p_ttx_dec->input_page_no | 0xf000;

            page_no = p_ttx_dec->input_page_no & 0xfff;
            cnt     = (p_ttx_dec->input_page_no & 0xf000) >> 12;

            if(cnt > 3)
                cnt = 3;

            if(cnt != 3
                    || (key != TTX_KEY_0 && key != TTX_KEY_9))
            {
                cnt --;
                page_no = (u16)((page_no & ~(0xf << (cnt * 4)))
                                | ((key - TTX_KEY_0) << (cnt * 4)));
            }

            page_no += (u16)(cnt << 12);
            p_ttx_dec->input_page_no = page_no;

            if(cnt == 0)
            {
                ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                            , page_no, 0, FALSE);
                b_flush = ttx_buf_flush_vsb(p_ttx_dec,
                                            p_ttx_dec->display_page.page_no, page_no);
                if(b_flush)
                {
                    //ttx_dec_reset_vsb(p_ttx_dec);
                    p_ttx_dec->input_page_no = page_no;
                }
                p_ttx_dec->is_buf_update = TRUE;
		  p_ttx_dec->display_page.manual_flag = 0;
                ttx_set_display_page_vsb(p_dvb_vbi
                                         , NULL, page_no, TTX_FIRST_SUBPAGE);
            }
            else
            {
                ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                            , page_no, 0, FALSE);
            }
        }
        break;
    }
    return;
}

RET_CODE vbi_pes_parse_vsb(u8 *p_pes_pkt)
{
#define PACKET_LENGTH_OFFSET        4
#define PTS_FLAG_OFFSET             7
#define PTS_FLAG_MASK               0x80
#define PTS_OFFSET                  9
#define HEADER_DATA_LENGTH_OFFSET   8

    u8              *p_pes_data_filed = NULL;
    u8              *p_vbi_data_filed = NULL;
    u8              *p_pes_pkt_last_byte = NULL;
    u16             pes_packet_length = 0;
    u8              pts_flag = 0;
    u32             pts = 0;
    u8              vbi_data_id = 0;
    u8              data_unit_id = 0;
    u8              data_unit_length = 0;

    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

//    printf("linda debug %s %d\n", __func__, __LINE__);
    p_dvb_vbi = dvb_vbi_get_handle();
 //   printf("linda debug %s %d\n", __func__, __LINE__);
    if(p_dvb_vbi == NULL)
    {
        VBI_PRINTF("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_FAILED;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    ttx_decoder_t   *p_ttx_dec  = p_dvb_vbi->p_ttx_dec;
    u32             i = 0;
//    printf("linda debug %s %d\n", __func__, __LINE__);
    if(p_pes_pkt[0] != 0x00 || p_pes_pkt[1] != 0x00
            || p_pes_pkt[2] != 0x01 || p_pes_pkt[3] != 0xbd)
    {
//        printf("linda debug %s %d\n", __func__, __LINE__);
        /*    'stream_id' can only occur at transport packet content boundary. */
        VBI_PRINTF("VBI!: Bad boundary #1.\n");
        VBI_PRINTF("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_BAD_BAOUNDARY;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    if(p_pes_pkt[8] != 0x24)
    {
 //       printf("linda debug %s %d\n", __func__, __LINE__);
        /*    'PES_header_data_length' must be 0x24    */
        VBI_PRINTF("VBI!: bad PES_header_data_length: 0x%2x\n", p_pes_pkt[8]);
        VBI_PRINTF("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_INVALID_DATA;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    p_pes_data_filed    = p_pes_pkt + 9 + 0x24;    /* point to pes_data_filed */
    p_vbi_data_filed    = p_pes_data_filed + 1;    /* point to data_unit_id   */
    pes_packet_length   = VBI_MAKE_U16(p_pes_pkt[4], p_pes_pkt[5]);
    p_pes_pkt_last_byte = p_pes_pkt + 6 + pes_packet_length;
//    printf("linda debug %s %d\n", __func__, __LINE__);

    VBI_PRINTF("p_pes_packet = %d\n", p_pes_pkt);
    VBI_PRINTF("p_pes_data_filed = %d\n", p_pes_data_filed);
    VBI_PRINTF("p_vbi_data_filed = %d\n", p_vbi_data_filed);
    VBI_PRINTF("pes_packet_length = %d\n", pes_packet_length);
    VBI_PRINTF("p_pes_packet_last_byte = %d\n", p_pes_pkt_last_byte);
//    printf("linda debug %s %d\n", __func__, __LINE__);
#if 1
    //Ryan add  , bug 59519
//    MT_USLEEP(20*1000);
#endif
//    printf("linda debug %s %d\n", __func__, __LINE__);
    vbi_data_id = p_pes_data_filed[0];
    if((vbi_data_id < 0x10)
            || (vbi_data_id > 0x1F && vbi_data_id < 0x99)
            || (vbi_data_id > 0x9B))
    {
//        printf("linda debug %s %d\n", __func__, __LINE__);
        /* 'data_identifier' was NOT for VBI. */
        VBI_PRINTF("VBI!:    bad data_identifier: 0x%2x\n", vbi_data_id);
        VBI_PRINTF("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_INVALID_DATA;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    pts_flag = p_pes_pkt[7] & PTS_FLAG_MASK;
    if(pts_flag != 0)
    {
 //       printf("linda debug %s %d\n", __func__, __LINE__);
        pts   = p_pes_pkt[PTS_OFFSET + 0] & 0x0e;
        pts <<= 7;
        pts  += p_pes_pkt[PTS_OFFSET + 1];
        pts <<= 8;
        pts  += p_pes_pkt[PTS_OFFSET + 2] & 0xfe;
        pts <<= 7;
        pts  += p_pes_pkt[PTS_OFFSET + 3];
        pts <<= 6;
        pts  += ((p_pes_pkt[PTS_OFFSET + 3] & 0xfe) >> 2);
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    if(p_ttx_dec == NULL)
    {
//        printf("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_SUCCESS;
    }
    if(p_dvb_vbi->is_ttx_pause)
    {
//        printf("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_SUCCESS;
    }
    if(p_ttx_dec == NULL || p_dvb_vbi->is_ttx_pause)
    {
//        printf("linda debug %s %d\n", __func__, __LINE__);
        return VBI_RC_SUCCESS;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    while(1)
    {
        /*
            VBI Packet Data
            ---------------
            in:        p_vbi_data_filed
                    pts
        */
  //      printf("linda debug %s %d\n", __func__, __LINE__);
        data_unit_id      = p_vbi_data_filed[0];
        data_unit_length  = p_vbi_data_filed[1];

        /*    step over the data_unit_length, point to vbi_data_filed    */
        p_vbi_data_filed += 2;
        VBI_PRINTF("VBI: data_unit_id = 0x%x, data_unit_length = %d\n"
                   , data_unit_id, data_unit_length);

        if((p_vbi_data_filed + data_unit_length) > p_pes_pkt_last_byte)
        {
            VBI_PRINTF("VBI!: Bad boundary #2.\n");
            break;
        }

        /* 0x02:    EBU Teletext non-subtitle data, transcode as EBU Teletext */
        /* 0x03:    EBU Teletext subtitle data, transcode as EBU Teletext     */
        /* 0xc0:    Inverted Teletext, transcode as EBU Teletext with an]
                    inverted framing code                                     */
        /* 0xC3:    VPS, transcode as VPS                                     */
        /* 0xC4:    WSS, transcode as WSS                                     */
        /* 0xC5:    Closed Captioning, transcode as Closed Captioning         */
        /* 0xC6:    monochrome 4:2:2 samples, transcode as raw VBI data       */
        if(1)
        {
            //whole length exclude clock-run-in 2bytes, framing code 1bytes
            u8 ttx_data[TTX_PACKET_LENGTH - 3];

            if(((data_unit_id == 0x02 || data_unit_id == 0x03)
                    && p_vbi_data_filed[1] == 0xe4)
                    || (data_unit_id == 0xc0 && p_vbi_data_filed[1] == 0x1b))
            {
                for(i = 0; i < (TTX_PACKET_LENGTH - 3); i ++)
                    ttx_data[i] = bit_swap_vsb[p_vbi_data_filed[2 + i]];

                ttx_dec_data_process_vsb(p_ttx_dec, ttx_data, data_unit_id);
#if 0
                if((p_ttx_dec->display_page.page_no != p_ttx_dec->input_page_no) &&
                        (p_ttx_dec->incoming_page_no != 0xffff) &&
                        (NULL != p_dvb_vbi->p_ttx_render) &&
                        (TRUE == p_ttx_dec->is_buf_update) &&//init state
                        (TRUE != p_ttx_dec->is_subtitle_mode))//no teletext subtitle
                {
                    ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                                , p_ttx_dec->incoming_page_no, 0, TRUE);
                }
#endif
            }
            else if(((data_unit_id == 0x02 || data_unit_id == 0x03)
                     && p_vbi_data_filed[1] == 0x27)
                    || (data_unit_id == 0xc0 && p_vbi_data_filed[1] == 0xd8))
            {
                for(i = 0; i < (TTX_PACKET_LENGTH - 3); i ++)
                    ttx_data[i] = p_vbi_data_filed[2 + i];

                ttx_dec_data_process_vsb(p_ttx_dec, ttx_data, data_unit_id);
#if 0
                if((p_ttx_dec->display_page.page_no != p_ttx_dec->input_page_no) &&
                        (p_ttx_dec->incoming_page_no != 0xffff) &&
                        (NULL != p_dvb_vbi->p_ttx_render))
                {
                    ttx_render_draw_page_no_vsb(p_dvb_vbi->p_ttx_render
                                                , p_ttx_dec->incoming_page_no, 0, TRUE);
                }
#endif
            }
        }

        p_vbi_data_filed += data_unit_length;

        if(p_vbi_data_filed >= p_pes_pkt_last_byte)
            break;
    }
//    printf("linda debug %s %d\n", __func__, __LINE__);
    return VBI_RC_SUCCESS;
}

static vbi_rc_t ttx_show_vsb(dvb_vbi_vsb_t *p_dvb_vbi, MT_BOOL is_subtitle, u32 page_no)
{
    vbi_rc_t        rc = VBI_RC_SUCCESS;
    ttx_decoder_t   *p_ttx_dec = p_dvb_vbi->p_ttx_dec;
    u32             sub_no     = TTX_FIRST_SUBPAGE;
	MT_UNF_TTX_CHAR_BUFFER_PARAM_S buf_param = {0};

    if(p_ttx_dec == NULL)
        return VBI_RC_FAILED;

    if(p_dvb_vbi->p_osd_page == NULL)
    {
        p_dvb_vbi->p_osd_page
            = malloc(sizeof(ttx_osd_page_t));
        MT_ASSERT(p_dvb_vbi->p_osd_page != NULL);
        memset(p_dvb_vbi->p_osd_page,0,sizeof(ttx_osd_page_t)); //fix bug 131318
        ttx_osd_page_init_vsb(p_dvb_vbi->p_ttx_dec
                              , p_dvb_vbi->p_osd_page);
    }

    if(p_dvb_vbi->p_ttx_render == NULL)
    {
        p_dvb_vbi->p_ttx_render
            = malloc(sizeof(ttx_render_vsb_t));

        MT_ASSERT(p_dvb_vbi->p_ttx_render != NULL);
        memset(p_dvb_vbi->p_ttx_render,0,sizeof(ttx_render_vsb_t));
        //max double size character
        if(!p_dvb_vbi->p_ttx_render->p_char_buf)
        {
        	buf_param.width = TTX_CHAR_MAX_W;
			buf_param.height = TTX_CHAR_MAX_H;

        	p_dvb_vbi->pfnCB(MT_NULL, MT_UNF_TTX_CB_CREATE_CHAR_BUFF,&buf_param);
			p_dvb_vbi->p_ttx_render->p_char_buf = buf_param.pBuf;
			p_dvb_vbi->p_ttx_render->char_buf_pitch = buf_param.pitch;
			VBI_PRINTF("\r\n p_char_buf:0x%08x", p_dvb_vbi->p_ttx_render->p_char_buf);
#if 0
            p_dvb_vbi->p_ttx_render->p_char_buf =
                (u8 *)mtos_malloc(TTX_CHAR_MAX_H * TTX_CHAR_MAX_W);
#endif
        }
        MT_ASSERT(p_dvb_vbi->p_ttx_render->p_char_buf != NULL);

        memcpy(p_dvb_vbi->p_ttx_render->clut
               , p_ttx_dec->def_mag.extension.color_map
               , sizeof(p_ttx_dec->def_mag.extension.color_map));

        memcpy(p_dvb_vbi->p_ttx_render->clut
               + sizeof(p_ttx_dec->def_mag.extension.color_map) / 4
               , p_ttx_dec->def_mag.extension.color_map
               , sizeof(p_ttx_dec->def_mag.extension.color_map));
        p_dvb_vbi->p_ttx_render->font_size = pre_font_size;
		p_dvb_vbi->p_ttx_render->pfnCB = p_dvb_vbi->pfnCB;
    }

    //fix bug6319
    if(vbi_get_region_pos() == TTX_REGION_OUT_DECODER)
        p_dvb_vbi->p_ttx_render->p_osd_hdl = p_dvb_vbi->p_rgn;

    if(p_ttx_dec->is_display)
    {
        if(p_ttx_dec->is_subtitle_mode != is_subtitle)
        {
            if(0)//p_ttx_dec->is_subtitle_mode)
            {
                ttx_render_delete_sub_region_vsb(p_dvb_vbi->p_ttx_render);
            }
            else
            {
                ttx_render_delete_region_vsb(p_dvb_vbi->p_ttx_render);
            }
        }
    }

    if(!p_ttx_dec->is_display || p_ttx_dec->is_subtitle_mode != is_subtitle)
    {
        if(0)//is_subtitle)
        {
            rc = ttx_render_create_sub_region_vsb(p_dvb_vbi->p_ttx_render
                                                  , p_dvb_vbi->vid_std);
        }
        else
        {
            rc = ttx_render_create_region_vsb(p_dvb_vbi->p_ttx_render
                                              , p_dvb_vbi->vid_std);
        }

        if (rc != VBI_RC_SUCCESS)
        {
            free(p_dvb_vbi->p_osd_page);
            p_dvb_vbi->p_osd_page = NULL;
            free(p_dvb_vbi->p_ttx_render);
            p_dvb_vbi->p_ttx_render = NULL;
            return rc;
        }
    }

    if(is_subtitle == TRUE)
    {
        // for bug 11417
        //p_dvb_vbi->p_ttx_render->page_h = p_dvb_vbi->p_ttx_render->page_h + 10;
        ttx_render_clear_page_vsb(p_dvb_vbi->p_ttx_render, SCREEM_COLOR_INDEX);
        //p_dvb_vbi->p_ttx_render->page_h = p_dvb_vbi->p_ttx_render->page_h -10;

        p_ttx_dec->is_display_subtitle = TRUE;
        sub_no                         = TTX_ANY_SUBPAGE;
        if(page_no >= 0x900)
            MT_ASSERT(0);
    }
    else
    {
        //ttx_render_clear_page_vsb(p_dvb_vbi->p_ttx_render, SCREEM_COLOR_INDEX);
        ttx_render_clear_header_vsb(p_dvb_vbi->p_ttx_render, 40 + TTX_BLACK);

        p_ttx_dec->is_display_subtitle = FALSE;
        sub_no                         = TTX_FIRST_SUBPAGE;
        if(page_no >= 0x900)
            page_no = p_ttx_dec->initial_page.page_no;
    }

    p_ttx_dec->is_display       = TRUE;
    p_ttx_dec->is_subtitle_mode = is_subtitle;

    if(is_subtitle == TRUE)
    {
        //ttx_dec_reset_vsb(p_ttx_dec);
        p_ttx_dec->is_buf_update = TRUE;
        //p_ttx_dec->input_page_no = page_no;
    }
    p_ttx_dec->input_page_no = page_no; //fix bug 18086    
	//fix bug 108790
	p_ttx_dec->initial_page.page_no = page_no;
    
    ttx_set_display_page_vsb(p_dvb_vbi, NULL, (u16)page_no, (u16)sub_no);


    return VBI_RC_SUCCESS;
}

static void ttx_hide_vsb(dvb_vbi_vsb_t *p_dvb_vbi)
{
    ttx_decoder_t *p_ttx_dec = p_dvb_vbi->p_ttx_dec;

    if(!p_ttx_dec)//fix bug 57305
        return;
    if(!p_ttx_dec->is_display)
        return;
    p_ttx_dec->is_display = FALSE;
    if(p_dvb_vbi->p_ttx_render != NULL)
    {
        if(p_ttx_dec->is_subtitle_mode)
        {
            ttx_render_delete_sub_region_vsb(p_dvb_vbi->p_ttx_render);
        }
        else
        {
            ttx_render_delete_region_vsb(p_dvb_vbi->p_ttx_render);
        }
    }
    if(p_dvb_vbi->p_osd_page != NULL)
    {
        free(p_dvb_vbi->p_osd_page);
        p_dvb_vbi->p_osd_page = NULL;
    }

    if(p_dvb_vbi->p_ttx_render != NULL)
    {
        if(NULL != p_dvb_vbi->p_ttx_render->p_char_buf)
        {
        	p_dvb_vbi->p_ttx_render->pfnCB(MT_NULL, MT_UNF_TTX_CB_DESTROY_CHARBUFF, NULL);
            p_dvb_vbi->p_ttx_render->p_char_buf = NULL;
        }
        free(p_dvb_vbi->p_ttx_render);
        p_dvb_vbi->p_ttx_render = NULL;
    }

    p_ttx_dec->display_page.page_no = TTX_NULL_PAGE_NO;
    p_ttx_dec->display_page.sub_no  = TTX_NULL_SUBPAGE;
    p_ttx_dec->waiting_page.page_no = 0x100;
    p_ttx_dec->waiting_page.sub_no  = TTX_FIRST_SUBPAGE;
    //bug 15405
    //ttx_dec_reset_vsb(p_ttx_dec);
}

static void ttx_start_vsb(dvb_vbi_vsb_t *p_dvb_vbi, u32 max_page_num, u32 max_sub_page_num,
                          ulong cb_addr)
{
    u16 mag_max_page_no = 0;
    //vbi_rc_t ret;
//    mem_user_dbg_info_t dbg_info ;
    int add_page_count;
    ttx_raw_t           *p_tmp_raw       = NULL;
    MT_ASSERT(p_dvb_vbi != NULL);

    if(p_dvb_vbi->p_ttx_dec != NULL)
    {
        return;
    }

    {
        //u32 size_raw = 0;
        //size_raw = sizeof(ttx_raw_t);
        //OS_PRINTF("ttx_start_vsb: [%x][%x][%x]\n", size_raw, max_page_num, max_sub_page_num);
    }


    p_dvb_vbi->p_ttx_dec = malloc(sizeof(ttx_decoder_t));
    MT_ASSERT(p_dvb_vbi->p_ttx_dec != NULL);
    memset(p_dvb_vbi->p_ttx_dec, 0, sizeof(ttx_decoder_t));
    add_page_count = 0;

    p_dvb_vbi->p_ttx_dec->p_raw_page_buf= malloc(sizeof(ttx_raw_t));
    if(p_dvb_vbi->p_ttx_dec->p_raw_page_buf)
    {
        memset(p_dvb_vbi->p_ttx_dec->p_raw_page_buf, 0, sizeof(ttx_raw_t));
        add_page_count++;
        p_tmp_raw = p_dvb_vbi->p_ttx_dec->p_raw_page_buf;
        while(add_page_count<(max_page_num + max_sub_page_num))
        {
            p_tmp_raw->p_next_buf = malloc(sizeof(ttx_raw_t));
            if(p_tmp_raw->p_next_buf)
            {
                memset(p_tmp_raw->p_next_buf, 0, sizeof(ttx_raw_t));
                add_page_count++;
                p_tmp_raw = p_tmp_raw->p_next_buf;
            }
            else
            {
                break;
            }
        }
        p_tmp_raw->p_next_buf = NULL;
    }

    p_dvb_vbi->p_ttx_dec->max_page_num = add_page_count;
    // 	p_dvb_vbi->p_ttx_dec->max_sub_page_num = 0;
    //one magzine have max 100 pages
    mag_max_page_no = (add_page_count > TTX_PAGE_STEPS)
                      ? TTX_PAGE_STEPS : add_page_count;
    //change into 0x mode, for example, 32 change into 0x32
    mag_max_page_no = ((mag_max_page_no / 10) << 4)|(mag_max_page_no % 10);
    p_dvb_vbi->p_ttx_dec->mag_max_page_no = mag_max_page_no;
    ttx_get_page_section_vsb(p_dvb_vbi->p_ttx_dec,
                                   0x899, &(p_dvb_vbi->p_ttx_dec->mag_max_page_section));

    ttx_dec_start_vsb(p_dvb_vbi->p_ttx_dec, ttx_dec_notify_vsb, cb_addr, p_dvb_vbi);
}

static void ttx_stop_vsb(dvb_vbi_vsb_t *p_dvb_vbi)
{
    ttx_decoder_t *p_ttx_dec = p_dvb_vbi->p_ttx_dec;
    int del_page_count;
    ttx_raw_t           *p_tmp_raw       = NULL;
    ttx_raw_t     *p_next;
    int del_count = 0;
//   mem_user_dbg_info_t dbg_info ;

    if(p_dvb_vbi->p_ttx_dec == NULL)
        return;

    if(p_dvb_vbi->p_osd_page != NULL)
    {
        free(p_dvb_vbi->p_osd_page);
        p_dvb_vbi->p_osd_page = NULL;
    }

    if(p_dvb_vbi->p_ttx_render != NULL)
    {
        if(p_ttx_dec->is_display)
        {
            if(p_ttx_dec->is_subtitle_mode)
                ttx_render_delete_sub_region_vsb(p_dvb_vbi->p_ttx_render);
            else
                ttx_render_delete_region_vsb(p_dvb_vbi->p_ttx_render);
        }

        free(p_dvb_vbi->p_ttx_render);
        p_dvb_vbi->p_ttx_render = NULL;
    }

    ttx_dec_stop_vsb(p_dvb_vbi->p_ttx_dec);
#if 0
    p_tmp_raw = p_dvb_vbi->p_ttx_dec->p_raw_page_buf;
    int del_count = 0;
    for(del_page_count = 0; del_page_count < p_dvb_vbi->p_ttx_dec->max_page_num; del_page_count++)
    {
        if(p_tmp_raw)
        {
            p_next = p_tmp_raw->p_next;
            mtos_free(p_tmp_raw);
            p_tmp_raw = p_next;
            del_count ++;
        }
    }
#endif

    for(del_page_count = 0, p_tmp_raw = p_dvb_vbi->p_ttx_dec->p_raw_page_buf; del_page_count < p_dvb_vbi->p_ttx_dec->max_page_num; del_page_count ++)
    {
        if(p_tmp_raw)
        {
            p_next = p_tmp_raw->p_next_buf;
            free(p_tmp_raw);
            p_tmp_raw = p_next;
            del_count ++;
        }
    }
    p_dvb_vbi->p_ttx_dec->p_raw_page_buf = NULL;
#if 0
    mtos_free(p_dvb_vbi->p_ttx_dec->p_raw_sub);
    p_dvb_vbi->p_ttx_dec->p_raw_sub = NULL;
#endif
    free(p_dvb_vbi->p_ttx_dec);
    p_dvb_vbi->p_ttx_dec = NULL;
}

#if 0
static void ttx_pause_vsb(dvb_vbi_vsb_t *p_dvb_vbi)
{
    if(p_dvb_vbi->p_ttx_dec == NULL || p_dvb_vbi->is_ttx_pause)
        return;

    p_dvb_vbi->is_ttx_pause = TRUE;
		g_inital_page = 0;
}

static void ttx_resume_vsb(dvb_vbi_vsb_t *p_dvb_vbi)
{
    if(p_dvb_vbi->p_ttx_dec == NULL || !p_dvb_vbi->is_ttx_pause)
        return;

    p_dvb_vbi->is_ttx_pause = FALSE;
}
#endif

static void set_language_code_vsb(dvb_vbi_vsb_t *p_dvb_vbi, u8 language_code, u8 language_code1, u8 language_code2)
{
    int i=0;
    int j=0;
    char language_iso639[4] = {0};

    if(p_dvb_vbi->p_ttx_dec == NULL)
    {
        return;
    }
    for(j = 0; j < 8; j ++)
    {
        p_dvb_vbi->p_ttx_dec->mag[j].extension.charset_code[0] = 0;
        p_dvb_vbi->p_ttx_dec->mag[j].extension.charset_code[1] = 0;
    }
    p_dvb_vbi->p_ttx_dec->def_mag.extension.charset_code[0] = 0;
    p_dvb_vbi->p_ttx_dec->def_mag.extension.charset_code[1] = 0;
    sprintf(language_iso639, "%c%c%c", tolower((char)language_code), tolower((char)language_code1), tolower((char)language_code2));

    for( i = 0; ppsz_default_triplet[i] != NULL; i++ )
    {
        if(!strcmp(language_iso639, ppsz_default_triplet[i] ) )
        {
            for(j = 0; j < 8; j ++)
            {
                p_dvb_vbi->p_ttx_dec->mag[j].extension.charset_code[0] = pi_default_triplet[i];
                p_dvb_vbi->p_ttx_dec->mag[j].extension.charset_code[1] = 0;
            }
            p_dvb_vbi->p_ttx_dec->def_mag.extension.charset_code[0] = pi_default_triplet[i];
            p_dvb_vbi->p_ttx_dec->def_mag.extension.charset_code[1] = 0;
        }
    }
}

RET_CODE vbi_init_vsb(void)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

    p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi != NULL)
        return ERR_FAILURE;


    p_dvb_vbi = malloc(sizeof(dvb_vbi_vsb_t));
    MT_ASSERT(p_dvb_vbi != NULL);
    memset(p_dvb_vbi, 0, sizeof(dvb_vbi_vsb_t));

    dvb_vbi_set_handle(p_dvb_vbi);
    p_dvb_vbi->region_pos = TTX_REGION_OUT_DECODER;
//	p_dvb_vbi->pfnCB = pfnCB;
		(mt_void)pthread_mutex_init(&render_mutex, NULL);

    return SUCCESS;
}

RET_CODE vbi_deinit_vsb(void)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

    p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi == NULL)
    {
        return ERR_FAILURE;
    }

	free(p_dvb_vbi);
	dvb_vbi_set_handle(NULL);
	pthread_mutex_destroy(&render_mutex);
    return SUCCESS;
}

RET_CODE vbi_set_callback(MT_UNF_TTX_CB_FN pfnCB)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

    p_dvb_vbi = dvb_vbi_get_handle();
	if(p_dvb_vbi == NULL)
		return ERR_FAILURE;
	p_dvb_vbi->pfnCB = pfnCB;

    return SUCCESS;
}

RET_CODE vbi_set_region_handle_vsb(void *p_rgn)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    p_dvb_vbi->p_rgn = p_rgn;

    return SUCCESS;
}

RET_CODE vbi_set_pid_vsb(u16 pid)
{
    return SUCCESS;
}

RET_CODE vbi_set_language_code_vsb(u8 language_code, u8 language_code1, u8 language_code2)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

    p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi == NULL)
        return ERR_FAILURE;
    set_language_code_vsb(p_dvb_vbi, language_code, language_code1, language_code2);    
    return SUCCESS;
}

RET_CODE vbi_ttx_start_vsb(u32 max_page_num, u32 max_sub_page_num, ulong cb_addr)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

    p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi == NULL)
        return ERR_FAILURE;

    p_dvb_vbi->is_big_mem = TRUE;

    ttx_start_vsb(p_dvb_vbi, max_page_num, max_sub_page_num, cb_addr);

    p_dvb_vbi->is_ttx_pause = FALSE;
    return SUCCESS;
}

RET_CODE vbi_ttx_stop_vsb(void)
{
	dvb_vbi_vsb_t *p_dvb_vbi = NULL;
	p_dvb_vbi = dvb_vbi_get_handle();
	if(p_dvb_vbi == NULL)
		return SUCCESS;
	ttx_stop_vsb(p_dvb_vbi);
  return SUCCESS;
}

RET_CODE vbi_ttx_pause_vsb(void)
{
    return SUCCESS;
}

RET_CODE vbi_ttx_resume_vsb(void)
{
    return SUCCESS;
}

RET_CODE vbi_ttx_show_vsb(MT_BOOL is_subtitle, u32 page_no)
{
    dvb_vbi_vsb_t *p_dvb_vbi = NULL;

    p_dvb_vbi = dvb_vbi_get_handle();

    //some ttx_subtitle do not have magzine number, but in fact it is in mag8
    if((page_no & 0xf00) == 0)
    {
        page_no = (page_no & 0x0ff) + 0x800;
    }

	ttx_show_vsb(p_dvb_vbi, is_subtitle, page_no);
    return SUCCESS;
}

RET_CODE vbi_ttx_hide_vsb(MT_BOOL sync)
{
	dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();

	ttx_hide_vsb(p_dvb_vbi);
  return SUCCESS;
}

RET_CODE vbi_post_ttx_key_vsb(ttx_key_t key)
{
	dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();

	pthread_mutex_lock(&render_mutex);
	ttx_key_proc_vsb(p_dvb_vbi, key);
	pthread_mutex_unlock(&render_mutex);

    return SUCCESS;
}

RET_CODE vbi_set_video_std_vsb(video_std_t std)
{
    return SUCCESS;
}

RET_CODE vbi_set_font_size_vsb(ttx_font_size_t font_size)
{
	dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
	ttx_render_vsb_t *p_ttx_render = p_dvb_vbi->p_ttx_render;
	if(p_ttx_render != NULL)
		p_ttx_render->font_size = font_size;
	else
		pre_font_size = font_size;
    return SUCCESS;
}

RET_CODE vbi_set_region_pos(ttx_region_pos_t pos)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    p_dvb_vbi->region_pos = pos;

    return SUCCESS;
}

ttx_region_pos_t vbi_get_region_pos(void)
{

    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    return  p_dvb_vbi->region_pos;
}

u8 *vbi_get_font(ttx_font_type_t src)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();

    switch(src)
    {
    case TTX_FONT_SRC_PAL:
        return p_dvb_vbi->p_pal_font;
    case TTX_FONT_SRC_NTSL:
        return p_dvb_vbi->p_ntsl_font;
    case TTX_FONT_SRC_SMALL:
        return p_dvb_vbi->p_small_font;
    case TTX_FONT_SRC_HD:
        return p_dvb_vbi->p_hd_font;
    }
    return NULL;

}

void vbi_set_font_src(ttx_font_src_t *p_font)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();

    p_dvb_vbi->p_pal_font = p_font->p_pal_font;
    p_dvb_vbi->p_ntsl_font = p_font->p_ntsl_font;
    p_dvb_vbi->p_small_font = p_font->p_small_font;
    p_dvb_vbi->p_hd_font = p_font->p_hd_font;
}

void vbi_reset_font_src(void)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();

    p_dvb_vbi->p_pal_font = NULL;
    p_dvb_vbi->p_ntsl_font = NULL;
    p_dvb_vbi->p_small_font = NULL;
    p_dvb_vbi->p_hd_font = NULL;
}

MT_BOOL vbi_is_big_mem(void)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();

    return p_dvb_vbi->is_big_mem;
}

void vbi_set_ts_in(u8 ts_in)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    p_dvb_vbi->ts_in = ts_in;
}


RET_CODE vbi_set_current_pid_vsb(u16 *pid)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    *pid = p_dvb_vbi->pid;
    if(*pid == 0x1fff)
        return ERR_FAILURE;
    else
        return SUCCESS;
}

void vbi_set_ttx_page_draw_mode(u8 draw_mode)
{
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi)
    {
        p_dvb_vbi->page_draw= draw_mode;
    }
}

u8 vbi_get_ttx_page_draw_mode(void)
{
    u8 ret = 0;
    dvb_vbi_vsb_t *p_dvb_vbi = dvb_vbi_get_handle();
    if(p_dvb_vbi)
    {
        ret=p_dvb_vbi->page_draw;
    }
    return ret;
}

