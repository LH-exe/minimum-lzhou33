/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 2
 * IRQ dispatch policy and per-source handlers (UART0 RX drain).
 */
#include "minemu/irq.h"
#include "minemu/platform.h"
#include "uart.h"

/* Per-source IRQ handlers. Only UART0 is enabled for HW1; the rest are
 * stubs that clear their device condition so no interrupt stays asserted. */
static void irq_handle_systick(void) {
    MINEMU_SYSTICK->ack = MINEMU_SYSTICK_ACK;
}

static void irq_handle_uart0(void) {
    /* Drain every pending byte: unread bytes keep the RX condition asserted
     * and the interrupt would re-fire immediately after EOI. */
    while ((MINEMU_UART0->status & MINEMU_UART_STATUS_RX_READY) != 0) {
        uart0_rx_push((unsigned char)(MINEMU_UART0->rx_data & UINT32_C(0xff)));
    }
}

static void irq_handle_uart1(void) {
    while ((MINEMU_UART1->status & MINEMU_UART_STATUS_RX_READY) != 0) {
        (void)MINEMU_UART1->rx_data;
    }
}

static void irq_handle_block(void) {
    MINEMU_BLOCK->ack = MINEMU_BLOCK_ACK;
}

static void (*const irq_handlers[4])(void) = {
    irq_handle_systick,
    irq_handle_uart0,
    irq_handle_uart1,
    irq_handle_block,
};

struct minemu_trap_frame *minemu_irq_dispatch(struct minemu_trap_frame *frame) {
    uint32_t source = (uint32_t)frame->exception_id;

    if (source < 4) {
        irq_handlers[source]();
    }
    /* Unknown source IDs have no device state to clear; still retire them. */
    MINEMU_INTERRUPT->eoi = source;
    return frame;
}
