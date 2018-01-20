/*******************************************************************************
 *
 * File: mutex.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 2.0
 *
 * Mutex synchronization primitive implemantation.
 ******************************************************************************/

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/string.h"         /* memset */
#include "../core/kernel_list.h"   /* kernel_list_t, kernel_list_node_t */
#include "../core/kernel_output.h" /* kernel_error */
#include "../core/panic.h"         /* kernel_panic */
#include "../core/scheduler.h"     /* lock_thread, unlock_thread */
#include "lock.h"                  /* lock_t */

#include "../debug.h"            /* DEBUG */

/* Header include */
#include "mutex.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E mutex_init(mutex_t* mutex, const uint32_t flags)
{
    OS_RETURN_E err;

    if(mutex == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the mutex*/
    memset(mutex, 0, sizeof(mutex_t));

    mutex->state = 1;
    spinlock_init(&mutex->lock);
    mutex->flags = flags;

    mutex->waiting_threads = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    mutex->init = 1;

    #ifdef DEBUG_MUTEX
    kernel_serial_debug("Mutex 0x%08x initialized\n", (uint32_t)mutex);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E mutex_destroy(mutex_t* mutex)
{
    kernel_list_node_t* node;
    OS_RETURN_E         err;

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
    while((node = kernel_list_delist_data(mutex->waiting_threads, &err))
        != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from mutex[%d]\n", err);
            kernel_panic();
        }

        err = unlock_thread(node, MUTEX, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mutex[%d]\n", err);
            kernel_panic();
        }

        #ifdef DEBUG_MUTEX
        kernel_serial_debug("Mutex 0x%08x unlocked thead %d\n",
                            (uint32_t)mutex,
                            ((kernel_thread_t*)node->data)->pid);
        #endif
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mutex[%d]\n", err);
        kernel_panic();
    }


    err = kernel_list_delete_list(&mutex->waiting_threads);
    mutex->init = 0;

    #ifdef DEBUG_MUTEX
    kernel_serial_debug("Mutex 0x%08x destroyed\n", (uint32_t)mutex);
    #endif

    spinlock_unlock(&mutex->lock);

    return OS_NO_ERR;
}

OS_RETURN_E mutex_pend(mutex_t* mutex)
{
    OS_RETURN_E         err;
    kernel_list_node_t* active_thread;

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
    while(mutex->init == 1 &&
          mutex->state != 1)
    {
        /* If the mutex is recursive and the thread acuired the mutex,
         * then don't block the thread
         */
        if((mutex->flags & MUTEX_FLAG_RECURSIVE) != 0 &&
            get_pid() == mutex->locker_pid)
        {
            break;
        }

        active_thread = lock_thread(MUTEX);
        if(active_thread == NULL)
        {
            kernel_error("Could not lock this thread to mutex[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic();
        }

        err = kernel_list_enlist_data(active_thread,
                                      mutex->waiting_threads, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to mutex[%d]\n", err);
            kernel_panic();
        }

        #ifdef DEBUG_MUTEX
        kernel_serial_debug("Mutex 0x%08x locked thead %d\n",
                            (uint32_t)mutex,
                            ((kernel_thread_t*)active_thread->data)->pid);
        #endif

        spinlock_unlock(&mutex->lock);
        schedule();
        spinlock_lock(&mutex->lock);
    }

    if(mutex->init != 1)
    {
        spinlock_unlock(&mutex->lock);

        return OS_ERR_MUTEX_UNINITIALIZED;
    }

    /* Set state to busy */
    mutex->state = 0;

    mutex->locker_pid = get_pid();

    #ifdef DEBUG_MUTEX
    kernel_serial_debug("Mutex 0x%08x aquired by thead %d\n",
                        (uint32_t)mutex,
                        get_pid());
    #endif

    spinlock_unlock(&mutex->lock);

    return OS_NO_ERR;
}

OS_RETURN_E mutex_post(mutex_t* mutex)
{
    kernel_list_node_t* node;
    OS_RETURN_E         err;

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
    if((node = kernel_list_delist_data(mutex->waiting_threads, &err)) != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from mutex[%d]\n", err);
            kernel_panic();
        }

        #ifdef DEBUG_MUTEX
        kernel_serial_debug("Mutex 0x%08x unlocked thead %d\n",
                            (uint32_t)mutex,
                            ((kernel_thread_t*)node->data)->pid);
        #endif

        spinlock_unlock(&mutex->lock);

        err = unlock_thread(node, MUTEX, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mutex[%d]\n", err);
            kernel_panic();
        }

        #ifdef DEBUG_MUTEX
        kernel_serial_debug("Mutex 0x%08x released by thead %d\n",
                            (uint32_t)mutex,
                            get_pid());
        #endif

        return OS_NO_ERR;
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mutex[%d]\n", err);
        kernel_panic();
    }

    #ifdef DEBUG_MUTEX
    kernel_serial_debug("Mutex 0x%08x released by thead %d\n",
                        (uint32_t)mutex,
                        get_pid());
    #endif

    /* If here, we did not find any waiting process */
    spinlock_unlock(&mutex->lock);

    return OS_NO_ERR;
}

OS_RETURN_E mutex_try_pend(mutex_t* mutex, int8_t* value)
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

        #ifdef DEBUG_MUTEX
        kernel_serial_debug("Locked mutex 0x%08x try pend by thead %d\n",
                            (uint32_t)mutex,
                            get_pid());
        #endif

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

    #ifdef DEBUG_MUTEX
    kernel_serial_debug("Unlocked mutex 0x%08x try pend and aquired by thead %d\n",
                        (uint32_t)mutex,
                        get_pid());
    #endif

    spinlock_unlock(&mutex->lock);

    return OS_NO_ERR;
}
