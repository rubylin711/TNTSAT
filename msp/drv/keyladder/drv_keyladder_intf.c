/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <asm/io.h>

#include "mt_drv_dev.h"
#include "mt_drv_module.h"
#include "drv_keyladder_ioctl.h"
#include "drv_keyladder.h"
#include "drv_kt_if.h"

typedef struct _kl_file_priv_s
{
    u32 take_lock;
    mt_handle handle;
} kl_file_priv_s;

static mt_device_s g_keyladder_register_data;

static void *keyladder_get_handle(void)
{
    return g_keyladder_register_data.priv;
}

static int keyladder_open(struct inode *inode, struct file *file)
{
    void *handle = get_mt_priv(iminor(inode));
    kl_file_priv_s *p_priv = (kl_file_priv_s *)kmalloc(sizeof(kl_file_priv_s), GFP_KERNEL);

    p_priv->handle = (mt_handle)handle;
    p_priv->take_lock = 0;
    file->private_data = p_priv;
    return 0;
}

static mt_s32 keyladder_drv_ioctl(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 retval = 0;
    kl_file_priv_s *p_priv = (kl_file_priv_s *)file->private_data;
    mt_handle handle = p_priv->handle;

    switch (cmd) {
    case KEYLADDER_DRV_IOC_LOCK:
        retval = DRV_KEYLADDER_Lock(handle);
        if (retval == 0) {
            p_priv->take_lock = 1;
        }
        break;
    case KEYLADDER_DRV_IOC_UNLOCK:
        p_priv->take_lock = 0;
        retval = DRV_KEYLADDER_Unlock(handle);
        break;
    case KEYLADDER_DRV_IOC_REQSEM:
        retval = DRV_KEYLADDER_ReqSem(handle);
        break;
    case KEYLADDER_DRV_IOC_RLSSEM:
        retval = DRV_KEYLADDER_RlsSem(handle);
        break;
    case KEYLADDER_DRV_IOC_SETSIGNATURE:
        retval = DRV_KEYLADDER_SetSignature(handle, (mt_u8 *)arg);
        break;
    case KEYLADDER_DRV_IOC_SELECTROOTKEY:
        retval = DRV_KEYLADDER_SelectRootkey(handle, *(mt_u32 *)arg);
        break;
    case KEYLADDER_DRV_IOC_LINKAES:
    {
        KL_LINK_CRYPTO_S *p_linkcrypto = (KL_LINK_CRYPTO_S *)arg;
        retval = DRV_KEYLADDER_LinkAES(handle, p_linkcrypto->input, p_linkcrypto->enc_ptye);
    }
    break;
    case KEYLADDER_DRV_IOC_LINKTDES:
    {
        KL_LINK_CRYPTO_S *p_linkcrypto = (KL_LINK_CRYPTO_S *)arg;
        retval = DRV_KEYLADDER_LinkTDES(handle, p_linkcrypto->input, p_linkcrypto->enc_ptye);
    }
    break;
    case KEYLADDER_DRV_IOC_LINKXOR:
        retval = DRV_KEYLADDER_LinkXOR(handle, (mt_u8 *)arg);
        break;
    case KEYLADDER_DRV_IOC_LINKHASH:
    {
        KL_LINK_HASH_S *p_linkhash = (KL_LINK_HASH_S *)arg;
        retval = DRV_KEYLADDER_LinkHash(handle, p_linkhash->input, p_linkhash->input_data_pos, p_linkhash->output_key_pos);
    }
    break;
    case KEYLADDER_DRV_IOC_LINKSEEDV:
    {
        KL_LINK_SEEDV_S *p_linkseedv = (KL_LINK_SEEDV_S *)arg;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        retval = DRV_KEYLADDER_LinkSeedv(handle, p_linkseedv->input, p_linkseedv->mask_key, p_linkseedv->profile);
#else
        retval = DRV_KEYLADDER_LinkSeedv(handle, p_linkseedv->mask_key, p_linkseedv->profile);
#endif
    }
    break;
    case KEYLADDER_DRV_IOC_STOREKEY:
        retval = DRV_KEYLADDER_StoreKey(handle, *(mt_u32 *)arg);
        break;
    case KEYLADDER_DRV_IOC_EXPORTKEY:
    {
        KL_EXPORT_KEY_S *p_exportkey = (KL_EXPORT_KEY_S *)arg;
        retval = DRV_KEYLADDER_ExportKey(handle, p_exportkey->export_dst, p_exportkey->slot_id);
    }
    break;
    case KEYLADDER_DRV_IOC_INPUTDATA:
    {
        KL_INPUT_DATA_S *p_inputdata = (KL_INPUT_DATA_S *)arg;
        retval = DRV_KEYLADDER_InputData(handle, p_inputdata->postion, p_inputdata->input);
    }
    break;
    case KEYLADDER_DRV_IOC_MOVECMD:
    {
        KL_MOVE_CMD_S *p_movecmd = (KL_MOVE_CMD_S *)arg;
        retval = DRV_KEYLADDER_MoveCmd(handle, p_movecmd->move_src, p_movecmd->move_dst, p_movecmd->trigger, p_movecmd->move_enc);
    }
    break;
    case KEYLADDER_DRV_IOC_STORECMD:
    {
        KL_SOTRE_CMD_S *p_storecmd = (KL_SOTRE_CMD_S *)arg;
        retval = DRV_KEYLADDER_StoreCmd(handle, p_storecmd->store_src, p_storecmd->store_dst, p_storecmd->lock);
    }
    break;
    case KEYLADDER_DRV_IOC_EXPORTCMD:
    {
        KL_EXPORT_CMD_S *p_exportcmd = (KL_EXPORT_CMD_S *)arg;
        retval = DRV_KEYLADDER_ExportCmd(handle, p_exportcmd->key_src, p_exportcmd->export_dst, p_exportcmd->slot_id);
    }
    break;
    case KEYLADDER_DRV_IOC_WAITCOMPLETE:
        retval = DRV_KEYLADDER_WaitComplete(handle, (mt_u32 *)arg);
        break;
    case KEYLADDER_DRV_IOC_READKEY:
        retval = DRV_KEYLADDER_ReadKey(handle, (mt_u8 *)arg);
        break;
    case KEYLADDER_DRV_IOC_SETSIGSRC:
	{
	    KL_SETSIGSRC_S *p_setsigsrc = (KL_SETSIGSRC_S *)arg;
	    retval = DRV_KEYLADDER_SetSigSrc(handle, p_setsigsrc->cpu, p_setsigsrc->keysrc);
	}
        break;
    case KEYLADDER_DRV_IOC_LOCKSIGSRC:
	{
	    KL_LOCKSIGSRC_S *p_locksigsrc = (KL_LOCKSIGSRC_S *)arg;
	    retval = DRV_KEYLADDER_LockSigSrc(handle, p_locksigsrc->cpu, p_locksigsrc->lock);
	}
        break;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    case KEYLADDER_DRV_IOC_EXTRTDC:
        retval = DRV_KEYLADDER_ExtrTDC(handle, (mt_u8 *)arg);
        break;
    case KEYLADDER_DRV_IOC_RUNTDC:
    {
        KL_MOVE_DST_E data_dst, key_dst;
        KL_LINK_CRYPTO_S *p_linkcrypto = (KL_LINK_CRYPTO_S *)arg;
        retval = DRV_KEYLADDER_RunTDC(handle, p_linkcrypto->input, p_linkcrypto->algo_type, p_linkcrypto->enc_ptye);
        if (retval) {
            return -EIO;
        }
        data_dst = p_linkcrypto->algo_type == KL_MOVE_ALGO_TDES ? KL_MOVE_DST_TDES_DIN : KL_MOVE_DST_AES_DIN;
        key_dst = p_linkcrypto->algo_type == KL_MOVE_ALGO_TDES ? KL_MOVE_DST_TDES_KEY : KL_MOVE_DST_AES_KEY;
        retval |= DRV_KEYLADDER_MoveCmd(handle, KL_MOVE_SRC_INVT_DOUT, data_dst, KL_ADDS_MOVE, p_linkcrypto->enc_ptye);
        retval |= DRV_KEYLADDER_MoveCmd(handle, KL_MOVE_SRC_AUTO_KEY, key_dst, KL_ADDS_MOVE_AND_TRIGGER, p_linkcrypto->enc_ptye);
    }
    break;
    case KEYLADDER_DRV_IOC_ADDITION:
    {
        KL_ADDITIONS_S *p_additions = (KL_ADDITIONS_S *)arg;
        DRV_KEYLADDER_Additions(handle, p_additions->addt, p_additions->en);
    }
    break;
#endif
    default:
        break;
    }

    if (retval < 0) {
        return -EIO;
    }
    return 0;
}

static long keyladder_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, keyladder_drv_ioctl);
}

static int keyladder_close(struct inode *inode, struct file *file)
{
    kl_file_priv_s *p_priv = (kl_file_priv_s *)file->private_data;
    mt_handle handle = p_priv->handle;

	if (DRV_KEYLADDER_Close(handle)) {
		return -1;
	}

    if (p_priv->take_lock) {
		DRV_KEYLADDER_Unlock(handle);
    }
    kfree(p_priv);
    return 0;
}

struct file_operations keyladder_fops = {
    .open = keyladder_open,
    .unlocked_ioctl = keyladder_ioctl,
    .release = keyladder_close,
};

int keyladder_init(void)
{
    mt_handle handle = 0;

    DRV_KEYLADDER_Init();

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
	g_keyladder_register_data.minor = UMAP_MIN_MINOR_KEYLADDER;
	g_keyladder_register_data.owner = THIS_MODULE;
	g_keyladder_register_data.fops = &keyladder_fops;
	DRV_KEYLADDER_Open(KL_TYPE_0, &handle);
	g_keyladder_register_data.priv = (mt_void *)handle;
	sprintf(g_keyladder_register_data.devfs_name, UMAP_DEVNAME_KEYLADDER);
    if (mt_drv_dev_register(&g_keyladder_register_data) < 0) {
		MT_PRINT("register %s failed.\n", g_keyladder_register_data.devfs_name);
		return MT_FAILURE;
    }
#else
    g_keyladder_register_data.minor = UMAP_MIN_MINOR_KEYLADDER_CW;
    g_keyladder_register_data.owner = THIS_MODULE;
    g_keyladder_register_data.fops = &keyladder_fops;

    DRV_KEYLADDER_Open(KL_TYPE_0, &handle);
    g_keyladder_register_data.priv = (mt_void *)handle;
    sprintf(g_keyladder_register_data.devfs_name, UMAP_DEVNAME_KEYLADDER_CW);
    if (mt_drv_dev_register(&g_keyladder_register_data) < 0) {
		MT_PRINT("register %s failed.\n", g_keyladder_register_data.devfs_name);
		return MT_FAILURE;
    }

    DRV_KEYLADDER_Open(KL_TYPE_1, &handle);
    g_keyladder_register_data.minor = UMAP_MIN_MINOR_KEYLADDER_PVR;
    g_keyladder_register_data.priv = (mt_void *)handle;
    sprintf(g_keyladder_register_data.devfs_name, UMAP_DEVNAME_KEYLADDER_PVR);
    if (mt_drv_dev_register(&g_keyladder_register_data) < 0) {
		MT_PRINT("register %s failed.\n", g_keyladder_register_data.devfs_name);
		return MT_FAILURE;
    }
#endif
    return MT_SUCCESS;
}

int keyladder_deinit(void)
{
	mt_handle kl_handle = (mt_handle)keyladder_get_handle();

	if (DRV_KEYLADDER_Close(kl_handle)) {
		return MT_SUCCESS;
	}

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	sprintf(g_keyladder_register_data.devfs_name, UMAP_DEVNAME_KEYLADDER);
	mt_drv_dev_unregister(&g_keyladder_register_data);
#else
	sprintf(g_keyladder_register_data.devfs_name, UMAP_DEVNAME_KEYLADDER_CW);
	mt_drv_dev_unregister(&g_keyladder_register_data);
	sprintf(g_keyladder_register_data.devfs_name, UMAP_DEVNAME_KEYLADDER_PVR);
	mt_drv_dev_unregister(&g_keyladder_register_data);
#endif

	return MT_FAILURE;
}
