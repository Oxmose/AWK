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

/**********************************
 * VGA constants
 *********************************/

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

/**********************************
 * STRUCTURES
 *********************************/
typedef enum VGA_MODE_E
{
    VGA_MODE_320_200_256,
    VGA_MODE_NOT_SUPPORTED
} VGA_MODE_E;

/**********************************
 * FUNCTIONS
 *********************************/

OS_RETURN_E init_vga(void);

OS_RETURN_E set_vga_mode(uint32_t width, uint32_t height, uint32_t colordepth);

OS_RETURN_E draw_pixel(uint32_t x, uint32_t y, 
	                   uint8_t red, uint8_t green, uint8_t blue);

OS_RETURN_E draw_rectangle(uint32_t x, uint32_t y, 
                           uint32_t width, uint32_t height,
                           uint8_t red, uint8_t green, uint8_t blue);

#endif /* __VGA_GRAPHIC_H_ */