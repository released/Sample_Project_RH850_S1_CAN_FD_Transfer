/* Project/API reading notes: Project-local image contract: format, MCU/Flash geometry, IRQ ABI
 * and required Boot identity.
 * Addresses and capacities use bytes.
 * Changing platform/layout values requires compatible Boot/App/linker/package settings; normal
 * App release edits use app_image_config.h.
 */
#ifndef IMAGE_METADATA_CONFIG_H
#define IMAGE_METADATA_CONFIG_H

/* Serialized image contract, independent of CAN and the Flash API.
 * Addresses are bytes. App identity comes from app_image_config.h.
 * Core128 + reserved128 at App+100h; CRC32 is the final four image bytes.
 */
#define IMAGE_FORMAT_VERSION              (6U)
#define IMAGE_MANIFEST_SIZE               (128U)
#define IMAGE_METADATA_REGION_SIZE        (256U)
#define IMAGE_MANIFEST_OFFSET             (0x100UL)
#define IMAGE_MANIFEST_MAGIC              (0x004E4143UL) /* CAN + NUL */
#define IMAGE_MANIFEST_FLAGS              (3U)
#define IMAGE_MANIFEST_FORMAT_WORD        (0x00038006UL)
#define IMAGE_METADATA_DETAILED_STATUS    (1U)
#define IMAGE_TARGET_ID                   (0x8501U)
#define IMAGE_APPLICATION_START           (0x10000UL)
#define IMAGE_APPLICATION_CAPACITY        (0xF0000UL)
#define IMAGE_CODE_FLASH_SIZE             (0x100000UL)
#define IMAGE_RESERVED_SIZE               (0x10000UL)
/* App occupies only the uniform32KiB erase blocks above the64KiB Boot. */
#define IMAGE_ERASE_BLOCK_SIZE            (0x8000UL)
#define IMAGE_MINIMUM_IMAGE_SIZE          (0x1004UL)
#define IMAGE_POLICY_WORD                 (0x01012003UL)
#define IMAGE_IRQ_ABI                     (0x31563852UL) /* R8V1 */
#define IMAGE_BOOT_VERSION                (0x00010000UL)
#define IMAGE_BOOT_BUILD                  (20261022UL)
#define IMAGE_VERSION_CONTROL_OFFSET      (0x74UL)

/* EVM R7F701684 / RH850/F1KM, LE ASCII padded with NUL. */
#define IMAGE_MCU_PART_0                   (0x37463752UL)
#define IMAGE_MCU_PART_1                   (0x38363130UL)
#define IMAGE_MCU_PART_2                   (0x00000034UL)
#define IMAGE_MCU_PART_3                   (0x00000000UL)
#define IMAGE_MCU_FAMILY_0                 (0x35384852UL)
#define IMAGE_MCU_FAMILY_1                 (0x31462F30UL)
#define IMAGE_MCU_FAMILY_2                 (0x00004D4BUL)

/* Architecture-specific fixed startup interface. Full512 table is App-local.
 * The entry prefix and metadata remain separate from512-byte EBASE vectors.
 */
#define IMAGE_APP_EXCEPTION_OFFSET        (0x200UL)
#define IMAGE_APP_IRQ_OFFSET              (0x400UL)
#define IMAGE_APP_IRQ_COUNT               (512U)
#define IMAGE_APP_CODE_OFFSET             (0x1000UL)

#endif
