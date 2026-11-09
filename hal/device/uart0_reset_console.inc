/* Included by the project-owned CAN App/Transfer workflow only.
 * UART0 is diagnostic text. Poll one byte; reset only in foreground context.
 * Boot, Flash operations, wire commands and CAN tracing remain independent.
 */
static void Uart0ResetPoll(void)
{
    unsigned char value, error;
#if defined(UART0_CONSOLE_RH850)
    if (!INTC2.ICRLIN30UR1.BIT.RFRLIN30UR1 &&
        !INTC2.ICRLIN30UR2.BIT.RFRLIN30UR2)
    {
        return;
    }
    error = (unsigned char)(RLN30.LEST & _UART_CLEAR_ERROR_FLAG);
    value = (unsigned char)RLN30.LURDR.UINT16;
    RLN30.LEST &= (unsigned char)~_UART_CLEAR_ERROR_FLAG;
    INTC2.ICRLIN30UR1.BIT.RFRLIN30UR1 = 0U;
    INTC2.ICRLIN30UR2.BIT.RFRLIN30UR2 = 0U;
#else
    if (!SRIF0)
    {
        return;
    }
    error = (unsigned char)(SSR01 & 7U);
    value = SDR01L;
    SRIF0 = 0U;
    SIR01 = error;
#endif
    if (error)
    {
        return;
    }
    if (value == (unsigned char)'z' || value == (unsigned char)'Z')
    {
        UART0_CONSOLE_RESET();
    }
#if defined(UART0_CONSOLE_OTHER)
    else
    {
        UART0_CONSOLE_OTHER(value);
    }
#endif
}
