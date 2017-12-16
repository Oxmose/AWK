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

typedef uint32_t size_t;

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
    
} OS_RETURN_E;

#endif /* __STDDEF_H_ */