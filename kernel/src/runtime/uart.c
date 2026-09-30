/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 1
 * UART0 transmit driver for the minimum kernel.
 *
 * Polling TX only for now; RX interrupt support arrives in Task 2 (msh).
 * All register accesses are single aligned 32-bit reads/writes through the
 * supplied volatile structs in minemu/platform.h, per the MMIO rules.
 */
#include "uart.h"
#include "minemu/irq.h"
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

/* Interrupt-driven receive buffer shared between the UART0 IRQ handler
 * (producer, advances head) and the shell (consumer, advances tail).
 * The handler runs with IRQs masked by hardware; the consumer masks IRQs
 * around every access so head/tail are never observed mid-update. */
#define UART0_RX_CAPACITY 1024

static unsigned char uart0_rx_buf[UART0_RX_CAPACITY];
static volatile uint32_t uart0_rx_head;
static volatile uint32_t uart0_rx_tail;

void uart0_init(void) {
    uart0_rx_head = 0;
    uart0_rx_tail = 0;
    MINEMU_UART0->control = MINEMU_UART_CONTROL_RX_IRQ_ENABLE;
}

void uart0_rx_push(unsigned char byte) {
    uint32_t next = (uart0_rx_head + 1) % UART0_RX_CAPACITY;
    if (next == uart0_rx_tail) {
        return;
    }
    uart0_rx_buf[uart0_rx_head] = byte;
    uart0_rx_head = next;
}

int uart0_try_getc(void) {
    int result;
    minemu_irq_disable();
    if (uart0_rx_head == uart0_rx_tail) {
        result = -1;
    } else {
        result = (int)uart0_rx_buf[uart0_rx_tail];
        uart0_rx_tail = (uart0_rx_tail + 1) % UART0_RX_CAPACITY;
    }
    minemu_irq_enable();
    return result;
}

int uart0_getc(void) {
    int c;
    do {
        c = uart0_try_getc();
    } while (c < 0);
    return c;
}
