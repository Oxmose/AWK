#include "../../core/scheduler.h"
#include "../../sync/mutex.h"
#include "../../core/kernel_output.h"
#include "../../lib/stdio.h"


thread_t thread_mutex2;
thread_t thread_mutex3;
thread_t thread_mutex4;
thread_t thread_mutex5;
thread_t thread_mutex1;

mutex_t mutex1;
mutex_t mutex2;

uint32_t lock_res;

void* test_rec(void* args)
{
    (void)args;
    if(mutex_pend(&mutex1))
    {
        printf("Failed to pend mutex1 1\n");
        return (void*)0;
    }

    if((int)args == 0)
    {
        if(mutex_pend(&mutex1) != OS_ERR_MUTEX_UNINITIALIZED)
        {
            printf("Failed to pend mutex1 1\n");
            return (void*)1;
        }
    }
    else
    {
        if(mutex_pend(&mutex1) != OS_NO_ERR)
        {
            printf("Failed to pend mutex1 1\n");
            return (void*)1;
        }
    }
    printf(" (T R END) ");
    return (void*)0;
}


void *mutex_thread_1(void *args)
{
    for(int i = 0; i < 1000000; ++i)
    {
        if(mutex_pend(&mutex1))
        {
            printf("Failed to pend mutex1 1\n");
            return NULL;
        }

        ++lock_res;

        //if(i % 500 == 0)
            //printf("T1\n");

        if(mutex_post(&mutex1))
        {
            printf("Failed to post mutex1 1\n");
            return NULL;
        }
    }
    printf(" (T1 END) ");
    (void )args;
    return NULL;
}
void *mutex_thread_2(void *args)
{
    for(int i = 0; i < 1000000; ++i)
    {
        if(mutex_pend(&mutex1))
        {
            printf("Failed to pend mutex1 2\n");
            return NULL;
        }


        ++lock_res;

        //if(i % 500 == 0)
            //printf("T2\n");

        if(mutex_post(&mutex1))
        {
            printf("Failed to post mutex1 2\n");
            return NULL;
        }
    }
    printf(" (T2 END) ");
    (void )args;
    return NULL;
}

void *mutex_thread_3(void *args)
{
    //printf("T3\n");
    int8_t val;
    OS_RETURN_E err;
    if((err = mutex_try_pend(&mutex2, &val) != OS_MUTEX_LOCKED) || val != 0)
    {
        printf("Failed to trypend mutex2 3, val %d | %d\n", val, err);
        perror(err);
        return NULL;
    }
    if(mutex_pend(&mutex2) != OS_ERR_MUTEX_UNINITIALIZED)
    {
        printf("Failed to pend mutex2 3\n");
        return NULL;
    }
    else
    {
        printf(" (T3 END) ");
        return NULL;
    }

    if(mutex_post(&mutex2))
    {
        printf("Failed to post mutex2 3\n");
        return NULL;
    }
    printf(" (T3 END) ");
    (void )args;
    return NULL;
}


int test_mutex(void)
{
    if(mutex_init(&mutex1, MUTEX_FLAG_NONE) != OS_NO_ERR)
    {
        printf("Failed to init mutex1\n");
        return -1;
    }
    if(mutex_init(&mutex2, MUTEX_FLAG_NONE) != OS_NO_ERR)
    {
        printf("Failed to init mutex2\n");
        return -1;
    }

    if(mutex_pend(&mutex2))
    {
        printf("Failed to pend mutex2\n");
        return -1;
    }

    lock_res = 0;

    if(create_thread(&thread_mutex1, mutex_thread_1, 1, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_mutex2, mutex_thread_2, 2, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_mutex3, mutex_thread_3, 3, "thread1", NULL) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }

    OS_RETURN_E err;
    if((err = wait_thread(thread_mutex1, NULL)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(wait_thread(thread_mutex2, NULL) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }

    /* Test non recursive mutex */
    int ret;
    if((create_thread(&thread_mutex1, test_rec, 1, "thread1", (void*)0)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }

    sleep(1000);

    if((err = mutex_destroy(&mutex1)) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy mutex1 %d\n", err);
        return -1;
    }

    if((err = wait_thread(thread_mutex1, (void*)&ret)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(ret != 0)
    {
        return -1;
    }

    /* Test recursive mutex */
    if(mutex_init(&mutex1, MUTEX_FLAG_RECURSIVE) != OS_NO_ERR)
    {
        printf("Failed to init mutex1\n");
        return -1;
    }

    if((create_thread(&thread_mutex1, test_rec, 1, "thread1", (void*)1)) != OS_NO_ERR)
    {
        kernel_error(" Error while creating the main thread!\n");
        return -1;
    }


    if((err = wait_thread(thread_mutex1, (void*)&ret)) != OS_NO_ERR)
    {
        kernel_error("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(ret != 0)
    {
        return -1;
    }

    if((err = mutex_destroy(&mutex1)) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy mutex1 %d\n", err);
        return -1;
    }

    if(mutex_destroy(&mutex2) != OS_NO_ERR)
    {
        kernel_error("Failed to destroy mutex2\n");
        return -1;
    }

    if(wait_thread(thread_mutex3, NULL) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        return -1;
    }

    /* Test recusive mutex */

    printf("Lock res = %d\n", lock_res);
    return lock_res != 2000000;
}
