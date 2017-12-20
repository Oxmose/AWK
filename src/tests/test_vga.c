#include "../drivers/vga_graphic.h"
#include "../core/scheduler.h"

typedef struct test_str
{
    int j;
    int jend;
    int r[5];
    int g[5];
    int b[5];
} test_str;

void *col_thread(void* args)
{
    test_str *str = (test_str*)args;
    int m = 0;
    while(1)
    {
        for(int j = str->j; j < str->jend; ++j)
        for(int i = 0; i < 320; ++i)
        {
            
            {
                draw_pixel(i, j, str->r[m], str->g[m], str->b[m]);
            }
            sleep(8);
        }
        
        m = (m+1) % 5;
    }
        return NULL;
}

void *pal(void *args)
{
    int er_l = 0;
    if((int)args == 1)
    {
        int x = 0;
        int y = 0;
        int y_off = 0;
        while(1)
        {
            
            /* draw palette */
            for(uint32_t g = 0; g < 8; ++g)
            {
                y = y_off + g * 10;
                x = 0;
                for(uint32_t r = 0; r < 8; ++r)
                {
                    for(uint32_t b = 0; b < 4; ++b)
                    {
                        draw_rectangle(x, y, 10, 10, r, g, b);
                        x += 10;
                        sleep(10);
                    }
                }
            }
        
            y_off  = (y_off + 80) % 160;

            if(er_l == 0)
            {
                er_l = 1;
                thread_t palette;
                create_thread(&palette, pal, 60, "test\0", (void*)0);
            }
        }
    }
    else
    {
        int x = 0;
        int y = 0;
        int y_off = 0;
        while(1)
        {
            
            /* draw palette */
            for(uint32_t g = 0; g < 8; ++g)
            {
                y = y_off + g * 10;
                x = 0;
                for(uint32_t r = 0; r < 8; ++r)
                {
                    for(uint32_t b = 0; b < 4; ++b)
                    {                        
                        draw_rectangle(x, y, 10, 10, 0, 0, 0);
                        x += 10;
                        sleep(10);
                    }
                }
            }
        
            y_off  = (y_off + 80) % 160;
        }
    }
}

void *vga_test(void*args)
{
    (void)args;
    init_vga();

    for(int j = 0; j < 200; ++j)
    {
        for(int i = 0; i < 320; ++i)
        {
            draw_pixel(i, j, 0xFF, 0xFF, 0xFF);
        }
        
    }
#if 1
    thread_t palette;
    create_thread(&palette, pal, 60, "test\0", (void*)1);

    wait_thread(palette, NULL);
#else
    test_str red   = {.j = 0, .jend=40,    .r={0x7, 0xFF, 0, 0, 0}, .g={0, 0xFF, 0, 0, 0x7}, .b={0, 0xFF, 0, 0x7, 0}};

    test_str green = {.j = 40, .jend=80,   .r={0, 0x7, 0xFF, 0, 0}, .g={0x7, 0, 0xFF, 0, 0}, .b={0, 0, 0xFF, 0, 0x7}};

    test_str blue  = {.j = 80, .jend=120,  .r={0, 0, 0x7, 0xFF, 0}, .g={0, 0x7, 0, 0xFF, 0}, .b={0x7, 0 ,0, 0xFF, 0}};

    test_str black = {.j = 120, .jend=160, .r={0, 0, 0, 0x7, 0xFF}, .g={0, 0, 0x7, 0, 0xFF}, .b={0, 0x7 ,0, 0, 0xFF}};

    test_str white = {.j = 160, .jend=200, .r={0xFF, 0, 0, 0, 0x7}, .g={0xFF, 0, 0, 0x7, 0}, .b={0xFF, 0, 0x7, 0, 0}};

    thread_t t1, t2, t3, t4, t5;
    create_thread(&t1, col_thread, 60, "test\0", (void*)&red);
    create_thread(&t2, col_thread, 60, "test\0", (void*)&green);
    create_thread(&t3, col_thread, 60, "test\0", (void*)&blue);
    create_thread(&t4, col_thread, 60, "test\0", (void*)&black);
    create_thread(&t5, col_thread, 60, "test\0", (void*)&white);

    wait_thread(t1, NULL);
    wait_thread(t2, NULL);
    wait_thread(t3, NULL);
    wait_thread(t4, NULL);
    wait_thread(t5, NULL);
#endif
    return NULL;
}