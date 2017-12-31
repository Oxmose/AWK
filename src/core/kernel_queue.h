/*******************************************************************************
 *
 * File: kernel_queues.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Kernel priority queues used to manage threads.
 * WARNING These queues are not thread safe!
 ******************************************************************************/

#ifndef __KERNEL_QUEUE_H_
#define __KERNEL_QUEUE_H_

#include "kernel_thread.h" /* kernel_thread_t */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/stdint.h" /* Generic int types */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* Thread queue structure */
typedef struct thread_queue
{
    struct thread_queue* next; /* Next node of the queue */
    struct thread_queue* prev; /* Previous node of the queue */

    uint32_t priority;         /* Priority of the element */

    kernel_thread_t* thread;   /* Thread contained by the node */
} thread_queue_t;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Enqueue a thread in the queue given as parameter. The thread will be placed
 * in the queue accordinlgy to the priority defined.
 *
 * @param thread A pointer to the thread to place in the queue.
 * @param queue The queue to manage.
 * @param priority The priority of the element to place in the queue, could be
 * something else than the thread priority.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_enqueue_thread(kernel_thread_t* thread,
                                  thread_queue_t* queue[2],
                                  const uint32_t priority);

/* Dequeue a thread pointer from the queue given as parameter. The thread
 * returned is the one with the highest priority parameter. Note that this
 * priority could be something else than the thread priority, as it is defined
 * while enqueuing the thread.
 *
 * @param queue The queue to manage.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 * @returns The thread poitner with the highest priority parameter is returned.
 */
kernel_thread_t* kernel_dequeue_thread(thread_queue_t* queue[2],
                                       OS_RETURN_E* error);

/* Rmove a thread pointer from the queue given as parameter.
 *
 * @param queue The queue to manage.
 * @param thread A pointer to the thread to remove from the queue.
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E kernel_remove_thread(thread_queue_t* queue[2],
                                 kernel_thread_t* thread);

#endif /* __KERNEL_QUEUE_H_ */
