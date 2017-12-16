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
#include "../lib/stdio.h"
#include "kernel_output.h"

void pit_action(void)
{

		printf("TICK\n");
}
void rtc_action(void)
{
   	uint32_t time = get_current_daytime();
   	date_t date = get_current_date();
	printf("TIME %02d:%02d:%02d | Date  %02d/%02d/%04d\n", time/3600, (time / 60) % 60, time % 60, date.day, date.month, date.year);

}
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

	
	/* INT DEMO */
	OS_EVENT_ID pit_id;
	OS_EVENT_ID rtc_id;
	register_pit_event(pit_action, 100, &pit_id);
	register_rtc_event(rtc_action, 1, &rtc_id);
	for(uint32_t i = 0; i < 1000000000; ++i);
	unregister_pit_event(pit_id);
	
	for(uint32_t i = 0; i < 1000000000; ++i);	
	unregister_rtc_event(rtc_id);

}