/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_type.h"
#include "sys_define.h"
#include "string.h"
//#include "mtos_printk.h"
//#include "mtos_int.h"
#include "string.h"
//#include "lib_util.h"
//#include "mtos_msg.h"
//#include "mdl.h"
//#include "drv_misc.h"

#include "ttx_lang_vsb.h"
#include "ttx_bcd_vsb.h"
#include "ttx_dec_vsb.h"
#include "ttx_db_vsb.h"
#ifdef WARRIORS
#include "vbi_api.h"
#endif

static u32 bcd_number_to_dec(u32 input)
{
  u32 mask = 0xF;
  u32 shift_rslt = 0;
  u8 cnt = 0;
  u32 out = 0;
  u32 dec_num = 1;

  for(cnt = 0; cnt < 8; cnt++)
  {
    shift_rslt = (input & mask) >> (cnt * 4);
    out += shift_rslt * dec_num;

    dec_num *= 10;
    mask *= 16;
  }

  return out;
}

static u32 dec_number_to_bcd(u32 input)
{
  u32 mask = 10000000;
  u32 shift_rslt = 0;
  s8 cnt = 0;
  u32 out = 0;
  u32 dec_num = input;

  for(cnt = 7; cnt >= 0; cnt --)
  {
    shift_rslt = (dec_num / mask);
    dec_num = (dec_num % mask);
    out |= shift_rslt << (cnt * 4);
    mask /= 10;
  }

  return out;
}

static inline MT_BOOL vbi_is_bcd_vsb(u32 bcd)
{
    u32 a = 0;
    u32 b = 0;
    u32 c = 0;

    a = (bcd + 0x06666666);
    b = (bcd ^ 0x06666666);
    c = a ^ b;
    if(0 == (c & 0x11111110))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
#if 0
void ttx_print_db_vsb(ttx_decoder_t *p_dec)
{
    u32 i = 0;
    ttx_raw_t *p_d = p_dec->p_raw;


    while(i < p_dec->max_page_num)
    {
        OS_PRINTF("%3.3x    %4.4x    %d\n", p_d->page_no, p_d->sub_no, p_d->priority);
        p_d ++;
        i ++;
    }
}
#endif
MT_BOOL ttx_buf_flush_vsb(ttx_decoder_t *p_dec, u16 display_page_no, u16 cur_page_no)
{
    u16 mag_max_page_no = p_dec->mag_max_page_no;
    u16 display_mag_no = 0;
    u16 cur_mag_no = 0;
    u16 display_page_section = 0;
    u16 cur_page_section = 0;
#ifdef WARRIORS
    MT_BOOL is_big_mem = vbi_is_big_mem();

    if(is_big_mem)  //do not need flush when full memory
        return FALSE;
#endif

    display_mag_no = (display_page_no & 0xf00) >> 8;
    cur_mag_no = (cur_page_no & 0xf00) >> 8;

    if(TTX_NULL_PAGE_NO == display_page_no)
    {
#ifdef WARRIORS
        return FALSE;
#else
        return FALSE;//TRUE;
#endif
    }

    if((!((display_page_no & 0xff) <= 0x99 && (display_page_no & 0x0f) <= 9)) ||
            (!((cur_page_no & 0xff) <= 0x99 && (cur_page_no & 0x0f) <= 9)))
    {
        return FALSE;
    }

    if(display_mag_no != cur_mag_no)//different magzine needs update
    {
        return TRUE;
    }
    else
    {
        display_page_section = (display_page_no & 0xff) / mag_max_page_no;
        cur_page_section = (cur_page_no & 0xff) / mag_max_page_no;
        if(display_page_section == cur_page_section)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
}

vbi_rc_t ttx_get_page_section_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_page_section)
{
    u16 max_page_num = (p_dec->max_page_num > TTX_PAGE_STEPS
                        ? TTX_PAGE_STEPS : p_dec->max_page_num);
    u16 cur_page_num = 0;

    if(!((cur_page_no & 0xff) <= 0x99 && (cur_page_no & 0x0f) <= 9))
    {
        return VBI_RC_FAILED;
    }

    cur_page_num = ((cur_page_no & 0xf0)>>4)*10 + (cur_page_no & 0x0f);
    *p_page_section = (cur_page_num / max_page_num);

    return VBI_RC_SUCCESS;
}

///TODO:add two function to support ttx +/- 100 page
vbi_rc_t ttx_get_next_page_up_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_next_page_no)
{
    u16 cur_page_num = 0;
    u16 cur_mag_num = 0;
    u16 next_mag_num = 0;

    if(!((cur_page_no & 0xff) <= 0x99
            && (cur_page_no & 0x0f) <= 9
            && (cur_page_no & 0xf00) < 0x900
            && (cur_page_no & 0xf00) > 0x0))
    {
        return VBI_RC_FAILED;
    }

    cur_mag_num = ((cur_page_no & 0xf00) >> 8);
    if(8 == cur_mag_num)
    {
        next_mag_num = 1;
    }
    else
    {
        next_mag_num = cur_mag_num + 1;
    }

    cur_page_num = ((cur_page_no & 0xf0) >> 4) * 10 + (cur_page_no & 0x0f);
    *p_next_page_no = (((next_mag_num << 8) & 0xf00)
                       | (((cur_page_num / 10) << 4) & 0xf0)
                       | ((cur_page_num % 10) & 0xf));

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_prev_page_down_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_prev_page_no)
{
    u16 cur_page_num = 0;
    u16 cur_mag_num = 0;
    u16 next_mag_num = 0;

    if(!((cur_page_no & 0xff) <= 0x99
            && (cur_page_no & 0x0f) <= 9
            && (cur_page_no & 0xf00) < 0x900
            && (cur_page_no & 0xf00) > 0x0))
    {
        return VBI_RC_FAILED;
    }

    cur_mag_num = ((cur_page_no & 0xf00) >> 8);
    if(1 == cur_mag_num)
    {
        next_mag_num = 8;
    }
    else
    {
        next_mag_num = cur_mag_num - 1;
    }

    cur_page_num = ((cur_page_no & 0xf0) >> 4) * 10 + (cur_page_no & 0x0f);
    *p_prev_page_no = (((next_mag_num << 8) & 0xf00)
                       | (((cur_page_num / 10) << 4) & 0xf0)
                       | ((cur_page_num % 10) & 0xf));

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_next_page_no_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_next_page_no)
{
    u16 cur_page_num = 0;
    u16 next_page_num = 0;
    u16 cur_mag_num = 0;
    u16 next_mag_num = 0;

    if(!((cur_page_no & 0xff) <= 0x99
            && (cur_page_no & 0x0f) <= 9
            && (cur_page_no & 0xf00) < 0x900
            && (cur_page_no & 0xf00) > 0x0))
    {
        return VBI_RC_FAILED;
    }

    cur_mag_num = ((cur_page_no & 0xf00) >> 8);
    //last one in a magzine
    if(0x99 == (cur_page_no & 0xff))
    {
        //last magzine
        if(8 == cur_mag_num)
        {
            next_mag_num = 1;
        }
        else
        {
            next_mag_num = cur_mag_num + 1;
        }

        *p_next_page_no = (next_mag_num << 8) & 0xf00;
    }
    else//move in the same magzine
    {
        next_mag_num = cur_mag_num;
        cur_page_num = ((cur_page_no & 0xf0) >> 4) * 10 + (cur_page_no & 0x0f);
        next_page_num = cur_page_num + 1;
        *p_next_page_no = (((next_mag_num << 8) & 0xf00)
                           | (((next_page_num / 10) << 4) & 0xf0)
                           | ((next_page_num % 10) & 0xf));
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_prev_page_no_vsb(ttx_decoder_t *p_dec, u16 cur_page_no, u16 *p_prev_page_no)
{
    u16 cur_page_num = 0;
    u16 prev_page_num = 0;
    u16 cur_mag_num = 0;
    u16 prev_mag_num = 0;

    if(!((cur_page_no & 0xff) <= 0x99
            && (cur_page_no & 0x0f) <= 9
            && (cur_page_no & 0xf00) < 0x900
            && (cur_page_no & 0xf00) > 0x0))
    {
        return VBI_RC_FAILED;
    }

    cur_mag_num = ((cur_page_no & 0xf00) >> 8);
    //first one in a magzine
    if(0x0 == (cur_page_no & 0xff))
    {
        //first magzine
        if(1 == cur_mag_num)
        {
            prev_mag_num = 8;
        }
        else
        {
            prev_mag_num = cur_mag_num - 1;
        }

        *p_prev_page_no = ((prev_mag_num << 8) & 0xf00) | 0x99;
    }
    else//move in the same magzine
    {
        prev_mag_num = cur_mag_num;
        cur_page_num = ((cur_page_no & 0xf0) >> 4) * 10 + (cur_page_no & 0x0f);
        prev_page_num = cur_page_num - 1;
        *p_prev_page_no = (((prev_mag_num << 8) & 0xf00)
                           | (((prev_page_num / 10) << 4) & 0xf0)
                           | ((prev_page_num % 10) & 0xf));
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_page_no_vsb(u16 cur_page_no, s16 add_page_no, u16 *p_new_page_no)
{    
	s16  no;    
   
	no = bcd_number_to_dec(cur_page_no);            
	no = no + add_page_no;        
	if(no > 899)        
	{            
		no = no - 800;        
	}        
	if(no < 100)        
	{            
		no = no + 800;        
	}	      
	*p_new_page_no = dec_number_to_bcd(no);    
	return VBI_RC_SUCCESS;
}	

vbi_rc_t ttx_set_section_start_page_no_vsb(ttx_decoder_t *p_dec,
        u16 page_section, u16 mag_no, u16 *p_page_no)
{
    u16 max_page_num = (p_dec->max_page_num > TTX_PAGE_STEPS
                        ? TTX_PAGE_STEPS : p_dec->max_page_num);

    MT_ASSERT(page_section <= p_dec->mag_max_page_section);

    *p_page_no = (((mag_no) << 8) |
                  (((page_section * max_page_num)/10) << 4) |
                  (((page_section * max_page_num)%10)));

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_set_section_end_page_no_vsb(ttx_decoder_t *p_dec,
        u16 page_section, u16 mag_no, u16 *p_page_no)
{
    u16 max_page_num = (p_dec->max_page_num > TTX_PAGE_STEPS
                        ? TTX_PAGE_STEPS : p_dec->max_page_num);

    MT_ASSERT(page_section <= p_dec->mag_max_page_section);

    if(page_section == p_dec->mag_max_page_section)
    {
        *p_page_no = (((mag_no) << 8) | 0x99);
    }
    else
    {
        *p_page_no = (((mag_no) << 8) |
                      ((((page_section + 1) * max_page_num - 1)/10) << 4) |
                      ((((page_section + 1) * max_page_num - 1)%10)));
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_remove_all_raw_page_vsb(ttx_decoder_t *p_dec, u16 mag_no)
{
    ttx_magazine_t  *p_mag  = &p_dec->mag[mag_no - 1];
    ttx_raw_t       *p_d    = NULL;

    if(p_mag->p_raw == NULL)
        return  VBI_RC_FAILED;

    for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
    {
        p_dec->raw_cnt --;
    }
    p_mag->p_raw = NULL;

    return VBI_RC_SUCCESS;
}

vbi_rc_t  ttx_remove_raw_page_vsb_from_mag(ttx_decoder_t *p_dec, u16 mag_no, u16 input_page_no, ttx_raw_t **pp_raw)
{
    ttx_magazine_t  *p_mag;
    ttx_raw_t       *p_d;	
    ttx_raw_inventory_t *p_inventory;
    s16  page_no;
	
	p_mag  = &p_dec->mag[mag_no - 1];
	p_d   = NULL;
    *pp_raw = NULL;
    if((p_mag == NULL) || (p_mag->p_raw == NULL))
    {
         return VBI_RC_FAILED;
    }

    if((p_mag->p_raw->page_no != input_page_no)
                && vbi_is_bcd_vsb(p_mag->p_raw->page_no) == TRUE
                && vbi_is_bcd_vsb(p_mag->p_raw->sub_no & 0xffff) == TRUE)		
    {
	 page_no = bcd_number_to_dec(p_mag->p_raw->page_no);
        p_inventory = p_dec->inventory + (page_no - 100);
        if(p_inventory)
        {
        	p_inventory->cached = 0;
        }     
    	*pp_raw = p_mag->p_raw;	
        p_mag->p_raw = p_mag->p_raw->p_next;
        p_dec->raw_cnt --;

	return VBI_RC_SUCCESS;
    }
	
    for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
    {
        //fix bug 98978 begin
		if(p_d->p_next == NULL)
		{
			return VBI_RC_SUCCESS;
		}
		//fix bug98978 end

        if((p_d->p_next->page_no  != input_page_no)
                && vbi_is_bcd_vsb(p_d->p_next->page_no) == TRUE
                && vbi_is_bcd_vsb(p_d->p_next->sub_no & 0xffff) == TRUE)
        {
		page_no = bcd_number_to_dec(p_d->p_next->page_no);
        	p_inventory = p_dec->inventory + (page_no - 100);
        	if(p_inventory)
        	{
            		p_inventory->cached = 0;
        	} 			
	     *pp_raw = p_d->p_next;			
            p_d->p_next = p_d->p_next->p_next;
            p_dec->raw_cnt --;
            return VBI_RC_SUCCESS;
        }
    }
        return VBI_RC_FAILED;
}

vbi_rc_t ttx_get_raw_buf_vsb(ttx_decoder_t *p_dec, u16 page_no, u16 sub_no, ttx_raw_t **pp_buf)
{
    u16 mag_max_page_no = p_dec->mag_max_page_no;
    u16 mag_no = (page_no & 0xf00) >> 8;
    u16                 input_page_no;
    ttx_raw_t  * p_free_raw;
    ttx_raw_t           *p_next;
    u8 i = 0;

    if(((page_no & 0xff) < mag_max_page_no) && ((page_no & 0x0f) <= 9))
    {
        if(p_dec->p_free_raw_buf != NULL)
        {
            *pp_buf = p_dec->p_free_raw_buf;
            p_dec->p_free_raw_buf = p_dec->p_free_raw_buf->p_next_buf;
            if(p_dec->p_free_raw_buf == NULL)
            {
                OS_PRINTF("ttx_get_raw_buf_vsb p_free_raw_buf is null!\n");
            }
            return VBI_RC_SUCCESS;
        }
        else
        {

            if(((p_dec->input_page_no & 0xf00)== ((page_no & 0xf00)))||((p_dec->input_page_no == 0xffff) && ((page_no & 0xf00) == 0x100)))
            {
                p_free_raw = NULL;

                for(i=0; i<8; i++)
                {
                    mag_no++;
                    if(mag_no>8)
                    {
                        mag_no = 1;
                    }
                    if(p_dec->input_page_no == 0xffff)
                    {
                        input_page_no = 0x100;
                    }
                    else
                    {
				        input_page_no = p_dec->input_page_no;
                    }
			
                    ttx_remove_raw_page_vsb_from_mag(p_dec, mag_no, input_page_no, &p_free_raw);
                    if(p_free_raw!=NULL)
                    {
                        p_next = p_free_raw->p_next_buf;
                        memset(p_free_raw, 0, sizeof(ttx_raw_t));
                        p_free_raw->priority = TTX_PAGE_PRI_LOWEST;
                        p_free_raw->function = TTX_PAGE_FUNC_DISCARD;
                        p_free_raw->p_next_buf = p_next;
                        *pp_buf = p_free_raw;			
                        return VBI_RC_SUCCESS;
                    }
                }

            }
        }
    }

    *pp_buf = NULL;
    return VBI_RC_FAILED;

#if 0
    u16 display_page_no = p_dec->display_page.page_no;
    u16 page_section = 0;
    u8 i = 0;
    ttx_raw_t *p_raw = NULL;
    ttx_raw_inventory_t *p_inventory = NULL;
    //work around for unmatched spec page
    static u16 input_page_old = 0;
    static u16 sub_no_old = 0;
    static MT_BOOL b_looped = FALSE;
    static MT_BOOL b_get_sub = FALSE;
    u16 input_page = bcd_number_to_dec(p_dec->input_page_no);
#ifdef WARRIORS
    vbi_rc_t            rc;
    MT_BOOL is_big_mem = vbi_is_big_mem();
#endif

    p_inventory = p_dec->inventory + (input_page - 100);

    if(TRUE == p_dec->is_buf_update)//enter page no.
    {
#ifdef WARRIORS
        rc = ttx_get_raw_page_entry_vsb(p_dec, page_no, &p_inventory);
#endif
        page_section = (p_dec->input_page_no & 0xff) / mag_max_page_no;
        if(((page_no & 0xff) <= ((mag_max_page_no * (page_section + 1)) > 0x99
                                 ? 0x99 : (mag_max_page_no * (page_section + 1)))) &&
                ((page_no & 0xff) >= (mag_max_page_no * page_section)) &&
                ((page_no & 0x0f) <= 9) &&
#ifdef WARRIORS
                /*((p_dec->input_page_no & 0xf00)== ((page_no & 0xf00))) &&*/
                ((0 == sub_no) || (1 == sub_no) ||(p_inventory->cached == 0)))
#else
                ((p_dec->input_page_no & 0xf00)== ((page_no & 0xf00))) &&
                ((0 == sub_no) || (1 == sub_no)))
#endif
        {
            if(p_dec->p_free_raw_buf != NULL)
            {
                *pp_buf = p_dec->p_free_raw_buf;
                p_dec->p_free_raw_buf = p_dec->p_free_raw_buf->p_next;

                return VBI_RC_SUCCESS;
            }
        }

        //work around for the special pages containing subpage, but subpage 1 do not exist.
        if(input_page_old != p_dec->input_page_no)
        {
            b_looped = FALSE;
            b_get_sub = FALSE;
            sub_no_old = 0;
        }

        input_page_old = p_dec->input_page_no;

        if(page_no == p_dec->input_page_no)
        {
            //the subpage have been sent more than once
            if(FALSE == b_get_sub)//the first incoming subpage other than page no 1
            {
                sub_no_old = sub_no;
                b_get_sub = TRUE;
            }
            else if(sub_no_old > sub_no)//get smallest subpage no
            {
                sub_no_old = sub_no;
            }
            //after a loop, put the smallest subpage in the buffer first
            //if packets order in 23456--23456, we can detect the subpage 2 as the smallest
            //but if  like 22223334455, we can not detect the smallest
            else if((sub_no_old == sub_no) && (0 != sub_no_old) && (1 != sub_no_old))
            {
                b_looped = TRUE;
            }
        }

        //sub page buffer
        if((page_no == p_dec->input_page_no) &&
                ((0 != sub_no) && (1 != sub_no)) &&
                ((sub_no & 0x0f) <= 9) &&
                ((sub_no & 0xf0) <= 0x90) &&
                //this first sub page already added to the raw buff  or work around condition match
                ((0 != p_inventory->cached) || (TRUE == b_looped)))
        {
            if(p_dec->p_free_raw_sub_buf != NULL)
            {
                *pp_buf = p_dec->p_free_raw_sub_buf;
                p_dec->p_free_raw_sub_buf = p_dec->p_free_raw_sub_buf->p_next;

                return VBI_RC_SUCCESS;
            }
            else//if full reset the buffer
            {
                for(i = 0, p_raw = p_dec->p_raw_sub; i < p_dec->max_sub_page_num; i ++, p_raw ++)
                {
                    ttx_remove_raw_page_vsb(p_dec, p_raw);
                    memset(p_raw, 0, sizeof(ttx_raw_t));
                    p_raw->priority = TTX_PAGE_PRI_LOWEST;
                    p_raw->function = TTX_PAGE_FUNC_DISCARD;
                    p_raw->p_next   = p_raw + 1;
                }
                (p_dec->p_raw_sub + (p_dec->max_sub_page_num - 1))->p_next = NULL;
                p_dec->p_free_raw_sub_buf = p_dec->p_raw_sub;
            }
        }
    }

    if((TTX_NULL_PAGE_NO == display_page_no) && (TRUE != p_dec->is_buf_update))//initial state
    {
        //initial buf the magzine 1 ,first max_page_no pages. example:0---0x31
        if(((page_no & 0xff) < mag_max_page_no) &&
                ((page_no & 0x0f) <= 9) &&
#ifdef WARRIORS
                (is_big_mem ? 1: (1 == ((page_no & 0xf00)>>8))) &&
                (((0 == sub_no) || (1 == sub_no)) || (p_inventory->cached == 0)))
#else
                (1 == ((page_no & 0xf00)>>8)) &&
                ((0 == sub_no) || (1 == sub_no)))
#endif
        {
            if(p_dec->p_free_raw_buf != NULL)
            {
                *pp_buf = p_dec->p_free_raw_buf;
                p_dec->p_free_raw_buf = p_dec->p_free_raw_buf->p_next;

                return VBI_RC_SUCCESS;
            }
        }
        p_inventory = p_dec->inventory;
        //0x100 sub page buffer
        if((page_no == 0x100) &&
                ((0 != sub_no) && (1 != sub_no)) &&
                ((sub_no & 0x0f) <= 9) &&
                ((sub_no & 0xf0) <= 0x90)
                /* && (0 != p_inventory->cached)*/)//this first sub page already added to the raw buff
        {
            if(p_dec->p_free_raw_sub_buf != NULL)
            {
                *pp_buf = p_dec->p_free_raw_sub_buf;
                p_dec->p_free_raw_sub_buf = p_dec->p_free_raw_sub_buf->p_next;

                return VBI_RC_SUCCESS;
            }
            else//if full reset the buffer
            {
                for(i = 0, p_raw = p_dec->p_raw_sub; i < p_dec->max_sub_page_num; i ++, p_raw ++)
                {
                    ttx_remove_raw_page_vsb(p_dec, p_raw);
                    memset(p_raw, 0, sizeof(ttx_raw_t));
                    p_raw->priority = TTX_PAGE_PRI_LOWEST;
                    p_raw->function = TTX_PAGE_FUNC_DISCARD;
                    p_raw->p_next   = p_raw + 1;
                }
                (p_dec->p_raw_sub + (p_dec->max_sub_page_num - 1))->p_next = NULL;
                p_dec->p_free_raw_sub_buf = p_dec->p_raw_sub;
            }
        }

    }

    *pp_buf = NULL;
    return VBI_RC_FAILED;
#endif
}

vbi_rc_t ttx_free_raw_buf_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_buf)
{
    memset(p_buf, 0, sizeof(ttx_raw_t));
    p_buf->function = TTX_PAGE_FUNC_DISCARD;
    p_buf->p_next = p_dec->p_free_raw_buf;
    p_dec->p_free_raw_buf = p_buf;


    return VBI_RC_FAILED;
}

vbi_rc_t ttx_add_raw_page_vsb(ttx_decoder_t *p_dec
                              , ttx_raw_t *p_raw, u16 sub_no_mask)
{
    s16             page_no = p_raw->page_no;
    s16             sub_no  = p_raw->sub_no & sub_no_mask;
    ttx_magazine_t  *p_mag  = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d = NULL, *p_prev = NULL;

    if(p_mag->p_raw == NULL)
    {
        p_mag->p_raw  = p_raw;
        p_raw->p_next = NULL;

        p_dec->raw_cnt ++;

        return VBI_RC_SUCCESS;
    }

    for(p_prev = NULL, p_d = p_mag->p_raw
                             ; p_d != NULL
            ; p_prev = p_d, p_d = p_d->p_next)
    {
        if(page_no < p_d->page_no)
        {
            p_raw->p_next = p_d;
            if(p_prev != NULL)
                p_prev->p_next = p_raw;
            else
                p_mag->p_raw = p_raw;

            p_dec->raw_cnt ++;
            break;
        }
        else if(page_no == p_d->page_no)
        {
            if((sub_no & sub_no_mask) < (p_d->sub_no & sub_no_mask))
            {
                p_raw->p_next = p_d;
                if(p_prev != NULL)
                    p_prev->p_next = p_raw;
                else
                    p_mag->p_raw = p_raw;

                p_dec->raw_cnt ++;
                break;
            }
            else if((sub_no & sub_no_mask) == (p_d->sub_no & sub_no_mask))
            {
                if(p_d != p_raw)
                    MT_ASSERT(0);
                break;
            }
        }

        if(p_d->p_next == NULL)
        {
            p_d->p_next     = p_raw;
            p_raw->p_next = NULL;

            p_dec->raw_cnt ++;
            break;
        }
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_remove_raw_page_vsb(ttx_decoder_t *p_dec, ttx_raw_t *p_raw)
{
    s16             page_no = p_raw->page_no;
    ttx_magazine_t  *p_mag  = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d    = NULL;

    if(p_mag->p_raw == p_raw)
    {
        p_mag->p_raw = p_raw->p_next;
        p_dec->raw_cnt --;
        return VBI_RC_SUCCESS;
    }

    for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
    {
        if(p_d->p_next == p_raw)
        {
            p_d->p_next = p_raw->p_next;
            p_dec->raw_cnt --;
            return VBI_RC_SUCCESS;
        }
    }

    return VBI_RC_FAILED;
}

vbi_rc_t ttx_get_raw_page_vsb(ttx_decoder_t *p_dec
                              , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw)
{
    ttx_magazine_t  *p_mag = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d   = NULL;

    MT_ASSERT(page_no < 0x8FF);

    *pp_raw = NULL;

    if(sub_no == TTX_FIRST_SUBPAGE
            || sub_no == TTX_ANY_SUBPAGE
            || sub_no == TTX_NULL_SUBPAGE)
    {
        for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
        {
            if(p_d->page_no == page_no)
            {
                //fix bug 6242, the first display subpage should be index1.
                //Index0 stands for no subpage
                //now use the ttx_get_raw_buf to store the subpage in sequence.
                *pp_raw = p_d;
                return VBI_RC_SUCCESS;
            }
        }
    }
    else
    {
        for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
        {
            if(p_d->page_no == page_no
                    && ((p_d->sub_no & sub_no_mask) == (sub_no & sub_no_mask)))
            {
                *pp_raw = p_d;
                return VBI_RC_SUCCESS;
            }
#ifdef WARRIORS
            if((p_d->page_no == page_no) && (p_dec->input_page_no != page_no)
                    && ((sub_no == 0) || (sub_no == 1)))
            {
                *pp_raw = p_d;
                return VBI_RC_SUCCESS;
            }
#endif
        }
    }

    return VBI_RC_FAILED;
}

vbi_rc_t ttx_get_next_raw_page_no_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 *p_next_page_no)
{
    s16                 no = 0;
    s16 min_no = 0;
    s16 max_no = 0;

    ttx_raw_inventory_t *p_inventory = NULL;

//  no = page_no;
    no = bcd_number_to_dec(page_no);
#if 0
    min_no = bcd_number_to_dec(page_no & 0xf00);
    max_no = min_no + 99;
#else
    min_no = 100;
    max_no = 899;
#endif

    p_inventory = p_dec->inventory + (no - 100);
    while(1)
    {
        no ++;
        p_inventory ++;
        if(no > max_no)
        {
            no          = min_no;
            p_inventory = p_dec->inventory + (no - 100);
        }

        if(no == bcd_number_to_dec(page_no))
        {
            *p_next_page_no = dec_number_to_bcd(no);
            return VBI_RC_FAILED;
        }


        if(p_inventory->cached != 0)
        {
            *p_next_page_no = dec_number_to_bcd(no);
            break;
        }
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_prev_raw_page_no_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 *p_prev_page_no)
{
    s16                 no = 0;
    ttx_raw_inventory_t *p_inventory = NULL;
    s16 min_no = 0;
    s16 max_no = 0;
    //no = page_no;
    no = bcd_number_to_dec(page_no);
#if 0
    min_no = bcd_number_to_dec(page_no & 0xf00);
    max_no = min_no + 99;
#else
    min_no = 100;
    max_no = 899;
#endif
    p_inventory = p_dec->inventory + (no - 100);
    while(1)
    {
        no --;
        p_inventory --;

        if(no < min_no)
        {
            no   = max_no;
            p_inventory = p_dec->inventory + (no - 100);
        }

        if(no == bcd_number_to_dec(page_no))
        {
            *p_prev_page_no = dec_number_to_bcd(no);
            return VBI_RC_FAILED;
        }

        if(p_inventory->cached != 0)
        {
            *p_prev_page_no = dec_number_to_bcd(no);
            break;
        }
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_next_raw_data_vsb(ttx_decoder_t *p_dec
                                   , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw)
{
    ttx_magazine_t  *p_mag = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d   = NULL;

    *pp_raw = NULL;

    for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
    {
        if(p_d->page_no > page_no
                && vbi_is_bcd_vsb(p_d->page_no) == TRUE
                && vbi_is_bcd_vsb(p_d->sub_no & sub_no_mask) == TRUE)
        {
            break;
        }
    }

    if(p_d != NULL)
    {
        *pp_raw = p_d;
    }
    else
    {
        if(((page_no & 0xF00) >> 8) == 8)
            p_mag = &p_dec->mag[0];
        else
            p_mag = &p_dec->mag[((page_no & 0xF00) >> 8)];

        *pp_raw = p_mag->p_raw;
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_prev_raw_data_vsb(ttx_decoder_t *p_dec
                                   , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw)
{
    ttx_magazine_t  *p_mag = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d   = NULL;

    *pp_raw = NULL;

    for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
    {
        if(p_d->p_next != NULL
                && p_d->p_next->page_no >= page_no
                && vbi_is_bcd_vsb(p_d->page_no) == TRUE
                && vbi_is_bcd_vsb(p_d->sub_no & sub_no_mask) == TRUE)
        {
            if(p_d->page_no != page_no)
            {
                *pp_raw = p_d;
            }
            else
            {
                if(((page_no & 0xF00) >> 8) == 1)
                    p_mag = &p_dec->mag[7];
                else
                    p_mag = &p_dec->mag[((page_no & 0xF00) >> 8) - 2];

                for(p_d = p_mag->p_raw; p_d != NULL; p_d = p_d->p_next)
                {
                    if(p_d->p_next == NULL)
                        break;
                }

                *pp_raw = p_d;
            }

            return VBI_RC_SUCCESS;
        }
    }

    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_next_raw_subpage_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw)
{
    ttx_magazine_t  *p_mag = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d   = NULL;
    ttx_raw_t       *p_sub = NULL;

    *pp_raw = NULL;

    for(p_sub = p_mag->p_raw; p_sub != NULL; p_sub = p_sub->p_next)
    {
        if(p_sub->page_no >= page_no)
        {
            if(p_sub->page_no == page_no)
                break;
            else
            	{
                return VBI_RC_FAILED;
            	}
        }
    }

    if(p_sub == NULL)
    	{
        return VBI_RC_FAILED;
    	}

if(p_dec->display_page.manual_flag == 0)
{
			p_dec->display_page.manual_flag = 1;
			*pp_raw = p_sub;
			return VBI_RC_SUCCESS;
}
else
{
    for(p_d = p_sub
              ; p_d != NULL && p_d->page_no == p_sub->page_no
            ; p_d = p_d->p_next)
    {
        if((p_d->sub_no & sub_no_mask) > (sub_no & sub_no_mask)
                && vbi_is_bcd_vsb(p_d->sub_no & sub_no_mask))
        {
        	p_dec->display_page.manual_flag = 1;
            *pp_raw = p_d;
            return VBI_RC_SUCCESS;
        }
    }

    	  p_dec->display_page.manual_flag = 0;
         *pp_raw = NULL;
}
    return VBI_RC_SUCCESS;
}
vbi_rc_t ttx_get_prev_raw_subpage_vsb(ttx_decoder_t *p_dec
                                      , s16 page_no, u16 sub_no, u16 sub_no_mask, ttx_raw_t **pp_raw)
{
    ttx_magazine_t  *p_mag = &p_dec->mag[((page_no & 0xF00) >> 8) - 1];
    ttx_raw_t       *p_d = NULL, *p_prev = NULL;
    ttx_raw_t       *p_sub = NULL;
    *pp_raw = NULL;
    for(p_sub = p_mag->p_raw; p_sub != NULL; p_sub = p_sub->p_next)
    {
        if(p_sub->page_no >= page_no)
        {
            if(p_sub->page_no == page_no)
                break;
            else
                return VBI_RC_FAILED;
        }
    }

    if(p_sub == NULL)
        return VBI_RC_FAILED;
#if 0
    if((p_sub->sub_no & sub_no_mask) == (sub_no & sub_no_mask))
    {
        for(p_d = p_sub; p_d != NULL; p_d = p_d->p_next)
        {
            if((NULL == p_d->p_next) ||(p_d->p_next->page_no != page_no))//only one page in the raw list
            {
                *pp_raw = p_d;
                return VBI_RC_SUCCESS;
            }
        }
    }
#endif
   if(p_dec->display_page.manual_flag == 0)
   {
   	for(p_d = p_sub; p_d != NULL; p_d = p_d->p_next)
	{
		if((p_d->p_next == NULL) || (p_d->p_next->page_no != p_sub->page_no))
		{
			p_dec->display_page.manual_flag = 1;
			*pp_raw = p_d;
			return VBI_RC_SUCCESS;
		}
	}
   }
   else
   {
    	for(p_prev = p_sub, p_d = p_sub->p_next; (p_d != NULL) && (p_d->page_no == p_sub->page_no); p_prev = p_d, p_d = p_d->p_next)
    	{
        	if(((p_d->sub_no & sub_no_mask) == (sub_no & sub_no_mask)) && vbi_is_bcd_vsb(p_d->sub_no & sub_no_mask))
        	{
        		p_dec->display_page.manual_flag = 1;
            		*pp_raw = p_prev;	
            		return VBI_RC_SUCCESS;
        	}
    	}
    	  p_dec->display_page.manual_flag = 0;
         *pp_raw = NULL;
   	}
    return VBI_RC_SUCCESS;
}

vbi_rc_t ttx_get_raw_page_entry_vsb(ttx_decoder_t *p_dec
                                    , s16 page_no, ttx_raw_inventory_t **pp_inventory)
{
    if(page_no < 0x100 || page_no > 0x8ff)
        return VBI_RC_FAILED;

    page_no -= 0x100;

    *pp_inventory = p_dec->inventory + bcd_number_to_dec(page_no);

    return VBI_RC_SUCCESS;
}
#if 0
vbi_rc_t ttx_recycle_vsb(ttx_decoder_t *p_dec, ttx_page_priority_t sprio, ttx_raw_t **pp_buf)
{
    u32         i = 0;
    vbi_rc_t    rc;
    u16         recycle = p_dec->recycle;
    ttx_raw_t   *p_d    = p_dec->p_raw + recycle;
    MT_BOOL        find    = FALSE;
    ttx_page_priority_t dprio;
    ttx_raw_t *p_t = NULL;

    rc = ttx_get_raw_page_vsb(p_dec
                              , p_dec->waiting_page.page_no
                              , p_dec->waiting_page.sub_no
                              , 0xff
                              , &p_t);
    if(rc != VBI_RC_SUCCESS)
    {
        p_t = NULL;
    }

    if (sprio == TTX_PAGE_PRI_LOWEST)
        sprio ++;

    while(sprio)
    {
        dprio = p_d->priority;
        if(p_d->page_no == p_dec->waiting_page.page_no
                && p_d->sub_no == p_dec->waiting_page.sub_no)
        {
            dprio = TTX_PAGE_PRI_WAITING_PAGE;
        }
        else if (p_d->page_no == p_dec->waiting_page.page_no)
        {
            dprio = TTX_PAGE_PRI_LINK_PAGE;
        }
        else if(p_t != NULL)
        {
            if (p_d->page_no == p_t->data.lop.link[0].page_no
                    || p_d->page_no == p_t->data.lop.link[1].page_no
                    || p_d->page_no == p_t->data.lop.link[2].page_no
                    || p_d->page_no == p_t->data.lop.link[3].page_no)
            {
                dprio = TTX_PAGE_PRI_LINK_PAGE;
            }
        }
        else
        {
            u32 tmp1 = 0, tmp2 = 0, tmp3 = 0;

            tmp1 = ((p_d->page_no & 0xf00) >> 8) * 100
                   + ((p_d->page_no & 0xf00) >> 8) * 10
                   + ((p_d->page_no & 0xf00) >> 8);

            tmp2 = ((p_t->page_no & 0xf00) >> 8) * 100
                   + ((p_t->page_no & 0xf00) >> 8) * 10
                   + ((p_t->page_no & 0xf00) >> 8);

            tmp3 = p_dec->max_page_num / 5;

            if((tmp1 < (tmp2 + tmp3) && tmp1 > (tmp2 - tmp3))
                    && (p_d->sub_no == TTX_NULL_SUBPAGE
                        || p_d->sub_no == TTX_ANY_SUBPAGE
                        || p_d->sub_no == TTX_FIRST_SUBPAGE))
            {
                dprio = TTX_PAGE_PRI_LINK_PAGE;
            }
        }

        if (dprio < sprio)
        {
            for(i = 0; i < 8; i ++)
            {
                if(p_d == p_dec->mag[i].p_cache)
                    break;
            }
            if(i >= 8)
                find = TRUE;
        }

        if(find == TRUE)
            break;

        recycle ++;
        p_d ++;
        if(recycle >= p_dec->max_page_num)
        {
            recycle = 0;
            p_d     = p_dec->p_raw;
        }

        if(recycle == p_dec->recycle)
        {
            if (sprio == TTX_PAGE_PRI_WAITING_PAGE)
                break;

            sprio ++;
        }
    }

    if(find != TRUE)
    {
        *pp_buf = NULL;
        return VBI_RC_FAILED;
    }

    p_dec->recycle = recycle;

    ttx_remove_raw_page_vsb(p_dec, p_d);
    memset(p_d, 0, sizeof(ttx_raw_t));
    p_d->function = TTX_PAGE_FUNC_DISCARD;

    *pp_buf = p_d;

    return VBI_RC_SUCCESS;
}
#endif
