/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h> // file_operations
#include "aesdchar.h"

#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#include "aesd-circular-buffer.h"
#include "aesdchar.h"


int aesd_major =   0; // use dynamic major
int aesd_minor =   0;
#include <linux/device.h>

static struct class *aesd_class = NULL;
static struct device *aesd_device_file = NULL;

MODULE_AUTHOR("Pooria Alaeienzhad"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");

    struct aesd_dev *dev;

    // container_of برای دسترسی به ساختار aesd_dev از inode
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev;  // ذخیره در file structure برای استفاده در read/write

    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");


    return 0;
}


ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *entry;
    size_t entry_offset;
    size_t bytes_copied = 0;
    ssize_t retval = 0;

    if (count == 0)
        return 0;

    mutex_lock(&dev->lock);

    size_t fpos = *f_pos;

    while (bytes_copied < count) {
        entry = aesd_circular_buffer_find_entry_offset_for_fpos(&dev->buffer, fpos, &entry_offset);
        if (!entry) {
            break; // no more data
        }

        size_t available = entry->size - entry_offset;
        size_t to_copy = (count - bytes_copied < available) ? (count - bytes_copied) : available;

        if (copy_to_user(buf + bytes_copied, entry->buffptr + entry_offset, to_copy)) {
            retval = -EFAULT;
            goto out;
        }

        fpos += to_copy;
        bytes_copied += to_copy;
    }

    *f_pos += bytes_copied;
    retval = bytes_copied;

out:
    mutex_unlock(&dev->lock);
    return retval;
}


ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct aesd_dev *dev = filp->private_data;
    char *kernel_buf;
    ssize_t retval = count;
    size_t offset = 0;
    struct aesd_buffer_entry new_entry;

    mutex_lock(&dev->lock);

    kernel_buf = kmalloc(count, GFP_KERNEL);
    if (!kernel_buf) {
        retval = -ENOMEM;
        goto out;
    }

    if (copy_from_user(kernel_buf, buf, count)) {
        kfree(kernel_buf);
        retval = -EFAULT;
        goto out;
    }

    while (offset < count) {
        char *newline_ptr = memchr(kernel_buf + offset, '\n', count - offset);
        size_t chunk_size;

        if (newline_ptr) {
            chunk_size = newline_ptr - (kernel_buf + offset) + 1; // include '\n'

            // Allocate buffer for pending + current chunk
            new_entry.size = dev->pending_write_size + chunk_size;
            new_entry.buffptr = kmalloc(new_entry.size, GFP_KERNEL);
            if (!new_entry.buffptr) {
                retval = -ENOMEM;
                break;
            }

            // Copy pending write data if any
            if (dev->pending_write_size > 0) {
                memcpy(new_entry.buffptr, dev->pending_write, dev->pending_write_size);
                kfree(dev->pending_write);
                dev->pending_write = NULL;
                dev->pending_write_size = 0;
            }

            // Copy new chunk after pending
            memcpy(new_entry.buffptr + (new_entry.size - chunk_size), kernel_buf + offset, chunk_size);

            aesd_circular_buffer_add_entry(&dev->buffer, &new_entry);
            printk(KERN_INFO "aesdchar: Added new entry of size %zu to circular buffer\n", new_entry.size);

            offset += chunk_size;
        } else {
            // Save to pending_write
            char *new_buf = krealloc(dev->pending_write, dev->pending_write_size + (count - offset), GFP_KERNEL);
            if (!new_buf) {
                retval = -ENOMEM;
                break;
            }

            memcpy(new_buf + dev->pending_write_size, kernel_buf + offset, count - offset);
            dev->pending_write = new_buf;
            dev->pending_write_size += (count - offset);
            break;
        }
    }

    kfree(kernel_buf);

out:
    mutex_unlock(&dev->lock);
    return retval;
}

loff_t aesd_llseek(struct file *filp, loff_t offset, int whence)
{
    struct aesd_dev *dev = &aesd_device;
    loff_t new_pos = -1;
    loff_t total_size = 0;
    uint8_t i;
    struct aesd_buffer_entry *entry;

    mutex_lock(&dev->lock);

    AESD_CIRCULAR_BUFFER_FOREACH(entry, &dev->buffer, i) {
        if (entry->buffptr != NULL) {
            total_size += entry->size;
        }
    }

    switch (whence) {
        case SEEK_SET:
            if (offset >= 0 && offset <= total_size)
                new_pos = offset;
            break;

        case SEEK_CUR:
            if ((filp->f_pos + offset >= 0) && (filp->f_pos + offset <= total_size))
                new_pos = filp->f_pos + offset;
            break;

        case SEEK_END:
            if ((total_size + offset >= 0) && (total_size + offset <= total_size))
                new_pos = total_size + offset;
            break;

        default:
            new_pos = -EINVAL;
            break;
    }

    if (new_pos >= 0)
        filp->f_pos = new_pos;

    mutex_unlock(&dev->lock);
    return new_pos;
}




struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
    .llseek =   aesd_llseek,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;

    result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
    if (result < 0) {
        printk(KERN_ERR "aesdchar: alloc_chrdev_region failed with error %d\n", result);
        return result;
    }

    aesd_major = MAJOR(dev);
    printk(KERN_INFO "aesdchar: registered with major=%d\n", aesd_major);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    aesd_circular_buffer_init(&aesd_device.buffer);
    mutex_init(&aesd_device.lock);
    aesd_device.pending_write = NULL;
    aesd_device.pending_write_size = 0;

    aesd_class = class_create(THIS_MODULE, "aesdchar");
    if (IS_ERR(aesd_class)) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "aesdchar: Failed to create class\n");
        return PTR_ERR(aesd_class);
    }
    
    aesd_device_file = device_create(aesd_class, NULL, dev, NULL, "aesdchar");
    if (IS_ERR(aesd_device_file)) {
        class_destroy(aesd_class);
        unregister_chrdev_region(dev, 1);
        printk(KERN_ERR "aesdchar: Failed to create device\n");
        return PTR_ERR(aesd_device_file);
    }
    
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    uint8_t i;
    struct aesd_buffer_entry *entry;

    for (i = 0; i < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED; i++) {
        entry = &aesd_device.buffer.entry[i];
        if (entry->buffptr != NULL) {
            kfree(entry->buffptr);
        }
    }

    if (aesd_device.pending_write != NULL) {
        kfree(aesd_device.pending_write);
    }
    device_destroy(aesd_class, MKDEV(aesd_major, aesd_minor));
    class_destroy(aesd_class);
    

    unregister_chrdev_region(devno, 1);
}


module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
