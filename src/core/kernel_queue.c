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
 * Kernel queues used to manage threads.
 ******************************************************************************/

#include "kernel_thread.h" /* kernel_thread_t */
#include "../lib/stddef.h" /* OS_RETURN_E */
#include "../lib/stdint.h" /* Generic int types */
#include "../lib/malloc.h" /* malloc, free */

/* Header include */
#include "kernel_queue.h"

OS_RETURN_E kernel_enqueue_thread(kernel_thread_t *thread, 
                                  thread_queue_t *queue[2],
                                  const uint32_t priority)
{
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    thread_queue_t *node = malloc(sizeof(thread_queue_t));
    if(node == NULL)
    {
        return OS_ERR_MALLOC;
    }

    node->priority = priority;
    node->thread   = thread;


    /* If this queue is empty */
    if(queue[0] == NULL)
    {
        /* Set the first item */
        queue[0] = node;
        queue[1] = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        thread_queue_t *cursor = queue[0];
        while(cursor != NULL && cursor->priority > priority)
        {
            cursor = cursor->next;
        }

        if(cursor != NULL)
        {
            node->next = cursor;
            node->prev = cursor->prev;
            cursor->prev = node;
            if(node->prev != NULL)
            {
                node->prev->next = node;
            }
            else
            {
                queue[0] = node;
            }
        }
        else
        {
            /* Just put on the tail */
            node->prev = queue[1];
            node->next = NULL;
            queue[1]->next = node;
            queue[1] = node;
        }
    }
    return OS_NO_ERR;
}

kernel_thread_t* kernel_dequeue_thread(thread_queue_t *queue[2],  
                                       OS_RETURN_E *error)
{
    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }
    /* If this priority queue is empty */
    if(queue[0] == NULL)
    {
        return NULL;
    }

    /* Dequeue the last item */
    thread_queue_t *node = queue[1];
    if(node->prev != NULL)
    {
        node->prev->next = NULL;
        queue[1] = node->prev;
    }
    else
    {
        queue[0] = NULL;
        queue[1] = NULL;
    }

    kernel_thread_t *ret = node->thread;

    free(node);

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return ret;
}

OS_RETURN_E kernel_remove_thread(thread_queue_t *queue[2], 
                                 kernel_thread_t *thread)
{
    /* If this priority queue is empty */
    if(queue[0] == NULL)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Search for the thread */
    thread_queue_t *node = queue[0];
    while(node != NULL && node->thread != thread)
    {
        node = node->next;
    }

    /* No such thread */
    if(node == NULL)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Manage link */
    if(node->prev != NULL && node->next != NULL)
    {
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }
    else if(node->prev == NULL && node->next != NULL)
    {
        queue[0] = node->next;
        node->next->prev = NULL;
    }
    else if(node->prev != NULL && node->next == NULL)
    {
        queue[1] = node->prev;
        node->prev->next = NULL;
    }
    else
    {
        queue[0] = NULL;
        queue[1] = NULL;
    }

    free(node);

    return OS_NO_ERR;
}