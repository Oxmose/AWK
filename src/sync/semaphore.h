/*******************************************************************************
 *
 * File: semaphore.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Semaphore synchronization primitive implemantation.
 ******************************************************************************/

#ifndef __SEMAPHORE_H_
#define __SEMAPHORE_H_

#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "lock.h"	              /* lock_t */

/*******************************
 * STRUCTURES
 ******************************/
typedef struct semaphore
{
	/**************************
	 * THREAD TABLE | 0: Head, 1: Tail
	 *************************/
	thread_queue_t *waiting_threads[2];

	/* Semaphore counter */
	int32_t sem_level;

	/* Spinlock to ensure atomic access to the semaphore */
	lock_t lock;

	/* Init state */
	int8_t init;
} semaphore_t;

OS_RETURN_E sem_init(semaphore_t *sem, const int32_t init_level);

OS_RETURN_E sem_destroy(semaphore_t *sem);

OS_RETURN_E sem_pend(semaphore_t *sem);

OS_RETURN_E sem_post(semaphore_t *sem);

OS_RETURN_E sem_try_pend(semaphore_t *sem, int8_t *value);

#endif /* __SEMAPHORE_H_ */