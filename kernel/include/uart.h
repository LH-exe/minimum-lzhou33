/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 1
 * UART0 driver API for the minimum kernel.
 */
#ifndef KERNEL_UART_H
#define KERNEL_UART_H

/* Transmit a single character on UART0 (polls until TX is ready). */
void uart0_putc(char c);

/* Transmit a NUL-terminated string on UART0. */
void uart0_puts(const char *s);

#endif /* KERNEL_UART_H */
