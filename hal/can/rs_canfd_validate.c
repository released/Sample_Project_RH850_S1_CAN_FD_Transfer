#include "rs_canfd.h"

static const uint8_t gs_payload_bytes[8] = {8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};
static const uint8_t gs_fifo_depths[8] = {0U, 4U, 8U, 16U, 32U, 48U, 64U, 128U};

static rs_canfd_result_t config_error(rs_canfd_config_report_t *report,
                                      rs_canfd_config_issue_t issue, uint8_t channel, uint8_t index)
{
    if (report != 0)
    {
        report->issue = issue;
        report->channel = channel;
        report->index = index;
    }
    return RS_CANFD_ERR_PARAM;
}

void rs_canfd_frame_clear(rs_canfd_frame_t *frame)
{
    uint8_t i;
    if (frame == 0)
    {
        return;
    }
    frame->id = 0UL;
    frame->id_format = RS_CANFD_ID_STANDARD;
    frame->frame_type = RS_CANFD_FRAME_DATA;
    frame->frame_mode = RS_CANFD_FRAME_CLASSIC;
    frame->brs = 0U;
    frame->esi = 0U;
    frame->store_history = 0U;
    frame->dlc = 0xFFU;
    frame->length = 0U;
    frame->label = 0U;
    frame->timestamp = 0U;
    for (i = 0U; i < RS_CANFD_FRAME_MAX_BYTES; i++)
    {
        frame->data[i] = 0U;
    }
}

rs_canfd_result_t rs_canfd_validate_frame(const rs_canfd_frame_t *frame)
{
    uint8_t expected;
    if (frame == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }
    if ((unsigned int)frame->id_format > RS_CANFD_ID_EXTENDED ||
        (unsigned int)frame->frame_type > RS_CANFD_FRAME_REMOTE ||
        (unsigned int)frame->frame_mode > RS_CANFD_FRAME_FD || frame->brs > 1U || frame->esi > 1U ||
        frame->store_history > 1U || frame->length > 64U ||
        frame->id > ((frame->id_format == RS_CANFD_ID_STANDARD) ? 0x7FFUL : 0x1FFFFFFFUL))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (frame->frame_mode == RS_CANFD_FRAME_CLASSIC)
    {
        if (frame->length > 8U || frame->brs != 0U || frame->esi != 0U)
        {
            return RS_CANFD_ERR_PARAM;
        }
    }
    if (frame->frame_type == RS_CANFD_FRAME_REMOTE)
    {
        /* A Classical RTR has a requested DLC, but carries no data bytes. */
        return (frame->frame_mode == RS_CANFD_FRAME_CLASSIC && frame->length == 0U &&
                frame->dlc <= 8U)
                   ? RS_CANFD_OK
                   : RS_CANFD_ERR_PARAM;
    }
    expected = rs_canfd_length_to_dlc(frame->length);
    if (frame->dlc != 0xFFU)
    {
        if (frame->dlc > 15U || frame->dlc != expected ||
            rs_canfd_dlc_to_length(frame->dlc) != frame->length)
        {
            return RS_CANFD_ERR_PARAM;
        }
    }
    /* Auto DLC permits e.g. 9 data bytes, with wire length 12 and zero padding.
     * The low-level writer clears bytes beyond length before submitting TX.
     */
    return RS_CANFD_OK;
}

static uint8_t timing_valid(const rs_canfd_bit_timing_t *t, uint8_t classic)
{
    if (t->nominal_brp > 1023U || t->nominal_tseg1 > 127U || t->nominal_tseg2 > 31U ||
        t->nominal_sjw > 31U || t->nominal_sjw > t->nominal_tseg2)
    {
        return 0U;
    }
    if (classic == 0U && (t->data_tseg1 > 15U || t->data_tseg2 > 7U || t->data_sjw > 7U ||
                          t->data_sjw > t->data_tseg2))
    {
        return 0U;
    }
    return 1U;
}

rs_canfd_result_t rs_canfd_validate_config(const rs_canfd_config_t *config,
                                           rs_canfd_config_report_t *report)
{
    uint8_t i, j, k, channels, fifo_mask, owned_fifo_mask;
    uint32_t common_mask, owned_common_mask, ram;
    uint16_t rules;
    const rs_canfd_channel_config_t *ch;
    const rs_canfd_rx_fifo_config_t *fifo;
    const rs_canfd_txrx_fifo_config_t *common;
    const rs_canfd_receive_filter_t *f;

    if (report != 0)
    {
        report->issue = RS_CANFD_CONFIG_VALID;
        report->channel = 255U;
        report->index = 255U;
        report->message_ram_bytes = 0UL;
        report->rule_count = 0U;
    }
    if (config == 0 || config->unit == 0 || config->channels == 0 || config->channel_count == 0U ||
        config->channel_count > RS_CANFD_MAX_CHANNELS)
    {
        return config_error(report, RS_CANFD_CONFIG_NULL, 255U, 255U);
    }
    if (config->rx_buffer.count > RS_CANFD_RX_BUFFER_COUNT ||
        (unsigned int)config->rx_buffer.payload > 7U)
    {
        return config_error(report, RS_CANFD_CONFIG_BUFFER, 255U, 255U);
    }
    ram = (uint32_t)config->rx_buffer.count * (12UL + gs_payload_bytes[config->rx_buffer.payload]);
    rules = 0U;
    channels = 0U;
    fifo_mask = 0U;
    common_mask = 0UL;
    for (i = 0U; i < config->channel_count; i++)
    {
        ch = &config->channels[i];
        if (ch->enable > 1U || ch->channel >= RS_CANFD_MAX_CHANNELS)
        {
            return config_error(report, RS_CANFD_CONFIG_CHANNEL, ch->channel, i);
        }
        if (ch->enable == 0U)
        {
            continue;
        }
        if ((channels & (1U << ch->channel)) != 0U ||
            (unsigned int)ch->fd_mode > RS_CANFD_CHANNEL_CLASSIC_ONLY)
        {
            return config_error(report, RS_CANFD_CONFIG_CHANNEL, ch->channel, i);
        }
        channels |= (uint8_t)(1U << ch->channel);
        if (timing_valid(&ch->bit_timing,
                         (uint8_t)(ch->fd_mode == RS_CANFD_CHANNEL_CLASSIC_ONLY)) == 0U ||
            ch->tx_delay_compensation_enable > 1U || ch->tx_delay_compensation_offset > 127U)
        {
            return config_error(report, RS_CANFD_CONFIG_TIMING, ch->channel, i);
        }
        if ((ch->rx_fifo_count != 0U && ch->rx_fifos == 0) ||
            (ch->txrx_fifo_count != 0U && ch->txrx_fifos == 0) ||
            ch->rx_fifo_count > RS_CANFD_RX_FIFO_COUNT ||
            ch->txrx_fifo_count > RS_CANFD_TXRX_FIFO_COUNT)
        {
            return config_error(report, RS_CANFD_CONFIG_FIFO, ch->channel, i);
        }
        owned_fifo_mask = 0U;
        owned_common_mask = 0UL;
        for (j = 0U; j < ch->rx_fifo_count; j++)
        {
            fifo = &ch->rx_fifos[j];
            if (fifo->fifo >= RS_CANFD_RX_FIFO_COUNT || fifo->enable > 1U ||
                fifo->interrupt_enable > 1U || fifo->interrupt_at_every_frame > 1U ||
                (unsigned int)fifo->depth > 7U || (unsigned int)fifo->payload > 7U ||
                (unsigned int)fifo->interrupt_timing > 7U ||
                (fifo_mask & (1U << fifo->fifo)) != 0U ||
                (fifo->enable != 0U && fifo->depth == RS_CANFD_FIFO_DEPTH_0))
            {
                return config_error(report, RS_CANFD_CONFIG_FIFO, ch->channel, j);
            }
            fifo_mask |= (uint8_t)(1U << fifo->fifo);
            if (fifo->enable != 0U)
            {
                owned_fifo_mask |= (uint8_t)(1U << fifo->fifo);
            }
            ram += (uint32_t)gs_fifo_depths[fifo->depth] * (12UL + gs_payload_bytes[fifo->payload]);
        }
        for (j = 0U; j < ch->txrx_fifo_count; j++)
        {
            common = &ch->txrx_fifos[j];
            if (common->fifo >= RS_CANFD_TXRX_FIFO_COUNT || common->fifo / 3U != ch->channel ||
                common->enable > 1U || (unsigned int)common->depth > 7U ||
                (unsigned int)common->payload > 7U ||
                (unsigned int)common->mode > RS_CANFD_TXRX_FIFO_MODE_GATEWAY ||
                (common_mask & (1UL << common->fifo)) != 0UL ||
                (common->enable != 0U && common->depth == RS_CANFD_FIFO_DEPTH_0))
            {
                return config_error(report, RS_CANFD_CONFIG_FIFO, ch->channel, j);
            }
            common_mask |= 1UL << common->fifo;
            if (common->enable != 0U)
            {
                owned_common_mask |= 1UL << common->fifo;
            }
            ram += (uint32_t)gs_fifo_depths[common->depth] *
                   (12UL + gs_payload_bytes[common->payload]);
        }
        if (ch->receive_filter_count != 0U && ch->receive_filters == 0)
        {
            return config_error(report, RS_CANFD_CONFIG_FILTER, ch->channel, i);
        }
        rules = (uint16_t)(rules + ch->receive_filter_count);
        if (rules > RS_CANFD_MAX_RX_RULES)
        {
            return config_error(report, RS_CANFD_CONFIG_FILTER, ch->channel, i);
        }
        for (k = 0U; k < ch->receive_filter_count; k++)
        {
            f = &ch->receive_filters[k];
            if ((unsigned int)f->id_match > RS_CANFD_RECEIVE_ID_EXTENDED ||
                (unsigned int)f->frame_match > RS_CANFD_RECEIVE_FRAME_REMOTE ||
                f->minimum_dlc > 15U || f->label > 4095U || f->route_to_rx_fifo > 1U ||
                f->route_to_rx_buffer > 1U || f->route_to_txrx_fifo > 1U || f->accept_own_tx > 1U ||
                f->id > 0x1FFFFFFFUL || f->id_mask > 0x1FFFFFFFUL ||
                (f->id_match == RS_CANFD_RECEIVE_ID_STANDARD &&
                 (f->id > 0x7FFUL || f->id_mask > 0x7FFUL)) ||
                (f->route_to_rx_buffer != 0U && f->rx_buffer_index >= config->rx_buffer.count) ||
                (f->route_to_rx_fifo != 0U &&
                 (f->rx_fifo >= 8U || (owned_fifo_mask & (1U << f->rx_fifo)) == 0U)) ||
                (f->route_to_txrx_fifo != 0U &&
                 (f->txrx_fifo >= RS_CANFD_TXRX_FIFO_COUNT ||
                  (owned_common_mask & (1UL << f->txrx_fifo)) == 0UL)) ||
                (f->route_to_rx_buffer == 0U && f->route_to_rx_fifo == 0U &&
                 f->route_to_txrx_fifo == 0U))
            {
                return config_error(report, RS_CANFD_CONFIG_FILTER, ch->channel, k);
            }
        }
    }
    if (report != 0)
    {
        report->message_ram_bytes = ram;
        report->rule_count = rules;
    }
    if (channels == 0U)
    {
        return config_error(report, RS_CANFD_CONFIG_CHANNEL, 255U, 255U);
    }
    if (ram > RS_CANFD_MESSAGE_RAM_BYTES)
    {
        return config_error(report, RS_CANFD_CONFIG_RAM, 255U, 255U);
    }
    return RS_CANFD_OK;
}
