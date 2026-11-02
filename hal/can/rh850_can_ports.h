#ifndef RH850_CAN_PORTS_H
#define RH850_CAN_PORTS_H
#include "can_port_config.h"
typedef struct can_port_frame
{
    unsigned long id;
    unsigned char channel, length, data[64];
} can_port_frame_t;
/* Saturated compatibility views of current service counters. irq counts
 * entries that obtained a frame for this port. api_errors counts service
 * error observations (retained level flags may be observed repeatedly).
 * last_result is the last initialization/TX outcome,not a wire fault history.
 */
typedef struct can_port_diagnostics
{
    unsigned short irq, dropped, invalid, api_errors, last_result;
    unsigned char peak;
} can_port_diagnostics_t;
/* Existing ISP API over v4. Send returns success only for a matching DONE
 * event, never just queue admission. Raw v4 queue/control APIs must not be
 * used concurrently with this foreground protocol owner.
 */
unsigned char CanPorts_Init(unsigned char fd, unsigned short request_id, unsigned char raw,
                            unsigned short raw_id, unsigned short raw_mask);
void CanPorts_Irq(void);
unsigned char CanPorts_Read(can_port_frame_t *frame);
unsigned char CanPorts_Send(unsigned char channel, unsigned short id, const unsigned char *data,
                            unsigned char length);
unsigned char CanPorts_Enabled(unsigned char channel);
unsigned char CanPorts_First(void);
void CanPorts_Diagnostics(unsigned char channel, can_port_diagnostics_t *out);
unsigned long CanPorts_Status(unsigned char channel);
unsigned long CanPorts_ErrorFlags(unsigned char channel);
void CanPorts_Stop(void);
unsigned long Device_Millis(void);
#endif
