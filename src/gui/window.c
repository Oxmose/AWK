#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../drivers/vesa.h"

#include "window.h"

/*******************************************************************************
* FUNCTIONS
******************************************************************************/
OS_RETURN_E init_window(window_t *window,
                        const uint32_t x, const uint32_t y,
                        const uint32_t width, const uint32_t height)
{
    if(window == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    window->pos_x = x;
    window->pos_y = y;
    window->width = width;
    window->height = height;

    return OS_NO_ERR;
}

OS_RETURN_E draw_window(const window_t *window)
{
    uint32_t real_width;
    uint32_t real_height;
    uint32_t screen_width = vesa_get_screen_width();
    uint32_t screen_height = vesa_get_screen_height();
    uint32_t bottom;
    uint32_t right;
    uint32_t i;

    /* Draw top border */
    real_width = 0;
    for(i = window->pos_x; i < screen_width && i < window->pos_x + window->width; ++i)
    {
        vesa_draw_pixel(i, window->pos_y,
                        0xFF, 0x95, 0xA5, 0xA6);
        vesa_draw_pixel(i, window->pos_y + 1,
                        0xFF, 0x95, 0xA5, 0xA6);
        ++real_width;
    }

    /* Draw bottom border */
    bottom = window->pos_y + window->height;
    if(bottom < screen_height)
    {
        for(i = window->pos_x; i < window->pos_x + real_width; ++i)
        {
            vesa_draw_pixel(i, bottom,
                            0xFF, 0x95, 0xA5, 0xA6);
            vesa_draw_pixel(i, bottom - 1,
                            0xFF, 0x95, 0xA5, 0xA6);
        }
    }

    /* Draw side border */
    right = window->pos_x + window->width;
    real_height = 0;
    for(i = window->pos_y; i < screen_height && i <= window->pos_y + window->height; ++i)
    {
        vesa_draw_pixel(window->pos_x, i,
                        0xFF, 0x95, 0xA5, 0xA6);
        vesa_draw_pixel(window->pos_x + 1, i,
                        0xFF, 0x95, 0xA5, 0xA6);
        vesa_draw_pixel(right, i,
                        0xFF, 0x95, 0xA5, 0xA6);
        vesa_draw_pixel(right - 1, i,
                        0xFF, 0x95, 0xA5, 0xA6);
        ++real_height;
    }

    /* Draw background */
    vesa_draw_rectangle(window->pos_x + 2, window->pos_y + 27,
                        real_width - 2, real_height - 29,
                        0xCC, 0xEC, 0xF0, 0xF1);

    /* Draw statusbar */
    if(real_width != window->width)
    {
        vesa_draw_rectangle(window->pos_x + 2, window->pos_y + 2,
                            real_width - 2, 25,
                            0xFF, 0x34, 0x49, 0x5E);
    }
    else
    {
        vesa_draw_rectangle(window->pos_x + 2, window->pos_y + 2,
                            real_width - 3, 25,
                            0xFF, 0x34, 0x49, 0x5E);
    }

    /* QUIT button */
    vesa_draw_rectangle(window->pos_x + 4, window->pos_y + 4,
                        21, 21,
                        0xFF, 0xE7, 0x4C, 0x3C);
                        
    /* MIN button */
    vesa_draw_rectangle(window->pos_x + 29, window->pos_y + 4,
                        21, 21,
                        0xFF, 0x27, 0xAE, 0x60);
    /* MAX button */
    vesa_draw_rectangle(window->pos_x + 54, window->pos_y + 4,
                        21, 21,
                        0xFF, 0x29, 0x80, 0xB9);

    return OS_NO_ERR;
}
