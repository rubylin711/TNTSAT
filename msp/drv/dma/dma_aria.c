/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <asm/cacheflush.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/io.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#include "mt_type.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dma.h"
#include "hal_dma_regs.h"
#include "dma_aria.h"
#include "mt_drv_log.h"
#include "mt_debug.h"
#include "mt_common.h"
#include "mt_mach/irq.h"


#define DMA_DEBUG 0

#if DMA_DEBUG
#define DMA_PRINT printk		//MT_WARN_LOG
#else
#define DMA_PRINT(...)
#endif

//#ifndef IRQ_DMA_ID
//#define IRQ_DMA_ID (6+32)
//#endif

static MT_BOOL irq_requested_dma = FALSE;
static struct mt_dmac_dev g_dmac_info;

static mt_s32 dma_aria_check(dma_chn_id_t chn_id);
static mt_s32 dma_aria_soft_reset(dma_chn_id_t chn_id);

static mt_u32 hal_get_u32(ulong reg);
static mt_void hal_put_u32(ulong addr, mt_u32 data);
static mt_u8 reg_bit_get(ulong reg, mt_u8 bshift);
static mt_void reg_bit_set(ulong reg, mt_u8 bshift, mt_u8 bval);

/*!
  Get 32 bits register value

  \param[in] p_addr register address

  \return register value
  */
static mt_u32 hal_get_u32(ulong reg)
{

	/*!
	  Get 32 bits register value
	*/
	return HAL_GET_U32((volatile void*)reg);
}

/*!
  Write 32 bits register

  \param[in] addr register address
  \param[in] p_addr data to write
  */
static mt_void hal_put_u32(ulong addr, mt_u32 data)
{
	/*!
	  Write 32 bits register
	*/
	HAL_PUT_U32((volatile void*)addr,data);
}



static mt_u8 reg_bit_get(ulong reg, mt_u8 bshift)
{
	return ((hal_get_u32(reg) >> bshift) & 0x1);
}

static mt_void reg_bit_set(ulong reg, mt_u8 bshift, mt_u8 bval)
{
	mt_u32 rval = hal_get_u32(reg);

	if (bval){
		hal_put_u32(reg, rval | (1 << bshift));
	}
	else{
		hal_put_u32(reg, rval & (~(1 << bshift)));
	}
}

static mt_u32 conv_fill_data(mt_u32 fill_data, mt_u32 fill_width)
{
	mt_u32 conv = 0;
	mt_u32 dtmp = 0;

	switch (fill_width){
		case DMA_TRANS_UNIT_BYTE:
			dtmp = fill_data & 0xff;
			conv = dtmp << 24 | dtmp << 16 | dtmp << 8 | dtmp;
			break;

		case DMA_TRANS_UNIT_HALF:
			dtmp = fill_data & 0xffff;
			conv = dtmp << 16 | dtmp;
			break;

		case DMA_TRANS_UNIT_WORD:
		case DMA_TRANS_UNIT_DWORD:
			conv = fill_data;
			break;

		default:
			break;
	}

	return conv;
}

static MT_BOOL is_all_chn_closed(mt_void)
{
	dma_chn_id_t chn_id = 0;

	for (chn_id = DMA_CHN_ID0; chn_id < DMA_CHN_ID_MAX; chn_id ++){
		if (dma_aria_check(chn_id)){
			return FALSE;
		}
	}

	return TRUE;
}

static mt_s32 dma_chn_config(dma_chn_id_t chn_id,
  hal_dma_param_t *p_param, dma_chn_lln_t *p_node,
  dma_priv_t *p_priv)
{
  mt_u32 dst = 0;
  mt_u32 src = 0;
  mt_u32 mode = 0;
  mt_u32 alu_mode = 0;
  mt_u32 alu_word = 0;
  mt_u32 config = 0;
  mt_u32 prio = 0;
  mt_u32 wlast = 0;
  mt_u32 len = 0;
  mt_u32 src_width = 0;
  mt_u32 dst_width = 0;
  mt_u32 src_block = 0;
  mt_u32 dst_block = 0;
  mt_u32 src_intv = 0;
  mt_u32 dst_intv = 0;
  ulong ptmp = 0;
  mt_u32 dtmp = 0;

  if (chn_id >= DMA_CHN_ID_MAX){
    DMA_PRINT("\r\n Dma Error: Channel Id = %d\n", chn_id);
    return DMA_ERR_PARAM;
  }

  // Set address of linked list node
  if (NULL == p_node){
    DMA_PRINT("\nDma Error: p_node is NULL\n");
    return DMA_ERR_PARAM;
  }

  DMA_PRINT("src = 0x%llx, dst = 0x%llx\n",p_param->phy_src_addr, p_param->phy_dst_addr);

  dst = p_param->phy_dst_addr;
  src = p_param->phy_src_addr;

  mode = p_param->mode;
  switch (mode){
    case DMA_MODE_ALU_AND:
      alu_mode = ALU_MODE_AND;
      break;

    case DMA_MODE_ALU_OR:
      alu_mode = ALU_MODE_OR;
      break;

    case DMA_MODE_ALU_XOR:
      alu_mode = ALU_MODE_XOR;
      break;

    case DMA_MODE_ALU_NOT:
      alu_mode = ALU_MODE_NOT;
      break;

    case DMA_MODE_ALU_FILL:
      alu_mode = ALU_MODE_FILL;
      src = (mt_u32)p_priv->p_alu_src_phy;
      break;

    default:
      alu_mode = ALU_MODE_NIL;
      break;
  }

  if ((0 == src) || (0 == dst)){
    DMA_PRINT("\r\n InValid Params, src = 0x%x, dst = 0x%x\n", src, dst);
    return DMA_ERR_PARAM;
  }

  prio = p_param->prio & DMA_PRIO_MASK;
  wlast = p_param->prio & DMA_WLAST_MASK;
  //DMA_PRINT("\nChannel RW priority = 0x%x\n", prio);

  ptmp = g_dmac_info.base+R_DMA_PLUS_PRI_SET;
  dtmp = hal_get_u32(ptmp);
  dtmp &= ~(0x3 << (chn_id * 2));
  dtmp |= prio << (chn_id * 2);
  if (wlast){
	dtmp |= wlast;
  }
  else{
	dtmp &= ~(1 << 16);
  }
  hal_put_u32(ptmp, dtmp);

  len = p_param->len & DMA_TRSCNT_MASK;
  src_intv = p_param->src_leap;
  dst_intv = p_param->dst_leap;

  if (!p_param->src_width){
    p_param->src_width = 0x700;
  }
  else{
    p_param->src_width &= DMA_WIDTH_MASK;
  }
  src_width = p_param->src_width;

  if (!p_param->dst_width){
    p_param->dst_width = 0x700;
  }
  else{
    p_param->dst_width &= DMA_WIDTH_MASK;
  }
  dst_width = p_param->dst_width;

  if ((src_width > DMA_WIDTH_MASK) || (dst_width > DMA_WIDTH_MASK) ||
       (src_intv > DMA_JUMP_MASK) || (dst_intv > DMA_JUMP_MASK)){
      DMA_PRINT("\r\n Dma Error: src_width=%d, dst_width=%d, src_intv=%d, dst_intv=%d\n",
      src_width, dst_width, src_intv, dst_intv);

    return DMA_ERR_PARAM;
  }

  /* configure */
  config |= (p_param->config.dst_peripheral & DMA_PERIPHERAL_MASK) << DMA_DST_PERIPHERAL_SHIFT;
  config |= (p_param->config.src_peripheral & DMA_PERIPHERAL_MASK) << DMA_SRC_PERIPHERAL_SHIFT;
  config |= alu_mode << DMA_ALU_SHIFT;
  config |= (p_param->config.dst_endian & DMA_ENDIAN_MASK) << DMA_DST_ENDIAN_SHIFT;
  config |= (p_param->config.src_endian & DMA_ENDIAN_MASK) << DMA_SRC_ENDIAN_SHIFT;
  config |= (p_param->config.dst_clk & DMA_CLK_MASK) << DMA_DST_CLK_SHIFT;
  config |= (p_param->config.src_clk & DMA_CLK_MASK) << DMA_SRC_CLK_SHIFT;
  config |= (p_param->config.dst_i & DMA_INC_MASK) << DMA_DI_SHIFT;
  config |= (p_param->config.src_i & DMA_INC_MASK) << DMA_SI_SHIFT;
  config |= (p_param->config.dst_usize & DMA_USIZE_MASK) << DMA_DST_USIZE_SHIFT;
  config |= (p_param->config.src_usize & DMA_USIZE_MASK) << DMA_SRC_USIZE_SHIFT;
  config |= (p_param->config.dst_bsize & DMA_BSIZE_MASK) << DMA_DST_BSIZE_SHIFT;
  config |= (p_param->config.src_bsize & DMA_BSIZE_MASK) << DMA_SRC_BSIZE_SHIFT;
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->config = config;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_CONFIG, chn_id);
    hal_put_u32(ptmp, config);
  }

  /* source address */
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->src_addr = (mt_u32)src & DMA_ADDR_MASK;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_SRC_ADDR, chn_id);
    hal_put_u32(ptmp, (mt_u32)src & DMA_ADDR_MASK);
  }

  /* destination address */
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->dst_addr = (mt_u32)dst & DMA_ADDR_MASK;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_DST_ADDR, chn_id);
    hal_put_u32(ptmp, (mt_u32)dst & DMA_ADDR_MASK);
  }

  /* source block */
  src_block = (src_width << DMA_WIDTH_SHIFT) | (src_intv << DMA_JUMP_SHIFT);
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->src_block = src_block;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_SRC_BLOCK, chn_id);
    hal_put_u32(ptmp, src_block);
  }

  /* destination block */
  dst_block = (dst_width << DMA_WIDTH_SHIFT) | (dst_intv << DMA_JUMP_SHIFT);
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->dst_block = dst_block;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_DST_BLOCK, chn_id);
    hal_put_u32(ptmp, dst_block);
  }

  /* transfer count */
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->trans_cnt = len;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_TRANSED_CNT, chn_id);
    hal_put_u32(ptmp, len);
  }

  /* alu word */
  if (alu_mode != ALU_MODE_FILL)
  {
    alu_word = p_param->alu_fill_data;
  }
  else
  {
    alu_word = conv_fill_data(p_param->alu_fill_data, p_param->alu_fill_width);
  }
  if (!p_param->control.chn_param_reg_en)
  {
    p_node->alu_word = alu_word;
  }
  else
  {
    ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_ALU_WORD, chn_id);
    hal_put_u32(ptmp, alu_word);
  }

  return DMA_SUCCESS;
}

static mt_s32 dma_chn_pause_en(dma_chn_id_t chn_id, MT_BOOL en)
{
	ulong ptmp = 0;

	ptmp = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_CONTROL, chn_id);
	reg_bit_set(ptmp, BIT_CH_PAUSE_EN, en);
	return DMA_SUCCESS;
}

static mt_s32 dma_chn_open(dma_chn_id_t chn_id,
  phys_addr_t p_node, dma_chn_control_t * p_control)
{
	ulong reg = 0;

	if (chn_id >= DMA_CHN_ID_MAX){
		DMA_PRINT("\nDma Error: Channel Id = %d", chn_id);
		return DMA_ERR_PARAM;
	}

	// Set address of linked list node
	if (0 == p_node){
		DMA_PRINT("\nDma Error: p_node is NULL");
		return DMA_ERR_PARAM;
	}

	/* Assign node's start address */
	reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_LLN, chn_id);

	if (!p_control->chn_param_reg_en){
		hal_put_u32(reg, (u32)p_node);
	}
	else{
		hal_put_u32(reg, (u32)0x0);
	}
	// Configure reg DMA_CHN_CONTROL
	reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_CONTROL, chn_id);
	reg_bit_set(reg, BIT_INT_LINK_EN, p_control->int_link_en? 1:0);
	reg_bit_set(reg, BIT_INT_NODE_EN, p_control->int_node_en? 1:0);
	reg_bit_set(reg, BIT_CH_PARA_REG_EN, p_control->chn_param_reg_en? 1:0);
	reg_bit_set(reg, BIT_CH_PAUSE_EN, 0);
	reg_bit_set(reg, BIT_CH_LOAD_EN, 0);

	dma_aria_soft_reset(chn_id); // DMA channel soft reset

	reg_bit_set(reg, BIT_CH_LOAD_EN, 1); // Load channel params
	reg_bit_set(reg, BIT_CH_ENABLE, 1); // Run channel
	return DMA_SUCCESS;
}

static mt_s32 dma_chn_close(dma_chn_id_t chn_id)
{
	ulong reg = 0;

	return 0;

	if (chn_id >= DMA_CHN_ID_MAX){
		DMA_PRINT("\r\n Dma Error: Channel Id = %d", chn_id);
		return DMA_ERR_PARAM;
	}

	reg_bit_set(g_dmac_info.base+R_DMA_PLUS_SOFT_RST_EN, chn_id, 1);

	while (DMA_STATUS_STOP != dma_aria_check(chn_id));

	reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_CONTROL, chn_id);
	reg_bit_set(reg, BIT_CH_LOAD_EN, 0);
	reg_bit_set(reg, BIT_CH_ENABLE, 0);

	reg_bit_set(R_DMA_PLUS_SOFT_RST_EN, chn_id, 0);

	return DMA_SUCCESS;
}

static irqreturn_t dma_isr(mt_s32 irq, mt_void *p_dma)
{
	dma_priv_t *p_priv = (dma_priv_t *)((hal_dma_op_t *)p_dma)->p_priv;
	mt_u32 dma_int_status = 0;
	dma_chn_id_t chn_id = 0;
	dma_event_t dma_event = DMA_EVENT_COMPLETE;

	DMA_PRINT("\n[ISR]");
	dma_int_status = hal_get_u32((ulong)(g_dmac_info.base+R_DMA_PLUS_INT_STATUS_M));
	if(!dma_int_status){
		DMA_PRINT("\nstatus is zero,maybe it is read by last event");
		return IRQ_HANDLED;
	}

	for (chn_id = DMA_CHN_ID0; chn_id < DMA_CHN_ID_MAX; chn_id ++){
		if ((dma_int_status >> chn_id) & 0x1){
			DMA_PRINT("\nchan_id=%d", chn_id);

			// Issue user ISR of specific channel
			if (NULL != p_priv->dma_notify_param[chn_id].fn_cb){
				if (dma_event & (p_priv->dma_notify_param[chn_id].event_mask)){
					(*p_priv->dma_notify_param[chn_id].fn_cb)(chn_id, dma_event,
					p_priv->dma_notify_param[chn_id].param1,
					p_priv->dma_notify_param[chn_id].param2);
				}
			}
		}
	}

	return IRQ_HANDLED;
}

static mt_s32 dma_aria_capacity_get(
  hal_dma_capacity_t *p_capacity, hal_dma_op_t *p_dma)
{
	dma_priv_t *p_priv = (dma_priv_t *)(p_dma->p_priv);

	memcpy(p_capacity, &p_priv->capacity, sizeof(hal_dma_capacity_t));

	return DMA_SUCCESS;
}

static mt_s32 dma_aria_start(mt_s32 chn_id, hal_dma_param_t *p_param,
  hal_dma_notify_t *p_notify, hal_dma_op_t *p_dma)
{
  mt_s32 ret = 0;
  dma_priv_t *p_priv = (dma_priv_t *)(p_dma->p_priv);
  dma_chn_lln_t *p_node = NULL;
  dma_chn_lln_t *p_node_tmp = NULL;
  dma_chn_lln_t *p_node_prev = NULL;
  dma_chn_lln_t *p_next_node = NULL;
  hal_dma_param_t *p_param_tmp = NULL;
  mt_u32 lln_cnt = 1;
  //ulong tmp_addr = 0;
  //dma_64bit_addr_t *p_node_64bit_tmp = NULL;
  dma_64bit_addr_t p_node_64bit[DMA_LLN_LEN_MAX] = {0};
  mt_u32 addr_id = 0;

  if (p_param == NULL){
    DMA_PRINT("\nInValid Params, Dma Params is NULL!");
    return DMA_ERR_PARAM;
  }

  if (chn_id >= DMA_CHN_ID_MAX){
    DMA_PRINT("\nDma Error: Channel Id = %d", chn_id);
    return DMA_ERR_PARAM;
  }

  // Stop this DMA channel
  ret = dma_chn_close(chn_id);
  if (DMA_SUCCESS != ret){
    return ret;
  }

  p_priv->lln_len[chn_id] = 0;

  if (NULL != p_notify)
  {
    if (!irq_requested_dma)
    {
      if (DMA_SUCCESS !=  request_irq(g_dmac_info.irq, dma_isr, IRQF_TRIGGER_HIGH, "dma", p_dma))
      {
        DMA_PRINT("request IRQ_DMA_ID failed\n");
      }
      else
      {
        irq_requested_dma = TRUE;
      }
    }
    memcpy(&p_priv->dma_notify_param[chn_id], p_notify, sizeof(hal_dma_notify_t));
  }
  else
  {
    if (irq_requested_dma && is_all_chn_closed())
    {
      free_irq(g_dmac_info.irq, p_dma);
      irq_requested_dma = FALSE;
    }
    memset(&p_priv->dma_notify_param[chn_id], 0x0, sizeof(hal_dma_notify_t));
  }

  if (NULL != p_priv->p_lln_head_vir[chn_id])
  {
    memset(p_priv->p_lln_head_vir[chn_id], 0x0, (sizeof(dma_chn_lln_t) * DMA_LLN_LEN_MAX));
  }
  else
  {
    DMA_PRINT("\nDma Error: p_lln_head[%d] is NULL\n", chn_id);
    return DMA_ERR_PARAM;
  }

  // Allocate memory for p_node
  p_node = p_priv->p_lln_head_vir[chn_id];
  p_param_tmp = p_param;
  p_node_tmp = p_node;

  p_node_64bit[addr_id].dma_chn_lln = p_node_tmp;
  p_node_64bit[addr_id].next_64bit_addr = 0;

#if 1
  do
  {
    DMA_PRINT("\nchannel[%d], p_param_tmp[0x%lx]\n", chn_id, (ulong)p_param_tmp);
    DMA_PRINT("vir_src_addr = 0x%lx, phy_src_addr = 0x%llx, vir_dst_addr = 0x%lx,  phy_dst_addr = 0x%llx\n",
           (ulong)p_param_tmp->vir_src_addr,  p_param_tmp->phy_src_addr,
           (ulong)p_param_tmp->vir_dst_addr,  p_param_tmp->phy_dst_addr);


    // configure DMA channel
    ret = dma_chn_config(chn_id, p_param_tmp, p_node_tmp, p_priv);
    if (ret != DMA_SUCCESS)
    {
      return ret;
    }

    // check param for next node if chn_param_reg_en wasn't set
    if ((NULL != p_param_tmp->p_next) && (!p_param_tmp->control.chn_param_reg_en))
    {
      // next param address
      p_param_tmp = p_param_tmp->p_next;

	  p_node_64bit[addr_id].dma_chn_lln = p_node_tmp;
	  p_node_64bit[addr_id].dma_chn_lln->next_node = 1;
	  p_next_node = (dma_chn_lln_t*)((u8 *)p_priv->p_lln_head_vir[chn_id] + (sizeof(dma_chn_lln_t) * lln_cnt++));

      p_node_prev = p_node_tmp;
      p_node_tmp = p_next_node;


      if (!p_node_prev->next_node)
      {
        return DMA_ERR_FAILURE;
      }

    }
    else
    {
      // next node address
      p_node_tmp->next_node = 0x0;

      DMA_PRINT("\nchannel[%d], p_node_tmp[0x%lx]", chn_id,(ulong)p_node_tmp);
      DMA_PRINT("\nsrc_addr[0x%x], dst_addr[0x%x]\nsrc_block[0x%x], dst_block[0x%x]",
        p_node_tmp->src_addr, p_node_tmp->dst_addr,
        p_node_tmp->src_block, p_node_tmp->dst_block);
      DMA_PRINT("\nconfig[0x%x], data_count[0x%x], alu_word[0x%x]\nnext_node[0x%x]\n",
        p_node_tmp->config, p_node_tmp->trans_cnt, p_node_tmp->alu_word,
        p_node_tmp->next_node);
      break;
    }
	addr_id ++;
  }while (lln_cnt <= DMA_LLN_LEN_MAX);

  // Length of linked list nodes
  p_priv->lln_len[chn_id] = (lln_cnt <= DMA_LLN_LEN_MAX) ? lln_cnt : DMA_LLN_LEN_MAX;
  DMA_PRINT("\nLinked List Node: %d\n", p_priv->lln_len[chn_id]);

  addr_id = 0;
  lln_cnt = 1;
  while (p_node_64bit[addr_id].dma_chn_lln && (mt_u32)0x0 != p_node_64bit[addr_id].dma_chn_lln->next_node)
  {
    DMA_PRINT("\nchannel[%d], p_node_tmp[0x%lx]", chn_id, (ulong)p_node_64bit[addr_id].dma_chn_lln);
    DMA_PRINT("\nsrc_addr[0x%x], dst_addr[0x%x]\nsrc_block[0x%x], dst_block[0x%x]",
      p_node_64bit[addr_id].dma_chn_lln->src_addr, p_node_64bit[addr_id].dma_chn_lln->dst_addr,
      p_node_64bit[addr_id].dma_chn_lln->src_block, p_node_64bit[addr_id].dma_chn_lln->dst_block);
    DMA_PRINT("\nconfig[0x%x], data_count[0x%x], alu_word[0x%x]\nvir_next_node[0x%x]\n",
      p_node_64bit[addr_id].dma_chn_lln->config, p_node_64bit[addr_id].dma_chn_lln->trans_cnt, p_node_64bit[addr_id].dma_chn_lln->alu_word,
      p_node_64bit[addr_id].dma_chn_lln->next_node);

    p_node_64bit[addr_id].next_64bit_addr = (ulong)(p_priv->p_lln_head_phy[chn_id] + (sizeof(dma_chn_lln_t) * lln_cnt++));
    p_node_64bit[addr_id].dma_chn_lln->next_node = (mt_u32)p_node_64bit[addr_id].next_64bit_addr;

	DMA_PRINT("\nafter change phy_next_node[0x%x]\n",p_node_64bit[addr_id].dma_chn_lln->next_node);
  	addr_id ++;
  }

#endif

  // Start DMA channel
  ret = dma_chn_open(chn_id, p_priv->p_lln_head_phy[chn_id], &p_param->control);
  if (ret != DMA_SUCCESS){
    return ret;
  }
  //DMA_PRINT("----------------\n");
  return DMA_SUCCESS;
}

mt_s32 dma_aria_check(dma_chn_id_t chn_id)
{

	ulong reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_STATUS, chn_id);

	return reg_bit_get(reg, BIT_CH_BUSY_STAT);
}

static mt_s32 dma_aria_pause(dma_chn_id_t chn_id)
{
	return dma_chn_pause_en(chn_id, TRUE);  // Pause channel running
}

static mt_s32 dma_aria_resume(dma_chn_id_t chn_id)
{
	return dma_chn_pause_en(chn_id, FALSE); // Run channel
}

static mt_s32 dma_aria_stop(dma_chn_id_t chn_id)
{
	return dma_chn_close(chn_id);
}

static mt_s32 dma_aria_soft_reset(dma_chn_id_t chn_id)
{
	if (chn_id >= DMA_CHN_ID_MAX){
		DMA_PRINT("\r\n Dma Error: Channel Id = %d\n", chn_id);
		return DMA_ERR_PARAM;
	}

	reg_bit_set(g_dmac_info.base+R_DMA_PLUS_SOFT_RST_EN, chn_id, 1);

	while (DMA_STATUS_STOP != dma_aria_check(chn_id));

	reg_bit_set(g_dmac_info.base+R_DMA_PLUS_SOFT_RST_EN, chn_id, 0);

	return DMA_SUCCESS;
}

static mt_s32 dma_aria_reset(mt_void)
{
	ulong ptmp = 0;
	mt_u32 dtmp = 0;
	mt_u8 reg_id = 0;
	mt_u8 reset_bit = 16;

	ptmp = R_RST_REQ(reg_id);
	dtmp = hal_get_u32(ptmp);
	dtmp |= 1 << reset_bit;
	hal_put_u32(ptmp, dtmp);

	ptmp = R_RST_ALLOW(reg_id);
	do
	{
		dtmp = hal_get_u32(ptmp);
	}
	while (!((dtmp >> reset_bit) & 0x1));

	ptmp = R_RST_CTRL(reg_id);
	dtmp = hal_get_u32(ptmp);
	dtmp &= ~ (1 << reset_bit);
	hal_put_u32(ptmp, dtmp);

	dtmp |= 1 << reset_bit;
	hal_put_u32(ptmp, dtmp);

	ptmp = R_RST_REQ(reg_id);
	dtmp = hal_get_u32(ptmp);
	dtmp &= ~ (1 << reset_bit);
	hal_put_u32(ptmp, dtmp);

	DMA_PRINT("DMA global reset\n");

	mdelay(2);

	hal_put_u32(g_dmac_info.base+R_DMA_PLUS_INT_MASK_M, 0xffffffff);

	return DMA_SUCCESS;
}

static mt_s32 dma_aria_push(dma_chn_id_t chn_id, mt_u32 length)
{
	ulong reg = 0;

	if (chn_id >= DMA_CHN_ID_MAX){
		DMA_PRINT("Dma Error: Channel Id = %d\n", chn_id);
		return DMA_ERR_PARAM;
	}

	DMA_PRINT("Convey data with unkown size on DMA channel %d\n", chn_id);
	if (DMA_STATUS_STOP == dma_aria_check(chn_id)){
		DMA_PRINT("DMA %d is stopped\n", chn_id);
		return DMA_ERR_STATUS;
	}

	dma_chn_pause_en(chn_id, TRUE);

	length &= DMA_TRSCNT_MASK;
	reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_STATUS, chn_id);
	DMA_PRINT("Already conveyed data size: %d\n", hal_get_u32(reg) >> 1);

	reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_TRANSED_CNT, chn_id);
	hal_put_u32(reg, length);
	DMA_PRINT("Total data size to be completed: %d\n", hal_get_u32(reg));

	dma_chn_pause_en(chn_id, FALSE);

	return DMA_SUCCESS;
}


int dma_aria_init(hal_dma_op_t *p_dma,struct mt_dmac_dev dev)
{
	dma_priv_t *p_priv = NULL;
	dma_chn_id_t chn_id = 0;
	ulong reg = 0;
	mmz_buffer_s      MemBuf;
	mt_s32            Ret;

	if (g_dmac_info.initflag == 1){
		return DMA_SUCCESS;
	}
	
	g_dmac_info.base = dev.base;
	g_dmac_info.dmaclk = dev.dmaclk;
	g_dmac_info.irq = dev.irq;
	
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
{
	mt_u32 val = 0;
	reg = GET_CHN_REG_ADDR(g_dmac_info.base+R_DMA_PLUS_CHN_LLN, chn_id);
	DMA_PRINT("reg = 0x%lx val = 0x%x\n",reg,hal_get_u32(reg));
	hal_put_u32(g_dmac_info.base+R_DMA_PLUS_INT_MASK_M, 0xffffffff);
	val = hal_get_u32(g_dmac_info.base+R_DMA_PLUS_CG_MODE);
	val &= (~0x7);
	hal_put_u32(g_dmac_info.base+R_DMA_PLUS_CG_MODE,val);
}
#endif

  p_dma->capacity_get = (mt_s32 (*)(mt_u32 *, mt_u32 *))dma_aria_capacity_get;
  p_dma->start = (mt_s32 (*)(mt_s32, mt_u32 *, mt_u32 *, mt_u32 *))dma_aria_start;
  p_dma->check = (mt_s32 (*)(mt_s32))dma_aria_check;
  p_dma->pause = (mt_s32 (*)(mt_s32))dma_aria_pause;
  p_dma->resume = (mt_s32 (*)(mt_s32))dma_aria_resume;
  p_dma->stop = (mt_s32 (*)(mt_s32))dma_aria_stop;
  p_dma->reset = (mt_s32 (*)(mt_void))dma_aria_reset;
  p_dma->soft_reset = (mt_s32 (*)(mt_s32))dma_aria_soft_reset;
  p_dma->push = (mt_s32 (*)(mt_s32, mt_u32))dma_aria_push;

  p_priv = (dma_priv_t *)kmalloc(sizeof(dma_priv_t), GFP_KERNEL);
  if (NULL == p_priv){
    return DMA_ERR_FAILURE;
  }
  memset(p_priv, 0, sizeof(dma_priv_t));

  /* set capacity */
  memset(&p_priv->capacity, 0x00, sizeof(hal_dma_capacity_t));

  p_dma->p_priv = p_priv;

  p_priv->capacity.chan_num = DMA_CHN_ID_MAX;

  for (chn_id = DMA_CHN_ID0; chn_id < DMA_CHN_ID_MAX; chn_id ++){
    p_priv->capacity.chan_cap[chn_id].type =
      DMA_TYPE_MEM2MEM | DMA_TYPE_MEM2PORT | DMA_TYPE_PORT2MEM | DMA_TYPE_PORT2PORT;
    p_priv->capacity.chan_cap[chn_id].trans_unit =
      DMA_TRANS_UNIT_BYTE | DMA_TRANS_UNIT_HALF | DMA_TRANS_UNIT_WORD | DMA_TRANS_UNIT_DWORD;
    p_priv->capacity.chan_cap[chn_id].support_2d = 1;
    p_priv->capacity.chan_cap[chn_id].addr_align_byte = 1;
    p_priv->capacity.chan_cap[chn_id].addr_align_half = 1;
    p_priv->capacity.chan_cap[chn_id].addr_align_word = 1;
    p_priv->capacity.chan_cap[chn_id].addr_align_dword = 1;

    p_priv->capacity.chan_cap[chn_id].max_size_one_req = DMA_TRSCNT_MASK;
    p_priv->capacity.chan_cap[chn_id].max_size_one_width = DMA_WIDTH_MASK;
    p_priv->capacity.chan_cap[chn_id].max_size_one_leap = DMA_JUMP_MASK;

	p_priv->p_lln_head_vir[chn_id] = NULL;
	
  	Ret = mt_drv_mmz_alloc_and_map("DMA_LLN_BUF", MMZ_OTHERS, sizeof(dma_chn_lln_t) * DMA_LLN_LEN_MAX, 0, &MemBuf);
  	if (Ret != MT_SUCCESS){
  		DMA_PRINT("malloc DMA_LLN_BUF mmz failed.\n");
  		goto mmz_buff_err;
  	}

  	p_priv->p_lln_head_phy[chn_id] = MemBuf.startPhyAddr;
  	p_priv->p_lln_head_vir[chn_id] = (dma_chn_lln_t *)MemBuf.startVirAddr;

  	DMA_PRINT("lln phy  addr = 0x%llx, vir addr = %p\n",p_priv->p_lln_head_phy[chn_id], p_priv->p_lln_head_vir[chn_id]);

    memset (p_priv->p_lln_head_vir[chn_id], 0x0, sizeof(dma_chn_lln_t) * DMA_LLN_LEN_MAX);

  }

	Ret = mt_drv_mmz_alloc_and_map("DMA_ALU_BUF", MMZ_OTHERS, 8, 0, &MemBuf);
	if (Ret != MT_SUCCESS){
		DMA_PRINT("malloc DMA_ALU_BUF mmz failed.\n");
		goto mmz_buff_err;
	}

	p_priv->p_alu_src_phy = MemBuf.startPhyAddr;
	p_priv->p_alu_src_vir = MemBuf.startVirAddr;

	DMA_PRINT("alu phy addr = 0x%llx, vir addr = %p\n",p_priv->p_alu_src_phy, p_priv->p_alu_src_vir);

	// DMA channel clock enable
	//reg = R_DMA_CLKEN_CONFIG;
	//reg_bit_set(reg, chn_id, 1);
	g_dmac_info.initflag = 1;
	return DMA_SUCCESS;

mmz_buff_err:
	for (chn_id = DMA_CHN_ID0; chn_id < DMA_CHN_ID_MAX; chn_id ++){
		if (p_priv->p_lln_head_vir[chn_id] == NULL){
			break;
		}
		MemBuf.startPhyAddr = p_priv->p_lln_head_phy[chn_id];
		MemBuf.startVirAddr = p_priv->p_lln_head_vir[chn_id];
		mt_drv_mmz_unmap_and_release(&MemBuf);
	}
	kfree(p_priv);
	return DMA_ERR_FAILURE;
}
EXPORT_SYMBOL(dma_aria_init);

int dma_aria_deinit(hal_dma_op_t *p_dma)
{
	dma_priv_t *p_priv;
	dma_chn_id_t chn_id = 0;
	mmz_buffer_s stMMZBuf;

	if (p_dma == NULL){
		return DMA_ERR_FAILURE;
	}

	p_priv = (dma_priv_t *)(p_dma->p_priv);
	if (p_priv == NULL){
		return DMA_ERR_FAILURE;
	}

	DMA_PRINT("p_dma = %p, p_priv = %p\n",p_dma,p_priv);
	for (chn_id = DMA_CHN_ID0; chn_id < DMA_CHN_ID_MAX; chn_id ++){
		dma_chn_close(chn_id);
	}

	if (NULL != p_priv->p_alu_src_vir){
		DMA_PRINT("p_alu_src = %p\n",p_priv->p_alu_src_vir);

		stMMZBuf.startPhyAddr = p_priv->p_alu_src_phy;
		stMMZBuf.startVirAddr = p_priv->p_alu_src_vir;
		stMMZBuf.size = 8;
		mt_drv_mmz_unmap(&stMMZBuf);
		p_priv->p_alu_src_vir = NULL;
		p_priv->p_alu_src_phy = 0;
	}

	if (irq_requested_dma){
		free_irq(g_dmac_info.irq, p_dma);
		irq_requested_dma = FALSE;
	}

	for (chn_id = DMA_CHN_ID0; chn_id < DMA_CHN_ID_MAX; chn_id ++){
		if (NULL != p_priv->p_lln_head_vir[chn_id]){
		  DMA_PRINT("p_lln_head_virtual = %p\n",p_priv->p_lln_head_vir[chn_id]);
		  stMMZBuf.startPhyAddr = p_priv->p_lln_head_phy[chn_id];
		  stMMZBuf.startVirAddr = p_priv->p_lln_head_vir[chn_id];
		  stMMZBuf.size = sizeof(dma_chn_lln_t) * DMA_LLN_LEN_MAX;
		  mt_drv_mmz_unmap(&stMMZBuf);

		  p_priv->p_lln_head_vir[chn_id] = NULL;
		  p_priv->p_lln_head_phy[chn_id] = 0;
		}
	}

	if (NULL != p_priv){
		kfree(p_priv);
		p_priv = NULL;
	}

	p_dma->capacity_get = NULL;
	p_dma->start = NULL;
	p_dma->check = NULL;
	p_dma->pause = NULL;
	p_dma->resume = NULL;
	p_dma->stop = NULL;
	p_dma->reset = NULL;
	p_dma->soft_reset = NULL;
	p_dma->push = NULL;
	g_dmac_info.initflag = 0;
	return DMA_SUCCESS;
}
EXPORT_SYMBOL(dma_aria_deinit);



