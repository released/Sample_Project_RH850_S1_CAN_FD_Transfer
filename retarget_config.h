#ifndef RETARGET_CONFIG_H
#define RETARGET_CONFIG_H

/* 1: C99 format engine, fixed C locale and nearest/ties-to-even rounding.
 * 0: preserve the existing project's formatter and attempted-output counts.
 * Set this once for the whole project, then rebuild. No extra library/install.
 * This header is project-local; a compiler -D value can override the default.
 */
#ifndef TINY_PRINTF_FULL
#define TINY_PRINTF_FULL 1
#endif

#if TINY_PRINTF_FULL != 0 && TINY_PRINTF_FULL != 1
#error TINY_PRINTF_FULL must be 0 or 1.
#endif

#endif
