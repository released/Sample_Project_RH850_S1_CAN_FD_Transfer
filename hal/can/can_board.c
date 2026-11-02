/* Project/API reading notes: Board binding for CAN pin routing, clocks and interrupt control.
 * Keep hardware-specific register access here; protocol and image validation use the service API.
 * Enabled ports and timing profiles must match the physical board connection.
 */
#include "r_smc_entry.h"
#include "can_board.h"
#include "can_service.h"

typedef struct board_pin
{
    uint8_t channel, rx_port, rx_pin, rx_alt, tx_port, tx_pin, tx_alt;
} board_pin_t;
/* R7F701684,100-pin EVM. CAN1/CAN4 have board transceivers; enabling another
 * row still requires an external transceiver and a reviewed physical route.
 * Clock setup/startup remain owned by the ISP EVM projections, not v4 demo.
 */
static const board_pin_t g_pins[] = {{0U, 0U, 1U, 7U, 0U, 0U, 2U},  {1U, 10U, 6U, 4U, 10U, 7U, 4U},
                                     {2U, 0U, 5U, 7U, 0U, 4U, 1U},  {3U, 0U, 7U, 7U, 0U, 8U, 5U},
                                     {4U, 0U, 9U, 5U, 0U, 10U, 5U}, {5U, 0U, 13U, 7U, 0U, 14U, 5U}};
typedef struct pin_registers
{
    uint8_t number;
    volatile uint16_t *pm, *pmc, *pfc, *pfce, *pfcae;
} pin_registers_t;
static const pin_registers_t g_ports[] = {
    {0U, &PORT.PM0, &PORT.PMC0, &PORT.PFC0, &PORT.PFCE0, &PORT.PFCAE0},
    {10U, &PORT.PM10, &PORT.PMC10, &PORT.PFC10, &PORT.PFCE10, &PORT.PFCAE10}};
static const pin_registers_t *Port(uint8_t number)
{
    uint8_t i;
    for (i = 0U; i < sizeof(g_ports) / sizeof(g_ports[0]); i++)
    {
        if (g_ports[i].number == number)
        {
            return &g_ports[i];
        }
    }
    return 0;
}
static uint8_t Valid(uint8_t number, uint8_t pin, uint8_t alt)
{
    return (uint8_t)(Port(number) != 0 && pin < 16U && alt >= 1U && alt <= 7U);
}
static void Alternate(uint8_t number, uint8_t pin, uint8_t alt, uint8_t input)
{
    const pin_registers_t *p;
    uint16_t mask, code;
    p = Port(number);
    mask = (uint16_t)(1U << pin);
    code = (uint16_t)(alt - 1U);
    *p->pmc &= (uint16_t)~mask;
    *p->pfc = (uint16_t)((*p->pfc & ~mask) | ((code & 1U) ? mask : 0U));
    *p->pfce = (uint16_t)((*p->pfce & ~mask) | ((code & 2U) ? mask : 0U));
    *p->pfcae = (uint16_t)((*p->pfcae & ~mask) | ((code & 4U) ? mask : 0U));
    if (input)
    {
        *p->pm |= mask;
    }
    else
    {
        *p->pm &= (uint16_t)~mask;
    }
    *p->pmc |= mask;
}
extern unsigned long Device_Millis(void);
uint32_t CanBoard_Millis(void)
{
    return (uint32_t)Device_Millis();
}
uint32_t CanBoard_Lock(void)
{
    uint32_t saved;
    saved = __stsr_rh(5, 0);
    DI();
    __syncp();
    return saved;
}
void CanBoard_Unlock(uint32_t saved)
{
    __syncp();
    if ((saved & 0x20UL) == 0UL)
    {
        EI();
    }
}
rs_canfd_result_t CanBoard_Prepare(void)
{
    uint8_t i, j, k, found;
    const board_pin_t *p, *q;
    CanBoard_EnableRxIrq(0U);
    /* The ISP clock projection establishes16MHz/PLL480/CPU120/CAN40. */
    if (CLKCTL.PLL1C != 0x0001031DUL || !(CLKCTL.PLL1S & 4UL) || CLKCTL.CKSC_CPUCLKS_ACT != 3UL ||
        CLKCTL.CKSC_CPUCLKD_ACT != 0x11UL || CLKCTL.CKSC_PPLLCLKS_ACT != 3UL ||
        CLKCTL.CKSC_IPERI2S_ACT != 1UL || CLKCTL.CKSC_ICANS_ACT != _CGC_RSCAN_CLK_SOURCE_PPLLCLK)
    {
        return RS_CANFD_ERR_STATE;
    }
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        found = 0U;
        for (j = 0U; j < sizeof(g_pins) / sizeof(g_pins[0]); j++)
        {
            if (g_pins[j].channel == g_can_profiles[i].channel)
            {
                found++;
            }
        }
        if (found != 1U)
        {
            return RS_CANFD_ERR_PARAM;
        }
    }
    for (i = 0U; i < sizeof(g_pins) / sizeof(g_pins[0]); i++)
    {
        p = &g_pins[i];
        if (!Can_ChannelEnabled(p->channel))
        {
            continue;
        }
        if (!Valid(p->rx_port, p->rx_pin, p->rx_alt) || !Valid(p->tx_port, p->tx_pin, p->tx_alt) ||
            (p->rx_port == p->tx_port && p->rx_pin == p->tx_pin))
        {
            return RS_CANFD_ERR_PARAM;
        }
        for (k = (uint8_t)(i + 1U); k < sizeof(g_pins) / sizeof(g_pins[0]); k++)
        {
            q = &g_pins[k];
            if (!Can_ChannelEnabled(q->channel))
            {
                continue;
            }
            if ((p->rx_port == q->rx_port && p->rx_pin == q->rx_pin) ||
                (p->rx_port == q->tx_port && p->rx_pin == q->tx_pin) ||
                (p->tx_port == q->rx_port && p->tx_pin == q->rx_pin) ||
                (p->tx_port == q->tx_port && p->tx_pin == q->tx_pin))
            {
                return RS_CANFD_ERR_PARAM;
            }
        }
    }
    for (i = 0U; i < sizeof(g_pins) / sizeof(g_pins[0]); i++)
    {
        p = &g_pins[i];
        if (!Can_ChannelEnabled(p->channel))
        {
            continue;
        }
        Alternate(p->rx_port, p->rx_pin, p->rx_alt, 1U);
        Alternate(p->tx_port, p->tx_pin, p->tx_alt, 0U);
    }
    rs_canfd_set_time_source(CanBoard_Millis, CAN_INIT_TIMEOUT_MS);
    return RS_CANFD_OK;
}
rs_canfd_result_t CanBoard_Start(const rs_canfd_config_t *config)
{
    return rs_canfd_init(config);
}
void CanBoard_EnableRxIrq(uint8_t enable)
{
    uint8_t i, pending;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = 0U;
    if (!enable)
    {
        return;
    }
    INTC1.ICRCANGRECC0.BIT.CTRCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.TBRCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P3RCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P2RCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P1RCANGRECC0 = 1U;
    INTC1.ICRCANGRECC0.BIT.P0RCANGRECC0 = 1U;
    pending = 0U;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (Can_ChannelRunning(g_can_profiles[i].channel) &&
            !(RS_CANFD_RX_FIFO_STS(&RCFDC0)[g_can_profiles[i].fifo].UINT32 &
              RS_CANFD_CFDRFSTS_RFEMP))
        {
            pending = 1U;
        }
    }
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = pending;
    INTC1.ICRCANGRECC0.BIT.MKRCANGRECC0 = 0U;
}
void CanBoard_AckRxIrq(uint8_t repend)
{
    uint8_t i;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (!Can_ChannelRunning(g_can_profiles[i].channel))
        {
            continue;
        }
        RS_CANFD_RX_FIFO_STS(&RCFDC0)
        [g_can_profiles[i].fifo].UINT32 = (uint32_t)~RS_CANFD_CFDRFSTS_RFIF;
        if (!(RS_CANFD_RX_FIFO_STS(&RCFDC0)[g_can_profiles[i].fifo].UINT32 &
              RS_CANFD_CFDRFSTS_RFEMP))
        {
            repend = 1U;
        }
    }
    INTC1.ICRCANGRECC0.BIT.RFRCANGRECC0 = repend;
    __syncp();
}
uint8_t CanBoard_TakeFifoLost(uint8_t fifo)
{
    if (!(RS_CANFD_RX_FIFO_STS(&RCFDC0)[fifo].UINT32 & RS_CANFD_CFDRFSTS_RFMLT))
    {
        return 0U;
    }
    RS_CANFD_RX_FIFO_STS(&RCFDC0)[fifo].UINT32 = (uint32_t)~RS_CANFD_CFDRFSTS_RFMLT;
    return 1U;
}
rs_canfd_result_t CanBoard_Stop(void)
{
    uint8_t i;
    rs_canfd_result_t first, result;
    CanBoard_EnableRxIrq(0U);
    first = RS_CANFD_OK;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        result = rs_canfd_enter_channel_mode(&RCFDC0, g_can_profiles[i].channel,
                                             RS_CANFD_CHANNEL_MODE_RESET);
        if (first == RS_CANFD_OK)
        {
            first = result;
        }
    }
    result = rs_canfd_enter_global_mode(&RCFDC0, RS_CANFD_GLOBAL_MODE_RESET);
    return first == RS_CANFD_OK ? result : first;
}
rs_canfd_result_t CanBoard_StopChannel(uint8_t channel)
{
    uint8_t i;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (g_can_profiles[i].channel != channel)
        {
            continue;
        }
        RS_CANFD_RX_FIFO_CC(&RCFDC0)
        [g_can_profiles[i].fifo].UINT32 &= (uint32_t)~RS_CANFD_CFDRFCC_RFE;
        RS_CANFD_RX_FIFO_STS(&RCFDC0)
        [g_can_profiles[i].fifo].UINT32 = (uint32_t)~RS_CANFD_CFDRFSTS_RFIF;
        return rs_canfd_enter_channel_mode(&RCFDC0, channel, RS_CANFD_CHANNEL_MODE_RESET);
    }
    return RS_CANFD_ERR_PARAM;
}
rs_canfd_result_t CanBoard_StartChannel(uint8_t channel)
{
    uint8_t i;
    rs_canfd_result_t result;
    for (i = 0U; i < CAN_ENABLED_COUNT; i++)
    {
        if (g_can_profiles[i].channel != channel)
        {
            continue;
        }
        result = rs_canfd_enter_channel_mode(&RCFDC0, channel, RS_CANFD_CHANNEL_MODE_COMMUNICATION);
        if (result == RS_CANFD_OK)
        {
            RS_CANFD_RX_FIFO_CC(&RCFDC0)[g_can_profiles[i].fifo].UINT32 |= RS_CANFD_CFDRFCC_RFE;
        }
        return result;
    }
    return RS_CANFD_ERR_PARAM;
}
rs_canfd_result_t CanBoard_ReadFifo(uint8_t fifo, rs_canfd_frame_t *frame)
{
    return rs_canfd_read_rx_fifo(&RCFDC0, fifo, frame);
}
rs_canfd_result_t CanBoard_ReadLatest(uint8_t buffer, rs_canfd_frame_t *frame)
{
    (void)buffer;
    (void)frame;
    return RS_CANFD_ERR_STATE;
}
rs_canfd_result_t CanBoard_Send(uint8_t channel, const rs_canfd_frame_t *frame)
{
    return rs_canfd_send_tx_buffer(&RCFDC0, channel, 0U, frame);
}
rs_canfd_result_t CanBoard_TxResult(uint8_t channel, rs_canfd_tx_result_t *result)
{
    return rs_canfd_get_tx_result(&RCFDC0, channel, 0U, result);
}
rs_canfd_result_t CanBoard_Abort(uint8_t channel)
{
    return rs_canfd_request_abort(&RCFDC0, channel, 0U);
}
rs_canfd_result_t CanBoard_Status(uint8_t channel, rs_canfd_status_t *status)
{
    status->global_status = RCFDC0.CFDGSTS.UINT32;
    status->global_error_flags = RCFDC0.CFDGERFL.UINT32;
    status->channel_status = RS_CANFD_CHANNEL_REGS(&RCFDC0)[channel].sts.UINT32;
    status->channel_error_flags = RS_CANFD_CHANNEL_REGS(&RCFDC0)[channel].erfl.UINT32;
    return RS_CANFD_OK;
}
void CanBoard_ClearErrors(uint8_t channel, uint32_t global, uint32_t errors)
{
    /* ISP retains hardware evidence: status reads/CLEAR_STATS never clear
     * CAN error flags. The service still retains its own software snapshot.
     */
    (void)channel;
    (void)global;
    (void)errors;
}
/* The owning project supplies its existing no-channel EIRET ISR/vector.
 * Do not import standalone v4's CanBoard_RxIsr or startup/vector tables.
 */
