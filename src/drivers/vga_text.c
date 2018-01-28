/*******************************************************************************
 *
 * File: vga_text.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.5
 *
 * VGA text mode driver.
 * Allows the kernel to display text and general ASCII characters to be
 * displayed on the screen.
 * Includes cursor management, screen colors management and other fancy
 * screen driver things.
 ******************************************************************************/

#include "../lib/stdint.h"     /* Generic int types */
#include "../lib/stddef.h"     /* OS_RETURN_E, NULL */
#include "../lib/string.h"     /* memmove */
#include "../cpu/cpu.h"        /* outb */
#include "../drivers/serial.h" /* serial_write */
#include "graphic.h"            /* structures */

#include "../debug.h"      /* kernel_serial_debug */

/* Header file */
#include "vga_text.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* Screen runtime parameters */
static colorscheme_t screen_scheme = {
    .background = BG_BLACK,
    .foreground = FG_WHITE,
    .vga_color  = 1
};

static cursor_t      screen_cursor;
static cursor_t      last_printed_cursor;

/* Set the last column printed with a char */
static uint8_t last_columns[SCREEN_LINE_SIZE] = {0};

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Print character to the selected coordinates
 *
 * @param line The line index where to write the character.
 * @param column The colums index where to write the character.
 * @param character The character to display on the screem.
 * @returns The error or success state.
 */
static OS_RETURN_E vga_print_char(const uint8_t line, const uint8_t column,
                                  const char character)
{
    uint16_t* screen_mem;

    if(line > SCREEN_LINE_SIZE - 1 || column > SCREEN_COL_SIZE - 1)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Get address to inject */
    screen_mem = vga_get_framebuffer(line, column);

    /* Inject the character with the current colorscheme */
    *screen_mem = character |
                  ((screen_scheme.background << 8) & 0xF000) |
                  ((screen_scheme.foreground << 8) & 0x0F00);

    return OS_NO_ERR;
}

/* Process the character in parameters.
 *
 * @param character The character to process.
 */
static void vga_process_char(const char character)
{
    #ifdef KERNEL_DEBUG
    /* Write on serial */
    serial_write(COM1, character);
    #endif

    /* If character is a normal ASCII character */
    if(character > 31 && character < 127)
    {
        /* Display character and move cursor */
        vga_print_char(screen_cursor.y, screen_cursor.x++,
                character);

        /* Manage end of line cursor position */
        if(screen_cursor.x > SCREEN_COL_SIZE - 1)
        {
            vga_put_cursor_at(screen_cursor.y + 1, 0);
            last_columns[screen_cursor.y] = screen_cursor.x;
        }

        /* Manage end of screen cursor position */
        if(screen_cursor.y >= SCREEN_LINE_SIZE)
        {
            vga_scroll(SCROLL_DOWN, 1);

        }
        else
        {
            /* Move cursor */
            vga_put_cursor_at(screen_cursor.y, screen_cursor.x);
            last_columns[screen_cursor.y] = screen_cursor.x;
        }
    }
    else
    {
        /* Manage special ACSII characters*/
        switch(character)
        {
            /* Backspace */
            case '\b':
                if(last_printed_cursor.y == screen_cursor.y)
                {
                    if(screen_cursor.x > last_printed_cursor.x)
                    {
                        vga_put_cursor_at(screen_cursor.y, screen_cursor.x - 1);
                        last_columns[screen_cursor.y] = screen_cursor.x;
                        vga_print_char(screen_cursor.y, screen_cursor.x, ' ');
                    }
                }
                else if(last_printed_cursor.y < screen_cursor.y)
                {
                    if(screen_cursor.x > 0)
                    {
                        vga_put_cursor_at(screen_cursor.y, screen_cursor.x - 1);
                        last_columns[screen_cursor.y] = screen_cursor.x;
                        vga_print_char(screen_cursor.y, screen_cursor.x, ' ');
                    }
                    else
                    {
                        if(last_columns[screen_cursor.y - 1] >=
                               SCREEN_COL_SIZE)
                        {
                            last_columns[screen_cursor.y - 1] =
                                SCREEN_COL_SIZE - 1;
                        }

                        vga_put_cursor_at(screen_cursor.y - 1,
                                      last_columns[screen_cursor.y - 1]);
                        vga_print_char(screen_cursor.y, screen_cursor.x, ' ');
                    }
                }
                break;
            /* Tab */
            case '\t':
                if(screen_cursor.x + 8 < SCREEN_COL_SIZE - 1)
                {
                    vga_put_cursor_at(screen_cursor.y,
                            screen_cursor.x  +
                            (8 - screen_cursor.x % 8));
                }
                else
                {
                    vga_put_cursor_at(screen_cursor.y,
                            SCREEN_COL_SIZE - 1);
                }
                last_columns[screen_cursor.y] = screen_cursor.x;
                break;
            /* Line feed */
            case '\n':
                if(screen_cursor.y < SCREEN_LINE_SIZE - 1)
                {
                    vga_put_cursor_at(screen_cursor.y + 1, 0);
                    last_columns[screen_cursor.y] = screen_cursor.x;
                }
                else
                {
                    vga_scroll(SCROLL_DOWN, 1);
                }
                break;
            /* Clear screen */
            case '\f':
                vga_clear_screen();
                break;
            /* Line return */
            case '\r':
                vga_put_cursor_at(screen_cursor.y, 0);
                last_columns[screen_cursor.y] = screen_cursor.x;
                break;
            /* Undefined */
            default:
                break;
        }
    }
}

uint16_t* vga_get_framebuffer(const uint8_t line, const uint8_t column)
{
    /* Avoid overflow on text mode */
    if(line > SCREEN_LINE_SIZE - 1 || column > SCREEN_COL_SIZE -1)
    {
        return (uint16_t*)(VGA_TEXT_FRAMEBUFFER);
    }

    /* Returns the mem adress of the coordinates */
    return (uint16_t*)(VGA_TEXT_FRAMEBUFFER + 2 *
           (column + line * SCREEN_COL_SIZE));
}

void vga_clear_screen(void)
{
    uint32_t i;
    uint32_t j;
    uint16_t blank = ' ' |
                     ((screen_scheme.background << 8) & 0xF000) |
                     ((screen_scheme.foreground << 8) & 0x0F00);

    /* Clear all screen cases */
    for(i = 0; i < SCREEN_LINE_SIZE; ++i)
    {
        for(j = 0; j < SCREEN_COL_SIZE; ++j)
        {
            *(vga_get_framebuffer(i, j)) = blank;
        }
        last_columns[i] = 0;
    }
}

OS_RETURN_E vga_put_cursor_at(const uint8_t line, const uint8_t column)
{
    int16_t cursor_position;

    /* Set new cursor position */
    screen_cursor.x = column;
    screen_cursor.y = line;

    /* Display new position on screen */
    cursor_position = column + line * SCREEN_COL_SIZE;
    /* Send low part to the screen */
    outb(CURSOR_COMM_LOW, SCREEN_COMM_PORT);
    outb((int8_t)(cursor_position & 0x00FF), SCREEN_DATA_PORT);

    /* Send high part to the screen */
    outb(CURSOR_COMM_HIGH, SCREEN_COMM_PORT);
    outb((int8_t)((cursor_position & 0xFF00) >> 8), SCREEN_DATA_PORT);

    return OS_NO_ERR;
}

OS_RETURN_E vga_save_cursor(cursor_t* buffer)
{
    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Save cursor attributes */
    buffer->x = screen_cursor.x;
    buffer->y = screen_cursor.y;

    return OS_NO_ERR;
}

OS_RETURN_E vga_restore_cursor(const cursor_t buffer)
{
    if(buffer.x >= SCREEN_COL_SIZE || buffer.y >= SCREEN_LINE_SIZE)
    {
        return OS_ERR_OUT_OF_BOUND;
    }
    /* Restore cursor attributes */
    vga_put_cursor_at(buffer.y, buffer.x);

    return OS_NO_ERR;
}

void vga_scroll(const SCROLL_DIRECTION_E direction, const uint8_t lines_count)
{
    uint8_t to_scroll;
    uint8_t i;
    uint8_t j;

    if(SCREEN_LINE_SIZE < lines_count)
    {
        to_scroll = SCREEN_LINE_SIZE;
    }
    else
    {
        to_scroll = lines_count;
    }

    /* Select scroll direction */
    if(direction == SCROLL_DOWN)
    {
        /* For each line scroll we want */
        for(j = 0; j < to_scroll; ++j)
        {
            /* Copy all the lines to the above one */
            for(i = 0; i < SCREEN_LINE_SIZE - 1; ++i)
            {
                memmove(vga_get_framebuffer(i, 0),
                        vga_get_framebuffer(i + 1, 0),
                        sizeof(uint16_t) * SCREEN_COL_SIZE);
                last_columns[i] = last_columns[i+1];
            }
            last_columns[SCREEN_LINE_SIZE - 1] = 0;
        }
        /* Clear last line */
        for(i = 0; i < SCREEN_COL_SIZE; ++i)
        {
            vga_print_char(SCREEN_LINE_SIZE - 1, i, ' ');
        }
    }

    /* Replace cursor */
    vga_put_cursor_at(SCREEN_LINE_SIZE - to_scroll, 0);

    if(to_scroll <= last_printed_cursor.y)
    {
        last_printed_cursor.y -= to_scroll;
    }
    else
    {
        last_printed_cursor.x = 0;
        last_printed_cursor.y = 0;
    }
}

void vga_set_color_scheme(const colorscheme_t color_scheme)
{
    screen_scheme.foreground = color_scheme.foreground;
    screen_scheme.background = color_scheme.background;
}

OS_RETURN_E vga_save_color_scheme(colorscheme_t* buffer)
{
    if(buffer == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Save color scheme into buffer */
    buffer->foreground = screen_scheme.foreground;
    buffer->background = screen_scheme.background;

    return OS_NO_ERR;
}

void vga_put_string(const char* string)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < strlen(string); ++i)
    {
        vga_process_char(string[i]);
    }
    last_printed_cursor = screen_cursor;
}

void vga_put_char(const char character)
{
    vga_process_char(character);
    last_printed_cursor = screen_cursor;
}

void vga_console_write_keyboard(const char* string, const uint32_t size)
{
    /* Output each character of the string */
    uint32_t i;
    for(i = 0; i < size; ++i)
    {
        vga_process_char(string[i]);
    }
}
