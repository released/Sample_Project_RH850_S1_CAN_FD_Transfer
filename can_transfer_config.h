#ifndef CAN_TRANSFER_CONFIG_H
#define CAN_TRANSFER_CONFIG_H

/* EVM R7F701684: X1=16 MHz, CPU=120 MHz, fCAN=40 MHz.
 * can_port_config.h selects MCU CAN channels; default ports are CAN1/CAN4.
 * Keep physical timing and the Tool's selected nominal rate consistent.
 */
#define CAN_TRANSFER_REQUEST_ID (0x710U)
#define CAN_TRANSFER_RESPONSE_ID (0x711U)
#define CAN_TRANSFER_TARGET_ID (0x8501U)
#define CAN_TRANSFER_UART_TRACE (1U)
#define CAN_TRANSFER_PART_SHORT "R7F70168"
/* Custom Transfer v1 has an eight-character identification field.
 * The EVM device is R7F701684; 8501 identifies this profile.
 */

#endif
