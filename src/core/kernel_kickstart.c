
#include "../drivers/pit.h"
#include "../sync/lock.h"

#include "kernel_output.h"

void kernel_kickstart(void)
{
	OS_RETURN_E err;

	/* Init PIT */
	err = init_pit();
	if(err == OS_NO_ERR)
	{
		kernel_success("PIT Initialized\n", 16);
	}
	else
	{
		kernel_error("PIT Initialization error [", 26);
		kernel_print_unsigned_hex(err, 3);
		kernel_print("]\n", 2);
	}


	enable_interrupt();
	kernel_success("INT unleached\n", 14);
}