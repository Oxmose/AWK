/*******************************************************************************
 *
 * File: stddef.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 04/10/2017
 *
 * Version: 1.0
 *
 * Standard definitions for the kernel.
 ******************************************************************************/

#ifndef __STDDEF_H_
#define __STDDEF_H_

#include "stdint.h" /* Generic int types */

#define NULL ((void *)0)

/* System return states */
typedef enum __attribute__ ((packed, aligned(2))) OS_RETURN
{
    OS_NO_ERR                              = 0,

    OS_ERR_NULL_POINTER                    = 1,
    OS_ERR_OUT_OF_BOUND                    = 2,
    
    OR_ERR_UNAUTHORIZED_INTERRUPT_LINE     = 3,
    OS_ERR_INTERRUPT_ALREADY_REGISTERED    = 4,
    OS_ERR_INTERRUPT_NOT_REGISTERED        = 5,
    
    OS_ERR_NO_SUCH_IRQ_LINE                = 6,
    
    OS_ERR_NO_MORE_FREE_EVENT              = 7,
    OS_ERR_NO_SUCH_ID                      = 8,
    OS_ERR_MALLOC                          = 9,
    OS_ERR_UNAUTHORIZED_ACTION             = 10,
    OS_ERR_FORBIDEN_PRIORITY               = 11,

    OS_ERR_MUTEX_UNINITIALIZED             = 12,
    OS_ERR_SEM_UNINITIALIZED               = 13,
    OS_ERR_MAILBOX_NON_INITIALIZED         = 14,                         
    OS_ERR_QUEUE_NON_INITIALIZED           = 15, 

    OS_ERR_NO_SEM_BLOCKED                  = 16,
    OS_ERR_NO_MUTEX_BLOCKED                = 17,
    OS_ERR_NO_QUEUE_BLOCKED                = 18,

    OS_ERR_GRAPHIC_MODE_NOT_SUPPORTED      = 19,

    OS_MUTEX_LOCKED                        = 20,
    OS_SEM_LOCKED                          = 21,

    OS_ERR_CHECKSUM_FAILED                 = 22,
    OS_ERR_ACPI_UNSUPPORTED                = 23,

    OS_ERR_NO_SUCH_LAPIC_ID                = 24,
} OS_RETURN_E;

typedef int32_t OS_EVENT_ID;

#ifndef __SIZE_TYPE__
#error __SIZE_TYPE__ not defined
#endif

typedef __SIZE_TYPE__ size_t;

#ifndef __PTRDIFF_TYPE__
#error __PTRDIFF_TYPE__ not defined
#endif

typedef __PTRDIFF_TYPE__ ptrdiff_t;

#ifdef NDEBUG

#define assert(expr) ((void)0)

#else

#define assert(expr) \
	((void)((expr) ? 0 : \
		(panic(__FILE__":%u: failed assertion `"#expr"'\n", \
			__LINE__), 0)))

#endif


#endif /* __STDDEF_H_ */