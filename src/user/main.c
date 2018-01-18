#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"

#include "../drivers/keyboard.h"

extern void* launch_tests(void*);

void* th(void*args)
{
	printf("HI %d ", (int)args);
	return NULL;
}

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;


	thread_t test_ths[200];
    thread_t test_th;

	for(int i = 0; i < 200; ++i)
	{
		create_thread(&test_ths[i], th, 0, "tests\0", (void*)i);
	}

	for(int i = 0; i < 200; ++i)
	{
		wait_thread(test_ths[i], NULL);
	}

	printf("\n");

    create_thread(&test_th, launch_tests, 64, "tests\0", NULL);
    wait_thread(test_th, NULL);

    printf("\nPress enter key to start demo");
    char t;
    getch(&t);

	printf("MAIN OUT \n");

    return 0;
}
