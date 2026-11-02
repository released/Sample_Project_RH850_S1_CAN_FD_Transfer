/* Project/API reading notes: Non-destructive CAN test contract, separate from the Boot update
 * session.
 * The caller supplies complete validated frames and sends replies through the selected physical
 * CAN port.
 */
#ifndef CAN_TRANSFER_PROTOCOL_H
#define CAN_TRANSFER_PROTOCOL_H

#include "device_types.h"

#define CT_STAT_COUNT (12U)
typedef struct ct_protocol
{
    uint16_t stats[CT_STAT_COUNT];
    uint8_t cached;
    uint8_t request[64];
    uint8_t response[64];
    uint8_t cached_length;
} ct_protocol_t;

void CT_Init(ct_protocol_t *state);
uint8_t CT_Crc8(const uint8_t *data, uint8_t size);
void CT_Increment(uint16_t *value);
/* Main-loop only. Input is a validated standard Classical data frame, DLC8.
 * Returns one response; malformed CRC/magic/version/sequence get ERROR7F.
 */
void CT_Process(ct_protocol_t *state, const uint8_t request[8], uint8_t response[8]);
/* Input frame mode, BRS and wire DLC/length are checked by the IRQ adapter. */
uint8_t CT_ProcessFrame(ct_protocol_t *state, const uint8_t *request, uint8_t length,
                        uint8_t response[64]);

#endif
