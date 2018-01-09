/*******************************************************************************
 *
 * File: kernel_output.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 2.0
 *
 * Simple output functions to print messages to screen. These are really basic
 * output too allow early kernel boot output and debug.
 ******************************************************************************/

#include "../lib/string.h"       /* memset, strlen */
#include "../lib/stdlib.h"       /* uitoa, itoa */
#include "../drivers/graphic.h"  /* save_color_scheme, set_color_sheme,
                                  * screen_put_char, screen_put_string */
#include "../drivers/serial.h"   /* serial_put_char, serial_put_string */

/* Header file */
#include "kernel_output.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

static volatile output_t current_output;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static void toupper(char* string)
{
    while(*string != 0)
    {
        if(*string > 96 && *string < 123)
        {
            *string = *string - 32;
        }
        ++string;
    }
}

static void tolower(char* string)
{
    while(*string != 0)
    {
        if(*string > 64 && *string < 91)
        {
            *string = *string + 32;
        }
        ++string;
    }
}

static void kprint_fmt(const char* str, __builtin_va_list args)
{
    uint32_t i;
    int32_t  char_val;
    char*    args_value;
    char     tmp[32];
    int32_t  str_size;
    uint32_t offset;
    char     char_padding = ' ';
    int32_t  padding = -1;

	for(i = 0; i < strlen(str); ++i)
	{
		if(str[i] == '%')
		{
            offset = 1;
            /* Search for padding */
            if((str[i + offset] >= 48 && str[i + offset] <= 57))
            {
                char_padding = str[i + offset];
                ++offset;

                /* Search for padding size */
                if((str[i + offset] >= 48 && str[i + offset] <= 57))
                {
                    padding = str[i + offset] - 48;
                    ++offset;
                }
                else
                {
                    padding = 0;
                }
            }

            /* Search for format */
			switch(str[i + offset])
			{
				case 's':
					args_value = __builtin_va_arg(args, char*);
					current_output.puts(args_value);
                    i += offset;
					continue;
                case 'i':
				case 'd':

					char_val = __builtin_va_arg(args, int32_t);
					memset(tmp, 0, sizeof(tmp));
					itoa(char_val, tmp, 10);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
					current_output.puts(tmp);
                    i += offset;
					continue;
                case 'u':
                    char_val = __builtin_va_arg(args, uint32_t);
                    memset(tmp, 0, sizeof(tmp));
                    uitoa(char_val, tmp, 10);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
                    current_output.puts(tmp);
                    i += offset;
					continue;
				case 'x':
					char_val = __builtin_va_arg(args, uint32_t);
					memset(tmp, 0, sizeof(tmp));
					uitoa(char_val, tmp, 16);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
                    tolower(tmp);
					current_output.puts(tmp);
                    i += offset;
					continue;
                case 'X':
                    char_val = __builtin_va_arg(args, uint32_t);
                    memset(tmp, 0, sizeof(tmp));
                    uitoa(char_val, tmp, 16);
                    str_size = strlen(tmp);
                    while(padding > str_size)
                    {
                        current_output.putc(char_padding);
                        --padding;
                    }
                    toupper(tmp);
                    current_output.puts(tmp);
                    i += offset;
					continue;
				case 'c':
					tmp[0] = (char)
                        ((__builtin_va_arg(args, int32_t) & 0x000000FF));
					current_output.putc(tmp[0]);
                    i += offset;
					continue;
				default:
                    ++i;
                    continue;
			}
		}
        else
        {
            padding = -1;
			current_output.putc(str[i]);
		}
	}
}

void kernel_printf(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }

    /* Prtinf format string */
    __builtin_va_start(args, fmt);
    current_output.putc = screen_put_char;
    current_output.puts = screen_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_error(const char* fmt, ...)
{
    __builtin_va_list args;
    colorscheme_t     buffer;
    colorscheme_t     new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_RED;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

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
    current_output.putc = screen_put_char;
    current_output.puts = screen_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_success(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_GREEN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

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
    current_output.putc = screen_put_char;
    current_output.puts = screen_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_info(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_CYAN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

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
    current_output.putc = screen_put_char;
    current_output.puts = screen_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_debug(const char* fmt, ...)
{
    __builtin_va_list    args;
    colorscheme_t        buffer;
    colorscheme_t        new_scheme;

    if(fmt == NULL)
    {
        return;
    }

    new_scheme.foreground = FG_YELLOW;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

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
    current_output.putc = screen_put_char;
    current_output.puts = screen_put_string;
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}

void kernel_serial_debug(const char* fmt, ...)
{
    __builtin_va_list args;

    if(fmt == NULL)
    {
        return;
    }

    __builtin_va_start(args, fmt);
    current_output.putc = serial_put_char;
    current_output.puts = serial_put_string;
    kprint_fmt("[DEBUG] ", args);
    kprint_fmt(fmt, args);
    __builtin_va_end(args);
}
