/*******************************************************************************
 *
 * File: vga_graphic.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 18/12/2017
 *
 * Version: 1.0
 *
 * VGA Graphic driver.
 ******************************************************************************/          

#include "../cpu/cpu.h"    /* outb, inb */
#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/string.h" /* memset */

/* Header file */            
#include "vga_graphic.h"

/* VGA current settigns */
static uint32_t screen_width;
static uint32_t screen_height;
static uint8_t  screen_depth;

/* VGA framebuffer */
static uint8_t *frame_buffer;

static VGA_MODE_E get_supported_mode(uint32_t width, uint32_t height, 
                                uint32_t colordepth)
{
    if(width == 320 && height == 200 && colordepth == 8)
    {
        return VGA_MODE_320_200_256;
    }
    else
    {
        return VGA_MODE_NOT_SUPPORTED;
    }
}

static void write_registers(uint8_t *registers)
{
    uint8_t i;
    uint8_t read;

    /* Write MISC */
    outb(*(registers++), VGA_MISC_WRITE);
    
    /* Write SEAUENCER */
    for(i = 0; i < 5; ++i)
    {
        outb(i, VGA_SEQ_INDEX);
        outb(*(registers++), VGA_SEQ_DATA);
    }
    
    /* Set CRT controller */
    outb(0x03, VGA_CRTC_INDEX);
    read = inb(VGA_CRTC_DATA) | 0x80;
    outb(read, VGA_CRTC_DATA);

    outb(0x11, VGA_CRTC_INDEX);
    read = inb(VGA_CRTC_DATA) & ~0x80;
    outb(read, VGA_CRTC_DATA);
    
    registers[0x03] |= 0x80;
    registers[0x11] &= ~0x80;
    
    for(i = 0; i < 25; ++i)
    {
        outb(i, VGA_CRTC_INDEX);
        outb(*(registers++), VGA_CRTC_DATA);
    }
    
    /* Set GC controller */
    for(i = 0; i < 9; ++i)
    {
        outb(i, VGA_GC_INDEX);
        outb(*(registers++), VGA_GC_DATA);
    }
    
    /* Set AC controller */
    for(i = 0; i < 21; ++i)
    {
        inb(VGA_AC_RESET);

        outb(i, VGA_AC_INDEX);
        outb(*(registers++), VGA_AC_WRITE);
    }
    inb(VGA_AC_RESET);
    outb(0x20, VGA_AC_INDEX);    
}

static uint8_t* get_frame_buffer(void)
{
    outb(0x06, VGA_GC_INDEX);
    uint8_t segmentNumber = inb(VGA_GC_DATA) & 0x0C;
    
    switch(segmentNumber)
    {
        case 0x00: 
            return (uint8_t*)NULL;
        case 0x04: 
            return (uint8_t*)VGA_GRAP_FRAMEBUFFER_A;
        case 0x08: 
            return (uint8_t*)VGA_GRAP_FRAMEBUFFER_B;
        case 0x0C: 
            return (uint8_t*)VGA_TEXT_FRAMEBUFFER;
    }

    return NULL;
}

static uint8_t get_color_index(uint8_t r, uint8_t g, uint8_t b)
{
    /* R3G3B2 */
    return (((r & 0x7) << 5) | ((g & 0x7) << 2) | (b & 0x3));
}

OS_RETURN_E init_vga(void)
{
    OS_RETURN_E err;
    err = set_vga_mode(320, 200, 8);
    return err;
}

OS_RETURN_E set_vga_mode(uint32_t width, uint32_t height, uint32_t colordepth)
{
    VGA_MODE_E vga_mode = get_supported_mode(width, height, colordepth);
    if(vga_mode == VGA_MODE_NOT_SUPPORTED)
    {
        return OS_ERR_GRAPHIC_MODE_NOT_SUPPORTED;
    }

    screen_width  = width;
    screen_height = height;
    screen_depth  = colordepth;
    
    /* Keep these array in the function to avoid taking useless memory space */
    unsigned char g_320x200x256[] =
    {
        /* MISC */
        0x63,
        /* SEQ */
        0x03, 0x01, 0x0F, 0x00, 0x0E,
        /* CRTC */
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3,
        0xFF,
        /* GC */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F,
        0xFF,
        /* AC */
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x41, 0x00, 0x0F, 0x00, 0x00
    };
    
    /* Write registers */
    if(vga_mode == VGA_MODE_320_200_256)
    {
        write_registers(g_320x200x256);

        /* Set 256 color palette */
        outb(0, VGA_DAC_WRITE_INDEX);
        for(uint32_t r = 0; r < 8; ++r)
        {
            for(uint32_t g = 0; g < 8; ++g)
            {
                for(uint32_t b = 0; b < 4; ++b)
                {

                    outb(r * 9, VGA_DAC_DATA);
                    outb(g * 9, VGA_DAC_DATA);
                    outb(b * 21, VGA_DAC_DATA);
                }
            }
        }
    }

    /* Get the new frame buffer */
    frame_buffer = get_frame_buffer();
    return OS_NO_ERR;
}

OS_RETURN_E draw_pixel(uint32_t x, uint32_t y, 
                       uint8_t red, uint8_t green, uint8_t blue)
{
    uint32_t offset;
    uint8_t color;

    /* Test bounds */
    if(x > screen_width || y > screen_height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    /* Compute pixel address in the buffer */
    offset = y * screen_width + x;

    /* Get pixel color */
    color = get_color_index(red, green, blue);

    /* Set memory */
    *(frame_buffer + offset) = color;

    return OS_NO_ERR;
}

OS_RETURN_E draw_rectangle(uint32_t x, uint32_t y, 
                           uint32_t width, uint32_t height,
                           uint8_t red, uint8_t green, uint8_t blue)
{
    /* Test bounds */
    if(x + width > screen_width || y + height> screen_height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }
    uint8_t color = get_color_index(red, green, blue);

    /* Set memory */
    for(uint32_t i = 0; i < height; ++i)
    {
        /* Compute pixel address in the buffer */
        uint32_t offset = (y + i) * screen_width + x;
        memset(frame_buffer + offset, color, width);
    }

    return OS_NO_ERR;
}