/*******************************************************************************
 *
 * File: mutex.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Mutex synchronization primitive implemantation.
 ******************************************************************************/

#ifndef __MUTEX_H_
#define __MUTEX_H_

#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "lock.h"	              /* lock_t */

/*******************************
 * STRUCTURES
 ******************************/
typedef struct mutex
{
	/**************************
	 * THREAD TABLE | 0: Head, 1: Tail
	 *************************/
	thread_queue_t *waiting_threads[2];

	/* Mutex lock state */
	uint8_t state;

	/* Spinlock to ensure atomic access to the mutex */
	lock_t lock;

	/* Init state */
	int8_t init;
} mutex_t;

OS_RETURN_E mutex_init(mutex_t *mutex);

OS_RETURN_E mutex_destroy(mutex_t *mutex);

OS_RETURN_E mutex_pend(mutex_t *mutex);

OS_RETURN_E mutex_post(mutex_t *mutex);

OS_RETURN_E mutex_try_pend(mutex_t *mutex, int8_t *value);

#endif /* __MUTEX_H_ */