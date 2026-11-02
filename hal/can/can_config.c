/* Project/API reading notes: CAN profiles and message-RAM allocation.
 * Only enabled physical channels receive queue/filter resources.
 * Timing values describe controller registers; metadata masks advertise those compiled values and
 * do not reconfigure a remote device.
 */
#include <string.h>
#include "can_config.h"

can_profile_t g_can_profiles[CAN_ENABLED_COUNT];
static rs_canfd_channel_config_t g_channels[CAN_ENABLED_COUNT];
static rs_canfd_rx_fifo_config_t g_fifos[CAN_ENABLED_COUNT];
static rs_canfd_receive_filter_t g_filters[CAN_ENABLED_COUNT][2];
static rs_canfd_config_t g_config;
static uint8_t g_fd, g_raw, g_selected;
static uint16_t g_request, g_raw_id, g_raw_mask;
static const uint8_t g_numbers[CAN_ENABLED_COUNT] = {
#if ENABLE_PORT_CAN0
    0U,
#endif
#if ENABLE_PORT_CAN1
    1U,
#endif
#if ENABLE_PORT_CAN2
    2U,
#endif
#if ENABLE_PORT_CAN3
    3U,
#endif
#if ENABLE_PORT_CAN4
    4U,
#endif
#if ENABLE_PORT_CAN5
    5U,
#endif
};
rs_canfd_result_t CanConfig_Select(uint8_t fd, uint16_t request, uint8_t raw, uint16_t raw_id,
                                   uint16_t raw_mask)
{
    uint8_t i;
    g_selected = 0U;
    if (fd > 1U || raw > 1U || request > 0x7FFU ||
        (raw != 0U && (raw_id > 0x7FFU || raw_mask == 0U || raw_mask > 0x7FFU)))
    {
        return RS_CANFD_ERR_PARAM;
    }
    g_fd = fd;
    g_raw = raw;
    g_request = request;
    g_raw_id = raw_id;
    g_raw_mask = raw_mask;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        g_can_profiles[i].channel = g_numbers[i];
        g_can_profiles[i].mode = fd ? CAN_MODE_FD : CAN_MODE_CLASSIC;
        g_can_profiles[i].nominal_bps = 500000UL;
        g_can_profiles[i].data_bps = fd ? 2000000UL : 0UL;
        g_can_profiles[i].rx_mode = CAN_RX_INTERRUPT;
        g_can_profiles[i].fifo = g_numbers[i];
        g_can_profiles[i].depth = CAN_PORT_HW_FIFO_DEPTH;
    }
    g_selected = 1U;
    return RS_CANFD_OK;
}
rs_canfd_result_t CanConfig_Build(rs_canfd_config_report_t *report)
{
    static const rs_canfd_bit_timing_t timing = RS_CANFD_BIT_TIMING_40MHZ_N500K_D2M;
    uint8_t i;
    rs_canfd_result_t result;
    if (report != 0)
    {
        memset(report, 0, sizeof(*report));
    }
    if (g_selected == 0U)
    {
        return RS_CANFD_ERR_STATE;
    }
    memset(&g_config, 0, sizeof(g_config));
    memset(g_channels, 0, sizeof(g_channels));
    memset(g_fifos, 0, sizeof(g_fifos));
    g_config.unit = &RCFDC0;
    g_config.global.tx_priority_by_id = 1U;
    g_config.rx_buffer.payload = RS_CANFD_PAYLOAD_8_BYTES;
    g_config.channels = g_channels;
    g_config.channel_count = CAN_ENABLED_COUNT;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        g_fifos[i].fifo = g_can_profiles[i].fifo;
        g_fifos[i].enable = 1U;
        g_fifos[i].interrupt_enable = 1U;
        g_fifos[i].interrupt_at_every_frame = 1U;
        g_fifos[i].interrupt_timing = RS_CANFD_FIFO_INT_FULL;
        g_fifos[i].depth = RS_CANFD_FIFO_DEPTH_16;
        g_fifos[i].payload = g_fd ? RS_CANFD_PAYLOAD_64_BYTES : RS_CANFD_PAYLOAD_8_BYTES;
        g_channels[i].channel = g_can_profiles[i].channel;
        g_channels[i].enable = 1U;
        g_channels[i].fd_mode = g_fd ? RS_CANFD_CHANNEL_FD_ONLY : RS_CANFD_CHANNEL_CLASSIC_ONLY;
        g_channels[i].bit_timing = timing;
        result = rs_canfd_receive_filter_accept_standard_id_to_rx_fifo(&g_filters[i][0], g_request,
                                                                       0x7FFU, g_fifos[i].fifo);
        if (result != RS_CANFD_OK)
        {
            return result;
        }
        rs_canfd_receive_filter_accept_data_frames(&g_filters[i][0]);
        rs_canfd_receive_filter_set_minimum_dlc(&g_filters[i][0], 1U);
        if (g_raw != 0U)
        {
            result = rs_canfd_receive_filter_accept_standard_id_to_rx_fifo(
                &g_filters[i][1], g_raw_id, g_raw_mask, g_fifos[i].fifo);
            if (result != RS_CANFD_OK)
            {
                return result;
            }
            rs_canfd_receive_filter_accept_data_frames(&g_filters[i][1]);
            rs_canfd_receive_filter_set_minimum_dlc(&g_filters[i][1], 1U);
        }
        g_channels[i].receive_filters = g_filters[i];
        g_channels[i].receive_filter_count = g_raw ? 2U : 1U;
        g_channels[i].rx_fifos = &g_fifos[i];
        g_channels[i].rx_fifo_count = 1U;
    }
    return rs_canfd_validate_config(&g_config, report);
}
const rs_canfd_config_t *CanConfig_Get(void)
{
    return &g_config;
}
