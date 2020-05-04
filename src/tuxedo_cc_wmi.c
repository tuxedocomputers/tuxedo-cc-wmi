/*!
 * Copyright (c) 2019-2020 TUXEDO Computers GmbH <tux@tuxedocomputers.com>
 *
 * This file is part of tuxedo-cc-wmi.
 *
 * tuxedo-cc-wmi is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.  If not, see <https://www.gnu.org/licenses/>.
 */
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include "clevo_wmi.h"
#include "tongfang_wmi.h"
#include "tuxedo_cc_wmi_ioctl.h"

MODULE_DESCRIPTION("WMI method control for TUXEDO laptops");
MODULE_AUTHOR("TUXEDO Computers GmbH <tux@tuxedocomputers.com>");
MODULE_VERSION("0.1.3");
MODULE_LICENSE("GPL");
MODULE_ALIAS("wmi:" CLEVO_WMI_METHOD_GUID);
MODULE_ALIAS("wmi:" UNIWILL_WMI_MGMT_GUID_BA);
MODULE_ALIAS("wmi:" UNIWILL_WMI_MGMT_GUID_BB);
MODULE_ALIAS("wmi:" UNIWILL_WMI_MGMT_GUID_BC);

/*static int fop_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int fop_release(struct inode *inode, struct file *file)
{
    return 0;
}*/

static long fop_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 result = 0;
    u32 argument = (u32) arg;
    const char *module_version = THIS_MODULE->version;

    u32 tf_arg[4];
    u32 tf_result[8];

    switch (cmd) {
        case R_MOD_VERSION:
            copy_to_user((char *) arg, module_version, strlen(module_version) + 1);
            break;
        case R_FANINFO1:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO1, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FANINFO2:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO2, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FANINFO3:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO3, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        /*case R_FANINFO4:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO4, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;*/
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
        case R_TF_BC:
            copy_from_user(&tf_arg, (void *) arg, sizeof(tf_arg));
            result = tongfang_wmi_evaluate(tf_arg[0], tf_arg[1], tf_arg[2], tf_arg[3], 1, tf_result);
            copy_to_user((void *) arg, &tf_result, sizeof(tf_result));
            break;
    }

    switch (cmd) {
        case W_FANSPEED:
            copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FANSPEED_VALUE, argument);
            break;
        case W_FANAUTO:
            copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FANSPEED_AUTO, argument);
            break;
        case W_WEBCAM_SW:
            copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_WEBCAM_SW, argument);
            break;
        case W_FLIGHTMODE_SW:
            copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FLIGHTMODE_SW, argument);
            break;
        case W_TOUCHPAD_SW:
            copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_TOUCHPAD_SW, argument);
            break;
        case W_TF_BC:
            copy_from_user(&tf_arg, (void *) arg, sizeof(tf_arg));
            result = tongfang_wmi_evaluate(tf_arg[0], tf_arg[1], tf_arg[2], tf_arg[3], 0, tf_result);
            copy_to_user((void *) arg, &tf_result, sizeof(tf_result));
            break;
    }
    return 0;
}

static struct file_operations fops_dev = {
    .owner              = THIS_MODULE,
    .unlocked_ioctl     = fop_ioctl
//    .open               = fop_open,
//    .release            = fop_release
};

struct class *tuxedo_cc_wmi_device_class;
dev_t tuxedo_cc_wmi_device_handle;

static struct cdev tuxedo_cc_wmi_cdev;

static int __init tuxedo_cc_wmi_init(void)
{
    int err;

    // Only initialize if WMI GUID exists
    // This is not hotpluggable so can be done in module init
    if (wmi_has_guid(CLEVO_WMI_METHOD_GUID)) {
        err = alloc_chrdev_region(&tuxedo_cc_wmi_device_handle, 0, 1, "tuxedo_cc_wmi_cdev");
        if (err != 0) {
            pr_err("Failed to allocate chrdev region\n");
            return err;
        }
        cdev_init(&tuxedo_cc_wmi_cdev, &fops_dev);
        err = (cdev_add(&tuxedo_cc_wmi_cdev, tuxedo_cc_wmi_device_handle, 1));
        if (err < 0) {
            pr_err("Failed to add cdev\n");
            unregister_chrdev_region(tuxedo_cc_wmi_device_handle, 1);
        }
        tuxedo_cc_wmi_device_class = class_create(THIS_MODULE, "tuxedo_cc_wmi");
        device_create(tuxedo_cc_wmi_device_class, NULL, tuxedo_cc_wmi_device_handle, NULL, "tuxedo_cc_wmi");
        pr_debug("Successfully initialized\n");
    } else {
        pr_debug("Expected GUID (%s) not found, module inactive\n", CLEVO_WMI_METHOD_GUID);
    }
    
    return 0;
}

static void __exit tuxedo_cc_wmi_exit(void)
{
    device_destroy(tuxedo_cc_wmi_device_class, tuxedo_cc_wmi_device_handle);
    class_destroy(tuxedo_cc_wmi_device_class);
    cdev_del(&tuxedo_cc_wmi_cdev);
    unregister_chrdev_region(tuxedo_cc_wmi_device_handle, 1);
    pr_debug("Module exit\n");
}

module_init(tuxedo_cc_wmi_init);
module_exit(tuxedo_cc_wmi_exit);
