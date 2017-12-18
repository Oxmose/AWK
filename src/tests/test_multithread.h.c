#include "../core/scheduler.h"
#include "../sync/semaphore.h"

#include "../lib/stdio.h"
#include "../drivers/vga_text.h"

#include "test_multithread.h"

thread_t thread1;
thread_t thread2;
thread_t thread3;
thread_t thread4;
thread_t thread5;
thread_t thread6;
thread_t thread7;

thread_t launch;

void *test_func(void *arg)
{
	int argv = (int)arg;
	printf("%d - ", argv);
	for(unsigned int i = 0; i < 100000000; ++i);
	printf("%d - ", argv);
	for(unsigned int i = 0; i < 100000000; ++i);
	printf("%d - ", argv);
	for(unsigned int i = 0; i < 100000000; ++i);
	printf("%d - ", argv);
	for(unsigned int i = 0; i < 100000000; ++i);

	printf("\n%d - END\n", argv);

return NULL;
}

void *launch_func(void*args)
{
    (void)args;
	if(create_thread(&thread1, test_func, 1, "thread1", (void*) 1) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }
    if(create_thread(&thread2, test_func, 1, "thread2", (void*) 2) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }
    if(create_thread(&thread3, test_func, 1, "thread3", (void*) 3) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }
    if(create_thread(&thread4, test_func, 4, "thread4",(void*) 4) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }
    if(create_thread(&thread5, test_func, 4, "thread5", (void*) 5) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }
    if(create_thread(&thread6, test_func, 4, "thread6", (void*) 6) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }
    if(create_thread(&thread7, test_func, 10, "thread7", (void*) 7) != OS_NO_ERR)
    {
        set_color_scheme(FG_RED | BG_BLACK);
        printf("[RDLK]");
        set_color_scheme(FG_WHITE | BG_BLACK);
        printf(" Error while creating the main thread!\n");
    }

    sleep(15000);

	OS_RETURN_E err;
	if((err = wait_thread(thread1, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
    if((err = wait_thread(thread2, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);    
    }
    if((err = wait_thread(thread3, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
    if((err = wait_thread(thread4, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
 	if((err = wait_thread(thread5, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
    if((err = wait_thread(thread6, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
    if((err = wait_thread(thread7, NULL)) != OS_NO_ERR)
    {
        printf("Error while waiting thread! [%d]\n", err);
        perror(err);
    }
    return NULL;
}

int test_multithread(void)
{
	if(create_thread(&launch, launch_func, 0, "thread_func", (void*)1) != OS_NO_ERR)
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