#ifndef RH850_CAN_H
#define RH850_CAN_H

#include "device_types.h"

/* Single RX interrupt producer, main-loop consumer; one slot remains empty. */
#define DEVICE_CAN_QUEUE_SLOTS (16U)
typedef struct device_can_frame
{
    uint32_t id;
    uint8_t data[8];
} device_can_frame_t;
typedef struct device_can_diagnostics
{
    uint16_t irq;
    uint16_t dropped;
    uint16_t invalid;
    uint16_t api_errors;
    uint16_t last_result;
    uint8_t peak;
} device_can_diagnostics_t;

uint8_t Device_CanInit(uint16_t request_id);
uint8_t Device_CanRead(device_can_frame_t *frame);
uint8_t Device_CanSend(uint16_t id, const uint8_t data[8]);
void Device_CanDiagnostics(device_can_diagnostics_t *out);
void Device_CanStop(void);
uint32_t Device_Millis(void);

#endif
