/* Project/API reading notes: Foreground/ISR boundary for enabled CAN ports.
 * The ISR enqueues bounded receive data; foreground processing handles completion, cancellation
 * and diagnostics.
 * Channel-local operations preserve other active ports and their allocated message RAM.
 */
#include <string.h>
#include "can_service.h"
#include "can_board.h"

typedef struct tx_item
{
    rs_canfd_frame_t frame;
    uint16_t tag;
} tx_item_t;
typedef struct tx_event_item
{
    uint16_t tag;
    can_tx_event_t event;
} tx_event_item_t;
typedef struct port_state
{
    volatile rs_canfd_frame_t rx[CAN_RX_QUEUE_DEPTH + 1U];
    volatile uint8_t rx_head, rx_tail;
    tx_item_t tx[CAN_TX_QUEUE_DEPTH + 1U];
    tx_event_item_t events[CAN_TX_QUEUE_DEPTH + 1U];
    uint8_t tx_head, tx_tail, event_head, event_tail, credits;
    uint8_t active, abort_pending, waiting;
    uint32_t tx_start, abort_start, wait_start;
    volatile can_diagnostics_t diag;
} port_state_t;

static port_state_t gs_ports[CAN_ENABLED_COUNT];
static volatile uint8_t gs_ready;
static uint8_t gs_irq_cursor, gs_poll_cursor;
static const char *gs_init_stage = "not initialized";

static uint8_t next_index(uint8_t index, uint8_t depth)
{
    return (index >= depth) ? 0U : (uint8_t)(index + 1U);
}

static uint8_t find_port(uint8_t channel)
{
    uint8_t i;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (g_can_profiles[i].channel == channel)
        {
            return i;
        }
    }
    return 255U;
}

uint8_t Can_IsReady(void)
{
    return gs_ready;
}
const char *Can_InitStage(void)
{
    return gs_init_stage;
}
uint8_t Can_ChannelEnabled(uint8_t channel)
{
    return (uint8_t)(find_port(channel) != 255U);
}
uint8_t Can_ChannelRunning(uint8_t channel)
{
    uint8_t i;
    i = find_port(channel);
    return (uint8_t)(gs_ready != 0U && i != 255U && gs_ports[i].diag.running != 0U);
}

rs_canfd_result_t Can_Init(rs_canfd_config_report_t *report)
{
    rs_canfd_result_t result;
    uint8_t i, has_irq;
    if (gs_ready != 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    gs_init_stage = "configuration";
    result = CanConfig_Build(report);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    gs_init_stage = "board pins/clock";
    result = CanBoard_Prepare();
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    memset(gs_ports, 0, sizeof(gs_ports));
    gs_irq_cursor = 0U;
    gs_poll_cursor = 0U;
    gs_init_stage = "controller initialization";
    result = CanBoard_Start(CanConfig_Get());
    if (result != RS_CANFD_OK)
    {
        CanBoard_EnableRxIrq(0U);
        (void)CanBoard_Stop();
        return result;
    }
    has_irq = 0U;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        gs_ports[i].diag.running = 1U;
        if (g_can_profiles[i].rx_mode == CAN_RX_INTERRUPT)
        {
            has_irq = 1U;
        }
    }
    gs_ready = 1U;
    gs_init_stage = "ready";
    CanBoard_EnableRxIrq(has_irq);
    return RS_CANFD_OK;
}

static void emit_tx(port_state_t *p, uint16_t tag, can_tx_event_t event)
{
    tx_event_item_t *out;
    out = &p->events[p->event_head];
    out->tag = tag;
    out->event = event;
    p->event_head = next_index(p->event_head, CAN_TX_QUEUE_DEPTH);
    if (event == CAN_TX_DONE)
    {
        p->diag.tx_completed++;
    }
    else if (event == CAN_TX_ABORTED)
    {
        p->diag.tx_aborted++;
    }
    else if (event == CAN_TX_TIMEOUT)
    {
        p->diag.tx_timeout++;
    }
    else if (event == CAN_TX_UNCONFIRMED)
    {
        p->diag.tx_unconfirmed++;
    }
    else if (event == CAN_TX_CANCELLED)
    {
        p->diag.tx_cancelled++;
    }
}

static void finish_tx(port_state_t *p, can_tx_event_t event)
{
    emit_tx(p, p->tx[p->tx_tail].tag, event);
    p->tx_tail = next_index(p->tx_tail, CAN_TX_QUEUE_DEPTH);
    p->active = 0U;
    p->abort_pending = 0U;
    p->waiting = 0U;
}

rs_canfd_result_t Can_StopChannel(uint8_t channel)
{
    uint8_t i;
    uint32_t saved;
    port_state_t *p;
    rs_canfd_result_t result;
    rs_canfd_tx_result_t tx;
    i = find_port(channel);
    if (i == 255U)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (gs_ready == 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    p = &gs_ports[i];
    saved = CanBoard_Lock();
    p->diag.running = 0U;
    CanBoard_Unlock(saved);
    if (p->active != 0U && CanBoard_TxResult(channel, &tx) == RS_CANFD_OK)
    {
        if (tx == RS_CANFD_TX_COMPLETE)
        {
            finish_tx(p, CAN_TX_DONE);
        }
        else if (tx == RS_CANFD_TX_ABORTED)
        {
            finish_tx(p, CAN_TX_ABORTED);
        }
    }
    /* Keep the timer/shared RX IRQ running during bounded hardware waits. */
    result = CanBoard_StopChannel(channel);
    if (p->active != 0U)
    {
        finish_tx(p, CAN_TX_UNCONFIRMED);
    }
    while (p->tx_head != p->tx_tail)
    {
        finish_tx(p, CAN_TX_CANCELLED);
    }
    p->diag.channel_stops++;
    if (result != RS_CANFD_OK)
    {
        p->diag.tx_fault = 1U;
    }
    return result;
}

rs_canfd_result_t Can_RecoverChannel(uint8_t channel)
{
    uint8_t i;
    uint32_t saved;
    port_state_t *p;
    rs_canfd_result_t result;
    i = find_port(channel);
    if (i == 255U)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (gs_ready == 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    result = Can_StopChannel(channel);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    p = &gs_ports[i];
    saved = CanBoard_Lock();
    while (p->rx_tail != p->rx_head)
    {
        p->rx_tail = next_index(p->rx_tail, CAN_RX_QUEUE_DEPTH);
        p->diag.rx_discarded++;
    }
    CanBoard_Unlock(saved);
    result = CanBoard_StartChannel(channel);
    if (result != RS_CANFD_OK)
    {
        p->diag.tx_fault = 1U;
        return result;
    }
    saved = CanBoard_Lock();
    p->diag.tx_fault = 0U;
    p->diag.bus_off = 0U;
    p->diag.running = 1U;
    p->diag.recoveries++;
    CanBoard_Unlock(saved);
    CanBoard_AckRxIrq(0U);
    return RS_CANFD_OK;
}

rs_canfd_result_t Can_CancelTx(uint8_t channel, uint16_t tag)
{
    uint8_t i, at, next, last;
    port_state_t *p;
    rs_canfd_result_t result;
    i = find_port(channel);
    if (i == 255U)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (gs_ready == 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    p = &gs_ports[i];
    at = p->tx_tail;
    while (at != p->tx_head && p->tx[at].tag != tag)
    {
        at = next_index(at, CAN_TX_QUEUE_DEPTH);
    }
    if (at == p->tx_head)
    {
        return RS_CANFD_ERR_EMPTY;
    }
    if (at == p->tx_tail && p->active != 0U)
    {
        if (p->abort_pending != 0U)
        {
            return (p->abort_pending == 2U) ? RS_CANFD_OK : RS_CANFD_ERR_BUSY;
        }
        result = CanBoard_Abort(channel);
        if (result != RS_CANFD_OK)
        {
            return result;
        }
        p->abort_pending = 2U;
        p->abort_start = CanBoard_Millis();
        return RS_CANFD_OK;
    }
    emit_tx(p, tag, CAN_TX_CANCELLED);
    if (at == p->tx_tail)
    {
        p->waiting = 0U;
    }
    last = (p->tx_head == 0U) ? CAN_TX_QUEUE_DEPTH : (uint8_t)(p->tx_head - 1U);
    while (at != last)
    {
        next = next_index(at, CAN_TX_QUEUE_DEPTH);
        p->tx[at] = p->tx[next];
        at = next;
    }
    p->tx_head = last;
    return RS_CANFD_OK;
}

void Can_Stop(void)
{
    uint8_t i;
    port_state_t *p;
    rs_canfd_result_t result;
    CanBoard_EnableRxIrq(0U);
    gs_ready = 0U;
    result = CanBoard_Stop();
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        p = &gs_ports[i];
        p->diag.running = 0U;
        if (p->active != 0U && p->tx_head != p->tx_tail)
        {
            finish_tx(p, CAN_TX_UNCONFIRMED);
        }
        while (p->tx_head != p->tx_tail)
        {
            finish_tx(p, CAN_TX_CANCELLED);
        }
        if (result != RS_CANFD_OK)
        {
            p->diag.tx_fault = 1U;
        }
    }
    gs_init_stage = (result == RS_CANFD_OK) ? "stopped" : "stop not confirmed";
}

static void drain_rx(uint8_t mode, uint8_t budget, uint8_t *cursor)
{
    uint8_t i, n, empty, slot, next, used, handled;
    port_state_t *p;
    rs_canfd_frame_t frame;
    rs_canfd_result_t result;
    if (gs_ready == 0U)
    {
        return;
    }
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (gs_ports[i].diag.running != 0U && g_can_profiles[i].rx_mode == mode &&
            CanBoard_TakeFifoLost(g_can_profiles[i].fifo) != 0U)
        {
            gs_ports[i].diag.rx_lost_events++;
        }
    }
    n = 0U;
    empty = 0U;
    handled = 0U;
    while (n < budget && empty < CAN_ENABLED_COUNT)
    {
        slot = *cursor;
        *cursor = (uint8_t)((slot + 1U) % CAN_ENABLED_COUNT);
        if (g_can_profiles[slot].rx_mode != mode || gs_ports[slot].diag.running == 0U)
        {
            empty++;
            continue;
        }
        result = CanBoard_ReadFifo(g_can_profiles[slot].fifo, &frame);
        if (result != RS_CANFD_OK)
        {
            if (result != RS_CANFD_ERR_EMPTY)
            {
                gs_ports[slot].diag.rx_invalid++;
            }
            empty++;
            continue;
        }
        empty = 0U;
        n++;
        p = &gs_ports[slot];
        if (mode == CAN_RX_INTERRUPT && (handled & (1U << slot)) == 0U)
        {
            p->diag.rx_irq_entries++;
            handled |= (uint8_t)(1U << slot);
        }
        /* RX carries the actual DLC. Remote frames have requested DLC but no
         * data. Auto-DLC transmit padding has already become wire data here.
         */
        if (rs_canfd_validate_frame(&frame) != RS_CANFD_OK)
        {
            p->diag.rx_invalid++;
            continue;
        }
        p->diag.rx_frames++;
        next = next_index(p->rx_head, CAN_RX_QUEUE_DEPTH);
        if (next == p->rx_tail)
        {
            p->diag.rx_dropped++;
            continue;
        }
        p->rx[p->rx_head] = frame;
        RS_CANFD_SYNC();
        p->rx_head = next;
        used = (p->rx_head >= p->rx_tail)
                   ? (uint8_t)(p->rx_head - p->rx_tail)
                   : (uint8_t)(CAN_RX_QUEUE_DEPTH + 1U + p->rx_head - p->rx_tail);
        if (used > p->diag.rx_peak)
        {
            p->diag.rx_peak = used;
        }
    }
    if (mode == CAN_RX_INTERRUPT)
    {
        CanBoard_AckRxIrq((uint8_t)(n == budget));
    }
}

void Can_RxIrq(void)
{
    if (gs_ready != 0U)
    {
        drain_rx(CAN_RX_INTERRUPT, CAN_RX_IRQ_BUDGET, &gs_irq_cursor);
    }
    else
    {
        CanBoard_EnableRxIrq(0U);
    }
}

rs_canfd_result_t Can_Read(uint8_t channel, rs_canfd_frame_t *frame)
{
    uint8_t i, tail;
    port_state_t *p;
    i = find_port(channel);
    if (i == 255U || frame == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (gs_ready == 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    p = &gs_ports[i];
    tail = p->rx_tail;
    if (tail == p->rx_head)
    {
        return RS_CANFD_ERR_EMPTY;
    }
    /* Single producer/consumer: ISR cannot reuse the occupied slot until the
     * consumer publishes its new tail. Volatile storage plus syncp preserves
     * this ordering without disabling IRQs for the whole payload copy.
     */
    RS_CANFD_SYNC();
    *frame = p->rx[tail];
    RS_CANFD_SYNC();
    p->rx_tail = next_index(tail, CAN_RX_QUEUE_DEPTH);
    return RS_CANFD_OK;
}

rs_canfd_result_t Can_ReadLatest(uint8_t channel, rs_canfd_frame_t *frame)
{
    if (find_port(channel) == 255U || frame == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (gs_ready == 0U || CAN_RX_BUFFER_SHADOW == 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    return CanBoard_ReadLatest(channel, frame);
}

rs_canfd_result_t Can_Send(uint8_t channel, const rs_canfd_frame_t *frame, uint16_t tag)
{
    uint8_t i;
    port_state_t *p;
    i = find_port(channel);
    if (i == 255U || rs_canfd_validate_frame(frame) != RS_CANFD_OK)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if ((g_can_profiles[i].mode == CAN_MODE_CLASSIC &&
         frame->frame_mode != RS_CANFD_FRAME_CLASSIC) ||
        (g_can_profiles[i].mode == CAN_MODE_FD && frame->frame_mode != RS_CANFD_FRAME_FD))
    {
        return RS_CANFD_ERR_PARAM;
    }
    p = &gs_ports[i];
    if (gs_ready == 0U || p->diag.running == 0U || p->diag.tx_fault != 0U || p->diag.bus_off != 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    /* Reserve event capacity too: completions are never silently overwritten.
     * Call Can_TakeTxEvent regularly to release the corresponding credits.
     */
    if (p->credits >= CAN_TX_QUEUE_DEPTH)
    {
        p->diag.tx_queue_full++;
        return RS_CANFD_ERR_FULL;
    }
    p->tx[p->tx_head].frame = *frame;
    p->tx[p->tx_head].tag = tag;
    p->tx_head = next_index(p->tx_head, CAN_TX_QUEUE_DEPTH);
    p->credits++;
    p->diag.tx_queued++;
    if (p->credits > p->diag.tx_peak)
    {
        p->diag.tx_peak = p->credits;
    }
    return RS_CANFD_OK;
}

static void process_tx(uint8_t i)
{
    port_state_t *p;
    rs_canfd_tx_result_t tx;
    rs_canfd_result_t result;
    uint32_t now;
    p = &gs_ports[i];
    now = CanBoard_Millis();
    if (p->tx_head == p->tx_tail || p->diag.tx_fault != 0U || p->diag.running == 0U)
    {
        return;
    }
    if (p->active != 0U)
    {
        result = CanBoard_TxResult(g_can_profiles[i].channel, &tx);
        if (result != RS_CANFD_OK)
        {
            p->diag.tx_fault = 1U;
            finish_tx(p, CAN_TX_UNCONFIRMED);
            return;
        }
        if (tx == RS_CANFD_TX_COMPLETE)
        {
            finish_tx(p, CAN_TX_DONE);
            return;
        }
        if (tx == RS_CANFD_TX_ABORTED)
        {
            finish_tx(p, (p->abort_pending == 2U)
                             ? CAN_TX_CANCELLED
                             : ((p->abort_pending == 1U) ? CAN_TX_TIMEOUT : CAN_TX_ABORTED));
            return;
        }
        if (p->abort_pending == 0U && (uint32_t)(now - p->tx_start) >= CAN_TX_TIMEOUT_MS)
        {
            result = CanBoard_Abort(g_can_profiles[i].channel);
            if (result != RS_CANFD_OK)
            {
                p->diag.tx_fault = 1U;
                finish_tx(p, CAN_TX_UNCONFIRMED);
                return;
            }
            p->abort_pending = 1U;
            p->abort_start = now;
        }
        else if (p->abort_pending != 0U && (uint32_t)(now - p->abort_start) >= CAN_TX_TIMEOUT_MS)
        {
            p->diag.tx_fault = 1U;
            finish_tx(p, CAN_TX_UNCONFIRMED);
        }
        return;
    }
    if (p->diag.bus_off != 0U)
    {
        return;
    }
    result = CanBoard_Send(g_can_profiles[i].channel, &p->tx[p->tx_tail].frame);
    if (result == RS_CANFD_OK)
    {
        p->active = 1U;
        p->waiting = 0U;
        p->tx_start = now;
        p->diag.tx_submitted++;
    }
    else if (result == RS_CANFD_ERR_BUSY)
    {
        if (p->waiting == 0U)
        {
            p->waiting = 1U;
            p->wait_start = now;
        }
        else if ((uint32_t)(now - p->wait_start) >= CAN_TX_TIMEOUT_MS)
        {
            p->diag.tx_fault = 1U;
            finish_tx(p, CAN_TX_UNCONFIRMED);
        }
    }
    else
    {
        p->diag.errors++;
        finish_tx(p, CAN_TX_CANCELLED);
    }
}

void Can_Process(void)
{
    uint8_t i, bus_off;
    rs_canfd_status_t status;
    port_state_t *p;
    uint32_t global_errors;
    if (gs_ready == 0U)
    {
        return;
    }
    drain_rx(CAN_RX_POLLING, CAN_RX_POLL_BUDGET, &gs_poll_cursor);
    global_errors = 0UL;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        p = &gs_ports[i];
        if (CanBoard_Status(g_can_profiles[i].channel, &status) == RS_CANFD_OK)
        {
            p->diag.channel_status = status.channel_status;
            bus_off = (uint8_t)((status.channel_status & RS_CANFD_CFDCCSTS_BOSTS) != 0UL);
            if (bus_off != 0U && p->diag.bus_off == 0U)
            {
                p->diag.bus_off_events++;
            }
            p->diag.bus_off = bus_off;
            if (status.channel_error_flags != 0UL || status.global_error_flags != 0UL)
            {
                p->diag.errors++;
                p->diag.channel_error_flags |= status.channel_error_flags;
                p->diag.global_error_flags |= status.global_error_flags;
                global_errors |= status.global_error_flags;
                CanBoard_ClearErrors(g_can_profiles[i].channel, 0UL, status.channel_error_flags);
            }
        }
        else
        {
            p->diag.errors++;
        }
        process_tx(i);
    }
    /* Global flags belong to the controller; retain the same snapshot for all
     * channel diagnostics before clearing the observed bits once.
     */
    if (global_errors != 0UL)
    {
        CanBoard_ClearErrors(g_can_profiles[0].channel, global_errors, 0UL);
    }
}

rs_canfd_result_t Can_TakeTxEvent(uint8_t channel, uint16_t *tag, can_tx_event_t *event)
{
    uint8_t i;
    port_state_t *p;
    i = find_port(channel);
    if (i == 255U || tag == 0 || event == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }
    p = &gs_ports[i];
    if (p->event_head == p->event_tail)
    {
        return RS_CANFD_ERR_EMPTY;
    }
    *tag = p->events[p->event_tail].tag;
    *event = p->events[p->event_tail].event;
    p->event_tail = next_index(p->event_tail, CAN_TX_QUEUE_DEPTH);
    p->credits--;
    return RS_CANFD_OK;
}

rs_canfd_result_t Can_GetDiagnostics(uint8_t channel, can_diagnostics_t *out)
{
    uint8_t i;
    uint32_t saved;
    i = find_port(channel);
    if (i == 255U || out == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }
    saved = CanBoard_Lock();
    *out = gs_ports[i].diag;
    CanBoard_Unlock(saved);
    return RS_CANFD_OK;
}

const char *Can_TxEventName(can_tx_event_t event)
{
    switch (event)
    {
    case CAN_TX_DONE:
        return "TX complete";
    case CAN_TX_ABORTED:
        return "TX aborted";
    case CAN_TX_TIMEOUT:
        return "TX timeout / abort confirmed";
    case CAN_TX_UNCONFIRMED:
        return "TX unconfirmed / stopped";
    case CAN_TX_CANCELLED:
        return "TX cancelled";
    default:
        return "unknown TX event";
    }
}
