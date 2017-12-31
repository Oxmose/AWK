/*******************************************************************************
 *
 * File: queue.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 2.0
 *
 * Kernel queues feature, allows to define custom sized queues.
 *
 ******************************************************************************/

#ifndef __QUEUE_H_
#define __QUEUE_H_

#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../lib/stdint.h"        /* Generic int types */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "../sync/lock.h"         /* lock_t */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/* Queue structure */
typedef struct queue
{
    lock_t lock;          /* Structure lock */

    uint32_t head;        /* Queue item list head */
    uint32_t tail;        /* Queue item list tail */

    uint32_t max_length;  /* Maximum length of the queue */
    uint32_t length;      /* Number of elements contained in the queue */

    void** container;     /* Queue's list */

    int8_t init;          /* Queue init sate */

    /***********************************
     * THREAD TABLE | 0: Head, 1: Tail
     *
     * FIFO fashioned
     **********************************/
    thread_queue_t* read_waiting_threads[2];
    thread_queue_t* write_waiting_threads[2];
} queue_t;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Initialize the queue given as parameter. The function will set the queue
 * structure and init the queue as empty. See system returns type for error
 * handling.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The queue pointer given as parameter is NULL.
 * OS_ERR_MALLOC: The memory allocation for the queue failed.
 *
 * @param queue A pointer to the queue to initialize. If NULL, the function
 * will immediatly return with the according error code.
 * @param length The depth of the queue.
 *
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E queue_init(queue_t *queue, const uint32_t length);

/* Pend on the queue given as parameter. This function will block the calling
 * thread if the queue is empty. See system returns type for error
 * handling.
 * The function will return NULL in case of error and the error pointer given as
 * parameter will be set accordingly. The queue item might be a NULL pointer.
 * In this case, error will be set to OS_NO_ERR if no error is detected.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The queue pointer given as parameter is NULL.
 * OS_ERR_QUEUE_NON_INITIALIZED: The queue has not been initialized before.
 *
 * @param queue A pointer to the queue to initialize. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 *
 * @returns The function returns the content of the queue if error is set to
 * OS_NO_ERR, NULL otherwise.
 */
void* queue_pend(queue_t *queue, OS_RETURN_E *error);


/* Post on the queue given as parameter. This function will block the calling
 * thread if the queue is full. See system returns type for error
 * handling.
 * The function will return the error code in case of error
 * The queue item might be a NULL pointer.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The queue pointer given as parameter is NULL.
 * OS_ERR_QUEUE_NON_INITIALIZED: The queue has not been initialized before.
 *
 * @param queue A pointer to the queue to post. If NULL, the function
 * will immediatly return with the according error code.
 * @param element A pointer to the element to store in the queue. Only the
 * pointer is stored in the queue, meaning the content of the pointed address
 * might change.
 *
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E queue_post(queue_t *queue, void *element);

/* Destroy the queue given as parameter. The function will set the queue
 * structure to uninitialized and destroy the queue. See system returns type
 * for error handling.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The queue pointer given as parameter is NULL.
 * OS_ERR_QUEUE_NON_INITIALIZED: The queue has not been initialized before.
 *
 * @param queue A pointer to the queue to destroy. If NULL, the function
 * will immediatly return with the according error code.
 *
 * @returns The function returns OS_NO_ERR on success, see system returns type
 * for further error description.
 */
OS_RETURN_E queue_destroy(queue_t *queue);

/* Give the queue emptyness status.
 *
 * The function will return -1 in case of error and the error pointer given as
 * parameter will be set accordingly. Returns values are [size of queue] if the
 * queue is not empty, 0 otherwise.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The queue pointer given as parameter is NULL or the
 * error parameter is NULL.
 * OS_ERR_QUEUE_NON_INITIALIZED: The queue has not been initialized before.
 *
 * @param queue A pointer to the queue to test. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 *
 * @returns The function returns -1 on error, 1 if the queue is empty and 0
 * otherwise.
 */
int32_t queue_length(queue_t *queue, OS_RETURN_E *error);

/* Give the queue emptyness status.
 *
 * The function will return -1 in case of error and the error pointer given as
 * parameter will be set accordingly. Returns values are 1 if the queue is
 * empty and 0 otherwise.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The queue pointer given as parameter is NULL or the
 * error parameter is NULL.
 * OS_ERR_QUEUE_NON_INITIALIZED: The queue has not been initialized before.
 *
 * @param queue A pointer to the queue to test. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param error A pointer to the variable that contains the function success
 * state. May be NULL.
 *
 * @returns The function returns -1 on error, 1 if the queue is empty and 0
 * otherwise.
 */
int8_t queue_isempty(queue_t *queue, OS_RETURN_E *error);

#endif /* __QUEUE_H_ */