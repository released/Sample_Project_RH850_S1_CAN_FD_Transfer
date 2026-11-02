/* Project/API reading notes: Standalone non-destructive CAN test protocol.
 * Discovery, echo, patterns, statistics and replay handling verify communication without
 * erasing/programming App Flash.
 * FD payload length and CRC checks remain part of the request contract.
 */
#include <string.h>
#include "can_transfer_config.h"
#include "can_transfer_protocol.h"

void CT_Increment(uint16_t *value)
{
    if (*value != 0xFFFFU)
    {
        (*value)++;
    }
}

void CT_Init(ct_protocol_t *state)
{
    memset(state, 0, sizeof(*state));
}

uint8_t CT_Crc8(const uint8_t *data, uint8_t size)
{
    uint8_t crc;
    uint8_t i;
    uint8_t bit;
    crc = 0U;
    for (i = 0U; i < size; i++)
    {
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++)
        {
            crc = (uint8_t)(((crc & 0x80U) != 0U) ? ((crc << 1) ^ 0x07U) : (crc << 1));
        }
    }
    return crc;
}

static void CT_Error(uint8_t *reply, uint8_t command, uint8_t status, uint8_t detail)
{
    reply[3] = 0x7FU;
    reply[4] = command;
    reply[5] = status;
    reply[6] = detail;
}

void CT_Process(ct_protocol_t *state, const uint8_t request[8], uint8_t response[8])
{
    static const char part[] = CAN_TRANSFER_PART_SHORT;
    uint8_t index;
    uint8_t status;
    uint16_t value;
    CT_Increment(&state->stats[0]);
    response[0] = 0x5AU;
    response[1] = 1U;
    response[2] = request[2];
    response[3] = request[3];
    response[4] = request[4];
    response[5] = request[5];
    response[6] = request[6];
    status = 0U;
    if (CT_Crc8(request, 7U) != request[7])
    {
        CT_Increment(&state->stats[5]);
        status = 5U;
    }
    else if (request[0] != 0xA5U)
    {
        status = 2U;
    }
    else if (request[1] != 1U)
    {
        status = 3U;
    }
    else if (request[2] == 0U)
    {
        status = 4U;
    }
    if (status != 0U)
    {
        CT_Increment(&state->stats[4]);
        CT_Error(response, request[3], status, 0U);
        response[7] = CT_Crc8(response, 7U);
        return;
    }
    if ((state->cached != 0U) && (memcmp(state->request, request, 8U) == 0))
    {
        CT_Increment(&state->stats[6]);
        memcpy(response, state->response, 8U);
        return;
    }
    if ((state->cached != 0U) && (request[2] == state->request[2]))
    {
        CT_Increment(&state->stats[4]);
        CT_Error(response, request[3], 4U, 0U);
        response[7] = CT_Crc8(response, 7U);
        return;
    }
    switch (request[3])
    {
    case 1U:
        index = request[4];
        if ((index > 6U) || (request[5] != 0U) || (request[6] != 0U))
        {
            status = 7U;
            break;
        }
        value = 0U;
        if (index == 0U)
        {
            value = 1U;
        }
        else if (index == 1U)
        {
            value = CAN_TRANSFER_TARGET_ID;
        }
        else if (index == 6U)
        {
            value = 0x7FU;
        }
        else
        {
            value = (uint8_t)part[(index - 2U) * 2U];
            value |= (uint16_t)((uint16_t)(uint8_t)part[(index - 2U) * 2U + 1U] << 8);
        }
        response[5] = (uint8_t)value;
        response[6] = (uint8_t)(value >> 8);
        break;
    case 2U:
    case 3U:
        break;
    case 4U:
        response[4] ^= 0xA5U;
        response[5] = (uint8_t)(request[5] + 0x3DU);
        response[6] = (uint8_t)((request[6] << 1) | (request[6] >> 7));
        break;
    case 5U:
        index = request[4];
        if ((index >= CT_STAT_COUNT) || (request[5] != 0U) || (request[6] != 0U))
        {
            status = 7U;
            break;
        }
        value = state->stats[index];
        response[5] = (uint8_t)value;
        response[6] = (uint8_t)(value >> 8);
        break;
    case 6U:
        if ((request[4] != 0xC3U) || (request[5] != 0x3CU) || (request[6] != 0U))
        {
            status = 7U;
            break;
        }
        memset(state->stats, 0, sizeof(state->stats));
        break;
    default:
        status = 6U;
        break;
    }
    if (status != 0U)
    {
        CT_Increment(&state->stats[4]);
        CT_Error(response, request[3], status, 0U);
    }
    response[7] = CT_Crc8(response, 7U);
    memcpy(state->request, request, 8U);
    memcpy(state->response, response, 8U);
    state->cached = 1U;
}

uint8_t CT_ProcessFrame(ct_protocol_t *state, const uint8_t *request, uint8_t length,
                        uint8_t response[64])
{
    uint8_t status;
    if (length != 8U && length != 64U)
    {
        return 0U;
    }
    if (length == 8U &&
        !(state->cached && state->cached_length == 64U && request[2] == state->request[2]))
    {
        CT_Process(state, request, response);
        state->cached_length = 8U;
        return 8U;
    }
    CT_Increment(&state->stats[0]);
    status = 0U;
    if (CT_Crc8(request, (uint8_t)(length - 1U)) != request[length - 1U])
    {
        CT_Increment(&state->stats[5]);
        status = 5U;
    }
    else if (request[0] != 0xA5U)
    {
        status = 2U;
    }
    else if (request[1] != 1U)
    {
        status = 3U;
    }
    else if (!request[2])
    {
        status = 4U;
    }
    else if (state->cached && request[2] == state->request[2])
    {
        if (length == state->cached_length && !memcmp(request, state->request, length))
        {
            CT_Increment(&state->stats[6]);
            memcpy(response, state->response, length);
            return length;
        }
        status = 4U;
    }
    else if (length != 64U || request[3] != 7U)
    {
        status = 7U;
    }
    if (status)
    {
        CT_Increment(&state->stats[4]);
        response[0] = 0x5AU;
        response[1] = 1U;
        response[2] = request[2];
        CT_Error(response, request[3], status, 0U);
        response[7] = CT_Crc8(response, 7U);
        return 8U;
    }
    memcpy(response, request, 64U);
    response[0] = 0x5AU;
    response[63] = CT_Crc8(response, 63U);
    memcpy(state->request, request, 64U);
    memcpy(state->response, response, 64U);
    state->cached = 1U;
    state->cached_length = 64U;
    return 64U;
}
