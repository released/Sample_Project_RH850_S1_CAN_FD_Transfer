/* Project/API reading notes: Expected values for this project's serialized image contract.
 * Root image_metadata_config.h and transport configuration supply platform values; App release
 * identity comes from generated build inputs.
 * Reserved bytes remain part of the validation contract.
 */
#include "image_metadata.h"
#include "image_metadata_config.h"
#include "can_boot_config.h"

#define PROFILE_WORDS(version, build, size, control)                                               \
    IMAGE_MANIFEST_MAGIC, IMAGE_MANIFEST_FORMAT_WORD, (1UL | (5UL << 16) | (1UL << 24)),           \
        (IMAGE_TARGET_ID | ((unsigned long)CAN_BOOT_CAN_MODE_CLASSICAL << 16)), (version),         \
        (build), (size), IMAGE_APPLICATION_START, IMAGE_APPLICATION_CAPACITY,                      \
        IMAGE_CODE_FLASH_SIZE, IMAGE_RESERVED_SIZE, IMAGE_ERASE_BLOCK_SIZE,                        \
        CAN_BOOT_NOMINAL_500K_MASK, CAN_BOOT_DATA_BITRATE_MASK, CAN_BOOT_REQUEST_ID,               \
        CAN_BOOT_RESPONSE_ID, IMAGE_MCU_PART_0, IMAGE_MCU_PART_1, IMAGE_MCU_PART_2,                \
        IMAGE_MCU_PART_3, IMAGE_MCU_FAMILY_0, IMAGE_MCU_FAMILY_1, IMAGE_MCU_FAMILY_2,              \
        IMAGE_MINIMUM_IMAGE_SIZE, 0x31445055UL, IMAGE_POLICY_WORD, IMAGE_IRQ_ABI,                  \
        IMAGE_BOOT_VERSION, 0UL, (control), 0UL, 0UL

const unsigned long g_image_metadata_contract[32] = {PROFILE_WORDS(0UL, 0UL, 0UL, 0UL)};

#ifdef RH850_DEVICE_APP
#include "app_identity.h"
/* Size is resolved only by the guarded post-link packager. Other fields must
 * already match the actually compiled C headers; no stale-input fallback.
 */
#pragma section const "APP_METADATA"
const unsigned long g_app_metadata[64] = {
    PROFILE_WORDS(DEVICE_APP_VERSION, DEVICE_APP_BUILD, 0xFFFFFFFFUL, DEVICE_APP_VERSION_CONTROL),
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL,
    0xFFFFFFFFUL};
#pragma section default
#endif
