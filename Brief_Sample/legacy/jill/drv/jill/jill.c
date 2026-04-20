/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/debugfs.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <net/sock.h>
#include <net/genetlink.h>
#include "jill.h"

#define 	JILL_BUFFER_SIZE 	(32)
#define 	JILL_MAX_OPEN 		(10)

#define 	JILL_SET 		(_IOWR('J', 1, int))
#define 	JILL_CLEAR 		(_IOWR('J', 2, int))

#define	JILL_NL_NAME		"jill_nl"

#define	JILL_MINOR			0
#define	JILL_MINORS			8

/* cmd0 can match data0 and data1, cmd1 also can match data0 and data1 */
enum {
	JILL_NL_OPS_CMD0,
	JILL_NL_OPS_CMD1,
	__JILL_NL_OPS_MAX,
};
#define	JILL_NL_OPS_AMOUNT			(__JILL_NL_OPS_MAX)

enum {
	JILL_NL_ATTR_UNSPEC,
	JILL_NL_ATTR_DATA0,
	JILL_NL_ATTR_DATA1,
	__JILL_NL_ATTR_MAX,
};
#define	JILL_NL_FAMILY_ATTR_MAX		(__JILL_NL_ATTR_MAX - 1)
#define	JILL_NL_FAMILY_ATTR_AMOUNT	(__JILL_NL_ATTR_MAX)

#undef CLOSE_CLEAR_BUFFER
#define JILL_PARENT

#ifdef CLOSE_CLEAR_BUFFER
#define CLEAR_BUFFER()	(jill_dev->used == 0)
#else
#define CLEAR_BUFFER()	(0)
#endif

struct jill_device {
	struct mutex mutex;
	struct completion jill_setup_done;
	wait_queue_head_t waitqueue_used;
	wait_queue_head_t waitqueue_size;
	struct hrtimer timer;
	ktime_t period_time;
	unsigned long size;		/* bytes in buf */
	struct task_struct *jill_thread;
	struct device *dev;
	struct dentry *jill_debug_dir;
	struct dentry *jill_debug_buf;
	unsigned char *buf;
	void *gpf;
	void *platdata;
	dev_t devt;
	int id;
#ifdef JILL_IRQ
	void *irq_dev;
	u32 irq;
#endif
	unsigned long page_addr;
	u32 order;
	u8 used;
	u8 stopping;
	u8 needs_read_fill;
};

struct jill_driver {
	struct cdev cdev;
	struct class *class;
	struct jill_device *jill_dev[JILL_MINORS];
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
	u8 minor_used[JILL_MINORS];
};

struct jill_netlink_data {
	pid_t pid;
	u32 seq;
	u32 x;
	u32 y;
	u32 data01;
	u8 cmd;
	char name[32];
};

struct jill_netlink_pid_list {
	struct list_head list;
	struct jill_netlink_data jill_nl_data;
	struct net *net;
};

static struct jill_driver *jill_drv;
static struct genl_ops jill_nl_ops[JILL_NL_OPS_AMOUNT];

static inline int condition_waitqueue_size(struct jill_device *jill_dev)
{
	int condition = 0;

	while (1) {
		if (mutex_lock_interruptible(&jill_dev->mutex))
			continue;

		if ((jill_dev->size > 0) || (jill_dev->stopping))
			condition = 1;

		mutex_unlock(&jill_dev->mutex);

		return condition;
	}
}

static inline int condition_waitqueue_used(struct jill_device *jill_dev)
{
	int condition = 0;

	while (1) {
		if (mutex_lock_interruptible(&jill_dev->mutex))
			continue;

		if (jill_dev->used < JILL_MAX_OPEN)
			condition = 1;

		mutex_unlock(&jill_dev->mutex);

		return condition;
	}
}

static int jill_open(struct inode *inode, struct file *file)
{
	struct jill_device *jill_dev;
	int ret = 0;
	int idx;

	idx = iminor(inode);
	jill_dev = jill_drv->jill_dev[idx];
	file->private_data = jill_dev;

	JILL_PAINT_DEBUG("%s	 jill_dev = %p\n\n", __FUNCTION__, jill_dev);

	/* if the device no support llseek method, we should call nonseekable_open */
	//nonseekable_open(inode, file);

	if (file->f_flags & O_NONBLOCK) {			//O_NDELAY
		if (mutex_lock_interruptible(&jill_dev->mutex))
			return -ERESTARTSYS;

		if (jill_dev->used >= JILL_MAX_OPEN)
			ret = -EAGAIN;

		mutex_unlock(&jill_dev->mutex);

		if (ret)
			return ret;
	}
	else {
		while (1) {
			if (wait_event_interruptible_exclusive(jill_dev->waitqueue_used,
				condition_waitqueue_used(jill_dev)))
				return -ERESTARTSYS;

			if (mutex_lock_interruptible(&jill_dev->mutex))
				return -ERESTARTSYS;

			if (jill_dev->used < JILL_MAX_OPEN) {
				jill_dev->used += 1;
				if (jill_dev->used > JILL_MAX_OPEN)
					jill_dev->used = JILL_MAX_OPEN;

				mutex_unlock(&jill_dev->mutex);
				break;
			}

			mutex_unlock(&jill_dev->mutex);
		}
	}
	return 0;
}

static int jill_release(struct inode *inode, struct file *file)
{
	struct jill_device *jill_dev;
	jill_dev = file->private_data;

	JILL_PAINT_DEBUG("%s jill_dev = %p\n\n", __FUNCTION__, jill_dev);

	if (mutex_lock_interruptible(&jill_dev->mutex))
		return -ERESTARTSYS;

	if (jill_dev->used > 0)
		jill_dev->used -= 1;
	if (jill_dev->used < JILL_MAX_OPEN)
		wake_up_interruptible(&jill_dev->waitqueue_used);

	mutex_unlock(&jill_dev->mutex);

	return 0;
}

static loff_t jill_llseek (struct file *file, loff_t postion, int origin)
{
	struct jill_device *jill_dev;
	loff_t new_pos;

	jill_dev = file->private_data;

	switch (origin) {
	case SEEK_SET:
		new_pos = postion;
		break;

	case SEEK_CUR:
		new_pos = postion + file->f_pos;
		break;

	case SEEK_END:
		if (mutex_lock_interruptible(&jill_dev->mutex))
			return -ERESTARTSYS;

		new_pos = jill_dev->size - 1 + postion;

		mutex_unlock(&jill_dev->mutex);
		break;

	default:
		return -EINVAL;
	}

	if (new_pos < 0 || new_pos >= JILL_BUFFER_SIZE)
		return -EINVAL;

	file->f_pos = new_pos;

	return new_pos;
}

static ssize_t jill_read(struct file *file, char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long rest;
	struct jill_device *jill_dev;

	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	jill_dev = file->private_data;

	if (*postion >= JILL_BUFFER_SIZE)
		return -EINVAL;

	if (cnt + *postion > JILL_BUFFER_SIZE)
		cnt = JILL_BUFFER_SIZE - *postion;

	if (mutex_lock_interruptible(&jill_dev->mutex))
		return -ERESTARTSYS;

	rest = copy_to_user((void *)buf,
		(void *)&(jill_dev->buf[*postion]), (unsigned long)cnt);

	mutex_unlock(&jill_dev->mutex);

	if (rest == -EFAULT)
		return -EFAULT;

	*postion += (cnt - rest);

	return cnt - rest;
}

static ssize_t jill_write(struct file *file, const char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long rest;
	struct jill_device *jill_dev;

	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	jill_dev = file->private_data;

	if (*postion >= JILL_BUFFER_SIZE)
		return -EINVAL;

	if (cnt + *postion > JILL_BUFFER_SIZE)
		cnt = JILL_BUFFER_SIZE - *postion;

	if (mutex_lock_interruptible(&jill_dev->mutex))
		return -ERESTARTSYS;

	rest = copy_from_user((void *)&(jill_dev->buf[*postion]),
		(void *)buf, (unsigned long)cnt);

	if (rest == -EFAULT) {
		mutex_unlock(&jill_dev->mutex);
		return -EFAULT;
	}

	*postion += (cnt - rest);

	if (*postion > jill_dev->size) {
		jill_dev->size = *postion;
	}
	if (jill_dev->size > 0)
		wake_up_interruptible(&jill_dev->waitqueue_size);

	mutex_unlock(&jill_dev->mutex);

	return cnt - rest;
}

static long jill_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct jill_device *jill_dev;
	void __user *addr = (void __user *)arg;
	int param;
	int rest;
#ifdef CONFIG_MACH_OK2440
	volatile u8 value;
#endif

	jill_dev = file->private_data;

	switch (cmd) {
	case JILL_SET:
#if 1		/* get_user suit that the size is 1 2 4 data */
		rest = get_user(param, (unsigned int *)addr);
#else
		rest = copy_from_user((void *)&iParam, (void *)addr, sizeof(iParam));
#endif
		if (rest == -EFAULT)
			return -EFAULT;

		if (mutex_lock_interruptible(&jill_dev->mutex))
			return -ERESTARTSYS;

		memset(jill_dev->buf, param, JILL_BUFFER_SIZE);
		jill_dev->size = JILL_BUFFER_SIZE;
		if (jill_dev->size > 0)
			wake_up_interruptible(&jill_dev->waitqueue_size);

		mutex_unlock(&jill_dev->mutex);

		break;

	case JILL_CLEAR:
		if (mutex_lock_interruptible(&jill_dev->mutex))
			return -ERESTARTSYS;

		memset(jill_dev->buf, 0, JILL_BUFFER_SIZE);
		jill_dev->size = 0;
#ifdef CONFIG_MACH_OK2440
		if (jill_dev->id == 0) {
			value = ioread8(jill_dev->gpf + JILL_GPFDATA_OFFSET);
			/* high level: quench	low level: lighten */
			value |= 0x78;
			iowrite8(value, jill_dev->gpf + JILL_GPFDATA_OFFSET);
		}
#endif

		mutex_unlock(&jill_dev->mutex);

		break;

	default:
		return -ENOTTY;
	}

	return 0;
}

static int jill_mmap(struct file *file, struct vm_area_struct *vma_struct)
{
	struct jill_device *jill_dev;
	unsigned int bytes;
	static char jill_mmap_strings[] = "JillMmap JillMmap JillMmap\n";

	jill_dev = file->private_data;

	JILL_PAINT_DEBUG("%s vma_struct->vm_start = %p\n\n",
		__FUNCTION__, (unsigned long *)vma_struct->vm_start);
	JILL_PAINT_DEBUG("%s vma_struct->vm_end = %p\n\n",
		__FUNCTION__, (unsigned long *)vma_struct->vm_end);
	JILL_PAINT_DEBUG("%s vma_struct->vm_pgoff = %ld\n\n",
		__FUNCTION__, vma_struct->vm_pgoff);

	bytes = vma_struct->vm_end - vma_struct->vm_start;
	if (jill_dev->order < get_order(bytes))
		return -EINVAL;

	JILL_PAINT_DEBUG("%s get_order(%x) = %d\n\n",
		__FUNCTION__, bytes, jill_dev->order);
	JILL_PAINT_DEBUG("%s jill_dev->page_addr = %p\n\n",
		__FUNCTION__, (unsigned long *)jill_dev->page_addr);
	JILL_PAINT_DEBUG("%s page_to_pfn(virt_to_page(jill_dev->page_addr) = %lx\n\n",
		__FUNCTION__, page_to_pfn(virt_to_page(jill_dev->page_addr)));

	if (remap_pfn_range(vma_struct, vma_struct->vm_start,
		page_to_pfn(virt_to_page(jill_dev->page_addr)),
		1 << (jill_dev->order - 1), vma_struct->vm_page_prot))
		return -EAGAIN;

	memcpy((unsigned long *)jill_dev->page_addr, jill_mmap_strings, strlen(jill_mmap_strings));

	return 0;
}

static struct file_operations jill_fops = {
	.owner = THIS_MODULE,
	.llseek = jill_llseek,
	.read = jill_read,
	.write = jill_write,
	.unlocked_ioctl = jill_ioctl,
	.open = jill_open,
	.release = jill_release,
	.mmap = jill_mmap,
};

static ssize_t jill_size_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	ssize_t count = 0;
	struct jill_device *jill_dev = dev_get_drvdata(dev);

	mutex_lock(&jill_dev->mutex);
	count = snprintf(buf, PAGE_SIZE, "%ld\n", jill_dev->size);
	mutex_unlock(&jill_dev->mutex);

	return count;
}

static ssize_t jill_size_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	unsigned long ret;
	struct jill_device *jill_dev = dev_get_drvdata(dev);

	ret = simple_strtoul(buf, NULL, 0);
	mutex_lock(&jill_dev->mutex);
	jill_dev->size = ret > JILL_BUFFER_SIZE ?  JILL_BUFFER_SIZE : ret;
	mutex_unlock(&jill_dev->mutex);

	return count;
}
static DEVICE_ATTR(jill_size, 0644, jill_size_show, jill_size_store);

static ssize_t jill_debug_buf_read(struct file *file, char __user *buf, size_t cnt, loff_t *postion)
{
	struct jill_device *jill_dev;
	unsigned long rest;
	unsigned long size;
	int format_cnt = 0;
	int i;
	/*
	* use %d, every u8 number in jill_dev->buf can arrive 255 (3 characters),
	* and need space after number in output buffer, so need quadruple.
	* and need change line symbol, one '\n',
	* and need one '\0'
	*/
	u8 format_buf[(JILL_BUFFER_SIZE << 2) + 1 + 1];
	u8 jill_buf[JILL_BUFFER_SIZE];

	jill_dev = file->private_data;

	if (mutex_lock_interruptible(&jill_dev->mutex))
		return -ERESTARTSYS;

	if (*postion != 0 || !jill_dev->needs_read_fill) {
		mutex_unlock(&jill_dev->mutex);
		return 0;
	}

	jill_dev->needs_read_fill = 0;

	memcpy(jill_buf, jill_dev->buf, jill_dev->size);
	size = jill_dev->size;

	mutex_unlock(&jill_dev->mutex);

	if (!size)
		return 0;

	for (i = 0; i < size; i++)
		format_cnt += sprintf(format_buf + format_cnt, "%d ", jill_buf[i]);
	format_cnt += sprintf(format_buf + format_cnt, "\n");

	rest = copy_to_user(buf, (void *)format_buf, format_cnt);

	if (rest == -EFAULT)
		return -EFAULT;

	return format_cnt - rest;
}

static ssize_t jill_debug_buf_write(struct file *file, const char __user *buf, size_t cnt, loff_t *postion)
{
	unsigned long rest;
	struct jill_device *jill_dev;
	unsigned long size;
	char *endp;
	int i = 0;
	/*
	* aspace following every number except last number,
	* and '\0' following the last number
	*/
	u8 format_buf[JILL_BUFFER_SIZE << 2];
	u8 jill_buf[JILL_BUFFER_SIZE];

	jill_dev = file->private_data;

	if (cnt < 2) {
		/* echo > jill_buf, there is only a '\0' */
		if (mutex_lock_interruptible(&jill_dev->mutex))
			return -ERESTARTSYS;

		jill_dev->size = 0;

		mutex_unlock(&jill_dev->mutex);

		return cnt;
	}

	memset(format_buf, '\0', sizeof(format_buf));
	size = (unsigned long)cnt > (sizeof(format_buf) - 1) ? (sizeof(format_buf) - 1) : cnt;
	rest = copy_from_user((void *)format_buf, (void *)buf, size);
	if (rest == -EFAULT)
		return -EFAULT;

	endp = format_buf;
	while ((unsigned long)endp < (unsigned long)(format_buf + size)) {
		jill_buf[i] = (u8)simple_strtoul(endp, &endp, 0);
		i++;
		endp++;
	}

	if (mutex_lock_interruptible(&jill_dev->mutex))
		return -ERESTARTSYS;

	memcpy(jill_dev->buf, jill_buf, i);
	jill_dev->size = i;

	jill_dev->needs_read_fill = 1;

	if (jill_dev->size > 0)
		wake_up_interruptible(&jill_dev->waitqueue_size);

	mutex_unlock(&jill_dev->mutex);

	return cnt - rest;
}

static int jill_debug_buf_open(struct inode *inode, struct file *file)
{
	struct jill_device *jill_dev;

	simple_open(inode, file);

	jill_dev = file->private_data;

	if (mutex_lock_interruptible(&jill_dev->mutex))
		return -ERESTARTSYS;

	jill_dev->needs_read_fill = 1;

	mutex_unlock(&jill_dev->mutex);

	return 0;
}

static struct file_operations jill_debug_buf_fops = {
	.owner = THIS_MODULE,
	.open = jill_debug_buf_open,
	.read = jill_debug_buf_read,
	.write = jill_debug_buf_write,
	.llseek = no_llseek,
};

static LIST_HEAD(jill_nl_pid_list);

static DEFINE_MUTEX(jill_nl_mutex);

static struct genl_family jill_nl_family = {
	.hdrsize	= 0,
	.name = JILL_NL_NAME,
	.version = 0x1,
	.maxattr = JILL_NL_FAMILY_ATTR_MAX,
	.netnsok = true,
	.ops = jill_nl_ops,
	.n_ops = ARRAY_SIZE(jill_nl_ops)
};

static const struct nla_policy jill_policy[JILL_NL_FAMILY_ATTR_AMOUNT] = {
	[JILL_NL_ATTR_DATA0]	= { .type = NLA_BINARY, .len = sizeof(struct jill_netlink_data) },
	[JILL_NL_ATTR_DATA1]	= { .type = NLA_BINARY, .len = sizeof(struct jill_netlink_data) },
};

static int jill_nl_recvmsg0(struct sk_buff *skb, struct genl_info *info)
{
	struct jill_netlink_data *jill_nl_data;
	struct jill_netlink_pid_list *jill_nl_pid;
	int err = 0;

	JILL_PAINT_DEBUG("%s\n", __FUNCTION__);

	if (info->attrs[JILL_NL_ATTR_DATA0]) {
		jill_nl_data = nla_data(info->attrs[JILL_NL_ATTR_DATA0]);
		jill_nl_pid = kzalloc(sizeof(struct jill_netlink_pid_list), GFP_KERNEL);
		jill_nl_pid->net = sock_net(skb->sk);
		jill_nl_pid->jill_nl_data = *jill_nl_data;		/* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
		JILL_PAINT_DEBUG("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
		JILL_PAINT_DEBUG("netlink : pid = %d\n", jill_nl_data->pid);
		JILL_PAINT_DEBUG("netlink : seq = 0x%x\n", jill_nl_data->seq);
		JILL_PAINT_DEBUG("netlink : cmd = %d\n", jill_nl_data->cmd);
		JILL_PAINT_DEBUG("netlink : data01 = %d\n", jill_nl_data->data01);
		JILL_PAINT_DEBUG("netlink : name : %s, x = %u, y = %u\n", jill_nl_data->name, jill_nl_data->x, jill_nl_data->y);
		err = (!(jill_nl_data->x != jill_nl_data->y));

		if (mutex_lock_interruptible(&jill_nl_mutex))
			return -ETIME;
		list_add_tail(&(jill_nl_pid->list), &jill_nl_pid_list);
		mutex_unlock(&jill_nl_mutex);
	}

	if (info->attrs[JILL_NL_ATTR_DATA1]) {
		jill_nl_data = nla_data(info->attrs[JILL_NL_ATTR_DATA1]);
		jill_nl_pid = kzalloc(sizeof(struct jill_netlink_pid_list), GFP_KERNEL);
		jill_nl_pid->net = sock_net(skb->sk);
		jill_nl_pid->jill_nl_data = *jill_nl_data;		/* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
		JILL_PAINT_DEBUG("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
		JILL_PAINT_DEBUG("netlink : pid = %d\n", jill_nl_data->pid);
		JILL_PAINT_DEBUG("netlink : seq = 0x%x\n", jill_nl_data->seq);
		JILL_PAINT_DEBUG("netlink : cmd = %d\n", jill_nl_data->cmd);
		JILL_PAINT_DEBUG("netlink : data01 = %d\n", jill_nl_data->data01);
		JILL_PAINT_DEBUG("netlink : name : %s, x = %u, y = %u\n", jill_nl_data->name, jill_nl_data->x, jill_nl_data->y);
		err = (!(jill_nl_data->x != jill_nl_data->y));

		if (mutex_lock_interruptible(&jill_nl_mutex))
			return -ETIME;
		list_add_tail(&(jill_nl_pid->list), &jill_nl_pid_list);
		mutex_unlock(&jill_nl_mutex);
	}

	return err;
}

static int jill_nl_recvmsg1(struct sk_buff *skb, struct genl_info *info)
{
	struct jill_netlink_data *jill_nl_data;
	struct jill_netlink_pid_list *jill_nl_pid;
	int err = 0;

	JILL_PAINT_DEBUG("%s\n", __FUNCTION__);

	if (info->attrs[JILL_NL_ATTR_DATA0]) {
		jill_nl_data = nla_data(info->attrs[JILL_NL_ATTR_DATA0]);
		jill_nl_pid = kzalloc(sizeof(struct jill_netlink_pid_list), GFP_KERNEL);
		jill_nl_pid->net = sock_net(skb->sk);
		jill_nl_pid->jill_nl_data = *jill_nl_data;		/* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
		JILL_PAINT_DEBUG("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
		JILL_PAINT_DEBUG("netlink : pid = %d\n", jill_nl_data->pid);
		JILL_PAINT_DEBUG("netlink : seq = 0x%x\n", jill_nl_data->seq);
		JILL_PAINT_DEBUG("netlink : cmd = %d\n", jill_nl_data->cmd);
		JILL_PAINT_DEBUG("netlink : data01 = %d\n", jill_nl_data->data01);
		JILL_PAINT_DEBUG("netlink : name : %s, x = %u, y = %u\n", jill_nl_data->name, jill_nl_data->x, jill_nl_data->y);
		err = (!(jill_nl_data->x != jill_nl_data->y));

		if (mutex_lock_interruptible(&jill_nl_mutex))
			return -ETIME;
		list_add_tail(&(jill_nl_pid->list), &jill_nl_pid_list);
		mutex_unlock(&jill_nl_mutex);
	}

	if (info->attrs[JILL_NL_ATTR_DATA1]) {
		jill_nl_data = nla_data(info->attrs[JILL_NL_ATTR_DATA1]);
		jill_nl_pid = kzalloc(sizeof(struct jill_netlink_pid_list), GFP_KERNEL);
		jill_nl_pid->net = sock_net(skb->sk);
		jill_nl_pid->jill_nl_data = *jill_nl_data;		/* copy from sk_buff, because we will use it in other thread, and sk_buff will be released soon */
		JILL_PAINT_DEBUG("netlink : app_pid = %d\n", NETLINK_CB(skb).portid);
		JILL_PAINT_DEBUG("netlink : pid = %d\n", jill_nl_data->pid);
		JILL_PAINT_DEBUG("netlink : seq = 0x%x\n", jill_nl_data->seq);
		JILL_PAINT_DEBUG("netlink : cmd = %d\n", jill_nl_data->cmd);
		JILL_PAINT_DEBUG("netlink : data01 = %d\n", jill_nl_data->data01);
		JILL_PAINT_DEBUG("netlink : name : %s, x = %u, y = %u\n", jill_nl_data->name, jill_nl_data->x, jill_nl_data->y);
		err = (!(jill_nl_data->x != jill_nl_data->y));

		if (mutex_lock_interruptible(&jill_nl_mutex))
			return -ETIME;
		list_add_tail(&(jill_nl_pid->list), &jill_nl_pid_list);
		mutex_unlock(&jill_nl_mutex);
	}

	return err;
}

static int jill_nl_sendmsg(struct jill_device *jill_dev)
{
	struct genl_info info;
	struct jill_netlink_data jill_nl_data_snd;
	struct sk_buff *skb;
	struct jill_netlink_data *jill_nl_data;
	struct jill_netlink_pid_list *jill_nl_pid;
	struct jill_netlink_pid_list *tmp;
	void *hdr;
	pid_t pid;

	if (mutex_lock_interruptible(&jill_nl_mutex))
		return -ERESTARTSYS;

	list_for_each_entry_safe(jill_nl_pid, tmp, &jill_nl_pid_list, list) {
		jill_nl_data = &(jill_nl_pid->jill_nl_data);
		pid = jill_nl_data->pid;

		skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
		if (!skb)
			return -ENOMEM;

		hdr = genlmsg_put(skb, 0/* from kernel */,
			jill_nl_data->seq, &jill_nl_family, 0, jill_nl_data->cmd);
		if (!hdr)
			goto out_nlmsg_free;

		/*
		* hdr is first nlattr or user header which size is genl_family->hdrsize,
		* if genl_family->hdrsize is 0, there is no user header
		*/

		info.snd_portid = pid;			/* to app */
		genl_info_net_set(&info, jill_nl_pid->net);

		NETLINK_CB(skb).portid = 0; 	/* from kernel */

		snprintf(jill_nl_data_snd.name, sizeof(jill_nl_data_snd.name), "kernel nl");
		jill_nl_data_snd.x = 1;
		jill_nl_data_snd.y = 2;
		jill_nl_data_snd.pid = pid;
		jill_nl_data_snd.seq = jill_nl_data->seq + 1;
		jill_nl_data_snd.cmd = jill_nl_data->cmd;
		jill_nl_data_snd.data01 = jill_nl_data->data01;

//		NLA_PUT_TYPE(skb, typeof(jill_nl_data_snd), jill_nl_data_snd.data01, jill_nl_data_snd);
		if (nla_put(skb, jill_nl_data_snd.data01, sizeof(jill_nl_data_snd), &jill_nl_data_snd))
			goto nla_put_failure;		

		genlmsg_end(skb, hdr);

		genlmsg_reply(skb, &info);

		list_del(&(jill_nl_pid->list));
		kfree(jill_nl_pid);
	}

	mutex_unlock(&jill_nl_mutex);
	return 0;

nla_put_failure:
	genlmsg_cancel(skb, hdr);
out_nlmsg_free:
	nlmsg_free(skb);
	mutex_unlock(&jill_nl_mutex);
	return -ENOMEM;
}

static struct genl_ops jill_nl_ops[JILL_NL_OPS_AMOUNT] = {
	[JILL_NL_OPS_CMD0] = {
		.cmd = JILL_NL_OPS_CMD0,
		.doit = jill_nl_recvmsg0,
		.policy = jill_policy,
	},
	[JILL_NL_OPS_CMD1] = {
		.cmd = JILL_NL_OPS_CMD1,
		.doit = jill_nl_recvmsg1,
		.policy = jill_policy,
	},
};

static enum hrtimer_restart jill_hrtimer_callback(struct hrtimer *timer)
{
	s64 nano_second_now;
	struct jill_device *jill_dev;
	struct timespec64 timespec_current;

	jill_dev = container_of(timer, struct jill_device, timer);
	hrtimer_forward_now(timer, jill_dev->period_time);
	nano_second_now = ktime_to_ns(hrtimer_cb_get_time(timer));
	ktime_get_coarse_real_ts64(&timespec_current);
	JILL_PAINT_DEBUG("nano_second_now = %lld, CURRENT_TIME = %lld\n",
		nano_second_now, timespec64_to_ns(&timespec_current));

	return HRTIMER_RESTART;
}

static int jill_thread(void *pdata)
{
	struct jill_device *jill_dev = (struct jill_device *)pdata;
	u32 i = 0;
#ifdef CONFIG_MACH_OK2440
	volatile u8 value;
#endif

	complete(&jill_dev->jill_setup_done);
	hrtimer_start(&jill_dev->timer, jill_dev->period_time, HRTIMER_MODE_REL);

	while (!kthread_should_stop()) {

		jill_nl_sendmsg(jill_dev);

		if (wait_event_interruptible_exclusive(jill_dev->waitqueue_size,
			condition_waitqueue_size(jill_dev)))
			continue;

		if (mutex_lock_interruptible(&jill_dev->mutex))
			continue;

		if (jill_dev->size > 0 && !jill_dev->stopping) {
			if (i >= jill_dev->size)
				i = 0;

#ifdef CONFIG_MACH_OK2440
			if (jill_dev->id == 0) {
				/* high level: quench	low level: lighten */
				value = ioread8(jill_dev->gpf + JILL_GPFDATA_OFFSET);
				value &= 0x87;

				if (CLEAR_BUFFER()) {
					value |= 0x78;
					jill_dev->size = 0;
				}
				else
					value |= ~((jill_dev->buf[i] & 0x0f) << 3);

				iowrite8(value, jill_dev->gpf + JILL_GPFDATA_OFFSET);
			} else {
				if (CLEAR_BUFFER())
					jill_dev->size = 0;
				else
					JILL_PAINT_DEBUG("buf[%d] = %x\n", i, jill_dev->buf[i]);
			}
#else
			if (CLEAR_BUFFER())
				jill_dev->size = 0;
			else
				JILL_PAINT_DEBUG("buf[%d] = %x\n", i, jill_dev->buf[i]);
#endif

			i++;
		}

		mutex_unlock(&jill_dev->mutex);

		if (!jill_dev->stopping) {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(100);
		}
	}

	return 0;
}

static int jill_probe(struct platform_device *pdev)
{
	struct jill_device *jill_dev = NULL;
	int ret = 0;
	int idx;
	struct jill_platform_data *jill_dev_platdata;
#ifdef CONFIG_MACH_OK2440
	struct resource *jill_resource = NULL;
	volatile u16 setting;
	volatile u8 value;
#endif
	char name[] = JILL_DEVICE_NAME"0";
	char thread_name[] = JILL_DEVICE_NAME"0_thread";

	for (idx = 0; idx < jill_drv->minors; idx++) {
		if (!jill_drv->minor_used[idx]) {
			jill_drv->minor_used[idx] = 1;
			break;
		}
	}
	if (idx >= jill_drv->minors) {
		ret = -ENODEV;
		goto fail_alloc_minor;
	}

	jill_dev = kzalloc(sizeof(struct jill_device), GFP_KERNEL);
	if (!jill_dev) {
		JILL_PAINT_ERROR("Error kzalloc jill_dev\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_jill_dev;
	}
	jill_dev->id = pdev->id;
	jill_dev->devt = jill_drv->devt + idx;
	jill_drv->jill_dev[idx] = jill_dev;
	name[strlen(name) - 1] = '0' + jill_dev->id;		/* jill0 always corresponding jill_platform_device0 */
	thread_name[strlen(JILL_DEVICE_NAME)] = '0' + jill_dev->id;

	jill_dev->buf = kzalloc(JILL_BUFFER_SIZE, GFP_KERNEL);
	if (!jill_dev->buf) {
		JILL_PAINT_ERROR("Error kzalloc buf\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_buf;
	}

	jill_dev->order = 1;
	jill_dev->page_addr = __get_free_pages(GFP_KERNEL, jill_dev->order);
	if (!jill_dev->page_addr) {
		JILL_PAINT_ERROR("Error __get_free_pages\n\n");
		ret = -ENOMEM;
		goto fail_get_free_pages;
	}

	JILL_PAINT_DEBUG("%s jill_dev = %p\n\n", __FUNCTION__, jill_dev);

	jill_dev->platdata = dev_get_platdata(&pdev->dev);
	jill_dev_platdata = jill_dev->platdata;
	init_waitqueue_head(&jill_dev->waitqueue_used);
	init_waitqueue_head(&jill_dev->waitqueue_size);
	mutex_init(&jill_dev->mutex);
	init_completion(&jill_dev->jill_setup_done);

#ifdef CONFIG_MACH_OK2440
	if (jill_dev->id == 0) {
		jill_resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (jill_resource)
			jill_dev->gpf = ioremap(jill_resource->start, jill_resource->end - jill_resource->start + 1);
		JILL_PAINT_DEBUG("gpf = %p\n\n", jill_dev->gpf);
		if (!jill_dev->gpf) {
			ret = -ENOMEM;
			goto fail_ioremap;
		}

		setting = ioread16(jill_dev->gpf + JILL_GPFCON_OFFSET);
		setting &= jill_dev_platdata->gpf_con_and;
		setting |= jill_dev_platdata->gpf_con_or;
		iowrite16((unsigned short)setting, jill_dev->gpf + JILL_GPFCON_OFFSET);

		setting = ioread16(jill_dev->gpf + JILL_GPFUP_OFFSET);
		setting |= jill_dev_platdata->gpf_up_or;
		iowrite16((unsigned char)setting, jill_dev->gpf + JILL_GPFUP_OFFSET);

		value = ~((jill_dev_platdata->default_value & 0x0f) << 3);
		iowrite8(value, jill_dev->gpf + JILL_GPFDATA_OFFSET);
	}
#endif

#ifdef JILL_IRQ
	jill_dev->irq_dev = kzalloc(jill_irq_data_size, GFP_KERNEL);
	jill_dev->irq = platform_get_irq(pdev, 0);		/* only one irq, no need use platform_get_irq_byname */
	if (request_irq(jill_dev->irq, jill_irq_fn, name, jill_dev->irq_dev)) {
		ret = -EINVAL;
		goto fail_request_irq;
	}
#endif

#ifdef JILL_PARENT
	jill_dev->dev = device_create(jill_drv->class, &pdev->dev, jill_dev->devt, NULL, name);
#else
	jill_dev->dev = device_create(jill_drv->class, NULL, jill_dev->devt, NULL, name);
#endif
	if (IS_ERR(jill_dev->dev)) {
		ret = PTR_ERR(jill_dev->dev);
		goto fail_device_create;
	}

	if (device_create_file(&pdev->dev, &dev_attr_jill_size)) {
		ret = -ENOENT;
		goto fail_device_create_file;
	}

	jill_dev->jill_debug_dir = debugfs_create_dir(name, NULL);
	if (IS_ERR_OR_NULL(jill_dev->jill_debug_dir)) {
		ret = jill_dev->jill_debug_dir ? PTR_ERR(jill_dev->jill_debug_dir) : -ENOENT;
		goto fail_debugfs_create_dir;
	}

	jill_dev->jill_debug_buf =
		debugfs_create_file(JILL_DEVICE_NAME "_buf",
			S_IRWXU | S_IRUGO, jill_dev->jill_debug_dir, jill_dev, &jill_debug_buf_fops);
	if (IS_ERR_OR_NULL(jill_dev->jill_debug_buf)) {
		ret = jill_dev->jill_debug_buf ? PTR_ERR(jill_dev->jill_debug_buf) : -ENOENT;
		goto fail_debugfs_create_file_buf;
	}

	hrtimer_init(&jill_dev->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	jill_dev->timer.function = jill_hrtimer_callback;
	jill_dev->period_time = ktime_set(jill_dev_platdata->seconds, jill_dev_platdata->nano_seconds);

	jill_dev->jill_thread = kthread_run(jill_thread, (void *)jill_dev, thread_name);
	if (IS_ERR(jill_dev->jill_thread)) {
		JILL_PAINT_ERROR("unable to start kernel thread\n");
		ret = PTR_ERR(jill_dev->jill_thread);
		goto fail_kthread_run;
	}
	wait_for_completion(&jill_dev->jill_setup_done);

	platform_set_drvdata(pdev, jill_dev);
	dev_set_drvdata(jill_dev->dev, jill_dev);

	return 0;

fail_kthread_run:
	hrtimer_cancel(&jill_dev->timer);
	debugfs_remove(jill_dev->jill_debug_buf);
fail_debugfs_create_file_buf:
	debugfs_remove(jill_dev->jill_debug_dir);
fail_debugfs_create_dir:
	device_remove_file(&pdev->dev, &dev_attr_jill_size);
fail_device_create_file:
	device_destroy(jill_drv->class, jill_dev->devt);
	jill_dev->dev = NULL;
fail_device_create:
#ifdef JILL_IRQ
	free_irq(jill_dev->irq);
fail_request_irq:
	kfree(jill_dev->irq_dev);
	jill_dev->irq = NULL;
#endif
#ifdef CONFIG_MACH_OK2440
	if (jill_dev->id == 0) {
		iounmap(jill_dev->gpf);
		jill_dev->gpf = NULL;
	}
fail_ioremap:
#endif
	mutex_destroy(&jill_dev->mutex);
	free_pages(jill_dev->page_addr, jill_dev->order);
	jill_dev->page_addr = 0;
fail_get_free_pages:
	kfree(jill_dev->buf);
	jill_dev->buf = NULL;
fail_kzalloc_buf:
	jill_drv->jill_dev[idx] = NULL;
	kfree(jill_dev);
	jill_dev = NULL;
fail_kzalloc_jill_dev:
	jill_drv->minor_used[idx] = 0;
fail_alloc_minor:
	return ret;
}

static int jill_remove(struct platform_device *pdev)
{
	struct jill_device *jill_dev = platform_get_drvdata(pdev);
	int idx;

	JILL_PAINT_DEBUG("%s	 jill_dev = %p\n\n", __FUNCTION__, jill_dev);

	hrtimer_cancel(&jill_dev->timer);

	if (jill_dev->jill_thread) {
again:
		if (mutex_lock_interruptible(&jill_dev->mutex))
			goto again;

		jill_dev->stopping = 1;
		wake_up_interruptible(&jill_dev->waitqueue_size);

		mutex_unlock(&jill_dev->mutex);

		kthread_stop(jill_dev->jill_thread);
		jill_dev->jill_thread = NULL;
	}

	debugfs_remove(jill_dev->jill_debug_buf);
	debugfs_remove(jill_dev->jill_debug_dir);
	device_remove_file(&pdev->dev, &dev_attr_jill_size);
	device_destroy(jill_drv->class, jill_dev->devt);
	idx = jill_dev->devt - jill_drv->devt;
	jill_drv->minor_used[idx] = 0;
	jill_drv->jill_dev[idx] = NULL;
	jill_dev->dev = NULL;

	if (jill_dev->buf) {
		kfree(jill_dev->buf);
		jill_dev->buf = NULL;
	}
	jill_dev->used = 0;
	jill_dev->size = 0;

	if (jill_dev->gpf && (jill_dev->id == 0)) {
		iounmap(jill_dev->gpf);
		jill_dev->gpf = NULL;
	}

#ifdef JILL_IRQ
	free_irq(jill_dev->irq);
	kfree(jill_dev->irq_dev);
	jill_dev->irq = NULL;
#endif

	if (jill_dev->page_addr) {
		free_pages(jill_dev->page_addr, jill_dev->order);
		jill_dev->page_addr = 0;
	}

	jill_dev->platdata = NULL;

	mutex_destroy(&jill_dev->mutex);

	platform_set_drvdata(pdev, NULL);

	if (jill_dev) {
		kfree(jill_dev);
		jill_dev = NULL;
	}

	return 0;
}

static int jill_suspend(struct platform_device *pdev, pm_message_t stState)
{
	struct jill_device *jill_dev = platform_get_drvdata(pdev);

	JILL_PAINT_DEBUG("%s	 jill_dev = %p\n\n", __FUNCTION__, jill_dev);
	//suspend_thread(jill_dev->jill_thread);
	return 0;
}

static int jill_resume(struct platform_device *pdev)
{
	struct jill_device *jill_dev = platform_get_drvdata(pdev);

	JILL_PAINT_DEBUG("%s	 jill_dev = %p\n\n", __FUNCTION__, jill_dev);
	//resume_thread(jill_dev->jill_thread);
	return 0;
}

static struct platform_driver jill_platform_driver = {
	.probe = jill_probe,
	.remove = jill_remove,
	.suspend = jill_suspend,
	.resume = jill_resume,
	.driver = {
		.name = JILL_DEVICE_NAME,
		.owner = THIS_MODULE,
	}
};

static int __init jill_module_init(void)
{
	int ret;

	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	jill_drv = kzalloc(sizeof(struct jill_driver), GFP_KERNEL);
	if (!jill_drv) {
		JILL_PAINT_ERROR("Error kzalloc jill_drv\n\n");
		ret = -ENOMEM;
		goto fail_kzalloc_jill_drv;
	}

	jill_drv->class = class_create(JILL_DEVICE_NAME);
	if (IS_ERR(jill_drv->class)) {
		ret = PTR_ERR(jill_drv->class);
		goto fail_class_create;
	}

	jill_drv->minor = JILL_MINOR;
	jill_drv->minors = JILL_MINORS;
	ret = alloc_chrdev_region(&jill_drv->devt, jill_drv->minor, jill_drv->minors, JILL_DEVICE_NAME);
	if (ret) {
		JILL_PAINT_ERROR("Error alloc_chrdev_region\n\n");
		ret = -EINVAL;
		goto fail_alloc_chrdev_region;
	}
	jill_drv->major = MAJOR(jill_drv->devt);

	cdev_init(&jill_drv->cdev, &jill_fops);
	jill_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&jill_drv->cdev, jill_drv->devt, jill_drv->minors);
	if (ret) {
		JILL_PAINT_ERROR("Error cdev_add\n\n");
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	JILL_PAINT_DEBUG("major =: %d     minor: %d     count: %d\n\n",
		MAJOR(jill_drv->devt), MINOR(jill_drv->devt), jill_drv->minors);

	ret = platform_driver_register(&jill_platform_driver);
	if (ret)
		goto fail_platform_driver_register;

	ret = genl_register_family(&jill_nl_family);
	if (ret) {
		ret = -EINVAL;
		goto fail_genetlink_family_create;
	}
	JILL_PAINT_DEBUG("jill_nl_family.id = 0x%x\n", jill_nl_family.id);

	return 0;

fail_genetlink_family_create:
	platform_driver_unregister(&jill_platform_driver);
fail_platform_driver_register:
	cdev_del(&jill_drv->cdev);
fail_cdev_add:
	unregister_chrdev_region(jill_drv->devt, jill_drv->minors);
fail_alloc_chrdev_region:
	class_destroy(jill_drv->class);
	jill_drv->class = NULL;
fail_class_create:
	kfree(jill_drv);
	jill_drv = NULL;
fail_kzalloc_jill_drv:
	return ret;
}

static void __exit jill_module_exit(void)
{
	JILL_PAINT_DEBUG("%s\n\n", __FUNCTION__);

	cdev_del(&jill_drv->cdev);
	unregister_chrdev_region(jill_drv->devt, jill_drv->minors);

	platform_driver_unregister(&jill_platform_driver);

	genl_unregister_family(&jill_nl_family);

	class_destroy(jill_drv->class);
	jill_drv->class = NULL;

	kfree(jill_drv);
	jill_drv = NULL;
}

module_init(jill_module_init);
module_exit(jill_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jill");
MODULE_DESCRIPTION("S3C24XX Jill driver");
MODULE_ALIAS("platform:" JILL_DEVICE_NAME);

