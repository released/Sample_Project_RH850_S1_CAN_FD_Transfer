/* Project/API reading notes: Device-facing CAN adapter.
 * Return complete frames with their physical channel, preserve bounded receive/diagnostic
 * handling and report real TX completion.
 * Boot/App/Transfer protocol work remains in foreground code.
 */
#include <string.h>
#include "r_smc_entry.h"
#include "device_platform.h"
#include "can_boot_config.h"
#include "rh850_can_ports.h"

static unsigned char g_reply_port;
unsigned char Device_CanInit(unsigned short request_id, unsigned char raw)
{
    g_reply_port = CanPorts_First();
    return CanPorts_Init(RH850_CAN_FD ? 1U : 0U, request_id, raw, CAN_BOOT_BLOCK_DATA_ID,
                         RH850_CAN_FD ? 0x7FFU : 0x7F8U);
}
/* One global RX FIFO interrupt dispatches the enabled, disjoint FIFO set. */
#pragma interrupt Device_CanRxIsr(enable = false, fpu = true, callt = false)
/* Capture CAN receive data into the bounded service queues.
 * Parsing and replies run later in foreground context.
 */
void Device_CanRxIsr(void)
{
    CanPorts_Irq();
}
unsigned char Device_CanRead(boot_frame_t *frame)
{
    can_port_frame_t received;
    if (!frame || !CanPorts_Read(&received))
    {
        return 0U;
    }
    frame->id = received.id;
    frame->length = received.length;
    frame->channel = received.channel;
    g_reply_port = received.channel;
    memcpy(frame->data, received.data, received.length);
    return 1U;
}
unsigned char Device_CanSend(unsigned short id, const unsigned char *data, unsigned char length)
{
    return CanPorts_Send(g_reply_port, id, data, length);
}
unsigned char Device_CanPort(void)
{
    return g_reply_port;
}
unsigned char Device_CanLastResult(void)
{
    can_port_diagnostics_t diagnostic;
    CanPorts_Diagnostics(g_reply_port, &diagnostic);
    return (unsigned char)diagnostic.last_result;
}
unsigned long Device_CanDiagnostics(void)
{
    can_port_diagnostics_t diagnostic;
    CanPorts_Diagnostics(g_reply_port, &diagnostic);
    return (unsigned long)diagnostic.dropped | ((unsigned long)diagnostic.invalid << 16);
}
void Device_CanStop(void)
{
    CanPorts_Stop();
}
