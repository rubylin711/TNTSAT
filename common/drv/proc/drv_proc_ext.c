/******************************************************************************


******************************************************************************/
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/ctype.h>
#include <linux/version.h>
#include "mt_drv_proc.h"
#include "mt_drv_userproc.h"
#include "mt_osal.h"

#define MAX_PROC_ENTRIES 256

struct proc_dir_entry *gp_msp_proc = NULL;
struct proc_dir_entry *gp_mcomm_proc = NULL;
static mt_proc_entry_t s_proc_items[MAX_PROC_ENTRIES];

static int mt_proc_open(struct inode *inode, struct file *file)
{
    mt_proc_entry_t *item = pde_data(inode);

    if (item && item->read)
        return single_open(file, item->read, item);

    return -ENOSYS;
}

static ssize_t mt_proc_write(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    struct seq_file *s = file->private_data;
    mt_proc_entry_t *item = s->private;

    if (item->write)
        return item->write(file, buf, count, ppos);
    return -ENOSYS;
}

static long mt_proc_ioctl(struct file * pFile, unsigned int cmd, unsigned long arg)
{
    struct seq_file *s = pFile->private_data;
    mt_proc_entry_t *item = s->private;


    if (item->ioctl)
    {
        return item->ioctl(s, cmd, arg);
    }

    return 0;
}

static struct proc_ops CMPI_proc_ops = {
    .proc_open    = mt_proc_open,
    .proc_read    = seq_read,
    .proc_write   = mt_proc_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
    .proc_ioctl   = mt_proc_ioctl,
};

static int mt_is_hex(char *ptr)
{
	int i, strlength;
	strlength = strlen(ptr);

	if(strlength<=2)
	{
		return MT_FALSE;
	}

	ptr += 2;
	for (i = 0; i < strlength -2; i++)
	{
		if((ptr[i]<'0') || ptr[i]>'f')
		{
			return MT_FALSE;
		}

		if((ptr[i]>'9') && ptr[i]<'a')
		{
			return MT_FALSE;
		}
	}
	return MT_TRUE;
}

static mt_s32 mt_string_to_digit(unsigned char  *ptr, mt_u32 *pu32Para)
{
	int i, strlength;
	mt_u32 u32Value = 0;

	strlength = strlen(ptr);
	if(strlength<=2)
	{
		return MT_FAILURE;
	}

	ptr += 2;
	for(i=0;i<strlength-2;i++)
	{
		u32Value <<= 4;
		if(ptr[i] <= '9')
			u32Value += ptr[i] - '0';
		else
			u32Value += ptr[i] - 'a' + 0x0a;
	}
    *pu32Para = u32Value;

	return MT_SUCCESS;
}

static mt_s32 mt_proc_paser_cmd(char *ptr, mt_u32 *pu32para1, mt_u32 *pu32para2)
{
    int i;
    mt_char the_args[2][64];

    while(*ptr==' ' && *ptr++ != '\0');

    /* covert into lowercase string */
    for (i = 0; i < strlen(ptr); i++)
    {
        ptr[i] = tolower(ptr[i]);
    }

    for(i=strlen(ptr);i>0;i--)
    {
		if((*(ptr+i-1) < '0') || (*(ptr+i-1) > 'f'))
		{
			*(ptr+i-1) = '\0';
		}
		else if((*(ptr+i-1) > '9') && (*(ptr+i-1) < 'a'))
		{
			*(ptr+i-1) = '\0';
		}
		else
			break;
    }

    for(i=0;i<2;i++)
    {
		int j = 0;
		while(*ptr==' ' && *ptr++ != '\0');
		while((*ptr !=' ') && (*ptr!='\0'))
		{
			the_args[i][j++] = *ptr++;
		}
		the_args[i][j] = '\0';
		if('\0' == *ptr)
		{
			i++;
			break;
		}
		the_args[i][j] = '\0';
    }

    if(('0' != the_args[0][0]) ||('x' != the_args[0][1]) ||
		('0' != the_args[1][0]) ||('x' != the_args[1][1]))
    {
		return -1;
    }
    if((!mt_is_hex(the_args[0])) || (!mt_is_hex(the_args[1])))
    {
		return -1;
    }

    if(MT_SUCCESS != mt_string_to_digit(the_args[0], pu32para1))
    {
        return -1;
    }

    if(MT_SUCCESS != mt_string_to_digit(the_args[1], pu32para2))
    {
        return -1;
    }

    return i;
}

static mt_void mwproc_show_help(mt_void)
{
	mt_drv_proc_echohelp("\nPLS type \"echo 0xxxxxxxxx 0xxxxxxxxx > /proc/msp/demux\"\n");
	mt_drv_proc_echohelp("E.g.: \"echo 0x40002 0x4010 > /proc/msp/demux\"\n");
}

ssize_t mt_drv_proc_mwrite(struct file * file,
    const char __user *buf, size_t count, loff_t *ppos, mt_proc_ctrl_func fun_ctl)
{
    char szBuffer[64];
    char *ptrBuf = szBuffer;
    mt_u32 para1, para2;

    if(count >= sizeof(szBuffer))
    {
        mwproc_show_help();
        goto out;
    }

    memset(szBuffer, 0, sizeof(szBuffer));
    if(copy_from_user(ptrBuf, buf, count))
        return -EFAULT;

    szBuffer[count > 1 ? count -1 : 0] = '\0' ;

    //printk("proc command:%s\n", szBuffer);
    if (mt_proc_paser_cmd(szBuffer, &para1, &para2) > 0)
    {
        //printk("value:0x%x, 0x%x\n", para1,  para2);
        fun_ctl(para1, para2);
    }
    else
    {
        mwproc_show_help();
    }

out:
    *ppos = count;
    return count;
}


static mt_proc_entry_t *mt_proc_drv_add_module(char *entry_name, mt_drv_proc_t* pFnOp, void * data)
{
#if !(0 == MT_PROC_SUPPORT)
    struct proc_dir_entry *entry;
    int i;

    if ((MT_NULL == entry_name) || (strlen(entry_name) > MAX_ENTRY_NAME_LEN))
    {
        return NULL;
    }

    for (i = 0; i < MAX_PROC_ENTRIES; i++)
        if (! s_proc_items[i].entry)
            break;

    if (MAX_PROC_ENTRIES == i)
        return NULL;

    mt_osal_strncpy(s_proc_items[i].entry_name, entry_name, sizeof(s_proc_items[i].entry_name) - 1);


    if (pFnOp != NULL)
    {
        s_proc_items[i].read = pFnOp->fnRead;
        s_proc_items[i].write = pFnOp->fnWrite;
        s_proc_items[i].ioctl = pFnOp->fnIoctl;
    }
    else
    {
        s_proc_items[i].read = NULL;
        s_proc_items[i].write = NULL;
        s_proc_items[i].ioctl = NULL;
    }

    s_proc_items[i].data = data;

    entry = proc_create_data(entry_name, 0, gp_msp_proc, &CMPI_proc_ops, &s_proc_items[i]);
    if (!entry)
        return NULL;

    s_proc_items[i].entry = entry;

    return &s_proc_items[i];
#else
    return &s_proc_items[0];
#endif
}

static mt_void mt_proc_drv_rm_module(char *entry_name)
{
#if !(0 == MT_PROC_SUPPORT)
    int i;
    for (i = 0; i < MAX_PROC_ENTRIES; i++)
        if (! mt_osal_strncmp(s_proc_items[i].entry_name, entry_name, sizeof(s_proc_items[i].entry_name)))
            break;
    if (MAX_PROC_ENTRIES == i)
        return ;

    remove_proc_entry(s_proc_items[i].entry_name, gp_msp_proc);
    s_proc_items[i].entry = NULL;

#endif
}

static mt_proc_param_t  procparam = {
	.addfunc = mt_proc_drv_add_module,
	.rmfunc = mt_proc_drv_rm_module,
};


mt_s32 mt_drv_proc_init(mt_void)
{
	memset(s_proc_items, 0x00, sizeof(s_proc_items));
	mt_drv_proc_set_param(&procparam);

#if !(0 == MT_PROC_SUPPORT)
    gp_mcomm_proc = proc_mkdir("mt", NULL);
	gp_msp_proc = proc_mkdir("msp", gp_mcomm_proc);

    proc_mkdir("graphics", gp_mcomm_proc);
    proc_symlink("msp", NULL, "mt/msp");
    proc_symlink("graphics", NULL, "mt/graphics");
#endif
	return MT_SUCCESS;
}

mt_void mt_drv_proc_exit(mt_void)
{
	mt_drv_proc_clear_param();

#if !(0 == MT_PROC_SUPPORT)
    remove_proc_entry("msp", NULL);
    remove_proc_entry("graphics", NULL);
    remove_proc_entry("msp", gp_mcomm_proc);
    remove_proc_entry("graphics", gp_mcomm_proc);
    remove_proc_entry("mt", NULL);
#endif
    return;
}

EXPORT_SYMBOL(mt_drv_proc_mwrite);

