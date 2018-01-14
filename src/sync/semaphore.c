/*******************************************************************************
 *
 * File: semaphore.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 2.0
 *
 * Semaphore synchronization primitive implemantation.
 ******************************************************************************/

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/string.h"         /* memset */
#include "../core/kernel_list.h"   /* kernel_list_t, kernel_list_node_t */
#include "../core/kernel_output.h" /* kernel_error */
#include "../core/panic.h"         /* kernel_panic */
#include "../core/scheduler.h"     /* lock_thread, unlock_thread */
#include "lock.h"                  /* lock_t */

/* Header include */
#include "semaphore.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E sem_init(semaphore_t* sem, const int32_t init_level)
{
    OS_RETURN_E err;

    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the semaphore*/
    memset(sem, 0, sizeof(semaphore_t));

    sem->sem_level = init_level;
    spinlock_init(&sem->lock);

    sem->waiting_threads = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    sem->init = 1;

    return OS_NO_ERR;
}

OS_RETURN_E sem_destroy(semaphore_t* sem)
{
    kernel_list_node_t* node;
    OS_RETURN_E         err;

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
    while((node = kernel_list_delist_data(sem->waiting_threads, &err)) != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
            kernel_panic();
        }
        err = unlock_thread(node, SEM, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from semaphore[%d]\n", err);
            kernel_panic();
        }
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
        kernel_panic();
    }

    spinlock_unlock(&sem->lock);

    return OS_NO_ERR;
}

OS_RETURN_E sem_pend(semaphore_t* sem)
{
    OS_RETURN_E         err;
    kernel_list_node_t* active_thread;

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
        active_thread = lock_thread(SEM);
        if(active_thread == NULL)
        {
            kernel_error("Could not lock this thread to semaphore[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic();
        }

        err = kernel_list_enlist_data(active_thread,
                                      sem->waiting_threads, 0);

        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread from semaphore[%d]\n", err);
            kernel_panic();
        }

        spinlock_unlock(&sem->lock);
        schedule();
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

OS_RETURN_E sem_post(semaphore_t* sem)
{
    kernel_list_node_t* node;
    OS_RETURN_E         err;

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
        if((node = kernel_list_delist_data(sem->waiting_threads, &err))
            != NULL)
        {
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
                kernel_panic();
            }

            spinlock_unlock(&sem->lock);

            /* Do not schedule, sem can be used in interrupt handlers */
            err = unlock_thread(node, SEM, 0);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not unlock thread from semaphore[%d]\n", err);
                kernel_panic();
            }

            return OS_NO_ERR;
        }
        if(err != OS_NO_ERR)
        {
           kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
           kernel_panic();
        }
    }

    /* If here, we did not find any waiting process */
    spinlock_unlock(&sem->lock);

    return OS_NO_ERR;
}

OS_RETURN_E sem_try_pend(semaphore_t* sem, int8_t* value)
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
