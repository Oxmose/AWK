/*******************************************************************************
 *
 * File: semaphore.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 2.0
 *
 * Semaphore synchronization primitive implemantation.
 ******************************************************************************/

#ifndef __SEMAPHORE_H_
#define __SEMAPHORE_H_

#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../lib/stdint.h"        /* Generic int types */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "lock.h"                  /* lock_t */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct semaphore
{
    /*******************************************************
     * THREAD TABLE
     * Sorted by priority:
     *     - FIFO
     *
     * Index 0 is the head, 1 is the tail
     *******************************************************/
    thread_queue_t* waiting_threads[2];

    /* Semaphore counter */
    int32_t sem_level;

    /* Spinlock to ensure atomic access to the semaphore */
    lock_t lock;

    /* Init state */
    int8_t init;
} semaphore_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Initialize the semaphore structure.
 * The initial state of a semaphore is given by the init_level parameter.
 *
 * @param sem The pointer to the semaphore to initialize.
 * @param init_level The initial value to set the semaphore with.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E sem_init(semaphore_t* sem, const int32_t init_level);

/* Destroy the semaphore given as parameter. Also unlock all the threads locked
 * on this semaphore.
 *
 * @param sem The semaphore to destroy.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E sem_destroy(semaphore_t* sem);

/* Pend the semaphore given as parameter.
 *
 * @param sem The semaphore to pend.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E sem_pend(semaphore_t* sem);

/* Post the semaphore given as parameter.
 *
 * @param sem The semaphore to post.
 * @returns OS_NO_ERR on success, otherwise an error is returned.
 */
OS_RETURN_E sem_post(semaphore_t* sem);

/* Try to pend the semaphore given as parameter.
 *
 * @param sem The semaphore to pend.
 * @param value The buffer that receive the semaphore state.
 * @returns OS_NO_ERR on success, OS_SEM_LOCKED if the semaphore is locked,
 * otherwise an error is returned.
 */
OS_RETURN_E sem_try_pend(semaphore_t* sem, int8_t* value);

#endif /* __SEMAPHORE_H_ */