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
typedef enum OS_RETURN
{
    OS_NO_ERR,
    OS_ERR_NULL_POINTER,
    OS_ERR_OUT_OF_BOUND,
    OR_ERR_UNAUTHORIZED_INTERRUPT_LINE,
    OS_ERR_INTERRUPT_ALREADY_REGISTERED,
    OS_ERR_INTERRUPT_NOT_REGISTERED,
    OS_ERR_NO_SUCH_IRQ_LINE,
    OS_ERR_NO_MORE_FREE_EVENT,
    OS_ERR_NO_SUCH_ID,
    OS_ERR_MALLOC,
    OS_ERR_UNAUTHORIZED_ACTION,
    OS_ERR_FORBIDEN_PRIORITY
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