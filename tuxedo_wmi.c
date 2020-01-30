
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "clevo_wmi.h"
#include "tuxedo_wmi_ioctl.h"

MODULE_LICENSE("GPL");

static int fop_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int fop_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t fop_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "fop_read()\n");
    return 0;
}

static ssize_t fop_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    printk(KERN_INFO "fop_write()\n");
    return 0;
}

static long fop_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 result = 0;
    u32 argument = (u32) arg;

    switch (cmd) {
        case R_FANINFO_CPU:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO_CPU, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FANINFO_GPU1:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO_GPU1, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FANINFO_GPU2:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO_GPU2, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_WEBCAM_SW:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_WEBCAM_SW, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FLIGHTMODE_SW:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FLIGHTMODE_SW, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_TOUCHPAD_SW:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_TOUCHPAD_SW, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
    }

    switch (cmd) {
        case W_FANSPEED:
            printk(KERN_INFO "W_FANSPEED\n");
            printk(KERN_INFO "speed param: %ld\n", arg);
            // copy_from_user(&local_argument, (int32_t *) arg, sizeof(local_argument));
            printk(KERN_INFO "speed copy: %d\n", argument);
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FANSPEED_VALUE, argument);
            break;
        case W_WEBCAM_SW:
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_WEBCAM_SW, argument);
            break;
        case W_FLIGHTMODE_SW:
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FLIGHTMODE_SW, argument);
            break;
        case W_TOUCHPAD_SW:
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_TOUCHPAD_SW, argument);
            break;
    }
    return 0;
}

static struct file_operations fops_dev = {
    .owner              = THIS_MODULE,
    .read               = fop_read,
    .write              = fop_write,
    .open               = fop_open,
    .unlocked_ioctl     = fop_ioctl,
    .release            = fop_release,
};

struct class *tuxedo_wmi_device_class;
dev_t tuxedo_wmi_device_handle;

static struct cdev tuxedo_wmi_cdev;

static int __init tuxedo_wmi_init(void)
{
    int err;

    err = alloc_chrdev_region(&tuxedo_wmi_device_handle, 0, 1, "tuxedo_wmi_cdev");
    if (err != 0) {
        printk(KERN_INFO "Failed to allocate chrdev region\n");
        return err;
    }
    cdev_init(&tuxedo_wmi_cdev, &fops_dev);
    if ((cdev_add(&tuxedo_wmi_cdev, tuxedo_wmi_device_handle, 1)) < 0) {
        printk(KERN_INFO "Failed to add cdev\n");
    }
    tuxedo_wmi_device_class = class_create(THIS_MODULE, "tuxedo_wmi");
    device_create(tuxedo_wmi_device_class, NULL, tuxedo_wmi_device_handle, NULL, "tuxedo_wmi");

    printk(KERN_INFO "WMI Module init.\n");
    if (!wmi_has_guid(CLEVO_WMI_METHOD_GUID)) {
        printk(KERN_INFO "GUID (%s) not found\n", CLEVO_WMI_METHOD_GUID);
        return -ENODEV;
    }
    
    return 0;
}

static void __exit tuxedo_wmi_exit(void)
{
    device_destroy(tuxedo_wmi_device_class, tuxedo_wmi_device_handle);
    class_destroy(tuxedo_wmi_device_class);
    cdev_del(&tuxedo_wmi_cdev);
    unregister_chrdev_region(tuxedo_wmi_device_handle, 1);
    printk(KERN_INFO "WMI Module exit.\n");
}

module_init(tuxedo_wmi_init);
module_exit(tuxedo_wmi_exit);
