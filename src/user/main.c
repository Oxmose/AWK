#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"
#include "../core/power_management.h"
#include "../drivers/vesa.h"
#include "../drivers/keyboard.h"


extern void* launch_tests(void*);
extern void* test_mouse2(void*);
int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;


	OS_RETURN_E err = OS_NO_ERR;
	err = vesa_enable_double_buffering();
	if(err != OS_NO_ERR)
	{
		printf("\nERROR WHILE ENABLING VESA DOUBLE BUFFER %d\n", err);
	}

    thread_t test_th;

	printf("\n");
	printf("\nPress enter key to start demo");
    char t;
    getch(&t);

	test_mouse2(NULL);

    create_thread(&test_th, launch_tests, 64, "tests\0", NULL);
    wait_thread(test_th, NULL);


	printf("MAIN OUT \n");

	//while(1)
	//{
	//	schedule();
//	}

    return 0;
}
