#include "../drivers/vesa.h"
#include "../drivers/rtc.h"
#include "../lib/stdint.h"
#include "../lib/stddef.h"
#include "../core/scheduler.h"

#include "statusbar.h"

static int32_t screen_width;
static int32_t screen_height;

static uint32_t save_clock_bg[64*16];
static uint32_t start_position_clock_x;
static uint32_t start_position_clock_y;

static thread_t clock_thread;

static void* draw_time(void* args)
{
    char time_str[8];
    uint32_t time;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    int32_t offset;
    uint32_t i;
    uint32_t j;

    (void)args;

    while(1)
    {
        time = get_current_daytime();
        seconds = time % 60;
        minutes = (time / 60) % 60;
        hours = time / 3600;

        time_str[0] = (hours / 10) + 48;
        time_str[1] = (hours % 10) + 48;
        time_str[2] = ':';
        time_str[3] = (minutes / 10) + 48;
        time_str[4] = (minutes % 10) + 48;
        time_str[5] = ':';
        time_str[6] = (seconds / 10) + 48;
        time_str[7] = (seconds % 10) + 48;

        /* Clean time */
        offset = 0;
        for(i = start_position_clock_y; i < start_position_clock_y + 16; ++i)
        {
            for(j = start_position_clock_x; j < start_position_clock_x + 64; ++j)
            {
                vesa_draw_pixel(j, i,
                                (uint8_t)(save_clock_bg[offset] >> 24),
                                (uint8_t)(save_clock_bg[offset] >> 16),
                                (uint8_t)(save_clock_bg[offset] >> 8),
                                (uint8_t)save_clock_bg[offset]);
                ++offset;
            }
        }

        for(i = 0; i < 8; ++i)
        {
            vesa_drawchar(time_str[i],
                         start_position_clock_x + i * 8,
                         start_position_clock_y,
                         0xFFFFFFFF, 0x00000000);
        }
        sleep(500);
    }

    return NULL;
}

OS_RETURN_E init_statusbar(void)
{
    uint32_t offset;
    uint32_t i;
    uint32_t j;
    uint8_t* addr;

    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

    vesa_draw_rectangle(5, 5,
                        screen_width - 10, 30,
                        0x30, 0x00, 0x00, 0x00);

    /* Save background of clock */
    start_position_clock_x = screen_width / 2 - 4 * 8;
    start_position_clock_y = 14;
    offset = 0;
    for(i = start_position_clock_y; i < start_position_clock_y + 16; ++i)
    {
        for(j = start_position_clock_x; j < start_position_clock_x + 64; ++j)
        {
            addr = (uint8_t*) &save_clock_bg[offset];
            vesa_get_pixel(j, i,
                           addr + 3,
                           addr + 2,
                           addr + 1,
                           addr);
            ++offset;
        }
    }

    return create_thread(&clock_thread, draw_time,
                         KERNEL_HIGHEST_PRIORITY, "UI clock", NULL);
}
