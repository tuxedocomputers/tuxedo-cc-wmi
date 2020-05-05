/*!
 * Copyright (c) 2020 TUXEDO Computers GmbH <tux@tuxedocomputers.com>
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
#include <linux/acpi.h>
#include <linux/wmi.h>

#define UNIWILL_WMI_MGMT_GUID_BA            "ABBC0F6D-8EA1-11D1-00A0-C90629100000"
#define UNIWILL_WMI_MGMT_GUID_BB            "ABBC0F6E-8EA1-11D1-00A0-C90629100000"
#define UNIWILL_WMI_MGMT_GUID_BC            "ABBC0F6F-8EA1-11D1-00A0-C90629100000"

static u32 tongfang_wmi_evaluate(u8 addr_low, u8 addr_high, u8 data_low, u8 data_high, u8 read_flag, u32 *return_buffer)
{
    acpi_status status;
    union acpi_object *out_acpi;
    u32 e_result = 0;

    u8 wmi_instance = 0x00;
    u32 wmi_method_id = 0x04;

    u32 *wmi_arg = (u32 *) kmalloc(sizeof(u32)*10, GFP_KERNEL);

    struct acpi_buffer in = { (acpi_size) sizeof(wmi_arg), wmi_arg};
    struct acpi_buffer out = { ACPI_ALLOCATE_BUFFER, NULL };

    u8 *wmi_arg_bytes = (u8 *) wmi_arg;

    wmi_arg[0] = 0x00;
    wmi_arg[1] = 0x00;
    wmi_arg[2] = 0x00;
    wmi_arg[3] = 0x00;
    wmi_arg[4] = 0x00;
    wmi_arg[5] = 0x00;
    wmi_arg[6] = 0x00;
    wmi_arg[7] = 0x00;
    wmi_arg[8] = 0x00;
    wmi_arg[9] = 0x00;

    wmi_arg_bytes[0] = addr_low;
    wmi_arg_bytes[1] = addr_high;
    wmi_arg_bytes[2] = data_low;
    wmi_arg_bytes[3] = data_high;

    if (read_flag != 0) {
        wmi_arg_bytes[5] = 0x01;
    }
    
    status = wmi_evaluate_method(UNIWILL_WMI_MGMT_GUID_BC, wmi_instance, wmi_method_id, &in, &out);
    out_acpi = (union acpi_object *)out.pointer;
    if (out_acpi && out_acpi->type == ACPI_TYPE_INTEGER) {
        e_result = (u32) out_acpi->integer.value;
    } else if (out_acpi && out_acpi->type == ACPI_TYPE_BUFFER) {
        memcpy(return_buffer, out_acpi->buffer.pointer, out_acpi->buffer.length);
        e_result = 0xd00df00d;
    }
    if (ACPI_FAILURE(status)) {
        printk(KERN_INFO "tongfang_wmi.h: Error evaluating method\n");
    }

    kfree(out_acpi);
    kfree(wmi_arg);

    return e_result;
}
