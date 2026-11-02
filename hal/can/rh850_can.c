#include <string.h>
#include "r_smc_entry.h"
#include "r_port.h"
#include "rs_canfd.h"
#include "rh850_can.h"

static volatile uint8_t g_head;
static volatile uint8_t g_tail;
static volatile device_can_frame_t g_queue[DEVICE_CAN_QUEUE_SLOTS];
static volatile device_can_diagnostics_t g_diag;
static uint8_t g_ready;

static void Increment(volatile uint16_t *value)
{
    if (*value != 0xFFFFU) { (*value)++; }
}

uint8_t Device_CanInit(uint16_t request_id)
{
    rs_canfd_receive_filter_t rule;
    rs_canfd_channel_config_t channel;
    rs_canfd_config_t config;
    rs_canfd_result_t result;
    static const rs_canfd_rx_fifo_config_t fifo =
        {0U, 1U, 1U, 1U, RS_CANFD_FIFO_INT_FULL,
         RS_CANFD_FIFO_DEPTH_16, RS_CANFD_PAYLOAD_8_BYTES};
    static const rs_canfd_bit_timing_t timing =
        RS_CANFD_BIT_TIMING_40MHZ_N500K_D2M;
    if (request_id > 0x7FFU) { return 0U; }
    g_ready = 0U;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 1U;
    g_head = 0U;
    g_tail = 0U;
    memset((void *)&g_diag, 0, sizeof(g_diag));
    R_PORT_SetAltFunc(Port0, 0U, Alt2, Output);
    R_PORT_SetAltFunc(Port0, 1U, Alt7, Input);
    result = rs_canfd_receive_filter_accept_standard_id_to_rx_fifo(
        &rule, request_id, 0x7FFU, 0U);
    if (result != RS_CANFD_OK) { return 0U; }
    rs_canfd_receive_filter_accept_data_frames(&rule);
    rs_canfd_receive_filter_set_minimum_dlc(&rule, 8U);
    memset(&channel, 0, sizeof(channel));
    channel.channel = 0U;
    channel.enable = 1U;
    channel.fd_mode = RS_CANFD_CHANNEL_CLASSIC_ONLY;
    channel.bit_timing = timing;
    channel.receive_filters = &rule;
    channel.receive_filter_count = 1U;
    channel.rx_buffer.payload = RS_CANFD_PAYLOAD_8_BYTES;
    channel.rx_fifos = &fifo;
    channel.rx_fifo_count = 1U;
    memset(&config, 0, sizeof(config));
    config.unit = &RCFDC0;
    config.global.tx_priority_by_id = 1U;
    config.channels = &channel;
    config.channel_count = 1U;
    result = rs_canfd_init(&config);
    g_diag.last_result = (uint16_t)result;
    if (result != RS_CANFD_OK) { return 0U; }
    /* Main-loop error snapshots; only the RX FIFO source is unmasked. */
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = 0U;
    INTC1.ICRCANGRECC0.BIT.TBRCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.CTRCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P3RCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P2RCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P1RCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P0RCANGRECC0 = 1U;
    g_ready = 1U;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 0U;
    return 1U;
}

#pragma interrupt rs_canfd_rx_fifo_interrupt(enable=false, channel=23, fpu=true, callt=false)
void rs_canfd_rx_fifo_interrupt(void)
{
    rs_canfd_frame_t frame;
    rs_canfd_result_t result;
    uint8_t next;
    uint8_t i;
    uint8_t guard;
    uint8_t depth;
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = 0U;
    Increment(&g_diag.irq);
    if ((RCFDC0.CFDRFSTS0.UINT32 & RS_CANFD_CFDRFSTS_RFMLT) != 0UL)
    {
        Increment(&g_diag.dropped);
    }
    for (guard = 0U; guard < 16U; guard++)
    {
        result = rs_canfd_read_rx_fifo(&RCFDC0, 0U, &frame);
        if (result == RS_CANFD_ERR_EMPTY) { break; }
        g_diag.last_result = (uint16_t)result;
        if (result != RS_CANFD_OK) { Increment(&g_diag.api_errors); break; }
        if ((frame.id_format != RS_CANFD_ID_STANDARD) ||
            (frame.frame_type != RS_CANFD_FRAME_DATA) ||
            (frame.frame_mode != RS_CANFD_FRAME_CLASSIC) ||
            (frame.brs != 0U) || (frame.dlc != 8U) || (frame.length != 8U))
        {
            Increment(&g_diag.invalid);
            continue;
        }
        next = (uint8_t)((g_head + 1U) % DEVICE_CAN_QUEUE_SLOTS);
        if (next == g_tail) { Increment(&g_diag.dropped); continue; }
        g_queue[g_head].id = frame.id;
        for (i = 0U; i < 8U; i++) { g_queue[g_head].data[i] = frame.data[i]; }
        __syncp();
        g_head = next;
        depth = (uint8_t)((g_head + DEVICE_CAN_QUEUE_SLOTS - g_tail) %
                          DEVICE_CAN_QUEUE_SLOTS);
        if (depth > g_diag.peak) { g_diag.peak = depth; }
    }
    RCFDC0.CFDRFSTS0.UINT32 =
        ~(RS_CANFD_CFDRFSTS_RFIF | RS_CANFD_CFDRFSTS_RFMLT);
    /* A bounded drain may leave frames when arrivals overlap the ISR.
     * Re-request one interrupt after a full batch, even if the last read
     * emptied the FIFO. The extra empty pass is bounded and harmless.
     * Hardware manual INTC RFxxx supports software request/re-issuing.
     */
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = (guard == 16U) ? 1U : 0U;
}

uint8_t Device_CanRead(device_can_frame_t *frame)
{
    uint8_t tail;
    uint8_t i;
    tail = g_tail;
    if (tail == g_head) { return 0U; }
    frame->id = g_queue[tail].id;
    for (i = 0U; i < 8U; i++) { frame->data[i] = g_queue[tail].data[i]; }
    __syncp();
    g_tail = (uint8_t)((tail + 1U) % DEVICE_CAN_QUEUE_SLOTS);
    return 1U;
}

uint8_t Device_CanSend(uint16_t id, const uint8_t data[8])
{
    rs_canfd_frame_t frame;
    uint32_t started;
    uint32_t guard;
    uint8_t status;
    rs_canfd_result_t result;
    if (g_ready == 0U) { return 0U; }
    memset(&frame, 0, sizeof(frame));
    frame.id = id;
    frame.dlc = 8U;
    frame.length = 8U;
    memcpy(frame.data, data, 8U);
    result = rs_canfd_send_tx_buffer(&RCFDC0, 0U, 0U, &frame);
    g_diag.last_result = (uint16_t)result;
    if (result != RS_CANFD_OK)
    {
        return 0U;
    }
    started = Device_Millis();
    guard = 2000000UL;
    while ((Device_Millis() - started < 100UL) && (guard != 0UL))
    {
        status = RS_CANFD_TX_BUFFER_TMSTS(&RCFDC0)[0];
        if ((status & RS_CANFD_CFDTMSTS_TMTSTS) == 0U)
        {
            g_diag.last_result = ((status & 6U) == 4U) ?
                RS_CANFD_OK : RS_CANFD_ERR_STATE;
            return ((status & 6U) == 4U) ? 1U : 0U;
        }
        guard--;
    }
    (void)rs_canfd_abort_tx_buffer(&RCFDC0, 0U, 0U);
    g_diag.last_result = RS_CANFD_ERR_TIMEOUT;
    return 0U;
}

void Device_CanDiagnostics(device_can_diagnostics_t *out)
{
    uint16_t mask;
    mask = INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 1U;
    *out = g_diag;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = mask;
}

void Device_CanStop(void)
{
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 1U;
    g_ready = 0U;
    (void)rs_canfd_deinit(&RCFDC0);
}

/* These named slots remain in the SC vector projection, but are masked. */
#pragma interrupt rs_canfd_global_error_interrupt(enable=false, channel=22, fpu=true, callt=false)
void rs_canfd_global_error_interrupt(void) { INTC1.ICRCANGERR0.BIT.MKRCANGERR0 = 1U; }
#pragma interrupt rs_canfd_channel0_error_interrupt(enable=false, channel=24, fpu=true, callt=false)
void rs_canfd_channel0_error_interrupt(void) { INTC1.ICRCAN0ERR.BIT.MKRCAN0ERR = 1U; }
#pragma interrupt rs_canfd_channel2_error_interrupt(enable=false, channel=217, fpu=true, callt=false)
void rs_canfd_channel2_error_interrupt(void) { INTC2.ICRCAN2ERR.BIT.MKRCAN2ERR = 1U; }
