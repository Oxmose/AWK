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

	printf("\nPress enter key to start demo");
    char t;
    getch(&t);

	//thread_t test_th;
    //create_thread(&test_th, launch_tests, 64, "tests", NULL);
    //wait_thread(test_th, NULL);

    return 0;
}
