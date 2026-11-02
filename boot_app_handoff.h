/* Project/API reading notes: Boot/App RAM handoff contract.
 * Preserve the reserved address, signature/request validation and AA/55 owner meanings across
 * both images.
 * These RAM markers coordinate startup/IRQ ownership; they do not replace complete image
 * validation.
 */
#ifndef BOOT_APP_HANDOFF_H
#define BOOT_APP_HANDOFF_H

/* Retention RAM top16 bytes, excluded from ordinary startup initialization.
 * Cold-reset startup initializes these words before any read (ECC).
 * Requests are consumed once; signature/complement are integrity checks,
 * not authentication or persistent rollback storage.
 */
#define BOOT_APP_SHARED_RAM_ADDRESS   (0xFEDFFFF0UL)
#define BOOT_APP_SIGNATURE            (0x31544842UL)
#define BOOT_APP_ENTER_REQUEST        (0x454E5452UL)
#define BOOT_APP_UPDATE_REQUEST       (0x55504454UL)
#define BOOT_APP_OWNER_BOOT           (0xAAUL)
#define BOOT_APP_OWNER_APP            (0x55UL)
#define BOOT_APP_UPDATED_FLAG         (0x100UL)
#define BOOT_APP_SIGNATURE_INDEX      (0U)
#define BOOT_APP_REQUEST_INDEX        (1U)
#define BOOT_APP_COMPLEMENT_INDEX     (2U)
#define BOOT_APP_OWNER_INDEX          (3U)

#endif
