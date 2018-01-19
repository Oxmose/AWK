#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"

#include "../drivers/keyboard.h"

extern void* launch_tests(void*);

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;


    thread_t test_th;

	printf("\n");

    create_thread(&test_th, launch_tests, 64, "tests\0", NULL);
    wait_thread(test_th, NULL);

    printf("\nPress enter key to start demo");
    char t;
    getch(&t);

	printf("MAIN OUT \n");

    return 0;
}
