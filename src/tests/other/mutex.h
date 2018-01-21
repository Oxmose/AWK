/*******************************************************************************
 *
 * File: mutex.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 2.0
 *
 * Mutex synchronization primitive implemantation.
 ******************************************************************************/

#ifndef __MUTEX_H_
#define __MUTEX_H_

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../core/kernel_list.h"   /* kernel_list_t, kernel_list_node_t */
#include "lock.h"                  /* lock_t */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define MUTEX_FLAG_NONE      0x00000000
#define MUTEX_FLAG_RECURSIVE 0x00000001

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct mutex
{
    /*******************************************************
     * THREAD TABLE
     * Sorted by priority:
     *     - FIFO
     *******************************************************/
    kernel_list_t* waiting_threads;

    /* Mutex lock state */
    volatile uint8_t state;

    /* FLAGS
     *     [0] = RECURSIVE
     */
    uint32_t flags;

    /* PID of the thread that acquired the lock */
    int32_t locker_pid;

    /* Spinlock to ensure atomic access to the mutex */
    lock_t lock;

    /* Init state */
    int8_t init;
} mutex_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Initialize the mutex structure.
 * The initial state of a mutex is unlocked.
 *
 * @param mutex The pointer to the mutex to initialize.
 * @param flags Mutex flags, see defines to get all the possible mutex flags.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E mutex_init(mutex_t* mutex, const uint32_t flags);

/* Destroy the mutex given as parameter. Also unlock all the threads locked on
 * this mutex.
 *
 * @param mutex The mustex to destroy.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E mutex_destroy(mutex_t* mutex);

/* Pend the mutex given as parameter.
 *
 * @param mutex The mustex to pend.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E mutex_pend(mutex_t* mutex);

/* Post the mutex given as parameter.
 *
 * @param mutex The mustex to post.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E mutex_post(mutex_t* mutex);

/* Try to pend the mutex given as parameter.
 *
 * @param mutex The mustex to pend.
 * @param value The buffer that receive the mutex state.
 * @returns OS_NO_ERR on success, OS_MUTEX_LOCKED if the mutex is locked,
 * otherwise an error is returned.
 */
OS_RETURN_E mutex_try_pend(mutex_t* mutex, int8_t* value);

#endif /* __MUTEX_H_ */
