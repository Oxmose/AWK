/*******************************************************************************
 *
 * File: vesa.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/01/2018
 *
 * Version: 1.0
 *
 * VESA VBE 2 graphic drivers
 ******************************************************************************/

#ifndef __VESA_H_
#define __VESA_H_

#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */
#include "graphic.h"            /* structures */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define BIOS_INTERRUPT_VESA 0x10

#define BIOS_CALL_GET_VESA_INFO 0x4F00
#define BIOS_CALL_GET_VESA_MODE 0x4F01
#define BIOS_CALL_SET_VESA_MODE 0x4F02

#define VESA_FLAG_LINEAR_FB  0x90
#define VESA_FLAG_LFB_ENABLE 0x4000

#define MAX_VESA_MODE_COUNT 245
/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct vbe_info_structure
{
    char     signature[4];
    uint16_t version;
    uint32_t oem;
    uint32_t capabilities;
    uint32_t video_modes;
    uint16_t video_memory;
    uint16_t software_rev;
    uint32_t vendor;
    uint32_t product_name;
    uint32_t product_rev;
    uint8_t  reserved[222];
    uint8_t  oem_data[256];
} __attribute__ ((packed)) vbe_info_structure_t;

typedef struct vbe_mode_info_structure
{
    uint16_t attributes;
    uint8_t  window_a;
    uint8_t  window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t  w_char;
    uint8_t  y_char;
    uint8_t  planes;
    uint8_t  bpp;
    uint8_t  banks;
    uint8_t  memory_model;
    uint8_t  bank_size;
    uint8_t  image_pages;
    uint8_t  reserved0;

    uint8_t  red_mask;
    uint8_t  red_position;
    uint8_t  green_mask;
    uint8_t  green_position;
    uint8_t  blue_mask;
    uint8_t  blue_position;
    uint8_t  reserved_mask;
    uint8_t  reserved_position;
    uint8_t  direct_color_attributes;

    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t  reserved1[206];
} __attribute__ ((packed)) vbe_mode_info_structure_t;

typedef struct vesa_mode
{
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
    uint16_t mode_id;

    uint32_t framebuffer;

    struct vesa_mode* next;
} __attribute__ ((packed)) vesa_mode_t;

typedef struct vesa_mode_info
{
    uint16_t width;
    uint16_t height;
    uint16_t bpp;
    uint16_t mode_id;
} vesa_mode_info_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init the VESA structures, gather available modes
 *
 * @return The state or error code.
 */
OS_RETURN_E init_vesa(void);

/* Switch for VGA text mode to VESA mode. The function will copy the content of
 * the VGA framebuffer to write it on the screen after the video mode switch.
 *
 * @return The state or error code.
 */
OS_RETURN_E text_vga_to_vesa(void);

/* Get the number of VESA modes detected in the system.
 *
 * @returns The  number of VESA modes detected in the system.
 */
uint16_t get_vesa_mode_count(void);

/* Fill a biffer given as parameter with the list of all VESA modes detected in
 * the system. If the buffer is too huge, the function just fill the need space.
 * if the buffer is too small, the function will stop filling the buffer when
 * the parameter size is reached.
 *
 * @param buffer The buffer that needs to be filled with the data.
 * @param size The size of the buffer in number of elements.
 * @return The state or error code.
 */
OS_RETURN_E get_vesa_modes(vesa_mode_info_t* buffer, const uint32_t size);

/* Set the VESA mode given as parameter. If the VESA mode is not correct or
 * does not correspond to a mode detected in the system, the function will
 * return an error. To get the list of all available modes please check
 * the get_vesa_modes function.
 *
 * @param mode The mode to be set.
 * @return The state or error code.
 */
OS_RETURN_E set_vesa_mode(const vesa_mode_info_t mode);

/* Draw a pixel on the screen at the given coordinates. The top left hand corner
 * of the screen has coordinates x = 0 and y = 0. The color of the pixel should
 * be expressed in the bpp format of the current set mode.
 *
 * @param x The x coordinate of the pixel.
 * @param y The y coordinate of the pixel.
 * @param red The red component of the pixel.
 * @param green The green component of the pixel.
 * @param blue The blue component of the pixel.
 * @return The state or error code.
 */
OS_RETURN_E vesa_draw_pixel(const uint16_t x, const uint16_t y,
                            const uint8_t red, const uint8_t green,
                            const uint8_t blue);

/* Draw a character on the screen at the given coordinates. The top left hand
 * corner of the screen has coordinates x = 0 and y = 0. The coordinates reffer
 * to the top left hand corner of the character.
 *
 * @param x The x coordinate of the character.
 * @param y The y coordinate of the character.
 * @param fgcolor The foreground color of the character.
 * @param bgcolor The background color of the character.
 * @return The state or error code.
 */
void vesa_drawchar(const unsigned char character,
                   const uint32_t x, const uint32_t y,
                   const uint32_t fgcolor, const uint32_t bgcolor);

/* Clear the screen */
void vesa_clear_screen(void);

/* Place the cursor to the selected coordinates given as parameters.
 *
 * @param line The line index where to place the cursor.
 * @param column The column index where to place the cursor.
 * @returns The error or success state.
 */
OS_RETURN_E vesa_put_cursor_at(const uint32_t line, const uint32_t column);

/* Save the cursor attributes in the buffer given as paramter
 *
 * @param buffer The cursor buffer in which the current cursor possition is
 * going to be saved.
 */
OS_RETURN_E vesa_save_cursor(cursor_t* buffer);

/* Restore The cursor attributes from the buffer given as parameter.
 *
 * @param buffer The cursor buffer containing the new coordinates of the
 * cursor.
 */
OS_RETURN_E vesa_restore_cursor(const cursor_t buffer);

/* Scroll in the desired direction of lines_count lines.
 *
 * @param direction The direction to whoch the console should be scrolled.
 * @param lines_count The number of lines to scroll.
 */
void vesa_scroll(const SCROLL_DIRECTION_E direction,
                 const uint32_t lines_count);

/* Set the color scheme of the screen.
 *
 * @param color_scheme The new color scheme to apply to the screen console.
 */
void vesa_set_color_scheme(const colorscheme_t color_scheme);

/* Save the color scheme in the buffer given as parameter.
 *
 * @param buffer The buffer that will receive the current color scheme used by
 * the screen console.
 */
OS_RETURN_E vesa_save_color_scheme(colorscheme_t* buffer);

/* Used by the kernel to display strings on the screen.
 *
 * @param str The string to display on the screen.
 */
void vesa_put_string(const char* str);

/* Used by the kernel to display character on the screen.
 *
 * @param character The character to display on the screen.
 */
void vesa_put_char(const char character);

/* Used by the kernel to display strings on the screen from a keyboard input.
 *
 * @param str The string to display on the screen from a keybaord input.
 * @param len The length of the string to display.
 */
void vesa_console_write_keyboard(const char* str, const uint32_t len);

#endif /* __VESA_H_ */
