#include "../drivers/vesa.h"
#include "../core/scheduler.h"
#include "pointer.h"
#include "statusbar.h"
#include "window.h"
#include "bg.h"

#include "gui.h"

static uint32_t screen_width;
static uint32_t screen_height;

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

    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

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
            vesa_draw_pixel(j, i,
                            gimp_image.pixel_data[offset_line + 3],
                            gimp_image.pixel_data[offset_line],
                            gimp_image.pixel_data[offset_line + 1],
                            gimp_image.pixel_data[offset_line + 2]);

            bmp_index = (bmp_index + gimp_image.bytes_per_pixel) %
                        (gimp_image.bytes_per_pixel * img_width);
        }
    }

    /* Init pointer */
    err = init_pointer();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Init statusbar */
    err = init_statusbar();
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Draw some windows */
    window_t windows[5];
    init_window(&windows[0], 60, 60, 400, 600);
    init_window(&windows[1], 1200, 300, 400, 600);
    init_window(&windows[2], 10, 500, 80, 1500);
    init_window(&windows[3], 400, 400, 200, 200);
    init_window(&windows[4], 1355, 700, 200, 200);


    for(i = 0; i < 5; ++i)
    {
        draw_window(&windows[i]);
    }

    return OS_NO_ERR;
}
