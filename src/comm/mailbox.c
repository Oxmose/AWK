/*******************************************************************************
 *
 * File: mailbox.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 16/12/2017
 *
 * Version: 2.0
 *
 * Kernel mailboxes features
 ******************************************************************************/

#include "../lib/stddef.h"         /* OS_RETURN_E */
#include "../lib/stdint.h"         /* Generic int types */
#include "../lib/string.h"         /* memset */
#include "../core/kernel_list.h"   /* kernel_list_t kernel_list_node_t */
#include "../core/kernel_output.h" /* kernel_error */
#include "../core/panic.h"         /* kernel_panic */
#include "../core/scheduler.h"     /* lock_thread, unlock_thread */
#include "../sync/lock.h"          /* spinlocks */

#include "../debug.h"      /* kernel_serial_debug */

/* Header include */
#include "mailbox.h"

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E mailbox_init(mailbox_t* mailbox)
{
    OS_RETURN_E err;

    /* Pointer check */
    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the mailbox */
    memset(mailbox, 0, sizeof(mailbox_t));

    mailbox->state = 0;
    spinlock_init(&mailbox->lock);

    mailbox->read_waiting_threads = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    mailbox->write_waiting_threads = kernel_list_create_list(&err);
    if(err != OS_NO_ERR)
    {
        kernel_list_delete_list(&mailbox->read_waiting_threads);
        return err;
    }

    mailbox->init = 1;

    #ifdef DEBUG_MAILBOX
    kernel_serial_debug("Mailbox 0x%08x INIT\n", (uint32_t)mailbox);
    #endif

    return OS_NO_ERR;
}

void* mailbox_pend(mailbox_t* mailbox, OS_RETURN_E* error)
{
    OS_RETURN_E         err;
    void*               ret_val;
    kernel_list_node_t* node;

    #ifdef DEBUG_MAILBOX
    kernel_serial_debug("Mailbox 0x%08x PEND\n", (uint32_t)mailbox);
    #endif

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

    /* If the mailbox is empty block thread */
    while(mailbox->init == 1 &&
          mailbox->state == 0)
    {
        node = lock_thread(QUEUE);
        if(node == NULL)
        {
            kernel_error("Could not lock this thread to mailbox[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic();
        }

        /* Adding the thread to the blocked set of reading threads */
        err = kernel_list_enlist_data(node,
                                      mailbox->read_waiting_threads,
                                      0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to mailbox[%d]\n", err);
            kernel_panic();
        }

        spinlock_unlock(&mailbox->lock);
        schedule();
        spinlock_lock(&mailbox->lock);
    }

    if(mailbox->init != 1)
    {
        spinlock_unlock(&mailbox->lock);

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

    /* Get mailbox value */
    ret_val = mailbox->value;

    /* Manage mailbox state */
    mailbox->state = 0;

    /* Check if we can wake up a thread */
    node = kernel_list_delist_data(mailbox->write_waiting_threads, &err);

    spinlock_unlock(&mailbox->lock);

    if(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }
    }
    else if (err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    #ifdef DEBUG_MAILBOX
    kernel_serial_debug("Mailbox 0x%08x ACQUIRED\n", (uint32_t)mailbox);
    #endif

    return ret_val;
}

OS_RETURN_E mailbox_post(mailbox_t* mailbox, void* element)
{
    OS_RETURN_E      err;
    kernel_list_node_t* node;

    #ifdef DEBUG_MAILBOX
    kernel_serial_debug("Mailbox 0x%08x POST\n", (uint32_t)mailbox);
    #endif

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
    while(mailbox->init == 1 &&
          mailbox->state == 1)
    {
        node = lock_thread(QUEUE);
        if(node == NULL)
        {
            kernel_error("Could not lock this thread to mailbox[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic();
        }

        /* Adding the thread to the blocked threads set. */
        err = kernel_list_enlist_data(node,
                                      mailbox->write_waiting_threads,
                                      0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue thread to mailbox[%d]\n", err);
            kernel_panic();
        }

        spinlock_unlock(&mailbox->lock);
        schedule();
        spinlock_lock(&mailbox->lock);
    }

    if(mailbox->init != 1)
    {
        spinlock_unlock(&mailbox->lock);

        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    /* Set value of the mailbox */
    mailbox->value = element;

    /* Manage mailbox state */
    mailbox->state = 1;

    /* Check if we can wake up a thread */
    node = kernel_list_delist_data(mailbox->read_waiting_threads, &err);
    spinlock_unlock(&mailbox->lock);
    if(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 1);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }
    }
    else if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }

    return OS_NO_ERR;
}

OS_RETURN_E mailbox_destroy(mailbox_t* mailbox)
{
    OS_RETURN_E         err;
    kernel_list_node_t* node;

    #ifdef DEBUG_MAILBOX
    kernel_serial_debug("Mailbox 0x%08x DESTROY\n", (uint32_t)mailbox);
    #endif

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

    /* Check if we can wake up a thread */
    node = kernel_list_delist_data(mailbox->read_waiting_threads, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }
        node = kernel_list_delist_data(mailbox->read_waiting_threads, &err);
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }

    node = kernel_list_delist_data(mailbox->write_waiting_threads, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        err = unlock_thread(node, QUEUE, 0);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not unlock thread from mailbox[%d]\n", err);
            kernel_panic();
        }
        node = kernel_list_delist_data(mailbox->write_waiting_threads, &err);
    }
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue thread from mailbox[%d]\n", err);
        kernel_panic();
    }

    /* Delete lists */
    err = kernel_list_delete_list(&mailbox->read_waiting_threads);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not delete list from mailbox[%d]\n", err);
        kernel_panic();
    }
    err = kernel_list_delete_list(&mailbox->write_waiting_threads);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not delete list from mailbox[%d]\n", err);
        kernel_panic();
    }

    /* Set the mailbox to a destroyed state */
    mailbox->init  = 0;
    mailbox->state = 0;

    spinlock_unlock(&mailbox->lock);

    return OS_NO_ERR;
}

int8_t mailbox_isempty(mailbox_t* mailbox, OS_RETURN_E* error)
{
    int8_t ret;

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

    ret = (mailbox->state == 0);

    spinlock_unlock(&mailbox->lock);

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return ret;
}
