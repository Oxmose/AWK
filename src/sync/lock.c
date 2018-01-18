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
#include "../core/interrupts.h" /* enable_interrupt, disable_interrupt */

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
    disable_interrupt();
#endif
    while(cpu_test_and_set(lock) == 1);
    
    return OS_NO_ERR;
}

OS_RETURN_E spinlock_unlock(lock_t* lock)
{

    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    *lock = 0;

#ifdef KERNEL_MONOCORE_SYNC
    enable_interrupt();
#endif

    return OS_NO_ERR;
}

OS_RETURN_E spinlock_init(lock_t* lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    *lock = 0;
    return OS_NO_ERR;
}
