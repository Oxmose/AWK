#include "../drivers/vesa.h"
#include "../lib/stdio.h"
#include "../drivers/mouse.h"
#include "../core/scheduler.h"
#include "../sync/semaphore.h"

#define TEST_PALETTE 1
#define TEST_LIGNES  0

semaphore_t sem_wait;
mouse_state_t current_state;

int8_t color = 0x3;

void test_mouse_event(void)
{
    mouse_state_t new_state;
    get_mouse_state(&new_state);

    current_state.pos_x += new_state.pos_x;
    current_state.pos_y -= new_state.pos_y;
    current_state.flags = new_state.flags;

    if(current_state.pos_y < 0)
        current_state.pos_y = 0;
    if(current_state.pos_x < 0)
        current_state.pos_x = 0;
    if(current_state.pos_y >= 200)
        current_state.pos_y = 199;
    if(current_state.pos_x >= 320)
        current_state.pos_x = 319;

    sem_post(&sem_wait);
}

void draw_mouse(void)
{
    #if 0
    if(current_state.flags & MOUSE_LEFT_CLICK)
        draw_rectangle(0, 0, 320, 200, 0xFF, 0xFF, 0xFF);

    if(current_state.flags & MOUSE_RIGHT_CLICK)
    {
        if(color == 0x3)
        {
            color = 0x1C;
        }
        else if(color == 0x1C)
        {
            color = 0xE0;
        }
        else
        {
            color = 0x3;
        }
    }

    draw_rectangle(current_state.pos_x - 4, current_state.pos_y - 1,
                   9, 3, (color >> 5) & 0x7, (color >> 2) & 0x7, color & 0x3);
    draw_rectangle(current_state.pos_x - 1, current_state.pos_y - 4,
                   3, 9, (color >> 5) & 0x7, (color >> 2) & 0x7, color & 0x3);
                   #endif
}

void *test_mouse(void*args)
{
    (void)args;

    sem_init(&sem_wait, 0);
    current_state.pos_x = 160;
    current_state.pos_y = 100;

    register_mouse_event(&test_mouse_event, NULL);
    while(1)
    {
        /* Avoid CPU consumption polling */
        sem_pend(&sem_wait);

        draw_mouse();
    }

    return NULL;
}