#include "r_cg_macrodriver.h"
#include "device_platform.h"

#pragma section r0_disp32 "READ_RAM"
unsigned char g_read_ram[4096];
#pragma section default
static unsigned char g_read_ready;

/* Raw data read, not a hardware blank check. Code runs from initialized RAM
 * while VCI data ECC is temporarily disabled,so an interrupted/erased image
 * can be rejected by metadata/CRC without causing a SYSERR during the read.
 * PE1 instruction ECC is not changed. No Flash callee/constant is used here.
 */
#pragma section text "RAM_READ"
unsigned char Device_ReadRawRam(unsigned long address, unsigned char *out, unsigned short size)
{
    unsigned long saved;
    unsigned short i;
    unsigned char ok;
    const volatile unsigned char *source;
    saved = CFECC_VCI.CFECCCTL_VCI.UINT32 & 3UL;
    CFECC_VCI.CFECCCTL_VCI.UINT32 = 0x4001UL | saved;
    __syncp();
    ok = (unsigned char)((CFECC_VCI.CFECCCTL_VCI.UINT32 & 1UL) != 0UL);
    source = (const volatile unsigned char *)address;
    if (ok)
    {
        for (i = 0U; i < size; i++)
        {
            out[i] = source[i];
        }
    }
    CFECC_VCI.CFECCCTL_VCI.UINT32 = 0x4000UL | saved;
    __syncp();
    if ((CFECC_VCI.CFECCCTL_VCI.UINT32 & 3UL) != saved)
    {
        ok = 0U;
    }
    return ok;
}
#pragma section default

unsigned char Device_ReadPrepare(void)
{
    unsigned long start, size, i;
    const unsigned char *source;
    start = Device_ReadRomStart();
    size = Device_ReadRomSize();
    g_read_ready = 0U;
    if (!size || size > sizeof(g_read_ram))
    {
        return 0U;
    }
    source = (const unsigned char *)start;
    for (i = 0UL; i < size; i++)
    {
        g_read_ram[i] = source[i];
    }
    Device_InstructionSync();
    g_read_ready = 1U;
    return 1U;
}
unsigned char Device_FlashRead(unsigned long address, unsigned char *out, unsigned short size)
{
    typedef unsigned char (*read_ram_fn)(unsigned long, unsigned char *, unsigned short);
    unsigned long psw;
    unsigned char result;
    read_ram_fn volatile function;
    if (!g_read_ready || !out || !size || address >= IMAGE_CODE_FLASH_SIZE ||
        (unsigned long)size > IMAGE_CODE_FLASH_SIZE - address)
    {
        return 0U;
    }
    psw = Device_IrqSave();
    function = (read_ram_fn)g_read_ram;
    result = function(address, out, size);
    Device_IrqRestore(psw);
    return result;
}
