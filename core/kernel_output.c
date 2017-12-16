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

/* Generic int types */
#include "../lib/stdint.h"

/* console_putbytes, process_char, colorscheme_t, save_color_scheme, 
 * set_color_scheme
 */
#include "../drivers/vga_text.h"

void kernel_print(const char *string, uint32_t size)
{
    console_putbytes(string, size);
}

void kernel_error(const char *string, uint32_t size)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_RED | BG_BLACK;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    console_putbytes("[ERROR] ", 8);

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Print message */
    console_putbytes(string, size);
}

void kernel_success(const char *string, uint32_t size)
{
    colorscheme_t buffer;
    colorscheme_t new_scheme = FG_GREEN | BG_BLACK;

    /* No need to test return value */
    save_color_scheme(&buffer);

    /* Set GREEN on BLACK color scheme */
    set_color_scheme(new_scheme);

    /* Print tag */
    console_putbytes("[OK] ", 5);

    /* Restore original screen color scheme */
    set_color_scheme(buffer);

    /* Print message */
    console_putbytes(string, size);
}

void kernel_print_unsigned_hex(const uint32_t value, uint32_t size)
{
    int8_t i;
    char val;

    process_char('0');
    process_char('x');

    if(size == 0)
        return;

    --size;

    if(size > 7)
    {
        size = 7;
    }

    for(i = 7 - (7 - size); i >= 0; --i)
    {
        val = (char)((value >> (i * 4) & 0xF));
        if(val > 9)
            val += 55;
        else
            val += 48;
        process_char(val);
    }
}

void kernel_print_unsigned64_hex(const uint32_t value, uint32_t size)
{
    int8_t i;
    char val;

    process_char('0');
    process_char('x');

    if(size > 15)
    {
        size = 15;
    }

    for(i = 15 - (15- size); i >= 0; --i)
    {
        val = (char)((value >> (i * 4) & 0xF));
        if(val > 9)
            val += 55;
        else
            val += 48;
        process_char(val);
    }
}