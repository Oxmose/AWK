/*******************************************************************************
 *
 * File: kernel_kickstart.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/12/2017
 *
 * Version: 1.0
 *
 * Init the rest of the kernel after GDT, IDT and PIC have been initialized.
 * AT THIS POINT INTERRUPT SHOULD BE DISABLED
 ******************************************************************************/

#include "../drivers/pit.h" /* init_pit */
#include "../sync/lock.h"   /* enable_interrupt */

#include "kernel_output.h"

void kernel_kickstart(void)
{
	OS_RETURN_E err;

	/* Init PIT */
	err = init_pit();
	if(err == OS_NO_ERR)
	{
		kernel_success("PIT Initialized\n");
	}
	else
	{
		kernel_error("PIT Initialization error [%d]\n", err);
	}

	/* Enable interrupts */
	enable_interrupt();
	kernel_info("INT unleached\n");

	for(int i = 0; i < 5; ++i)
	{
		int m = 5/i;
		m++;
	}
}