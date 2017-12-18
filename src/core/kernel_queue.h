/*******************************************************************************
 *
 * File: kernel_queues.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Kernel queues used to manage threads.
 ******************************************************************************/

#ifndef __KERNEL_QUEUE_H_
#define __KERNEL_QUEUE_H_

#include "kernel_thread.h" /* kernel_thread_t */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/stdint.h" /* Generic int types */

/*****************************************
 * STRUCTURES
 ****************************************/
/* Thread queue structure */
typedef struct thread_queue
{
	struct thread_queue* next;
	struct thread_queue* prev;

	uint32_t priority;

	kernel_thread_t* thread;
} thread_queue_t;


/************************************
 * FUNCTIONS 
 ***********************************/
OS_RETURN_E kernel_enqueue_thread(kernel_thread_t *thread, thread_queue_t *queue[2],
                           const uint32_t priority_check);

kernel_thread_t* kernel_dequeue_thread(thread_queue_t *queue[2],
                                OS_RETURN_E *error);

OS_RETURN_E kernel_remove_thread(thread_queue_t *queue[2], kernel_thread_t *thread);

#endif /* __KERNEL_QUEUE_H_ */
