/***************************************
 *
 * File: semaphore.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 06/10/2017
 *
 * Version: 1.0
 *
 * See: semaphore.c
 *
 * Semaphore synchronization primitive implemantation.
 */
/* Sys includes */
#include "../lib/stddef.h"
#include "../core/scheduler.h"
#include "../core/kernel_queue.h"
#include "lock.h"

/* Header include */
#include "semaphore.h"

OS_RETURN_E sem_init(semaphore_t *sem, const int32_t init_level)
{
	if(sem == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	/* Init the semaphore*/
	sem->sem_level = init_level;

	spinlock_init(&sem->lock);

	sem->waiting_threads[0] = NULL;
	sem->waiting_threads[1] = NULL;

	sem->init = 1;

	return OS_NO_ERR;
}

OS_RETURN_E sem_destroy(semaphore_t *sem)
{
	/* Check if semaphore is initialized */
	if(sem == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&sem->lock);

	if(sem->init != 1)
	{
		spinlock_unlock(&sem->lock);

		return OS_ERR_SEM_UNINITIALIZED;
	}

	sem->init = 0;

	/* Unlock all threead*/
	kernel_thread_t *thread;
	OS_RETURN_E err;
	while((thread = dequeue_thread(sem->waiting_threads, &err)) != NULL)
    {
    	if(err != OS_NO_ERR)
        {
        	/* TODO PANIC */
        }
        unlock_thread(thread, SEM, 0);
    }
    if(err != OS_NO_ERR)
    {
    	/* TODO PANIC */
    }

	spinlock_unlock(&sem->lock);
	
	return OS_NO_ERR;
}

OS_RETURN_E sem_pend(semaphore_t *sem)
{
	/* Check if semaphore is initialized */
	if(sem == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&sem->lock);

	if(sem->init != 1)
	{
		spinlock_unlock(&sem->lock);

		return OS_ERR_SEM_UNINITIALIZED;
	}

	/* Check if we can enter the critical section, also check if the semaphore
	 * has not been destroyed
	 */
	while(sem != NULL &&
		  sem->init == 1 &&
		  sem->sem_level < 1)
	{	
		if(enqueue_thread(get_active_thread(), sem->waiting_threads, 0)
                != OS_NO_ERR)
	    {
	        /* TODO PANIC */
	    }
		
		spinlock_unlock(&sem->lock);

		lock_thread(SEM);
		
		spinlock_lock(&sem->lock);
	}

	if(sem == NULL || sem->init != 1)
	{
		spinlock_unlock(&sem->lock);

		return OS_ERR_SEM_UNINITIALIZED;
	}

	/* Decrement sem level */
	--(sem->sem_level);

	spinlock_unlock(&sem->lock);

	return OS_NO_ERR;
}

OS_RETURN_E sem_post(semaphore_t *sem)
{
	/* Check if semaphore is initialized */
	if(sem == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&sem->lock);

	if(sem->init != 1)
	{
		spinlock_unlock(&sem->lock);

		return OS_ERR_SEM_UNINITIALIZED;
	}

	/* Increment sem level */
	++sem->sem_level;

	/* Check if we can unlock a blocked thread on the semaphore */
	if(sem->sem_level > 0)
	{
		kernel_thread_t *thread;
		OS_RETURN_E err;
		if((thread = dequeue_thread(sem->waiting_threads, &err)) 
			!= NULL)
        {
        	if(err != OS_NO_ERR)
		    {
		        /* TODO PANIC */
		    }

        	spinlock_unlock(&sem->lock);

            unlock_thread(thread, SEM, 1);

           	return OS_NO_ERR;
        }
        if(err != OS_NO_ERR)
	    {
	        /* TODO PANIC */
	    }
	}

	/* If here, we did not find any waiting process */
	spinlock_unlock(&sem->lock);

	return OS_NO_ERR;
}

OS_RETURN_E sem_try_pend(semaphore_t *sem, int8_t *value)
{
	/* Check if semaphore is initialized */
	if(sem == NULL || value == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	spinlock_lock(&sem->lock);

	if(sem->init != 1)
	{
		spinlock_unlock(&sem->lock);

		return OS_ERR_SEM_UNINITIALIZED;
	}

	/* Check if we can enter the critical section, also check if the semaphore
	 * has not been destroyed
	 */
	if(sem != NULL &&
	   sem->sem_level < 1)
	{	
		*value = sem->sem_level;

		spinlock_unlock(&sem->lock);

		return OS_SEM_LOCKED;
	}
	else if(sem != NULL && sem->init == 1)
	{
		*value = --sem->sem_level;
	}
	else 
	{
		spinlock_unlock(&sem->lock);

		return OS_ERR_SEM_UNINITIALIZED;
	}

	spinlock_unlock(&sem->lock);

	return OS_NO_ERR;
}