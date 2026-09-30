/* Author: Lin Han Zhou (lzhou33)
 * CS 492 - Homework 1, Task 2
 * Minimum shell (msh): prompt, line reader, and command execution.
 */
#include "msh.h"
#include "uart.h"

#include <stddef.h>

/* Longest accepted command line, in bytes. Longer input is consumed through
 * the terminating newline and then discarded. Sized to accept the required
 * echo lines (e.g. the 25-byte "echo hello from user mode") while the
 * 120-byte overlong case is still discarded. */
#define MSH_MAX_LINE 64

static int msh_is_echo(const char *line, size_t start, size_t len) {
    return len == 4 && line[start] == 'e' && line[start + 1] == 'c' &&
           line[start + 2] == 'h' && line[start + 3] == 'o';
}

static void msh_do_echo(const char *line, size_t len, size_t arg) {
    /* Skip the separator spaces after the command word. */
    while (arg < len && line[arg] == ' ') {
        ++arg;
    }
    /* Print words separated by single spaces; runs of spaces collapse and
     * trailing spaces are dropped (a space prints only before a word). */
    while (arg < len) {
        if (line[arg] == ' ') {
            while (arg < len && line[arg] == ' ') {
                ++arg;
            }
            if (arg < len) {
                uart0_putc(' ');
            }
            continue;
        }
        uart0_putc(line[arg]);
        ++arg;
    }
    uart0_putc('\n');
}

static void msh_execute(const char *line, size_t len) {
    size_t i = 0;

    /* Leading spaces are ignored. */
    while (i < len && line[i] == ' ') {
        ++i;
    }
    if (i == len) {
        return;
    }

    size_t cmd = i;
    while (i < len && line[i] != ' ') {
        ++i;
    }
    size_t cmd_len = i - cmd;

    if (msh_is_echo(line, cmd, cmd_len)) {
        msh_do_echo(line, len, i);
        return;
    }

    uart0_puts("command not found: ");
    for (size_t k = 0; k < cmd_len; ++k) {
        uart0_putc(line[cmd + k]);
    }
    uart0_putc('\n');
}

void msh_run(void) {
    char line[MSH_MAX_LINE];

    for (;;) {
        size_t len = 0;
        int overlong = 0;

        uart0_puts("msh> ");
        for (;;) {
            int c = uart0_getc();
            if (c == '\r') {
                continue;
            }
            if (c == '\n') {
                break;
            }
            if (c == 0x08 || c == 0x7f) {
                /* Backspace deletes one buffered byte; ignored on an
                 * empty line. While overlong the line is already doomed,
                 * so keep discarding through the newline. */
                if (!overlong && len > 0) {
                    --len;
                }
                continue;
            }
            if (overlong) {
                continue;
            }
            if (len < MSH_MAX_LINE) {
                line[len++] = (char)c;
            } else {
                overlong = 1;
            }
        }
        if (overlong) {
            continue;
        }
        msh_execute(line, len);
    }
}
