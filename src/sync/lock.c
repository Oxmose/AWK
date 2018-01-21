/*******************************************************************************
 *
 * File: lock.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Basic lock and synchronization primitives
 ******************************************************************************/

#include "../cpu/cpu.h"         /* cpu_test_and_set */
#include "../lib/stdint.h"      /* Generic int types */
#include "../lib/stddef.h"      /* OS_RETURN_E */
#include "../core/interrupts.h" /* enable_local_interrupt, disable_local_interrupt */

/* Header file */
#include "lock.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E spinlock_lock(lock_t* lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#ifdef KERNEL_MONOCORE_SYNC
    disable_local_interrupt();
#else
    if(lock->pid == get_pid())
    {
        if(lock->nest_level < UINT16_MAX)
        {
            ++lock->nest_level;
        }
        return OS_NO_ERR;
    }
    while(cpu_test_and_set(&lock->lock) == 1);
    lock->pid = get_pid();
    lock->nest_level = 0;
#endif

    return OS_NO_ERR;
}

OS_RETURN_E spinlock_unlock(lock_t* lock)
{

    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#ifdef KERNEL_MONOCORE_SYNC
    enable_local_interrupt();
#else
    if(lock->nest_level > 1)
    {
        --lock->nest_level;
    }
    else
    {
        lock->nest_level = 0;
        lock->pid = -1;
        lock->lock = 0;
    }
#endif

    return OS_NO_ERR;
}

OS_RETURN_E spinlock_init(lock_t* lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    lock->nest_level = 0;
    lock->pid = -1;
    lock->lock = 0;
    return OS_NO_ERR;
}
