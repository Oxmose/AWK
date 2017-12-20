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

#include "../cpu/cpu.h"    /* sti, cli, cpu_test_and_set */
#include "../lib/stddef.h" /* OS_RETURN_E */

/* Header file */
#include "lock.h"

/* Keep track on the nexting level, kernel start with interrupt disabled */
static uint32_t int_lock_nesting = 1;

void enable_interrupt(void)
{
    if(int_lock_nesting > 0)
    {
        --int_lock_nesting;
    }
    if(int_lock_nesting == 0)
    {
        sti();
    }
}

void disable_interrupt(void)
{
    cli();
    ++int_lock_nesting;
}

OS_RETURN_E spinlock_lock(lock_t *lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if KERNEL_MONOCORE
    disable_interrupt();
    *lock = 1;
#else
    while(cpu_test_and_set(lock) == 1);
#endif
    
    return OS_NO_ERR;
}

OS_RETURN_E spinlock_unlock(lock_t *lock)
{
    if(lock == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

#if KERNEL_MONOCORE
    *lock = 0;
    enable_interrupt();
#else
    *lock = 0;
#endif

    return OS_NO_ERR;
}

OS_RETURN_E spinlock_init(lock_t *lock)
{
#if KERNEL_MONOCORE
    *lock = 0;
#else
    return spinlock_unlock(lock);
#endif
    
}