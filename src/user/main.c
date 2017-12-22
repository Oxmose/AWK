#include "../lib/stdio.h"
#include "../core/scheduler.h"

extern void* launch_tests(void*);

thread_t threads[10];

void *th_func2(void *args)
{
	(void)args;
	for(int i = 0; i < 1000000000; ++i);
	return NULL;
}

void *th_func(void *args)
{
		create_thread(threads + 5 + (int)args, th_func2, 64, "th_dep\0", NULL);
	
	return NULL;
}

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; ++i)
	{
		printf("Arg %d: %s\n", i, argv[i]);
	}
	int32_t size = 50;
	thread_info_t array[50];

	for(int i = 0; i < 5; ++i)
	{
		create_thread(threads + i, th_func, 64, "th\0", (void*) i);
	}

	get_threads_info(array, &size);


	for(int i = 0; i < size; ++i)
	{
		/*printf("%s | PID: %d | PPID: %d | PRIO: %d | STATE: %d\n\tStart time: %d | Execution time: %d | End time: %d\n", 
			array[i].name,
			array[i].pid,
			array[i].ppid,
			array[i].priority,
			array[i].state,
			array[i].start_time,
			array[i].exec_time,
			array[i].end_time
			);*/
		printf("\t%d Child: ",array[i].pid);
		thread_queue_t *cursor = array[i].children;
		while(cursor != NULL)
		{
			printf("%d ", cursor->thread->pid);
			cursor = cursor->next;
		}
		printf("\n");
	}

	for(int i = 0; i < 5; ++i)
	{
		wait_thread(threads[i], NULL);
	}

	printf("\n");
	size = 50;
	get_threads_info(array, &size);



	for(int i = 0; i < size; ++i)
	{
		/*printf("%s | PID: %d | PPID: %d | PRIO: %d | STATE: %d\n\tStart time: %d | Execution time: %d | End time: %d\n", 
			array[i].name,
			array[i].pid,
			array[i].ppid,
			array[i].priority,
			array[i].state,
			array[i].start_time,
			array[i].exec_time,
			array[i].end_time
			);*/
		printf("\t%d Child: ",array[i].pid);
		thread_queue_t *cursor = array[i].children;
		while(cursor != NULL)
		{
			printf("%d ", cursor->thread->pid);
			cursor = cursor->next;
		}
		printf("\n");
	}

	//thread_t tests;
	//create_thread(&tests, launch_tests, 63, "tests\0", NULL);
	//wait_thread(tests, NULL);
	return 0;
}