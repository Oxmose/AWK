/*******************************************************************************
 *
 * File: lock.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Basic lock and synchronization primitives
 ******************************************************************************/

#include "../cpu/cpu.h"    /* cpu_test_and_set */
#include "../lib/stddef.h" /* OS_RETURN_E */

/* Header file */
#include "lock.h"

OS_RETURN_E spinlock_lock(lock_t *lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    while(cpu_test_and_set(lock) == 1);
    
    return OS_NO_ERR;
}

OS_RETURN_E spinlock_unlock(lock_t *lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    *lock = 0;

    return OS_NO_ERR;
}

OS_RETURN_E spinlock_init(lock_t *lock)
{
    return spinlock_unlock(lock);    
}