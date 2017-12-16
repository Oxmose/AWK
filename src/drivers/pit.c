/*******************************************************************************
 *
 * File: pit.c
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 17/12/2017
 *
 * Version: 1.0
 *
 * PIT (Programmable interval timer) driver
 * System main timer management. Allows to define system ticks.
 *
 ******************************************************************************/

#include "pic.h"                   /* set_IRQ_EOI, set_IRQ_mask */
#include "../cpu/cpu.h"            /* outb */
#include "../core/interrupts.h"    /* register_interrupt, cpu_state, 
                                    * stack_state */
#include "../lib/stdint.h"         /* Generioc int types */
#include "../lib/stddef.h"         /* OS_RETURN_E, OS_EVENT_ID */
#include "../sync/lock.h"          /* enable_interrupt, disable_interrupt */

/* Header include */
#include "pit.h"

/* Uptime in miliseconds */
static uint32_t uptime;

/* Tick counter, circular when reaching overflow */
static uint32_t tick_count;

/* Events table */
static pit_event_t pit_events[PIT_MAX_EVENT_COUNT];

void pit_interrupt_handler(cpu_state_t *cpu_state, uint32_t int_id, 
                           stack_state_t *stack_state)
                                             
{
    /* Update system time */
    ++tick_count;
    uptime += 1000 / PIT_FREQ;

    (void)cpu_state;
    (void)stack_state;
    (void)int_id;

    /* Send EOI signal */
    set_IRQ_EOI(PIT_IRQ);
}

OS_RETURN_E init_pit(void)
{
    /* Init system times */
    uptime = 0;
    tick_count = 0;

    /* Init clock events */
    uint32_t i;
    for(i = 0; i < PIT_MAX_EVENT_COUNT; ++i)
    {
        pit_events[i].enabled = 0;
    }

    /* Set clock frequency */
    uint16_t tick_freq = PIT_QUARTZ_FREQ / PIT_FREQ;
    outb(PIT_COMM_SET_FREQ, PIT_COMM_PORT);
    outb(tick_freq & 0x00FF, PIT_DATA_PORT);
    outb(tick_freq >> 8, PIT_DATA_PORT);

    /* Set clock interrupt handler */
    OS_RETURN_E err = register_interrupt_handler(PIT_INTERRUPT_LINE, 
                                                 pit_interrupt_handler);

    if(err == OS_NO_ERR)
    {
        /* Set IRQ mask for system clock */
        err = set_IRQ_mask(PIT_IRQ, 1);
    }

    return err;
}

uint32_t get_tick_count(void)
{
    return tick_count;
}

uint32_t get_current_uptime(void)
{
    return uptime;
}

OS_RETURN_E register_pit_event(void (*function)(void*), 
                                 const uint32_t period,
                                 OS_EVENT_ID *event_id)
{ 
    if(function == NULL)
    {
        if(event_id != NULL)
        {
            *event_id = -1;
        }
        return OS_ERR_NULL_POINTER;
    }

    disable_interrupt();

    /* Search for free event id */
    uint32_t i;
    for(i = 0; i < PIT_MAX_EVENT_COUNT && pit_events[i].enabled == 1; ++i);

    if(i == PIT_MAX_EVENT_COUNT)
    {
        if(event_id != NULL)
        {
            *event_id = -1;
        }
        enable_interrupt();
        return OS_ERR_NO_MORE_FREE_EVENT;
    }

    /* Create new event */
    pit_events[i].execute  = function;
    pit_events[i].period   = period;
    pit_events[i].event_id = i;
    pit_events[i].enabled  = 1;
    if(event_id != NULL)
    {
        *event_id = -1;
    }

    enable_interrupt();

    return OS_NO_ERR;
}

OS_RETURN_E unregister_pit_event(const OS_EVENT_ID event_id)
{
    if(event_id >= PIT_MAX_EVENT_COUNT)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    disable_interrupt();

    if(pit_events[event_id].enabled == 0)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Disable event */
    pit_events[event_id].execute = NULL;
    pit_events[event_id].period  = 0;
    pit_events[event_id].enabled = 0;

    enable_interrupt();

    return OS_NO_ERR;
}