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

	test_dyn_sched();
	
	return 0;
}