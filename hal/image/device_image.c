/* Project/API reading notes: Image validation boundary between raw Flash reads and boot decisions.
 * Read through the supplied memory accessor, validate metadata/ranges/vectors and CRC, then
 * expose installed identity only for a complete valid App.
 */
#include <string.h>
#include "device_image.h"
#include "can_boot_config.h"

unsigned long Device_Read32(const unsigned char *p)
{
    return (unsigned long)p[0] | ((unsigned long)p[1] << 8) | ((unsigned long)p[2] << 16) |
           ((unsigned long)p[3] << 24);
}
void Device_Write32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)v;
    p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16);
    p[3] = (unsigned char)(v >> 24);
}
unsigned long Device_Crc32Update(unsigned long crc, const unsigned char *data, unsigned short size)
{
    static const unsigned long table[16] = {0x00000000UL, 0x1DB71064UL, 0x3B6E20C8UL, 0x26D930ACUL,
                                            0x76DC4190UL, 0x6B6B51F4UL, 0x4DB26158UL, 0x5005713CUL,
                                            0xEDB88320UL, 0xF00F9344UL, 0xD6D6A3E8UL, 0xCB61B38CUL,
                                            0x9B64C2B0UL, 0x86D3D2D4UL, 0xA00AE278UL, 0xBDBDF21CUL};
    unsigned short i;
    for (i = 0U; i < size; i++)
    {
        crc ^= data[i];
        crc = (crc >> 4) ^ table[crc & 15UL];
        crc = (crc >> 4) ^ table[crc & 15UL];
    }
    return crc;
}
unsigned long Device_Crc32(const unsigned char *data, unsigned short size)
{
    return Device_Crc32Update(0xFFFFFFFFUL, data, size) ^ 0xFFFFFFFFUL;
}
static unsigned char BranchValid(const unsigned char *p, unsigned long at, unsigned long end)
{
    unsigned long target;
    if (p[0] == 0x1FU && p[1] == 0U)
    {
        p += 2;
        at += 2UL;
    }
    if (p[0] != 0xE0U || p[1] != 2U)
    {
        return 0U;
    }
    target = at + Device_Read32(p + 2);
    return (unsigned char)(!(target & 1UL) &&
                           target >= IMAGE_APPLICATION_START + IMAGE_APP_CODE_OFFSET &&
                           target < end);
}
unsigned char Device_ImageVectors(device_read_fn read, unsigned long size)
{
    unsigned char data[128];
    unsigned short i;
    unsigned long off, end, value;
    end = IMAGE_APPLICATION_START + size - 4UL;
    if (!read(IMAGE_APPLICATION_START, data, 8U) ||
        !BranchValid(data, IMAGE_APPLICATION_START, end))
    {
        return 0U;
    }
    for (off = 0UL; off < 512UL; off += 16UL)
    {
        if (!read(IMAGE_APPLICATION_START + IMAGE_APP_EXCEPTION_OFFSET + off, data, 8U) ||
            !BranchValid(data, IMAGE_APPLICATION_START + IMAGE_APP_EXCEPTION_OFFSET + off, end))
        {
            return 0U;
        }
    }
    for (off = 0UL; off < 2048UL; off += 128UL)
    {
        if (!read(IMAGE_APPLICATION_START + IMAGE_APP_IRQ_OFFSET + off, data, 128U))
        {
            return 0U;
        }
        for (i = 0U; i < 128U; i += 4U)
        {
            value = Device_Read32(data + i);
            if ((value & 1UL) || value < IMAGE_APPLICATION_START + IMAGE_APP_CODE_OFFSET ||
                value >= end)
            {
                return 0U;
            }
        }
    }
    return 1U;
}
unsigned char Device_ImageValidate(device_read_fn read, device_image_info_t *out)
{
    unsigned long metadata[64];
    unsigned char data[128];
    unsigned char stored[4];
    unsigned long size, off, crc;
    unsigned short count;
    memset(out, 0, sizeof(*out));
    if (!read(IMAGE_APPLICATION_START + IMAGE_MANIFEST_OFFSET, (unsigned char *)metadata, 256U) ||
        ImageMetadataValidate(metadata) != 0U)
    {
        return 0U;
    }
    size = metadata[6];
    crc = 0xFFFFFFFFUL;
    for (off = 0UL; off < size - 4UL; off += count)
    {
        count = (unsigned short)((size - 4UL - off > 128UL) ? 128UL : (size - 4UL - off));
        if (!read(IMAGE_APPLICATION_START + off, data, count))
        {
            return 0U;
        }
        crc = Device_Crc32Update(crc, data, count);
    }
    if (!read(IMAGE_APPLICATION_START + size - 4UL, stored, 4U) ||
        (crc ^ 0xFFFFFFFFUL) != Device_Read32(stored) || !Device_ImageVectors(read, size))
    {
        return 0U;
    }
    out->version = metadata[4];
    out->build = metadata[5];
    out->size = size;
    out->version_control = metadata[29];
    out->valid = 1U;
    return 1U;
}
unsigned long Device_ImageInfo(const device_image_info_t *im, unsigned char page, unsigned char app,
                               unsigned long health, unsigned long diagnostics, unsigned char *ok)
{
    static const unsigned char words[27] = {0,  0,  0,  4,  5,  7,  8,  9,  10, 11, 0,  12, 13, 14,
                                            15, 16, 17, 18, 19, 20, 21, 22, 6,  23, 25, 26, 0};
    *ok = 1U;
    if (page == 0x1BU)
    {
        return app ? health : 0UL;
    }
    if (page == 0x1CU)
    {
        return diagnostics;
    }
    if (page == 0x1DU)
    {
        return im->valid ? (1UL | (im->version_control << 1)) : 0UL;
    }
    if (page > 26U)
    {
        *ok = 0U;
        return 0UL;
    }
    if (page == 0U)
    {
        return IMAGE_TARGET_ID | (5UL << 16) | (6UL << 24);
    }
    if (page == 1U)
    {
        return IMAGE_BOOT_VERSION;
    }
    if (page == 2U)
    {
        return IMAGE_BOOT_BUILD;
    }
    if (page == 3U)
    {
        return im->valid ? im->version : 0xFFFFFFFFUL;
    }
    if (page == 4U)
    {
        return im->valid ? im->build : 0xFFFFFFFFUL;
    }
    if (page == 22U)
    {
        return im->valid ? im->size : 0xFFFFFFFFUL;
    }
    if (page == 10U)
    {
        return CAN_BOOT_CAN_MODE_CLASSICAL | (1UL << 16) | (1UL << 24);
    }
    if (page == 26U)
    {
        return CAN_BOOT_BLOCK_INFO;
    }
    return g_image_metadata_contract[words[page]];
}
