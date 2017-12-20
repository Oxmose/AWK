#include "../lib/stdio.h"
#include "../core/scheduler.h"

extern void* launch_tests(void*);

int main(int argc, char** argv)
{
	for(int i = 0; i < argc; ++i)
	{
		printf("Arg %d: %s\n", i, argv[i]);
	}
	sleep(1000);
	int32_t size = 50;
	thread_info_t array[50];

	get_threads_info(array, &size);

	for(int i = 0; i < size; ++i)
	{
		printf("%s | PID: %d | PPID: %d | PRIO: %d | STATE: %d\n\tStart time: %d | Execution time: %d | End time: %d\n", 
			array[i].name,
			array[i].pid,
			array[i].ppid,
			array[i].priority,
			array[i].state,
			array[i].start_time,
			array[i].exec_time,
			array[i].end_time
			);
	}

	launch_tests(NULL);

	return 0;
}