#include "../core/scheduler.h"
#include "../sync/mutex.h"

#include "../lib/stdio.h"
#include "../drivers/vga_text.h"


thread_t thread_mutex2;
thread_t thread_mutex3;
thread_t thread_mutex4;
thread_t thread_mutex5;
thread_t thread_mutex1;

mutex_t mutex1;
mutex_t mutex2;

uint32_t lock_res;

void *mutex_thread_1(void *args)
{
	for(int i = 0; i < 1000; ++i)
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
		sleep(1);
	}
	printf("Thread 1 end\n");
	(void )args;
	return NULL;
}
void *mutex_thread_2(void *args)
{
	for(int i = 0; i < 1000; ++i)
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
		sleep(1);
	}	
	printf("Thread 2 end\n");
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
		//printf("Thread 3 end\n");
		return NULL;
	}

	if(mutex_post(&mutex2))
	{
		printf("Failed to post mutex2 3\n");
		return NULL;
	}
	printf("Thread 3 end\n");
	(void )args;
	return NULL;
}


int test_mutex(void)
{
	if(mutex_init(&mutex1) != OS_NO_ERR)
	{
		printf("Failed to init mutex1\n");
		return -1;
	}
	if(mutex_init(&mutex2) != OS_NO_ERR)
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
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_mutex2, mutex_thread_2, 2, "thread1", NULL) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
        return -1;
    }
    if(create_thread(&thread_mutex3, mutex_thread_3, 3, "thread1", NULL) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
        return -1;
    }

    OS_RETURN_E err;
	if((err = wait_thread(thread_mutex1, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        return -1;
    }
    if(wait_thread(thread_mutex2, NULL) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        return -1;
    }

	if(mutex_destroy(&mutex1) != OS_NO_ERR)
	{
		printf("Failed to destroy mutex1\n");
		return -1;
	}
	if(mutex_destroy(&mutex2) != OS_NO_ERR)
	{
		printf("Failed to destroy mutex2\n");
		return -1;
	}

	
    if(wait_thread(thread_mutex3, NULL) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        return -1;
    }

    return lock_res != 2000;
}