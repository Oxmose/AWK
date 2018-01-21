#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"
#include "../core/power_management.h"
#include "../drivers/vesa.h"
#include "../drivers/keyboard.h"
#include "../tests/other/tests.h"

extern void start_gui(void);

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



	printf("\n");
	printf("\nPress enter key to start demo");
    char t;
    getch(&t);

	printf("MEM IS %08x\n", (uint32_t) launch_tests);

	thread_t test_th[2];
    create_thread(&test_th[0], launch_tests, 64, "tests\0", NULL);
    wait_thread(test_th[0], NULL);

	start_gui();

	printf("MAIN OUT \n");

    return 0;
}
