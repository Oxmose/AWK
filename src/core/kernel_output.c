/*******************************************************************************
 *
 * File: kernel_output.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 1.0
 *
 * Simple output functions to print messages to screen. These are really basic
 * output too allow early kernel boot output and debug.
 ******************************************************************************/

#include "../lib/stdio.h"        /* printf, vprintf */
#include "../drivers/vga_text.h" /* save_color_scheme, set_color_sheme */

/* Header file */
#include "kernel_output.h"

void kernel_printf(const char *fmt, ...)
{
    /* Print tag */
    printf("[SYS] ");

    /* Prtinf format string */
    __builtin_va_list    args;
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_error(const char *fmt, ...)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_RED | BG_BLACK;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    printf("[ERROR] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_list    args;
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_success(const char *fmt, ...)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_GREEN | BG_BLACK;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    printf("[OK] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_list    args;
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_info(const char *fmt, ...)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_CYAN | BG_BLACK;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    printf("[INFO] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_list    args;
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}