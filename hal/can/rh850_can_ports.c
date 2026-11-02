/* Project/API reading notes: ISP adapter over the CAN service.
 * Preserve the physical channel on receive/reply, drain enabled-port queues fairly and wait for
 * the matching TX completion tag.
 * Per-port diagnostics do not grant ownership of the Boot update workspace.
 */
#include <string.h>
#include "can_service.h"
#include "can_board.h"
#include "rh850_can_ports.h"

static unsigned char g_fd, g_cursor;
static unsigned short g_tag;
static unsigned short g_last[CAN_ENABLED_COUNT], g_invalid[CAN_ENABLED_COUNT];
static unsigned char Index(unsigned char channel)
{
    unsigned char i;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (g_can_profiles[i].channel == channel)
        {
            return i;
        }
    }
    return CAN_PORT_INVALID;
}
static unsigned short Saturate(uint32_t value)
{
    return value > 65535UL ? 65535U : (unsigned short)value;
}
unsigned char CanPorts_Enabled(unsigned char channel)
{
    return (unsigned char)(channel < 6U && (CAN_ENABLED_MASK & (1U << channel)) != 0U);
}
unsigned char CanPorts_First(void)
{
#if ENABLE_PORT_CAN0
    return 0U;
#elif ENABLE_PORT_CAN1
    return 1U;
#elif ENABLE_PORT_CAN2
    return 2U;
#elif ENABLE_PORT_CAN3
    return 3U;
#elif ENABLE_PORT_CAN4
    return 4U;
#else
    return 5U;
#endif
}
/* Open the selected compiled CAN profiles and initialize per-port adapter state.
 * Timing/filters and message-RAM allocation are checked before use.
 */
unsigned char CanPorts_Init(unsigned char fd, unsigned short request_id, unsigned char raw,
                            unsigned short raw_id, unsigned short raw_mask)
{
    rs_canfd_config_report_t report;
    rs_canfd_result_t result;
    unsigned char i;
    if (Can_IsReady())
    {
        return 0U;
    }
    result = CanConfig_Select(fd, request_id, raw, raw_id, raw_mask);
    if (result == RS_CANFD_OK)
    {
        result = Can_Init(&report);
    }
    g_fd = fd;
    g_cursor = 0U;
    g_tag = 0U;
    memset(g_invalid, 0, sizeof(g_invalid));
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        g_last[i] = (unsigned short)result;
    }
    return (unsigned char)(result == RS_CANFD_OK);
}
/* Run the bounded CAN receive-queue ISR path.
 * No protocol decoding, Flash calls, UART formatting or TX wait belongs here.
 */
void CanPorts_Irq(void)
{
    Can_RxIrq();
}
/* Service foreground completion/diagnostics, then return one queued frame with its physical
 * source channel for protocol ownership/reply routing.
 */
unsigned char CanPorts_Read(can_port_frame_t *frame)
{
    rs_canfd_frame_t received;
    rs_canfd_result_t result;
    unsigned char i, tries;
    if (frame == 0 || !Can_IsReady())
    {
        return 0U;
    }
    Can_Process();
    for (tries = 0U; tries < CAN_ENABLED_COUNT; tries++)
    {
        i = g_cursor;
        g_cursor = (unsigned char)((g_cursor + 1U) % CAN_ENABLED_COUNT);
        result = Can_Read(g_can_profiles[i].channel, &received);
        if (result != RS_CANFD_OK)
        {
            continue;
        }
        if (received.id_format != RS_CANFD_ID_STANDARD ||
            received.frame_type != RS_CANFD_FRAME_DATA ||
            received.frame_mode != (g_fd ? RS_CANFD_FRAME_FD : RS_CANFD_FRAME_CLASSIC) ||
            received.brs != (g_fd ? 1U : 0U) || received.length == 0U ||
            received.length > (g_fd ? 64U : 8U) ||
            received.length != rs_canfd_dlc_to_length(received.dlc))
        {
            if (g_invalid[i] != 65535U)
            {
                g_invalid[i]++;
            }
            continue;
        }
        frame->id = received.id;
        frame->channel = g_can_profiles[i].channel;
        frame->length = received.length;
        memcpy(frame->data, received.data, received.length);
        return 1U;
    }
    return 0U;
}
/* Send on the specified physical channel and accept success only for the matching completed TX
 * tag; queue insertion alone is not delivery.
 */
unsigned char CanPorts_Send(unsigned char channel, unsigned short id, const unsigned char *data,
                            unsigned char length)
{
    rs_canfd_frame_t frame;
    rs_canfd_result_t result;
    can_tx_event_t event;
    unsigned short tag;
    unsigned char i;
    unsigned long started, guard;
    i = Index(channel);
    if (!Can_IsReady() || i == CAN_PORT_INVALID || data == 0 || id > 0x7FFU || length == 0U ||
        (length > 8U && (!g_fd || length != 64U)))
    {
        return 0U;
    }
    rs_canfd_frame_clear(&frame);
    frame.id = id;
    frame.length = length;
    frame.frame_mode = g_fd ? RS_CANFD_FRAME_FD : RS_CANFD_FRAME_CLASSIC;
    frame.brs = g_fd ? 1U : 0U;
    memcpy(frame.data, data, length);
    g_tag++;
    result = Can_Send(channel, &frame, g_tag);
    if (result != RS_CANFD_OK)
    {
        g_last[i] = (unsigned short)result;
        return 0U;
    }
    started = CanBoard_Millis();
    guard = 2000000UL;
    while (guard != 0UL && (uint32_t)(CanBoard_Millis() - started) < 2UL * CAN_TX_TIMEOUT_MS + 20UL)
    {
        Can_Process();
        if (Can_TakeTxEvent(channel, &tag, &event) == RS_CANFD_OK)
        {
            if (tag == g_tag && event == CAN_TX_DONE)
            {
                g_last[i] = RS_CANFD_OK;
                return 1U;
            }
            if (tag != g_tag)
            {
                (void)Can_StopChannel(channel);
                while (Can_TakeTxEvent(channel, &tag, &event) == RS_CANFD_OK)
                {
                }
            }
            g_last[i] = (unsigned short)((event == CAN_TX_TIMEOUT) ? RS_CANFD_ERR_TIMEOUT
                                                                   : RS_CANFD_ERR_STATE);
            return 0U;
        }
        guard--;
    }
    /* Broken tick/no terminal result: stop just this channel; never reuse an
     * unconfirmed TX buffer or restart/replay the ISP transaction implicitly.
     */
    (void)Can_StopChannel(channel);
    while (Can_TakeTxEvent(channel, &tag, &event) == RS_CANFD_OK)
    {
    }
    g_last[i] = RS_CANFD_ERR_TIMEOUT;
    return 0U;
}
void CanPorts_Diagnostics(unsigned char channel, can_port_diagnostics_t *out)
{
    can_diagnostics_t diagnostic;
    unsigned char i;
    if (out == 0)
    {
        return;
    }
    memset(out, 0, sizeof(*out));
    i = Index(channel);
    if (i == CAN_PORT_INVALID || Can_GetDiagnostics(channel, &diagnostic) != RS_CANFD_OK)
    {
        return;
    }
    out->irq = Saturate(diagnostic.rx_irq_entries);
    out->dropped =
        Saturate((uint32_t)Saturate(diagnostic.rx_dropped) + Saturate(diagnostic.rx_lost_events));
    out->invalid = Saturate((uint32_t)Saturate(diagnostic.rx_invalid) + g_invalid[i]);
    out->api_errors = Saturate(diagnostic.errors);
    out->last_result = g_last[i];
    out->peak = diagnostic.rx_peak;
}
unsigned long CanPorts_Status(unsigned char channel)
{
    rs_canfd_status_t status;
    if (!Can_ChannelEnabled(channel) || CanBoard_Status(channel, &status) != RS_CANFD_OK)
    {
        return 0UL;
    }
    return status.channel_status;
}
unsigned long CanPorts_ErrorFlags(unsigned char channel)
{
    rs_canfd_status_t status;
    if (!Can_ChannelEnabled(channel) || CanBoard_Status(channel, &status) != RS_CANFD_OK)
    {
        return 0UL;
    }
    return status.channel_error_flags;
}
/* Stop only an opened CAN service.
 * The cold valid-App handoff can reach this path before CAN clocks/registers were enabled.
 */
void CanPorts_Stop(void)
{
    /* The valid-image Boot path may hand off without ever opening CAN.
     * Do not enter register waits while that controller is still clock-gated.
     * A running service still owns its normal stop and pending-TX cleanup.
     */
    if (Can_IsReady())
    {
        Can_Stop();
    }
}
