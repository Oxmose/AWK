#include "tests.h"

#include "test_sem.h"
#include "test_mutex.h"
#include "test_multithread.h"

#include "../lib/stdio.h"
#include "../drivers/vga_text.h"
#include "../drivers/vga_graphic.h"

#include "../sync/lock.h"
#include "../core/scheduler.h"

static const int32_t tests_count = 3;


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

void *launch_tests(void*args)
{
    init_vga();

    for(int j = 0; j < 200; ++j)
    {
        for(int i = 0; i < 320; ++i)
        {
            draw_pixel(i, j, 0xFF, 0xFF, 0xFF);
        }
        
    }

    test_str red   = {.j = 0, .jend=40,    .r={0xA8, 0xFF, 0, 0, 0}, .g={0, 0xFF, 0, 0, 0xA8}, .b={0, 0xFF, 0, 0xA8, 0}};

    test_str green = {.j = 40, .jend=80,   .r={0, 0xA8, 0xFF, 0, 0}, .g={0xA8, 0, 0xFF, 0, 0}, .b={0, 0, 0xFF, 0, 0xA8}};

    test_str blue  = {.j = 80, .jend=120,  .r={0, 0, 0xA8, 0xFF, 0}, .g={0, 0xA8, 0, 0xFF, 0}, .b={0xA8, 0 ,0, 0xFF, 0}};

    test_str black = {.j = 120, .jend=160, .r={0, 0, 0, 0xA8, 0xFF}, .g={0, 0, 0xA8, 0, 0xFF}, .b={0, 0xA8 ,0, 0, 0xFF}};

    test_str white = {.j = 160, .jend=200, .r={0xFF, 0, 0, 0, 0xA8}, .g={0xFF, 0, 0, 0xA8, 0}, .b={0xFF, 0, 0xA8, 0, 0}};

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

    while(1);

    (void)args;
#ifdef TEST_SEM
    printf("1/%d\n", tests_count);
    if(test_sem())
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[ERROR]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test semaphores failed\n");
    }
    else
    {
        set_color_scheme(FG_GREEN | BG_BLACK);
        printf("[SUCCESS]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test semaphores passed\n");
    }
#endif
#ifdef TEST_MUTEX
    printf("2/%d\n", tests_count);
    if(test_mutex())
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[ERROR]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test mutex failed\n");
    }
    else
    {
        set_color_scheme(FG_GREEN | BG_BLACK);
        printf("[SUCCESS]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test mutex passed\n");
    }
#endif
#ifdef TEST_MULTITHREAD
    printf("3/%d\n", tests_count);
    if(test_multithread())
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[ERROR]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test multithread failed\n");
    }
    else
    {
        set_color_scheme(FG_GREEN | BG_BLACK);
        printf("[SUCCESS]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test multithread passed\n");
    }
#endif

    return NULL;
}