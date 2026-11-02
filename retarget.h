#ifndef RETARGET_H
#define RETARGET_H
#include "retarget_config.h"

#if TINY_PRINTF_FULL
#if defined(__CCRH__)
/* CC-RH has no wchar.h. Wide literals use unsigned-short code units;
 * C-locale conversion accepts ASCII, and rejects unrepresentable characters.
 */
typedef unsigned short tiny_wchar_t;
typedef unsigned int tiny_wint_t;
#else
#include <wchar.h>
typedef wchar_t tiny_wchar_t;
typedef wint_t tiny_wint_t;
#endif
#endif

/* Foreground console selected in retarget_config.h.
 * Full: C99 conversions/lengths/dynamic fields, C locale, nearest/even rounding;
 * IEEE32/64 double, matching long double, ASCII wide conversion and checked UART.
 * Current: retain the original formats/rounding/attempted-character count.
 * Both stream precision digits. Full returns -1 on format/count/encoding/output
 * error. A nonnegative Full count means UART acceptance, not wire completion.
 * SendChar owns the board UART; this header has no peripheral dependency.
 */
/* Return the unsigned byte on UART acceptance, or -1 on bounded-wait timeout. */
int SendChar(unsigned char ch);
int putchar(int c);
int tiny_printf(const char *format, ...);

#endif
