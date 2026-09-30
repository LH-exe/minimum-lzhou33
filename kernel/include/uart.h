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

/* Enable UART0 RX interrupt generation and reset the receive buffer.
 * The interrupt controller and CPU IRQ flag are enabled separately. */
void uart0_init(void);

/* Called from the UART0 IRQ handler to store one received byte.
 * Drops the byte if the buffer is full; never blocks. */
void uart0_rx_push(unsigned char byte);

/* Non-blocking receive: returns -1 when the buffer is empty,
 * otherwise the next byte (0-255). */
int uart0_try_getc(void);

/* Blocking receive: spins until a byte is available. */
int uart0_getc(void);

#endif /* KERNEL_UART_H */
