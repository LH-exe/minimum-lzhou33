/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 2, Task 1
 * SVC dispatcher implementing the shared ioctl ABI (minemu/user_abi.h).
 * Register convention: r7 = syscall number (1 = ioctl), r0 = fd,
 * r1 = request, r2/r3 = up to two device-specific arguments.
 * Every user-memory range is validated before any visible effect; the
 * result is written to the saved r0 so the user resumes with it.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "minemu/platform.h"
#include "minemu/syscall.h"
#include "minemu/trap.h"
#include "minemu/user_abi.h"

#define CPSR_MODE_MASK UINT32_C(0x1f)
#define CPSR_MODE_USR UINT32_C(0x10)
#define CPSR_THUMB UINT32_C(0x20)
#define A32_SVC_ZERO UINT32_C(0xef000000)

bool user_range_valid(uint32_t vaddr, uint32_t length, bool need_write);
void *user_ptr(uint32_t vaddr);

static volatile struct minemu_uart_regs *uart_for_fd(int fd) {
    switch (fd) {
    case MINEMU_FD_UART_IN:
    case MINEMU_FD_UART_OUT:
        return MINEMU_UART0;
    case MINEMU_FD_UART_DIAG:
        return MINEMU_UART1;
    default:
        return NULL;
    }
}

static int32_t uart_write_op(volatile struct minemu_uart_regs *uart, uint32_t src,
                             uint32_t length) {
    if (length > MINEMU_IOCTL_MAX_TRANSFER) {
        length = MINEMU_IOCTL_MAX_TRANSFER;
    }
    if (!user_range_valid(src, length, false)) {
        return MINEMU_IOCTL_ERROR;
    }
    for (uint32_t index = 0; index < length; ++index) {
        uint8_t *byte = user_ptr(src + index);
        if (byte == NULL) {
            return MINEMU_IOCTL_ERROR;
        }
        while ((uart->status & MINEMU_UART_STATUS_TX_READY) == 0) {
            /* spin: TX_READY is always set per the device ABI */
        }
        uart->tx_data = (uint32_t)*byte;
    }
    return (int32_t)length;
}

static int32_t uart_read_op(volatile struct minemu_uart_regs *uart, uint32_t dst,
                            uint32_t max_length) {
    if (max_length > MINEMU_IOCTL_MAX_TRANSFER) {
        max_length = MINEMU_IOCTL_MAX_TRANSFER;
    }
    if (!user_range_valid(dst, max_length, true)) {
        return MINEMU_IOCTL_ERROR;
    }
    uint32_t count = 0;
    while (count < max_length && (uart->status & MINEMU_UART_STATUS_RX_READY) != 0) {
        uint8_t byte = (uint8_t)(uart->rx_data & UINT32_C(0xff));
        uint8_t *slot = user_ptr(dst + count);
        if (slot == NULL) {
            return MINEMU_IOCTL_ERROR;
        }
        *slot = byte;
        count++;
    }
    return (int32_t)count;
}

static int32_t uart_status_op(volatile struct minemu_uart_regs *uart) {
    uint32_t status = uart->status;
    int32_t result = 0;
    if ((status & MINEMU_UART_STATUS_RX_READY) != 0) {
        result |= MINEMU_IOCTL_UART_RX_READY;
    }
    if ((status & MINEMU_UART_STATUS_TX_READY) != 0) {
        result |= MINEMU_IOCTL_UART_TX_READY;
    }
    return result;
}

static int32_t rng_state_op(uint32_t out) {
    if (!user_range_valid(out, sizeof(uint32_t), true)) {
        return MINEMU_IOCTL_ERROR;
    }
    uint32_t *slot = user_ptr(out);
    if (slot == NULL) {
        return MINEMU_IOCTL_ERROR;
    }
    *slot = MINEMU_RNG->state;
    return 0;
}

static int32_t rng_next_op(uint32_t out) {
    if (!user_range_valid(out, sizeof(uint32_t), true)) {
        return MINEMU_IOCTL_ERROR;
    }
    uint32_t *slot = user_ptr(out);
    if (slot == NULL) {
        return MINEMU_IOCTL_ERROR;
    }
    *slot = MINEMU_RNG->data;
    return 0;
}

struct minemu_trap_frame *minemu_svc_dispatch(struct minemu_trap_frame *frame) {
    int32_t result = MINEMU_IOCTL_ERROR;

    uint32_t mode = frame->spsr & CPSR_MODE_MASK;
    if (frame->exception_id != MINEMU_EXCEPTION_SVC || mode != CPSR_MODE_USR ||
        (frame->spsr & CPSR_THUMB) != 0) {
        frame->r[0] = (uint32_t)result;
        return frame;
    }

    if (*(const uint32_t *)(uintptr_t)frame->fault_pc != A32_SVC_ZERO ||
        frame->r[7] != (uint32_t)MINEMU_SYSCALL_IOCTL) {
        frame->r[0] = (uint32_t)result;
        return frame;
    }

    int fd = (int)frame->r[0];
    uint32_t request = frame->r[1];
    uint32_t arg2 = frame->r[2];
    uint32_t arg3 = frame->r[3];

    switch (request) {
    case MINEMU_IOCTL_UART_READ: {
        volatile struct minemu_uart_regs *uart = uart_for_fd(fd);
        if (uart != NULL) {
            result = uart_read_op(uart, arg2, arg3);
        }
        break;
    }
    case MINEMU_IOCTL_UART_WRITE: {
        volatile struct minemu_uart_regs *uart = uart_for_fd(fd);
        if (uart != NULL) {
            result = uart_write_op(uart, arg2, arg3);
        }
        break;
    }
    case MINEMU_IOCTL_UART_STATUS: {
        volatile struct minemu_uart_regs *uart = uart_for_fd(fd);
        if (uart != NULL) {
            result = uart_status_op(uart);
        }
        break;
    }
    case MINEMU_IOCTL_RNG_STATE:
        if (fd == MINEMU_FD_RNG) {
            result = rng_state_op(arg2);
        }
        break;
    case MINEMU_IOCTL_RNG_NEXT:
        if (fd == MINEMU_FD_RNG) {
            result = rng_next_op(arg2);
        }
        break;
    case MINEMU_IOCTL_RNG_SEED:
        if (fd == MINEMU_FD_RNG) {
            MINEMU_RNG->seed = arg2;
            result = 0;
        }
        break;
    case MINEMU_IOCTL_TRACE_EVENT:
        if (fd == MINEMU_FD_TRACE) {
            MINEMU_TRACE->event = arg2;
            result = 0;
        }
        break;
    default:
        break;
    }

    frame->r[0] = (uint32_t)result;
    return frame;
}
