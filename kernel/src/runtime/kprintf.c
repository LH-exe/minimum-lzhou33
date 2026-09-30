/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 1
 * Minimal printf-like console logging built on the UART0 driver.
 *
 * Note: the platform has no standard library (no newlib), so this is a
 * freestanding, hand-rolled formatter using only <stdarg.h> (a compiler
 * header, not a libc).
 */
#include "kprintf.h"
#include "uart.h"
#include <stdarg.h>
#include <stdint.h>

static void put_unsigned(uint32_t value, unsigned base, int uppercase) {
    char buf[10]; /* 32 bits in base 2 needs at most 32 chars; base>=2 -> 10 in hex worst case is 8, binary never used */
    char *p = buf + sizeof(buf);
    static const char digits_lower[] = "0123456789abcdef";
    static const char digits_upper[] = "0123456789ABCDEF";
    const char *digits = uppercase ? digits_upper : digits_lower;

    if (value == 0) {
        *--p = '0';
    } else {
        while (value != 0) {
            *--p = digits[value % base];
            value /= base;
        }
    }
    while (p < buf + sizeof(buf)) {
        uart0_putc(*p++);
    }
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    for (const char *p = fmt; *p != '\0'; ++p) {
        if (*p != '%') {
            uart0_putc(*p);
            continue;
        }
        ++p;
        switch (*p) {
        case 's': {
            const char *s = va_arg(args, const char *);
            uart0_puts(s ? s : "(null)");
            break;
        }
        case 'c':
            uart0_putc((char)va_arg(args, int));
            break;
        case 'd':
        case 'i': {
            int32_t v = va_arg(args, int32_t);
            if (v < 0) {
                uart0_putc('-');
                put_unsigned((uint32_t)(-(v + 1)) + 1u, 10, 0);
            } else {
                put_unsigned((uint32_t)v, 10, 0);
            }
            break;
        }
        case 'u':
            put_unsigned(va_arg(args, uint32_t), 10, 0);
            break;
        case 'x':
            put_unsigned(va_arg(args, uint32_t), 16, 0);
            break;
        case 'X':
            put_unsigned(va_arg(args, uint32_t), 16, 1);
            break;
        case 'p': {
            uintptr_t v = (uintptr_t)va_arg(args, void *);
            uart0_puts("0x");
            put_unsigned((uint32_t)v, 16, 0);
            break;
        }
        case '%':
            uart0_putc('%');
            break;
        case '\0':
            /* Trailing '%' at end of format: stop. */
            goto done;
        default:
            /* Unknown conversion: pass through verbatim. */
            uart0_putc('%');
            uart0_putc(*p);
            break;
        }
    }

done:
    va_end(args);
}
