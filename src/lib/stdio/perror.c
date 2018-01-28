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

#include "../stddef.h" /* OS_RETURN_E */

/* Header file */
#include "../stdio.h"

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
        case OS_ERR_MAILBOX_NON_INITIALIZED:
            printf("Mailbox not initialized");
            break;
        case OS_ERR_QUEUE_NON_INITIALIZED:
            printf("Queue not initialized");
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
        case OS_ERR_GRAPHIC_MODE_NOT_SUPPORTED:
            printf("Graphic mode is not supported");
            break;
        case OS_ERR_CHECKSUM_FAILED:
            printf("Checksum failed");
            break;
        case OS_ERR_ACPI_UNSUPPORTED:
            printf("ACPI unsuported");
            break;
        case OS_ERR_NO_SUCH_LAPIC_ID:
            printf("No such LAPIC id");
            break;
        case OS_ERR_NO_SUCH_SERIAL_BAUDRATE:
            printf("Baudrate not supported");
            break;
        case OS_ERR_NO_SUCH_SERIAL_PARITY:
            printf("Serial parity not supported");
            break;
        case OS_ERR_ATA_DEVICE_NOT_PRESENT:
            printf("ATA device is not present");
            break;
        case OS_ERR_ATA_DEVICE_ERROR:
            printf("ATA device error");
            break;
        case OS_ERR_ATA_BAD_SECTOR_NUMBER:
            printf("ATA driver do not support this sector number");
            break;
        case OS_ERR_ATA_SIZE_TO_HUGE:
            printf("ATA driver do not support this read/write size, try to \
divide your request into mulstiple read/write");
            break;
        case OS_ERR_VESA_NOT_SUPPORTED:
            printf("VESA is not supported");
            break;
        case OS_ERR_VESA_MODE_NOT_SUPPORTED:
            printf("VESA mode not supported");
            break;
        case OS_ERR_VESA_NOT_INIT:
            printf("VESA driver non initialized");
            break;
        case OS_ERR_NO_MORE_FREE_MEM:
            printf("No more free memory in the system");
            break;
        case OS_ERR_PAGING_NOT_INIT:
            printf("Paging has not been initialized");
            break;
        default:
            printf("Unknown error");
    }

    return 0;
}
