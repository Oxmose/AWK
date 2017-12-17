/*******************************************************************************
 *
 * File: mutex.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Mutex synchronization primitive implemantation.
 ******************************************************************************/

#include "../core/scheduler.h"    /* lock_thread, unlock_thread */
#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "lock.h"	              /* lock_t */

/* Header include */
#include "mutex.h"

OS_RETURN_E mutex_init(mutex_t *mutex)
{
	if(mutex == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	/* Init the mutex*/
	mutex->state = 1;

	spinlock_init(&mutex->lock);

	mutex->waiting_threads[0] = NULL;
	mutex->waiting_threads[1] = NULL;

	mutex->init = 1;

	return OS_NO_ERR;
}

OS_RETURN_E mutex_destroy(mutex_t *mutex)
{
	/* Check if mutex is initialized */
	if(mutex == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&mutex->lock);

	if(mutex->init != 1)
	{
		spinlock_unlock(&mutex->lock);

		return OS_ERR_MUTEX_UNINITIALIZED;
	}

	/* Unlock all thread*/
	kernel_thread_t *thread;
	OS_RETURN_E err;
	while((thread = dequeue_thread(mutex->waiting_threads, &err)) 
		!= NULL)
    {
    	if(err != OS_NO_ERR)
        {
        	/* TODO PANIC */
        }
        unlock_thread(thread, MUTEX, 0);
    }
    if(err != OS_NO_ERR)
    {
    	/* TODO PANIC */
    }


	mutex->init = 0;

	spinlock_unlock(&mutex->lock);
	
	return OS_NO_ERR;
} 

OS_RETURN_E mutex_pend(mutex_t *mutex)
{
	/* Check if mutex is initialized */
	if(mutex == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&mutex->lock);

	if(mutex->init != 1)
	{
		spinlock_unlock(&mutex->lock);

		return OS_ERR_MUTEX_UNINITIALIZED;
	}

	/* Check if we can enter the critical section, also check if the mutex
	 * has not been destroyed
	 */
	while(mutex != NULL &&
		  mutex->init == 1 &&
		  mutex->state != 1)
	{	
		if(enqueue_thread(get_active_thread(), mutex->waiting_threads, 0)
                != OS_NO_ERR)
	    {
	        /* TODO PANIC */
	    }

		spinlock_unlock(&mutex->lock);

		lock_thread(MUTEX);

		spinlock_lock(&mutex->lock);
	}

	if(mutex == NULL || mutex->init != 1)
	{
		spinlock_unlock(&mutex->lock);

		return OS_ERR_MUTEX_UNINITIALIZED;
	}

	/* Set state to busy */
	mutex->state = 0;

	spinlock_unlock(&mutex->lock);

	return OS_NO_ERR;
}

OS_RETURN_E mutex_post(mutex_t *mutex)
{
	/* Check if mutex is initialized */
	if(mutex == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&mutex->lock);

	if(mutex->init != 1)
	{
		spinlock_unlock(&mutex->lock);

		return OS_ERR_MUTEX_UNINITIALIZED;
	}

	/* Increment mutex level */
	mutex->state = 1;

	/* Check if we can unlock a blocked thread on the mutex */
	kernel_thread_t *thread;
	OS_RETURN_E err;
	if((thread = dequeue_thread(mutex->waiting_threads, &err)) != NULL)
    {
    	if(err != OS_NO_ERR)
	    {
	        /* TODO PANIC */
	    }

    	spinlock_unlock(&mutex->lock);

        unlock_thread(thread, MUTEX, 1);

       	return OS_NO_ERR;
    }
    if(err != OS_NO_ERR)
    {
        /* TODO PANIC */
    }

	/* If here, we did not find any waiting process */
	spinlock_unlock(&mutex->lock);

	return OS_NO_ERR;
}

OS_RETURN_E mutex_try_pend(mutex_t *mutex, int8_t *value)
{
	/* Check if mutex is initialized */
	if(mutex == NULL || value == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&mutex->lock);

	if(mutex->init != 1)
	{
		spinlock_unlock(&mutex->lock);

		return OS_ERR_MUTEX_UNINITIALIZED;
	}


	/* Check if we can enter the critical section, also check if the mutex
	 * has not been destroyed
	 */
	if(mutex != NULL &&
	   mutex->state != 1)
	{	
		*value = mutex->state;

		spinlock_unlock(&mutex->lock);

		return OS_MUTEX_LOCKED;
	}
	else if(mutex != NULL &&mutex->init == 1)
	{
		mutex->state = 0;
	}
	else 
	{
		spinlock_unlock(&mutex->lock);

		return OS_ERR_MUTEX_UNINITIALIZED;
	}

	spinlock_unlock(&mutex->lock);

	return OS_NO_ERR;
}