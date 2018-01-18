#include "../../core/scheduler.h"
#include "../../sync/semaphore.h"
#include "../../core/kernel_output.h"
#include "../../lib/stdio.h"

thread_t thread_sem1;
thread_t thread_sem2;
thread_t thread_sem3;
thread_t thread_sem4;
thread_t thread_sem5;

semaphore_t sem1;
semaphore_t sem2;
semaphore_t sem3;
semaphore_t sem4;
semaphore_t sem_end;

uint32_t lock_res;

void *sem_thread_1(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        if(sem_pend(&sem1))
        {
            printf("Failed to pend sem1\n");
            (void )args;
            return NULL;
        }
        printf(" (T1) ");
        ++lock_res;
        sleep(500);
        if(sem_post(&sem2))
        {
            printf("Failed to post sem2\n");
            (void )args;
            return NULL;
        }
    }
    printf(" (T1 END) ");

    return NULL;
}
void *sem_thread_2(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        if(sem_pend(&sem2))
        {
            printf("Failed to pend sem2\n");
            (void )args;
            return NULL;
        }
        printf(" (T2) ");
        ++lock_res;
        sleep(300);
        if(sem_post(&sem3))
        {
            printf("Failed to post sem3\n");
            (void )args;
            return NULL;
        }
    }
    printf(" (T2 END) ");

    return NULL;
}

void *sem_thread_3(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        if(sem_pend(&sem3))
        {
            printf("Failed to pend sem3\n");
            (void )args;
            return NULL;
        }
        printf(" (T3) ");
        ++lock_res;
        if(sem_post(&sem1))
        {
            printf("Failed to post sem1\n");
            (void )args;
            return NULL;
        }
    }
    if(sem_post(&sem_end))
    {
        printf("Failed to post sem_end\n");
        (void )args;
            return NULL;
    }
    printf(" (T3 END) ");

    return NULL;
}

void *sem_thread_4(void *args)
{
    int8_t val;
    if(sem_try_pend(&sem4, &val) != OS_SEM_LOCKED)
    {
        printf("Failed to try_pend sem4\n");
        (void )args;
            return NULL;
    }
    if(val == -1)
    {
        if(sem_post(&sem1))
        {
            printf("Failed to post sem1\n");
            (void )args;
            return NULL;
        }
    }
    else
    {
        printf("Failed to try_pend sem4, wrong value\n");
        lock_res = 535;
        (void )args;
            return NULL;
    }
    for(int i = 0; i < 3; ++i)
    {
        OS_RETURN_E err;
        err = sem_pend(&sem4);
        if(err != OS_ERR_SEM_UNINITIALIZED)
        {
            printf("Failed to pend sem4,%d\n", i);
            lock_res = -3;

            (void )args;
            return NULL;
        }
    }
    printf(" (T4 END) ");

    return NULL;
}

void *sem_thread_5(void *args)
{
    for(int i = 0; i < 3; ++i)
    {
        OS_RETURN_E err;
        err = sem_pend(&sem4);
        if(err != OS_ERR_SEM_UNINITIALIZED)
        {
            printf("Failed to pend sem4,%d\n", i);
            lock_res = -3;

            (void )args;
            return NULL;
        }
    }
    printf(" (T5 END) ");

    return NULL;
}


int test_sem(void)
{
    if(sem_init(&sem1, 0) != OS_NO_ERR)
    {
        printf("Failed to init sem1\n");
        return -1;
    }
    if(sem_init(&sem2, 0) != OS_NO_ERR)
    {
        printf("Failed to init sem2\n");
        return -1;
    }
    if(sem_init(&sem3, 0) != OS_NO_ERR)
    {
        printf("Failed to init sem3\n");
        return -1;
    }
    if(sem_init(&sem4, -1) != OS_NO_ERR)
    {
        printf("Failed to init sem4\n");
        return -1;
    }
    if(sem_init(&sem_end, 0) != OS_NO_ERR)
    {
        printf("Failed to init sem_end\n");
        return -1;
    }


    lock_res = 0;

    if(create_thread(&thread_sem1, sem_thread_1, 1, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_sem2, sem_thread_2, 2, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_sem3, sem_thread_3, 3, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_sem4, sem_thread_4, 4, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_sem5, sem_thread_5, 5, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }


    if(sem_pend(&sem_end) != OS_NO_ERR)
    {
        kernel_error("Failed to pend sem_end\n");
        return -1;
    }

    if(sem_destroy(&sem1) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem1\n");
        return -1;
    }
    if(sem_destroy(&sem2) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem2\n");
        return -1;
    }
    if(sem_destroy(&sem3) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem3\n");
        return -1;
    }
    if(sem_destroy(&sem4) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem4\n");
        return -1;
    }
    if(sem_destroy(&sem_end) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy sem_end\n");
        return -1;
    }
    OS_RETURN_E err;
    if((err = wait_thread(thread_sem1, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(wait_thread(thread_sem2, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(wait_thread(thread_sem3, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(wait_thread(thread_sem4, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
     if(wait_thread(thread_sem5, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }

    printf("\n");

    return lock_res != 9;
}
