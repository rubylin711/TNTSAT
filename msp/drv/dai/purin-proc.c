/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/skbuff.h>
#include "purin-proc.h"

#define SND_PROCFS_NAME "snd"
#define SEQ_FILE_BUFSIZE        PAGE_SIZE
#define MAX_CMD_STRING_LENGTH   256

static char ___procfs_cmd[MAX_CMD_STRING_LENGTH];

#if defined(CONFIG_PM)
extern void audio_relax(void);
#endif

struct snd_info snd_proc_info;

struct snd_config
{
    char *name;
    void *data;
    int (*config_set)(void *, void *, int);
    int (*config_get)(void *, struct seq_file *, int);
    int (*update)(void *, struct seq_file *, int);
    char *hint;
};

int snd_config_set_int(void *cfg, void *input, int index)
{
    struct snd_config *working_cfg = (struct snd_config *) cfg;
    int *working_data = (int *) working_cfg->data;
    *working_data = simple_strtoul(input, NULL, 10);

    printk(KERN_DEBUG "snd: set %s=%d\n", working_cfg->name, *working_data);

    return 0;
}

int snd_config_get_int(void *cfg, struct seq_file *s, int index)
{
    struct snd_config *working_cfg = (struct snd_config *) cfg;
    int *working_data = (int *) working_cfg->data;

    seq_printf(s, "%d\n", *working_data);
    printk("snd: %s=%d\n",working_cfg->name, *working_data);

    return 0;
}

int snd_config_set_wakelock(void *cfg, void *input, int index)
{
#if defined(CONFIG_PM)
    struct snd_config *working_cfg = (struct snd_config *) cfg;
    int *working_data = (int *) working_cfg->data;
    *working_data = simple_strtoul(input, NULL, 10);

    if (*working_data == SND_WAKELOCK_DISABLE)
    {
        audio_relax();
    }

    printk(KERN_DEBUG "snd: set %s=%d\n", working_cfg->name, *working_data);
#endif
    return 0;
}

static struct snd_config config[] =
{
    { "snd_wakelock_enabled", &snd_proc_info.wakelock_enabled,
        snd_config_set_wakelock, snd_config_get_int, NULL ,"\n        Enable/Disable wakelock", },
    { "snd_debug_type", &snd_proc_info.debug_type,
        snd_config_set_int, snd_config_get_int, NULL ,     "\n        Display what type of debug msg"
                                                           "\n        BIT0 - Power Manager"
                                                           "\n        BIT1 - REG UPDATE"
                                                           "\n        BIT2 - Recovery", },
    { "snd_wakelock_isr_skiptime", &snd_proc_info.wakelock_isr_skiptime,
        snd_config_set_int, snd_config_get_int, NULL ,     "\n        Not to stayawake again before "
                                                           "exceed this timeout(in jiffies)"},
};

#define NUM_OF_CONFIGS  (sizeof(config)/sizeof(struct snd_config))
#undef isdigit
#define isdigit(c)	('0' <= (c) && (c) <= '9')

void snd_procfs_show_usage(struct seq_file *s)
{
    int i;

    seq_printf(s, "Configurable attributes list\n");

    for(i=0;i<NUM_OF_CONFIGS;i++)
    {
        seq_printf(s, "   %s", config[i].name);

        if (config[i].hint)
            seq_printf(s, "\t\t%s", config[i].hint);

        seq_printf(s, "\n");
    }

    seq_printf(s, "\nExample:\n");
    seq_printf(s, "   echo snd_debug_type=1 > /proc/snd\n");
}

static int snd_procfs_exec_get_command(char* cmd, struct seq_file *s)
{
    int i;
    int k;
    int index = 0;

    //printk(KERN_DEBUG "snd: get %s\n", cmd);

    if (0==strlen(cmd))
    {
        snd_procfs_show_usage(s);
        return 0;
    }

    k = strlen(cmd) - 1;
    if (k>=0 && isdigit(cmd[k]))
    {
        index = cmd[k] - '0';
        k--;

        if (k>=0 && isdigit(cmd[k]))
        {
            index += (cmd[k] - '0') * 10;
            k--;
        }

        if ((k>=0)&&(cmd[k]=='.'))
            cmd[k] = '\0';
    }

    for(i=0;i<NUM_OF_CONFIGS;i++)
    {
        if (!strcmp(config[i].name, cmd))
        {
            if (config[i].config_get)
                config[i].config_get(&config[i], s, 0);

            break;
        }
    }

    return 0;
}

static int snd_procfs_exec_set_command(char* cmd)
{
    char *p;
    int i;
    int k;
    int index = 0;

    if (0==strlen(cmd))
        return 0;

    p = strchr(cmd, '=');
    if (p==NULL)
        return 0;

    *p = '\0';
    p++;

    k = strlen(cmd) - 1;
    if (k>=0 && isdigit(cmd[k]))
    {
        index = cmd[k] - '0';
        k--;

        if (k>=0 && isdigit(cmd[k]))
        {
            index += (cmd[k] - '0') * 10;
            k--;
        }

        if ((k>=0)&&(cmd[k]=='.'))
            cmd[k] = '\0';
    }

    for(i=0;i<NUM_OF_CONFIGS;i++)
    {
        if (!strcmp(config[i].name, cmd))
        {
            if (config[i].config_set)
                config[i].config_set(&config[i], p, index);

            break;
        }
    }

    return 1;
}

static int snd_procfs_show(struct seq_file *s, void *priv)
{
    char user_cmd[MAX_CMD_STRING_LENGTH];

    strcpy(user_cmd, ___procfs_cmd);
    ___procfs_cmd[0] = '\0';

    snd_procfs_exec_get_command(user_cmd, s);
    return 0;
}

static int snd_procfs_open(struct inode *inode, struct file *file)
{
    int ret;
    struct seq_file *p;

    ret = single_open(file, snd_procfs_show, NULL);
    if (ret==0)
    {
        p = (struct seq_file *) file->private_data;

        if (p->buf==NULL)
        {
            p->buf = kmalloc(SEQ_FILE_BUFSIZE, GFP_KERNEL);

            if (p->buf)
                p->size = SEQ_FILE_BUFSIZE;
        }
    }

    return ret;
}

static ssize_t snd_procfs_write(struct file *file, const char __user *buffer, size_t count, loff_t *data)
{
    int copy_count;
    char user_cmd[MAX_CMD_STRING_LENGTH];

    copy_count = (count > (MAX_CMD_STRING_LENGTH - 1)) ? (MAX_CMD_STRING_LENGTH - 1):count;

    if (0==copy_from_user(user_cmd, buffer, copy_count))
    {
        if (copy_count>0)
        {
            user_cmd[copy_count] = '\0';
            if (user_cmd[copy_count-1]=='\n')
                user_cmd[copy_count-1] = '\0';
        }
        else
        {
            user_cmd[0] = '\0';
        }

        if (!snd_procfs_exec_set_command(user_cmd))
        {
            strcpy(___procfs_cmd, user_cmd);
        }

        return copy_count;
    }
    else
    {
        ___procfs_cmd[0] = '\0';

        return -EIO;
    }
}

static const struct file_operations snd_procfs_fops = {
    .open       = snd_procfs_open,
    .read       = seq_read,
    .llseek     = seq_lseek,
    .release    = single_release,
    .write      = snd_procfs_write,
};

int snd_procfs_init(void)
{
    if (NULL==proc_create(SND_PROCFS_NAME, S_IWUSR, NULL, &snd_procfs_fops))
    {
        return -EIO;
    }

    snd_proc_info.debug_type = 0;
    snd_proc_info.wakelock_enabled = SND_WAKELOCK_ENABLE;
    snd_proc_info.wakelock_isr_skiptime = 5; // 5 jiffies = 50ms

    return 0;
}
