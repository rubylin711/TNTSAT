/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2025 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef __UBOOT__
#include <common.h>
#include <command.h>
#include <asm/arch-symphony6/mt_common.h>
#include <asm/arch-symphony6/mt_drv_analog.h>
#include <asm/arch-symphony6/mt_drv_clock.h>
#elif defined(__KERNEL__)
#include <linux/types.h>
#include <linux/string.h>

#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_log.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#else
/* RTOS */
#include <string.h>

#include <sys_types.h>
#include <sys_define.h>

#include "mtos_misc.h"

#include "mt_io.h"
#include "mt_reg_base.h"
#include "mt_log.h"
#include "mt_drv_clock.h"
#include "mt_drv_analog.h"
#endif

#include "mt_mod_reset.h"

#define CFG_RESET_SUPPORT

#define MAX_COM_COUNT			5

/* module cmds */
enum
{
	CMD_GET_STATUS = 0,
	CMD_ENABLE,
	CMD_DISABLE,
	CMD_GET_RATE,
	CMD_SET_RATE,
	CMD_RESET,
	CMD_GET_RESET_STATUS,
};

#define CHECK_ID(id)	do {	\
							if (id < 0 || id >= HAL_MODULES_NUM) {	\
								MT_LOGE("%s: Invalid id(%d)!\n", __func__, id);	\
								return -22;	\
							}	\
						} while(0)

/* composite module */
typedef struct mt_composite_module {

	int id;				/* HAL ID */

	const char *name;	/* HAL Module Name */

	/* Components */
	void *ana_com[MAX_COM_COUNT];		/* analog components */
	void *dig_com[MAX_COM_COUNT];		/* digital components */

	void *rate;							/* clock rate component */

#ifdef CFG_RESET_SUPPORT
	void *resets[MAX_COM_COUNT];		/* reset components */
#endif

} mt_comp_mod_t;

#if defined(CONFIG_ARCH_SYMPHONY6) || defined(CONFIG_MT_CHIP_SYMPHONY6) || (ARCH==SYMPHONY6)
#include "mt_hal_mod_composite_sym6.c"
#else
#error "Composite Module only supports Sym6 by now!"
#endif

/* find module by id */
static mt_comp_mod_t *find_mod_by_id(int id)
{
	int i;

	for (i=0; i<mt_comp_modules_count; i++)
	{
		if (mt_comp_modules[i].id == id)
		{
			MT_LOGV("%s: found module[%d] - %s\n", __func__, id, mt_comp_modules[i].name);
			return &mt_comp_modules[i];
		}
	}

	return NULL;
}

/* find module by name */
static mt_comp_mod_t *find_mod_by_name(const char *name)
{
	int i;

	for (i=0; i<mt_comp_modules_count; i++)
	{
		if (!strcasecmp(mt_comp_modules[i].name, name))
		{
			MT_LOGV("%s: found module[%d] - %s\n", __func__, mt_comp_modules[i].id, name);
			return &mt_comp_modules[i];
		}
	}

	return NULL;
}

static int _do_mod_batch_get_status(mt_comp_mod_t *mod, int *status)
{
	int i;
	int ret;

	*status = 0;

	/* analog */
	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->ana_com[i]) {

			ret = mt_analog_get_status((mt_ana_t*)mod->ana_com[i], (u32*)status);

			if (!ret && !*status)
				return 0;

		} else {
			break;
		}
	}

	/* digital */
	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->dig_com[i]) {

			ret = mt_clk_get_status((mt_clk_t*)mod->dig_com[i], (u32*)status);

			if (!ret && !*status)
				return 0;

		} else {
			break;
		}
	}

	return 0;
}

static int _do_mod_batch_enable(mt_comp_mod_t *mod)
{
	int i;

	/* enable digital first */
	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->dig_com[i])
			mt_clk_enable((mt_clk_t*)mod->dig_com[i]);
		else
			break;
	}

	/* then enable analog */
	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->ana_com[i])
			mt_analog_enable((mt_ana_t*)mod->ana_com[i]);
		else
			break;
	}

	return 0;
}

static int _do_mod_batch_disable(mt_comp_mod_t *mod)
{
	int i;

	/* disable analog first */
	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->ana_com[i])
			mt_analog_disable((mt_ana_t*)mod->ana_com[i]);
		else
			break;
	}

	/* then disable digital */
	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->dig_com[i])
			mt_clk_disable((mt_clk_t*)mod->dig_com[i]);
		else
			break;
	}

	return 0;
}

#ifdef CFG_RESET_SUPPORT
static int _do_mod_batch_get_reset_status(mt_comp_mod_t *mod, int *status)
{
	int i;
	int ret;

	*status = 0;

	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->resets[i]) {

			ret = mt_mod_get_reset_status((mt_mod_t*)mod->resets[i], status);

			if (!ret && *status)
				return 0;

		} else {
			break;
		}
	}

	return 0;
}

static int _do_mod_batch_reset(mt_comp_mod_t *mod, unsigned int flag)
{
	int i;

	for (i=0; i<MAX_COM_COUNT; i++)
	{
		if (mod->resets[i]) {

			if ((flag & FLAG_HAL_MOD_RESET_EXP) != 0)
				mt_mod_reset_exp((mt_mod_t*)mod->resets[i]);
			else
				mt_mod_reset((mt_mod_t*)mod->resets[i]);

		} else {
			break;
		}
	}

	return 0;
}
#endif

/*
 * Module's components loop process
 *   cmd: CMD_GET_STATUS, CMD_ENABLE, CMD_DISABLE
 */
static int _mod_cmd_proc(mt_comp_mod_t *mod, int cmd, void *param)
{
	int ret = -1;

	switch (cmd)
	{
		case CMD_GET_STATUS:
		{
			ret = _do_mod_batch_get_status(mod, param);
			break;
		}
		case CMD_ENABLE:
		{
			ret = _do_mod_batch_enable(mod);
			break;
		}
		case CMD_DISABLE:
		{
			ret = _do_mod_batch_disable(mod);
			break;
		}
		case CMD_GET_RATE:
		{
			if (mod->rate) {
				ret = mt_clk_get_rate((mt_clk_t*)mod->rate, param);
			} else {
				*(unsigned long*)param = 0;
				MT_LOGW("%s: module[%d] %s has no rate!\n", __func__, mod->id, mod->name);
			}
			break;
		}
		case CMD_SET_RATE:
		{
			if (mod->rate) {
				ret = mt_clk_set_rate((mt_clk_t*)mod->rate, *(unsigned long*)param);
			} else {
				MT_LOGW("%s: module[%d] %s has no rate!\n", __func__, mod->id, mod->name);
			}
			break;
		}
#ifdef CFG_RESET_SUPPORT
		case CMD_RESET:
		{
			unsigned int flag = *(unsigned int*)param;
			ret = _do_mod_batch_reset(mod, flag);
			break;
		}
		case CMD_GET_RESET_STATUS:
		{
			ret = _do_mod_batch_get_reset_status(mod, (int*)param);
			break;
		}
#endif
		default:
			MT_LOGE("%s: Unknown CMD %d!\n", __func__, cmd);
			break;
	}

	return ret;
}

//---------------------------------------------------------------------------//

int mt_hal_mod_get_id(const char *name)
{
	mt_comp_mod_t *mod;

	if (name == NULL)
	{
		MT_LOGE("%s: name is null!\n", __func__);
		return -22;
	}

	mod = find_mod_by_name(name);

	if (!mod)
	{
		MT_LOGV("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	return mod->id;
}

int mt_hal_mod_get_status(int id, int *status)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	if (status == NULL)
	{
		MT_LOGE("%s: status is null!\n", __func__);
		return -22;
	}

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_GET_STATUS, status);

	MT_LOGV("%s: module[%d] status = %d\n", __func__, id, *status);
	return ret;
}

int mt_hal_mod_enable(int id)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_ENABLE, NULL);

	MT_LOGV("%s: enable module[%d] success.\n", __func__, id);
	return ret;
}

int mt_hal_mod_disable(int id)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_DISABLE, NULL);

	MT_LOGV("%s: disable module[%d] success.\n", __func__, id);
	return ret;
}

int mt_hal_mod_get_rate(int id, unsigned long *rate)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	if (rate == NULL)
	{
		MT_LOGE("%s: rate is null!\n", __func__);
		return -22;
	}

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_GET_RATE, rate);

	if (!ret) {
		MT_LOGV("%s: get module[%d] rate = %lu\n", __func__, id, *rate);
	}

	return ret;
}

int mt_hal_mod_set_rate(int id, unsigned long rate)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_SET_RATE, &rate);

	if (!ret) {
		MT_LOGV("%s: set module[%d] rate = %lu\n", __func__, id, rate);
	}

	return ret;
}

#ifdef CFG_RESET_SUPPORT
int mt_hal_mod_reset(int id, unsigned int flag)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_RESET, &flag);

	MT_LOGV("%s: reset module[%d] success.\n", __func__, id);
	return ret;
}

int mt_hal_mod_get_reset_status(int id, int *status)
{
	int ret;
	mt_comp_mod_t *mod = NULL;

	CHECK_ID(id);

	if (status == NULL)
	{
		MT_LOGE("%s: status is null!\n", __func__);
		return -22;
	}

	mod = find_mod_by_id(id);

	if (!mod)
	{
		MT_LOGE("%s: module[%d] not found!\n", __func__, id);
		return -2;
	}

	ret = _mod_cmd_proc(mod, CMD_GET_RESET_STATUS, status);

	MT_LOGV("%s: get module[%d] reset status = %d\n", __func__, id, *status);
	return ret;
}
#endif

#if defined(__UBOOT__)
void mt_hal_mod_dump_state(void)
#elif defined(__KERNEL__)
void mt_hal_mod_dump_state(void *s)
#else
/* RTOS */
void mt_hal_mod_dump_state(int (*DP_LOG)(const char *fmt, ...))
#endif
{
	int i;

	int id;
	const char *name;

	int status;
	unsigned long rate;
#ifdef CFG_RESET_SUPPORT
	int reset_status;
#endif

	DP_LOG("============== DUMP HAL MODULE STATE ==============\n");

	DP_LOG("%3s %15s %3s %7s %7s\n",
		"ID",
		"NAME",
		"STA",
		"RATE",
		"RESET");

	for (i=0; i<mt_comp_modules_count; i++)
	{
		id = mt_comp_modules[i].id;
		name = mt_comp_modules[i].name;

		status = 0;
		mt_hal_mod_get_status(id, &status);

		rate = 0;
		mt_hal_mod_get_rate(id, &rate);

#ifdef CFG_RESET_SUPPORT
		reset_status = 0;
		mt_hal_mod_get_reset_status(id, &reset_status);
#endif

		DP_LOG("%3d %15s %3s %7lu %7s\n", id, name,
			status?"ON":"OFF",
			rate/1000,
#ifdef CFG_RESET_SUPPORT
			reset_status?"RESET":"RELEASE"
#else
			" "
#endif
			);
	}
}

#ifdef __UBOOT__

static int isdigit(int c)
{
	return ((c >= '0') && (c <= '9'));
}

static int do_cmd_hal_mod(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int ret = 0;
	int id;
	unsigned char *ch;
	char *name = "unknown";

	if (argc < 2)
		return CMD_RET_USAGE;

	if (!strcmp(argv[1], "dump"))
	{
		mt_hal_mod_dump_state();
	}
	else
	{
		if (argc < 3)
			return CMD_RET_USAGE;

		ch = (unsigned char*)argv[2];

		if (isdigit((int)ch[0])) {
			id = (int)simple_strtol(argv[2], NULL, 0);
		} else {
			id = mt_hal_mod_get_id(argv[2]);
			name = argv[2];
		}

		if (!strcmp(argv[1], "enable"))
		{
			ret = mt_hal_mod_enable(id);
			if (ret == 0)
				printf("enable module[%d] %s - success.\n", id, name);
			else
				printf("enable module[%d] %s - failed!\n", id, name);
		}
		else if (!strcmp(argv[1], "disable"))
		{
			ret = mt_hal_mod_disable(id);
			if (ret == 0)
				printf("disable module[%d] %s - success.\n", id, name);
			else
				printf("disable module[%d] %s - failed!\n", id, name);
		}
		else if (!strcmp(argv[1], "set_rate"))
		{
			unsigned long rate = 0;

			if (argc >= 4)
				rate = simple_strtoul(argv[3], NULL, 0);
			else
				return CMD_RET_USAGE;

			ret = mt_hal_mod_set_rate(id, rate);
			if (ret == 0)
				printf("set module[%d] %s rate %lu - success.\n", id, name, rate/1000);
			else
				printf("set module[%d] %s rate %lu - failed!\n", id, name, rate/1000);
		}
#ifdef CFG_RESET_SUPPORT
		else if (!strcmp(argv[1], "reset"))
		{
			unsigned int flag = FLAG_HAL_MOD_NONE;

			if (argc >= 4)
				flag = (unsigned int)simple_strtoul(argv[3], NULL, 0);

			ret = mt_hal_mod_reset(id, flag);
			if (ret == 0)
				printf("reset module[%d] %s - success.\n", id, name);
			else
				printf("reset module[%d] %s - failed!\n", id, name);
		}
#endif
		else
		{
			return CMD_RET_USAGE;
		}
	}

	if (ret == 0)
		return CMD_RET_SUCCESS;
	else
		return CMD_RET_FAILURE;
}

U_BOOT_CMD(
	mt_mod, 4, 0,	do_cmd_hal_mod,
	"HAL Module Commands",
	"\nmt_mod enable id|name - enable module\n"
	"mt_mod disable id|name - disable module\n"
	"mt_mod set_rate id|name rate - set module clock rate\n"
#ifdef CFG_RESET_SUPPORT
	"mt_mod reset id|name [flag] - reset module, flag: 0x1 exception reset\n"
#endif
	"mt_mod dump - dump state\n"
);
#endif

