/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
/******************************************************************************
 * Copyright (C) 2021, Montage Technology Co., Ltd.
 *
 * File Name      : mt_clk_testsuite.c
 * Version        : V0.9
 * Author         : MA-SW
 * Created        : 2021/02/08
 * Description    : Montage Clock driver testsuite.
 * History        :
 * 1.Date         : 2021/02/08
 *   Author       : LZ100218
 *   Modification : Created file
 *
 *****************************************************************************/
#include <linux/printk.h>

#include "mt_type.h"

#include "mt_clk.h"

#define TEST_PRINTF			printk

#define TEST_ASSERT(expr)	if (!(expr)) {	\
								ret = -1;	\
								TEST_PRINTF("%s@%d: assert!\n", __FUNCTION__, __LINE__);	\
								goto Err;	\
							}

#define TEST_RESULT(ret)	do {	\
								if ((ret) == 0) {	\
									TEST_PRINTF("CLK TEST - %s: Pass.\n",__FUNCTION__);	\
								} else {	\
									TEST_PRINTF("CLK TEST - %s: Fail(%d)!\n",__FUNCTION__,ret);	\
								}	\
							} while(0)

//---------------------------------------------------------------------------//

static int testcase01_gate(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 status = 0;

	//wrong clk name
	clk = mt_clk_get(MT_CLK_DEV, "xxx_clk");
	TEST_ASSERT(clk == NULL);

	clk = mt_clk_get(MT_CLK_DEV, MT_MAC_CLK);
	TEST_ASSERT(clk != NULL);

	status = mt_clk_get_enable(clk);
	TEST_ASSERT(status == 1);

	mt_clk_disable(clk);

	status = mt_clk_get_enable(clk);
	TEST_ASSERT(status == 0);

	ret = mt_clk_enable(clk);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_enable(clk);
	TEST_ASSERT(status == 1);

Err:
	TEST_RESULT(ret);

	return ret;
}

static int testcase02_rate(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 status = 0;
	mt_u32 org_rate = 0;

	clk = mt_clk_get(MT_CLK_DEV, MT_GRA_CLK);
	TEST_ASSERT(clk != NULL);

	org_rate = mt_clk_get_rate(clk);
	TEST_ASSERT(org_rate != 0);

	//wrong rate
	ret = mt_clk_set_rate(clk, 888000);
	TEST_ASSERT(ret != 0);

	//262
	ret = mt_clk_set_rate(clk, 262000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 262000);

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	//147.5
	ret = mt_clk_set_rate(clk, 147500);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 147500);

	//205.5
	ret = mt_clk_set_rate(clk, 205500);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 205500);

	//131
	ret = mt_clk_set_rate(clk, 131000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 131000);

#elif defined(CONFIG_MT_CHIP_SYMPHONY2)
	//144
	ret = mt_clk_set_rate(clk, 144000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 144000);

	//205.5
	ret = mt_clk_set_rate(clk, 205500);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 205500);

	//131
	ret = mt_clk_set_rate(clk, 131000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 131000);

#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	//360
	ret = mt_clk_set_rate(clk, 360000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 360000);

	//444
	ret = mt_clk_set_rate(clk, 444000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 444000);

	//320
	ret = mt_clk_set_rate(clk, 320000);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == 320000);

#endif

	//recover
	ret = mt_clk_set_rate(clk, org_rate);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_rate(clk);
	TEST_ASSERT(status == org_rate);

Err:
	TEST_RESULT(ret);

	return ret;
}

static int testcase03_div(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 status = 0;
	mt_u32 org_div = 0;

	clk = mt_clk_get(MT_CLK_DEV, MT_HVENC_CLK);
	TEST_ASSERT(clk != NULL);

	org_div = mt_clk_get_div(clk);
	TEST_ASSERT(org_div != 0);

	//wrong div
	ret = mt_clk_set_div(clk, MT_DIV_3);
	TEST_ASSERT(ret != 0);

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	//1
	ret = mt_clk_set_div(clk, MT_DIV_1);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_div(clk);
	TEST_ASSERT(status == MT_DIV_1);
#endif

	//2
	ret = mt_clk_set_div(clk, MT_DIV_2);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_div(clk);
	TEST_ASSERT(status == MT_DIV_2);

	//4
	ret = mt_clk_set_div(clk, MT_DIV_4);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_div(clk);
	TEST_ASSERT(status == MT_DIV_4);

	//6
	ret = mt_clk_set_div(clk, MT_DIV_6);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_div(clk);
	TEST_ASSERT(status == MT_DIV_6);

	//8
	ret = mt_clk_set_div(clk, MT_DIV_8);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_div(clk);
	TEST_ASSERT(status == MT_DIV_8);

	//org
	ret = mt_clk_set_div(clk, org_div);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_div(clk);
	TEST_ASSERT(status == org_div);

Err:
	TEST_RESULT(ret);

	return ret;
}

static int testcase04_mux(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 status = 0;
	mt_u32 org_mux = 0;

	clk = mt_clk_get(MT_CLK_DEV, MT_MAC_CLK);
	TEST_ASSERT(clk != NULL);

	org_mux = mt_clk_get_mux(clk);

	//0
	ret = mt_clk_set_mux(clk, 0);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_mux(clk);
	TEST_ASSERT(status == 0);

	//1
	ret = mt_clk_set_mux(clk, 1);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_mux(clk);
	TEST_ASSERT(status == 1);

	//org
	ret = mt_clk_set_mux(clk, org_mux);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_mux(clk);
	TEST_ASSERT(status == org_mux);

Err:
	TEST_RESULT(ret);

	return ret;
}

static int testcase05_edge(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 status = 0;
	mt_u32 org_edge = 0;

#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY1)
	clk = mt_clk_get(MT_CLK_DEV, MT_TS0_CLK);
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
	clk = mt_clk_get(MT_CLK_DEV, "ddrpll_pnclk_clksel");
#endif
	TEST_ASSERT(clk != NULL);

	org_edge = mt_clk_get_edge(clk);

	//0
	ret = mt_clk_set_edge(clk, 0);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_edge(clk);
	TEST_ASSERT(status == 0);

	//1
	ret = mt_clk_set_edge(clk, 1);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_edge(clk);
	TEST_ASSERT(status == 1);

	//org
	ret = mt_clk_set_edge(clk, org_edge);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_edge(clk);
	TEST_ASSERT(status == org_edge);

Err:
	TEST_RESULT(ret);

	return ret;
}

static int testcase06_attr(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 status = 0;
	mt_u32 org_attr = 0;

	clk = mt_clk_get(MT_CLK_DEV, MT_MAC_CLK);
	TEST_ASSERT(clk != NULL);

	org_attr = mt_clk_get_attr(clk);

	//0
	ret = mt_clk_set_attr(clk, 0);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_attr(clk);
	TEST_ASSERT(status == 0);

	//1
	ret = mt_clk_set_attr(clk, 1);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_attr(clk);
	TEST_ASSERT(status == 1);

	//org
	ret = mt_clk_set_attr(clk, org_attr);
	TEST_ASSERT(ret == 0);

	status = mt_clk_get_attr(clk);
	TEST_ASSERT(status == org_attr);

Err:
	TEST_RESULT(ret);

	return ret;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
static int testcase07_xtal_sel(void)
{
	int ret = 0;
	struct mt_clk *clk;
	mt_u32 org_rate = 0;

	clk = mt_clk_get(MT_CLK_DEV, MT_PNG_CLK);
	TEST_ASSERT(clk != NULL);

	org_rate = mt_clk_get_rate(clk);
	TEST_ASSERT(org_rate != 0);

	//1.select xtal clk
	ret = mt_clk_apl_select_xtal(clk);
	TEST_ASSERT(ret == 0);

	//TODO: check if select to xtal clock

	//2.unselect xtal clk
	ret = mt_clk_apl_unselect_xtal(clk, org_rate);
	TEST_ASSERT(ret == 0);

Err:
	TEST_RESULT(ret);

	return ret;
}
#endif

int mt_clk_testsuite(void)
{
	int ret = 0;

	TEST_PRINTF("================ MT CLK TestSuite ================\n");

	ret = testcase01_gate();

	ret |= testcase02_rate();

	ret |= testcase03_div();

	ret |= testcase04_mux();

	ret |= testcase05_edge();

	ret |= testcase06_attr();

#if defined(CONFIG_MT_CHIP_SYMPHONY4)
	ret |= testcase07_xtal_sel();
#endif

	TEST_PRINTF("================ Result: %d ================\n", ret);

	return ret;
}

