#ifndef DEVICE_TYPES_H
#define DEVICE_TYPES_H

/* CC-RH C90 headers do not provide the C99 stdint typedefs.
 * These match the SC macrodriver's widths on RH850; native tests use stdint.
 */
#if defined(__CCRH__)
#include "r_cg_macrodriver.h"
#else
#include <stdint.h>
#endif

#endif
