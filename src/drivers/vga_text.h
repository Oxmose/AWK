/*******************************************************************************
 *
 * File: vga_text.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * VGA text mode driver.
 * Allows the kernel to display text and general ASCII characters to be
 * displayed on the screen.
 * Includes cursor management, screen colors management and other fancy
 * screen driver things.
 *
 ******************************************************************************/

#ifndef __VGA_TEXT_H_
#define __VGA_TEXT_H_

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E, NULL */
#include "graphic.h"            /* structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/* SCREEN SETTINGS */
#define VGA_TEXT_FRAMEBUFFER   0xB8000
#define SCREEN_DATA_PORT       0x3D5
#define SCREEN_COMM_PORT       0x3D4
#define SCREEN_COL_SIZE        80
#define SCREEN_LINE_SIZE       25

/* CURSOR SETTINGS */
#define CURSOR_COMM_LOW  0x0F
#define CURSOR_COMM_HIGH 0x0E

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Return the memory address of the screen framebuffer position at the
 * coordinates
 * given as arguments.
 *
 * @returns The address at which the driver has to write the bytes to display.
 * @param line The line index of the coordinates.
 * @param column The column index of the coordinates.
 */
uint16_t* vga_get_framebuffer(const uint8_t line, const uint8_t column);

/* Clear the screen by printing space character on black background */
void vga_clear_screen(void);

/* Place the cursor to the selected coordinates given as parameters.
 *
 * @param line The line index where to place the cursor.
 * @param column The column index where to place the cursor.
 * @returns The error or success state.
 */
OS_RETURN_E vga_put_cursor_at(const uint8_t line, const uint8_t column);

/* Save the cursor attributes in the buffer given as paramter
 *
 * @param buffer The cursor buffer in which the current cursor possition is
 * going to be saved.
 */
OS_RETURN_E vga_save_cursor(cursor_t* buffer);

/* Restore The cursor attributes from the buffer given as parameter.
 *
 * @param buffer The cursor buffer containing the new coordinates of the
 * cursor.
 */
OS_RETURN_E vga_restore_cursor(const cursor_t buffer);

/* Scroll in the desired direction of lines_count lines.
 *
 * @param direction The direction to whoch the console should be scrolled.
 * @param lines_count The number of lines to scroll.
 */
void vga_scroll(const SCROLL_DIRECTION_E direction, const uint8_t lines_count);

/* Set the color scheme of the screen.
 *
 * @param color_scheme The new color scheme to apply to the screen console.
 */
void vga_set_color_scheme(const colorscheme_t color_sheme);

/* Save the color scheme in the buffer given as parameter.
 *
 * @param buffer The buffer that will receive the current color scheme used by
 * the screen console.
 */
OS_RETURN_E vga_save_color_scheme(colorscheme_t* buffer);

/* Used by the kernel to display strings on the screen.
 *
 * @param str The string to display on the screen.
 * @param len The length of the string to display.
 */
void vga_console_putbytes(const char* str, const uint32_t len);

/* Used by the kernel to display strings on the screen from a keyboard input.
 *
 * @param str The string to display on the screen from a keybaord input.
 * @param len The length of the string to display.
 */
void vga_console_write_keyboard(const char* str, const uint32_t len);

#endif /* __VGA_TEXT_H_ */
