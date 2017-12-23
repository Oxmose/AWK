#include "../lib/stdio.h"
#include "../core/scheduler.h"

extern void* launch_tests(void*);
extern int test_dyn_sched(void);


int main(int argc, char** argv)
{
	for(int i = 0; i < argc; ++i)
	{
		printf("Arg %d: %s\n", i, argv[i]);
	}

	create_thread(NULL, launch_tests, 64, "tests\0", NULL);
	
	return 0;
}