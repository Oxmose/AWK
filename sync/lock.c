/*******************************************************************************
 *
 * File: lock.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * Basic lock and synchronization primitives
 ******************************************************************************/

#include "../cpu/cpu.h" /* sti, cli */

/* Header file */
#include "lock.h"

/* Keep track on the nexting level */
static uint32_t int_lock_nesting = 0;

void enable_interrupt(void)
{
    if(int_lock_nesting > 0)
    {
        --int_lock_nesting;
    }
    if(int_lock_nesting == 0)
    {
        sti();
    }
}

void disable_interrupt(void)
{
    cli();
    ++int_lock_nesting;
}