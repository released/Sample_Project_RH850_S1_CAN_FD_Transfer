#ifndef RH850_FLASH_GEOMETRY_H
#define RH850_FLASH_GEOMETRY_H
#include "image_metadata_config.h"
/* Blocks0..7 are8KiB; the remaining Code Flash uses32KiB blocks. */
#define DEVICE_FLASH_BLOCK_COUNT (8UL + IMAGE_APPLICATION_CAPACITY / IMAGE_ERASE_BLOCK_SIZE)
#define DEVICE_FLASH_PAGE_BYTES (256UL)
unsigned char Device_FlashPageAllowed(unsigned long address);
unsigned char Device_FlashBlockAllowed(unsigned long address,unsigned long *index);
#endif
