/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "drv_ce_if.h"

typedef struct _MT_CE_HANDLE_S
{
    /*!
      The private data of low level driver.
      */
    void *p_priv;
} MT_CE_HANDLE_S, *MT_CE_HANDLE_P;

mt_s32 crypto_engine_init(MT_CE_HANDLE_P hdlr)
{
    void *p_aux = kmalloc(sizeof(MT_CE_AUX_S), GFP_KERNEL);

    if (p_aux)
    {
        hdlr->p_priv = p_aux;
        drv_ce_aux_init(hdlr->p_priv);

        return CE_SUCCESS;
    }
    else
    {
        return CE_INIT_FAILED;
    }
}

mt_s32 crypto_engine_deinit(MT_CE_HANDLE_P hdlr)
{
    drv_ce_aux_deinit(hdlr->p_priv);
    kfree(hdlr->p_priv);

    return CE_SUCCESS;
}

static int ce_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int ce_close(struct inode *inode, struct file *file)
{
    return 0;
}

static mt_s32 ce_drv_ioctl(struct inode *inode, struct file *file, mt_u32 cmd, mt_void *arg)
{
    mt_s32 ret = -2000;

    switch (cmd)
    {
    case CE_IOC_ADES_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_ADES_CREATE_S *cmd_ades_create = (CMD_ADES_CREATE_S *)arg;
            ret = drv_ce_ades_create(&(cmd_ades_create->session), (mt_u8)cmd_ades_create->channel, hdlr->p_priv);
            cmd_ades_create->ret = ret;
        }
        break;
    case CE_IOC_ADES_DESTROY:
        {
            CMD_ADES_DESTROY_S *cmd_ades_destroy = (CMD_ADES_DESTROY_S *)arg;
            ret = drv_ce_ades_destroy(cmd_ades_destroy->session);
            cmd_ades_destroy->ret = ret;
        }
        break;
    case CE_IOC_ADES_CONFIG:
        {
            CMD_ADES_CTRL_S *cmd_ades_ctrl = (CMD_ADES_CTRL_S *)arg;
            ret = drv_ce_ades_config(cmd_ades_ctrl->session, &(cmd_ades_ctrl->ctrl));
            cmd_ades_ctrl->ret = ret;

        }
        break;
    case CE_IOC_ADES_PROCESS:
        {
            CMD_ADES_PROCESS_S *cmd_ades_process = (CMD_ADES_PROCESS_S *)arg;
            ret = drv_ce_ades_process(cmd_ades_process->session,
                                      cmd_ades_process->is_phy_addr,
                                      cmd_ades_process->p_src_addr,
                                      cmd_ades_process->p_dst_addr,
                                      cmd_ades_process->clear_length,
                                      cmd_ades_process->length);
            cmd_ades_process->ret = ret;
        }
        break;
        /*
           case CE_IOC_ADES_PROCESS_START:
           {
           CMD_ADES_PROCESS_START_S *cmd_ades_proc_start = (CMD_ADES_PROCESS_START_S *)arg;
           ret = drv_ce_ades_process_start(cmd_ades_proc_start->session, cmd_ades_proc_start->p_iv_addr,
           cmd_ades_proc_start->p_src_addr, cmd_ades_proc_start->p_dst_addr, cmd_ades_proc_start->length);
           cmd_ades_proc_start->ret = ret;
           }
           break;
           case CE_IOC_ADES_PROCESS_POLLING:
           {
           CMD_ADES_PROCESS_POLLING_S *cmd_ades_proc_polling = (CMD_ADES_PROCESS_POLLING_S *)arg;
           ret = drv_ce_ades_process_polling(cmd_ades_proc_polling->session, cmd_ades_proc_polling->timeout);
           cmd_ades_proc_polling->ret = ret;
           }
           break;
           case CE_IOC_ADES_PROCESS_STOP:
           {
           CMD_ADES_PROCESS_STOP_S *cmd_ades_proc_stop = (CMD_ADES_PROCESS_STOP_S *)arg;
           ret = drv_ce_ades_process_stop(cmd_ades_proc_stop->session);
           cmd_ades_proc_stop->ret = ret;
           }
           break;
           */
#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    case CE_IOC_ADES_ASYNC_REQUEST:
        {
            CMD_ADES_ASYNC_REQUEST_S *cmd_ades_async_request = (CMD_ADES_ASYNC_REQUEST_S *)arg;
            ret = drv_ce_ades_async_request(cmd_ades_async_request->session,
                                            cmd_ades_async_request->is_phy_addr,
                                            cmd_ades_async_request->src,
                                            cmd_ades_async_request->dst,
                                            cmd_ades_async_request->count,
                                            cmd_ades_async_request->clear_length,
                                            cmd_ades_async_request->protected_length);
            cmd_ades_async_request->ret = ret;
        }
        break;
    case CE_IOC_ADES_ASYNC_START:
        {
            CMD_ADES_ASYNC_START_S *cmd_ades_async_start = (CMD_ADES_ASYNC_START_S *)arg;
            ret = drv_ce_ades_async_start(cmd_ades_async_start->session);
            cmd_ades_async_start->ret = ret;
        }
        break;
    case CE_IOC_ADES_ASYNC_WAIT:
        {
            CMD_ADES_ASYNC_WAIT_S *cmd_ades_async_wait = (CMD_ADES_ASYNC_WAIT_S *)arg;
            ret = drv_ce_ades_async_wait(cmd_ades_async_wait->session);
            cmd_ades_async_wait->ret = ret;
        }
        break;
#endif
    case CE_IOC_ADES_GET_INFO:
        {
            CMD_ADES_INFO_S *cmd_ades_info = (CMD_ADES_INFO_S *)arg;
            ret = drv_ce_ades_get_infor(cmd_ades_info->session, &(cmd_ades_info->ctrl));
            cmd_ades_info->ret = ret;
        }
        break;
    case CE_IOC_TS_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_TS_CREATE_S *cmd_ts_create = (CMD_TS_CREATE_S *)arg;
            ret = drv_ce_ts_create(&(cmd_ts_create->session), cmd_ts_create->channel, hdlr->p_priv);
            cmd_ts_create->ret = ret;
        }
        break;
    case CE_IOC_TS_DESTROY:
        {
            CMD_TS_DESTROY_S *cmd_ts_destroy = (CMD_TS_DESTROY_S *)arg;
            ret = drv_ce_ts_destroy(cmd_ts_destroy->session);
            cmd_ts_destroy->ret = ret;
        }
        break;
    case CE_IOC_TS_CONFIG:
        {
            CMD_TS_CTRL_S *cmd_ts_ctrl = (CMD_TS_CTRL_S *)arg;
            ret = drv_ce_ts_config(cmd_ts_ctrl->session, &(cmd_ts_ctrl->ctrl));
            cmd_ts_ctrl->ret = ret;
        }
        break;
    case CE_IOC_TS_PROCESS:
        {
            CMD_TS_PROCESS_S *cmd_ts_process = (CMD_TS_PROCESS_S *)arg;
            ret = drv_ce_ts_process(cmd_ts_process->session, cmd_ts_process->is_phy_addr, cmd_ts_process->p_src_addr, cmd_ts_process->p_dst_addr, cmd_ts_process->length);
            cmd_ts_process->ret = ret;
        }
        break;
    case CE_IOC_TS_GET_INFO:
        {
            CMD_TS_INFO_S *cmd_ts_info = (CMD_TS_INFO_S *)arg;
            ret = drv_ce_ts_get_infor(cmd_ts_info->session, &(cmd_ts_info->ctrl));
            cmd_ts_info->ret = ret;
        }
        break;
    case CE_IOC_SHA_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_SHA_CREATE_S *cmd_sha_create = (CMD_SHA_CREATE_S *)arg;
            ret = drv_ce_sha_create(&(cmd_sha_create->session), hdlr->p_priv);
            cmd_sha_create->ret = ret;
        }
        break;
    case CE_IOC_SHA_DESTROY:
        {
            CMD_SHA_DESTROY_S *cmd_sha_destroy = (CMD_SHA_DESTROY_S *)arg;
            ret = drv_ce_sha_destroy(cmd_sha_destroy->session);
            cmd_sha_destroy->ret = ret;
        }
        break;
    case CE_IOC_SHA_INIT:
        {
            CMD_SHA_INIT_S *cmd_sha_init = (CMD_SHA_INIT_S *)arg;
            ret = drv_ce_sha_init(cmd_sha_init->session, &(cmd_sha_init->ctrl));
            cmd_sha_init->ret = ret;
        }
        break;
    case CE_IOC_SHA_ATTR:
        {
            CMD_SHA_ATTR_S *cmd_sha_attr = (CMD_SHA_ATTR_S *)arg;
            ret = drv_ce_sha_get_attr(cmd_sha_attr->session, &(cmd_sha_attr->ctrl));
            cmd_sha_attr->ret = ret;
        }
        break;
    case CE_IOC_SHA_UPDATE:
        {
            CMD_SHA_UPDATE_S *cmd_sha_update = (CMD_SHA_UPDATE_S *)arg;
            ret = drv_ce_sha_update(cmd_sha_update->session, cmd_sha_update->is_phy_addr, cmd_sha_update->p_msg, cmd_sha_update->length);
            cmd_sha_update->ret = ret;
        }
        break;
    case CE_IOC_SHA_FINAL:
        {
            CMD_SHA_FINAL_S *cmd_sha_final = (CMD_SHA_FINAL_S *)arg;
            ret = drv_ce_sha_final(cmd_sha_final->session, cmd_sha_final->p_dgst);
            cmd_sha_final->ret = ret;
        }
        break;
    case CE_IOC_MAC_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_SHA_CREATE_S *cmd_sha_create = (CMD_SHA_CREATE_S *)arg;
            ret = drv_ce_mac_create(&(cmd_sha_create->session), hdlr->p_priv);
            cmd_sha_create->ret = ret;
        }
        break;
    case CE_IOC_MAC_DESTROY:
        {
            CMD_SHA_DESTROY_S *cmd_sha_destroy = (CMD_SHA_DESTROY_S *)arg;
            ret = drv_ce_mac_destroy(cmd_sha_destroy->session);
            cmd_sha_destroy->ret = ret;
        }
        break;
    case CE_IOC_MAC_INIT:
        {
            CMD_SHA_INIT_S *cmd_sha_init = (CMD_SHA_INIT_S *)arg;
            ret = drv_ce_mac_init(cmd_sha_init->session, &(cmd_sha_init->ctrl));
            cmd_sha_init->ret = ret;
        }
        break;
    case CE_IOC_MAC_UPDATE:
        {
            CMD_SHA_UPDATE_S *cmd_sha_update = (CMD_SHA_UPDATE_S *)arg;
            ret = drv_ce_mac_update(cmd_sha_update->session, cmd_sha_update->is_phy_addr, cmd_sha_update->p_msg, cmd_sha_update->length);
            cmd_sha_update->ret = ret;
        }
        break;
    case CE_IOC_MAC_FINAL:
        {
            CMD_SHA_FINAL_S *cmd_sha_final = (CMD_SHA_FINAL_S *)arg;
            ret = drv_ce_mac_final(cmd_sha_final->session, cmd_sha_final->p_dgst);
            cmd_sha_final->ret = ret;
        }
        break;

    case CE_IOC_RSA_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_RSA_CREATE_S *cmd_rsa_create = (CMD_RSA_CREATE_S *)arg;
            ret = drv_ce_rsa_create(&(cmd_rsa_create->session), hdlr->p_priv);
            cmd_rsa_create->ret = ret;
        }
        break;
    case CE_IOC_RSA_DESTROY:
        {
            CMD_RSA_DESTROY_S *cmd_rsa_destroy = (CMD_RSA_DESTROY_S *)arg;
            ret = drv_ce_rsa_destroy(cmd_rsa_destroy->session);
            cmd_rsa_destroy->ret = ret;
        }
        break;
    case CE_IOC_RSA_CONFIG:
        {
            CMD_RSA_CTRL_S *cmd_rsa_ctrl = (CMD_RSA_CTRL_S *)arg;
            ret = drv_ce_rsa_config(cmd_rsa_ctrl->session, &(cmd_rsa_ctrl->ctrl));
            cmd_rsa_ctrl->ret = ret;
        }
        break;
    case CE_IOC_RSA_PROCESS:
        {
            CMD_RSA_PROCESS_S *cmd_rsa_process = (CMD_RSA_PROCESS_S *)arg;
            DRV_PKA_RSA_CTRL_S *rsa_para;
            mt_u8 src[256];
            mt_u8 dst[256];

            rsa_para = (DRV_PKA_RSA_CTRL_S *)cmd_rsa_process->session;
            if (rsa_para == NULL)
            {
                cmd_rsa_process->ret = CE_BAD_PARAMETERS;
                break;
            }

            memset(src, 0, rsa_para->key_length);
            if (copy_from_user(src + rsa_para->key_length - cmd_rsa_process->src_length,
                               cmd_rsa_process->p_src_addr, cmd_rsa_process->src_length) != 0)
            {
                cmd_rsa_process->ret = CE_RSA_PROCESS_FAILED;
                break;
            }

            ret = drv_ce_rsa_process(cmd_rsa_process->session, src, dst, rsa_para->key_length);
            if (ret == CE_SUCCESS)
            {
                if (copy_to_user(cmd_rsa_process->p_dst_addr, dst, rsa_para->key_length) != 0)
                {
                    cmd_rsa_process->ret = CE_RSA_PROCESS_FAILED;
                    break;
                }
            }

            cmd_rsa_process->ret = ret;
        }
        break;
    case CE_IOC_BN_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_BN_CREATE_S *cmd_bn_create = (CMD_BN_CREATE_S *)arg;
            ret = drv_ce_BN_create(&(cmd_bn_create->session), hdlr->p_priv);
            cmd_bn_create->ret = ret;
        }
        break;
    case CE_IOC_BN_DESTROY:
        {
            CMD_BN_DESTROY_S *cmd_bn_destroy = (CMD_BN_DESTROY_S *)arg;
            ret = drv_ce_BN_destroy(cmd_bn_destroy->session);
            cmd_bn_destroy->ret = ret;
        }
        break;
    case CE_IOC_BN_MOD_MOD:
        {
            CMD_BN_MOD_MOD_S *cmd_bn_mod_mod = (CMD_BN_MOD_MOD_S *)arg;
            ret = drv_ce_BN_mod_mod(cmd_bn_mod_mod->session, cmd_bn_mod_mod->r, cmd_bn_mod_mod->d, cmd_bn_mod_mod->m,
                                    cmd_bn_mod_mod->d_length, cmd_bn_mod_mod->m_length);
            cmd_bn_mod_mod->ret = ret;
        }
        break;
    case CE_IOC_BN_MOD_MUL:
        {
            CMD_BN_MOD_MUL_S *cmd_bn_mod_mul = (CMD_BN_MOD_MUL_S *)arg;
            ret = drv_ce_BN_mod_mul(cmd_bn_mod_mul->session, cmd_bn_mod_mul->r, cmd_bn_mod_mul->a, cmd_bn_mod_mul->b,
                                    cmd_bn_mod_mul->m, cmd_bn_mod_mul->a_length, cmd_bn_mod_mul->b_length, cmd_bn_mod_mul->m_length);
            cmd_bn_mod_mul->ret = ret;
        }
        break;
    case CE_IOC_BN_MOD_ADD:
        {
            CMD_BN_MOD_ADD_S *cmd_bn_mod_add = (CMD_BN_MOD_ADD_S *)arg;
            ret = drv_ce_BN_mod_add(cmd_bn_mod_add->session, cmd_bn_mod_add->r, cmd_bn_mod_add->a, cmd_bn_mod_add->b,
                                    cmd_bn_mod_add->m, cmd_bn_mod_add->a_length, cmd_bn_mod_add->b_length, cmd_bn_mod_add->m_length);
            cmd_bn_mod_add->ret = ret;
        }
        break;
    case CE_IOC_BN_MOD_SUB:
        {
            CMD_BN_MOD_SUB_S *cmd_bn_mod_sub = (CMD_BN_MOD_SUB_S *)arg;
            ret = drv_ce_BN_mod_sub(cmd_bn_mod_sub->session, cmd_bn_mod_sub->r, cmd_bn_mod_sub->a, cmd_bn_mod_sub->b,
                                    cmd_bn_mod_sub->m, cmd_bn_mod_sub->a_length, cmd_bn_mod_sub->b_length, cmd_bn_mod_sub->m_length);
            cmd_bn_mod_sub->ret = ret;
        }
        break;
    case CE_IOC_BN_MOD_INV:
        {
            CMD_BN_MOD_INV_S *cmd_bn_mod_inv = (CMD_BN_MOD_INV_S *)arg;
            ret = drv_ce_BN_mod_inverse(cmd_bn_mod_inv->session, cmd_bn_mod_inv->r, cmd_bn_mod_inv->d, cmd_bn_mod_inv->m,
                                        cmd_bn_mod_inv->d_length, cmd_bn_mod_inv->m_length);
            cmd_bn_mod_inv->ret = ret;
        }
        break;
    case CE_IOC_BN_MOD_EXP:
        {
            CMD_BN_MOD_EXP_S *cmd_bn_mod_exp = (CMD_BN_MOD_EXP_S *)arg;
            ret = drv_ce_BN_mod_exp(cmd_bn_mod_exp->session, cmd_bn_mod_exp->r, cmd_bn_mod_exp->a, cmd_bn_mod_exp->p,
                                    cmd_bn_mod_exp->m, cmd_bn_mod_exp->a_length, cmd_bn_mod_exp->p_length, cmd_bn_mod_exp->m_length);
            cmd_bn_mod_exp->ret = ret;
        }
        break;
    case CE_IOC_EC_POINT_CREATE:
        {
            MT_CE_HANDLE_P hdlr = (MT_CE_HANDLE_P)get_mt_priv(iminor(inode));
            CMD_EC_POINT_CREATE_S *cmd_ec_point_create = (CMD_EC_POINT_CREATE_S *)arg;
            ret = drv_ce_EC_POINT_create(&(cmd_ec_point_create->session), hdlr->p_priv);
            cmd_ec_point_create->ret = ret;
        }
        break;
    case CE_IOC_EC_POINT_DESTROY:
        {
            CMD_EC_POINT_DESTROY_S *cmd_ec_point_destroy = (CMD_EC_POINT_DESTROY_S *)arg;
            ret = drv_ce_EC_POINT_destroy(cmd_ec_point_destroy->session);
            cmd_ec_point_destroy->ret = ret;
        }
        break;
    case CE_IOC_EC_POINT_MUL:
        {
            MT_CE_EC_PARAMS_S xParams;
            MT_CE_EC_POINT_S r, p;
            CMD_EC_POINT_MUL_S *cmd_ec_point_mul = (CMD_EC_POINT_MUL_S *)arg;
            xParams.q = cmd_ec_point_mul->xParams_q;
            xParams.a = cmd_ec_point_mul->xParams_a;
            xParams.GX = cmd_ec_point_mul->xParams_GX;
            xParams.GY = cmd_ec_point_mul->xParams_GY;
            xParams.keySize = cmd_ec_point_mul->xParams_keySize;
            r.X = cmd_ec_point_mul->r_X;
            r.Y = cmd_ec_point_mul->r_Y;
            p.X = cmd_ec_point_mul->p_point_X;
            p.Y = cmd_ec_point_mul->p_point_Y;
            if (cmd_ec_point_mul->p_point_flag)
            {
                ret = drv_ce_EC_POINT_mul(cmd_ec_point_mul->session, xParams, &r, NULL, &p, cmd_ec_point_mul->p_scalar);
            }
            else
            {
                ret = drv_ce_EC_POINT_mul(cmd_ec_point_mul->session, xParams, &r, cmd_ec_point_mul->g_scalar, NULL, NULL);
            }
            cmd_ec_point_mul->ret = ret;
        }
        break;
    case CE_IOC_EC_POINT_ADD:
        {
            MT_CE_EC_PARAMS_S xParams;
            MT_CE_EC_POINT_S r, a, b;
            CMD_EC_POINT_ADD_S *cmd_ec_point_add = (CMD_EC_POINT_ADD_S *)arg;
            xParams.q = cmd_ec_point_add->xParams_q;
            xParams.a = cmd_ec_point_add->xParams_a;
            xParams.keySize = cmd_ec_point_add->xParams_keySize;
            r.X = cmd_ec_point_add->r_X;
            r.Y = cmd_ec_point_add->r_Y;
            a.X = cmd_ec_point_add->a_X;
            a.Y = cmd_ec_point_add->a_Y;
            b.X = cmd_ec_point_add->b_X;
            b.Y = cmd_ec_point_add->b_Y;
            ret = drv_ce_EC_POINT_add(cmd_ec_point_add->session, xParams, &r, &a, &b);
            cmd_ec_point_add->ret = ret;
        }
        break;

#if defined CONFIG_MT_CHIP_SYMPHONY4 || defined(CONFIG_MT_CHIP_SYMPHONY6)
    case CE_IOC_BGC_REQUEST:
        {
            CMD_BGC_REQ_S *cmd_bgc_request = (CMD_BGC_REQ_S *)arg;
            ret = drv_ce_bgc_request(&cmd_bgc_request->session, cmd_bgc_request->req_time_out);
        }
        break;
    case CE_IOC_BGC_RELEASE:
        {
            CMD_BGC_RLS_S *cmd_bgc_release = (CMD_BGC_RLS_S *)arg;
            ret = drv_ce_bgc_release(cmd_bgc_release->session, cmd_bgc_release->req_time_out);
        }
        break;
    case CE_IOC_BGC_SETUP:
        {
            CMD_BGC_SETUP_S *cmd_bgc_setup = (CMD_BGC_SETUP_S *)arg;
            ret = drv_ce_bgc_setup(cmd_bgc_setup->session, cmd_bgc_setup->start_addr, cmd_bgc_setup->size, \
                                   cmd_bgc_setup->delay, cmd_bgc_setup->golden_hash, cmd_bgc_setup->lock);
        }
        break;
#endif

    case CE_IOC_RST:
        {
            CMD_CE_RST_S *cmd_ce_rst = (CMD_CE_RST_S *)arg;
            ret = drv_ce_reset(cmd_ce_rst->mode);
            cmd_ce_rst->ret = ret;
        }
        break;
    default:
        return -ENOTTY;
    }

    if (ret < 0)
        return -EIO;

    return 0;
}

static long ce_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, ce_drv_ioctl);
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
extern int (*p_calc_default_mac)(unsigned char *buf1, unsigned int len, unsigned char *buf2, unsigned int len2, unsigned char *p_dst_addr);

static void ce_prepare_external_api(void)
{
    extern int calc_default_mac(unsigned char *buf1, unsigned int len, unsigned char *buf2,
                         unsigned int len2, unsigned char *p_dst_addr);

    p_calc_default_mac = calc_default_mac;
}
#endif

struct file_operations ce_fops =
{
    .open                   = ce_open,
    .unlocked_ioctl         = ce_ioctl,
    .release                = ce_close,
};

static mt_device_s g_ce_register_data;

void *drv_ce_get_handle(void)
{
    return ((MT_CE_HANDLE_P)(g_ce_register_data.priv))->p_priv;
}

//int __init crypto_engine_setup(void)
int crypto_engine_setup(void)
{
    MT_CE_HANDLE_P ce_hdlr = (MT_CE_HANDLE_P)kmalloc(sizeof(MT_CE_HANDLE_S), GFP_KERNEL);
    crypto_engine_init(ce_hdlr);

    g_ce_register_data.minor	= UMAP_MIN_MINOR_CRYPTO_ENGINE;
    g_ce_register_data.owner	= THIS_MODULE;
    g_ce_register_data.fops	= &ce_fops;
    g_ce_register_data.priv	= (void *)ce_hdlr;
    sprintf(g_ce_register_data.devfs_name, UMAP_DEVNAME_CRYPTO_ENGINE);
    if (mt_drv_dev_register(&g_ce_register_data) < 0)
    {
        MT_PRINT("register %s failed.\n", g_ce_register_data.devfs_name);
        return MT_FAILURE;
    }

	drv_ce_ades_init();

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    ce_prepare_external_api();
#endif

    //printk("crypto engine init success\n");
    return MT_SUCCESS;
}

//void __exit crypto_engine_cleanup(void)
void crypto_engine_cleanup(void)
{
    MT_CE_HANDLE_P ce_hdlr = (MT_CE_HANDLE_P)g_ce_register_data.priv;
    crypto_engine_deinit(ce_hdlr);
    kfree(ce_hdlr);

	drv_ce_ades_deinit();

    //sprintf(g_ce_register_data.devfs_name, UMAP_DEVNAME_CRYPTO_ENGINE);
    mt_drv_dev_unregister(&g_ce_register_data);

    //printk("crypto engine cleanup success\n");
}
