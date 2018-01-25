#include "../drivers/rtc.h"
#include "../drivers/vesa.h"
#include "../lib/stdint.h"
#include "../lib/stddef.h"

#include "gui.h"

#include "statusbar.h"

static int32_t screen_width;
static int32_t screen_height;

static uint32_t start_position_clock_x;
static uint32_t start_position_clock_y;

OS_RETURN_E init_statusbar(void)
{
    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

    start_position_clock_x = screen_width / 2 - 24;
    start_position_clock_y = 12;

    gui_draw_rect(5, 5,
                  screen_width - 10, 30,
                  0x30, 0x00, 0x00, 0x00);

    return OS_NO_ERR;
}

void draw_status_bar(void)
{
    char time_str[8];
    uint32_t time;
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint32_t i;

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

    gui_draw_rect(5, 5,
                  screen_width - 10, 30,
                  0x30, 0x00, 0x00, 0x00);

    for(i = 0; i < 8; ++i)
    {
        gui_drawchar(time_str[i],
                     start_position_clock_x + i * 8,
                     start_position_clock_y,
                     0xFFFFFFFF, 0x00000000);
    }
}
