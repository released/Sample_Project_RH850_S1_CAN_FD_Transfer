/* Project/API reading notes: CAN transport configuration only: command/response IDs, mode and
 * compiled timing declarations.
 * Keep actual driver timing, Boot/App declarations and Tool selections compatible; neutral image
 * geometry belongs to image_metadata_config.h.
 */
#ifndef CAN_BOOT_CONFIG_H
#define CAN_BOOT_CONFIG_H

/* Compiled transport: changing a rate mask alone does not change registers.
 * RH850_CAN_FD selects the paired FD project; Classical uses the default0.
 */
#ifndef RH850_CAN_FD
#define RH850_CAN_FD (0U)
#endif
#define CAN_BOOT_REQUEST_ID          (0x700U)
#define CAN_BOOT_RESPONSE_ID         (0x701U)
#define CAN_BOOT_BLOCK_DATA_ID       (0x720U)
#define CAN_BOOT_BLOCK_BYTES         (64U)
#define CAN_BOOT_PROTOCOL_ID         (1U)
#define CAN_BOOT_PROTOCOL_VERSION    (5U)
#define CAN_BOOT_CAN_ID_STANDARD     (0U)
#define CAN_BOOT_NOMINAL_500K_MASK    (4UL)
#define CAN_BOOT_INACTIVITY_MS       (30000UL)
#define IMAGE_CHECKSUM_CRC32         (1U)
#if RH850_CAN_FD
#define CAN_BOOT_CAN_MODE_CLASSICAL  (2U) /* CAN mode mask: FD */
#define CAN_BOOT_DATA_BITRATE_MASK   (2UL) /* 2 Mbit/s */
#else
#define CAN_BOOT_CAN_MODE_CLASSICAL  (1U) /* Classical */
#define CAN_BOOT_DATA_BITRATE_MASK   (0UL)
#endif
#define CAN_BOOT_BLOCK_INFO (((unsigned long)CAN_BOOT_BLOCK_DATA_ID << 16) | 64UL)

#endif
