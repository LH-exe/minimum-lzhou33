/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 2, Task 1
 * Kernel entry point. Boot-info validation is supplied template code
 * (retained per assignment instructions); afterwards the kernel loads the
 * "minimum-user" module into user space and never returns.
 * No trace output on success paths: the hw2 fixtures assert exact trace
 * histories produced only by the user program.
 */
#include "minemu/boot.h"
#include "minemu/trap.h"

void usermode_boot(const struct minemu_boot_info *boot_info) __attribute__((noreturn));

void minemu_kernel_main(const struct minemu_boot_info *boot_info) {
    if ((uintptr_t)boot_info != MINEMU_BOOT_INFO_VADDR ||
        boot_info->magic != MINEMU_BOOT_INFO_MAGIC ||
        boot_info->version != MINEMU_ABI_VERSION ||
        boot_info->size != sizeof(*boot_info) ||
        boot_info->system_rom_base != UINT32_C(0x08000000) ||
        boot_info->direct_map_vaddr != UINT32_C(0xc0000000) ||
        boot_info->direct_map_paddr != UINT32_C(0x40000000) ||
        boot_info->direct_map_size != UINT32_C(0x04000000)) {
        minemu_fail_stop();
    }

    usermode_boot(boot_info);
}
