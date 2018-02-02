#include "../drivers/vesa.h"
#include "../core/scheduler.h"
#include "../memory/heap.h"
#include "../sync/lock.h"
#include "../lib/string.h"
#include "pointer.h"
#include "statusbar.h"
#include "window.h"
#include "bg.h"
#include "../fonts/fonts.h"

#include "gui.h"

static uint32_t screen_width;
static uint32_t screen_height;
static uint32_t* desktop_buffer;
static uint32_t* background;

static uint32_t screen_mem_size;

static uint8_t update_enabled = 0;
static thread_t update_thread;

static lock_t desktop_buffer_lock;

window_t windows[10];

static void* update_desktop(void* args)
{
    uint32_t i;

    (void)args;
    while(update_enabled)
    {

        spinlock_lock(&desktop_buffer_lock);
        memcpy(desktop_buffer, background, screen_mem_size);
        spinlock_unlock(&desktop_buffer_lock);

        draw_status_bar();

        for(i = 0; i < 10; ++i)
        {
            //draw_window(&windows[i]);
        }

        draw_mouse();

        spinlock_lock(&desktop_buffer_lock);
        vesa_fill_screen(desktop_buffer);
        spinlock_unlock(&desktop_buffer_lock);

        sleep(10);
    }

    return NULL;
}

OS_RETURN_E start_gui(void)
{
    OS_RETURN_E err;
    uint32_t img_width;
    uint32_t img_height;
    uint32_t bmp_index;
    uint32_t offset;
    uint32_t offset_line;
    uint32_t i;
    uint32_t j;

    err = vesa_enable_double_buffering();
    if(err != OS_NO_ERR)
    {
        kernel_error("ERROR WHILE ENABLING VESA DOUBLE BUFFER %d\n", err);
    }

    spinlock_init(&desktop_buffer_lock);

    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

    screen_mem_size = screen_width * screen_height * sizeof(uint32_t);
    background = kmalloc(screen_mem_size);
    desktop_buffer = kmalloc(screen_mem_size);

    /* Draw background */
    img_width = gimp_image.width;
    img_height = gimp_image.height;
    bmp_index = 0;
    for(i = 0; i < screen_height; ++i)
    {
        offset = (gimp_image.bytes_per_pixel * img_width * i) %
                 (img_width * img_height * gimp_image.bytes_per_pixel);
        bmp_index = 0;
        for(j = 0; j < screen_width; ++j)
        {
            offset_line = offset + bmp_index;
            uint32_t pixel = (*(uint32_t*)(gimp_image.pixel_data + offset_line));
            background[j +  i * screen_width] = (pixel & 0x00FF0000) >> 16 |
                                                (pixel & 0x0000FF00) |
                                                (pixel & 0x000000FF) << 16 |
                                                0xFF000000;
            bmp_index = (bmp_index + gimp_image.bytes_per_pixel) %
                        (gimp_image.bytes_per_pixel * img_width);
        }
    }

    /* Init statusbar */
    err = init_statusbar();
    if(err != OS_NO_ERR)
    {
        kfree(background);
        kfree(desktop_buffer);
        return err;
    }

    /* Draw some windows */
    for(uint32_t w = 0; w < 10; ++w)
    {
        init_window(&windows[w], 50 + 30 * w, 50 + 30 * w, 200, 200);
    }

    //init_window(&windows[10], -50, -50, 100, 100);
    //init_window(&windows[11], screen_width - 50, -50, 100, 100);
    //init_window(&windows[12], screen_width - 50, screen_height - 50, 100, 100);
    //init_window(&windows[13],  -50, screen_height - 50, 100, 100);
    //init_window(&windows[14], screen_width / 2 - 50, screen_height / 2 - 50, 100, 100);

    /* Init pointer */
    err = init_pointer();
    if(err != OS_NO_ERR)
    {
        kfree(background);
        kfree(desktop_buffer);
        return err;
    }

    update_enabled = 1;

    err = create_thread(&update_thread, update_desktop, KERNEL_HIGHEST_PRIORITY,
                        "UI desktop", NULL);

    if(err != OS_NO_ERR)
    {
        kfree(background);
        kfree(desktop_buffer);
    }

    return OS_NO_ERR;
}

OS_RETURN_E gui_fill_rect(const uint32_t* ptr,
                          const uint32_t x, const uint32_t y,
                          const uint32_t width, const uint32_t height,
                          const uint32_t width_gap)
{
    uint32_t* ptr_offset;
    uint32_t  pixel_offset;

    uint32_t i;
    uint32_t j;

    uint32_t pixel;
    uint32_t new_pixel;
    uint8_t  mask;

    if(x > screen_width || x + width > screen_width ||
       y + height > screen_height || y > screen_height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    spinlock_lock(&desktop_buffer_lock);

    pixel_offset = 0;

    for(i = y; i < y + height; ++i)
    {
        for(j = x; j < x + width; ++j)
        {
            ptr_offset = desktop_buffer + (i * screen_width + j);

            /* Get current dipplayed pixel and new pixel to display */
            pixel = *ptr_offset;
            new_pixel = ptr[pixel_offset];

            mask = (new_pixel & 0xFF000000) >> 24;
            if(mask == 0)
            {
                ++pixel_offset;
                continue;
            }
            if(mask != 0xFF)
            {
                /* Compute pixel with alpha channel */
                new_pixel = ((((new_pixel & 0xFF) * mask + (pixel  & 0xFF) * (255 - mask))  >> 8) & 0xFF) |
                            ((((new_pixel & 0xFF00) * mask + (pixel  & 0xFF00) * (255 - mask))  >> 8) & 0xFF00) |
                            ((((new_pixel & 0xFF0000) * mask + (pixel  & 0xFF0000) * (255 - mask)) >> 8) & 0xFF0000);
            }
            ++pixel_offset;

            /* Save pixel to buffer with aplha 0xFF */
            *ptr_offset = new_pixel | 0xFF000000;
        }
        pixel_offset += width_gap;
    }

    spinlock_unlock(&desktop_buffer_lock);

    return OS_NO_ERR;
}


OS_RETURN_E gui_drawchar(const unsigned char charracter,
                         const uint32_t x, const uint32_t y,
                         const uint32_t fgcolor, const uint32_t bgcolor)
{
    uint32_t cx;
    uint32_t cy;

    uint32_t mask[8] = {1, 2, 4, 8, 16, 32, 64, 128};

    unsigned char *glyph = __font_bitmap__ + (charracter - 31) * 16;

    uint8_t pixel[4] = {0};

    if(x > screen_width || x + 8 > screen_width ||
       y + 16 > screen_height || y > screen_height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    for(cy = 0; cy < 16; ++cy)
    {
        for(cx = 0; cx < 8; ++cx)
        {
            *((uint32_t*)pixel) = glyph[cy] & mask[cx] ? fgcolor : bgcolor;

            gui_draw_pixel(x + (7 - cx ), y + cy,
                            pixel[3], pixel[2], pixel[1], pixel[0]);
        }
    }

    return OS_NO_ERR;
}

OS_RETURN_E gui_draw_rect(const uint32_t x, const uint32_t y,
                          const uint32_t width, const uint32_t height,
                          const uint8_t alpha, const uint8_t red,
                          const uint8_t green, const uint8_t blue)
{
    uint32_t* ptr_offset;

    uint32_t i;
    uint32_t j;

    uint32_t pixel;
    uint32_t new_pixel;
    uint32_t init_pixel;

    if(x > screen_width || x + width > screen_width ||
       y + height > screen_height || y > screen_height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    init_pixel = blue | green << 8 | red << 16;

    spinlock_lock(&desktop_buffer_lock);

    for(i = y; i < y + height; ++i)
    {
        for(j = x; j < x + width; ++j)
        {
            ptr_offset = desktop_buffer + (i * screen_width + j);

            /* Get current dipplayed pixel and new pixel to display */
            pixel = *ptr_offset;
            new_pixel = init_pixel;

            if(alpha == 0)
            {
                continue;
            }
            if(alpha != 0xFF)
            {
                /* Compute pixel with alpha channel */
                new_pixel = ((((new_pixel & 0xFF) * alpha + (pixel  & 0xFF) * (255 - alpha))  >> 8) & 0xFF) |
                            ((((new_pixel & 0xFF00) * alpha + (pixel  & 0xFF00) * (255 - alpha))  >> 8) & 0xFF00) |
                            ((((new_pixel & 0xFF0000) * alpha + (pixel  & 0xFF0000) * (255 - alpha)) >> 8) & 0xFF0000);
            }

            /* Save pixel to buffer with aplha 0xFF */
            *ptr_offset = new_pixel | 0xFF000000;
        }
    }

    spinlock_unlock(&desktop_buffer_lock);

    return OS_NO_ERR;
}

OS_RETURN_E gui_draw_pixel(const uint32_t x, const uint32_t y,
                           const uint8_t alpha, const uint8_t red,
                           const uint8_t green, const uint8_t blue)
{
    uint32_t* ptr_offset;

    uint32_t pixel;
    uint32_t new_pixel;

    if(x > screen_width  || y > screen_height)
    {
        return OS_ERR_OUT_OF_BOUND;
    }

    spinlock_lock(&desktop_buffer_lock);

    ptr_offset = desktop_buffer + x + screen_width * y;

    /* Get current dipplayed pixel and new pixel to display */
    pixel = *ptr_offset;
    new_pixel = blue | green << 8 | red << 16;

    if(alpha != 0)
    {
        if(alpha != 0xFF)
        {
            /* Compute pixel with alpha channel */
            new_pixel = ((((new_pixel & 0xFF) * alpha + (pixel  & 0xFF) * (255 - alpha))  >> 8) & 0xFF) |
                        ((((new_pixel & 0xFF00) * alpha + (pixel  & 0xFF00) * (255 - alpha))  >> 8) & 0xFF00) |
                        ((((new_pixel & 0xFF0000) * alpha + (pixel  & 0xFF0000) * (255 - alpha)) >> 8) & 0xFF0000);
        }

        /* Save pixel to buffer with aplha 0xFF */
        *ptr_offset = new_pixel | 0xFF000000;
    }

    spinlock_unlock(&desktop_buffer_lock);

    return OS_NO_ERR;
}
