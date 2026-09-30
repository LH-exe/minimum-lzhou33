/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 1
 * UART0 transmit driver for the minimum kernel.
 *
 * Polling TX only for now; RX interrupt support arrives in Task 2 (msh).
 * All register accesses are single aligned 32-bit reads/writes through the
 * supplied volatile structs in minemu/platform.h, per the MMIO rules.
 */
#include "uart.h"
#include "minemu/platform.h"

void uart0_putc(char c) {
    /* Wait until the transmit register will accept a byte. */
    while ((MINEMU_UART0->status & MINEMU_UART_STATUS_TX_READY) == 0) {
        /* spin */
    }
    MINEMU_UART0->tx_data = (uint32_t)(unsigned char)c;
}

void uart0_puts(const char *s) {
    for (const char *p = s; *p != '\0'; ++p) {
        uart0_putc(*p);
    }
}
