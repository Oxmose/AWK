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

	printf("AWK v0.4.52\n");

    char login[25];
	char passwd[25];

	do
	{
		printf("Login: ");
	    secure_read_keyboard(login, 24);
		printf("Password: ");
		secure_read_keyboard(passwd, 24);
		printf("\n");
	}
	while(strncmp(passwd, "root", 4) != 0);

	printf("Welcome %s\n", login);
	sleep(2000);

	keyboard_disable_display();
	if(start_gui() != OS_NO_ERR)
	{
		kernel_panic();
	}

    return 0;
}
