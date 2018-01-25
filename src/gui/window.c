#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/string.h"
#include "../memory/heap.h"
#include "../drivers/vesa.h"
#include "gui.h"

#include "../debug.h"

#include "window.h"


static uint32_t screen_width;
static uint32_t screen_height;

/*******************************************************************************
* FUNCTIONS
******************************************************************************/
static void setup_window(window_t *window)
{
    uint32_t i;
    uint32_t j;
    uint32_t offset_0;
    uint32_t offset_1;
    uint32_t offset_2;
    uint32_t* canvas;

    canvas = window->canvas;

    /* Draw top and bottom borders */
    offset_0 = window->width;
    offset_1 = window->width * (window->height - 1);
    offset_2 = window->width * (window->height - 2);
    for(i = 0; i < window->width; ++i)
    {
        canvas[i] = 0xFF111111;
        canvas[offset_0 + i] = 0xFF111111;
        canvas[offset_1 + i] = 0xFF111111;
        canvas[offset_2 + i] = 0xFF111111;
    }

    /* Draw left and right borders */
    for(i = 2; i < window->height - 2; ++i)
    {
        canvas[i * window->width] = 0xFF111111;
        canvas[i * window->width + 1] = 0xFF111111;
        canvas[i * window->width + window->width - 1] = 0xFF111111;
        canvas[i * window->width + window->width - 2] = 0xFF111111;
    }

    /* Draw title bar */
    for(i = 2; i < 27; ++i)
    {
        /* REST */
        for(j = 2; j < window->width - 2; ++j)
        {
            canvas[i * window->width + j] = 0xFF2c3e50;
        }
    }

    for(i = 7; i < 22; ++i)
    {
        /* QUIT */
        for(j = 7; j < 22; ++j)
        {
            canvas[i * window->width + j] = 0xFFc0392b;
        }

        /* MIN */
        for(j = 27; j < 42; ++j)
        {
            canvas[i * window->width + j] = 0xFF27ae60;
        }

        /* MAX */
        for(j = 47; j < 62; ++j)
        {
            canvas[i * window->width + j] = 0xFF2980b9;
        }
    }
}

OS_RETURN_E init_window(window_t *window,
                        const int32_t x, const int32_t y,
                        const uint32_t width, const uint32_t height)
{
    if(window == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(width < 50 || height < 50)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

    window->pos_x = x;
    window->pos_y = y;
    window->width = width;
    window->height = height;

    /* Init window canvas */
    window->canvas = kmalloc(width * height * sizeof(uint32_t));
    if(window->canvas == NULL)
    {
        return OS_ERR_MALLOC;
    }
    memset(window->canvas, 0xAA, width * height * sizeof(uint32_t));

    setup_window(window);

    return OS_NO_ERR;
}

OS_RETURN_E draw_window(const window_t *window)
{
    /* Screen position */
    uint32_t real_x;
    uint32_t real_y;

    /* Canvas position */
    int32_t adjusted_x;
    int32_t adjusted_y;
    int32_t adjusted_width;
    int32_t adjusted_height;

    uint32_t* canvas;

    /* Other data */
    int32_t i;
    int32_t j;
    uint32_t offset;
    uint32_t offset_y;

    if(window == NULL || window->canvas == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check for left screen borders */
    if(window->pos_x < 0)
    {
        real_x = 0;
        adjusted_x = -window->pos_x;
        adjusted_width = window->width + window->pos_x;
    }
    else
    {
        real_x = window->pos_x;
        adjusted_x = 0;
        adjusted_width = window->width;
    }
    if(adjusted_width <= 0)
    {
        return OS_NO_ERR;
    }

    /* Check for right screen borders */
    if(real_x >= screen_width)
    {
        return OS_NO_ERR;
    }
    if(real_x + adjusted_width >= screen_width)
    {
        adjusted_width = screen_width - real_x;
    }

    /* Check for top screen borders */
    if(window->pos_y < 0)
    {
        real_y = 0;
        adjusted_y = -window->pos_y;
        adjusted_height = window->height + window->pos_y;
    }
    else
    {
        real_y = window->pos_y;
        adjusted_y = 0;
        adjusted_height = window->height;
    }
    if(adjusted_height <= 0)
    {
        return OS_NO_ERR;
    }

    /* Check for bottom screen borders */
    if(real_y >= screen_height)
    {
        return OS_NO_ERR;
    }
    if(real_y + adjusted_height >= screen_height)
    {
        adjusted_height = screen_height - real_y;
    }

    /* Draw canvas on screen */
    canvas = window->canvas;
    offset_y = adjusted_y;
    for(i = real_y; i < (int32_t)real_y + adjusted_height; ++i)
    {
        offset = offset_y * window->height + adjusted_x;
        for(j = real_x; j < (int32_t)real_x + adjusted_width; ++j)
        {
            uint32_t pixel = canvas[offset];
            gui_draw_pixel(j, i,
                            (uint8_t)(pixel >> 24),
                            (uint8_t)(pixel >> 16),
                            (uint8_t)(pixel >> 8),
                            (uint8_t)(pixel));
            ++offset;
        }
        ++offset_y;
    }

    return OS_NO_ERR;
}
