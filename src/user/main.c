#include "../lib/stdio.h"
#include "../lib/string.h"
#include "../core/scheduler.h"

#include "../drivers/ata.h"
#include "../drivers/keyboard.h"

#include "../sync/semaphore.h"

#include "../core/kernel_output.h"
#include "../cpu/bios_call.h"

extern void* launch_tests(void*);
extern int test_vesa(void);

int main(int argc, char** argv)
{
	(void)argc;
	(void)argv;
    //thread_t test_th;
    //create_thread(&test_th, launch_tests, 64, "tests\0", NULL);



    //wait_thread(test_th, NULL);

    printf("\nPress enter key to start demo");
    char t;
    getch(&t);

    //test_vesa();
    return 0;
}