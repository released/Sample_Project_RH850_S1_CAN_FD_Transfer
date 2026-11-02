#ifndef CAN_SERVICE_H
#define CAN_SERVICE_H
#include "can_config.h"

typedef enum can_tx_event
{
    CAN_TX_DONE = 0,
    CAN_TX_ABORTED,
    CAN_TX_TIMEOUT,
    CAN_TX_UNCONFIRMED,
    CAN_TX_CANCELLED
} can_tx_event_t;

typedef struct can_diagnostics
{
    uint32_t rx_frames, rx_dropped, rx_invalid, rx_lost_events, rx_irq_entries;
    uint32_t tx_queued, tx_submitted, tx_completed, tx_aborted, tx_timeout;
    uint32_t tx_queue_full, tx_unconfirmed, errors, bus_off_events;
    uint32_t channel_status, channel_error_flags, global_error_flags;
    uint8_t rx_peak, tx_peak, bus_off, tx_fault;
    uint32_t tx_cancelled, channel_stops, recoveries, rx_discarded;
    uint8_t running;
} can_diagnostics_t;

/* Call from one foreground context. Can_Init never enables IRQs after failure.
 * Do not initialize/reconfigure the low-level controller behind this service.
 */
rs_canfd_result_t Can_Init(rs_canfd_config_report_t *report);
void Can_Stop(void);
/* Explicit, channel-local foreground operations. Recovery discards only that
 * channel's old RX data and retains its completion events/statistics. Other
 * channels and global CAN RAM/filter configuration are never reinitialized.
 */
rs_canfd_result_t Can_StopChannel(uint8_t channel);
rs_canfd_result_t Can_RecoverChannel(uint8_t channel);
/* Cancel the oldest outstanding queued/submitted frame with this tag.
 * Active cancellation waits for hardware confirmation in Can_Process(). A
 * success racing cancellation remains DONE; no unconfirmed frame is resent.
 */
rs_canfd_result_t Can_CancelTx(uint8_t channel, uint16_t tag);
void Can_Process(void);
void Can_RxIrq(void);
uint8_t Can_IsReady(void);
const char *Can_InitStage(void);
uint8_t Can_ChannelEnabled(uint8_t channel);
uint8_t Can_ChannelRunning(uint8_t channel);
rs_canfd_result_t Can_Read(uint8_t channel, rs_canfd_frame_t *frame);
rs_canfd_result_t Can_ReadLatest(uint8_t channel, rs_canfd_frame_t *frame);

/* OK means queued, not sent. One service-owned hardware TX buffer (local 0)
 * per enabled channel; application tags identify eventual completion events.
 * No automatic application resend, routing between buses, or ISP protocol.
 */
rs_canfd_result_t Can_Send(uint8_t channel, const rs_canfd_frame_t *frame, uint16_t tag);
rs_canfd_result_t Can_TakeTxEvent(uint8_t channel, uint16_t *tag, can_tx_event_t *event);
rs_canfd_result_t Can_GetDiagnostics(uint8_t channel, can_diagnostics_t *out);
const char *Can_TxEventName(can_tx_event_t event);
#endif
