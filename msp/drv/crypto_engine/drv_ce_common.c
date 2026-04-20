/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <asm/cacheflush.h>
#include <linux/uaccess.h>
#include "drv_ce_if.h"

void dump(const char *tag, mt_u8 *buffer, mt_u32 len)
{
    mt_u32 i = 0;
    printk("\n%s:\n", tag);
    printk("==================================\n");
    for(i = 0; i < len; i++)
    {
        if((i%16) == 0 && i != 0)
            printk("\n");
        printk("%02X ", buffer[i]);
    }
    printk("\n");
    printk("==================================\n");
}

static int drv_down_sem(void *sem)
{
    int ret = 0;
    struct completion *ce_sem = (struct completion *)sem;

    if (wait_for_completion_interruptible(ce_sem)) {
        ret = -EINTR;
        printk("%s:%d, Semaphore down error!\n", __func__, __LINE__);
    }

    return ret;
}

static void drv_up_sem(void *sem)
{
    struct completion *ce_sem = (struct completion *)sem;

    complete(ce_sem);
}

void drv_ce_aux_init(MT_CE_AUX_S *p_aux)
{
    init_completion(&p_aux->sem);
    p_aux->sem.done = 1;

    sema_init(&p_aux->sem_rsa, 1);
    sema_init(&p_aux->sem_ecc, 1);
}

void drv_ce_aux_deinit(MT_CE_AUX_S *p_aux)
{

}

void drv_ce_check_private(void *priv)
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

void drv_ce_cipher_lock(void)
{
    MT_CE_AUX_S *p_aux = (MT_CE_AUX_S *)drv_ce_get_handle();

    if (p_aux) {
        if (drv_down_sem((void *)&p_aux->sem)) {
            printk("%s:%d, m2m cipher lock failed!\n", __func__, __LINE__);
        }
    }
}

void drv_ce_cipher_unlock(void)
{
	MT_CE_AUX_S *p_aux = (MT_CE_AUX_S *)drv_ce_get_handle();

    if (p_aux) {
        drv_up_sem((void *)&p_aux->sem);
    }
}

void drv_ce_rsa_lock(mt_session session)
{
	MT_CE_AUX_S *p_aux = (MT_CE_AUX_S *)(*(mt_session *)session);

	if (p_aux)
	{
		//MT_ASSERT(MT_TRUE == mtos_sem_take(&(p_aux->sem_rsa), 0));
		if (down_interruptible(&p_aux->sem_rsa))
		{
			printk("rsa lock failed!\n");
		}
	}
}

void drv_ce_rsa_unlock(mt_session session)
{
	MT_CE_AUX_S *p_aux = (MT_CE_AUX_S *)(*(mt_session *)session);

	if (p_aux)
	{
		//MT_ASSERT(MT_TRUE == mtos_sem_give(&(p_aux->sem_rsa)));
		up(&p_aux->sem_rsa);
	}
}

void drv_ce_ecc_lock(mt_session session)
{
	MT_CE_AUX_S *p_aux = (MT_CE_AUX_S *)(*(mt_session *)session);

	if (p_aux)
	{
		//MT_ASSERT(MT_TRUE == mtos_sem_take(&(p_aux->sem_ecc), 0));
		if (down_interruptible(&p_aux->sem_ecc))
		{
			printk("ecc lock failed!\n");
		}
	}
}

void drv_ce_ecc_unlock(mt_session session)
{
	MT_CE_AUX_S *p_aux = (MT_CE_AUX_S *)(*(mt_session *)session);

	if (p_aux)
	{
		//MT_ASSERT(MT_TRUE == mtos_sem_give(&(p_aux->sem_ecc)));
		up(&p_aux->sem_ecc);
	}
}

MT_BOOL drv_ce_check_keep(mt_session session, u32 same_diff_both)
{
    return MT_TRUE;
}

void drv_ce_data_print(s8 *string, u8 *data, u32 length, u8 align)
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

void concerto_flush_dcache_by_vaddr(void *paddr, u32 size)
{
}

void concerto_invalidate_dcache_by_vaddr(void *p_addr, u32 size)
{
}

void drv_ce_lock_user(void)
{
    //printk("hold user pages\n");
//    down_read(&current->mm->mmap_sem);
}

void drv_ce_unlock_user(void)
{
//    up_read(&current->mm->mmap_sem);
    //printk("release user pages\n");
}

/* copy user data */
mt_u8 *drv_ce_copy_from_user_data(mt_u8 *buf, mt_u32 length)
{
    unsigned long bytes_not_copied = 0;
    mt_u8 *p_user_data = NULL;

    p_user_data = (mt_u8 *)kmalloc(length, GFP_KERNEL);
    if (p_user_data != NULL) {
        bytes_not_copied = copy_from_user(p_user_data, buf, (unsigned long)length);
        if (bytes_not_copied > 0) {
            kfree(p_user_data);
            return NULL;
        }
    }

    /* map success, release sem after read data */
    return p_user_data;
}

mt_s32 drv_ce_copy_to_user_data(mt_u8 *src_buf, mt_u8 *dst_buf, mt_u32 length)
{
    unsigned long bytes_not_copied = 0;

    bytes_not_copied = copy_to_user(dst_buf, src_buf, length);
    if (bytes_not_copied > 0) {
        return -1;
    }

    return CE_SUCCESS;
}
