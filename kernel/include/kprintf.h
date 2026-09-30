/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 1
 * Minimal printf-like console logging for the minimum kernel.
 */
#ifndef KERNEL_KPRINTF_H
#define KERNEL_KPRINTF_H

/* Formatted print to UART0.
 * Supported conversions: %s %c %d %i %u %x %X %p %%
 * No field widths, precision, or padding (not needed for this platform).
 */
void kprintf(const char *fmt, ...);

#endif /* KERNEL_KPRINTF_H */
