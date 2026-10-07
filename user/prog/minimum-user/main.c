/* CS 492 - Homework 2, Task 1
 * User-mode hello world: prints via the ioctl syscall, then idles.
 * The kernel loader maps this as the "minimum-user" module.
 */
#include <stdint.h>

#include "minemu/user_abi.h"

void minemu_user_main(void) {
    static const char message[] = "hello world\n";
    (void)ioctl(MINEMU_FD_UART_OUT, MINEMU_IOCTL_UART_WRITE, (const void *)message,
                sizeof(message) - 1U);
    for (;;) {
        __asm__ volatile("nop");
    }
}
