#ifndef CAN_USER_CONFIG_H
#define CAN_USER_CONFIG_H
#include "can_port_config.h"

/* ISP uses the v4 service with IRQ-owned FIFOs only. Port selection remains
 * in can_port_config.h. Preserve the existing 16-slot/15-usable RX capacity.
 * TX queue credits include undrained completion events. No heap is used.
 */
#define CAN_ENABLED_COUNT CAN_PORT_ENABLED_COUNT
#define CAN_ENABLED_MASK                                                                           \
    ((ENABLE_PORT_CAN0 << 0) | (ENABLE_PORT_CAN1 << 1) | (ENABLE_PORT_CAN2 << 2) |                 \
     (ENABLE_PORT_CAN3 << 3) | (ENABLE_PORT_CAN4 << 4) | (ENABLE_PORT_CAN5 << 5))
#define CAN_RX_POLLING (0U)
#define CAN_RX_INTERRUPT (1U)
#define CAN_MODE_CLASSIC (0U)
#define CAN_MODE_FD (1U)
#define CAN_MODE_MIXED (2U)
#define CAN_RX_QUEUE_DEPTH (CAN_PORT_QUEUE_SLOTS - 1U)
/* ISP has one synchronous reply outstanding. Four completion credits retain
 * optional foreground controls while fitting all six ports below the E2 RAM
 * reservation; the shared v4 service implementation is unchanged.
 */
#define CAN_TX_QUEUE_DEPTH (4U)
#define CAN_RX_IRQ_BUDGET (16U)
#define CAN_RX_POLL_BUDGET (16U)
#define CAN_RX_BUFFER_SHADOW (0U)
#define CAN_TX_TIMEOUT_MS (100UL)
#define CAN_INIT_TIMEOUT_MS (100UL)
#if CAN_RX_QUEUE_DEPTH < 1 || CAN_RX_QUEUE_DEPTH > 254 || CAN_TX_QUEUE_DEPTH < 1 ||                \
    CAN_TX_QUEUE_DEPTH > 254
#error CAN queue capacities must be1..254.
#endif
#endif
