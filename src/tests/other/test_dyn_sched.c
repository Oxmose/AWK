#include "../../core/scheduler.h"
#include "../../sync/lock.h"
#include "../../lib/stdio.h"
#include "../../core/kernel_output.h"

lock_t lock;

thread_t launch;
thread_t locked_th;

void *locked_th_func(void *args)
{
    (void)args;
    printf(" (T2 P %d) ", get_priority());
    spinlock_lock(&lock);

    printf(" (T2 P %d) ", get_priority());

    spinlock_unlock(&lock);
    return NULL;
}

void *launch_func_block(void*args)
{
    (void)args;
    printf(" (T1 P %d) ", get_priority());
    spinlock_lock(&lock);

    create_thread(&locked_th, locked_th_func, 25, "thread_func", (void*)1);
    schedule();

    printf(" (T1 P %d) ", get_priority());

    spinlock_unlock(&lock);
    return NULL;
}

int test_dyn_sched(void)
{
    spinlock_init(&lock);
    if(create_thread(&launch, launch_func_block, 32, "thread_func", (void*)1) != OS_NO_ERR)
    {

        kernel_error("Error while creating the main thread!\n");
        return -1;
    }
    OS_RETURN_E err;
    if((err = wait_thread(launch, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }

    printf("\n");
    return 0;
}
