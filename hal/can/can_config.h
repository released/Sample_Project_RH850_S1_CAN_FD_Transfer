#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H
#include "rs_canfd.h"
#include "can_user_config.h"
typedef struct can_profile
{
    uint8_t channel, mode;
    uint32_t nominal_bps, data_bps;
    uint8_t rx_mode, fifo, depth;
} can_profile_t;
/* Writable only before Can_Init, from the existing compiled protocol caller.
 * This adapter does not offer live mode/rate reconfiguration over CAN.
 */
extern can_profile_t g_can_profiles[CAN_ENABLED_COUNT];
rs_canfd_result_t CanConfig_Select(uint8_t fd, uint16_t request, uint8_t raw, uint16_t raw_id,
                                   uint16_t raw_mask);
rs_canfd_result_t CanConfig_Build(rs_canfd_config_report_t *report);
const rs_canfd_config_t *CanConfig_Get(void);
#endif
