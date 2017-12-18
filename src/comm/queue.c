/***************************************
 *
 * File: queue.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 15/10/2017
 *
 * Version: 1.0
 *
 * See: queue.h
 *
 * Kernel queues, allow to define custom sized queues.
 * This file may be modified as the kernel might get some dynamic memory 
 * allocator. At the moment the custom size i totally artificial and all the 
 * queues take a lot of space in memory.
 */

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../core/kernel_queue.h"  /* thread_queue_t */
#include "../core/kernel_output.h" /* kernel_error */
#include "../core/panic.h"         /* kernel_panic */
#include "../core/scheduler.h"     /* lock_thread, unlock_thread */
#include "../sync/lock.h"          /* lock_t */
#include "../lib/malloc.h"		   /* malloc, free */

/* Header include */
#include "queue.h"

OS_RETURN_E queue_init(queue_t *queue, const uint32_t length)
{
	if(queue == NULL)
	{
		return OS_ERR_NULL_POINTER;
	}

	/* Init the queue */
	queue->head = 0;
	queue->tail = 0;
	spinlock_init(&queue->lock);

	queue->max_length = length;
	queue->length     = 0;

	queue->container = malloc(sizeof(void*) * length);
	if(queue->container == NULL)
	{
		return OS_ERR_MALLOC;
	}

	queue->read_waiting_threads[0]  = NULL;
    queue->read_waiting_threads[1]  = NULL;
    queue->write_waiting_threads[0] = NULL;
    queue->write_waiting_threads[1] = NULL;

	queue->init = 1;

	return OS_NO_ERR;
}

void* queue_pend(queue_t *queue, OS_RETURN_E *error)
{
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

	OS_RETURN_E err;

	/* If the queue is empty block thread */
	while(queue->length == 0)
	{
		/* Adding the thread to the blocked set of reading threads */
	    err = kernel_enqueue_thread(get_active_thread(), 
	    	                        queue->read_waiting_threads, 0);

	    if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not enqueue thread to queue[%d]\n", err);
        	kernel_panic();
	    }

	    spinlock_unlock(&queue->lock);

	    /* Scheduling the thread */
	    err = lock_thread(QUEUE);
	    if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not lock thread to queue[%d]\n", err);
        	kernel_panic();
	    }

	    spinlock_lock(&queue->lock);
	}

	/* Get value */
	void *ret_val = queue->container[queue->tail];

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
	
	kernel_thread_t *thread;
    if((thread = kernel_dequeue_thread(queue->write_waiting_threads, &err)) 
    	!= NULL)
    {
		if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not dequeue thread from queue[%d]\n", err);
	    	kernel_panic();
	    }
      	spinlock_unlock(&queue->lock);

      	err = unlock_thread(thread, QUEUE, 1);

      	if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not unlock thread from queue[%d]\n", err);
	    	kernel_panic();
	    }

      	return OS_NO_ERR;
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
    	kernel_panic();
    }
	
	spinlock_unlock(&queue->lock);

	if(error != NULL)
	{
		*error = OS_NO_ERR;
	}

	return ret_val;
}

OS_RETURN_E queue_post(queue_t *queue, void *element)
{
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

	OS_RETURN_E err;

	/* If the queue is full block thread */
	while(queue->length == queue->max_length)
	{
		/* Adding the thread to the blocked threads set. */
	    err = kernel_enqueue_thread(get_active_thread(), 
	    	                        queue->write_waiting_threads, 0);
	    
	    if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not enqueue thread to queue[%d]\n", err);
        	kernel_panic();
	    }

	    spinlock_unlock(&queue->lock);

	    err = lock_thread(QUEUE);
	    if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not lock thread to queue[%d]\n", err);
        	kernel_panic();
	    }

	    spinlock_lock(&queue->lock);
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
	kernel_thread_t *thread;
	if((thread = kernel_dequeue_thread(queue->read_waiting_threads, &err)) 
	   != NULL)
	{
	    if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not dequeue thread from queue[%d]\n", err);
	    	kernel_panic();
	    }
	    
	    spinlock_unlock(&queue->lock);

	    err = unlock_thread(thread, QUEUE, 1);
	    if(err != OS_NO_ERR)
    	{
        	kernel_error("Could not unlock thread from queue[%d]\n", err);
    		kernel_panic();
    	}

		return OS_NO_ERR;
	}
	if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
    	kernel_panic();
    }

	spinlock_unlock(&queue->lock);

	return OS_NO_ERR;
}

OS_RETURN_E queue_destroy(queue_t *queue)
{
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

	queue->init = 0;
	queue->head = 0;
	queue->tail = 0;

	queue->max_length = 0;

	queue->length     = 0;
	free(queue->container);

	/* Check if we can wake up threads */

	kernel_thread_t *thread;
    OS_RETURN_E err;
    while((thread = kernel_dequeue_thread(queue->read_waiting_threads, &err))
          != NULL)
    {
    	if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not dequeue thread from queue[%d]\n", err);
	    	kernel_panic();
	    }
        err = unlock_thread(thread, QUEUE, 0);
        if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not unlock thread from queue[%d]\n", err);
	    	kernel_panic();
	    }
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
    	kernel_panic();
    }
    while((thread = kernel_dequeue_thread(queue->write_waiting_threads, &err))
     != NULL)
    {
    	if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not dequeue thread from queue[%d]\n", err);
	    	kernel_panic();
	    }
        err = unlock_thread(thread, QUEUE, 0);
        if(err != OS_NO_ERR)
	    {
	        kernel_error("Could not unlock thread from queue[%d]\n", err);
	    	kernel_panic();
	    }
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from queue[%d]\n", err);
    	kernel_panic();
    }

	spinlock_unlock(&queue->lock);

	return OS_NO_ERR;
}

int32_t queue_length(queue_t *queue, OS_RETURN_E *error)
{
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
		if(error != NULL)
		{
			*error = OS_ERR_QUEUE_NON_INITIALIZED;
		}

		spinlock_unlock(&queue->lock);

		return -1;
	}

	int32_t queue_size = queue->length;

	spinlock_unlock(&queue->lock);

	if(error != NULL)
	{
		*error = OS_NO_ERR;
	}

	return queue_size;
}

int8_t queue_isempty(queue_t *queue, OS_RETURN_E *error)
{
	int32_t size = queue_length(queue, error);
	if(size == -1)
		return -1;

	return (size == 0);
}