/*******************************************************************************
 *
 * File: queue.c
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

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/string.h"         /* memset */
#include "../core/kernel_list.h"   /* kernel_list_t kernel_list_node_t */
#include "../core/kernel_output.h" /* kernel_error */
#include "../core/panic.h"         /* kernel_panic */
#include "../core/scheduler.h"     /* lock_thread, unlock_thread */
#include "../sync/lock.h"          /* lock_t */
#include "../memory/heap.h"        /* kmalloc, kfree */

#include "../debug.h"      /* kernel_serial_debug */

/* Header include */
#include "queue.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E queue_init(queue_t* queue, const uint32_t length)
{
    OS_RETURN_E err;

    if(queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the queue */
    memset(queue, 0, sizeof(queue_t));

    queue->container = kmalloc(sizeof(void*) * length);
    if(queue->container == NULL)
    {
        return OS_ERR_MALLOC;
    }

    spinlock_init(&queue->lock);
    queue->head = 0;
    queue->tail = 0;

    queue->max_length = length;
    queue->length     = 0;

    queue->read_waiting_threads = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    queue->write_waiting_threads = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_list(&queue->read_waiting_threads);
        return err;
    }

    queue->init = 1;

    #ifdef DEBUG_QUEUE
    kernel_serial_debug("Queue 0x%08x INIT\n", (uint32_t)queue);
    #endif

    return OS_NO_ERR;
}

void* queue_pend(queue_t* queue, OS_RETURN_E* error)
{
    OS_RETURN_E      err;
    void*            ret_val;
    kernel_list_node_t* node;

    #ifdef DEBUG_QUEUE
    kernel_serial_debug("Queue 0x%08x PEND\n", (uint32_t)queue);
    #endif

    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return NULL;
    }

    spinlock_lock(&queue->lock);

    if(queue->init != 1)
    {
        spinlock_unlock(&queue->lock);

        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }

        return NULL;
    }

    /* If the queue is empty block thread */
    while(queue->init == 1 &&
          queue->length == 0)
    {
        /* Adding the thread to the blocked set of reading threads */
        node = lock_thread(QUEUE);
        if(node == NULL)
        {
            kernel_error("Could not lock this thread to queue[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic();
        }

        err = kernel_list_enlist_data(node,
                                      queue->read_waiting_threads,
                                      0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to queue[%d]\n", err);
            kernel_panic();
        }

        spinlock_unlock(&queue->lock);
        schedule();
        spinlock_lock(&queue->lock);
    }

    if(queue->init != 1)
    {
        spinlock_unlock(&queue->lock);

        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }

        return NULL;
    }

    /* Get value */
    ret_val = queue->container[queue->tail];

    /* Manage index */
    if(queue->tail >= queue->max_length)
    {
        queue->tail = 0;
    }
    else
    {
        ++queue->tail;
    }
    --queue->length;

    /* Check if we can wake up a thread */
    node = kernel_list_delist_data(queue->write_waiting_threads, &err);

    spinlock_unlock(&queue->lock);

    if(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from queue[%d]\n", err);
            kernel_panic();
        }
    }
    else if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
        kernel_panic();
    }


    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    #ifdef DEBUG_QUEUE
    kernel_serial_debug("Queue 0x%08x ACQUIRED\n", (uint32_t)queue);
    #endif

    return ret_val;
}

OS_RETURN_E queue_post(queue_t* queue, void* element)
{
    OS_RETURN_E         err;
    kernel_list_node_t* node;

    #ifdef DEBUG_QUEUE
    kernel_serial_debug("Queue 0x%08x POST\n", (uint32_t)queue);
    #endif

    if(queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    spinlock_lock(&queue->lock);

    if(queue->init != 1)
    {
        spinlock_unlock(&queue->lock);

        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    /* If the queue is full block thread */
    while(queue->init == 1 &&
          queue->length == queue->max_length)
    {
        node = lock_thread(QUEUE);
        if(node == NULL)
        {
            kernel_error("Could not lock this thread to queue[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic();
        }

        /* Adding the thread to the blocked threads set. */
        err = kernel_list_enlist_data(node,
                                      queue->write_waiting_threads,
                                      0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to queue[%d]\n", err);
            kernel_panic();
        }

        spinlock_unlock(&queue->lock);
        schedule();
        spinlock_lock(&queue->lock);
    }

    if(queue->init != 1)
    {
        spinlock_unlock(&queue->lock);

        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    /* Set value */
    queue->container[queue->head] = element;

    /* Manage index */
    if(queue->head >= queue->max_length)
    {
        queue->head = 0;
    }
    else
    {
        ++queue->head;
    }
    ++queue->length;

    /* Check if we can wake up a thread */
    node = kernel_list_delist_data(queue->read_waiting_threads, &err);
    spinlock_unlock(&queue->lock);
    if(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from queue[%d]\n", err);
            kernel_panic();
        }
    }
    else if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
        kernel_panic();
    }

    return OS_NO_ERR;
}

OS_RETURN_E queue_destroy(queue_t* queue)
{
    OS_RETURN_E         err;
    kernel_list_node_t* node;

    #ifdef DEBUG_QUEUE
    kernel_serial_debug("Queue 0x%08x DESTROY\n", (uint32_t)queue);
    #endif

    if(queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    spinlock_lock(&queue->lock);

    if(queue->init != 1)
    {
        spinlock_unlock(&queue->lock);

        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    /* Check if we can wake up threads */
    node = kernel_list_delist_data(queue->read_waiting_threads, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from queue[%d]\n", err);
            kernel_panic();
        }
        node = kernel_list_delist_data(queue->read_waiting_threads, &err);
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
        kernel_panic();
    }

    node = kernel_list_delist_data(queue->write_waiting_threads, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from queue[%d]\n", err);
            kernel_panic();
        }
        node = kernel_list_delist_data(queue->write_waiting_threads, &err);
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
        kernel_panic();
    }

    /* Delete lists */
    err = kernel_list_delete_list(&queue->read_waiting_threads);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not delete list from queue[%d]\n", err);
        kernel_panic();
    }
    err = kernel_list_delete_list(&queue->write_waiting_threads);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not delete list from queue[%d]\n", err);
        kernel_panic();
    }

    queue->init = 0;
    queue->head = 0;
    queue->tail = 0;

    queue->max_length = 0;
    queue->length     = 0;

    kfree(queue->container);

    spinlock_unlock(&queue->lock);

    return OS_NO_ERR;
}

int32_t queue_length(queue_t* queue, OS_RETURN_E* error)
{
    int32_t     queue_size;

    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return -1;
    }

    spinlock_lock(&queue->lock);

    if(queue->init != 1)
    {
        spinlock_unlock(&queue->lock);

        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }

        return -1;
    }

    queue_size = queue->length;

    spinlock_unlock(&queue->lock);

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return queue_size;
}

int8_t queue_isempty(queue_t* queue, OS_RETURN_E* error)
{
    int32_t size = queue_length(queue, error);
    if(size == -1)
    {
        return -1;
    }

    return (size == 0);
}
