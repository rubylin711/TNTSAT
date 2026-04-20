/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_kt_if.h"

typedef struct _MT_KT_HANDLE_S
{
    /*!
	The private data of low level driver.
	*/
    void *p_priv;
} MT_KT_HANDLE_S, *MT_KT_HANDLE_P;

extern void drv_kt_lock(void *priv);
extern void drv_kt_unlock(void *priv);

mt_s32 keytable_init(MT_KT_HANDLE_P hdlr)
{
    void *p_aux = kmalloc(sizeof(MT_KT_AUX_S), GFP_KERNEL);

    kt_status_init();

    if (p_aux) {
        hdlr->p_priv = p_aux;
        drv_kt_aux_init(hdlr->p_priv);

        return KT_SUCCESS;
    } else {
        return KT_INIT_FAILED;
    }
}

mt_s32 keytable_deinit(MT_KT_HANDLE_P hdlr)
{
    drv_kt_aux_deinit(hdlr->p_priv);
    kfree(hdlr->p_priv);

    return KT_SUCCESS;
}

static int kt_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int kt_close(struct inode *inode, struct file *file)
{
    return 0;
}

static mt_s32 kt_drv_ioctl(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
    MT_KT_HANDLE_P hdlr = (MT_KT_HANDLE_P)get_mt_priv(iminor(inode));
    mt_s32 ret = KT_SUCCESS;


    switch (cmd) {
    case KT_IOC_SLOT_REQUEST:
    {
        CMD_KT_SLOT_REQUEST_S *cmd_kt_slot_request = (CMD_KT_SLOT_REQUEST_S *)arg;
        drv_kt_slot_request(hdlr->p_priv, &(cmd_kt_slot_request->slot_id));
    }
	break;
    case KT_IOC_SLOT_REQUEST_MULTI:
    {
        CMD_KT_SLOT_REQUEST_MULTI_S *cmd_kt_slot_request_multi = (CMD_KT_SLOT_REQUEST_MULTI_S *)arg;
        drv_kt_slot_request_multi(hdlr->p_priv, cmd_kt_slot_request_multi->num, cmd_kt_slot_request_multi->slot_ids);
    }
	break;
    case KT_IOC_SLOT_RELEASE:
    {
        CMD_KT_SLOT_RELEASE_S *cmd_kt_slot_release = (CMD_KT_SLOT_RELEASE_S *)arg;
        drv_kt_slot_release(hdlr->p_priv, cmd_kt_slot_release->slot_id);
    }
	break;
    case KT_IOC_SLOT_ACTIVE:
    {
        CMD_KT_SLOT_ACTIVE_S *cmd_kt_slot_active = (CMD_KT_SLOT_ACTIVE_S *)arg;
        drv_kt_slot_active(hdlr->p_priv, cmd_kt_slot_active->slot_id, cmd_kt_slot_active->active);
    }
	break;
    case KT_IOC_WRITE_ATTR:
    {
        CMD_KT_WRITE_ATTR_S *cmd_kt_write_attr = (CMD_KT_WRITE_ATTR_S *)arg;
        drv_kt_write_attribute(hdlr->p_priv, cmd_kt_write_attr->slot_id, cmd_kt_write_attr->attr);
    }
	break;
    case KT_IOC_READ_ATTR:
    {
        CMD_KT_READ_ATTR_S *cmd_kt_read_attr = (CMD_KT_READ_ATTR_S *)arg;
        drv_kt_read_attribute(hdlr->p_priv, cmd_kt_read_attr->slot_id, &(cmd_kt_read_attr->attr));
    }
	break;
    case KT_IOC_WRITE_KEY:
    {
        CMD_KT_WRITE_KEY_S *cmd_kt_write_key = (CMD_KT_WRITE_KEY_S *)arg;
        drv_kt_write_key(hdlr->p_priv, cmd_kt_write_key->slot_id, cmd_kt_write_key->key, cmd_kt_write_key->size);
    }
	break;
    case KT_IOC_READ_KEY:
    {
        CMD_KT_READ_KEY_S *cmd_kt_read_key = (CMD_KT_READ_KEY_S *)arg;
        drv_kt_read_key(hdlr->p_priv, cmd_kt_read_key->slot_id, cmd_kt_read_key->key, cmd_kt_read_key->size);
    }
	break;
    case KT_IOC_WRITE_IV:
    {
        CMD_KT_WRITE_IV_S *cmd_kt_write_iv = (CMD_KT_WRITE_IV_S *)arg;
        drv_kt_write_iv(hdlr->p_priv, cmd_kt_write_iv->slot_id, cmd_kt_write_iv->iv, cmd_kt_write_iv->size);
    }
	break;
    case KT_IOC_READ_IV:
    {
        CMD_KT_READ_IV_S *cmd_kt_read_iv = (CMD_KT_READ_IV_S *)arg;
        drv_kt_read_iv(hdlr->p_priv, cmd_kt_read_iv->slot_id, cmd_kt_read_iv->iv, cmd_kt_read_iv->size);
    }
	break;
    case KT_IOC_GET_STATE:
    {
        CMD_KT_GET_STATE_S *cmd_kt_state = (CMD_KT_GET_STATE_S *)arg;
        drv_kt_get_state(hdlr->p_priv, cmd_kt_state->slot_id, &cmd_kt_state->state);
    }
	break;
    case KT_IOC_SLOT_INFO:
    {
        CMD_KT_INFO_S *cmd_kt_info = (CMD_KT_INFO_S *)arg;
        drv_kt_slot_info_get(hdlr->p_priv, cmd_kt_info->slot_id,
            &cmd_kt_info->status, &cmd_kt_info->valid, &cmd_kt_info->attr, &cmd_kt_info->teedata);
    }
	break;
    case KT_IOC_CONTROL:
    {
        CMD_KT_CONTROL_S *cmd_kt_control = (CMD_KT_CONTROL_S *)arg;
        drv_kt_control_get(&cmd_kt_control->size, cmd_kt_control->control);
    }
	break;
    case KT_IOC_READ_METADATA:
    {
        CMD_KT_READ_METADATA_S *cmd_kt_read_metadata = (CMD_KT_READ_METADATA_S *)arg;
        drv_kt_read_metadata(hdlr->p_priv, cmd_kt_read_metadata->slot_id, &(cmd_kt_read_metadata->metadata));
    }
	break;
    default:
	break;
    }

    if (ret < 0)
        return -EIO;

    return 0;
}

static long kt_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, kt_drv_ioctl);
}

struct file_operations kt_fops = {
    .open = kt_open,
    .unlocked_ioctl = kt_ioctl,
    .release = kt_close,
};

static mt_s32 kt_resume(basedev_s *pdev)
{
    MT_PRINT("kt resume OK\n");
    kt_status_init();

    return 0;
}

static baseops_s KT_DRVOPS = {
    .probe        = NULL,
    .remove       = NULL,
    .shutdown     = NULL,
    .prepare      = NULL,
    .complete     = NULL,
    .suspend      = NULL,
    .suspend_late = NULL,
    .resume_early = NULL,
    .resume       = kt_resume,
};

static mt_device_s g_kt_register_data;

void *drv_kt_get_handle(void)
{
    return ((MT_KT_HANDLE_P)(g_kt_register_data.priv))->p_priv;
}

//int __init keytable_setup(void)
int keytable_setup(void)
{
    MT_KT_HANDLE_P kt_hdlr = (MT_KT_HANDLE_P)kmalloc(sizeof(MT_KT_HANDLE_S), GFP_KERNEL);
    keytable_init(kt_hdlr);

    g_kt_register_data.minor = UMAP_MIN_MINOR_KEYTABLE;
    g_kt_register_data.owner = THIS_MODULE;
    g_kt_register_data.fops = &kt_fops;
    g_kt_register_data.priv = (void *)kt_hdlr;
    g_kt_register_data.drvops = &KT_DRVOPS;
    snprintf(g_kt_register_data.devfs_name, sizeof(g_kt_register_data.devfs_name), UMAP_DEVNAME_KEYTABLE);
    if (mt_drv_dev_register(&g_kt_register_data) < 0) {
	MT_PRINT("register %s failed.\n", g_kt_register_data.devfs_name);
	return MT_FAILURE;
    }

    return MT_SUCCESS;
}

//void __exit keytable_cleanup(void)
void keytable_cleanup(void)
{
    MT_KT_HANDLE_P kt_hdlr = (MT_KT_HANDLE_P)g_kt_register_data.priv;

    keytable_deinit(kt_hdlr);
    kfree(kt_hdlr);

    //sprintf(g_ce_register_data.devfs_name, UMAP_DEVNAME_CRYPTO_ENGINE);
    mt_drv_dev_unregister(&g_kt_register_data);
}
