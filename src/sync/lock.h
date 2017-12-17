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

#define KERNEL_MONOCORE 1

typedef volatile uint32_t lock_t;

/* Enable CPU interrupt */
void enable_interrupt(void);

/* Disable CPU interrupt */
void disable_interrupt(void);

/* Lock the lock given as parameter
 *
 * @param The lock to lock.
 */
OS_RETURN_E spinlock_lock(lock_t *lock);

/* Unlock the lock given as parameter
 *
 * @param The lock to unlock.
 */
OS_RETURN_E spinlock_unlock(lock_t *lock);

/* Init the lock given as parameter
 *
 * @param The lock to Init.
 */
OS_RETURN_E spinlock_init(lock_t *lock);

#endif /* __LOCK_H_ */