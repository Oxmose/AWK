/*******************************************************************************
 *
 * File: mailbox.h
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

#ifndef __MAILBOX_H_
#define __MAILBOX_H_

#include "../lib/stddef.h"        /* OS_RETURN_E */
#include "../lib/stdint.h"        /* Generic int types */
#include "../core/kernel_queue.h" /* thread_queue_t */
#include "../sync/lock.h"	      /* lock_t */

/*******************************
 * CONSTANTS
 ******************************/


/*******************************
 * STRUCTURES
 ******************************/

/* Mailbox structure */
typedef struct mailbox
{
	lock_t lock;    /* Structure lock */

	void *value;    /* Mailbox value */
	int8_t init;  	/* Mailbox initialization state */
	int8_t state;   /* Mailbox current state (0 = empy, 1 = full) */

	/**************************
	 * THREAD TABLE | 0: Head, 1: Tail
	 *************************/
	thread_queue_t *read_waiting_threads[2];
	thread_queue_t *write_waiting_threads[2];
} mailbox_t;

/*******************************
 * FUNCTIONS
 ******************************/

/* Initialize the mailbox given as parameter. The function will set the mailbox
 * structure and init the mailbox as empty. See system returns type for error 
 * handling.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The mailbox pointer given as parameter is NULL.
 *
 * @param mailbox A pointer to the mailbox to initialize. If NULL, the function
 * will immediatly return with the according error code.
 *
 * @returns The function returns OS_NO_ERR on success, see system returns type 
 * for further error description.
 */
OS_RETURN_E mailbox_init(mailbox_t *mailbox);

/* Pend on the mailbox given as parameter. This function will block the calling
 * thread if the mailbox is empty. See system returns type for error 
 * handling.
 * The function will return NULL in case of error and the error pointer given as
 * parameter will be set accordingly. The mailbox item might be a NULL pointer. 
 * In this case, error will be set to OS_NO_ERR if no error is detected.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The mailbox pointer given as parameter is NULL.
 * OS_ERR_MAILBOX_NON_INITIALIZED: The mailbox has not been initialized before.
 *
 * @param mailbox A pointer to the mailbox to initialize. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param error A pointer to the variable that contains the function success 
 * state. If NULL, the function will simply return NULL.
 *
 * @returns The function returns the content of the mailbox if error is set to
 * OS_NO_ERR, NULL otherwise.
 */
void* mailbox_pend(mailbox_t *mailbox, OS_RETURN_E *error);

/* Post on the mailbox given as parameter. This function will block the calling
 * thread if the mailbox is full. See system returns type for error 
 * handling.
 * The function will return the error code in case of error
 * The mailbox item might be a NULL pointer. 
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The mailbox pointer given as parameter is NULL.
 * OS_ERR_MAILBOX_NON_INITIALIZED: The mailbox has not been initialized before.
 *
 * @param mailbox A pointer to the mailbox to initialize. If NULL, the function
 * will immediatly return with the according error code.
 * @param element A pointer to the element to store in the mailbox. Only the 
 * pointer is stored in the mailbox, meaning the content of the pointed address
 * might change.
 *
 * @returns The function returns OS_NO_ERR on success, see system returns type 
 * for further error description.
 */
OS_RETURN_E mailbox_post(mailbox_t *mailbox, void *element);

/* Destroy the mailbox given as parameter. The function will set the mailbox
 * structure to uninitialized and destroy the mailbox. See system returns type 
 * for error handling.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The mailbox pointer given as parameter is NULL.
 * OS_ERR_MAILBOX_NON_INITIALIZED: The mailbox has not been initialized before.
 *
 * @param mailbox A pointer to the mailbox to initialize. If NULL, the function
 * will immediatly return with the according error code.
 *
 * @returns The function returns OS_NO_ERR on success, see system returns type 
 * for further error description.
 */
OS_RETURN_E mailbox_destroy(mailbox_t *mailbox);

/* Give the mailbox emptyness status. 
 *
 * The function will return -1 in case of error and the error pointer given as
 * parameter will be set accordingly. Returns values are 1 if the mailbox is 
 * empty and 0 otherwise.
 *
 * Possible OS_RETURN_E value:
 *
 * OS_NO_ERR: The process succeded.
 * OS_ERR_NULL_POINTER: The mailbox pointer given as parameter is NULL or the
 * error parameter is NULL.
 * OS_ERR_MAILBOX_NON_INITIALIZED: The mailbox has not been initialized before.
 *
 * @param mailbox A pointer to the mailbox to initialize. If NULL, the function
 * will immediatly return and set error with the according error code.
 * @param error A pointer to the variable that contains the function success 
 * state. If NULL, the function will simply return NULL.
 *
 * @returns The function returns -1 on error, 1 if the mailbox is empty and 0
 * otherwise.
 */
int8_t mailbox_isempty(mailbox_t *mailbox, OS_RETURN_E *error);


#endif /* __MAILBOX_H_ */