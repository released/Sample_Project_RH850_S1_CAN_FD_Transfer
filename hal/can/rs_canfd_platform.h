#ifndef RS_CANFD_PLATFORM_H
#define RS_CANFD_PLATFORM_H

/* Keep compiler/MCU dependencies behind this include. Public application code
 * normally uses can_service.h, not register addresses. The native test switch
 * supplies only types/register storage and never accesses physical hardware.
 */
#if defined(CAN_NATIVE_TEST)
#include <stdint.h>
#include "iodefine.h"
#undef RCFDC0
#define RCFDC0 g_can_test_registers
#define __syncp() ((void)0)
#else
#include "r_cg_macrodriver.h"
#include "iodefine.h"
#endif

#ifndef RS_CANFD_SYNC
#define RS_CANFD_SYNC() __syncp()
#endif

/* Generated tag names vary with the device file. Keep that dependency here,
 * rather than repeating vendor-generated names through every driver API.
 * The supplied R7F701692 DVF exposes channels0..2, 48 RX buffers and 96 TX
 * buffers. A different MCU requires its matching header/profile/board pins.
 */
#if defined(__R7F701692IODEFINE_HEADER__)
typedef struct __tag358 rs_canfd_unit_t;
typedef union __tag158 rs_canfd_reg32_t;
#define RS_CANFD_MAX_CHANNELS (3U)
#elif defined(__R7F701684IODEFINE_HEADER__)
typedef struct __tag445 rs_canfd_unit_t;
typedef union __tag201 rs_canfd_reg32_t;
#define RS_CANFD_MAX_CHANNELS (6U)
#else
#error Add a reviewed RS-CANFD device header/profile mapping here.
#endif
typedef char rs_canfd_u32_width_check[(sizeof(uint32_t) == 4U) ? 1 : -1];
typedef char rs_canfd_register_width_check[(sizeof(rs_canfd_reg32_t) == 4U) ? 1 : -1];
#if defined(CAN_NATIVE_TEST)
extern rs_canfd_unit_t g_can_test_registers;
#endif

#endif
