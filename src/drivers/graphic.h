/*******************************************************************************
 *
 * File: graphic.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/01/2018
 *
 * Version: 1.0
 *
 * Graphic drivers abtraction
 ******************************************************************************/

#ifndef __GRAPHIC_H_
#define __GRAPHIC_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define VGA_DRIVER_SELECTED  0x01
#define VESA_DRIVER_SELECTED 0x02

/* COLOR DEFINITIONS */
#define BG_BLACK            0x00
#define BG_BLUE             0x10
#define BG_GREEN            0x20
#define BG_CYAN             0x30
#define BG_RED              0x40
#define BG_MAGENTA          0x50
#define BG_BROWN            0x60
#define BG_GREY             0x70
#define BG_DARKGREY         0x80
#define BG_BRIGHTBLUE       0x90
#define BG_BRIGHTGREEN      0xA0
#define BG_BRIGHTCYAN       0xB0
#define BG_BRIGHTRED        0xC0
#define BG_BRIGHTMAGENTA    0xD0
#define BG_YELLOW           0xE0
#define BG_WHITE            0xF0

#define FG_BLACK            0x00
#define FG_BLUE             0x01
#define FG_GREEN            0x02
#define FG_CYAN             0x03
#define FG_RED              0x04
#define FG_MAGENTA          0x05
#define FG_BROWN            0x06
#define FG_GREY             0x07
#define FG_DARKGREY         0x08
#define FG_BRIGHTBLUE       0x09
#define FG_BRIGHTGREEN      0x0A
#define FG_BRIGHTCYAN       0x0B
#define FG_BRIGHTRED        0x0C
#define FG_BRIGHTMAGENTA    0x0D
#define FG_YELLOW           0x0E
#define FG_WHITE            0x0F
/* COLOR DEFINITIONS END */

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef enum GRAPHIC_DRIVER
{
    VGA_DRIVER  = VGA_DRIVER_SELECTED,
    VESA_DRIVER = VESA_DRIVER_SELECTED
} GRAPHIC_DRIVER_E;

/* Cursor structure */
typedef struct cursor
{
    uint32_t x;
    uint32_t y;
} cursor_t;

/* Scroll direction enum */
typedef enum SCROLL_DIRECTION
{
    SCROLL_DOWN,
    SCROLL_UP
} SCROLL_DIRECTION_E;

/* Screen color scheme struct */
typedef struct colorscheme
{
    uint32_t foreground;
    uint32_t background;
    uint8_t  vga_color;
} colorscheme_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Set the current selected driver.
 *
 * @param sel The driver signature to select.
 */
void set_selected_driver(const GRAPHIC_DRIVER_E sel);

/* Clear the screen */
void clear_screen(void);

/* Place the cursor to the selected coordinates given as parameters.
 *
 * @param line The line index where to place the cursor.
 * @param column The column index where to place the cursor.
 * @returns The error or success state.
 */
OS_RETURN_E put_cursor_at(const uint32_t line, const uint32_t column);

/* Save the cursor attributes in the buffer given as paramter
 *
 * @param buffer The cursor buffer in which the current cursor possition is
 * going to be saved.
 */
OS_RETURN_E save_cursor(cursor_t* buffer);

/* Restore The cursor attributes from the buffer given as parameter.
 *
 * @param buffer The cursor buffer containing the new coordinates of the
 * cursor.
 */
OS_RETURN_E restore_cursor(const cursor_t buffer);

/* Scroll in the desired direction of lines_count lines.
 *
 * @param direction The direction to whoch the console should be scrolled.
 * @param lines_count The number of lines to scroll.
 */
void scroll(const SCROLL_DIRECTION_E direction,
            const uint32_t lines_count);

/* Set the color scheme of the screen.
 *
 * @param color_scheme The new color scheme to apply to the screen console.
 * @param vga_color If set to 1 and in VESA mode then the colors are intepreted
 * as 16 color VGA text scheme.
 */
void set_color_scheme(colorscheme_t color_scheme);

/* Save the color scheme in the buffer given as parameter.
 *
 * @param buffer The buffer that will receive the current color scheme used by
 * the screen console.
 */
OS_RETURN_E save_color_scheme(colorscheme_t* buffer);

/* Used by the kernel to display strings on the screen.
 *
 * @param str The string to display on the screen.
 * @param len The length of the string to display.
 */
void console_putbytes(const char* str, const uint32_t len);

/* Used by the kernel to display strings on the screen from a keyboard input.
 *
 * @param str The string to display on the screen from a keybaord input.
 * @param len The length of the string to display.
 */
void console_write_keyboard(const char* str, const uint32_t len);

#endif /* __GRAPHIC_H_ */