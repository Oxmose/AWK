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

#ifndef __LOCK_H_
#define __LOCK_H_

#include "../lib/stdint.h" /* Generic int types */
#include "../lib/stddef.h" /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define KERNEL_MONOCORE 1

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef volatile uint32_t lock_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Lock the lock given as parameter
 *
 * @param The lock to lock.
 */
OS_RETURN_E spinlock_lock(lock_t* lock);

/* Unlock the lock given as parameter
 *
 * @param The lock to unlock.
 */
OS_RETURN_E spinlock_unlock(lock_t* lock);

/* Init the lock given as parameter
 *
 * @param The lock to init.
 */
OS_RETURN_E spinlock_init(lock_t* lock);

#endif /* __LOCK_H_ */