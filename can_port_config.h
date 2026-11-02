/* Project/API reading notes: Select physical MCU CAN channels.
 * Enabled ports need valid pin/transceiver routing and independent queue/filter/message-RAM
 * allocation.
 * The Tool bridge-port selection refers to the host, not to these device channel numbers.
 */
#ifndef RH850_CAN_PORT_CONFIG_H
#define RH850_CAN_PORT_CONFIG_H

/* Compile-time port selection: 0 disables a port, 1 enables it.
 * The EVM transceiver connectors use MCU CAN1 and CAN4.
 * Other ports require matching pin routing and an external transceiver.
 * Example: enable CAN0 alone with CAN0=1 and all other flags=0.
 * Boot and its App use this same configuration; it is not a Tool port index.
 */
#ifndef ENABLE_PORT_CAN0
#define ENABLE_PORT_CAN0 (0U)
#endif
#ifndef ENABLE_PORT_CAN1
#define ENABLE_PORT_CAN1 (1U)
#endif
#ifndef ENABLE_PORT_CAN2
#define ENABLE_PORT_CAN2 (0U)
#endif
#ifndef ENABLE_PORT_CAN3
#define ENABLE_PORT_CAN3 (0U)
#endif
#ifndef ENABLE_PORT_CAN4
#define ENABLE_PORT_CAN4 (1U)
#endif
#ifndef ENABLE_PORT_CAN5
#define ENABLE_PORT_CAN5 (0U)
#endif

#if ((ENABLE_PORT_CAN0 != 0) && (ENABLE_PORT_CAN0 != 1)) ||                                        \
    ((ENABLE_PORT_CAN1 != 0) && (ENABLE_PORT_CAN1 != 1)) ||                                        \
    ((ENABLE_PORT_CAN2 != 0) && (ENABLE_PORT_CAN2 != 1)) ||                                        \
    ((ENABLE_PORT_CAN3 != 0) && (ENABLE_PORT_CAN3 != 1)) ||                                        \
    ((ENABLE_PORT_CAN4 != 0) && (ENABLE_PORT_CAN4 != 1)) ||                                        \
    ((ENABLE_PORT_CAN5 != 0) && (ENABLE_PORT_CAN5 != 1))
#error CAN port flags must be 0 or 1.
#endif
#define CAN_PORT_ENABLED_COUNT                                                                     \
    (ENABLE_PORT_CAN0 + ENABLE_PORT_CAN1 + ENABLE_PORT_CAN2 + ENABLE_PORT_CAN3 +                   \
     ENABLE_PORT_CAN4 + ENABLE_PORT_CAN5)
#if CAN_PORT_ENABLED_COUNT == 0
#error Enable at least one CAN port.
#endif
#define CAN_PORT_QUEUE_SLOTS (16U)
/* RS-CANFD manual24.10.1.5: six channels provide3584*6 bytes for
 * RX buffers/RX FIFOs/common FIFOs. This backend uses only16-depth RX
 * FIFOs and reserves the worst-case64-byte payload plus12-byte header.
 * Dedicated TX buffers are a separate fixed allocation in the controller.
 */
#define CAN_PORT_HW_FIFO_DEPTH (16U)
#define CAN_PORT_HW_RX_RAM_BYTES (CAN_PORT_ENABLED_COUNT * CAN_PORT_HW_FIFO_DEPTH * (12UL + 64UL))
#if CAN_PORT_HW_RX_RAM_BYTES > (3584UL * 6UL)
#error CAN RX FIFO allocation exceeds the six-channel hardware RAM budget.
#endif
#define CAN_PORT_INVALID (255U)

#endif
