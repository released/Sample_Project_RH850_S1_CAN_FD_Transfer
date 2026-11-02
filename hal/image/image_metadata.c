/* Portable C90 metadata operations; configuration is image-only. */
/* Project/API reading notes: Decode and validate the serialized image contract without touching
 * peripheral registers or Flash APIs.
 * This parser checks metadata fields; callers separately check executable vectors, the full-image
 * CRC, version policy and safe handoff conditions.
 */
#include "image_metadata.h"
#include "image_metadata_config.h"

#if IMAGE_FORMAT_VERSION != 6U || IMAGE_MANIFEST_SIZE != 128U || IMAGE_METADATA_REGION_SIZE != 256U
#error This parser requires format6, core128 and region256.
#endif

#if IMAGE_METADATA_DETAILED_STATUS != 0U && IMAGE_METADATA_DETAILED_STATUS != 1U
#error IMAGE_METADATA_DETAILED_STATUS must be 0 or 1.
#endif

/* CC-RL targets are little-endian: inspect the version's high byte directly.
 * The native-word fallback also supports decoded words on big-endian CPUs.
 */
#if defined(__CCRL__)
#define IMAGE_METADATA_VERSION_HIGH(words) (((const unsigned char IMAGE_METADATA_FAR *)(words))[19])
#else
#define IMAGE_METADATA_VERSION_HIGH(words) ((words)[4] & 0xFF000000UL)
#endif

/* Diagnostic detail changes result labels only, never acceptance checks. */
#if IMAGE_METADATA_DETAILED_STATUS
#define IMAGE_METADATA_POLICY_RESULT IMAGE_METADATA_BAD_POLICY
#define IMAGE_METADATA_FIELD_RESULT(index)                                                         \
    (((index) == 25U)   ? IMAGE_METADATA_BAD_POLICY                                                \
     : ((index) == 26U) ? IMAGE_METADATA_BAD_IRQ_ABI                                               \
                        : (((unsigned char)((index) - 7U) < 5U) ? IMAGE_METADATA_BAD_LAYOUT        \
                                                                : IMAGE_METADATA_BAD_FIELD))
#else
#define IMAGE_METADATA_POLICY_RESULT IMAGE_METADATA_BAD_IMAGE
#define IMAGE_METADATA_FIELD_RESULT(index) IMAGE_METADATA_BAD_IMAGE
#endif

/* Decode one bounded little-endian field from metadata.
 * This read does not validate the image or perform Flash access/erase/program operations.
 */
unsigned char ImageMetadataRead32(const unsigned char IMAGE_METADATA_FAR *data,
                                  unsigned short length, unsigned short offset,
                                  unsigned long *value)
{
    if (value == 0)
    {
        return 0U;
    }
    *value = 0UL;
    if (data == 0 || length < 256U || offset > 252U)
    {
        return 0U;
    }
    *value = (unsigned long)data[offset] | ((unsigned long)data[offset + 1U] << 8) |
             ((unsigned long)data[offset + 2U] << 16) | ((unsigned long)data[offset + 3U] << 24);
    return 1U;
}

/* Check the declared size, format, identity/layout/policy/IRQ contract and reserved words.
 * App version/build/size are variable fields; full-image CRC and executable vectors are checked
 * separately by the caller.
 */
unsigned char ImageMetadataValidate(const unsigned long IMAGE_METADATA_FAR *words)
{
    unsigned char i;
    unsigned long size;
    size = words[6];
    if (size < IMAGE_MINIMUM_IMAGE_SIZE || size > IMAGE_APPLICATION_CAPACITY || (size & 3UL) ||
        IMAGE_METADATA_VERSION_HIGH(words) || words[5] == 0xFFFFFFFFUL ||
        (words[29] & 0xFFFFFFFEUL))
    {
        return IMAGE_METADATA_BAD_IMAGE;
    }
    if (words[27] > IMAGE_BOOT_VERSION)
    {
        return IMAGE_METADATA_POLICY_RESULT;
    }
    for (i = 0U; i < 64U; i++)
    {
        if ((i >= 4U && i <= 6U) || i == 27U || i == 29U)
        {
            continue;
        }
        if (words[i] != ((i < 32U) ? g_image_metadata_contract[i] : 0xFFFFFFFFUL))
        {
            return IMAGE_METADATA_FIELD_RESULT(i);
        }
    }
    return IMAGE_METADATA_OK;
}
