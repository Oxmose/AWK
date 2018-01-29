#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"
#include "../core/power_management.h"
#include "../drivers/vesa.h"
#include "../drivers/keyboard.h"
#include "../tests/other/tests.h"
#include "../gui/gui.h"

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;

	//thread_t test_th[2];
	//create_thread(&test_th[0], launch_tests, 64, "tests\0", NULL);
	//wait_thread(test_th[0], NULL);

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

	keyboard_disable_display();
	if(start_gui() != OS_NO_ERR)
	{
		kernel_panic();
	}

    return 0;
}
