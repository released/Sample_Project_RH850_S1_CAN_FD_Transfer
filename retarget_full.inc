/* Included only by retarget.c. Original C99-format implementation.
 * Fixed C locale; decimal conversion is exact integer arithmetic followed by
 * nearest/ties-to-even rounding. Precision zeros are streamed, never allocated.
 * No heap, global formatter state, vendor printf or peripheral dependency.
 */
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#if LDBL_MANT_DIG != DBL_MANT_DIG || LDBL_MAX_EXP != DBL_MAX_EXP
#error This port requires long double and double to use the same IEEE format.
#endif

#define FP_LEFT  1U
#define FP_PLUS  2U
#define FP_SPACE 4U
#define FP_ZERO  8U
#define FP_HASH 16U
#define FP_DECIMAL_CAP (DBL_MANT_DIG - DBL_MIN_EXP + DBL_MAX_10_EXP + 16)

enum fp_length { FP_NONE, FP_HH, FP_H, FP_L, FP_LL, FP_J, FP_Z, FP_T, FP_CAP_L };

/* The signed counterpart must match the type, not merely its bit width. */
#if defined(__CCRH__)
typedef long fp_signed_size;
#elif SIZE_MAX == UINT_MAX
typedef int fp_signed_size;
#elif SIZE_MAX == ULONG_MAX
typedef long fp_signed_size;
#else
typedef long long fp_signed_size;
#endif
#if PTRDIFF_MAX == INT_MAX
typedef unsigned int fp_unsigned_difference;
#elif PTRDIFF_MAX == LONG_MAX
typedef unsigned long fp_unsigned_difference;
#else
typedef unsigned long long fp_unsigned_difference;
#endif

typedef struct
{
    int count;
    int error;
} fp_output;

typedef struct
{
    uint64_t significand;
    int exponent;
    int negative;
    int special;
    int normal;
} fp_binary;

typedef struct
{
    unsigned char digit[FP_DECIMAL_CAP]; /* Least significant decimal first. */
    int length;
    int scale; /* value = coefficient * 10^-scale. */
} fp_decimal;

static void fp_emit(fp_output *out, int ch)
{
    if (!out->error)
    {
        if (out->count == INT_MAX || putchar((unsigned char)ch) < 0)
        {
            out->error = 1;
        }
        else
        {
            out->count++;
        }
    }
}

static void fp_repeat(fp_output *out, int ch, long long count)
{
    if (count > INT_MAX - out->count)
    {
        out->error = 1;
    }
    while (count-- > 0 && !out->error)
    {
        fp_emit(out, ch);
    }
}

static int fp_begin(fp_output *out, unsigned int flags, int width,
                    long long body, const char *prefix, int prefix_length)
{
    long long total = body + prefix_length;
    int pad;
    int i;
    if (total > INT_MAX - out->count)
    {
        out->error = 1;
        return 0;
    }
    pad = width > total ? width - (int)total : 0;
    if (pad > INT_MAX - out->count - total)
    {
        out->error = 1;
        return 0;
    }
    if (!(flags & (FP_LEFT | FP_ZERO)))
    {
        fp_repeat(out, ' ', pad);
    }
    for (i = 0; i < prefix_length; i++)
    {
        fp_emit(out, prefix[i]);
    }
    if ((flags & FP_ZERO) && !(flags & FP_LEFT))
    {
        fp_repeat(out, '0', pad);
    }
    return pad;
}

static void fp_end(fp_output *out, unsigned int flags, int pad)
{
    if (flags & FP_LEFT)
    {
        fp_repeat(out, ' ', pad);
    }
}

static void fp_integer(fp_output *out, uintmax_t value, int negative,
                      unsigned int base, unsigned int flags, int width,
                      int precision, int upper, int pointer)
{
    char digits[sizeof(uintmax_t) * CHAR_BIT + 1];
    char prefix[3];
    int count = 0;
    int prefix_length = 0;
    long long zeros;
    int pad;
    int nonzero = value != 0;
    if (value != 0 || precision != 0)
    {
        do
        {
            int d = (int)(value % base);
            digits[count++] = (char)(d < 10 ? '0' + d : (upper ? 'A' : 'a') + d - 10);
            value /= base;
        } while (value != 0);
    }
    zeros = precision > count ? precision - count : 0;
    if (negative || (flags & (FP_PLUS | FP_SPACE)))
    {
        prefix[prefix_length++] = negative ? '-' : ((flags & FP_PLUS) ? '+' : ' ');
    }
    if ((base == 16 && nonzero && (flags & FP_HASH)) || pointer)
    {
        prefix[prefix_length++] = '0';
        prefix[prefix_length++] = upper ? 'X' : 'x';
    }
    if (base == 8 && (flags & FP_HASH) && zeros == 0 && (!count || digits[count - 1] != '0'))
    {
        zeros = 1;
    }
    if (precision >= 0)
    {
        flags &= ~FP_ZERO;
    }
    pad = fp_begin(out, flags, width, (long long)count + zeros, prefix, prefix_length);
    fp_repeat(out, '0', zeros);
    while (count > 0 && !out->error)
    {
        fp_emit(out, digits[--count]);
    }
    fp_end(out, flags, pad);
}

static int fp_ascii(unsigned long value)
{
    /* C-locale wide conversion: reject an unrepresentable character. */
    return value <= 127UL ? (int)value : -1;
}

static void fp_string(fp_output *out, const char *text, const tiny_wchar_t *wide,
                     int is_wide, unsigned int flags, int width, int precision)
{
    int length = 0;
    int pad;
    int i;
    if ((!is_wide && text == NULL) || (is_wide && wide == NULL))
    {
        text = "(null)";
        is_wide = 0;
    }
    while (precision < 0 || length < precision)
    {
        int ch = is_wide ? fp_ascii((unsigned long)wide[length]) : (unsigned char)text[length];
        if (ch < 0)
        {
            out->error = 1;
            return;
        }
        if (ch == 0)
        {
            break;
        }
        if (length == INT_MAX)
        {
            out->error = 1;
            return;
        }
        length++;
    }
    flags &= ~FP_ZERO;
    pad = fp_begin(out, flags, width, length, "", 0);
    for (i = 0; i < length && !out->error; i++)
    {
        fp_emit(out, is_wide ? (int)wide[i] : (unsigned char)text[i]);
    }
    fp_end(out, flags, pad);
}

static fp_binary fp_inspect(double value)
{
    fp_binary parts;
    unsigned int order = 1;
    const unsigned char *p = (const unsigned char *)&value;
    uint64_t bits = 0;
    int fraction_bits = DBL_MANT_DIG - 1;
    unsigned int mask = (1U << ((int)sizeof(value) * CHAR_BIT - DBL_MANT_DIG)) - 1U;
    unsigned int field;
    int i;
    for (i = 0; i < (int)sizeof(value); i++)
    {
        int index = *(const unsigned char *)&order ? (int)sizeof(value) - i - 1 : i;
        bits = (bits << CHAR_BIT) | p[index];
    }
    field = (unsigned int)((bits >> fraction_bits) & mask);
    parts.significand = bits & ((UINT64_C(1) << fraction_bits) - 1);
    parts.special = field == mask ? (parts.significand ? 2 : 1) : 0;
    parts.negative = (int)(bits >> ((int)sizeof(value) * CHAR_BIT - 1));
    parts.normal = field != 0;
    parts.exponent = (field ? (int)field : 1) - (int)(mask >> 1);
    if (field)
    {
        parts.significand |= UINT64_C(1) << fraction_bits;
    }
    return parts;
}

static void fp_multiply(fp_decimal *number, unsigned int factor)
{
    unsigned int carry = 0;
    int i;
    for (i = 0; i < number->length; i++)
    {
        unsigned int v = number->digit[i] * factor + carry;
        number->digit[i] = (unsigned char)(v % 10);
        carry = v / 10;
    }
    while (carry)
    {
        number->digit[number->length++] = (unsigned char)(carry % 10);
        carry /= 10;
    }
}

static void fp_decimal_value(fp_decimal *number, fp_binary parts)
{
    uint64_t sig = parts.significand;
    int shift = parts.exponent - (DBL_MANT_DIG - 1);
    number->length = 0;
    number->scale = 0;
    if (sig == 0)
    {
        number->digit[number->length++] = 0;
        return;
    }
    while (shift < 0 && !(sig & 1))
    {
        sig >>= 1;
        shift++;
    }
    do
    {
        number->digit[number->length++] = (unsigned char)(sig % 10);
        sig /= 10;
    } while (sig);
    if (shift < 0)
    {
        number->scale = -shift;
        while (shift <= -8)
        {
            fp_multiply(number, 390625U); /* 5^8, safe in uint32_t. */
            shift += 8;
        }
        while (shift++ < 0)
        {
            fp_multiply(number, 5U);
        }
    }
    else
    {
        while (shift >= 8)
        {
            fp_multiply(number, 256U);
            shift -= 8;
        }
        if (shift)
        {
            fp_multiply(number, 1U << shift);
        }
    }
}

static int fp_exponent(const fp_decimal *number)
{
    return number->length == 1 && !number->digit[0] ? 0 : number->length - number->scale - 1;
}

static void fp_round(fp_decimal *number, long long keep)
{
    int cut;
    int up = 0;
    int i;
    if (keep >= number->length)
    {
        return;
    }
    cut = keep < 0 ? number->length : number->length - (int)keep;
    if (keep >= 0)
    {
        int next = number->digit[cut - 1];
        up = next > 5;
        if (next == 5)
        {
            up = keep > 0 && (number->digit[cut] & 1);
            for (i = 0; i < cut - 1; i++)
            {
                up |= number->digit[i] != 0;
            }
        }
    }
    if (keep <= 0)
    {
        number->digit[0] = (unsigned char)up;
        number->length = 1;
        number->scale -= cut;
        return;
    }
    for (i = cut; i < number->length; i++)
    {
        number->digit[i - cut] = number->digit[i];
    }
    number->length -= cut;
    number->scale -= cut;
    i = 0;
    while (up && i < number->length)
    {
        number->digit[i]++;
        up = number->digit[i] == 10;
        if (up)
        {
            number->digit[i] = 0;
        }
        i++;
    }
    if (up)
    {
        number->digit[number->length++] = 1;
    }
}

static int fp_digit(const fp_decimal *number, long long power)
{
    long long index = power + number->scale;
    return index < 0 || index >= number->length ? 0 : number->digit[(int)index];
}

static int fp_exp_digits(int exponent)
{
    unsigned int n = exponent < 0 ? (unsigned int)-exponent : (unsigned int)exponent;
    int count = 1;
    while (n >= 10)
    {
        n /= 10;
        count++;
    }
    return count;
}

static void fp_emit_exponent(fp_output *out, char marker, int exponent, int minimum)
{
    char text[12];
    int count = 0;
    unsigned int n = exponent < 0 ? (unsigned int)-exponent : (unsigned int)exponent;
    fp_emit(out, marker);
    fp_emit(out, exponent < 0 ? '-' : '+');
    do
    {
        text[count++] = (char)('0' + n % 10);
        n /= 10;
    } while (n);
    fp_repeat(out, '0', minimum - count);
    while (count)
    {
        fp_emit(out, text[--count]);
    }
}

static void fp_hex_float(fp_output *out, fp_binary parts, unsigned int flags,
                         int width, int precision, int upper)
{
    unsigned char digits[(DBL_MANT_DIG + 2) / 4];
    int natural = (DBL_MANT_DIG - 1 + 3) / 4;
    int leading = parts.normal ? 1 : 0;
    int exponent = parts.significand ? parts.exponent : 0;
    uint64_t bits = parts.significand & ((UINT64_C(1) << (DBL_MANT_DIG - 1)) - 1);
    int i;
    int pad;
    int prefix_length = 0;
    char prefix[3];
    int point;
    long long body;
    bits <<= natural * 4 - (DBL_MANT_DIG - 1);
    for (i = natural - 1; i >= 0; i--)
    {
        digits[i] = (unsigned char)(bits & 15);
        bits >>= 4;
    }
    if (precision < 0)
    {
        precision = natural;
    }
    else if (precision < natural)
    {
        int up = digits[precision] > 8;
        if (digits[precision] == 8)
        {
            up = (precision ? digits[precision - 1] : leading) & 1;
            for (i = precision + 1; i < natural; i++)
            {
                up |= digits[i] != 0;
            }
        }
        i = precision;
        while (up && i > 0)
        {
            digits[--i]++;
            up = digits[i] == 16;
            if (up)
            {
                digits[i] = 0;
            }
        }
        leading += up;
    }
    if (parts.negative || (flags & (FP_PLUS | FP_SPACE)))
    {
        prefix[prefix_length++] = parts.negative ? '-' : ((flags & FP_PLUS) ? '+' : ' ');
    }
    prefix[prefix_length++] = '0';
    prefix[prefix_length++] = upper ? 'X' : 'x';
    point = precision > 0 || (flags & FP_HASH);
    body = 1LL + point + precision + 2 + fp_exp_digits(exponent);
    pad = fp_begin(out, flags, width, body, prefix, prefix_length);
    fp_emit(out, '0' + leading);
    if (point)
    {
        fp_emit(out, '.');
    }
    for (i = 0; i < precision && !out->error; i++)
    {
        int d = i < natural ? digits[i] : 0;
        fp_emit(out, d < 10 ? '0' + d : (upper ? 'A' : 'a') + d - 10);
    }
    fp_emit_exponent(out, upper ? 'P' : 'p', exponent, 1);
    fp_end(out, flags, pad);
}

static void fp_float(fp_output *out, double value, unsigned int flags,
                     int width, int precision, char spec)
{
    fp_binary parts = fp_inspect(value);
    fp_decimal number;
    int upper = spec >= 'A' && spec <= 'Z';
    char lower = (char)(upper ? spec + ('a' - 'A') : spec);
    char sign = parts.negative ? '-' : ((flags & FP_PLUS) ? '+' : ((flags & FP_SPACE) ? ' ' : '\0'));
    int exponent;
    int scientific;
    long long decimals;
    int integer_digits;
    int point;
    int exponent_digits;
    long long body;
    int pad;
    long long i;
    if (parts.special)
    {
        const char *word = parts.special == 1 ? (upper ? "INF" : "inf") : (upper ? "NAN" : "nan");
        pad = fp_begin(out, flags & ~FP_ZERO, width, 3, &sign, sign != '\0');
        for (i = 0; i < 3; i++)
        {
            fp_emit(out, word[i]);
        }
        fp_end(out, flags, pad);
        return;
    }
    if (lower == 'a')
    {
        fp_hex_float(out, parts, flags, width, precision, upper);
        return;
    }
    if (precision < 0)
    {
        precision = 6;
    }
    if (lower == 'g' && precision == 0)
    {
        precision = 1;
    }
    fp_decimal_value(&number, parts);
    exponent = fp_exponent(&number);
    fp_round(&number, lower == 'f' ? (long long)exponent + 1 + precision :
                     (long long)precision + (lower == 'e'));
    exponent = fp_exponent(&number);
    scientific = lower == 'e' || (lower == 'g' && (exponent < -4 || exponent >= precision));
    decimals = lower == 'g' ? (long long)precision - 1 - (scientific ? 0 : exponent) : precision;
    if (lower == 'g' && !(flags & FP_HASH))
    {
        int low = 0;
        long long needed;
        while (low < number.length - 1 && !number.digit[low])
        {
            low++;
        }
        needed = scientific ? exponent - (low - number.scale) : number.scale - low;
        if (number.length == 1 && !number.digit[0])
        {
            needed = 0;
        }
        if (decimals > needed)
        {
            decimals = needed;
        }
        if (decimals < 0)
        {
            decimals = 0;
        }
    }
    integer_digits = scientific ? 1 : (exponent >= 0 ? exponent + 1 : 1);
    point = decimals > 0 || (flags & FP_HASH);
    exponent_digits = fp_exp_digits(exponent);
    if (exponent_digits < 2)
    {
        exponent_digits = 2;
    }
    body = (long long)integer_digits + point + decimals + (scientific ? exponent_digits + 2 : 0);
    pad = fp_begin(out, flags, width, body, &sign, sign != '\0');
    for (i = 0; i < integer_digits && !out->error; i++)
    {
        fp_emit(out, '0' + fp_digit(&number, scientific ? exponent : integer_digits - i - 1));
    }
    if (point)
    {
        fp_emit(out, '.');
    }
    for (i = 0; i < decimals && !out->error; i++)
    {
        fp_emit(out, '0' + fp_digit(&number, (scientific ? exponent : 0LL) - i - 1));
    }
    if (scientific)
    {
        fp_emit_exponent(out, upper ? 'E' : 'e', exponent, 2);
    }
    fp_end(out, flags, pad);
}

static intmax_t fp_signed_arg(va_list *args, enum fp_length length)
{
    switch (length)
    {
    case FP_HH: return (signed char)va_arg(*args, int);
    case FP_H: return (short)va_arg(*args, int);
    case FP_L: return va_arg(*args, long);
    case FP_LL: return va_arg(*args, long long);
    case FP_J: return va_arg(*args, intmax_t);
    case FP_Z: return va_arg(*args, fp_signed_size);
    case FP_T: return va_arg(*args, ptrdiff_t);
    default: return va_arg(*args, int);
    }
}

static uintmax_t fp_unsigned_arg(va_list *args, enum fp_length length)
{
    switch (length)
    {
    case FP_HH: return (unsigned char)va_arg(*args, int);
    case FP_H: return (unsigned short)va_arg(*args, int);
    case FP_L: return va_arg(*args, unsigned long);
    case FP_LL: return va_arg(*args, unsigned long long);
    case FP_J: return va_arg(*args, uintmax_t);
    case FP_Z: return va_arg(*args, size_t);
    case FP_T: return va_arg(*args, fp_unsigned_difference);
    default: return va_arg(*args, unsigned int);
    }
}

static void fp_store_count(va_list *args, enum fp_length length, int count)
{
    switch (length)
    {
    case FP_HH: *va_arg(*args, signed char *) = (signed char)count; break;
    case FP_H: *va_arg(*args, short *) = (short)count; break;
    case FP_L: *va_arg(*args, long *) = count; break;
    case FP_LL: *va_arg(*args, long long *) = count; break;
    case FP_J: *va_arg(*args, intmax_t *) = count; break;
    case FP_Z: *va_arg(*args, fp_signed_size *) = count; break;
    case FP_T: *va_arg(*args, ptrdiff_t *) = count; break;
    default: *va_arg(*args, int *) = count; break;
    }
}

static int fp_number(const char **format, int *value)
{
    const char *p = *format;
    *value = 0;
    while (*p >= '0' && *p <= '9')
    {
        int digit = *p++ - '0';
        if (*value > (INT_MAX - digit) / 10)
        {
            return 0;
        }
        *value = *value * 10 + digit;
    }
    *format = p;
    return 1;
}

int tiny_printf(const char *format, ...)
{
    va_list args;
    fp_output out;
    out.count = 0;
    out.error = format == NULL;
    va_start(args, format);
    while (!out.error && *format)
    {
        unsigned int flags = 0;
        int width = 0;
        int precision = -1;
        enum fp_length length = FP_NONE;
        char spec;
        if (*format++ != '%')
        {
            fp_emit(&out, (unsigned char)format[-1]);
            continue;
        }
        for (;;)
        {
            unsigned int flag = *format == '-' ? FP_LEFT : *format == '+' ? FP_PLUS :
                *format == ' ' ? FP_SPACE : *format == '0' ? FP_ZERO : *format == '#' ? FP_HASH : 0;
            if (!flag)
            {
                break;
            }
            flags |= flag;
            format++;
        }
        if (*format == '*')
        {
            width = va_arg(args, int);
            format++;
            if (width < 0)
            {
                if (width == INT_MIN)
                {
                    out.error = 1;
                    break;
                }
                width = -width;
                flags |= FP_LEFT;
            }
        }
        else if (!fp_number(&format, &width))
        {
            out.error = 1;
            break;
        }
        if (*format == '.')
        {
            format++;
            if (*format == '*')
            {
                precision = va_arg(args, int);
                format++;
                if (precision < 0)
                {
                    precision = -1;
                }
            }
            else if (!fp_number(&format, &precision))
            {
                out.error = 1;
                break;
            }
        }
        if (*format == 'h')
        {
            length = *++format == 'h' ? (++format, FP_HH) : FP_H;
        }
        else if (*format == 'l')
        {
            length = *++format == 'l' ? (++format, FP_LL) : FP_L;
        }
        else if (*format == 'j' || *format == 'z' || *format == 't' || *format == 'L')
        {
            length = *format == 'j' ? FP_J : *format == 'z' ? FP_Z : *format == 't' ? FP_T : FP_CAP_L;
            format++;
        }
        spec = *format;
        if (!spec)
        {
            out.error = 1;
            break;
        }
        format++;
        if (spec == 'd' || spec == 'i')
        {
            intmax_t value = fp_signed_arg(&args, length);
            uintmax_t magnitude = value < 0 ? (uintmax_t)0 - (uintmax_t)value : (uintmax_t)value;
            fp_integer(&out, magnitude, value < 0, 10, flags, width, precision, 0, 0);
        }
        else if (spec == 'u' || spec == 'o' || spec == 'x' || spec == 'X')
        {
            uintmax_t value = fp_unsigned_arg(&args, length);
            unsigned int base = spec == 'o' ? 8U : (spec == 'u' ? 10U : 16U);
            fp_integer(&out, value, 0, base, flags & ~(FP_PLUS | FP_SPACE), width, precision, spec == 'X', 0);
        }
        else if (spec == 'p')
        {
            uintptr_t value = (uintptr_t)va_arg(args, void *);
            fp_integer(&out, value, 0, 16, flags & ~(FP_PLUS | FP_SPACE), width, -1, 0, 1);
        }
        else if (spec == 'n')
        {
            fp_store_count(&args, length, out.count);
        }
        else if (spec == 's')
        {
            if (length == FP_L)
            {
                fp_string(&out, NULL, va_arg(args, const tiny_wchar_t *), 1, flags, width, precision);
            }
            else
            {
                fp_string(&out, va_arg(args, const char *), NULL, 0, flags, width, precision);
            }
        }
        else if (spec == 'c')
        {
            int ch;
            int pad;
            if (length == FP_L)
            {
#if defined(__CCRH__)
                ch = fp_ascii(va_arg(args, tiny_wint_t));
#else
                ch = fp_ascii((unsigned long)va_arg(args, int));
#endif
            }
            else
            {
                ch = (unsigned char)va_arg(args, int);
            }
            if (ch < 0)
            {
                out.error = 1;
                break;
            }
            pad = fp_begin(&out, flags & ~FP_ZERO, width, 1, "", 0);
            fp_emit(&out, ch);
            fp_end(&out, flags, pad);
        }
        else if (spec == 'f' || spec == 'F' || spec == 'e' || spec == 'E' ||
                 spec == 'g' || spec == 'G' || spec == 'a' || spec == 'A')
        {
            double value = length == FP_CAP_L ? (double)va_arg(args, long double) : va_arg(args, double);
            fp_float(&out, value, flags, width, precision, spec);
        }
        else if (spec == '%')
        {
            fp_emit(&out, '%');
        }
        else
        {
            out.error = 1;
        }
    }
    va_end(args);
    return out.error ? -1 : out.count;
}
