#include <linux/acpi.h>
#include <linux/wmi.h>

#define CLEVO_WMI_METHOD_GUID               "ABBC0F6D-8EA1-11D1-00A0-C90629100000"

// The clevo get commands expect no parameters
#define CLEVO_WMI_CMD_GET_FANINFO1          0x63
#define CLEVO_WMI_CMD_GET_FANINFO2          0x64
#define CLEVO_WMI_CMD_GET_FANINFO3          0x6e
#define CLEVO_WMI_CMD_GET_FANINFO4          0x6f

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
