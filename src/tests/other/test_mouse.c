#include "../../drivers/vesa.h"
#include "../../drivers/rtc.h"
#include "../../lib/stdio.h"
#include "../../drivers/mouse.h"
#include "../../core/scheduler.h"
#include "../../sync/semaphore.h"

#define TEST_PALETTE 1
#define TEST_LIGNES  0

mouse_state_t current_state;

int32_t screen_width;
int32_t screen_height;

uint8_t blink = 0;
uint32_t i;
uint8_t last_seconds = 0;


void mouse_event(void)
{
    mouse_state_t new_state;
    get_mouse_state(&new_state);

    current_state.pos_x += new_state.pos_x;
    current_state.pos_y -= new_state.pos_y;
    current_state.flags = new_state.flags;

    if(current_state.pos_y < 40)
        current_state.pos_y = 40;
    if(current_state.pos_x < 0)
        current_state.pos_x = 0;
    if(current_state.pos_y >= screen_height)
        current_state.pos_y = screen_height - 1;
    if(current_state.pos_x >= screen_width)
        current_state.pos_x = screen_width - 1;

    clear_screen();

    vesa_draw_rectangle(current_state.pos_x - 9, current_state.pos_y - 1,
                            19, 3,
                            0xbd, 0xc3, 0xc7);
    vesa_draw_rectangle(current_state.pos_x - 1, current_state.pos_y - 9,
                        3, 19,
                        0xbd, 0xc3, 0xc7);
}

void start_gui(void)
{
    current_state.pos_x = 160;
    current_state.pos_y = 100;

    screen_width = vesa_get_screen_width();
    screen_height = vesa_get_screen_height();

    register_mouse_event(&mouse_event, NULL);

    vesa_draw_rectangle(0, 40,
                            screen_width, screen_height - 40,
                            0x34, 0x98, 0xdb);

    vesa_draw_rectangle(current_state.pos_x - 4, current_state.pos_y - 1,
                            9, 3,
                            0xbd, 0xc3, 0xc7);
    vesa_draw_rectangle(current_state.pos_x - 1, current_state.pos_y - 4,
                        3, 9,
                        0xbd, 0xc3, 0xc7);

    while(1)
    {
        vesa_draw_rectangle(0, 0,
                        screen_width, 40,
                        0x2c, 0x3e, 0x50);


        uint32_t time = get_current_daytime();
        uint32_t seconds = time % 60;
        uint32_t minutes = (time / 60) % 60;
        uint32_t hours = time / 3600;

        char time_str[8];

        if(seconds != last_seconds)
        {
            blink = (blink + 1) % 2;
        }
        last_seconds = seconds;

        time_str[0] = (hours / 10) + 48;
        time_str[1] = (hours % 10) + 48;
        time_str[3] = (minutes / 10) + 48;
        time_str[4] = (minutes % 10) + 48;
        time_str[6] = (seconds / 10) + 48;
        time_str[7] = (seconds % 10) + 48;

        if(blink)
        {
            time_str[2] = ':';
            time_str[5] = ':';
        }
        else
        {
            time_str[2] = ' ';
            time_str[5] = ' ';
        }

        for(i = 0; i < 8; ++i)
        {
            vesa_drawchar(time_str[i], screen_width - (10 - i) * 8, 13,  0xc0392b, 0x2c3e50);
        }

        sleep(50);
    }
}

void *test_mouse2(void*args)
{
    (void)args;

    start_gui();

    return NULL;
}
