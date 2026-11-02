/* Project/API reading notes: CAN update state machine, independent of board registers.
 * Metadata staging, update ownership, sequence/replay checks and image policy precede Flash
 * writes.
 * Accepted transport blocks accumulate into physical Flash pages; END validates the complete
 * image before requesting reset.
 */
#include <string.h>
#include "boot_engine.h"
#include "can_boot_config.h"

static void ResetStaging(boot_engine_t *s)
{
    s->staged = 0U;
    s->sealed = 0U;
    s->block_bad = 1U;
    s->last_valid = 0U;
    s->page_used = 0U;
    memset(s->page, 255, sizeof(s->page));
}
/* Initialize a fresh RAM update session and inspect the installed App through injected read
 * callbacks.
 * Flash readiness is a separate prerequisite for programming.
 */
void Boot_Init(boot_engine_t *s, const boot_io_t *io, unsigned char ready)
{
    memset(s, 0, sizeof(*s));
    s->io = *io;
    s->flash_ready = ready;
    ResetStaging(s);
    (void)Device_ImageValidate(io->read, &s->installed);
}
/* Release RAM staging/ownership only.
 * This does not undo Flash writes or create a persistent downgrade-policy override.
 */
void Boot_ReleasePort(boot_engine_t *s)
{
    s->active = 0U;
    s->complete = 0U;
    ResetStaging(s);
    s->owner_valid = 0U;
    s->owner_channel = 255U;
}
static unsigned char FlushPage(boot_engine_t *s, unsigned long page_offset)
{
    unsigned long address;
    address = IMAGE_APPLICATION_START + page_offset;
    if ((address & 255UL) || address < IMAGE_APPLICATION_START ||
        address > IMAGE_CODE_FLASH_SIZE - 256UL)
    {
        return 0U;
    }
    if (!s->io.write_page(address, (const unsigned char *)s->page))
    {
        return 0U;
    }
    s->page_used = 0U;
    memset(s->page, 255, sizeof(s->page));
    return 1U;
}
static void BeginBlock(boot_engine_t *s, unsigned long off, unsigned long crc)
{
    unsigned long left;
    s->block_bad = 1U;
    s->block_received = 0U;
    if (!s->active || (off & 63UL) || off >= s->size ||
        (off != s->next && (!s->last_valid || off != s->last_offset)))
    {
        return;
    }
    left = s->size - off;
    s->block_length = (unsigned char)((left < 64UL) ? left : 64UL);
    s->block_mask = (unsigned char)(255U >> (8U - ((s->block_length + 7U) / 8U)));
    s->block_offset = off;
    s->block_crc = crc;
    s->block_bad = 0U;
    memset(s->block, 255, sizeof(s->block));
}
static void Segment(boot_engine_t *s, const boot_frame_t *f)
{
    unsigned char i;
#if !RH850_CAN_FD
    unsigned char pos, count, bit;
#endif
    unsigned char *bytes;
    if (s->block_bad || !s->active)
    {
        return;
    }
#if RH850_CAN_FD
    if (f->id != CAN_BOOT_BLOCK_DATA_ID || f->length != 64U)
    {
        s->block_bad = 1U;
        return;
    }
    bytes = (unsigned char *)s->block;
    for (i = s->block_length; i < 64U; i++)
    {
        if (f->data[i] != 255U)
        {
            s->block_bad = 1U;
            return;
        }
    }
    if (s->block_received && memcmp(bytes, f->data, s->block_length))
    {
        s->block_bad = 1U;
        return;
    }
    memcpy(bytes, f->data, s->block_length);
    s->block_received = s->block_mask;
#else
    pos = (unsigned char)((f->id - CAN_BOOT_BLOCK_DATA_ID) * 8UL);
    if (pos >= s->block_length)
    {
        s->block_bad = 1U;
        return;
    }
    count = (unsigned char)(s->block_length - pos);
    if (count > 8U)
    {
        count = 8U;
    }
    if (f->length != count)
    {
        s->block_bad = 1U;
        return;
    }
    bytes = (unsigned char *)s->block;
    bit = (unsigned char)(1U << (pos / 8U));
    for (i = 0U; i < count; i++)
    {
        if ((s->block_received & bit) && bytes[pos + i] != f->data[i])
        {
            s->block_bad = 1U;
        }
        bytes[pos + i] = f->data[i];
    }
    s->block_received |= bit;
#endif
}
static unsigned char Commit(boot_engine_t *s, unsigned long off, unsigned long crc)
{
    const unsigned char *bytes;
    unsigned short i;
    unsigned long page_offset;
    if (!s->active)
    {
        return 2U;
    }
    if (s->block_bad || off != s->block_offset || crc != s->block_crc ||
        s->block_received != s->block_mask)
    {
        return 3U;
    }
    bytes = (const unsigned char *)s->block;
    if (Device_Crc32(bytes, s->block_length) != crc)
    {
        return 6U;
    }
    if (s->last_valid && off == s->last_offset)
    {
        return (unsigned char)((s->last_length == s->block_length && s->last_crc == crc &&
                                !memcmp(s->last, bytes, s->block_length))
                                   ? 0U
                                   : 3U);
    }
    if (off != s->next)
    {
        return 3U;
    }
    if (off >= 0x100UL && off < 0x200UL &&
        memcmp(bytes, (const unsigned char *)s->metadata + off - 0x100UL, s->block_length))
    {
        s->diagnostics |= 1UL;
        return 9U;
    }
    if (s->page_used + s->block_length > 256U)
    {
        return 3U;
    }
    /* Erase on the first accepted CAN block of an erase region. This keeps
     * the long operation on the wire address carrying the erase timeout.
     * Replay returns above and never erases again. Page writes remain256B.
     */
    if (!((IMAGE_APPLICATION_START + off) & (IMAGE_ERASE_BLOCK_SIZE - 1UL)) &&
        !s->io.erase(IMAGE_APPLICATION_START + off))
    {
        s->diagnostics |= 2UL;
        s->active = 0U;
        s->sealed = 0U;
        return 5U;
    }
    memcpy((unsigned char *)s->page + s->page_used, bytes, s->block_length);
    page_offset = off - (unsigned long)s->page_used;
    i = (unsigned short)(s->page_used + s->block_length);
    s->page_used = i;
    if (i == 256U || off + s->block_length == s->size)
    {
        if (!FlushPage(s, page_offset))
        {
            s->diagnostics |= 2UL;
            s->active = 0U;
            s->sealed = 0U;
            return 5U;
        }
    }
    memcpy(s->last, bytes, s->block_length);
    s->last_offset = off;
    s->last_crc = crc;
    s->last_length = s->block_length;
    s->last_valid = 1U;
    s->next += s->block_length;
    return 0U;
}
/* Process one complete CAN frame from its physical port.
 * Keep raw-block acceptance, page programming, final CRC and the deferred reset request as
 * separate decisions.
 */
unsigned char Boot_Process(boot_engine_t *s, const boot_frame_t *r, unsigned char reply[8],
                           unsigned char *action)
{
    unsigned char command, status, ok;
    unsigned short detail, off16;
    unsigned long value, off;
    unsigned char stored[4];
    *action = 0U;
    /* Discovery is read-only on every bus. All session mutations and raw
     * blocks belong to the bus that successfully reset metadata staging.
     */
    if (s->owner_valid && r->channel != s->owner_channel)
    {
        if (r->id != CAN_BOOT_REQUEST_ID || !r->length)
        {
            return 0U;
        }
        command = r->data[0];
        if (command != 1U && command != 7U)
        {
            if (command == 0x0BU)
            {
                return 0U;
            }
            reply[0] = command;
            reply[1] = 2U;
            reply[2] = s->owner_channel;
            reply[3] = 0U;
            Device_Write32(reply + 4, 0UL);
            return 8U;
        }
    }
    if (!s->owner_valid)
    {
        if ((r->id & 0x7F8UL) == CAN_BOOT_BLOCK_DATA_ID)
        {
            return 0U;
        }
        if (r->id == CAN_BOOT_REQUEST_ID && r->length)
        {
            command = r->data[0];
            if (command == 2U || command == 8U || command == 9U || command == 12U || command == 14U)
            {
                reply[0] = command;
                reply[1] = 2U;
                reply[2] = 255U;
                reply[3] = 0U;
                Device_Write32(reply + 4, 0UL);
                return 8U;
            }
        }
    }
    if ((r->id & 0x7F8UL) == CAN_BOOT_BLOCK_DATA_ID)
    {
        Segment(s, r);
        return 0U;
    }
    if (r->id != CAN_BOOT_REQUEST_ID || !r->length)
    {
        return 0U;
    }
    command = r->data[0];
    status = 0U;
    detail = 0U;
    value = 0UL;
    if (command == 0x0BU)
    {
        s->block_bad = 1U;
        if (r->length == 8U)
        {
            off = (unsigned long)r->data[1] | ((unsigned long)r->data[2] << 8) |
                  ((unsigned long)r->data[3] << 16);
            BeginBlock(s, off, Device_Read32(r->data + 4));
        }
        return 0U;
    }
    switch (command)
    {
    case 1U:
        if (r->length != 1U)
        {
            status = 4U;
            break;
        }
        status = s->flash_ready ? 0U : 5U;
        detail = 0x0605U;
        value = IMAGE_TARGET_ID | ((IMAGE_APPLICATION_CAPACITY / 1024UL) << 16);
        break;
    case 7U:
        if (r->length != 2U)
        {
            status = 4U;
            break;
        }
        value = Device_ImageInfo(&s->installed, r->data[1], 0U, 0UL, s->diagnostics, &ok);
        status = ok ? 0U : 4U;
        detail = (unsigned short)(r->data[1] | 0x200U);
        break;
    case 10U:
        if (r->length != 1U)
        {
            status = 4U;
            break;
        }
        if (s->active)
        {
            status = 2U;
            break;
        }
        ResetStaging(s);
        s->complete = 0U;
        value = 256UL;
        s->owner_valid = 1U;
        s->owner_channel = r->channel;
        break;
    case 8U:
        if (r->length != 8U || r->data[3])
        {
            status = 4U;
            break;
        }
        if (s->active)
        {
            status = 2U;
            break;
        }
        off16 = (unsigned short)(r->data[1] | ((unsigned short)r->data[2] << 8));
        if ((off16 & 3U) || off16 >= 256U || off16 > s->staged)
        {
            status = 3U;
            break;
        }
        if (off16 < s->staged)
        {
            if (memcmp((const unsigned char *)s->metadata + off16, r->data + 4, 4U))
            {
                s->sealed = 0U;
                status = 9U;
            }
        }
        else
        {
            memcpy((unsigned char *)s->metadata + off16, r->data + 4, 4U);
            s->staged += 4U;
            s->sealed = 0U;
        }
        detail = s->staged;
        value = 256UL;
        break;
    case 9U:
        s->sealed = 0U;
        if (r->length != 8U || r->data[1] || r->data[2] || r->data[3])
        {
            status = 4U;
            break;
        }
        if (s->active)
        {
            status = 2U;
            break;
        }
        status = ImageMetadataValidate(s->metadata);
        if (s->staged != 256U)
        {
            status = 9U;
        }
        if (!status &&
            Device_Crc32((const unsigned char *)s->metadata, 256U) != Device_Read32(r->data + 4))
        {
            status = 6U;
        }
        s->sealed = (unsigned char)(status == 0U);
        detail = s->staged;
        value = 256UL;
        break;
    case 2U:
    case 14U:
        if (r->length != 8U)
        {
            status = 4U;
            break;
        }
        if (r->data[1] != 5U ||
            ((unsigned short)r->data[2] | ((unsigned short)r->data[3] << 8)) != IMAGE_TARGET_ID)
        {
            status = 8U;
            break;
        }
        if (!s->sealed || Device_Read32(r->data + 4) != s->metadata[6])
        {
            status = 9U;
            break;
        }
        if (s->active)
        {
            status = s->next ? 2U : 0U;
            value = IMAGE_APPLICATION_START;
            break;
        }
        (void)Device_ImageValidate(s->io.read, &s->installed);
        if (command != 14U && s->installed.valid && s->installed.version_control &&
            s->metadata[4] < s->installed.version)
        {
            status = 14U;
            value = s->installed.version;
            break;
        }
        if (!s->flash_ready)
        {
            status = 5U;
            break;
        }
        s->size = s->metadata[6];
        s->next = 0UL;
        s->active = 1U;
        s->ever_started = 1U;
        s->page_used = 0U;
        s->last_valid = 0U;
        s->complete = 0U;
        memset(s->page, 255, sizeof(s->page));
        value = IMAGE_APPLICATION_START;
        break;
    case 12U:
        if (r->length != 8U)
        {
            status = 4U;
            break;
        }
        off = (unsigned long)r->data[1] | ((unsigned long)r->data[2] << 8) |
              ((unsigned long)r->data[3] << 16);
        status = Commit(s, off, Device_Read32(r->data + 4));
        detail = (unsigned short)(off / 64UL);
        value = IMAGE_APPLICATION_START + s->next;
        break;
    case 4U:
        if (r->length != 1U)
        {
            status = 4U;
            break;
        }
        if (!s->active || s->next != s->size || s->page_used)
        {
            status = 4U;
            break;
        }
        if (!Device_ImageValidate(s->io.read, &s->installed) || s->installed.size != s->size)
        {
            status = 9U;
            break;
        }
        if (!s->io.read(IMAGE_APPLICATION_START + s->size - 4UL, stored, 4U))
        {
            status = 5U;
            break;
        }
        value = Device_Read32(stored);
        detail = (unsigned short)(s->size / 4UL);
        s->complete = 1U;
        *action = 1U;
        break;
    case 5U:
        if (r->length != 1U)
        {
            status = 4U;
            break;
        }
        Boot_ReleasePort(s);
        (void)Device_ImageValidate(s->io.read, &s->installed);
        break;
    case 13U:
        if (r->length != 1U)
        {
            status = 4U;
            break;
        }
        if (s->active)
        {
            status = 2U;
            break;
        }
        if (!Device_ImageValidate(s->io.read, &s->installed))
        {
            status = 9U;
            break;
        }
        if (!s->io.read(IMAGE_APPLICATION_START + s->installed.size - 4UL, stored, 4U))
        {
            status = 5U;
            break;
        }
        value = Device_Read32(stored);
        break;
    default:
        status = 1U;
        break;
    }
    reply[0] = command;
    reply[1] = status;
    reply[2] = (unsigned char)detail;
    reply[3] = (unsigned char)(detail >> 8);
    Device_Write32(reply + 4, value);
    return 8U;
}
