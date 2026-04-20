/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "mt_type.h"

#include "drv_dev_priv.h"

#include "vbi_inserter.h"
#include "vbi_inserter_priv.h"
//#include "hal_concerto_irq.h"
//#include "../../pti/concerto/dmx_reg_concerto.h"
#include "vbi_inserter_aria.h"
#include "mtos_mem.h"
#include "mtos_printk.h"
#include "hal_demux_regs.h"
#include "mtos_int.h"
#include "mt_debug.h"
//#include "../../glbinfo/jazz/drv_global.h"

#define AV_CONFIG_2        0xBF232008
#define AV_CONFIG_5        0xBF232014
#define VBI_BASE_ADDR 0xbf450000
#define VBI_CTRL_REG (VBI_BASE_ADDR + 0x00)
#define LINE_EN_REG (VBI_BASE_ADDR + 0x20)
#define PHY_CTRL_REG (VBI_BASE_ADDR + 0x24)
#define AMP_COEF_REG (VBI_BASE_ADDR + 0x28)
#define VBI_INT_STATE (VBI_BASE_ADDR + 0x3c)
#define VBI_FIFO_CNT (VBI_BASE_ADDR + 0x30)
#define VBI_FIFO (VBI_BASE_ADDR + 0x54)
#define VBI_SOFT_CTRL (VBI_BASE_ADDR + 0x04)
#define VBI_INT_MASK_N (VBI_BASE_ADDR + 0x34)

#define VBI_EN_BIT          (1<<0)
#define TTTEX_EN_BIT        (1<<16)
#define TTTEX_DEC_EN_BIT    (1<<17)
#define CC_EN_BIT           (3<<20)
#define CC_DEC_EN_BIT       (1<<22)
#define VPS_EN_BIT          (1<<24)
#define WSS_EN_BIT          (1<<28)

#define SOFTWARE_CTRL (VBI_BASE_ADDR + 0x04)
#define CC_DATA_REG (VBI_BASE_ADDR + 0x08)
#define WSS_DATA_REG (VBI_BASE_ADDR + 0x1c)
#define VPS_DATA1_REG (VBI_BASE_ADDR + 0x0c)
#define VPS_DATA2_REG (VBI_BASE_ADDR + 0x10)
#define VPS_DATA3_REG (VBI_BASE_ADDR + 0x14)
#define VPS_DATA4_REG (VBI_BASE_ADDR + 0x18)

#define VBI_USER_DATA_DUMP      0x6f810024 //

#define PACKET_START_CODE_PREFIX   0x000001
#define PRIVATE_STREAM_1  0xbd

#define MAX_PES_PACKET_LEN (64 * 1024)

static mt_u32 pes_bak_len = 0;
static mt_u8  pes_bak_buffer[MAX_PES_PACKET_LEN];

static mt_u8 vbi_mode = 0;  // 1: cc   2:ttxt  3 vps  4 wss 5 atsc

static vbi_buf_t vbi_buf = {0};

#define VBI_BUFFER_SIZE      0xc800 //50K
static ulong vbi_get_fifo_by_chipver()
{
    //chip_rev_t  chip_ver;
    ulong vbi_fifo = 0x0;
/*
    chip_ver = hal_get_chip_rev();
    if(CHIP_CONCERTO_B0 <= chip_ver)
    {
        vbi_fifo = VBI_BASE_ADDR + 0x54;
    }
    else
    {
        vbi_fifo = VBI_BASE_ADDR + 0x40;
    }
*/
    vbi_fifo = VBI_BASE_ADDR + 0x54;
    return vbi_fifo;
}
static mt_u32 inl(ulong port)
{
    return *((volatile mt_u32 *)(port));
}

static void outl(ulong port,mt_u32 val)
{
   *((volatile mt_u32 *)(port))= val;
}

/*
the bufferSize should be 4 bytes aligned
*/
static mt_u32 vbi_buf_init(mt_u32 bufferSize)
{
  mt_u8 *p_ptr = NULL;

  p_ptr = (mt_u8 *)(ulong)mtos_malloc(bufferSize);
  if(!p_ptr)
  {
    OS_PRINTF("Failed To SYS_Malloc [%d] \n",bufferSize);
    return 1;
  }
  vbi_buf.p_vbibuffer_head = p_ptr;
  vbi_buf.p_vbibuffer_tail = (mt_u8 *)((ulong)p_ptr + bufferSize);
  vbi_buf.p_vbibuffer_rdp = vbi_buf.p_vbibuffer_head;
  vbi_buf.p_vbibuffer_wrp = vbi_buf.p_vbibuffer_head;
  vbi_buf.vbibuffer_size = bufferSize;
  OS_PRINTF("s_VBI_Buffer.p_vbibuffer_head = 0x%x\n",vbi_buf.p_vbibuffer_head);
  OS_PRINTF("s_VBI_Buffer.vbibuffer_size = 0x%x\n",vbi_buf.vbibuffer_size);

  return 0;
}

static mt_u32 vbi_buf_free(void)
{
  mtos_free(vbi_buf.p_vbibuffer_head );
  return 0;
}

static mt_u32 vbi_put_data(mt_u8 *pBuf, mt_u32 Len)
{
  mt_u32 length = 0;
  mt_u8 *p_databuf = pBuf;
  if(p_databuf == NULL)
  {
    OS_PRINTF("[%s][%d]Error pBuf NULL\n", __FUNCTION__, __LINE__);
    return 1;
  }
  if(Len > vbi_buf.vbibuffer_size)
  {
    OS_PRINTF("[%s][%d]Fatal Error Len(%d) > %d\n", __FUNCTION__, __LINE__, Len, vbi_buf.vbibuffer_size);
    return 1;
  }
  if((vbi_buf.p_vbibuffer_wrp + Len) > vbi_buf.p_vbibuffer_tail)
  {
    length = vbi_buf.p_vbibuffer_tail - vbi_buf.p_vbibuffer_wrp;
    memcpy(vbi_buf.p_vbibuffer_wrp, p_databuf, length);
    vbi_buf.p_vbibuffer_wrp = vbi_buf.p_vbibuffer_head;
    p_databuf += length;
    memcpy(vbi_buf.p_vbibuffer_wrp, p_databuf, Len - length);
    vbi_buf.p_vbibuffer_wrp += (Len - length);
  }
  else
  {
    memcpy(vbi_buf.p_vbibuffer_wrp, p_databuf, Len);
    vbi_buf.p_vbibuffer_wrp += Len;
  }
  return 0;

}

static mt_u32 vbi_get_data(mt_u8 *pBuf, mt_u32 Len)
{
  mt_u32 length = 0;
  mt_u8 *p_databuf = pBuf;

  if(p_databuf == NULL)
  {
    OS_PRINTF("[%s][%d]Error pBuf NULL\n", __FUNCTION__, __LINE__);
    return 1;
  }
  if(Len > vbi_buf.vbibuffer_size)
  {
    OS_PRINTF("[%s][%d]Fatal Error Len(%d) > %d\n",
    __FUNCTION__, __LINE__, Len, vbi_buf.vbibuffer_size);
    return 1;
  }
  if((vbi_buf.p_vbibuffer_rdp + Len) > vbi_buf.p_vbibuffer_tail)
  {
    length = vbi_buf.p_vbibuffer_tail - vbi_buf.p_vbibuffer_rdp;
    memcpy(p_databuf, vbi_buf.p_vbibuffer_rdp, length);
    vbi_buf.p_vbibuffer_rdp = vbi_buf.p_vbibuffer_head;
    p_databuf += length;
    memcpy(p_databuf, vbi_buf.p_vbibuffer_rdp, Len - length);
    vbi_buf.p_vbibuffer_rdp += (Len - length);
  }
  else
  {
    memcpy(p_databuf, vbi_buf.p_vbibuffer_rdp, Len);
    vbi_buf.p_vbibuffer_rdp += Len;
  }
  return 0;
}

static void vbi_get_buf_space(mt_u32 *freespace, mt_u32 *usedspace)
{
  mt_u32 freelength = 0;


  if(vbi_buf.p_vbibuffer_wrp < vbi_buf.p_vbibuffer_rdp)
  {
    freelength = vbi_buf.p_vbibuffer_rdp - vbi_buf.p_vbibuffer_wrp - 1;
  }
  else
  {
    freelength = vbi_buf.vbibuffer_size + vbi_buf.p_vbibuffer_rdp - vbi_buf.p_vbibuffer_wrp - 1;
  }
  if(freespace != NULL)
    *freespace = freelength;
  if(usedspace != NULL)
    *usedspace = vbi_buf.vbibuffer_size - freelength - 1;
}

mt_u32 vbi_parse_ESpacket(mt_u8 *pBuffer,mt_u32 Len)
{
  mt_u8 data_identifier = 0;
  mt_u8 data_unit_id = 0;
  mt_u8 data_unit_length = 0;
  mt_u8 data_unit_id_offset = 0;
  mt_u8 data_unit_length_offset = 0;
  mt_u32 offset = 0;

  data_identifier = pBuffer[0];

  if(!(((data_identifier >= 0x10)&&(data_identifier <= 0x1f)) ||
    ((data_identifier >= 0x99)&&(data_identifier <= 0x9b))))
  {
    return 0;
  }
  pBuffer++;/*pointer to data unit*/
  Len--;
  data_unit_id_offset = 0;
  data_unit_length_offset = 1;
  offset = 0;
  while(Len > 0)
  {
    data_unit_id = pBuffer[offset + data_unit_id_offset];
    data_unit_length = pBuffer[offset + data_unit_length_offset];

    if((data_unit_id == 0x02) || (data_unit_id == 0x03) || (data_unit_id == 0xc0)
      || (data_unit_id == 0xc1) || (data_unit_id == 0xc3) || (data_unit_id == 0xc4)
      || (data_unit_id == 0xc5))
    {

      if(vbi_mode == 1)//cc
      {
        pBuffer[offset + 2] = pBuffer[offset + 2] & 0x3f;
        vbi_put_data(&pBuffer[offset + 2], 3);
      }

      if(vbi_mode == 2)//TTX
      {
        pBuffer[offset + 2] = pBuffer[offset + 2] & 0x3f;
        vbi_put_data(&pBuffer[offset + 2], 44);
      }

      if(vbi_mode == 3)//vps
      {
        pBuffer[offset + 2] = pBuffer[offset + 2] & 0x3f;
        vbi_put_data(&pBuffer[offset + 2], 14);
      }
      else if(vbi_mode == 4) //wss
      {
        pBuffer[offset + 2] = pBuffer[offset + 2] & 0x3f;
        vbi_put_data(&pBuffer[offset + 2], 3);
      }
      offset += (data_unit_length + 2);
      if(Len > (data_unit_length + 2))
      {
        Len = Len - data_unit_length - 2;
      }
      else
      {
        Len = 0;
      }
    }
    else if(data_unit_id == 0xff)
    {
      offset += (data_unit_length + 2);
      if(Len > (data_unit_length + 2))
      {
        Len = Len - data_unit_length - 2;
      }
      else
      {
        Len = 0;
      }
    }
    else
    {
       OS_PRINTF("\nInvalid data field\n");
       return 1;
    }

  }

  return 0;
}

mt_u32 vbi_parse_PEStoES(mt_u8 *pesBuffer, mt_u32 pesLen, mt_u8 *esBuffer, mt_u8 *esLen)
{
  mt_u32 offset = 0;
  mt_u32 start_code_prefix = 0;

  start_code_prefix = pesBuffer[0];
  start_code_prefix = (start_code_prefix << 8) + pesBuffer[1];
  start_code_prefix = (start_code_prefix << 8) + pesBuffer[2];

  if((start_code_prefix != PACKET_START_CODE_PREFIX) ||(pesBuffer[3] != PRIVATE_STREAM_1))
  {
    return 1;
  }
  offset = 9;
  offset += pesBuffer[8];

  if(offset > pesLen)
    return 1;

  *(mt_u32 *)esBuffer = (ulong)(pesBuffer + offset);
  *(mt_u32 *)esLen = pesLen - offset;

  return 0;

}

mt_u8 vbi_parse_TStoPES(mt_u8 * one_ts)
{
  mt_u32 PaylaodLength = 0;
  mt_u8  *p_TsHead = 0;
//  mt_u8  TsContinueCount= 0x10;
//  int ts_lost;
  mt_u16 mt_u16TsPid = 0;
  mt_u8   * p_dest = 0;
  mt_u32 * p_dest_len = 0;

  p_TsHead = one_ts;

  mt_u16TsPid = (((mt_u16)(p_TsHead[1] & 0x1f)) << 8) | ((mt_u16)p_TsHead[2]);
  if(mt_u16TsPid == 0 || mt_u16TsPid >= 0x1fff)
  {
    return 0;
  }
  p_dest = pes_bak_buffer;
  p_dest_len = &pes_bak_len;
  if (((*(p_TsHead + 1)) & 0x80) != 0)
  {

    *p_dest_len = 0;
    return 2;
  }
  if(*p_dest_len == 0)
  {
    if(((*(p_TsHead + 1)) & 0x40) != 0)/*¨°???PES¦Ì??a¨º?*/
    {
      if(((*(p_TsHead + 3)) & 0x30) == 0x30)        /*¨®D¨¬?3?*/
      {

        PaylaodLength = 188 - 4 - 1 - *(p_TsHead + 4);
        if((PaylaodLength + *p_dest_len) < MAX_PES_PACKET_LEN)
        {
          memcpy(p_dest + *p_dest_len, p_TsHead + (188 - PaylaodLength), PaylaodLength);
          *p_dest_len += PaylaodLength;
        }
        else
        {

        }
      }
      else if(((*(p_TsHead + 3)) & 0x30) == 0x10)            /*?T¨¬?3? */
      {

        PaylaodLength = 184;
        if((PaylaodLength + *p_dest_len) < MAX_PES_PACKET_LEN)
        {
          memcpy(p_dest + *p_dest_len, p_TsHead + (188 - PaylaodLength), PaylaodLength);
          *p_dest_len += PaylaodLength;
        }
        else
        {

        }
      }
      else
      {

      }
    }
    else
    {

    }
  }
  else
  {
    if(((*(p_TsHead + 1)) & 0x40) != 0)       /*D?¦Ì?RTSP¡ã¨¹¨°??a¨º?¡ê?¡¤¡é?¨ª¨°?¨ª¨º3¨¦¦Ì?*/
    {

      return 1;
    }

    if(((*(p_TsHead + 3)) & 0x30) == 0x30)                    /*¨®D¨¬?3?*/
    {

      PaylaodLength = 188 - 4 - 1 - *(p_TsHead + 4);
      if((PaylaodLength + *p_dest_len) < MAX_PES_PACKET_LEN)
      {
        memcpy(p_dest + *p_dest_len, p_TsHead + (188 - PaylaodLength), PaylaodLength);
        *p_dest_len += PaylaodLength;
      }
      else
      {

      }
    }
    else if(((*(p_TsHead + 3)) & 0x30) == 0x10)                 /*?T¨¬?3?*/
    {

      PaylaodLength = 184;
      if((PaylaodLength + *p_dest_len) < MAX_PES_PACKET_LEN)
      {
        memcpy(p_dest + *p_dest_len, p_TsHead + (188 - PaylaodLength), PaylaodLength);
        *p_dest_len += PaylaodLength;
      }
      else
      {

      }
    }
    else
    {

    }
  }

  return 0;

}

/*
*********************************************************************************************
*                                         PTI_CallBackTsRev
*
* Description: PTI CallBack TS Rev
* Arguments  :
*
* Returns    : ERROR_CODE_NO_ERROR
*********************************************************************************************
*/
void PTI_CallBackTs2PES(
    mt_u8 *buff,
    mt_u16 len)
{
  mt_u32 err = 0.;
  mt_u8  mt_s8Result = 0;
  ulong esbuffer = 0;
  mt_u32 es_length = 0;


  /*PTI TS¨ºy?Y??¦Ì¡Âo¡¥¨ºy,¨¤¡ä¨¢?¨°???TS¡ã¨¹¦Ì?¨ºy?Y¡ê??¡À?¨®¡Áa3¨¦?aPES¡ã¨¹¨¨?o¨®¡ä?¨¨??o3???*/
  //OS_PRINTF("buf addr %x len %d cc %x\n", buff, len,buff[3] & 0x0F);

  mt_s8Result = vbi_parse_TStoPES(buff);
  //OS_PRINTF("mt_s8Result = %d\n", mt_s8Result);
  if(mt_s8Result == 1)
  {

    err = vbi_parse_PEStoES(pes_bak_buffer,pes_bak_len, (mt_u8 *)&esbuffer, (mt_u8 *)&es_length);
    if(err)
    {
      OS_PRINTF("Failed[%x] at Line %d of file:%s\n",err,__LINE__,__FILE__);
      return;
    }
    //OS_PRINTF("esbuffer:%x es len %d\n", esbuffer, es_length);
    err = vbi_parse_ESpacket((mt_u8 *)esbuffer, es_length);
    if(err)
    {
      OS_PRINTF("Failed[%x] at Line %d of file:%s\n",err,__LINE__,__FILE__);
      return;
    }
    else
    {
      //OS_PRINTF("succ[%x] at Line %d of file:%s\n",err,__LINE__,__FILE__);
    }
    pes_bak_len = 0;
#if 1
    /*???¡ä¨°?¡ä?¡ê?¨¦?¨°???¡ã¨¹?¨¢?2o¨ª¦Ì¡À?¡ã¡ã¨¹¦Ì??a¨º??¨²¨ª?¨°???TS¡ã¨¹?¨²*/
    mt_s8Result = vbi_parse_TStoPES(buff);
    if(mt_s8Result == 1)
    {
      OS_PRINTF("Get PES2:%d\n",pes_bak_len);
      //displayHex(pes_bak_buffer,16);
      err = vbi_parse_PEStoES(pes_bak_buffer,pes_bak_len, (mt_u8 *)&esbuffer, (mt_u8 *)&es_length);
      if(err)
      {
        OS_PRINTF("Failed[%x] at Line %d of file:%s\n", err, __LINE__, __FILE__);
        return;
      }
      err = vbi_parse_ESpacket((mt_u8 *)esbuffer, es_length);
      if(err)
      {
        OS_PRINTF("Failed[%x] at Line %d of file:%s\n", err, __LINE__, __FILE__);
        return;
      }
      pes_bak_len = 0;
    }
#endif
  }
}

mt_u32 PTI_VBI_TS_CallBack(
  mt_u8 *pBUffer,
  mt_u16 Len)
{
  //if(s_VBI_Status_Flag == VBI_STATUS_START)
  {

          PTI_CallBackTs2PES(pBUffer, Len);
  }
  return 1;
}

extern mt_u32 drv_glbalhandlerregister(mt_u8 devid, mt_u8 GlbInx, void (*p_handler)(void));


static mt_u32 vbi_wp = 0;
static mt_u32 vbi_rp = 0;
static ulong vbi_buf_start = 0;
static mt_u32 vbi_buf_size = 0;

void vbi_rp_reset(void)
{
  vbi_buf_start = reg_get_trpp_ch_data_start_addr_trpp_ch_data_saddr(2) << 3;
  vbi_buf_size = reg_get_trpp_ch_data_end_addr_trpp_ch_data_eaddr(2)-vbi_buf_start+1;
  OS_PRINTF("~vbi_buf_start=0x%08x~vbi_buf_size=0x%08x\n",
    vbi_buf_start, vbi_buf_size);
}

#define BIGL(A)        ((((unsigned int)((ulong)A) & 0xff000000) >> 24) | \
                                                       (((unsigned int)(A) & 0x00ff0000) >> 8) | \
                                                       (((unsigned int)(A) & 0x0000ff00) << 8) | \
                                                       (((unsigned int)(A) & 0x000000ff) << 24))


void vbi_isr(void)
{
  int i = 0;

  unsigned int state = *((volatile unsigned int *)(VBI_INT_STATE));

  unsigned int N_FIFO = *((volatile unsigned int *)(VBI_FIFO_CNT));

  unsigned int vbi_date = 0;

  int num = 0;

  int have_date = 0;
  //OS_PRINTF("[0]state:%d\n",state);
  state = *((volatile unsigned int *)(VBI_INT_STATE));
  //OS_PRINTF("state after read:%d  0x%08x\n", state,vbi_get_fifo_by_chipver());
//  if(state & (1<<5))
//    OS_PRINTF("state:%d\n", state);
    vbi_rp = reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(2);
    vbi_wp = reg_get_trpp_ch_data_wr_addr_trpp_ch_data_waddr(2);
    //OS_PRINTF("vbi_rp:0x%x  vbi_wp:0x%x\n", vbi_rp, vbi_wp);

    if(vbi_wp >= vbi_rp)
    {
      vbi_date = (vbi_wp - vbi_rp) / 64;
    }
    else
    {
      vbi_date = (vbi_buf_size - (vbi_rp - vbi_wp)) / 64;
    }

    num = N_FIFO / 16;

//    test ++;
//    if(test%10 == 0)
//      OS_PRINTF("num=%d,vbi=%d\n", num, vbi_date);

    while(num > 0 && vbi_date > 0)
    {
       have_date = 1;
      for(i = 0; i < 16; i++)
      {
        *((volatile unsigned int *)(vbi_get_fifo_by_chipver())) = BIGL(*(unsigned int *)((vbi_buf_start + vbi_rp) |0xa0000000));
        vbi_rp = vbi_rp + 4;
        if(vbi_rp == vbi_buf_size)
            vbi_rp = 0;
      }
      reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(2, vbi_rp);
      vbi_date--;
      num--;
    }
    if(have_date != 0)
      *((volatile unsigned int *)(VBI_SOFT_CTRL)) |= (1 << 8);
}

void vbi_sw_ttx_isr(void)
{
  //unsigned int state = *((volatile unsigned int *)(VBI_INT_STATE));
  unsigned int N_FIFO = *((volatile unsigned int *)(VBI_FIFO_CNT));
  int have_date = 0;
  mt_u32 usedspace = 0;
  mt_u32 cnt = 0;
  vbi_get_buf_space(NULL, &usedspace);
//  OS_PRINTF("usedspace:%d, N_FIFO:%d\n",usedspace, N_FIFO);
  while((N_FIFO > 0) && (cnt < usedspace))
  {
    have_date = 1;
    *((volatile unsigned int *)(vbi_get_fifo_by_chipver())) = (vbi_buf.p_vbibuffer_rdp[0] << 24)
                                         | (vbi_buf.p_vbibuffer_rdp[1] << 16)
                                         | (vbi_buf.p_vbibuffer_rdp[2] << 8)
                                         | vbi_buf.p_vbibuffer_rdp[3];
//  OS_PRINTF("%08x",*((volatile unsigned int *)(vbi_get_fifo_by_chipver())));
    vbi_buf.p_vbibuffer_rdp += 4;
    N_FIFO--;
    cnt++;
    if(vbi_buf.p_vbibuffer_rdp == vbi_buf.p_vbibuffer_tail)
      vbi_buf.p_vbibuffer_rdp = vbi_buf.p_vbibuffer_head;
  }
//  OS_PRINTF("cnt:%d\n",cnt);
  if(have_date != 0)
     *((volatile unsigned int *)(VBI_SOFT_CTRL)) |= (1 << 8);
}


void vbi_sw_cc_isr(void)
{
  //unsigned int state = *((volatile unsigned int *)(VBI_INT_STATE));
  mt_u32 usedspace = 0;
  mt_u32 err = 0;
  mt_u8 ccdata[3] = {0};
  mt_u32 data = 0;
  mt_u32 field_offset = 0;
  mt_u32 odd_flag = 0;

  vbi_get_buf_space(NULL, &usedspace);
//  OS_PRINTF("usedspace:%d\n", usedspace);
  if(usedspace >= 3)
  {
      err = vbi_get_data(ccdata, 3);
      if(err)
      {
          OS_PRINTF("Failed[%x] at Line %d of file:%s\n", err, __LINE__, __FILE__);
          return;
       }

      field_offset = ccdata[0];
      data = ccdata[1];
      data = (data << 8) + ccdata[2];

      if(field_offset & 0x20)
          odd_flag = 1;

      if(odd_flag == 1)
      {
      	  mt_u32 data_tmp = 0;
         mt_u32 ctrl_tmp = 0;
      	  data_tmp = inl(CC_DATA_REG);
	  data_tmp = (data ) | (data_tmp & 0xffff0000);
         outl(CC_DATA_REG, data);
	  ctrl_tmp = inl(SOFTWARE_CTRL) ;
         field_offset = (field_offset | (1 << 16) |(ctrl_tmp & 0x100000));
         outl(SOFTWARE_CTRL, field_offset);
	  //OS_PRINTF("send cc odd data[0x%08x] ctrl[0x%08x]\n", data_tmp,field_offset);
      }
      else
      {
      	  mt_u32 data_tmp = 0;
         mt_u32 ctrl_tmp = 0;
      	  data_tmp = inl(CC_DATA_REG);
	  data_tmp = (data << 16) | (data_tmp & 0x0000ffff);
         outl(CC_DATA_REG, data_tmp);
         ctrl_tmp = inl(SOFTWARE_CTRL) ;
         field_offset = (field_offset | (1 << 20) |(ctrl_tmp & 0x10000));
         outl(SOFTWARE_CTRL, field_offset);
         //OS_PRINTF("send cc even data[0x%08x] ctrl[0x%08x]\n", data_tmp,field_offset);
      }
  }
}


void vbi_vps_isr(void)
{
  //unsigned int state = *((volatile unsigned int *)(VBI_INT_STATE));
  mt_u32 usedspace = 0;
  mt_u32 err = 0;
  mt_u8 vpsdata[14] = {0};
  mt_u32 data = 0;
  mt_u32 field_offset = 0;

  vbi_get_buf_space(NULL, &usedspace);
//  OS_PRINTF("usedspace:%d\n", usedspace);
  if(usedspace >= 14)
  {
      err = vbi_get_data(vpsdata, 14);
      if(err)
      {
          OS_PRINTF("Failed[%x] at Line %d of file:%s\n", err, __LINE__, __FILE__);
          return;
      }

      field_offset = vpsdata[0];
      data = vpsdata[1];
      data = (data << 8) + vpsdata[2];
      data = (data << 8) + vpsdata[3];
      data = (data << 8) + vpsdata[4];
      outl(VPS_DATA1_REG, data);
      data = vpsdata[5];
      data = (data << 8) + vpsdata[6];
      data = (data << 8) + vpsdata[7];
      data = (data << 8) + vpsdata[8];
      outl(VPS_DATA2_REG, data);
      data = vpsdata[9];
      data = (data << 8) + vpsdata[10];
      data = (data << 8) + vpsdata[11];
      data = (data << 8) + vpsdata[12];
      outl(VPS_DATA3_REG, data);
      data = vpsdata[13];
      outl(VPS_DATA4_REG, data);
      field_offset = field_offset | (1 << 24);
      outl(SOFTWARE_CTRL, field_offset);
      OS_PRINTF("#");
    }
}

void vbi_wss_isr(void)
{
  //unsigned int state = *((volatile unsigned int *)(VBI_INT_STATE));
  mt_u32 usedspace = 0;
  mt_u32 err = 0;
  mt_u8 wssdata[3] = {0};
  mt_u32 data = 0;
  mt_u32 data1 = 0;
  mt_u32 data2 = 0;
  mt_u32 field_offset = 0;

  vbi_get_buf_space(NULL, &usedspace);
//  OS_PRINTF("usedspace:%d\n", usedspace);
  if(usedspace >= 3)
  {
      err = vbi_get_data(wssdata, 3);
      if(err)
      {
          OS_PRINTF("Failed[%x] at Line %d of file:%s\n", err, __LINE__, __FILE__);
          return;
      }

      field_offset = wssdata[0];
      data1 = wssdata[1] << 6;
      data2 = wssdata[2] >> 2;
      data = data1 + data2;
      outl(WSS_DATA_REG, data);
      field_offset = field_offset | (1 << 28);
      outl(SOFTWARE_CTRL, field_offset);
      OS_PRINTF("&");
    }
}

static mt_s32 aria_vbi_inserter_start(void *p_priv, mt_u32 cfg)
{
    mt_u32 vbi_control_reg_val = 0;
    OS_PRINTF("\n aria_vbi_inserter_start  cfg=0x%x\n",cfg);

    vbi_control_reg_val = inl(VBI_CTRL_REG);

    if(!(cfg & VBI_ALL_INSERTION))
        return MT_FAILURE;

    //only run once to register isr in av cpu
    if(VBI_INSERTION_CC == (cfg & VBI_INSERTION_CC))
    {
        vbi_rp_reset();
        vbi_mode = 1;
        vbi_control_reg_val = (CC_DEC_EN_BIT +(3<<20)+ VBI_EN_BIT);
        outl(VBI_CTRL_REG, vbi_control_reg_val);
        OS_PRINTF("start  cc\n");
        mtos_irq_request(21, vbi_isr, 1);
        //enable fifo half empty and fifo empty interrupt
        outl(VBI_INT_MASK_N, 0x7);
    }

    if(VBI_INSERTION_TTX == (cfg & VBI_INSERTION_TTX))
    {
        vbi_rp_reset();
        OS_PRINTF("start  ttx\n");
        vbi_mode = 2;
        vbi_control_reg_val = (TTTEX_DEC_EN_BIT + TTTEX_EN_BIT + VBI_EN_BIT);
        OS_PRINTF("start  ttx vbi_control_reg_val = 0x%x\n",vbi_control_reg_val);
        outl(VBI_CTRL_REG,vbi_control_reg_val);
        OS_PRINTF("start  ttx set ok vbi_control_reg_val=0x%08x\n", inl(VBI_CTRL_REG));
        //outl(0xbf450034,0x4);
        mtos_irq_request(21, vbi_isr, 1);
        //enable fifo half empty and fifo empty interrupt
        outl(VBI_INT_MASK_N, 0x7);
        OS_PRINTF("start  ttx isr 21 ok\n");
    }

    if(VBI_INSERTION_WSS == (cfg & VBI_INSERTION_WSS))
    {
        vbi_buf_init(VBI_BUFFER_SIZE);
        vbi_control_reg_val =  (WSS_EN_BIT + VBI_EN_BIT);
        outl(VBI_CTRL_REG, vbi_control_reg_val);
        vbi_mode = 4;
        mtos_irq_request(21, vbi_wss_isr, 1);
        //enable top start interrupt
        outl(VBI_INT_MASK_N, 0x10);
        OS_PRINTF("start  wss\n");
    }

    if(VBI_INSERTION_VPS == (cfg & VBI_INSERTION_VPS))
    {
        vbi_buf_init(VBI_BUFFER_SIZE);
        vbi_mode = 3;
        vbi_control_reg_val =  (VPS_EN_BIT + VBI_EN_BIT);
        outl(VBI_CTRL_REG,vbi_control_reg_val);
        mtos_irq_request(21, vbi_vps_isr, 1);
        //enable top start interrupt
        outl(VBI_INT_MASK_N, 0x10);
        OS_PRINTF("start  vps\n");
    }

    if(VBI_INSERTION_ATSC == (cfg & VBI_INSERTION_ATSC))
    {
        OS_PRINTF("start  atsc\n");
    }

    if(VBI_INSERTION_CC_SW == (cfg & VBI_INSERTION_CC_SW))
    {
        vbi_buf_init(VBI_BUFFER_SIZE);

        vbi_mode = 1;
        vbi_control_reg_val =  (CC_EN_BIT + VBI_EN_BIT);
        outl(VBI_CTRL_REG, vbi_control_reg_val);
        mtos_irq_request(21, vbi_sw_cc_isr, 1);
        //enable top start, bot start interrupt
        outl(VBI_INT_MASK_N, 0x18);

        OS_PRINTF("start  sw cc\n");
    }
   if(VBI_INSERTION_TTX_SW == (cfg & VBI_INSERTION_TTX_SW))
    {
        vbi_buf_init(VBI_BUFFER_SIZE);
        vbi_mode = 2;
        vbi_control_reg_val =  (TTTEX_EN_BIT + VBI_EN_BIT);
        outl(VBI_CTRL_REG,vbi_control_reg_val);
        mtos_irq_request(21, vbi_sw_ttx_isr, 1);
        //enable fifo half empty and fifo empty interrupt
        outl(VBI_INT_MASK_N, 0x7);
        OS_PRINTF("start  sw ttx\n");
    }
    return MT_SUCCESS;

}


static mt_s32 aria_vbi_inserter_stop(void *p_priv, mt_u32 cfg)
{
    mt_u32 dtmp =0;
    OS_PRINTF("\n aria_vbi_inserter_stop\n");
    if((VBI_INSERTION_CC == (cfg & VBI_INSERTION_CC)) ||
        (VBI_INSERTION_TTX == (cfg & VBI_INSERTION_TTX)))
    {
        mtos_irq_release(21, vbi_isr);
    }
    if(VBI_INSERTION_CC_SW == (cfg & VBI_INSERTION_CC_SW))
    {
        mtos_irq_release(21, vbi_sw_cc_isr);
        vbi_buf_free();
    }
    if(VBI_INSERTION_TTX_SW == (cfg & VBI_INSERTION_TTX_SW))
    {
        mtos_irq_release(21, vbi_sw_ttx_isr);
        vbi_buf_free();
    }
    if(VBI_INSERTION_VPS == (cfg & VBI_INSERTION_VPS))
    {
        mtos_irq_release(21, vbi_vps_isr);
        vbi_buf_free();
    }
    if(VBI_INSERTION_WSS == (cfg & VBI_INSERTION_WSS))
    {
        mtos_irq_release(21, vbi_wss_isr);
        vbi_buf_free();
    }
    vbi_mode = 0;
    dtmp = inl(VBI_CTRL_REG);
    dtmp &= ~(VBI_EN_BIT);
    outl(VBI_CTRL_REG, dtmp);
    //disable vbi interrupt
    outl(VBI_INT_MASK_N, 0);
    return MT_SUCCESS;
}


static mt_s32 aria_vbi_inserter_set_video_std(void *p_priv, video_std_t std)
{
    return MT_SUCCESS;
}


static mt_s32 aria_vbi_inserter_inserte_data(void *p_priv,
                                         mt_u32 pts,
                                         mt_u8 *p_pes_data_filed,
                                         mt_u8 *p_pes_last_byte)
{
    return MT_SUCCESS;
}


static long aria_vbi_inserter_open(void *p_inserter, void *p_cfg)
{
    vbi_cfg_t *p_config = (vbi_cfg_t *)p_cfg;
    mt_u32 dtmp = 0;
    //config vbi buffer addr
    dtmp = (1 << 26) | ((p_config->vbi_pes_buf_addr >> 3) & 0x3ffffff);
    outl(AV_CONFIG_2, dtmp);
    //config vbi buffer size
    dtmp = p_config->vbi_pes_buf_size >> 6;
    outl(AV_CONFIG_5, dtmp);
    OS_PRINTF("VBI PES BUF ADDR:0x%08x, VBI_PES_BUF_SIZE:0x%08x:0x%08x\n",
          p_config->vbi_pes_buf_addr, p_config->vbi_pes_buf_size);
    return MT_SUCCESS;
}


//static mt_s32 aria_vbi_inserter_close(vbi_inserter_t *p_inserter)
static mt_s32 aria_vbi_inserter_close(void *p_inserter)
{
    OS_PRINTF("concerto_vbi_inserter_close\n");
    //jazz_vbi_inserter_stop(p_inserter->p_priv
    //     , VBI_ALL_INSERTION);

    return MT_SUCCESS;
}


//static mt_s32 aria_vbi_inserter_io_ctrl(vbi_inserter_t *p_inserter,
//                                           mt_u32 cmd, mt_u32 param)
static signed long aria_vbi_inserter_io_ctrl(void *p_inserter,
										   unsigned long cmd, unsigned long param)
{
    return MT_SUCCESS;
}


//static void aria_vbi_inserter_detach(vbi_inserter_t *p_inserter)
static void aria_vbi_inserter_detach(void *p_inserter)
{
  return;
}


mt_s32 aria_vbi_inserter_attach(char *p_name)
{
    vbi_inserter_device_t *p_dev = NULL;
    struct device_base *p_base = NULL;
    vbi_inserter_t *p_inserter = NULL;

    p_dev = (vbi_inserter_device_t *)dev_allocate(p_name
        , SYS_DEV_TYPE_VBI_INSERTER
        , sizeof(vbi_inserter_device_t), sizeof(vbi_inserter_t));

    if(p_dev == NULL)
    {
      MT_ASSERT(0);
      return MT_FAILURE;
    }

    p_base = (struct device_base *)(p_dev->p_base);
    p_inserter = (vbi_inserter_t *)p_dev->p_priv;

    /* link base function */
    p_base->open = aria_vbi_inserter_open;
    p_base->close = aria_vbi_inserter_close;
    p_base->detach = aria_vbi_inserter_detach;
    p_base->io_ctrl = aria_vbi_inserter_io_ctrl;


    p_inserter->p_start
      = aria_vbi_inserter_start;

    p_inserter->p_stop
      = aria_vbi_inserter_stop;

    p_inserter->p_set_vid_std
      = aria_vbi_inserter_set_video_std;

    p_inserter->p_inserte_data
      = aria_vbi_inserter_inserte_data;

    return MT_SUCCESS;
}
