#include "flash_geometry.h"
unsigned char Device_FlashPageAllowed(unsigned long address)
{
    return (unsigned char)(address >= IMAGE_APPLICATION_START && !(address & 255UL) &&
                           address <= IMAGE_CODE_FLASH_SIZE - 256UL);
}
unsigned char Device_FlashBlockAllowed(unsigned long address, unsigned long *index)
{
    if (index == 0 || address < IMAGE_APPLICATION_START || address >= IMAGE_CODE_FLASH_SIZE ||
        (address & 0x7FFFUL))
    {
        return 0U;
    }
    *index = 8UL + (address - IMAGE_APPLICATION_START) / 0x8000UL;
    return (unsigned char)(*index < DEVICE_FLASH_BLOCK_COUNT);
}
