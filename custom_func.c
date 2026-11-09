/* Project/API reading notes: Project-owned initialization and foreground work.
 * The selected main/startup adapter calls hardware_init() once and loop() repeatedly.
 * Keep application changes here so generated entry sources remain replaceable; lower-level APIs
 * live in this project's hal directories.
 */
#include "r_smc_entry.h"
#include "custom_func.h"
#include "retarget.h"
#define UART0_CONSOLE_RH850 1
static void Uart0ConsoleReset(void)
{
    unsigned char retry;
    DI();
    for (retry = 0U; retry < 4U; ++retry)
    {
        WPROTR.PROTCMD0 = 0xA5U;
        RESCTL.SWRESA = 1UL;
        RESCTL.SWRESA = ~1UL;
        RESCTL.SWRESA = 1UL;
        __syncp();
    }
    for (;;)
    {
        NOP();
    }
}
#define UART0_CONSOLE_RESET() Uart0ConsoleReset()
#include "uart0_reset_console.inc"
#include "device_platform.h"
#include "can_transfer_protocol.h"
#include "can_transfer_config.h"
#include "rh850_can_ports.h"

static volatile unsigned long g_millis;
static ct_protocol_t g_protocol[6];
volatile UART_MANAGER_T UART0Manager;
unsigned long Device_Millis(void)
{
    return g_millis;
}
/* Advance the monotonic timebase only.
 * Protocol timeout, application work and text output are handled outside this ISR.
 */
void ostmr_1ms_IRQ(void)
{
    g_millis++;
}
void tmr_1ms_IRQ(void)
{
}
void UARTx_Process(unsigned char value)
{
    (void)value;
}
void UARTx_ErrorCheckProcess(unsigned char value)
{
    (void)value;
}
/* Foreground-only diagnostic output used by retarget.c.
 * Preserve a finite wait when UART0 is busy or stopped; ISP data uses its own link.
 */
int SendChar(unsigned char value)
{
    unsigned long guard;
    guard = 100000UL;
    while ((RLN30.LST & _UART_TRANSMISSION_OPERATED) && guard)
    {
        guard--;
    }
    if (guard)
    {
        RLN30.LUTDR.UINT16 = value;
        return (int)value;
    }
    return -1;
}
static void Trace(const char *direction, const unsigned char *bytes, unsigned char length)
{
#if CAN_TRANSFER_UART_TRACE
    unsigned char i;
    tiny_printf("%s CAN%u FD+BRS data=[", direction, (unsigned int)Device_CanPort());
    for (i = 0U; i < length; i++)
    {
        tiny_printf("%02X ", (unsigned int)bytes[i]);
    }
    tiny_printf("]\r\n");
#else
    (void)direction;
    (void)bytes;
    (void)length;
#endif
}

static boot_frame_t g_custom_frame;
static can_port_diagnostics_t g_custom_diagnostic;
static ct_protocol_t *g_custom_protocol;
static unsigned char g_custom_response[64], g_custom_length, g_custom_ready, g_custom_channel,
    g_custom_errors, g_custom_previous[6];
static unsigned short g_custom_drop_baseline[6], g_custom_invalid_seen[6], g_custom_invalid;
static unsigned long g_custom_status;

/* Initialize the role selected by this project before foreground processing.
 * Preserve clock, image/handoff and peripheral/IRQ enable order when adding application
 * initialization.
 */
void hardware_init(void)
{
    DI();
    R_Systeminit();
    for (g_custom_channel = 0U; g_custom_channel < 6U; g_custom_channel++)
    {
        CT_Init(&g_protocol[g_custom_channel]);
        g_custom_previous[g_custom_channel] = 0U;
        g_custom_drop_baseline[g_custom_channel] = 0U;
        g_custom_invalid_seen[g_custom_channel] = 0U;
    }
    R_Config_UART0_Start();
    INTC2.ICRLIN30UR0.BIT.MKRLIN30UR0 = 1U;
    INTC2.ICRLIN30UR1.BIT.MKRLIN30UR1 = 1U;
    INTC2.ICRLIN30UR2.BIT.MKRLIN30UR2 = 1U;
    R_Config_OSTM0_Start();
    EI();
    g_custom_ready = Device_CanInit(CAN_TRANSFER_REQUEST_ID, 0U);
    tiny_printf("RH850 CAN FD DEVICE TRANSFER: 500K/2M BRS; per-port FIFO IRQ\r\n");
    tiny_printf("%s", g_custom_ready ? "CAN init: OK\r\n" : "CAN init: FAILED\r\n");
    tiny_printf("UART0 console: z/Z reset\r\n");
}

/* Foreground service point called repeatedly by the selected main adapter.
 * Keep protocol/application work and text logging bounded so pending traffic and deferred actions
 * can continue.
 */
void loop(void)
{
    Uart0ResetPoll();


    if (!g_custom_ready)
    {
        return;
    }
    for (g_custom_channel = 0U; g_custom_channel < 6U; g_custom_channel++)
    {
        if (!CanPorts_Enabled(g_custom_channel))
        {
            continue;
        }
        g_custom_protocol = &g_protocol[g_custom_channel];
        CanPorts_Diagnostics(g_custom_channel, &g_custom_diagnostic);
        g_custom_protocol->stats[2] = (unsigned short)(g_custom_diagnostic.dropped -
                                                       g_custom_drop_baseline[g_custom_channel]);
        g_custom_invalid =
            (unsigned short)(g_custom_diagnostic.invalid - g_custom_invalid_seen[g_custom_channel]);
        g_custom_invalid_seen[g_custom_channel] = g_custom_diagnostic.invalid;
        while (g_custom_invalid)
        {
            CT_Increment(&g_custom_protocol->stats[4]);
            g_custom_invalid--;
        }
        g_custom_status = CanPorts_Status(g_custom_channel);
        g_custom_errors = 0U;
        if (((g_custom_status >> 8) & 255UL) >= 96UL || ((g_custom_status >> 16) & 255UL) >= 96UL)
        {
            g_custom_errors |= 1U;
        }
        if (g_custom_status & 16UL)
        {
            g_custom_errors |= 2U;
        }
        if (g_custom_status & 8UL)
        {
            g_custom_errors |= 4U;
        }
        if ((g_custom_errors & 1U) && !(g_custom_previous[g_custom_channel] & 1U))
        {
            CT_Increment(&g_custom_protocol->stats[7]);
        }
        if ((g_custom_errors & 2U) && !(g_custom_previous[g_custom_channel] & 2U))
        {
            CT_Increment(&g_custom_protocol->stats[8]);
        }
        if ((g_custom_errors & 4U) && !(g_custom_previous[g_custom_channel] & 4U))
        {
            CT_Increment(&g_custom_protocol->stats[9]);
        }
        g_custom_previous[g_custom_channel] = g_custom_errors;
        g_custom_protocol->stats[11] = (unsigned short)CanPorts_ErrorFlags(g_custom_channel);
        g_custom_protocol->stats[10] = g_custom_diagnostic.last_result;
    }
    if (Device_CanRead(&g_custom_frame))
    {
        g_custom_channel = g_custom_frame.channel;
        g_custom_protocol = &g_protocol[g_custom_channel];
        Trace("TRACE CT RX", g_custom_frame.data, g_custom_frame.length);
        g_custom_length = CT_ProcessFrame(g_custom_protocol, g_custom_frame.data,
                                          g_custom_frame.length, g_custom_response);
        if (!g_custom_length)
        {
            CT_Increment(&g_custom_protocol->stats[4]);
        }
        if (g_custom_length == 8U && g_custom_response[3] == 6U && !g_custom_protocol->stats[6])
        {
            CanPorts_Diagnostics(g_custom_channel, &g_custom_diagnostic);
            g_custom_drop_baseline[g_custom_channel] = g_custom_diagnostic.dropped;
        }
        if (g_custom_length)
        {
            if (Device_CanSend(CAN_TRANSFER_RESPONSE_ID, g_custom_response, g_custom_length))
            {
                CT_Increment(&g_custom_protocol->stats[1]);
                Trace("TRACE CT TX", g_custom_response, g_custom_length);
            }
            else
            {
                CT_Increment(&g_custom_protocol->stats[3]);
            }
        }
    }
}
