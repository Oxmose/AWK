#include "../core/scheduler.h"
#include "../sync/lock.h"
#include "../lib/stdio.h"
#include "../drivers/vga_text.h"

lock_t lock;

thread_t launch;
thread_t locked_th;

void *locked_th_func(void *args)
{
    (void)args;
    printf("TH2 STARTS\n");
    printf("TH2 PRIo: %d\n", get_priority());
    spinlock_lock(&lock);

    printf("TH2 PRIo: %d\n", get_priority());

    spinlock_unlock(&lock);
    printf("TH2 ENDS\n");
    return NULL;
}

void *launch_func_block(void*args)
{
    (void)args;
    printf("TH1 STARTS\n");
    printf("TH1 PRIo: %d\n", get_priority());
    spinlock_lock(&lock);

    create_thread(&locked_th, locked_th_func, 25, "thread_func", (void*)1);
    schedule();

    printf("TH1 PRIo: %d\n", get_priority());

    spinlock_unlock(&lock);
    printf("TH1 ENDS\n");
    return NULL;
}

int test_dyn_sched(void)
{
    spinlock_init(&lock);
    if(create_thread(&launch, launch_func_block, 32, "thread_func", (void*)1) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
        return -1;
    }
    OS_RETURN_E err;
    if((err = wait_thread(launch, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
    return 0;
}