#include "rs_canfd.h"

static const uint8_t gs_rs_canfd_dlc_to_length[16] = {0U, 1U,  2U,  3U,  4U,  5U,  6U,  7U,
                                                      8U, 12U, 16U, 20U, 24U, 32U, 48U, 64U};

/* Board defaults live in can_config.c, not in this reusable driver. */
static rs_canfd_millis_t gs_millis;
static uint32_t gs_timeout_ms = 100UL;

void rs_canfd_set_time_source(rs_canfd_millis_t source, uint32_t timeout_ms)
{
    gs_millis = source;
    gs_timeout_ms = timeout_ms;
}

static uint32_t rs_canfd_replace_field(uint32_t reg, uint8_t pos, uint8_t width, uint32_t value)
{
    uint32_t mask;

    mask = RS_CANFD_FIELD_MASK(pos, width);
    reg &= (uint32_t)(~mask);
    reg |= RS_CANFD_FIELD_PREP(pos, width, value);
    return reg;
}

static rs_canfd_result_t rs_canfd_wait_mask(volatile rs_canfd_reg32_t *reg, uint32_t mask,
                                            uint32_t expected, uint32_t timeout)
{
    uint32_t guard;
    uint32_t value;
    uint32_t start;

    guard = timeout;
    start = (gs_millis != 0) ? gs_millis() : 0UL;
    while (guard > 0UL)
    {
        value = reg->UINT32 & mask;
        if (value == expected)
        {
            return RS_CANFD_OK;
        }
        guard--;
        if ((gs_millis != 0) && ((uint32_t)(gs_millis() - start) >= gs_timeout_ms))
        {
            break;
        }
    }

    return RS_CANFD_ERR_TIMEOUT;
}

static rs_canfd_result_t rs_canfd_validate_channel(uint8_t channel)
{
    if (channel >= RS_CANFD_MAX_CHANNELS)
    {
        return RS_CANFD_ERR_PARAM;
    }

    return RS_CANFD_OK;
}

void rs_canfd_receive_filter_clear(rs_canfd_receive_filter_t *filter)
{
    if (filter == 0)
    {
        return;
    }

    filter->id_match = RS_CANFD_RECEIVE_ID_ANY;
    filter->id = 0UL;
    filter->id_mask = 0UL;
    filter->frame_match = RS_CANFD_RECEIVE_FRAME_ANY;
    filter->minimum_dlc = 0U;
    filter->accept_own_tx = 0U;
    filter->route_to_rx_buffer = 0U;
    filter->rx_buffer_index = 0U;
    filter->route_to_rx_fifo = 0U;
    filter->rx_fifo = 0U;
    filter->route_to_txrx_fifo = 0U;
    filter->txrx_fifo = 0U;
    filter->label = 0U;
}

rs_canfd_result_t rs_canfd_receive_filter_accept_any_to_rx_fifo(rs_canfd_receive_filter_t *filter,
                                                                uint8_t fifo)
{
    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * User-facing recipe for one GAFL rule: accept every ID/frame type and put
     * the received frame into the selected RCFDCnCFDRFSTSx/RFCCx RX FIFO.
     */
    rs_canfd_receive_filter_clear(filter);
    rs_canfd_receive_filter_accept_all(filter);
    return rs_canfd_receive_filter_route_to_rx_fifo(filter, fifo);
}

rs_canfd_result_t
rs_canfd_receive_filter_accept_any_to_rx_fifo_and_buffer(rs_canfd_receive_filter_t *filter,
                                                         uint8_t fifo, uint8_t buffer)
{
    rs_canfd_result_t result;

    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT) || (buffer >= RS_CANFD_RX_BUFFER_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * Bring-up-friendly recipe for one GAFL rule: accept every ID/frame type,
     * enqueue it in an RX FIFO, and keep the latest copy in one receive buffer.
     */
    rs_canfd_receive_filter_clear(filter);
    rs_canfd_receive_filter_accept_all(filter);
    result = rs_canfd_receive_filter_route_to_rx_fifo(filter, fifo);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    return rs_canfd_receive_filter_route_to_rx_buffer(filter, buffer);
}

rs_canfd_result_t
rs_canfd_receive_filter_accept_standard_id_to_rx_fifo(rs_canfd_receive_filter_t *filter,
                                                      uint16_t id, uint16_t id_mask, uint8_t fifo)
{
    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * User-facing recipe for one GAFL rule: compare an 11-bit standard ID,
     * ignore ID bits where id_mask is 0, and route the frame to an RX FIFO.
     */
    rs_canfd_receive_filter_clear(filter);
    rs_canfd_receive_filter_accept_standard_id(filter, id, id_mask);
    return rs_canfd_receive_filter_route_to_rx_fifo(filter, fifo);
}

rs_canfd_result_t rs_canfd_receive_filter_accept_standard_id_to_rx_fifo_and_buffer(
    rs_canfd_receive_filter_t *filter, uint16_t id, uint16_t id_mask, uint8_t fifo, uint8_t buffer)
{
    rs_canfd_result_t result;

    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT) || (buffer >= RS_CANFD_RX_BUFFER_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * Compare a standard ID, enqueue matching frames in an RX FIFO, and keep
     * the latest matching frame in a receive buffer for polling diagnostics.
     */
    rs_canfd_receive_filter_clear(filter);
    rs_canfd_receive_filter_accept_standard_id(filter, id, id_mask);
    result = rs_canfd_receive_filter_route_to_rx_fifo(filter, fifo);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    return rs_canfd_receive_filter_route_to_rx_buffer(filter, buffer);
}

rs_canfd_result_t
rs_canfd_receive_filter_accept_extended_id_to_rx_fifo(rs_canfd_receive_filter_t *filter,
                                                      uint32_t id, uint32_t id_mask, uint8_t fifo)
{
    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * User-facing recipe for one GAFL rule: compare a 29-bit extended ID,
     * ignore ID bits where id_mask is 0, and route the frame to an RX FIFO.
     */
    rs_canfd_receive_filter_clear(filter);
    rs_canfd_receive_filter_accept_extended_id(filter, id, id_mask);
    return rs_canfd_receive_filter_route_to_rx_fifo(filter, fifo);
}

rs_canfd_result_t rs_canfd_receive_filter_accept_extended_id_to_rx_fifo_and_buffer(
    rs_canfd_receive_filter_t *filter, uint32_t id, uint32_t id_mask, uint8_t fifo, uint8_t buffer)
{
    rs_canfd_result_t result;

    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT) || (buffer >= RS_CANFD_RX_BUFFER_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * Compare an extended ID, enqueue matching frames in an RX FIFO, and keep
     * the latest matching frame in a receive buffer for polling diagnostics.
     */
    rs_canfd_receive_filter_clear(filter);
    rs_canfd_receive_filter_accept_extended_id(filter, id, id_mask);
    result = rs_canfd_receive_filter_route_to_rx_fifo(filter, fifo);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    return rs_canfd_receive_filter_route_to_rx_buffer(filter, buffer);
}

void rs_canfd_receive_filter_accept_all(rs_canfd_receive_filter_t *filter)
{
    if (filter == 0)
    {
        return;
    }

    filter->id_match = RS_CANFD_RECEIVE_ID_ANY;
    filter->id = 0UL;
    filter->id_mask = 0UL;
    filter->frame_match = RS_CANFD_RECEIVE_FRAME_ANY;
}

void rs_canfd_receive_filter_accept_standard_id(rs_canfd_receive_filter_t *filter, uint16_t id,
                                                uint16_t id_mask)
{
    if (filter == 0)
    {
        return;
    }

    filter->id_match = RS_CANFD_RECEIVE_ID_STANDARD;
    filter->id = (uint32_t)(id & 0x07FFU);
    filter->id_mask = (uint32_t)(id_mask & 0x07FFU);
}

void rs_canfd_receive_filter_accept_extended_id(rs_canfd_receive_filter_t *filter, uint32_t id,
                                                uint32_t id_mask)
{
    if (filter == 0)
    {
        return;
    }

    filter->id_match = RS_CANFD_RECEIVE_ID_EXTENDED;
    filter->id = id & 0x1FFFFFFFUL;
    filter->id_mask = id_mask & 0x1FFFFFFFUL;
}

void rs_canfd_receive_filter_accept_data_frames(rs_canfd_receive_filter_t *filter)
{
    if (filter != 0)
    {
        filter->frame_match = RS_CANFD_RECEIVE_FRAME_DATA;
    }
}

void rs_canfd_receive_filter_accept_remote_frames(rs_canfd_receive_filter_t *filter)
{
    if (filter != 0)
    {
        filter->frame_match = RS_CANFD_RECEIVE_FRAME_REMOTE;
    }
}

void rs_canfd_receive_filter_accept_data_or_remote(rs_canfd_receive_filter_t *filter)
{
    if (filter != 0)
    {
        filter->frame_match = RS_CANFD_RECEIVE_FRAME_ANY;
    }
}

void rs_canfd_receive_filter_set_minimum_dlc(rs_canfd_receive_filter_t *filter, uint8_t minimum_dlc)
{
    if (filter != 0)
    {
        filter->minimum_dlc = (uint8_t)(minimum_dlc & 0x0FU);
    }
}

void rs_canfd_receive_filter_accept_own_tx(rs_canfd_receive_filter_t *filter, uint8_t enable)
{
    if (filter != 0)
    {
        filter->accept_own_tx = (enable != 0U) ? 1U : 0U;
    }
}

rs_canfd_result_t rs_canfd_receive_filter_route_to_rx_fifo(rs_canfd_receive_filter_t *filter,
                                                           uint8_t fifo)
{
    if ((filter == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    filter->route_to_rx_fifo = 1U;
    filter->rx_fifo = fifo;
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_receive_filter_route_to_rx_buffer(rs_canfd_receive_filter_t *filter,
                                                             uint8_t buffer)
{
    if ((filter == 0) || (buffer >= RS_CANFD_RX_BUFFER_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    filter->route_to_rx_buffer = 1U;
    filter->rx_buffer_index = buffer;
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_receive_filter_route_to_txrx_fifo(rs_canfd_receive_filter_t *filter,
                                                             uint8_t fifo)
{
    if ((filter == 0) || (fifo >= RS_CANFD_TXRX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    filter->route_to_txrx_fifo = 1U;
    filter->txrx_fifo = fifo;
    return RS_CANFD_OK;
}

void rs_canfd_receive_filter_set_label(rs_canfd_receive_filter_t *filter, uint16_t label)
{
    if (filter != 0)
    {
        filter->label = label;
    }
}

static rs_canfd_result_t rs_canfd_receive_filter_to_rule(const rs_canfd_receive_filter_t *filter,
                                                         rs_canfd_rx_rule_t *rule)
{
    if ((filter == 0) || (rule == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    if ((filter->route_to_rx_buffer == 0U) && (filter->route_to_rx_fifo == 0U) &&
        (filter->route_to_txrx_fifo == 0U))
    {
        return RS_CANFD_ERR_PARAM;
    }

    rule->id = filter->id & 0x1FFFFFFFUL;
    rule->id_mask = filter->id_mask & 0x1FFFFFFFUL;
    rule->ide = 0U;
    rule->ide_compare = 0U;
    rule->rtr = 0U;
    rule->rtr_compare = 0U;
    rule->local_loopback = filter->accept_own_tx;
    rule->minimum_dlc = (uint8_t)(filter->minimum_dlc & 0x0FU);
    rule->use_receive_buffer = 0U;
    rule->receive_buffer_index = 0U;
    rule->label = filter->label;
    rule->rx_fifo_mask = 0U;
    rule->txrx_fifo_mask = 0UL;

    if (filter->id_match == RS_CANFD_RECEIVE_ID_STANDARD)
    {
        rule->id &= 0x07FFUL;
        rule->id_mask &= 0x07FFUL;
        rule->ide = 0U;
        rule->ide_compare = 1U;
    }
    else if (filter->id_match == RS_CANFD_RECEIVE_ID_EXTENDED)
    {
        rule->ide = 1U;
        rule->ide_compare = 1U;
    }
    else
    {
        rule->id = 0UL;
        rule->id_mask = 0UL;
    }

    if (filter->frame_match == RS_CANFD_RECEIVE_FRAME_DATA)
    {
        rule->rtr = 0U;
        rule->rtr_compare = 1U;
    }
    else if (filter->frame_match == RS_CANFD_RECEIVE_FRAME_REMOTE)
    {
        rule->rtr = 1U;
        rule->rtr_compare = 1U;
    }
    else
    {
        rule->rtr = 0U;
        rule->rtr_compare = 0U;
    }

    if (filter->route_to_rx_buffer != 0U)
    {
        if (filter->rx_buffer_index >= RS_CANFD_RX_BUFFER_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }
        rule->use_receive_buffer = 1U;
        rule->receive_buffer_index = filter->rx_buffer_index;
    }

    if (filter->route_to_rx_fifo != 0U)
    {
        if (filter->rx_fifo >= RS_CANFD_RX_FIFO_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }
        rule->rx_fifo_mask = (uint8_t)(1U << filter->rx_fifo);
    }

    if (filter->route_to_txrx_fifo != 0U)
    {
        if (filter->txrx_fifo >= RS_CANFD_TXRX_FIFO_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }
        rule->txrx_fifo_mask = (uint32_t)(1UL << filter->txrx_fifo);
    }

    return RS_CANFD_OK;
}

static rs_canfd_result_t rs_canfd_write_receive_rule_entry(volatile rs_canfd_unit_t *unit,
                                                           uint16_t table_index,
                                                           const rs_canfd_rx_rule_t *rule)
{
    volatile rs_canfd_rx_rule_regs_t *rule_regs;
    uint8_t selected_page;
    uint8_t slot;
    uint32_t id_word;
    uint32_t mask_word;
    uint32_t ptr0_word;
    uint32_t ptr1_word;

    if ((unit == 0) || (rule == 0) || (table_index >= RS_CANFD_MAX_RX_RULES))
    {
        return RS_CANFD_ERR_PARAM;
    }

    rule_regs = RS_CANFD_RX_RULE_REGS(unit);
    selected_page = (uint8_t)(table_index / RS_CANFD_RX_RULE_PAGE_SIZE);
    slot = (uint8_t)(table_index % RS_CANFD_RX_RULE_PAGE_SIZE);
    unit->CFDGAFLECTR.UINT32 = RS_CANFD_CFDGAFLECTR_AFLDAE |
                               RS_CANFD_FIELD_PREP(RS_CANFD_CFDGAFLECTR_AFLPN_POS,
                                                   RS_CANFD_CFDGAFLECTR_AFLPN_WIDTH, selected_page);

    id_word = rule->id &
              RS_CANFD_FIELD_MASK(RS_CANFD_CFDGAFLID_GAFLID_POS, RS_CANFD_CFDGAFLID_GAFLID_WIDTH);
    if (rule->local_loopback != 0U)
    {
        id_word |= RS_CANFD_CFDGAFLID_GAFLLB;
    }
    if (rule->rtr != 0U)
    {
        id_word |= RS_CANFD_CFDGAFLID_GAFLRTR;
    }
    if (rule->ide != 0U)
    {
        id_word |= RS_CANFD_CFDGAFLID_GAFLIDE;
    }

    mask_word = rule->id_mask &
                RS_CANFD_FIELD_MASK(RS_CANFD_CFDGAFLM_GAFLIDM_POS, RS_CANFD_CFDGAFLM_GAFLIDM_WIDTH);
    if (rule->rtr_compare != 0U)
    {
        mask_word |= RS_CANFD_CFDGAFLM_GAFLRTRM;
    }
    if (rule->ide_compare != 0U)
    {
        mask_word |= RS_CANFD_CFDGAFLM_GAFLIDEM;
    }

    ptr0_word = RS_CANFD_FIELD_PREP(RS_CANFD_CFDGAFLP0_GAFLDLC_POS,
                                    RS_CANFD_CFDGAFLP0_GAFLDLC_WIDTH, rule->minimum_dlc);
    if (rule->use_receive_buffer != 0U)
    {
        ptr0_word |= RS_CANFD_CFDGAFLP0_GAFLRMV;
        ptr0_word |=
            RS_CANFD_FIELD_PREP(RS_CANFD_CFDGAFLP0_GAFLRMDP_POS, RS_CANFD_CFDGAFLP0_GAFLRMDP_WIDTH,
                                rule->receive_buffer_index);
    }
    ptr0_word |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDGAFLP0_GAFLPTR_POS,
                                     RS_CANFD_CFDGAFLP0_GAFLPTR_WIDTH, rule->label);

    ptr1_word = RS_CANFD_FIELD_PREP(RS_CANFD_CFDGAFLP1_GAFLFDP_RX_POS,
                                    RS_CANFD_CFDGAFLP1_GAFLFDP_RX_WIDTH, rule->rx_fifo_mask);
    ptr1_word |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDGAFLP1_GAFLFDP_CF_POS,
                                     RS_CANFD_CFDGAFLP1_GAFLFDP_CF_WIDTH, rule->txrx_fifo_mask);

    rule_regs[slot].id.UINT32 = id_word;
    rule_regs[slot].mask.UINT32 = mask_word;
    rule_regs[slot].ptr0.UINT32 = ptr0_word;
    rule_regs[slot].ptr1.UINT32 = ptr1_word;

    return RS_CANFD_OK;
}

static uint8_t rs_canfd_rule_count_get(volatile rs_canfd_unit_t *unit, uint8_t channel)
{
    uint32_t reg;
    uint8_t shift;
    uint8_t count;

    if (channel < 4U)
    {
        reg = unit->CFDGAFLCFG0.UINT32;
    }
    else
    {
#if RS_CANFD_MAX_CHANNELS > 4
        reg = unit->CFDGAFLCFG1.UINT32;
#else
        reg = 0UL;
#endif
    }

    shift = (uint8_t)((3U - (channel & 0x03U)) * 8U);
    count = (uint8_t)RS_CANFD_FIELD_GET(reg, shift, RS_CANFD_CFDGAFLCFG_RNC_WIDTH);
    return count;
}

static void rs_canfd_rule_count_set(volatile rs_canfd_unit_t *unit, uint8_t channel, uint8_t count)
{
    uint32_t reg;
    uint8_t shift;

    shift = (uint8_t)((3U - (channel & 0x03U)) * 8U);
    if (channel < 4U)
    {
        reg = unit->CFDGAFLCFG0.UINT32;
        reg = rs_canfd_replace_field(reg, shift, RS_CANFD_CFDGAFLCFG_RNC_WIDTH, count);
        unit->CFDGAFLCFG0.UINT32 = reg;
    }
    else
    {
#if RS_CANFD_MAX_CHANNELS > 4
        reg = unit->CFDGAFLCFG1.UINT32;
        reg = rs_canfd_replace_field(reg, shift, RS_CANFD_CFDGAFLCFG_RNC_WIDTH, count);
        unit->CFDGAFLCFG1.UINT32 = reg;
#endif
    }
}

static uint16_t rs_canfd_rule_start_index_get(volatile rs_canfd_unit_t *unit, uint8_t channel)
{
    uint8_t ch;
    uint16_t start_index;

    start_index = 0U;
    for (ch = 0U; ch < channel; ch++)
    {
        start_index = (uint16_t)(start_index + rs_canfd_rule_count_get(unit, ch));
    }

    return start_index;
}

static uint32_t rs_canfd_frame_id_word(const rs_canfd_frame_t *frame)
{
    uint32_t id_word;

    id_word = frame->id & RS_CANFD_FIELD_MASK(RS_CANFD_CFD_MSG_ID_POS, RS_CANFD_CFD_MSG_ID_WIDTH);
    if (frame->frame_type == RS_CANFD_FRAME_REMOTE)
    {
        id_word |= RS_CANFD_CFD_MSG_RTR;
    }
    if (frame->id_format == RS_CANFD_ID_EXTENDED)
    {
        id_word |= RS_CANFD_CFD_MSG_IDE;
    }

    return id_word;
}

static void rs_canfd_data_to_regs(volatile rs_canfd_reg32_t *data_regs, const uint8_t *data,
                                  uint8_t length)
{
    uint8_t word_index;
    uint8_t byte_index;
    uint8_t offset;
    uint32_t value;

    for (word_index = 0U; word_index < 16U; word_index++)
    {
        value = 0UL;
        for (byte_index = 0U; byte_index < 4U; byte_index++)
        {
            offset = (uint8_t)((word_index * 4U) + byte_index);
            if (offset < length)
            {
                value |= ((uint32_t)data[offset] << (byte_index * 8U));
            }
        }
        data_regs[word_index].UINT32 = value;
    }
}

static void rs_canfd_regs_to_data(uint8_t *data, volatile rs_canfd_reg32_t *data_regs,
                                  uint8_t length)
{
    uint8_t word_index;
    uint8_t byte_index;
    uint8_t offset;
    uint32_t value;

    for (word_index = 0U; word_index < 16U; word_index++)
    {
        value = data_regs[word_index].UINT32;
        for (byte_index = 0U; byte_index < 4U; byte_index++)
        {
            offset = (uint8_t)((word_index * 4U) + byte_index);
            if (offset < length)
            {
                data[offset] = (uint8_t)((value >> (byte_index * 8U)) & 0xFFUL);
            }
        }
    }
}

static rs_canfd_result_t rs_canfd_write_tx_buffer_regs(volatile rs_canfd_tx_buffer_regs_t *tx,
                                                       const rs_canfd_frame_t *frame)
{
    uint8_t dlc;
    uint8_t length;
    uint32_t ptr_word;
    uint32_t fd_word;
    uint32_t id_word;

    if ((tx == 0) || (rs_canfd_validate_frame(frame) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }

    length = frame->length;
    if (length > RS_CANFD_FRAME_MAX_BYTES)
    {
        return RS_CANFD_ERR_PARAM;
    }

    dlc = frame->dlc;
    if (dlc > 15U)
    {
        dlc = rs_canfd_length_to_dlc(length);
    }

    id_word = rs_canfd_frame_id_word(frame);
    if (frame->store_history != 0U)
    {
        id_word |= RS_CANFD_CFDTMID_THLEN;
    }

    ptr_word = RS_CANFD_FIELD_PREP(RS_CANFD_CFDTMPTR_TMPTR_POS, RS_CANFD_CFDTMPTR_TMPTR_WIDTH,
                                   frame->label);
    ptr_word |=
        RS_CANFD_FIELD_PREP(RS_CANFD_CFDTMPTR_TMDLC_POS, RS_CANFD_CFDTMPTR_TMDLC_WIDTH, dlc);

    fd_word = 0UL;
    if (frame->frame_mode == RS_CANFD_FRAME_FD)
    {
        fd_word |= RS_CANFD_CFDTMFDCTR_TMFDF;
    }
    if (frame->brs != 0U)
    {
        fd_word |= RS_CANFD_CFDTMFDCTR_TMBRS;
    }
    if (frame->esi != 0U)
    {
        fd_word |= RS_CANFD_CFDTMFDCTR_TMESI;
    }

    tx->id.UINT32 = id_word;
    tx->ptr.UINT32 = ptr_word;
    tx->fdctr.UINT32 = fd_word;
    rs_canfd_data_to_regs(&tx->data[0], frame->data, length);

    return RS_CANFD_OK;
}

static rs_canfd_result_t rs_canfd_write_fifo_regs(volatile rs_canfd_message_ram_regs_t *fifo,
                                                  const rs_canfd_frame_t *frame)
{
    uint8_t dlc;
    uint8_t length;
    uint32_t ptr_word;
    uint32_t fd_word;

    if ((fifo == 0) || (rs_canfd_validate_frame(frame) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }

    length = frame->length;
    if (length > RS_CANFD_FRAME_MAX_BYTES)
    {
        return RS_CANFD_ERR_PARAM;
    }

    dlc = frame->dlc;
    if (dlc > 15U)
    {
        dlc = rs_canfd_length_to_dlc(length);
    }

    ptr_word = RS_CANFD_FIELD_PREP(RS_CANFD_CFD_MSG_PTR_LABEL_POS, RS_CANFD_CFD_MSG_PTR_LABEL_WIDTH,
                                   frame->label);
    ptr_word |=
        RS_CANFD_FIELD_PREP(RS_CANFD_CFD_MSG_PTR_DLC_POS, RS_CANFD_CFD_MSG_PTR_DLC_WIDTH, dlc);

    fd_word = 0UL;
    if (frame->frame_mode == RS_CANFD_FRAME_FD)
    {
        fd_word |= RS_CANFD_CFD_MSG_FDSTS_FDF;
    }
    if (frame->brs != 0U)
    {
        fd_word |= RS_CANFD_CFD_MSG_FDSTS_BRS;
    }
    if (frame->esi != 0U)
    {
        fd_word |= RS_CANFD_CFD_MSG_FDSTS_ESI;
    }

    fifo->id.UINT32 = rs_canfd_frame_id_word(frame);
    fifo->ptr.UINT32 = ptr_word;
    fifo->fdsts.UINT32 = fd_word;
    rs_canfd_data_to_regs(&fifo->data[0], frame->data, length);

    return RS_CANFD_OK;
}

static rs_canfd_result_t rs_canfd_read_message_regs(volatile rs_canfd_message_ram_regs_t *message,
                                                    rs_canfd_frame_t *frame)
{
    uint32_t id_word;
    uint32_t ptr_word;
    uint32_t fd_word;
    uint8_t dlc;

    if ((message == 0) || (frame == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    id_word = message->id.UINT32;
    ptr_word = message->ptr.UINT32;
    fd_word = message->fdsts.UINT32;
    dlc = (uint8_t)RS_CANFD_FIELD_GET(ptr_word, RS_CANFD_CFD_MSG_PTR_DLC_POS,
                                      RS_CANFD_CFD_MSG_PTR_DLC_WIDTH);

    frame->id = RS_CANFD_FIELD_GET(id_word, RS_CANFD_CFD_MSG_ID_POS, RS_CANFD_CFD_MSG_ID_WIDTH);
    frame->id_format =
        ((id_word & RS_CANFD_CFD_MSG_IDE) != 0UL) ? RS_CANFD_ID_EXTENDED : RS_CANFD_ID_STANDARD;
    frame->frame_type =
        ((id_word & RS_CANFD_CFD_MSG_RTR) != 0UL) ? RS_CANFD_FRAME_REMOTE : RS_CANFD_FRAME_DATA;
    frame->frame_mode = ((fd_word & RS_CANFD_CFD_MSG_FDSTS_FDF) != 0UL) ? RS_CANFD_FRAME_FD
                                                                        : RS_CANFD_FRAME_CLASSIC;
    frame->brs = ((fd_word & RS_CANFD_CFD_MSG_FDSTS_BRS) != 0UL) ? 1U : 0U;
    frame->esi = ((fd_word & RS_CANFD_CFD_MSG_FDSTS_ESI) != 0UL) ? 1U : 0U;
    frame->store_history = 0U;
    frame->dlc = dlc;
    frame->length = rs_canfd_dlc_to_length(dlc);
    if (frame->frame_type == RS_CANFD_FRAME_REMOTE)
    {
        frame->length = 0U;
    }
    frame->timestamp = (uint16_t)RS_CANFD_FIELD_GET(ptr_word, RS_CANFD_CFD_MSG_PTR_TS_POS,
                                                    RS_CANFD_CFD_MSG_PTR_TS_WIDTH);
    frame->label = (uint16_t)RS_CANFD_FIELD_GET(fd_word, RS_CANFD_CFD_MSG_FDSTS_PTR_POS,
                                                RS_CANFD_CFD_MSG_FDSTS_PTR_WIDTH);
    rs_canfd_regs_to_data(frame->data, &message->data[0], frame->length);

    return RS_CANFD_OK;
}

static void rs_canfd_configure_global(volatile rs_canfd_unit_t *unit,
                                      const rs_canfd_global_config_t *global)
{
    uint32_t reg;

    reg = 0UL;
    if (global->tx_priority_by_id != 0U)
    {
        reg |= RS_CANFD_CFDGCFG_TPRI;
    }
    if (global->dlc_check_enable != 0U)
    {
        reg |= RS_CANFD_CFDGCFG_DCE;
    }
    if (global->dlc_replace_enable != 0U)
    {
        reg |= RS_CANFD_CFDGCFG_DRE;
    }
    if (global->mirror_mode_enable != 0U)
    {
        reg |= RS_CANFD_CFDGCFG_MME;
    }
    if (global->use_canfd_clock != 0U)
    {
        reg |= RS_CANFD_CFDGCFG_DCS;
    }
    if (global->timestamp_source != 0U)
    {
        reg |= RS_CANFD_CFDGCFG_TSSS;
    }
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDGCFG_TSP_POS, RS_CANFD_CFDGCFG_TSP_WIDTH,
                               global->timestamp_prescaler);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDGCFG_TSBTCS_POS, RS_CANFD_CFDGCFG_TSBTCS_WIDTH,
                               global->timestamp_clock_source);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDGCFG_ITRCP_POS, RS_CANFD_CFDGCFG_ITRCP_WIDTH,
                               global->interval_timer_prescaler);
    unit->CFDGCFG.UINT32 = reg;
}

static void rs_canfd_configure_channel_mode_bits(volatile rs_canfd_unit_t *unit,
                                                 const rs_canfd_channel_config_t *channel_config)
{
    volatile rs_canfd_channel_regs_t *ch_regs;
    volatile rs_canfd_channel_fd_regs_t *fd_regs;
    uint8_t channel;
    uint32_t reg;

    ch_regs = RS_CANFD_CHANNEL_REGS(unit);
    fd_regs = RS_CANFD_CHANNEL_FD_REGS(unit);
    channel = channel_config->channel;

    reg = 0UL;
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCNCFG_NBRP_POS, RS_CANFD_CFDCNCFG_NBRP_WIDTH,
                               channel_config->bit_timing.nominal_brp);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCNCFG_NSJW_POS, RS_CANFD_CFDCNCFG_NSJW_WIDTH,
                               channel_config->bit_timing.nominal_sjw);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCNCFG_NTSEG1_POS, RS_CANFD_CFDCNCFG_NTSEG1_WIDTH,
                               channel_config->bit_timing.nominal_tseg1);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCNCFG_NTSEG2_POS, RS_CANFD_CFDCNCFG_NTSEG2_WIDTH,
                               channel_config->bit_timing.nominal_tseg2);
    ch_regs[channel].ncfg.UINT32 = reg;

    reg = 0UL;
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCDCFG_DBRP_POS, RS_CANFD_CFDCDCFG_DBRP_WIDTH,
                               channel_config->bit_timing.data_brp);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCDCFG_DTSEG1_POS, RS_CANFD_CFDCDCFG_DTSEG1_WIDTH,
                               channel_config->bit_timing.data_tseg1);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCDCFG_DTSEG2_POS, RS_CANFD_CFDCDCFG_DTSEG2_WIDTH,
                               channel_config->bit_timing.data_tseg2);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCDCFG_DSJW_POS, RS_CANFD_CFDCDCFG_DSJW_WIDTH,
                               channel_config->bit_timing.data_sjw);
    fd_regs[channel].dcfg.UINT32 = reg;

    reg = RS_CANFD_CFDCFDCFG_REFE;
    if (channel_config->fd_mode == RS_CANFD_CHANNEL_FD_ONLY)
    {
        reg |= RS_CANFD_CFDCFDCFG_FDOE;
    }
    else if (channel_config->fd_mode == RS_CANFD_CHANNEL_CLASSIC_ONLY)
    {
        reg |= RS_CANFD_CFDCFDCFG_FDOE;
        reg |= RS_CANFD_CFDCFDCFG_CLOE;
    }
    else
    {
        /* Mixed mode: FDOE = 0, CLOE = 0. */
    }

    if (channel_config->tx_delay_compensation_enable != 0U)
    {
        reg |= RS_CANFD_CFDCFDCFG_TDCE;
        reg |= RS_CANFD_CFDCFDCFG_TDCOC;
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFDCFG_TDCO_POS, RS_CANFD_CFDCFDCFG_TDCO_WIDTH,
                                   channel_config->tx_delay_compensation_offset);
    }

    fd_regs[channel].fdcfg.UINT32 = reg;
}

static rs_canfd_result_t
rs_canfd_enable_runtime_blocks(volatile rs_canfd_unit_t *unit,
                               const rs_canfd_channel_config_t *channel_config)
{
    volatile rs_canfd_reg32_t *rxcc;
    volatile rs_canfd_reg32_t *cfcc;
    volatile rs_canfd_reg32_t *txqcc;
    volatile rs_canfd_reg32_t *thlcc;
    const rs_canfd_rx_fifo_config_t *rx_fifo;
    const rs_canfd_txrx_fifo_config_t *txrx_fifo;
    uint8_t index;
    uint8_t fifo;
    uint8_t channel;

    if ((unit == 0) || (channel_config == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    rxcc = RS_CANFD_RX_FIFO_CC(unit);
    cfcc = RS_CANFD_TXRX_FIFO_CC(unit);
    txqcc = RS_CANFD_TX_QUEUE_CC(unit);
    thlcc = RS_CANFD_TX_HISTORY_CC(unit);
    channel = channel_config->channel;

    for (index = 0U; index < channel_config->rx_fifo_count; index++)
    {
        rx_fifo = &channel_config->rx_fifos[index];
        fifo = rx_fifo->fifo;
        if (fifo >= RS_CANFD_RX_FIFO_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }
        if (rx_fifo->enable != 0U)
        {
            rxcc[fifo].UINT32 |= RS_CANFD_CFDRFCC_RFE;
        }
    }

    for (index = 0U; index < channel_config->txrx_fifo_count; index++)
    {
        txrx_fifo = &channel_config->txrx_fifos[index];
        fifo = txrx_fifo->fifo;
        if (fifo >= RS_CANFD_TXRX_FIFO_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }
        if (txrx_fifo->enable != 0U)
        {
            cfcc[fifo].UINT32 |= RS_CANFD_CFDCFCC_CFE;
        }
    }

    if (channel >= RS_CANFD_TX_QUEUE_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }

    if (channel_config->tx_queue.enable != 0U)
    {
        txqcc[channel].UINT32 |= RS_CANFD_CFDTXQCC_TXQE;
    }
    if (channel_config->tx_history.enable != 0U)
    {
        thlcc[channel].UINT32 |= RS_CANFD_CFDTHLCC_THLE;
    }

    return RS_CANFD_OK;
}

static rs_canfd_result_t rs_canfd_configure_all_rule_counts(const rs_canfd_config_t *config)
{
    volatile rs_canfd_unit_t *unit;
    const rs_canfd_channel_config_t *ch_config;
    uint8_t index;

    unit = config->unit;
    unit->CFDGAFLCFG0.UINT32 = 0UL;
#if RS_CANFD_MAX_CHANNELS > 4
    unit->CFDGAFLCFG1.UINT32 = 0UL;
#endif

    for (index = 0U; index < config->channel_count; index++)
    {
        ch_config = &config->channels[index];
        if (ch_config->enable != 0U)
        {
            if (rs_canfd_validate_channel(ch_config->channel) != RS_CANFD_OK)
            {
                return RS_CANFD_ERR_PARAM;
            }
            rs_canfd_rule_count_set(unit, ch_config->channel, ch_config->receive_filter_count);
        }
    }

    return RS_CANFD_OK;
}

uint8_t rs_canfd_dlc_to_length(uint8_t dlc)
{
    if (dlc > 15U)
    {
        return RS_CANFD_FRAME_MAX_BYTES;
    }

    return gs_rs_canfd_dlc_to_length[dlc];
}

uint8_t rs_canfd_length_to_dlc(uint8_t length)
{
    uint8_t dlc;

    for (dlc = 0U; dlc < 16U; dlc++)
    {
        if (length <= gs_rs_canfd_dlc_to_length[dlc])
        {
            return dlc;
        }
    }

    return 15U;
}

rs_canfd_result_t rs_canfd_enter_global_mode(volatile rs_canfd_unit_t *unit,
                                             rs_canfd_global_mode_t mode)
{
    uint32_t reg;
    rs_canfd_result_t result;

    if (unit == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }

    reg = unit->CFDGCTR.UINT32;

    if (mode == RS_CANFD_GLOBAL_MODE_STOP)
    {
        reg |= RS_CANFD_CFDGCTR_GSLPR;
        unit->CFDGCTR.UINT32 = reg;
        return rs_canfd_wait_mask(&unit->CFDGSTS, RS_CANFD_CFDGSTS_GSLPSTS,
                                  RS_CANFD_CFDGSTS_GSLPSTS, RS_CANFD_WAIT_TIMEOUT);
    }

    reg &= (uint32_t)(~RS_CANFD_CFDGCTR_GSLPR);
    unit->CFDGCTR.UINT32 = reg;
    result =
        rs_canfd_wait_mask(&unit->CFDGSTS, RS_CANFD_CFDGSTS_GSLPSTS, 0UL, RS_CANFD_WAIT_TIMEOUT);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    reg = unit->CFDGCTR.UINT32;
    reg = rs_canfd_replace_field(reg, RS_CANFD_CFDGCTR_GMDC_POS, RS_CANFD_CFDGCTR_GMDC_WIDTH,
                                 (uint32_t)mode);
    unit->CFDGCTR.UINT32 = reg;

    if (mode == RS_CANFD_GLOBAL_MODE_RESET)
    {
        return rs_canfd_wait_mask(&unit->CFDGSTS, RS_CANFD_CFDGSTS_GRSTSTS,
                                  RS_CANFD_CFDGSTS_GRSTSTS, RS_CANFD_WAIT_TIMEOUT);
    }
    if (mode == RS_CANFD_GLOBAL_MODE_TEST)
    {
        return rs_canfd_wait_mask(&unit->CFDGSTS, RS_CANFD_CFDGSTS_GHLTSTS,
                                  RS_CANFD_CFDGSTS_GHLTSTS, RS_CANFD_WAIT_TIMEOUT);
    }

    result = rs_canfd_wait_mask(
        &unit->CFDGSTS,
        (RS_CANFD_CFDGSTS_GRSTSTS | RS_CANFD_CFDGSTS_GHLTSTS | RS_CANFD_CFDGSTS_GSLPSTS), 0UL,
        RS_CANFD_WAIT_TIMEOUT);
    return result;
}

rs_canfd_result_t rs_canfd_enter_channel_mode(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                              rs_canfd_channel_mode_t mode)
{
    volatile rs_canfd_channel_regs_t *ch_regs;
    uint32_t reg;
    rs_canfd_result_t result;

    if ((unit == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }

    ch_regs = RS_CANFD_CHANNEL_REGS(unit);
    reg = ch_regs[channel].ctr.UINT32;

    if (mode == RS_CANFD_CHANNEL_MODE_STOP)
    {
        reg |= RS_CANFD_CFDCCCTR_CSLPR;
        ch_regs[channel].ctr.UINT32 = reg;
        return rs_canfd_wait_mask(&ch_regs[channel].sts, RS_CANFD_CFDCCSTS_CSLPSTS,
                                  RS_CANFD_CFDCCSTS_CSLPSTS, RS_CANFD_WAIT_TIMEOUT);
    }

    reg &= (uint32_t)(~RS_CANFD_CFDCCCTR_CSLPR);
    ch_regs[channel].ctr.UINT32 = reg;
    result = rs_canfd_wait_mask(&ch_regs[channel].sts, RS_CANFD_CFDCCSTS_CSLPSTS, 0UL,
                                RS_CANFD_WAIT_TIMEOUT);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    reg = ch_regs[channel].ctr.UINT32;
    reg = rs_canfd_replace_field(reg, RS_CANFD_CFDCCCTR_CHMDC_POS, RS_CANFD_CFDCCCTR_CHMDC_WIDTH,
                                 (uint32_t)mode);
    ch_regs[channel].ctr.UINT32 = reg;

    if (mode == RS_CANFD_CHANNEL_MODE_RESET)
    {
        return rs_canfd_wait_mask(&ch_regs[channel].sts, RS_CANFD_CFDCCSTS_CRSTSTS,
                                  RS_CANFD_CFDCCSTS_CRSTSTS, RS_CANFD_WAIT_TIMEOUT);
    }
    if (mode == RS_CANFD_CHANNEL_MODE_HALT)
    {
        return rs_canfd_wait_mask(&ch_regs[channel].sts, RS_CANFD_CFDCCSTS_CHLTSTS,
                                  RS_CANFD_CFDCCSTS_CHLTSTS, RS_CANFD_WAIT_TIMEOUT);
    }

    result = rs_canfd_wait_mask(
        &ch_regs[channel].sts,
        (RS_CANFD_CFDCCSTS_CSLPSTS | RS_CANFD_CFDCCSTS_CHLTSTS | RS_CANFD_CFDCCSTS_CRSTSTS), 0UL,
        RS_CANFD_WAIT_TIMEOUT);
    return result;
}

rs_canfd_result_t rs_canfd_configure_receive_rules(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                                   const rs_canfd_rx_rule_t *rules,
                                                   uint8_t rule_count, uint8_t page)
{
    uint16_t start_index;
    uint16_t table_index;
    uint8_t index;
    rs_canfd_result_t result;

    /*
     * The old page argument is kept only for advanced callers that already use
     * this raw API. Hardware page selection is derived from the full rule table
     * index so callers do not need to know RCFDCnCFDGAFLECTR.AFLPN.
     */
    (void)page;

    if ((unit == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if ((rule_count != 0U) && (rules == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    rs_canfd_rule_count_set(unit, channel, rule_count);
    start_index = rs_canfd_rule_start_index_get(unit, channel);
    if ((uint16_t)(start_index + rule_count) > RS_CANFD_MAX_RX_RULES)
    {
        return RS_CANFD_ERR_PARAM;
    }

    for (index = 0U; index < rule_count; index++)
    {
        table_index = (uint16_t)(start_index + index);
        result = rs_canfd_write_receive_rule_entry(unit, table_index, &rules[index]);
        if (result != RS_CANFD_OK)
        {
            unit->CFDGAFLECTR.UINT32 = 0UL;
            return result;
        }
    }

    unit->CFDGAFLECTR.UINT32 = 0UL;
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_configure_receive_filters(volatile rs_canfd_unit_t *unit,
                                                     uint8_t channel,
                                                     const rs_canfd_receive_filter_t *filters,
                                                     uint8_t filter_count)
{
    rs_canfd_rx_rule_t rule;
    uint16_t start_index;
    uint16_t table_index;
    uint8_t index;
    rs_canfd_result_t result;

    if ((unit == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if ((filter_count != 0U) && (filters == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    /*
     * Figure 24.19: set the receive rule count first, then enable write access
     * and write GAFLID/GAFLM/GAFLP0/GAFLP1 entries. The filter object hides the
     * register bit layout from application code.
     */
    rs_canfd_rule_count_set(unit, channel, filter_count);
    start_index = rs_canfd_rule_start_index_get(unit, channel);
    if ((uint16_t)(start_index + filter_count) > RS_CANFD_MAX_RX_RULES)
    {
        return RS_CANFD_ERR_PARAM;
    }

    for (index = 0U; index < filter_count; index++)
    {
        result = rs_canfd_receive_filter_to_rule(&filters[index], &rule);
        if (result != RS_CANFD_OK)
        {
            unit->CFDGAFLECTR.UINT32 = 0UL;
            return result;
        }

        table_index = (uint16_t)(start_index + index);
        result = rs_canfd_write_receive_rule_entry(unit, table_index, &rule);
        if (result != RS_CANFD_OK)
        {
            unit->CFDGAFLECTR.UINT32 = 0UL;
            return result;
        }
    }

    unit->CFDGAFLECTR.UINT32 = 0UL;
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_configure_buffers(volatile rs_canfd_unit_t *unit,
                                             const rs_canfd_channel_config_t *channel_config)
{
    volatile rs_canfd_reg32_t *rxcc;
    volatile rs_canfd_reg32_t *cfcc;
    volatile rs_canfd_reg32_t *txqcc;
    volatile rs_canfd_reg32_t *thlcc;
    const rs_canfd_rx_fifo_config_t *rx_fifo;
    const rs_canfd_txrx_fifo_config_t *txrx_fifo;
    uint8_t index;
    uint8_t fifo;
    uint8_t channel;
    uint32_t reg;

    if ((unit == 0) || (channel_config == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (rs_canfd_validate_channel(channel_config->channel) != RS_CANFD_OK)
    {
        return RS_CANFD_ERR_PARAM;
    }

    rxcc = RS_CANFD_RX_FIFO_CC(unit);
    cfcc = RS_CANFD_TXRX_FIFO_CC(unit);
    txqcc = RS_CANFD_TX_QUEUE_CC(unit);
    thlcc = RS_CANFD_TX_HISTORY_CC(unit);
    channel = channel_config->channel;

    for (index = 0U; index < channel_config->rx_fifo_count; index++)
    {
        rx_fifo = &channel_config->rx_fifos[index];
        fifo = rx_fifo->fifo;
        if (fifo >= RS_CANFD_RX_FIFO_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }

        reg = 0UL;
        if (rx_fifo->interrupt_enable != 0U)
        {
            reg |= RS_CANFD_CFDRFCC_RFIE;
        }
        if (rx_fifo->interrupt_at_every_frame != 0U)
        {
            reg |= RS_CANFD_CFDRFCC_RFIM;
        }
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDRFCC_RFPLS_POS, RS_CANFD_CFDRFCC_RFPLS_WIDTH,
                                   rx_fifo->payload);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDRFCC_RFDC_POS, RS_CANFD_CFDRFCC_RFDC_WIDTH,
                                   rx_fifo->depth);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDRFCC_RFIGCV_POS, RS_CANFD_CFDRFCC_RFIGCV_WIDTH,
                                   rx_fifo->interrupt_timing);
        rxcc[fifo].UINT32 = reg;
    }

    for (index = 0U; index < channel_config->txrx_fifo_count; index++)
    {
        txrx_fifo = &channel_config->txrx_fifos[index];
        fifo = txrx_fifo->fifo;
        if (fifo >= RS_CANFD_TXRX_FIFO_COUNT)
        {
            return RS_CANFD_ERR_PARAM;
        }

        reg = 0UL;
        if (txrx_fifo->rx_interrupt_enable != 0U)
        {
            reg |= RS_CANFD_CFDCFCC_CFRXIE;
        }
        if (txrx_fifo->tx_interrupt_enable != 0U)
        {
            reg |= RS_CANFD_CFDCFCC_CFTXIE;
        }
        if (txrx_fifo->interrupt_at_every_frame != 0U)
        {
            reg |= RS_CANFD_CFDCFCC_CFIM;
        }
        if (txrx_fifo->interval_timer_enable != 0U)
        {
            reg |= RS_CANFD_CFDCFCC_CFITR;
        }
        if (txrx_fifo->interval_timer_source != 0U)
        {
            reg |= RS_CANFD_CFDCFCC_CFITSS;
        }
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFCC_CFPLS_POS, RS_CANFD_CFDCFCC_CFPLS_WIDTH,
                                   txrx_fifo->payload);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFCC_CFM_POS, RS_CANFD_CFDCFCC_CFM_WIDTH,
                                   txrx_fifo->mode);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFCC_CFIGCV_POS, RS_CANFD_CFDCFCC_CFIGCV_WIDTH,
                                   txrx_fifo->interrupt_timing);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFCC_CFTML_POS, RS_CANFD_CFDCFCC_CFTML_WIDTH,
                                   txrx_fifo->linked_tx_buffer);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFCC_CFDC_POS, RS_CANFD_CFDCFCC_CFDC_WIDTH,
                                   txrx_fifo->depth);
        reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCFCC_CFITT_POS, RS_CANFD_CFDCFCC_CFITT_WIDTH,
                                   txrx_fifo->interval_timer);
        cfcc[fifo].UINT32 = reg;
    }

    if (channel >= RS_CANFD_TX_QUEUE_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }

    reg = 0UL;
    if (channel_config->tx_queue.interrupt_enable != 0U)
    {
        reg |= RS_CANFD_CFDTXQCC_TXQTXIE;
    }
    if (channel_config->tx_queue.interrupt_at_every_frame != 0U)
    {
        reg |= RS_CANFD_CFDTXQCC_TXQIM;
    }
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDTXQCC_TXQDC_POS, RS_CANFD_CFDTXQCC_TXQDC_WIDTH,
                               channel_config->tx_queue.depth);
    txqcc[channel].UINT32 = reg;

    reg = 0UL;
    if (channel_config->tx_history.interrupt_enable != 0U)
    {
        reg |= RS_CANFD_CFDTHLCC_THLIE;
    }
    if (channel_config->tx_history.interrupt_at_every_entry != 0U)
    {
        reg |= RS_CANFD_CFDTHLCC_THLIM;
    }
    if (channel_config->tx_history.store_data != 0U)
    {
        reg |= RS_CANFD_CFDTHLCC_THLDTE;
    }
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDTHLCC_THLIMC_POS, RS_CANFD_CFDTHLCC_THLIMC_WIDTH,
                               channel_config->tx_history.interrupt_threshold);
    thlcc[channel].UINT32 = reg;

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_configure_interrupts(volatile rs_canfd_unit_t *unit,
                                                const rs_canfd_channel_config_t *channel_config)
{
    volatile rs_canfd_channel_regs_t *ch_regs;
    volatile rs_canfd_reg32_t *tmiec;
    uint8_t channel;
    uint32_t reg;

    if ((unit == 0) || (channel_config == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (rs_canfd_validate_channel(channel_config->channel) != RS_CANFD_OK)
    {
        return RS_CANFD_ERR_PARAM;
    }

    ch_regs = RS_CANFD_CHANNEL_REGS(unit);
    tmiec = RS_CANFD_TX_BUFFER_IE(unit);
    channel = channel_config->channel;

    if (channel_config->interrupts.global_error_enable != 0U)
    {
        unit->CFDGCTR.UINT32 |= (RS_CANFD_CFDGCTR_DEIE | RS_CANFD_CFDGCTR_MEIE);
    }

    reg = ch_regs[channel].ctr.UINT32;
    if (channel_config->interrupts.channel_error_enable != 0U)
    {
        reg |= RS_CANFD_CFDCCCTR_BEIE;
        reg |= RS_CANFD_CFDCCCTR_EWIE;
        reg |= RS_CANFD_CFDCCCTR_EPIE;
        reg |= RS_CANFD_CFDCCCTR_BOEIE;
        reg |= RS_CANFD_CFDCCCTR_BORIE;
        reg |= RS_CANFD_CFDCCCTR_OLIE;
        reg |= RS_CANFD_CFDCCCTR_BLIE;
        reg |= RS_CANFD_CFDCCCTR_ALIE;
    }
    if (channel_config->interrupts.tx_complete_enable != 0U)
    {
        reg |= RS_CANFD_CFDCCCTR_TAIE;
    }
    ch_regs[channel].ctr.UINT32 = reg;
    tmiec[channel].UINT32 = channel_config->interrupts.tx_buffer_interrupt_mask;

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_init(const rs_canfd_config_t *config)
{
    volatile rs_canfd_unit_t *unit;
    const rs_canfd_channel_config_t *ch_config;
    rs_canfd_result_t result;
    uint8_t index;
    uint32_t sync_dummy;

    result = rs_canfd_validate_config(config, 0);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    unit = config->unit;

    result =
        rs_canfd_wait_mask(&unit->CFDGSTS, RS_CANFD_CFDGSTS_GRAMINIT, 0UL, RS_CANFD_WAIT_TIMEOUT);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    result = rs_canfd_enter_global_mode(unit, RS_CANFD_GLOBAL_MODE_RESET);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    for (index = 0U; index < config->channel_count; index++)
    {
        ch_config = &config->channels[index];
        if (ch_config->enable != 0U)
        {
            result =
                rs_canfd_enter_channel_mode(unit, ch_config->channel, RS_CANFD_CHANNEL_MODE_RESET);
            if (result != RS_CANFD_OK)
            {
                return result;
            }
        }
    }

    rs_canfd_configure_global(unit, &config->global);
    unit->CFDRMNB.UINT32 =
        RS_CANFD_FIELD_PREP(RS_CANFD_CFDRMNB_NRXMB_POS, RS_CANFD_CFDRMNB_NRXMB_WIDTH,
                            config->rx_buffer.count) |
        RS_CANFD_FIELD_PREP(RS_CANFD_CFDRMNB_RMPLS_POS, RS_CANFD_CFDRMNB_RMPLS_WIDTH,
                            config->rx_buffer.payload);

    for (index = 0U; index < config->channel_count; index++)
    {
        ch_config = &config->channels[index];
        if (ch_config->enable != 0U)
        {
            rs_canfd_configure_channel_mode_bits(unit, ch_config);
        }
    }

    result = rs_canfd_configure_all_rule_counts(config);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    for (index = 0U; index < config->channel_count; index++)
    {
        ch_config = &config->channels[index];
        if (ch_config->enable != 0U)
        {
            result = rs_canfd_configure_receive_filters(unit, ch_config->channel,
                                                        ch_config->receive_filters,
                                                        ch_config->receive_filter_count);
            if (result != RS_CANFD_OK)
            {
                return result;
            }

            result = rs_canfd_configure_buffers(unit, ch_config);
            if (result != RS_CANFD_OK)
            {
                return result;
            }

            result = rs_canfd_configure_interrupts(unit, ch_config);
            if (result != RS_CANFD_OK)
            {
                return result;
            }
        }
    }

    result = rs_canfd_enter_global_mode(unit, RS_CANFD_GLOBAL_MODE_OPERATING);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    for (index = 0U; index < config->channel_count; index++)
    {
        ch_config = &config->channels[index];
        if (ch_config->enable != 0U)
        {
            result = rs_canfd_enter_channel_mode(unit, ch_config->channel,
                                                 RS_CANFD_CHANNEL_MODE_COMMUNICATION);
            if (result != RS_CANFD_OK)
            {
                return result;
            }
        }
    }

    for (index = 0U; index < config->channel_count; index++)
    {
        ch_config = &config->channels[index];
        if (ch_config->enable != 0U)
        {
            result = rs_canfd_enable_runtime_blocks(unit, ch_config);
            if (result != RS_CANFD_OK)
            {
                return result;
            }
        }
    }

    sync_dummy = unit->CFDGSTS.UINT32;
    (void)sync_dummy;
    RS_CANFD_SYNC();

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_deinit(volatile rs_canfd_unit_t *unit)
{
    rs_canfd_result_t result;
    uint8_t channel;

    if (unit == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }

    for (channel = 0U; channel < RS_CANFD_MAX_CHANNELS; channel++)
    {
        result = rs_canfd_enter_channel_mode(unit, channel, RS_CANFD_CHANNEL_MODE_RESET);
        if (result != RS_CANFD_OK)
        {
            return result;
        }
    }

    result = rs_canfd_enter_global_mode(unit, RS_CANFD_GLOBAL_MODE_RESET);
    return result;
}

rs_canfd_result_t rs_canfd_send_tx_buffer(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                          uint8_t tx_buffer_index, const rs_canfd_frame_t *frame)
{
    volatile rs_canfd_tx_buffer_regs_t *tx_regs;
    volatile unsigned char *tmc;
    volatile unsigned char *tmsts;
    rs_canfd_result_t result;
    uint16_t buffer;

    if ((unit == 0) || (rs_canfd_validate_frame(frame) != RS_CANFD_OK) ||
        (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (tx_buffer_index >= RS_CANFD_CHANNEL_TX_BUFFER_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }

    buffer = (uint16_t)(((uint16_t)channel * RS_CANFD_CHANNEL_TX_BUFFER_COUNT) + tx_buffer_index);
    if (buffer >= RS_CANFD_TX_BUFFER_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }

    tmc = RS_CANFD_TX_BUFFER_TMC(unit);
    tmsts = RS_CANFD_TX_BUFFER_TMSTS(unit);
    tx_regs = RS_CANFD_TX_BUFFER_REGS(unit);

    if ((tmsts[buffer] & RS_CANFD_CFDTMSTS_TMTSTS) != 0U)
    {
        return RS_CANFD_ERR_BUSY;
    }

    tmc[buffer] = 0U;
    tmsts[buffer] = 0U;
    result = rs_canfd_write_tx_buffer_regs(&tx_regs[buffer], frame);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    tmc[buffer] = RS_CANFD_CFDTMC_TMTR;

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_get_tx_result(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                         uint8_t tx_buffer_index, rs_canfd_tx_result_t *result)
{
    uint16_t buffer;
    uint8_t status;
    uint8_t flag;
    if (unit == 0 || result == 0 || channel >= RS_CANFD_MAX_CHANNELS ||
        tx_buffer_index >= RS_CANFD_CHANNEL_TX_BUFFER_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }
    buffer = (uint16_t)((uint16_t)channel * RS_CANFD_CHANNEL_TX_BUFFER_COUNT + tx_buffer_index);
    status = RS_CANFD_TX_BUFFER_TMSTS(unit)[buffer];
    flag = (uint8_t)((status >> RS_CANFD_CFDTMSTS_TMTRF_POS) & 3U);
    if ((status & RS_CANFD_CFDTMSTS_TMTSTS) != 0U)
    {
        *result = RS_CANFD_TX_PENDING;
    }
    else if (flag == 2U || flag == 3U)
    {
        *result = RS_CANFD_TX_COMPLETE;
    }
    else if (flag == 1U)
    {
        *result = RS_CANFD_TX_ABORTED;
    }
    else
    {
        *result = RS_CANFD_TX_NONE;
    }
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_request_abort(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                         uint8_t tx_buffer_index)
{
    uint16_t buffer;
    if (unit == 0 || channel >= RS_CANFD_MAX_CHANNELS ||
        tx_buffer_index >= RS_CANFD_CHANNEL_TX_BUFFER_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }
    buffer = (uint16_t)((uint16_t)channel * RS_CANFD_CHANNEL_TX_BUFFER_COUNT + tx_buffer_index);
    RS_CANFD_TX_BUFFER_TMC(unit)[buffer] = RS_CANFD_CFDTMC_TMTAR;
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_abort_tx_buffer(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                           uint8_t tx_buffer_index)
{
    volatile unsigned char *tmc;
    volatile unsigned char *tmsts;
    uint16_t buffer;
    uint32_t guard;

    if ((unit == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (tx_buffer_index >= RS_CANFD_CHANNEL_TX_BUFFER_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }

    buffer = (uint16_t)(((uint16_t)channel * RS_CANFD_CHANNEL_TX_BUFFER_COUNT) + tx_buffer_index);
    tmc = RS_CANFD_TX_BUFFER_TMC(unit);
    tmsts = RS_CANFD_TX_BUFFER_TMSTS(unit);
    tmc[buffer] |= RS_CANFD_CFDTMC_TMTAR;

    guard = RS_CANFD_WAIT_TIMEOUT;
    while (guard > 0UL)
    {
        if ((tmsts[buffer] & RS_CANFD_CFDTMSTS_TMTARM) != 0U)
        {
            tmsts[buffer] = 0U;
            return RS_CANFD_OK;
        }
        guard--;
    }

    return RS_CANFD_ERR_TIMEOUT;
}

rs_canfd_result_t rs_canfd_write_txrx_fifo(volatile rs_canfd_unit_t *unit, uint8_t fifo,
                                           const rs_canfd_frame_t *frame)
{
    volatile rs_canfd_message_ram_regs_t *fifo_regs;
    volatile rs_canfd_reg32_t *sts;
    volatile rs_canfd_reg32_t *pctr;
    rs_canfd_result_t result;

    if ((unit == 0) || (frame == 0) || (fifo >= RS_CANFD_TXRX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    sts = RS_CANFD_TXRX_FIFO_STS(unit);
    if ((sts[fifo].UINT32 & RS_CANFD_CFDCFSTS_CFFLL) != 0UL)
    {
        return RS_CANFD_ERR_FULL;
    }

    fifo_regs = RS_CANFD_TXRX_FIFO_REGS(unit);
    pctr = RS_CANFD_TXRX_FIFO_PCTR(unit);
    result = rs_canfd_write_fifo_regs(&fifo_regs[fifo], frame);
    if (result != RS_CANFD_OK)
    {
        return result;
    }
    pctr[fifo].UINT32 = RS_CANFD_CFDCFPCTR_CFPC;

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_send_tx_queue(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                         const rs_canfd_frame_t *frame)
{
    volatile rs_canfd_reg32_t *txqsts;
    volatile rs_canfd_reg32_t *txqpctr;
    rs_canfd_result_t result;
    uint8_t tx_buffer_index;

    if ((unit == 0) || (frame == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }
    if (channel >= RS_CANFD_TX_QUEUE_COUNT)
    {
        return RS_CANFD_ERR_PARAM;
    }

    txqsts = RS_CANFD_TX_QUEUE_STS(unit);
    if ((txqsts[channel].UINT32 & RS_CANFD_CFDTXQSTS_TXQFLL) != 0UL)
    {
        return RS_CANFD_ERR_FULL;
    }

    tx_buffer_index = 0U;
    result = rs_canfd_send_tx_buffer(unit, channel, tx_buffer_index, frame);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    txqpctr = RS_CANFD_TX_QUEUE_PCTR(unit);
    txqpctr[channel].UINT32 = RS_CANFD_CFDTXQPCTR_TXQPC;
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_read_rx_buffer(volatile rs_canfd_unit_t *unit, uint8_t rx_buffer_index,
                                          rs_canfd_frame_t *frame)
{
    volatile rs_canfd_message_ram_regs_t *rx_regs;
    volatile rs_canfd_reg32_t *newdata;
    rs_canfd_result_t result;
    uint8_t word_index;
    uint8_t bit_index;
    uint32_t bit_mask;

    if ((unit == 0) || (frame == 0) || (rx_buffer_index >= RS_CANFD_RX_BUFFER_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    newdata = RS_CANFD_RX_BUFFER_NEWDATA(unit);
    word_index = (uint8_t)(rx_buffer_index / 32U);
    bit_index = (uint8_t)(rx_buffer_index % 32U);
    bit_mask = RS_CANFD_BIT(bit_index);
    if ((newdata[word_index].UINT32 & bit_mask) == 0UL)
    {
        return RS_CANFD_ERR_EMPTY;
    }

    rx_regs = RS_CANFD_RX_BUFFER_REGS(unit);
    result = rs_canfd_read_message_regs(&rx_regs[rx_buffer_index], frame);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    newdata[word_index].UINT32 = (uint32_t)(~bit_mask);
    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_read_rx_fifo(volatile rs_canfd_unit_t *unit, uint8_t fifo,
                                        rs_canfd_frame_t *frame)
{
    volatile rs_canfd_message_ram_regs_t *fifo_regs;
    volatile rs_canfd_reg32_t *sts;
    volatile rs_canfd_reg32_t *pctr;
    rs_canfd_result_t result;

    if ((unit == 0) || (frame == 0) || (fifo >= RS_CANFD_RX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    sts = RS_CANFD_RX_FIFO_STS(unit);
    if ((sts[fifo].UINT32 & RS_CANFD_CFDRFSTS_RFEMP) != 0UL)
    {
        return RS_CANFD_ERR_EMPTY;
    }

    fifo_regs = RS_CANFD_RX_FIFO_REGS(unit);
    result = rs_canfd_read_message_regs(&fifo_regs[fifo], frame);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    pctr = RS_CANFD_RX_FIFO_PCTR(unit);
    pctr[fifo].UINT32 = RS_CANFD_CFDRFPCTR_RFPC;
    sts[fifo].UINT32 = (uint32_t)(~(RS_CANFD_CFDRFSTS_RFIF | RS_CANFD_CFDRFSTS_RFMLT));

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_read_txrx_fifo(volatile rs_canfd_unit_t *unit, uint8_t fifo,
                                          rs_canfd_frame_t *frame)
{
    volatile rs_canfd_message_ram_regs_t *fifo_regs;
    volatile rs_canfd_reg32_t *sts;
    volatile rs_canfd_reg32_t *pctr;
    rs_canfd_result_t result;

    if ((unit == 0) || (frame == 0) || (fifo >= RS_CANFD_TXRX_FIFO_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    sts = RS_CANFD_TXRX_FIFO_STS(unit);
    if ((sts[fifo].UINT32 & RS_CANFD_CFDCFSTS_CFEMP) != 0UL)
    {
        return RS_CANFD_ERR_EMPTY;
    }

    fifo_regs = RS_CANFD_TXRX_FIFO_REGS(unit);
    result = rs_canfd_read_message_regs(&fifo_regs[fifo], frame);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    pctr = RS_CANFD_TXRX_FIFO_PCTR(unit);
    pctr[fifo].UINT32 = RS_CANFD_CFDCFPCTR_CFPC;
    sts[fifo].UINT32 = (uint32_t)(~(RS_CANFD_CFDCFSTS_CFRXIF | RS_CANFD_CFDCFSTS_CFTXIF |
                                    RS_CANFD_CFDCFSTS_CFMLT));

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_read_tx_history(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                           rs_canfd_tx_history_entry_t *entry)
{
    volatile rs_canfd_tx_history_access_regs_t *acc;
    volatile rs_canfd_reg32_t *sts;
    volatile rs_canfd_reg32_t *pctr;
    uint32_t acc0;
    uint32_t acc1;

    if ((unit == 0) || (entry == 0) || (channel >= RS_CANFD_TX_HISTORY_COUNT))
    {
        return RS_CANFD_ERR_PARAM;
    }

    sts = RS_CANFD_TX_HISTORY_STS(unit);
    if ((sts[channel].UINT32 & RS_CANFD_CFDTHLSTS_THLELT) != 0UL)
    {
        return RS_CANFD_ERR_EMPTY;
    }

    acc = RS_CANFD_TX_HISTORY_ACC_REGS(unit);
    acc0 = acc[channel].acc0.UINT32;
    acc1 = acc[channel].acc1.UINT32;
    entry->valid = 1U;
    entry->tx_buffer_index = (uint8_t)(acc0 & 0xFFUL);
    entry->label = (uint16_t)((acc0 >> 16U) & 0xFFFFUL);
    entry->timestamp = (uint16_t)(acc1 & 0xFFFFUL);
    entry->raw_acc0 = acc0;
    entry->raw_acc1 = acc1;

    pctr = RS_CANFD_TX_HISTORY_PCTR(unit);
    pctr[channel].UINT32 = RS_CANFD_CFDTHLPCTR_THLPC;
    sts[channel].UINT32 = (uint32_t)(~RS_CANFD_CFDTHLSTS_THLIF);

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_get_global_status(volatile rs_canfd_unit_t *unit,
                                             rs_canfd_status_t *status)
{
    uint8_t index;
    volatile rs_canfd_reg32_t *rxsts;
    volatile rs_canfd_reg32_t *cfsts;

    if ((unit == 0) || (status == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    status->global_status = unit->CFDGSTS.UINT32;
    status->global_error_flags = unit->CFDGERFL.UINT32;
    status->channel_status = 0UL;
    status->channel_error_flags = 0UL;
    status->tx_queue_status = 0UL;
    status->tx_history_status = 0UL;

    rxsts = RS_CANFD_RX_FIFO_STS(unit);
    cfsts = RS_CANFD_TXRX_FIFO_STS(unit);
    for (index = 0U; index < RS_CANFD_RX_FIFO_COUNT; index++)
    {
        status->rx_fifo_status[index] = rxsts[index].UINT32;
    }
    for (index = 0U; index < RS_CANFD_TXRX_FIFO_COUNT; index++)
    {
        status->txrx_fifo_status[index] = cfsts[index].UINT32;
    }

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_get_channel_status(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                              rs_canfd_status_t *status)
{
    volatile rs_canfd_channel_regs_t *ch_regs;
    volatile rs_canfd_reg32_t *txqsts;
    volatile rs_canfd_reg32_t *thlsts;
    rs_canfd_result_t result;

    if ((unit == 0) || (status == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }

    result = rs_canfd_get_global_status(unit, status);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    ch_regs = RS_CANFD_CHANNEL_REGS(unit);
    status->channel_status = ch_regs[channel].sts.UINT32;
    status->channel_error_flags = ch_regs[channel].erfl.UINT32;

    if (channel < RS_CANFD_TX_QUEUE_COUNT)
    {
        txqsts = RS_CANFD_TX_QUEUE_STS(unit);
        thlsts = RS_CANFD_TX_HISTORY_STS(unit);
        status->tx_queue_status = txqsts[channel].UINT32;
        status->tx_history_status = thlsts[channel].UINT32;
    }

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_clear_error_flags(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                             uint32_t global_mask, uint32_t channel_mask)
{
    volatile rs_canfd_channel_regs_t *ch_regs;

    if ((unit == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }

    if (global_mask != 0UL)
    {
        unit->CFDGERFL.UINT32 = (uint32_t)(~global_mask);
    }

    if (channel_mask != 0UL)
    {
        ch_regs = RS_CANFD_CHANNEL_REGS(unit);
        ch_regs[channel].erfl.UINT32 = (uint32_t)(~channel_mask);
    }

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_configure_dma_trigger(volatile rs_canfd_unit_t *unit,
                                                 const rs_canfd_dma_trigger_config_t *config)
{
    uint32_t reg;

    if ((unit == 0) || (config == 0))
    {
        return RS_CANFD_ERR_PARAM;
    }

    reg = RS_CANFD_FIELD_PREP(RS_CANFD_CFDCDT_RFDMAE_POS, RS_CANFD_CFDCDT_RFDMAE_WIDTH,
                              config->rx_fifo_mask);
    reg |= RS_CANFD_FIELD_PREP(RS_CANFD_CFDCDT_CFDMAE_POS, RS_CANFD_CFDCDT_CFDMAE_WIDTH,
                               config->txrx_fifo_mask);
    unit->CFDCDTCT.UINT32 = reg;

    return RS_CANFD_OK;
}

rs_canfd_result_t rs_canfd_enter_self_test(volatile rs_canfd_unit_t *unit, uint8_t channel,
                                           rs_canfd_test_mode_t mode)
{
    volatile rs_canfd_channel_regs_t *ch_regs;
    rs_canfd_result_t result;
    uint32_t reg;
    uint8_t ctms;

    if ((unit == 0) || (rs_canfd_validate_channel(channel) != RS_CANFD_OK))
    {
        return RS_CANFD_ERR_PARAM;
    }

    if (mode == RS_CANFD_TEST_MODE_LISTEN_ONLY)
    {
        ctms = 1U;
    }
    else if (mode == RS_CANFD_TEST_MODE_EXTERNAL_LOOPBACK)
    {
        ctms = 2U;
    }
    else
    {
        ctms = 3U;
    }

    result = rs_canfd_enter_channel_mode(unit, channel, RS_CANFD_CHANNEL_MODE_HALT);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    ch_regs = RS_CANFD_CHANNEL_REGS(unit);
    reg = ch_regs[channel].ctr.UINT32;
    reg |= RS_CANFD_CFDCCCTR_CTME;
    reg =
        rs_canfd_replace_field(reg, RS_CANFD_CFDCCCTR_CTMS_POS, RS_CANFD_CFDCCCTR_CTMS_WIDTH, ctms);
    ch_regs[channel].ctr.UINT32 = reg;

    result = rs_canfd_enter_channel_mode(unit, channel, RS_CANFD_CHANNEL_MODE_COMMUNICATION);
    return result;
}

rs_canfd_result_t rs_canfd_run_ram_test(volatile rs_canfd_unit_t *unit)
{
    volatile rs_canfd_reg32_t *rpgacc;
    rs_canfd_result_t result;
    uint32_t guard;
    uint32_t pattern;
    uint32_t reg_value;
    uint16_t page;
    uint8_t word;

    if (unit == 0)
    {
        return RS_CANFD_ERR_PARAM;
    }

    result = rs_canfd_enter_global_mode(unit, RS_CANFD_GLOBAL_MODE_TEST);
    if (result != RS_CANFD_OK)
    {
        return result;
    }

    /*
     * Figure 24.36: write protection release data 1, data 2, then set RTME
     * in immediate succession for the RAM test protected global test function.
     */
    unit->CFDGLOCKK.UINT16[0] = RS_CANFD_CFDGLOCKK_RAM_TEST_KEY1;
    unit->CFDGLOCKK.UINT16[0] = RS_CANFD_CFDGLOCKK_RAM_TEST_KEY2;
    unit->CFDGTSTCTR.UINT32 |= RS_CANFD_CFDGTSTCTR_RTME;
    guard = RS_CANFD_WAIT_TIMEOUT;
    while (guard > 0UL)
    {
        if ((unit->CFDGTSTCTR.UINT32 & RS_CANFD_CFDGTSTCTR_RTME) != 0UL)
        {
            break;
        }
        guard--;
    }

    if (guard == 0UL)
    {
        return RS_CANFD_ERR_TIMEOUT;
    }

    rpgacc = RS_CANFD_RAM_TEST_ACC_REGS(unit);

    for (page = 0U; page < RS_CANFD_RAM_TEST_PAGE_COUNT; page++)
    {
        reg_value = unit->CFDGTSTCFG.UINT32;
        reg_value = rs_canfd_replace_field(reg_value, RS_CANFD_CFDGTSTCFG_RTMPS_POS,
                                           RS_CANFD_CFDGTSTCFG_RTMPS_WIDTH, page);
        unit->CFDGTSTCFG.UINT32 = reg_value;

        for (word = 0U; word < RS_CANFD_RAM_TEST_PAGE_WORDS; word++)
        {
            pattern = 0xA5A50000UL;
            pattern ^= ((uint32_t)page << 6U);
            pattern ^= (uint32_t)word;
            rpgacc[word].UINT32 = pattern;
        }

        for (word = 0U; word < RS_CANFD_RAM_TEST_PAGE_WORDS; word++)
        {
            pattern = 0xA5A50000UL;
            pattern ^= ((uint32_t)page << 6U);
            pattern ^= (uint32_t)word;
            if (rpgacc[word].UINT32 != pattern)
            {
                unit->CFDGTSTCTR.UINT32 &= (uint32_t)(~RS_CANFD_CFDGTSTCTR_RTME);
                return RS_CANFD_ERR_STATE;
            }
        }

        for (word = 0U; word < RS_CANFD_RAM_TEST_PAGE_WORDS; word++)
        {
            rpgacc[word].UINT32 = 0UL;
        }
    }

    unit->CFDGTSTCTR.UINT32 &= (uint32_t)(~RS_CANFD_CFDGTSTCTR_RTME);
    return RS_CANFD_OK;
}
