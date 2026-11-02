/* Full tiny_printf from the existing RH850 UART_printf example.
 * Preserve the full formatter and original alternate branch. Formatting uses
 * C99, including long long; other ISP modules retain their C90 authoring style.
 * Formatting has no heap/global state. Integer and floating precision stream
 * to the existing output hook; IEEE special values and full exponent fields
 * are handled before normalization. Width/precision/count overflow is rejected.
 * Foreground only: putchar delegates to the project's bounded SendChar hook.
 */
/*_____ I N C L U D E S ____________________________________________________*/
#include "retarget.h"
#include <float.h>
#include <limits.h>
#include <string.h>

#if FLT_RADIX != 2 ||                                                                              \
    !((DBL_MANT_DIG == 24 && DBL_MAX_EXP == 128) || (DBL_MANT_DIG == 53 && DBL_MAX_EXP == 1024))
#error retarget requires IEEE binary32 or binary64 double.
#endif

/*_____ D E C L A R A T I O N S ____________________________________________*/

/*_____ D E F I N I T I O N S ______________________________________________*/

/*_____ M A C R O S ________________________________________________________*/

/*_____ F U N C T I O N S __________________________________________________*/

// retarget printf
int putchar(int c)
{
#if TINY_PRINTF_FULL
    return SendChar((unsigned char)c);
#elif 1
    // if (c == '\r')
    // {
    //     SendChar('\n');
    // }
    SendChar((char)c);
    return (char)c;
#else
    DISABLE_UART_INT_STATUS_FLAG = 1U; /* disable INTST0 interrupt */
    UART_XFER = (unsigned char)c;
    while (CLEAR_UART_INT_STATUS_FLAG == 0)
    {
    }
    CLEAR_UART_INT_STATUS_FLAG = 0U; /* clear INTST0 interrupt flag */
    return c;
#endif
}

#if TINY_PRINTF_FULL
#include "retarget_full.inc"
#else
#if 1
#include <stdarg.h>
// #include <stdint.h>
#include <stdbool.h>
// #include <stdio.h>   // for putchar()

// Flags for formatting
#define FLAG_LEFT (1 << 0)  // '-'
#define FLAG_PLUS (1 << 1)  // '+'
#define FLAG_SPACE (1 << 2) // ' '
#define FLAG_ZERO (1 << 3)  // '0'
#define FLAG_HASH (1 << 4)  // '#'

// Forward declarations of helper functions
static int print_string(const char *s, int width, int precision, int flags);
static int print_char(char c, int width, int flags);
static int print_integer(long long value, int base, int width, int precision, int flags,
                         bool uppercase);
static int print_unsigned(unsigned long long value, int base, int width, int precision, int flags,
                          bool uppercase);
static int print_pointer(void *ptr, int width, int precision, int flags);
static int print_float(double value, int width, int precision, int flags, char spec);

typedef unsigned long retarget_uintptr_t;

/* A negative helper result or an unrepresentable int count is an error. */
static int add_output_count(int *total, int added)
{
    if (added < 0 || *total > INT_MAX - added)
    {
        return 0;
    }
    *total += added;
    return 1;
}

// The main super_printf function
int tiny_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int printed = 0;
    char ch;
    while ((ch = *fmt++) != '\0')
    {
        if (ch != '%')
        {
            // Literal character
            putchar(ch);
            if (!add_output_count(&printed, 1))
            {
                va_end(args);
                return -1;
            }
        }
        else
        {
            // Parse flags
            int flags = 0;
            bool parsing = true;
            while (parsing)
            {
                switch (*fmt)
                {
                case '-':
                    flags |= FLAG_LEFT;
                    fmt++;
                    break;
                case '+':
                    flags |= FLAG_PLUS;
                    fmt++;
                    break;
                case ' ':
                    flags |= FLAG_SPACE;
                    fmt++;
                    break;
                case '0':
                    flags |= FLAG_ZERO;
                    fmt++;
                    break;
                case '#':
                    flags |= FLAG_HASH;
                    fmt++;
                    break;
                default:
                    parsing = false;
                    break;
                }
            }
            // Parse width (number)
            int width = 0;
            while (*fmt >= '0' && *fmt <= '9')
            {
                int digit = *fmt - '0';
                if (width > (INT_MAX - digit) / 10)
                {
                    va_end(args);
                    return -1;
                }
                width = width * 10 + digit;
                fmt++;
            }
            // Parse precision
            int precision = -1;
            if (*fmt == '.')
            {
                fmt++;
                precision = 0;
                while (*fmt >= '0' && *fmt <= '9')
                {
                    int digit = *fmt - '0';
                    if (precision > (INT_MAX - digit) / 10)
                    {
                        va_end(args);
                        return -1;
                    }
                    precision = precision * 10 + digit;
                    fmt++;
                }
            }
            // Parse length modifier (l or ll)
            int length = 0; // 0=none, 1=l, 2=ll
            if (*fmt == 'l')
            {
                fmt++;
                if (*fmt == 'l')
                {
                    length = 2;
                    fmt++;
                }
                else
                {
                    length = 1;
                }
            }
            // Specifier
            char spec = *fmt++;
            /* An incomplete format ends here; do not read beyond its terminator. */
            if (spec == '\0')
            {
                putchar('%');
                if (!add_output_count(&printed, 1))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            switch (spec)
            {
            case '%':
                putchar('%');
                if (!add_output_count(&printed, 1))
                {
                    va_end(args);
                    return -1;
                }
                break;
            case 'c':
            {
                // Character
                char c = (char)va_arg(args, int);
                if (!add_output_count(&printed, print_char(c, width, flags)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 's':
            {
                // String
                char *s = va_arg(args, char *);
                if (!add_output_count(&printed, print_string(s, width, precision, flags)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'd':
            case 'i':
            {
                // Signed integer
                long long val;
                if (length == 2)
                {
                    val = va_arg(args, long long);
                }
                else if (length == 1)
                {
                    val = va_arg(args, long);
                }
                else
                {
                    val = va_arg(args, int);
                }
                if (!add_output_count(&printed,
                                      print_integer(val, 10, width, precision, flags, false)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'u':
            {
                // Unsigned integer
                unsigned long long val;
                if (length == 2)
                {
                    val = va_arg(args, unsigned long long);
                }
                else if (length == 1)
                {
                    val = va_arg(args, unsigned long);
                }
                else
                {
                    val = va_arg(args, unsigned int);
                }
                if (!add_output_count(&printed,
                                      print_unsigned(val, 10, width, precision, flags, false)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'x':
            {
                // Hex lowercase
                unsigned long long val;
                if (length == 2)
                {
                    val = va_arg(args, unsigned long long);
                }
                else if (length == 1)
                {
                    val = va_arg(args, unsigned long);
                }
                else
                {
                    val = va_arg(args, unsigned int);
                }
                if (!add_output_count(&printed,
                                      print_unsigned(val, 16, width, precision, flags, false)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'X':
            {
                // Hex uppercase
                unsigned long long val;
                if (length == 2)
                {
                    val = va_arg(args, unsigned long long);
                }
                else if (length == 1)
                {
                    val = va_arg(args, unsigned long);
                }
                else
                {
                    val = va_arg(args, unsigned int);
                }
                if (!add_output_count(&printed,
                                      print_unsigned(val, 16, width, precision, flags, true)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'o':
            {
                // Octal
                unsigned long long val;
                if (length == 2)
                {
                    val = va_arg(args, unsigned long long);
                }
                else if (length == 1)
                {
                    val = va_arg(args, unsigned long);
                }
                else
                {
                    val = va_arg(args, unsigned int);
                }
                if (!add_output_count(&printed,
                                      print_unsigned(val, 8, width, precision, flags, false)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'p':
            {
                // Pointer (print as 0xHEX)
                void *ptr = va_arg(args, void *);
                if (!add_output_count(&printed, print_pointer(ptr, width, precision, flags)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'f':
            case 'F':
            {
                // Floating point fixed
                double val = va_arg(args, double);
                if (precision < 0)
                {
                    precision = 6;
                }
                if (!add_output_count(&printed, print_float(val, width, precision, flags, spec)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            case 'e':
            case 'E':
            {
                // Floating point scientific
                double val = va_arg(args, double);
                if (precision < 0)
                {
                    precision = 6;
                }
                if (!add_output_count(&printed, print_float(val, width, precision, flags, spec)))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
            default:
                // Unsupported specifier - print literally
                putchar('%');
                if (!add_output_count(&printed, 1))
                {
                    va_end(args);
                    return -1;
                }
                putchar(spec);
                if (!add_output_count(&printed, 1))
                {
                    va_end(args);
                    return -1;
                }
                break;
            }
        }
    }
    va_end(args);

    return printed;
}

// Print a single char with optional padding
static int print_char(char c, int width, int flags)
{
    int printed = 0;
    int pad = (width > 1) ? width - 1 : 0;
    if (!(flags & FLAG_LEFT))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    putchar(c);
    printed++;
    if (flags & FLAG_LEFT)
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    return printed;
}

// Print a string with width and precision
static int print_string(const char *s, int width, int precision, int flags)
{
    int printed = 0;
    if (!s)
    {
        s = "(null)";
    }
    // Compute string length up to precision
    int len = 0;
    while (s[len] && (precision < 0 || len < precision))
    {
        len++;
    }
    int pad = (width > len) ? width - len : 0;
    if (!(flags & FLAG_LEFT))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    for (int i = 0; i < len; i++)
    {
        putchar(s[i]);
        printed++;
    }
    if (flags & FLAG_LEFT)
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    return printed;
}

// Print a signed integer (possibly 64-bit) in given base
static int print_integer(long long value, int base, int width, int precision, int flags,
                         bool uppercase)
{
    int printed = 0;
    bool negative = false;
    unsigned long long uval;
    if (value < 0)
    {
        negative = true;
        uval = 0ULL - (unsigned long long)value;
    }
    else
    {
        uval = (unsigned long long)value;
    }
    // Convert number to string (in reverse)
    char buf[32];
    int len = 0;
    if (uval == 0)
    {
        buf[len++] = '0';
    }
    else
    {
        while (uval > 0)
        {
            int digit = (int)(uval % (unsigned int)base);
            if (digit < 10)
            {
                buf[len++] = '0' + digit;
            }
            else
            {
                buf[len++] = (uppercase ? 'A' : 'a') + (digit - 10);
            }
            uval /= base;
        }
    }
    // Apply precision (minimum digits)
    int zeroes = (precision > len) ? precision - len : 0;
    // Determine if we need a sign or space
    int sign_char = 0;
    if (negative)
    {
        sign_char = 1;
    }
    else if (flags & FLAG_PLUS)
    {
        sign_char = 1;
    }
    else if (flags & FLAG_SPACE)
    {
        sign_char = 1;
    }
    if (len + zeroes > INT_MAX - sign_char)
    {
        return -1;
    }
    int total_len = len + zeroes + sign_char;
    int pad = (width > total_len) ? width - total_len : 0;
    // Print left spaces if needed
    if (!(flags & FLAG_LEFT) && !(flags & FLAG_ZERO))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    // Print sign or space
    if (negative)
    {
        putchar('-');
        printed++;
    }
    else if (flags & FLAG_PLUS)
    {
        putchar('+');
        printed++;
    }
    else if (flags & FLAG_SPACE)
    {
        putchar(' ');
        printed++;
    }
    // Print zero padding
    if (!(flags & FLAG_LEFT) && (flags & FLAG_ZERO))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar('0');
            printed++;
        }
    }
    // Print the digits in correct order
    for (int i = 0; i < zeroes; i++)
    {
        putchar('0');
        printed++;
    }
    for (int i = len - 1; i >= 0; i--)
    {
        putchar(buf[i]);
        printed++;
    }
    // Print right spaces if left-aligned
    if (flags & FLAG_LEFT)
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    return printed;
}

// Print an unsigned integer in given base (e.g. decimal, hex, octal)
static int print_unsigned(unsigned long long uval, int base, int width, int precision, int flags,
                          bool uppercase)
{
    int printed = 0;
    char buf[32];
    int len = 0;
    if (uval == 0)
    {
        buf[len++] = '0';
    }
    else
    {
        while (uval > 0)
        {
            int digit = (int)(uval % (unsigned int)base);
            if (digit < 10)
            {
                buf[len++] = '0' + digit;
            }
            else
            {
                buf[len++] = (uppercase ? 'A' : 'a') + (digit - 10);
            }
            uval /= base;
        }
    }
    int zeroes = (precision > len) ? precision - len : 0;
    int total_len = len + zeroes;
    int pad = (width > total_len) ? width - total_len : 0;
    if (!(flags & FLAG_LEFT) && !(flags & FLAG_ZERO))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    if (!(flags & FLAG_LEFT) && (flags & FLAG_ZERO))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar('0');
            printed++;
        }
    }
    for (int i = 0; i < zeroes; i++)
    {
        putchar('0');
        printed++;
    }
    for (int i = len - 1; i >= 0; i--)
    {
        putchar(buf[i]);
        printed++;
    }
    if (flags & FLAG_LEFT)
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    return printed;
}

// Print a pointer (treated as hex), with leading 0x
static int print_pointer(void *ptr, int width, int precision, int flags)
{
    int printed = 0;
    (void)precision;
    unsigned long long uval = (unsigned long long)(retarget_uintptr_t)ptr;
    char buf[32];
    int len = 0;
    if (uval == 0)
    {
        buf[len++] = '0';
    }
    else
    {
        while (uval > 0)
        {
            int digit = (int)(uval % 16ULL);
            if (digit < 10)
            {
                buf[len++] = '0' + digit;
            }
            else
            {
                buf[len++] = 'a' + (digit - 10);
            }
            uval /= 16;
        }
    }
    int total_len = len + 2; // "0x" prefix
    int pad = (width > total_len) ? width - total_len : 0;
    if (!(flags & FLAG_LEFT))
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    // "0x" prefix
    putchar('0');
    printed++;
    putchar('x');
    printed++;
    for (int i = len - 1; i >= 0; i--)
    {
        putchar(buf[i]);
        printed++;
    }
    if (flags & FLAG_LEFT)
    {
        for (int i = 0; i < pad; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    return printed;
}

// Print a floating-point value (%f or %e/%E)
/* Inspect the IEEE representation without dividing by zero or comparing NaN.
 * RH850 double modes are binary32/binary64; byte order is detected locally.
 */
typedef struct
{
    unsigned long long significand;
    int exponent;
    int negative;
    int special; /* 0 = finite, 1 = infinity, 2 = NaN. */
} retarget_float_parts;

static retarget_float_parts inspect_float(double value)
{
    retarget_float_parts parts;
    unsigned int endian = 1U;
    int little = (*(const unsigned char *)&endian != 0U);
    const unsigned char *bytes = (const unsigned char *)&value;
    unsigned long long bits = 0ULL;
    int size = (int)sizeof(value);
    int fraction_bits = DBL_MANT_DIG - 1;
    int exponent_bits = size * CHAR_BIT - DBL_MANT_DIG;
    unsigned int exponent_mask = (1U << exponent_bits) - 1U;
    unsigned int bias = exponent_mask >> 1;
    unsigned int field;
    unsigned long long fraction;
    int i;
    for (i = 0; i < size; i++)
    {
        int index = little ? size - i - 1 : i;
        bits = (bits << CHAR_BIT) | bytes[index];
    }
    field = (unsigned int)((bits >> fraction_bits) & exponent_mask);
    fraction = bits & ((1ULL << fraction_bits) - 1ULL);
    parts.negative = (int)((bits >> (size * CHAR_BIT - 1)) & 1ULL);
    parts.special = (field == exponent_mask) ? ((fraction != 0ULL) ? 2 : 1) : 0;
    parts.exponent = (field == 0U) ? 1 - (int)bias : (int)field - (int)bias;
    parts.significand = fraction | ((field != 0U) ? (1ULL << fraction_bits) : 0ULL);
    return parts;
}

/* A large fixed value is already integral at binary32/binary64 precision.
 * Convert its significand, then apply the binary shift to bounded decimal
 * digits. Eight binary bits per step avoid a large-value uint64 conversion.
 */
static int large_integer_digits(unsigned char *digits, int capacity, unsigned long long significand,
                                int shift)
{
    int count = 0;
    do
    {
        if (count == capacity)
        {
            return -1;
        }
        digits[count++] = (unsigned char)(significand % 10ULL);
        significand /= 10ULL;
    } while (significand != 0ULL);
    while (shift > 0)
    {
        int step = (shift > 8) ? 8 : shift;
        unsigned int factor = 1U << step;
        unsigned int carry = 0U;
        int i;
        for (i = 0; i < count; i++)
        {
            unsigned int value = (unsigned int)digits[i] * factor + carry;
            digits[i] = (unsigned char)(value % 10U);
            carry = value / 10U;
        }
        while (carry != 0U)
        {
            if (count == capacity)
            {
                return -1;
            }
            digits[count++] = (unsigned char)(carry % 10U);
            carry /= 10U;
        }
        shift -= step;
    }
    return count;
}

static int print_float(double value, int width, int precision, int flags, char spec)
{
    retarget_float_parts parts = inspect_float(value);
    int scientific = (spec == 'e' || spec == 'E');
    int uppercase = (spec == 'F' || spec == 'E');
    int exponent = 0;
    int printed = 0;
    char sign =
        parts.negative ? '-' : ((flags & FLAG_PLUS) ? '+' : ((flags & FLAG_SPACE) ? ' ' : '\0'));
    unsigned char digits[DBL_MAX_10_EXP + 2];
    int count;
    double fraction = 0.0;
    double rounding = 0.5;
    unsigned int exponent_value;
    int exponent_digits = 2;
    int point;
    unsigned long long length;
    int padding;
    int i;

    if (parts.special != 0)
    {
        char text[5];
        const char *word =
            (parts.special == 1) ? (uppercase ? "INF" : "inf") : (uppercase ? "NAN" : "nan");
        int n = 0;
        if (sign != '\0')
        {
            text[n++] = sign;
        }
        for (i = 0; i < 3; i++)
        {
            text[n++] = word[i];
        }
        text[n] = '\0';
        /* Non-finite values use spaces even when zero padding was requested. */
        return print_string(text, width, -1, flags & ~FLAG_ZERO);
    }
    if (parts.negative)
    {
        value = -value;
    }
    if (precision < 0)
    {
        precision = 6;
    }
    if (scientific && value != 0.0)
    {
        while (value >= 10.0)
        {
            value /= 10.0;
            exponent++;
        }
        while (value < 1.0)
        {
            value *= 10.0;
            exponent--;
        }
    }
    /* Keep the original finite rounding rule. Once the increment underflows
     * to zero, further precision does not need more floating-point work.
     */
    for (i = 0; i < precision && rounding != 0.0; i++)
    {
        rounding /= 10.0;
    }
    value += rounding;
    if (scientific && value >= 10.0)
    {
        value /= 10.0;
        exponent++;
    }
    if (!scientific && parts.exponent >= 64)
    {
        count = large_integer_digits(digits, (int)sizeof(digits), parts.significand,
                                     parts.exponent - (DBL_MANT_DIG - 1));
        if (count < 0)
        {
            return -1;
        }
    }
    else
    {
        /* The scientific mantissa is below ten; the other path is below 2^64. */
        unsigned long long whole = (unsigned long long)value;
        fraction = value - (double)whole;
        count = 0;
        do
        {
            digits[count++] = (unsigned char)(whole % 10ULL);
            whole /= 10ULL;
        } while (whole != 0ULL);
    }
    exponent_value = (exponent < 0) ? 0U - (unsigned int)exponent : (unsigned int)exponent;
    if (scientific)
    {
        unsigned int remaining = exponent_value / 100U;
        while (remaining != 0U)
        {
            exponent_digits++;
            remaining /= 10U;
        }
    }
    point = (precision > 0 || (flags & FLAG_HASH)) ? 1 : 0;
    length = (unsigned long long)count + (unsigned int)point + (unsigned int)precision;
    length += (sign != '\0') ? 1U : 0U;
    if (scientific)
    {
        length += (unsigned int)exponent_digits + 2U;
    }
    if (length > (unsigned long long)INT_MAX)
    {
        return -1;
    }
    padding = (width > (int)length) ? width - (int)length : 0;
    if (!(flags & FLAG_LEFT) && !(flags & FLAG_ZERO))
    {
        for (i = 0; i < padding; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    if (sign != '\0')
    {
        putchar(sign);
        printed++;
    }
    if (!(flags & FLAG_LEFT) && (flags & FLAG_ZERO))
    {
        for (i = 0; i < padding; i++)
        {
            putchar('0');
            printed++;
        }
    }
    for (i = count; i > 0; i--)
    {
        putchar('0' + digits[i - 1]);
        printed++;
    }
    if (point)
    {
        putchar('.');
        printed++;
    }
    /* Stream fractional digits; precision never indexes a fixed-size array. */
    for (i = 0; i < precision; i++)
    {
        int digit = 0;
        if (fraction != 0.0)
        {
            fraction *= 10.0;
            digit = (int)fraction;
            fraction -= digit;
        }
        putchar('0' + digit);
        printed++;
    }
    if (scientific)
    {
        putchar(spec);
        putchar((exponent < 0) ? '-' : '+');
        printed += 2;
        printed += print_unsigned(exponent_value, 10, exponent_digits, -1, FLAG_ZERO, false);
    }
    if (flags & FLAG_LEFT)
    {
        for (i = 0; i < padding; i++)
        {
            putchar(' ');
            printed++;
        }
    }
    return printed;
}

#else

// Print unsigned number with given base, width, padding, alignment
static void mini_print_unsigned(unsigned int num, unsigned int base, int width, int left_align,
                                char fill, int uppercase)
{
    char buf[16];
    int i = 0;
    const char *digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    // Convert number to string (reversed)
    do
    {
        buf[i++] = digits[num % base];
        num /= base;
    } while (num);

    int pad = width - i;
    if (pad < 0)
    {
        pad = 0;
    }

    if (!left_align)
    {
        while (pad--)
        {
            putchar(fill);
        }
    }

    while (i--)
    {
        putchar(buf[i]);
    }

    if (left_align)
    {
        while (pad--)
        {
            putchar(' ');
        }
    }
}

// Print signed number with width, padding, alignment
static void mini_print_signed(int num, int width, int left_align, char fill)
{
    if (num < 0)
    {
        putchar('-');
        num = -num;
        width--;
    }
    mini_print_unsigned((unsigned int)num, 10, width, left_align, fill, 0);
}

// Print float number (only 2 decimal places)
static void mini_print_float(double f, int width, int left_align, char fill)
{
    if (f < 0)
    {
        putchar('-');
        f = -f;
        width--;
    }

    int int_part = (int)f;
    int frac_part = (int)((f - int_part) * 100 + 0.5); // 2 decimal places

    // Adjust width (e.g., for ".xx")
    width -= 3;
    int tmp = int_part;
    while (tmp)
    {
        width--;
        tmp /= 10;
    }

    if (!left_align)
    {
        while (width-- > 0)
        {
            putchar(fill);
        }
    }

    // Print integer part
    mini_print_unsigned(int_part, 10, 0, 0, fill, 0);
    putchar('.');
    // Print fractional part
    if (frac_part < 10)
    {
        putchar('0'); // leading zero
    }
    mini_print_unsigned(frac_part, 10, 0, 0, fill, 0);

    if (left_align)
    {
        while (width-- > 0)
        {
            putchar(' ');
        }
    }
}

int tiny_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int printed = 0;

    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;

            int left_align = 0;
            char fill = ' ';
            int width = 0;
            // int uppercase = 0;

            // Check for left align
            if (*fmt == '-')
            {
                left_align = 1;
                fmt++;
            }

            // Check for zero padding
            if (*fmt == '0')
            {
                fill = '0';
                fmt++;
            }

            // Parse width
            while (*fmt >= '0' && *fmt <= '9')
            {
                width = width * 10 + (*fmt - '0');
                fmt++;
            }

            // Format specifiers
            switch (*fmt)
            {
            case 'd':
                mini_print_signed(va_arg(args, int), width, left_align, fill);
                break;
            case 'u':
                mini_print_unsigned(va_arg(args, unsigned int), 10, width, left_align, fill, 0);
                break;
            case 'x':
                mini_print_unsigned(va_arg(args, unsigned int), 16, width, left_align, fill, 0);
                break;
            case 'X':
                mini_print_unsigned(va_arg(args, unsigned int), 16, width, left_align, fill, 1);
                break;
            case 'c':
                putchar((char)va_arg(args, int));
                break;
            case 's':
            {
                const char *str = va_arg(args, const char *);
                int len = 0;
                const char *p = str;
                while (*p++)
                {
                    len++;
                }

                int pad = width - len;
                if (pad < 0)
                {
                    pad = 0;
                }

                if (!left_align)
                {
                    while (pad--)
                    {
                        putchar(fill);
                    }
                }

                while (*str)
                {
                    putchar(*str++);
                }

                if (left_align)
                {
                    while (pad--)
                    {
                        putchar(' ');
                    }
                }
                break;
            }
            case 'f':
                mini_print_float(va_arg(args, double), width, left_align, fill);
                break;
            case '%':
                putchar('%');
                break;
            default:
                putchar('?');
                break;
            }
        }
        else
        {
            putchar(*fmt);
        }
        fmt++;
    }

    va_end(args);

    return printed;
}

#endif
#endif /* TINY_PRINTF_FULL */
