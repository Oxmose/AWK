/*******************************************************************************
 *
 * File: mailbox.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 1.0
 *
 * Kernel mailboxes features
 *
 ******************************************************************************/

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../core/kernel_queue.h"  /* thread_queue_t */
#include "../core/kernel_output.h" /* kernel_error */
#include "../core/panic.h"         /* kernel_panic */
#include "../core/scheduler.h"     /* lock_thread, unlock_thread */
#include "../sync/lock.h"          /* lock_t */

/* Header include */
#include "mailbox.h"

OS_RETURN_E mailbox_init(mailbox_t *mailbox)
{
    /* Pointer check */
    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the mailbox */
    mailbox->state = 0;
    spinlock_init(&mailbox->lock);

    mailbox->read_waiting_threads[0]  = NULL;
    mailbox->read_waiting_threads[1]  = NULL;
    mailbox->write_waiting_threads[0] = NULL;
    mailbox->write_waiting_threads[1] = NULL; 

    mailbox->init  = 1;

    return OS_NO_ERR;
}

void* mailbox_pend(mailbox_t *mailbox, OS_RETURN_E *error)
{
  /* Check mailbox pointer */
  if(mailbox == NULL)
  {
    if(error != NULL)
    {
      *error = OS_ERR_NULL_POINTER;
    }

    return NULL;
  }

  spinlock_lock(&mailbox->lock);

  /* Check for mailbox initialization */
  if(mailbox->init != 1)
  {
    spinlock_unlock(&mailbox->lock);

    if(error != NULL)
    {
      *error = OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    return NULL;
  }

  OS_RETURN_E err;

  /* If the mailbox is empty block thread */
  while(mailbox->state == 0)
  {
    
    /* Adding the thread to the blocked set of reading threads */
    err = kernel_enqueue_thread(get_active_thread(), mailbox->read_waiting_threads, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue thread to mailbox[%d]\n", err);
        kernel_panic();
    }

    spinlock_unlock(&mailbox->lock);

    /* Scheduling the thread */
    err = lock_thread(QUEUE);

    if(err != OS_NO_ERR)
    {
        kernel_error("Could not lock thread to mailbox[%d]\n", err);
        kernel_panic();
    }

    spinlock_lock(&mailbox->lock);
  }

  /* Get mailbox value */
  void *ret_val = mailbox->value;

  /* Manage mailbox state */
  mailbox->state = 0;

  /* Check if we can wake up a thread */
    kernel_thread_t *thread;
    if((thread = kernel_dequeue_thread(mailbox->write_waiting_threads, &err)) 
        != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
            kernel_panic();
        }
        spinlock_unlock(&mailbox->lock);

        err = unlock_thread(thread, QUEUE, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }

        return OS_NO_ERR;
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }

  spinlock_unlock(&mailbox->lock);

  if(error != NULL)
  {
    *error = OS_NO_ERR;
  }

  return ret_val;
}

OS_RETURN_E mailbox_post(mailbox_t *mailbox, void *element)
{
  if(mailbox == NULL)
  {
    return OS_ERR_NULL_POINTER;
  }

  spinlock_lock(&mailbox->lock);

  if(mailbox->init != 1)
  {
    spinlock_unlock(&mailbox->lock);

    return OS_ERR_MAILBOX_NON_INITIALIZED;
  }

  /* If the mailbox is full then posting block thread */
  while(mailbox->state == 1)
  {
    OS_RETURN_E err;
    /* Adding the thread to the blocked threads set. */
    err = kernel_enqueue_thread(get_active_thread(), 
                         mailbox->write_waiting_threads, 0);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not enqueue thread to mailbox[%d]\n", err);
        kernel_panic();
    }

    spinlock_unlock(&mailbox->lock);

    err = lock_thread(QUEUE);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not lock thread to mailbox[%d]\n", err);
        kernel_panic();
    }

    spinlock_lock(&mailbox->lock);
  }

  /* Set value of the mailbox */
  mailbox->value = element;

  /* Manage mailbox state */
  mailbox->state = 1;

  /* Check if we can wake up a thread */
    kernel_thread_t *thread;
    OS_RETURN_E err;
    if((thread = kernel_dequeue_thread(mailbox->read_waiting_threads, &err)) 
      != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
            kernel_panic();
        }

      spinlock_unlock(&mailbox->lock);

      err = unlock_thread(thread, QUEUE, 1);
      if(err != OS_NO_ERR)
      {
        kernel_error("Could not unlock thread from mailbox[%d]\n", err);
        kernel_panic();
      }

      return OS_NO_ERR;
    }
    if(err != OS_NO_ERR)
    {
      kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
      kernel_panic();
    }

  spinlock_unlock(&mailbox->lock);

  return OS_NO_ERR;
}

OS_RETURN_E mailbox_destroy(mailbox_t *mailbox)
{
  if(mailbox == NULL)
  {
    return OS_ERR_NULL_POINTER;
  }

  spinlock_lock(&mailbox->lock);

  if(mailbox->init != 1)
  {
    spinlock_unlock(&mailbox->lock);

    return OS_ERR_MAILBOX_NON_INITIALIZED;
  }

  /* Set the mailbox to a destroyed state */
  mailbox->init  = 0;
  mailbox->state = 0;

  /* Check if we can wake up a thread */
    kernel_thread_t *thread;
    OS_RETURN_E err;
    while((thread = kernel_dequeue_thread(mailbox->read_waiting_threads, &err)) 
      != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
            kernel_panic();
        }

        err = unlock_thread(thread, QUEUE, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }
    while((thread = kernel_dequeue_thread(mailbox->write_waiting_threads, &err))
      != NULL)
    {
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
            kernel_panic();
        }

        err = unlock_thread(thread, QUEUE, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }

  spinlock_unlock(&mailbox->lock);

  return OS_NO_ERR;
}

int8_t mailbox_isempty(mailbox_t *mailbox, OS_RETURN_E *error)
{
  if(mailbox == NULL)
  {
    if(error != NULL)
    {
      *error = OS_ERR_NULL_POINTER;
    }
    return -1;
  }

  spinlock_lock(&mailbox->lock);

  if(mailbox->init != 1)
  {
    spinlock_unlock(&mailbox->lock);
  
    if(error != NULL)
    {
      *error = OS_ERR_MAILBOX_NON_INITIALIZED;
    }
    
    return -1;
  }

  int8_t ret = (mailbox->state == 0);

  spinlock_unlock(&mailbox->lock);

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

  return ret;
}