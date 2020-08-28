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
#include <linux/delay.h>
#include "clevo_wmi.h"
#include "tongfang_wmi.h"
#include "tuxedo_cc_wmi_ioctl.h"

MODULE_DESCRIPTION("WMI method control for TUXEDO laptops");
MODULE_AUTHOR("TUXEDO Computers GmbH <tux@tuxedocomputers.com>");
MODULE_VERSION("0.1.6");
MODULE_LICENSE("GPL");
MODULE_ALIAS("wmi:" CLEVO_WMI_METHOD_GUID);
MODULE_ALIAS("wmi:" UNIWILL_WMI_MGMT_GUID_BA);
MODULE_ALIAS("wmi:" UNIWILL_WMI_MGMT_GUID_BB);
MODULE_ALIAS("wmi:" UNIWILL_WMI_MGMT_GUID_BC);

// Initialized in module init, global for ioctl interface
static u32 id_check_clevo;
static u32 id_check_uniwill;

/*static int fop_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int fop_release(struct inode *inode, struct file *file)
{
    return 0;
}*/

static long clevo_ioctl_interface(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 result = 0;
    u32 copy_result;
    u32 argument = (u32) arg;
    
    switch (cmd) {
        case R_FANINFO1:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO1, 0);
            copy_result = copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FANINFO2:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO2, 0);
            copy_result = copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FANINFO3:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO3, 0);
            copy_result = copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        /*case R_FANINFO4:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FANINFO4, 0);
            copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;*/
        case R_WEBCAM_SW:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_WEBCAM_SW, 0);
            copy_result = copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_FLIGHTMODE_SW:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_FLIGHTMODE_SW, 0);
            copy_result = copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
        case R_TOUCHPAD_SW:
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_TOUCHPAD_SW, 0);
            copy_result = copy_to_user((int32_t *) arg, &result, sizeof(result));
            break;
    }

    switch (cmd) {
        case W_FANSPEED:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FANSPEED_VALUE, argument);
            // Note: Delay needed to let hardware catch up with the written value.
            // No known ready flag. If the value is read too soon, the old value
            // will still be read out.
            // (Theoretically needed for other methods as well.)
            // Can it be lower? 50ms is too low
            msleep(100);
            break;
        case W_FANAUTO:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FANSPEED_AUTO, argument);
            break;
        case W_WEBCAM_SW:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            result = clevo_wmi_evaluate(CLEVO_WMI_CMD_GET_WEBCAM_SW, 0);
            // Only set status if it isn't already the right value
            // (workaround for old and/or buggy WMI interfaces that toggle on write)
            if ((argument & 0x01) != (result & 0x01)) {
                clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_WEBCAM_SW, argument);
            }
            break;
        case W_FLIGHTMODE_SW:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_FLIGHTMODE_SW, argument);
            break;
        case W_TOUCHPAD_SW:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            clevo_wmi_evaluate(CLEVO_WMI_CMD_SET_TOUCHPAD_SW, argument);
            break;
    }

    return 0;
}

static long uniwill_ioctl_interface(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 result = 0;
    u32 copy_result;
    u32 argument;
    union uw_ec_read_return reg_read_return;
    union uw_ec_write_return reg_write_return;

#ifdef DEBUG
    u32 uw_arg[10];
    u32 uw_result[10];
    int i;
    for (i = 0; i < 10; ++i) {
        uw_result[i] = 0xdeadbeef;
    }
#endif

    switch (cmd) {
        case R_UW_FANSPEED:
            uniwill_wmi_ec_read(0x04, 0x18, &reg_read_return);
            result = reg_read_return.bytes.data_low;
            copy_result = copy_to_user((void *) arg, &result, sizeof(result));
            break;
        case R_UW_FAN_TEMP:
            uniwill_wmi_ec_read(0x3e, 0x04, &reg_read_return);
            result = reg_read_return.bytes.data_low;
            copy_result = copy_to_user((void *) arg, &result, sizeof(result));
            break;
        case R_UW_MODE:
            uniwill_wmi_ec_read(0x51, 0x07, &reg_read_return);
            result = reg_read_return.bytes.data_low;
            copy_result = copy_to_user((void *) arg, &result, sizeof(result));
            break;
        case R_UW_MODE_ENABLE:
            uniwill_wmi_ec_read(0x41, 0x07, &reg_read_return);
            result = reg_read_return.bytes.data_low;
            copy_result = copy_to_user((void *) arg, &result, sizeof(result));
            break;
#ifdef DEBUG
        case R_TF_BC:
            copy_result = copy_from_user(&uw_arg, (void *) arg, sizeof(uw_arg));
            pr_info("R_TF_BC args [%0#2x, %0#2x, %0#2x, %0#2x]\n", uw_arg[0], uw_arg[1], uw_arg[2], uw_arg[3]);
            result = uniwill_wmi_ec_evaluate(uw_arg[0], uw_arg[1], uw_arg[2], uw_arg[3], 1, uw_result);
            copy_result = copy_to_user((void *) arg, &uw_result, sizeof(uw_result));
            break;
#endif
    }

    switch (cmd) {
        case W_UW_FANSPEED:
            // Get fan speed argument
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));

            // Check current mode
            uniwill_wmi_ec_read(0x51, 0x07, &reg_read_return);
            if (reg_read_return.bytes.data_low != 0x40) {
                // If not "full fan mode" (ie. 0x40) switch to it (required for fancontrol)
                uniwill_wmi_ec_write(0x51, 0x07, 0x40, reg_read_return.bytes.data_high, &reg_write_return);
            }
            // Set speed
            uniwill_wmi_ec_read(0x04, 0x18, &reg_read_return);
            uniwill_wmi_ec_write(0x04, 0x18, argument & 0xff, reg_read_return.bytes.data_high, &reg_write_return);
            break;
        case W_UW_MODE:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            uniwill_wmi_ec_read(0x51, 0x07, &reg_read_return);
            uniwill_wmi_ec_write(0x51, 0x07, argument & 0xff, reg_read_return.bytes.data_high, &reg_write_return);
            break;
        case W_UW_MODE_ENABLE:
            copy_result = copy_from_user(&argument, (int32_t *) arg, sizeof(argument));
            uniwill_wmi_ec_read(0x41, 0x07, &reg_read_return);
            uniwill_wmi_ec_write(0x41, 0x07, argument & 0x01, reg_read_return.bytes.data_high, &reg_write_return);
            break;
#ifdef DEBUG
        case W_TF_BC:
            copy_result = copy_from_user(&uw_arg, (void *) arg, sizeof(uw_arg));
            result = uniwill_wmi_ec_evaluate(uw_arg[0], uw_arg[1], uw_arg[2], uw_arg[3], 0, uw_result);
            copy_result = copy_to_user((void *) arg, &uw_result, sizeof(uw_result));
            reg_write_return.dword = uw_result[0];
            /*pr_info("data_high %0#2x\n", reg_write_return.bytes.data_high);
            pr_info("data_low %0#2x\n", reg_write_return.bytes.data_low);
            pr_info("addr_high %0#2x\n", reg_write_return.bytes.addr_high);
            pr_info("addr_low %0#2x\n", reg_write_return.bytes.addr_low);*/
            break;
#endif
    }

    return 0;
}

static long fop_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    u32 status;
    // u32 result = 0;
    u32 copy_result;

    const char *module_version = THIS_MODULE->version;
    switch (cmd) {
        case R_MOD_VERSION:
            copy_result = copy_to_user((char *) arg, module_version, strlen(module_version) + 1);
            break;
        // Hardware id checks, 1 = positive, 0 = negative
        case R_HWCHECK_CL:
            copy_result = copy_to_user((void *) arg, (void *) &id_check_clevo, sizeof(id_check_clevo));
            break;
        case R_HWCHECK_UW:
            copy_result = copy_to_user((void *) arg, (void *) &id_check_uniwill, sizeof(id_check_uniwill));
            break;
    }

    status = clevo_ioctl_interface(file, cmd, arg);
    if (status != 0) return status;
    status = uniwill_ioctl_interface(file, cmd, arg);
    if (status != 0) return status;

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

    // Hardware identification
    // This is not hotpluggable so can be done in module init
    id_check_clevo = clevo_identify() == 0 ? 1 : 0;
    id_check_uniwill = uniwill_identify() == 0 ? 1 : 0;

#ifdef DEBUG
    if (id_check_clevo == 0 && id_check_uniwill == 0) {
        pr_debug("No matching hardware found\n");
    }
#endif

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
    pr_debug("Module init successful\n");
    
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
