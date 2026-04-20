/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//#include "mt_type.h"
#include "mt_unf_ir.h"
#include "demo.h"
#include "ui_manager.h"
#include "mt_unf_keyled.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static MT_S32 g_bIrTaskRunning = MT_FALSE;
static MT_S32 g_bFrontendTaskRunning = MT_FALSE;

unsigned int g_read_timeout = 200;

static pthread_t g_frontend_task;
static pthread_t g_irtask;


static ir_attr_t ir_key[] =
{
	{IR_ID_POWER,	0xf50a7f80},
	{IR_ID_UP,		0xa15e7f80},
	{IR_ID_DOWN, 	0xa9567f80},
	{IR_ID_LEFT,	0xa45b7f80},
	{IR_ID_RIGHT,	0xa7587f80},
	{IR_ID_OK,		0xa55a7f80},
};


mt_s32 ir_proc(mt_u64 value)
{
	mt_u32 i = 0;

	for(i = 0; i < ARRAY_SIZE(ir_key); i++)
	{
		if(ir_key[i].key_value == value)
			break;
	}

	manage_update_sub_event(ROOT_ID_FRONT, 1, &value);
	if(i < ARRAY_SIZE(ir_key))
		manage_ir_proc(ir_key[i].key_id);
	else
		manage_ir_proc(IR_ID_OTHERS);

	return MT_SUCCESS;
}

void *ir_recv_thread(void *arg)
{
    int ret;
    mt_u64 key;
    char name[64];
    MT_UNF_KEY_STATUS_E status;
    printf("%s(line: %d), entry.\n", __FUNCTION__, __LINE__);
	
    while(g_bIrTaskRunning == MT_TRUE)
    {
		ret = MT_UNF_IR_GetValueWithProtocol(&status, &key, name, sizeof(name), g_read_timeout);
		if (!ret) 
		{
			printf("%s(line: %d) Received key: 0x%.08llx,\tprotocol: %s.\n", __FUNCTION__, __LINE__, key, name);
			ir_proc(key);
		}

		MT_USLEEP(50000);	
    }
	
    return (void *)0;
}

mt_s32 demo_ir_init(void)
{
	mt_s32 ret;
	
    ir_wavefilter_config_s wavefiler = {0};

	ret = MT_UNF_IR_Init();
	if (ret) 
	{
		printf("Fail to open ir dev! ret = %d\n", ret);
		return ret;
	}

	MT_UNF_IR_EnableRepKey(MT_TRUE);
	MT_UNF_IR_SetRepKeyTimeoutAttr(400);
	
    ret = MT_UNF_IR_SetFetchMode(0);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_UNF_IR_Enable (MT_TRUE, IRDA_NEC);
    wavefiler.irda_wfilt_channel_cfg[0].protocol = IRDA_NEC;
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_UNF_IR_Enable (MT_TRUE, RC_PROTO_NEC_);
    wavefiler.irda_wfilt_channel_cfg[0].protocol = RC_PROTO_NEC_;
#endif
    wavefiler.irda_wfilt_channel = 1;
	wavefiler.irda_wfilt_channel_cfg[0].addr_len = 32;
	wavefiler.irda_wfilt_channel_cfg[0].wfilt_code = 0x7F800AF5;   // 16 bit usercode | 8 bit keycode | 8 bit reversed keycode
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    MT_UNF_IR_SetWaveFilter(wavefiler);
#elif defined CONFIG_MT_CHIP_SYMPHONY6
    MT_UNF_IR_SetWaveFilter(&wavefiler);
#endif

	

	g_bIrTaskRunning = MT_TRUE;
	ret = pthread_create(&g_irtask, NULL, ir_recv_thread, NULL);
	if (ret < 0) 
	{
		 g_bIrTaskRunning = 0;
		printf("Failt to create ir thread!");
		return ret;
	}

	return MT_SUCCESS;
}

mt_s32 demo_ir_stop(void)
{
	mt_s32 ret;
	g_bIrTaskRunning = MT_FALSE;
	
	ret = pthread_join(g_irtask, NULL);
    if(MT_SUCCESS != ret)
    {
        printf("pthread_join failed.\n");
    }

	printf("%s(line: %d), return\n", __FUNCTION__, __LINE__);
}

void *frontend_recv_thread(void *arg)
{
	mt_s32 ret;
	mt_s32 u32PressStatus=0, u32KeyId = 0;

	printf("%s(line: %d), start ... \n", __FUNCTION__, __LINE__);
	while(g_bFrontendTaskRunning == MT_TRUE)
	{
		ret = MT_UNF_KEY_GetValue(&u32PressStatus, &u32KeyId);
		if ((MT_SUCCESS != ret) )
		{
			continue;
		}

		printf("%s(line: %d), status 0x%x, keyid 0x%x\n", __FUNCTION__, __LINE__, u32PressStatus, u32KeyId);
		manage_update_sub_event(ROOT_ID_FRONT, 3, &u32KeyId);
		manage_update_ui(UPDATE_MODULE, ROOT_ID_FRONT);
		
	}

	printf("%s(line: %d), end ... \n", __FUNCTION__, __LINE__);

	return;
}


mt_s32 demo_frontend_init(void)
{
	mt_s32 ret = MT_SUCCESS;
	MT_UNF_KEYLED_TYPE_V2_E keyled_type_v2;
	

	ret=MT_UNF_LED_Open();
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_LED_Open failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	keyled_type_v2.keyled_type = MT_UNF_KEYLED_TYPE_KEYADC;
	keyled_type_v2.kadc_type   = KADC_KEY_TYPE_7;
	ret = MT_UNF_KEYLED_SelectType_V2(keyled_type_v2);
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_KEYLED_SelectType_V2 failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	ret = MT_UNF_KEY_setKeyadcType(KADC_KEY_TYPE_7);
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_KEY_setKeyadcType failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret = MT_UNF_KEYLED_Hw_Init();
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_KEYLED_Hw_Init failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}

	ret = MT_UNF_KEY_RepKeyTimeoutVal(200);
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_KEY_RepKeyTimeoutVal failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret= MT_UNF_KEY_IsRepKey(FALSE);
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_KEY_IsRepKey failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	
	ret= MT_UNF_KEY_IsKeyUp(FALSE);
	if(ret != MT_SUCCESS)
	{
		printf("%s(line: %d), MT_UNF_KEYLED_SelectType failed. ret=0x%x\n", __FUNCTION__, __LINE__, ret);
		return ret;
	}
	g_bFrontendTaskRunning = MT_TRUE;
//	ret = pthread_create(&g_frontend_task, NULL, frontend_recv_thread, NULL);
	if (ret < 0) 
	{
		 g_bFrontendTaskRunning = MT_FALSE;
		printf("Failt to create ir thread!");
		return ret;
	}
	return MT_SUCCESS;
}

mt_s32 demo_frontend_stop(void)
{
	mt_s32 ret;
	g_bFrontendTaskRunning = MT_FALSE;
	
	ret = pthread_join(g_frontend_task, NULL);
    if(MT_SUCCESS != ret)
    {
        printf("pthread_join failed.\n");
    }
	printf("%s(line: %d), return\n", __FUNCTION__, __LINE__);
}


