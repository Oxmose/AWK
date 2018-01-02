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

#include "../lib/stdio.h"        /* vprintf, sprintf */
#include "../lib/string.h"       /* memset */
#include "../drivers/vga_text.h" /* save_color_scheme, set_color_sheme */
#include "../drivers/serial.h"   /* serial_write */
#include "interrupts.h"          /* get_current_uptime */

/* Header file */
#include "kernel_output.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

void kernel_printf(const char* fmt, ...)
{
    __builtin_va_list args;

    /* Prtinf format string */
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_error(const char* fmt, ...)
{
    __builtin_va_list args;
    colorscheme_t     buffer;
    colorscheme_t     new_scheme = FG_RED | BG_BLACK;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("[ERROR] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_success(const char* fmt, ...)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_GREEN | BG_BLACK;
    __builtin_va_list    args;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("[OK] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_info(const char* fmt, ...)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_CYAN | BG_BLACK;
    __builtin_va_list    args;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("[INFO] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_debug(const char* fmt, ...)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_YELLOW | BG_BLACK;
    __builtin_va_list    args;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("[DEBUG] ");

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Printf format string */
    __builtin_va_start(args, fmt);
    vprintf(fmt, args);
    __builtin_va_end(args);
}

void kernel_serial_debug(const char* fmt, ...)
{
    __builtin_va_list args;
    uint32_t          i;
    char              char_buffer[2048] = {0};

    sprintf(char_buffer, "[DEBUG %d]", get_current_uptime());
    for(i = 0; i < 2048 && char_buffer[i] != 0; ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, char_buffer[i]);
    }

    memset(char_buffer, 0, 2048 * sizeof(char));

    /* Print format string in buffer */
    __builtin_va_start(args, fmt);
    vsprintf(char_buffer, fmt, args);
    __builtin_va_end(args);

    /* Send to serial */
    for(i = 0; i < 2048 && char_buffer[i] != 0; ++i)
    {
        serial_write(SERIAL_DEBUG_PORT, char_buffer[i]);
    }
}