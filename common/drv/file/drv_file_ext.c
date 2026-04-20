#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/seq_file.h>

#include "mt_type.h"
#include "mt_drv_log.h"
#include "mt_drv_file.h"
#include "drv_log.h"

struct file* mt_drv_file_open(const mt_s8* ps8FileName, mt_s32 s32Flags)
{
    struct file *pFile = NULL;

	if (NULL == ps8FileName)
	{
		return NULL;
	}

    if (s32Flags == 0)
    {
        s32Flags = O_RDONLY;
    }
    else
    {
        s32Flags = O_WRONLY | O_CREAT | O_APPEND;
    }

    pFile = filp_open(ps8FileName, s32Flags | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);

    return (IS_ERR(pFile)) ? NULL : pFile;
}

mt_void mt_drv_file_close(struct file * pFile)
{
    if ( NULL != pFile )
    {
        filp_close(pFile, NULL);
    }

	return;
}

mt_s32 mt_drv_file_read(struct file * pFile,  mt_u8* ps8Buf, mt_u32 u32Len)
{
	mt_s32 s32ReadLen = 0;

    if (pFile == NULL || NULL == ps8Buf)
    {
        return -ENOENT; /* No such file or directory */
    }

    if ((pFile->f_op->read == NULL) && (pFile->f_op->read_iter == NULL))
    {
        return -ENOSYS; /* Function not implemented */
    }

    if (((pFile->f_flags & O_ACCMODE) & (O_RDONLY | O_RDWR)) != 0)
    {
        return -EACCES; /* Permission denied */
    }

	s32ReadLen = kernel_read(pFile,ps8Buf,u32Len,&pFile->f_pos);

    return s32ReadLen;
}
mt_s32 mt_drv_file_write(struct file* pFile, mt_s8* ps8Buf, mt_u32 u32Len)
{
    mt_s32 s32WriteLen = 0;

    if (pFile == NULL || ps8Buf == NULL)
    {
        return -ENOENT; /* No such file or directory */
    }

    if ((pFile->f_op->write == NULL) && (pFile->f_op->write_iter == NULL))
    {
        return -ENOSYS; /* Function not implemented */
    }

    if (((pFile->f_flags & O_ACCMODE) & (O_WRONLY | O_RDWR)) == 0)
    {
        return -EACCES; /* Permission denied */
    }

	s32WriteLen = kernel_write(pFile,ps8Buf,u32Len,&pFile->f_pos);

    return s32WriteLen;
}

mt_s32 mt_drv_file_lseek(struct file *pFile, mt_s32 s32Offset, mt_s32 s32Whence)
{
	mt_s32 s32Ret;

	loff_t res = vfs_llseek(pFile, s32Offset, s32Whence);
	s32Ret = res;
	if (res != (loff_t)s32Ret)
		s32Ret = -EOVERFLOW;

	return s32Ret;
}

mt_s32 mt_drv_file_get_storepath(mt_s8* ps8Buf, mt_u32 u32Len)
{
    if (ps8Buf == NULL || u32Len == 0)
    {
        return MT_FAILURE;
    }

    memset(ps8Buf, 0, u32Len);

    return mt_drv_log_get_storepath(ps8Buf, u32Len);
}


EXPORT_SYMBOL(mt_drv_file_open);
EXPORT_SYMBOL(mt_drv_file_close);
EXPORT_SYMBOL(mt_drv_file_read);
EXPORT_SYMBOL(mt_drv_file_write);
EXPORT_SYMBOL(mt_drv_file_lseek);
EXPORT_SYMBOL(mt_drv_file_get_storepath);

