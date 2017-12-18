#include "tests.h"

#include "test_sem.h"
#include "test_mutex.h"
#include "test_multithread.h"

#include "../lib/stdio.h"
#include "../drivers/vga_text.h"

static const int32_t tests_count = 3;

void *launch_tests(void*args)
{
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