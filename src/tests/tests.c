#include "tests.h"

#include "test_sem.h"
#include "test_mutex.h"
#include "test_multithread.h"
#include "test_vga.h"
#include "test_mouse.h"
#include "test_dyn_sched.h"
#include "test_ata.h"

#include "../lib/stdio.h"
#include "../drivers/vga_text.h"
#include "../drivers/vga_graphic.h"

#include "../sync/lock.h"
#include "../core/scheduler.h"

#ifdef TEST
static const int32_t tests_count = 5;
#endif

/***************
 TEST MUST BE EXECUTED ON THE LOWEST PRIORITY POSSIBLE
 ****************/

void *launch_tests(void*args)
{
    //test_mouse(NULL);

    //while(1);

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
    printf("\n");
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
    printf("\n");
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
    printf("\n");
#ifdef TEST_DYN_SCHED
    printf("4/%d\n", tests_count);
    if(test_dyn_sched())
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[ERROR]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test dyn sched failed\n");
    }
    else
    {
        set_color_scheme(FG_GREEN | BG_BLACK);
        printf("[SUCCESS]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test dyn sched passed\n");
    }
#endif
#ifdef TEST_ATA
    printf("5/%d\n", tests_count);
    if(test_ata())
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[ERROR]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test ATA failed\n");
    }
    else
    {
        set_color_scheme(FG_GREEN | BG_BLACK);
        printf("[SUCCESS]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Test ATA passed\n");
    }
#endif

    return NULL;
}