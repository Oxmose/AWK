#include "../drivers/vesa.h"
#include "../drivers/mouse.h"
#include "../lib/stdint.h"
#include "../lib/stddef.h"

#include "gui.h"

#include "cursor.h"
#include "pointer.h"

#define CURSOR_WIDTH 12
#define CURSOR_HEIGHT 15
#define CURSOR_BPP 4

static mouse_state_t current_state;
static volatile uint32_t last_poll = 0;

static uint32_t screen_width;
static uint32_t screen_height;

static uint32_t cursor_bmp[CURSOR_WIDTH * CURSOR_HEIGHT];
static uint32_t cursor_bmp_neg[CURSOR_WIDTH * CURSOR_HEIGHT];

static void mouse_event(void)
{
    mouse_state_t new_state;
    uint32_t width;
    uint32_t height;

    get_mouse_state(&new_state);

    /* Manage mouse sensitivity */
    if(current_state.flags == new_state.flags && last_poll++ < 2)
    {
        return;
    }
    last_poll = 0;

    /* Restore what was under the cursor */
    height = CURSOR_HEIGHT;
    width = CURSOR_WIDTH;
    if(current_state.pos_y + height >= screen_height)
    {
        height = screen_height - current_state.pos_y;
    }
    if(current_state.pos_x + width >= screen_width)
    {
        width = screen_width - current_state.pos_x;
    }

    /* Compute new position */
    current_state.pos_x += new_state.pos_x * 2;
    current_state.pos_y -= new_state.pos_y * 2;
    current_state.flags = new_state.flags;

    if(current_state.pos_y < 0)
    {
        current_state.pos_y = 0;
    }
    else if((uint32_t)current_state.pos_y >= screen_height)
    {
        current_state.pos_y = screen_height - 1;
    }
    if(current_state.pos_x < 0)
    {
        current_state.pos_x = 0;
    }
    else if((uint32_t)current_state.pos_x >= screen_width)
    {
        current_state.pos_x = screen_width - 1;
    }

    update_mouse(current_state.pos_x, current_state.pos_y);
}

void update_mouse(const uint32_t x, const uint32_t y)
{
    uint32_t width;
    uint32_t height;

    height = CURSOR_HEIGHT;
    width = CURSOR_WIDTH;
    if(y + height > screen_height)
    {
        height = screen_height - y;
    }
    if(x + width > screen_width)
    {
        width = screen_width - x;
    }

    /* Draw mouse cursor */
    gui_fill_rect((current_state.flags & MOUSE_LEFT_CLICK) ?
                  cursor_bmp_neg : cursor_bmp,
                  x, y,
                  width, height,
                  CURSOR_WIDTH - width);
}

OS_RETURN_E init_pointer(void)
{
    OS_RETURN_E err;
    uint32_t i;
    uint32_t j;
    uint32_t height;
    uint32_t width;

    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

    current_state.pos_x = screen_width / 2 - CURSOR_WIDTH / 2;
    current_state.pos_y = screen_height / 2 - CURSOR_HEIGHT / 2;
    current_state.flags = 0;

    /* Init negative mouse pointer */
    height = CURSOR_HEIGHT;
    width = CURSOR_WIDTH;
    for(i = 0; i < height; ++i)
    {
        for(j = 0; j < width; ++j)
        {
            uint32_t pixel = (*((uint32_t*)(cursor_bmp_export.pixel_data) + j + i * width));

            uint8_t r = (pixel & 0x000000FF);
            uint8_t g = (pixel & 0x0000FF00) >> 8;
            uint8_t b = (pixel & 0x00FF0000) >> 16;

            cursor_bmp[j + i * width] = b |
                                        (g << 8) |
                                        (r << 16) |
                                        (pixel & 0xFF000000);
            cursor_bmp_neg[j + i * width] = ((~cursor_bmp[j + i * width]) & 0x00FFFFFF) |
                                            (pixel & 0xFF000000);
        }
    }

    draw_mouse();

    err = register_mouse_event(&mouse_event, NULL);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    return OS_NO_ERR;
}

void draw_mouse(void)
{
    update_mouse(current_state.pos_x, current_state.pos_y);
}
