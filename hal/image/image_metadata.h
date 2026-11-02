/* Project/API reading notes: Portable metadata API.
 * Byte offsets are relative to the 256-byte metadata region, not Flash zero.
 * Decoding a field is distinct from accepting an image; use the validation result and the
 * caller's CRC/vector checks together.
 */
#ifndef IMAGE_METADATA_H
#define IMAGE_METADATA_H

#include <limits.h>
#if CHAR_BIT != 8
#error Metadata bytes require 8-bit unsigned char.
#endif

/*
 * C90 metadata reader/validator. No peripheral, Flash-write or reset API.
 * unsigned long must be 32 bits; byte order on the wire is little-endian.
 * CC-RL ROM access uses __far; other compilers use their ordinary pointer.
 * Build image_metadata.c with this project's image_metadata_config.h and
 * image_metadata_profile.c. The latter supplies the expected 32-word core.
 */
#if defined(__CCRL__)
#define IMAGE_METADATA_FAR __far
#else
#define IMAGE_METADATA_FAR
#endif
typedef char ImageMetadataWordMustBe32Bits[(sizeof(unsigned long) == 4U) ? 1 : -1];

#define IMAGE_METADATA_OK            (0x00U)
#define IMAGE_METADATA_BAD_IMAGE     (0x09U)
#define IMAGE_METADATA_BAD_FIELD     (0x0AU)
#define IMAGE_METADATA_BAD_POLICY    (0x0BU)
#define IMAGE_METADATA_BAD_IRQ_ABI   (0x0CU)
#define IMAGE_METADATA_BAD_LAYOUT    (0x0DU)

#ifdef __cplusplus
extern "C" {
#endif

/* Profile words have the serialized core layout. Word27 is the supported
 * Boot version; variable App fields 4/5/6/27/29 are checked separately.
 * Each MCU/ISP project provides exactly one definition, never the driver.
 */
extern const unsigned long g_image_metadata_contract[32];

/* Decode any LE32 window in the 256-byte metadata region.
 * data points to metadata byte0, not the beginning of the App image.
 * length must describe the available buffer (at least256); offset0..252.
 * Unaligned byte buffers are supported. Returns1 on success,0 on bad input;
 * a supplied output is cleared on failure. This reads fields only: success
 * is NOT identity, full-metadata, CRC or installed-image validation.
 */
unsigned char ImageMetadataRead32(const unsigned char IMAGE_METADATA_FAR *data,
                                  unsigned short length, unsigned short offset,
                                  unsigned long *value);

/* Validate all64 words (core32 + reserved32), without CRC or Flash access.
 * words must point to64 aligned native32-bit words decoded from the metadata.
 * Little-endian CPUs may use an aligned Flash/RAM word view directly.
 * Byte-oriented or big-endian backends first decode64 words into aligned
 * storage with ImageMetadataRead32. Extent/alignment are caller responsibilities.
 * Returns one IMAGE_METADATA_* result; unknown fields/reserved bytes fail.
 * No staged state, global error latch or armed version override is retained.
 */
unsigned char ImageMetadataValidate(const unsigned long IMAGE_METADATA_FAR *words);

#ifdef __cplusplus
}
#endif

#endif
