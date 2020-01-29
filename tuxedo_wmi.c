
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

/*static void test_wmi_notify(u32 value, void *context)
{
    printk(KERN_INFO "WMI notification: %d", value);
}*/

struct class *device_class;
dev_t device_handle;

static struct cdev wmi_driver_cdev;

static int __init wmi_driver_init(void)
{
    int err;

    err = alloc_chrdev_region(&device_handle, 0, 1, "wmi_driver_cdev");
    if (err != 0) {
        printk(KERN_INFO "Failed to allocate chrdev region\n");
        return err;
    }
    cdev_init(&wmi_driver_cdev, &fops_dev);
    if ((cdev_add(&wmi_driver_cdev, device_handle, 1)) < 0) {
        printk(KERN_INFO "Failed to add cdev\n");
    }
    device_class = class_create(THIS_MODULE, "wmi_driver_class");
    device_create(device_class, NULL, device_handle, NULL, "wmi_driver_dev");


    // acpi_status status;

    printk(KERN_INFO "WMI Module init.\n");
    if (!wmi_has_guid(CLEVO_WMI_METHOD_GUID)) {
        printk(KERN_INFO "GUID (%s) not found\n", CLEVO_WMI_METHOD_GUID);
        return -ENODEV;
    }
    
    /*status = wmi_install_notify_handler(GUID, test_wmi_notify, NULL);
    if (ACPI_FAILURE(status)) {
        printk(KERN_INFO "Failure installing notify handler");
        return -ENODEV;
    }*/

    return 0;
}

static void __exit wmi_driver_exit(void)
{
    device_destroy(device_class, device_handle);
    class_destroy(device_class);
    cdev_del(&wmi_driver_cdev);
    unregister_chrdev_region(device_handle, 1);
    printk(KERN_INFO "WMI Module exit.\n");
    // wmi_remove_notify_handler(GUID);
}

int param_int = 0;

static int set_param_int(const char *val, const struct kernel_param *kp)
{
    int value = 0;
    int result;

    result = kstrtoint(val, 10, &value);
    if (result != 0 || value < 0 || value > 255) {
        return -EINVAL;
    }

    return param_set_int(val, kp);
}

static int get_param_int(char *buffer, const struct kernel_param *kp)
{
    u32 wmi_cmd = 0x79;
    u32 wmi_sub_cmd = 0x0f;
    u32 wmi_sub_arg = param_int & 0x01;
    u32 wmi_arg = (wmi_sub_cmd << 0x18) | (wmi_sub_arg && 0x00ffffff);

    struct wmi_args {
        u32 arg2;
    };
    struct wmi_args arg = {
        .arg2 = wmi_arg
    };
    struct acpi_buffer in = { (acpi_size) sizeof(arg), &arg};
    struct acpi_buffer out = { ACPI_ALLOCATE_BUFFER, NULL };
    
    acpi_status status;
    union acpi_object *out_acpi;
    u32 e_result;
    status = wmi_evaluate_method(CLEVO_WMI_METHOD_GUID, 0, wmi_cmd, &in, &out);
    out_acpi = (union acpi_object *)out.pointer;
    if (out_acpi && out_acpi->type == ACPI_TYPE_INTEGER) {
        e_result = (u32) out_acpi->integer.value;
        printk(KERN_INFO "Result: %d\n", e_result);
    }
    if (ACPI_FAILURE(status)) {
        printk(KERN_INFO "Error evaluating method\n");
    }

    kfree(out_acpi);

    printk(KERN_INFO "p_int = %d\n", param_int);
    return param_get_int(buffer, kp);
}

static const struct kernel_param_ops param_ops = {
    .set    = set_param_int,
    .get    = get_param_int
};

module_param_cb(p_int, &param_ops, &param_int, S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
MODULE_PARM_DESC(p_int, "Integer sample parameter");

module_init(wmi_driver_init);
module_exit(wmi_driver_exit);
