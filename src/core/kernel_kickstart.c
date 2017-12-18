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
#include "../drivers/rtc.h" /* init_rtc */
#include "../sync/lock.h"   /* enable_interrupt */
#include "kernel_output.h"  /* kernel_succes, kernel_error, kernell_info */
#include "scheduler.h"      /* init_scheduler */

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

	/* Init RTC */
	err = init_rtc();
	if(err == OS_NO_ERR)
	{
		kernel_success("RTC Initialized\n");
	}
	else
	{
		kernel_error("RTC Initialization error [%d]\n", err);
	}

	/* Enable interrupts */
	enable_interrupt();
	kernel_info("INT unleached\n");

	/* Init scheduler */
	err = init_scheduler();

	kernel_error("SCHED Initialization error [%d]\n", err);
}