/*******************************************************************************
 *
 * File: vga_graphic.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 18/12/2017
 *
 * Version: 1.0
 *
 * VGA Graphic driver.
 ******************************************************************************/

#ifndef __VGA_GRAPHIC_H_
#define __VGA_GRAPHIC_H_

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define VGA_AC_INDEX        0x3C0
#define VGA_AC_WRITE        0x3C0
#define VGA_AC_READ         0x3C1
#define VGA_AC_RESET        0x3DA

#define VGA_MISC_WRITE      0x3C2
#define VGA_MISC_READ       0x3CC

#define VGA_SEQ_INDEX       0x3C4
#define VGA_SEQ_DATA        0x3C5

#define VGA_DAC_READ_INDEX  0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9

#define VGA_GC_INDEX        0x3CE
#define VGA_GC_DATA         0x3CF

#define VGA_CRTC_INDEX      0x3D4
#define VGA_CRTC_DATA       0x3D5

#define VGA_TEXT_FRAMEBUFFER   0xB8000
#define VGA_GRAP_FRAMEBUFFER_A 0xA0000
#define VGA_GRAP_FRAMEBUFFER_B 0xB0000

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef enum VGA_MODE_E
{
    VGA_MODE_320_200_256,
    VGA_MODE_NOT_SUPPORTED
} VGA_MODE_E;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Init VGA driver structures and hardware.
 *
 * @returns OS_NO_ERR if successfull. Otherwise an error is returned.
 */
OS_RETURN_E init_vga(void);

/* Set the new VGA mode corresponding to the settings given as parameter. If the
 * settings are not supported an error is returned.
 *
 * @param width The resolution's width in pixels.
 * @param height The resolution's height in pixels.
 * @param colordepth The colordepth to use.
 * @returns OS_NO_ERR if successfull. Otherwise an error is returned.
 */
OS_RETURN_E set_vga_mode(const uint32_t width, const uint32_t height,
                         const uint32_t colordepth);

/* Draw a pixel on the screen at the corresponding coordinates.
 * If the coordinates are out of bound an error is returned.
 * Top left coordinates are 0, 0.
 *
 * @param x The x coordinate.
 * @param y The y cooridnate.
 * @param red The red component value.
 * @param green The green component value.
 * @param blue The blue component value.
 * @returns OS_NO_ERR if successfull. Otherwise an error is returned.
 */
OS_RETURN_E draw_pixel(const uint32_t x, const uint32_t y,
                       const uint8_t red, const uint8_t green,
                       const uint8_t blue);

/* Draw a rectangle on the screen at the corresponding coordinates.
 * If the coordinates are out of bound or the size is too big for the screen and
 * the mode, an error is returned.
 * Top left coordinates are 0, 0.
 *
 * @param x The x coordinate.
 * @param y The y cooridnate.
 * @param width The width of the rectangle.
 * @param height The height of the rectangle.
 * @param red The red component value.
 * @param green The green component value.
 * @param blue The blue component value.
 * @returns OS_NO_ERR if successfull. Otherwise an error is returned.
 */
OS_RETURN_E draw_rectangle(const uint32_t x, const uint32_t y,
                           const uint32_t width, const uint32_t height,
                           const uint8_t red, const uint8_t green,
                           const uint8_t blue);

#endif /* __VGA_GRAPHIC_H_ */