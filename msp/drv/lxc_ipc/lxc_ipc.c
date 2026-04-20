/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include "lxc_ipc.h"
#include "lxc_ipc_priv.h"
#include "mt_drv_mmz.h"
#include "mt_drv_dev.h"

#define LXC_IPC_MINOR UMAP_MIN_MINOR_LXC
#define LXC_IPC_MINORS  UMAP_DEV_NUM_LXC

#define LXC_IPC_BUFFERS_PER_THREAD 1024
#define LXC_IPC_BUFFER_RESIZE_MAX 16

struct lxc_ipc_driver {
	struct cdev cdev;
	struct lxc_ipc_device *lxc_ipc_dev[LXC_IPC_MINORS];
	dev_t devt;
	dev_t major;
	dev_t minor;
	u32 minors;
};

struct lxc_ipc_device {
	struct list_head list_name;
	dev_t devt;
	struct mutex mutex;
//	wait_queue_head_t waitqueue;
	struct device *dev;
	int open_cnt;
};

struct lxc_ipc_buffer {
	mmz_buffer_s in_sMBuf;
	mmz_buffer_s out_sMBuf;
	int buf_cnt;
	struct completion done;
	struct completion call;
	int do_what;
	int connected;
	bool in_buf_need;
	bool out_buf_need;
	mmz_buffer_s release_mem[LXC_IPC_BUFFER_RESIZE_MAX];
};

struct lxc_ipc_thread {
	struct list_head list;	/* join into list_thread of lxc_ipc_name  */
	struct lxc_ipc_buffer *buffers[LXC_IPC_BUFFERS_PER_THREAD];
	int tid_cnt;
	pid_t tid;
};

struct lxc_ipc_name {
	struct list_head list;	/* join into list_name of lxc_ipc_device  */
	struct list_head list_thread;
	int name_cnt;
	struct completion attach;
	char name[LXC_IPC_NAME_SIZE];
};

static struct lxc_ipc_driver *lxc_ipc_drv;
static struct class *lxc_ipc_class;

static int lxc_ipc_open(struct inode *inode, struct file *file)
{
	struct lxc_ipc_device *lxc_ipc_dev;
	int idx;

	idx = iminor(inode) - lxc_ipc_drv->minor;
	if ((idx < 0) || (idx >= lxc_ipc_drv->minors)) {
		return -ENODEV;
	}
	lxc_ipc_dev = lxc_ipc_drv->lxc_ipc_dev[idx];
	file->private_data = lxc_ipc_dev;

	/* if the device no support llseek method, we should call nonseekable_open */
	nonseekable_open(inode, file);

	mutex_lock(&lxc_ipc_dev->mutex);
	lxc_ipc_dev->open_cnt++;
	mutex_unlock(&lxc_ipc_dev->mutex);

	return 0;
}

static int lxc_ipc_release(struct inode *inode, struct file *file)
{
	struct lxc_ipc_device *lxc_ipc_dev;
	struct lxc_ipc_name *name;
	struct lxc_ipc_name *name_safe;
	struct lxc_ipc_thread *thread;
	struct lxc_ipc_thread *thread_safe;
	struct lxc_ipc_buffer *buf;
	int i;

	lxc_ipc_dev = file->private_data;

	mutex_lock(&lxc_ipc_dev->mutex);
	if (--lxc_ipc_dev->open_cnt <= 0) {
		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			list_del(&name->list);
			list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
				list_del(&thread->list);
				for (i = 0; i < LXC_IPC_BUFFERS_PER_THREAD; i++) {
					if (thread->buffers[i] != NULL) {
						buf = thread->buffers[i];
						if (buf->in_buf_need) {
							mt_drv_mmz_release(&buf->in_sMBuf);
						}
						if (buf->out_buf_need) {
							mt_drv_mmz_release(&buf->out_sMBuf);
						}
						kfree(thread->buffers[i]);
						thread->buffers[i] = NULL;
					}
				}
				kfree(thread);
			}
			kfree(name);
		}
	}
	mutex_unlock(&lxc_ipc_dev->mutex);

	return 0;
}

static bool lxc_ipc_lazy_release(struct lxc_ipc_buffer*buf, mmz_buffer_s* p)
{
    int i;

    for(i = 0; i < LXC_IPC_BUFFER_RESIZE_MAX; i++) {
        if(!buf->release_mem[i].size) {
            buf->release_mem[i] = *p;
            break;
        }

    }

    return i != LXC_IPC_BUFFER_RESIZE_MAX;

}

static long lxc_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct lxc_ipc_device *lxc_ipc_dev = file->private_data;
	struct lxc_ipc_user user;
	struct lxc_ipc_name *name;
	struct lxc_ipc_name *name_safe;
	struct lxc_ipc_thread *thread;
	struct lxc_ipc_thread *thread_safe;
	struct lxc_ipc_buffer *buf;
	size_t size_tmp;
	unsigned int order;
	unsigned int i;
	int timeout;
	int ret = 0;
	int ret_wait;
	bool found = false;
	bool buf_need;
	char mmz_name[LXC_IPC_NAME_SIZE + 16]; /* 16 = MTL_MMB_NAME_LEN - LXC_IPC_NAME_SIZE */

	copy_from_user(&user, (struct lxc_ipc_user *)arg, sizeof(user));

	switch (cmd) {
	case LXC_IPC_ATTACH:
		mutex_lock(&lxc_ipc_dev->mutex);

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				found = true;
				break;
			}
		}

		if (!found) {
			name = kzalloc(sizeof(struct lxc_ipc_name), GFP_KERNEL);
			strncpy(name->name, user.name, sizeof(name->name));
			init_completion(&name->attach);
			INIT_LIST_HEAD(&name->list_thread);
			name->name_cnt++;
			list_add_tail(&name->list, &lxc_ipc_dev->list_name);
		} else {
			name->name_cnt++;
		}

		if (user.tid == 1) {
			/* client attach */
			user.tid = current->pid;
			found = false;
			list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
				if (thread->tid == user.tid) {
					found = true;
					break;
				}
			}

			if (!found) {
				thread = kzalloc(sizeof(struct lxc_ipc_thread), GFP_KERNEL);
				thread->tid = user.tid;
				thread->tid_cnt++;
				list_add_tail(&thread->list, &name->list_thread);
			} else {
				thread->tid_cnt++;
			}

			found = false;
			for (i = 0; i < LXC_IPC_BUFFERS_PER_THREAD; i++) {
				if (thread->buffers[i] == NULL) {
					found = true;
					thread->buffers[i] = kzalloc(sizeof(struct lxc_ipc_buffer), GFP_KERNEL);
					buf = thread->buffers[i];
					user.buf_id = i;
					buf->buf_cnt++;
					init_completion(&buf->done);
					init_completion(&buf->call);
					break;
				}
			}

			if (!found) {
				printk("\33[44;31m only allow open %d times in same thread with same name: %s, tid: %d \33[0m \n",
					LXC_IPC_BUFFERS_PER_THREAD, name->name, thread->tid);
				ret = -ENOMEM;
				goto err0;
			}

			complete(&name->attach);
		} else {
			user.buf_id = -1;		/* server attach */
		}

		mutex_unlock(&lxc_ipc_dev->mutex);

		break;

	case LXC_IPC_DETACH:
		mutex_lock(&lxc_ipc_dev->mutex);

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				found = true;
				break;
			}
		}
		if (!found) {
			ret = -ENODEV;
			goto err0;
		}

		found = false;
		list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
			if (thread->tid == user.tid) {
				found = true;
				break;
			}
		}
		if (!found && user.tid != 0) {
			ret = -ENODEV;
			goto err0;
		}

		if (user.tid != 0 && user.buf_id != -1) {
			/* client and accepted detach should buf_cnt-- and tid_cnt-- */
			buf = thread->buffers[user.buf_id];
			buf->connected = -1;
			if (--buf->buf_cnt <= 0) {
				if (buf->in_buf_need) {
					mt_drv_mmz_release(&buf->in_sMBuf);
				}
				if (buf->out_buf_need) {
					mt_drv_mmz_release(&buf->out_sMBuf);
				}
				kfree(thread->buffers[user.buf_id]);
				thread->buffers[user.buf_id] = NULL;
			}

			if (--thread->tid_cnt <= 0) {
				list_del(&thread->list);
				kfree(thread);
			}
		}

		if (--name->name_cnt <= 0) {
			list_del(&name->list);
			kfree(name);
		}

		mutex_unlock(&lxc_ipc_dev->mutex);

		break;

	case LXC_IPC_RESIZE:
		mutex_lock(&lxc_ipc_dev->mutex);

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
					if ((thread->tid == user.tid)) {
						if (thread->buffers[user.buf_id] != NULL) {
							buf = thread->buffers[user.buf_id];
							found = true;
						}
						goto found_buffer_resize;
					}
				}
			}
		}
found_buffer_resize:

		if (!found) {
			ret = -ENODEV;
			goto err0;
		}

		size_tmp = user.in_size;
		if (size_tmp == 0) {
			buf_need = false;
			if (buf->in_buf_need && !lxc_ipc_lazy_release(buf, &buf->in_sMBuf)) {
                ret = -ENOMEM;
                goto err0;
			}
		} else {
			buf_need = true;
			size_tmp = (size_tmp / PAGE_SIZE) + ((size_tmp % PAGE_SIZE) ? 1 : 0);
			for (i = 0; ; i++) {
				if ((1 << i) >= size_tmp) {
					break;
				}
			}
			order = i;
			size_tmp = (size_t)((1 << order) << PAGE_SHIFT);

			if (buf->in_sMBuf.size != size_tmp) {
                if (buf->in_buf_need && !lxc_ipc_lazy_release(buf, &buf->in_sMBuf)) {
                    ret = -ENOMEM;
                    goto err0;
                }

				snprintf(mmz_name, sizeof(mmz_name), "%s_%d_%d_i", name->name, thread->tid, user.buf_id);
				mt_drv_mmz_alloc(mmz_name, MMZ_OTHERS, size_tmp, (int)user.in_align, &buf->in_sMBuf);
			}
		}
		buf->in_buf_need = buf_need;

		size_tmp = user.out_size;
		if (size_tmp == 0) {
			buf_need = false;
			if (buf->out_buf_need && !lxc_ipc_lazy_release(buf, &buf->out_sMBuf)) {
                ret = -ENOMEM;
                goto err0;
			}
		} else {
			buf_need = true;
			size_tmp = (size_tmp / PAGE_SIZE) + ((size_tmp % PAGE_SIZE) ? 1 : 0);
			for (i = 0; ; i++) {
				if ((1 << i) >= size_tmp) {
					break;
				}
			}
			order = i;
			size_tmp = (size_t)((1 << order) << PAGE_SHIFT);

			if (buf->out_sMBuf.size != size_tmp) {
                if (buf->out_buf_need && !lxc_ipc_lazy_release(buf, &buf->out_sMBuf)) {
                    ret = -ENOMEM;
                    goto err0;
                }

				snprintf(mmz_name, sizeof(mmz_name), "%s_%d_%d_o", name->name, thread->tid, user.buf_id);
				mt_drv_mmz_alloc(mmz_name, MMZ_OTHERS, size_tmp, (int)user.out_align, &buf->out_sMBuf);
			}
		}
		buf->out_buf_need = buf_need;

		if (buf->in_buf_need) {
			user.in_size = (ulong)buf->in_sMBuf.size;
			user.in_phys_addr = (ulong)buf->in_sMBuf.startPhyAddr;
		} else {
			user.in_size = 0;
			user.in_phys_addr = 0;
		}
		if (buf->out_buf_need) {
			user.out_size = (ulong)buf->out_sMBuf.size;
			user.out_phys_addr = (ulong)buf->out_sMBuf.startPhyAddr;
		} else {
			user.out_size = 0;
			user.out_phys_addr = 0;
		}

		mutex_unlock(&lxc_ipc_dev->mutex);

		break;

	case LXC_IPC_CALL:
		mutex_lock(&lxc_ipc_dev->mutex);

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
					if (thread->tid == user.tid) {
						if (thread->buffers[user.buf_id] != NULL) {
							buf = thread->buffers[user.buf_id];
							found = true;
						}
						goto found_buffer_call;
					}
				}
			}
		}
found_buffer_call:

		if (!found) {
			ret = -ENODEV;
			goto err0;
		}

		buf->do_what = user.do_what;

		complete(&buf->call);

		mutex_unlock(&lxc_ipc_dev->mutex);

		if (user.timeout == 0) {
			if (wait_for_completion_interruptible(&buf->done)) {
				ret = -EINTR;
				goto err1;
			}
		} else {
			timeout = (user.timeout * HZ / 1000) < 1 ? 1 : (user.timeout * HZ / 1000);
			ret_wait = wait_for_completion_interruptible_timeout(&buf->done, timeout);
			if (ret_wait == 0) {
				printk("\33[44;31m timeout at name: %s, do_what: %d \33[0m \n", name->name, buf->do_what);
				ret = -ETIME;
				goto err1;
			} else if (ret_wait < 0) {
				ret = -EINTR;
				goto err1;
			}
		}

		break;

	case LXC_IPC_LISTEN:
		mutex_lock(&lxc_ipc_dev->mutex);

		if (user.tid != 0) {
			ret = -EINVAL;
			goto err0;
		}

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				found = true;
				break;
			}
		}

		if (!found) {
			ret = -ENODEV;
			goto err0;
		}

		mutex_unlock(&lxc_ipc_dev->mutex);

		if (wait_for_completion_interruptible(&name->attach)) {
			ret = -EINTR;
			goto err1;
		}

		mutex_lock(&lxc_ipc_dev->mutex);
		found = false;
		list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
			for (i = 0; i < LXC_IPC_BUFFERS_PER_THREAD; i++) {
				if (thread->buffers[i] != NULL) {
					buf = thread->buffers[i];
					if (buf->connected == 0) {
						buf->connected = 1;
						found = true;
						goto found_name_listen;
					}
				}
			}
		}
found_name_listen:

		if (!found)	{
			ret = -ENODEV;
			goto err0;
		}

		user.do_what = -1;
		user.tid = thread->tid;
		user.buf_id = i;
		buf->buf_cnt++;
		thread->tid_cnt++;
		name->name_cnt++;

		mutex_unlock(&lxc_ipc_dev->mutex);

		break;

	case LXC_IPC_ACCEPT:
		mutex_lock(&lxc_ipc_dev->mutex);

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
					if (thread->tid == user.tid) {
						if (thread->buffers[user.buf_id] != NULL) {
							buf = thread->buffers[user.buf_id];
							found = true;
						}
						goto found_buffer_accept;
					}
				}
			}
		}
found_buffer_accept:

		if (!found) {
			ret = -ENODEV;
			goto err0;
		}

		mutex_unlock(&lxc_ipc_dev->mutex);

		if (wait_for_completion_interruptible(&buf->call)) {
			ret = -EINTR;
			goto err1;
		}

		mutex_lock(&lxc_ipc_dev->mutex);

		if (buf->in_buf_need) {
			user.in_size = (ulong)buf->in_sMBuf.size;
			user.in_phys_addr = (ulong)buf->in_sMBuf.startPhyAddr;
		} else {
			user.in_size = 0;
			user.in_phys_addr = 0;
		}
		if (buf->out_buf_need) {
			user.out_size = (ulong)buf->out_sMBuf.size;
			user.out_phys_addr = (ulong)buf->out_sMBuf.startPhyAddr;
		} else {
			user.out_size = 0;
			user.out_phys_addr = 0;
		}

		user.do_what = buf->do_what;

		mutex_unlock(&lxc_ipc_dev->mutex);

		break;

	case LXC_IPC_DONE:
		mutex_lock(&lxc_ipc_dev->mutex);

		list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
			if (!strncmp(name->name, user.name, sizeof(name->name))) {
				list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
					if (thread->tid == user.tid) {
						if (thread->buffers[user.buf_id] != NULL) {
							buf = thread->buffers[user.buf_id];
							found = true;
						}
						goto found_buffer_done;
					}
				}
			}
		}
found_buffer_done:

		if (!found) {
			ret = -ENODEV;
			goto err0;
		}

        for(i = 0; buf->release_mem[i].size; i++) {
            mt_drv_mmz_release(&buf->release_mem[i]);
        }

		complete(&buf->done);

		mutex_unlock(&lxc_ipc_dev->mutex);

		break;

	default:
		ret = -EINVAL;
		goto err1;
	}

	copy_to_user((struct lxc_ipc_user *)arg, &user, sizeof(user));

	return 0;

err0:
	mutex_unlock(&lxc_ipc_dev->mutex);
err1:

	return ret;
}

static struct file_operations lxc_ipc_fops = {
	.owner = THIS_MODULE,
	.llseek = no_llseek,
	.unlocked_ioctl = lxc_ipc_ioctl,
	.open = lxc_ipc_open,
	.release = lxc_ipc_release,
};

static ssize_t lxc_ipc_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
	ssize_t count = 0;
	struct lxc_ipc_device *lxc_ipc_dev = dev_get_drvdata(dev);
	struct lxc_ipc_name *name;
	struct lxc_ipc_name *name_safe;
	struct lxc_ipc_thread *thread;
	struct lxc_ipc_thread *thread_safe;
	struct lxc_ipc_buffer *buf;
	int i;

	mutex_lock(&lxc_ipc_dev->mutex);
	count += snprintf(buffer + count, PAGE_SIZE - count, "open_cnt: %d\n", lxc_ipc_dev->open_cnt);
	list_for_each_entry_safe(name, name_safe, &lxc_ipc_dev->list_name, list) {
		count += snprintf(buffer + count, PAGE_SIZE - count,
				"	name_cnt: %d, name: %s\n",
				name->name_cnt,
				name->name);
		list_for_each_entry_safe(thread, thread_safe, &name->list_thread, list) {
			count += snprintf(buffer + count, PAGE_SIZE - count,
							"		tid_cnt: %d, tid: %d\n",
							thread->tid_cnt,
							thread->tid);
			for (i = 0; i < LXC_IPC_BUFFERS_PER_THREAD; i++) {
				if (thread->buffers[i] != NULL) {
					buf = thread->buffers[i];
					count += snprintf(buffer + count, PAGE_SIZE - count,
									"			buf_cnt: %d, in_size = %ld, out_size = %ld, buf_id: %d\n",
									buf->buf_cnt,
									buf->in_sMBuf.size, buf->out_sMBuf.size,
									i);
				}
			}
		}
	}
	mutex_unlock(&lxc_ipc_dev->mutex);

	return count;
}
static DEVICE_ATTR_RO(lxc_ipc);

static int lxc_ipc_probe(struct platform_device *pdev)
{
	struct lxc_ipc_device *lxc_ipc_dev = NULL;
	int ret = 0;
	int idx;

	for (idx = 0; idx < lxc_ipc_drv->minors; idx++) {
		if (!lxc_ipc_drv->lxc_ipc_dev[idx]) {
			break;
		}
	}
	if (idx >= lxc_ipc_drv->minors) {
		ret = -ENODEV;
		goto fail_alloc_minor;
	}

	lxc_ipc_dev = kzalloc(sizeof(struct lxc_ipc_device), GFP_KERNEL);
	if (!lxc_ipc_dev) {
		ret = -ENOMEM;
		goto fail_kzalloc_lxc_ipc_dev;
	}
	lxc_ipc_dev->devt = MKDEV(MAJOR(lxc_ipc_drv->devt), lxc_ipc_drv->minor + idx);
	lxc_ipc_drv->lxc_ipc_dev[idx] = lxc_ipc_dev;

	INIT_LIST_HEAD(&lxc_ipc_dev->list_name);
	mutex_init(&lxc_ipc_dev->mutex);

	lxc_ipc_dev->dev = device_create(lxc_ipc_class, NULL, lxc_ipc_dev->devt, NULL, LXC_IPC_NAME);
	if (IS_ERR(lxc_ipc_dev->dev)) {
		ret = PTR_ERR(lxc_ipc_dev->dev);
		goto fail_device_create;
	}

	if (device_create_file(&pdev->dev, &dev_attr_lxc_ipc)) {
		ret = -ENOENT;
		goto fail_device_create_file;
	}

	platform_set_drvdata(pdev, lxc_ipc_dev);
	dev_set_drvdata(lxc_ipc_dev->dev, lxc_ipc_dev);

	return 0;

fail_device_create_file:
	device_destroy(lxc_ipc_class, lxc_ipc_dev->devt);
	lxc_ipc_dev->dev = NULL;
fail_device_create:
	mutex_destroy(&lxc_ipc_dev->mutex);
	lxc_ipc_drv->lxc_ipc_dev[idx] = NULL;
	kfree(lxc_ipc_dev);
	lxc_ipc_dev = NULL;
fail_kzalloc_lxc_ipc_dev:
fail_alloc_minor:
	return ret;
}

static int lxc_ipc_remove(struct platform_device *pdev)
{
	int idx;
	struct lxc_ipc_device *lxc_ipc_dev = platform_get_drvdata(pdev);

	device_remove_file(&pdev->dev, &dev_attr_lxc_ipc);
	device_destroy(lxc_ipc_class, lxc_ipc_dev->devt);
	idx = lxc_ipc_dev->devt - lxc_ipc_drv->devt;
	lxc_ipc_drv->lxc_ipc_dev[idx] = NULL;
	lxc_ipc_dev->dev = NULL;

	mutex_destroy(&lxc_ipc_dev->mutex);

	platform_set_drvdata(pdev, NULL);

	if (lxc_ipc_dev) {
		kfree(lxc_ipc_dev);
		lxc_ipc_dev = NULL;
	}

	return 0;
}

static int lxc_ipc_suspend(struct platform_device *pdev, pm_message_t stState)
{
	//struct lxc_ipc_device *lxc_ipc_dev = platform_get_drvname(pdev);
	return 0;
}

static int lxc_ipc_resume(struct platform_device *pdev)
{
	//struct lxc_ipc_device *lxc_ipc_dev = platform_get_drvname(pdev);
	return 0;
}

#if defined(CONFIG_OF)
static const struct of_device_id lxc_ipc_of_match[] = {
	{ .compatible = "mt,lxc_ipc" },
	{},
};
MODULE_DEVICE_TABLE(of, lxc_ipc_of_match);
#endif

static struct platform_driver lxc_ipc_platform_driver = {
	.probe = lxc_ipc_probe,
	.remove = lxc_ipc_remove,
	.suspend = lxc_ipc_suspend,
	.resume = lxc_ipc_resume,
	.driver = {
		.name = LXC_IPC_NAME,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(lxc_ipc_of_match),
	}
};

static void lxc_ipc_release_device(struct device *pdev)
{
	/* if platform_device is dynamic alloc, we can free platform_device here. */
}

static struct platform_device lxc_ipc_platform_device = {
	.name = LXC_IPC_NAME,
	.id = PLATFORM_DEVID_NONE,
	.dev = {
		.release = lxc_ipc_release_device,
	},
};

int __init lxc_ipc_drv_modinit(void)
{
	int ret;

	lxc_ipc_drv = kzalloc(sizeof(struct lxc_ipc_driver), GFP_KERNEL);
	if (!lxc_ipc_drv) {
		ret = -ENOMEM;
		goto fail_kzalloc_drv;
	}

	lxc_ipc_class = class_create(LXC_IPC_NAME);
	if (IS_ERR(lxc_ipc_class)) {
		ret = PTR_ERR(lxc_ipc_class);
		goto fail_class_create;
	}

	lxc_ipc_drv->minor = LXC_IPC_MINOR;
	lxc_ipc_drv->minors = LXC_IPC_MINORS;
	lxc_ipc_drv->devt = MKDEV(MT_DEVICE_MAJOR, lxc_ipc_drv->minor);
	lxc_ipc_drv->major = MAJOR(lxc_ipc_drv->devt);

	cdev_init(&lxc_ipc_drv->cdev, &lxc_ipc_fops);
	lxc_ipc_drv->cdev.owner = THIS_MODULE;

	ret = cdev_add(&lxc_ipc_drv->cdev, lxc_ipc_drv->devt, lxc_ipc_drv->minors);
	if (ret) {
		ret = -EINVAL;
		goto fail_cdev_add;
	}

	ret = platform_driver_register(&lxc_ipc_platform_driver);
	if (ret)
		goto fail_platform_driver_register;

	ret = platform_device_register(&lxc_ipc_platform_device);
	if (ret)
		goto fail_platform_device_register;

	return 0;

fail_platform_device_register:
	platform_driver_unregister(&lxc_ipc_platform_driver);
fail_platform_driver_register:
	cdev_del(&lxc_ipc_drv->cdev);
fail_cdev_add:
	unregister_chrdev_region(lxc_ipc_drv->devt, lxc_ipc_drv->minors);
	class_destroy(lxc_ipc_class);
	lxc_ipc_class = NULL;
fail_class_create:
	kfree(lxc_ipc_drv);
	lxc_ipc_drv = NULL;
fail_kzalloc_drv:
	return ret;
}

static void __exit lxc_ipc_drv_modexit(void)
{
	cdev_del(&lxc_ipc_drv->cdev);
	unregister_chrdev_region(lxc_ipc_drv->devt, lxc_ipc_drv->minors);

	platform_driver_unregister(&lxc_ipc_platform_driver);
	platform_device_unregister(&lxc_ipc_platform_device);

	class_destroy(lxc_ipc_class);
	lxc_ipc_class = NULL;

	kfree(lxc_ipc_drv);
	lxc_ipc_drv = NULL;
}

