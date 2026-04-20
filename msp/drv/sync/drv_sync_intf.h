#ifndef __SYNC_INTF_H__
#define __SYNC_INTF_H__

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif

typedef struct mtSYNC_Intf_Register_Param{
    mt_proc_read_func  rdproc;
    mt_drv_proc_write_func    wtproc;
}SYNC_REGISTER_PARAM_S;

typedef struct mtSYNC_Priv_Data{
    struct clk *hdclk;
    struct clk *sdclk_27m;
    struct clk *tsiclk;
}SYNC_Priv_Data_S;

SYNC_S *SYNC_getInfoPtr(mt_u32 SyncId);
mt_s32  SYNC_IntfRegister(SYNC_REGISTER_PARAM_S *param);
mt_void SYNC_IntfUnRegister(mt_void);
mt_s32  SYNC_Ioctl(struct inode *inode, struct file *file, unsigned int cmd, mt_void *arg);
mt_s32  SYNC_DRV_Open(struct inode *finode, struct file  *ffile);
mt_s32  SYNC_DRV_Close(struct inode *finode, struct file  *ffile);
mt_s32  SYNC_Suspend(basedev_s *pdev, pm_message_t state);
mt_s32  SYNC_Resume(basedev_s *pdev);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif

