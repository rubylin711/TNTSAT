/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <pthread.h>

#include "mt_type.h"
#include "mt_unf_keyled.h"
#include "drv_keyled_ioctl.h"


static int openHandle = -1;

#define MT_ERR_PARAM        (-3)
#define LEDMAPNUM 10
#define MAX_GRID_NUM        (4)

/* LED bitmap */
typedef struct _led_bitmap
{
	MT_U8 character;
	MT_U8 bitmap;
} led_bitmap;


static led_bitmap BCD_decode_tab[LEDMAPNUM] =
{
	{'0', 0x3F}, {'1', 0x06}, {'2', 0x5B}, {'3', 0x4F},
	{'4', 0x66}, {'5', 0x6D}, {'6', 0x7D}, {'7', 0x07},
	{'8', 0x7F}, {'9', 0x6F}
};

static MT_U8 Led_Get_Code(char cTemp)
{
    MT_U8 i, bitmap=0x00;

    for(i = 0; i < LEDMAPNUM; i ++)
    {
        if(BCD_decode_tab[i].character == cTemp)
        {
            bitmap = BCD_decode_tab[i].bitmap;
            break;
        }
    }

    return bitmap;
}

MT_S32 MT_UNF_KEYLED_Init(MT_VOID)
{
	if(openHandle < 0)
		openHandle = open("/dev/mt_keyled", O_RDWR | O_CLOEXEC);

	if(openHandle < 0)
	{
	    return MT_UNF_KEYLED_ERR_OPEN_FAIL;
	}
	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEYLED_DeInit(MT_VOID)
{
	if(openHandle > 0)
	{
	    close(openHandle);
	    openHandle = -1;
	}
    return MT_SUCCESS;
}

MT_S32 MT_UNF_KEYLED_SelectType(MT_UNF_KEYLED_TYPE_E enKeyLedType)
{
	int ret = 0;
	MT_U8 type = enKeyLedType;

	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_SELECT_TYPE, &type);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEYLED_SelectType_V2(MT_UNF_KEYLED_TYPE_V2_E KeyLedType)
{
	int ret = 0;
	keyled_select_type_v2 led_type;

	if(openHandle <= 0)
	{
		printf("[%s %d]openHandle <= 0\n", __FUNCTION__, __LINE__);
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	led_type.keyled_type = KeyLedType.keyled_type;
	led_type.kadc_type = KeyLedType.kadc_type;
	ret = ioctl(openHandle, KEYLED_IOC_SELECT_TYPE_V2, &led_type);
	if(ret < 0)
	{
		printf("[%s %d]keyled_type = %d kadc_type = %d ret = %d\n", __FUNCTION__, __LINE__,led_type.keyled_type,
			led_type.kadc_type,ret);
		return ret;
	}
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Open(MT_VOID)
{
	if(openHandle < 0)
		openHandle = open("/dev/mt_keyled", O_RDWR | O_CLOEXEC);

	if(openHandle < 0)
	{
	    return MT_UNF_KEYLED_ERR_OPEN_FAIL;
	}
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Close(MT_VOID)
{
	if(openHandle > 0)
	{
	    close(openHandle);
	    openHandle = -1;
	}
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Display(MT_U32 u32CodeValue)
{
	int ret = 0;
	MT_U32 ledval = u32CodeValue;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_DISPLAY, &ledval);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Display_Asc(MT_U8 *p_data)
{
	int ret = 0;
	//MT_U8 i;
	keyled_display_char_s ledbuf;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	if(p_data == NULL)
		return MT_SUCCESS;
	memset(&ledbuf, 0x00, sizeof(keyled_display_char_s));
	memcpy(&ledbuf, p_data, MAX_GRID_NUM);

	ret = ioctl(openHandle, KEYLED_IOC_DISPLAY_ASC, &ledbuf);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Vfd_Display(MT_U8 *p_buf, MT_U8 len)
{
	int ret = 0;

	vfd_display_char ledbuf;
	if((NULL == p_buf) || (0 == len))
	{
		printf("[%s %d]error(null pointer or len=0), len=%d\n", __FUNCTION__, __LINE__, len);
		return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	if (len > 16)
	{
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	memcpy(ledbuf.ch, p_buf, len);
	ledbuf.len= len;
	ret = ioctl(openHandle, KEYLED_IOC_VFD_DISPLAY, &ledbuf);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Vfd_Display_Special_char(MT_UNF_VFD_DIS_SPECIAL_CHAR *s_data)
{
	int ret = 0;
	//MT_U8 i;
	led_special_dis_cfg_t ledbuf;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	if(s_data == NULL)
		return MT_SUCCESS;
    ledbuf.on_off = s_data->on_off;
    ledbuf.ch = s_data->ch;

	ret = ioctl(openHandle, KEYLED_IOC_VFD_SPECIAL_CHAR_DISPLAY, &ledbuf);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Display_Bright_Level(MT_UNF_DISPLAY_LED_BRIGHT level)
{
	int ret = 0;
	keyled_display_led_bright_t value = level;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	ret = ioctl(openHandle, KEYLED_IOC_SET_BRIGHT_LEVEL, &value);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_LED_Display_LBD(MT_UNF_DIS_PLAY_LBD_E *p_data)
{
	int ret = 0;
	//MT_U8 i;
	keyled_display_lbd_s lbdbuf;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	if(p_data == NULL)
		return MT_SUCCESS;
	memset(&lbdbuf, 0x00, sizeof(keyled_display_lbd_s));
	lbdbuf.enable = p_data->enable;
	lbdbuf.type = p_data->type;
	lbdbuf.grid_pos = p_data->grid_pos;
	lbdbuf.seg_pos = p_data->seg_pos;

	ret = ioctl(openHandle, KEYLED_IOC_DISPLAY_LBD, &lbdbuf);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_LED_DisplayTime(MT_UNF_KEYLED_TIME_S stLedTime)
{
	MT_U8 hour_h = 0;
	MT_U8 hour_l = 0;
	MT_U8 min_h = 0;
	MT_U8 min_l = 0;
	MT_U32 ledval = 0;

	hour_h = (MT_U8)((stLedTime.u32Hour / 10) + '0');
	hour_l = (MT_U8)((stLedTime.u32Hour % 10) + '0');
	min_h = (MT_U8)((stLedTime.u32Minute / 10) + '0');
	min_l = (MT_U8)((stLedTime.u32Minute % 10) + '0');

	hour_h = Led_Get_Code((char)hour_h);
	hour_l = Led_Get_Code((char)hour_l);
	min_h = Led_Get_Code((char)min_h);
	min_l = Led_Get_Code((char)min_l);
	hour_l |= 0x80;

	ledval = (MT_U32)((hour_h << 24) | (hour_l << 16) | (min_h << 8) | min_l);

	return MT_UNF_LED_Display(ledval);
}

MT_S32 MT_UNF_KEY_GetValue(MT_U32 *pu32PressStatus, MT_U32 *pu32KeyId)
{
	int ret = 0;
	keyled_keyval_s kv;

	if((NULL == pu32PressStatus) || (0 == pu32KeyId))
	{
		printf("[%s %d]error(null pointer)\n", __FUNCTION__, __LINE__);
		return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_GET_VALUE, &kv);
	if(ret < 0)
	    return ret;
	*pu32PressStatus = kv.pressStatus;
	*pu32KeyId = kv.keyVal;
	return MT_SUCCESS;
}


MT_S32 MT_UNF_KEY_RepKeyTimeoutVal(MT_U32 u32RepTimeMs)
{
	int ret = 0;
	MT_U32 repkey_timeout = u32RepTimeMs;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_SET_REPKEY_TIMEOUT, &repkey_timeout);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEY_IsRepKey(MT_U32 u32IsRepKey)
{
	int ret = 0;
	MT_U32 repkey_en = u32IsRepKey;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_ENABLE_REPKEY, &repkey_en);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEY_IsKeyUp(MT_U32 u32IsKeyUp)
{
	int ret = 0;
	MT_U32 keyup_en = u32IsKeyUp;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_ENABLE_KEYUP, &keyup_en);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEY_setKeyadcType(MT_UNF_KEYLED_KADC_TYPR_E u8KadcType)
{
	int ret = 0;
	MT_U32 key_type = u8KadcType;
	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_SET_KADC_TYPE, &key_type);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEYLED_Hw_Init(MT_VOID)
{
	int ret = 0;

	if(openHandle <= 0)
	{
		printf("[%s %d]openHandle <= 0\n", __FUNCTION__, __LINE__);
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	ret = ioctl(openHandle, KEYLED_IOC_HW_INIT, NULL);
	if(ret < 0)
	{
		printf("[%s %d]ret < 0\n", __FUNCTION__, __LINE__);
	    return ret;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEYLED_SetLedPos(MT_U8 *p_buf, MT_U8 len)
{
	int ret = 0;
	keyled_param_s param;

	if((NULL == p_buf) || (0 == len))
	{
		printf("[%s %d]error(null pointer or len=0), len=%d\n", __FUNCTION__, __LINE__, len);
		return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	if(openHandle <= 0)
	{
		printf("[%s %d]openHandle <= 0\n", __FUNCTION__, __LINE__);
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	if (len > 16)
	{
		printf("[%s %d]error, len=%d, >16\n", __FUNCTION__, __LINE__, len);
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	memcpy(param.buf, p_buf, len);
	param.length = len;
	ret = ioctl(openHandle, KEYLED_IOC_SET_LED_POS, &param);
	if(ret < 0)
	{
		printf("[%s %d]ret < 0\n", __FUNCTION__, __LINE__);
	    return ret;
	}

	return MT_SUCCESS;
}

MT_S32 MT_UNF_KEYLED_SetLedMap(MT_U8 *p_buf, MT_U8 len)
{
	int ret = 0;
	keyled_param_s param;

	if((NULL == p_buf) || (0 == len))
	{
		printf("[%s %d]error(null pointer or len=0), len=%d\n", __FUNCTION__, __LINE__, len);
		return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	if(openHandle <= 0)
	{
		printf("[%s %d]openHandle <= 0\n", __FUNCTION__, __LINE__);
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	if (len > 16)
	{
		printf("[%s %d]error, len=%d, >16\n", __FUNCTION__, __LINE__, len);
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;
	}

	memcpy(param.buf, p_buf, len);
	param.length = len;
	ret = ioctl(openHandle, KEYLED_IOC_SET_LED_MAP, &param);
	if(ret < 0)
	{
		printf("[%s %d]ret < 0\n", __FUNCTION__, __LINE__);
	    return ret;
	}

	return MT_SUCCESS;
}

#ifdef CONFIG_MT_FPGA

MT_S32 MT_UNF_KEYLED_Reset(MT_U8 reset)
{
	int ret = 0;
	MT_U8 key_reset = reset;

	if(openHandle <= 0)
	    return MT_UNF_KEYLED_ERR_DEV_NOT_OPENED;

	ret = ioctl(openHandle, KEYLED_IOC_VFD_RESET, &key_reset);
	if(ret < 0)
	    return ret;
	return MT_SUCCESS;
}

#endif
