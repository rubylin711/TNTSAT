/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_apl.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Montage Clock driver Application Layer(APL).
 * History        :
 * 1.Date         : 2021/02/08
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/string.h>
#include <linux/printk.h>

#include "mt_clk.h"

#define LEVEL_ERR			"[E]"

#define CLK_ERROR(...)		printk(KERN_ERR __VA_ARGS__)

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) sizeof(a)/sizeof((a)[0])
#endif

#define CHECK_NULL_RETURN(arg, ret)	\
			do {					\
				if ((void*)(arg) == NULL) {	\
					CLK_ERROR(LEVEL_ERR "%s@%d: param is null or zero!\n", __FUNCTION__, __LINE__);	\
					return ret;	\
				}	\
			} while (0)

//---------------------------------------------------------------------------//

/* clk and xtal select clk map */
struct xtal_sel_clk_map
{
	const char *clk_name;			//clk name
	const char *xtal_sel_clk_name;	//xtal select clk name
	mt_u32 rate_hide_xtal;			//<CN>Òþº¬Ñ¡Ôñ¾§Õñ

};

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
/* clk and xtal select clk map table */
static struct xtal_sel_clk_map g_xtal_sel_clk_table[] =
{
	{MT_AVCPU_CLK, 		"avcpu_xtal_sel", 		90000},
	{MT_APB_CLK, 		"apb_xtal_sel", 		45000},

	{"m2m_clk", 		"cipher_xtal_sel", 		131000},

	{"secure_clk", 		"secure_xtal_sel", 		131000},
	{MT_CI_CLK, 		"ci_xtal_sel", 			120000},
	{MT_TSI_CLK, 		"tsi_xtal_sel", 		131000},
	{"ifcp_crypto_clk", "ifcpcrypto_xtal_sel", 	120000},
	{MT_PNG_CLK, 		"png_xtal_sel", 		144000},
	{MT_AOUT_CLK, 		"aout_xtal_sel", 		131000},
	{MT_VDEC_CLK, 		"vdec_xtal_sel", 		45000},

	{"disposdc_clk", 	"disp_xtal_sel", 		60000},
	{MT_DI_CLK,			"disp_xtal_sel",		60000},
	{MT_DISP_CLK,		"disp_xtal_sel",		60000},

	{MT_JPG_CLK, 		"jpg_xtal_sel", 		144000},
	{MT_GRA_CLK, 		"gra_xtal_sel", 		320000},
	{MT_DMA_CLK, 		"dma_xtal_sel", 		131000},
	{"apbackup_clk1", 	"apbackup_xtal_sel", 	60000}
};

#else

//sym1/2: for compile
static struct xtal_sel_clk_map g_xtal_sel_clk_table[] =
{ {"null", "null", 0} };

#endif

static const char *get_xtal_sel_clk_name(const char *clk_name, mt_u32 *hide_rate)
{
	int i;
	int n = ARRAY_SIZE(g_xtal_sel_clk_table);

	for (i=0; i<n; i++)
	{
		if (strcmp(clk_name, g_xtal_sel_clk_table[i].clk_name) == 0)
		{
			if (hide_rate)
				*hide_rate = g_xtal_sel_clk_table[i].rate_hide_xtal;

			return g_xtal_sel_clk_table[i].xtal_sel_clk_name;
		}
	}

	return NULL;
}

static struct mt_clk *get_xtal_sel_clk(const char *clk_name, mt_u32 *hide_rate)
{
	const char *xtal_sel_clk_name = get_xtal_sel_clk_name(clk_name, hide_rate);

	if (xtal_sel_clk_name == NULL)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s - xtal sel clk not found!\n", __FUNCTION__, clk_name);
		return NULL;
	}

	return mt_clk_get(MT_CLK_DEV, xtal_sel_clk_name);
}

//---------------------------------------------------------------------------//

mt_s32 mt_clk_apl_select_xtal(struct mt_clk *clk)
{
	mt_s32 ret;
	struct mt_clk *xtal_sel_clk;
	mt_u32 rate_hide_xtal;

	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);

	xtal_sel_clk = get_xtal_sel_clk(clk->name, &rate_hide_xtal);
	CHECK_NULL_RETURN(xtal_sel_clk, MT_ERR_PARAM);

	//xtal_sel: 1
	ret = mt_clk_select_xtal(xtal_sel_clk);
	if (ret != MT_SUCCESS)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s - xtal sel failed(%d)!\n",
			__FUNCTION__, clk->name, ret);
		goto Err;
	}

	//rate_hide_xtal: <CN>Òþº¬Ñ¡Ôñ¾§Õñ
	// => xtal clk
	ret = mt_clk_set_rate(clk, rate_hide_xtal);
	if (ret != MT_SUCCESS)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s - set hide xtal rate failed(%d)!\n",
			__FUNCTION__, clk->name, ret);
		goto Err;
	}

Err:
	return ret;
}

mt_s32 mt_clk_apl_unselect_xtal(struct mt_clk *clk, mt_u32 rate)
{
	mt_s32 ret;
	struct mt_clk *xtal_sel_clk;

	CHECK_NULL_RETURN(clk, MT_ERR_PARAM);
	CHECK_NULL_RETURN(clk->name, MT_ERR_PARAM);

	xtal_sel_clk = get_xtal_sel_clk(clk->name, NULL);
	CHECK_NULL_RETURN(xtal_sel_clk, MT_ERR_PARAM);

	//xtal_sel: 0
	ret = mt_clk_unselect_xtal(xtal_sel_clk);
	if (ret != MT_SUCCESS)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s - xtal unsel failed(%d)!\n",
			__FUNCTION__, clk->name, ret);
		goto Err;
	}

	//recover rate
	ret = mt_clk_set_rate(clk, rate);
	if (ret != MT_SUCCESS)
	{
		CLK_ERROR(LEVEL_ERR "%s: clk %s - set rate failed(%d)!\n",
			__FUNCTION__, clk->name, ret);
		goto Err;
	}

Err:
	return ret;
}

