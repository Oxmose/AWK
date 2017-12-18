/***************************************
 *
 * File: queue.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/10/2017
 *
 * Version: 1.0
 *
 * See: queue.c
 *
 * Kernel queues, allow to define custom sized queues.
 * This file may be modified as the kernel might get some dynamic memory 
 * allocator. At the moment the custom size i totally artificial and all the 
 * queues take a lot of space in memory.
 */

#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../lib/stdint.h"        /* Generic int types */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "../sync/lock.h"	      /* lock_t */

/*******************************
 * CONSTANTS
 ******************************/

/*******************************
 * STRUCTURES
 ******************************/
typedef struct queue
{
	lock_t lock;

	uint32_t head;
	uint32_t tail;

	uint32_t max_length;
	uint32_t length;

	void *container[256];

	int8_t init;

	/**************************
	 * THREAD TABLE | 0: Head, 1: Tail
	 *************************/
	thread_queue_t *read_waiting_threads[2];
	thread_queue_t *write_waiting_threads[2];
} queue_t;

/*******************************
 * FUNCTIONS
 ******************************/
OS_RETURN_E queue_init(queue_t *queue, const uint32_t length);

void* queue_pend(queue_t *queue, OS_RETURN_E *error);

OS_RETURN_E queue_post(queue_t *queue, void *element);

OS_RETURN_E queue_destroy(queue_t *queue);

int32_t queue_length(queue_t *queue, OS_RETURN_E *error);

int8_t queue_isempty(queue_t *queue, OS_RETURN_E *error);

#endif /* __QUEUE_H_ */