/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_kt_if.h"

void drv_kt_aux_init(MT_KT_AUX_S *p_aux)
{
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_ch0), 1));
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_ch1), 1));
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_ch2), 1));
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_ch3), 1));
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_rsa), 1));
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_ecc), 1));
	//MT_ASSERT(MT_TRUE == mtos_sem_create(&(p_aux->sem_kt), 1));
	
	sema_init(&p_aux->sem_kt, 1);
}

void drv_kt_aux_deinit(MT_KT_AUX_S *p_aux)
{

}

void drv_kt_check_private(void *priv)
{
	static void *exclusive_addr_in_project = 0;

	if (priv)
	{
		if (0 == exclusive_addr_in_project)
		{
			exclusive_addr_in_project = priv;
		}
		else
		{
			MT_ASSERT(exclusive_addr_in_project == priv);
		}
	}
	else
	{
		MT_ASSERT(0 == exclusive_addr_in_project);
	}
}

void drv_kt_lock(void *priv)
{
	MT_KT_AUX_S *p_aux = (MT_KT_AUX_S *)priv;

	if (p_aux)
	{
		//MT_ASSERT(MT_TRUE == mtos_sem_take(&(p_aux->sem_kt), 0));
		if (down_interruptible(&p_aux->sem_kt))
		{
			printk("kt lock failed!\n");
		}
	}
}

void drv_kt_unlock(void *priv)
{
	MT_KT_AUX_S *p_aux = (MT_KT_AUX_S *)priv;

	if (p_aux)
	{
		//MT_ASSERT(MT_TRUE == mtos_sem_give(&(p_aux->sem_kt)));
		up(&p_aux->sem_kt);
	}
}

void drv_kt_data_print(s8 *string, u8 *data, u32 length, u8 align)
{
	u32 i;

	if (NULL != string)
	{
		printk("%s \n", string);
	}

	if (NULL != data)
	{
		for(i=0; i<length; i++)
		{
			printk("%02x ", data[i]);
			if(0 == ((i+1)%align))//(0 == ((i+1) & (align-1)))
			{
				printk("\n");
			}
		}
		printk("\n");
	}
}
 
