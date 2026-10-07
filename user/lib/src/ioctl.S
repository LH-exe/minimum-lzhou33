/* CS 492 - Homework 2, Task 1
 * User-side ioctl wrapper. C passes fd/request/args in r0-r3; we set the
 * syscall number in r7, trap, restore r7, and return the kernel result.
 */
#include "minemu/user_abi.h"

.syntax unified
.cpu cortex-a9
.arm

.global ioctl
.type ioctl, %function

ioctl:
    push {r7, lr}
    mov r7, #MINEMU_SYSCALL_IOCTL
    svc #0
    pop {r7, pc}
