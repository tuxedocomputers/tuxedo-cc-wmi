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
#include <linux/acpi.h>
#include <linux/wmi.h>

#define CLEVO_WMI_METHOD_GUID               "ABBC0F6D-8EA1-11D1-00A0-C90629100000"
#define CLEVO_WMI_EVENT_GUID                "ABBC0F6B-8EA1-11D1-00A0-C90629100000"

// The clevo get commands expect no parameters
#define CLEVO_WMI_CMD_GET_FANINFO1          0x63
#define CLEVO_WMI_CMD_GET_FANINFO2          0x64
#define CLEVO_WMI_CMD_GET_FANINFO3          0x6e

#define CLEVO_WMI_CMD_GET_WEBCAM_SW         0x06
#define CLEVO_WMI_CMD_GET_FLIGHTMODE_SW     0x07
#define CLEVO_WMI_CMD_GET_TOUCHPAD_SW       0x09

// The clevo set commands expect a parameter
#define CLEVO_WMI_CMD_SET_FANSPEED_VALUE    0x68
#define CLEVO_WMI_CMD_SET_FANSPEED_AUTO     0x69

#define CLEVO_WMI_CMD_SET_WEBCAM_SW         0x22
#define CLEVO_WMI_CMD_SET_FLIGHTMODE_SW     0x20
#define CLEVO_WMI_CMD_SET_TOUCHPAD_SW       0x2a

static u32 clevo_wmi_evaluate(u8, u32);

static u32 clevo_wmi_evaluate(u8 cmd, u32 arg)
{
    u32 wmi_cmd = cmd;
    // u32 wmi_sub_cmd = 0x0f;
    // u32 wmi_sub_arg = param_int & 0x01;
    // u32 arg = arg; // (wmi_sub_cmd << 0x18) | (wmi_sub_arg && 0x00ffffff);

    struct wmi_args {
        u32 arg2;
    };
    struct wmi_args wmi_arg = {
        .arg2 = arg
    };
    struct acpi_buffer in = { (acpi_size) sizeof(wmi_arg), &wmi_arg};
    struct acpi_buffer out = { ACPI_ALLOCATE_BUFFER, NULL };
    
    acpi_status status;
    union acpi_object *out_acpi;
    u32 e_result = 0;
    status = wmi_evaluate_method(CLEVO_WMI_METHOD_GUID, 0, wmi_cmd, &in, &out);
    out_acpi = (union acpi_object *)out.pointer;
    if (out_acpi && out_acpi->type == ACPI_TYPE_INTEGER) {
        e_result = (u32) out_acpi->integer.value;
    }
    if (ACPI_FAILURE(status)) {
        printk(KERN_INFO "clevo_wmi.h: Error evaluating method\n");
    }

    kfree(out_acpi);

    return e_result;
}

static u32 clevo_identify(void) {
    int result;

    if (!wmi_has_guid(CLEVO_WMI_EVENT_GUID))
    {
        pr_debug("probe: Clevo event guid missing\n");
        return -ENODEV;
    }

    if (!wmi_has_guid(CLEVO_WMI_METHOD_GUID))
    {
        pr_debug("probe: Clevo method guid missing\n");
        return -ENODEV;
    }

    // Since the WMI GUIDs aren't unique let's (at least)
    // check the return of some "known existing general" method
    result = clevo_wmi_evaluate(0x52, 0);
    if (result == 0xffffffff)
    {
        pr_debug("probe: Clevo GUIDs present but method returned unexpected value\n");
        return -ENODEV;
    }

    return 0;
}
