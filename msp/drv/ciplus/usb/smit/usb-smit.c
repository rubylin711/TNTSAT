/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2020 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/usb.h>
#include <linux/mutex.h>


/* Define these values to match your devices */
#define USB_SMIT_VENDOR_ID   0x29df
#define USB_ICAST_PRODUCT_ID	0x0100
#define USB_CARDLESS_PRODUCT_ID		0x1002


//#define USB_DEBUG_PRINT
#ifdef USB_DEBUG_PRINT
#define DBGPRINT(x,...) printk(x, ##__VA_ARGS__);
#else
#define DBGPRINT(x,...)
#endif

/* table of devices that work with this driver */
static const struct usb_device_id smit_dev_table[] = {
   { USB_DEVICE(USB_SMIT_VENDOR_ID, USB_ICAST_PRODUCT_ID) },   	
   { USB_DEVICE(USB_SMIT_VENDOR_ID, USB_CARDLESS_PRODUCT_ID) },
   { }               /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, smit_dev_table);

typedef enum {
	EP_TYPE_BULK = 0x01,
	EP_TYPE_INT	 = 0x02,
} ENDPOINT_TYPE;
/* Get a minor range for your devices from the usb maintainer */
#define USB_CIP_MINOR_BASE   192

/* our private defines. if this grows any larger, use your own .h file */
#define MAX_TRANSFER    (188*1024)

#define MAX_MEDIA_RXTX_TRANSFER	(188*1024)
#define MAX_CMD_RXTX_TRANSFER	(4096)
/* MAX_TRANSFER is chosen so that the VM is not stressed by
   allocations > PAGE_SIZE and the number of packets in a page
   is an integer 512 is the largest possible packet on EHCI */
#define WRITES_IN_FLIGHT   8
/* arbitrarily chosen */

/* Structure to hold all of our device specific stuff */
struct usb_ciplus {
   struct usb_device *udev;         /* the usb device for this device */
   struct usb_interface *interface;    /* the interface for this device */
   struct semaphore  limit_sem;     /* limiting the number of writes in progress */
   struct usb_anchor submitted;     /* in case we need to retract our submissions */
	struct urb *rx_urb;
	__u8 int_in_ep;
	__u8 int_out_ep;
	__u8 intf_protocol;
	char *rx_buf;
	char *tx_buf;
	size_t tx_size;
	size_t rx_size;
	size_t rx_filled;  
	size_t rx_copied;

	__u8 rx_ep_addr;
	__u8 tx_ep_addr;

	__u8 rx_ep_type;
	__u8 tx_ep_type;
   
   int         errors;        /* the last request tanked */
   bool        ongoing_read;     /* a read is going on */
   spinlock_t     err_lock;      /* lock for errors */
   struct kref    kref;
   struct mutex      io_mutex;      /* synchronize I/O with disconnect */
   unsigned long     disconnected:1;
   wait_queue_head_t rx_wait;     /* to wait for an ongoing read */
   dma_addr_t data_dma;
};
#define to_ciplus_dev(d) container_of(d, struct usb_ciplus, kref)

static struct usb_driver ciplus_driver;
static void ciplus_draw_down(struct usb_ciplus *dev);


static void ciplus_delete(struct kref *kref)
{
   struct usb_ciplus *dev = to_ciplus_dev(kref);
   usb_free_urb(dev->rx_urb);
   usb_put_intf(dev->interface);
   usb_put_dev(dev->udev);
   kfree(dev->rx_buf);
   kfree(dev);
}


static int ciplus_open(struct inode *inode, struct file *file)
{
   struct usb_ciplus *dev;
   struct usb_interface *interface;
   int subminor;
   int retval = 0;

   subminor = iminor(inode);
   interface = usb_find_interface(&ciplus_driver, subminor);
   if (!interface) {
      pr_err("%s - error, can't find device for minor %d\n",
         __func__, subminor);
      retval = -ENODEV;
      goto exit;
   }

   dev = usb_get_intfdata(interface);
   if (!dev) {
      retval = -ENODEV;
      goto exit;
   }

   retval = usb_autopm_get_interface(interface);
   if (retval)
      goto exit;

   /* increment our usage count for the device */
   kref_get(&dev->kref);
   /* save our object in the file's private structure */
   file->private_data = dev;

exit:
   return retval;
}

static int ciplus_release(struct inode *inode, struct file *file)
{
   struct usb_ciplus *dev;

   dev = file->private_data;
   if (dev == NULL)
      return -ENODEV;

   /* allow the device to be autosuspended */
   usb_autopm_put_interface(dev->interface);

   /* decrement the count on our device */
   kref_put(&dev->kref, ciplus_delete);
   return 0;
}

static int ciplus_flush(struct file *file, fl_owner_t id)
{
   struct usb_ciplus *dev;
   int res;

   dev = file->private_data;
   if (dev == NULL)
      return -ENODEV;

   /* wait for io to stop */
   mutex_lock(&dev->io_mutex);
   ciplus_draw_down(dev);

   /* read out errors, leave subsequent opens a clean slate */
   spin_lock_irq(&dev->err_lock);
   res = dev->errors ? (dev->errors == -EPIPE ? -EPIPE : -EIO) : 0;
   dev->errors = 0;
   spin_unlock_irq(&dev->err_lock);

   mutex_unlock(&dev->io_mutex);

   return res;
}

static void ciplus_read_callback(struct urb *urb)
{
   struct usb_ciplus *dev;
   unsigned long flags;

   dev = urb->context;
   spin_lock_irqsave(&dev->err_lock, flags);
   /* sync/async unlink faults aren't errors */
   if (urb->status) {
      if (!(urb->status == -ENOENT || urb->status == -ECONNRESET ||urb->status == -ESHUTDOWN)) {
		dev_err(&dev->interface->dev,"%s - read bulk status: %d\n",__func__, urb->status);
		dev_err(&dev->interface->dev, "actual len: %u, Err cnt=%u\n", urb->actual_length, urb->error_count);
      }

      dev->errors = urb->status;
   } else {
      dev->rx_filled= urb->actual_length;
   }
   
   dev->ongoing_read = 0;
 	//  printk("ongoing_read=0----\n");
   spin_unlock_irqrestore(&dev->err_lock, flags);
 
   wake_up_interruptible(&dev->rx_wait);
}

static int ciplus_do_read_io(struct usb_ciplus *dev)
{
   int retval = 0;
   /* prepare a read */
   if(dev->rx_ep_type== EP_TYPE_BULK) {
   		printk("bulk transfer\n");
   		usb_fill_bulk_urb(dev->rx_urb,dev->udev,
			usb_rcvbulkpipe(dev->udev,dev->rx_ep_addr),
         	dev->rx_buf,dev->rx_size,ciplus_read_callback,dev);
		
   	} else if(dev->rx_ep_type == EP_TYPE_INT) {
		printk("int transfer\n");
		usb_fill_int_urb(dev->rx_urb, dev->udev,
			usb_rcvintpipe(dev->udev, dev->rx_ep_addr),
			dev->rx_buf, dev->rx_size,ciplus_read_callback, dev, 1);
		
   	} else {
   		dev_err(&dev->interface->dev,"unsuport endpoint type:%d\n",dev->rx_ep_type);
   	}
   
   /* tell everybody to leave the URB alone */
   spin_lock_irq(&dev->err_lock);
   dev->ongoing_read = 1;
   spin_unlock_irq(&dev->err_lock);

   /* submit bulk in urb, which means no data to deliver */
   dev->rx_filled= 0;
   dev->rx_copied= 0;

   /* submit urb */
   retval = usb_submit_urb(dev->rx_urb, GFP_KERNEL);
   if (retval < 0) {
      dev_err(&dev->interface->dev,"%s - failed submitting read urb, error %d\n",__func__, retval);
      retval = (retval == -ENOMEM) ? retval : -EIO;
      spin_lock_irq(&dev->err_lock);
      dev->ongoing_read = 0;
      spin_unlock_irq(&dev->err_lock);
   }
   
   return retval;
}

#ifdef USB_DEBUG_PRINT
/**
 * @brief     Print hexadecimal dump of data buffer
 * @param     data Pointer to data to hex dump
 * @param     size Size of data in bytes
 */
static void ciplus_hex_dump(unsigned char  *data, size_t size)
{
   const char digits[16] = "0123456789ABCDEF";
   char buff[52];
   size_t dlen, blen;

   for( dlen = 0, blen = 0; dlen != size; dlen++, data++ )
   {
      buff[blen++] = digits[*data >> 4];
      buff[blen++] = digits[*data & 0xf];
      switch ( dlen & 0xf )
      {
         case 0xf:
         {
            buff[blen] = '\0';
            DBGPRINT("%s\n", buff)
            blen = 0;
            break;
         }
         case 0x7:
            buff[blen++] = '\t';
            break;
         default:
            buff[blen++] = ' ';
            break;
      }
   }
   buff[blen] = '\0';
   DBGPRINT("%s\n", buff)
}
#endif

static ssize_t ciplus_read(struct file *file, char *usr_buff, size_t count, loff_t *ppos)
{
   int retval;
   struct usb_ciplus *dev;
   size_t available, read_size;
   bool ongoing_io;

   dev = file->private_data;
   //DBGPRINT("USB ciplus READ %lu bytes, from %p", count, dev->bulk_in_urb)
   
   /* if we cannot read at all, return EOF */
   if (!count)
      return 0;

   /* no concurrent readers */
   retval = mutex_lock_interruptible(&dev->io_mutex);
   if (retval < 0)
      return retval;

   if (dev->disconnected) {      /* disconnect() was called */
      retval = -ENODEV;
      goto exit;
   }
   /* if IO is under way, we must not touch things */
retry:
   spin_lock_irq(&dev->err_lock);
   ongoing_io = dev->ongoing_read;
   spin_unlock_irq(&dev->err_lock);

   if (ongoing_io) {
      /* nonblocking return directly */
      if (file->f_flags & O_NONBLOCK) {
         retval = -EAGAIN;
         goto exit;
      }
      /*
       * IO may take forever
       * hence wait in an interruptible state
       */
      retval = wait_event_interruptible(dev->rx_wait, (!dev->ongoing_read));
      if (retval < 0) {
         DBGPRINT("USB ciplus READ 1 rv=%x", -retval);
         goto exit;
      }
   } 

   /* errors must be reported */
   retval = dev->errors;
   if (retval < 0) {
#ifdef USB_DEBUG_PRINT
      DBGPRINT("USB ciplus READ Error %d\n", retval);
      ciplus_hex_dump(dev->rx_buf, 8);
#endif

      /* any error is reported once */
      dev->errors = 0;
      /* to preserve notifications about reset */
      retval = (retval == -EPIPE) ? retval : -EIO;
      /* report it */
      goto exit;
   }

   available = dev->rx_filled - dev->rx_copied;
   if (!available) {
      /* no available data in the buffer - we need to start IO*/
      retval = ciplus_do_read_io(dev);
      if (retval < 0) {
         DBGPRINT("USB ciplus READ 3 rv=%x", -retval);
         goto exit;
      } else {
         goto retry;
      }
   }
   
   read_size = min(available, count);
   /* read_size tells us how much shall be copied */
   if (copy_to_user(usr_buff, dev->rx_buf+ dev->rx_copied, read_size)) {
      retval = -EFAULT;
   } else {
      retval = read_size;
   }
   dev->rx_copied += read_size;
   count -= read_size;

   /*
    * if user are asked for more than we have, we start IO but don't wait
    * return to usr space deal with
    */
   if (available < count) {
      ciplus_do_read_io(dev);
   }

exit:
   mutex_unlock(&dev->io_mutex);
   return retval;   
}



static void ciplus_write_callback(struct urb *urb)
{
   struct usb_ciplus *dev;
   unsigned long flags;
   dev = urb->context;

   /* sync/async unlink faults aren't errors */
	if (urb->status) {
		if (!(urb->status == -ENOENT ||
			urb->status == -ECONNRESET ||
			urb->status == -ESHUTDOWN)) {
			
			dev_err(&dev->interface->dev,
				"%s - nonzero write bulk status received: %d\n",
				__func__, urb->status);
		}

		spin_lock_irqsave(&dev->err_lock, flags);
		dev->errors = urb->status;
		spin_unlock_irqrestore(&dev->err_lock, flags);
	}
	
   up(&dev->limit_sem);
}


static ssize_t ciplus_write(struct file *file, const char *user_buffer,
           size_t count, loff_t *ppos)
{
   struct usb_ciplus *dev;
   int retval = 0;
   struct urb *urb = NULL;
   size_t writesize;

   dev = file->private_data;
   if (!dev)
	return -ENODEV;

   /*
	* limit the number of URBs in flight to stop a user from using up all
	* RAM
	*/
   if (!(file->f_flags & O_NONBLOCK)) {
	  if (down_interruptible(&dev->limit_sem)) {
		 retval = -ERESTARTSYS;
		 goto exit;
	  }
   } else {
	  if (down_trylock(&dev->limit_sem)) {		
		 retval = -EAGAIN;
		 goto exit;
	  }
   }
   
   spin_lock_irq(&dev->err_lock);
   retval = dev->errors;
   if (retval < 0) {
	  /* any error is reported once */
	  dev->errors = 0;
	  /* to preserve notifications about reset */
	  retval = (retval == -EPIPE) ? retval : -EIO;
   }
   spin_unlock_irq(&dev->err_lock);
   if (retval < 0){
   		  goto error;
   }

   writesize = min(count,dev->tx_size);

   /* create a urb, and a buffer for it, and copy the data to the urb */
   urb = usb_alloc_urb(0, GFP_KERNEL);
   if (!urb) {
	  retval = -ENOMEM;
	  goto error;
   }

   /* this lock makes sure we don't submit URBs to gone devices */
   mutex_lock(&dev->io_mutex);
   if (dev->disconnected) { 	 /* disconnect() was called */
	  mutex_unlock(&dev->io_mutex);
	  retval = -ENODEV;
	  goto error;
   }

   if (copy_from_user(dev->tx_buf, user_buffer, writesize)) {
		 retval = -EFAULT;
		 mutex_unlock(&dev->io_mutex);
		 goto error;
	  }
   
   /* initialize the urb properly */
	if(dev->tx_ep_type==EP_TYPE_BULK) {
		usb_fill_bulk_urb(urb, dev->udev,
			usb_sndbulkpipe(dev->udev, dev->tx_ep_addr),
			dev->tx_buf, writesize, ciplus_write_callback, dev);
		urb->transfer_dma = dev->data_dma;
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		if (writesize == 0) {
			urb->transfer_flags |= URB_ZERO_PACKET;
		}
		
	} else if(dev->tx_ep_type==EP_TYPE_INT) {
		usb_fill_int_urb(urb, dev->udev,
			usb_sndintpipe(dev->udev, dev->tx_ep_addr),
			dev->tx_buf, writesize, ciplus_write_callback,dev, 1);
		urb->transfer_dma = dev->data_dma;
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
		if (writesize == 0) {
			urb->transfer_flags |= URB_ZERO_PACKET;
		}
		
	} else {
		dev_err(&dev->interface->dev,"unspport tx endpoint type");
	}

   usb_anchor_urb(urb, &dev->submitted);

   /* send the data out the bulk port */
   retval = usb_submit_urb(urb, GFP_KERNEL);
   mutex_unlock(&dev->io_mutex);
   if (retval) {
	  dev_err(&dev->interface->dev,
	  	"%s - failed submitting write urb, error %d\n",
	  	__func__, retval);
	  goto error_unanchor;
   }

   /*
	* release our reference to this urb, the USB core will eventually free
	* it entirely
	*/
   usb_free_urb(urb);

 //  printk("USB ciplus written %lu bytes", writesize);

   return writesize;

error_unanchor:
   usb_unanchor_urb(urb);
error:
   if (urb) {
	  /*usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);*/
	  usb_free_urb(urb);
   }
   up(&dev->limit_sem);
exit:
   return retval;
}


static char *cip_devnode(const struct device *dev, umode_t *mode)
{
	if (mode != NULL)
		*mode = 0666;
	if (dev == NULL)
		return NULL;
	
	return kasprintf(GFP_KERNEL, "%s", dev_name(dev));
}

static const struct file_operations ciplus_fops = {
   .owner = THIS_MODULE,
   .read =     ciplus_read,
   .write = ciplus_write,
   .open =     ciplus_open,
   .release =  ciplus_release,
   .flush = ciplus_flush,
   .llseek =   noop_llseek,
};

/*
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with the driver core
 */
static struct usb_class_driver ciplus_class = {
   .name =     "ciplus%d",
   .devnode = cip_devnode,
   .fops =     &ciplus_fops,
   .minor_base =  USB_CIP_MINOR_BASE,
};


static int ciplus_probe(struct usb_interface *interface,
            const struct usb_device_id *id)
{
   struct usb_ciplus *dev;
   struct usb_host_interface *cur_seting;
   struct usb_interface_descriptor interface_desc;
   struct usb_endpoint_descriptor *endpoint_desc;
   int index;
   int retval;

   /* allocate memory for our device state and initialize it */
   dev = kzalloc(sizeof(*dev), GFP_KERNEL);
   if (!dev)
      return -ENOMEM;

   kref_init(&dev->kref);
   sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
   mutex_init(&dev->io_mutex);
   spin_lock_init(&dev->err_lock);
   init_usb_anchor(&dev->submitted);
   init_waitqueue_head(&dev->rx_wait);

   dev->udev = usb_get_dev(interface_to_usbdev(interface));
   dev->interface = usb_get_intf(interface);

   /* set up the endpoint information */
   cur_seting = interface->cur_altsetting;
   interface_desc = cur_seting->desc;
   printk(KERN_INFO "bNumEndpoints=%u bInterfaceClass=%x bInterfaceSubClass=%x bInterfaceProtocol=%x if=%u\n", interface_desc.bNumEndpoints,
      interface_desc.bInterfaceClass, interface_desc.bInterfaceSubClass, interface_desc.bInterfaceProtocol, interface_desc.iInterface);

	for(index = 0; index != interface_desc.bNumEndpoints; index++){
		endpoint_desc = &cur_seting->endpoint[index].desc;
		if(usb_endpoint_is_int_in(endpoint_desc)) {
			dev->rx_ep_addr= endpoint_desc->bEndpointAddress;
			dev->rx_ep_type = EP_TYPE_INT;

		} else if(usb_endpoint_is_int_out(endpoint_desc)) {
			dev->tx_ep_addr = endpoint_desc->bEndpointAddress;
			dev->tx_ep_type = EP_TYPE_INT;

		} else if(usb_endpoint_is_bulk_in(endpoint_desc)) {
			dev->rx_ep_addr= endpoint_desc->bEndpointAddress;
			dev->rx_ep_type = EP_TYPE_BULK;

		} else if(usb_endpoint_is_bulk_out(endpoint_desc)) {
			dev->tx_ep_addr = endpoint_desc->bEndpointAddress;
			dev->tx_ep_type = EP_TYPE_BULK;
			
		} else {
			printk(KERN_WARNING "no ciplus dev\n");
			retval = -ENODEV;
			goto error0;
		}
	}
	
	if(interface_desc.bInterfaceProtocol==0x01) {
		dev->intf_protocol = 0x01;
		dev->rx_size = MAX_CMD_RXTX_TRANSFER;	   
		dev->tx_size = MAX_CMD_RXTX_TRANSFER;

	} else if(interface_desc.bInterfaceProtocol==0x02) {
		dev->intf_protocol = 0x02;
		dev->rx_size = MAX_MEDIA_RXTX_TRANSFER;		  
		dev->tx_size = MAX_MEDIA_RXTX_TRANSFER;

	} else {
		retval = -ENODEV;
		goto error0;
	}

	dev->rx_urb= usb_alloc_urb(0,GFP_KERNEL);
	if(!dev->rx_urb) {
		retval = -ENOMEM;
		goto error0;
	}

	dev->rx_buf = kmalloc(dev->rx_size, GFP_KERNEL);
	if (!dev->rx_buf) {
		retval = -ENOMEM;
		goto error0;
	}

	dev->tx_buf = usb_alloc_coherent(dev->udev, 
		dev->tx_size, GFP_KERNEL,&dev->data_dma);
	if (!dev->tx_buf) {
		retval = -ENOMEM;
		goto error0;
	}

   /* save our data pointer(usb_ciplus *dev) in this interface device */
   usb_set_intfdata(interface, dev);

   /* we can register the device interface now, as it is ready */
   retval = usb_register_dev(interface, &ciplus_class);
   if (retval) {
      /* something prevented us from registering this driver */
      dev_err(&interface->dev,
         "Not able to get a minor for this device.\n");
      usb_set_intfdata(interface, NULL);
      goto error0;
   }

   dev_info(&dev->interface->dev,"rx ep, addr:0x%x,type:%d\n",dev->rx_ep_addr,dev->rx_ep_type);   
   dev_info(&dev->interface->dev,"tx ep, addr:0x%x,type:%d\n",dev->tx_ep_addr,dev->tx_ep_type);
   /* let the user know what node this device is now attached to */
   dev_info(&interface->dev,"USB ciplus #%d device now attached",interface->minor);
   return 0;

error0:
   /* this frees allocated memory */
   kref_put(&dev->kref, ciplus_delete);

   return retval;
}

static void ciplus_disconnect(struct usb_interface *interface)
{
   struct usb_ciplus *dev;
   int minor = interface->minor;

   dev = usb_get_intfdata(interface);
   usb_set_intfdata(interface, NULL);

   /* give back our minor */
   usb_deregister_dev(interface, &ciplus_class);

   /* prevent more I/O from starting */
   mutex_lock(&dev->io_mutex);
   dev->disconnected = 1;
   mutex_unlock(&dev->io_mutex);

   usb_kill_anchored_urbs(&dev->submitted);
   usb_free_coherent(dev->udev, dev->tx_size, dev->tx_buf, dev->data_dma);

   /* decrement our usage count */
   kref_put(&dev->kref, ciplus_delete);

   dev_info(&interface->dev, "USB ciplus #%d now disconnected", minor);
}

static void ciplus_draw_down(struct usb_ciplus *dev)
{
   int time;

   time = usb_wait_anchor_empty_timeout(&dev->submitted, 1000);
   if (!time)
      usb_kill_anchored_urbs(&dev->submitted);
   usb_kill_urb(dev->rx_urb);
}

static int ciplus_suspend(struct usb_interface *intf, pm_message_t message)
{
   struct usb_ciplus *dev = usb_get_intfdata(intf);

   if (!dev)
      return 0;
   ciplus_draw_down(dev);
   return 0;
}

static int ciplus_resume(struct usb_interface *intf)
{
   return 0;
}

static int ciplus_pre_reset(struct usb_interface *intf)
{
   struct usb_ciplus *dev = usb_get_intfdata(intf);

   mutex_lock(&dev->io_mutex);
   ciplus_draw_down(dev);

   return 0;
}

static int ciplus_post_reset(struct usb_interface *intf)
{
   struct usb_ciplus *dev = usb_get_intfdata(intf);

   /* we are sure no URBs are active - no locking needed */
   dev->errors = -EPIPE;
   mutex_unlock(&dev->io_mutex);

   return 0;
}

static struct usb_driver ciplus_driver = {
   .name =     "smit",
   .probe = ciplus_probe,
   .disconnect =  ciplus_disconnect,
   .suspend =  ciplus_suspend,
   .resume =   ciplus_resume,
   .pre_reset =   ciplus_pre_reset,
   .post_reset =  ciplus_post_reset,
   .id_table = smit_dev_table,
   .supports_autosuspend = 1,
};

module_usb_driver(ciplus_driver);

MODULE_LICENSE("GPL v2");
