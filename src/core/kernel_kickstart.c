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

#include "kernel_queue.h"
#include "../lib/stdio.h"

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
#if 0
	int val;
	thread_queue_t *queue[2] = {NULL};
	enqueue_thread((kernel_thread_t*)1, queue, 0);
	enqueue_thread((kernel_thread_t*)2, queue, 0);
	enqueue_thread((kernel_thread_t*)3, queue, 0);
	enqueue_thread((kernel_thread_t*)4, queue, 4);
	enqueue_thread((kernel_thread_t*)5, queue, 5);
	enqueue_thread((kernel_thread_t*)6, queue, 6);
	enqueue_thread((kernel_thread_t*)7, queue, 4);
	enqueue_thread((kernel_thread_t*)8, queue, 4);
	enqueue_thread((kernel_thread_t*)9, queue, 7);
	enqueue_thread((kernel_thread_t*)10, queue, 10);
	enqueue_thread((kernel_thread_t*)11, queue, 0);

	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);
	val = (int)dequeue_thread(queue, NULL);
	printf("VAL=%d\n", val);

	while(1);
#endif
	/* Enable interrupts */
	enable_interrupt();
	kernel_info("INT unleached\n");

	/* Init scheduler */
	err = init_scheduler();

	kernel_error("SCHED Initialization error [%d]\n", err);
}