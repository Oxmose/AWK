/*******************************************************************************
 *
 * File: perror.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * perror function. Used to print OS error codes in a string way using the error
 * code.
 *
 ******************************************************************************/

#include "stddef.h" /* OS_RETURN_E */

/* Header file */
#include "stdio.h"

int perror(const int error)
{
	switch(error)
	{
		case OS_ERR_OUT_OF_BOUND:
		    printf("Index out of bound");
		    break;
	    case OS_ERR_NO_MORE_FREE_EVENT:
	    	printf("No more free event to allocate");
	    	break;	    	
	    case OS_ERR_NULL_POINTER:
	    	printf("Null pointer used");
	    	break;
	    case OS_ERR_NO_SUCH_ID:
	    	printf("No such id");
	    	break;
	    case OR_ERR_UNAUTHORIZED_INTERRUPT_LINE:
	    	printf("No such interrupt line or protected line");
	    	break;
	    case OS_ERR_INTERRUPT_ALREADY_REGISTERED:
	    	printf("A handler is already registered for this line.");
	    	break;
	    case OS_ERR_INTERRUPT_NOT_REGISTERED:
	    	printf("Not any handler registered for this line.");
	    	break;
	    case OS_ERR_NO_SUCH_IRQ_LINE:
	    	printf("No such IRQ line");
	    	break;
	    case OS_ERR_MALLOC:
	    	printf("Malloc error");
	    	break;
	    case OS_ERR_UNAUTHORIZED_ACTION:
	    	printf("Unauthorized action");
	    	break;
	    case OS_ERR_FORBIDEN_PRIORITY:
	    	printf("Forbiden priority");
	    	break;
	    case OS_ERR_MUTEX_UNINITIALIZED:
	    	printf("Mutex not initialized");
	    	break;
	    case OS_ERR_SEM_UNINITIALIZED:
	    	printf("Semaphore not initialized");
	    	break;
	    case OS_ERR_NO_SEM_BLOCKED:
	    	printf("Thread is not blocked by semaphore");
	    	break;
	    case OS_ERR_NO_MUTEX_BLOCKED:
	    	printf("Thread is not blocked by mutex");
	    	break;
	    case OS_ERR_NO_QUEUE_BLOCKED:
	    	printf("Thread is not blocked by queue");
	    	break;
	    case OS_MUTEX_LOCKED:
	    	printf("Mutex locked");
	    	break;
	    case OS_SEM_LOCKED:
	    	printf("Semaphore locked");
	    	break;
	    default:
	    	printf("Unknown error");
	}

	return 0;
}